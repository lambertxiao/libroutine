#include "thread_env.h"
#include "routine.h"
#include "eventloop.h"
#include <stdio.h>
#include "logger.h"

// 定义了一个线程级别的变量
static __thread RoutineThreadEnv* routineEnvPerThread = NULL;

RoutineThreadEnv::RoutineThreadEnv() : call_stack_size_(0) {
  auto attr = new RoutineAttr();
  attr->name = "main";
  // 创建主协程
  auto main_routine = new Routine(this, true, attr, nullptr, nullptr);
  LOG_DEBUG("create main routine:%p", main_routine);

  call_stack_[call_stack_size_++] = main_routine;
  pending_rt_ = nullptr;
  occupy_rt_ = nullptr;
  loop_ = new EventLoop();
}

RoutineThreadEnv::~RoutineThreadEnv() { delete loop_; }

RoutineThreadEnv* get_curr_thread_env() {
  if (routineEnvPerThread) {
    return routineEnvPerThread;
  }

  routineEnvPerThread = new RoutineThreadEnv();
  return routineEnvPerThread;
}

Routine* RoutineThreadEnv::get_curr_routine() { return call_stack_[call_stack_size_ - 1]; }

void RoutineThreadEnv::push_to_call_stack(Routine* r) { call_stack_[call_stack_size_++] = r; }
