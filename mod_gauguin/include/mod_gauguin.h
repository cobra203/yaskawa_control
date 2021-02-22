#ifndef _MOD_GAUGUIN_H_
#define _MOD_GAUGUIN_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <data.h>
#include <can_driver.h>

#include <cobra_linux.h>
#include <cobra_sys.h>

#define LOG_GAUGUIN_LEVEL	LOG_LEVEL_INFO

#if (LOG_GAUGUIN_LEVEL > LOG_LEVEL_NOT)
#define GAUGUIN_INFO(fm, ...) { \
		console_cmdline_clean(); \
		console("GAUGUIN  : " fm, ##__VA_ARGS__); \
		console_cmdline_restore(); \
	}
#else
#define GAUGUIN_INFO(fm, ...)
#endif

#if (LOG_GAUGUIN_LEVEL > LOG_LEVEL_INFO)
#define GAUGUIN_DEBUG(fm, ...) { \
		console_cmdline_clean(); \
		console("GAUGUIN  : " fm, ##__VA_ARGS__); \
		console_cmdline_restore(); \
	}
#else
#define GAUGUIN_DEBUG(fm, ...)
#endif

#define GAUGUIN_LOG(level, fm, ...) GAUGUIN_##level(fm, ##__VA_ARGS__)

#define GAUGUIN_BUSNAME		"0"
#define GAUGUIN_BAUDRATE	"125K"

#define SLAVE_NODE_MAX		5

typedef struct parm_comm_s
{
	CO_Data		*od;
	uint16_t	index_parm;					/* 1000 */
	uint16_t	index_mapp;					/* 0 */
	uint16_t	store_base_index;			/* 2060 */
	uint8_t		store_base_subindex;
	uint32_t	sync_cobid;					/* 2060:01 */
	uint32_t	sync_cycle;					/* 2060:02 */
	uint16_t	guard_time;					/* 2060:03 */
	uint8_t		lift_tim;					/* 2060:04 */
	uint32_t	emcy_cobid;					/* 2060:05 */
	uint32_t	consumer_heartbeat_time;	/* 2060:06 */
	uint16_t	producer_heartbeat_time;	/* 2060:07 */
	uint8_t		communication_error;		/* 2060:08 */
} PARM_COMM_S;

typedef struct parm_rpdo_s
{
	CO_Data		*od;
	uint16_t	index_parm;					/* 1400 */
	uint16_t	index_mapp;					/* 1600 */
	uint16_t	store_base_index;			/* 2070 */
	uint8_t		store_base_subindex;
	uint32_t	cobid;						/* 207x:x1 */
	uint8_t		transmission_type;			/* 207x:x2 */
	uint8_t		mapping_size;				/* 207x:x3 */
	uint32_t	mapping[8];					/* 207x:x8 ~ 207x:xf */
} PARM_RPDO_S;

typedef struct parm_tpdo_s
{
	CO_Data		*od;
	uint16_t	index_parm;					/* 1800 */
	uint16_t	index_mapp;					/* 1A00 */
	uint16_t	store_base_index;			/* 2080 */
	uint8_t		store_base_subindex;
	uint32_t	cobid;						/* 208x:x1 */
	uint32_t	transmission_type;			/* 208x:x2 */
	uint32_t	inhibit_time;				/* 208x:x3 */
	uint16_t	event_time;					/* 208x:x4 */
	uint8_t		mapping_size;				/* 208x:x5 */
	uint32_t	mapping[8];					/* 208x:x8 ~ 208x:xf */
} PARM_TPDO_S;

#define _READ_LOCAL_DICT(OD, INDEX, SUBID, SIZE, TYPE, DATA, ACCESS_TYPE) \
({ \
	uint32_t data_size = (SIZE); \
	uint8_t data_type = (TYPE); \
	uint32_t ret = 0; \
	ret = readLocalDict( \
			(OD),				/* CO_Data* d*/ \
			(INDEX),			/* UNS16 index */ \
			(SUBID),			/* UNS8 subind */ \
			(DATA),				/* void *pSourceData,*/ \
			&data_size,			/* UNS32 *pExpectedSize */ \
			&data_type,			/* UNS8 *pDataType */ \
			(ACCESS_TYPE));		/* UNS8 checkAccess */ \
	ret; \
})

#define _WRITE_LOCAL_DICT(OD, INDEX, SUBID, SIZE, DATA, ACCESS_TYPE) \
({ \
	uint32_t data = (DATA); \
	uint32_t data_size = (SIZE); \
	uint32_t ret = 0; \
	ret = writeLocalDict( \
			(OD),				/* CO_Data* d*/ \
			(INDEX),			/* UNS16 index */ \
			(SUBID),			/* UNS8 subind */ \
			&data,				/* void *pSourceData,*/ \
			&data_size,			/* UNS32 *pExpectedSize */ \
			(ACCESS_TYPE));		/* UNS8 checkAccess */ \
	ret; \
})

#define READ_MASTER_DICT(OD, INDEX, SUBID, SIZE, TYPE, DATA, STR) \
do { \
	uint32_t ret = 0; \
	ret = _READ_LOCAL_DICT((OD), (INDEX), (SUBID), (SIZE), (TYPE), (DATA), RW); \
	if(ret) { \
		GAUGUIN_LOG(INFO, "Master : Read LocalDict : [%04x:%02x], abort_codes[%08x], Failure, [%s]\n", \
			(INDEX), (SUBID), ret, (STR)); \
		return CBA_FAILURE; \
	} \
	else { \
		GAUGUIN_LOG(INFO, "Master : Read LocalDict : [%04x:%02x], 0x%08x, Success [%s]\n", \
			(INDEX), (SUBID), *(DATA), (STR)); \
	} \
} while(0)

#define WRITE_MASTER_DICT(OD, INDEX, SUBID, SIZE, DATA, STR) \
do { \
	uint32_t ret = 0; \
	ret = _WRITE_LOCAL_DICT(OD, (INDEX), (SUBID), (SIZE), (DATA), RW); \
	GAUGUIN_LOG(INFO, "################################################################################\n"); \
	if(ret) { \
		GAUGUIN_LOG(INFO, "Master : write LocalDict : [%04x:%02x], abort_codes[%08x], [%s] Failure\n", \
			(INDEX), (SUBID), ret, (STR)); \
		GAUGUIN_LOG(INFO, "################################################################################\n"); \
		return CBA_FAILURE; \
	} \
	else { \
		GAUGUIN_LOG(INFO, "Master : write LocalDict : [%04x:%02x], 0x%08x, [%s] Success\n", \
			(INDEX), (SUBID), (DATA), (STR)); \
		GAUGUIN_LOG(INFO, "################################################################################\n"); \
	} \
} while(0)

#define READ_STORE_DICT(PARM, OS_SUBID, SIZE, TYPE, DATA, STR) \
do { \
	uint32_t ret = 0; \
	ret = _READ_LOCAL_DICT((PARM)->od, (PARM)->store_base_index, (PARM)->store_base_subindex + (OS_SUBID), \
		(SIZE), (TYPE), (DATA), RW); \
	if(ret) { \
		GAUGUIN_LOG(INFO, "Master : Read LocalDict : [%04x:%02x], abort_codes[%08x], [%s] Failure\n", \
			(PARM)->store_base_index, (PARM)->store_base_subindex + (OS_SUBID), ret, (STR)); \
		return CBA_FAILURE; \
	} \
	else { \
		GAUGUIN_LOG(INFO, "Master : Read LocalDict : [%04x:%02x], 0x%08x, [%s] Success\n", \
			(PARM)->store_base_index, (PARM)->store_base_subindex + (OS_SUBID), *(DATA), (STR)); \
	} \
} while(0)


#define WRITE_PARM_DICT(PARM, SO_ID, SUBID, SIZE, DATA, STR) \
do { \
	uint32_t ret = 0; \
	ret = _WRITE_LOCAL_DICT((PARM)->od, (PARM)->index_parm + (SO_ID), (SUBID), (SIZE), (DATA), RW); \
	if(ret) { \
		GAUGUIN_LOG(INFO, "Master : write LocalDict : [%04x:%02x], abort_codes[%08x], [%s] Failure\n", \
			(PARM)->index_parm + (SO_ID), (SUBID), ret, (STR)); \
		return CBA_FAILURE; \
	} \
	else { \
		GAUGUIN_LOG(INFO, "Master : write LocalDict : [%04x:%02x], 0x%08x, [%s] Success\n", \
			(PARM)->index_parm + (SO_ID), (SUBID), (DATA), (STR)); \
	} \
} while(0)

#define WRITE_MAPP_DICT(PARM, SO_ID, SUBID, SIZE, DATA, STR) \
do { \
	uint32_t ret = 0; \
	ret = _WRITE_LOCAL_DICT((PARM)->od, (PARM)->index_mapp + (SO_ID), (SUBID), (SIZE), (DATA), RW); \
	if(ret) { \
		GAUGUIN_LOG(INFO, "Master : write LocalDict : [%04x:%02x], abort_codes[%08x], [%s] Failure\n", \
			(PARM)->index_mapp + (SO_ID), (SUBID), ret, (STR)); \
		return CBA_FAILURE; \
	} \
	else { \
		GAUGUIN_LOG(INFO, "Master : write LocalDict : [%04x:%02x], 0x%08x, [%s] Success\n", \
			(PARM)->index_mapp + (SO_ID), (SUBID), (DATA), (STR)); \
	} \
} while(0)

#define _WRITE_NETWORK_DICT(OD, NODEID, ID, SUBID, SIZE, TYPE, DATA, CALLBACK, MODE) \
({ \
	uint32_t data = (DATA); \
	uint32_t ret = 0; \
	ret = writeNetworkDictCallBack( \
			(OD),							/* CO_Data* d*/ \
			(NODEID),						/* UNS8 nodeId */ \
			(ID),							/* UNS16 index */ \
			(SUBID),						/* UNS8 subind */ \
			(SIZE),							/* UNS8 pExpectedSize */ \
			(TYPE),							/* UNS8 dataType */ \
			&data,							/* void *pSourceData,*/ \
			(CALLBACK),						/* SDOCallback_t Callback */ \
			(MODE));						/* UNS8 use Block Mode */ \
	ret; \
})

#define WRITE_SLAVE_DICT(NODE, SO_ID, SUBID, SIZE, DATA, CALLBACK, STR) \
do { \
	uint32_t ret = 0; \
	(NODE)->operate_pstr = STR; \
	GAUGUIN_LOG(INFO, "################################################################################\n"); \
	GAUGUIN_LOG(INFO, "Slave[%d] : write NetworkDict : [%04x:%02x], 0x%08x, [%s] Start\n", \
			(NODE)->id, (SO_ID), (SUBID), (DATA), (STR)); \
	GAUGUIN_LOG(INFO, "################################################################################\n"); \
	ret = _WRITE_NETWORK_DICT((NODE)->host_od, (NODE)->id, (SO_ID), (SUBID), (SIZE), 0, (DATA), (CALLBACK), 0); \
	if(ret) { \
		GAUGUIN_LOG(INFO, "################################################################################\n"); \
		GAUGUIN_LOG(INFO, "Slave[%d] : write NetworkDict : [%04x:%02x], error_codes[%02x], Failure, [%s]\n", \
			(NODE)->id, (SO_ID), (SUBID), ret, (STR)); \
		GAUGUIN_LOG(INFO, "################################################################################\n"); \
		return CBA_FAILURE; \
	} \
} while(0)

#define _READ_NETWORK_DICT(OD, NODEID, ID, SUBID, TYPE, CALLBACK, MODE) \
({ \
	uint32_t ret = 0; \
	ret = readNetworkDictCallback( \
			(OD),							/* CO_Data* d*/ \
			(NODEID),						/* UNS8 nodeId */ \
			(ID),							/* UNS16 index */ \
			(SUBID),						/* UNS8 subind */ \
			(TYPE),							/* UNS8 dataType */ \
			(CALLBACK),						/* SDOCallback_t Callback */ \
			(MODE));						/* UNS8 use Block Mode */ \
	ret; \
})

#define READ_SLAVE_DICT(NODE, SO_ID, SUBID, CALLBACK, STR) \
do { \
	uint32_t ret = 0; \
	(NODE)->operate_pstr = STR; \
	GAUGUIN_LOG(INFO, "################################################################################\n"); \
	GAUGUIN_LOG(INFO, "Slave[%d] : read NetworkDict : [%04x:%02x], [%s] Start\n", \
			(NODE)->id, (SO_ID), (SUBID), (STR)); \
	GAUGUIN_LOG(INFO, "################################################################################\n"); \
	ret = _READ_NETWORK_DICT((NODE)->host_od, (NODE)->id, (SO_ID), (SUBID), 0, (CALLBACK), 0); \
	if(ret) { \
		GAUGUIN_LOG(INFO, "################################################################################\n"); \
		GAUGUIN_LOG(INFO, "Slave[%d] : read NetworkDict : [%04x:%02x], error_codes[%02x], Failure, [%s]\n", \
			(NODE)->id, (SO_ID), (SUBID), ret, (STR)); \
		GAUGUIN_LOG(INFO, "################################################################################\n"); \
		return CBA_FAILURE; \
	} \
} while(0)

struct can_node_s;
typedef CBA_BOOL (*node_operate_t)(struct can_node_s *);
typedef CBA_BOOL (*node_read_t)(struct can_node_s *);

typedef struct can_node_s
{
	uint8_t			id;
	uint8_t			master_id;
	uint8_t			status;
	CO_Data			*host_od;
	PARM_COMM_S		*parm_comm;
	PARM_RPDO_S 	*parm_rpdo;
	PARM_TPDO_S 	*parm_tpdo;
	uint8_t			step;
	void			*operate_args;
	node_operate_t	next_step;
	node_operate_t	complete;
	node_operate_t	offline;
	char			*operate_pstr;

	void			*read_result;
	uint8_t			read_step;
	node_read_t		read_next_step;
	node_read_t		read_complete;

	uint8_t			heartbeat_index;
	LIST_S			list;
} CAN_NODE_S;

typedef struct gauguin_status_s
{
	uint8_t		enable	:1;
	uint8_t				:7;
} GAUGUIN_STATUS_S;

typedef struct mod_gauguin_s
{
	SYS_STATUS_S		*sys_status;
	GAUGUIN_STATUS_S	status;
	s_BOARD				board;
	int					sock_fd;
	PARM_COMM_S			parm_comm;
	PARM_RPDO_S			parm_rpdo[4];
	PARM_TPDO_S			parm_tpdo[4];
	uint8_t				node_num;
	uint8_t				heartbeat_enable;
	uint8_t				sync_enable;
	CO_Data				*od;
	uint8_t				id;
} MOD_GAUGUIN_S;

extern MOD_GAUGUIN_S gl_mod_gauguin;

void mod_gauguin_deinit(void);
void mod_gauguin_init(COBRA_SYS_S *sys);

CBA_BOOL struct_communications_parameters_init(CO_Data *od, PARM_COMM_S *com);
CBA_BOOL struct_recive_pdo_parameters_init(CO_Data *od, PARM_RPDO_S *rpdo);
CBA_BOOL slave_write_complete(CO_Data* d, UNS8 nodeId);
CBA_BOOL slave_read_complete(CO_Data* d, UNS8 nodeId);

void CheckSDOAnd_write_callback(CO_Data* d, UNS8 nodeId);
void CheckSDOAnd_read_callback(CO_Data* d, UNS8 nodeId);

#ifdef __cplusplus
}
#endif

#endif /* _GAUGUIN_H_ */
