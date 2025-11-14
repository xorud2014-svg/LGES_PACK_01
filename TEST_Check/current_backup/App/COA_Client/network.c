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
extern volatile S_COA_CLIENT *myPs;
extern volatile S_TEST_CONDITION *myTestCond; //kjg_170810
extern char psName[PROCESS_NAME_SIZE];

int InitNetwork(void)
{
	sleep(2);

	if(myPs->misc.network_socket1 > 0) close(myPs->misc.network_socket1);
	
    myPs->misc.network_socket1 = SetClientSock(myPs->config.networkPort1,
		(char *)&myPs->config.ipAddr);

    if(myPs->misc.network_socket1 < 0) {
		close(myPs->misc.network_socket1);
	    userlog(DEBUG_LOG, psName, "Can not initialize network1 : %d\n",
			myPs->misc.network_socket1);
		return -1;
    }

	if(myPs->misc.network_port_type == 2) {
		if(myPs->misc.network_socket2 > 0) close(myPs->misc.network_socket2);
	
    	myPs->misc.network_socket2 = SetClientSock(myPs->config.networkPort2,
			(char *)&myPs->config.ipAddr);

	    if(myPs->misc.network_socket2 < 0) {
			close(myPs->misc.network_socket1);
			close(myPs->misc.network_socket2);
		    userlog(DEBUG_LOG, psName, "Can not initialize network2 : %d\n",
				myPs->misc.network_socket2);
			return -2;
		}
	}

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms; //jhkw_190503

	userlog(DEBUG_LOG, psName, "command socket connected : %d, %d\n",
		myPs->misc.network_socket1, myPs->misc.network_socket2);
    return 0;
}

int NetworkPacket_Receive(void)
{
	char maxPacketBuf[MAX_P1_RECV_PACKET_LENGTH];
	int rcv_size, read_size, i, start, index, toPs;
	S_MSG_VAL SendMsg;

	memset(maxPacketBuf, 0, MAX_P1_RECV_PACKET_LENGTH);
		
	if(ioctl(myPs->misc.network_socket1, FIONREAD, &rcv_size) < 0) {
		userlog(DEBUG_LOG, psName, "packet receive ioctl error\n");
		close(myPs->misc.network_socket1);
		if(myPs->misc.network_socket2 > 0) close(myPs->misc.network_socket2);
		return -1;
	}

	if(rcv_size > MAX_P1_RECV_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName, "max packet size over\n");
		read_size = readn(myPs->misc.network_socket1, maxPacketBuf,
			MAX_P1_RECV_PACKET_LENGTH);
		if(read_size != MAX_P1_RECV_PACKET_LENGTH)
			userlog(DEBUG_LOG, psName, "packet readn size error1\n");
		close(myPs->misc.network_socket1);
		if(myPs->misc.network_socket2 > 0) close(myPs->misc.network_socket2);
		return -2;
	} else if(rcv_size
		> (MAX_P1_RECV_PACKET_LENGTH - myPs->rcvPacket.usedBufSize)) {
		userlog(DEBUG_LOG, psName, "packet buffer overflow\n");
		read_size = readn(myPs->misc.network_socket1, maxPacketBuf, rcv_size);
		if(read_size != rcv_size)
			userlog(DEBUG_LOG, psName, "packet readn size error2\n");
		close(myPs->misc.network_socket1);
		if(myPs->misc.network_socket2 > 0) close(myPs->misc.network_socket2);
		return -3;
	} else if(rcv_size <= 0) {
		userlog(DEBUG_LOG, psName, "packet sock_rcv error %d\n", rcv_size);
		close(myPs->misc.network_socket1);
		if(myPs->misc.network_socket2 > 0) close(myPs->misc.network_socket2);

		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		toPs = COA1_TO_APP + myPs->config.groupNo;
		SendMsg.msg = MSG_COA_APP_PROCESS_KILL;
		SendMsg.val[0] = (int)myPs->config.groupNo;
		send_msg(toPs, (char *)&SendMsg);
		return -4;
	} else {
		read_size = readn(myPs->misc.network_socket1, maxPacketBuf, rcv_size);
		if(read_size != rcv_size) {
			userlog(DEBUG_LOG, psName, "packet readn size error3 : %d, %d\n",
				read_size, rcv_size);
			close(myPs->misc.network_socket1);
			if(myPs->misc.network_socket2 > 0)
				close(myPs->misc.network_socket2);
			return -5;
		}
	}
	
	i = myPs->rcvPacket.rcvCount;
	myPs->rcvPacket.rcvCount++;
	if(myPs->rcvPacket.rcvCount > (MAX_P1_RECV_PACKET_COUNT-1))
		myPs->rcvPacket.rcvCount = 0;
	
	if(i == 0) index = MAX_P1_RECV_PACKET_COUNT - 1;
	else index = i - 1;
	start = myPs->rcvPacket.rcvStartPoint[index]
		+ myPs->rcvPacket.rcvSize[index];
	if(start >= MAX_P1_RECV_PACKET_LENGTH) {
		myPs->rcvPacket.rcvStartPoint[i]
			= abs(start - MAX_P1_RECV_PACKET_LENGTH);
	} else {
		myPs->rcvPacket.rcvStartPoint[i] = start;
	}

	myPs->rcvPacket.rcvSize[i] = read_size;
	myPs->rcvPacket.usedBufSize += read_size;
	
	start = myPs->rcvPacket.rcvStartPoint[i];
	if((start + read_size) > MAX_P1_RECV_PACKET_LENGTH) {
		index = MAX_P1_RECV_PACKET_LENGTH - start;
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
	//char debug[MAX_P1_RECV_PACKET_LENGTH]; //kjg_d
	int i, j, k, cmdBuf_index, start_point;
	
	if(myPs->rcvPacket.rcvCount == myPs->rcvPacket.parseCount) return;
	
	i = myPs->rcvPacket.parseCount;
	myPs->rcvPacket.parseCount++;
	if(myPs->rcvPacket.parseCount > (MAX_P1_RECV_PACKET_COUNT-1))
		myPs->rcvPacket.parseCount = 0;
	
	cmdBuf_index = myPs->rcvCmd.cmdBufSize;
	myPs->rcvCmd.cmdBufSize += myPs->rcvPacket.rcvSize[i];
	
	start_point = myPs->rcvPacket.parseStartPoint[i];

/*	userlog(COA_LOG, psName, "recvCmd1 %s:end %d %d\n", 
		myPs->rcvCmd.cmdBuf, i, start_point);
		
	memset(debug, 0, sizeof debug);
	memcpy((char *)&debug, (char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
		myPs->rcvPacket.rcvSize[i]);
	userlog(COA_LOG, psName, "recvCmd2 %s:end %d %d\n",
		debug, cmdBuf_index, myPs->rcvPacket.rcvSize[i]); //kjg_d*/

	j = start_point + myPs->rcvPacket.rcvSize[i];
	if(j <= MAX_P1_RECV_PACKET_LENGTH) {
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point],
			myPs->rcvPacket.rcvSize[i]);
	} else {
		k = MAX_P1_RECV_PACKET_LENGTH - start_point;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
		cmdBuf_index += k;
		start_point = 0;
		k = j - MAX_P1_RECV_PACKET_LENGTH;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
	}
		
	start_point = myPs->rcvPacket.parseStartPoint[i]
		+ myPs->rcvPacket.rcvSize[i];
	if(start_point >= MAX_P1_RECV_PACKET_LENGTH) {
		j = i + 1;
		if(j >= MAX_P1_RECV_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j]
			= abs(start_point - MAX_P1_RECV_PACKET_LENGTH);
	} else {
		j = i + 1;
		if(j >= MAX_P1_RECV_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j] = start_point;
	}
		
	myPs->rcvPacket.usedBufSize -= myPs->rcvPacket.rcvSize[i];
	
/*	userlog(COA_LOG, psName, "recvCmd3 %d %d\n", myPs->rcvPacket.usedBufSize,
		myPs->rcvCmd.cmdBufSize); //kjg_d*/
}

int NetworkCommand_Receive(void)
{
	//char debug[MAX_P1_RECV_PACKET_LENGTH]; //kjg_d
	char tmpBuf[MAX_P1_RECV_PACKET_LENGTH];
	int cmd_size, cmdBuf_index;
	S_P1_CMD_HEADER header;

	if(myPs->rcvCmd.cmdBufSize < sizeof(S_P1_CMD_HEADER)) return -1;
	
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmdBuf,
		sizeof(S_P1_CMD_HEADER));
	cmd_size = header.body_size + sizeof(S_P1_CMD_HEADER);
	if(myPs->rcvCmd.cmdBufSize < cmd_size) return -2;
			
/*	userlog(COA_LOG, psName, "recvCmd4 %s:end %d\n",
		myPs->rcvCmd.cmdBuf, cmd_size); //kjg_d*/
	
	memset((char *)&myPs->rcvCmd.cmd, 0, MAX_P1_RECV_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmd, (char *)&myPs->rcvCmd.cmdBuf, cmd_size);
	myPs->rcvCmd.cmdSize = cmd_size;
	
	cmdBuf_index = cmd_size;
	myPs->rcvCmd.cmdBufSize -= cmd_size;
	cmd_size = myPs->rcvCmd.cmdBufSize;
	memset((char *)&tmpBuf[0], 0, MAX_P1_RECV_PACKET_LENGTH);
	memcpy((char *)&tmpBuf[0],
		(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
	memset((char *)&myPs->rcvCmd.cmdBuf[0], 0, MAX_P1_RECV_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmdBuf[0], (char *)&tmpBuf[0], cmd_size);
	
/*	memset(debug, 0, sizeof debug);
	memcpy((char *)&debug, (char *)&myPs->rcvCmd.cmdBuf, cmd_size);
	userlog(COA_LOG, psName, "recvCmd5 %s:end %d %d\n",
		debug, cmd_size, cmdBuf_index); //kjg_d*/

	return 0;
}

int NetworkCommand_Parsing(void)
{
	unsigned char ch, msg;	//jhkw_190830
	unsigned char tmp, log_flag;
	int	rtn, i;
	S_P1_CMD_HEADER header;
	S_P1_RCV_CMD_RESPONSE	cmd;

	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd, sizeof(S_P1_CMD_HEADER));
	
	if(myPs->config.CmdRcvLog == P1) {
		log_flag = 0;
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			switch(header.cmd_id) {
				case P1_CMD_TO_SBC_COMM_CHECK:
				case P1_CMD_TO_SBC_COMM_CHECK_2: //jhkw_200508
				case P1_CMD_TO_SBC_AUX_SET:
				case P1_CMD_TO_SBC_CAN_RECEIVE_SET:
				case P1_CMD_TO_SBC_CAN_TRANSMIT_SET:
				case P1_CMD_TO_SBC_CH_CAN_RECEIVE_SET:
				case P1_CMD_TO_SBC_CH_CAN_TRANSMIT_SET:
				case P1_CMD_TO_SBC_TESTCOND_STEP:
				case P1_CMD_TO_SBC_TESTCOND_PATTERN:
					break;
				case P1_CMD_TO_SBC_RESPONSE:
					memset((char *)&cmd, 0, sizeof(S_P1_RCV_CMD_RESPONSE));
					memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
						sizeof(S_P1_RCV_CMD_RESPONSE));
					if(cmd.response.code != P1_CD_ACK) log_flag = 1;
					break;
				default: log_flag = 1; break;
			}
		}

		if(log_flag == 1) {
			userlog(COA_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		}
	}
	
	if(myPs->config.CmdRcvLog_Hex == P1) {
		log_flag = 0;
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			switch(header.cmd_id) {
				case P1_CMD_TO_SBC_COMM_CHECK:
				case P1_CMD_TO_SBC_COMM_CHECK_2: //jhkw_200508
				case P1_CMD_TO_SBC_AUX_SET:
				case P1_CMD_TO_SBC_CAN_RECEIVE_SET:
				case P1_CMD_TO_SBC_CAN_TRANSMIT_SET:
				case P1_CMD_TO_SBC_CH_CAN_RECEIVE_SET:
				case P1_CMD_TO_SBC_CH_CAN_TRANSMIT_SET:
				case P1_CMD_TO_SBC_TESTCOND_STEP:
				case P1_CMD_TO_SBC_TESTCOND_PATTERN:
					break;
				case P1_CMD_TO_SBC_RESPONSE:
					memset((char *)&cmd, 0, sizeof(S_P1_RCV_CMD_RESPONSE));
					memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
						sizeof(S_P1_RCV_CMD_RESPONSE));
					if(cmd.response.code != P1_CD_ACK) log_flag = 1;
					//log_flag = 1; //kjg_d
					break;
				default: log_flag = 1; break;
			}
		}

		if(log_flag == 1) {
			userlog(COA_LOG, psName, "recvCmd");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				tmp = myPs->rcvCmd.cmd[i];
				userlog2(COA_LOG, psName, " %02x", tmp);
			}
			userlog2(COA_LOG, psName, ":end\n");
		}
	}
	//jhkw_190830s
	ch = msg = 0;
#ifdef __10MS__
	if(cmd.response.code == P1_CD_ACK && cmd.response.cmd == P1_CMD_TO_PC_CH_DATA3) {
		if(cmd.header.chFlag[0] == 1) {
			ch = 0;
		} else if(cmd.header.chFlag[0] == 2) {
			ch = 1;
		} else if(cmd.header.chFlag[0] == 4) {
			ch = 2;
		} else if(cmd.header.chFlag[0] == 8) {
			ch = 3;
		}
		myData->signal_10ms[ch].response_msg_count 
			= myData->signal_10ms[ch].send_msg_count;
		msg = myData->signal_10ms[ch].response_msg_count;
		if(myData->mData.signal[M_SIG_DATA_SAVE_ERROR] == P1) {
			userlog(DEBUG_LOG, psName, "retry ch - %d, response - %d, send - %d, index - %ld\n",
				ch, myData->signal_10ms[ch].response_msg_count,
				myData->signal_10ms[ch].send_msg_count,
				myData->save_msg_10ms[ch][msg]
					.val[myData->save_msg_10ms[ch][msg].count_100].chData.resultIndex);
			myData->save_msg_10ms[ch][msg].count_100 = 0;
		} else {
			userlog(DEBUG_LOG, psName, "ack ch - %d, response - %d, send - %d, index - %ld\n",
				ch, myData->signal_10ms[ch].response_msg_count,
				myData->signal_10ms[ch].send_msg_count,
				myData->save_msg_10ms[ch][msg]
					.val[myData->save_msg_10ms[ch][msg].count_100].chData.resultIndex);
		}
	}
#endif
	//jhkw_190830e
	
	rtn = CmdHeader_Check((char *)&header);
	if(rtn < 0) return -1;
	
/*	rtn = Check_ReplyCmd((char *)&header);
	if(rtn < 0) return -2; //kjg_w*/

	switch(header.cmd_id) {
		case P1_CMD_TO_SBC_MODULE_INFO_REQUEST:
			rtn = rcv_cmd_module_info_request();
			break;
		case P1_CMD_TO_SBC_MODULE_SET_DATA:
			rtn = rcv_cmd_module_set_data();
			break;
		case P1_CMD_TO_SBC_AUX_INFO_REQUEST:
			rtn = rcv_cmd_aux_info_request();
			break;
		case P1_CMD_TO_SBC_CAN_RECEIVE_INFO_REQUEST:
			rtn = rcv_cmd_can_receive_info_request();
			break;
		case P1_CMD_TO_SBC_CAN_TRANSMIT_INFO_REQUEST:
			rtn = rcv_cmd_can_transmit_info_request();
			break;
		case P1_CMD_TO_SBC_CH_ATTRIBUTE_REQUEST:
			rtn = rcv_cmd_ch_attribute_request();
			break;
		case P1_CMD_TO_SBC_RUN:
			rtn = rcv_cmd_run();
			break;
		case P1_CMD_TO_SBC_STOP:
			rtn = rcv_cmd_stop();
			break;
		case P1_CMD_TO_SBC_PAUSE:
			rtn = rcv_cmd_pause();
			break;
		case P1_CMD_TO_SBC_CONTINUE:
			rtn = rcv_cmd_continue();
			break;
		case P1_CMD_TO_SBC_NEXT_STEP:
			rtn = rcv_cmd_next_step();
			break;
		case P1_CMD_TO_SBC_GOTO_STEP: //kjhw_151211
			rtn = rcv_cmd_goto_step();
			break;
		case P1_CMD_TO_SBC_CHAMBER_FLAG:
			rtn = rcv_cmd_chamber_flag();
			break;
		case P1_CMD_TO_SBC_CHAMBER_CONTINUE:
			rtn = rcv_cmd_chamber_continue();
			break;
		case P1_CMD_TO_SBC_CABLE_CHECK:
			rtn = rcv_cmd_cable_check();
			break;
		case P1_CMD_TO_SBC_CELL_CHECK:
			rtn = rcv_cmd_cell_check();
			break;
		case P1_CMD_TO_SBC_CYCLE_CONTINUE:
			rtn = rcv_cmd_cycle_continue();
			break;
		case P1_CMD_TO_SBC_CAN_COMM_CHECK: //kjhw_141201
			rtn = rcv_cmd_can_comm_check();
			break;
		case P1_CMD_TO_SBC_OUT_MUX_USE: //kjhw_151021
			rtn = rcv_cmd_out_mux_use();
			break;
		case P1_CMD_TO_SBC_TESTCOND_START:
			rtn = rcv_cmd_testcond_start();
			break;
		case P1_CMD_TO_SBC_TESTCOND_SAFETY:
			rtn = rcv_cmd_testcond_safety();
			break;
		case P1_CMD_TO_SBC_TESTCOND_STEP:
			rtn = rcv_cmd_testcond_step();
			break;
		case P1_CMD_TO_SBC_TESTCOND_END:
			rtn = rcv_cmd_testcond_end();
			break;
		case P1_CMD_TO_SBC_TESTCOND_PATTERN:
			rtn = rcv_cmd_testcond_pattern();
			break;
		case P1_CMD_TO_SBC_STEP_COND_REQUEST: //kjg_170810_s
			rtn = rcv_cmd_step_cond_request();
			break;
		case P1_CMD_TO_SBC_STEP_COND_UPDATE:
			rtn = rcv_cmd_step_cond_update();
			break;
		case P1_CMD_TO_SBC_COMMON_SAFETY_COND_REQUEST:
			rtn = rcv_cmd_common_safety_cond_request();
			break;
		case P1_CMD_TO_SBC_COMMON_SAFETY_COND_UPDATE:
			rtn = rcv_cmd_common_safety_cond_update();
			break; //kjg_170810_e
		case P1_CMD_TO_SBC_RESET_RESERVED_CMD:
			rtn = rcv_cmd_reset_reserved_cmd();
			break;
		case P1_CMD_TO_SBC_TESTCOND_UPDATE_START: //kjg_170810_s
			rtn = rcv_cmd_testcond_update_start();
			break;
		case P1_CMD_TO_SBC_TESTCOND_UPDATE_END:
			rtn = rcv_cmd_testcond_update_end();
			break;
		//case P1_CMD_TO_SBC_TESTCOND_PATTERN_UPDATE:
		//	rtn = rcv_cmd_testcond_pattern_update();
		//	break; //kjg_170810_e
		case P1_CMD_TO_SBC_CALI_METER_CONNECT:
			rtn = rcv_cmd_cali_meter_connect();
			break;
		case P1_CMD_TO_SBC_CALI_START:
			rtn = rcv_cmd_cali_start();
			break;
		case P1_CMD_TO_SBC_CALI_UPDATE:
			rtn = rcv_cmd_cali_update();
			break;
		case P1_CMD_TO_SBC_RESPONSE:
			rtn = rcv_cmd_response();
			break;
		case P1_CMD_TO_SBC_COMM_CHECK:
			rtn = rcv_cmd_comm_check();
			break;
		//jhkw_200508s
		case P1_CMD_TO_SBC_COMM_CHECK_2:
			rtn = rcv_cmd_comm_check_2();
			break;
		//jhkw_200508e
		/*case P1_CMD_TO_SBC_RESET:
			rtn = rcv_cmd_reset();
			break; kjg_w*/
		case P1_CMD_TO_SBC_CH_ATTRIBUTE_SET:
			rtn = rcv_cmd_ch_attribute_set();
			break;
		case P1_CMD_TO_SBC_AUX_SET:
			rtn = rcv_cmd_aux_set();
			break;
		case P1_CMD_TO_SBC_CAN_RECEIVE_SET:
			rtn = rcv_cmd_can_receive_set();
			break;
		case P1_CMD_TO_SBC_CH_CAN_RECEIVE_SET:
			rtn = rcv_cmd_ch_can_receive_set();
			break;
		case P1_CMD_TO_SBC_CAN_TRANSMIT_SET:
			rtn = rcv_cmd_can_transmit_set();
			break;
		case P1_CMD_TO_SBC_CH_CAN_TRANSMIT_SET:
			rtn = rcv_cmd_ch_can_transmit_set();
			break;
		case P1_CMD_TO_SBC_REAL_TIME_REPLY:
			rtn = rcv_cmd_real_time_reply();
			break;
		case P1_CMD_TO_SBC_BMS_COMM_REQUEST:
			rtn = rcv_cmd_bms_comm_request();
			break;
		case P1_CMD_TO_SBC_CAN_TRANSMIT_CHANGE:
			rtn = rcv_cmd_can_transmit_change();
			break;
		case P1_CMD_TO_SBC_BMS_EOL_DATA1_REQUEST:
			rtn = rcv_cmd_bms_eol_data1_request();
			break;
		case P1_CMD_TO_SBC_BMS_EOL_DATA2_REQUEST:
			rtn = rcv_cmd_bms_eol_data2_request();
			break;
		case P1_CMD_TO_SBC_BMS_EOL_PACK_ID_WRITE:
			rtn = rcv_cmd_bms_eol_pack_id_write();
			break;
		case P1_CMD_TO_SBC_BMS_EOL_PACK_ID_CHECK:
			rtn = rcv_cmd_bms_eol_pack_id_check();
			break;
		case P1_CMD_TO_SBC_BMS_EOL_HI_POT_TEST:
			rtn = rcv_cmd_bms_eol_hi_pot_test();
			break;
		case P1_CMD_TO_SBC_UDS_VBF_INFO_REQUEST:
			rtn = rcv_cmd_uds_vbf_info_request();
			break;
		case P1_CMD_TO_SBC_UDS_VBF_CHANGE_REQUEST:
			rtn = rcv_cmd_uds_vbf_change_request();
			break;
		case P1_CMD_TO_SBC_BMS_EOL_DATA3_REQUEST:
			rtn = rcv_cmd_bms_eol_data3_request();
			break;
		case P1_CMD_TO_SBC_BMS_EOL_DATA3_LED_REQUEST:
			rtn = rcv_cmd_bms_eol_data3_led_request();
			break;
		case P1_CMD_TO_SBC_BMS_EOL_CVTN_ID_REQUEST:
			rtn = rcv_cmd_bms_eol_cvtn_id_request();
			break;
		case P1_CMD_TO_SBC_BMS_EOL_DTC_CLEAR:
			rtn = rcv_cmd_bms_eol_dtc_clear();
			break;
		case P1_CMD_TO_SBC_BMS_EOL_RESET_MICRO:
			rtn = rcv_cmd_bms_eol_reset_micro();
			break;
		case P1_CMD_TO_SBC_BMS_EOL_3P_SYSTEM_REQUEST: //kjg_120619
			rtn = rcv_cmd_bms_eol_3p_system_request();
			break;
		case P1_CMD_TO_SBC_DAQ_ISOLATION_REQUEST:
			rtn = rcv_cmd_daq_isolation_request();
			break;
		case P1_CMD_TO_SBC_DAQ_ISOLATION_REQUEST2: //jhkw_150224
			rtn = rcv_cmd_daq_isolation_request2();
			break;
		case P1_CMD_TO_SBC_OUT_MUX_SELECT_REQUEST: //kjhw_151021
			rtn = rcv_cmd_out_mux_select_request();
			break;
		case P1_CMD_TO_SBC_TH_TABLE_SET: //jhkw_191108
			rtn = rcv_cmd_th_table_set();
			break;
		case P1_CMD_TO_SBC_EOL_PROCEDURE_REQUEST: //kjg_120709
			rtn = rcv_cmd_eol_procedure_request();
			break;
		case P1_CMD_TO_SBC_BMS_PROCEDURE_REQUEST: //kjg_161207
			rtn = rcv_cmd_bms_procedure_request();
			break;
		case P1_CMD_TO_SBC_CALIMODE: //jhkw_130119
			rtn = rcv_cmd_calimode_request();
			break;
		case P1_CMD_TO_SBC_CALI_STOP:	//jhkw_200317s
			rtn = rcv_cmd_cali_stop();
			break;
		case P1_CMD_TO_SBC_AUTOCALI_REQUEST:
			rtn = rcv_cmd_autocali_request();
			break;
		case P1_CMD_TO_SBC_AUTOCALI_TEMP_REQUEST:
			rtn = rcv_cmd_autocali_temp_request();
			break;	//jhkw_200317e
		case P1_CMD_TO_SBC_LOAD_TYPE_SET: //kjhw_130129
			rtn = rcv_cmd_load_type_set();
			break;
		case P1_CMD_TO_SBC_CH_IO_SET:
			rtn = rcv_cmd_ch_io_set();
			break;
		case P1_CMD_TO_SBC_BUZZER_STOP:
			rtn = rcv_cmd_buzzer_stop();
			break;
		case P1_CMD_TO_SBC_ALARM_RESET:
			rtn = rcv_cmd_alarm_reset();
			break;
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
			//if(myPs->rcvCmd.cmdFail >= 3) {
			if(myPs->rcvCmd.cmdFail >= 1) { //kjg_101104
				myPs->rcvCmd.cmdFail = 0;
				myPs->rcvCmd.cmdBufSize = 0;
				memset((char *)&myPs->rcvCmd.cmdBuf, 0,
					MAX_P1_RECV_PACKET_LENGTH);
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
	S_P1_CMD_HEADER	header;
	
	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_P1_CMD_HEADER));
	
	switch(header.cmd_id) {
		case P1_CMD_TO_SBC_MODULE_INFO_REQUEST:
			length = sizeof(S_P1_RCV_CMD_MODULE_INFO_REQUEST); 
			break;
		case P1_CMD_TO_SBC_MODULE_SET_DATA:
			length = sizeof(S_P1_RCV_CMD_MODULE_SET_DATA); 
			break;
		case P1_CMD_TO_SBC_AUX_INFO_REQUEST:
			length = sizeof(S_P1_RCV_CMD_AUX_INFO_REQUEST); 
			break;
		case P1_CMD_TO_SBC_CAN_RECEIVE_INFO_REQUEST:
			length = sizeof(S_P1_RCV_CMD_CAN_RECEIVE_INFO_REQUEST); 
			break;
		case P1_CMD_TO_SBC_CAN_TRANSMIT_INFO_REQUEST:
			length = sizeof(S_P1_RCV_CMD_CAN_TRANSMIT_INFO_REQUEST); 
			break;
		case P1_CMD_TO_SBC_CH_ATTRIBUTE_REQUEST:
			length = sizeof(S_P1_RCV_CMD_CH_ATTRIBUTE_REQUEST);
			break;
		case P1_CMD_TO_SBC_RUN:
			length = sizeof(S_P1_RCV_CMD_RUN); 
			break;
		case P1_CMD_TO_SBC_STOP:
			length = sizeof(S_P1_RCV_CMD_STOP); 
			break;
		case P1_CMD_TO_SBC_TESTCOND_START:
			length = sizeof(S_P1_RCV_CMD_TESTCOND_START); 
			break;
		case P1_CMD_TO_SBC_TESTCOND_SAFETY:
			length = sizeof(S_P1_RCV_CMD_TESTCOND_SAFETY);
			break;
		case P1_CMD_TO_SBC_TESTCOND_STEP:
			length = sizeof(S_P1_RCV_CMD_TESTCOND_STEP); 
			break;
		case P1_CMD_TO_SBC_TESTCOND_END:
			length = sizeof(S_P1_RCV_CMD_TESTCOND_END); 
			break;
		case P1_CMD_TO_SBC_STEP_COND_REQUEST:
			length = sizeof(S_P1_RCV_CMD_STEP_COND_REQUEST); 
			break;
		case P1_CMD_TO_SBC_STEP_COND_UPDATE:
			length = sizeof(S_P1_RCV_CMD_STEP_COND_UPDATE); 
			break;
		case P1_CMD_TO_SBC_COMMON_SAFETY_COND_REQUEST:
			length = sizeof(S_P1_RCV_CMD_COMMON_SAFETY_COND_REQUEST); 
			break;
		case P1_CMD_TO_SBC_COMMON_SAFETY_COND_UPDATE:
			length = sizeof(S_P1_RCV_CMD_COMMON_SAFETY_COND_UPDATE); 
			break;
		case P1_CMD_TO_SBC_RESET_RESERVED_CMD:
			length = sizeof(S_P1_RCV_CMD_RESET_RESERVED_CMD); 
			break;
		case P1_CMD_TO_SBC_TESTCOND_UPDATE_START:
			length = sizeof(S_P1_RCV_CMD_TESTCOND_UPDATE_START); 
			break;
		case P1_CMD_TO_SBC_TESTCOND_UPDATE_END:
			length = sizeof(S_P1_RCV_CMD_TESTCOND_UPDATE_END); 
			break;
		case P1_CMD_TO_SBC_CONTINUE:
			length = sizeof(S_P1_RCV_CMD_CONTINUE); 
			break;
		case P1_CMD_TO_SBC_PAUSE:
			length = sizeof(S_P1_RCV_CMD_PAUSE); 
			break;
		case P1_CMD_TO_SBC_NEXT_STEP:
			length = sizeof(S_P1_RCV_CMD_NEXT_STEP); 
			break;
		case P1_CMD_TO_SBC_CHAMBER_FLAG:
			length = sizeof(S_P1_RCV_CMD_CHAMBER_FLAG); 
			break;
		case P1_CMD_TO_SBC_CHAMBER_CONTINUE:
			length = sizeof(S_P1_RCV_CMD_CHAMBER_CONTINUE); 
			break;
		case P1_CMD_TO_SBC_CABLE_CHECK:
			length = sizeof(S_P1_RCV_CMD_CABLE_CHECK); 
			break;
		case P1_CMD_TO_SBC_CELL_CHECK:
			length = sizeof(S_P1_RCV_CMD_CELL_CHECK); 
			break;
		case P1_CMD_TO_SBC_CYCLE_CONTINUE:
			length = sizeof(S_P1_RCV_CMD_CYCLE_CONTINUE); 
			break;
		case P1_CMD_TO_SBC_CAN_COMM_CHECK: //kjhw_141201
			length = sizeof(S_P1_RCV_CMD_CAN_COMM_CHECK); 
			break;
		case P1_CMD_TO_SBC_OUT_MUX_USE: //kjhw_151021
			length = sizeof(S_P1_RCV_CMD_OUT_MUX_USE); 
			break;
		case P1_CMD_TO_SBC_RESPONSE:
			length = sizeof(S_P1_RCV_CMD_RESPONSE); 
			break;
		case P1_CMD_TO_SBC_CALI_METER_CONNECT:
			length = sizeof(S_P1_RCV_CMD_CALI_METER_CONNECT); 
			break;
		case P1_CMD_TO_SBC_CALI_START:
			length = sizeof(S_P1_RCV_CMD_CALI_START); 
			break;
		case P1_CMD_TO_SBC_CALI_UPDATE:
			length = sizeof(S_P1_RCV_CMD_CALI_UPDATE); 
			break;
		case P1_CMD_TO_SBC_COMM_CHECK:
			length = sizeof(S_P1_RCV_CMD_COMM_CHECK); 
			break;
		//jhkw_200508s
		case P1_CMD_TO_SBC_COMM_CHECK_2:
			length = sizeof(S_P1_RCV_CMD_COMM_CHECK_2); 
			break;
		//jhkw_200508e
		case P1_CMD_TO_SBC_CH_ATTRIBUTE_SET:
			length = sizeof(S_P1_RCV_CMD_CH_ATTRIBUTE_SET); 
			break;
		case P1_CMD_TO_SBC_AUX_SET:
			length = sizeof(S_P1_RCV_CMD_AUX_SET);
			break;
		case P1_CMD_TO_SBC_CAN_RECEIVE_SET:
			length = sizeof(S_P1_RCV_CMD_CAN_RECEIVE_SET);
			break;
		case P1_CMD_TO_SBC_CAN_TRANSMIT_SET:
			length = sizeof(S_P1_RCV_CMD_CAN_TRANSMIT_SET);
			break;
		case P1_CMD_TO_SBC_CH_CAN_RECEIVE_SET:
			length = sizeof(S_P1_RCV_CMD_CH_CAN_RECEIVE_SET);
			break;
		case P1_CMD_TO_SBC_CH_CAN_TRANSMIT_SET:
			length = sizeof(S_P1_RCV_CMD_CH_CAN_TRANSMIT_SET);
			break;
		case P1_CMD_TO_SBC_TESTCOND_PATTERN:
			length = sizeof(S_P1_RCV_CMD_TESTCOND_PATTERN);
			break;
		case P1_CMD_TO_SBC_REAL_TIME_REPLY:
			length = sizeof(S_P1_RCV_CMD_REAL_TIME_REPLY);
			break;
		case P1_CMD_TO_SBC_BMS_COMM_REQUEST:
			length = sizeof(S_P1_RCV_CMD_BMS_COMM_REQUEST);
			break;
		case P1_CMD_TO_SBC_CAN_TRANSMIT_CHANGE:
			length = sizeof(S_P1_RCV_CMD_CAN_TRANSMIT_CHANGE);
			break;
		case P1_CMD_TO_SBC_BMS_EOL_DATA1_REQUEST:
			length = sizeof(S_P1_RCV_CMD_BMS_EOL_DATA1_REQUEST);
			break;
		case P1_CMD_TO_SBC_BMS_EOL_DATA2_REQUEST:
			length = sizeof(S_P1_RCV_CMD_BMS_EOL_DATA2_REQUEST);
			break;
		case P1_CMD_TO_SBC_BMS_EOL_PACK_ID_WRITE:
			length = sizeof(S_P1_RCV_CMD_BMS_EOL_PACK_ID_WRITE);
			break;
		case P1_CMD_TO_SBC_BMS_EOL_PACK_ID_CHECK:
			length = sizeof(S_P1_RCV_CMD_BMS_EOL_PACK_ID_CHECK);
			break;
		case P1_CMD_TO_SBC_BMS_EOL_HI_POT_TEST:
			length = sizeof(S_P1_RCV_CMD_BMS_EOL_HI_POT_TEST);
			break;
		case P1_CMD_TO_SBC_UDS_VBF_INFO_REQUEST:
			length = sizeof(S_P1_RCV_CMD_UDS_VBF_INFO_REQUEST);
			break;
		case P1_CMD_TO_SBC_UDS_VBF_CHANGE_REQUEST:
			length = sizeof(S_P1_RCV_CMD_UDS_VBF_CHANGE_REQUEST);
			break;
		case P1_CMD_TO_SBC_BMS_EOL_DATA3_REQUEST:
			length = sizeof(S_P1_RCV_CMD_BMS_EOL_DATA3_REQUEST);
			break;
		case P1_CMD_TO_SBC_BMS_EOL_DATA3_LED_REQUEST:
			length = sizeof(S_P1_RCV_CMD_BMS_EOL_DATA3_LED_REQUEST);
			break;
		case P1_CMD_TO_SBC_BMS_EOL_CVTN_ID_REQUEST:
			length = sizeof(S_P1_RCV_CMD_BMS_EOL_CVTN_ID_REQUEST);
			break;
		case P1_CMD_TO_SBC_BMS_EOL_DTC_CLEAR:
			length = sizeof(S_P1_RCV_CMD_BMS_EOL_DTC_CLEAR);
			break;
		case P1_CMD_TO_SBC_BMS_EOL_RESET_MICRO:
			length = sizeof(S_P1_RCV_CMD_BMS_EOL_RESET_MICRO);
			break;
		case P1_CMD_TO_SBC_BMS_EOL_3P_SYSTEM_REQUEST: //kjg_120619
			length = sizeof(S_P1_RCV_CMD_BMS_EOL_3P_SYSTEM_REQUEST);
			break;
		case P1_CMD_TO_SBC_DAQ_ISOLATION_REQUEST:
			length = sizeof(S_P1_RCV_CMD_DAQ_ISOLATION_REQUEST);
			break;
		case P1_CMD_TO_SBC_DAQ_ISOLATION_REQUEST2: //jhkw_150224
			length = sizeof(S_P1_RCV_CMD_DAQ_ISOLATION_REQUEST2);
			break;
		case P1_CMD_TO_SBC_OUT_MUX_SELECT_REQUEST: //kjhw_151021
			length = sizeof(S_P1_RCV_CMD_OUT_MUX_SELECT_REQUEST);
			break;
		case P1_CMD_TO_SBC_TH_TABLE_SET: //jhkw_191108
			length = sizeof(S_P1_RCV_CMD_TH_TABLE_SET);
			break;
		case P1_CMD_TO_SBC_EOL_PROCEDURE_REQUEST: //kjg_120709
			length = sizeof(S_P1_RCV_CMD_EOL_PROCEDURE_REQUEST);
			break;
		case P1_CMD_TO_SBC_BMS_PROCEDURE_REQUEST: //kjg_161207
			length = sizeof(S_P1_RCV_CMD_BMS_PROCEDURE_REQUEST);
			break;
		case P1_CMD_TO_SBC_CALIMODE: //jhkw_130119
			length = sizeof(S_P1_RCV_CMD_CALIMODE);
			break;
		case P1_CMD_TO_SBC_CALI_STOP:	//jhkw_200317s
			length = sizeof(S_P1_RCV_CMD_CALI_STOP);
			break;
		case P1_CMD_TO_SBC_AUTOCALI_REQUEST:
			length = sizeof(S_P1_RCV_CMD_AUTOCALI_REQUEST);
			break;
		case P1_CMD_TO_SBC_AUTOCALI_TEMP_REQUEST:
			length = sizeof(S_P1_RCV_CMD_AUTOCALI_TEMP_REQUEST);
			break; //jhkw_200317e
		case P1_CMD_TO_SBC_LOAD_TYPE_SET: //kjhw_130129
			length = sizeof(S_P1_RCV_CMD_LOAD_TYPE_SET);
			break;
		case P1_CMD_TO_SBC_CH_IO_SET:
			length = sizeof(S_P1_RCV_CMD_CH_IO_SET);
			break;
		case P1_CMD_TO_SBC_BUZZER_STOP:
			length = sizeof(S_P1_RCV_CMD_BUZZER_STOP);
			break;
		case P1_CMD_TO_SBC_ALARM_RESET:
			length = sizeof(S_P1_RCV_CMD_ALARM_RESET);
			break;
		case P1_CMD_TO_SBC_GOTO_STEP: //kjhw_151211
			length = sizeof(S_P1_RCV_CMD_GOTO_STEP);
			break;
		default:
			send_cmd_unknown(header.cmd_id, P1_CMD_ID_ERROR);
			userlog(DEBUG_LOG, psName, "RcvCmd command id error %x\n",
				header.cmd_id);
			return -1;
	}
	
	if(header.cmd_id == P1_CMD_TO_SBC_TESTCOND_PATTERN) {
		if(length >= myPs->rcvCmd.cmdSize) {
			send_cmd_unknown(header.cmd_id, P1_SIZE_ERROR);
			userlog(DEBUG_LOG, psName,
				"RcvCmd(0x%x) size error1 length:%d rcv:%d\n",
				header.cmd_id, length, myPs->rcvCmd.cmdSize);
			return -2;
		}
	} else {
		if(length != myPs->rcvCmd.cmdSize) {
			send_cmd_unknown(header.cmd_id, P1_SIZE_ERROR);
			userlog(DEBUG_LOG, psName,
				"RcvCmd(0x%x) size error2 length:%d rcv:%d\n",
				header.cmd_id, length, myPs->rcvCmd.cmdSize);
			return -2;
		}
	}

	if(header.cmd_serial > MAX_P1_CMD_SERIAL) {
		send_cmd_unknown(header.cmd_id, P1_CD_SEQ_NO_ERROR); //kjg_w
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
	S_P1_CMD_HEADER header;

	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_P1_CMD_HEADER));
	
	rtn = 0;
	/*memset(buf, 0, sizeof buf);
	strncpy(buf, (char *)&header.seqno, sizeof(S_P1_CMD_HEADER));
	seqno = atoi(buf); kjg_w*/
	
	if(myPs->reply.timer_run == P1) {
		/*if(myPs->reply.retry.seqno == seqno
			&& kjg_w*/
		if(myPs->reply.retry.replyCmd == header.cmd_id) {
			myPs->reply.timer_run = P0;
			memset((char *)&myPs->reply.retry, 0, sizeof(S_P1_RETRY_DATA));
		}
	}
	return rtn;
}

int rcv_cmd_module_info_request(void)
{
	int rtn;

	userlog(DEBUG_LOG, psName, "rcv_cmd_module_info_request\n");

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	if(myPs->misc.network_port_type == 2) {
		rtn = send_cmd_module_info_reply(2);
	}

	return send_cmd_module_info_reply(1);
}

int rcv_cmd_module_set_data(void)
{
	//int rtn;
	S_P1_RCV_CMD_MODULE_SET_DATA cmd;

	userlog(DEBUG_LOG, psName, "rcv_cmd_module_set_data\n");

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_MODULE_SET_DATA));
	
	if(cmd.connection_retry != 0) {
		myPs->signal[P1_SIG_NO_CONNECTION_RETRY] = P1;
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
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	/*rtn = Write_COA_Client_Config();
	if(rtn < 0) {
		rtn = send_cmd_response((char *)&cmd.header, P1_CD_FILE_WRITE_ERROR);
		return 0;
	}*/
	
	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_testcond_start(void)
{
	S_P1_RCV_CMD_TESTCOND_START	cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_TESTCOND_START));
	
	if(cmd.testCondHeader.totalStep > MAX_P1_STEP) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_STEP_COUNT_ERROR); //kjg_w
	}

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#elif __COA_VER_100D__
	//kjg_130430
	if(cmd.testCondHeader.totalPatternCount > MAX_P1_STEP) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_PATTERN_COUNT_ERROR); //kjg_w
	}
#else //COA_VER_100F~
	//kjh_160418
	if(cmd.testCondHeader.totalPatternCount > MAX_P1_STEP) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_PATTERN_COUNT_ERROR); //kjg_w
	}
	if(cmd.testCondHeader.totalTimeSchCount > MAX_P1_STEP) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_TIMESCHEDULE_COUNT_ERROR);
	}
#endif

	memcpy((char *)&myPs->testCond.header,
		(char *)&cmd.testCondHeader, sizeof(S_P1_TEST_COND_HEADER));

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
	myPs->signal[P1_SIG_TEST_STEP_RCV] = P0;
	myPs->signal[P1_SIG_TEST_STEP_PATTERN_RCV] = P0;
	myPs->signal[P1_SIG_TEST_HEADER_RCV] = P1;
#elif __COA_VER_100D__
	myPs->misc.rcv_test_step_count = 0;
	myPs->misc.rcv_test_pattern_count = 0;
	myPs->signal[P1_SIG_TEST_HEADER_RCV] = P1;
#else //COA_VER_100F~ //kjh_160418
	myPs->misc.rcv_test_step_count = 0;
	myPs->misc.rcv_test_pattern_count = 0;
	myPs->misc.rcv_test_timesch_count = 0;
	myPs->signal[P1_SIG_TEST_HEADER_RCV] = P1;
	myPs->signal[P1_SIG_TEST_HEADER_UPDATE_RCV] = P0;			//jhkw_200816
#endif
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
	userlog(DEBUG_LOG, psName, "rcv_cmd_testcond_start %d\n",
		cmd.testCondHeader.totalStep);
#elif __COA_VER_100D__
	myPs->misc.rcv_test_pattern_count = cmd.testCondHeader.totalPatternCount;

	userlog(DEBUG_LOG, psName, "rcv_cmd_testcond_start %d, %d\n",
		cmd.testCondHeader.totalStep, cmd.testCondHeader.totalPatternCount);
#else //COA_VER_100F~
	myPs->misc.rcv_test_pattern_count = cmd.testCondHeader.totalPatternCount;
	myPs->misc.rcv_test_timesch_count = cmd.testCondHeader.totalTimeSchCount;

	userlog(DEBUG_LOG, psName, "rcv_cmd_testcond_start %d, %d, %d\n",
		cmd.testCondHeader.totalStep, cmd.testCondHeader.totalPatternCount,
		cmd.testCondHeader.totalTimeSchCount);
#endif

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_testcond_safety(void)
{
	S_P1_RCV_CMD_TESTCOND_SAFETY cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_TESTCOND_SAFETY));

	if(myPs->signal[P1_SIG_TEST_HEADER_RCV] != P1) {
		return send_cmd_response((char *)&cmd.header, P1_CD_TEST_HEADER_UNRCV);
	}
	
	memcpy((char *)&myPs->testCond.safety, (char *)&cmd.safety,
		sizeof(S_P1_TEST_COND_SAFETY));

	userlog(DEBUG_LOG, psName,
		"rcv_cmd_testcond_safety lowerV:%ld, upperV:%ld\n",
		cmd.safety.faultLowerV, cmd.safety.faultUpperV);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_testcond_step(void)
{
	int step;
	S_P1_RCV_CMD_TESTCOND_STEP cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_TESTCOND_STEP));
	
	if(myPs->signal[P1_SIG_TEST_HEADER_RCV] != P1) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_HEADER_UNRCV);
	}

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
	step = (int)myPs->signal[P1_SIG_TEST_STEP_RCV];
#else //COA_VER_100D~
	step = (int)myPs->misc.rcv_test_step_count;
#endif

	if((step+1) > (int)myPs->testCond.header.totalStep || step < 0) {
		userlog(DEBUG_LOG, psName, "test_step_error %d %d\n", step+1,
			(int)myPs->testCond.header.totalStep);
		return send_cmd_response((char *)&cmd.header, P1_CD_TEST_STEP_ERROR);
	}

	memcpy((char *)&myPs->testCond.step[step],
		(char *)&cmd.testCondStep, sizeof(S_P1_TEST_COND_STEP));

#ifdef __COA_VER_100B__
	myPs->signal[P1_SIG_TEST_STEP_RCV]++;
#elif __COA_VER_100B2__
	myPs->signal[P1_SIG_TEST_STEP_RCV]++;
#elif __COA_VER_100C__
	myPs->signal[P1_SIG_TEST_STEP_RCV]++;
#else //COA_VER_100D~
	myPs->misc.rcv_test_step_count++;
#endif

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	userlog(DEBUG_LOG, psName, "rcv_cmd_testcond_step %d\n",
		myPs->misc.rcv_test_step_count);
#endif

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
int rcv_cmd_testcond_pattern(void)
{
	int ch, step, rtn, i, j, point, pattern_size, pattern_index;
	unsigned long chFlag, chFlag1;
	S_P1_RCV_CMD_TESTCOND_PATTERN cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_TESTCOND_PATTERN));

	point = sizeof(S_P1_CMD_HEADER) + sizeof(S_P1_TEST_COND_PATTERN);
	pattern_size = (int)cmd.header.body_size
		- sizeof(S_P1_TEST_COND_PATTERN);
	if(pattern_size <= 0) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_SIZE_MISMATCH); //kjg_w
	}
	
	if(myPs->signal[P1_SIG_TEST_HEADER_RCV] != P1) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_HEADER_UNRCV); //kjg_w
	}
	
	step = (int)myPs->signal[P1_SIG_TEST_STEP_RCV];
	if((step+1) > (int)myPs->testCond.header.totalStep || step < 0) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_STEP_ERROR); //kjg_w
	}
	
	pattern_index = (int)myPs->signal[P1_SIG_TEST_STEP_PATTERN_RCV];

	memcpy((char *)&myPs->testCond.step[step].header,
		(char *)&cmd.testCondPattern.header, sizeof(S_P1_TEST_STEP_HEADER));
	myPs->testCond.step[step].header.patternIndex
		= (unsigned char)pattern_index;
	memcpy((char *)&myPs->testCond.step[step].reference,
		(char *)&cmd.testCondPattern.reference,
		sizeof(S_P1_TEST_STEP_REFERENCE) * MAX_P1_SUB_STEP);
	memcpy((char *)&myPs->testCond.step[step].record,
		(char *)&cmd.testCondPattern.record, sizeof(S_P1_TEST_RECORD_COND));
	myPs->testCond.step[step].faultUpperV = cmd.testCondPattern.faultUpperV;
	myPs->testCond.step[step].faultLowerV = cmd.testCondPattern.faultLowerV;
	myPs->testCond.step[step].faultUpperI = cmd.testCondPattern.faultUpperI;
	myPs->testCond.step[step].faultLowerI = cmd.testCondPattern.faultLowerI;
	myPs->testCond.step[step].faultUpper_AmpareHour
		= cmd.testCondPattern.faultUpper_AmpareHour;
	myPs->testCond.step[step].faultLower_AmpareHour
		= cmd.testCondPattern.faultLower_AmpareHour;
	myPs->testCond.step[step].faultUpperZ = cmd.testCondPattern.faultUpperZ;
	myPs->testCond.step[step].faultLowerZ = cmd.testCondPattern.faultLowerZ;
	myPs->testCond.step[step].faultUpperTemp
		= cmd.testCondPattern.faultUpperTemp;
	myPs->testCond.step[step].faultLowerTemp
		= cmd.testCondPattern.faultLowerTemp;

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			rtn = Write_Pattern_File_1(psName, ch, pattern_index,
				(char *)&myPs->rcvCmd.cmd + point);
			//kjg_w break;
		}
	}
	
	myPs->signal[P1_SIG_TEST_STEP_RCV]++;
	myPs->signal[P1_SIG_TEST_STEP_PATTERN_RCV]++;

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	rtn = send_cmd_response((char *)&cmd.header, P1_CD_ACK);

	return rtn;
}
#else //COA_VER_100D~
int rcv_cmd_testcond_pattern(void)
{
	int ch, step, rtn, i, j, pattern_size, pattern_index;
	unsigned long chFlag, chFlag1;
	S_P1_RCV_CMD_TESTCOND_PATTERN cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_TESTCOND_PATTERN));

	pattern_size = (int)cmd.header.body_size
		- sizeof(S_P1_TEST_COND_PATTERN);
	if(pattern_size <= 0) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_SIZE_MISMATCH); //kjg_w
	}
	
	if(myPs->signal[P1_SIG_TEST_HEADER_RCV] != P1) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_HEADER_UNRCV); //kjg_w
	}
	
	step = (int)cmd.testCondPattern.stepNo;
	if((step+1) > (int)myPs->testCond.header.totalStep || step < 0) {
		userlog(DEBUG_LOG, psName, "test_step_error %d %d\n", step+1,
			(int)myPs->testCond.header.totalStep);
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_STEP_ERROR); //kjg_w
	}
	
	pattern_index = step;

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
	
	myPs->misc.rcv_test_pattern_count++;

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	userlog(DEBUG_LOG, psName,
		"rcv_cmd_testcond_pattern %d, pattern_size %d, body_size %ld, pattern_length %ld\n",
		myPs->misc.rcv_test_pattern_count, pattern_size,
		cmd.header.body_size, cmd.testCondPattern.length);

	rtn = send_cmd_response((char *)&cmd.header, P1_CD_ACK);

	return rtn;
}
#endif

int rcv_cmd_testcond_end(void)
{ //kjg_170810
	int ch, i, j, rtn;
	unsigned long chFlag, chFlag1;
	S_P1_RCV_CMD_TESTCOND_END cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_TESTCOND_END));

	if(myPs->signal[P1_SIG_TEST_HEADER_RCV] != P1) {
		userlog(DEBUG_LOG, psName, "test_header_error %d\n",
			(int)myPs->signal[P1_SIG_TEST_HEADER_RCV]);
		return send_cmd_response((char *)&cmd.header, P1_CD_TEST_HEADER_UNRCV);
	}
	
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
	if(myPs->signal[P1_SIG_TEST_STEP_RCV] == P0) {
		userlog(DEBUG_LOG, psName, "test_step_rcv_error %d\n",
			(int)myPs->signal[P1_SIG_TEST_STEP_RCV]);
		return send_cmd_response((char *)&cmd.header, P1_CD_TEST_STEP_UNRCV);
	}
	
	if((int)myPs->testCond.header.totalStep
		!= (int)myPs->signal[P1_SIG_TEST_STEP_RCV]) {
		userlog(DEBUG_LOG, psName, "test_step_rcv_error %d\n",
			(int)myPs->signal[P1_SIG_TEST_STEP_RCV]);
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_STEP_COUNT_ERROR); //kjg_w
	}
	
	myPs->signal[P1_SIG_TEST_HEADER_RCV] = P0;
	myPs->signal[P1_SIG_TEST_STEP_RCV] = P0;
	myPs->signal[P1_SIG_TEST_STEP_PATTERN_RCV] = P0;
#else //COA_VER_100D~
#endif

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#elif __COA_VER_100D__
	myPs->testCond.test_cond_file_size = cmd.test_cond_file_size;
	myPs->testCond.test_cond_file_checksum = cmd.test_cond_file_checksum;

	if((int)myPs->testCond.header.totalPatternCount
		!= (int)myPs->misc.rcv_test_pattern_count) {
		userlog(DEBUG_LOG, psName, "test_pattern_rcv_error %d\n",
			(int)myPs->misc.rcv_test_pattern_count);
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_PATTERN_COUNT_ERROR); //kjg_w
	}
#else //COA_VER_100F~ //kjh_160418
	myPs->testCond.test_cond_file_size = cmd.test_cond_file_size;
	myPs->testCond.test_cond_file_checksum = cmd.test_cond_file_checksum;

	if((int)myPs->testCond.header.totalPatternCount
		!= (int)myPs->misc.rcv_test_pattern_count) {
		userlog(DEBUG_LOG, psName, "test_pattern_rcv_error %d\n",
			(int)myPs->misc.rcv_test_pattern_count);
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_PATTERN_COUNT_ERROR); //kjg_w
	}

	myPs->testCond_time.test_cond_time_file_size
		= cmd.test_cond_time_file_size;
	myPs->testCond_time.test_cond_time_file_checksum
		= cmd.test_cond_time_file_checksum;

	if((int)myPs->testCond.header.totalTimeSchCount
		!= (int)myPs->misc.rcv_test_timesch_count) {
		userlog(DEBUG_LOG, psName, "test_timesch_rcv_error %d\n",
			(int)myPs->misc.rcv_test_timesch_count);
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_TIMESCHEDULE_COUNT_ERROR);
	}
#endif
	
	chFlag = 0x01; rtn = 0;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		userlog(DEBUG_LOG, psName, "test_cond_conversion start %d, %d\n",
			ch + 1, rtn);

		//myTestCond Assign kjg_170810
		//kjg_171219 myTestCond = (S_TEST_CONDITION *)&(myData->testCond[ch]);
		myTestCond = &(myData->testCond[ch]);

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
		rtn = convert_test_cond(ch);
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName,
				"convert_test_cond error %d, %d\n", ch, rtn);
			return send_cmd_response((char *)&cmd.header,
				P1_CD_TEST_STEP_ERROR); //kjg_w
		}

		userlog(DEBUG_LOG, psName, "test_cond_conversion end %d, %d\n",
			ch + 1, rtn);
#else //COA_VER_100D~
		rtn = read_test_cond_step_file(ch, 0);
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName,
				"read_test_cond_step_file error %d, %d\n", ch, rtn);
			break;
		}

		rtn = convert_test_cond(ch);
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName, "convert_test_cond error %d, %d\n",
				ch, rtn);
			break;
		}

		userlog(DEBUG_LOG, psName, "test_cond_conversion end %d, %d\n",
			ch + 1, rtn);

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~ //kjh_160418
		if(myPs->testCond.header.totalTimeSchCount > 0) {
			userlog(DEBUG_LOG, psName,
				"test_cond_conversion_time start %d, %d\n", ch + 1, rtn);

			rtn = read_test_cond_time_step_file(ch, 0);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"read_test_cond_time_step_file error %d, %d\n", ch, rtn);
				break;
			}
	
			rtn = convert_test_cond_time(ch);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"convert_test_cond_time error %d, %d\n", ch, rtn);
				break;
			}

			userlog(DEBUG_LOG, psName, "test_cond_time_conversion end %d, %d\n",
				ch + 1, rtn);
		} else {
			//kjh_161109
			memset((char *)&myData->TimeSch.testCond[ch], 0,
				sizeof(S_TIMESCH_TEST_CONDITION));
		}
#endif
#endif

		myData->ChAttribute[ch].chamber_control = 0;
		myData->ChAttribute[ch].can_comm_check = 0; //kjhw_141201
	}

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
	rtn = send_cmd_response((char *)&cmd.header, P1_CD_ACK);
#else //COA_VER_100D~
	if(rtn < 0) {
		rtn = send_cmd_response((char *)&cmd.header, P1_CD_ACK);
		send_cmd_test_cond_conversion_end(0); //nack

		chFlag = 0x01;
		for(i=0; i < myPs->misc.chInGroup; i++) {
			j = i / 32;
			chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
			if(chFlag1 == 0) continue;

			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

			myPs->signal[P1_SIG_TEST_CONVERSION_END_1 + ch] = P0;
		}
	} else {
		rtn = send_cmd_response((char *)&cmd.header, P1_CD_ACK);
		send_cmd_test_cond_conversion_end(1); //ack

		chFlag = 0x01;
		for(i=0; i < myPs->misc.chInGroup; i++) {
			j = i / 32;
			chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
			if(chFlag1 == 0) continue;

			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

			myPs->signal[P1_SIG_TEST_CONVERSION_END_1 + ch] = P1;
		}
	}

	userlog(DEBUG_LOG, psName, "test_cond_conversion chFlag %x %x\n",
		cmd.header.chFlag[0], cmd.header.chFlag[1]);
#endif

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#elif __COA_VER_100D__
	userlog(DEBUG_LOG, psName, "rcv_cmd_testcond_end %d %d\n",
		myPs->misc.rcv_test_step_count, myPs->misc.rcv_test_pattern_count);

	myPs->signal[P1_SIG_TEST_HEADER_RCV]++;
	myPs->misc.rcv_test_step_count = 0;
	myPs->misc.rcv_test_pattern_count = 0;
#else //COA_VER_100F~
	userlog(DEBUG_LOG, psName, "rcv_cmd_testcond_end %d %d %d\n",
		myPs->misc.rcv_test_step_count, myPs->misc.rcv_test_pattern_count,
		myPs->misc.rcv_test_timesch_count);

	myPs->signal[P1_SIG_TEST_HEADER_RCV]++;
	myPs->misc.rcv_test_step_count = 0;
	myPs->misc.rcv_test_pattern_count = 0;
	myPs->misc.rcv_test_timesch_count = 0;
#endif

	return rtn;
}

int rcv_cmd_step_cond_request(void)
{ //kjg_170810
	int i, j, ch, stepNo;
	unsigned long chFlag, chFlag1;
	S_P1_RCV_CMD_STEP_COND_REQUEST cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_STEP_COND_REQUEST));

	chFlag = 0x01; ch = (-1);
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

			if(myData->cData[ch].op.state == C_RUN
				|| myData->cData[ch].op.state == C_PAUSE) {
				break;
			} else {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
			}
		}
	}

	if(ch == (-1)) {
		return send_cmd_response((char *)&cmd.header, P1_CD_CH_ID_ERROR);
	}

	stepNo = (int)cmd.stepNo;

	if(stepNo >= myData->testCond[ch].common_object[IDX_COM_OBJ_TOTAL_STEP]) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_STEP_COUNT_ERROR);
	}

	userlog(DEBUG_LOG, psName, "rcv_cmd_step_cond_request ch:%d, stepNo:%d\n",
		ch + 1, stepNo + 1);

	return send_cmd_step_cond_reply((char *)&cmd.header, ch, stepNo);
}

int rcv_cmd_step_cond_update(void)
{ //kjg_170810
	int i, j, ch, rtn, toPs ,group;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&myPs->step_cond_update, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_STEP_COND_UPDATE));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32))
			& myPs->step_cond_update.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		if(myData->ChAttribute[ch].opType != OP_INDEPENDENT
			&& myData->ChAttribute[ch].chNo_master == 0) {
			return send_cmd_response((char *)&myPs->step_cond_update.header,
				P1_CD_CH_ID_ERROR);
		}

		if(myData->cData[ch].op.state != C_RUN
			&& myData->cData[ch].op.state != C_PAUSE) {
			return send_cmd_response((char *)&myPs->step_cond_update.header,
				P1_CD_CH_STATE_ERROR);
		}
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32))
			& myPs->step_cond_update.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));

		rtn = convert_test_cond_step_update(ch);
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName,
				"convert_test_cond_step_update error %d, %d, %d\n",
				ch+1, myPs->step_cond_update.stepNo+1, rtn);
			return send_cmd_response(
				(char *)&myPs->step_cond_update.header,
				P1_CD_TEST_STEP_ERROR); //kjg_w
		}
	}

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CMD_TEST_COND_UPDATE;
	SendMsg.val[0] = group;
	SendMsg.val[1] = 1; //step_cond_update
	SendMsg.val[2] = myPs->step_cond_update.stepNo;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName,
		"rcv_cmd_step_cond_step_update %x %x %x %x, %x %x %x %x, %d %d %d\n",
		ch_flag.flag[0], ch_flag.flag[1], ch_flag.flag[2], ch_flag.flag[3],
		ch_flag.flag[4], ch_flag.flag[5], ch_flag.flag[6], ch_flag.flag[7],
		group, SendMsg.val[1], myPs->step_cond_update.stepNo+1);

	return send_cmd_response((char *)&myPs->step_cond_update.header, P1_CD_ACK);
}

int rcv_cmd_common_safety_cond_request(void)
{ //kjg_170810
	int i, j, ch;
	unsigned long chFlag, chFlag1;
	S_P1_RCV_CMD_COMMON_SAFETY_COND_REQUEST cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_COMMON_SAFETY_COND_REQUEST));

	chFlag = 0x01; ch = (-1);
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

			if(myData->cData[ch].op.state == C_RUN
				|| myData->cData[ch].op.state == C_PAUSE) {
				break;
			} else {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
			}
		}
	}

	if(ch == (-1)) {
		return send_cmd_response((char *)&cmd.header, P1_CD_CH_ID_ERROR);
	}

	userlog(DEBUG_LOG, psName, "rcv_cmd_common_safety_cond_request ch:%d\n",
		ch+1);

	return send_cmd_common_safety_cond_reply((char *)&cmd.header, ch);
}

int rcv_cmd_common_safety_cond_update(void)
{ //kjg_170810
	int i, j, ch, rtn, toPs, group;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&myPs->common_safety_cond_update, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_COMMON_SAFETY_COND_UPDATE));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32))
			& myPs->common_safety_cond_update.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		if(myData->ChAttribute[ch].opType != OP_INDEPENDENT
			&& myData->ChAttribute[ch].chNo_master == 0) {
			return send_cmd_response((char *)&myPs->common_safety_cond_update
				.header, P1_CD_CH_ID_ERROR);
		}

		if(myData->cData[ch].op.state != C_RUN
			&& myData->cData[ch].op.state != C_PAUSE) {
			return send_cmd_response((char *)&myPs->common_safety_cond_update
				.header, P1_CD_CH_STATE_ERROR);
		}
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32))
			& myPs->common_safety_cond_update.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));

		rtn = convert_test_cond_common_safety_update(ch);
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName,
				"convert_test_cond_common_safety_update error %d, %d\n",
				ch+1, rtn);
			return send_cmd_response(
				(char *)&myPs->common_safety_cond_update.header,
				P1_CD_TEST_COMMON_SAFETY_ERROR); //kjg_w
		}
	}

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CMD_TEST_COND_UPDATE;
	SendMsg.val[0] = group;
	SendMsg.val[1] = 0; //common_safety_cond_update
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName,
		"rcv_cmd_common_safety_cond_update %x %x %x %x, %x %x %x %x, %d %d\n",
		ch_flag.flag[0], ch_flag.flag[1], ch_flag.flag[2], ch_flag.flag[3],
		ch_flag.flag[4], ch_flag.flag[5], ch_flag.flag[6], ch_flag.flag[7],
		group, SendMsg.val[1]);

	return send_cmd_response((char *)&myPs->common_safety_cond_update.header,
		P1_CD_ACK);
}

int rcv_cmd_reset_reserved_cmd(void)
{
	int ch=0, i, installedCh;
	unsigned long chFlag, chFlag1;
	S_P1_RCV_CMD_RESET_RESERVED_CMD	cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_RESET_RESERVED_CMD));

	installedCh = myData->mData.config.installedCh;
	if(installedCh > P1_MAX_CH_PER_MODULE) installedCh = P1_MAX_CH_PER_MODULE;
	
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
	
	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_testcond_update_start(void)
{ //kjg_170810
	S_P1_RCV_CMD_TESTCOND_UPDATE_START cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_TESTCOND_UPDATE_START));
	
	if(cmd.testCondHeader.totalStep > MAX_P1_STEP) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_STEP_COUNT_ERROR); //kjg_w
	}

	if(cmd.testCondHeader.totalPatternCount > MAX_P1_STEP) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_PATTERN_COUNT_ERROR); //kjg_w
	}
	if(cmd.testCondHeader.totalTimeSchCount > MAX_P1_STEP) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_TIMESCHEDULE_COUNT_ERROR);
	}

	memcpy((char *)&myPs->testCond.header,
		(char *)&cmd.testCondHeader, sizeof(S_P1_TEST_COND_HEADER));

	myPs->misc.rcv_test_step_count = 0;
	myPs->misc.rcv_test_pattern_count = 0;
	myPs->misc.rcv_test_timesch_count = 0;
	myPs->signal[P1_SIG_TEST_HEADER_UPDATE_RCV] = P1;
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	myPs->misc.rcv_test_pattern_count = cmd.testCondHeader.totalPatternCount;
	myPs->misc.rcv_test_timesch_count = cmd.testCondHeader.totalTimeSchCount;

	userlog(DEBUG_LOG, psName, "rcv_cmd_testcond_update_start %d, %d, %d\n",
		cmd.testCondHeader.totalStep, cmd.testCondHeader.totalPatternCount,
		cmd.testCondHeader.totalTimeSchCount);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_testcond_update_end(void)
{ //kjg_170810
	int ch, i, j, rtn, toPs, group;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_TESTCOND_END cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_TESTCOND_END));

	if(myPs->signal[P1_SIG_TEST_HEADER_UPDATE_RCV] != P1) {
		userlog(DEBUG_LOG, psName, "test_header_update_error %d\n",
			(int)myPs->signal[P1_SIG_TEST_HEADER_UPDATE_RCV]);
		return send_cmd_response((char *)&cmd.header, P1_CD_TEST_HEADER_UNRCV);
	}
	
	myPs->testCond.test_cond_file_size = cmd.test_cond_file_size;
	myPs->testCond.test_cond_file_checksum = cmd.test_cond_file_checksum;

	if((int)myPs->testCond.header.totalPatternCount
		!= (int)myPs->misc.rcv_test_pattern_count) {
		userlog(DEBUG_LOG, psName, "test_pattern_update_rcv_error %d\n",
			(int)myPs->misc.rcv_test_pattern_count);
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_PATTERN_COUNT_ERROR); //kjg_w
	}

	myPs->testCond_time.test_cond_time_file_size
		= cmd.test_cond_time_file_size;
	myPs->testCond_time.test_cond_time_file_checksum
		= cmd.test_cond_time_file_checksum;

	if((int)myPs->testCond.header.totalTimeSchCount
		!= (int)myPs->misc.rcv_test_timesch_count) {
		userlog(DEBUG_LOG, psName, "test_timesch_update_rcv_error %d\n",
			(int)myPs->misc.rcv_test_timesch_count);
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_TIMESCHEDULE_COUNT_ERROR);
	}
	
	chFlag = 0x01; rtn = 0;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		if(myData->ChAttribute[ch].opType != OP_INDEPENDENT
			&& myData->ChAttribute[ch].chNo_master == 0) {
			return send_cmd_response((char *)&cmd.header, P1_CD_CH_ID_ERROR);
		}

		userlog(DEBUG_LOG, psName, "test_cond_conversion_update start %d, %d\n",
			ch+1, rtn);

		//myTestCond Assign kjg_170810
		//kjg_171219 myTestCond = (S_TEST_CONDITION *)&(myData->testCond_update);
		myTestCond = &(myData->testCond_update);

		rtn = read_test_cond_step_file(ch, 1);
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName,
				"read_test_cond_step_file(update) error %d, %d\n", ch, rtn);
			break;
		}

		rtn = convert_test_cond(ch);
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName,
				"convert_test_cond(update) error %d, %d\n", ch, rtn);
			break;
		}

		userlog(DEBUG_LOG, psName, "test_cond_conversion(update) end %d, %d\n",
			ch+1, rtn);

		if(myPs->testCond.header.totalTimeSchCount > 0) {
			userlog(DEBUG_LOG, psName,
				"test_cond_conversion_time(update) start %d, %d\n", ch+1, rtn);

			rtn = read_test_cond_time_step_file(ch, 1);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"read_test_cond_time_step_file(update) error %d, %d\n",
					ch, rtn);
				break;
			}

			rtn = convert_test_cond_time(ch);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"convert_test_cond_time(update) error %d, %d\n", ch, rtn);
				break;
			}

			userlog(DEBUG_LOG, psName,
				"test_cond_time_conversion(update) end %d, %d\n", ch+1, rtn);
		} else {
			memset((char *)&myData->TimeSch.testCond[ch], 0,
				sizeof(S_TIMESCH_TEST_CONDITION)); //kjh_161109
		}

		myData->ChAttribute[ch].chamber_control = 0;
		myData->ChAttribute[ch].can_comm_check = 0; //kjhw_141201
	}

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	if(rtn < 0) {
		rtn = send_cmd_response((char *)&cmd.header, P1_CD_ACK);
		send_cmd_test_cond_conversion_update_end(0); //nack

		chFlag = 0x01;
		for(i=0; i < myPs->misc.chInGroup; i++) {
			j = i / 32;
			chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
			if(chFlag1 == 0) continue;

			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

			myPs->signal[P1_SIG_TEST_CONVERSION_UPDATE_END_1 + ch] = P0;
		}
	} else {
		rtn = send_cmd_response((char *)&cmd.header, P1_CD_ACK);
		send_cmd_test_cond_conversion_update_end(1); //ack

		chFlag = 0x01;
		for(i=0; i < myPs->misc.chInGroup; i++) {
			j = i / 32;
			chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
			if(chFlag1 == 0) continue;

			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

			j = i / 8;
			ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));

			myPs->signal[P1_SIG_TEST_CONVERSION_UPDATE_END_1 + ch] = P1;
		}

		myPs->signal[P1_SIG_TEST_HEADER_UPDATE_RCV] = P0;
		myPs->misc.rcv_test_step_count = 0;
		myPs->misc.rcv_test_pattern_count = 0;
		myPs->misc.rcv_test_timesch_count = 0;

		group = myPs->config.groupNo;
		toPs = COA1_TO_MODULE + myPs->config.groupNo;
		send_msg_ch_flag(toPs, (char *)&ch_flag);
		SendMsg.msg = MSG_COA_MODULE_CMD_TEST_COND_UPDATE;
		SendMsg.val[0] = group;
		SendMsg.val[1] = 2; //test_cond_update
		send_msg(toPs, (char *)&SendMsg);

		userlog(DEBUG_LOG, psName,
		"test_cond_conversion(update) chFlag %x %x %x %x, %x %x %x %x, %d %d\n",
			ch_flag.flag[0], ch_flag.flag[1], ch_flag.flag[2], ch_flag.flag[3],
			ch_flag.flag[4], ch_flag.flag[5], ch_flag.flag[6], ch_flag.flag[7],
			group, SendMsg.val[1]);
	}

	userlog(DEBUG_LOG, psName, "rcv_cmd_testcond_update end %d %d %d\n",
		myPs->misc.rcv_test_step_count, myPs->misc.rcv_test_pattern_count,
		myPs->misc.rcv_test_timesch_count);

	return rtn;
}

int rcv_cmd_testcond_pattern_update(void)
{ //kjg_170810
	int ch, step, rtn, i, j, pattern_size, pattern_index;
	unsigned long chFlag, chFlag1;
	S_P1_RCV_CMD_TESTCOND_PATTERN cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_TESTCOND_PATTERN));

	pattern_size = (int)cmd.header.body_size - sizeof(S_P1_TEST_COND_PATTERN);
	if(pattern_size <= 0) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_SIZE_MISMATCH); //kjg_w
	}
	
	if(myPs->signal[P1_SIG_TEST_HEADER_UPDATE_RCV] != P1) {
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_HEADER_UNRCV); //kjg_w
	}
	
	step = (int)cmd.testCondPattern.stepNo;
	if((step+1) > (int)myPs->testCond.header.totalStep || step < 0) {
		userlog(DEBUG_LOG, psName, "test_pattern_update_error %d %d\n", step+1,
			(int)myPs->testCond.header.totalStep);
		return send_cmd_response((char *)&cmd.header,
			P1_CD_TEST_STEP_ERROR); //kjg_w
	}
	
	pattern_index = step;

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
	
	myPs->misc.rcv_test_pattern_count++;

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	userlog(DEBUG_LOG, psName,
		"rcv_cmd_testcond_pattern_update %d, pattern_size %d, body_size %ld, pattern_length %ld\n",
		myPs->misc.rcv_test_pattern_count, pattern_size,
		cmd.header.body_size, cmd.testCondPattern.length);

	rtn = send_cmd_response((char *)&cmd.header, P1_CD_ACK);

	return rtn;
}

int rcv_cmd_cali_meter_connect(void)
{
	int toPs;
	S_P1_RCV_CMD_CALI_METER_CONNECT	cmd;
	S_MSG_VAL SendMsg;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CALI_METER_CONNECT));

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	toPs = COA1_TO_METER + myPs->config.groupNo;
	SendMsg.msg = MSG_COA_METER_INITIALIZE;
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
	int group, ch, i, j, type, range, mode, toPs;
//#ifdef __B_TYPE__
	int count; //kjhw_120503 Vref x 2
//#endif
	unsigned long chFlag, chFlag1, shunt_value; //jhkw_200317
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CALI_START	cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CALI_START));

	type = cmd.tmpCond.type;
	if(type != CALI_TYPE_VOLTAGE && type != CALI_TYPE_CURRENT) {
		return send_cmd_response((char *)&cmd.header, P1_CD_TYPE_MISMATCH);
	}

	range = cmd.tmpCond.range;
	if(range >= MAX_RANGE || type < 0) {
		return send_cmd_response((char *)&cmd.header, P1_CD_RANGE_MISMATCH);
	}

	mode = cmd.tmpCond.mode;
	//if(mode != CALI_MODE_NORMAL && mode != CALI_MODE_CHECK) {
	if(mode != CALI_MODE_NORMAL && mode != CALI_MODE_CHECK
		&& mode != CALI_MODE_ONLY_CHECK) {	//jhkw_200317
		return send_cmd_response((char *)&cmd.header, P1_CD_MODE_MISMATCH);
	}

//#ifdef __B_TYPE__
	//kjhw_120503s Vref x 2
	count = cmd.tmpCond.count;
	if(count != MAX_VREF_DAC_JUMP && count != 0) {
		return send_cmd_response((char *)&cmd.header, P1_CD_COUNT_MISMATCH);
	}
	//kjhw_120503e
//#endif
	//jhkw_200317s
	shunt_value = cmd.tmpCond.shunt_value;
	if(shunt_value >= MAX_SUM_ULONG || (shunt_value <= 0 
		&& type == CALI_TYPE_CURRENT)) {
		return send_cmd_response((char *)&cmd.header, P1_CD_SHUNT_MISMATCH);
	}
	//jhkw_200317s

	group = myPs->config.groupNo;
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_ISNT_STANDBY);
			}
		}
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
		
		myData->cali[ch].type = type;
		myData->cali[ch].range = range;
		myData->cali[ch].mode = mode;
//#ifdef __B_TYPE__
		myData->cali[ch].count = count; //kjhw_120506 Vref x 2
//#endif
		myData->cali[ch].shunt_value = shunt_value;	//jhkw_200317
		
		memcpy((char *)&myData->cali[ch].tmpCond[type][range],
			(char *)&cmd.tmpCond, sizeof(S_CALI_TMP_COND));

		memset((char *)&myData->cali[ch].tmpData[type][range],
			0, sizeof(S_CALI_TMP_DATA));
		
		switch(myData->AppControl.config.systemModel) {
		 	case C_LGC_50V_40A_10A_4A: //csk_120206
		 	case C_LGC_50V_40A_10A_4A_2:
		 	case C_LGC_50V_40A_10A_4A_3:
		 	case C_LGC_50V_40A_10A_4A_4:
			case C_SDI_70V_50A_5A_4KW:
			case C_SDI_70V_50A_5A_4KW_2:
			case C_SDI_70V_50A_5A_7KW:
			case C_SDI_70V_50A_5A_7KW_2:
			case C_SDI_70V_250A_25A_18KW:
				if(type != 0) break;

				myData->cali[ch+4].type = type;
				myData->cali[ch+4].range = range;
				myData->cali[ch+4].mode = mode;

				memcpy((char *)&myData->cali[ch+4].tmpCond[0][range],
					(char *)&cmd.tmpCond, sizeof(S_CALI_TMP_COND));
				memcpy((char *)&myData->cali[ch+4].tmpCond[1][range],
					(char *)&cmd.tmpCond, sizeof(S_CALI_TMP_COND));

				memset((char *)&myData->cali[ch+4].tmpData[0][range],
					0, sizeof(S_CALI_TMP_DATA));
				memset((char *)&myData->cali[ch+4].tmpData[1][range],
					0, sizeof(S_CALI_TMP_DATA));
				break;
			case C_SBL_150V_250A_10A_38KW: //lki_111010_s
			case C_SBL_150V_250A_10A_75KW_4:
			case C_SBL_150V_250A_10A_75KW_5:
				if(type != 0) break;

				if(ch == 0) {
					myData->cali[ch+4].type = type;
					myData->cali[ch+4].range = range;
					myData->cali[ch+4].mode = mode;
	
					memcpy((char *)&myData->cali[ch+4].tmpCond[0][range],
						(char *)&cmd.tmpCond, sizeof(S_CALI_TMP_COND));
					memcpy((char *)&myData->cali[ch+4].tmpCond[1][range],
						(char *)&cmd.tmpCond, sizeof(S_CALI_TMP_COND));

					memset((char *)&myData->cali[ch+4].tmpData[0][range],
						0, sizeof(S_CALI_TMP_DATA));
					memset((char *)&myData->cali[ch+4].tmpData[1][range],
						0, sizeof(S_CALI_TMP_DATA));

					myData->cali[ch+5].type = type;
					myData->cali[ch+5].range = range;
					myData->cali[ch+5].mode = mode;

					memcpy((char *)&myData->cali[ch+5].tmpCond[0][range],
						(char *)&cmd.tmpCond, sizeof(S_CALI_TMP_COND));
					memcpy((char *)&myData->cali[ch+5].tmpCond[1][range],
						(char *)&cmd.tmpCond, sizeof(S_CALI_TMP_COND));

					memset((char *)&myData->cali[ch+5].tmpData[0][range],
						0, sizeof(S_CALI_TMP_DATA));
					memset((char *)&myData->cali[ch+5].tmpData[1][range],
						0, sizeof(S_CALI_TMP_DATA));
				} else if(ch == 1) {
					myData->cali[ch+5].type = type;
					myData->cali[ch+5].range = range;
					myData->cali[ch+5].mode = mode;

					memcpy((char *)&myData->cali[ch+5].tmpCond[0][range],
						(char *)&cmd.tmpCond, sizeof(S_CALI_TMP_COND));
					memcpy((char *)&myData->cali[ch+5].tmpCond[1][range],
						(char *)&cmd.tmpCond, sizeof(S_CALI_TMP_COND));

					memset((char *)&myData->cali[ch+5].tmpData[0][range],
						0, sizeof(S_CALI_TMP_DATA));
					memset((char *)&myData->cali[ch+5].tmpData[1][range],
						0, sizeof(S_CALI_TMP_DATA));

					myData->cali[ch+6].type = type;
					myData->cali[ch+6].range = range;
					myData->cali[ch+6].mode = mode;

					memcpy((char *)&myData->cali[ch+6].tmpCond[0][range],
						(char *)&cmd.tmpCond, sizeof(S_CALI_TMP_COND));
					memcpy((char *)&myData->cali[ch+6].tmpCond[1][range],
						(char *)&cmd.tmpCond, sizeof(S_CALI_TMP_COND));

					memset((char *)&myData->cali[ch+6].tmpData[0][range],
						0, sizeof(S_CALI_TMP_DATA));
					memset((char *)&myData->cali[ch+6].tmpData[1][range],
						0, sizeof(S_CALI_TMP_DATA));

				}
				break;  //lki_111010_e
			default:
				break;
		}
#ifdef __B_TYPE__
		//kjhw_120506s
		userlog(DEBUG_LOG, psName,
			"cali_start \n ch: %d type: %d range: %d mode: %d count: %d shunt: %ld\n"
			, ch, type, range, mode, count, shunt_value);
		//kjhw_120506e
#endif

		group = myPs->config.groupNo;
		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		toPs = COA1_TO_METER + group;
		SendMsg.msg = MSG_COA_METER_INITIALIZE;
		SendMsg.val[0] = type;
		SendMsg.val[1] = range;
		send_msg(toPs, (char *)&SendMsg);

		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		toPs = COA1_TO_MODULE + group;
		SendMsg.msg = MSG_COA_MODULE_CH_CALI;
		SendMsg.val[0] = ch;
		send_msg(toPs, (char *)&SendMsg);
		break;
	}
	
	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_cali_update(void)
{
	int bd, ch, i, rtn, write_bd[MAX_BD_PER_MODULE], installedCh;
	unsigned long chFlag, chFlag1;
	S_P1_RCV_CMD_CALI_UPDATE cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CALI_UPDATE));

	for(bd=0; bd < MAX_BD_PER_MODULE; bd++) {
		write_bd[bd] = 0;
	}

	installedCh = myData->mData.config.installedCh;
	if(installedCh > P1_MAX_CH_PER_MODULE) installedCh = P1_MAX_CH_PER_MODULE;

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
//#ifdef __B_TYPE__
			if(myData->cali[ch].count == 4) {
				CaliUpdateCh1(ch);
			} else {
				CaliUpdateCh(ch);
			}
//#else
			//CaliUpdateCh(ch); //kjhw_120504e
//#endif
			switch(myData->AppControl.config.systemModel) {
		 		case C_LGC_50V_40A_10A_4A: //csk_120206
		 		case C_LGC_50V_40A_10A_4A_2:
		 		case C_LGC_50V_40A_10A_4A_3:
		 		case C_LGC_50V_40A_10A_4A_4:
				case C_SDI_70V_50A_5A_4KW:
				case C_SDI_70V_50A_5A_4KW_2:
				case C_SDI_70V_50A_5A_7KW:
				case C_SDI_70V_50A_5A_7KW_2:
				case C_SDI_70V_250A_25A_18KW:
					CaliUpdateCh(ch+4);
					break;
				case C_SBL_150V_250A_10A_38KW:  //lki_111010_s
				case C_SBL_150V_250A_10A_75KW_4:
				case C_SBL_150V_250A_10A_75KW_5:
					if(ch == 0) {
						CaliUpdateCh(ch+4);
						CaliUpdateCh(ch+5);
					}else if(ch == 1) {
						CaliUpdateCh(ch+5);
						CaliUpdateCh(ch+6);
					}
					break; //lki_111010_e
				default:
					break;
			}

			write_bd[bd] = 1;
		}
	}

	for(bd=0; bd < MAX_BD_PER_MODULE; bd++) {
		if(write_bd[bd] == 1) {
			rtn = Write_BdCaliData(bd);
			if(rtn < 0) {
				return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
			}
		}
	}

	userlog(DEBUG_LOG, psName, "cali_update complete\n");
	
	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_response(void)
{
	S_P1_RCV_CMD_RESPONSE cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_RESPONSE));
	
	if(cmd.response.code == P1_CD_ACK) {
		myPs->misc.net_time = myData->mData.misc.timer_1sec;
		myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
		myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
		myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;
	} else {
		return -1; //kjg_w
	}
	return 0;
}

int rcv_cmd_run(void)
{
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int k;
#endif

	int group, ch, i, j, toPs, idx, control, range, debug_on, cable_check_flag;
#ifdef __10MS__
	int h; //jhkw_190830
#endif
	long val;
	unsigned long chFlag, chFlag1, advCycleStep;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_RUN cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd, sizeof(S_P1_RCV_CMD_RUN));

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	if(myPs->signal[P1_SIG_TEST_HEADER_RCV] == P2) {
		myPs->signal[P1_SIG_TEST_HEADER_RCV] = P0;
	} else {
		userlog(DEBUG_LOG, psName, "run process error %d\n",
			(int)myPs->signal[P1_SIG_TEST_HEADER_RCV]);
		return send_cmd_response((char *)&cmd.header, P1_CD_TEST_END_UNRCV);
	}
#endif

	switch(myData->AppControl.config.systemModel) {
		case C_KATECH_20V_1000A_500A_100A:
		case C_LGC_50V_40A_10A_4A: //csk_120206
		case C_LGC_50V_40A_10A_4A_2:
		case C_LGC_50V_40A_10A_4A_3:
		case C_LGC_50V_40A_10A_4A_4:
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
		case C_SDI_70V_250A_25A_18KW:
		case C_SKI_AUX_BOX:
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
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
			k = P1_SIG_TEST_CONVERSION_END_1 + ch;
			if(myPs->signal[k] != P1) {
				myPs->signal[k] = P0;
				userlog(DEBUG_LOG, psName, "run process error2 ch:%d, %d\n",
					ch+1, (int)myPs->signal[k]);
				return send_cmd_response((char *)&cmd.header,
					P1_CD_TEST_END_UNRCV);
			}
#endif

			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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
								P1_CD_CH_CABLE_ERROR);
						} else if(myData->dio.signal[DIO_SIG_CABLE_FAIL1 + ch]
							== P1) {
							return send_cmd_response((char *)&cmd.header,
								P1_CD_CH_CABLE_ERROR);
						}
					}
				}
			}
		//jhkw_190830s
#ifdef __10MS__
		if(myData->COA_Client[0].config.data_save_10ms == 1) {
			userlog(DEBUG_LOG, psName, "run ch:%d\n", ch);
			for(h=0; h < MAX_10MS_SAVE_MSG_RING; h++) {
				myData->save_msg_10ms[ch][h].count_100 = 0;
				myData->save_msg_10ms[ch][h].total_count = 0;
				myData->save_msg_10ms[ch][h].write_idx = 0;
				myData->save_msg_10ms[ch][h].read_idx = 0;
			}
			myData->signal_10ms[ch].msg_count = 0;
			myData->signal_10ms[ch].send_msg_count = 0;
			myData->signal_10ms[ch].response_msg_count = 0;
			myData->signal_10ms[ch].save_start_flag = 1;
		}
#endif
		//jhkw_190830e
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

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
		k = P1_SIG_TEST_CONVERSION_END_1 + ch;
		myPs->signal[k] = P0;
#endif

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
			for(j=0; j < MAX_P1_STEP; j++) {
				if(myData->testCond[ch].local_object[j][idx] == STEP_CYCLE) {
					advCycleStep = (unsigned long)j;
				}
				if(j == (int)cmd.control.stepNo-1) {
					break;
				}
			}
			if(j == MAX_P1_STEP) advCycleStep = 0;
			myData->testCond[ch].reserved.select_advCycleStep = advCycleStep;
		}
		myData->testCond[ch].reserved.reserved_cmd = 0;
		myData->testCond[ch].reserved.reserved_stepNo = 0;
		myData->testCond[ch].reserved.reserved_cycleNo = 0;
	}

	debug_on = i = 0;
/*kjg_120221
	if(myData->AppControl.config.systemModel == C_LGC_100V_200A_10A_20KW) {
		if(myData->testCond[0].common_object[IDX_COM_OBJ_TOTAL_STEP] == 6) {
			//cycle->rest 5s->charge 200A 5s->discharge -200A 5s->loop->end
			if(myData->testCond[0].local_object[1][IDX_LOC_OBJ_END_V_UPPER]
				== 99900000) i++;
			if(myData->testCond[0].local_object[1][IDX_LOC_OBJ_TYPE]
				== STEP_REST) i++;
			if(myData->testCond[0].local_object[1][IDX_LOC_OBJ_END_TIME]
				== 500) i++;
			if(myData->testCond[0].local_object[2][IDX_LOC_OBJ_TYPE]
				== STEP_CHARGE) i++;
			if(myData->testCond[0].local_object[2][IDX_LOC_OBJ_END_TIME]
				== 500) i++;
			if(myData->testCond[0].local_object[3][IDX_LOC_OBJ_TYPE]
				== STEP_DISCHARGE) i++;
			if(myData->testCond[0].local_object[3][IDX_LOC_OBJ_END_TIME]
				== 500) i++;
			if(i == 7) debug_on = 1; //debug_on(calibration)
		}
	}
	if(myData->AppControl.config.systemModel == C_LGC_100V_200A_10A_20KW
		&& debug_on == 0) {
		val = myData->testCond[0].common_object[IDX_COM_OBJ_TOTAL_STEP];
		for(i=0; i < (int)val; i++) {
			j = 0;
			if(myData->testCond[0].local_object[i][IDX_LOC_OBJ_TYPE]
				== STEP_REST) j++;
			if(myData->testCond[0].local_object[i][IDX_LOC_OBJ_END_V_UPPER]
				== 99900000) {
				j++;
				if(j == 2) debug_on = 2; //debug_on(flash_request)
			}
			if(myData->testCond[0].local_object[i][IDX_LOC_OBJ_END_V_UPPER]
				== 99800000) {
				j++;
				if(j == 2) debug_on = 3; //debug_on(flash_reset)
			}
		}
	}
*/
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CMD_RUN;
	SendMsg.val[0] = group;
	SendMsg.val[1] = control;
	SendMsg.val[2] = debug_on;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName,
		"rcv_cmd_run %x %x %x %x, %x %x %x %x, %d %d %d\n",
		ch_flag.flag[0], ch_flag.flag[1], ch_flag.flag[2], ch_flag.flag[3],
		ch_flag.flag[4], ch_flag.flag[5], ch_flag.flag[6], ch_flag.flag[7],
		group, control, debug_on);
//shh_231124s
#if defined __DEBUG__	
	if(myData->AppControl.config.debugType == 230) {
		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		SendMsg.msg = MSG_COA1_SIL_DAQ_START_REQUEST;
		SendMsg.val[0] = group;
		SendMsg.val[1] = (int)0x01;
		send_msg(COA1_TO_SIL, (char *)&SendMsg);
		
		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		SendMsg.msg = MSG_COA1_SIL_VI_START_REQUEST;
		SendMsg.val[0] = group;
		SendMsg.val[1] = (int)0x0F;
		send_msg(COA1_TO_SIL2, (char *)&SendMsg);
	}
#endif
//shh_231124e
	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_stop(void)
{
	short int advCycleStep, advLoopStep; //jhkw_201127
	int group, ch, i, j, toPs, control=0, idx;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_STOP cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd, sizeof(S_P1_RCV_CMD_STOP));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_RUN
				&& myData->cData[ch].op.state != C_PAUSE) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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
		//jhkw_201127s
		advCycleStep = advLoopStep = 0;
		idx = IDX_LOC_OBJ_TYPE;
		for(j=0; j < MAX_P1_STEP; j++) {
			if(myData->testCond[ch].local_object[j][idx] == STEP_CYCLE) {
				advCycleStep = (short int)j;
			}
			if(j >= (int)cmd.control.stepNo-1) {
				if(myData->testCond[ch].local_object[j][idx] == STEP_LOOP) {
					advLoopStep = (short int)j;
					break;
				}
			}
		}
		if(j == MAX_P1_STEP) advCycleStep = advLoopStep = 0;
		//jhkw_201127e
		
		if(cmd.control.stepNo == 0 || cmd.control.cycleNo == 0) {
			control = 0; //direct command
		} else {
			control = 1; //reserved
		}
		if(control == 0) { //direct stop
			myData->testCond[ch].reserved.reserved_cmd = 0;
			myData->testCond[ch].reserved.reserved_stepNo = 0;
			myData->testCond[ch].reserved.reserved_cycleNo = 0;
			myData->testCond[ch].reserved.select_advCycleStep = 0; //jhkw_201127
			myData->testCond[ch].reserved.select_advLoopStep = 0; //jhkw_201127
		} else { //reserved stop
			myData->testCond[ch].reserved.reserved_cmd = 1;
			myData->testCond[ch].reserved.reserved_stepNo
				= (unsigned long)cmd.control.stepNo;
			myData->testCond[ch].reserved.reserved_cycleNo
				= (unsigned long)cmd.control.cycleNo;
			//jhkw_201127
			myData->testCond[ch].reserved.select_advCycleStep = advCycleStep;
			myData->testCond[ch].reserved.select_advLoopStep = advLoopStep; //jhkw_201127
		}
		myData->testCond[ch].reserved.select_run = 0;
		myData->testCond[ch].reserved.select_stepNo = 0;
		myData->testCond[ch].reserved.select_cycleNo = 0;
		//myData->testCond[ch].reserved.select_advCycleStep = 0; //jhkw_201127
	}
	
	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CMD_STOP;
	SendMsg.val[0] = group;
	SendMsg.val[1] = control;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName,
		"rcv_cmd_stop %x %x %x %x, %x %x %x %x, %d %d\n",
		ch_flag.flag[0], ch_flag.flag[1], ch_flag.flag[2], ch_flag.flag[3],
		ch_flag.flag[4], ch_flag.flag[5], ch_flag.flag[6], ch_flag.flag[7],
		group, control);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_pause(void)
{
   	short int advCycleStep, advLoopStep; //jhkw_201127
	int group, ch, i, j, toPs, control=0, idx;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_PAUSE cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd, sizeof(S_P1_RCV_CMD_PAUSE));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_RUN) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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
		//jhkw_201127s
		advCycleStep = advLoopStep = 0;
		idx = IDX_LOC_OBJ_TYPE;
		for(j=0; j < MAX_P1_STEP; j++) {
			if(myData->testCond[ch].local_object[j][idx] == STEP_CYCLE) {
				advCycleStep = (short int)j;
			}
			if(j >= (int)cmd.control.stepNo-1) {
				if(myData->testCond[ch].local_object[j][idx] == STEP_LOOP) {
					advLoopStep = (short int)j;
					break;
				}
			}
		}
		if(j == MAX_P1_STEP) advCycleStep = advLoopStep = 0;
		//jhkw_201127e

		if(cmd.control.stepNo == 0 || cmd.control.cycleNo == 0) {
			control = 0; //direct command
		} else {
			control = 1; //reserved command
		}
		if(control == 0) { //direct command
			myData->testCond[ch].reserved.reserved_cmd = 0;
			myData->testCond[ch].reserved.reserved_stepNo = 0;
			myData->testCond[ch].reserved.reserved_cycleNo = 0;
			myData->testCond[ch].reserved.select_advCycleStep = 0; //jhkw_201127
			myData->testCond[ch].reserved.select_advLoopStep = 0; //jhkw_201127
		} else { //reserved command
			myData->testCond[ch].reserved.reserved_cmd = 2;
			myData->testCond[ch].reserved.reserved_stepNo
				= (unsigned long)cmd.control.stepNo;
			myData->testCond[ch].reserved.reserved_cycleNo
				= (unsigned long)cmd.control.cycleNo;
			//jhkw_201127
			myData->testCond[ch].reserved.select_advCycleStep = advCycleStep;
			myData->testCond[ch].reserved.select_advLoopStep = advLoopStep; //jhkw_201127
		}
		myData->testCond[ch].reserved.select_run = 0;
		myData->testCond[ch].reserved.select_stepNo = 0;
		myData->testCond[ch].reserved.select_cycleNo = 0;
		//myData->testCond[ch].reserved.select_advCycleStep = 0; //jhkw_201127
	}
	
	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CMD_PAUSE;
	SendMsg.val[0] = group;
	SendMsg.val[1] = control;
	SendMsg.val[2] = cmd.control.code;	//jhkw_201102	//1:chamber	2:chiller
	send_msg(toPs, (char *)&SendMsg);

	/*userlog(DEBUG_LOG, psName,
		"rcv_cmd_pause %x %x %x %x, %x %x %x %x, %d %d\n",
		ch_flag.flag[0], ch_flag.flag[1], ch_flag.flag[2], ch_flag.flag[3],
		ch_flag.flag[4], ch_flag.flag[5], ch_flag.flag[6], ch_flag.flag[7],
		group, control);*/ 
	userlog(DEBUG_LOG, psName,
		"rcv_cmd_pause %x %x %x %x, %x %x %x %x, %d %d %d\n",
		ch_flag.flag[0], ch_flag.flag[1], ch_flag.flag[2], ch_flag.flag[3],
		ch_flag.flag[4], ch_flag.flag[5], ch_flag.flag[6], ch_flag.flag[7],
		group, control, cmd.control.code);	//jhkw_201102

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_continue(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CONTINUE cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CONTINUE));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_PAUSE) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CMD_CONTINUE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName,
		"rcv_cmd_continue %x %x %x %x, %x %x %x %x, %d\n",
		ch_flag.flag[0], ch_flag.flag[1], ch_flag.flag[2], ch_flag.flag[3],
		ch_flag.flag[4], ch_flag.flag[5], ch_flag.flag[6], ch_flag.flag[7],
		group);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_next_step(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_NEXT_STEP cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_NEXT_STEP));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_RUN
				&& myData->cData[ch].op.state != C_PAUSE) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CMD_NEXT_STEP;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName,
		"rcv_cmd_next_step %x %x %x %x, %x %x %x %x, %d\n",
		ch_flag.flag[0], ch_flag.flag[1], ch_flag.flag[2], ch_flag.flag[3],
		ch_flag.flag[4], ch_flag.flag[5], ch_flag.flag[6], ch_flag.flag[7],
		group);
	
	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_goto_step(void) //kjhw_151211s
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_GOTO_STEP cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_GOTO_STEP));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_RUN
				&& myData->cData[ch].op.state != C_PAUSE) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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
		myData->cData[ch].misc.user_branch_stepNo = cmd.user_branch_stepNo;
	}

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CMD_GOTO_STEP;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName,
		"rcv_cmd_goto_step %x %x %x %x, %x %x %x %x, %d, %d\n",
		ch_flag.flag[0], ch_flag.flag[1], ch_flag.flag[2], ch_flag.flag[3],
		ch_flag.flag[4], ch_flag.flag[5], ch_flag.flag[6], ch_flag.flag[7],
		group, cmd.user_branch_stepNo);
	
	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_chamber_flag(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CHAMBER_FLAG cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CHAMBER_FLAG));

	chFlag = 0x01;
	ch = 0; //kjhw_150210
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));

		myData->ChAttribute[ch].chamber_control = cmd.chamber_control;
		myData->ChAttribute[ch].chamber_standby = cmd.chamber_standby;
		//kjhw_141223
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__ || defined __COA_VER_100F__
#else //COA_VER_100F01~
		myData->ChAttribute[ch].chiller_control = cmd.chiller_control;
		//kjhw_170906
#endif
	}

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__ || defined __COA_VER_100F__
	userlog(DEBUG_LOG, psName, "recv_chamber control:%d, standby:%d\n",
		myData->ChAttribute[ch].chamber_control,
		myData->ChAttribute[ch].chamber_standby); //kjhw_141223
#else //COA_VER_100F01~ //kjhw_170906
	userlog(DEBUG_LOG, psName, "recv_chamber control:%d, standby:%d,
		chiller_control:%d\n",
		myData->ChAttribute[ch].chamber_control,
		myData->ChAttribute[ch].chamber_standby,
		myData->ChAttribute[ch].chiller_control);
#endif

	group = myPs->config.groupNo;
	toPs = COA1_TO_APP + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_APP_WRITE_CH_ATTRIBUTE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_chamber_continue(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CHAMBER_CONTINUE cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CHAMBER_CONTINUE));

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
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CMD_CHAMBER_CONTINUE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName, "recv_chamber_continue %x %x\n",
		ch_flag.flag[0], ch_flag.flag[1]);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_cable_check(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CABLE_CHECK cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CABLE_CHECK));

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
			P1_CD_GP_ISNT_IDLE_STANDBY);
	}

	for(i=0; i < P1_MAX_CH_PER_MODULE; i++) {
		if(myData->cData[i].op.state == C_STANDBY) {
			//myTestCond Assign kjg_170810
			//kjg_171219 myTestCond = (S_TEST_CONDITION *)&(myData->testCond[i]);
			myTestCond = &(myData->testCond[i]);

			convert_test_cond_cable_check(i);
		}
	}

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CMD_CABLE_CHECK;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName, "recv_cable_check\n");

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_cell_check(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CELL_CHECK cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CELL_CHECK));

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
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CMD_CELL_CHECK;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName, "recv_cell_check\n");

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_cycle_continue(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CYCLE_CONTINUE cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CYCLE_CONTINUE));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_PAUSE) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CMD_CYCLE_CONTINUE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName, "recv_cycle_continue %x %x\n",
		ch_flag.flag[0], ch_flag.flag[1]);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_can_comm_check(void)
{ //kjhw_141201s
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CAN_COMM_CHECK cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CAN_COMM_CHECK));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));

		myData->ChAttribute[ch].can_comm_check = cmd.can_comm_check;
	}

	group = myPs->config.groupNo;
	toPs = COA1_TO_APP + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_APP_WRITE_CH_ATTRIBUTE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
	//kjhw_141201e
}

int rcv_cmd_out_mux_use(void)
{ //kjhw_141201s
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_OUT_MUX_USE cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_OUT_MUX_USE));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));

		myData->ChAttribute[ch].out_mux_use = cmd.out_mux_use;
		userlog(DEBUG_LOG, psName, "out_mux_use ch:%d, val:%d\n",
			ch, cmd.out_mux_use);
	}

	group = myPs->config.groupNo;
	toPs = COA1_TO_APP + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_APP_WRITE_CH_ATTRIBUTE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
	//kjhw_141201e
}

int rcv_cmd_reset(void)
{
	int group, ch, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_RESET cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd, sizeof(S_P1_RCV_CMD_RESET));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
			}

			j = i / 8;
			ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));
		}
	}

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CMD_RESET;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	
	userlog(DEBUG_LOG, psName,
		"rcv_cmd_reset %x %x %x %x, %x %x %x %x, %d\n",
		ch_flag.flag[0], ch_flag.flag[1], ch_flag.flag[2], ch_flag.flag[3],
		ch_flag.flag[4], ch_flag.flag[5], ch_flag.flag[6], ch_flag.flag[7],
		group);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_comm_check_reply(void)
{
	char buf[8];
	S_P1_RCV_CMD_COMM_CHECK_REPLY comm_check_reply;
	
	memcpy((char *)&comm_check_reply, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_COMM_CHECK_REPLY));
	
	memset(buf, 0, sizeof buf);
	memcpy((char *)&buf, (char *)&comm_check_reply.result, 2);
	if(atoi(buf) == P1_CD_ACK) {
		myPs->misc.net_time = myData->mData.misc.timer_1sec;
		myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
		myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
		myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;
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
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	send_cmd_comm_check_reply();
	return 0;
}

int rcv_cmd_ch_attribute_set(void)
{
	int group, toPs;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CH_ATTRIBUTE_SET cmd;

	userlog(DEBUG_LOG, psName, "rcv_cmd_ch_attribute_set\n");

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CH_ATTRIBUTE_SET));
	
	switch(myData->AppControl.config.systemModel) { 
		//formation, output_power_tester
		case F_SDI_5V_50A_5A:
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
		case F_SDI_5V_450A_200A_100A_10A:

		//triangle_cycler
		case C_SDI_5V_60A_10A_1A:
		case C_SBL_5V_250A_60A_10A:
		case C_SDI_5V_300A_60A_10A:
		case C_SDI_5V_450A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A_2:

		//pack_cycler (linear)
		case C_LGC_5V_600A_10A:
		case C_TAESUNG_20V_600A_60A:
		case C_KATECH_20V_1000A_500A_100A: //csk_120309
		case C_LGC_50V_40A_10A_4A: //csk_120206
		case C_LGC_50V_40A_10A_4A_2:
		case C_LGC_50V_40A_10A_4A_3:
		case C_LGC_50V_40A_10A_4A_4:
		case C_LGC_50V_200A_10A:
		case C_ENERTECH_55V_100A_10A:
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
		case C_SDI_70V_250A_25A_18KW:
		case C_SAEHAN_72V_200A_20A:
		case C_VK_100V_200A_20A:
		case C_EIG_100V_200A_20A_2:
		
		//pack_cycler (switching)
		case C_JNU_50V_200A_100A_10KW:	//jhk_161202
		case C_LGC_50V_300A_10A_20KW:
		case C_SEBANG_50V_300A_100A_50A_15KW:	//jhkw_130912
		case C_SNU_60V_200A_100A_50A_1CH_12KW:  //jhkw_130221
		case C_HLGP_60V_200A_10A_12KW:
		case C_CANSYSTEM_60V_350A_100A_21KW:    //jhk_140828
		case C_LGCUSA_70V_350A_24KW:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_2:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_3:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_4:	//jhk_160720
		case C_LGCCHINA_80V_150A_12KW:		//jhk_160725
		case C_LGCCHINA_80V_150A_12KW_2:	//jhk_160725
		//case C_LGC_60V_250A_10A_30KW:
		//case C_LGC_60V_250A_10A_30KW_2:
		//case C_LGC_60V_250A_10A_30KW_3:
		case C_SKI_100V_100A_10A_10KW:	//jhk_120329
		case C_LGC_100V_100A_10A_10KW:	//jhk_121009
		case C_LGC_100V_100A_10A_10KW_2:	//jhk_121009
		case C_LGC_100V_100A_10A_10KW_3:	//jhk_121009
		case C_LGC_100V_100A_10A_10KW_4:	//jhk_121009
		case C_LGCCHINA_100V_200A_20KW_3:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_4:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_5:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_6:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_7:	//jhk_160731
		case C_LGC_100V_200A_10A_20KW:
		case C_KATECH_120V_150A_100A_50A_18KW:	//jhkw_130621
		case C_SBL_150V_250A_10A_38KW: //csk_111007_s
		case C_SBL_150V_250A_10A_75KW_4:
		case C_SBL_150V_250A_10A_75KW_5: //csk_111007_e
		case C_SEBANG_200V_100A_10A:
		case C_SEBANG_200V_100A_10A_2:
		case C_SEBANG_200V_100A_10A_3:
		case C_SEBANG_200V_100A_10A_4:
		case C_SEBANG_200V_100A_10A_5:
		case C_SEBANG_200V_200A_10A:
		case C_ECOCAR_200V_200A_100A_50A_40KW:  //jhkw_130827
		case C_SDIXIAN_300V_400A_100A_50A_25A:	//jhk_160527
		case C_LGC_400V_60A_10A:
		case C_ROTEM_400V_60A_10A:
		case C_KATECH_400V_60A_10A_24KW:
		case C_LGC_400V_100A_10A_40KW:
		case C_SKI_400V_100A_10A_40KW: //jhk_120112
		case C_3PSYSTEM_400V_100A_10A_40KW_1:	//jhk_120511
		case C_3PSYSTEM_400V_100A_10A_40KW_2:	//jhk_120708
		case C_AVL_400V_200A_10A_80KW:
		case C_ERAE_400V_300A_100A_30KW:	//jhk_150119
		case C_LGC_450V_200A_10A_6:
		case C_ROTEM_450V_200A_10A:
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_HYUNDAI_450V_200A_10A_90KW:
		case C_HYUNDAI_450V_200A_10A_90KW_2:
		case C_VENS_450V_200A_10A_90KW:
		case C_KEPCO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_50A_90KW: //kjh_150518
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
		case C_LGE_450V_250A_50A_112KW:		//jhk_150703
		case C_KATECH_450V_250A_10A_115KW:
		case C_LGC_500V_20A:
		case C_LGCCHINA_500V_150A_75KW:		//jhk_160727
		case C_LGCCHINA_500V_150A_75KW_2:	//jhk_160727
		case C_LGCCHINA_500V_150A_75KW_3:	//jhk_160727
		case C_KEPCO_500V_200A_10A_100KW:
		case C_LGC_500V_200A_10A:
		case C_GANGSO_500V_400A_200KW:	//jhk_160518
		case C_LGE_600V_250A_50A_150KW:	//jhk_150520
		case C_DAEHWA_600V_400A_200A_100A_200KW:	//jhk_170120
		case C_DAEHWA_750V_300A_100A_150KW:	//jhk_170203
		case C_BOSUNG_1200V_200A_100A_200KW:	//jhk_170106
		case C_KTL_1200V_300A_100A_300KW:	//kjh_171013
		case C_KATECH_1200V_300A_100A_50A_300KW:	//jhk_150108
			return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
		default:
			break;
	}

	memcpy((char *)&myData->ChAttribute, (char *)&cmd.attr,
		sizeof(S_P1_CH_ATTRIBUTE) * P1_MAX_CH_PER_MODULE);

	myData->mData.signal[M_SIG_PARALLER_MODE] = P0;	//jhkw_131011

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = COA1_TO_APP + group;
	SendMsg.msg = MSG_COA_APP_WRITE_CH_ATTRIBUTE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_aux_set(void)
{
	int group, toPs;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_AUX_SET cmd;

	userlog(DEBUG_LOG, psName, "rcv_cmd_aux_set\n");

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_AUX_SET));

	memcpy((char *)&myData->auxSetData, (char *)&cmd.auxSetData,
		sizeof(S_P1_AUX_SET_DATA) * MAX_AUX_DATA);

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = COA1_TO_APP + group;
	SendMsg.msg = MSG_COA_APP_WRITE_AUX_SET_DATA;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;
	
	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_can_receive_set(void)
{
	int group, toPs, ch, i, j;
	unsigned long chFlag, chFlag1;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CAN_RECEIVE_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CAN_RECEIVE_SET));

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
		return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
	}

	userlog(DEBUG_LOG, psName, "rcv_cmd_can_receive_set %d\n", ch+1);
/*
	if(myData->AppControl.config.systemModel == C_SK_450V_200A_10A_360KW) {
		if(ch == 0) {
		} else if(ch == 1 || ch == 2) {
			userlog(DEBUG_LOG, psName, "rcv_cmd_can_receive_set error %d\n",
				ch+1);
			return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
		} else if(ch == 3) {
		} else {
			userlog(DEBUG_LOG, psName, "rcv_cmd_can_receive_set error %d\n",
				ch+1);
			return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
		}
	}*/
	//kjhw_170629s
	switch(myData->AppControl.config.systemModel) {
		case C_SKI_120V_400A_100A_192KW:
		case C_SKI_120V_400A_100A_192KW_2:
		case C_SKI_120V_400A_100A_192KW_3:
		case C_SKI_120V_400A_100A_192KW_4:
		case C_SKI_120V_400A_100A_192KW_5:
		case C_SKI_120V_400A_100A_192KW_6:
		case C_SKI_120V_400A_100A_192KW_7:
		case C_SKI_120V_400A_100A_192KW_8:
			if((ch == 0) || (ch == 2)) {
			} else {
				userlog(DEBUG_LOG, psName, "rcv_cmd_can_rx_set error %d\n",
					ch+1);
				return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
			}
			break;
		case C_SK_450V_200A_10A_360KW:
			if((ch == 0) || (ch == 3)) {
			} else {
				userlog(DEBUG_LOG, psName, "rcv_cmd_can_rx_set error %d\n",
					ch+1);
				return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
			}
			break;
		default: break;
	}
	//kjhw_170629e
/*
	userlog(DEBUG_LOG, psName, "can_receive(%d) %lx %lx %d %d\n",
		ch+1,
		cmd.canReceiveSetData.normalData[0][0].canID,
		cmd.canReceiveSetData.normalData[0][1].canID,
		cmd.canReceiveSetData.normalData[0][0].byte_order,
		cmd.canReceiveSetData.normalData[0][1].byte_order);*/

	memcpy((char *)&myData->canReceiveSetData, (char *)&cmd.canReceiveSetData,
		sizeof(S_P1_CAN_RECEIVE_SET_DATA));

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = COA1_TO_APP + group;
	SendMsg.msg = MSG_COA_APP_WRITE_CAN_RECEIVE_SET_DATA;
	SendMsg.val[0] = group;
	SendMsg.val[1] = ch;
	send_msg(toPs, (char *)&SendMsg);
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_can_transmit_set(void)
{
	int group, toPs, ch, i, j;
	unsigned long chFlag, chFlag1;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CAN_TRANSMIT_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CAN_TRANSMIT_SET));

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
		return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
	}

	userlog(DEBUG_LOG, psName, "rcv_cmd_can_transmit_set %d\n", ch+1);

	/*if(myData->AppControl.config.systemModel == C_SK_450V_200A_10A_360KW) {
		if(ch == 0) {
		} else if(ch == 1 || ch == 2) {
			userlog(DEBUG_LOG, psName, "rcv_cmd_can_transmit_set error %d\n",
				ch+1);
			return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
		} else if(ch == 3) {
		} else {
			userlog(DEBUG_LOG, psName, "rcv_cmd_can_transmit_set error %d\n",
				ch+1);
			return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
		}
	}*/
	//kjhw_170629s
	switch(myData->AppControl.config.systemModel) {
		case C_SKI_120V_400A_100A_192KW:
		case C_SKI_120V_400A_100A_192KW_2:
		case C_SKI_120V_400A_100A_192KW_3:
		case C_SKI_120V_400A_100A_192KW_4:
		case C_SKI_120V_400A_100A_192KW_5:
		case C_SKI_120V_400A_100A_192KW_6:
		case C_SKI_120V_400A_100A_192KW_7:
		case C_SKI_120V_400A_100A_192KW_8:
		case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
			if((ch == 0) || (ch == 2)) {
			} else {
				userlog(DEBUG_LOG, psName, "rcv_cmd_can_tx_set error %d\n",
					ch+1);
				return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
			}
			break;
		case C_SK_450V_200A_10A_360KW:
			if((ch == 0) || (ch == 3)) {
			} else {
				userlog(DEBUG_LOG, psName, "rcv_cmd_can_tx_set error %d\n",
					ch+1);
				return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
			}
			break;
		default: break;
	}
	//kjhw_170629e

	memcpy((char *)&myData->canTransmitSetData, (char *)&cmd.canTransmitSetData,
		sizeof(S_P1_CAN_TRANSMIT_SET_DATA));

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = COA1_TO_APP + group;
	SendMsg.msg = MSG_COA_APP_WRITE_CAN_TRANSMIT_SET_DATA;
	SendMsg.val[0] = group;
	SendMsg.val[1] = ch;
	send_msg(toPs, (char *)&SendMsg);
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_ch_can_receive_set(void)
{
	int group, toPs, ch, i, j;
	unsigned long chFlag, chFlag1;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CH_CAN_RECEIVE_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CH_CAN_RECEIVE_SET));

	toPs = ch = 0;
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
			} else {
				toPs = 1;
				break;
			}
		}
	}

	if(toPs == 0) {
		return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
	}

	userlog(DEBUG_LOG, psName, "rcv_cmd_ch_can_receive_set %d, %x, %x\n",
		ch+1, cmd.header.chFlag[0], cmd.header.chFlag[1]);

	switch(myData->AppControl.config.systemModel) {
		case C_SKI_120V_400A_100A_192KW: //kjhw_170629
		case C_SKI_120V_400A_100A_192KW_2:
		case C_SKI_120V_400A_100A_192KW_3:
		case C_SKI_120V_400A_100A_192KW_4:
		case C_SKI_120V_400A_100A_192KW_5:
		case C_SKI_120V_400A_100A_192KW_6:
		case C_SKI_120V_400A_100A_192KW_7:
		case C_SKI_120V_400A_100A_192KW_8:
		case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
			if((ch == 0) || (ch == 2)) {
			} else {
				userlog(DEBUG_LOG, psName, "rcv_cmd_ch_can_rx_set error %d\n",
					ch+1);
				return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
			}
			break;
		case C_SK_450V_200A_10A_360KW:
			if((ch == 0) || (ch == 3)) {
			} else {
				userlog(DEBUG_LOG, psName, "rcv_cmd_ch_can_rx_set error %d\n",
					ch+1);
				return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
			}
			break;
		default: break;
	}

	//kjg_d_161207 cmd.commonData[0].bms_type = 50;
	//kjg_d_161207 cmd.commonData[1].bms_type = 50;
	
	userlog(DEBUG_LOG, psName,
		"kjg_d ch(%d) master can_rx baud:%d ext:%d bms:%d sjw:%d\n",
		ch+1, cmd.commonData[0].can_baudrate, cmd.commonData[0].extended_id,
		cmd.commonData[0].bms_type, cmd.commonData[0].sjw);
	userlog(DEBUG_LOG, psName,
		"kjg_d fd_flag:%d data_rate:%d crc:%d terminal_r:%d\n",
		cmd.commonData[0].can_fd_flag, cmd.commonData[0].can_datarate,
		cmd.commonData[0].crc_type, cmd.commonData[0].terminal_r);
	userlog(DEBUG_LOG, psName,
		"kjg_d ch(%d) slave can_rx baud:%d ext:%d bms:%d sjw:%d\n",
		ch+1, cmd.commonData[1].can_baudrate, cmd.commonData[1].extended_id,
		cmd.commonData[1].bms_type, cmd.commonData[1].sjw);
	userlog(DEBUG_LOG, psName,
		"kjg_d fd_flag:%d data_rate:%d crc:%d terminal_r:%d\n",
		cmd.commonData[1].can_fd_flag, cmd.commonData[1].can_datarate,
		cmd.commonData[1].crc_type, cmd.commonData[1].terminal_r);
		
	memcpy((char *)&myData->canReceiveSetData.commonData[ch][0],
		(char *)&cmd.commonData[0],
		sizeof(S_P1_CAN_RECEIVE_COMMON_DATA) * MAX_CAN_TYPE);
	memcpy((char *)&myData->canReceiveSetData.normalData[ch][0],
		(char *)&cmd.normalData[0],
		sizeof(S_P1_CAN_RECEIVE_NORMAL_DATA) * MAX_CAN_DATA);

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = COA1_TO_APP + group;
	SendMsg.msg = MSG_COA_APP_WRITE_CAN_RECEIVE_SET_DATA;
	SendMsg.val[0] = group;
	SendMsg.val[1] = ch;
	send_msg(toPs, (char *)&SendMsg);
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_ch_can_transmit_set(void)
{
	int group, toPs, ch, i, j;
	unsigned long chFlag, chFlag1;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CH_CAN_TRANSMIT_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CH_CAN_TRANSMIT_SET));

	toPs = ch = 0;
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
			} else {
				toPs = 1;
				break;
			}
		}
	}

	if(toPs == 0) {
		return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
	}

	userlog(DEBUG_LOG, psName, "rcv_cmd_ch_can_transmit_set %d, %x, %x\n",
		ch+1, cmd.header.chFlag[0], cmd.header.chFlag[1]);

	switch(myData->AppControl.config.systemModel) {
		case C_SKI_120V_400A_100A_192KW: //kjhw_170629
		case C_SKI_120V_400A_100A_192KW_2:
		case C_SKI_120V_400A_100A_192KW_3:
		case C_SKI_120V_400A_100A_192KW_4:
		case C_SKI_120V_400A_100A_192KW_5:
		case C_SKI_120V_400A_100A_192KW_6:
		case C_SKI_120V_400A_100A_192KW_7:
		case C_SKI_120V_400A_100A_192KW_8:
		case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
			if((ch == 0) || (ch == 2)) {
			} else {
				userlog(DEBUG_LOG, psName, "rcv_cmd_ch_can_tx_set error %d\n",
					ch+1);
				return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
			}
			break;
		case C_SK_450V_200A_10A_360KW:
			if((ch == 0) || (ch == 3)) {
			} else {
				userlog(DEBUG_LOG, psName, "rcv_cmd_ch_can_tx_set error %d\n",
					ch+1);
				return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
			}
			break;
		default: break;
	}

	//kjg_d_161207 cmd.commonData[0].bms_type = 50;
	//kjg_d_161207 cmd.commonData[1].bms_type = 50;
	
	userlog(DEBUG_LOG, psName,
		"kjg_d ch(%d) master can_tx baud:%d ext:%d bms:%d sjw:%d\n",
		ch+1, cmd.commonData[0].can_baudrate, cmd.commonData[0].extended_id,
		cmd.commonData[0].bms_type, cmd.commonData[0].sjw);
	userlog(DEBUG_LOG, psName,
		"kjg_d fd_flag:%d data_rate:%d crc:%d terminal_r:%d\n",
		cmd.commonData[0].can_fd_flag, cmd.commonData[0].can_datarate,
		cmd.commonData[0].crc_type, cmd.commonData[0].terminal_r);
	userlog(DEBUG_LOG, psName,
		"kjg_d ch(%d) slave can_tx baud:%d ext:%d bms:%d sjw:%d\n",
		ch+1, cmd.commonData[1].can_baudrate, cmd.commonData[1].extended_id,
		cmd.commonData[1].bms_type, cmd.commonData[1].sjw);
	userlog(DEBUG_LOG, psName,
		"kjg_d fd_flag:%d data_rate:%d crc:%d terminal_r:%d\n",
		cmd.commonData[1].can_fd_flag, cmd.commonData[1].can_datarate,
		cmd.commonData[1].crc_type, cmd.commonData[1].terminal_r);
		
	if(myPs->config.protocol_version < P1_PROTOCOL_VERSION_11) {
		memcpy((char *)&myData->canTransmitSetData.commonData[ch][0],
			(char *)&cmd.commonData[0],
			sizeof(S_P1_CAN_TRANSMIT_COMMON_DATA) * MAX_CAN_TYPE);
	} else {
		memcpy((char *)&myData->canTransmitSetData.commonData[ch][0],
			(char *)&cmd.commonData[0],
			sizeof(S_P1_CAN_TRANSMIT_COMMON_DATA) * MAX_CAN_TYPE);
	}
	memcpy((char *)&myData->canTransmitSetData.normalData[ch][0],
		(char *)&cmd.normalData[0],
		sizeof(S_P1_CAN_TRANSMIT_NORMAL_DATA) * MAX_CAN_DATA);

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	group = myPs->config.groupNo;
	toPs = COA1_TO_APP + group;
	SendMsg.msg = MSG_COA_APP_WRITE_CAN_TRANSMIT_SET_DATA;
	SendMsg.val[0] = group;
	SendMsg.val[1] = ch;
	send_msg(toPs, (char *)&SendMsg);

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_ch_attribute_request(void)
{
	userlog(DEBUG_LOG, psName, "rcv_cmd_ch_attribute_request\n");

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_ch_attribute_reply();
}

int rcv_cmd_aux_info_request(void)
{
	userlog(DEBUG_LOG, psName, "rcv_cmd_aux_info_request\n");

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_aux_info_reply();
}

int rcv_cmd_can_receive_info_request(void)
{
	userlog(DEBUG_LOG, psName, "rcv_cmd_can_receive_info_request\n");

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_can_receive_info_reply();
}

int rcv_cmd_can_transmit_info_request(void)
{
	userlog(DEBUG_LOG, psName, "rcv_cmd_can_transmit_info_request\n");

	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_can_transmit_info_reply();
}

int rcv_cmd_real_time_reply(void)
{
	S_P1_RCV_CMD_REAL_TIME_REPLY cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_REAL_TIME_REPLY));

	Update_RealTime((char *)&cmd.real_time);
	sleep(1);

	myPs->misc.sent_real_time_request = myData->mData.real_time[4]; //day

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_bms_comm_request(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BMS_COMM_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BMS_COMM_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_BMS_COMM_REQUEST;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.function_div;
	send_msg(toPs, (char *)&SendMsg);

	if((int)cmd.function_div == CAN_RX_FUNC_DIV_PACK_ISOLATION_TEST_START
		|| (int)cmd.function_div == CAN_RX_FUNC_DIV_PACK_ISOLATION_TEST_STOP) {
	} else {
#ifdef __COA__
		if(myData->mData.config.division_CAN == 1) { //kjhw_140620
			send_cmd_bms_comm_reply(ch, (int)cmd.function_div);
		} else {
			send_cmd_bms_comm_reply(ch*2, (int)cmd.function_div);
		}
#else
		send_cmd_bms_comm_reply(ch*2, (int)cmd.function_div);
#endif
	}

	return 0;
}

int rcv_cmd_can_transmit_change(void)
{
	int group, toPs, ch, i, j;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CAN_TRANSMIT_CHANGE cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CAN_TRANSMIT_CHANGE));

	toPs = ch = 0;
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			//if(myData->cData[ch].op.state != C_STANDBY) {
			//	return send_cmd_response((char *)&cmd.header,
			//		P1_CD_CH_STATE_ERROR);
			//} else {
				toPs = 1;
				break;
			//}
		}
	}

	if(toPs == 0) {
		return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
	}

	userlog(DEBUG_LOG, psName, "rcv_cmd_can_transmit_change %d, %x, %x\n",
		ch+1, cmd.header.chFlag[0], cmd.header.chFlag[1]);

	/*if(myData->AppControl.config.systemModel == C_SK_450V_200A_10A_360KW) {
		if(ch == 0) {
		} else if(ch == 1 || ch == 2) {
			userlog(DEBUG_LOG, psName, "rcv_cmd_can_transmit_change error %d\n",
				ch+1);
			return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
		} else if(ch == 3) {
		} else {
			userlog(DEBUG_LOG, psName, "rcv_cmd_can_transmit_change error %d\n",
				ch+1);
			return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
		}
	}*/
	//kjhw_170629s
	switch(myData->AppControl.config.systemModel) {
		case C_SKI_120V_400A_100A_192KW:
		case C_SKI_120V_400A_100A_192KW_2:
		case C_SKI_120V_400A_100A_192KW_3:
		case C_SKI_120V_400A_100A_192KW_4:
		case C_SKI_120V_400A_100A_192KW_5:
		case C_SKI_120V_400A_100A_192KW_6:
		case C_SKI_120V_400A_100A_192KW_7:
		case C_SKI_120V_400A_100A_192KW_8:
		case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
			if((ch == 0) || (ch == 2)) {
			} else {
				userlog(DEBUG_LOG, psName, "rcv_cmd_can_tx_change error %d\n",
					ch+1);
				return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
			}
			break;
		case C_SK_450V_200A_10A_360KW:
			if((ch == 0) || (ch == 3)) {
			} else {
				userlog(DEBUG_LOG, psName, "rcv_cmd_can_tx_change error %d\n",
					ch+1);
				return send_cmd_response((char *)&cmd.header, P1_CD_NACK);
			}
			break;
		default: break;
	}
	//kjhw_170629e

	userlog(DEBUG_LOG, psName, "can_transmit_change(%d) %ld %ld %d %d\n",
		ch+1,
		cmd.changeData[0].canID, cmd.changeData[1].canID,
		cmd.changeData[0].startBit, cmd.changeData[1].startBit);

	memcpy((char *)&myData->canTransmitChange.changeData[ch][0],
		(char *)&cmd.changeData[0],
		sizeof(S_P1_CAN_TRANSMIT_CHANGE_DATA) * MAX_CAN_TRANSMIT_CHANGE_DATA);

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));
	}

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CAN_TRANSMIT_CHANGE;
	SendMsg.val[0] = group;
	SendMsg.val[1] = ch;
	send_msg(toPs, (char *)&SendMsg);
	
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_bms_eol_data1_request(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BMS_EOL_DATA1_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BMS_EOL_DATA1_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	userlog(DEBUG_LOG, psName, "recv_bms_eol_data1_request\n");

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_BMS_EOL_DATA1_REQUEST;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}

int rcv_cmd_bms_eol_data2_request(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BMS_EOL_DATA2_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BMS_EOL_DATA2_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	userlog(DEBUG_LOG, psName, "recv_bms_eol_data2_request\n");

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_BMS_EOL_DATA2_REQUEST;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}

int rcv_cmd_bms_eol_pack_id_write(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BMS_EOL_PACK_ID_WRITE cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BMS_EOL_PACK_ID_WRITE));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	memcpy((char *)&myPs->misc.write_pack_id[0], (char *)&cmd.pack_id[0], 40);

	userlog(DEBUG_LOG, psName, "recv_bms_eol_pack_id_write : %s\n",
		myPs->misc.write_pack_id);

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_BMS_EOL_PACK_ID_WRITE;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}

int rcv_cmd_bms_eol_pack_id_check(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BMS_EOL_PACK_ID_CHECK cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BMS_EOL_PACK_ID_CHECK));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	userlog(DEBUG_LOG, psName, "recv_bms_eol_pack_id_check\n");

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_BMS_EOL_PACK_ID_CHECK;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}

int rcv_cmd_bms_eol_hi_pot_test(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BMS_EOL_HI_POT_TEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BMS_EOL_HI_POT_TEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	userlog(DEBUG_LOG, psName, "recv_bms_eol_hi_pot_test %d\n", cmd.type);

	if(cmd.type <= 0 || cmd.type >= 5) {
		send_cmd_bms_eol_hi_pot_test_reply(cmd.type, 0); //NG
	} else {
		group = myPs->config.groupNo;
		toPs = COA1_TO_MODULE + group;
		send_msg_ch_flag(toPs, (char *)&ch_flag);
		SendMsg.msg = MSG_COA_MODULE_BMS_EOL_HI_POT_TEST;
		SendMsg.val[0] = group;
		SendMsg.val[1] = (int)cmd.type;
		send_msg(toPs, (char *)&SendMsg);
	}

	return 0;
}

int rcv_cmd_uds_vbf_info_request(void)
{
	S_P1_RCV_CMD_UDS_VBF_INFO_REQUEST cmd;
	
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_UDS_VBF_INFO_REQUEST));

	userlog(DEBUG_LOG, psName, "recv_bms_uds_vbf_info_request\n");

	send_cmd_uds_vbf_info_reply();
	return 0;
}

int rcv_cmd_uds_vbf_change_request(void)
{
	int group, toPs;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_UDS_VBF_CHANGE_REQUEST cmd;
	
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_UDS_VBF_CHANGE_REQUEST));

	userlog(DEBUG_LOG, psName, "recv_bms_uds_vbf_change_request\n");

	memcpy((char *)&myData->CAN.tmp_can_flash_file_name[0],
		(char *)&cmd.vbf_file_name[0], 128);
	memcpy((char *)&myData->CAN.tmp_can_flash_file[0],
		(char *)&cmd.vbf_data[0], MAX_CAN_FLASH_FILE_SIZE);

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	SendMsg.msg = MSG_COA_APP_WRITE_VBF_DATA;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);
/*
	rtn = Write_VBF_File((char *)&cmd.vbf_file_name[0],
		(char *)&cmd.vbf_data[0]);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "vbf write error %d\n", rtn);
		send_cmd_uds_vbf_change_reply(0, (char *)&cmd.vbf_file_name[0]);
	} else {
		rtn = Read_CanFlashFile((char *)&psName);
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName, "vbf read error %d\n", rtn);
			send_cmd_uds_vbf_change_reply(0, (char *)&cmd.vbf_file_name[0]);
		} else {
			send_cmd_uds_vbf_change_reply(1, (char *)&cmd.vbf_file_name[0]);
		}
	}
*/
	return 0;
}

int rcv_cmd_bms_eol_data3_request(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BMS_EOL_DATA3_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BMS_EOL_DATA3_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	userlog(DEBUG_LOG, psName, "recv_bms_eol_data3_request\n");

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_BMS_EOL_DATA3_REQUEST;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}

int rcv_cmd_bms_eol_data3_led_request(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BMS_EOL_DATA3_LED_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BMS_EOL_DATA3_LED_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	userlog(DEBUG_LOG, psName, "recv_bms_eol_data3_led_request\n");

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_BMS_EOL_DATA3_LED_REQUEST;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}

int rcv_cmd_bms_eol_cvtn_id_request(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BMS_EOL_CVTN_ID_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BMS_EOL_CVTN_ID_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	userlog(DEBUG_LOG, psName, "recv_bms_eol_cvtn_id_request\n");

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_BMS_EOL_CVTN_ID_REQUEST;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}

int rcv_cmd_bms_eol_dtc_clear(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BMS_EOL_DTC_CLEAR cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BMS_EOL_DTC_CLEAR));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	userlog(DEBUG_LOG, psName, "recv_bms_eol_dtc_clear\n");

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_BMS_EOL_DTC_CLEAR;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}

int rcv_cmd_bms_eol_reset_micro(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BMS_EOL_RESET_MICRO cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BMS_EOL_RESET_MICRO));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	userlog(DEBUG_LOG, psName, "recv_bms_eol_reset_micro\n");

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_BMS_EOL_RESET_MICRO;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}

int rcv_cmd_bms_eol_3p_system_request(void)
{ //kjg_120619
	int ch=0, i, j;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BMS_EOL_3P_SYSTEM_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BMS_EOL_3P_SYSTEM_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	myData->CAN.percent_input_v[0] = cmd.percent_input_v;

	userlog(DEBUG_LOG, psName, "recv_bms_eol_3p_system_request %f\n",
		myData->CAN.percent_input_v[0]);

	send_cmd_bms_eol_3p_system_reply();

	return 0;
}

int rcv_cmd_daq_isolation_request(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_DAQ_ISOLATION_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_DAQ_ISOLATION_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_DAQ_ISOLATION_REQUEST;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.iso; //0:don't iso, 1:iso
	send_msg(toPs, (char *)&SendMsg);

	//kjg_120131 send_cmd_daq_isolation_reply((int)cmd.iso);

	return 0;
}

int rcv_cmd_daq_isolation_request2(void)
{ //jhkw_150224
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_DAQ_ISOLATION_REQUEST2 cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_DAQ_ISOLATION_REQUEST2));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	userlog(DEBUG_LOG, psName, "recv_daq_iso %d\n, div_ch : %d\n", 
			cmd.iso, cmd.div_ch);

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_DAQ_ISOLATION_REQUEST2;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.iso; //0:don't iso, 1:iso
	SendMsg.val[2] = (int)cmd.div_ch; //1: 1ch, 2: 2ch, 3: 3ch, 4: 4ch, 5: 5ch
	send_msg(toPs, (char *)&SendMsg);

	//kjg_120131 send_cmd_daq_isolation_reply((int)cmd.iso);

	return 0;
}

int rcv_cmd_out_mux_select_request(void)
{ //kjhw_151021
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_OUT_MUX_SELECT_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_OUT_MUX_SELECT_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				userlog(DEBUG_LOG, psName,
					"out_mux_select State Error ch:%d\n", ch);
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
			}
		}
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
		userlog(DEBUG_LOG, psName,
			"recv_out_mux_select ch:%d, out_mux:%d\n", ch, cmd.out_mux);

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));
	}

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_OUT_MUX_SELECT_REQUEST;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.out_mux; //0:all_off, 1:mux_a, 2:mux_b
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}

//jhkw_191108s
int rcv_cmd_th_table_set(void)
{ //kjhw_181111
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_TH_TABLE_SET cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_TH_TABLE_SET));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				userlog(DEBUG_LOG, psName,
					"Th_table State Error ch:%d\n", ch);
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
			}
		}
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
		userlog(DEBUG_LOG, psName,
			"recv th_table ch:%d, tableNo%d\n",
			ch, cmd.th_table);
		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));
	}

	group = myPs->config.groupNo;
	toPs = COA1_TO_APP + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_APP_READ_TH_TABLE_SET;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.th_table;
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}
//jhkw_191108e
int rcv_cmd_eol_procedure_request(void)
{ //kjg_120709
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_EOL_PROCEDURE_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_EOL_PROCEDURE_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	userlog(DEBUG_LOG, psName, "recv_eol_procedure %d %d\n",
		cmd.index, cmd.sub_index);

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_EOL_PROCEDURE_REQUEST;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.index;
	SendMsg.val[2] = (int)cmd.sub_index;
	send_msg(toPs, (char *)&SendMsg);

	if(cmd.index == 35) {
		memcpy((char *)&myPs->misc.write_pack_id[0],
			(char *)&cmd.string_value[0], 40);
		userlog(DEBUG_LOG, psName, "recv_eol_procedure pack_id : %s\n",
			myPs->misc.write_pack_id);
	}

	return 0;
}

int rcv_cmd_bms_procedure_request(void)
{ //kjg_161207
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BMS_PROCEDURE_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BMS_PROCEDURE_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 == 0) continue;

		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;

		userlog(DEBUG_LOG, psName, "recv_bms_procedure ch:%d bms_type:%d index:%d\n",
			ch+1, cmd.bms_type, cmd.index);

		j = i / 8;
		ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));
	}

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_BMS_PROCEDURE_REQUEST;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.bms_type;
	SendMsg.val[2] = (int)cmd.index;
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}

int rcv_cmd_calimode_request(void)    //jhkw_130119s
{
    int group, ch=0, i, j, toPs;
    unsigned long chFlag, chFlag1;
    S_MSG_CH_FLAG ch_flag;
    S_MSG_VAL SendMsg;
    S_P1_RCV_CMD_CALIMODE cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
    memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

    memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
	    sizeof(S_P1_RCV_CMD_CALIMODE));

    chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
	    j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state == C_RUN) {
	            return send_cmd_response((char *)&cmd.header,
		            P1_CD_CH_STATE_ERROR);
			}
		}
	}

	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
        chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
        if(chFlag1 == 0) continue;
		
		ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
		userlog(DEBUG_LOG, psName,
			"recv_calimode_request ch:%d mode:%d\n", ch, cmd.mode);

		j = i / 8;
        ch_flag.flag[j] |= (unsigned char)(chFlag << (i % 8));
	}
	                                                                                group = myPs->config.groupNo;
    toPs = COA1_TO_MODULE + group;
    send_msg_ch_flag(toPs, (char *)&ch_flag);
    SendMsg.msg = MSG_COA_MODULE_CALIMODE_REQUEST;
    SendMsg.val[0] = group;
    SendMsg.val[1] = (int)cmd.mode;
    //0:meter,  1:cali
    send_msg(toPs, (char *)&SendMsg);

	return 0;
}   //jhkw_130119e

//jhkw_200317s
int rcv_cmd_cali_stop(void)
{
	int group, toPs;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CALI_STOP cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CALI_STOP));

	if(myData->cData[cmd.ch].op.state != C_CALI) {
		userlog(DEBUG_LOG, psName,
			"dont cali stop cmd ch:%d State Not CALI Mode!!\n", cmd.ch);
		return send_cmd_response((char *)&cmd.header,
			P1_CD_CH_STATE_ERROR);
	}
	userlog(DEBUG_LOG, psName, "value : %d, ch : %d\n",
			cmd.value, cmd.ch);

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	SendMsg.msg = MSG_COA_MODULE_CMD_CALI_STOP;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.value;
	SendMsg.val[2] = (int)cmd.ch;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_autocali_request(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_AUTOCALI_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_AUTOCALI_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				userlog(DEBUG_LOG, psName,
					"autocali request error ch:%d State Run!!\n", ch);
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
			}
		}
	}
	userlog(DEBUG_LOG, psName, "mode : %d, measure_model : %d\n",
			cmd.mode, cmd.measure_model);

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	SendMsg.msg = MSG_COA_MODULE_CMD_AUTOCALI;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.mode;
	SendMsg.val[2] = (int)cmd.measure_model;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_autocali_temp_request(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_AUTOCALI_TEMP_REQUEST cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_AUTOCALI_TEMP_REQUEST));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				userlog(DEBUG_LOG, psName,
					"autocali_temp request error ch:%d State Run!!\n", ch);
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
			}
		}
	}
	userlog(DEBUG_LOG, psName, "autocali_temp mode : %d\n", cmd.mode);

	group = myPs->config.groupNo;
	toPs = COA1_TO_APP + group;
	SendMsg.msg = MSG_COA_APP_CMD_AUTOCALI_TEMP;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.mode;	//0:none, 1:initial, 2:update
	send_msg(toPs, (char *)&SendMsg);

	return 0;
}
//jhkw_200317e

int rcv_cmd_load_type_set(void) //kjhw_130129s
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_LOAD_TYPE_SET cmd;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_LOAD_TYPE_SET));
	
	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state == C_RUN) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

		userlog(DEBUG_LOG, psName,
			"recv_load_type ch:%d type:%d\n", ch, cmd.type);
	}
	
	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_LOAD_TYPE_SET;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.type;
	//0:EDLC, 1:Li-cell
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
} //kjhw_130129e

int rcv_cmd_ch_io_set(void)
{
	int group, ch=0, i, j, toPs;
	unsigned long chFlag, chFlag1;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_CH_IO_SET cmd;
	
	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_CH_IO_SET));

	chFlag = 0x01;
	for(i=0; i < myPs->misc.chInGroup; i++) {
		j = i / 32;
		chFlag1 = (chFlag << (i % 32)) & cmd.header.chFlag[j];
		if(chFlag1 != 0) {
			ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
			if(myData->cData[ch].op.state != C_STANDBY) {
				return send_cmd_response((char *)&cmd.header,
					P1_CD_CH_STATE_ERROR);
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

	userlog(DEBUG_LOG, psName, "recv_ch_io_set %d %d\n", cmd.type, cmd.value);

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	send_msg_ch_flag(toPs, (char *)&ch_flag);
	SendMsg.msg = MSG_COA_MODULE_CH_IO_SET;
	SendMsg.val[0] = group;
	SendMsg.val[1] = (int)cmd.type;
		//0:none, 1:key_on, 2:charge_on, 3:pack_relay
	SendMsg.val[2] = (int)cmd.value; //0:off, 1:on
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_buzzer_stop(void)
{
	int group, toPs;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_BUZZER_STOP cmd;
	
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_BUZZER_STOP));

	userlog(DEBUG_LOG, psName, "recv_buzzer_stop\n");

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	SendMsg.msg = MSG_COA_MODULE_BUZZER_STOP;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int rcv_cmd_alarm_reset(void)
{
	int group, toPs;
	S_MSG_VAL SendMsg;
	S_P1_RCV_CMD_ALARM_RESET cmd;
	
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_P1_RCV_CMD_ALARM_RESET));

	userlog(DEBUG_LOG, psName, "recv_alarm_reset\n");

	group = myPs->config.groupNo;
	toPs = COA1_TO_MODULE + group;
	SendMsg.msg = MSG_COA_MODULE_ALARM_RESET;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P1_CD_ACK);
}

int send_cmd_response(char *rcvHeader, int code)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_RESPONSE cmd;
	S_P1_CMD_HEADER header;

	cmd_size = sizeof(S_P1_SEND_CMD_RESPONSE);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_RESPONSE, SEQNUM_AUTO, body_size);
	
	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_P1_CMD_HEADER));

	cmd.header.reserved1 = header.reserved1;
	cmd.header.reserved2 = header.reserved2;
	cmd.header.chFlag[0] = header.chFlag[0];
	cmd.header.chFlag[1] = header.chFlag[1];
	cmd.response.cmd = (int)header.cmd_id;
	cmd.response.code = code;
	//cmd.response.cmdSerial = header.cmd_serial; //kjg_w

	if(code != P1_CD_ACK) { //kjg_170810
		userlog(DEBUG_LOG, psName, "cmd send response cmd_id:%x, code:%d\n",
			header.cmd_id, code);
	}

	rtn = send_command((char *)&cmd, cmd_size, P1_CMD_TO_PC_RESPONSE, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_RESPONSE);
	}
	return 0;
}

int send_cmd_test_cond_conversion_end(int code)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_TEST_COND_CONVERSION_END cmd;

	cmd_size = sizeof(S_P1_SEND_CMD_RESPONSE);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_TEST_COND_CONVERSION_END, SEQNUM_AUTO, body_size);

	cmd.response.cmd = 0;//(int)header.cmd_id;
	cmd.response.code = code; //0:nack, 1:ack
	//cmd.response.cmdSerial = header.cmd_serial; //kjg_w
	
	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_TEST_COND_CONVERSION_END, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_TEST_COND_CONVERSION_END);
	}
	return 0;
}

int send_cmd_test_cond_conversion_update_end(int code)
{ //kjg_170810
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_TEST_COND_CONVERSION_UPDATE_END cmd;

	cmd_size = sizeof(S_P1_SEND_CMD_RESPONSE);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_TEST_COND_CONVERSION_UPDATE_END, SEQNUM_AUTO, body_size);

	cmd.response.cmd = 0;//(int)header.cmd_id;
	cmd.response.code = code; //0:nack, 1:ack
	//cmd.response.cmdSerial = header.cmd_serial; //kjg_w
	
	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_TEST_COND_CONVERSION_UPDATE_END, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_TEST_COND_CONVERSION_UPDATE_END);
	}
	return 0;
}

int send_cmd_module_info_reply(int port_no)
{
	int cmd_size, body_size, rtn, i;
	S_P1_SEND_CMD_MODULE_INFO_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_MODULE_INFO_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_MODULE_INFO_REPLY, SEQNUM_AUTO, body_size);
	
	cmd.md_info.group_id = (unsigned int)myData->AppControl.config.moduleNo;
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

	cmd.md_info.can_comm_type = myData->CAN.config.commType; //kjg_180405

	cmd.md_info.ratioV = myData->mData.config.ratioV; //jhkw_231127s
	cmd.md_info.ratioI = myData->mData.config.ratioI; 
	cmd.md_info.ratioP = myData->mData.config.ratioP; //jhkw_231127e

	cmd.md_info.installedBd
		= (unsigned short int)myData->mData.config.installedBd ;
	i = myData->mData.config.chPerBd;
	if(i > P1_MAX_CH_PER_BD) i = P1_MAX_CH_PER_BD;
	cmd.md_info.chPerBd = (unsigned short int)i;

	i = myData->mData.config.installedCh;
	if(i > P1_MAX_CH_PER_MODULE) i = P1_MAX_CH_PER_MODULE;
	cmd.md_info.installedCh = (unsigned int)i;

	//kjg_w_s
	cmd.md_info.totalJig = 0; //myData->mData.config.totalJig;
	for(i=0; i < 16; i++) {
		cmd.md_info.BdinJig[i] = 0; //myData->mData.config.bdInJig[i];
	} //kjg_w_e
	
	userlog(DEBUG_LOG, psName, "send_cmd_module_info_reply %d\n", port_no);

	rtn = send_command((char *)&cmd, cmd_size, P1_CMD_TO_PC_MODULE_INFO_REPLY,
		port_no);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_MODULE_INFO_REPLY);
	}
	return 0;
}
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
int send_cmd_aux_info_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_AUX_INFO_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_AUX_INFO_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_AUX_INFO_REPLY, SEQNUM_AUTO, body_size);
	
	cmd.installedTemp = (short int)myData->mData.config.installedTemp;
	cmd.installedAuxV = (short int)myData->mData.config.installedAuxV;

	memcpy((char *)&cmd.auxSetData[0], (char *)&myData->auxSetData[0],
		sizeof(S_P1_AUX_SET_DATA) * MAX_AUX_DATA);

	userlog(DEBUG_LOG, psName, "send_cmd_aux_info_reply\n");

	rtn = send_command((char *)&cmd, cmd_size, P1_CMD_TO_PC_AUX_INFO_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_AUX_INFO_REPLY);
	}
	return 0;
}
#else //COA_VER_100F~
int send_cmd_aux_info_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_AUX_INFO_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_AUX_INFO_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_AUX_INFO_REPLY, SEQNUM_AUTO, body_size);
	
	cmd.installedTemp = (short int)myData->mData.config.installedTemp;
	cmd.installedAuxV = (short int)myData->mData.config.installedAuxV;
	cmd.installedTH = (short int)myData->mData.config.installedTH; //kjh_160610

	memcpy((char *)&cmd.auxSetData[0], (char *)&myData->auxSetData[0],
		sizeof(S_P1_AUX_SET_DATA) * MAX_AUX_DATA);

	userlog(DEBUG_LOG, psName, "send_cmd_aux_info_reply\n");

	rtn = send_command((char *)&cmd, cmd_size, P1_CMD_TO_PC_AUX_INFO_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_AUX_INFO_REPLY);
	}
	return 0;
}
#endif

int send_cmd_can_receive_info_reply(void)
{
	int cmd_size, body_size, rtn, toPs;
	S_P1_SEND_CMD_CAN_RECEIVE_INFO_REPLY cmd;
	S_MSG_VAL SendMsg;
	
	cmd_size = sizeof(S_P1_SEND_CMD_CAN_RECEIVE_INFO_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_CAN_RECEIVE_INFO_REPLY, SEQNUM_AUTO, body_size);
	
	for(rtn=0; rtn < P1_MAX_CH_PER_MODULE; rtn++) {
		cmd.canReceiveDataCount[rtn]
			= (short int)(myData->canReceiveDataCount[rtn][0]
			+ myData->canReceiveDataCount[rtn][1]);
	}

	memcpy((char *)&cmd.canReceiveSetData, (char *)&myData->canReceiveSetData,
		sizeof(S_P1_CAN_RECEIVE_SET_DATA));

	userlog(DEBUG_LOG, psName, "send_cmd_can_receive_info_reply\n");

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_CAN_RECEIVE_INFO_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x size:%d\n", rtn,
			P1_CMD_TO_PC_CAN_RECEIVE_INFO_REPLY,
			sizeof(S_P1_SEND_CMD_CAN_RECEIVE_INFO_REPLY));
	} else {
		if(myPs->config.protocol_version < P1_PROTOCOL_VERSION_5) {
			myPs->signal[P1_SIG_NET_CONNECTED] = P1;
			myPs->misc.net_time = myData->mData.misc.timer_1sec;
			myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
			myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
			myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;
			toPs = COA1_TO_MODULE + myPs->config.groupNo;
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_COA_MODULE_SAVE_MSG_FLAG;
			send_msg(toPs, (char *)&SendMsg); //run
		}
	}

	return 0;
}

int send_cmd_can_transmit_info_reply(void)
{
	int cmd_size, body_size, rtn, toPs;
	S_P1_SEND_CMD_CAN_TRANSMIT_INFO_REPLY cmd;
	S_MSG_VAL SendMsg;
	
	cmd_size = sizeof(S_P1_SEND_CMD_CAN_TRANSMIT_INFO_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_CAN_TRANSMIT_INFO_REPLY, SEQNUM_AUTO, body_size);
	
	for(rtn=0; rtn < P1_MAX_CH_PER_MODULE; rtn++) {
		cmd.canTransmitDataCount[rtn]
			= (short int)(myData->canTransmitDataCount[rtn][0]
			+ myData->canTransmitDataCount[rtn][1]);
	}

	memcpy((char *)&cmd.canTransmitSetData, (char *)&myData->canTransmitSetData,
		sizeof(S_P1_CAN_TRANSMIT_SET_DATA));

	userlog(DEBUG_LOG, psName, "send_cmd_can_transmit_info_reply\n");

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_CAN_TRANSMIT_INFO_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x size:%d\n", rtn,
			P1_CMD_TO_PC_CAN_TRANSMIT_INFO_REPLY,
			sizeof(S_P1_SEND_CMD_CAN_TRANSMIT_INFO_REPLY));
	} else {
		myPs->signal[P1_SIG_NET_CONNECTED] = P1;
		myPs->misc.net_time = myData->mData.misc.timer_1sec;
		myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
		myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
		myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;
		toPs = COA1_TO_MODULE + myPs->config.groupNo;
		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		SendMsg.msg = MSG_COA_MODULE_SAVE_MSG_FLAG;
		send_msg(toPs, (char *)&SendMsg); //run
	}

	return 0;
}

int send_cmd_ch_attribute_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_CH_ATTRIBUTE_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_CH_ATTRIBUTE_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_CH_ATTRIBUTE_REPLY, SEQNUM_AUTO, body_size);
	
	memcpy((char *)&cmd.attr, (char *)&myData->ChAttribute,
		sizeof(S_P1_CH_ATTRIBUTE) * P1_MAX_CH_PER_MODULE);

	userlog(DEBUG_LOG, psName, "send_cmd_ch_attribute_reply\n");

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_CH_ATTRIBUTE_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_CH_ATTRIBUTE_REPLY);
	}
	return 0;
}

void send_cmd_ch_data2(void)
{
	int i, msg, chInGroup, max_send_ch;
	long diff, time1, time2;

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_50A_5A:
			chInGroup = myData->mData.config.installedCh;
			break;
		default:
			chInGroup = myPs->misc.chInGroup;
			if(chInGroup > P1_MAX_CH_IN_GROUP) chInGroup = P1_MAX_CH_IN_GROUP;
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
		for(i=0; i < max_send_ch; i++) {
			send_monitoring_data(i);
		}

		for(i=0; i < 100; i++) {
			send_cmd_pulse_data();
		}

		myPs->misc.sent_monitor_data_time = time1;
		myPs->misc.sent_monitor_data_time2 = time2;
	}
}

void send_monitoring_data(int i)
{
	int cmd_size, body_size, rtn, ch, j, k, index;
	long val;
	S_P1_SEND_CMD_CH_DATA2	cmd;

	ch = myData->CellArray1[myPs->misc.chOffset + i].number2 - 1;
	index = i;

	cmd_size = sizeof(S_P1_CMD_HEADER) + sizeof(S_P1_CH_DATA)
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
		+ sizeof(S_P1_AUX_DATA)
			* (myData->auxDataCount[i][0] + myData->auxDataCount[i][1])
#else //COA_VER_100F~
		//kjh_160610
		+ sizeof(S_P1_AUX_DATA)
			* (myData->auxDataCount[i][0] + myData->auxDataCount[i][1]
			+ myData->auxDataCount[i][2])
#endif
		+ sizeof(S_P1_CAN_DATA)
			* (myData->canReceiveDataCount[i][0]
			+ myData->canReceiveDataCount[i][1]);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	cmd.header.reserved1 = (unsigned short)i;
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_CH_DATA2, SEQNUM_AUTO, body_size);
	
	cmd.chData.ch = i + 1;
	cmd.chData.select = SAVE_FLAG_MONITORING_DATA;

	cmd.chData.state
		= (unsigned char)convert_ch_state(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.state);
	cmd.chData.stepType
		= (unsigned char)convert_step_type(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.stepType);
	cmd.chData.stepMode
		= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.stepMode);

	cmd.chData.code
		= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.code);
	cmd.chData.stepNo = myData->cData[ch].op.idxStepNo + 1;
	cmd.chData.grade = myData->cData[ch].op.grade;

	cmd.chData.Vsens = myData->cData[ch].op.Vsens;

	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		val = myData->cData[ch].op.Isens;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.Isens;
			}
		}
		cmd.chData.Isens = val;

		val = myData->cData[ch].op.charge_AmpareHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_AmpareHour;
			}
		}
		cmd.chData.charge_AmpareHour = val;

		val = myData->cData[ch].op.discharge_AmpareHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_AmpareHour;
			}
		}
		cmd.chData.discharge_AmpareHour = val;

		val = myData->cData[ch].op.watt;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.watt;
			}
		}
		cmd.chData.watt = val;

		val = myData->cData[ch].op.charge_WattHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_WattHour;
			}
		}
		cmd.chData.charge_WattHour = val;

		val = myData->cData[ch].op.discharge_WattHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_WattHour;
			}
		}
		cmd.chData.discharge_WattHour = val;

		val = myData->cData[ch].op.meanI;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.meanI;
			}
		}
		cmd.chData.avgI = val;
	} else {
		cmd.chData.Isens = myData->cData[ch].op.Isens;
		cmd.chData.charge_AmpareHour = myData->cData[ch].op.charge_AmpareHour;
		cmd.chData.discharge_AmpareHour
			= myData->cData[ch].op.discharge_AmpareHour;
		cmd.chData.watt = myData->cData[ch].op.watt;
		cmd.chData.charge_WattHour = myData->cData[ch].op.charge_WattHour;
		cmd.chData.discharge_WattHour = myData->cData[ch].op.discharge_WattHour;
		cmd.chData.avgI = myData->cData[ch].op.meanI;
		if(myData->ChAttribute[ch].chNo_master == 0) {//kjhw_190521
			cmd.chData.Vsens = 0;
			cmd.chData.Isens = 0;
			cmd.chData.charge_AmpareHour = 0;
			cmd.chData.discharge_AmpareHour = 0;
			cmd.chData.watt = 0;
			cmd.chData.charge_WattHour = 0;
			cmd.chData.discharge_WattHour = 0;
			cmd.chData.avgI = 0;
		}
	}

	cmd.chData.capacitance = myData->cData[ch].op.capacitance;
	cmd.chData.z = myData->cData[ch].op.z;
	cmd.chData.SOC = myData->cData[ch].op.SOC;//20181219 KHK

#ifdef __COA_VER_100B__
	cmd.chData.runTime = myData->cData[ch].op.runTime;
	cmd.chData.totalRunTime = myData->cData[ch].op.totalRunTime;
	cmd.chData.cvTime = myData->cData[ch].op.cvTime;
#else //COA_VER_100B2~
	cmd.chData.runTime_day = myData->cData[ch].op.runTime_day;
	cmd.chData.runTime = myData->cData[ch].op.runTime;
	cmd.chData.totalRunTime_day = myData->cData[ch].op.totalRunTime_day;
	cmd.chData.totalRunTime = myData->cData[ch].op.totalRunTime;
	cmd.chData.cvTime_day = myData->cData[ch].op.cvTime_day;
	cmd.chData.cvTime = myData->cData[ch].op.cvTime;
#endif
	cmd.chData.realDate
		= myData->mData.real_time[6] * 10000 //year
		+ myData->mData.real_time[5] * 100 //month
		+ myData->mData.real_time[4]; //day
	cmd.chData.realClock
		= myData->mData.real_time[3] * 10000000 //hour
		+ myData->mData.real_time[2] * 100000 //min
		+ myData->mData.real_time[1] * 1000 //sec
		+ myData->mData.real_time[0]; //msec

	cmd.chData.reservedCmd = myData->cData[ch].op.reservedCmd;
	cmd.chData.external_comm_state = myData->cData[ch].misc.external_comm_state;
	cmd.chData.ch_output_state = myData->cData[ch].misc.ch_output_state; //kjg_101219
	cmd.chData.ch_input_state = myData->cData[ch].misc.ch_input_state; //kjg_101219

	cmd.chData.totalCycle = myData->cData[ch].misc.totalCycle;
	cmd.chData.elementCycle = myData->cData[ch].elementCycle.cycle_count;
	for(i=0; i < MAX_ACC_CYCLE; i++) {
		cmd.chData.accCycle[i] = myData->cData[ch].accCycle[i].cycle_count;
	}
	for(i=0; i < MAX_MULTI_CYCLE; i++) {
		cmd.chData.multiCycle[i] = myData->cData[ch].multiCycle[i].cycle_count;
	}

	if(myData->ChAttribute[ch].chamber_control == 0) {
		cmd.chData.chamber_control = 0;
	} else {
		cmd.chData.chamber_control = 1;
	}

	//kjhw_141201s
	//if(myData->ChAttribute[ch].can_comm_check == 0) {
	//	cmd.chData.can_comm_check = 0;
	//} else {
	//	cmd.chData.can_comm_check = 1;
	//}
	//kjhw_141201e
	//kjhw_151021s
	if(myData->ChAttribute[ch].out_mux_use == 0) {
		cmd.chData.out_mux_use = 0;
	} else {
		cmd.chData.out_mux_use = 1;
	}
	cmd.chData.out_mux_backup = myData->cData[ch].misc.out_mux_backup;
	//kjhw_151021e
//20181219 KHK-----------------------------------------------
	cmd.chData.maxAuxTemp = myData->cData[ch].misc.maxAuxT; //kjhw_181223
	cmd.chData.minAuxTemp = myData->cData[ch].misc.minAuxT; //kjhw_181223
	cmd.chData.devAuxTemp = myData->cData[ch].misc.diffAuxT; //kjhw_181223
	cmd.chData.avgAuxTemp = myData->cData[ch].misc.avgAuxT; //kjhw_181223
	cmd.chData.SOC = (long)myData->cData[ch].op.SOC;
//----------------------------------------------------------
	cmd.chData.avgV = myData->cData[ch].op.meanV;

	cmd.chData.Vinput = myData->cData[ch].misc.Vinput;
	cmd.chData.Vpower = myData->cData[ch].misc.Vpower;
	cmd.chData.Vbus = myData->cData[ch].misc.Vbus;
	//cmd.chData.reserved1[0] = myData->cData[ch].misc.reserved1[0];
	//cmd.chData.reserved1[1] = myData->cData[ch].misc.reserved1[1];
	//cmd.chData.reserved1[2] = myData->cData[ch].misc.reserved1[2];
	//cmd.chData.reserved1[3] = myData->cData[ch].misc.reserved1[3];
	//cmd.chData.reserved1[4] = myData->cData[ch].misc.reserved1[4];
	//cmd.chData.reserved1[5] = myData->cData[ch].misc.reserved1[5];
	//cmd.chData.reserved1[6] = myData->cData[ch].misc.reserved1[6];
	//kjhw_141201s
	//cmd.chData.reserved1 = myData->cData[ch].misc.reserved1[0];
	//cmd.chData.reserved1[0] = myData->cData[ch].misc.reserved1[0];
	//cmd.chData.reserved1[1] = myData->cData[ch].misc.reserved1[1];
	//kjhw_141201e
	

	//shhw_230614s
	//for displaying delta Min/Max AuxV and I to CTSMonPro
	/*
	cmd.chData.dMaxAuxV = myData->cData[ch].misc.dMaxAuxV;
	cmd.chData.dMinAuxV = myData->cData[ch].misc.dMinAuxV;
	cmd.chData.dMaxAuxVChNo = myData->cData[ch].misc.dMaxAuxVChNo;
	cmd.chData.dMinAuxVChNo = myData->cData[ch].misc.dMinAuxVChNo;
	cmd.chData.dIsens = myData->cData[ch].op.Isens; 
	*/
	//shhw_230614e

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
	cmd.chData.auxDataCount = (short int)(myData->auxDataCount[index][0]
		+ myData->auxDataCount[index][1]);
#else //COA_VER_100F~
	//kjh_160610
	cmd.chData.auxDataCount = (short int)(myData->auxDataCount[index][0]
		+ myData->auxDataCount[index][1] + myData->auxDataCount[index][2]);
#endif
	cmd.chData.canReceiveDataCount
		= (short int)(myData->canReceiveDataCount[index][0]
		+ myData->canReceiveDataCount[index][1]);

	k = 0;
	for(j=0; j < MAX_AUX_DATA; j++) {
		if((index+1) == myData->auxSetData[j].chNo) {
			cmd.auxData[k].auxChNo = myData->auxSetData[j].auxChNo;
			ch = myData->auxSetData[j].auxChNo - 1;
			cmd.auxData[k].auxType = myData->auxSetData[j].auxType;
			switch(myData->auxSetData[j].auxType) {
				case 0: //temperature
					cmd.auxData[k].val = myData->COM.com_port[1].value[ch]; 
					//MILLI UNIT ex) 1100 = 1.1 degree Celsius
					break;
				case 1: //sub sensing voltage
				case 2: //thermistor //kjh_160610
						ch = myData->aux_ch_num[ch].daq_ch;	//jhkw_201117
						cmd.auxData[k].val = myData->SubSensV.ch[ch].sensV;
					break;
				default:
					cmd.auxData[k].val = 0;
					break;
			}
			k++;
		}
	}

	k = myData->canReceiveDataCount[index][0]
		+ myData->canReceiveDataCount[index][1];
	for(j=0; j < k; j++) {
		cmd.canData[j].canType
			= myData->canReceiveSetData.normalData[index][j].canType;
		cmd.canData[j].data_type
			= myData->canReceiveSetData.normalData[index][j].data_type;
		cmd.canData[j].function_div
			= myData->canReceiveSetData.normalData[index][j].function_div;

		switch(myData->AppControl.config.systemModel) {
			case C_SDI_70V_50A_5A_4KW:
			case C_SDI_70V_50A_5A_4KW_2:
			case C_SDI_70V_50A_5A_7KW:
			case C_SDI_70V_50A_5A_7KW_2:
			case C_SDI_70V_250A_25A_18KW:
				i = 0;
				if(j < myData->canReceiveDataCount[index][0]) {
					if((myData->cData[ch].misc.external_comm_state & 0x04)
						!= 0) i = 10;
				} else {
					if((myData->cData[ch].misc.external_comm_state & 0x08)
						!= 0) i = 10;
				}
				if(i == 0) {
					i = (int)myData->canReceiveSetData
						.normalData[index][j].data_type;
				}
				break;
			default:
				i = (int)myData->canReceiveSetData
					.normalData[index][j].data_type;
				break;
		}

		switch(i) {
			case 0: //unsigned
			case 1: //signed
			case 2: //float
				if(myData->canReceiveSetData.normalData[index][j].bitCount
					<= 16) {
					cmd.canData[j].val.f_val[0]
						= myData->CanData[index][j].f_val[0];
				} else {
					memcpy((char *)&cmd.canData[j].val,
						(char *)&myData->CanData[index][j], 8);
				}
				//cmd.canData[j].val.d_val
				//	= myData->CanData[index][j].d_val;
				break;
			case 3: //string
				memcpy((char *)&cmd.canData[j].val,
					(char *)&myData->CanData[index][j], 8);
				break;
				//kjhw_140811s
			case 4: //hex
				if(myData->canReceiveSetData.normalData[index][j].bitCount
					<= 16) {
					cmd.canData[j].val.f_val[0]
						= myData->CanData[index][j].f_val[0];
				} else {
					memcpy((char *)&cmd.canData[j].val,
						(char *)&myData->CanData[index][j], 8);
				}
				break;
				//kjhw_140811e
			default:
				memset((char *)&cmd.canData[j].val, 0, 8);
				break;
		}
	}

	i = sizeof(S_P1_AUX_DATA)
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
		* (myData->auxDataCount[index][0] + myData->auxDataCount[index][1]);
#else //COA_VER_100F~
		//kjh_160610
		* (myData->auxDataCount[index][0] + myData->auxDataCount[index][1]
		+ myData->auxDataCount[index][2]);
#endif
	j = sizeof(S_P1_CAN_DATA) * (myData->canReceiveDataCount[index][0]
		+ myData->canReceiveDataCount[index][1]);
	memcpy((char *)&cmd.auxData + i, (char *)&cmd.canData, j);

	if(myPs->misc.network_port_type == 1) {
		rtn = send_command((char *)&cmd, cmd_size, P1_CMD_TO_PC_CH_DATA2, 1);
	} else {
		rtn = send_command((char *)&cmd, cmd_size, P1_CMD_TO_PC_CH_DATA2, 2);
	}
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x (ch:%d)\n", rtn,
			P1_CMD_TO_PC_CH_DATA2, index+1);
	}
}

void send_save_data(int data_count)
{ //kjg_110921
	int cmd_size, body_size, rtn, ch, msg, idx, send_count, rcv_count, i, j;
	int chInGroup;

	S_P1_SEND_CMD_CH_DATA2	cmd;

	chInGroup = myPs->misc.chInGroup;
	if(chInGroup > P1_MAX_CH_IN_GROUP) chInGroup = P1_MAX_CH_IN_GROUP;

	msg = myPs->config.groupNo;

/*	rcv_count = myData->save_msg[msg].total_count;
	if(rcv_count > data_count) rcv_count = data_count;

	if(rcv_count == 0) {
		j = 0;
		for(i=0; i < chInGroup; i++) {
			j += myData->save_msg[msg].count[i];
		}
		rcv_count = j;
		if(rcv_count > data_count) rcv_count = data_count;
	}*/

	if(myData->save_msg[msg].write_idx == myData->save_msg[msg].read_idx) {
		myData->save_msg[msg].total_count = 0;
		for(i=0; i < chInGroup; i++) {
			myData->save_msg[msg].count[i] = 0;
		}
		return;
	}

	rcv_count = data_count;
	for(send_count=0; send_count < rcv_count; send_count++) {
		if(myData->save_msg[msg].write_idx == myData->save_msg[msg].read_idx) {
			continue;
		}

		myData->save_msg[msg].read_idx++;
		if(myData->save_msg[msg].read_idx >= MAX_SAVE_MSG)
			myData->save_msg[msg].read_idx = 0;
		idx = myData->save_msg[msg].read_idx;

		if(myData->save_msg[msg].total_count > 0)
			myData->save_msg[msg].total_count--;

		ch = (int)myData->save_msg[msg].val[idx].chData.ch - 1;
		if(myData->save_msg[msg].count[ch] > 0)
			myData->save_msg[msg].count[ch]--;

		i = sizeof(S_P1_AUX_DATA)
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
			* (myData->auxDataCount[ch][0] + myData->auxDataCount[ch][1]);
#else //COA_VER_100F~
			//kjh_160610
			* (myData->auxDataCount[ch][0] + myData->auxDataCount[ch][1]
			+ myData->auxDataCount[ch][2]);
#endif
		j = sizeof(S_P1_CAN_DATA) * (myData->canReceiveDataCount[ch][0]
			+ myData->canReceiveDataCount[ch][1]);
		cmd_size = sizeof(S_P1_CMD_HEADER) + sizeof(S_P1_CH_DATA) + i + j;
		body_size = cmd_size - sizeof(S_P1_CMD_HEADER);

		memset((char *)&cmd, 0, cmd_size);
		cmd.header.reserved1 = (unsigned short)ch;
		make_header((char *)&cmd, REPLY_NO,
			P1_CMD_TO_PC_CH_DATA2, SEQNUM_AUTO, body_size);
	
		body_size = sizeof(S_P1_CH_DATA);
		memcpy((char *)&cmd.chData,
			(char *)&myData->save_msg[msg].val[idx].chData, body_size);

		memcpy((char *)&cmd.auxData,
			(char *)&myData->save_msg[msg].val[idx].auxData, i);

		memcpy((char *)&cmd.auxData + i,
			(char *)&myData->save_msg[msg].val[idx].canData, j);

		if(myPs->misc.network_port_type == 1) {
			rtn = send_command((char *)&cmd, cmd_size,
				P1_CMD_TO_PC_CH_DATA2, 1);
		} else {
			rtn = send_command((char *)&cmd, cmd_size,
				P1_CMD_TO_PC_CH_DATA2, 2);
		}
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x (ch:%d)\n",
				rtn, P1_CMD_TO_PC_CH_DATA2, ch+1);
		}
	}
}

void send_cmd_pulse_data(void)
{ //kjg_110921
	int cmd_size, body_size, rtn, ch, idx, msg, i, j, send_flag, data_count;
	int cnt1;
	S_P1_SEND_CMD_PULSE_DATA cmd;
	
	data_count = 100;
	cnt1 = 100;
	msg = 0;
	
	for(i=0; i < myData->mData.config.installedCh; i++) {
		send_flag = 0;
		//ch = myData->CellArray1[i].number2 - 1;
		ch = i;

		if(myData->pulse_msg_1[msg].write_idx[ch]
			== myData->pulse_msg_1[msg].read_idx[ch]) {
			cnt1++;
			myData->pulse_msg_1[msg].count[ch] = 0;
			continue;
		}

		myData->pulse_msg_1[msg].read_idx[ch]++;
		if(myData->pulse_msg_1[msg].read_idx[ch] >= MAX_PULSE_MSG)
			myData->pulse_msg_1[msg].read_idx[ch] = 0;
		idx = myData->pulse_msg_1[msg].read_idx[ch];

		if(myData->pulse_msg_1[msg].total_count > 0)
			myData->pulse_msg_1[msg].total_count--;

		if(myData->pulse_msg_1[msg].count[ch] > 0) {
			myData->pulse_msg_1[msg].count[ch]--;
		}

		myPs->pulse_data[ch].totalCycle
			= myData->pulse_msg_1[msg].val[idx][ch].totalCycle;
		myPs->pulse_data[ch].stepNo
			= myData->pulse_msg_1[msg].val[idx][ch].stepNo;
		switch(myData->pulse_msg_1[msg].val[idx][ch].count_flag) {
			case 0: //start
				myPs->pulse_data[ch].dataCount = 1;
				break;
			case 1: //running
				myPs->pulse_data[ch].dataCount++;
				if(myPs->pulse_data[ch].dataCount >= data_count) {
					send_flag = 1;
				}
				break;
			case 2: //end
				myPs->pulse_data[ch].dataCount++;
				send_flag = 1;
				break;
			default:
				myPs->pulse_data[ch].dataCount++;
				break;
		}

		j = myPs->pulse_data[ch].dataCount - 1;
		myPs->pulse_data[ch].val[j].runTime
			= myData->pulse_msg_1[msg].val[idx][ch].runTime;
		myPs->pulse_data[ch].val[j].Vsens
			= myData->pulse_msg_1[msg].val[idx][ch].Vsens;
		myPs->pulse_data[ch].val[j].Isens
			= myData->pulse_msg_1[msg].val[idx][ch].Isens;
		myPs->pulse_data[ch].val[j].capacity
			= myData->pulse_msg_1[msg].val[idx][ch].capacity;
		myPs->pulse_data[ch].val[j].wattHour
			= myData->pulse_msg_1[msg].val[idx][ch].wattHour;

		if(send_flag == 0) continue;

		cmd_size = sizeof(S_P1_CMD_HEADER) + sizeof(long) * 3
			+ sizeof(S_P1_PULSE_VAL) * myPs->pulse_data[ch].dataCount;
		body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
		memset((char *)&cmd, 0, cmd_size);
		cmd.header.reserved1 = (unsigned short)ch;
		make_header((char *)&cmd, REPLY_NO,
			P1_CMD_TO_PC_PULSE_DATA, SEQNUM_AUTO, body_size);

		memcpy((char *)&cmd.totalCycle, (char *)&myPs->pulse_data[ch],
			body_size);
		myPs->pulse_data[ch].dataCount = 0;

		if(myPs->misc.network_port_type == 1) {
			rtn = send_command((char *)&cmd, cmd_size,
				P1_CMD_TO_PC_PULSE_DATA, 1);
		} else {
			rtn = send_command((char *)&cmd, cmd_size,
				P1_CMD_TO_PC_PULSE_DATA, 2);
		}
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x (ch:%d)\n",
				rtn, P1_CMD_TO_PC_PULSE_DATA, ch+1);
		}
	}

	if(myData->mData.config.installedCh == cnt1) {
		myData->pulse_msg_1[msg].total_count = 0;
	}
}
//jhkw_190830s
void send_save_data_2(int data_count, int ch, int msg)
{
#ifdef __10MS__
	//int cmd_size, rtn, ch, msg, idx, send_count, rcv_count, i, j, m;
	int cmd_size, rtn, idx, send_count, rcv_count, i, j, m;
	int chInGroup;
	unsigned long body_size, body_size_total;

	S_P1_SEND_CMD_CH_DATA3	cmd;

	//cmd_size = ch = body_size_total = 0;
	cmd_size = body_size_total = 0;

	chInGroup = myPs->misc.chInGroup;
	if(chInGroup > P1_MAX_CH_IN_GROUP) chInGroup = P1_MAX_CH_IN_GROUP;

	//msg = 0;
	if(myData->save_msg_10ms[ch][msg].write_idx == myData->save_msg_10ms[ch][msg].read_idx) {
		myData->save_msg_10ms[ch][msg].total_count = 0;
		for(i=0; i < chInGroup; i++) {
			myData->save_msg_10ms[ch][msg].count[i] = 0;
		}
		return;
	}
	//userlog(DEBUG_LOG, psName, "ch_data3 before write idx = %d , read idx =  %d\n", 
	//		myData->save_msg_10ms[ch][msg].write_idx,
	//		myData->save_msg_10ms[ch][msg].read_idx);

	rcv_count = data_count;
	for(send_count=0; send_count < rcv_count; send_count++) {
		if(myData->save_msg_10ms[ch][msg].write_idx == myData->save_msg_10ms[ch][msg].read_idx) {
			//continue;
			break;
		}

		myData->save_msg_10ms[ch][msg].read_idx++;
		if(myData->save_msg_10ms[ch][msg].read_idx >= MAX_SAVE_MSG) {
			myData->save_msg_10ms[ch][msg].read_idx = 0;
		}
		idx = myData->save_msg_10ms[ch][msg].read_idx;

		if(myData->save_msg_10ms[ch][msg].total_count > 0) {
			myData->save_msg_10ms[ch][msg].total_count--;
		}

		//ch = (int)myData->save_msg_10ms[ch][msg].val[idx].chData.ch - 1;
		if(myData->save_msg_10ms[ch][msg].count[ch] > 0) {
			myData->save_msg_10ms[ch][msg].count[ch]--;
		}

		i = sizeof(S_P1_CH_DATA);
		j = sizeof(S_P1_AUX_DATA)
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
			* (myData->auxDataCount[ch][0] + myData->auxDataCount[ch][1]);
#else //COA_VER_100F~
			//kjh_160610
			* (myData->auxDataCount[ch][0] + myData->auxDataCount[ch][1]
			+ myData->auxDataCount[ch][2]);
#endif
		m = sizeof(S_P1_CAN_DATA) * (myData->canReceiveDataCount[ch][0]
			+ myData->canReceiveDataCount[ch][1]);
		body_size = i + j + m;
		memset((char *)&cmd.add_data[send_count], 0, body_size);
		
		memcpy((char *)&cmd.add_data[send_count].chData,
			(char *)&myData->save_msg_10ms[ch][msg].val[idx].chData, i);

		memcpy((char *)&cmd.add_data[send_count].auxData,
			(char *)&myData->save_msg_10ms[ch][msg].val[idx].auxData, j);

		memcpy((char *)&cmd.add_data[send_count].auxData + j,
			(char *)&myData->save_msg_10ms[ch][msg].val[idx].canData, m);

		if(send_count != 0) {
			memcpy((char *)&cmd.add_data[0] + body_size_total,
				(char *)&cmd.add_data[send_count], body_size);
		}

		body_size_total += body_size;
	}
	if(body_size_total > 0) {
		cmd.ch_count = send_count;
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
		cmd.aux_count = myData->auxDataCount[ch][0] + myData->auxDataCount[ch][1];
#else //COA_VER_100F~
		cmd.aux_count = myData->auxDataCount[ch][0] + myData->auxDataCount[ch][1]
						+ myData->auxDataCount[ch][2];
#endif
		cmd.can_count = myData->canReceiveDataCount[ch][0] 
						+ myData->canReceiveDataCount[ch][1];
		cmd.header.reserved1 = (unsigned short)ch;
		cmd_size = (sizeof(S_P1_CMD_HEADER) + sizeof(cmd.ch_count)
					+ sizeof(cmd.aux_count) + sizeof(cmd.can_count)
					+ sizeof(cmd.reserved) + body_size_total);
		//608byte = 24byte + 2byte + 2byte + 2byte + 2byte;
		
		body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
		//584byte = 608byte - 24byte;

		make_header((char *)&cmd, REPLY_NO,
			P1_CMD_TO_PC_CH_DATA3, SEQNUM_AUTO, body_size);
	
		if(myPs->misc.network_port_type == 1) {
			rtn = send_command((char *)&cmd, cmd_size,
				P1_CMD_TO_PC_CH_DATA2, 1);
		} else {
			rtn = send_command((char *)&cmd, cmd_size,
				P1_CMD_TO_PC_CH_DATA3, 2);
			myData->signal_10ms[ch].send_msg_count = msg;
			myData->signal_10ms[ch].save_start_flag = 0;
		}
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x (ch:%d)\n",
				rtn, P1_CMD_TO_PC_CH_DATA3, ch+1);
		}
	}
#endif
}
//jhkw_190830e

int send_cmd_step_cond_reply(char *rcvHeader, int ch, int stepNo)
{ //kjg_170810
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_STEP_COND_REPLY cmd;
	S_P1_CMD_HEADER header;

	cmd_size = sizeof(S_P1_SEND_CMD_STEP_COND_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO, P1_CMD_TO_PC_STEP_COND_REPLY,
		SEQNUM_AUTO, body_size);

	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_P1_CMD_HEADER));

	cmd.header.reserved1 = header.reserved1;
	cmd.header.reserved2 = header.reserved2;
	cmd.header.chFlag[0] = header.chFlag[0];
	cmd.header.chFlag[1] = header.chFlag[1];

	convert_test_cond_module_coa_step(ch, stepNo);

	memcpy((char *)&cmd.testCond_step,
		(char *)&myPs->step_cond_update.testCond_step,
		sizeof(S_P1_TEST_COND_STEP));

	rtn = send_command((char *)&cmd, cmd_size, P1_CMD_TO_PC_STEP_COND_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x size:%d\n", rtn,
			P1_CMD_TO_PC_STEP_COND_REPLY, cmd_size);
	}

	userlog(DEBUG_LOG, psName, "send_cmd_step_cond_reply ch:%d, stepNo:%d\n",
		ch+1, stepNo+1);

	return 0;
}

int send_cmd_common_safety_cond_reply(char *rcvHeader, int ch)
{ //kjg_170810
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_COMMON_SAFETY_COND_REPLY cmd;
	S_P1_CMD_HEADER header;

	cmd_size = sizeof(S_P1_SEND_CMD_COMMON_SAFETY_COND_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO, P1_CMD_TO_PC_COMMON_SAFETY_COND_REPLY,
		SEQNUM_AUTO, body_size);

	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_P1_CMD_HEADER));

	cmd.header.reserved1 = header.reserved1;
	cmd.header.reserved2 = header.reserved2;
	cmd.header.chFlag[0] = header.chFlag[0];
	cmd.header.chFlag[1] = header.chFlag[1];

	convert_test_cond_module_coa_common_safety(ch);

	memcpy((char *)&cmd.testCond_header,
		(char *)&myPs->common_safety_cond_update.testCond_header,
		sizeof(S_P1_TEST_COND_HEADER));

	memcpy((char *)&cmd.testCond_safety,
		(char *)&myPs->common_safety_cond_update.testCond_safety,
		sizeof(S_P1_TEST_COND_SAFETY));

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_COMMON_SAFETY_COND_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x size:%d\n", rtn,
			P1_CMD_TO_PC_COMMON_SAFETY_COND_REPLY, cmd_size);
	}

	userlog(DEBUG_LOG, psName, "send_cmd_common_safety_cond_reply ch:%d\n",
		ch+1);

	return 0;
}

void send_cmd_meter_connect_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_METER_CONNECT_REPLY cmd;

	cmd_size = sizeof(S_P1_SEND_CMD_METER_CONNECT_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_CALI_METER_CONNECT_REPLY, SEQNUM_AUTO, body_size);
	
	cmd.header.chFlag[0] = 0;
	cmd.header.chFlag[1] = 0;
	cmd.state = 1; //disconnect:0, connect:1 //kjg_w
	
	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_CALI_METER_CONNECT_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_CALI_METER_CONNECT_REPLY);
	}
}

void send_cmd_cali_start_reply(int ch, int code)
{
	int cmd_size, body_size, rtn, i;
	unsigned long chFlag;
	S_P1_SEND_CMD_CALI_START_REPLY cmd;

	cmd_size = sizeof(S_P1_SEND_CMD_CALI_START_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_CALI_START_REPLY, SEQNUM_AUTO, body_size);
	
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
	cmd.response.cmd = P1_CMD_TO_SBC_CALI_START;
	cmd.response.code = code;
	//cmd.response.cmdSerial = header.cmd_serial; //kjg_w
	
	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_CALI_START_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_CALI_START_REPLY);
	}
}

void send_cmd_cali_normal_result(int ch)
{
	int cmd_size, body_size, rtn, i, type, range, point;
	unsigned long chFlag;
	S_P1_SEND_CMD_CALI_NORMAL_RESULT cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_CALI_NORMAL_RESULT);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_CALI_NORMAL_RESULT, SEQNUM_AUTO, body_size);
	
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
		//jhkw_200317
		cmd.result.setPointDVM_Real[point]
			= myData->cali[ch].tmpData[type][range].set_meter_real[point];
	}

	for(point=0; point < cmd.result.checkPointNum; point++) {
		cmd.result.checkPointAD[point]
			= myData->cali[ch].tmpData[type][range].check_ad[point];
		cmd.result.checkPointDVM[point]
			= myData->cali[ch].tmpData[type][range].check_meter[point];
		//jhkw_200317
		cmd.result.checkPointDVM_Real[point]
			= myData->cali[ch].tmpData[type][range].check_meter_real[point];
	}
	
	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_CALI_NORMAL_RESULT, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_CALI_NORMAL_RESULT);
	}
}

//#ifdef __B_TYPE__
void send_cmd_cali_normal_result_1(int ch, int count)
{ //kjhw_120504 Vref x 2
	int cmd_size, body_size, rtn, i, type, range, point;
	unsigned long chFlag;
	S_P1_SEND_CMD_CALI_NORMAL_RESULT cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_CALI_NORMAL_RESULT);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_CALI_NORMAL_RESULT, SEQNUM_AUTO, body_size);
	
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
	cmd.result.count = count; //kjhw_120504 Vref x 2
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
		//jhkw_200317
		cmd.result.setPointDVM_Real[point]
			= myData->cali[ch].tmpData[type][range].set_meter_real[point];
	}

	for(point=0; point < cmd.result.checkPointNum; point++) {
		cmd.result.checkPointAD[point]
			= myData->cali[ch].tmpData[type][range].check_ad[point];
		cmd.result.checkPointDVM[point]
			= myData->cali[ch].tmpData[type][range].check_meter[point];
		//jhkw_200317
		cmd.result.checkPointDVM_Real[point]
			= myData->cali[ch].tmpData[type][range].check_meter_real[point];
	}
	
	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_CALI_NORMAL_RESULT, 1);
	userlog(DEBUG_LOG, psName, "cmd send !!! %d 0x%x\n", rtn,
		P1_CMD_TO_PC_CALI_NORMAL_RESULT);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_CALI_NORMAL_RESULT);
	}
}
//#endif

//void send_cmd_cali_check_result(int ch)
void send_cmd_cali_check_result(int ch, int count) //jhkw_200317
{
	int cmd_size, body_size, rtn, i, type, range, point;
	unsigned long chFlag;
	S_P1_SEND_CMD_CALI_CHECK_RESULT cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_CALI_CHECK_RESULT);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_CALI_CHECK_RESULT, SEQNUM_AUTO, body_size);
	
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
	cmd.result.count = count;	//jhkw_200317
	cmd.result.range = (unsigned char)range;
	cmd.result.checkPointNum
		= myData->cali[ch].tmpCond[type][range].point.checkPointNum;
	cmd.result.ch = (unsigned char)(i+1);

	for(point=0; point < cmd.result.checkPointNum; point++) {
		cmd.result.checkPointAD[point]
			= myData->cali[ch].tmpData[type][range].check_ad[point];
		cmd.result.checkPointDVM[point]
			= myData->cali[ch].tmpData[type][range].check_meter[point];
		//jhkw_200317
		cmd.result.checkPointDVM_Real[point]
			= myData->cali[ch].tmpData[type][range].check_meter_real[point];
	}
	
	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_CALI_CHECK_RESULT, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_CALI_CHECK_RESULT);
	}
}

void send_cmd_comm_check_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_COMM_CHECK_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_COMM_CHECK_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_COMM_CHECK_REPLY, SEQNUM_AUTO, body_size);

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_COMM_CHECK_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_COMM_CHECK_REPLY);
	}
}
//jhkw_200508s
int rcv_cmd_comm_check_2(void)
{
	myPs->misc.net_time = myData->mData.misc.timer_1sec;
	myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
	myPs->misc.pause_time = myData->mData.misc.timer_1sec; //jhkw_190503
	myPs->misc.pause_time2 = myData->mData.misc.timer_1000ms;

	//userlog(DEBUG_LOG, psName, "rcv cmd comm check_2 !!! \n");
	send_cmd_comm_check_reply_2();
	return 0;
}
void send_cmd_comm_check_reply_2(void)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_COMM_CHECK_REPLY_2 cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_COMM_CHECK_REPLY_2);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_COMM_CHECK_REPLY_2, SEQNUM_AUTO, body_size);

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_COMM_CHECK_REPLY_2, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_COMM_CHECK_REPLY_2);
	}
}
//jhkw_200508e

void send_cmd_comm_check(void)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_COMM_CHECK cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_COMM_CHECK);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_COMM_CHECK, SEQNUM_AUTO, body_size);

	rtn = send_command((char *)&cmd, cmd_size, P1_CMD_TO_PC_COMM_CHECK, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_COMM_CHECK);
	}
}

void send_cmd_trouble_code(int code, int val, int index)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_EMG_STATUS	cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_EMG_STATUS);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_EMG_STATUS, SEQNUM_AUTO, body_size);

	cmd.code = (short int)code;
	cmd.val = (short int)val;

	if(code == P1_G_CD_FAULT_CAN_WARNNING || code == P1_G_CD_FAULT_CAN_ERROR) {
		memcpy((char *)&cmd.name[0],
			(char *)&myData->canReceiveSetData.normalData[val-1][index].name[0],
			MAX_CAN_NAME_SIZE);
	}

	userlog(DEBUG_LOG, psName, "send_cmd_trouble_code %d %d %d : %s\n",
		code, val, index, cmd.name);

	rtn = send_command((char *)&cmd, cmd_size, P1_CMD_TO_PC_EMG_STATUS, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_EMG_STATUS);
	}
}

void send_cmd_unknown(int seqno, int ret)
{ /*kjg_w
	int	rtn, cmd_size, body_size;
	S_P1_SEND_CMD_REQUEST cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_REQUEST);
	memset((char *)&cmd, 0, cmd_size);
	body_size = 0;
	userlog(DEBUG_LOG, psName, "ack1a %d\n", body_size);
	make_header((char*)&cmd, REPLY_NO, P1_CMD_TO_PC_RESPONSE, SEQNUM_AUTO,
		body_size);

	rtn = send_command((char *)&cmd, cmd_size, P1_CMD_TO_PC_RESPONSE, 1);
	if(rtn < 0)
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_REPONSE);*/
}

void send_cmd_real_time_request(void)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_REAL_TIME_REQUEST	cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_REAL_TIME_REQUEST);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_REAL_TIME_REQUEST, SEQNUM_AUTO, body_size);

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_REAL_TIME_REQUEST, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_REAL_TIME_REQUEST);
	}
}

void send_cmd_bms_comm_reply(int can_ch, int function_div)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_BMS_COMM_REPLY	cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_BMS_COMM_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
#ifdef __COA__
	if(myData->mData.config.division_CAN == 1) { //kjhw_140620
		cmd.header.reserved1 = (unsigned short)(can_ch);
	} else {
		cmd.header.reserved1 = (unsigned short)(can_ch / 2);
	}
#else
	cmd.header.reserved1 = (unsigned short)(can_ch / 2);
#endif
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_BMS_COMM_REPLY, SEQNUM_AUTO, body_size);

	cmd.function_div = (short int)function_div;

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_BMS_COMM_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_BMS_COMM_REPLY);
	}
}

void send_cmd_bms_comm_end(int can_ch, int function_div)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_BMS_COMM_END	cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_BMS_COMM_END);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
#ifdef __COA__
	if(myData->mData.config.division_CAN == 1) { //kjhw_140620
		cmd.header.reserved1 = (unsigned short)(can_ch);
	} else {
		cmd.header.reserved1 = (unsigned short)(can_ch / 2);
	}
#else
	cmd.header.reserved1 = (unsigned short)(can_ch / 2);
#endif
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_BMS_COMM_END, SEQNUM_AUTO, body_size);

	cmd.function_div = (short int)function_div;

	rtn = send_command((char *)&cmd, cmd_size, P1_CMD_TO_PC_BMS_COMM_END, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_BMS_COMM_END);
	}
}

void send_cmd_bms_eol_data1_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_BMS_EOL_DATA1_REPLY	cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_BMS_EOL_DATA1_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_BMS_EOL_DATA1_REPLY, SEQNUM_AUTO, body_size);

	memcpy((char *)&cmd.cvtn_id, (char *)&myData->mData.bms_eol_data.cvtn_id,
		sizeof(S_BMS_EOL_DATA));
	userlog(DEBUG_LOG, psName, "send_bms_eol_data1_reply\n");

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_BMS_EOL_DATA1_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_BMS_EOL_DATA1_REPLY);
	}
}

void send_cmd_bms_eol_data2_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_BMS_EOL_DATA2_REPLY	cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_BMS_EOL_DATA2_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_BMS_EOL_DATA2_REPLY, SEQNUM_AUTO, body_size);

	memcpy((char *)&cmd.cvtn_id, (char *)&myData->mData.bms_eol_data.cvtn_id,
		sizeof(S_BMS_EOL_DATA));
	userlog(DEBUG_LOG, psName, "send_bms_eol_data2_reply\n");

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_BMS_EOL_DATA2_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_BMS_EOL_DATA2_REPLY);
	}
}

void send_cmd_bms_eol_pack_id_write_reply(int val)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_BMS_EOL_PACK_ID_WRITE_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_BMS_EOL_PACK_ID_WRITE_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_BMS_EOL_PACK_ID_WRITE_REPLY, SEQNUM_AUTO, body_size);

	cmd.response = (unsigned char)val;
	userlog(DEBUG_LOG, psName, "send_bms_eol_pack_id_write_reply %d\n",
		cmd.response);

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_BMS_EOL_PACK_ID_WRITE_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_BMS_EOL_PACK_ID_WRITE_REPLY);
	}
}

void send_cmd_bms_eol_pack_id_check_reply(int val)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_BMS_EOL_PACK_ID_CHECK_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_BMS_EOL_PACK_ID_CHECK_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_BMS_EOL_PACK_ID_CHECK_REPLY, SEQNUM_AUTO, body_size);

	cmd.response = (unsigned char)val;
	memcpy((char *)&cmd.pack_id[0],
		(char *)&myData->mData.bms_eol_data2.pack_id[3][0], 40);

	userlog(DEBUG_LOG, psName, "send_bms_eol_pack_id_check_reply %d : %s\n",
		cmd.response, cmd.pack_id);

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_BMS_EOL_PACK_ID_CHECK_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_BMS_EOL_PACK_ID_CHECK_REPLY);
	}
}

void send_cmd_bms_eol_hi_pot_test_reply(int val1, int val2)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_BMS_EOL_HI_POT_TEST_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_BMS_EOL_HI_POT_TEST_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_BMS_EOL_HI_POT_TEST_REPLY, SEQNUM_AUTO, body_size);

	cmd.type = (unsigned char)val1;
	cmd.response = (unsigned char)val2;
	userlog(DEBUG_LOG, psName, "send_bms_eol_hi_pot_test_reply %d, %d\n",
		cmd.type, cmd.response);

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_BMS_EOL_HI_POT_TEST_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_BMS_EOL_HI_POT_TEST_REPLY);
	}
}

void send_cmd_uds_vbf_info_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_UDS_VBF_INFO_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_UDS_VBF_INFO_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_UDS_VBF_INFO_REPLY, SEQNUM_AUTO, body_size);

	memcpy((char *)&cmd.vbf_file_name[0],
		(char *)&myData->CAN.can_flash_file_name[0], 128);

	userlog(DEBUG_LOG, psName, "send_uds_vbf_info_reply %s\n",
		cmd.vbf_file_name);

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_UDS_VBF_INFO_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_UDS_VBF_INFO_REPLY);
	}
}

void send_cmd_uds_vbf_change_reply(int val, char *file_name)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_UDS_VBF_CHANGE_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_UDS_VBF_CHANGE_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_UDS_VBF_CHANGE_REPLY, SEQNUM_AUTO, body_size);

	cmd.response = (unsigned char)val;
	memcpy((char *)&cmd.vbf_file_name[0], file_name, 128);

	userlog(DEBUG_LOG, psName, "send_uds_vbf_change_reply %d : %s\n",
		cmd.response, cmd.vbf_file_name);

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_UDS_VBF_CHANGE_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_UDS_VBF_CHANGE_REPLY);
	}
}

void send_cmd_bms_eol_data3_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_BMS_EOL_DATA3_REPLY	cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_BMS_EOL_DATA3_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_BMS_EOL_DATA3_REPLY, SEQNUM_AUTO, body_size);

	memcpy((char *)&cmd.UBMS_State, (char *)&myData->mData.bms_eol_data3,
		sizeof(S_BMS_EOL_DATA3));
	userlog(DEBUG_LOG, psName, "send_bms_eol_data3_reply\n");

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_BMS_EOL_DATA3_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_BMS_EOL_DATA3_REPLY);
	}
}

void send_cmd_bms_eol_cvtn_id_reply(void)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_BMS_EOL_CVTN_ID_REPLY	cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_BMS_EOL_DATA1_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_BMS_EOL_CVTN_ID_REPLY, SEQNUM_AUTO, body_size);

	memcpy((char *)&cmd.cvtn_id, (char *)&myData->mData.bms_eol_data.cvtn_id,
		sizeof(S_BMS_EOL_DATA));
	userlog(DEBUG_LOG, psName, "send_bms_eol_cvtn_id_reply\n");

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_BMS_EOL_CVTN_ID_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_BMS_EOL_CVTN_ID_REPLY);
	}
}

void send_cmd_bms_eol_3p_system_reply(void)
{ //kjg_120619
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_BMS_EOL_3P_SYSTEM_REPLY	cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_BMS_EOL_3P_SYSTEM_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_BMS_EOL_3P_SYSTEM_REPLY, SEQNUM_AUTO, body_size);

	cmd.percent_input_v = myData->CAN.percent_input_v[0];
	userlog(DEBUG_LOG, psName, "send_bms_eol_3p_system_reply %f\n",
		cmd.percent_input_v);

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_BMS_EOL_3P_SYSTEM_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_BMS_EOL_3P_SYSTEM_REPLY);
	}
}

void send_cmd_daq_isolation_reply(int iso)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_DAQ_ISOLATION_REPLY	cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_DAQ_ISOLATION_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_DAQ_ISOLATION_REPLY, SEQNUM_AUTO, body_size);

	cmd.iso = (unsigned char)iso;

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_DAQ_ISOLATION_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_DAQ_ISOLATION_REPLY);
	}
}

void send_cmd_daq_isolation_reply2(int iso, int div_ch)
{ //jhkw_150224
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_DAQ_ISOLATION_REPLY2	cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_DAQ_ISOLATION_REPLY2);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_DAQ_ISOLATION_REPLY2, SEQNUM_AUTO, body_size);

	cmd.iso = (unsigned char)iso;
	cmd.div_ch = (unsigned char)div_ch;
	userlog(DEBUG_LOG, psName, "send_iso %d\n div_ch %d\n", 
			cmd.iso, cmd.div_ch);

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_DAQ_ISOLATION_REPLY2, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_DAQ_ISOLATION_REPLY2);
	}
}

void send_cmd_eol_procedure_reply(int data_index)
{ //kjg_120709
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_EOL_PROCEDURE_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_EOL_PROCEDURE_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_EOL_PROCEDURE_REPLY, SEQNUM_AUTO, body_size);

	userlog(DEBUG_LOG, psName, "send_eol_procedure %d %d %d : %d\n",
		myData->mData.eol_data[data_index].index,
		myData->mData.eol_data[data_index].sub_index,
		myData->mData.eol_data[data_index].response, data_index);

	body_size = (int)myData->mData.eol_data[data_index].index % 100;
	switch(body_size) {
		case 20: //dtc_list
			userlog(DEBUG_LOG, psName, "eol_data(1) %02x %02x %02x, %02x %02x %02x, %02x %02x %02x, %02x %02x %02x\n",
				myData->mData.eol_data[data_index].string_value[0],
				myData->mData.eol_data[data_index].string_value[1],
				myData->mData.eol_data[data_index].string_value[2],
				myData->mData.eol_data[data_index].string_value[3],
				myData->mData.eol_data[data_index].string_value[4],
				myData->mData.eol_data[data_index].string_value[5],
				myData->mData.eol_data[data_index].string_value[6],
				myData->mData.eol_data[data_index].string_value[7],
				myData->mData.eol_data[data_index].string_value[8],
				myData->mData.eol_data[data_index].string_value[9],
				myData->mData.eol_data[data_index].string_value[10],
				myData->mData.eol_data[data_index].string_value[11]
				);
			userlog(DEBUG_LOG, psName, "eol_data(2) %02x %02x %02x, %02x %02x %02x, %02x %02x %02x, %02x %02x %02x\n",
				myData->mData.eol_data[data_index].string_value[12],
				myData->mData.eol_data[data_index].string_value[13],
				myData->mData.eol_data[data_index].string_value[14],
				myData->mData.eol_data[data_index].string_value[15],
				myData->mData.eol_data[data_index].string_value[16],
				myData->mData.eol_data[data_index].string_value[17],
				myData->mData.eol_data[data_index].string_value[18],
				myData->mData.eol_data[data_index].string_value[19],
				myData->mData.eol_data[data_index].string_value[20],
				myData->mData.eol_data[data_index].string_value[21],
				myData->mData.eol_data[data_index].string_value[22],
				myData->mData.eol_data[data_index].string_value[23]
				);
			userlog(DEBUG_LOG, psName, "eol_data(3) %02x %02x %02x, %02x %02x %02x, %02x %02x %02x, %02x %02x %02x\n",
				myData->mData.eol_data[data_index].string_value[24],
				myData->mData.eol_data[data_index].string_value[25],
				myData->mData.eol_data[data_index].string_value[26],
				myData->mData.eol_data[data_index].string_value[27],
				myData->mData.eol_data[data_index].string_value[28],
				myData->mData.eol_data[data_index].string_value[29],
				myData->mData.eol_data[data_index].string_value[30],
				myData->mData.eol_data[data_index].string_value[31],
				myData->mData.eol_data[data_index].string_value[32],
				myData->mData.eol_data[data_index].string_value[33],
				myData->mData.eol_data[data_index].string_value[34],
				myData->mData.eol_data[data_index].string_value[35]
				);
			break;
		case 36: //pack_id
			userlog(DEBUG_LOG, psName, "eol_data %s\n",
				myData->mData.eol_data[data_index].string_value);
			break;
		case 54: //Pack_Part_Number
			userlog(DEBUG_LOG, psName, "eol_data %s\n",
				myData->mData.eol_data[data_index].string_value);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"eol_data long_value:%ld, %ld, %ld, %ld, float_value:%f, %f, %f, %f, byte_value:%x, %x, %x, %x\n",
				myData->mData.eol_data[data_index].long_value[0],
				myData->mData.eol_data[data_index].long_value[1],
				myData->mData.eol_data[data_index].long_value[2],
				myData->mData.eol_data[data_index].long_value[3],
				myData->mData.eol_data[data_index].float_value[0],
				myData->mData.eol_data[data_index].float_value[1],
				myData->mData.eol_data[data_index].float_value[2],
				myData->mData.eol_data[data_index].float_value[3],
				myData->mData.eol_data[data_index].byte_value[0],
				myData->mData.eol_data[data_index].byte_value[1],
				myData->mData.eol_data[data_index].byte_value[2],
				myData->mData.eol_data[data_index].byte_value[3]);
			break;
	}

	if(myData->mData.eol_data[data_index].index >= 100) {
	} else {
		memcpy((char *)&cmd.index, (char *)&myData->mData.eol_data[data_index]
			.index, sizeof(S_EOL_DATA));

		rtn = send_command((char *)&cmd, cmd_size,
			P1_CMD_TO_PC_EOL_PROCEDURE_REPLY, 1);
		if(rtn < 0) {
			userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
				P1_CMD_TO_PC_EOL_PROCEDURE_REPLY);
		}
	}
}

void send_cmd_bms_procedure_reply(int ch, int bms_type, int index, int response)
{ //kjg_161207
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_BMS_PROCEDURE_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_BMS_PROCEDURE_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_BMS_PROCEDURE_REPLY, SEQNUM_AUTO, body_size);

	cmd.ch = (unsigned char)ch+1;
	cmd.bms_type = (unsigned char)bms_type;
	cmd.index = (unsigned char)index;
	cmd.response = (unsigned char)response;

	userlog(DEBUG_LOG, psName, "send_bms_procedure ch:%d bms_type:%d index:%d response:%d\n",
		cmd.ch, cmd.bms_type, cmd.index, cmd.response);

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_BMS_PROCEDURE_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_BMS_PROCEDURE_REPLY);
	}
}

//kjhw_151021s
void send_cmd_out_mux_select_reply(int ch, int val)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_OUT_MUX_SELECT_REPLY	cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_OUT_MUX_SELECT_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_OUT_MUX_SELECT_REPLY, SEQNUM_AUTO, body_size);

	cmd.ch = (unsigned char)ch+1;
	cmd.fault = (unsigned char)val; //0:NG, 1:OK
	cmd.setting = (unsigned char)myData->cData[ch].misc.out_mux;
	cmd.state = (unsigned char)myData->cData[ch].misc.out_mux_backup;
	cmd.fault_flag = (unsigned char)myData->cData[ch].misc.out_mux_fault;
	userlog(DEBUG_LOG, psName, "out_mux_select ch:%d, fault:%d, setting:%d,
		state:%d, fault_flag:%d\n", 
		cmd.ch, cmd.fault, cmd.setting, cmd.state, cmd.fault_flag);

	sleep(1);
	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_OUT_MUX_SELECT_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_OUT_MUX_SELECT_REPLY);
	}
}
//kjhw_151021e

//kjh_160418s
void send_cmd_timesch_coa_next_step(ch)
{
	int toPs;

	S_MSG_VAL SendMsg;

	myData->testCond[ch].reserved.reserved_cmd = 0;
	myData->testCond[ch].reserved.reserved_stepNo = 0;
	myData->testCond[ch].reserved.reserved_cycleNo = 0;
	myData->testCond[ch].reserved.select_run = 0;
	myData->testCond[ch].reserved.select_stepNo = 0;
	myData->testCond[ch].reserved.select_cycleNo = 0;
	myData->testCond[ch].reserved.select_advCycleStep = 0;

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	toPs = COA1_TO_MODULE;
	SendMsg.msg = MSG_COA_MODULE_TIMESCH_CMD_NEXT_STEP;
	SendMsg.val[0] = ch;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName, "rcv_timesch_coa_cmd_next_step %d\n", ch);
}
//kjh_160418e

void send_cmd_th_table_set_reply(int val)
{ //jhkw_191108
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_TH_TABLE_SET_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_TH_TABLE_SET_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_TH_TABLE_SET_REPLY, SEQNUM_AUTO, body_size);

	cmd.th_table = (unsigned char)val;

	userlog(DEBUG_LOG, psName, "send_th_table_reply %d\n", cmd.th_table);

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_TH_TABLE_SET_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_TH_TABLE_SET_REPLY);
	}
}

//jhkw_200317s
void send_cmd_autocali_temp_reply(int val)
{
	int cmd_size, body_size, rtn;
	S_P1_SEND_CMD_AUTOCALI_TEMP_REPLY cmd;
	
	cmd_size = sizeof(S_P1_SEND_CMD_AUTOCALI_TEMP_REPLY);
	body_size = cmd_size - sizeof(S_P1_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, REPLY_NO,
		P1_CMD_TO_PC_AUTOCALI_TEMP_REPLY, SEQNUM_AUTO, body_size);

	cmd.value = (unsigned char)val;

	userlog(DEBUG_LOG, psName, "send_autocali_temp_reply %d\n", cmd.value);

	rtn = send_command((char *)&cmd, cmd_size,
		P1_CMD_TO_PC_AUTOCALI_TEMP_REPLY, 1);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d 0x%x\n", rtn,
			P1_CMD_TO_PC_AUTOCALI_TEMP_REPLY);
	}
}
//jhkw_200317e

void make_header(char *cmd, char reply, int cmd_id, int seqno, int body_size)
{
	int i, installedCh;
	unsigned long	tmp;
	S_P1_CMD_HEADER	header, tmpHeader;
	
	memset((char *)&header, 0, sizeof(S_P1_CMD_HEADER));
	
	cmd_id = (unsigned long)cmd_id;
	switch(cmd_id) {
		case P1_CMD_TO_PC_COMM_CHECK_REPLY:
			header.cmd_id = P1_CMD_TO_PC_RESPONSE;
			break;
		default:
			header.cmd_id = cmd_id;
			break;
	}

	installedCh = myData->mData.config.installedCh;
	if(installedCh > P1_MAX_CH_PER_MODULE) installedCh = P1_MAX_CH_PER_MODULE;

	switch(cmd_id) {
		case P1_CMD_TO_PC_CH_DATA:
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
		case P1_CMD_TO_PC_CH_DATA2:
		case P1_CMD_TO_PC_CH_DATA3:	//jhkw_190830
		case P1_CMD_TO_PC_PULSE_DATA:
		case P1_CMD_TO_PC_BMS_COMM_REPLY:
		case P1_CMD_TO_PC_BMS_COMM_END:
			memcpy((char *)&tmpHeader, cmd, sizeof(S_P1_CMD_HEADER));
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
		if(myPs->misc.cmd_serial > MAX_P1_CMD_SERIAL)
			myPs->misc.cmd_serial = 1;
		header.cmd_serial = myPs->misc.cmd_serial;
	} else if(seqno == SEQNUM_NONE) {
		header.cmd_serial = myPs->misc.cmd_serial;
	}

	memcpy(cmd, (char *)&header.cmd_id, sizeof(S_P1_CMD_HEADER));
	
/*	userlog(COA_LOG, psName, "header");
	for(i=0; i < sizeof(S_P1_CMD_HEADER); i++) {
		userlog2(COA_LOG, psName, " %x", *(cmd + i));
	}
	userlog2(COA_LOG, psName, ":end\n");
	userlog(COA_LOG, psName, "header %s:end\n", cmd); //kjg_d*/
}

void make_header_2(char *cmd, char reply, int cmd_id, int seqno, int body_size, int ch)
{
	int i, installedCh;
	unsigned long tmp;
	S_P1_CMD_HEADER	header;
	
	memset((char *)&header, 0, sizeof(S_P1_CMD_HEADER));
	
	cmd_id = (unsigned long)cmd_id;
	switch(cmd_id) {
		case P1_CMD_TO_PC_COMM_CHECK_REPLY:
			header.cmd_id = P1_CMD_TO_PC_RESPONSE;
			break;
		default:
			header.cmd_id = cmd_id;
			break;
	}

	installedCh = myData->mData.config.installedCh;
	if(installedCh > P1_MAX_CH_PER_MODULE) installedCh = P1_MAX_CH_PER_MODULE;

	switch(cmd_id) {
		case P1_CMD_TO_PC_PULSE_DATA:
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
		if(myPs->misc.cmd_serial > MAX_P1_CMD_SERIAL)
			myPs->misc.cmd_serial = 1;
		header.cmd_serial = myPs->misc.cmd_serial;
	} else if(seqno == SEQNUM_NONE) {
		header.cmd_serial = myPs->misc.cmd_serial;
	}

	memcpy(cmd, (char *)&header.cmd_id, sizeof(S_P1_CMD_HEADER));
	
/*	userlog(COA_LOG, psName, "header");
	for(i=0; i < sizeof(S_MAIN_CMD_HEADER); i++) {
		userlog2(COA_LOG, psName, " %x", *(cmd + i));
	}
	userlog2(COA_LOG, psName, ":end\n");
	userlog(COA_LOG, psName, "header %s:end\n", cmd); //kjg_d*/
}

int	send_command(char *cmd, int size, int cmd_id, int port_no)
{
	char	packet[MAX_P1_SEND_PACKET_LENGTH];
	unsigned char tmp, log_flag=0;
	int i, rtn, tmp2;

	if(size > MAX_P1_SEND_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName,
			"CMD SEND FAIL!! TOO LARGE SIZE[%d]\n", size);
		return -1;
	}
	
	memset((char *)&packet, 0, MAX_P1_SEND_PACKET_LENGTH);
	memcpy((char *)&packet, cmd, size);

/*	if(cmd.header.reply == REPLY_YES) {
		if(myPs->reply.timer_run == P0) {
			memset((char *)&myPs->reply.retry, 0, sizeof(S_P1_RETRY_DATA));

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
	
	if(port_no == 1) {
		rtn = writen(myPs->misc.network_socket1, packet, size);
	} else {
		rtn = writen(myPs->misc.network_socket2, packet, size);
	}

	if(myPs->config.CmdSendLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			switch(cmd_id) {
				case P1_CMD_TO_PC_COMM_CHECK_REPLY:
				case P1_CMD_TO_PC_COMM_CHECK:
				case P1_CMD_TO_PC_CH_DATA:
				case P1_CMD_TO_PC_PULSE_DATA:
				case P1_CMD_TO_PC_CH_DATA2:
				case P1_CMD_TO_PC_CH_DATA3:	//jhkw_190830
				case P1_CMD_TO_PC_AUX_INFO_REPLY:
				case P1_CMD_TO_PC_CAN_RECEIVE_INFO_REPLY:
				case P1_CMD_TO_PC_CAN_TRANSMIT_INFO_REPLY:
					break;
				case P1_CMD_TO_PC_RESPONSE:
					tmp2 = *(cmd + sizeof(S_P1_CMD_HEADER) + sizeof(int));
					if(tmp2 != P1_CD_ACK) log_flag = 1;
					break;
				default: log_flag = 1; break;
			}
		}

		if(log_flag == 1) {
			userlog(COA_LOG, psName, "sendCmd %s:end\n", packet);
		}
	}
	
	if(myPs->config.CmdSendLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			switch(cmd_id) {
				case P1_CMD_TO_PC_COMM_CHECK_REPLY:
				case P1_CMD_TO_PC_COMM_CHECK:
				case P1_CMD_TO_PC_CH_DATA:
				case P1_CMD_TO_PC_PULSE_DATA:
				case P1_CMD_TO_PC_CH_DATA2:
				case P1_CMD_TO_PC_CH_DATA3:	//jhkw_190830
				case P1_CMD_TO_PC_AUX_INFO_REPLY:
				case P1_CMD_TO_PC_CAN_RECEIVE_INFO_REPLY:
				case P1_CMD_TO_PC_CAN_TRANSMIT_INFO_REPLY:
					break;
				case P1_CMD_TO_PC_RESPONSE:
					tmp2 = *(cmd + sizeof(S_P1_CMD_HEADER) + sizeof(int));
					if(tmp2 != P1_CD_ACK) log_flag = 1;
					//log_flag = 1; //kjg_d
					break;
				default: log_flag = 1; break;
			}
		}

		if(log_flag == 1) {
			userlog(COA_LOG, psName, "sendCmd");
			for(i=0; i < size; i++) {
				tmp = *(cmd + i);
				userlog2(COA_LOG, psName, " %02x", tmp);
			}
			userlog2(COA_LOG, psName, ":end\n");
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
		case P1_CMD_TO_SBC_COMM_CHECK:	
			rtn = P1_CMD_TO_PC_RESPONSE;	
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
	//kjg_120227_w check_cmd_reply_timeout();
	if(check_network_timeout() < 0) {
		close(myPs->misc.network_socket1);
		if(myPs->misc.network_socket2 > 0) close(myPs->misc.network_socket2);
		userlog(DEBUG_LOG, psName, "network communication error3\n");
		myPs->signal[P1_SIG_NET_CONNECTED] = P0;
		return -1;
	}
	return 0;
}

void network_ping(void)
{
	long diff, time1, time2;

	if(myPs->signal[P1_SIG_NET_CONNECTED] != P1) return;

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
	
	if(myPs->signal[P1_SIG_NET_CONNECTED] != P1) return;
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
			memset((char *)&myPs->reply.retry, 0, sizeof(S_P1_RETRY_DATA));
			//kjg_w : next working
		} else {
			myPs->reply.timer_run = P1;
			myPs->reply.time = time1;
			myPs->reply.time2 = time2;
			myPs->reply.retry.count++;
				
			if(myPs->config.CmdSendLog == P1) {
				userlog(DEBUG_LOG, psName, "retry %s\n", myPs->reply.retry.buf);
			} //kjg_w hex

			rtn = writen(myPs->misc.network_socket1,
				(char *)&myPs->reply.retry.buf, myPs->reply.retry.size);
		}
	}
}

int	check_network_timeout(void)
{
	long diff, time1, time2;

	if(myPs->signal[P1_SIG_NET_CONNECTED] != P1) return 0;

	if(myData->AppControl.signal[APP_SIG_MODULE_CONTROL_PROCESS] == P1) {
		time1 = myData->mData.misc.timer_1sec;
		time2 = myData->mData.misc.timer_1000ms;
		diff = (time1 - myPs->misc.net_time) * 1000;
		diff += (time2 - myPs->misc.net_time2);
		if(diff >= myPs->config.netTimeout || diff < 0) return -1;
	}
	return 0;
}

