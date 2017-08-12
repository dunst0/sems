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
/** @file AmCallWatcher.h */

#ifndef _AMCALLWATCHER_H_
#define _AMCALLWATCHER_H_

//
// States are put into map on an Initialize event.
// States are held in a map identified by call_id
// (opaque identifier) and updated with Update events.
// Once an Obsolete event is received, the states are
// moved to soft-state map, where they are held until
// queried to a maximum of WATCHER_SOFT_EXPIRE_SECONDS

#define WATCHER_SOFT_EXPIRE_SECONDS 5

#include "AmEvent.h"
#include "AmEventQueue.h"
#include "AmThread.h"

#include <map>
#include <string>
#include <utility>

class AmCallStatus;
class AmCallStatusUpdateEvent;

typedef std::map<std::string, AmCallStatus*> CallStatusMap;
typedef std::map<std::string, std::pair<AmCallStatus*, time_t>>
    CallStatusTimedMap;

/**
 * \brief interface for an update-able call status (AmCallWatcher)
 */
class AmCallStatus
{
 public:
  AmCallStatus();
  virtual ~AmCallStatus();

  /** update from an event */
  virtual void update(AmCallStatusUpdateEvent* e) = 0;

  /** get a copy of self with relevant data */
  virtual AmCallStatus* copy() = 0;
  virtual void          dump();
};

/**
 * \brief event that carries out call status update
 */
class AmCallStatusUpdateEvent : public AmEvent
{
  std::string call_id;

  AmCallStatus* init_status;

 public:
  enum UpdateType
  {
    Initialize = 0,
    Update,
    Obsolete
  };

  AmCallStatusUpdateEvent(UpdateType t, const std::string& call_id);
  AmCallStatusUpdateEvent(const std::string& call_id,
                          AmCallStatus*      init_status);

  ~AmCallStatusUpdateEvent();

  std::string   get_call_id();
  AmCallStatus* get_init_status();
};

/**
 * \brief garbage collector for the AmCallWatcher
 *
 * checks garbage every two seconds.
 * A bit inefficient with two threads, but AmCallWatcher
 * shouldn't be blocked by event.
 */
class AmCallWatcherGarbageCollector : public AmThread
{
  CallStatusTimedMap& garbage;
  AmMutex&            garbage_mutex;

 protected:
  void run();
  void on_stop();

 public:
  AmCallWatcherGarbageCollector(CallStatusTimedMap& garbage, AmMutex& mutex);
};

/**
 * \brief manages call status to be queried by external processes
 * call watcher is an entity for managing call status
 * via events that change status. Events are executed in a
 * separate thread serially by processing the event queue,
 * so synchronous status queries do not block the thread
 * reporting the status change.
 */
class AmCallWatcher
    : public AmThread
    , public AmEventQueue
    , public AmEventHandler
{
 private:
  CallStatusMap states;
  AmMutex       states_mutex;

  CallStatusTimedMap             soft_states;
  AmMutex                        soft_states_mutex;
  AmCallWatcherGarbageCollector* garbage_collector;

 protected:
  void run();
  void on_stop();

 public:
  AmCallWatcher();
  ~AmCallWatcher();

  // eventhandler
  void process(AmEvent*);

  AmCallStatus* getStatus(const std::string& call_id);

  // dump all states
  void dump();
};

#endif
