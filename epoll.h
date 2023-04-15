#ifndef LIBROUTINE_EPOLL_H_
#define LIBROUTINE_EPOLL_H_

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/epoll.h>

class RtEpollEvents {
 public:
  int size_;
  epoll_event *events_;

  RtEpollEvents(int n) {
    size_ = n;
    events_ = (struct epoll_event *)calloc(1, n * sizeof(struct epoll_event));
  }

  ~RtEpollEvents() { free(events_); }
};

int rt_epoll_create(int size);
int rt_epoll_wait(int epfd, RtEpollEvents *events, int maxevents, int timeout);
int rt_epoll_ctl(int epfd, int op, int fd, epoll_event *ev);

struct epoll_res *rt_epoll_res_alloc(int n);
void rt_epoll_res_free(struct epoll_res *);

#endif
