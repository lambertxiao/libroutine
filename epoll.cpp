#include "epoll.h"

int rt_epoll_create(int size) {
  return epoll_create(size);
}

int rt_epoll_wait(int epfd, RtEpollEvents* events, int maxevents, int timeout) {
  return epoll_wait(epfd, events->events_, maxevents, timeout);
}

int rt_epoll_ctl(int epfd, int op, int fd, epoll_event* ev) {
  return epoll_ctl(epfd, op, fd, ev);
}

