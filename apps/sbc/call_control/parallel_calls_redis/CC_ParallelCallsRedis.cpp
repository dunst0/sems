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

#define SBCVAR_PARALLEL_CALLS_UUID "uuid"

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

unsigned int     CCParallelCallsRedis::refuse_code   = 402;
string           CCParallelCallsRedis::refuse_reason = "Too Many Simultaneous Calls";
CCParallelCallsRedis* CCParallelCallsRedis::_instance     = NULL;

CCParallelCallsRedis::CCParallelCallsRedis() {}

CCParallelCallsRedis::~CCParallelCallsRedis() {
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
                            SBCCallProfile* call_profile, const AmArg& values,
                            const AmSipRequest* req, AmArg& res)
{
  unsigned int max_calls     = 1; // default
  unsigned int current_calls = 0;
  bool         do_limit      = false;
  string       uuid;

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

  ERROR("YUMMMY - c %s f %s\n", req->callid.c_str(), req->from_tag.c_str());

  call_profile->cc_vars[cc_namespace + "::" + SBCVAR_PARALLEL_CALLS_UUID] =
      uuid;

  if (values.hasMember("max_calls") && isArgCStr(values["max_calls"])) {
    if (str2i(values["max_calls"].asCStr(), max_calls)) {
      ERROR("max_calls '%s' could not be interpreted!\n",
            values["max_calls"].asCStr());
      goto error;
    }
  }

  DBG("enforcing limit of %i calls for uuid '%s'\n", max_calls, uuid.c_str());

  call_control_mutex.lock();
  if (max_calls) {
    map<string, unsigned int>::iterator it = call_control_calls_count.find(uuid);
    if (it == call_control_calls_count.end()) {
      call_control_calls_count[uuid] = current_calls = 1;
    }
    else {
      if (it->second < max_calls) {
        it->second++;
      }
      else {
        do_limit = true;
      }

      current_calls = it->second;
    }
  }
  call_control_mutex.unlock();

  DBG("uuid %s has %u active calls (limit = %s)\n", uuid.c_str(), current_calls,
      do_limit ? "true" : "false");

  if (do_limit) {
    vector<string> hdrs;
    hdrs.push("TLN-CC: cc_pcalls;uuid=");

    res.push(AmArg());
    AmArg& res_cmd                 = res[0];
    res_cmd[SBC_CC_ACTION]         = SBC_CC_REFUSE_ACTION;
    res_cmd[SBC_CC_REFUSE_CODE]    = (int) refuse_code;
    res_cmd[SBC_CC_REFUSE_REASON]  = refuse_reason;
    res_cmd[SBC_CC_REFUSE_HEADERS] = AmArg(hdrs);
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
  if (!call_profile) {
    ERROR("internal: call_profile object not found in parameters\n");
    return;
  }

  SBCVarMapIterator vars_it = call_profile->cc_vars.find(
      cc_namespace + "::" + SBCVAR_PARALLEL_CALLS_UUID);
  if (vars_it == call_profile->cc_vars.end() || !isArgCStr(vars_it->second)) {
    ERROR("internal: could not find UUID for ending call '%s'\n", ltag.c_str());
    return;
  }
  string uuid = vars_it->second.asCStr();
  call_profile->cc_vars.erase(cc_namespace + "::" + SBCVAR_PARALLEL_CALLS_UUID);

  unsigned int new_call_count = 0;

  call_control_mutex.lock();
  if (call_control_calls_count[uuid] > 1) {
    new_call_count = --call_control_calls_count[uuid];
  }
  else {
    call_control_calls_count.erase(uuid);
  }
  call_control_mutex.unlock();

  DBG("uuid '%s' now has %u active calls\n", uuid.c_str(), new_call_count);
}
