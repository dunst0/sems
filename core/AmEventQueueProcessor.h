/*
 * Copyright (C) 2012 Frafos GmbH
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

#ifndef _AMEVENTQUEUEPROCESSOR_H_
#define _AMEVENTQUEUEPROCESSOR_H_

#include "AmEventQueue.h"
#include "AmThread.h"

#include <deque>
#include <vector>

class AmEventQueueWorker
    : public AmThread
    , public AmEventNotificationSink
{
  std::deque<AmEventQueue*> process_queues;
  AmMutex                   process_queues_mutex;

 protected:
  // AmThread interface
  void run();
  void on_stop();

 public:
  AmEventQueueWorker()
      : name("EventQueueWorker")
  {
  }
  ~AmEventQueueWorker() {}

  // AmEventNotificationSink interface
  void notify(AmEventQueue* sender);
  void startEventQueue(AmEventQueue* q);
};

/**
 * The event queue processor processes event queues
 * using a pool of workers.
 *
 * Note: the queue's ref-count should be increased
 * before binding the queue to the processor. Also,
 * the ref-count should be decreased at some place
 * during or after finalize(), so that the queue
 * gets disposed correctly.
 */
class AmEventQueueProcessor
{
  typedef std::vector<AmEventQueueWorker*> Workers;

  Workers           threads;
  AmMutex           threads_mutex;
  Workers::iterator threads_it;

 public:
  AmEventQueueProcessor();
  ~AmEventQueueProcessor();

  void addThreads(unsigned int num_threads);

  AmEventQueueWorker* getWorker();
  int                 startEventQueue(AmEventQueue* q);
};

#endif
