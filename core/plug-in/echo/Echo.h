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

#ifndef _ECHO_H_
#define _ECHO_H_

#include "AmAudioEcho.h"
#include "AmSession.h"

#include <map>
#include <string>

#define MODULE_NAME "echo"

/** \brief Factory for echo sessions */
class EchoFactory : public AmSessionFactory
{
  AmSessionEventHandlerFactory* session_timer_f;
  AmConfigReader                conf;

 public:
  EchoFactory(const std::string& _app_name);
  virtual int onLoad();
  virtual AmSession*
  onInvite(const AmSipRequest& req, const std::string& app_name,
           const std::map<std::string, std::string>&   app_params);
};

/** \brief echo session logic implementation */
class EchoDialog : public AmSession
{
  AmAudioEcho echo;
  PlayoutType playout_type;

 public:
  EchoDialog();
  ~EchoDialog();

  void onSessionStart();
  // UNUSED
  // void onSessionStart(const AmSipRequest& req);
  // void onSessionStart(const AmSipReply& rep);
  // UNUSED_END

  void onBye(const AmSipRequest& req);
  void onDtmf(int event, int duration);
};

#endif
