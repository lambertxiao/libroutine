#ifndef LIBROUTINE_TIMEWHEEL_H_
#define LIBROUTINE_TIMEWHEEL_H_

class TWSlotLink;

class TimeWheel {
 public:
  // 时间轮槽，每一个槽里是一个双端队列
	TWSlotLink* slots;
  // 轮上有多少个槽
	int slotCnt;

  // 时间轮的当前时间
	unsigned long long ullStart;
  // 当前停留在哪个槽
	long long llStartIdx;
};

class TWSlotLinkItem {
	enum
	{
		eMaxTimeout = 40 * 1000 //40s
	};
  // 前后节点
	TWSlotLinkItem *pPrev;
	TWSlotLinkItem *pNext;
  
  // 指向节点所在链表
	TWSlotLink *link;

  // 到期时间
	// unsigned long long ullExpireTime;

  // 准备事件和处理事件的函数指针
	// OnPreparePfn_t pfnPrepare;
	// OnProcessPfn_t pfnProcess;

	// void *pArg; // routine 
  // 是否超时
	bool bTimeout;
};

class TWSlotLink {
  TWSlotLinkItem* head;
  TWSlotLinkItem* tail;
};

#endif

