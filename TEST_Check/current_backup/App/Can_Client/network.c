#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
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
#include "CAN.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_CAN_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];


int InitNetwork(void)
{
	sleep(2);

	if(myPs->config.network_socket > 0)
		close(myPs->config.network_socket);
	
	myPs->config.network_socket
		= SetClientSock(myPs->config.networkPort,
						(char *)&myPs->config.ipAddr);
    if(myPs->config.network_socket < 0) {
		close(myPs->config.network_socket);
	    userlog(DEBUG_LOG, psName,
			"Can not initialize network(network Port) : %d\n",
			myPs->config.network_socket);
		return -2;
	}

	myPs->signal[CAN_SIG_NET_CONNECTED] = P1;
	myPs->pingTimer = myData->mData.misc.timer_1sec;
	myPs->netTimer = myData->mData.misc.timer_1sec;

	memset((char *)&myPs->rcvCmd, 0, sizeof(S_P1_RCV_COMMAND));
	memset((char *)&myPs->rcvPacket, 0, sizeof(S_P1_RCV_PACKET));

	userlog(DEBUG_LOG, psName,
		"command socket connected network_socket %d)\n",
		myPs->config.network_socket);
	
	return 0;
}

int NetworkPacket_Receive(void)
{
	int rcv_size, read_size, i, start, index;
	char maxPacketBuf[MAX_P1_RECV_PACKET_LENGTH];

	memset(maxPacketBuf, 0, MAX_P1_RECV_PACKET_LENGTH);

	if(ioctl(myPs->config.network_socket, FIONREAD, &rcv_size) < 0) {
		userlog(DEBUG_LOG, psName, "packet receive ioctl error\n");
		return -1;
	}

	if(rcv_size > MAX_P1_RECV_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName,
			"max packet size over\n");
		read_size = readn(myPs->config.network_socket, maxPacketBuf,
//		read_size = readn(myPs->config.fd_socket, maxPacketBuf,
			MAX_P1_RECV_PACKET_LENGTH);
		if(read_size != MAX_P1_RECV_PACKET_LENGTH)
			userlog(DEBUG_LOG, psName,
				"packet readn size error1\n");
		return -2;
	} else if(rcv_size > (MAX_P1_RECV_PACKET_LENGTH - myPs->rcvPacket.usedBufSize)) {
		userlog(DEBUG_LOG, psName,
			"packet buffer overflow\n");
		read_size = readn(myPs->config.network_socket, maxPacketBuf, rcv_size);
//		read_size = readn(myPs->config.fd_socket, maxPacketBuf, rcv_size);
		if(read_size != rcv_size)
			userlog(DEBUG_LOG, psName,
				"packet readn size error2\n");
		return -3;
	} else if(rcv_size <= 0) {
		userlog(DEBUG_LOG, psName,
			"packet sock_rcv error %d\n", rcv_size);
		return -4;
	} else {
		read_size = readn(myPs->config.network_socket, maxPacketBuf, rcv_size);
//		read_size = readn(myPs->config.fd_socket, maxPacketBuf, rcv_size);
		if(read_size != rcv_size) {
			userlog(DEBUG_LOG, psName,
				"packet readn size error3 : %d, %d\n", read_size, rcv_size);
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
		myPs->rcvPacket.rcvStartPoint[i] = abs(start - MAX_P1_RECV_PACKET_LENGTH);
	} else {
		myPs->rcvPacket.rcvStartPoint[i] = start;
	}

	myPs->rcvPacket.rcvSize[i] = read_size;
	myPs->rcvPacket.usedBufSize += read_size;
	
	start = myPs->rcvPacket.rcvStartPoint[i];
	if((start + read_size) > MAX_P1_RECV_PACKET_LENGTH) {
		index = MAX_P1_RECV_PACKET_LENGTH - start;
		memcpy((char *)&myPs->rcvPacket.rcvPacketBuf[start],
			(char *)&maxPacketBuf[0], index);
		memcpy((char *)&myPs->rcvPacket.rcvPacketBuf[0],
			(char *)&maxPacketBuf[index], read_size - index);
	} else {
		memcpy((char *)&myPs->rcvPacket.rcvPacketBuf[start],
			(char *)&maxPacketBuf[0], read_size);
	}
	return 0;
}

void NetworkPacket_Parsing(void)
{
	int i, j, k, cmdBuf_index, start_point;
	
	if(myPs->rcvPacket.rcvCount == myPs->rcvPacket.parseCount) return;
	
	i = myPs->rcvPacket.parseCount;
	myPs->rcvPacket.parseCount++;
	if(myPs->rcvPacket.parseCount > (MAX_P1_RECV_PACKET_COUNT-1))
		myPs->rcvPacket.parseCount = 0;
	
	cmdBuf_index = myPs->rcvCmd.cmdBufSize;
	myPs->rcvCmd.cmdBufSize += myPs->rcvPacket.rcvSize[i];
	
	start_point = myPs->rcvPacket.parseStartPoint[i];

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
}

int NetworkCommand_Receive(void)
{
	int cmd_size, cmdBuf_index;
	char tmpBuf[MAX_P1_RECV_PACKET_LENGTH];

	S_CAN_CLIENT_CMD_HEADER header;

	if(myPs->rcvCmd.cmdBufSize < sizeof(header)) return -1;
	
	// buf is full (rest packet 0 or a little)i
	// if rest packet is 0 : rcvCmd.cmdBufSize = 0;
	// else rcvCmd.cmdBufSize = rest Size;
	// copy rest packet to &rcvCmd.cmdBuf[0]  
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmdBuf, sizeof(S_CAN_CLIENT_CMD_HEADER));
	cmd_size = header.body_size + sizeof(S_CAN_CLIENT_CMD_HEADER);
	if(myPs->rcvCmd.cmdBufSize < cmd_size) return -2;
	
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
	memcpy((char *)&myPs->rcvCmd.cmdBuf[0],	(char *)&tmpBuf[0], cmd_size);
	
	return 0;
}

int NetworkCommand_Parsing(void)
{
	char buf[12];
	int	rtn;

	S_CAN_CLIENT_CMD_HEADER header;

	memset((char *)&header, 0, sizeof(header));
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd,
		sizeof(header));
	
	memset(buf, 0, sizeof(buf));
	memcpy((char *)&buf[0], (char *)&header.cmd_id, sizeof(header.cmd_id));
	rtn = CmdHeaderCheck((char *)&header);
	if(rtn < 0)	{
		userlog(DEBUG_LOG, psName, 
			"Can Client CmdHeaderCheck error %d\n", rtn);
		return -1;
	}
	
	switch(header.cmd_id) {
		case CAN_TO_SBC_CMD_RECEIVE_DATA:
			rtn = rcv_cmd_receive_data();
			userlog(DEBUG_LOG, psName, 
				"CAN_TO_SBC_CMD_DAQ_DATA size[%d]\n",header.body_size);
			break;
		case CAN_TO_SBC_CMD_HEARTBEAT_REQ:
			rtn = rcv_cmd_heartbeat_req();
			break;
		case CAN_TO_SBC_CMD_RESPONSE:
			rtn = rcv_cmd_response();
			//userlog(DEBUG_LOG, psName, 
			//	"CAN_TO_SBC_CMD_RESPONSE Cmd[%d]\n",header.cmd_id);
			break;
		case CAN_TO_SBC_CMD_STANDBY_REQ:
			rtn = rcv_cmd_standby_req();
			break;
		case CAN_TO_SBC_CMD_RECEIVE_SET_REQ:
			rtn = rcv_cmd_can_receive_set_req();
			break;
		case CAN_TO_SBC_CMD_TRANSMIT_SET_REQ:
			rtn = rcv_cmd_can_transmit_set_req();
			break;
		default:
			userlog(DEBUG_LOG, psName, 
				"Can't Find Cmd[%d]\n",header.cmd_id);
			rtn = -1;
			break;
	}
	if(rtn < 0) return -3;
	else return 0;
}

int Parsing_NetworkEvent(void)
{
	int rtn=0;

	NetworkPacket_Parsing();
	if(NetworkCommand_Receive() >= 0) {
		if(NetworkCommand_Parsing() < 0) {
			myPs->rcvCmd.cmdFail++;
			if(myPs->rcvCmd.cmdFail >= 3) {
				myPs->rcvCmd.cmdFail = 0;
				myPs->rcvCmd.cmdBufSize = 0;
				memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00,
					MAX_P1_RECV_PACKET_LENGTH);
				rtn = -1;
			}
		} else {
			myPs->rcvCmd.cmdFail = 0;
		}
	}
	
	return rtn;
}


int CmdHeaderCheck(char *rcvHeader)
{
	//int cmd_id, 
	int len=0, rtn;
	char buf[12];
	S_CAN_CLIENT_CMD_HEADER header;
	
	memset((char *)&header, 0, sizeof(header));
	memcpy((char *)&header, (char *)rcvHeader, sizeof(header));
	memset(buf, 0, sizeof(buf));
	memcpy((char *)&buf[0], (char *)&header.cmd_id, sizeof(header.cmd_id));
	
	switch(header.cmd_id) {
		case CAN_TO_SBC_CMD_RECEIVE_DATA:
			len = sizeof(S_CAN_CLIENT_RCV_RECEIVE_DATA);
			break;
		case CAN_TO_SBC_CMD_HEARTBEAT_REQ:
			len = sizeof(S_CAN_CLIENT_RCV_CMD_HEARTBEAT_REQ);
			break;
		case CAN_TO_SBC_CMD_RESPONSE:
			len = sizeof(S_CAN_CLIENT_RCV_CMD_RESPONSE);
			break;
		case CAN_TO_SBC_CMD_STANDBY_REQ:
			len = sizeof(S_CAN_CLIENT_RCV_CMD_START_REQ);
			break;
		case CAN_TO_SBC_CMD_RECEIVE_SET_REQ:
			len = sizeof(S_CAN_CLIENT_RCV_CMD_RECEIVE_SET_REQ);
			break;
		case CAN_TO_SBC_CMD_TRANSMIT_SET_REQ:
			len = sizeof(S_CAN_CLIENT_RCV_CMD_TRANSMIT_SET_REQ);
			break;
		default:
			userlog(DEBUG_LOG, psName, "Can_Client RcvCmd command id error : %ld : %ld\n", 
				header.cmd_id, header.body_size);
			rtn = -1;
			break;
	}

	if(len != myPs->rcvCmd.cmdSize) {
		userlog(DEBUG_LOG, psName, 
			"CanClient RcvCmd Data Size error cmd_id %ld (%d : %d), %d\n", header.cmd_id,
		len, myPs->rcvCmd.cmdSize, header.body_size);
		return -8;
	}
	return 0;
}

void make_header(char *buf, int cmd_id, int ch, int body_size)
{
	char temp[12];
	unsigned long ch_flag;
	
	S_CAN_CLIENT_CMD_HEADER header;
	
	memset((char *)&header, 0x00, sizeof(header));

	memset(temp, 0x00, sizeof(temp));
	ch_flag = 0x01;
	ch_flag = ch_flag << ch;
	header.ch_flag = ch_flag;
	header.cmd_id = cmd_id;
	header.msg_count = myPs->misc.msg_count;
	header.body_size = body_size;
//	userlog(DEBUG_LOG, psName, "make header %d %d\n", cmd_id, ch_flag);
	switch(header.cmd_id) {
		case SBC_TO_CAN_CMD_PACK_DATA:
			header.ch_flag = ch;
			break;
		default:
			break;
	}

	memcpy(buf, (char *)&header, sizeof(S_CAN_CLIENT_CMD_HEADER));
}

int check_network_ping(void)
{
	int rtn = 0;

	if(myPs->signal[CAN_SIG_NET_CONNECTED] != P1) return 0;

	if(myPs->pingCount >= myPs->config.retryCount) { 
		myPs->pingCount = 0;
		rtn = -1;
	}
	return rtn;
}

int network_ping(void)
{
	int diff, rtn = 0;

	if(myPs->signal[CAN_SIG_NET_CONNECTED] != P1) return 0;

	diff = (int)labs(myData->mData.misc.timer_1sec - myPs->pingTimer);
	if(diff > myPs->config.heartbeatTimeout) { //Sec
		myPs->pingTimer = myData->mData.misc.timer_1sec;
		myPs->pingCount++;
	}

	return rtn;
}

int Check_NetworkState(void)
{
	int rtn=0;

	rtn = check_network_ping();
	if( rtn < 0){
		userlog(DEBUG_LOG, psName, "network communication error %d\n", rtn);
		return rtn;
	}
	rtn = network_ping();
	return rtn;
}

int	send_command(char *cmd, int cmd_size)
{
	char	packet[MAX_P1_RECV_PACKET_LENGTH];
	int 	rtn;
	S_CAN_CLIENT_CMD_HEADER header;

	memset((char *)&header, 0x00, sizeof(S_CAN_CLIENT_CMD_HEADER));
	memcpy((char *)&header, cmd, sizeof(S_CAN_CLIENT_CMD_HEADER));

	
	if(cmd_size > MAX_P1_RECV_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName,
			"CMD SEND FAIL!! TOO LARGE SIZE[%d]\n", cmd_size);
		return -1;
	}
	
	memset((char *)&packet, 0x00, sizeof(packet));
	memcpy((char*)packet, cmd, cmd_size);

	rtn = writen(myPs->config.network_socket, packet, cmd_size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd[%d] send error!!! %d\n", header.cmd_id, rtn);
		return rtn;
	}
	return 0;
}

void send_cmd_ch_data(void)
{
	//int i=0;
	long diff, time1, time2, interval;
	
	interval = 200;	
	//interval = 150;	
	time1 = myData->mData.misc.timer_1sec;
	time2 = myData->mData.misc.timer_1000ms;
	if(myPs->signal[CAN_SIG_DATA_START] != P1) {
		myPs->misc.sent_monitor_data_time = time1;
		myPs->misc.sent_monitor_data_time2 = time2;
		return;
	}
	diff = (time1 - myPs->misc.sent_monitor_data_time) * 1000;
	diff += (time2 - myPs->misc.sent_monitor_data_time2);
	if(diff >= interval || diff < 0) {
		send_cmd_pack_cycler_data();
	//	userlog(DEBUG_LOG, psName, "send_data %ld\n", diff);
		myPs->misc.sent_monitor_data_time = time1;
		myPs->misc.sent_monitor_data_time2 = time2;
		//myPs->signal[CAN_SIG_DATA_START] = P0;
	}
	myPs->pingCount = 0;
	myPs->pingTimer = myData->mData.misc.timer_1sec;
}

int send_cmd_pack_cycler_data(void)
{
	unsigned char ch= 0;
	short int sens_ch;
	int	cmd_size, body_size, rtn, j;
	//shhw_231004s	
	short int func_div;
	float default_value = 0.0;
	//shhw_231004e
	long ratioV, ratioI;	//shh_231208	
	S_CAN_CLIENT_SEND_PACK_DATA	cmd;
	
	cmd_size = sizeof(S_CAN_CLIENT_SEND_PACK_DATA);
	memset((char *)&cmd, 0x00, cmd_size);
	body_size = cmd_size - sizeof(S_CAN_CLIENT_CMD_HEADER);
	make_header((char *)&cmd, SBC_TO_CAN_CMD_PACK_DATA, ch, body_size);

	ratioV = (long)myData->mData.patt_ratioV; //shh_231208
	ratioI = (long)myData->mData.patt_ratioI; //shh_231208

	for(ch=0; ch<4; ch++) {

		//shhw_231208s
		 if(myData->mData.config.ratioV == MICRO_UNIT) {         //uV,uA
			cmd.pack_data[ch].voltage = myData->cData[ch].op.Vsens;
			cmd.pack_data[ch].Isens = myData->cData[ch].op.Isens;
		} else if(myData->mData.config.ratioV == MILLI_UNIT) {  //mV,mA
			cmd.pack_data[ch].voltage = myData->cData[ch].op.Vsens * ratioV;
			cmd.pack_data[ch].Isens = myData->cData[ch].op.Isens * ratioI;
		 } else {                                                //V,A
			cmd.pack_data[ch].voltage = myData->cData[ch].op.Vsens * ratioV;
			cmd.pack_data[ch].Isens = myData->cData[ch].op.Isens * ratioI;
		}
		//cmd.pack_data[ch].voltage = myData->cData[ch].op.Vsens;
		//cmd.pack_data[ch].Isens = myData->cData[ch].op.Isens;
		//shhw_231208e
		
		if(myData->cData[ch].signal[C_SIG_TX_CELL_BALANCING] == 1) {
			cmd.pack_data[ch].cell_balancing = 1;
		} else {
			cmd.pack_data[ch].cell_balancing = 0;
		}
		
		sens_ch = 0;
		for(j=0; j < MAX_TC_DATA; j++) {
			if(myPs->misc.tc_ch[ch][j] == 0) continue;
			sens_ch = myPs->misc.tc_ch[ch][j] - 1;
			cmd.pack_data[ch].tc[j] = myData->COM.com_port[AUX_TEMP].value[sens_ch];
		}
		sens_ch = 0;
		for(j=0; j < MAX_NTC_DATA; j++) {
			if(myPs->misc.ntc_ch[ch][j] == 0) continue;
			sens_ch = myPs->misc.ntc_ch[ch][j] - 1;
			sens_ch += myData->mData.config.installedAuxV;
			sens_ch = myData->aux_ch_num[sens_ch].daq_ch;
			cmd.pack_data[ch].ntc[j] = myData->SubSensV.ch[sens_ch].sensV;
		}
		sens_ch = 0;
		for(j=0; j < MAX_AUX_V_DATA; j++) {
			if(myPs->misc.auxV_ch[ch][j] == 0) continue;
			sens_ch = myPs->misc.auxV_ch[ch][j] - 1;
			sens_ch = myData->aux_ch_num[sens_ch].daq_ch;
			cmd.pack_data[ch].auxV[j] = myData->SubSensV.ch[sens_ch].sensV;
		}
		//shhw_231031s
		//CAN_TX_FUNC_DIV_VALUE_CHANGE ~ CAN_TX_FUNC_DIV_VALUE_CHANGE5
		for(j = 0; j < 5; j++) {
			if(myData->cData[ch].op.state == C_RUN) {
				func_div = CAN_TX_FUNC_DIV_VALUE_CHANGE + j;
				cmd.pack_data[ch].tx_func_data[j] 
				= (long)(FindCanChangeData_LTC(ch,default_value,func_div) * 1000);
				
				cmd.pack_data[ch].tx_func_div[j] = func_div;
			} else {
				cmd.pack_data[ch].tx_func_data[j] = 0; 
				cmd.pack_data[ch].tx_func_div[j] = 0;
			}
		}
		//shhw_231031e
	}
	rtn = send_command((char *)&cmd, cmd_size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "CMD[%d] SEND FAIL!(pack_data)\n", cmd.header.cmd_id);
		return -1;
	} else {
	}
	return 0;
}

int send_cmd_can_receive_set(int ch)
{
	int	cmd_size, body_size, rtn, type, data;
	S_CAN_CLIENT_SEND_RECEIVE_SET_DATA	cmd;
	
	cmd_size = sizeof(S_CAN_CLIENT_SEND_RECEIVE_SET_DATA);
	memset((char *)&cmd, 0x00, cmd_size);
	body_size = cmd_size - sizeof(S_CAN_CLIENT_CMD_HEADER);
	make_header((char *)&cmd, SBC_TO_CAN_CMD_RECEIVE_SET, ch, body_size);

	//common data
	for(type=0; type < MAX_CAN_TYPE; type++) {
		cmd.commonData[type].baudrate 
			= myData->canReceiveSetData.commonData[ch][type].can_baudrate;
		cmd.commonData[type].extended_id
			= myData->canReceiveSetData.commonData[ch][type].extended_id;
		cmd.commonData[type].bms_type
			= myData->canReceiveSetData.commonData[ch][type].bms_type;
		cmd.commonData[type].sjw
			= myData->canReceiveSetData.commonData[ch][type].sjw;
		cmd.commonData[type].fd_flag
			= myData->canReceiveSetData.commonData[ch][type].can_fd_flag;
		cmd.commonData[type].can_datarate
			= myData->canReceiveSetData.commonData[ch][type].can_datarate;
		cmd.commonData[type].terminal_r
			= myData->canReceiveSetData.commonData[ch][type].terminal_r;
		cmd.commonData[type].crc_type
			= myData->canReceiveSetData.commonData[ch][type].crc_type;
	}
	//nomal data
	for(data=0; data < MAX_RX_LTC_DATA; data++) {
		cmd.normalData[data].canID
			= myData->canReceiveSetData.normalData[ch][data].canID;
	}
	
	rtn = send_command((char *)&cmd, cmd_size);
	userlog(DEBUG_LOG, psName, "send_cmd_can_receive_set %d %d\n", rtn, cmd.header.ch_flag);
	userlog(DEBUG_LOG, psName, "send id %ld size %ld\n", cmd.normalData[0].canID, cmd.header.body_size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "CMD[%d] SEND FAIL!(receive_set)\n", cmd.header.cmd_id);
		return -1;
	}
	return 0;
}

int send_cmd_can_transmit_set(int ch)
{
	unsigned char tc, ntc, cell;
	short int offset, msg_count, func_div, func_div2, func_div3;
	int	cmd_size, body_size, rtn, type, data;
	float default_value = 0.0;
	
	S_CAN_CLIENT_SEND_TRANSMIT_SET_DATA	cmd;
	
	cmd_size = sizeof(S_CAN_CLIENT_SEND_TRANSMIT_SET_DATA);
	memset((char *)&cmd, 0x00, cmd_size);
	body_size = cmd_size - sizeof(S_CAN_CLIENT_CMD_HEADER);
	make_header((char *)&cmd, SBC_TO_CAN_CMD_TRANSMIT_SET, ch, body_size);
	
	memset((char *)&myPs->misc.tc_ch[ch], 0, sizeof(short int) * MAX_TC_DATA);
	memset((char *)&myPs->misc.ntc_ch[ch], 0, sizeof(short int) * MAX_NTC_DATA);
	memset((char *)&myPs->misc.auxV_ch[ch], 0, sizeof(short int) * MAX_AUX_V_DATA);
	
	tc = ntc = cell = 0;
	func_div = func_div2 = func_div3 = 0;
	msg_count = 0;
	if(myPs->misc.msg_count != 0) {
		msg_count = myPs->misc.msg_count - 1;
	}
	offset = msg_count * MAX_TX_LTC_DATA;
	//common data
	for(type=0; type < MAX_CAN_TYPE; type++) {
		cmd.commonData[type].baudrate 
			= myData->canTransmitSetData.commonData[ch][type].can_baudrate;
		cmd.commonData[type].extended_id
			= myData->canTransmitSetData.commonData[ch][type].extended_id;
		cmd.commonData[type].bms_type
			= myData->canTransmitSetData.commonData[ch][type].bms_type;
		cmd.commonData[type].sjw
			= myData->canTransmitSetData.commonData[ch][type].sjw;
		cmd.commonData[type].fd_flag
			= myData->canTransmitSetData.commonData[ch][type].can_fd_flag;
		cmd.commonData[type].can_datarate
			= myData->canTransmitSetData.commonData[ch][type].can_datarate;
		//cmd.commonData[type].terminal_r
		//	= myData->canTransmitSetData.commonData[ch][type].terminal_r;
		cmd.commonData[type].terminal_r = 0;
		cmd.commonData[type].crc_type
			= myData->canTransmitSetData.commonData[ch][type].crc_type;
	}
	//nomal data
	for(data=0; data < MAX_TX_LTC_DATA; data++) {
		cmd.normalData[data].canType 
			= myData->canTransmitSetData.normalData[ch][data + offset].canType;
		cmd.normalData[data].byte_order
			= myData->canTransmitSetData.normalData[ch][data + offset].byte_order;
		cmd.normalData[data].data_type
			= myData->canTransmitSetData.normalData[ch][data + offset].data_type;
		cmd.normalData[data].dlc
			= myData->canTransmitSetData.normalData[ch][data + offset].dlc;
		cmd.normalData[data].factor
			= myData->canTransmitSetData.normalData[ch][data + offset].factor;
		cmd.normalData[data].startBit
			= myData->canTransmitSetData.normalData[ch][data + offset].startBit;
		cmd.normalData[data].bitCount
			= myData->canTransmitSetData.normalData[ch][data + offset].bitCount;
		cmd.normalData[data].canID
			= myData->canTransmitSetData.normalData[ch][data + offset].canID;
		cmd.normalData[data].offset
			= myData->canTransmitSetData.normalData[ch][data + offset].offset;
		cmd.normalData[data].default_value
			= myData->canTransmitSetData.normalData[ch][data + offset].default_value;
		cmd.normalData[data].send_period
			= myData->canTransmitSetData.normalData[ch][data + offset].send_period;
		cmd.normalData[data].func_div
			= myData->canTransmitSetData.normalData[ch][data + offset].function_div;
		cmd.normalData[data].func_div2
			= myData->canTransmitSetData.normalData[ch][data + offset].function_div2;
		cmd.normalData[data].func_div3
			= myData->canTransmitSetData.normalData[ch][data + offset].function_div3;
	}
	if(myPs->misc.msg_count >= 8) {
		for(data=0; data < (MAX_TX_LTC_DATA * myPs->misc.msg_count); data++) {
			func_div = myData->canTransmitSetData.normalData[ch][data].function_div;
			func_div2 = myData->canTransmitSetData.normalData[ch][data].function_div2;
			func_div3 = myData->canTransmitSetData.normalData[ch][data].function_div3;
			default_value = myData->canTransmitSetData.normalData[ch][data].default_value;
			if(func_div == CAN_TX_FUNC_DIV_TC_DATA
				|| func_div2 == CAN_TX_FUNC_DIV_TC_DATA
				|| func_div3 == CAN_TX_FUNC_DIV_TC_DATA) {
				myPs->misc.tc_ch[ch][tc] = default_value;
				tc++;
			} else if(func_div == CAN_TX_FUNC_DIV_NTC_DATA
				|| func_div2 == CAN_TX_FUNC_DIV_NTC_DATA
				|| func_div3 == CAN_TX_FUNC_DIV_NTC_DATA) {
				myPs->misc.ntc_ch[ch][ntc] = default_value;
				ntc++;
			} else if(func_div == CAN_TX_FUNC_DIV_CELL_DATA
				|| func_div2 == CAN_TX_FUNC_DIV_CELL_DATA
				|| func_div3 == CAN_TX_FUNC_DIV_CELL_DATA) {
				myPs->misc.auxV_ch[ch][cell] = default_value;
				cell++;
			}
		}
		myPs->misc.msg_count = 0;
	}
	rtn = send_command((char *)&cmd, cmd_size);
	userlog(DEBUG_LOG, psName, "send_cmd_can_transmit_set %d, %d\n", ch, myPs->misc.msg_count);
	userlog(DEBUG_LOG, psName, "send id %ld size %ld\n", cmd.normalData[0].canID, cmd.header.body_size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "CMD[%d] SEND FAIL!(transmit_set)\n", cmd.header.cmd_id);
		return -1;
	}
	return 0;
}

int	send_cmd_response(int cmd_id, int reply)
{ 
	int	cmd_size, body_size, rtn, ch;
	S_CAN_CLIENT_SEND_CMD_RESPONSE	cmd;
	
	ch = 0;
	cmd_size = sizeof(S_CAN_CLIENT_SEND_CMD_RESPONSE);
	body_size = cmd_size - sizeof(S_CAN_CLIENT_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);

	make_header((char *)&cmd, SBC_TO_CAN_CMD_RESPONSE, ch, body_size);
	cmd.response.cmd_id = cmd_id;
	cmd.response.state = reply;
	userlog(DEBUG_LOG, psName, "response %d\n", cmd.header.cmd_id);
	//userlog(DEBUG_LOG, psName, "response_cmd_id[%d] , response code = %d\n", cmd.response.cmd_id,
	//	cmd.response.code);
	rtn = send_command((char *)&cmd, cmd_size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "CMD[%d] SEND FAIL!(response)\n", cmd.header.cmd_id);
		return -1;
	}
	return 0;
}

int send_cmd_heartbeat_reply(void)
{
	int	cmd_size, body_size, rtn;
	S_CAN_CLIENT_RCV_CMD_HEARTBEAT_RPY cmd;

	cmd_size = sizeof(S_CAN_CLIENT_RCV_CMD_HEARTBEAT_RPY);
	body_size = cmd_size - sizeof(S_CAN_CLIENT_CMD_HEADER);
	memset((char *)&cmd, 0, cmd_size);
	make_header((char *)&cmd, SBC_TO_CAN_CMD_HEARTBEAT_RPY, 0, body_size);
	
	rtn = send_command((char *)&cmd, cmd_size);
	userlog(DEBUG_LOG, psName, "send_cmd_heartbeet_reply %d\n", rtn);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "CMD[%d] SEND FAIL!\n", cmd.header.cmd_id);
		return -1;
	}
	return 0;
}

int	rcv_cmd_heartbeat_req(void)
{
	myPs->pingCount = 0;
	myPs->pingTimer = myData->mData.misc.timer_1sec;

	send_cmd_heartbeat_reply();
	userlog(DEBUG_LOG, psName, "rcv_cmd_heartbeat_req \n");
	return 0;
}

int rcv_cmd_response(void)
{
	S_CAN_CLIENT_RCV_CMD_RESPONSE cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_CAN_CLIENT_RCV_CMD_RESPONSE));

	if(cmd.response.state == CAN_CLIENT_CD_ACK) {
		myPs->pingCount = 0;
		myPs->pingTimer = myData->mData.misc.timer_1sec;
		//userlog(DEBUG_LOG, psName, "response ACK : %d cmd_id[%d]\n", 
		//	cmd.response.code, cmd.response.cmd_id);
	} else {
		userlog(DEBUG_LOG, psName, "response error code : %d cmd_id[%d]\n", 
			cmd.response.state, cmd.response.cmd_id);
		return -1;
	}
	return 0;
}

int rcv_cmd_standby_req(void)
{
	myPs->signal[CAN_SIG_DATA_START] = P1;
	myPs->pingCount = 0;
	myPs->pingTimer = myData->mData.misc.timer_1sec;
	userlog(DEBUG_LOG, psName, "rcv_cmd_standby_req \n");
	return send_cmd_response(CAN_TO_SBC_CMD_STANDBY_REQ, CAN_CLIENT_CD_ACK);
}

int rcv_cmd_can_receive_set_req(void)
{
	int ch, ch_flag;
	S_CAN_CLIENT_RCV_CMD_RECEIVE_SET_REQ cmd;
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_CAN_CLIENT_RCV_CMD_RECEIVE_SET_REQ));

	ch = ch_flag = 0;	
	myPs->pingCount = 0;
	myPs->pingTimer = myData->mData.misc.timer_1sec;
	
	ch_flag = cmd.header.ch_flag;
	if(ch_flag == 0x01) {
		ch = 0;
	} else if(ch_flag == 0x02) {
		ch = 1;
	} else if(ch_flag == 0x04) {
		ch = 2;
	} else if(ch_flag == 0x08) {
		ch = 3;
	}
	userlog(DEBUG_LOG, psName, "rcv_cmd_can_receive_set_req ch %d %d\n", ch, ch_flag);
	return send_cmd_can_receive_set(ch);
}

int rcv_cmd_can_transmit_set_req(void)
{
	int ch, ch_flag;
	S_CAN_CLIENT_RCV_CMD_TRANSMIT_SET_REQ cmd;
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_CAN_CLIENT_RCV_CMD_TRANSMIT_SET_REQ));
	
	ch = ch_flag = 0;	
	myPs->pingCount = 0;
	myPs->pingTimer = myData->mData.misc.timer_1sec;
	
	ch_flag = cmd.header.ch_flag;
	if(ch_flag == 0x01) {
		ch = 0;
	} else if(ch_flag == 0x02) {
		ch = 1;
	} else if(ch_flag == 0x04) {
		ch = 2;
	} else if(ch_flag == 0x08) {
		ch = 3;
	}
	userlog(DEBUG_LOG, psName, "rcv_cmd_can_transmit_set_req ch %d, %d\n", ch, cmd.header.msg_count);
	myPs->misc.msg_count = cmd.header.msg_count;
	return send_cmd_can_transmit_set(ch);
}

int rcv_cmd_receive_data(void)
{
	int ch_flag, ch=0, i;
	S_CAN_CLIENT_RCV_RECEIVE_DATA cmd;
	memset((char *)&cmd, 0, sizeof(S_CAN_CLIENT_RCV_RECEIVE_DATA));
	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_CAN_CLIENT_RCV_RECEIVE_DATA));

	ch = ch_flag = 0;	
	myPs->pingCount = 0;
	myPs->pingTimer = myData->mData.misc.timer_1sec;

	ch_flag = cmd.header.ch_flag;
	if(ch_flag == 0x01) {
		ch = 0;
	} else if(ch_flag == 0x02) {
		ch = 1;
	} else if(ch_flag == 0x04) {
		ch = 2;
	} else if(ch_flag == 0x08) {
		ch = 3;
	}
	myData->cData[ch].misc.comm_state = cmd.header.can_status & ch_flag;
	//userlog(DEBUG_LOG, psName, "receive data error ch[%d] - %d\n",
	//	ch+1, cmd.header.can_status);
	for(i=0;i<MAX_RX_LTC_DATA;i++) {
		if(cmd.value[i].can_id == 0) {
			myPs->can_msg[ch].data_count = i;
			break;
		}
		myPs->can_msg[ch].msg[i].can_id = cmd.value[i].can_id;
		myPs->can_msg[ch].msg[i].data[0] = cmd.value[i].data[0];
		myPs->can_msg[ch].msg[i].data[1] = cmd.value[i].data[1];
		myPs->can_msg[ch].msg[i].data[2] = cmd.value[i].data[2];
		myPs->can_msg[ch].msg[i].data[3] = cmd.value[i].data[3];
		myPs->can_msg[ch].msg[i].data[4] = cmd.value[i].data[4];
		myPs->can_msg[ch].msg[i].data[5] = cmd.value[i].data[5];
		myPs->can_msg[ch].msg[i].data[6] = cmd.value[i].data[6];
		myPs->can_msg[ch].msg[i].data[7] = cmd.value[i].data[7];
	}
	if(ch_flag == 0x01) {
		//userlog(DEBUG_LOG, psName, "rcv_data %02x %02x %02x %02x %02x %02x %02x %02x ,ID - %08x\n", 
		//	cmd.value[0].data[0], cmd.value[0].data[1], cmd.value[0].data[2],
		//	cmd.value[0].data[3], cmd.value[0].data[4], cmd.value[0].data[5],
		//	cmd.value[0].data[6], cmd.value[0].data[7], cmd.value[0].can_id);
		userlog(DEBUG_LOG, psName, "rcv_data  %02x%02x%02x%02x %02x%02x%02x%02x ,ID - %08x\n", 
			cmd.value[0].data[3], cmd.value[0].data[2], cmd.value[0].data[1],
			cmd.value[0].data[0], cmd.value[0].data[7], cmd.value[0].data[6],
			cmd.value[0].data[5], cmd.value[0].data[4], cmd.value[0].can_id);
	} else if(ch_flag == 0x08) {
		myPs->signal[CAN_SIG_DATA_START] = P1;
		//if(myPs->signal[CAN_SIG_DATA_START] == P2) {
			send_cmd_ch_data();
		//} else {
		//	myPs->signal[CAN_SIG_DATA_START]++;
		//}
	}
	return 0;
}


