#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <asm/io.h>
#include "../../INC/datastore.h"
#include "main.h"

void Test_Print(void)
{
	int i;

	i = 1; if(i == 1) DataStructureSize_Print();
}

void DataStructureSize_Print(void)
{
	int i, j, size[100];

	memset(size, 0, sizeof size);
	i = j = 0;
	size[i] = sizeof(S_SYSTEM_DATA);
    printf("SysData size %d\n", size[i]);

	i++;
	size[i] = sizeof(S_MSG) * MAX_MSG_RING;
	printf("msg size %d = %d * %d\n", size[i], sizeof(S_MSG), MAX_MSG_RING);
	i++;
	size[i] = sizeof(S_SAVE_MSG) * MAX_SAVE_MSG_RING;
	printf("save_msg size %d = %d * %d\n", size[i], sizeof(S_SAVE_MSG),
		MAX_SAVE_MSG_RING);

	i++;
	size[i] = sizeof(S_PULSE_MSG_1) * MAX_PULSE_MSG_RING;
	printf("pulse_msg_1 size %d = %d * %d\n", size[i], sizeof(S_PULSE_MSG_1),
		MAX_PULSE_MSG_RING);

#ifdef __COB__
	i++;
	size[i] = sizeof(S_F_SAVE_MSG) * MAX_F_SAVE_MSG_RING;
	printf("f_save_msg size %d = %d * %d\n", size[i], sizeof(S_F_SAVE_MSG),
		MAX_F_SAVE_MSG_RING);
#endif

	i++;
	size[i] = sizeof(S_LOGFILE) * MAX_LOG;
	printf("log size %d = %d * %d\n", size[i], sizeof(S_LOGFILE), MAX_LOG);

	i++;
	size[i] = sizeof(S_CALIBRATION) * MAX_CH_8;
	printf("cali size %d = %d * %d\n", size[i], sizeof(S_CALIBRATION),
		MAX_CH_8);

#ifdef __COB__
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
		MAX_TEST_COND_COUNT);
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
	size[i] = sizeof(S_TEST_CONDITION);
	printf("testCond_update size %d\n", size[i]);

	i++;
	size[i] = sizeof(S_TIMESCH_CONTROL);
	printf("size TimeSch %d\n", size[i], sizeof(S_TIMESCH_CONTROL));

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
	printf("	pulse_data %d = %d * %d\n", sizeof(S_P1_PULSE_DATA) * MAX_CH_8,
		sizeof(S_P1_PULSE_DATA), MAX_CH_8);

#ifdef __COB__
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

#if defined __COC__ //kjg_171223
	i++;
	size[i] = sizeof(S_COC_CLIENT) * MAX_GROUP_1;
	printf("size COC_Client %d = %d * %d\n", size[i], sizeof(S_COC_CLIENT),
		MAX_GROUP_1);
#endif

#if defined __COD__ //kjg_171223
	i++;
	size[i] = sizeof(S_COC_CLIENT) * MAX_GROUP_1; //kjgw
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

	i++;
	size[i] = sizeof(S_BD_DATA) * MAX_BD_16;
	printf("size bdData %d = %d * %d\n", size[i], sizeof(S_BD_DATA), MAX_BD_16);

	i++;
	size[i] = sizeof(S_CH_DATA) * MAX_CH_256;
	printf("size cData %d = %d * %d\n", size[i], sizeof(S_CH_DATA), MAX_CH_256);

	i++;
	size[i] = sizeof(unsigned short int) * MAX_CH_8 * MAX_AUX_DATA * MAX_AUX_FUNCTION;
	printf("size local_codeCount_aux %d\n", size[i]);

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

#ifdef __COB__
	i++;
	size[i] = sizeof(int) * MAX_GROUP_2 * MAX_CH_256;
	printf("size COB_save_step %d = %d * %d * %d\n", size[i], sizeof(int),
		MAX_GROUP_2, MAX_CH_256);
	i++;
	size[i] = sizeof(S_CH_OP_DATA) * MAX_GROUP_2 *  MAX_CH_256 * MAX_P2_STEP;
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
#ifdef __COB__
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
	size[i] = sizeof(U_CAN_VAL) * MAX_CH_8 * MAX_CAN_DATA * 12;
	printf("size tmpCanData %d = %d * %d * %d * %d\n", size[i],
		sizeof(U_CAN_VAL), MAX_CH_8, MAX_CAN_DATA, 12);
	i++;
	size[i] = sizeof(U_CAN_VAL) * MAX_CH_8 * MAX_CAN_DATA * 12;
	printf("size tmpCanData_sort %d = %d * %d * %d * %d\n", size[i],
		sizeof(U_CAN_VAL), MAX_CH_8, MAX_CAN_DATA, 12);
	i++;
	size[i] = sizeof(unsigned char) * MAX_CH_8 * MAX_CAN_DATA * 12;
	printf("size tmpCanData_sort_flag %d = %d * %d * %d * %d\n", size[i],
		sizeof(U_CAN_VAL), MAX_CH_8, MAX_CAN_DATA, 12);
	i++;
	size[i] = sizeof(unsigned char) * MAX_CH_8;
	printf("size tmpCanData_sort_count %d = %d * %d\n", size[i],
		sizeof(U_CAN_VAL), MAX_CH_8);
	i++;
	size[i] = sizeof(unsigned char) * MAX_CH_8 * MAX_CAN_DATA;
	printf("size tmpCanData_sort_index %d = %d * %d * %d\n", size[i],
		sizeof(U_CAN_VAL), MAX_CH_8, MAX_CAN_DATA);
	i++;
	size[i] = sizeof(unsigned char) * MAX_CH_8 * MAX_CAN_DATA;
	printf("size tmpCanData_sort_count %d = %d * %d * %d\n", size[i],
		sizeof(U_CAN_VAL), MAX_CH_8, MAX_CAN_DATA);

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
	size[i] = sizeof(int) * 8 * MAX_TEST_VALUE;
	printf("size test_val_i %d = %d * %d\n", size[i], sizeof(int),
		8 * MAX_TEST_VALUE);
	i++;
	size[i] = sizeof(long) * MAX_TEST_VALUE;
	printf("size test_val_l %d = %d * %d\n", size[i], sizeof(long),
		MAX_TEST_VALUE);
	i++;
	size[i] = sizeof(long long) * MAX_TEST_VALUE;
	printf("size test_val_ll %d = %d * %d\n", size[i], sizeof(long long),
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
	size[i] = sizeof(long) * 2;
	printf("size delayCount %d = %d * %d\n", size[i], sizeof(long), 2);

	i++;
	size[i] = sizeof(long) * 4;
	printf("size compV %d = %d * %d\n", size[i], sizeof(long), 4);

	i++;
	j = i;
	for(i=1; i < j; i++) {
		size[j] += size[i];
	}
	printf("total %d : %d\n", size[j], size[0]);

	printf("\n");
}

int main(void)
{
	Test_Print();

    exit(0);
}

