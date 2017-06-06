#ifndef _MSG_LOGGER_H_
#define _MSG_LOGGER_H_

#include "AmThread.h"
#include "atomic_types.h"
#include "cstring.h"

#include <set>
#include <string>

struct sockaddr_storage;

class msg_logger : public atomic_ref_cnt
{
 public:
  msg_logger() {}
  virtual ~msg_logger() {}
  virtual int log(const char* buf, int len, sockaddr_storage* src_ip,
                  sockaddr_storage* dst_ip, cstring method,
                  int reply_code = 0) = 0;
};

class exclusive_file;

class file_msg_logger : public msg_logger
{
 protected:
  exclusive_file* excl_fp;

  int write(const void* buf, int len);
  int writev(const struct iovec* iov, int iovcnt);

  virtual int write_file_header() = 0;

 public:
  file_msg_logger()
      : excl_fp(NULL)
  {
  }
  ~file_msg_logger();

  int open(const char* filename);
  int log(const char* buf, int len, sockaddr_storage* src_ip,
          sockaddr_storage* dst_ip, cstring method, int reply_code = 0) = 0;
};

class cf_msg_logger : public file_msg_logger
{
  std::set<std::string> known_destinations;

  int write_src_dst(const std::string& obj);

 protected:
  int write_file_header() { return 0; }

 public:
  int log(const char* buf, int len, sockaddr_storage* src_ip,
          sockaddr_storage* dst_ip, cstring method, int reply_code = 0);
};

#endif
