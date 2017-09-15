/*
 * Copyright (C) 2017 Rick Barenthin, Julien Arlt - terralink networks GmbH
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

#ifndef _CC_PARALLELCALLSREDIS_H_
#define _CC_PARALLELCALLSREDIS_H_

#include "SBCCallProfile.h"

#include "AmApi.h"

#include <map>

/**
 * call control module limiting parallel number of calls
 */
class CCParallelCallsRedis : public AmDynInvoke
{
  static unsigned int refuse_code;
  static std::string  refuse_reason;
  static std::string  refuse_header;
  static bool         strict;

  static CallCounter* callCounter;

  static CCParallelCallsRedis* _instance;

  void start(const std::string& cc_namespace, const std::string& ltag,
             SBCCallProfile* call_profile, const AmArg& values,
             const AmSipRequest* req, AmArg& res);
  void end(const std::string& cc_namespace, const std::string& ltag,
           SBCCallProfile* call_profile, const AmSipRequest* req);

  CCParallelCallsRedis();
  ~CCParallelCallsRedis();

 public:
  static CCParallelCallsRedis* instance();
  void invoke(const std::string& method, const AmArg& args, AmArg& ret);
  int  onLoad();
};

#endif
