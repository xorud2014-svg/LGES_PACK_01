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
	//jhkw_190830s
	//unsigned char		reserved1[2];
	unsigned char		data_save_10ms;
	//unsigned char		reserved1;
	unsigned char		Real_Time_request;
	//jhkw_190830e

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

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
typedef struct s_p1_test_cond_header_tag {
	unsigned char		totalStep;
	unsigned char		reserved1[3];
	long				reserved2[2];
} S_P1_TEST_COND_HEADER;
#elif __COA_VER_100D__
typedef struct s_p1_test_cond_header_tag {
	short int			totalStep;
	short int			totalPatternCount; //PatternCount + UsermapCount
	long				reserved1[2];
} S_P1_TEST_COND_HEADER;
#else //COA_VER_100F~
typedef struct s_p1_test_cond_header_tag {
	short int			totalStep;
	short int			totalPatternCount; //PatternCount + UsermapCount
	short int			totalTimeSchCount; //TimeScheduleCount //kjh_160418
	short int			reserved;
	long				reserved1[2];
} S_P1_TEST_COND_HEADER;
#endif

typedef struct s_p1_test_cond_safety_tag {
	long				faultLowerV;
	long				faultUpperV;
	//kjhw_150730 long				reserved1;
	long				faultCompAuxV; //kjhw_150730
	long				faultUpperI;
	long				faultLowerTemp;
	long				faultUpperTemp;
	long				faultUpper_AmpareHour;
	long				faultUpperP;
	long				faultUpper_WattHour;
	//jhkw_200507s
	long				faultUpper_AuxV;
	long				faultLower_AuxV;
	long				faultUpper_Temp;
	long				reserved1;						
	//jhkw_200507e

	short int			can_func_div[MAX_P1_CAN_FUNCTION];
	unsigned char		can_compare_type[MAX_P1_CAN_FUNCTION];
	unsigned char		can_data_type[MAX_P1_CAN_FUNCTION];
	float				can_value[MAX_P1_CAN_FUNCTION];

	short int			aux_func_div[MAX_P1_AUX_FUNCTION];
	unsigned char		aux_compare_type[MAX_P1_AUX_FUNCTION];
	unsigned char		aux_data_type[MAX_P1_AUX_FUNCTION];
	float				aux_value[MAX_P1_AUX_FUNCTION];
	//jhkw_190503s
	long				rptsoc;
	long				soc;
	long				reserved2;
	//long				reserved2[3];
	//jhkw_190503e
	//jhkw_191108s
	unsigned long		current_AH;
	unsigned long		initial_AH;
	unsigned long		crate_factor;
	//jhkw_191108e
//20181219 KHK------------------------------------------
	unsigned char		schedule_link_flag;
	unsigned char		user_define_mode_flag;
	unsigned char		reserved[2];
//--------------------------------------
	//shhw_230605s
	long				faultDelta_I;		
	long				faultDelta_AuxV;
	//shhw_230605s
} S_P1_TEST_COND_SAFETY;

typedef struct s_p1_test_step_header_tag {
	int					type; //kjg_w change(unsigned char)

	short int			stepNo;	//jhkw_200410
	//unsigned char		stepNo;
	unsigned char		mode;
	unsigned char		testEnd;
	unsigned char		subStep;

	unsigned char		reserved1; //kjg_090326 useSocFlag;
	unsigned char		cycle_pause; //0:none, 1:pause(element_cycle count)
									 //only use at loop step
	unsigned char		patternIndex; //host program is reserved
	//unsigned char		reserved2;	//jhkw_200410
} S_P1_TEST_STEP_HEADER;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__
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

	unsigned long		Time; //step : endTime, cycle : cycleEndTime
	long				V_Upper; //step
	long				Current; //step
	long				AmpareHour_SumAmpareHour;
							//step : AmpareHour, cycle : SumAmpareHour

	long				V_Upper_Branch_CycleCount_Branch;
							//step : V_Upper_Branch, loop : CycleCount_Branch
	long				V_Lower_Branch_CycleCount;
							//step : V_Lower_Branch, loop : CycleCount
	
	long				DeltaV; //step : (Vpeak - Vsens)
	long				Power; //step
	long				WattHour_SumWattHour;
							//step : WattHour, cycle : SumWattHour

	long				Time_Branch_AccCycleCount;
							//step : Time_Branch, loop : AccCycleCount
	long				CVTime_Branch_AccCycleCountId;
							//step : CVTime_Branch, loop : AccCycleCountId
	unsigned long		CVTime; //step
	
	unsigned char		reserved[3];
	unsigned char		ValueRate_Item; //step 0:Idle, 1:AmpareHour, 2:WattHour

	long				V_Lower_MultiCycleCount;
							//step : V_Lower, loop : MultiCycleCount

	unsigned char		AmpareHour_Branch_MultiCycleCountId;
							//step : AmpareHour_Branch
							//cycle or loop : MultiCycleCountId
	unsigned char		WattHour_Branch_MultiCycleCount_Branch;
							//step : WattHour_Branch
							//loop : MultiCycleCount_Branch
	unsigned char		ValueRate_Branch; //step
	unsigned char		AccCycleCount_Branch; //loop

	short int			can_func_div[MAX_P1_CAN_FUNCTION];
	unsigned char		can_compare_type[MAX_P1_CAN_FUNCTION];
	unsigned char		can_data_type[MAX_P1_CAN_FUNCTION];
	short int			can_branch[MAX_P1_CAN_FUNCTION];
	float				can_value[MAX_P1_CAN_FUNCTION];

	short int			aux_func_div[MAX_P1_AUX_FUNCTION];
	unsigned char		aux_compare_type[MAX_P1_AUX_FUNCTION];
	unsigned char		aux_data_type[MAX_P1_AUX_FUNCTION];
	short int			aux_branch[MAX_P1_AUX_FUNCTION];
	float				aux_value[MAX_P1_AUX_FUNCTION];
	long				reserved2[2];
} S_P1_TEST_STEP_REFERENCE;
#else //COA_VER_100C~
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
	//unsigned char		ValueRate_Compare; //step
	short int			ValueRate_Compare; //jhkw_200410
								//for cycle : end WattHour (kjg_100103)
								//-> 0:all, 1:charge, 2:discharge
	unsigned short		ValueRate; //step

	unsigned long		endTime_day;
	unsigned long		endTime; //step : endTime, cycle : cycleEndTime
	long				V_Upper; //step
	long				Current; //step
	long				AmpareHour_SumAmpareHour;
							//step : AmpareHour, cycle : SumAmpareHour

	long				V_Upper_Branch_CycleCount_Branch;
							//step : V_Upper_Branch, loop : CycleCount_Branch
	long				V_Lower_Branch_CycleCount;
							//step : V_Lower_Branch, loop : CycleCount
	
	long				DeltaV; //step : (Vpeak - Vsens) //20181219 KHK if step type is rest than DeltaV is Cell Balancing Delta V
	long				Power; //step
	long				WattHour_SumWattHour;
							//step : WattHour, cycle : SumWattHour

	long				Time_Branch_AccCycleCount;
							//step : Time_Branch, loop : AccCycleCount
	long				CVTime_Branch_AccCycleCountId;
							//step : CVTime_Branch, loop : AccCycleCountId
	unsigned long		CVTime_day; //step
	unsigned long		CVTime; //step
	
	unsigned char		chamber_step_check_flag;
							//0:not pause, 1:pause //kjhw_150821
//	unsigned char		reserved[2];
//20181219 KHK-----------------------------------
	unsigned char		SOC_Tracking_flag;
	unsigned char		DisCharge_SOC_Tracking_flag;	//jhkw_201102
	unsigned char		Cell_Balancing_flag;
//-------------------------------------------------	
	unsigned char		ValueRate_Item; //step 0:Idle, 1:AmpareHour, 2:WattHour

	long				V_Lower_MultiCycleCount;
							//step : V_Lower, loop : MultiCycleCount

	//unsigned char		AmpareHour_Branch_MultiCycleCountId;
	short int			AmpareHour_Branch_MultiCycleCountId;	//jhkw_200410
							//step : AmpareHour_Branch
							//cycle or loop : MultiCycleCountId
	//unsigned char		WattHour_Branch_MultiCycleCount_Branch;
	short int			WattHour_Branch_MultiCycleCount_Branch;	//jhkw_200410
							//step : WattHour_Branch
							//loop : MultiCycleCount_Branch
	//unsigned char		ValueRate_Branch; //step
	//unsigned char		AccCycleCount_Branch; //loop
	short int			ValueRate_Branch; //step	//jhkw_200410
	short int			AccCycleCount_Branch; //loop	//jhkw_200410

	short int			can_func_div[MAX_P1_CAN_FUNCTION];
	unsigned char		can_compare_type[MAX_P1_CAN_FUNCTION];
	unsigned char		can_data_type[MAX_P1_CAN_FUNCTION];
	short int			can_branch[MAX_P1_CAN_FUNCTION];
	float				can_value[MAX_P1_CAN_FUNCTION];

	short int			aux_func_div[MAX_P1_AUX_FUNCTION];
	unsigned char		aux_compare_type[MAX_P1_AUX_FUNCTION];
	unsigned char		aux_data_type[MAX_P1_AUX_FUNCTION];
	short int			aux_branch[MAX_P1_AUX_FUNCTION];
	float				aux_value[MAX_P1_AUX_FUNCTION];
	unsigned int		aux_delay_time[MAX_P1_AUX_FUNCTION]; //kjhw_170904
//20181219 KHK-----------------------------------------
	unsigned short int	rptSOC;
	unsigned short int	endSOC;

	unsigned short int	endSOC_branch;
	//short int			reserved2;
	unsigned char		reserved2;	//jhkw_200410

	long				limitCurrent_Lower;
	//long				limitCurrent_Upper;
	long				cellBalancingStart;
	long				cellBalancingStop;
	long				cellBalancingEnd;
//------------------------------------------------------
	unsigned short int	sequence_charge_flag;	//jhkw_221205s
	unsigned short int	reserved;
	long				pattern_max_refI;
	long				pattern_min_refI;	//jhkw_221205e
} S_P1_TEST_STEP_REFERENCE;
#endif

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

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
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
	long				reserved1[2];
} S_P1_TEST_COND_STEP;
#else //COA_VER_100D~
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
	unsigned char		crate_flag; //jhkw_200524
/*	//jhkw_180823s
	long				faultDeltaV;
	unsigned long		faultDeltaV_T;
	long				faultDeltaI;
	unsigned long		faultDeltaI_T;
*/	//jhkw_180823e

	//long				reserved1[2];
	unsigned char		pattern_time_type; //kjg_130430
	unsigned char		can_comm_step_check_flag;
							//0: no check, 1:check //kjhw_150821
	//unsigned char		pattern_mode_type;
	unsigned char		fault_can_check_flag; //kjhw_141208
	unsigned char		can_tx_stop; //kjhw_141212
	//unsigned char		reserved1[2];
	
	float				pattern_max_val;
	long				pattern_file_size; //usermap file size
	long				pattern_file_checksum; //usermap file checksum
//	long				reserved2[2];
//20181219 KHK------------------------------------------
	//unsigned char		schedule_link_flag;
	//unsigned char		reserved[4];
	unsigned char		reserved[3];	//jhkw_200524
	long				reserved2;
//----------------------------------------------------------	
} S_P1_TEST_COND_STEP;
#endif

//kjh_160418s
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
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
#endif //kjh_160418e

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
typedef struct s_p1_test_cond_pattern_tag {
	S_P1_TEST_STEP_HEADER	header;
	S_P1_TEST_STEP_REFERENCE	reference[MAX_P1_SUB_STEP];
	S_P1_TEST_RECORD_COND	record;

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
	long				reserved1[2];

	//insert pattern data
} S_P1_TEST_COND_PATTERN;
#else //COA_VER_100D~
typedef struct s_p1_test_cond_pattern_tag {
	long				stepNo;
	long				length;
	long				type;

	char				reserved1[4];
} S_P1_TEST_COND_PATTERN;
#endif

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
typedef struct s_p1_test_cond_pattern_data_tag {
	long				t_val;
	long				cmd_val;
} S_P1_TEST_COND_PATTERN_DATA;
#else //COA_VER_100D~
typedef struct s_p1_test_cond_pattern_data_tag {
	long				t_val;
	long				cmd_val[2];

	unsigned char		waveform_type; //0;rectangle, 1:triangle
	unsigned char		reserved1[3];
} S_P1_TEST_COND_PATTERN_DATA;
#endif

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
typedef struct s_p1_test_condition_tag {
	S_P1_TEST_COND_HEADER	header;
	S_P1_TEST_COND_SAFETY	safety;
   	S_P1_TEST_COND_STEP		step[MAX_P1_STEP];

	short int			stepCount;
	short int			reserved1;
} S_P1_TEST_CONDITION;
//#else //COA_VER_100D~
#elif __COA_VER_100D__
typedef struct s_p1_test_condition_tag {
	S_P1_TEST_COND_HEADER	header;
	S_P1_TEST_COND_SAFETY	safety;
   	S_P1_TEST_COND_STEP		step[MAX_P1_STEP];

//	short int				stepCount;
//	short int				reserved1;
	long					test_cond_file_size; //kjg_130430
	long					test_cond_file_checksum; //kjg_130430
} S_P1_TEST_CONDITION;
#else //COA_VER_100F~
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
#endif

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

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
typedef struct s_p1_file_test_cond_time_tag { //kjh_160418
	S_P1_TEST_COND_HEADER	header;
	S_P1_TEST_COND_TIME_STEP	time_step[MAX_P1_STEP];
} S_P1_FILE_TEST_COND_TIME;
#endif

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
	short int			code;	//jhkw_201102
	//short int			reserved1;

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

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
typedef struct s_p1_rcv_cmd_testcond_end_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_TESTCOND_END;
//#else //COA_VER_100D~
#elif __COA_VER_100D__
typedef struct s_p1_rcv_cmd_testcond_end_tag {
	S_P1_CMD_HEADER		header;

	long				test_cond_file_size;
	long				test_cond_file_checksum;
	long				reserved1[2];
} S_P1_RCV_CMD_TESTCOND_END;
#else //COA_VER_100F~
typedef struct s_p1_rcv_cmd_testcond_end_tag {
	S_P1_CMD_HEADER		header;

	long				test_cond_file_size;
	long				test_cond_file_checksum;
	long				test_cond_time_file_size; //timeschedule
	long				test_cond_time_file_checksum; //timeschedule
} S_P1_RCV_CMD_TESTCOND_END;
#endif

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
//#ifdef __B_TYPE__
	int					count; //kjhw_120503 Vref x 2
//#endif
    unsigned long       shunt_value; //jhkw_200317
	S_P1_CALI_POINT		point;
} S_P1_CALI_TMP_COND;

typedef struct s_p1_rcv_cmd_cali_start_tag {
	S_P1_CMD_HEADER		header;
	S_P1_CALI_TMP_COND	tmpCond;
} S_P1_RCV_CMD_CALI_START;

typedef struct s_p1_rcv_cmd_cali_update_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_CALI_UPDATE;

//jhkw_200317s
typedef struct s_p1_rcv_cmd_cali_stop_tag {
	S_P1_CMD_HEADER		header;
	unsigned char		value;	//1 : stop
	unsigned char		ch;
	unsigned char		reserved[2];
} S_P1_RCV_CMD_CALI_STOP;
//jhkw_200317e

typedef struct s_p1_rcv_cmd_comm_check_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_COMM_CHECK;
//jhkw_200508s
typedef struct s_p1_rcv_cmd_comm_check_2_tag {
	S_P1_CMD_HEADER		header;
} S_P1_RCV_CMD_COMM_CHECK_2;
//jhkw_200508e

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

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
typedef struct s_p1_aux_set_data_tag {
	unsigned char		chNo;			//1base : machine channel number
	unsigned char		reserved1[3];
	short int			auxChNo;		//1base : aux channel number
	short int			auxType;		//0:temperature, 1:v
	char				name[MAX_AUX_NAME_SIZE];
	long				fault_upper;
	long				fault_lower;
	long				end_upper;
	long				end_lower;

	short int			function_div;
	short int			function_div2;
	short int			function_div3;
	short int			reserved2;
} S_P1_AUX_SET_DATA;
#else //COA_VER_100F~
typedef struct s_p1_aux_set_data_tag { //kjh_160610
	unsigned char		chNo;			//1base : machine channel number
	//unsigned char		reserved1[3];
	unsigned char		tableNo;		//auxType=2 0:Non, 1:Atype, 2:Btype, 3:Ctype
	unsigned char		reserved1[2];
	short int			auxChNo;		//1base : aux channel number
	short int			auxType;		//0:temperature, 1:v, 2:thermistor
	char				name[MAX_AUX_NAME_SIZE];
	long				fault_upper;
	long				fault_lower;
	long				end_upper;
	long				end_lower;

	short int			function_div;
	short int			function_div2;
	short int			function_div3;
	short int			reserved2;
} S_P1_AUX_SET_DATA;
#endif

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
	unsigned char		terminal_r; //0:open, 1:120ohm(default)
	unsigned char		crc_type; //0:non_iso_crc, 1:iso_crc(default)

	long				reserved2[2]; //kjg_180405_e

	short int			func_div[MAX_P1_CAN_FUNCTION]; //kjg_101220_s
	unsigned char		compare_type[MAX_P1_CAN_FUNCTION];
	unsigned char		data_type[MAX_P1_CAN_FUNCTION];
	float				value[MAX_P1_CAN_FUNCTION]; //kjg_101220_e
} S_P1_CAN_RECEIVE_COMMON_DATA;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__
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
} S_P1_CAN_RECEIVE_NORMAL_DATA;
#else //COA_VER_100C~
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
#endif

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
	unsigned char		terminal_r; //0:open, 1:120ohm(default)
	unsigned char		crc_type; //0:non_iso_crc, 1:iso_crc(default)

	long				reserved2; //kjg_180405_e

	short int			func_div[MAX_P1_CAN_FUNCTION];
	unsigned char		compare_type[MAX_P1_CAN_FUNCTION];
	unsigned char		data_type[MAX_P1_CAN_FUNCTION];
	float				value[MAX_P1_CAN_FUNCTION];
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
	unsigned char		reserved1[3];
} S_P1_RCV_CMD_DAQ_ISOLATION_REQUEST;

typedef struct s_p1_rcv_cmd_daq_isolation_request2_tag { //jhkw_150224
	S_P1_CMD_HEADER     header;

	unsigned char       iso;
	unsigned char       div_ch;
	unsigned char       reserved1[2];
} S_P1_RCV_CMD_DAQ_ISOLATION_REQUEST2;

typedef struct s_p1_rcv_cmd_out_mux_select_request_tag { //kjhw_151021
	S_P1_CMD_HEADER		header;

	unsigned char		out_mux; //0: all_off, 1:mux_a, 2:mux_b
	unsigned char		reserved1[3];
} S_P1_RCV_CMD_OUT_MUX_SELECT_REQUEST;

typedef struct s_p1_rcv_cmd_th_table_set_tag { //jhkw_191108
	S_P1_CMD_HEADER		header;

	unsigned char		th_table; //
	unsigned char		reserved1[3];
} S_P1_RCV_CMD_TH_TABLE_SET;

typedef struct s_p1_send_cmd_th_table_set_reply_tag { //jhkw_191108
	S_P1_CMD_HEADER		header;

	unsigned char		th_table; //
	unsigned char		reserved1[3];
} S_P1_SEND_CMD_TH_TABLE_SET_REPLY;

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

typedef struct s_p1_rcv_cmd_calimode_tag {    //jhkw_130119s
	S_P1_CMD_HEADER     header;

    unsigned char       mode;
    //0:display,  1:cali
    unsigned char       reserved1[3];
} S_P1_RCV_CMD_CALIMODE;  //jhkw_130119e

//jhkw_200317s
typedef struct s_p1_rcv_cmd_autocali_request_tag {
	S_P1_CMD_HEADER     header;

    unsigned char       mode;	//0:display,  1:cali
    unsigned char       measure_model;	//0:34401A, 1:VITREK_4700
    unsigned char       reserved[2];
} S_P1_RCV_CMD_AUTOCALI_REQUEST;

typedef struct s_p1_rcv_cmd_autocali_temp_request_tag {
	S_P1_CMD_HEADER     header;

    unsigned char       mode;	//0:none, 1:initial
    unsigned char       reserved[3];
} S_P1_RCV_CMD_AUTOCALI_TEMP_REQUEST;

typedef struct s_p1_send_cmd_autocali_temp_reply_tag {
	S_P1_CMD_HEADER		header;

	unsigned char		value; //0:ng 1:ok
	unsigned char		reserved1[3];
} S_P1_SEND_CMD_AUTOCALI_TEMP_REPLY;
//jhkw_200317e

typedef struct s_p1_rcv_cmd_load_type_set_tag {    //kjhw_130129s
	S_P1_CMD_HEADER     header;

    unsigned char       type;
    //0:EDLC,  1:cell
    unsigned char       reserved1[3];
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

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__ || defined __COA_VER_100F__
typedef struct s_p1_rcv_cmd_chamber_flag_tag { //kjg_101221
	S_P1_CMD_HEADER		header;

	unsigned char		chamber_control; //0:none, 1:pc_control
	unsigned char		chamber_standby; //0: no_standby 1:standby kjhw_141223
	unsigned char		reserved1[2];
} S_P1_RCV_CMD_CHAMBER_FLAG;
#else //COA_VER_100F01~ //kjhw_170906
typedef struct s_p1_rcv_cmd_chamber_flag_tag { //kjg_101221
	S_P1_CMD_HEADER		header;

	unsigned char		chamber_control; //0:none, 1:pc_control
	unsigned char		chamber_standby; //0: no_standby 1:standby kjhw_141223
	unsigned char		chiller_control; //0:none, 1:pc_control //kjhw_170906
	unsigned char		reserved1;
} S_P1_RCV_CMD_CHAMBER_FLAG;
#endif

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
	short int			reserved1;
	//short int			reserved1[2]; //kjh_160610
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

#ifdef __COA_VER_100B__
typedef struct s_p1_ch_data_tag {
	unsigned char		ch;
	unsigned char		state;
	unsigned char		stepType;
	unsigned char		stepMode;

	unsigned char		select;
	unsigned char		code;
	unsigned char		stepNo;
	unsigned char		grade;

	long				Vsens;
	long				Isens;
	long				charge_AmpareHour;
	long				discharge_AmpareHour;
	long				capacitance;
	long				watt;
	long				charge_WattHour;
	long				discharge_WattHour;
	unsigned long		runTime;
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

	unsigned long		cvTime;
	long				realDate;
	long				realClock;

	long				Vinput; //kjg_101102_s
	long				Vpower;
	long				Vbus; //kjg_101102_e

	unsigned char		chamber_control; //kjg_101220
	unsigned char		record_index;	//RECORD_T1~T5 1base
	unsigned char		reserved1[2];

	long				reserved2[6];
} S_P1_CH_DATA;
#else //COA_VER_100B2~
typedef struct s_p1_ch_data_tag {
	unsigned char		ch;
	unsigned char		state;
	unsigned char		stepType;
	unsigned char		stepMode;

	unsigned char		select;
	unsigned char		code;
	//unsigned char		stepNo;	//jhkw_200410
	short int			stepNo;	//jhkw_200410
	unsigned char		grade;
	unsigned char		reseved1[3]; //jhkw_200410

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
//20181219 KHK------------------------------------
	long				maxAuxTemp;
	long				minAuxTemp;
	long				devAuxTemp;
	long				avgAuxTemp;
	long				SOC;
	//long				reserved2;	shhw_230605 
//------------------------------------------------
	//shhw_230605s
	long 				dIsens;
	long			 	dMaxAuxV; 		//dMaxAuxV[2] -> dMaxAuxV			//shhw_230614s	
	long				dMinAuxV;		//dMinAuxV[2] -> dMinAuxV
	short int			dMaxAuxVChNo;	//dMaxAuxVChNo[2] -> dMaxAuxVChNo
	short int			dMinAuxVChNo;	//dMinAuxVChNo[2] -> dMinAuxVChNo	//shhw_230614e
	/*
	long				dMaxAuxV[2];	
	long				dMinAuxV[2];
	short int			dMaxAuxVChNo[2];	
	short int			dMinAuxVChNo[2];
	*/
	//shhw_230605e	
} S_P1_CH_DATA;
#endif

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

//jhkw_190830s
#ifdef __10MS__
typedef struct s_p1_send_cmd_ch_data4_tag {
	S_P1_CH_DATA		chData;
	S_P1_AUX_DATA		auxData[MAX_AUX_DATA];
	S_P1_CAN_DATA		canData[MAX_CAN_DATA];
} S_P1_SEND_CMD_CH_DATA4;

typedef struct s_p1_send_cmd_ch_data3_tag {
	S_P1_CMD_HEADER		header;
	short int			ch_count;
	short int			aux_count;
	short int			can_count;
	short int			reserved;

	S_P1_SEND_CMD_CH_DATA4	add_data[200];
} S_P1_SEND_CMD_CH_DATA3;
#endif
//jhkw_190830e

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
	//unsigned char		reserved1[7]; //jhkw_231127s
	unsigned char		ratioV; //0:uV,  1:mV, 2:V
	unsigned char		ratioI; //0:uA,  1:mA, 2:A
	unsigned char		ratioP; //0:Non, 1:mW, 2:W
	unsigned char		reserved[4]; //jhkw_231127e

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
//#ifdef __B_TYPE__
	int					count; //kjhw_120504 Vref x 2
//#endif
	unsigned char		range;
	unsigned char		setPointNum;
	unsigned char		checkPointNum;
	unsigned char		ch;
	long				setPointAD[MAX_CALI_POINT];
	long				setPointDVM[MAX_CALI_POINT];
	long				setPointDVM_Real[MAX_CALI_POINT]; //jhkw_200317
	long				checkPointAD[MAX_CALI_POINT];
	long				checkPointDVM[MAX_CALI_POINT];
	long				checkPointDVM_Real[MAX_CALI_POINT]; //jhkw_200317
} S_P1_CALI_NORMAL_RESULT;

typedef struct s_p1_cali_check_result_tag {
	int					type;
//#ifdef __B_TYPE__
	int					count; //jhkw_200317
//#endif
	unsigned char		range;
	unsigned char		reserved1;
	unsigned char		checkPointNum;
	unsigned char		ch;
	long				checkPointAD[MAX_CALI_POINT];
	long				checkPointDVM[MAX_CALI_POINT];
	long				checkPointDVM_Real[MAX_CALI_POINT];	//jhkw_200317
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
//jhkw_200508s
typedef struct s_p1_send_cmd_comm_check_reply_2_tag {
	S_P1_CMD_HEADER		header;
} S_P1_SEND_CMD_COMM_CHECK_REPLY_2;
//jhkw_200508e

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
	unsigned char		reserved1[3];
} S_P1_SEND_CMD_DAQ_ISOLATION_REPLY;

typedef struct s_p1_send_cmd_daq_isolation_reply2_tag { //jhkw_150224
	S_P1_CMD_HEADER     header;

	unsigned char       iso;
	unsigned char       div_ch;
	unsigned char       reserved1[2];
} S_P1_SEND_CMD_DAQ_ISOLATION_REPLY2;

typedef struct s_p1_send_cmd_out_mux_select_reply_tag { //kjhw_151021
	S_P1_CMD_HEADER     header;

	unsigned char       ch;
	unsigned char       fault; //0:NG, 1:OK
	unsigned char       setting;
	unsigned char       state;
	unsigned char       fault_flag;
	unsigned char       reserved[3];
} S_P1_SEND_CMD_OUT_MUX_SELECT_REPLY;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
typedef struct s_p1_misc_tag {
   	int    				network_socket1; //common socket
   	int    				network_socket2; //ch_data socket

	unsigned long		cmd_serial;

	int					CO_No;
	int					psSignal;
	int					chInGroup;
	int					chOffset;
	int					processPointer;

	long				net_time;
	long				net_time2;
	long				pause_time;	//jhkw_190503
	long				pause_time2; //jhkw_190503
	long				ping_time;
	long				ping_time2;
	long				sent_monitor_data_time;
	long				sent_monitor_data_time2;
	long				sent_real_time_request;

	unsigned char		network_port_type; //1:net_port 1ea, 2:net_port 2ea
	unsigned char		reserved1[3];

	char				write_pack_id[40];

	short int			rcv_test_step_count; //kjg_120430
	short int			rcv_test_pattern_count; //kjg_120430
} S_P1_MISC;
#else //COA_VER_100F~
typedef struct s_p1_misc_tag {
   	int    				network_socket1; //common socket
   	int    				network_socket2; //ch_data socket

	unsigned long		cmd_serial;

	int					CO_No;
	int					psSignal;
	int					chInGroup;
	int					chOffset;
	int					processPointer;

	long				net_time;
	long				net_time2;
	long				pause_time; //jhkw_190503
	long				pause_time2; //jhkw_190503
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
#endif

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
