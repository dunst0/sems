#ifndef _PCAP_LOGGER_H_
#define _PCAP_LOGGER_H_

#include "msg_logger.h"

#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <sys/socket.h>

/** class for logging sent/received data in PCAP format */
class pcap_logger : public file_msg_logger
{
 protected:
  int write_file_header();

 public:
  int log(const char* data, int data_len, struct sockaddr* src,
          struct sockaddr* dst, size_t addr_len);

  int log(const char* buf, int len, sockaddr_storage* src_ip,
          sockaddr_storage* dst_ip, cstring method, int reply_code = 0);
};

#endif
