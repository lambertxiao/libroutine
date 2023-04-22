#include "rt.h"
#include "routine.h"
#include <memory>
#include <string.h>
#include "routine_ctx.h"
#include "routine_cond.h"
#include "logger.h"

extern "C" {
extern void rtctx_swap(RoutineCtx*, RoutineCtx*) asm("rtctx_swap");
};

static int routineRealEntryFunc(Routine* rt, void*);

int rt_create(Routine** co, RoutineAttr* attr, RoutineFunc func, void* arg) {
  auto rt = new Routine(get_curr_thread_env(), false, attr, func, arg);
  *co = rt;
  return 0;
}

void rt_resume(Routine* rt) {
  RoutineThreadEnv* env = rt->env_;
  auto curr_rt = env->get_curr_routine();

  if (!rt->is_start_) {
    rt->init_ctx((RoutineEntryFunc(routineRealEntryFunc)));
    rt->is_start_ = true;
  }
  // 把目标协程压栈
  env->push_to_call_stack(rt);
  // 切换新协程和当前正在运行的协程
  rt_swap(curr_rt, rt);
};

// 作为协程的启动函数
static int routineRealEntryFunc(Routine* rt, void*) {
  LOG_DEBUG("exec routine:%p", rt);
  // 执行协程的具体行为
  if (rt->func_) {
    rt->func_(rt->arg_);
  }
  // 当这就执行结束了
  rt->is_stop_ = true;

  // 执行完了退出cpu
  rt_yield_env(rt->env_);

  return 0;
}

void rt_yield(Routine* co) {};

void rt_yield_ct() {
  rt_yield_env(get_curr_thread_env());
};

void rt_yield_env(RoutineThreadEnv* env) {
  // 当前协程的调用协程
  Routine* last = env->call_stack_[env->call_stack_size_ - 2];
  // 当前协程
  Routine* curr = env->call_stack_[env->call_stack_size_ - 1];

  // 调用栈减小
  env->call_stack_size_--;

  // 切换上下文
  rt_swap(curr, last);
};

void rt_release(Routine* co) {};
void rt_reset(Routine* co) {};

// 传入当前正在运行的协程和想要运行的协程
void rt_swap(Routine* curr, Routine* pending_rt) {
  if (curr == pending_rt) {
    return;
  }

  LOG_DEBUG("swap routine %p => %p", curr, pending_rt);
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

  if (update_occupy_co && update_pending_co && update_occupy_co != update_pending_co) {
    // 进了这个判断代表前面做了协程切换
    // resume stack buffer
    if (update_pending_co->stack_buff_ && update_pending_co->stack_buff_len_ > 0) {
      // 从缓冲区中恢复栈内容
      memcpy(update_pending_co->stack_sp_, update_pending_co->stack_buff_, update_pending_co->stack_buff_len_);
    }
  }
}

RoutineCond* rt_cond_alloc() { return new RoutineCond(); }

int rt_cond_free(RoutineCond* cond) {
  if (!cond) {
    return -1;
  }
  delete cond;
  return 0;
}

int rt_cond_wait(RoutineCond* cond, int timeout) {
  if (!cond) {
    printf("invalid routine cond\n");
    return -1;
  }
  auto wait_item = new RoutineCondWaitItem();
  wait_item->timeout_ = timeout;
  wait_item->bind_rt_ = get_curr_routine();
  wait_item->wait_cb_ = [](Routine* rt) {
    // rt_resume会让协程回到下面的yield之后
    rt_resume(rt);
  };

  int ret = cond->wait(wait_item);
  if (ret != 0) {
    delete wait_item;
    return ret;
  }
  // 让出cpu
  rt_yield_ct();

  // 执行到这里证明，重新拿到了cpu的执行权了，因此删掉对条件变量的等待
  cond->items_.remove(wait_item);
  delete wait_item;

  return 0;
}

uint64_t get_timems() { return 0; }

int rt_cond_signal(RoutineCond* cond) { return cond->signal(); }

int rt_cond_broadcast(RoutineCond* cond) { return cond->broadcast(); }

void rt_enable_hook_sys() {
  auto rt = get_curr_routine();
  if (rt) {
    rt->enable_hook_sys_ = true;
  }
}

void rt_eventloop(EventLoop* ctx, EventLoopFunc* func, void* arg) { ctx->loop(); }

Routine* get_curr_routine() {
  auto env = get_curr_thread_env();
  if (!env) {
    return nullptr;
  }

  return env->call_stack_[env->call_stack_size_ - 1];
}

void rt_disable_hook_sys() {
  auto rt = get_curr_routine();
  if (rt) {
    rt->enable_hook_sys_ = false;
  }
}

bool rt_is_enable_sys_hook() {
  auto rt = get_curr_routine();
  return rt && rt->enable_hook_sys_;
}

ShareStack* rt_alloc_share_stack(int count, uint64_t size) {
  auto ss = new ShareStack(count, size);
  return ss;
}
