#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"
#include "local_message.h"
#include "com_io.h"
#include "comm.h"
#include "serial.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_ANALOG_METER  *myPs;
extern char	psName[PROCESS_NAME_SIZE];
extern CommParam comm_param;// = {"/dev/ttyS0", TMT_B96, TMT_FXONXOFF};

int InitComPort(void)
{
	int tty, bps, commType, rtn;

	sleep(2);

	tty = myPs->config.comPort;
	if(myPs->config.functionType == 0) {
		bps = myPs->config.comBps;
		commType = (int)myPs->config.commType;
	} else {
		bps = myPs->config.comBps2;
		commType = (int)myPs->config.commType2;
	}

	memset((char *)&comm_param.device, 0, COM_DEVICE_SIZE);
	switch(tty) {
		case 1:
			memcpy((char *)&comm_param.device, "/dev/ttyS0", 10);	break;
		case 2:
			memcpy((char *)&comm_param.device, "/dev/ttyS1", 10);	break;
		case 3:
			memcpy((char *)&comm_param.device, "/dev/ttyS2", 10);	break;
		case 4:
			memcpy((char *)&comm_param.device, "/dev/ttyS3", 10);	break;
		default:
			memcpy((char *)&comm_param.device, "/dev/ttyS0", 10);	break;
	}
	switch(bps) {
		case 300:	comm_param.bps = TMT_B3;	break;
		case 600:	comm_param.bps = TMT_B6;	break;
		case 1200:	comm_param.bps = TMT_B12;	break;
		case 2400:	comm_param.bps = TMT_B24;	break;
		case 4800:	comm_param.bps = TMT_B48;	break;
		case 9600:	comm_param.bps = TMT_B96;	break;
		case 19200:	comm_param.bps = TMT_B192;	break;
		case 38400:	comm_param.bps = TMT_B384;	break;
		case 57600:	comm_param.bps = TMT_B576;	break;
		case 115200:	comm_param.bps = TMT_B1152;	break;
	}
	comm_param.flow = TMT_FNONE;
	//comm_param.flow = TMT_FXONXOFF;
	comm_param.type = commType;

	rtn = opentty();
	myPs->misc.ttyS_fd = rtn;
	return rtn;
}
	
int SerialPacket_Receive(void)
{
	char maxPacketBuf[MAX_SERIAL_PACKET_LENGTH];
	int rcv_size, read_size, i, start, index;

	memset(maxPacketBuf, 0, MAX_SERIAL_PACKET_LENGTH);
		
	if(ioctl(myPs->misc.ttyS_fd, FIONREAD, &rcv_size) < 0) {
		userlog(DEBUG_LOG, psName, "packet receive ioctl error\n");
		close(myPs->misc.ttyS_fd);
		return -1;
	}

	if(rcv_size > MAX_SERIAL_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName, "max packet size over\n");
		read_size = readn(myPs->misc.ttyS_fd, maxPacketBuf,
			MAX_SERIAL_PACKET_LENGTH);
		if(read_size != MAX_SERIAL_PACKET_LENGTH)
			userlog(DEBUG_LOG, psName, "packet readn size error1\n");
		close(myPs->misc.ttyS_fd);
		return -2;
	} else if(rcv_size > (MAX_SERIAL_PACKET_LENGTH
		- myPs->rcvPacket.usedBufSize)) {
		userlog(DEBUG_LOG, psName, "packet buffer overflow\n");
		read_size = readn(myPs->misc.ttyS_fd, maxPacketBuf, rcv_size);
		if(read_size != rcv_size)
			userlog(DEBUG_LOG, psName, "packet readn size error2\n");
		close(myPs->misc.ttyS_fd);
		return -3;
	} else if(rcv_size <= 0) {
		memset((char *)&myPs->rcvPacket, 0, sizeof(S_ANALOG_METER_RCV_PACKET));
		memset((char *)&myPs->rcvCmd, 0, sizeof(S_ANALOG_METER_RCV_COMMAND));
		userlog(DEBUG_LOG, psName, "packet sock_rcv error %d\n", rcv_size);
		close(myPs->misc.ttyS_fd);
		return -4;
	} else {
		read_size = readn(myPs->misc.ttyS_fd, maxPacketBuf, rcv_size);
		if(read_size != rcv_size) {
			userlog(DEBUG_LOG, psName, "packet readn size error3 : %d, %d\n",
				read_size, rcv_size);
			close(myPs->misc.ttyS_fd);
			return -5;
		}
	}

	//userlog(DEBUG_LOG, psName, "recvCmd %s\n", maxPacketBuf); //kjgd

	i = myPs->rcvPacket.rcvCount;
	myPs->rcvPacket.rcvCount++;
	if(myPs->rcvPacket.rcvCount > (MAX_SERIAL_PACKET_COUNT-1))
		myPs->rcvPacket.rcvCount = 0;
	
	if(i == 0) index = MAX_SERIAL_PACKET_COUNT - 1;
	else index = i - 1;
	start = myPs->rcvPacket.rcvStartPoint[index]
		+ myPs->rcvPacket.rcvSize[index];
	if(start >= MAX_SERIAL_PACKET_LENGTH) {
		myPs->rcvPacket.rcvStartPoint[i]
			= abs(start - MAX_SERIAL_PACKET_LENGTH);
	} else {
		myPs->rcvPacket.rcvStartPoint[i] = start;
	}

	myPs->rcvPacket.rcvSize[i] = read_size;
	myPs->rcvPacket.usedBufSize += read_size;
	
	start = myPs->rcvPacket.rcvStartPoint[i];
	if((start + read_size) > MAX_SERIAL_PACKET_LENGTH) {
		index = MAX_SERIAL_PACKET_LENGTH - start;
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

void SerialPacket_Parsing(void)
{
//	char debug[MAX_SERIAL_PACKET_LENGTH]; //kjgd
	int i, j, k, cmdBuf_index, start_point;
	
	if(myPs->rcvPacket.rcvCount == myPs->rcvPacket.parseCount) return;
	
	i = myPs->rcvPacket.parseCount;
	myPs->rcvPacket.parseCount++;
	if(myPs->rcvPacket.parseCount > (MAX_SERIAL_PACKET_COUNT-1))
		myPs->rcvPacket.parseCount = 0;
	
	cmdBuf_index = myPs->rcvCmd.cmdBufSize;
	myPs->rcvCmd.cmdBufSize += myPs->rcvPacket.rcvSize[i];
	
	start_point = myPs->rcvPacket.parseStartPoint[i];

//	userlog(METER2_LOG, psName, "recvCmd1 %s:end %d %d\n",
//		myPs->rcvPacket.rcvPacketBuf[start_point], i, start_point); //kjgd
/*
	memset(debug, 0, sizeof debug);
	memcpy((char *)&debug, (char *)&myPs->rcvPacket.rcvPacketBuf[start_point],
		myPs->rcvPacket.rcvSize[i]);
	//userlog(METER2_LOG, psName, "recvCmd2 %s:end %d %d\n",
	//	debug, start_point, myPs->rcvPacket.rcvSize[i]); //kjgd
	
	userlog(METER2_LOG, psName, "recvCmd2a");
	for(j=0; j < myPs->rcvPacket.rcvSize[i]; j++) {
		userlog2(METER2_LOG, psName, " %02x", debug[j]);
	}
	userlog2(METER2_LOG, psName, ":end\n"); //kjgd
*/
	j = start_point + myPs->rcvPacket.rcvSize[i];
	if(j <= MAX_SERIAL_PACKET_LENGTH) {
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point],
			myPs->rcvPacket.rcvSize[i]);
	} else {
		k = MAX_SERIAL_PACKET_LENGTH - start_point;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
		cmdBuf_index += k;
		start_point = 0;
		k = j - MAX_SERIAL_PACKET_LENGTH;
		memcpy((char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myPs->rcvPacket.rcvPacketBuf[start_point], k);
	}
		
	start_point = myPs->rcvPacket.parseStartPoint[i]
		+ myPs->rcvPacket.rcvSize[i];
	if(start_point >= MAX_SERIAL_PACKET_LENGTH) {
		j = i + 1;
		if(j >= MAX_SERIAL_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j]
			= abs(start_point - MAX_SERIAL_PACKET_LENGTH);
	} else {
		j = i + 1;
		if(j >= MAX_SERIAL_PACKET_COUNT) j = 0;
		myPs->rcvPacket.parseStartPoint[j] = start_point;
	}
		
	myPs->rcvPacket.usedBufSize -= myPs->rcvPacket.rcvSize[i];

//	userlog(METER2_LOG, psName, "recvCmd3 %d %d\n", myPs->rcvPacket.usedBufSize,
//		myPs->rcvCmd.cmdBufSize); //kjgd
}

int SerialCommand_Receive(void)
{
	//char debug[MAX_SERIAL_PACKET_LENGTH]; //kjgd
	int i, cmd_size=0, cmdBuf_index;

	if(myPs->rcvCmd.cmdBufSize < 1) return -1;
	
	if(myPs->config.functionType == 0) { //Measurement Computing CB-7018
		if(myPs->rcvCmd.cmdBuf[0] == '!' || myPs->rcvCmd.cmdBuf[0] == '?'
			|| myPs->rcvCmd.cmdBuf[0] == '>') {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\r') { //0x0D
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -2;
		} else if(myPs->rcvCmd.cmdBuf[0] == '%' || myPs->rcvCmd.cmdBuf[0] == '$'
			|| myPs->rcvCmd.cmdBuf[0] == '#') { //for EHCO DATA(RS485)
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\r') { //0x0D
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -3;

			cmdBuf_index = cmd_size;
			myPs->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myPs->rcvCmd.cmdBufSize;
			memset((char *)&myPs->rcvCmd.tmpBuf, 0, MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.tmpBuf,
				(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myPs->rcvCmd.cmdBuf, 0, MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.cmdBuf,
				(char *)&myPs->rcvCmd.tmpBuf, cmd_size);

			return 1;
		} else {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\r') { //0x0D
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -4;

			cmdBuf_index = cmd_size;
			myPs->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myPs->rcvCmd.cmdBufSize;
			memset((char *)&myPs->rcvCmd.tmpBuf, 0, MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.tmpBuf,
				(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myPs->rcvCmd.cmdBuf, 0, MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.cmdBuf,
				(char *)&myPs->rcvCmd.tmpBuf, cmd_size);

			return 2;
		}
	} else { //Yokogawa XL122
//		userlog(METER2_LOG, psName, "recvCmd4 %d %02x\n",
//			myPs->rcvCmd.cmdBufSize, myPs->rcvCmd.cmdBuf[0]); //kjgd

		if((myPs->rcvCmd.cmdBuf[0] == 0x1B && myPs->rcvCmd.cmdBuf[1] == 'O')
			|| (myPs->rcvCmd.cmdBuf[0] == 0x1B && myPs->rcvCmd.cmdBuf[1] == 'C')
			|| (myPs->rcvCmd.cmdBuf[0] == 'N' && myPs->rcvCmd.cmdBuf[1] == ' ')
			|| (myPs->rcvCmd.cmdBuf[0] == 'S' && myPs->rcvCmd.cmdBuf[1] == ' ')
			|| (myPs->rcvCmd.cmdBuf[0] == 'O' && myPs->rcvCmd.cmdBuf[1] == ' ')
			|| (myPs->rcvCmd.cmdBuf[0] == 'E' && myPs->rcvCmd.cmdBuf[1] == ' ')
			) {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\n') { //0x0A
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -2;
		} else if(myPs->rcvCmd.cmdBuf[0] == 0x09
			|| (myPs->rcvCmd.cmdBuf[0] == 'E' && myPs->rcvCmd.cmdBuf[1] == '0')
			|| (myPs->rcvCmd.cmdBuf[0] == 'E' && myPs->rcvCmd.cmdBuf[1] == '1')
			|| (myPs->rcvCmd.cmdBuf[0] == 'E' && myPs->rcvCmd.cmdBuf[1] == '2')
			|| (myPs->rcvCmd.cmdBuf[0] == 'E' && myPs->rcvCmd.cmdBuf[1] == 'A')
			|| (myPs->rcvCmd.cmdBuf[0] == 'D' && myPs->rcvCmd.cmdBuf[1] == 'A')
			|| (myPs->rcvCmd.cmdBuf[0] == 'T' && myPs->rcvCmd.cmdBuf[1] == 'I')
			|| (myPs->rcvCmd.cmdBuf[0] == 'E' && myPs->rcvCmd.cmdBuf[1] == 'N')
			) {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\n') { //0x0A
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -3;

			cmdBuf_index = cmd_size;
			myPs->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myPs->rcvCmd.cmdBufSize;
			memset((char *)&myPs->rcvCmd.tmpBuf, 0, MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.tmpBuf,
				(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myPs->rcvCmd.cmdBuf, 0, MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.cmdBuf,
				(char *)&myPs->rcvCmd.tmpBuf, cmd_size);

			return 1;
		} else {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\n') { //0x0A
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -4;

			cmdBuf_index = cmd_size;
			myPs->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myPs->rcvCmd.cmdBufSize;
			memset((char *)&myPs->rcvCmd.tmpBuf, 0, MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.tmpBuf,
				(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myPs->rcvCmd.cmdBuf, 0, MAX_SERIAL_PACKET_LENGTH);
			memcpy((char *)&myPs->rcvCmd.cmdBuf,
				(char *)&myPs->rcvCmd.tmpBuf, cmd_size);

			return 2;
		}
	}

//	userlog(METER2_LOG, psName, "recvCmd5 %s:end %d\n",
//		myPs->rcvCmd.cmdBuf, cmd_size); //kjgd
			
	memset((char *)&myPs->rcvCmd.cmd, 0, MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmd, (char *)&myPs->rcvCmd.cmdBuf, cmd_size);
	myPs->rcvCmd.cmdSize = cmd_size;
	
	cmdBuf_index = cmd_size;
	myPs->rcvCmd.cmdBufSize -= cmd_size;
	cmd_size = myPs->rcvCmd.cmdBufSize;
	memset((char *)&myPs->rcvCmd.tmpBuf, 0, MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.tmpBuf,
		(char *)&myPs->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
	memset((char *)&myPs->rcvCmd.cmdBuf, 0, MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&myPs->rcvCmd.cmdBuf,
		(char *)&myPs->rcvCmd.tmpBuf, cmd_size);

/*	memset(debug, 0, sizeof debug);
	memcpy((char *)&debug, (char *)&myPs->rcvCmd.cmdBuf, myPs->rcvCmd.cmdSize);
	userlog(METER2_LOG, psName, "recvCmd6 %s:end %d %d\n",
		debug, myPs->rcvCmd.cmdSize, cmdBuf_index); //kjgd*/
	
	return 0;
}

int SerialCommand_Parsing(void)
{
	unsigned char tmp, log_flag;
	int		rtn, i;
	
	if(myPs->config.CmdRcvLog == P1) {
		log_flag = 0;
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			log_flag = 1;
		}

		if(log_flag == 1) {
			userlog(METER2_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
		}
	}
	
	if(myPs->config.CmdRcvLog_Hex == P1) {
		log_flag = 0;
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			log_flag = 1;
		}

		if(log_flag == 1) {
			userlog(METER2_LOG, psName, "recvCmd");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				tmp = myPs->rcvCmd.cmd[i];
				userlog2(METER2_LOG, psName, " %02x", tmp);
			}
			userlog2(METER2_LOG, psName, ":end\n");
		}
	}

	if(myPs->config.functionType == 0) { //Measurement Computing CB-7018
		rtn = CmdHeader_Check1();
		if(rtn < 0) return -1;
	
		rtn = rcv_cmd_answer1();
	} else { //Yokogawa XL122
		rtn = CmdHeader_Check2();

		if(rtn == 1) {
			rtn = rcv_cmd_open_close();
		} else if(rtn == 2) {
			rtn = rcv_cmd_answer2();
		} else rtn = (-1);
	}

	return rtn;
}

int Parsing_SerialEvent(void)
{
	int rtn;

	SerialPacket_Parsing();

	while(1) {
		rtn = SerialCommand_Receive();
		if(rtn < 0) break;
		else if(rtn > 0) continue;
		else {
			if(SerialCommand_Parsing() < 0) {
				myPs->rcvCmd.cmdFail++;
				if(myPs->rcvCmd.cmdFail >= 3) {
					myPs->rcvCmd.cmdFail = 0;
					myPs->rcvCmd.cmdBufSize = 0;
					memset((char *)&myPs->rcvCmd.cmdBuf, 0,
						MAX_SERIAL_PACKET_LENGTH);
					return -1;
				}
			} else {
				myPs->rcvCmd.cmdFail = 0;
			}
		}
	}

	return 0;
}

int CmdHeader_Check1(void)
{
	int	length;
	S_ANALOG_METER_CMD_HEADER	header;

	length = sizeof(S_ANALOG_METER_CMD_HEADER);
	memcpy((char *)&header, (char *)myPs->rcvCmd.cmd, length);

	if(length > myPs->rcvCmd.cmdSize) {
		userlog(DEBUG_LOG, psName, "RcvCmd size error (%d:%d)\n",
			length, myPs->rcvCmd.cmdSize);
		return -1;
	}
	
	if(header.stx != '!' && header.stx != '?' && header.stx != '>') {
		userlog(DEBUG_LOG, psName, "RcvCmd stx error : 0x%x\n", header.stx);
		return -2;
	}
	
	if(header.stx == '!' || header.stx == '?') {
		if(header.addr1 != '0') {
			userlog(DEBUG_LOG, psName, "RcvCmd addr1 error : 0x%x\n",
				header.addr1);
			return -3;
		}

		if(header.addr2 != '1' && header.addr2 != '2'
			&& header.addr2 != '3' && header.addr2 != '4') {
			userlog(DEBUG_LOG, psName, "RcvCmd addr2 error : 0x%x\n",
				header.addr2);
			return -4;
		}
	}

	return 0;
}

int CmdHeader_Check2(void)
{
	int	val;
	S_ANALOG_METER_RCV_CMD_ANSWER2	answer;

	if(myPs->rcvCmd.cmdSize == 7) {
		if((myPs->rcvCmd.cmd[0] == 0x1B)
			&& (myPs->rcvCmd.cmd[1] == 'O' || myPs->rcvCmd.cmd[1] == 'C')
			&& (myPs->rcvCmd.cmd[2] == ' ') && (myPs->rcvCmd.cmd[5] == 0x0D)) {
			return 1;
		} else {
			userlog(DEBUG_LOG, psName, "RcvCmd2 open_close error %d\n",
				myPs->signal[ANALOG_METER_SIG_MEASURE]);
			return -1;
		}
	} else if(myPs->rcvCmd.cmdSize == 27) {
		memcpy((char *)&answer, (char *)myPs->rcvCmd.cmd, 27);
	} else {
		/*kjgd userlog(DEBUG_LOG, psName, "RcvCmd2 size error1 %d : %d\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], myPs->rcvCmd.cmdSize);*/
		return -1;
	}

	if((answer.Status != 'N') && (answer.Status != 'S')
		&& (answer.Status != 'O') && (answer.Status != 'E')) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data Status error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.Status);
		return -2;
	}

	if(answer.SP1 != ' ') {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data SP1 error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.SP1);
		return -3;
	}
	
	if(answer.AnalogChannel != '0') {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data AnalogChannel error %d:0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.AnalogChannel);
		return -4;
	}

	val = (((int)answer.CH[0] - 0x30) * 10);
	val += ((int)answer.CH[1] - 0x30);
	if((val < 1) || (val > 16)) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data CH error %d : %d : %x %x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE],
			val, answer.CH[0], answer.CH[1]);
		return -5;
	}

	if((answer.Alarm[1] != ' ') || (answer.Alarm[2] != ' ')
		|| (answer.Alarm[3] != ' ')) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data Alarm error %d:%x %x %x %x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE],
			answer.Alarm[0], answer.Alarm[1], answer.Alarm[2], answer.Alarm[3]);
		return -6;
	}
	
	if((answer.Units[0] != '^') || (answer.Units[1] != 'C')
		|| (answer.Units[2] != ' ') || (answer.Units[3] != ' ')
		|| (answer.Units[4] != ' ') || (answer.Units[5] != ' ')) {
		userlog(DEBUG_LOG, psName,
			"RcvCmd2 data Units error %d:%x %x %x %x %x %x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE],
			answer.Units[0], answer.Units[1], answer.Units[2],
			answer.Units[3], answer.Units[4], answer.Units[5]);
		return -7;
	}
	
	if((answer.Sign != '+') && (answer.Sign != '-')) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data Sign error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.Sign);
		return -8;
	}
	
	if(answer.E != 'E') {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data E error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.E);
		return -9;
	}
	
	if((answer.Plus_Minus != '+') && (answer.Plus_Minus != '-')) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data Plus_Minus error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.Plus_Minus);
		return -10;
	}
	
	if(answer.CR != 0x0D) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 data CR error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], answer.CR);
		return -11;
	}
	
	return 2;
}

int rcv_cmd_answer1(void)
{
	char buf[10];
	int i, index=0, temp_bd, temp_ch, ch;
	long temp[8];
	S_ANALOG_METER_RCV_CMD_ANSWER	answer;
	
	memset((char *)&answer, 0, sizeof(S_ANALOG_METER_RCV_CMD_ANSWER));
	memcpy((char *)&answer, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_ANALOG_METER_RCV_CMD_ANSWER));
	
	for(i=0; i < 100; i++) {
		if(answer.data[i] == 0) {
			index = i;
			break;
		} else {
		}
	}

	if(answer.data[0] == '!' || answer.data[0] == '$') {
		return 0;
	}

	if(answer.data[0] == '>') {
		if(index == 9) {
			if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P0) {
			} else {
				userlog(DEBUG_LOG, psName, "answer size error1 %d\n", index);
				index = (int)myPs->signal[ANALOG_METER_SIG_MEASURE] % 10;
				if(index > myPs->config.countMeter) {
					index = 1;
				}
				myPs->signal[ANALOG_METER_SIG_MEASURE] = (unsigned char)index;
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
			}
			return 0;
		} else if(index == 58) {
		} else {
			/*kjg070909 userlog(DEBUG_LOG, psName, "answer size error2 %d\n", index);
			index = (int)myPs->signal[ANALOG_METER_SIG_MEASURE] % 10;
			if(index > myPs->config.countMeter) {
				index = 1;
			}
			myPs->signal[ANALOG_METER_SIG_MEASURE] = (unsigned char)index;
			myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;*/
			return 0;
		}
	}

	temp_ch = (-2);
	index = 0;
	memset(buf, 0, 10);
	for(i=1; i < 100; i++) {
		if(answer.data[i] == '+' || answer.data[i] == '-') {
			temp_ch++;
			if(temp_ch >= 0) temp[temp_ch] = (long)(atof(buf) * 1000.0);
			memset(buf, 0, 10);
			index = 0;
			buf[index] = answer.data[i];
		} else if(answer.data[i] == 0) {
			temp_ch++;
			temp[temp_ch] = (long)(atof(buf) * 1000.0);
			break;
		} else {
			index++;
			buf[index] = answer.data[i];
		}
	}

	temp_bd = (int)((int)myPs->signal[ANALOG_METER_SIG_MEASURE] % 10) - 1;
	if(temp_bd < 0) {
		userlog(DEBUG_LOG, psName, "AnalogValue ");
		for(temp_ch=0; temp_ch < 8; temp_ch++) {
			userlog2(DEBUG_LOG, psName, "%f ", (float)temp[temp_ch] / 1000.0);
		}
		userlog2(DEBUG_LOG, psName, "\n");
		return 0;
	}

	for(temp_ch=0; temp_ch < 8; temp_ch++) {
		i = temp_bd * 8 + temp_ch; //temp_hw_no index
		index = (int)myPs->Array2[i][0] - 1; //temp_monitor_no index
		if(index >= 0) {
			ch = myPs->Array1[index][1] - 1;
			myPs->tmp_value[ch] = temp[temp_ch]
				+ myPs->config.measure_offset[temp_bd];
			myPs->value[ch] = myPs->tmp_value[ch];
		}
	}

	if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P11) {
		if(myPs->config.countMeter > 1) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P2;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P12) {
		if(myPs->config.countMeter > 2) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P3;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P13) {
		if(myPs->config.countMeter > 3) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P4;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P14) {
		if(myPs->config.countMeter > 4) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P5;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	}

	return 0;
}

int rcv_cmd_open_close(void)
{
	char buf[4];
	int bd, measure_bd;

	memset((char *)&buf, 0, sizeof buf);
	memcpy((char *)&buf, (char *)&myPs->rcvCmd.cmd[3], 2);
	bd = atoi(buf);
	measure_bd = (int)myPs->signal[ANALOG_METER_SIG_MEASURE] % 10;

	if(myPs->rcvCmd.cmd[1] == 'O') {
		if(bd >= 1 && bd <= myPs->config.countMeter) {
			if(bd == measure_bd) {
				myPs->signal[ANALOG_METER_SIG_COMM_BUS_ENABLE]
					= (unsigned char)bd;
				myPs->signal[ANALOG_METER_SIG_MEASURE] += 10;
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				memset((char *)&myPs->rcvPacket, 0,
					sizeof(S_ANALOG_METER_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0,
					sizeof(S_ANALOG_METER_RCV_COMMAND));
			}
		}
	} else if(myPs->rcvCmd.cmd[1] == 'C') {
		if(bd >= 1 && bd <= myPs->config.countMeter) {
			if(bd == measure_bd) {
				myPs->signal[ANALOG_METER_SIG_COMM_BUS_ENABLE] = P0;
				/*if(myPs->signal[ANALOG_METER_SIG_MEASURE] < P71) {
					myPs->signal[ANALOG_METER_SIG_MEASURE] += 10;
				} else {*/
					if((bd+1) <= myPs->config.countMeter) {
						myPs->signal[ANALOG_METER_SIG_MEASURE] = (int)(bd+1);
					} else {
						myPs->signal[ANALOG_METER_SIG_MEASURE] = P1;
					}
				//}
				myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
				memset((char *)&myPs->rcvPacket, 0,
					sizeof(S_ANALOG_METER_RCV_PACKET));
				memset((char *)&myPs->rcvCmd, 0,
					sizeof(S_ANALOG_METER_RCV_COMMAND));
			}
		}
	} else {
		userlog(DEBUG_LOG, psName, "Open_Close error %d : 0x%x\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], (char)myPs->rcvCmd.cmd[1]);
		return -1;
	}

	return 0;
}

int rcv_cmd_answer2(void)
{
	char buf[12];
	int i, index=0, temp_bd, temp_ch, ch;
	long temp;
	S_ANALOG_METER_RCV_CMD_ANSWER2	answer;
	
//	userlog(DEBUG_LOG, psName, "test1 %d %d\n",
//		sizeof(S_ANALOG_METER_RCV_CMD_ANSWER2), myPs->rcvCmd.cmdSize);
	memset((char *)&answer, 0, sizeof(S_ANALOG_METER_RCV_CMD_ANSWER2));
	memcpy((char *)&answer, (char *)&myPs->rcvCmd.cmd, myPs->rcvCmd.cmdSize);
	
	if(answer.Status == 'N') {
		memset(buf, 0, sizeof(buf));
		memcpy((char *)&buf, (char *)&answer.Sign, 10);
		temp = atoi(buf) * 100;
	} else if(answer.Status == 'O') {
		temp = 9999000;
	} else if((answer.Status == 'S') || (answer.Status == 'E')) {
		temp = -9999000;
	} else temp = -9999000;

	if((answer.CH[0] >= '0') && (answer.CH[0] <= '9')) {
		if((answer.CH[1] >= '0') && (answer.CH[1] <= '9')) {
		} else temp = -9999000;
	} else temp = -9999000;

	if(temp != (-9999000)) {
		memset(buf, 0, sizeof(buf));
		memcpy((char *)&buf, (char *)&answer.CH[0], 2);
		temp_ch = atoi(buf) - 1;
		if((temp_ch < 0) || (temp_ch > 15)) temp_ch = (-1);
	} else temp_ch = (-1);

//	userlog(DEBUG_LOG, psName, "test2 %ld %d\n", temp, temp_ch);
	temp_bd = ((int)myPs->signal[ANALOG_METER_SIG_MEASURE] % 10) - 1;
	if(temp_bd < 0) {
		userlog(DEBUG_LOG, psName, "AnalogValue sig:%d, ch:%d val:%f\n",
			myPs->signal[ANALOG_METER_SIG_MEASURE], temp_ch,
			(float)temp / 1000.0);
		return 0;
	}

	if(temp_bd != ((int)myPs->signal[ANALOG_METER_SIG_COMM_BUS_ENABLE] - 1)) {
		userlog(DEBUG_LOG, psName, "AnalogValue bd_num %d %d\n",
			myPs->signal[ANALOG_METER_SIG_COMM_BUS_ENABLE]-1, temp_bd);
		return 0;
	}

	if((temp_ch >= 0) && (temp != (-9999000))) {
//		userlog(DEBUG_LOG, psName, "test3 %d %d\n", temp_bd, temp_ch);
		i = temp_bd * 16 + temp_ch; //temp_hw_no index
		index = (int)myPs->Array2[i][0] - 1; //temp_monitor_no index
		if(index >= 0) {
			//ch = myPs->Array1[index][1] - 1;
			ch = index;
			myPs->tmp_value[ch] = temp + myPs->config.measure_offset[temp_bd];
			/*if(ch == 1) myPs->tmp_value[ch] = 21200;
			else if(ch == 15) myPs->tmp_value[ch] = 36300;
			else if(ch == 17) myPs->tmp_value[ch] = 20500;
			else if(ch == 30) myPs->tmp_value[ch] = 23900;
			else myPs->tmp_value[ch] = 9999000;*/
			myPs->value[ch] = myPs->tmp_value[ch];
		}
	}

	if(temp_ch != 15) return 0;

	memset((char *)&myPs->rcvPacket, 0, sizeof(S_ANALOG_METER_RCV_PACKET));
	memset((char *)&myPs->rcvCmd, 0, sizeof(S_ANALOG_METER_RCV_COMMAND));

//	userlog(DEBUG_LOG, psName, "test4 %d %d\n",
//		myPs->signal[ANALOG_METER_SIG_MEASURE],
//		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR]);
	if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P31) {
		if(myPs->config.countMeter > 1) {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P41;
		} else {
			myPs->signal[ANALOG_METER_SIG_MEASURE] = P21;
		}
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P32) {
		myPs->signal[ANALOG_METER_SIG_MEASURE] += 10;
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P33) {
		myPs->signal[ANALOG_METER_SIG_MEASURE] += 10;
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	} else if(myPs->signal[ANALOG_METER_SIG_MEASURE] == P34) {
		myPs->signal[ANALOG_METER_SIG_MEASURE] += 10;
		myPs->signal[ANALOG_METER_SIG_MEASURE_ERROR] = 0;
	}

	return 0;
}

void send_cmd_initialize(int type, int addr, int val)
{
	int rtn, cmd_size;
	S_ANALOG_METER_SEND_CMD_INITIALIZE	cmd;
	
	memset((char *)&cmd, 0, sizeof(S_ANALOG_METER_SEND_CMD_INITIALIZE));

	switch(type) {
		case 1: //set module configuration
			cmd_size = 12;
			if(addr == 1) {
				if(val == 1) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%01010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0101050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0101060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%01020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0102050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0102060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%01030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0103050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0103060600\r", cmd_size);
					}
				} else if(val == 4) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%01040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0104050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0104060600\r", cmd_size);
					}
				}
			} else if(addr == 2) {
				if(val == 1) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%02010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0201050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0201060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%02020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0202050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0202060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%02030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0203050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0203060600\r", cmd_size);
					}
				} else if(val == 4) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%02040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0204050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0204060600\r", cmd_size);
					}
				}
			} else if(addr == 3) {
				if(val == 1) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%03010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0301050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0301060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%03020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0302050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0302060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%03030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0303050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0303060600\r", cmd_size);
					}
				} else if(val == 4) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%03040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0304050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0304060600\r", cmd_size);
					}
				}
			} else if(addr == 4) {
				if(val == 1) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%04010F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0401050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0401060600\r", cmd_size);
					}
				} else if(val == 2) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%04020F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0402050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0402060600\r", cmd_size);
					}
				} else if(val == 3) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%04030F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0403050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0403060600\r", cmd_size);
					}
				} else if(val == 4) {
					if(myPs->config.readType == READ_T) {
						memcpy((char *)&cmd.data, "%04040F0600\r", cmd_size);
					} else if(myPs->config.readType == READ_V) {
						memcpy((char *)&cmd.data, "%0404050600\r", cmd_size);
					} else { //READ_I
						memcpy((char *)&cmd.data, "%0404060600\r", cmd_size);
					}
				}
			}
			break;
		case 2: //set channel enable
			cmd_size = 7;
			if(val == 1) {
				memcpy((char *)&cmd.data, "$015FF\r", cmd_size);
			} else if(val == 2) {
				memcpy((char *)&cmd.data, "$025FF\r", cmd_size);
			} else if(val == 3) {
				memcpy((char *)&cmd.data, "$035FF\r", cmd_size);
			} else if(val == 4) {
				memcpy((char *)&cmd.data, "$045FF\r", cmd_size);
			}
			break;
		default:
			userlog(DEBUG_LOG, psName, "Unknown cmd\n");
			return;
	}
	
	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void send_cmd_request(int type, int bd)
{
	if(myPs->config.functionType == 0) {
		send_cmd_request1(type, bd);
	} else {
		send_cmd_request2(type, bd);
	}
}

void send_cmd_request1(int type, int bd)
{
	char buf[12];
	int rtn, cmd_size;
	S_ANALOG_METER_SEND_CMD_REQUEST	cmd;
	
	memset((char *)&cmd, 0, sizeof(S_ANALOG_METER_SEND_CMD_REQUEST));

	switch(type) {
		case 0: //Read Analog Input
			cmd_size = 4;
			if(bd == 1) {
				memcpy((char *)&cmd.data, "#01\r", cmd_size);
			} else if(bd == 2) {
				memcpy((char *)&cmd.data, "#02\r", cmd_size);
			} else if(bd == 3) {
				memcpy((char *)&cmd.data, "#03\r", cmd_size);
			} else if(bd == 4) {
				memcpy((char *)&cmd.data, "#04\r", cmd_size);
			} else {
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "#%02d\r", bd);
				memcpy((char *)&cmd.data, (char *)&buf, cmd_size);
			}
			break;
		case 1: //Read CJC Temperature
			cmd_size = 5;
			if(bd == 1) {
				memcpy((char *)&cmd.data, "$013\r", cmd_size);
			} else if(bd == 2) {
				memcpy((char *)&cmd.data, "$023\r", cmd_size);
			} else if(bd == 3) {
				memcpy((char *)&cmd.data, "$033\r", cmd_size);
			} else if(bd == 4) {
				memcpy((char *)&cmd.data, "$043\r", cmd_size);
			}
			break;
		default:
			userlog(DEBUG_LOG, psName, "Unknown cmd\n");
			return;
	}
	
	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void send_cmd_request2(int type, int bd)
{
	char buf[12];
	int rtn, cmd_size;
	S_ANALOG_METER_SEND_CMD_REQUEST	cmd;
	
	memset((char *)&myPs->rcvPacket, 0, sizeof(S_ANALOG_METER_RCV_PACKET));
	memset((char *)&myPs->rcvCmd, 0, sizeof(S_ANALOG_METER_RCV_COMMAND));

	memset((char *)&cmd, 0, sizeof(S_ANALOG_METER_SEND_CMD_REQUEST));

	switch(type) {
		case 0:
			cmd_size = 12;
			if(bd == 1) {
				memcpy((char *)&cmd.data, "FD 0,01,16\r\n", cmd_size);
			} else if(bd == 2) {
				memcpy((char *)&cmd.data, "FD 0,01,16\r\n", cmd_size);
			} else if(bd == 3) {
				memcpy((char *)&cmd.data, "FD 0,01,16\r\n", cmd_size);
			} else if(bd == 4) {
				memcpy((char *)&cmd.data, "FD 0,01,16\r\n", cmd_size);
			} else {
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "#%02d\r", bd);
				memcpy((char *)&cmd.data, (char *)&buf, cmd_size);
			}
			break;
		case 1:
			cmd_size = 5;
			if(bd == 1) {
				memcpy((char *)&cmd.data, "$013\r", cmd_size);
			} else if(bd == 2) {
				memcpy((char *)&cmd.data, "$023\r", cmd_size);
			} else if(bd == 3) {
				memcpy((char *)&cmd.data, "$033\r", cmd_size);
			} else if(bd == 4) {
				memcpy((char *)&cmd.data, "$043\r", cmd_size);
			}
			break;
		default:
			userlog(DEBUG_LOG, psName, "Unknown cmd\n");
			return;
	}
	
	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void send_cmd_open(int bd)
{
	char buf[12], tmp[4];
	int rtn, cmd_size;
	
	cmd_size = 7;
	memset(buf, 0, sizeof(buf));
	buf[0] = 0x1B; //ESC
	buf[1] = 'O';
	buf[2] = ' ';

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%02d", bd);
	memcpy((char *)&buf[3], (char *)&tmp, 2);

	buf[5] = 0x0D; //CR
	buf[6] = 0x0A; //\n
	
	rtn = send_command((char *)&buf, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}

	//usleep(250000);

	memset((char *)&myPs->rcvPacket, 0, sizeof(S_ANALOG_METER_RCV_PACKET));
	memset((char *)&myPs->rcvCmd, 0, sizeof(S_ANALOG_METER_RCV_COMMAND));
}

void send_cmd_close(int bd)
{
	char buf[12], tmp[4];
	int rtn, cmd_size;

	cmd_size = 7;
	memset(buf, 0, sizeof(buf));
	buf[0] = 0x1B; //ESC
	buf[1] = 'C';
	buf[2] = ' ';

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%02d", bd);
	memcpy((char *)&buf[3], (char *)&tmp, 2);

	buf[5] = 0x0D; //CR
	buf[6] = 0x0A; //\n
	
	rtn = send_command((char *)&buf, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}

	//usleep(250000);
	
	memset((char *)&myPs->rcvPacket, 0, sizeof(S_ANALOG_METER_RCV_PACKET));
	memset((char *)&myPs->rcvCmd, 0, sizeof(S_ANALOG_METER_RCV_COMMAND));
}

void make_header(char *cmd, char reply, int cmd_id, int seqno, int body_size)
{
	S_ANALOG_METER_CMD_HEADER	header;
	
	memset((char *)&header, 0, sizeof(S_ANALOG_METER_CMD_HEADER));
	
	memcpy(cmd, (char *)&header, sizeof(S_ANALOG_METER_CMD_HEADER));
	
/*	userlog(METER2_LOG, psName, "header");
	for(i=0; i < sizeof(S_ANALOG_METER_CMD_HEADER); i++) {
		userlog2(METER2_LOG, psName, " %x", *(cmd + i));
	}
	userlog2(METER2_LOG, psName, ":end\n");
	userlog(METER2_LOG, psName, "header %s:end\n", cmd); //kjgd*/
}

void make_check_sum(char *cmd, int size)
{
	int i;
	char check_sum = 0;
	
	for(i=1; i < (size - 2); i++) {
		check_sum ^= *(cmd + i);
	}
	
	*(cmd + size - 1) = check_sum;
}

int	send_command(char *cmd, int size, int cmd_id)
{
	unsigned char tmp, log_flag;
	char	packet[MAX_SERIAL_PACKET_LENGTH];
	int i;

	if(size > MAX_SERIAL_PACKET_LENGTH) {
		userlog(DEBUG_LOG, psName, "CMD SEND FAIL!! TOO LARGE SIZE:%d\n", size);
		return -1;
	}
	
	memset((char *)&packet, 0, MAX_SERIAL_PACKET_LENGTH);
	memcpy((char *)&packet, cmd, size);

	if(myPs->config.CmdSendLog == P1) {
		log_flag = 0;
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			log_flag = 1;
		}

		if(log_flag == 1) {
			userlog(METER2_LOG, psName, "sendCmd %s:end\n", packet);
		}
	}
	
	if(myPs->config.CmdSendLog_Hex == P1) {
		log_flag = 0;
		if(myPs->config.CommCheckLog == P1) {
			log_flag = 1;
		} else {
			log_flag = 1;
		}

		if(log_flag == 1) {
			userlog(METER2_LOG, psName, "sendCmd");
			for(i=0; i < size; i++) {
				tmp = *(cmd + i);
				userlog2(METER2_LOG, psName, " %02x", tmp);
			}
			userlog2(METER2_LOG, psName, ":end\n");
		}
	}

	i = writen(myPs->misc.ttyS_fd, packet, size);
	tcdrain(myPs->misc.ttyS_fd);
	usleep(10000);
	return i;
}

int Check_ComPortState(void)
{//kjgw
	return 0;
}
