#include "stack_mem.h"
#include <memory>

StackMem::StackMem(int stack_size)
    : stack_size_(stack_size),
      occupy_rt_(nullptr),
      stack_buffer_((char*)malloc(stack_size)) {

  // 栈由高地址往低地址增长
  stack_bp_ = stack_buffer_ + stack_size;
}

ShareStack::ShareStack(int count, uint64_t stack_size)
    : count_(count), stack_size_(stack_size) {
  stack_array_ = (StackMem**)malloc(count * sizeof(StackMem));
  for (int i = 0; i < count; i++) {
    auto sm = new StackMem(stack_size);
    stack_array_[i] = sm;
  }
}

ShareStack::~ShareStack() {
  for (int i = 0; i < count_; i++) {
    delete stack_array_[i];
  }
  delete stack_array_;
}

StackMem* ShareStack::alloc_stack_mem() {
  int idx = alloc_idx_ % count_;
  alloc_idx_++;

  return stack_array_[idx];
}
