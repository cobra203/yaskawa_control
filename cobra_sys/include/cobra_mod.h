#ifndef _COBRA_MOD_H_
#define _COBRA_MOD_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_linux.h>

typedef struct mod_status_s
{
	struct gauguin_status_s	*gauguin;
	struct socket_status_s *socket;
} MOD_STATUS_S;

typedef struct cobra_mod_s
{
	MOD_STATUS_S			status;

	struct mod_gauguin_s	*gauguin;
	struct mod_socket_s		*socket;
} COBRA_MOD_S;

#if COBRA_CMD_ENABLE
void cobra_mod_status_dump(void);
#endif
void cobra_mod_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _COBRA_MOD_H_ */
