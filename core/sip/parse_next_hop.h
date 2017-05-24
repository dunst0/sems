#ifndef _PARSE_NEXT_HOP_H_
#define _PARSE_NEXT_HOP_H_

#include "cstring.h"

#include <list>

using std::list;

struct sip_destination
{
  cstring        host;
  unsigned short port;
  cstring        trsp;

  sip_destination()
      : host()
      , port(0)
      , trsp()
  {
  }
};

int parse_next_hop(const cstring& next_hop, list<sip_destination>& dest_list);

#endif
