#include "fd_attr.h"
#include "logger.h"

FdAttr* FdAttrs::fds_[102400];

FdAttr* FdAttrs::alloc(int fd) {
  if (fd < 0 || fd > max_cnt_) {
    LOG_ERROR("invalid fd:%d", fd);
    return nullptr;
  }

  fds_[fd] = new FdAttr();
  return fds_[fd];
}

void FdAttrs::free(int fd) {
  if (fd < 0 || fd > max_cnt_) {
    LOG_ERROR("invalid fd:%d", fd);
    return;
  }

  if (fds_[fd]) {
    delete fds_[fd];
  }
}
FdAttr* FdAttrs::get(int fd) {
  if (fd < 0 || fd > max_cnt_) {
    LOG_ERROR("invalid fd:%d", fd);
    return nullptr;
  }

  return fds_[fd];
}
