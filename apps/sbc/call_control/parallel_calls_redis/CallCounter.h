/*
 * Copyright (C) 2017 Rick Barenthin, Julien Arlt - terralink networks GmbH
 */

#ifndef _CALLCOUNTER_H_
#define _CALLCOUNTER_H_

#include "AmThread.h"

#include <map>
#include <set>
#include <string>

class CallCounter
{
  bool strict;

  AmMutex call_counter_mutex;
  /* map uuid to call counts */
  std::map<std::string, unsigned int> call_uuid_count;
  /* keep uuid with call-id and from-tag for less strict counting */
  std::set<std::string> know_calls_uuid;

 public:
  CallCounter(bool strict, bool useRedis);
  ~CallCounter();

  bool tryIncrement(std::string uuid, std::string callid, std::string from_tag,
                    unsigned int max_calls);
  void refresh(std::string uuid, std::string callid, std::string from_tag);
  void decrement(std::string uuid, std::string callid, std::string from_tag);
}

#endif
