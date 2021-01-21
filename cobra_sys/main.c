#include <main.h>

int main(void)
{
	cobra_epoll_init();

	cobra_console_init();

	cobra_sys_init();
	/* Infinite loop */
	while (1) {
#if (COBRA_CONSOLE_ENABLE && COBRA_CMD_ENABLE)
		if(epoll_monitor_handle()) {
			break;
		}
#endif
	}
	cobra_console_deinit();

	return 0;
}
