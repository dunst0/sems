#ifndef _AMSIPMSG_H_
#define _AMSIPMSG_H_

#include "AmArg.h"
#include "AmMimeBody.h"
#include "sip/trans_layer.h"

#include <list>
#include <string>

/* enforce common naming in Req&Rpl */
class _AmSipMsgInDlg : public AmObject
{
 public:
  std::string from;
  std::string from_tag;

  std::string to;
  std::string to_tag;

  std::string callid;

  unsigned int cseq;
  std::string  cseq_method;

  unsigned int rseq;

  std::string route;
  std::string contact;

  std::string hdrs;

  AmMimeBody body;

  // transaction ticket from sip stack
  trans_ticket tt;

  std::string        remote_ip;
  unsigned short int remote_port;
  std::string        local_ip;
  unsigned short int local_port;
  std::string        trsp;

  _AmSipMsgInDlg()
      : cseq(0)
      , rseq(0)
  {
  }
  virtual ~_AmSipMsgInDlg(){};

  virtual std::string print() const = 0;
};

#ifdef PROPAGATE_UNPARSED_REPLY_HEADERS

struct AmSipHeader
{
  std::string name, value;
  AmSipHeader() {}
  AmSipHeader(const std::string& _name, const std::string& _value)
      : name(_name)
      , value(_value)
  {
  }
  AmSipHeader(const cstring& _name, const cstring& _value)
      : name(_name.s, _name.len)
      , value(_value.s, _value.len)
  {
  }
};

#endif

/** \brief represents a SIP reply */
class AmSipReply : public _AmSipMsgInDlg
{
 public:
  unsigned int code;
  std::string  reason;
  std::string  to_uri;
#ifdef PROPAGATE_UNPARSED_REPLY_HEADERS
  std::list<AmSipHeader> unparsed_headers;
#endif

  AmSipReply()
      : _AmSipMsgInDlg()
      , code(0)
  {
  }
  ~AmSipReply() {}
  std::string print() const;
};

/** \brief represents a SIP request */
class AmSipRequest : public _AmSipMsgInDlg
{
 public:
  std::string method;

  std::string user;
  std::string domain;
  std::string r_uri;
  std::string from_uri;

  std::string  rack_method;
  unsigned int rack_cseq;

  std::string vias;
  std::string via1;
  std::string via_branch;
  bool        first_hop;

  int max_forwards;

  unsigned short local_if;

  AmSipRequest();
  ~AmSipRequest() {}

  std::string print() const;
  void log(msg_logger* logger) const;
};

std::string getHeader(const std::string& hdrs, const std::string& hdr_name,
                      bool single = false);

std::string getHeader(const std::string& hdrs, const std::string& hdr_name,
                      const std::string& compact_hdr_name, bool single = false);

/** find a header, starting from char skip
    if found, value is between pos1 and pos2
    and hdr start is the start of the header
    @return true if found */
bool findHeader(const std::string& hdrs, const std::string& hdr_name,
                const size_t skip, size_t& pos1, size_t& pos2,
                size_t& hdr_start);

/** @return whether header hdr_name is in hdrs */
bool hasHeader(const std::string& hdrs, const std::string& hdr_name);

bool removeHeader(std::string& hdrs, const std::string& hdr_name);

/** add an option tag @param tag to list @param hdr_name */
void addOptionTag(std::string& hdrs, const std::string& hdr_name,
                  const std::string& tag);

/** remove an option tag @param tag from list @param hdr_name */
void removeOptionTag(std::string& hdrs, const std::string& hdr_name,
                     const std::string& tag);

#endif
