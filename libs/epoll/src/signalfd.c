#include <sys/signalfd.h>
#undef read
#undef close

#include <sys/event.h>
#include <sys/types.h>

#include <sys/event.h>

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define _SIG_MAXSIG 128

struct signalfd_context {
	int fd;
	int flags;
	struct signalfd_context *next;
};

static struct signalfd_context *signalfd_contexts;
pthread_mutex_t signalfd_context_mtx = PTHREAD_MUTEX_INITIALIZER;

struct signalfd_context *
get_signalfd_context(int fd, bool create_new)
{
	for (struct signalfd_context *ctx = signalfd_contexts; ctx;
	     ctx = ctx->next) {
		if (fd == ctx->fd) {
			return ctx;
		}
	}

	if (create_new) {
		struct signalfd_context *new_ctx =
		    calloc(1, sizeof(struct signalfd_context));
		if (!new_ctx) {
			return NULL;
		}
		new_ctx->fd = -1;
		new_ctx->next = signalfd_contexts;
		signalfd_contexts = new_ctx;
		return new_ctx;
	}

	return NULL;
}

static int
signalfd_impl(int fd, const sigset_t *sigs, int flags)
{
	if (fd != -1) {
		errno = EINVAL;
		return -1;
	}

	if (flags & ~(SFD_NONBLOCK | SFD_CLOEXEC)) {
		errno = EINVAL;
		return -1;
	}

	struct signalfd_context *ctx = get_signalfd_context(-1, true);
	if (!ctx) {
		errno = EMFILE;
		return -1;
	}

	ctx->fd = kqueue();
	if (ctx->fd < 0) {
		return -1;
	}

	ctx->flags = flags;

	struct kevent kevs[_SIG_MAXSIG];
	int n = 0;

	for (int i = 1; i <= _SIG_MAXSIG; ++i) {
		if (sigismember(sigs, i)) {
			EV_SET(&kevs[n++], i, EVFILT_SIGNAL, EV_ADD, 0, 0, 0);
		}
	}

	int ret = kevent(ctx->fd, kevs, n, NULL, 0, NULL);
	if (ret < 0) {
		close(ctx->fd);
		ctx->fd = -1;
		return -1;
	}

	return ctx->fd;
}

int
signalfd(int fd, const sigset_t *sigs, int flags)
{
	pthread_mutex_lock(&signalfd_context_mtx);
	int ret = signalfd_impl(fd, sigs, flags);
	pthread_mutex_unlock(&signalfd_context_mtx);
	return ret;
}

ssize_t
signalfd_read(struct signalfd_context *ctx, void *buf, size_t nbytes)
{
	int fd = ctx->fd;
	int flags = ctx->flags;
	pthread_mutex_unlock(&signalfd_context_mtx);

	// TODO(jan): fix this to read multiple signals
	if (nbytes != sizeof(struct signalfd_siginfo)) {
		errno = EINVAL;
		return -1;
	}

	struct timespec timeout = {0, 0};
	struct kevent kev;
	int ret = kevent(
	    fd, NULL, 0, &kev, 1, (flags & SFD_NONBLOCK) ? &timeout : NULL);
	if (ret == -1) {
		return -1;
	}
	if (ret == 0) {
		errno = EAGAIN;
		return -1;
	}

	memset(buf, '\0', nbytes);
	struct signalfd_siginfo *sig_buf = buf;
	sig_buf->ssi_signo = (uint32_t)kev.ident;
	return (ssize_t)nbytes;
}

int
signalfd_close(struct signalfd_context *ctx)
{
	int ret = close(ctx->fd);
	ctx->fd = -1;
	return ret;
}
