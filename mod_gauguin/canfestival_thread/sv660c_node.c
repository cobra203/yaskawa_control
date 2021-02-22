#include <sv660c_node.h>
#include <sv660c_master.h>

typedef struct DATA_ITEM_S
{
	uint32_t	data;
	uint32_t	size;
} data_item_s;

CBA_BOOL sv660c_node_get_identity(CAN_NODE_S *node)
{
	static uint8_t step = 0;
	static node_read_t complete = CBA_NULL;;
	static node_read_t next_step = CBA_NULL;

	if(!node->read_step) {
		next_step = node->read_next_step;
		complete = node->read_complete;
		node->read_next_step = &sv660c_node_get_identity;
		step = 1;
	}
	node->read_step++;

	GAUGUIN_LOG(INFO, "###################### %s: step[%d], node->read_step[%d]\n", __func__, step, node->read_step);

	switch(step++) {
	case 1:
		READ_SLAVE_DICT(node, 0x1000, 0x00, CheckSDOAnd_read_callback, "Device Type");
		break;
	case 2:
		READ_SLAVE_DICT(node, 0x1018, 0x01, CheckSDOAnd_read_callback, "Vendor ID");
		break;
	case 3:
		READ_SLAVE_DICT(node, 0x1018, 0x02, CheckSDOAnd_read_callback, "Product Code");
		break;
	case 4:
		READ_SLAVE_DICT(node, 0x1018, 0x03, CheckSDOAnd_read_callback, "Revision Number");
		break;
	case 5:
		node->read_complete = complete;
		node->read_next_step = next_step;
		return slave_read_complete(node->host_od, node->id);
	}
	return CBA_SUCCESS;
}

static CBA_BOOL master_set_SDO_parm(CAN_NODE_S *node, uint8_t array_id)
{
	WRITE_MASTER_DICT(node->host_od, 0x1280 + array_id, 0x1, 4, 0x600 + node->id, "COB-ID Client to Server");
	WRITE_MASTER_DICT(node->host_od, 0x1280 + array_id, 0x2, 4, 0x580 + node->id, "COB-ID Server to Client");
	WRITE_MASTER_DICT(node->host_od, 0x1280 + array_id, 0x3, 1, node->id, "Node-ID of the SDO Server");

	return CBA_SUCCESS;
}

static CBA_BOOL master_set_RPDO_parm(PARM_RPDO_S *prdoes, uint8_t nodeid, uint8_t array_id)
{
	uint8_t i = 0;
	uint8_t mapp_i = 0;

	GAUGUIN_LOG(INFO, "####here############################################################################\n");
	for(i = 0; i < 4; i++) {
		WRITE_PARM_DICT(&prdoes[i], array_id * 4, 0x1, 4, (prdoes[i].cobid - 0x80 + nodeid) | 0x80000000, "RRDO COB-ID");
		WRITE_PARM_DICT(&prdoes[i], array_id * 4, 0x2, 1, prdoes[i].transmission_type, "Transmission Type");

		for(mapp_i = 0; mapp_i < prdoes[i].mapping_size; mapp_i++) {
			WRITE_MAPP_DICT(&prdoes[i], array_id * 4, 0x1 + mapp_i, 4, prdoes[i].mapping[mapp_i], "Mapping");
		}
		WRITE_MAPP_DICT(&prdoes[i], array_id * 4, 0x0, 1, prdoes[i].mapping_size, "Mapping Number");

		WRITE_PARM_DICT(&prdoes[i], array_id * 4, 0x1, 4, prdoes[i].cobid - 0x80 + nodeid, "RRDO COB-ID");
	}
	GAUGUIN_LOG(INFO, "####here############################################################################\n");

	return CBA_SUCCESS;
}

static CBA_BOOL master_set_TPDO_parm(PARM_TPDO_S *prdoes, uint8_t nodeid, uint8_t array_id)
{
	uint8_t i = 0;
	uint8_t mapp_i = 0;

	GAUGUIN_LOG(INFO, "################################################################################\n");
	for(i = 0; i < 4; i++) {
		WRITE_PARM_DICT(&prdoes[i], array_id * 4, 0x1, 4, (prdoes[i].cobid + 0x80 + nodeid) | 0x80000000, "RRDO COB-ID");
		WRITE_PARM_DICT(&prdoes[i], array_id * 4, 0x2, 1, prdoes[i].transmission_type, "Transmission Type");
		WRITE_PARM_DICT(&prdoes[i], array_id * 4, 0x3, 1, prdoes[i].inhibit_time, "Inhibit Time");
		WRITE_PARM_DICT(&prdoes[i], array_id * 4, 0x4, 1, prdoes[i].event_time, "Event Time");

		for(mapp_i = 0; mapp_i < prdoes[i].mapping_size; mapp_i++) {
			WRITE_MAPP_DICT(&prdoes[i], array_id * 4, 0x1 + mapp_i, 4, prdoes[i].mapping[mapp_i], "Mapping");
		}
		WRITE_MAPP_DICT(&prdoes[i], array_id * 4, 0x0, 1, prdoes[i].mapping_size, "Mapping Number");

		WRITE_PARM_DICT(&prdoes[i], array_id * 4, 0x1, 4, prdoes[i].cobid + 0x80 + nodeid, "RRDO COB-ID");
	}
	GAUGUIN_LOG(INFO, "################################################################################\n");

	return CBA_SUCCESS;
}


static CBA_BOOL get_index_of_consumer_heartbeat_time(CAN_NODE_S *node, uint8_t *index)
{
	uint32_t number = 0;
	uint32_t time = 0;
	uint8_t i = 0;

	*index = 0;
	READ_MASTER_DICT(node->host_od, 0x1016, 0x00, 4, uint32, &number, "Number of Entries");
	for(i = 1; i <= number; i++) {
		READ_MASTER_DICT(node->host_od, 0x1016, i, 4, uint32, &time, "Consumer Heartbeat Time");
		if(0 == time) {
			*index = i;
			return CBA_SUCCESS;
		}
	}

	return CBA_SUCCESS;
}

CBA_BOOL sv660c_node_release(CAN_NODE_S *node)
{
	if(0 == node->step) {
		return CBA_SUCCESS;
	}

	GAUGUIN_LOG(INFO, "Slave[%d] : Release\n", node->id);

	if(node->heartbeat_index) {
		WRITE_MASTER_DICT(node->host_od, 0x1016, node->heartbeat_index, 4,
			0, "Consumer Heartbeat Time");
		node->heartbeat_index = 0;
	}
	node->step = 0;

	return CBA_SUCCESS;
}

UNS32 sv660c_rpdo_recvice(CO_Data* d, UNS16 unsused_indextable, UNS8 unsused_bSubindex)
{
	GAUGUIN_LOG(INFO, "################################################################################\n");
	GAUGUIN_LOG(INFO, "[%04x:%02x]\n", unsused_indextable, unsused_bSubindex);

	switch(unsused_indextable) {
	case 0x6060:
		GAUGUIN_LOG(INFO, "OnMaster Controlword : 0x%04x\n", Controlword);
		GAUGUIN_LOG(INFO, "OnMaster Target_velocity : %d\n", Target_velocity);
		GAUGUIN_LOG(INFO, "OnMaster Modes_of_operation : %d\n", Modes_of_operation);
		break;
	case 0x6081:
		GAUGUIN_LOG(INFO, "OnMaster Target_position : %d\n", Target_position);
		GAUGUIN_LOG(INFO, "OnMaster Profile_velocity : 0x%08x\n", Profile_velocity);
		break;
	case 0x3000:
		GAUGUIN_LOG(INFO, "OnMaster Button_Start : %d\n", Button_Start);
		GAUGUIN_LOG(INFO, "OnMaster Button_Stop : %d\n", Button_Stop);
		GAUGUIN_LOG(INFO, "OnMaster Button_Reboot : %d\n", Button_Reboot);
		GAUGUIN_LOG(INFO, "OnMaster Button_Rinse : %d\n", Button_Rinse);
		GAUGUIN_LOG(INFO, "OnMaster Button_Fill : %d\n", Button_Fill);
		break;
	}
	GAUGUIN_LOG(INFO, "################################################################################\n");
	return 0;
}

typedef struct MAPPING_ARGS_S
{
	uint8_t parm_index;

} mapping_args_s;

CBA_BOOL sv660c_node_tpdo_mapping(CAN_NODE_S *node)
{
	static uint8_t step = 0;
	static node_operate_t complete = CBA_NULL;
	static node_operate_t next_step = CBA_NULL;
	mapping_args_s *args = (mapping_args_s *)node->operate_args;
	uint8_t step_cur = 0;
	uint8_t mapping_size = 0;
	uint32_t mapping_value = 0;
	uint32_t cobid = 0;

	if(!node->step) {
		next_step = node->next_step;
		complete = node->complete;
		node->next_step = &sv660c_node_tpdo_mapping;
		step = 1;
	}
	node->step++;
	step_cur = step++;

#define STEP_COM 3
	mapping_size = node->parm_rpdo[args->parm_index].mapping_size;
	cobid = node->parm_rpdo[args->parm_index].cobid;
	if(step_cur < mapping_size + STEP_COM + 2) {
		switch(step_cur) {
		case 1:
			WRITE_SLAVE_DICT(node, 0x1800+args->parm_index, 0x01, 4, (cobid-0x80+node->id)|0x80000000,
				CheckSDOAnd_write_callback, "TPDO COB-ID");
			break;
		case 2:
			WRITE_SLAVE_DICT(node, 0x1800+args->parm_index, 0x02, 1, node->parm_rpdo[args->parm_index].transmission_type,
				CheckSDOAnd_write_callback, "TPDO Transmit Type");
			break;
		case STEP_COM:
			WRITE_SLAVE_DICT(node, 0x1A00+args->parm_index, 0x00, 1, mapping_size,
				CheckSDOAnd_write_callback, "TPDO Mapping : Number of Entries");
			break;
		default:
			if(STEP_COM + mapping_size + 1 == step_cur) {
				WRITE_SLAVE_DICT(node, 0x1800+args->parm_index, 0x01, 4, (cobid-0x80+node->id),
					CheckSDOAnd_write_callback, "TPDO COB-ID");
				break;
			}
			else {
				mapping_value = node->parm_rpdo[args->parm_index].mapping[step_cur-STEP_COM-1];
				WRITE_SLAVE_DICT(node, 0x1A00+args->parm_index, step_cur-STEP_COM, 4, mapping_value,
						CheckSDOAnd_write_callback, "TPDO Mapping : mapping");
				if(STEP_COM + mapping_size == step_cur) {
					GAUGUIN_LOG(INFO, "######################: RegisterSetODentryCallBack : [%04x:%02x]\n",
						mapping_value >> 16, (mapping_value & 0xff00) >> 8);
					RegisterSetODentryCallBack(node->host_od,
						mapping_value >> 16, (mapping_value & 0xff00) >> 8, &sv660c_rpdo_recvice);
				}
			}
			break;
		}
	}
	else {
		node->complete = complete;
		node->next_step = next_step;
		return slave_write_complete(node->host_od, node->id);
	}
	return CBA_SUCCESS;
}

CBA_BOOL sv660c_node_rpdo_mapping(CAN_NODE_S *node)
{
	static uint8_t step = 0;
	static node_operate_t complete = CBA_NULL;
	static node_operate_t next_step = CBA_NULL;
	mapping_args_s *args = (mapping_args_s *)node->operate_args;
	uint8_t step_cur = 0;
	uint8_t mapping_size = 0;
	uint32_t mapping_value = 0;
	uint32_t cobid = 0;

	if(!node->step) {
		next_step = node->next_step;
		complete = node->complete;
		node->next_step = &sv660c_node_rpdo_mapping;
		step = 1;
	}
	node->step++;
	step_cur = step++;

#define STEP_COM 3
	mapping_size = node->parm_tpdo[args->parm_index].mapping_size;
	cobid = node->parm_tpdo[args->parm_index].cobid;
	if(step_cur < mapping_size + STEP_COM + 2) {
		switch(step_cur) {
		case 1:
			WRITE_SLAVE_DICT(node, 0x1400+args->parm_index, 0x01, 4, (cobid+node->master_id+0x80)|0x80000000,
				CheckSDOAnd_write_callback, "RPDO COB-ID");
			break;
		case 2:
			WRITE_SLAVE_DICT(node, 0x1400+args->parm_index, 0x02, 1, node->parm_tpdo[args->parm_index].transmission_type,
				CheckSDOAnd_write_callback, "RPDO Transmit Type");
			break;
		case STEP_COM:
			WRITE_SLAVE_DICT(node, 0x1600+args->parm_index, 0x00, 1, mapping_size,
				CheckSDOAnd_write_callback, "TPDO Mapping : Number of Entries");
			break;
		default:
			if(STEP_COM + mapping_size + 1 == step_cur) {
				WRITE_SLAVE_DICT(node, 0x1400+args->parm_index, 0x01, 4, (cobid+node->master_id+0x80),
					CheckSDOAnd_write_callback, "RPDO COB-ID");
				break;
			}
			else {
				mapping_value = node->parm_tpdo[args->parm_index].mapping[step_cur-STEP_COM-1];
				WRITE_SLAVE_DICT(node, 0x1600+args->parm_index, step_cur-STEP_COM, 4, mapping_value,
						CheckSDOAnd_write_callback, "RPDO Mapping : mapping");
			#if 0
				if(STEP_COM + mapping_size == step_cur) {
					CAN_LOG(INFO, "######################: RegisterSetODentryCallBack : [%04x:%02x]\n",
						mapping_value >> 16, (mapping_value & 0xff00) >> 8);
					RegisterSetODentryCallBack(node->host_od,
						mapping_value >> 16, (mapping_value & 0xff00) >> 8, &sv660c_rpdo_recvice);
				}
			#endif
			}
			break;
		}
	}
	else {
		node->complete = complete;
		node->next_step = next_step;
		return slave_write_complete(node->host_od, node->id);
	}
	return CBA_SUCCESS;
}

CBA_BOOL sv660c_node_tpdo_setting(CAN_NODE_S *node)
{
	static uint8_t step = 0;
	static node_operate_t complete = CBA_NULL;;
	static node_operate_t next_step = CBA_NULL;
	static mapping_args_s args = {0};

	if(!node->step) {
		next_step = node->next_step;
		complete = node->complete;
		node->next_step = &sv660c_node_tpdo_setting;
		step = 1;
	}
	node->step++;

	GAUGUIN_LOG(INFO, "###################### %s: step[%d], node->step[%d]\n", __func__, step, node->step);

	switch(step++) {
	case 1:
	case 2:
	case 3:
	case 4:
		node->step = 0;
		node->complete = &sv660c_node_tpdo_setting;
		args.parm_index = step-2;
		node->operate_args = &args;
		sv660c_node_tpdo_mapping(node);
		break;

	case 5:
		node->complete = complete;
		node->next_step = next_step;
		return slave_write_complete(node->host_od, node->id);
	}

	return CBA_SUCCESS;
}

CBA_BOOL sv660c_node_rpdo_setting(CAN_NODE_S *node)
{
	static uint8_t step = 0;
	static node_operate_t complete = CBA_NULL;;
	static node_operate_t next_step = CBA_NULL;
	static mapping_args_s args = {0};

	if(!node->step) {
		next_step = node->next_step;
		complete = node->complete;
		node->next_step = &sv660c_node_rpdo_setting;
		step = 1;
	}
	node->step++;

	GAUGUIN_LOG(INFO, "###################### %s: step[%d], node->step[%d]\n", __func__, step, node->step);

	switch(step++) {
	case 1:
	case 2:
	case 3:
	case 4:
		node->step = 0;
		node->complete = &sv660c_node_rpdo_setting;
		args.parm_index = step-2;
		node->operate_args = &args;
		sv660c_node_rpdo_mapping(node);
		break;

	case 5:
		node->complete = complete;
		node->next_step = next_step;
		return slave_write_complete(node->host_od, node->id);
	}

	return CBA_SUCCESS;
}

CBA_BOOL sv660c_node_register(CAN_NODE_S *node)
{
	static uint8_t step = 0;
	static node_operate_t complete = CBA_NULL;
	uint8_t index = 0;

	if(!node->step) {
		node->next_step = &sv660c_node_register;
		complete = node->complete;
		step = 1;
	}
	node->step++;

	switch(step++) {
	case 1:
		get_index_of_consumer_heartbeat_time(node, &index);
		if(!index) {
			return CBA_FAILURE;
		}
		WRITE_MASTER_DICT(node->host_od, 0x1016, index, 4,
			node->id << 16 | node->parm_comm->consumer_heartbeat_time, "Consumer Heartbeat Time");
		node->heartbeat_index = index;

		master_set_SDO_parm(node, node->heartbeat_index-1);

		WRITE_SLAVE_DICT(node, 0x1017, 0x00, 2, node->parm_comm->producer_heartbeat_time,
			CheckSDOAnd_write_callback, "Producer Heartbeat Time");
		break;

	case 2:
		master_set_RPDO_parm(node->parm_rpdo, node->id, node->heartbeat_index-1);
		node->step = 0;
		node->complete = &sv660c_node_register;
		sv660c_node_tpdo_setting(node);
		break;

	case 3:
		master_set_TPDO_parm(node->parm_tpdo, node->master_id, node->heartbeat_index-1);
		node->step = 0;
		node->complete = &sv660c_node_register;
		sv660c_node_rpdo_setting(node);
		break;

	case 4:
		node->complete = complete;
		return slave_write_complete(node->host_od, node->id);
	}

	return CBA_SUCCESS;
}
