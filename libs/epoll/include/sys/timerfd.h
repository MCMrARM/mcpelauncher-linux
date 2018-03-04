#ifndef SHIM_SYS_TIMERFD_H
#define SHIM_SYS_TIMERFD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fcntl.h>
#include <time.h>

#define TFD_NONBLOCK O_NONBLOCK
#define TFD_CLOEXEC O_CLOEXEC

#define TFD_TIMER_ABSTIME 1

// struct itimerspec;

// int timerfd_create(int /*clockid*/, int /*flags*/);
// int timerfd_settime(int /*fd*/, int /*flags*/,
    // const struct itimerspec * /*new*/, struct itimerspec * /*old*/);
#if 0
int timerfd_gettime(int, struct itimerspec *);
#endif

#ifndef SHIM_SYS_SHIM_HELPERS
#define SHIM_SYS_SHIM_HELPERS
#include <unistd.h> /* IWYU pragma: keep */

extern int epoll_shim_close(int /*fd*/);
extern ssize_t epoll_shim_read(int /*fd*/, void * /*buf*/, size_t /*nbytes*/);
#define read epoll_shim_read
#define close epoll_shim_close
#endif

#ifdef __cplusplus
}
#endif

#endif
