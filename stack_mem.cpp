#include "stack_mem.h"
#include <memory>

StackMem::StackMem(int stack_size)
    : stack_size_(stack_size),
      occupy_rt_(nullptr),
      stack_buffer_((char*)malloc(stack_size)) {

  // 栈由高地址往低地址增长
  stack_bp_ = stack_buffer_ + stack_size;
}

StackMem* ShareStack::alloc_stack_mem() {
  int idx = alloc_idx_ % count_;
  alloc_idx_++;

  return stack_array_[idx];
}
