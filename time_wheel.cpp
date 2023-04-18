#include "time_wheel.h"
#include "common.h"
#include <stdio.h>

TimeWheel::TimeWheel(int slot_cnt)
    : slot_cnt_(slot_cnt), curr_time_ms_(get_time_ms()), curr_slot_(0) {}

int TimeWheel::add_item(TimeWheelSlotLinkItem* item) {
  // 当前时间小于时间轮的开始时间了，属于异常情况
  if (item->timeout_ms_ < curr_time_ms_) {
    printf("add item to timewheel error, timeout_ms:%lu curr_time:%lu\n",
           item->timeout_ms_, curr_time_ms_);
    return -1;
  }

  uint64_t diff = item->timeout_ms_ - curr_time_ms_;
  if ((int)diff > slot_cnt_) {
    printf("add item to timewheel error, diff:%lu\n", diff);
    return -1;
  }

  int pos = (curr_slot_ + diff) % slot_cnt_;

  if (!slots_[pos]) {
    slots_[pos] = new TimeWheelSlotLink();
  }

  slots_[pos]->add_back(item);
  return 0;
}

void TimeWheel::timeout_takeaway(TimeWheelSlotLink* link) {
  
}

