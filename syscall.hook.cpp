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

// 定义了一堆的函数指针
typedef int (*poll_pfn_t)(pollfd fds[], nfds_t nfds, int timeout);
poll_pfn_t g_sys_poll_func= (poll_pfn_t)dlsym(RTLD_NEXT, "poll");

// dlsym函数是一个C语言的动态链接库函数，用于在运行时动态地获取一个共享库（或DLL）中的函数或变量地址
// 下面的逻辑将所有同步的系统调用如connect、send等函数的地址先保存了下来

// 接受一个函数名，将该函数替换掉，如传入connect函数名，则将g_sys_connect_func
#define HOOK_SYS_FUNC(name) if( !g_sys_##name##_func ) { g_sys_##name##_func = (name##_pfn_t)dlsym(RTLD_NEXT,#name); }

extern int co_poll_inner(EventLoop *ctx,struct pollfd fds[], nfds_t nfds, int timeout, poll_pfn_t pollfunc);

// 在给定的超时时间内，轮训fds
int poll(struct pollfd fds[], nfds_t nfds, int timeout) {
	HOOK_SYS_FUNC( poll );

	if (!rt_is_enable_sys_hook() || timeout == 0) {
		return g_sys_poll_func(fds, nfds, timeout);
	}
  
  // 合并重复的文件描述符
  std::unordered_map<int, short> events_map;
  for (size_t i = 0; i < nfds; i++) {
    events_map[fds[i].fd] |= fds[i].events;
  }

  // 调用事件循环的poll函数
  auto loop = get_thread_eventloop();
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

void rt_enable_hook_sys() {
  auto rt = get_curr_routine();
  if (rt) {
    rt->enable_hook_sys_ = true;
  }
}  

