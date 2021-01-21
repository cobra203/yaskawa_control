#ifndef _COBRA_CMD_H_
#define _COBRA_CMD_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_linux.h>
#include <cobra_list.h>

/***********************************************/
/* cmdline max size is _CMDLINE_MAX_SIZE_=32   */
/* so a cmd format is:                         */
/* prefix_subcmd arg                           */
/* 15+1 + 15+1 + 223 + '\0' = 256              */
/***********************************************/
#define _PREFIX_SIZE_		(15 + 1)						/* the 1 is '_' */
#define _SUBCMD_SIZE_		(15 + 1)						/* the 1 is ' ' */
#define _ARG_SIZE_			(223 + 1)						/* the 1 is '\0' */
#define _CMD_SIZE			(_PREFIX_SIZE_ + _SUBCMD_SIZE_)
#define _TABCMD_MAX_SIZE_	(_CMD_SIZE + 1)					/* the 1 is '\0' */
#define _CMDLINE_MAX_SIZE_	(_CMD_SIZE + _ARG_SIZE_)

#define COBRA_LIST_CMD_ENABLE	1

typedef struct cobra_cmd_s
{
	char			prefix[_PREFIX_SIZE_];
	char			subcmd[_SUBCMD_SIZE_];
	char			arg[_ARG_SIZE_];
	uint8_t			status;
	void			(*process)		(void *);
	LIST_S			list;
} COBRA_CMD_S;

#define CMD_CREATE(PREFIX, SUBCMD, PROCESS) \
struct cobra_cmd_s cmd_##PREFIX##_##SUBCMD = { \
	.prefix = #PREFIX, \
	.subcmd = #SUBCMD, \
	.process = PROCESS, \
}

#define CMD_CREATE_SIMPLE(PREFIX, PROCESS) \
struct cobra_cmd_s cmd_##PREFIX = { \
	.prefix = #PREFIX, \
	.process = PROCESS, \
}

CBA_BOOL cobra_cmd_parse(const char *cmdline, COBRA_CMD_S *cmd);
CBA_BOOL cobra_cmd_process(COBRA_CMD_S *head, COBRA_CMD_S *cmd);
CBA_BOOL cobra_cmd_register(COBRA_CMD_S *head, COBRA_CMD_S *cmd);
void cobra_cmd_init(COBRA_CMD_S *head);

#ifdef __cplusplus
}
#endif

#endif /* _COBRA_CMD_H_ */
