/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __APPLICFG_LINUX__
#define __APPLICFG_LINUX__

#ifndef __KERNEL__
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#else
#include <linux/types.h>
#endif

#include <cobra_console.h>
/*  Define the architecture : little_endian or big_endian
 -----------------------------------------------------
 Test :
 UNS32 v = 0x1234ABCD;
 char *data = &v;

 Result for a little_endian architecture :
 data[0] = 0xCD;
 data[1] = 0xAB;
 data[2] = 0x34;
 data[3] = 0x12;

 Result for a big_endian architecture :
 data[0] = 0x12;
 data[1] = 0x34;
 data[2] = 0xAB;
 data[3] = 0xCD;
 */

/* Integers */
#define INTEGER8 int8_t
#define INTEGER16 int16_t
#define INTEGER24 int32_t
#define INTEGER32 int32_t
#define INTEGER40 int64_t
#define INTEGER48 int64_t
#define INTEGER56 int64_t
#define INTEGER64 int64_t

/* Unsigned integers */
#define UNS8   u_int8_t
#define UNS16  u_int16_t
#define UNS32  u_int32_t
#define UNS24  u_int32_t
#define UNS40  u_int64_t
#define UNS48  u_int64_t
#define UNS56  u_int64_t
#define UNS64  u_int64_t

/* Reals */
#define REAL32	float
#define REAL64 double

/* Add cobra console debug macros */
#ifdef __COBRA_CONSOLE__

#if (LOG_CAN_LEVEL > LOG_LEVEL_NOT)
#define MSG_DUMP_START()	console_cmdline_clean();console("CAN      : ");
#define MSG_DUMP_END()		console_cmdline_restore()
#define MSG_DUMP(fm, ...)	console(fm, ##__VA_ARGS__)
#else
#define MSG_DUMP_START()
#define MSG_DUMP_END()
#define MSG_DUMP(fm, ...)
#endif

#if (LOG_CAN_LEVEL > LOG_LEVEL_NOT)
#define CAN_INFO(fm, ...) { \
	console_cmdline_clean(); \
	console("CAN      : " fm, ##__VA_ARGS__); \
	console_cmdline_restore(); \
}
#else
#define CAN_INFO(fm, ...)
#endif

#if (LOG_CAN_LEVEL > LOG_LEVEL_INFO)
#define CAN_DEBUG(fm, ...) { \
	console_cmdline_clean(); \
	console("CAN      : " fm, ##__VA_ARGS__); \
	console_cmdline_restore(); \
}
#else
#define CAN_DEBUG(fm, ...)
#endif

#define CAN_LOG(level, fm, ...) CAN_##level(fm, ##__VA_ARGS__)

#endif

/* Definition of error and warning macros */
/* -------------------------------------- */
#ifdef __KERNEL__
#	define MSG(...) printk (__VA_ARGS__)
//#elif defined USE_RTAI
//#	define MSG(...) rt_printk (__VA_ARGS__)
#elif defined USE_XENO
#	define MSG(...)
#else
#	include <stdio.h>
#ifdef __COBRA_CONSOLE__
#	define MSG(...) CAN_LOG(INFO, __VA_ARGS__)
#else
#	define MSG(...) printf (__VA_ARGS__)
#	define MSG_DUMP_START()
#	define MSG_DUMP_END()
#	define MSG_DUMP(fm, ...) MSG(fm, ##__VA_ARGS__)
#endif
#endif

/* Definition of MSG_ERR */
/* --------------------- */
#ifdef DEBUG_ERR_CONSOLE_ON
#    define MSG_ERR(num, str, val)            \
          MSG("%s,%d : 0X%x %s 0X%x \n",__FILE__, __LINE__,num, str, val);
#else
#    define MSG_ERR(num, str, val)
#endif

/* Definition of MSG_WAR */
/* --------------------- */
#ifdef DEBUG_WAR_CONSOLE_ON
#    define MSG_WAR(num, str, val)          \
          MSG("%s,%d : 0X%x %s 0X%x \n",__FILE__, __LINE__,num, str, val);
#else
#    define MSG_WAR(num, str, val)
#endif

typedef void* CAN_HANDLE;

typedef void* CAN_PORT;

#endif
