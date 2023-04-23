#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <stack>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#include "../rt.h"

using namespace std;
struct Worker {
  Routine *rt;
  int fd;
};

static stack<Worker *> workers;
static int g_listen_fd = -1;
static int SetNonBlock(int iSock) {
  int iFlags;

  iFlags = fcntl(iSock, F_GETFL, 0);
  iFlags |= O_NONBLOCK;
  iFlags |= O_NDELAY;
  int ret = fcntl(iSock, F_SETFL, iFlags);
  return ret;
}

static void *workerRoutineFunc(void *arg) {
  rt_enable_hook_sys();

  Worker *w = (Worker *)arg;
  char buf[1024 * 16];

  while (true) {
    // 没有设置fd的时候，交出cpu
    if (w->fd == -1) {
      workers.push(w);
      rt_yield_ct();
      continue;
    }
    int fd = w->fd;
    w->fd = -1;

    while (true) {
      struct pollfd pf = {0};
      pf.fd = fd;
      pf.events = (POLLIN | POLLERR | POLLHUP);
      poll(&pf, 1, 1000);

      int ret = read(fd, buf, sizeof(buf));
      if (ret > 0) {
        ret = write(fd, buf, ret);
      }
      if (ret > 0 || (-1 == ret && EAGAIN == errno)) {
        continue;
      }
      close(fd);
      break;
    }
  }
  return 0;
}

static void *accept_routine(void *) {
  rt_enable_hook_sys();
  printf("start accept_routine\n");

  while (true) {
    if (workers.empty()) {
      printf("workers is empty, wait worker\n");  // sleep
      poll(NULL, 0, 1000);
      continue;
    }

    struct sockaddr_in addr;  // maybe sockaddr_un;
    memset(&addr, 0, sizeof(addr));
    socklen_t len = sizeof(addr);

    int fd = accept(g_listen_fd, (struct sockaddr *)&addr, &len);
    if (fd < 0) {
      struct pollfd pf = {0};
      pf.fd = g_listen_fd;
      pf.events = (POLLIN | POLLERR | POLLHUP);
      poll(&pf, 1, 1000);
      continue;
    }

    if (workers.empty()) {
      close(fd);
      continue;
    }

    printf("accept new connection, fd:%d\n", fd);
    SetNonBlock(fd);
    Worker *w = workers.top();
    w->fd = fd;
    workers.pop();
    rt_resume(w->rt);
  }
  return 0;
}

static void SetAddr(const char *pszIP, const unsigned short shPort, struct sockaddr_in &addr) {
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(shPort);
  addr.sin_addr.s_addr = inet_addr(pszIP);
}

static int CreateTcpSocket(const unsigned short shPort /* = 0 */, const char *pszIP /* = "*" */, bool bReuse /* = false */) {
  int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd >= 0) {
    if (shPort != 0) {
      if (bReuse) {
        int nReuseAddr = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &nReuseAddr, sizeof(nReuseAddr));
      }
      struct sockaddr_in addr;
      SetAddr(pszIP, shPort, addr);
      int ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
      if (ret != 0) {
        close(fd);
        return -1;
      }
    }
  }
  return fd;
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf(
        "Usage:\n"
        "example_echosvr [IP] [PORT] [TASK_COUNT]\n"
        "example_echosvr [IP] [PORT] [TASK_COUNT] -d   # "
        "daemonize mode\n");
    return -1;
  }
  const char *ip = argv[1];
  int port = atoi(argv[2]);
  int cnt = atoi(argv[3]);
  bool deamonize = argc >= 6 && strcmp(argv[4], "-d") == 0;

  g_listen_fd = CreateTcpSocket(port, ip, true);
  listen(g_listen_fd, 1024);
  if (g_listen_fd == -1) {
    printf("Port %d is in use\n", port);
    return -1;
  }

  printf("listen at %d %s:%d\n", g_listen_fd, ip, port);
  SetNonBlock(g_listen_fd);

  // 创建工作协程
  for (int i = 0; i < cnt; i++) {
    Worker *w = (Worker *)calloc(1, sizeof(Worker));
    w->fd = -1;
    workers.push(w);

    rt_create(&(w->rt), NULL, workerRoutineFunc, w);
    rt_resume(w->rt);
  }

  // 创建一个接受协程
  Routine *accept_rt = NULL;
  rt_create(&accept_rt, NULL, accept_routine, 0);
  printf("create accept_routine %p\n", accept_rt);
  rt_resume(accept_rt);

  rt_eventloop(rt_get_thread_eventloop(), 0, 0);
  exit(0);
  if (!deamonize) wait(NULL);
  return 0;
}
