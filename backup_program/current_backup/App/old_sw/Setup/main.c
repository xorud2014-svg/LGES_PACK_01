#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
/*
int Initialize(void)
{
	int cnt;
    if(ReadSystemParameter() < 0) return -4; //utils.c
    if(ReadCalibrationData() < 0) return -5; //utils.c

	system(myData->misc.DataBaseDir);
	cnt = 0;
	while(1) {
		if(myData->mData.signal[M_SIG_DATABASE_LOADED] == PHASE1) break;
		sleep(1);
		cnt++;
		if(cnt > 10) {
			userlog("DataBase load fail\n");
			return -6;
		}
	}
	system(myData->misc.NvRamDir);
	cnt = 0;
	while(1) {
		if(myData->mData.signal[M_SIG_NVRAM_LOADED] == PHASE1) break;
		sleep(1);
		cnt++;
		if(cnt > 10) {
			userlog("NvRam load fail\n");
			return -7;
		}
	}
//    if(myData->mData.moduleNo > 100) system(myData->misc.meterDir);
	if(myData->misc.hwType == LINEAR)
		system(myData->misc.lModuleDir);
	else system(myData->misc.sModuleDir);
	cnt = 0;
	while(1) {
		if(myData->mData.signal[M_SIG_MODULE_LOADED] == PHASE1) break;
		sleep(1);
		cnt++;
		if(cnt > 10) {
			userlog("lControl load fail\n");
			return -8;
		}
	}
	return 0;
}

void DataBase_kill(void)
{
	int rtn;
	char buf[20], processKill[20] = "kill ";
    FILE *fp;

	system("ps -a | grep DataBase > DataBaseKill.txt");
    if((fp = fopen("DataBaseKill.txt", "r")) != NULL) {
    	rtn = fscanf(fp, "%s", buf);
		if(rtn > 0) {
			strcat(processKill, buf);
			printf("DataBase %s\n", processKill);
			system(processKill);
		}
	}
	fclose(fp);
}

void NvRam_kill(void)
{
	int rtn;
	char buf[20], processKill[20] = "kill ";
    FILE *fp;

	system("ps -a | grep NvRam > NvRamKill.txt");
    if((fp = fopen("NvRamKill.txt", "r")) != NULL) {
    	rtn = fscanf(fp, "%s", buf);
		if(rtn > 0) {
			strcat(processKill, buf);
			printf("NvRam %s\n", processKill);
			system(processKill);
		}
	}
	fclose(fp);
}

*/

int main(void)
{
    int rtn, tmp, i, j=0, k=0, moduleNo;
	char temp[30], num[5], path[50];
    FILE *fp;

	memset(temp, 0x00, sizeof(temp));
	memset(num, 0x00, sizeof(num));

    if((fp = fopen("/etc/sysconfig/network-scripts/ifcfg-eth0", "r")) != NULL) {
		rtn = 0;
    	tmp = fscanf(fp, "%s", temp);	tmp = fscanf(fp, "%s", temp);
    	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
    } else rtn = -1;
    fclose(fp);
	
	if(rtn == 0) {
		printf("%s\n", temp);
		for(i=0; i < 30; i++) {
			if(temp[i] == 0x00) break;
			if(j == 3) {
				num[k] = temp[i];
				k++;
			}
			if(temp[i] == '.') j++;
		}
	} else {
		printf("ifcfg-eth0 file read error\n");
		return rtn;
	}
	moduleNo = atoi(num) - 130;
	printf("moduleNo=%d\n", moduleNo);
	
    if((fp = fopen("../Client/config/moduleNo", "w")) != NULL) {
		fprintf(fp, "%d\n", moduleNo);
	} else rtn = -2;
	fclose(fp);
	if(rtn == 0) {
		
	} else {
		printf("moduleNo file open error\n");
		return rtn;
	}

	memset(path, 0x00, sizeof(path));
	strcat(path, "cd ../Client; rm -rf s15*.tar");
	printf("%s\n", path);
	system(path);

	memset(path, 0x00, sizeof(path));	
	strcat(path, "cd ../Client; rm -rf cali");
	printf("%s\n", path);
	system(path);

	memset(path, 0x00, sizeof(path));	
	memset(temp, 0x00, sizeof(temp));
	if(moduleNo < 10) {
		temp[0] = (char)(48 + moduleNo);
	} else {
		temp[0] = (char)(48 + moduleNo/10);
		temp[1] = (char)(48 + moduleNo%10);
	}
	strcat(path, "cp -rf ../../../../s");
	strcat(path, temp);
	strcat(path, "_*.tar ../Client");
	printf("%s\n", path);
	system(path);

	memset(path, 0x00, sizeof(path));	
	memset(temp, 0x00, sizeof(temp));
	if(moduleNo < 10) {
		temp[0] = (char)(48 + moduleNo);
	} else {
		temp[0] = (char)(48 + moduleNo/10);
		temp[1] = (char)(48 + moduleNo%10);
	}
	strcat(path, "cd ../Client; tar xvf s");
	strcat(path, temp);
	strcat(path, "_*.tar");
	printf("%s\n", path);
	system(path);
    return rtn;
}
