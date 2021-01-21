#include <cobra_keyboard.h>
#include <cobra_console.h>

#if (LOG_SHORTCUT_LEVEL > LOG_LEVEL_NOT)
#define SHORTCUT_INFO(fm, ...) { \
	console_cmdline_clean(); \
	console("SHORTCUT : " fm, ##__VA_ARGS__); \
	console_cmdline_restore(); \
}
#else
#define SHORTCUT_INFO(fm, ...)
#endif

#if (LOG_SHORTCUT_LEVEL > LOG_LEVEL_INFO)
#define SHORTCUT_DEBUG(fm, ...) { \
	console_cmdline_clean(); \
	console("KEYBOARD : " fm, ##__VA_ARGS__); \
	console_cmdline_restore(); \
}
#else
#define SHORTCUT_DEBUG(fm, ...)
#endif

#define SHORTCUT_LOG(level, fm, ...) SHORTCUT_##level(fm, ##__VA_ARGS__)

CBA_BOOL cobra_shortcut_process(COBRA_SHORTCUT_S *head, uint8_t com_index, uint8_t key_index)
{
	COBRA_SHORTCUT_S *pos;

	list_for_each_entry(pos, &head->list, COBRA_SHORTCUT_S, list) {
		if(pos->key_index_min <= key_index &&
		   key_index <= pos->key_index_max &&
		   com_index == pos->com_index) {
			pos->process((void *)&key_index);
			return CBA_SUCCESS;
		}
	}
	return CBA_FAILURE;
}

CBA_BOOL cobra_shortcut_register(COBRA_SHORTCUT_S *head, COBRA_SHORTCUT_S *shortcut)
{
	if(head->list.next && head->list.prev) {
		list_add_tail(&shortcut->list, &head->list);
		return CBA_SUCCESS;
	}
	else {
		SHORTCUT_LOG(INFO, "%s Invalid Shortcut Head\n", __func__);
		return CBA_FAILURE;
	}
}

void cobra_shortcut_init(COBRA_SHORTCUT_S *head)
{
	INIT_LIST_HEAD(&head->list);

	SHORTCUT_LOG(INFO, "%s ... OK\n", __func__);
}
