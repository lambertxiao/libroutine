#ifndef LIBROUTINE_SYSCALL_H_
#define LIBROUTINE_SYSCALL_H_

#include <sys/poll.h>
#include <sys/socket.h>

typedef int (*poll_pfn_t)(pollfd fds[], nfds_t nfds, int timeout);
typedef int (*connect_pfn_t)(int socket, const struct sockaddr* address, socklen_t address_len);
typedef ssize_t (*read_pfn_t)(int fd, void* buf, size_t nbyte);
typedef ssize_t (*write_pfn_t)(int fd, const void* buf, size_t nbyte);
typedef int (*fcntl_pfn_t)(int fd, int cmd, ...);

#endif

