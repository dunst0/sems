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

#ifndef _CC_TEMPLATE_H_
#define _CC_TEMPLATE_H_

#include "ExtendedCCInterface.h"
#include "SBCCallProfile.h"

#include "AmApi.h"

/**
 * sample call control module
 */
class CCTemplate
    : public AmDynInvoke
#ifdef CC_EXTENDED_INTERFACE
    , public AmObject
    , public ExtendedCCInterface
#endif
{
  static CCTemplate* _instance;

  void start(const std::string& cc_name, const std::string& ltag,
             SBCCallProfile* call_profile, int start_ts_sec, int start_ts_usec,
             const AmArg& values, int timer_id, AmArg& res);
  void connect(const std::string& cc_name, const std::string& ltag,
               SBCCallProfile* call_profile, const std::string& other_ltag,
               int connect_ts_sec, int connect_ts_usec);
  void end(const std::string& cc_name, const std::string& ltag,
           SBCCallProfile* call_profile, int end_ts_sec, int end_ts_usec);

 public:
  CCTemplate();
  ~CCTemplate();
  static CCTemplate* instance();
  void invoke(const std::string& method, const AmArg& args, AmArg& ret);
  int onLoad();

#ifdef CC_EXTENDED_INTERFACE
  // extended call control interface
#ifdef CC_EXTENDED_INTERFACE_CALLS
  //    --- calls
  bool init(SBCCallLeg* call, const std::map<std::string, std::string>& values);
  CCChainProcessing onInitialInvite(SBCCallLeg*                  call,
                                    *InitialInviteHandlerParams& params);
  CCChainProcessing onBLegRefused(SBCCallLeg* call, const AmSipReply& reply);

  void onDestroyLeg(SBCCallLeg* call);
  void onStateChange(SBCCallLeg*                        call,
                     const CallLeg::StatusChangeCause*& cause);
  CCChainProcessing onInDialogRequest(SBCCallLeg*          call,
                                      const AmSipRequest*& req);
  CCChainProcessing onInDialogReply(SBCCallLeg* call, const AmSipReply*& reply);
  CCChainProcessing onEvent(SBCCallLeg* call, AmEvent* e);
  CCChainProcessing onDtmf(SBCCallLeg* call, int event, int duration);
  CCChainProcessing putOnHold(SBCCallLeg* call);
  CCChainProcessing resumeHeld(SBCCallLeg* call, bool send_reinvite);
  CCChainProcessing createHoldRequest(SBCCallLeg* call, AmSdp& sdp);
  CCChainProcessing handleHoldReply(SBCCallLeg* call, bool succeeded);
  int relayEvent(SBCCallLeg* call, AmEvent* e);
#else
  //    --- simple relay
  bool init(SBCCallProfile& profile, SimpleRelayDialog* relay,
            void**& user_data);
  void initUAC(const AmSipRequest& req, void* user_data);
  void initUAS(const AmSipRequest& req, void* user_data);
  void finalize(void* user_data);
  void onSipRequest(const AmSipRequest& req, void* user_data);
  void onSipReply(const AmSipRequest& req, const AmSipReply& reply,
                  AmBasicSipDialog::Status old_dlg_status, void* user_data);
  void onB2BRequest(const AmSipRequest& req, void* user_data);
  void onB2BReply(const AmSipReply& reply, void* user_data);
#endif
#endif
};

#endif
