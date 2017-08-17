/*
 * Copyright (C) 2006 iptego GmbH
 *
 * This file is part of SEMS, a free SIP media server.
 *
 * SEMS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. This program is released under
 * the GPL with the additional exemption that compiling, linking,
 * and/or using OpenSSL is allowed.
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

#ifndef _AMURIPARSER_H_
#define _AMURIPARSER_H_

#include <map>
#include <string>

struct AmUriParser
{
  std::string display_name;
  std::string uri;

  std::string uri_user;
  std::string uri_host;
  std::string uri_port;
  std::string uri_headers;
  std::string uri_param; // <sip:user@host;uri_param>
                         // <sip:user;user_param@host>

  std::map<std::string, std::string> params; // <sip:user;@host>;params

  bool isEqual(const AmUriParser& c) const;
  /** parse nameaddr from pos
       @return true on success
       @return end of current nameaddr */
  bool parse_contact(const std::string& line, size_t pos, size_t& end);
  /** parse a name-addr @return true on success */
  bool parse_nameaddr(const std::string& line);

  /** @return true on success */
  bool parse_uri();
  bool parse_params(const std::string& line, int& pos);

  /** param_string is semicolon separated list of parameters with or without
   * value. method can be used to add/replace param for uri and user
   * parameters
   */
  static std::string add_param_to_param_list(const std::string& param_name,
                                             const std::string& param_value,
                                             const std::string& param_list);

  void        dump() const;
  std::string uri_str() const;
  std::string canon_uri_str() const;
  std::string nameaddr_str() const;

  AmUriParser() {}

  std::string print() const;
};

#endif
