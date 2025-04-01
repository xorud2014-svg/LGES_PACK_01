#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <math.h>
//#include <asm/io.h>
#include <sys/io.h>
#include "../INC/com_socket.h"
#include "../INC/com_io.h"
#include "../INC/comm.h"
#include "../INC/comm1.h"
#include "../INC/switch.h"

#define VOLTAGE 0
#define CURRENT 1
#define OFF 0
#define ON 1
#define COUNT 5
#define REF_IC_READ 5
#define COUNT1 50 
#define NonUse	 0
#define SBC_CTRL 1
#define PC_CTRL 2

#define START_ADDR	0x01
#define AD_HADDR	0x01
#define	AD_LADDR	0x00
#define AUX_DA_SYNC 0x05
#define AUX_HADDR	0x06
#define AUX_LADDR	0x07
#define DIV_ADDR	0x0F
#define REF_HIGH	0x02
#define REF_VLT		0x03
#define REF_CRRNT	0x04
#define MUX1_ADDR	0x08
#define MUX2_ADDR	0x09
#define MUX3_ADDR	0x0A
#define H_CLR		0x0B

#define LGCN_PB2	64
#define VKEIG_5V10A	32
#define LG_5V10A    25

#define MAX_CH_PER_BD	LGCN_PB2
#define MUX				LGCN_PB2

short int tmpValue[COUNT1];
int state, scanType, chNum,Channel;
int meter,calbox;
int ChBdNum, baseAddr;
int usePort, caliAddr;
char value[256];

typedef struct avstdev_tag{
    int ave;
    int stdev;
	int data1;
	int data2;
} AVSTDEV;

typedef struct vref_tag {
    int Vref_P5V;
    int Vref_0V;
    int Vref_N5V;
    int data;
} VREF;

typedef struct iref_tag {
    int Iref_P100;
    int Iref_0;
    int Iref_N100;
    int data;
} IREF;

union ADDAVal {
    short int val;
    unsigned char data[2];
};

FILE *fp;

union ADDAVal myVal;
union ADDAVal cmd_v;
union ADDAVal cmd_i;

CommParam comm_param = {"/dev/ttyS1", TMT_B96, TMT_FXONXOFF};  //Calibration Box
CommParam comm_param1 = {"/dev/ttyS0", TMT_B96, TMT_FXONXOFF};  //Meter(34401A)

int CharToInt(char cmp)
{
	int num;
	char character[10] = {'0','1','2','3','4','5','6','7','8','9'};
	for(num=0; num < 10; num++) {
		if(cmp == character[num]) break;
	}
	return num;
}

long Conversion(void)
{
	int tmp, i, exp;
	long real, real1, temp;
	
	temp = 1000000L;
	tmp = CharToInt(value[1]);
	real1 = tmp * 10000000L;
	for(i=3; i <= 10; i++) {
		tmp = CharToInt(value[i]);
		real1 += tmp * temp;
		temp /= 10L;
	}

	tmp = CharToInt(value[13]);
	exp = tmp * 10;
	tmp = CharToInt(value[14]);
	exp += tmp;

	temp = 1L;
	for(i=0; i < exp; i++) {
		temp *= 10L;
	}
	if(value[12] == '-') {
		real = real1 / temp;
	} else real = real1 * temp;
	return real;
}

int RealData(void)
{
	int rtn;
	long real;
	rtn=rxsettings1("READ?\n",0);
	if(value[11] == 'E') {
		real = Conversion();
		if(value[0] == '-') real *= -1L;
	} else real = 0L;
	return real;
}

int swapBit(int ch) {
	int i, result;
	char array[4];
	result &= 0x00;
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
	for(i=0; i<4; i++)
		result |= array[i];
	return result;
}

void SetAuxDA(int num, unsigned char val)
{
    int j = 0;
    unsigned char auxDAChannel, low, sync;
    if(iopl(3)) exit(1);
    if(num < 0) {
		for(j = 0; j < 64; j++){
			if(j >= MAX_CH_PER_BD) break;
			sync = 0x01;
			sync = sync << (j / 8);
			outb(sync, (baseAddr + AUX_DA_SYNC));

    		auxDAChannel = (unsigned char)(j % 8); 
	   		auxDAChannel = auxDAChannel << 4;
		   	auxDAChannel |= val >> 4;
			outb(auxDAChannel, (baseAddr + AUX_HADDR)); //high
			low = val << 4;
			outb(low, (baseAddr + AUX_LADDR)); //low
			usleep(100);
		}
		printf("all aux da set\n");
    } else {
		j = num;
		if(j >= MAX_CH_PER_BD) return;
		printf("aux cali ch num : %d, val : %x\n", j, val);
		sync = 0x01;
		sync = sync << (j / 8);
		outb(sync, (baseAddr + AUX_DA_SYNC));

    	auxDAChannel = (unsigned char)(j % 8); 
	    auxDAChannel = auxDAChannel << 4;
	    auxDAChannel |= val >> 4;
		outb(auxDAChannel, (baseAddr + AUX_HADDR)); //high
		low = val << 4;
		outb(low, (baseAddr + AUX_LADDR)); //low
		usleep(100);
    }
}

void RelayAllOn(void)
{
    int offSetAddr;
    if(iopl(3)) exit(1);

	printf("Relay all on\n");
	outb(0x01, (baseAddr + DIV_ADDR));	
	for(offSetAddr=0;offSetAddr<8;offSetAddr++)
	{
		outb(0xff, (baseAddr+offSetAddr));
	} 
	outb(0x00, (baseAddr + DIV_ADDR));
  }
  
void RelayAllOff(void)
{
    int offSetAddr;
    if(iopl(3)) exit(1);

	printf("Relay all off\n");
	outb(0x01, (baseAddr + DIV_ADDR));	
	for(offSetAddr=0;offSetAddr<8;offSetAddr++)
	{
		outb(0x00, (baseAddr+offSetAddr));
	} 
	outb(0x00, (baseAddr + DIV_ADDR));	
}

void RelayOnOff(int num, int type)
{
    int offSetAddr;
    unsigned char icVal;
    if(iopl(3)) exit(1);
    offSetAddr = num / 8;
    icVal = 0x01;
    icVal = icVal << (num % 8);
    if(type == ON) {
		outb(0x01, (baseAddr + DIV_ADDR));	
		outb(icVal, (baseAddr + offSetAddr));
		outb(0x00, (baseAddr + DIV_ADDR));
		printf("Relay : %d %x ---> on\n", num+1, baseAddr+offSetAddr);
    
	} else { // OFF
		outb(0x01, (baseAddr + DIV_ADDR));	
		outb(0x00, (baseAddr+offSetAddr));
		outb(0x00, (baseAddr + DIV_ADDR));
		printf("Relay : %d %x ---> off\n", num+1, baseAddr+offSetAddr);
    }
}

void SetMux(int num, int type)
{
    unsigned char muxVal1, muxVal2=0x00, muxVal3=0x00;
	int mux_ch;
	if(iopl(3)) exit(1);

	if(MUX == LGCN_PB2){
		mux_ch = 16; //LGCN_PB2
	}else{
		mux_ch = 8;
	}

	muxVal1 = 0x01;
	muxVal1 = muxVal1 << (num / mux_ch);
	if(type == VOLTAGE) { // voltage
		muxVal2 = 0x00;
	} else if(type==CURRENT){ // current
		muxVal2 = 0x10;
	}
	muxVal3 = (num % mux_ch) & 0x0F;
	
	outb(muxVal1 | 0x40 , (baseAddr + MUX1_ADDR));
	outb(muxVal2 , (baseAddr + MUX2_ADDR));
	outb(muxVal3 , (baseAddr + MUX3_ADDR));

}

void VoltRefReset()
{
	printf("Voltage Refrence RESET\n");
	outb(0x00, (baseAddr + REF_HIGH));	
	outb(0x00, (baseAddr + REF_VLT));
}
void CurrRefReset()
{
	printf("Current Refrence RESET\n");
	outb(0x00, (baseAddr + REF_HIGH));	
	outb(0x00, (baseAddr + REF_CRRNT));
}
	
int ReadCommand(int fd, int *cmd, int *val)
{
    int	ret,VERSION;

    ret = readn(fd, (char *)cmd, sizeof(int));
    if(ret != sizeof(int)) {
		printf("Client connection fail2-1\n");
		CloseFD(fd);
		state = 0;
		return -1;
    }
    ret = readn(fd, (char *)val, sizeof(int));
    if(ret != sizeof(int)) {
		printf("Client connection fail2-2\n");
		CloseFD(fd);
		state = 0;
		return -2;
    }
	if(*cmd==19){
		VERSION=0x1002;
		writen(fd,(char *)&VERSION, sizeof(int));
		printf("This program version is : %x\n",VERSION); 
	}else
    write_ACK(fd);
    return 0;
}

void CreatADFile(int count,int type)
{
	char fname[128];
	int i;
	FILE *fp;

	memset(fname,0x00,sizeof(fname));
	if(type==0){
		sprintf(fname,"./tmp/ADVALUEVolt_%dB%dC.csv",ChBdNum+1,Channel+1);
	}else{
		sprintf(fname,"./tmp/ADVALUECurr_%dB%dC.csv",ChBdNum+1,Channel+1);
	}
	fp=fopen(fname,"w");
	if(fp==NULL){
		puts("Cannot open file\n");
	}
	for(i=0;i<count;i++){
		fprintf(fp,"%d,%d\n",i+1,tmpValue[i]);
	}
	fclose(fp);
}

int AD_stdev(float Average,int count){
	int i,val=0;
	float tol=0;
	for(i=0; i< count; i++){
		tol+=(((float)tmpValue[i]-Average)*((float)tmpValue[i]-Average));
	}
	val=(int )(sqrt(tol/count)*10000);
	return val;
}
	
int DoRead(int fd)
{
    unsigned char muxVal1=0x00;
    int	ret, rtn, val, i, j,stdev, tmp=0, count;
	float ave;
	long real;

	AVSTDEV avstdev;
	VREF vref;
	IREF iref;

    if(iopl(3)) exit(1);
    if(state == 0) {
		printf("New client entered\n");
		ret = readn(fd, (char *)&rtn, sizeof(int));
		if(ret < 0) {
		    printf("Client connection fail1\n");
	    	return 0;
		}
		write_ACK(fd);
		printf("Calibration Client connected !\n");
		state = 1;

    } else {
		ret = ReadCommand(fd, &rtn, &val);
		if(ret < 0) return 0;
		switch(rtn) {
	    	case 2:// main b'd select
				printf("Set channel board number : %d\n", val);
				ChBdNum = val - 1;
				baseAddr = 0x620 + (0x10*ChBdNum);
				SetAuxDA(-1, 0x7f);
				break;
	    	case 3://write current reference
				printf("Set current reference : %d\n", val);
				myVal.val = (short int)val;
				cmd_i.val = (short int)val;
				
				outb(myVal.data[1], (baseAddr + REF_HIGH));
				outb(myVal.data[0], (baseAddr + REF_CRRNT));
				break;
	    	case 4://write voltage reference
				printf("Set voltage reference : %d\n", val);
				myVal.val = (short int)val;
				cmd_v.val = (short int)val;
				outb(myVal.data[1], (baseAddr + REF_HIGH));
				outb(myVal.data[0], (baseAddr + REF_VLT));
				break;
	    	case 5:
				printf("Set scan  channel : %d\n", val+1);
				break;
		    case 6://relay, tray switch on
				if(scanType==CURRENT){
					printf("Set relay on : %d\n", val+1);
					RelayOnOff(val, ON);
					if(calbox==SBC_CTRL){//if calibration box controled by SBC
						printf("TraySwitch:On, BdNum:%d, ChNo:%d\n",ChBdNum+1,val+1);
						TraySwitchOnOff(ON,ChBdNum,val);//Calibrator Fixture B'd S/W On,khk
					}
				}else if(scanType==VOLTAGE){
					if(calbox==NonUse){
						printf("Set relay on : %d\n", val+1);
						RelayOnOff(val, ON);
					}else if(calbox==SBC_CTRL){
						printf("TraySwitch:On, BdNum:%d, ChNo:%d\n",ChBdNum+1,val+1);
						TraySwitchOnOff(ON,ChBdNum,val);//Calibrator Fixture B'd S/W On,khk
					}
				}
				break;
		    case 7://relay,tray switch off
				if(scanType==CURRENT){
					printf("Set relay off : %d\n", val+1);
					RelayOnOff(val, OFF);
					if(calbox==SBC_CTRL){
						printf("TraySwitch:Off, BdNum:%d, ChNo:%d\n",ChBdNum+1,val+1);
						TraySwitchOnOff(OFF,ChBdNum,val);//Calibrator Fixture S/W Off,khk
					}
				}else if(scanType==VOLTAGE){
					if(calbox==NonUse){
						printf("Set relay off : %d\n", val+1);
						RelayOnOff(val, OFF);
					}else if(calbox==SBC_CTRL){
						printf("TraySwitch:Off, BdNum:%d, ChNo:%d\n",ChBdNum+1,val+1);
						TraySwitchOnOff(OFF,ChBdNum,val);//Calibrator Fixture S/W Off,khk
					}
				}
				break;
		    case 8: //cc(1),cv(0) mode select
				scanType = val;
				printf("Set scan type : %d\n", val);
				if(calbox==SBC_CTRL){
					ScanTypeSetting(val);	
				}else if(calbox==NonUse){
					if(val == 0){
						outb(0x00,0x610); //CC/CV Relay Togle
					}else{
						outb(0x01,0x610); //CC/CV Relay Togle
					}
				}
				if(val==VOLTAGE && calbox!=NonUse){
					RelayAllOn();//H/W Bugger control only for LG 70setserise
				}else{
					RelayAllOff();//H/W Bugger control only for LG 70setserise
				}
				break;
		    case 9://mux select
				Channel=val;
				printf("Set mux : %d %d\n", val+1, scanType);
				SetMux(val, scanType);
				break;
		    case 10://Send AD value(low acuracy) 
				tmp=0;
				ave=0;
				stdev=0;
				for(i=0;i<COUNT;i++){
				tmp+=tmpValue[i];
				}
				ave=(float)tmp/COUNT;
				stdev=AD_stdev(ave,COUNT);
				avstdev.ave=(int )ave;
				avstdev.stdev=stdev;
				avstdev.data1=0;
				avstdev.data2=0;
    			printf("AD Average : %d , AD STDEV %d: \n",(int)ave, stdev);
    			ret = writen(fd, (char *)&avstdev, sizeof(AVSTDEV));
    			if(ret != sizeof(AVSTDEV))
   				printf("ERROR: AD Average don't send\n");
				break;
	    	case 11: //AD Start(high acuracy)
				printf("AD Start\n");
				for(i=0; i <COUNT1; i++) {
					outb(0x00, (baseAddr + START_ADDR));
					usleep(1000);
					myVal.data[1] = inb(baseAddr + AD_HADDR);
					myVal.data[0] = inb(baseAddr + AD_LADDR);
					tmpValue[i] = myVal.val;
				}
				count=COUNT1;
				CreatADFile(count,scanType);//for AD Value confirmation
				break;
		    case 12:
				chNum = 0;
				printf("Set channel number = 0 for AUX DA\n");
				break;
		    case 13:
				SetAuxDA(chNum, (unsigned char)val);
				printf("Set AUX DA at %d to %d\n", chNum+1, val);
				chNum++;
				break;
			case 14://Read meter value
				if(meter==SBC_CTRL){
				real = RealData();
				printf("Meter value : %ld\n", real);
				ret = writen(fd, (char *)&real, sizeof(long));
				if(ret != sizeof(long))
				   	printf("Error :Meter value don't send: %ld\n", real);
				}else{
					real=0;
					ret = writen(fd, (char *)&real, sizeof(long));
				}
				break;
			case 15://send AD average
				tmp=0;
				ave=0;
				stdev=0;
				for(i=0;i<COUNT1;i++){
				tmp+=tmpValue[i];
				}
				ave=(float)tmp/COUNT1;
				stdev=AD_stdev(ave,COUNT1);
				avstdev.ave=(int )ave;
				avstdev.stdev=stdev;
				avstdev.data1=0;
				avstdev.data2=0;

    			printf("AD Average : %d , AD STDEV %d: \n", (int)ave, stdev);
    			ret = writen(fd, (char *)&avstdev, sizeof(AVSTDEV));
    			if(ret != sizeof(AVSTDEV))
   				printf("ERROR: AD Average don't send\n");
				break;
			case 16://Meter Control select
				switch(val){
					case 0:
						printf("Meter controled by PC\n");
						meter=PC_CTRL;
						break;
					case 1:
						printf("Meter controled by SBC\n");
						meter=SBC_CTRL;
						txsettings1("*RST\n",0);
						txsettings1("SYST:REM\n",0);
						txsettings1("CONF:VOLT:DC DEF,DEF\n",0);
						txsettings1("SAMPLE:COUNT 1\n",0);
						break;
					default:
						printf("Meter controled by PC\n");
						meter=PC_CTRL;
						break;
				}	
				break;
			case 17://Calibration Control Box select
				switch(val){
					case 0:
						printf("Unuse Calibration Box \n");
						calbox=NonUse;
						break;
					case 1:
						printf("Calibration Box controled by SBC\n");
						calbox=SBC_CTRL;
						break;
					case 2:
						printf("Calibration Box controled by PC\n");
						calbox=PC_CTRL;
						break;
					default:
						printf("Unuse Calibration Box\n");
						calbox=NonUse;
						break;
					}
				break;
			case 18://Reset
//				ResetTraySwitch();
				RelayAllOff();
				SetAuxDA(-1, 0x7f);
				VoltRefReset();
				CurrRefReset();
				iref.Iref_P100=0;
				iref.Iref_0=0;
				iref.Iref_N100=0;
				vref.Vref_P5V=0;
				vref.Vref_0V=0;
				vref.Vref_N5V=0;
				system("rm -rf tmp/*");
				break;
	    	case 20://AD Start(low acuracy)
				printf("AD Start\n");
				for(i=0; i <COUNT; i++) {
					outb(0x00, (baseAddr + START_ADDR));
					usleep(1000);
					myVal.data[1] = inb(baseAddr + AD_HADDR);
					myVal.data[0] = inb(baseAddr + AD_LADDR);
					tmpValue[i] = myVal.val;
				    printf("AD value : %d\n", tmpValue[i]);
				}
				count=COUNT;
				CreatADFile(count,scanType);//for AD Value confirmation
				break;
	    	case 21: //Ref IC Read V
				for(i=0; i<3; i++){
					//mux
					if(i==0){
						muxVal1=0x00;
					}else if(i==1){
						muxVal1=0x02;
					}else if(i==2){
						muxVal1=0x01;
					}
					outb(muxVal1, (baseAddr + MUX2_ADDR));
					outb(0x70, (baseAddr + MUX1_ADDR));
					usleep(1000);
					//AD
					tmp=0;
					for(j=0;j<REF_IC_READ;j++){
						outb(0x00, (baseAddr + START_ADDR));
						myVal.data[1] = 0;
						myVal.data[0] = 0;
						usleep(1000);
						myVal.data[1] = inb(baseAddr + AD_HADDR);
						myVal.data[0] = inb(baseAddr + AD_LADDR);
						tmp+=myVal.val;
					}
    				if(i==0){
						vref.Vref_P5V
				       		= tmp/REF_IC_READ;
					}
    				if(i==1){
						vref.Vref_0V
				       		= tmp/REF_IC_READ;
					}
    				if(i==2){
						vref.Vref_N5V
				       		= myVal.val;
					}
					vref.data=0;
				}
				
    			printf("Reference V --> 5V: %d, 0V: %d, -5V: %d\n",
						vref.Vref_P5V,
						vref.Vref_0V,
						vref.Vref_N5V);
    			ret = writen(fd, (char *)&vref, sizeof(VREF));
    			if(ret != sizeof(VREF))
   				printf("ERROR: Reference V don't send \n");
				break;
	    	case 22: //Ref IC Read I
				for(i=0; i<3; i++){
					//mux
					if(i==0){
						muxVal1=0x10;
					}else if(i==1){
						muxVal1=0x18;
					}else if(i==2){
						muxVal1=0x14;
					}
					outb(muxVal1, (baseAddr + MUX2_ADDR));
					outb(0x70, (baseAddr + MUX1_ADDR));
					usleep(100000);
					//AD
					tmp=0;
					for(j=0;j<REF_IC_READ;j++){
						outb(0x00, (baseAddr + START_ADDR));
						myVal.data[1] = 0;
						myVal.data[0] = 0;
						usleep(50000);
						myVal.data[1] = inb(baseAddr + AD_HADDR);
						myVal.data[0] = inb(baseAddr + AD_LADDR);
						tmp+=myVal.val;
					}
    				if(i==0){
						iref.Iref_P100
				       		= tmp/REF_IC_READ;
					}
    				if(i==1){
						iref.Iref_0
				       		= tmp/REF_IC_READ;
					}
    				if(i==2){
						iref.Iref_N100
				       		= tmp/REF_IC_READ;
					}
					iref.data=0;
				}
    			printf("Reference I-> 110mA : %d, 0mA: %d, -110mA : %d\n",
						iref.Iref_P100,
						iref.Iref_0,
						iref.Iref_N100);
    			ret = writen(fd, (char *)&iref, sizeof(IREF));
    			if(ret != sizeof(IREF))
   				printf("ERROR: Reference I don't send \n");
				break;
			case 23:// V Ref CMD READ
				break;
			default: break;
		}
		
    }
    return 1;
}
	
int main(int argc, char *argv[])
{
    int	netPort, rtn;

    if(iopl(3)) exit(1);
    //if(ioperm(0x3f0, 50, 1)) { perror("ioperm1"); exit(1); }
    
    state = 0;
    scanType = VOLTAGE;
	CellMapping();
    if(argc != 3) {
		printf("Usage : %s NumNetPort NumUsePort\n",argv[0]);
    } else {
		netPort = atoi(argv[1]);
		usePort = atoi(argv[2]);
		printf("Net Port : %d, Use Port : %d\n", netPort, usePort);
		rtn = ServerEngine(netPort, DoRead);
    }
   
    //if(ioperm(0x3f0, 50, 0)) { perror("ioperm2"); exit(1); }
    exit(0);
}
