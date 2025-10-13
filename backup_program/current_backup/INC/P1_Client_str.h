#ifndef __P1_CLIENT_STR_H__
#define __P1_CLIENT_STR_H__

#include "SysDefine.h"
#include "CAN_def.h"
#include "P1_Client_def.h"

typedef struct s_p1_config_tag {
	short int			groupId;
	short int			groupNo;

   	char				ipAddr[16];

	int					networkPort1; //common port
	int					networkPort2; //ch_data port
	int					protocol_version;

	int					retryCount;
	long				replyTimeout;
	long				netTimeout;
	long				pingTimeout;

	unsigned char		CmdSendLog;
	unsigned char		CmdRcvLog;
	unsigned char		CmdSendLog_Hex;
	unsigned char		CmdRcvLog_Hex;

	unsigned char		CommCheckLog;
	unsigned char		state_change;
	unsigned char		reserved1[2];

	long				send_monitor_data_interval;
	long				send_save_data_interval;
} S_P1_CONFIG;

typedef struct s_p1_rcv_packet_tag {
	int					usedBufSize;

	int					rcvCount;
	int					rcvStartPoint[MAX_P1_RECV_PACKET_COUNT];
	int					rcvSize[MAX_P1_RECV_PACKET_COUNT];
	char				rcvPacketBuf[MAX_P1_RECV_PACKET_LENGTH];

	int					parseCount;
	int					parseStartPoint[MAX_P1_RECV_PACKET_COUNT];
} S_P1_RCV_PACKET;

typedef struct s_p1_rcv_command_tag {
	int					cmdBufSize;
	char				cmd[MAX_P1_RECV_PACKET_LENGTH];
	char				cmdBuf[MAX_P1_RECV_PACKET_LENGTH];
	int					cmdFail;
	int					cmdSize;

	int					rcvCmdIndex;
	unsigned char		rcvCmdCompleteFlag;
	unsigned char		rcvCmdRestFlag;
	unsigned char		reserved1[2];
} S_P1_RCV_COMMAND;

typedef struct s_p1_retry_data_tag {
	int					seqno;
	int					replyCmd;
	int					count;
	int					size;
	char				buf[MAX_P1_RECV_PACKET_LENGTH];
} S_P1_RETRY_DATA;

typedef struct s_p1_reply_tag {
	int					timer_run;
	unsigned long		time;
	unsigned long		time2;
	S_P1_RETRY_DATA		retry;
} S_P1_REPLY;

typedef struct s_p1_cmd_header_tag {
	unsigned long		cmd_id;
	unsigned long		cmd_serial;
	unsigned short		reserved1;
	unsigned short		reserved2;
	unsigned long		chFlag[2];
	unsigned long		body_size;
} S_P1_CMD_HEADER;

typedef struct s_p1_test_cond_header_tag {
	short int			totalStep;
	short int			totalPatternCount; //PatternCount + UsermapCount
	short int			totalTimeSchCount; //TimeScheduleCount //kjh_160418
	short int			reserved;
	long				reserved1[2];
} S_P1_TEST_COND_HEADER;

typedef struct s_p1_test_cond_safety_tag {
	long				faultLowerV;
	long				faultUpperV;
	long				faultCompAuxV;
	long				faultCompAuxV2;			//csk_190808s
	long				faultCompAuxV3;
	long				faultCompAux_RefV1;
	long				faultCompAux_RefV2;
	unsigned char		faultCompAuxV_vent_flag;	//0 : no use, 1: use
	unsigned char		faultCompType_select;	//0 : Aux, 1: CAN	//ktg_241008
	unsigned char		reserved1[2];
	//unsigned char		reserved1[3];

	long				faultUpper_AuxV;
	long				faultLower_AuxV;		//csk_190808e
	//long				faultAuxV; //kjhw_181111
	long				faultUpperI;
	long				faultLowerTemp;
	long				faultUpperTemp; //kjhw_181111 use
	long				faultUpper_AmpareHour;
	long				faultUpperP;
	long				faultUpper_WattHour;

	short int			can_func_div[MAX_P1_CAN_FUNCTION];
	unsigned char		can_compare_type[MAX_P1_CAN_FUNCTION];
	unsigned char		can_data_type[MAX_P1_CAN_FUNCTION];
	float				can_value[MAX_P1_CAN_FUNCTION];

	short int			aux_func_div[MAX_P1_AUX_FUNCTION];
	unsigned char		aux_compare_type[MAX_P1_AUX_FUNCTION];
	unsigned char		aux_data_type[MAX_P1_AUX_FUNCTION];
	long				aux_value[MAX_P1_AUX_FUNCTION];

	long				faultCompGroupAuxV[MAX_AUX_GROUP];				//ktg_220220s
	long				faultCompGroupAuxV2[MAX_AUX_GROUP];
	long				faultCompGroupAuxV3[MAX_AUX_GROUP];
	long				faultCompGroupAux_RefV1[MAX_AUX_GROUP];
	long				faultCompGroupAux_RefV2[MAX_AUX_GROUP];
	unsigned char		faultCompGroupAuxV_vent_flag[MAX_AUX_GROUP];	//0 : no use, 1: use	//ktg_220220e
	
	long				reserved2[4];
} S_P1_TEST_COND_SAFETY;

typedef struct s_p1_test_step_header_tag {
	int					type; //kjg_w change(unsigned char)

	unsigned char		stepNo;
	unsigned char		mode;
	unsigned char		testEnd;
	unsigned char		subStep;

	unsigned char		reserved1; //kjg_090326 useSocFlag;
	unsigned char		cycle_pause; //0:none, 1:pause(element_cycle count)
									 //only use at loop step
	/*unsigned char		patternIndex; //host program is reserved	//jhj_250612	//MAX_STEP_420
	unsigned char		reserved2;*/
	short int			s_stepNo;		//jhj_250612	//MAX_STEP_420
} S_P1_TEST_STEP_HEADER;

typedef struct s_p1_test_step_reference_tag {
	long				refV_upper; //kjg_101102_s
	long				refV_lower;
	long				refI;
	long				refP;
	long				refR; //kjg_101102_e
//	long				refV;	//pattern : refV_upper
//	long				refI;	//pattern : refV_lower
//	long				refP;

	unsigned char		rangeI; //0:auto, 1:r1, 2:r2, 3:r3, 4:r4
								//for cycle : end Ah (kjg_100103)
								//-> 0:all, 1:charge, 2:discharge
	unsigned char		ValueRate_Compare; //step
								//for cycle : end WattHour (kjg_100103)
								//-> 0:all, 1:charge, 2:discharge
	unsigned short		ValueRate; //step

	unsigned long		endTime_day;
							//step : endTime_day, loop : Ah_Upper_Branch
	unsigned long		endTime;
							//step : endTime, cycle : cycleEndTime, loop : Ah_Lower_Branch
	long				V_Upper; //step : V_Upper, loop : AmpareHour_Upper
	long				Current; //step : Current, loop : AmpareHour_Lower
	long				AmpareHour_SumAmpareHour;
							//step : AmpareHour, cycle : SumAmpareHour

	long				V_Upper_Branch_CycleCount_Branch;
							//step : V_Upper_Branch, loop : CycleCount_Branch
	long				V_Lower_Branch_CycleCount;
							//step : V_Lower_Branch, loop : CycleCount

	long				DeltaV; //step : (Vpeak - Vsens), loop : WattHour_Upper
	long				Power; //step : Power, loop : WattHour_Lower
	long				WattHour_SumWattHour;
							//step : WattHour, cycle : SumWattHour

	long				Time_Branch_AccCycleCount;
							//step : Time_Branch, loop : AccCycleCount
	long				CVTime_Branch_AccCycleCountId;
							//step : CVTime_Branch, loop : AccCycleCountId
	unsigned long		CVTime_day; //step : CVTime_day, loop : Wh_Upper_Branch
	unsigned long		CVTime; //step : CVTime, loop : Wh_Lower_Branch

	unsigned char		chamber_step_check_flag;
							//0:not pause, 1:pause //kjhw_150821
	unsigned char		reserved[2];
	unsigned char		ValueRate_Item; //step 0:Idle, 1:AmpareHour, 2:WattHour

	long				V_Lower_MultiCycleCount;
							//step : V_Lower, loop : MultiCycleCount

	//unsigned char		AmpareHour_Branch_MultiCycleCountId;
	short int			AmpareHour_Branch_MultiCycleCountId;	//jhj_250612 s	//MAX_STEP_420
							//step : AmpareHour_Branch
							//cycle or loop : MultiCycleCountId
	//unsigned char		WattHour_Branch_MultiCycleCount_Branch;
	short int			WattHour_Branch_MultiCycleCount_Branch;
							//step : WattHour_Branch
							//loop : MultiCycleCount_Branch
	//unsigned char		ValueRate_Branch; //step
	//unsigned char		AccCycleCount_Branch; //loop
	short int			ValueRate_Branch; //step
	short int			AccCycleCount_Branch; //loop			//jhj_250612 e	//MAX_STEP_420

	short int			can_func_div[MAX_P1_CAN_FUNCTION];
	unsigned char		can_compare_type[MAX_P1_CAN_FUNCTION];
	unsigned char		can_data_type[MAX_P1_CAN_FUNCTION];
	short int			can_branch[MAX_P1_CAN_FUNCTION];
	float				can_value[MAX_P1_CAN_FUNCTION];
	unsigned int		can_delay_time[MAX_P1_CAN_FUNCTION];	//khj_201120

	short int			aux_func_div[MAX_P1_AUX_FUNCTION];
	unsigned char		aux_compare_type[MAX_P1_AUX_FUNCTION];
	unsigned char		aux_data_type[MAX_P1_AUX_FUNCTION];
	short int			aux_branch[MAX_P1_AUX_FUNCTION];
	long				aux_value[MAX_P1_AUX_FUNCTION];
	unsigned int		aux_delay_time[MAX_P1_AUX_FUNCTION]; //kjhw_170904

	//long				reserved2[2];	//ktg_220512s //shh_220607s
	long				pattern_max_refI;
	long				pattern_min_refI;	//ktg_220512e	//shh_220607e
	
	unsigned char		AmpareHour_WattHour_calc_flag;	//ktg_230728s
	unsigned char		reserved2[3];

	long				AmpareHour_SumAmpareHour_Lower;
	long				WattHour_SumWattHour_Lower;

	unsigned char		can_group_no[MAX_P1_CAN_FUNCTION];		//khj_210208	//jhj_250615	//AUX_CAN_AND
	unsigned char		aux_group_no[MAX_P1_CAN_FUNCTION];		//khj_210208	//jhj_250615	//AUX_CAN_AND
} S_P1_TEST_STEP_REFERENCE;

typedef struct s_p1_test_comp_cond_tag {
	long				lowerValue;
	long				upperValue;
	unsigned long		time;
} S_P1_TEST_COMP_COND;

typedef struct s_p1_test_delta_cond_tag {
	long				lowerValue;
	long				upperValue;
	unsigned long		time;
} S_P1_TEST_DELTA_COND;

typedef struct s_p1_test_record_cond_tag {
	unsigned long		time;
	long				deltaV;
	long				deltaI;
	long				deltaT; //temperature
	long				deltaP;
	long				reserved;
} S_P1_TEST_RECORD_COND;

typedef struct s_p1_test_edlc_cond_tag {
	long				capacitanceV1;
	long				capacitanceV2;
	unsigned long		startT_Z;
	unsigned long		endT_Z;
	unsigned long		startT_LC;
	unsigned long		endT_LC;
} S_P1_TEST_EDLC_COND;

typedef struct s_p1_test_grade_step_tag {
	unsigned char		gradeCode;
	unsigned char		reserved1; //-> item
	short int			reserved2;
	long				lowerValue;	//equal and upper(lowerValue <= x)
	long				upperValue;	//only lower(upperValue > x)
} S_P1_TEST_GRADE_STEP;

typedef struct s_p1_test_grade_cond_tag {
	unsigned char		item;
	unsigned char		gradeStepCount;
	short int			reserved1; //->itemType

	S_P1_TEST_GRADE_STEP	gradeStep[MAX_P1_GRADE_STEP];
} S_P1_TEST_GRADE_COND;

typedef struct s_p1_test_cond_step_tag {
	S_P1_TEST_STEP_HEADER	header;
	S_P1_TEST_STEP_REFERENCE	reference[MAX_P1_SUB_STEP];
	S_P1_TEST_COMP_COND		compV[MAX_P1_COMP_POINT];
	S_P1_TEST_COMP_COND		compI[MAX_P1_COMP_POINT];
	S_P1_TEST_DELTA_COND	deltaV;
	S_P1_TEST_DELTA_COND	deltaI;
	S_P1_TEST_RECORD_COND	record;
	S_P1_TEST_EDLC_COND		edlc;
	S_P1_TEST_GRADE_COND	grade[MAX_P1_GRADE_ITEM];

	long				faultUpperV;
	long				faultLowerV;
	long				faultUpperI;
	long				faultLowerI;
	long				faultUpper_AmpareHour;
	long				faultLower_AmpareHour;
	long				faultUpperZ;
	long				faultLowerZ;
	long				faultUpperTemp;
	long				faultLowerTemp;
	long				faultCompAuxV;		//csk_190527
	long				faultCompAuxTemp;			//csk_190808s
	long				faultCompAuxTh;
	long				faultCompAuxT;
	long				faultDelta_AuxV;	//uV
	long				faultDelta_AuxV_T;	//sec	//csk_190808e

	unsigned char		pattern_time_type; //kjg_130430
	unsigned char		can_comm_step_check_flag;
							//0: no check, 1:check //kjhw_150821
	unsigned char		fault_can_check_flag; //kjhw_141208
	unsigned char		can_tx_stop; //kjhw_141212

	float				pattern_max_val;
	long				pattern_file_size; //usermap file size
	long				pattern_file_checksum; //usermap file checksum

	unsigned char		fault_balance_check_flag;	//csk_190114
	unsigned char		faultCompAuxV_vent_flag;	//csk_190808s
	unsigned char		faultCompAuxTemp_vent_flag;
	unsigned char		faultCompAuxTh_vent_flag;

	unsigned char		faultCompAuxT_vent_flag;
	unsigned char		faultDelta_AuxV_vent_flag;
	//unsigned char		reserved1[2];
	//long				reserved2[2];				//csk_190808e
	
	unsigned char		faultSVDF_vent_flag;			//ktg_210105s
	unsigned char		faultSVDF_count;
	
	long				faultSVDF_yt1;	//uV
	long				faultSVDF_vcell_aver;	//uV
	long				faultSVDF_yt2;	//uV
	long				faultSVDF_yt3;	//uV
	
	unsigned char		faultSVDF_use_flag;				//ktg_210105e
	unsigned char		reserved3[2];					//ktg_210330
	unsigned char		faultcv_interlock_vent_flag;	//ktg_210330

	long				faultcv_interlockI;	//ktg_210315

	long				faultDropVoltage_value;			//ktg_210408s
	unsigned char		faultDropVoltage_use_flag;
	unsigned char		faultDropVoltage_vent_flag;
	unsigned char		faultDropVoltage_count;
	unsigned char		reserved4;					//ktg_210408e
	
	
	long				faultSVDF_can_yt1;	//uV		//ktg_210706s
	long				faultSVDF_can_vcell_aver;	//uV
	long				faultSVDF_can_yt2;	//uV
	long				faultSVDF_can_yt3;	//uV
	unsigned char		faultSVDF_can_use_flag;
	unsigned char		faultSVDF_can_vent_flag;
	unsigned char		faultSVDF_can_count;

	unsigned char		faultDropVoltage_can_use_flag;	//uV
	unsigned char		faultDropVoltage_can_vent_flag;
	unsigned char		faultDropVoltage_can_count;
	unsigned char		faultCompCanV_use_flag;
	unsigned char		faultCompCanV_vent_flag;
	
	long				faultDropVoltage_can_value;
	long				faultCompCanV_value;			//ktg_210706

	unsigned char		faultSoftvent_use_flag;			//ktg_220331s
	unsigned char		faultSoftvent_vent_flag;
	unsigned char		faultSoftvent_count;
	unsigned char		reserved5;
	long				faultSoftvent_value;

	unsigned char		faultHardvent_use_flag;
	unsigned char		faultHardvent_vent_flag;
	unsigned char		faultHardvent_count;
	unsigned char		reserved6;
	long				faultHardvent_value;
	
	unsigned char		faultSoftvent_can_use_flag;
	unsigned char		faultSoftvent_can_vent_flag;
	unsigned char		faultSoftvent_can_count;
	unsigned char		reserved7;
	long				faultSoftvent_can_value;

	unsigned char		faultHardvent_can_use_flag;
	unsigned char		faultHardvent_can_vent_flag;
	unsigned char		faultHardvent_can_count;
	unsigned char		reserved8;
	long				faultHardvent_can_value;		//ktg_220331e
	unsigned long		faultVrising_time_init;			//shhw_220916s
	unsigned long		faultVrising_time_check;			
	long				faultVrising_reff_value;		//shhw_220916e		
	unsigned long		faultCC_delta_v_time_init;		//shhw_220919s
	unsigned long		faultCC_delta_v_time_period;
	long				faultCC_delta_v_reff_value;		//shhw_220919e

	unsigned char		faultCanFreeze_use_flag;		//ktg_230728s
	unsigned char		reserved9[3];			//ktg_250313
	unsigned short int	faultCanFreeze_time;
	unsigned char		reserved14[2];			//ktg_250313
	unsigned long		faultCanFreeze_CellV;
	unsigned long		faultCanFreeze_I;				//only positive	//ktg_230728e

	long				reserved10;		//ktg_241008	for AmpareHour

	unsigned char		reserved11;		//ktg_241008	//for P1023s
	unsigned char		reserved12[3];
	unsigned long		reserved13;		//for P1023e	
} S_P1_TEST_COND_STEP;

typedef struct s_p1_test_cond_time_step_tag {
	short int			stepNo; //step number
	unsigned char		timeInit; //0:no clear, 1:clear
	unsigned char		reserved;
	short int			wait_day; //day
	short int			wait_hour; //hour
	short int			wait_min; //minute
	short int			wait_sec; //second
	long				reserved2[2];
} S_P1_TEST_COND_TIME_STEP;

typedef struct s_p1_test_cond_pattern_tag {
	long				stepNo;
	long				length;
	long				type;

	char				reserved1[4];
} S_P1_TEST_COND_PATTERN;

typedef struct s_p1_test_cond_pattern_data_tag {
	long				t_val;
	long				cmd_val[2];

	unsigned char		waveform_type; //0;rectangle, 1:triangle
	unsigned char		reserved1[3];
} S_P1_TEST_COND_PATTERN_DATA;

typedef struct s_p1_test_condition_tag {
	S_P1_TEST_COND_HEADER	header;
	S_P1_TEST_COND_SAFETY	safety;
   	S_P1_TEST_COND_STEP		step[MAX_P1_STEP];

//	short int				stepCount;
//	short int				reserved1;
	long					test_cond_file_size; //kjg_130430
	long					test_cond_file_checksum; //kjg_130430
} S_P1_TEST_CONDITION;

typedef struct s_p1_test_condition_time_tag { //kjh_160418
   	S_P1_TEST_COND_TIME_STEP	time_step[MAX_P1_STEP];

	long					test_cond_time_file_size; //timesch
	long					test_cond_time_file_checksum; //timesch
} S_P1_TEST_CONDITION_TIME;

//kjg_130430
typedef struct ps_file_id_header_tag {
	unsigned int		file_id;
	unsigned int		file_version;

	char				create_data_time[64];
	char				description[128];
	char				reserved1[128];
} PS_FILE_ID_HEADER; //328

typedef struct file_test_information_tag {
	long				id;
	long				type;

	char				name[64];
	char				description[128];
	char				creator[64];
	char				modified_time[64];
} FILE_TEST_INFORMATION; //328

typedef struct file_cell_check_param_tag {
	float				max_v;
	float				min_v;
	float				max_i;
	float				min_i;
	float				max_t;
	float				min_t;
	float				max_c;

	long				max_w;
	long				max_wh;

	short int			can_func_div[MAX_P1_CAN_FUNCTION];
	unsigned char		can_compare_type[MAX_P1_CAN_FUNCTION];
	unsigned char		can_data_type[MAX_P1_CAN_FUNCTION];
	float				can_value[MAX_P1_CAN_FUNCTION];

	short int			aux_func_div[MAX_P1_AUX_FUNCTION];
	unsigned char		aux_compare_type[MAX_P1_AUX_FUNCTION];
	unsigned char		aux_data_type[MAX_P1_AUX_FUNCTION];
	float				aux_value[MAX_P1_AUX_FUNCTION];
} FILE_CELL_CHECK_PARAM; //196

typedef struct s_p1_file_test_cond_tag {
//	PS_FILE_ID_HEADER		file_id_header; //328b
//	FILE_TEST_INFORMATION	file_test_info1; //328b
//	FILE_TEST_INFORMATION	file_test_info2; //328b
//	FILE_CELL_CHECK_PARAM	file_cell_check_param; //196b
	S_P1_TEST_COND_HEADER	header;
	S_P1_TEST_COND_SAFETY	safety;
	S_P1_TEST_COND_STEP		step[MAX_P1_STEP];
//	char					pattern_file_name[MAX_P1_STEP][256];
} S_P1_FILE_TEST_COND;

typedef struct s_p1_file_test_cond_time_tag { //kjh_160418
	S_P1_TEST_COND_HEADER	header;
	S_P1_TEST_COND_TIME_STEP	time_step[MAX_P1_STEP];
} S_P1_FILE_TEST_COND_TIME;

typedef struct s_p1_response_tag {
	int					cmd;
	int					code;
} S_P1_RESPONSE;

typedef struct s_p1_rcv_cmd_module_info_request_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_MODULE_INFO_REQUEST;

typedef struct s_p1_rcv_cmd_module_set_data_tag {
	S_P1_CMD_HEADER		header;
	unsigned char		connection_retry;
	unsigned char		line_mode;
	unsigned char		control_mode;
	unsigned char		working_mode;
	unsigned int		auto_report_interval;
	unsigned int		data_save_interval;
	unsigned int		reserved1[16];
} S_P1_RCV_CMD_MODULE_SET_DATA;

typedef struct s_p1_cmd_control_tag {
	long				stepNo;
	long				cycleNo;

	short int			virtual_ch;
	short int			reserved1;

	long				reserved2[5];
} S_P1_CMD_CONTROL;

typedef struct s_p1_rcv_cmd_run_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CMD_CONTROL	control;
} S_P1_RCV_CMD_RUN;

typedef struct s_p1_rcv_cmd_stop_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CMD_CONTROL	control;
} S_P1_RCV_CMD_STOP;

typedef struct s_p1_rcv_cmd_pause_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CMD_CONTROL	control;
} S_P1_RCV_CMD_PAUSE;

typedef struct s_p1_rcv_cmd_continue_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_CONTINUE;

typedef struct s_p1_rcv_cmd_next_step_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_NEXT_STEP;

typedef struct s_p1_rcv_cmd_goto_step_tag { //kjhw_151211
	S_P1_CMD_HEADER		header;

	unsigned short int	user_branch_stepNo;
	unsigned short int	reserved;
	unsigned long		reserved1[3];
} S_P1_RCV_CMD_GOTO_STEP;

typedef struct s_p1_rcv_cmd_reset_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_RESET;

typedef struct s_p1_rcv_cmd_testcond_start_tag {
	S_P1_CMD_HEADER		header;
	S_P1_TEST_COND_HEADER	testCondHeader;
} S_P1_RCV_CMD_TESTCOND_START;

typedef struct s_p1_rcv_cmd_testcond_safety_tag {
	S_P1_CMD_HEADER		header;
	S_P1_TEST_COND_SAFETY	safety;
} S_P1_RCV_CMD_TESTCOND_SAFETY;

typedef struct s_p1_rcv_cmd_testcond_step_tag {
	S_P1_CMD_HEADER		header;
	S_P1_TEST_COND_STEP	testCondStep;
} S_P1_RCV_CMD_TESTCOND_STEP;

typedef struct s_p1_rcv_cmd_testcond_end_tag {
	S_P1_CMD_HEADER		header;

	long				test_cond_file_size;
	long				test_cond_file_checksum;
	long				test_cond_time_file_size; //timeschedule
	long				test_cond_time_file_checksum; //timeschedule
} S_P1_RCV_CMD_TESTCOND_END;

typedef struct s_p1_rcv_cmd_step_cond_request_tag { //kjg_170810
	S_P1_CMD_HEADER		header;
	long				stepNo;
	long				reserved1[3];
} S_P1_RCV_CMD_STEP_COND_REQUEST;

typedef struct s_p1_rcv_cmd_step_cond_update_tag { //kjg_170810
	S_P1_CMD_HEADER		header;
	long				stepNo;
	long				reserved1[3];
	S_P1_TEST_COND_STEP	testCond_step;
} S_P1_RCV_CMD_STEP_COND_UPDATE;

typedef struct s_p1_rcv_cmd_common_safety_cond_request_tag { //kjg_170810
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_COMMON_SAFETY_COND_REQUEST;

typedef struct s_p1_rcv_cmd_common_safety_cond_update_tag { //kjg_170810
	S_P1_CMD_HEADER			header;
	S_P1_TEST_COND_HEADER	testCond_header;
	S_P1_TEST_COND_SAFETY	testCond_safety;
} S_P1_RCV_CMD_COMMON_SAFETY_COND_UPDATE;

typedef struct s_p1_rcv_cmd_reset_reserved_cmd_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_RESET_RESERVED_CMD;

typedef struct s_p1_rcv_cmd_testcond_update_start_tag { //kjg_170810
	S_P1_CMD_HEADER		header;
	S_P1_TEST_COND_HEADER	testCondHeader;
} S_P1_RCV_CMD_TESTCOND_UPDATE_START;

typedef struct s_p1_rcv_cmd_testcond_update_end_tag { //kjg_170810
	S_P1_CMD_HEADER		header;

	long				test_cond_file_size;
	long				test_cond_file_checksum;
	long				test_cond_time_file_size; //timeschedule
	long				test_cond_time_file_checksum; //timeschedule
} S_P1_RCV_CMD_TESTCOND_UPDATE_END;

typedef struct s_p1_rcv_cmd_testcond_pattern_tag {
	S_P1_CMD_HEADER		header;
	S_P1_TEST_COND_PATTERN	testCondPattern;
} S_P1_RCV_CMD_TESTCOND_PATTERN;

typedef struct s_p1_rcv_cmd_response_tag {
	S_P1_CMD_HEADER		header;
	S_P1_RESPONSE		response;
} S_P1_RCV_CMD_RESPONSE;

typedef struct s_p1_rcv_cmd_cali_meter_connect_tag {
	S_P1_CMD_HEADER		header;
	long				type;
} S_P1_RCV_CMD_CALI_METER_CONNECT;

typedef struct s_p1_cali_point_tag {
	unsigned char		setPointNum;
	unsigned char		checkPointNum;
	unsigned char		reserved1[2];
	long				setPoint[MAX_CALI_POINT];
	long				checkPoint[MAX_CALI_POINT];
} S_P1_CALI_POINT;

typedef struct s_p1_cali_tmp_cond_tag {
	int					type;
	int					range;
	int					mode;
	int					count; //kjhw_120503 Vref x 2
	S_P1_CALI_POINT		point;
} S_P1_CALI_TMP_COND;

typedef struct s_p1_rcv_cmd_cali_start_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CALI_TMP_COND	tmpCond;
} S_P1_RCV_CMD_CALI_START;

typedef struct s_p1_rcv_cmd_cali_update_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_CALI_UPDATE;

typedef struct s_p1_rcv_cmd_comm_check_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_COMM_CHECK;

typedef struct s_p1_rcv_cmd_comm_check_reply_tag {
	S_P1_CMD_HEADER		header;
	char				result[2];
	char				sent_cmd[4];
} S_P1_RCV_CMD_COMM_CHECK_REPLY;

typedef struct s_p1_ch_attribute_tag {
	unsigned char		chNo_master;	//1base
	unsigned char		chNo_slave[3];	//1base

	unsigned char		opType;			//0:independent, 1:parallel
	unsigned char		reserved1[3];

	unsigned char		reserved2[4];	//kjhw_151021
} S_P1_CH_ATTRIBUTE;

typedef struct s_p1_rcv_cmd_ch_attribute_set_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CH_ATTRIBUTE	attr[P1_MAX_CH_PER_MODULE];
} S_P1_RCV_CMD_CH_ATTRIBUTE_SET;

typedef struct s_p1_rcv_cmd_ch_attribute_request_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_CH_ATTRIBUTE_REQUEST;

typedef struct s_p1_aux_set_data_tag { //csk_200113
	unsigned char		chNo;			//1base : machine channel number
	//unsigned char		reserved1[3];
	unsigned char		tableNo;		//auxType=2 0:Non, 1:Atype, 2:Btype, 3:Ctype
	unsigned char		vent_use_flag;
	//unsigned char		reserved1;
	unsigned char		GroupNo;		//ktg_220220
	short int			auxChNo;		//1base : aux channel number
	short int			auxType;		//0:temperature, 1:v, 2:thermistor, 3:humidity, 4:gas
	char				name[MAX_AUX_NAME_SIZE];
	long				fault_upper;
	long				fault_lower;
	long				end_upper;
	long				end_lower;
	long				vent_upper;
	long				vent_lower;

	short int			function_div;
	short int			function_div2;
	short int			function_div3;
	short int			reserved2;
} S_P1_AUX_SET_DATA;

typedef struct s_p1_rcv_cmd_aux_set_tag {
	S_P1_CMD_HEADER		header;
	S_P1_AUX_SET_DATA	auxSetData[MAX_AUX_DATA];
} S_P1_RCV_CMD_AUX_SET;

typedef struct s_p1_can_receive_common_data_tag {
	unsigned char		can_baudrate; //0:125K, 1:250K, 2:500K(default), 3:1M
	unsigned char		extended_id; //0:unused(default), 1:used
	unsigned char		bms_type;
						//0:bms_none(default), 1:pack_relay, 2:sbl_bms, 3:hmc
	unsigned char		sjw; //0:default

	long				controller_canID;
	long				mask[2];
	long				filter[6];

	//kjg_180405_s long				reserved1[3];
	unsigned char		can_fd_flag; //0:can_2.0B(default), 1:can_fd
	unsigned char		can_datarate;
						//0:500K, 1:833K, 2:1M, 3:1M5, 4:2M(default), 5:3M, 6:4M
						//7:3M2 kjg_220714
	unsigned char		terminal_r; //0:open, 1:120ohm(default)
	unsigned char		crc_type; //0:non_iso_crc, 1:iso_crc(default)

	//long				reserved2[2]; //kjg_180405_e
	unsigned char		can_lin_flag; //kjhw_181111
	unsigned char		reserved1[3];

	//long				reserved2;
	unsigned char		cell_check_use_flag;	//ktg_210706
	unsigned char		reserved2[3];

	short int			func_div[MAX_P1_CAN_FUNCTION]; //kjg_101220_s
	unsigned char		compare_type[MAX_P1_CAN_FUNCTION];
	unsigned char		data_type[MAX_P1_CAN_FUNCTION];
	float				value[MAX_P1_CAN_FUNCTION]; //kjg_101220_e

	//ksh_241112_s
	unsigned char		manual_can_bitconfig; //CAN_BitConfigure 0: OFF, 1: Manual ON
	unsigned char		reserved3[3];

	unsigned char		nominal_brp; 	//CAN_BitConfigure nominal
	unsigned char		nominal_tseg1;	//CAN_BitConfigure nominal
	unsigned char		nominal_tseg2;	//CAN_BitConfigure nominal
	unsigned char		nominal_sjw; //CAN_BitConfigure nominal

	float				nominal_sample_point; //CAN_BitConfigure nominal

	unsigned char		data_brp; 	//CAN_BitConfigure data
	unsigned char		data_tseg1;	//CAN_BitConfigure data
	unsigned char		data_tseg2;	//CAN_BitConfigure data
	unsigned char		data_sjw;	//CAN_BitConfigure data

	float				data_sample_point;	//CAN_BitConfigure data

	//ksh_241112_e
} S_P1_CAN_RECEIVE_COMMON_DATA;

typedef struct s_p1_can_receive_normal_data_tag {
	unsigned char		canType;		//0:unused, 1:master, 2:slave
	unsigned char		byte_order;		//0:intel, 1:motolora
	unsigned char		data_type;	//0:unsigned, 1:signed, 2:float, 3:string
	unsigned char		reserved1;

	float				factor;
	short int			startBit;
	short int			bitCount;

	long				canID;			//hex value
	char				name[MAX_CAN_NAME_SIZE];
	float				fault_upper;
	float				fault_lower;
	float				end_upper;
	float				end_lower;

	float				default_value;
	float				offset;

	short int			function_div;
	short int			function_div2;
	short int			function_div3;
	short int			reserved2;

	long				sent_period;
	long				reserved3[2];

	short int			startBit2; //kjhw_131204s
	short int			bitCount2;

	unsigned char		byte_order2;	//0:intel, 1:motolora
	unsigned char		data_type2;		//0:unsigned
	unsigned char		reserved4[2];

	float				compare_value; //kjhw_131204e
} S_P1_CAN_RECEIVE_NORMAL_DATA;

typedef struct s_p1_can_receive_set_data_tag {
	S_P1_CAN_RECEIVE_COMMON_DATA
		commonData[P1_MAX_CH_PER_MODULE][MAX_CAN_TYPE];
	S_P1_CAN_RECEIVE_NORMAL_DATA
		normalData[P1_MAX_CH_PER_MODULE][MAX_CAN_DATA];
} S_P1_CAN_RECEIVE_SET_DATA;

typedef struct s_p1_rcv_cmd_can_recieve_set_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CAN_RECEIVE_SET_DATA	canReceiveSetData;
} S_P1_RCV_CMD_CAN_RECEIVE_SET;

typedef struct s_p1_rcv_cmd_ch_can_recieve_set_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CAN_RECEIVE_COMMON_DATA commonData[MAX_CAN_TYPE];
	S_P1_CAN_RECEIVE_NORMAL_DATA normalData[MAX_CAN_DATA];
} S_P1_RCV_CMD_CH_CAN_RECEIVE_SET;

typedef struct s_p1_can_transmit_common_data_tag {
	unsigned char		can_baudrate; //0:125K, 1:250K, 2:500K(default), 3:1M
	unsigned char		extended_id; //0:unused(default), 1:used
	unsigned char		bms_type;
						//0:bms_none(default), 1:pack_relay, 2:sbl_bms, 3:hmc
	unsigned char		sjw; //0:default

	long				controller_canID;

	//kjg_180405_s long				reserved1[2];
	unsigned char		can_fd_flag; //0:can_2.0B(default), 1:can_fd
	unsigned char		can_datarate;
						//0:500K, 1:833K, 2:1M, 3:1M5, 4:2M(default), 5:3M, 6:4M
						//7:3M2 kjg_220714
	unsigned char		terminal_r; //0:open, 1:120ohm(default)
	unsigned char		crc_type; //0:non_iso_crc, 1:iso_crc(default)

	//long				reserved2; //kjg_180405_e
	unsigned char		can_lin_flag; //kjhw_181111
	unsigned char		reserved1[3];

	short int			func_div[MAX_P1_CAN_FUNCTION];
	unsigned char		compare_type[MAX_P1_CAN_FUNCTION];
	unsigned char		data_type[MAX_P1_CAN_FUNCTION];
	float				value[MAX_P1_CAN_FUNCTION];

	//ksh_241112_s
	unsigned char		manual_can_bitconfig; //CAN_BitConfigure 0: OFF, 1: Manual ON
	unsigned char		reserved2[3];
	
	unsigned char		nominal_brp; 	//CAN_BitConfigure nominal
	unsigned char		nominal_tseg1;	//CAN_BitConfigure nominal
	unsigned char		nominal_tseg2;	//CAN_BitConfigure nominal
	unsigned char		nominal_sjw; //CAN_BitConfigure nominal

	float				nominal_sample_point; //CAN_BitConfigure nominal

	unsigned char		data_brp; 	//CAN_BitConfigure data
	unsigned char		data_tseg1;	//CAN_BitConfigure data
	unsigned char		data_tseg2;	//CAN_BitConfigure data
	unsigned char		data_sjw;	//CAN_BitConfigure data

	float				data_sample_point;	//CAN_BitConfigure data
	//ksh_241112_e
} S_P1_CAN_TRANSMIT_COMMON_DATA;

typedef struct s_p1_can_transmit_normal_data_tag {
	unsigned char		canType;		//0:unused, 1:master, 2:slave
	unsigned char		byte_order;		//0:intel, 1:motolora
	unsigned char		data_type;	//0:unsigned, 1:signed, 2:float, 3:string
	unsigned char		dlc; //kjg_180405 0, 1~8, 12, 16, 20, 24, 32, 48, 64, default(8)

	float				factor;
	float				default_value;
	short int			startBit;
	short int			bitCount;

	long				canID;			//hex value
	long				send_period;
	char				name[MAX_CAN_NAME_SIZE];

	long				reserved2;
	float				offset;

	short int			function_div;
	short int			function_div2;
	short int			function_div3;
	short int			reserved3;

	long				reserved4[2];
} S_P1_CAN_TRANSMIT_NORMAL_DATA;

typedef struct s_p1_can_transmit_set_data_tag {
	S_P1_CAN_TRANSMIT_COMMON_DATA
		commonData[P1_MAX_CH_PER_MODULE][MAX_CAN_TYPE];
	S_P1_CAN_TRANSMIT_NORMAL_DATA
		normalData[P1_MAX_CH_PER_MODULE][MAX_CAN_DATA];
} S_P1_CAN_TRANSMIT_SET_DATA;

typedef struct s_p1_rcv_cmd_can_transmit_set_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CAN_TRANSMIT_SET_DATA	canTransmitSetData;
} S_P1_RCV_CMD_CAN_TRANSMIT_SET;

typedef struct s_p1_ch_can_transmit_set_data_tag {
	S_P1_CAN_TRANSMIT_COMMON_DATA commonData[MAX_CAN_TYPE];
	S_P1_CAN_TRANSMIT_NORMAL_DATA normalData[MAX_CAN_DATA];
} S_P1_CH_CAN_TRANSMIT_SET_DATA;

typedef struct s_p1_rcv_cmd_ch_can_transmit_set_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CAN_TRANSMIT_COMMON_DATA commonData[MAX_CAN_TYPE];
	S_P1_CAN_TRANSMIT_NORMAL_DATA normalData[MAX_CAN_DATA];
} S_P1_RCV_CMD_CH_CAN_TRANSMIT_SET;

typedef struct s_p1_rcv_cmd_aux_info_request_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_AUX_INFO_REQUEST;

typedef struct s_p1_rcv_cmd_can_receive_info_request_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_CAN_RECEIVE_INFO_REQUEST;

typedef struct s_p1_rcv_cmd_can_transmit_info_request_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_CAN_TRANSMIT_INFO_REQUEST;

typedef struct s_p1_rcv_cmd_real_time_reply_tag {
	S_P1_CMD_HEADER		header;
	char				real_time[24];
} S_P1_RCV_CMD_REAL_TIME_REPLY;

typedef struct s_p1_rcv_cmd_bms_communication_request_tag {
	S_P1_CMD_HEADER		header;

	short int			function_div;
	short int			reserved1;
} S_P1_RCV_CMD_BMS_COMM_REQUEST;

typedef struct s_p1_can_transmit_change_data_tag {
	unsigned char		canType;		//0:unused, 1:master, 2:slave
	unsigned char		reserved1[3];

	float				default_value;

	long				canID;			//hex value

	short int			startBit;
	short int			function_div;
	short int			function_div2;
	short int			function_div3;

	long				reserved2[2];
} S_P1_CAN_TRANSMIT_CHANGE_DATA;

typedef struct s_p1_rcv_cmd_can_transmit_change_tag {
	S_P1_CMD_HEADER		header;

	S_P1_CAN_TRANSMIT_CHANGE_DATA changeData[10];
} S_P1_RCV_CMD_CAN_TRANSMIT_CHANGE;

typedef struct s_p1_rcv_cmd_bms_eol_3p_system_request_tag { //kjg_120619_s
	S_P1_CMD_HEADER		header;

	float				percent_input_v;
	long				reserved1[4];
} S_P1_RCV_CMD_BMS_EOL_3P_SYSTEM_REQUEST;

typedef struct s_p1_send_cmd_bms_eol_3p_system_reply_tag {
	S_P1_CMD_HEADER		header;

	float				percent_input_v;
	long				reserved1[4];
} S_P1_SEND_CMD_BMS_EOL_3P_SYSTEM_REPLY; //kjg_120619_e

typedef struct s_p1_rcv_cmd_daq_isolation_request_tag {
	S_P1_CMD_HEADER		header;

	unsigned char		iso;
	//jhkw_180404s
	unsigned char       main_ch;
	unsigned char       reserved1[2];
	//unsigned char		reserved1[3];
	//jhkw_180404e
} S_P1_RCV_CMD_DAQ_ISOLATION_REQUEST;

//jhkw_150224s
typedef struct s_p1_rcv_cmd_daq_isolation_request2_tag {
	S_P1_CMD_HEADER     header;

	unsigned char       iso;
	unsigned char       div_ch;
	//jhkw_180404s
	unsigned char       main_ch;
	unsigned char       reserved1;
	//unsigned char       reserved1[2];
	//jhkw_180404e
} S_P1_RCV_CMD_DAQ_ISOLATION_REQUEST2;
//jhkw_150224e

typedef struct s_p1_rcv_cmd_out_mux_select_request_tag { //kjhw_151021
	S_P1_CMD_HEADER		header;

	unsigned char		out_mux; //0: all_off, 1:mux_a, 2:mux_b
	unsigned char		reserved1[3];
} S_P1_RCV_CMD_OUT_MUX_SELECT_REQUEST;

typedef struct s_p1_rcv_cmd_can_or_lintocan_set_tag { //kjhw_181111
	S_P1_CMD_HEADER		header;

	unsigned char		lintocan_set; //0: can, 1:LintoCan
	unsigned char		reserved1[3];
} S_P1_RCV_CMD_CAN_OR_LINTOCAN_SET;

typedef struct s_p1_rcv_cmd_th_table_set_tag { //kjhw_181111
	S_P1_CMD_HEADER		header;

	unsigned char		th_table; //
	unsigned char		reserved1[3];
} S_P1_RCV_CMD_TH_TABLE_SET;

typedef struct s_p1_send_cmd_th_table_set_reply_tag { //kjhw_181111
	S_P1_CMD_HEADER		header;

	unsigned char		th_table; //
	unsigned char		reserved1[3];
} S_P1_SEND_CMD_TH_TABLE_SET_REPLY;

typedef struct s_p1_rcv_cmd_humidity_table_set_tag { //csk_200207
	S_P1_CMD_HEADER		header;

	unsigned char		humidity_table;
	unsigned char		reserved1[3];
} S_P1_RCV_CMD_HUMIDITY_TABLE_SET;

typedef struct s_p1_send_cmd_humidity_table_set_reply_tag { //csk_200207
	S_P1_CMD_HEADER		header;

	unsigned char		humidity_table;
	unsigned char		reserved1[3];
} S_P1_SEND_CMD_HUMIDITY_TABLE_SET_REPLY;

typedef struct s_p1_rcv_cmd_can_th_table_set_tag { //ktg_220614
			S_P1_CMD_HEADER		header;

				unsigned char		can_th_table;
					unsigned char		reserved1[3];
} S_P1_RCV_CMD_CAN_TH_TABLE_SET;

typedef struct s_p1_send_cmd_can_th_table_set_reply_tag { //ktg_220614
			S_P1_CMD_HEADER		header;

				unsigned char		can_th_table;
					unsigned char		reserved1[3];
} S_P1_SEND_CMD_CAN_TH_TABLE_SET_REPLY;

typedef struct s_p1_rcv_cmd_eol_procedure_request_tag { //kjg_120709_s
	S_P1_CMD_HEADER		header;

	unsigned char		index;
	unsigned char		sub_index;
	short int			response; //0:0k, 1~:fail code

	long				long_value[4];
	char				string_value[256];
	float				float_value[4];
	unsigned char		byte_value[4];
} S_P1_RCV_CMD_EOL_PROCEDURE_REQUEST;

typedef struct s_p1_send_cmd_eol_procedure_reply_tag {
	S_P1_CMD_HEADER		header;

	unsigned char		index;
	unsigned char		sub_index;
	short int			response; //0:0k, 1~:fail code

	long				long_value[4];
	char				string_value[256];
	float				float_value[4];
	unsigned char		byte_value[4];
} S_P1_SEND_CMD_EOL_PROCEDURE_REPLY; //kjg_120709_e

typedef struct s_p1_rcv_cmd_bms_procedure_request_tag { //kjg_161207_s
	S_P1_CMD_HEADER		header;

	unsigned char		bms_type;
	unsigned char		index;
	unsigned char		reserved1[2];
} S_P1_RCV_CMD_BMS_PROCEDURE_REQUEST;

typedef struct s_p1_send_cmd_bms_procedure_reply_tag {
	S_P1_CMD_HEADER		header;

	unsigned char		ch;
	unsigned char		bms_type;
	unsigned char		index;
	unsigned char		response; //0:0k, 1~:fail code
} S_P1_SEND_CMD_BMS_PROCEDURE_REPLY; //kjg_161207_e

typedef struct s_p1_rcv_cmd_calimode_tag {	//jhkw_130119s
	S_P1_CMD_HEADER		header;

	unsigned char		mode;
	//0:display,  1:cali
	unsigned char		reserved1[3];
} S_P1_RCV_CMD_CALIMODE;	//jhkw_130119e

typedef struct s_p1_rcv_cmd_cali_temp_request_tag {		//khj_210802
	S_P1_CMD_HEADER header;

	unsigned char mode;
	unsigned char reserved[3];
} S_P1_RCV_CMD_CALI_TEMP_REQUEST;

typedef struct s_p1_send_cmd_cali_temp_reply_tag {		//khj_210802
	S_P1_CMD_HEADER	header;

	unsigned char 	value;
	unsigned char	reserved[3];
} S_P1_SEND_CMD_CALI_TEMP_REPLY;

typedef struct s_p1_rcv_cmd_load_type_set_tag {	//kjhw_130129s
	S_P1_CMD_HEADER		header;

	unsigned char		type;
	//0:EDLC,  1:cell
	unsigned char		reserved1[3];
} S_P1_RCV_CMD_LOAD_TYPE_SET;  //kjhw_130129e

typedef struct s_p1_rcv_cmd_ch_io_set_tag { //kjg_101219
	S_P1_CMD_HEADER		header;

	unsigned char		type; //0:none, 1:key_on, 2:charge_on, 3:pack_relay
	unsigned char		value; //0:off, 1:on
	unsigned char		reserved1[2];
} S_P1_RCV_CMD_CH_IO_SET;

typedef struct s_p1_rcv_cmd_buzzer_stop_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_BUZZER_STOP;

typedef struct s_p1_rcv_cmd_alarm_reset_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_ALARM_RESET;

typedef struct s_p1_rcv_cmd_chamber_vent_stop_tag {	//csk_200122
	S_P1_CMD_HEADER		header;

	unsigned char		ch;
	unsigned char		close_open_flag;	//0:close, 1:open
	unsigned char		reserved[2];
} S_P1_RCV_CMD_CHAMBER_VENT_STOP;

typedef struct s_p1_rcv_cmd_cap_bank_set_tag { //kjh_191014
	S_P1_CMD_HEADER		header;

	unsigned char		cap_flag;
	unsigned char		reserved[3];
} S_P1_RCV_CMD_CAP_BANK_SET;

typedef struct s_p1_rcv_cmd_gui_emg_set_tag { //khj_201227
	S_P1_CMD_HEADER		header;

	unsigned char		emg_flag;
	unsigned char		reserved[3];
} S_P1_RCV_CMD_GUI_EMG_SET;

typedef struct s_p1_rcv_cmd_gui_shutdown_tag { //ktg_250410	//jhj_250410
	S_P1_CMD_HEADER		header;
	/*
	unsigned char		emg_flag;
	unsigned char		reserved[3];
	*/
} S_P1_RCV_CMD_GUI_SHUTDOWN;

typedef struct s_p1_rcv_cmd_chamber_flag_tag { //kjg_101221
	S_P1_CMD_HEADER		header;

	unsigned char		chamber_control; //0:none, 1:pc_control
	unsigned char		chamber_standby; //0: no_standby 1:standby kjhw_141223
	unsigned char		chiller_control; //0:none, 1:pc_control //kjhw_170906
	unsigned char		reserved1;
} S_P1_RCV_CMD_CHAMBER_FLAG;

typedef struct s_p1_rcv_cmd_chamber_continue_tag { //kjg_101221
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_CHAMBER_CONTINUE;

typedef struct s_p1_rcv_cmd_cable_check_tag { //kjg_101221
	S_P1_CMD_HEADER		header;

	unsigned char		reserved1[4];
} S_P1_RCV_CMD_CABLE_CHECK;

typedef struct s_p1_rcv_cmd_cell_check_tag { //kjg_101221
	S_P1_CMD_HEADER		header;

	unsigned char		reserved1[4];
} S_P1_RCV_CMD_CELL_CHECK;

typedef struct s_p1_rcv_cmd_cycle_continue_tag { //kjg_101221
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_CYCLE_CONTINUE;

typedef struct s_p1_rcv_cmd_can_comm_check_tag { //kjhw_141201
	S_P1_CMD_HEADER		header;

	unsigned char		can_comm_check; //0:non-check, 1:check
	unsigned char		reserved1[3];
} S_P1_RCV_CMD_CAN_COMM_CHECK;

typedef struct s_p1_rcv_cmd_out_mux_use_tag { //kjhw_151021
	S_P1_CMD_HEADER		header;

	unsigned char		out_mux_use; //0:non-use, 1:use
	unsigned char		reserved1[3];
} S_P1_RCV_CMD_OUT_MUX_USE;

typedef struct s_p1_send_cmd_ch_attribute_reply_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CH_ATTRIBUTE	attr[P1_MAX_CH_PER_MODULE];
} S_P1_SEND_CMD_CH_ATTRIBUTE_REPLY;

typedef struct s_p1_send_cmd_aux_info_reply_tag {
	S_P1_CMD_HEADER		header;

	short int			installedTemp;
	short int			installedAuxV;
	short int			installedTH;
	short int			installedHumidity;		//csk_200113
	short int			installedGas;		//sec_220926
	short int			reserved1;			//sec_221017
	S_P1_AUX_SET_DATA	auxSetData[MAX_AUX_DATA];
} S_P1_SEND_CMD_AUX_INFO_REPLY;

typedef struct s_p1_send_cmd_can_receive_info_reply_tag {
	S_P1_CMD_HEADER		header;

	short int			canReceiveDataCount[P1_MAX_CH_PER_MODULE];
	S_P1_CAN_RECEIVE_SET_DATA	canReceiveSetData;
} S_P1_SEND_CMD_CAN_RECEIVE_INFO_REPLY;

typedef struct s_p1_send_cmd_can_transmit_info_reply_tag {
	S_P1_CMD_HEADER		header;

	short int			canTransmitDataCount[P1_MAX_CH_PER_MODULE];
	S_P1_CAN_TRANSMIT_SET_DATA	canTransmitSetData;
} S_P1_SEND_CMD_CAN_TRANSMIT_INFO_REPLY;

//jhkw_220103s
typedef union u_p1_sub_code_tag {
	unsigned long		ul_val[1];
	long				l_val[1];
	float				f_val[1];
	short int			si_val[2];
	unsigned char		uc_val[4];
	char				c_val[4];
} U_P1_SUB_CODE;
//jhkw_220103e
typedef struct s_p1_ch_data_tag {
	unsigned char		ch;
	unsigned char		state;
	unsigned char		stepType;
	unsigned char		stepMode;

	unsigned char		select;
	//unsigned char		code;
	unsigned short int	code;			//ktg_210807
	unsigned char		stepNo;
	unsigned char		grade;
	unsigned char		cv_flag;		//shhw_220707
	short int			s_stepNo;		//jhj_250612	//MAX_STEP_420
	//unsigned char		reserved4[2];	//shhw_220707	//jhj_250612
	//unsigned char		reserved4[3];	//ktg_210807

	long				Vsens;
	long				Isens;
	long				charge_AmpareHour;
	long				discharge_AmpareHour;
	long				capacitance;
	long				watt;
	long				charge_WattHour;
	long				discharge_WattHour;
	unsigned long		runTime_day;
	unsigned long		runTime;
	unsigned long		totalRunTime_day;
	unsigned long		totalRunTime;
	long				z;

	unsigned char		reservedCmd;	//0:normal, 1:stop, 2:pause
	unsigned char		external_comm_state;
							//0:normal, 1:error
							//0bit:aux_temp, 1bit:aux_v,
							//2bit:can_master, 3bit:can_slave
	unsigned char		ch_output_state; //kjg_101219
							//0:off, 1:on
							//0bit:key_on, 1bit:charge_on, 2bit:pack_relay_on
	unsigned char		ch_input_state; //kjg_101219

	short int			auxDataCount;
	short int			canReceiveDataCount;

	unsigned long		totalCycle;
	unsigned long		elementCycle;
	unsigned long		accCycle[MAX_P1_ACC_CYCLE];
	unsigned long		multiCycle[MAX_P1_MULTI_CYCLE];

	long				avgV;
	long				avgI;
	long				resultIndex;

	unsigned long		cvTime_day;
	unsigned long		cvTime;

	long				realDate;
	long				realClock;

	long				Vinput; //kjg_101102_s
	long				Vpower;
	long				Vbus; //kjg_101102_e

	unsigned char		chamber_control; //kjg_101220
	unsigned char		record_index;	//RECORD_T1~T5 1base
	//kjhw_141201s
	//unsigned char		reserved1[2];
	//unsigned char		can_comm_check;
	//unsigned char		reserved1;
	unsigned char		out_mux_use; //kjhw_151021 0:no use, 1:use
	unsigned char		out_mux_backup; //kjhw_151021 0:open, 1:A_mux, 2:B_mux
	//kjhw_141201e

	unsigned char		cap_bank; //kjh_191014
	unsigned char		freeze_flag;	//ktg_230728
	//unsigned char		reserved2[3]; //kjh_191014
	unsigned char		reserved2[2]; //kjh_191014

	//long				reserved2[6]; //kjh_191014
	long				maxCell;		//ktg_210706s
	long				minCell;
	short int			maxCell_dataNo;
	short int			minCell_dataNo;

	long				diffCell;
	//long				reserved3[5];	
	//long				reserved3;		//ktg_210706e
	long                minAuxV;    //jhkw_211014s
	long                maxAuxV;
	long                minAuxTemp;
	long                maxAuxTemp;
	long                minAuxTH;
	long                maxAuxTH;
	
	short int           minAuxV_auxChNo;
	short int           maxAuxV_auxChNo;
	short int           minAuxTemp_auxChNo;
	short int           maxAuxTemp_auxChNo;
	short int           minAuxTH_auxChNo;
	short int           maxAuxTH_auxChNo;
	//long				reserved3;	//jhkw_211014e
	short int           maxFreeze_CANChNo;  //ktg_230822
	short int           maxFreeze_CANTime;  //ktg_230822

	unsigned long		CycleTime_day;	//ktg_231115
	unsigned long		CycleTime;	//ktg_231115
	//jhkw_220103s
	//long 				sub_code[4];
	U_P1_SUB_CODE		sub_code[4];
	//jhkw_220103e
} S_P1_CH_DATA;

typedef struct s_p1_send_cmd_ch_data_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CH_DATA		chData[P1_MAX_CH_PER_MODULE];
} S_P1_SEND_CMD_CH_DATA;

typedef struct s_p1_aux_data_tag {
	short int			auxChNo;		//1base
	short int			auxType;		//0:temperature, 1:v
	//short int			function_div;
	//short int			reserved1;
	long				val;
} S_P1_AUX_DATA;

typedef union u_p1_can_val_tag {
	unsigned long		ul_val[2];
	long				l_val[2];
	float				f_val[2];
	unsigned char		uc_val[8];
	char				c_val[8];
	double				d_val[1];
} U_P1_CAN_VAL;

typedef struct s_p1_can_data_tag {
	unsigned char		canType; //0:unused, 1:master, 2:slave
	unsigned char		data_type; //0:unsigned, 1:signed, 2:float, 3:string
	short int			function_div;

	U_P1_CAN_VAL		val;
} S_P1_CAN_DATA;

typedef struct s_p1_send_cmd_ch_data2_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CH_DATA		chData;
	S_P1_AUX_DATA		auxData[MAX_AUX_DATA];
	S_P1_CAN_DATA		canData[MAX_CAN_DATA];
} S_P1_SEND_CMD_CH_DATA2;

typedef struct s_p1_pulse_val_tag {
	long				runTime;
	long				Vsens;
	long				Isens;
	long				capacity;
	long				wattHour;
} S_P1_PULSE_VAL;

typedef struct s_p1_pulse_data_tag {
	long				totalCycle;
	long				stepNo;
	long				dataCount;

	S_P1_PULSE_VAL		val[P1_MAX_PULSE_MSG];
} S_P1_PULSE_DATA;

typedef struct s_p1_send_cmd_pulse_data_tag {
	S_P1_CMD_HEADER		header;

	long				totalCycle;
	long				stepNo;
	long				dataCount;

	S_P1_PULSE_VAL		val[P1_MAX_PULSE_MSG];
} S_P1_SEND_CMD_PULSE_DATA;

typedef struct s_p1_module_info_tag {
	unsigned int		group_id;
	unsigned int		systemType;
	unsigned int		protocol_version;
	char				modelName[128];
	unsigned int		osVersion;
	unsigned short int	voltage_range;
	unsigned short int	current_range;
	unsigned int		voltage_spec[5];
	unsigned int		current_spec[5];
	unsigned char		can_comm_type; //kjg_180405
	//kjh_211021 unsigned char		reserved1[7];
	unsigned char		ratioV;	//0:uV, 1:mV, 2:V	//kjh_211021
	unsigned char		ratioI;	//0:uA, 1:mA, 2:A	//kjh_211021
	unsigned char		ratioP;	//0:Non,1:mW, 2:W	//kjh_211021
	unsigned char		reserved1[4];	//kjh_211021

	unsigned short int	installedBd;
	unsigned short int	chPerBd;
	unsigned int		installedCh;
	unsigned int		totalJig;
	unsigned int		BdinJig[16];
	int					reserved2[4];
} S_P1_MODULE_INFO;

typedef struct s_p1_send_cmd_module_info_reply_tag {
	S_P1_CMD_HEADER		header;
	S_P1_MODULE_INFO	md_info;
} S_P1_SEND_CMD_MODULE_INFO_REPLY;

typedef struct s_p1_send_cmd_response_tag {
	S_P1_CMD_HEADER		header;
	S_P1_RESPONSE		response;
} S_P1_SEND_CMD_RESPONSE;

typedef struct s_p1_send_cmd_test_cond_conversion_end_tag {
	S_P1_CMD_HEADER		header;
	S_P1_RESPONSE		response;
} S_P1_SEND_CMD_TEST_COND_CONVERSION_END;

typedef struct s_p1_send_cmd_test_cond_conversion_update_end_tag { //kjg_170810
	S_P1_CMD_HEADER		header;
	S_P1_RESPONSE		response;
} S_P1_SEND_CMD_TEST_COND_CONVERSION_UPDATE_END;

typedef struct s_p1_send_cmd_meter_connect_reply_tag {
	S_P1_CMD_HEADER		header;
	long				state;
} S_P1_SEND_CMD_METER_CONNECT_REPLY;

typedef struct s_p1_send_cmd_cali_start_reply_tag {
	S_P1_CMD_HEADER		header;
	S_P1_RESPONSE		response;
} S_P1_SEND_CMD_CALI_START_REPLY;

typedef struct s_p1_cali_normal_result_tag {
	int					type;
	int					count; //kjhw_120504 Vref x 2
	unsigned char		range;
	unsigned char		setPointNum;
	unsigned char		checkPointNum;
	unsigned char		ch;
	long				setPointAD[MAX_CALI_POINT];
	long				setPointDVM[MAX_CALI_POINT];
	long				checkPointAD[MAX_CALI_POINT];
	long				checkPointDVM[MAX_CALI_POINT];
} S_P1_CALI_NORMAL_RESULT;

typedef struct s_p1_cali_check_result_tag {
	int					type;
	unsigned char		range;
	unsigned char		reserved1;
	unsigned char		checkPointNum;
	unsigned char		ch;
	long				checkPointAD[MAX_CALI_POINT];
	long				checkPointDVM[MAX_CALI_POINT];
} S_P1_CALI_CHECK_RESULT;

typedef struct s_p1_send_cmd_cali_normal_result_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CALI_NORMAL_RESULT	result;
} S_P1_SEND_CMD_CALI_NORMAL_RESULT;

typedef struct s_p1_send_cmd_cali_check_result_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CALI_CHECK_RESULT	result;
} S_P1_SEND_CMD_CALI_CHECK_RESULT;

typedef struct s_p1_send_cmd_comm_check_reply_tag {
	S_P1_CMD_HEADER		header;
} S_P1_SEND_CMD_COMM_CHECK_REPLY;

typedef struct s_p1_send_cmd_comm_check_tag {
	S_P1_CMD_HEADER		header;
} S_P1_SEND_CMD_COMM_CHECK;

typedef struct s_p1_send_cmd_emg_status_tag {
	S_P1_CMD_HEADER		header;

	short int			code;
	short int			val;
	short int			reserved1[2];

	char				name[MAX_CAN_NAME_SIZE];
} S_P1_SEND_CMD_EMG_STATUS;

typedef struct s_p1_send_cmd_step_cond_reply_tag {
	S_P1_CMD_HEADER		header;
	S_P1_TEST_COND_STEP	testCond_step;
} S_P1_SEND_CMD_STEP_COND_REPLY;

typedef struct s_p1_send_cmd_common_safety_cond_reply_tag {
	S_P1_CMD_HEADER			header;
	S_P1_TEST_COND_HEADER	testCond_header;
	S_P1_TEST_COND_SAFETY	testCond_safety;
} S_P1_SEND_CMD_COMMON_SAFETY_COND_REPLY;

typedef struct s_p1_send_cmd_real_time_request_tag {
	S_P1_CMD_HEADER		header;
} S_P1_SEND_CMD_REAL_TIME_REQUEST;

typedef struct s_p1_send_cmd_bms_communication_reply_tag {
	S_P1_CMD_HEADER		header;

	short int			function_div;
	short int			reserved1;
} S_P1_SEND_CMD_BMS_COMM_REPLY;

typedef struct s_p1_send_cmd_bms_communication_end_tag {
	S_P1_CMD_HEADER		header;

	short int			function_div;
	short int			reserved1;
} S_P1_SEND_CMD_BMS_COMM_END;

typedef struct s_p1_rcv_cmd_bms_eol_data1_request_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_BMS_EOL_DATA1_REQUEST;

typedef struct s_p1_send_cmd_bms_eol_data1_reply_tag {
	S_P1_CMD_HEADER		header;

	short int			cvtn_id; //uds
	short int			reserved1;

	unsigned char		sw_version[4]; //uds

	unsigned char		HvBattHvil_D_Fault; //can
	unsigned char		DTC[3]; //uds

	float				module_delta_temp; //uds
	float				thermistor_sensor; //uds
	float				cell_delta_v; //uds

	//0:fail, 1:ok
	unsigned char		HvBattAuxCntct_D_Actl[4]; //aux_contactor can
	unsigned char		HvBattCntct_D_Actl[4]; //main_contactor can
	unsigned char		HvBattNeg_D_Actl[4]; //negative_contactor can

	//0:all off, 1:aux on, 2:aux & main on, 3:all off
	float				Cycler_Pack_V[4]; //cycler
	float				HvBatt_U_Actl[4]; //can
	float				DID_480D_Pack_V[4]; //uds
	float				Link_V[4]; //uds
	float				Charge_V[4]; //uds
	float				Pack2_V[4]; //uds
	float				Charger_Fuse_V[4]; //uds
	float				Elac_Fuse_V[4]; //uds

	float				isolation_resistance; //uds
	float				isolation_Va; //uds
	float				isolation_Vb; //uds

	unsigned char		DTC_List[256][4]; //uds
} S_P1_SEND_CMD_BMS_EOL_DATA1_REPLY;

typedef struct s_p1_rcv_cmd_bms_eol_data2_request_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_BMS_EOL_DATA2_REQUEST;

typedef struct s_p1_send_cmd_bms_eol_data2_reply_tag {
	S_P1_CMD_HEADER		header;

	short int			cvtn_id;
	short int			reserved1;

	unsigned char		sw_version[4];

	unsigned char		HvBattHvil_D_Fault;
	unsigned char		DTC[3];

	float				module_delta_temp;
	float				thermistor_sensor;
	float				cell_delta_v;

	//0:fail, 1:ok
	unsigned char		HvBattAuxCntct_D_Actl[4]; //aux_contactor
	unsigned char		HvBattCntct_D_Actl[4]; //main_contactor
	unsigned char		HvBattNeg_D_Actl[4]; //negative_contactor

	//0:all off, 1:aux on, 2:aux & main on, 3:all off
	float				Cycler_Pack_V[4];
	float				HvBatt_U_Actl[4];
	float				DID_480D_Pack_V[4];
	float				Link_V[4];
	float				Charge_V[4];
	float				Pack2_V[4];
	float				Charger_Fuse_V[4];
	float				Elac_Fuse_V[4];

	float				isolation_resistance;
	float				isolation_Va;
	float				isolation_Vb;

	unsigned char		DTC_List[256][4];
} S_P1_SEND_CMD_BMS_EOL_DATA2_REPLY;

typedef struct s_p1_send_cmd_bms_eol_data3_reply_tag {
	S_P1_CMD_HEADER		header;

	unsigned char		UBMS_State;
	unsigned char		HW_Version;
	unsigned char		SW_Version;
	unsigned char		UBMS_ID;

	unsigned char		reserved1[4];
	unsigned char		reserved2[4];

	int					cali_v[4];
	int					cell_v[4];
	int					temp;
	int					module_v;
	int					obd;

	unsigned char		cali_val[4];
	int					reserved3[3];
} S_P1_SEND_CMD_BMS_EOL_DATA3_REPLY;

typedef struct s_p1_rcv_cmd_bms_eol_pack_id_write_tag {
	S_P1_CMD_HEADER		header;

	unsigned char		reserved1[4];

	char				pack_id[40];
} S_P1_RCV_CMD_BMS_EOL_PACK_ID_WRITE;

typedef struct s_p1_send_cmd_bms_eol_pack_id_write_reply_tag {
	S_P1_CMD_HEADER		header;

	unsigned char		response; //0:NG, 1:OK
	unsigned char		reserved1[3];

	char				reserved2[40];
} S_P1_SEND_CMD_BMS_EOL_PACK_ID_WRITE_REPLY;

typedef struct s_p1_rcv_cmd_bms_eol_pack_id_check_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_BMS_EOL_PACK_ID_CHECK;

typedef struct s_p1_send_cmd_bms_eol_pack_id_check_reply_tag {
	S_P1_CMD_HEADER		header;

	unsigned char		response; //0:NG, 1:OK
	unsigned char		reserved1[3];

	char				pack_id[40];
} S_P1_SEND_CMD_BMS_EOL_PACK_ID_CHECK_REPLY;

typedef struct s_p1_rcv_cmd_bms_eol_hi_pot_test_tag {
	S_P1_CMD_HEADER		header;

	unsigned char		type;
		//1:positive(+), 2:positive(-), 3:negative(+), 4:negative(-)
	unsigned char		reserved1[3];
} S_P1_RCV_CMD_BMS_EOL_HI_POT_TEST;

typedef struct s_p1_send_cmd_bms_eol_hi_pot_test_reply_tag {
	S_P1_CMD_HEADER		header;

	unsigned char		type;
	unsigned char		response; //0:NG, 1:OK
	unsigned char		reserved1[2];
} S_P1_SEND_CMD_BMS_EOL_HI_POT_TEST_REPLY;

typedef struct s_p1_rcv_cmd_uds_vbf_info_request_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_UDS_VBF_INFO_REQUEST;

typedef struct s_p1_send_cmd_uds_vbf_info_reply_tag {
	S_P1_CMD_HEADER		header;

	char				vbf_file_name[128];
} S_P1_SEND_CMD_UDS_VBF_INFO_REPLY;

typedef struct s_p1_rcv_cmd_uds_vbf_change_request_tag {
	S_P1_CMD_HEADER		header;

	char				vbf_file_name[128];

	char				vbf_data[MAX_CAN_FLASH_FILE_SIZE]; //1024*50
} S_P1_RCV_CMD_UDS_VBF_CHANGE_REQUEST;

typedef struct s_p1_send_cmd_uds_vbf_change_reply_tag {
	S_P1_CMD_HEADER		header;

	unsigned char		response; //0:NG, 1:OK
	unsigned char		reserved1[3];

	char				vbf_file_name[128];
} S_P1_SEND_CMD_UDS_VBF_CHANGE_REPLY;

typedef struct s_p1_rcv_cmd_bms_eol_data3_request_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_BMS_EOL_DATA3_REQUEST;

typedef struct s_p1_rcv_cmd_bms_eol_data3_led_request_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_BMS_EOL_DATA3_LED_REQUEST;

typedef struct s_p1_rcv_cmd_bms_eol_cvtn_id_request_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_BMS_EOL_CVTN_ID_REQUEST;

typedef struct s_p1_send_cmd_bms_eol_cvtn_id_reply_tag {
	S_P1_CMD_HEADER		header;

	short int			cvtn_id; //uds
	short int			reserved1;

	unsigned char		sw_version[4]; //uds

	unsigned char		HvBattHvil_D_Fault; //can
	unsigned char		DTC[3]; //uds

	float				module_delta_temp; //uds
	float				thermistor_sensor; //uds
	float				cell_delta_v; //uds

	//0:fail, 1:ok
	unsigned char		HvBattAuxCntct_D_Actl[4]; //aux_contactor can
	unsigned char		HvBattCntct_D_Actl[4]; //main_contactor can
	unsigned char		HvBattNeg_D_Actl[4]; //negative_contactor can

	//0:all off, 1:aux on, 2:aux & main on, 3:all off
	float				Cycler_Pack_V[4]; //cycler
	float				HvBatt_U_Actl[4]; //can
	float				DID_480D_Pack_V[4]; //uds
	float				Link_V[4]; //uds
	float				Charge_V[4]; //uds
	float				Pack2_V[4]; //uds
	float				Charger_Fuse_V[4]; //uds
	float				Elac_Fuse_V[4]; //uds

	float				isolation_resistance; //uds
	float				isolation_Va; //uds
	float				isolation_Vb; //uds

	unsigned char		DTC_List[256][4]; //uds
} S_P1_SEND_CMD_BMS_EOL_CVTN_ID_REPLY;

typedef struct s_p1_rcv_cmd_bms_eol_dtc_clear_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_BMS_EOL_DTC_CLEAR;

typedef struct s_p1_rcv_cmd_bms_eol_reset_micro_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_BMS_EOL_RESET_MICRO;

typedef struct s_p1_send_cmd_daq_isolation_reply_tag {
	S_P1_CMD_HEADER		header;

	unsigned char		iso;
	//jhkw_180404s
	unsigned char		main_ch;
	unsigned char		reserved1[2];
	//unsigned char		reserved1[3];
	//jhkw_180404e
} S_P1_SEND_CMD_DAQ_ISOLATION_REPLY;

//jhkw_150224s
typedef struct s_p1_send_cmd_daq_isolation_reply2_tag {
	S_P1_CMD_HEADER		header;

	unsigned char		iso;
	unsigned char		div_ch;
	//jhkw_180404s
	unsigned char		main_ch;
	unsigned char		reserved1;
	//unsigned char		reserved1[2];
	//jhkw_180404e
} S_P1_SEND_CMD_DAQ_ISOLATION_REPLY2;
//jhkw_150224e

typedef struct s_p1_send_cmd_out_mux_select_reply_tag { //kjhw_151021
	S_P1_CMD_HEADER		header;

	unsigned char		ch;
	unsigned char		fault; //0:NG, 1:OK
	unsigned char		setting;
	unsigned char		state;
	unsigned char		fault_flag;
	unsigned char		reserved[3];
} S_P1_SEND_CMD_OUT_MUX_SELECT_REPLY;

typedef struct s_p1_misc_tag {
	int					network_socket1; //common socket
	int					network_socket2; //ch_data socket

	unsigned long		cmd_serial;

	int					CO_No;
	int					psSignal;
	int					chInGroup;
	int					chOffset;
	int					processPointer;

	long				net_time;
	long				net_time2;
	long				pause_time;	//jhkw_190503	//jhj_250310 SK_network_timeout_merge
	long				pause_time2; //jhkw_190503	//jhj_250310 SK_network_timeout_merge
	long				ping_time;
	long				ping_time2;
	long				sent_monitor_data_time;
	long				sent_monitor_data_time2;
	long				sent_real_time_request;

	unsigned char		network_port_type; //1:net_port 1ea, 2:net_port 2ea
	unsigned char		reserved1[3];

	char				write_pack_id[40];

	short int			rcv_test_step_count; //kjg_120430
	short int			rcv_test_pattern_count;
	short int			rcv_test_timesch_count; //kjh_160418
	short int			reserved2; //kjg_171219
} S_P1_MISC;

typedef struct s_p1_test_config_tag {
	long				maxRefV;
	long				minRefV;
	long				maxRefI;
	long				minRefI;
	long				failDeltaV;
	long				failDeltaI;
	long				check_lowerI;
	long				check_lowerOCV;
	long				fail_charge_lower_dv;
} S_P1_TEST_CONFIG;
#endif
