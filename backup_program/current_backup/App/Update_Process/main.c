#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"
#include "local_message.h"
#include "main.h"

volatile S_SYSTEM_DATA	*myData;
volatile S_UPDATE_PROCESS	*myPs; //my process : UpdateProcess
char psName[16];
	
int main(void)
{
	int rtn;
	struct timeval tv;
	fd_set rfds;

	if(Initialize() < 0) return 0;

	while(myData->AppControl.signal[APP_SIG_UPDATE_PROCESS] == P1) {
		tv.tv_sec = 0;
		tv.tv_usec = 10000000;
		FD_ZERO(&rfds);

		rtn = select(0, &rfds, NULL, NULL, &tv);
		if(rtn == 0) {
			Update_Process_Control();
		} else {
		}
	}
	Close_Process();
	return 0;
}

int Initialize(void)
{
	if(Open_SystemMemory(0) < 0) return -1;
		
	myPs = &(myData->UpdateProcess);

	Init_SystemMemory();

	myData->AppControl.signal[APP_SIG_UPDATE_PROCESS] = P1;
	return 0;
}

void Update_Process_Control(void)
{
	S_MSG_VAL SendMsg;
	
	
	Check_Message();
	if(myData->mData.state == M_STANDBY) {	//module state check
		if(Update_file_state_check() == 1) {	//ok.txt file check, Auto Update file check
		
			if(myData->mData.Auto_Update_module_rdy == 0) {	//module off
				userlog(DEBUG_LOG, psName, "Auto Update_Excute file check done\n");
				myData->mData.Auto_Update_module_rdy = 1;
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_UPDATE_IO_EXIT;
				SendMsg.val[0] = M_CD_FAULT_NORMAL_TERMINAL_QUIT;
				send_msg(UPDATE_TO_IO, (char *)&SendMsg);
			}
			if(myData->mData.Auto_Update_rdy == 1) {
				userlog(DEBUG_LOG, psName, "Auto Update_Excute file module off\n");
				myData->mData.Auto_Update_rdy = 0;
				Update_Execute();	//update run
			}
		} else if(RollBack_file_state_check() == 1) {
			if(myData->mData.Auto_Update_module_rdy == 0) {	//module off
				userlog(DEBUG_LOG, psName, "Auto RollBack_Excute file check done\n");
				myData->mData.Auto_Update_module_rdy = 1;
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_UPDATE_IO_EXIT;
				SendMsg.val[0] = M_CD_FAULT_NORMAL_TERMINAL_QUIT;
				send_msg(UPDATE_TO_IO, (char *)&SendMsg);
			}
			if(myData->mData.Auto_Update_rdy == 1) {
				userlog(DEBUG_LOG, psName, "Auto RollBack_Excute file module off\n");
				myData->mData.Auto_Update_rdy = 0;
				RollBack_Execute();	//update run
			}
		}

	}
}

int Update_file_state_check(void)
{
	char fileName[128];
	int fileExist = -1;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/update/ok.txt");
	fileExist = access(fileName, 0); //ok.txt exist
	if(fileExist == 0) {	//ok.txt Exist
		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/update/Auto_Update.tar.gz");
		fileExist = -1;
		fileExist = access(fileName, 0); //update file Exist
		if(fileExist == 0) {	//update file Exist
			return 1;
		} else {	//update file Not Exist
			return 0;
		}
	} else {	//ok.txt Not Exist
		return 0;
	}
}

int RollBack_file_state_check(void)
{
	char fileName[128];
	int fileExist = -1;
	
	memset(fileName, 0x00, sizeof(fileName));
	strcpy(fileName, "/root/update/rollback.txt");
	fileExist = access(fileName, 0); //ok.txt exist
	if(fileExist == 0) {	//ok.txt Exist
		memset(fileName, 0x00, sizeof(fileName));
		strcpy(fileName, "/root/update/Auto_Update.tar.gz");
		fileExist = -1;
		fileExist = access(fileName, 0); //update file Exist
		if(fileExist == 0) {	//update file Exist
			return 1;
		} else {	//update file Not Exist
			return 0;
		}
	} else {	//ok.txt Not Exist
		return 0;
	}
}

void Update_Execute(void)
{
	userlog(DEBUG_LOG, psName, "Auto Update_Excute old file remove\n");
	
	system("rm -rf /root/update/SBC_Update");
	usleep(500000);
	system("rm -rf /root/update/backup_program");
	usleep(500000);
	userlog(DEBUG_LOG, psName, "Auto Update_Excute Unzip\n");
	
	system("tar xvfz /root/update/Auto_Update.tar.gz -C /root/update");
	usleep(500000);
	system("cd /root/update/backup_program ; tar xvfz current_backup.tar.gz");
	usleep(500000);
	system("cd /root/update/backup_program ; tar xvfz config_backup.tar.gz");
	usleep(500000);
	system("cd /root/update/backup_program/current_backup/App/AppControl ; cp SBC_Update /root/update");
	usleep(500000);

	userlog(DEBUG_LOG, psName, "Auto Update_Excute Update script excute\n");
	system("cd /root/update ; ./SBC_Update");
}

void RollBack_Execute(void)
{
	userlog(DEBUG_LOG, psName, "Auto RollBack_Excute old file remove\n");
	
	system("rm -rf /root/update/SBC_RollBack");
	usleep(500000);
	system("rm -rf /root/update/backup_program");
	usleep(500000);
	userlog(DEBUG_LOG, psName, "Auto RollBack_Excute Unzip\n");
	
	system("tar xvfz /root/update/Auto_Update.tar.gz -C /root/update");
	usleep(500000);
	system("cd /root/update/backup_program ; tar xvfz current_backup.tar.gz");
	usleep(500000);
	system("cd /root/update/backup_program ; tar xvfz config_backup.tar.gz");
	usleep(500000);
	system("cd /project/system_mcts/current/App/AppControl ; cp SBC_RollBack /root/update");	//rollback //ktg_231113
	usleep(500000);

	userlog(DEBUG_LOG, psName, "Auto RollBack_Excute Update script excute\n");
	system("cd /root/update ; ./SBC_RollBack");
}


void Close_Process(void)
{
	myData->AppControl.signal[APP_SIG_UPDATE_PROCESS] = P3;
	
}
