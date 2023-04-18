#ifndef LIBROUTINE_TIMEWHEEL_H_
#define LIBROUTINE_TIMEWHEEL_H_

#include <stdint.h>
#include <list>
#include <vector>
#include "double_list.h"

class TimeWheelSlotLinkItem;

// 时间轮格子上的链表，需要频繁增删，因此用list
class TimeWheelSlotLink : public DoublyLinkedList<TimeWheelSlotLinkItem> {};

class TimeWheel {
 public:
  // 时间轮槽，每一个槽里是一个双端队列
  // slots的大小是初始化决定的，不会扩所容，因此可以用vector实现
  std::vector<TimeWheelSlotLink*> slots_;
  // 轮上有多少个槽，一个槽1ms
  int slot_cnt_;

  // 时间轮的当前时间
  uint64_t curr_time_ms_;
  // 当前停留在哪个槽
  uint64_t curr_slot_;

  TimeWheel(int slot_cnt);
  int add_item(TimeWheelSlotLinkItem* item);
  void timeout_takeaway(TimeWheelSlotLink* link);
};

typedef void (*TimeCallBack)(TimeWheelSlotLinkItem*);

struct TimeWheelSlotLinkItem : public DoublyLinkedListNode<TimeWheelSlotLinkItem> {
  enum {
    eMaxTimeout = 40 * 1000  // 40s
  };
  // 前后节点
  // TimeWheelSlotLinkItem* pPrev;
  // TimeWheelSlotLinkItem* pNext;

  // 指向节点所在链表
  // TimeWheelSlotLink link_;

  // 到期时间
  uint64_t timeout_ms_;

  // 准备事件和处理事件的函数指针
  // OnPreparePfn_t pfnPrepare;
  // 时间一到，会调用cb(arg)
  TimeCallBack cb_pre_;
  TimeCallBack cb_;
  void* arg_;  // arg实际上会是回调的routine

  // 是否超时
  bool is_timeout_;
};

#endif
