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

#include "AmEventQueueProcessor.h"
#include "AmEventQueue.h"

#include <deque>

using std::deque;

AmEventQueueProcessor::AmEventQueueProcessor() { threads_it = threads.begin(); }

AmEventQueueProcessor::~AmEventQueueProcessor()
{
  threads_mutex.lock();

  threads_it = threads.begin();
  while (threads_it != threads.end()) {
    (*threads_it)->stop();
    threads_it++;
  }

  threads_it = threads.begin();
  while (threads_it != threads.end()) {
    (*threads_it)->join();
    delete (*threads_it);
    threads_it++;
  }

  threads_mutex.unlock();
}

AmEventQueueWorker* AmEventQueueProcessor::getWorker()
{
  threads_mutex.lock();

  if (!threads.size()) {
    ERROR("requesting EventQueue processing thread but none available\n");
    threads_mutex.unlock();
    return NULL;
  }

  // round robin
  if (threads_it == threads.end()) threads_it = threads.begin();

  AmEventQueueWorker* res = *threads_it;
  threads_it++;

  threads_mutex.unlock();

  return res;
}

int AmEventQueueProcessor::startEventQueue(AmEventQueue* q)
{
  AmEventQueueWorker* worker = getWorker();
  if (!worker) return -1;

  worker->startEventQueue(q);
  return 0;
}

void AmEventQueueProcessor::addThreads(unsigned int num_threads)
{
  DBG("starting %u session processor threads\n", num_threads);

  threads_mutex.lock();

  for (unsigned int i = 0; i < num_threads; i++) {
    AmEventQueueWorker* worker = new AmEventQueueWorker();
    threads.push_back(worker);
    worker->start();
  }

  threads_it = threads.begin();
  DBG("now %zd session processor threads running\n", threads.size());
  threads_mutex.unlock();
}

void AmEventQueueWorker::notify(AmEventQueue* sender)
{
  process_queues_mutex.lock();

  process_queues.push_back(sender);
  inc_ref(sender);
  getRunCondition().set(true);

  process_queues_mutex.unlock();
}

void AmEventQueueWorker::run()
{
  DBG("EventQueueWorker (%s_%lu) is start running.\n", thread_name.c_str(),
      getPid());

  while (isRunning()) {
    getRunCondition().wait_for();

    if (!isRunning()) {
      break;
    }

    DBG("EventQueueWorker (%s_%lu) is running processing loop\n",
        thread_name.c_str(), getPid());

    process_queues_mutex.lock();
    while (!process_queues.empty()) {
      AmEventQueue* ev_q = process_queues.front();
      process_queues.pop_front();
      process_queues_mutex.unlock();

      if (!ev_q->processingCycle()) {
        ev_q->setEventNotificationSink(NULL);

        if (!ev_q->isFinalized()) {
          ev_q->finalize();
        }
      }

      dec_ref(ev_q);

      process_queues_mutex.lock();
    }

    getRunCondition().set(false);
    process_queues_mutex.unlock();
  }

  DBG("EventQueueWorker (%s_%lu) has stopped running.\n", thread_name.c_str(),
      getPid());
}

void AmEventQueueWorker::on_stop()
{
  DBG("EventQueueWorker (%s_%lu) requesting to stop worker.\n",
      thread_name.c_str(), getPid());
}

void AmEventQueueWorker::startEventQueue(AmEventQueue* q)
{
  if (q->startup()) {
    // register us to be notified if some event comes to the session
    q->setEventNotificationSink(this);
  }
}
