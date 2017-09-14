/*
 * Copyright (C) 2002-2003 Fhg Fokus
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

#ifndef _CONFERENCE_H_
#define _CONFERENCE_H_

#ifdef USE_MYSQL
#include <mysql++/mysql++.h>
#endif

#include "AmApi.h"
#include "AmAudioFile.h"
#include "AmConferenceChannel.h"
#include "AmPlaylist.h"
#include "AmRingTone.h"
#include "AmSession.h"
#include "AmThread.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class ConferenceStatus;
class ConferenceStatusContainer;

enum
{
  CS_normal = 0,
  CS_dialing_out,
  CS_dialed_out,
  CS_dialout_connected
};

enum
{
  DoConfConnect = 100,
  DoConfDisconnect,
  DoConfRinging,
  DoConfError
};

/** \brief Event to trigger connecting/disconnecting between dialout session and
 * main conference session */
struct DialoutConfEvent : public AmEvent
{
  std::string conf_id;

  DialoutConfEvent(int event_id, const std::string& conf_id)
      : AmEvent(event_id)
      , conf_id(conf_id)
  {
  }
};

/** \brief Factory for conference sessions */
class ConferenceFactory : public AmSessionFactory
{
  static AmSessionEventHandlerFactory* session_timer_f;
  static AmConfigReader                cfg;

 public:
  static std::string  AudioPath;
  static std::string  LonelyUserFile;
  static std::string  JoinSound;
  static std::string  DropSound;
  static std::string  DialoutSuffix;
  static PlayoutType  m_PlayoutType;
  static unsigned int MaxParticipants;
  static bool         UseRFC4240Rooms;

  static void setupSessionTimer(AmSession* s);

#ifdef USE_MYSQL
  static mysqlpp::Connection Connection;
#endif

  ConferenceFactory(const std::string& _app_name);
  virtual AmSession*
  onInvite(const AmSipRequest&, const std::string& app_name,
           const std::map<std::string, std::string>& app_params);
  virtual AmSession*
              onRefer(const AmSipRequest& req, const std::string& app_name,
                      const std::map<std::string, std::string>& app_params);
  virtual int onLoad();
};

/** \brief session logic implementation of conference sessions */
class ConferenceDialog : public AmSession
{
  AmPlaylist play_list;

  std::unique_ptr<AmAudioFile> LonelyUserFile;
  std::unique_ptr<AmAudioFile> JoinSound;
  std::unique_ptr<AmAudioFile> DropSound;
  std::unique_ptr<AmRingTone>  RingTone;
  std::unique_ptr<AmRingTone>  ErrorTone;

  std::string                          conf_id;
  std::unique_ptr<AmConferenceChannel> channel;

  int                                  state;
  std::string                          dtmf_seq;
  bool                                 dialedout;
  std::string                          dialout_suffix;
  std::string                          dialout_id;
  std::unique_ptr<AmConferenceChannel> dialout_channel;

  bool allow_dialout;

  std::string from_header;
  std::string extra_headers;
  std::string language;

  bool listen_only;

  std::unique_ptr<AmSipRequest> transfer_req;

  void createDialoutParticipant(const std::string& uri);
  void disconnectDialout();
  void connectMainChannel();
  void closeChannels();
  void setupAudio();

#ifdef WITH_SAS_TTS
  void                      sayTTS(std::string text);
  std::string               last_sas;
  cst_voice*                tts_voice;
  std::vector<AmAudioFile*> TTSFiles;
#endif

 public:
  ConferenceDialog(const std::string&   conf_id,
                   AmConferenceChannel* dialout_channel = 0);

  ~ConferenceDialog();

  void process(AmEvent* ev);
  void onStart();
  void onDtmf(int event, int duration);
  void onInvite(const AmSipRequest& req);
  void onSessionStart();
  void onBye(const AmSipRequest& req);

  void onSipRequest(const AmSipRequest& req);
  void onSipReply(const AmSipRequest& req, const AmSipReply& reply,
                  AmBasicSipDialog::Status old_dlg_status);

#ifdef WITH_SAS_TTS
  void onZRTPEvent(zrtp_event_t event, zrtp_stream_ctx_t* stream_ctx);
#endif
};

#endif
