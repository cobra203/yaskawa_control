#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <cobra_console.h>
#include <cobra_cmd.h>
#include <cobra_sys.h>
#include <mod_socket.h>

#define LOG_SOCKET_LEVEL	LOG_LEVEL_INFO

#if (LOG_SOCKET_LEVEL > LOG_LEVEL_NOT)
#define SOCKET_INFO(fm, ...) { \
		console_cmdline_clean(); \
		console("SOCKET   : " fm, ##__VA_ARGS__); \
		console_cmdline_restore(); \
	}
#else
#define SOCKET_INFO(fm, ...)
#endif

#if (LOG_SOCKET_LEVEL > LOG_LEVEL_INFO)
#define SOCKET_DEBUG(fm, ...) { \
		console_cmdline_clean(); \
		console("SOCKET   : " fm, ##__VA_ARGS__); \
		console_cmdline_restore(); \
	}
#else
#define SOCKET_DEBUG(fm, ...)
#endif

#define SOCKET_LOG(level, fm, ...) SOCKET_##level(fm, ##__VA_ARGS__)

MOD_SOCKET_S gl_mod_socket;

struct cobra_epoll_s epoll_socket_connect;
static int _socket_connect_callback(void *arg)
{
	int size;
	char tmp_buff[8+1] = {0};

	if(gl_mod_socket.conn_epoll->ev_callbak.events & EPOLLIN) {
		SOCKET_LOG(INFO, "events: 0x%08x\n", gl_mod_socket.conn_epoll->ev_callbak.events);
		size = read(*gl_mod_socket.conn_fd, &tmp_buff, sizeof(tmp_buff)-1);

		if(size) {
			tmp_buff[size] = 0;
			SOCKET_LOG(INFO, "conn_fd[%d]: [%s] receive size = %d\n", *gl_mod_socket.conn_fd, tmp_buff, size);
		}
		else {
			SOCKET_LOG(INFO, "conn_fd[%d]: disconnect\n", *gl_mod_socket.conn_fd);
			epoll_bind_deregister(&gl_mod_socket.conn_epoll, &epoll_socket_connect);
			close(*gl_mod_socket.conn_fd);
			return CBA_SUCCESS;
		}
	}
	if(gl_mod_socket.status.write && gl_mod_socket.conn_epoll->ev_callbak.events & EPOLLOUT) {
		SOCKET_LOG(INFO, "events: 0x%08x\n", gl_mod_socket.conn_epoll->ev_callbak.events);
		SOCKET_LOG(INFO, "conn_fd[%d]: need send\n", *gl_mod_socket.conn_fd);
	}

	return CBA_SUCCESS;
}
EPOLL_CREATE_SIMPLE(socket_connect, 0, _socket_connect_callback);

static int _socket_listen_callback(void *arg)
{
	if((*gl_mod_socket.conn_fd = accept(*gl_mod_socket.listen_fd, (struct sockaddr*)NULL, NULL)) == -1){
		SOCKET_LOG(INFO, "accept : %s : %d\n", strerror(errno), errno);
		return CBA_FAILURE;
	}
	SOCKET_LOG(INFO, "conn_fd[%d] connect OK\n", *gl_mod_socket.conn_fd);

	epoll_bind_register(&gl_mod_socket.conn_epoll, &epoll_socket_connect, EP_TYPE_IN);

	return CBA_SUCCESS;
}
EPOLL_CREATE_SIMPLE(socket_listen, CONSOLE_FD, _socket_listen_callback);

static void _cmd_socketsend(void *cmd)
{
	COBRA_CMD_S *pcmd = (COBRA_CMD_S *)cmd;

	SOCKET_LOG(INFO, "============================================================\r\n");

	if(0 != strlen(pcmd->arg)) {
		SOCKET_LOG(INFO, "arg : \"%s\"[%d]\n", pcmd->arg, (int)strlen(pcmd->arg));
		if(send(*gl_mod_socket.conn_fd, pcmd->arg, strlen(pcmd->arg), 0) < 0) {
            SOCKET_LOG(INFO, "send msg error: %s(errno: %d)\n", strerror(errno), errno);
        }
	}
	else {
		SOCKET_LOG(INFO, "Invalid Arguments\r\n");
		SOCKET_LOG(INFO, "Usage: socketsend [data]\r\n");
	}
	SOCKET_LOG(INFO, "============================================================\r\n");
}
CMD_CREATE_SIMPLE(socketsend, _cmd_socketsend);

static CBA_BOOL _mod_socket_create(void)
{
	struct sockaddr_in servaddr;
	int opt = 1;

	if((*gl_mod_socket.listen_fd  = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		SOCKET_LOG(INFO, "socket: %s : %d\n", strerror(errno), errno);
		return CBA_FAILURE;
	}
	SOCKET_LOG(INFO, "listen_fd[%d] socket OK\n", *gl_mod_socket.listen_fd);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(6666);
	bzero(&(servaddr.sin_zero),8);

	setsockopt(*gl_mod_socket.listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if(bind(*gl_mod_socket.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
		SOCKET_LOG(INFO, "bind : %s : %d\n", strerror(errno), errno);
		return CBA_FAILURE;
	}
	SOCKET_LOG(INFO, "listen_fd[%d] bind OK\n", *gl_mod_socket.listen_fd);

	if(listen(*gl_mod_socket.listen_fd, 10) == -1){
		SOCKET_LOG(INFO, "listen: %s : %d\n", strerror(errno), errno);
		return CBA_FAILURE;
	}
	SOCKET_LOG(INFO, "listen_fd[%d] listen OK\n", *gl_mod_socket.listen_fd);

	epoll_bind_register(&gl_mod_socket.listen_epoll, &epoll_socket_listen, EP_TYPE_IO);

	return CBA_SUCCESS;
}

void mod_socket_init(COBRA_SYS_S *sys)
{
	memset(&gl_mod_socket, 0, sizeof(gl_mod_socket));

	sys->mod->socket			= &gl_mod_socket;
	sys->status.mod->socket		= &gl_mod_socket.status;
	gl_mod_socket.sys_status	= &sys->status;

	gl_mod_socket.listen_fd		= &epoll_socket_listen.fd;
	gl_mod_socket.conn_fd		= &epoll_socket_connect.fd;

	_mod_socket_create();

#if COBRA_CMD_ENABLE
	cobra_console_cmd_register(&cmd_socketsend);
#endif
	SOCKET_LOG(INFO, "%s ... OK\n", __func__);
}
