#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdlib.h>
#include "comm.h"
#include "comm1.h"
#include "switch.h"

static int CellArray[BD_PER_MODULE][CH_PER_BD]; 
char rxData[256], txData[256];

void ScanTypeSetting(int type)
{
	int cmd, ch, val;
	
	if(type == 1) {//CC mode
		cmd = CMD_MODE_OUT;
		ch = 0;
		val = 7;
	} else { //CV mode
		cmd = CMD_MODE_OUT;
		ch = 0;
		val = 0;
	}

	TransData(cmd, ch, val);
}
		
void TraySwitchOnOff(int OnOff, int bd, int cnl)
{
	int cmd, ch, val, trayCh;
		
	trayCh = CellArray[bd][cnl] - 1;	
//	printf("MainBdCh=%d, trayCh=%d\n", cnl+1, trayCh+1);
		
	if(OnOff == 0) { //OFF
		cmd = CMD_ALL_OFF;
	} else { //ON
		cmd = CMD_ONE_ON;
	}
	ch = trayCh;
	val = 0;

	TransData(cmd, ch, val);
}

void TransData(int cmd, int ch, int val)
{
	char tmp;
	int rtn;

	memset(txData, 0, sizeof txData);
	tmp = STX; txData[0] = tmp;
	tmp = SWITCH_ADDR; txData[1] = tmp;
	txData[2] = (unsigned char)cmd;
	txData[3] = (unsigned char)ch;
	txData[4] = (unsigned char)val;

	rtn = SendSerialData();
}

int SendSerialData(void)
{
	int rtn;

	BccCreate(5);
	txData[6] = ETX;
	
//	SendDataPrint(7);

	rtn = rxsettings(txData,7);
	if(rtn < 0) return rtn;

//	ReceiveDataPrint(7);

	rtn = BccCheck(5);
	if(rtn < 0) return rtn;

	rtn = CodeCheck(2);

	return rtn;
}

void BccCreate(int count)
{
	char bcc;
	int i;

	bcc = 0;
	for(i=0; i < count; i++) {
		bcc ^= txData[i];
	}

	txData[count] = bcc;
}

void SendDataPrint(int count)
{
	int i;

	//printf("send %s\n", txData);
	for(i=0; i < count; i++) {
		printf("txData[%d] :%x\n ", i, (unsigned char)txData[i]);
	}
}

void ReceiveDataPrint(int count)
{
	int i;

	for(i=0; i < count; i++) {
		printf("rxData[%d] :%x\n ", i, (unsigned char)rxData[i]);
	}
}

int BccCheck(int count)
{
	char bcc;
	int i;

	bcc = 0;
	for(i=0; i < count; i++) {
		bcc ^= rxData[i];
	}
	printf("rxData[bcc] = %x : check bcc = %x\n",
		(unsigned char)rxData[count], (unsigned char)bcc);

	if(rxData[count] == bcc) return 0;
	else return -1;
}

int CodeCheck(int count)
{
	char cmp;

	cmp = CMD_ACK;
	printf("rxData[cmd] = %x\n", (unsigned char)rxData[count]);
	if(rxData[count] == cmp) return 0;
	else return -1;
}

void CellMapping(void)
{
	char buf[10];
    int tmp, i, j;
    FILE *fp;

    if((fp = fopen("config/ChMapping", "r")) == NULL) {
		printf("ChMapping read error\n");
    	fclose(fp);
		exit(0);
	}

	memset(CellArray, 0, sizeof(CellArray));
	memset(buf, 0, sizeof buf);

	for(j=0; j < BD_PER_MODULE; j++) {
		for(i=0; i < CH_PER_BD; i++) {
			tmp = fscanf(fp, "%s", buf);
    		CellArray[j][i] = atoi(buf);
		}
	}

	fclose(fp);
}

void ResetTraySwitch(void)
{
	int cmd, ch, val;

	printf("Reset Calibration Tray Switch\n");

	cmd = CMD_RESET;
	ch = 0;
	val = 0;

	TransData(cmd, ch, val);	
}
