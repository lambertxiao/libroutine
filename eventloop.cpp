#include <stdio.h>
#include "eventloop.h"
#include "rt.h"
#include "common.h"
#include "logger.h"

EventLoop::EventLoop() {
  epfd_ = epoll_create(ep_size_);
  events_ = (epoll_event*)calloc(ep_size_, sizeof(epoll_event));

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

void EventLoop::loop() {
  while (true) {
    // 1. 找一下epoll上有没有事发生，只等1ms
    int ret = poll_wait(events_, ep_size_, 1);

    for (int i = 0; i < ret; i++) {
      auto item = (TimeWheelSlotItem*)events_[i].data.ptr;
      // 这里的item可能是PollFdGroup，也可能是PollFdItem
      if (item->cb_pre_) {
        item->cb_pre_(item);
      }
      active_list_->add_back(item);
    }

    auto now = get_time_ms();
    // 2. 找一下时间轮上是否有事件超时了
    time_wheel_->time_forward(timeout_list_, now);

    // 3. 无论是活跃事件还是超时事件都执行回调
    TimeWheelSlotLink* links[2] = {active_list_, timeout_list_};

    if (active_list_->size() != 0 || timeout_list_->size() != 0) {
      LOG_DEBUG("run loop, active_cnt: %d, timeout_cnt:%d", active_list_->size(), timeout_list_->size());
    }
    for (auto alink : links) {
      while (true) {
        auto item = alink->pop_front();
        if (!item) {
          break;
        }
        if (item->cb_) {
          item->cb_(item);
        }
      }
    }
  }
}

int EventLoop::poll_wait(epoll_event* events, int maxevents, int timeout) { return epoll_wait(epfd_, events, maxevents, timeout); }

int EventLoop::poll_ctl(int op, int fd, epoll_event* ev) { return epoll_ctl(epfd_, op, fd, ev); }

static void OnPollFdGroupDone(TimeWheelSlotItem* item) {
  auto routine = (Routine*)item->arg_;
  rt_resume(routine);
}

int EventLoop::poll(struct pollfd fds[], nfds_t nfds, int timeout_ms, sys_poll_func poll_func) {
  if (timeout_ms == 0) {
    return poll_func(fds, nfds, 0);
  }
  if (timeout_ms < 0) {
    timeout_ms = INT32_MAX;
  }

  auto curr_rt = get_curr_routine();
  auto group = new PollFdGroup();
  group->epfd = epfd_;
  group->fds_ = fds;
  group->nfds_ = nfds;
  group->cb_ = OnPollFdGroupDone;
  group->arg_ = curr_rt;

  // 告诉epoll我关心这些fd的事件
  for (int i = 0; i < nfds; i++) {
    // 这里考虑一下是放堆上好还是放栈上好
    auto item = new PollFdItem();
    item->group_ = group;
    item->fd_ = &fds[i];
    group->items_.emplace_back(item);

    epoll_event& ev = item->events_;

    if (fds[i].fd > -1) {
      ev.data.ptr = item;
      int ret = poll_ctl(EPOLL_CTL_ADD, fds[i].fd, &ev);
      if (ret < 0) {
        // free()
        return poll_func(fds, nfds, timeout_ms);
      }
    }
  }

  auto now = get_time_ms();
  group->timeout_ms_ = now + timeout_ms;

  // 整组时间挂在时间轮上
  int ret = time_wheel_->add_item(group);
  if (ret != 0) {
    LOG_ERROR("add timewheel error");
    return ret;
  } else {
    // 交出cpu
    rt_yield_ct();
    // 等上一个方法回来时，重新拿到了cpu
  }

  // 将事件从epoll上移除
  for (int i = 0; i < nfds; i++) {
    if (fds[i].fd > -1) {
      auto item = group->items_[i];
      int ret = poll_ctl(EPOLL_CTL_DEL, fds[i].fd, &item->events_);
    }
  }

  int trigger_cnt = group->trigger_cnt_;
  delete group;
  return trigger_cnt;
}
