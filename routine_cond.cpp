#include "routine_cond.h"
#include "common.h"

int RoutineCond::wait(RoutineCondWaitItem* item) {
  item->timeitem_.arg_ = item;
  item->timeitem_.cb_ = [](TimeWheelSlotLinkItem* timeitem) {
    auto waititem = (RoutineCondWaitItem*)timeitem->arg_;
    waititem->wait_cb_(waititem->bind_rt_);
  };

  if (item->timeout_ > 0) {
    // 设置等待有效期
    item->timeitem_.timeout_ms_ = get_time_ms() + item->timeout_;
    int ret =
        get_curr_thread_env()->loop_->time_wheel_->add_item(&item->timeitem_);
    if (ret != 0) {
      return ret;
    }
  }

  items_.emplace_back(item);
  return 0;
}

int RoutineCond::signal() {
  if (items_.size() == 0) {
    return 0;
  }
  auto item = items_.front();
  items_.pop_front();
  // 从时间轮上移除
  item->timeitem_.link_.remove(&item->timeitem_);

  // 被唤醒之后的对象并不是马上执行，而是放入当前线程的activeList里，等待执行
  get_curr_thread_env()->loop_->active_list_->emplace_back(&item->timeitem_);
  return 0;
}

int RoutineCond::broadcast() {
  while (true) {
    if (items_.size() == 0) {
      return 0;
    }

    auto item = items_.front();
    items_.pop_front();
    item->timeitem_.link_.remove(&item->timeitem_);
    get_curr_thread_env()->loop_->active_list_->emplace_back(&item->timeitem_);
  }
}
