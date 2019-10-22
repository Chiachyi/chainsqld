
#include <peersafe/app/misc/TxPool.h>
#include <ripple/app/ledger/LedgerMaster.h>
#include <ripple/core/JobQueue.h>

namespace ripple {

    h256Set TxPool::topTransactions(uint64_t const& limit)
    {
        h256Set ret;
        int txCnt = 0;

        std::lock_guard<std::mutex> lock(mutexTxPoll_);

        JLOG(j_.info()) << "Currently mTxsSet size: " << mTxsSet.size() 
            << ", mAvoid size: " << mAvoid.size();

        for (auto iter = mTxsSet.begin(); txCnt < limit && iter != mTxsSet.end(); ++iter)
        {
            if (!mAvoid.count((*iter)->getID()))
            {
                ret.insert((*iter)->getID());
                txCnt++;

                // update avoid set
                mAvoid.insert((*iter)->getID());
            }
        }

        return ret;
    }

    TER TxPool::insertTx(std::shared_ptr<Transaction> transaction,int ledgerSeq)
    {
        std::lock_guard<std::mutex> lock(mutexTxPoll_);

        if (mTxsSet.size() >= mMaxTxsInPool)
        {
            JLOG(j_.warn()) << "Txs pool is full, insert failed, Tx hash: " 
                << transaction->getID();
            return telTX_POOL_FULL;
        }

        TER ter = tefPAST_SEQ;
        auto result = mTxsSet.insert(transaction);

        JLOG(j_.trace()) << "Inserting a " << (result.second ? "new" : "exist")
            << " Tx: " << transaction->getID();

        if (result.second)
        {
            if (mTxsHash.emplace(make_pair(transaction->getID(), result.first)).second)
            {
                ter = tesSUCCESS;
				// Init sync_status
				if (mSyncStatus.pool_start_seq == 0)
				{
					mSyncStatus.pool_start_seq = ledgerSeq;
				}
            }
            else
            {
                JLOG(j_.error()) << "mTxsHash.emplace failed, Tx: " << transaction->getID();
                mTxsSet.erase(transaction);
                ter = telLOCAL_ERROR;
            }
        }

        return ter;
    }

    void TxPool::removeTxs(SHAMap const& cSet, int const ledgerSeq, uint256 const& prevHash)
    {
        std::lock_guard<std::mutex> lock(mutexTxPoll_);

		JLOG(j_.info()) << "Remove txs for ledger "<< ledgerSeq;
		TransactionSet::iterator iterSet;
        for (auto const& item : cSet)
        {
            if (!txExists(item.key()))
                continue;

            try
            {
                // If not exist, throw std::out_of_range exception.
                iterSet = mTxsHash.at(item.key());

                // remove from Tx pool.
                mTxsHash.erase(item.key());
                mTxsSet.erase(iterSet);

                // remove from avoid set.
                mAvoid.erase(item.key());
            }
            catch (std::exception const&)
            {
                JLOG(j_.warn()) << "Tx: " << item.key() << " throws, not in pool";
            }
        }

		//update sync_status
		if (mTxsSet.size() == 0)
		{
			mSyncStatus.init();
			return;
		}

		if (mSyncStatus.pool_start_seq > 0)
		{
			if (mSyncStatus.max_advance_seq < ledgerSeq)
			{
				mSyncStatus.max_advance_seq = ledgerSeq;
			}
			
			if (ledgerSeq == mSyncStatus.pool_start_seq)
			{
				mSyncStatus.pool_start_seq = mSyncStatus.max_advance_seq + 1;
			}
			mSyncStatus.prevHash = prevHash;
		}
    }

    void TxPool::updateAvoid(RCLTxSet const& cSet)
    {
        // If the Tx set had be added into avoid set recently, don't add it again.
        // TODO
        if (0)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(mutexTxPoll_);

        for (auto const& item : *(cSet.map_))
        {
            if (txExists(item.key()))
                mAvoid.insert(item.key());
        }
    }

	bool TxPool::isAvailable()
	{
		return mSyncStatus.max_advance_seq <= mSyncStatus.pool_start_seq;
	}

	void TxPool::timerEntry()
	{
		if (!isAvailable())
		{
			// we need to switch the ledger we're working from
			auto prevLedger = app_.getLedgerMaster().getLedgerByHash(mSyncStatus.prevHash);
			if (prevLedger)
			{
				JLOG(j_.info()) << "TxPool found ledger " << prevLedger->info().seq;
				removeTxs(prevLedger->txMap(), prevLedger->info().seq, prevLedger->info().parentHash);
			}
		}
	}

	void TxPool::removeTx(uint256 hash)
	{
		std::lock_guard<std::mutex> lock(mutexTxPoll_);
		try
		{
			// If not exist, throw std::out_of_range exception.
			auto iterSet = mTxsHash.at(hash);

			// remove from Tx pool.
			mTxsHash.erase(hash);
			mTxsSet.erase(iterSet);

			// remove from avoid set.
			mAvoid.erase(hash);
		}
		catch (std::exception const&)
		{
			JLOG(j_.warn()) << "Tx: " << hash << " throws in removetx, not in pool";
		}

		if (mTxsSet.size() == 0)
		{
			mSyncStatus.init();
		}
	}
}