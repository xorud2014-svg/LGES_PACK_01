#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "common_message.h"
#include "local_message.h"
#include "com_io.h"
#include "com_socket.h"
#include "network.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_COC_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

int InitNetwork(void)
{
	sleep(2);

	if(myPs->misc.network_socket > 0) close(myPs->misc.network_socket);
	
    myPs->misc.network_socket
		= SetClientSock(myPs->config.networkPort, (char *)&myPs->config.ipAddr);

    if(myPs->misc.network_socket < 0) {
		close(myPs->misc.network_socket);
	    userlog(DEBUG_LOG, psName, "Can not initialize network : %d\n",
			myPs->misc.network_socket);
		return -1;
    }

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.sent_monitor_data_time = myData->mData.misc.timer_1sec;
	myPs->misc.sent_monitor_data_time2 = myData->mData.misc.timer_1000ms;

	userlog(DEBUG_LOG, psName, "command socket connected : %d\n",
		myPs->misc.network_socket);
    return 0;
}

int NetworkPacket_Receive(void)
{
	char maxPacketBuf[MAX_P3_RECV_PACKET_LENGTH];
	int rcv_size, read_size, i, start, index, toPs;
	S_MSG_VAL SendMsg;

	memset(maxPacketBuf, 0, MAX_P3_RECV_PACKET_LENGTH);
		
	if(ioctl(myPs->misc.network_socket, FIONREAD, &rcv_size) < 0) {
		userlog(DEBUG_LOG, psName, "packet receive ioctl error\n");
		close(myPs->misc.network_socket);
		return -1;
	}

	if(rcv_size > MAX_P3_RECV_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName, "max packet size over\n");
		read_size = readn(myPs->misc.network_socket, maxPacketBuf,
			MAX_P3_RECV_PACKET_LENGTH);
		if(read_size != MAX_P3_RECV_PACKET_LENGTH)
			userlog(DEBUG_LOG, psName, "packet readn size error1\n");
		close(myPs->misc.network_socket);
		return -2;
	} else if(rcv_size
		> (MAX_P3_RECV_PACKET_LENGTH - myPs->rcvPacket.usedBufSize)) {
		userlog(DEBUG_LOG, psName, "packet buffer overflow\n");
		read_size = readn(myPs->misc.network_socket, maxPacketBuf, rcv_size);
		if(read_size != rcv_size)
			userlog(DEBUG_LOG, psName, "packet readn size error2\n");
		close(myPs->misc.network_socket);
		return -3;
	} else if(rcv_size <= 0) {
		userlog(DEBUG_LOG, psName, "packet sock_rcv error %d\n", rcv_size);
		close(myPs->misc.network_socket); //kjg_w

		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		toPs = COC1_TO_APP + myPs->config.groupNo;
		SendMsg.msg = MSG_COC_APP_PROCESS_KILL;
		SendMsg.val[0] = (int)myPs->config.groupNo;
		send_msg(toPs, (char *)&SendMsg);
		return -4;
	} else {
		read_size = readn(myPs->misc.network_socket, maxPacketBuf, rcv_size);
		if(read_size != rcv_size) {
			userlog(DEBUG_LOG, psName, "packet readn size error3 : %d, %d\n",
				read_size, rcv_size);
			close(myPs->misc.network_socket);
			return -5;
		}
	}
	
	//userlog(DEBUG_LOG, psName, "recvCmd %s\n", maxPacketBuf); //kjg_d

	i = myPs->rcvPacket.rcvCount;
	myPs->rcvPacket.rcvCount++;
	if(myPs->rcvPacket.rcvCount > (MAX_P3_RECV_PACKET_COUNT-1))
		myPs->rcvPacket.rcvCount = 0;
	
	if(i == 0) index = MAX_P3_RECV_PACKET_COUNT - 1;
	else index = i - 1;
	start = myPs->rcvPacket.rcvStartPoint[index]
		+ myPs->rcvPacket.rcvSize[index];
	if(start >= MAX_P3_RECV_PACKET_LENGTH) {
		myPs->rcvPacket.rcvStartPoint[i]
			= abs(start - MAX_P3_RECV_PACKET_LENGTH);
	} else {
		myPs->rcvPacket.rcvStartPoint[i] = start;
	}

	myPs->rcvPacket.rcvSize[i] = read_size;
	myPs->rcvPacket.usedBufSize += read_size;
	
	start = myPs->rcvPacket.rcvStartPoint[i];
	if((start + read_size) > MAX_P3_RECV_PACKET_LENGTH) {
		index = MAX_P3_RECV_PACKET_LENGTH - start;
		memcpy((char *)&myPs->rcvPacket.rcvPacketBuf[start],
			(char *)&maxPacketBuf, index);
		memcpy((char *)&myPs->rcvPacket.rcvPacketBuf,
			(char *)&maxPacketBuf[index], read_size - index);
	} else {
		memcpy((char *)&myPs->rcvPacket.rcvPacketBuf[start],
			(char *)&maxPacketBuf, read_size);
	}
	return 0;
}

void NetworkPacket_Parsing(void)
{
	//char debug[MAX_P3_RECV_PACKET_LENGTH]; //kjg_d
	int i, j, k, cmdBuf_index, start_point;
	
	if(myPs->rcvPacket.rcvCount == myPs->rcvPacket.parseCount) return;
	
	i = myPs->rcvPacket.parseCount;
	myPs->rcvPacket.parseCount++;
	if(myPs->rcvPacket.parseCount > (MAX_P3_RECV_PACKET_COUNT-1))
		myPs->rcvPacket.parseCount = 0;
	
	cmdBuf_index = myPs->rcvCmd.cmdBufSize;
	myPs->rcvCmd.cmdBufSize += myPs->rcvPacket.rcvSize[i];
	
	start_point = myPs->rcvPacket.parseStartPoint[i];

/*	userlog(COC_LOG, psName, "recvCmd1 %s:end %d %d\n", 
		myPs->rcvCmd.cmdBuf, i, start_point);
		
	memset(debug, 0, sizeof debug);
	memcpy((char *)&debug, (char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
		myPs->rcvPacket.rcvSize[i]);
	userlog(COC_LOG, psName, "recvCmd2 %s:end %d %d\n",
		debug, cmdBuf_index, myPs->rcvPacket.rcvSize[i]); //kjg_d*/

	j = start_point + myPs->rcvPacket.rcvSize[i];
	if(j <= MAX_P3_RECV_PACKET_LENGTH) {
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point],
			myPs->rcvPacket.rcvSize[i]);
	} else {
		k = MAX_P3_RECV_PACKET_LENGTH - start_point;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
		cmdBuf_index += k;
		start_point = 0;
		k = j - MAX_P3_RECV_PACKET_LENGTH;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
	}
		
	start_point = myPs->rcvPacket.parseStartPoint[i]
		+ myPs->rcvPacket.rcvSize[i];
	if(start_point >= MAX_P3_RECV_PACKET_LENGTH) {
		j = i + 1;
		if(j >= MAX_P3_RECV_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j]
			= abs(start_point - MAX_P3_RECV_PACKET_LENGTH);
	} else {
		j = i + 1;
		if(j >= MAX_P3_RECV_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j] = start_point;
	}
		
	myPs->rcvPacket.usedBufSize -= myPs->rcvPacket.rcvSize[i];
	
/*	userlog(COC_LOG, psName, "recvCmd3 %d %d\n", myPs->rcvPacket.usedBufSize,
		myPs->rcvCmd.cmdBufSize); //kjg_d*/
}

int NetworkCommand_Receive(void)
{
	//char debug[MAX_P3_RECV_PACKET_LENGTH]; //kjg_d
	char tmpBuf[MAX_P3_RECV_PACKET_LENGTH];
	int cmd_size, cmdBuf_index;
	S_P3_CMD_HEADER header;

	if(myPs->rcvCmd.cmdBufSize < sizeof(S_P3_CMD_HEADER)) return -1;
	
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmdBuf,
		sizeof(S_P3_CMD_HEADER));
	cmd_size = header.body_size + sizeof(S_P3_CMD_HEADER);
	if(myPs->rcvCmd.cmdBufSize < cmd_size) return -2;
			
/*	userlog(COC_LOG, psName, "recvCmd4 %s:end %d\n",
		myPs->rcvCmd.cmdBuf, cmd_size); //kjg_d*/
	
	memset((char *)&myPs->rcvCmd.cmd, 0, MAX_P3_RECV_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmd, (char *)&myPs->rcvCmd.cmdBuf, cmd_size);
	myPs->rcvCmd.cmdSize = cmd_size;
	
	cmdBuf_index = cmd_size;
	myPs->rcvCmd.cmdBufSize -= cmd_size;
	cmd_size = myPs->rcvCmd.cmdBufSize;
	memset((char *)&tmpBuf[0], 0, MAX_P3_RECV_PACKET_LENGTH);
	memcpy((char *)&tmpBuf[0],
		(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
	memset((char *)&myPs->rcvCmd.cmdBuf, 0, MAX_P3_RECV_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmdBuf, (char *)&tmpBuf[0], cmd_size);
	
/*	memset(debug, 0, sizeof debug);
	memcpy((char *)&debug, (char *)&myPs->rcvCmd.cmdBuf, cmd_size);
	userlog(COC_LOG, psName, "recvCmd5 %s:end %d %d\n",
		debug, cmd_size, cmdBuf_index); //kjg_d*/

	return 0;
}

int NetworkCommand_Parsing(void)
{
	unsigned char tmp, log_flag;
	int	rtn, i;
	S_P3_CMD_HEADER header;
	S_P3_RCV_CMD_RESPONSE	cmd;

	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd, sizeof(S_P3_CMD_HEADER));
	
	if(myPs->config.CmdRcvLog == P1) {
		log_flag = 0;
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			switch(header.cmd_id) {
				case P3_CMD_TO_SBC_COMM_CHECK:
				case P3_CMD_TO_SBC_AUX_SET:
				case P3_CMD_TO_SBC_CAN_RECEIVE_SET:
				case P3_CMD_TO_SBC_CAN_TRANSMIT_SET:
				case P3_CMD_TO_SBC_TESTCOND_STEP:
				case P3_CMD_TO_SBC_TESTCOND_PATTERN:
				case P3_CMD_TO_SBC_SET_MEASURE_DATA:
					break;
				case P3_CMD_TO_SBC_RESPONSE:
					memset((char *)&cmd, 0, sizeof(S_P3_RCV_CMD_RESPONSE));
					memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
						sizeof(S_P3_RCV_CMD_RESPONSE));
					if(cmd.response.code != P3_CD_ACK) log_flag = 1;
					break;
				default: log_flag = 1; break;
			}
		}

		if(log_flag == 1) {
			userlog(COC_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		}
	}
	
	if(myPs->config.CmdRcvLog_Hex == P1) {
		log_flag = 0;
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			switch(header.cmd_id) {
				case P3_CMD_TO_SBC_COMM_CHECK:
				case P3_CMD_TO_SBC_AUX_SET:
				case P3_CMD_TO_SBC_CAN_RECEIVE_SET:
				case P3_CMD_TO_SBC_CAN_TRANSMIT_SET:
				case P3_CMD_TO_SBC_TESTCOND_STEP:
				case P3_CMD_TO_SBC_TESTCOND_PATTERN:
				case P3_CMD_TO_SBC_SET_MEASURE_DATA:
					break;
				case P3_CMD_TO_SBC_RESPONSE:
					memset((char *)&cmd, 0, sizeof(S_P3_RCV_CMD_RESPONSE));
					memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
						sizeof(S_P3_RCV_CMD_RESPONSE));
					if(cmd.response.code != P3_CD_ACK) log_flag = 1;
					//log_flag = 1; //kjg_d
					break;
				default: log_flag = 1; break;
			}
		}

		if(log_flag == 1) {
			userlog(COC_LOG, psName, "recvCmd");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				tmp = myPs->rcvCmd.cmd[i];
				userlog2(COC_LOG, psName, " %02x", tmp);
			}
			userlog2(COC_LOG, psName, ":end\n");
		}
	}
	
	rtn = CmdHeader_Check((char *)&header);
	if(rtn < 0) return -1;
	
/*	rtn = Check_ReplyCmd((char *)&header);
	if(rtn < 0) return -2; //kjg_w*/

	switch(header.cmd_id) {
		case P3_CMD_TO_SBC_MODULE_INFO_REQUEST:
			rtn = rcv_cmd_module_info_request();
			break;
		case P3_CMD_TO_SBC_MODULE_SET_DATA:
			rtn = rcv_cmd_module_set_data();
			break;
		case P3_CMD_TO_SBC_AUX_INFO_REQUEST:
			rtn = rcv_cmd_aux_info_request();
			break;
		case P3_CMD_TO_SBC_CAN_RECEIVE_INFO_REQUEST:
			rtn = rcv_cmd_can_receive_info_request();
			break;
		case P3_CMD_TO_SBC_CAN_TRANSMIT_INFO_REQUEST:
			rtn = rcv_cmd_can_transmit_info_request();
			break;
		case P3_CMD_TO_SBC_CH_ATTRIBUTE_REQUEST:
			rtn = rcv_cmd_ch_attribute_request();
			break;
		case P3_CMD_TO_SBC_RUN:
			rtn = rcv_cmd_run();
			break;
		case P3_CMD_TO_SBC_STOP:
			rtn = rcv_cmd_stop();
			break;
		case P3_CMD_TO_SBC_PAUSE:
			rtn = rcv_cmd_pause();
			break;
		case P3_CMD_TO_SBC_CONTINUE:
			rtn = rcv_cmd_continue();
			break;
		case P3_CMD_TO_SBC_NEXT_STEP:
			rtn = rcv_cmd_next_step();
			break;
		case P3_CMD_TO_SBC_CHAMBER_FLAG:
			rtn = rcv_cmd_chamber_flag();
			break;
		case P3_CMD_TO_SBC_CHAMBER_CONTINUE:
			rtn = rcv_cmd_chamber_continue();
			break;
		case P3_CMD_TO_SBC_CABLE_CHECK:
			rtn = rcv_cmd_cable_check();
			break;
		case P3_CMD_TO_SBC_CELL_CHECK:
			rtn = rcv_cmd_cell_check();
			break;
		case P3_CMD_TO_SBC_CYCLE_CONTINUE:
			rtn = rcv_cmd_cycle_continue();
			break;
		case P3_CMD_TO_SBC_TESTCOND_START:
			rtn = rcv_cmd_testcond_start();
			break;
		case P3_CMD_TO_SBC_TESTCOND_SAFETY:
			rtn = rcv_cmd_testcond_safety();
			break;
		case P3_CMD_TO_SBC_TESTCOND_STEP:
			rtn = rcv_cmd_testcond_step();
			break;
		case P3_CMD_TO_SBC_TESTCOND_END:
			rtn = rcv_cmd_testcond_end();
			break;
		case P3_CMD_TO_SBC_TESTCOND_PATTERN:
			rtn = rcv_cmd_testcond_pattern();
			break;
		case P3_CMD_TO_SBC_STEP_COND_REQUEST:
			rtn = rcv_cmd_step_cond_request();
			break;
		case P3_CMD_TO_SBC_STEP_COND_UPDATE:
			rtn = rcv_cmd_step_cond_update();
			break;
		case P3_CMD_TO_SBC_SAFETY_COND_REQUEST:
			rtn = rcv_cmd_safety_cond_request();
			break;
		case P3_CMD_TO_SBC_SAFETY_COND_UPDATE:
			rtn = rcv_cmd_safety_cond_update();
			break;
		case P3_CMD_TO_SBC_RESET_RESERVED_CMD:
			rtn = rcv_cmd_reset_reserved_cmd();
			break;
		case P3_CMD_TO_SBC_CALI_METER_CONNECT:
			rtn = rcv_cmd_cali_meter_connect();
			break;
		case P3_CMD_TO_SBC_CALI_START:
			rtn = rcv_cmd_cali_start();
			break;
		case P3_CMD_TO_SBC_CALI_UPDATE:
			rtn = rcv_cmd_cali_update();
			break;
		case P3_CMD_TO_SBC_RESPONSE:
			rtn = rcv_cmd_response();
			break;
		case P3_CMD_TO_SBC_COMM_CHECK:
			rtn = rcv_cmd_comm_check();
			break;
		case P3_CMD_TO_SBC_SET_MEASURE_DATA:
			rtn = rcv_cmd_set_measure_data();
			break;
		case P3_CMD_TO_SBC_CH_ATTRIBUTE_SET:
			rtn = rcv_cmd_ch_attribute_set();
			break;
		case P3_CMD_TO_SBC_AUX_SET:
			rtn = rcv_cmd_aux_set();
			break;
		case P3_CMD_TO_SBC_CAN_RECEIVE_SET:
			rtn = rcv_cmd_can_receive_set();
			break;
		case P3_CMD_TO_SBC_CAN_TRANSMIT_SET:
			rtn = rcv_cmd_can_transmit_set();
			break;
		case P3_CMD_TO_SBC_CH_INIT:
			rtn = rcv_cmd_init();
			break;
		case P3_CMD_TO_SBC_REAL_TIME_REPLY:
			rtn = rcv_cmd_real_time_reply();
			break;
		case P3_CMD_TO_SBC_BMS_COMM_REQUEST:
			rtn = rcv_cmd_bms_comm_request();
			break;
		case P3_CMD_TO_SBC_DAQ_ISOLATION_REQUEST:
			rtn = rcv_cmd_daq_isolation_request();
			break;
		/*case P3_CMD_TO_SBC_RESET:
			rtn = rcv_cmd_reset();
			break; kjg_w*/
		default:
			userlog(DEBUG_LOG, psName, "Can't Find Cmd[%x]\n", header.cmd_id);
			rtn = -10;
			break;
	}
	return rtn;
}

int Parsing_NetworkEvent(void)
{
	NetworkPacket_Parsing();
	while(NetworkCommand_Receive() >= 0) {
		if(NetworkCommand_Parsing() < 0) {
			myPs->rcvCmd.cmdFail++;
			if(myPs->rcvCmd.cmdFail >= 3) {
				myPs->rcvCmd.cmdFail = 0;
				myPs->rcvCmd.cmdBufSize = 0;
				memset((char *)&myPs->rcvCmd.cmdBuf, 0,
					MAX_P3_RECV_PACKET_LENGTH);
				return -1;
			}
		} else {
			myPs->rcvCmd.cmdFail = 0;
		}
	}
	
	return 0;
}
	
int CmdHeader_Check(char *rcvHeader)
{
	int length;
	S_P3_CMD_HEADER	header;
	S_P3_RCV_CMD_TESTCOND_PATTERN pattern;
	
	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_P3_CMD_HEADER));
	
	switch(header.cmd_id) {
		case P3_CMD_TO_SBC_MODULE_INFO_REQUEST:
			length = sizeof(S_P3_RCV_CMD_MODULE_INFO_REQUEST); 
			break;
		case P3_CMD_TO_SBC_MODULE_SET_DATA:
			length = sizeof(S_P3_RCV_CMD_MODULE_SET_DATA); 
			break;
		case P3_CMD_TO_SBC_AUX_INFO_REQUEST:
			length = sizeof(S_P3_RCV_CMD_AUX_INFO_REQUEST); 
			break;
		case P3_CMD_TO_SBC_CAN_RECEIVE_INFO_REQUEST:
			length = sizeof(S_P3_RCV_CMD_CAN_RECEIVE_INFO_REQUEST); 
			break;
		case P3_CMD_TO_SBC_CAN_TRANSMIT_INFO_REQUEST:
			length = sizeof(S_P3_RCV_CMD_CAN_TRANSMIT_INFO_REQUEST); 
			break;
		case P3_CMD_TO_SBC_CH_ATTRIBUTE_REQUEST:
			length = sizeof(S_P3_RCV_CMD_CH_ATTRIBUTE_REQUEST);
			break;
		case P3_CMD_TO_SBC_RUN:
			length = sizeof(S_P3_RCV_CMD_RUN); 
			break;
		case P3_CMD_TO_SBC_STOP:
			length = sizeof(S_P3_RCV_CMD_STOP); 
			break;
		case P3_CMD_TO_SBC_TESTCOND_START:
			length = sizeof(S_P3_RCV_CMD_TESTCOND_START); 
			break;
		case P3_CMD_TO_SBC_TESTCOND_SAFETY:
			length = sizeof(S_P3_RCV_CMD_TESTCOND_SAFETY);
			break;
		case P3_CMD_TO_SBC_TESTCOND_STEP:
			length = sizeof(S_P3_RCV_CMD_TESTCOND_STEP); 
			break;
		case P3_CMD_TO_SBC_TESTCOND_END:
			length = sizeof(S_P3_RCV_CMD_TESTCOND_END); 
			break;
		case P3_CMD_TO_SBC_TESTCOND_PATTERN:
			length = sizeof(S_P3_RCV_CMD_TESTCOND_PATTERN);
			break;
		case P3_CMD_TO_SBC_STEP_COND_REQUEST:
			length = sizeof(S_P3_RCV_CMD_STEP_COND_REQUEST); 
			break;
		case P3_CMD_TO_SBC_STEP_COND_UPDATE:
			length = sizeof(S_P3_RCV_CMD_STEP_COND_UPDATE); 
			break;
		case P3_CMD_TO_SBC_SAFETY_COND_REQUEST:
			length = sizeof(S_P3_RCV_CMD_SAFETY_COND_REQUEST); 
			break;
		case P3_CMD_TO_SBC_SAFETY_COND_UPDATE:
			length = sizeof(S_P3_RCV_CMD_SAFETY_COND_UPDATE); 
			break;
		case P3_CMD_TO_SBC_RESET_RESERVED_CMD:
			length = sizeof(S_P3_RCV_CMD_RESET_RESERVED_CMD); 
			break;
		case P3_CMD_TO_SBC_CONTINUE:
			length = sizeof(S_P3_RCV_CMD_CONTINUE); 
			break;
		case P3_CMD_TO_SBC_PAUSE:
			length = sizeof(S_P3_RCV_CMD_PAUSE); 
			break;
		case P3_CMD_TO_SBC_NEXT_STEP:
			length = sizeof(S_P3_RCV_CMD_NEXT_STEP); 
			break;
		case P3_CMD_TO_SBC_CHAMBER_FLAG:
			length = sizeof(S_P3_RCV_CMD_CHAMBER_FLAG); 
			break;
		case P3_CMD_TO_SBC_CHAMBER_CONTINUE:
			length = sizeof(S_P3_RCV_CMD_CHAMBER_CONTINUE); 
			break;
		case P3_CMD_TO_SBC_CABLE_CHECK:
			length = sizeof(S_P3_RCV_CMD_CABLE_CHECK); 
			break;
		case P3_CMD_TO_SBC_CELL_CHECK:
			length = sizeof(S_P3_RCV_CMD_CELL_CHECK); 
			break;
		case P3_CMD_TO_SBC_CYCLE_CONTINUE:
			length = sizeof(S_P3_RCV_CMD_CYCLE_CONTINUE); 
			break;
		case P3_CMD_TO_SBC_RESPONSE:
			length = sizeof(S_P3_RCV_CMD_RESPONSE); 
			break;
		case P3_CMD_TO_SBC_CALI_METER_CONNECT:
			length = sizeof(S_P3_RCV_CMD_CALI_METER_CONNECT); 
			break;
		case P3_CMD_TO_SBC_CALI_START:
			length = sizeof(S_P3_RCV_CMD_CALI_START); 
			break;
		case P3_CMD_TO_SBC_CALI_UPDATE:
			length = sizeof(S_P3_RCV_CMD_CALI_UPDATE); 
			break;
		case P3_CMD_TO_SBC_COMM_CHECK:
			length = sizeof(S_P3_RCV_CMD_COMM_CHECK); 
			break;
		case P3_CMD_TO_SBC_SET_MEASURE_DATA:
			length = sizeof(S_P3_RCV_CMD_SET_MEASURE_DATA);
			break;
		case P3_CMD_TO_SBC_CH_ATTRIBUTE_SET:
			length = sizeof(S_P3_RCV_CMD_CH_ATTRIBUTE_SET); 
			break;
		case P3_CMD_TO_SBC_AUX_SET:
			length = sizeof(S_P3_RCV_CMD_AUX_SET);
			break;
		case P3_CMD_TO_SBC_CAN_RECEIVE_SET:
			length = sizeof(S_P3_RCV_CMD_CAN_RECEIVE_SET);
			break;
		case P3_CMD_TO_SBC_CAN_TRANSMIT_SET:
			length = sizeof(S_P3_RCV_CMD_CAN_TRANSMIT_SET);
			break;
		case P3_CMD_TO_SBC_REAL_TIME_REPLY:
			length = sizeof(S_P3_RCV_CMD_REAL_TIME_REPLY);
			break;
		case P3_CMD_TO_SBC_BMS_COMM_REQUEST:
			length = sizeof(S_P3_RCV_CMD_BMS_COMM_REQUEST);
			break;
		case P3_CMD_TO_SBC_DAQ_ISOLATION_REQUEST:
			length = sizeof(S_P3_RCV_CMD_DAQ_ISOLATION_REQUEST);
			break;
		case P3_CMD_TO_SBC_CH_INIT:
			length = sizeof(S_P3_RCV_CMD_INIT);
			break;
		default:
			send_cmd_unknown(header.cmd_id, P3_CMD_ID_ERROR);
			userlog(DEBUG_LOG, psName, "RcvCmd command id error %x\n",
				header.cmd_id);
			return -1;
	}
	
	if(header.cmd_id == P3_CMD_TO_SBC_TESTCOND_PATTERN) {
		memcpy((char *)&pattern, (char *)&myPs->rcvCmd.cmd,
			sizeof(S_P3_RCV_CMD_TESTCOND_PATTERN));
		length += (pattern.testCondPattern.length
			* sizeof(S_P3_TEST_COND_PATTERN_DATA));
		if(length != myPs->rcvCmd.cmdSize) {
			send_cmd_unknown(header.cmd_id, P3_SIZE_ERROR);
			userlog(DEBUG_LOG, psName,
				"RcvCmd(0x%x) size error1 length:%d rcv:%d\n",
				header.cmd_id, length, myPs->rcvCmd.cmdSize);
			return -2;
		}
	} else {
		if(length != myPs->rcvCmd.cmdSize) {
			send_cmd_unknown(header.cmd_id, P3_SIZE_ERROR);
			userlog(DEBUG_LOG, psName,
				"RcvCmd(0x%x) size error2 length:%d rcv:%d\n",
				header.cmd_id, length, myPs->rcvCmd.cmdSize);
			return -2;
		}
	}

	if(header.cmd_serial > MAX_P3_CMD_SERIAL) {
		send_cmd_unknown(header.cmd_id, P3_CD_SEQ_NO_ERROR); //kjg_w
		userlog(DEBUG_LOG, psName, "RcvCmd(0x%x) sequence no error : %d\n",
			header.cmd_id, header.cmd_serial);
		return -5;
	}

	return 0;
}

int Check_ReplyCmd(char *rcvHeader)
{
	int rtn;
	//int seq_no;
	S_P3_CMD_HEADER header;

	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_P3_CMD_HEADER));
	
	rtn = 0;
	/*memset(buf, 0, sizeof buf);
	strncpy(buf, (char *)&header.seqno, sizeof(S_P3_CMD_HEADER));
	seqno = atoi(buf); kjg_w*/
	
	if(myPs->reply.timer_run == P1) {
		/*if(myPs->reply.retry.seqno == seqno
			&& kjg_w*/
		if(myPs->reply.retry.replyCmd == header.cmd_id) {
			myPs->reply.timer_run = P0;
			memset((char *)&myPs->reply.retry, 0, sizeof(S_P3_RETRY_DATA));
		}
	}
	return rtn;
}

int rcv_cmd_module_info_request(void)
{
	userlog(DEBUG_LOG, psName, "rcv_cmd_module_info_request\n");

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_module_info_reply();
}

int rcv_cmd_module_set_data(void)
{
	int rtn, toPs;
	S_P3_RCV_CMD_MODULE_SET_DATA cmd;
	S_MSG_VAL SendMsg;

	userlog(DEBUG_LOG, psName, "rcv_cmd_module_set_data\n");

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_MODULE_SET_DATA));
	
	if(cmd.connection_retry != 0) {
		myPs->signal[P3_SIG_NO_CONNECTION_RETRY] = P1;
	}

	//kjg_w line, control, working mode
	
	if(cmd.auto_report_interval != 0) {
		myPs->config.send_monitor_data_interval
			= (unsigned long)cmd.auto_report_interval * 1000;
	}

	if(cmd.data_save_interval != 0) {
		myPs->config.send_save_data_interval
			= (unsigned long)cmd.data_save_interval * 1000;
	}

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	rtn = Write_COC_Client_Config();
	if(rtn < 0) {
		rtn = send_cmd_response((char *)&cmd.header, P3_CD_FILE_WRITE_ERROR);
		return 0;
	}
	
	rtn = send_cmd_response((char *)&cmd.header, P3_CD_ACK);
	if(rtn >= 0) {
		myPs->signal[P3_SIG_NET_CONNECTED] = P1;
		myPs->misc.net_time = myData->mData.misc.timer_1sec;
		myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
		toPs = COC1_TO_MODULE + myPs->config.groupNo;
		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		SendMsg.msg = MSG_COC_MODULE_SAVE_MSG_FLAG;
		send_msg(toPs, (char *)&SendMsg); //run
	}

	return rtn;
}

int rcv_cmd_testcond_start(void)
{
	S_P3_RCV_CMD_TESTCOND_START	cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_TESTCOND_START));
	
	if(cmd.testCondHeader.totalStep > MAX_P3_STEP) {
		return send_cmd_response((char *)&cmd.header,
			P3_CD_TEST_STEP_COUNT_ERROR); //kjg_w
	}

	memcpy((char *)&myPs->testCond.header,
		(char *)&cmd.testCondHeader, sizeof(S_P3_TEST_COND_HEADER));

	myPs->signal[P3_SIG_TEST_STEP_RCV] = P0;
	myPs->signal[P3_SIG_TEST_STEP_PATTERN_RCV] = P0;
	myPs->signal[P3_SIG_TEST_HEADER_RCV] = P1;
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_testcond_safety(void)
{
	S_P3_RCV_CMD_TESTCOND_SAFETY cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_TESTCOND_SAFETY));

	if(myPs->signal[P3_SIG_TEST_HEADER_RCV] != P1) {
		return send_cmd_response((char *)&cmd.header,
			P3_CD_TEST_HEADER_UNRCV); //kjg_w
	}
	
	memcpy((char *)&myPs->testCond.safety,
		(char *)&cmd.safety, sizeof(S_P3_TEST_COND_SAFETY)); //kjg_w

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_testcond_step(void)
{
	int step;
	S_P3_RCV_CMD_TESTCOND_STEP cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_TESTCOND_STEP));
	
	if(myPs->signal[P3_SIG_TEST_HEADER_RCV] != P1) {
		return send_cmd_response((char *)&cmd.header,
			P3_CD_TEST_HEADER_UNRCV); //kjg_w
	}
	
	step = (int)myPs->signal[P3_SIG_TEST_STEP_RCV];
	if((step+1) > (int)myPs->testCond.header.totalStep || step < 0) {
		userlog(DEBUG_LOG, psName, "test_step_error %d %d\n", step+1,
			(int)myPs->testCond.header.totalStep);
		return send_cmd_response((char *)&cmd.header,
			P3_CD_TEST_STEP_ERROR); //kjg_w
	}

	memcpy((char *)&myPs->testCond.step[step],
		(char *)&cmd.testCondStep, sizeof(S_P3_TEST_COND_STEP));

//	userlog(DEBUG_LOG, psName, "rcv_cmd_testcond_step %d %ld %ld %ld\n",
//		step+1, cmd.testCondStep.record.time,
//		cmd.testCondStep.reference[0].refV,
//		cmd.testCondStep.reference[0].refI); //kjg_d

//	userlog(DEBUG_LOG, psName, "rcv_cmd_testcond_step %d %ld %ld %ld\n",
//		step+1, cmd.testCondStep.reference[0].GotoCondition,
//		cmd.testCondStep.reference[0].cycleCount,
//		cmd.testCondStep.reference[0].gotoCycleCount); //kjg_d

	myPs->signal[P3_SIG_TEST_STEP_RCV]++;

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}
/*
int rcv_cmd_testcond_pattern(void)
{
	char *in_delimiter = " ,\t\n\r", *token, buf[40];
	int step, rtn, i, point, pattern_size, pattern_index, mode;
	S_P3_RCV_CMD_TESTCOND_PATTERN cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_TESTCOND_PATTERN));

	point = sizeof(S_P3_CMD_HEADER) + sizeof(S_P3_TEST_COND_PATTERN);
	pattern_size = (int)cmd.header.body_size
		- sizeof(S_P3_TEST_COND_PATTERN);
	if(pattern_size <= 0) {
		return send_cmd_response((char *)&cmd.header,
			P3_CD_SIZE_MISMATCH); //kjg_w
	}
	
	if(myPs->signal[P3_SIG_TEST_HEADER_RCV] != P1) {
		return send_cmd_response((char *)&cmd.header,
			P3_CD_TEST_HEADER_UNRCV); //kjg_w
	}
	
	step = (int)myPs->signal[P3_SIG_TEST_STEP_RCV];
	if((step+1) > (int)myPs->testCond.header.totalStep || step < 0) {
		userlog(DEBUG_LOG, psName, "test_step_error %d %d\n", step+1,
			(int)myPs->testCond.header.totalStep);
		return send_cmd_response((char *)&cmd.header,
			P3_CD_TEST_STEP_ERROR); //kjg_w
	}
	
	if(myPs->signal[P3_SIG_TEST_STEP_PATTERN_RCV]
		>= MAX_P3_STEP_PATTERN) {
		return send_cmd_response((char *)&cmd.header,
			P3_CD_TEST_STEP_ERROR); //kjg_w
	}

	pattern_index = (int)myPs->signal[P3_SIG_TEST_STEP_PATTERN_RCV];

	memcpy((char *)&myPs->testCond.step[step].header,
		(char *)&cmd.testCondPattern.header, sizeof(S_P3_TEST_STEP_HEADER));
	myPs->testCond.step[step].header.patternIndex
		= (unsigned char)pattern_index;
	memcpy((char *)&myPs->testCond.step[step].reference,
		(char *)&cmd.testCondPattern.reference,
		sizeof(S_P3_TEST_STEP_REFERENCE) * MAX_P3_SUB_STEP);
	memcpy((char *)&myPs->testCond.step[step].record,
		(char *)&cmd.testCondPattern.record, sizeof(S_P3_TEST_RECORD_COND));
	myPs->testCond.step[step].faultUpperV = cmd.testCondPattern.faultUpperV;
	myPs->testCond.step[step].faultLowerV = cmd.testCondPattern.faultLowerV;
	myPs->testCond.step[step].faultUpperI = cmd.testCondPattern.faultUpperI;
	myPs->testCond.step[step].faultLowerI = cmd.testCondPattern.faultLowerI;
	myPs->testCond.step[step].faultUpper_AmpareHour
		= cmd.testCondPattern.faultUpper_AmpareHour;
	myPs->testCond.step[step].faultLower_AmpareHour
		= cmd.testCondPattern.faultLower_AmpareHour;
//kjg_w	myPs->testCond.step[step].faultUpper_Capacitance
//		= cmd.testCondPattern.faultUpper_Capacitance;
//	myPs->testCond.step[step].faultLower_Capacitance
//		= cmd.testCondPattern.faultLower_Capacitance;
	myPs->testCond.step[step].faultUpperZ = cmd.testCondPattern.faultUpperZ;
	myPs->testCond.step[step].faultLowerZ = cmd.testCondPattern.faultLowerZ;
	myPs->testCond.step[step].faultUpperTemp
		= cmd.testCondPattern.faultUpperTemp;
	myPs->testCond.step[step].faultLowerTemp
		= cmd.testCondPattern.faultLowerTemp;

	memcpy((char *)&myPs->testCond.pattern[pattern_index],
		(char *)&cmd.testCondPattern, sizeof(S_P3_TEST_COND_PATTERN));
	myPs->testCond.pattern[pattern_index].faultUpperV
		= cmd.testCondPattern.faultUpperV;
	myPs->testCond.pattern[pattern_index].faultLowerV
		= cmd.testCondPattern.faultLowerV;
	myPs->testCond.pattern[pattern_index].faultUpperI
		= cmd.testCondPattern.faultUpperI;
	myPs->testCond.pattern[pattern_index].faultLowerI
		= cmd.testCondPattern.faultLowerI;
	myPs->testCond.pattern[pattern_index].faultUpper_AmpareHour
		= cmd.testCondPattern.faultUpper_AmpareHour;
	myPs->testCond.pattern[pattern_index].faultLower_AmpareHour
		= cmd.testCondPattern.faultLower_AmpareHour;
//kjg_w	myPs->testCond.pattern[pattern_index].faultUpper_Capacitance
//		= cmd.testCondPattern.faultUpper_Capacitance;
//	myPs->testCond.pattern[pattern_index].faultLower_Capacitance
//		= cmd.testCondPattern.faultLower_Capacitance;
	myPs->testCond.pattern[pattern_index].faultUpperZ
		= cmd.testCondPattern.faultUpperZ;
	myPs->testCond.pattern[pattern_index].faultLowerZ
		= cmd.testCondPattern.faultLowerZ;
	myPs->testCond.pattern[pattern_index].faultUpperTemp
		= cmd.testCondPattern.faultUpperTemp;
	myPs->testCond.pattern[pattern_index].faultLowerTemp
		= cmd.testCondPattern.faultLowerTemp;
	myPs->testCond.pattern[pattern_index].header.patternIndex
		= (unsigned char)pattern_index;
	memset((char *)&myPs->testCond.pattern_data[pattern_index][0], 0,
		sizeof(S_P3_TEST_COND_PATTERN_DATA) * MAX_P3_PATTERN_DATA);

	token = strtok((char *)&myPs->rcvCmd.cmd + point, in_delimiter);
	if(token == NULL) {
		return send_cmd_response((char *)&cmd.header, P3_CD_NACK); //kjg_w
	}

	mode = P3_MODE_IDLE;
	rtn = 0;
	for(i=0; i < MAX_P3_PATTERN_DATA; i++) {
		memset(buf, 0, sizeof buf);
		strcpy(buf, token);
		if(strncmp(buf, "EOF", 3) == 0) {
			rtn = 1;
			break;
		}
		if(i == 0) {
			if((strncmp(buf, "t1", 2) == 0) || (strncmp(buf, "T1", 2) == 0)) {
				//accumulation time
				myPs->testCond.pattern_data[pattern_index][i].t_val = 0;
			} else if((strncmp(buf, "t2", 2) == 0)
				|| (strncmp(buf, "T2", 2) == 0)) {
				//displacement time
				myPs->testCond.pattern_data[pattern_index][i].t_val = 1;
			} else {
				rtn = -1;
				break;
			}
		} else {
			myPs->testCond.pattern_data[pattern_index][i].t_val
				= string_to_long(buf, mode, 0);
		}

		token = strtok(NULL, in_delimiter);
		if(token) {
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
			if(strncmp(buf, "EOF", 3) == 0) {
				rtn = 0;
				break;
			}
		} else {
			rtn = -1;
			break;
		}
		if(i == 0) {
			if((strncmp(buf, "i", 1) == 0) || (strncmp(buf, "I", 1) == 0)) {
				mode = P3_MODE_CC;
				myPs->testCond.pattern_data[pattern_index][i].cmd_val = 0;
			} else if((strncmp(buf, "p", 1) == 0)
				|| (strncmp(buf, "P", 1) == 0)) {
				mode = P3_MODE_CP;
				myPs->testCond.pattern_data[pattern_index][i].cmd_val = 1;
			} else {
				rtn = -1;
				break;
			}
		} else {
			myPs->testCond.pattern_data[pattern_index][i].cmd_val
				= string_to_long(buf, mode, 1);
		}

		token = strtok(NULL, in_delimiter);
		if(token) {
		} else {
			rtn = -1;
			break;
		}
	}

	if(rtn != 1) {
		return send_cmd_response((char *)&cmd.header, P3_CD_NACK); //kjg_w
	}

	myPs->testCond.pattern_data[pattern_index][i].t_val = (-1);
	
	myPs->signal[P3_SIG_TEST_STEP_RCV]++;
	myPs->signal[P3_SIG_TEST_STEP_PATTERN_RCV]++;

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	rtn = send_cmd_response((char *)&cmd.header, P3_CD_ACK);
	return rtn;
}
*/
int rcv_cmd_testcond_pattern(void)
{
	int ch, step, rtn, i, j, pattern_size, pattern_index;//, point;
	unsigned long chFlag, chFlag1;
	S_P3_RCV_CMD_TESTCOND_PATTERN cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_TESTCOND_PATTERN));

//	point = sizeof(S_P3_CMD_HEADER) + sizeof(S_P3_TEST_COND_PATTERN);
	pattern_size = (int)cmd.header.body_size
		- sizeof(S_P3_TEST_COND_PATTERN);
	if(pattern_size <= 0) {
		return send_cmd_response((char *)&cmd.header,
			P3_CD_SIZE_MISMATCH); //kjg_w
	}
	
	if(myPs->signal[P3_SIG_TEST_HEADER_RCV] != P1) {
		return send_cmd_response((char *)&cmd.header,
			P3_CD_TEST_HEADER_UNRCV); //kjg_w
	}
	
//	step = (int)myPs->signal[P3_SIG_TEST_STEP_RCV];
	step = (int)cmd.testCondPattern.stepNo;
	//userlog(DEBUG_LOG, psName, "test_step_pattern %d\n", step+1); //kjg_d
	if((step+1) > (int)myPs->testCond.header.totalStep || step < 0) {
		userlog(DEBUG_LOG, psName, "test_step_error %d %d\n", step+1,
			(int)myPs->testCond.header.totalStep);
		return send_cmd_response((char *)&cmd.header,
			P3_CD_TEST_STEP_ERROR); //kjg_w
	}
	
	//pattern_index = (int)myPs->signal[P3_SIG_TEST_STEP_PATTERN_RCV];
	pattern_index = step;
/*
	memcpy((char *)&myPs->testCond.step[step].header,
		(char *)&cmd.testCondPattern.header, sizeof(S_P3_TEST_STEP_HEADER));
	myPs->testCond.step[step].header.patternIndex
		= (unsigned char)pattern_index;
	memcpy((char *)&myPs->testCond.step[step].reference,
		(char *)&cmd.testCondPattern.reference,
		sizeof(S_P3_TEST_STEP_REFERENCE) * MAX_P3_SUB_STEP);
	memcpy((char *)&myPs->testCond.step[step].record,
		(char *)&cmd.testCondPattern.record, sizeof(S_P3_TEST_RECORD_COND));
	myPs->testCond.step[step].faultUpperV = cmd.testCondPattern.faultUpperV;
	myPs->testCond.step[step].faultLowerV = cmd.testCondPattern.faultLowerV;
	myPs->testCond.step[step].faultUpperI = cmd.testCondPattern.faultUpperI;
	myPs->testCond.step[step].faultLowerI = cmd.testCondPattern.faultLowerI;
	myPs->testCond.step[step].faultUpper_AmpareHour
		= cmd.testCondPattern.faultUpper_AmpareHour;
	myPs->testCond.step[step].faultLower_AmpareHour
		= cmd.testCondPattern.faultLower_AmpareHour;
//kjg_w	myPs->testCond.step[step].faultUpper_Capacitance
//		= cmd.testCondPattern.faultUpper_Capacitance;
//	myPs->testCond.step[step].faultLower_Capacitance
//		= cmd.testCondPattern.faultLower_Capacitance;
	myPs->testCond.step[step].faultUpperZ = cmd.testCondPattern.faultUpperZ;
	myPs->testCond.step[step].faultLowerZ = cmd.testCondPattern.faultLowerZ;
	myPs->testCond.step[step].faultUpperTemp
		= cmd.testCondPattern.faultUpperTemp;
	myPs->testCond.step[step].faultLowerTemp
		= cmd.testCondPattern.faultLowerTemp;
*/
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			rtn = Write_Pattern_File_2(psName, ch, pattern_index,
				(char *)&myPs->rcvCmd.cmd);
			//kjg_w break;
		}
	}
	
//	myPs->signal[P3_SIG_TEST_STEP_RCV]++;
	myPs->signal[P3_SIG_TEST_STEP_PATTERN_RCV]++;

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	rtn = send_cmd_response((char *)&cmd.header, P3_CD_ACK);

	return rtn;
}

int rcv_cmd_testcond_end(void)
{
	int ch, i, j, rtn;
	unsigned long chFlag, chFlag1;
	S_P3_RCV_CMD_TESTCOND_END cmd;
	
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_TESTCOND_END));

	if(myPs->signal[P3_SIG_TEST_HEADER_RCV] != P1) {
		userlog(DEBUG_LOG, psName, "test_header_error %d\n",
			(int)myPs->signal[P3_SIG_TEST_HEADER_RCV]);
		return send_cmd_response((char *)&cmd.header, P3_CD_TEST_HEADER_UNRCV);
	}
	
	if(myPs->signal[P3_SIG_TEST_STEP_RCV] == P0) {
		userlog(DEBUG_LOG, psName, "test_step_rcv_error %d\n",
			(int)myPs->signal[P3_SIG_TEST_STEP_RCV]);
		return send_cmd_response((char *)&cmd.header, P3_CD_TEST_STEP_UNRCV);
	}
	
	if((int)myPs->testCond.header.totalStep
		!= (int)myPs->signal[P3_SIG_TEST_STEP_RCV]) {
		userlog(DEBUG_LOG, psName, "test_step_rcv_error %d\n",
			(int)myPs->signal[P3_SIG_TEST_STEP_RCV]);
		return send_cmd_response((char *)&cmd.header,
			P3_CD_TEST_STEP_COUNT_ERROR); //kjg_w
	}
	
	myPs->signal[P3_SIG_TEST_HEADER_RCV] = P0;
	myPs->signal[P3_SIG_TEST_STEP_RCV] = P0;
	myPs->signal[P3_SIG_TEST_STEP_PATTERN_RCV] = P0;

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			convert_test_cond(ch);
			myData->ChAttribute[ch].chamber_control = 0;
		}
	}
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	rtn = send_cmd_response((char *)&cmd.header, P3_CD_ACK);

	return rtn;
}

int rcv_cmd_set_measure_data(void)
{
	int ch=0, i, installedCh;
	unsigned long chFlag, chFlag1;
	S_P3_RCV_CMD_SET_MEASURE_DATA	cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_RESET_RESERVED_CMD));

	installedCh = myData->mData.config.installedCh;
	if(installedCh > P3_MAX_CH_PER_MODULE) installedCh = P3_MAX_CH_PER_MODULE;
	
	chFlag = 0x00000001;
	for(i=0; i < installedCh; i++) {
		if(i < 32) {
			chFlag1 = (chFlag << i) & cmd.header.chFlag[0];
		} else {
			chFlag1 = (chFlag << (i-32)) & cmd.header.chFlag[1];
		}
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

			if(cmd.id == PS_TEMPERATURE) {
				myData->cData[ch].misc.groupTemp = cmd.data;
			}
		}
	}
	
	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_step_cond_request(void)
{
	return send_cmd_step_cond_reply();
}

int rcv_cmd_step_cond_update(void)
{
	return 0;
}

int rcv_cmd_safety_cond_request(void)
{
	return send_cmd_safety_cond_reply();
}

int rcv_cmd_safety_cond_update(void)
{
	return 0;
}

int rcv_cmd_reset_reserved_cmd(void)
{
	int ch=0, i, installedCh;
	unsigned long chFlag, chFlag1;
	S_P3_RCV_CMD_RESET_RESERVED_CMD	cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_RESET_RESERVED_CMD));

	installedCh = myData->mData.config.installedCh;
	if(installedCh > P3_MAX_CH_PER_MODULE) installedCh = P3_MAX_CH_PER_MODULE;
	
	chFlag = 0x00000001;
	for(i=0; i < installedCh; i++) {
		if(i < 32) {
			chFlag1 = (chFlag << i) & cmd.header.chFlag[0];
		} else {
			chFlag1 = (chFlag << (i-32)) & cmd.header.chFlag[1];
		}
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

			myData->testCond[ch].reserved.reserved_cmd = 0;
			myData->testCond[ch].reserved.reserved_stepNo = 0;
			myData->testCond[ch].reserved.reserved_cycleNo = 0;
			myData->cData[ch].op.reservedCmd = 0;
			myData->testCond[ch].reserved.select_run = 0;
			myData->testCond[ch].reserved.select_stepNo = 0;
			myData->testCond[ch].reserved.select_cycleNo = 0;
			myData->testCond[ch].reserved.select_advCycleStep = 0;
		}
	}
	
	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_cali_meter_connect(void)
{
	int toPs;
	S_P3_RCV_CMD_CALI_METER_CONNECT	cmd;
	S_MSG_VAL SendMsg;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_CALI_METER_CONNECT));

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	toPs = COC1_TO_METER + myPs->config.groupNo;
	SendMsg.msg = MSG_COC_METER_INITIALIZE;
	if(cmd.type == 0) { //v
		SendMsg.val[0] = 0;
		SendMsg.val[1] = 0;
	} else { //i
		SendMsg.val[0] = 0;
		SendMsg.val[1] = 1;
	}
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}

int rcv_cmd_cali_start(void)
{
	int group, ch, i, j, type, range, mode, toPs, debug;
	unsigned long chFlag, chFlag1;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_CALI_START	cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_CALI_START));

	type = cmd.tmpCond.type;
	if(type != CALI_TYPE_VOLTAGE && type != CALI_TYPE_CURRENT) {
		return send_cmd_response((char *)&cmd.header, P3_CD_TYPE_MISMATCH);
	}

	range = cmd.tmpCond.range;
	if(range >= MAX_RANGE || type < 0) {
		return send_cmd_response((char *)&cmd.header, P3_CD_RANGE_MISMATCH);
	}

	mode = cmd.tmpCond.mode;
	if(mode != CALI_MODE_NORMAL && mode != CALI_MODE_CHECK
		&& mode != CALI_MODE_ONLY_CHECK) {
		return send_cmd_response((char *)&cmd.header, P3_CD_MODE_MISMATCH);
	}

	group = myPs->config.groupNo;
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P3_CD_CH_ISNT_STANDBY);
			}
		}
	}

	debug = 0; //kjg_d
	if(debug == 1) {
		userlog(DEBUG_LOG, psName, "cali_start type:%d, range:%d, mode:%d\n",
			cmd.tmpCond.type, cmd.tmpCond.range, cmd.tmpCond.mode);
		userlog(DEBUG_LOG, psName, "setPointNum:%d, checkPointNum:%d\n",
			cmd.tmpCond.point.setPointNum, cmd.tmpCond.point.checkPointNum);
		userlog(DEBUG_LOG, psName, "setPoint ");
		for(i=0; i < MAX_CALI_POINT; i++) {
			userlog2(DEBUG_LOG, psName, "%ld ",
				cmd.tmpCond.point.setPoint[i]);
		}
		userlog2(DEBUG_LOG, psName, "\n");
		userlog(DEBUG_LOG, psName, "checkPoint ");
		for(i=0; i < MAX_CALI_POINT; i++) {
			userlog2(DEBUG_LOG, psName, "%ld ",
				cmd.tmpCond.point.checkPoint[i]);
		}
		userlog2(DEBUG_LOG, psName, "\n");

		j = send_cmd_response((char *)&cmd.header, P3_CD_ACK);

		ch = 0;
		myData->cali[ch].type = type;
		myData->cali[ch].range = range;
		myData->cali[ch].mode = mode;

		memcpy((char *)&myData->cali[ch].tmpCond[type][range],
			(char *)&cmd.tmpCond, sizeof(S_CALI_TMP_COND));

		memset((char *)&myData->cali[ch].tmpData[type][range], 0,
			sizeof(S_CALI_TMP_DATA));

		for(i=0; i < myData->cali[ch].tmpCond[type][range].point.setPointNum;
			i++) {
			myData->cali[ch].tmpData[type][range].set_ad[i] = 1000000 * i;
			myData->cali[ch].tmpData[type][range].set_meter[i] = 1100000 * i;
		}

		for(i=0; i < myData->cali[ch].tmpCond[type][range].point.checkPointNum;
			i++) {
			myData->cali[ch].tmpData[type][range].check_ad[i] = 2000000 * i;
			myData->cali[ch].tmpData[type][range].check_meter[i] = 2100000 * i;
		}

		send_cmd_cali_normal_result(ch);
		return 0;
	} else {
		for(i=0; i < myPs->misc.chInGroup; i++) {
			j = i / 32;
			chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
			if(chFlag1 == 0) continue;

			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

			myData->cali[ch].type = type;
			myData->cali[ch].range = range;
			myData->cali[ch].mode = mode;

			memcpy((char *)&myData->cali[ch].tmpCond[type][range],
				(char *)&cmd.tmpCond, sizeof(S_CALI_TMP_COND));

			memset((char *)&myData->cali[ch].tmpData[type][range], 0,
				sizeof(S_CALI_TMP_DATA));

			group = myPs->config.groupNo;
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			toPs = COC1_TO_METER + group;
			SendMsg.msg = MSG_COC_METER_INITIALIZE;
			SendMsg.val[0] = type;
			SendMsg.val[1] = range;
			send_msg(toPs, (char *)&SendMsg);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			toPs = COC1_TO_MODULE + group;
			SendMsg.msg = MSG_COC_MODULE_CH_CALI;
			SendMsg.val[0] = ch;
			send_msg(toPs, (char *)&SendMsg);
			break;
		}

		return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
	}
}

int rcv_cmd_cali_update(void)
{
	int bd, ch, i, rtn, write_bd[MAX_BD_PER_MODULE], installedCh, group, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_CALI_UPDATE cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_CALI_UPDATE));

	for(bd=0; bd < MAX_BD_PER_MODULE; bd++) {
		write_bd[bd] = 0;
	}

	group = myPs->config.groupNo;
	installedCh = myData->mData.config.installedCh;
	if(installedCh > P3_MAX_CH_PER_MODULE) installedCh = P3_MAX_CH_PER_MODULE;

	chFlag = 0x01;
	for(i=0; i < installedCh; i++) {
		if(i < 32) {
			chFlag1 = (chFlag << i) & cmd.header.chFlag[0];
		} else {
			chFlag1 = (chFlag << (i-32)) & cmd.header.chFlag[1];
		}
		if(chFlag1 != 0) {
			bd = myData->CellArray1[myPs->misc.chOffset + i].bd;
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			CaliUpdateCh(ch);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			toPs = COC1_TO_MODULE + group;
			SendMsg.msg = MSG_COC_MODULE_CH_CALI_UPDATE;
			SendMsg.val[0] = ch;
			send_msg(toPs, (char *)&SendMsg);

			write_bd[bd] = 1;
		}
	}

	for(bd=0; bd < MAX_BD_PER_MODULE; bd++) {
		if(write_bd[bd] == 1) {
			rtn = Write_BdCaliData(bd);
			if(rtn < 0) {
				return send_cmd_response((char *)&cmd.header, P3_CD_NACK);
			}
		}
	}

	userlog(DEBUG_LOG, psName, "cali_update complete\n");
	
	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_response(void)
{
	S_P3_RCV_CMD_RESPONSE cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_RESPONSE));
	
	if(cmd.response.code == P3_CD_ACK) {
		myPs->misc.net_time = myData->mData.misc.timer_1sec;
		myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	} else {
		return -1; //kjg_w
	}
	return 0;
}

int rcv_cmd_run(void)
{
	int group, ch, i, j, toPs, idx, control, range, cable_check_flag;
	long val;
	unsigned long chFlag, chFlag1, advCycleStep;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_RUN cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd, sizeof(S_P3_RCV_CMD_RUN));
	
	switch(myData->AppControl.config.systemModel) {
		case C_SDI_5V_60A_10A_1A:
		case C_SDI_5V_250A_25A_5A:
		case C_SBL_5V_250A_25A_5A:
		case C_SBL_5V_250A_60A_10A:
		case C_SDI_5V_300A_60A_10A:
		case C_SDI_5V_450A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A_2:
			cable_check_flag = 0;
			break;
		default:
			cable_check_flag = 1;
			break;
	}

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P3_CD_CH_STATE_ERROR);
			} else { //cable check
				if(myData->dio.config.dio_Control_Flag == P0) {
				} else {
					if(cable_check_flag == 1) {
						range = myData->cData[ch].op.rangeV;
						val = myData->mData.config.minV[range]
							- (long)((float)myData->mData.config.maxV[range]
							* 0.01);
						if(myData->cData[ch].op.Vsens <= val) {
							return send_cmd_response((char *)&cmd.header,
								P3_CD_CH_CABLE_ERROR);
						} else if(myData->dio.signal[DIO_SIG_CABLE_FAIL1 + ch]
							== P1) {
							return send_cmd_response((char *)&cmd.header,
								P3_CD_CH_CABLE_ERROR);
						}
					}
				}
			}
		}
	}

	if(cmd.control.stepNo == 0 || cmd.control.cycleNo == 0) {
		control = 0; //direct command
	} else {
		control = 1; //reserved command
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));

		if(control == 0) { //direct command
			myData->testCond[ch].reserved.select_run = 0;
			myData->testCond[ch].reserved.select_stepNo = 0;
			myData->testCond[ch].reserved.select_cycleNo = 0;
			myData->testCond[ch].reserved.select_advCycleStep = 0;
		} else { //reserved command
			myData->testCond[ch].reserved.select_run = 1;
			myData->testCond[ch].reserved.select_stepNo
				= (unsigned long)cmd.control.stepNo;
			myData->testCond[ch].reserved.select_cycleNo
				= (unsigned long)cmd.control.cycleNo;
			advCycleStep = 0;
			idx = IDX_LOC_OBJ_TYPE;
			for(j=0; j < MAX_P3_STEP; j++) {
				if(myData->testCond[ch].local_object[j][idx] == STEP_CYCLE) {
					advCycleStep = (unsigned long)j;
				}
				if(j == (int)cmd.control.stepNo-1) {
					break;
				}
			}
			if(j == MAX_P3_STEP) advCycleStep = 0;
			myData->testCond[ch].reserved.select_advCycleStep = advCycleStep;
		}
		myData->testCond[ch].reserved.reserved_cmd = 0;
		myData->testCond[ch].reserved.reserved_stepNo = 0;
		myData->testCond[ch].reserved.reserved_cycleNo = 0;
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = COC1_TO_APP + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_APP_WRITE_CH_ATTRIBUTE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = COC1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_MODULE_CMD_RUN;
	SendMsg.val[0] = group;
	SendMsg.val[1] = control;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName,
		"rcv_cmd_run %x %x %x %x, %x %x %x %x, %d %d\n",
		ch_flag.flag[0], ch_flag.flag[1], ch_flag.flag[2], ch_flag.flag[3],
		ch_flag.flag[4], ch_flag.flag[5], ch_flag.flag[6], ch_flag.flag[7],
		group, control);

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_stop(void)
{
	int group, ch, i, j, toPs, control=0;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_STOP cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd, sizeof(S_P3_RCV_CMD_STOP));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_RUN
				&& myData->cData[ch].op.state != C_PAUSE) {
				return send_cmd_response((char *)&cmd.header,
					P3_CD_CH_STATE_ERROR);
			}
		}
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));

		if(cmd.control.stepNo == 0 || cmd.control.cycleNo == 0) {
			control = 0; //direct command
		} else {
			if(cmd.control.stepNo < (long)(myData->cData[ch].op.idxStepNo+1)) {
				if(cmd.control.cycleNo
					< myData->cData[ch].elementCycle.cycle_count) {
					control = 0; //direct command
				} else if(cmd.control.cycleNo
					== myData->cData[ch].elementCycle.cycle_count) {
					control = 1; //reserved command
				} else {
					control = 1; //reserved command
				}
			} else if(cmd.control.stepNo
				== (long)(myData->cData[ch].op.idxStepNo+1)) {
				if(cmd.control.cycleNo
					< myData->cData[ch].elementCycle.cycle_count) {
					control = 0; //direct command
				} else if(cmd.control.cycleNo
					== myData->cData[ch].elementCycle.cycle_count) {
					control = 1; //reserved command
				} else {
					control = 1; //reserved command
				}
			} else {
				if(cmd.control.cycleNo
					< myData->cData[ch].elementCycle.cycle_count) {
					control = 0; //direct
				} else if(cmd.control.cycleNo
					== myData->cData[ch].elementCycle.cycle_count) {
					//control = 0; //direct kjg_110217
					control = 1; //reserved command
				} else {
					control = 1; //reserved
				}
			}
		}
		if(control == 0) { //direct stop
			myData->testCond[ch].reserved.reserved_cmd = 0;
			myData->testCond[ch].reserved.reserved_stepNo = 0;
			myData->testCond[ch].reserved.reserved_cycleNo = 0;
		} else { //reserved stop
			myData->testCond[ch].reserved.reserved_cmd = 1;
			myData->testCond[ch].reserved.reserved_stepNo
				= (unsigned long)cmd.control.stepNo;
			myData->testCond[ch].reserved.reserved_cycleNo
				= (unsigned long)cmd.control.cycleNo;
		}
		myData->testCond[ch].reserved.select_run = 0;
		myData->testCond[ch].reserved.select_stepNo = 0;
		myData->testCond[ch].reserved.select_cycleNo = 0;
		myData->testCond[ch].reserved.select_advCycleStep = 0;
	}
	
	group = myPs->config.groupNo;
	toPs = COC1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_MODULE_CMD_STOP;
	SendMsg.val[0] = group;
	SendMsg.val[1] = control;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_pause(void)
{
	int group, ch, i, j, toPs, control=0;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_PAUSE cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd, sizeof(S_P3_RCV_CMD_PAUSE));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_RUN) {
				return send_cmd_response((char *)&cmd.header,
					P3_CD_CH_STATE_ERROR);
			}
		}
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));

		if(cmd.control.stepNo == 0 || cmd.control.cycleNo == 0) {
			control = 0; //direct command
		} else {
			if(cmd.control.stepNo < (long)(myData->cData[ch].op.idxStepNo+1)) {
				if(cmd.control.cycleNo
					< myData->cData[ch].elementCycle.cycle_count) {
					control = 0; //direct command
				} else if(cmd.control.cycleNo
					== myData->cData[ch].elementCycle.cycle_count) {
					control = 1; //reserved command
				} else {
					control = 1; //reserved command
				}
			} else if(cmd.control.stepNo
				== (long)(myData->cData[ch].op.idxStepNo+1)) {
				if(cmd.control.cycleNo
					< myData->cData[ch].elementCycle.cycle_count) {
					control = 0; //direct command
				} else if(cmd.control.cycleNo
					== myData->cData[ch].elementCycle.cycle_count) {
					control = 1; //reserved command
				} else {
					control = 1; //reserved command
				}
			} else {
				if(cmd.control.cycleNo
					< myData->cData[ch].elementCycle.cycle_count) {
					control = 0; //direct command
				} else if(cmd.control.cycleNo
					== myData->cData[ch].elementCycle.cycle_count) {
					//control = 0; //direct command kjg_110217
					control = 1; //reserved command
				} else {
					control = 1; //reserved command
				}
			}
		}
		if(control == 0) { //direct command
			myData->testCond[ch].reserved.reserved_cmd = 0;
			myData->testCond[ch].reserved.reserved_stepNo = 0;
			myData->testCond[ch].reserved.reserved_cycleNo = 0;
		} else { //reserved command
			myData->testCond[ch].reserved.reserved_cmd = 2;
			myData->testCond[ch].reserved.reserved_stepNo
				= (unsigned long)cmd.control.stepNo;
			myData->testCond[ch].reserved.reserved_cycleNo
				= (unsigned long)cmd.control.cycleNo;
		}
		myData->testCond[ch].reserved.select_run = 0;
		myData->testCond[ch].reserved.select_stepNo = 0;
		myData->testCond[ch].reserved.select_cycleNo = 0;
		myData->testCond[ch].reserved.select_advCycleStep = 0;
	}
	
	group = myPs->config.groupNo;
	toPs = COC1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_MODULE_CMD_PAUSE;
	SendMsg.val[0] = group;
	SendMsg.val[1] = control;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_continue(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_CONTINUE cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_CONTINUE));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_PAUSE) {
				return send_cmd_response((char *)&cmd.header,
					P3_CD_CH_STATE_ERROR);
			}
		}
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));

		myData->testCond[ch].reserved.reserved_cmd = 0;
		myData->testCond[ch].reserved.reserved_stepNo = 0;
		myData->testCond[ch].reserved.reserved_cycleNo = 0;
		myData->testCond[ch].reserved.select_run = 0;
		myData->testCond[ch].reserved.select_stepNo = 0;
		myData->testCond[ch].reserved.select_cycleNo = 0;
		myData->testCond[ch].reserved.select_advCycleStep = 0;
	}
	
	group = myPs->config.groupNo;
	toPs = COC1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_MODULE_CMD_CONTINUE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	
	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_init(void)
{
	int group, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_INIT cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd, sizeof(S_P3_RCV_CMD_INIT));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			j = i / 8;
			ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));
		}
	}

	group = myPs->config.groupNo;
	toPs = COC1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_MODULE_CMD_INIT;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	
	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_next_step(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_NEXT_STEP cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_NEXT_STEP));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_RUN
				&& myData->cData[ch].op.state != C_PAUSE) {
				return send_cmd_response((char *)&cmd.header,
					P3_CD_CH_STATE_ERROR);
			}
		}
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));

		myData->testCond[ch].reserved.reserved_cmd = 0;
		myData->testCond[ch].reserved.reserved_stepNo = 0;
		myData->testCond[ch].reserved.reserved_cycleNo = 0;
		myData->testCond[ch].reserved.select_run = 0;
		myData->testCond[ch].reserved.select_stepNo = 0;
		myData->testCond[ch].reserved.select_cycleNo = 0;
		myData->testCond[ch].reserved.select_advCycleStep = 0;
	}

	group = myPs->config.groupNo;
	toPs = COC1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_MODULE_CMD_NEXT_STEP;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	//kjg_d printf("next %x %x\n", (int)cmd.header.chFlag[0], (int)cmd.header.chFlag[1]);
	
	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_chamber_flag(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_CHAMBER_FLAG cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_CHAMBER_FLAG));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));

		myData->ChAttribute[ch].chamber_control = cmd.chamber_control;
	}

	group = myPs->config.groupNo;
	toPs = COC1_TO_APP + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_APP_WRITE_CH_ATTRIBUTE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	
	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_chamber_continue(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_CHAMBER_CONTINUE cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_CHAMBER_CONTINUE));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));
	}

	group = myPs->config.groupNo;
	toPs = COC1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_MODULE_CMD_CHAMBER_CONTINUE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	
	userlog(DEBUG_LOG, psName, "recv_chamber_continue %x %x\n",
		ch_flag.flag[0], ch_flag.flag[1]);

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_cable_check(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_CABLE_CHECK cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_CABLE_CHECK));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));
	}

	if(myData->mData.state != M_STANDBY) {
		return send_cmd_response((char *)&cmd.header,
			P3_CD_GP_ISNT_IDLE_STANDBY);
	}

	for(i=0; i < P3_MAX_CH_PER_MODULE; i++) {
		if(myData->cData[i].op.state == C_STANDBY) {
			convert_test_cond_cable_check(i);
		}
	}

	group = myPs->config.groupNo;
	toPs = COC1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_MODULE_CMD_CABLE_CHECK;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	
	userlog(DEBUG_LOG, psName, "recv_cable_check\n");

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_cell_check(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_CELL_CHECK cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_CELL_CHECK));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));
	}

	group = myPs->config.groupNo;
	toPs = COC1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_MODULE_CMD_CELL_CHECK;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	
	userlog(DEBUG_LOG, psName, "recv_cell_check\n");

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_cycle_continue(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_CYCLE_CONTINUE cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_CYCLE_CONTINUE));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_PAUSE) {
				return send_cmd_response((char *)&cmd.header,
					P3_CD_CH_STATE_ERROR);
			}
		}
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));

		myData->testCond[ch].reserved.reserved_cmd = 0;
		myData->testCond[ch].reserved.reserved_stepNo = 0;
		myData->testCond[ch].reserved.reserved_cycleNo = 0;
		myData->testCond[ch].reserved.select_run = 0;
		myData->testCond[ch].reserved.select_stepNo = 0;
		myData->testCond[ch].reserved.select_cycleNo = 0;
		myData->testCond[ch].reserved.select_advCycleStep = 0;
	}
	
	group = myPs->config.groupNo;
	toPs = COC1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_MODULE_CMD_CYCLE_CONTINUE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	
	userlog(DEBUG_LOG, psName, "recv_cycle_continue %x %x\n",
		ch_flag.flag[0], ch_flag.flag[1]);

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_reset(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_RESET cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd, sizeof(S_P3_RCV_CMD_RESET));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P3_CD_CH_STATE_ERROR);
			}

			j = i / 8;
			ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));
		}
	}

	group = myPs->config.groupNo;
	toPs = COC1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_MODULE_CMD_RESET;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	
	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_comm_check_reply(void)
{
	char buf[8];
	S_P3_RCV_CMD_COMM_CHECK_REPLY comm_check_reply;
	
	memcpy((char *)&comm_check_reply, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_COMM_CHECK_REPLY));
	
	memset(buf, 0, sizeof buf);
	memcpy((char *)&buf, (char *)&comm_check_reply.result, 2);
	if(atoi(buf) == P3_CD_ACK) {
		myPs->misc.net_time = myData->mData.misc.timer_1sec;
		myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	} else {
		userlog(DEBUG_LOG, psName, "rcv_cmd_comm_check_reply result:%s\n", buf);
	}
	
	memset(buf, 0, sizeof buf);
	memcpy((char *)&buf, (char *)&comm_check_reply.sent_cmd, 4);
	if(strncmp((char *)&buf, "0x21", 4) == 0) {
	} else {
		userlog(DEBUG_LOG, psName,
			"rcv_cmd_comm_check_reply sendt_cmd:%s\n", buf);
	}
	return 0;
}

int rcv_cmd_comm_check(void)
{
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	send_cmd_comm_check_reply();
	return 0;
}

int rcv_cmd_ch_attribute_set(void)
{
	int group, toPs;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_CH_ATTRIBUTE_SET cmd;

	userlog(DEBUG_LOG, psName, "rcv_cmd_ch_attribute_set\n");

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_CH_ATTRIBUTE_SET));
	
	switch(myData->AppControl.config.systemModel) { 
		//formation, output_power_tester
		case F_SDI_5V_50A_5A:
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
		case F_SDI_5V_450A_200A_100A_10A:

		//triangle_cycler
		case C_SDI_5V_60A_10A_1A:
		case C_SDI_5V_250A_25A_5A:
		case C_SBL_5V_250A_25A_5A:
		case C_SBL_5V_250A_60A_10A:
		case C_SDI_5V_300A_60A_10A:
		case C_SDI_5V_450A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A_2:

		//pack_cycler (1ch only)
		case C_LGC_5V_600A_10A:
		case C_TAESUNG_20V_600A_60A:
		case C_LGC_50V_200A_10A:
		case C_ENERTECH_55V_100A_10A:
		case C_HLGP_60V_200A_10A_12KW:
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
		case C_SDI_70V_250A_25A_18KW:
		case C_SAEHAN_72V_200A_20A:
		case C_LGC_100V_200A_10A_20KW:
		case C_VK_100V_200A_20A:
		case C_EIG_100V_200A_20A_2:
		case C_SEBANG_200V_100A_10A:
		case C_SEBANG_200V_100A_10A_2:
		case C_SEBANG_200V_100A_10A_3:
		case C_SEBANG_200V_100A_10A_4:
		case C_SEBANG_200V_100A_10A_5:
		case C_SEBANG_200V_200A_10A:
		case C_LGC_400V_60A_10A:
		case C_ROTEM_400V_60A_10A:
		case C_KATECH_400V_60A_10A_24KW:
		case C_LGC_400V_100A_10A_40KW:
		case C_AVL_400V_200A_10A_80KW:
		case C_LGC_450V_200A_10A_6:
		case C_ROTEM_450V_200A_10A:
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_HYUNDAI_450V_200A_10A_90KW:
		case C_HYUNDAI_450V_200A_10A_90KW_2:
		case C_VENS_450V_200A_10A_90KW:
		case C_KEPCO_450V_200A_10A_90KW:
		case C_LGC_500V_20A:
		case C_KEPCO_500V_200A_10A_100KW:
		case C_LGC_500V_200A_10A:
			return send_cmd_response((char *)&cmd.header, P3_CD_NACK);
		default:
			break;
	}

	memcpy((char *)&myData->ChAttribute, (char *)&cmd.attr,
		sizeof(S_P3_CH_ATTRIBUTE) * P3_MAX_CH_PER_MODULE);

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = COC1_TO_APP + group;
	SendMsg.msg = MSG_COC_APP_WRITE_CH_ATTRIBUTE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_aux_set(void)
{
	int group, toPs;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_AUX_SET cmd;

	userlog(DEBUG_LOG, psName, "rcv_cmd_aux_set\n");

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_AUX_SET));

	memcpy((char *)&myData->auxSetData, (char *)&cmd.auxSetData,
		sizeof(S_P3_AUX_SET_DATA) * MAX_AUX_DATA);

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = COC1_TO_APP + group;
	SendMsg.msg = MSG_COC_APP_WRITE_AUX_SET_DATA;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	
	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_can_receive_set(void)
{
	int group, toPs, ch, i, j;
	unsigned long chFlag, chFlag1;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_CAN_RECEIVE_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_CAN_RECEIVE_SET));

	toPs = ch = 0;
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			toPs = 1;
			break;
		}
	}

	if(toPs == 0) {
		return send_cmd_response((char *)&cmd.header, P3_CD_NACK);
	}

	userlog(DEBUG_LOG, psName, "rcv_cmd_can_receive_set %d\n", ch+1);

	if(myData->AppControl.config.systemModel == C_SK_450V_200A_10A_360KW) {
		if(ch == 0) {
		} else if(ch == 1 || ch == 2) {
			userlog(DEBUG_LOG, psName, "rcv_cmd_can_receive_set error %d\n",
				ch+1);
			return send_cmd_response((char *)&cmd.header, P3_CD_NACK);
		} else if(ch == 3) {
		} else {
			userlog(DEBUG_LOG, psName, "rcv_cmd_can_receive_set error %d\n",
				ch+1);
			return send_cmd_response((char *)&cmd.header, P3_CD_NACK);
		}
	}

/*kjg_d	userlog(DEBUG_LOG, psName, "can_receive(%d) %ld %ld %ld %d\n",
		ch+1,
		cmd.canReceiveSetData.normalData[0][0].canID,
		cmd.canReceiveSetData.normalData[1][0].canID,
		cmd.canReceiveSetData.normalData[2][0].canID,
		cmd.canReceiveSetData.normalData[3][0].canID
		);*/

	memcpy((char *)&myData->canReceiveSetData, (char *)&cmd.canReceiveSetData,
		sizeof(S_P3_CAN_RECEIVE_SET_DATA));

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = COC1_TO_APP + group;
	SendMsg.msg = MSG_COC_APP_WRITE_CAN_RECEIVE_SET_DATA;
	SendMsg.val[0] = group;
	SendMsg.val[1] = ch;
	send_msg(toPs, (char *)&SendMsg);
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_can_transmit_set(void)
{
	int group, toPs, ch, i, j;
	unsigned long chFlag, chFlag1;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_CAN_TRANSMIT_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_CAN_TRANSMIT_SET));

	toPs = ch = 0;
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			toPs = 1;
			break;
		}
	}

	if(toPs == 0) {
		return send_cmd_response((char *)&cmd.header, P3_CD_NACK);
	}

	userlog(DEBUG_LOG, psName, "rcv_cmd_can_transmit_set %d\n", ch+1);

	if(myData->AppControl.config.systemModel == C_SK_450V_200A_10A_360KW) {
		if(ch == 0) {
		} else if(ch == 1 || ch == 2) {
			userlog(DEBUG_LOG, psName, "rcv_cmd_can_transmit_set error %d\n",
				ch+1);
			return send_cmd_response((char *)&cmd.header, P3_CD_NACK);
		} else if(ch == 3) {
		} else {
			userlog(DEBUG_LOG, psName, "rcv_cmd_can_transmit_set error %d\n",
				ch+1);
			return send_cmd_response((char *)&cmd.header, P3_CD_NACK);
		}
	}

	memcpy((char *)&myData->canTransmitSetData, (char *)&cmd.canTransmitSetData,
		sizeof(S_P3_CAN_TRANSMIT_SET_DATA));

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = COC1_TO_APP + group;
	SendMsg.msg = MSG_COC_APP_WRITE_CAN_TRANSMIT_SET_DATA;
	SendMsg.val[0] = group;
	SendMsg.val[1] = ch;
	send_msg(toPs, (char *)&SendMsg);
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_ch_attribute_request(void)
{
	userlog(DEBUG_LOG, psName, "rcv_cmd_ch_attribute_request\n");

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_ch_attribute_reply();
}

int rcv_cmd_aux_info_request(void)
{
	userlog(DEBUG_LOG, psName, "rcv_cmd_aux_info_request\n");

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_aux_info_reply();
}

int rcv_cmd_can_receive_info_request(void)
{
	userlog(DEBUG_LOG, psName, "rcv_cmd_can_receive_info_request\n");

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_can_receive_info_reply();
}

int rcv_cmd_can_transmit_info_request(void)
{
	userlog(DEBUG_LOG, psName, "rcv_cmd_can_transmit_info_request\n");

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_can_transmit_info_reply();
}

int rcv_cmd_real_time_reply(void)
{
	S_P3_RCV_CMD_REAL_TIME_REPLY cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_REAL_TIME_REPLY));

	Update_RealTime((char *)&cmd.real_time);
	sleep(1);

	myPs->misc.sent_real_time_request = myData->mData.real_time[4]; //day

	return send_cmd_response((char *)&cmd.header, P3_CD_ACK);
}

int rcv_cmd_bms_comm_request(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_BMS_COMM_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_BMS_COMM_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P3_CD_CH_STATE_ERROR);
			}
		}
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));
	}

	//userlog(DEBUG_LOG, psName, "recv_func_div %d\n", cmd.function_div); //kjg_d

	group = myPs->config.groupNo;
	toPs = COC1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_MODULE_BMS_COMM_REQUEST;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.function_div;
	send_msg(toPs, (char *)&SendMsg);

	send_cmd_bms_comm_reply(ch*2, (int)cmd.function_div);

	return 0;
}

int rcv_cmd_daq_isolation_request(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P3_RCV_CMD_DAQ_ISOLATION_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P3_RCV_CMD_DAQ_ISOLATION_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P3_CD_CH_STATE_ERROR);
			}
		}
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));
	}

	userlog(DEBUG_LOG, psName, "recv_daq_iso %d\n", cmd.iso); //kjg_d

	group = myPs->config.groupNo;
	toPs = COC1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COC_MODULE_DAQ_ISOLATION_REQUEST;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.iso; //0:don't iso, 1:iso
	send_msg(toPs, (char *)&SendMsg);

	send_cmd_daq_isolation_reply((int)cmd.iso);

	return 0;
}

int send_cmd_response(char *rcvHeader, int code)
{
	int cmd_size, body_size, rtn;
	S_P3_SEND_CMD_RESPONSE cmd;
	S_P3_CMD_HEADER header;

	cmd_size = sizeof(S_P3_SEND_CMD_RESPONSE);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_RESPONSE, SEQNUM_AUTO, body_size);
	
	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_P3_CMD_HEADER));

	cmd.header.reserved1 = header.reserved1;
	cmd.header.reserved2 = header.reserved2;
	cmd.header.chFlag[0] = header.chFlag[0];
	cmd.header.chFlag[1] = header.chFlag[1];
	cmd.response.cmd = (int)header.cmd_id;
	cmd.response.code = code;
	//cmd.response.cmdSerial = header.cmd_serial; //kjg_w
	
	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_RESPONSE);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_RESPONSE);
	}
	return 0;
}

int send_cmd_module_info_reply(void)
{
	int cmd_size, body_size, rtn, i;
	S_P3_SEND_CMD_MODULE_INFO_REPLY cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_MODULE_INFO_REPLY);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_MODULE_INFO_REPLY, SEQNUM_AUTO, body_size);
	
	//cmd.md_info.group_id = (unsigned int)myData->AppControl.config.moduleNo;
	cmd.md_info.group_id = (unsigned int)myPs->config.groupId;
	cmd.md_info.systemType = myData->AppControl.config.systemType;
	cmd.md_info.protocol_version = myPs->config.protocol_version;
	memcpy((char *)&cmd.md_info.modelName,
		(char *)&myData->AppControl.config.modelName, 128);
	cmd.md_info.osVersion = myData->AppControl.config.osVersion;
	cmd.md_info.voltage_range
		= (unsigned short int)myData->mData.config.rangeV;
	cmd.md_info.current_range
		= (unsigned short int)myData->mData.config.rangeI;
	for(i=0; i < MAX_RANGE; i++) {
		cmd.md_info.voltage_spec[i] = myData->mData.config.maxV[i];
		switch(myData->AppControl.config.systemModel) {
			case C_ENERTECH_55V_100A_10A:
			case C_SAEHAN_72V_200A_20A:
				cmd.md_info.current_spec[i]
					= labs(myData->mData.config.minI[i]);
				break;
			default:
				cmd.md_info.current_spec[i] = myData->mData.config.maxI[i];
				break;
		}
	}
	cmd.md_info.voltage_spec[4] = 0;
	cmd.md_info.current_spec[4] = 0;

	cmd.md_info.installedBd
		= (unsigned short int)myData->mData.config.installedBd ;
	i = myData->mData.config.chPerBd;
	if(i > P3_MAX_CH_PER_BD) i = P3_MAX_CH_PER_BD;
	cmd.md_info.chPerBd = (unsigned short int)i;

	i = myData->mData.config.installedCh;
	if(i > P3_MAX_CH_PER_MODULE) i = P3_MAX_CH_PER_MODULE;
	cmd.md_info.installedCh = (unsigned int)i;

	//kjg_w_s
	cmd.md_info.totalJig = 0; //myData->mData.config.totalJig;
	for(i=0; i < 16; i++) {
		cmd.md_info.BdinJig[i] = 0; //myData->mData.config.bdInJig[i];
	}
	//kjg_w_e
	
	userlog(DEBUG_LOG, psName, "send_cmd_module_info_reply\n");

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_MODULE_INFO_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_MODULE_INFO_REPLY);
	}
	return 0;
}

int send_cmd_set_measure_data(int id)
{
	int cmd_size, body_size, rtn;
	S_P3_SEND_CMD_SET_MEASURE_DATA cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_SET_MEASURE_DATA);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_SET_MEASURE_DATA, SEQNUM_AUTO, body_size);
	
	cmd.id = (unsigned short)id;
	switch(cmd.id){
		case PS_TEMPERATURE:
			cmd.type = 0;
			cmd.data = 0;
			break;
	}

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_SET_MEASURE_DATA);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n", rtn,
			P3_CMD_TO_PC_SET_MEASURE_DATA);
	}
	return 0;
}

int send_cmd_aux_info_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P3_SEND_CMD_AUX_INFO_REPLY cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_AUX_INFO_REPLY);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_AUX_INFO_REPLY, SEQNUM_AUTO, body_size);
	
	cmd.installedTemp = (short int)myData->mData.config.installedTemp;
	cmd.installedAuxV = (short int)myData->mData.config.installedAuxV;

	memcpy((char *)&cmd.auxSetData[0], (char *)&myData->auxSetData[0],
		sizeof(S_P3_AUX_SET_DATA) * MAX_AUX_DATA);

	userlog(DEBUG_LOG, psName, "send_cmd_aux_info_reply\n");

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_AUX_INFO_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_AUX_INFO_REPLY);
	}
	return 0;
}

int send_cmd_can_receive_info_reply(void)
{
	int cmd_size, body_size, rtn, toPs;
	S_P3_SEND_CMD_CAN_RECEIVE_INFO_REPLY cmd;
	S_MSG_VAL SendMsg;
	
	cmd_size = sizeof(S_P3_SEND_CMD_CAN_RECEIVE_INFO_REPLY);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_CAN_RECEIVE_INFO_REPLY, SEQNUM_AUTO, body_size);
	
	for(rtn=0; rtn < P3_MAX_CH_PER_MODULE; rtn++) {
		cmd.canReceiveDataCount[rtn]
			= (short int)(myData->canReceiveDataCount[rtn][0]
			+ myData->canReceiveDataCount[rtn][1]);
	}

	memcpy((char *)&cmd.canReceiveSetData, (char *)&myData->canReceiveSetData,
		sizeof(S_P3_CAN_RECEIVE_SET_DATA));

	userlog(DEBUG_LOG, psName, "send_cmd_can_receive_info_reply\n");

	rtn = send_command((char *)&cmd, cmd_size,
		P3_CMD_TO_PC_CAN_RECEIVE_INFO_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x size:%d\n", rtn,
			P3_CMD_TO_PC_CAN_RECEIVE_INFO_REPLY,
			sizeof(S_P3_SEND_CMD_CAN_RECEIVE_INFO_REPLY));
	} else {
		if(myPs->config.protocol_version < P3_PROTOCOL_VERSION_5) {
			myPs->signal[P3_SIG_NET_CONNECTED] = P1;
			myPs->misc.net_time = myData->mData.misc.timer_1sec;
			myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
			toPs = COC1_TO_MODULE + myPs->config.groupNo;
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_COC_MODULE_SAVE_MSG_FLAG;
			send_msg(toPs, (char *)&SendMsg); //run
		}
	}

	return 0;
}

int send_cmd_can_transmit_info_reply(void)
{
	int cmd_size, body_size, rtn, toPs;
	S_P3_SEND_CMD_CAN_TRANSMIT_INFO_REPLY cmd;
	S_MSG_VAL SendMsg;
	
	cmd_size = sizeof(S_P3_SEND_CMD_CAN_TRANSMIT_INFO_REPLY);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_CAN_TRANSMIT_INFO_REPLY, SEQNUM_AUTO, body_size);
	
	for(rtn=0; rtn < P3_MAX_CH_PER_MODULE; rtn++) {
		cmd.canTransmitDataCount[rtn]
			= (short int)(myData->canTransmitDataCount[rtn][0]
			+ myData->canTransmitDataCount[rtn][1]);
	}

	memcpy((char *)&cmd.canTransmitSetData, (char *)&myData->canTransmitSetData,
		sizeof(S_P3_CAN_TRANSMIT_SET_DATA));

	userlog(DEBUG_LOG, psName, "send_cmd_can_transmit_info_reply\n");

	rtn = send_command((char *)&cmd, cmd_size,
		P3_CMD_TO_PC_CAN_TRANSMIT_INFO_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x size:%d\n", rtn,
			P3_CMD_TO_PC_CAN_TRANSMIT_INFO_REPLY,
			sizeof(S_P3_SEND_CMD_CAN_TRANSMIT_INFO_REPLY));
	} else {
		myPs->signal[P3_SIG_NET_CONNECTED] = P1;
		myPs->misc.net_time = myData->mData.misc.timer_1sec;
		myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
		toPs = COC1_TO_MODULE + myPs->config.groupNo;
		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		SendMsg.msg = MSG_COC_MODULE_SAVE_MSG_FLAG;
		send_msg(toPs, (char *)&SendMsg); //run
	}

	return 0;
}

int send_cmd_ch_attribute_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P3_SEND_CMD_CH_ATTRIBUTE_REPLY cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_CH_ATTRIBUTE_REPLY);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_CH_ATTRIBUTE_REPLY, SEQNUM_AUTO, body_size);
	
	memcpy((char *)&cmd.attr, (char *)&myData->ChAttribute,
		sizeof(S_P3_CH_ATTRIBUTE) * P3_MAX_CH_PER_MODULE);

	userlog(DEBUG_LOG, psName, "send_cmd_ch_attribute_reply\n");

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_CH_ATTRIBUTE_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_CH_ATTRIBUTE_REPLY);
	}
	return 0;
}

void send_cmd_monitor_data(void)
{
	int count, rtn, msg, chInGroup, max_send_ch; //, i;
	long diff, time1, time2;

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_50A_5A:
			chInGroup = myData->mData.config.installedCh;
			break;
		default:
			chInGroup = myPs->misc.chInGroup;
			if(chInGroup > P3_MAX_CH_IN_GROUP) chInGroup = P3_MAX_CH_IN_GROUP;
			break;
	}

	if(chInGroup <= 8) {
		max_send_ch = chInGroup;
	} else {
		max_send_ch = 8;
	}

	msg = myPs->config.groupNo;

	time1 = myData->mData.misc.timer_1sec;
	time2 = myData->mData.misc.timer_1000ms;
	diff = (time1 - myPs->misc.sent_monitor_data_time) * 1000;
	diff += (time2 - myPs->misc.sent_monitor_data_time2);
	if(diff >= myPs->config.send_monitor_data_interval || diff < 0) {
		//userlog(DEBUG_LOG, psName, "send_cmd_ch_data\n"); //kjg_d

		rtn = send_cmd_ch_data();
		if(rtn > 0) count = 50;
		else count = 0;
		while(count) {
			rtn = send_cmd_ch_data();
			if(rtn == 0) count = 0;
			else count--;
		}
/*
		for(i=0; i < max_send_ch; i++) {
			rtn = send_cmd_ch_pulse_data(i);
			if(rtn > 0) count = 15;
			else count = 0;
			while(count) {
				rtn = send_cmd_ch_pulse_data(i);
				if(rtn == 0) count = 0;
				else count--;
			}
		}
*/
		//kjg_w rtn = send_cmd_fadm_pulse_data(i);
		//kjg_w rtn = send_cmd_sensor_data();

		rtn = send_cmd_record_data();

		myPs->misc.sent_monitor_data_time = time1;
		myPs->misc.sent_monitor_data_time2 = time2;
	}
}

int send_cmd_ch_data(void)
{ //kjg_110921
	int cmd_size, body_size, rtn, ch, idx, count, msg, i, j, k, i2, cnt1;
	long val, val1, val2;
	S_P3_SEND_CMD_CH_DATA	cmd;

	cmd_size = sizeof(S_P3_SEND_CMD_CH_DATA);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_CH_DATA, SEQNUM_AUTO, body_size);

	count = 0;
	cnt1 = 0;
	msg = 0;
	
	for(i=0; i < myData->mData.config.installedCh; i++) {
		ch = myData->CellArray1[i].number2 - 1;

		if(myData->save_msg_1[msg].write_idx[ch]
			== myData->save_msg_1[msg].read_idx[ch]) {
			myData->save_msg_1[msg].count[ch] = 0;
			cnt1++;

			cmd.chData[i].ch = i + 1;
			cmd.chData[i].select = SAVE_FLAG_MONITORING_DATA;

			cmd.chData[i].state
				= (unsigned char)convert_ch_state(CONVERT_ORG_TO_P3,
				(long)myData->cData[ch].op.state);
			cmd.chData[i].stepType
				= (unsigned char)convert_step_type(CONVERT_ORG_TO_P3,
				(long)myData->cData[ch].op.stepType);
			cmd.chData[i].stepMode
				= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P3,
				(long)myData->cData[ch].op.stepMode);

			cmd.chData[i].code
				= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P3,
				(long)myData->cData[ch].op.code);
			cmd.chData[i].stepNo = myData->cData[ch].op.idxStepNo + 1;
			cmd.chData[i].grade = myData->cData[ch].op.grade;

			cmd.chData[i].Vsens = myData->cData[ch].op.Vsens;

			if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
				val = myData->cData[ch].op.Isens;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.Isens;
					}
				}
				cmd.chData[i].Isens = val;

				val = myData->cData[ch].op.charge_AmpareHour;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.charge_AmpareHour;
					}
				}
				//cmd.chData[i].charge_AmpareHour = val;
				val1 = val;

				val = myData->cData[ch].op.discharge_AmpareHour;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.discharge_AmpareHour;
					}
				}
				//cmd.chData[i].discharge_AmpareHour = val;
				val2 = val;
				if(myData->cData[ch].op.stepType == STEP_PATTERN) {
					cmd.chData[i].capacity = val1 + val2;
				} else {
					cmd.chData[i].capacity = val1 + (val2 * (-1));
				}

				val = myData->cData[ch].op.watt;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.watt;
					}
				}
				cmd.chData[i].watt = val;

				val = myData->cData[ch].op.charge_WattHour;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.charge_WattHour;
					}
				}
				//cmd.chData[i].charge_WattHour = val;
				val1 = val;

				val = myData->cData[ch].op.discharge_WattHour;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.discharge_WattHour;
					}
				}
				//cmd.chData[i].discharge_WattHour = val;
				val2 = val;
				if(myData->cData[ch].op.stepType == STEP_PATTERN) {
					cmd.chData[i].wattHour = val1 + val2;
				} else {
					cmd.chData[i].wattHour = val1 + (val2 * (-1));
				}

				val = myData->cData[ch].op.meanI;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.meanI;
					}
				}
				cmd.chData[i].avgI = val;
			} else {
				cmd.chData[i].Isens = myData->cData[ch].op.Isens;
				//cmd.chData[i].charge_AmpareHour
				val1 = myData->cData[ch].op.charge_AmpareHour;
				//cmd.chData[i].discharge_AmpareHour
				val2 = myData->cData[ch].op.discharge_AmpareHour;
				if(myData->cData[ch].op.stepType == STEP_PATTERN) {
					cmd.chData[i].capacity = val1 + val2;
				} else {
					cmd.chData[i].capacity = val1 + (val2 * (-1));
				}
				cmd.chData[i].watt = myData->cData[ch].op.watt;
				//cmd.chData[i].charge_WattHour
				val1 = myData->cData[ch].op.charge_WattHour;
				//cmd.chData[i].discharge_WattHour
				val2 = myData->cData[ch].op.discharge_WattHour;
				if(myData->cData[ch].op.stepType == STEP_PATTERN) {
					cmd.chData[i].wattHour = val1 + val2;
				} else {
					cmd.chData[i].wattHour = val1 + (val2 * (-1));
				}
				cmd.chData[i].avgI = myData->cData[ch].op.meanI;
			}

			//cmd.chData[i].capacitance = myData->cData[ch].op.capacitance;
			cmd.chData[i].z = myData->cData[ch].op.z;

			j = 0;
			for(i2=0; i2 < MAX_AUX_DATA; i2++) {
				if((ch+1) == myData->auxSetData[i2].chNo) {
					k = myData->auxSetData[i2].auxChNo - 1;
					cmd.chData[i].temp[j] = myData->COM.com_port[1].value[k];
					j++;
				}
				if(j == 3) break;
			}

			cmd.chData[i].runTime = myData->cData[ch].op.runTime;
			cmd.chData[i].totalRunTime = myData->cData[ch].op.totalRunTime;

			cmd.chData[i].reservedCmd = myData->cData[ch].op.reservedCmd;

			cmd.chData[i].totalCycle = myData->cData[ch].misc.totalCycle;
			cmd.chData[i].elementCycle
				= myData->cData[ch].elementCycle.cycle_count;
			cmd.chData[i].gotoCycleCount
				= myData->cData[ch].accCycle[0].cycle_count;

			cmd.chData[i].avgV = myData->cData[ch].op.meanV;

			if(myData->ChAttribute[ch].chamber_control == 0) {
				cmd.chData[i].chamber_control = 0;
			} else {
				cmd.chData[i].chamber_control = 1;
			}
/*
			cmd.chData[i].cvTime = myData->cData[ch].op.cvTime;

			for(j=0; j < MAX_ACC_CYCLE; j++) {
				cmd.chData[i].accCycle[j]
					= myData->cData[ch].accCycle[j].cycle_count;
			}
			for(j=0; j < MAX_MULTI_CYCLE; j++) {
				cmd.chData[i].multiCycle[j]
					= myData->cData[ch].multiCycle[j].cycle_count;
			}

			cmd.chData[i].realDate
				= myData->mData.real_time[6] * 10000 //year
				+ myData->mData.real_time[5] * 100 //month
				+ myData->mData.real_time[4]; //day
			cmd.chData[i].realClock
				= myData->mData.real_time[3] * 10000000 //hour
				+ myData->mData.real_time[2] * 100000 //min
				+ myData->mData.real_time[1] * 1000 //sec
				+ myData->mData.real_time[0]; //msec
*/
		} else {
			if(myData->save_msg_1[msg].total_count > 0) {
				myData->save_msg_1[msg].total_count--;
			}

			myData->save_msg_1[msg].read_idx[ch]++;
			if(myData->save_msg_1[msg].read_idx[ch] >= MAX_SAVE_MSG)
				myData->save_msg_1[msg].read_idx[ch] = 0;
			idx = myData->save_msg_1[msg].read_idx[ch];

			if(myData->save_msg_1[msg].count[ch] > 0) {
				myData->save_msg_1[msg].count[ch]--;
			}
			if(myData->save_msg_1[msg].count[ch] > count) {
				count = myData->save_msg_1[msg].count[ch];
			}
			memcpy((char *)&cmd.chData[i],
				(char *)&myData->save_msg_1[msg].val[idx][ch],
				sizeof(S_SAVE_MSG_1_VAL));
			cmd.chData[i].ch = i + 1;

			//userlog(DEBUG_LOG, psName, "send_cmd_ch_data save\n"); //kjg_d
		}
	}

	if(myData->mData.config.installedCh == cnt1) {
		myData->save_msg_1[msg].total_count = 0;
	}

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_CH_DATA);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_CH_DATA);
	}

	return count;
}

int send_cmd_record_data(void)
{ //kjg_110921
	int cmd_size, body_size, rtn, ch, idx, count, msg, i, j, k, i2, send_flag;
	int cnt1;
	long val, val1, val2;
	S_P3_SEND_CMD_CH_RECORD_DATA	cmd;

	cmd_size = sizeof(S_P3_SEND_CMD_CH_RECORD_DATA);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_RECORD_DATA, SEQNUM_AUTO, body_size);

	send_flag = 0;
	count = 0;
	cnt1 = 0;
	msg = 0;
	
	for(i=0; i < myData->mData.config.installedCh; i++) {
		ch = myData->CellArray1[i].number2 - 1;

		if(myData->record_msg_1[msg].write_idx[ch]
			== myData->record_msg_1[msg].read_idx[ch]) {
			myData->record_msg_1[msg].count[ch] = 0;
			cnt1++;

			cmd.chData[i].ch = i + 1;
			cmd.chData[i].select = SAVE_FLAG_MONITORING_DATA;

			cmd.chData[i].state
				= (unsigned char)convert_ch_state(CONVERT_ORG_TO_P3,
				(long)myData->cData[ch].op.state);
			cmd.chData[i].stepType
				= (unsigned char)convert_step_type(CONVERT_ORG_TO_P3,
				(long)myData->cData[ch].op.stepType);
			cmd.chData[i].stepMode
				= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P3,
				(long)myData->cData[ch].op.stepMode);

			cmd.chData[i].code
				= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P3,
				(long)myData->cData[ch].op.code);
			cmd.chData[i].stepNo = myData->cData[ch].op.idxStepNo + 1;
			cmd.chData[i].grade = myData->cData[ch].op.grade;

			cmd.chData[i].Vsens = myData->cData[ch].op.Vsens;

			if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
				val = myData->cData[ch].op.Isens;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.Isens;
					}
				}
				cmd.chData[i].Isens = val;

				val = myData->cData[ch].op.charge_AmpareHour;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.charge_AmpareHour;
					}
				}
				//cmd.chData[i].charge_AmpareHour = val;
				val1 = val;

				val = myData->cData[ch].op.discharge_AmpareHour;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.discharge_AmpareHour;
					}
				}
				//cmd.chData[i].discharge_AmpareHour = val;
				val2 = val;
				if(myData->cData[ch].op.stepType == STEP_PATTERN) {
					cmd.chData[i].capacity = val1 + val2;
				} else {
					cmd.chData[i].capacity = val1 + (val2 * (-1));
				}

				val = myData->cData[ch].op.watt;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.watt;
					}
				}
				cmd.chData[i].watt = val;

				val = myData->cData[ch].op.charge_WattHour;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.charge_WattHour;
					}
				}
				//cmd.chData[i].charge_WattHour = val;
				val1 = val;

				val = myData->cData[ch].op.discharge_WattHour;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.discharge_WattHour;
					}
				}
				//cmd.chData[i].discharge_WattHour = val;
				val2 = val;
				if(myData->cData[ch].op.stepType == STEP_PATTERN) {
					cmd.chData[i].wattHour = val1 + val2;
				} else {
					cmd.chData[i].wattHour = val1 + (val2 * (-1));
				}

				val = myData->cData[ch].op.meanI;
				for(j=0; j < 3; j++) {
					k = (int)myData->ChAttribute[ch].chNo_slave[j] - 1;
					if(k >= 0) {
						val += myData->cData[k].op.meanI;
					}
				}
				cmd.chData[i].avgI = val;
			} else {
				cmd.chData[i].Isens = myData->cData[ch].op.Isens;
				//cmd.chData[i].charge_AmpareHour
				val1 = myData->cData[ch].op.charge_AmpareHour;
				//cmd.chData[i].discharge_AmpareHour
				val2 = myData->cData[ch].op.discharge_AmpareHour;
				if(myData->cData[ch].op.stepType == STEP_PATTERN) {
					cmd.chData[i].capacity = val1 + val2;
				} else {
					cmd.chData[i].capacity = val1 + (val2 * (-1));
				}
				cmd.chData[i].watt = myData->cData[ch].op.watt;
				//cmd.chData[i].charge_WattHour
				val1 = myData->cData[ch].op.charge_WattHour;
				//cmd.chData[i].discharge_WattHour
				val2 = myData->cData[ch].op.discharge_WattHour;
				if(myData->cData[ch].op.stepType == STEP_PATTERN) {
					cmd.chData[i].wattHour = val1 + val2;
				} else {
					cmd.chData[i].wattHour = val1 + (val2 * (-1));
				}
				cmd.chData[i].avgI = myData->cData[ch].op.meanI;
			}

			//cmd.chData[i].capacitance = myData->cData[ch].op.capacitance;
			cmd.chData[i].z = myData->cData[ch].op.z;

			j = 0;
			for(i2=0; i2 < MAX_AUX_DATA; i2++) {
				if((ch+1) == myData->auxSetData[i2].chNo) {
					k = myData->auxSetData[i2].auxChNo - 1;
					cmd.chData[i].temp[j] = myData->COM.com_port[1].value[k];
					j++;
				}
				if(j == 3) break;
			}

			cmd.chData[i].runTime = myData->cData[ch].op.runTime;
			cmd.chData[i].totalRunTime = myData->cData[ch].op.totalRunTime;

			cmd.chData[i].reservedCmd = myData->cData[ch].op.reservedCmd;

			cmd.chData[i].totalCycle = myData->cData[ch].misc.totalCycle;
			cmd.chData[i].elementCycle
				= myData->cData[ch].elementCycle.cycle_count;
			cmd.chData[i].gotoCycleCount
				= myData->cData[ch].accCycle[0].cycle_count;

			cmd.chData[i].avgV = myData->cData[ch].op.meanV;
/*
			cmd.chData[i].cvTime = myData->cData[ch].op.cvTime;

			for(j=0; j < MAX_ACC_CYCLE; j++) {
				cmd.chData[i].accCycle[j]
					= myData->cData[ch].accCycle[j].cycle_count;
			}
			for(j=0; j < MAX_MULTI_CYCLE; j++) {
				cmd.chData[i].multiCycle[j]
					= myData->cData[ch].multiCycle[j].cycle_count;
			}

			cmd.chData[i].realDate
				= myData->mData.real_time[6] * 10000 //year
				+ myData->mData.real_time[5] * 100 //month
				+ myData->mData.real_time[4]; //day
			cmd.chData[i].realClock
				= myData->mData.real_time[3] * 10000000 //hour
				+ myData->mData.real_time[2] * 100000 //min
				+ myData->mData.real_time[1] * 1000 //sec
				+ myData->mData.real_time[0]; //msec
*/
		} else {
			send_flag = 1;

			if(myData->record_msg_1[msg].total_count > 0) {
				myData->record_msg_1[msg].total_count--;
			}

			myData->record_msg_1[msg].read_idx[ch]++;
			if(myData->record_msg_1[msg].read_idx[ch] >= MAX_RECORD_MSG)
				myData->record_msg_1[msg].read_idx[ch] = 0;
			idx = myData->record_msg_1[msg].read_idx[ch];

			if(myData->record_msg_1[msg].count[ch] > 0) {
				myData->record_msg_1[msg].count[ch]--;
			}
			if(myData->record_msg_1[msg].count[ch] > count) {
				count = myData->record_msg_1[msg].count[ch];
			}
			memcpy((char *)&cmd.chData[i],
				(char *)&myData->record_msg_1[msg].val[idx][ch],
				sizeof(S_RECORD_MSG_1_VAL));
			cmd.chData[i].ch = i + 1;

			userlog(DEBUG_LOG, psName, "send_cmd_ch_record_data %d\n", ch+1);
		}
	}

	if(myData->mData.config.installedCh == cnt1) {
		myData->record_msg_1[msg].total_count = 0;
	}

	if(send_flag == 0) return count;

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_RECORD_DATA);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_RECORD_DATA);
	}

	return count;
}

int send_cmd_step_cond_reply(void)
{
	return 0;
}

int send_cmd_safety_cond_reply(void)
{
	return 0;
}

void send_cmd_meter_connect_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P3_SEND_CMD_METER_CONNECT_REPLY cmd;

	cmd_size = sizeof(S_P3_SEND_CMD_METER_CONNECT_REPLY);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_CALI_METER_CONNECT_REPLY, SEQNUM_AUTO, body_size);
	
	cmd.header.chFlag[0] = 0;
	cmd.header.chFlag[1] = 0;
	cmd.state = 1; //disconnect:0, connect:1 //kjg_w
	
	rtn = send_command((char *)&cmd, cmd_size,
		P3_CMD_TO_PC_CALI_METER_CONNECT_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_CALI_METER_CONNECT_REPLY);
	}
}

void send_cmd_cali_start_reply(int ch, int code)
{
	int cmd_size, body_size, rtn, i;
	unsigned long chFlag;
	S_P3_SEND_CMD_CALI_START_REPLY cmd;

	cmd_size = sizeof(S_P3_SEND_CMD_CALI_START_REPLY);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_CALI_START_REPLY, SEQNUM_AUTO, body_size);
	
	i = myData->CellArray2[ch].number1 - 1;
	chFlag = 0x00000001;
	if(i < 32) {
		chFlag = chFlag << i;
		cmd.header.chFlag[0] = chFlag;
		cmd.header.chFlag[1] = 0;
	} else {
		chFlag = chFlag << (i-32);
		cmd.header.chFlag[0] = 0;
		cmd.header.chFlag[1] = chFlag;
	}

	cmd.header.reserved1 = 0;
	cmd.header.reserved2 = 0;
	cmd.response.cmd = P3_CMD_TO_SBC_CALI_START;
	cmd.response.code = code;
	//cmd.response.cmdSerial = header.cmd_serial; //kjg_w
	
	rtn = send_command((char *)&cmd, cmd_size,
		P3_CMD_TO_PC_CALI_START_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_CALI_START_REPLY);
	}
}

void send_cmd_cali_normal_result(int ch)
{
	int cmd_size, body_size, rtn, i, type, range, point;
	unsigned long chFlag;
	S_P3_SEND_CMD_CALI_NORMAL_RESULT cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_CALI_NORMAL_RESULT);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_CALI_NORMAL_RESULT, SEQNUM_AUTO, body_size);
	
	i = myData->CellArray2[ch].number1 - 1;
	chFlag = 0x00000001;
	if(i < 32) {
		chFlag = chFlag << i;
		cmd.header.chFlag[0] = chFlag;
		cmd.header.chFlag[1] = 0;
	} else {
		chFlag = chFlag << (i-32);
		cmd.header.chFlag[0] = 0;
		cmd.header.chFlag[1] = chFlag;
	}

	type = myData->cali[ch].type;
	range = myData->cali[ch].range;

	cmd.result.type = type;
	cmd.result.range = (unsigned char)range;
	cmd.result.setPointNum
		= myData->cali[ch].tmpCond[type][range].point.setPointNum;
	cmd.result.checkPointNum
		= myData->cali[ch].tmpCond[type][range].point.checkPointNum;
	cmd.result.ch = (unsigned char)(i+1);

	for(point=0; point < cmd.result.setPointNum; point++) {
		if(type == 0) {
			cmd.result.setPointAD[point]
				= (long)myData->cali[ch].tmpData[type][range].set_ad[point];
		} else {
			cmd.result.setPointAD[point]
				= (long)myData->cali[ch].tmpData[type][range].set_ad[point];
		}
		cmd.result.setPointDVM[point]
			= myData->cali[ch].tmpData[type][range].set_meter[point];
	}

	for(point=0; point < cmd.result.checkPointNum; point++) {
		cmd.result.checkPointAD[point]
			= myData->cali[ch].tmpData[type][range].check_ad[point];
		cmd.result.checkPointDVM[point]
			= myData->cali[ch].tmpData[type][range].check_meter[point];
	}
	
	userlog(DEBUG_LOG, psName, "cmd send cali normal result\n");

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_CALI_NORMAL_RESULT);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_CALI_NORMAL_RESULT);
	}
}

void send_cmd_cali_check_result(int ch)
{
	int cmd_size, body_size, rtn, i, type, range, point;
	unsigned long chFlag;
	S_P3_SEND_CMD_CALI_CHECK_RESULT cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_CALI_CHECK_RESULT);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_CALI_CHECK_RESULT, SEQNUM_AUTO, body_size);
	
	i = myData->CellArray2[ch].number1 - 1;
	chFlag = 0x00000001;
	if(i < 32) {
		chFlag = chFlag << i;
		cmd.header.chFlag[0] = chFlag;
		cmd.header.chFlag[1] = 0;
	} else {
		chFlag = chFlag << (i-32);
		cmd.header.chFlag[0] = 0;
		cmd.header.chFlag[1] = chFlag;
	}

	type = myData->cali[ch].type;
	range = myData->cali[ch].range;

	cmd.result.type = type;
	cmd.result.range = (unsigned char)range;
	cmd.result.checkPointNum
		= myData->cali[ch].tmpCond[type][range].point.checkPointNum;
	cmd.result.ch = (unsigned char)(i+1);

	for(point=0; point < cmd.result.checkPointNum; point++) {
		cmd.result.checkPointAD[point]
			= myData->cali[ch].tmpData[type][range].check_ad[point];
		cmd.result.checkPointDVM[point]
			= myData->cali[ch].tmpData[type][range].check_meter[point];
	}
	
	userlog(DEBUG_LOG, psName, "cmd send cali check result\n");

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_CALI_CHECK_RESULT);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_CALI_CHECK_RESULT);
	}
}

void send_cmd_comm_check_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P3_SEND_CMD_COMM_CHECK_REPLY cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_COMM_CHECK_REPLY);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_COMM_CHECK_REPLY, SEQNUM_AUTO, body_size);

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_COMM_CHECK_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_COMM_CHECK_REPLY);
	}
}

void send_cmd_comm_check(void)
{
	int cmd_size, body_size, rtn;
	S_P3_SEND_CMD_COMM_CHECK cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_COMM_CHECK);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_COMM_CHECK, SEQNUM_AUTO, body_size);

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_COMM_CHECK);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_COMM_CHECK);
	}
}

void send_cmd_trouble_code(int code, int ch)
{
	int cmd_size, body_size, rtn, val;
	S_P3_SEND_CMD_EMG_STATUS	cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_EMG_STATUS);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_EMG_STATUS, SEQNUM_AUTO, body_size);

	cmd.code = (long)code;
	if(code == P3_G_CD_FAULT_LOAD_CABLE_ERROR
		|| code == P3_G_CD_FAULT_LOAD_CABLE_ERROR2
		|| code == P3_G_CD_FAULT_UPPER_VOLTAGE
		|| code == P3_G_CD_FAULT_UPPER_CURRENT
		|| code == P3_G_CD_FAULT_GROUP_OT
		|| code == P3_G_CD_FAULT_MAIN_PS) {
		val = (int)myPs->config.groupId;
		val = (val << 16);
		val |= (ch + 1);
		memcpy((char *)&cmd.val, (char *)&val, 4);
	} else {
		cmd.val = 0;
	}

	userlog(DEBUG_LOG, psName, "send_trouble_code %d %x\n", cmd.code, cmd.val);

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_EMG_STATUS);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_EMG_STATUS);
	}
}

void send_cmd_unknown(int seqno, int ret)
{
	/*kjg_w
	int	rtn, cmd_size, body_size;
	S_P3_SEND_CMD_REQUEST cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_REQUEST);
	memset((char *)&cmd, 0, cmd_size);
	body_size = 0;
	userlog(DEBUG_LOG, psName, "ack1a %d\n", body_size);
	make_header((char*)&cmd, REPLY_NO, P3_CMD_TO_PC_RESPONSE, SEQNUM_AUTO,
		body_size);

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_RESPONSE);
	if(rtn < 0)
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_REPONSE);*/
}

void send_cmd_real_time_request(void)
{
	int cmd_size, body_size, rtn;
	S_P3_SEND_CMD_REAL_TIME_REQUEST	cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_REAL_TIME_REQUEST);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_REAL_TIME_REQUEST, SEQNUM_AUTO, body_size);

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_REAL_TIME_REQUEST);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_REAL_TIME_REQUEST);
	}
}

void send_cmd_bms_comm_reply(int can_ch, int function_div)
{
	int cmd_size, body_size, rtn;
	S_P3_SEND_CMD_BMS_COMM_REPLY	cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_BMS_COMM_REPLY);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	cmd.header.reserved1 = (unsigned short)(can_ch / 2);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_BMS_COMM_REPLY, SEQNUM_AUTO, body_size);

	cmd.function_div = (short int)function_div;
	//userlog(DEBUG_LOG, psName, "send_func_div1 %d\n", cmd.function_div);//kjg_d

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_BMS_COMM_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_BMS_COMM_REPLY);
	}
}

void send_cmd_bms_comm_end(int can_ch, int function_div)
{
	int cmd_size, body_size, rtn;
	S_P3_SEND_CMD_BMS_COMM_END	cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_BMS_COMM_END);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	cmd.header.reserved1 = (unsigned short)(can_ch / 2);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_BMS_COMM_END, SEQNUM_AUTO, body_size);

	cmd.function_div = (short int)function_div;
	//userlog(DEBUG_LOG, psName, "send_func_div2 %d\n", cmd.function_div);//kjg_d

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_BMS_COMM_END);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_BMS_COMM_END);
	}
}

void send_cmd_daq_isolation_reply(int iso)
{
	int cmd_size, body_size, rtn;
	S_P3_SEND_CMD_DAQ_ISOLATION_REPLY	cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_DAQ_ISOLATION_REPLY);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_DAQ_ISOLATION_REPLY, SEQNUM_AUTO, body_size);

	cmd.iso = (unsigned char)iso;
	//userlog(DEBUG_LOG, psName, "send_iso %d\n", cmd.iso);//kjg_d

	rtn = send_command((char *)&cmd, cmd_size,
		P3_CMD_TO_PC_DAQ_ISOLATION_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_BMS_COMM_REPLY);
	}
}

void send_cmd_cable_check_reply(void)
{
	int cmd_size, body_size, rtn, i;
	S_P3_SEND_CMD_CABLE_CHECK_REPLY	cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_CABLE_CHECK_REPLY);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_CABLE_CHECK_REPLY, SEQNUM_AUTO, body_size);

	userlog(DEBUG_LOG, psName, "send_cable_check ");
	for(i=0; i < P3_MAX_CH_PER_MODULE; i++) {
		cmd.code[i] = myData->mData.misc.cable_check.code[i]; //0:ng, 1:ok
		userlog2(DEBUG_LOG, psName, "%d ", cmd.code[i]);
	}
	userlog2(DEBUG_LOG, psName, "\n");

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_CABLE_CHECK_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_CABLE_CHECK_REPLY);
	}
}

void send_cmd_cell_check_reply(void)
{
	int cmd_size, body_size, rtn, i;
	S_P3_SEND_CMD_CELL_CHECK_REPLY	cmd;
	
	cmd_size = sizeof(S_P3_SEND_CMD_CELL_CHECK_REPLY);
	body_size = cmd_size - sizeof(S_P3_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P3_CMD_TO_PC_CELL_CHECK_REPLY, SEQNUM_AUTO, body_size);

	userlog(DEBUG_LOG, psName, "send_cell_check ");
	for(i=0; i < P3_MAX_CH_PER_MODULE; i++) {
		cmd.code[i] = myData->mData.misc.cell_check.code[i]; //0:ng, 1:ok
		userlog2(DEBUG_LOG, psName, "%d ", cmd.code[i]);
	}
	userlog2(DEBUG_LOG, psName, "\n");

	rtn = send_command((char *)&cmd, cmd_size, P3_CMD_TO_PC_CELL_CHECK_REPLY);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P3_CMD_TO_PC_CELL_CHECK_REPLY);
	}
}

void make_header(char *cmd, char reply, int cmd_id, int seqno, int body_size)
{
	int i, installedCh;
	unsigned long	tmp;
	S_P3_CMD_HEADER	header, tmpHeader;
	
	memset((char *)&header, 0, sizeof(S_P3_CMD_HEADER));
	
	cmd_id = (unsigned long)cmd_id;
	switch(cmd_id) {
		case P3_CMD_TO_PC_COMM_CHECK_REPLY:
			header.cmd_id = P3_CMD_TO_PC_RESPONSE;
			break;
		default:
			header.cmd_id = cmd_id;
			break;
	}

	installedCh = myData->mData.config.installedCh;
	if(installedCh > P3_MAX_CH_PER_MODULE) installedCh = P3_MAX_CH_PER_MODULE;

	switch(cmd_id) {
		case P3_CMD_TO_PC_CH_DATA:
		case P3_CMD_TO_PC_RECORD_DATA:
			tmp = 0x01;
			for(i=0; i < installedCh; i++) {
				if(i < 32) {
					header.chFlag[0] |= tmp;
				} else {
					header.chFlag[1] |= tmp;
				}
				tmp = tmp << 1;
				if(i == 31) tmp = 0x01;
			}
			break;
		case P3_CMD_TO_PC_CH_DATA2:
		case P3_CMD_TO_PC_BMS_COMM_REPLY:
		case P3_CMD_TO_PC_BMS_COMM_END:
			memcpy((char *)&tmpHeader, cmd, sizeof(S_P3_CMD_HEADER));
			tmp = 0x01;
			for(i=0; i < installedCh; i++) {
				if(i == (int)tmpHeader.reserved1) {
					if(i < 32) {
						header.chFlag[0] |= tmp;
					} else {
						header.chFlag[1] |= tmp;
					}
				}
				tmp = tmp << 1;
				if(i == 31) tmp = 0x01;
			}
			break;
		default:
			break;
	}
	
	header.body_size = body_size;

	if(seqno == SEQNUM_AUTO) {
		myPs->misc.cmd_serial++;
		if(myPs->misc.cmd_serial > MAX_P3_CMD_SERIAL)
			myPs->misc.cmd_serial = 1;
		header.cmd_serial = myPs->misc.cmd_serial;
	} else if(seqno == SEQNUM_NONE) {
		header.cmd_serial = myPs->misc.cmd_serial;
	}

	memcpy(cmd, (char *)&header.cmd_id, sizeof(S_P3_CMD_HEADER));
	
/*	userlog(COC_LOG, psName, "header");
	for(i=0; i < sizeof(S_P3_CMD_HEADER); i++) {
		userlog2(COC_LOG, psName, " %x", *(cmd + i));
	}
	userlog2(COC_LOG, psName, ":end\n");
	userlog(COC_LOG, psName, "header %s:end\n", cmd); //kjg_d */
}

void make_header_2(char *cmd, char reply, int cmd_id, int seqno, int body_size, int ch)
{
	int i, installedCh;
	unsigned long tmp;
	S_P3_CMD_HEADER	header;
	
	memset((char *)&header, 0, sizeof(S_P3_CMD_HEADER));
	
	cmd_id = (unsigned long)cmd_id;
	switch(cmd_id) {
		case P3_CMD_TO_PC_COMM_CHECK_REPLY:
			header.cmd_id = P3_CMD_TO_PC_RESPONSE;
			break;
		default:
			header.cmd_id = cmd_id;
			break;
	}

	installedCh = myData->mData.config.installedCh;
	if(installedCh > P3_MAX_CH_PER_MODULE) installedCh = P3_MAX_CH_PER_MODULE;

	switch(cmd_id) {
		case P3_CMD_TO_PC_PULSE_DATA:
			tmp = 0x01;
			for(i=0; i < installedCh; i++) {
				if(i < 32) {
					if(i == ch) header.chFlag[0] |= tmp;
				} else {
					if(i == ch) header.chFlag[1] |= tmp;
				}
				tmp = tmp << 1;
				if(i == 31) tmp = 0x01;
			}
			break;
		default:
			break;
	}
	
	header.body_size = body_size;

	if(seqno == SEQNUM_AUTO) {
		myPs->misc.cmd_serial++;
		if(myPs->misc.cmd_serial > MAX_P3_CMD_SERIAL)
			myPs->misc.cmd_serial = 1;
		header.cmd_serial = myPs->misc.cmd_serial;
	} else if(seqno == SEQNUM_NONE) {
		header.cmd_serial = myPs->misc.cmd_serial;
	}

	memcpy(cmd, (char *)&header.cmd_id, sizeof(S_P3_CMD_HEADER));
	
/*	userlog(COC_LOG, psName, "header");
	for(i=0; i < sizeof(S_MAIN_CMD_HEADER); i++) {
		userlog2(COC_LOG, psName, " %x", *(cmd + i));
	}
	userlog2(COC_LOG, psName, ":end\n");
	userlog(COC_LOG, psName, "header %s:end\n", cmd); //kjg_d*/
}

int	send_command(char *cmd, int size, int cmd_id)
{
	char	packet[MAX_P3_SEND_PACKET_LENGTH];
	unsigned char tmp, log_flag=0;
	int i, rtn, tmp2;

	if(size > MAX_P3_SEND_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName,
			"CMD SEND FAIL!! TOO LARGE SIZE[%d]\n", size);
		return -1;
	}
	
	memset((char *)&packet, 0, MAX_P3_SEND_PACKET_LENGTH);
	memcpy((char *)&packet, cmd, size);

/*	if(cmd.header.reply == REPLY_YES) {
		if(myPs->reply.timer_run == P0) {
			memset((char *)&myPs->reply.retry, 0, sizeof(S_P3_RETRY_DATA));

			memset(buf, 0, sizeof buf);
			strncpy(buf, (char *)&cmd.header.seqno[0],
				sizeof(cmd.header.seqno));
			myPs->reply.retry.seqno = atoi(buf);
			
			myPs->reply.retry.replyCmd = get_reply_cmdid(cmd.header.cmd_id);

			strncpy((char *)&myPs->reply.retry.buf[0], cmd, size);
				myPs->reply.retry.size = size;
				myPs->reply.timer = myData->mData.misc.timer_1sec;
				myPs->reply.timer_run = P1;
			}
	} kjg_w*/
	
	rtn = writen(myPs->misc.network_socket, packet, size);

	if(myPs->config.CmdSendLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			switch(cmd_id) {
				case P3_CMD_TO_PC_COMM_CHECK_REPLY:
				case P3_CMD_TO_PC_COMM_CHECK:
				case P3_CMD_TO_PC_CH_DATA:
				case P3_CMD_TO_PC_PULSE_DATA:
				case P3_CMD_TO_PC_CH_DATA2:
				case P3_CMD_TO_PC_RECORD_DATA:
				case P3_CMD_TO_PC_AUX_INFO_REPLY:
				case P3_CMD_TO_PC_CAN_RECEIVE_INFO_REPLY:
				case P3_CMD_TO_PC_CAN_TRANSMIT_INFO_REPLY:
					break;
				case P3_CMD_TO_PC_RESPONSE:
					tmp2 = *(cmd + sizeof(S_P3_CMD_HEADER) + sizeof(int));
					if(tmp2 != P3_CD_ACK) log_flag = 1;
					break;
				default: log_flag = 1; break;
			}
		}

		if(log_flag == 1) {
			userlog(COC_LOG, psName, "sendCmd %s:end\n", packet);
		}
	}
	
	if(myPs->config.CmdSendLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			switch(cmd_id) {
				case P3_CMD_TO_PC_COMM_CHECK_REPLY:
				case P3_CMD_TO_PC_COMM_CHECK:
				case P3_CMD_TO_PC_CH_DATA:
				case P3_CMD_TO_PC_PULSE_DATA:
				case P3_CMD_TO_PC_CH_DATA2:
				case P3_CMD_TO_PC_RECORD_DATA:
				case P3_CMD_TO_PC_AUX_INFO_REPLY:
				case P3_CMD_TO_PC_CAN_RECEIVE_INFO_REPLY:
				case P3_CMD_TO_PC_CAN_TRANSMIT_INFO_REPLY:
					break;
				case P3_CMD_TO_PC_RESPONSE:
					tmp2 = *(cmd + sizeof(S_P3_CMD_HEADER) + sizeof(int));
					if(tmp2 != P3_CD_ACK) log_flag = 1;
					//log_flag = 1; //kjg_d
					break;
				default: log_flag = 1; break;
			}
		}

		if(log_flag == 1) {
			userlog(COC_LOG, psName, "sendCmd");
			for(i=0; i < size; i++) {
				tmp = *(cmd + i);
				userlog2(COC_LOG, psName, " %02x", tmp);
			}
			userlog2(COC_LOG, psName, ":end\n");
		}
	}

	return rtn;
}

int	get_reply_cmdid(char *cmd_id)
{
	char buf[10];
	int cmdId, rtn;
	
	memset(buf, 0, sizeof buf);
	strncpy(buf, cmd_id, 4);
	cmdId = atoi(buf);
	switch(cmdId) {
		case P3_CMD_TO_SBC_COMM_CHECK:	
			rtn = P3_CMD_TO_PC_RESPONSE;	
			break;
		default:	
			rtn = 0;	
			break;
	}
	return rtn;
}

int Check_NetworkState(void)
{
	network_ping();
	check_cmd_reply_timeout();
	if(check_network_timeout() < 0) {
		close(myPs->misc.network_socket);
		userlog(DEBUG_LOG, psName, "network communication error3\n");
		myPs->signal[P3_SIG_NET_CONNECTED] = P0;
		return -1;
	}
	return 0;
}

void network_ping(void)
{
	long diff, time1, time2;

	if(myPs->signal[P3_SIG_NET_CONNECTED] != P1) return;

	if(myPs->reply.timer_run == P0) {
		time1 = myData->mData.misc.timer_1sec;
		time2 = myData->mData.misc.timer_1000ms;
		diff = (time1 - myPs->misc.ping_time) * 1000;
		diff += (time2 - myPs->misc.ping_time2);
		if(diff >= myPs->config.pingTimeout || diff < 0) {
			send_cmd_comm_check();

			myPs->misc.ping_time = time1;
			myPs->misc.ping_time2 = time2;
		}
	}
}

void check_cmd_reply_timeout(void)
{
	int rtn;
	long diff, time1, time2;
	
	if(myPs->signal[P3_SIG_NET_CONNECTED] != P1) return;
	if(myPs->reply.timer_run == P0) return;

	time1 = myData->mData.misc.timer_1sec;
	time2 = myData->mData.misc.timer_1000ms;
	diff = (time1 - myPs->reply.time) * 1000;
	diff += (time2 - myPs->reply.time2);
	if(diff >= myPs->config.replyTimeout || diff < 0) {
		userlog(DEBUG_LOG, psName,
			"TIMEOUT: retryCount[%d] replyCmd[%d] replySeqNo[%d]\n",
			myPs->reply.retry.count, myPs->reply.retry.replyCmd,
			myPs->reply.retry.seqno);
		if(myPs->reply.retry.count >= myPs->config.retryCount) {
			myPs->reply.timer_run = P0;
			myPs->reply.time = time1;
			myPs->reply.time2 = time2;
			memset((char *)&myPs->reply.retry, 0, sizeof(S_P3_RETRY_DATA));
			//kjg_w : next working
		} else {
			myPs->reply.timer_run = P1;
			myPs->reply.time = time1;
			myPs->reply.time2 = time2;
			myPs->reply.retry.count++;
				
			if(myPs->config.CmdSendLog == P1) {
				userlog(DEBUG_LOG, psName, "retry %s\n", myPs->reply.retry.buf);
			} //kjg_w hex

			rtn = writen(myPs->misc.send_socket,
				(char *)&myPs->reply.retry.buf, myPs->reply.retry.size);
		}
	}
}

int	check_network_timeout(void)
{
	long diff, time1, time2;

	if(myPs->signal[P3_SIG_NET_CONNECTED] != P1) return 0;

	if(myData->AppControl.signal[APP_SIG_MODULE_CONTROL_PROCESS] == P1) {
		time1 = myData->mData.misc.timer_1sec;
		time2 = myData->mData.misc.timer_1000ms;
		diff = (time1 - myPs->misc.net_time) * 1000;
		diff += (time2 - myPs->misc.net_time2);
		if(diff >= myPs->config.netTimeout || diff < 0) return -1;
	}
	return 0;
}

