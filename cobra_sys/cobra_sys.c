#include <cobra_sys.h>

COBRA_SYS_S gl_sys;

/************************************************************/
/* system command                                           */
/************************************************************/
#if COBRA_CMD_ENABLE
static void cobra_sys_status(void *cmd)
{
	COBRA_CMD_S *pcmd = (COBRA_CMD_S *)cmd;

	SYS_LOG(INFO, "============================================================\r\n");
	if(0 == strlen(pcmd->arg)) {
#if COBRA_SYS_MOD_ENABLE
		cobra_mod_status_dump();
#endif
	}
	else {
		SYS_LOG(INFO, "Invalid Arguments\r\n");
		SYS_LOG(INFO, "Usage: status\r\n");
	}
	SYS_LOG(INFO, "============================================================\r\n");
}
CMD_CREATE_SIMPLE(status, cobra_sys_status);
#endif /* COBRA_CMD_ENABLE */
/************************************************************/

void cobra_sys_init(void)
{
	memset(&gl_sys, 0, sizeof(gl_sys));

#if COBRA_CMD_ENABLE
	cobra_console_cmd_register(&cmd_status);
#endif

#if COBRA_SYS_MOD_ENABLE
	cobra_mod_init();
#endif

	SYS_LOG(INFO, "%s ... OK\r\n", __func__);
}
