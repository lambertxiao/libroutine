#ifndef LIBROUTINE_ROUTINE_H_
#define LIBROUTINE_ROUTINE_H_

#include <sys/poll.h>
#include <stdint.h>
#include "routine_ctx.h"
#include "eventloop.h"
#include "thread_env.h"
#include "stack_mem.h"

struct RoutineAttr {
  int stack_size;
  // 是否使用共享栈
  ShareStack* share_stack;

  RoutineAttr() {
    stack_size = 128 * 1024;
    share_stack = NULL;
  }
} __attribute__((packed));

// 这是用户定义的协程执行方法
typedef void* (*RoutineFunc)(void*);
// 这是实际让cpu执行的协程方法
typedef void* (*RoutineEntryFunc)(void* s, void* s2);

class Routine {
 public:
  bool is_main_;
  RoutineThreadEnv* env_;
  RoutineCtx ctx_;
  RoutineFunc func_;
  void* arg_;

  bool is_start_;
  bool is_stop_;
  // 是否共享协程栈
  bool is_share_stack;

  StackMem* stack_mem_;

  // 当协程退出cpu的时候，需要将栈上的数据缓存在该buff里
  char* stack_buff_;
  uint64_t stack_buff_len_;

  // 栈顶指针, todo 这玩意为什么不放stackmem里
  char* stack_sp_;

  Routine(RoutineThreadEnv* env, bool is_main, RoutineAttr* attr,
          RoutineFunc func, void* arg);

  // 初始化上下文
  void init_ctx(RoutineEntryFunc func);
  void save_stack_to_buff();
};

struct EventLoopFunc {};
struct PollFD {};


// 初始化协程上下文
Routine* rt_self();

EventLoop* get_thread_eventloop();

#endif
