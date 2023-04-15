#ifndef LIBROUTINE_THREAD_ENV_H_
#define LIBROUTINE_THREAD_ENV_H_

#include "eventloop.h"

class Routine;

// 协程环境的结构体，用来存储协程调度器的相关信息，一个线程有一个环境，线程上的所有协程共用它
class RoutineThreadEnv {
 public:
  // 当前线程里的协程调用栈
  Routine* call_stack_[128];
  // 表示调用栈中当前记录的协程数量
  int call_stack_size_;
  // 指向协程调度器使用的epoll实例的指针
  EventLoop* loop_;

  // pending和occupy仅在共享栈模式下有用
  Routine* pending_rt_;
  Routine* occupy_rt_;

  RoutineThreadEnv();
  ~RoutineThreadEnv();

  // 获取当前线程里正在执行的协程
  Routine* get_curr_routine();

  // 将给定的协程压入线程调用栈
  void push_to_call_stack(Routine* r);
};

// 初始化协程所在的线程环境
void init_curr_thread_env();
RoutineThreadEnv* get_curr_thread_env();

#endif
