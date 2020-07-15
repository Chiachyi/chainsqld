//------------------------------------------------------------------------------
/*
    This file is part of rippled: https://github.com/ripple/rippled
    Copyright (c) 2012-2014 Ripple Labs Inc.

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#include <BeastConfig.h>
#include <ripple/net/RPCErr.h>
#include <ripple/protocol/ErrorCodes.h>
#include <ripple/protocol/JsonFields.h>
#include <ripple/rpc/Context.h>
#include <ripple/rpc/impl/RPCHelpers.h>


namespace ripple {

// FIXME: This leaks RPCSub objects for JSON-RPC.  Shouldn't matter for anyone
// sane.
Json::Value doGetNodePub(RPC::Context& context)
{
    Json::Value jvResult (Json::objectValue);

#ifdef GM_ALG_PROCESS

	std::pair<std::string, std::string> pubAddrRet = getPubAddrInCard(
        HardEncryptObj::getInstance()->nodeVerifyKey,HardEncryptObj::getInstance()->nodeKeyIndex_);
    jvResult[jss::public_key] = pubAddrRet.first;
    jvResult[jss::account_id] = pubAddrRet.second;

#else

    jvResult = RPC::make_error(rpcUNKNOWN_COMMAND, "This API just for GMAlg");
#endif

    return jvResult;
}

Json::Value doGetSyncPub(RPC::Context& context)
{
    Json::Value jvResult(Json::objectValue);

#ifdef GM_ALG_PROCESS
    std::pair<std::string, std::string> pubAddrRet = getPubAddrInCard(
        HardEncryptObj::getInstance()->syncTableKey,SYNC_TABLE_KEY_INDEX);
    jvResult[jss::public_key] = pubAddrRet.first;
    jvResult[jss::account_id] = pubAddrRet.second;

#else

    jvResult = RPC::make_error(rpcUNKNOWN_COMMAND, "This API just for GMAlg");
#endif

	return jvResult;
}

} // ripple
