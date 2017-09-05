#ifndef _RedisConnectionPool_h_
#define _RedisConnectionPool_h_

#include "AmThread.h"

#include <hiredis/hiredis.h>

#include <list>
#include <string>
#include <vector>

class RedisConnectionPool : public AmThread
{
  std::list<redisContext*> connections;
  unsigned int             total_connections;
  unsigned int             failed_connections;
  AmMutex                  connections_mut;

  AmCondition<bool> have_active_connection;
  AmCondition<bool> try_connect;

  std::vector<unsigned int> retry_timers;
  unsigned int              retry_index;

  std::string  redis_server;
  unsigned int redis_port;
  unsigned int max_wait;

 protected:
  void run();
  void on_stop();

 public:
  RedisConnectionPool();
  ~RedisConnectionPool();

  redisContext* getActiveConnection();

  void returnConnection(redisContext* c);

  void returnFailedConnection(redisContext* c);

  void set_config(std::string& server, unsigned int port,
                  std::vector<unsigned int> timers, unsigned int max_conn_wait);

  void add_connections(unsigned int count);
};

#endif
