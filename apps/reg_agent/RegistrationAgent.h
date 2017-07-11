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

#ifndef _REGISTRATIONAGENT_H_
#define _REGISTRATIONAGENT_H_

#include "AmConfigReader.h"
#include "AmSession.h"

#include <string>
#include <vector>

struct RegInfo
{
  std::string domain;
  std::string user;
  std::string display_name;
  std::string auth_user;
  std::string passwd;
  std::string proxy;
  std::string contact;

  std::string handle;
};

class RegThread : public AmThread
{
  vector<RegInfo> registrations;

  void create_registration(RegInfo& ri);
  bool check_registration(const RegInfo& ri);

 protected:
  void run();
  void on_stop();

 public:
  void add_reg(const RegInfo& ri);
  void postEvent(AmEvent* ev);
};

class RegistrationAgentFactory : public AmSessionFactory
{
  RegThread                     dialer;
  AmSessionEventHandlerFactory* uac_auth_f;

 public:
  RegistrationAgentFactory(const std::string& _app_name);

  int        onLoad();
  AmSession* onInvite(const AmSipRequest& req, const std::string& app_name,
                      const std::map<std::string, std::string>&   app_params);
  void postEvent(AmEvent* ev);
};

#endif
