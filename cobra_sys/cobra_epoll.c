#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>

#include <cobra_list.h>
#include <cobra_linux.h>
#include <cobra_epoll.h>
#include <cobra_console.h>

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

typedef struct EPOLL_HANDLE_S
{
	int					fd;
	COBRA_EPOLL_S		head;
	struct epoll_event	events[MAX_EPOLL_EVENTS];
	uint8_t				conn_count;
} epoll_handle_s;

epoll_handle_s gl_epoll;

int epoll_monitor_handle(void)
{
	COBRA_EPOLL_S *pos;
	int num_fds = 0;
	int i = 0;

	num_fds = epoll_wait(gl_epoll.fd, gl_epoll.events, gl_epoll.conn_count, 0);
	if (-1 == num_fds) {
		return errno;
	}

	for(i = 0; i < num_fds; i++) {
		list_for_each_entry(pos, &gl_epoll.head.list, COBRA_EPOLL_S, list) {
			if(gl_epoll.events[i].data.fd == pos->fd) {
				memcpy(&pos->ev_callbak, &gl_epoll.events[i], sizeof(struct epoll_event));
				return pos->callback(pos->data);
			}
		}
	}
	return 0;
}

int epoll_bind_register(COBRA_EPOLL_S **bind, COBRA_EPOLL_S *epoll, uint8_t type)
{
	*bind = epoll;

	if(gl_epoll.head.list.next && gl_epoll.head.list.prev) {
		switch(type) {
		case EP_TYPE_IN:
			epoll->ev.events = EPOLLIN;
			break;

		case EP_TYPE_OUT:
			epoll->ev.events = EPOLLOUT;
			break;

		default:
			epoll->ev.events = EPOLLIN | EPOLLOUT;
			break;
		}
		epoll->ev.data.fd = epoll->fd;

		if(epoll_ctl(gl_epoll.fd, EPOLL_CTL_ADD, epoll->fd, &epoll->ev) == -1) {
			EPOLL_LOG(INFO, "epoll_ctl : %s : %d\r\n", strerror(errno), errno);
			return errno;
		}
		list_add_tail(&epoll->list, &gl_epoll.head.list);
		gl_epoll.conn_count++;

		EPOLL_LOG(INFO, "epoll %s[%d] : register\n", epoll->name, epoll->fd);
	}
	return CBA_SUCCESS;
}

int epoll_bind_deregister(COBRA_EPOLL_S **bind, COBRA_EPOLL_S *epoll)
{
	list_del(&epoll->list);
	*bind = CBA_NULL;

	if(epoll_ctl(gl_epoll.fd, EPOLL_CTL_DEL, epoll->fd, &epoll->ev) == -1) {
		EPOLL_LOG(INFO, "epoll_ctl : %s : %d\r\n", strerror(errno), errno);
		return errno;
	}
	if(gl_epoll.conn_count) {
		gl_epoll.conn_count--;
	}
	else {
		return ENOTSUP;
	}
	EPOLL_LOG(INFO, "epoll %s[%d] : deregister\n", epoll->name, epoll->fd);

	return CBA_SUCCESS;
}

#if 0
int epoll_ctl_mod(COBRA_EPOLL_S **bind, COBRA_EPOLL_S *epoll)
{
	if(epoll_ctl(gl_epoll.fd, EPOLL_CTL_MOD, epoll->fd, &epoll->ev) == -1) {
		EPOLL_LOG(INFO, "epoll_ctl : %s : %d\r\n", strerror(errno), errno);
		return errno;
	}
}
#endif

int cobra_epoll_init(void)
{
	memset(&gl_epoll, 0, sizeof(gl_epoll));

#if (COBRA_CONSOLE_ENABLE && COBRA_CMD_ENABLE)
	gl_epoll.fd = epoll_create1(0);
	if(gl_epoll.fd == -1) {
		EPOLL_LOG(INFO, "epoll_create1 : %s : %d\r\n", strerror(errno), errno);
		return errno;
	}

	INIT_LIST_HEAD(&gl_epoll.head.list);
#endif

	EPOLL_LOG(INFO, "%s ... OK\r\n", __func__);

	return CBA_SUCCESS;
}
