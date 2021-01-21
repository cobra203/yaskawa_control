#ifndef _COBRA_DEBUG_H_
#define _COBRA_DEBUG_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_console.h>

void cobra_dump(uint8_t *mod_name, uint8_t *title, const void *data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* _COBRA_DEBUG_H_ */
