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
extern volatile S_TEMP_CLIENT *myPs;
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

	myPs->signal[TEMP_SIG_NET_CONNECTED] = P1;
	myPs->pingTimer = myData->mData.misc.timer_1sec;
	myPs->netTimer = myData->mData.misc.timer_1sec;

	userlog(DEBUG_LOG, psName,
		"command socket connected network_socket%d)\n",
		myPs->config.network_socket);
	
	return 0;
}

int NetworkPacket_Receive(void)
{
	int rcv_size, read_size, i, start, index;
	char maxPacketBuf[MAX_P1_RECV_PACKET_LENGTH];

	memset(maxPacketBuf, 0x00, MAX_P1_RECV_PACKET_LENGTH);
		
	if(ioctl(myPs->config.network_socket, FIONREAD, &rcv_size) < 0) {
		userlog(DEBUG_LOG, psName,
			"packet receive ioctl error\n");
		return -1;
	}

	if(rcv_size > MAX_P1_RECV_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName,
			"max packet size over\n");
		read_size = readn(myPs->config.network_socket, maxPacketBuf,
			MAX_P1_RECV_PACKET_LENGTH);
		if(read_size != MAX_P1_RECV_PACKET_LENGTH)
			userlog(DEBUG_LOG, psName,
				"packet readn size error1\n");
		return -2;
	} else if(rcv_size > (MAX_P1_RECV_PACKET_LENGTH - myPs->rcvPacket.usedBufSize)) {
		userlog(DEBUG_LOG, psName,
			"packet buffer overflow\n");
		read_size = readn(myPs->config.network_socket, maxPacketBuf, rcv_size);
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
	
	if(myPs->rcvPacket.rcvCount
		== myPs->rcvPacket.parseCount) return;
	
	i = myPs->rcvPacket.parseCount;
	myPs->rcvPacket.parseCount++;
	if(myPs->rcvPacket.parseCount > (MAX_P1_RECV_PACKET_COUNT-1))
		myPs->rcvPacket.parseCount = 0;
	
	cmdBuf_index = myPs->rcvCmd.cmdBufSize;
	myPs->rcvCmd.cmdBufSize
		+= myPs->rcvPacket.rcvSize[i];
	
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
	int	i, flag;
	char tmpBuf[MAX_P1_RECV_PACKET_LENGTH];

	S_TEMP_CLIENT_CMD_HEADER header;

	if(myPs->rcvCmd.cmdBufSize < sizeof(header)) return -1;
	
	if(myPs->rcvCmd.cmdBuf[0] != TEMP_CLIENT_STX) {
		userlog(DEBUG_LOG, psName, "packet STX error\n");	
		return -2; 
	}
	
	flag = 0;	
	for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
		if(myPs->rcvCmd.cmdBuf[i] == TEMP_CLIENT_ETX) {
			flag = 1;
			break;
		}
	}
	// buf is full (rest packet 0 or a little)i
	// if rest packet is 0 : rcvCmd.cmdBufSize = 0;
	// else rcvCmd.cmdBufSize = rest Size;
	// copy rest packet to &rcvCmd.cmdBuf[0]  
	if(flag == 1) {
		cmd_size = i+1;	
		memset((char *)&myPs->rcvCmd.cmd[0], 0x00, MAX_P1_RECV_PACKET_LENGTH);
		memcpy((char *)&myPs->rcvCmd.cmd[0],
			(char *)&myPs->rcvCmd.cmdBuf[0], cmd_size);
		myPs->rcvCmd.cmdSize = cmd_size;
	
		cmdBuf_index = cmd_size;
		myPs->rcvCmd.cmdBufSize -= cmd_size;
		cmd_size = myPs->rcvCmd.cmdBufSize;
		memset((char *)&tmpBuf[0], 0x00, MAX_P1_RECV_PACKET_LENGTH);
		memcpy((char *)&tmpBuf[0],
			(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
		memset((char *)&myPs->rcvCmd.cmdBuf[0], 0x00, MAX_P1_RECV_PACKET_LENGTH);
		memcpy((char *)&myPs->rcvCmd.cmdBuf[0],
			(char *)&tmpBuf[0], cmd_size);
	} else { //buf is not full (wait 1 more socket event)
		return -3;
	}
	
	return 0;
}

int NetworkCommand_Parsing(void)
{
	char			buf[12], tmp[12];
	char			body[MAX_P1_RECV_PACKET_LENGTH];
//	char			temp_log[MAX_P1_RECV_PACKET_LENGTH];
	int				cmdid, rtn, i, size;

	S_TEMP_CLIENT_CMD_HEADER	header;

	memset((char *)&header, 0x00, sizeof(header));
	memcpy((char *)&header, (char *)&myPs->rcvCmd.cmd[1],
		sizeof(header));
	
	memset(buf, 0x00, sizeof(buf));
	memcpy((char *)&buf[0], (char *)&header.cmdid[0], sizeof(header.cmdid));
	cmdid = atoi(buf);
	
	switch(cmdid){
		default:
			memset(buf, 0x00, sizeof(buf));
			memcpy(buf, (char *)&myPs->rcvCmd.cmd[1], sizeof(header));

			size = sizeof(header)+1;	
			memset(body, 0x00, sizeof(MAX_P1_RECV_PACKET_LENGTH));
			memset(tmp, 0x00, sizeof(tmp));
			for(i=size; i<MAX_P1_RECV_PACKET_LENGTH; i++) {
				if(myPs->rcvCmd.cmd[i] == TEMP_CLIENT_ETX) {
					memcpy(tmp, (char *)&body[i-size-2], 2);
					body[i-size-2]=0;	
					body[i-size-1]=0;	
					body[i-size] = 0;
					break;
				}
				body[i-size] = myPs->rcvCmd.cmd[i];
			}
//			sprintf(temp_log, "[T->M] CmdID=%3d Header=%s Body=%s ",
//				cmdid, (char *)&buf, (char *)&body);
//			userlog(DEB_LOG, psName, "%s\n",(char *)&temp_log);
			break;
	}
	
	rtn = CmdHeaderCheck((char *)&header);
	if(rtn < 0)	{
		userlog(DEBUG_LOG, psName, 
			"Temp Client CmdHeaderCheck error %d\n", rtn);
		return 0;
	}
	
	switch(cmdid) {
		case TEMP_TO_SBC_CMD_HEARTBEAT_RPY:
			rtn = rcv_cmd_heartbeat_rpy();
			break;
		default:
			userlog(DEBUG_LOG, psName, 
				"Can't Find Cmd[%d]\n",cmdid);
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
	int cmd_id, len=0, rtn;
	char buf[12];
	S_TEMP_CLIENT_CMD_HEADER	header;
	
	memset((char *)&header, 0x00, sizeof(header));
	memcpy((char *)&header, (char *)rcvHeader, sizeof(header));
	memset(buf, 0x00, sizeof(buf));
	memcpy((char *)&buf[0], (char *)&header.cmdid[0], sizeof(header.cmdid));
	cmd_id = atoi(buf);
	
	switch(cmd_id) {
		case TEMP_TO_SBC_CMD_HEARTBEAT_RPY:
			len = sizeof(S_TEMP_CLIENT_RCV_CMD_HEARTBEAT_RPY);
			break;
		default:
				userlog(DEBUG_LOG, psName, 
					"Templient RcvCmd command id error : %d\n", cmd_id);
			rtn = -1;
			break;
	}

	len += 2;

	if(len != myPs->rcvCmd.cmdSize) {
		userlog(DEBUG_LOG, psName, 
			"TempClient RcvCmd Data Size error (%d : %d)\n",
		len, myPs->rcvCmd.cmdSize);
		return -8;
	}
	return 0;
}

void make_header(char *buf, int cmd_id, int data_size)
{
	char temp[12];
	S_TEMP_CLIENT_CMD_HEADER	header;
	
	memset((char *)&header, 0x00, sizeof(header));

	sprintf((char *)&header.cmdid, "%03d", cmd_id);

	memset(temp, 0x00, sizeof(temp));
	sprintf(temp, "%03d", data_size);
	memcpy((char *)&header.data_size, temp, sizeof(header.data_size));

	memcpy(buf, (char *)&header, sizeof(S_TEMP_CLIENT_CMD_HEADER));
}

int	send_command(char *cmd, int cmd_size)
{
	char	packet[MAX_P1_RECV_PACKET_LENGTH];
	char	body[MAX_P1_RECV_PACKET_LENGTH];
	char	temp_log[MAX_P1_RECV_PACKET_LENGTH];
	char 	buf[12], tmp[12], header1[20];
	int i, rtn, cmdid, size;
	S_TEMP_CLIENT_CMD_HEADER header;

	memset((char *)&header, 0x00, sizeof(S_TEMP_CLIENT_CMD_HEADER));
	memcpy((char *)&header, cmd, sizeof(S_TEMP_CLIENT_CMD_HEADER));

	memset((char *)&header1, 0x00, sizeof(header1));
	memcpy((char *)&header1, cmd, sizeof(S_TEMP_CLIENT_CMD_HEADER));
	
	if(cmd_size > MAX_P1_RECV_PACKET_LENGTH-2) {
		userlog(DEBUG_LOG, psName,
			"CMD SEND FAIL!! TOO LARGE SIZE[%d]\n", cmd_size + 2);
		return -1;
	}
	
	memset((char *)&packet, 0x00, sizeof(packet));
	packet[0] = TEMP_CLIENT_STX;
	memcpy((char*)packet + 1, cmd, cmd_size);
	packet[cmd_size + 1] = TEMP_CLIENT_ETX;

	rtn = writen(myPs->config.network_socket, packet, cmd_size + 2);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd[%d] send error!!!\n",
			header.cmdid);
		return rtn;
	}

	if(myPs->config.CmdSendLog == P1) {
		size = sizeof(S_TEMP_CLIENT_CMD_HEADER)+1;
		memset(body, 0x00, sizeof(MAX_P1_RECV_PACKET_LENGTH));
		memset(tmp, 0x00, sizeof(tmp));
		for(i=size; i<MAX_P1_RECV_PACKET_LENGTH; i++) {
			if(packet[i] == TEMP_CLIENT_ETX) {
				memcpy(tmp, (char *)&body[i-size-2], 2);
				body[i-size-2] = 0;
				body[i-size-1] = 0;
				body[i-size] = 0;
				break;
			}
			body[i-size] = packet[i];
		}
		memset(temp_log, 0x00, sizeof(MAX_P1_RECV_PACKET_LENGTH));
		memset(buf, 0x00, sizeof(buf));
		memcpy(buf, (char *)&header.cmdid, sizeof(header.cmdid));
		cmdid = atoi(buf);

		switch(cmdid){
			case SBC_TO_TEMP_CMD_TEMP_REQ:
				break;
			default:
//				sprintf(temp_log, "CmdID=%3d Header=%s Body=%s CRC=%x%x",
//					cmdid, (char *)&header1, (char *)&body, (unsigned char)tmp[0], (unsigned char)tmp[1]);
//					userlog(TEMP_LOG, psName, "%s\n", (char *)&temp_log);
				break;
		}

	}
	return 0;
}

void make_crc(unsigned short incrc16, char *buf, unsigned int buflen)
{
	int i = 0;
	if(myPs->config.crc_type == P1){
		i = crc16polynomail_buf(incrc16, &buf[0], buflen);
	}else if(myPs->config.crc_type == P2){
		i = crc16ccitt_compute_buf(incrc16, &buf[0], buflen);
	}else if(myPs->config.crc_type == P3){
		i = crc16modbus_compute_buf(incrc16, &buf[0], buflen);
	}else{
	}
	buf[buflen] = (unsigned char)(i);
	buf[buflen + 1] = (unsigned char)(i >> 8);
//	userlog(DEBUG_LOG, psName, "Make CRC : %x, %x \n", (unsigned char)buf[buflen], (unsigned char)buf[buflen + 1]);
}

int check_crc(unsigned short incrc16, char *buf, unsigned int buflen)
{
	int i;
	if(myPs->config.crc_type == P1){
		i = crc16polynomail_buf(incrc16, &buf[0], buflen);
	}else if(myPs->config.crc_type == P2){
		i = crc16ccitt_compute_buf(incrc16, &buf[0], buflen);
	}else if(myPs->config.crc_type == P3){
		i = crc16modbus_compute_buf(incrc16, &buf[0], buflen);
	}else{
		return 0;
	}
/*	userlog(DEBUG_LOG, psName, "CRC Rcv Cmd Check CRC : %x, %x, Check CRC : %x, %x \n", (unsigned char)buf[buflen], (unsigned char)buf[buflen + 1], (unsigned char)i, (unsigned char)(i>>8));
*/
	if((unsigned char)buf[buflen] != (unsigned char)(i) ||
		(unsigned char)buf[buflen + 1] != (unsigned char)(i >> 8)) {
		userlog(DEBUG_LOG, psName, "Rcv Cmd CRC Error, Rcv CRC : %x, %x, Check CRC : %x, %x \n", (unsigned char)buf[buflen], (unsigned char)buf[buflen + 1], (unsigned char)i, (unsigned char)(i>>8));
		return -1;
	}
	return 0;
}


// Compute CRC-16-CCITT for buf; incrc16 - input value (must be 0 for the first call)
unsigned short crc16ccitt_compute_buf(unsigned short incrc16, char *buf, unsigned int buflen)
{
  unsigned int crc16tab[256]={
  0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
  0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
  0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
  0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
  0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
  0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
  0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
  0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
  0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
  0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
  0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
  0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
  0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
  0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
  0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
  0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
  0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
  0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
  0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
  0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
  0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
  0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
  0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
  0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
  0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
  0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
  0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
  0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
  0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
  0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
  0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
  0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0};

  unsigned int i;
  unsigned char *buf1;
  unsigned short crc16;
  
  crc16=incrc16;
  buf1=(unsigned char *)buf;
  for(i=0; i< buflen; i++)
  {
    crc16=(crc16<<8)^crc16tab[((crc16>>8)^*buf1) & 0x00FF];
    buf1++;
  }  
  return(crc16);
}

unsigned short crc16polynomail_buf(unsigned short incrc16, char *buf, unsigned int buflen)
{
//POLYNOMIAL 0x8005
 unsigned short crc16tab[256] = {0x0000,
	0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
	0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027,
	0x0022, 0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D,
	0x8077, 0x0072, 0x0050, 0x8055, 0x805F, 0x005A, 0x804B,
	0x004E, 0x0044, 0x8041, 0x80C3, 0x00C6, 0x00CC, 0x80C9,
	0x00D8, 0x80DD, 0x80D7, 0x00D2, 0x00F0, 0x80F5, 0x80FF,
	0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1, 0x00A0, 0x80A5,
	0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1, 0x8093,
	0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
	0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197,
	0x0192, 0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE,
	0x01A4, 0x81A1, 0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB,
	0x01FE, 0x01F4, 0x81F1, 0x81D3, 0x01D6, 0x01DC, 0x81D9,
	0x01C8, 0x81CD, 0x81C7, 0x01C2, 0x0140, 0x8145, 0x814F,
	0x014A, 0x815B, 0x015E, 0x0154, 0x8151, 0x8173, 0x0176,
	0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162, 0x8123,
	0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
	0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104,
	0x8101, 0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D,
	0x8317, 0x0312, 0x0330, 0x8335, 0x833F, 0x033A, 0x832B,
	0x032E, 0x0324, 0x8321, 0x0360, 0x8365, 0x836F, 0x036A,
	0x837B, 0x037E, 0x0374, 0x8371, 0x8353, 0x0356, 0x035C,
	0x8359, 0x0348, 0x834D, 0x8347, 0x0342, 0x03C0, 0x83C5,
	0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1, 0x83F3,
	0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
	0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7,
	0x03B2, 0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E,
	0x0384, 0x8381, 0x0280, 0x8285, 0x828F, 0x028A, 0x829B,
	0x029E, 0x0294, 0x8291, 0x82B3, 0x02B6, 0x02BC, 0x82B9,
	0x02A8, 0x82AD, 0x82A7, 0x02A2, 0x82E3, 0x02E6, 0x02EC,
	0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2, 0x02D0, 0x82D5,
	0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1, 0x8243,
	0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
	0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264,
	0x8261, 0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E,
	0x0234, 0x8231, 0x8213, 0x0216, 0x021C, 0x8219, 0x0208,
	0x820D, 0x8207, 0x0202 };

  unsigned int i;
  unsigned char *buf1;
  unsigned short crc16;
  
  crc16=incrc16;
  buf1=(unsigned char *)buf;
  for(i=0; i< buflen; i++)
  {
    crc16=(crc16<<8)^crc16tab[((crc16>>8)^*buf1) & 0x00FF];
    buf1++;
  }  
  return(crc16);
}


// Compute CRC-16-MODBUS for buf; incrc16 - input value (must be 0xFFFF for the first call)
// NOTE: polynom is 0xA001
unsigned short crc16modbus_compute_buf(unsigned short incrc16, char *buf, unsigned int buflen)
{
  unsigned short crc16mod_tab[256]={
  0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
  0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
  0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
  0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
  0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
  0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
  0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
  0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
  0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
  0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
  0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
  0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
  0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
  0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
  0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
  0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
  0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
  0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
  0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
  0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
  0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
  0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
  0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
  0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
  0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
  0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
  0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
  0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
  0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
  0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
  0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
  0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040};

  unsigned short i;
  unsigned char *buf1;
  unsigned short crc16, c;
  
  crc16=incrc16;
  buf1=(unsigned char *)buf;
  for(i=0; i<buflen; i++)
  {
    c=0x00FF & (unsigned short)*buf1;
    crc16=(crc16>>8)^crc16mod_tab[(crc16^c) & 0xFF];
    buf1++;
  }
  return(crc16);
}

int check_network_ping(void)
{
	int rtn = 0;

	if(myPs->signal[TEMP_SIG_NET_CONNECTED] != P1) return 0;

	if(myPs->pingCount >= myPs->config.retryCount) { 
		rtn = -1;
	}
	return rtn;
}

int network_ping(void)
{
	int diff, rtn = 0;

	if(myPs->signal[TEMP_SIG_NET_CONNECTED] != P1) return 0;

	diff = (int)labs(myData->mData.misc.timer_1sec - myPs->pingTimer);
	if(diff > myPs->config.heartbeatTimeout) { //Sec
		myPs->pingTimer = myData->mData.misc.timer_1sec;
		myPs->pingCount++;
		rtn = send_cmd_heartbeat_req();
	}

	return rtn;
}

int Check_NetworkState(void)
{
	int rtn=0;

	rtn = check_network_ping();
	if( rtn < 0){
		close(myPs->config.network_socket);
		return rtn;
	}
	rtn = network_ping();
	return rtn;
}

int	send_cmd_heartbeat_req(void)
{ 
	int	cmd_size, data_size;
	S_TEMP_CLIENT_SEND_CMD_HEARTBEAT_REQ	cmd;
	
	cmd_size = sizeof(S_TEMP_CLIENT_SEND_CMD_HEARTBEAT_REQ);
	memset((char *)&cmd, 0x00, cmd_size);

	data_size = 0;
	make_header((char *)&cmd, SBC_TO_TEMP_CMD_HEARTBEAT_REQ, data_size);
	if(send_command((char*)&cmd, cmd_size) < 0) {
		userlog(DEBUG_LOG, psName, "CMD[%d] SEND FAIL!\n", atoi(cmd.header.cmdid));
		return -1;
	}
	return 0;
}

int	send_cmd_response(int cmdid, int reply)
{ 
	int	cmd_size, data_size, ch;
	S_TEMP_CLIENT_SEND_CMD_RESPONSE	cmd;
	
	cmd_size = sizeof(S_TEMP_CLIENT_SEND_CMD_RESPONSE);
	memset((char *)&cmd, 0x00, cmd_size);

	data_size = 0;
	ch = 0;
	make_header((char *)&cmd, cmdid, data_size);
//	make_crc(0, (char *)&cmd, cmd_size - 2);
	if(send_command((char*)&cmd, cmd_size) < 0) {
		userlog(DEBUG_LOG, psName, "CMD[%d] SEND FAIL!\n", atoi(cmd.header.cmdid));
		return -1;
	}
	return 0;
}


int send_cmd_temp_req(void)
{
	int cmd_size, body_size, rtn=0;
	S_TEMP_CLIENT_SEND_CMD_TEMP_DATA_REQ	cmd;
	
	cmd_size = sizeof(S_TEMP_CLIENT_SEND_CMD_TEMP_DATA_REQ);
	body_size = cmd_size - sizeof(S_TEMP_CLIENT_CMD_HEADER)-2;

	memset((char *)&cmd, 0x00, cmd_size);
	make_header((char *)&cmd, SBC_TO_TEMP_CMD_TEMP_REQ, body_size);

//	make_crc(0, (char *)&cmd, cmd_size - 2);
	rtn = send_command((char *)&cmd, cmd_size);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "cmd send error!!! %d cmdid[%d]\n", rtn,
			SBC_TO_TEMP_CMD_TEMP_REQ);
	}
	return rtn;
}

int	rcv_cmd_heartbeat_rpy(void)
{
	myPs->pingCount = 0;

	return 0;
}

int rcv_cmd_response(void)
{
	myPs->pingCount = 0;
	return 0;
}

