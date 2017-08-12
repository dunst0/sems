/*
 *  (c) 2007 iptego GmbH
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

#include "AmCallWatcher.h"
#include "log.h"

#include <sys/time.h>
#include <unistd.h>

using std::string;
using std::make_pair;

AmCallStatus::AmCallStatus() {}

AmCallStatus::~AmCallStatus() {}

void AmCallStatus::dump() {}

AmCallStatusUpdateEvent::AmCallStatusUpdateEvent(UpdateType    t,
                                                 const string& call_id)
    : AmEvent(t)
    , call_id(call_id)
{
}

AmCallStatusUpdateEvent::AmCallStatusUpdateEvent(const string& call_id,
                                                 AmCallStatus* init_status)
    : AmEvent(Initialize)
    , call_id(call_id)
    , init_status(init_status)
{
}

AmCallStatusUpdateEvent::~AmCallStatusUpdateEvent() {}

string AmCallStatusUpdateEvent::get_call_id() { return call_id; }

AmCallStatus* AmCallStatusUpdateEvent::get_init_status() { return init_status; }

AmCallWatcherGarbageCollector::AmCallWatcherGarbageCollector(
    CallStatusTimedMap& garbage, AmMutex& mutex)
    : garbage(garbage)
    , garbage_mutex(mutex)
{
}

void AmCallWatcherGarbageCollector::on_stop() {}

void AmCallWatcherGarbageCollector::run()
{
  DBG("AmCallWatcherGarbageCollector started.\n");

  while (isRunning()) {
    sleep(2);

    struct timeval now;
    gettimeofday(&now, NULL);

    bool erased = false;

    garbage_mutex.lock();

    CallStatusTimedMap::iterator it = garbage.begin();
    while (it != garbage.end()) {
      if (it->second.second < now.tv_sec) {
        CallStatusTimedMap::iterator d_it = it;
        it++;
        delete (d_it->second.first);
        garbage.erase(d_it);
        erased = true;
      }
      else {
        it++;
      }
    }

    if (erased) {
      DBG("cleared old soft-states (%u soft-states remaining)\n",
          (unsigned int) garbage.size());
    }

    garbage_mutex.unlock();
  }
}

AmCallWatcher::AmCallWatcher()
    : AmEventQueue(this)
{
  garbage_collector =
      new AmCallWatcherGarbageCollector(soft_states, soft_states_mutex);
}

AmCallWatcher::~AmCallWatcher() {}

void AmCallWatcher::run()
{
  DBG("starting call watcher.\n");

  garbage_collector->start();

  while (isRunning()) {
    waitForEvent();
    processEvents();
  }
}

void AmCallWatcher::on_stop() {}

void AmCallWatcher::process(AmEvent* ev)
{
  AmCallStatusUpdateEvent* csu = dynamic_cast<AmCallStatusUpdateEvent*>(ev);
  if (NULL == csu) {
    ERROR("received invalid event!\n");
    return;
  }

  switch (csu->event_id) {
    case AmCallStatusUpdateEvent::Initialize: {
      states_mutex.lock();
      DBG("adding  call state '%s'\n", csu->get_call_id().c_str());

      // check whether already there
      CallStatusMap::iterator it = states.find(csu->get_call_id());
      if (it != states.end()) {
        WARN("implementation error: state '%s' already in list!\n",
             csu->get_call_id().c_str());
        // avoid leak - delete the old state
        delete it->second;
      }

      // insert the new one
      states[csu->get_call_id()] = csu->get_init_status();
      states_mutex.unlock();
    } break;

    case AmCallStatusUpdateEvent::Update: {
      states_mutex.lock();
      CallStatusMap::iterator it = states.find(csu->get_call_id());
      if (it != states.end()) {
        it->second->update(csu);
        it->second->dump();
        states_mutex.unlock();
      }
      else {
        states_mutex.unlock();

        soft_states_mutex.lock();
        CallStatusTimedMap::iterator it = soft_states.find(csu->get_call_id());
        if (it != soft_states.end()) {
          it->second.first->update(csu);
          it->second.first->dump();
        }
        else {
          DBG("received update event for inexistent call '%s'\n",
              csu->get_call_id().c_str());
        }
        soft_states_mutex.unlock();
      }
    } break;

    case AmCallStatusUpdateEvent::Obsolete: {
      states_mutex.lock();
      CallStatusMap::iterator it = states.find(csu->get_call_id());
      if (it != states.end()) {
        AmCallStatus* cs = it->second;
        states.erase(it);
        size_t s_size = states.size();
        states_mutex.unlock();

        struct timeval now;
        gettimeofday(&now, NULL);

        soft_states_mutex.lock();
        soft_states[csu->get_call_id()] =
            make_pair(cs, now.tv_sec + WATCHER_SOFT_EXPIRE_SECONDS);
        size_t soft_size = soft_states.size();
        soft_states_mutex.unlock();

        DBG("moved call state '%s' to soft-state map (%u states, %u "
            "soft-states)\n",
            csu->get_call_id().c_str(), (unsigned int) s_size,
            (unsigned int) soft_size);
      }
      else {
        DBG("received obsolete event for inexistent call '%s'\n",
            csu->get_call_id().c_str());
        states_mutex.unlock();
      }
    } break;
  }
}

void AmCallWatcher::dump()
{
  states_mutex.lock();
  for (CallStatusMap::iterator it = states.begin(); it != states.end(); it++) {
    it->second->dump();
  }
  states_mutex.unlock();
}

AmCallStatus* AmCallWatcher::getStatus(const string& call_id)
{
  AmCallStatus* res = NULL;

  states_mutex.lock();

  CallStatusMap::iterator it = states.find(call_id);
  if (it != states.end()) {
    res = it->second->copy();
    states_mutex.unlock();
  }
  else {
    states_mutex.unlock();

    // check obsolete states
    soft_states_mutex.lock();
    CallStatusTimedMap::iterator it = soft_states.find(call_id);
    if (it != soft_states.end()) {
      // got it. return and remove from map
      res = it->second.first;
      soft_states.erase(it);
      DBG("erased call state '%s' (%u in list).\n", call_id.c_str(),
          (unsigned int) soft_states.size());
    }
    else {
      DBG("state for call '%s' not found.\n", call_id.c_str());
    }
    soft_states_mutex.unlock();
  }

  return res;
}
