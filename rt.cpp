#include "rt.h"
#include "routine.h"
#include <memory>
#include <string.h>
#include "routine_ctx.h"

extern "C" {
extern void rtctx_swap(RoutineCtx*, RoutineCtx*) asm("rtctx_swap");
};

int rt_create(Routine** co, RoutineAttr* attr, RoutineFunc func, void* arg) {
  auto rt = new Routine(get_curr_thread_env(), false, attr, func, arg);
  *co = rt;
  return 0;
}

void rt_resume(Routine* rt) {
  RoutineThreadEnv* env = rt->env_;
  auto curr_rt = env->get_curr_routine();
  // 把目标协程压栈
  env->push_to_call_stack(rt);
  // 切换新协程和当前正在运行的协程
  rt_swap(curr_rt, rt);
};

void rt_yield(Routine* co) {};
void rt_yield_ct() {};
void rt_release(Routine* co) {};
void rt_reset(Routine* co) {};

// 传入当前正在运行的协程和想要运行的协程
void rt_swap(Routine* curr, Routine* pending_rt) {
  if (curr == pending_rt) {
    return;
  }

  RoutineThreadEnv* env = curr->env_;

  // 将当前协程的栈指针设置为当前栈顶的地址。这里使用一个局部变量c的地址作为栈指针，因为c是一个自动变量，它的地址就是当前栈顶的地址。
  char c;
  curr->stack_sp_ = &c;

  if (!pending_rt->is_share_stack) {
    // 自己开心快活就行了
    env->pending_rt_ = NULL;
    env->occupy_rt_ = NULL;
  } else {
    // 将当前要执行的协程设置到等待位置上
    env->pending_rt_ = pending_rt;
    // 查看该协程的栈内存是否已经有其他协程在用了
    // todo 这里有点奇怪，通过共享栈的当前正在使用的协程来作为occupy_rt
    // 这里的occupy_rt只有可能是正在线程上运行的协吧？
    Routine* occupy_rt = pending_rt->stack_mem_->occupy_rt_;
    // 更新占用协程为自己
    pending_rt->stack_mem_->occupy_rt_ = pending_rt;

    // 理论上env上的协程就是occupy_rt了，这里是无效赋值？
    env->occupy_rt_ = occupy_rt;

    if (occupy_rt && occupy_rt != pending_rt) {
      // 由于当前协程是共享栈模式，那么当有其他协程用到了pending_co的栈内存，
      // 则需要这个占用的协程将它存在共享栈内存里的数据拷贝到它自己的缓冲区中去
      occupy_rt->save_stack_to_buff();
    }
  }

  // swap context交换协程上下文了
  rtctx_swap(&(curr->ctx_), &(pending_rt->ctx_));

  // stack buffer may be overwrite, so get again;
  Routine* update_occupy_co = env->occupy_rt_;
  Routine* update_pending_co = env->pending_rt_;

  if (update_occupy_co && update_pending_co &&
      update_occupy_co != update_pending_co) {
    // 进了这个判断代表前面做了协程切换
    // resume stack buffer
    if (update_pending_co->stack_buff_ &&
        update_pending_co->stack_buff_len_ > 0) {
      // 从缓冲区中恢复栈内容
      memcpy(update_pending_co->stack_sp_, update_pending_co->stack_buff_,
             update_pending_co->stack_buff_len_);
    }
  }
}

RtCond* rt_cond_alloc() { return nullptr; }

void rt_cond_wait(RtCond* cond, int timeout) {}
void rt_cond_signal(RtCond* cond) {}
void rt_cond_broadcast(RtCond* cond) {}

int rt_poll(EventLoop* ctx, PollFD fds[], int timeout_ms) { return 0; }

void rt_eventloop(EventLoop* ctx, EventLoopFunc* func, void* arg) {}
