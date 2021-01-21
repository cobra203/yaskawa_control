#ifndef _COBRA_EPOLL_H_
#define _COBRA_EPOLL_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <sys/epoll.h>
#include <cobra_linux.h>
#include <cobra_list.h>

#define MAX_EPOLL_EVENTS 20

typedef enum epoll_type_e
{
	EP_TYPE_IN,
	EP_TYPE_OUT,
} EPOLL_TYPE_E;

typedef struct cobra_epoll_s
{
	char		*name;
	int			fd;
	struct		epoll_event ev;
	void		*data;
	int			(*callback)		(void *);
	LIST_S		list;
} COBRA_EPOLL_S;

#define EPOLL_CREATE(NAME, FD, CALLBACK, DATA) \
struct cobra_epoll_s epoll_##CALLBACK = { \
	.name = (NAME), \
	.fd = (FD), \
	.callback = CALLBACK, \
	.data = (void *)(DATA), \
}

#define EPOLL_CREATE_SIMPLE(NAME, FD, CALLBACK) \
struct cobra_epoll_s epoll_##CALLBACK = { \
	.name = NAME, \
	.fd = (FD), \
	.callback = CALLBACK, \
}

int epoll_monitor_handle(void);
int epoll_bind_register(COBRA_EPOLL_S **bind, COBRA_EPOLL_S *epoll, uint8_t type);
int epoll_bind_deregister(COBRA_EPOLL_S **bind, COBRA_EPOLL_S *epoll);
int cobra_epoll_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _COBRA_EPOLL_H_ */
