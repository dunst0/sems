#ifndef _PARSE_EXTENSIONS_H_
#define _PARSE_EXTENSIONS_H_

enum
{
  SIP_EXTENSION_100REL = 1 << 0,
#define SIP_EXTENSION_100REL SIP_EXTENSION_100REL
};

bool parse_extensions(unsigned int* extensions, const char* start, int len);

#endif
