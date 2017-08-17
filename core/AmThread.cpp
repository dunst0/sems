/*
 * Copyright (C) 2002-2003 Fhg Fokus
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

#include "AmThread.h"

#include "log.h"

#include "errno.h"
#include <string>
#include <unistd.h>

using std::string;
using std::queue;

AmMutex::AmMutex(bool recursive)
{
  if (recursive) {
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m, &attr);
  }
  else {
    pthread_mutex_init(&m, NULL);
  }
}

AmMutex::~AmMutex() { pthread_mutex_destroy(&m); }

void AmMutex::lock() { pthread_mutex_lock(&m); }

void AmMutex::unlock() { pthread_mutex_unlock(&m); }

AmThread::AmThread()
    : running(false)
    , run_condition(false)
    , thread_name("thread")
{
}

void* AmThread::threadStart(void* self)
{
  AmThread* _this = (AmThread*) self;
  _this->pid      = (unsigned long int) _this->thread_id;

  DBG("Thread (%s_%lu) is starting.\n", _this->name, _this->pid);
  _this->run();
  DBG("Thread (%s_%lu) is ending.\n", _this->name, _this->pid);

  return NULL;
}

AmCondition<bool>& AmThread::getRunCondition() { return run_condition; }

void AmThread::start()
{
  pthread_attr_t attr;
  int            res;

  running.lock();
  if (running.unsafe_get()) {
    running.unlock();
    ERROR("Thread (%s_%lu) is already running\n", name, pid);
    return;
  }
  running.unsafe_set(true);
  running.unlock();

  pid = 0;

  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 1024 * 1024); // 1 MB
  res = pthread_create(&thread_id, &attr, threadStart, this);
  pthread_attr_destroy(&attr);

  if (res == 0) {
    DBG("Thread (%s_%lu) is just created.\n", name,
        (unsigned long int) thread_id);
  }
  else {
    if (res == EAGAIN) {
      ERROR("The system lacked the necessary resources to create another "
            "thread");
    }
    else if (res == EINVAL) {
      ERROR("The value specified by attr is invalid");
    }
    else if (res == EPERM) {
      ERROR("The caller does not have appropriate permission");
    }
    else {
      ERROR("pthread create failed with code %i\n", res);
    }

    throw string("Thread could not be started");
  }

  if (res != 0) {
    ERROR("pthread create failed with code %i\n", res);
  }
}

void AmThread::stop()
{
  running.lock();
  if (!running.unsafe_get()) {
    running.unlock();
    return;
  }

  DBG("Thread (%s_%lu) calling on_stop, give it a chance to clean up.\n", name,
      pid);

  try {
    on_stop();
  }
  catch (...) {
    // on purpose the thread is stopping anyway
  }

  running.unsafe_set(false);
  run_condition.set(true);
  running.unlock();
}

void AmThread::cancel()
{
  if (!running.get()) {
    return;
  }

  thread_mutex.lock();

  int res = pthread_cancel(thread_id);

  if (res == 0) {
    DBG("Thread (%s_%lu) is canceled.\n", name, pid);
    running.set(false);
  }
  else if (res == ESRCH) {
    ERROR("No corresponding thread for thread id could be found");
  }
  else {
    ERROR("pthread_cancel failed with code %i\n", res);
  }

  thread_mutex.unlock();
}

void AmThread::detach()
{
  if (!running.get()) {
    return;
  }

  thread_mutex.lock();
  int res = pthread_detach(thread_id);

  if (res == 0) {
    DBG("Thread (%s_%lu) finished detach.\n", name, pid);
  }
  else if (res == EINVAL) {
    WARN("The thread id does not refer to a joinable thread\n");
  }
  else if (res == ESRCH) {
    WARN("No corresponding thread for thread id could be found\n");
  }
  else {
    WARN("pthread_detach failed with code %i\n", res);
  }

  thread_mutex.unlock();
}

void AmThread::join()
{
  if (!running.get()) {
    return;
  }

  thread_mutex.lock();
  int res = pthread_join(thread_id, NULL);

  if (res == 0) {
    DBG("Thread (%s_%lu) successfull joined.\n", name, pid);
  }
  else if (res == EINVAL) {
    WARN("The thread id does not refer to a joinable thread\n");
  }
  else if (res == ESRCH) {
    WARN("No corresponding thread for thread id could be found\n");
  }
  else {
    WARN("pthread_join failed with code %i\n", res);
  }

  thread_mutex.unlock();
}

bool AmThread::isRunning() { return running.get(); }

unsigned long int AmThread::getPid() { return pid; }

int AmThread::setRealtime()
{
  // set process realtime
  //     int policy;
  //     struct sched_param rt_param;
  //     memset (&rt_param, 0, sizeof (rt_param));
  //     rt_param.sched_priority = 80;
  //     int res = sched_setscheduler(0, SCHED_FIFO, &rt_param);
  //     if (res) {
  // 	ERROR("sched_setscheduler failed. Try to run SEMS as root or suid.\n");
  //     }

  //     policy = sched_getscheduler(0);

  //     std::string str_policy = "unknown";
  //     switch(policy) {
  // 	case SCHED_OTHER: str_policy = "SCHED_OTHER"; break;
  // 	case SCHED_RR: str_policy = "SCHED_RR"; break;
  // 	case SCHED_FIFO: str_policy = "SCHED_FIFO"; break;
  //     }

  //     DBG("Thread has now policy '%s' - priority 80 (from %d to %d).\n",
  //     str_policy.c_str(),
  // 	sched_get_priority_min(policy), sched_get_priority_max(policy));
  //     return 0;
  return 0;
}

AmThreadWatcher* AmThreadWatcher::_instance = NULL;
AmMutex          AmThreadWatcher::_instance_mutex;

AmThreadWatcher* AmThreadWatcher::instance()
{
  _instance_mutex.lock();
  if (!_instance) {
    _instance = new AmThreadWatcher();
    _instance->start();
  }

  _instance_mutex.unlock();
  return _instance;
}

void AmThreadWatcher::add(AmThread* thread)
{
  DBG("trying to add thread %lu to thread watcher.\n", thread->getPid());

  thread_queue_mutex.lock();
  thread_queue.push(thread);
  getRunCondition().set(true);
  thread_queue_mutex.unlock();

  DBG("added thread %lu to thread watcher.\n", thread->getPid());
}

void AmThreadWatcher::run()
{
  while (isRunning()) {
    getRunCondition().wait_for();
    sleep(10); // Let the threads some time to stop

    thread_queue_mutex.lock();
    DBG("ThreadWatcher starting its work\n");

    try {
      queue<AmThread*> thread_queue_active;

      while (!thread_queue.empty()) {
        AmThread* current_thread = thread_queue.front();
        thread_queue.pop();
        thread_queue_mutex.unlock();

        DBG("Thread %lu is to be processed in thread watcher.\n",
            current_thread->getPid());

        if (!isRunning()) {
          DBG("Telling thread %lu to finish its work\n",
              current_thread->getPid());
          current_thread->stop();
          current_thread->join();
        }
        else if (current_thread->isRunning()) {
          DBG("tThread %lu still running.\n", current_thread->getPid());
          thread_queue_active.push(current_thread);
        }
        else {
          DBG("Thread %lu has been destroyed.\n", current_thread->getPid());
          delete current_thread;
        }
      }

      thread_queue_mutex.lock();
      swap(thread_queue, thread_queue_active);
    }
    catch (...) {
      /* this one is IMHO very important, as lock is called in try block!
       */
      ERROR("unexpected exception, state may be invalid!\n");
    }

    DBG("Thread watcher finished\n");

    if (thread_queue.empty()) {
      getRunCondition().set(false);
    }

    thread_queue_mutex.unlock();
  }
}
