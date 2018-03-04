#ifndef	SHIM_SYS_EPOLL_H
#define	SHIM_SYS_EPOLL_H

#ifdef __cplusplus
extern "C" {
#endif

/* include the same file as musl */
#include <sys/types.h> /* IWYU pragma: keep */

#include <fcntl.h>
#include <stdint.h>

#if 0
#define __NEED_sigset_t

#include <bits/alltypes.h>
#endif

#define EPOLL_CLOEXEC O_CLOEXEC
#define EPOLL_NONBLOCK O_NONBLOCK

enum EPOLL_EVENTS { __EPOLL_DUMMY };
#define EPOLLIN 0x001
#define EPOLLPRI 0x002
#define EPOLLOUT 0x004
#define EPOLLRDNORM 0x040
#define EPOLLRDBAND 0x080
#define EPOLLWRNORM 0x100
#define EPOLLWRBAND 0x200
#define EPOLLMSG 0x400
#define EPOLLERR 0x008
#define EPOLLHUP 0x010
#define EPOLLRDHUP 0x2000
#define EPOLLWAKEUP (1U<<29)
#define EPOLLONESHOT (1U<<30)
#define EPOLLET (1U<<31)

#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3

typedef union epoll_data {
	void *ptr;
	int fd;
	uint32_t u32;
	uint64_t u64;
} epoll_data_t;

struct epoll_event {
	uint32_t events;
	epoll_data_t data;
}
#if defined(__amd64__)
__attribute__((packed))
#endif
;

int epoll_create(int /*size*/);
// int epoll_create1(int /*flags*/);
int epoll_ctl(
    int /*fd*/, int /*op*/, int /*fd2*/, struct epoll_event * /*ev*/);
int epoll_wait(
    int /*fd*/, struct epoll_event * /*ev*/, int /*cnt*/, int /*to*/);
#if 0
int epoll_pwait(int, struct epoll_event *, int, int, const sigset_t *);
#endif


#ifdef __cplusplus
}
#endif

#endif /* sys/epoll.h */
