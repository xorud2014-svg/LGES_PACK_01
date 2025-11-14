#ifndef __P2_CLIENT_STR_H__
#define __P2_CLIENT_STR_H__

#include "SysDefine.h"
#include "P2_Client_def.h"

typedef struct s_p2_array1_tag {
	short int			index;
	short int			bd;
	short int			ch;
	short int			reserved1;
} S_P2_ARRAY1;

typedef struct s_p2_config_data_tag {
	unsigned char		vAutoCal;		//voltage auto cali. flag-default:1
	unsigned char		iAutoCal;		//current auto cali. flag-default:0
	short int			huntingCount;	//count of hunting check-default:3

	long				vHuntingLevel;	//voltage hunting level(mV)-default:150
	long				iHuntingLevel;	//current hunting level(mA)-default:180
	long				stableTime;		//stabilization time(sec)-default:10
	long				dVstableTime;	//stable time for deltaV(sec)-default:60
	long				vRefNGLevel;	//V Ref IC fail level(digit)-default:300
	long				iRefNGLevel;	//I Ref IC fail level(digit)-default:800
	long				reserved1[2];
} S_P2_CONFIG_DATA;

typedef struct s_p2_config_tag {
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
	unsigned char		workMode;
	unsigned char		groupControlMode;

	long				send_monitor_data_interval;
	long				send_save_data_interval;
	long				send_sensor_data_interval;

	S_P2_ARRAY1			ChArray1[MAX_CH_256]; //hardware number
	short int			ChArray2[MAX_CH_256];
	S_P2_ARRAY1			ChArray3[MAX_CH_256]; //monitor idx -> hardware

	unsigned char		autoProcess;
	unsigned char		trayCodeReadType;
	unsigned char		useTempLimitFlag;
	unsigned char		useGasLimitFlag;
	long				maxTempLimit;
	long				maxGasLimit;
} S_P2_CONFIG;

typedef struct s_p2_misc_tag {
  	int    				send_socket;
   	int    				receive_socket;
   	int    				network_socket; //common socket

	unsigned long		cmd_serial;

	unsigned char		state;
	unsigned char		code;
	unsigned char		tmpState;
	unsigned char		tmpCode;

	unsigned short		tmpSensorState[8];

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
	long				sent_sensor_data_time;
	long				sent_sensor_data_time2;
	long				sent_chamber_data_time;
	long				sent_chamber_data_time2;

	short int			stepCount;
	short int			gradeCount;
	char				test_serial_no[32];

	unsigned char		nonCell[P2_MAX_CH_PER_GROUP];

	unsigned char		jobModel;
	unsigned char		parallel_count;
	unsigned char		reserved1[2];
} S_P2_MISC;

typedef struct s_p2_test_config_tag {
	long				maxRefV;
	long				minRefV;
	long				maxRefI;
	long				minRefI;
	long				failDeltaV;
	long				failDeltaI;
	long				check_lowerI;
	long				check_lowerOCV;
	long				fail_charge_lower_dv;
} S_P2_TEST_CONFIG;

typedef struct s_p2_rcv_packet_tag {
	int					usedBufSize;

	int					rcvCount;
	int					rcvStartPoint[MAX_P2_RECV_PACKET_COUNT];
	int					rcvSize[MAX_P2_RECV_PACKET_COUNT];
	char				rcvPacketBuf[MAX_P2_RECV_PACKET_LENGTH];

	int					parseCount;
	int					parseStartPoint[MAX_P2_RECV_PACKET_COUNT];
} S_P2_RCV_PACKET;

typedef struct s_p2_rcv_command_tag {
	int					cmdBufSize;
	char				cmd[MAX_P2_RECV_PACKET_LENGTH];
	char				cmdBuf[MAX_P2_RECV_PACKET_LENGTH];
	char				tmpBuf[MAX_P2_RECV_PACKET_LENGTH];
	int					cmdFail;
	int					cmdSize;

	int					rcvCmdIndex;
	char				rcvCmd[MAX_P2_RECV_PACKET_LENGTH];
	unsigned char		rcvCmdCompleteFlag;
	unsigned char		rcvCmdRestFlag;
	unsigned char		reserved1[2];
} S_P2_RCV_COMMAND;

typedef struct s_p2_retry_data_tag {
	int					seqno;
	int					replyCmd;
	int					count;
	int					size;
	char				buf[MAX_P2_RECV_PACKET_LENGTH];
} S_P2_RETRY_DATA;

typedef struct s_p2_reply_tag {
	int					timer_run;
	unsigned long		time;
	unsigned long		time2;
	S_P2_RETRY_DATA		retry;
} S_P2_REPLY;

typedef struct s_p2_cmd_header_tag {
	unsigned int		packet_id;
	unsigned short		group_id;
	unsigned short		ch;
	unsigned int		cmd_id;
	unsigned int		cmd_size;
} S_P2_CMD_HEADER;

typedef struct s_p2_test_header_tag {
	char				testSerial[24];
	char				resultFileName[256];
	unsigned char		totalStep;
	unsigned char		totalGrade;
	unsigned char		reserved1[2];
} S_P2_TEST_HEADER;

typedef struct s_p2_test_precheck_tag {
	long				upperOCV;
	long				lowerOCV;
	long				refV;
	long				refI;
	unsigned long		endTime;
	long				deltaV;
	long				maxFault;
	unsigned char		compFlag;
	unsigned char		autoProcessingYN;
	unsigned char		reserved1[2];
} S_P2_TEST_PRECHECK;

typedef struct s_p2_test_precheck2_tag {
	long				upperOCV;
	long				lowerOCV;
	long				refV;
	long				refI;
	unsigned long		endTime;
	long				deltaVmax;
	long				deltaVmin;
	long				current_judge_ratio;
	long				reverse_voltage;
	long				detect_voltage;
	long				maxFault;
	unsigned char		compFlag;
	unsigned char		autoProcessingYN;
	unsigned char		upper_temp; //0~255degreeC
	unsigned char		lower_temp; //0~255degreeC
} S_P2_TEST_PRECHECK2;

typedef struct s_p2_test_common_safety_tag {
	long				charge_limit_voltage;
	long				charge_limit_capacity;
	long				charge_check_voltage;
	long				charge_check_time;

	long				discharge_limit_voltage;
	long				discharge_limit_time;

	long				over_current_limit;
	long				over_charge_voltage;
	long				charge_lower_current_limit;
	long				discharge_lower_limit_capacity;

	long				comp_value1[3][5]; //t, value1_max, value1_min
	long				comp_value2[3][5]; //t, value2_max, value2_min
} S_P2_TEST_COMMON_SAFETY;

typedef struct s_p2_test_ng_condition_tag {
	unsigned char		stepNo;
	unsigned char		reserved1[3];

	//50A charge, discharge -> Ah
	//50A ocv -> voltage
	//400A charge, discharge -> Power
	//400A ocv -> voltage
	long				value1[2]; //data 0:min, 1:max
	long				value2[2]; //avg 0:min, 1:max
	long				reserved2[2];
	long				reserved3[2];
} S_P2_TEST_NG_COND;

typedef struct s_p2_test_step_header_tag {
	unsigned char		stepNo;
	unsigned char		type;
	unsigned char		mode;
	unsigned char		gradeFlag;
	long				reserved1;
} S_P2_TEST_STEP_HEADER;

typedef struct s_p2_test_step_ocv_tag {
	long				upperV;
	long				lowerV;
	unsigned char		reportCode[P2_RESULT_ITEM_NO];
} S_P2_TEST_STEP_OCV;

typedef struct s_p2_test_step_charge_tag {
	long				refV;
	long				refI;
	unsigned long		endTime;
	long				endV;
	long				endI;
	long				endCapacity;
	long				endDV;
	long				endDI;

	unsigned char		useActualCapacity;
	unsigned char		reserved1;
	unsigned short int	socRate;

	long				upperV;
	long				lowerV;
	long				upperI;
	long				lowerI;
	long				upperCapacity;
	long				lowerCapacity;

	unsigned long		compTimeV[P2_COMP_POINT];
	long				compLowerV[P2_COMP_POINT];
	long				compUpperV[P2_COMP_POINT];
	unsigned long		compTimeI[P2_COMP_POINT];
	long				compLowerI[P2_COMP_POINT];
	long				compUpperI[P2_COMP_POINT];

	unsigned long		deltaTimeV;
	long				deltaV;
	unsigned long		deltaTimeI;
	long				deltaI;
	long				recordDeltaTime;// This is use to flag if 0 don't record but 1 must be record of below condition
	long				recordDeltaV;
	long				recordDeltaI;
	long				recordDeltaT;
	long				delta_V1;// This is parameter to capacity
	long				delta_V2;// This is parameter to capacity
//	long				tmv;
//	unsigned long		rTime;
	unsigned char		reportCode[P2_RESULT_ITEM_NO];
//	unsigned long		restTime;
} S_P2_TEST_STEP_CHARGE;

typedef struct s_p2_test_step_discharge_tag {
	long				refV;
	long				refI;
	unsigned long		endTime;
	long				endV;
	long				endI;
	long				endCapacity;
	long				endDV;
	long				endDI;

	unsigned char		useActualCapacity;
	unsigned char		reserved1;
	unsigned short int	socRate;

	long				upperV;
	long				lowerV;
	long				upperI;
	long				lowerI;
	long				upperCapacity;
	long				lowerCapacity;

	unsigned long		compTimeV[P2_COMP_POINT];
	long				compLowerV[P2_COMP_POINT];
	long				compUpperV[P2_COMP_POINT];
	unsigned long		compTimeI[P2_COMP_POINT];
	long				compLowerI[P2_COMP_POINT];
	long				compUpperI[P2_COMP_POINT];

	unsigned long		deltaTimeV;
	long				deltaV;
	unsigned long		deltaTimeI;
	long				deltaI;
	long				recordDeltaTime;// This is use to flag if 0 don't record but 1 must be record of below condition
	long				recordDeltaV;
	long				recordDeltaI;
	long				recordDeltaT;
	long				delta_V1;// This is parameter to capacity
	long				delta_V2;// This is parameter to capacity
//	long				tmv;
//	unsigned long		rTime;
	unsigned char		reportCode[P2_RESULT_ITEM_NO];
//	unsigned long		restTime;
} S_P2_TEST_STEP_DISCHARGE;

typedef struct s_p2_test_step_z_tag {
	long				refV;
	long				refI;
	unsigned long		endTime;
	long				startVref;
	long				startIref;
	unsigned long		startTime;
	long				upperV;
	long				lowerV;
	long				upperI;
	long				lowerI;
	long				upperZ;
	long				lowerZ;
	long				recordDeltaTime;// This is use to flag if 0 don't record but 1 must be record of below condition
	long				recordDeltaV;
	long				recordDeltaI;
	long				recordDeltaT;
	unsigned char		reportCode[P2_RESULT_ITEM_NO];
//	unsigned long		restTime;
} S_P2_TEST_STEP_Z;

typedef struct s_p2_test_step_rest_tag {
	unsigned long		endTime;
	long				upperV;
	long				lowerV;

	//long				jigUp;
	unsigned char		jigUp_Flag; //kjg_090807
	//unsigned char		long_time_rest_Flag;
	unsigned char		reserved1[3];

	long				recordDeltaTime;// This is use to flag if 0 don't record but 1 must be record of below condition
	long				recordDeltaV;
	long				recordDeltaI;
	long				recordDeltaT;
	unsigned char		reportCode[P2_RESULT_ITEM_NO];
} S_P2_TEST_STEP_REST;

typedef struct s_p2_test_step_long_time_rest_tag {
	unsigned long		endTime;
	long				upperV;
	long				lowerV;

	//long				jigUp;
	unsigned char		jigUp_Flag; //kjg_090807
	//unsigned char		long_time_rest_Flag;
	unsigned char		reserved1[3];

	long				recordDeltaTime;// This is use to flag if 0 don't record but 1 must be record of below condition
	long				recordDeltaV;
	long				recordDeltaI;
	long				recordDeltaT;
	unsigned char		reportCode[P2_RESULT_ITEM_NO];
} S_P2_TEST_STEP_LONG_TIME_REST;

typedef struct s_p2_test_step_end_tag {
} S_P2_TEST_STEP_END;

typedef struct s_p2_test_grade_header_tag {
	unsigned char		stepNo;		//zero base
	unsigned char		totalGrade;	//one base
	short int			item;
} S_P2_TEST_GRADE_HEADER;

typedef struct s_p2_test_grade_tag {
	unsigned char		code;
	unsigned char		reserved1;
	short int			reserved2;
	long				value1;
	long				value2;
} S_P2_TEST_GRADE;

typedef struct s_p2_test_condition_tag {
	S_P2_TEST_HEADER	header;
	S_P2_TEST_PRECHECK	precheck;
	S_P2_TEST_PRECHECK2	precheck2; //for 0x1002
	S_P2_TEST_COMMON_SAFETY	common_safety; //for 0x1002
	S_P2_TEST_NG_COND	ng_cond[MAX_P2_STEP]; //for 0x1002
	S_P2_TEST_STEP_HEADER	stepHeader[MAX_P2_STEP];
	S_P2_TEST_STEP_OCV	ocv[MAX_P2_STEP];
	S_P2_TEST_STEP_CHARGE	charge[MAX_P2_STEP];
	S_P2_TEST_STEP_DISCHARGE	discharge[MAX_P2_STEP];
	S_P2_TEST_STEP_Z	z[MAX_P2_STEP];
	S_P2_TEST_STEP_REST	rest[MAX_P2_STEP];
	S_P2_TEST_STEP_LONG_TIME_REST	long_time_rest[MAX_P2_STEP];
	S_P2_TEST_STEP_END	end;
	S_P2_TEST_GRADE_HEADER	gradeHeader[MAX_P2_STEP];
	S_P2_TEST_GRADE		grade[MAX_P2_STEP][MAX_P2_GRADE];
} S_P2_TEST_CONDITION;

typedef struct s_p2_rcv_cmd_test_header_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_HEADER	testHeader;
} S_P2_RCV_CMD_TEST_HEADER;

typedef struct s_p2_rcv_cmd_test_precheck_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_PRECHECK	precheck;
} S_P2_RCV_CMD_TEST_PRECHECK;

typedef struct s_p2_rcv_cmd_test_precheck2_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_PRECHECK2	precheck2;
} S_P2_RCV_CMD_TEST_PRECHECK2;

typedef struct s_p2_rcv_cmd_test_common_safety_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_COMMON_SAFETY	common_safety;
} S_P2_RCV_CMD_TEST_COMMON_SAFETY;

typedef struct s_p2_rcv_cmd_test_ng_condition_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_NG_COND	ng_cond;
} S_P2_RCV_CMD_TEST_NG_COND;

typedef struct s_p2_rcv_cmd_test_step_header_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_STEP_HEADER	stepHeader;
} S_P2_RCV_CMD_TEST_STEP_HEADER;

typedef struct s_p2_rcv_cmd_test_step_ocv_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_STEP_HEADER	stepHeader;
	S_P2_TEST_STEP_OCV	ocv;
} S_P2_RCV_CMD_TEST_STEP_OCV;

typedef struct s_p2_rcv_cmd_test_step_charge_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_STEP_HEADER	stepHeader;
	S_P2_TEST_STEP_CHARGE	charge;
} S_P2_RCV_CMD_TEST_STEP_CHARGE;

typedef struct s_p2_rcv_cmd_test_step_discharge_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_STEP_HEADER	stepHeader;
	S_P2_TEST_STEP_DISCHARGE	discharge;
} S_P2_RCV_CMD_TEST_STEP_DISCHARGE;

typedef struct s_p2_rcv_cmd_test_step_rest_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_STEP_HEADER	stepHeader;
	S_P2_TEST_STEP_REST	rest;
} S_P2_RCV_CMD_TEST_STEP_REST;

typedef struct s_p2_rcv_cmd_test_step_long_time_rest_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_STEP_HEADER	stepHeader;
	S_P2_TEST_STEP_LONG_TIME_REST	long_time_rest;
} S_P2_RCV_CMD_TEST_STEP_LONG_TIME_REST;

typedef struct s_p2_rcv_cmd_test_step_z_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_STEP_HEADER	stepHeader;
	S_P2_TEST_STEP_Z	z;
} S_P2_RCV_CMD_TEST_STEP_Z;

typedef struct s_p2_rcv_cmd_test_step_end_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_STEP_HEADER	stepHeader;
	S_P2_TEST_STEP_END	end;
} S_P2_RCV_CMD_TEST_STEP_END;

typedef struct s_p2_rcv_cmd_test_grade_header_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_GRADE_HEADER	gradeHeader;
} S_P2_RCV_CMD_TEST_GRADE_HEADER;

typedef struct s_p2_rcv_cmd_test_info_request_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_TEST_INFO_REQUEST;

typedef struct s_p2_tray_data_tag {
	unsigned char		cellCode[P2_MAX_CH_PER_GROUP];
	unsigned char		cellUpdate[P2_MAX_CH_PER_GROUP];
} S_P2_TRAY_DATA;

typedef struct s_p2_rcv_cmd_tray_data_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TRAY_DATA		tray_data;
} S_P2_RCV_CMD_TRAY_DATA;

typedef struct s_p2_rcv_cmd_version_request_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_VERSION_REQUEST;

typedef struct	s_p2_rcv_cnd_set_line_mode_tag {
	S_P2_CMD_HEADER		header;
	unsigned int	onlineMode;
	unsigned int	controlMode;
	unsigned char	reserved[4];
} S_P2_RCV_CMD_SET_LINE_MODE;

typedef struct s_p2_rcv_cmd_line_mode_request_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_LINE_MODE_REQUEST;

typedef struct s_p2_rcv_cmd_set_auto_report_tag {
	S_P2_CMD_HEADER		header;
	unsigned int		interval;
	int					reserved1;
} S_P2_RCV_CMD_SET_AUTO_REPORT;

typedef struct s_p2_rcv_cmd_check_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_CHECK;

typedef struct s_p2_rcv_cmd_run_tag {
	S_P2_CMD_HEADER		header;
	char				test_serial_no[32];
	unsigned long		useFlag;
	int					inputCellNo[16];
	unsigned char		code[P2_MAX_CH_PER_MODULE];
} S_P2_RCV_CMD_RUN;

typedef struct s_p2_rcv_cmd_stop_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_STOP;

typedef struct s_p2_rcv_cmd_pause_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_PAUSE;

typedef struct s_p2_rcv_cmd_continue_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_CONTINUE;

typedef struct s_p2_rcv_cmd_next_step_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_NEXT_STEP;

typedef struct s_p2_rcv_cmd_clear_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_CLEAR;

typedef struct s_p2_rcv_cmd_reset_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_RESET;

typedef struct s_p2_rcv_cmd_response_tag {
	S_P2_CMD_HEADER		header;
	int					sent_cmd_id;
	int					code;
} S_P2_RCV_CMD_RESPONSE;

/*kjg_110702 typedef struct s_p2_cali_set_tag {
	unsigned char		range;
	unsigned char		caliPointNum;
	unsigned char		reserved1[2];
	long				caliPoint[MAX_CALI_POINT];
} S_P2_CALI_SET_POINT;

typedef struct s_p2_cali_point_tag {
	unsigned char		range;
	unsigned char		caliPointNum;
	unsigned char		caliCheckPointNum;
	unsigned char		reserved1;
	long				caliPoint[MAX_CALI_POINT];
	long				caliCheckPoint[MAX_CALI_CHECK_POINT];
} S_P2_CALI_POINT;

typedef struct s_p2_rcv_cmd_cali_set_tag {
	S_P2_CMD_HEADER		header;
	S_P2_CALI_SET_POINT	set_point[MAX_TYPE][MAX_RANGE];
} S_P2_RCV_CMD_CALI_SET_POINT;

typedef struct s_p2_rcv_cmd_cali_voltage_tag {
	S_P2_CMD_HEADER		header;
	S_P2_CALI_POINT		cali_point;
} S_P2_RCV_CMD_CALI_VOLTAGE;

typedef struct s_p2_rcv_cmd_cali_current_tag {
	S_P2_CMD_HEADER		header;
	S_P2_CALI_POINT		cali_point;
} S_P2_RCV_CMD_CALI_CURRENT;

typedef struct s_p2_rcv_cmd_cali_update_tag {
	S_P2_CMD_HEADER		header;
	long				ch;
} S_P2_RCV_CMD_CALI_UPDATE; */

typedef struct s_p2_rcv_cmd_cali_meter_connect_tag { //kjg_110712
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_CALI_METER_CONNECT;

typedef struct s_p2_cali_point_tag {
	unsigned char		cali_point_num;
	unsigned char		cali_check_point_num;
	unsigned char		reserved1[2];

	long				cali_point[MAX_CALI_POINT];
	long				cali_check_point[MAX_CALI_CHECK_POINT];
} S_P2_CALI_POINT;

typedef struct s_p2_set_cali_point_tag {
	unsigned char		cali_division;	//0:main_da, 1:ch
	unsigned char		cali_multi;		//0:sigle, 1:multi
	unsigned char		trayType;		//cali_tray:0, measure_tray:1
	unsigned char		parallel;		//0:independent, 1:parallel

	unsigned char		v_range_num;
	unsigned char		i_range_num;
	unsigned char		reserved2[2];

	S_P2_CALI_POINT		v_cali_point[MAX_RANGE];
	S_P2_CALI_POINT		i_cali_point[MAX_RANGE];
} S_P2_SET_CALI_POINT;

typedef struct s_p2_rcv_cmd_set_cali_point_tag {
	S_P2_CMD_HEADER		header;

	S_P2_SET_CALI_POINT	set_point;
} S_P2_RCV_CMD_SET_CALI_POINT;

typedef struct s_p2_rcv_cmd_get_main_cali_point_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_GET_MAIN_CALI_POINT;

typedef struct s_p2_send_cmd_get_main_cali_point_reply_tag {
	S_P2_CMD_HEADER		header;

	S_P2_SET_CALI_POINT	set_point;
} S_P2_SEND_CMD_GET_MAIN_CALI_POINT_REPLY;

typedef struct s_p2_rcv_cmd_get_ch_cali_point_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_GET_CH_CALI_POINT;

typedef struct s_p2_send_cmd_get_ch_cali_point_reply_tag {
	S_P2_CMD_HEADER		header;

	S_P2_SET_CALI_POINT	set_point;
} S_P2_SEND_CMD_GET_CH_CALI_POINT_REPLY;

typedef struct s_p2_rcv_cmd_cali_start_tag {
	S_P2_CMD_HEADER		header;

	unsigned char		cali_division;	//0:main_da, 1:ch
	unsigned char		cali_multi;		//0:single, 1:multi
	unsigned char		trayType;		//cali_tray:0, measure_tray:1
	unsigned char		parallel;		//0:independent, 1:parallel

	int					cali_type;		//0:v, 1:i
	int					cali_board_no;	//main_bd_no 0:all_board, 1base
	int					cali_range;		//0:all_range, 1~4:select_range

	unsigned char		cali_flag[64];	//0:none, 1:execute
} S_P2_RCV_CMD_CALI_START;

typedef struct s_p2_rcv_cmd_cali_stop_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_CALI_STOP;

typedef struct s_p2_rcv_cmd_cali_pause_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_CALI_PAUSE;

typedef struct s_p2_rcv_cmd_cali_resume_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_CALI_RESUME;

typedef struct s_p2_send_cmd_cali_end_tag {
	S_P2_CMD_HEADER		header;

	unsigned char		cali_division;	//0:main_da, 1:ch
	unsigned char		cali_multi;		//0:single, 1:multi
	unsigned char		trayType;		//cali_tray:0, measure_tray:1
	unsigned char		parallel;		//0:independent, 1:parallel

	int					cali_type;		//0:v, 1:i
	int					cali_board_no;	//main_bd_no 0:all_board, 1base
	int					cali_range;		//0:all_range, 1~4:select_range

	unsigned char		cali_flag[64];	//0:none, 1:execute
} S_P2_SEND_CMD_CALI_END;

typedef struct s_p2_cali_data_tag {
	unsigned char		cali_point_num;
	unsigned char		cali_check_point_num;
	unsigned char		reserved1[2];

	long				cali_set_point[MAX_CALI_POINT];
	long				cali_ad_point[MAX_CALI_POINT];
	long				cali_dvm_point[MAX_CALI_POINT];

	long				cali_set_check_point[MAX_CALI_CHECK_POINT];
	long				cali_ad_check_point[MAX_CALI_CHECK_POINT];
	long				cali_dvm_check_point[MAX_CALI_CHECK_POINT];
} S_P2_CALI_DATA;

typedef struct s_p2_send_cmd_cali_data_tag {
	S_P2_CMD_HEADER		header;

	unsigned char		cali_division;	//0:main_da, 1:ch
	unsigned char		cali_multi;		//0:single, 1:multi
	unsigned char		trayType;		//cali_tray:0, measure_tray:1
	unsigned char		parallel;		//0:independent, 1:parallel

	int					cali_type;		//0:v, 1:i
	int					cali_board_no;	//main_bd_no 0:all_board, 1base
	int					monitor_ch;		//1base
	int					hw_ch;			//1base(board_ch_no)
	int					cali_range;		//0:all_range, 1~4:select_range

	int					cali_set_point_index;	//0base
	long				cali_ad_point;			//uV/uA
	long				cali_dvm_point;			//uV/uA

	unsigned char		reserved2[4];
} S_P2_SEND_CMD_CALI_DATA;

typedef struct s_p2_rcv_cmd_cali_check_start_tag {
	S_P2_CMD_HEADER		header;

	unsigned char		cali_division;	//0:main_da, 1:ch
	unsigned char		cali_multi;		//0:single, 1:multi
	unsigned char		trayType;		//cali_tray:0, measure_tray:1
	unsigned char		parallel;		//0:independent, 1:parallel

	int					cali_type;		//0:v, 1:i
	int					cali_board_no;	//main_bd_no 0:all_board, 1base
	int					cali_range;		//0:all_range, 1~4:select_range

	unsigned char		cali_flag[64];	//0:none, 1:execute
} S_P2_RCV_CMD_CALI_CHECK_START;

typedef struct s_p2_send_cmd_cali_check_end_tag {
	S_P2_CMD_HEADER		header;

	unsigned char		cali_division;	//0:main_da, 1:ch
	unsigned char		cali_multi;		//0:single, 1:multi
	unsigned char		trayType;		//cali_tray:0, measure_tray:1
	unsigned char		parallel;		//0:independent, 1:parallel

	int					cali_type;		//0:v, 1:i
	int					cali_board_no;	//main_bd_no 0:all_board, 1base
	int					cali_range;		//0:all_range, 1~4:select_range

	unsigned char		cali_flag[64];	//0:none, 1:execute
} S_P2_SEND_CMD_CALI_CHECK_END;

typedef struct s_p2_cali_check_data_tag {
	unsigned char		cali_check_point_num;
	unsigned char		reserved1[3];

	long				cali_set_check_point[MAX_CALI_CHECK_POINT];
	long				cali_ad_check_point[MAX_CALI_CHECK_POINT];
	long				cali_dvm_check_point[MAX_CALI_POINT];
} S_P2_CALI_CHECK_DATA;

typedef struct s_p2_send_cmd_cali_check_data_tag {
	S_P2_CMD_HEADER		header;

	unsigned char		cali_division;	//0:main_da, 1:ch
	unsigned char		cali_multi;		//0:single, 1:multi
	unsigned char		trayType;		//cali_tray:0, measure_tray:1
	unsigned char		parallel;		//0:independent, 1:parallel

	int					cali_type;		//0:v, 1:i
	int					cali_board_no;	//main_bd_no 0:all_board, 1base
	int					monitor_ch;		//1base
	int					hw_ch;			//1base(board_ch_no)
	int					cali_range;		//0:all_range, 1~4:select_range

	int					cali_set_point_index;	//0base
	long				cali_ad_point;			//uV/uA
	long				cali_dvm_point;			//uV/uA

	unsigned char		reserved2[4];
} S_P2_SEND_CMD_CALI_CHECK_DATA;

typedef struct s_p2_rcv_cmd_cali_update_tag {
	S_P2_CMD_HEADER		header;

	unsigned char		cali_division;	//0:main_da, 1:ch
	unsigned char		cali_multi;		//0:single, 1:multi
	unsigned char		trayType;		//cali_tray:0, measure_tray:1
	unsigned char		parallel;		//0:independent, 1:parallel

	int					cali_type;		//0:v, 1:i
	int					cali_board_no;	//main_bd_no 0:all_board, 1base
	int					cali_range;		//0:all_range, 1~4:select_range

	unsigned char		cali_flag[64];	//0:none, 1:execute
} S_P2_RCV_CMD_CALI_UPDATE;

typedef struct s_p2_rcv_cmd_real_measure_start_tag {
	S_P2_CMD_HEADER		header;

	int					cali_type;	//0:v, 1:i
	unsigned char		monitor_ch[P2_MAX_CH_PER_MODULE];	//0:skip, 1:measure

	unsigned short int	recordTime;
	unsigned char		reserved1[2];
} S_P2_RCV_CMD_REAL_MEASURE_START;

typedef struct s_p2_rcv_cmd_real_measure_stop_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_REAL_MEASURE_STOP;

typedef struct s_p2_send_cmd_real_measure_data_tag {
	S_P2_CMD_HEADER		header;

	int					cali_type;		//0:v, 1:i
	int					cali_board_no;	//main_bd_no 1base
	int					monitor_ch;		//1base
	int					hw_ch;			//1base
	int					measure_range;	//0:all_range, 1~4:select_range

	long				set_value;		//uV/uA
	long				ad_value;		//uV/uA
	long				dvm_value;		//uV/uA
	
	unsigned char		reserved1[4];
} S_P2_SEND_CMD_REAL_MEASURE_DATA;

typedef struct s_p2_send_cmd_real_measure_end_tag {
	S_P2_CMD_HEADER		header;
} S_P2_SEND_CMD_REAL_MEASURE_END;

typedef struct s_p2_hw_map_tag {
	unsigned short int	mon;	//1base
	unsigned short int	bd;		//1base
	unsigned short int	ch;		//1base
	unsigned short int	reserved1;
} S_P2_HW_MAP;

typedef struct s_p2_send_cmd_hw_map_req_rpy_tag {
	S_P2_CMD_HEADER		header;

	S_P2_HW_MAP			map[P2_MAX_CH_PER_MODULE];
} S_P2_SEND_CMD_HW_MAP_REQ_RPY;

typedef struct s_p2_rcv_cmd_hw_map_req_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_HW_MAP_REQ;

typedef struct s_p2_rcv_cmd_ch_attribute_set_tag {
	S_P2_CMD_HEADER		header;

	unsigned char		parallel_count; //1:independent, 2, 3, 4
	unsigned char		reserved1[3];
} S_P2_RCV_CMD_CH_ATTRIBUTE_SET;

typedef struct s_p2_send_cmd_ch_attribute_set_reply_tag {
	S_P2_CMD_HEADER		header;

	unsigned char		state;	//0:ng, 1:ok
	unsigned char		reserved1[3];
} S_P2_SEND_CMD_CH_ATTRIBUTE_SET_REPLY;

typedef struct s_p2_jig_temp_set_data_tag {
	unsigned short int	curJigAvgTemp;
	unsigned short int	checkTempTime;

	unsigned short int	targetTemp;
	unsigned char		useJigTempSetData;
	unsigned char		reserved1;
} S_P2_JIG_TEMP_SET_DATA;

typedef struct s_p2_rcv_cmd_jig_temp_set_data_tag {
	S_P2_CMD_HEADER		header;

	S_P2_JIG_TEMP_SET_DATA	jigTempSetData;
} S_P2_RCV_CMD_JIG_TEMP_SET_DATA;

typedef struct s_p2_rcv_cmd_set_group_tag {
	S_P2_CMD_HEADER		header;

	unsigned char		netConnectApproval;
	unsigned char		nvRamFlag;
	unsigned char		useTemp;
	unsigned char		useGas;

	long				maxTemp;
	long				maxGas;
	unsigned int		autoReportInterval;

	unsigned char		trayReadType;
	unsigned char		reserved1;
	short int			reserved2;

	long				vHighLimit; //kjg_w
	long				iHighLimit; //kjg_w
	long				reserved3[8];
} S_P2_RCV_CMD_SET_GROUP;

typedef struct s_p2_digital_in_tag {
	unsigned char		data[8];
} S_P2_DIGITAL_IN;

typedef struct s_p2_digital_out_tag {
	unsigned char		data[8];
} S_P2_DIGITAL_OUT;

typedef struct s_p2_ref_data_tag {
	long				CaliData;
	char				CaliTime[32]; // 2006/02/17/14:38:00
	long				CurrData;
	char				CurrTime[32]; // 2006/02/17/14:38:00
	long				MinData; //after power on
	char				MinTime[32]; // 2006/02/17/14:38:00
	long				MaxData; //after power on
	char				MaxTime[32]; // 2006/02/17/14:38:00
} S_P2_REF_DATA;

typedef struct s_p2_ref_da_data_tag {
	S_P2_REF_DATA		vPlus;
	S_P2_REF_DATA		vZero;
	S_P2_REF_DATA		vMinus;
	S_P2_REF_DATA		iPlus;
	S_P2_REF_DATA		iZero;
	S_P2_REF_DATA		iMinus;
} S_P2_REF_AD_DATA;

typedef struct s_p2_ref_ic_data_tag {
	S_P2_REF_AD_DATA	data[8];
	long				reserved1[4];
} S_P2_REF_IC_DATA;

typedef struct s_p2_host_time_tag {
	unsigned short		year;
	unsigned short		month;
	unsigned short		day;
	unsigned short		hour;
	unsigned short		minute;
	unsigned short		second;
} S_P2_HOST_TIME;

typedef struct s_p2_sensor_set_tag {
	unsigned char		useSensorFlag[MAX_P2_SENSOR_CH];
	long				maxLimit;
	unsigned char		limitFlag;
	unsigned char		reserved1;
	unsigned short		reserved2;
} S_P2_SENSOR_SET;

typedef struct s_p2_sensor_limit_tag {
	unsigned short	warning1;
	unsigned short	warning2;
	S_P2_SENSOR_SET		sensorSet1;
	S_P2_SENSOR_SET		sensorSet2;
	long				reserved1;
} S_P2_SENSOR_LIMIT;

typedef struct s_p2_rcv_cmd_set_sensor_limit_tag {
	S_P2_CMD_HEADER		header;
	S_P2_SENSOR_LIMIT	sensor_limit;
} S_P2_RCV_CMD_SET_SENSOR_LIMIT;

typedef struct s_p2_rcv_cmd_config_request_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_CONFIG_REQUEST;

typedef struct s_p2_rcv_cmd_set_config_tag {
	S_P2_CMD_HEADER		header;
	S_P2_CONFIG_DATA	configData;
} S_P2_RCV_CMD_SET_CONFIG;

typedef struct s_p2_rcv_cmd_set_use_stkcrane_tag {
	S_P2_CMD_HEADER		header;
	unsigned int		useStkCrane;
} S_P2_RCV_CMD_SET_USE_STKCRANE;

typedef struct s_p2_chamber_set_tag {
	unsigned char		type;
						//0:CHMABER_STOP, 1:CHAMBER_RUN, 2:CHAMBER_SET
						//3:CHAMBER_DOOR_OPEN, 4:CHAMBER_DOOR_CLOSE
	unsigned char		reserved1[3];

	short int			set_temp; //100 -> 1.0degree
	short int			set_humidity; //100 -> 1.0percent
	long				set_time; //1 -> 1sec
	long				reserved2;
} S_P2_CHAMBER_SET;
	
typedef struct s_p2_rcv_cmd_chamber_set_tag {
	S_P2_CMD_HEADER		header;
	S_P2_CHAMBER_SET	chamber_set;
} S_P2_RCV_CMD_CHAMBER_SET;

typedef struct s_p2_rcv_cmd_job_change_set_tag {
	S_P2_CMD_HEADER		header;

	unsigned char		jobModel; //0:none, 1~8
	unsigned char		reserved1[3];
} S_P2_RCV_CMD_JOB_CHANGE_SET;

typedef struct s_p2_rcv_cmd_comm_check_tag {
	S_P2_CMD_HEADER		header;
} S_P2_RCV_CMD_COMM_CHECK;

typedef struct s_p2_rcv_cmd_comm_check_reply_tag {
	S_P2_CMD_HEADER		header;
	char				result[4];
	char				sended_cmd[4];
} S_P2_RCV_CMD_COMM_CHECK_REPLY;

typedef struct s_p2_rcv_cmd_user_cmd_tag {
	S_P2_CMD_HEADER		header;
	int					cmd;
	int					data;
	unsigned char		reserved[8];
} S_P2_RCV_CMD_USER_CMD;

typedef struct s_p2_send_cmd_version_data_tag {
	S_P2_CMD_HEADER		header;
	int					moduleNo;
	int					versionNo;
	unsigned char		sbcType;
	unsigned char		parallel_count; //1:independent, 2, 3, 4
	unsigned char		reserved1[2];
	
	unsigned short		installedBd;
	unsigned short		chPerBd;
	int					groupNo;
	int					chInGroup;
	unsigned short		trayType;
	unsigned short		totalTrayNo;
	unsigned short		chInTray[MAX_TRAY_16];
} S_P2_SEND_CMD_VERSION_DATA;

typedef struct s_p2_send_cmd_line_mode_data_tag {
	S_P2_CMD_HEADER		header;
	int					onlineMode;
	int					controlMode;
	int					reserved1;
} S_P2_SEND_CMD_LINE_MODE_DATA;

typedef struct s_p2_send_cmd_test_info_data_tag {
	S_P2_CMD_HEADER		header;
	S_P2_TEST_HEADER	testHeader;
} S_P2_SEND_CMD_TEST_INFO_DATA;

typedef struct s_p2_send_cmd_response_tag {
	S_P2_CMD_HEADER		header;
	int					received_cmd_id;
	int					code;
} S_P2_SEND_CMD_RESPONSE;

typedef struct s_p2_send_cmd_trouble_code_tag {
	S_P2_CMD_HEADER		header;
	int					code;
	int					chNo;
} S_P2_SEND_CMD_TROUBLE_CODE;

typedef struct s_p2_send_cmd_switch_data_tag {
	S_P2_CMD_HEADER		header;
	unsigned char		switch1;
	unsigned char		reserved1[3];
} S_P2_SEND_CMD_SWITCH_DATA;

typedef struct s_p2_sensor_data_tag {
	long				value;
	long				reserved1;
} S_P2_SENSOR_DATA;

typedef struct s_p2_send_cmd_sensor_data_tag {
	S_P2_CMD_HEADER		header;
	S_P2_SENSOR_DATA	sensorData1[MAX_P2_SENSOR_CH];
	S_P2_SENSOR_DATA	sensorData2[MAX_P2_SENSOR_CH];
} S_P2_SEND_CMD_SENSOR_DATA;

typedef struct s_p2_step_end_header_tag {
	unsigned short		stepNo;
	unsigned short 		reserved;
	long				Data1[64];
	long				Data2[64];
} S_P2_STEP_END_HEADER;

typedef struct s_p2_ch_data_tag {
	unsigned short		ch;
	unsigned short		state;
	unsigned char		grade;
	unsigned char		code;
	unsigned short		stepNo;
	unsigned long		runTime;
	unsigned long		totalRunTime;
	long				Vsens;
	long				Isens;
	long				watt;
	long				wattHour;
	long				capacity;
	long				z;
} S_P2_CH_DATA;

typedef struct s_p2_ch_data2_tag {
	unsigned short		ch;
	unsigned short		state;
	unsigned char		grade;
	unsigned char		code;
	unsigned short		stepNo;
	unsigned long		runTime;
	unsigned long		totalRunTime;
	long				Vsens;
	long				Isens;
	long				watt;
	long				wattHour;
	long				capacity;
	long				z;
	unsigned long		ccTime;
} S_P2_CH_DATA2;

typedef struct s_p2_send_cmd_group_state_tag {
	S_P2_CMD_HEADER		header;
	unsigned short		groupState;
	unsigned short		code;
	unsigned short		sensorState[8];
						//[4] => jig sensor 16bits, [5] => jig sensor 16bits
} S_P2_SEND_CMD_GROUP_STATE;

typedef struct s_p2_send_cmd_check_data_tag {
	S_P2_CMD_HEADER		header;
	unsigned short		normalChNo;
	unsigned short		reserved;
	unsigned char		normalChCode[P2_MAX_CH_PER_MODULE]; // This field is 0 or 1 : 0 -> bad cell, 1 normal cell
} S_P2_SEND_CMD_CHECK_DATA;

typedef struct s_p2_send_cmd_check_data2_tag {
	S_P2_CMD_HEADER		header;
	unsigned short		normalChNo;
	unsigned short		reserved;
	unsigned char		normalChCode[P2_MAX_CH_PER_MODULE]; // This field is 0 or 1 : 0 -> bad cell, 1 normal cell
	unsigned char		code[P2_MAX_CH_PER_MODULE];
} S_P2_SEND_CMD_CHECK_DATA2;

typedef struct s_p2_send_cmd_meter_connect_reply_tag {
	S_P2_CMD_HEADER		header;
} S_P2_SEND_CMD_METER_CONNECT_REPLY;

/*kjg_110712 typedef struct s_p2_cali_check_data_tag {
	unsigned char		range;
	unsigned char		caliPointNum;
	unsigned char		caliCheckPointNum;
	unsigned char		ch;
	long				caliPointAD[MAX_CALI_POINT];
	long				caliPointDVM[MAX_CALI_POINT];
	long				caliCheckPointAD[MAX_CALI_POINT];
	long				caliCheckPointDVM[MAX_CALI_POINT];
} S_P2_CALI_CHECK_DATA;

typedef struct s_p2_send_cmd_cali_check_data_tag {
	S_P2_CMD_HEADER		header;
	S_P2_CALI_CHECK_DATA	caliCheckData;
} S_P2_SEND_CMD_CALI_CHECK_DATA; */

typedef struct s_p2_send_cmd_config_data_tag {
	S_P2_CMD_HEADER		header;
	S_P2_CONFIG_DATA	configData;
} S_P2_SEND_CMD_CONFIG_DATA;

typedef struct s_p2_send_cmd_comm_check_reply_tag {
	S_P2_CMD_HEADER		header;
	char				object_id[4];
	char				result[4];
	char				sended_cmd[4];
} S_P2_SEND_CMD_COMM_CHECK_REPLY;

typedef struct s_p2_send_cmd_comm_check_tag {
	S_P2_CMD_HEADER		header;
} S_P2_SEND_CMD_COMM_CHECK;

typedef struct s_p2_send_cmd_bcr_info_tag {
	S_P2_CMD_HEADER		header;
	char				bcr[P2_BCR_SIZE];
	char				reserved[64];
} S_P2_CMD_BCR_INFO;

typedef struct s_p2_send_cmd_user_cmd_tag {
	S_P2_CMD_HEADER		header;
	int					cmd;
	int					data;
	unsigned char		reserved[8];
} S_P2_SEND_CMD_USER_CMD;

typedef struct s_p2_rcv_cmd_set_tray_ready_tag {
	S_P2_CMD_HEADER		header;
	int					code;
	int					data;
} S_P2_RCV_CMD_SET_TRAY_READY;

typedef struct s_p2_real_time_data_tag {
	unsigned long		time;
	long				volt;
	long				curr;
} S_P2_REAL_TIME_DATA;

typedef struct s_p2_send_cmd_real_time_data_tag {
	int						ch;
	S_P2_REAL_TIME_DATA		realData[10];
} S_P2_SEND_CMD_REAL_TIME_DATA;

typedef struct s_p2_send_cmd_all_ng_tag {
	S_P2_CMD_HEADER		header;
} S_P2_SEND_CMD_ALL_NG;

typedef struct s_p2_chamber_data_tag {
	unsigned char		mode; //0:CHAMBER_STOP, 1:CHAMBER_RUN, 2:CHAMBER_PAUSE
	unsigned char		door_state; //0:DOOR_OPEN, 1:DOOR_CLOSE
	unsigned char		reserved1[2];

	short int			current_temp; //100 -> 1.0degree
	short int			current_humidity; //100 -> 1.0percent
	long				current_time; //1 -> 1sec

	short int			set_temp; //100 -> 1.0degree
	short int			set_humidity; //100 -> 1.0percent
	long				set_time; //1 -> 1sec

	long				reserved2;
} S_P2_CHAMBER_DATA;

typedef struct s_p2_send_cmd_chamber_data_tag {
	S_P2_CMD_HEADER		header;
	S_P2_CHAMBER_DATA	chamber;
} S_P2_SEND_CMD_CHAMBER_DATA;

typedef struct s_p2_send_cmd_test_header_tag { //kjg_110713
	S_P2_CMD_HEADER		header;

	S_P2_TEST_HEADER	testHeader;
} S_P2_SEND_CMD_TEST_HEADER;

typedef struct s_p2_send_cmd_test_precheck_tag {
	S_P2_CMD_HEADER		header;

	S_P2_TEST_PRECHECK	precheck;
} S_P2_SEND_CMD_TEST_PRECHECK;

typedef struct s_p2_send_cmd_test_step_header_tag {
	S_P2_CMD_HEADER		header;

	S_P2_TEST_STEP_HEADER	stepHeader;
} S_P2_SEND_CMD_TEST_STEP_HEADER;

typedef struct s_p2_send_cmd_test_step_ocv_tag {
	S_P2_CMD_HEADER		header;

	S_P2_TEST_STEP_HEADER	stepHeader;
	S_P2_TEST_STEP_OCV		ocv;
} S_P2_SEND_CMD_TEST_STEP_OCV;

typedef struct s_p2_send_cmd_test_step_charge_tag {
	S_P2_CMD_HEADER		header;

	S_P2_TEST_STEP_HEADER	stepHeader;
	S_P2_TEST_STEP_CHARGE	charge;
} S_P2_SEND_CMD_TEST_STEP_CHARGE;

typedef struct s_p2_send_cmd_test_step_discharge_tag {
	S_P2_CMD_HEADER		header;

	S_P2_TEST_STEP_HEADER	stepHeader;
	S_P2_TEST_STEP_DISCHARGE	discharge;
} S_P2_SEND_CMD_TEST_STEP_DISCHARGE;

typedef struct s_p2_send_cmd_test_step_rest_tag {
	S_P2_CMD_HEADER		header;

	S_P2_TEST_STEP_HEADER	stepHeader;
	S_P2_TEST_STEP_REST		discharge;
} S_P2_SEND_CMD_TEST_STEP_REST;

typedef struct s_p2_send_cmd_test_step_z_tag {
	S_P2_CMD_HEADER		header;

	S_P2_TEST_STEP_HEADER	stepHeader;
	S_P2_TEST_STEP_Z		z;
} S_P2_SEND_CMD_TEST_STEP_Z;

typedef struct s_p2_send_cmd_test_step_end_tag {
	S_P2_CMD_HEADER		header;

	S_P2_TEST_STEP_HEADER	stepHeader;
	S_P2_TEST_STEP_END		end;
} S_P2_SEND_CMD_TEST_STEP_END;

typedef struct s_p2_send_cmd_test_grade_header_tag {
	S_P2_CMD_HEADER		header;

	S_P2_TEST_GRADE_HEADER	gradeHeader;
} S_P2_SEND_CMD_TEST_GRADE_HEADER;

#endif
