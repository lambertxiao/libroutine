#ifndef LIBROUTINE_RT_H_
#define LIBROUTINE_RT_H_

#include "routine.h"
#include "routine_cond.h"

// 创建一个协程
int rt_create(Routine** r, RoutineAttr* attr, RoutineFunc func, void* arg);
// 启动一个协程
void rt_resume(Routine* r);
// 将传入的协程退出cpu
void rt_yield(Routine* r);
// 将当前正在执行的协程让出CPU
void rt_yield_ct();
// 将当前线程中的协程切出
void rt_yield_env(RoutineThreadEnv* env);
// 释放资源
void rt_release(Routine* r);
// 重置状态
void rt_reset(Routine* r);

// r1退位，r2上位
void rt_swap(Routine* r1, Routine* r2);
static int routineRealEntryFunc(Routine * rt, void *);
RoutineCond* rt_cond_alloc();
int rt_cond_wait(RoutineCond* cond, int timeout);
void rt_cond_signal(RoutineCond* cond);
void rt_cond_broadcast(RoutineCond* cond);

Routine* get_curr_routine();
int rt_poll(EventLoop* ctx, PollFD fds[], int timeout_ms);
void rt_eventloop(EventLoop* ctx, EventLoopFunc* func, void* arg);

#endif
