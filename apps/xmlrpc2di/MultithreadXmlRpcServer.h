/**
 * Written by Jae An (jae_young_an@yahoo.com)
 * http://sourceforge.net/forum/forum.php?thread_id=910224&forum_id=240495
 *
 * adapted for SEMS by Stefan Sayer stefan.sayer at iptego.com
 */

#ifndef _MULTI_THREAD_XMLRPCSERVER_H_
#define _MULTI_THREAD_XMLRPCSERVER_H_

#include "XmlRpcDispatch.h"
#include "XmlRpcServer.h"

#include "AmEventQueue.h"
#include "AmThread.h"

#include <queue>
#include <vector>

namespace XmlRpc
{
class MultithreadXmlRpcServer;

class WorkerThread
    : public AmThread
    , public AmEventQueueInterface
{
  MultithreadXmlRpcServer* chief;
  AmCondition<bool>        runcond;

  AmCondition<bool> running;

 protected:
  void run();
  void on_stop();

 public:
  WorkerThread(MultithreadXmlRpcServer* chief);

  void addXmlRpcSource(XmlRpcSource* source,
                       unsigned eventMask); // call this method to make it run
  void wakeup();


  void postEvent(AmEvent* ev);

 private:
  XmlRpcDispatch dispatcher;
};

#define MAX_THREAD_SIZE 8

//! Multi-threaded sever class to handle XML RPC requests
class MultithreadXmlRpcServer : public XmlRpcServer
{
 public:
  //! Create a server object.
  MultithreadXmlRpcServer();
  //! Destructor.
  virtual ~MultithreadXmlRpcServer();

  /* report back from work */
  void reportBack(WorkerThread* thr);
  void createThreads(unsigned int n);

 protected:
  //! Accept a client connection request
  virtual void acceptConnection();

 private:
  AmMutex                    waiting_mutex;
  std::queue<WorkerThread*>  waiting;
  AmCondition<bool>          have_waiting;
  std::vector<WorkerThread*> workers;
  WorkerThread*              getIdleThread();
};

} // namespace XmlRpc

#endif
