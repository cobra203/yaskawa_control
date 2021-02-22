#include <cobra_console.h>

#if (LOG_CONSOLE_LEVEL > LOG_LEVEL_NOT)
#define CONSOLE_INFO(fm, ...) { \
	console_cmdline_clean(); \
	console("CONSOLE  : " fm, ##__VA_ARGS__); \
	console_cmdline_restore(); \
}
#else
#define CONSOLE_INFO(fm, ...)
#endif

#if (LOG_CONSOLE_LEVEL > LOG_LEVEL_INFO)
#define CONSOLE_DEBUG(fm, ...) { \
	console_cmdline_clean(); \
	console("CONSOLE  : " fm, ##__VA_ARGS__); \
	console_cmdline_restore(); \
}
#else
#define CONSOLE_DEBUG(fm, ...)
#endif

#define CONSOLE_LOG(level, fm, ...) CONSOLE_##level(fm, ##__VA_ARGS__)

COBRA_CONSOLE_S gl_console;

/*===================================================================================*/
/* For console log                                                                   */
/*===================================================================================*/
void inline _console_stdout_backspace(uint8_t size)
{
	while(size--) {
		console("\b \b");
	}
}

void console_cmdline_clean(void)
{
#if (COBRA_CONSOLE_ENABLE && COBRA_CMD_ENABLE)
	char size = gl_console.tabcmd_size ? gl_console.tabcmd_size : gl_console.cmdline_size;
#else
	char size = 0;
#endif
	size += sizeof(CONSOLE_TAG);
	_console_stdout_backspace(size);
}

void console_cmdline_restore(void)
{
#if (COBRA_CONSOLE_ENABLE && COBRA_CMD_ENABLE)
	uint8_t *line = gl_console.tabcmd_size ? gl_console.tabcmd : gl_console.cmdline;
	console(CONSOLE_TAG);
	console("%s", line);
#endif
}

#if (COBRA_CONSOLE_ENABLE && COBRA_CMD_ENABLE)
static inline void _console_tabcmd_to_cmdline(void)
{
	memcpy(gl_console.cmdline, gl_console.tabcmd, gl_console.tabcmd_size);
	gl_console.cmdline_size = gl_console.tabcmd_size;
	gl_console.cmdline[gl_console.cmdline_size] = 0;
	gl_console.tabcmd_size = 0;
	gl_console.tabcmd[0] = 0;
	gl_console.tablast = -1;
}
#endif

/*===================================================================================*/
/* For console cmdline key process                                                   */
/*===================================================================================*/
#if (COBRA_CONSOLE_ENABLE && COBRA_CMD_ENABLE)
static void _console_cmdline_normal(uint8_t key)
{

	if(gl_console.tabcmd_size) {
		_console_tabcmd_to_cmdline();
	}

	if(gl_console.cmdline_size < _CMDLINE_MAX_SIZE_ - 1) {
		gl_console.cmdline[gl_console.cmdline_size++] = key;
		gl_console.cmdline[gl_console.cmdline_size] = 0;
		PUTCHAR(key);
	}
}

static void _console_cmdline_tab(uint8_t key)
{
	static CBA_BOOL next = CBA_FALSE;
	COBRA_CMD_S *pos;
	uint8_t i = 0;
	char tabcmd[_TABCMD_MAX_SIZE_] = {0};
	CBA_BOOL found = CBA_FALSE;

	if(!next) {
		gl_console.tablast = -1;
	}

	if(gl_console.cmd_head.list.next && gl_console.cmd_head.list.prev) {
		list_for_each_entry(pos, &gl_console.cmd_head.list, COBRA_CMD_S, list) {
			if(strlen(pos->subcmd)) {
				snprintf(tabcmd, sizeof(tabcmd), "%s_%s", pos->prefix, pos->subcmd);;
			}
			else {
				snprintf(tabcmd, sizeof(tabcmd), "%s", pos->prefix);
			}
			if(gl_console.cmdline_size < strlen(tabcmd)) {
				if(!strncmp(tabcmd, (const char *)gl_console.cmdline, gl_console.cmdline_size)) {
					if(i > gl_console.tablast) {
						if(!found) {
							memcpy(gl_console.tabcmd, tabcmd, strlen(tabcmd) + 1);
							if(gl_console.tabcmd_size > gl_console.cmdline_size) {
								_console_stdout_backspace(gl_console.tabcmd_size - gl_console.cmdline_size);
							}
							gl_console.tabcmd_size = strlen(tabcmd);
							console("%s", &gl_console.tabcmd[gl_console.cmdline_size]);

							gl_console.tablast = i;
							found = CBA_TRUE;
							next = CBA_FALSE;
						}
						else {
							next = CBA_TRUE;
							return;
						}
					}
				}
			}
			i++;
		}
	}
}

static void _console_cmdline_enter(uint8_t key)
{
	COBRA_CMD_S cmd;
	uint8_t *line = gl_console.tabcmd_size ? gl_console.tabcmd : gl_console.cmdline;
	uint8_t size = gl_console.tabcmd_size ? gl_console.tabcmd_size : gl_console.cmdline_size;
	uint8_t	ret = 0;

	//PUTCHAR('\n');
	console("\n%s", CONSOLE_TAG);
	if(size) {
		ret = cobra_cmd_parse((const char *)line, &cmd);
		gl_console.cmdline_size = 0;
		gl_console.cmdline[0] = 0;
		gl_console.tabcmd_size = 0;
		gl_console.cmdline[0] = 0;
		if(CBA_SUCCESS == ret) {
			if(CBA_SUCCESS != cobra_cmd_process(&gl_console.cmd_head, &cmd)) {
				CONSOLE_LOG(INFO, "Invalid command\r\n");
			}
		}
		else {
			CONSOLE_LOG(INFO, "Format error\r\n");
		}
	}
	gl_console.tablast = -1;
}

static void _console_cmdline_backspace(uint8_t key)
{
	if(gl_console.tabcmd_size > 0) {
		_console_tabcmd_to_cmdline();
	}
	if(gl_console.cmdline_size > 0) {
		console("\b \b");
		gl_console.cmdline_size--;
		gl_console.cmdline[gl_console.cmdline_size] = 0;
	}
}

static void _console_cmdline_arrow(uint8_t key_index)
{
	console("arrow[%s]", gl_keys_table[key_index].str);
}

/*===================================================================================*/
/* For console epoll handle                                                          */
/*===================================================================================*/
static int console_recv_callback(void *arg)
{
	int size;
	uint8_t key_value;

	size = read(CONSOLE_FD, &key_value, 1);
	if(0 == size) {
		key_value = 4;
	}

	if(3 == key_value) {
		return 1;
	}

	gl_console.keyboard->key_id_process(key_value);

	return 0;
}
EPOLL_CREATE_SIMPLE(console_recv, CONSOLE_FD, console_recv_callback);

#if 0
/*===================================================================================*/
/* For console shortcut define                                                      */
/*===================================================================================*/
static void console_f1_f4_callback(void *data)
{
	switch(*(uint8_t *)data) {
	case KEY_FUNC_F1:
		console("[cmdline_size=%d, tabcmd_size=%d, tablast=%d]",
			gl_console.cmdline_size, gl_console.tabcmd_size, gl_console.tablast);
		break;
	default:
		CONSOLE_LOG(INFO, "This is a [%s]\n", gl_keys_table[*(uint8_t *)data].str);
	}
}
SHORTCUT_CREATE(KEY_COM_NULL, KEY_FUNC_F1, KEY_FUNC_F4, console_f1_f4_callback);
#endif
#endif

/*===================================================================================*/
/* For console cmd function													         */
/*===================================================================================*/
#if COBRA_CMD_ENABLE
/*===================================================================================*/
/* For console cmd function													         */
/*===================================================================================*/
#if COBRA_LIST_CMD_ENABLE
static void console_cmd_list(void *cmd)
{
#if (LOG_CONSOLE_LEVEL > LOG_LEVEL_NOT)
	uint8_t i = 0;
#endif
	COBRA_CMD_S *pos;

	CONSOLE_LOG(INFO, "============================================================\r\n");
	list_for_each_entry(pos, &gl_console.cmd_head.list, COBRA_CMD_S, list) {
		if(strlen(pos->subcmd)) {
			CONSOLE_LOG(INFO, "%-2d: %s_%s\r\n", i++, pos->prefix, pos->subcmd);
		}
		else {
			CONSOLE_LOG(INFO, "%-2d: %s\r\n", i++, pos->prefix);
		}
	}
	CONSOLE_LOG(INFO, "============================================================\r\n");
}
CMD_CREATE(cmd, list, console_cmd_list);

static void cobra_testcmd1(void *cmd)
{
	CONSOLE_LOG(INFO, "test cmd 1\r\n");
}
CMD_CREATE_SIMPLE(testcmd1, cobra_testcmd1);

static void cobra_testcmd2(void *cmd)
{
	CONSOLE_LOG(INFO, "test cmd 2\r\n");
}
CMD_CREATE_SIMPLE(testcmd2, cobra_testcmd2);

#endif /* COBRA_LIST_CMD_ENABLE */
/*===================================================================================*/
void cobra_console_cmd_register(COBRA_CMD_S *cmd)
{
	if(CBA_SUCCESS == cobra_cmd_register(&gl_console.cmd_head, cmd)) {
		if(strlen(cmd->subcmd)) {
			CONSOLE_LOG(INFO, "cmd_register: [%s_%s]\r\n", cmd->prefix, cmd->subcmd);
		}
		else {
			CONSOLE_LOG(INFO, "cmd_register: [%s]\r\n", cmd->prefix);
		}
	}
}
#endif /* COBRA_CMD_ENABLE */
/*===================================================================================*/

void cobra_console_deinit(void)
{
#if COBRA_CMD_ENABLE
	epoll_bind_deregister(&gl_console.epoll, &epoll_console_recv);
	cobra_keyboard_bind(&gl_console.keyboard, CBA_DISABLE);
	console("\n%sQuit\n", CONSOLE_TAG);
#endif
}

int cobra_console_init(void)
{
#if COBRA_CONSOLE_ENABLE
	console("\r\n+====================================================================+\r\n");
	console(COBRA_SYSTEM_NAME);
	console(COBRA_SYSTEM_VERSION);
	console("| author  : sm723@qq.com                                             |\r\n");
	console("| build   : v0.1.0 - 21/01/18                                        |\r\n");
	console("+====================================================================+\r\n");

#if COBRA_CMD_ENABLE
	epoll_bind_register(&gl_console.epoll, &epoll_console_recv, EP_TYPE_IN);

	cobra_keyboard_bind(&gl_console.keyboard, CBA_ENABLE);
	gl_console.keyboard->key_backspace_callback = _console_cmdline_backspace;
	gl_console.keyboard->key_tab_callback = _console_cmdline_tab;
	gl_console.keyboard->key_enter_callback = _console_cmdline_enter;
	gl_console.keyboard->key_normol_callback = _console_cmdline_normal;
	gl_console.keyboard->key_arrow_callback = _console_cmdline_arrow;

	//cobra_keyboard_shortcut_register(&shortcut_console_f1_f4_callback);

	cobra_cmd_init(&gl_console.cmd_head);
#if COBRA_LIST_CMD_ENABLE
	cobra_console_cmd_register(&cmd_cmd_list);
#endif
	cobra_console_cmd_register(&cmd_testcmd1);
	cobra_console_cmd_register(&cmd_testcmd2);
#endif
#endif
	CONSOLE_LOG(INFO, "%s ... OK\n", __func__);

	return 0;
}
