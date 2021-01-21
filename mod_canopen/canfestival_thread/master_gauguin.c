#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>

#include <canfestival.h>
#include <can_driver.h>
#include <timers_driver.h>

#include <master_gauguin.h>
#include <master_test.h>

GAUGUIN_S gl_gauguin;

s_BOARD board_gauguin = {"0", "125K"}; //can0, baudrate ignore

#if 0
void read_network_dict(void)
{
	readNetworkDictCallbackAI();
}
#endif

void master_gauguin_deinit(void)
{
	TimerCleanup();
}

int master_gauguin_init(void)
{
	unsigned char nodeID = 0x00;

	TimerInit();

	if(!canOpen(&board_gauguin, &master_test_Data)){
		printf("Cannot open Gauguin Can%s\n", board_gauguin.busname);
		TimerCleanup();
	}

	setNodeId(&master_test_Data, nodeID);
	setState(&master_test_Data, Initialisation);
	setState(&master_test_Data, Operational);

	return 0;
}

#ifdef __cplusplus
}
#endif
