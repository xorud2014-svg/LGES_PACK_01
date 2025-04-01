#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdlib.h>
#include "comm.h"
#include "main.h"

char rxData[256], txData[256];
int retryCount, retry;
CommParam comm_param = { "/dev/ttyS0", TMT_B96, TMT_FXONXOFF};

int main(int argc, char *argv[])
{
    int cmd, ch, val, com;
	char tmp[10];
   
	if(argc == 1) return 0;
	else {
		retry = atoi(argv[1]);
		com = atoi(argv[2]);
		memset(&comm_param.device, 0x00, sizeof comm_param.device);
		switch(com) {
			case 1:
				memcpy(&comm_param.device, "/dev/ttyS0",
					sizeof comm_param.device);
				break;
			case 2:
				memcpy(&comm_param.device, "/dev/ttyS1",
					sizeof comm_param.device);
				break;
			case 3:
				memcpy(&comm_param.device, "/dev/ttyS2",
					sizeof comm_param.device);
				break;
			case 4:
				memcpy(&comm_param.device, "/dev/ttyS3",
					sizeof comm_param.device);
				break;
			default: return 0;
		}
	}
	
    while(1) {
		retryCount = 0;
		printf("SwitchTest >> ");
		scanf("%s", tmp);
		
		if(strcmp(tmp, "test") == 0) {
			scanf("%d %x", &ch, &val);
			printf("test %x %x\n", ch, val);
			cmd = CMD_TEST;
			TransData(cmd, ch, val);
		} else if(strcmp(tmp, "ch") == 0) {
			scanf("%d %x", &ch, &val);
			if(val == 0) {
				printf("off %d\n", ch);
				cmd = CMD_ONE_OFF;
				val = 0;
				TransData(cmd, ch-1, val);
			} else if(val == 1) {
				printf("on %d\n", ch);
				cmd = CMD_ONE_ON;
				val = 0;
				TransData(cmd, ch-1, val);
			} else if(val == 3) {
				printf("on %d\n", ch);
				cmd = CMD_ONE_ON;
				val = 0;
				TransData(cmd, ch-1, val);
				usleep(500000);
				printf("off %d\n", ch);
				cmd = CMD_ONE_OFF;
				val = 0;
				TransData(cmd, ch-1, val);
			}
		} else if(strcmp(tmp, "scan") == 0) {
			scanf("%d %d", &ch, &val);
			ch--;
			for(com=0; com < 32; com++) {
				cmd = CMD_ONE_ON;
				TransData(cmd, (com + ch * 32), 0);
				if(val != 0) usleep(500000);
			}
			cmd = CMD_ONE_OFF;
			TransData(cmd, (com-1 + ch * 32), 0);
		} else if(strcmp(tmp, "quit") == 0) {
			exit(0);
		}
    }
    return 0;
}

void TransData(int cmd, int ch, int val)
{
	int rtn;
	char tmp;
	memset(txData, 0x00, sizeof txData);
	tmp = STX; txData[0] = tmp; tmp = SWITCH_ADDR; txData[1] = tmp;
	txData[2] = (unsigned char)cmd;
	txData[3] = (unsigned char)ch;
	txData[4] = (unsigned char)val;

	while(1) {
		rtn = SendSerialData();
		if(rtn < 0) {
			if(RetryCheck() < 0) break;
		} else break;
	}
}

int SendSerialData(void)
{
	int rtn;
	BccCreate(5);
	txData[6] = ETX;
	
	SendDataPrint(7);
	rtn = rxsettings(txData, 7);
	if(rtn < 0) return rtn;
	ReceiveDataPrint(7);

	rtn = BccCheck(5);
	if(rtn < 0) return rtn;
	rtn = CodeCheck(2);
	return rtn;
}

void BccCreate(int count)
{
	int i;
	char bcc;
	bcc = 0x00;
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
		printf("%x ", (unsigned char)txData[i]);
	}
	printf("_se\n");
}

void ReceiveDataPrint(int count)
{
	int i;
	//printf("retryCount %d, receive %s\n", retryCount, rxData);
	for(i=0; i < count; i++) {
		printf("%x ", (unsigned char)rxData[i]);
	}
	printf("_re\n");
}

int BccCheck(int count)
{
	int i;
	char bcc;
	bcc = 0x00;
	for(i=0; i < count; i++) {
		bcc ^= rxData[i];
	}
	printf("rxData[bcc] = %x : check bcc = %x\n", (unsigned char)rxData[count],
		(unsigned char)bcc);
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

int RetryCheck(void)
{
	retryCount++;
	printf("retry %d\n", retryCount);
	if(retryCount >= retry) return -1;
	else return 0;
}
