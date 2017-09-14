/*
 * Copyright (C) 2007 Sipwise GmbH
 * Based on the concept of "announcement", Copyright (C) 2002-2003 Fhg Fokus
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

#ifndef _CLICK2DIAL_H_
#define _CLICK2DIAL_H_

#include "AmAudioFile.h"
#include "AmB2BSession.h"
#include "AmConfigReader.h"
#include "AmSession.h"
#include "AmUACAuth.h"

#include <map>
#include <string>

class Click2DialFactory : public AmSessionFactory
{
  std::string getAnnounceFile(const AmSipRequest& req);

 public:
  static std::string            AnnouncePath;
  static std::string            AnnounceFile;
  static AmSessionEventHandler* AuthHandler;

  static bool relay_early_media_sdp;

  Click2DialFactory(const std::string& _app_name);

  int        onLoad();
  AmSession* onInvite(const AmSipRequest& req, const std::string& app_name,
                      const std::map<std::string, std::string>& app_params);
  AmSession* onInvite(const AmSipRequest& req, const std::string& app_name,
                      AmArg& session_params);
};

class C2DCallerDialog
    : public AmB2BCallerSession
    , public CredentialHolder
{
  AmAudioFile                  wav_file;
  std::string                  filename;
  std::string                  callee_uri;
  std::unique_ptr<UACAuthCred> cred;

 public:
  C2DCallerDialog(const AmSipRequest& req, const std::string& filename,
                  const std::string& callee_uri,
                  UACAuthCred*       credentials = NULL);

  void                process(AmEvent* event);
  void                onInvite(const AmSipRequest& req);
  void                onInvite2xx(const AmSipReply& reply);
  void                onSessionStart();
  void                createCalleeSession();
  inline UACAuthCred* getCredentials() { return cred.get(); }
  void                onB2BEvent(B2BEvent*);
  void updateUACTransCSeq(unsigned int old_cseq, unsigned int new_cseq);
  void onSipReply(const AmSipRequest& req, const AmSipReply& reply,
                  AmBasicSipDialog::Status old_dlg_status);
};

class C2DCalleeDialog
    : public AmB2BCalleeSession
    , public CredentialHolder
{
  std::unique_ptr<UACAuthCred> cred;
  void                         setAuthHandler();

 public:
  C2DCalleeDialog(const AmB2BCallerSession* caller,
                  UACAuthCred*              credentials = NULL);
  inline UACAuthCred* getCredentials() { return cred.get(); }
};

#endif
