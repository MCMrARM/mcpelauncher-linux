// #include <sys/timerfd.h>
// #undef read
// #undef close
//
// #include <sys/event.h>
//
// #include <pthread.h>
// // #include <pthread.h>
//
// #include <errno.h>
// #include <signal.h>
// #include <stdbool.h>
// #include <stdint.h>
// #include <stdlib.h>
// #include <string.h>
// #include <signal.h>
// #include <time.h>
//
// struct timerfd_context {
// 	int fd;
// 	pthread_t worker;
// 	timer_t timer;
// 	int flags;
// 	struct timerfd_context *next;
// };
//
// static struct timerfd_context *timerfd_contexts;
// pthread_mutex_t timerfd_context_mtx = PTHREAD_MUTEX_INITIALIZER;
//
// struct timerfd_context *
// get_timerfd_context(int fd, bool create_new)
// {
// 	for (struct timerfd_context *ctx = timerfd_contexts; ctx;
// 	     ctx = ctx->next) {
// 		if (fd == ctx->fd) {
// 			return ctx;
// 		}
// 	}
//
// 	if (create_new) {
// 		struct timerfd_context *new_ctx =
// 		    calloc(1, sizeof(struct timerfd_context));
// 		if (!new_ctx) {
// 			return NULL;
// 		}
// 		new_ctx->fd = -1;
// 		new_ctx->next = timerfd_contexts;
// 		timerfd_contexts = new_ctx;
// 		return new_ctx;
// 	}
//
// 	return NULL;
// }
//
// static void *
// worker_function(void *arg)
// {
// 	struct timerfd_context *ctx = arg;
//
// 	siginfo_t info;
// 	sigset_t rt_set;
// 	sigset_t block_set;
//
// 	sigemptyset(&rt_set);
// 	sigaddset(&rt_set, SIGKILL);
// 	// sigaddset(&rt_set, SIGRTMIN + 1);
//
// 	sigfillset(&block_set);
//
// 	(void)pthread_sigmask(SIG_BLOCK, &block_set, NULL);
//
// 	struct kevent kev;
// 	uint64_t   tid;
// 	pthread_t         self;
// 	self = pthread_self();
// 	pthread_threadid_np(self, &tid);
// 	EV_SET(&kev, 0, EVFILT_USER, 0, NOTE_TRIGGER, 0,
// 	    (void *)(intptr_t)&tid);
// 	(void)kevent(ctx->fd, &kev, 1, NULL, 0, NULL);
//
// 	for (;;) {
// 		// if (sigwaitinfo(&rt_set, &info) != SIGKILL) {
// 			// break;
// 		// }
// 		EV_SET(&kev, 0, EVFILT_USER, 0, NOTE_TRIGGER, 0,
// 		    (void *)(intptr_t)timer_getoverrun(ctx->timer));
// 		(void)kevent(ctx->fd, &kev, 1, NULL, 0, NULL);
// 	}
//
// 	return NULL;
// }
//
// static int
// timerfd_create_impl(int clockid, int flags)
// {
// 	if (clockid != CLOCK_MONOTONIC && clockid != CLOCK_REALTIME) {
// 		return EINVAL;
// 	}
//
// 	if (flags & ~(TFD_CLOEXEC | TFD_NONBLOCK)) {
// 		return EINVAL;
// 	}
//
// 	struct timerfd_context *ctx = get_timerfd_context(-1, true);
// 	if (!ctx) {
// 		errno = ENOMEM;
// 		return -1;
// 	}
//
// 	ctx->fd = kqueue();
// 	if (ctx->fd < 0) {
// 		return -1;
// 	}
//
// 	ctx->flags = flags;
//
// 	struct kevent kev;
// 	EV_SET(&kev, 0, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, 0);
// 	if (kevent(ctx->fd, &kev, 1, NULL, 0, NULL) < 0) {
// 		close(ctx->fd);
// 		ctx->fd = -1;
// 		return -1;
// 	}
//
// 	if (pthread_create(&ctx->worker, NULL, worker_function, ctx) < 0) {
// 		close(ctx->fd);
// 		ctx->fd = -1;
// 		return -1;
// 	}
//
// 	int ret = kevent(ctx->fd, NULL, 0, &kev, 1, NULL);
// 	if (ret < 0) {
// 		pthread_kill(ctx->worker, SIGKILL);
// 		pthread_join(ctx->worker, NULL);
// 		close(ctx->fd);
// 		ctx->fd = -1;
// 		return -1;
// 	}
//
// 	int tid = (int)(intptr_t)kev.udata;
//
// 	struct sigevent sigev = {.sigev_notify = SIGEV_THREAD,
// 	    .sigev_signo = SIGKILL,
// 	    ._sigev_un._tid = tid};
//
// 	if (timer_create(clockid, &sigev, &ctx->timer) < 0) {
// 		pthread_kill(ctx->worker, SIGKILL);
// 		pthread_join(ctx->worker, NULL);
// 		close(ctx->fd);
// 		ctx->fd = -1;
// 		return -1;
// 	}
//
// 	return ctx->fd;
// }
//
// int
// timerfd_create(int clockid, int flags)
// {
// 	pthread_mutex_lock(&timerfd_context_mtx);
// 	int ret = timerfd_create_impl(clockid, flags);
// 	pthread_mutex_unlock(&timerfd_context_mtx);
// 	return ret;
// }
//
// static int
// timerfd_settime_impl(
//     int fd, int flags, const struct itimerspec *new, struct itimerspec *old)
// {
// 	struct timerfd_context *ctx = get_timerfd_context(fd, false);
// 	if (!ctx) {
// 		errno = EINVAL;
// 		return -1;
// 	}
//
// 	if (flags & ~(TFD_TIMER_ABSTIME)) {
// 		errno = EINVAL;
// 		return -1;
// 	}
//
// 	return timer_settime(ctx->timer,
// 	    (flags & TFD_TIMER_ABSTIME) ? TIMER_ABSTIME : 0, new, old);
// }
//
// int
// timerfd_settime(
//     int fd, int flags, const struct itimerspec *new, struct itimerspec *old)
// {
// 	pthread_mutex_lock(&timerfd_context_mtx);
// 	int ret = timerfd_settime_impl(fd, flags, new, old);
// 	pthread_mutex_unlock(&timerfd_context_mtx);
// 	return ret;
// }
//
// #if 0
// int timerfd_gettime(int fd, struct itimerspec *cur)
// {
// 	return syscall(SYS_timerfd_gettime, fd, cur);
// }
// #endif
//
// ssize_t
// timerfd_read(struct timerfd_context *ctx, void *buf, size_t nbytes)
// {
// 	int fd = ctx->fd;
// 	int flags = ctx->flags;
// 	pthread_mutex_unlock(&timerfd_context_mtx);
//
// 	if (nbytes < sizeof(uint64_t)) {
// 		errno = EINVAL;
// 		return -1;
// 	}
//
// 	struct timespec timeout = {0, 0};
// 	struct kevent kev;
// 	int ret = kevent(
// 	    fd, NULL, 0, &kev, 1, (flags & TFD_NONBLOCK) ? &timeout : NULL);
// 	if (ret == -1) {
// 		return -1;
// 	}
//
// 	if (ret == 0) {
// 		errno = EAGAIN;
// 		return -1;
// 	}
//
// 	uint64_t nr_expired = 1 + (uint64_t)kev.udata;
// 	memcpy(buf, &nr_expired, sizeof(uint64_t));
//
// 	return sizeof(uint64_t);
// }
//
// int
// timerfd_close(struct timerfd_context *ctx)
// {
// 	timer_delete(ctx->timer);
// 	pthread_kill(ctx->worker, SIGKILL);
// 	pthread_join(ctx->worker, NULL);
// 	int ret = close(ctx->fd);
// 	ctx->fd = -1;
// 	return ret;
// }
