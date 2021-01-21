#ifndef _COBRA_KEYBOARD_H_
#define _COBRA_KEYBOARD_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_linux.h>
#include <cobra_list.h>
#include <cobra_shortcut.h>

typedef enum spec_key_id_e
{
	KEY_GROUP_EDIT,
	KEY_EDIT_HOME,
	KEY_EDIT_INS,
	KEY_EDIT_DEL,
	KEY_EDIT_END,
	KEY_EDIT_PGUP,
	KEY_EDIT_PGDN,
	KEY_GROUP_ARROW,
	KEY_ARROW_UP,
	KEY_ARROW_DOWN,
	KEY_ARROW_RIGHT,
	KEY_ARROW_LEFT,
	KEY_GROUP_FUNC_1_4,
	KEY_FUNC_F1,
	KEY_FUNC_F2,
	KEY_FUNC_F3,
	KEY_FUNC_F4,
	KEY_GROUP_FUNC_5_12,
	KEY_FUNC_F9,
	KEY_FUNC_F10,
	KEY_FUNC_NULL_0,
	KEY_FUNC_F11,
	KEY_FUNC_F12,
	KEY_FUNC_F5,
	KEY_FUNC_NULL_1,
	KEY_FUNC_F6,
	KEY_FUNC_F7,
	KEY_FUNC_F8,
	KEY_GROUP_COM,
	KEY_COM_SHIFT,
	KEY_COM_ALT,
	KEY_COM_NULL,
	KEY_COM_CTRL,
	KEY_GROUP_NORMAL,
} SPEC_KEY_ID_E;

typedef struct key_str_s
{
	int index;
	char *str;
} KEY_STR_S;

extern KEY_STR_S gl_keys_table[];

typedef struct cobra_keyboard_s
{
	COBRA_SHORTCUT_S shortcut_head;

	void	(*key_id_process)			(uint8_t);

	void	(*key_backspace_callback)	(uint8_t);
	void	(*key_tab_callback)			(uint8_t);
	void	(*key_enter_callback)		(uint8_t);
	void	(*key_normol_callback)		(uint8_t);
	void	(*key_arrow_callback)		(uint8_t);
} COBRA_KEYBOARD_S;

extern COBRA_KEYBOARD_S gl_keyboard;

void cobra_keyboard_bind(COBRA_KEYBOARD_S **keyboard, CBA_BOOL enable);
void cobra_keyboard_shortcut_register(COBRA_SHORTCUT_S *shortcut);

#ifdef __cplusplus
}
#endif

#endif /* _COBRA_KEYBOARD_H_ */
