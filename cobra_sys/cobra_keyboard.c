#include <cobra_keyboard.h>
#include <cobra_console.h>

#if (LOG_KEYBOARD_LEVEL > LOG_LEVEL_NOT)
#define KEYBOARD_INFO(fm, ...) { \
	console_cmdline_clean(); \
	console("KEYBOARD : " fm, ##__VA_ARGS__); \
	console_cmdline_restore(); \
}
#else
#define KEYBOARD_INFO(fm, ...)
#endif

#if (LOG_CONSOLE_LEVEL > LOG_LEVEL_INFO)
#define KEYBOARD_DEBUG(fm, ...) { \
	console_cmdline_clean(); \
	console("KEYBOARD : " fm, ##__VA_ARGS__); \
	console_cmdline_restore(); \
}
#else
#define KEYBOARD_DEBUG(fm, ...)
#endif

#define KEYBOARD_LOG(level, fm, ...) KEYBOARD_##level(fm, ##__VA_ARGS__)

COBRA_KEYBOARD_S gl_keyboard;

KEY_STR_S gl_keys_table[] = {
	{0, ""},		//KEY_GROUP_EDIT
	{49, "HOME"},
	{50, "INS"},
	{51, "DEL"},
	{52, "END"},
	{53, "PGUP"},
	{54, "PGDN"},
	{7, ""},		//KEY_GROUP_ARROW
	{65, "UP"},
	{66, "DOWN"},
	{67, "RIGHT"},
	{68, "LEFT"},
	{12, ""},		//KEY_GROUP_FUNC_1_4
	{80, "F1"},
	{81, "F2"},
	{82, "F3"},
	{83, "F4"},
	{17, ""},		//KEY_GROUP_FUNC_5_12
	{48, "F9"},
	{49, "F10"},
	{50, ""},
	{51, "F11"},
	{52, "F12"},
	{53, "F5"},
	{54, ""},
	{55, "F6"},
	{56, "F7"},
	{57, "F8"},
	{28, ""},		//KEY_GROUP_COM
	{50, "SHIFT"},
	{51, "ALT"},
	{52, ""},
	{53, "CTRL"},
	{33, ""},		//KEY_GROUP_NORMAL
	{0, ""},
};
#define KEY_INDEX(GROUP, KEY) (gl_keys_table[GROUP].index + 1 + (KEY) - gl_keys_table[GROUP+1].index)

static void spec_key_process(uint8_t com_index, uint8_t key_index)
{
	uint8_t key_value = 0;

	if(CBA_SUCCESS == cobra_shortcut_process(&gl_keyboard.shortcut_head, com_index, key_index)) {
		return;
	}

	if(KEY_COM_NULL != com_index) {
		if(key_index > KEY_GROUP_NORMAL) {
			key_value = key_index - KEY_GROUP_NORMAL - 1;
			KEYBOARD_LOG(INFO, "Shortcut key [%s-%c] : Undefined\n",
				gl_keys_table[com_index].str, key_value < 33 ? key_value - 1 + 'A' : key_value);
		}
		else {
			KEYBOARD_LOG(INFO, "Shortcut key [%s-%s] : Undefined\n",
				gl_keys_table[com_index].str, gl_keys_table[key_index].str);
		}
	}
	else {
		KEYBOARD_LOG(INFO, "Shortcut key [%s] : Undefined\n",
			gl_keys_table[key_index].str);
	}
}

static inline int key_bit_zero(uint8_t key, const uint8_t *spec)
{
	switch(key) {
	case 27:
		return 1;
	case 127:
	case '\b':
		if(gl_keyboard.key_backspace_callback) {
			gl_keyboard.key_backspace_callback(key);
		}
		return 0;
	case '\t':
		if(gl_keyboard.key_tab_callback) {
			gl_keyboard.key_tab_callback(key);
		}
		return 0;
	case '\n':
	case '\r':
		if(gl_keyboard.key_enter_callback) {
			gl_keyboard.key_enter_callback(key);
		}
		return 0;
	}

	if(key <= 126) {
		if(32 <= key) {
			if(gl_keyboard.key_normol_callback) {
				gl_keyboard.key_normol_callback(key);
			}
		}
		else {
			spec_key_process(KEY_COM_CTRL, KEY_INDEX(KEY_GROUP_NORMAL, key));
		}
	}

	return 0;
}

static inline int key_bit_one(uint8_t key, const uint8_t *spec)
{
	if(79 == key || 91 == key) {
		return 1;
	}
	else if(32 <= key && key <= 126) {
		spec_key_process(KEY_COM_ALT, KEY_INDEX(KEY_GROUP_NORMAL, key));
	}
	return 0;
}

static inline int key_bit_two(uint8_t key, const uint8_t *spec)
{
	if(79 == spec[1]) {
		if(80 <= key && key <= 83) {
			spec_key_process(KEY_COM_NULL, KEY_INDEX(KEY_GROUP_FUNC_1_4, key));
		}
	}
	else if(91 == spec[1]) {
		if(65 <= key && key <= 68) {
			if(gl_keyboard.key_arrow_callback) {
				gl_keyboard.key_arrow_callback(KEY_INDEX(KEY_GROUP_ARROW, key));
			}
		}
		else if (49 <= key && key <= 54) {
			return 1;
		}
	}
	return 0;
}

static inline int key_bit_three(uint8_t key, const uint8_t *spec)
{
	if(126 == key) {
		spec_key_process(KEY_COM_NULL, KEY_INDEX(KEY_GROUP_EDIT, spec[2]));
	}
	else if(59 == key && 52 != spec[2] && 49 <= spec[2] && spec[2] <= 54) {
		return 1;
	}
	else if(50 == spec[2] && (50 != key && 48 <= key && key <= 52)) {
		return 1;
	}
	else if(49 == spec[2] && (54 != key && 53 <= key && key <= 57)) {
		return 1;
	}
	return 0;
}

static inline int key_bit_four(uint8_t key, const uint8_t *spec)
{
	if(126 == key) {
		if(48 <= spec[3] && spec[3] <= 57) {
			spec_key_process(KEY_COM_NULL, KEY_INDEX(KEY_GROUP_FUNC_5_12, spec[3]));
		}
	}
	else if(59 == spec[3] && 52 != key && 50 <= key && key <= 53) {
		return 1;
	}
	else if(59 != spec[3] && 59 == key) {
		return 1;
	}
	return 0;
}

static inline int key_bit_five(uint8_t key, const uint8_t *spec)
{
	if(50 <= spec[4] && spec[4] <= 53) {
		if((72 == key && 49 == spec[2]) || (126 == key && 49 != spec[2])) {
			spec_key_process(KEY_INDEX(KEY_GROUP_COM, spec[4]), KEY_INDEX(KEY_GROUP_EDIT, spec[2]));
		}
		else if(70 == key && 49 == spec[2]) {
			spec_key_process(KEY_INDEX(KEY_GROUP_COM, spec[4]), KEY_INDEX(KEY_GROUP_EDIT, 52));
		}
		else if(80 <= key && key <= 83) {
			spec_key_process(KEY_INDEX(KEY_GROUP_COM, spec[4]), KEY_INDEX(KEY_GROUP_FUNC_1_4, key));
		}
		else if(65 <= key && key <= 68) {
			spec_key_process(KEY_INDEX(KEY_GROUP_COM, spec[4]), KEY_INDEX(KEY_GROUP_ARROW, key));
		}
	}
	if(59 == spec[4] && 52 != key && 50 <= key && key <= 53) {
		return 1;
	}
	return 0;
}

static inline int key_bit_six(uint8_t key, const uint8_t *spec)
{
	if(126 == key) {
		spec_key_process(KEY_INDEX(KEY_GROUP_COM, spec[5]), KEY_INDEX(KEY_GROUP_FUNC_5_12, spec[3]));
	}
	return 0;
}

static void keyboard_key_id_process(uint8_t key)
{
	static uint8_t key_buffer[7];
	static uint8_t key_bit = 0;
	uint8_t ret = 0;

	switch(key_bit) {
	case 0:
		ret = key_bit_zero(key, (const uint8_t *)&key_buffer);
		break;
	case 1:
		ret = key_bit_one(key, (const uint8_t *)&key_buffer);
		break;
	case 2:
		ret = key_bit_two(key, (const uint8_t *)&key_buffer);
		break;
	case 3:
		ret = key_bit_three(key, (const uint8_t *)&key_buffer);
		break;
	case 4:
		ret = key_bit_four(key, (const uint8_t *)&key_buffer);
		break;
	case 5:
		ret = key_bit_five(key, (const uint8_t *)&key_buffer);
		break;
	case 6:
		ret = key_bit_six(key, (const uint8_t *)&key_buffer);
		break;
	}

	if(ret) {
		key_buffer[key_bit++] = key;
	}
	else {
		key_bit = 0;
	}
}

void cobra_keyboard_shortcut_register(COBRA_SHORTCUT_S *shortcut)
{
	if(CBA_SUCCESS == cobra_shortcut_register(&gl_keyboard.shortcut_head, shortcut)) {
		if(shortcut->key_index_min == shortcut->key_index_max) {
			if(KEY_COM_NULL != shortcut->com_index) {
				if(shortcut->key_index_min > KEY_GROUP_NORMAL) {
					KEYBOARD_LOG(INFO, "shortcut_register: [%s-%c]\n",
						gl_keys_table[shortcut->com_index].str, shortcut->key_index_min - KEY_GROUP_NORMAL);
				}
				else {
					KEYBOARD_LOG(INFO, "shortcut_register: [%s-%s]\n",
						gl_keys_table[shortcut->com_index].str, gl_keys_table[shortcut->key_index_min].str);
				}
			}
			else {
				KEYBOARD_LOG(INFO, "shortcut_register: [%s]\n", gl_keys_table[shortcut->key_index_min].str);
			}
		}
		else {
			if(KEY_COM_NULL != shortcut->com_index) {
				if(shortcut->key_index_min > KEY_GROUP_NORMAL) {
					KEYBOARD_LOG(INFO, "shortcut_register: [%s-%c] to [%s-%c]\n",
						gl_keys_table[shortcut->com_index].str, shortcut->key_index_min - KEY_GROUP_NORMAL,
						gl_keys_table[shortcut->com_index].str, shortcut->key_index_max - KEY_GROUP_NORMAL);
				}
				else {
					KEYBOARD_LOG(INFO, "shortcut_register: [%s-%s] to [%s-%s]\n",
						gl_keys_table[shortcut->com_index].str, gl_keys_table[shortcut->key_index_min].str,
						gl_keys_table[shortcut->com_index].str, gl_keys_table[shortcut->key_index_max].str);
				}
			}
			else {
				KEYBOARD_LOG(INFO, "shortcut_register: [%s] to [%s]\n",
					gl_keys_table[shortcut->key_index_min].str,
					gl_keys_table[shortcut->key_index_max].str);
			}
		}
	}
}

void cobra_keyboard_bind(COBRA_KEYBOARD_S **keyboard, CBA_BOOL enable)
{
	static struct termios record;
	struct termios cfg;

	if(enable) {
		tcgetattr(CONSOLE_FD, &record);
	    cfg = record;
	    cfg.c_iflag |= IGNCR;
	    cfg.c_lflag |= ~ICANON;
	    cfg.c_cc[VMIN] = 0;
	    cfg.c_cc[VTIME] = 0;
	    tcsetattr(CONSOLE_FD, TCSANOW, &cfg);

		cobra_shortcut_init(&gl_keyboard.shortcut_head);

		gl_keyboard.key_id_process = keyboard_key_id_process;

		*keyboard = &gl_keyboard;

		KEYBOARD_LOG(INFO, "%s ... enable\n", __func__);
	}
	else {
		tcsetattr(CONSOLE_FD, TCSANOW, &record);
		*keyboard = CBA_NULL;
	}
}
