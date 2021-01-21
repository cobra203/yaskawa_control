#ifndef _COBRA_CONSOLE_H_
#define _COBRA_CONSOLE_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_linux.h>
#include <cobra_epoll.h>
#include <cobra_keyboard.h>
#include <cobra_cmd.h>

#define CONSOLE_FD STDIN_FILENO

#define CONSOLE_TAG			"[ cobra ]:/# "

#if defined(SYSTEM_NAME)
#define COBRA_SYSTEM_NAME		SYSTEM_NAME
#else
#define COBRA_SYSTEM_NAME		"| system  : cobra system                                             |\r\n"
#endif

#if defined(SYSTEM_VERSION)
#define COBRA_SYSTEM_VERSION	SYSTEM_VERSION
#else
#define COBRA_SYSTEM_VERSION	"| version : release                                                  |\r\n"
#endif

#define LOG_LEVEL_NOT	0
#define LOG_LEVEL_INFO	1
#define LOG_LEVEL_DEBUG	2

/* ============= define console log of module =============*/
#if COBRA_CONSOLE_ENABLE
#define LOG_CONSOLE_LEVEL	LOG_LEVEL_INFO
#define LOG_EPOLL_LEVEL		LOG_LEVEL_INFO
#define LOG_KEYBOARD_LEVEL	LOG_LEVEL_INFO
#define LOG_SHORTCUT_LEVEL	LOG_LEVEL_INFO
#define LOG_MAIN_LEVEL		LOG_LEVEL_INFO
#define LOG_CMD_LEVEL		LOG_LEVEL_INFO
#define LOG_DEBUG_LEVEL		LOG_LEVEL_INFO
#define LOG_SYS_LEVEL		LOG_LEVEL_INFO
#define LOG_CANOPEN_LEVEL	LOG_LEVEL_INFO
#endif /* COBRA_CONSOLE_ENABLE */
/* ========================================================*/
/* ============= define console log of canfestival ========*/
#define __COBRA_CONSOLE__
#define LOG_CAN_LEVEL		LOG_LEVEL_INFO
/* ========================================================*/
/*===================================================================================*/
/* Define console function															 */
/*===================================================================================*/
#if COBRA_CONSOLE_ENABLE
#define console(format, ...) \
	do { \
		dprintf(CONSOLE_FD, format, ##__VA_ARGS__); \
	} while(0)
#define PUTCHAR(BYTE) \
	do { \
		uint8_t byte = (BYTE); \
		byte = write(CONSOLE_FD, &byte, 1); \
	} while(0)
#else
#define console(format, ...)
#define PUTCHAR(BYTE)
#endif

typedef struct cobra_console_s
{
#if (COBRA_CONSOLE_ENABLE && COBRA_CMD_ENABLE)
	COBRA_CMD_S 		cmd_head;
	uint8_t				cmdline[_CMDLINE_MAX_SIZE_];
	uint8_t				cmdline_size;
	uint8_t				tabcmd[_TABCMD_MAX_SIZE_];
	uint8_t				tabcmd_size;
	int					tablast;

	COBRA_EPOLL_S		*epoll;
	COBRA_KEYBOARD_S	*keyboard;
#endif
} COBRA_CONSOLE_S;

void console_cmdline_clean(void);
void console_cmdline_restore(void);
void cobra_console_cmd_register(COBRA_CMD_S *cmd);
int cobra_console_init(void);
void cobra_console_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* _COBRA_CONSOLE_H_ */
