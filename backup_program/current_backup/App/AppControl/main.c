#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"
#include "local_message.h"
#include "StandardInput.h"
#include "main.h"

volatile S_SYSTEM_DATA *myData;
volatile S_APP_CONTROL *myPs; //my process : AppControl
volatile S_TEST_CONDITION *myTestCond;
char psName[PROCESS_NAME_SIZE];

int main(int argc, char *argv[])
{
	int rtn;
	struct timeval tv;
	fd_set rfds;

	if(SystemLoader(argc, argv) < 0) return 0;
	
	while(myPs->signal[APP_SIG_APP_CONTROL_PROCESS] == P1) {
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
			
		rtn = select(1, &rfds, NULL, NULL, &tv);
		//userlog(DEBUG_LOG, psName, "event %d\n", rtn); //kjg_d
		if(rtn > 0) {
			StandardInput_Receive();
			AppControl();
		} else if(rtn == 0) {
			AppControl();
		}
	}

	CloseAppControl();
	return 0;
}

void AppControl(void)
{
	Check_Message();
	Check_Signal();
	Check_Process();

#ifndef __SBC_EM104_A5362__ //kjhw_150731
	Sync_Time(); //kjhw_130619
#endif
}

void Check_Signal(void)
{
	int i, rtn, bd;
	long long diff;
	time_t the_time;
	S_MSG_VAL SendMsg;

	switch(myPs->signal[APP_SIG_QUIT]) {
		case P1:
			SystemCode_Print();

			//IO_Print_log(); //kjhw_150507
			if(Read_InPoint(0, 0, I_NO_POWER_OFF) == 0) { //power off
			} else { //Don't power off
				myPs->signal[APP_SIG_QUIT_TYPE] = P0;
				myPs->signal[APP_SIG_QUIT_VALUE] = P0;
				myPs->signal[APP_SIG_QUIT] = P0;
				break;
			}

			if(myPs->signal[APP_SIG_QUIT_TYPE] == P0) { //Warning
				myPs->signal[APP_SIG_QUIT] = P0;
			} else if(myPs->signal[APP_SIG_QUIT_TYPE] == P5) { //Pause
				myPs->signal[APP_SIG_QUIT] = P0;
			} else if(myPs->signal[APP_SIG_QUIT_TYPE] == P1) { //Quit
				myPs->signal[APP_SIG_APP_CONTROL_PROCESS] = P2;
				myPs->signal[APP_SIG_QUIT] = P0;
			} else if(myPs->signal[APP_SIG_QUIT_TYPE] == P2) { //Shutdown
				(void)time(&the_time);
				myPs->misc.quitDelayTime = the_time;
				myPs->signal[APP_SIG_QUIT] = P2;
			} else if(myPs->signal[APP_SIG_QUIT_TYPE] == P3) { //force quit
				myPs->signal[APP_SIG_APP_CONTROL_PROCESS] = P2;
				myPs->signal[APP_SIG_QUIT] = P0;
			} else { //force shutdowm
				(void)time(&the_time);
				myPs->misc.quitDelayTime = the_time;
				myPs->signal[APP_SIG_QUIT] = P2;
			}
			break;
		case P2:
			(void)time(&the_time);
			diff = the_time - myPs->misc.quitDelayTime;
			if(diff > 3) { //3sec
				myPs->signal[APP_SIG_APP_CONTROL_PROCESS] = P10;
				myPs->signal[APP_SIG_QUIT] = P0;
			}
			break;
		default:
			break;
	}

	switch(myData->mData.misc.module_type) {
		case MODULE_FCH:
			if(myPs->signal[APP_SIG_CALI_UPDATE] == P0
				&& myData->mData.misc.timer_1sec > 2) {
				for(i=0; i < myData->mData.config.installedCh; i++) {
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_APP_MODULE_CH_CALI_UPDATE;
					SendMsg.val[0] = i;
					send_msg(APP_TO_MODULE, (char *)&SendMsg);
				}
				myPs->signal[APP_SIG_CALI_UPDATE] = P1;
			}
			break;
		case MODULE_PWM2:
			if(myPs->signal[APP_SIG_CALI_UPDATE] != P0) {
				if(myPs->signal[APP_SIG_CALI_UPDATE] == P50) {
					for(bd=0; bd < myData->mData.config.installedBd; bd++) {
						for(i=0; i < myData->mData.config.rangeI; i++) {
							rtn = Write_Ch_CaliData(bd, i);
							if(rtn >= 0) Read_Ch_CaliData(bd, i);
						}
					}
				} else {
					for(i=0; i < myData->mData.config.rangeI; i++) {
						bd = (int)myPs->signal[APP_SIG_CALI_UPDATE] - 1;
						rtn = Write_Ch_CaliData(bd, i);
						if(rtn >= 0) Read_Ch_CaliData(bd, i);
					}
				}
				userlog(DEBUG_LOG, psName, "CaliData update complete %d\n",
					(int)myPs->signal[APP_SIG_CALI_UPDATE]);
				myPs->signal[APP_SIG_CALI_UPDATE] = P0;
			}
			break;
		default: break;
	}

	if(myPs->signal[APP_SIG_REAL_TIME_REQUEST] == P0) {
		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		if(myPs->misc.Load_Process[PROCESS_GROUP_COA][0] == P1) {
			SendMsg.msg = MSG_APP_COA_REAL_TIME_REQUEST;
			send_msg(APP_TO_COA1, (char *)&SendMsg);
		} else if(myPs->misc.Load_Process[PROCESS_GROUP_COB][0] == P1) {
			SendMsg.msg = MSG_APP_COB_REAL_TIME_REQUEST;
			send_msg(APP_TO_COB1, (char *)&SendMsg);
		} else if(myPs->misc.Load_Process[PROCESS_GROUP_COC][0] == P1) {
			SendMsg.msg = MSG_APP_COC_REAL_TIME_REQUEST;
			send_msg(APP_TO_COC1, (char *)&SendMsg);
		} else if(myPs->misc.Load_Process[PROCESS_GROUP_COD][0] == P1) {
			SendMsg.msg = MSG_APP_COD_REAL_TIME_REQUEST;
			send_msg(APP_TO_COD1, (char *)&SendMsg);
		}
		myPs->signal[APP_SIG_REAL_TIME_REQUEST] = P1;
	}

	if(myPs->signal[APP_SIG_CALI_DA_UPDATE] != P0) {
		if(myPs->signal[APP_SIG_CALI_DA_UPDATE] != P50) {
			i = (int)myPs->signal[APP_SIG_CALI_DA_UPDATE] - 1;
			rtn = Write_Main_CaliData(i, 0);
			if(rtn == 1) Read_Main_CaliData(i, 0);
			rtn = Write_Main_CaliData(i, 1);
			if(rtn == 1) Read_Main_CaliData(i, 1);
		} else {
			for(i=0; i < myPs->config.totalGroup; i++) {
				rtn = Write_Main_CaliData(i, 0);
				if(rtn == 1) Read_Main_CaliData(i, 0);
				rtn = Write_Main_CaliData(i, 1);
				if(rtn == 1) Read_Main_CaliData(i, 1);
			}
		}

		userlog(DEBUG_LOG, psName, "MainDA_CaliData update %d\n",
			myPs->signal[APP_SIG_CALI_DA_UPDATE]);
		myPs->signal[APP_SIG_CALI_DA_UPDATE] = P0;
	}
}

void SystemCode_Print(void)
{
	int val;

	val = (int)myPs->signal[APP_SIG_QUIT_VALUE];

	//switch(myData->mData.code) {
	switch(myPs->misc.recv_code) { //kjg_101012
		case M_CD_FAULT_AC_POWER_SHORT:
			userlog(DEBUG_LOG, psName, "Power fail : short %d\n", val);
			break;
		case M_CD_FAULT_AC_POWER_LONG:
			userlog(DEBUG_LOG, psName, "Power fail : long %d\n", val);
			break;
		case M_CD_FAULT_UPS_BATTERY:
			userlog(DEBUG_LOG, psName, "UPS battery fail %d\n", val);
			break;
		case M_CD_FAULT_MAIN_EMG_SWITCH:
			userlog(DEBUG_LOG, psName, "Main emergency button %d\n", val);
			break;
		case M_CD_FAULT_SUB_EMG_SWITCH:
			userlog(DEBUG_LOG, psName, "Sub emergency button %d\n", val);
			break;
		case M_CD_FAULT_CONTROL_PS:
			userlog(DEBUG_LOG, psName, "Control PS fail %d\n", val);
			break;
		case M_CD_FAULT_OT:
			userlog(DEBUG_LOG, psName, "Module OT %d\n", val);
			break;
		case M_CD_FAULT_WARNING_POWER_OFF:
			userlog(DEBUG_LOG, psName, "Warning power off %d\n", val);
			break;
		case M_CD_FAULT_NORMAL_POWER_OFF:
			userlog(DEBUG_LOG, psName, "Normal power off %d\n", val);
			break;
		case M_CD_FAULT_FORCE_POWER_OFF:
			userlog(DEBUG_LOG, psName, "Force power off %d\n", val);
			break;
		case M_CD_FAULT_NORMAL_TERMINAL_QUIT:
			userlog(DEBUG_LOG, psName,
				"Terminal key input : normal quit %d\n", val);
			break;
		case M_CD_FAULT_FORCE_TERMINAL_QUIT:
			userlog(DEBUG_LOG, psName,
				"Terminal key input : force quit %d\n", val);
			break;
		case M_CD_FAULT_NORMAL_TERMINAL_HALT:
			userlog(DEBUG_LOG, psName,
				"Terminal key input : normal halt %d\n", val);
			break;
		case M_CD_FAULT_FORCE_TERMINAL_HALT:
			userlog(DEBUG_LOG, psName,
				"Terminal key input : force halt %d\n", val);
			break;
		case M_CD_FAULT_CPU_WATCHDOG:
			userlog(DEBUG_LOG, psName, "CPU Watchdog detect %d\n", val);
			break;
		case M_CD_FAULT_CALI_METER_COMM_ERROR:
			userlog(DEBUG_LOG, psName, "Calibration DMM Comm Error %d\n", val);
			break;
		case M_CD_FAULT_CALIBRATOR_COMM_ERROR:
			userlog(DEBUG_LOG, psName, "Calibrator Comm Error %d\n", val);
			break;
		case M_CD_FAULT_FUSE:
			userlog(DEBUG_LOG, psName, "Fuse disconnect %d\n", val);
			break;
		case M_CD_FAULT_MCCB: //lki_111010
			userlog(DEBUG_LOG, psName, "MCCB disconnect %d\n", val);
			break;
		case M_CD_FAULT_UPPER_VOLTAGE:
			userlog(DEBUG_LOG, psName, "Main power over voltage %d\n", val);
			break;
		case M_CD_FAULT_LOWER_VOLTAGE:
			userlog(DEBUG_LOG, psName, "Main power under voltage %d\n", val);
			break;
		case M_CD_FAULT_MAIN_PS:
			userlog(DEBUG_LOG, psName, "Main power fail %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER:
			userlog(DEBUG_LOG, psName, "Chamber fail %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_FIRE:
			userlog(DEBUG_LOG, psName, "Chamber fire detect fail %d\n", val);
			break;
		case M_CD_FAULT_FAN:
			userlog(DEBUG_LOG, psName, "Fan fail %d\n", val);
			break;
		case M_CD_FAULT_LOAD_LINE:
			userlog(DEBUG_LOG, psName, "LoadLine fail %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_DOOR_OPEN:
			userlog(DEBUG_LOG, psName, "Chamber door open %d\n", val);
			break;
		case M_CD_FAULT_JOB_CHANGE:
			userlog(DEBUG_LOG, psName, "Job change fail %d\n", val);
			break;
		case M_CD_FAULT_PITCH_CHANGE:
			userlog(DEBUG_LOG, psName, "Pitch change fail %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_AREA_SENSOR:
			userlog(DEBUG_LOG, psName, "Chamber area sensor fail %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_MANUAL_DOOR:
			userlog(DEBUG_LOG, psName, "Chamber manual door fail %d\n", val);
			break;
		//hun_160822
		case M_CD_FAULT_CHAMBER_MANUAL_FIRE:
			userlog(DEBUG_LOG, psName, "Chamber manual FIRE fail %d\n", val);
			break;
		case M_CD_FAULT_PLC_NFB_TRIP:		//hun_150420
			userlog(DEBUG_LOG, psName, "Chamber NFB TRIP %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_AIR:
			userlog(DEBUG_LOG, psName, "Chamber air fail %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_APR_S:
			userlog(DEBUG_LOG, psName, "Chamber power connect error %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_REF:
			userlog(DEBUG_LOG, psName, "Chamber ref. fail %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_COIL_TRIP:
			userlog(DEBUG_LOG, psName, "Chamber emg %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_OVER_TEMP:
			userlog(DEBUG_LOG, psName, "Chamber over temperature %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_FAN:
			userlog(DEBUG_LOG, psName, "Chamber fan fail %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_SMOKE:
			userlog(DEBUG_LOG, psName, "Chamber smoke detect %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_POWER:
			userlog(DEBUG_LOG, psName, "Chamber power fail %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_LEAK: //lki_111010
			userlog(DEBUG_LOG, psName, "Chamber leak fail %d\n", val);
			break;
		case M_CD_FAULT_CHAMBER_CP_TRIP: //lki_111010
			userlog(DEBUG_LOG, psName, "Chamber cp fail %d\n", val);
			break;
		case M_CD_FAULT_USER_EMG: //kjg_131002
			userlog(DEBUG_LOG, psName, "User emg. %d\n", val);
			break;
		case M_CD_FAULT_EXT_OT: //jhkw_160219
			userlog(DEBUG_LOG, psName, "External OT detect %d\n", val);
			break;
		case M_CD_FAULT_OC: //kjh_131219
			userlog(DEBUG_LOG, psName, "Inverter_OC_Fault %d\n", val);
			break;
		case M_CD_FAULT_ETC: //kjh_131219
			userlog(DEBUG_LOG, psName, "Inverter_ETC_Fault %d\n", val);
			break;
		case M_CD_FAULT_LEAKAGE_CURRENT: //ktg_211104
			userlog(DEBUG_LOG, psName, "leakage_current detect %d, %ld\n", val , myData->cData[val-1].misc2.leakageI);
			myData->cData[val-1].misc2.leakageI = 0;
			break;
		case M_CD_FAULT_DC_LINK_FUSE: //phb_230717
			userlog(DEBUG_LOG, psName, "DC_Link_Fuse_disconnect %d\n", val);
			break;

		case G_CD_FAULT_UPPER_VOLTAGE:
			userlog(DEBUG_LOG, psName, "Upper Voltage %d\n", val);
			break;
		case G_CD_FAULT_UPPER_CURRENT:
			userlog(DEBUG_LOG, psName, "Upper Current %d\n", val);
			break;
		case G_CD_FAULT_RUN_TIME_OVER:
			userlog(DEBUG_LOG, psName, "Run time over %d\n", val);
			break;
		case G_CD_FAULT_DATASAVE_PROCESS_ERROR:
			userlog(DEBUG_LOG, psName, "DataSave process error %d\n", val);
			break;
		case G_CD_FAULT_ADC:
			userlog(DEBUG_LOG, psName, "Group ADC Error %d\n", val);
			break;
		case G_CD_FAULT_OT:
			userlog(DEBUG_LOG, psName, "Group OT %d\n", val);
			break;

		case G_CD_FAULT_JIG_ACTIVE_ERROR:
			userlog(DEBUG_LOG, psName, "Jig Active %d\n", val);
			break;
		case G_CD_FAULT_JIG_TRAY_ERROR:
			userlog(DEBUG_LOG, psName, "Jig Tray %d\n", val);
			break;
		case G_CD_FAULT_JIG_SMOKE_ERROR:
			userlog(DEBUG_LOG, psName, "Jig Smoke/Fire %d\n", val);
			break;
		case G_CD_FAULT_JIG_TEMP_ERROR:
			userlog(DEBUG_LOG, psName, "Jig Temperature %d\n", val);
			break;
		case G_CD_FAULT_JIG_GAS_ERROR:
			userlog(DEBUG_LOG, psName, "Jig Gas %d\n", val);
			break;
		case G_CD_FAULT_JIG_DOOR_ERROR:
			userlog(DEBUG_LOG, psName, "Jig Door %d\n", val);
			break;
		case G_CD_FAULT_JIG_AIR_PRESS_ERROR:
			userlog(DEBUG_LOG, psName, "Jig Air Press %d\n", val);
			break;
		case G_CD_FAULT_JIG_STACKER_ERROR:
			userlog(DEBUG_LOG, psName, "Jig Stacker %d\n", val);
			break;
		case M_CD_FAULT_FRAME_DOOR_OPEN:
			userlog(DEBUG_LOG, psName, "Frame Door Open %d\n", val);
			break;
		case B_CD_FAULT_OT:
			userlog(DEBUG_LOG, psName, "Board OT %d\n", val);
			break;
		case B_CD_FAULT_ADC:
			userlog(DEBUG_LOG, psName, "Board ADC Error %d\n", val);
			break;
		default:
			userlog(DEBUG_LOG, psName, "Unknown exit message : %d %d\n",
				myData->mData.code, val);
			break;
	}
}

void CloseAppControl(void)
{
	int phase, i;
	S_MSG_VAL SendMsg;

	phase = Write_CanReceiveSetData();
	if(phase < 0) phase = Write_CanReceiveSetData();
	phase = Write_CanTransmitSetData();
	if(phase < 0) phase = Write_CanTransmitSetData();

	phase = 0; //kjg_140916_s
	for(i=0; i < MAX_CH_256; i++) {
		if(myData->cData[i].signal[C_SIG_EXT_OP_TYPE] == P1) { //E_CAN_2
			phase++;
		}
	}
	if(phase == 0) Save_SystemMemory(); //kjg_140916_e

	if(myPs->signal[APP_SIG_QUIT_TYPE] == 2) {
		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		SendMsg.msg = MSG_APP_IO_SHUTDOWN;
		send_msg(APP_TO_IO, (char *)&SendMsg);
		usleep(10000);
	}

	Close_Process_All();

	userlog(DEBUG_LOG, psName, "S_SYSTEM_DATA size %d\n",
		sizeof(S_SYSTEM_DATA));
	userlog(DEBUG_LOG, psName, "runningTime[0] max:%ld[%ld], min:%ld[%ld]\n",
		myData->mData.runningTime[2][0], myData->mData.runningTime[2][1],
		myData->mData.runningTime[3][0], myData->mData.runningTime[3][1]);
	userlog(DEBUG_LOG, psName, "runningTime[1] max:%ld[%ld], min:%ld[%ld]\n",
		myData->mData.runningTime[4][0], myData->mData.runningTime[4][1],
		myData->mData.runningTime[5][0], myData->mData.runningTime[5][1]);

	if(myPs->signal[APP_SIG_APP_CONTROL_PROCESS] == P10) {
		userlog(DEBUG_LOG, psName, "system shutdown\n");
		phase = 1;
	} else {
		userlog(DEBUG_LOG, psName, "system quit\n");
		myData->mData.Auto_Update_rdy = 1;
		phase = 2;
	}

	Close_SystemMemory();
	system("rmmod mbuff");

	system("update");
	sleep(1);

	if(phase == 1) system("shutdown -h now");
}

void IO_Print_log(void)
{
	int	i;

	userlog(DEBUG_LOG, psName, "in_address\n");
	userlog(DEBUG_LOG, psName,
		"\n%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
		myData->dio.function_set.in_address[0],
		myData->dio.function_set.in_address[1],
		myData->dio.function_set.in_address[2],
		myData->dio.function_set.in_address[3],
		myData->dio.function_set.in_address[4],
		myData->dio.function_set.in_address[5],
		myData->dio.function_set.in_address[6],
		myData->dio.function_set.in_address[7],
		myData->dio.function_set.in_address[8],
		myData->dio.function_set.in_address[9],
		myData->dio.function_set.in_address[10],
		myData->dio.function_set.in_address[11],
		myData->dio.function_set.in_address[12],
		myData->dio.function_set.in_address[13],
		myData->dio.function_set.in_address[14],
		myData->dio.function_set.in_address[15],
		myData->dio.function_set.in_address[16],
		myData->dio.function_set.in_address[17],
		myData->dio.function_set.in_address[18],
		myData->dio.function_set.in_address[19],
		myData->dio.function_set.in_address[20],
		myData->dio.function_set.in_address[21],
		myData->dio.function_set.in_address[22],
		myData->dio.function_set.in_address[23],
		myData->dio.function_set.in_address[24],
		myData->dio.function_set.in_address[25],
		myData->dio.function_set.in_address[26],
		myData->dio.function_set.in_address[27],
		myData->dio.function_set.in_address[28],
		myData->dio.function_set.in_address[29],
		myData->dio.function_set.in_address[30],
		myData->dio.function_set.in_address[31]);

	userlog(DEBUG_LOG, psName, "in_byte\n");
	for(i=0; i < 4; i++) {
		userlog(DEBUG_LOG, psName,
			"\n%03x:%02x %03x:%02x %03x:%02x %03x:%02x %03x:%02x %03x:%02x %03x:%02x %03x:%02x\n",
			myData->dio.function_set.in_address[i*8+0],
			myData->dio.in.bytes[i*8+0],
			myData->dio.function_set.in_address[i*8+1],
			myData->dio.in.bytes[i*8+1],
			myData->dio.function_set.in_address[i*8+2],
			myData->dio.in.bytes[i*8+2],
			myData->dio.function_set.in_address[i*8+3],
			myData->dio.in.bytes[i*8+3],
			myData->dio.function_set.in_address[i*8+4],
			myData->dio.in.bytes[i*8+4],
			myData->dio.function_set.in_address[i*8+5],
			myData->dio.in.bytes[i*8+5],
			myData->dio.function_set.in_address[i*8+6],
			myData->dio.in.bytes[i*8+6],
			myData->dio.function_set.in_address[i*8+7],
			myData->dio.in.bytes[i*8+7]);
	}
	userlog(DEBUG_LOG, psName, "out_address\n");
	userlog(DEBUG_LOG, psName,
		"\n%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
		myData->dio.function_set.out_address[0],
		myData->dio.function_set.out_address[1],
		myData->dio.function_set.out_address[2],
		myData->dio.function_set.out_address[3],
		myData->dio.function_set.out_address[4],
		myData->dio.function_set.out_address[5],
		myData->dio.function_set.out_address[6],
		myData->dio.function_set.out_address[7],
		myData->dio.function_set.out_address[8],
		myData->dio.function_set.out_address[9],
		myData->dio.function_set.out_address[10],
		myData->dio.function_set.out_address[11],
		myData->dio.function_set.out_address[12],
		myData->dio.function_set.out_address[13],
		myData->dio.function_set.out_address[14],
		myData->dio.function_set.out_address[15],
		myData->dio.function_set.out_address[16],
		myData->dio.function_set.out_address[17],
		myData->dio.function_set.out_address[18],
		myData->dio.function_set.out_address[19],
		myData->dio.function_set.out_address[20],
		myData->dio.function_set.out_address[21],
		myData->dio.function_set.out_address[22],
		myData->dio.function_set.out_address[23],
		myData->dio.function_set.out_address[24],
		myData->dio.function_set.out_address[25],
		myData->dio.function_set.out_address[26],
		myData->dio.function_set.out_address[27],
		myData->dio.function_set.out_address[28],
		myData->dio.function_set.out_address[29],
		myData->dio.function_set.out_address[30],
		myData->dio.function_set.out_address[31]);
	for(i=0; i < 4; i++) {
		userlog(DEBUG_LOG, psName,
			"\n%03x:%02x %03x:%02x %03x:%02x %03x:%02x %03x:%02x %03x:%02x %03x:%02x %03x:%02x\n",
			myData->dio.function_set.out_address[i*8+0],
			myData->dio.out.bytes[i*8+0],
			myData->dio.function_set.out_address[i*8+1],
			myData->dio.out.bytes[i*8+1],
			myData->dio.function_set.out_address[i*8+2],
			myData->dio.out.bytes[i*8+2],
			myData->dio.function_set.out_address[i*8+3],
			myData->dio.out.bytes[i*8+3],
			myData->dio.function_set.out_address[i*8+4],
			myData->dio.out.bytes[i*8+4],
			myData->dio.function_set.out_address[i*8+5],
			myData->dio.out.bytes[i*8+5],
			myData->dio.function_set.out_address[i*8+6],
			myData->dio.out.bytes[i*8+6],
			myData->dio.function_set.out_address[i*8+7],
			myData->dio.out.bytes[i*8+7]);
	}
}
