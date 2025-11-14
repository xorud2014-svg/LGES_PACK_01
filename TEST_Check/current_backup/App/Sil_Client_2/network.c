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

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_SIL_CLIENT2 *myPs;
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

	myPs->signal[SIL_SIG_NET_CONNECTED] = P1;
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

	S_SIL_CLIENT2_CMD_HEADER header;

	if(myPs->rcvCmd.cmdBufSize < sizeof(header)) return -1;
	
	if(myPs->rcvCmd.cmdBuf[0] != SIL_CLIENT_STX) {
		userlog(DEBUG_LOG, psName, "packet STX error : %s, %s\n",myPs->rcvCmd.cmdBuf[0],
						myPs->rcvCmd.cmdBuf[1]);	
		return -2; 
	}
	
	// buf is full (rest packet 0 or a little)i
	// if rest packet is 0 : rcvCmd.cmdBufSize = 0;
	// else rcvCmd.cmdBufSize = rest Size;
	// copy rest packet to &rcvCmd.cmdBuf[0]  
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmdBuf, sizeof(S_SIL_CLIENT2_CMD_HEADER));
	cmd_size = header.body_size + sizeof(S_SIL_CLIENT2_CMD_HEADER) 
		+ sizeof(S_SIL_CLIENT2_CMD_TAIL);
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
	char			buf[12];
	int				rtn;

	S_SIL_CLIENT2_CMD_HEADER header;

	memset((char *)&header, 0, sizeof(header));
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd,
		sizeof(header));
	
	memset(buf, 0, sizeof(buf));
	memcpy((char *)&buf[0], (char *)&header.cmd_id, sizeof(header.cmd_id));
	rtn = CmdHeaderCheck((char *)&header);
	if(rtn < 0)	{
		userlog(DEBUG_LOG, psName, 
			"Sil Client CmdHeaderCheck error %d\n", rtn);
		return 0;
	}
	
	switch(header.cmd_id) {
		case SIL2_TO_SBC_CMD_HEARTBEAT_REQ:
			rtn = rcv_cmd_heartbeat_req();
			break;
		case SIL2_TO_SBC_CMD_DAQ_DATA_RPY:
			rtn = rcv_cmd_daq_data_rpy(header.module_no[1]);
			break;
		case SIL2_TO_SBC_CMD_RESPONSE:
			rtn = rcv_cmd_response();
			userlog(DEBUG_LOG, psName, 
				"SIL2_TO_SBC_CMD_RESPONSE Cmd[%d]\n",header.cmd_id);
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
	S_SIL_CLIENT2_CMD_HEADER header;
	
	memset((char *)&header, 0, sizeof(header));
	memcpy((char *)&header, (char *)rcvHeader, sizeof(header));
	memset(buf, 0, sizeof(buf));
	memcpy((char *)&buf[0], (char *)&header.cmd_id, sizeof(header.cmd_id));
	
	switch(header.cmd_id) {
		case SIL2_TO_SBC_CMD_HEARTBEAT_REQ:
			len = sizeof(S_SIL_CLIENT2_RCV_CMD_HEARTBEAT_REQ);
			break;
		case SIL2_TO_SBC_CMD_DAQ_DATA_RPY:
			len = sizeof(S_SIL_CLIENT2_RCV_CMD_DAQ_DATA);
			break;
		case SIL2_TO_SBC_CMD_RESPONSE:
			len = sizeof(S_SIL_CLIENT2_RCV_CMD_RESPONSE);
			break;
		default:
			userlog(DEBUG_LOG, psName, "Sil_Client RcvCmd command id error : %d : %d : %d : %d\n", 
				header.module_no[0], header.module_no[1], header.cmd_id, header.body_size);
			rtn = -1;
			break;
	}

	if(len != myPs->rcvCmd.cmdSize) {
		userlog(DEBUG_LOG, psName, 
			"SilClient RcvCmd Data Size error (%d : %d)\n",
		len, myPs->rcvCmd.cmdSize);
		return -8;
	}
	return 0;
}

void make_header(char *buf, int cmd_id, int body_size)
{
	char temp[12];
	S_SIL_CLIENT2_CMD_HEADER header;
	
	memset((char *)&header, 0x00, sizeof(header));

	memset(temp, 0x00, sizeof(temp));
	header.module_no[0] = SIL_CLIENT_STX;
	header.module_no[1] = 0x0F;
	header.cmd_id = cmd_id;
	header.body_size = body_size;

	memcpy(buf, (char *)&header, sizeof(S_SIL_CLIENT2_CMD_HEADER));
}

int check_network_ping(void)
{
	int rtn = 0;

	if(myPs->signal[SIL_SIG_NET_CONNECTED] != P1) return 0;

	if(myPs->pingCount >= myPs->config.retryCount) { 
		myPs->pingCount = 0;
		rtn = -1;
	}
	return rtn;
}

int network_ping(void)
{
	int diff, rtn = 0;

	if(myPs->signal[SIL_SIG_NET_CONNECTED] != P1) return 0;

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
		return rtn;
	}
	rtn = network_ping();
	return rtn;
}

int	send_command(char *cmd, int cmd_size)
{
	char	packet[MAX_P1_RECV_PACKET_LENGTH];
	int 	rtn;
	S_SIL_CLIENT2_CMD_HEADER header;

	memset((char *)&header, 0x00, sizeof(S_SIL_CLIENT2_CMD_HEADER));
	memcpy((char *)&header, cmd, sizeof(S_SIL_CLIENT2_CMD_HEADER));

	
	if(cmd_size > MAX_P1_RECV_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName,
			"CMD SEND FAIL!! TOO LARGE SIZE[%d]\n", cmd_size);
		return -1;
	}
	
	memset((char *)&packet, 0x00, sizeof(packet));
	memcpy((char*)packet, cmd, cmd_size);

	rtn = writen(myPs->config.network_socket, packet, cmd_size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd[%d] send error!!!\n", header.cmd_id);
		return rtn;
	}
	return 0;
}

int send_cmd_vi_start(int ch)
{
	int cmd_size, body_size, rtn;
	S_SIL_CLIENT2_RCV_CMD_START_REQ	cmd;

	cmd_size = sizeof(S_SIL_CLIENT2_RCV_CMD_START_REQ);
	memset((char *)&cmd, 0, cmd_size);
	//cmd.header.module_no[1] = ch;
	cmd.tail.etx = SIL_CLIENT_ETX;
	body_size = 0;
	make_header((char *)&cmd, SBC_TO_SIL2_CMD_START, body_size);
	rtn = send_command((char *)&cmd, cmd_size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "CMD[%d] SEND FAIL!\n", cmd.header.cmd_id);
		return -1;
	}
	return 0;
}

int send_cmd_vi_stop(int ch)
{
	int cmd_size, body_size, rtn;
	S_SIL_CLIENT2_RCV_CMD_STOP_REQ	cmd;

	cmd_size = sizeof(S_SIL_CLIENT2_RCV_CMD_STOP_REQ);
	memset((char *)&cmd, 0, cmd_size);
	//cmd.header.module_no[1] = ch;
	cmd.tail.etx = SIL_CLIENT_ETX;
	body_size = 0;
	make_header((char *)&cmd, SBC_TO_SIL2_CMD_STOP, body_size);
	rtn = send_command((char *)&cmd, cmd_size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "CMD[%d] SEND FAIL!\n", cmd.header.cmd_id);
		return -1;
	}
	return 0;
}

int	send_cmd_response(int cmd_id, int reply)
{ 
	int	cmd_size, body_size, rtn;
	S_SIL_CLIENT2_SEND_CMD_RESPONSE	cmd;
	
	cmd_size = sizeof(S_SIL_CLIENT2_SEND_CMD_RESPONSE);
	body_size = cmd_size - sizeof(S_SIL_CLIENT2_CMD_HEADER) - sizeof(S_SIL_CLIENT_CMD_TAIL);
	memset((char *)&cmd, 0, cmd_size);

	make_header((char *)&cmd, SBC_TO_SIL2_CMD_RESPONSE, body_size);
	cmd.response.cmd_id = cmd_id;
	cmd.response.code = reply;
	cmd.tail.etx = SIL_CLIENT_ETX;
	//userlog(DEBUG_LOG, psName, "response_cmd_id[%d] , response code = %d, body_size = %d\n"
	//	, cmd.response.cmd_id, cmd.response.code, body_size);
	rtn = send_command((char *)&cmd, cmd_size);
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
	return send_cmd_response(SIL2_TO_SBC_CMD_HEARTBEAT_REQ, P1_CD_ACK);
}

int rcv_cmd_response(void)
{
	S_SIL_CLIENT2_RCV_CMD_RESPONSE cmd;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_SIL_CLIENT2_RCV_CMD_RESPONSE));

	if(cmd.response.code == SIL_CLIENT_CD_ACK) {
		myPs->pingCount = 0;
		myPs->pingTimer = myData->mData.misc.timer_1sec;
		//userlog(DEBUG_LOG, psName, "response ACK : %d cmd_id[%d]\n", 
		//	cmd.response.code, cmd.response.cmd_id);
	} else {
		userlog(DEBUG_LOG, psName, "response error code : %d cmd_id[%d]\n", 
			cmd.response.code, cmd.response.cmd_id);
		return -1;
	}
	return 0;
}

int rcv_cmd_daq_data_rpy(int ch)
{
	int rtn=0;
	S_SIL_CLIENT2_RCV_CMD_DAQ_DATA cmd;

	if(ch == 0x01) ch = 0;
	else if(ch == 0x02) ch = 1;
	else if(ch == 0x04) ch = 2;
	else if(ch == 0x08) ch = 3;
	else ch = 0;
	myPs->pingCount = 0;
	myPs->pingTimer = myData->mData.misc.timer_1sec;

	memcpy((char *)&cmd, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_SIL_CLIENT2_RCV_CMD_DAQ_DATA));
	myData->cData[ch].misc.sil_v_i_val[0] = cmd.Vsens;
	myData->cData[ch].misc.sil_v_i_val[1] = cmd.Isens;
	//userlog(DEBUG_LOG, psName, 
	//	"cmd.Vsens = %d, cmd.Isens = %d, ch = %d\n",myData->cData[ch].misc.sil_v_i_val[0], myData->cData[ch].misc.sil_v_i_val[1], ch);
	return rtn;
}
