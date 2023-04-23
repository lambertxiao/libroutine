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

// 记录阻塞的系统调用的地址 
typedef int (*poll_pfn_t)(pollfd fds[], nfds_t nfds, int timeout);
poll_pfn_t g_sys_poll_func = (poll_pfn_t)dlsym(RTLD_NEXT, "poll");

typedef int (*connect_pfn_t)(int socket, const struct sockaddr* address, socklen_t address_len);
static connect_pfn_t g_sys_connect_func = (connect_pfn_t)dlsym(RTLD_NEXT, "connect");

typedef ssize_t (*read_pfn_t)(int fildes, void* buf, size_t nbyte);
static read_pfn_t g_sys_read_func = (read_pfn_t)dlsym(RTLD_NEXT, "read");

typedef ssize_t (*write_pfn_t)(int fildes, const void* buf, size_t nbyte);
static write_pfn_t g_sys_write_func = (write_pfn_t)dlsym(RTLD_NEXT, "write");

// dlsym函数是一个C语言的动态链接库函数，用于在运行时动态地获取一个共享库（或DLL）中的函数或变量地址
// 下面的逻辑将所有同步的系统调用如connect、send等函数的地址先保存了下来
#define HOOK_SYS_FUNC(name)                                      \
  if (!g_sys_##name##_func) {                                    \
    g_sys_##name##_func = (name##_pfn_t)dlsym(RTLD_NEXT, #name); \
  }

// extern int co_poll_inner(EventLoop* ctx, struct pollfd fds[], nfds_t nfds, int timeout, poll_pfn_t pollfunc);

// 单线程非阻塞的poll，在给定的超时时间内，轮训fds
int poll(struct pollfd fds[], nfds_t nfds, int timeout) {
  HOOK_SYS_FUNC(poll);

  if (!rt_is_enable_sys_hook() || timeout == 0) {
    return g_sys_poll_func(fds, nfds, timeout);
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

  int ret = loop->poll(pollfds.data(), pollfds.size(), timeout, g_sys_poll_func);

  // 更新原始的pollfd结构体数组
  for (size_t i = 0; i < nfds; i++) {
    auto it = events_map.find(fds[i].fd);
    if (it != events_map.end()) {
      fds[i].revents = pollfds[it->second].revents & fds[i].events;
    }
  }
  return ret;
}

// 将fd设置为非阻塞
int fd_nonblock(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    return -1;
  }

  flags |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) == -1) {
    return -1;
  }
  return 0;
}

int connect(int fd, const struct sockaddr* address, socklen_t address_len) {
  HOOK_SYS_FUNC(connect);

  if (!rt_is_enable_sys_hook()) {
    return g_sys_connect_func(fd, address, address_len);
  }

  int ret = fd_nonblock(fd);
  if (ret < 0) {
    return ret;
  }

  ret = g_sys_connect_func(fd, address, address_len);
  if (ret < 0) {
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
  LOG_DEBUG("exec hook read\n");
  HOOK_SYS_FUNC(read);

  if (!rt_is_enable_sys_hook()) {
    return g_sys_read_func(fd, buf, nbyte);
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

  // 到了这里，重新拿到了cpu的执行权, 此时fd已经可读了，将数据读出
  ssize_t readret = g_sys_read_func(fd, (char*)buf, nbyte);

  if (readret < 0) {
    LOG_ERROR("read error, ret:%ld\n", readret);
  }

  return readret;
}

ssize_t write(int fd, const void* buf, size_t nbyte) {
  LOG_DEBUG("exec hook write\n");
  HOOK_SYS_FUNC(write);

  if (!rt_is_enable_sys_hook()) {
    return g_sys_write_func(fd, buf, nbyte);
  }

  size_t wrotelen = 0;

  // 尝试直接写，写不了就告诉epoll能写了再通知我
  ssize_t writeret = g_sys_write_func(fd, (const char*)buf + wrotelen, nbyte - wrotelen);

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

    writeret = g_sys_write_func(fd, (const char*)buf + wrotelen, nbyte - wrotelen);

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
