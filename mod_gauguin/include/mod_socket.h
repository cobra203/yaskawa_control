#ifndef _MOD_SOCKET_H_
#define _MOD_SOCKET_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_linux.h>
#include <cobra_sys.h>
#include <cobra_epoll.h>

typedef struct socket_status_s
{
	uint8_t		enable	:1;
	uint8_t		write	:1;
	uint8_t				:6;
} SOCKET_STATUS_S;

typedef struct mod_socket_s
{
	SYS_STATUS_S		*sys_status;
	SOCKET_STATUS_S		status;
	int					*listen_fd;
	COBRA_EPOLL_S		*listen_epoll;
	int					*conn_fd;
	COBRA_EPOLL_S		*conn_epoll;
} MOD_SOCKET_S;

extern MOD_SOCKET_S gl_mod_socket;

void mod_socket_init(COBRA_SYS_S *sys);

#ifdef __cplusplus
}
#endif

#endif /* _MOD_SOCKET_H_ */
