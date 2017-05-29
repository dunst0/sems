#ifndef _PARSE_100REL_H_
#define _PARSE_100REL_H_

#include "cstring.h"
#include "parse_header.h"

#include <assert.h>

struct sip_rack : public sip_parsed_hdr
{
  unsigned rseq;
  unsigned cseq;
  cstring  cseq_str;
  int      method;
  cstring  method_str;
};

bool parse_rseq(unsigned* rseq, const char* start, int len);
bool parse_rack(sip_rack* rack, const char* start, int len);

inline static sip_rack* get_rack(const sip_msg* msg)
{
  assert(msg->rack);
  return dynamic_cast<sip_rack*>(msg->rack->p);
}

#endif
