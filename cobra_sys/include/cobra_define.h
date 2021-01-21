#ifndef _COBRA_DEFINE_H_
#define _COBRA_DEFINE_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <cobra_common.h>

/************************************************************/
/* for project enum defind                                  */
/************************************************************/
/*
#define PROJECT_A				0
#define PROJECT_B				1
#define PROJECT_C				2
*/
/************************************************************/
/* project type                                             */
/************************************************************/
/*
#define PROJECT_TYPE			PROJECT_A
*/
/************************************************************/
/* system info                                              */
/************************************************************/
#define SYSTEM_NAME \
	"| system  : cobra system for CanFestival control                     |\r\n"
#define SYSTEM_VERSION \
	"| version : v0.1 - 2021/01/15                                        |\r\n"

/************************************************************/
/* for cmd                                                  */
/************************************************************/
#define CMD_EN					1

/************************************************************/
/* for console                                              */
/************************************************************/
#define CONSOLE_EN				1

/************************************************************/
/* for stm library                                          */
/************************************************************/
#if defined(CONSOLE_EN)
#define COBRA_CONSOLE_ENABLE	CONSOLE_EN
#else
#define COBRA_CONSOLE_ENABLE	0
#endif

#if defined(CMD_EN)
#define COBRA_CMD_ENABLE		(COBRA_CONSOLE_ENABLE && CMD_EN)
#else
#define COBRA_CMD_ENABLE		0
#endif

/************************************************************/
/* for mod_canopen                                          */
/************************************************************/
#define MOD_CANOPEN_EN			1

#ifdef __cplusplus
}
#endif

#endif /* _COBRA_DEFINE_H_ */
