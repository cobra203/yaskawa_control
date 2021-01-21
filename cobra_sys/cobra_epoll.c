#include <stdlib.h>
#include <errno.h>
#include <sys/epoll.h>

#include <cobra_list.h>
#include <cobra_linux.h>
#include <cobra_epoll.h>

#define LOG_LEVEL_NOT	0
#define LOG_LEVEL_INFO	1
#define LOG_LEVEL_DEBUG	2

#if (LOG_EPOLL_LEVEL > LOG_LEVEL_NOT)
#define EPOLL_INFO(fm, ...) { \
	console_cmdline_clean(); \
	console("EPOLL    : " fm, ##__VA_ARGS__); \
	console_cmdline_restore(); \
}
#else
#define EPOLL_INFO(fm, ...)
#endif

#if (LOG_EPOLL_LEVEL > LOG_LEVEL_INFO)
#define EPOLL_DEBUG(fm, ...) { \
	console_cmdline_clean(); \
	console("EPOLL    : " fm, ##__VA_ARGS__); \
	console_cmdline_restore(); \
}
#else
#define EPOLL_DEBUG(fm, ...)
#endif

#define EPOLL_LOG(level, fm, ...) EPOLL_##level(fm, ##__VA_ARGS__)

static int gl_epoll_fd = 0;
struct epoll_event gl_epoll_events[MAX_EPOLL_EVENTS];
static COBRA_EPOLL_S gl_epoll_head;
static uint8_t gl_epoll_count = 0;

int epoll_monitor_handle(void)
{
	COBRA_EPOLL_S *pos;
	int nfds = 0;
	int i = 0;

	nfds = epoll_wait(gl_epoll_fd, gl_epoll_events, gl_epoll_count, 0);
	if (nfds == -1) {
		return errno;
	}

	for(i = 0; i < nfds; i++) {
		list_for_each_entry(pos, &gl_epoll_head.list, COBRA_EPOLL_S, list) {
			if(gl_epoll_events[i].data.fd == pos->fd) {
				return pos->callback(pos->data);
			}
		}
	}
	return 0;
}

int epoll_bind_register(COBRA_EPOLL_S **bind, COBRA_EPOLL_S *epoll, uint8_t type)
{
	*bind = epoll;

	if(gl_epoll_head.list.next && gl_epoll_head.list.prev) {
		switch(type) {
		case EP_TYPE_OUT:
			epoll->ev.events = EPOLLOUT;
			break;
		default:
			epoll->ev.events = EPOLLIN;
			break;
		}
		if(epoll_ctl(gl_epoll_fd, EPOLL_CTL_ADD, epoll->fd, &epoll->ev) == -1) {
			return errno;
		}
		list_add_tail(&epoll->list, &gl_epoll_head.list);
		gl_epoll_count++;
		EPOLL_LOG(INFO, "epoll_bind_register: %s\n", epoll->name);
	}
	return CBA_SUCCESS;
}

int epoll_bind_deregister(COBRA_EPOLL_S **bind, COBRA_EPOLL_S *epoll)
{
	list_del(&epoll->list);
	*bind = CBA_NULL;

	if(epoll_ctl(gl_epoll_fd, EPOLL_CTL_DEL, epoll->fd, &epoll->ev) == -1) {
		return errno;
	}
	if(gl_epoll_count) {
		gl_epoll_count--;
	}
	else {
		return ENOTSUP;
	}
	EPOLL_LOG(INFO, "epoll_bind_deregister: %s\n", epoll->name);

	return CBA_SUCCESS;
}

int cobra_epoll_init(void)
{
#if (COBRA_CONSOLE_ENABLE && COBRA_CMD_ENABLE)
	gl_epoll_fd = epoll_create1(0);
	if(gl_epoll_fd == -1) {
		return errno;
	}

	INIT_LIST_HEAD(&gl_epoll_head.list);
#endif
	EPOLL_LOG(INFO, "%s ... OK\r\n", __func__);

	return CBA_SUCCESS;
}
