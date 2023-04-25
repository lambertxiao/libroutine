#include <errno.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stack>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "../rt.h"

using namespace std;
struct EndPoint {
  char *ip;
  unsigned short int port;
};

static void SetAddr(const char *pszIP, const unsigned short shPort, struct sockaddr_in &addr) {
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(shPort);
  addr.sin_addr.s_addr = inet_addr(pszIP);
  ;
}

static int iSuccCnt = 0;
static int iFailCnt = 0;
static int iTime = 0;

void AddSuccCnt() {
  int now = time(NULL);
  if (now > iTime) {
    printf("time: %d succ_cnt: %d fail_cnt: %d\n", iTime, iSuccCnt, iFailCnt);
    iTime = now;
    iSuccCnt = 0;
    iFailCnt = 0;
  } else {
    iSuccCnt++;
  }
}
void AddFailCnt() {
  int now = time(NULL);
  if (now > iTime) {
    printf("time: %d succ_cnt %d fail_cnt %d\n", iTime, iSuccCnt, iFailCnt);
    iTime = now;
    iSuccCnt = 0;
    iFailCnt = 0;
  } else {
    iFailCnt++;
  }
}

static void *worker_routine(void *arg) {
  rt_enable_hook_sys();

  EndPoint *endpoint = (EndPoint *)arg;
  char str[8] = "sarlmol";
  char buf[1024 * 16];
  int fd = -1;
  int ret = 0;

  while (true) {
    if (fd < 0) {
      fd = socket(PF_INET, SOCK_STREAM, 0);
      struct sockaddr_in addr;
      SetAddr(endpoint->ip, endpoint->port, addr);
      ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));

      if (errno == EALREADY || errno == EINPROGRESS) {
        struct pollfd pf = {0};
        pf.fd = fd;
        pf.events = (POLLOUT | POLLERR | POLLHUP);

        poll(&pf, 1, 200);
        // check connect
        int error = 0;
        uint32_t socklen = sizeof(error);
        errno = 0;
        ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)&error, &socklen);
        if (ret == -1) {
          // printf("getsockopt ERROR ret %d %d:%s\n", ret, errno,
          // strerror(errno));
          close(fd);
          fd = -1;
          AddFailCnt();
          continue;
        }
        if (error) {
          errno = error;
          // printf("connect ERROR ret %d %d:%s\n", error, errno,
          // strerror(errno));
          close(fd);
          fd = -1;
          AddFailCnt();
          continue;
        }
      }
    }

    ret = write(fd, str, 8);
    if (ret > 0) {
      ret = read(fd, buf, sizeof(buf));
      if (ret <= 0) {
        close(fd);
        fd = -1;
        AddFailCnt();
      } else {
        AddSuccCnt();
      }
    } else {
      close(fd);
      fd = -1;
      AddFailCnt();
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printf("Usage:\n example_echocli [IP] [PORT] [TASK_COUNT]\n");
    return -1;
  }

  EndPoint endpoint;
  endpoint.ip = argv[1];
  endpoint.port = atoi(argv[2]);
  int cnt = atoi(argv[3]);

  for (int i = 0; i < cnt; i++) {
    Routine *rt = 0;
    rt_create(&rt, NULL, worker_routine, &endpoint);
    rt_resume(rt);
  }
  rt_eventloop(rt_get_thread_eventloop(), NULL, NULL);
  exit(0);
}
