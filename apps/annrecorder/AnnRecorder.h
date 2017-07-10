/*
 * Copyright (C) 2008 iptego GmbH
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

#ifndef _ANNRECORDER_H_
#define _ANNRECORDER_H_

#include "AmAudioFile.h"
#include "AmConfigReader.h"
#include "AmPromptCollection.h"
#include "AmSession.h"
#include "AmUACAuth.h"

#include <map>
#include <memory>
#include <string>

// prompts used

//"Welcome to iptel dot org voip service."
#define WELCOME "welcome"
// "Your auto attendant greeting sounds like this: -"
#define YOUR_PROMPT "your_prompt"
// "To record a new auto attendant greeting, press any key. End recording with
// any key. -"
#define TO_RECORD "to_record"
//"Press one to keep the new greeting, or two to record a new one. -"
#define CONFIRM "confirm"
// "Your new auto attendant greeting has been set."
#define GREETING_SET "greeting_set"
// "Thank you for using the iptel dot org service. Good Bye. - "
#define BYE "bye"
#define BEEP "beep"

#define ANNREC_ANNOUNCE_PATH "/usr/local/lib/sems/audio/annrecorder/"

/** \brief Factory for announcement sessions */
class AnnRecorderFactory : public AmSessionFactory
{
  void getAppParams(const AmSipRequest& req,
                    std::map<std::string, std::string>& params);
  AmPromptCollection prompts;

 public:
  static AmDynInvokeFactory* message_storage_fact;

  static std::string AnnouncePath;
  static std::string DefaultAnnounce;
  static bool        SimpleMode;

  AnnRecorderFactory(const std::string& _app_name);

  int        onLoad();
  AmSession* onInvite(const AmSipRequest& req, const std::string& app_name,
                      const std::map<std::string, std::string>&   app_params);
  AmSession* onInvite(const AmSipRequest& req, const std::string& app_name,
                      AmArg& session_params);
};

/**\brief  announcement session logic implementation */
class AnnRecorderDialog
    : public AmSession
    , public CredentialHolder
{
  AmPromptCollection& prompts;
  AmPlaylist          playlist;
  // we need only one separator in queue
  std::unique_ptr<AmPlaylistSeparator> playlist_separator;

  AmAudioFile wav_file;
  std::map<std::string, std::string> params;

  std::string msg_filename; // recorded file

  AmDynInvoke* msg_storage;

  std::unique_ptr<UACAuthCred> cred;

  enum AnnRecorderState
  {
    S_WAIT_START,
    S_BYE,
    S_RECORDING,
    S_CONFIRM
  };

  AnnRecorderState state;
  void             enqueueCurrent();
  void             saveAndConfirm();
  void             replayRecording();
  void enqueueSeparator(int id);

  FILE* getCurrentMessage();
  void saveMessage(FILE* fp);

 public:
  AnnRecorderDialog(const std::map<std::string, std::string>& params,
                    AmPromptCollection& prompts,
                    UACAuthCred*        credentials = NULL);
  ~AnnRecorderDialog();

  void onSessionStart();
  void onBye(const AmSipRequest& req);
  void onDtmf(int event, int duration_msec);

  void process(AmEvent* event);

  UACAuthCred* getCredentials();
};

#endif
// Local Variables:
// mode:C++
// End:
