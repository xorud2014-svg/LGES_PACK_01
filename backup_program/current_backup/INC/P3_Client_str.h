#ifndef __P3_CLIENT_STR_H__
#define __P3_CLIENT_STR_H__

#include "SysDefine.h"
#include "P3_Client_def.h"

typedef struct s_p3_config_tag {
	short int			groupId;
	short int			groupNo;

   	char				ipAddr[16];

	int					sendPort;
	int					receivePort;
	int					networkPort; //common port
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
} S_P3_CONFIG;

typedef struct s_p3_misc_tag {
  	int    				send_socket;
   	int    				receive_socket;
   	int    				network_socket; //common socket

	unsigned long		cmd_serial;

	int					CO_No;
	int					psSignal;
	int					chInGroup;
	int					chOffset;
	int					processPointer;

	long				net_time;
	long				net_time2;
	long				ping_time;
	long				ping_time2;
	long				sent_monitor_data_time;
	long				sent_monitor_data_time2;
	long				sent_real_time_request;
} S_P3_MISC;

typedef struct s_p3_test_config_tag {
	long				maxRefV;
	long				minRefV;
	long				maxRefI;
	long				minRefI;
	long				failDeltaV;
	long				failDeltaI;
	long				check_lowerI;
	long				check_lowerOCV;
	long				fail_charge_lower_dv;
} S_P3_TEST_CONFIG;

typedef struct s_p3_rcv_packet_tag {
	int					usedBufSize;

	int					rcvCount;
	int					rcvStartPoint[MAX_P3_RECV_PACKET_COUNT];
	int					rcvSize[MAX_P3_RECV_PACKET_COUNT];
	char				rcvPacketBuf[MAX_P3_RECV_PACKET_LENGTH];

	int					parseCount;
	int					parseStartPoint[MAX_P3_RECV_PACKET_COUNT];
} S_P3_RCV_PACKET;

typedef struct s_p3_rcv_command_tag {
	int					cmdBufSize;
	char				cmd[MAX_P3_RECV_PACKET_LENGTH];
	char				cmdBuf[MAX_P3_RECV_PACKET_LENGTH];
//	char				tmpBuf[MAX_P3_RECV_PACKET_LENGTH];
	int					cmdFail;
	int					cmdSize;

	int					rcvCmdIndex;
//	char				rcvCmd[MAX_P3_RECV_PACKET_LENGTH];
	unsigned char		rcvCmdCompleteFlag;
	unsigned char		rcvCmdRestFlag;
	unsigned char		reserved1[2];
} S_P3_RCV_COMMAND;

typedef struct s_p3_retry_data_tag {
	int					seqno;
	int					replyCmd;
	int					count;
	int					size;
	char				buf[MAX_P3_RECV_PACKET_LENGTH];
} S_P3_RETRY_DATA;

typedef struct s_p3_reply_tag {
	int					timer_run;
	unsigned long		time;
	unsigned long		time2;
	S_P3_RETRY_DATA		retry;
} S_P3_REPLY;

typedef struct s_p3_cmd_header_tag {
	unsigned long		cmd_id;
	unsigned long		cmd_serial;
	unsigned short		reserved1;
	unsigned short		reserved2;
	unsigned long		chFlag[2];
	unsigned long		body_size;
} S_P3_CMD_HEADER;
/*for_pack_cycler
typedef struct s_p3_test_cond_header_tag {
	unsigned char		totalStep;
	unsigned char		reserved1[3];
	long				reserved2[2];
} S_P3_TEST_COND_HEADER;

typedef struct s_p3_test_cond_safety_tag {
	long				faultLowerV;
	long				faultUpperV;
	long				faultLowerI;
	long				faultUpperI;
	long				faultLower_AmpareHour;
	long				faultUpper_AmpareHour;
	//kjg_w long				faultLower_Capacitance;
	//kjg_w long				faultUpper_Capacitance;
	long				faultLowerTemp; //kjg_w_s don't use
	long				faultUpperTemp;	//kjg_w_e
	long				reserved1[4];
} S_P3_TEST_COND_SAFETY;

typedef struct s_p3_test_step_header_tag {
	int					type; //kjg_w change(unsigned char)

	unsigned char		stepNo;
	unsigned char		mode;
	unsigned char		testEnd;
	unsigned char		subStep;

	unsigned char		reserved1; //kjg_090326 useSocFlag;
	unsigned char		patternIndex; //host program is reserved
	unsigned char		reserved2[2];
} S_P3_TEST_STEP_HEADER;

typedef struct s_p3_test_step_reference_tag {
	long				refV;	//pattern : refV_upper
	long				refI;	//pattern : refV_lower

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
	
	//kjg_090304 0x1008(4104)
	//Aux branch condition (step)
	long				Aux_V_Upper;
	long				Aux_V_Lower;
	long				Aux_Temp_Upper;
	long				Aux_Temp_Lower;

	unsigned char		Aux_V_Upper_Branch;
	unsigned char		Aux_V_Lower_Branch;
	unsigned char		Aux_Temp_Upper_Branch;
	unsigned char		Aux_Temp_Lower_Branch;

	//CAN branch condition (step)
	long				CAN_V_Upper;
	long				CAN_V_Lower;
	long				CAN_Temp_Upper;
	long				CAN_Temp_Lower;
	long				CAN_Soc_Upper;
	long				CAN_Soc_Lower;
	long				CAN_Fault;

	unsigned char		CAN_V_Upper_Branch;
	unsigned char		CAN_V_Lower_Branch;
	unsigned char		CAN_Temp_Upper_Branch;
	unsigned char		CAN_Temp_Lower_Branch;

	unsigned char		CAN_Soc_Upper_Branch;
	unsigned char		CAN_Soc_Lower_Branch;
	unsigned char		CAN_Fault_Branch;
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

	long				reserved2[2];
} S_P3_TEST_STEP_REFERENCE;

typedef struct s_p3_test_comp_cond_tag {
	long				lowerValue;
	long				upperValue;
	unsigned long		time;
} S_P3_TEST_COMP_COND;

typedef struct s_p3_test_delta_cond_tag {
	long				lowerValue;
	long				upperValue;
	unsigned long		time;
} S_P3_TEST_DELTA_COND;

typedef struct s_p3_test_record_cond_tag {
	unsigned long		deltaT;
	long				deltaV;
	long				deltaI;
	long				deltaTemp;
	long				deltaP;
	long				reserved;
} S_P3_TEST_RECORD_COND;

typedef struct s_p3_test_edlc_cond_tag {
	long				capacitanceV1;
	long				capacitanceV2;
	unsigned long		startT_Z;
	unsigned long		endT_Z;
	unsigned long		startT_LC;
	unsigned long		endT_LC;
} S_P3_TEST_EDLC_COND;

typedef struct s_p3_test_grade_step_tag {
	unsigned char		gradeCode;
	unsigned char		reserved1; //-> item
	short int			reserved2;
	long				lowerValue;	//equal and upper(lowerValue <= x)
	long				upperValue;	//only lower(upperValue > x)
} S_P3_TEST_GRADE_STEP;

typedef struct s_p3_test_grade_cond_tag {
	unsigned char		item;
	unsigned char		gradeStepCount;
	short int			reserved1; //->itemType

	S_P3_TEST_GRADE_STEP	gradeStep[MAX_P3_GRADE_STEP];
} S_P3_TEST_GRADE_COND;

typedef struct s_p3_test_cond_step_tag {
	S_P3_TEST_STEP_HEADER	header;
	S_P3_TEST_STEP_REFERENCE	reference[MAX_P3_SUB_STEP];
	S_P3_TEST_COMP_COND		compV[MAX_P3_COMP_POINT];
	S_P3_TEST_COMP_COND		compI[MAX_P3_COMP_POINT];
	S_P3_TEST_DELTA_COND	deltaV;
	S_P3_TEST_DELTA_COND	deltaI;
	S_P3_TEST_RECORD_COND	record;
	S_P3_TEST_EDLC_COND		edlc;
	S_P3_TEST_GRADE_COND	grade[MAX_P3_GRADE_ITEM];

	long				faultUpperV;
	long				faultLowerV;
	long				faultUpperI;
	long				faultLowerI;
	long				faultUpper_AmpareHour;
	long				faultLower_AmpareHour;
	//kjg_w long				faultUpper_Capacitance;
	//kjg_w long				faultLower_Capacitance;
	long				faultUpperZ;
	long				faultLowerZ;
	long				faultUpperTemp;
	long				faultLowerTemp;
	long				reserved1[2];
} S_P3_TEST_COND_STEP;

typedef struct s_p3_test_cond_pattern_tag {
	S_P3_TEST_STEP_HEADER	header;
	S_P3_TEST_STEP_REFERENCE	reference[MAX_P3_SUB_STEP];
	S_P3_TEST_RECORD_COND	record;

	long				faultUpperV;
	long				faultLowerV;
	long				faultUpperI;
	long				faultLowerI;
	long				faultUpper_AmpareHour;
	long				faultLower_AmpareHour;
	//kjg_w long				faultUpper_Capacitance;
	//kjg_w long				faultLower_Capacitance;
	long				faultUpperZ;
	long				faultLowerZ;
	long				faultUpperTemp;
	long				faultLowerTemp;
	long				reserved1[2];

	//insert pattern data
} S_P3_TEST_COND_PATTERN;

typedef struct s_p3_test_cond_pattern_data_tag {
	long				t_val;
	long				cmd_val;
} S_P3_TEST_COND_PATTERN_DATA;

typedef struct s_p3_test_condition_tag {
	S_P3_TEST_COND_HEADER	header;
	S_P3_TEST_COND_SAFETY	safety;
   	S_P3_TEST_COND_STEP	step[MAX_P3_STEP];
//	S_P3_TEST_COND_PATTERN	pattern[MAX_P3_STEP_PATTERN]; //kjg_w
//	S_P3_TEST_COND_PATTERN_DATA
//			pattern_data[MAX_P3_STEP_PATTERN][MAX_P3_PATTERN_DATA]; //kjg_w
	short int			stepCount;
	short int			reserved1;
} S_P3_TEST_CONDITION;
*/
typedef struct s_p3_test_cond_header_tag {
	unsigned char		totalStep;
	unsigned char		reserved1[3];
	long				reserved2[2];
} S_P3_TEST_COND_HEADER;

typedef struct s_p3_test_cond_safety_tag {
	long				faultLowerV;
	long				faultUpperV;
	long				faultLowerI;
	long				faultUpperI;
	long				faultLowerC;
	long				faultUpperC;
	long				faultLowerTemp;
	long				faultUpperTemp;
	long				reserved1[4];
} S_P3_TEST_COND_SAFETY;

typedef struct s_p3_test_step_header_tag {
	int					type;

	unsigned char		stepNo;
	unsigned char		mode;
	unsigned char		testEnd;
	unsigned char		subStep;

	unsigned char		useSocFlag;
	unsigned char		cycle_pause; //0:none, 1:pause(element_cycle count)
									 //only use at loop step
	unsigned char		reserved1[2];
} S_P3_TEST_STEP_HEADER;

typedef struct s_p3_test_step_reference_tag {
	long				refV; //x100
	long				refI; //x100
	long				refTemp; //x1000

	unsigned long		endT;
	long				endV;
	long				endI;
	long				endC;
	long				GotoCondition; //general_element_cycle_goto
	long				endTGoto;
	long				endVGoto;
	long				endIGoto;
	long				endCGoto;
	long				cycleCount; //general_element_cycle
	long				endDeltaV;
	unsigned short int	endSoc;
	unsigned short int	socCapStepNo;
	long				endWatt;
	long				endWattHour;
	long				startTemp; //x1000
	long				endTemp; //x1000
	unsigned short		gotoCycleCount; //general_multi_cycle

//	unsigned long		endT_CV;		//version 0x1006
//	long				endTCVGoto;		//version 0x1006

	unsigned short		reserved1;
} S_P3_TEST_STEP_REFERENCE;

typedef struct s_p3_test_comp_cond_tag {
	long				lowerValue;
	long				upperValue;
	unsigned long		time;
} S_P3_TEST_COMP_COND;

typedef struct s_p3_test_delta_cond_tag {
	long				lowerValue;
	long				upperValue;
	unsigned long		time;
} S_P3_TEST_DELTA_COND;

typedef struct s_p3_test_record_cond_tag {
	unsigned long		time;
	long				deltaV;
	long				deltaI;
	long				deltaT; //temperature
	long				deltaP;
	long				reserved1;
} S_P3_TEST_RECORD_COND;

typedef struct s_p3_test_edlc_cond_tag {
	long				capacitanceV1;
	long				capacitanceV2;
	unsigned long		startT_Z;		//for sbl RECORD_T1
	unsigned long		endT_Z;			//for sbl RECORD_T2
	unsigned long		startT_LC;		//for sbl RECORD_T3
	unsigned long		endT_LC;
} S_P3_TEST_EDLC_COND;

typedef struct s_p3_test_grade_step_tag {
	unsigned char		gradeCode;
	unsigned char		item;
	short int			reserved1;
	long				lowerValue;	//equal and upper(lowerValue <= x)
	long				upperValue;	//only lower(upperValue > x)
} S_P3_TEST_GRADE_STEP;

typedef struct s_p3_test_grade_cond_tag {
	unsigned char		item;
	unsigned char		gradeStepCount;
	short int			reserved1;

	S_P3_TEST_GRADE_STEP	gradeStep[MAX_P3_GRADE_STEP];
} S_P3_TEST_GRADE_COND;

typedef struct s_p3_test_cond_step_tag {
	S_P3_TEST_STEP_HEADER	header;
	S_P3_TEST_STEP_REFERENCE	reference[MAX_P3_SUB_STEP];
	S_P3_TEST_COMP_COND		compV[MAX_P3_COMP_POINT];
	S_P3_TEST_COMP_COND		compI[MAX_P3_COMP_POINT];
	S_P3_TEST_DELTA_COND	deltaV;
	S_P3_TEST_DELTA_COND	deltaI;
	S_P3_TEST_RECORD_COND	record;
	S_P3_TEST_EDLC_COND		edlc;
	S_P3_TEST_GRADE_COND	grade[MAX_P3_GRADE_ITEM];

	long				faultUpperV;
	long				faultLowerV;
	long				faultUpperI;
	long				faultLowerI;
	long				faultUpperC;
	long				faultLowerC;
	long				faultUpperZ;
	long				faultLowerZ;
	long				faultUpperTemp;
	long				faultLowerTemp;
	long				reserved1[2];
} S_P3_TEST_COND_STEP;
/*
typedef struct s_p3_test_cond_pattern_tag {
	S_P3_TEST_STEP_HEADER	header;
	S_P3_TEST_STEP_REFERENCE	reference[MAX_P3_SUB_STEP];
	S_P3_TEST_RECORD_COND	record;

	long				faultUpperV;
	long				faultLowerV;
	long				faultUpperI;
	long				faultLowerI;
	long				faultUpper_AmpareHour;
	long				faultLower_AmpareHour;
	//kjg_w long				faultUpper_Capacitance;
	//kjg_w long				faultLower_Capacitance;
	long				faultUpperZ;
	long				faultLowerZ;
	long				faultUpperTemp;
	long				faultLowerTemp;
	long				reserved1[2];

	//insert pattern data
} S_P3_TEST_COND_PATTERN;*/

typedef struct s_p3_test_cond_pattern_tag {
	long				stepNo;
	long				length;
	long				type;

	char				reserved1[4];
} S_P3_TEST_COND_PATTERN;

typedef struct s_p3_test_cond_pattern_data_tag {
	long				t_val;
	long				cmd_val[2];

	unsigned char		waveform_type; //0:rectangle, 1:triangle
	unsigned char		reserved1[3];
} S_P3_TEST_COND_PATTERN_DATA;

typedef struct s_p3_test_condition_tag {
	S_P3_TEST_COND_HEADER	header;
	S_P3_TEST_COND_SAFETY	safety;
   	S_P3_TEST_COND_STEP	step[MAX_P3_STEP];

	short int			stepCount;
	short int			reserved1;
} S_P3_TEST_CONDITION;

typedef struct s_p3_response_tag {
	int					cmd;
	int					code;
} S_P3_RESPONSE;

typedef struct s_p3_rcv_cmd_module_info_request_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_MODULE_INFO_REQUEST;

typedef struct s_p3_rcv_cmd_module_set_data_tag {
	S_P3_CMD_HEADER		header;
	unsigned char		connection_retry;
	unsigned char		line_mode;
	unsigned char		control_mode;
	unsigned char		working_mode;
	unsigned int		auto_report_interval;
	unsigned int		data_save_interval;
	unsigned int		reserved1[16];
} S_P3_RCV_CMD_MODULE_SET_DATA;

typedef struct s_p3_cmd_control_tag {
	long				stepNo;
	long				cycleNo;
	long				reserved1[6];
} S_P3_CMD_CONTROL;

typedef struct s_p3_rcv_cmd_run_tag {
	S_P3_CMD_HEADER		header;
	S_P3_CMD_CONTROL	control;
} S_P3_RCV_CMD_RUN;

typedef struct s_p3_rcv_cmd_stop_tag {
	S_P3_CMD_HEADER		header;
	S_P3_CMD_CONTROL	control;
} S_P3_RCV_CMD_STOP;

typedef struct s_p3_rcv_cmd_pause_tag {
	S_P3_CMD_HEADER		header;
	S_P3_CMD_CONTROL	control;
} S_P3_RCV_CMD_PAUSE;

typedef struct s_p3_rcv_cmd_continue_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_CONTINUE;

typedef struct s_p3_rcv_cmd_next_step_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_NEXT_STEP;

typedef struct s_p3_rcv_cmd_chamber_flag_tag {
	S_P3_CMD_HEADER		header;

	unsigned char		chamber_control; //0:none, 1:pc_control
	unsigned char		reserved1[3];
} S_P3_RCV_CMD_CHAMBER_FLAG;

typedef struct s_p3_rcv_cmd_chamber_continue_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_CHAMBER_CONTINUE;

typedef struct s_p3_rcv_cmd_cable_check_tag {
	S_P3_CMD_HEADER		header;

	unsigned char		reserved1[4];
} S_P3_RCV_CMD_CABLE_CHECK;

typedef struct s_p3_rcv_cmd_cell_check_tag {
	S_P3_CMD_HEADER		header;

	unsigned char		reserved1[4];
} S_P3_RCV_CMD_CELL_CHECK;

typedef struct s_p3_rcv_cmd_cycle_continue_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_CYCLE_CONTINUE;

typedef struct s_p3_rcv_cmd_init_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_INIT;

typedef struct s_p3_rcv_cmd_reset_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_RESET;

typedef struct s_p3_rcv_cmd_testcond_start_tag {
	S_P3_CMD_HEADER		header;
	S_P3_TEST_COND_HEADER	testCondHeader;
} S_P3_RCV_CMD_TESTCOND_START;

typedef struct s_p3_rcv_cmd_testcond_safety_tag {
	S_P3_CMD_HEADER		header;
	S_P3_TEST_COND_SAFETY	safety;
} S_P3_RCV_CMD_TESTCOND_SAFETY;

typedef struct s_p3_rcv_cmd_testcond_step_tag {
	S_P3_CMD_HEADER		header;
	S_P3_TEST_COND_STEP	testCondStep;
} S_P3_RCV_CMD_TESTCOND_STEP;

typedef struct s_p3_rcv_cmd_testcond_end_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_TESTCOND_END;

typedef struct s_p3_rcv_cmd_step_cond_request_tag {
	S_P3_CMD_HEADER		header;
	long				stepNo;
	long				reserved1[3];
} S_P3_RCV_CMD_STEP_COND_REQUEST;

typedef struct s_p3_rcv_cmd_step_cond_update_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_STEP_COND_UPDATE;

typedef struct s_p3_rcv_cmd_safety_cond_request_tag {
	S_P3_CMD_HEADER		header;
	long				stepNo;
	long				reserved1[3];
} S_P3_RCV_CMD_SAFETY_COND_REQUEST;

typedef struct s_p3_rcv_cmd_safety_cond_update_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_SAFETY_COND_UPDATE;

typedef struct s_p3_rcv_cmd_reset_reserved_cmd_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_RESET_RESERVED_CMD;

typedef struct s_p3_rcv_cmd_testcond_pattern_tag {
	S_P3_CMD_HEADER		header;
	S_P3_TEST_COND_PATTERN	testCondPattern;
} S_P3_RCV_CMD_TESTCOND_PATTERN;

typedef struct s_p3_rcv_cmd_response_tag {
	S_P3_CMD_HEADER		header;
	S_P3_RESPONSE		response;
} S_P3_RCV_CMD_RESPONSE;

typedef struct s_p3_rcv_cmd_set_measure_data_tag {
	S_P3_CMD_HEADER		header;

	unsigned short int	id;
	unsigned short int	type;
	long				data;
	long				reserved1;
} S_P3_RCV_CMD_SET_MEASURE_DATA;

typedef struct s_p3_rcv_cmd_cali_meter_connect_tag {
	S_P3_CMD_HEADER		header;
	long				type;
} S_P3_RCV_CMD_CALI_METER_CONNECT;

typedef struct s_p3_cali_point_tag {
	unsigned char		setPointNum;
	unsigned char		checkPointNum;
	unsigned char		reserved1[2];
	long				setPoint[MAX_CALI_POINT];
	long				checkPoint[MAX_CALI_POINT];
} S_P3_CALI_POINT;

typedef struct s_p3_cali_tmp_cond_tag {
	int					type;
	int					range;
	int					mode;
	S_P3_CALI_POINT		point;
} S_P3_CALI_TMP_COND;

typedef struct s_p3_rcv_cmd_cali_start_tag {
	S_P3_CMD_HEADER		header;
	S_P3_CALI_TMP_COND	tmpCond;
} S_P3_RCV_CMD_CALI_START;

typedef struct s_p3_rcv_cmd_cali_update_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_CALI_UPDATE;

typedef struct s_p3_rcv_cmd_sensor_limit_set_tag {
	S_P3_CMD_HEADER		header;

	long				smokeUseFlag;
	long				smokeUpper;
} S_P3_RCV_CMD_SENSOR_LIMIT_SET;

typedef struct s_p3_rcv_cmd_comm_check_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_COMM_CHECK;

typedef struct s_p3_rcv_cmd_comm_check_reply_tag {
	S_P3_CMD_HEADER		header;
	char				result[2];
	char				sent_cmd[4];
	//kjg_171219_w char				reserved1[2];
} S_P3_RCV_CMD_COMM_CHECK_REPLY;

typedef struct s_p3_ch_attribute_tag {
	unsigned char		chNo_master;	//1base
	unsigned char		chNo_slave[3];	//1base
	unsigned char		opType;			//0:independent, 1:parallel
	unsigned char		reserved1[3];
} S_P3_CH_ATTRIBUTE;

typedef struct s_p3_rcv_cmd_ch_attribute_set_tag {
	S_P3_CMD_HEADER		header;
	S_P3_CH_ATTRIBUTE	attr[P3_MAX_CH_PER_MODULE];
} S_P3_RCV_CMD_CH_ATTRIBUTE_SET;

typedef struct s_p3_rcv_cmd_ch_attribute_request_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_CH_ATTRIBUTE_REQUEST;

typedef struct s_p3_aux_set_data_tag {
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
	short int			reserved2;
	long				reserved3;
} S_P3_AUX_SET_DATA;

typedef struct s_p3_rcv_cmd_aux_set_tag {
	S_P3_CMD_HEADER		header;
	S_P3_AUX_SET_DATA	auxSetData[MAX_AUX_DATA];
} S_P3_RCV_CMD_AUX_SET;

typedef struct s_p3_can_receive_common_data_tag {
	unsigned char		can_baudrate;	//0:125K, 1:250K, 2:500K, 3:1M, 4:User
	unsigned char		extended_id;	//0:unused, 1:used
	unsigned char		reserved1[2];

	long				controller_canID;
	long				mask[2];
	long				filter[6];

	long				reserved2[3];
} S_P3_CAN_RECEIVE_COMMON_DATA;

typedef struct s_p3_can_receive_normal_data_tag {
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
	unsigned char		user_control;
	unsigned char		tmp_user_control;
} S_P3_CAN_RECEIVE_NORMAL_DATA;

typedef struct s_p3_can_receive_set_data_tag {
	S_P3_CAN_RECEIVE_COMMON_DATA
		commonData[P3_MAX_CH_PER_MODULE][MAX_CAN_TYPE];
	S_P3_CAN_RECEIVE_NORMAL_DATA
		normalData[P3_MAX_CH_PER_MODULE][MAX_CAN_DATA];
} S_P3_CAN_RECEIVE_SET_DATA;

typedef struct s_p3_rcv_cmd_can_recieve_set_tag {
	S_P3_CMD_HEADER		header;
	S_P3_CAN_RECEIVE_SET_DATA	canReceiveSetData;
} S_P3_RCV_CMD_CAN_RECEIVE_SET;

typedef struct s_p3_can_transmit_common_data_tag {
	unsigned char		can_baudrate;	//0:125K, 1:250K, 2:500K, 3:1M, 4:User
	unsigned char		extended_id;	//0:unused, 1:used
	unsigned char		reserved1[2];

	long				controller_canID;
	long				reserved2[2];
} S_P3_CAN_TRANSMIT_COMMON_DATA;

typedef struct s_p3_can_transmit_normal_data_tag {
	unsigned char		canType;		//0:unused, 1:master, 2:slave
	unsigned char		byte_order;		//0:intel, 1:motolora
	unsigned char		data_type;	//0:unsigned, 1:signed, 2:float, 3:string
	unsigned char		reserved1;

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
	unsigned char		user_control;		//only sbc used
	unsigned char		tmp_user_control;	//only sbc used
} S_P3_CAN_TRANSMIT_NORMAL_DATA;

typedef struct s_p3_can_transmit_set_data_tag {
	S_P3_CAN_TRANSMIT_COMMON_DATA
		commonData[P3_MAX_CH_PER_MODULE][MAX_CAN_TYPE];
	S_P3_CAN_TRANSMIT_NORMAL_DATA
		normalData[P3_MAX_CH_PER_MODULE][MAX_CAN_DATA];
} S_P3_CAN_TRANSMIT_SET_DATA;

typedef struct s_p3_rcv_cmd_can_transmit_set_tag {
	S_P3_CMD_HEADER		header;
	S_P3_CAN_TRANSMIT_SET_DATA	canTransmitSetData;
} S_P3_RCV_CMD_CAN_TRANSMIT_SET;

typedef struct s_p3_rcv_cmd_aux_info_request_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_AUX_INFO_REQUEST;

typedef struct s_p3_rcv_cmd_can_receive_info_request_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_CAN_RECEIVE_INFO_REQUEST;

typedef struct s_p3_rcv_cmd_real_time_reply_tag {
	S_P3_CMD_HEADER		header;
	char				real_time[24];
} S_P3_RCV_CMD_REAL_TIME_REPLY;

typedef struct s_p3_rcv_cmd_bms_communication_request_tag {
	S_P3_CMD_HEADER		header;

	short int			function_div;
	short int			reserved1;
} S_P3_RCV_CMD_BMS_COMM_REQUEST;

typedef struct s_p3_rcv_cmd_daq_isolation_request_tag {
	S_P3_CMD_HEADER		header;

	unsigned char		iso;
	unsigned char		reserved1[3];
} S_P3_RCV_CMD_DAQ_ISOLATION_REQUEST;

typedef struct s_p3_rcv_cmd_can_transmit_info_request_tag {
	S_P3_CMD_HEADER		header;
} S_P3_RCV_CMD_CAN_TRANSMIT_INFO_REQUEST;

typedef struct s_p3_send_cmd_ch_attribute_reply_tag {
	S_P3_CMD_HEADER		header;
	S_P3_CH_ATTRIBUTE	attr[P3_MAX_CH_PER_MODULE];
} S_P3_SEND_CMD_CH_ATTRIBUTE_REPLY;

typedef struct s_p3_send_cmd_aux_info_reply_tag {
	S_P3_CMD_HEADER		header;
	short int			installedTemp;
	short int			installedAuxV;
	short int			reserved1[2];
	S_P3_AUX_SET_DATA	auxSetData[MAX_AUX_DATA];
} S_P3_SEND_CMD_AUX_INFO_REPLY;

typedef struct s_p3_send_cmd_can_receive_info_reply_tag {
	S_P3_CMD_HEADER		header;
	short int			canReceiveDataCount[P3_MAX_CH_PER_MODULE];
	S_P3_CAN_RECEIVE_SET_DATA	canReceiveSetData;
} S_P3_SEND_CMD_CAN_RECEIVE_INFO_REPLY;

typedef struct s_p3_send_cmd_can_transmit_info_reply_tag {
	S_P3_CMD_HEADER		header;
	short int			canTransmitDataCount[P3_MAX_CH_PER_MODULE];
	S_P3_CAN_TRANSMIT_SET_DATA	canTransmitSetData;
} S_P3_SEND_CMD_CAN_TRANSMIT_INFO_REPLY;

typedef struct s_p3_ch_data_tag {
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
	long				capacity;
	long				watt;
	long				wattHour;
	unsigned long		runTime;
	unsigned long		totalRunTime;
	long				z;
	long				temp[3];

	unsigned char		chamber_control;
	unsigned char		record_index;	//RECORD_T1~T5 1base
	unsigned char		reserved1[2];
	long				reserved2;

	unsigned char		reservedCmd;	//0:normal, 1:stop, 2:pause
	unsigned char		virRangeReservedNo;
	unsigned short		gotoCycleCount;

	unsigned long		totalCycle;
	unsigned long		elementCycle;

	long				avgV;
	long				avgI;
	long				resultIndex;
/*
	long				integral_AmpareHour;
	long				integral_WattHour;
	long				charge_AmpareHour;
	long				charge_WattHour;
	long				discharge_AmpareHour;
	long				discharge_WattHour;
	long				cvTime;
	long				reserved1[3];
*/
//	short int			auxDataCount;
//	short int			canReceiveDataCount;

//	unsigned long		accCycle[MAX_P3_ACC_CYCLE];
//	unsigned long		multiCycle[MAX_P3_MULTI_CYCLE];


//	long				realDate;
//	long				realClock;
} S_P3_CH_DATA;

typedef struct s_p3_ch_data2_tag {
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
	unsigned char		reserved1[3];

	short int			auxDataCount;
	short int			canReceiveDataCount;

	unsigned long		totalCycle;
	unsigned long		elementCycle;
	unsigned long		accCycle[MAX_P3_ACC_CYCLE];
	unsigned long		multiCycle[MAX_P3_MULTI_CYCLE];

	long				avgV;
	long				avgI;
	long				resultIndex;

	unsigned long		cvTime;
	long				realDate;
	long				realClock;
} S_P3_CH_DATA2;

typedef struct s_p3_send_cmd_ch_data_tag {
	S_P3_CMD_HEADER		header;
	S_P3_CH_DATA		chData[P3_MAX_CH_PER_MODULE];
} S_P3_SEND_CMD_CH_DATA;

typedef struct s_p3_send_cmd_ch_record_data_tag {
	S_P3_CMD_HEADER		header;
	S_P3_CH_DATA		chData[P3_MAX_CH_PER_MODULE];
} S_P3_SEND_CMD_CH_RECORD_DATA;

typedef struct s_p3_send_cmd_cable_check_reply_tag {
	S_P3_CMD_HEADER		header;

	unsigned char		code[P3_MAX_CH_PER_MODULE]; //0:ng, 1:ok
	unsigned char		reserved1[P3_MAX_CH_PER_MODULE];
} S_P3_SEND_CMD_CABLE_CHECK_REPLY;

typedef struct s_p3_send_cmd_cell_check_reply_tag {
	S_P3_CMD_HEADER		header;

	unsigned char		code[P3_MAX_CH_PER_MODULE]; //0:ng, 1:ok
	unsigned char		reserved1[P3_MAX_CH_PER_MODULE];
} S_P3_SEND_CMD_CELL_CHECK_REPLY;

typedef struct s_p3_aux_data_tag {
	short int			auxChNo;		//1base
	short int			auxType;		//0:temperature, 1:v
	//short int			function_div;
	//short int			reserved1;
	long				val;
} S_P3_AUX_DATA;

typedef union u_p3_can_val_tag {
	unsigned long		ul_val[2];
	long				l_val[2];
	float				f_val[2];
	unsigned char		uc_val[8];
	char				c_val[8];
	double				d_val[1];
} U_P3_CAN_VAL;

typedef struct s_p3_can_data_tag {
	unsigned char		canType;		//0:unused, 1:master, 2:slave
	unsigned char		data_type;	//0:unsigned, 1:signed, 2:float, 3:string
	short int			function_div;

	U_P3_CAN_VAL		val;
} S_P3_CAN_DATA;

typedef struct s_p3_send_cmd_ch_data2_tag {
	S_P3_CMD_HEADER		header;
	S_P3_CH_DATA2		chData;
	S_P3_AUX_DATA		auxData[MAX_AUX_DATA];
	S_P3_CAN_DATA		canData[MAX_CAN_DATA];
} S_P3_SEND_CMD_CH_DATA2;
/*
typedef struct s_p3_ch_pulse_val_tag {
	long				runTime;
	long				Vsens;
	long				Isens;
	long				capacity;
	long				wattHour;
} S_P3_CH_PULSE_VAL;

typedef struct s_p3_send_cmd_ch_pulse_data_tag {
	S_P3_CMD_HEADER		header;

	long				totalCycle;
	long				stepNo;
	long				dataCount;

	S_P3_CH_PULSE_VAL	val[MAX_PULSE_MSG];
} S_P3_SEND_CMD_CH_PULSE_DATA;

typedef struct s_p3_ch_pulse_data_tag {
	long				totalCycle;
	long				stepNo;
	long				dataCount;

	S_PULSE_MSG_VAL		val[MAX_PULSE_MSG];
} S_P3_CH_PULSE_DATA;
*/
typedef struct s_p3_module_info_tag {
	unsigned int		group_id;
	unsigned int		systemType;
	unsigned int		protocol_version;
	char				modelName[128];
	unsigned int		osVersion;
	unsigned short int	voltage_range;
	unsigned short int	current_range;
	unsigned int		voltage_spec[5];
	unsigned int		current_spec[5];
	unsigned char		reserved1[8];

	unsigned short int	installedBd;
	unsigned short int	chPerBd;
	unsigned int		installedCh;
	unsigned int		totalJig;
	unsigned int		BdinJig[16];
	int					reserved2[4];	
} S_P3_MODULE_INFO;

typedef struct s_p3_send_cmd_module_info_reply_tag {
	S_P3_CMD_HEADER		header;
	S_P3_MODULE_INFO	md_info;
} S_P3_SEND_CMD_MODULE_INFO_REPLY;

typedef struct s_p3_send_cmd_response_tag {
	S_P3_CMD_HEADER		header;
	S_P3_RESPONSE		response;
} S_P3_SEND_CMD_RESPONSE;

typedef struct s_p3_send_cmd_meter_connect_reply_tag {
	S_P3_CMD_HEADER		header;
	long				state;
} S_P3_SEND_CMD_METER_CONNECT_REPLY;

typedef struct s_p3_send_cmd_cali_start_reply_tag {
	S_P3_CMD_HEADER		header;
	S_P3_RESPONSE		response;
} S_P3_SEND_CMD_CALI_START_REPLY;

typedef struct s_p3_cali_normal_result_tag {
	int					type;
	unsigned char		range;
	unsigned char		setPointNum;
	unsigned char		checkPointNum;
	unsigned char		ch;
	long				setPointAD[MAX_CALI_POINT];
	long				setPointDVM[MAX_CALI_POINT];
	long				checkPointAD[MAX_CALI_POINT];
	long				checkPointDVM[MAX_CALI_POINT];
} S_P3_CALI_NORMAL_RESULT;

typedef struct s_p3_cali_check_result_tag {
	int					type;
	unsigned char		range;
	unsigned char		reserved1;
	unsigned char		checkPointNum;
	unsigned char		ch;
	long				checkPointAD[MAX_CALI_POINT];
	long				checkPointDVM[MAX_CALI_POINT];
} S_P3_CALI_CHECK_RESULT;

typedef struct s_p3_send_cmd_cali_normal_result_tag {
	S_P3_CMD_HEADER		header;
	S_P3_CALI_NORMAL_RESULT	result;
} S_P3_SEND_CMD_CALI_NORMAL_RESULT;

typedef struct s_p3_send_cmd_cali_check_result_tag {
	S_P3_CMD_HEADER		header;
	S_P3_CALI_CHECK_RESULT	result;
} S_P3_SEND_CMD_CALI_CHECK_RESULT;

typedef struct s_p3_send_cmd_comm_check_reply_tag {
	S_P3_CMD_HEADER		header;
} S_P3_SEND_CMD_COMM_CHECK_REPLY;

typedef struct s_p3_send_cmd_comm_check_tag {
	S_P3_CMD_HEADER		header;
} S_P3_SEND_CMD_COMM_CHECK;

typedef struct s_p3_send_cmd_emg_status_tag {
	S_P3_CMD_HEADER		header;
	long				code;
	long				val;
} S_P3_SEND_CMD_EMG_STATUS;

typedef struct s_p3_send_cmd_step_cond_reply_tag {
	S_P3_CMD_HEADER		header;
	S_P3_TEST_COND_STEP	testCondStep;
} S_P3_SEND_CMD_STEP_COND_REPLY;

typedef struct s_p3_send_cmd_safety_cond_reply_tag {
	S_P3_CMD_HEADER		header;
	S_P3_TEST_COND_SAFETY	safety;
} S_P3_SEND_CMD_SAFETY_COND_REPLY;

typedef struct s_p3_send_cmd_real_time_request_tag {
	S_P3_CMD_HEADER		header;
} S_P3_SEND_CMD_REAL_TIME_REQUEST;

typedef struct s_p3_send_cmd_bms_communication_reply_tag {
	S_P3_CMD_HEADER		header;

	short int			function_div;
	short int			reserved1;
} S_P3_SEND_CMD_BMS_COMM_REPLY;

typedef struct s_p3_send_cmd_bms_communication_end_tag {
	S_P3_CMD_HEADER		header;

	short int			function_div;
	short int			reserved1;
} S_P3_SEND_CMD_BMS_COMM_END;

typedef struct s_p3_send_cmd_daq_isolation_reply_tag {
	S_P3_CMD_HEADER		header;

	unsigned char		iso;
	unsigned char		reserved1[3];
} S_P3_SEND_CMD_DAQ_ISOLATION_REPLY;

typedef struct s_p3_send_cmd_set_measure_data_tag {
	S_P3_CMD_HEADER		header;

	unsigned short int	id;
	unsigned short int	type;
	long				data;
	long				reserved1;
} S_P3_SEND_CMD_SET_MEASURE_DATA;
#endif
