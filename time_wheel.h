#ifndef LIBROUTINE_TIMEWHEEL_H_
#define LIBROUTINE_TIMEWHEEL_H_

#include <stdint.h>
#include <list>
#include <vector>
#include "doubly_list.h"
#include <sys/epoll.h>

class TimeWheelSlotItem;

// 时间轮格子上的链表，需要频繁增删，因此用list
class TimeWheelSlotLink : public DoublyLinkedList<TimeWheelSlotItem> {};

class TimeWheel {
 public:
  // 时间轮槽，每一个槽里是一个双端队列
  // slots的大小是初始化决定的，不会扩缩容，因此可以用vector实现
  std::vector<TimeWheelSlotLink*> slots_;
  // 轮上有多少个槽，一个槽1ms
  int slot_cnt_;

  // 时间轮的当前时间
  uint64_t curr_time_ms_;
  // 当前停留在哪个槽
  uint64_t curr_slot_;

  TimeWheel(int slot_cnt);
  int add_item(TimeWheelSlotItem* item);
  void remove_item(TimeWheelSlotItem* item);
  // 将时间轮前进到当前时间，同时将超时事件取出，放到link上
  void time_forward(TimeWheelSlotLink* link, uint64_t now);
};

typedef void (*EventCallBack)(TimeWheelSlotItem*);
typedef void (*EventPreCallBack)(TimeWheelSlotItem*, epoll_event ev);

struct TimeWheelSlotItem : public DoublyLinkedListNode<TimeWheelSlotItem> {
  enum {
    eMaxTimeout = 40 * 1000  // 40s
  };

  // 到期时间
  uint64_t timeout_ms_;

  // 当该item是某个pollGroup中的成员时，该cb存在, 这个函数的调用必须早于pollGroup的cb的调用
  EventPreCallBack group_member_cb_;
  EventCallBack cb_;
  void* arg_;  // arg实际上会是回调的routine

  // 是否超时
  bool is_timeout_;
};

#endif
