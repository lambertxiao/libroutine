#include <sys/socket.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <sys/un.h>

#include <dlfcn.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <errno.h>
#include <time.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <pthread.h>

#include <resolv.h>
#include <netdb.h>

#include <time.h>
#include <unordered_map>
#include "rt.h"
#include "logger.h"
#include "fd_attr.h"
#include "syscall.h"

// 记录阻塞的系统调用的地址
static socket_pfn_t sys_socket_func = (socket_pfn_t)dlsym(RTLD_NEXT, "socket");
static poll_pfn_t sys_poll_func = (poll_pfn_t)dlsym(RTLD_NEXT, "poll");
static connect_pfn_t sys_connect_func = (connect_pfn_t)dlsym(RTLD_NEXT, "connect");
static read_pfn_t sys_read_func = (read_pfn_t)dlsym(RTLD_NEXT, "read");
static write_pfn_t sys_write_func = (write_pfn_t)dlsym(RTLD_NEXT, "write");
static fcntl_pfn_t sys_fcntl_func = (fcntl_pfn_t)dlsym(RTLD_NEXT, "fcntl");
static accpet_pfn_t sys_accpet = (accpet_pfn_t)dlsym(RTLD_NEXT, "accept");

// dlsym函数是一个C语言的动态链接库函数，用于在运行时动态地获取一个共享库（或DLL）中的函数或变量地址
// 下面的逻辑将所有同步的系统调用如connect、send等函数的地址先保存了下来
#define HOOK_SYS_FUNC(name)                                    \
  if (!sys_##name##_func) {                                    \
    sys_##name##_func = (name##_pfn_t)dlsym(RTLD_NEXT, #name); \
  }

// 单线程非阻塞的poll，在给定的超时时间内，轮训fds
int poll(struct pollfd fds[], nfds_t nfds, int timeout_ms) {
  HOOK_SYS_FUNC(poll);

  if (!rt_is_enable_sys_hook() || timeout_ms == 0) {
    return sys_poll_func(fds, nfds, timeout_ms);
  }

  // 合并重复的文件描述符
  std::unordered_map<int, short> events_map;
  for (size_t i = 0; i < nfds; i++) {
    events_map[fds[i].fd] |= fds[i].events;
  }

  // 调用事件循环的poll函数
  auto loop = rt_get_thread_eventloop();
  std::vector<struct pollfd> pollfds;
  for (auto& kv : events_map) {
    struct pollfd pfd;
    pfd.fd = kv.first;
    pfd.events = kv.second;
    pollfds.push_back(pfd);
  }

  int ret = loop->poll(pollfds.data(), pollfds.size(), timeout_ms, sys_poll_func);

  // 更新原始的pollfd结构体数组
  for (size_t i = 0; i < nfds; i++) {
    auto it = events_map.find(fds[i].fd);
    if (it != events_map.end()) {
      fds[i].revents = pollfds[it->second].revents & fds[i].events;
    }
  }
  return ret;
}

int socket(int domain, int type, int protocol) {
  HOOK_SYS_FUNC(socket);

  if (!rt_is_enable_sys_hook()) {
    return sys_socket_func(domain, type, protocol);
  }

  int fd = sys_socket_func(domain, type, protocol);
  if (fd < 0) {
    return fd;
  }

  FdAttrs::alloc(fd);
  // lp->domain = domain;
  // fcntl(fd, F_SETFL, sys_fcntl_func(fd, F_GETFL,0 ));

  return fd;
}

int fcntl(int fd, int cmd, ...) {
  HOOK_SYS_FUNC(fcntl);

  if (fd < 0) {
    return -1;
  }

  va_list arg_list;
  va_start(arg_list, cmd);

  int ret = -1;
  auto attr = FdAttrs::get(fd);

  switch (cmd) {
    case F_DUPFD: {
      int param = va_arg(arg_list, int);
      ret = sys_fcntl_func(fd, cmd, param);
      break;
    }
    case F_GETFD: {
      ret = sys_fcntl_func(fd, cmd);
      break;
    }
    case F_SETFD: {
      int param = va_arg(arg_list, int);
      ret = sys_fcntl_func(fd, cmd, param);
      break;
    }
    case F_GETFL: {
      ret = sys_fcntl_func(fd, cmd);
      if (attr && !attr->is_nonblock()) {
        ret = ret & (~O_NONBLOCK);
      }
      break;
    }
    case F_SETFL: {
      int param = va_arg(arg_list, int);
      int flag = param;
      if (rt_is_enable_sys_hook() && attr) {
        flag |= O_NONBLOCK;
      }
      ret = sys_fcntl_func(fd, cmd, flag);
      if (0 == ret && attr) {
        attr->flags_ = param;
      }
      break;
    }
    case F_GETOWN: {
      ret = sys_fcntl_func(fd, cmd);
      break;
    }
    case F_SETOWN: {
      int param = va_arg(arg_list, int);
      ret = sys_fcntl_func(fd, cmd, param);
      break;
    }
    case F_GETLK: {
      struct flock* param = va_arg(arg_list, struct flock*);
      ret = sys_fcntl_func(fd, cmd, param);
      break;
    }
    case F_SETLK: {
      struct flock* param = va_arg(arg_list, struct flock*);
      ret = sys_fcntl_func(fd, cmd, param);
      break;
    }
    case F_SETLKW: {
      struct flock* param = va_arg(arg_list, struct flock*);
      ret = sys_fcntl_func(fd, cmd, param);
      break;
    }
  }

  va_end(arg_list);

  return ret;
}

int accpet(int fd, const struct sockaddr* address, socklen_t address_len) {
  int accpet_fd = sys_accpet(fd, address, address_len);
  if (accpet_fd >= 0) {
    FdAttrs::alloc(fd);
  }

  return accpet_fd;
}

int connect(int fd, const struct sockaddr* address, socklen_t address_len) {
  HOOK_SYS_FUNC(connect);

  if (!rt_is_enable_sys_hook()) {
    return sys_connect_func(fd, address, address_len);
  }

  int ret = sys_connect_func(fd, address, address_len);

  FdAttr* attr = FdAttrs::get(fd);
  if (!attr) {
    return ret;
  }

  if (attr->is_nonblock()) {
    return ret;
  }

  if (!(ret < 0 && errno == EINPROGRESS)) {
    return ret;
  }

  // 重试3次
  pollfd pf;
  int pollret = 0;
  for (int i = 0; i < 3; i++) {
    memset(&pf, 0, sizeof(pf));
    pf.fd = fd;
    pf.events = POLLOUT | POLLERR | POLLHUP;
    pollret = poll(&pf, 1, 10000);

    if (1 == pollret) {
      break;
    }
  }

  // 连上了，可以往后发了
  if (pf.revents & POLLOUT) {
    // 这里还需要做什么检查？
    return 0;
  } else {
    return ETIMEDOUT;
  }
}

ssize_t read(int fd, void* buf, size_t nbyte) {
  HOOK_SYS_FUNC(read);

  if (!rt_is_enable_sys_hook()) {
    return sys_read_func(fd, buf, nbyte);
  }

  // 这里需要先判断fd是否已经设置过nonBlocking了，
  // 如果是nonBlocking，则直接转调sys_read,
  // 如果是blocking，需要拆分成两个步骤，等待可读事件+有了可读事件后的处理
  auto attr = FdAttrs::get(fd);
  // 如果没有找到这个fd或者fd设置了非阻塞，如果上层已经设置了非阻塞了，则这里什么都不用做
  if (!attr || attr->is_nonblock()) {
    return sys_read_func(fd, buf, nbyte);
  }

  struct pollfd pf = {0};
  pf.fd = fd;

  // 读事件，错误事件，断开事件
  pf.events = (POLLIN | POLLERR | POLLHUP);

  int pollret = poll(&pf, 1, 1000);
  if (pollret < 0) {
    LOG_ERROR("poll error in read, fd:%d ret:%d", fd, pollret);
    return pollret;
  }


  LOG_DEBUG("read poll ret %d", pollret);


  //if (pollret == 0) {
  //  return 0;
  //}

  // 到了这里，重新拿到了cpu的执行权, 此时fd已经可读了，将数据读出
  ssize_t readret = sys_read_func(fd, (char*)buf, nbyte);

  if (readret < 0) {
    LOG_ERROR("read error, pollret:%d ret:%ld\n", pollret, readret);
  }

  return readret;
}

ssize_t write(int fd, const void* buf, size_t nbyte) {
  HOOK_SYS_FUNC(write);

  if (!rt_is_enable_sys_hook()) {
    return sys_write_func(fd, buf, nbyte);
  }

  size_t wrotelen = 0;

  // 尝试直接写，写不了就告诉epoll能写了再通知我
  ssize_t writeret = sys_write_func(fd, (const char*)buf + wrotelen, nbyte - wrotelen);

  if (writeret == 0) {
    return writeret;
  }

  if (writeret > 0) {
    wrotelen += writeret;
  }
  // 循环，直到数据全部写完
  while (wrotelen < nbyte) {
    struct pollfd pf = {0};
    pf.fd = fd;
    // 关注可写事件，错误事件，断开事件
    pf.events = (POLLOUT | POLLERR | POLLHUP);
    poll(&pf, 1, 1000);

    writeret = sys_write_func(fd, (const char*)buf + wrotelen, nbyte - wrotelen);

    if (writeret <= 0) {
      break;
    }
    wrotelen += writeret;
  }
  if (writeret <= 0 && wrotelen == 0) {
    return writeret;
  }
  return wrotelen;
}
