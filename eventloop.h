#ifndef LIBROUTINE_EVENTLOOP_H_
#define LIBROUTINE_EVENTLOOP_H_

#include "time_wheel.h"

// 每一个线程持有一个eventloop
class EventLoop {
 public:
   int epfd_;
   int ep_size_ = 1024 * 10;
   // 用一个时间轮来存放所有的定时任务
   TimeWheel* time_wheel_;
   TWSlotLink* timeout_list_;
   TWSlotLink* active_list_;
   EventLoop();
   ~EventLoop();
};

#endif

