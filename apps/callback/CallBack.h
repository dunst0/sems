/*
 * Copyright (C) 2007 iptego GmbH
 *
 * This file is part of SEMS, a free SIP media server.
 *
 * SEMS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * For a license to use the sems software under conditions
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
#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#include "AmApi.h"
#include "AmAudio.h"
#include "AmB2ABSession.h"
#include "AmPlaylist.h"
#include "AmPromptCollection.h"
#include "AmUACAuth.h"

#include <regex.h>
#include <sys/types.h>

#include <map>
#include <string>

// configuration parameter names
#define WELCOME_PROMPT "welcome_prompt"
#define DIGITS_DIR "digits_dir"
#define ACCEPT_CALLER_RE "accept_caller_re"

class CallBackFactory
    : public AmSessionFactory
    , public AmThread
{
  AmPromptCollection prompts;

  // UNUSED
  // bool configured;
  // UNUSED_END

  regex_t accept_caller_re;

  std::multimap<time_t, std::string> scheduled_calls;
  AmMutex scheduled_calls_mut;
  // seconds to wait before calling back
  int cb_wait;

  void createCall(const std::string& number);

 public:
  static std::string gw_user;
  static std::string gw_domain;
  static std::string auth_user;
  static std::string auth_pwd;

  static std::string DigitsDir;
  static PlayoutType m_PlayoutType;

  CallBackFactory(const std::string& _app_name);
  ~CallBackFactory();
  AmSession* onInvite(const AmSipRequest&, const std::string&   app_name,
                      const std::map<std::string, std::string>& app_params);
  AmSession* onInvite(const AmSipRequest& req, const std::string& app_name,
                      AmArg& session_params);
  int onLoad();

  void run();
  void on_stop();
};

enum CBState
{
  CBNone = 0,
  CBEnteringNumber,
  CBTellingNumber,
  CBConnecting,
  CBConnected
};

class CallBackDialog
    : public AmB2ABCallerSession
    , public CredentialHolder
{
 private:
  AmPlaylist play_list;

  AmPromptCollection& prompts;

  std::string  call_number;
  UACAuthCred* cred;

  CBState state;

 public:
  CallBackDialog(AmPromptCollection& prompts, UACAuthCred* cred);
  ~CallBackDialog();

  void process(AmEvent* ev);
  void onInvite(const AmSipRequest& req);
  void onSessionStart();
  void onDtmf(int event, int duration);

  UACAuthCred*         getCredentials() { return cred; }
  AmB2ABCalleeSession* createCalleeSession();
};

class CallBackCalleeDialog
    : public AmB2ABCalleeSession
    , public CredentialHolder
{
  UACAuthCred* cred;

 public:
  CallBackCalleeDialog(const std::string&       other_tag,
                       AmSessionAudioConnector* connector, UACAuthCred* cred);
  ~CallBackCalleeDialog();
  UACAuthCred* getCredentials() { return cred; }
};

#endif
