#include <stdio.h>
#include "eventloop.h"
#include "epoll.h"
#include "rt.h"
#include "common.h"

EventLoop::EventLoop() {
  epfd_ = rt_epoll_create(ep_size_);
  // todo 改为多级时间轮
  time_wheel_ = new TimeWheel(60 * 1000);
  active_list_ = new TimeWheelSlotLink();
  timeout_list_ = new TimeWheelSlotLink();
}

EventLoop::~EventLoop() {
  if (time_wheel_) {
    delete time_wheel_;
  }
  if (active_list_) {
    delete active_list_;
  }
  if (time_wheel_) {
    delete time_wheel_;
  }
}

int EventLoop::poll(struct pollfd fds[], nfds_t nfds, int timeout_ms, poll_func func) {
  if (timeout_ms == 0) {
    return func(fds, nfds, 0);
  }
  if (timeout_ms < 0) {
    timeout_ms = INT32_MAX;
  }

  auto curr_rt = get_curr_routine();
  auto group = new PollFdGroup();
  group->epfd = epfd_;
  group->fds_ = fds;
  group->nfds_ = nfds;

  // group->cb_ = OnPollProcessEvent;
  group->arg_ = curr_rt;

  for (int i = 0; i < nfds; i++) {
    auto item = new PollFdItem();
    item->group = group;
    item->pSelf = &fds[i];
    group->items_.emplace_back(item);

    epoll_event& ev = item->events_;

    if (fds[i].fd > -1) {
      ev.data.ptr = item;
      int ret = rt_epoll_ctl(epfd_, EPOLL_CTL_ADD, fds[i].fd, &ev);
      if (ret < 0) {
        // free()
        return func(fds, nfds, timeout_ms);
      }
    }
  }

  auto now = get_time_ms();
  group->timeout_ms_ = now + timeout_ms;

  int ret = time_wheel_->add_item(group);
  if (ret != 0) {
    printf("add timewheel error\n");
  } else {
    // 交出cpu
    rt_yield_ct();
    // 等上一个方法回来时，重新拿到了cpu
  }

  // 将事件从epoll上移除

  return 0;
}

