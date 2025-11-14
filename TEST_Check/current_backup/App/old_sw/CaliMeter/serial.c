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
#include "common_utils.h"
#include "local_utils.h"
#include "local_message.h"
#include "com_io.h"
#include "comm.h"
#include "serial.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_CALI_METER  *myPs;
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
	} else { //Display Board
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
		case 57600: comm_param.bps = TMT_B576;	break;
		case 115200: comm_param.bps = TMT_B1152;	break;
	}
	//comm_param.flow = TMT_FNONE;
	comm_param.flow = TMT_FXONXOFF;
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
		memset((char *)&myPs->rcvPacket, 0, sizeof(S_CALI_METER_RCV_PACKET));
		memset((char *)&myPs->rcvCmd, 0, sizeof(S_CALI_METER_RCV_COMMAND));
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
	//char debug[MAX_SERIAL_PACKET_LENGTH]; //kjgd
	int i, j, k, cmdBuf_index, start_point;
	
	if(myPs->rcvPacket.rcvCount == myPs->rcvPacket.parseCount) return;
	
	i = myPs->rcvPacket.parseCount;
	myPs->rcvPacket.parseCount++;
	if(myPs->rcvPacket.parseCount > (MAX_SERIAL_PACKET_COUNT-1))
		myPs->rcvPacket.parseCount = 0;
	
	cmdBuf_index = myPs->rcvCmd.cmdBufSize;
	myPs->rcvCmd.cmdBufSize += myPs->rcvPacket.rcvSize[i];
	
	start_point = myPs->rcvPacket.parseStartPoint[i];

/*	userlog(METER_LOG, psName, "recvCmd1 %s:end %d %d\n",
		myPs->recvCmd.cmdBuf, i, start_point);

	memset(debug, 0, sizeof debug);
	memcpy((char *)&debug, (char *)&myPs->recvCmd.cmdBuf[cmdBuf_index],
		myPs->rcvPacket.rcvSize[i]);
	userlog(METER_LOG, psName, "recvCmd2 %s:end %d %d\n",
		debug, cmdBuf_index, myPs->rcvPacket.rcvSize[i]); //kjgd*/
	
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

/*	userlog(METER_LOG, psName, "recvCmd3 %d %d\n", myPs->rcvPacket.usedBufSize,
		myPs->rcvCmd.cmdBufSize); //kjgd*/
}

int SerialCommand_Receive(void)
{
	//char debug[MAX_SERIAL_PACKET_LENGTH]; //kjgd
	int i, cmd_size=0, cmdBuf_index;

	if(myPs->rcvCmd.cmdBufSize < 1) return -1;
	
	if(myPs->config.functionType == 0) {
		if(myPs->rcvCmd.cmdBuf[0] == '+' || myPs->rcvCmd.cmdBuf[0] == '-') {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\n') {
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -2;
		} else {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == '\n') {
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
		}
	} else { //Display Board
		if(myPs->rcvCmd.cmdBuf[0] == 0x02) {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == 0x03) {
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -2;
		} else {
			for(i=0; i < myPs->rcvCmd.cmdBufSize; i++) {
				if(myPs->rcvCmd.cmdBuf[i] == 0x03) {
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
		}
	}

/*	userlog(METER_LOG, psName, "recvCmd4 %s:end %d\n",
		myPs->rcvCmd.cmdBuf, cmd_size); //kjgd*/
			
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
	memcpy((char *)&debug, (char *)&myPs->rcvCmd.cmdBuf, cmd_size);
	userlog(METER_LOG, psName, "recvCmd5 %s:end %d %d\n",
		debug, cmd_size, cmdBuf_index); //kjgd
*/	
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
			userlog(METER_LOG, psName, "recvCmd %s:end\n", myPs->rcvCmd.cmd);
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
			userlog(METER_LOG, psName, "recvCmd");
			for(i=0; i < myPs->rcvCmd.cmdSize; i++) {
				tmp = myPs->rcvCmd.cmd[i];
				userlog2(METER_LOG, psName, " %02x", tmp);
			}
			userlog2(METER_LOG, psName, ":end\n");
		}
	}

	if(myPs->config.functionType == 0) { //Agilent 34401A
		rtn = CmdHeader_Check1();
		if(rtn < 0) return -1;
	
		rtn = rcv_cmd_answer1();
	} else { //Display Board
		rtn = CmdHeader_Check2();
		if(rtn < 0) return -1;
	
		rtn = rcv_cmd_answer2();
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
	char buf[12];
	int	length;
	long tmp;
	S_CALI_METER_CMD_HEADER	header;

	length = sizeof(S_CALI_METER_CMD_HEADER);
	memcpy((char *)&header, (char *)myPs->rcvCmd.cmd, length);

	if(length != myPs->rcvCmd.cmdSize) {
		userlog(DEBUG_LOG, psName, "RcvCmd size error (%d:%d)\n",
			length, myPs->rcvCmd.cmdSize);
		return -1;
	}
	
	if(header.sign1 != ' ' && header.sign1 != '+' && header.sign1 != '-') {
		userlog(DEBUG_LOG, psName, "RcvCmd sign1 error : 0x%x\n", header.sign1);
		return -2;
	}
	
	if(header.digit1 < '0' || header.digit1 > '9') {
		userlog(DEBUG_LOG, psName, "RcvCmd digit1 error : 0x%x\n",
			header.digit1);
		return -3;
	}

	if(header.dot != '.') {
		userlog(DEBUG_LOG, psName, "RcvCmd dot error : 0x%x\n", header.dot);
		return -4;
	}

	memset(buf, 0, sizeof buf);
	memcpy((char *)&buf, (char *)&header.digit2, 8);
	tmp = atol(buf);
	if(tmp < 0 || tmp > 99999999) {
		userlog(DEBUG_LOG, psName, "RcvCmd digit2 error : %ld\n", tmp);
		return -5;
	}

	if(header.exponent != 'E' && header.exponent != 'e') {
		userlog(DEBUG_LOG, psName, "RcvCmd exponent error : 0x%x\n",
			header.exponent);
		return -6;
	}

	if(header.sign2 != ' ' && header.sign2 != '+' && header.sign2 != '-') {
		userlog(DEBUG_LOG, psName, "RcvCmd sign2 error : 0x%x\n", header.sign2);
		return -7;
	}
	
	memset(buf, 0, sizeof buf);
	memcpy((char *)&buf, (char *)&header.digit3, 2);
	tmp = atol(buf);
	if(tmp < 0 || tmp > 99) {
		userlog(DEBUG_LOG, psName, "RcvCmd digit3 error : %ld\n", tmp);
		return -8;
	}

	if(header.cr != 0x0D) {
		userlog(DEBUG_LOG, psName, "RcvCmd cr error : 0x%x\n", header.cr);
		return -9;
	}

	if(header.nl != '\n') {
		userlog(DEBUG_LOG, psName, "RcvCmd nl error : 0x%x\n", header.nl);
		return -10;
	}

	return 0;
}

int CmdHeader_Check2(void)
{
	int	length;
	S_CALI_METER_CMD_HEADER2	header;

	length = 11;
	memcpy((char *)&header, (char *)myPs->rcvCmd.cmd,
		sizeof(S_CALI_METER_CMD_HEADER2));

	if(length != myPs->rcvCmd.cmdSize) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 size error (%d:%d)\n",
			length, myPs->rcvCmd.cmdSize);
		return -1;
	}
	
	if((header.addr[0] >= 0x30 && header.addr[0] <= 0x39)
		|| (header.addr[0] >= 0x41 && header.addr[0] <= 0x46)) {
	} else {
		userlog(DEBUG_LOG, psName, "RcvCmd2 addr[0] error : 0x%x\n",
			header.addr[0]);
		return -2;
	}
	
	if((header.addr[1] >= 0x30 && header.addr[1] <= 0x39)
		|| (header.addr[1] >= 0x41 && header.addr[1] <= 0x46)) {
	} else {
		userlog(DEBUG_LOG, psName, "RcvCmd2 addr[1] error : 0x%x\n",
			header.addr[1]);
		return -3;
	}
	
	if((header.cmd[0] >= 0x30 && header.cmd[0] <= 0x39)
		|| (header.cmd[0] >= 0x41 && header.cmd[0] <= 0x46)) {
	} else {
		userlog(DEBUG_LOG, psName, "RcvCmd2 cmd[0] error : 0x%x\n",
			header.cmd[0]);
		return -4;
	}
	
	if((header.cmd[1] >= 0x30 && header.cmd[1] <= 0x39)
		|| (header.cmd[1] >= 0x41 && header.cmd[1] <= 0x46)) {
	} else {
		userlog(DEBUG_LOG, psName, "RcvCmd2 cmd[1] error : 0x%x\n",
			header.cmd[1]);
		return -5;
	}
	
	return 0;
}

int rcv_cmd_answer1(void)
{
	char buf[16];
	double val;
	S_CALI_METER_RCV_CMD_ANSWER	answer;
	
	memset((char *)&answer, 0, sizeof(S_CALI_METER_RCV_CMD_ANSWER));
	memcpy((char *)&answer, (char *)&myPs->rcvCmd.cmd,
		sizeof(S_CALI_METER_RCV_CMD_ANSWER));
	
	memset(buf, 0, sizeof buf);
	memcpy((char *)&buf, (char *)&answer.header.sign1, 15);
	val = strtod(buf, (char **)NULL);
	myPs->value = (long)(val * 1000000.0);
	userlog(DEBUG_LOG, psName, "RcvCmd value : %f(mV/mA)\n", val*1000.0);

	if(myPs->signal[CALI_METER_SIG_REQUEST_PHASE] == P2) {
		myPs->signal[CALI_METER_SIG_REQUEST_PHASE] = P3;
	}

	return 0;
}

int rcv_cmd_answer2(void)
{
	int i, checksum, cmp_checksum, checksum_type, tmp;
	S_CALI_METER_RCV_CMD_ANSWER2	recvCmd;
	
	memset((char *)&recvCmd, 0, sizeof(S_CALI_METER_RCV_CMD_ANSWER2));
	memcpy((char *)&recvCmd, (char *)&myPs->rcvCmd.cmd, myPs->rcvCmd.cmdSize);
	
	checksum = 0;
	checksum_type = 1; //0:dec, 1:hex
	for(i=1; i < 6; i++) {
		checksum += (int)recvCmd.data[i];
	}

	if(checksum_type == 0) { //dec
		cmp_checksum = ((int)recvCmd.data[6] << 12);
		cmp_checksum |= ((int)recvCmd.data[7] << 8);
		cmp_checksum |= ((int)recvCmd.data[8] << 4);
		cmp_checksum |= (int)recvCmd.data[9];
	} else { //hex
		tmp = (int)recvCmd.data[6];
		if(tmp <= 0x39) tmp -= 0x30;
		else tmp -= 0x37;
		cmp_checksum = (tmp << 12);
		tmp = (int)recvCmd.data[7];
		if(tmp <= 0x39) tmp -= 0x30;
		else tmp -= 0x37;
		cmp_checksum |= (tmp << 8);
		tmp = (int)recvCmd.data[8];
		if(tmp <= 0x39) tmp -= 0x30;
		else tmp -= 0x37;
		cmp_checksum |= (tmp << 4);
		tmp = (int)recvCmd.data[9];
		if(tmp <= 0x39) tmp -= 0x30;
		else tmp -= 0x37;
		cmp_checksum |= tmp;
	}

	if(checksum != cmp_checksum) {
		userlog(DEBUG_LOG, psName,
			"RcvCmd2 checksum error : %02x %02x %02x %02x : %x : %x\n",
			recvCmd.data[6], recvCmd.data[7], recvCmd.data[8], recvCmd.data[9],
			checksum, cmp_checksum);
	}

	if(recvCmd.data[5] != 0x06) {
		userlog(DEBUG_LOG, psName, "RcvCmd2 nack %02x\n", recvCmd.data[5]);
	} else {
/*		printf("kjgd %d (%x %x), (%x %x)\n",
			myPs->signal[CALI_METER_SIG_DISPLAY],
			recvCmd.data[1], recvCmd.data[2],
			recvCmd.data[3], recvCmd.data[4]);*/
		switch(myPs->signal[CALI_METER_SIG_DISPLAY]) {
			case P11:
				/*if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x31) {
					//myPs->signal[CALI_METER_SIG_DISPLAY] -= P9;
					myPs->signal[CALI_METER_SIG_DISPLAY] = P2;
					//myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
				}*/
				if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x32) {
					//myPs->signal[CALI_METER_SIG_DISPLAY] -= P9;
					myPs->signal[CALI_METER_SIG_DISPLAY] = P2;
					//myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
				}
				break;
			case P12:
				/*if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x30) {
					//myPs->signal[CALI_METER_SIG_DISPLAY] -= P9;
					myPs->signal[CALI_METER_SIG_DISPLAY] = P3;
					//myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
				}*/
				if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x33) {
					//myPs->signal[CALI_METER_SIG_DISPLAY] -= P9;
					myPs->signal[CALI_METER_SIG_DISPLAY] = P3;
					//myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
				}
				break;
			case P13:
				if(myPs->config.functionType == 1) {
					/*if(recvCmd.data[3] == 0x31 && recvCmd.data[4] == 0x31) {
						//myPs->signal[CALI_METER_SIG_DISPLAY] -= P9;
						myPs->signal[CALI_METER_SIG_DISPLAY] = P4;
						//myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
					}*/
					if(recvCmd.data[3] == 0x31 && recvCmd.data[4] == 0x32) {
						//myPs->signal[CALI_METER_SIG_DISPLAY] -= P9;
						myPs->signal[CALI_METER_SIG_DISPLAY] = P4;
						//myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
					}
				} else {
					/*if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x31) {
						//myPs->signal[CALI_METER_SIG_DISPLAY] -= P9;
						myPs->signal[CALI_METER_SIG_DISPLAY] = P4;
						//myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
					}*/
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x32) {
						//myPs->signal[CALI_METER_SIG_DISPLAY] -= P9;
						myPs->signal[CALI_METER_SIG_DISPLAY] = P4;
						//myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
					}
				}
				break;
			case P14:
				if(myPs->config.functionType == 1) {
					/*if(recvCmd.data[3] == 0x31 && recvCmd.data[4] == 0x30) {
						myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
					}*/
					if(recvCmd.data[3] == 0x31 && recvCmd.data[4] == 0x33) {
						myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
					}
				} else {
					/*if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x30) {
						myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
					}*/
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x33) {
						myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
					}
				}
				break;
			default:
				break;
		}
	}

	return 0;
}

void send_cmd_initialize(int type)
{
	int rtn, cmd_size;
	S_CALI_METER_SEND_CMD_INITIALIZE	cmd;
	
	memset((char *)&cmd, 0, sizeof(S_CALI_METER_SEND_CMD_INITIALIZE));

	switch(type) {
		case 1:
			cmd_size = 5;
			memcpy((char *)&cmd.data, "*RST\n", cmd_size);
			break;
		case 2:
			cmd_size = 9;
			memcpy((char *)&cmd.data, "SYST:REM\n", cmd_size);
			break;
		case 3:
			cmd_size = 22;
			memcpy((char *)&cmd.data, "CONF:VOLT:DC DEF, DEF\n", cmd_size);
			break;
		case 4:
			cmd_size = 15;
			memcpy((char *)&cmd.data, "SAMPLE:COUNT 1\n", cmd_size);
			break;
		case 5:
			cmd_size = 22;
			memcpy((char *)&cmd.data, "CONF:CURR:DC DEF, DEF\n", cmd_size);
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

void send_cmd_request(void)
{
	int rtn, cmd_size;
	S_CALI_METER_SEND_CMD_REQUEST	cmd;
	
	memset((char *)&cmd, 0, sizeof(S_CALI_METER_SEND_CMD_REQUEST));

	cmd_size = 6;
	memcpy((char *)&cmd.data, "READ?\n", cmd_size);
	
	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void send_cmd_display_value(int addr, int type, int test)
{
	unsigned char flag[6], val3;
	int rtn, cmd_size, i, checksum, tmp, checksum_type;
	long val, val2;
	S_CALI_METER_SEND_CMD_DISPLAY_VALUE	cmd;
	
	memset((char *)&cmd, 0, sizeof(S_CALI_METER_SEND_CMD_DISPLAY_VALUE));

	if((type % 10) == 0) {
		cmd_size = 12; //LED
	} else {
		cmd_size = 16; //V, I, P
	}

	cmd.data[0] = 0x02; //STX
	cmd.data[1] = '0'; //addr1
	cmd.data[2] = (char)(0x30 + addr);
	switch(type % 10) {
		case 0: //LED
			if(type < 10) {
				cmd.data[3] = '0';
				cmd.data[4] = '0';
			} else {
				cmd.data[3] = '1';
				cmd.data[4] = '0';
			}

			val2 = 0;
			if(test == 1) {
				val = 0xFF;
				break;
			}

			if(myPs->config.functionType == 1 && type < 10) {
				val = 0;
				flag[0] = Read_InPoint(0, I_FUSE_FAIL);
				flag[1] = Read_InPoint(0, I_INVERTER_HV_FAULT);
				flag[2] = Read_InPoint(0, I_INVERTER_MODULE_R_FAIL);
				flag[3] = Read_InPoint(0, I_INVERTER_MODULE_S_FAIL);
				flag[4] = Read_InPoint(0, I_INVERTER_MODULE_T_FAIL);
				flag[5] = Read_InPoint(0, I_INVERTER_OT);
				val3 = 0;
				for(i=0; i < 6; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x02;
				}

				if(myData->dio.signal[DIO_SIG_REMOTE_PS] == P100) {
					val |= 0x01;
				}

				flag[0] = Read_InPoint(1, I_CONVERTER_OV);
				flag[1] = Read_InPoint(1, I_CONVERTER_OC);
				flag[2] = Read_InPoint(1, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(1, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(1, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
					val |= 0x40;
				}

				if(myData->cData[0].op.state == C_RUN) {
					val |= 0x08;
					if(myData->cData[0].op.stepType == STEP_CHARGE) {
						val |= 0x10;
					} else if((myData->cData[0].op.stepType == STEP_DISCHARGE)
						|| (myData->cData[0].op.stepType == STEP_Z)) {
						val |= 0x20;
					} else if(myData->cData[0].op.stepType == STEP_PATTERN) {
						if(myData->cData[0].op.Isens >= 0) {
							val |= 0x10;
						} else {
							val |= 0x20;
						}
					}
				}

				val |= 0x04; //power led
			} else if(myPs->config.functionType == 1 && type >= 10) {
				val = 0;

				flag[0] = Read_InPoint(2, I_CONVERTER_OV);
				flag[1] = Read_InPoint(2, I_CONVERTER_OC);
				flag[2] = Read_InPoint(2, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(2, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(2, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				if(myData->ChAttribute[1].opType != OP_INDEPENDENT) {
					val |= 0x40;
				}

				if(myData->cData[1].op.state == C_RUN) {
					val |= 0x08;
					if(myData->cData[1].op.stepType == STEP_CHARGE) {
						val |= 0x10;
					} else if((myData->cData[1].op.stepType == STEP_DISCHARGE)
						|| (myData->cData[1].op.stepType == STEP_Z)) {
						val |= 0x20;
					} else if(myData->cData[1].op.stepType == STEP_PATTERN) {
						if(myData->cData[1].op.Isens >= 0) {
							val |= 0x10;
						} else {
							val |= 0x20;
						}
					}
				}

				val |= 0x04; //power led
			} else if(myPs->config.functionType == 2 && addr == 0) {
				val = 0;
				flag[0] = Read_InPoint(1, I_FUSE_FAIL);
				flag[1] = Read_InPoint(1, I_INVERTER_HV_FAULT);
				flag[2] = Read_InPoint(1, I_INVERTER_MODULE_R_FAIL);
				flag[3] = Read_InPoint(1, I_INVERTER_MODULE_S_FAIL);
				flag[4] = Read_InPoint(1, I_INVERTER_MODULE_T_FAIL);
				flag[5] = Read_InPoint(1, I_INVERTER_OT);
				val3 = 0;
				for(i=0; i < 6; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x02;
				}

				if(myData->dio.signal[DIO_SIG_REMOTE_PS] == P100) {
					val |= 0x01;
				}

				flag[0] = Read_InPoint(1, I_CONVERTER_OV);
				flag[1] = Read_InPoint(1, I_CONVERTER_OC);
				flag[2] = Read_InPoint(1, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(1, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(1, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
					val |= 0x40;
				}

				if(myData->cData[0].op.state == C_RUN) {
					val |= 0x08;
					if(myData->cData[0].op.stepType == STEP_CHARGE) {
						val |= 0x10;
					} else if((myData->cData[0].op.stepType == STEP_DISCHARGE)
						|| (myData->cData[0].op.stepType == STEP_Z)) {
						val |= 0x20;
					} else if(myData->cData[0].op.stepType == STEP_PATTERN) {
						if(myData->cData[0].op.Isens >= 0) {
							val |= 0x10;
						} else {
							val |= 0x20;
						}
					}
				}

				val |= 0x04; //power led
			//} else if(myPs->config.functionType == 2 && addr == 1) {
			} else {
				val = 0;
				flag[0] = Read_InPoint(2, I_FUSE_FAIL);
				flag[1] = Read_InPoint(2, I_INVERTER_HV_FAULT);
				flag[2] = Read_InPoint(2, I_INVERTER_MODULE_R_FAIL);
				flag[3] = Read_InPoint(2, I_INVERTER_MODULE_S_FAIL);
				flag[4] = Read_InPoint(2, I_INVERTER_MODULE_T_FAIL);
				flag[5] = Read_InPoint(2, I_INVERTER_OT);
				val3 = 0;
				for(i=0; i < 6; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x02;
				}

				if(myData->dio.signal[DIO_SIG_REMOTE_PS] == P100) {
					val |= 0x01;
				}


				flag[0] = Read_InPoint(2, I_CONVERTER_OV);
				flag[1] = Read_InPoint(2, I_CONVERTER_OC);
				flag[2] = Read_InPoint(2, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(2, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(2, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
					val |= 0x40;
				}

				if(myData->cData[1].op.state == C_RUN) {
					val |= 0x08;
					if(myData->cData[1].op.stepType == STEP_CHARGE) {
						val |= 0x10;
					} else if((myData->cData[1].op.stepType == STEP_DISCHARGE)
						|| (myData->cData[1].op.stepType == STEP_Z)) {
						val |= 0x20;
					} else if(myData->cData[1].op.stepType == STEP_PATTERN) {
						if(myData->cData[1].op.Isens >= 0) {
							val |= 0x10;
						} else {
							val |= 0x20;
						}
					}
				}

				val |= 0x04; //power led
			}
			break;
		case 1: //V
			if(type < 10) {
				cmd.data[3] = '0';
				cmd.data[4] = '1';
			} else {
				cmd.data[3] = '1';
				cmd.data[4] = '1';
			}

			val2 = 1;
			if(test == 1) {
				val = -888888888; //-888.888888V
				break;
			}

			if(addr == 0) {
				if(type < 10) {
					val = myData->cData[0].op.Vsens;
				} else {
					val = myData->cData[1].op.Vsens;
				}
			} else {
				val = myData->cData[1].op.Vsens;
			}
			break;
		case 2: //I
			if(type < 10) {
				cmd.data[3] = '0';
				cmd.data[4] = '2';
			} else {
				cmd.data[3] = '1';
				cmd.data[4] = '2';
			}

			val2 = 1;
			if(test == 1) {
				val = 999999999; //999.999999A
				break;
			}

			if(addr == 0) {
				if(type < 10) {
					val = myData->cData[0].op.Isens;
				} else {
					val = myData->cData[1].op.Isens;
				}
			} else {
				val = myData->cData[1].op.Isens;
			}
			//val = myData->mData.misc.timer_1sec * 1000000;
			break;
		case 3: //P
			if(type < 10) {
				cmd.data[3] = '0';
				cmd.data[4] = '3';
			} else {
				cmd.data[3] = '1';
				cmd.data[4] = '3';
			}

			val2 = 1;
			if(test == 1) {
				val = -789789789; //-789.789789kW
				break;
			}

			if(addr == 0) {
				if(type < 10) {
					val = myData->cData[0].op.watt;
				} else {
					val = myData->cData[1].op.watt;
				}
			} else {
				val = myData->cData[1].op.watt;
			}
			//val = myData->mData.misc.timer_1sec * 1000000;
			break;
		default:
			val = 0;
			val2 = -1;
			break;
	}

	if(val2 == 0) { //flag
		val3 = (val & 0xF0) >> 4;
		if(val3 < 10) val3 += 0x30;
		else val3 += 0x37;
		cmd.data[5] = val3;

		val3 = val & 0x0F;
		if(val3 < 10) val3 += 0x30;
		else val3 += 0x37;
		cmd.data[6] = val3;
	} else if(val2 == 1) { //uV, uA, mW
		if(val >= 0) cmd.data[5] = ' ';
		else {
			cmd.data[5] = '-';
			val = val * (-1);
		}
		if(val == 0x80000000) val = 2000000000;
		if(val >= 2000000000) {
			cmd.data[6] = 'F';
			cmd.data[7] = 'F';
			cmd.data[8] = 'F';
			cmd.data[9] = 'F';
			cmd.data[10] = 'F';
		} else if(val >= 1000000000) { //1000.
			cmd.data[6] = val / 1000000000 + 0x30;
			val2 = val % 1000000000;
			cmd.data[7] = val2 / 100000000 + 0x30;
			val2 = val2 % 100000000;
			cmd.data[8] = val2 / 10000000 + 0x30;
			val2 = val2 % 10000000;
			cmd.data[9] = val2 / 1000000 + 0x30;
			val2 = val2 % 1000000;
			if(val2 >= 500000 && cmd.data[9] < 0x39) cmd.data[9] += 1;
			cmd.data[10] = '.';
		} else if(val >= 100000000) { //100.0
			cmd.data[6] = val / 100000000 + 0x30;
			val2 = val % 100000000;
			cmd.data[7] = val2 / 10000000 + 0x30;
			val2 = val2 % 10000000;
			cmd.data[8] = val2 / 1000000 + 0x30;
			val2 = val2 % 1000000;
			cmd.data[9] = '.';
			cmd.data[10] = val2 / 100000 + 0x30;
			val2 = val2 % 100000;
			if(val2 >= 50000 && cmd.data[10] < 0x39) cmd.data[10] += 1;
		} else if(val >= 10000000) { //10.00
			cmd.data[6] = val / 10000000 + 0x30;
			val2 = val % 10000000;
			cmd.data[7] = val2 / 1000000 + 0x30;
			val2 = val2 % 1000000;
			cmd.data[8] = '.';
			cmd.data[9] = val2 / 100000 + 0x30;
			val2 = val2 % 100000;
			cmd.data[10] = val2 / 10000 + 0x30;
			val2 = val2 % 10000;
			if(val2 >= 5000 && cmd.data[10] < 0x39) cmd.data[10] += 1;
		} else if(val >= 1000000) { //1.000
			cmd.data[6] = val / 1000000 + 0x30;
			val2 = val % 1000000;
			cmd.data[7] = '.';
			cmd.data[8] = val2 / 100000 + 0x30;
			val2 = val2 % 100000;
			cmd.data[9] = val2 / 10000 + 0x30;
			val2 = val2 % 10000;
			cmd.data[10] = val2 / 1000 + 0x30;
			val2 = val2 % 1000;
			if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
		} else if(val >= 100000) { //0.100
			cmd.data[6] = 0x30;
			cmd.data[7] = '.';
			cmd.data[8] = val / 100000 + 0x30;
			val2 = val % 100000;
			cmd.data[9] = val2 / 10000 + 0x30;
			val2 = val2 % 10000;
			cmd.data[10] = val2 / 1000 + 0x30;
			val2 = val2 % 1000;
			if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
		} else if(val >= 10000) { //0.010
			cmd.data[6] = 0x30;
			cmd.data[7] = '.';
			cmd.data[8] = 0x30;
			cmd.data[9] = val / 10000 + 0x30;
			val2 = val % 10000;
			cmd.data[10] = val2 / 1000 + 0x30;
			val2 = val2 % 1000;
			if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
		} else { //if(val >= 1000) { //0.001
			cmd.data[6] = 0x30;
			cmd.data[7] = '.';
			cmd.data[8] = 0x30;
			cmd.data[9] = 0x30;
			cmd.data[10] = val / 1000 + 0x30;
			val2 = val % 1000;
			if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
		}
	}

	checksum = 0;
	checksum_type = 1; //0:dec, 1:hex
	for(i=1; i < (cmd_size - 5); i++) {
		checksum += (int)cmd.data[i];
	}

	if(checksum_type == 0) { //dec
		tmp = (checksum / 1000) + 0x30;
		cmd.data[i] = (char)tmp;

		checksum = checksum % 1000;
		tmp = (checksum / 100) + 0x30;
		i++;
		cmd.data[i] = (char)tmp;

		checksum = checksum % 100;
		tmp = (checksum / 10) + 0x30;
		i++;
		cmd.data[i] = (char)tmp;

		tmp = (checksum % 10) + 0x30;
		i++;
		cmd.data[i] = (char)tmp;
	} else { //hex
		tmp = (checksum & 0xF000) >> 12;
		if(tmp < 10) {
			tmp += 0x30;
		} else {
			tmp += 0x37;
		}
		cmd.data[i] = (char)tmp;

		tmp = (checksum & 0x0F00) >> 8;
		if(tmp < 10) {
			tmp += 0x30;
		} else {
			tmp += 0x37;
		}
		i++;
		cmd.data[i] = (char)tmp;

		tmp = (checksum & 0x00F0) >> 4;
		if(tmp < 10) {
			tmp += 0x30;
		} else {
			tmp += 0x37;
		}
		i++;
		cmd.data[i] = (char)tmp;

		tmp = checksum & 0x000F;
		if(tmp < 10) {
			tmp += 0x30;
		} else {
			tmp += 0x37;
		}
		i++;
		cmd.data[i] = (char)tmp;
	}

	i++;
	cmd.data[i] = 0x03; //ETX

	rtn = send_command((char *)&cmd, cmd_size, 0);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void make_header(char *cmd, char reply, int cmd_id, int seqno, int body_size)
{
	S_CALI_METER_CMD_HEADER	header;
	
	memset((char *)&header, 0, sizeof(S_CALI_METER_CMD_HEADER));
	
	memcpy(cmd, (char *)&header, sizeof(S_CALI_METER_CMD_HEADER));
	
/*	userlog(METER_LOG, psName, "header");
	for(i=0; i < sizeof(S_CALI_METER_CMD_HEADER); i++) {
		userlog2(METER_LOG, psName, " %x", *(cmd + i));
	}
	userlog2(METER_LOG, psName, ":end\n");
	userlog(METER_LOG, psName, "header %s:end\n", cmd); //for debug kjg*/
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
			userlog(METER_LOG, psName, "sendCmd %s:end\n", packet);
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
			userlog(METER_LOG, psName, "sendCmd");
			for(i=0; i < size; i++) {
				tmp = *(cmd + i);
				userlog2(METER_LOG, psName, " %02x", tmp);
			}
			userlog2(METER_LOG, psName, ":end\n");
		}
	}

	i = writen(myPs->misc.ttyS_fd, packet, size);
	tcdrain(myPs->misc.ttyS_fd);
	usleep(100000);
	return i;
}

int Check_ComPortState(void)
{//kjgw
	return 0;
}
