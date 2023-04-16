#ifndef LIBROUTINE_ROUTINE_COND_
#define LIBROUTINE_ROUTINE_COND_

#include <list>
#include "time_wheel.h"
#include "thread_env.h"

typedef void (*RoutineCallBack)(Routine*);

class RoutineCond;
class RoutineCondWaitItem {
 public:
  // RoutineCondWaitItem* prev_;
	// RoutineCondWaitItem* next_;
	RoutineCond* cond_;
  Routine* bind_rt_; // 绑定的协程，是指向的协程正在进行等待
  RoutineCallBack wait_cb_;
  int timeout_;
  TimeWheelSlotLinkItem timeitem_;
};

class RoutineCond {
 public:
  std::list<RoutineCondWaitItem*> items_;
  int wait(RoutineCondWaitItem* item);
  int signal();
  int broadcast();
};

#endif

