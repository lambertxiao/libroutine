#ifndef LIBROUTINE_FD_HOOK_H_
#define LIBROUTINE_FD_HOOK_H_

#include <fcntl.h>
#include "logger.h"

// 记录fd的状态
struct FdAttr {
  int flags_ = 0;

  void set_nonblock() { flags_ |= O_NONBLOCK; }
  bool is_nonblock() { return flags_ & O_NONBLOCK; }
};

class FdAttrs {
 private:
  const static int max_cnt_ = 102400;
  static FdAttr* fds_[102400];
 
 public:
  static FdAttr* alloc(int fd);
  static void free(int fd);
  static FdAttr* get(int fd);
 
};

#endif
