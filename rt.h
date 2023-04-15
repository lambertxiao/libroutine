#ifndef LIBROUTINE_RT_H_
#define LIBROUTINE_RT_H_

#include "routine.h"

// 创建一个协程
int rt_create(Routine** r, RoutineAttr* attr, RoutineFunc func, void* arg);
// 启动一个协程
void rt_resume(Routine* r);
// 将传入的协程退出cpu
void rt_yield(Routine* r);
// 将当前正在执行的协程让出CPU
void rt_yield_ct();
// 释放资源
void rt_release(Routine* r);
// 重置状态
void rt_reset(Routine* r);

// r1退位，r2上位
void rt_swap(Routine* r1, Routine* r2);

RtCond* rt_cond_alloc();
void rt_cond_wait(RtCond* cond, int timeout);
void rt_cond_signal(RtCond* cond);
void rt_cond_broadcast(RtCond* cond);

int rt_poll(EventLoop* ctx, PollFD fds[], int timeout_ms);
void rt_eventloop(EventLoop* ctx, EventLoopFunc* func, void* arg);

#endif
