#ifndef __P2_CLIENT_DEF_H__
#define __P2_CLIENT_DEF_H__

//P2 client define
#define P2_MAX_CH_PER_MODULE				256
#define P2_MAX_CH_PER_GROUP					256
#define P2_MAX_CH_PER_BD					16

//protocol
#define P2_PROTOCOL_VERSION					0x1001
#define P2_PROTOCOL_VERSION_2				0x1002
	//090728 test_precheck2, test_common_safety, S_P2_CH_DATA2

//test condition
#define MAX_P2_STEP							210
#define MAX_P2_GRADE						20
#define MAX_P2_SENSOR_CH					64

//packet
#define	MAX_P2_SEND_PACKET_LENGTH			(1024*10)
//#define	MAX_P2_RECV_PACKET_LENGTH			(1024*10)
#define	MAX_P2_RECV_PACKET_LENGTH			(1024*3072) //kjg_111006 10k->3M
#define MAX_P2_RECV_PACKET_COUNT			64

//packet id define
#define PACKET_P2_FORMATION					1
#define PACKET_P2_HOST						2
#define PACKET_P2_IR_OCV					3
#define PACKET_P2_OCV						4
#define PACKET_P2_AGING						5
#define PACKET_P2_CYCLER					6

//command define
#define MAX_P2_CMD_SERIAL					100000000

//command list
#define P2_CMD_TO_SBC_COMM_CHECK			0
#define P2_CMD_TO_SBC_COMM_RESPONSE			1
#define P2_CMD_TO_SBC_RESPONSE				0x00000001	//1d
#define P2_CMD_TO_SBC_CHECK					0x00000010	//16d
#define P2_CMD_TO_SBC_RUN					0x00000011	//17d
#define P2_CMD_TO_SBC_PAUSE					0x00000012	//18d
#define P2_CMD_TO_SBC_CONTINUE				0x00000013	//19d
#define P2_CMD_TO_SBC_STOP					0x00000014	//20d
#define P2_CMD_TO_SBC_RESET					0x00000015	//21d
#define P2_CMD_TO_SBC_NEXT_STEP				0x00000016	//22d
#define P2_CMD_TO_SBC_CLEAR					0x00000019	//25d
#define P2_CMD_TO_SBC_TEST_INFO_REQUEST		0x00001005	//4101d
#define P2_CMD_TO_SBC_SET_TRAY_READY		0x00001009
#define P2_CMD_TO_SBC_SET_SENSOR_LIMIT		0x0000100D	//4109d
#define P2_CMD_TO_SBC_TRAY_DATA				0x0000100E	//4110d
#define P2_CMD_TO_SBC_USER_CMD				0x00001011
#define P2_CMD_TO_SBC_SET_AUTO_REPORT		0x00001012	//4114d
#define P2_CMD_TO_SBC_GET_DIGITAL_IN		0x00001013	//4115d kjg_w_s
#define P2_CMD_TO_SBC_SET_DIGITAL_OUT		0x00001014	//4116d
#define P2_CMD_TO_SBC_GET_REF_IC_DATA		0x00001015	//4117d
#define P2_CMD_TO_SBC_HOST_TIME				0x00011016	//4118d
#define P2_CMD_TO_SBC_CONFIG_REQUEST		0x00001017	//4119d kjg_w_e
#define P2_CMD_TO_SBC_SET_CONFIG			0x00001018	//4120d
#define P2_CMD_SET_USE_STKCRANE				0x0000101A	//4122d
#define P2_CMD_TO_SBC_CHAMBER_SET			0x0000101B	//4123d
#define P2_CMD_TO_SBC_JOB_CHANGE_SET		0x0000101C

#define P2_CMD_TO_SBC_JIG_TEMP_SET_DATA		0x00001020 //kjg_110708

#define P2_CMD_TO_SBC_TEST_STEP				0x00002003	//8195d
#define P2_CMD_TO_SBC_TEST_GRADE			0x00002004	//8196d
#define P2_CMD_TO_SBC_TEST_HEADER			0x00002005	//8197d
#define P2_CMD_TO_SBC_TEST_PRECHECK			0x00002006	//8198d
#define P2_CMD_TO_SBC_TEST_COMMON_SAFETY	0x00002007	//8199d
#define P2_CMD_TO_SBC_TEST_NG_COND			0x00002008	//8200d
#define P2_CMD_TO_SBC_VERSION_REQUEST		0x00002010	//8208d
#define P2_CMD_TO_SBC_SET_GROUP				0x00002013	//8211d
#define P2_CMD_TO_SBC_SET_LINE_MODE			0x00002015	//8213d
#define P2_CMD_TO_SBC_LINE_MODE_REQUEST		0x00002016	//8214d	
#define P2_CMD_TO_SBC_HW_MAP_REQ			0x00002017 //kjg_110708
#define P2_CMD_TO_SBC_CH_ATTRIBUTE_SET		0x00002019
#define P2_CMD_TO_SBC_VERSION_REQUEST2		0x0000201A

#define P2_CMD_TO_SBC_SET_CALI_POINT		0x00003001 //kjg_110708_s
#define P2_CMD_TO_SBC_GET_MAIN_CALI_POINT	0x00003002
#define P2_CMD_TO_SBC_CALI_STOP				0x00003003
#define P2_CMD_TO_SBC_CALI_PAUSE			0x00003004
#define P2_CMD_TO_SBC_CALI_RESUME			0x00003005
#define P2_CMD_TO_SBC_CALI_START			0x00003006
#define P2_CMD_TO_SBC_CALI_CHECK_START		0x00003007
#define P2_CMD_TO_SBC_CALI_UPDATE			0x0000300C
#define P2_CMD_TO_SBC_GET_CH_CALI_POINT		0x0000300D

#define P2_CMD_TO_SBC_REAL_MEASURE_START	0x00003101
#define P2_CMD_TO_SBC_BUZZER_OFF			0x00003102
#define P2_CMD_TO_SBC_REAL_MEASURE_STOP		0x00003103 //kjg_110708_e

#define P2_CMD_TO_SBC_SET_NVRAM				118
#define P2_CMD_TO_SBC_SET_TRAY_SERIAL		119
#define P2_CMD_TO_SBC_INIT_NVRAM			120
#define P2_CMD_TO_SBC_TRAY_SERIAL_REQUEST	123
//#define P2_CMD_TO_SBC_SET_LINE_MODE			124
#define P2_CMD_TO_SBC_REF_DATA_REQUEST		127
#define P2_CMD_TO_SBC_REF_DATA_UPDATE		128

#define P2_CMD_TO_PC_RESPONSE				0x00000001	//1d
#define P2_CMD_TO_PC_CH_DATA				0x00000004	//4d
#define P2_CMD_TO_PC_STEP_DATA				0x00000005	//5d
#define P2_CMD_TO_PC_GROUP_STATE			0x00000006	//6d
#define P2_CMD_TO_PC_SENSOR_DATA			0x00000007	//7d
#define P2_CMD_TO_PC_TROUBLE_CODE			0x00000008	//8d
#define P2_CMD_TO_PC_SWITCH_DATA			0x0000000B	//11 100109
#define P2_CMD_TO_PC_CHAMBER_DATA			0x0000000C	//12d
#define P2_CMD_TO_PC_TEST_INFO_DATA			0x00011005	//69637d
#define P2_CMD_TO_PC_BCR_INFO				0x00001006
#define P2_CMD_TO_PC_CHECK_DATA				0x00001008	//4104d
#define P2_CMD_TO_PC_REAL_TIME_DATA			0x00001019

#define P2_CMD_TO_PC_USER_CMD				0x00011011
#define P2_CMD_TO_PC_DIGITAL_IN_DATA		0x00011013	//kjg_w_s
#define P2_CMD_TO_PC_REF_IC_DATA			0x00011015
#define P2_CMD_TO_PC_HOST_TIME_REQUEST		0x00001016	//kjg_w_e
#define P2_CMD_TO_PC_CONFIG_DATA			0x00011017	//69655d
#define P2_CMD_TO_PC_VERSION_DATA			0x00012010	//73744d
#define P2_CMD_TO_PC_LINE_MODE_DATA			0x00012016	//73750d
#define P2_CMD_TO_PC_HW_MAP_REQ_RPY			0x00012017 //kjg_110708
#define P2_CMD_TO_PC_CH_ATTRIBUTE_SET_REPLY	0x00012019
#define P2_CMD_TO_PC_VERSION_DATA2			0x0001201A

#define P2_CMD_TO_PC_TEST_STEP				0x00002003 //kjg_110713_s
#define P2_CMD_TO_PC_TEST_GRADE				0x00002004
#define P2_CMD_TO_PC_TEST_HEADER			0x00002005
#define P2_CMD_TO_PC_TEST_PRECHECK			0x00002006 //kjg_110713_e

#define P2_CMD_TO_PC_ALL_NG					0x00002020

#define P2_CMD_TO_PC_GET_MAIN_CALI_POINT_REPLY	0x00013002 //kjg_110708_s
#define P2_CMD_TO_PC_CALI_DATA				0x00013008
#define P2_CMD_TO_PC_CALI_CHECK_DATA		0x00013009
#define P2_CMD_TO_PC_CALI_END				0x0001300A
#define P2_CMD_TO_PC_CALI_CHECK_END			0x0001300B
#define P2_CMD_TO_PC_GET_CH_CALI_POINT_REPLY	0x0001300D

#define P2_CMD_TO_PC_REAL_MEASURE_DATA		0x00003102
#define P2_CMD_TO_PC_REAL_MEASURE_END		0x00003104 //kjg_110708_e

#define P2_CMD_TO_PC_METER_CONNECT_REPLY	205
#define P2_CMD_TO_PC_COMM_CHECK_REPLY		207
#define P2_CMD_TO_PC_COMM_CHECK				208
#define P2_CMD_TO_PC_SET_REPLY				209

//command error
#define P2_CMD_ID_ERROR						1
#define P2_SIZE_ERROR						2
#define P2_CHECK_SUM_ERROR					3
#define P2_BODY_SIZE_ERROR					4
#define P2_DIRECTION_ERROR					5
#define P2_BOX_ID_ERROR						6
#define P2_OBJECT_ID_ERROR					7
#define P2_PROCESS_ID_ERROR					8
#define P2_ETC_ERROR						9

//command response code
#define P2_CD_NACK							0x0000
#define P2_CD_ACK							0x0001
#define P2_CD_TIMEOUT						0x0002
#define P2_CD_SIZE_ERROR					0x0003
#define P2_CD_RX_BUF_OVERFLOW				0x0004
#define P2_CD_TX_BUF_OVERFLOW				0x0005
#define P2_CD_UNKNOWN_CMD					0x0006
#define P2_CD_UNKNOWN_CODE					0x0007
#define P2_CD_PACKET_ID_ERROR				0x0008
#define P2_CD_GP_ID_ERROR					0x0009
#define P2_CD_SEQ_NO_ERROR					0x000A

//group
#define P2_CD_GP_WORK_TYPE_ERROR			0x0100
#define P2_CD_GP_STATE_ERROR				0x0101
#define P2_CD_GP_TOTAL_CH_ERROR				0x0102
#define P2_CD_GP_CH_ALL_FAULT				0x0103
#define P2_CD_GP_UNKNOWN_STEP_TYPE			0x0104
#define P2_CD_GP_TEST_HEADER_UNRCV			0x0105
#define P2_CD_GP_TEST_PRECHECK_UNRCV		0x0106
#define P2_CD_GP_TEST_STEP_UNRCV			0x0107
#define P2_CD_GP_TEST_STEP_COUNT_ERROR		0x0108
#define P2_CD_GP_TEST_STEP_TYPE_ERROR		0x0109
#define P2_CD_GP_TEST_STEP_DATA_ERROR		0x010A
#define P2_CD_GP_RANGE_ERROR				0x010B
#define P2_CD_GP_COMPFLAG_IS_DISABLE		0x010C
#define P2_CD_GP_CALI_RANGE_MISMATCH		0x010D
#define P2_CD_GP_STATE_ERROR2				0x010E
#define P2_CD_GP_SAVED_FILE_DELETE_ERROR	0x010F
#define P2_CD_GP_TEST_TOTAL_GRADE_ERROR		0x0110
#define P2_CD_GP_TEST_COMMON_SAFETY_UNRCV	0x0111
#define P2_CD_GP_TEST_NG_COND_UNRCV			0x0112
#define P2_CD_GP_NETWORK_COMM				0x0124
#define P2_CD_GP_DATA_SAVE					0x012A

//channel
#define P2_CD_CH_ISNT_RUN					0x0200
#define P2_CD_CH_STATE_ERROR				0x0201
#define P2_CD_CH_ISNT_STANDBY				0x0202
                                    	
//jig                               	
#define P2_CD_JG_ISNT_READY					0x0300
#define P2_CD_JG_IS_LOCAL					0x0301
#define P2_CD_JG_DOOR_IS_OPEN				0x0302
#define P2_CD_JG_NVRAM_DISABLE				0x0303
#define P2_CD_JG_MAIN_CYL_UP_TIMEOUT		0x0304
#define P2_CD_JG_MAIN_CYL_DOWN_TIMEOUT		0x0305
#define P2_CD_JG_LATCH_CYL_OPEN_TIMEOUT		0x0306
#define P2_CD_JG_LATCH_CYL_CLOSE_TIMEOUT	0x0307
#define P2_CD_JG_GRIP_CYL_OPEN_TIMEOUT		0x0308
#define P2_CD_JG_GRIP_CYL_CLOSE_TIMEOUT		0x0309
#define P2_CD_JG_SMOKE_SENS					0x030A
#define P2_CD_JG_TEMP_OVER					0x030B
#define P2_CD_JG_TRAY_UNLOAD				0x030C
#define P2_CD_JG_MAIN_CYL_UP_SENS			0x030D
#define P2_CD_JG_MAIN_CYL_DOWN_SENS			0x030E
#define P2_CD_JG_LATCH_CYL_OPEN_SENS		0x0310
#define P2_CD_JG_LATCH_CYL_CLOSE_SENS		0x0311
#define P2_CD_JG_GRIP_CYL_OPEN_SENS			0x0312
#define P2_CD_JG_GRIP_CYL_CLOSE_SENS		0x0313

//signal
#define P2_SIG_NET_CONNECTED				0
#define P2_SIG_REST_END						1
#define P2_SIG_AC_POWER_FAIL_A				2
#define P2_SIG_SEND_RESULT_1_POWER_FAIL		3
#define P2_SIG_SEND_RESULT_2_POWER_FAIL		4
#define P2_SIG_CALCURATE_CHECK_RESULT		5
#define P2_SIG_SEND_CHECK_RESULT			6
#define P2_SIG_SEND_CMD_COMM_CHECK_TIMER	7
#define P2_SIG_SEND_MODULE_INFO				8
#define P2_SIG_SEND_ERROR_CODE				9
#define P2_SIG_SEND_CALI_DATA				10
#define P2_SIG_TEST_COND_RCV				11
#define P2_SIG_RECEIVED_MODULE_INFO_REPLY	12
#define P2_SIG_METER_CONNECT_REPLY			13
#define P2_SIG_SEND_STEP_DATA				14
#define P2_SIG_NET_CONNECT_DISAPPROVAL		15
#define P2_SIG_SEND_GROUP_STATE				16
#define P2_SIG_RCV_AUTO_REPORT_DATA			17
#define P2_SIG_RCV_CH_ATTRIBUTE_SET			18
#define P2_SIG_NO_CONNECTION_RETRY			19

//client state
#define P2_G_IDLE							0
#define P2_G_SELFTEST						1
#define P2_G_STANDBY						2
#define P2_G_RUN							3
#define P2_G_PAUSE							4
#define P2_G_FAIL							5
#define P2_G_MAINTENANCE					6
#define P2_G_END							14

//channel state
#define	P2_C_IDLE							0
#define P2_C_SELFTEST						1
#define	P2_C_STANDBY						2
#define P2_C_RUN							3
#define P2_C_PAUSE							4
#define P2_C_FAIL							5
#define P2_C_MAINTERNANCE					6
#define P2_C_OCV							7
#define P2_C_CHARGE							8
#define P2_C_DISCHARGE						9
#define P2_C_REST							10
#define P2_C_Z								11
#define P2_C_CHECK							12
#define P2_C_STOP							13
#define P2_C_END							14
#define P2_C_FAULT							15
#define P2_C_COMMON							16
#define P2_C_NONCELL						17
#define P2_C_READY							18
#define P2_C_LONG_TIME_REST					19

//step type
#define P2_STEP_IDLE						0
#define P2_STEP_CHARGE						1
#define P2_STEP_DISCHARGE					2
#define P2_STEP_REST						3
#define P2_STEP_OCV							4
#define P2_STEP_Z							5
#define P2_STEP_END							6
#define P2_STEP_ADV_CYCLE					7
#define P2_STEP_CYCLE						P2_STEP_ADV_CYCLE
#define P2_STEP_PATTERN						8
#define P2_STEP_LONG_TIME_REST				9
#define P2_STEP_LOOP						12

//step mode
#define P2_MODE_IDLE						0
#define P2_MODE_CC_CV						1
#define P2_MODE_CC							2
#define P2_MODE_CV							3
#define P2_MODE_DC							4
#define P2_MODE_AC							5
#define P2_MODE_CP							6
#define P2_MODE_CCP							7
#define P2_MODE_CR							8
#define P2_MODE_CP_CC						9

//grade item
#define P2_GRADE_ITEM_IDLE					0
#define P2_GRADE_ITEM_V						1
#define P2_GRADE_ITEM_CAPACITY				2
#define P2_GRADE_ITEM_Z						3

//channel code
#define P2_C_CD_NONE						0 //code 64  ~ 95
#define P2_C_CD_NONCELL						1
#define P2_C_CD_MACHINE_ERROR				2 //code 176 ~ 255
#define P2_C_CD_BAD_CELL					3 //code 128 ~ 175
#define P2_C_CD_CELL_CHECK_ERROR			4 //code 96  ~ 127
#define P2_C_CD_END_CODE_START				64 //end code
#define P2_C_CD_END_T						64
#define P2_C_CD_END_V						65
#define P2_C_CD_END_I						66
#define P2_C_CD_END_C						67
#define P2_C_CD_END_OCV						68
#define P2_C_CD_END_STEP					69
#define P2_C_CD_END_STOP_CMD				70
#define P2_C_CD_END_PAUSE_CMD				71
#define P2_C_CD_END_CHECK					72
#define P2_C_CD_END_NEXTSTEP_CMD			73
#define P2_C_CD_END_Z						74
#define P2_C_CD_END_CYCLE_STEP				75
#define P2_C_CD_END_LOOP_STEP				76
#define P2_C_CD_END_DELTA_V					77
#define P2_C_CD_END_SOC						78
#define P2_C_CD_END_RESERVED1				79
#define P2_C_CD_STOP_CODE_START				80 //stop code
#define P2_C_CD_STOP_T						80
#define P2_C_CD_STOP_V						81
#define P2_C_CD_STOP_I						82
#define P2_C_CD_STOP_C						83
#define P2_C_CD_STOP_POWER					84
#define P2_C_CD_STOP_WATTHOUR				85
#define P2_C_CD_END_DELTA_I					86
#define P2_C_CD_END_P						87
#define P2_C_CD_END_WATT_HOUR				88
#define P2_C_CD_END_GOTO_STEP_CMD			89
#define P2_C_CD_END_VALUE_RATE_WATT_HOUR	90
#define P2_C_CD_END_SUM_AMPARE_HOUR			91
#define P2_C_CD_END_SUM_WATT_HOUR			92
#define P2_C_CD_END_SUM_TIME				93
#define P2_C_CD_FAULT_CHECK_CODE_START		96 //check step fault code
#define P2_C_CD_FAULT_CHECK_UPPER_OCV		96
#define P2_C_CD_FAULT_CHECK_LOWER_OCV		97
#define P2_C_CD_FAULT_CHECK_UPPER_V			98
#define P2_C_CD_FAULT_CHECK_LOWER_V			99
#define P2_C_CD_FAULT_CHECK_UPPER_I			100
#define P2_C_CD_FAULT_CHECK_LOWER_I			101
#define P2_C_CD_FAULT_CHECK_CONTACT_BAD		102
#define P2_C_CD_FAULT_CHECK_CONTACT_BAD2	103
#define P2_C_CD_FAULT_CHECK_CONTACT_BAD3	104
#define P2_C_CD_FAULT_CHECK_BAD_CELL		105
#define P2_C_CD_FAULT_CHECK_BAD_CELL2		106
#define P2_C_CD_FAULT_CHECK_BAD_CELL3		107
#define P2_C_CD_FAULT_CHECK_SHORT			108
#define P2_C_CD_FAULT_CHECK_ERROR_NO		109
#define P2_C_CD_FAULT_CHECK_ERROR_YES		110
#define P2_C_CD_FAULT_CHECK_DETECT_V		111
#define P2_C_CD_FAULT_CHECK_REVERSE_V		112
#define P2_C_CD_FAULT_CHECK_I_JUDGE_RATIO	113
#define P2_C_CD_FAULT_CHECK_UPPER_DELTA_V	114
#define P2_C_CD_FAULT_CHECK_LOWER_DELTA_V	115
#define P2_C_CD_FAULT_SOFT_CODE_START		128 //soft fault code
#define P2_C_CD_FAULT_UPPER_V				128
#define P2_C_CD_FAULT_LOWER_V				129
#define P2_C_CD_FAULT_UPPER_DELTA_V			130
#define P2_C_CD_FAULT_LOWER_DELTA_V			131
#define P2_C_CD_FAULT_UPPER_COMP_V			132
#define P2_C_CD_FAULT_LOWER_COMP_V			133
#define P2_C_CD_FAULT_UPPER_OCV				134
#define P2_C_CD_FAULT_LOWER_OCV				135
#define P2_C_CD_FAULT_UPPER_I				136
#define P2_C_CD_FAULT_LOWER_I				137
#define P2_C_CD_FAULT_UPPER_DELTA_I			138
#define P2_C_CD_FAULT_LOWER_DELTA_I			139
#define P2_C_CD_FAULT_UPPER_COMP_I			140
#define P2_C_CD_FAULT_LOWER_COMP_I			141
#define P2_C_CD_FAULT_UPPER_C				142
#define P2_C_CD_FAULT_LOWER_C				143
#define P2_C_CD_FAULT_UPPER_DELTA_C			144
#define P2_C_CD_FAULT_LOWER_DELTA_C			145
#define P2_C_CD_FAULT_UPPER_COMP_C			146
#define P2_C_CD_FAULT_LOWER_COMP_C			147
#define P2_C_CD_FAULT_UPPER_Z				148
#define P2_C_CD_FAULT_LOWER_Z				149
#define P2_C_CD_FAULT_UPPER_T				150
#define P2_C_CD_FAULT_LOWER_T				151
#define P2_C_CD_FAULT_STOP_CMD				152
#define P2_C_CD_FAULT_PAUSE_CMD				153
#define P2_C_CD_FAULT_NEXTSTEP_CMD			154
#define P2_C_CD_FAULT_UPPER_COMP_V2			155
#define P2_C_CD_FAULT_LOWER_COMP_V2			156
#define P2_C_CD_FAULT_UPPER_COMP_V3			157
#define P2_C_CD_FAULT_LOWER_COMP_V3			158
#define P2_C_CD_FAULT_CC_TIME_FAIL			159
#define P2_C_CD_FAULT_OPERATOR_FAIL			160
#define P2_C_CD_FAULT_UPPER_COMP_I2			161
#define P2_C_CD_FAULT_LOWER_COMP_I2			162
#define P2_C_CD_FAULT_UPPER_COMP_I3			163
#define P2_C_CD_FAULT_LOWER_COMP_I3			164
#define P2_C_CD_FAULT_ACC_CYCLE_COUNT		165
#define P2_C_CD_FAULT_UPPER_CAPACITANCE		166
#define P2_C_CD_FAULT_LOWER_CAPACITANCE		167
#define P2_C_CD_FAULT_UPPER_WATT_HOUR		168
#define P2_C_CD_FAULT_LOWER_WATT_HOUR		169
#define P2_C_CD_FAULT_MULTI_CYCLE_COUNT		170
#define P2_C_CD_FAULT_RUN_HOLD				171
#define P2_C_CD_FAULT_BAD_CELL				172
#define P2_C_CD_FAULT_UPPER_P				173
#define P2_C_CD_FAULT_LOWER_P				174
//#define P2_C_CD_FAULT_HARD_CODE_START		208 //hard_fault
#define P2_C_CD_FAULT_HARD_CODE_START		200 //hard_fault 100213
#define P2_C_CD_FAULT_WATCHDOG				200
#define P2_C_CD_FAULT_ADC					201
#define P2_C_CD_FAULT_FILE_ERROR			202
#define P2_C_CD_FAULT_CH_V_FAIL				208
#define P2_C_CD_FAULT_CH_I_FAIL				209
#define P2_C_CD_FAULT_OT					210
#define P2_C_CD_FAULT_SMPS					211
#define P2_C_CD_FAULT_FORCE_POWER			212
#define P2_C_CD_FAULT_AC_POWER				213
#define P2_C_CD_FAULT_UPS_BATTERY			214
#define P2_C_CD_FAULT_MAIN_EMG				215
#define P2_C_CD_FAULT_SUB_EMG				216
#define P2_C_CD_FAULT_NETWORK_COMM			217
#define P2_C_CD_FAULT_UPPER_TEMP			218
#define P2_C_CD_FAULT_LOWER_TEMP			219
#define P2_C_CD_FAULT_CHARGE_UPPER_V		220 //for formation start
#define P2_C_CD_FAULT_CHARGE_UPPER_AMPARE_HOUR	221
#define P2_C_CD_FAULT_CHARGE_END_UPPER_V	222
#define P2_C_CD_FAULT_CHARGE_LOWER_I		223
#define P2_C_CD_FAULT_DISCHARGE_LOWER_V		224
#define P2_C_CD_FAULT_DISCHARGE_RUN_TIME	225
#define P2_C_CD_FAULT_DISCHARGE_LOWER_AMPARE_HOUR	226
#define P2_C_CD_FAULT_OVER_CURRENT_LIMIT	227
#define P2_C_CD_FAULT_COMP_CHECK_V			228
#define P2_C_CD_FAULT_COMP_V1				229
#define P2_C_CD_FAULT_COMP_V2				230
#define P2_C_CD_FAULT_COMP_I1				231
#define P2_C_CD_FAULT_COMP_LOWER_CV_TIME	232
#define P2_C_CD_FAULT_COMP_UPPER_CV_TIME	233
#define P2_C_CD_FAULT_VOLTAGE_DATA_MIN		234 //for formation ng cond
#define P2_C_CD_FAULT_VOLTAGE_DATA_MAX		235
#define P2_C_CD_FAULT_VOLTAGE_AVG_MIN		236
#define P2_C_CD_FAULT_VOLTAGE_AVG_MAX		237
#define P2_C_CD_FAULT_AMPARE_HOUR_DATA_MIN	238
#define P2_C_CD_FAULT_AMPARE_HOUR_DATA_MAX	239
#define P2_C_CD_FAULT_AMPARE_HOUR_AVG_MIN	240
#define P2_C_CD_FAULT_AMPARE_HOUR_AVG_MAX	241
#define P2_C_CD_FAULT_POWER_DATA_MIN		242
#define P2_C_CD_FAULT_POWER_DATA_MAX		243
#define P2_C_CD_FAULT_POWER_AVG_MIN			244
#define P2_C_CD_FAULT_POWER_AVG_MAX			245 //for formation end
#define P2_C_CD_FAULT_UNKNOWN				255

//work mode
#define P2_WORK_ONLINE						0
#define	P2_WORK_OFFLINE						1

#define P2_WORK_CONTROL						0
#define P2_WORK_MAINTENANCE					1

//etc define
#define MAX_P2_CMD_SERIAL					100000000
#define P2_DATA_TYPE_SAVE					21
#define P2_DATA_TYPE_MONITOR				22
#define P2_RESULT_ITEM_NO					8
#define P2_COMP_POINT						3
#define P2_CALI_VER_NAME_SIZE				64
#define P2_MAIN_BD_SERIAL_SIZE				12
#define P2_CALI_DATE_SIZE					32
#define P2_TEST_SERIAL_LENGTH				24
#define P2_TRAY_SERIAL_LENGTH				8
#define P2_TRAY_NAME_LENGTH					32
#define P2_LOT_NAME_LENGTH					32
#define P2_PROCESS_ID_LENGTH				12
#define P2_DATE_TIME_LENGTH					16
#define P2_CELL_NO_LENGTH					8

//trouble code
#define P2_FAIL_NONE						0
#define P2_FAIL_HOLD						255
#define P2_FAIL_AC_POWER_SHORT				1
#define P2_FAIL_AC_POWER_LONG				2
#define P2_FAIL_UPS_BATTERY					3
#define P2_FAIL_MAIN_EMG					4
#define P2_FAIL_SUB_EMG						5
#define P2_FAIL_SMPS						6
#define P2_FAIL_OT							7
#define P2_FAIL_FORCE_POWER					8
#define P2_FAIL_CPU_WATCHDOG				9
#define P2_FAIL_CALI_METER_COMM_ERROR		10 //kjg_w_s
#define P2_FAIL_CALIBRATOR_COMM_ERROR		11 
#define P2_FAIL_AD_PART						12
#define P2_FAIL_OVP							13
#define P2_FAIL_OCP							14
#define P2_FAIL_UPPER_VOLTAGE				15
#define P2_FAIL_UPPER_CURRENT				16
#define P2_FAIL_RUN_TIME_OVER				17
#define P2_FAIL_BCR_COMM_ERROR				18 
#define P2_FAIL_FAN							19
#define P2_FAIL_LOAD_LINE					20
#define P2_FAIL_JIG_ACTIVE_ERROR			64
#define P2_FAIL_JIG_TRAY_ERROR				65
#define P2_FAIL_JIG_SMOKE_ERROR				66
#define P2_FAIL_JIG_TEMP_ERROR				67
#define P2_FAIL_JIG_GAS_ERROR				68
#define P2_FAIL_JIG_DOOR_ERROR				69
#define P2_FAIL_JIG_AIR_PRESS_ERROR			70
#define P2_FAIL_JIG_STACKER_ERROR			71 //kjg_w_e
#define P2_FAIL_DATASAVE_PROCESS_ERROR		72
#define P2_FAIL_ALL_CHANNEL_ERROR			73
#define P2_FAIL_CHAMBER_FIRE				74
#define P2_FAIL_CHAMBER						75
#define P2_FAIL_CHAMBER_DOOR_OPEN			76
#define P2_FAIL_JOB_CHANGE					77
#define P2_FAIL_PITCH_CHANGE				78
#define P2_FAIL_CHAMBER_AREA_SENSOR			79
#define P2_FAIL_CHAMBER_MANUAL_DOOR			80
#define P2_FAIL_CHAMBER_AIR					81
#define P2_FAIL_CHAMBER_APR_S				82
#define P2_FAIL_CHAMBER_REF					83
#define P2_FAIL_CHAMBER_COIL_TRIP			84

#define P2_BCR_SIZE							64

#endif
