#ifndef LIBROUTINE_TIMEWHEEL_H_
#define LIBROUTINE_TIMEWHEEL_H_

#include <stdint.h>

class TimeWheelSlotLink;
class TimeWheelSlotLinkItem;

class TimeWheel {
 public:
  // 时间轮槽，每一个槽里是一个双端队列
  TimeWheelSlotLink* slots;
  // 轮上有多少个槽
  int slotCnt;

  // 时间轮的当前时间
  unsigned long long ullStart;
  // 当前停留在哪个槽
  long long llStartIdx;

  int add_item(TimeWheelSlotLinkItem* item);
};

class TimeWheelSlotLinkItem;
typedef void (*TimeCallBack)(TimeWheelSlotLinkItem*);

class TimeWheelSlotLinkItem {
 public:
  enum {
    eMaxTimeout = 40 * 1000  // 40s
  };
  // 前后节点
  TimeWheelSlotLinkItem* pPrev;
  TimeWheelSlotLinkItem* pNext;

  // 指向节点所在链表
  TimeWheelSlotLink* link;

  // 到期时间
  uint64_t timeout_ms;

  // 准备事件和处理事件的函数指针
  // OnPreparePfn_t pfnPrepare;
  // 时间一到，会调用cb(arg)
  TimeCallBack cb; 
  void* arg; // arg实际上会是回调的routine

  // 是否超时
  bool bTimeout;
};

class TimeWheelSlotLink {
  TimeWheelSlotLinkItem* head;
  TimeWheelSlotLinkItem* tail;
};

#endif
