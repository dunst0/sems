#ifndef _FIFO_BUFFER_H_
#define _FIFO_BUFFER_H_

#include <sys/uio.h>

class fifo_buffer
{
  unsigned char* data;
  unsigned char* data_end;

  // write at _p_head
  unsigned char* p_head;

  // read from p_tail
  unsigned char* p_tail;

  unsigned int size;
  unsigned int free_space;

 public:
  fifo_buffer(unsigned int size);
  ~fifo_buffer();

  // returns space occupied in buffer
  unsigned int get_buffered_bytes() { return size - free_space; }

  // returns space left in buffer
  unsigned int get_free_space() { return free_space; }

  // write at the head of the buffer
  int write(const void* buf, unsigned int len);
  int writev(const struct iovec* iov, int iovcnt);

  // returns size of linearly readable data
  unsigned int get_read_bs()
  {
    return (p_tail <= p_head) ? (unsigned int) (p_head - p_tail)
                              : (unsigned int) (data_end - p_tail);
  }

  // returns the buffer's tail
  void* get_read_ptr() { return p_tail; }

  // moves the tail's pointer by the length passed
  void skip(unsigned int len)
  {
    p_tail += len;
    if (p_tail >= data_end) p_tail = p_tail - data_end + data;
    free_space += len;
  }
};

#endif
