#ifndef _MOD_CANOPEN_H_
#define _MOD_CANOPEN_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_linux.h>
#include <cobra_sys.h>

typedef struct canopen_status_s
{
	uint8_t		enable	:1;
	uint8_t				:7;
} CANOPEN_STATUS_S;

typedef struct mod_canopen_s
{
	SYS_STATUS_S		*sys_status;
	CANOPEN_STATUS_S	status;
} MOD_CANOPEN_S;

extern MOD_CANOPEN_S gl_mod_canopen;

void mod_canopen_init(COBRA_SYS_S *sys);


#ifdef __cplusplus
}
#endif

#endif /* _BATTERY_H_ */

