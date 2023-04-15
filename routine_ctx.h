#ifndef LIBROUTINE_ROUTINE_CTX_H_
#define LIBROUTINE_ROUTINE_CTX_H_

#include <stdlib.h>

// copy from libco
struct RoutineCtx {
#if defined(__i386__)
  void *regs[8];
#else
  void *regs[14];
#endif
  size_t ss_size;
  char *ss_sp;
};
#endif
