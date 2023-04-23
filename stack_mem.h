#ifndef LIBROUTINE_STACKMEM_H_
#define LIBROUTINE_STACKMEM_H_

#include <stdint.h>

class Routine;
// 协程栈空间
class StackMem {
 public:
  // 栈大小
  int stack_size_;
  // 指向使用该栈内存的协程
  Routine* occupy_rt_;
  // 起始地址，栈的底部
  char* stack_buffer_;
  // 栈底部地址+栈大小
  char* stack_bp_;

  StackMem(int stack_size);
};

// 在开启共享栈的情况下，持有着所有已经分配的栈空间
class ShareStack {
 public:
  uint32_t alloc_idx_;
  int count_;
  uint64_t stack_size_;
  StackMem** stack_array_;

  ShareStack(int count, uint64_t stack_size_);
  ~ShareStack();

  StackMem* alloc_stack_mem();
};

#endif
