/*
 * Copyright (C) 2017 Rick Barenthin, Julien Arlt - terralink networks GmbH
 */

#include "CallCounter.h"

#include "log.h"

using std::string;
using std::map;

CallCounter::CallCounter(bool strict, bool useRedis)
    : strict(strict)
{
}

CallCounter::~CallCounter() {}

bool CallCounter::tryIncrement(string uuid, string callid, string from_tag,
                               unsigned int max_calls)
{
  call_counter_mutex.lock();
  map<string, unsigned int>::iterator call_count_it =
      call_uuid_count.find(uuid);

  if (call_count_it == call_uuid_count.end()) {
    DBG("no call counter for uuid '%s'\n", uuid.c_str());
    call_uuid_count[uuid] = 0;
  }

  string call_key = uuid + "-" + callid + "-" + from_tag;
  DBG("building call key <%s> for uuid '%s'\n", call_key.c_str(), uuid.c_str());

  if (strict || known_calls_uuid.count(call_key) == 0) {
    // TODO check redis

    if (call_uuid_count[uuid] == max_calls) {
      DBG("uuid '%s' has already reached max calls %u reject call\n",
          uuid.c_str(), max_calls);

      call_counter_mutex.unlock();
      return false;
    }

    // incr redis
    ++call_uuid_count[uuid];
    DBG("uuid '%s' has now %u active calls\n", uuid.c_str(),
        call_uuid_count[uuid]);

    DBG("insert call key <%s> into known call list\n", call_key.c_str());
    known_calls_uuid.insert(call_key);
  }
  else {
    DBG("known call key <%s> into known call list\n", call_key.c_str());
  }

  call_counter_mutex.unlock();
  return true;
}

void CallCounter::refresh(string uuid, string callid, string from_tag) {}

void CallCounter::decrement(string uuid, string callid, string from_tag)
{
  call_counter_mutex.lock();
  if (call_uuid_count[uuid] > 1) {
    --call_uuid_count[uuid];
    DBG("uuid '%s' now has %u active calls\n", uuid.c_str(),
        call_uuid_count[uuid]);
  }
  else {
    call_uuid_count.erase(uuid);
    DBG("uuid '%s' now has 0 active calls, deleting call counter\n",
        uuid.c_str());
  }
  call_counter_mutex.unlock();
}
