#include "time_wheel.h"
#include "common.h"
#include <stdio.h>

TimeWheel::TimeWheel(int slot_cnt)
    : slot_cnt_(slot_cnt), curr_time_ms_(get_time_ms()), curr_slot_(0), slots_(slot_cnt, new TimeWheelSlotLink()) {}

int TimeWheel::add_item(TimeWheelSlotItem* item) {
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

  auto link = slots_.at(pos);
  link->add_back(item);
  return 0;
}

void TimeWheel::time_forward(TimeWheelSlotLink* timeout_link, uint64_t now) {
  if (now < curr_time_ms_) {
    return;
  }
  // 过去了多少毫秒
  int cnt = now - curr_time_ms_ + 1;
  if (cnt > slot_cnt_) {
    cnt = slot_cnt_;
  }

  for (int i = 0; i < cnt; i++) {
    int pos = (curr_slot_ + i) % slot_cnt_;
    auto link = slots_[pos];
    auto curr = link->head->next;
    while (curr != link->tail) {
      if (curr->timeout_ms_ > now) {
        continue;
      }

      auto node = curr;
      curr = curr->next;
      
      // 先跟原本的链断开
      link->delete_node(node);
      timeout_link->add_back(node);
    }
  }

  // 时间往前走
  curr_time_ms_ = now;
  curr_slot_ += cnt - 1;
}

