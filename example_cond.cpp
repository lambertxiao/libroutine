#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include "rt.h"

using namespace std;
struct stTask_t {
  int id;
};

struct stEnv_t {
  RoutineCond* cond;
  queue<stTask_t*> task_queue;
};

void* Producer(void* args) {
  printf("exec producer function");
  // co_enable_hook_sys();
  stEnv_t* env = (stEnv_t*)args;
  int id = 0;
  while (true) {
    stTask_t* task = (stTask_t*)calloc(1, sizeof(stTask_t));
    task->id = id++;
    env->task_queue.push(task);
    printf("%s:%d produce task %d\n", __func__, __LINE__, task->id);
    rt_cond_signal(env->cond);
    // poll此时已经被co_enable_hook_sys hook掉了
    poll(NULL, 0, 1000);
  }
  return NULL;
}

void* Consumer(void* args) {
  printf("exec consumer function");
  // co_enable_hook_sys();
  stEnv_t* env = (stEnv_t*)args;
  while (true) {
    if (env->task_queue.empty()) {
      // 阻塞住，等待唤醒
      rt_cond_wait(env->cond, -1);
      continue;
    }
    stTask_t* task = env->task_queue.front();
    env->task_queue.pop();
    printf("%s:%d consume task %d\n", __func__, __LINE__, task->id);
    free(task);
  }
  return NULL;
}

int main() {
  stEnv_t* env = new stEnv_t;
  env->cond = rt_cond_alloc();

  printf("start consumer routine\n");
  Routine* consumer_routine;
  rt_create(&consumer_routine, NULL, Consumer, env);
  rt_resume(consumer_routine);

  printf("start producer routine\n");
  Routine* producer_routine;
  rt_create(&producer_routine, NULL, Producer, env);
  rt_resume(producer_routine);

  printf("run eventloop");
  rt_eventloop(get_thread_eventloop(), NULL, NULL);
  return 0;
}
