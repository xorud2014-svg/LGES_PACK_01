#include <pthread.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <linux/slab.h>
#include <linux/time.h>
//#include "../rt_can/rt_can_ext.h"	//kjg_181121
//#include "rt_can_ext.h"	//kjg_181121
#ifdef __CAN_FD__			//csk_190617s
#include "rt_can_ext.h"	//kjg_181121
#else		//can2.0
#include "../rt_can/rt_can.h"	//kjg_181121
#endif						//csk_190617e
#include "../rt_com/rt_com.h"
#include "../../INC/datastore.h"
#include "local_message.h"
#include "InOutControl.h"
#include "local_utils.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;
extern S_DIO *myDio;
extern S_GROUP_DATA *myGroup;
extern unsigned char *dma_buff;

struct tm
{ //kjg_121106_s
	int tm_sec;		//Seconds		[0-59] (1 leap second)
	int tm_min;		//Minutes		[0-59]
	int tm_hour;	//Hours			[0-23]
	int tm_mday;	//Day			[1-31]
	int tm_mon;		//Month			[0-11]
	int tm_year;	//Year-1900
	int tm_wday;	//Day of week	[0-6]
	int tm_yday;	//Days in year	[0-364]
	int tm_isdst;	//DST			[-1/0/1]

	long int tm_gmtoff;		//we don't care, we count from GMT
	const char *tm_zone;	//we don't care, we count from GMT
};

#define SPD 24*60*60 //kjg_121106_e

//kjg_240213 void Initialize(void)
int Initialize(void)
{
	int i;

	myPs = &(myData->mData);
	myDio = &(myData->dio);

	Init_SystemMemory_1();

	if(myData->dio.config.logic_type == 1) { //kjg_logic_type_140324
		//outb(0x01, 0x604);
		Select_OutPoint(0, 0, O_LOGIC_TYPE, ON); //kjhw_180213
	}

	if(myData->dio.config.watchdogType == 1) { //kjg_logic_type_140324
		CheckWDT();
		EnableWDT();
	}

	Init_DMA_DAQ();

	//kjg_240213 Init_RT_CAN();
	myData->CAN.rt_can_ch_phase = 0;
	for(i=0; i < myData->mData.config.installedCAN; i++) {
		myData->CAN.rt_can_control_phase[i] = 0;
	}
	if(myData->AppControl.misc.Load_Process_CAN[0] == P1) {
#ifdef __CAN_FD__ //kjh_190706s
		if(init_rt_can() < 0) return -1;
#else //CAN_2p0b
		 Init_RT_CAN();
#endif //kjh_190706e
	}

	Init_RT_COM();

	myData->AppControl.signal[APP_SIG_MODULE_CONTROL_PROCESS] = P1;

	return 0;
}

void Init_SystemMemory_1(void)
{
	S_MSG_VAL SendMsg;

	myData->mData.misc.processPointer = (int)&myData;

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	SendMsg.msg = MSG_MODULE_IO_RUN_LED;
	SendMsg.val[0] = ON;
	send_msg(MODULE_TO_IO, (char *)&SendMsg);

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	SendMsg.msg = MSG_MODULE_IO_REMOTE_PS;
	SendMsg.val[0] = ON;
	send_msg(MODULE_TO_IO, (char *)&SendMsg);
}

void Init_DMA_DAQ(void)
{
	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1) return;

	if(myPs->config.daq_type == DAQ_TYPE1) {
		dma_buff = kmalloc(256, GFP_DMA);
	} else {
		dma_buff = kmalloc(512, GFP_DMA);
	}
	request_dma(1, "KJG_DMA");
}

void Close_DMA_DAQ(void)
{
	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1) return;

	kfree(dma_buff);
	free_dma(1);
}

		//csk_240301
#ifdef __CAN_FD__ //kjh_190706s //ksh_241112
/*void Init_RT_CAN(void)
{
	int i, rtn;

	if(myData->AppControl.misc.Load_Process_CAN[0] != P1) return;

	for(i=0; i < myPs->config.installedCAN; i++) {
		rtn = rt_can_setup(i, 2); //kjg_180405
		rtl_printf("rt_can_setup open ch:%d, rtn:%d\n", i, rtn);
	}
}

void Close_RT_CAN(void)
{
	int i, rtn;

	if(myData->AppControl.misc.Load_Process_CAN[0] != P1) return;

	for(i=0; i < myPs->config.installedCAN; i++) {
		rtn = rt_can_setup(i, -2); //kjg_180405
		rtl_printf("rt_can_setup close ch:%d, rtn:%d\n", i, rtn);
	}
}*/
#else //CAN_2p0b
void Init_RT_CAN(void)
{
	int i, bps, sjw;

	if(myData->AppControl.misc.Load_Process_CAN[0] != P1) return;

	for(i=0; i < myPs->config.installedCAN; i++) {
		switch(myData->canReceiveSetData.commonData[i/2][i%2].can_baudrate) {
			case 0: //125K
				bps = 125000; 	break;
			case 1: //250K
				bps = 250000;	break;
			case 2: //500K
				bps = 500000;	break;
			case 3: //1M
				bps = 1000000;	break;
			default: //User
				bps = 0;		break;
		}
		sjw = (int)myData->canReceiveSetData.commonData[i/2][i%2].sjw;
		//kjg_w rt_can_setup(i, bps, sjw, (int)myData->canReceiveSetData
		//	.commonData[i/2][i%2].extended_id);
		//rt_can_set_param(i, bps, sjw, (int)myData->canReceiveSetData.commonData[i/2][i%2].extended_id, 0);
		rt_can_set_param(i, bps, sjw, (int)myData->canReceiveSetData.commonData[i/2][i%2].extended_id, 0,
		0, 0, 0, 0, 0); //ksh_241112
	}
}

void Close_RT_CAN(void)
{
	int i;

	if(myData->AppControl.misc.Load_Process_CAN[0] != P1) return;

	for(i=0; i < myPs->config.installedCAN; i++) {
		//rt_can_setup(i, -1, 0, 0);
		rt_can_setup(i, -1, 0, 0, 0, 0, 0, 0, 0); //ksh_241112
	}
}
#endif //kjh_190706e
		//csk_240301
		//ksh_241112

void Init_RT_COM(void)
{
	int i, bps, hwComPort, comPort2;

	if(myData->AppControl.misc.Load_Process_COM[0] != P1) return;

	//for(i=0; i < myPs->config.installedCOM; i++) {
	for(i=0; i < MAX_COM_PORT; i++) {
		if(myData->COM.config.functionType[i] == COM_FUNC_TYPE_NONE) {
			continue;
		} else if(myData->COM.config.functionType[i]
			== COM_FUNC_TYPE_DISPLAY1) {
			if(myData->AppControl.misc
				.Load_Process_COM[PROCESS_COM_CALI_METER] == P1) {
				continue;
			}
		} else if(myData->COM.config.functionType[i]
			== COM_FUNC_TYPE_METER1) {
			if(myData->AppControl.misc
				.Load_Process_COM[PROCESS_COM_CALI_METER] == P0) {
				continue;
			}
		}
		if(myData->COM.com_port[i].misc.externPort_useFlag == P0) {
			comPort2 = i;
		} else {
			comPort2 = myData->COM.config.externPort[i] - 1;
		}
		hwComPort = myData->COM.config.comPortId[comPort2] - 1;
		bps = myData->COM.config.comBps[comPort2];
		switch(myData->COM.config.commType[comPort2]) {
			case 0: //none, 1stop, 8data
				rt_com_setup(hwComPort, bps, RT_COM_PARITY_NONE, 1, 8);
				break;
			case 1: //even, 2stop, 7data
				rt_com_setup(hwComPort, bps, RT_COM_PARITY_EVEN, 2, 7);
				break;
			case 2: //even, 1stop, 7data
				rt_com_setup(hwComPort, bps, RT_COM_PARITY_EVEN, 1, 7);
				break;
			default:	break;
		}
		//kjg_w rt_com_setup(i, 230400, RT_COM_PARITY_NONE, 1, 8);
	}
}

void Close_RT_COM(void)
{
	int i, hwComPort;

	if(myData->AppControl.misc.Load_Process_COM[0] != P1) return;

	//for(i=0; i < myPs->config.installedCOM; i++) {
	for(i=0; i < MAX_COM_PORT; i++) {
		if(myData->COM.config.functionType[i] == COM_FUNC_TYPE_NONE) {
			continue;
		} else if(myData->COM.config.functionType[i]
			== COM_FUNC_TYPE_DISPLAY1) {
			if(myData->AppControl.misc
				.Load_Process_COM[PROCESS_COM_CALI_METER] == P1) {
				continue;
			}
		} else if(myData->COM.config.functionType[i]
			== COM_FUNC_TYPE_METER1) {
			if(myData->AppControl.misc
				.Load_Process_COM[PROCESS_COM_CALI_METER] == P0) {
				continue;
			}
		}
		hwComPort = myData->COM.config.comPortId[i] - 1;
		rt_com_setup(hwComPort, -1, 0, 0, 0);
	}
}

void timer_1sec_increment(void)
{
	int count;

	count = 1000 / (int)myPs->config.scan_period;
	myPs->misc.timer_1sec_count++;
	if((int)myPs->misc.timer_1sec_count >= count) {
		myPs->misc.timer_1sec_count = 0;
		myPs->misc.timer_1sec++;
	}

	myPs->misc.timer_1000ms_count++;
	count = (int)myPs->misc.timer_1000ms_count * (int)myPs->config.scan_period;
	if(count >= 1000) {
		myPs->misc.timer_1000ms_count = 0;
		myPs->misc.timer_1000ms = 0;
	} else {
		myPs->misc.timer_1000ms = (long)count;
	}
}

void sub_sens_v_count_increment(void)
{
	if(myPs->misc.timer_1sec < 3) return; //kjg_100129 1 -> 3

	myPs->misc.SubSensV_SourceSensCount++;
	if(myPs->misc.SubSensV_SourceSensCount
		>= MAX_SUB_SENS_V_SOURCE_SENS_COUNT) {
		myPs->misc.SubSensV_SourceSensCount = 0;
		myPs->misc.SubSensV_SourceSensCountFlag = P1;
	}

	myPs->misc.SubSensV_ChSensCount++;
	if(myPs->misc.SubSensV_ChSensCountFlag == P0) {
		if(myPs->misc.SubSensV_ChSensCount
			>= MAX_SUB_SENS_V_FILTER_AD_COUNT) {
			myPs->misc.SubSensV_ChSensCount = 0;
			myPs->misc.SubSensV_ChSensCountFlag = P1;
		}
	} else if(myPs->misc.SubSensV_ChSensCountFlag == P1) {
		if(myPs->misc.SubSensV_ChSensCount
			>= MAX_SUB_SENS_V_FILTER_AD_COUNT) {
			myPs->misc.SubSensV_ChSensCount = 0;
		}
	} else {
		myPs->misc.SubSensV_ChSensCount = 0;
		myPs->misc.SubSensV_ChSensCountFlag = P0;
	}
}

int ch_sens_count_increment(int ch)
{
	int cnt_end, filter_ad_count;

	filter_ad_count = (int)myPs->config.filter_ad_count;

	if(myData->cData[ch].misc.sensCountFlag == P2) {
		myData->cData[ch].misc.sensCount = 0;
		myData->cData[ch].misc.sensCountFlag = P0;
	}

	myData->cData[ch].misc.tmp_sensCount = myData->cData[ch].misc.sensCount;
	cnt_end = myData->cData[ch].misc.sensCount;
	if(myData->cData[ch].misc.sensCountFlag == P0) {
		cnt_end = cnt_end + 1;
	} else {
		cnt_end = filter_ad_count;
	}

	myData->cData[ch].misc.sensCount++;
	if(myData->cData[ch].misc.sensCount >= filter_ad_count) { //kjg_100103
		myData->cData[ch].misc.sensCount = 0;
		myData->cData[ch].misc.sensCountFlag = P1;
	}

	return cnt_end;
}

int bd_sens_count_increment(void)
{
	int cnt_end, cnt, bd, filter_ad_count;

	filter_ad_count = (int)myPs->config.filter_ad_count;

	cnt_end = myPs->misc.bd_sensCount;
	if(myPs->misc.bd_sensCountFlag == P0) {
		cnt_end = cnt_end + 1;
	} else {
		cnt_end = filter_ad_count;
	}

	myPs->misc.tmp_ref_muxVal = myPs->misc.ref_muxVal;
	myPs->misc.ref_muxVal++;
	if(myPs->misc.ref_muxVal >= 4) {
		myPs->misc.ref_muxVal = 0;

		myPs->misc.bd_sensCount++;
		if(myPs->misc.bd_sensCount >= filter_ad_count) {
			myPs->misc.bd_sensCount = 0;
			myPs->misc.bd_sensCountFlag = P1;
		}

		if(myPs->misc.bd_sensCountFlag == P2) {
			myPs->misc.bd_sensCount = 0;
			myPs->misc.bd_sensCountFlag = P0;

			myPs->misc.source_sensCount = 0;
			myPs->misc.source_sensCountFlag = P0;

			for(bd=0; bd < MAX_BD_16; bd++) {
				for(cnt=0; cnt < 4; cnt++) {
					myData->bData[bd].misc.source2[cnt].totalV = 0.0;
					myData->bData[bd].misc.source2[cnt].totalI = 0.0;
				}
			}
			return cnt_end;
		}

		if(myPs->misc.timer_1sec >= 1) { //after 1sec
			myPs->misc.source_sensCount++;
			if(myPs->misc.source_sensCount >= MAX_SOURCE_SENS_COUNT) {
				myPs->misc.source_sensCount = 0;
				myPs->misc.source_sensCountFlag = P1;
			}
		}
	}

	return cnt_end;
}

int ch_sens_count_increment_2(int ch)
{ //kjg_logic_type_140325
	int cnt_end, filter_ad_count;

	filter_ad_count = (int)myPs->config.filter_ad_count;

	if(myData->cData[ch].misc.sensCountFlag == P2) {
		myData->cData[ch].misc.sensCount = 0;
		myData->cData[ch].misc.sensCountFlag = P0;
	}

	myData->cData[ch].misc.tmp_sensCount = myData->cData[ch].misc.sensCount;
	cnt_end = myData->cData[ch].misc.sensCount;
	if(myData->cData[ch].misc.sensCountFlag == P0) {
		cnt_end = cnt_end + 4;
	} else {
		cnt_end = filter_ad_count;
	}

	myData->cData[ch].misc.sensCount += 4;
	if(myData->cData[ch].misc.sensCount >= filter_ad_count) { //kjg_100103
		myData->cData[ch].misc.sensCount = 0;
		myData->cData[ch].misc.sensCountFlag = P1;
	}

	return cnt_end;
}

void bd_sens_count_increment_2(void)
{ //kjg_logic_type_140325
	int filter_ad_count;

	if(myPs->misc.bd_sensCountFlag == P0) {
		myPs->misc.bd_sensCount = 0;
		myPs->misc.bd_sensCountFlag = P1;
		return;
	}

	filter_ad_count = (int)myPs->config.filter_ad_count;

	myPs->misc.bd_sensCount += 4;
	if(myPs->misc.bd_sensCount >= filter_ad_count) {
		myPs->misc.bd_sensCount = 0;
		myPs->misc.bd_sensCountFlag = P2;
	}

	myPs->misc.source_sensCount++;
	if(myPs->misc.source_sensCount >= MAX_SOURCE_SENS_COUNT) {
		myPs->misc.source_sensCount = 0;
		myPs->misc.source_sensCountFlag = P1;
	}
}

void module_runningTime(int index)
{ //kjg_240213
	long val1, val2;

	if(myPs->misc.timer_1sec <= 4) return;

	val1 = myPs->misc.hrt[1] - myPs->misc.hrt[0];
	val2 = myPs->misc.hrt[2] - myPs->misc.hrt[0];

	myPs->runningTime[0][index] = val1;
	myPs->runningTime[1][index] = val2;

	if(myPs->runningTime[0][index] > myPs->runningTime[2][0]) {
		myPs->runningTime[2][0] = myPs->runningTime[0][index];
		myPs->runningTime[2][1] = (long)index;
	}

	if(myPs->runningTime[0][index] < myPs->runningTime[3][0]) {
		myPs->runningTime[3][0] = myPs->runningTime[0][index];
		myPs->runningTime[3][1] = (long)index;
	}

	if(myPs->runningTime[1][index] > myPs->runningTime[4][0]) {
		myPs->runningTime[4][0] = myPs->runningTime[1][index];
		myPs->runningTime[4][1] = (long)index;
	}

	if(myPs->runningTime[1][index] < myPs->runningTime[5][0]) {
		myPs->runningTime[5][0] = myPs->runningTime[1][index];
		myPs->runningTime[5][1] = (long)index;
	}
}
//kjhw_140520s
void module_runningTime1(int index, long val1, long val2, long val3, long val4, long val5, long val6, long val7)
{
	if(myPs->misc.timer_1sec <= 4) return;

	myPs->runningTime[0][20] = val1;
	if(myPs->runningTime[0][20] > myPs->runningTime[0][21]) {
		myPs->runningTime[0][21] = myPs->runningTime[0][20];
		myPs->runningTime[0][22] = (long)index;
	}
	myPs->runningTime[0][30] = val2;
	if(myPs->runningTime[0][30] > myPs->runningTime[0][31]) {
		myPs->runningTime[0][31] = myPs->runningTime[0][30];
		myPs->runningTime[0][32] = (long)index;
	}
	myPs->runningTime[0][40] = val3;
	if(myPs->runningTime[0][40] > myPs->runningTime[0][41]) {
		myPs->runningTime[0][41] = myPs->runningTime[0][40];
		myPs->runningTime[0][42] = (long)index;
	}
	myPs->runningTime[0][50] = val4;
	if(myPs->runningTime[0][50] > myPs->runningTime[0][51]) {
		myPs->runningTime[0][51] = myPs->runningTime[0][50];
		myPs->runningTime[0][52] = (long)index;
	}
	myPs->runningTime[0][60] = val5;
	if(myPs->runningTime[0][60] > myPs->runningTime[0][61]) {
		myPs->runningTime[0][61] = myPs->runningTime[0][60];
		myPs->runningTime[0][62] = (long)index;
	}
	myPs->runningTime[0][70] = val6;
	if(myPs->runningTime[0][70] > myPs->runningTime[0][71]) {
		myPs->runningTime[0][71] = myPs->runningTime[0][70];
		myPs->runningTime[0][72] = (long)index;
	}
	myPs->runningTime[0][80] = val7;
	if(myPs->runningTime[0][80] > myPs->runningTime[0][81]) {
		myPs->runningTime[0][81] = myPs->runningTime[0][80];
		myPs->runningTime[0][82] = (long)index;
	}
}

void module_runningTime2(int index)
{ //kjg_240213
	long val1, val2;

	if(myPs->misc.timer_1sec <= 3) return;

	val1 = myPs->misc.hrt[1] - myPs->misc.hrt[0];
	val2 = myPs->misc.hrt[2] - myPs->misc.hrt[0];

	myPs->runningTime[0][index] = val1;
	myPs->runningTime[1][index] = val2;
}

void module_runningTime3(void)
{ //kjg_240213
	int i, max1_index, min1_index, max2_index, min2_index;
	long val1, val2, val1_max, val1_min, val2_max, val2_min;

	if(myPs->runningTime[2][11] == 0 && myPs->misc.timer_1sec > 4) {
		myPs->runningTime[2][11] = 1;

		myPs->runningTime[5][0] = 0;
		myPs->runningTime[5][1] = 0;
		myPs->runningTime[5][2] = 90000000;
		myPs->runningTime[5][3] = 0;

		myPs->runningTime[5][5] = 0;
		myPs->runningTime[5][6] = 0;
		myPs->runningTime[5][7] = 90000000;
		myPs->runningTime[5][8] = 0;
	}

	if(myPs->misc.main_slot != 0) return;
	if(myPs->misc.timer_1sec <= 4) return;

	val1 = myPs->runningTime[0][0];
	val1_max = val1_min = val1;
	max1_index = min1_index = 0;

	val2 = myPs->runningTime[1][0];
	val2_max = val2_min = val2;
	max2_index = min2_index = 0;

	//for(i=1; i < myPs->config.scan_period; i++) { //kjgw_1ms
	for(i=1; i < 10; i++) { //kjgw_1ms	//csk_240523d
		val1 += myPs->runningTime[0][i];
		if(myPs->runningTime[0][i] > val1_max) {
			val1_max = myPs->runningTime[0][i];
			max1_index = i;
		}
		if(myPs->runningTime[0][i] < val1_min) {
			val1_min = myPs->runningTime[0][i];
			min1_index = i;
		}

		val2 += myPs->runningTime[1][i];
		if(myPs->runningTime[1][i] > val2_max) {
			val2_max = myPs->runningTime[1][i];
			max2_index = i;
		}
		if(myPs->runningTime[1][i] < val2_min) {
			val2_min = myPs->runningTime[1][i];
			min2_index = i;
		}
	}

	myPs->runningTime[2][0] = val1_max;
	myPs->runningTime[2][1] = max1_index;
	myPs->runningTime[2][2] = val1_min;
	myPs->runningTime[2][3] = min1_index;
	myPs->runningTime[2][4] = val1 / (long)myPs->config.scan_period;

	myPs->runningTime[2][5] = val2_max;
	myPs->runningTime[2][6] = max2_index;
	myPs->runningTime[2][7] = val2_min;
	myPs->runningTime[2][8] = min2_index;
	myPs->runningTime[2][9] = val2 / (long)myPs->config.scan_period;

	i = myPs->runningTime[2][10];
	myPs->runningTime[2][10]++;
	if(myPs->runningTime[2][10] >= 100) myPs->runningTime[2][10] = 0;

	myPs->runningTime[3][i] = myPs->runningTime[2][4];
	myPs->runningTime[4][i] = myPs->runningTime[2][9];

	if(i == 99) {
		val1 = myPs->runningTime[3][0];
		val2 = myPs->runningTime[4][0];
		
		for(i=1; i < 100; i++) {
			val1 += myPs->runningTime[3][i];
			val2 += myPs->runningTime[4][i];
		}

		myPs->runningTime[5][4] = val1 / 100;
		myPs->runningTime[5][9] = val2 / 100;
	}

	if(myPs->runningTime[2][0] > myPs->runningTime[5][0]) {
		myPs->runningTime[5][0] = myPs->runningTime[2][0];
		myPs->runningTime[5][1] = myPs->runningTime[2][1];
	}
	if(myPs->runningTime[2][2] < myPs->runningTime[5][2]) {
		myPs->runningTime[5][2] = myPs->runningTime[2][2];
		myPs->runningTime[5][3] = myPs->runningTime[2][3];
	}

	if(myPs->runningTime[2][5] > myPs->runningTime[5][5]) {
		myPs->runningTime[5][5] = myPs->runningTime[2][5];
		myPs->runningTime[5][6] = myPs->runningTime[2][6];
	}
	if(myPs->runningTime[2][7] < myPs->runningTime[5][7]) {
		myPs->runningTime[5][7] = myPs->runningTime[2][7];
		myPs->runningTime[5][8] = myPs->runningTime[2][8];
	}
}

//kjg_121106_s
void localtime(const time_t *timepr, struct tm *r)
{
	int j;
	time_t i;
	time_t timep;
	extern struct timezone sys_tz;
	const unsigned int __spm[12] = {
		0,
		(31),
		(31+28),
		(31+28+31),
		(31+28+31+30),
		(31+28+31+30+31),
		(31+28+31+30+31+30),
		(31+28+31+30+31+30+31),
		(31+28+31+30+31+30+31+31),
		(31+28+31+30+31+30+31+31+30),
		(31+28+31+30+31+30+31+31+30+31),
		(31+28+31+30+31+30+31+31+30+31+30),
	};
	const unsigned int __spm2[12] = {
		0,
		(31),
		(31+29),
		(31+29+31),
		(31+29+31+30),
		(31+29+31+30+31),
		(31+29+31+30+31+30),
		(31+29+31+30+31+30+31),
		(31+29+31+30+31+30+31+31),
		(31+29+31+30+31+30+31+31+30),
		(31+29+31+30+31+30+31+31+30+31),
		(31+29+31+30+31+30+31+31+30+31+30),
	};
	register time_t work;

	timep = (*timepr) - (sys_tz.tz_minuteswest * 60);
	work = timep % (SPD);
	r->tm_sec = work % 60;
	work /= 60;
	r->tm_min = work % 60;
	r->tm_hour = work / 60;
	work = timep / (SPD);
	r->tm_wday = (4 + work) % 7;
	for (i=1970; ; ++i) {
		register time_t k = (!(i%4) && ((i%100) || !(i%400))) ? 366 : 365;
		if (work > k) {
			work -= k;
			j = (int)k;
		} else {
			j = (int)k;
			break;
		}
	}
	r->tm_year = i - 1900;
	if(j == 366) {
		for (i=11; i && __spm2[i] > work; --i) ;
		r->tm_mon = i;
		r->tm_mday = work - __spm2[i] + 1;
	} else {
		for (i=11; i && __spm[i] > work; --i) ;
		r->tm_mon = i;
		r->tm_mday = work - __spm[i] + 1;
	}
	if((r->tm_mon+1) == 12 && r->tm_mday == 32) {
		r->tm_year += 1;
		r->tm_mon = 0;
		r->tm_mday = 1;
	}
}

void Sync_RTC(void)
{
	struct timeval tv;
	struct tm realTime;

	do_gettimeofday(&tv);
	localtime(&tv.tv_sec, &realTime);

	//printk(KERN_INFO "(currentTime:%.04d-%.02d-%.02d %.02d:%.02d:%.02d)\n",
	//	realTime.tm_year+1900, realTime.tm_mon+1, realTime.tm_mday,
	//	realTime.tm_hour, realTime.tm_min, realTime.tm_sec);
//KHK 20191121 -------------------------------------------------------------
	if(myPs->real_time[1] != realTime.tm_sec){
		myPs->misc.slot_tic_timer = 0;
	}
//KHK 20191121 -------------------------------------------------------------
	
	//myPs->real_time[0] = 0; //kjg_w (long)(tv.tv_usec + 500) / 1000; //msec
	myPs->real_time[0] = myPs->misc.slot_tic_timer;  //msec	//KHK 20191121
	myPs->real_time[1] = (long)realTime.tm_sec; //sec
	myPs->real_time[2] = (long)realTime.tm_min; //min
	myPs->real_time[3] = (long)realTime.tm_hour; //hour

	myPs->real_time[4] = (long)realTime.tm_mday; //day
	myPs->real_time[5] = (long)realTime.tm_mon + 1; //month
	myPs->real_time[6] = (long)realTime.tm_year + 1900; //year
} //kjg_121106_e

//KHK 20191121 -------------------------------------------------------------

void slot_tic_timer(void)
{
	myPs->misc.slot_tic_timer += (short int)(myPs->misc.rt_periodic / 1000000);
	if(myPs->misc.slot_tic_timer >= 1000) {	//1 Sec
		myPs->misc.slot_tic_timer = 0;
	}
}

//KHK 20191121 -------------------------------------------------------------

/*
void Sync_RTC(void)
{
	long tmp[6];
	struct timeval tv;

	//kjg_w set_time
	//tv.tv_sec = mktime(2008, 8, 13, 22, 20, 0);
	//tv.tv_usec = 0;
	//do_settimeofday(&tv);

	do_gettimeofday(&tv);
	tv.tv_sec += (3600 * 9); //seoul clock

	myPs->real_time[0] = (long)(tv.tv_usec + 500) / 1000; //msec
	myPs->real_time[1] = (long)(tv.tv_sec % 60); //sec
	tmp[0] = (long)(tv.tv_sec / 60);
	myPs->real_time[2] = tmp[0] % 60; //min
	tmp[1] = tmp[0] / 60;
	myPs->real_time[3] = tmp[1] % 24; //hour

	tmp[2] = tmp[1] / 24 + 719499;

	tmp[3] = tmp[2] / 365;
	tmp[4] = (tmp[3] / 4) - (tmp[3] / 100) + (tmp[3] / 400);
	myPs->real_time[6] = tmp[3] - (tmp[4] / 365); //year

	tmp[5] = tmp[2] - (myPs->real_time[6] * 365);
	tmp[5] -= tmp[4];
	myPs->real_time[5] = tmp[5] * 12 / 367; //month
	myPs->real_time[4] = tmp[5] - (myPs->real_time[5] * 367 / 12);//day

	myPs->real_time[5] += 2;
	if(myPs->real_time[5] > 12) myPs->real_time[5] -= 12;
}
*/
void sens_count_increment_group(void)
{
	int group, type;

	if(myPs->misc.timer_1sec < 1) return;

	for(group=0; group < myData->AppControl.config.totalGroup; group++) {
		myGroup = &(myData->gData[group]);
		myGroup->ch_sens_count++;
		if(myGroup->ch_sens_count >= MAX_CH_SENS_COUNT) {
			myGroup->ch_sens_count = 0;
			myGroup->ch_sens_count_flag = P1;
		}

		for(type = 0; type < MAX_TYPE; type++){
			myGroup->auto_cali_ref[type].ref_sens_count1++;
			if(myGroup->auto_cali_ref[type].ref_sens_count1 >= MAX_REF_SENS_COUNT1) {
				myGroup->auto_cali_ref[type].ref_sens_count1 = 0;
				myGroup->auto_cali_ref[type].ref_sens_count_flag1 = P1;
			}

			if(myGroup->auto_cali_ref[type].ref_sens_count_flag1 == P1){
				myGroup->auto_cali_ref[type].ref_sens_count2++;
				if(myGroup->auto_cali_ref[type].ref_sens_count2 >= MAX_REF_SENS_COUNT2) {
					myGroup->auto_cali_ref[type].ref_sens_count2 = 0;
					myGroup->auto_cali_ref[type].ref_sens_count_flag2 = P1;
				}
			}else{
				myGroup->auto_cali_ref[type].ref_sens_count2 = 0;
				myGroup->auto_cali_ref[type].ref_sens_count_flag2 = P0;
			}
		}
	}
}

void sens_count_increment_bd(int bd)
{
	if(myPs->misc.timer_1sec < 1) return;

	myData->bData[bd].misc.ch_sens_count++;
	if(myData->bData[bd].misc.ch_sens_count >= MAX_CH_SENS_COUNT) {
		myData->bData[bd].misc.ch_sens_count = 0;
		myData->bData[bd].misc.ch_sens_count_flag = P1;
	}
}

void time_cal_increment(unsigned long time_day, unsigned long time, unsigned long val)
{
	unsigned long tmp, tmp_day, tmp_time, tmp_val;

	tmp_val = time;
	tmp = tmp_val + val;
	tmp_val = time_day;
	tmp_day = tmp_val + (tmp / (360000 * 24));
	tmp_time = (tmp % (360000 * 24));

	myData->mData.misc.tmp_inc_time_day = tmp_day;
	myData->mData.misc.tmp_inc_time = tmp_time;
}

void time_cal_decrement(unsigned long time_day, unsigned long time, unsigned long val)
{
	unsigned long tmp, tmp_day, tmp_time, tmp_val;

	tmp = val / (360000 * 24);
	tmp_val = time_day;
	tmp_day = tmp_val - tmp;
	if(tmp_day < 0) tmp_day = 0;

	tmp = (val % (360000 * 24));
	tmp_val = time;
	tmp_time = tmp_val - tmp;
	if(tmp_time < 0) {
		if(tmp_day > 0) {
			tmp_day--;
			tmp_time = (360000 * 24) + tmp_time;
		} else {
			tmp_time = 0;
		}
	}

	myData->mData.misc.tmp_dec_time_day = tmp_day;
	myData->mData.misc.tmp_dec_time = tmp_time;
}

int time_cal_compare(int compType, unsigned long cmp_time_day, unsigned long cmp_time, unsigned long time_day, unsigned long time)
{
	int rtn;

	switch(compType) {
		case COMP_NONE:
			rtn = 0;
			break;
		case COMP_LESS_THAN: //cmp_time < time
			if(cmp_time_day < time_day) {
				rtn = 1;
			} else if(cmp_time_day == time_day) {
				if(cmp_time < time) {
					rtn = 1;
				} else if(cmp_time == time) {
					rtn = 0;
				} else {
					rtn = 0;
				}
			} else {
				rtn = 0;
			}
			break;
		case COMP_LESS_THAN_OR_EQUAL_TO: //cmp_time <= time
			if(cmp_time_day < time_day) {
				rtn = 1;
			} else if(cmp_time_day == time_day) {
				if(cmp_time < time) {
					rtn = 1;
				} else if(cmp_time == time) {
					rtn = 1;
				} else {
					rtn = 0;
				}
			} else {
				rtn = 0;
			}
			break;
		case COMP_GREATER_THAN: //cmp_time > time
			if(cmp_time_day < time_day) {
				rtn = 0;
			} else if(cmp_time_day == time_day) {
				if(cmp_time < time) {
					rtn = 0;
				} else if(cmp_time == time) {
					rtn = 0;
				} else {
					rtn = 1;
				}
			} else {
				rtn = 1;
			}
			break;
		case COMP_GREATER_THAN_OR_EQUAL_TO: //cmp_time >= time
			if(cmp_time_day < time_day) {
				rtn = 0;
			} else if(cmp_time_day == time_day) {
				if(cmp_time < time) {
					rtn = 0;
				} else if(cmp_time == time) {
					rtn = 1;
				} else {
					rtn = 1;
				}
			} else {
				rtn = 1;
			}
			break;
		case COMP_EQUAL_TO: //cmp_time == time
			if(cmp_time_day < time_day) {
				rtn = 0;
			} else if(cmp_time_day == time_day) {
				if(cmp_time < time) {
					rtn = 0;
				} else if(cmp_time == time) {
					rtn = 1;
				} else {
					rtn = 0;
				}
			} else {
				rtn = 0;
			}
			break;
		case COMP_NOT_EQUAL_TO: //cmp_time != time
			if(cmp_time_day < time_day) {
				rtn = 1;
			} else if(cmp_time_day == time_day) {
				if(cmp_time < time) {
					rtn = 1;
				} else if(cmp_time == time) {
					rtn = 0;
				} else {
					rtn = 1;
				}
			} else {
				rtn = 1;
			}
			break;
		default:
			rtn = 0;
			break;
	}

	return rtn;
}

void Aux_Voltage_Temp_Thermistor_compare(int ch)
{ //kjhw_170820
	unsigned char flag, k; //kjhw_181223
	short int func_div, func_div1, func_div2, func_div3; //kjhw_181223
	short int comp_count_auxTemp, comp_count_auxTH; //kjhw_181223
	short int comp_count_auxV; //kjhw_190821
	short int comp_count_group_auxV[MAX_AUX_GROUP], count_group_auxV[MAX_AUX_GROUP]; //ktg_220220
	int i, j, temp_count, auxV_count, auxTH_count, max_aux_data;
	long val;
	long maxAuxTemp, minAuxTemp, maxAuxV, minAuxV, maxAuxTH, minAuxTH;
	long avgT; //kjhw_181223
	long avgAuxV;	//ktg_210105
	long maxGroupAuxV[MAX_AUX_GROUP], minGroupAuxV[MAX_AUX_GROUP];

	if(myPs->config.installedTemp == 0
		&& myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0
		&& myPs->config.installedHumidity == 0
		&& myPs->config.installedGas == 0) return;	//sec_220926

	//kjhw_170906s
	maxAuxTemp = myData->cData[ch].misc.maxAuxTemp = 0;
	minAuxTemp = myData->cData[ch].misc.minAuxTemp = 0;
	maxAuxV = myData->cData[ch].misc.maxAuxV = 0;
	minAuxV = myData->cData[ch].misc.minAuxV = 0;
	maxAuxTH = myData->cData[ch].misc.maxAuxTH = 0;
	minAuxTH = myData->cData[ch].misc.minAuxTH = 0;
	//kjhw_170906e
	
	for(i=0; i < MAX_AUX_GROUP; i++) {	//ktg_220220s
		maxGroupAuxV[i] = myData->cData[ch].misc.maxGroupAuxV[i] = 0;
		minGroupAuxV[i] = myData->cData[ch].misc.minGroupAuxV[i] = 0;
		comp_count_group_auxV[i] = 0;
		count_group_auxV[i] = 0;
		}	//ktg_220220e
	avgAuxV = 0;	//ktg_210105
	
	avgT = 0; //kjhw_181223
	//comp_count_auxTemp = comp_count_auxTH = 0; //kjhw_181223
	comp_count_auxTemp = comp_count_auxTH = comp_count_auxV = 0; //kjhw_190821
	//AUX_FUNC_DIV_COMPARE_FLAG use flag	//csk_190808
	//0 : No Use, 1 : for Only Temperature, 2 : for Only Voltage, 3: for All
	//jhkw_201117s
	flag = myPs->config.Compare_flag;
	max_aux_data = myPs->config.installedTemp + myPs->config.installedAuxV 
		+ myPs->config.installedTH + myPs->config.installedHumidity
		+ myPs->config.installedGas;	//sec_220926
	if(max_aux_data >= MAX_AUX_DATA) {
	max_aux_data = MAX_AUX_DATA;
	}
	//jhkw_201117e
	
	temp_count = auxV_count = auxTH_count = 0;
	for(i=0; i < max_aux_data; i++) {
		if(ch+1 != myData->auxSetData[i].chNo) continue;
		switch(myData->auxSetData[i].auxType) {
			case 0: //temperature
				temp_count++;
				break; //kjh_171214
			case 1: //sub sensing voltage
				auxV_count++;
				if(myData->auxSetData[i].GroupNo != 0) { //ktg_220220
					count_group_auxV[myData->auxSetData[i].GroupNo-1]++;
				}
				break; //kjh_171214
			case 2: //sub sensing thermistor
				auxTH_count++;
				break; //kjh_171214
			default: break;
		}
	}

	for(i=0; i < max_aux_data; i++) {
		if(ch+1 != myData->auxSetData[i].chNo) continue;
		if(((myData->cData[ch].op.runTime % 100) == 0) 
			&& (myData->cData[ch].op.state == C_RUN)) {	//ktg_220331s	1s
			j = myData->auxSetData[i].auxChNo - 1;
			if((myData->auxSetData[i].auxType == 1)
				|| (myData->auxSetData[i].auxType == 2)) {
				j = myData->aux_ch_num[j].daq_ch;
			}
			if(myData->aux_ch_num[j].type >= 11) continue;
			switch(myData->auxSetData[i].auxType) {
				case 1: //sub sensing voltage
					func_div = AUX_FUNC_DIV_GAS_SENSOR;	//ktg_241008
					//func_div = AUX_FUNC_DIV_VNETING_GAS_FLAG;
					func_div1 = myData->auxSetData[i].function_div;
					func_div2 = myData->auxSetData[i].function_div2;
					func_div3 = myData->auxSetData[i].function_div3;
					if((func_div == func_div1) || (func_div == func_div2)
						|| (func_div == func_div3)) {
							myData->SubSensV.ch[j].gasSensV[1] = myData->SubSensV.ch[j].gasSensV[0];
							myData->SubSensV.ch[j].gasSensV[0] = myData->SubSensV.ch[j].sensV;
							myData->SubSensV.ch[j].diffGasSensV 
							= myData->SubSensV.ch[j].gasSensV[0] - myData->SubSensV.ch[j].gasSensV[1];
					}
					if(myData->cData[ch].signal[C_SIG_GAS_VENTING_DETECT_PHASE] != P2) {
						myData->SubSensV.ch[j].gasHardVentCount = 0;
						myData->SubSensV.ch[j].gasSoftVentCount = 0;
					}
					break;
				default: break;
			}
		}	//ktg_220331e		
		//csk_190808s
		if(flag != 0) {	//csk_190808
			func_div = AUX_FUNC_DIV_COMPARE_FLAG;
			func_div1 = myData->auxSetData[i].function_div;
			func_div2 = myData->auxSetData[i].function_div2;
			func_div3 = myData->auxSetData[i].function_div3;
			if((func_div == func_div1) || (func_div == func_div2)
				|| (func_div == func_div3)) {
				//comp_count++;
			} else {
//20181219 KHK------------------------------
				if(myData->auxSetData[i].auxType == 1) {
					if(flag != 1) continue;	//csk_190808
				} else {
					//continue;
					if(flag != 2) continue;	//csk_190808
				}
//------------------------------------------
			}
		} //kjhw_181223e
		//csk_190808e

		j = myData->auxSetData[i].auxChNo - 1;
		if((myData->auxSetData[i].auxType == 1) //kjhw_180809s
			|| (myData->auxSetData[i].auxType == 2)) {
			j = myData->aux_ch_num[j].daq_ch; //jhkw_201117
		} //kjhw_180809e
		if(myData->aux_ch_num[j].type >= 11) continue;
		switch(myData->auxSetData[i].auxType) {
			case 0: //temperature
				val = myData->COM.com_port[AUX_TEMP].value[j];
				if(temp_count < 2) {
					myData->cData[ch].misc.maxAuxTemp = val;
					myData->cData[ch].misc.minAuxTemp = val;
					myData->cData[ch].misc.diffAuxTemp
						= (long)(myData->cData[ch].misc.maxAuxTemp
						- myData->cData[ch].misc.minAuxTemp);
					myData->cData[ch].misc.maxAuxTemp_auxChNo
						= myData->auxSetData[i].auxChNo;
					myData->cData[ch].misc.minAuxTemp_auxChNo
						= myData->auxSetData[i].auxChNo;
				} else {
					if(maxAuxTemp == 0 && minAuxTemp == 0) {
						myData->cData[ch].misc.maxAuxTemp = val;
						myData->cData[ch].misc.minAuxTemp = val;
						myData->cData[ch].misc.maxAuxTemp_auxChNo
							= myData->auxSetData[i].auxChNo;
						myData->cData[ch].misc.minAuxTemp_auxChNo
							= myData->auxSetData[i].auxChNo;
						//kjhw_170906s
						maxAuxTemp = myData->cData[ch].misc.maxAuxTemp;
						minAuxTemp = myData->cData[ch].misc.minAuxTemp;
						//kjhw_170906e
					} else if(val > maxAuxTemp) {
						myData->cData[ch].misc.maxAuxTemp = val;
						myData->cData[ch].misc.maxAuxTemp_auxChNo
							= myData->auxSetData[i].auxChNo;
						//kjhw_171107s
						maxAuxTemp = myData->cData[ch].misc.maxAuxTemp;
						//kjhw_171107e
					} else if(val < minAuxTemp) {
						myData->cData[ch].misc.minAuxTemp = val;
						myData->cData[ch].misc.minAuxTemp_auxChNo
							= myData->auxSetData[i].auxChNo;
						//kjhw_171107s
						minAuxTemp = myData->cData[ch].misc.minAuxTemp;
						//kjhw_171107e
					} else {}
					myData->cData[ch].misc.diffAuxTemp
						= (long)(myData->cData[ch].misc.maxAuxTemp
						- myData->cData[ch].misc.minAuxTemp);
				}
				//csk_190808s
				avgT = avgT + val; //jhkw_190314
				comp_count_auxTemp++;
				myData->cData[ch].misc.comp_count_auxTemp1 = comp_count_auxTemp; //kjhw_19819
				//csk_190808e
				break;
			case 1: //sub sensing voltage
				if(myData->aux_ch_num[j].type != 1) {
					break;
				}
				val = myData->SubSensV.ch[j].sensV;
				if(auxV_count < 2) {
					myData->cData[ch].misc.maxAuxV = val;
					myData->cData[ch].misc.minAuxV = val;
					myData->cData[ch].misc.diffAuxV
						= (long)(myData->cData[ch].misc.maxAuxV
						- myData->cData[ch].misc.minAuxV);
					myData->cData[ch].misc.maxAuxV_auxChNo
						= myData->auxSetData[i].auxChNo;
					myData->cData[ch].misc.minAuxV_auxChNo
						= myData->auxSetData[i].auxChNo;
				} else {
					if(maxAuxV == 0 && minAuxV == 0) {
						myData->cData[ch].misc.maxAuxV = val;
						myData->cData[ch].misc.minAuxV = val;
						myData->cData[ch].misc.maxAuxV_auxChNo
							= myData->auxSetData[i].auxChNo;
						myData->cData[ch].misc.minAuxV_auxChNo
							= myData->auxSetData[i].auxChNo;
						//kjhw_170906s
						maxAuxV = myData->cData[ch].misc.maxAuxV;
						minAuxV = myData->cData[ch].misc.minAuxV;
						//kjhw_170906e
					} else if(val > maxAuxV) {
						myData->cData[ch].misc.maxAuxV = val;
						myData->cData[ch].misc.maxAuxV_auxChNo
							= myData->auxSetData[i].auxChNo;
						//kjhw_171107s
						maxAuxV = myData->cData[ch].misc.maxAuxV;
						//kjhw_171107e
					} else if(val < minAuxV) {
						myData->cData[ch].misc.minAuxV = val;
						myData->cData[ch].misc.minAuxV_auxChNo
							= myData->auxSetData[i].auxChNo;
						//kjhw_171107s
						minAuxV = myData->cData[ch].misc.minAuxV;
						//kjhw_171107e
					} else {}
					myData->cData[ch].misc.diffAuxV
						= (long)(myData->cData[ch].misc.maxAuxV
						- myData->cData[ch].misc.minAuxV);
				}
				if(myData->auxSetData[i].GroupNo != 0) { //ktg_220220s
					val = myData->SubSensV.ch[j].sensV;
					k = myData->auxSetData[i].GroupNo-1;
					if(count_group_auxV[k] < 2) {
						myData->cData[ch].misc.maxGroupAuxV[k] = val;
						myData->cData[ch].misc.minGroupAuxV[k] = val;
						myData->cData[ch].misc.diffGroupAuxV[k]
							= (long)(myData->cData[ch].misc.maxGroupAuxV[k]
							- myData->cData[ch].misc.minGroupAuxV[k]);
						myData->cData[ch].misc.maxGroupAuxV_auxChNo[k]
							= myData->auxSetData[i].auxChNo;
						myData->cData[ch].misc.minGroupAuxV_auxChNo[k]
							= myData->auxSetData[i].auxChNo;
					} else {
						if(maxGroupAuxV[k] == 0 && minGroupAuxV[k] == 0) {
							myData->cData[ch].misc.maxGroupAuxV[k] = val;
							myData->cData[ch].misc.minGroupAuxV[k] = val;
							myData->cData[ch].misc.maxGroupAuxV_auxChNo[k]
								= myData->auxSetData[i].auxChNo;
							myData->cData[ch].misc.minGroupAuxV_auxChNo[k]
								= myData->auxSetData[i].auxChNo;
							maxGroupAuxV[k] = myData->cData[ch].misc.maxGroupAuxV[k];
							minGroupAuxV[k] = myData->cData[ch].misc.minGroupAuxV[k];
						} else if(val > maxGroupAuxV[k]) {
							myData->cData[ch].misc.maxGroupAuxV[k] = val;
							myData->cData[ch].misc.maxGroupAuxV_auxChNo[k]
							= myData->auxSetData[i].auxChNo;
							maxGroupAuxV[k] = myData->cData[ch].misc.maxGroupAuxV[k];
						} else if(val < minGroupAuxV[k]) {
							myData->cData[ch].misc.minGroupAuxV[k] = val;
							myData->cData[ch].misc.minGroupAuxV_auxChNo[k]
								= myData->auxSetData[i].auxChNo;
							minGroupAuxV[k] = myData->cData[ch].misc.minGroupAuxV[k];
						} else {}
						myData->cData[ch].misc.diffGroupAuxV[k]
							= (long)(myData->cData[ch].misc.maxGroupAuxV[k]
							- myData->cData[ch].misc.minGroupAuxV[k]);
					}
				}	//ktg_220220e
				
				//ktg_210105s
				if(myData->cData[ch].op.runTime % 100 == 0) {
					if(myData->cData[ch].op.stepType == STEP_REST) {	//1sec
						avgAuxV = avgAuxV + val;
						myData->SubSensV.ch[j].prepreAuxV = myData->SubSensV.ch[j].preAuxV;
						myData->SubSensV.ch[j].preAuxV = myData->SubSensV.ch[j].currentAuxV;
						myData->SubSensV.ch[j].currentAuxV = val;	//ktg_220214
					}
				}
				if(myData->cData[ch].signal[C_SIG_SVDF_DETECT_PHASE] == P5 ||
						myData->cData[ch].signal[C_SIG_SVDF_DETECT_PHASE] == P0) {
					myData->SubSensV.ch[j].prepreAuxV = 0;
					myData->SubSensV.ch[j].preAuxV = 0;
					myData->SubSensV.ch[j].currentAuxV = 0;	//ktg_220214
					myData->SubSensV.ch[j].svdfcount = 0;
				}
				//ktg_210105e
				if((myData->cData[ch].misc.checkDelayTime == 10)	//ktg_210419s
					&& (myData->cData[ch].misc.checkDelayTime_day == 0)) {
					myData->SubSensV.ch[j].preSensV = val;
					myData->cData[ch].signal[C_SIG_FAULT_AUX_DELTA_V_PHASE] = P3;
					myData->cData[ch].misc.check_fault_delta_AuxV_Time_day
						= myData->cData[ch].misc.checkDelayTime_day;
					myData->cData[ch].misc.check_fault_delta_AuxV_Time
						= myData->cData[ch].misc.checkDelayTime;
				}	//ktg_210419e
				
				comp_count_auxV++; //kjhw_190821
				myData->cData[ch].misc.comp_count_auxV1 = comp_count_auxV; //kjhw_19819
				break;
			case 2: //sub sensing thermistor
				val = myData->SubSensV.ch[j].sensV;
				if(auxTH_count < 2) {
					myData->cData[ch].misc.maxAuxTH = val;
					myData->cData[ch].misc.minAuxTH = val;
					myData->cData[ch].misc.diffAuxTH
						= (long)(myData->cData[ch].misc.maxAuxTH
						- myData->cData[ch].misc.minAuxTH);
					myData->cData[ch].misc.maxAuxTH_auxChNo
						= myData->auxSetData[i].auxChNo;
					myData->cData[ch].misc.minAuxTH_auxChNo
						= myData->auxSetData[i].auxChNo;
				} else {
					if(maxAuxTH == 0 && minAuxTH == 0) {
						myData->cData[ch].misc.maxAuxTH = val;
						myData->cData[ch].misc.minAuxTH = val;
						myData->cData[ch].misc.maxAuxTH_auxChNo
							= myData->auxSetData[i].auxChNo;
						myData->cData[ch].misc.minAuxTH_auxChNo
							= myData->auxSetData[i].auxChNo;
						//kjhw_170906s
						maxAuxTH = myData->cData[ch].misc.maxAuxTH;
						minAuxTH = myData->cData[ch].misc.minAuxTH;
						//kjhw_170906e
					} else if(val > maxAuxTH) {
						myData->cData[ch].misc.maxAuxTH = val;
						myData->cData[ch].misc.maxAuxTH_auxChNo
							= myData->auxSetData[i].auxChNo;
						//kjhw_171107s
						maxAuxTH = myData->cData[ch].misc.maxAuxTH;
						//kjhw_171107e
					} else if(val < minAuxTH) {
						myData->cData[ch].misc.minAuxTH = val;
						myData->cData[ch].misc.minAuxTH_auxChNo
							= myData->auxSetData[i].auxChNo;
						//kjhw_171107s
						minAuxTH = myData->cData[ch].misc.minAuxTH;
						//kjhw_171107e
					} else {}
					myData->cData[ch].misc.diffAuxTH
						= (long)(myData->cData[ch].misc.maxAuxTH
						- myData->cData[ch].misc.minAuxTH);
				}
				//csk_190808s
				avgT = avgT + (val / 1000); //jhkw_190314
				comp_count_auxTH++;
				myData->cData[ch].misc.comp_count_auxTH1 = comp_count_auxTH; //kjhw_19819
				//csk_190808e
				break;
			default: break;
		}
	}

	//csk_190808s
	if((comp_count_auxTemp != 0) && (comp_count_auxTH != 0)) {
		if(myData->cData[ch].misc.maxAuxTemp
			>= (myData->cData[ch].misc.maxAuxTH / 1000)) {
			myData->cData[ch].misc.maxAuxT = myData->cData[ch].misc.maxAuxTemp;
		} else {
			myData->cData[ch].misc.maxAuxT
				= myData->cData[ch].misc.maxAuxTH / 1000;
		}
		if(myData->cData[ch].misc.minAuxTemp
			<= (myData->cData[ch].misc.minAuxTH / 1000)) {
			myData->cData[ch].misc.minAuxT = myData->cData[ch].misc.minAuxTemp;
		} else {
			myData->cData[ch].misc.minAuxT
				= myData->cData[ch].misc.minAuxTH / 1000;
		}
	} else if((comp_count_auxTemp != 0) && (comp_count_auxTH == 0)) {
		myData->cData[ch].misc.maxAuxT = myData->cData[ch].misc.maxAuxTemp;
		myData->cData[ch].misc.minAuxT = myData->cData[ch].misc.minAuxTemp;
	} else if((comp_count_auxTemp == 0) && (comp_count_auxTH != 0)) {
		myData->cData[ch].misc.maxAuxT = myData->cData[ch].misc.maxAuxTH / 1000;
		myData->cData[ch].misc.minAuxT = myData->cData[ch].misc.minAuxTH / 1000;
	}
	myData->cData[ch].misc.diffAuxT
		= myData->cData[ch].misc.maxAuxT - myData->cData[ch].misc.minAuxT;
	if((comp_count_auxTemp == 0) && (comp_count_auxTH == 0)) {
		myData->cData[ch].misc.avgAuxT = 0;
	} else {
		myData->cData[ch].misc.avgAuxT
			= (long)((float)avgT / (comp_count_auxTemp + comp_count_auxTH));
			//= (long)(((float)avgT / (comp_count_auxTemp + comp_count_auxTH))
			//* 1000);
	}
	//csk_190808e
	//ktg_210105s
	if((((myData->cData[ch].op.runTime % 100) == 0) && (myData->cData[ch].op.stepType == STEP_REST))
			|| (myData->cData[ch].signal[C_SIG_SVDF_DETECT_PHASE] == P5)) {	//1sec or reset
		if(myData->cData[ch].signal[C_SIG_SVDF_DETECT_PHASE] >= P3) {
			myData->cData[ch].misc.avgAuxV[2] = myData->cData[ch].misc.avgAuxV[1];
		}
		if(myData->cData[ch].signal[C_SIG_SVDF_DETECT_PHASE] >= P2) {
			myData->cData[ch].misc.avgAuxV[1] = myData->cData[ch].misc.avgAuxV[0];
		}
		//myData->cData[ch].misc.avgAuxV[0] = avgAuxV / auxV_count;
		//ktg_210217s
		if(comp_count_auxV > 0) {
		myData->cData[ch].misc.avgAuxV[0] = avgAuxV / comp_count_auxV;
		}	//ktg_210217e

		//if(myData->cData[ch].signal[C_SIG_SVDF_DETECT_PHASE] == P5 &&	//ktg_210706
		if(myData->cData[ch].signal[C_SIG_SVDF_DETECT_PHASE] == P5 
			|| myData->cData[ch].signal[C_SIG_SVDF_DETECT_PHASE] == P0) {
			myData->cData[ch].misc.avgAuxV[2] = 0;
			myData->cData[ch].misc.avgAuxV[1] = 0;
			myData->cData[ch].misc.avgAuxV[0] = 0;
		}
		for(i=0; i < MAX_AUX_DATA; i++) {
			if(ch+1 != myData->auxSetData[i].chNo) continue;
			if(flag != 0) {
				func_div = AUX_FUNC_DIV_COMPARE_FLAG;
				func_div1 = myData->auxSetData[i].function_div;
				func_div2 = myData->auxSetData[i].function_div2;
				func_div3 = myData->auxSetData[i].function_div3;
				if((func_div == func_div1) || (func_div == func_div2)
					|| (func_div == func_div3)) {
				} else {
					if(myData->auxSetData[i].auxType == 1) {
						if(flag != 1) continue;	//csk_190808
					} else {
						//continue;
						if(flag != 2) continue;	//csk_190808
					}
				}
			}
		
			j = myData->auxSetData[i].auxChNo - 1;
			if((myData->auxSetData[i].auxType == 1)
				|| (myData->auxSetData[i].auxType == 2)) {
				j = myData->aux_ch_num[j].daq_ch; //jhkw_201117
			}
			switch(myData->auxSetData[i].auxType) {
				case 1: //sub sensing voltage
					if(myData->cData[ch].signal[C_SIG_SVDF_DETECT_PHASE] == P4) {
						val = myData->SubSensV.ch[j].currentAuxV;	//ktg_220214
						myData->SubSensV.ch[j].svdfAuxV 
							+= (myData->SubSensV.ch[j].prepreAuxV - val)
							- (myData->cData[ch].misc.avgAuxV[2] - myData->cData[ch].misc.avgAuxV[0]);
					} else if(myData->cData[ch].signal[C_SIG_SVDF_DETECT_PHASE] == P5 ||
						myData->cData[ch].signal[C_SIG_SVDF_DETECT_PHASE] == P0) {
						myData->SubSensV.ch[j].svdfAuxV = 0; 
					}
					if(myData->cData[ch].signal[C_SIG_DROP_VOLTAGE_PHASE] == P0) {
						myData->SubSensV.ch[j].DropmaxAuxV = 0;
						myData->SubSensV.ch[j].DropminAuxV = 0;
						myData->SubSensV.ch[j].DropAuxVcount = 0;
					}
					break;
				default: break;
			}
		}
	}
	//ktg_210105e
}

void CAN_function_compare(int ch)	//ktg_2106
{
	int i;
	short int func_div, func_div1, func_div2, func_div3;
	unsigned int cellpoint;
	unsigned int dataCount;
	unsigned int cellCount;
	long l_val, maxCell, minCell, avgCell, l_val2;

	if(myData->AppControl.misc.Load_Process_CAN[0] != P1) return;
	if(myPs->config.installedCAN == 0) return;
	/*if((myData->canReceiveSetData.commonData[ch][0].cell_check_use_flag == 0)
		&& (myData->canReceiveSetData.commonData[ch][1].cell_check_use_flag == 0)) {
		return;
	}*/

	switch(myPs->config.division_CAN) {
		case 1:
			dataCount = myData->canReceiveDataCount[ch][0];
			cellCount = myData->canReceiveCellCount[ch][0];
			if(dataCount == 0) return;
			//if(cellCount == 0) return;
			break;
		default:
			dataCount = myData->canReceiveDataCount[ch][0]
				+ myData->canReceiveDataCount[ch][1];
			cellCount = myData->canReceiveCellCount[ch][0]
				+ myData->canReceiveCellCount[ch][1];
			if(dataCount == 0) return;
			//if(cellCount == 0) return;
			break;
	}
	
	maxCell = myData->cData[ch].misc.maxCell = 0;
	minCell = myData->cData[ch].misc.minCell = 0;
	avgCell = 0;
	cellpoint = 0;

	for(i=0; i < dataCount; i++) {
		if(myData->canReceiveSetData.normalData[ch][i].canType == CAN_TYPE_NONE) break;
		if(((myData->cData[ch].op.runTime % 100) == 0) 
			&& (myData->cData[ch].op.state == C_RUN)) {	//ktg_220331s	1s
			func_div1 = myData->canReceiveSetData.normalData[ch][i].function_div;
			func_div2 = myData->canReceiveSetData.normalData[ch][i].function_div2;
			func_div3 = myData->canReceiveSetData.normalData[ch][i].function_div3;
			func_div = CAN_RX_FUNC_DIV_VENTING_GAS_FLAG;
			if(func_div1 == func_div || func_div2 == func_div
				|| func_div3 == func_div) {
				l_val = (long)(myData->CanData[ch][i].f_val[0] * 1000);
				myData->CANCell[ch][i].gasCell[1] = myData->CANCell[ch][i].gasCell[0];
				myData->CANCell[ch][i].gasCell[0] = l_val;
				myData->CANCell[ch][i].diffGasCell
					= myData->CANCell[ch][i].gasCell[0] - myData->CANCell[ch][i].gasCell[1];
			}
			if(myData->cData[ch].signal[C_SIG_CAN_GAS_VENTING_DETECT_PHASE] != P2) {
				myData->CANCell[ch][i].gasSoftVentCanCount = 0;
				myData->CANCell[ch][i].gasHardVentCanCount = 0;
			}
		}	//ktg_220331e
		if(myData->canReceiveSetData.normalData[ch][i].cell_check == 0) continue;
		if(cellpoint == cellCount) break;
		if(cellpoint < myData->canReceiveCellCount[ch][0]) {	//master
			if(myData->canReceiveSetData.commonData[ch][0].cell_check_use_flag == 1) {
				//1 : mV , 2 : V
				l_val = (long)(myData->CanData[ch][i].f_val[0]);
			} else {
				l_val = (long)(myData->CanData[ch][i].f_val[0] * 1000);
			}
		} else { 	//slave
			if(myData->canReceiveSetData.commonData[ch][1].cell_check_use_flag == 1) {
				//1 : mV , 2 : V
				l_val = (long)(myData->CanData[ch][i].f_val[0]);
			} else {
				l_val = (long)(myData->CanData[ch][i].f_val[0] * 1000);
			}
		}
		if(cellCount < 2) {	// cellCount == 1
			myData->cData[ch].misc.maxCell = l_val;
			myData->cData[ch].misc.maxCell_dataNo = i;
			myData->cData[ch].misc.minCell = l_val;
			myData->cData[ch].misc.maxCell_dataNo = i;
			myData->cData[ch].misc.avgCell[0] = l_val;
		} else {	//cellCount >= 2
			if(cellpoint == 0) { //first time
				myData->cData[ch].misc.maxCell = l_val;
				myData->cData[ch].misc.minCell = l_val;
				myData->cData[ch].misc.maxCell_dataNo = i;
				myData->cData[ch].misc.maxCell_dataNo = i;
				maxCell = myData->cData[ch].misc.maxCell;
				minCell = myData->cData[ch].misc.minCell;
			} else if(l_val > maxCell) {
				myData->cData[ch].misc.maxCell = l_val;
				myData->cData[ch].misc.maxCell_dataNo = i;
				maxCell = myData->cData[ch].misc.maxCell;
			} else if(l_val < minCell) {
				myData->cData[ch].misc.minCell = l_val;
				myData->cData[ch].misc.minCell_dataNo = i;
				minCell = myData->cData[ch].misc.minCell;
			}
		}

		if(myData->cData[ch].op.runTime % 100 == 0) {
			if(myData->cData[ch].op.stepType == STEP_REST) {	//1sec
				avgCell = avgCell + l_val;
				myData->CANCell[ch][cellpoint].prepreCell = myData->CANCell[ch][cellpoint].preCell;
				myData->CANCell[ch][cellpoint].preCell = myData->CANCell[ch][cellpoint].currentCell;
				myData->CANCell[ch][cellpoint].currentCell = l_val;	//ktg_220214
			}
		}
		if(myData->cData[ch].op.runTime % 10 == 0) {	//ktg_230728s
			if(myData->cData[ch].op.stepType == STEP_CHARGE 
				|| myData->cData[ch].op.stepType == STEP_DISCHARGE
				|| myData->cData[ch].op.stepType == STEP_PATTERN) {	//100msec
				myData->CANCell[ch][i].preCell_100ms = myData->CANCell[ch][i].currentCell_100ms;
				myData->CANCell[ch][i].currentCell_100ms = l_val;
				if(myData->cData[ch].signal[C_SIG_CAN_FREEZE_PHASE] != P0) {
					myData->CANCell[ch][i].diffCell_100ms 
						= myData->CANCell[ch][i].currentCell_100ms 
						- myData->CANCell[ch][i].preCell_100ms;
					if(myData->CANCell[ch][i].diffCell_100ms < 0) {
						myData->CANCell[ch][i].diffCell_100ms 
						 = myData->CANCell[ch][i].diffCell_100ms * (-1);
					}
				}
				if(myData->cData[ch].signal[C_SIG_CAN_FREEZE_PHASE] == P2) {
					l_val2 = myData->CANCell[ch][i].diffCell_100ms;
					if(cellpoint == 0) { //first time
						myData->cData[ch].misc2.freeze_time
							= myData->CANCell[ch][i].CanFreezecount;
						myData->cData[ch].misc2.freeze_ChNo = i;
					} else if (myData->CANCell[ch][i].diffCell_100ms >= l_val2) {
						myData->cData[ch].misc2.freeze_time
							= myData->CANCell[ch][i].CanFreezecount;
						myData->cData[ch].misc2.freeze_ChNo = i;
					}
				}
			}
		}	//ktg_230728e
		if(myData->cData[ch].signal[C_SIG_CAN_SVDF_DETECT_PHASE] == P5
			|| myData->cData[ch].signal[C_SIG_CAN_SVDF_DETECT_PHASE] == P0) {
			myData->CANCell[ch][cellpoint].prepreCell = 0;
			myData->CANCell[ch][cellpoint].preCell = 0;
			myData->CANCell[ch][cellpoint].currentCell = 0;	//ktg_220214
			myData->CANCell[ch][cellpoint].can_svdfcount = 0;
		}
		if(myData->cData[ch].signal[C_SIG_CAN_DROP_VOLTAGE_PHASE] == P0) {	//ktg_211217s
			myData->CANCell[ch][i].DropmaxCell = 0;
			myData->CANCell[ch][i].DropminCell = 0;
			myData->CANCell[ch][i].DropCellcount = 0;
		}	//ktg_211217e
		if(myData->cData[ch].signal[C_SIG_CAN_FREEZE_PHASE] == P0) {	//ktg_230728s
			myData->CANCell[ch][i].CanFreezecount = 0;
			myData->CANCell[ch][i].currentCell_100ms = 0;
			myData->cData[ch].misc2.freeze_time = 0;
			myData->cData[ch].misc2.freeze_ChNo = 0;
		}	//ktg_230728e
		cellpoint++;
	}
	myData->cData[ch].misc.diffCell
		= myData->cData[ch].misc.maxCell - myData->cData[ch].misc.minCell;

	if((((myData->cData[ch].op.runTime % 100) == 0) && (myData->cData[ch].op.stepType == STEP_REST))
		|| (myData->cData[ch].signal[C_SIG_CAN_SVDF_DETECT_PHASE] == P5)) { //1sec or reset
		
		if(myData->cData[ch].signal[C_SIG_CAN_SVDF_DETECT_PHASE] >= P3) {
			myData->cData[ch].misc.avgCell[2] = myData->cData[ch].misc.avgCell[1];
		}
		
		if(myData->cData[ch].signal[C_SIG_CAN_SVDF_DETECT_PHASE] >= P2) {
			myData->cData[ch].misc.avgCell[1] = myData->cData[ch].misc.avgCell[0];
		}
		
		if(cellpoint > 0) {
			myData->cData[ch].misc.avgCell[0] = avgCell / cellCount;
		}
		
		if(myData->cData[ch].signal[C_SIG_CAN_SVDF_DETECT_PHASE] == P5 ||
			myData->cData[ch].signal[C_SIG_CAN_SVDF_DETECT_PHASE] == P0) {
			myData->cData[ch].misc.avgCell[2] = 0;
			myData->cData[ch].misc.avgCell[1] = 0;
			myData->cData[ch].misc.avgCell[0] = 0;
		}
		
		cellpoint = 0;
		
		for(i=0; i < dataCount; i++) {
			if(myData->canReceiveSetData.normalData[ch][i].canType == CAN_TYPE_NONE) break;
			if(myData->canReceiveSetData.normalData[ch][i].cell_check == 0) continue;
			if(cellpoint == cellCount) break;
			if(cellpoint < myData->canReceiveCellCount[ch][0]) {	//master
				if(myData->canReceiveSetData.commonData[ch][0].cell_check_use_flag == 1) {
					//1 : mV , 2 : mV
					l_val = (long)(myData->CanData[ch][i].f_val[0]);
				} else {
					l_val = (long)(myData->CanData[ch][i].f_val[0] * 1000);
				}
			} else { 	//slave
				if(myData->canReceiveSetData.commonData[ch][1].cell_check_use_flag == 1) {
					//1 : mV , 2 : mV
					l_val = (long)(myData->CanData[ch][i].f_val[0]);
				} else {
					l_val = (long)(myData->CanData[ch][i].f_val[0] * 1000);
				}
			}
			if(myData->cData[ch].signal[C_SIG_CAN_SVDF_DETECT_PHASE] == P4) {
				myData->CANCell[ch][cellpoint].svdfCell +=
					(myData->CANCell[ch][cellpoint].prepreCell - myData->CANCell[ch][cellpoint].currentCell) -
					(myData->cData[ch].misc.avgCell[2] - myData->cData[ch].misc.avgCell[0]);	//ktg_220214
			} else if (myData->cData[ch].signal[C_SIG_CAN_SVDF_DETECT_PHASE] == P5 ||
				myData->cData[ch].signal[C_SIG_CAN_SVDF_DETECT_PHASE] == P0) {
				myData->CANCell[ch][cellpoint].svdfCell = 0;
			}
			cellpoint++;
		}
	}
}
