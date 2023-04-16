#include "eventloop.h"
#include "epoll.h"

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
