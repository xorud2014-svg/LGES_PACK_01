#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include "../../INC/datastore.h"
#include "common_utils.h"
#include "main.h"
#include "time.h" //kjhw_150331_ttt

volatile S_SYSTEM_DATA *myData;
int bd, file_write;
long min[10], max[10];

void Test_Print(void)
{
	int i, j;

	i = 0; if(i == 1) TimeSlot_Print();
	i = 0; if(i == 1) TimeSlot_Print2(); //kjg_240213
	i = 0; if(i == 1) DataStructureSize_Print();
	i = 0; if(i == 1) Debug_Print();

	i = 0; if(i == 1) AppControl_Print();
	i = 0; if(i == 1) COA_Client_Print();
	i = 0; if(i == 1) {
				for(j=0; j < myData->AppControl.config.totalGroup; j++)
					COB_Client_Print(j);
			}
	i = 0; if(i == 1) COC_Client_Print();

	i = 0; if(i == 1) DataSave_Print();

	i = 0; if(i == 1) {
				for(j=0; j < myData->AppControl.config.totalGroup; j++)
					Jig_Control_Print(j);
			}
	i = 0; if(i == 1) {
				for(j=0; j < myData->AppControl.config.totalGroup; j++)
					Job_Change_Print(j);
			}
	i = 0; if(i == 1) {
				for(j=0; j < myData->AppControl.config.totalGroup; j++)
					Pitch_Change_Print(j);
			}

	i = 0; if(i == 1) ModuleState_Print();
	i = 0; if(i == 1) GroupState_Print();
	i = 0; if(i == 1) BoardState_Print();
	i = 0; if(i == 1) ChannelState_Print();
	i = 0; if(i == 1) FCH_State_Print();
	i = 0; if(i == 1) {
				for(j=0; j < myData->AppControl.config.totalGroup; j++)
					COB_Channel_Print(j);
			}
	i = 0; if(i == 1) LineCheck_Print(0);

	i = 0; if(i == 1) IO_Print();
	i = 0; if(i == 1) TestCond_Print();
	i = 0; if(i == 1) CaliData_Print();
	i = 0; if(i == 1) SubSensV_Print();
	i = 0; if(i == 1) AuxV_Print();
	i = 0; if(i == 1) Humidity_Print();
	i = 0; if(i == 1) Gas_Print();
	i = 0; if(i == 1) AuxTemp_Print();
	i = 0; if(i == 1) Message_Print();
	i = 0; if(i == 1) COM_Print();
	i = 1; if(i == 1) CAN_Print();

	i = 0; if(i == 1) Chamber_Print();
	i = 0; if(i == 1) jhk_test();
	
	
	switch(file_write) {
		case 1:
			file_write = 0;
			data_10ms_write(0, 0);
			data_10ms_write(0, 1);
			data_10ms_write(1, 0);
			data_10ms_write(1, 1);
//			data_10ms_write(0, 2);
//			data_10ms_write(0, 3);
//			data_10ms_write(1, 2);
//			data_10ms_write(1, 3);
			break;
		case 2:
			file_write = 0;
			COA_Client_TestCond_write_V1009(0); //COA?
			break;
		case 3:
			file_write = 0;
			COA_Client_TestCond_write_V100B(0); //COA?
			break;
		case 4:
			file_write = 0;
			COA_Client_TestCond_write_V100C(0); //COA?
			break;
		case 5:
			file_write = 0;
			COA_Client_TestCond_write_V100D(0); //kjhw_150210
			COA_Client_TestCond_write_TimeSch(0); //kjhw_150210
			break;
		case 6:
			file_write = 0;
			COA_Client_TestCond_write_V100D_pattern(0); //COA?
			break;
		case 7:
			if(myData->mData.signal[M_SIG_EXIT_PHASE] > 0) {
				write_IO_Print(0);
			}
		default:
			file_write = 0;
			break;
	}
}

void TimeSlot_Print(void)
{
	int i, j, end_idx;
	long tmp1, tmp2;

	printf("TimeSlot\n");

	tmp1 = tmp2 = 0;
	switch(myData->mData.config.scan_period) {
		case 25: //25ms
			end_idx = 5;	break; //1ms
		case 50: //50ms
			end_idx = 10;	break; //1ms
		case 100: //100ms
			end_idx = 20;	break; //1ms
		case 200: //200ms
			end_idx = 20;	break; //2ms
		default: //10ms
			end_idx = 2;	break; //1ms
	}

	for(i=0; i < end_idx; i++) {
		printf("Time %02d ", i*5);
		for(j=0; j < 5; j++) {
			printf("%07ld ", myData->mData.runningTime[0][i*5 + j]);
			tmp1 += myData->mData.runningTime[0][i*5 + j];
			tmp2 += myData->mData.runningTime[1][i*5 + j];
		}
		printf("\n");
	}

	printf("runTime average [function]:%07ld, [all]:%07ld\n",
		tmp1/(end_idx*5), tmp2/(end_idx*5));

	printf("runTime max:%07ld[%ld], min:%07ld[%ld]\n",
		myData->mData.runningTime[2][0], myData->mData.runningTime[2][1],
		myData->mData.runningTime[3][0], myData->mData.runningTime[3][1]);

	printf("runTime max2:%07ld[%ld], min2:%07ld[%ld]\n",
		myData->mData.runningTime[4][0], myData->mData.runningTime[4][1],
		myData->mData.runningTime[5][0], myData->mData.runningTime[5][1]);

	printf("runTime/1000 max:%04ld[%ld], min:%04ld[%ld]\n",
		myData->mData.runningTime[2][0] / 1000,
		myData->mData.runningTime[2][1],
		myData->mData.runningTime[3][0] / 1000,
		myData->mData.runningTime[3][1]);

	//kjhw_140520s
	/*printf("runTime_main_slot_5msec val1_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][21], myData->mData.runningTime[0][22]);
	printf("runTime_main_slot_5msec val2_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][31], myData->mData.runningTime[0][32]);
	printf("runTime_main_slot_5msec val3_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][41], myData->mData.runningTime[0][42]);
	printf("runTime_main_slot_5msec val4_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][51], myData->mData.runningTime[0][52]);
	printf("runTime_main_slot_5msec val5_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][61], myData->mData.runningTime[0][62]);
	printf("runTime_main_slot_5msec val6_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][71], myData->mData.runningTime[0][72]);
	printf("runTime_main_slot_5msec val7_max:%07ld[%ld]\n",
		myData->mData.runningTime[0][81], myData->mData.runningTime[0][82]);
	//kjhw_140520e
*/
	printf("\n");
}

void TimeSlot_Print2(void)
{ //kjg_240213
	int i, j, end_idx;

	printf("TimeSlot2\n");

	switch(myData->mData.config.scan_period) {
		case 25: //25ms
			end_idx = 5;	break; //1ms
		case 50: //50ms
			end_idx = 10;	break; //1ms
		case 100: //100ms
			end_idx = 20;	break; //1ms
		case 200: //200ms
			end_idx = 20;	break; //2ms
		case 20: //20ms
			end_idx = 2;	break; //2ms
		default: //10ms
			end_idx = 2;	break; //1ms
	}

	for(i=0; i < end_idx; i++) {
		printf("Time %02d ", i*5);
		for(j=0; j < 5; j++) {
			printf("%07ld ", myData->mData.runningTime[0][i*5 + j]);
		}
		printf("\n");
	}

	printf("runTime val1/1000 max:%04ld[%ld], min:%04ld[%ld], avg:%04ld\n",
		myData->mData.runningTime[2][0] / 1000, myData->mData.runningTime[2][1],
		myData->mData.runningTime[2][2] / 1000, myData->mData.runningTime[2][3],
		myData->mData.runningTime[2][4] / 1000);

	printf("runTime val2/1000 max:%04ld[%ld], min:%04ld[%ld], avg:%04ld\n",
		myData->mData.runningTime[2][5] / 1000, myData->mData.runningTime[2][6],
		myData->mData.runningTime[2][7] / 1000, myData->mData.runningTime[2][8],
		myData->mData.runningTime[2][9] / 1000);

	printf("runTime val1(1s) max:%04ld[%ld], min:%04ld[%ld], avg:%04ld\n",
		myData->mData.runningTime[5][0] / 1000, myData->mData.runningTime[5][1],
		myData->mData.runningTime[5][2] / 1000, myData->mData.runningTime[5][3],
		myData->mData.runningTime[5][4] / 1000);

	printf("runTime val2(1s) max:%04ld[%ld], min:%04ld[%ld], avg:%04ld\n",
		myData->mData.runningTime[5][5] / 1000, myData->mData.runningTime[5][6],
		myData->mData.runningTime[5][7] / 1000, myData->mData.runningTime[5][8],
		myData->mData.runningTime[5][9] / 1000);

	printf("\n");
}

void DataStructureSize_Print(void)
{
	int i, j, size[100];

	memset(size, 0, sizeof size);
	i = j = 0;
	size[i] = sizeof(S_SYSTEM_DATA);
    printf("SysData size %d\n", size[i]);

	i++;
	size[i] = sizeof(S_CH_MISC);
	printf("s_ch_misce %d \n", size[i]);
	i++;
	size[i] = sizeof(S_MSG) * MAX_MSG_RING;
	printf("msg size %d = %d * %d\n", size[i], sizeof(S_MSG), MAX_MSG_RING);
	i++;
	size[i] = sizeof(S_SAVE_MSG) * MAX_SAVE_MSG_RING;
	printf("save_msg size %d = %d * %d\n", size[i], sizeof(S_SAVE_MSG),
		MAX_SAVE_MSG_RING);
#if defined __COB__ || defined __COC__
	i++;
	size[i] = sizeof(S_SAVE_MSG_1) * MAX_SAVE_MSG_RING;
	printf("save_msg_1 size %d = %d * %d\n", size[i], sizeof(S_SAVE_MSG_1),
		MAX_SAVE_MSG_RING);
	i++;
	size[i] = sizeof(S_RECORD_MSG_1) * MAX_RECORD_MSG_RING;
	printf("record_msg_1 size %d = %d * %d\n", size[i], sizeof(S_RECORD_MSG_1),
		MAX_RECORD_MSG_RING);
#endif
	i++;
	size[i] = sizeof(S_PULSE_MSG_1) * MAX_PULSE_MSG_RING;
	printf("pulse_msg_1 size %d = %d * %d\n", size[i], sizeof(S_PULSE_MSG_1),
		MAX_PULSE_MSG_RING);

#if defined __COB__
	i++;
	size[i] = sizeof(S_F_SAVE_MSG) * MAX_F_SAVE_MSG_RING;
	printf("f_save_msg size %d = %d * %d\n", size[i], sizeof(S_F_SAVE_MSG),
		MAX_F_SAVE_MSG_RING);
	i++;
	size[i] = sizeof(S_F_DCIR_SAVE_MSG) * MAX_GROUP_2;
	printf("f_save_dcir_msg size %d = %d * %d\n", size[i],
		sizeof(S_F_DCIR_SAVE_MSG), MAX_GROUP_2);
	i++;
	size[i] = sizeof(S_F_SAVE_MSG) * MAX_GROUP_2;
	printf("f_save_msg(real_data) size %d = %d * %d\n", size[i],
		sizeof(S_F_SAVE_MSG), MAX_GROUP_2);
#endif

	i++;
	size[i] = sizeof(S_LOGFILE) * MAX_LOG;
	printf("log size %d = %d * %d\n", size[i], sizeof(S_LOGFILE), MAX_LOG);

	i++;
	size[i] = sizeof(S_CALIBRATION) * MAX_CH_8;
	printf("cali size %d = %d * %d\n", size[i], sizeof(S_CALIBRATION),
		MAX_CH_8);

#if defined __COB__
	i++;
	size[i] = sizeof(S_F_CALIBRATION);
	printf("f_cali size %d\n", size[i]);
	i++;
	size[i] = sizeof(S_CALI_SET_DATA);
	printf("cali_set_data size %d\n", size[i]);
	i++;
	size[i] = sizeof(S_MSG_CH_DATA_FLAG);
	printf("cali_ch_flag size %d\n", size[i]);
	i++;
	size[i] = sizeof(S_MSG_CH_DATA_FLAG);
	printf("measure_ch_flag size %d\n", size[i]);

	i++;
	size[i] = sizeof(S_CH_TEMP) * MAX_CH_256;
	printf("TempArray1 size %d = %d * %d\n", size[i], sizeof(S_CH_TEMP),
		MAX_CH_256);
	i++;
	size[i] = sizeof(S_CH_TEMP) * MAX_CH_256;
	printf("TempArray2 size %d = %d * %d\n", size[i], sizeof(S_CH_TEMP),
		MAX_CH_256);
#endif

	i++;
	size[i] = sizeof(S_TEST_CONDITION) * MAX_TEST_COND_COUNT;
	printf("testCond size %d = %d * %d\n", size[i], sizeof(S_TEST_CONDITION),
		MAX_TEST_COND_4);
	printf("	common_object %d = %d * %d\n",
		sizeof(long) * MAX_COM_TEST_OBJECT, sizeof(long), MAX_COM_TEST_OBJECT);
	printf("	local_object %d = %d * %d * %d\n",
		sizeof(long) * MAX_TEST_STEP * MAX_LOC_TEST_OBJECT, sizeof(long),
		MAX_TEST_STEP, MAX_LOC_TEST_OBJECT);
	printf("	grade %d = %d * %d\n",
		sizeof(S_CH_GRADE) * MAX_TEST_STEP, sizeof(S_CH_GRADE), MAX_TEST_STEP);
	printf("	common_chCode %d = %d * %d\n",
		sizeof(S_CH_CODE) * MAX_COM_CODE, sizeof(S_CH_CODE), MAX_COM_CODE);
	printf("	local_chCode %d = %d * %d * %d\n",
		sizeof(S_CH_CODE) * MAX_TEST_STEP * MAX_LOC_CODE,
		sizeof(S_CH_CODE), MAX_TEST_STEP, MAX_LOC_CODE);
	printf("	aux_chCode %d = %d * %d\n",
		sizeof(S_CH_CODE) * MAX_AUX_CODE, sizeof(S_CH_CODE), MAX_AUX_CODE);
	printf("	reserved %d\n", sizeof(S_TEST_COND_RESERVED));
	printf("	pattern %d = %d * %d\n",
		sizeof(S_TEST_COND_PATTERN_DATA) * MAX_PATTERN_DATA,
		sizeof(S_TEST_COND_PATTERN_DATA), MAX_PATTERN_DATA);
	printf("	external_data %d = %d * %d\n",
		sizeof(S_TEST_COND_EXTERNAL_DATA) * 2,
		sizeof(S_TEST_COND_EXTERNAL_DATA), 2);

	i++;
	size[i] = sizeof(S_CH_NUMBER) * MAX_CH_256;
	printf("size CellArray1 %d = %d * %d\n", size[i], sizeof(S_CH_NUMBER),
		MAX_CH_256);
	i++;
	size[i] = sizeof(S_CH_NUMBER) * MAX_CH_256;
	printf("size CellArray2 %d = %d * %d\n", size[i], sizeof(S_CH_NUMBER),
		MAX_CH_256);

	i++;
	size[i] = sizeof(S_APP_CONTROL);
	printf("size AppControl %d\n", size[i]);

	i++;
	size[i] = sizeof(S_COA_CLIENT) * MAX_GROUP_1;
	printf("size COA_Client %d = %d * %d\n", size[i], sizeof(S_COA_CLIENT),
		MAX_GROUP_1);
	printf("	signal %d\n", sizeof(unsigned char) * MAX_SIGNAL);
	printf("	config %d\n", sizeof(S_P1_CONFIG));
	printf("	misc %d\n", sizeof(S_P1_MISC));
	printf("	testConfig %d\n", sizeof(S_P1_TEST_CONFIG));
	printf("	rcv_packet %d\n", sizeof(S_P1_RCV_PACKET));
	printf("	rcv_command %d\n", sizeof(S_P1_RCV_COMMAND));
	printf("	reply %d\n", sizeof(S_P1_REPLY));
	printf("	testCond %d\n", sizeof(S_P1_TEST_CONDITION));
	printf("	pulse_data %d = %d * %d\n",
		sizeof(S_P1_PULSE_DATA) * MAX_CH_8, sizeof(S_P1_PULSE_DATA), MAX_CH_8);

#if defined __COB__
	i++;
	size[i] = sizeof(S_COB_CLIENT) * MAX_GROUP_2;
	printf("size COB_Client %d = %d * %d\n", size[i], sizeof(S_COB_CLIENT),
		MAX_GROUP_2);
	printf("	signal %d\n", sizeof(unsigned char) * MAX_SIGNAL);
	printf("	config %d\n", sizeof(S_P2_CONFIG));
	printf("	configData %d\n", sizeof(S_P2_CONFIG_DATA));
	printf("	misc %d\n", sizeof(S_P2_MISC));
	printf("	testConfig %d\n", sizeof(S_P2_TEST_CONFIG));
	printf("	rcv_packet %d\n", sizeof(S_P2_RCV_PACKET));
	printf("	rcv_command %d\n", sizeof(S_P2_RCV_COMMAND));
	printf("	bcr %d\n", sizeof(S_P2_CMD_BCR_INFO));
	printf("	reply %d\n", sizeof(S_P2_REPLY));
	printf("	sensor_limit %d\n", sizeof(S_P2_SENSOR_LIMIT));
	printf("	chamber_set %d\n", sizeof(S_P2_CHAMBER_SET));
	printf("	testCond %d\n", sizeof(S_P2_TEST_CONDITION));
#endif

#if defined __COC__
	i++;
	size[i] = sizeof(S_COC_CLIENT) * MAX_GROUP_1;
	printf("size COC_Client %d = %d * %d\n", size[i], sizeof(S_COC_CLIENT),
		MAX_GROUP_1);
#endif

#if defined __COD__
	i++;
	size[i] = sizeof(S_COC_CLIENT) * MAX_GROUP_1; //kjg_w
	printf("size COD_Client %d = %d * %d\n", size[i], sizeof(S_COC_CLIENT),
		MAX_GROUP_1);
#endif

	i++;
	size[i] = sizeof(S_DATA_SAVE);
	printf("size DataSave %d\n", size[i]);

	i++;
	size[i] = sizeof(S_MODULE_DATA);
	printf("size mData %d\n", size[i]);

	i++;
	size[i] = sizeof(S_GROUP_DATA) * MAX_GROUP_2;
	printf("size gData %d = %d * %d\n", size[i], sizeof(S_GROUP_DATA),
		MAX_GROUP_2);

#if defined __COB__
	i++;
	size[i] = sizeof(S_BD_DATA) * MAX_BD_32;
	printf("size bdData %d = %d * %d\n", size[i], sizeof(S_BD_DATA), MAX_BD_32);
#else
	i++;
	size[i] = sizeof(S_BD_DATA) * MAX_BD_16;
	printf("size bdData %d = %d * %d\n", size[i], sizeof(S_BD_DATA), MAX_BD_16);
#endif

	i++;
	size[i] = sizeof(S_CH_DATA) * MAX_CH_256;
	printf("size cData %d = %d * %d\n", size[i], sizeof(S_CH_DATA), MAX_CH_256);

#if defined __COC__
	i++;
	size[i] = sizeof(S_FCH_DATA);
	printf("size fch %d\n", size[i]);
#endif

	i++;
	size[i] = sizeof(S_PWM3_DATA);
	printf("size pwm3_ch %d\n", size[i]);

	i++;
	size[i] = sizeof(S_LINE_CHECK) * MAX_GROUP_2 * MAX_CH_256;
	printf("size line_check %d = %d %d\n", size[i],
		MAX_GROUP_2, MAX_CH_256);

#if defined __COB__
	i++;
	size[i] = sizeof(int) * MAX_GROUP_2 * MAX_CH_256;
	printf("size COB_save_step %d = %d * %d * %d\n", size[i], sizeof(int),
		MAX_GROUP_2, MAX_CH_256);
	i++;
	size[i] = sizeof(S_CH_OP_DATA)
		* MAX_GROUP_2 *  MAX_CH_256 * MAX_P2_STEP;
	printf("size COB_opSave %d = %d * %d * %d * %d\n", size[i],
		sizeof(S_CH_OP_DATA), MAX_GROUP_2, MAX_CH_256, MAX_P2_STEP);
	i++;
	size[i] = sizeof(long) * MAX_GROUP_2 * MAX_CH_256;
	printf("size dcr_check %d = %d * %d * %d\n", size[i], sizeof(long),
		MAX_GROUP_2, MAX_CH_256);
#endif

	i++;
	size[i] = sizeof(S_ADDR_MAP);
	printf("size addr_map %d\n", size[i]);
	i++;
	size[i] = sizeof(S_DIO);
	printf("size dio %d\n", size[i]);
#if defined __COB__
	i++;
	size[i] = sizeof(S_JIG_DATA) * MAX_JIG_8;
	printf("size jigData %d = %d * %d\n", size[i], sizeof(S_JIG_DATA),
		MAX_JIG_8);
#endif

	i++;
	size[i] = sizeof(S_CH_ATTRIBUTE) * MAX_CH_256;
	printf("size chAttr %d = %d * %d\n", size[i], sizeof(S_CH_ATTRIBUTE),
		MAX_CH_256);

	i++;
	size[i] = sizeof(S_SUB_SENS_V);
	printf("size SubSensV %d\n", size[i]);

	i++;
	size[i] = sizeof(int) * MAX_CH_256 * MAX_AUX_TYPE;
	printf("size auxDataCount %d = %d * %d\n", size[i], sizeof(int),
		MAX_CH_256 * MAX_AUX_TYPE);
	i++;
	size[i] = sizeof(S_AUX_SET_DATA) * MAX_AUX_DATA;
	printf("size auxSetData %d = %d * %d\n", size[i], sizeof(S_AUX_SET_DATA),
		MAX_AUX_DATA);

	i++;
	size[i] = sizeof(S_CAN);
	printf("size CAN %d\n", size[i]);
	i++;
	size[i] = sizeof(int) * MAX_CH_8 * MAX_CAN_TYPE;
	printf("size canReceiveDataCount %d = %d * %d\n", size[i], sizeof(int),
		MAX_CH_8 * MAX_CAN_TYPE);
	i++;
	size[i] = sizeof(S_CAN_RECEIVE_SET_DATA);
	printf("size canReceiveSetData %d\n", size[i]);
	i++;
	size[i] = sizeof(int) * MAX_CH_8 * MAX_CAN_TYPE;
	printf("size canTransmitDataCount %d = %d * %d\n", size[i], sizeof(int),
		MAX_CH_8 * MAX_CAN_TYPE);
	i++;
	size[i] = sizeof(S_CAN_TRANSMIT_SET_DATA);
	printf("size canTransmitSetData %d\n", size[i]);
	i++;
	size[i] = sizeof(long) * MAX_CH_8 * MAX_CAN_DATA;
	printf("size canTransmitTime %d = %d * %d\n", size[i], sizeof(long),
		MAX_CH_8 * MAX_CAN_DATA);
	i++;
	size[i] = sizeof(S_CAN_TRANSMIT_CHANGE);
	printf("size canTransmitChange %d\n", size[i]);
	i++;
	size[i] = sizeof(U_CAN_VAL) * MAX_CH_8 * MAX_CAN_DATA;
	printf("size CanData %d = %d * %d * %d\n", size[i], sizeof(U_CAN_VAL),
		MAX_CH_8, MAX_CAN_DATA);

	i++;
	size[i] = sizeof(S_COM);
	printf("size COM %d\n", size[i]);

	i++;
	size[i] = sizeof(S_MEASURE_CALI) * 2 * MAX_SUB_SENS_V_DATA;
	printf("size measure_cali %d = %d * %d\n", size[i], sizeof(S_MEASURE_CALI),
		2 * MAX_SUB_SENS_V_DATA);

	i++;
	size[i] = sizeof(S_TH_TABLE) * MAX_TH_TABLE;
	printf("size th_table %d = %d * %d\n", size[i], sizeof(S_TH_TABLE),
		MAX_TH_TABLE);

	i++;
	size[i] = sizeof(unsigned char) * 8 * MAX_TEST_VALUE;
	printf("size test_val_uc %d = %d * %d\n", size[i], sizeof(unsigned char),
		8 * MAX_TEST_VALUE);
	i++;
	size[i] = sizeof(long) * MAX_TEST_VALUE;
	printf("size test_val_l1 %d = %d * %d\n", size[i], sizeof(long),
		MAX_TEST_VALUE);
	i++;
	size[i] = sizeof(long long) * MAX_TEST_VALUE;
	printf("size test_val_ll1 %d = %d * %d\n", size[i], sizeof(long long),
		MAX_TEST_VALUE);
	i++;
	size[i] = sizeof(float) * MAX_TEST_VALUE;
	printf("size test_val_f %d = %d * %d\n", size[i], sizeof(float),
		MAX_TEST_VALUE);

	i++;
	size[i] = sizeof(int) * 2;
	printf("size data_10ms_count %d\n", size[i]);
	i++;
	size[i] = sizeof(long) * 2 * 4 * 2000;
	printf("size data_10ms %d = %d * %d\n", size[i], sizeof(long),
		2 * 4 * 2000);
	i++;
	size[i] = sizeof(long) * 2 * 4 * 2000;
	printf("size data_10ms_tmp %d = %d * %d\n", size[i], sizeof(long),
		2 * 4 * 2000);

	i++;
	j = i;
	for(i=1; i < j; i++) {
		size[j] += size[i];
	}
	printf("total %d : %d\n", size[j], size[0]);

	printf("\n");
}

void Debug_Print(void)
{
	int i, j;

	printf("Debug \n");

	/*printf("test_val_f ");
	for(i=0; i < MAX_TEST_VALUE; i++) {
		printf("%f ", myData->test_val_f[i]);
	}
	printf("\n");*/

	printf("test_val_l1 ");
	for(i=0; i < MAX_TEST_VALUE; i++) {
		//printf("%lx ", myData->test_val_l1[i]);
		printf("%ld ", myData->test_val_l1[i]);
	}
	printf("\n");

	printf("test_val_l2 ");
	for(i=0; i < MAX_TEST_VALUE; i++) {
		printf("%ld ", myData->test_val_l2[i]);
	}
	printf("\n");

	for(j=0; j < 8; j++) {
		printf("test_val_i[%d] ", j);
		for(i=0; i < MAX_TEST_VALUE; i++) {
			if(i == 1 || i == 2 || i == 7 || i == 8) {
				printf("%03d ", myData->test_val_i[j][i]);
			} else if(i == 5 || i == 11) {
				printf("%06d ", myData->test_val_i[j][i]);
			} else {
				printf("%02d ", myData->test_val_i[j][i]);
			}
		}
		printf("\n");
	}

	printf("test_val_ll1 ");
	for(i=0; i < MAX_TEST_VALUE; i++) {
		//printf("%llx ", myData->test_val_ll1[i]);
		printf("%lld ", myData->test_val_ll1[i]);
	}
	printf("\n");

	printf("test_val_ll hex ");
	for(i=0; i < MAX_TEST_VALUE; i++) {
		printf("%llx ", myData->test_val_ll1[i]);
	}
	printf("\n");

	/*	//kjg_141114
	printf("sort_count:%d\n",
		myData->tmpCanData_sort_count[0]);
	printf("(1) %f, %f, %f\n",
		myData->tmpCanData_sort[0][0][0].f_val[0],
		myData->tmpCanData_sort[0][0][1].f_val[0],
		myData->tmpCanData_sort[0][0][2].f_val[0]);
	printf("(2) %f, %f, %f\n",
		myData->tmpCanData_sort[0][1][0].f_val[0],
		myData->tmpCanData_sort[0][1][1].f_val[0],
		myData->tmpCanData_sort[0][1][2].f_val[0]);
	printf("(3) %f, %f, %f\n",
		myData->tmpCanData_sort[0][2][0].f_val[0],
		myData->tmpCanData_sort[0][2][1].f_val[0],
		myData->tmpCanData_sort[0][2][2].f_val[0]);
	printf("(4) %f, %f, %f\n",
		myData->tmpCanData_sort[0][3][0].f_val[0],
		myData->tmpCanData_sort[0][3][1].f_val[0],
		myData->tmpCanData_sort[0][3][2].f_val[0]);
	printf("(5) %f, %f, %f\n",
		myData->tmpCanData_sort[0][4][0].f_val[0],
		myData->tmpCanData_sort[0][4][1].f_val[0],
		myData->tmpCanData_sort[0][4][2].f_val[0]);
	printf("(6) %f, %f, %f\n",
		myData->tmpCanData_sort[0][5][0].f_val[0],
		myData->tmpCanData_sort[0][5][1].f_val[0],
		myData->tmpCanData_sort[0][5][2].f_val[0]);
	printf("(7) %f, %f, %f\n",
		myData->tmpCanData_sort[0][6][0].f_val[0],
		myData->tmpCanData_sort[0][6][1].f_val[0],
		myData->tmpCanData_sort[0][6][2].f_val[0]);	*/
		
	/*for(j=0; j < 8; j++) {
		printf("test_val_uc[%d] ", j);
		for(i=0; i < MAX_TEST_VALUE; i++) {
			printf("%x ", myData->test_val_uc[j][i]);
			//printf("%d ", myData->test_val_uc[j][i]);
		}
		printf("\n");
	}

	for(j=0; j < 8; j++) { //kjg_200331
		printf("test_val_ll2[%d] ", j);
		for(i=0; i < MAX_TEST_VALUE; i++) {
			printf("%lld ", myData->test_val_ll2[j][i]);
		}
		printf("\n");
	}*/

	printf("\n");
}

void AppControl_Print(void)
{
	int i, j;

	printf("AppControl %d\n", myData->AppControl.config.systemModel);
	printf("app signal %d, %d, %d(%d)\n",
		myData->AppControl.signal[APP_SIG_APP_CONTROL_PROCESS],
		myData->AppControl.signal[APP_SIG_COA1_CLIENT_PROCESS],
		myData->AppControl.signal[APP_SIG_ANALOG_METER_PROCESS],
		myData->AppControl.signal[APP_SIG_ANALOG_METER_PROCESS_CHECK]);
	printf("totalGroup %d\n", myData->AppControl.config.totalGroup);

	printf("process_etc");
	for(i=0; i < MAX_PROCESS_GROUP; i++) {
		for(j=0; j < MAX_GROUP_8; j++) {
			printf(" %d",
				myData->AppControl.misc.Load_Process[i][j]);
		}
		printf("\n");
	}

	printf("signal %d\n",
		myData->AppControl.signal[APP_SIG_CALI_UPDATE]);

	printf("\n");
}

void COA_Client_Print(void)
{ //debug_size_coa
	int group;

	printf("COA_Client\n");

	group = 0;
	printf("groupId %d, groupNo %d, time %ld\n",
		myData->COA_Client[group].config.groupId,
		myData->COA_Client[group].config.groupNo,
		myData->mData.misc.timer_1sec);
	printf("netTimeout %ld, ping_time %ld, %ld, net_time %ld, %ld\n",
		myData->COA_Client[group].config.netTimeout,
		myData->COA_Client[group].misc.ping_time,
		myData->COA_Client[group].misc.ping_time2,
		myData->COA_Client[group].misc.net_time,
		myData->COA_Client[group].misc.net_time2);
	printf("send_monitor_data_interval %ld, sent_monitor_data_time %ld, %ld\n",
		myData->COA_Client[group].config.send_monitor_data_interval,
		myData->COA_Client[group].misc.sent_monitor_data_time,
		myData->COA_Client[group].misc.sent_monitor_data_time2);
	printf("send_save_data_interval %ld\n",
		myData->COA_Client[group].config.send_save_data_interval);
	printf("signal %d, %d\n",
		myData->COA_Client[group].signal[P1_SIG_NET_CONNECTED],
		myData->COA_Client[group].signal[P1_SIG_SEND_REAL_TIME_REQUEST]);

	printf("\n");
	printf("testCond.header\n");
	printf("totalStep:%d, totalPatternCount:%d, totalTimeSchCount:%d\n",
		myData->COA_Client[group].common_safety_cond_update
			.testCond_header.totalStep,
		myData->COA_Client[group].common_safety_cond_update
			.testCond_header.totalPatternCount,
		myData->COA_Client[group].common_safety_cond_update
			.testCond_header.totalTimeSchCount);

	printf("testCond.safety\n");
	printf("faultLowerV:%ld, faultUpperV:%ld, faultCompAuxV:%ld\n",
		myData->COA_Client[group].common_safety_cond_update
			.testCond_safety.faultLowerV,
		myData->COA_Client[group].common_safety_cond_update
			.testCond_safety.faultUpperV,
		myData->COA_Client[group].common_safety_cond_update
			.testCond_safety.faultCompAuxV);

	printf("\n");
	printf("testCond.step\n");
	printf("type:%d, stepNo:%d, mode:%d, testEnd:%d, subStep:%d\n",
		myData->COA_Client[group].step_cond_update
			.testCond_step.header.type,
		myData->COA_Client[group].step_cond_update
			.testCond_step.header.stepNo,
		myData->COA_Client[group].step_cond_update
			.testCond_step.header.mode,
		myData->COA_Client[group].step_cond_update
			.testCond_step.header.testEnd,
		myData->COA_Client[group].step_cond_update
			.testCond_step.header.subStep);
	printf("endTime_day:%ld, endTime:%ld\n",
		myData->COA_Client[group].step_cond_update
			.testCond_step.reference[0].endTime_day,
		myData->COA_Client[group].step_cond_update
			.testCond_step.reference[0].endTime);

	printf("\n");
	
	/*	//ktg_190822s
		printf("faultCompAuxV : %ld\n",
			myData->COA_Client[0].testCond.step[1].faultCompAuxV);
		printf("faultCompAuxTemp : %ld\n",
			myData->COA_Client[0].testCond.step[1].faultCompAuxTemp);
		printf("faultCompAuxTh : %ld\n",
			myData->COA_Client[0].testCond.step[1].faultCompAuxTh);
		printf("faultCompAuxT : %ld\n",
			myData->COA_Client[0].testCond.step[1].faultCompAuxT);
		printf("faultDelta_AuxV : %ld\n",
			myData->COA_Client[0].testCond.step[1].faultDelta_AuxV);
		printf("faultDelta_AuxV_T : %ld\n",
			myData->COA_Client[0].testCond.step[1].faultDelta_AuxV_T);
	*/	//ktg_190822e
}

void COB_Client_Print(int group)
{ //debug_size_cob
/*	printf("COB_Client(%d) state:%d, tmpState:%d, code:%d, tmpCode:%d, mode:%d\n", group+1,
		(int)myData->COB_Client[group].misc.state,
		(int)myData->COB_Client[group].misc.tmpState,
		(int)myData->COB_Client[group].misc.code,
		(int)myData->COB_Client[group].misc.tmpCode,
		(int)myData->COB_Client[group].config.workMode);

	printf("groupId %d, groupNo %d, time %ld\n",
		myData->COB_Client[group].config.groupId,
		myData->COB_Client[group].config.groupNo,
		myData->mData.misc.timer_1sec);
	printf("netTimer %ld %ld, netTimeout %ld\n",
		myData->COB_Client[group].misc.net_time,
		myData->COB_Client[group].misc.net_time2,
		myData->COB_Client[group].config.netTimeout);
	printf("send_monitor_data_interval %ld, sent_monitor_data_time %ld, %ld\n",
		myData->COB_Client[group].config.send_monitor_data_interval,
		myData->COB_Client[group].misc.sent_monitor_data_time,
		myData->COB_Client[group].config.send_save_data_interval);
	printf("signal %d\n",
		myData->COB_Client[group].signal[P2_SIG_NET_CONNECTED]);
	printf("chOffset %d\n",
		myData->COB_Client[group].misc.chOffset);
	printf("sent_sensor_data_time %ld, %ld, sent_chamber_data_time %ld, %ld\n",
		myData->COB_Client[group].misc.sent_sensor_data_time,
		myData->COB_Client[group].misc.sent_sensor_data_time2,
		myData->COB_Client[group].misc.sent_chamber_data_time,
		myData->COB_Client[group].misc.sent_chamber_data_time2);

	printf("\n");*/
}

void COC_Client_Print(void)
{ //debug_size_coc
#if defined __COC__ //kjg_171223
	int group;

	printf("COC_Client\n");

	group = 0;
	printf("groupId %d, groupNo %d, time %ld\n",
		myData->COC_Client[group].config.groupId,
		myData->COC_Client[group].config.groupNo,
		myData->mData.misc.timer_1sec);
	printf("netTimer %ld %ld, netTimeout %ld\n",
		myData->COC_Client[group].misc.net_time,
		myData->COC_Client[group].misc.net_time2,
		myData->COC_Client[group].config.netTimeout);
	printf("send_monitor_data_interval %ld, sent_monitor_data_time %ld, %ld\n",
		myData->COC_Client[group].config.send_monitor_data_interval,
		myData->COC_Client[group].misc.sent_monitor_data_time,
		myData->COC_Client[group].misc.sent_monitor_data_time2);
	printf("send_save_data_interval %ld\n",
		myData->COC_Client[group].config.send_save_data_interval);
	printf("signal %d, %d\n",
		myData->COC_Client[group].signal[P3_SIG_NET_CONNECTED],
		myData->COC_Client[group].signal[P3_SIG_SEND_REAL_TIME_REQUEST]);

	printf("\n");
#endif
}

void DataSave_Print(void)
{
	printf("DataSave\n");
	printf("\n");
}

void Jig_Control_Print(int jig)
{
#if defined __COB__
	printf("Jig_Control(%d) state:%d, code:%d, phase:%d, emgPhase:%d\n", jig+1,
		(int)myData->jData[jig].jigState,
		(int)myData->jData[jig].code,
		(int)myData->jData[jig].microState,
		(int)myData->jData[jig].emgPhase);
	printf("timer_1sec %ld, emgCheckDelay %ld\n",
		myData->mData.misc.timer_1sec, myData->jData[jig].emgCheckDelay);
/*
	printf("jigUseFlag:%d, totalTrayNo:%d\n",
		myData->jData[jig].config.jigUseFlag,
		myData->jData[jig].config.totalTrayNo);
*/
	printf("tray_state:%d, tmp_tray_state:%d, stopper_state:%d, door_state:%d\n",
		(int)myData->jData[jig].trayState[0],
		(int)myData->jData[jig].tmpTrayState[0],
		(int)myData->jData[jig].stopperState,
		(int)myData->jData[jig].doorState[0]);

	printf("jigNotMove %d\n", myData->jData[jig].jigNotMove);
/*
	printf("tmpTrayState:%d %ld %ld\n",
		(int)myData->jData[jig].tmpTrayState[0],
		myData->mData.misc.timer_1sec,
		myData->jData[jig].traySensingDelay);

	printf("tray_state_ext %d, tray_state_dir %d\n\n",
		Read_InPoint(1, 1, I_JIG_TRAY_STATE),
		Read_InPoint(1, 2, I_JIG_TRAY_STATE));

	printf("latch_left_open %d, latch_right_open %d\n",
		Read_InPoint(1, 0, I_JIG_LATCH_CYLINDER_L_OFF),
		Read_InPoint(1, 0, I_JIG_LATCH_CYLINDER_R_OFF));

	printf("latch_left_close %d, latch_right_close %d\n",
		Read_InPoint(1, 0, I_JIG_LATCH_CYLINDER_L_ON),
		Read_InPoint(1, 0, I_JIG_LATCH_CYLINDER_R_ON));

	printf("main_off %d, main_on %d\n",
		Read_InPoint(1, 0, I_JIG_MAIN_CYLINDER_L_OFF),
		Read_InPoint(1, 0, I_JIG_MAIN_CYLINDER_L_ON));
*/
	printf("fire1 %d, fire2 %d, chamber_fault %d, chamber_power %d\n",
		Read_InPoint(1, 0, I_JIG_FIRE_SENS),
		Read_InPoint(1, 0, I_JIG_SMOKE_SENS),
		Read_InPoint(1, 0, I_CHAMBER_FAULT),
		Read_InPoint(0, 0, I_JIG_CHAMBER_POWER));

	printf("door %d\n", myData->jData[jig].signal[J_SIG_DOOR_OPEN]);

	printf("\n");
#endif
}

void Job_Change_Print(int jig)
{
#if defined __COB__
	printf("Job_Change_Control[%d], state:%d, phase:%d, end:%d\n",
		jig+1,
		(int)myData->jData[jig].signal[J_SIG_JOB_CHANGE],
		(int)myData->jData[jig].jobChangePhase,
		(int)myData->jData[jig].jobChangeEnd);
	printf("\n");
#endif
}

void Pitch_Change_Print(int jig)
{
#if defined __COB__
	printf("Pitch_Change_Control[%d], state:%d, phase:%d, end:%d\n",
		jig+1,
		(int)myData->jData[jig].signal[J_SIG_PITCH_CHANGE],
		(int)myData->jData[jig].pitchChangePhase,
		(int)myData->jData[jig].pitchChangeEnd);
	printf("\n");
#endif
}

void ModuleState_Print(void)
{
	char buf[24];
	int i;
	long realDate, realClock;

	realDate = realClock = 0;

	printf("Module state : ");
	memset(buf, 0, sizeof buf);
	switch(myData->mData.state) {
		case M_IDLE:	strcpy(buf, "IDLE"); 		break;
		case M_STANDBY:	strcpy(buf, "STANDBY");		break;
		case M_RUN: 	strcpy(buf, "RUN"); 		break;
		case M_PAUSE:	strcpy(buf, "PAUSE"); 		break;
		case M_CALI:	strcpy(buf, "CALI"); 		break;
		case M_FAIL:	strcpy(buf, "FAIL");		break;
		default: 		strcpy(buf, "UNKNOWN");		break;
	}
	printf("%s\n", buf);

	printf("phase : %d, code : %d\n", myData->mData.phase, myData->mData.code);
	printf("timer_1sec %ld, increment_period %ld\n",
		myData->mData.misc.timer_1sec,
		myData->mData.misc.increment_period);
	printf("timer_1000ms %ld, increment_period %ld\n",
		myData->mData.misc.timer_1000ms,
		myData->mData.misc.increment_period);
	printf("chPerBd %d, installedBd %d, installedCh %d\n",
		myData->mData.config.chPerBd,
		myData->mData.config.installedBd,
		myData->mData.config.installedCh);
	printf("fan_run_count %ld, fan_stop_count %ld\n",
		myData->mData.misc.fan_run_count,
		myData->mData.misc.fan_stop_count);

	printf("maxV ");
	for(i=0; i < MAX_RANGE; i++) {
		printf("%ld ", myData->mData.config.maxV[i]);
	}
	printf("\n");

	printf("maxI ");
	for(i=0; i < MAX_RANGE; i++) {
		printf("%ld ", myData->mData.config.maxI[i]);
	}
	printf("\n");

	printf("signal exit_type %d, exit_phase %d, fan_relay %d, fan_active %d\n",
		myData->mData.signal[M_SIG_EXIT_TYPE],
		myData->mData.signal[M_SIG_EXIT_PHASE],
		myData->mData.signal[M_SIG_FAN_RELAY],
		myData->mData.signal[M_SIG_FAN_ACTIVE]);
/*
	printf("fan_delay_time %ld, timer_1sec %ld\n",
		myData->mData.misc.fan_delay_time, myData->mData.misc.timer_1sec);
	printf("fan_fail_detect_time %ld, time2 %ld\n",
		myData->mData.misc.fan_fail_detect_time,
		myData->mData.misc.fan_fail_detect_time2);

	printf("real_time ");
	for(i=0; i < 7; i++) {
		printf("%ld ", myData->mData.real_time[i]);
	}
	printf("\n");

	realDate
		= myData->mData.real_time[6] * 10000 //year
		+ myData->mData.real_time[5] * 100 //month
		+ myData->mData.real_time[4]; //day
	realClock
		= myData->mData.real_time[3] * 10000000 //hour
		+ myData->mData.real_time[2] * 100000 //min
		+ myData->mData.real_time[1] * 1000 //sec
		+ myData->mData.real_time[0]; //msec
	printf("real_clock %ld, %ld\n", realDate, realClock);
*/
/*	printf("ref_muxVal %d\n",
		myData->mData.misc.ref_muxVal);

	printf("signal %d %d %d %d %d\n",
		myData->mData.signal[M_SIG_CALI_GROUP],
		myData->mData.signal[M_SIG_CALI_BD_START],
		myData->mData.signal[M_SIG_CALI_VI_SELECT],
		myData->mData.signal[M_SIG_CALI_CD_SELECT],
		myData->mData.signal[M_SIG_CALI_STEP]);
*/
	printf("exit_value %d\n", myData->mData.misc.exit_value);

	printf("TOWER_LAMP signal red(%d) amber(%d) green(%d) blue(%d)\
				   	buzzer(%d/%d)\n",
		myData->mData.signal[M_SIG_TOWER_LAMP_RED],
		myData->mData.signal[M_SIG_TOWER_LAMP_AMBER],
		myData->mData.signal[M_SIG_TOWER_LAMP_GREEN],
		myData->mData.signal[M_SIG_TOWER_LAMP_BLUE],
		myData->mData.signal[M_SIG_TOWER_LAMP_BUZZER],
		myData->mData.signal[M_SIG_BUZZER_SET]);
/*
	printf("signal %d %d %d\n",
		myData->mData.signal[M_SIG_RUNNING_GROUP],
		myData->mData.signal[M_SIG_CABLE_CHECK_PHASE],
		myData->mData.signal[M_SIG_CELL_CHECK_PHASE]);
*/
/*
	printf("cvtn_id %x, sw_version %x %x %x %x, hvil %d, dtc %x %x %x\n",
		myData->mData.bms_eol_data.cvtn_id,
		myData->mData.bms_eol_data.sw_version[0],
		myData->mData.bms_eol_data.sw_version[1],
		myData->mData.bms_eol_data.sw_version[2],
		myData->mData.bms_eol_data.sw_version[3],
		myData->mData.bms_eol_data.HvBattHvil_D_Fault,
		myData->mData.bms_eol_data.DTC[0],
		myData->mData.bms_eol_data.DTC[1],
		myData->mData.bms_eol_data.DTC[2]);

	printf("module_dt %f, thermistor %f, cell_dv %f\n",
		myData->mData.bms_eol_data.module_delta_temp,
		myData->mData.bms_eol_data.thermistor_sensor,
		myData->mData.bms_eol_data.cell_delta_v);

	printf("Aux %d %d %d %d, Main %d %d %d %d, Neg %d %d %d %d\n",
		myData->mData.bms_eol_data.HvBattAuxCntct_D_Actl[0],
		myData->mData.bms_eol_data.HvBattAuxCntct_D_Actl[1],
		myData->mData.bms_eol_data.HvBattAuxCntct_D_Actl[2],
		myData->mData.bms_eol_data.HvBattAuxCntct_D_Actl[3],
		myData->mData.bms_eol_data.HvBattCntct_D_Actl[0],
		myData->mData.bms_eol_data.HvBattCntct_D_Actl[1],
		myData->mData.bms_eol_data.HvBattCntct_D_Actl[2],
		myData->mData.bms_eol_data.HvBattCntct_D_Actl[3],
		myData->mData.bms_eol_data.HvBattNeg_D_Actl[0],
		myData->mData.bms_eol_data.HvBattNeg_D_Actl[1],
		myData->mData.bms_eol_data.HvBattNeg_D_Actl[2],
		myData->mData.bms_eol_data.HvBattNeg_D_Actl[3]);

	printf("Pack_V %f %f %f %f, HvBatt_V %f %f %f %f\n",
		myData->mData.bms_eol_data.Cycler_Pack_V[0],
		myData->mData.bms_eol_data.Cycler_Pack_V[1],
		myData->mData.bms_eol_data.Cycler_Pack_V[2],
		myData->mData.bms_eol_data.Cycler_Pack_V[3],
		myData->mData.bms_eol_data.HvBatt_U_Actl[0],
		myData->mData.bms_eol_data.HvBatt_U_Actl[1],
		myData->mData.bms_eol_data.HvBatt_U_Actl[2],
		myData->mData.bms_eol_data.HvBatt_U_Actl[3]);

	printf("480D_V %f %f %f %f, Link_V %f %f %f %f, Charg_V %f %f %f %f\n",
		myData->mData.bms_eol_data.DID_480D_Pack_V[0],
		myData->mData.bms_eol_data.DID_480D_Pack_V[1],
		myData->mData.bms_eol_data.DID_480D_Pack_V[2],
		myData->mData.bms_eol_data.DID_480D_Pack_V[3],
		myData->mData.bms_eol_data.Link_V[0],
		myData->mData.bms_eol_data.Link_V[1],
		myData->mData.bms_eol_data.Link_V[2],
		myData->mData.bms_eol_data.Link_V[3],
		myData->mData.bms_eol_data.Charge_V[0],
		myData->mData.bms_eol_data.Charge_V[1],
		myData->mData.bms_eol_data.Charge_V[2],
		myData->mData.bms_eol_data.Charge_V[3]);

	printf("Pack2_V %f %f %f %f, Cha_Fuse_V %f %f %f %f, Elac_Fuse_V %f %f %f %f\n",
		myData->mData.bms_eol_data.Pack2_V[0],
		myData->mData.bms_eol_data.Pack2_V[1],
		myData->mData.bms_eol_data.Pack2_V[2],
		myData->mData.bms_eol_data.Pack2_V[3],
		myData->mData.bms_eol_data.Charger_Fuse_V[0],
		myData->mData.bms_eol_data.Charger_Fuse_V[1],
		myData->mData.bms_eol_data.Charger_Fuse_V[2],
		myData->mData.bms_eol_data.Charger_Fuse_V[3],
		myData->mData.bms_eol_data.Elac_Fuse_V[0],
		myData->mData.bms_eol_data.Elac_Fuse_V[1],
		myData->mData.bms_eol_data.Elac_Fuse_V[2],
		myData->mData.bms_eol_data.Elac_Fuse_V[3]);

	printf("iso_res %f, iso_Va %f, iso_Vb %f\n",
		myData->mData.bms_eol_data.isolation_resistance,
		myData->mData.bms_eol_data.isolation_Va,
		myData->mData.bms_eol_data.isolation_Vb);

	i = 0;
	printf("dtc list\n");
	printf("%x %x %x %x, %x %x %x %x, %x %x %x %x, %x %x %x %x, %x %x %x %x\n",
		myData->mData.bms_eol_data.DTC_List[i][0],
		myData->mData.bms_eol_data.DTC_List[i][1],
		myData->mData.bms_eol_data.DTC_List[i][2],
		myData->mData.bms_eol_data.DTC_List[i][3],
		myData->mData.bms_eol_data.DTC_List[i+1][0],
		myData->mData.bms_eol_data.DTC_List[i+1][1],
		myData->mData.bms_eol_data.DTC_List[i+1][2],
		myData->mData.bms_eol_data.DTC_List[i+1][3],
		myData->mData.bms_eol_data.DTC_List[i+2][0],
		myData->mData.bms_eol_data.DTC_List[i+2][1],
		myData->mData.bms_eol_data.DTC_List[i+2][2],
		myData->mData.bms_eol_data.DTC_List[i+2][3],
		myData->mData.bms_eol_data.DTC_List[i+3][0],
		myData->mData.bms_eol_data.DTC_List[i+3][1],
		myData->mData.bms_eol_data.DTC_List[i+3][2],
		myData->mData.bms_eol_data.DTC_List[i+3][3],
		myData->mData.bms_eol_data.DTC_List[i+4][0],
		myData->mData.bms_eol_data.DTC_List[i+4][1],
		myData->mData.bms_eol_data.DTC_List[i+4][2],
		myData->mData.bms_eol_data.DTC_List[i+4][3]);

	printf("dtc_data ");
	for(i=0; i < 100; i++) {
		printf("%02x ", myData->mData.misc.dtc_data[i]);
	}
	printf("\n");

	printf("tx_phase_uds_data %d %d %d %d\n",
		myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA1],
		myData->CAN.signal[1][CAN_SIG_TX_PHASE_UDS_DATA1],
		myData->CAN.signal[2][CAN_SIG_TX_PHASE_UDS_DATA1],
		myData->CAN.signal[3][CAN_SIG_TX_PHASE_UDS_DATA1]);
*/
	printf("can_flash total_block:%d, point:%d %d %d %d, size:%d %d %d %d\n",
		myData->CAN.can_flash_data_total_block,
		myData->CAN.can_flash_data_block_point[0],
		myData->CAN.can_flash_data_block_point[1],
		myData->CAN.can_flash_data_block_point[2],
		myData->CAN.can_flash_data_block_point[3],
		myData->CAN.can_flash_data_block_size[0],
		myData->CAN.can_flash_data_block_size[1],
		myData->CAN.can_flash_data_block_size[2],
		myData->CAN.can_flash_data_block_size[3]);

	printf("block_count %d, block_point %d, uds_ff_count %d, uds_tx_size %d, uds_tx_block_size %d\n",
		myData->CAN.block_count, myData->CAN.block_point, myData->CAN.uds_ff_count, myData->CAN.uds_tx_size, myData->CAN.uds_tx_block_size);

	printf("can_flash_file b1 : %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x\n",
		myData->CAN.can_flash_file[1065-4],
		myData->CAN.can_flash_file[1065-3],
		myData->CAN.can_flash_file[1065-2],
		myData->CAN.can_flash_file[1065-1],
		myData->CAN.can_flash_file[1065],
		myData->CAN.can_flash_file[1065+1],
		myData->CAN.can_flash_file[1065+2],
		myData->CAN.can_flash_file[1065+3],
		myData->CAN.can_flash_file[1065+4],
		myData->CAN.can_flash_file[1065+5],
		myData->CAN.can_flash_file[1065+6],
		myData->CAN.can_flash_file[1065+7]);

	printf("can_flash_file b2 : %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x\n",
		myData->CAN.can_flash_file[1123-4],
		myData->CAN.can_flash_file[1123-3],
		myData->CAN.can_flash_file[1123-2],
		myData->CAN.can_flash_file[1123-1],
		myData->CAN.can_flash_file[1123],
		myData->CAN.can_flash_file[1123+1],
		myData->CAN.can_flash_file[1123+2],
		myData->CAN.can_flash_file[1123+3],
		myData->CAN.can_flash_file[1123+4],
		myData->CAN.can_flash_file[1123+5],
		myData->CAN.can_flash_file[1123+6],
		myData->CAN.can_flash_file[1123+7]);

	printf("can_flash_file b3 : %02x %02x %02x %02x, %02x %02x %02x %02x, %02x %02x %02x %02x\n",
		myData->CAN.can_flash_file[1141-4],
		myData->CAN.can_flash_file[1141-3],
		myData->CAN.can_flash_file[1141-2],
		myData->CAN.can_flash_file[1141-1],
		myData->CAN.can_flash_file[1141],
		myData->CAN.can_flash_file[1141+1],
		myData->CAN.can_flash_file[1141+2],
		myData->CAN.can_flash_file[1141+3],
		myData->CAN.can_flash_file[1141+4],
		myData->CAN.can_flash_file[1141+5],
		myData->CAN.can_flash_file[1141+6],
		myData->CAN.can_flash_file[1141+7]);

	printf("signal eol_procedure_index %d, eol_procedure_phase %d\n",
		myData->mData.signal[M_SIG_EOL_PROCEDURE_INDEX],
		myData->mData.signal[M_SIG_EOL_PROCEDURE_PHASE]);

	printf("signal eol %d, eol_flash %d, uds_data_index %d, uds_tx_size %d\n",
		myData->mData.signal[M_SIG_BMS_EOL_PHASE],
		myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE],
		myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX],
		myData->CAN.uds_tx_size);

	printf("coa %s, pack_id %s, %s, %s, %s\n",
		myData->COA_Client[0].misc.write_pack_id,
		myData->mData.bms_eol_data2.pack_id[0],
		myData->mData.bms_eol_data2.pack_id[1],
		myData->mData.bms_eol_data2.pack_id[2],
		myData->mData.bms_eol_data2.pack_id[3]);
/*
	printf("signal eol3 %d\n",
		myData->mData.signal[M_SIG_BMS_EOL3_PHASE]);
	printf("eol3 ubms_state %x, sw %x, hw %x, id %x\n",
		myData->mData.bms_eol_data3.UBMS_State,
		myData->mData.bms_eol_data3.SW_Version,
		myData->mData.bms_eol_data3.HW_Version,
		myData->mData.bms_eol_data3.UBMS_ID);
	printf("eol3 cali_v %d %d %d %d\n", myData->mData.bms_eol_data3.cali_v[0],
		myData->mData.bms_eol_data3.cali_v[1],
		myData->mData.bms_eol_data3.cali_v[2],
		myData->mData.bms_eol_data3.cali_v[3]);
	printf("eol3 cali_val %d %d %d %d\n",
		myData->mData.bms_eol_data3.cali_val[0],
		myData->mData.bms_eol_data3.cali_val[1],
		myData->mData.bms_eol_data3.cali_val[2],
		myData->mData.bms_eol_data3.cali_val[3]);
	printf("eol3 cell_v %d %d %d %d\n", myData->mData.bms_eol_data3.cell_v[0],
		myData->mData.bms_eol_data3.cell_v[1],
		myData->mData.bms_eol_data3.cell_v[2],
		myData->mData.bms_eol_data3.cell_v[3]);
	printf("eol3 temp %d, m_v %d, obd %d\n",
		myData->mData.bms_eol_data3.temp,
		myData->mData.bms_eol_data3.module_v,
		myData->mData.bms_eol_data3.obd);
*/
	printf("\n");
}

void GroupState_Print(void)
{
	char buf[24];
	int i;

	printf("Group state ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		memset(buf, 0, sizeof buf);
		switch(myData->gData[i].state) {
			case G_IDLE:	strcpy(buf, "IDLE"); 		break;
			case G_STANDBY:	strcpy(buf, "STANDBY");		break;
			case G_RUN: 	strcpy(buf, "RUN"); 		break;
			case G_PAUSE:	strcpy(buf, "PAUSE"); 		break;
			case G_CALI:	strcpy(buf, "CALI"); 		break;
			default: 		strcpy(buf, "UNKNOWN");		break;
		}
		printf("%s ", buf);
	}
	printf("\n");

	printf("phase ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].phase);
	}
	printf("\n");

	printf("code ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].code);
	}
	printf("\n");

	printf("attribute ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].attribute);
	}
	printf("\n");
/*
	printf("workMode ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].workMode);
	}
	printf("\n");

	printf("group_control_mode ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].group_control_mode);
	}
	printf("\n");
*/
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("scan_ch:%d %d, scan_step:%d %d, scan_idxStepNo_offset:%d %d, rest_area:%d %d, %d %d\n",
			myData->gData[i].misc.scan_ch[0],
			myData->gData[i].misc.scan_ch[1],
			myData->gData[i].misc.scan_step[0],
			myData->gData[i].misc.scan_step[1],
			myData->gData[i].misc.scan_idxStepNo_offset[0],
			myData->gData[i].misc.scan_idxStepNo_offset[1],
			myData->gData[i].misc.long_rest_area1[10],
			myData->gData[i].misc.long_rest_area2[10],
			myData->gData[i].misc.long_rest_area1[11],
			myData->gData[i].misc.long_rest_area2[11]);
	}

	printf("total_ng_ch:%d %d, tmp_scan_step:%d %d, %d %d\n",
		myData->gData[0].misc.total_ng_ch,
		myData->gData[1].misc.total_ng_ch,
		myData->gData[0].misc.tmp_scan_step[0],
		myData->gData[0].misc.tmp_scan_step[1],
		myData->gData[1].misc.tmp_scan_step[0],
		myData->gData[1].misc.tmp_scan_step[1]);
/*
	printf("chOffset ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].misc.chOffset);
	}
	printf("\n");

	printf("chInGroup ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->mData.config.chInGroup[i]);
	}
	printf("\n");

	printf("runTime ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%ld ", myData->gData[i].runTime);
	}
	printf("\n");

	printf("rest1 ");
	for(i=0; i < myData->mData.config.chInGroup[0]; i++) {
		printf("%ld ", myData->gData[0].misc.long_rest_time1[i]);
	}
	printf("\n");

	printf("rest2 ");
	for(i=0; i < myData->mData.config.chInGroup[0]; i++) {
		printf("%ld ", myData->gData[0].misc.long_rest_time2[i]);
	}
	printf("\n");
*/
	printf("signal exit_phase ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].signal[G_SIG_EXIT_PHASE]);
	}
	printf("\n");
/*
	printf("signal net_check ");
	for(i=0; i < myData->AppControl.config.totalGroup; i++) {
		printf("%d ", myData->gData[i].signal[G_SIG_NET_CHECK]);
	}
	printf("\n");
*/
	printf("\n");
}

void BoardState_Print(void)
{
	int i;

	printf("BoardState bd(%d)\n", bd+1);

	printf("sumV(+) ");
	for(i=0; i < myData->mData.config.ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[0].sumV[i]);
	}
	printf("\n");

	if(min[0] == 0) min[0] = myData->bData[bd].misc.source[0].sumV[0];
	if(max[0] == 0) max[0] = myData->bData[bd].misc.source[0].sumV[0];
	for(i=0; i < myData->mData.config.ad_count; i++) {
		if(myData->bData[bd].misc.source[0].sumV[i] < min[0])
			min[0] = myData->bData[bd].misc.source[0].sumV[i];
		if(myData->bData[bd].misc.source[0].sumV[i] > max[0])
			max[0] = myData->bData[bd].misc.source[0].sumV[i];
	}
	printf("min %ld, max %ld : %ld\n", min[0], max[0], min[0]-max[0]);

	printf("sumV(-) ");
	for(i=0; i < myData->mData.config.ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[1].sumV[i]);
	}
	printf("\n");

	if(min[1] == 0) min[1] = myData->bData[bd].misc.source[1].sumV[0];
	if(max[1] == 0) max[1] = myData->bData[bd].misc.source[1].sumV[0];
	for(i=0; i < myData->mData.config.ad_count; i++) {
		if(myData->bData[bd].misc.source[1].sumV[i] < min[1])
			min[1] = myData->bData[bd].misc.source[1].sumV[i];
		if(myData->bData[bd].misc.source[1].sumV[i] > max[1])
			max[1] = myData->bData[bd].misc.source[1].sumV[i];
	}
	printf("min %ld, max %ld : %ld\n", min[1], max[1], min[1]-max[1]);

	printf("sumV(0) ");
	for(i=0; i < myData->mData.config.ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[2].sumV[i]);
	}
	printf("\n");

	printf("sensSumV(+) ");
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[0].sensSumV[i]);
	}
	printf("\n");

	if(min[4] == 0) min[4] = myData->bData[bd].misc.source[0].sensSumV[0];
	if(max[4] == 0) max[4] = myData->bData[bd].misc.source[0].sensSumV[0];
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		if(myData->bData[bd].misc.source[0].sensSumV[i] < min[4])
			min[4] = myData->bData[bd].misc.source[0].sensSumV[i];
		if(myData->bData[bd].misc.source[0].sensSumV[i] > max[4])
			max[4] = myData->bData[bd].misc.source[0].sensSumV[i];
	}
	printf("min %ld, max %ld : %ld\n", min[4], max[4], min[4]-max[4]);

	printf("sensSumV(-) ");
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[1].sensSumV[i]);
	}
	printf("\n");

	if(min[5] == 0) min[5] = myData->bData[bd].misc.source[1].sensSumV[0];
	if(max[5] == 0) max[5] = myData->bData[bd].misc.source[1].sensSumV[0];
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		if(myData->bData[bd].misc.source[1].sensSumV[i] < min[5])
			min[5] = myData->bData[bd].misc.source[1].sensSumV[i];
		if(myData->bData[bd].misc.source[1].sensSumV[i] > max[5])
			max[5] = myData->bData[bd].misc.source[1].sensSumV[i];
	}
	printf("min %ld, max %ld : %ld\n", min[5], max[5], min[5]-max[5]);

	printf("ref_muxVal:%d, bd_sensCount:%d\n",
		myData->mData.misc.ref_muxVal,
		myData->mData.misc.bd_sensCount);
	printf("sensSumV(0) ");
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[2].sensSumV[i]);
	}
	printf("\n");

	if(min[8] == 0) min[8] = myData->bData[bd].misc.source[2].sensSumV[0];
	if(max[8] == 0) max[8] = myData->bData[bd].misc.source[2].sensSumV[0];
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		if(myData->bData[bd].misc.source[2].sensSumV[i] < min[8])
			min[8] = myData->bData[bd].misc.source[2].sensSumV[i];
		if(myData->bData[bd].misc.source[2].sensSumV[i] > max[8])
			max[8] = myData->bData[bd].misc.source[2].sensSumV[i];
	}
	printf("min %ld, max %ld : %ld\n", min[8], max[8], min[8]-max[8]);

	printf("sourceV ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source[i].sourceV);
	}
	printf("\n");

	printf("source2_sumV(+) ");
	for(i=0; i < 10; i++) {
		printf("%ld ", myData->bData[bd].misc.source2[0].sumV[i]);
	}
	printf("\n");

	printf("source2_totalV ");
	for(i=0; i < 4; i++) {
		printf("%f ", myData->bData[bd].misc.source2[i].totalV);
	}
	printf("\n");

	printf("source2V ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source2[i].sourceV);
	}
	printf("\n");
	printf("\n");

	printf("sumI(+) ");
	for(i=0; i < myData->mData.config.ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[0].sumI[i]);
	}
	printf("\n");

	if(min[2] == 0) min[2] = myData->bData[bd].misc.source[0].sumI[0];
	if(max[2] == 0) max[2] = myData->bData[bd].misc.source[0].sumI[0];
	for(i=0; i < myData->mData.config.ad_count; i++) {
		if(myData->bData[bd].misc.source[0].sumI[i] < min[2])
			min[2] = myData->bData[bd].misc.source[0].sumI[i];
		if(myData->bData[bd].misc.source[0].sumI[i] > max[2])
			max[2] = myData->bData[bd].misc.source[0].sumI[i];
	}
	printf("min %ld, max %ld : %ld\n", min[2], max[2], min[2]-max[2]);

	printf("sumI(-) ");
	for(i=0; i < myData->mData.config.ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[1].sumI[i]);
	}
	printf("\n");

	if(min[3] == 0) min[3] = myData->bData[bd].misc.source[1].sumI[0];
	if(max[3] == 0) max[3] = myData->bData[bd].misc.source[1].sumI[0];
	for(i=0; i < myData->mData.config.ad_count; i++) {
		if(myData->bData[bd].misc.source[1].sumI[i] < min[3])
			min[3] = myData->bData[bd].misc.source[1].sumI[i];
		if(myData->bData[bd].misc.source[1].sumI[i] > max[3])
			max[3] = myData->bData[bd].misc.source[1].sumI[i];
	}
	printf("min %ld, max %ld : %ld\n", min[3], max[3], min[3]-max[3]);

	printf("sumI(0) ");
	for(i=0; i < myData->mData.config.ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[2].sumI[i]);
	}
	printf("\n");

	printf("sensSumI(+) ");
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[0].sensSumI[i]);
	}
	printf("\n");

	if(min[6] == 0) min[6] = myData->bData[bd].misc.source[0].sensSumI[0];
	if(max[6] == 0) max[6] = myData->bData[bd].misc.source[0].sensSumI[0];
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		if(myData->bData[bd].misc.source[0].sensSumI[i] < min[6])
			min[6] = myData->bData[bd].misc.source[0].sensSumI[i];
		if(myData->bData[bd].misc.source[0].sensSumI[i] > max[6])
			max[6] = myData->bData[bd].misc.source[0].sensSumI[i];
	}
	printf("min %ld, max %ld : %ld\n", min[6], max[6], min[6]-max[6]);

	printf("sensSumI(-) ");
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", myData->bData[bd].misc.source[1].sensSumI[i]);
	}
	printf("\n");

	if(min[7] == 0) min[7] = myData->bData[bd].misc.source[1].sensSumI[0];
	if(max[7] == 0) max[7] = myData->bData[bd].misc.source[1].sensSumI[0];
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		if(myData->bData[bd].misc.source[1].sensSumI[i] < min[7])
			min[7] = myData->bData[bd].misc.source[1].sensSumI[i];
		if(myData->bData[bd].misc.source[1].sensSumI[i] > max[7])
			max[7] = myData->bData[bd].misc.source[1].sensSumI[i];
	}
	printf("min %ld, max %ld : %ld\n", min[7], max[7], min[7]-max[7]);

	printf("sourceI ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source[i].sourceI);
	}
	printf("\n");

	printf("source2I ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source2[i].sourceI);
	}
	printf("\n");
	printf("\n");

	printf("calSourceV ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source[i].calSourceV);
	}
	printf("\n");

	printf("calSourceV AD_A:%f, AD_B:%f, AD_A_N:%f, AD_B_N:%f\n",
		myData->bData[bd].misc.Vsource_AD_a,
		myData->bData[bd].misc.Vsource_AD_b,
		myData->bData[bd].misc.Vsource_AD_a_N,
		myData->bData[bd].misc.Vsource_AD_b_N);

	printf("calSourceI ");
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->bData[bd].misc.source[i].calSourceI);
	}
	printf("\n");

	printf("calSourceI AD_A:%f, AD_B:%f, AD_A_N:%f, AD_B_N:%f\n",
		myData->bData[bd].misc.Isource_AD_a,
		myData->bData[bd].misc.Isource_AD_b,
		myData->bData[bd].misc.Isource_AD_a_N,
		myData->bData[bd].misc.Isource_AD_b_N);

	printf("source_sensCount %d, totalV %f, totaI %f\n",
		myData->mData.misc.source_sensCount,
		myData->bData[bd].misc.source2[0].totalV,
		myData->bData[bd].misc.source2[0].totalI);

	printf("\n");
}

void ChannelState_Print(void)
{
	char buf[24];
	int i, j, tmp, ch_start, ch_end;

	j = 0;
	printf("=================================================\n");
	printf("ChannelState bd(%d) %d %d\n", bd+1,
		myData->gData[0].misc.scan_ch[0]+1, myData->gData[0].misc.scan_ch[1]+1);

	ch_start = myData->mData.config.chPerBd * bd;
	//ch_end = ch_start + myData->mData.config.chPerBd;
	ch_end = ch_start + 4;

	printf("state : ");
	for(i=ch_start; i < ch_end; i++) {
		memset(buf, 0, sizeof buf);
		tmp = (int)myData->cData[i].op.state;
		switch(tmp) {
			case C_IDLE:	strcpy(buf, "IDLE"); 		break;
			case C_STANDBY:	strcpy(buf, "STANDBY");		break;
			case C_RUN: 	strcpy(buf, "RUN"); 		break;
			case C_PAUSE:	strcpy(buf, "PAUSE"); 		break;
			case C_CALI:	strcpy(buf, "CALI"); 		break;
			case C_FAULT:	strcpy(buf, "FAULT");		break;
			default: 		strcpy(buf, "UNKNOWN");		break;
		}
		printf("%s ", buf);
	}
	printf("\n");
/*
	printf("tmpState : ");
	for(i=ch_start; i < ch_end; i++) {
		memset(buf, 0, sizeof buf);
		tmp = (int)myData->cData[i].misc.tmpState;
		switch(tmp) {
			case C_IDLE:	strcpy(buf, "IDLE"); 		break;
			case C_STANDBY:	strcpy(buf, "STANDBY");		break;
			case C_RUN: 	strcpy(buf, "RUN"); 		break;
			case C_PAUSE:	strcpy(buf, "PAUSE"); 		break;
			case C_CALI:	strcpy(buf, "CALI"); 		break;
			case C_FAULT:	strcpy(buf, "FAULT");		break;
			default: 		strcpy(buf, "UNKNOWN");		break;
		}
		printf("%s ", buf);
	}
	printf("\n");
*/
	printf("phase : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.phase);
	}
	printf("\n");

	printf("stepType : ");
	for(i=ch_start; i < ch_end; i++) {
		memset(buf, 0, sizeof buf);
		tmp = (int)myData->cData[i].op.stepType;
		switch(tmp) {
			case STEP_IDLE:			strcpy(buf, "IDLE"); 		break;
			case STEP_CHARGE:		strcpy(buf, "CHARGE"); 		break;
			case STEP_DISCHARGE:	strcpy(buf, "DISCHARGE"); 	break;
			case STEP_REST:			strcpy(buf, "REST"); 		break;
			case STEP_LONG_TIME_REST:	strcpy(buf, "LONG_TIME_REST"); 	break;
			case STEP_OCV:			strcpy(buf, "OCV"); 		break;
			case STEP_Z:			strcpy(buf, "Z"); 			break;
			case STEP_END:			strcpy(buf, "END");			break;
			case STEP_CYCLE:		strcpy(buf, "CYCLE");		break;
			case STEP_LOOP:			strcpy(buf, "LOOP");		break;
			case STEP_PATTERN:		strcpy(buf, "PATTERN");		break;
			case STEP_EXTERNAL_CAN:	strcpy(buf, "EXT_CAN");		break;
			case STEP_USERMAP:		strcpy(buf, "USERMAP");		break;
			case STEP_EXTERNAL_CAN_2:	strcpy(buf, "EXT_CAN_2");	break;
			default:				strcpy(buf, "UNKNOWN");		break;
		}
		printf("%s ", buf);
	}
	printf("\n");

	printf("stepMode : ");
	for(i=ch_start; i < ch_end; i++) {
		memset(buf, 0, sizeof buf);
		tmp = (int)myData->cData[i].op.stepMode;
		switch(tmp) {
			case MODE_IDLE:	strcpy(buf, "IDLE");	break;
			case MODE_CC_CV:	strcpy(buf, "CCCV");	break;
			case MODE_CC:	strcpy(buf, "CC");		break;
			case MODE_CV:	strcpy(buf, "CV");		break;
			case MODE_DC:	strcpy(buf, "DC");		break;
			case MODE_AC:	strcpy(buf, "AC");		break;
			case MODE_CP:	strcpy(buf, "CP");		break;
			case MODE_CCP:	strcpy(buf, "CCP");		break;
			case MODE_CR:	strcpy(buf, "CR");		break;
			default:		strcpy(buf, "UNKNOWN");	break;
		}
		printf("%s ", buf);
	}
	printf("\n");

	printf("code : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.code);
	}
	printf("\n");
/*
	printf("T_can_lin_flag[0][0] : %ld\n",
		myData->canTransmitSetData.commonData[0][0].can_lin_flag);
	printf("T_can_lin_flag[0][1] : %ld\n",
		myData->canTransmitSetData.commonData[0][1].can_lin_flag);
	printf("T_can_lin_flag[1][0] : %ld\n",
		myData->canTransmitSetData.commonData[1][0].can_lin_flag);
	printf("T_can_lin_flag[1][1] : %ld\n",
		myData->canTransmitSetData.commonData[1][1].can_lin_flag);
	printf("R_can_lin_flag[0][0] : %ld\n",
		myData->canReceiveSetData.commonData[0][0].can_lin_flag);
	printf("R_can_lin_flag[0][1] : %ld\n",
		myData->canReceiveSetData.commonData[0][1].can_lin_flag);
	printf("R_can_lin_flag[1][0] : %ld\n",
		myData->canReceiveSetData.commonData[1][0].can_lin_flag);
	printf("R_can_lin_flag[1][1] : %ld\n",
		myData->canReceiveSetData.commonData[1][1].can_lin_flag);
*/
/*
	printf("sub_code : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.sub_code);
	}
	printf("\n");

	printf("tmpCode : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.tmpCode);
	}
	printf("\n");

	printf("saveCode : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.saveCode);
	}
	printf("\n");

	printf("attr : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.attribute);
	}
	printf("\n");
*/
	printf("idxStepNo : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.idxStepNo);
	}
	printf("\n");

	printf("comm_state : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%x ", myData->cData[i].misc.external_comm_state);
	}
	printf("\n");
/*
	printf("can_comm_check           : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->ChAttribute[i].can_comm_check);
	}
	printf("\n");

	printf("can_comm_step_check_flag : ");
	for(i=ch_start; i < ch_end; i++) {
		j = myData->cData[i].op.idxStepNo;
		printf("%ld ", myData->testCond[i]
			.local_object[j][IDX_LOC_OBJ_CAN_COMM_STEP_CHECK_FLAG]);
	}
	printf("\n");

	printf("chamber_con              : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->ChAttribute[i].chamber_control);
	}
	printf("\n");

	printf("chamber_step_check_flag  : ");
	for(i=ch_start; i < ch_end; i++) {
		j = myData->cData[i].op.idxStepNo;
		printf("%ld ", myData->testCond[i]
			.local_object[j][IDX_LOC_OBJ_CHAMBER_STEP_CHECK_FLAG]);
	}
	printf("\n");

	printf("chamber_standby          : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->ChAttribute[i].chamber_standby);
	}
	printf("\n");

	printf("out_mux_use              : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->ChAttribute[i].out_mux_use);
	}
	printf("\n");

	printf("ch_output_state : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%x ", myData->cData[i].misc.ch_output_state);
	}
	printf("\n");

	printf("ch_input_state : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%x ", myData->cData[i].misc.ch_input_state);
	}
	printf("\n");
*/
	printf("active_div : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.active_division);
	}
	printf("\n");
/*
	printf("grade\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.grade);
	}
	printf("\n");

	printf("temp\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.temp);
	}
	printf("\n");

	printf("sensCount\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.sensCount);
	}
	printf("\n");

	printf("sumV ");
	for(tmp=ch_start; tmp < ch_end; tmp++) {
		for(i=0; i < myData->mData.config.ad_count; i++) {
			printf("%ld ", myData->cData[tmp].misc.sumV[i]);
		}
		printf(",");
	}
	printf("\n");

	printf("sensSumV ");
	for(tmp=ch_start; tmp < ch_end; tmp++) {
		for(i=0; i < myData->mData.config.filter_ad_count; i++) {
			printf("%ld ", myData->cData[tmp].misc.sensSumV[i]);
		}
		printf(",");
	}
	printf("\n");

	if(min[8] == 0) min[8] = myData->cData[0].op.Vsens;
	if(max[8] == 0) max[8] = myData->cData[0].op.Vsens;
	if(min[8] > myData->cData[0].op.Vsens) {
		min[8] = myData->cData[0].op.Vsens;
	}
	if(max[8] < myData->cData[0].op.Vsens) {
		max[8] = myData->cData[0].op.Vsens;
	}

	printf("meanV\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.meanV);
	}
	printf("\n");

	printf("meanI\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.meanI);
	}
	printf("\n");

	printf("AD_flage %d, %d\n",
		myData->test_val_uc[0][0], myData->test_val_uc[0][1]);
*/
	printf("voltage ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.Vsens);
	}
	printf(" min:%ld, max:%ld\n", min[8], max[8]);
/*
	printf("tmpVsens ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.tmpVsens);
	}
	printf("\n");

	printf("Vpower ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.Vpower);
	}
	printf("\n");

	printf("Vbus ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.Vbus);
	}
	printf("\n");
*/
/*
	printf("vs_value\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.vs_value);
	}
	printf("\n");
	printf("tmpVsens : %ld, diff : %ld, val2_cap[1] : %ld, val2_cap[2] : %ld\n ", 
			myData->test_val_l1[0], myData->test_val_l1[1], myData->test_val_l1[2], myData->test_val_l1[3]);
*/
/*
	i=1;
	if(i == 0) {
		if(min[8] == 0) min[8] = myData->cData[0].misc.tmpVsens;
		if(max[8] == 0) max[8] = myData->cData[0].misc.tmpVsens;
		if(myData->cData[0].misc.tmpVsens < min[8])
			min[8] = myData->cData[0].misc.tmpVsens;
		if(myData->cData[0].misc.tmpVsens > max[8])
			max[8] = myData->cData[0].misc.tmpVsens;
	} else {
		if(min[8] == 0) min[8] = myData->cData[0].op.Vsens;
		if(max[8] == 0) max[8] = myData->cData[0].op.Vsens;
		if(myData->cData[0].op.Vsens < min[8])
			min[8] = myData->cData[0].op.Vsens;
		if(myData->cData[0].op.Vsens > max[8])
			max[8] = myData->cData[0].op.Vsens;
	}
	printf("min=%ld, max=%ld : %ld\n", min[8], max[8], min[8]-max[8]);

	printf("sumI ");
	for(i=0; i < myData->mData.config.ad_count; i++) {
		printf("%ld ", myData->cData[bd*4].misc.sumI[i]);
	}
	printf("\n");

	printf("sensSumI ");
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", myData->cData[bd*4].misc.sensSumI[i]);
	}
	printf("\n");

	printf("total_Isens\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.total_Isens);
	}
	printf("\n");
*/
	printf("current ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.Isens);
	}
	printf("\n");

	/*
	printf("current(save) ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].opSave.Isens);
	}
	printf("\n");

	printf("misc.maxV ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.maxV);
	}
	printf("\n");

	printf("misc.minV ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.minV);
	}
	printf("\n");

	printf("misc.maxI ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.maxI);
	}
	printf("\n");

	printf("misc.minI ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.minI);
	}
	printf("\n");

	printf("delay_count1\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.delay_count1);
	}
	printf("\n");

	printf("tmpIsens\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.tmpIsens);
	}
	printf("\n");

	i=1;
	if(i == 0) {
		if(min[9] == 0) min[9] = myData->cData[0].misc.tmpIsens;
		if(max[9] == 0) max[9] = myData->cData[0].misc.tmpIsens;
		if(myData->cData[0].misc.tmpIsens < min[9])
			min[9] = myData->cData[0].misc.tmpIsens;
		if(myData->cData[0].misc.tmpIsens > max[9])
			max[9] = myData->cData[0].misc.tmpIsens;
	} else {
		if(min[9] == 0) min[9] = myData->cData[0].op.Isens;
		if(max[9] == 0) max[9] = myData->cData[0].op.Isens;
		if(myData->cData[0].op.Isens < min[9])
			min[9] = myData->cData[0].op.Isens;
		if(myData->cData[0].op.Isens > max[9])
			max[9] = myData->cData[0].op.Isens;
	}
	printf("min=%ld, max=%ld : %ld\n", min[9], max[9], min[9]-max[9]);

	if(myData->cData[ch].Isens < min[8]) {
		min[8] = myData->cData[ch].op.Isens;
		for(i=0; i < myData->mData.config.filter_ad_count; i++) {
			min[i] = myData->cData[ch].misc.sensSumI[i];
		}
	}
	if(myData->cData[ch].Isens > max[8]) {
		max[8] = myData->cData[ch].op.Isens;
		for(i=0; i < myData->mData.config.filter_ad_count; i++) {
			max[i] = myData->cData[ch].misc.sensSumI[i];
		}
	}
	printf("min %ld : ", min[8]);
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", min[i]);
	}
	printf("\n");
	printf("max %ld : ", max[8]);
	for(i=0; i < myData->mData.config.filter_ad_count; i++) {
		printf("%ld ", max[i]);
	}
	printf("\n");

	printf("v_power_bus\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->fch.ch[i].v_power_bus);
	}
	printf("\n");

	printf("v_output_bus\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->fch.ch[i].v_output_bus);
	}
	printf("\n");

	printf("d_r1\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.d_r1);
	}
	printf("\n");

	printf("d_r2\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.d_r2);
	}
	printf("\n");

	printf("patternTime\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.pattern_time);
	}
	printf("\n");
*/
	printf("runTime ");
	for(i=ch_start; i < ch_end; i++) {
		printf("(%ld):%ld ", myData->cData[i].op.runTime_day, myData->cData[i].op.runTime);
	}
	printf("\n");

	printf("totalRunTime\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("(%ld):%ld ", myData->cData[i].op.totalRunTime_day, myData->cData[i].op.totalRunTime);
	}
	printf("\n");
/*
	printf("sendDataCount\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.sendDataCount);
	}
	printf("\n");

	//kjhw_131206s
	printf("sum_CycleTime\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("(%ld):%ld ",
		myData->cData[i].misc.sum_CycleTime_day,
		myData->cData[i].misc.sum_CycleTime);
	}
	printf("\n");
	printf("increment_period %ld\n", myData->mData.misc.increment_period);

	printf("elementCycle.cycle_idxStepNo\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].elementCycle.cycle_idxStepNo);
	}
	printf("\n");
	printf("elementCycle.loop_idxStepNo\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].elementCycle.loop_idxStepNo);
	}
	printf("\n");
	//kjhw_131206e
*/

	printf("checkDelayTime : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.checkDelayTime);
	}
	printf("\n");
/*
	printf("cvTime\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.cvTime);
	}
	printf("\n");

	printf("cvTime\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("(%ld):%ld ", myData->cData[i].op.cvTime_day, myData->cData[i].op.cvTime);
	}
	printf("\n");

	printf("maxI\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.maxI);
	}
	printf("\n");

	printf("minI\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.minI);
	}
	printf("\n");

	printf("watt ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%010ld ", myData->cData[i].op.watt);
	}
	printf("\n");

	printf("tmpWatt ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%010ld ", myData->cData[i].misc.tmpWatt);
	}
	printf("\n");

	printf("wattHour ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%010ld ", myData->cData[i].op.wattHour);
	}
	printf("\n");

	printf("charge_AmpareHour\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.charge_AmpareHour);
	}
	printf("\n");

	printf("discharge_AmpareHour\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.discharge_AmpareHour);
	}
	printf("\n");

	printf("sumCapacity\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%f ", myData->cData[i].misc.sumCapacity);
	}
	printf("\n");

	printf("seedCapacity\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%f ", myData->cData[i].misc.seedCapacity);
	}
	printf("\n");

	printf("capacitiance\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.capacitance);
	}
	printf("\n");

	printf("ccv\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->mData.ccv[bd][i].value);
	}
	printf("\n");

	printf("d_voltage\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.d_voltage);
	}
	printf("\n");

	printf("z\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.z);
	}
	printf("\n");

	printf("pid_ui1\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%f ", myData->cData[i].misc.pid_ui1[1]);
	}
	printf("\n");

	printf("pid_error1\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%f ", myData->cData[i].misc.pid_error1[1]);
	}
	printf("\n");

	printf("elementCycle.cycle_idxStepNo\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].elementCycle.cycle_idxStepNo);
	}
	printf("\n");

	printf("elementCycle.cycle_count\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].elementCycle.cycle_count);
	}
	printf("\n");

	printf("totalCycle\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.totalCycle);
	}
	printf("\n");
*/
	printf("semiSwitch : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.semiSwitchState);
	}
	printf("\n");
/*
	printf("reservedCmd\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.reservedCmd);
	}
	printf("\n");

	printf("endState ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.endState);
	}
	printf("\n");
*/
	printf("rangeV ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.rangeV);
	}
	printf("\n");

	printf("rangeI ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].op.rangeI);
	}
	printf("\n");
/*
	printf("accumlateGroupCycleID ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.accumulateGroupCycleID);
	}
	printf("\n");

	printf("accumulateGroupCycle ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.accumulateGroupCycle);
	}
	printf("\n");
*/
	printf("cmd_v ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld:%ld ", myData->cData[i].misc.cmd_v[0],
			myData->cData[i].misc.cmd_v[1]);
	}
	printf("\n");

	printf("cmd_i ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld:%ld, ", myData->cData[i].misc.cmd_i[0],
			myData->cData[i].misc.cmd_i[1]);
	}
	printf("\n");

	printf("cmd_p ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld:%ld, ", myData->cData[i].misc.cmd_p[0],
			myData->cData[i].misc.cmd_p[1]);
	}
	printf("\n");
	printf("\n");
	printf("test_val_l1 ");
	printf("0 : %ld, 1 : %ld, 2 : %ld, 3 : %ld, 4 : %ld, 5 : %ld, 6 : %ld, 7 : %ld, 8 : %ld\n",
		myData->test_val_l1[0], myData->test_val_l1[1], myData->test_val_l1[2],
		myData->test_val_l1[3], myData->test_val_l1[4], myData->test_val_l1[5],
		myData->test_val_l1[6], myData->test_val_l1[7], myData->test_val_l1[8]);
	printf("\n");

	for(i=0; i < 4; i++) {
		printf("CH %d  minflag : %d CVminI %2.5ld\t maxflag: %d CVmaxI %2.5ld \n", i+1,
			myData->cData[i].misc.cv_minI_flag, myData->cData[i].misc.cv_minI,
		   	myData->cData[i].misc.cv_maxI_flag, myData->cData[i].misc.cv_maxI);
	}
	printf("\n");

/*
	printf("test_val_i ");
	printf("1 : %d, 2 : %d, 3 : %d, 4 : %d, 5 : %d, 6 : %d, 7 : %d\n",
		myData->test_val_i[0][0], myData->test_val_i[0][1], myData->test_val_i[0][2],
		myData->test_val_i[0][3], myData->test_val_i[0][4], myData->test_val_i[0][5],
		myData->test_val_i[0][6]);

	printf("cmd_val ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld : %d, ", myData->testCond[0].pattern[1].cmd_val[3],
				myData->test_val_i[0][i]);
	}
	printf("\n");

	printf("t_val ");
	for(i=0; i < 5; i++) {
		printf("%ld, ", myData->testCond[0].pattern[i].t_val);
	}
	printf("\n");*/
/*
	printf("cmd_i_div ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.cmd_i_div);
	}
	printf("\n");

	printf("cmd_p ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld:%ld ", myData->cData[i].misc.cmd_p[0],
			myData->cData[i].misc.cmd_p[1]);
	}
	printf("\n");

	printf("bms_link_power ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.bms_link_flag);
	}
	printf("\n");

	printf("waveform_type ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.waveform_type);
	}
	printf("\n");

	printf("cv_select ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.cv_select);
	}
	printf("\n");

	printf("fbI ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].misc.fbI);
	}
	printf("\n");
*/
	//131011
/*	printf("external_data : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->testCond[i].external_data[1].control);
	}
	printf("\n");

	printf("signal c_sig_cmd_continue : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_CONTINUE]);
	}
	printf("\n");
*/
	printf("signal c_sig_cmd_pause : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_PAUSE]);
	}
	printf("\n");
/*
	printf("signal : %d %d\n",
			myData->cData[i].signal[C_SIG_EXT_CAN_NM_STAT_1],
			myData->cData[i].signal[C_SIG_EXT_CAN_NM_FLAG_1]);
*/
	printf("signal out_switch : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_OUT_SWITCH]);
	}
	printf("\n");
	printf("signal semi_switch ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_SEMI_SWITCH]);
	}
	printf("\n");

	printf("signal selected_range_i ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_SELECTED_RANGE_I]);
	}
	printf("\n");
/*
	printf("signal cali_normal : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CALI_NORMAL]);
	}
	printf("\n");

	printf("signal cali_point : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CALI_POINT]);
	}
	printf("\n");

	printf("signal cali_phase : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CALI_PHASE]);
	}
	printf("\n");

	printf("signal cali_update : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CALI_UPDATE]);
	}
	printf("\n");

	printf("signal cali_update_phase : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CALI_UPDATE_PHASE]);
	}
	printf("\n");

	printf("signal meter_reply : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_METER_REPLY]);
	}
	printf("\n");

	printf("signal cmd_run\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_RUN]);
	}
	printf("\n");

	printf("signal cmd_next_step\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_NEXT_STEP]);
	}
	printf("\n");

	printf("signal cmd_pause\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_PAUSE]);
	}
	printf("\n");

	printf("signal cmd_goto_step\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_GOTO_STEP]);
	}
	printf("\n");
*/
	printf("signal module_fault : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_MODULE_FAULT]);
	}
	printf("\n");

	printf("myCh->misc.total_Isens ");	//ktg_210511
	for(i=0; i < 4; i++) {
		printf("%ld ", myData->cData[i].misc.total_Isens);
	}
	printf("\n");
	printf("gcac_link_flag ");	//ktg_210511
	for(i=0; i < 4; i++) {
		printf("%d ", (int)myData->cData[i].misc.gcac_link_flag);
	}
	printf("\n");
/*	printf("pattern_count ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.pattern_count);
	}
	printf("\n");

	printf("pattern_t_val ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->testCond[i].pattern[myData->cData[i].misc.pattern_count].t_val);
	}
	printf("\n");

	printf("signal ext_op_type\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_EXT_OP_TYPE]);
	}
	printf("\n");

	printf("signal cmd_run\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CMD_RUN]);
	}
	printf("\n");

	printf("signal aux_dav ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_AUX_DAV]);
	}
	printf("\n");

	printf("signal aux_dai ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_AUX_DAI]);
	}
	printf("\n");

	printf("signal cable_check\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CABLE_CHECK]);
	}
	printf("\n");

	printf("resultIndex \n");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.resultIndex);
	}
	printf("\n");

	printf("tmp_val(Vsens) \n");
	for(i=0; i < 6; i++) {
		printf("%08ld ", myData->cData[0].misc.tmp_val[0][i]);
	}
	printf("\n");
	printf("tmp_val(Vpower) \n");
	for(i=0; i < 6; i++) {
		printf("%08ld ", myData->cData[0].misc.tmp_val[2][i]);
	}
	printf("\n");
	printf("tmp_val(Vbus) \n");
	for(i=0; i < 6; i++) {
		printf("%08ld ", myData->cData[0].misc.tmp_val[3][i]);
	}
	printf("\n");

	printf("canTrans.common.value \n");
	for(i=0; i < 10; i++) {
		printf("%f ", myData->canTransmitSetData.commonData[0][0].value[i]);
	}
	printf("\n");

	printf("C_SIG_EXT_CAN_E12_STAT_ERROR: %d, E12_NM_STAT: %d\n",
		myData->cData[0].signal[C_SIG_EXT_CAN_E12_STAT_ERROR],
		myData->cData[0].signal[C_SIG_EXT_CAN_E12_NM_STAT]);
	printf("C_SIG_EXT_RECEIVE_PARSING_COUNT: %d\n",
		myData->cData[0].signal[C_SIG_EXT_RECEIVE_PARSING_COUNT]);
	printf("CAN_SIG_SET_CONTROL: %d\n",
		myData->CAN.signal[0][CAN_SIG_SET_CONTROL]);

	printf("-----kimjaeho test-----\n");
	printf("0. attr_count: %ld\n", myData->test_val_l1[0]);
	printf("1. max_power1(1ch): %ld\n", myData->test_val_l1[1]);
	printf("2. max_power(attr_count): %ld\n", myData->test_val_l1[2]);
	printf("3. Vsens: %ld\n", myData->test_val_l1[3]);
	if(myData->test_val_l1[15] == 1) {
		printf("15. stepType: %s\n", "CHARGE");
	} else if(myData->test_val_l1[15] == 2) {
		printf("15. stepType: %s\n", "DISCHARGE");
	} else if(myData->test_val_l1[15] == 3) {
		printf("15. stepType: %s\n", "PATTERN");
	} else {
		printf("15. stepType: %s\n", "REST");
	}
	printf("4.5. MODE_CC_CV, CV, CC: d_val1: %ld, d_val2: %ld\n",
		myData->test_val_l1[4], myData->test_val_l1[5]);
	printf("6.7. MODE_CP: refP: %ld, refP1: %ld\n",
		myData->test_val_l1[6], myData->test_val_l1[7]);
	printf("14. RETURN: %ld\n", myData->test_val_l1[14]);
	printf("\n");

	//kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
	printf("C_SIG_CMD_CHAMBER_CONTINUE: %d\n",
		myData->cData[0].signal[C_SIG_CMD_CHAMBER_CONTINUE]);
*/

/*	for(i=0; i < MAX_P1_USERMAP_ROW_DATA; i++) {
		for(j=0; j < MAX_P1_USERMAP_COL_DATA; j++) {
			printf("%ld ", myData->testCond[0].usermap.table[i][j]);
		}
		printf("\n");
	}
	printf("\n");
*/
/*	printf("CAN_SIG_TX_PHASE_SEC: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->CAN.signal[i][CAN_SIG_TX_PHASE_SEC]);
	}
	printf("\n");

	printf("can_timer_1sec_count: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->CAN.can_timer_1sec_count[i]);
	}
	printf("\n");

	printf("can_timer_1sec: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->CAN.can_timer_1sec[i]);
	}
	printf("\n");

	printf("CAN_SIG_TX_PHASE_MIN: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->CAN.signal[i][CAN_SIG_TX_PHASE_MIN]);
	}
	printf("\n");

	printf("can_timer_1min_count: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->CAN.can_timer_1min_count[i]);
	}
	printf("\n");

	printf("can_timer_1min_count2: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->CAN.can_timer_1min_count2[i]);
	}
	printf("\n");

	printf("can_timer_1min: ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->CAN.can_timer_1min[i]);
	}
	printf("\n");

	printf("signal_out_mux_select ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_OUT_MUX_SELECT]);
	}
	printf("\n");
	printf("misc.out_mux          ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.out_mux);
	}
	printf("\n");
	printf("misc.out_mux_backup   ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].misc.out_mux_backup);
	}
	printf("\n");
	printf("temp_continue : %d\n",
		myData->cData[0].signal[C_SIG_TEMP_CONTINUE]);
*/
	printf("\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("ch:%d maxTemp:%ld, ChNo:%d, minTemp:%ld, ChNo:%d, diff:%ld\n",
			i, myData->cData[i].misc.maxAuxTemp,
			myData->cData[i].misc.maxAuxTemp_auxChNo,
			myData->cData[i].misc.minAuxTemp,
			myData->cData[i].misc.minAuxTemp_auxChNo,
			myData->cData[i].misc.diffAuxTemp);
		printf("ch:%d maxAuxV:%ld, ChNo:%d, minAuxV:%ld, ChNo:%d, diff:%ld\n",
			i, myData->cData[i].misc.maxAuxV,
			myData->cData[i].misc.maxAuxV_auxChNo,
			myData->cData[i].misc.minAuxV,
			myData->cData[i].misc.minAuxV_auxChNo,
			myData->cData[i].misc.diffAuxV);
		printf("ch:%d maxAuxTH:%ld, ChNo:%d, minAuxTH:%ld, ChNo:%d, diff:%ld\n",
			i, myData->cData[i].misc.maxAuxTH,
			myData->cData[i].misc.maxAuxTH_auxChNo,
			myData->cData[i].misc.minAuxTH,
			myData->cData[i].misc.minAuxTH_auxChNo,
			myData->cData[i].misc.diffAuxTH);
	}
	/*
	printf("\n");
	for(i=ch_start; i < ch_end; i++) {
		printf("ch:%d count_auxTemp1:%d, count_auxV1:%d, comp_count_auxTH1:%d\n",
			i, myData->cData[i].misc.comp_count_auxTemp1,
			myData->cData[i].misc.comp_count_auxV1,
			myData->cData[i].misc.comp_count_auxTH1);
	}
	printf("\n");

	printf("-------canTransmitChange--------\n"); //kjhw_170911
	for(i=ch_start; i < ch_end; i++) {
		for(j=0; j < MAX_CAN_TRANSMIT_CHANGE_DATA; j++) {
			printf("ch:%02d, function_div: %d, default_value: %f\n ", i+1,
			myData->canTransmitChange.changeData[i][j].function_div,
			myData->canTransmitChange.changeData[i][j].default_value);
		}
	}
	printf("fbCountI : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ",myData->cData[i].misc.fbCountI);
	}
#if defined __COA_VER_100B__ || defined __COA_VER_100C__ || defined __COA_VER_100D__ || defined __COA_VER_100F__ || defined __COA_VER_100F1__ || defined __COA_VER_1011__ || defined __COA_VER_1012__ || defined __COA_VER_1013__ || defined __COA_VER_1014__
#else	//COA_VER_1015~
	printf("signal fault_aux_delta_v_phase : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_FAULT_AUX_DELTA_V_PHASE]);
	}
	printf("\n");
#endif
*/
/*
		printf("\n");
	printf("test_val_l1 ");
	printf("0 : %ld, 1 : %ld, 2 : %ld, 3 : %ld, 4 : %ld, 5 : %ld, 6 : %ld, 7 : %ld, 8 : %ld\n",
		myData->test_val_l1[0], myData->test_val_l1[1], myData->test_val_l1[2],
		myData->test_val_l1[3], myData->test_val_l1[4], myData->test_val_l1[5],
		myData->test_val_l1[6], myData->test_val_l1[7], myData->test_val_l1[8]);
	printf("\n");
*/
	//ktg_210706s
/*	printf("signal CAN_SVDF_PHASE : ");
	for(i=ch_start; i < 4; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CAN_SVDF_DETECT_PHASE]);
	}
	printf("\n");


	printf("avgCell : ");
	for(i=ch_start; i < 1; i++) {
		printf("%ld ", myData->cData[i].misc.avgCell[0]);
		printf("%ld ", myData->cData[i].misc.avgCell[1]);
		printf("%ld ", myData->cData[i].misc.avgCell[2]);
	printf("\n");
	}
	printf("\n");
	
	for(i=0; i < 5; i++) {
		printf("prepreCell[%d] : %ld \t",i+1, myData->CANCell[0][i].prepreCell);
		printf("preCell[%d]    : %ld \t",i+1, myData->CANCell[0][i].preCell);
		printf("svdfCell[%d]   : %ld \t",i+1, myData->CANCell[0][i].svdfCell);
		printf("svdfcount[%d]  : %ld \n",i+1, myData->CANCell[0][i].can_svdfcount);
		printf("\n");
	}
	printf("\n");
*/	//ktg_210706e
//	printf("minCell  : %ld \n", myData->cData[0].misc.minCell);
//	printf("minCell_No  : %d \n", myData->cData[0].misc.minCell_dataNo);
//	printf("maxCell  : %ld \n", myData->cData[0].misc.maxCell);
//	printf("maxCell_No  : %d \n", myData->cData[0].misc.maxCell_dataNo);
	
	printf("min[0]AuxV  : %ld \n", myData->cData[0].misc.minGroupAuxV[0]);
	printf("diff[0]AuxV  : %ld \n", myData->cData[0].misc.diffGroupAuxV[0]);
	printf("test_val_l1 ");
	printf("0 : %ld, 1 : %ld, 2 : %ld, 3 : %ld, 4 : %ld, 5 : %ld, 6 : %ld, 7 : %ld, 8 : %ld\n",
		myData->test_val_l1[0], myData->test_val_l1[1], myData->test_val_l1[2],
		myData->test_val_l1[3], myData->test_val_l1[4], myData->test_val_l1[5],
		myData->test_val_l1[6], myData->test_val_l1[7], myData->test_val_l1[8]);
	printf("\n");

}

void FCH_State_Print(void)
{
#if defined __COC__
	int i, j, ch_start, ch_end;

	j = 0;
	printf("FCH_State\n");

	ch_start = myData->mData.config.chPerBd * bd;
	ch_end = ch_start + myData->mData.config.chPerBd;
/*
	for(i=ch_start; i < ch_end; i++) {
		printf("model_name %d ", i+1);
		for(j=0; j < 8; j++) {
			printf("%c", myData->fch.ch[i].model_name[j]);
		}
		printf("\n");
	}
*/
	for(i=ch_start; i < ch_end; i++) {
		printf("firmware_version %d ", i+1);
		for(j=0; j < 8; j++) {
			printf("%c", myData->fch.ch[i].firmware_version[j]);
		}
		printf("\n");
	}
/*
	printf("ah ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->fch.ch[i].ah);
	}
	printf("\n");

	printf("wh ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->fch.ch[i].wh);
	}
	printf("\n");
*/
	printf("i_val ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%x ", (int)myData->fch.ch[i].i_val[0]);
	}
	printf("\n");
/*
	printf("v_power_bus ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->fch.ch[i].v_power_bus);
	}
	printf("\n");

	printf("v_output_bus ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->fch.ch[i].v_output_bus);
	}
	printf("\n");

	printf("cable_check ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CABLE_CHECK]);
	}
	printf("\n");

	printf("cable_check_delay ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_CABLE_CHECK_DELAY]);
	}
	printf("\n");

	printf("Vsens ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->cData[i].op.Vsens);
	}
	printf("\n");
*/

	printf("\n");
#endif
}

void COB_Channel_Print(int group)
{ //debug_size_cob
#if defined __COB__
	char buf[24];
	int i, tmp, ch_start, ch_end, scan_step=0;

	//printf("COB_Channel bd(%d)\n", bd+1);
	//ch_start = myData->mData.config.chPerBd * bd;
	//ch_end = ch_start + myData->mData.config.chPerBd;

	printf("COB_Channel group(%d)\n", group+1);
	ch_start = 0;
	ch_end = myData->mData.config.chInGroup[group];

	printf("save_step : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->COB_save_step[group][i]);
	}
	printf("\n");

	printf("state : ");
	for(i=ch_start; i < ch_end; i++) {
		memset(buf, 0, sizeof buf);
		tmp = (int)myData->COB_opSave[group][i][scan_step].state;
		switch(tmp) {
			case C_IDLE:	strcpy(buf, "IDLE");		break;
			case C_STANDBY:	strcpy(buf, "STANDBY");		break;
			case C_RUN:		strcpy(buf, "RUN");			break;
			case C_PAUSE:	strcpy(buf, "PAUSE");		break;
			case C_CALI:	strcpy(buf, "CALI");		break;
			case C_FAULT:	strcpy(buf, "FAULT");		break;
			default:		strcpy(buf, "UNKNOWN");		break;
		}
		printf("%s ", buf);
	}
	printf("\n");

	printf("phase : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->COB_opSave[group][i][scan_step].phase);
	}
	printf("\n");

	printf("stepType : ");
	for(i=ch_start; i < ch_end; i++) {
		memset(buf, 0, sizeof buf);
		tmp = (int)myData->COB_opSave[group][i][scan_step].stepType;
		switch(tmp) {
			case STEP_IDLE:			strcpy(buf, "IDLE");		break;
			case STEP_CHARGE:		strcpy(buf, "CHARGE");		break;
			case STEP_DISCHARGE:	strcpy(buf, "DISCHARGE");	break;
			case STEP_REST:			strcpy(buf, "REST");		break;
			case STEP_LONG_TIME_REST:
				strcpy(buf, "LONG_TIME_REST");		break;
			case STEP_OCV:			strcpy(buf, "OCV");			break;
			case STEP_Z:			strcpy(buf, "Z");			break;
			case STEP_END:			strcpy(buf, "END");			break;
			case STEP_CYCLE:		strcpy(buf, "CYCLE");		break;
			case STEP_LOOP:			strcpy(buf, "LOOP");		break;
			case STEP_PATTERN:		strcpy(buf, "PATTERN");		break;
			default:				strcpy(buf, "UNKNOWN");		break;
		}
		printf("%s ", buf);
	}
	printf("\n");

	printf("code : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->COB_opSave[group][i][scan_step].code);
	}
	printf("\n");

	printf("code : ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%d ", myData->COB_opSave[group][i][scan_step].code);
	}
	printf("\n");

	printf("voltage ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->COB_opSave[group][i][scan_step].Vsens);
	}
	printf("\n");

	printf("current ");
	for(i=ch_start; i < ch_end; i++) {
		printf("%ld ", myData->COB_opSave[group][i][scan_step].Isens);
	}
	printf("\n");

	printf("\n");
#endif
}

void LineCheck_Print(int group)
{
	int i;
/*
	printf("lineCheck val\n");
	for(i=0; i < 24; i++) {
		printf("%02d(%ld:%ld) ", i+1,
			myData->lineCheck[group][i].avg_val[0],
			myData->lineCheck[group][i].avg_val[1]);
	}
	printf("\n");
	printf("lineCheck diff\n");
	for(i=0; i < 24; i++) {
		printf("%02d(%ld) ", i+1,
			myData->lineCheck[group][i].avg_val[0]
			- myData->lineCheck[group][i].avg_val[1]);
	}
	printf("\n");
*/
	printf("lineCheck2 val\n");
	for(i=0; i < 24; i++) {
		printf("%02d(%ld:%ld) ", i+1,
			myData->lineCheck[group][i].avg_val[2],
			myData->lineCheck[group][i].avg_val[3]);
	}
	printf("\n");
	printf("lineCheck2 diff\n");
	for(i=0; i < 24; i++) {
		printf("%02d(%ld) ", i+1,
			myData->lineCheck[group][i].avg_val[2]
			- myData->lineCheck[group][i].avg_val[3]);
	}
	printf("\n");
/*
	printf("lineCheck path_p:n:result ");
	for(i=0; i < 24; i++) {
		printf("%02d(%d:%d:%d) ", i+1,
			myData->lineCheck[group][i].path_val_p,
			myData->lineCheck[group][i].path_val_n,
			myData->lineCheck[group][i].result_code);
	}
	printf("\n");
*/
	printf("lineCheck4 val\n");
	for(i=0; i < 24; i++) {
		printf("%02d(%ld) ", i+1,
			myData->lineCheck[group][i].avg_val[4]);
	}
	printf("\n");

	printf("lineCheck4 diff\n");
	for(i=0; i < 24; i++) {
		printf("%02d(%ld) ", i+1,
			myData->lineCheck[group][i].avg_val[4]
			- myData->lineCheck[group][i].avg_val[2]);
	}
	printf("\n");
}

void IO_Print(void)
{
	int i, j;

	printf("IO\n");

	printf("ac_power_detect %ld, %ld, %ld\n",
		myData->dio.misc.ac_power_detect1,
		myData->dio.misc.ac_power_detect2,
		myData->dio.misc.ac_power_detect3);

	printf("delayTimer %ld\n", myData->dio.misc.delayTimer);

	printf("powerSwitchTimer %ld\n", myData->dio.misc.powerSwitchTimer);

	printf("signal remote_ps %d, in_external1 %d, in_external2 %d\n",
		myData->dio.signal[DIO_SIG_REMOTE_PS],
		myData->dio.signal[DIO_SIG_IN_EXTERNAL1],
		myData->dio.signal[DIO_SIG_IN_EXTERNAL2]);

	printf("signal chamber_over_temp_fail %d, over_temp_2 %d\n",
		myData->dio.signal[DIO_SIG_CHAMBER_OVER_TEMP_FAIL],
		myData->dio.signal[DIO_SIG_CHAMBER_OVER_TEMP_FAIL_2]);

	printf("dio_sig_remote_ps %d\n", myData->dio.misc.dio_sig_remote_ps);

	printf("in_address\n");
	for(i=0; i < MAX_DIO_IN_BYTES; i++) {
		printf("%x ", myData->dio.function_set.in_address[i]);
	}
	printf("\n");

	printf("in bytes\n");
	for(i=0; i < 4; i++) {
		for(j=0; j < 8; j++) {
			printf("%03x:", myData->dio.function_set.in_address[i*8+j]);
			printf("%02x ", myData->dio.in.bytes[i*8+j]);
		}
		printf("\n");
	}
	printf("\n");

	printf("out_address\n");
	for(i=0; i < MAX_DIO_OUT_BYTES; i++) {
		printf("%x ", myData->dio.function_set.out_address[i]);
	}
	printf("\n");
/*
	printf("out use1\n");
	for(i=0; i < 16; i++) {
		printf("%x : ", myData->dio.function_set.out_address[i]);
		for(j=0; j < 8; j++) {
			printf("%d ", myData->dio.function_set.out_set[i*8+j].use);
		}
		printf("\n");
	}
	printf("\n");

	printf("out use2\n");
	for(i=16; i < 32; i++) {
		printf("%x : ", myData->dio.function_set.out_address[i]);
		for(j=0; j < 8; j++) {
			printf("%d ", myData->dio.function_set.out_set[i*8+j].use);
		}
		printf("\n");
	}
	printf("\n");

	printf("out_set function1\n");
	for(i=0; i < 16; i++) {
		printf("%x : ", myData->dio.function_set.out_address[i]);
		for(j=0; j < 8; j++) {
			printf("%d ", myData->dio.function_set.out_set[i*8+j].function);
		}
		printf("\n");
	}
	printf("\n");

	printf("out_set function2\n");
	for(i=16; i < 32; i++) {
		printf("%x : ", myData->dio.function_set.out_address[i]);
		for(j=0; j < 8; j++) {
			printf("%d ", myData->dio.function_set.out_set[i*8+j].function);
		}
		printf("\n");
	}
	printf("\n");*/
/*
	printf("out function1\n");
	for(i=0; i < 16; i++) {
		printf("%x : ", myData->dio.function_set.out_address[i]);
		for(j=0; j < 8; j++) {
			printf("%d ", myData->dio.out.function[i*8+j]);
		}
		printf("\n");
	}
	printf("\n");

	printf("out function2\n");
	for(i=16; i < 32; i++) {
		printf("%x : ", myData->dio.function_set.out_address[i]);
		for(j=0; j < 8; j++) {
			printf("%d ", myData->dio.out.function[i*8+j]);
		}
		printf("\n");
	}
	printf("\n");
*/
/*	printf("out bytes\n");
	for(i=0; i < 4; i++) {
		for(j=0; j < 8; j++) {
			printf("%03x:", myData->dio.function_set.out_address[i*8+j]);
			printf("%02x ", myData->dio.out.bytes[i*8+j]);
		}
		printf("\n");
	}
	printf("\n");*/

	printf("signal in_start %d %d, in_stop %d %d\n",
		myData->dio.signal[DIO_SIG_IN_START1],
		myData->dio.signal[DIO_SIG_IN_START2],
		myData->dio.signal[DIO_SIG_IN_STOP1],
		myData->dio.signal[DIO_SIG_IN_STOP2]);

	printf("\n");
}

void TestCond_Print(void)
{
	int i;
	int step, monitor_ch, channel, totalStep, idx, idx2, idx3, idx4;
	long val1, val2, val3, val4, val5, val6;

	idx = idx2 = idx3 = idx4 = 0;
	i = 0;
	monitor_ch = 1;
	//monitor_ch = 2;
	//monitor_ch = 13;
	//channel = myData->CellArray1[monitor_ch-1].number2 - 1;
	channel = 0;
	printf("TestCond ch:%d\n", monitor_ch);

	idx = IDX_COM_OBJ_TOTAL_STEP;
	totalStep = (int)myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	val1 = myData->testCond[channel].common_object[idx];
	printf("totalStep %d, attr_count %ld\n", totalStep, val1);

	//for(step=0; step < 5; step++) {
	//for(step=30; step < (30+5); step++) {
	for(step=0; step < totalStep; step++) {
		idx = IDX_LOC_OBJ_STEP_NO;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_TYPE;
		val2 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_MODE;
		val3 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_ATTRIBUTE;
		val4 = myData->testCond[channel].local_object[step][idx];

		printf("step %ld(%d), type %ld, mode %ld, attr %ld\n",
			val1, step, val2, val3, val4);

		idx = IDX_LOC_OBJ_SAVE_DELTA_T;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_SAVE_DELTA_V;
		val2 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_SAVE_DELTA_I;
		val3 = myData->testCond[channel].local_object[step][idx];

		printf("saveDt %ld, saveDv %ld, saveDi %ld\n", val1, val2, val3);

		idx = IDX_LOC_OBJ_REF_V;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_REF_V2;
		val2 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_REF_I;
		val3 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_REF_P;
		val4 = myData->testCond[channel].local_object[step][idx];

		printf("refV %ld, refV2 %ld, refI %ld, refP %ld\n",
			val1, val2, val3, val4);

		idx = IDX_LOC_OBJ_REF_R;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_RANGE_V;
		val2 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_RANGE_I;
		val3 = myData->testCond[channel].local_object[step][idx];

		printf("refR %ld, rangeV %ld, rangeI %ld\n", val1, val2, val3);

		idx = IDX_LOC_OBJ_END_I;
		printf("endI %ld\n",
			myData->testCond[channel].local_object[step][idx]);
/*		idx = IDX_LOC_OBJ_END_DELTA_V;
		printf("endDeltaV %ld\n",
			myData->testCond[channel].local_object[step][idx]);
		idx = IDX_LOC_OBJ_FAULT_UPPER_I;
		printf("faultUpperI %ld\n",
			myData->testCond[channel].local_object[step][idx]);
		idx = IDX_LOC_OBJ_FAULT_LOWER_I;
		printf("faultLowerI %ld\n",
			myData->testCond[channel].local_object[step][idx]);
		idx = IDX_LOC_OBJ_SAVE_DELTA_T;
		printf("saveDt %ld\n",
			myData->testCond[channel].local_object[step][idx]);
		idx = IDX_LOC_OBJ_Z_T1;
		printf("z_t1 %ld\n",
			myData->testCond[channel].local_object[step][idx]);
		idx = IDX_LOC_OBJ_Z_T2;
		printf("z_t2 %ld\n",
			myData->testCond[channel].local_object[step][idx]);
*/
		idx = IDX_LOC_OBJ_END_TIME;
		idx2 = IDX_LOC_OBJ_END_TIME_BRANCH;
		printf("endT %ld, endT_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);
/*
		idx = IDX_LOC_OBJ_END_CV_TIME;
		idx2 = IDX_LOC_OBJ_END_CV_TIME_BRANCH;
		printf("endCVT %ld, endCVT_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_V_UPPER;
		idx2 = IDX_LOC_OBJ_END_V_UPPER_BRANCH;
		printf("endV_upper %ld, endV_upper_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_V_LOWER;
		idx2 = IDX_LOC_OBJ_END_V_LOWER_BRANCH;
		printf("endV_lower %ld, endV_lower_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
		idx2 = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
		printf("endAmpareHour %ld, endAmpareHour_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_WATT_HOUR;
		idx2 = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
		printf("endWattHour %ld, endWattHour_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		idx2 = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
		idx3 = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		printf("endAmpareHourRate %ld, branch %ld, compare %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3]);

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
		idx2 = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
		idx3 = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
		printf("endWattHourRate %ld, branch %ld, compare %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3]);
*/
/*		idx = IDX_LOC_OBJ_END_CYCLE_COUNT;
		idx2 = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
		printf("cycleCount %ld, branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT;
		idx2 = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_BRANCH;
		idx3 = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_COMPARE;
		printf("multiCycleCount %ld, branch %ld, compare %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3]);

		idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT;
		idx2 = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT_BRANCH;
		idx3 = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT_COMPARE;
		printf("accCycleCount %ld, branch %ld, compare %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3]);*/
/*
		idx = IDX_LOC_OBJ_END_SUM_TIME;
		idx2 = IDX_LOC_OBJ_END_SUM_TIME_BRANCH;
		printf("endSumT %ld, endSumT_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR;
		idx2 = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR_BRANCH;
		printf("endSumAmpareHour %ld, endSumAmpareHour_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_SUM_WATT_HOUR;
		idx2 = IDX_LOC_OBJ_END_SUM_WATT_HOUR_BRANCH;
		printf("endSumWattHour %ld, endSumWattHour_branch %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_AUX_V_UPPER;
		idx2 = IDX_LOC_OBJ_END_AUX_V_UPPER_BRANCH;
		printf("aux_v_upper %ld, %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);

		idx = IDX_LOC_OBJ_END_AUX_V_LOWER;
		idx2 = IDX_LOC_OBJ_END_AUX_V_LOWER_BRANCH;
		printf("aux_v_lower %ld, %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2]);
*/
		idx = IDX_LOC_OBJ_FAULT_HW_UPPER_V;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx2 = C_CD_FAULT_HW_UPPER_V - C_CD_END_START;
		val2 = myData->testCond[channel].local_chCode[step][idx2].compCount;

		val3 = myData->testCond[channel].local_chCode[step][idx2].check_delay_count;

		idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
		val4 = myData->testCond[channel].local_object[step][idx];

		idx2 = C_CD_FAULT_HW_LOWER_V - C_CD_END_START;
		val5 = myData->testCond[channel].local_chCode[step][idx2].compCount;

		val6 = myData->testCond[channel].local_chCode[step][idx2].check_delay_count;

		printf("hw_faultUpperV %ld %ld %ld, LowerV %ld %ld %ld\n",
			val1, val2, val3, val4, val5, val6);

		idx = IDX_LOC_OBJ_FAULT_HW_UPPER_I;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx2 = C_CD_FAULT_HW_UPPER_I - C_CD_END_START;
		val2 = myData->testCond[channel].local_chCode[step][idx2].compCount;

		val3 = myData->testCond[channel].local_chCode[step][idx2].check_delay_count;

		idx = IDX_LOC_OBJ_FAULT_HW_LOWER_I;
		val4 = myData->testCond[channel].local_object[step][idx];

		idx2 = C_CD_FAULT_HW_LOWER_I - C_CD_END_START;
		val5 = myData->testCond[channel].local_chCode[step][idx2].compCount;

		val6 = myData->testCond[channel].local_chCode[step][idx2].check_delay_count;

		printf("hw_faultUpperI %ld %ld %ld, LowerI %ld %ld %ld\n",
			val1, val2, val3, val4, val5, val6);

		idx = IDX_LOC_OBJ_FAULT_UPPER_V;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_FAULT_LOWER_V;
		val2 = myData->testCond[channel].local_object[step][idx];
		printf("faultUpperV %ld, LowerV %ld\n", val1, val2);

		idx = IDX_LOC_OBJ_FAULT_UPPER_I;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_FAULT_LOWER_I;
		val2 = myData->testCond[channel].local_object[step][idx];
		printf("faultUpperI %ld, LowerI %ld\n", val1, val2);

		//csk_190808s
		idx = IDX_LOC_OBJ_FAULT_COMP_AUX_V;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_FAULT_COMP_AUX_V_VENT_FLAG;
		val2 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_FAULT_COMP_AUX_TEMP;
		val3 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_FAULT_COMP_AUX_TEMP_VENT_FLAG;
		val4 = myData->testCond[channel].local_object[step][idx];

		printf("fault COMP_AUX_V %ld VENT %ld, COMP_AUX_TEMP %ld VENT %ld\n",
			val1, val2, val3, val4);

		idx = IDX_LOC_OBJ_FAULT_COMP_AUX_TH;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_FAULT_COMP_AUX_TH_VENT_FLAG;
		val2 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_FAULT_COMP_AUX_T;
		val3 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_FAULT_COMP_AUX_T_VENT_FLAG;
		val4 = myData->testCond[channel].local_object[step][idx];

		printf("fault COMP_AUX_TH %ld VENT %ld, COMP_AUX_T %ld VENT %ld\n",
			val1, val2, val3, val4);

		idx = IDX_LOC_OBJ_FAULT_DELTA_AUX_V;
		val1 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_FAULT_DELTA_AUX_V_T;
		val2 = myData->testCond[channel].local_object[step][idx];

		idx = IDX_LOC_OBJ_FAULT_DELTA_AUX_V_VENT_FLAG;
		val3 = myData->testCond[channel].local_object[step][idx];

		printf("fault DELTA_AUX_V %ld V_T %ld, VENT %ld\n",
			val1, val2, val3);
		//csk_190818e
/*
		idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_I;
		printf("faultCheck_UpperI %ld\n",
			myData->testCond[channel].local_object[step][idx]);
		idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_I;
		printf("faultCheck_LowerI %ld\n",
			myData->testCond[channel].local_object[step][idx]);

		//for sbl ng cond
		idx = IDX_LOC_OBJ_FAULT_VOLTAGE_DATA_MIN;
		idx2 = IDX_LOC_OBJ_FAULT_VOLTAGE_DATA_MAX;
		idx3 = IDX_LOC_OBJ_FAULT_VOLTAGE_AVG_MIN;
		idx4 = IDX_LOC_OBJ_FAULT_VOLTAGE_AVG_MAX;
		printf("fault_ng_Voltage %ld, %ld, %ld, %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3],
			myData->testCond[channel].local_object[step][idx4]);
		idx = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_DATA_MIN;
		idx2 = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_DATA_MAX;
		idx3 = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_AVG_MIN;
		idx4 = IDX_LOC_OBJ_FAULT_AMPARE_HOUR_AVG_MAX;
		printf("fault_ng_Ah %ld, %ld, %ld, %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3],
			myData->testCond[channel].local_object[step][idx4]);
		idx = IDX_LOC_OBJ_FAULT_POWER_DATA_MIN;
		idx2 = IDX_LOC_OBJ_FAULT_POWER_DATA_MAX;
		idx3 = IDX_LOC_OBJ_FAULT_POWER_AVG_MIN;
		idx4 = IDX_LOC_OBJ_FAULT_POWER_AVG_MAX;
		printf("fault_ng_Power %ld, %ld, %ld, %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx2],
			myData->testCond[channel].local_object[step][idx3],
			myData->testCond[channel].local_object[step][idx4]);
*/
/*		idx = IDX_LOC_OBJ_CAN_VALUE_1;
		printf("can_value %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx+1],
			myData->testCond[channel].local_object[step][idx+2],
			myData->testCond[channel].local_object[step][idx+3],
			myData->testCond[channel].local_object[step][idx+4],
			myData->testCond[channel].local_object[step][idx+5],
			myData->testCond[channel].local_object[step][idx+6],
			myData->testCond[channel].local_object[step][idx+7],
			myData->testCond[channel].local_object[step][idx+8],
			myData->testCond[channel].local_object[step][idx+9]
			);
*/
			
		idx = IDX_LOC_OBJ_CAN_FUNC_DIV_1;
		printf("can_func %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx+1],
			myData->testCond[channel].local_object[step][idx+2],
			myData->testCond[channel].local_object[step][idx+3],
			myData->testCond[channel].local_object[step][idx+4],
			myData->testCond[channel].local_object[step][idx+5],
			myData->testCond[channel].local_object[step][idx+6],
			myData->testCond[channel].local_object[step][idx+7],
			myData->testCond[channel].local_object[step][idx+8],
			myData->testCond[channel].local_object[step][idx+9]
			);
		printf("\n");

		/*
		idx = IDX_LOC_OBJ_CAN_BRANCH_1;
		printf("can_branch %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx+1],
			myData->testCond[channel].local_object[step][idx+2],
			myData->testCond[channel].local_object[step][idx+3],
			myData->testCond[channel].local_object[step][idx+4],
			myData->testCond[channel].local_object[step][idx+5],
			myData->testCond[channel].local_object[step][idx+6],
			myData->testCond[channel].local_object[step][idx+7],
			myData->testCond[channel].local_object[step][idx+8],
			myData->testCond[channel].local_object[step][idx+9]
			);
		printf("\n");

		idx = IDX_LOC_OBJ_AUX_VALUE_1;
		printf("aux_value %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx+1],
			myData->testCond[channel].local_object[step][idx+2],
			myData->testCond[channel].local_object[step][idx+3],
			myData->testCond[channel].local_object[step][idx+4],
			myData->testCond[channel].local_object[step][idx+5],
			myData->testCond[channel].local_object[step][idx+6],
			myData->testCond[channel].local_object[step][idx+7],
			myData->testCond[channel].local_object[step][idx+8],
			myData->testCond[channel].local_object[step][idx+9]
			);

		idx = IDX_LOC_OBJ_AUX_FUNC_DIV_1;
		printf("aux_func %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx+1],
			myData->testCond[channel].local_object[step][idx+2],
			myData->testCond[channel].local_object[step][idx+3],
			myData->testCond[channel].local_object[step][idx+4],
			myData->testCond[channel].local_object[step][idx+5],
			myData->testCond[channel].local_object[step][idx+6],
			myData->testCond[channel].local_object[step][idx+7],
			myData->testCond[channel].local_object[step][idx+8],
			myData->testCond[channel].local_object[step][idx+9]
			);
		printf("\n");

		idx = IDX_LOC_OBJ_AUX_BRANCH_1;
		printf("aux_branch %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
			myData->testCond[channel].local_object[step][idx],
			myData->testCond[channel].local_object[step][idx+1],
			myData->testCond[channel].local_object[step][idx+2],
			myData->testCond[channel].local_object[step][idx+3],
			myData->testCond[channel].local_object[step][idx+4],
			myData->testCond[channel].local_object[step][idx+5],
			myData->testCond[channel].local_object[step][idx+6],
			myData->testCond[channel].local_object[step][idx+7],
			myData->testCond[channel].local_object[step][idx+8],
			myData->testCond[channel].local_object[step][idx+9]
			);
*/
		printf("\n");
	}

	//common_condition
	idx = IDX_COM_OBJ_FAULT_HW_UPPER_V;
	val1 = myData->testCond[channel].common_object[idx];

	idx2 = C_CD_COM_FAULT_HW_UPPER_V - C_CD_COM_START;
	val2 = myData->testCond[channel].common_chCode[idx2].compCount;

	val3 = myData->testCond[channel].common_chCode[idx2].check_delay_count;

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_V;
	val4 = myData->testCond[channel].common_object[idx];

	idx2 = C_CD_COM_FAULT_HW_LOWER_V - C_CD_COM_START;
	val5 = myData->testCond[channel].common_chCode[idx2].compCount;

	val6 = myData->testCond[channel].common_chCode[idx2].check_delay_count;
	printf("hw_upper_v %ld %ld %ld, hw_lower_v %ld %ld %ld\n",
		val1, val2, val3, val4, val5, val6);

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_I;
	val1 = myData->testCond[channel].common_object[idx];

	idx2 = C_CD_COM_FAULT_HW_UPPER_I - C_CD_COM_START;
	val2 = myData->testCond[channel].common_chCode[idx2].compCount;

	val3 = myData->testCond[channel].common_chCode[idx2].check_delay_count;

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_I;
	val4 = myData->testCond[channel].common_object[idx];

	idx2 = C_CD_COM_FAULT_HW_LOWER_I - C_CD_COM_START;
	val5 = myData->testCond[channel].common_chCode[idx2].compCount;

	val6 = myData->testCond[channel].common_chCode[idx2].check_delay_count;
	printf("hw_upper_i %ld %ld %ld, hw_lower_i %ld %ld %ld\n",
		val1, val2, val3, val4, val5, val6);

	idx = IDX_COM_OBJ_FAULT_UPPER_V;
	val1 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_LOWER_V;
	val2 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_I;
	val3 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_AMPARE_HOUR;
	val4 = myData->testCond[channel].common_object[idx];

	printf("safetyUpperV %ld, LowerV %ld, UpperI %ld, UpperAh %ld\n",
		val1, val2, val3, val4);

	idx = IDX_COM_OBJ_FAULT_UPPER_WATT_HOUR;
	val1 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_P;
	val2 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_TEMP;
	val3 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_LOWER_TEMP;
	val4 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_UPPER_AUX_V;
	val5 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_LOWER_AUX_V;
	val6 = myData->testCond[channel].common_object[idx];

	printf("safetyUpperWh %ld, UpperP %ld, UpperTemp %ld, LowerTemp %ld
			faultUpper_AuxV %ld, faultLower_AuxV %ld\n",
		val1, val2, val3, val4, val5, val6);

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V;
	val1 = myData->testCond[channel].common_object[idx];
	printf("comp_aux_V %ld", val1);

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_REF_V1;
	val1 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V2;
	val2 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_REF_V2;
	val3 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V3;
	val4 = myData->testCond[channel].common_object[idx];

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_VENT_FLAG;
	val5 = myData->testCond[channel].common_object[idx];
	printf(" REF(%ld), comp_aux_V2 %ld REF(%ld), comp_aux_V3 %ld : vent_flag(%ld)", val1, val2, val3, val4, val5);
	
	printf("\n");
	for(i=0; i < 1; i++) {
		printf("ch:%d maxTemp:%ld, ChNo:%d, minTemp:%ld, ChNo:%d, diff:%ld\n",
			i, myData->cData[i].misc.maxAuxTemp,
			myData->cData[i].misc.maxAuxTemp_auxChNo,
			myData->cData[i].misc.minAuxTemp,
			myData->cData[i].misc.minAuxTemp_auxChNo,
			myData->cData[i].misc.diffAuxTemp);
		printf("ch:%d maxAuxV:%ld, ChNo:%d, minAuxV:%ld, ChNo:%d, diff:%ld\n",
			i, myData->cData[i].misc.maxAuxV,
			myData->cData[i].misc.maxAuxV_auxChNo,
			myData->cData[i].misc.minAuxV,
			myData->cData[i].misc.minAuxV_auxChNo,
			myData->cData[i].misc.diffAuxV);
		printf("ch:%d maxAuxTH:%ld, ChNo:%d, minAuxTH:%ld, ChNo:%d, diff:%ld\n",
			i, myData->cData[i].misc.maxAuxTH,
			myData->cData[i].misc.maxAuxTH_auxChNo,
			myData->cData[i].misc.minAuxTH,
			myData->cData[i].misc.minAuxTH_auxChNo,
			myData->cData[i].misc.diffAuxTH);
	}
/*	idx = IDX_COM_OBJ_CAN_FUNC_DIV_1;
	printf("safety_can_func %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
		myData->testCond[channel].common_object[idx],
		myData->testCond[channel].common_object[idx+1],
		myData->testCond[channel].common_object[idx+2],
		myData->testCond[channel].common_object[idx+3],
		myData->testCond[channel].common_object[idx+4],
		myData->testCond[channel].common_object[idx+5],
		myData->testCond[channel].common_object[idx+6],
		myData->testCond[channel].common_object[idx+7],
		myData->testCond[channel].common_object[idx+8],
		myData->testCond[channel].common_object[idx+9]
		);

	idx = IDX_COM_OBJ_CAN_VALUE_1;
	printf("safety_can_val %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
		myData->testCond[channel].common_object[idx],
		myData->testCond[channel].common_object[idx+1],
		myData->testCond[channel].common_object[idx+2],
		myData->testCond[channel].common_object[idx+3],
		myData->testCond[channel].common_object[idx+4],
		myData->testCond[channel].common_object[idx+5],
		myData->testCond[channel].common_object[idx+6],
		myData->testCond[channel].common_object[idx+7],
		myData->testCond[channel].common_object[idx+8],
		myData->testCond[channel].common_object[idx+9]
		);

	idx = C_CD_COM_CAN_FAULT_FUNC_DIV_1 - C_CD_CAN_START;
	printf("safety_can_comp_type %d %d %d %d %d, %d %d %d %d %d\n",
		myData->testCond[channel].can_chCode[idx].compType,
		myData->testCond[channel].can_chCode[idx+1].compType,
		myData->testCond[channel].can_chCode[idx+2].compType,
		myData->testCond[channel].can_chCode[idx+3].compType,
		myData->testCond[channel].can_chCode[idx+4].compType,
		myData->testCond[channel].can_chCode[idx+5].compType,
		myData->testCond[channel].can_chCode[idx+6].compType,
		myData->testCond[channel].can_chCode[idx+7].compType,
		myData->testCond[channel].can_chCode[idx+8].compType,
		myData->testCond[channel].can_chCode[idx+9].compType
		);

	idx = IDX_COM_OBJ_AUX_FUNC_DIV_1;
	printf("safety_aux_func %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
		myData->testCond[channel].common_object[idx],
		myData->testCond[channel].common_object[idx+1],
		myData->testCond[channel].common_object[idx+2],
		myData->testCond[channel].common_object[idx+3],
		myData->testCond[channel].common_object[idx+4],
		myData->testCond[channel].common_object[idx+5],
		myData->testCond[channel].common_object[idx+6],
		myData->testCond[channel].common_object[idx+7],
		myData->testCond[channel].common_object[idx+8],
		myData->testCond[channel].common_object[idx+9]
		);

	idx = IDX_COM_OBJ_AUX_VALUE_1;
	printf("safety_aux_val %ld %ld %ld %ld %ld, %ld %ld %ld %ld %ld\n",
		myData->testCond[channel].common_object[idx],
		myData->testCond[channel].common_object[idx+1],
		myData->testCond[channel].common_object[idx+2],
		myData->testCond[channel].common_object[idx+3],
		myData->testCond[channel].common_object[idx+4],
		myData->testCond[channel].common_object[idx+5],
		myData->testCond[channel].common_object[idx+6],
		myData->testCond[channel].common_object[idx+7],
		myData->testCond[channel].common_object[idx+8],
		myData->testCond[channel].common_object[idx+9]
		);

	idx = C_CD_COM_AUX_FAULT_FUNC_DIV_1 - C_CD_AUX_START;
	printf("safety_aux_comp_type %d %d %d %d %d, %d %d %d %d %d\n",
		myData->testCond[channel].aux_chCode[idx].compType,
		myData->testCond[channel].aux_chCode[idx+1].compType,
		myData->testCond[channel].aux_chCode[idx+2].compType,
		myData->testCond[channel].aux_chCode[idx+3].compType,
		myData->testCond[channel].aux_chCode[idx+4].compType,
		myData->testCond[channel].aux_chCode[idx+5].compType,
		myData->testCond[channel].aux_chCode[idx+6].compType,
		myData->testCond[channel].aux_chCode[idx+7].compType,
		myData->testCond[channel].aux_chCode[idx+8].compType,
		myData->testCond[channel].aux_chCode[idx+9].compType
		);*/

	idx = C_CD_AUX_FAULT_VENT_UPPER - C_CD_AUX_START;
	printf("upper %ld %d %ld\n",
		myData->testCond[channel].aux_chCode[idx].compCount,
		myData->testCond[channel].aux_chCode[idx+1].compType,
		myData->auxSetData[0].vent_upper);
	/*
	//for sbl safety
	idx = IDX_COM_OBJ_FAULT_CHARGE_UPPER_V;
	printf("safetyChargeUpperV %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_CHARGE_UPPER_AMPARE_HOUR;
	printf("safetyChargeUpperAh %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_CHARGE_END_UPPER_V;
	printf("safetyChargeEndUpperV %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_CHARGE_LOWER_I;
	printf("safetyChargeLowerI %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_DISCHARGE_LOWER_V;
	printf("safetyDischargeLowerV %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_DISCHARGE_RUN_TIME;
	printf("safetyDischargeRunTime %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_DISCHARGE_LOWER_AMPARE_HOUR;
	printf("safetyDischargeLowerAh %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_OVER_CURRENT_LIMIT;
	printf("safetyOverCurrent %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_COMP_V_T1;
	printf("safetyComp_CheckV_T %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_V1;
	printf("safetyComp_CheckLowerV %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_COMP_V_T2;
	printf("safetyComp_V_T1 %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_V2;
	printf("safetyComp_LowerV1 %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_UPPER_V2;
	printf("safetyComp_UpperV1 %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_COMP_V_T3;
	printf("safetyComp_V_T2 %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_V3;
	printf("safetyComp_LowerV2 %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_UPPER_V3;
	printf("safetyComp_UpperV2 %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_CV_TIME;
	printf("safetyComp_Lower_CV_T %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_UPPER_CV_TIME;
	printf("safetyComp_Upper_CV_T %ld\n",
		myData->testCond[channel].common_object[idx]);

	idx = IDX_COM_OBJ_FAULT_COMP_I_T1;
	printf("safetyComp_I_T1 %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_LOWER_I1;
	printf("safetyComp_LowerI1 %ld\n",
		myData->testCond[channel].common_object[idx]);
	idx = IDX_COM_OBJ_FAULT_COMP_UPPER_I1;
	printf("safetyComp_UpperI1 %ld\n",
		myData->testCond[channel].common_object[idx]);
	printf("\n");

	//for sbl check
	idx = IDX_LOC_OBJ_FAULT_CHECK_DETECT_V;
	printf("checkDetectV %ld\n",
		myData->testCond[channel].local_object[1][idx]);
	idx = IDX_LOC_OBJ_FAULT_CHECK_REVERSE_V;
	printf("checkReverseV %ld\n",
		myData->testCond[channel].local_object[1][idx]);
	idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_OCV;
	printf("checkLowerOCV %ld\n",
		myData->testCond[channel].local_object[1][idx]);
	idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_OCV;
	printf("checkUpperOCV %ld\n",
		myData->testCond[channel].local_object[1][idx]);

	idx = IDX_LOC_OBJ_END_TIME;
	printf("checkEndT %ld\n",
		myData->testCond[channel].local_object[2][idx]);
	idx = IDX_LOC_OBJ_REF_V;
	printf("checkRefV %ld\n",
		myData->testCond[channel].local_object[2][idx]);
	idx = IDX_LOC_OBJ_REF_I;
	printf("checkRefI %ld\n",
		myData->testCond[channel].local_object[2][idx]);
	idx = IDX_LOC_OBJ_FAULT_CHECK_I_JUDGE_RATIO;
	printf("checkI_judgeRatio %ld\n",
		myData->testCond[channel].local_object[2][idx]);
	idx = IDX_LOC_OBJ_FAULT_CHECK_UPPER_DELTA_V;
	printf("checkUpperDV %ld\n",
		myData->testCond[channel].local_object[2][idx]);
	idx = IDX_LOC_OBJ_FAULT_CHECK_LOWER_DELTA_V;
	printf("checkLowerDV %ld\n",
		myData->testCond[channel].local_object[2][idx]);
*/
/*	printf("reserved_cmd:%d, reserved_stepNo:%ld, reserved_cycleNo:%ld\n",
		(int)myData->testCond[channel].reserved.reserved_cmd,
		myData->testCond[channel].reserved.reserved_stepNo,
		myData->testCond[channel].reserved.reserved_cycleNo);

	printf("select_run:%d, select_stepNo:%ld, select_cycleNo:%ld, select_advCycleStep:%ld\n",
		(int)myData->testCond[channel].reserved.select_run,
		myData->testCond[channel].reserved.select_stepNo,
		myData->testCond[channel].reserved.select_cycleNo,
		myData->testCond[channel].reserved.select_advCycleStep);

	printf("pattern %d", channel);
	for(i=0; i < 25; i++) {
		printf("(%d)%ld:%ld:%ld:%ld:%d ", i,
			myData->testCond[channel].pattern[i].t_val,
			myData->testCond[channel].pattern[i].cmd_val[0],
			myData->testCond[channel].pattern[i].cmd_val[1],
			myData->testCond[channel].pattern[i].cmd_val[2], //kjhw_150914
			myData->testCond[channel].pattern[i].waveform_type);
	}
	printf("\n");*/
/*
	printf("can compCount(type) ");
	for(i=0; i < 5; i++) {
		printf("%d(%d) ",
			myData->testCond[channel].can_chCode[i].compCount,
			myData->testCond[channel].can_chCode[i].compType);
	}
	printf("\n");
*/
/*	printf("external_can %d", channel);
	printf("%ld:%ld:%ld:%ld:%ld:%ld:%ld:%d:%d:%d:%d\n",
		myData->testCond[channel].external_data[1].cmd_v[0],
		myData->testCond[channel].external_data[1].cmd_v[1],
		myData->testCond[channel].external_data[1].cmd_i[0],
		myData->testCond[channel].external_data[1].cmd_i[1],
		myData->testCond[channel].external_data[1].cmd_p[0],
		myData->testCond[channel].external_data[1].cmd_p[1],
		myData->testCond[channel].external_data[1].t_val,
		myData->testCond[channel].external_data[1].version,
		myData->testCond[channel].external_data[1].mode,
		myData->testCond[channel].external_data[1].waveform_type,
		myData->testCond[channel].external_data[1].control);
*/
	printf("\n");
}

void CaliData_Print(void)
{
	int i, channel, type, range;
	int j,k,l;

	printf("CaliData------------\n");

	channel = 0;
	type = 0;
	range = 0;

	/*
	for(j=0; j < 5; j++) {
		printf("ch %d------\n", j);
		for(k=0; k < 1; k++) {
			for(i=0; i < MAX_CALI_POINT; i++) {
//		printf("%d DA_A:%f, DA_B:%f\n", i,
//		myData->cali[channel].tmpData[type][range].DA_A[i],
//		myData->cali[channel].tmpData[type][range].DA_B[i]);
				printf("%d DA_A:%f, DA_B:%f\n", i,
					myData->cali[j].data[k][range].DA_A[i],
					myData->cali[j].data[k][range].DA_B[i]);
			}

			for(i=0; i < MAX_CALI_POINT; i++) {
//		printf("%d AD_A:%f, AD_B:%f\n", i,
//			myData->cali[channel].tmpData[type][range].AD_A[i],
//			myData->cali[channel].tmpData[type][range].AD_B[i]);
				printf("%d AD_A:%f, AD_B:%f\n", i,
					myData->cali[j].data[k][range].AD_A[i],
					myData->cali[j].data[k][range].AD_B[i]);
			}
		}
	}*/
	for(i=0; i < myData->mData.config.installedCh; i++) {
		for(j=0; j < MAX_TYPE; j++) {
			for(k=0; k < MAX_RANGE; k++) {
				printf("ch %d, type %d, range %d, count %d\n",
					i, j, k, myData->cali[i].count);
				printf("CALI caliPoint ");
				for(l=0; l < myData->cali[channel].tmpCond[type][range]
					.point.setPointNum; l++) {
					printf("%ld ", myData->cali[i].tmpCond[j][k]
						.point.setPoint[l]);
				}
				printf("\n");

				printf("caliCheckPoint ");
				for(l=0; l < myData->cali[i].tmpCond[j][k]
					.point.checkPointNum; l++) {
					printf("%ld ", myData->cali[i].tmpCond[j][k]
						.point.checkPoint[l]);
				}
				printf("\n");
			}
		}
	}
/*
	printf("CALI caliPointNum %d, caliCheckPointNum %d\n",
		myData->cali[channel].tmpCond[type][range].point.setPointNum,
		myData->cali[channel].tmpCond[type][range].point.checkPointNum);

	printf("CALI caliPoint ");
	for(i=0; i < myData->cali[channel].tmpCond[type][range]
		.point.setPointNum; i++) {
		printf("%ld ", myData->cali[channel].tmpCond[type][range]
			.point.setPoint[i]);
	}
	printf("\n");

	printf("caliCheckPoint ");
	for(i=0; i < myData->cali[channel].tmpCond[type][range]
		.point.checkPointNum; i++) {
		printf("%ld ", myData->cali[channel].tmpCond[type][range]
			.point.checkPoint[i]);
	}
	printf("\n");
	*/
/*
	printf("set ad    ");
	for(i=0; i < MAX_CALI_POINT; i++) {
		printf("%f, ", (float)myData->cali[channel].tmpData[type][range]
			.set_ad[i]);
	}
	printf("\n");

	printf("set meter ");
	for(i=0; i < MAX_CALI_POINT; i++) {
		printf("%f, ", (float)myData->cali[channel].tmpData[type][range]
			.set_meter[i]);
	}
	printf("\n");

	printf("check ad    ");
	for(i=0; i < MAX_CALI_POINT; i++) {
		printf("%f, ", (float)myData->cali[channel].tmpData[type][range]
			.check_ad[i]);
	}
	printf("\n");

	printf("check meter ");
	for(i=0; i < MAX_CALI_POINT; i++) {
		printf("%f, ", (float)myData->cali[channel].tmpData[type][range]
			.check_meter[i]);
	}
	printf("\n");

	printf("check AD_ratio %f, %f\n",
		(float)myData->cali[channel].tmpData[type][range].AD_Ratio[0],
		(float)myData->cali[channel].tmpData[type][range].AD_Ratio[1]);
	printf("\n");

	printf("check DA_A[1] %f, DA_B[1] %f\n",
		(float)myData->cali[channel].tmpData[type][range].DA_A[1],
		(float)myData->cali[channel].tmpData[type][range].DA_B[1]);
	printf("check DA_A[2] %f, DA_B[2] %f\n",
		(float)myData->cali[channel].tmpData[type][range].DA_A[2],
		(float)myData->cali[channel].tmpData[type][range].DA_B[2]);
	printf("\n");

	for(i=0; i < myData->cali[channel].tmpCond[type][range]
		.point.setPointNum-1; i++) {
		printf("check DA_A[%d] %f, DA_B[%d] %f\n", i,
			(float)myData->cali[channel].tmpData[type][range].DA_A[0], i,
			(float)myData->cali[channel].tmpData[type][range].DA_B[0]);
	}
	printf("\n");
	*/
}

void SubSensV_Print(void)
{
	int i, k, step;
	long val1;//, val2, val3, val4;	//ktg_210105
/*
	printf("SubSensV %d\n",
		myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION]);
	i = j = k = 0;

	printf("org_data\n");
	for(i=0; i < 6; i++) {
		printf("%02x ", myData->SubSensV.org_data.zero_val[i]);
	}
	printf("\n");

	for(i=0; i < 3; i++) {
		for(j=0; j < 4; j++) {
			if(myData->mData.config.daq_type == DAQ_TYPE1) {
				printf("%02x %02x %02x %02x ",
					myData->SubSensV.org_data.ref_val[i][j][0].byte[1],
					myData->SubSensV.org_data.ref_val[i][j][0].byte[0],
					myData->SubSensV.org_data.ref_val[i][j][1].byte[1],
					myData->SubSensV.org_data.ref_val[i][j][1].byte[0]);
			} else {
				printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
					myData->SubSensV.org_data.ref_val[i][j][0].byte[1],
					myData->SubSensV.org_data.ref_val[i][j][0].byte[0],
					myData->SubSensV.org_data.ref_val[i][j][1].byte[1],
					myData->SubSensV.org_data.ref_val[i][j][1].byte[0],
					myData->SubSensV.org_data.ref_val[i][j][2].byte[1],
					myData->SubSensV.org_data.ref_val[i][j][2].byte[0],
					myData->SubSensV.org_data.ref_val[i][j][3].byte[1],
					myData->SubSensV.org_data.ref_val[i][j][3].byte[0]);

				printf("%d %d %d %d\n",
					myData->SubSensV.org_data.ref_val[i][j][0].val,
					myData->SubSensV.org_data.ref_val[i][j][1].val,
					myData->SubSensV.org_data.ref_val[i][j][2].val,
					myData->SubSensV.org_data.ref_val[i][j][3].val);
			}
		}
	}
	printf("\n");

	//for(i=0; i < 8; i++) {
	for(i=0; i < 1; i++) {
		for(j=0; j < 4; j++) {
			if(myData->mData.config.daq_type == DAQ_TYPE1) {
				printf("%02x %02x, %02x %02x ",
					myData->SubSensV.org_data.sub_val[i][j][0].byte[1],
					myData->SubSensV.org_data.sub_val[i][j][0].byte[0],
					myData->SubSensV.org_data.sub_val[i][j][1].byte[1],
					myData->SubSensV.org_data.sub_val[i][j][1].byte[0]);
			} else {
				printf("%02x %02x, %02x %02x, %02x %02x, %02x %02x\n",
					myData->SubSensV.org_data.sub_val[i][j][0].byte[1],
					myData->SubSensV.org_data.sub_val[i][j][0].byte[0],
					myData->SubSensV.org_data.sub_val[i][j][1].byte[1],
					myData->SubSensV.org_data.sub_val[i][j][1].byte[0],
					myData->SubSensV.org_data.sub_val[i][j][2].byte[1],
					myData->SubSensV.org_data.sub_val[i][j][2].byte[0],
					myData->SubSensV.org_data.sub_val[i][j][3].byte[1],
					myData->SubSensV.org_data.sub_val[i][j][3].byte[0]);
			}
		}
	}
	printf("%02x %02x\n", myData->SubSensV.org_data.equal_val[0],
		myData->SubSensV.org_data.equal_val[1]);
	*/
/*
	printf("SubSensV count:%d flag:%d\n",
		myData->mData.misc.SubSensV_SourceSensCount,
		myData->mData.misc.SubSensV_SourceSensCountFlag);
	//for(i=0; i < MAX_SUB_SENS_V_SOURCE_SENS_COUNT; i++) {
	for(i=0; i < 200; i++) {
		printf("%d ", myData->SubSensV.source_div1.source[0][0].tmpV[0][i]);
	}
	printf("\n");
*/
	switch(myData->mData.config.daq_type) {
		case DAQ_TYPE1:	k = 2; break;
		case DAQ_TYPE2:	k = 4; break;
		case DAQ_TYPE3:	k = 4; break;
		default:		k = 4; break;
	}
/*
	for(j=0; j < k; j++) {
		switch(j) {
			case 0: printf("sourceA\n"); break;
			case 1: printf("sourceB\n"); break;
			case 2: printf("sourceC\n"); break;
			case 3: printf("sourceD\n"); break;
			default:	break;
		}
		for(i=0; i < 4; i++) {
   			printf("sensV[%d] %ld, %ld, %ld\n", i,
			myData->SubSensV.source_div1.source[i][j].sensV[0],
			myData->SubSensV.source_div1.source[i][j].sensV[1],
			myData->SubSensV.source_div1.source[i][j].sensV[2]);
	   		printf("calV[%d] %ld, %ld, %ld\n", i,
			myData->SubSensV.source_div1.source[i][j].calV[0],
			myData->SubSensV.source_div1.source[i][j].calV[1],
			myData->SubSensV.source_div1.source[i][j].calV[2]);
			printf("V_AD a:%f, b:%f, a_N:%f, b_N:%f\n",
				myData->SubSensV.source_div1.source[i][j].V_AD_a,
				myData->SubSensV.source_div1.source[i][j].V_AD_b,
				myData->SubSensV.source_div1.source[i][j].V_AD_a_N,
				myData->SubSensV.source_div1.source[i][j].V_AD_b_N);
		}
		printf("\n");
	}

	switch(myData->mData.config.daq_type) {
		case DAQ_TYPE1:	k = 0; break;
		case DAQ_TYPE2:	k = 0; break;
		case DAQ_TYPE3:	k = 4; break;
		default:		k = 0; break;
	}

	for(j=0; j < k; j++) {
		switch(j) {
			case 0: printf("sourceE\n"); break;
			case 1: printf("sourceF\n"); break;
			case 2: printf("sourceG\n"); break;
			case 3: printf("sourceH\n"); break;
			default:	break;
		}
		for(i=0; i < 4; i++) {
   			printf("sensV[%d] %ld, %ld, %ld\n", i,
			myData->SubSensV.source_div2.source[i][j].sensV[0],
			myData->SubSensV.source_div2.source[i][j].sensV[1],
			myData->SubSensV.source_div2.source[i][j].sensV[2]);
	   		printf("calV[%d] %ld, %ld, %ld\n", i,
			myData->SubSensV.source_div2.source[i][j].calV[0],
			myData->SubSensV.source_div2.source[i][j].calV[1],
			myData->SubSensV.source_div2.source[i][j].calV[2]);
			printf("V_AD a:%f, b:%f, a_N:%f, b_N:%f\n",
				myData->SubSensV.source_div2.source[i][j].V_AD_a,
				myData->SubSensV.source_div2.source[i][j].V_AD_b,
				myData->SubSensV.source_div2.source[i][j].V_AD_a_N,
				myData->SubSensV.source_div2.source[i][j].V_AD_b_N);
		}
		printf("\n");
	}
*/
//ktg_210105s
/*

	//j = 64;
	j = myData->mData.config.installedAuxV;

	printf("sub_val\n");
	printf("tmpSensV ");
	for(i=0; i < 64; i++) {
		printf("%ld[%d] ", myData->SubSensV.ch[i].tmpSensV, i);
	}
	printf("\n");

	printf("sensV ");
	//for(0; i < j; i++) {
*/	//for(i=0; i < 64; i++) {
/*
	for(i=0; i < 4; i++) {
		printf("%ld[%d] ", myData->SubSensV.ch[i].sensV, i);
	}
	printf("\n");
	//ktg_210105e

	printf("preSensV ");
	for(i=0; i < 4; i++) {
		printf("%ld[%d](%ld) ", myData->SubSensV.ch[i].preSensV, i, myData->SubSensV.ch[i].sensV - myData->SubSensV.ch[i].preSensV);
	}
	printf("\n");

 //csk_190816

	printf("SIG_FAULT_DELTA_AUX_V_PHASE\n");
	for(i=0; i < 4; i++) {
		printf("%d ", myData->cData[i].signal[C_SIG_FAULT_AUX_DELTA_V_PHASE]);
	}
	printf("\n");
	
	printf("fault_delta_AuxV_Time\n");
	for(i=0; i < 4; i++) {
		printf("%06ld ", myData->cData[i].misc.check_fault_delta_AuxV_Time);
	}
	printf("\n");
	
	printf("checkDelayTime       \n");
	for(i=0; i < 4; i++) {
		printf("%06ld ", myData->cData[i].misc.checkDelayTime);
	}
	printf("\n");
	
	//ktg_210105s	
*/
	/*
	i=0;
	step = 1;
	printf("GUI VALUE %d\n", step);
	idx = IDX_LOC_OBJ_FAULT_SVDF_YT_1;
	val1 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_yt1;
	
	idx = IDX_LOC_OBJ_FAULT_SVDF_VCELL_AVER;
	val2 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_vcell_aver;

	idx = IDX_LOC_OBJ_FAULT_SVDF_YT_2;
	val3 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_yt2;

	idx = IDX_LOC_OBJ_FAULT_SVDF_YT_3;
	val4 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_yt3;

	printf("YT1 %ld, VCELL_AVER %ld, YT2 %ld YT3 %ld\n",val1, val2, val3, val4);

	idx = IDX_LOC_OBJ_FAULT_SVDF_COUNT;
	val1 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_count;

	idx = IDX_LOC_OBJ_FAULT_SVDF_VENT_FLAG;
	val2 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_vent_flag;

	printf("SVDF_COUNT %ld, SVDF_VENT %ld\n\n",val1, val2);

	idx = IDX_LOC_OBJ_FAULT_SVDF_USE_FLAG;
	val1 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_use_flag;

	printf("USE SVDF %ld\n\n",val1);

	i=0;
	step = 4;
	printf("GUI VALUE %d\n", step);
	idx = IDX_LOC_OBJ_FAULT_SVDF_YT_1;
	val1 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_yt1;
	
	idx = IDX_LOC_OBJ_FAULT_SVDF_VCELL_AVER;
	val2 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_vcell_aver;

	idx = IDX_LOC_OBJ_FAULT_SVDF_YT_2;
	val3 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_yt2;

	idx = IDX_LOC_OBJ_FAULT_SVDF_YT_3;
	val4 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_yt3;

	printf("YT1 %ld, VCELL_AVER %ld, YT2 %ld YT3 %ld\n",val1, val2, val3, val4);

	idx = IDX_LOC_OBJ_FAULT_SVDF_COUNT;
	val1 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_count;

	idx = IDX_LOC_OBJ_FAULT_SVDF_VENT_FLAG;
	val2 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_vent_flag;

	printf("SVDF_COUNT %ld, SVDF_VENT %ld\n\n",val1, val2);

	idx = IDX_LOC_OBJ_FAULT_SVDF_USE_FLAG;
	val1 = (long)myData->COA_Client[i].testCond.step[step].faultSVDF_use_flag;

	printf("USE SVDF %ld\n\n",val1);
*/
/*
	printf("SBC_VALUE\n");
	printf("OP.RunTime\n");
	for(i=0; i < 4; i++) {
	printf("%ld\t", myData->cData[i].op.runTime);
	}
	printf("\n");
	printf("prepreAuxV ");
	for(i=0; i < 2; i++) {
		printf("%4ld[%d]\t", myData->SubSensV.ch[i].prepreAuxV, i);
	}
	for(i=43; i < 44; i++) {
		printf("%4ld[%d]\t", myData->SubSensV.ch[i].prepreAuxV, i);
	}
	printf("\n");
	printf("preAuxV    ");
	for(i=0; i < 2; i++) {
		printf("%4ld[%d]\t", myData->SubSensV.ch[i].preAuxV, i);
	}
	for(i=43; i < 44; i++) {
		printf("%4ld[%d]\t", myData->SubSensV.ch[i].preAuxV, i);
	}
	printf("\n");
	printf("SensV      ");
	for(i=0; i < 2; i++) {
		printf("%4ld[%d]\t", myData->SubSensV.ch[i].sensV, i);
	}
	for(i=43; i < 44; i++) {
		printf("%4ld[%d]\t", myData->SubSensV.ch[i].sensV, i);
	}
	printf("\n");
	printf("svdfAuxV   ");
	for(i=0; i < 2; i++) {
		printf("%4ld[%d]\t", myData->SubSensV.ch[i].svdfAuxV, i);
	}
	for(i=43; i < 44; i++) {
		printf("%4ld[%d]\t", myData->SubSensV.ch[i].svdfAuxV, i);
	}
	printf("\n");
	printf("svdfcount  ");
	for(i=0; i < 2; i++) {
		printf("%4ld[%d]\t", myData->SubSensV.ch[i].svdfcount, i);
	}
	for(i=43; i < 44; i++) {
		printf("%4ld[%d]\t", myData->SubSensV.ch[i].svdfcount, i);
	}
	printf("\n");

	printf("C_SIG_SVDF_DETECT_PHASE ");
	for(i=0; i < 1; i++) {
		printf("%d\t", myData->cData[i].signal[C_SIG_SVDF_DETECT_PHASE]);
	}
	printf("\n");
	
	for(i=0; i < 1; i++) {
		printf("CH %d\n", i+1);
		printf("avgV[2] : %4lduV\t", myData->cData[i].misc.avgAuxV[2]);
		printf("avgV[1] : %4lduV\t", myData->cData[i].misc.avgAuxV[1]);
		printf("avgV[0] : %4lduV\t", myData->cData[i].misc.avgAuxV[0]);
		printf("\n");
	}
*/
	//ktg_210105e
	
	//ktg_210419s
	i=0;
	step = 2;
	printf("GUI VALUE %d\n", step);
	val1 = (long)myData->COA_Client[i].testCond.step[step].faultDropVoltage_use_flag;
	printf("DROP_VOLTAGE_USE_FLAG %ld, \n\n",val1);
	printf("test_val_l1[0] : %ld\n", myData->test_val_l1[0]);
	printf("test_val_l1[1] : %ld\n", myData->test_val_l1[1]);
	printf("test_val_l1[2] : %ld\n", myData->test_val_l1[2]);
	
	val1 = (long)myData->COA_Client[i].testCond.step[step].faultDropVoltage_value;
	printf("IDX_LOC_OBJ_FAULT_DROP_VOLTAGE_VALUE %ld, \n\n",val1);
	
	val1 = (long)myData->COA_Client[i].testCond.step[step].faultDropVoltage_count;
	printf("IDX_LOC_OBJ_FAULT_DROP_VOLTAGE_COUNT %ld, \n\n",val1);
	
	printf("C_SIG_DROP_VOLTAGE_PHASE \n");
	for(i=0; i < 4; i++) {
		printf("%d\t", myData->cData[i].signal[C_SIG_DROP_VOLTAGE_PHASE]);
	}
	printf("\n");
	for(i=0; i < 4; i++) {
		printf("%4ld[%1d]\t", myData->SubSensV.ch[i].DropmaxAuxV, i);
	}
	printf("\n");
	
	printf("DropAuxVCount \n");
	printf("\n");
	for(i=0; i < 4; i++) {
		printf("%d\t", myData->SubSensV.ch[i].DropAuxVcount);
	}
	printf("\n");
	//ktg_210419e
	
	/*
	printf("test_val_l1 ");
	printf("0 : %ld, 1 : %ld, 2 : %ld, 3 : %ld, 4 : %ld, 5 : %ld, 6 : %ld, 7 : %ld, 8 : %ld\n",
		myData->test_val_l1[0], myData->test_val_l1[1], myData->test_val_l1[2],
		myData->test_val_l1[3], myData->test_val_l1[4], myData->test_val_l1[5],
		myData->test_val_l1[6], myData->test_val_l1[7], myData->test_val_l1[8]);
	printf("\n");
	*/
	printf("\n");
}

void AuxV_Print(void)
{
	int i;

	printf("AuxSetData\n");
	for(i=0; i < 64; i++) {
		printf("auxChNo : %d, auxType : %d, chNo : %d, name : %s, tableNo : %d\n", 
			(int)myData->auxSetData[i].auxChNo,
			(int)myData->auxSetData[i].auxType,
			(int)myData->auxSetData[i].chNo,
			myData->auxSetData[i].name,
			(int)myData->auxSetData[i].tableNo);
	}
	printf("temp_count ch1:%d ch2:%d\n", myData->auxDataCount[0][0],
		myData->auxDataCount[1][0]);
	printf("auxV_count ch1:%d ch2:%d\n", myData->auxDataCount[0][1],
		myData->auxDataCount[1][1]);
	printf("T  H_count ch1:%d ch2:%d\n", myData->auxDataCount[0][2],
		myData->auxDataCount[1][2]);
	printf("\n");
	//kjhw_150731
	printf("ch1 minV: %ld, maxV: %ld, diffAuxV: %ld\n",
		myData->cData[0].misc.minAuxV,
		myData->cData[0].misc.maxAuxV,
		myData->cData[0].misc.diffAuxV);
	printf("ch2 minV: %ld, maxV: %ld, diffAuxV: %ld\n",
		myData->cData[1].misc.minAuxV,
		myData->cData[1].misc.maxAuxV,
		myData->cData[1].misc.diffAuxV);
	printf("\n");
}

void Humidity_Print(void)		//khj_191205
{
	int i,j;

	printf("HUMIDITY_Data\n");
	
	for(i=0; i < 6; i++){	
		for(j=0; j < 21; j++){
			printf("table : %d\t", i);	
			printf("humidity_type : %d\t",myData->humidity_table[i].humidity_type);
			printf("V_HUMIDITY%d : %f\n", j ,myData->humidity_table[i].H_V[j][1]);
		}
	}

}

void Gas_Print(void)		//sec_220926
{
	int i;

	printf("Gas_Data\n");
	for(i=282; i < 286; i++) {
		printf("auxChNo : %d, auxType : %d, chNo : %d, name : %s, sensV : %d\n", 
			(int)myData->auxSetData[i].auxChNo,
			(int)myData->auxSetData[i].auxType,
			(int)myData->auxSetData[i].chNo,
			myData->auxSetData[i].name,
			(int)myData->SubSensV.ch[i].sensV);
	}
	printf("gas_count ch1:%d ch2:%d\n", myData->auxDataCount[0][4],
		myData->auxDataCount[1][4]);
	printf("\n");
}



void AuxTemp_Print(void)
{
	int i, ch_per_meter;
	char fileName[256];
	FILE *fp;

	printf("AuxTemp\n");

	if(myData->COM.config.functionModel[1] == 0) ch_per_meter = 8;
	else ch_per_meter = 16;

	printf("value ");
	for(i=0; i < (myData->COM.config.countMeter[1] * ch_per_meter); i++) {
		printf("%ld(%d) ",
		myData->COM.com_port[AUX_TEMP].value[i],
		myData->COM.com_port[AUX_TEMP].fault_count[i]);
	}
	printf("\n");

	printf("kkkkkkkkkkkkkkkk\n");
	for(i=0; i < (myData->COM.config.countMeter[1] * ch_per_meter); i++) {
		if(myData->COM.com_port[AUX_TEMP].fault_count[i] > 0) {
			printf("%ld(%d)\n ",
			myData->COM.com_port[AUX_TEMP].value[i],
			myData->COM.com_port[AUX_TEMP].fault_count[i]);

			memset(fileName, 0, sizeof(fileName));

			sprintf(fileName, "./TEMP_fault_list");
			if((fp = fopen(fileName, "w")) == NULL) {
				printf("Can not open\n");
				break;
			}
			fprintf(fp, "temp_count ");
			fprintf(fp, "%d : %ld(%d)\n ",
				i,
				myData->COM.com_port[AUX_TEMP].value[i],
				myData->COM.com_port[AUX_TEMP].fault_count[i]);
			fprintf(fp,"\n");
			fclose(fp);
		}
	}
	printf("kkkkkkkkkkkkkkkk\n");
	/*
	for(i=0; i < (myData->COM.config.countMeter[1] * ch_per_meter); i++) {
		printf("%ld ", myData->COM.com_port[1].value[i]);
	}*/
	printf("\n");

	printf("\n");
}

void Message_Print(void)
{
	int msg, fromPs, write_idx;

	printf("Message\n");

	fromPs = MODULE_TO_APP;
	write_idx = myData->msg[fromPs].write_idx;
	printf("write_idx %d, read_idx %d, send_msg %d\n",
		write_idx, myData->msg[fromPs].read_idx,
		myData->msg[fromPs].msg_val[write_idx].msg);

	msg = 0;
	printf("save send_flag %d\n",
		myData->save_msg[msg].send_flag);
	printf("save count %d, write_idx %d, read_idx %d\n",
		myData->save_msg[msg].total_count,
		myData->save_msg[msg].write_idx,
		myData->save_msg[msg].read_idx);

	msg = 1;
	printf("save send_flag %d\n",
		myData->save_msg[msg].send_flag);
	printf("save count %d, write_idx %d, read_idx %d\n",
		myData->save_msg[msg].total_count,
		myData->save_msg[msg].write_idx,
		myData->save_msg[msg].read_idx);
	printf("\n");
}

void COM_Print(void)
{
	int comPort, i;

	i = 0;
	printf("Load_Process_COM ");
	for(i=0; i < 12; i++) {
		printf("%d ", myData->AppControl.misc.Load_Process_COM[i]);
	}
	printf("\n");

	for(comPort=0; comPort < 5; comPort++) {
		printf("Type:%d, Model:%d, port:%d\n",
			myData->COM.config.functionType[comPort],
			myData->COM.config.functionModel[comPort],
			myData->COM.config.comPortId[comPort]
			);
/*		printf("com:%d rcvPacket:", comPort);
		for(i=0; i < 64; i++) {
			printf("%02x ", (unsigned char)myData->COM.com_port[comPort]
				.rcvPacket.rcvPacketBuf[i]);
		}
		printf("\n");

		printf("com:%d cmdBuf:", comPort);
		for(i=0; i < 64; i++) {
			//if(myData->COM.com_port[comPort].rcvCmd.cmd[i] == 0x2B)
			//	printf("\n");
			printf("%02x ",
				(unsigned char)myData->COM.com_port[comPort].rcvCmd.cmdBuf[i]);
		}
		printf("\n");

		printf("com:%d cmd:", comPort);
		for(i=0; i < 64; i++) {
			printf("%02x ",
				(unsigned char)myData->COM.com_port[comPort].rcvCmd.cmd[i]);
		}
		printf("\n");*/

/*		printf("rcvCount %d, parseCount %d\n",
			myData->COM.com_port[comPort].rcvPacket.rcvCount,
			myData->COM.com_port[comPort].rcvPacket.parseCount);
		printf("cmdBufSize %d, cmdSize %d\n",
			myData->COM.com_port[comPort].rcvCmd.cmdBufSize,
			myData->COM.com_port[comPort].rcvCmd.cmdSize);*/
/*
		if(comPort == 0) {
			printf("signal[REQUEST_PHASE]:%d, READ_COUNT:%d, RETRY_COUNT:%d\n",
				myData->COM.com_port[comPort].signal[COM_SIG_BCR_REQUEST_PHASE],
				myData->COM.com_port[comPort].signal[COM_SIG_BCR_READ_COUNT],
				myData->COM.com_port[comPort].signal[COM_SIG_BCR_RETRY_COUNT]);
			printf("\n");

			printf("com:%d tray_id[0]:", comPort);
			for(i=0; i < COM_BCR_SIZE; i++) {
				if(myData->COM.com_port[comPort].rcvCmd.cmd[i] == 0x2B)
					printf("\n");
				printf("%c ", (unsigned char)myData->COM.com_port[comPort]
					.misc.tmp_tray_id[0][i]);
			}
			printf("\n");

			printf("com:%d tray_id[1]:", comPort);
			for(i=0; i < COM_BCR_SIZE; i++) {
				if(myData->COM.com_port[comPort].rcvCmd.cmd[i] == 0x2B)
					printf("\n");
				printf("%c ", (unsigned char)myData->COM.com_port[comPort]
					.misc.tmp_tray_id[1][i]);
			}
			printf("\n");

			printf("com:%d tray_id[2]:", comPort);
			for(i=0; i < COM_BCR_SIZE; i++) {
				if(myData->COM.com_port[comPort].rcvCmd.cmd[i] == 0x2B)
					printf("\n");
				printf("%c ", (unsigned char)myData->COM.com_port[comPort]
					.misc.tmp_tray_id[2][i]);
			}
			printf("\n");
		}*/
/*
		printf("com:%d, signal[DISPLAY]:%d, countMeter:%d\n",
			comPort,
			myData->COM.com_port[comPort].signal[COM_SIG_DISPLAY],
			myData->COM.config.countMeter[comPort]);
		printf("retry_time %d, retry_count %d\n",
			myData->COM.com_port[comPort].misc.retry_time,
			myData->COM.com_port[comPort].misc.retry_count);
		printf("singal[METER_INIT]:%d, signal[METER_REQ_PHA]:%d\n",
			myData->COM.com_port[comPort].signal[COM_SIG_CALI_METER_INITIALIZE],
			myData->COM.com_port[comPort]
				.signal[COM_SIG_CALI_METER_REQUEST_PHASE]);*/

		printf("plc_idle_time %ld\n",
			myData->COM.com_port[comPort].misc.plc_idle_time);
		printf("signal[PLC_REQUEST_PHASE]:%d\n",
			myData->COM.com_port[comPort].signal[COM_SIG_PLC_REQUEST_PHASE]);
		printf("\n");
	}
	printf("\n");
}

void CAN_Print(void)
{
	int i, j, ch=0;
	long tmp;

	printf("canReceiveDataCount %d %d\n",
		myData->canReceiveDataCount[ch][0], myData->canReceiveDataCount[ch][1]);
	printf("canTransmitDataCount %d %d\n",
		myData->canTransmitDataCount[ch][0],
		myData->canTransmitDataCount[ch][1]);

	printf("val1 ");
	for(i=0; i < 15; i++) {
		printf("%f ", myData->CanData[ch][i].f_val[0]);
	}
	printf("\n");

	printf("val2\n");
	j = 0;
	for(i=20; i < 40; i++) {
		printf("%x ", (unsigned int)myData->CanData[ch][i].ul_val[0]);
		j++;
		if((j % 2) == 0) printf("\n");
	}
	printf("\n");
	printf("flagkkkkkkkkkkkkkkkkkkkkkk\n");
	printf("%d\n", myData->tmpCanData_sort_flag[3][0][0]);
	printf("candata\n");
	printf("%f\n", myData->CanData[3][0].f_val[0]);
	printf("tmpcandata\n");
	printf("%f\n", myData->tmpCanData[3][0][0].f_val[0]);

	for(i=0; i < 2; i++) {
		printf("ch(%d) master_slave %d : manual_can_bitconfig:%d norminal_brp:%d norminal_tseg1:%d norminal_tseg2:%d norminal_sjw:%d nominal_sample_point:%f
			data_brp:%d data_tseg1:%d data_tseg2:%d data_sjw:%d data_sample_point:%f\n"
			, ch, i,
			myData->canReceiveSetData.commonData[ch][i].manual_can_bitconfig,
			myData->canReceiveSetData.commonData[ch][i].nominal_brp,
			myData->canReceiveSetData.commonData[ch][i].nominal_tseg1,
			myData->canReceiveSetData.commonData[ch][i].nominal_tseg2,
			myData->canReceiveSetData.commonData[ch][i].nominal_sjw,
			myData->canReceiveSetData.commonData[ch][i].nominal_sample_point,
			myData->canReceiveSetData.commonData[ch][i].data_brp,
			myData->canReceiveSetData.commonData[ch][i].data_tseg1,
			myData->canReceiveSetData.commonData[ch][i].data_tseg2,
			myData->canReceiveSetData.commonData[ch][i].data_sjw,
			myData->canReceiveSetData.commonData[ch][i].data_sample_point);
		printf("\n");
	}


	printf("ch(%d) Nominal : BRP:%ld tseg1:%ld tseg2:%ld sjw:%ld\n"
			, ch,
			myData->test_val_l1[7], //BRP
			myData->test_val_l1[8], //TSEG1
			myData->test_val_l1[9], //TSEG2
			myData->test_val_l1[10]);//SJW
	printf("\n");
	printf("ch(%d) Data : BRP:%ld tseg1:%ld tseg2:%ld sjw:%ld\n"
			, ch,
			myData->test_val_l1[11], //BRP
			myData->test_val_l1[12], //TSEG1
			myData->test_val_l1[13], //TSEG2
			myData->test_val_l1[14]);//SJW
	printf("\n");
/*	

	for(i=0; i < 10; i++) {
		printf("%d canType:%d, canID:%ld, startBit:%d, bitCount:%d, factor(ud):%ld, factor(ld):%ld, factor(f):%f, func_div:%d\n", i,
			myData->canReceiveSetData.normalData[ch][i].canType,
			myData->canReceiveSetData.normalData[ch][i].canID,
			myData->canReceiveSetData.normalData[ch][i].startBit,
			myData->canReceiveSetData.normalData[ch][i].bitCount,
			(unsigned long)myData->canReceiveSetData.normalData[ch][i].factor,
			(long)myData->canReceiveSetData.normalData[ch][i].factor,
			myData->canReceiveSetData.normalData[ch][i].factor,
			myData->canReceiveSetData.normalData[ch][i].function_div);
		printf("value_ul(%d) %ld %ld\n", i,
			myData->CanData[ch][i].ul_val[0],
			myData->CanData[ch][i].ul_val[1]);
		//printf("value_l(%d) %ld %ld\n", i,
		//	myData->CanData[ch][i].l_val[0],
		//	myData->CanData[ch][i].l_val[1]);
		printf("value_f(%d) %f %f\n", i,
			myData->CanData[ch][i].f_val[0],
			myData->CanData[ch][i].f_val[1]);
		tmp = (long)myData->CanData[ch][i].f_val[0];
		//printf("tmp %ld %d\n", tmp, (unsigned char)tmp);
		printf("value_c(%d) %02x %02x %02x %02x %02x %02x %02x %02x\n", i,
			myData->CanData[ch][i].c_val[0],
			myData->CanData[ch][i].c_val[1],
			myData->CanData[ch][i].c_val[2],
			myData->CanData[ch][i].c_val[3],
			myData->CanData[ch][i].c_val[4],
			myData->CanData[ch][i].c_val[5],
			myData->CanData[ch][i].c_val[6],
			myData->CanData[ch][i].c_val[7]);
		printf("\n");
	}

	j = myData->canReceiveDataCount[ch][0];
	for(i=j; i < (j+5); i++) {
		printf("%d canType:%d, canID:%ld, startBit:%d, bitCount:%d, factor(ud):%ld, factor(ld):%ld, factor(f):%f\n", i,
			myData->canReceiveSetData.normalData[ch][i].canType,
			myData->canReceiveSetData.normalData[ch][i].canID,
			myData->canReceiveSetData.normalData[ch][i].startBit,
			myData->canReceiveSetData.normalData[ch][i].bitCount,
			(unsigned long)myData->canReceiveSetData.normalData[ch][i].factor,
			(long)myData->canReceiveSetData.normalData[ch][i].factor,
			myData->canReceiveSetData.normalData[ch][i].factor);
		printf("value_ul(%d) %ld %ld\n", i,
			myData->CanData[ch][i].ul_val[0],
			myData->CanData[ch][i].ul_val[1]);
		printf("value_l(%d) %ld %ld\n", i,
			myData->CanData[ch][i].l_val[0],
			myData->CanData[ch][i].l_val[1]);
		printf("value_f(%d) %f %f\n", i,
			myData->CanData[ch][i].f_val[0],
			myData->CanData[ch][i].f_val[1]);
		printf("value_c(%d) %02x %02x %02x %02x %02x %02x %02x %02x\n", i,
			myData->CanData[ch][i].c_val[0],
			myData->CanData[ch][i].c_val[1],
			myData->CanData[ch][i].c_val[2],
			myData->CanData[ch][i].c_val[3],
			myData->CanData[ch][i].c_val[4],
			myData->CanData[ch][i].c_val[5],
			myData->CanData[ch][i].c_val[6],
			myData->CanData[ch][i].c_val[7]);
		printf("\n");
	}
*/
/*	for(i=0; i < 20; i++) {
		printf("%d %ld %ld\n", i, myData->canTransmitTime[ch][i],
			myData->canTransmitSetData.normalData[ch][i].send_period);
	}*/
/*

	printf("comm_time ");
	for(i=0; i < myData->mData.config.installedCAN; i++) {
		printf("(%ld, %ld) %ld", myData->CAN.receive_time[i],
			myData->CAN.transmit_time[i], myData->mData.misc.timer_1sec);
	}
	printf("\n");

	printf("signal ");
	for(i=0; i < myData->mData.config.installedCAN; i++) {
		printf("%d ",
			myData->CAN.signal[i][CAN_SIG_ABORT_TRANSMISSION]);
	}
	printf("\n");

	printf("rt_can_control_phase ");
	for(i=0; i < myData->mData.config.installedCAN; i++) {
		printf("%d ",
			myData->CAN.rt_can_control_phase[i]);
	}
	printf("\n");

	printf("signal %d %d %d\n",
		myData->dio.signal[DIO_SIG_IN_EXTERNAL1],
		myData->CAN.signal[ch][CAN_SIG_START_TRANSMISSION],
		myData->CAN.signal[ch][CAN_SIG_TX_PHASE_INV_CAP_V]);
	printf("signal %d %d %d\n",
		myData->CAN.signal[ch][CAN_SIG_TX_PHASE_MCU_TEMP],
		myData->CAN.signal[ch][CAN_SIG_RX_PHASE_FAN_SPEED],
		myData->CAN.signal[ch][CAN_SIG_FAN_TEST]);
	printf("signal %d %d %d\n",
		myData->CAN.signal[ch][CAN_SIG_ECU_ID_REQUEST],
		myData->CAN.signal[ch][CAN_SIG_TX_PHASE_ECU_ID],
		myData->CAN.signal[ch][CAN_SIG_RX_PHASE_ECU_ID]);
	printf("signal daq_mux %d %d %d %d\n",
		myData->CAN.signal[0][CAN_SIG_TX_PHASE_DAQ_MUX],
		myData->CAN.signal[1][CAN_SIG_TX_PHASE_DAQ_MUX],
		myData->CAN.signal[2][CAN_SIG_TX_PHASE_DAQ_MUX],
		myData->CAN.signal[3][CAN_SIG_TX_PHASE_DAQ_MUX]);
	printf("signal charger_phase %d %d %d\n",
		myData->CAN.signal[ch][CAN_SIG_CHARGER_PHASE],
		myData->dio.signal[DIO_SIG_IN_CHARGER_WAKEUP],
		myData->dio.signal[DIO_SIG_IN_CHARGER_PHASE]);
	printf("signal %d %d %d\n",
		myData->CAN.signal[ch][CAN_SIG_TX_PHASE_DEBUG_ON],
		myData->CAN.signal[ch][CAN_SIG_TX_PHASE_CALIBRATION],
		myData->CAN.signal[ch][CAN_SIG_TX_PHASE_FLASH_CMD]);*/
/*	printf("signal %d %d %d, %d %d %d, %d %d %d, %d %d %d\n",
		myData->CAN.signal[0][CAN_SIG_TX_PHASE_TEST1],
		myData->CAN.signal[0][CAN_SIG_TX_PHASE_OP_TYPE],
		myData->CAN.signal[0][CAN_SIG_TX_PHASE_WAKE_UP],
		myData->CAN.signal[1][CAN_SIG_TX_PHASE_TEST1],
		myData->CAN.signal[1][CAN_SIG_TX_PHASE_OP_TYPE],
		myData->CAN.signal[1][CAN_SIG_TX_PHASE_WAKE_UP],
		myData->CAN.signal[2][CAN_SIG_TX_PHASE_TEST1],
		myData->CAN.signal[2][CAN_SIG_TX_PHASE_OP_TYPE],
		myData->CAN.signal[2][CAN_SIG_TX_PHASE_WAKE_UP],
		myData->CAN.signal[3][CAN_SIG_TX_PHASE_TEST1],
		myData->CAN.signal[3][CAN_SIG_TX_PHASE_OP_TYPE],
		myData->CAN.signal[3][CAN_SIG_TX_PHASE_WAKE_UP]);

	for(i=0; i < myData->mData.config.installedCAN; i++) {
		printf("uds_rx[%d] tx_uds_phase(%d, %d) %x, %x, %x, %x, %x, %x, %x, %x\n", i,
			myData->CAN.signal[i][CAN_SIG_TX_PHASE_UDS_DATA1],
			myData->CAN.signal[i][CAN_SIG_TX_PHASE_UDS_DATA_INDEX],
			myData->CAN.uds_rx[i].uc_val[0], myData->CAN.uds_rx[i].uc_val[1],
			myData->CAN.uds_rx[i].uc_val[2], myData->CAN.uds_rx[i].uc_val[3],
			myData->CAN.uds_rx[i].uc_val[4], myData->CAN.uds_rx[i].uc_val[5],
			myData->CAN.uds_rx[i].uc_val[6], myData->CAN.uds_rx[i].uc_val[7]);
	}

	for(i=0; i < myData->mData.config.installedCAN; i++) {
		printf("%d main_on %d, main_off %d\n", i,
			myData->CAN.signal[i][CAN_SIG_TX_PHASE_MAIN_ON],
			myData->CAN.signal[i][CAN_SIG_TX_PHASE_MAIN_OFF]);
	}*/

	printf("\n");
}

void Watt_limited(void)
{
}

void Chamber_Print(void)
{
	int i;

	for(i=0; i < MAX_COM_PORT; i++) {
		if(myData->COM.config.functionType[i] == COM_FUNC_TYPE_CHAMBER1) {
			break;
		}
	}
	if(i >= MAX_COM_PORT) return;

	printf("TEMP_NPV %d, TEMP_NSP %d, NOWSTS %x,OTHERSTS %x\n",
		myData->COM.com_port[i].misc.TEMP_NPV,
		myData->COM.com_port[i].misc.TEMP_NSP,
		myData->COM.com_port[i].misc.NOWSTS,
		myData->COM.com_port[i].misc.OTHERSTS);
	printf("PROC_TIME_H %d, PROC_TIME_L %d\n",
		myData->COM.com_port[i].misc.PROC_TIME_H,
		myData->COM.com_port[i].misc.PROC_TIME_L);

	printf("\n");
}

void jhk_test(void)
{
	float f_val, f_val2;
	long l_val, l_val2;
	double d_val, d_val2;

	f_val = 25.700001;
	l_val = f_val * 1000 + 0.5;
	printf("f_val : %f, l_val : %ld\n", f_val, l_val);
	d_val = l_val;
	printf("d_val %f\n", d_val);
	d_val /= (long)1000;
	printf("d_val %f\n", d_val);
	f_val = d_val;
	printf("f_val %f\n", f_val);

	f_val2 = 25.699999;
	l_val2 = f_val2 * 1000 + 0.5;
	printf("f_val2 : %f, l_val2 : %ld\n", f_val2, l_val2);
	d_val2 = l_val2;
	printf("d_val2 %f\n", d_val2);
	d_val2 /= (long)1000;
	printf("d_val2 %f\n", d_val2);
	f_val2 = d_val2;
	printf("f_val2 %f\n", f_val2);

	f_val = 25.700000;
	f_val2 = 25.7;
	printf("f_val %f, f_val2 %f\n", f_val, f_val2);
	f_val = (float)25.700000;
	f_val2 = (float)25.7;
	printf("f_val %f, f_val2 %f\n", f_val, f_val2);

	printf("\n");
}

int data_10ms_write(int scan_ch, int div)
{
	char fileName[256];
	int i;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	if(div == 0) {
		sprintf(fileName, "./data_10ms_data_%d.csv", scan_ch+1);
	} else if(div == 1) {
		sprintf(fileName, "./data_10ms_data_tmp_%d.csv", scan_ch+1);
	} else if(div == 2) {
		sprintf(fileName, "./data_10ms_data_d_%d.csv", scan_ch+1);
	} else {
		sprintf(fileName, "./data_10ms_data_tmp_d_%d.csv", scan_ch+1);
	}

	if((fp = fopen(fileName, "w")) == NULL) {
		if(div == 0) {
			printf("Can not open 10ms_data file(write)\n");
		} else if(div == 1) {
			printf("Can not open 10ms_data_tmp file(write)\n");
		} else if(div == 2) {
			printf("Can not open 10ms_data_d file(write)\n");
		} else {
			printf("Can not open 10ms_data_tmp_d file(write)\n");
		}
		return -1;
	}

	fprintf(fp, "t, v, i, w\n");
	if(div == 0) {
		for(i=0; i < myData->data_10ms_count[scan_ch]; i++) {
			fprintf(fp, "%ld, %ld, %ld, %ld\n",
				myData->data_10ms[scan_ch][0][i],
				myData->data_10ms[scan_ch][1][i],
				myData->data_10ms[scan_ch][2][i],
				myData->data_10ms[scan_ch][3][i]);
		}
	} else if(div == 1) {
		for(i=0; i < myData->data_10ms_count[scan_ch]; i++) {
			fprintf(fp, "%ld, %ld, %ld, %ld\n",
				myData->data_10ms_tmp[scan_ch][0][i],
				myData->data_10ms_tmp[scan_ch][1][i],
				myData->data_10ms_tmp[scan_ch][2][i],
				myData->data_10ms_tmp[scan_ch][3][i]);
		}
	} /*else if(div == 2) {
		for(i=0; i < myData->data_10ms_count_d[scan_ch]; i++) {
			fprintf(fp, "%ld, %ld, %ld, %ld\n",
				myData->data_10ms_d[scan_ch][0][i],
				myData->data_10ms_d[scan_ch][1][i],
				myData->data_10ms_d[scan_ch][2][i],
				myData->data_10ms_d[scan_ch][3][i]);
		}
	} else {
		for(i=0; i < myData->data_10ms_count_d[scan_ch]; i++) {
			fprintf(fp, "%ld, %ld, %ld, %ld\n",
				myData->data_10ms_tmp_d[scan_ch][0][i],
				myData->data_10ms_tmp_d[scan_ch][1][i],
				myData->data_10ms_tmp_d[scan_ch][2][i],
				myData->data_10ms_tmp_d[scan_ch][3][i]);
		}
	}*/

	fclose(fp);

	return 0;
}

int COA_Client_TestCond_write_V1009(int div)
{
//khjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj	
	myData->cData[0].op.runTime = 8639500; //23:59:55
/*
	char fileName[256];
	int i,j,k = 0;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));

	if(div == 0) {
		sprintf(fileName, "./COA1_Client_Receive_Data_V1009");
//	} else if(div == 1) {
//		sprintf(fileName, "./data_10ms_data_tmp_%d.csv", scan_ch+1);
//	} else if(div == 2) {
//		sprintf(fileName, "./data_10ms_data_d_%d.csv", scan_ch+1);
	} else {
		sprintf(fileName, "./Pack_Client_Receive_Data_V1009");
	}

	if((fp = fopen(fileName, "w")) == NULL) {
		if(div == 0) {
			printf("Can not open COA1_Client_Receive_Data file(write)\n");
//		} else if(div == 1) {
//			printf("Can not open 10ms_data_tmp file(write)\n");
//		} else if(div == 2) {
//			printf("Can not open 10ms_data_d file(write)\n");
		} else {
			printf("Can not open Pack_Client_Receive_Data file(write)\n");
		}
		return -1;
	}

//	fprintf(fp, "t, v, i, w\n");

	if(div == 0) {
		fprintf(fp, "COA1_Client_Receive_Data_V1009 \n");
		fprintf(fp,"\n");

		fprintf(fp, "Test_Cond_Header \n");
		fprintf(fp, "totalStep	:	%d \n",
				myData->COA_Client[div].testCond.header.totalStep);
		fprintf(fp, "reserverd1[3]	:	%d, %d, %d \n",
				myData->COA_Client[div].testCond.header.reserved1[0],
				myData->COA_Client[div].testCond.header.reserved1[1],
				myData->COA_Client[div].testCond.header.reserved1[2]);
		fprintf(fp, "reserverd2[2]	:	%ld, %ld \n",
				myData->COA_Client[div].testCond.header.reserved2[0],
				myData->COA_Client[div].testCond.header.reserved2[1]);
		printf("\n");

		fprintf(fp, "Test_Cond_Safety \n");
		fprintf(fp, "faultLowerV	:	%ld \n",
				myData->COA_Client[div].testCond.safety.faultLowerV);
		fprintf(fp, "faultUpperV	:	%ld \n",
				myData->COA_Client[div].testCond.safety.faultUpperV);
		fprintf(fp, "faultLowerI	:	%ld \n",
				myData->COA_Client[div].testCond.safety.faultLowerI);
		fprintf(fp, "faultUpperI	:	%ld \n",
				myData->COA_Client[div].testCond.safety.faultUpperI);
		fprintf(fp, "faultLower_AmpareHour	:	%ld \n",
				myData->COA_Client[div].testCond.safety.faultLower_AmpareHour);
		fprintf(fp, "faultUpper_AmpareHour	:	%ld \n",
				myData->COA_Client[div].testCond.safety.faultUpper_AmpareHour);
		fprintf(fp, "faultLowerTemp	:	%ld \n",
				myData->COA_Client[div].testCond.safety.faultLowerTemp);
		fprintf(fp, "faultUpperTemp	:	%ld \n",
				myData->COA_Client[div].testCond.safety.faultUpperTemp);
		fprintf(fp, "reserved1[4]	:	%ld, %ld, %ld, %ld \n",
				myData->COA_Client[div].testCond.safety.reserved1[0],
				myData->COA_Client[div].testCond.safety.reserved1[1],
				myData->COA_Client[div].testCond.safety.reserved1[2],
				myData->COA_Client[div].testCond.safety.reserved1[3]);
		fprintf(fp,"\n");

		fprintf(fp, "Test_Cond_Step \n");
		fprintf(fp,"\n");
		for(i=0; i < myData->COA_Client[div].testCond.header.totalStep; i++){
			fprintf(fp, "Step %d \n",i+1);
			fprintf(fp, "Step %d Header \n",i+1);
			fprintf(fp, "type	:	%d \n",
				myData->COA_Client[div].testCond.step[i].header.type);
			fprintf(fp, "stepNo	:	%d \n",
				myData->COA_Client[div].testCond.step[i].header.stepNo);
			fprintf(fp, "mode	:	%d \n",
				myData->COA_Client[div].testCond.step[i].header.mode);
			fprintf(fp, "testEnd	:	%d \n",
				myData->COA_Client[div].testCond.step[i].header.testEnd);
			fprintf(fp, "subStep	:	%d \n",
				myData->COA_Client[div].testCond.step[i].header.subStep);

			fprintf(fp, "reserved1	:	%d, \n",
				myData->COA_Client[div].testCond.step[i].header.reserved1);
			fprintf(fp, "patternIndex	:	%d \n",
				myData->COA_Client[div].testCond.step[i].header.patternIndex);
			fprintf(fp, "reserved2[2]	:	%d, %d \n",
				myData->COA_Client[div].testCond.step[i].header.reserved2[0],
				myData->COA_Client[div].testCond.step[i].header.reserved2[1]);
		fprintf(fp,"\n");
			j = 0;
//			for(j=0;j < MAX_P1_SUB_STEP ; j++){
				fprintf(fp, "Step Reference[%d][%d] \n",i,j);
				fprintf(fp, "refV	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].refV);
				fprintf(fp, "refI	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].refI);
				fprintf(fp, "rangeI	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].rangeI);
				fprintf(fp, "ValueRate_Compare	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].ValueRate_Compare);
				fprintf(fp, "ValueRate	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].ValueRate);
				fprintf(fp, "Time	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Time);
				fprintf(fp, "V_Upper	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].V_Upper);
				fprintf(fp, "Current	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Current);
				fprintf(fp, "AmpareHour_SumAmpareHour	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].AmpareHour_SumAmpareHour);
				fprintf(fp, "V_Upper_Branch_CycleCount_Branch	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].V_Upper_Branch_CycleCount_Branch);
				fprintf(fp, "V_Lower_Branch_CycleCount	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].V_Lower_Branch_CycleCount);
				fprintf(fp, "DeltaV	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].DeltaV);
				fprintf(fp, "Power	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Power);
				fprintf(fp, "WattHour_SumWattHour	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].WattHour_SumWattHour);
				fprintf(fp, "Time_Branch_AccCycleCount	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Time_Branch_AccCycleCount);
				fprintf(fp, "CVTime_Branch_AccCycleCountId	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CVTime_Branch_AccCycleCountId);
				fprintf(fp, "CVTime	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CVTime);
				fprintf(fp, "Aux_V_Upper	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_V_Upper);
				fprintf(fp, "Aux_V_Lower	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_V_Lower);
				fprintf(fp, "Aux_Temp_Upper	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_Temp_Upper);
				fprintf(fp, "Aux_Temp_Lower	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_Temp_Lower);
				fprintf(fp, "Aux_V_Upper_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_V_Upper_Branch);
				fprintf(fp, "Aux_V_Lower_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_V_Lower_Branch);
				fprintf(fp, "Aux_Temp_Upper_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_Temp_Upper_Branch);
				fprintf(fp, "Aux_Temp_Lower_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].Aux_Temp_Lower_Branch);
				fprintf(fp, "CAN_V_Upper	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_V_Upper);
				fprintf(fp, "CAN_V_Lower	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_V_Lower);
				fprintf(fp, "CAN_Temp_Upper	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Temp_Upper);
				fprintf(fp, "CAN_Temp_Lower	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Temp_Lower);
				fprintf(fp, "CAN_Soc_Upper	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Soc_Upper);
				fprintf(fp, "CAN_Soc_Lower	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Soc_Lower);
				fprintf(fp, "CAN_Fault	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Fault);
				fprintf(fp, "CAN_V_Upper_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_V_Upper_Branch);
				fprintf(fp, "CAN_V_Lower_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_V_Lower_Branch);
				fprintf(fp, "CAN_Temp_Upper_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Temp_Upper_Branch);
				fprintf(fp, "CAN_Temp_Lower_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Temp_Lower_Branch);
				fprintf(fp, "CAN_Soc_Upper_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Soc_Upper_Branch);
				fprintf(fp, "CAN_Soc_Lower_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Soc_Lower_Branch);
				fprintf(fp, "CAN_Fault_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].CAN_Fault_Branch);
				fprintf(fp, "ValueRate_Item	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].ValueRate_Item);
				fprintf(fp, "V_Lower_MultiCycleCount	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].V_Lower_MultiCycleCount);
				fprintf(fp, "AmpareHour_Branch_MultiCycleCountId	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].AmpareHour_Branch_MultiCycleCountId);
				fprintf(fp, "WattHour_Branch_MultiCycleCount_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].WattHour_Branch_MultiCycleCount_Branch);
				fprintf(fp, "ValueRate_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].ValueRate_Branch);
				fprintf(fp, "AccCycleCount_Branch	:	%d \n",
					myData->COA_Client[div].testCond.step[i].reference[j].AccCycleCount_Branch);
				fprintf(fp, "reserved2[2]	:	%ld, %ld \n",
					myData->COA_Client[div].testCond.step[i].reference[j].reserved2[0],
					myData->COA_Client[div].testCond.step[i].reference[j].reserved2[1]);
		fprintf(fp,"\n");
//			}

			for(j=0;j < MAX_P1_COMP_POINT ; j++){
				fprintf(fp, "Step CompV[%d][%d] \n",i,j);
				fprintf(fp, "lowerValue	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].compV[j].lowerValue);
				fprintf(fp, "upperValue	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].compV[j].upperValue);
				fprintf(fp, "time	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].compV[j].time);
		fprintf(fp,"\n");
			}

			for(j=0;j < MAX_P1_COMP_POINT ; j++){
				fprintf(fp, "Step CompI[%d][%d] \n",i,j);
				fprintf(fp, "lowerValue	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].compI[j].lowerValue);
				fprintf(fp, "upperValue	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].compI[j].upperValue);
				fprintf(fp, "time	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].compI[j].time);
		fprintf(fp,"\n");
			}

			fprintf(fp, "Step %d DeltaV \n",i+1);
			fprintf(fp, "lowerValue	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].deltaV.lowerValue);
			fprintf(fp, "upperValue	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].deltaV.upperValue);
			fprintf(fp, "time	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].deltaV.time);
		fprintf(fp,"\n");

			fprintf(fp, "Step %d DeltaI  \n",i+1);
			fprintf(fp, "lowerValue	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].deltaI.lowerValue);
			fprintf(fp, "upperValue	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].deltaI.upperValue);
			fprintf(fp, "time	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].deltaI.time);
		fprintf(fp,"\n");

			fprintf(fp, "Step %d Record \n",i+1);
			fprintf(fp, "deltaT	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].record.deltaT);
			fprintf(fp, "deltaV	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].record.deltaV);
			fprintf(fp, "deltaI	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].record.deltaI);
			fprintf(fp, "deltaTemp	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].record.deltaTemp);
			fprintf(fp, "deltaP	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].record.deltaP);
			fprintf(fp, "reserved	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].record.reserved);
		fprintf(fp,"\n");

			fprintf(fp, "Step %d Edlc \n",i+1);
			fprintf(fp, "capacitanceV1	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].edlc.capacitanceV1);
			fprintf(fp, "capacitanceV2	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].edlc.capacitanceV2);
			fprintf(fp, "startT_Z	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].edlc.startT_Z);
			fprintf(fp, "endT_Z	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].edlc.endT_Z);
			fprintf(fp, "startT_LC	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].edlc.startT_LC);
			fprintf(fp, "endT_LC	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].edlc.endT_LC);
		fprintf(fp,"\n");

			j = 0;
//			for(j=0;j < MAX_P1_GRADE_ITEM ; j++){
				fprintf(fp, "Step Grade[%d][%d] \n",i,j);
				fprintf(fp, "item	:	%d \n",
					myData->COA_Client[div].testCond.step[i].grade[j].item);
				fprintf(fp, "gradeStepCount	:	%d \n",
					myData->COA_Client[div].testCond.step[i].grade[j].gradeStepCount);
				fprintf(fp, "reserved1	:	%d \n",
					myData->COA_Client[div].testCond.step[i].grade[j].reserved1);
		fprintf(fp,"\n");

				k = 0;
		//		for(k=0;k < MAX_P1_GRADE_STEP ; k++){
					fprintf(fp, "Step GradeStep[%d][%d][%d] \n",i,j,k);
					fprintf(fp, "gradeCode	:	%d \n",
					myData->COA_Client[div].testCond.step[i].grade[j].gradeStep[k].gradeCode);
					fprintf(fp, "reserved1	:	%d \n",
					myData->COA_Client[div].testCond.step[i].grade[j].gradeStep[k].reserved1);
					fprintf(fp, "reserved2	:	%d \n",
					myData->COA_Client[div].testCond.step[i].grade[j].gradeStep[k].reserved2);
					fprintf(fp, "lowerValue	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].grade[j].gradeStep[k].lowerValue);
					fprintf(fp, "upperValue	:	%ld \n",
					myData->COA_Client[div].testCond.step[i].grade[j].gradeStep[k].upperValue);
		fprintf(fp,"\n");

//				}
//			}

		fprintf(fp, "Test_Cond_Step \n");

			fprintf(fp, "faultUpperV	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultUpperV);
			fprintf(fp, "faultLowerV	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultLowerV);
			fprintf(fp, "faultUpperI	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultUpperI);
			fprintf(fp, "faultLowerI	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultLowerI);
			fprintf(fp, "faultUpper_AmpareHour	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultUpper_AmpareHour);
			fprintf(fp, "faultLower_AmpareHour	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultLower_AmpareHour);
			fprintf(fp, "faultUpperZ	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultUpperZ);
			fprintf(fp, "faultLowerZ	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultLowerZ);
			fprintf(fp, "faultUpperTemp	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultUpperTemp);
			fprintf(fp, "faultLowerTemp	:	%ld \n",
				myData->COA_Client[div].testCond.step[i].faultLowerTemp);
			fprintf(fp, "reserved1[2]	:	%ld, %ld \n",
				myData->COA_Client[div].testCond.step[i].reserved1[0],
				myData->COA_Client[div].testCond.step[i].reserved1[1]);
		fprintf(fp,"\n");
		}

		fprintf(fp, "Test_Cond \n");
		fprintf(fp, "stepCount	:	%d \n",
			myData->COA_Client[div].testCond.stepCount);
		fprintf(fp, "reserved1	:	%d \n",
			myData->COA_Client[div].testCond.reserved1);
	}

	fclose(fp);
*/
	return 0;
}

int COA_Client_TestCond_write_V100B(int div)
{
#ifdef __COA_VER_100B__
	char fileName[256];
	int i,j,k = 0;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "./COA%d_Client_TestCond_V100B", div+1);

	if((fp = fopen(fileName, "w")) == NULL) {
		printf("Can not open COA%d_Client_TestCond file(write)\n", div+1);
		return -1;
	}

	fprintf(fp, "COA%d_Client_Receive_Data_V100B\n", div+1);
	fprintf(fp,"\n");

	fprintf(fp, "TestCond_Header\n");
	fprintf(fp, "totalStep : %d\n",
		myData->COA_Client[div].testCond.header.totalStep);
	fprintf(fp, "reserverd1[3] : %d, %d, %d\n",
		myData->COA_Client[div].testCond.header.reserved1[0],
		myData->COA_Client[div].testCond.header.reserved1[1],
		myData->COA_Client[div].testCond.header.reserved1[2]);
	fprintf(fp, "reserverd2[2] : %ld, %ld\n",
		myData->COA_Client[div].testCond.header.reserved2[0],
		myData->COA_Client[div].testCond.header.reserved2[1]);
	fprintf(fp, "\n");

	fprintf(fp, "Common_Safety\n");
	fprintf(fp, "faultLowerV : %ld\n",
		myData->COA_Client[div].testCond.safety.faultLowerV);
	fprintf(fp, "faultUpperV : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperV);
	fprintf(fp, "reserved1 : %ld\n",
		myData->COA_Client[div].testCond.safety.reserved1);
	fprintf(fp, "faultUpperI : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperI);
	fprintf(fp, "faultLowerTemp : %ld\n",
		myData->COA_Client[div].testCond.safety.faultLowerTemp);
	fprintf(fp, "faultUpperTemp : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperTemp);
	fprintf(fp, "faultUpper_AmpareHour : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpper_AmpareHour);
	fprintf(fp, "faultUpperP : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperP);
	fprintf(fp, "faultUpper_WattHour : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpper_WattHour);
	fprintf(fp,"\n");

	fprintf(fp, "CAN_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.can_func_div[i],
			myData->COA_Client[div].testCond.safety.can_compare_type[i],
			myData->COA_Client[div].testCond.safety.can_data_type[i],
			myData->COA_Client[div].testCond.safety.can_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "Aux_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.aux_func_div[i],
			myData->COA_Client[div].testCond.safety.aux_compare_type[i],
			myData->COA_Client[div].testCond.safety.aux_data_type[i],
			myData->COA_Client[div].testCond.safety.aux_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "reserved2[4] : %ld, %ld, %ld, %ld\n",
		myData->COA_Client[div].testCond.safety.reserved2[0],
		myData->COA_Client[div].testCond.safety.reserved2[1],
		myData->COA_Client[div].testCond.safety.reserved2[2],
		myData->COA_Client[div].testCond.safety.reserved2[3]);
	fprintf(fp,"\n");

	fprintf(fp, "TestCond_Step\n");
	fprintf(fp,"\n");

	for(i=0; i < myData->COA_Client[div].testCond.header.totalStep; i++) {
		fprintf(fp, "Step_Header : %d\n", i+1);
		fprintf(fp, "type : %d\n",
			myData->COA_Client[div].testCond.step[i].header.type);
		fprintf(fp, "stepNo : %d\n",
			myData->COA_Client[div].testCond.step[i].header.stepNo);
		fprintf(fp, "mode : %d\n",
			myData->COA_Client[div].testCond.step[i].header.mode);
		fprintf(fp, "testEnd : %d\n",
			myData->COA_Client[div].testCond.step[i].header.testEnd);
		fprintf(fp, "subStep : %d\n",
			myData->COA_Client[div].testCond.step[i].header.subStep);

		fprintf(fp, "reserved1 : %d\n",
			myData->COA_Client[div].testCond.step[i].header.reserved1);
		fprintf(fp, "cycle_pause : %d\n",
			myData->COA_Client[div].testCond.step[i].header.cycle_pause);
		fprintf(fp, "patternIndex : %d\n",
			myData->COA_Client[div].testCond.step[i].header.patternIndex);
		fprintf(fp, "reserved2 : %d\n",
			myData->COA_Client[div].testCond.step[i].header.reserved2);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_SUB_STEP; j++) {
			fprintf(fp, "Step_Reference[%d][%d]\n", i, j);
			fprintf(fp, "refV_upper : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].refV_upper);
			fprintf(fp, "refV_lower : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].refV_lower);
			fprintf(fp, "refI : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refI);
			fprintf(fp, "refP : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refP);
			fprintf(fp, "refR : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refR);
			fprintf(fp, "rangeI : %d\n",
				myData->COA_Client[div].testCond.step[i].reference[j].rangeI);
			fprintf(fp, "ValueRate_Compare : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Compare);
			fprintf(fp, "ValueRate : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate);
			fprintf(fp, "Time : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Time);
			fprintf(fp, "V_Upper : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].V_Upper);
			fprintf(fp, "Current : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Current);
			fprintf(fp, "AmpareHour_SumAmpareHour : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AmpareHour_SumAmpareHour);
			fprintf(fp, "V_Upper_Branch_CycleCount_Branch : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Upper_Branch_CycleCount_Branch);
			fprintf(fp, "V_Lower_Branch_CycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Lower_Branch_CycleCount);
			fprintf(fp, "DeltaV : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].DeltaV);
			fprintf(fp, "Power : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Power);
			fprintf(fp, "WattHour_SumWattHour : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].WattHour_SumWattHour);
			fprintf(fp, "Time_Branch_AccCycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].Time_Branch_AccCycleCount);
			fprintf(fp, "CVTime_Branch_AccCycleCountId : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].CVTime_Branch_AccCycleCountId);
			fprintf(fp, "CVTime : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].CVTime);
			fprintf(fp, "reserved[3] : %d, %d, %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[0],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[1],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[2]);
			fprintf(fp, "ValueRate_Item : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Item);
			fprintf(fp, "V_Lower_MultiCycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Lower_MultiCycleCount);
			fprintf(fp, "AmpareHour_Branch_MultiCycleCountId : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AmpareHour_Branch_MultiCycleCountId);
			fprintf(fp, "WattHour_Branch_MultiCycleCount_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].WattHour_Branch_MultiCycleCount_Branch);
			fprintf(fp, "ValueRate_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Branch);
			fprintf(fp, "AccCycleCount_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AccCycleCount_Branch);
			fprintf(fp,"\n");

			fprintf(fp, "Step_CAN\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value\n");
			for(k=0; k < MAX_P1_CAN_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %f\n", k+1,
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_func_div[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_compare_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_data_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_branch[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_value[k]);
			}
			fprintf(fp,"\n");

			fprintf(fp, "Step_Aux\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value\n");
			for(k=0; k < MAX_P1_AUX_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %f\n", k+1,
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_func_div[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_compare_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_data_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_branch[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_value[k]);
			}
			fprintf(fp,"\n");

			fprintf(fp, "reserved2[2] : %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved2[0],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved2[1]);
			fprintf(fp,"\n");
		}

		fprintf(fp, "Step_CompV[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i].compV[j].lowerValue,
				myData->COA_Client[div].testCond.step[i].compV[j].upperValue,
				myData->COA_Client[div].testCond.step[i].compV[j].time);
		}
		fprintf(fp,"\n");

		fprintf(fp, "Step_CompI[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i].compI[j].lowerValue,
				myData->COA_Client[div].testCond.step[i].compI[j].upperValue,
				myData->COA_Client[div].testCond.step[i].compI[j].time);
		}
		fprintf(fp,"\n");

		fprintf(fp, "Step_DeltaV\n");
		fprintf(fp, "lowerValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.upperValue);
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_DeltaI\n");
		fprintf(fp, "lowerValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.upperValue);
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_Record\n");
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.time);
		fprintf(fp, "deltaV : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaV);
		fprintf(fp, "deltaI : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaI);
		fprintf(fp, "deltaT : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaT);
		fprintf(fp, "deltaP : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaP);
		fprintf(fp, "reserved : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.reserved);
		fprintf(fp,"\n");

		fprintf(fp, "Step_EDLC\n");
		fprintf(fp, "capacitanceV1 : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.capacitanceV1);
		fprintf(fp, "capacitanceV2 : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.capacitanceV2);
		fprintf(fp, "startT_Z : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.startT_Z);
		fprintf(fp, "endT_Z : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.endT_Z);
		fprintf(fp, "startT_LC : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.startT_LC);
		fprintf(fp, "endT_LC : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.endT_LC);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_GRADE_ITEM; j++) {
			fprintf(fp, "Step_Grade[%d][%d]\n", i, j);
			fprintf(fp, "item : %d\n",
				myData->COA_Client[div].testCond.step[i].grade[j].item);
			fprintf(fp, "gradeStepCount : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.grade[j].gradeStepCount);
			fprintf(fp, "reserved1 : %d\n",
				myData->COA_Client[div].testCond.step[i].grade[j].reserved1);
			fprintf(fp,"\n");

			for(k=0; k < 1; k++) {
//			for(k=0; k < MAX_P1_GRADE_STEP; k++) {
				fprintf(fp, "Step_Grade_Step[%d][%d][%d]\n", i, j, k);
				fprintf(fp, "gradeCode : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].gradeCode);
				fprintf(fp, "reserved1 : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].reserved1);
				fprintf(fp, "reserved2 : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].reserved2);
				fprintf(fp, "lowerValue : %ld\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].lowerValue);
				fprintf(fp, "upperValue : %ld\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].upperValue);
				fprintf(fp,"\n");
			}
		}

		fprintf(fp, "TestCond_Step_Fault\n");
		fprintf(fp, "faultUpperV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperV);
		fprintf(fp, "faultLowerV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerV);
		fprintf(fp, "faultUpperI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperI);
		fprintf(fp, "faultLowerI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerI);
		fprintf(fp, "faultUpper_AmpareHour : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpper_AmpareHour);
		fprintf(fp, "faultLower_AmpareHour : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLower_AmpareHour);
		fprintf(fp, "faultUpperZ : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperZ);
		fprintf(fp, "faultLowerZ : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerZ);
		fprintf(fp, "faultUpperTemp : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperTemp);
		fprintf(fp, "faultLowerTemp : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerTemp);
		fprintf(fp, "reserved1[2] : %ld, %ld\n",
			myData->COA_Client[div].testCond.step[i].reserved1[0],
			myData->COA_Client[div].testCond.step[i].reserved1[1]);
		fprintf(fp,"\n");
	}

	fprintf(fp, "TestCond_ETC\n");
	fprintf(fp, "stepCount : %d\n", myData->COA_Client[div].testCond.stepCount);
	fprintf(fp, "reserved1 : %d\n", myData->COA_Client[div].testCond.reserved1);

	fclose(fp);
#endif
	return 0;
}

int COA_Client_TestCond_write_V100C(int div)
{
#ifdef __COA_VER_100C__
	char fileName[256];
	int i,j,k = 0;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "./COA%d_Client_TestCond_V100C", div+1);

	if((fp = fopen(fileName, "w")) == NULL) {
		printf("Can not open COA%d_Client_TestCond file(write)\n", div+1);
		return -1;
	}

	fprintf(fp, "COA%d_Client_Receive_Data_V100C\n", div+1);
	fprintf(fp,"\n");

	fprintf(fp, "TestCond_Header\n");
	fprintf(fp, "totalStep : %d\n",
		myData->COA_Client[div].testCond.header.totalStep);
	fprintf(fp, "reserverd1[3] : %d, %d, %d\n",
		myData->COA_Client[div].testCond.header.reserved1[0],
		myData->COA_Client[div].testCond.header.reserved1[1],
		myData->COA_Client[div].testCond.header.reserved1[2]);
	fprintf(fp, "reserverd2[2] : %ld, %ld\n",
		myData->COA_Client[div].testCond.header.reserved2[0],
		myData->COA_Client[div].testCond.header.reserved2[1]);
	fprintf(fp, "\n");

	fprintf(fp, "Common_Safety\n");
	fprintf(fp, "faultLowerV : %ld\n",
		myData->COA_Client[div].testCond.safety.faultLowerV);
	fprintf(fp, "faultUpperV : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperV);
	fprintf(fp, "reserved1 : %ld\n",
		myData->COA_Client[div].testCond.safety.reserved1);
	fprintf(fp, "faultUpperI : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperI);
	fprintf(fp, "faultLowerTemp : %ld\n",
		myData->COA_Client[div].testCond.safety.faultLowerTemp);
	fprintf(fp, "faultUpperTemp : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperTemp);
	fprintf(fp, "faultUpper_AmpareHour : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpper_AmpareHour);
	fprintf(fp, "faultUpperP : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperP);
	fprintf(fp, "faultUpper_WattHour : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpper_WattHour);
	fprintf(fp,"\n");

	fprintf(fp, "CAN_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.can_func_div[i],
			myData->COA_Client[div].testCond.safety.can_compare_type[i],
			myData->COA_Client[div].testCond.safety.can_data_type[i],
			myData->COA_Client[div].testCond.safety.can_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "Aux_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.aux_func_div[i],
			myData->COA_Client[div].testCond.safety.aux_compare_type[i],
			myData->COA_Client[div].testCond.safety.aux_data_type[i],
			myData->COA_Client[div].testCond.safety.aux_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "reserved2[4] : %ld, %ld, %ld, %ld\n",
		myData->COA_Client[div].testCond.safety.reserved2[0],
		myData->COA_Client[div].testCond.safety.reserved2[1],
		myData->COA_Client[div].testCond.safety.reserved2[2],
		myData->COA_Client[div].testCond.safety.reserved2[3]);
	fprintf(fp,"\n");

	fprintf(fp, "TestCond_Step\n");
	fprintf(fp,"\n");

	for(i=0; i < myData->COA_Client[div].testCond.header.totalStep; i++) {
		fprintf(fp, "Step_Header : %d\n", i+1);
		fprintf(fp, "type : %d\n",
			myData->COA_Client[div].testCond.step[i].header.type);
		fprintf(fp,

		fp
			myData->COA_Client[div].testCo
		fprintf(fp, "testEnd : %d\n",
			myData->COA_Client[div].testCond.step[i].header.testEnd);
		fprintf(fp, "subStep : %d\n",
			myData->COA_Client[div].testCond.step[i].header.subStep);

		fprintf(fp, "reserved1 : %d\n",
			myData->COA_Client[div].testCond.step[i].header.reserved1);
		fprintf(fp, "cycle_pause : %d\n",
			myData->COA_Client[div].testCond.step[i].header.cycle_pause);
		fprintf(fp, "patternIndex : %d\n",
			myData->COA_Client[div].testCond.step[i].header.patternIndex);
		fprintf(fp, "reserved2 : %d\n",
			myData->COA_Client[div].testCond.step[i].header.reserved2);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_SUB_STEP; j++) {
			fprintf(fp, "Step_Reference[%d][%d]\n", i, j);
			fprintf(fp, "refV_upper : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].refV_upper);
			fprintf(fp, "refV_lower : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].refV_lower);
			fprintf(fp, "refI : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refI);
			fprintf(fp, "refP : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refP);
			fprintf(fp, "refR : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refR);
			fprintf(fp, "rangeI : %d\n",
				myData->COA_Client[div].testCond.step[i].reference[j].rangeI);
			fprintf(fp, "ValueRate_Compare : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Compare);
			fprintf(fp, "ValueRate : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate);
			fprintf(fp, "endTime_day : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].endTime_day);
			fprintf(fp, "endTime : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].endTime);
			fprintf(fp, "V_Upper : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].V_Upper);
			fprintf(fp, "Current : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Current);
			fprintf(fp, "AmpareHour_SumAmpareHour : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AmpareHour_SumAmpareHour);
			fprintf(fp, "V_Upper_Branch_CycleCount_Branch : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Upper_Branch_CycleCount_Branch);
			fprintf(fp, "V_Lower_Branch_CycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Lower_Branch_CycleCount);
			fprintf(fp, "DeltaV : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].DeltaV);
			fprintf(fp, "Power : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Power);
			fprintf(fp, "WattHour_SumWattHour : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].WattHour_SumWattHour);
			fprintf(fp, "Time_Branch_AccCycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].Time_Branch_AccCycleCount);
			fprintf(fp, "CVTime_Branch_AccCycleCountId : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].CVTime_Branch_AccCycleCountId);
			fprintf(fp, "CVTime_day : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].CVTime_day);
			fprintf(fp, "CVTime : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].CVTime);
			fprintf(fp, "reserved[3] : %d, %d, %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[0],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[1],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[2]);
			fprintf(fp, "ValueRate_Item : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Item);
			fprintf(fp, "V_Lower_MultiCycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Lower_MultiCycleCount);
			fprintf(fp, "AmpareHour_Branch_MultiCycleCountId : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AmpareHour_Branch_MultiCycleCountId);
			fprintf(fp, "WattHour_Branch_MultiCycleCount_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].WattHour_Branch_MultiCycleCount_Branch);
			fprintf(fp, "ValueRate_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Branch);
			fprintf(fp, "AccCycleCount_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AccCycleCount_Branch);
			fprintf(fp,"\n");

			fprintf(fp, "Step_CAN\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value\n");
			for(k=0; k < MAX_P1_CAN_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %f\n", k+1,
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_func_div[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_compare_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_data_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_branch[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_value[k]);
			}
			fprintf(fp,"\n");

			fprintf(fp, "Step_Aux\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value\n");
			for(k=0; k < MAX_P1_AUX_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %f\n", k+1,
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_func_div[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_compare_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_data_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_branch[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_value[k]);
			}
			fprintf(fp,"\n");

			fprintf(fp, "reserved2[2] : %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved2[0],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved2[1]);
			fprintf(fp,"\n");
		}

		fprintf(fp, "Step_CompV[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i].compV[j].lowerValue,
				myData->COA_Client[div].testCond.step[i].compV[j].upperValue,
				myData->COA_Client[div].testCond.step[i].compV[j].time);
		}
		fprintf(fp,"\n");

		fprintf(fp, "Step_CompI[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i].compI[j].lowerValue,
				myData->COA_Client[div].testCond.step[i].compI[j].upperValue,
				myData->COA_Client[div].testCond.step[i].compI[j].time);
		}
		fprintf(fp,"\n");

		fprintf(fp, "Step_DeltaV\n");
		fprintf(fp, "lowerValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.upperValue);
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_DeltaI\n");
		fprintf(fp, "lowerValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.upperValue);
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_Record\n");
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.time);
		fprintf(fp, "deltaV : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaV);
		fprintf(fp, "deltaI : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaI);
		fprintf(fp, "deltaT : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaT);
		fprintf(fp, "deltaP : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaP);
		fprintf(fp, "reserved : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.reserved);
		fprintf(fp,"\n");

		fprintf(fp, "Step_EDLC\n");
		fprintf(fp, "capacitanceV1 : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.capacitanceV1);
		fprintf(fp, "capacitanceV2 : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.capacitanceV2);
		fprintf(fp, "startT_Z : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.startT_Z);
		fprintf(fp, "endT_Z : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.endT_Z);
		fprintf(fp, "startT_LC : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.startT_LC);
		fprintf(fp, "endT_LC : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.endT_LC);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_GRADE_ITEM; j++) {
			fprintf(fp, "Step_Grade[%d][%d]\n", i, j);
			fprintf(fp, "item : %d\n",
				myData->COA_Client[div].testCond.step[i].grade[j].item);
			fprintf(fp, "gradeStepCount : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.grade[j].gradeStepCount);
			fprintf(fp, "reserved1 : %d\n",
				myData->COA_Client[div].testCond.step[i].grade[j].reserved1);
			fprintf(fp,"\n");

			for(k=0; k < 1; k++) {
//			for(k=0; k < MAX_P1_GRADE_STEP; k++) {
				fprintf(fp, "Step_Grade_Step[%d][%d][%d]\n", i, j, k);
				fprintf(fp, "gradeCode : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].gradeCode);
				fprintf(fp, "reserved1 : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].reserved1);
				fprintf(fp, "reserved2 : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].reserved2);
				fprintf(fp, "lowerValue : %ld\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].lowerValue);
				fprintf(fp, "upperValue : %ld\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].upperValue);
				fprintf(fp,"\n");
			}
		}

		fprintf(fp, "TestCond_Step_Fault\n");
		fprintf(fp, "faultUpperV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperV);
		fprintf(fp, "faultLowerV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerV);
		fprintf(fp, "faultUpperI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperI);
		fprintf(fp, "faultLowerI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerI);
		fprintf(fp, "faultUpper_AmpareHour : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpper_AmpareHour);
		fprintf(fp, "faultLower_AmpareHour : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLower_AmpareHour);
		fprintf(fp, "faultUpperZ : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperZ);
		fprintf(fp, "faultLowerZ : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerZ);
		fprintf(fp, "faultUpperTemp : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperTemp);
		fprintf(fp, "faultLowerTemp : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerTemp);
		fprintf(fp, "reserved1[2] : %ld, %ld\n",
			myData->COA_Client[div].testCond.step[i].reserved1[0],
			myData->COA_Client[div].testCond.step[i].reserved1[1]);
		fprintf(fp,"\n");
	}

	fprintf(fp, "TestCond_ETC\n");
	fprintf(fp, "stepCount : %d\n", myData->COA_Client[div].testCond.stepCount);
	fprintf(fp, "reserved1 : %d\n", myData->COA_Client[div].testCond.reserved1);

	fclose(fp);
#endif
	return 0;
}

int COA_Client_TestCond_write_V100D(int div)
{
	char fileName[256];
	int i,j,k,l = 0;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));

#ifdef __COA_VER_100D__
	sprintf(fileName, "./COA%d_Client_TestCond_V100D", div+1);
#elif __COA_VER_100F__
	sprintf(fileName, "./COA%d_Client_TestCond_V100F", div+1);
#elif __COA_VER_1011__
	sprintf(fileName, "./COA%d_Client_TestCond_V1011", div+1);
#elif __COA_VER_1013__
	sprintf(fileName, "./COA%d_Client_TestCond_V1013", div+1);
#elif __COA_VER_1014__
	sprintf(fileName, "./COA%d_Client_TestCond_V1014", div+1);
#elif __COA_VER_1015__
	sprintf(fileName, "./COA%d_Client_TestCond_V1015", div+1);
#else
	sprintf(fileName, "./COA%d_Client_TestCond_V10**", div+1);
#endif

	if((fp = fopen(fileName, "w")) == NULL) {
		printf("Can not open COA%d_Client_TestCond file(write)\n", div+1);
		return -1;
	}

#ifdef __COA_VER_100D__
	fprintf(fp, "COA%d_Client_Receive_Data_V100D\n", div+1);
#elif __COA_VER_100F__
	fprintf(fp, "COA%d_Client_Receive_Data_V100F\n", div+1);
#elif __COA_VER_1011__
	fprintf(fp, "COA%d_Client_Receive_Data_V1011\n", div+1);
#elif __COA_VER_1013__
	fprintf(fp, "COA%d_Client_Receive_Data_V1013\n", div+1);
#elif __COA_VER_1014__
	fprintf(fp, "COA%d_Client_Receive_Data_V1014\n", div+1);
#else
	fprintf(fp, "COA%d_Client_Receive_Data_V10**\n", div+1);
#endif
	fprintf(fp,"\n");

	fprintf(fp, "TestCond_Header\n");
	fprintf(fp, "totalStep : %d\n",
		myData->COA_Client[div].testCond.header.totalStep);
	fprintf(fp, "totalPatternCount : %d\n",
		myData->COA_Client[div].testCond.header.totalPatternCount);
	fprintf(fp, "reserverd1[2] : %ld, %ld\n",
		myData->COA_Client[div].testCond.header.reserved1[0],
		myData->COA_Client[div].testCond.header.reserved1[1]);
	fprintf(fp, "\n");

	fprintf(fp, "Common_Safety\n");
	fprintf(fp, "faultLowerV : %ld\n",
		myData->COA_Client[div].testCond.safety.faultLowerV);
	fprintf(fp, "faultUpperV : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperV);
	//fprintf(fp, "reserved1 : %ld\n",
	//	myData->COA_Client[div].testCond.safety.reserved1);
	fprintf(fp, "faultComp_AuxV : %ld\n",
		myData->COA_Client[div].testCond.safety.faultCompAuxV);

	fprintf(fp, "faultComp_AuxV2 : %ld\n",
		myData->COA_Client[div].testCond.safety.faultCompAuxV2);
	fprintf(fp, "faultComp_AuxV3 : %ld\n",
		myData->COA_Client[div].testCond.safety.faultCompAuxV3);
	fprintf(fp, "faultComp_Aux_RefV1 : %ld\n",
		myData->COA_Client[div].testCond.safety.faultCompAux_RefV1);
	fprintf(fp, "faultComp_Aux_RefV2 : %ld\n",
		myData->COA_Client[div].testCond.safety.faultCompAux_RefV2);
	fprintf(fp, "faultComp_AuxV_vent : %d\n",
		myData->COA_Client[div].testCond.safety.faultCompAuxV_vent_flag);
	
	fprintf(fp, "faultUpper_AuxV : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpper_AuxV);
	fprintf(fp, "faultLower_AuxV : %ld\n",
		myData->COA_Client[div].testCond.safety.faultLower_AuxV);

	fprintf(fp, "faultUpperI : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperI);
	fprintf(fp, "faultLowerTemp : %ld\n",
		myData->COA_Client[div].testCond.safety.faultLowerTemp);
	fprintf(fp, "faultUpperTemp : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperTemp);
	fprintf(fp, "faultUpper_AmpareHour : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpper_AmpareHour);
	fprintf(fp, "faultUpperP : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperP);
	fprintf(fp, "faultUpper_WattHour : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpper_WattHour);
		for(l=0; l < MAX_AUX_GROUP; l++) {
		fprintf(fp, "V1(%ld), RefV1(%ld), V2(%ld), RefV2(%ld), V3(%ld), Vent(%d)\n",
			myData->COA_Client[div].testCond.safety.faultCompGroupAuxV[l],
			myData->COA_Client[div].testCond.safety.faultCompGroupAux_RefV1[l],
			myData->COA_Client[div].testCond.safety.faultCompGroupAuxV2[l],
			myData->COA_Client[div].testCond.safety.faultCompGroupAux_RefV2[l],
			myData->COA_Client[div].testCond.safety.faultCompGroupAuxV3[l],
			myData->COA_Client[div].testCond.safety.faultCompGroupAuxV_vent_flag[l]);
		}
fprintf(fp,"\n");

	fprintf(fp, "CAN_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.can_func_div[i],
			myData->COA_Client[div].testCond.safety.can_compare_type[i],
			myData->COA_Client[div].testCond.safety.can_data_type[i],
			myData->COA_Client[div].testCond.safety.can_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "Aux_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %ld\n", i+1,
			myData->COA_Client[div].testCond.safety.aux_func_div[i],
			myData->COA_Client[div].testCond.safety.aux_compare_type[i],
			myData->COA_Client[div].testCond.safety.aux_data_type[i],
			myData->COA_Client[div].testCond.safety.aux_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "reserved2[4] : %ld, %ld, %ld, %ld\n",
		myData->COA_Client[div].testCond.safety.reserved2[0],
		myData->COA_Client[div].testCond.safety.reserved2[1],
		myData->COA_Client[div].testCond.safety.reserved2[2],
		myData->COA_Client[div].testCond.safety.reserved2[3]);
	fprintf(fp,"\n");

	fprintf(fp, "TestCond_Step\n");
	fprintf(fp,"\n");

	for(i=0; i < myData->COA_Client[div].testCond.header.totalStep; i++) {
		fprintf(fp, "Step_Header : %d\n", i+1);
		fprintf(fp, "type : %d\n",
			myData->COA_Client[div].testCond.step[i].header.type);
		fprintf(fp, "stepNo : %d\n",
			myData->COA_Client[div].testCond.step[i].header.stepNo);
		fprintf(fp, "mode : %d\n",
			myData->COA_Client[div].testCond.step[i].header.mode);
		fprintf(fp, "testEnd : %d\n",
			myData->COA_Client[div].testCond.step[i].header.testEnd);
		fprintf(fp, "subStep : %d\n",
			myData->COA_Client[div].testCond.step[i].header.subStep);

		fprintf(fp, "reserved1 : %d\n",
			myData->COA_Client[div].testCond.step[i].header.reserved1);
		fprintf(fp, "cycle_pause : %d\n",
			myData->COA_Client[div].testCond.step[i].header.cycle_pause);
		fprintf(fp, "patternIndex : %d\n",
			myData->COA_Client[div].testCond.step[i].header.patternIndex);
		fprintf(fp, "reserved2 : %d\n",
			myData->COA_Client[div].testCond.step[i].header.reserved2);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_SUB_STEP; j++) {
			fprintf(fp, "Step_Reference[%d][%d]\n", i, j);
			fprintf(fp, "refV_upper : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].refV_upper);
			fprintf(fp, "refV_lower : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].refV_lower);
			fprintf(fp, "refI : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refI);
			fprintf(fp, "refP : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refP);
			fprintf(fp, "refR : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].refR);
			fprintf(fp, "rangeI : %d\n",
				myData->COA_Client[div].testCond.step[i].reference[j].rangeI);
			fprintf(fp, "ValueRate_Compare : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Compare);
			fprintf(fp, "ValueRate : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate);
			fprintf(fp, "endTime_day : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].endTime_day);
			fprintf(fp, "endTime : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].endTime);
			fprintf(fp, "V_Upper : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].V_Upper);
			fprintf(fp, "Current : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Current);
			fprintf(fp, "AmpareHour_SumAmpareHour : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AmpareHour_SumAmpareHour);
			fprintf(fp, "V_Upper_Branch_CycleCount_Branch : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Upper_Branch_CycleCount_Branch);
			fprintf(fp, "V_Lower_Branch_CycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Lower_Branch_CycleCount);
			fprintf(fp, "DeltaV : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].DeltaV);
			fprintf(fp, "Power : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].Power);
			fprintf(fp, "WattHour_SumWattHour : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].WattHour_SumWattHour);
			fprintf(fp, "Time_Branch_AccCycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].Time_Branch_AccCycleCount);
			fprintf(fp, "CVTime_Branch_AccCycleCountId : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].CVTime_Branch_AccCycleCountId);
			fprintf(fp, "CVTime_day : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].CVTime_day);
			fprintf(fp, "CVTime : %ld\n",
				myData->COA_Client[div].testCond.step[i].reference[j].CVTime);
			fprintf(fp, "reserved[3] : %d, %d, %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[0],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[1],
				myData->COA_Client[div].testCond.step[i]
				.reference[j].reserved[2]);
			fprintf(fp, "ValueRate_Item : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Item);
			fprintf(fp, "V_Lower_MultiCycleCount : %ld\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].V_Lower_MultiCycleCount);
			fprintf(fp, "AmpareHour_Branch_MultiCycleCountId : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AmpareHour_Branch_MultiCycleCountId);
			fprintf(fp, "WattHour_Branch_MultiCycleCount_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].WattHour_Branch_MultiCycleCount_Branch);
			fprintf(fp, "ValueRate_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].ValueRate_Branch);
			fprintf(fp, "AccCycleCount_Branch : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.reference[j].AccCycleCount_Branch);
			fprintf(fp,"\n");

			fprintf(fp, "Step_CAN\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value\n");
			for(k=0; k < MAX_P1_CAN_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %f\n", k+1,
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_func_div[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_compare_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_data_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_branch[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].can_value[k]);
			}
			fprintf(fp,"\n");

			fprintf(fp, "Step_Aux\n");
			fprintf(fp, "index, Division, Compare_Type, Data_Type, Branch, Value, delay_time\n");
			for(k=0; k < MAX_P1_AUX_FUNCTION; k++) {
				fprintf(fp, "%02d, %d, %d, %d, %d, %ld, %d\n", k+1,
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_func_div[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_compare_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_data_type[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_branch[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_value[k],
					myData->COA_Client[div].testCond.step[i]
					.reference[j].aux_delay_time[k]);
			}
			fprintf(fp,"\n");

			//fprintf(fp, "reserved2[2] : %ld, %ld\n",
			//	myData->COA_Client[div].testCond.step[i]
			//	.reference[j].reserved2[0],
			//	myData->COA_Client[div].testCond.step[i]
			//	.reference[j].reserved2[1]);
			//fprintf(fp,"\n");
		}

		fprintf(fp, "Step_CompV[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i].compV[j].lowerValue,
				myData->COA_Client[div].testCond.step[i].compV[j].upperValue,
				myData->COA_Client[div].testCond.step[i].compV[j].time);
		}
		fprintf(fp,"\n");

		fprintf(fp, "Step_CompI[%d]\n", i);
		fprintf(fp, "lowerVlaue, upperValue, time\n");
		for(j=0; j < MAX_P1_COMP_POINT; j++) {
			fprintf(fp, "%ld, %ld, %ld\n",
				myData->COA_Client[div].testCond.step[i].compI[j].lowerValue,
				myData->COA_Client[div].testCond.step[i].compI[j].upperValue,
				myData->COA_Client[div].testCond.step[i].compI[j].time);
		}
		fprintf(fp,"\n");

		fprintf(fp, "Step_DeltaV\n");
		fprintf(fp, "lowerValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.upperValue);
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaV.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_DeltaI\n");
		fprintf(fp, "lowerValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.lowerValue);
		fprintf(fp, "upperValue : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.upperValue);
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].deltaI.time);
		fprintf(fp,"\n");

		fprintf(fp, "Step_Record\n");
		fprintf(fp, "time : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.time);
		fprintf(fp, "deltaV : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaV);
		fprintf(fp, "deltaI : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaI);
		fprintf(fp, "deltaT : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaT);
		fprintf(fp, "deltaP : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.deltaP);
		fprintf(fp, "reserved : %ld\n",
			myData->COA_Client[div].testCond.step[i].record.reserved);
		fprintf(fp,"\n");

		fprintf(fp, "Step_EDLC\n");
		fprintf(fp, "capacitanceV1 : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.capacitanceV1);
		fprintf(fp, "capacitanceV2 : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.capacitanceV2);
		fprintf(fp, "startT_Z : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.startT_Z);
		fprintf(fp, "endT_Z : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.endT_Z);
		fprintf(fp, "startT_LC : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.startT_LC);
		fprintf(fp, "endT_LC : %ld\n",
			myData->COA_Client[div].testCond.step[i].edlc.endT_LC);
		fprintf(fp,"\n");

		for(j=0; j < 1; j++) {
//		for(j=0; j < MAX_P1_GRADE_ITEM; j++) {
			fprintf(fp, "Step_Grade[%d][%d]\n", i, j);
			fprintf(fp, "item : %d\n",
				myData->COA_Client[div].testCond.step[i].grade[j].item);
			fprintf(fp, "gradeStepCount : %d\n",
				myData->COA_Client[div].testCond.step[i]
				.grade[j].gradeStepCount);
			fprintf(fp, "reserved1 : %d\n",
				myData->COA_Client[div].testCond.step[i].grade[j].reserved1);
			fprintf(fp,"\n");

			for(k=0; k < 1; k++) {
//			for(k=0; k < MAX_P1_GRADE_STEP; k++) {
				fprintf(fp, "Step_Grade_Step[%d][%d][%d]\n", i, j, k);
				fprintf(fp, "gradeCode : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].gradeCode);
				fprintf(fp, "reserved1 : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].reserved1);
				fprintf(fp, "reserved2 : %d\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].reserved2);
				fprintf(fp, "lowerValue : %ld\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].lowerValue);
				fprintf(fp, "upperValue : %ld\n",
					myData->COA_Client[div].testCond.step[i]
					.grade[j].gradeStep[k].upperValue);
				fprintf(fp,"\n");
			}
		}

		fprintf(fp, "TestCond_Step_Fault\n");
		fprintf(fp, "faultUpperV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperV);
		fprintf(fp, "faultLowerV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerV);
		fprintf(fp, "faultUpperI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperI);
		fprintf(fp, "faultLowerI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerI);
		fprintf(fp, "faultUpper_AmpareHour : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpper_AmpareHour);
		fprintf(fp, "faultLower_AmpareHour : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLower_AmpareHour);
		fprintf(fp, "faultUpperZ : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperZ);
		fprintf(fp, "faultLowerZ : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerZ);
		fprintf(fp, "faultUpperTemp : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultUpperTemp);
		fprintf(fp, "faultLowerTemp : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultLowerTemp);

		fprintf(fp, "faultCompAuxV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultCompAuxV);
		fprintf(fp, "faultCompAuxTemp : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultCompAuxTemp);
		fprintf(fp, "faultCompAuxTh : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultCompAuxTh);
		fprintf(fp, "faultCompAuxT : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultCompAuxT);
		fprintf(fp, "faultDelta_AuxV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultDelta_AuxV);
		fprintf(fp, "faultDelta_AuxV_T : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultDelta_AuxV_T);

		/*//jhkw_180823s
		fprintf(fp, "faultDeltaV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultDeltaV);
		fprintf(fp, "faultDeltaV_T : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultDeltaV_T);
		fprintf(fp, "faultDeltaI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultDeltaI);
		fprintf(fp, "faultDeltaI_T : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultDeltaI_T);
		*///jhkw_180823s

		//ktg_210807s
		fprintf(fp, "faultSVDF_vent_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_vent_flag);
		fprintf(fp, "faultSVDF_count : %d\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_count);
		
		fprintf(fp, "faultSVDF_yt1 : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_yt1);
		fprintf(fp, "faultSVDF_vcell_aver : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_vcell_aver);
		fprintf(fp, "faultSVDF_yt2 : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_yt2);
		fprintf(fp, "faultSVDF_yt3 : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_yt3);
		fprintf(fp, "faultSVDF_use_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_use_flag);
		fprintf(fp, "faultcv_interlock_vent_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultcv_interlock_vent_flag);
		fprintf(fp, "faultcv_interlockI : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultcv_interlockI);
		fprintf(fp, "faultDropVoltage_value : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultDropVoltage_value);
		fprintf(fp, "faultDropVoltage_use_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultDropVoltage_use_flag);
		fprintf(fp, "faultDropVoltage_vent_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultDropVoltage_vent_flag);
		fprintf(fp, "faultDropVoltage_count : %d\n",
			myData->COA_Client[div].testCond.step[i].faultDropVoltage_count);
				
		fprintf(fp, "faultSVDF_can_yt1 : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_can_yt1);
		fprintf(fp, "faultSVDF_can_vcell_aver : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_can_vcell_aver);
		fprintf(fp, "faultSVDF_can_yt2 : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_can_yt2);
		fprintf(fp, "faultSVDF_can_yt3 : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_can_yt3);

		fprintf(fp, "faultSVDF_can_use_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_can_use_flag);
		fprintf(fp, "faultSVDF_can_vent_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_can_vent_flag);
		fprintf(fp, "faultSVDF_can_count : %d\n",
			myData->COA_Client[div].testCond.step[i].faultSVDF_can_count);
		fprintf(fp, "faultDropVoltage_can_use_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultDropVoltage_can_use_flag);
		fprintf(fp, "faultDropVoltage_can_vent_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultDropVoltage_can_vent_flag);
		fprintf(fp, "faultDropVoltage_can_count : %d\n",
			myData->COA_Client[div].testCond.step[i].faultDropVoltage_can_count);
		fprintf(fp, "faultCompCanV_use_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultCompCanV_use_flag);
		fprintf(fp, "faultCompCanV_vent_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultCompCanV_vent_flag);

		fprintf(fp, "faultDropVoltage_can_value : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultDropVoltage_can_value);
		fprintf(fp, "faultCompCanV_value : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultCompCanV_value);
		//ktg_210807e
		//ktg_220331s
		fprintf(fp, "faultSoftvent_use_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultSoftvent_use_flag);
		fprintf(fp, "faultSoftvent_vent_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultSoftvent_vent_flag);
		fprintf(fp, "faultSoftvent_count : %d\n",
			myData->COA_Client[div].testCond.step[i].faultSoftvent_count);
		fprintf(fp, "faultSoftvent_value : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultSoftvent_value);
		
		fprintf(fp, "faultHardvent_use_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultHardvent_use_flag);
		fprintf(fp, "faultHardvent_vent_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultHardvent_vent_flag);
		fprintf(fp, "faultHardvent_count : %d\n",
			myData->COA_Client[div].testCond.step[i].faultHardvent_count);
		fprintf(fp, "faultHardvent_value : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultHardvent_value);
		
		fprintf(fp, "faultSoftvent_can_use_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultSoftvent_can_use_flag);
		fprintf(fp, "faultSoftvent_can_vent_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultSoftvent_can_vent_flag);
		fprintf(fp, "faultSoftvent_can_count : %d\n",
			myData->COA_Client[div].testCond.step[i].faultSoftvent_can_count);
		fprintf(fp, "faultSoftvent_can_value : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultSoftvent_can_value);
		
		fprintf(fp, "faultHardvent_can_use_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultHardvent_can_use_flag);
		fprintf(fp, "faultHardvent_can_vent_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultHardvent_can_vent_flag);
		fprintf(fp, "faultHardvent_can_count : %d\n",
			myData->COA_Client[div].testCond.step[i].faultHardvent_can_count);
		fprintf(fp, "faultHardvent_can_value : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultHardvent_can_value);
		//ktg_220331e
		//shhw_220916s
		fprintf(fp, "faultVrising_time_init : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultVrising_time_init);
		fprintf(fp, "faultVrising_time_check : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultVrising_time_check);
		fprintf(fp, "faultVrising_value: %ld\n",
			myData->COA_Client[div].testCond.step[i].faultVrising_reff_value);
		//shhw_220916e
		//shhw_220919s
		fprintf(fp, "faultCC_delta_v_time_init : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultCC_delta_v_time_init);
		fprintf(fp, "faultCC_delta_v_time_period : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultCC_delta_v_time_period);
		fprintf(fp, "faultCC_delta_v_reff_value: %ld\n",
			myData->COA_Client[div].testCond.step[i].faultCC_delta_v_reff_value);
		//shhw_220919e
		fprintf(fp, "pattern_time_type : %d\n",
			myData->COA_Client[div].testCond.step[i].pattern_time_type);
		fprintf(fp, "can_comm_step_check_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].can_comm_step_check_flag);
		fprintf(fp, "fault_can_check_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].fault_can_check_flag);
		fprintf(fp, "can_tx_stop : %d\n",
			myData->COA_Client[div].testCond.step[i].can_tx_stop);
		fprintf(fp, "pattern_max_val : %f\n",
			myData->COA_Client[div].testCond.step[i].pattern_max_val);
		fprintf(fp, "pattern_file_size : %ld\n",
			myData->COA_Client[div].testCond.step[i].pattern_file_size);
		fprintf(fp, "pattern_file_checksum : %ld\n",
			myData->COA_Client[div].testCond.step[i].pattern_file_checksum);
		fprintf(fp, "fault_balance_check_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].fault_balance_check_flag);
		fprintf(fp, "faultCanFreeze_use_flag : %d\n",
			myData->COA_Client[div].testCond.step[i].faultCanFreeze_use_flag);
		fprintf(fp, "faultCanFreeze_time : %d\n",
			myData->COA_Client[div].testCond.step[i].faultCanFreeze_time);
		fprintf(fp, "faultCanFreeze_CellV : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultCanFreeze_CellV);
		fprintf(fp, "faultCanFreeze_I : %ld\n",
			myData->COA_Client[div].testCond.step[i].faultCanFreeze_I);

		fprintf(fp, "vent_flag : CompAuxV(%d), CompAuxTemp(%d), CompAuxTh(%d), CompAuxT(%d), Delta_AuxV(%d)\n",
			myData->COA_Client[div].testCond.step[i].faultCompAuxV_vent_flag,
			myData->COA_Client[div].testCond.step[i].faultCompAuxTemp_vent_flag,
			myData->COA_Client[div].testCond.step[i].faultCompAuxTh_vent_flag,
			myData->COA_Client[div].testCond.step[i].faultCompAuxT_vent_flag,
			myData->COA_Client[div].testCond.step[i].faultDelta_AuxV_vent_flag);
		/*fprintf(fp, "reserved1 : %d, %d,\n",
			myData->COA_Client[div].testCond.step[i].reserved1[0],
			myData->COA_Client[div].testCond.step[i].reserved1[1]);
		fprintf(fp, "reserved2 : %ld, %ld,\n",
			myData->COA_Client[div].testCond.step[i].reserved2[0],
			myData->COA_Client[div].testCond.step[i].reserved2[1]);*/	//ktg_210105

		fprintf(fp,"\n");
	}

	fprintf(fp, "TestCond_ETC\n");
	fprintf(fp, "test_cond_file_size : %ld\n",
		myData->COA_Client[div].testCond.test_cond_file_size);
	fprintf(fp, "test_cond_file_checksum : %ld\n",
		myData->COA_Client[div].testCond.test_cond_file_checksum);

	fclose(fp);
	return 0;
}

int COA_Client_TestCond_write_V100D_pattern(int div)
{
#ifdef __COA_VER_100D__
	char fileName[256];
	int i, j, ch;
	FILE *fp;
	int k = 0;

	ch = 2;
	for(i=0; i < ch; i++) {
		memset(fileName, 0, sizeof(fileName));

		sprintf(fileName, "./COA%d_Client_conversion_V100D_pattern_ch%d", div+1,i+1);
		if((fp = fopen(fileName, "w")) == NULL) {
			printf("Can not open COA%d_Client_TestCond file(write)\n", div+1);
			return -1;
		}
		fprintf(fp, "COA%d_Client_Receive_Data_V100D_pattern\n", div+1);
			for(k=0; k < MAX_P1_AUX_FUNCTION; k++) {
				for(j=0; j < 10; j++) {
					fprintf(fp, "%02d, %d, %d, %d, %d, %f\n", k+1,
						myData->COA_Client[1].testCond.step[j]
						.reference[0].aux_func_div[k],
						myData->COA_Client[1].testCond.step[j]
						.reference[0].aux_compare_type[k],
						myData->COA_Client[1].testCond.step[j]
						.reference[0].aux_data_type[k],
						myData->COA_Client[1].testCond.step[j]
						.reference[0].aux_branch[k],
						myData->COA_Client[1].testCond.step[j]
						.reference[0].aux_value[k]);
				}
			}
		fprintf(fp,"\n");
		fclose(fp);
	}
#endif
	return 0;
}

int COA_Client_TestCond_write_TimeSch(int div)
{
//kjh_160418s
	char fileName[256];
	int i = 0;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "./COA%d_Client_TestCond_TimeSch", div+1);

	if((fp = fopen(fileName, "w")) == NULL) {
		printf("Can not open COA%d_Client_TestCond_TimeSch file(write)\n", div+1);
		return -1;
	}

	fprintf(fp, "COA%d_Client_Receive_Data_V100F_TimeSch\n", div+1);
	fprintf(fp,"\n");

	fprintf(fp, "TestCond_Header\n");
	fprintf(fp, "totalStep : %d\n",
		myData->COA_Client[div].testCond.header.totalStep);
	fprintf(fp, "totalPatternCount : %d\n",
		myData->COA_Client[div].testCond.header.totalPatternCount);
	fprintf(fp, "totalTimeSchCount : %d\n",
		myData->COA_Client[div].testCond.header.totalTimeSchCount);
	fprintf(fp, "reserved : %d\n",
		myData->COA_Client[div].testCond.header.reserved);
	fprintf(fp, "reserverd1[2] : %ld, %ld\n",
		myData->COA_Client[div].testCond.header.reserved1[0],
		myData->COA_Client[div].testCond.header.reserved1[1]);
	fprintf(fp, "\n");
/*
	fprintf(fp, "Common_Safety\n");
	fprintf(fp, "faultLowerV : %ld\n",
		myData->COA_Client[div].testCond.safety.faultLowerV);
	fprintf(fp, "faultUpperV : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperV);
	//fprintf(fp, "reserved1 : %ld\n",
	//	myData->COA_Client[div].testCond.safety.reserved1);
	fprintf(fp, "faultCompAuxV : %ld\n",
		myData->COA_Client[div].testCond.safety.faultCompAuxV);
	fprintf(fp, "faultUpperI : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperI);
	fprintf(fp, "faultLowerTemp : %ld\n",
		myData->COA_Client[div].testCond.safety.faultLowerTemp);
	fprintf(fp, "faultUpperTemp : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperTemp);
	fprintf(fp, "faultUpper_AmpareHour : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpper_AmpareHour);
	fprintf(fp, "faultUpperP : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpperP);
	fprintf(fp, "faultUpper_WattHour : %ld\n",
		myData->COA_Client[div].testCond.safety.faultUpper_WattHour);
	fprintf(fp,"\n");

	fprintf(fp, "CAN_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.can_func_div[i],
			myData->COA_Client[div].testCond.safety.can_compare_type[i],
			myData->COA_Client[div].testCond.safety.can_data_type[i],
			myData->COA_Client[div].testCond.safety.can_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "Aux_Common_Safety\n");
	fprintf(fp, "index, Division, Compare_Type, Data_Type, Value\n");
	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		fprintf(fp, "%02d, %d, %d, %d, %f\n", i+1,
			myData->COA_Client[div].testCond.safety.aux_func_div[i],
			myData->COA_Client[div].testCond.safety.aux_compare_type[i],
			myData->COA_Client[div].testCond.safety.aux_data_type[i],
			myData->COA_Client[div].testCond.safety.aux_value[i]);
	}
	fprintf(fp,"\n");

	fprintf(fp, "reserved2[4] : %ld, %ld, %ld, %ld\n",
		myData->COA_Client[div].testCond.safety.reserved2[0],
		myData->COA_Client[div].testCond.safety.reserved2[1],
		myData->COA_Client[div].testCond.safety.reserved2[2],
		myData->COA_Client[div].testCond.safety.reserved2[3]);
	fprintf(fp,"\n");
*/
	fprintf(fp, "TestCond_Step\n");
	fprintf(fp,"\n");

	for(i=0; i < myData->COA_Client[div].testCond.header.totalStep; i++) {
		fprintf(fp, "Step_Header : %d\n", i+1);
		fprintf(fp, "type : %d\n",
			myData->COA_Client[div].testCond.step[i].header.type);
		fprintf(fp, "stepNo : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].stepNo);
		fprintf(fp, "timeInit : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].timeInit);
		fprintf(fp, "reserved : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].reserved);
		fprintf(fp, "wait_day : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].wait_day);
		fprintf(fp, "wait_hour : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].wait_hour);
		fprintf(fp, "wait_min : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].wait_min);
		fprintf(fp, "wait_sec : %d\n",
			myData->COA_Client[div].testCond_time.time_step[i].wait_sec);
		fprintf(fp, "reserved2[0][1] : %ld, %ld\n",
			myData->COA_Client[div].testCond_time.time_step[i].reserved2[0],
			myData->COA_Client[div].testCond_time.time_step[i].reserved2[1]);
		fprintf(fp,"\n");
	}
/*	for(i=0; i < myData->COA_Client[div].testCond.header.totalStep; i++) {
		for(j=0;
			j < myData->COA_Client[div].testCond.header.totalTimeSchCount;
			j++) {
			if(myData->COA_Client[div].testCond_time.time_step[j].stepNo != i+1) continue;
			fprintf(fp, "Step_Header : %d\n", i+1);
			fprintf(fp, "type : %d\n",
				myData->COA_Client[div].testCond.step[i].header.type);
			fprintf(fp, "stepNo : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].stepNo);
			fprintf(fp, "timeInit : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].timeInit);
			fprintf(fp, "reserved : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].reserved);
			fprintf(fp, "wait_day : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].wait_day);
			fprintf(fp, "wait_hour : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].wait_hour);
			fprintf(fp, "wait_min : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].wait_min);
			fprintf(fp, "wait_sec : %d\n",
				myData->COA_Client[div].testCond_time.time_step[j].wait_sec);
			fprintf(fp, "reserved2[0][1] : %ld, %ld\n",
				myData->COA_Client[div].testCond_time.time_step[i].reserved2[0],
				myData->COA_Client[div].testCond_time.time_step[i].reserved2[1]);
			fprintf(fp,"\n");
		}
	}
	*/
	fprintf(fp, "TestCond_ETC\n");
	fprintf(fp, "test_cond_file_size : %ld\n",
		myData->COA_Client[div].testCond_time.test_cond_time_file_size);
	fprintf(fp, "test_cond_file_checksum : %ld\n",
		myData->COA_Client[div].testCond_time.test_cond_time_file_checksum);

	fclose(fp);
	return 0;
//kjh_160418e
}

int write_IO_Print(int k)
{
	struct	tm *tm;
	char	*asciitime, fileName[256];
	time_t  curtime;
	int		i,j= 0;
	FILE	*fp;

	time(&curtime);
	asciitime = ctime(&curtime);
	asciitime[24] = 0;
	tm = localtime(&curtime);

	memset(fileName, 0, sizeof(fileName));

	sprintf(fileName, "./write_IO_Print.csv");

	fp = fopen(fileName, "a");
	if(fp == NULL || fp < 0) {
		printf("Can not open write_IO_Print\n");
		return -1;
	}

	fprintf(fp, "time %02d%02d %s\n", tm->tm_mon+1, tm->tm_mday, asciitime);

	fprintf(fp, "in_address\n");
	for(i=0; i < MAX_DIO_IN_BYTES; i++) {
		fprintf(fp, "%x ", myData->dio.function_set.in_address[i]);
	}
	fprintf(fp, "\n");

	fprintf(fp, "in bytes\n");
	for(i=0; i < 4; i++) {
		for(j=0; j < 8; j++) {
			fprintf(fp, "%03x:", myData->dio.function_set.in_address[i*8+j]);
			fprintf(fp, "%02x ", myData->dio.in.bytes[i*8+j]);
		}
		fprintf(fp, "\n");
	}
	fprintf(fp, "\n");

	fprintf(fp, "out_address\n");
	for(i=0; i < MAX_DIO_OUT_BYTES; i++) {
		fprintf(fp, "%x ", myData->dio.function_set.out_address[i]);
	}
	fprintf(fp, "\n");
	fflush(fp);
	fclose(fp);
	return 0;
}

int KeyInput(void)
{
	int i, rtn;

	rtn = 0;

	i = getc(stdin);
	switch(i) {
		case '1':
			bd = 0;
			for(i=0; i < 10; i++) {
				min[i] = 0;
				max[i] = 0;
			}
			break;
		case '2':
			bd = 1;
			for(i=0; i < 10; i++) {
				min[i] = 0;
				max[i] = 0;
			}
			break;
		//case '3':
		//	bd = 2;
		//	for(i=0; i < 10; i++) {
		//		min[i] = 0;
		//		max[i] = 0;
		//	}
		//	break;
		case '3':
			//bd = 3;
			//for(i=0; i < 10; i++) {
			//	min[i] = 0;
			//	max[i] = 0;
			//}
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 3;
			break;
		case '4':
			//myData->mData.signal[M_SIG_BMS_EOL_PHASE] = P1;
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 4;
			break;
		case '5':
			//myData->mData.signal[M_SIG_BMS_EOL_PHASE] = P100;
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 5;
			break;
		case '6':
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] = P100;
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 6;
			break;
		case '7':
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] = P1;
			//memset((char *)&myData->COA_Client[0].misc.write_pack_id[0], 0, 40);
			//strcpy((char *)&myData->COA_Client[0].misc.write_pack_id[0],
			//	"KJG_test_120130");
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 7;
			break;
		case '8':
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] = P0;
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 2;
			break;
		case '9':
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] = P0;
			//myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE2] = P0;
			myData->cData[0].signal[C_SIG_CMD_GOTO_STEP] = P1;
			myData->cData[0].misc.user_branch_stepNo = 10;
			break;
		case 'a':
			//myData->cData[0].op.runTime += (360000 * 24 * 5); //5Day
			//myData->cData[0].op.totalRunTime += (360000 * 24 * 5); //5Day
			//myData->cData[0].op.runTime += (360000); //1h
			//myData->cData[0].op.totalRunTime += (360000); //1h
			myData->test_val_l1[0] = 1;
			myData->test_val_l1[1] = 1;
			myData->test_val_l1[2] = 1;
			myData->test_val_l1[3] = 1;
			myData->test_val_l1[4] = 1;
			myData->test_val_l1[5] = 1;
			myData->test_val_l1[6] = 1;
			myData->test_val_l1[7] = 1;
			break;
		case 'b':
			myData->test_val_l1[0] = 3;
			break;
		case 'c':
			myData->test_val_l1[0] = 0;
			break;
		/*case 'b': //kjg_140916_s
			myData->test_val_l1[0]++;
			switch(myData->test_val_l1[0]) {
				case 1:
					myData->test_val_l1[1] = 0; //fault_clear 0:idle, 1:clear
					myData->test_val_l1[2] = 0; //e48_stat_error 0:idle, x:fault
					myData->test_val_l1[3] = 0; //e48_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[4] = 1; //pc_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[5] = 0; //rp_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[6] = 15000; //charge_v
					myData->test_val_l1[7] = 100; //discharge_v
					myData->test_val_l1[8] = 2; //set_mode 2:cc, 6:cp
					myData->test_val_l1[9] = 0; //cmd_i
					myData->test_val_l1[10] = 0; //cmd_p
					myData->test_val_l1[11] = 0; //cmd_t
					myData->test_val_l1[12] = 0;
						//set_control 0:idle, 1:run, 2:pause, 3:continue, 4:end
					myData->test_val_l1[13] = 100; //receive_parsing_period
					myData->test_val_l1[14] = 10; //can_transmit_period
					break;
				case 2:
					myData->test_val_l1[1] = 0; //fault_clear 0:idle, 1:clear
					myData->test_val_l1[2] = 0; //e48_stat_error 0:idle, x:fault
					myData->test_val_l1[3] = 0; //e48_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[4] = 1; //pc_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[5] = 0; //rp_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[6] = 15000; //charge_v
					myData->test_val_l1[7] = 100; //discharge_v
					myData->test_val_l1[8] = 2; //set_mode 2:cc, 6:cp
					myData->test_val_l1[9] = 0; //cmd_i
					myData->test_val_l1[10] = 0; //cmd_p
					myData->test_val_l1[11] = 0; //cmd_t
					myData->test_val_l1[12] = 1;
						//set_control 0:idle, 1:run, 2:pause, 3:continue, 4:end
					myData->test_val_l1[13] = 100; //receive_parsing_period
					myData->test_val_l1[14] = 10; //can_transmit_period
					break;
				case 3:
					myData->test_val_l1[1] = 0; //fault_clear 0:idle, 1:clear
					myData->test_val_l1[2] = 0; //e48_stat_error 0:idle, x:fault
					myData->test_val_l1[3] = 0; //e48_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[4] = 1; //pc_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[5] = 0; //rp_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[6] = 15000; //charge_v
					myData->test_val_l1[7] = 100; //discharge_v
					myData->test_val_l1[8] = 2; //set_mode 2:cc, 6:cp
					myData->test_val_l1[9] = 1000; //cmd_i
					myData->test_val_l1[10] = 0; //cmd_p
					myData->test_val_l1[11] = 0; //cmd_t
					myData->test_val_l1[12] = 1;
						//set_control 0:idle, 1:run, 2:pause, 3:continue, 4:end
					myData->test_val_l1[13] = 100; //receive_parsing_period
					myData->test_val_l1[14] = 10; //can_transmit_period
					break;
				case 4:
					myData->test_val_l1[1] = 0; //fault_clear 0:idle, 1:clear
					myData->test_val_l1[2] = 0; //e48_stat_error 0:idle, x:fault
					myData->test_val_l1[3] = 0; //e48_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[4] = 1; //pc_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[5] = 0; //rp_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[6] = 15000; //charge_v
					myData->test_val_l1[7] = 100; //discharge_v
					myData->test_val_l1[8] = 2; //set_mode 2:cc, 6:cp
					myData->test_val_l1[9] = 0; //cmd_i
					myData->test_val_l1[10] = 0; //cmd_p
					myData->test_val_l1[11] = 0; //cmd_t
					myData->test_val_l1[12] = 1;
						//set_control 0:idle, 1:run, 2:pause, 3:continue, 4:end
					myData->test_val_l1[13] = 100; //receive_parsing_period
					myData->test_val_l1[14] = 10; //can_transmit_period
					break;
				case 5:
					myData->test_val_l1[1] = 0; //fault_clear 0:idle, 1:clear
					myData->test_val_l1[2] = 0; //e48_stat_error 0:idle, x:fault
					myData->test_val_l1[3] = 0; //e48_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[4] = 1; //pc_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[5] = 0; //rp_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[6] = 15000; //charge_v
					myData->test_val_l1[7] = 100; //discharge_v
					myData->test_val_l1[8] = 2; //set_mode 2:cc, 6:cp
					myData->test_val_l1[9] = 0; //cmd_i
					myData->test_val_l1[10] = 0; //cmd_p
					myData->test_val_l1[11] = 0; //cmd_t
					myData->test_val_l1[12] = 4;
						//set_control 0:idle, 1:run, 2:pause, 3:continue, 4:end
					myData->test_val_l1[13] = 100; //receive_parsing_period
					myData->test_val_l1[14] = 10; //can_transmit_period
					break;
				case 6:
					myData->test_val_l1[1] = 0; //fault_clear 0:idle, 1:clear
					myData->test_val_l1[2] = 0; //e48_stat_error 0:idle, x:fault
					myData->test_val_l1[3] = 0; //e48_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[4] = 1; //pc_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[5] = 0; //rp_nm 0:idle, 1:run, 2:req
					myData->test_val_l1[6] = 15000; //charge_v
					myData->test_val_l1[7] = 100; //discharge_v
					myData->test_val_l1[8] = 2; //set_mode 2:cc, 6:cp
					myData->test_val_l1[9] = 0; //cmd_i
					myData->test_val_l1[10] = 0; //cmd_p
					myData->test_val_l1[11] = 0; //cmd_t
					myData->test_val_l1[12] = 0;
						//set_control 0:idle, 1:run, 2:pause, 3:continue, 4:end
					myData->test_val_l1[13] = 100; //receive_parsing_period
					myData->test_val_l1[14] = 10; //can_transmit_period
					break;
				default:
					break;
			}
			break;
		case 'c':
			for(i=0; i < MAX_TEST_VALUE; i++) myData->test_val_l1[i] = 0;
			break; //kjg_140916_e
			*/
		case 'd':
			myData->cData[0].signal[C_SIG_CMD_CHAMBER_CONTINUE] = P1;
			break;
		case 'e':
			myData->cData[1].signal[C_SIG_CMD_CHAMBER_CONTINUE] = P1;
			break;
		case 'q':
			rtn = (-1);
			break;
		case 's':
			for(i=0; i < MAX_TEST_VALUE; i++) {
				myData->test_val_i[0][i] = 0;
				myData->test_val_i[1][i] = 0;
				myData->test_val_i[2][i] = 0;
				myData->test_val_i[3][i] = 0;
				myData->test_val_i[4][i] = 0;
				myData->test_val_i[5][i] = 0;
				myData->test_val_i[6][i] = 0;
				myData->test_val_i[7][i] = 0;
			}
			//myData->mData.signal[M_SIG_EXIT_PHASE] = P3;
			//myData->mData.signal[M_SIG_BMS_EOL3_PHASE] = P1;
			break;
		//case 't': myData->CAN.signal[0][CAN_SIG_ECU_ID_REQUEST] = P1; break;
		/*case 't':	//ktg_220331s
			myData->test_val_l1[6] = 1;
			break;
		case 'f':
			myData->test_val_l1[6] = 2;
			break;	//ktg_220331e*/
		case 't': file_write = 1; break;
		case 'f': file_write = 2; break; //TestCond_write_V1009(0);
		case 'F': file_write = 3; break; //TestCond_write_V100B(0);
		case 'G': file_write = 4; break; //TestCond_write_V100C(0);
		case 'H': file_write = 5; break; //TestCond_write_V100D(0);
		case 'K': file_write = 6; break; //TestCond_write_V100D_Pattern(0);
		case 'J': file_write = 7; break; //write_IO_Print(0);
		case 'r':
			myData->cData[0].op.state = C_RUN;
			myData->cData[0].op.stepType = STEP_END;
			myData->cData[0].op.phase = P102;
			break;
		case 'k':
			//myData->CAN.signal[0][CAN_SIG_TX_PHASE_FLASH_CMD] = P0;
			//myData->mData.signal[M_SIG_BMS_EOL3_PHASE] = P10;
			myData->mData.code = M_CD_FAULT_NORMAL_TERMINAL_QUIT;
			myData->mData.signal[M_SIG_EXIT_PHASE] = P1;
			myData->mData.signal[M_SIG_EXIT_TYPE] = P1;
			break;
		case 'l':
			myData->test_val_l1[0]++;
			switch(myData->test_val_l1[0]) {
				case 1:
					myData->test_val_l1[1] = 1000;
					break;
				case 2:
					myData->test_val_l1[1] = 900;
					break;
				case 3:
					myData->test_val_l1[1] = 800;
					break;
				case 4:
					myData->test_val_l1[1] = 700;
					break;
				case 5:
					myData->test_val_l1[1] = 600;
					break;
				case 6:
					myData->test_val_l1[1] = 500;
					break;
				case 7:
					myData->test_val_l1[1] = 400;
					break;
				case 8:
					myData->test_val_l1[1] = 300;
					break;
				case 9:
					myData->test_val_l1[1] = 200;
					break;
				case 10:
					myData->test_val_l1[1] = 100;
					break;
				default:
					myData->test_val_l1[0] = 0;
					myData->test_val_l1[1] = 0;
					break;
			}
			break;
			/*
		case 'l':
			myData->test_val_l1[0]++;
			switch(myData->test_val_l1[0]) {
				case 1:
					myData->test_val_l1[1] = 5000000;
					break;
				case 2:
					myData->test_val_l1[1] = 15000000;
					break;
				case 3:
					myData->test_val_l1[1] = 25000000;
					break;
				case 4:
					myData->test_val_l1[1] = 35000000;
					break;
				case 5:
					myData->test_val_l1[1] = 45000000;
					break;
				case 6:
					myData->test_val_l1[1] = 55000000;
					break;
				case 7:
					myData->test_val_l1[1] = 65000000;
					break;
				case 8:
					myData->test_val_l1[1] = 75000000;
					break;
				case 9:
					myData->test_val_l1[1] = 85000000;
					break;
				case 10:
					myData->test_val_l1[1] = 95000000;
					break;
				case 11:
					myData->test_val_l1[1] = 105000000;
					break;
				default:
					myData->test_val_l1[0] = 0;
					myData->test_val_l1[1] = 0;
					break;
			}
			break;
			*/
		case 'o':
			for(i=0; i < 100; i++) {
				myData->mData.runningTime[0][i] = 0;
				myData->mData.runningTime[1][i] = 0;
				myData->mData.runningTime[2][i] = 0;
				myData->mData.runningTime[3][i] = 0;
				myData->mData.runningTime[4][i] = 0;
				myData->mData.runningTime[5][i] = 0;
			}
			for(i=0; i < 16; i++) {
				myData->test_val_i[0][i] = 0;
				myData->test_val_i[1][i] = 0;
				myData->test_val_i[2][i] = 0;
				myData->test_val_i[3][i] = 0;
				myData->test_val_i[4][i] = 0;
				myData->test_val_i[5][i] = 0;
				myData->test_val_i[6][i] = 0;
				myData->test_val_i[7][i] = 0;
				myData->test_val_ll1[i] = 0;
				myData->test_val_ll2[0][i] = 0;
				myData->test_val_ll2[1][i] = 0;
				myData->test_val_ll2[2][i] = 0;
				myData->test_val_ll2[3][i] = 0;
				myData->test_val_ll2[4][i] = 0;
				myData->test_val_ll2[5][i] = 0;
				myData->test_val_ll2[6][i] = 0;
				myData->test_val_ll2[7][i] = 0;
			}

			//myData->mData.runningTime[2][11] = 0;
			break;
		case 'O':
			myData->test_val_ll1[15] = 1;
			myData->test_val_ll1[14] = 0;
			myData->test_val_ll1[13] = 3;
			myData->test_val_ll1[12] = 1;
			break;
		case 'p':
			myData->test_val_ll1[14] = 0;
			myData->test_val_i[1][15] = 1;
			myData->test_val_i[2][15] = 1;
			myData->test_val_i[3][15] = 1;
			myData->test_val_i[4][15] = 1;
			myData->test_val_i[5][15] = 1;
			myData->test_val_i[6][15] = 1;
			myData->test_val_i[7][15] = 1;
			break;
		case 'P':
			myData->test_val_ll1[12] = 0;
			break;
		case 'u':	//kjhw_181111
			//myData->cData[0].misc.maxAuxTemp = 1234;
			myData->COM.com_port[AUX_TEMP].value[3] = 11000;
			break;
		case 'U':	//kjhw_181111
			//myData->cData[0].misc.maxAuxTemp = 1234;
			myData->COM.com_port[AUX_TEMP].value[3] = 80000;
			break;
		case 'w':	//reset
			myData->test_val_i[1][0] = 0;	//Vsens = 50V
			myData->test_val_i[1][1] = 0;	//Isens = 100A
			break;
		case 'W':	//Charge_CC_test
			myData->test_val_i[1][0] = 1;	//Vsens = 100mV/sec ++
			myData->test_val_i[1][1] = 0;	//Isens = 100A
			break;
		case 'x':	//Charge_CV_test
			myData->test_val_i[1][0] = 3;	//Vsens = stop
			myData->test_val_i[1][1] = 2;	//Isens = 100mA/sec --
			break;
		case 'X':	//DisCharge CC_test
			myData->test_val_i[1][0] = 2;	//Vsens = 100mV/sec --
			myData->test_val_i[1][1] = 0;	//Isens = -100A
			break;
		case 'y':	//DisCharge CV_test
			myData->test_val_i[1][0] = 3;	//Vsens = stop
			myData->test_val_i[1][1] = 1;	//Isens = 100mA/sec ++
			break;
		default: break;
	}
	return rtn;
}

int main(void)
{
    int	retval, rtn, i;
    struct timeval tv;
    fd_set rfds;

	if(Open_SystemMemory(0) < 0) return -1;

	bd = 0;

	for(i=0; i < 10; i++) {
		min[i] = 0;
		max[i] = 0;
	}

    while(1) {
	    tv.tv_sec = 0;
	    //tv.tv_usec = 800000;
	    tv.tv_usec = 995500;
	    FD_ZERO(&rfds);
	    FD_SET(0, &rfds);
		retval = select(1, &rfds, NULL, NULL, &tv);
	    if(retval == 0) {
			Test_Print();
		} else {
			printf("rtn %d\n", retval);
			rtn = KeyInput();
			if(rtn < 0) break;
		}
    }

	Close_SystemMemory();
    return 0;
}
