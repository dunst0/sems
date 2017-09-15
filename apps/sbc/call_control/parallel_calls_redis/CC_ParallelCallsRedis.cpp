/*
 * Copyright (C) 2011 Stefan Sayer
 *
 * This file is part of SEMS, a free SIP media server.
 *
 * SEMS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * For a license to use the SEMS software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact iptel.org by e-mail at the following addresses:
 *    info@iptel.org
 *
 * SEMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "CC_ParallelCallsRedis.h"

#include "SBCCallControlAPI.h"

#include "AmArg.h"
#include "AmPlugIn.h"
#include "AmSipHeaders.h"
#include "log.h"

#include <string.h>

using std::string;
using std::map;
using std::vector;

#define SBCVAR_PARALLEL_CALLS_REDIS_UUID "uuid"
#define SBCVAR_PARALLEL_CALLS_REDIS_CALLID "callid"
#define SBCVAR_PARALLEL_CALLS_REDIS_FROM_TAG "from_tag"

class CCParallelCallsRedisFactory : public AmDynInvokeFactory
{
 public:
  CCParallelCallsRedisFactory(const string& name)
      : AmDynInvokeFactory(name)
  {
  }

  AmDynInvoke* getInstance() { return CCParallelCallsRedis::instance(); }

  int onLoad()
  {
    if (CCParallelCallsRedis::instance()->onLoad()) return -1;

    DBG("redis parallel call control loaded.\n");

    return 0;
  }
};

EXPORT_PLUGIN_CLASS_FACTORY(CCParallelCallsRedisFactory, MOD_NAME);

unsigned int CCParallelCallsRedis::refuse_code = 402;
string CCParallelCallsRedis::refuse_reason     = "Too Many Simultaneous Calls";
string CCParallelCallsRedis::refuse_header     = "";
bool   CCParallelCallsRedis::strict            = true;
CCParallelCallsRedis* CCParallelCallsRedis::_instance = NULL;

CCParallelCallsRedis::CCParallelCallsRedis() {}

CCParallelCallsRedis::~CCParallelCallsRedis()
{
  if (callCounter) {
    delete callCounter;
  }

  if (_instance) {
    delete _instance;
  }
}

CCParallelCallsRedis* CCParallelCallsRedis::instance()
{
  if (!_instance) {
    _instance = new CCParallelCallsRedis();
  }

  return _instance;
}

int CCParallelCallsRedis::onLoad()
{
  AmConfigReader cfg;

  if (cfg.loadFile(AmConfig::ModConfigPath + string(MOD_NAME ".conf"))) {
    INFO(MOD_NAME "configuration  file (%s) not found, assuming default "
                  "configuration is fine\n",
         (AmConfig::ModConfigPath + string(MOD_NAME ".conf")).c_str());
    return 0;
  }

  refuse_reason = cfg.getParameter("refuse_reason", refuse_reason);

  if (cfg.hasParameter("refuse_code")) {
    if (str2i(cfg.getParameter("refuse_code"), refuse_code)) {
      ERROR("refuse_code '%s' not understood\n",
            cfg.getParameter("refuse_code").c_str());
      return -1;
    }
  }

  refuse_header = cfg.getParameter("refuse_header", refuse_header);
  strict        = cfg.getParameter("strict") == "yes";
  DBG("parallel calls restriction strict is: %s\n", strict ? "on" : "off");

  callCounter = new CallCounter(strict, false);

  return 0;
}

void CCParallelCallsRedis::invoke(const string& method, const AmArg& args,
                                  AmArg& ret)
{
  if (method == "start") {
    SBCCallProfile* call_profile = dynamic_cast<SBCCallProfile*>(
        args[CC_API_PARAMS_CALL_PROFILE].asObject());

    const AmSipRequest* req = dynamic_cast<const AmSipRequest*>(
        args[CC_API_PARAMS_SIP_MSG].asObject());

    start(args[CC_API_PARAMS_CC_NAMESPACE].asCStr(),
          args[CC_API_PARAMS_LTAG].asCStr(), call_profile,
          args[CC_API_PARAMS_CFGVALUES], req, ret);
  }
  else if (method == "connect") {
    // no action
  }
  else if (method == "end") {
    args[CC_API_PARAMS_TIMESTAMPS].assertArrayFmt("iiiiii");

    SBCCallProfile* call_profile = dynamic_cast<SBCCallProfile*>(
        args[CC_API_PARAMS_CALL_PROFILE].asObject());

    end(args[CC_API_PARAMS_CC_NAMESPACE].asCStr(),
        args[CC_API_PARAMS_LTAG].asCStr(), call_profile);
  }
  else if (method == CC_INTERFACE_MAND_VALUES_METHOD) {
    ret.push("uuid");
  }
  else if (method == "_list") {
    ret.push("start");
    ret.push("connect");
    ret.push("end");
  }
  else {
    throw AmDynInvoke::NotImplemented(method);
  }
}

void CCParallelCallsRedis::start(const string& cc_namespace, const string& ltag,
                                 SBCCallProfile* call_profile,
                                 const AmArg& values, const AmSipRequest* req,
                                 AmArg& res)
{
  unsigned int max_calls = 1;
  string       uuid;

  if (!req) {
    ERROR("internal: req object not found in parameters\n");
    goto error;
  }

  if (!call_profile) {
    ERROR("internal: call_profile object not found in parameters\n");
    goto error;
  }

  if (!values.hasMember("uuid") || !isArgCStr(values["uuid"])
      || !strlen(values["uuid"].asCStr())) {
    ERROR("configuration error: uuid missing for parallel calls redis"
          "call control!\n");
    goto error;
  }

  uuid = values["uuid"].asCStr();

  call_profile
      ->cc_vars[cc_namespace + "::" + SBCVAR_PARALLEL_CALLS_REDIS_UUID] = uuid;
  call_profile
      ->cc_vars[cc_namespace + "::" + SBCVAR_PARALLEL_CALLS_REDIS_CALLID] =
      req->callid;
  call_profile
      ->cc_vars[cc_namespace + "::" + SBCVAR_PARALLEL_CALLS_REDIS_FROM_TAG] =
      req->from_tag;

  if (values.hasMember("max_calls") && isArgCStr(values["max_calls"])) {
    if (str2i(values["max_calls"].asCStr(), max_calls)) {
      ERROR("max_calls '%s' could not be interpreted!\n",
            values["max_calls"].asCStr());
      goto error;
    }
  }

  if (!callCounter->tryIncrement(uuid, req->callid, req->from_tag, max_calls)) {
    res.push(AmArg());
    AmArg& res_cmd                = res[0];
    res_cmd[SBC_CC_ACTION]        = SBC_CC_REFUSE_ACTION;
    res_cmd[SBC_CC_REFUSE_CODE]   = (int) refuse_code;
    res_cmd[SBC_CC_REFUSE_REASON] = refuse_reason;

    if (!refuse_header.empty()) {
      res_cmd[SBC_CC_REFUSE_HEADERS] = AmArg();
      res_cmd[SBC_CC_REFUSE_HEADERS].push(
          refuse_header + string(": " MOD_NAME ";uuid=") + uuid);
    }
  }

  return;

error:
  res.push(AmArg());
  AmArg& res_cmd                = res[0];
  res_cmd[SBC_CC_ACTION]        = SBC_CC_REFUSE_ACTION;
  res_cmd[SBC_CC_REFUSE_CODE]   = 500;
  res_cmd[SBC_CC_REFUSE_REASON] = SIP_REPLY_SERVER_INTERNAL_ERROR;
}

void CCParallelCallsRedis::end(const string& cc_namespace, const string& ltag,
                               SBCCallProfile* call_profile)
{
  string uuid;
  string callid;
  string from_tag;

  if (!call_profile) {
    ERROR("internal: call_profile object not found in parameters\n");
    return;
  }

  SBCVarMapIterator vars_it;

  vars_it = call_profile->cc_vars.find(
      cc_namespace + "::" + SBCVAR_PARALLEL_CALLS_REDIS_UUID);
  if (vars_it == call_profile->cc_vars.end() || !isArgCStr(vars_it->second)) {
    ERROR("internal: could not find UUID for ending call '%s'\n", ltag.c_str());
  }
  else {
    uuid = vars_it->second.asCStr();
  }

  vars_it = call_profile->cc_vars.find(
      cc_namespace + "::" + SBCVAR_PARALLEL_CALLS_REDIS_CALLID);
  if (vars_it == call_profile->cc_vars.end() || !isArgCStr(vars_it->second)) {
    ERROR("internal: could not find callid for ending call '%s'\n",
          ltag.c_str());
  }
  else {
    callid = vars_it->second.asCStr();
  }

  vars_it = call_profile->cc_vars.find(
      cc_namespace + "::" + SBCVAR_PARALLEL_CALLS_REDIS_FROM_TAG);
  if (vars_it == call_profile->cc_vars.end() || !isArgCStr(vars_it->second)) {
    ERROR("internal: could not find from_tag for ending call '%s'\n",
          ltag.c_str());
  }
  else {
    from_tag = vars_it->second.asCStr();
  }

  if (uuid.empty() || callid.empty() || from_tag.empty()) {
    return;
  }

  if (callCounter->decrement(uuid, callid, from_tag)) {
    call_profile->cc_vars.erase(cc_namespace
                                + "::" + SBCVAR_PARALLEL_CALLS_REDIS_UUID);
    call_profile->cc_vars.erase(cc_namespace
                                + "::" + SBCVAR_PARALLEL_CALLS_REDIS_CALLID);
    call_profile->cc_vars.erase(cc_namespace
                                + "::" + SBCVAR_PARALLEL_CALLS_REDIS_FROM_TAG);
  }
}
