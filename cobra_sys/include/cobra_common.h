#ifndef _COBRA_COMMON_H_
#define _COBRA_COMMON_H_

/************************************************************/
/* top head files for all                                   */
/************************************************************/

#ifdef __cplusplus
 extern "C" {
#endif

#if defined(USE_LINUX_DRIVER)
#define _LINUX_DRIVER_
#else
/*****************************/
#error "Please specifie platform in defined symbols on IAR"
/*****************************/
#endif

/************************************************************/

/************************************************************/

/************************************************************/
/* platform head files                                      */
/************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#if defined(_LINUX_DRIVER_)
#include <stdint.h>
#endif

/************************************************************/
/* common macro                                             */
/************************************************************/
#define MASK_SET(s)		(0x1UL << (s))
#define BIT_ISSET(a, s) (((a) >> (s)) & 0x1)
#define BIT_SET(a, s)   ((a) = (a) | 0x1 << (s))
#define BIT_CLR(a, s)   ((a) = (a) & ~(0x1 << (s)))

#define CBA_SUCCESS     0
#define CBA_FAILURE     -1

#define CBA_TRUE        1
#define CBA_FALSE       0

#define CBA_ENABLE      CBA_TRUE
#define CBA_DISABLE     CBA_FALSE

#define CBA_HIGH		CBA_TRUE
#define CBA_LOW			CBA_FALSE

#define CBA_NULL		((void *)0)
#define CBA_BOOL		uint8_t

#ifdef __cplusplus
}
#endif

#endif /* _COBRA_COMMON_H_ */
