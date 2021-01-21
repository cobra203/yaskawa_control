#include <cobra_mod.h>
#include <cobra_list.h>
#include <cobra_cmd.h>
#include <cobra_console.h>
#include <cobra_sys.h>
#include <mod_canopen.h>

COBRA_MOD_S gl_mod;

/************************************************************/
/* module IRQ Handler define                                */
/************************************************************/

/************************************************************/

#if COBRA_CMD_ENABLE
void cobra_mod_status_dump(void)
{
	MOD_STATUS_S *status = gl_sys.status.mod;

	if(status->canopen) {
		SYS_LOG(INFO, "| canopen  | enable    : %d                                 |\r\n",
						status->canopen->enable);
	}
}
#endif

void cobra_mod_init(void)
{
	memset(&gl_mod, 0, sizeof(gl_mod));
	gl_sys.mod = &gl_mod;
	gl_sys.status.mod = &gl_mod.status;

#if MOD_CANOPEN_EN
	mod_canopen_init(&gl_sys);
#endif

	SYS_LOG(INFO, "%s ... OK\r\n", __func__);
}
