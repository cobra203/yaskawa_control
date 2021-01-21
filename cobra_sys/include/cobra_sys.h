#ifndef _COBRA_SYS_H_
#define _COBRA_SYS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_linux.h>
#include <cobra_console.h>
#include <cobra_mod.h>

#if (LOG_SYS_LEVEL > LOG_LEVEL_NOT)
#define SYS_INFO(fm, ...) { \
		console_cmdline_clean(); \
		console("SYS      : " fm, ##__VA_ARGS__); \
		console_cmdline_restore(); \
	}
#else
#define SYS_INFO(fm, ...)
#endif

#if (LOG_SYS_LEVEL > LOG_LEVEL_INFO)
#define SYS_DEBUG(fm, ...) { \
		console_cmdline_clean(); \
		console("SYS      : " fm, ##__VA_ARGS__); \
		console_cmdline_restore(); \
	}
#else
#define SYS_DEBUG(fm, ...)
#endif

#define SYS_LOG(level, fm, ...) SYS_##level(fm, ##__VA_ARGS__)

#define COBRA_SYS_MOD_ENABLE		1

typedef struct sys_status_s
{
	uint32_t		flag;
	MOD_STATUS_S	*mod;
} SYS_STATUS_S;

typedef struct cobra_sys_s
{
	SYS_STATUS_S			status;

	COBRA_MOD_S				*mod;
} COBRA_SYS_S;

extern COBRA_SYS_S gl_sys;
void cobra_sys_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _COBRA_SYS_H_ */
