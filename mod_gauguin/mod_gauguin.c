#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>

#include <canfestival.h>
#include <timers_driver.h>
#include <applicfg.h>
#include <lss.h>

#include <cobra_console.h>
#include <cobra_cmd.h>
#include <cobra_sys.h>
#include <mod_gauguin.h>
#include <sv660c_master.h>
//#include <TestMaster.h>
#include <sv660c_node.h>

MOD_GAUGUIN_S gl_mod_gauguin;
static CAN_NODE_S gl_can_node_head;
static int MasterSyncCount = 0;

static CBA_BOOL _master_set_producer_heartbeat_time(CO_Data *od, uint16_t ms)
{

	WRITE_MASTER_DICT(od, 0x1017, 0x00, 2, ms, "Producer Heartbeat Time");

	GAUGUIN_LOG(INFO, "Master : Producer_Heartbeat_Time set to %d ms Success\n", ms);

	return CBA_SUCCESS;
}

static CBA_BOOL _master_set_communication_cycle_period(CO_Data *od, uint32_t us)
{
	WRITE_MASTER_DICT(od, 0x1006, 0x00, 4, us, "Communication / Cycle Period");

	GAUGUIN_LOG(INFO, "Master : Communication / Cycle Period set to %d ms Success\n", us/1000);

	return CBA_SUCCESS;
}

static void _master_communication_enable(CO_Data *od, CBA_BOOL enable)
{
	if(enable) {
		_master_set_communication_cycle_period(od, gl_mod_gauguin.parm_comm.sync_cycle);
	}
	else {
		_master_set_communication_cycle_period(od, 0);
	}
}

static void _slave_node_distclean(void)
{
	CAN_NODE_S *pos = NULL;
	CAN_NODE_S *n = NULL;

	list_for_each_entry_safe(pos, n, &gl_can_node_head.list, CAN_NODE_S, list) {
		pos->offline(pos);
		list_del(&pos->list);
		free(pos);
		gl_mod_gauguin.node_num--;
	}
}

static CBA_BOOL slave_read_callback(CAN_NODE_S *node)
{
	uint32_t abortCode;
	uint32_t data = 0;
	uint32_t buffer_size = 4;
	CBA_BOOL ret = CBA_FAILURE;

	if(getReadResultNetworkDict(node->host_od, node->id, &data, &buffer_size, &abortCode) != SDO_FINISHED) {
		GAUGUIN_LOG(INFO, "################################################################################\n");
		GAUGUIN_LOG(INFO, "Slave[%d] : SDO_FAILED, read NetworkDict [%s] Failure, AbortCode : %4.4x\n",
				node->id, node->operate_pstr, abortCode);
	}
	else {
		GAUGUIN_LOG(INFO, "################################################################################\n");
		GAUGUIN_LOG(INFO, "Slave[%d] : SDO_FINISHED read NetworkDict [%s] : [%02x:%08x] Success\n",
				node->id, node->operate_pstr, buffer_size, data);
		ret = CBA_SUCCESS;

	}
	GAUGUIN_LOG(INFO, "################################################################################\n");

	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(node->host_od, node->id, SDO_CLIENT);

	return ret;
}

CBA_BOOL slave_read_complete(CO_Data* d, UNS8 nodeId)
{
	CAN_NODE_S *pos = NULL;
	CAN_NODE_S *n = NULL;

	list_for_each_entry_safe(pos, n, &gl_can_node_head.list, CAN_NODE_S, list) {
		if(nodeId == pos->id) {
			if(pos->read_complete && CBA_SUCCESS != pos->read_complete(pos)) {
				pos->offline(pos);
				list_del(&pos->list);
				free(pos);
				gl_mod_gauguin.node_num--;
				return CBA_FAILURE;
			}
			break;
		}
	}
	return CBA_SUCCESS;
}

void CheckSDOAnd_read_callback(CO_Data* d, UNS8 nodeId)
{
	CAN_NODE_S *pos = NULL;
	CAN_NODE_S *n = NULL;

	list_for_each_entry_safe(pos, n, &gl_can_node_head.list, CAN_NODE_S, list) {
		if(nodeId == pos->id) {
			if(CBA_SUCCESS != slave_read_callback(pos)) {
				pos->offline(pos);
				list_del(&pos->list);
				free(pos);
				gl_mod_gauguin.node_num--;
				break;
			}
			if(pos->read_next_step) {
				pos->read_next_step(pos);
			}
			break;
		}
	}
}

static CBA_BOOL slave_write_callback(CAN_NODE_S *node)
{
	uint32_t abortCode;
	CBA_BOOL ret = CBA_FAILURE;

	if(getWriteResultNetworkDict(node->host_od, node->id, &abortCode) != SDO_FINISHED) {
		GAUGUIN_LOG(INFO, "################################################################################\n");
		GAUGUIN_LOG(INFO, "Slave[%d] : SDO_FAILED, write NetworkDict [%s] Failure, AbortCode : %4.4x\n",
				node->id, node->operate_pstr, abortCode);
	}
	else {
		GAUGUIN_LOG(INFO, "################################################################################\n");
		GAUGUIN_LOG(INFO, "Slave[%d] : SDO_FINISHED write NetworkDict [%s] Success\n",
				node->id, node->operate_pstr);
		ret = CBA_SUCCESS;

	}
	GAUGUIN_LOG(INFO, "################################################################################\n");

	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(node->host_od, node->id, SDO_CLIENT);

	return ret;
}

CBA_BOOL slave_write_complete(CO_Data* d, UNS8 nodeId)
{
	CAN_NODE_S *pos = NULL;
	CAN_NODE_S *n = NULL;

	list_for_each_entry_safe(pos, n, &gl_can_node_head.list, CAN_NODE_S, list) {
		if(nodeId == pos->id) {
			if(pos->complete && CBA_SUCCESS != pos->complete(pos)) {
				pos->offline(pos);
				list_del(&pos->list);
				free(pos);
				gl_mod_gauguin.node_num--;
				return CBA_FAILURE;
			}
			break;
		}
	}
	return CBA_SUCCESS;
}

void CheckSDOAnd_write_callback(CO_Data* d, UNS8 nodeId)
{
	CAN_NODE_S *pos = NULL;
	CAN_NODE_S *n = NULL;

	list_for_each_entry_safe(pos, n, &gl_can_node_head.list, CAN_NODE_S, list) {
		if(nodeId == pos->id) {
			if(CBA_SUCCESS != slave_write_callback(pos)) {
				pos->offline(pos);
				list_del(&pos->list);
				free(pos);
				gl_mod_gauguin.node_num--;
				break;
			}
			if(pos->next_step) {
				pos->next_step(pos);
			}
			break;
		}
	}
}

static CBA_BOOL _slave_node_bootup_complete(CAN_NODE_S *node)
{
	if(1 == gl_mod_gauguin.node_num) {
		_master_communication_enable(node->host_od, CBA_ENABLE);

		GAUGUIN_LOG(INFO, "################################################################################\n");
		GAUGUIN_LOG(INFO, "bootup complete : set slave[%d] state to Operational\n", node->id);
		GAUGUIN_LOG(INFO, "################################################################################\n");

		setState(node->host_od, Operational);
	}

	masterSendNMTstateChange(node->host_od, node->id, NMT_Start_Node);
	return CBA_SUCCESS;
}

static CBA_BOOL _slave_node_get_identity_complete(CAN_NODE_S *node)
{
	GAUGUIN_LOG(INFO, "################################################################################\n");
	GAUGUIN_LOG(INFO, "get identity complete : slave[%d]\n", node->id);
	GAUGUIN_LOG(INFO, "################################################################################\n");
	return CBA_SUCCESS;
}

#if 0
static void ConfigureLSSNode(CO_Data* d);
// Step counts number of times ConfigureLSSNode is called
UNS8 init_step_LSS=1;

static void CheckLSSAndContinue(CO_Data* d, UNS8 command)
{
	UNS32 dat1;
	UNS8 dat2;

	CAN_LOG(INFO, "CheckLSS-> step %d\n", init_step_LSS);
	if(getConfigResultNetworkNode (d, command, &dat1, &dat2) != LSS_FINISHED){
			CAN_LOG(INFO, "Master : Failed in LSS comand %d.  Trying again\n", command);
	}
	else
	{
		if(dat1) {
			CAN_LOG(INFO, "%s dat1=%x\n", __func__, dat1);
		}
		if(dat2) {
			CAN_LOG(INFO, "%s dat2=%x\n", __func__, dat2);
		}
		init_step_LSS++;

		switch(command){
		case LSS_CONF_NODE_ID:
   			switch(dat1){
   				case 0: CAN_LOG(INFO, "Node ID change succesful\n");break;
   				case 1: CAN_LOG(INFO, "Node ID change error:out of range\n");break;
   				case 0xFF:CAN_LOG(INFO, "Node ID change error:specific error\n");break;
   				default:break;
   			}
   			break;
   		case LSS_CONF_BIT_TIMING:
   			switch(dat1){
   				case 0: CAN_LOG(INFO, "Baud rate change succesful\n");break;
   				case 1: CAN_LOG(INFO, "Baud rate change error: change baud rate not supported\n");break;
   				case 0xFF:CAN_LOG(INFO, "Baud rate change error:specific error\n");break;
   				default:break;
   			}
   			break;
   		case LSS_CONF_STORE:
   			switch(dat1){
   				case 0: CAN_LOG(INFO, "Store configuration succesful\n");break;
   				case 1: CAN_LOG(INFO, "Store configuration error:not supported\n");break;
   				case 0xFF:CAN_LOG(INFO, "Store configuration error:specific error\n");break;
   				default:break;
   			}
   			break;
   		case LSS_CONF_ACT_BIT_TIMING:
   			if(dat1==0){
   				UNS8 LSS_mode=LSS_WAITING_MODE;
				UNS32 SINC_cicle=50000;// us
				UNS32 size = sizeof(UNS32);

				/* The slaves are now configured (nodeId and Baudrate) via the LSS services.
   			 	* Switch the LSS state to WAITING and restart the slaves. */

				/*TODO: change the baud rate of the master!!*/
   			 	//board_gauguin.baudrate="125K";


	   			CAN_LOG(INFO, "Master : Switch Delay period finished. Switching to LSS WAITING state\n");
   				configNetworkNode(d,LSS_SM_GLOBAL,&LSS_mode,0,NULL);

   				CAN_LOG(INFO, "Master : Restarting all the slaves\n");
   				masterSendNMTstateChange (d, 0x00, NMT_Reset_Comunication);

   				CAN_LOG(INFO, "Master : Starting the SYNC producer\n");
   				writeLocalDict( d, /*CO_Data* d*/
					0x1006, /*UNS16 index*/
					0x00, /*UNS8 subind*/
					&SINC_cicle, /*void * pSourceData,*/
					&size, /* UNS8 * pExpectedSize*/
					RW);  /* UNS8 checkAccess */

				return;
			}
   			else{
   				UNS16 Switch_delay=1;
				UNS8 LSS_mode=LSS_CONFIGURATION_MODE;

	   			CAN_LOG(INFO, "Master : unable to activate bit timing. trying again\n");
				configNetworkNode(d,LSS_CONF_ACT_BIT_TIMING,&Switch_delay,0,CheckLSSAndContinue);
				return;
   			}
   			break;
		case LSS_SM_SELECTIVE_SERIAL:
   			CAN_LOG(INFO, "Slave in LSS CONFIGURATION state\n");
   			break;
   		case LSS_IDENT_REMOTE_SERIAL_HIGH:
   			CAN_LOG(INFO, "node identified\n");
   			break;
   		case LSS_IDENT_REMOTE_NON_CONF:
   			if(dat1==0) {
   				CAN_LOG(INFO, "There are no-configured remote slave(s) in the net\n");
   			}
   			else {
   				UNS16 Switch_delay=1;
				UNS8 LSS_mode=LSS_CONFIGURATION_MODE;

				/*The configuration of the slaves' nodeId ended.
				 * Start the configuration of the baud rate. */
				CAN_LOG(INFO, "Master : There are not no-configured slaves in the net\n");
				CAN_LOG(INFO, "Switching all the nodes to LSS CONFIGURATION state\n");
				configNetworkNode(d,LSS_SM_GLOBAL,&LSS_mode,0,NULL);
//				configNetworkNode(d,LSS_CONF_BIT_TIMING,&Switch_delay,&board_gauguin.baudrate,NULL);
//				CAN_LOG(INFO, "LSS=>Activate Bit Timing. baudrate=0x%x, %s\n", &board_gauguin.baudrate, *(char **)(&board_gauguin.baudrate));
//				configNetworkNode(d,LSS_CONF_ACT_BIT_TIMING,&Switch_delay,&board_gauguin.baudrate,CheckLSSAndContinue);
//				return;
   			}
   			break;
   		case LSS_INQ_VENDOR_ID:
   			CAN_LOG(INFO, "Slave VendorID %x\n", dat1);
   			break;
   		case LSS_INQ_PRODUCT_CODE:
   			CAN_LOG(INFO, "Slave Product Code %x\n", dat1);
   			break;
   		case LSS_INQ_REV_NUMBER:
   			CAN_LOG(INFO, "Slave Revision Number %x\n", dat1);
   			break;
   		case LSS_INQ_SERIAL_NUMBER:
   			CAN_LOG(INFO, "Slave Serial Number %x\n", dat1);
   			break;
   		case LSS_INQ_NODE_ID:
   			CAN_LOG(INFO, "Slave nodeid %x\n", dat1);
   			break;
#ifdef CO_ENABLE_LSS_FS
   		case LSS_IDENT_FASTSCAN:
   			if(dat1==0) {
   				CAN_LOG(INFO, "Slave node identified with FastScan\n");
   			}
   			else {
   				CAN_LOG(INFO, "There is not unconfigured node in the net\n");
   				return;
   			}
   			init_step_LSS++;
   			break;
#endif

		}
	}

	CAN_LOG(INFO, "end\n");
	ConfigureLSSNode(d);
}


/* Initial nodeID and VendorID. They are incremented by one for each slave*/
UNS8 NodeID=0x02;
UNS32 Vendor_ID=0x12345678;

/* Configuration of the nodeID and baudrate with LSS services:
 * --First ask if there is a node with an invalid nodeID.
 * --If FastScan is activated it is used to put the slave in the state â€œconfigurationâ€?
 * --If FastScan is not activated, identification services are used to identify the slave. Then
 * 	 switch mode service is used to put it in configuration state.
 * --Next, all the inquire services are used (only for example) and a valid nodeId and a
 * 	 new baudrate are assigned to the slave.
 * --Finally, the slave's LSS state is restored to â€œwaitingâ€?and all the process is repeated
 * 	 again until there isn't any node with an invalid nodeID.
 * --After the configuration of all the slaves finished the LSS state of all of them is switched
 * 	 again to "configuration" and the Activate Bit Timing service is requested. On sucessfull, the
 * 	 LSS state is restored to "waiting" and NMT state is changed to reset (by means of the NMT services).
 * */
static void ConfigureLSSNode(CO_Data* d)
{
	UNS32 Product_Code=0x90123456;
	UNS32 Revision_Number=0x78901234;
	UNS32 Serial_Number=0x56789012;
	UNS32 Revision_Number_high=0x78901240;
	UNS32 Revision_Number_low=0x78901230;
	UNS32 Serial_Number_high=0x56789020;
	UNS32 Serial_Number_low=0x56789010;
	UNS8 LSS_mode=LSS_WAITING_MODE;
	UNS8 Baud_Table=0;
	//UNS8 Baud_BitTiming=3;
	char* Baud_BitTiming="125K";
	UNS8 res;
	CAN_LOG(INFO, "ConfigureLSSNode step %d -> \n",init_step_LSS);

	switch(init_step_LSS){
		case 0:	/* LSS=>change the Baud rate */
			CAN_LOG(INFO, "LSS=>change the Baud rate\n");
			res=configNetworkNode(d,LSS_CONF_BIT_TIMING,&Baud_Table,&board_gauguin.baudrate,CheckLSSAndContinue);
			break;
		case 1:	/* LSS=>identify non-configured remote slave */
			CAN_LOG(INFO, "LSS=>identify no-configured remote slave(s)\n");
			res=configNetworkNode(d,LSS_IDENT_REMOTE_NON_CONF,0,0,CheckLSSAndContinue);
			break;
#ifdef CO_ENABLE_LSS_FS
		case 2:	/* LSS=>FastScan */
		{
			lss_fs_transfer_t lss_fs;
			CAN_LOG(INFO, "LSS=>FastScan\n");
			/* The VendorID and ProductCode are partialy known, except the last two digits (8 bits). */
			lss_fs.FS_LSS_ID[0]=Vendor_ID;
			lss_fs.FS_BitChecked[0]=8;
			lss_fs.FS_LSS_ID[1]=Product_Code;
			lss_fs.FS_BitChecked[1]=8;
			/* serialNumber and RevisionNumber are unknown, i.e. the 8 digits (32bits) are unknown. */
			lss_fs.FS_BitChecked[2]=32;
			lss_fs.FS_BitChecked[3]=32;
			res=configNetworkNode(d,LSS_IDENT_FASTSCAN,&lss_fs,0,CheckLSSAndContinue);
		}
		break;
#else
		case 2:	/* LSS=>identify node */
			CAN_LOG(INFO, "LSS=>identify node\n");
			res=configNetworkNode(d,LSS_IDENT_REMOTE_VENDOR,&Vendor_ID,0,NULL);
			res=configNetworkNode(d,LSS_IDENT_REMOTE_PRODUCT,&Product_Code,0,NULL);
			res=configNetworkNode(d,LSS_IDENT_REMOTE_REV_LOW,&Revision_Number_low,0,NULL);
			res=configNetworkNode(d,LSS_IDENT_REMOTE_REV_HIGH,&Revision_Number_high,0,NULL);
			res=configNetworkNode(d,LSS_IDENT_REMOTE_SERIAL_LOW,&Serial_Number_low,0,NULL);
			res=configNetworkNode(d,LSS_IDENT_REMOTE_SERIAL_HIGH,&Serial_Number_high,0,CheckLSSAndContinue);
			break;
		case 3: /*LSS=>put in configuration mode*/
			CAN_LOG(INFO, "LSS=>put in configuration mode\n");
			res=configNetworkNode(d,LSS_SM_SELECTIVE_VENDOR,&Vendor_ID,0,NULL);
			res=configNetworkNode(d,LSS_SM_SELECTIVE_PRODUCT,&Product_Code,0,NULL);
			res=configNetworkNode(d,LSS_SM_SELECTIVE_REVISION,&Revision_Number,0,NULL);
			res=configNetworkNode(d,LSS_SM_SELECTIVE_SERIAL,&Serial_Number,0,CheckLSSAndContinue);
			Vendor_ID++;
			break;
#endif
		case 4:	/* LSS=>inquire nodeID */
			CAN_LOG(INFO, "LSS=>inquire nodeID\n");
			res=configNetworkNode(d,LSS_INQ_NODE_ID,0,0,CheckLSSAndContinue);
			break;
		case 5:	/* LSS=>inquire VendorID */
			CAN_LOG(INFO, "LSS=>inquire VendorID\n");
			res=configNetworkNode(d,LSS_INQ_VENDOR_ID,0,0,CheckLSSAndContinue);
			break;
		case 6:	/* LSS=>inquire Product code */
			CAN_LOG(INFO, "LSS=>inquire Product code\n");
			res=configNetworkNode(d,LSS_INQ_PRODUCT_CODE,0,0,CheckLSSAndContinue);
			break;
		case 7:	/* LSS=>inquire Revision Number */
			CAN_LOG(INFO, "LSS=>inquire Revision Number\n");
			res=configNetworkNode(d,LSS_INQ_REV_NUMBER,0,0,CheckLSSAndContinue);
			break;
		case 8:	/* LSS=>inquire Serial Number */
			CAN_LOG(INFO, "LSS=>inquire Serial Number\n");
			res=configNetworkNode(d,LSS_INQ_SERIAL_NUMBER,0,0,CheckLSSAndContinue);
			break;
		case 9:	/* LSS=>change the nodeID */
			CAN_LOG(INFO, "LSS=>change the nodeId\n");
			res=configNetworkNode(d,LSS_CONF_NODE_ID,&NodeID,0,CheckLSSAndContinue);
			NodeID++;
			break;
		case 10:	/* LSS=>change the Baud rate */
			CAN_LOG(INFO, "LSS=>change the Baud rate\n");
			res=configNetworkNode(d,LSS_CONF_BIT_TIMING,&Baud_Table,&board_gauguin.baudrate,CheckLSSAndContinue);
			break;
		case 11:
			/*LSS=>store configuration*/
			CAN_LOG(INFO, "LSS=>store configuration\n");
			res=configNetworkNode(d,LSS_CONF_STORE,0,0,CheckLSSAndContinue);
			break;
		case 12: /* LSS=>put in waiting mode */
			CAN_LOG(INFO, "LSS=>put in waiting mode\n");
			res=configNetworkNode(d,LSS_SM_GLOBAL,&LSS_mode,0,NULL);
			/* Search again for no-configured slaves*/
			CAN_LOG(INFO, "LSS=>identify no-configured remote slave(s)\n");
			res=configNetworkNode(d,LSS_IDENT_REMOTE_NON_CONF,0,0,CheckLSSAndContinue);
			init_step_LSS=1;
			break;
	}
}
#endif

/*===================================================================================*/
/* Define heartbeatError callback for Canfestival                                    */
/*===================================================================================*/
static void _gauguin_heartbeatError(CO_Data* d, UNS8 heartbeatID)
{
	CAN_NODE_S *pos = NULL;

	GAUGUIN_LOG(INFO, "heartbeatError %d\n", heartbeatID);

	list_for_each_entry(pos, &gl_can_node_head.list, CAN_NODE_S, list) {
		if(heartbeatID == pos->id) {
			pos->offline(pos);
			list_del(&pos->list);
			free(pos);
			gl_mod_gauguin.node_num--;
			if(0 == gl_mod_gauguin.node_num) {
				_master_communication_enable(d, CBA_DISABLE);
			}
			break;
		}
	}
}

/*===================================================================================*/
/* Define initialisation callback for Canfestival                                    */
/*===================================================================================*/
void _gauguin_initialisation(CO_Data* od)
{
	GAUGUIN_LOG(INFO, "initialisation\n");

	_master_communication_enable(od, CBA_DISABLE);

	//_master_set_RPDO_parm(&gl_mod_gauguin.parm_rpdo[0], 0x2, 0x1);
}

/*===================================================================================*/
/* Define preOperational callback for Canfestival                                    */
/*===================================================================================*/
static void _gauguin_preOperational(CO_Data* d)
{
	GAUGUIN_LOG(INFO, "preOperational\n");
	/* Ask slaves to go in stop mode */
	//masterSendNMTstateChange (d, 0, NMT_Stop_Node);
	//ConfigureLSSNode(gl_gauguin.od);
	masterSendNMTstateChange (d, 0, NMT_Reset_Comunication);
}

/*===================================================================================*/
/* Define operational callback for Canfestival                                       */
/*===================================================================================*/
static void _gauguin_operational(CO_Data* d)
{
	GAUGUIN_LOG(INFO, "operational\n");
}

/*===================================================================================*/
/* Define stopped callback for Canfestival                                           */
/*===================================================================================*/
static void _gauguin_stopped(CO_Data* d)
{
	GAUGUIN_LOG(INFO, "stopped\n");
}

/*===================================================================================*/
/* Define post_sync callback for Canfestival                                         */
/*===================================================================================*/
static void _gauguin_post_sync(CO_Data* d)
{
	GAUGUIN_LOG(INFO, "post_sync\n");
}

/*===================================================================================*/
/* Define post_TPDO callback for Canfestival                                         */
/*===================================================================================*/
void _gauguin_post_TPDO(CO_Data* d)
{
	GAUGUIN_LOG(INFO, "post_TPDO : MasterSyncCount = %d \n", MasterSyncCount);
#if 0
	if(MasterSyncCount % 3 == 1){
		CAN_LOG(INFO, "Master : Ask RTR PDO (0x1402)\n");
		sendPDOrequest(gl_mod_gauguin.od, 0x1402);
		sendPDOrequest(gl_mod_gauguin.od, 0x1403);
	}
#endif
	MasterSyncCount++;
}

/*===================================================================================*/
/* Define post_emcy callback for Canfestival                                         */
/*===================================================================================*/
static void _gauguin_post_emcy(CO_Data* d,
		UNS8 nodeID, UNS16 errCode, UNS8 errReg, const UNS8 errSpec[5])
{
	GAUGUIN_LOG(INFO, "post_emcy : Master received EMCY message. Node: %2.2x  ErrorCode: %4.4x  ErrorRegister: %2.2x\n",
		nodeID, errCode, errReg);
}

/*===================================================================================*/
/* Define post_SlaveBootup callback for Canfestival                                  */
/*===================================================================================*/
static void _gauguin_node_init(CAN_NODE_S *node, CO_Data *od, uint8_t nodeid, uint8_t id)
{
	node->id = nodeid;
	node->host_od = od;
	node->master_id = id;
	node->offline = sv660c_node_release;
	node->parm_comm = &gl_mod_gauguin.parm_comm;
	node->parm_rpdo = &gl_mod_gauguin.parm_rpdo[0];
	node->parm_tpdo = &gl_mod_gauguin.parm_tpdo[0];
}

static void _gauguin_post_SlaveBootup(CO_Data* d, UNS8 nodeId)
{
	CAN_NODE_S *node = NULL;
	CAN_NODE_S *pos = NULL;

	list_for_each_entry(pos, &gl_can_node_head.list, CAN_NODE_S, list) {
		if(nodeId == pos->id) {
			GAUGUIN_LOG(INFO, "post_SlaveBootup : ID[%x] is exist\n", nodeId);
			return;
		}
	}

	node = malloc(sizeof(CAN_NODE_S));
	memset(node, 0, sizeof(CAN_NODE_S));

	_gauguin_node_init(node, d, nodeId, gl_mod_gauguin.id);

	list_add_tail(&node->list, &gl_can_node_head.list);
	gl_mod_gauguin.node_num++;

	GAUGUIN_LOG(INFO, "post_SlaveBootup : Slave Can Node ID[%x] init\n", nodeId);

	node->step = 0;
	node->complete = _slave_node_bootup_complete;
	sv660c_node_register(node);
}

static void _gauguin_master_init(CO_Data* d, UNS32 id)
{
	if(strcmp(gl_mod_gauguin.board.baudrate, "none")){

		//RegisterSetODentryCallBack(gl_mod_gauguin.od, 0x60FF, 0, &OnMaster_Target_velocity_Recvice);
		//RegisterSetODentryCallBack(gl_mod_gauguin.od, 0x3000, 0x1, &OnMaster_Modes_of_operation_Recvice);

		/* Defining the node Id */
		setNodeId(gl_mod_gauguin.od, gl_mod_gauguin.id);

		/* init */
		setState(gl_mod_gauguin.od, Initialisation);
		//setState(gl_mod_gauguin.od, Operational);
	}
}

/************************************************************/
/* mod command                                              */
/************************************************************/
#if COBRA_CMD_ENABLE
static void _node_dump(CAN_NODE_S *pos)
{
	GAUGUIN_LOG(INFO, "| nodeId | status        : %d                               |\r\n",
						pos->status);
	GAUGUIN_LOG(INFO, "| [%02x]   | heartbeat_index    : %d                           |\r\n",
						pos->id, pos->heartbeat_index);
}

static void _cmd_nodedump(void *cmd)
{
	COBRA_CMD_S *pcmd = (COBRA_CMD_S *)cmd;
	CAN_NODE_S *pos = NULL;

	GAUGUIN_LOG(INFO, "============================================================\r\n");

	if(0 != strlen(pcmd->arg)) {
		GAUGUIN_LOG(INFO, "Invalid Arguments\r\n");
		GAUGUIN_LOG(INFO, "Usage: nodedump\r\n");
	}
	else {
		list_for_each_entry(pos, &gl_can_node_head.list, CAN_NODE_S, list) {
			_node_dump(pos);
		}
	}
	GAUGUIN_LOG(INFO, "============================================================\r\n");
}
CMD_CREATE_SIMPLE(nodedump, _cmd_nodedump);

static void _cmd_nmtsend(void *cmd)
{
	COBRA_CMD_S *pcmd = (COBRA_CMD_S *)cmd;
	uint32_t nodeid = 0;
	uint32_t state = 0;

	GAUGUIN_LOG(INFO, "============================================================\r\n");

	if(0 == strlen(pcmd->arg)) {
		GAUGUIN_LOG(INFO, "Invalid Arguments\r\n");
		GAUGUIN_LOG(INFO, "Usage: nmtsend [nodeid] [state]\r\n");
		GAUGUIN_LOG(INFO, "state:\r\n");
		GAUGUIN_LOG(INFO, "       0: NMT_Stop_Node\r\n");
		GAUGUIN_LOG(INFO, "       1: NMT_Start_Node\r\n");
		GAUGUIN_LOG(INFO, "       2: NMT_Enter_PreOperational\r\n");
		GAUGUIN_LOG(INFO, "       3: NMT_Reset_Node\r\n");
		GAUGUIN_LOG(INFO, "       4: NMT_Reset_Comunication\r\n");

	}
	else {
		sscanf(pcmd->arg, "%d %d", &nodeid, &state);
		GAUGUIN_LOG(INFO, "nodeid: %d, state: %d\r\n", nodeid, state);
		switch(state) {
		case 0:
			masterSendNMTstateChange(gl_mod_gauguin.od, nodeid, NMT_Stop_Node);
			break;
		case 1:
			masterSendNMTstateChange(gl_mod_gauguin.od, nodeid, NMT_Start_Node);
			break;
		case 2:
			masterSendNMTstateChange(gl_mod_gauguin.od, nodeid, NMT_Enter_PreOperational);
			break;
		case 3:
			masterSendNMTstateChange(gl_mod_gauguin.od, nodeid, NMT_Reset_Node);
			break;
		case 4:
			masterSendNMTstateChange(gl_mod_gauguin.od, nodeid, NMT_Reset_Comunication);
			break;
		default:
			GAUGUIN_LOG(INFO, "Invalid State\r\n");
			break;
		}
	}
	GAUGUIN_LOG(INFO, "============================================================\r\n");
}
CMD_CREATE_SIMPLE(nmtsend, _cmd_nmtsend);

#endif /* COBRA_CMD_ENABLE */
/************************************************************/

/*===================================================================================*/
/* For console shortcut define                                                       */
/*===================================================================================*/
static void console_f1_f4_callback(void *data)
{
	static CBA_BOOL tpdo1_sw = CBA_FALSE;
	CAN_NODE_S *pos = NULL;
	CAN_NODE_S *n = NULL;

	switch(*(uint8_t *)data) {
	case KEY_FUNC_F1:
		if(CBA_DISABLE == gl_mod_gauguin.status.enable){
			gl_mod_gauguin.status.enable = CBA_ENABLE;
			CAN_LOG(INFO, "Shortcut key [%s] : Gauguin start\n", gl_keys_table[*(uint8_t *)data].str);
			setState(gl_mod_gauguin.od, Initialisation);
		}
		else {
			gl_mod_gauguin.status.enable = CBA_DISABLE;
			CAN_LOG(INFO, "Shortcut key [%s] : Gauguin stop\n", gl_keys_table[*(uint8_t *)data].str);
			setState(gl_mod_gauguin.od, Stopped);
			_slave_node_distclean();
		}
		break;

	case KEY_FUNC_F2:
		if(tpdo1_sw) {
			Button_Start = 0;
			//Button_Stop = 0;
			//Button_Reboot = 0;
			//Button_Rinse = 0;
			//Button_Fill = 0;
		}
		else {
			Button_Start = 1;
			//Button_Stop = 1;
			//Button_Reboot = 1;
			//Button_Rinse = 1;
			//Button_Fill = 1;
		}
		tpdo1_sw = tpdo1_sw ? CBA_FALSE : CBA_TRUE;
		CAN_LOG(INFO, "################################################################################\n");
		CAN_LOG(INFO, "Start[%d], Stop[%d], Reboot[%d], Rinse[%d], Fill[%d]\n",
					Button_Start, Button_Stop, Button_Reboot, Button_Rinse, Button_Fill);
		CAN_LOG(INFO, "################################################################################\n");

		sendPDOevent(gl_mod_gauguin.od);
		break;

	case KEY_FUNC_F3:
		list_for_each_entry_safe(pos, n, &gl_can_node_head.list, CAN_NODE_S, list) {
			if(2 == pos->id) {
				pos->read_step = 0;
				pos->read_complete = _slave_node_get_identity_complete;
				sv660c_node_get_identity(pos);
				break;
			}
		}
		break;

	case KEY_FUNC_F4:
		CAN_LOG(INFO, "============================================================\r\n");
		CAN_LOG(INFO, "| MasterSyncCount : %-10d                             |\r\n",
			MasterSyncCount);
		CAN_LOG(INFO, "============================================================\r\n");
		break;

	default:
		CAN_LOG(INFO, "Shortcut key [%s] : Undefined\n", gl_keys_table[*(uint8_t *)data].str);
	}
}
SHORTCUT_CREATE(KEY_COM_NULL, KEY_FUNC_F1, KEY_FUNC_F4, console_f1_f4_callback);

CBA_BOOL struct_communications_parameters_init(CO_Data *od, PARM_COMM_S *comm)
{
	READ_STORE_DICT(comm, 1, 4, uint32, &comm->sync_cobid, "SYNC COB-ID");
	READ_STORE_DICT(comm, 2, 4, uint32, &comm->sync_cycle, "Communication Cycle Period");
	READ_STORE_DICT(comm, 3, 2, uint16, &comm->guard_time, "Guard Time");
	READ_STORE_DICT(comm, 4, 1, uint8,	&comm->lift_tim, "Lift Tiem Factor");
	READ_STORE_DICT(comm, 5, 4, uint32, &comm->emcy_cobid, "Emergency COB-ID");
	READ_STORE_DICT(comm, 6, 4, uint32, &comm->consumer_heartbeat_time, "Consumer Heartbeat Time");
	READ_STORE_DICT(comm, 7, 2, uint16, &comm->producer_heartbeat_time, "Producer Heartbeat Time");
	READ_STORE_DICT(comm, 8, 1, uint8,	&comm->communication_error, "Communication Error");

	return CBA_SUCCESS;
}

CBA_BOOL struct_recive_pdo_parameters_init(CO_Data *od, PARM_RPDO_S *rpdo)
{
	uint8_t i = 0;

	READ_STORE_DICT(rpdo, 0x1, 4, uint32, &rpdo->cobid, "SYNC COB-ID");
	READ_STORE_DICT(rpdo, 0x2, 1, uint8,  &rpdo->transmission_type, "Transmission Type");
	READ_STORE_DICT(rpdo, 0x3, 1, uint8,  &rpdo->mapping_size, "Mapping Number");
	for(i = 0; i < 8; i++) {
		READ_STORE_DICT(rpdo, 0x8 + i, 4, uint32, &rpdo->mapping[i], "Mapping");
	}

	return CBA_SUCCESS;
}

CBA_BOOL struct_transmit_pdo_parameters_init(CO_Data *od, PARM_TPDO_S *tpdo)
{
	uint8_t i = 0;

	READ_STORE_DICT(tpdo, 0x1, 4, uint32, &tpdo->cobid, "SYNC COB-ID");
	READ_STORE_DICT(tpdo, 0x2, 1, uint8,  &tpdo->transmission_type, "Transmission Type");
	READ_STORE_DICT(tpdo, 0x3, 4, uint32, &tpdo->inhibit_time, "Inhibit Time");
	READ_STORE_DICT(tpdo, 0x4, 2, uint16, &tpdo->event_time, "Event Time");
	READ_STORE_DICT(tpdo, 0x5, 1, uint8,  &tpdo->mapping_size, "Mapping Number");
	for(i = 0; i < 8; i++) {
		READ_STORE_DICT(tpdo, 0x8 + i, 4, uint32, &tpdo->mapping[i], "Mapping");
	}

	return CBA_SUCCESS;
}

void mod_gauguin_deinit(void)
{
	TimerCleanup();
}


CBA_BOOL gl_mod_gauguin_init(void)
{
	uint8_t i = 0;

	gl_mod_gauguin.board.busname = GAUGUIN_BUSNAME;
	gl_mod_gauguin.board.baudrate = GAUGUIN_BAUDRATE;

	//gl_mod_gauguin.od = &TestMaster_Data;
	gl_mod_gauguin.od = &sv660c_master_Data;
	gl_mod_gauguin.id = 0x01;

	gl_mod_gauguin.parm_comm.od = gl_mod_gauguin.od;
	gl_mod_gauguin.parm_comm.store_base_index = 0x2060;
	gl_mod_gauguin.parm_comm.store_base_subindex = 0x0;
	gl_mod_gauguin.parm_comm.index_parm = 0x1000;
	struct_communications_parameters_init(gl_mod_gauguin.od, &gl_mod_gauguin.parm_comm);

	for(i = 0; i < 4; i++) {
		gl_mod_gauguin.parm_rpdo[i].od = gl_mod_gauguin.od;
		gl_mod_gauguin.parm_rpdo[i].store_base_index = 0x2070;
		gl_mod_gauguin.parm_rpdo[i].store_base_subindex = 0x10 * i;
		gl_mod_gauguin.parm_rpdo[i].index_parm = 0x1400 + i;
		gl_mod_gauguin.parm_rpdo[i].index_mapp = 0x1600 + i;
		struct_recive_pdo_parameters_init(gl_mod_gauguin.od, &gl_mod_gauguin.parm_rpdo[i]);
	}

	for(i = 0; i < 4; i++) {
		gl_mod_gauguin.parm_tpdo[i].od = gl_mod_gauguin.od;
		gl_mod_gauguin.parm_tpdo[i].store_base_index = 0x2080;
		gl_mod_gauguin.parm_tpdo[i].store_base_subindex = 0x10 * i;
		gl_mod_gauguin.parm_tpdo[i].index_parm = 0x1800 + i;
		gl_mod_gauguin.parm_tpdo[i].index_mapp = 0x1A00 + i;
		struct_transmit_pdo_parameters_init(gl_mod_gauguin.od, &gl_mod_gauguin.parm_tpdo[i]);
	}

	if(strcmp(gl_mod_gauguin.board.baudrate, "none")){
		gl_mod_gauguin.od->heartbeatError		= _gauguin_heartbeatError;
		gl_mod_gauguin.od->initialisation		= _gauguin_initialisation;
		gl_mod_gauguin.od->preOperational		= _gauguin_preOperational;
		gl_mod_gauguin.od->operational			= _gauguin_operational;
		gl_mod_gauguin.od->stopped				= _gauguin_stopped;
		gl_mod_gauguin.od->post_sync			= _gauguin_post_sync;
		gl_mod_gauguin.od->post_TPDO			= _gauguin_post_TPDO;
		gl_mod_gauguin.od->post_emcy			= _gauguin_post_emcy;
		gl_mod_gauguin.od->post_SlaveBootup		= _gauguin_post_SlaveBootup;

		if(!canOpen(&gl_mod_gauguin.board, gl_mod_gauguin.od)){
			GAUGUIN_LOG(INFO, "Cannot open Gauguin Can%s\n",gl_mod_gauguin.board.baudrate);
			TimerCleanup();
			return CBA_FAILURE;
		}
		else {
			GAUGUIN_LOG(INFO, "Open Gauguin Can%s\n", gl_mod_gauguin.board.busname);
		}
	}

	return CBA_SUCCESS;
}

void mod_gauguin_init(COBRA_SYS_S *sys)
{
	memset(&gl_mod_gauguin, 0, sizeof(gl_mod_gauguin));

	if(CBA_SUCCESS != gl_mod_gauguin_init()) {
		GAUGUIN_LOG(INFO, "%s Failure\n", __func__);
		return;
	}

	sys->mod->gauguin			= &gl_mod_gauguin;
	sys->status.mod->gauguin	= &gl_mod_gauguin.status;
	gl_mod_gauguin.sys_status	= &sys->status;

	INIT_LIST_HEAD(&gl_can_node_head.list);

	TimerInit();

	StartTimerLoop(&_gauguin_master_init);

	cobra_keyboard_shortcut_register(&shortcut_console_f1_f4_callback);

#if COBRA_CMD_ENABLE
	cobra_console_cmd_register(&cmd_nodedump);
	cobra_console_cmd_register(&cmd_nmtsend);
#endif

	GAUGUIN_LOG(INFO, "%s ... OK\n", __func__);
}
