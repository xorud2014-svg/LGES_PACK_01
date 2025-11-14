#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/io.h>
#include <math.h>
#include "com_socket.h"
#include "com_io.h"
#include "comm.h"
#include "main.h"

char MeterValue[256];
short int adValue[5];
int state, scanType, auxChNum, I_offset;

extern CommParam comm_param;// = {"/dev/ttyS0", TMT_B96, TMT_FXONXOFF};

int main(int argc, char *argv[])
{
    int	netPort, rtn;

    if(iopl(3)) exit(1);

	outb(0, 0x603);
	outb(0, 0x604);
	outb(0, 0x610);
	outb(0, 0x611);
	sleep(1);

	memset((char *)&comm_param.device[0], 0, COM_DEVICE_SIZE);
	memcpy((char *)&comm_param.device[0], "/dev/ttyS0", 10);
	comm_param.bps = TMT_B96;
	comm_param.flow = TMT_FXONXOFF;

    state = 0; //0:client not connect, 1:client connected
    scanType = VOLTAGE;
	memset(MeterValue, 0, sizeof MeterValue);

    if(argc == 3) {
		netPort = atoi(argv[1]);
		if(netPort == 0) {
			exit(0);
		} else {
			printf("Net Port : %d\n", netPort);
			I_offset = atoi(argv[2]);
			printf("I_offset : %duA\n", I_offset);

			outb(0xFF, 0x603);
			outb(0x08, 0x604);

			rtn = ServerEngine(netPort, DoRead);
		}
    }
    
	outb(0, 0x603);
	outb(0, 0x604);
	outb(0, 0x610);
	outb(0, 0x611);
    exit(0);
}

int DoRead(int fd)
{
    int	rtn, cmd, val, i, j;
	long real;
	double tmp;

	union ADDAVal {
    	short int val;
    	char data[2];
	};
	union ADDAVal myVal;

	typedef struct source_tag {
		int	source_p;
		int	source_0;
		int source_n;
		int data;
	} SOURCE;
	SOURCE V_source;
	SOURCE I_source;

	typedef struct std_value_tag {
		int	average;
		int	std_v;
		int data1;
		int	data2;
	} STD_VALUE;
	STD_VALUE std_value;

    if(iopl(3)) exit(1);

    if(state == 0) {
		printf("New client entered\n");
		rtn = readn(fd, (char *)&cmd, sizeof(int));
		if(rtn < 0) {
		    printf("Client connection fail1\n");
	    	return 0;
		}

		write_ACK(fd);
		printf("Calibration client connected! %d\n", cmd);
		state = 1;
		return 1;
    }

	rtn = ReadCommand(fd, &cmd, &val);
	if(rtn < 0) return 0;

	printf("RcvCmd %d\n", cmd);
	switch(cmd) {
    	case 2:
			//mux
			outb(0, 0x622);
			outb(0x15, 0x621);
			outb(0, 0x620);
			//auxDA
			SetAuxDA(-1, 0x7F);
			//relay
			outb(0, 0x640);
			outb(0, 0x641);
			outb(0, 0x642);
			outb(0, 0x643);
			outb(0, 0x644);
			//mainDA
			outb(0, 0x648); //high
			outb(0, 0x649); //low
			outb(0, 0x64A); //low
			printf("          Initialize1\n");
    		write_ACK(fd);
			break;
	   	case 3:
			printf("          Main DAI : %xh\n", val);
			myVal.val = (short int)val;
			outb(myVal.data[1], 0x648); //high
			outb(myVal.data[0], 0x64A); //low
    		write_ACK(fd);
			break;
	   	case 4:
			printf("          Main DAV : %xh\n", val);
			if(val == 0x1999) val = -6553;
			printf("          Main DAV2 : %xh\n", val);
			myVal.val = (short int)val;
			outb(myVal.data[1], 0x648); //high
			outb(myVal.data[0], 0x649); //low
    		write_ACK(fd);
			break;
    	case 5:
			printf("          Set scan ch:%d\n", val+1); //kjgw
    		write_ACK(fd);
			break;
	    case 6:
			printf("          Relay on ch:%d\n", val+1);
			RelayOnOff(val, 1);
    		write_ACK(fd);
			break;
	    case 7:
			printf("          Relay off ch:%d\n", val+1);
			RelayOnOff(val, 0);
    		write_ACK(fd);
			break;
	    case 8:
			printf("          Scan type : %d\n", val);
			scanType = val;
			if(val == 0) { //voltage
				outb(0x00, 0x610);
				txsettings("*RST\n", 0);
				txsettings("SYST:REM\n", 0);
				txsettings("CONF:VOLT:DC DEF,DEF\n", 0);
				txsettings("SAMPLE:COUNT 1\n", 0);
			} else { //current
				outb(0x0C, 0x610);
				txsettings("*RST\n", 0);
				txsettings("SYST:REM\n", 0);
				txsettings("CONF:CURR:DC DEF,DEF\n", 0);
				txsettings("SAMPLE:COUNT 1\n", 0);
			}
    		write_ACK(fd);
			break;
	    case 9:
			printf("          Set mux ch:%d scanType:%d\n", val+1, scanType);
			SetMux(val, scanType);
    		write_ACK(fd);
			break;
	    case 10:
			printf("          AD value request\n");
    		write_ACK(fd);
			val = 0;
			for(i=0; i < COUNT; i++) {
	    		val += adValue[i];
			}
			tmp = (double)val / (double)COUNT;
			std_value.average = (int)tmp;
			std_value.std_v = cal_std_value(tmp, COUNT);
			std_value.data1 = 0;
			std_value.data2 = 0;
		   	printf("          AD value : (average)%d, (std_v)%d\n",
				std_value.average, std_value.std_v);
			rtn = writen(fd, (char *)&std_value, sizeof(STD_VALUE));
			if(rtn != sizeof(STD_VALUE)) {
				printf("          Send error (AD value)\n");
			}
			/*val = 0;
			for(i=0; i < COUNT; i++) {
				val = adValue[i];
				rtn = writen(fd, (char *)&val, sizeof(int));
				if(rtn != sizeof(int)) {
					printf("Read ad value error : 0x%x\n", val);
				} else {
					printf("Read ad value : 0x%x\n", val);
				}
			}*/
			break;
    	case 11:
			printf("          Start AD\n");
			for(i=0; i < COUNT; i++) {
				val = inb(0x645);
				usleep(1000);
				myVal.data[0] = inb(0x646); //low
				myVal.data[1] = inb(0x647); //high
				adValue[i] = myVal.val;
			}	
    		write_ACK(fd);
			break;
	    case 12:
			printf("          Set channel number = 1 for AUX DA\n");
			auxChNum = 0;
    		write_ACK(fd);
			break;
	    case 13:
			printf("          AUX DA ch:%d val:%xh\n", auxChNum+1, val);
			SetAuxDA(auxChNum, (unsigned char)val);
			auxChNum++;
    		write_ACK(fd);
			break;
		case 14:
			printf("          Read meter value\n");
    		write_ACK(fd);
			real = RealData();
			rtn = writen(fd, (char *)&real, sizeof(long));
			if(rtn != sizeof(long)) {
			   	printf("          Meter value send error : %ld\n", real);
			} else {
				printf("          Meter value : %ld\n", real);
			}
			break;
		case 16:
			printf("          DMM Setting %d\n", val);
			if(val == DMM_IN_SBC) {
				txsettings("*RST\n", 0);
				txsettings("SYST:REM\n", 0);
				txsettings("CONF:VOLT:DC DEF,DEF\n", 0);
				txsettings("SAMPLE:COUNT 1\n", 0);
			}
    		write_ACK(fd);
			break;
		case 17:
			switch(val) {
				case 0:
					printf("          Unuse calibration box\n");
					break;
				case 1:
					printf("          Calibration box controlled by SBC\n");
					break;
				case 2:
					printf("          Calibration box conttrolled by PC\n");
					break;
				default:
					printf("          Unuse calibration box\n");
					break;
			}
    		write_ACK(fd);
			break;
		case 18:
			//mux
			outb(0, 0x622);
			outb(0x15, 0x621);
			outb(0, 0x620);
			//auxDA
			SetAuxDA(-1, 0x7F);
			//relay
			outb(0, 0x640);
			outb(0, 0x641);
			outb(0, 0x642);
			outb(0, 0x643);
			outb(0, 0x644);
			//mainDA
			outb(0, 0x648); //high
			outb(0, 0x649); //low
			outb(0, 0x64A); //low
			printf("          Initialize2\n");
    		write_ACK(fd);
			break;
		case 19:
			val = 0x1002;
			writen(fd, (char *)&val, sizeof(int));
			printf("          Program version : 0x%x\n", val);
			break;
    	case 20:
			printf("          Start AD\n");
			for(i=0; i < COUNT; i++) {
				val = inb(0x645);
				usleep(1000);
				myVal.data[0] = inb(0x646); //low
				myVal.data[1] = inb(0x647); //high
				adValue[i] = myVal.val;
			}	
    		write_ACK(fd);
			break;
		case 21:
    		write_ACK(fd);
			for(i=0; i < 3; i++) {
				SetMux(i, 2);
				usleep(10000);
				real = 0;
				for(j=0; j < SOURCE_COUNT; j++) {
					val = inb(0x645);
					usleep(1000);
					myVal.data[0] = inb(0x646); //low
					myVal.data[1] = inb(0x647); //high
					real += myVal.val;
				}	
				if(i == 0) {
					V_source.source_p = real / SOURCE_COUNT;
				} else if(i == 1) {
					V_source.source_n = real / SOURCE_COUNT;
				} else if(i == 2) {
					V_source.source_0 = real / SOURCE_COUNT;
				}
				V_source.data = 0;
			}
			printf("          V_source %d, %d, %d\n", V_source.source_p,
				V_source.source_0, V_source.source_n);
			rtn = writen(fd, (char *)&V_source, sizeof(SOURCE));
			if(rtn != sizeof(SOURCE)) {
			   	printf("          V_source send error\n");
			}
			break;
		case 22:
    		write_ACK(fd);
			for(i=0; i < 3; i++) {
				SetMux(i, 3);
				usleep(10000);
				real = 0;
				for(j=0; j < SOURCE_COUNT; j++) {
					val = inb(0x645);
					usleep(1000);
					myVal.data[0] = inb(0x646); //low
					myVal.data[1] = inb(0x647); //high
					real += myVal.val;
				}	
				if(i == 0) {
					I_source.source_p = real / SOURCE_COUNT;
				} else if(i == 1) {
					I_source.source_n = real / SOURCE_COUNT;
				} else if(i == 2) {
					I_source.source_0 = real / SOURCE_COUNT;
				}
				I_source.data = 0;
			}
			printf("          I_source %d, %d, %d\n", I_source.source_p,
				I_source.source_0, I_source.source_n);
			rtn = writen(fd, (char *)&I_source, sizeof(SOURCE));
			if(rtn != sizeof(SOURCE)) {
			   	printf("          I_source send error\n");
			}
			break;
		case 23:
    		write_ACK(fd);
			SetMux(3, 2);
			usleep(10000);
			real = 0;
			for(j=0; j < SOURCE_COUNT; j++) {
				val = inb(0x645);
				usleep(1000);
				myVal.data[0] = inb(0x646); //low
				myVal.data[1] = inb(0x647); //high
				real += myVal.val;
			}	
			val = real / SOURCE_COUNT;
			printf("          V_ref %d\n", val);
			rtn = writen(fd, (char *)&val, sizeof(int));
			if(rtn != sizeof(int)) {
			   	printf("          V_ref send error\n");
			}
			break;
    	default:
			printf("Unknown command %d\n", rtn);
    		write_ACK(fd);
			break;
    }

    return 1;
}

int ReadCommand(int fd, int *cmd, int *val)
{
    int	rtn;

    rtn = readn(fd, (char *)cmd, sizeof(int));
    if(rtn != sizeof(int)) {
		printf("Client connection fail2-1\n");
		CloseFD(fd);
		state = 0;
		return -1;
    }

    rtn = readn(fd, (char *)val, sizeof(int));
    if(rtn != sizeof(int)) {
		printf("Client connection fail2-2\n");
		CloseFD(fd);
		state = 0;
		return -2;
    }

    return 0;
}

int cal_std_value(double average, int count)
{
	int i, val;
	double sum;

	sum = 0.0;
	for(i=0; i < count; i++) {
		sum += (((double)adValue[i] - average)
			* ((double)adValue[i] = average));
	}

	val = (int)(sqrt(sum / (double)count) * 10000.0);
	return val;
}

int CharToInt(char cmp)
{
	char character[10] = {'0','1','2','3','4','5','6','7','8','9'};
	int num;

	for(num=0; num < 10; num++) {
		if(cmp == character[num]) break;
	}

	return num;
}

long Conversion(void)
{
	int tmp, i, exp;
	long real, real1, temp;
	
	temp = 1000000;
	tmp = CharToInt(MeterValue[1]);
	real1 = tmp * 10000000;
	for(i=3; i <= 10; i++) {
		tmp = CharToInt(MeterValue[i]);
		real1 += tmp * temp;
		temp /= 10;
	}

	tmp = CharToInt(MeterValue[13]);
	exp = tmp * 10;
	tmp = CharToInt(MeterValue[14]);
	exp += tmp;

	temp = 1;
	for(i=0; i < exp; i++) {
		temp *= 10;
	}
	if(MeterValue[12] == '-') {
		real = real1 / temp;
	} else real = real1 * temp;

	return real;
}

int RealData(void)
{
	int rtn;
	long real;
	double tmp;

	rtn = rxsettings("READ?\n", 0);
	if(MeterValue[11] == 'E') {
		real = Conversion();
		if(MeterValue[0] == '-') real *= -1;
	} else real = 0;

	if(scanType == 0) {
	} else {
		tmp = (double)real / 2000000.0;
		tmp *= 1500000000.0;
		real = (long)tmp + (long)I_offset;
	}

	return real;
}

int swapBit(int ch)
{
	char array[4];
	int i, result;

	if(ch & 0x01)
		array[0] = 0x08;
	else
		array[0] = 0x00;

	if(ch & 0x02)
		array[1] = 0x04;
	else
		array[1] = 0x00;

	if(ch & 0x04)
		array[2] = 0x02;
	else
		array[2] = 0x00;

	if(ch & 0x08)
		array[3] = 0x01;
	else
		array[3] = 0x00;

	result = 0;
	for(i=0; i < 4; i++) {
		result |= (int)array[i];
	}

	return result;
}

void SetAuxDA(int num, unsigned char val)
{
    unsigned char sync, byte1, byte2;

	if(num >= 40) return;

    if(iopl(3)) exit(1);

    if(num < 0) {
		for(num=0; num < 64; num++) {
			if(num >= 40) return;
			sync = 0x01;
			sync = sync << (num / 8);
			outb(sync, 0x624);

			byte1 = (unsigned char)(num % 8);
			byte1 = byte1 << 4;
			byte1 |= val >> 4;
			outb(byte1, 0x625); //high

			byte2 = val << 4;
			outb(byte2, 0x626); //low
			usleep(1000);
		}
		return;
    }
	printf("kjg %d %x\n", num+1, val);

	sync = 0x01;
	sync = sync << (num / 8);
	outb(sync, 0x624);
	usleep(1000);

	byte1 = (unsigned char)(num % 8);
	byte1 = byte1 << 4;
	byte1 |= val >> 4;
	outb(byte1, 0x625); //high
	usleep(1000);

	byte2 = val << 4;
	outb(byte2, 0x626); //low
	usleep(1000);
}

void RelayOnOff(int num, int type)
{
    unsigned char relay_select;
    int ic_addr;

	if(num >= 40) return;

    if(iopl(3)) exit(1);

    ic_addr = 0x640 + (num / 8);
    relay_select = 0x01;
    relay_select = relay_select << (num % 8);

    if(type == 1) { // ON
		outb(relay_select, ic_addr);
	} else { // OFF
		outb(0, ic_addr);
    }
}

void SetMux(int num, int type)
{
    unsigned char mux1, mux2, mux3;

	if(num >= 40) return;

	if(iopl(3)) exit(1);

	mux1 = mux2 = mux3 = 0x00;
	
	switch(type) {
		case 0: //voltage
			mux3 = 0x03;
			mux2 = 0x35;
			mux1 = 0x01;
			mux1 = mux1 << (num / 8);
			mux1 = mux1 << 3;
			mux1 |= (unsigned char)(num % 8);
			break;
		case 1: //current
			mux3 = 0x03;
			mux2 = 0x25;
			mux1 = 0x01;
			mux1 = mux1 << (num / 8);
			mux1 = mux1 << 3;
			mux1 |= (unsigned char)(num % 8);
			break;
		case 2: //V_source
			mux3 = 0x00;
			mux2 = (unsigned char)num;
			mux2 = 0x30 | ((~mux2) & 0x0F);
			mux1 = 0x00;
			break;
		case 3: //I_source
			mux3 = 0x00;
			mux2 = (unsigned char)num << 2;
			mux2 = 0x20 | ((~mux2) & 0x0F);
			mux1 = 0x00;
			break;
	}

	outb(mux3, 0x622);
	outb(mux2, 0x621);
	outb(mux1, 0x620);
}
