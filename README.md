# 简介

libroutine其实是受libco的启发而来的，由于不喜欢libco在代码实现层面上的一些问题，决定重新造一个轮子。当然由于本人技术水平有限，部分代码的实现参考libco。

## 实现目标

不侵入业务代码，通过hook掉阻塞的系统调用，从而在一个操作系统线程上实现多协程间的调用

## 实现原理

- dlsym
  
  通过dlsym可以获得系统函数的地址，再加上协程库内部对系统函数的覆写，可以来实现对系统函数的hook

- ctx_swap

  通过修改cpu的各个寄存器，来实现协程间的切换

- epoll + 时间轮

  由于协程库的底层需要将阻塞的操作替换成非阻塞操作，一般需要将关心的事件交给epoll后，然后通过时间轮结构设置超时回调后交出cpu；
  然后通过协程库内部eventloop来决定是执行epoll的事件回调还是时间轮的超时回调。

## todo list

- [x] 抽象eventloop结构
- [x] 抽象时间轮结构
- [x] 协程的创建、启动、切换、销毁
- [x] 条件变量实现
- [ ] 阻塞的系统调用的hook(如poll, read, write等函数)
- [ ] 目前的时间轮采用的数据结构是数组+双向链表的形式，在遍历双向链表的时候效率差，需要换种数据结构
- [ ] 共享栈模式下内存高频malloc，free的问题

