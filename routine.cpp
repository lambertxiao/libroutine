#include "routine.h"
#include "thread_env.h"
#include <memory>
#include <string.h>
#include "stack_mem.h"

enum {
  // RDI寄存器用于存储函数的第一个参数
  kRDI = 7,
  // RSI寄存器用于存储函数的第二个参数
  kRSI = 8,
  // RET寄存器用于存储函数的返回地址
  kRETAddr = 9,
  // RSP寄存器用于存储当前栈指针，即栈顶地址
  kRSP = 13,
};

Routine::Routine(RoutineThreadEnv* env, bool is_main, RoutineAttr* attr, RoutineFunc func, void* arg) : 
  is_main_(is_main), 
  env_(env), 
  func_(func), 
  arg_(arg),
  stack_buff_(nullptr),
  stack_buff_len_(0), 
  is_start_(false), 
  is_stop_(false) {
	
  RoutineAttr at;
	if (attr) {
    at = *attr;
	}

  // 如果没设置stack_size，则stack的大小为128K
  // 最大为8M
	if (at.stack_size <= 0) {
		at.stack_size = 128 * 1024;
	} else if( at.stack_size > 1024 * 1024 * 8) {
		at.stack_size = 1024 * 1024 * 8;
	}

  // 如果是共享栈的模式，则从共享栈里取的一块内存
	StackMem* stack_mem = NULL;
	
  if (at.share_stack) {
    // 在共享栈池中分配一块栈内存
    stack_mem = at.share_stack->alloc_stack_mem();
		at.stack_size = at.share_stack->stack_size_;
	} else {
    // 否则在堆上开辟一块内存
    // todo 考虑做内存池化
		stack_mem = new StackMem(at.stack_size);
	}

	stack_mem_ = stack_mem;
	ctx_.ss_sp = stack_mem->stack_buffer_;
	ctx_.ss_size = at.stack_size;

	// lp->cEnableSysHook = 0;
  is_share_stack = at.share_stack != nullptr;
  init_ctx();
};

// only __x86_64__
void Routine::init_ctx() {
  auto ctx = &ctx_;
  char* sp = ctx->ss_sp + ctx->ss_size - sizeof(void*);
  sp = (char*)((unsigned long)sp & -16LL);

  memset(ctx->regs, 0, sizeof(ctx->regs));
  void** ret_addr = (void**)(sp);
  *ret_addr = (void*)func_;

  // 记住栈顶位置
  ctx->regs[kRSP] = sp;
  // 记住返回地址
  ctx->regs[kRETAddr] = (char*)func_;
  // 记住第一个参数 
  ctx->regs[kRDI] = (char*)this;
  // 记住第二个参数
  ctx->regs[kRSI] = (char*)0;
}

void Routine::save_stack_to_buff() {
  // 栈底在高地址，栈顶在低地址
	int len = stack_mem_->stack_bp_ - stack_sp_;

	if (stack_buff_) {
		free(stack_buff_);
    stack_buff_ = nullptr;
	}

  // 将栈内容拷贝到缓冲区中
	stack_buff_ = (char*)malloc(len); //malloc buf;
	stack_buff_len_ = len;

	memcpy(stack_buff_, stack_sp_, len);
}

EventLoop* get_thread_eventloop() {
  return get_curr_thread_env()->loop_;
}

