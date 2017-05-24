#ifndef _RAW_SENDER_H_
#define _RAW_SENDER_H_

struct sockaddr_storage;

class raw_sender
{
  static int rsock;

  raw_sender() {}
  ~raw_sender() {}

 public:
  static int init();
  static int send(const char* buf, unsigned int len, int sys_if_idx,
                  const sockaddr_storage* from, const sockaddr_storage* to);
};

#endif
