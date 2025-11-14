#include <pthread.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <linux/slab.h>
#include <linux/time.h>
//#include "../rt_can/rt_can_ext.h"	//kjg_181121
#ifdef __CAN_FD__	//jhkw_190714s
#include "rt_can_ext.h"	//kjg_181121
#else //CAN_2P0B
#include "../rt_can/rt_can.h"
#endif	//jhkw_190714e
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

void Initialize(void)
{
	myPs = &(myData->mData);
	myDio = &(myData->dio);

	Init_SystemMemory_1();

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_50A_5A:
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
			outb(0x01, 0x064F); //io board reset clear
			break;
		default:	break;
	}

	if(myData->dio.config.logic_type == 1) { //kjg_logic_type_140324
		//outb(0x01, 0x604);
		Select_OutPoint(0, 0, O_LOGIC_TYPE, ON); //kjhw_180213
	}
	
	if(myData->dio.config.watchdogType == 1) { //kjg_logic_type_140324
		CheckWDT();
		EnableWDT();
	}

	Init_DMA_DAQ();
	Init_RT_CAN();
	Init_RT_COM();

	myData->AppControl.signal[APP_SIG_MODULE_CONTROL_PROCESS] = P1;
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

#ifdef __CAN_FD__ //jhkw_190714s
void Init_RT_CAN(void)
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
}
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
		rt_can_set_param(i, bps, sjw, (int)myData->canReceiveSetData
			.commonData[i/2][i%2].extended_id, 0);
	}
}

void Close_RT_CAN(void)
{
	int i;

	if(myData->AppControl.misc.Load_Process_CAN[0] != P1) return;

	for(i=0; i < myPs->config.installedCAN; i++) {
		rt_can_setup(i, -1, 0, 0);
	}
}
#endif //jhkw_190714e

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

void ch_sens_count_increment(int ch)
{
	int filter_ad_count;

	filter_ad_count = (int)myPs->config.filter_ad_count;

/*	if(myData->cData[ch].misc.sensCountFlag == P2) {
		myData->cData[ch].misc.sensCount = 0;
		myData->cData[ch].misc.sensCountFlag = P0;
	}
*/
//	myData->cData[ch].misc.tmp_sensCount = myData->cData[ch].misc.sensCount;
//	cnt_end = myData->cData[ch].misc.sensCount;
/*	if(myData->cData[ch].misc.sensCountFlag == P0) {
		cnt_end = cnt_end + 1;
	} else {
		cnt_end = filter_ad_count;
	}
*/
	myData->cData[ch].misc.sensCount++;
	if(myData->cData[ch].misc.sensCount >= filter_ad_count) { //kjg_100103
		myData->cData[ch].misc.sensCount = 0;
		myData->cData[ch].misc.sensCountFlag = P1;
	}
}

/*
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
*/

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

void bd_sens_count_increment_12(void)
{
	int filter_ad_count;

	filter_ad_count = (int)myPs->config.filter_ad_count;

	myPs->misc.bd_sensCount++;
	if(myPs->misc.bd_sensCount >= filter_ad_count) {
		myPs->misc.bd_sensCount = 0;
		myPs->misc.bd_sensCountFlag = P1;
	}

	if(myPs->misc.timer_1sec >= 1) { //after 1sec
		myPs->misc.source_sensCount++;
		if(myPs->misc.source_sensCount >= MAX_SOURCE_SENS_COUNT) {
			myPs->misc.source_sensCount = 0;
			myPs->misc.source_sensCountFlag = P1;
		}
	}
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

void module_runningTime(int index, long val1, long val2)
{
	if(myPs->misc.timer_1sec <= 4) return;

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
//kjhw_140520e

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

	myPs->real_time[0] = 0; //kjg_w (long)(tv.tv_usec + 500) / 1000; //msec
	myPs->real_time[1] = (long)realTime.tm_sec; //sec
	myPs->real_time[2] = (long)realTime.tm_min; //min
	myPs->real_time[3] = (long)realTime.tm_hour; //hour

	myPs->real_time[4] = (long)realTime.tm_mday; //day
	myPs->real_time[5] = (long)realTime.tm_mon + 1; //month
	myPs->real_time[6] = (long)realTime.tm_year + 1900; //year
} //kjg_121106_e
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
	unsigned char flag; //kjhw_181223
	short int func_div, func_div1, func_div2, func_div3; //kjhw_181223
	short int comp_count_auxTemp, comp_count_auxTH; //kjhw_181223
	int i, j, max_aux_data;
	long val;
	long maxAuxTemp, minAuxTemp, maxAuxV, minAuxV, maxAuxTH, minAuxTH;
	long avgT; //kjhw_181223
	
	if(myPs->config.installedTemp == 0
		&& myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0) return;

	maxAuxTemp = myData->cData[ch].misc.maxAuxTemp = 0; //kjhw_170906s
	minAuxTemp = myData->cData[ch].misc.minAuxTemp = 0;
	maxAuxV = myData->cData[ch].misc.maxAuxV = 0;
	minAuxV = myData->cData[ch].misc.minAuxV = 0;
	maxAuxTH = myData->cData[ch].misc.maxAuxTH = 0;
	minAuxTH = myData->cData[ch].misc.minAuxTH = 0; //kjhw_170906e

	avgT = 0; //kjhw_181223
	comp_count_auxTemp = comp_count_auxTH = 0; //kjhw_181223
	//jhkw_201117s
	flag = myData->daq.config.Compare_flag;
	max_aux_data = myPs->config.installedTemp 
		+ myPs->config.installedAuxV + myPs->config.installedTH;
	if(max_aux_data >= MAX_AUX_DATA) {
		max_aux_data = MAX_AUX_DATA;
	}
	//jhkw_201117e
		

	for(i=0; i < max_aux_data; i++) {
		if(ch+1 != myData->auxSetData[i].chNo) continue;
		if(flag == 1) { //kjhw_181223s
			func_div = AUX_FUNC_DIV_COMPARE_FLAG;
			func_div1 = myData->auxSetData[i].function_div;
			func_div2 = myData->auxSetData[i].function_div2;
			func_div3 = myData->auxSetData[i].function_div3;
			if((func_div == func_div1) || (func_div == func_div2)
				|| (func_div == func_div3)) {
			} else {
				if((myData->auxSetData[i].auxType == 1) 
					&& (myData->auxSetData[i].fix_safety_flag != 1)) {		//khj_200908
				}else{
					continue;
				}
			}
		} //kjhw_181223e
		j = myData->auxSetData[i].auxChNo - 1;
		if((myData->auxSetData[i].auxType == 1) //kjhw_180809s
			|| (myData->auxSetData[i].auxType == 2)) {
			j = myData->aux_ch_num[j].daq_ch; //jhkw_201117
		} //kjhw_180809e
		if(myData->aux_ch_num[j].type >= 11) continue;
		switch(myData->auxSetData[i].auxType) {
			case 0: //temperature
				val = myData->COM.com_port[AUX_TEMP].value[j];
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
				
				//avgT = avgT + (val / 1000); //kjhw_181223
				avgT = avgT + val; //jhkw_190314
				comp_count_auxTemp++;
				break;
			case 1: //sub sensing voltage
				if(myData->aux_ch_num[j].type != 1) {
					break;
				}
				val = myData->SubSensV.ch[j].sensV;
				
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
				break;
			case 2: //sub sensing thermistor
				val = myData->SubSensV.ch[j].sensV;
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
				//avgT = avgT + (val / 1000000); //kjhw_181223
				avgT = avgT + (val / 1000); //jhkw_190314
				comp_count_auxTH++;
				break;
			default: break;
		}
	}
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
		myData->cData[ch].misc.maxAuxT = 0;	//jhkw_201117s
		myData->cData[ch].misc.minAuxT = 0;
		myData->cData[ch].misc.diffAuxT = 0;	//jhkw_201117e
		myData->cData[ch].misc.avgAuxT = 0;
	} else {
		myData->cData[ch].misc.avgAuxT
			= (long)((float)avgT / (comp_count_auxTemp + comp_count_auxTH));
			//= (long)(((float)avgT / (comp_count_auxTemp + comp_count_auxTH))
			//* 1000);
	}
}
//jhkw_221205s
long cFind_Sequence_Charge_Current(int ch)
{
	unsigned char row, col, row2, sequence_count;
	int i = 0, k = 0, j, tmp,idxStepNo; //shhw_230525
	//int i, j, tmp, idx, func_div, idxStepNo;
	long temp = 0, SOC = 0, val = 0, row2_val = 0;

	row = myData->testCond[ch].SQ_Charge.row;
	col = myData->testCond[ch].SQ_Charge.col;
	row2 = myData->testCond[ch].SQ_Charge.row2;
    idxStepNo = myData->cData[ch].op.idxStepNo;
	sequence_count = myData->cData[ch].misc.sequence_count;

	
	//shhw_230525s
	
	//CUT OFF Start
	//row2 1: Unuse, 2:maxAuV, 3: minAuxV, 4: canData(AuxV)
	switch(row2) {
		case 1:
			row2_val = 0;
			break;
		case 2:
			row2_val = myData->cData[ch].misc.maxAuxV;
			break;
		case 3:
			row2_val = myData->cData[ch].misc.minAuxV;
			break;
		case 4:
			row2_val = myData->cData[ch].misc.can_data_l[0]; //CAN_RX_FUNC_DIV_C_TABLE_ROW	
			//row2_val = myData->cData[ch].misc.can_data_l[0] * 1000;
			break;
		default:
			row2_val = 0;
			break;
	}
	//CUT OFF End
	
	//SOC, Capacity(Ah) Start	
	//row 1: Unuse, 2: SOC, 3: Ah, from 4, it doesn't use yet
	switch(row) {
		case 1:
			SOC = 0; 		
			break;
		case 2:
			SOC = (long)myData->cData[ch].op.SOC;
			break;
		case 3:
			SOC = myData->cData[ch].misc.total_AmpareHour;	
			break;
		case 4:
			SOC = myData->cData[ch].misc.maxAuxV;
			break;
		case 5:
			SOC = myData->cData[ch].misc.sum_AmpareHour;
			break;
		default:
			SOC = 0;
			break;
	}
	//SOC, Capacity(Ah) End
	
	//Sequence Col Start
	//col 1: maxAuxT, 2: minAuxT, 3: canData(T)		
	switch(col) {
		case 1:
			temp = myData->cData[ch].misc.maxAuxT;
			break;
		case 2:
			temp = myData->cData[ch].misc.minAuxT;
			break;
		case 3:
			temp = myData->cData[ch].misc.can_data_l[1];	//CAN_RX_FUNC_DIV_C_TABLE_COL
			break;
		default:
			temp = 0;
			break;
	}
	//Sequence Col End
	
	//shhw_230525e

	//	temp = 15000.0;//KHK Test
	//	SOC = 100;//KHK Test
	tmp = myData->testCond[ch].SQ_Charge.col_num;
	if(tmp > MAX_SQ_COL_DATA) tmp = MAX_SQ_COL_DATA; //MAX_SQ_COL_DATA : 20
	if(tmp <= 0) tmp = 1;
	
	for(j=0; j < tmp; j++) { // Temp
		if(temp < myData->testCond[ch].SQ_Charge.COL[j]) break;
	}
	j = j - 1;
	if(j < 0){
		j = 0;
	}
	tmp = myData->testCond[ch].SQ_Charge.row_num;
	if(tmp > (MAX_SQ_ROW_DATA)) tmp = MAX_SQ_ROW_DATA; //MAX_SQ_ROW_DATA : 20
	if(tmp <= 0) tmp = 1;

	//shhw_230525s
	if(row2 > 1) { //using row2 option
		for(i = sequence_count; i < tmp; i++) { 
			if(row2_val < myData->testCond[ch].SQ_Charge.div_voltage[i][j]) break;
		}
		if((i >= tmp) && (row2_val >= myData->testCond[ch].SQ_Charge.div_voltage[tmp-1][j])) {
			myData->cData[ch].misc.sequence_end_flag = P1;
			i = tmp-1;
		}
	} 
	if(row > 1) { 
		for(k = sequence_count; k < tmp; k++) { // SOC
			if(SOC < myData->testCond[ch].SQ_Charge.ROW[k]) break;
		}
		if((k >= tmp) && (SOC >= myData->testCond[ch].SQ_Charge.ROW[tmp-1])) {
			myData->cData[ch].misc.sequence_end_flag = P1;
			k = tmp-1;
		}
	}
	if(row2 == 1 || k > i) i = k;
	//shhw_230525e
	
	sequence_count = i;
	i = i -1;
	if(i < 0){
	   	i = 0;
		sequence_count = 0;
	}
	myData->cData[ch].misc.sequence_count = sequence_count;
	val = myData->testCond[ch].SQ_Charge.limit_current[sequence_count][j];

	return (long)val;
}
//jhkw_221205e


