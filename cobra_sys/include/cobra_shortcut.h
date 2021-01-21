#ifndef _COBRA_SHORTCUT_H_
#define _COBRA_SHORTCUT_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_linux.h>
#include <cobra_list.h>

typedef struct cobra_shortcut_s
{
	uint8_t		com_index;
	uint8_t		key_index_min;
	uint8_t		key_index_max;
	void		(*process)		(void *);
	LIST_S		list;
} COBRA_SHORTCUT_S;

#define SHORTCUT_CREATE(COM_IDX, KEY_IDX_MIN, KEY_IDX_MAX, PROCESS) \
struct cobra_shortcut_s shortcut_##PROCESS = { \
	.com_index = (COM_IDX), \
	.key_index_min = (KEY_IDX_MIN), \
	.key_index_max = (KEY_IDX_MAX), \
	.process = PROCESS, \
}

#define SHORTCUT_CREATE_SIMPLE(COM_IDX, KEY_IDX, PROCESS) \
struct cobra_shortcut_s shortcut_##PROCESS = { \
	.com_index = (COM_IDX), \
	.key_index_min = (KEY_IDX), \
	.key_index_max = (KEY_IDX), \
	.process = PROCESS, \
}

CBA_BOOL cobra_shortcut_process(COBRA_SHORTCUT_S *head, uint8_t com_index, uint8_t key_index);
CBA_BOOL cobra_shortcut_register(COBRA_SHORTCUT_S *head, COBRA_SHORTCUT_S *shortcut);
void cobra_shortcut_init(COBRA_SHORTCUT_S *head);

#ifdef __cplusplus
}
#endif

#endif /* _COBRA_SHORTCUT_H_ */
