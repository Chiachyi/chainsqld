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
#include <ripple/rpc/Context.h>
#include <ripple/protocol/ErrorCodes.h>
#include <ripple/protocol/JsonFields.h>
#include <ripple/rpc/impl/RPCHelpers.h>
#include <peersafe/app/table/TableSync.h>


namespace ripple {

std::pair<Json::Value, bool> checkJsonFields(Json::Value originJson)
{
	std::pair<Json::Value, bool> ret;
	ret.second = false;
	if (!originJson.isObject())
	{
		ret.first = RPC::object_field_error(jss::params);
		return ret;
	}

	if (!originJson.isMember(jss::account))
	{
		ret.first = RPC::make_error(rpcSRC_ACT_MISSING,
			RPC::missing_field_message("account"));
		return ret;
	}

	if (!originJson.isMember(jss::tablename))
	{
		ret.first = RPC::make_error(rpcINVALID_PARAMS,
			RPC::missing_field_message("tablename"));
		return ret;
	}

	ret.first = Json::Value();
	ret.second = true;
	return ret;
}
// FIXME: This leaks RPCSub objects for JSON-RPC.  Shouldn't matter for anyone
// sane.
Json::Value doAddEncTabToSync(RPC::Context& context)
{
    Json::Value jvResult (Json::objectValue);

#ifdef GM_ALG_PROCESS

	// Application& appTemp = context.app;
	std::string errMsgStr("");

	//Json::Value jsonRpcObj = context.params[jss::tx_json];
	Json::Value jsonParams = context.params;

	auto checkResult = checkJsonFields(jsonParams);
	if (!checkResult.second)
		return checkResult.first;
	auto const srcAddressID = parseBase58<AccountID>(jsonParams[jss::account].asString());
	if (srcAddressID == boost::none)
	{
		errMsgStr = "account field is invalid!";
		return RPC::make_error(rpcINVALID_PARAMS, errMsgStr);
	}
	
	std::string tableNameStr = jsonParams[jss::tablename].asString();
	if ("" == tableNameStr)
	{
		errMsgStr = "tablename is invalid";
		return RPC::make_error(rpcINVALID_PARAMS, errMsgStr);
	}
    
	auto addRet = context.app.getTableSync().CreateSingleItemFromAPI(*srcAddressID, tableNameStr);
    if(!addRet.first)
    {
        return RPC::make_error(rpcINTERNAL, addRet.second);
    }
#else

    jvResult = RPC::make_error(rpcUNKNOWN_COMMAND, "This API just for GMAlg");
#endif

    return jvResult;
}

} // ripple
