#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "common_message.h"
#include "network.h"
#include "local_message.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_COA_CLIENT *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	switch(myPs->config.groupNo) {
		case 0:
			msgParsing(APP_TO_COA1);
			msgParsing(MODULE_TO_COA1);
			msgParsing(DATASAVE_TO_COA1);
			msgParsing(METER_TO_COA1);
			msgParsing(TIMESCH_TO_COA1); //kjh_160418
			break;
		case 1:
			msgParsing(APP_TO_COA2);
			msgParsing(MODULE_TO_COA2);
			msgParsing(DATASAVE_TO_COA2);
			msgParsing(METER_TO_COA2);
			break;
		default: break;
	}
}

void msgParsing(int fromPs)
{
	int idx;
	S_MSG_VAL RecvMsg;
	
	idx = rcv_msg(fromPs, (S_MSG_VAL *)&RecvMsg);
	if(idx < 0) return;
	
	switch(fromPs) {
		case APP_TO_COA1:
		case APP_TO_COA2:
			msgParsing_App_to_COA(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case MODULE_TO_COA1:
		case MODULE_TO_COA2:
			msgParsing_Module_to_COA(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case DATASAVE_TO_COA1:
		case DATASAVE_TO_COA2:
			msgParsing_DataSave_to_COA(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case METER_TO_COA1:
		case METER_TO_COA2:
			msgParsing_Meter_to_COA(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case TIMESCH_TO_COA1: //kjh_160418
			msgParsing_TimeSch_to_COA(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", fromPs, idx, RecvMsg.msg);
			break;
	}
}

void msgParsing_App_to_COA(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_APP_COA_REAL_TIME_REQUEST:
			myPs->signal[P1_SIG_SEND_REAL_TIME_REQUEST] = P1;
			break;
		case MSG_APP_COA_WRITE_VBF_DATA_REPLY:
			//0:NG, 1:OK
			send_cmd_uds_vbf_change_reply(RecvMsg->val[1],
				(char *)&myData->CAN.tmp_can_flash_file_name[0]);
			break;
		case MSG_APP_COA_TEST_COND_CONVERSION_END:
			send_cmd_test_cond_conversion_end(RecvMsg->val[1]);
			break;
		case MSG_APP_COA_READ_TH_TABLE_SET_REPLY: //jhkw_191108
			send_cmd_th_table_set_reply(RecvMsg->val[1]);
			break;
		case MSG_APP_COA_CMD_AUTOCALI_TEMP_REPLY: //jhkw_200317
			send_cmd_autocali_temp_reply(RecvMsg->val[1]);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"App to COA Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_Module_to_COA(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	int code;

	switch(RecvMsg->msg) {
		case MSG_MODULE_COA_TROUBLE_CODE:
			code = convert_group_trouble_code(CONVERT_ORG_TO_P1,
				RecvMsg->val[0]);
			if(code > 0) {
				send_cmd_trouble_code(code, RecvMsg->val[2], RecvMsg->val[3]);
			}
			break;
		case MSG_MODULE_COA_BMS_COMM_END:
			send_cmd_bms_comm_end(RecvMsg->val[0], RecvMsg->val[1]);
			break;
		case MSG_MODULE_COA_OUT_PACK_ISOLATION:
			if(RecvMsg->val[0] == 1) {
				send_cmd_bms_comm_reply(0,
					CAN_RX_FUNC_DIV_PACK_ISOLATION_TEST_START);
			} else {
				send_cmd_bms_comm_reply(0,
					CAN_RX_FUNC_DIV_PACK_ISOLATION_TEST_STOP);
			}
			break;
		case MSG_MODULE_COA_BMS_EOL_DATA1_REPLY:
			send_cmd_bms_eol_data1_reply();
			break;
		case MSG_MODULE_COA_BMS_EOL_DATA2_REPLY:
			send_cmd_bms_eol_data2_reply();
			break;
		case MSG_MODULE_COA_BMS_EOL_PACK_ID_WRITE_REPLY:
			send_cmd_bms_eol_pack_id_write_reply(RecvMsg->val[0]);
			break;
		case MSG_MODULE_COA_BMS_EOL_PACK_ID_CHECK_REPLY:
			send_cmd_bms_eol_pack_id_check_reply(RecvMsg->val[0]);
			break;
		case MSG_MODULE_COA_BMS_EOL_HI_POT_TEST_REPLY:
			send_cmd_bms_eol_hi_pot_test_reply(RecvMsg->val[0],
				RecvMsg->val[1]);
			break;
		case MSG_MODULE_COA_BMS_EOL_DATA3_REPLY:
			send_cmd_bms_eol_data3_reply();
			break;
		case MSG_MODULE_COA_BMS_EOL_CVTN_ID_REPLY:
			send_cmd_bms_eol_cvtn_id_reply();
			break;
		case MSG_MODULE_COA_DAQ_ISOLATION_REPLY:
			send_cmd_daq_isolation_reply(RecvMsg->val[1]);
			break;
		//jhkw_150224s
		case MSG_MODULE_COA_DAQ_ISOLATION_REPLY2:
			send_cmd_daq_isolation_reply2(RecvMsg->val[1], RecvMsg->val[2]);
			break;
		//jhkw_150224e
		case MSG_MODULE_COA_EOL_PROCEDURE_REPLY:
			send_cmd_eol_procedure_reply(RecvMsg->val[1]);
			break;
		case MSG_MODULE_COA_BMS_PROCEDURE_REPLY: //kjg_161205
			send_cmd_bms_procedure_reply(RecvMsg->val[0], RecvMsg->val[1],
				RecvMsg->val[2], RecvMsg->val[3]);
			break;
		case MSG_MODULE_COA_OUT_MUX_SELECT_REPLY:
			send_cmd_out_mux_select_reply(RecvMsg->val[0], RecvMsg->val[1]);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Module to COA Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_DataSave_to_COA(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_DATASAVE_COA_CALI_NORMAL_RESULT_SEND:
//#ifdef __B_TYPE__
			//kjhw_120517 Vref x 2
			send_cmd_cali_normal_result_1(RecvMsg->val[0], RecvMsg->val[1]);
//#else
//			send_cmd_cali_normal_result(RecvMsg->val[0]);
//#endif
			break;
		case MSG_DATASAVE_COA_CALI_CHECK_RESULT_SEND:
			send_cmd_cali_check_result(RecvMsg->val[0], RecvMsg->val[1]);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"DataSave to COA Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_Meter_to_COA(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_METER_COA_INITIALIZE_REPLY:
			myPs->signal[P1_SIG_METER_CONNECT_REPLY] = P1;
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Meter to COA Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

//kjh_160418s
void msgParsing_TimeSch_to_COA(int fromPs,  int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_TIMESCH_COA_CMD_NEXT_STEP:
			send_cmd_timesch_coa_next_step(RecvMsg->val[0]);	//kjh_160418
			//send_cmd_timesch_coa_next_step(ch);	//kjh_160418
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"TimeSch to COA Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}
//kjh_160418e
