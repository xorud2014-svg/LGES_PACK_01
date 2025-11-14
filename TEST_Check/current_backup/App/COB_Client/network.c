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
extern volatile S_COB_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

int InitNetwork(void)
{
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

	userlog(DEBUG_LOG, psName, "command socket connected : %d\n",
		myPs->misc.network_socket);
    return 0;
}

int NetworkPacket_Receive(void)
{
	char maxPacketBuf[MAX_P2_RECV_PACKET_LENGTH];
	int rcv_size, read_size, i, start, index, toPs;
	S_MSG_VAL SendMsg;

	memset(maxPacketBuf, 0, MAX_P2_RECV_PACKET_LENGTH);
		
	if(ioctl(myPs->misc.network_socket, FIONREAD, &rcv_size) < 0) {
		userlog(DEBUG_LOG, psName, "packet receive ioctl error\n");
		close(myPs->misc.network_socket);
		return -1;
	}

	if(rcv_size > MAX_P2_RECV_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName, "max packet size over\n");
		read_size = readn(myPs->misc.network_socket, maxPacketBuf,
			MAX_P2_RECV_PACKET_LENGTH);
		if(read_size != MAX_P2_RECV_PACKET_LENGTH)
			userlog(DEBUG_LOG, psName, "packet readn size error1\n");
		close(myPs->misc.network_socket);
		return -2;
	} else if(rcv_size
		> (MAX_P2_RECV_PACKET_LENGTH - myPs->rcvPacket.usedBufSize)) {
		userlog(DEBUG_LOG, psName, "packet buffer overflow\n");
		read_size = readn(myPs->misc.network_socket, maxPacketBuf, rcv_size);
		if(read_size != rcv_size)
			userlog(DEBUG_LOG, psName, "packet readn size error2\n");
		close(myPs->misc.network_socket);
		return -3;
	} else if(rcv_size <= 0) {
		userlog(DEBUG_LOG, psName, "packet sock_rcv error %d\n", rcv_size);
		close(myPs->misc.network_socket); //kjgw
		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		toPs = COB1_TO_APP + myPs->config.groupNo;
		SendMsg.msg = MSG_COB_APP_PROCESS_KILL;
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
	
	//userlog(DEBUG_LOG, psName, "recvCmd %s\n", maxPacketBuf); //kjgd

	i = myPs->rcvPacket.rcvCount;
	myPs->rcvPacket.rcvCount++;
	if(myPs->rcvPacket.rcvCount > (MAX_P2_RECV_PACKET_COUNT-1))
		myPs->rcvPacket.rcvCount = 0;
	
	if(i == 0) index = MAX_P2_RECV_PACKET_COUNT - 1;
	else index = i - 1;
	start = myPs->rcvPacket.rcvStartPoint[index]
		+ myPs->rcvPacket.rcvSize[index];
	if(start >= MAX_P2_RECV_PACKET_LENGTH) {
		myPs->rcvPacket.rcvStartPoint[i]
			= abs(start - MAX_P2_RECV_PACKET_LENGTH);
	} else {
		myPs->rcvPacket.rcvStartPoint[i] = start;
	}

	myPs->rcvPacket.rcvSize[i] = read_size;
	myPs->rcvPacket.usedBufSize += read_size;
	
	start = myPs->rcvPacket.rcvStartPoint[i];
	if((start + read_size) > MAX_P2_RECV_PACKET_LENGTH) {
		index = MAX_P2_RECV_PACKET_LENGTH - start;
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
	//char debug[MAX_P2_RECV_PACKET_LENGTH]; //kjgd
	int i, j, k, cmdBuf_index, start_point;
	
	if(myPs->rcvPacket.rcvCount == myPs->rcvPacket.parseCount) return;
	
	i = myPs->rcvPacket.parseCount;
	myPs->rcvPacket.parseCount++;
	if(myPs->rcvPacket.parseCount > (MAX_P2_RECV_PACKET_COUNT-1))
		myPs->rcvPacket.parseCount = 0;
	
	cmdBuf_index = myPs->rcvCmd.cmdBufSize;
	myPs->rcvCmd.cmdBufSize += myPs->rcvPacket.rcvSize[i];
	
	start_point = myPs->rcvPacket.parseStartPoint[i];

/*	userlog(COB_LOG, psName, "recvCmd1 %s:end %d %d\n", 
		myPs->rcvCmd.cmdBuf, i, start_point);
		
	memset(debug, 0, sizeof debug);
	memcpy((char *)&debug, (char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
		myPs->rcvPacket.rcvSize[i]);
	userlog(COB_LOG, psName, "recvCmd2 %s:end %d %d\n",
		debug, cmdBuf_index, myPs->rcvPacket.rcvSize[i]); //kjgd */

	j = start_point + myPs->rcvPacket.rcvSize[i];
	if(j <= MAX_P2_RECV_PACKET_LENGTH) {
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point],
			myPs->rcvPacket.rcvSize[i]);
	} else {
		k = MAX_P2_RECV_PACKET_LENGTH - start_point;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
		cmdBuf_index += k;
		start_point = 0;
		k = j - MAX_P2_RECV_PACKET_LENGTH;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
	}
		
	start_point = myPs->rcvPacket.parseStartPoint[i]
		+ myPs->rcvPacket.rcvSize[i];
	if(start_point >= MAX_P2_RECV_PACKET_LENGTH) {
		j = i + 1;
		if(j >= MAX_P2_RECV_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j]
			= abs(start_point - MAX_P2_RECV_PACKET_LENGTH);
	} else {
		j = i + 1;
		if(j >= MAX_P2_RECV_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j] = start_point;
	}
		
	myPs->rcvPacket.usedBufSize -= myPs->rcvPacket.rcvSize[i];
	
/*	userlog(COB_LOG, psName, "recvCmd3 %d %d\n", myPs->rcvPacket.usedBufSize,
		myPs->rcvCmd.cmdBufSize); //kjgd*/
}

int NetworkCommand_Receive(void)
{
	//char debug[MAX_P2_RECV_PACKET_LENGTH]; //kjgd
	int cmd_size, cmdBuf_index;
	S_P2_CMD_HEADER header;

	if(myPs->rcvCmd.cmdBufSize < sizeof(S_P2_CMD_HEADER)) return -1;
	
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmdBuf,
		sizeof(S_P2_CMD_HEADER));
	cmd_size = header.cmd_size;
	if(myPs->rcvCmd.cmdBufSize < cmd_size) return -2;
			
/*	userlog(COB_LOG, psName, "recvCmd4 %s:end %d\n",
		myPs->rcvCmd.cmdBuf, cmd_size); //kjgd*/
	
	memset((char *)&myPs->rcvCmd.cmd, 0, MAX_P2_RECV_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmd, (char *)&myPs->rcvCmd.cmdBuf, cmd_size);
	myPs->rcvCmd.cmdSize = cmd_size;
	
	cmdBuf_index = cmd_size;
	myPs->rcvCmd.cmdBufSize -= cmd_size;
	cmd_size = myPs->rcvCmd.cmdBufSize;
	memset((char *)&myPs->rcvCmd.tmpBuf, 0, MAX_P2_RECV_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.tmpBuf,
		(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
	memset((char *)&myPs->rcvCmd.cmdBuf, 0, MAX_P2_RECV_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmdBuf,
		(char *)&myPs->rcvCmd.tmpBuf, cmd_size);
	
/*	memset(debug, 0, sizeof debug);
	memcpy((char *)&debug, (char *)&myPs->rcvCmd.cmdBuf, cmd_size);
	userlog(COB_LOG, psName, "recvCmd5 %s:end %d %d\n",
		debug, cmd_size, cmdBuf_index); //kjgd*/

	return 0;
}

int NetworkCommand_Parsing(void)
{
	unsigned char tmp, log_flag;
	int	rtn, i, group;
	S_P2_CMD_HEADER header;
	S_P2_RCV_CMD_RESPONSE	cmd;

	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd, sizeof(S_P2_CMD_HEADER));
	
	if(myPs->config.CmdRcvLog == P1) {
		log_flag = 0;
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			switch(header.cmd_id) {
				case P2_CMD_TO_SBC_COMM_CHECK:
					break;
				case P2_CMD_TO_SBC_RESPONSE:
					memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_RESPONSE));
					memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
						sizeof(S_P2_RCV_CMD_RESPONSE));
					if(cmd.code != P2_CD_ACK) log_flag = 1;
					break;
				default: log_flag = 1; break;
			}
		}

		if(log_flag == 1) {
			userlog(COB_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		}
	}
	
	if(myPs->config.CmdRcvLog_Hex == P1) {
		log_flag = 0;
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			switch(header.cmd_id) {
				case P2_CMD_TO_SBC_COMM_CHECK:
					break;
				case P2_CMD_TO_SBC_RESPONSE:
					memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_RESPONSE));
					memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
						sizeof(S_P2_RCV_CMD_RESPONSE));
					if(cmd.code != P2_CD_ACK) log_flag = 1;
					//log_flag = 1; //kjgd
					break;
				default: log_flag = 1; break;
			}
		}

		if(log_flag == 1) {
			userlog(COB_LOG, psName, "recvCmd");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				tmp = myPs->rcvCmd.cmd[i];
				userlog2(COB_LOG, psName, " %02x", tmp);
			}
			userlog2(COB_LOG, psName, ":end\n");
		}
	}
	
	rtn = CmdHeader_Check((char *)&header);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "CmdHeader_Check error %d\n", rtn);
		return -1;
	}
	
/*	rtn = Check_ReplyCmd((char *)&header);
	if(rtn < 0) return -2; //kjgw*/

	group = (int)myPs->config.groupNo;

	switch(header.cmd_id) {
		case P2_CMD_TO_SBC_VERSION_REQUEST:
			rtn = rcv_cmd_version_request(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_version_request error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_VERSION_REQUEST2:
			rtn = rcv_cmd_version_request2(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_version_request2 error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_SET_GROUP:
			rtn = rcv_cmd_set_group(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_set_group error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_RESPONSE:
			rtn = rcv_cmd_response(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_response error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_LINE_MODE_REQUEST:
			rtn = rcv_cmd_line_mode_request(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_line_mode_request error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_SET_AUTO_REPORT:
			rtn = rcv_cmd_set_auto_report(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_set_auto_report error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_TEST_HEADER:
			rtn = rcv_cmd_test_header(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_test_header error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_TEST_PRECHECK:
			rtn = rcv_cmd_test_precheck(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_test_precheck error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_TEST_COMMON_SAFETY:
			rtn = rcv_cmd_test_common_safety(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_test_common_safety error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_TEST_NG_COND:
			rtn = rcv_cmd_test_ng_condition(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_test_ng_condition error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_TEST_STEP:
			rtn = rcv_cmd_test_step(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_test_step error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_TEST_GRADE:
			rtn = rcv_cmd_test_grade(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_test_grade error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_TEST_INFO_REQUEST:
			rtn = rcv_cmd_test_info_request(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_test_info_request error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_SET_SENSOR_LIMIT:
			rtn = rcv_cmd_set_sensor_limit(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_set_sensor_limit error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_TRAY_DATA:
			rtn = rcv_cmd_tray_data(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_tray_data error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_CHECK:
			rtn = rcv_cmd_check(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_check error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_RUN:
			rtn = rcv_cmd_run(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_run error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_STOP:
			rtn = rcv_cmd_stop(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_stop error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_PAUSE:
			rtn = rcv_cmd_pause(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_pause error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_CONTINUE:
			rtn = rcv_cmd_continue(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_continue error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_NEXT_STEP:
			rtn = rcv_cmd_next_step(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_next_step error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_RESET:
			rtn = rcv_cmd_reset(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_reset error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_CLEAR:
			rtn = rcv_cmd_clear(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_clear error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_CONFIG_REQUEST:
			rtn = rcv_cmd_config_request(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_config_request error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_SET_CONFIG:
			rtn = rcv_cmd_set_config(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_set_config error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_COMM_CHECK:
			rtn = rcv_cmd_comm_check(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_comm_check error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_SET_LINE_MODE:
			rtn = rcv_cmd_set_line_mode(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_set_line_mode error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_USER_CMD:
			rtn = rcv_cmd_user_cmd(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_user_cmd error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_SET_TRAY_READY:
			rtn = rcv_cmd_set_tray_ready(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_set_tray_ready error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_CHAMBER_SET:
			rtn = rcv_cmd_chamber_set(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_chamber_set error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_JOB_CHANGE_SET:
			rtn = rcv_cmd_job_change_set(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_job_change_set error %d\n", rtn);
			}
			break;

		//kjg_110712
		case P2_CMD_TO_SBC_SET_CALI_POINT:
			rtn = rcv_cmd_set_cali_point(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_set_cali_point error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_GET_MAIN_CALI_POINT:
			rtn = rcv_cmd_get_main_cali_point(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_get_main_cali_point error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_GET_CH_CALI_POINT:
			rtn = rcv_cmd_get_ch_cali_point(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_get_ch_cali_point error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_CALI_START:
			rtn = rcv_cmd_cali_start(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_cali_start error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_CALI_CHECK_START:
			rtn = rcv_cmd_cali_check_start(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_cali_check_start error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_CALI_STOP:
			rtn = rcv_cmd_cali_stop(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_cali_stop error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_CALI_PAUSE:
			rtn = rcv_cmd_cali_pause(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_cali_pause error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_CALI_RESUME:
			rtn = rcv_cmd_cali_resume(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_cali_resume error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_CALI_UPDATE:
			rtn = rcv_cmd_cali_update(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_cali_update error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_REAL_MEASURE_START:
			rtn = rcv_cmd_real_measure_start(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_real_measure_start error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_REAL_MEASURE_STOP:
			rtn = rcv_cmd_real_measure_stop(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_real_measure_stop error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_HW_MAP_REQ:
			rtn = rcv_cmd_hw_map_req(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_hw_map_req error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_JIG_TEMP_SET_DATA:
			rtn = rcv_cmd_jig_temp_set_data(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_jig_temp_set_data error %d\n", rtn);
			}
			break;
		case P2_CMD_TO_SBC_CH_ATTRIBUTE_SET:
			rtn = rcv_cmd_ch_attribute_set(group);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"rcv_cmd_ch_attribute_set error %d\n", rtn);
			}
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
			if(myPs->rcvCmd.cmdFail >= 3) {
				myPs->rcvCmd.cmdFail = 0;
				myPs->rcvCmd.cmdBufSize = 0;
				memset((char *)&myPs->rcvCmd.cmdBuf, 0,
					MAX_P2_RECV_PACKET_LENGTH);
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
	int length, skip;
	S_P2_CMD_HEADER	header;
	
	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_P2_CMD_HEADER));
	
	skip = 0; length = 0;
	switch(header.cmd_id) {
		case P2_CMD_TO_SBC_VERSION_REQUEST:
			length = sizeof(S_P2_RCV_CMD_VERSION_REQUEST); 
			break;
		case P2_CMD_TO_SBC_VERSION_REQUEST2:
			length = sizeof(S_P2_RCV_CMD_VERSION_REQUEST); 
			break;
		case P2_CMD_TO_SBC_SET_GROUP:
			length = sizeof(S_P2_RCV_CMD_SET_GROUP); 
			break;
		case P2_CMD_TO_SBC_RESPONSE:
			length = sizeof(S_P2_RCV_CMD_RESPONSE); 
			break;
		case P2_CMD_TO_SBC_TEST_HEADER:
			length = sizeof(S_P2_RCV_CMD_TEST_HEADER); 
			break;
		case P2_CMD_TO_SBC_SET_LINE_MODE:
			length = sizeof(S_P2_RCV_CMD_SET_LINE_MODE); 
			break;
		case P2_CMD_TO_SBC_TEST_PRECHECK:
			if(myPs->config.protocol_version == P2_PROTOCOL_VERSION) {
				length = sizeof(S_P2_RCV_CMD_TEST_PRECHECK);
			} else {
				length = sizeof(S_P2_RCV_CMD_TEST_PRECHECK2);
			}
			break;
		case P2_CMD_TO_SBC_TEST_COMMON_SAFETY:
			length = sizeof(S_P2_RCV_CMD_TEST_COMMON_SAFETY);
			break;
		case P2_CMD_TO_SBC_TEST_NG_COND:
			length = sizeof(S_P2_RCV_CMD_TEST_NG_COND);
			break;
		case P2_CMD_TO_SBC_TEST_STEP:
			//kjgw length = sizeof(S_P2_RCV_CMD_TEST_STEP); 
			skip = 1;
			break;
		case P2_CMD_TO_SBC_TEST_GRADE:
			//kjgw length = sizeof(S_P2_RCV_CMD_TEST_GRADE); 
			skip = 1;
			break;
		case P2_CMD_TO_SBC_TEST_INFO_REQUEST:
			length = sizeof(S_P2_RCV_CMD_TEST_INFO_REQUEST); 
			break;
		case P2_CMD_TO_SBC_SET_SENSOR_LIMIT:
			length = sizeof(S_P2_RCV_CMD_SET_SENSOR_LIMIT);
			break;
		case P2_CMD_TO_SBC_CHECK:
			length = sizeof(S_P2_RCV_CMD_CHECK); 
			break;
		case P2_CMD_TO_SBC_RUN:
			length = sizeof(S_P2_RCV_CMD_RUN); 
			break;
		case P2_CMD_TO_SBC_STOP:
			length = sizeof(S_P2_RCV_CMD_STOP); 
			break;
		case P2_CMD_TO_SBC_PAUSE:
			length = sizeof(S_P2_RCV_CMD_PAUSE); 
			break;
		case P2_CMD_TO_SBC_CONTINUE:
			length = sizeof(S_P2_RCV_CMD_CONTINUE); 
			break;
		case P2_CMD_TO_SBC_NEXT_STEP:
			length = sizeof(S_P2_RCV_CMD_NEXT_STEP); 
			break;
		case P2_CMD_TO_SBC_RESET:
			length = sizeof(S_P2_RCV_CMD_RESET); 
			break;
		case P2_CMD_TO_SBC_CLEAR:
			length = sizeof(S_P2_RCV_CMD_CLEAR); 
			break;
		case P2_CMD_TO_SBC_TRAY_DATA:
			length = sizeof(S_P2_RCV_CMD_TRAY_DATA); 
			break;
		case P2_CMD_TO_SBC_LINE_MODE_REQUEST:
			length = sizeof(S_P2_RCV_CMD_LINE_MODE_REQUEST); 
			break;
		case P2_CMD_TO_SBC_SET_AUTO_REPORT:
			length = sizeof(S_P2_RCV_CMD_SET_AUTO_REPORT); 
			break;
		case P2_CMD_TO_SBC_CONFIG_REQUEST:
			length = sizeof(S_P2_RCV_CMD_CONFIG_REQUEST); 
			break;
		case P2_CMD_TO_SBC_SET_CONFIG:
			length = sizeof(S_P2_RCV_CMD_SET_CONFIG); 
			break;
		case P2_CMD_TO_SBC_SET_TRAY_READY:
			length = sizeof(S_P2_RCV_CMD_SET_TRAY_READY); 
			break;
		case P2_CMD_TO_SBC_USER_CMD:
			length = sizeof(S_P2_RCV_CMD_USER_CMD); 
			break;
		case P2_CMD_TO_SBC_CHAMBER_SET:
			length = sizeof(S_P2_RCV_CMD_CHAMBER_SET);
			break;
		case P2_CMD_TO_SBC_JOB_CHANGE_SET:
			length = sizeof(S_P2_RCV_CMD_JOB_CHANGE_SET);
			break;

		//kjg_110712
		case P2_CMD_TO_SBC_SET_CALI_POINT:
			length = sizeof(S_P2_RCV_CMD_SET_CALI_POINT);
			break;
		case P2_CMD_TO_SBC_GET_MAIN_CALI_POINT:
			length = sizeof(S_P2_RCV_CMD_GET_MAIN_CALI_POINT);
			break;
		case P2_CMD_TO_SBC_GET_CH_CALI_POINT:
			length = sizeof(S_P2_RCV_CMD_GET_CH_CALI_POINT);
			break;
		case P2_CMD_TO_SBC_CALI_STOP:
			length = sizeof(S_P2_RCV_CMD_CALI_STOP);
			break;
		case P2_CMD_TO_SBC_CALI_PAUSE:
			length = sizeof(S_P2_RCV_CMD_CALI_PAUSE);
			break;
		case P2_CMD_TO_SBC_CALI_START:
			length = sizeof(S_P2_RCV_CMD_CALI_START);
			break;
		case P2_CMD_TO_SBC_CALI_CHECK_START:
			length = sizeof(S_P2_RCV_CMD_CALI_CHECK_START);
			break;
		case P2_CMD_TO_SBC_CALI_UPDATE:
			length = sizeof(S_P2_RCV_CMD_CALI_UPDATE);
			break;
		case P2_CMD_TO_SBC_REAL_MEASURE_START:
			length = sizeof(S_P2_RCV_CMD_REAL_MEASURE_START);
			break;
		case P2_CMD_TO_SBC_REAL_MEASURE_STOP:
			length = sizeof(S_P2_RCV_CMD_REAL_MEASURE_STOP);
			break;
		case P2_CMD_TO_SBC_HW_MAP_REQ:
			length = sizeof(S_P2_RCV_CMD_HW_MAP_REQ);
			break;
		case P2_CMD_TO_SBC_JIG_TEMP_SET_DATA:
			length = sizeof(S_P2_RCV_CMD_JIG_TEMP_SET_DATA);
			break;
		case P2_CMD_TO_SBC_CH_ATTRIBUTE_SET:
			length = sizeof(S_P2_RCV_CMD_CH_ATTRIBUTE_SET);
			break;
		default:
			//kjgw send_cmd_unknown(header.cmd_id, P2_CMD_ID_ERROR);
			userlog(DEBUG_LOG, psName, "RcvCmd command id error %x\n",
				header.cmd_id);
			return -1;
	}
	
	if(header.packet_id != PACKET_P2_HOST) {
		userlog(DEBUG_LOG, psName,
			"RcvCmd(0x%x) packet id error %d\n",
			header.cmd_id, header.packet_id);
		send_cmd_response((char *)&header, P2_CD_PACKET_ID_ERROR);
		return -2;
	}

	if(header.cmd_id != P2_CMD_TO_SBC_RESPONSE
		&& header.cmd_id != P2_CMD_TO_SBC_VERSION_REQUEST
		&& header.cmd_id != P2_CMD_TO_SBC_VERSION_REQUEST2
		&& header.cmd_id != P2_CMD_TO_SBC_SET_GROUP) {
		//if(header.group_id != (unsigned short)myPs->config.groupId) {
		if(header.group_id != (unsigned short)myPs->config.groupNo+1) {
			userlog(DEBUG_LOG, psName, "RcvCmd(0x%x) group id error %d\n",
				header.cmd_id, header.group_id);
			send_cmd_response((char *)&header, P2_CD_GP_ID_ERROR);
			return -3;
		}
	}

	if(skip == 0 && length != myPs->rcvCmd.cmdSize) {
		//kjgw send_cmd_unknown(header.cmd_id, P2_CD_SIZE_ERROR);
		userlog(DEBUG_LOG, psName, "RcvCmd(0x%x) size error length:%d rcv:%d\n",
			header.cmd_id, length, myPs->rcvCmd.cmdSize);
		return -4;
	}
/*kjgw
	if(header.cmd_serial > MAX_P2_CMD_SERIAL) {
		send_cmd_unknown(header.cmd_id, P2_CD_SEQ_NO_ERROR); //kjgw
		userlog(DEBUG_LOG, psName, "RcvCmd(0x%x) sequence no error : %d\n",
			header.cmd_id, header.cmd_serial);
		return -5;
	}*/

	return 0;
}

int Check_ReplyCmd(char *rcvHeader)
{
	int rtn; //, seq_no;
	S_P2_CMD_HEADER header;

	memcpy((char *)&header, (char *)rcvHeader, sizeof(S_P2_CMD_HEADER));
	
	rtn = 0;
	/*memset(buf, 0, sizeof buf);
	strncpy(buf, (char *)&header.seqno, sizeof(S_P2_CMD_HEADER));
	seqno = atoi(buf); kjgw*/
	
	if(myPs->reply.timer_run == P1) {
		/*if(myPs->reply.retry.seqno == seqno
			&& kjgw*/
		if(myPs->reply.retry.replyCmd == header.cmd_id) {
			myPs->reply.timer_run = P0;
			memset((char *)&myPs->reply.retry, 0, sizeof(S_P2_RETRY_DATA));
		}
	}
	return rtn;
}

int rcv_cmd_version_request(int group)
{
	S_P2_RCV_CMD_VERSION_REQUEST cmd;

	userlog(DEBUG_LOG, psName, "rcv_cmd_version_request\n");

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_VERSION_REQUEST));

	return send_cmd_version_data((char *)&cmd.header);
}

int rcv_cmd_version_request2(int group)
{
	S_P2_RCV_CMD_VERSION_REQUEST cmd;

	userlog(DEBUG_LOG, psName, "rcv_cmd_version_request2\n");

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_VERSION_REQUEST));

	if(myPs->signal[P2_SIG_RCV_CH_ATTRIBUTE_SET] == P2) {
		myPs->signal[P2_SIG_RCV_CH_ATTRIBUTE_SET]++;
	}

	return send_cmd_version_data2((char *)&cmd.header);
}

int rcv_cmd_set_group(int group)
{
	S_P2_RCV_CMD_SET_GROUP	cmd;

	userlog(DEBUG_LOG, psName, "rcv_cmd_set_group\n");

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_SET_GROUP));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_SET_GROUP));

	if(cmd.netConnectApproval == P1) {
		myPs->config.autoProcess = cmd.nvRamFlag;
		myPs->config.useTempLimitFlag = cmd.useTemp;
		myPs->config.useGasLimitFlag = cmd.useGas;
		myPs->config.maxTempLimit = cmd.maxTemp;
		myPs->config.maxGasLimit = cmd.maxGas;
		myPs->config.send_monitor_data_interval
			= (long)cmd.autoReportInterval * 1000;
		myPs->config.trayCodeReadType = cmd.trayReadType;

		myPs->misc.net_time = myData->mData.misc.timer_1sec;
		myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
		myPs->signal[P2_SIG_NET_CONNECT_DISAPPROVAL] = P0;
		myPs->signal[P2_SIG_SEND_GROUP_STATE] = P1;
	} else {
		userlog(DEBUG_LOG, psName, "Network Connect Disapproval !!!\n");
		myPs->signal[P2_SIG_NET_CONNECTED] = P0;
		myPs->signal[P2_SIG_NET_CONNECT_DISAPPROVAL] = P1;
	}
	
	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_set_line_mode(int group)
{
	int rtn, toPs;
	S_P2_RCV_CMD_SET_LINE_MODE	cmd;
	S_MSG_VAL SendMsg;
	
	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_SET_LINE_MODE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_SET_LINE_MODE));
	
	switch(cmd.onlineMode) {
		case P2_WORK_ONLINE:
			myPs->config.workMode = P2_WORK_ONLINE;
			break;	
		case P2_WORK_OFFLINE:
			myPs->config.workMode = P2_WORK_OFFLINE;
			break;	
		default: break;
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_WORK_MODE;
	SendMsg.val[0] = (int)myPs->config.workMode;
	send_msg(toPs, (char *)&SendMsg);
	
	rtn = Write_WorkMode();
	if(rtn < 0){
		userlog(DEBUG_LOG, psName, "WorkMode write fail !!!!\n");
	}

	rtn = 0;	
	switch(cmd.controlMode) {
		case P2_WORK_MAINTENANCE:
			myPs->config.groupControlMode = P2_WORK_MAINTENANCE;
			break;
		case P2_WORK_CONTROL:
			myPs->config.groupControlMode = P2_WORK_CONTROL;
//			rtn = 1;
			break;
		default: break;
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CONTROL_MODE;
	SendMsg.val[0] = (int)myPs->config.groupControlMode;
	send_msg(toPs, (char *)&SendMsg);

/*	
	if(rtn == 1) { //maintenance -> control
		rtn = 0;
		if(myData->LGClient[group].state == BOX_PAUSE) {
		} else if(myData->LGClient[group].state == BOX_TROUBLE) {
			switch(myData->LGClient[group].microState) {
				case P6:
					myData->LGClient[group].state = BOX_IDLE;
					myData->LGClient[group].microState = P16;
					send_msg(SUB_TO_MODULE, MSG_SUB_MODULE_JIG_TRY_CONTACT,
						group, 0);
					break;
				case P9:
					myData->LGClient[group].state = BOX_RUN;
					myData->LGClient[group].microState = P5;
					send_msg(SUB_TO_MODULE, MSG_SUB_MODULE_JIG_TRY_DISCONTACT,
						group, 0);
					break;
				default: break;
			}
		}
	}
*/		
	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_line_mode_request(int group)
{
	S_P2_RCV_CMD_LINE_MODE_REQUEST	cmd;

	userlog(DEBUG_LOG, psName, "rcv_cmd_line_mode_request\n");

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_LINE_MODE_REQUEST));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_LINE_MODE_REQUEST));
	
	return send_cmd_line_mode_data((char *)&cmd.header);
}

int rcv_cmd_set_auto_report(int group)
{
	S_P2_RCV_CMD_SET_AUTO_REPORT	cmd;

	userlog(DEBUG_LOG, psName, "rcv_cmd_set_auto_report\n");

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_SET_AUTO_REPORT));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_SET_AUTO_REPORT));

	myPs->config.send_monitor_data_interval = (long)cmd.interval * 1000;
	myPs->signal[P2_SIG_RCV_AUTO_REPORT_DATA] = P1;
	myPs->signal[P2_SIG_NET_CONNECTED] = P1;
	
	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_config_request(int group)
{
	S_P2_RCV_CMD_CONFIG_REQUEST	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_CONFIG_REQUEST));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_CONFIG_REQUEST));
	
	return send_cmd_config_data((char *)&cmd.header);
}

int rcv_cmd_set_config(int group)
{
	int rtn;
	S_P2_RCV_CMD_SET_CONFIG	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_SET_CONFIG));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_SET_CONFIG));

	memcpy((char *)&myPs->configData, (char *)&cmd.configData,
		sizeof(S_P2_CONFIG_DATA));

	rtn = Write_COB_ConfigData();
	if(rtn < 0) rtn = P2_CD_NACK; //kjgw
	else rtn = P2_CD_ACK;

	return send_cmd_response((char *)&cmd.header, rtn);
}

int rcv_cmd_test_header(int group)
{
	S_P2_RCV_CMD_TEST_HEADER	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_HEADER));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_HEADER));

	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myData->gData[group].state != G_STANDBY) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_STATE_ERROR);
	}
	
	if(cmd.testHeader.totalStep > MAX_P2_STEP) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_TEST_STEP_COUNT_ERROR);
	}
	
	memset((char *)&myPs->testCond, 0, sizeof(S_P2_TEST_CONDITION));
	memcpy((char *)&myPs->testCond.header, (char *)&cmd.testHeader,
		sizeof(S_P2_TEST_HEADER));

	myPs->misc.stepCount = 0;
	myPs->misc.gradeCount = 0;
	myPs->signal[P2_SIG_TEST_COND_RCV] = P1;

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_test_precheck(int group)
{
	int rtn;

	if(myPs->config.protocol_version == P2_PROTOCOL_VERSION) {
		rtn = rcv_cmd_test_precheck_1(group);
	} else {
		rtn = rcv_cmd_test_precheck_2(group);
	}

	return rtn;
}

int rcv_cmd_test_precheck_1(int group)
{
	S_P2_RCV_CMD_TEST_PRECHECK	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_PRECHECK));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_PRECHECK));
	
	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myData->gData[group].state != G_STANDBY) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_STATE_ERROR);
	}

	if(myPs->signal[P2_SIG_TEST_COND_RCV] != P1) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_TEST_HEADER_UNRCV);
	}
	
	memcpy((char *)&myPs->testCond.precheck,
		(char *)&cmd.precheck, sizeof(S_P2_TEST_PRECHECK));

//	userlog(DEBUG_LOG, psName, "rcv_cmd_test_precheck\n"); //kjgd

	myPs->signal[P2_SIG_TEST_COND_RCV] = P2;

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_test_precheck_2(int group)
{
	S_P2_RCV_CMD_TEST_PRECHECK2	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_PRECHECK2));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_PRECHECK2));
	
	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myData->gData[group].state != G_STANDBY) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_STATE_ERROR);
	}

	if(myPs->signal[P2_SIG_TEST_COND_RCV] != P1) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_TEST_HEADER_UNRCV);
	}
	
	memcpy((char *)&myPs->testCond.precheck2,
		(char *)&cmd.precheck2, sizeof(S_P2_TEST_PRECHECK2));

//	userlog(DEBUG_LOG, psName, "rcv_cmd_test_precheck2\n"); //kjgd

	myPs->signal[P2_SIG_TEST_COND_RCV] = P2;

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_test_common_safety(int group)
{
	S_P2_RCV_CMD_TEST_COMMON_SAFETY cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_COMMON_SAFETY));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_COMMON_SAFETY));
	
	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myData->gData[group].state != G_STANDBY) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_STATE_ERROR);
	}

	if(myPs->signal[P2_SIG_TEST_COND_RCV] != P2) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_TEST_PRECHECK_UNRCV);
	}

	memcpy((char *)&myPs->testCond.common_safety,
		(char *)&cmd.common_safety, sizeof(S_P2_TEST_COMMON_SAFETY));

//	userlog(DEBUG_LOG, psName, "rcv_cmd_test_common_safety\n"); //kjgd

	myPs->signal[P2_SIG_TEST_COND_RCV] = P3;

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_test_ng_condition(int group)
{
	int stepNo;
	S_P2_RCV_CMD_TEST_NG_COND cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_NG_COND));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_NG_COND));
	
	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myData->gData[group].state != G_STANDBY) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_STATE_ERROR);
	}

	if(myPs->signal[P2_SIG_TEST_COND_RCV] != P3
		&& myPs->signal[P2_SIG_TEST_COND_RCV] != P4) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_TEST_COMMON_SAFETY_UNRCV);
	}

	stepNo = (int)cmd.ng_cond.stepNo;
	memcpy((char *)&myPs->testCond.ng_cond[stepNo],
		(char *)&cmd.ng_cond, sizeof(S_P2_TEST_NG_COND));

//	userlog(DEBUG_LOG, psName, "rcv_cmd_test_ng_cond(%d)\n", stepNo); //kjgd

	myPs->signal[P2_SIG_TEST_COND_RCV] = P4;

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_test_step(int group)
{
	int count, rtn;
	S_P2_RCV_CMD_TEST_STEP_HEADER	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_STEP_HEADER));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_STEP_HEADER));
	
	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myData->gData[group].state != G_STANDBY) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_STATE_ERROR);
	}

	if(myPs->config.protocol_version == P2_PROTOCOL_VERSION) {
		if(myPs->signal[P2_SIG_TEST_COND_RCV] != P2) {
			return send_cmd_response((char *)&cmd.header,
				P2_CD_GP_TEST_PRECHECK_UNRCV);
		}
	} else {
		if(myPs->signal[P2_SIG_TEST_COND_RCV] != P4) {
			return send_cmd_response((char *)&cmd.header,
				P2_CD_GP_TEST_NG_COND_UNRCV);
		}
	}
	
	count = myPs->misc.stepCount;
	if(count != cmd.stepHeader.stepNo || count >= (MAX_P2_STEP-1)) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_TEST_STEP_COUNT_ERROR);
	}
	
	memcpy((char *)&myPs->testCond.stepHeader[count],
		(char *)&cmd.stepHeader, sizeof(S_P2_TEST_STEP_HEADER));

	rtn = 0;
	switch(cmd.stepHeader.type) {
		case P2_STEP_OCV:
			rtn = get_test_step_ocv(count);
			break;
		case P2_STEP_CHARGE:
			rtn = get_test_step_charge(count);
			break;
		case P2_STEP_DISCHARGE:
			rtn = get_test_step_discharge(count);
			break;
		case P2_STEP_Z:
			rtn = get_test_step_z(count);
			break;
		case P2_STEP_REST:
			rtn = get_test_step_rest(count);
			break;
		case P2_STEP_LONG_TIME_REST:
			rtn = get_test_step_long_time_rest(count);
			break;
		case P2_STEP_END:
			rtn = get_test_step_end(count);
			break;
		default:
			return send_cmd_response((char *)&cmd.header,
				P2_CD_GP_TEST_STEP_TYPE_ERROR);
			break;
	}

	if(rtn < 0) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_TEST_STEP_DATA_ERROR);
	}

	//userlog(DEBUG_LOG, psName, "rcv_cmd_step stepCount:%d, totalStep:%d\n",
	//	myPs->misc.stepCount, myPs->testCond.header.totalStep); //kjgd

	myPs->misc.stepCount++;
	if(myPs->misc.stepCount == myPs->testCond.header.totalStep
		&& myPs->misc.gradeCount == myPs->testCond.header.totalGrade) {
		myPs->signal[P2_SIG_TEST_COND_RCV] = P0;
		convert_test_cond();
		myPs->misc.state = P2_G_STANDBY;
	}

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int get_test_step_ocv(int count)
{
	S_P2_RCV_CMD_TEST_STEP_OCV	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_STEP_OCV));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_STEP_OCV));
	
	memcpy((char *)&myPs->testCond.ocv[count],
		(char *)&cmd.ocv, sizeof(S_P2_TEST_STEP_OCV));

	return 0;
}

int get_test_step_charge(int count)
{
	S_P2_RCV_CMD_TEST_STEP_CHARGE	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_STEP_CHARGE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_STEP_CHARGE));
	
	memcpy((char *)&myPs->testCond.charge[count],
		(char *)&cmd.charge, sizeof(S_P2_TEST_STEP_CHARGE));
#if 0
	userlog(DEBUG_LOG, psName, "charge %d %ld %ld %ld %ld %ld %ld\n", count,
		myPs->testCond.charge[count].refV, 
		myPs->testCond.charge[count].refI,
		myPs->testCond.charge[count].recodeDeltaTime,
		myPs->testCond.charge[count].recodeDeltaT,
		myPs->testCond.charge[count].delta_V1,
		myPs->testCond.charge[count].delta_V2);
#endif	
	return 0;
}

int get_test_step_discharge(int count)
{
	S_P2_RCV_CMD_TEST_STEP_DISCHARGE	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_STEP_DISCHARGE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_STEP_DISCHARGE));
	
	memcpy((char *)&myPs->testCond.discharge[count],
		(char *)&cmd.discharge, sizeof(S_P2_TEST_STEP_DISCHARGE));
#if 0
	userlog(DEBUG_LOG, psName, "discharge %d %ld %ld %ld %ld %ld %ld\n", count,
		myPs->testCond.discharge[count].refV, 
		myPs->testCond.discharge[count].refI,
		myPs->testCond.discharge[count].recodeDeltaTime,
		myPs->testCond.discharge[count].recodeDeltaT,
		myPs->testCond.discharge[count].delta_V1,
		myPs->testCond.discharge[count].delta_V2);
#endif
	return 0;
}

int get_test_step_z(int count)
{
	S_P2_RCV_CMD_TEST_STEP_Z	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_STEP_Z));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_STEP_Z));
	
	memcpy((char *)&myPs->testCond.z[count],
		(char *)&cmd.z, sizeof(S_P2_TEST_STEP_Z));
#if 0
	userlog(DEBUG_LOG, psName, "impedance %d %ld %ld %ld %ld\n", count,
		myPs->testCond.z[count].refV, 
		myPs->testCond.z[count].refI,
		myPs->testCond.z[count].recodeDeltaTime,
		myPs->testCond.z[count].recodeDeltaT);
#endif
	return 0;
}

int get_test_step_rest(int count)
{
	S_P2_RCV_CMD_TEST_STEP_REST	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_STEP_REST));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_STEP_REST));
	
	memcpy((char *)&myPs->testCond.rest[count],
		(char *)&cmd.rest, sizeof(S_P2_TEST_STEP_REST));

	return 0;
}

int get_test_step_long_time_rest(int count)
{
	S_P2_RCV_CMD_TEST_STEP_LONG_TIME_REST	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_STEP_LONG_TIME_REST));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_STEP_LONG_TIME_REST));
	
	memcpy((char *)&myPs->testCond.long_time_rest[count],
		(char *)&cmd.long_time_rest, sizeof(S_P2_TEST_STEP_LONG_TIME_REST));

	return 0;
}

int get_test_step_end(int count)
{
	S_P2_RCV_CMD_TEST_STEP_END	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_STEP_END));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_STEP_END));
	
	memcpy((char *)&myPs->testCond.end,
		(char *)&cmd.end, sizeof(S_P2_TEST_STEP_END));

	if(myPs->testCond.header.totalStep != (count+1)) {
		return -1;
	}
	
	return 0;
}

int rcv_cmd_test_grade(int group)
{
	int stepNo;//group
	S_P2_RCV_CMD_TEST_GRADE_HEADER	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_GRADE_HEADER));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_GRADE_HEADER));
	
	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myData->gData[group].state != G_STANDBY) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_STATE_ERROR);
	}


	if(myPs->config.protocol_version == P2_PROTOCOL_VERSION) {
		if(myPs->signal[P2_SIG_TEST_COND_RCV] != P2) {
			return send_cmd_response((char *)&cmd.header,
				P2_CD_GP_TEST_PRECHECK_UNRCV);
		}
	} else {
		if(myPs->signal[P2_SIG_TEST_COND_RCV] != P4) {
			return send_cmd_response((char *)&cmd.header,
				P2_CD_GP_TEST_NG_COND_UNRCV);
		}
	}
	
	
	if(cmd.gradeHeader.totalGrade > MAX_P2_GRADE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_TEST_TOTAL_GRADE_ERROR);
	}

	stepNo = cmd.gradeHeader.stepNo;
	memcpy((char *)&myPs->testCond.gradeHeader[stepNo],
		(char *)&cmd.gradeHeader, sizeof(S_P2_TEST_GRADE_HEADER));

	memcpy((char *)&myPs->testCond.grade[stepNo][0],
		(char *)&myPs->rcvCmd.cmd[0] + sizeof(S_P2_CMD_HEADER)
		+ sizeof(S_P2_TEST_GRADE_HEADER),
		sizeof(S_P2_TEST_GRADE)*cmd.gradeHeader.totalGrade);

	myPs->misc.gradeCount++;
	if(myPs->misc.stepCount == myPs->testCond.header.totalStep
		&& myPs->misc.gradeCount == myPs->testCond.header.totalGrade) {
		myPs->signal[P2_SIG_TEST_COND_RCV] = P0;
		convert_test_cond();
		myPs->misc.state = P2_G_STANDBY;
	}

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_test_info_request(int group)
{
	S_P2_RCV_CMD_TEST_INFO_REQUEST	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TEST_INFO_REQUEST));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TEST_INFO_REQUEST));
	
	return send_cmd_test_info_data((char *)&cmd.header);
}

int rcv_cmd_set_sensor_limit(int group)
{
	int jig, rtn;
	S_P2_RCV_CMD_SET_SENSOR_LIMIT	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_SET_SENSOR_LIMIT));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_SET_SENSOR_LIMIT));

	jig = groupNo_to_jigNo(group);
	if(jig > 0) jig--;
	else jig = 0;

	memcpy((char *)&myPs->sensor_limit, (char *)&cmd.sensor_limit,
		sizeof(S_P2_SENSOR_LIMIT));
	rtn = Write_sensor_limit();

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_tray_data(int group)
{
	int jig, i, chInGroup, ch, chIdx;
	S_P2_RCV_CMD_TRAY_DATA	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_TRAY_DATA));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_TRAY_DATA));

	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myPs->misc.state != P2_G_STANDBY) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	jig = groupNo_to_jigNo(group);
	if(jig > 0) jig--;
	else jig = 0;
	if(myData->jData[jig].config.localRemote == J_LOCAL) {
		return send_cmd_response((char *)&cmd.header, P2_CD_JG_IS_LOCAL);
	}

	chInGroup = myData->mData.config.chInGroup[group];

	userlog(DEBUG_LOG, psName, "rcv_tray_data ");
	for(i=0; i < chInGroup; i++) {
		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_450A_200A_100A_10A: //kjgw
				chIdx = i;
				ch = group * chInGroup + i;
				break;
			case F_SDI_5V_400A_200A_100A_10A_2:
				chIdx = i;
				ch = i;
				break;
			default:
				chIdx = (int)myPs->config.ChArray2[i];
				if(chIdx < 0) continue;
				ch = group * chInGroup + i;
				break;
		}

		myPs->misc.nonCell[ch]
			= cmd.tray_data.cellCode[chIdx]; //0:normal, 1:nonCell
//		myData->cData[ch].misc.errorCell
//			= cmd.tray_data.cellGrade[chIdx];
		userlog2(DEBUG_LOG, psName, "%d(%d) ", chIdx+1, myPs->misc.nonCell[ch]);
	}
	userlog2(DEBUG_LOG, psName, "\n");

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_chamber_set(int group)
{
	int rtn, toPs;
	S_P2_RCV_CMD_CHAMBER_SET cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_CHAMBER_SET));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_CHAMBER_SET));

	memcpy((char *)&myPs->chamber_set, (char *)&cmd.chamber_set,
		sizeof(S_P2_CHAMBER_SET));
	rtn = Write_chamber_set();

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CHAMBER_SET;
	SendMsg.val[0] = myPs->config.groupNo;
	SendMsg.val[1] = (int)cmd.chamber_set.type;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName,
		"rcv_cmd_chamber_set type:%d, set_temp:%d\n",
		myPs->chamber_set.type, myPs->chamber_set.set_temp);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_job_change_set(int group)
{
	int toPs, rtn;
	S_P2_RCV_CMD_JOB_CHANGE_SET	cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_JOB_CHANGE_SET));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_JOB_CHANGE_SET));

	if(cmd.jobModel != 1 && cmd.jobModel != 7) {
		return send_cmd_response((char *)&cmd.header, P2_CD_NACK);
	}

	rtn = Write_job_change_set((int)cmd.jobModel);

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_JIGC1 + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_JIGC_JOB_CHANGE;
	SendMsg.val[0] = (int)cmd.jobModel;
	send_msg(toPs, (char *)&SendMsg);

	userlog(DEBUG_LOG, psName, "rcv_cmd_job_change_set %d\n", cmd.jobModel);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_check(int group)
{
	S_P2_RCV_CMD_CHECK	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_CHECK));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_CHECK));

	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	//send_msg(SUB_TO_MODULE, MSG_SUB_MODULE_CMD_CHECK, 1, 0);
	
	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_run(int group)
{
	int tray, OnOff, jig, totalTrayNo, toPs;
	unsigned long flag, check_flag;
	S_P2_RCV_CMD_RUN	cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_RUN));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_RUN));

	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	jig = groupNo_to_jigNo(group);
	if(jig > 0) jig--;
	else jig = 0;

	totalTrayNo = (int)myData->jData[jig].config.totalTrayNo;

	if(myPs->misc.state != P2_G_STANDBY) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	if(myData->mData.state == M_RUN || myData->mData.state == M_PAUSE
		|| myData->mData.state == M_CALI) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	if(cmd.useFlag == 0) {
		return send_cmd_response((char *)&cmd.header, P2_CD_NACK);
	}

	if(myData->jData[jig].trayState[0] != T_LOAD) {
		return send_cmd_response((char *)&cmd.header, P2_CD_JG_TRAY_UNLOAD);
	}

	memcpy((char *)&myPs->misc.test_serial_no[0],
		(char *)&cmd.test_serial_no[0], 32);

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
		case F_SDI_5V_450A_200A_100A_10A:
			myData->data_10ms_count[0] = 0;
			myData->data_10ms_count[1] = 0;
			memset((char *)&myData->data_10ms[0][0][0], 0,
				sizeof(long) * 2 * 4 * 2000);
			memset((char *)&myData->data_10ms_tmp[0][0][0], 0,
				sizeof(long) * 2 * 4 * 2000);
			break;
		default: break;
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CMD_RUN;
	SendMsg.val[0] = myPs->config.groupNo;
	send_msg(toPs, (char *)&SendMsg);

	flag = 0x00000001;
	for(tray = 0; tray < totalTrayNo; tray++) {
		check_flag = (flag << tray) & cmd.useFlag;
		if(check_flag != 0) {
			OnOff = 1;
			tray = tray + totalTrayNo * group;
		}else{
			OnOff = 0;
			tray = tray + totalTrayNo * group;
		}

		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

		toPs = COB1_TO_MODULE + myPs->config.groupNo;
		SendMsg.msg = MSG_COB_MODULE_CMD_SET_BCR_LAMP;
		SendMsg.val[0] = tray;
		SendMsg.val[1] = OnOff;
		send_msg(toPs, (char *)&SendMsg);
	}

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_stop(int group)
{
	int toPs;
	S_P2_RCV_CMD_STOP	cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_STOP));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_STOP));
	
	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myPs->misc.state != P2_G_RUN && myPs->misc.state != P2_G_PAUSE) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CMD_STOP;
	SendMsg.val[0] = myPs->config.groupNo;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_pause(int group)
{
	int toPs;
	S_P2_RCV_CMD_PAUSE	cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_PAUSE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_PAUSE));
	
	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myPs->misc.state != P2_G_RUN) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CMD_PAUSE;
	SendMsg.val[0] = myPs->config.groupNo;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_continue(int group)
{
	int toPs, jig;
	S_P2_RCV_CMD_CONTINUE	cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_CONTINUE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_CONTINUE));
	
	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	jig = groupNo_to_jigNo(group);
	if(jig > 0) jig--;
	else jig = 0;

	if(myData->jData[jig].trayState[0] != T_LOAD) {
		return send_cmd_response((char *)&cmd.header, P2_CD_JG_TRAY_UNLOAD);
	}

	if(myPs->misc.state != P2_G_PAUSE) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CMD_CONTINUE;
	SendMsg.val[0] = myPs->config.groupNo;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_next_step(int group)
{
	int toPs;
	S_P2_RCV_CMD_NEXT_STEP	cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_NEXT_STEP));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_NEXT_STEP));
	
	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myPs->misc.state == P2_G_RUN) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CMD_NEXT_STEP;
	SendMsg.val[0] = myPs->config.groupNo;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_reset(int group)
{
	int toPs;
	S_P2_RCV_CMD_RESET	cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_RESET));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_RESET));

//Online Mode Enable
/*	
	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}
*/
	if(myPs->misc.state == P2_G_RUN) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	myPs->misc.state = P2_G_IDLE;
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CMD_RESET;
	SendMsg.val[0] = myPs->config.groupNo;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_clear(int group)
{
	int toPs;
	S_P2_RCV_CMD_CLEAR	cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_CLEAR));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_CLEAR));
	
	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CMD_CLEAR;
	SendMsg.val[0] = myPs->config.groupNo;
	send_msg(toPs, (char *)&SendMsg);
	
	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_cali_meter_connect(int group)
{
//	int toPs;
	S_P2_RCV_CMD_CALI_METER_CONNECT	cmd;
//	S_MSG_VAL SendMsg;

	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}
/*
	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_CALI_METER_CONNECT));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_CALI_METER_CONNECT));

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_METER + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_METER_INITIALIZE;
	send_msg(toPs, (char *)&SendMsg);
*/
	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_response(int group)
{
	S_P2_RCV_CMD_RESPONSE	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_RESPONSE));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_RESPONSE));
	
	if(cmd.code == P2_CD_ACK) {
		//kjgw if(cmd.sent_cmd_id == P2_CMD_TO_PC_COMM_CHECK) {
			myPs->misc.net_time = myData->mData.misc.timer_1sec;
			myPs->misc.net_time2 = myData->mData.misc.timer_1000ms;
		//}
	} else {
		userlog(DEBUG_LOG, psName, "rcv_cmd_response id(hex):%x code(dec):%d\n",
			cmd.sent_cmd_id, cmd.code);
		return -1; //kjgw
	}
	return 0;
}

int rcv_cmd_comm_check_reply(int group)
{
	char buf[8];
	S_P2_RCV_CMD_COMM_CHECK_REPLY comm_check_reply;
	
	memset((char *)&comm_check_reply, 0,
		sizeof(S_P2_RCV_CMD_COMM_CHECK_REPLY));
	memcpy((char *)&comm_check_reply, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_COMM_CHECK_REPLY));
	
	memset(buf, 0, sizeof buf);
	memcpy((char *)&buf[0], (char *)&comm_check_reply.result[0], 2);
	if(atoi(buf) == P2_CD_ACK) {
	} else {
		userlog(DEBUG_LOG, psName, "rcv_cmd_comm_check_reply result:%s\n", buf);
	}
	
	memset(buf, 0, sizeof buf);
	memcpy((char *)&buf[0], (char *)&comm_check_reply.sended_cmd[0], 4);
	if(strncmp((char *)&buf[0], "0x21", 4) == 0) {
	} else {
		userlog(DEBUG_LOG, psName,
			"rcv_cmd_comm_check_reply sended_cmd:%s\n", buf);
	}
	return 0;
}

int rcv_cmd_comm_check(int group)
{
	S_P2_RCV_CMD_COMM_CHECK_REPLY	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_COMM_CHECK_REPLY));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_COMM_CHECK_REPLY));

	return send_cmd_comm_check_reply((char *)&cmd.header);
}

int rcv_cmd_user_cmd(int group)
{
	int rtn;
	S_P2_RCV_CMD_USER_CMD cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_USER_CMD));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0]
			,sizeof(S_P2_RCV_CMD_USER_CMD));
	switch(cmd.cmd){
		case P2_CMD_TO_SBC_RUN:
			rtn = user_cmd_run(group);
			break;
		case P2_CMD_TO_SBC_PAUSE:
			rtn = user_cmd_pause(group);
			break;
		case P2_CMD_TO_SBC_CONTINUE:
			rtn = user_cmd_continue(group);
			break;
		case P2_CMD_TO_SBC_STOP:
			rtn = user_cmd_stop(group);
			break;
		case P2_CMD_TO_SBC_RESET:
			rtn = user_cmd_reset(group);
			break;
		default:
			userlog(DEBUG_LOG, psName, "Can't Find user Cmd[%x]\n", cmd.cmd);
			rtn = -10;
			break;
	}
	return rtn;
}

int user_cmd_run(int group)
{
	int serial_no, toPs;
//	char **ptr;
	S_P2_RCV_CMD_USER_CMD cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_USER_CMD));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0]
			,sizeof(S_P2_RCV_CMD_USER_CMD));

	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myPs->misc.state != P2_G_STANDBY) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	serial_no = strtol((char *)&myPs->misc.test_serial_no,(char **)NULL, 0);
	serial_no++;
	sprintf((char *)&myPs->misc.test_serial_no,"%d", serial_no);

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CMD_RUN;
	SendMsg.val[0] = myPs->config.groupNo;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_set_tray_ready(int group)
{
	int tray, OnOff, jig, totalTrayNo, toPs;
	S_P2_RCV_CMD_SET_TRAY_READY	cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_SET_TRAY_READY));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_SET_TRAY_READY));

	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	jig = groupNo_to_jigNo(group);
	if(jig > 0) jig--;
	else jig = 0;

	totalTrayNo = (int)myData->jData[jig].config.totalTrayNo;

	if(cmd.data == 0){
		return send_cmd_response((char *)&cmd.header, P2_CD_NACK);	
	}else{
		tray = cmd.code-1;
		tray = tray + totalTrayNo * group;
		OnOff = cmd.data;

		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

		toPs = COB1_TO_MODULE + myPs->config.groupNo;
		SendMsg.msg = MSG_COB_MODULE_CMD_SET_BCR_LAMP;
		SendMsg.val[0] = tray;
		SendMsg.val[1] = OnOff;
		send_msg(toPs, (char *)&SendMsg);

		return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
	}
}

int user_cmd_stop(int group)
{
	int toPs;
	S_P2_RCV_CMD_USER_CMD cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_USER_CMD));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0]
			,sizeof(S_P2_RCV_CMD_USER_CMD));

	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myPs->misc.state != P2_G_RUN && myPs->misc.state != P2_G_PAUSE) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CMD_STOP;
	SendMsg.val[0] = myPs->config.groupNo;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int user_cmd_pause(int group)
{
	int toPs;
	S_P2_RCV_CMD_USER_CMD cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_USER_CMD));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0]
			,sizeof(S_P2_RCV_CMD_USER_CMD));

	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myPs->misc.state != P2_G_RUN) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CMD_PAUSE;
	SendMsg.val[0] = myPs->config.groupNo;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int user_cmd_continue(int group)
{
	int toPs;
	S_P2_RCV_CMD_USER_CMD cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_USER_CMD));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0]
			,sizeof(S_P2_RCV_CMD_USER_CMD));

	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myPs->misc.state != P2_G_PAUSE) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CMD_CONTINUE;
	SendMsg.val[0] = myPs->config.groupNo;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int user_cmd_reset(int group)
{
	int toPs;
	S_P2_RCV_CMD_USER_CMD cmd;
	S_MSG_VAL SendMsg;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_USER_CMD));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0]
			,sizeof(S_P2_RCV_CMD_USER_CMD));

	if(myData->gData[group].workMode != P2_WORK_OFFLINE) {
		return send_cmd_response((char *)&cmd.header,
			P2_CD_GP_WORK_TYPE_ERROR);
	}

	if(myPs->misc.state != P2_G_STANDBY) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	myPs->misc.state = P2_G_IDLE;

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	SendMsg.msg = MSG_COB_MODULE_CMD_RESET;
	SendMsg.val[0] = myPs->config.groupNo;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

//kjg_110712
int rcv_cmd_set_cali_point(int group)
{
	int i, j;
	S_P2_RCV_CMD_SET_CALI_POINT	cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_SET_CALI_POINT));

	if(cmd.set_point.cali_division == 0) { //main_da
		for(i=0; i < MAX_RANGE; i++) {
			myData->cali_set_data.main_dav.set_point_num[i]
				= (int)cmd.set_point.v_cali_point[i].cali_point_num;
			for(j=0; j < MAX_F_CALI_POINT; j++) {
				myData->cali_set_data.main_dav.set_point[i][j]
					= cmd.set_point.v_cali_point[i].cali_point[j];
			}

			myData->cali_set_data.main_dav.check_point_num[i]
				= (int)cmd.set_point.v_cali_point[i].cali_check_point_num;
			for(j=0; j < MAX_F_CALI_CHECK_POINT; j++) {
				myData->cali_set_data.main_dav.check_point[i][j]
					= cmd.set_point.v_cali_point[i].cali_check_point[j];
			}

			myData->cali_set_data.main_dai.set_point_num[i]
				= (int)cmd.set_point.i_cali_point[i].cali_point_num;
			for(j=0; j < MAX_F_CALI_POINT; j++) {
				myData->cali_set_data.main_dai.set_point[i][j]
					= cmd.set_point.i_cali_point[i].cali_point[j];
			}

			myData->cali_set_data.main_dai.check_point_num[i]
				= (int)cmd.set_point.i_cali_point[i].cali_check_point_num;
			for(j=0; j < MAX_F_CALI_CHECK_POINT; j++) {
				myData->cali_set_data.main_dai.check_point[i][j]
					= cmd.set_point.i_cali_point[i].cali_check_point[j];
			}
		}

		if(Write_Cali_Set_Main() < 0) {
			userlog(DEBUG_LOG, psName,
				"RcvCmd(0x%x) Write_Cali_Set_Main error\n", cmd.header.cmd_id);
			return -1;
		}
	} else if(cmd.set_point.cali_division == 1) { //ch
		for(i=0; i < MAX_RANGE; i++) {
			myData->cali_set_data.ch_v.set_point_num[i]
				= (int)cmd.set_point.v_cali_point[i].cali_point_num;
			for(j=0; j < MAX_F_CALI_POINT; j++) {
				myData->cali_set_data.ch_v.set_point[i][j]
					= cmd.set_point.v_cali_point[i].cali_point[j];
			}

			myData->cali_set_data.ch_v.check_point_num[i]
				= (int)cmd.set_point.v_cali_point[i].cali_check_point_num;
			for(j=0; j < MAX_F_CALI_CHECK_POINT; j++) {
				myData->cali_set_data.ch_v.check_point[i][j]
					= cmd.set_point.v_cali_point[i].cali_check_point[j];
			}

			myData->cali_set_data.ch_i.set_point_num[i]
				= (int)cmd.set_point.i_cali_point[i].cali_point_num;
			printf("kjgd cali_point %d : ", i);
			for(j=0; j < MAX_F_CALI_POINT; j++) {
				printf("%ld ", cmd.set_point.i_cali_point[i].cali_point[j]);
				myData->cali_set_data.ch_i.set_point[i][j]
					= cmd.set_point.i_cali_point[i].cali_point[j];
			}
			printf("\n");

			myData->cali_set_data.ch_i.check_point_num[i]
				= (int)cmd.set_point.i_cali_point[i].cali_check_point_num;
			printf("kjgd cali_check_point %d : ", i);
			for(j=0; j < MAX_F_CALI_CHECK_POINT; j++) {
				printf("%ld ", cmd.set_point.i_cali_point[i].cali_check_point[j]);
				myData->cali_set_data.ch_i.check_point[i][j]
					= cmd.set_point.i_cali_point[i].cali_check_point[j];
			}
			printf("\n");
		}

		if(Write_Cali_Set_Ch() < 0) {
			userlog(DEBUG_LOG, psName,
				"RcvCmd(0x%x) Write_Cali_Set_Ch error\n", cmd.header.cmd_id);
			return -2;
		}
	} else {
		userlog(DEBUG_LOG, psName, "RcvCmd(0x%x) cali_division error:%d\n",
			cmd.header.cmd_id, cmd.set_point.cali_division);
		return -3;
	}

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_get_main_cali_point(int group)
{
	S_P2_RCV_CMD_GET_MAIN_CALI_POINT cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_GET_MAIN_CALI_POINT));

	return send_cmd_get_main_cali_point_reply((char *)&cmd.header);
}

int rcv_cmd_get_ch_cali_point(int group)
{
	S_P2_RCV_CMD_GET_CH_CALI_POINT cmd;

	userlog(DEBUG_LOG, psName, "rcv_cmd_get_ch_cali_point %d\n", group);

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_GET_CH_CALI_POINT));

	return send_cmd_get_ch_cali_point_reply((char *)&cmd.header);
}

int rcv_cmd_cali_start(int group)
{
	int ch, toPs;
	S_P2_RCV_CMD_CALI_START	cmd;
	S_MSG_VAL SendMsg;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_CALI_START));

	if(myPs->config.workMode == P2_WORK_ONLINE)
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);

	if(myPs->misc.state == P2_G_RUN 
		|| myPs->misc.state == P2_G_PAUSE) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	toPs = COB1_TO_MODULE + myPs->config.groupNo;

	if(cmd.cali_division == 0) { //main_da
	} else { //ch
//		if(cmd.parallel == 0) {
			printf("kjgd cali_start flag ");
			for(ch=0; ch < myData->mData.config.installedCh; ch++) {
				myData->cali_ch_flag.flag[ch] = cmd.cali_flag[ch];
				printf("%d ", cmd.cali_flag[ch]);
			}
			printf("\n");
//		} else {
//			for(ch=0; ch < (myData->mData.config.installedCh/2); ch++) {
//				myData->cali_ch_flag.flag[ch*2] = cmd.cali_flag[ch];
//			}
//		}

		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		SendMsg.msg = MSG_COB_MODULE_CALI_MULTI;
		SendMsg.val[0] = group;
		SendMsg.val[1] = (int)cmd.cali_multi;
		SendMsg.val[2] = (int)cmd.parallel; //0:independent 1:parallel
		send_msg(toPs, (char *)&SendMsg);

		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		SendMsg.msg = MSG_COB_MODULE_CALI_MAP;
		SendMsg.val[0] = group;
		SendMsg.val[1] = (int)cmd.trayType;
		send_msg(toPs, (char *)&SendMsg);

		userlog(DEBUG_LOG, psName, "cali multi:%d, trayType:%d\n",
			cmd.cali_multi, cmd.trayType);

		if(cmd.cali_type == 0) {
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_COB_MODULE_CALI_V_RANGE;
			SendMsg.val[0] = group;
			SendMsg.val[1] = (int)cmd.cali_range;
			send_msg(toPs, (char *)&SendMsg);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_COB_MODULE_CALI_V;
			SendMsg.val[0] = group;
			SendMsg.val[1] = (int)cmd.cali_board_no;
			send_msg(toPs, (char *)&SendMsg);
			userlog(DEBUG_LOG, psName, "cali v: bd[%d] range[%d]\n",
				cmd.cali_board_no, cmd.cali_range);
		} else {
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_COB_MODULE_CALI_I_RANGE;
			SendMsg.val[0] = group;
			SendMsg.val[1] = (int)cmd.cali_range;
			send_msg(toPs, (char *)&SendMsg);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_COB_MODULE_CALI_I;
			SendMsg.val[0] = group;
			SendMsg.val[1] = (int)cmd.cali_board_no;
			send_msg(toPs, (char *)&SendMsg);
			userlog(DEBUG_LOG, psName, "cali i: bd[%d] range[%d]\n",
				cmd.cali_board_no, cmd.cali_range);
		}
	}
	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_cali_check_start(int group)
{
	int ch, toPs;
	S_P2_RCV_CMD_CALI_CHECK_START cmd;
	S_MSG_VAL SendMsg;

	if(myPs->config.workMode == P2_WORK_ONLINE)
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_CALI_CHECK_START));

	if(myPs->misc.state == P2_G_RUN || myPs->misc.state == P2_G_PAUSE) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	toPs = COB1_TO_MODULE + myPs->config.groupNo;

	if(cmd.cali_division == 0) { //main_da
	} else { //ch
//		if(cmd.parallel == 0) {
			printf("kjgd cali_check_start flag ");
			for(ch=0; ch < myData->mData.config.installedCh; ch++) {
				myData->cali_ch_flag.flag[ch] = cmd.cali_flag[ch];
				printf("%d ", cmd.cali_flag[ch]);
			}
			printf("\n");
//		} else {
//			for(ch=0; ch < (myData->mData.config.installedCh/2); ch++) {
//				myData->cali_ch_flag.flag[ch*2] = cmd.cali_flag[ch];
//			}
//		}

		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		SendMsg.msg = MSG_COB_MODULE_CALI_MULTI;
		SendMsg.val[0] = group;
		SendMsg.val[1] = (int)cmd.cali_multi;
		SendMsg.val[2] = (int)cmd.parallel; //0:independent 1:parallel
		send_msg(toPs, (char *)&SendMsg);

		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		SendMsg.msg = MSG_COB_MODULE_CALI_MAP;
		SendMsg.val[0] = group;
		SendMsg.val[1] = (int)cmd.trayType;
		send_msg(toPs, (char *)&SendMsg);

		userlog(DEBUG_LOG, psName, "cali check multi:%d, trayType:%d\n",
			cmd.cali_multi, cmd.trayType);

		if(cmd.cali_type == 0) {
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_COB_MODULE_CALI_V_RANGE;
			SendMsg.val[0] = group;
			SendMsg.val[1] = (int)cmd.cali_range;
			send_msg(toPs, (char *)&SendMsg);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_COB_MODULE_CALI_CHECK_V;
			SendMsg.val[0] = group;
			SendMsg.val[1] = (int)cmd.cali_board_no;
			send_msg(toPs, (char *)&SendMsg);
			userlog(DEBUG_LOG, psName, "cali check v: bd[%d] range[%d]\n",
				cmd.cali_board_no, cmd.cali_range);
		} else {
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_COB_MODULE_CALI_I_RANGE;
			SendMsg.val[0] = group;
			SendMsg.val[1] = (int)cmd.cali_range;
			send_msg(toPs, (char *)&SendMsg);
			userlog(DEBUG_LOG, psName, "cali check i: bd[%d] range[%d]\n",
				cmd.cali_board_no, cmd.cali_range);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_COB_MODULE_CALI_CHECK_I;
			SendMsg.val[0] = group;
			SendMsg.val[1] = (int)cmd.cali_board_no;
			send_msg(toPs, (char *)&SendMsg);
		}
	}

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_cali_stop(int group)
{
	int toPs;
	S_P2_RCV_CMD_CALI_STOP cmd;
	S_MSG_VAL SendMsg;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_CALI_STOP));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	SendMsg.val[0] = group;
	SendMsg.msg = MSG_COB_MODULE_CALI_STOP;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_cali_pause(int group)
{
	S_P2_RCV_CMD_CALI_PAUSE	cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_CALI_PAUSE));

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_cali_resume(int group)
{
	S_P2_RCV_CMD_CALI_RESUME cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_CALI_RESUME));

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_cali_update(int group)
{
	int toPs;
	S_P2_RCV_CMD_CALI_UPDATE cmd;
	S_MSG_VAL SendMsg;

	toPs = COB1_TO_APP + myPs->config.groupNo;
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	SendMsg.msg = MSG_COB_APP_CALI_UPDATE;
	SendMsg.val[0] = group;
	SendMsg.val[1] = 50;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_real_measure_start(int group)
{
	int ch, chNo, toPs;
	S_P2_RCV_CMD_REAL_MEASURE_START	cmd;
	S_MSG_VAL SendMsg;

	if(myPs->config.workMode == P2_WORK_ONLINE)
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_REAL_MEASURE_START));
	
	if(myPs->misc.state != P2_G_RUN) {
		return send_cmd_response((char *)&cmd.header, P2_CD_GP_STATE_ERROR);
	}

	userlog(DEBUG_LOG, psName, "rcv_cmd_real_measure_start %d\n", group);

	memset((char *)&myData->measure_ch_flag, 0,
		sizeof(unsigned char) * MAX_CH_256);

	for(ch=0; ch < myData->mData.config.chInGroup[group]; ch++) {
		chNo = (int)myData->COB_Client[group].config.ChArray3[ch].index;
		myData->measure_ch_flag.flag[chNo] = cmd.monitor_ch[ch];
	}

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	SendMsg.msg = MSG_COB_MODULE_MEASURE_TIME;
	SendMsg.val[0] = group;
	SendMsg.val[1] = cmd.recordTime * 1000;
	send_msg(toPs, (char *)&SendMsg);

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	SendMsg.msg = MSG_COB_MODULE_MEAS;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_real_measure_stop(int group)
{
	int toPs;
	S_P2_RCV_CMD_REAL_MEASURE_STOP	cmd;
	S_MSG_VAL SendMsg;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_REAL_MEASURE_STOP));

	toPs = COB1_TO_MODULE + myPs->config.groupNo;
	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	SendMsg.msg = MSG_COB_MODULE_MEASURE_STOP;
	SendMsg.val[0] = group;
	send_msg(toPs, (char *)&SendMsg);

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int rcv_cmd_hw_map_req(int group)
{
	userlog(DEBUG_LOG, psName, "rcv_cmd_hw_map_req %d\n", group);

	return send_cmd_hw_map_req_rpy(group);
}

int rcv_cmd_ch_attribute_set(int group)
{
	S_P2_RCV_CMD_CH_ATTRIBUTE_SET cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_CH_ATTRIBUTE_SET));

	myPs->misc.parallel_count = cmd.parallel_count;

	myPs->signal[P2_SIG_RCV_CH_ATTRIBUTE_SET] = P1;

	userlog(DEBUG_LOG, psName, "rcv_cmd_ch_attribute_set %d\n",
		cmd.parallel_count);

	return 0;
}

int send_cmd_ch_attribute_set_reply(int state)
{
	int cmd_id, cmd_size;
	S_P2_SEND_CMD_CH_ATTRIBUTE_SET_REPLY cmd;
	
	cmd_id = P2_CMD_TO_PC_CH_ATTRIBUTE_SET_REPLY;
	cmd_size = sizeof(S_P2_SEND_CMD_CH_ATTRIBUTE_SET_REPLY);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	cmd.state = (unsigned char)state;

	userlog(DEBUG_LOG, psName, "send_cmd_ch_attribute_set_reply %d, %d\n",
		state, myPs->misc.parallel_count);

	return send_command((char *)&cmd);
}

int rcv_cmd_jig_temp_set_data(int group)
{
//	int rtn;
	S_P2_RCV_CMD_JIG_TEMP_SET_DATA	cmd;

	memset((char *)&cmd, 0, sizeof(S_P2_RCV_CMD_JIG_TEMP_SET_DATA));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd[0],
		sizeof(S_P2_RCV_CMD_JIG_TEMP_SET_DATA));

	memcpy((char *)&myData->gData[group].jigTempSetData,
		(char *)&cmd.jigTempSetData, sizeof(S_P2_JIG_TEMP_SET_DATA));
//	rtn = Write_jig_temp_set_data(); //khkw

	return send_cmd_response((char *)&cmd.header, P2_CD_ACK);
}

int send_cmd_version_data(char *rcvHeader)
{
	int cmd_id, cmd_size, group, jig;
	S_P2_SEND_CMD_VERSION_DATA cmd;
	
	cmd_id = P2_CMD_TO_PC_VERSION_DATA;
	cmd_size = sizeof(S_P2_SEND_CMD_VERSION_DATA);
	memset((char *)&cmd, 0, cmd_size);
	make_header2(cmd_id, cmd_size, (char *)&cmd, rcvHeader);

	group = (int)myPs->config.groupNo;
	cmd.moduleNo = (int)myPs->config.groupId; //1base
	cmd.versionNo = (int)myPs->config.protocol_version;
	cmd.sbcType = myData->AppControl.config.sbcType;
	cmd.parallel_count = myPs->misc.parallel_count;
	cmd.installedBd
		= (unsigned short)myData->mData.config.installedBd;
		//= (unsigned short)myData->mData.config.bdInGroup[group];
	cmd.chPerBd = (unsigned short)myData->mData.config.chPerBd;
	cmd.groupNo = group + 1; //1base
	//cmd.chInGroup = (int)myData->mData.config.chInGroup[group];
	cmd.chInGroup = get_chInGroup(group);

	jig = groupNo_to_jigNo(group);
	if(jig > 0) jig--;
	else jig = 0;

	cmd.trayType = (unsigned short)myData->jData[jig].config.trayType;
	cmd.totalTrayNo = (unsigned short)myData->jData[jig].config.totalTrayNo;
	//cmd.chInTray[0] = (unsigned short)myData->mData.config.chInGroup[group];
	cmd.chInTray[0] = get_chInGroup(group);

	userlog(DEBUG_LOG, psName, "send_cmd_version_data %d %d\n",
		cmd.chInGroup, cmd.chInTray[0]);

	return send_command((char *)&cmd);
}

int send_cmd_version_data2(char *rcvHeader)
{
	int cmd_id, cmd_size, group, jig;
	S_P2_SEND_CMD_VERSION_DATA cmd;
	
	cmd_id = P2_CMD_TO_PC_VERSION_DATA2;
	cmd_size = sizeof(S_P2_SEND_CMD_VERSION_DATA);
	memset((char *)&cmd, 0, cmd_size);
	make_header2(cmd_id, cmd_size, (char *)&cmd, rcvHeader);

	group = (int)myPs->config.groupNo;
	cmd.moduleNo = (int)myPs->config.groupId; //1base
	cmd.versionNo = (int)myPs->config.protocol_version;
	cmd.sbcType = myData->AppControl.config.sbcType;
	cmd.parallel_count = myPs->misc.parallel_count;
	cmd.installedBd
		= (unsigned short)myData->mData.config.installedBd;
		//= (unsigned short)myData->mData.config.bdInGroup[group];
	cmd.chPerBd = (unsigned short)myData->mData.config.chPerBd;
	cmd.groupNo = group + 1; //1base
	//cmd.chInGroup = (int)myData->mData.config.chInGroup[group];
	cmd.chInGroup = get_chInGroup(group);

	jig = groupNo_to_jigNo(group);
	if(jig > 0) jig--;
	else jig = 0;

	cmd.trayType = (unsigned short)myData->jData[jig].config.trayType;
	cmd.totalTrayNo = (unsigned short)myData->jData[jig].config.totalTrayNo;
	//cmd.chInTray[0] = (unsigned short)myData->mData.config.chInGroup[group];
	cmd.chInTray[0] = get_chInGroup(group);

	if(myPs->signal[P2_SIG_RCV_CH_ATTRIBUTE_SET] == P3) {
		myPs->signal[P2_SIG_RCV_CH_ATTRIBUTE_SET]++;
	}

	userlog(DEBUG_LOG, psName, "send_cmd_version_data2 %d %d\n",
		cmd.chInGroup, cmd.chInTray[0]);

	return send_command((char *)&cmd);
}

int send_cmd_group_state(void)
{
	int cmd_id, cmd_size, group, jig, i;
	unsigned short flag;
	S_P2_SEND_CMD_GROUP_STATE cmd;
	
	cmd_id = P2_CMD_TO_PC_GROUP_STATE;
	cmd_size = sizeof(S_P2_SEND_CMD_GROUP_STATE);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	group = (int)myPs->config.groupNo;
	jig = groupNo_to_jigNo(group);//1 base
	if(jig > 0) jig--;
	else return 0;

	myPs->misc.tmpState = (unsigned short)myPs->misc.state;
	myPs->misc.tmpCode = (unsigned short)myPs->misc.code;
	memset((char *)&myPs->misc.tmpSensorState, 0, sizeof(unsigned short) * 8);

	switch(myData->AppControl.config.systemModel) {
		/*case L_5V_15A_10A:
			flag = 0x0001;
			for(i = 0; i < myData->jData[jig].config.totalTrayNo; i++){
				flag = 0x0001 << i;
				if(myData->jData[jig].jigState == J_CONTACT) {
					myPs->misc.tmpSensorState[J_S_TRAYSTATE_CHECK] |= flag;
				} else {
					myPs->misc.tmpSensorState[J_S_TRAYSTATE_CHECK] &= ~flag;
				}
			}
			break;*/
		default:
			if(myData->jData[jig].jigState == J_CONTACT) {
				myPs->misc.tmpSensorState[J_S_JIGSTATE_CHECK] = 0x01;
			} else {
				myPs->misc.tmpSensorState[J_S_JIGSTATE_CHECK] = OFF;
			}
			break;
	}

	flag = 0x0001;
	for(i=0; i < myData->jData[jig].config.totalTrayNo; i++){
		flag = 0x0001 << i;
		if(myData->jData[jig].trayState[i] == T_LOAD) {
			myPs->misc.tmpSensorState[J_S_TRAYSTATE_CHECK] |= flag;
		} else {
			myPs->misc.tmpSensorState[J_S_TRAYSTATE_CHECK] &= ~flag;
		}
	}

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_450A_200A_100A_10A:
			myPs->misc.tmpSensorState[J_S_STOPPERSTATE_CHECK]
				= myPs->misc.jobModel;
			break;
		default:
			if(myData->jData[jig].stopperState == S_CONTACT) {
				myPs->misc.tmpSensorState[J_S_STOPPERSTATE_CHECK] = 0x01;
			} else {
				myPs->misc.tmpSensorState[J_S_STOPPERSTATE_CHECK] = OFF;
			}
			break;
	}

	flag = 0x0001;
	for(i=0; i < MAX_TRAY_PER_GROUP; i++){
		flag = 0x0001 << i;
		if(myData->jData[jig].doorState[i] == D_CLOSE) {
			myPs->misc.tmpSensorState[J_S_DOORSTATE_CHECK] |= flag;
		} else {
			myPs->misc.tmpSensorState[J_S_DOORSTATE_CHECK] &= ~flag;
		}
	}

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_50A_5A:
			if(group == 0) {
				i = MAX_DIO_II_BYTES + MAX_DIO_MI_BYTES + MAX_DIO_CI_BYTES;
			} else {
				i = MAX_DIO_II_BYTES + MAX_DIO_MI_BYTES + MAX_DIO_CI_BYTES + 3;
			}
			flag = (unsigned short)myData->dio.in.bytes[i+1];
			flag = flag << 8;
			flag |= (unsigned short)myData->dio.in.bytes[i];
			myPs->misc.tmpSensorState[J_S_IN_BITS1] = flag;

			flag = (unsigned short)myData->dio.in.bytes[i+2];
			myPs->misc.tmpSensorState[J_S_IN_BITS2] = flag;
			break;
		case F_SDI_5V_400A_200A_100A_10A:
			i = MAX_DIO_II_BYTES;
			flag = (unsigned short)myData->dio.in.bytes[i+1];
			flag = flag << 8;
			flag |= (unsigned short)myData->dio.in.bytes[i];
			myPs->misc.tmpSensorState[J_S_IN_BITS1] = flag;

			flag = (unsigned short)myData->dio.in.bytes[i+2];
			myPs->misc.tmpSensorState[J_S_IN_BITS2] = flag;
			break;
		case F_SDI_5V_400A_200A_100A_10A_2:
			if(group == 0) {
				i = MAX_DIO_II_BYTES;
				flag = (unsigned short)myData->dio.in.bytes[i+1] & 0x0038;
				flag = flag << 8;
				flag |= ((unsigned short)myData->dio.in.bytes[i] & 0x0023);
				myPs->misc.tmpSensorState[J_S_IN_BITS1] = flag;

				flag = (unsigned short)myData->dio.in.bytes[i+2];
				myPs->misc.tmpSensorState[J_S_IN_BITS2] = flag;
			} else {
				i = MAX_DIO_II_BYTES;
				flag = (unsigned short)myData->dio.in.bytes[i+1] & 0x0007;
				flag = flag << 8;
				flag |= ((unsigned short)myData->dio.in.bytes[i] & 0x0007);
				myPs->misc.tmpSensorState[J_S_IN_BITS1] = flag;

				flag = (unsigned short)myData->dio.in.bytes[i+2];
				myPs->misc.tmpSensorState[J_S_IN_BITS2] = flag;
			}
			break;
		case F_SDI_5V_450A_200A_100A_10A: //kjgw
			if(group == 0) {
				i = MAX_DIO_II_BYTES;
				flag = (unsigned short)myData->dio.in.bytes[i+1] & 0x0038;
				flag = flag << 8;
				flag |= ((unsigned short)myData->dio.in.bytes[i] & 0x0023);
				myPs->misc.tmpSensorState[J_S_IN_BITS1] = flag;

				flag = (unsigned short)myData->dio.in.bytes[i+2];
				myPs->misc.tmpSensorState[J_S_IN_BITS2] = flag;
			} else {
				i = MAX_DIO_II_BYTES;
				flag = (unsigned short)myData->dio.in.bytes[i+1] & 0x0007;
				flag = flag << 8;
				flag |= ((unsigned short)myData->dio.in.bytes[i] & 0x0007);
				myPs->misc.tmpSensorState[J_S_IN_BITS1] = flag;

				flag = (unsigned short)myData->dio.in.bytes[i+2];
				myPs->misc.tmpSensorState[J_S_IN_BITS2] = flag;
			}
			break;
		default: break;
	}

	cmd.groupState = myPs->misc.tmpState;
	cmd.code = myPs->misc.tmpCode;

	memcpy((char *)&cmd.sensorState, (char *)&myPs->misc.tmpSensorState,
		sizeof(unsigned short) * 8);

	userlog(DEBUG_LOG, psName, "send_cmd_group_state %d %d\n",
		cmd.groupState, cmd.code);

	return send_command((char *)&cmd);
}

int send_cmd_config_data(char *rcvHeader)
{
	int cmd_id, cmd_size;
	S_P2_SEND_CMD_CONFIG_DATA cmd;
	
	cmd_id = P2_CMD_TO_PC_CONFIG_DATA;
	cmd_size = sizeof(S_P2_SEND_CMD_CONFIG_DATA);
	memset((char *)&cmd, 0, cmd_size);
	make_header2(cmd_id, cmd_size, (char *)&cmd, rcvHeader);

	memcpy((char *)&cmd.configData, (char *)&myPs->configData,
		sizeof(S_P2_CONFIG_DATA));

	return send_command((char *)&cmd);
}

int send_cmd_response(char *rcvHeader, int code)
{
	int cmd_id, cmd_size;
	S_P2_SEND_CMD_RESPONSE cmd;
	S_P2_CMD_HEADER header;

	cmd_id = P2_CMD_TO_PC_RESPONSE;
	cmd_size = sizeof(S_P2_SEND_CMD_RESPONSE);
	memset((char *)&cmd, 0, cmd_size);
	make_header2(cmd_id, cmd_size, (char *)&cmd, rcvHeader);
	
	memcpy((char *)&header, rcvHeader, sizeof(S_P2_CMD_HEADER));
	cmd.received_cmd_id = header.cmd_id;
	cmd.code = code;
	
	return send_command((char *)&cmd);
}

int send_cmd_trouble_code(int code, int chNo)
{
	int cmd_id, cmd_size;
	S_P2_SEND_CMD_TROUBLE_CODE cmd;

	cmd_id = P2_CMD_TO_PC_TROUBLE_CODE;
	cmd_size = sizeof(S_P2_SEND_CMD_TROUBLE_CODE);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);
	
	cmd.code = code;
	cmd.chNo = chNo;
	
	return send_command((char *)&cmd);
}

int send_cmd_switch_data(int val)
{
	int cmd_id, cmd_size;
	S_P2_SEND_CMD_SWITCH_DATA cmd;

	cmd_id = P2_CMD_TO_PC_SWITCH_DATA;
	cmd_size = sizeof(S_P2_SEND_CMD_SWITCH_DATA);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);
	
	cmd.switch1 = (unsigned char)val;
	
	return send_command((char *)&cmd);
}

int send_cmd_line_mode_data(char *rcvHeader)
{
	int cmd_id, cmd_size, group;
	S_P2_SEND_CMD_LINE_MODE_DATA cmd;
	
	cmd_id = P2_CMD_TO_PC_LINE_MODE_DATA;
	cmd_size = sizeof(S_P2_SEND_CMD_LINE_MODE_DATA);
	memset((char *)&cmd, 0, cmd_size);
	make_header2(cmd_id, cmd_size, (char *)&cmd, rcvHeader);

	group = (int)myPs->config.groupNo;
	if(myData->gData[group].workMode == P2_WORK_ONLINE) {
		cmd.onlineMode = P2_WORK_ONLINE;
	} else if(myData->gData[group].workMode == P2_WORK_OFFLINE) {
		cmd.onlineMode = P2_WORK_OFFLINE;
	}
	if(myData->gData[group].group_control_mode == P2_WORK_CONTROL) {
		cmd.controlMode = P2_WORK_CONTROL;
	}else if(myData->gData[group].group_control_mode == P2_WORK_MAINTENANCE) {
		cmd.controlMode = P2_WORK_MAINTENANCE;
	}

	userlog(DEBUG_LOG, psName, "send_cmd_line_mode_data %d %d\n",
		cmd.onlineMode, cmd.controlMode);
	
	return send_command((char *)&cmd);
}

int send_cmd_test_info_data(char *rcvHeader)
{
	int cmd_id, cmd_size;
	S_P2_SEND_CMD_TEST_INFO_DATA cmd;
	
	cmd_id = P2_CMD_TO_PC_TEST_INFO_DATA;
	cmd_size = sizeof(S_P2_SEND_CMD_TEST_INFO_DATA);
	memset((char *)&cmd, 0, cmd_size);
	make_header2(cmd_id, cmd_size, (char *)&cmd, rcvHeader);

	memcpy((char *)&cmd.testHeader, (char *)&myPs->testCond.header,
		sizeof(S_P2_TEST_HEADER));
	
	return send_command((char *)&cmd);
}

int send_cmd_ch_data(void)
{
	int rtn;

	if(myPs->config.protocol_version == P2_PROTOCOL_VERSION) {
		rtn = send_cmd_ch_data_1();
	} else {
		rtn = send_cmd_ch_data_2();
	}

	return rtn;
}

int send_cmd_ch_data_1(void)
{
	int cmd_id, header_size, body_size, cmd_size;
	int ch, i, group, chInGroup, div, scan_step=0;
	S_P2_CMD_HEADER	header;
	S_P2_CH_DATA body;

	group = (int)myPs->config.groupNo;
	chInGroup = (int)myData->mData.config.chInGroup[group];

	cmd_id = P2_CMD_TO_PC_CH_DATA;
	header_size = sizeof(S_P2_CMD_HEADER);
	body_size = sizeof(S_P2_CH_DATA) * chInGroup;
	cmd_size = header_size + body_size;
	memset((char *)&header, 0, header_size);
	make_header(cmd_id, cmd_size, (char *)&header);

	if(send_command2((char *)&header, header_size) < 0) return -1;

	for(i=0; i < chInGroup; i++) {
		memset((char *)&body, 0, sizeof(S_P2_CH_DATA));

		ch = group * chInGroup + i;

		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
			case F_SDI_5V_450A_200A_100A_10A:
				body.ch = (unsigned short)i;
				if(myData->gData[group].state == G_RUN) {
					if(myData->gData[group].misc.scan_ch[0] == i) {
						ch = 0;
						div = 0;
					} else if((myData->gData[group].misc.scan_ch[1]+12) == i) {
						ch = 0;
						div = 0;
					} else {
						div = 1;
						scan_step = myData->COB_save_step[group][ch];
					}
				} else {
					div = 1;
					scan_step = myData->COB_save_step[group][ch];
				}
				break;
			default:
				body.ch = (unsigned short)myPs->config.ChArray2[i];
				div = 0;
				break;
		}

		if(div == 0) {
			body.state = (unsigned short)convert_ch_state_org_to_p2(
				(long)myData->cData[ch].op.state,
				(long)myData->cData[ch].op.stepType,
				(long)myData->cData[ch].op.attribute,
				(long)myData->cData[ch].op.phase);
			body.code = (unsigned char)convert_ch_code(CONVERT_ORG_TO_P2,
				(long)myData->cData[ch].op.code);
			body.grade = (unsigned char)myData->cData[ch].op.grade;
			body.stepNo = (short int)myData->cData[ch].op.client_stepNo;
			body.totalRunTime
				= (unsigned long)myData->cData[ch].op.totalRunTime / 10;
			body.runTime = (unsigned long)myData->cData[ch].op.runTime / 10;
			body.Vsens = myData->cData[ch].op.Vsens / 100;
			body.Isens = myData->cData[ch].op.Isens / 10;
			body.watt = myData->cData[ch].op.watt * 100;
			body.wattHour = (myData->cData[ch].op.charge_WattHour * 100)
				+ (myData->cData[ch].op.discharge_WattHour * 100);
			body.capacity = (myData->cData[ch].op.charge_AmpareHour / 10)
				+ (myData->cData[ch].op.discharge_AmpareHour / 10);
			body.z = myData->cData[ch].op.z / 10;
		} else { //div == 1
			body.state = (unsigned short)convert_ch_state_org_to_p2(
				(long)myData->COB_opSave[group][ch][scan_step].state,
				(long)myData->COB_opSave[group][ch][scan_step].stepType,
				(long)myData->COB_opSave[group][ch][scan_step].attribute,
				(long)myData->COB_opSave[group][ch][scan_step].phase);
			body.code = (unsigned char)convert_ch_code(CONVERT_ORG_TO_P2,
				(long)myData->COB_opSave[group][ch][scan_step].code);
			body.grade = (unsigned char)myData->
				COB_opSave[group][ch][scan_step].grade;
			body.stepNo = (short int)myData->
				COB_opSave[group][ch][scan_step].client_stepNo;
			body.totalRunTime = (unsigned long)
				myData->COB_opSave[group][ch][scan_step].totalRunTime / 10;
			body.runTime = (unsigned long)myData->
				COB_opSave[group][ch][scan_step].runTime / 10;
			body.Vsens = myData->COB_opSave[group][ch][scan_step].Vsens / 100;
			body.Isens = myData->COB_opSave[group][ch][scan_step].Isens / 10;
			body.watt = myData->COB_opSave[group][ch][scan_step].watt * 100;
			body.wattHour = (myData->COB_opSave[group][ch][scan_step]
				.charge_WattHour * 100)
				+ (myData->COB_opSave[group][ch][scan_step]
				.discharge_WattHour * 100);
			body.capacity = (myData->COB_opSave[group][ch][scan_step]
				.charge_AmpareHour / 10)
				+ (myData->COB_opSave[group][ch][scan_step]
				.discharge_AmpareHour / 10);
			body.z = myData->COB_opSave[group][ch][scan_step].z / 10;
		}

		if(send_command2((char *)&body, sizeof(S_P2_CH_DATA)) < 0) return -2;
	}

	return 0;
}

int send_cmd_ch_data_2(void)
{
	int cmd_id, header_size, body_size, cmd_size;
	int ch, i, group, chInGroup, div, scan_step=0;
	S_P2_CMD_HEADER	header;
	S_P2_CH_DATA2 body;

	group = (int)myPs->config.groupNo;
	chInGroup = (int)myData->mData.config.chInGroup[group];
	//chInGroup = get_chInGroup(group);

	cmd_id = P2_CMD_TO_PC_CH_DATA;
	header_size = sizeof(S_P2_CMD_HEADER);
	if(myData->ChAttribute[0].opType == 0) {
		body_size = sizeof(S_P2_CH_DATA2) * chInGroup;
	} else {
		body_size = sizeof(S_P2_CH_DATA2) * (chInGroup / 2);
	}
	cmd_size = header_size + body_size;
	memset((char *)&header, 0, header_size);
	make_header(cmd_id, cmd_size, (char *)&header);

	if(send_command2((char *)&header, header_size) < 0) return -1;

	for(i=0; i < chInGroup; i++) {
		if(myData->ChAttribute[0].opType == 0) {
		} else {
			if((i % 2) != 0) continue;
		}

		memset((char *)&body, 0, sizeof(S_P2_CH_DATA2));

		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
				ch = 0;
				body.ch = (unsigned short)i;
				if(myData->gData[group].state == G_RUN) {
					if(myData->gData[group].misc.scan_ch[0] == i) {
						ch = 0;
						div = 0;
					} else if((myData->gData[group].misc.scan_ch[1]+12) == i) {
						ch = 0;
						div = 0;
					} else {
						div = 1;
						scan_step = myData->COB_save_step[group][ch];
					}
				} else {
					div = 1;
					scan_step = myData->COB_save_step[group][ch];
				}
				break;
			case F_SDI_5V_450A_200A_100A_10A:
				ch = 0;
				body.ch = (unsigned short)i;
				if(myData->gData[group].state == G_RUN) {
					if(myData->gData[group].misc.scan_ch[0] == i) {
						if(myData->cData[0].op.state == C_RUN) {
							ch = 0;
							div = 0;
						} else {
							div = 1;
							ch = i;
							scan_step = myData->COB_save_step[group][ch];
						}
					} else if((myData->gData[group].misc.scan_ch[1]+12) == i) {
						if(myData->cData[1].op.state == C_RUN) {
							ch = 1;
							div = 0;
						} else {
							div = 1;
							ch = i;
							scan_step = myData->COB_save_step[group][ch];
						}
					} else {
						div = 1;
						ch = i;
						scan_step = myData->COB_save_step[group][ch];
					}
				} else {
					div = 1;
					ch = i;
					scan_step = myData->COB_save_step[group][ch];
				}
				break;
			case F_PNE_5V_15A_30AP_SW:
				ch = group * chInGroup + i; //get_chNo(group, i);
				if(myData->ChAttribute[0].opType == 0) {
					body.ch = (unsigned short)myPs->config.ChArray2[i];
				} else {
					body.ch = (unsigned short)myPs->config.ChArray2[i] / 2;
				}
				div = 0;
				break;
			default:
				ch = group * chInGroup + i;
				body.ch = (unsigned short)myPs->config.ChArray2[i];
				div = 0;
				break;
		}

		if(div == 0) {
			body.state = (unsigned short)convert_ch_state_org_to_p2(
				(long)myData->cData[ch].op.state,
				(long)myData->cData[ch].op.stepType,
				(long)myData->cData[ch].op.attribute,
				(long)myData->cData[ch].op.phase);
			body.code = (unsigned char)convert_ch_code(CONVERT_ORG_TO_P2,
				(long)myData->cData[ch].op.code);
			body.grade = (unsigned char)myData->cData[ch].op.grade;
			body.stepNo = (short int)myData->cData[ch].op.client_stepNo;
			body.runTime = (unsigned long)myData->cData[ch].op.runTime / 10;
			body.totalRunTime
				= (unsigned long)myData->cData[ch].op.totalRunTime / 10;
			body.Vsens = myData->cData[ch].op.Vsens / 100;
			body.Isens = myData->cData[ch].op.Isens / 10;
			body.watt = myData->cData[ch].op.watt * 100;
			body.wattHour = (myData->cData[ch].op.charge_WattHour * 100)
				+ (myData->cData[ch].op.discharge_WattHour * 100);
			body.capacity = (myData->cData[ch].op.charge_AmpareHour / 10)
				+ (myData->cData[ch].op.discharge_AmpareHour / 10);
			body.z = myData->cData[ch].op.z / 10;
			body.ccTime = (unsigned long)myData->cData[ch].op.ccTime / 10;
		} else { //div == 1
			body.state = (unsigned short)convert_ch_state_org_to_p2(
				(long)myData->COB_opSave[group][ch][scan_step].state,
				(long)myData->COB_opSave[group][ch][scan_step].stepType,
				(long)myData->COB_opSave[group][ch][scan_step].attribute,
				(long)myData->COB_opSave[group][ch][scan_step].phase);
			body.code = (unsigned char)convert_ch_code(CONVERT_ORG_TO_P2,
				(long)myData->COB_opSave[group][ch][scan_step].code);
			body.grade = (unsigned char)myData->
				COB_opSave[group][ch][scan_step].grade;
			body.stepNo = (short int)myData->
				COB_opSave[group][ch][scan_step].client_stepNo;
			body.totalRunTime = (unsigned long)myData->
				COB_opSave[group][ch][scan_step].totalRunTime / 10;
			body.runTime = (unsigned long)myData->
				COB_opSave[group][ch][scan_step].runTime / 10;
			switch(myData->AppControl.config.systemModel) {
				case F_SDI_5V_450A_200A_100A_10A:
					if(i < 12) {
						body.Vsens = myData->SubSensV.ch[i].sensV / 100;
					} else {
						body.Vsens = myData->SubSensV.ch[i+20].sensV / 100;
					}
					break;
				default:
					body.Vsens = myData->COB_opSave[group][ch][scan_step]
						.Vsens / 100;
					break;
			}
			body.Isens = myData->COB_opSave[group][ch][scan_step].Isens / 10;
			body.watt = myData->COB_opSave[group][ch][scan_step].watt * 100;
			body.wattHour = (myData->COB_opSave[group][ch][scan_step]
				.charge_WattHour * 100)
				+ (myData->COB_opSave[group][ch][scan_step]
				.discharge_WattHour * 100);
			body.capacity = (myData->COB_opSave[group][ch][scan_step]
				.charge_AmpareHour / 10)
				+ (myData->COB_opSave[group][ch][scan_step]
				.discharge_AmpareHour / 10);
			body.z = myData->COB_opSave[group][ch][scan_step].z / 10;
			body.ccTime = (unsigned long)myData->
				COB_opSave[group][ch][scan_step].ccTime / 10;
		}

		if(send_command2((char *)&body, sizeof(S_P2_CH_DATA2)) < 0) {
			return -2;
		}
	}

	return 0;
}

int send_cmd_step_data(void)
{
	int rtn;

	if(myPs->config.protocol_version == P2_PROTOCOL_VERSION) {
		rtn = send_cmd_step_data_1();
	} else {
		rtn = send_cmd_step_data_2();
	}

	return rtn;
}

int send_cmd_step_data_1(void)
{ //kjg_110921
	unsigned char stepNo;
	int cmd_id, header_size, body1_size, body2_size, cmd_size;
	int fromPs, idx, count, group, chInGroup, ch, i, fault_count, code;
	S_P2_CMD_HEADER header;
	S_P2_STEP_END_HEADER body1;
	S_P2_CH_DATA data[MAX_CH_PER_MODULE];

	group = (int)myPs->config.groupNo;

	fromPs = SAVE_GROUP_TO_COB1 + group;

	if(myData->f_save_msg[fromPs].write_idx
		== myData->f_save_msg[fromPs].read_idx) {
		myData->f_save_msg[fromPs].count = 0;
		return 0;
	}

	myData->f_save_msg[fromPs].read_idx++;
	if(myData->f_save_msg[fromPs].read_idx >= MAX_F_SAVE_MSG)
		myData->f_save_msg[fromPs].read_idx = 0;
	idx = myData->f_save_msg[fromPs].read_idx;

	if(myData->f_save_msg[fromPs].count > 0) {
		myData->f_save_msg[fromPs].count--;
	}
	count = myData->f_save_msg[fromPs].count;

	if(myPs->config.workMode == P2_WORK_ONLINE) return count;

//	userlog(DEBUG_LOG, psName, "cmd_step_data type:%d, attr:%d\n",
//		myData->f_save_msg[fromPs].val[idx].type,
//		myData->f_save_msg[fromPs].val[idx].attribute); //kjgd

	chInGroup = (int)myData->mData.config.chInGroup[group];
	fault_count = 0;
	for(i=0; i < chInGroup; i++) {
		ch = group * chInGroup + i;
		code = myData->f_save_msg[fromPs].val[idx].chData[ch].code;
		if(code == P2_C_CD_NONE || (code >= P2_C_CD_END_CODE_START
			&& code <= P2_C_CD_FAULT_CHECK_CODE_START-1)) {
		} else fault_count++;
	}

	i = 0; //0:return, 1:check_return, 2:send
	switch(myData->f_save_msg[fromPs].val[idx].type) {
		case P2_STEP_CYCLE:
			i = 0;
			break;
		case P2_STEP_LOOP:
			switch(myData->f_save_msg[fromPs].val[idx].attribute) {
				case ATTR_CHECK1_END:
				case ATTR_CHECK2_END:
				case ATTR_CHECK3_END:
					i = 1;
					//i = 0;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case P2_STEP_CHARGE:
		case P2_STEP_DISCHARGE:
			switch(myData->f_save_msg[fromPs].val[idx].attribute) {
				case ATTR_CHECK1_1:
				case ATTR_CHECK1_2:
				case ATTR_CHECK2_1:
				case ATTR_CHECK2_2:
				case ATTR_CHECK3_1:
				case ATTR_CHECK3_2:
					if(fault_count == chInGroup) i = 1;
					else i = 0;
					//i = 0;
					break;
				case ATTR_IDLE:
				case ATTR_DC1_1:
				case ATTR_DC2_3:
				case ATTR_DC3_3:
					i = 2;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case P2_STEP_OCV:
		case P2_STEP_REST:
		case P2_STEP_LONG_TIME_REST:
			switch(myData->f_save_msg[fromPs].val[idx].attribute) {
				case ATTR_CHECK1_1:
				case ATTR_CHECK1_2:
				case ATTR_CHECK2_1:
				case ATTR_CHECK2_2:
				case ATTR_CHECK3_1:
				case ATTR_CHECK3_2:
					if(fault_count == chInGroup) i = 1;
					else i = 0;
					//i = 0;
					break;
				case ATTR_IDLE:
					i = 2;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case P2_STEP_END:
			i = 2;
			break;
		default:
			i = 0;
			break;
	}

	if(i == 0) {
		return count;
	} else if(i == 1) {
		i = send_cmd_check_data(group, fromPs, idx);
		return count;
	}

	memset((char *)&header, 0, sizeof(S_P2_CMD_HEADER));
	memset((char *)&body1, 0, sizeof(S_P2_STEP_END_HEADER));
	memset((char *)&data, 0, sizeof(S_P2_CH_DATA) * MAX_CH_PER_MODULE);

	cmd_id = P2_CMD_TO_PC_STEP_DATA;
	header_size = sizeof(S_P2_CMD_HEADER);
	body1_size = sizeof(S_P2_STEP_END_HEADER);
	body2_size = sizeof(S_P2_CH_DATA) * chInGroup;
	cmd_size = header_size + body1_size + body2_size;

	make_header(cmd_id, cmd_size, (char *)&header);

	stepNo = 0;
	for(i=0; i < chInGroup; i++) {
		ch = group * chInGroup + i;
		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
			case F_SDI_5V_450A_200A_100A_10A: //kjgw
				data[i].ch = (unsigned short)i;
				break;
			default:
				data[i].ch = (unsigned short)myPs->config.ChArray2[i];
				break;
		}

		stepNo = myData->f_save_msg[fromPs].val[idx].chData[ch].stepNo; //kjgw_f
		data[i].state = (unsigned short)myData->f_save_msg[fromPs]
			.val[idx].chData[ch].state;
		data[i].code = (unsigned char)myData->f_save_msg[fromPs]
			.val[idx].chData[ch].code;
		data[i].grade = (unsigned char)myData->f_save_msg[fromPs]
			.val[idx].chData[ch].grade; //kjgw_f
		data[i].stepNo = (short int)myData->f_save_msg[fromPs]
			.val[idx].chData[ch].stepNo;
		data[i].totalRunTime = (unsigned long)myData->f_save_msg[fromPs]
			.val[idx].chData[ch].totalRunTime / 10;
		data[i].runTime = (unsigned long)myData->f_save_msg[fromPs]
			.val[idx].chData[ch].runTime / 10;
		data[i].Vsens = myData->f_save_msg[fromPs]
			.val[idx].chData[ch].Vsens / 100;
		data[i].Isens = myData->f_save_msg[fromPs]
			.val[idx].chData[ch].Isens / 10;
		data[i].watt = myData->f_save_msg[fromPs]
			.val[idx].chData[ch].watt * 100;
		data[i].wattHour = (myData->f_save_msg[fromPs]
			.val[idx].chData[ch].charge_WattHour * 100)
			+ (myData->f_save_msg[fromPs]
			.val[idx].chData[ch].discharge_WattHour * 100);
		data[i].capacity = (myData->f_save_msg[fromPs]
			.val[idx].chData[ch].charge_AmpareHour / 10)
			+ (myData->f_save_msg[fromPs]
			.val[idx].chData[ch].discharge_AmpareHour / 10);
		data[i].z = myData->f_save_msg[fromPs].val[idx].chData[ch].z / 10;
	}
	
	body1.stepNo = (unsigned short)stepNo; //kjgw_f

	if(send_command2((char *)&header, header_size) < 0) return -1;
	if(send_command2((char *)&body1, body1_size) < 0) return -2;
	for(i=0; i < chInGroup; i++) {
		if(send_command2((char *)&data[i], sizeof(S_P2_CH_DATA)) < 0)
			return -3;
	}

	userlog(DEBUG_LOG, psName, "send_cmd_step_data - type:%d, attr:%d\n",
		myData->f_save_msg[fromPs].val[idx].type,
		myData->f_save_msg[fromPs].val[idx].attribute);

	return count;
}

int send_cmd_step_data_2(void)
{ //kjg_110921
	unsigned char stepNo;
	int cmd_id, header_size, body1_size, body2_size, cmd_size, ng_count=0;
	int fromPs, idx, count, group, chInGroup, ch, i, fault_count, code;
	S_P2_CMD_HEADER header;
	S_P2_STEP_END_HEADER body1;
	S_P2_CH_DATA2 data[MAX_CH_256];

	group = (int)myPs->config.groupNo;

	fromPs = SAVE_GROUP_TO_COB1 + group;

	if(myData->f_save_msg[fromPs].write_idx
		== myData->f_save_msg[fromPs].read_idx) {
		myData->f_save_msg[fromPs].count = 0;
		return 0;
	}

	myData->f_save_msg[fromPs].read_idx++;
	if(myData->f_save_msg[fromPs].read_idx >= MAX_F_SAVE_MSG)
		myData->f_save_msg[fromPs].read_idx = 0;
	idx = myData->f_save_msg[fromPs].read_idx;

	if(myData->f_save_msg[fromPs].count > 0) {
		myData->f_save_msg[fromPs].count--;
	}
	count = myData->f_save_msg[fromPs].count;

	if(myPs->config.workMode == P2_WORK_ONLINE) return count;

	//userlog(DEBUG_LOG, psName, "cmd_step_data type:%d, attr:%d\n",
	//	myData->f_save_msg[fromPs].val[idx].type,
	//	myData->f_save_msg[fromPs].val[idx].attribute);

	chInGroup = (int)myData->mData.config.chInGroup[group];
	//chInGroup = get_chInGroup(group);
	fault_count = 0;
	for(i=0; i < chInGroup; i++) {
		if(myData->ChAttribute[0].opType == 0) {
		} else {
			if((i % 2) != 0) continue;
		}

		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_400A_200A_100A_10A_2:
			case F_SDI_5V_450A_200A_100A_10A:
				ch = i;
				break;
			default:
				ch = group * chInGroup + i;
				break;
		}
		code = myData->f_save_msg[fromPs].val[idx].chData[ch].code;
		if(code == P2_C_CD_NONE || (code >= P2_C_CD_END_CODE_START
			&& code < P2_C_CD_FAULT_CHECK_CODE_START)) {
		} else fault_count++;
	}

	i = 0; //0:return, 1:check_return, 2:send
	switch(myData->f_save_msg[fromPs].val[idx].type) {
		case P2_STEP_CYCLE:
			i = 0;
			break;
		case P2_STEP_LOOP:
			switch(myData->f_save_msg[fromPs].val[idx].attribute) {
				case ATTR_CHECK1_END:
				case ATTR_CHECK2_END:
				case ATTR_CHECK3_END:
					i = 1;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case P2_STEP_CHARGE:
		case P2_STEP_DISCHARGE:
			switch(myData->f_save_msg[fromPs].val[idx].attribute) {
				case ATTR_CHECK1_1:
				case ATTR_CHECK1_2:
				case ATTR_CHECK2_1:
				case ATTR_CHECK2_2:
				case ATTR_CHECK3_1:
				case ATTR_CHECK3_2:
					switch(myData->AppControl.config.systemModel) {
						case F_SDI_5V_400A_200A_100A_10A:
						case F_SDI_5V_400A_200A_100A_10A_2:
						case F_SDI_5V_450A_200A_100A_10A: //kjgw
							i = 0;
							break;
						default:
							if(myData->ChAttribute[0].opType == 0) {
								if(fault_count == chInGroup) i = 1;
								else i = 0;
							} else {
								if(fault_count == (chInGroup / 2)) i = 1;
								else i = 0;
							}
							break;
					}
					break;
				case ATTR_IDLE:
				case ATTR_DC1_1:
				case ATTR_DC2_3:
				case ATTR_DC3_3:
					i = 2;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case P2_STEP_OCV:
		case P2_STEP_REST:
		case P2_STEP_LONG_TIME_REST:
			switch(myData->f_save_msg[fromPs].val[idx].attribute) {
				case ATTR_CHECK1_1:
				case ATTR_CHECK1_2:
				case ATTR_CHECK2_1:
				case ATTR_CHECK2_2:
				case ATTR_CHECK3_1:
				case ATTR_CHECK3_2:
					switch(myData->AppControl.config.systemModel) {
						case F_SDI_5V_400A_200A_100A_10A:
						case F_SDI_5V_400A_200A_100A_10A_2:
						case F_SDI_5V_450A_200A_100A_10A: //kjgw
							i = 0;
							break;
						default:
							if(myData->ChAttribute[0].opType == 0) {
								if(fault_count == chInGroup) i = 1;
								else i = 0;
							} else {
								if(fault_count == (chInGroup / 2)) i = 1;
								else i = 0;
							}
							break;
					}
					break;
				case ATTR_IDLE:
					i = 2;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case P2_STEP_END:
			i = 2;
			break;
		default:
			i = 0;
			break;
	}

	if(i == 0) {
		return count;
	} else if(i == 1) {
		userlog(DEBUG_LOG, psName,
			"send_cmd_check_data - type:%d, attr:%d, fault_count:%d\n",
			myData->f_save_msg[fromPs].val[idx].type,
			myData->f_save_msg[fromPs].val[idx].attribute, fault_count);

		i = send_cmd_check_data(group, fromPs, idx);
		return count;
	}

	memset((char *)&header, 0, sizeof(S_P2_CMD_HEADER));
	memset((char *)&body1, 0, sizeof(S_P2_STEP_END_HEADER));
	memset((char *)&data, 0, sizeof(S_P2_CH_DATA2) * MAX_CH_256);

	cmd_id = P2_CMD_TO_PC_STEP_DATA;
	header_size = sizeof(S_P2_CMD_HEADER);
	body1_size = sizeof(S_P2_STEP_END_HEADER);
	if(myData->ChAttribute[0].opType == 0) {
		body2_size = sizeof(S_P2_CH_DATA2) * chInGroup;
	} else {
		body2_size = sizeof(S_P2_CH_DATA2) * (chInGroup / 2);
	}
	cmd_size = header_size + body1_size + body2_size;

	make_header(cmd_id, cmd_size, (char *)&header);

	stepNo = 0;
	ng_count = 0;
	for(i=0; i < chInGroup; i++) {
		if(myData->ChAttribute[0].opType == 0) {
		} else {
			if((i % 2) != 0) continue;
		}

		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
			case F_SDI_5V_450A_200A_100A_10A:
				ch = i;
				data[i].ch = (unsigned short)i;
				break;
			case F_PNE_5V_15A_30AP_SW:
				if(myData->ChAttribute[0].opType == 0) {
					ch = group * chInGroup + i;
					data[i].ch = (unsigned short)myPs->config.ChArray2[i];
				} else {
					ch = group * chInGroup + i;
					data[i].ch = (unsigned short)myPs->config.ChArray2[i] / 2;
				}
				break;
			default:
				ch = group * chInGroup + i;
				data[i].ch = (unsigned short)myPs->config.ChArray2[i];
				break;
		}

		stepNo = myData->f_save_msg[fromPs].val[idx].chData[ch].stepNo; //kjgw_f
		data[i].state = (unsigned short)myData->f_save_msg[fromPs]
			.val[idx].chData[ch].state;
		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_400A_200A_100A_10A_2:
				if(myPs->misc.nonCell[i] == 0) { //normal
					data[i].code = (unsigned char)myData->f_save_msg[fromPs]
						.val[idx].chData[ch].code;
				} else { //nonCell
					data[i].code = P2_C_CD_NONCELL;
				}
				break;
			default:
				data[i].code = (unsigned char)myData->f_save_msg[fromPs]
					.val[idx].chData[ch].code;
				break;
		}
		data[i].grade = (unsigned char)myData->f_save_msg[fromPs]
			.val[idx].chData[ch].grade; //kjgw_f
		data[i].stepNo = (short int)myData->f_save_msg[fromPs]
			.val[idx].chData[ch].stepNo;
		data[i].totalRunTime = (unsigned long)myData->f_save_msg[fromPs]
			.val[idx].chData[ch].totalRunTime / 10;
		data[i].runTime = (unsigned long)myData->f_save_msg[fromPs]
			.val[idx].chData[ch].runTime / 10;
		data[i].Vsens = myData->f_save_msg[fromPs]
			.val[idx].chData[ch].Vsens / 100;
		data[i].Isens = myData->f_save_msg[fromPs]
			.val[idx].chData[ch].Isens / 10;
		data[i].watt = myData->f_save_msg[fromPs]
			.val[idx].chData[ch].watt * 100;
		data[i].wattHour = (myData->f_save_msg[fromPs]
			.val[idx].chData[ch].charge_WattHour * 100)
			+ (myData->f_save_msg[fromPs]
			.val[idx].chData[ch].discharge_WattHour * 100);
		data[i].capacity = (myData->f_save_msg[fromPs]
			.val[idx].chData[ch].charge_AmpareHour / 10)
			+ (myData->f_save_msg[fromPs]
			.val[idx].chData[ch].discharge_AmpareHour / 10);
		data[i].z = myData->f_save_msg[fromPs].val[idx].chData[ch].z / 10;
		data[i].ccTime = (unsigned long)myData->f_save_msg[fromPs]
			.val[idx].chData[ch].ccTime / 10; //ccTime

		if(data[i].code == P2_C_CD_NONE
			|| (data[i].code >= P2_C_CD_END_CODE_START
			&& data[i].code < P2_C_CD_FAULT_CHECK_CODE_START)) {
		} else ng_count++;
	}
	
	body1.stepNo = (unsigned short)stepNo; //kjgw_f

	if(send_command2((char *)&header, header_size) < 0) return -1;
	if(send_command2((char *)&body1, body1_size) < 0) return -2;
	for(i=0; i < chInGroup; i++) {
		if(myData->ChAttribute[0].opType == 0) {
		} else {
			if((i % 2) != 0) continue;
		}

		if(send_command2((char *)&data[i], sizeof(S_P2_CH_DATA2)) < 0)
			return -3;
	}

	userlog(DEBUG_LOG, psName, "send_cmd_step_data - type:%d, attr:%d, ng:%d\n",
		myData->f_save_msg[fromPs].val[idx].type,
		myData->f_save_msg[fromPs].val[idx].attribute, ng_count);

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_50A_5A:
		case F_PNE_5V_15A_30AP_SW:
			if(ng_count == chInGroup) {
				i = send_cmd_all_ng();
			}
			break;
		default:	break;
	}

	return count;
}

int send_cmd_check_data(int group, int fromPs, int idx)
{
	int rtn;

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_50A_5A:
			rtn = send_cmd_check_data_2(group, fromPs, idx);
			break;
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
		case F_SDI_5V_450A_200A_100A_10A: //kjgw
		case F_PNE_5V_15A_30AP_SW:
			rtn = send_cmd_check_data_3(group, fromPs, idx);
			break;
		default:
			rtn = send_cmd_check_data_1(group, fromPs, idx);
			break;
	}

	return rtn;
}

int send_cmd_all_ng(void)
{
	int cmd_id, cmd_size;
	S_P2_SEND_CMD_ALL_NG cmd;

	cmd_id = P2_CMD_TO_PC_ALL_NG;
	cmd_size = sizeof(S_P2_SEND_CMD_ALL_NG);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	return send_command((char *)&cmd);
}

int send_cmd_check_data_1(int group, int fromPs, int idx)
{
	int cmd_id, cmd_size, code;
	int chInGroup, ch, i, normalChNo;
	S_P2_SEND_CMD_CHECK_DATA cmd;

	cmd_id = P2_CMD_TO_PC_CHECK_DATA;
	cmd_size = sizeof(S_P2_SEND_CMD_CHECK_DATA);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	chInGroup = (int)myData->mData.config.chInGroup[group];

	normalChNo = 0;
	for(i=0; i < chInGroup; i++) {
		ch = group * chInGroup + i;
		code = myData->f_save_msg[fromPs].val[idx].chData[ch].code;
		if(code == C_CD_END_CHECK) {
			normalChNo++;
		} else cmd.normalChCode[i] = 1; //kjgw_f
	}

	cmd.normalChNo = (unsigned short)normalChNo;

	userlog(DEBUG_LOG, psName, "normalChNo %d\n", normalChNo);
	
	return send_command((char *)&cmd);
}

int send_cmd_check_data_2(int group, int fromPs, int idx)
{
	int cmd_id, cmd_size, code;
	int chInGroup, ch, i, normalChNo, monitor_ch;
	S_P2_SEND_CMD_CHECK_DATA2 cmd;

	cmd_id = P2_CMD_TO_PC_CHECK_DATA;
	cmd_size = sizeof(S_P2_SEND_CMD_CHECK_DATA2);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	chInGroup = (int)myData->mData.config.chInGroup[group];

	normalChNo = 0;
	userlog(DEBUG_LOG, psName, "check_data ");
	for(i=0; i < chInGroup; i++) {
		ch = group * chInGroup + i;
		monitor_ch = (int)myPs->config.ChArray2[i];

		code = myData->f_save_msg[fromPs].val[idx].chData[ch].code;

		//myPs->misc.nonCell[ch] = 1; //kjgd

		if(myPs->misc.nonCell[ch] == 0 && code == P2_C_CD_END_T) {
			cmd.normalChCode[monitor_ch] = 0; //step run
			cmd.code[monitor_ch] = (unsigned char)code;
		} else if(myPs->misc.nonCell[ch] == 1 && code != P2_C_CD_END_T) {
			cmd.normalChCode[monitor_ch] = 1; //fault
			cmd.code[monitor_ch] = (unsigned char)code;
		} else if(myPs->misc.nonCell[ch] == 0 && code != P2_C_CD_END_T) {
			cmd.normalChCode[monitor_ch] = 2; //fault ERROR_NO
			cmd.code[monitor_ch] = (unsigned char)P2_C_CD_FAULT_CHECK_ERROR_NO;
			myData->cData[ch].op.code = C_CD_FAULT_CHECK_ERROR_NO;
			myData->cData[ch].opSave.code = C_CD_FAULT_CHECK_ERROR_NO;
		//} else if(myPs->misc.nonCell[ch] == 1 && code == P2_C_CD_END_T) {
		} else {
			cmd.normalChCode[monitor_ch] = 3; //step run -> fault ERROR_YES
			cmd.code[monitor_ch] = (unsigned char)P2_C_CD_FAULT_CHECK_ERROR_YES;
			myData->cData[ch].op.code = C_CD_FAULT_CHECK_ERROR_YES;
			myData->cData[ch].opSave.code = C_CD_FAULT_CHECK_ERROR_YES;
		}

		if(cmd.code[monitor_ch] == P2_C_CD_END_T) {
			normalChNo++;
		}

		userlog2(DEBUG_LOG, psName, "%d(%d:%d:%d) ", monitor_ch+1,
			myPs->misc.nonCell[ch], code, cmd.normalChCode[monitor_ch]);
	}
	userlog2(DEBUG_LOG, psName, "\n");

	cmd.normalChNo = (unsigned short)normalChNo;

	userlog(DEBUG_LOG, psName, "normalChNo %d\n", normalChNo);
	
	return send_command((char *)&cmd);
}

int send_cmd_check_data_3(int group, int fromPs, int idx)
{
	int cmd_id, cmd_size, code;
	int chInGroup, ch, i, normalChNo, monitor_ch;
	S_P2_SEND_CMD_CHECK_DATA2 cmd;

	cmd_id = P2_CMD_TO_PC_CHECK_DATA;
	cmd_size = sizeof(S_P2_SEND_CMD_CHECK_DATA2);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	chInGroup = (int)myData->mData.config.chInGroup[group];

	normalChNo = 0;
	userlog(DEBUG_LOG, psName, "check_data ");
	for(i=0; i < chInGroup; i++) {
		if(myData->ChAttribute[0].opType == 0) {
		} else {
			if((i % 2) != 0) continue;
		}

		ch = i; //group * chInGroup + i;
		if(myData->ChAttribute[0].opType == 0) {
			monitor_ch = i;//(int)myPs->config.ChArray2[i];
		} else {
			monitor_ch = i / 2;//(int)myPs->config.ChArray2[i];
		}

		code = myData->f_save_msg[fromPs].val[idx].chData[ch].code;

		//myPs->misc.nonCell[ch] = 1; //kjgd

		if(myPs->misc.nonCell[ch] == 0 && code == P2_C_CD_END_T) {
			cmd.normalChCode[monitor_ch] = 0; //step run
			cmd.code[monitor_ch] = (unsigned char)code;
		} else if(myPs->misc.nonCell[ch] == 1 && code != P2_C_CD_END_T) {
			cmd.normalChCode[monitor_ch] = 1; //fault
			cmd.code[monitor_ch] = (unsigned char)code;
		} else if(myPs->misc.nonCell[ch] == 0 && code != P2_C_CD_END_T) {
			cmd.normalChCode[monitor_ch] = 2; //fault ERROR_NO
			if(code == P2_C_CD_FAULT_CHECK_CONTACT_BAD2) { //kjg_update_100215
				cmd.code[monitor_ch] = (unsigned char)code;
			} else {
				//cmd.code[monitor_ch]
				//	= (unsigned char)P2_C_CD_FAULT_CHECK_ERROR_NO;
				cmd.code[monitor_ch] = (unsigned char)code;
				myData->COB_opSave[group][ch][1].code
					= C_CD_FAULT_CHECK_ERROR_NO;
			}
		//} else if(myPs->misc.nonCell[ch] == 1 && code == P2_C_CD_END_T) {
		} else {
			cmd.normalChCode[monitor_ch] = 3; //step run -> fault ERROR_YES
			cmd.code[monitor_ch] = (unsigned char)P2_C_CD_FAULT_CHECK_ERROR_YES;
			myData->COB_opSave[group][ch][1].code = C_CD_FAULT_CHECK_ERROR_YES;
		}

		if(cmd.code[monitor_ch] == P2_C_CD_END_T) {
			normalChNo++;
		}

		userlog2(DEBUG_LOG, psName, "%d(%d:%d:%d) ", monitor_ch+1,
			myPs->misc.nonCell[ch], code, cmd.normalChCode[monitor_ch]);
	}
	userlog2(DEBUG_LOG, psName, "\n");

	cmd.normalChNo = (unsigned short)normalChNo;

	userlog(DEBUG_LOG, psName, "normalChNo %d\n", normalChNo);
	
	return send_command((char *)&cmd);
}

int send_cmd_sensor_data(void)
{
	int group, jig, cmd_id, cmd_size, i, chInGroup;
//kjgw   	int bd, ch,	i, j;
	S_P2_SEND_CMD_SENSOR_DATA cmd;

	group = (int)myPs->config.groupNo;
	jig = groupNo_to_jigNo(group);
	if(jig > 0) jig--;
	else jig = 0;

	cmd_id = P2_CMD_TO_PC_SENSOR_DATA;
	cmd_size = sizeof(S_P2_SEND_CMD_SENSOR_DATA);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

//	userlog(DEBUG_LOG, psName, "temp%d %ld\n", group,
//		myData->ioComm.temperature[jig][0]); //kjgd

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_50A_5A:
		case F_SDI_5V_400A_200A_100A_10A:
			chInGroup = myData->mData.config.chInGroup[group];
			for(i=0; i < chInGroup; i++) {
				if(group == 0) {
					cmd.sensorData1[i].value = myData->cData[i].op.temp / 10;
				} else if(group == 1) {
					cmd.sensorData1[i].value
						= myData->cData[group * chInGroup + i].op.temp / 10;
				}
			}
			break;
		case F_SDI_5V_400A_200A_100A_10A_2:
		case F_SDI_5V_450A_200A_100A_10A: //kjgw
			chInGroup = myData->mData.config.chInGroup[group];
			for(i=0; i < chInGroup; i++) {
				cmd.sensorData1[i].value = myData->cData[i].op.temp / 10;
			}
			break;
		default:	break;
	}

	return send_command((char *)&cmd);
}

int send_cmd_chamber_data(void)
{
	int group, jig, cmd_id, cmd_size, i, val;
	S_P2_SEND_CMD_CHAMBER_DATA cmd;

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_450A_200A_100A_10A:
			i = 1;
			break;
		default:
			i = 0;
			break;
	}
	if(i == 0) return 0;

	for(i=0; i < MAX_COM_PORT; i++) {
		if(myData->COM.config.functionType[i] == COM_FUNC_TYPE_CHAMBER1) {
			break;
		}
	}
	if(i >= MAX_COM_PORT) return 0;

	group = (int)myPs->config.groupNo;
	jig = groupNo_to_jigNo(group);
	if(jig > 0) jig--;
	else jig = 0;

	cmd_id = P2_CMD_TO_PC_CHAMBER_DATA;
	cmd_size = sizeof(S_P2_SEND_CMD_CHAMBER_DATA);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	//0:CHMABER_STOP, 1:RUN, 2:PAUSE;
	if((myData->COM.com_port[i].misc.OTHERSTS & 0x0010) != 0x0000) { //RUN
		cmd.chamber.mode = 1;
	} else {
		if((myData->COM.com_port[i].misc.NOWSTS & 0x0008) != 0x0000) { //HOLD
			cmd.chamber.mode = 2;
		} else {
			cmd.chamber.mode = 0;
		}
	}

	if(myData->jData[jig].signal[J_SIG_DOOR_OPEN] == P1) {
		cmd.chamber.door_state = 0; //0:CHAMBER_DOOR_OPEN, 1:CLOSE;
	} else {
		cmd.chamber.door_state = 1; //0:CHAMBER_DOOR_OPEN, 1:CLOSE;
	}

	cmd.chamber.current_temp = myData->COM.com_port[i].misc.TEMP_NPV * 10;
	cmd.chamber.current_humidity = 0;
	val = myData->COM.com_port[i].misc.PROC_TIME_H * 3600
		+ myData->COM.com_port[i].misc.PROC_TIME_L * 60;
	cmd.chamber.current_time = val;

	cmd.chamber.set_temp = myData->COM.com_port[i].misc.TEMP_NSP * 10;
	cmd.chamber.set_humidity = 0;
	cmd.chamber.set_time = 0;

//	userlog(DEBUG_LOG, psName,
//		"send_cmd_chamber_data curr_temp:%d, set_temp:%d, curr_time:%d\n",
//		cmd.chamber.current_temp, cmd.chamber.set_temp,
//		cmd.chamber.current_time); //kjgd

	return send_command((char *)&cmd);
}

int send_cmd_meter_connect_reply(char *rcvHeader)
{
	int cmd_id, cmd_size;
	S_P2_SEND_CMD_METER_CONNECT_REPLY cmd;

	cmd_id = P2_CMD_TO_PC_METER_CONNECT_REPLY;
	cmd_size = sizeof(S_P2_SEND_CMD_METER_CONNECT_REPLY);
	memset((char *)&cmd, 0, cmd_size);
	make_header2(cmd_id, cmd_size, (char *)&cmd, rcvHeader);
	
	/*kjgw cmd.header.totalCh = 1; //disconnect:0, connect:1
	cmd.header.chFlag1 = 0;
	cmd.header.chFlag2 = 0;
	*/
	return send_command((char *)&cmd);
}

int send_cmd_comm_check_reply(char *rcvHeader)
{
	int cmd_id, cmd_size;
	S_P2_SEND_CMD_COMM_CHECK_REPLY	cmd;
	
	cmd_id = P2_CMD_TO_PC_COMM_CHECK_REPLY;
	cmd_size = sizeof(S_P2_SEND_CMD_COMM_CHECK_REPLY);
	memset((char *)&cmd, 0, cmd_size);
	make_header2(cmd_id, cmd_size, (char *)&cmd, rcvHeader);

	return send_command((char *)&cmd);
}

int send_cmd_comm_check(void)
{
	int cmd_id, cmd_size;
	S_P2_SEND_CMD_COMM_CHECK	cmd;
	
	cmd_id = P2_CMD_TO_PC_COMM_CHECK;
	cmd_size = sizeof(S_P2_SEND_CMD_COMM_CHECK);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	return send_command((char *)&cmd);
}

int send_cmd_bcr_info(int group)
{
	char test_tray_id[P2_BCR_SIZE];
	int cmd_id, cmd_size;
	S_P2_CMD_BCR_INFO	cmd;

	memset((char *)&test_tray_id[0], 0, sizeof(P2_BCR_SIZE));
	//sprintf((char *)&test_tray_id[0], "K00002");
	//sprintf((char *)&test_tray_id[0], "TA006");
	sprintf((char *)&test_tray_id[0], "GA006");
	
	cmd_id = P2_CMD_TO_PC_BCR_INFO;
	cmd_size = sizeof(S_P2_CMD_BCR_INFO);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	if(myData->AppControl.config.debugType == 100) { //test_mode
		memcpy((char *)&cmd.bcr, (char *)&test_tray_id[0], P2_BCR_SIZE);
	} else { //normal_mode
		memcpy((char *)&cmd.bcr, (char *)&myData->gData[group].tray_id,
			P2_BCR_SIZE);
	}

	userlog(DEBUG_LOG, psName, "Tray_Id %s\n", cmd.bcr);

	return send_command((char *)&cmd);
}

int send_cmd_user_cmd(int code)
{
	int cmd_id, cmd_size;
	S_P2_SEND_CMD_USER_CMD	cmd;
	
	cmd_id = P2_CMD_TO_PC_USER_CMD;
	cmd_size = sizeof(S_P2_SEND_CMD_USER_CMD);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);
/*kjgw_f
	switch(code){
		case MSG_MODULE_SUB_START:
			cmd_id = P2_CMD_TO_SBC_RUN;
			break;
		case MSG_MODULE_SUB_STOP:
			cmd_id = P2_CMD_TO_SBC_STOP;
			break;
		case MSG_MODULE_SUB_CONTINUE:
			cmd_id = P2_CMD_TO_SBC_CONTINUE;
			break;
		case MSG_MODULE_SUB_PAUSE:
			cmd_id = P2_CMD_TO_SBC_PAUSE;
			break;
		case MSG_MODULE_SUB_RESET:
			cmd_id = P2_CMD_TO_SBC_RESET;
			break;
		default:
			return -1;
			break;
	}

	cmd.cmd = cmd_id;
*/
	return send_command((char *)&cmd);
}

int send_cmd_dcir_real_time_data(void)
{/*kjgw_f kjg_110921
	unsigned char stepType, stepNo;
	int cmd_id, header_size, body_size, cmd_size;
	int fromPs, idx, count, group, chInGroup, chIdx, bd, ch, i;
	S_P2_CMD_HEADER header;
	S_P2_SEND_CMD_REAL_TIME_DATA data[MAX_CH_PER_MODULE];

	group = (int)myPs->config.groupNo;

	fromPs = SAVE_DCIR_GROUP_TO_SUB1 + group;

	if(myData->save_dcir_msg[fromPs].write_idx
		== myData->save_dcir_msg[fromPs].read_idx) {
		myData->save_dcir_msg[fromPs].count = 0;
		return 0;
	}

	myData->save_dcir_msg[fromPs].read_idx++;
	if(myData->save_dcir_msg[fromPs].read_idx >= MAX_SAVE_MSG)
		myData->save_dcir_msg[fromPs].read_idx = 0;
	idx = myData->save_dcir_msg[fromPs].read_idx;

	if(myData->save_dcir_msg[fromPs].count > 0) {
		myData->save_dcir_msg[fromPs].count--;
	}
	count = myData->save_dcir_msg[fromPs].count;

	if(myPs->config.workMode == WORK_ONLINE) return count;

	i = 0; //0:return, 2:send
	switch(myData->save_msg[fromPs].val[idx].type) {
		case STEP_ADV_CYCLE:
			i = 0;
			break;
		case STEP_LOOP:
			switch(myData->save_msg[fromPs].val[idx].attribute) {
				case ATTR_CHECK1_END:
				case ATTR_CHECK2_END:
					i = 0;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case STEP_CHARGE:
		case STEP_DISCHARGE:
			switch(myData->save_msg[fromPs].val[idx].attribute) {
				case ATTR_IDLE:
					i = 0;
					break;
				case ATTR_DC1_1:
				case ATTR_DC2_3:
				case ATTR_DC3_3:
					i = 2;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case STEP_OCV:
		case STEP_REST:
		case STEP_LONG_TIME_REST:
			switch(myData->save_msg[fromPs].val[idx].attribute) {
				case ATTR_IDLE:
					i = 0;
					break;
				default:
					i = 0;
					break;
			}
			break;
		case STEP_END:
			i = 1;
			break;
		default:
			i = 0;
			break;
	}

	if(i == 0){
		return count;
	}else if(i == 1){
		return 0;
	}

	memset((char *)&header, 0, sizeof(S_P2_CMD_HEADER));
	memset((char *)&data, 0, sizeof(S_P2_SEND_CMD_REAL_TIME_DATA) * MAX_CH_PER_MODULE);

	chInGroup = (int)myData->mData.config.chInGroup[group];
	cmd_id = P2_CMD_TO_PC_REAL_TIME_DATA;
	header_size = sizeof(S_P2_CMD_HEADER);
	body_size = sizeof(S_P2_SEND_CMD_REAL_TIME_DATA) * chInGroup;
	cmd_size = header_size + body_size;

	make_header(cmd_id, cmd_size, (char *)&header);

	stepType = 0; stepNo = 0;
	for(bd=0; bd < MAX_BD_PER_MODULE; bd++) {
		if(bdNo_to_groupNo(bd) != group+1) continue;
	
		for(ch=0; ch < myData->mData.config.chPerBd; ch++) {
			i = bd * myData->mData.config.chPerBd + ch;
			i = i - group * chInGroup;
			chIdx = (int)myPs->config.ChArray2[i]; 
			if(chIdx < 0) continue;
			
			data[chIdx].ch = chIdx;
			memcpy((char *)&data[chIdx].realData
				,(char *)&myData->save_dcir_msg[fromPs].val[idx].op[bd][ch][0]
				,sizeof(long)*3*10);
		}
	}

	if(send_command2((char *)&header, header_size) < 0) return -1;
	for(i=0; i < chInGroup; i++) {
		if(send_command2((char *)&data[i], sizeof(S_P2_SEND_CMD_REAL_TIME_DATA)) < 0)
			return -3;
	}
	return count;*/
	return 0;
}

//kjg_110712
int send_cmd_step_recipe(group)
{
	int	rtn;

	rtn = send_cmd_test_header(group);
	rtn = send_cmd_test_step(group);

	return rtn;
}

int send_cmd_test_header(int group)
{
	int cmd_id, cmd_size;
	S_P2_SEND_CMD_TEST_HEADER cmd;

	cmd_size = sizeof(S_P2_SEND_CMD_TEST_HEADER);
	memset((char *)&cmd, 0, cmd_size);

	cmd_id = P2_CMD_TO_PC_TEST_HEADER;
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);
	
	//kjgw_110713 cmd.testHeader.totalStep = myData->gData[group].lg_convert_step;
	userlog(DEBUG_LOG, psName, "send cmd test header totalStep[%d]\n",
		cmd.testHeader.totalStep);

	return send_command((char *)&cmd);
}

int send_cmd_test_step(int group)
{/*kjgw_110713
	int i, step_type, stepNo, attribute, rtn;

	stepNo = myData->gData[group].group_convert_step;

	for(i=0; i <= stepNo; i++) {
		attribute =myData->gData[group].testCond.object[i][IDX_OBJ_ATTRIBUTE];
		if(attribute >= ATTR_CHECK1_START
			&& attribute <= ATTR_CHECK1_END) continue;

		step_type = myData->gData[group].testCond.object[i][IDX_OBJ_TYPE];
		if(step_type == STEP_OCV) {
			rtn = send_test_step_ocv(group, i);
		} else if(step_type == STEP_REST) {
			rtn = send_test_step_rest(group, i);
		} else if(step_type == STEP_CHARGE
			|| step_type == STEP_Z_CHARGE) {
			rtn = send_test_step_charge(group, i);
		} else if(step_type == STEP_DISCHARGE
			|| step_type == STEP_Z
			|| step_type == STEP_Z_DISCHARGE) {
			rtn = send_test_step_discharge(group, i);
		} else if(step_type == STEP_END) {
			rtn = send_test_step_end(group, i);
			break;
		}
	}

	return rtn;
*/
	return 0;
}

int send_test_step_ocv(int group, int stepNo)
{
	int cmd_id, cmd_size;
	S_P2_RCV_CMD_TEST_STEP_OCV cmd;

	cmd_size = sizeof(S_P2_RCV_CMD_TEST_STEP_OCV);
	memset((char *)&cmd, 0, cmd_size);

	cmd_id = P2_CMD_TO_PC_TEST_STEP;
	make_header(cmd_id, cmd_size, (char *)&cmd);
/*kjgw_110713
	cmd.stepHeader.stepNo = myData->gData[group].testCond
		.object[stepNo][IDX_OBJ_LG_TO_SUB_CLIENT_STEP_NO] - 1;
	cmd.stepHeader.type = P2_STEP_OCV;
	cmd.stepHeader.mode 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_MODE];

	cmd.ocv.upperV 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_UPPER_V];
	cmd.ocv.lowerV 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_LOWER_V];
	
	userlog(DEBUG_LOG, psName, "send cmd Test Step OCV\n");
*/
	return send_command((char *)&cmd);
}

int send_test_step_rest(int group, int stepNo)
{
	int cmd_id, cmd_size;
	S_P2_RCV_CMD_TEST_STEP_REST	cmd;

	cmd_size = sizeof(S_P2_RCV_CMD_TEST_STEP_REST);
	memset((char *)&cmd, 0, cmd_size);

	cmd_id = P2_CMD_TO_PC_TEST_STEP;
	make_header(cmd_id, cmd_size, (char *)&cmd);
/*kjgw_110713
	cmd.stepHeader.stepNo = myData->gData[group].testCond
		.object[stepNo][IDX_OBJ_LG_TO_SUB_CLIENT_STEP_NO] - 1;
	cmd.stepHeader.type = P2_STEP_REST;
	cmd.stepHeader.mode 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_MODE];

	cmd.rest.endTime 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_END_T];

	cmd.rest.upperV 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_UPPER_V];
	cmd.rest.lowerV 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_LOWER_V];
	
	userlog(DEBUG_LOG, psName, "send cmd Test Step REST\n");
*/
	return send_command((char *)&cmd);
}

int send_test_step_end(int group, int stepNo)
{
	int cmd_id, cmd_size;
	S_P2_RCV_CMD_TEST_STEP_END cmd;

	cmd_size = sizeof(S_P2_RCV_CMD_TEST_STEP_END);
	memset((char *)&cmd, 0, cmd_size);

	cmd_id = P2_CMD_TO_PC_TEST_STEP;
	make_header(cmd_id, cmd_size, (char *)&cmd);
/*kjgw_110713
	cmd.stepHeader.stepNo = myData->gData[group].testCond
		.object[stepNo][IDX_OBJ_LG_TO_SUB_CLIENT_STEP_NO] - 1;
	cmd.stepHeader.type = P2_STEP_END;
	cmd.stepHeader.mode 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_MODE];

	userlog(DEBUG_LOG, psName, "send cmd Test Step END\n");
*/
	return send_command((char *)&cmd);
}

int send_test_step_charge(int group, int stepNo)
{
	int cmd_id, cmd_size;
	S_P2_RCV_CMD_TEST_STEP_CHARGE cmd;

	cmd_size = sizeof(S_P2_RCV_CMD_TEST_STEP_CHARGE);
	memset((char *)&cmd, 0, cmd_size);

	cmd_id = P2_CMD_TO_PC_TEST_STEP;
	make_header(cmd_id, cmd_size, (char *)&cmd);
/*kjgw_110713
	cmd.stepHeader.stepNo = myData->gData[group].testCond
		.object[stepNo][IDX_OBJ_LG_TO_SUB_CLIENT_STEP_NO] - 1;
	cmd.stepHeader.type = P2_STEP_CHARGE;
	cmd.stepHeader.mode 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_MODE];

	cmd.charge.refV 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_REF_V];
	cmd.charge.refI 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_REF_I];
	cmd.charge.endTime 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_END_T];
	cmd.charge.endV 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_END_V];
	cmd.charge.endI 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_END_I];
	cmd.charge.endCapacity 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_END_C];
	cmd.charge.useActualCapacity = 0; 
	cmd.charge.socRate = 0;

	cmd.charge.upperV 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_UPPER_V];
	cmd.charge.lowerV 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_LOWER_V];
	cmd.charge.upperI 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_UPPER_I];
	cmd.charge.lowerI 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_LOWER_I];
	cmd.charge.upperCapacity = 0;
	cmd.charge.lowerCapacity = 0;

	cmd.charge.compTimeV[0] 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_COMP_V_T1];
	cmd.charge.compLowerV[0] = myData->gData[group].testCond
		.object[stepNo][IDX_OBJ_FAULT_COMP_LOWER_V1];
	cmd.charge.compUpperV[0] = 0;

	cmd.charge.compTimeI[0] = 0; 
	cmd.charge.compLowerI[0] = 0;
	cmd.charge.compUpperI[0] = 0;
	
	cmd.charge.deltaTimeV = 0;
	cmd.charge.deltaV = 0;
	cmd.charge.deltaTimeI = 0;
	cmd.charge.deltaI = 0;

	cmd.charge.recodeDeltaTime = 1;
	cmd.charge.recodeDeltaV = 0;
	cmd.charge.recodeDeltaI = 0;
	cmd.charge.recodeDeltaT = myData->gData[group].testCond
		.object[stepNo][IDX_OBJ_RECODE_DELTA_TIME] / 1000;
	cmd.charge.delta_V1 = 0;
	cmd.charge.delta_V2 = 0;
	cmd.charge.reportCode[0] = 0;

	userlog(DEBUG_LOG, psName, "send cmd Test Step Charge\n");
*/
	return send_command((char *)&cmd);
}

int send_test_step_discharge(int group, int stepNo)
{
	int cmd_id, cmd_size;
	S_P2_RCV_CMD_TEST_STEP_DISCHARGE cmd;

	cmd_size = sizeof(S_P2_RCV_CMD_TEST_STEP_DISCHARGE);
	memset((char *)&cmd, 0, cmd_size);

	cmd_id = P2_CMD_TO_PC_TEST_STEP;
	make_header(cmd_id, cmd_size, (char *)&cmd);
/*kjgw_110713
	cmd.stepHeader.stepNo = myData->gData[group].testCond
		.object[stepNo][IDX_OBJ_LG_TO_SUB_CLIENT_STEP_NO] - 1;
	cmd.stepHeader.type = P2_STEP_DISCHARGE;
	cmd.stepHeader.mode 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_MODE];

	cmd.discharge.refV 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_REF_V];
	cmd.discharge.refI 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_REF_I];
	cmd.discharge.endTime 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_END_T];
	cmd.discharge.endV 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_END_V];
	cmd.discharge.endI 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_END_I];
	cmd.discharge.endCapacity 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_END_C];
	cmd.discharge.useActualCapacity = 0; 
	cmd.discharge.socRate = 0;

	cmd.discharge.upperV 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_UPPER_V];
	cmd.discharge.lowerV 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_LOWER_V];
	cmd.discharge.upperI 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_UPPER_I];
	cmd.discharge.lowerI 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_LOWER_I];
	cmd.discharge.upperCapacity = 0;
	cmd.discharge.lowerCapacity = 0;

	cmd.discharge.compTimeV[0] 
		= myData->gData[group].testCond.object[stepNo][IDX_OBJ_FAULT_COMP_V_T1];
	cmd.discharge.compLowerV[0] = myData->gData[group].testCond
		.object[stepNo][IDX_OBJ_FAULT_COMP_LOWER_V1];
	cmd.discharge.compUpperV[0] = 0;

	cmd.discharge.compTimeI[0] = 0; 
	cmd.discharge.compLowerI[0] = 0;
	cmd.discharge.compUpperI[0] = 0;
	
	cmd.discharge.deltaTimeV = 0;
	cmd.discharge.deltaV = 0;
	cmd.discharge.deltaTimeI = 0;
	cmd.discharge.deltaI = 0;

	cmd.discharge.recodeDeltaTime = 1;
	cmd.discharge.recodeDeltaV = 0;
	cmd.discharge.recodeDeltaI = 0;
	cmd.discharge.recodeDeltaT = myData->gData[group].testCond
		.object[stepNo][IDX_OBJ_RECODE_DELTA_TIME] / 1000;
	cmd.discharge.delta_V1 = 0;
	cmd.discharge.delta_V2 = 0;
	cmd.discharge.reportCode[0] = 0;

	userlog(DEBUG_LOG, psName, "send cmd Test Step Discharge\n");
*/
	return send_command((char *)&cmd);
}

//kjg_110713
int send_cmd_get_main_cali_point_reply(char *rcvHeader)
{
	int cmd_id, cmd_size, i, j;
	S_P2_SEND_CMD_GET_MAIN_CALI_POINT_REPLY cmd;
	
	cmd_id = P2_CMD_TO_PC_GET_MAIN_CALI_POINT_REPLY;
	cmd_size = sizeof(S_P2_SEND_CMD_GET_MAIN_CALI_POINT_REPLY);
	memset((char *)&cmd, 0, cmd_size);
	make_header2(cmd_id, cmd_size, (char *)&cmd, rcvHeader);

	cmd.set_point.cali_division = 0; //main_da
	cmd.set_point.v_range_num
		= 1;
//		= myData->mData.config.rangeV;
	cmd.set_point.i_range_num
		= 1;
//		= myData->mData.config.rangeI;

	for(i=0; i < MAX_RANGE; i++) {
		cmd.set_point.v_cali_point[i].cali_point_num
			= (unsigned char)myData->cali_set_data.main_dav.set_point_num[i];
		for(j=0; j < MAX_F_CALI_POINT; j++) {
			cmd.set_point.v_cali_point[i].cali_point[j]
				= myData->cali_set_data.main_dav.set_point[i][j];
		}

		cmd.set_point.v_cali_point[i].cali_check_point_num
			= (unsigned char)myData->cali_set_data.main_dav.check_point_num[i];
		for(j=0; j < MAX_F_CALI_CHECK_POINT; j++) {
			cmd.set_point.v_cali_point[i].cali_check_point[j]
				= myData->cali_set_data.main_dav.check_point[i][j];
		}

		cmd.set_point.i_cali_point[i].cali_point_num
			= (unsigned char)myData->cali_set_data.main_dai.set_point_num[i];
		for(j=0; j < MAX_F_CALI_POINT; j++) {
			cmd.set_point.i_cali_point[i].cali_point[j]
				= myData->cali_set_data.main_dai.set_point[i][j];
		}

		cmd.set_point.i_cali_point[i].cali_check_point_num
			= (unsigned char)myData->cali_set_data.main_dai.check_point_num[i];
		for(j=0; j < MAX_F_CALI_CHECK_POINT; j++) {
			cmd.set_point.i_cali_point[i].cali_check_point[j]
				= myData->cali_set_data.main_dai.check_point[i][j];
		}
	}
	
	return send_command((char *)&cmd);
}

int send_cmd_get_ch_cali_point_reply(char *rcvHeader)
{
	int cmd_id, cmd_size, i, j;
	S_P2_SEND_CMD_GET_CH_CALI_POINT_REPLY cmd;
	
	cmd_id = P2_CMD_TO_PC_GET_CH_CALI_POINT_REPLY;
	cmd_size = sizeof(S_P2_SEND_CMD_GET_CH_CALI_POINT_REPLY);
	memset((char *)&cmd, 0, cmd_size);
	make_header2(cmd_id, cmd_size, (char *)&cmd, rcvHeader);

	cmd.set_point.cali_division = 1; //ch
	cmd.set_point.v_range_num 
		= myData->mData.config.rangeV;
	cmd.set_point.i_range_num
		= myData->mData.config.rangeI;

	for(i=0; i < MAX_RANGE; i++) {
		cmd.set_point.v_cali_point[i].cali_point_num
			= (unsigned char)myData->cali_set_data.ch_v.set_point_num[i];
		for(j=0; j < MAX_F_CALI_POINT; j++) {
			cmd.set_point.v_cali_point[i].cali_point[j]
				= myData->cali_set_data.ch_v.set_point[i][j];
		}

		cmd.set_point.v_cali_point[i].cali_check_point_num
			= (unsigned char)myData->cali_set_data.ch_v.check_point_num[i];
		for(j=0; j < MAX_F_CALI_CHECK_POINT; j++) {
			cmd.set_point.v_cali_point[i].cali_check_point[j]
				= myData->cali_set_data.ch_v.check_point[i][j];
		}

		cmd.set_point.i_cali_point[i].cali_point_num
			= (unsigned char)myData->cali_set_data.ch_i.set_point_num[i];
		for(j=0; j < MAX_F_CALI_POINT; j++) {
			cmd.set_point.i_cali_point[i].cali_point[j]
				= myData->cali_set_data.ch_i.set_point[i][j];
		}

		cmd.set_point.i_cali_point[i].cali_check_point_num
			= (unsigned char)myData->cali_set_data.ch_i.check_point_num[i];
		for(j=0; j < MAX_F_CALI_CHECK_POINT; j++) {
			cmd.set_point.i_cali_point[i].cali_check_point[j]
				= myData->cali_set_data.ch_i.check_point[i][j];
		}
	}
	
	userlog(DEBUG_LOG, psName, "send_cmd_get_ch_cali_point_reply\n");

	return send_command((char *)&cmd);
}

int send_cmd_cali_data(int bd, int ch)
{
	int cmd_id, cmd_size, i, j, type, chIdx, range, chInBdSum, group;
	S_P2_SEND_CMD_CALI_DATA cmd;

	cmd_id = P2_CMD_TO_PC_CALI_DATA;
	cmd_size = sizeof(S_P2_SEND_CMD_CALI_DATA);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	if(myData->mData.signal[M_SIG_CALI_PARALLEL] == P0) {
		cmd.parallel = 0;
	} else {
		cmd.parallel = 1;
	}
	cmd.cali_division = 1; //ch
	type = (int)myData->mData.signal[M_SIG_CALI_VI_SELECT_TMP];
	cmd.cali_type = type;
	cmd.cali_board_no = bd + 1;

	chInBdSum = 0;
	group = myPs->config.groupNo;
	for(i=0; i < MAX_BD_PER_MODULE; i++) {
		if(bdNo_to_groupNo(i) != group+1) continue;
		chInBdSum += myData->mData.config.chInBd[i];
		if(i == bd) break;
	}

	j = chInBdSum;
	j -= myData->mData.config.chInBd[bd];
	i = j + ch;

	chIdx = (int)myPs->config.ChArray2[i]; 
	cmd.monitor_ch = myData->mData.config.chPerBd * bd + ch + 1;
	cmd.hw_ch = ch + 1;
	if(type == 0) {
		range = myData->mData.signal[M_SIG_CALI_RANGE_V] - 1;
	} else {
		range = myData->mData.signal[M_SIG_CALI_RANGE_I] - 1;
	}
	cmd.cali_range = range + 1;

	i = (int)myData->mData.signal[M_SIG_CALI_STEP_TMP];
	cmd.cali_set_point_index = i;
	cmd.cali_ad_point 
		= myData->f_cali.tmp_cData[bd][ch].ad_value_P[type][range][i];
	cmd.cali_dvm_point
		= myData->f_cali.tmp_cData[bd][ch].meter_value_P[type][range][i];

	userlog(DEBUG_LOG, psName, "send_cmd_cali_data ad:%ld, meter:%ld\n",
		cmd.cali_ad_point, cmd.cali_dvm_point);

	return send_command((char *)&cmd);
}

int send_cmd_cali_end(int bd, int type)
{
	int cmd_id, cmd_size, i, range;
	S_P2_SEND_CMD_CALI_END cmd;

	cmd_id = P2_CMD_TO_PC_CALI_END;
	cmd_size = sizeof(S_P2_SEND_CMD_CALI_END);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	if(myData->mData.signal[M_SIG_CALI_PARALLEL] == P0) {
		cmd.parallel = 0;
	} else {
		cmd.parallel = 1;
	}
	cmd.cali_division = 1; //ch
	cmd.cali_type = type;
	cmd.cali_board_no = bd+1;

	if(type ==0) {
		range = myData->mData.signal[M_SIG_CALI_RANGE_V] - 1;
	} else {
		range = myData->mData.signal[M_SIG_CALI_RANGE_I] - 1;
	}
	cmd.cali_range = range+1;

//	if(cmd.parallel == 0) {
		for(i=0; i < (int)myData->mData.config.chInBd[bd]; i++) {
			cmd.cali_flag[i] = myData->cali_ch_flag.flag[i];
		}
//	} else {
//		for(i=0; i < (int)myData->mData.config.chInBd[bd]; i++) {
//			cmd.cali_flag[i] = myData->cali_ch_flag.flag[i];
//		}
//	}

	userlog(DEBUG_LOG, psName, "send_cmd_cali_end bd:%d, type:%d\n", bd, type);

	return send_command((char *)&cmd);
}

int send_cmd_cali_check_data(int bd, int ch)
{
	int cmd_id, cmd_size, i, j, type, range, chIdx, chInBdSum, group;
	S_P2_SEND_CMD_CALI_CHECK_DATA cmd;

	cmd_id = P2_CMD_TO_PC_CALI_CHECK_DATA;
	cmd_size = sizeof(S_P2_SEND_CMD_CALI_CHECK_DATA);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	if(myData->mData.signal[M_SIG_CALI_PARALLEL] == P0) {
		cmd.parallel = 0;
	} else {
		cmd.parallel = 1;
	}
	cmd.cali_division = 1; //ch
	type = (int)myData->mData.signal[M_SIG_CALI_VI_SELECT_TMP];
	cmd.cali_type = type;
	cmd.cali_board_no = bd+1;

	chInBdSum = 0;
	group = myPs->config.groupNo;
	for(i=0; i < MAX_BD_PER_MODULE; i++) {
		if(bdNo_to_groupNo(i) != group+1) continue;
		chInBdSum += myData->mData.config.chInBd[i];
		if(i == bd) break;
	}

	j = chInBdSum;
	j -= myData->mData.config.chInBd[bd];
	i = j + ch;

	chIdx = (int)myPs->config.ChArray2[i]; 
	cmd.monitor_ch = myData->mData.config.chPerBd * bd + ch + 1;
	cmd.hw_ch = ch + 1;

	if(type ==0){
		range = myData->mData.signal[M_SIG_CALI_RANGE_V]-1;
	}else{
		range = myData->mData.signal[M_SIG_CALI_RANGE_I]-1;
	}
	cmd.cali_range = range+1;

	i = (int)myData->mData.signal[M_SIG_CALI_STEP_TMP];
	cmd.cali_set_point_index = i;
	cmd.cali_ad_point = myData->f_cali.tmp_cData[bd][ch].ad_value_N[type][range][i];
	cmd.cali_dvm_point
		= myData->f_cali.tmp_cData[bd][ch].meter_value_N[type][range][i];

	userlog(DEBUG_LOG, psName, "send_cmd_cali_check_data ad:%ld, meter:%ld\n",
		cmd.cali_ad_point, cmd.cali_dvm_point);

	return send_command((char *)&cmd);
}

int send_cmd_cali_check_end(int bd, int type)
{
	int cmd_id, cmd_size, i, range;
	S_P2_SEND_CMD_CALI_CHECK_END cmd;

	cmd_id = P2_CMD_TO_PC_CALI_CHECK_END;
	cmd_size = sizeof(S_P2_SEND_CMD_CALI_CHECK_END);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	if(myData->mData.signal[M_SIG_CALI_PARALLEL] == P0) {
		cmd.parallel = 0;
	} else {
		cmd.parallel = 1;
	}
	cmd.cali_division = 1; //ch
	cmd.cali_type = type;
	cmd.cali_board_no = bd+1;
	if(type ==0){
		range = myData->mData.signal[M_SIG_CALI_RANGE_V]-1;
	}else{
		range = myData->mData.signal[M_SIG_CALI_RANGE_I]-1;
	}
	cmd.cali_range = range+1;

//	if(cmd.parallel == 0) {
		for(i=0; i < (int)myData->mData.config.chInBd[bd]; i++) {
			cmd.cali_flag[i] = myData->cali_ch_flag.flag[i];
		}
//	} else {
//		for(i=0; i < (int)myData->mData.config.chInBd[bd]; i++) {
//			cmd.cali_flag[i] = myData->cali_ch_flag.flag[i];
//		}
//	}

	userlog(DEBUG_LOG, psName, "send_cmd_cali_check_end bd:%d, type:%d\n",
		bd, type);

	return send_command((char *)&cmd);
}

int send_cmd_real_measure_data(int type, int ch)
{
	int cmd_id, cmd_size;//, group;
	S_P2_SEND_CMD_REAL_MEASURE_DATA cmd;

	cmd_id = P2_CMD_TO_PC_REAL_MEASURE_DATA;
	cmd_size = sizeof(S_P2_SEND_CMD_REAL_MEASURE_DATA);
	memset((char *)&cmd, 0, cmd_size);

	make_header(cmd_id, cmd_size, (char *)&cmd);
/*kjgw_110713
	group = (int)myData->mData.signal[M_SIG_REAL_MEASURE_GROUP];
	cmd.cali_type = type;
	cmd.cali_board_no = 1;
	cmd.monitor_ch = ch+1;
	cmd.hw_ch = ch+1 + ((int)myData->mData.config.chPerBd * group); //khkw 20091205
	cmd.measure_range = 0;

	cmd.set_value = myData->bData[group].misc.VICmd[type].value;
	if(type == 0) {
		cmd.ad_value = myData->bData[group].cData[ch].op.Vsens;
		cmd.dvm_value = myData->mData.misc.meter_value[0]; //khkw
	} else {
		cmd.ad_value = myData->bData[group].cData[ch].op.Isens;
		cmd.dvm_value = myData->mData.misc.meter_value[0] * 100;//khkw
	}
*/
	return send_command((char *)&cmd);
}

int send_cmd_real_measure_end(int group, int type)
{
	int cmd_id, cmd_size;
	S_P2_SEND_CMD_REAL_MEASURE_END cmd;

	cmd_id = P2_CMD_TO_PC_REAL_MEASURE_END;
	cmd_size = sizeof(S_P2_SEND_CMD_REAL_MEASURE_END);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	return send_command((char *)&cmd);
}

int send_cmd_hw_map_req_rpy(int group)
{
	int cmd_id, cmd_size, i;
	S_P2_SEND_CMD_HW_MAP_REQ_RPY cmd;

	cmd_id = P2_CMD_TO_PC_HW_MAP_REQ_RPY;
	cmd_size = sizeof(S_P2_SEND_CMD_HW_MAP_REQ_RPY);
	memset((char *)&cmd, 0, cmd_size);
	make_header(cmd_id, cmd_size, (char *)&cmd);

	for(i=0; i < myData->mData.config.chInGroup[group]; i++) {
		cmd.map[i].mon = (unsigned short int)(i+1);
		cmd.map[i].bd //group bd?
			= (unsigned short int)(myPs->config.ChArray3[i].bd+1);
		cmd.map[i].ch 
			= (unsigned short int)(myPs->config.ChArray3[i].ch+1);
		//printf("kjgd %d %d %d\n", i+1, cmd.map[i].bd, cmd.map[i].ch);
	}

	userlog(DEBUG_LOG, psName, "send_cmd_hw_map_req_rpy %d\n", group);

	return send_command((char *)&cmd);
}

void make_header(int cmd_id, int cmd_size, char *cmd)
{
//	int i; //kjgd
	S_P2_CMD_HEADER header;

	memset((char *)&header, 0, sizeof(S_P2_CMD_HEADER));
	
	header.packet_id = PACKET_P2_FORMATION;
	header.group_id = (unsigned short)myPs->config.groupId;
	header.ch = 0;

	switch(cmd_id) { //cmd_id
		case P2_CMD_TO_PC_COMM_CHECK_REPLY:
		case P2_CMD_TO_PC_SET_REPLY:
			header.cmd_id = P2_CMD_TO_PC_RESPONSE;
			break;
		default:
			header.cmd_id = cmd_id;
			break;
	}

	header.cmd_size = cmd_size;

/*	myPs->misc.cmd_serial++;
	header.cmd_serial = myPs->misc.cmd_serial;

	switch(cmd_id) { //cmd_reply
		case P2_CMD_TO_PC_CH_DATA:
			header.cmd_reply = REPLY_YES;
			break;
		case P2_CMD_TO_PC_VERSION_DATA:
		case P2_CMD_TO_PC_VERSION_DATA2:
		case P2_CMD_TO_PC_RESPONSE:
			header.cmd_reply = REPLY_NO;
			break;
	}kjgw*/
	
	memcpy(cmd, (char *)&header, sizeof(S_P2_CMD_HEADER));

/*	userlog(COB_LOG, psName, "header");
	for(i=0; i < sizeof(S_P2_CMD_HEADER); i++) {
		userlog2(COB_LOG, psName, " %x", *(cmd + i));
	}
	userlog2(COB_LOG, psName, ":end\n");
	userlog(COB_LOG, psName, "header %s:end\n", cmd);//kjgd */
}

void make_header2(int cmd_id, int cmd_size, char *cmd, char *rcvHeader)
{
//	int i; //kjgd
	S_P2_CMD_HEADER header, rcvHeader2;

	memset((char *)&header, 0, sizeof(S_P2_CMD_HEADER));
	memcpy((char *)&rcvHeader2, rcvHeader, sizeof(S_P2_CMD_HEADER));
	
	header.packet_id = PACKET_P2_FORMATION;
	header.group_id = (unsigned short)myPs->config.groupId;
	header.ch = 0;

	switch(cmd_id) { //cmd_id
		case P2_CMD_TO_PC_COMM_CHECK_REPLY:
		case P2_CMD_TO_PC_SET_REPLY:
			header.cmd_id = P2_CMD_TO_PC_RESPONSE;
			break;
		default:
			header.cmd_id = cmd_id;
			break;
	}

	header.cmd_size = cmd_size;

/*	switch(cmd_id) { //cmd_serial
		case P2_CMD_TO_PC_VERSION_DATA:
		case P2_CMD_TO_PC_VERSION_DATA2:
		case P2_CMD_TO_PC_RESPONSE:
			header.cmd_serial = rcvHeader2.cmd_serial;
			break;
		default:
			myPs->misc.cmd_serial++;
			header.cmd_serial = myPs->misc.cmd_serial;
			break;
	}

	switch(cmd_id) { //cmd_reply
		case P2_CMD_TO_PC_CH_DATA:
			header.cmd_reply = REPLY_YES;
			break;
		case P2_CMD_TO_PC_VERSION_DATA:
		case P2_CMD_TO_PC_VERSION_DATA2:
		case P2_CMD_TO_PC_RESPONSE:
			header.cmd_reply = REPLY_NO;
			break;
	}kjgw*/
	
	memcpy(cmd, (char *)&header, sizeof(S_P2_CMD_HEADER));

/*	userlog(COB_LOG, psName, "header");
	for(i=0; i < sizeof(S_P2_CMD_HEADER); i++) {
		userlog2(COB_LOG, psName, " %x", *(cmd + i));
	}
	userlog2(COB_LOG, psName, ":end\n");
	userlog(COB_LOG, psName, "header %s:end\n", cmd);//kjgd */
}

int	send_command(char *cmd)
{
	char	packet[MAX_P2_SEND_PACKET_LENGTH];
	unsigned char tmp;
	int i, rtn, tmp2;
	S_P2_CMD_HEADER header;

	memcpy((char *)&header, cmd, sizeof(S_P2_CMD_HEADER));
	
	if(header.cmd_size > MAX_P2_SEND_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName,
			"CMD SEND FAIL!! TOO LARGE SIZE[%d]\n", header.cmd_size);
		return -1;
	}

	memset((char *)&packet[0], 0, MAX_P2_SEND_PACKET_LENGTH);
	memcpy((char *)&packet[0], cmd, header.cmd_size);

/*	if(header.reply == REPLY_YES) {
		memset((char *)&myPs->reply.retry, 0, sizeof(S_P2_RETRY_DATA));

		myPs->reply.retry.cmd_serial = header.cmd_serial;
		myPs->reply.retry.replyCmd = get_reply_cmdid(header.cmd_id);

		strncpy((char *)&myPs->reply.retry.buf[0], cmd, header.cmd_size);
		myPs->reply.retry.cmd_size = header.cmd_size;
		myPs->reply.timer = myData->mData.misc.timer_1sec;
		myPs->reply.timer_run = P1;
	} kjgw*/
	
	rtn = writen(myPs->misc.network_socket, packet, header.cmd_size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d %d\n",
			header.cmd_id, rtn);
		return rtn;
	}

	if(myPs->config.CmdSendLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(COB_LOG, psName, "sendCmd %s:end\n", packet);
		} else {
			if(header.cmd_id == P2_CMD_TO_PC_COMM_CHECK_REPLY) {
			} else if(header.cmd_id == P2_CMD_TO_PC_CH_DATA) {
			} else if(header.cmd_id == P2_CMD_TO_PC_STEP_DATA) {
			} else if(header.cmd_id == P2_CMD_TO_PC_REAL_TIME_DATA) {
			} else if(header.cmd_id == P2_CMD_TO_PC_BCR_INFO) {
			} else if(header.cmd_id == P2_CMD_TO_PC_SENSOR_DATA) {
			} else if(header.cmd_id == P2_CMD_TO_PC_RESPONSE) {
				tmp2 = *(cmd + sizeof(S_P2_CMD_HEADER) + sizeof(int));
				if(tmp2 != P2_CD_ACK) {
					userlog(COB_LOG, psName, "sendCmd %s:end\n", packet);
				}
			} else {
				userlog(COB_LOG, psName, "sendCmd %s:end\n", packet);
			}
		}
	}
	
	if(myPs->config.CmdSendLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
			userlog(COB_LOG, psName, "sendCmd");
			for(i=0; i < header.cmd_size; i++) {
				tmp = *(cmd + i);
				userlog2(COB_LOG, psName, " %02x", tmp);
			}
			userlog2(COB_LOG, psName, ":end\n");
		} else {
			if(header.cmd_id == P2_CMD_TO_PC_COMM_CHECK_REPLY) {
			} else if(header.cmd_id == P2_CMD_TO_PC_CH_DATA) {
			} else if(header.cmd_id == P2_CMD_TO_PC_STEP_DATA) {
			} else if(header.cmd_id == P2_CMD_TO_PC_REAL_TIME_DATA) {
			} else if(header.cmd_id == P2_CMD_TO_PC_BCR_INFO) {
			} else if(header.cmd_id == P2_CMD_TO_PC_SENSOR_DATA) {
			} else if(header.cmd_id == P2_CMD_TO_PC_RESPONSE) {
				tmp2 = *(cmd + sizeof(S_P2_CMD_HEADER) + sizeof(int));
				if(tmp2 != P2_CD_ACK) {
					userlog(COB_LOG, psName, "sendCmd");
					for(i=0; i < header.cmd_size; i++) {
						tmp = *(cmd + i);
						userlog2(COB_LOG, psName, " %02x", tmp);
					}
					userlog2(COB_LOG, psName, ":end\n");
				}
			} else {
				userlog(COB_LOG, psName, "sendCmd");
				for(i=0; i < header.cmd_size; i++) {
					tmp = *(cmd + i);
					userlog2(COB_LOG, psName, " %02x", tmp);
				}
				userlog2(COB_LOG, psName, ":end\n");
			}
		}
	}
	return 0;
}

int	send_command2(char *cmd, int size)
{
	char	packet[MAX_P2_SEND_PACKET_LENGTH];
	unsigned char tmp;
	int i, rtn;
	S_P2_CMD_HEADER header;

	memcpy((char *)&header, cmd, sizeof(S_P2_CMD_HEADER));

	if(size > MAX_P2_SEND_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName,
			"CMD SEND FAIL!! TOO LARGE SIZE2[%d]\n", size);
		return -1;
	}

	memset((char *)&packet[0], 0, MAX_P2_SEND_PACKET_LENGTH);
	memcpy((char *)&packet[0], cmd, size);

/*	if(header.reply == REPLY_YES) {
		memset((char *)&myPs->reply.retry, 0, sizeof(S_P2_RETRY_DATA));

		myPs->reply.retry.cmd_serial = header.cmd_serial;
		myPs->reply.retry.replyCmd = get_reply_cmdid(header.cmd_id);

		strncpy((char *)&myPs->reply.retry.buf[0], cmd, header.cmd_size);
		myPs->reply.retry.cmd_size = header.cmd_size;
		myPs->reply.timer = myData->mData.misc.timer_1sec;
		myPs->reply.timer_run = P1;
	} kjgw*/
	
	rtn = writen(myPs->misc.network_socket, packet, size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error2!!! %d %d\n",
			header.cmd_id, rtn);
		return rtn;
	}

	if(myPs->config.CmdSendLog == P1) {
		if(myPs->config.CommCheckLog == P1) {
//			userlog(COB_LOG, psName, "sendCmd2 %s:end\n", packet);
		}
	}
	
	if(myPs->config.CmdSendLog_Hex == P1) {
		if(myPs->config.CommCheckLog == P1) {
//			userlog(COB_LOG, psName, "sendCmd2");
			for(i=0; i < size; i++) {
				tmp = *(cmd + i);
//				userlog2(COB_LOG, psName, " %02x", tmp);
			}
//			userlog2(COB_LOG, psName, ":end\n");
		}
	}

	return 0;
}

int	get_reply_cmdid(char *cmd_id)
{
	char buf[10];
	int cmdId, rtn;
	
	memset(buf, 0, sizeof buf);
	strncpy(buf, cmd_id, 4);
	cmdId = atoi(buf);
	switch(cmdId) {
		case P2_CMD_TO_SBC_COMM_CHECK:	
			rtn = P2_CMD_TO_PC_RESPONSE;	
			break;
		default:	
			rtn = 0;	
			break;
	}

	return rtn;
}

int Check_NetworkState(void)
{
	check_cmd_reply_timeout();
	network_ping();
	if(check_network_timeout() < 0) {
		close(myPs->misc.network_socket);
		userlog(DEBUG_LOG, psName, "network communication error3\n");
		myPs->signal[P2_SIG_NET_CONNECTED] = P0;
		return -1;
	}
	return 0;
}

void network_ping(void)
{
	long diff, time1, time2;

	if(myPs->signal[P2_SIG_NET_CONNECTED] != P1) return;

	if(myPs->reply.timer_run == P0) {
		time1 = myData->mData.misc.timer_1sec;
		time2 = myData->mData.misc.timer_1000ms;
		diff = (time1 - myPs->misc.ping_time) * 1000;
		diff += (time2 - myPs->misc.ping_time2);
		if(diff >= myPs->config.pingTimeout || diff < 0) {
			//send_cmd_comstate_rpt_req(boxid); //925
			
			myPs->misc.ping_time = myData->mData.misc.timer_1sec;
			myPs->misc.ping_time2 = myData->mData.misc.timer_1000ms;
		}
	}
}

void check_cmd_reply_timeout(void)
{
	int rtn;
	long diff, time1, time2;
	
	if(myPs->signal[P2_SIG_NET_CONNECTED] != P1) return;
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
			memset((char *)&myPs->reply.retry, 0, sizeof(S_P2_RETRY_DATA));
			//kjgw : next working
		} else {
			myPs->reply.timer_run = P1;
			myPs->reply.time = time1;
			myPs->reply.time2 = time2;
			myPs->reply.retry.count++;
				
			if(myPs->config.CmdSendLog == P1) {
				userlog(DEBUG_LOG, psName, "retry %s\n", myPs->reply.retry.buf);
			} //kjgw hex

			rtn = writen(myPs->misc.send_socket,
				(char *)&myPs->reply.retry.buf, myPs->reply.retry.size);
		}
	}
}

int	check_network_timeout(void)
{
	long diff, time1, time2;

	if(myPs->signal[P2_SIG_NET_CONNECTED] != P1) return 0;

	if(myData->AppControl.signal[APP_SIG_MODULE_CONTROL_PROCESS] == P1) {
		time1 = myData->mData.misc.timer_1sec;
		time2 = myData->mData.misc.timer_1000ms;
		diff = (time1 - myPs->misc.net_time) * 1000;
		diff += (time2 - myPs->misc.net_time2);
		if(diff >= myPs->config.netTimeout || diff < 0) return -1;
	}
	return 0;
}

