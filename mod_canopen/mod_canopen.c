#include <mod_canopen.h>
#include <cobra_console.h>
#include <cobra_cmd.h>
#include <cobra_sys.h>

#include <master_gauguin.h>

#define LOG_CANOPEN_LEVEL	LOG_LEVEL_INFO

#if (LOG_CANOPEN_LEVEL > LOG_LEVEL_NOT)
#define CANOPEN_INFO(fm, ...) { \
		console_cmdline_clean(); \
		console("CANOPEN : " fm, ##__VA_ARGS__); \
		console_cmdline_restore(); \
	}
#else
#define CANOPEN_INFO(fm, ...)
#endif

#if (LOG_CANOPEN_LEVEL > LOG_LEVEL_INFO)
#define CANOPEN_DEBUG(fm, ...) { \
		console_cmdline_clean(); \
		console("CANOPEN : " fm, ##__VA_ARGS__); \
		console_cmdline_restore(); \
	}
#else
#define CANOPEN_DEBUG(fm, ...)
#endif

#define CANOPEN_LOG(level, fm, ...) CANOPEN_##level(fm, ##__VA_ARGS__)

MOD_CANOPEN_S gl_mod_canopen;

/************************************************************/
/* mod command                                              */
/************************************************************/
#if COBRA_CMD_ENABLE
static void mod_canopen_cmd(void *cmd)
{
	COBRA_CMD_S *pcmd = (COBRA_CMD_S *)cmd;

	CANOPEN_LOG(INFO, "============================================================\r\n");
	if(0 == strlen(pcmd->arg)) {
		CANOPEN_LOG(INFO, "power: %d\r\n", gl_mod_canopen.status.enable);
	}
	else if(strncmp("on", pcmd->arg, strlen("on")) == 0 ||
		strncmp("1", pcmd->arg, strlen("1")) == 0) {
	}
	else if(strncmp("off", pcmd->arg, strlen("off")) == 0 ||
		strncmp("0", pcmd->arg, strlen("0")) == 0) {
	}
	else {
		CANOPEN_LOG(INFO, "Invalid Arguments\r\n");
		CANOPEN_LOG(INFO, "Usage: canopen [on|1]|[off|0]\r\n");
	}
	CANOPEN_LOG(INFO, "============================================================\r\n");
}
CMD_CREATE_SIMPLE(canopen, mod_canopen_cmd);

static void mod_can_send(void *cmd)
{
	COBRA_CMD_S *pcmd = (COBRA_CMD_S *)cmd;
	int byte = 0;

	CANOPEN_LOG(INFO, "============================================================\r\n");

	if(0 == strlen(pcmd->arg)) {
		CANOPEN_LOG(INFO, "Invalid Arguments\r\n");
		CANOPEN_LOG(INFO, "Usage: can_send [byte]\r\n");
	}
	else {
		sscanf(&pcmd->arg[0], "%d", &byte);
		CANOPEN_LOG(INFO, "need send [0x%02x]\r\n", byte & 0xff);
		//can_socket_send(&byte, 1);
	}
	CANOPEN_LOG(INFO, "============================================================\r\n");
}
CMD_CREATE_SIMPLE(cansend, mod_can_send);

#endif /* COBRA_CMD_ENABLE */
/************************************************************/

void mod_canopen_init(COBRA_SYS_S *sys)
{
	memset(&gl_mod_canopen, 0, sizeof(gl_mod_canopen));

	sys->mod->canopen			= &gl_mod_canopen;
	sys->status.mod->canopen	= &gl_mod_canopen.status;
	gl_mod_canopen.sys_status	= &sys->status;

#if COBRA_CMD_ENABLE
	cobra_console_cmd_register(&cmd_canopen);
	cobra_console_cmd_register(&cmd_cansend);
#endif

#if 1
	if(CBA_SUCCESS != master_gauguin_init()) {
		CANOPEN_LOG(INFO, "%s ... Failed\r\n", __func__);
	}
	else {
		CANOPEN_LOG(INFO, "%s ... OK\r\n", __func__);
	}
#endif
}
