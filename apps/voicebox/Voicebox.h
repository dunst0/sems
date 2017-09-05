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

#ifndef _VOICEBOX_H_
#define _VOICEBOX_H_

#include "AmApi.h"
#include "AmAudio.h"
#include "AmPlaylist.h"
#include "AmPromptCollection.h"
#include "AmSession.h"
#include "PromptOptions.h"

#include <list>
#include <map>
#include <string>

class VoiceboxFactory : public AmSessionFactory
{
  std::map<std::string, std::map<std::string, AmPromptCollection*>> prompts;
  std::map<std::string, std::map<std::string, PromptOptions>> prompt_options;

  AmPromptCollection* getPrompts(const std::string& domain,
                                 const std::string& language,
                                 PromptOptions&     po);

  /** finds prompts from specific to default prompts */
  AmPromptCollection* findPrompts(const std::string& domain,
                                  const std::string& language,
                                  PromptOptions&     po);

  AmPromptCollection* loadPrompts(std::string prompt_base_path,
                                  std::string domain, std::string language,
                                  bool load_digits);
  static AmDynInvokeFactory* MessageStorage;
  static unsigned int        repeat_key;
  static unsigned int        save_key;
  static unsigned int        delete_key;
  static unsigned int        startover_key;
  static std::string         default_language;
  static bool                SimpleMode;

 public:
  VoiceboxFactory(const std::string& _app_name);

  AmSession* onInvite(const AmSipRequest&, const std::string&   app_name,
                      const std::map<std::string, std::string>& app_params);
  // AmSession* onInvite(const AmSipRequest& req, AmArg& session_params);
  int onLoad();

  friend class VoiceboxDialog;
};

#endif
