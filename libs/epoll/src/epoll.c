#include <sys/epoll.h>

#include <sys/event.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <time.h>

#define EV_FORCEONESHOT 0x0100

int
epoll_create(int size)
{
	return kqueue();
}

// int
// epoll_create1(int flags)
// {
// 	if (flags != EPOLL_CLOEXEC) {
// 		errno = EINVAL;
// 		return -1;
// 	}
//
// 	return kqueue();
// }

static int poll_fd = -1;
static int poll_epoll_fd = -1;
static void *poll_ptr;

#define KEY_BITS (20)
#define VAL_BITS (32 - KEY_BITS)
static int
kqueue_save_state(int kq, uint32_t key, uint16_t val)
{
	struct kevent kev[VAL_BITS * 2];
	int n = 0;
	int i;
	int oe, e;

	if ((key & ~(((uint32_t)1 << KEY_BITS) - 1)) ||
	    (val & ~(((uint16_t)1 << VAL_BITS) - 1))) {
		return (-EINVAL);
	}

	for (i = 0; i < VAL_BITS; ++i) {
		uint32_t info_bit = (uint32_t)1 << i;
		uint32_t kev_key = key | (info_bit << KEY_BITS);
		EV_SET(&kev[n], kev_key, EVFILT_USER, EV_ADD, 0, 0, 0);
		++n;
		if (!(val & info_bit)) {
			EV_SET(&kev[n], kev_key, EVFILT_USER, /**/
			    EV_DELETE, 0, 0, 0);
			++n;
		}
	}

	oe = errno;
	if ((n = kevent(kq, kev, n, NULL, 0, NULL)) < 0) {
		e = errno;
		errno = oe;
		return (-e);
	}

	return (0);
}

static int
kqueue_load_state(int kq, uint32_t key, uint16_t *val)
{
	struct kevent kev[VAL_BITS];
	int n = 0;
	int i;
	uint16_t nval = 0;
	int oe, e;

	if ((key & ~(((uint32_t)1 << KEY_BITS) - 1))) {
		return (-EINVAL);
	}

	for (i = 0; i < VAL_BITS; ++i) {
		uint32_t info_bit = (uint32_t)1 << i;
		uint32_t kev_key = key | (info_bit << KEY_BITS);
		EV_SET(&kev[i], kev_key, EVFILT_USER, EV_RECEIPT, 0, 0, 0);
	}

	oe = errno;
	if ((n = kevent(kq, kev, VAL_BITS, kev, VAL_BITS, NULL)) < 0) {
		e = errno;
		errno = oe;
		return (-e);
	}

	for (i = 0; i < n; ++i) {
		if (!(kev[i].flags & EV_ERROR)) {
			return (-EINVAL);
		}

		if (kev[i].data == 0) {
			nval |= (uint32_t)1 << i;
		} else if (kev[i].data != ENOENT) {
			return (-EINVAL);
		}
	}

	*val = nval;

	return (0);
}

#define KQUEUE_STATE_REGISTERED 0x1u
#define KQUEUE_STATE_EPOLLIN 0x2u
#define KQUEUE_STATE_EPOLLOUT 0x4u
#define KQUEUE_STATE_EPOLLRDHUP 0x8u
#define KQUEUE_STATE_NYCSS 0x10u
#define KQUEUE_STATE_ISFIFO 0x20u
#define KQUEUE_STATE_ISSOCK 0x40u

static int
is_not_yet_connected_stream_socket(int s)
{
	int type;
	socklen_t length = sizeof(int);

	if (getsockopt(s, SOL_SOCKET, SO_TYPE, &type, &length) == 0 &&
	    (type == SOCK_STREAM || type == SOCK_SEQPACKET)) {
		struct sockaddr name;
		socklen_t namelen = 0;
		if (getpeername(s, &name, &namelen) < 0 && errno == ENOTCONN) {
			return 1;
		}
	}

	return 0;
}

int
epoll_ctl(int fd, int op, int fd2, struct epoll_event *ev)
{
	struct kevent kev[2];
	uint16_t flags;
	int e;

	if ((!ev && op != EPOLL_CTL_DEL) ||
	    (ev &&
		((ev->events &
		    ~(EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLRDHUP | EPOLLERR))
		    /* the user should really set one of EPOLLIN or EPOLLOUT
		     * so that EPOLLHUP and EPOLLERR work. Don't make this a
		     * hard error for now, though. */
		    /* || !(ev->events & (EPOLLIN | EPOLLOUT)) */))) {
		errno = EINVAL;
		return (-1);
	}

	if (fd2 < 0 || ((uint32_t)fd2 & ~(((uint32_t)1 << KEY_BITS) - 1))) {
		errno = EBADF;
		return (-1);
	}

	if ((e = kqueue_load_state(fd, fd2, &flags)) < 0) {
		errno = e;
		return (-1);
	}

	if (op == EPOLL_CTL_ADD) {
		if (flags & KQUEUE_STATE_REGISTERED) {
			errno = EEXIST;
			return (-1);
		}

		EV_SET(&kev[0], fd2, EVFILT_READ,
		    EV_ADD | (ev->events & EPOLLIN ? 0 : EV_DISABLE), 0, 0,
		    ev->data.ptr);
		EV_SET(&kev[1], fd2, EVFILT_WRITE,
		    EV_ADD | (ev->events & EPOLLOUT ? 0 : EV_DISABLE), 0, 0,
		    ev->data.ptr);

		flags = KQUEUE_STATE_REGISTERED;

#define SET_FLAG(flag)                                                        \
	do {                                                                  \
		if (ev->events & (flag)) {                                    \
			flags |= KQUEUE_STATE_##flag;                         \
		}                                                             \
	} while (0)

		SET_FLAG(EPOLLIN);
		SET_FLAG(EPOLLOUT);
		SET_FLAG(EPOLLRDHUP);

#undef SET_FLAG

	} else if (op == EPOLL_CTL_DEL) {
		if (poll_fd == fd2 && fd == poll_epoll_fd) {
			poll_fd = -1;
			poll_epoll_fd = -1;
			poll_ptr = NULL;
			return 0;
		}

		if (!(flags & KQUEUE_STATE_REGISTERED)) {
			errno = ENOENT;
			return (-1);
		}

		EV_SET(&kev[0], fd2, EVFILT_READ, EV_DELETE, 0, 0, 0);
		EV_SET(&kev[1], fd2, EVFILT_WRITE, EV_DELETE, 0, 0, 0);

		flags = 0;
	} else if (op == EPOLL_CTL_MOD) {
		if (!(flags & KQUEUE_STATE_REGISTERED)) {
			errno = ENOENT;
			return (-1);
		}

		EV_SET(&kev[0], fd2, EVFILT_READ,
		    ev->events & EPOLLIN ? EV_ENABLE : EV_DISABLE, 0, 0,
		    ev->data.ptr);
		EV_SET(&kev[1], fd2, EVFILT_WRITE,
		    ev->events & EPOLLOUT ? EV_ENABLE : EV_DISABLE, 0, 0,
		    ev->data.ptr);

#define SET_FLAG(flag)                                                        \
	do {                                                                  \
		if (ev->events & (flag)) {                                    \
			flags |= KQUEUE_STATE_##flag;                         \
		} else {                                                      \
			flags &= ~KQUEUE_STATE_##flag;                        \
		}                                                             \
	} while (0)

		SET_FLAG(EPOLLIN);
		SET_FLAG(EPOLLOUT);
		SET_FLAG(EPOLLRDHUP);

#undef SET_FLAG

	} else {
		errno = EINVAL;
		return (-1);
	}

	for (int i = 0; i < 2; ++i) {
		kev[i].flags |= EV_RECEIPT;
	}

	int ret = kevent(fd, kev, 2, kev, 2, NULL);
	if (ret < 0) {
		return -1;
	}

	if (ret != 2) {
		errno = EINVAL;
		return -1;
	}

	for (int i = 0; i < 2; ++i) {
		if (!(kev[i].flags & EV_ERROR)) {
			errno = EINVAL;
			return -1;
		}

		if (kev[i].data == ENODEV && poll_fd < 0) {
			poll_fd = fd2;
			poll_epoll_fd = fd;
			poll_ptr = ev->data.ptr;
			return 0;
		}

		/* ignore EVFILT_WRITE registration EINVAL errors (some fd
		 * types such as kqueues themselves don't support it) */
		if (i == 1 && kev[i].data == EINVAL) {
			continue;
		}

		if (kev[i].data != 0) {
			errno = kev[i].data;
			return -1;
		}
	}

	if (op != EPOLL_CTL_DEL && is_not_yet_connected_stream_socket(fd2)) {
		EV_SET(&kev[0], fd2, EVFILT_READ, EV_ENABLE | EV_FORCEONESHOT, // EV_FORCEONESHOT
		    0, 0, ev->data.ptr);
		if (kevent(fd, kev, 1, NULL, 0, NULL) < 0) {
			return -1;
		}

		flags |= KQUEUE_STATE_NYCSS;
	}

	struct stat statbuf;
	if (fstat(fd2, &statbuf) < 0) {
		return -1;
	}

	if (S_ISFIFO(statbuf.st_mode)) {
		flags |= KQUEUE_STATE_ISFIFO;
	} else if (S_ISSOCK(statbuf.st_mode)) {
		flags |= KQUEUE_STATE_ISSOCK;
	}

	if ((e = kqueue_save_state(fd, fd2, flags)) < 0) {
		errno = e;
		return (-1);
	}

	return 0;
}

#undef VAL_BITS
#undef KEY_BITS

#if 0
int
epoll_pwait(
    int fd, struct epoll_event *ev, int cnt, int to, const sigset_t *sigs)
{
	int r = __syscall(SYS_epoll_pwait, fd, ev, cnt, to, sigs, _NSIG / 8);
#ifdef SYS_epoll_wait
	if (r == -ENOSYS && !sigs)
		r = __syscall(SYS_epoll_wait, fd, ev, cnt, to);
#endif
	return __syscall_ret(r);
}
#endif

int
epoll_wait(int fd, struct epoll_event *ev, int cnt, int to)
{
	if (cnt < 1) {
		errno = EINVAL;
		return -1;
	} else if (cnt > 32) {
		cnt = 32;
	}

	if (poll_fd != -1 && fd == poll_epoll_fd) {
		struct pollfd pfds[2];
		pfds[0].fd = poll_fd;
		pfds[0].events = POLLIN;
		pfds[1].fd = fd;
		pfds[1].events = POLLIN;
		int ret = poll(pfds, 2, to);
		if (ret <= 0) {
			return ret;
		}
		if (pfds[0].revents & POLLIN) {
			ev[0].events = EPOLLIN;
			ev[0].data.ptr = poll_ptr;
			return 1;
		}
		to = 0;
	}

	struct timespec timeout = {0, 0};
	if (to > 0) {
		timeout.tv_sec = to / 1000;
		timeout.tv_nsec = (to % 1000) * 1000 * 1000;
	}

	struct timespec *ptimeout = NULL;
	if (to >= 0) {
		ptimeout = &timeout;
	}

again:;
	struct kevent evlist[32];
	int ret = kevent(fd, NULL, 0, evlist, cnt, ptimeout);
	if (ret < 0) {
		return -1;
	}

	int j = 0;

	for (int i = 0; i < ret; ++i) {
		int events = 0;
		if (evlist[i].filter == EVFILT_READ) {
			events |= EPOLLIN;
			if (evlist[i].flags & EV_ONESHOT) {
				uint16_t flags = 0;
				kqueue_load_state(fd, evlist[i].ident, &flags);

				if (flags & KQUEUE_STATE_NYCSS) {
					if (is_not_yet_connected_stream_socket(
						evlist[i].ident)) {

						events = EPOLLHUP;
						if (flags &
						    KQUEUE_STATE_EPOLLOUT) {
							events |= EPOLLOUT;
						}

						struct kevent nkev[2];
						EV_SET(&nkev[0],
						    evlist[i].ident,
						    EVFILT_READ, EV_ADD, /**/
						    0, 0, evlist[i].udata);
						EV_SET(&nkev[1],
						    evlist[i].ident,
						    EVFILT_READ,
						    EV_ENABLE |
							EV_FORCEONESHOT,
						    0, 0, evlist[i].udata);

						kevent(fd, nkev, 2, NULL, 0,
						    NULL);
					} else {
						flags &= ~KQUEUE_STATE_NYCSS;

						struct kevent nkev[2];
						EV_SET(&nkev[0],
						    evlist[i].ident,
						    EVFILT_READ, EV_ADD, /**/
						    0, 0, evlist[i].udata);
						EV_SET(&nkev[1],
						    evlist[i].ident,
						    EVFILT_READ,
						    flags & KQUEUE_STATE_EPOLLIN
							? EV_ENABLE
							: EV_DISABLE,
						    0, 0, evlist[i].udata);

						kevent(fd, nkev, 2, NULL, 0,
						    NULL);
						kqueue_save_state(fd,
						    evlist[i].ident, flags);

						continue;
					}
				}
			}
		} else if (evlist[i].filter == EVFILT_WRITE) {
			events |= EPOLLOUT;
		}

		if (evlist[i].flags & EV_ERROR) {
			events |= EPOLLERR;
		}

		if (evlist[i].flags & EV_EOF) {
			if (evlist[i].fflags) {
				events |= EPOLLERR;
			}

			uint16_t flags = 0;
			kqueue_load_state(fd, evlist[i].ident, &flags);

			int epoll_event;

			if (flags & KQUEUE_STATE_ISFIFO) {
				if (evlist[i].filter == EVFILT_READ) {
					epoll_event = EPOLLHUP;
					if (evlist[i].data == 0) {
						events &= ~EPOLLIN;
					}
				} else if (evlist[i].filter == EVFILT_WRITE) {
					epoll_event = EPOLLERR;
				} else {
					/* should not happen */
					return -1;
				}
			} else if (flags & KQUEUE_STATE_ISSOCK) {
				if (evlist[i].filter == EVFILT_READ) {
					/* do some special EPOLLRDHUP handling
					 * for sockets */

					/* if we are reading, we just know for
					 * sure that we can't receive any more,
					 * so use EPOLLIN/EPOLLRDHUP per
					 * default */
					epoll_event = EPOLLIN;

					if (flags & KQUEUE_STATE_EPOLLRDHUP) {
						epoll_event |= EPOLLRDHUP;
					}
				} else if (evlist[i].filter == EVFILT_WRITE) {
					epoll_event = EPOLLOUT;
				} else {
					/* should not happen */
					return -1;
				}

				struct pollfd pfd = {.fd = evlist[i].ident,
				    .events = POLLIN | POLLOUT | POLLHUP};

				if (poll(&pfd, 1, 0) == 1) {
					if (pfd.revents & POLLHUP) {
						/*
						 * We need to set these flags
						 * so that readers still have a
						 * chance to read the last data
						 * from the socket. This is
						 * very important to preserve
						 * Linux poll/epoll semantics
						 * when coming from an
						 * EVFILT_WRITE event.
						 */
						if (flags &
						    KQUEUE_STATE_EPOLLIN) {
							epoll_event |= EPOLLIN;
						}
						if (flags &
						    KQUEUE_STATE_EPOLLRDHUP) {
							epoll_event |=
							    EPOLLRDHUP;
						}

						epoll_event |= EPOLLHUP;
					}

					/* might as well steal flags from the
					 * poll call while we're here */

					if ((pfd.revents & POLLIN) &&
					    (flags & KQUEUE_STATE_EPOLLIN)) {
						epoll_event |= EPOLLIN;
					}

					if ((pfd.revents & POLLOUT) &&
					    (flags & KQUEUE_STATE_EPOLLOUT)) {
						epoll_event |= EPOLLOUT;
					}
				}
			} else {
				epoll_event = EPOLLHUP;
			}

			events |= epoll_event;
		}
		ev[j].events = events;
		ev[j].data.ptr = evlist[i].udata;
		++j;
	}

	if (ret && j == 0) {
		goto again;
	}

	return j;
}
