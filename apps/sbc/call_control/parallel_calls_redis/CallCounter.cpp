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
      uuid_call_count.find(uuid);

  if (call_count_it == uuid_call_count.end()) {
    DBG("no call counter for uuid '%s'\n", uuid.c_str());
    uuid_call_count[uuid] = 0;
  }

  string call_key = uuid + "-" + callid + "-" + from_tag;
  DBG("building call key <%s> for uuid '%s'\n", call_key.c_str(), uuid.c_str());

  if (strict || uuid_known_calls.count(call_key) == 0) {
    // TODO check redis

    if (uuid_call_count[uuid] == max_calls) {
      DBG("uuid '%s' has already reached max calls %u reject call\n",
          uuid.c_str(), max_calls);

      call_counter_mutex.unlock();
      return false;
    }

    // incr redis
    ++uuid_call_count[uuid];
    DBG("uuid '%s' has now %u active calls\n", uuid.c_str(),
        uuid_call_count[uuid]);

    DBG("insert call key <%s> into known call list\n", call_key.c_str());
    uuid_known_calls[call_key] = 1;
  }
  else {
    DBG("known call key <%s> into known call list\n", call_key.c_str());
    ++uuid_known_calls[call_key];
  }

  DBG("uuid '%s' has now %u active calls with %u branches\n", uuid.c_str(),
      uuid_call_count[uuid], uuid_known_calls[call_key]);

  call_counter_mutex.unlock();
  return true;
}

void CallCounter::refresh(string uuid, string callid, string from_tag) {}

bool CallCounter::decrement(string uuid, string callid, string from_tag)
{
  call_counter_mutex.lock();
  string call_key = uuid + "-" + callid + "-" + from_tag;

  if (uuid_known_calls[call_key] > 1) {
    --uuid_known_calls[call_key];
    DBG("uuid '%s' now has %u active calls, with %u branches\n", uuid.c_str(),
        uuid_call_count[uuid], uuid_known_calls[call_key]);

    call_counter_mutex.unlock();
    return false;
  }
  else {
    uuid_known_calls.erase(call_key);
    DBG("uuid '%s' now has %u active calls, with 0 branches, deleting branche "
        "counter\n",
        uuid.c_str(), uuid_call_count[uuid]);
  }

  if (uuid_call_count[uuid] > 1) {
    --uuid_call_count[uuid];
    DBG("uuid '%s' now has %u active calls\n", uuid.c_str(),
        uuid_call_count[uuid]);
  }
  else {
    uuid_call_count.erase(uuid);
    DBG("uuid '%s' now has 0 active calls, deleting call counter\n",
        uuid.c_str());
  }
  call_counter_mutex.unlock();

  return true;
}
