#ifndef LIBCORT_COROUTINE_H_
#define LIBCORT_COROUTINE_H_

struct Routine {};
struct RoutineAttr {};
struct EventLoop {};
struct EventLoopFunc {};
struct PollFD {};

typedef void* (*routine_func)(void*);

// 创建一个协程
int rt_create(Routine* co, const Routine *attr, routine_func func, void *arg);
// 启动一个协程
void rt_resume(Routine *co);
// 将传入的协程退出cpu 
void rt_yield(Routine *co);
// 将当前正在执行的协程让出CPU
void rt_yield_ct(); 
// 释放资源
void rt_release(Routine* co);
// 重置状态
void rt_reset(Routine* co); 

Routine* rt_self();

int	rt_poll(EventLoop *ctx, PollFD fds[], int timeout_ms);
void rt_eventloop(EventLoop *ctx, EventLoopFunc func, void *arg);

#endif

