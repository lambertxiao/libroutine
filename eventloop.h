#ifndef LIBROUTINE_EVENTLOOP_H_
#define LIBROUTINE_EVENTLOOP_H_

#include <sys/poll.h>
#include "time_wheel.h"
#include <sys/epoll.h>
#include "syscall.h"

// 每一个线程持有一个eventloop
class EventLoop {
 public:
  int epfd_;
  int ep_size_ = 10 * 1024;
  epoll_event* events_;

  // 用一个时间轮来存放所有的定时任务
  TimeWheel* time_wheel_;
  TimeWheelSlotLink* timeout_list_;
  TimeWheelSlotLink* active_list_;
  EventLoop();
  ~EventLoop();

  void loop();

  // 轮训fds，如果有设置超时，由于单线程里不能阻塞操作，需要将回调设置到时间轮上
  int poll(struct pollfd fds[], nfds_t nfds, int timeout_ms, poll_pfn_t func);

  int poll_wait(epoll_event* events, int maxevents, int timeout);
  int poll_ctl(int op, int fd, epoll_event* ev);
};

struct PollItem;

// 多个fd封装在一个group里挂在时间轮上的结构
struct PollGroup : public TimeWheelSlotItem {
  // 待监控的fd
  struct pollfd* fds_;
  // 待监控fd的个数
  nfds_t nfds_;  // typedef unsigned long int nfds_t;

  std::vector<PollItem*> items_;

  // 表示是否将所有事件从epoll实例中分离，等待处理
  int is_all_event_detach;

  // epoll描述符
  int epfd;

  // 触发了事件的fd个数
  int trigger_cnt_;
};

// 这是单个fd在时间轮上的结构
struct PollItem : public TimeWheelSlotItem {
  // 描述了哪个fd关心什么事件，收到了什么事件
  struct pollfd* fd_;
  PollGroup* group_;
  // 发生了什么事件
  struct epoll_event events_;
};

#endif
