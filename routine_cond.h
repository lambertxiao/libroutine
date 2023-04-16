#ifndef LIBROUTINE_ROUTINE_COND_
#define LIBROUTINE_ROUTINE_COND_

#include <list>
#include "time_wheel.h"
#include "thread_env.h"
#include "common.h"

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


class RoutineCond : public std::list<RoutineCondWaitItem*> {
 public:
  int add_wait_item(RoutineCondWaitItem* item) {
    item->timeitem_.arg = item;
    item->timeitem_.cb = [](TimeWheelSlotLinkItem* timeitem){
      auto waititem = (RoutineCondWaitItem*)timeitem->arg;
      waititem->wait_cb_(waititem->bind_rt_);
    };
   	
    if (item->timeout_ > 0){
      // 设置等待有效期
		  item->timeitem_.timeout_ms = get_time_ms() + item->timeout_;
      int ret = get_curr_thread_env()->loop_->time_wheel_->add_item(&item->timeitem_);
	  	if( ret != 0 ) {
		  	return ret;
		  }
	  }
  
    emplace_back(item); 
    return 0;
  }
};

#endif

