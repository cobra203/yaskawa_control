#include <cobra_debug.h>

#if (LOG_DEBUG_LEVEL > LOG_LEVEL_NOT)
#define COBRA_DUMP_START()	console_cmdline_clean()
#define COBRA_DUMP_END()	console_cmdline_restore()
#define COBRA_DUMP(fm, ...) console(fm, ##__VA_ARGS__)
#else
#define COBRA_DUMP_START()
#define COBRA_DUMP_END()
#define COBRA_DUMP(fm, ...)
#endif

#if COBRA_CONSOLE_ENABLE
void cobra_dump(uint8_t *mod_name, uint8_t *title, const void *data, uint32_t len) {
    int     i = 0;
    uint8_t *pdata = (uint8_t *)data;

	COBRA_DUMP_START();

    COBRA_DUMP("%-8s: HEXDUMP[%d] : %s\r\n", mod_name, len, title);
	COBRA_DUMP("%-8s: ============================================================\r\n", mod_name);
	COBRA_DUMP("%-8s: ", mod_name);
    for(i = 0; i < len; i++) {
        COBRA_DUMP("%02x", pdata[i]);
        if(i != len - 1) {
            COBRA_DUMP(" ");
        }
    }
    COBRA_DUMP("\r\n%-8s: ============================================================\r\n", mod_name);

	COBRA_DUMP_END();
}
#endif
