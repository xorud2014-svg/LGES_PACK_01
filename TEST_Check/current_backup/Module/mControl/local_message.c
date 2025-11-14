#include <asm/io.h>
#include <pthread.h>
#include <rtl_core.h>
#include "../../INC/datastore.h"
#include "local_utils.h"
#include "InOutControl.h"
#include "GroupControl.h"
#include "ChannelControl.h"
#include "ch_utils.h"
//#include "../rt_can/rt_can_ext.h"	//kjg_181121
#ifdef __CAN_FD__	//jhkw_190714s
#include "rt_can_ext.h"	//kjg_181121
#else //CAN_2P0B
#include "../rt_can/rt_can.h"
#endif	//jhkw_190714e
#include "../rt_com/rt_com.h"
#include "CAN.h"
#include "COM.h"
#include "local_message.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;

void Check_Message(int slot)
{
	int i, j=3;

	switch(slot) {
		case 0: //module
			for(i=0; i < j; i++) {
				if(msgParsing(APP_TO_MODULE) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(DATASAVE_TO_MODULE) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(COA1_TO_MODULE) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(COA2_TO_MODULE) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(COB1_TO_MODULE) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(COB2_TO_MODULE) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(COC1_TO_MODULE) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(COC2_TO_MODULE) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(IO_TO_MODULE) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(METER_TO_MODULE) < 0) break;
			}
			break;
		case 1: //jigManager, meter, meter2
			//jigManager
			for(i=0; i < j; i++) {
				if(msgParsing(JIGM_TO_MODULE) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(APP_TO_JIGM) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(MODULE_TO_JIGM) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(JIGC1_TO_JIGM) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(JIGC2_TO_JIGM) < 0) break;
			}

			//meter
			for(i=0; i < j; i++) {
				if(msgParsing(APP_TO_METER) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(COA1_TO_METER) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(COC1_TO_METER) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(MODULE_TO_METER) < 0) break;
			}

			//meter2
			for(i=0; i < j; i++) {
				if(msgParsing(APP_TO_METER2) < 0) break;
			}
			break;
		case 2: //ioControl
			for(i=0; i < j; i++) {
				if(msgParsing(APP_TO_IO) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(MODULE_TO_IO) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(JIGC1_TO_IO) < 0) break;
			}
			for(i=0; i < j; i++) {
				if(msgParsing(JIGC2_TO_IO) < 0) break;
			}
			break;
		default:
			break;
	}
}

int msgParsing(int fromPs)
{
	int idx;
	S_MSG_VAL RecvMsg;

	idx = rcv_msg(fromPs, (S_MSG_VAL *)&RecvMsg);
	if(idx < 0) return -1;
	
	switch(fromPs) {
		//module
		case APP_TO_MODULE:
			msgParsing_App_to_Module(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case DATASAVE_TO_MODULE:
			msgParsing_DataSave_to_Module(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case COA1_TO_MODULE:
		case COA2_TO_MODULE:
			msgParsing_COA_to_Module(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
#ifdef __COB__
		case COB1_TO_MODULE:
		case COB2_TO_MODULE:
			msgParsing_COB_to_Module(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
#endif
#ifdef __COC__
		case COC1_TO_MODULE:
		case COC2_TO_MODULE:
			msgParsing_COC_to_Module(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
#endif
		case IO_TO_MODULE:
			msgParsing_IO_to_Module(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case METER_TO_MODULE:
			msgParsing_Meter_to_Module(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;

		//jigManager
		case JIGM_TO_MODULE:
			msgParsing_JigM_to_Module(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case APP_TO_JIGM:
			msgParsing_App_to_JigM(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case MODULE_TO_JIGM:
			msgParsing_Module_to_JigM(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case JIGC1_TO_JIGM:
		case JIGC2_TO_JIGM:
			msgParsing_JigC_to_JigM(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		
		//ioControl
		case APP_TO_IO:
			msgParsing_App_to_IO(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case MODULE_TO_IO:
			msgParsing_Module_to_IO(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case JIGC1_TO_IO:
		case JIGC2_TO_IO:
			msgParsing_JigC_to_IO(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;

		//meter
		case APP_TO_METER:
			msgParsing_App_to_Meter(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case COA1_TO_METER:
			msgParsing_COA_to_Meter(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case COC1_TO_METER:
			msgParsing_COC_to_Meter(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case MODULE_TO_METER:
			msgParsing_Module_to_Meter(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;

		//meter2
		case APP_TO_METER2:
			msgParsing_App_to_Meter2(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;

		default: break;
	}

	return 0;
}
	
void msgParsing_App_to_Module(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	unsigned char chFlag, chFlag1;
	unsigned char flag=0; //kjhw_170629
	int group, ch, val, i, j, k, comPort, rtn;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	S_RT_CAN_MSG can_msg;

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
	
	group = RecvMsg->val[0];
	chFlag = 0x01;
	k = 0;
	for(i=0; i < MAX_GROUP_PER_MODULE; i++) {
		if(group == i) {
			break;
		}
		k += (int)myPs->config.chInGroup[i];
	}

	memcpy((char *)&ch_flag, (char *)&myData->msg[fromPs].msg_ch_flag[idx],
		sizeof(S_MSG_CH_FLAG));

	switch(RecvMsg->msg) {
		case MSG_APP_MODULE_CH_CALI:
			ch = RecvMsg->val[0];
			myData->cData[ch].signal[C_SIG_CALI_NORMAL] = P1;
			break;
		case MSG_APP_MODULE_CAN_INITIALIZE:
			ch = RecvMsg->val[0];
			//send_cmd_can_controller_initialize(ch); //kjg_w_can
			break;
		case MSG_APP_MODULE_CAN_STATUS_REQUEST:
			ch = RecvMsg->val[0];
			//send_cmd_can_status_request(ch); //kjg_w_can
			break;
		case MSG_APP_MODULE_CAN_BAUDRATE_SET:
			ch = RecvMsg->val[0];
			val = RecvMsg->val[1];
#ifdef __CAN_FD__ //jhkw_190714s
			i = RecvMsg->val[2];
			j = RecvMsg->val[3];
			k = RecvMsg->val[4];

			rtl_printf("can param1 ch:%d baud:%d sjw:%d ext:%d fd_flag:%d\n",
				ch, val, j, i, k);
			rtl_printf("can param1 data_rate:%d crc:%d terminal_r:%d\n",
				RecvMsg->val[5], RecvMsg->val[6], RecvMsg->val[7]);

			rtn = rt_can_set_param(ch, val, j, i, k,
				RecvMsg->val[5], RecvMsg->val[6], RecvMsg->val[7]);

			myData->CAN.signal[ch][CAN_SIG_ABORT_TRANSMISSION] = 2; //kjg_180523
#else //CAN_2P0B
			rtn = rt_can_set_param(ch, val, RecvMsg->val[3], RecvMsg->val[2], 1);
			if(rtn < 0) {
				rtl_printf("kjg_d_can error1 %d, ch:%d val:%d sjw:%d ext:%d\n",
					rtn, ch, val, RecvMsg->val[3], RecvMsg->val[2]);
			}
#endif //jhkw_190714e
			break;
		case MSG_APP_MODULE_CAN_ERROR_CLEAR:
			ch = RecvMsg->val[0];
			//send_cmd_can_error_clear(ch); //kjg_w_can
			break;
		case MSG_APP_MODULE_RS232_BAUDRATE_SET:
			ch = RecvMsg->val[0];
			val = RecvMsg->val[1];
			//send_cmd_rs232_baudrate_set(ch, val); //kjg_w_com
			break;
		case MSG_APP_MODULE_CAN_DATA_CLEAR:
			ch = RecvMsg->val[0];
			memset((char *)&myData->CanData[ch][0], 0,
				sizeof(U_CAN_VAL) * MAX_CAN_DATA);

			switch(myData->AppControl.config.systemModel) { //kjg_141114
				case C_HLGP_500V_300A_100A_300KW:
				case C_HLGP_500V_300A_100A_300KW_2:
				case C_HLGP_500V_300A_100A_300KW_3:
				case C_HLGP_500V_300A_100A_300KW_4:
				case C_HLGP_500V_300A_100A_300KW_5:
				case C_LGC_450V_250A_50A_225KW: //kjhw_170724
				case C_LGC_450V_250A_50A_225KW_2: //kjhw_170724
					memset((char *)&myData->tmpCanData[ch][0][0], 0,
						sizeof(U_CAN_VAL) * MAX_CAN_DATA * 9);
					memset((char *)&myData->tmpCanData_sort[ch][0][0], 0,
						sizeof(U_CAN_VAL) * MAX_CAN_DATA * 9);
					memset((char *)&myData->tmpCanData_sort_flag[ch][0][0], 0,
						sizeof(unsigned char) * MAX_CAN_DATA * 9);
					myData->tmpCanData_sort_count[ch] = 3;
					memset((char *)&myData->tmpCanData_index[ch][0], 0,
						sizeof(unsigned char) * MAX_CAN_DATA);
					memset((char *)&myData->tmpCanData_count[ch][0], 0,
						sizeof(unsigned char) * MAX_CAN_DATA);
					break;
				default:
					break;
			}

			if(myData->mData.config.division_CAN == 1) { //kjhw_140620
				myData->CAN.signal[ch][CAN_SIG_TX_PHASE_DAQ_MUX] = P1;

				//kjg_111119 for lgc bms
				myData->CAN.signal[ch][CAN_SIG_BMS_TEST1] = P1;
				myData->CAN.signal[ch][CAN_SIG_BMS_TEST2] = P1;
				myData->CAN.signal[ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				//myData->CAN.signal[ch][CAN_SIG_TX_PHASE_TEST1] = P2;
				myData->CAN.signal[ch][CAN_SIG_TX_PHASE_OP_TYPE] = P6;
				myData->CAN.signal[ch][CAN_SIG_TX_PHASE_WAKE_UP] = P27;
				//myData->CAN.signal[ch][CAN_SIG_TX_PHASE_VALUE_CHANGE] = P10;
				//kjhw_170808

				//kjg_120619
				myData->CAN.rx_time_old[ch] = 0;
				myData->CAN.rx_time[ch] = 0;
				myData->CAN.rx_time_max[ch] = 0;
				myData->CAN.rx_time_min[ch] = 1000000000; //1000ms
			} else {
				switch(myData->AppControl.config.systemModel) { //kjhw_170629s
					case C_SKI_120V_400A_100A_192KW:
					case C_SKI_120V_400A_100A_192KW_2:
					case C_SKI_120V_400A_100A_192KW_3:
					case C_SKI_120V_400A_100A_192KW_4:
					case C_SKI_120V_400A_100A_192KW_5:
					case C_SKI_120V_400A_100A_192KW_6:
					case C_SKI_120V_400A_100A_192KW_7:
					case C_SKI_120V_400A_100A_192KW_8:
					case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
						if(ch == 0) flag = 0;
						else if(ch == 2) flag = 1;
						break;
					case C_SK_450V_200A_10A_360KW:
						if(ch == 0) flag = 0;
						else if(ch == 3) flag = 1;
						break;
					default:
						//if(ch == 0) flag = 0;
						//else flag = 1;
						if(ch == 0) flag = 0;
						else if(ch == 1) flag = 1;
						else if(ch == 2) flag = 2;
						else if(ch == 3) flag = 3;
						break;
				}

				if(flag == 0) {
					myData->CAN.signal[0][CAN_SIG_TX_PHASE_DAQ_MUX] = P1;
					myData->CAN.signal[1][CAN_SIG_TX_PHASE_DAQ_MUX] = P1;
					myData->CAN.signal[0][CAN_SIG_BMS_TEST1] = P1;
					myData->CAN.signal[0][CAN_SIG_BMS_TEST2] = P1;
					myData->CAN.signal[0][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[1][CAN_SIG_BMS_TEST1] = P1;
					myData->CAN.signal[1][CAN_SIG_BMS_TEST2] = P1;
					myData->CAN.signal[1][CAN_SIG_BMS_UDS_DELAY1] = 0;
					//myData->CAN.signal[0][CAN_SIG_TX_PHASE_TEST1] = P2;
					//myData->CAN.signal[1][CAN_SIG_TX_PHASE_TEST1] = P2;
					myData->CAN.signal[0][CAN_SIG_TX_PHASE_OP_TYPE] = P6;
					myData->CAN.signal[1][CAN_SIG_TX_PHASE_OP_TYPE] = P6;
					myData->CAN.signal[0][CAN_SIG_TX_PHASE_WAKE_UP] = P27;
					myData->CAN.signal[1][CAN_SIG_TX_PHASE_WAKE_UP] = P27;
					//myData->CAN.signal[0][CAN_SIG_TX_PHASE_VALUE_CHANGE] = P10;
					//myData->CAN.signal[1][CAN_SIG_TX_PHASE_VALUE_CHANGE] = P10;
					//kjhw_170808
					myData->CAN.rx_time_old[0] = 0;
					myData->CAN.rx_time[0] = 0;
					myData->CAN.rx_time_max[0] = 0;
					myData->CAN.rx_time_min[0] = 1000000000; //1000ms
					myData->CAN.rx_time_old[1] = 0;
					myData->CAN.rx_time[1] = 0;
					myData->CAN.rx_time_max[1] = 0;
					myData->CAN.rx_time_min[1] = 1000000000; //1000ms
				} else if(flag == 1) {
					myData->CAN.signal[2][CAN_SIG_TX_PHASE_DAQ_MUX] = P1;
					myData->CAN.signal[3][CAN_SIG_TX_PHASE_DAQ_MUX] = P1;
					myData->CAN.signal[2][CAN_SIG_BMS_TEST1] = P1;
					myData->CAN.signal[2][CAN_SIG_BMS_TEST2] = P1;
					myData->CAN.signal[2][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[3][CAN_SIG_BMS_TEST1] = P1;
					myData->CAN.signal[3][CAN_SIG_BMS_TEST2] = P1;
					myData->CAN.signal[3][CAN_SIG_BMS_UDS_DELAY1] = 0;
					//myData->CAN.signal[2][CAN_SIG_TX_PHASE_TEST1] = P2;
					//myData->CAN.signal[3][CAN_SIG_TX_PHASE_TEST1] = P2;
					myData->CAN.signal[2][CAN_SIG_TX_PHASE_OP_TYPE] = P6;
					myData->CAN.signal[3][CAN_SIG_TX_PHASE_OP_TYPE] = P6;
					myData->CAN.signal[2][CAN_SIG_TX_PHASE_WAKE_UP] = P27;
					myData->CAN.signal[3][CAN_SIG_TX_PHASE_WAKE_UP] = P27;
					//myData->CAN.signal[2][CAN_SIG_TX_PHASE_VALUE_CHANGE] = P10;
					//myData->CAN.signal[3][CAN_SIG_TX_PHASE_VALUE_CHANGE] = P10;
					//kjhw_170808
					myData->CAN.rx_time_old[2] = 0;
					myData->CAN.rx_time[2] = 0;
					myData->CAN.rx_time_max[2] = 0;
					myData->CAN.rx_time_min[2] = 1000000000; //1000ms
					myData->CAN.rx_time_old[3] = 0;
					myData->CAN.rx_time[3] = 0;
					myData->CAN.rx_time_max[3] = 0;
					myData->CAN.rx_time_min[3] = 1000000000; //1000ms
				} else if(flag == 2) {
					myData->CAN.signal[4][CAN_SIG_TX_PHASE_DAQ_MUX] = P1;
					myData->CAN.signal[5][CAN_SIG_TX_PHASE_DAQ_MUX] = P1;
					myData->CAN.signal[4][CAN_SIG_BMS_TEST1] = P1;
					myData->CAN.signal[4][CAN_SIG_BMS_TEST2] = P1;
					myData->CAN.signal[4][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[5][CAN_SIG_BMS_TEST1] = P1;
					myData->CAN.signal[5][CAN_SIG_BMS_TEST2] = P1;
					myData->CAN.signal[5][CAN_SIG_BMS_UDS_DELAY1] = 0;
					//myData->CAN.signal[2][CAN_SIG_TX_PHASE_TEST1] = P2;
					//myData->CAN.signal[3][CAN_SIG_TX_PHASE_TEST1] = P2;
					myData->CAN.signal[4][CAN_SIG_TX_PHASE_OP_TYPE] = P6;
					myData->CAN.signal[5][CAN_SIG_TX_PHASE_OP_TYPE] = P6;
					myData->CAN.signal[4][CAN_SIG_TX_PHASE_WAKE_UP] = P27;
					myData->CAN.signal[5][CAN_SIG_TX_PHASE_WAKE_UP] = P27;
					//myData->CAN.signal[2][CAN_SIG_TX_PHASE_VALUE_CHANGE] = P10;
					//myData->CAN.signal[3][CAN_SIG_TX_PHASE_VALUE_CHANGE] = P10;
					//kjhw_170808
					myData->CAN.rx_time_old[4] = 0;
					myData->CAN.rx_time[4] = 0;
					myData->CAN.rx_time_max[4] = 0;
					myData->CAN.rx_time_min[4] = 1000000000; //1000ms
					myData->CAN.rx_time_old[5] = 0;
					myData->CAN.rx_time[5] = 0;
					myData->CAN.rx_time_max[5] = 0;
					myData->CAN.rx_time_min[5] = 1000000000; //1000ms
				} else if(flag == 3) {
					myData->CAN.signal[6][CAN_SIG_TX_PHASE_DAQ_MUX] = P1;
					myData->CAN.signal[7][CAN_SIG_TX_PHASE_DAQ_MUX] = P1;
					myData->CAN.signal[6][CAN_SIG_BMS_TEST1] = P1;
					myData->CAN.signal[6][CAN_SIG_BMS_TEST2] = P1;
					myData->CAN.signal[6][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[7][CAN_SIG_BMS_TEST1] = P1;
					myData->CAN.signal[7][CAN_SIG_BMS_TEST2] = P1;
					myData->CAN.signal[7][CAN_SIG_BMS_UDS_DELAY1] = 0;
					//myData->CAN.signal[2][CAN_SIG_TX_PHASE_TEST1] = P2;
					//myData->CAN.signal[3][CAN_SIG_TX_PHASE_TEST1] = P2;
					myData->CAN.signal[6][CAN_SIG_TX_PHASE_OP_TYPE] = P6;
					myData->CAN.signal[7][CAN_SIG_TX_PHASE_OP_TYPE] = P6;
					myData->CAN.signal[6][CAN_SIG_TX_PHASE_WAKE_UP] = P27;
					myData->CAN.signal[7][CAN_SIG_TX_PHASE_WAKE_UP] = P27;
					//myData->CAN.signal[2][CAN_SIG_TX_PHASE_VALUE_CHANGE] = P10;
					//myData->CAN.signal[3][CAN_SIG_TX_PHASE_VALUE_CHANGE] = P10;
					//kjhw_170808
					myData->CAN.rx_time_old[6] = 0;
					myData->CAN.rx_time[6] = 0;
					myData->CAN.rx_time_max[6] = 0;
					myData->CAN.rx_time_min[6] = 1000000000; //1000ms
					myData->CAN.rx_time_old[7] = 0;
					myData->CAN.rx_time[7] = 0;
					myData->CAN.rx_time_max[7] = 0;
					myData->CAN.rx_time_min[7] = 1000000000; //1000ms
				} //kjhw_170629e
			}
			break;
		case MSG_APP_MODULE_CAN_TEST1:
			ch = RecvMsg->val[0];
			for(i=0; i < myPs->config.installedCAN; i++) {
				myData->CAN.signal[i][CAN_SIG_TEST1] = (unsigned char)ch;
			}
			break;
		case MSG_APP_MODULE_CAN_TEST2:
			ch = RecvMsg->val[0];
			memset((char *)&can_msg, 0, sizeof(S_RT_CAN_MSG));
			can_msg.id = 0x50;
			can_msg.type = 0;
			can_msg.length = 3;
			if(RecvMsg->val[1] == CAN_TX_FUNC_USER_RUN) {
				can_msg.data[0] = 0x02;
				can_msg.data[1] = 0x00;
			} else if(RecvMsg->val[1] == CAN_TX_FUNC_USER_STOP) {
				can_msg.data[0] = 0x01;
				can_msg.data[1] = 0x00;
			} else if(RecvMsg->val[1] == CAN_TX_FUNC_USER_PAUSE) {
				can_msg.data[0] = 0x08;
				can_msg.data[1] = 0x00;
			} else if(RecvMsg->val[1] == CAN_TX_FUNC_USER_CONTINUE) {
				can_msg.data[0] = 0x04;
				can_msg.data[1] = 0x00;
			} else if(RecvMsg->val[1] == CAN_TX_FUNC_USER_NEXT) {
				can_msg.data[0] = 0x10;
				can_msg.data[1] = 0x00;
			} else if(RecvMsg->val[1] == CAN_TX_FUNC_USER_GOTO) {
				can_msg.data[0] = 0x00;
				can_msg.data[1] = (unsigned char)RecvMsg->val[2];
			} else if(RecvMsg->val[1] == CAN_TX_FUNC_USER_VALUE1) {
				can_msg.id = 0x51;
				can_msg.data[0] = (unsigned char)RecvMsg->val[2];
				can_msg.data[1] = 0x00;
			} else {
				can_msg.data[0] = (unsigned char)RecvMsg->val[1];
				can_msg.data[1] = (unsigned char)RecvMsg->val[2];
			}
			if(myData->cData[ch / 2].op.state == C_RUN) {
				can_msg.data[2]
					= myData->cData[ch / 2].op.idxStepNo + 1;
			}

			rt_can_write(ch, (char *)&can_msg);
			break;
		case MSG_APP_MODULE_CAN_TEST3:
			ch = RecvMsg->val[0];
			memset((char *)&can_msg, 0, sizeof(S_RT_CAN_MSG));

			can_msg.id = 0x411;
			can_msg.type = 0;
			can_msg.length = 8;
			can_msg.data[0] = 0x11;
			can_msg.data[1] = 0x12;
			can_msg.data[2] = 0x13;
			can_msg.data[3] = 0x14;
			can_msg.data[4] = 0x25;
			can_msg.data[5] = 0x26;
			can_msg.data[6] = 0x27;
			can_msg.data[7] = 0x28;

			rt_can_write(ch, (char *)&can_msg);

			for(val=0; val < 20; val++) {
				can_msg.id = 0x412 + val;
				can_msg.data[7] = 0x29 + val;
				rt_can_write(ch, (char *)&can_msg);
			}
			break;
#ifdef __CAN_FD__ //jhkw_190714s
		case MSG_APP_MODULE_CAN_FD_TEST: //kjg_180405
			ch = RecvMsg->val[0];
			memset((char *)&can_msg, 0, sizeof(S_RT_CAN_MSG));

			can_msg.id = 0x411;
			can_msg.type = 0;
			can_msg.length = 64;
			can_msg.can_fd_flag = 0;
			can_msg.data[0] = 0x11;
			can_msg.data[1] = 0x12;
			can_msg.data[2] = 0x13;
			can_msg.data[3] = 0x14;
			can_msg.data[4] = 0x15;
			can_msg.data[5] = 0x16;
			can_msg.data[6] = 0x17;
			can_msg.data[7] = 0x18;
			can_msg.data[8] = 0x21;
			can_msg.data[9] = 0x22;
			can_msg.data[10] = 0x23;
			can_msg.data[11] = 0x24;
			can_msg.data[12] = 0x25;
			can_msg.data[13] = 0x26;
			can_msg.data[14] = 0x27;
			can_msg.data[15] = 0x28;
			can_msg.data[56] = 0x61;
			can_msg.data[57] = 0x62;
			can_msg.data[58] = 0x63;
			can_msg.data[59] = 0x64;
			can_msg.data[60] = 0x65;
			can_msg.data[61] = 0x66;
			can_msg.data[62] = 0x67;
			can_msg.data[63] = 0x68;

			rt_can_write(ch, (char *)&can_msg);

			can_msg.id = 0x412;
			rt_can_write(ch, (char *)&can_msg);
			break;
#endif //jhkw_190714e
		case MSG_APP_MODULE_CAN_ABORT_TRANSMISSION:
			ch = RecvMsg->val[0];
			myData->CAN.signal[ch][CAN_SIG_ABORT_TRANSMISSION] = 1;
			break;
		case MSG_APP_MODULE_READ_BCR:
			for(comPort=0; comPort < MAX_COM_PORT; comPort++) {
				if(myData->COM.config.functionType[comPort]
					== COM_FUNC_TYPE_BCR1) break;
			}
			if(comPort >= MAX_COM_PORT) break;

			val = RecvMsg->val[0]; //1base groupNo
			myData->COM.com_port[comPort].misc.bcr_reservation[val-1] = P1;
			break;
		case MSG_APP_MODULE_TEST_PAUSE:
			val = RecvMsg->val[0];
			if(myData->gData[val].state == G_RUN) {
				myData->gData[val].phase = P24;
			}
			for(i=0; i < myData->mData.config.chInGroup[val]; i++) {
				if(myData->cData[i].op.state == C_RUN) {
					myData->cData[i].signal[C_SIG_CMD_PAUSE] = P1;
				}
			}
			break;
		case MSG_APP_MODULE_TEST_CONTINUE:
			//debug_size_cob
/*			val = RecvMsg->val[0];
			if(myData->gData[val].state == G_PAUSE
				&& myData->gData[val].phase == P3) {
				i = myData->gData[val].misc.scan_ch[0];
				ch = (int)myData->COB_Client[val].config.ChArray2[i];
				//Select_OutPoint(2, myData->gData[val].misc.scan_ch[0]+1,
				//	O_RUN_RELAY, ON);
				Select_OutPoint(2, ch+1, O_RUN_RELAY, ON);
				myData->gData[val].cmd[CMD_MODE_USER] = CMD_TYPE_CONTINUE;
			}*/
			break;
		case MSG_APP_MODULE_TEST_STOP:
			val = RecvMsg->val[0];
			if(myData->gData[val].state == G_PAUSE
				&& myData->gData[val].phase == P3) {
				myData->gData[val].cmd[CMD_MODE_USER] = CMD_TYPE_STOP;
			}
			break;
		case MSG_APP_MODULE_CH_CALI_UPDATE:
			ch = RecvMsg->val[0];
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = P0;
			myData->cData[ch].signal[C_SIG_CALI_UPDATE] = P1;
			break;
		case MSG_APP_MODULE_CH_IO_SET:
			group = RecvMsg->val[0];

			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				switch(RecvMsg->val[1]) {
					case 1: //key_on
						if(RecvMsg->val[2] == 1) {
							myData->cData[ch].misc.ch_output_state = 0x01;
						} else {
							myData->cData[ch].misc.ch_output_state = 0x00;
						}
						break;
					case 2: //charge_on
						if(RecvMsg->val[2] == 1) {
							myData->cData[ch].misc.ch_output_state = 0x02;
						} else {
							myData->cData[ch].misc.ch_output_state = 0x00;
						}
						break;
					case 3: //pack_relay
						if(RecvMsg->val[2] == 1) {
							myData->cData[ch].misc.ch_output_state = 0x04;
						} else {
							myData->cData[ch].misc.ch_output_state = 0x00;
						}
						break;
					default:
						break;
				}
				myData->cData[ch].misc.tmp_ch_output_state
					= myData->cData[ch].misc.ch_output_state;
			}
			break;
		case MSG_APP_MODULE_CALI_DAV:
			if(myPs->state != M_STANDBY) break;
			group = RecvMsg->val[0] - 1;
			i = RecvMsg->val[1] - 1; //bd -> 0 single, 1 parallel
			myPs->signal[M_SIG_CALI_GROUP] = (unsigned char)group;
			myPs->signal[M_SIG_CALI_BD_START] = 0; //(unsigned char)i;
			myPs->signal[M_SIG_CALI_PARALLEL] = (unsigned char)i;
			myPs->signal[M_SIG_CALI_VI_SELECT] = P0; //voltage
			myPs->signal[M_SIG_CALI_CD_SELECT] = P0;
			myPs->signal[M_SIG_CALI_STEP] = P0;
			i = 0; //myPs->config.chPerBd * i;
			myData->cData[i].signal[C_SIG_RUN_RELAY_ON] = P0; //kjg_110814
			for(i=0; i < MAX_GROUP_4; i++) {
				for(j=0; j < MAX_TYPE; j++) {
					for(k=0; k < MAX_RANGE; k++) {
						myData->gData[i].cali_main_dac.tmp_da_caliFlag[j][k]
							= P0;
					}
				}
			}

			myData->gData[group].state = G_CALI;
			myData->gData[group].phase = P30;
			break;
		case MSG_APP_MODULE_CALI_DAI:
			if(myPs->state != M_STANDBY) break;
			group = RecvMsg->val[0] - 1;
			i = RecvMsg->val[1] - 1; //bd -> 0 single, 1 parallel
			myPs->signal[M_SIG_CALI_GROUP] = (unsigned char)group;
			myPs->signal[M_SIG_CALI_BD_START] = 0; //(unsigned char)i;
			myPs->signal[M_SIG_CALI_PARALLEL] = (unsigned char)i;
			myPs->signal[M_SIG_CALI_VI_SELECT] = P1; //current
			myPs->signal[M_SIG_CALI_CD_SELECT] = P1;
			myPs->signal[M_SIG_CALI_STEP] = P0;
			i = 0; //myPs->config.chPerBd * i;
			myData->cData[i].signal[C_SIG_RUN_RELAY_ON] = P0; //kjg_110814
			for(i=0; i < MAX_GROUP_4; i++) {
				for(j=0; j < MAX_TYPE; j++) {
					for(k=0; k < MAX_RANGE; k++) {
						myData->gData[i].cali_main_dac.tmp_da_caliFlag[j][k]
							= P0;
					}
				}
			}

			myData->gData[group].state = G_CALI;
			myData->gData[group].phase = P30;
			break;
		case MSG_APP_MODULE_CALI_DAI_SW:
			if(myPs->state != M_STANDBY) break;
			group = 0;
			i = RecvMsg->val[0] - 1; //bd -> 0 single, 1 parallel
			myPs->signal[M_SIG_CALI_GROUP] = (unsigned char)group;
			myPs->signal[M_SIG_CALI_BD_START] = 0; //(unsigned char)i;
			myPs->signal[M_SIG_CALI_PARALLEL] = (unsigned char)i;
			if(RecvMsg->val[1] > 0) {
				myPs->signal[M_SIG_CALI_RANGE_I]
					= (unsigned char)RecvMsg->val[1];
			}
			myPs->signal[M_SIG_CALI_VI_SELECT] = P1; //current
			myPs->signal[M_SIG_CALI_CD_SELECT] = P1;
			myPs->signal[M_SIG_CALI_STEP] = P0;
			i = 0; //myPs->config.chPerBd * i;
			myData->cData[i].signal[C_SIG_RUN_RELAY_ON] = P0; //kjg_110814
			for(i=0; i < MAX_GROUP_4; i++) {
				for(j=0; j < MAX_TYPE; j++) {
					for(k=0; k < MAX_RANGE; k++) {
						myData->gData[i].cali_main_dac.tmp_da_caliFlag[j][k]
							= P0;
					}
				}
			}

			myData->gData[group].state = G_CALI;
			myData->gData[group].phase = P30;
			break;
		case MSG_APP_MODULE_CALI_RANGE:
			if(RecvMsg->val[0] == 0) { //voltage
				myPs->signal[M_SIG_CALI_RANGE_V]
					= (unsigned char)RecvMsg->val[1];
				myPs->signal[M_SIG_CALI_VI_SELECT] = P0;
			} else { //current
				myPs->signal[M_SIG_CALI_RANGE_I]
					= (unsigned char)RecvMsg->val[1];
				myPs->signal[M_SIG_CALI_VI_SELECT] = P1;
			}
			break;
		default: break;
	}
}

void msgParsing_DataSave_to_Module(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	unsigned char chFlag1, chFlag;
	int group, ch, i, j, k;
	S_MSG_CH_FLAG ch_flag;
	
	group = RecvMsg->val[0];
	chFlag = 0x01;
	k = 0;
	for(i=0; i < MAX_GROUP_PER_MODULE; i++) {
		if(group == i) {
			break;
		}
		k += (int)myPs->config.chInGroup[i];
	}

	memcpy((char *)&ch_flag, (char *)&myData->msg[fromPs].msg_ch_flag[idx],
		sizeof(S_MSG_CH_FLAG));

	switch(RecvMsg->msg) {
		//for pack cycler
		case MSG_DATASAVE_MODULE_CMD_RUN:
			for(i=0; i < myData->mData.config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;
				myData->cData[ch].signal[C_SIG_CMD_RUN] = P1;
			}

			switch(RecvMsg->val[2]) {
				case 1: //debug_on(calibration)
					myData->cData[0].signal[C_SIG_BMS_DEBUG_ON] = P1;
					break;
				case 2: //debug_on(flash_request)
					myData->cData[0].signal[C_SIG_BMS_DEBUG_ON] = P2;
					break;
				case 3: //debug_on(flash_reset)
					myData->cData[0].signal[C_SIG_BMS_DEBUG_ON] = P3;
					break;
				default:
					myData->cData[0].signal[C_SIG_BMS_DEBUG_ON] = P0;
					break;
			}
			break;
		case MSG_DATASAVE_MODULE_CMD_CONTINUE:
			for(i=0; i < myData->mData.config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;
				myData->cData[ch].signal[C_SIG_CMD_CONTINUE] = P1;
			}
			break;
		case MSG_DATASAVE_MODULE_CALI_NORMAL_RESULT_SAVED:
			ch = RecvMsg->val[0];
			myData->cData[ch].signal[C_SIG_CALI_NORMAL_RESULT_SAVED] = P1;
			break;
		case MSG_DATASAVE_MODULE_CALI_CHECK_RESULT_SAVED:
			ch = RecvMsg->val[0];
			myData->cData[ch].signal[C_SIG_CALI_CHECK_RESULT_SAVED] = P1;
			break;
		case MSG_DATASAVE_MODULE_READ_PATTERN_FILE_END:
			ch = RecvMsg->val[0];
			chFlag = (unsigned char)RecvMsg->val[1];
			myData->cData[ch].signal[C_SIG_READ_PATTERN_FILE_END] = chFlag;
			break;
//20181219 KHK----------------------------------------------------			
		case MSG_DATASAVE_MODULE_READ_END_SOC_TRACKING_FILE:
			ch = RecvMsg->val[0];
			chFlag = (unsigned char)RecvMsg->val[1];
			myData->cData[ch].signal[C_SIG_READ_END_SOC_TRACKING_FILE] = chFlag;
			break;
//-------------------------------------------------------------------		
		case MSG_DATASAVE_MODULE_READ_END_SEQUENCE_CHARGE_FILE:
			ch = RecvMsg->val[0];
			chFlag = (unsigned char)RecvMsg->val[1];
			myData->cData[ch].signal[C_SIG_READ_END_SEQUENCE_CHARGE_FILE] = chFlag;
			break;
		case MSG_DATASAVE_MODULE_READ_USERMAP_FILE_END: //kjhw_140828
			ch = RecvMsg->val[0];
			chFlag = (unsigned char)RecvMsg->val[1];
			myData->cData[ch].signal[C_SIG_READ_USERMAP_FILE_END] = chFlag;
			break;

		//for formation
		case MSG_DATASAVE_MODULE_SAVED_FILE_DELETE_COMPLETE:
			myData->gData[group].signal[G_SIG_SAVED_FILE_DELETE_COMPLETE] = P1;
			break;
		case MSG_DATASAVE_MODULE_FAIL:
			myData->gData[group].code = (unsigned char)RecvMsg->val[1];
			myData->gData[group].signal[G_SIG_DATA_SAVE_FAIL] = P1;
			break;
		default: break;
	}
}

void msgParsing_COA_to_Module(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	unsigned char chFlag, chFlag1;
	int group, ch, i, j, k, toPs;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	
	group = RecvMsg->val[0];
	chFlag = 0x01;
	k = 0;
	for(i=0; i < MAX_GROUP_PER_MODULE; i++) {
		if(group == i) {
			break;
		}
		k += (int)myPs->config.chInGroup[i];
	}

	memcpy((char *)&ch_flag, (char *)&myData->msg[fromPs].msg_ch_flag[idx],
		sizeof(S_MSG_CH_FLAG));

	switch(RecvMsg->msg) {
		case MSG_COA_MODULE_CMD_RUN:
			myPs->signal[M_SIG_BUZZER_SET] = P0;	//csk_190625
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].op.reservedCmd
					= myData->testCond[ch].reserved.reserved_cmd;
			}

			memcpy((char *)&SendMsg,
				(char *)&myData->msg[fromPs].msg_val[idx], sizeof(S_MSG_VAL));

			toPs = MODULE_TO_DATASAVE;
			send_msg_ch_flag(toPs, (char *)&ch_flag);
			SendMsg.msg = MSG_MODULE_DATASAVE_RCVED_CMD_RUN;
			SendMsg.val[2] = RecvMsg->val[2]; //debug_on
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_COA_MODULE_CMD_STOP:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].op.reservedCmd
					= myData->testCond[ch].reserved.reserved_cmd;

				if(RecvMsg->val[1] == 0) { //direct command
					myData->cData[ch].signal[C_SIG_CMD_STOP] = P1;
				}
			}
			break;
		case MSG_COA_MODULE_CMD_PAUSE:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].op.reservedCmd
					= myData->testCond[ch].reserved.reserved_cmd;

				if(RecvMsg->val[1] == 0) { //direct command
					//jhkw_201102s
					//myData->cData[ch].signal[C_SIG_CMD_PAUSE] = P1;
					if(RecvMsg->val[2] == 0) { //user_pause
						myData->cData[ch].signal[C_SIG_CMD_PAUSE] = P1;
					} else {	//gui_error
						myData->cData[ch].signal[C_SIG_CMD_PAUSE] = P2;
						myData->cData[ch].op.gui_error_code = RecvMsg->val[2];
					}	//jhkw_201102e
				}
			}
			break;
		case MSG_COA_MODULE_CMD_CONTINUE:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].op.reservedCmd
					= myData->testCond[ch].reserved.reserved_cmd;
			}

			memcpy((char *)&SendMsg,
				(char *)&myData->msg[fromPs].msg_val[idx], sizeof(S_MSG_VAL));

			toPs = MODULE_TO_DATASAVE;
			send_msg_ch_flag(toPs, (char *)&ch_flag);
			SendMsg.msg = MSG_MODULE_DATASAVE_RCVED_CMD_CONTINUE;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_COA_MODULE_CMD_NEXT_STEP:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].op.reservedCmd
					= myData->testCond[ch].reserved.reserved_cmd;

				myData->cData[ch].signal[C_SIG_CMD_NEXT_STEP] = P1;
			}
			break;
		case MSG_COA_MODULE_CMD_GOTO_STEP: //kjhw_151211
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].op.reservedCmd
					= myData->testCond[ch].reserved.reserved_cmd;

				myData->cData[ch].signal[C_SIG_CMD_GOTO_STEP] = P1;
			}
			break;
		case MSG_COA_MODULE_TIMESCH_CMD_NEXT_STEP: //kjh_160418
			ch = RecvMsg->val[0];
			myData->cData[ch].op.reservedCmd
				= myData->testCond[ch].reserved.reserved_cmd;

			myData->cData[ch].signal[C_SIG_TIMESCH_CMD_NEXT_STEP] = P1;
			break;
		case MSG_COA_MODULE_CH_CALI:
			ch = RecvMsg->val[0];
			myData->cData[ch].signal[C_SIG_CALI_NORMAL] = P1;
			break;
		case MSG_COA_MODULE_CH_CALI_UPDATE:
			ch = RecvMsg->val[0];
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = P0;
			myData->cData[ch].signal[C_SIG_CALI_UPDATE] = P1;
			break;
		case MSG_COA_MODULE_CALI_RELAY:
			if(RecvMsg->val[0] == 0) { //voltage
				myData->mData.signal[M_SIG_CALI_RELAY] = P0;
				myData->mData.signal[M_SIG_CALI_RELAY2] = P0;
			} else { //current
				/*kjg_w if(RecvMsg->val[1] == 0) { //RANGE1
					myData->mData.signal[M_SIG_CALI_RELAY] = P1;
					myData->mData.signal[M_SIG_CALI_RELAY2] = P1;
				} else if(RecvMsg->val[1] == 1) { //RANGE2
					myData->mData.signal[M_SIG_CALI_RELAY] = P1;
					myData->mData.signal[M_SIG_CALI_RELAY2] = P0;
				}*/
			}
			break;
		case MSG_COA_MODULE_SAVE_MSG_FLAG:
			//val[0]:msg, val[1]:0-run,1-stop
			j = RecvMsg->val[0];
			if(RecvMsg->val[1] == 0) {
				myData->gData[group].signal[G_SIG_NET_CHECK] = P0;
			} else if(RecvMsg->val[1] == 1) {
				myData->gData[group].signal[G_SIG_NET_CHECK] = P1;
			}
			myData->save_msg[j].send_flag = RecvMsg->val[1];
			break;
		case MSG_COA_MODULE_BMS_COMM_REQUEST:
			if(myData->AppControl.config.systemModel
				== C_LGC_400V_60A_10A) {
				if(myData->dio.signal[DIO_SIG_IN_EXTERNAL1] != P3) break;
				//kjg_w warnning message
			}

			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				switch(RecvMsg->val[1]) {
					case CAN_RX_FUNC_DIV_FAN_SPEED:
						myData->CAN.signal[ch * 2][CAN_SIG_FAN_TEST] = P1;
						break;
					case CAN_RX_FUNC_DIV_ECU_ID:
						myData->CAN.signal[ch * 2][CAN_SIG_ECU_ID_REQUEST] = P1;
						break;
					case CAN_RX_FUNC_DIV_PACK_ISOLATION_TEST_STOP:
						Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION, OFF);
						myPs->signal[M_SIG_PACK_ISOLATION_PHASE] = P11;
						myPs->signal[M_SIG_PACK_ISOLATION_COUNT] = P0;
						break;
					case CAN_RX_FUNC_DIV_PACK_ISOLATION_TEST_START:
						Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION, ON);
						myPs->signal[M_SIG_PACK_ISOLATION_PHASE] = P1;
						myPs->signal[M_SIG_PACK_ISOLATION_COUNT] = P0;
						break;
					case CAN_RX_FUNC_DIV_BMS_IGNITION_ON:
						myData->dio.signal[DIO_SIG_IN_EXTERNAL1] = P1;
						break;
					case CAN_RX_FUNC_DIV_BMS_IGNITION_OFF:
						myData->dio.signal[DIO_SIG_IN_EXTERNAL1] = P0;
						break;
					default:	break;
				}
			}
			break;
		case MSG_COA_MODULE_BMS_EOL_DATA1_REQUEST:
			myPs->signal[M_SIG_BMS_EOL_PHASE] = P1;
			myPs->signal[M_SIG_BMS_EOL_CVTN_ID_FLAG] = P0;
			break;
		case MSG_COA_MODULE_BMS_EOL_DATA2_REQUEST:
			myPs->signal[M_SIG_BMS_EOL_PHASE] = P100;
			myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
			myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
			break;
		case MSG_COA_MODULE_BMS_EOL_PACK_ID_WRITE:
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P1;
			break;
		case MSG_COA_MODULE_BMS_EOL_PACK_ID_CHECK:
			myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P100;
			break;
		case MSG_COA_MODULE_BMS_EOL_HI_POT_TEST:
			if(RecvMsg->val[1] == 1 || RecvMsg->val[1] == 2) {
				myPs->signal[M_SIG_BMS_EOL_PHASE] = P150;
			} else if(RecvMsg->val[1] == 3 || RecvMsg->val[1] == 4) {
				myPs->signal[M_SIG_BMS_EOL_PHASE] = P160;
			}
			break;
		case MSG_COA_MODULE_BMS_EOL_DATA3_REQUEST:
			myPs->signal[M_SIG_BMS_EOL3_PHASE] = P1;
			break;
		case MSG_COA_MODULE_BMS_EOL_DATA3_LED_REQUEST:
			myPs->signal[M_SIG_BMS_EOL3_PHASE] = P20;
			break;
		case MSG_COA_MODULE_BMS_EOL_CVTN_ID_REQUEST:
			myPs->signal[M_SIG_BMS_EOL_PHASE] = P1;
			myPs->signal[M_SIG_BMS_EOL_CVTN_ID_FLAG] = P1;
			break;
		case MSG_COA_MODULE_BMS_EOL_DTC_CLEAR:
			myPs->signal[M_SIG_BMS_EOL_PHASE] = P170;
			break;
		case MSG_COA_MODULE_BMS_EOL_RESET_MICRO:
			myPs->signal[M_SIG_BMS_EOL_PHASE] = P180;
			break;
		case MSG_COA_MODULE_DAQ_ISOLATION_REQUEST:
			if(RecvMsg->val[1] == 1) { //iso
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION] = P1;
			} else {
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION] = P0;
			}

			if(RecvMsg->val[1] == 1) { //iso
				Select_OutPoint(0, 1, O_OUT_PACK_ISOLATION, OFF);
				Select_OutPoint(0, 2, O_OUT_PACK_ISOLATION, OFF); //kjhw_150216
			} else {
				Select_OutPoint(0, 1, O_OUT_PACK_ISOLATION, ON);
				Select_OutPoint(0, 2, O_OUT_PACK_ISOLATION, ON); //kjhw_150216
			}

			myPs->signal[M_SIG_DAQ_ISOLATION_REPLY_DELAY] = P1;
			break;
		case MSG_COA_MODULE_DAQ_ISOLATION_REQUEST2: //jhkw_150224
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;
				
				ch = myData->CellArray1[k + i].number2 - 1;
				if(RecvMsg->val[1] == 1) { //iso
					myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION] = P1;
				} else {
					myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION] = P0;
				}

				if(myData->cData[0].op.state != C_RUN) {
					if(RecvMsg->val[1] == 1) { //iso
						Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION2, OFF);
						Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION3, OFF);
						Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION4, OFF);
						Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION5, OFF);
						Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION6, OFF);
						myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P0;
					} else {
						if(RecvMsg->val[2] == 1) { //div_ch 1
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION2, ON);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION3,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION4,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION5,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION6,OFF);
							myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P1;
						} else if(RecvMsg->val[2] == 2){	//div_ch 2
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION2,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION3, ON);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION4,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION5,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION6,OFF);
							myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P2;
						} else if(RecvMsg->val[2] == 3){	//div_ch 3
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION2,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION3,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION4, ON);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION5,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION6,OFF);
							myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P3;
						} else if(RecvMsg->val[2] == 4){	//div_ch 4
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION2,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION3,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION4,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION5, ON);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION6,OFF);
							myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P4;
						} else if(RecvMsg->val[2] == 5){	//div_ch 5
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION2,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION3,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION4,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION5,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION6, ON);
							myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P5;
						} else {
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION2,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION3,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION4,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION5,OFF);
							Select_OutPoint(0, ch+1, O_OUT_PACK_ISOLATION6,OFF);
							myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P0;
						}
					}
				} else {
					myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P10;
				}
				myPs->signal[M_SIG_DAQ_ISOLATION_REPLY_DELAY2] = P1;
			}
			break;
		case MSG_COA_MODULE_OUT_MUX_SELECT_REQUEST: //kjhw_151021
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].misc.out_mux = (unsigned char)RecvMsg->val[1];

				if(myData->cData[ch].signal[C_SIG_OUT_MUX_SELECT] == P0) {
					myData->cData[ch].signal[C_SIG_OUT_MUX_SELECT] = P1;
				}
			}
			break;
		case MSG_COA_MODULE_EOL_PROCEDURE_REQUEST:	//kjg_120709
			if(RecvMsg->val[1] >= 100) { //test
				myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION] = P1;
				myPs->signal[M_SIG_EOL_PROCEDURE_INDEX]
					= (unsigned char)(RecvMsg->val[1] - 100);
			} else {
				myPs->signal[M_SIG_EOL_PROCEDURE_DIVISION] = P0;
				myPs->signal[M_SIG_EOL_PROCEDURE_INDEX]
					= (unsigned char)RecvMsg->val[1];
			}
			myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]
				= (unsigned char)RecvMsg->val[2];
			myPs->signal[M_SIG_EOL_PROCEDURE_PHASE] = P1;
			break;
		case MSG_COA_MODULE_BMS_PROCEDURE_REQUEST:	//kjg_161207
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				if(RecvMsg->val[1] != myData->canTransmitSetData
					.commonData[ch][0].bms_type) {
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					toPs = MODULE_TO_COA1 + group;
					send_msg_ch_flag(toPs, (char *)&ch_flag);
					SendMsg.msg = MSG_MODULE_COA_BMS_PROCEDURE_REPLY;
					SendMsg.val[0] = ch;
					SendMsg.val[1] = myData->canTransmitSetData
						.commonData[ch][0].bms_type;
					SendMsg.val[2] = RecvMsg->val[2]; //index
					SendMsg.val[3] = 1; //error code 0:ok, 1~:ng
					send_msg(toPs, (char *)&SendMsg);
					continue;
				}

				if(myData->CAN.signal[ch * 2][CAN_SIG_BMS_PROCEDURE_PHASE]
					== P0) {
					myData->CAN.signal[ch * 2][CAN_SIG_BMS_PROCEDURE_BMS_TYPE]
						= (unsigned char)RecvMsg->val[1];
					myData->CAN.signal[ch * 2][CAN_SIG_BMS_PROCEDURE_INDEX]
						= (unsigned char)RecvMsg->val[2];
					myData->CAN.signal[ch * 2][CAN_SIG_BMS_PROCEDURE_PHASE]
						= P1;
				} else {
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					toPs = MODULE_TO_COA1 + group;
					send_msg_ch_flag(toPs, (char *)&ch_flag);
					SendMsg.msg = MSG_MODULE_COA_BMS_PROCEDURE_REPLY;
					SendMsg.val[0] = ch;
					SendMsg.val[1] = myData->canTransmitSetData
						.commonData[ch][0].bms_type;
					SendMsg.val[2] = RecvMsg->val[2]; //index
					SendMsg.val[3] = 2; //error code 0:ok, 1~:ng
					send_msg(toPs, (char *)&SendMsg);
				}
			}
			break;
		case MSG_COA_MODULE_CALIMODE_REQUEST: //jhkw_130119
			if(RecvMsg->val[1] == 0) { //display
				Select_OutPoint(0, 1, O_CALI_MODE, OFF);
				myData->AppControl.misc.Load_Process_COM[1] = 0;
				myData->AppControl.misc.Load_Process_COM[2] = 1;
				myData->COM.config.autoStart[0] = 1;
				myData->AppControl.misc
					.Load_Process_COM[PROCESS_COM_CALI_METER] = P0;
				myData->COM.com_port[0].misc.externPort_useFlag = P0;
				rt_com_setup(0, 19200, RT_COM_PARITY_NONE, 1, 8);
			//	myData->COM.com_port[0].signal[COM_SIG_DISPLAY] = P0;
			} else if(RecvMsg->val[1] == 1) { //cali
				Select_OutPoint(0, 1, O_CALI_MODE, ON);
			//	myData->AppControl.misc.Load_Process_COM[1] = 1;
			//	myData->AppControl.misc.Load_Process_COM[2] = 0;
			//	myData->COM.config.autoStart[0] = 0;
			//	myData->AppControl.misc
			//		.Load_Process_COM[PROCESS_COM_CALI_METER] = P1;
				myData->COM.com_port[0].misc.externPort_useFlag = P1;
			//	rt_com_setup(0, 9600, RT_COM_PARITY_EVEN, 2, 7);
			//	myData->COM.com_port[0].signal[COM_SIG_DISPLAY] = P1;
			//	send_cmd_request_AGILENT_34401A(2);
				myData->COM.com_port[2]
						.signal[COM_SIG_CALI_MODE_INITIALIZE] = P1;
			}
			break;
		//jhkw_200317s
		case MSG_COA_MODULE_CMD_AUTOCALI:
			if(RecvMsg->val[1] == 0) { //display
				Select_OutPoint(0, 1, O_CALI_MODE, OFF);
				myData->AppControl.misc.Load_Process_COM[1] = 0;
				myData->AppControl.misc.Load_Process_COM[2] = 1;
				myData->COM.config.autoStart[0] = 1;
				myData->AppControl.misc
					.Load_Process_COM[PROCESS_COM_CALI_METER] = P0;
				myData->COM.com_port[0].misc.externPort_useFlag = P0;
				rt_com_setup(0, 19200, RT_COM_PARITY_NONE, 1, 8);
			} else if(RecvMsg->val[1] == 1) { //cali
				Select_OutPoint(0, 1, O_CALI_MODE, ON);
				myData->COM.com_port[0].misc.externPort_useFlag = P1;
				myData->COM.com_port[2]
						.signal[COM_SIG_CALI_MODE_INITIALIZE] = P1;
			}
			break;
		case MSG_COA_MODULE_CMD_CALI_STOP:
			ch = RecvMsg->val[2];
			if(RecvMsg->val[1] == 1) { 
				myData->cData[ch].signal[C_SIG_CMD_CALI_STOP] = P1;
			}
			break;
		//jhkw_200317e
		case MSG_COA_MODULE_LOAD_TYPE_SET: //kjhw_130129
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				if(RecvMsg->val[1] == 0) { //EDLC
					Select_OutPoint(0, ch+1, O_SELECT_V_CALI_PI, OFF);
				} else { //Cell
					Select_OutPoint(0, ch+1, O_SELECT_V_CALI_PI, ON);
				}
			}
			break;
		case MSG_COA_MODULE_CH_IO_SET:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				switch(RecvMsg->val[1]) {
					case 1: //key_on
						if(RecvMsg->val[2] == 1) {
							myData->cData[ch].misc.ch_output_state = 0x01;
						} else {
							myData->cData[ch].misc.ch_output_state = 0x00;
						}
						break;
					case 2: //charge_on
						if(RecvMsg->val[2] == 1) {
							myData->cData[ch].misc.ch_output_state = 0x02;
						} else {
							myData->cData[ch].misc.ch_output_state = 0x00;
						}
						break;
					case 3: //pack_relay
						if(RecvMsg->val[2] == 1) {
							myData->cData[ch].misc.ch_output_state = 0x04;
						} else {
							myData->cData[ch].misc.ch_output_state = 0x00;
						}
						break;
					default:
						break;
				}
				myData->cData[ch].misc.tmp_ch_output_state
					= myData->cData[ch].misc.ch_output_state;
			}
			break;
		case MSG_COA_MODULE_CMD_CHAMBER_CONTINUE:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				if(myData->cData[ch].op.state == C_PAUSE
					&& myData->cData[ch].op.code == C_CD_CHAMBER_CONTROL_WAIT)
					myData->cData[ch].signal[C_SIG_CMD_CHAMBER_CONTINUE] = P1;
			}
			break;
		case MSG_COA_MODULE_CMD_CYCLE_CONTINUE:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				if(myData->cData[ch].op.state == C_PAUSE
					&& myData->cData[ch].op.code == C_CD_CYCLE_CONTROL_WAIT)
					myData->cData[ch].signal[C_SIG_CMD_CYCLE_CONTINUE] = P1;
			}
			break;
		case MSG_COA_MODULE_CMD_CABLE_CHECK:
			memset((char *)&myPs->misc.cable_check, 0, sizeof(S_CABLE_CHECK));
			myPs->signal[M_SIG_CABLE_CHECK_PHASE] = P1;
			break;
		case MSG_COA_MODULE_CMD_CELL_CHECK:
			memset((char *)&myPs->misc.cell_check, 0, sizeof(S_CABLE_CHECK));
			myPs->signal[M_SIG_CELL_CHECK_PHASE] = P1;
			break;
		case MSG_COA_MODULE_BUZZER_STOP:
			//myPs->signal[M_SIG_BUZZER_SET] = P0;
			myPs->signal[M_SIG_BUZZER_SET] = P2;	//ktg_200412
			break;
		case MSG_COA_MODULE_ALARM_RESET:
			myPs->signal[M_SIG_ALARM_SET] = P0;
			myPs->signal[M_SIG_BUZZER_SET] = P0;
			break;
		case MSG_COA_MODULE_CMD_TEST_COND_UPDATE: //kjg_170810
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].signal[C_SIG_CMD_TEST_COND_UPDATE] = P1;
				myData->cData[ch].signal[C_SIG_CMD_TEST_COND_UPDATE_TYPE]
					= (unsigned char)RecvMsg->val[1];
					//0 : common_safety_update
					//1 : step_cond_update
					//2 : test_cond_update
				myData->cData[ch].misc.update_stepNo
					= (long)RecvMsg->val[2];
			}
			break;
		default: break;
	}
}

void msgParsing_COB_to_Module(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
#ifdef __COB__
	int group, toPs, comPort, i;
	S_MSG_VAL SendMsg;
	
	switch(RecvMsg->msg) {
		case MSG_COB_MODULE_CALI_METER_INIT:
			comPort = Find_comPort(COM_FUNC_TYPE_METER1);
			if(comPort >= 0) {
				if(RecvMsg->val[0] == 0) {
					myData->COM.com_port[comPort]
						.signal[COM_SIG_CALI_METER_INITIALIZE] = P21;
				} else {
					myData->COM.com_port[comPort]
						.signal[COM_SIG_CALI_METER_INITIALIZE] = P31;
				}
			}
			break;
		case MSG_COB_MODULE_WORK_MODE:
			group = RecvMsg->val[0];
			myData->gData[group].workMode = (unsigned char)RecvMsg->val[1];
			break;
		case MSG_COB_MODULE_CONTROL_MODE:
			group = RecvMsg->val[0];
			myData->gData[group].group_control_mode
				= (unsigned char)RecvMsg->val[1];
			break;
		case MSG_COB_MODULE_CMD_RUN:
			group = RecvMsg->val[0];
			if(myData->gData[group].group_control_mode != WORK_MODE_CONTROL) {
				myData->gData[group].group_control_mode = WORK_MODE_CONTROL;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				toPs = MODULE_TO_COB1 + group;
				SendMsg.msg = MSG_MODULE_COB_CONTROL_MODE;
				SendMsg.val[0] = group;
				SendMsg.val[1] = WORK_MODE_CONTROL;
				send_msg(toPs, (char *)&SendMsg);
			}

			if(myData->gData[group].state == G_STANDBY
				&& myData->gData[group].phase == P1
				&& myData->gData[group].cmd[CMD_MODE_USER] == CMD_TYPE_NONE) {
				myData->gData[group].cmd[CMD_MODE_USER] = CMD_TYPE_RUN;
			}

			myData->gData[group].code = G_CD_NONE;
			break;
		case MSG_COB_MODULE_CMD_STOP:
			group = RecvMsg->val[0];
			if(myData->gData[group].state == G_RUN
				&& myData->gData[group].phase < P20
				&& myData->gData[group].cmd[CMD_MODE_USER] == CMD_TYPE_NONE) {
				myData->gData[group].cmd[CMD_MODE_USER] = CMD_TYPE_STOP;
				myData->gData[group].code = G_CD_NONE;
			} else if(myData->gData[group].state == G_PAUSE
				&& myData->gData[group].cmd[CMD_MODE_USER] == CMD_TYPE_NONE) {
				myData->gData[group].cmd[CMD_MODE_USER] = CMD_TYPE_STOP;
				myData->gData[group].code = G_CD_NONE;
			}
			break;
		case MSG_COB_MODULE_CMD_PAUSE:
			group = RecvMsg->val[0];
			if(myData->gData[group].state == G_RUN
				&& (myData->gData[group].phase >= P10
				&& myData->gData[group].phase < P20)) {
				myData->gData[group].cmd[CMD_MODE_USER] = CMD_TYPE_PAUSE;
			}
			break;
		case MSG_COB_MODULE_CMD_CONTINUE:
			group = RecvMsg->val[0];
			if(myData->gData[group].state == G_PAUSE) {
				myData->gData[group].cmd[CMD_MODE_USER] = CMD_TYPE_CONTINUE;
			}
			break;
		case MSG_COB_MODULE_CMD_NEXT_STEP:
			group = RecvMsg->val[0];
			if(myData->gData[group].state == G_RUN) {
				myData->gData[group].cmd[CMD_MODE_USER] = CMD_TYPE_NEXT_STEP;
			}
			break;
		case MSG_COB_MODULE_CMD_RESET:
			group = RecvMsg->val[0];
			myData->gData[group].state = G_IDLE;
			myData->gData[group].phase = P0;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			toPs = MODULE_TO_JIGM;
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(toPs, (char *)&SendMsg);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			toPs = MODULE_TO_COB1 + group;
			SendMsg.msg = MSG_MODULE_COB_RESET_COMPLETE;
			SendMsg.val[0] = group;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_COB_MODULE_CMD_CLEAR:
			group = RecvMsg->val[0];
			myData->gData[group].cmd[CMD_MODE_USER] = CMD_TYPE_CLEAR;
			myData->gData[group].code = G_CD_NONE;
			break;
		case MSG_COB_MODULE_JIG_TRY_CONTACT:
			group = RecvMsg->val[0];

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			toPs = MODULE_TO_JIGM;
			SendMsg.msg = MSG_MODULE_JIGM_TRY_CONTACT;
			SendMsg.val[0] = group;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_COB_MODULE_JIG_TRY_DISCONTACT:
			group = RecvMsg->val[0];

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			toPs = MODULE_TO_JIGM;
			SendMsg.msg = MSG_MODULE_JIGM_TRY_DISCONTACT;
			SendMsg.val[0] = group;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_COB_MODULE_PLC_DOOR_OPEN:
			for(comPort=0; comPort < MAX_COM_PORT; comPort++) {
				if(myData->COM.config.functionType[comPort]
					== COM_FUNC_TYPE_IO_COMM1) break;
			}
			if(comPort >= MAX_COM_PORT) break;

			myData->COM.com_port[comPort].signal[COM_SIG_PLC_DOOR_OPEN] = P1;
			break;
		case MSG_COB_MODULE_PLC_DOOR_CLOSE:
			for(comPort=0; comPort < MAX_COM_PORT; comPort++) {
				if(myData->COM.config.functionType[comPort]
					== COM_FUNC_TYPE_IO_COMM1) break;
			}
			if(comPort >= MAX_COM_PORT) break;

			myData->COM.com_port[comPort].signal[COM_SIG_PLC_DOOR_CLOSE] = P1;
			break;
		case MSG_COB_MODULE_PLC_FAULT_READ:
			for(comPort=0; comPort < MAX_COM_PORT; comPort++) {
				if(myData->COM.config.functionType[comPort]
					== COM_FUNC_TYPE_IO_COMM1) break;
			}
			if(comPort >= MAX_COM_PORT) break;

			myData->COM.com_port[comPort].signal[COM_SIG_PLC_FAULT_READ] = P1;
			break;
		case MSG_COB_MODULE_CHAMBER_SET:
			switch(RecvMsg->val[1]) {
				case 0: //CHAMBER_STOP
					for(comPort=0; comPort < MAX_COM_PORT; comPort++) {
						if(myData->COM.config.functionType[comPort]
							== COM_FUNC_TYPE_CHAMBER1) break;
					}
					if(comPort >= MAX_COM_PORT) break;

					myData->COM.com_port[comPort]
						.signal[COM_SIG_CHAMBER_STOP] = P1;
					break;
				case 1: //CHAMBER_RUN
					for(comPort=0; comPort < MAX_COM_PORT; comPort++) {
						if(myData->COM.config.functionType[comPort]
							== COM_FUNC_TYPE_CHAMBER1) break;
					}
					if(comPort >= MAX_COM_PORT) break;

					myData->COM.com_port[comPort]
						.signal[COM_SIG_CHAMBER_RUN] = P1;
					break;
				case 2: //CHAMBER_SET
					for(comPort=0; comPort < MAX_COM_PORT; comPort++) {
						if(myData->COM.config.functionType[comPort]
							== COM_FUNC_TYPE_CHAMBER1) break;
					}
					if(comPort >= MAX_COM_PORT) break;

					myData->COM.com_port[comPort]
						.signal[COM_SIG_CHAMBER_SET] = P1;
					break;
				case 3: //CHAMBER_DOOR_OPEN
					for(comPort=0; comPort < MAX_COM_PORT; comPort++) {
						if(myData->COM.config.functionType[comPort]
							== COM_FUNC_TYPE_IO_COMM1) break;
					}
					if(comPort >= MAX_COM_PORT) break;

					myData->COM.com_port[comPort]
						.signal[COM_SIG_PLC_DOOR_OPEN] = P1;
					break;
				case 4: //CHAMBER_DOOR_CLOSE
					for(comPort=0; comPort < MAX_COM_PORT; comPort++) {
						if(myData->COM.config.functionType[comPort]
							== COM_FUNC_TYPE_IO_COMM1) break;
					}
					if(comPort >= MAX_COM_PORT) break;

					myData->COM.com_port[comPort]
						.signal[COM_SIG_PLC_DOOR_CLOSE] = P1;
					break;
				default: break;
			}
			break;
		case MSG_COB_MODULE_CALI_MULTI: //kjg_110816_w
			if(RecvMsg->val[2] == 0) {
				myPs->signal[M_SIG_CALI_PARALLEL] = P0;
			} else {
				myPs->signal[M_SIG_CALI_PARALLEL] = P1;
			}
			break;
		case MSG_COB_MODULE_CALI_MAP: //kjg_110816_w
			break;
		case MSG_COB_MODULE_CALI_V_RANGE:
			if(RecvMsg->val[1] > 0) {
				myPs->signal[M_SIG_CALI_RANGE_V]
					= (unsigned char)RecvMsg->val[1];
			}
			break;
		case MSG_COB_MODULE_CALI_V:
			group = RecvMsg->val[0];
			i = RecvMsg->val[1]; //bd
			if(i == 0) {
				myPs->signal[M_SIG_CALI_BD_START] = 0;
				myPs->signal[M_SIG_CALI_BD_COUNT] = myPs->config.installedBd-1;
			} else {
				myPs->signal[M_SIG_CALI_BD_START] = (unsigned char)(i - 1);
				myPs->signal[M_SIG_CALI_BD_COUNT] = 1;
			}
			myPs->signal[M_SIG_CALI_GROUP] = (unsigned char)group;
			myPs->signal[M_SIG_CALI_SEQUENCE] = P2;
			myPs->signal[M_SIG_CALI_CH] = 0;
			myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = 0;
			myPs->signal[M_SIG_CALI_VI_SELECT] = P0; //voltage
			myPs->signal[M_SIG_CALI_CD_SELECT] = P0; //charge
			myPs->signal[M_SIG_CALI_STEP] = P0;
			myPs->signal[M_SIG_CALI_RANGE_I] = 1;

			myData->gData[group].state = G_CALI;
			myData->gData[group].phase = P0;
			break;
		case MSG_COB_MODULE_CALI_CHECK_V:
			group = RecvMsg->val[0];
			i = RecvMsg->val[1]; //bd
			if(i == 0) {
				myPs->signal[M_SIG_CALI_BD_START] = 0;
				myPs->signal[M_SIG_CALI_BD_COUNT] = myPs->config.installedBd-1;
			} else {
				myPs->signal[M_SIG_CALI_BD_START] = (unsigned char)(i - 1);
				myPs->signal[M_SIG_CALI_BD_COUNT] = 1;
			}
			myPs->signal[M_SIG_CALI_GROUP] = (unsigned char)group;
			myPs->signal[M_SIG_CALI_SEQUENCE] = P3;
			myPs->signal[M_SIG_CALI_CH] = 0;
			myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = 0;
			myPs->signal[M_SIG_CALI_VI_SELECT] = P0; //voltage
			myPs->signal[M_SIG_CALI_CD_SELECT] = P0; //charge
			myPs->signal[M_SIG_CALI_STEP] = P0;
			myPs->signal[M_SIG_CALI_RANGE_I] = 1;

			myData->gData[group].state = G_CALI;
			myData->gData[group].phase = P0;
			break;
		case MSG_COB_MODULE_CALI_I_RANGE:
			if(RecvMsg->val[1] > 0) {
				myPs->signal[M_SIG_CALI_RANGE_I]
					= (unsigned char)RecvMsg->val[1];
			}
			break;
		case MSG_COB_MODULE_CALI_I:
			group = RecvMsg->val[0];
			i = RecvMsg->val[1]; //bd
			if(i == 0) {
				myPs->signal[M_SIG_CALI_BD_START] = 0;
				myPs->signal[M_SIG_CALI_BD_COUNT] = myPs->config.installedBd-1;
			} else {
				myPs->signal[M_SIG_CALI_BD_START] = (unsigned char)(i - 1);
				myPs->signal[M_SIG_CALI_BD_COUNT] = 1;
			}
			myPs->signal[M_SIG_CALI_GROUP] = (unsigned char)group;
			myPs->signal[M_SIG_CALI_SEQUENCE] = P2;
			myPs->signal[M_SIG_CALI_CH] = 0;
			myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = 0;
			myPs->signal[M_SIG_CALI_VI_SELECT] = P1; //current
			//myPs->signal[M_SIG_CALI_CD_SELECT] = P1; //discharge
			myPs->signal[M_SIG_CALI_STEP] = P0;
			//myPs->signal[M_SIG_CALI_RANGE_I] = 1;
			myPs->signal[M_SIG_CALI_RANGE_V] = 1;

			myData->gData[group].state = G_CALI;
			myData->gData[group].phase = P0;
			break;
		case MSG_COB_MODULE_CALI_CHECK_I:
			group = RecvMsg->val[0];
			i = RecvMsg->val[1]; //bd
			if(i == 0) {
				myPs->signal[M_SIG_CALI_BD_START] = 0;
				myPs->signal[M_SIG_CALI_BD_COUNT] = myPs->config.installedBd-1;
			} else {
				myPs->signal[M_SIG_CALI_BD_START] = (unsigned char)(i - 1);
				myPs->signal[M_SIG_CALI_BD_COUNT] = 1;
			}
			myPs->signal[M_SIG_CALI_GROUP] = (unsigned char)group;
			myPs->signal[M_SIG_CALI_SEQUENCE] = P3;
			myPs->signal[M_SIG_CALI_CH] = 0;
			myPs->signal[M_SIG_CALI_CH_FLAG_IDX] = 0;
			myPs->signal[M_SIG_CALI_VI_SELECT] = P1; //current
			//myPs->signal[M_SIG_CALI_CD_SELECT] = P1; //discharge
			myPs->signal[M_SIG_CALI_STEP] = P0;
			//myPs->signal[M_SIG_CALI_RANGE_I] = 1;
			myPs->signal[M_SIG_CALI_RANGE_V] = 1;

			myData->gData[group].state = G_CALI;
			myData->gData[group].phase = P0;
			break;
		case MSG_COB_MODULE_CALI_STOP:
			group = RecvMsg->val[0];
			if(myData->gData[group].state == G_CALI) {
				myData->gData[group].state = G_CALI;
				myData->gData[group].phase = P20;
			}
			break;
		default: break;
	}
#endif
}

void msgParsing_COC_to_Module(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
#ifdef __COC__
	unsigned char chFlag, chFlag1;
	int group, ch, i, j, k, toPs;
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL SendMsg;
	
	group = RecvMsg->val[0];
	chFlag = 0x01;
	k = 0;
	for(i=0; i < MAX_GROUP_PER_MODULE; i++) {
		if(group == i) {
			break;
		}
		k += (int)myPs->config.chInGroup[i];
	}

	memcpy((char *)&ch_flag, (char *)&myData->msg[fromPs].msg_ch_flag[idx],
		sizeof(S_MSG_CH_FLAG));

	switch(RecvMsg->msg) {
		case MSG_COC_MODULE_CMD_RUN:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].op.reservedCmd
					= myData->testCond[ch].reserved.reserved_cmd;
			}

			memcpy((char *)&SendMsg,
				(char *)&myData->msg[fromPs].msg_val[idx], sizeof(S_MSG_VAL));

			toPs = MODULE_TO_DATASAVE;
			send_msg_ch_flag(toPs, (char *)&ch_flag);
			SendMsg.msg = MSG_MODULE_DATASAVE_RCVED_CMD_RUN;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_COC_MODULE_CMD_STOP:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].op.reservedCmd
					= myData->testCond[ch].reserved.reserved_cmd;

				if(RecvMsg->val[1] == 0) { //direct command
					myData->cData[ch].signal[C_SIG_CMD_STOP] = P1;
				}
			}
			break;
		case MSG_COC_MODULE_CMD_PAUSE:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].op.reservedCmd
					= myData->testCond[ch].reserved.reserved_cmd;

				if(RecvMsg->val[1] == 0) { //direct command
					myData->cData[ch].signal[C_SIG_CMD_PAUSE] = P1;
				}
			}
			break;
		case MSG_COC_MODULE_CMD_CONTINUE:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].op.reservedCmd
					= myData->testCond[ch].reserved.reserved_cmd;
			}

			memcpy((char *)&SendMsg,
				(char *)&myData->msg[fromPs].msg_val[idx], sizeof(S_MSG_VAL));

			toPs = MODULE_TO_DATASAVE;
			send_msg_ch_flag(toPs, (char *)&ch_flag);
			SendMsg.msg = MSG_MODULE_DATASAVE_RCVED_CMD_CONTINUE;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_COC_MODULE_CMD_NEXT_STEP:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].op.reservedCmd
					= myData->testCond[ch].reserved.reserved_cmd;

				myData->cData[ch].signal[C_SIG_CMD_NEXT_STEP] = P1;
			}
			break;
		case MSG_COC_MODULE_CMD_CHAMBER_CONTINUE:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				if(myData->cData[ch].op.state == C_PAUSE
					&& myData->cData[ch].op.code == C_CD_CHAMBER_CONTROL_WAIT)
					myData->cData[ch].signal[C_SIG_CMD_CHAMBER_CONTINUE] = P1;
			}
			break;
		case MSG_COC_MODULE_CMD_CYCLE_CONTINUE:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				if(myData->cData[ch].op.state == C_PAUSE
					&& myData->cData[ch].op.code == C_CD_CYCLE_CONTROL_WAIT)
					myData->cData[ch].signal[C_SIG_CMD_CYCLE_CONTINUE] = P1;
			}
			break;
		case MSG_COC_MODULE_CMD_INIT:
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				myData->cData[ch].op.state = C_IDLE;
				myData->cData[ch].op.phase = P0;
			}
			break;
		case MSG_COC_MODULE_CH_CALI:
			ch = RecvMsg->val[0];
			myData->cData[ch].signal[C_SIG_CALI_NORMAL] = P1;
			break;
		case MSG_COC_MODULE_CH_CALI_UPDATE:
			ch = RecvMsg->val[0];
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = P0;
			myData->cData[ch].signal[C_SIG_CALI_UPDATE] = P1;
			break;
		case MSG_COC_MODULE_CALI_RELAY:
			if(RecvMsg->val[0] == 0) { //voltage
				myData->mData.signal[M_SIG_CALI_RELAY] = P0;
				myData->mData.signal[M_SIG_CALI_RELAY2] = P0;
			} else { //current
				/*kjg_w if(RecvMsg->val[1] == 0) { //RANGE1
					myData->mData.signal[M_SIG_CALI_RELAY] = P1;
					myData->mData.signal[M_SIG_CALI_RELAY2] = P1;
				} else if(RecvMsg->val[1] == 1) { //RANGE2
					myData->mData.signal[M_SIG_CALI_RELAY] = P1;
					myData->mData.signal[M_SIG_CALI_RELAY2] = P0;
				}*/
			}
			break;
		case MSG_COC_MODULE_SAVE_MSG_FLAG:
			//val[0]:msg, val[1]:0-run,1-stop
			j = RecvMsg->val[0];
			if(RecvMsg->val[1] == 0) {
				myData->gData[group].signal[G_SIG_NET_CHECK] = P0;
			} else if(RecvMsg->val[1] == 1) {
				myData->gData[group].signal[G_SIG_NET_CHECK] = P1;
			}
			myData->save_msg_1[j].send_flag = RecvMsg->val[1];
			break;
		case MSG_COC_MODULE_CMD_CABLE_CHECK:
			memset((char *)&myPs->misc.cable_check, 0, sizeof(S_CABLE_CHECK));
			myPs->signal[M_SIG_CABLE_CHECK_PHASE] = P1;
			break;
		case MSG_COC_MODULE_CMD_CELL_CHECK:
			memset((char *)&myPs->misc.cell_check, 0, sizeof(S_CABLE_CHECK));
			myPs->signal[M_SIG_CELL_CHECK_PHASE] = P1;
			break;
		default: break;
	}
#endif
}

void msgParsing_IO_to_Module(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int ch = 0;		//csk_190620

	switch(RecvMsg->msg) {
		case MSG_IO_MODULE_EXIT:
			if(myPs->signal[M_SIG_EXIT_PHASE] != P0) break;
			myPs->code = RecvMsg->val[0];
			if(myPs->code == M_CD_FAULT_WARNING_POWER_OFF
				|| myPs->code == M_CD_FAULT_CALI_METER_COMM_ERROR
				|| myPs->code == M_CD_FAULT_CALIBRATOR_COMM_ERROR
				|| myPs->code == M_CD_FAULT_FAN
				|| myPs->code == M_CD_FAULT_JOB_CHANGE
				|| myPs->code == M_CD_FAULT_PITCH_CHANGE
				|| myPs->code == M_CD_FAULT_LOAD_LINE2
				|| myPs->code == M_CD_FAULT_CAN_WARNNING
				|| myPs->code == G_CD_FAULT_DATASAVE_PROCESS_ERROR
				) { //Warnning
				myPs->signal[M_SIG_EXIT_TYPE] = P0;
			} else if(myPs->code == M_CD_FAULT_AC_POWER_SHORT) { //shh_231217
				//Pause
				myPs->signal[M_SIG_EXIT_TYPE] = P5;
				myPs->signal[M_SIG_EXIT_GROUP] = 0;

				myPs->signal[M_SIG_ALARM_SET] = P1;
				myPs->signal[M_SIG_BUZZER_SET] = P1;
			} else if(myPs->code == M_CD_FAULT_CHAMBER
				|| myPs->code == M_CD_FAULT_LOAD_LINE
				|| myPs->code == M_CD_FAULT_CHAMBER_DOOR_OPEN
				|| myPs->code == M_CD_FAULT_CHAMBER_AREA_SENSOR
				|| myPs->code == M_CD_FAULT_CHAMBER_MANUAL_DOOR
				|| myPs->code == M_CD_FAULT_CHAMBER_AIR
				|| myPs->code == M_CD_FAULT_CHAMBER_APR_S
				|| myPs->code == M_CD_FAULT_CHAMBER_REF
				|| myPs->code == M_CD_FAULT_CHAMBER_FAN
				|| myPs->code == M_CD_FAULT_CHAMBER_POWER
				|| myPs->code == M_CD_FAULT_CHAMBER_REF_ERROR
				|| myPs->code == M_CD_FAULT_CHAMBER_CP_TRIP //lki_111010

				|| myPs->code == G_CD_FAULT_UPPER_VOLTAGE
				|| myPs->code == G_CD_FAULT_UPPER_CURRENT
				|| myPs->code == G_CD_FAULT_RUN_TIME_OVER
				|| myPs->code == G_CD_FAULT_ADC
				|| myPs->code == G_CD_FAULT_JIG_ACTIVE_ERROR
				|| myPs->code == G_CD_FAULT_JIG_TRAY_ERROR
				|| myPs->code == G_CD_FAULT_JIG_DOOR_ERROR
				|| myPs->code == G_CD_FAULT_JIG_AIR_PRESS_ERROR
				|| myPs->code == G_CD_FAULT_JIG_STACKER_ERROR

				|| myPs->code == G_CD_FAULT_J_MAIN_CYL_UP_TIMEOUT
				|| myPs->code == G_CD_FAULT_J_MAIN_CYL_DOWN_TIMEOUT
				|| myPs->code == G_CD_FAULT_J_MAIN_CYL_UP_SENS
				|| myPs->code == G_CD_FAULT_J_MAIN_CYL_DOWN_SENS
				|| myPs->code == G_CD_FAULT_J_LATCH_CYL_OPEN_TIMEOUT
				|| myPs->code == G_CD_FAULT_J_LATCH_CYL_CLOSE_TIMEOUT
				|| myPs->code == G_CD_FAULT_J_LATCH_CYL_OPEN_SENS
				|| myPs->code == G_CD_FAULT_J_LATCH_CYL_CLOSE_SENS
				|| myPs->code == G_CD_FAULT_J_GRIP_CYL_OPEN_TIMEOUT
				|| myPs->code == G_CD_FAULT_J_GRIP_CYL_CLOSE_TIMEOUT
				|| myPs->code == G_CD_FAULT_J_GRIP_CYL_OPEN_SENS
				|| myPs->code == G_CD_FAULT_J_GRIP_CYL_CLOSE_SENS
				|| myPs->code == G_CD_FAULT_J_TRAY_STATUS_SENS
				|| myPs->code == G_CD_FAULT_J_TRAY_DIR_SENS
				|| myPs->code == G_CD_FAULT_J_DOOR_SENS
				|| myPs->code == G_CD_FAULT_J_STK_SIGNAL_INVALID
				|| myPs->code == G_CD_FAULT_J_STK_UNLOADING_END
				|| myPs->code == G_CD_FAULT_J_TRAY_UNLOAD
				|| myPs->code == B_CD_FAULT_ADC
				) { //Pause
				myPs->signal[M_SIG_EXIT_TYPE] = P5;
				myPs->signal[M_SIG_EXIT_GROUP] = (unsigned char)RecvMsg->val[1];

				myPs->signal[M_SIG_ALARM_SET] = P1;
				myPs->signal[M_SIG_BUZZER_SET] = P1;
			} else if(myPs->code == M_CD_NONE
				|| myPs->code == M_CD_FAULT_NORMAL_TERMINAL_QUIT
				) { //Quit
				myPs->signal[M_SIG_EXIT_TYPE] = P1;
			} else if(myPs->code == M_CD_FAULT_FORCE_TERMINAL_QUIT
				) { //force Quit
				myPs->signal[M_SIG_EXIT_TYPE] = P3;
			} else if(myPs->code == M_CD_FAULT_FORCE_POWER_OFF
				|| myPs->code == M_CD_FAULT_FORCE_TERMINAL_HALT
				) { //force Shutdown
				myPs->signal[M_SIG_EXIT_TYPE] = P4;
			} else { //Shutdown
				myPs->signal[M_SIG_EXIT_TYPE] = P2;

				if(myPs->code != M_CD_FAULT_NORMAL_POWER_OFF) {
					myPs->signal[M_SIG_ALARM_SET] = P1;
					myPs->signal[M_SIG_BUZZER_SET] = P1;
				}
			}
			//myPs->signal[M_SIG_EXIT_VALUE] = (unsigned char)RecvMsg->val[1];
			myPs->misc.exit_value = RecvMsg->val[1];
			myPs->misc.exit_value_2 = RecvMsg->val[2];
			myPs->signal[M_SIG_EXIT_PHASE] = P1;
			break;
		case MSG_IO_MODULE_CH_PAUSE:	//csk_190620
			ch = RecvMsg->val[1];
			if((myData->cData[ch].op.state == C_RUN)
				&& (myData->cData[ch].signal[C_SIG_MODULE_FAULT] == P0)) {
				myData->cData[ch].signal[C_SIG_MODULE_FAULT] = P3;
				myData->cData[ch].op.code = RecvMsg->val[0];
			}
			break;
		default: break;
	}
}

void msgParsing_Meter_to_Module(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int ch;
	
	switch(RecvMsg->msg) {
		case MSG_METER_MODULE_REQUEST_REPLY:
			ch = RecvMsg->val[0];
			myData->cData[ch].signal[C_SIG_METER_REPLY] = P1;
			break;
		default: break;
	}
}

void msgParsing_JigM_to_Module(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int group, toPs1;
	S_MSG_VAL SendMsg;

	toPs1 = MODULE_TO_COB1;
	
	switch(RecvMsg->msg) {
		case MSG_JIGM_MODULE_CONTACT_COMPLETE:
			group = RecvMsg->val[0];
			if(myData->gData[group].state == G_RUN) {
				myData->gData[group].signal[G_SIG_CONTACT_COMPLETE] = P1;
			}
			break;
		case MSG_JIGM_MODULE_DISCONTACT_COMPLETE:
			group = RecvMsg->val[0];
			if(myData->gData[group].state == G_RUN) {
				myData->gData[group].signal[G_SIG_DISCONTACT_COMPLETE] = P1;
			}
			break;
		case MSG_JIGM_MODULE_FAIL:
			group = RecvMsg->val[0];
			myData->gData[group].code = (unsigned char)RecvMsg->val[1];
			myData->gData[group].signal[G_SIG_JIG_FAIL] = P1;
			break;
		case MSG_JIGM_MODULE_START_BUTTON:
			group = RecvMsg->val[0];
			toPs1 += group;
			if(myData->gData[group].state == G_STANDBY){
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_COB_START;
				SendMsg.val[0] = group;
				send_msg(toPs1, (char *)&SendMsg);
			}
			break;
		case MSG_JIGM_MODULE_STOP_BUTTON:
			group = RecvMsg->val[0];
			toPs1 += group;
			if(myData->gData[group].state == G_RUN
				&& (myData->gData[group].phase >= P10 
				&& myData->gData[group].phase < P20)) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_COB_STOP;
				SendMsg.val[0] = group;
				send_msg(toPs1, (char *)&SendMsg);
			}
			break;
		case MSG_JIGM_MODULE_CONTINUE_BUTTON:
			group = RecvMsg->val[0];
			toPs1 += group;
			if(myData->gData[group].state == G_PAUSE) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_COB_CONTINUE;
				SendMsg.val[0] = group;
				send_msg(toPs1, (char *)&SendMsg);
			}
			break;
		default: break;
	}
}

void msgParsing_App_to_JigM(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int group, jig, toPs;
	S_MSG_VAL SendMsg;

	group = RecvMsg->val[0];
	jig = groupNo_to_jigNo(group);
	if(jig > 0) jig--;
	else return;

	switch(jig) {
		case 0: toPs = JIGM_TO_JIGC1;	break;
		case 1: toPs = JIGM_TO_JIGC2;	break;
		default: toPs = JIGM_TO_JIGC1;	break;
	}
	
	switch(RecvMsg->msg) {
		case MSG_APP_JIGM_TRY_CONTACT:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_TRY_CONTACT;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_APP_JIGM_TRY_DISCONTACT:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_TRY_DISCONTACT;
			send_msg(toPs, (char *)&SendMsg);
			break;
		default: break;
	}
}

void msgParsing_Module_to_JigM(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int group, jig, toPs;
	S_MSG_VAL SendMsg;

	group = RecvMsg->val[0];
	jig = groupNo_to_jigNo(group);
	if(jig > 0) jig--;
	else return;

	switch(jig) {
		case 0: toPs = JIGM_TO_JIGC1;	break;
		case 1: toPs = JIGM_TO_JIGC2;	break;
		default: toPs = JIGM_TO_JIGC1;	break;
	}
	
	switch(RecvMsg->msg) {
		case MSG_MODULE_JIGM_TRY_CONTACT:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_TRY_CONTACT;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_TRY_DISCONTACT:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_TRY_DISCONTACT;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_LAMP:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_LAMP;
			SendMsg.val[0] = group;
			SendMsg.val[1] = RecvMsg->val[1]; //state
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_DOOR_OPEN:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_DOOR_OPEN;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_DOOR_CLOSE:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_DOOR_CLOSE;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_PLC_ALARM_AREA_SENSOR:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_PLC_ALARM_AREA_SENSOR;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_PLC_ALARM_MANUAL_DOOR_OPEN:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_PLC_ALARM_MANUAL_DOOR_OPEN;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_PLC_ALARM_AIR:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_PLC_ALARM_AIR;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_PLC_ALARM_APR_S:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_PLC_ALARM_APR_S;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_PLC_ALARM_REF:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_PLC_ALARM_REF;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_PLC_ALARM_COIL_TRIP:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_PLC_ALARM_COIL_TRIP;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_PLC_ALARM_TEMP:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_PLC_ALARM_TEMP;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_PLC_ALARM_SMOKE:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_PLC_ALARM_SMOKE;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_PLC_ALARM_FIRE:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_PLC_ALARM_FIRE;
			send_msg(toPs, (char *)&SendMsg);
			break;
		case MSG_MODULE_JIGM_PLC_ALARM_EMG:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_JIGM_JIGC_PLC_ALARM_EMG;
			send_msg(toPs, (char *)&SendMsg);
			break;
		default: break;
	}
}
/*kjg_w
void msgParsing_Module_to_G_JigM(int fromPs, int msg, int val1, int val2, int idx)
{
	int group, jig, toPs, count, totalGroup;

	totalGroup = myData->AppControl.config.totalGroup;
	switch(msg) {
		case MSG_MODULE_JIGM_TRY_DISCONTACT:
			count = 0;
			for(group = 0; group < totalGroup; group++){
				if((myData->gData[group].state == G_RUN 
					&&myData->gData[group].microState > P30)
					|| myData->gData[group].state != G_RUN) count++;
			}
			if(totalGroup == count){
				for(group = 0; group <= MAX_GROUP_PER_MODULE; group++){
					jig = groupNo_to_jigNo(group);
					if(jig > 0) jig--;
					else continue;
					switch(jig) {
						case 0: toPs = JIGM_TO_JIG1C;	break;
						case 1: toPs = JIGM_TO_JIG2C;	break;
						case 2: toPs = JIGM_TO_JIG3C;	break;
						case 3: toPs = JIGM_TO_JIG4C;	break;
						case 4: toPs = JIGM_TO_JIG5C;	break;
						case 5: toPs = JIGM_TO_JIG6C;	break;
						case 6: toPs = JIGM_TO_JIG7C;	break;
						case 7: toPs = JIGM_TO_JIG8C;	break;
						default: toPs = JIGM_TO_JIG1C;	break;
					}
					send_msg(toPs, MSG_JIGM_JIGC_TRY_DISCONTACT, 0, 0);
				}
			}
			break;
		default: break;
	}
}*/

void msgParsing_JigC_to_JigM(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int group, jig, comPort;
	S_MSG_VAL SendMsg;

	switch(fromPs) {
		case JIGC1_TO_JIGM: jig = 0; break;
		case JIGC2_TO_JIGM: jig = 1; break;
		default: jig = 0; break;
	}

	switch(RecvMsg->msg) {
		case MSG_JIGC_JIGM_CONTACT_COMPLETE:
			group = jigNo_to_groupNo(jig);
			if(group > 0) {
				group--;
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGM_MODULE_CONTACT_COMPLETE;
				SendMsg.val[0] = group;
				send_msg(JIGM_TO_MODULE, (char *)&SendMsg);
			}
			break;
		case MSG_JIGC_JIGM_DISCONTACT_COMPLETE:
			group = jigNo_to_groupNo(jig);
			if(group > 0) {
				group--;
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGM_MODULE_DISCONTACT_COMPLETE;
				SendMsg.val[0] = group;
				send_msg(JIGM_TO_MODULE, (char *)&SendMsg);
			}
			break;
		case MSG_JIGC_JIGM_FAIL:
			group = jigNo_to_groupNo(jig);
			if(group > 0) {
				group--;
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGM_MODULE_FAIL;
				SendMsg.val[0] = group;
				SendMsg.val[1] = RecvMsg->val[0];
				send_msg(JIGM_TO_MODULE, (char *)&SendMsg);
			}
			break;
		case MSG_JIGC_JIGM_START_BUTTON:
			group = jigNo_to_groupNo(jig);
			if(group > 0) {
				group--;
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGM_MODULE_START_BUTTON;
				SendMsg.val[0] = group;
				SendMsg.val[1] = RecvMsg->val[0];
				send_msg(JIGM_TO_MODULE, (char *)&SendMsg);
			}
			break;
		case MSG_JIGC_JIGM_STOP_BUTTON:
			group = jigNo_to_groupNo(jig);
			if(group > 0) {
				group--;
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGM_MODULE_STOP_BUTTON;
				SendMsg.val[0] = group;
				SendMsg.val[1] = RecvMsg->val[0];
				send_msg(JIGM_TO_MODULE, (char *)&SendMsg);
			}
			break;
		case MSG_JIGC_JIGM_CONTINUE_BUTTON:
			group = jigNo_to_groupNo(jig);
			if(group > 0) {
				group--;
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_JIGM_MODULE_CONTINUE_BUTTON;
				SendMsg.val[0] = group;
				SendMsg.val[1] = RecvMsg->val[0];
				send_msg(JIGM_TO_MODULE, (char *)&SendMsg);
			}
			break;
		case MSG_JIGC_JIGM_READ_BCR:
			for(comPort=0; comPort < MAX_COM_PORT; comPort++) {
				if(myData->COM.config.functionType[comPort]
					== COM_FUNC_TYPE_BCR1) break;
			}
			if(comPort >= MAX_COM_PORT) break;

			group = RecvMsg->val[0]; //1base groupNo
			myData->COM.com_port[comPort].misc.bcr_reservation[group-1] = P1;
			break;
		default: break;
	}
}

void msgParsing_App_to_IO(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int group, ch;
	S_MSG_VAL SendMsg;

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	switch(RecvMsg->msg) {
		case MSG_APP_IO_EXIT:
			SendMsg.msg = MSG_IO_MODULE_EXIT;
			SendMsg.val[0] = RecvMsg->val[0];
			SendMsg.val[1] = RecvMsg->val[1];
			send_msg(IO_TO_MODULE, (char *)&SendMsg);
			break;
		case MSG_APP_IO_SHUTDOWN:
			outb(0x02, 0x605); //kjg_logic_type_140324 120sec

			group = 0; ch = 0;
			Select_OutPoint(group, ch, O_POWER_OFF, ON);
			
			DisableWDT(); //kjg_logic_type_140324
			break;
		default: break;
	}
}

void msgParsing_Module_to_IO(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int group, ch;

	switch(RecvMsg->msg) {
		case MSG_MODULE_IO_EXIT:
			group = 0; ch = 0;
			if(Read_InPoint(group, ch, I_NO_POWER_OFF) == 0) { //power off
				Select_OutPoint(group, ch, O_RUN_LED, OFF);
				myData->dio.signal[DIO_SIG_REMOTE_PS] = P101;
			} else { //Don't power off
			}
			break;
		case MSG_MODULE_IO_RUN_LED:
			group = 0; ch = 0;
			Select_OutPoint(group, ch,
				O_RUN_LED, (unsigned char)RecvMsg->val[0]);
			break;
		case MSG_MODULE_IO_REMOTE_PS:
			if(RecvMsg->val[0] == ON) {
				myData->dio.signal[DIO_SIG_REMOTE_PS] = P1;
			} else { //off
				myData->dio.signal[DIO_SIG_REMOTE_PS] = P101;
			}
			break;
		default: break;
	}
}

void msgParsing_JigC_to_IO(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	//int comPort=0;
	S_MSG_VAL SendMsg;

	switch(RecvMsg->msg) {
		case MSG_JIGC_IO_EMG:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_IO_MODULE_EXIT;
			SendMsg.val[0] = RecvMsg->val[0];
			SendMsg.val[1] = RecvMsg->val[1];
			send_msg(IO_TO_MODULE, (char *)&SendMsg);
			break;
		case MSG_JIGC_IO_READ_BCR:
			//send_cmd_bcr1_request(comPort, 0, 0);
			//send_cmd_bcr1_request(comPort, 0, 1);
			break;
		default: break;
	}
}

void msgParsing_App_to_Meter(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int comPort=0, comPort2; //tmp;
	int functionModel = 0;	//jhkw_131209

	if(myData->COM.com_port[comPort].misc.externPort_useFlag == P0) {
		comPort2 = comPort;
	} else {
		comPort2 = myData->COM.config.externPort[comPort] - 1;
	}

	switch(RecvMsg->msg) {
		case MSG_APP_METER_INITIALIZE:
			/*kjg_110814 if(RecvMsg->val[0] == CALI_TYPE_VOLTAGE) {
				myData->COM.com_port[comPort2]
					.signal[COM_SIG_CALI_METER_INITIALIZE] = P1;
			} else { //CALI_TYPE_CURRENT
				myData->COM.com_port[comPort2]
					.signal[COM_SIG_CALI_METER_INITIALIZE] = P11;
			}*/
			comPort = Find_comPort(COM_FUNC_TYPE_METER1);
			if(comPort >= 0) {
				if(RecvMsg->val[0] == CALI_TYPE_VOLTAGE) {
					myData->COM.com_port[comPort]
						.signal[COM_SIG_CALI_METER_INITIALIZE] = P1;
				} else {
					myData->COM.com_port[comPort]
						.signal[COM_SIG_CALI_METER_INITIALIZE] = P11;
				}
			}
			break;
		case MSG_APP_METER_REQUEST:
			//send_cmd_request_AGILENT_34401A(comPort2);
			//break;
			//jhkw_131209s
			functionModel = myData->COM.config.functionModel[comPort];
			switch(functionModel) {
				case 0:	//34401A
			        send_cmd_request_AGILENT_34401A(comPort2);
			        break;
				case 1:	//4700
			        send_cmd_request_VITREK_4700(comPort2);
					break;
			    default:
			        break;
			}
			//jhkw_131209e
			break;
		/*kjg_w case MSG_APP_METER_TEST:
			send_cmd_display_value(0, RecvMsg->val[0], 1);
			break;
		case MSG_APP_METER_DISPLAY_START:
			tmp = 0;
			if(myPs->config.functionType == 0) {
				tmp = -1;
			} else {
				if(myPs->config.autoStart2 == 0) tmp = -1;
			}
			if(tmp < 0) break;

			myPs->signal[CALI_METER_SIG_DISPLAY] = P1;
			break;*/
		default: break;
	}
}

void msgParsing_COA_to_Meter(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int comPort=0, comPort2;

	if(myData->COM.com_port[comPort].misc.externPort_useFlag == P0) {
		comPort2 = comPort;
	} else {
		comPort2 = myData->COM.config.externPort[comPort] - 1;
	}

	switch(RecvMsg->msg) {
		case MSG_COA_METER_INITIALIZE:
			if(RecvMsg->val[0] == CALI_TYPE_VOLTAGE) {
				myData->COM.com_port[comPort2]
					.signal[COM_SIG_CALI_METER_INITIALIZE] = P21;
				myData->COM.com_port[comPort2]
					.signal[COM_SIG_CALI_VOLTAGE_RANGE]
					= (unsigned char)RecvMsg->val[1];
			} else { //CALI_TYPE_CURRENT
				myData->COM.com_port[comPort2]
					.signal[COM_SIG_CALI_METER_INITIALIZE] = P31;
				myData->COM.com_port[comPort2]
					.signal[COM_SIG_CALI_CURRENT_RANGE]
					= (unsigned char)RecvMsg->val[1];
			}
			break;
		default: break;
	}
}

void msgParsing_COC_to_Meter(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int comPort=0, comPort2;

	if(myData->COM.com_port[comPort].misc.externPort_useFlag == P0) {
		comPort2 = comPort;
	} else {
		comPort2 = myData->COM.config.externPort[comPort] - 1;
	}

	switch(RecvMsg->msg) {
		case MSG_COC_METER_INITIALIZE:
			if(RecvMsg->val[0] == CALI_TYPE_VOLTAGE) {
				myData->COM.com_port[comPort2]
					.signal[COM_SIG_CALI_METER_INITIALIZE] = P21;
				myData->COM.com_port[comPort2]
					.signal[COM_SIG_CALI_VOLTAGE_RANGE]
					= (unsigned char)RecvMsg->val[1];
			} else { //CALI_TYPE_CURRENT
				myData->COM.com_port[comPort2]
					.signal[COM_SIG_CALI_METER_INITIALIZE] = P31;
				myData->COM.com_port[comPort2]
					.signal[COM_SIG_CALI_CURRENT_RANGE]
					= (unsigned char)RecvMsg->val[1];
			}
			break;
		default: break;
	}
}

void msgParsing_Module_to_Meter(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int comPort=0, comPort2;

	if(myData->COM.com_port[comPort].misc.externPort_useFlag == P0) {
		comPort2 = comPort;
	} else {
		comPort2 = myData->COM.config.externPort[comPort] - 1;
	}

	switch(RecvMsg->msg) {
		case MSG_MODULE_METER_REQUEST:
			myPs->misc.receivedCh = RecvMsg->val[0];
			myData->COM.com_port[comPort2]
				.signal[COM_SIG_CALI_METER_REQUEST_PHASE] = P1;
			break;
		default: break;
	}
}

void msgParsing_App_to_Meter2(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int comPort=1, comPort2, tmp, i;

	if(myData->COM.com_port[comPort].misc.externPort_useFlag == P0) {
		comPort2 = comPort;
	} else {
		comPort2 = myData->COM.config.externPort[comPort] - 1;
	}

	switch(RecvMsg->msg) {
		case MSG_APP_METER2_INITIALIZE:
			if(RecvMsg->val[0] == 1) {
				if(RecvMsg->val[1] == 1) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P1;
				} else if(RecvMsg->val[1] == 2) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P3;
				} else if(RecvMsg->val[1] == 3) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P5;
				} else if(RecvMsg->val[1] == 4) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P7;
				}
			} else if(RecvMsg->val[0] == 2) {
				if(RecvMsg->val[1] == 1) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P11;
				} else if(RecvMsg->val[1] == 2) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P13;
				} else if(RecvMsg->val[1] == 3) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P15;
				} else if(RecvMsg->val[1] == 4) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P17;
				}
			} else if(RecvMsg->val[0] == 3) {
				if(RecvMsg->val[1] == 1) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P21;
				} else if(RecvMsg->val[1] == 2) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P23;
				} else if(RecvMsg->val[1] == 3) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P25;
				} else if(RecvMsg->val[1] == 4) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P27;
				}
			} else if(RecvMsg->val[0] == 4) {
				if(RecvMsg->val[1] == 1) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P31;
				} else if(RecvMsg->val[1] == 2) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P33;
				} else if(RecvMsg->val[1] == 3) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P35;
				} else if(RecvMsg->val[1] == 4) {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_INITIALIZE] = P37;
				}
			}
			break;
		case MSG_APP_METER2_REQUEST:
			send_cmd_request_CB_7018(RecvMsg->val[0], RecvMsg->val[1],
				comPort2);
			break;
		case MSG_APP_METER2_MEASURE:
			tmp = 0;
			if(myData->COM.config.functionType[comPort2] == COM_FUNC_TYPE_METER2
				&& myData->COM.config.functionModel[comPort2] == 0) {
				if(myData->COM.config.autoStart[comPort2] == 1) tmp = -1;
				else {
					for(i=0; i < myData->COM.config.countMeter[comPort2]; i++) {
						send_cmd_close_CB_7018(i+1, comPort2);
						usleep(100000);
					}
				}
			} else {
				tmp = -1;
			}
			if(tmp < 0) break;

			if(RecvMsg->val[0] == 0) {
				if(myData->COM.config.countMeter[comPort2] > 0) {
					if(RecvMsg->val[1] == 1 && RecvMsg->val[1]
						<= myData->COM.config.countMeter[comPort2]) {
						myData->COM.com_port[comPort2]
							.signal[COM_SIG_ANALOG_METER_MEASURE] = P1;
					} else if(RecvMsg->val[1] == 2 && RecvMsg->val[1]
						<= myData->COM.config.countMeter[comPort2]) {
						myData->COM.com_port[comPort2]
							.signal[COM_SIG_ANALOG_METER_MEASURE] = P2;
					} else if(RecvMsg->val[1] == 3 && RecvMsg->val[1]
						<= myData->COM.config.countMeter[comPort2]) {
						myData->COM.com_port[comPort2]
							.signal[COM_SIG_ANALOG_METER_MEASURE] = P3;
					} else if(RecvMsg->val[1] == 4 && RecvMsg->val[1]
						<= myData->COM.config.countMeter[comPort2]) {
						myData->COM.com_port[comPort2]
							.signal[COM_SIG_ANALOG_METER_MEASURE] = P4;
					}
				} else {
					myData->COM.com_port[comPort2]
						.signal[COM_SIG_ANALOG_METER_MEASURE] = P0;
				}
			} else {
				myData->COM.com_port[comPort2]
					.signal[COM_SIG_ANALOG_METER_MEASURE] = P0;
			}
			break;
		default: break;
	}
}

void send_save_msg(int ch)
{
	int i, j, group;
#ifdef __10MS__
	unsigned char msg, msg_1;	//jhkw_190830
#endif

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_400A_200A_100A_10A_2:
			group = (int)myData->mData.signal[M_SIG_RUNNING_GROUP];
			break;
		case F_SDI_5V_450A_200A_100A_10A:
			group = 0;
			break;
		case C_CANSYSTEM_60V_350A_100A_21KW: //kjg_140916
		case C_LGE_450V_250A_50A_112KW:		//jhk_150703
		case C_LGE_450V_250A_50A_225KW:		//jhk_150605
		case C_LGE_450V_250A_50A_225KW_2:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_3:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_4:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_5:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_6:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_7:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_8:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_9:	//jhk_150605
		case C_LGE_500V_250A_50A_250KW:	//jhk_150607
		case C_LGE_600V_250A_50A_150KW:	//jhk_150520
			if(myData->cData[ch].signal[C_SIG_EXT_OP_TYPE] == P0) { //Normal
			} else { //E_CAN_2
				return;
			}
			break;
		default:
			if(ch < myPs->config.chInGroup[0]) group = 0;
			else group = 1; //kjg_w
			break;
	}

	myData->cData[ch].op.save_flag = P1;

	i = 0;
	j = 0;
	//debug_size_cob
/*	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
			i = (int)myData->gData[group].misc.scan_ch[ch];
			j = (int)myData->gData[group].misc.scan_step[ch];
			myData->COB_opSave[group][i][j] = myData->cData[ch].op;
			myData->COB_opSave[group][i][j].temp = myData->cData[i].op.temp;
			break;
		case F_SDI_5V_450A_200A_100A_10A:
			if(ch == 0) {
				i = (int)myData->gData[group].misc.scan_ch[ch];
			} else {
				i = (int)myData->gData[group].misc.scan_ch[ch] + 12;
			}
			j = (int)myData->gData[group].misc.scan_step[ch];
			myData->COB_opSave[group][i][j] = myData->cData[ch].op;
			myData->COB_opSave[group][i][j].temp = myData->cData[i].op.temp;
			break;
		default:
			break;
	}
*/

	myData->cData[ch].opSave = myData->cData[ch].op;

	if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COA][0] == P1) {
		if(myData->ChAttribute[ch].chNo_master == 0) return;

		myData->cData[ch].op.resultIndex++;
		send_save_msg_2(ch, 0); //COA1_Client	
		//jhkw_190830s
#ifdef __10MS__
		if(myData->COA_Client[0].config.data_save_10ms == 1) {
			msg = myData->signal_10ms[ch].msg_count;
			if(msg >= MAX_10MS_SAVE_MSG_RING) {
		//		|| (myData->signal_10ms[ch].save_start_flag == 1)) {
				msg = 0;
		//	} else if(myData->save_msg_10ms[ch][msg].count_100 >= 100) {
			}
			if(myData->save_msg_10ms[ch][msg].count_100 >= 100) {
				if(msg >= (MAX_10MS_SAVE_MSG_RING - 1)) {
					msg = 0;
				} else {
					msg++;
				}
			}
			msg_1 = myData->signal_10ms[ch].response_msg_count;
			if((myData->signal_10ms[ch].response_msg_count 
				== myData->signal_10ms[ch].send_msg_count)
				&& (myData->save_msg_10ms[ch][msg].count_100 >= 80)
				&& (msg != msg_1)) {
				myData->save_msg_10ms[ch][msg_1].count_100 = 0;
				myData->save_msg_10ms[ch][msg_1].write_idx = 0;
				myData->save_msg_10ms[ch][msg_1].read_idx = 0;
			}
			if(msg == (MAX_10MS_SAVE_MSG_RING - 1)) {
				if(myData->save_msg_10ms[ch][msg].count_100 >= 50) {
					if(myData->save_msg_10ms[ch][0].count_100 != 0) {
						myData->COA_Client[0]
						.signal[P1_SIG_10MS_TO_SBC_RESPONSE_CHECK] = 2;
					}
				}
			//} else if(msg == (MAX_10MS_SAVE_MSG_RING - 2)) {
			} else {
				if(myData->save_msg_10ms[ch][msg + 1].count_100 != 0) {
					myData->COA_Client[0]
					.signal[P1_SIG_10MS_TO_SBC_RESPONSE_CHECK] = 2;
				}
			}
			send_save_msg_2_10ms(ch, msg); //COA1_Client
			myData->signal_10ms[ch].msg_count = msg;
		} else {
			send_save_msg_2(ch, 1); //DataSave
		}
#else
		send_save_msg_2(ch, 1); //DataSave
#endif
		//jhkw_190830e
	}

#ifdef __COC__
	if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COC][0] == P1) {
		if(myData->ChAttribute[ch].chNo_master == 0) return;
		if(myData->cData[ch].op.attribute >= ATTR_CHECK4_START
			&& myData->cData[ch].op.attribute <= ATTR_CHECK4_END) {
			if(myData->cData[ch].op.select == SAVE_FLAG_SAVING_END) {
				i = myData->cData[ch].op.idxStepNo;
				myPs->misc.cable_check.Vsens[ch][i]
					= myData->cData[ch].op.Vsens;
				myPs->misc.cable_check.Isens[ch][i]
					= myData->cData[ch].op.Isens;
				myPs->misc.cable_check.v_power_bus[ch][i]
					= myData->fch.ch[ch].v_power_bus;
				myPs->misc.cable_check.v_output_bus[ch][i]
					= myData->fch.ch[ch].v_output_bus;
			}
			return;
		}

		myData->cData[ch].op.resultIndex++;
		send_save_msg_3(ch, 0); //COC1_Client
		send_save_msg_3(ch, 1); //DataSave
	}
#endif
}

void send_save_msg_2(int ch, int msg)
{
	//unsigned char item, idxStepNo;
	//int idx, group, i, j, k;
	//long val;
	//int idx, idx2, group, i, j, k;
	//long val, val2, refI, refI2, tmp_refI, tmp_i;
	//long val, val2, refI, refI2;
	unsigned char item, save_type;
	int idx, group, i, j, k, idxStepNo, rangeI;
	long val, val2, refI, tmp_refI, v_val, i_val;
	long max_refI, min_refI, rangeI_val, tmp_rangeI, min_refI2;
	double tmp_val;

	group = 0; //kjg_w

	if(myData->save_msg[msg].send_flag == 1) { //send save_msg stop
		return;
	}

	if(myData->save_msg[msg].total_count >= (MAX_SAVE_MSG - 50)) {
		if(myData->cData[ch].op.select != SAVE_FLAG_SAVING_END) {
			return;
		} else {
			if(myData->save_msg[msg].total_count >= (MAX_SAVE_MSG - 5)) {
				return;
			}
		}
	}

	idx = myData->save_msg[msg].write_idx;
	idx++;
	if(idx >= MAX_SAVE_MSG) idx = 0;

	switch(myData->AppControl.config.systemModel) {
		case C_SKI_60V_400A_200A_100A_96KW:		//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_2:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_3:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_4:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_5:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_6:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_7:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_8:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_9:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_10:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_11:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_12:	//ktg_190319
		case C_SKI_120V_400A_200A_100A_50A_192KW:
		case C_SKI_120V_400A_200A_100A_50A_192KW_2:
		case C_SKI_120V_400A_200A_100A_50A_192KW_3:
		case C_SKI_120V_400A_200A_100A_50A_192KW_4:
		case C_SKI_120V_400A_200A_100A_50A_192KW_5:
		case C_SKI_120V_400A_200A_100A_50A_192KW_6:
		case C_SKI_120V_400A_200A_100A_50A_192KW_7:
		case C_SKI_120V_400A_200A_100A_50A_192KW_8:
		case C_SKI_120V_400A_200A_100A_50A_192KW_9:
		case C_SKI_120V_400A_200A_100A_50A_192KW_10:
		case C_SKI_120V_400A_200A_100A_50A_192KW_11:
		case C_SKI_120V_400A_200A_100A_50A_192KW_12:
		case C_SKI_120V_400A_200A_100A_50A_192KW_13:
		case C_SKI_120V_400A_200A_100A_50A_192KW_14:
		case C_SKI_120V_400A_200A_100A_50A_192KW_15:
		case C_SKI_120V_400A_200A_100A_50A_192KW_16:
		case C_SKI_120V_400A_200A_100A_50A_192KW_17:
		case C_SKI_120V_400A_200A_100A_50A_192KW_18:
		case C_SKI_120V_400A_200A_100A_192KW:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_2:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_3:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_4:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_5:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_6:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_7:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_8:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_9:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_10:	//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_11:	//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_12:	//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_13:	//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_14:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_15:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_16:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_17:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_18:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_19:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_20:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_21:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_22:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_23:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_24:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_25:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_26:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_27:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_28:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_29:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_30:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_31:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_32:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_33:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_34:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_35:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_36:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_37:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_38:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_39:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_40:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_41:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_42:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_43:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_44:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_45:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_46:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_47:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_48:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_49:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_50:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_51:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_52:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_53:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_54:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_55:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_56:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_57:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_58:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_59:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_60:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_61:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_62:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_63:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_64:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_65:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_66:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_67:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_68:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_69:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_70:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_71:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_72:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_73:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_74:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_75:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_76:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_77:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_78:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_79:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_80:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_81:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_82:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_83:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_84:	//ktg_190805
	//	case C_CTP_150V_150A_100A_90KW:
			save_type = 1; //kjhw_180325
			break;
		default:
			save_type = 0;
			break;
	}
	if(myData->cData[ch].signal[C_SIG_SEND_SAVE_MSG_DIVISION] == P1) { //kjg_170810
		if(idx == 0) i = MAX_SAVE_MSG - 1;
		else i = idx - 1;

		memcpy((char *)&myData->save_msg[msg].val[idx],
			(char *)&myData->save_msg[msg].val[i], sizeof(S_SAVE_MSG_VAL));

		myData->save_msg[msg].val[idx].chData.resultIndex
			= myData->cData[ch].op.resultIndex;
		myData->save_msg[msg].val[idx].chData.select
			= myData->cData[ch].op.select;
		myData->save_msg[msg].val[idx].chData.code
			= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P1,
			(long)myData->cData[ch].op.code);

		myData->save_msg[msg].write_idx = idx;
		myData->save_msg[msg].total_count++;
		myData->save_msg[msg].count[ch]++;
		return;
	}
	
	myData->save_msg[msg].val[idx].chData.ch = (unsigned char)(ch + 1);
	myData->save_msg[msg].val[idx].chData.resultIndex
		= myData->cData[ch].op.resultIndex;
	myData->save_msg[msg].val[idx].chData.state
		= (unsigned char)convert_ch_state(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.state);
	myData->save_msg[msg].val[idx].chData.stepType
		= (unsigned char)convert_step_type(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.stepType);
	myData->save_msg[msg].val[idx].chData.stepMode
		= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.stepMode);
	
//kjhw_150616s
#ifdef __COA_VER_100B__
	if(myData->cData[ch].op.runTime <= 10) {
#else //COA_VER_100B2~
	if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
		myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
	 	//0, 10) > 0) {
		0, 15) > 0) { //kjhw_180325
#endif
		myData->save_msg[msg].val[idx].chData.Vsens
			= myData->cData[ch].misc.tmpVsens;
//kjhw_150616e
	} else {
		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].misc.checkDelayTime_day,
			myData->cData[ch].misc.checkDelayTime,
			0, 5) > 0) { //kjhw_180325
			myData->save_msg[msg].val[idx].chData.Vsens
				= myData->cData[ch].misc.tmpVsens;
		} else {
			myData->save_msg[msg].val[idx].chData.Vsens
				= myData->cData[ch].op.Vsens;
		}
	}
	if(myData->cData[ch].op.stepType == STEP_END) { //kjhw_180417
		//myData->save_msg[msg].val[idx].chData.Vsens = 0;
	}
	v_val = myData->save_msg[msg].val[idx].chData.Vsens; //kjhw_180325

	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
//kjhw_150616s
#ifdef __COA_VER_100B__
		if(myData->cData[ch].op.runTime <= 10) {
#else //COA_VER_100B2~
		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
			//0, 10) > 0) {
			0, 15) > 0) { //kjhw_180325
#endif
			if(save_type == 1) { //kjhw_180325s
				k = 1;
				val = myData->cData[ch].misc.tmpIsens;
				for(i=0; i < 3; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].misc.tmpIsens;
						k++;
					}
				}
				val2 = 0;
				if(myData->cData[ch].op.stepType == STEP_CHARGE
					|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
					|| myData->cData[ch].op.stepType == STEP_PATTERN) {
					if((myData->cData[ch].misc.cv_select != P1)
						&& (myData->cData[ch].misc.cv_select != P2)) {
						refI = myData->cData[ch].misc.cmd_i[0] * k; //50A
						tmp_refI = refI * 0.05; //5% 2.5A
						rangeI = myData->cData[ch].op.rangeI;
						rangeI_val = myPs->config.maxI[rangeI] * k;
						tmp_rangeI = rangeI_val * 0.0005; //0.05% //
						if(refI >= 0) { //charge
							max_refI = refI + tmp_rangeI; //50.05
							min_refI = refI - tmp_rangeI; //49.95
						} else { //discharge
							max_refI = refI - tmp_rangeI; //-50.025
							min_refI = refI + tmp_rangeI; //-49.975
						}
						min_refI2 = refI - tmp_refI; //50-2.5 = 47.5 chg
						if(refI > 0) { //charge
							if((val >= min_refI2) 
								&& (val >= max_refI || val <= min_refI)) {
								tmp_val = (double)(refI / 1000) * 100; //mA
								tmp_val
									= (double)(val + (tmp_val * 1000)) / 101;
								val = (long)tmp_val;
							}
						} else if(refI < 0) { //discharge
							if((val <= min_refI2) 
								&& (val <= max_refI || val >= min_refI)) {
								tmp_val = (double)(refI / 1000) * 100; //mA
								tmp_val
									= (double)(val + (tmp_val * 1000)) / 101;
								val = (long)tmp_val;
							}
						}
					}
				}
			} else {
				val = myData->cData[ch].misc.tmpIsens;
				for(i=0; i < 3; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].misc.tmpIsens;
					}
				}
			} //kjhw_180325e
//kjhw_150616e
		} else {
			if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
				myData->cData[ch].misc.checkDelayTime_day,
				myData->cData[ch].misc.checkDelayTime,
				0, 5) > 0) { //kjhw_180325
				if(save_type == 1) { //kjhw_180325s
					k = 1;
					val = myData->cData[ch].misc.tmpIsens;
					for(i=0; i < 3; i++) {
						j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
						if(j >= 0) {
							val += myData->cData[j].misc.tmpIsens;
							k++;
						}
					}
					val2 = 0;
					if(myData->cData[ch].op.stepType == STEP_CHARGE
						|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
						|| myData->cData[ch].op.stepType == STEP_PATTERN) {
						if((myData->cData[ch].misc.cv_select != P1)
							&& (myData->cData[ch].misc.cv_select != P2)) {
							refI = myData->cData[ch].misc.cmd_i[0] * k; //50A
							tmp_refI = refI * 0.05; //5% 2.5A
							rangeI = myData->cData[ch].op.rangeI;
							rangeI_val = myPs->config.maxI[rangeI] * k;
							tmp_rangeI = rangeI_val * 0.0005; //0.05% //
							if(refI >= 0) { //charge
								max_refI = refI + tmp_rangeI; //50.05
								min_refI = refI - tmp_rangeI; //49.95
							} else { //discharge
								max_refI = refI - tmp_rangeI; //-50.025
								min_refI = refI + tmp_rangeI; //-49.975
							}
							min_refI2 = refI - tmp_refI; //50-2.5 = 47.5 chg
							if(refI > 0) { //charge
								if((val >= min_refI2) 
									&& (val >= max_refI || val <= min_refI)) {
									tmp_val = (double)(refI / 1000) * 100; //mA
									tmp_val
										= (double)(val + (tmp_val * 1000))
										/ 101;
									val = (long)tmp_val;
								}
							} else if(refI < 0) { //discharge
								if((val <= min_refI2) 
									&& (val <= max_refI || val >= min_refI)) {
									tmp_val = (double)(refI / 1000) * 100; //mA
									tmp_val
										= (double)(val + (tmp_val * 1000))
										/ 101;
									val = (long)tmp_val;
								}
							}
						}
					}
				} else {
					val = myData->cData[ch].misc.tmpIsens;
					for(i=0; i < 3; i++) {
						j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
						if(j >= 0) {
							val += myData->cData[j].misc.tmpIsens;
						}
					}
				} //kjhw_180325e
			} else {
				val = myData->cData[ch].op.Isens;
				for(i=0; i < 3; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].op.Isens;
					}
				}
			}
		}
		myData->save_msg[msg].val[idx].chData.Isens = val;
		if(myData->cData[ch].op.stepType == STEP_END) { //kjhw_180417
			//myData->save_msg[msg].val[idx].chData.Isens = 0;
		}
		i_val = myData->save_msg[msg].val[idx].chData.Isens; //kjhw_180325

		val = myData->cData[ch].op.charge_AmpareHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_AmpareHour;
			}
		}
		myData->save_msg[msg].val[idx].chData.charge_AmpareHour = val;

		val = myData->cData[ch].op.discharge_AmpareHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_AmpareHour;
			}
		}
		myData->save_msg[msg].val[idx].chData.discharge_AmpareHour = val;

#ifdef __COA_VER_100B__
		if(myData->cData[ch].op.runTime <= 10) {
#else //COA_VER_100B2~
		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
		 	//0, 10) > 0) {
			0, 15) > 0) { //kjhw_180325
#endif
			if(save_type == 1) { //kjhw_180325s
				val = myData->cData[ch].misc.tmpWatt;
				for(i=0; i < 3; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].misc.tmpWatt;
					}
				}
				if(myData->cData[ch].op.stepType == STEP_CHARGE
					|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
					|| myData->cData[ch].op.stepType == STEP_PATTERN) {
					if((myData->cData[ch].misc.cv_select != P1)
						&& (myData->cData[ch].misc.cv_select != P2)) {
						tmp_val = ((double)v_val / 1000.0)
							* ((double)i_val / 1000.0);
						val = (long)(tmp_val / 1000.0);
					}
				}
			} else {
				val = myData->cData[ch].misc.tmpWatt;
				for(i=0; i < 3; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].misc.tmpWatt;
					}
				}
			} //kjhw_180417e
		} else {
			if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
				myData->cData[ch].misc.checkDelayTime_day,
				myData->cData[ch].misc.checkDelayTime,
				0, 5) > 0) { //kjhw_180325
				if(save_type == 1) { //kjhw_180325s
					val = myData->cData[ch].misc.tmpWatt;
					for(i=0; i < 3; i++) {
						j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
						if(j >= 0) {
							val += myData->cData[j].misc.tmpWatt;
						}
					}
					if(myData->cData[ch].op.stepType == STEP_CHARGE
						|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
						|| myData->cData[ch].op.stepType == STEP_PATTERN) {
						if((myData->cData[ch].misc.cv_select != P1)
							&& (myData->cData[ch].misc.cv_select != P2)) {
							tmp_val = ((double)v_val / 1000.0)
								* ((double)i_val / 1000.0);
							val	= (long)(tmp_val / 1000.0);
						}
					}
				} else {
					val = myData->cData[ch].misc.tmpWatt;
					for(i=0; i < 3; i++) {
						j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
						if(j >= 0) {
							val += myData->cData[j].misc.tmpWatt;
						}
					}
				} //kjhw_180325e
			} else {
				val = myData->cData[ch].op.watt;
				for(i=0; i < 3; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].op.watt;
					}
				}
			}
		}
		myData->save_msg[msg].val[idx].chData.watt = val;

		val = myData->cData[ch].op.charge_WattHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_WattHour;
			}
		}
		myData->save_msg[msg].val[idx].chData.charge_WattHour = val;

		val = myData->cData[ch].op.discharge_WattHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_WattHour;
			}
		}
		myData->save_msg[msg].val[idx].chData.discharge_WattHour = val;

		val = myData->cData[ch].op.meanI;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.meanI;
			}
		}
		myData->save_msg[msg].val[idx].chData.avgI = val;
	} else {
//kjhw_150616s
#ifdef __COA_VER_100B__
		if(myData->cData[ch].op.runTime <= 10) {
#else //COA_VER_100B2~
		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
		 	//0, 10) > 0) {
			0, 15) > 0) { //kjhw_180325
#endif
			if(save_type == 1) { //kjhw_180325s
				val = myData->cData[ch].misc.tmpIsens;
				if(myData->cData[ch].op.stepType == STEP_CHARGE
					|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
					|| myData->cData[ch].op.stepType == STEP_PATTERN) {
					if((myData->cData[ch].misc.cv_select != P1)
						&& (myData->cData[ch].misc.cv_select != P2)) {
						refI = myData->cData[ch].misc.cmd_i[0]; //50A
						tmp_refI = refI * 0.05; //5% 2.5A
						rangeI = myData->cData[ch].op.rangeI;
						rangeI_val = myPs->config.maxI[rangeI];
						tmp_rangeI = rangeI_val * 0.0005; //0.05% //
						if(refI >= 0) { //charge
							max_refI = refI + tmp_rangeI; //50.05
							min_refI = refI - tmp_rangeI; //49.95
						} else { //discharge
							max_refI = refI - tmp_rangeI; //-50.025
							min_refI = refI + tmp_rangeI; //-49.975
						}
						min_refI2 = refI - tmp_refI; //50-2.5 = 47.5 chg
						if(refI > 0) { //charge
							if((val >= min_refI2) 
								&& (val >= max_refI || val <= min_refI)) {
								tmp_val = (double)(refI / 1000) * 100; //mA
								tmp_val
									= (double)(val + (tmp_val * 1000)) / 101;
								val = (long)tmp_val;
							}
						} else if(refI < 0) { //discharge
							if((val <= min_refI2) 
								&& (val <= max_refI || val >= min_refI)) {
								tmp_val = (double)(refI / 1000) * 100; //mA
								tmp_val
									= (double)(val + (tmp_val * 1000)) / 101;
								val = (long)tmp_val;
							}
						}
					}
				}
			} else {
				val = myData->cData[ch].misc.tmpIsens;
			} //kjhw_180325e
//kjhw_150616e
		} else {
			if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
				myData->cData[ch].misc.checkDelayTime_day,
				myData->cData[ch].misc.checkDelayTime,
				0, 5) > 0) { //kjhw_180325
				if(save_type == 1) { //kjhw_180325s
					val = myData->cData[ch].misc.tmpIsens;
					if(myData->cData[ch].op.stepType == STEP_CHARGE
						|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
						|| myData->cData[ch].op.stepType == STEP_PATTERN) {
						if((myData->cData[ch].misc.cv_select != P1)
							&& (myData->cData[ch].misc.cv_select != P2)) {
							refI = myData->cData[ch].misc.cmd_i[0]; //50A
							tmp_refI = refI * 0.05; //5% 2.5A
							rangeI = myData->cData[ch].op.rangeI;
							rangeI_val = myPs->config.maxI[rangeI];
							tmp_rangeI = rangeI_val * 0.0005; //0.05% //
							if(refI >= 0) { //charge
								max_refI = refI + tmp_rangeI; //50.05
								min_refI = refI - tmp_rangeI; //49.95
							} else { //discharge
								max_refI = refI - tmp_rangeI; //-50.025
								min_refI = refI + tmp_rangeI; //-49.975
							}
							min_refI2 = refI - tmp_refI; //50-2.5 = 47.5 chg
							if(refI > 0) { //charge
								if((val >= min_refI2) 
									&& (val >= max_refI || val <= min_refI)) {
									tmp_val = (double)(refI / 1000) * 100; //mA
									tmp_val = (double)(val + (tmp_val * 1000))
											/ 101;
									val = (long)tmp_val;
								}
							} else if(refI < 0) { //discharge
								if((val <= min_refI2) 
									&& (val <= max_refI || val >= min_refI)) {
									tmp_val = (double)(refI / 1000) * 100; //mA
									tmp_val = (double)(val + (tmp_val * 1000))
											/ 101;
									val = (long)tmp_val;
								}
							}
						}
					}
				} else {
					val = myData->cData[ch].misc.tmpIsens;
				} //kjhw_180325e
			} else {
				val	= myData->cData[ch].op.Isens;
			}
		}
		myData->save_msg[msg].val[idx].chData.Isens = val;
		if(myData->cData[ch].op.stepType == STEP_END) { //kjhw_180417
			//myData->save_msg[msg].val[idx].chData.Isens = 0;
		}
		i_val = myData->save_msg[msg].val[idx].chData.Isens; //kjhw_180325

		myData->save_msg[msg].val[idx].chData.charge_AmpareHour
			= myData->cData[ch].op.charge_AmpareHour;
		myData->save_msg[msg].val[idx].chData.discharge_AmpareHour
			= myData->cData[ch].op.discharge_AmpareHour;
#ifdef __COA_VER_100B__
		if(myData->cData[ch].op.runTime <= 10) {
#else //COA_VER_100B2~
		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
		 	//0, 10) > 0) {
			0, 15) > 0) { //kjhw_180325
#endif
			if(save_type == 1) { //kjhw_180325s
				val = myData->cData[ch].misc.tmpWatt;
				if(myData->cData[ch].op.stepType == STEP_CHARGE
					|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
					|| myData->cData[ch].op.stepType == STEP_PATTERN) {
					if((myData->cData[ch].misc.cv_select != P1)
						&& (myData->cData[ch].misc.cv_select != P2)) {
						tmp_val = ((double)v_val / 1000.0)
							* ((double)i_val / 1000.0);
						val	= (long)(tmp_val / 1000.0);
					}
				}
			} else {
				val = myData->cData[ch].misc.tmpWatt;
			} //kjhw_180325e
		} else {
			if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
				myData->cData[ch].misc.checkDelayTime_day,
				myData->cData[ch].misc.checkDelayTime,
				0, 5) > 0) { //kjhw_180325
				if(save_type == 1) { //kjhw_180325s
					val = myData->cData[ch].misc.tmpWatt;
					if(myData->cData[ch].op.stepType == STEP_CHARGE
						|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
						|| myData->cData[ch].op.stepType == STEP_PATTERN) {
						if((myData->cData[ch].misc.cv_select != P1)
							&& (myData->cData[ch].misc.cv_select != P2)) {
							tmp_val = ((double)v_val / 1000.0)
								* ((double)i_val / 1000.0);
							val = (long)(tmp_val / 1000.0);
						}
					}
				} else {
					val = myData->cData[ch].misc.tmpWatt;
				} //kjhw_180325e
			} else {
				val = myData->cData[ch].op.watt;
			}
		}
		myData->save_msg[msg].val[idx].chData.watt = val; //kjhw_180325
		myData->save_msg[msg].val[idx].chData.charge_WattHour
			= myData->cData[ch].op.charge_WattHour;
		myData->save_msg[msg].val[idx].chData.discharge_WattHour
			= myData->cData[ch].op.discharge_WattHour;
		myData->save_msg[msg].val[idx].chData.avgI = myData->cData[ch].op.meanI;
	}
	//kjhw_130429s
	switch(myData->AppControl.config.systemModel) {
		//case C_SDI_100V_150A_70A_30KW:
		//case C_SDI_100V_150A_70A_30KW_2:
		case C_HLGP_500V_200A_100A_200KW_2:
		case C_HLGP_500V_200A_100A_200KW_3:
		case C_HLGP_500V_200A_100A_200KW_4:
		case C_HLGP_500V_200A_100A_200KW_5:
		case C_HLGP_500V_300A_100A_300KW:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_2:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_3:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_4:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_5:	//jhk_140305
			if(myData->cData[ch].op.stepType == STEP_PATTERN) {
				if(((myData->cData[ch].op.stepMode == MODE_CC)
					&& (myData->cData[ch].misc.cmd_i[0] == 0))
					|| ((myData->cData[ch].op.stepMode == MODE_CP)
					&& (myData->cData[ch].misc.cmd_p[0] == 0))) {
					myData->save_msg[msg].val[idx].chData.watt = 0;
				}
			}
			break;
		default: break;
	}
	//kjhw_130429e
	myData->save_msg[msg].val[idx].chData.capacitance
		= myData->cData[ch].op.capacitance;
	myData->save_msg[msg].val[idx].chData.z = myData->cData[ch].op.z;

	myData->save_msg[msg].val[idx].chData.select
		= myData->cData[ch].op.select;
	myData->save_msg[msg].val[idx].chData.code
		= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.code);
	myData->save_msg[msg].val[idx].chData.stepNo
		= myData->cData[ch].op.idxStepNo + 1;

#ifdef __COA_VER_100B__
	myData->save_msg[msg].val[idx].chData.runTime
		= myData->cData[ch].op.runTime;
	myData->save_msg[msg].val[idx].chData.totalRunTime
		= myData->cData[ch].op.totalRunTime;
	myData->save_msg[msg].val[idx].chData.cvTime
		= myData->cData[ch].op.cvTime;
	//myData->save_msg[msg].val[idx].chData.ccTime
	//	= myData->cData[ch].op.ccTime; //kjg_w
#else //COA_VER_100B2~
	if(myData->cData[ch].op.stepType == STEP_END) {
		myData->cData[ch].op.runTime_day = 0;
	} //kjhw_180417
	myData->save_msg[msg].val[idx].chData.runTime_day
		= myData->cData[ch].op.runTime_day;
	myData->save_msg[msg].val[idx].chData.totalRunTime_day
		= myData->cData[ch].op.totalRunTime_day;
	myData->save_msg[msg].val[idx].chData.cvTime_day
		= myData->cData[ch].op.cvTime_day;
	//myData->save_msg[msg].val[idx].chData.ccTime_day
	//	= myData->cData[ch].op.ccTime_day; //kjg_w
	//
	if(myData->cData[ch].op.stepType == STEP_END) {
		myData->cData[ch].op.runTime = 0;
	} //kjhw_180417
	myData->save_msg[msg].val[idx].chData.runTime
		= myData->cData[ch].op.runTime;
	myData->save_msg[msg].val[idx].chData.totalRunTime
		= myData->cData[ch].op.totalRunTime;
	myData->save_msg[msg].val[idx].chData.cvTime
		= myData->cData[ch].op.cvTime;
	//myData->save_msg[msg].val[idx].chData.ccTime
	//	= myData->cData[ch].op.ccTime; //kjg_w
#endif

	myData->save_msg[msg].val[idx].chData.realDate
		= myData->mData.real_time[6] * 10000 //year
		+ myData->mData.real_time[5] * 100 //month
		+ myData->mData.real_time[4]; //day
	myData->save_msg[msg].val[idx].chData.realClock
		= myData->mData.real_time[3] * 10000000 //hour
		+ myData->mData.real_time[2] * 100000 //min
		+ myData->mData.real_time[1] * 1000 //sec
		+ myData->mData.real_time[0]; //msec

	myData->save_msg[msg].val[idx].chData.reservedCmd
		= myData->cData[ch].op.reservedCmd;
	myData->save_msg[msg].val[idx].chData.external_comm_state
		= myData->cData[ch].misc.external_comm_state;
	myData->save_msg[msg].val[idx].chData.ch_output_state
		= myData->cData[ch].misc.ch_output_state; //kjg_101219
	myData->save_msg[msg].val[idx].chData.ch_input_state
		= myData->cData[ch].misc.ch_input_state; //kjg_101219

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
	myData->save_msg[msg].val[idx].chData.auxDataCount
		= (short int)(myData->auxDataCount[ch][0]
		+ myData->auxDataCount[ch][1]);
#else //COA_VER_100F~
	//kjh_160610
	myData->save_msg[msg].val[idx].chData.auxDataCount
		= (short int)(myData->auxDataCount[ch][0]
		+ myData->auxDataCount[ch][1] + myData->auxDataCount[ch][2]);
#endif
	myData->save_msg[msg].val[idx].chData.canReceiveDataCount
		= (short int)(myData->canReceiveDataCount[ch][0]
		+ myData->canReceiveDataCount[ch][1]);

	myData->save_msg[msg].val[idx].chData.totalCycle
		= myData->cData[ch].misc.totalCycle;
	myData->save_msg[msg].val[idx].chData.elementCycle
		= myData->cData[ch].elementCycle.cycle_count;
	for(i=0; i < MAX_ACC_CYCLE; i++) {
		myData->save_msg[msg].val[idx].chData.accCycle[i]
			= myData->cData[ch].accCycle[i].cycle_count;
	}
	for(i=0; i < MAX_MULTI_CYCLE; i++) {
		myData->save_msg[msg].val[idx].chData.multiCycle[i]
			= myData->cData[ch].multiCycle[i].cycle_count;
	}

	myData->save_msg[msg].val[idx].chData.avgV = myData->cData[ch].op.meanV;

	//kjg_101102
	myData->save_msg[msg].val[idx].chData.Vinput
		= myData->cData[ch].misc.Vinput;
	myData->save_msg[msg].val[idx].chData.Vpower
		= myData->cData[ch].misc.Vpower;
	myData->save_msg[msg].val[idx].chData.Vbus
		= myData->cData[ch].misc.Vbus;

	if(myData->ChAttribute[ch].chamber_control == 0) {
		myData->save_msg[msg].val[idx].chData.chamber_control = 0;
	} else {
		myData->save_msg[msg].val[idx].chData.chamber_control = 1;
	}

	myData->save_msg[msg].val[idx].chData.out_mux_use
		= myData->ChAttribute[ch].out_mux_use;
	myData->save_msg[msg].val[idx].chData.out_mux_backup
		= myData->cData[ch].misc.out_mux_backup;
	//myData->save_msg[msg].val[idx].chData.reserved1[0]
	//	= myData->cData[ch].misc.reserved1[0];
	//myData->save_msg[msg].val[idx].chData.reserved1[1]
	//	= myData->cData[ch].misc.reserved1[1];
	/*
	myData->save_msg[msg].val[idx].chData.reserved2[0]
		= myData->cData[ch].misc.reserved1[0];
	myData->save_msg[msg].val[idx].chData.reserved2[1]
		= myData->cData[ch].misc.reserved1[1];
	myData->save_msg[msg].val[idx].chData.reserved2[2]
		= myData->cData[ch].misc.reserved1[2];
	myData->save_msg[msg].val[idx].chData.reserved2[3]
		= myData->cData[ch].misc.reserved1[3];
	myData->save_msg[msg].val[idx].chData.reserved2[4]
		= myData->cData[ch].misc.reserved1[4];
	myData->save_msg[msg].val[idx].chData.reserved2[5]
		= myData->cData[ch].misc.reserved1[5];
	*/ //kjhw_181223e
	myData->save_msg[msg].val[idx].chData.maxAuxT //kjhw_181223s
		= myData->cData[ch].misc.maxAuxT;
	myData->save_msg[msg].val[idx].chData.minAuxT
		= myData->cData[ch].misc.minAuxT;
	myData->save_msg[msg].val[idx].chData.diffAuxT
		= myData->cData[ch].misc.diffAuxT;
	myData->save_msg[msg].val[idx].chData.avgAuxT
		= myData->cData[ch].misc.avgAuxT;
//20181219 KHK-------------------------	
	myData->save_msg[msg].val[idx].chData.SOC
		= (long)myData->cData[ch].op.SOC;
//-------------------------------------	
//	myData->save_msg[msg].val[idx].chData.reserved2[0]
//		= myData->cData[ch].misc.reserved1[0];
	/*myData->save_msg[msg].val[idx].chData.reserved2	//shhw_230605
		= (long)myData->cData[ch].misc.reserved1[0]; //kjhw_181223e*/

	if(myData->cData[ch].op.select == SAVE_FLAG_SAVING_END
		&& (myData->cData[ch].op.stepType == STEP_CHARGE
		|| myData->cData[ch].op.stepType == STEP_DISCHARGE
		|| myData->cData[ch].op.stepType == STEP_Z
		|| myData->cData[ch].op.stepType == STEP_OCV
		|| myData->cData[ch].op.stepType == STEP_REST)) {
		idxStepNo = myData->cData[ch].op.idxStepNo;
		item = myData->testCond[ch].grade[idxStepNo].item;
		if(item == GRADE_ITEM_V) {
			val = myData->save_msg[msg].val[idx].chData.Vsens;
			myData->save_msg[msg].val[idx].chData.grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else if(item == GRADE_ITEM_AMPARE_HOUR) {
			val = myData->save_msg[msg].val[idx].chData.charge_AmpareHour
				+ (myData->save_msg[msg].val[idx].chData.discharge_AmpareHour
				* (-1));
			myData->save_msg[msg].val[idx].chData.grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else if(item == GRADE_ITEM_Z) {
			val = myData->save_msg[msg].val[idx].chData.z;
			myData->save_msg[msg].val[idx].chData.grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else if(item == GRADE_ITEM_CAPACITANCE) {
			val = myData->save_msg[msg].val[idx].chData.capacitance;
			myData->save_msg[msg].val[idx].chData.grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else {
			myData->save_msg[msg].val[idx].chData.grade = 0;
		}
	}

	j = 0;
	for(i=0; i < MAX_AUX_DATA; i++) {
		if((ch+1) == myData->auxSetData[i].chNo) {
			myData->save_msg[msg].val[idx].auxData[j].auxChNo
				= myData->auxSetData[i].auxChNo;
			k = myData->auxSetData[i].auxChNo - 1;
			myData->save_msg[msg].val[idx].auxData[j].auxType
				= myData->auxSetData[i].auxType;
			switch(myData->auxSetData[i].auxType) {
				case 0: //temperature
					myData->save_msg[msg].val[idx].auxData[j].val
						= myData->COM.com_port[1].value[k];
					break;
				case 1: //sub sensing voltage
					k = myData->aux_ch_num[k].daq_ch; //jhkw_201117
					myData->save_msg[msg].val[idx].auxData[j].val
						= myData->SubSensV.ch[k].sensV;
					break;
				case 2: //sub sensing thermistor //kjh_160610
					k = myData->aux_ch_num[k].daq_ch; //jhkw_201117
					myData->save_msg[msg].val[idx].auxData[j].val
						= myData->SubSensV.ch[k].sensV;
					break;
				default:
					myData->save_msg[msg].val[idx].auxData[j].val = 0;
					break;
			}
			j++;
		}
	}
	//shhw_230614s
	myData->save_msg[msg].val[idx].chData.dMaxAuxV = myData->cData[ch].misc.dMaxAuxV;
	myData->save_msg[msg].val[idx].chData.dMinAuxV = myData->cData[ch].misc.dMinAuxV;
	myData->save_msg[msg].val[idx].chData.dMaxAuxVChNo = myData->cData[ch].misc.dMaxAuxVChNo;
	myData->save_msg[msg].val[idx].chData.dMinAuxVChNo = myData->cData[ch].misc.dMinAuxVChNo;
	myData->save_msg[msg].val[idx].chData.dIsens = myData->cData[ch].op.dIsens;
	//shhw_230614e

	j = myData->canReceiveDataCount[ch][0] + myData->canReceiveDataCount[ch][1];
	for(i=0; i < j; i++) {
		myData->save_msg[msg].val[idx].canData[i].canType
				= myData->canReceiveSetData.normalData[ch][i].canType;
		myData->save_msg[msg].val[idx].canData[i].data_type
				= myData->canReceiveSetData.normalData[ch][i].data_type;

		switch(myData->AppControl.config.systemModel) {
			case C_SDI_70V_50A_5A_4KW:
			case C_SDI_70V_50A_5A_4KW_2:
			case C_SDI_70V_50A_5A_7KW:
			case C_SDI_70V_50A_5A_7KW_2:
			case C_SDI_70V_250A_25A_18KW:
				k = 0;
				if(i < myData->canReceiveDataCount[ch][0]) {
					if((myData->cData[ch].misc.external_comm_state & 0x04)
						!= 0) k = 10;
				} else {
					if((myData->cData[ch].misc.external_comm_state & 0x08)
						!= 0) k = 10;
				}
				if(k == 0) {
					k = (int)myData->canReceiveSetData
						.normalData[ch][i].data_type;
				}
				break;
			default:
				k = (int)myData->canReceiveSetData.normalData[ch][i].data_type;
				break;
		}

		switch(k) {
			case 0: //unsigned
				myData->save_msg[msg].val[idx].canData[i].val.ul_val[0]
					= myData->CanData[ch][i].ul_val[0];
				break;
			case 1: //signed
				myData->save_msg[msg].val[idx].canData[i].val.l_val[0]
					= myData->CanData[ch][i].l_val[0];
				break;
			case 2: //float
				myData->save_msg[msg].val[idx].canData[i].val.f_val[0]
					= myData->CanData[ch][i].f_val[0];
				break;
			case 3: //string
				memcpy((char *)&myData->save_msg[msg].val[idx].canData[i].val,
					(char *)&myData->CanData[ch][i], 8);
				break;
			case 4: //hex //kjhw_140811
				myData->save_msg[msg].val[idx].canData[i].val.ul_val[0]
					= myData->CanData[ch][i].ul_val[0];
				break;
			default:
				memset((char *)&myData->save_msg[msg].val[idx].canData[i].val,
					0, 8);
				break;
		}
	}

	myData->save_msg[msg].write_idx = idx;
	myData->save_msg[msg].total_count++;
	myData->save_msg[msg].count[ch]++;
}

void send_save_msg_2_10ms(int ch, int msg)
{
#ifdef __10MS__
	unsigned char item, save_type;
	int idx, group, i, j, k, idxStepNo, rangeI;
	long val, val2, refI, tmp_refI, v_val, i_val;
	long max_refI, min_refI, rangeI_val, tmp_rangeI, min_refI2;
	double tmp_val;

	group = 0; //kjg_w

	//if(myData->save_msg_10ms[ch][msg].send_flag == 1) { //send save_msg stop
	//	return;
	//}

	if(myData->save_msg_10ms[ch][msg].total_count >= (MAX_SAVE_MSG - 50)) {
		if(myData->cData[ch].op.select != SAVE_FLAG_SAVING_END) {
			return;
		} else {
			if(myData->save_msg_10ms[ch][msg].total_count >= (MAX_SAVE_MSG - 5)) {
				return;
			}
		}
	}

	idx = myData->save_msg_10ms[ch][msg].write_idx;
	idx++;
	if(idx >= MAX_SAVE_MSG) idx = 0;

	save_type = 1; //kjhw_180325
	
	if(myData->cData[ch].signal[C_SIG_SEND_SAVE_MSG_DIVISION] == P1) { //kjg_170810
		if(idx == 0) i = MAX_SAVE_MSG - 1;
		else i = idx - 1;

		memcpy((char *)&myData->save_msg_10ms[ch][msg].val[idx],
			(char *)&myData->save_msg_10ms[ch][msg].val[i], sizeof(S_SAVE_MSG_VAL));

		myData->save_msg_10ms[ch][msg].val[idx].chData.resultIndex
			= myData->cData[ch].op.resultIndex;
		myData->save_msg_10ms[ch][msg].val[idx].chData.select
			= myData->cData[ch].op.select;
		myData->save_msg_10ms[ch][msg].val[idx].chData.code
			= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P1,
			(long)myData->cData[ch].op.code);

		myData->save_msg_10ms[ch][msg].write_idx = idx;
		myData->save_msg_10ms[ch][msg].total_count++;
		myData->save_msg_10ms[ch][msg].count[ch]++;
		return;
	}

	myData->save_msg_10ms[ch][msg].val[idx].chData.ch = (unsigned char)(ch + 1);
	myData->save_msg_10ms[ch][msg].val[idx].chData.resultIndex
		= myData->cData[ch].op.resultIndex;
	myData->save_msg_10ms[ch][msg].val[idx].chData.state
		= (unsigned char)convert_ch_state(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.state);
	myData->save_msg_10ms[ch][msg].val[idx].chData.stepType
		= (unsigned char)convert_step_type(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.stepType);
	myData->save_msg_10ms[ch][msg].val[idx].chData.stepMode
		= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.stepMode);
	
//kjhw_150616s
#ifdef __COA_VER_100B__
	if(myData->cData[ch].op.runTime <= 10) {
#else //COA_VER_100B2~
	if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
		myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
	 	//0, 10) > 0) {
		0, 15) > 0) { //kjhw_180325
#endif
		myData->save_msg_10ms[ch][msg].val[idx].chData.Vsens
			= myData->cData[ch].misc.tmpVsens;
//kjhw_150616e
	} else {
		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].misc.checkDelayTime_day,
			myData->cData[ch].misc.checkDelayTime,
			0, 5) > 0) { //kjhw_180325
			myData->save_msg_10ms[ch][msg].val[idx].chData.Vsens
				= myData->cData[ch].misc.tmpVsens;
		} else {
			myData->save_msg_10ms[ch][msg].val[idx].chData.Vsens
				= myData->cData[ch].op.Vsens;
		}
	}
	if(myData->cData[ch].op.stepType == STEP_END) { //kjhw_180417
		//myData->save_msg_10ms[ch][msg].val[idx].chData.Vsens = 0;
	}
	v_val = myData->save_msg_10ms[ch][msg].val[idx].chData.Vsens; //kjhw_180325

	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
//kjhw_150616s
#ifdef __COA_VER_100B__
		if(myData->cData[ch].op.runTime <= 10) {
#else //COA_VER_100B2~
		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
			//0, 10) > 0) {
			0, 15) > 0) { //kjhw_180325
#endif
			if(save_type == 1) { //kjhw_180325s
				k = 1;
				val = myData->cData[ch].misc.tmpIsens;
				for(i=0; i < 3; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].misc.tmpIsens;
						k++;
					}
				}
				val2 = 0;
				if(myData->cData[ch].op.stepType == STEP_CHARGE
					|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
					|| myData->cData[ch].op.stepType == STEP_PATTERN) {
					if((myData->cData[ch].misc.cv_select != P1)
						&& (myData->cData[ch].misc.cv_select != P2)) {
						refI = myData->cData[ch].misc.cmd_i[0] * k; //50A
						tmp_refI = refI * 0.05; //5% 2.5A
						rangeI = myData->cData[ch].op.rangeI;
						rangeI_val = myPs->config.maxI[rangeI] * k;
						tmp_rangeI = rangeI_val * 0.0005; //0.05% //
						if(refI >= 0) { //charge
							max_refI = refI + tmp_rangeI; //50.05
							min_refI = refI - tmp_rangeI; //49.95
						} else { //discharge
							max_refI = refI - tmp_rangeI; //-50.025
							min_refI = refI + tmp_rangeI; //-49.975
						}
						min_refI2 = refI - tmp_refI; //50-2.5 = 47.5 chg
						if(refI > 0) { //charge
							if((val >= min_refI2) 
								&& (val >= max_refI || val <= min_refI)) {
								tmp_val = (double)(refI / 1000) * 100; //mA
								tmp_val
									= (double)(val + (tmp_val * 1000)) / 101;
								val = (long)tmp_val;
							}
						} else if(refI < 0) { //discharge
							if((val <= min_refI2) 
								&& (val <= max_refI || val >= min_refI)) {
								tmp_val = (double)(refI / 1000) * 100; //mA
								tmp_val
									= (double)(val + (tmp_val * 1000)) / 101;
								val = (long)tmp_val;
							}
						}
					}
				}
			} else {
				val = myData->cData[ch].misc.tmpIsens;
				for(i=0; i < 3; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].misc.tmpIsens;
					}
				}
			} //kjhw_180325e
//kjhw_150616e
		} else {
			if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
				myData->cData[ch].misc.checkDelayTime_day,
				myData->cData[ch].misc.checkDelayTime,
				0, 5) > 0) { //kjhw_180325
				if(save_type == 1) { //kjhw_180325s
					k = 1;
					val = myData->cData[ch].misc.tmpIsens;
					for(i=0; i < 3; i++) {
						j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
						if(j >= 0) {
							val += myData->cData[j].misc.tmpIsens;
							k++;
						}
					}
					val2 = 0;
					if(myData->cData[ch].op.stepType == STEP_CHARGE
						|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
						|| myData->cData[ch].op.stepType == STEP_PATTERN) {
						if((myData->cData[ch].misc.cv_select != P1)
							&& (myData->cData[ch].misc.cv_select != P2)) {
							refI = myData->cData[ch].misc.cmd_i[0] * k; //50A
							tmp_refI = refI * 0.05; //5% 2.5A
							rangeI = myData->cData[ch].op.rangeI;
							rangeI_val = myPs->config.maxI[rangeI] * k;
							tmp_rangeI = rangeI_val * 0.0005; //0.05% //
							if(refI >= 0) { //charge
								max_refI = refI + tmp_rangeI; //50.05
								min_refI = refI - tmp_rangeI; //49.95
							} else { //discharge
								max_refI = refI - tmp_rangeI; //-50.025
								min_refI = refI + tmp_rangeI; //-49.975
							}
							min_refI2 = refI - tmp_refI; //50-2.5 = 47.5 chg
							if(refI > 0) { //charge
								if((val >= min_refI2) 
									&& (val >= max_refI || val <= min_refI)) {
									tmp_val = (double)(refI / 1000) * 100; //mA
									tmp_val
										= (double)(val + (tmp_val * 1000))
										/ 101;
									val = (long)tmp_val;
								}
							} else if(refI < 0) { //discharge
								if((val <= min_refI2) 
									&& (val <= max_refI || val >= min_refI)) {
									tmp_val = (double)(refI / 1000) * 100; //mA
									tmp_val
										= (double)(val + (tmp_val * 1000))
										/ 101;
									val = (long)tmp_val;
								}
							}
						}
					}
				} else {
					val = myData->cData[ch].misc.tmpIsens;
					for(i=0; i < 3; i++) {
						j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
						if(j >= 0) {
							val += myData->cData[j].misc.tmpIsens;
						}
					}
				} //kjhw_180325e
			} else {
				val = myData->cData[ch].op.Isens;
				for(i=0; i < 3; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].op.Isens;
					}
				}
			}
		}
		myData->save_msg_10ms[ch][msg].val[idx].chData.Isens = val;
		if(myData->cData[ch].op.stepType == STEP_END) { //kjhw_180417
			//myData->save_msg_10ms[ch][msg].val[idx].chData.Isens = 0;
		}
		i_val = myData->save_msg_10ms[ch][msg].val[idx].chData.Isens; //kjhw_180325

		val = myData->cData[ch].op.charge_AmpareHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_AmpareHour;
			}
		}
		myData->save_msg_10ms[ch][msg].val[idx].chData.charge_AmpareHour = val;

		val = myData->cData[ch].op.discharge_AmpareHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_AmpareHour;
			}
		}
		myData->save_msg_10ms[ch][msg].val[idx].chData.discharge_AmpareHour = val;

#ifdef __COA_VER_100B__
		if(myData->cData[ch].op.runTime <= 10) {
#else //COA_VER_100B2~
		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
		 	//0, 10) > 0) {
			0, 15) > 0) { //kjhw_180325
#endif
			if(save_type == 1) { //kjhw_180325s
				val = myData->cData[ch].misc.tmpWatt;
				for(i=0; i < 3; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].misc.tmpWatt;
					}
				}
				if(myData->cData[ch].op.stepType == STEP_CHARGE
					|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
					|| myData->cData[ch].op.stepType == STEP_PATTERN) {
					if((myData->cData[ch].misc.cv_select != P1)
						&& (myData->cData[ch].misc.cv_select != P2)) {
						tmp_val = ((double)v_val / 1000.0)
							* ((double)i_val / 1000.0);
						val = (long)(tmp_val / 1000.0);
					}
				}
			} else {
				val = myData->cData[ch].misc.tmpWatt;
				for(i=0; i < 3; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].misc.tmpWatt;
					}
				}
			} //kjhw_180417e
		} else {
			if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
				myData->cData[ch].misc.checkDelayTime_day,
				myData->cData[ch].misc.checkDelayTime,
				0, 5) > 0) { //kjhw_180325
				if(save_type == 1) { //kjhw_180325s
					val = myData->cData[ch].misc.tmpWatt;
					for(i=0; i < 3; i++) {
						j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
						if(j >= 0) {
							val += myData->cData[j].misc.tmpWatt;
						}
					}
					if(myData->cData[ch].op.stepType == STEP_CHARGE
						|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
						|| myData->cData[ch].op.stepType == STEP_PATTERN) {
						if((myData->cData[ch].misc.cv_select != P1)
							&& (myData->cData[ch].misc.cv_select != P2)) {
							tmp_val = ((double)v_val / 1000.0)
								* ((double)i_val / 1000.0);
							val	= (long)(tmp_val / 1000.0);
						}
					}
				} else {
					val = myData->cData[ch].misc.tmpWatt;
					for(i=0; i < 3; i++) {
						j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
						if(j >= 0) {
							val += myData->cData[j].misc.tmpWatt;
						}
					}
				} //kjhw_180325e
			} else {
				val = myData->cData[ch].op.watt;
				for(i=0; i < 3; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].op.watt;
					}
				}
			}
		}
		myData->save_msg_10ms[ch][msg].val[idx].chData.watt = val;

		val = myData->cData[ch].op.charge_WattHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_WattHour;
			}
		}
		myData->save_msg_10ms[ch][msg].val[idx].chData.charge_WattHour = val;

		val = myData->cData[ch].op.discharge_WattHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_WattHour;
			}
		}
		myData->save_msg_10ms[ch][msg].val[idx].chData.discharge_WattHour = val;

		val = myData->cData[ch].op.meanI;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.meanI;
			}
		}
		myData->save_msg_10ms[ch][msg].val[idx].chData.avgI = val;
	} else {
//kjhw_150616s
#ifdef __COA_VER_100B__
		if(myData->cData[ch].op.runTime <= 10) {
#else //COA_VER_100B2~
		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
		 	//0, 10) > 0) {
			0, 15) > 0) { //kjhw_180325
#endif
			if(save_type == 1) { //kjhw_180325s
				val = myData->cData[ch].misc.tmpIsens;
				if(myData->cData[ch].op.stepType == STEP_CHARGE
					|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
					|| myData->cData[ch].op.stepType == STEP_PATTERN) {
					if((myData->cData[ch].misc.cv_select != P1)
						&& (myData->cData[ch].misc.cv_select != P2)) {
						refI = myData->cData[ch].misc.cmd_i[0]; //50A
						tmp_refI = refI * 0.05; //5% 2.5A
						rangeI = myData->cData[ch].op.rangeI;
						rangeI_val = myPs->config.maxI[rangeI];
						tmp_rangeI = rangeI_val * 0.0005; //0.05% //
						if(refI >= 0) { //charge
							max_refI = refI + tmp_rangeI; //50.05
							min_refI = refI - tmp_rangeI; //49.95
						} else { //discharge
							max_refI = refI - tmp_rangeI; //-50.025
							min_refI = refI + tmp_rangeI; //-49.975
						}
						min_refI2 = refI - tmp_refI; //50-2.5 = 47.5 chg
						if(refI > 0) { //charge
							if((val >= min_refI2) 
								&& (val >= max_refI || val <= min_refI)) {
								tmp_val = (double)(refI / 1000) * 100; //mA
								tmp_val
									= (double)(val + (tmp_val * 1000)) / 101;
								val = (long)tmp_val;
							}
						} else if(refI < 0) { //discharge
							if((val <= min_refI2) 
								&& (val <= max_refI || val >= min_refI)) {
								tmp_val = (double)(refI / 1000) * 100; //mA
								tmp_val
									= (double)(val + (tmp_val * 1000)) / 101;
								val = (long)tmp_val;
							}
						}
					}
				}
			} else {
				val = myData->cData[ch].misc.tmpIsens;
			} //kjhw_180325e
//kjhw_150616e
		} else {
			if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
				myData->cData[ch].misc.checkDelayTime_day,
				myData->cData[ch].misc.checkDelayTime,
				0, 5) > 0) { //kjhw_180325
				if(save_type == 1) { //kjhw_180325s
					val = myData->cData[ch].misc.tmpIsens;
					if(myData->cData[ch].op.stepType == STEP_CHARGE
						|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
						|| myData->cData[ch].op.stepType == STEP_PATTERN) {
						if((myData->cData[ch].misc.cv_select != P1)
							&& (myData->cData[ch].misc.cv_select != P2)) {
							refI = myData->cData[ch].misc.cmd_i[0]; //50A
							tmp_refI = refI * 0.05; //5% 2.5A
							rangeI = myData->cData[ch].op.rangeI;
							rangeI_val = myPs->config.maxI[rangeI];
							tmp_rangeI = rangeI_val * 0.0005; //0.05% //
							if(refI >= 0) { //charge
								max_refI = refI + tmp_rangeI; //50.05
								min_refI = refI - tmp_rangeI; //49.95
							} else { //discharge
								max_refI = refI - tmp_rangeI; //-50.025
								min_refI = refI + tmp_rangeI; //-49.975
							}
							min_refI2 = refI - tmp_refI; //50-2.5 = 47.5 chg
							if(refI > 0) { //charge
								if((val >= min_refI2) 
									&& (val >= max_refI || val <= min_refI)) {
									tmp_val = (double)(refI / 1000) * 100; //mA
									tmp_val = (double)(val + (tmp_val * 1000))
											/ 101;
									val = (long)tmp_val;
								}
							} else if(refI < 0) { //discharge
								if((val <= min_refI2) 
									&& (val <= max_refI || val >= min_refI)) {
									tmp_val = (double)(refI / 1000) * 100; //mA
									tmp_val = (double)(val + (tmp_val * 1000))
											/ 101;
									val = (long)tmp_val;
								}
							}
						}
					}
				} else {
					val = myData->cData[ch].misc.tmpIsens;
				} //kjhw_180325e
			} else {
				val	= myData->cData[ch].op.Isens;
			}
		}
		myData->save_msg_10ms[ch][msg].val[idx].chData.Isens = val;
		if(myData->cData[ch].op.stepType == STEP_END) { //kjhw_180417
			//myData->save_msg_10ms[ch][msg].val[idx].chData.Isens = 0;
		}
		i_val = myData->save_msg_10ms[ch][msg].val[idx].chData.Isens; //kjhw_180325

		myData->save_msg_10ms[ch][msg].val[idx].chData.charge_AmpareHour
			= myData->cData[ch].op.charge_AmpareHour;
		myData->save_msg_10ms[ch][msg].val[idx].chData.discharge_AmpareHour
			= myData->cData[ch].op.discharge_AmpareHour;
#ifdef __COA_VER_100B__
		if(myData->cData[ch].op.runTime <= 10) {
#else //COA_VER_100B2~
		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
		 	//0, 10) > 0) {
			0, 15) > 0) { //kjhw_180325
#endif
			if(save_type == 1) { //kjhw_180325s
				val = myData->cData[ch].misc.tmpWatt;
				if(myData->cData[ch].op.stepType == STEP_CHARGE
					|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
					|| myData->cData[ch].op.stepType == STEP_PATTERN) {
					if((myData->cData[ch].misc.cv_select != P1)
						&& (myData->cData[ch].misc.cv_select != P2)) {
						tmp_val = ((double)v_val / 1000.0)
							* ((double)i_val / 1000.0);
						val	= (long)(tmp_val / 1000.0);
					}
				}
			} else {
				val = myData->cData[ch].misc.tmpWatt;
			} //kjhw_180325e
		} else {
			if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
				myData->cData[ch].misc.checkDelayTime_day,
				myData->cData[ch].misc.checkDelayTime,
				0, 5) > 0) { //kjhw_180325
				if(save_type == 1) { //kjhw_180325s
					val = myData->cData[ch].misc.tmpWatt;
					if(myData->cData[ch].op.stepType == STEP_CHARGE
						|| myData->cData[ch].op.stepType == STEP_DISCHARGE 
						|| myData->cData[ch].op.stepType == STEP_PATTERN) {
						if((myData->cData[ch].misc.cv_select != P1)
							&& (myData->cData[ch].misc.cv_select != P2)) {
							tmp_val = ((double)v_val / 1000.0)
								* ((double)i_val / 1000.0);
							val = (long)(tmp_val / 1000.0);
						}
					}
				} else {
					val = myData->cData[ch].misc.tmpWatt;
				} //kjhw_180325e
			} else {
				val = myData->cData[ch].op.watt;
			}
		}
		myData->save_msg_10ms[ch][msg].val[idx].chData.watt = val; //kjhw_180325
		myData->save_msg_10ms[ch][msg].val[idx].chData.charge_WattHour
			= myData->cData[ch].op.charge_WattHour;
		myData->save_msg_10ms[ch][msg].val[idx].chData.discharge_WattHour
			= myData->cData[ch].op.discharge_WattHour;
		myData->save_msg_10ms[ch][msg].val[idx].chData.avgI = myData->cData[ch].op.meanI;
	}
	//kjhw_130429s
	switch(myData->AppControl.config.systemModel) {
		//case C_SDI_100V_150A_70A_30KW:
		//case C_SDI_100V_150A_70A_30KW_2:
		case C_HLGP_500V_200A_100A_200KW_2:
		case C_HLGP_500V_200A_100A_200KW_3:
		case C_HLGP_500V_200A_100A_200KW_4:
		case C_HLGP_500V_200A_100A_200KW_5:
		case C_HLGP_500V_300A_100A_300KW:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_2:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_3:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_4:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_5:	//jhk_140305
			if(myData->cData[ch].op.stepType == STEP_PATTERN) {
				if(((myData->cData[ch].op.stepMode == MODE_CC)
					&& (myData->cData[ch].misc.cmd_i[0] == 0))
					|| ((myData->cData[ch].op.stepMode == MODE_CP)
					&& (myData->cData[ch].misc.cmd_p[0] == 0))) {
					myData->save_msg_10ms[ch][msg].val[idx].chData.watt = 0;
				}
			}
			break;
		default: break;
	}
	//kjhw_130429e
	myData->save_msg_10ms[ch][msg].val[idx].chData.capacitance
		= myData->cData[ch].op.capacitance;
	myData->save_msg_10ms[ch][msg].val[idx].chData.z = myData->cData[ch].op.z;

	myData->save_msg_10ms[ch][msg].val[idx].chData.select
		= myData->cData[ch].op.select;
	myData->save_msg_10ms[ch][msg].val[idx].chData.code
		= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.code);
	myData->save_msg_10ms[ch][msg].val[idx].chData.stepNo
		= myData->cData[ch].op.idxStepNo + 1;

#ifdef __COA_VER_100B__
	myData->save_msg_10ms[ch][msg].val[idx].chData.runTime
		= myData->cData[ch].op.runTime;
	myData->save_msg_10ms[ch][msg].val[idx].chData.totalRunTime
		= myData->cData[ch].op.totalRunTime;
	myData->save_msg_10ms[ch][msg].val[idx].chData.cvTime
		= myData->cData[ch].op.cvTime;
	//myData->save_msg_10ms[ch][msg].val[idx].chData.ccTime
	//	= myData->cData[ch].op.ccTime; //kjg_w
#else //COA_VER_100B2~
	if(myData->cData[ch].op.stepType == STEP_END) {
		myData->cData[ch].op.runTime_day = 0;
	} //kjhw_180417
	myData->save_msg_10ms[ch][msg].val[idx].chData.runTime_day
		= myData->cData[ch].op.runTime_day;
	myData->save_msg_10ms[ch][msg].val[idx].chData.totalRunTime_day
		= myData->cData[ch].op.totalRunTime_day;
	myData->save_msg_10ms[ch][msg].val[idx].chData.cvTime_day
		= myData->cData[ch].op.cvTime_day;
	//myData->save_msg_10ms[ch][msg].val[idx].chData.ccTime_day
	//	= myData->cData[ch].op.ccTime_day; //kjg_w
	//
	if(myData->cData[ch].op.stepType == STEP_END) {
		myData->cData[ch].op.runTime = 0;
	} //kjhw_180417
	myData->save_msg_10ms[ch][msg].val[idx].chData.runTime
		= myData->cData[ch].op.runTime;
	myData->save_msg_10ms[ch][msg].val[idx].chData.totalRunTime
		= myData->cData[ch].op.totalRunTime;
	myData->save_msg_10ms[ch][msg].val[idx].chData.cvTime
		= myData->cData[ch].op.cvTime;
	//myData->save_msg_10ms[ch][msg].val[idx].chData.ccTime
	//	= myData->cData[ch].op.ccTime; //kjg_w
#endif

	myData->save_msg_10ms[ch][msg].val[idx].chData.realDate
		= myData->mData.real_time[6] * 10000 //year
		+ myData->mData.real_time[5] * 100 //month
		+ myData->mData.real_time[4]; //day
	myData->save_msg_10ms[ch][msg].val[idx].chData.realClock
		= myData->mData.real_time[3] * 10000000 //hour
		+ myData->mData.real_time[2] * 100000 //min
		+ myData->mData.real_time[1] * 1000 //sec
		+ myData->mData.real_time[0]; //msec

	myData->save_msg_10ms[ch][msg].val[idx].chData.reservedCmd
		= myData->cData[ch].op.reservedCmd;
	myData->save_msg_10ms[ch][msg].val[idx].chData.external_comm_state
		= myData->cData[ch].misc.external_comm_state;
	myData->save_msg_10ms[ch][msg].val[idx].chData.ch_output_state
		= myData->cData[ch].misc.ch_output_state; //kjg_101219
	myData->save_msg_10ms[ch][msg].val[idx].chData.ch_input_state
		= myData->cData[ch].misc.ch_input_state; //kjg_101219

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
	myData->save_msg_10ms[ch][msg].val[idx].chData.auxDataCount
		= (short int)(myData->auxDataCount[ch][0]
		+ myData->auxDataCount[ch][1]);
#else //COA_VER_100F~
	//kjh_160610
	myData->save_msg_10ms[ch][msg].val[idx].chData.auxDataCount
		= (short int)(myData->auxDataCount[ch][0]
		+ myData->auxDataCount[ch][1] + myData->auxDataCount[ch][2]);
#endif
	myData->save_msg_10ms[ch][msg].val[idx].chData.canReceiveDataCount
		= (short int)(myData->canReceiveDataCount[ch][0]
		+ myData->canReceiveDataCount[ch][1]);

	myData->save_msg_10ms[ch][msg].val[idx].chData.totalCycle
		= myData->cData[ch].misc.totalCycle;
	myData->save_msg_10ms[ch][msg].val[idx].chData.elementCycle
		= myData->cData[ch].elementCycle.cycle_count;
	for(i=0; i < MAX_ACC_CYCLE; i++) {
		myData->save_msg_10ms[ch][msg].val[idx].chData.accCycle[i]
			= myData->cData[ch].accCycle[i].cycle_count;
	}
	for(i=0; i < MAX_MULTI_CYCLE; i++) {
		myData->save_msg_10ms[ch][msg].val[idx].chData.multiCycle[i]
			= myData->cData[ch].multiCycle[i].cycle_count;
	}

	myData->save_msg_10ms[ch][msg].val[idx].chData.avgV = myData->cData[ch].op.meanV;

	//kjg_101102
	myData->save_msg_10ms[ch][msg].val[idx].chData.Vinput
		= myData->cData[ch].misc.Vinput;
	myData->save_msg_10ms[ch][msg].val[idx].chData.Vpower
		= myData->cData[ch].misc.Vpower;
	myData->save_msg_10ms[ch][msg].val[idx].chData.Vbus
		= myData->cData[ch].misc.Vbus;

	if(myData->ChAttribute[ch].chamber_control == 0) {
		myData->save_msg_10ms[ch][msg].val[idx].chData.chamber_control = 0;
	} else {
		myData->save_msg_10ms[ch][msg].val[idx].chData.chamber_control = 1;
	}

	myData->save_msg_10ms[ch][msg].val[idx].chData.out_mux_use
		= myData->ChAttribute[ch].out_mux_use;
	myData->save_msg_10ms[ch][msg].val[idx].chData.out_mux_backup
		= myData->cData[ch].misc.out_mux_backup;
	//myData->save_msg_10ms[ch][msg].val[idx].chData.reserved1[0]
	//	= myData->cData[ch].misc.reserved1[0];
	//myData->save_msg_10ms[ch][msg].val[idx].chData.reserved1[1]
	//	= myData->cData[ch].misc.reserved1[1];
	/*
	myData->save_msg_10ms[ch][msg].val[idx].chData.reserved2[0]
		= myData->cData[ch].misc.reserved1[0];
	myData->save_msg_10ms[ch][msg].val[idx].chData.reserved2[1]
		= myData->cData[ch].misc.reserved1[1];
	myData->save_msg_10ms[ch][msg].val[idx].chData.reserved2[2]
		= myData->cData[ch].misc.reserved1[2];
	myData->save_msg_10ms[ch][msg].val[idx].chData.reserved2[3]
		= myData->cData[ch].misc.reserved1[3];
	myData->save_msg_10ms[ch][msg].val[idx].chData.reserved2[4]
		= myData->cData[ch].misc.reserved1[4];
	myData->save_msg_10ms[ch][msg].val[idx].chData.reserved2[5]
		= myData->cData[ch].misc.reserved1[5];
	*/ //kjhw_181223e
	myData->save_msg_10ms[ch][msg].val[idx].chData.maxAuxT //kjhw_181223s
		= myData->cData[ch].misc.maxAuxT;
	myData->save_msg_10ms[ch][msg].val[idx].chData.minAuxT
		= myData->cData[ch].misc.minAuxT;
	myData->save_msg_10ms[ch][msg].val[idx].chData.diffAuxT
		= myData->cData[ch].misc.diffAuxT;
	myData->save_msg_10ms[ch][msg].val[idx].chData.avgAuxT
		= myData->cData[ch].misc.avgAuxT;
//20181219 KHK-------------------------	
	myData->save_msg_10ms[ch][msg].val[idx].chData.SOC
		= (long)myData->cData[ch].op.SOC;
//-------------------------------------	
//	myData->save_msg_10ms[ch][msg].val[idx].chData.reserved2[0]
//		= myData->cData[ch].misc.reserved1[0];
	myData->save_msg_10ms[ch][msg].val[idx].chData.reserved2
		= (long)myData->cData[ch].misc.reserved1[0]; //kjhw_181223e

	if(myData->cData[ch].op.select == SAVE_FLAG_SAVING_END
		&& (myData->cData[ch].op.stepType == STEP_CHARGE
		|| myData->cData[ch].op.stepType == STEP_DISCHARGE
		|| myData->cData[ch].op.stepType == STEP_Z
		|| myData->cData[ch].op.stepType == STEP_OCV
		|| myData->cData[ch].op.stepType == STEP_REST)) {
		idxStepNo = myData->cData[ch].op.idxStepNo;
		item = myData->testCond[ch].grade[idxStepNo].item;
		if(item == GRADE_ITEM_V) {
			val = myData->save_msg_10ms[ch][msg].val[idx].chData.Vsens;
			myData->save_msg_10ms[ch][msg].val[idx].chData.grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else if(item == GRADE_ITEM_AMPARE_HOUR) {
			val = myData->save_msg_10ms[ch][msg].val[idx].chData.charge_AmpareHour
				+ (myData->save_msg_10ms[ch][msg].val[idx].chData.discharge_AmpareHour
				* (-1));
			myData->save_msg_10ms[ch][msg].val[idx].chData.grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else if(item == GRADE_ITEM_Z) {
			val = myData->save_msg_10ms[ch][msg].val[idx].chData.z;
			myData->save_msg_10ms[ch][msg].val[idx].chData.grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else if(item == GRADE_ITEM_CAPACITANCE) {
			val = myData->save_msg_10ms[ch][msg].val[idx].chData.capacitance;
			myData->save_msg_10ms[ch][msg].val[idx].chData.grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else {
			myData->save_msg_10ms[ch][msg].val[idx].chData.grade = 0;
		}
	}

	j = 0;
	for(i=0; i < MAX_AUX_DATA; i++) {
		if((ch+1) == myData->auxSetData[i].chNo) {
			myData->save_msg_10ms[ch][msg].val[idx].auxData[j].auxChNo
				= myData->auxSetData[i].auxChNo;
			k = myData->auxSetData[i].auxChNo - 1;
			myData->save_msg_10ms[ch][msg].val[idx].auxData[j].auxType
				= myData->auxSetData[i].auxType;
			switch(myData->auxSetData[i].auxType) {
				case 0: //temperature
					myData->save_msg_10ms[ch][msg].val[idx].auxData[j].val
						= myData->COM.com_port[1].value[k];
					break;
				case 1: //sub sensing voltage
				case 2: //sub sensing thermistor //kjh_160610
					switch(myData->AppControl.config.systemModel) {
						case C_LGC_40V_300A_100A_48KW: //jhk_181018
						case C_LGC_40V_300A_100A_48KW_2: //jhk_181018
						case C_LGC_40V_500A_250A_100A_80KW:	//jhk_181014
						case C_LGC_40V_500A_250A_100A_80KW_2: //jhk_181014
							if((k >= 72) && (k < 96)) k += 24;
							else if((k >= 96) && (k < 120)) k -= 24;
							break;
						case C_LGC_50V_40A_10A_4A: //csk_120225
						case C_LGC_50V_40A_10A_4A_2:
						case C_LGC_50V_40A_10A_4A_3:
						case C_LGC_50V_40A_10A_4A_4:
							if((k >= 10) && (k < 20)) k += 22;
							else if((k >= 20) && (k < 30)) k += 44;
							else if((k >= 30) && (k < 40)) k += 66;
							else if((k >= 40) && (k < 62)) k -= 30;
							else if((k >= 62) && (k < 84)) k -= 20;
							else if((k >= 84) && (k < 106)) k -= 10;
							break;
						case C_SDI_70V_50A_5A_4KW:
						case C_SDI_70V_50A_5A_4KW_2:
						case C_SDI_70V_50A_5A_7KW:
						case C_SDI_70V_50A_5A_7KW_2:
						case C_SDI_70V_250A_25A_18KW:
							if((k >= 30) && (k <= 39)) k += 2;
							else if((k >= 40) && (k <= 69)) k += 24;
							else if((k >= 70) && (k <= 79)) k += 26;
							else if((k >= 80) && (k <= 81)) k -= 50;
							else if((k >= 82) && (k <= 103)) k -= 40;
							else if((k >= 104) && (k <= 105)) k -= 10;
							break;
						case C_SNU_60V_200A_100A_50A_1CH_12KW:  //jhkw_130221
						    if((k >= 8) && (k < 14)) k += 24;
						    else if((k >= 32) && (k < 38)) k -= 24;
						    break;
						//case C_LGC_60V_250A_10A_30KW: //110422_csk
						//case C_LGC_60V_250A_10A_30KW_2:
						//case C_LGC_60V_250A_10A_30KW_3:
						//	if((k >= 8) && (k < 16)) k += 24;
						//	else if((k >= 32) && (k < 40)) k -= 24;
						//	break;
						case C_LGC_60V_250A_10A_30KW:	//jhk_120601
						case C_LGC_60V_250A_10A_30KW_2:	//jhk_120601
						case C_LGC_60V_250A_10A_30KW_3:	//jhk_120601
						case C_HYUNDAI_60V_300A_100A_36KW:	//jhk_160825
						case C_HYUNDAI_60V_300A_100A_36KW_2:	//jhk_161111
							if((k >= 15) && (k < 30)) k += 17;
							else if((k >= 32) && (k < 47)) k -= 17;
							break;
						case C_DAEHWA_60V_500A_250A_50A_60KW:	//jhk_160913
						case C_DAEHWA_60V_500A_250A_50A_60KW_2:	//jhk_160913
						    if((k >= 8) && (k < 16)) k += 24;
						    else if((k >= 32) && (k < 40)) k -= 24;
						    break;
						//case C_LGC_60V_300A_30A_36KW:	//jhk_150406
						case C_LGC_60V_300A_30A_36KW_2:	//jhk_150406
						case C_LGC_60V_300A_30A_36KW_3:	//jhk_150406
						case C_LGC_70V_250A_10A_35KW:
						case C_LGC_70V_250A_10A_35KW_2:
						case C_LGC_70V_250A_10A_35KW_3:
						case C_LGC_70V_250A_10A_35KW_4:
						case C_LGC_70V_250A_10A_35KW_5:
						case C_LGC_70V_250A_10A_35KW_6:
						case C_LGC_70V_250A_10A_35KW_7:
						case C_LGC_70V_250A_10A_35KW_8:
						//case C_LGC_70V_250A_10A_35KW_9:	//jhkw_131204
						//case C_LGC_70V_250A_10A_35KW_10:	//jhkw_131204
						//case C_LGC_70V_250A_10A_35KW_11:	//jhkw_131204
						//case C_LGC_70V_250A_10A_35KW_12:	//jhkw_131204
						//case C_LGC_70V_250A_10A_35KW_13:	//jhkw_131204
						case C_LGC_70V_250A_10A_35KW_14:
						case C_LGC_70V_250A_10A_35KW_15:
						case C_LGC_70V_250A_10A_35KW_16: //jhkw_130812
						case C_LGC_70V_250A_10A_35KW_17: //jhkw_130812
						case C_LGC_70V_250A_10A_35KW_18: //jhkw_130812
						case C_LGC_70V_250A_10A_35KW_23: //jhk_120702
						case C_LGC_70V_250A_10A_35KW_24: //jhk_120702
						case C_LGC_70V_250A_10A_35KW_25: //jhk_120702
						case C_LGC_70V_250A_10A_35KW_26: //jhk_120702
						case C_LGC_70V_250A_10A_35KW_27: //jhk_120702
						//case C_LGC_70V_250A_10A_35KW_28: //jhk_120702
						case C_LGC_70V_250A_50A_35KW:		//jhk_150502
						case C_LGC_70V_250A_50A_35KW_2:		//jhk_150502
						case C_LGC_70V_250A_50A_35KW_3:		//jhk_150502
						case C_LGC_70V_250A_50A_35KW_4:		//jhk_150502
						case C_LGC_70V_250A_50A_35KW_5:		//jhk_150502
						case C_LGC_70V_250A_50A_35KW_6:		//jhk_150502
						case C_LGC_70V_250A_50A_35KW_7:		//jhk_150502
						case C_LGC_70V_250A_50A_35KW_8:		//jhk_150502
						//case C_LGC_70V_250A_100A_50A_35KW:      //jhkw_121019
						//case C_LGC_70V_250A_100A_50A_35KW_2:    //jhkw_121019
						case C_LGC_70V_250A_100A_50A_35KW_3:    //jhkw_121019
						case C_LGC_70V_250A_100A_50A_35KW_4:    //jhkw_121019
							if((k >= 12) && (k < 24)) k += 20;
							else if((k >= 32) && (k < 44)) k -= 20;
							break;
						case C_LGC_60V_300A_30A_36KW:	//kjh_171110
						case C_LGC_70V_250A_10A_35KW_28:	//kjh_170927
						case C_LGC_70V_250A_100A_50A_35KW:      //kjh_170927
						case C_LGC_70V_250A_100A_50A_35KW_2:    //kjh_170927
							if((k >= 12) && (k < 24)) k += 20;
							else if((k >= 24) && (k < 32)) k += 40;
							else if((k >= 32) && (k < 44)) k -= 20;
							else if((k >= 64) && (k < 72)) k -= 40;
							break;
						//case C_LGC_70V_250A_10A_35KW_16: //110421_csk
						//case C_LGC_70V_250A_10A_35KW_17:
						//case C_LGC_70V_250A_10A_35KW_18:
						case C_LGC_70V_250A_10A_35KW_19:    //111202_csk
						case C_LGC_70V_250A_10A_35KW_20:    //111202_csk
						case C_LGC_70V_250A_10A_35KW_21:    //jhk_120229
						case C_LGC_70V_250A_10A_35KW_22:    //jhk_120229
							if((k >= 10) && (k < 20)) k += 22;
							else if((k >= 32) && (k < 42)) k -= 22;
							break;
						case C_LGC_70V_250A_50A_35KW_9:		//jhk_160515
						case C_LGC_70V_250A_50A_35KW_10:	//jhk_160515
						case C_LGC_70V_250A_50A_35KW_11:	//jhk_160515
						case C_LGC_70V_250A_50A_35KW_12:	//jhk_160515
						case C_LGC_70V_250A_50A_35KW_13:	//jhk_160515
						case C_LGC_70V_250A_50A_35KW_14:	//jhk_160515
						case C_LGC_70V_250A_50A_35KW_15:	//jhk_160515
						case C_LGC_70V_250A_50A_35KW_16:	//jhk_160515
							if((k >= 28) && (k < 34)) k += 4;
							else if((k >= 32) && (k < 38)) k -= 4;
							break;
						case C_LGC_70V_300A_100A_42KW:	//jhk_170211
							if((k >= 60) && (k < 66)) k += 4;
							else if((k >= 64) && (k < 70)) k -= 4;
							break;
						case C_LGC_70V_300A_100A_50A_10A_42KW:	//jhk_120810
						case C_LGC_70V_300A_100A_50A_10A_42KW_2: //jhkw_120813
							if((k >= 15) && (k < 30)) k += 17;
							else if((k >= 32) && (k < 47)) k -= 17;
							break;
						case C_LGCUSA_70V_300A_42KW:	//jhk_160802
						case C_LGCUSA_70V_300A_42KW_2:	//jhk_160802
						case C_LGCUSA_70V_350A_24KW:	//jhk_160720
						case C_LGCUSA_70V_350A_24KW_2:	//jhk_160720
						case C_LGCUSA_70V_350A_24KW_3:	//jhk_160720
						case C_LGCUSA_70V_350A_24KW_4:	//jhk_160720
							if((k >= 16) && (k < 20)) k += 16;
							else if((k >= 32) && (k < 36)) k -= 16;
							break;
						case C_LGCCHINA_80V_150A_12KW:		//jhk_160725
						case C_LGCCHINA_80V_150A_12KW_2:	//jhk_160725
							if((k >= 20) && (k < 24)) k += 12;
							else if((k >= 32) && (k < 36)) k -= 12;
							break;
						case C_LGC_90V_300A_100A_108KW:	//jhk_181016
						case C_LGC_90V_300A_100A_108KW_2:	//jhk_181016
						case C_LGC_90V_300A_100A_108KW_3:	//jhk_181016
						case C_LGC_90V_300A_100A_108KW_4:	//jhk_181016
						case C_LGC_90V_300A_100A_108KW_5:	//jhk_181016
						case C_LGC_90V_300A_100A_108KW_6:	//jhk_181016
						case C_LGC_90V_300A_100A_108KW_7:	//jhk_181016
						case C_LGC_90V_300A_100A_108KW_8:	//jhk_181016
						case C_LGC_90V_500A_250A_100A_180KW:	//jhk_181006
						case C_LGC_90V_500A_250A_100A_180KW_2:	//jhk_181006
						case C_LGC_90V_500A_250A_100A_180KW_3:	//jhk_181006
						case C_LGC_90V_500A_250A_100A_180KW_4:	//jhk_181006
						case C_LGC_90V_500A_250A_100A_180KW_5:	//jhk_181006
						case C_LGC_90V_500A_250A_100A_180KW_6:	//jhk_181006
							if((k >= 72) && (k < 96)) k += 24;
							else if((k >= 96) && (k < 120)) k -= 24;
							break;
						case C_LGC_90V_500A_250A_100A_90KW:	//jhk_181107
							if((k >= 36) && (k < 48)) k += 28;
							else if((k >= 64) && (k < 76)) k -= 28;
							break;
						case C_GITC_100V_100A_50A_10A_40KW:	//jhk_180917
						case C_POWERLOGICS_100V_100A_50A_40KW:	//jhk_180919
							if((k >= 20) && (k < 40)) k += 12;
							else if((k >= 32) && (k < 52)) k -= 12;
							break;
						case C_LGCCHINA_100V_200A_20KW_3:	//jhk_160731
						case C_LGCCHINA_100V_200A_20KW_4:	//jhk_160731
						case C_LGCCHINA_100V_200A_20KW_5:	//jhk_160731
						case C_LGCCHINA_100V_200A_20KW_6:	//jhk_160731
						case C_LGCCHINA_100V_200A_20KW_7:	//jhk_160731
							if((k >= 16) && (k < 20)) k += 16;
							else if((k >= 32) && (k < 36)) k -= 16;
							break;
						case C_H2_100V_300A_100A_60KW:	//jhk_150726
						    if((k >= 8) && (k < 16)) k += 24;
						    else if((k >= 32) && (k < 40)) k -= 24;
						    break;
						//case C_SKI_100V_300A_150A_50A_10A_60KW:    //jhk_120604 //shh_200709
						//case C_SKI_100V_300A_150A_50A_10A_60KW_2:  //jhk_120604
						//case C_SKI_100V_300A_150A_50A_10A_60KW_3:  //jhk_120604 //shh_200709
						//case C_SKI_100V_300A_150A_50A_10A_60KW_4:  //jhk_120604
						//case C_SKI_100V_300A_150A_50A_10A_60KW_5:  //jhk_120604
						//case C_SKI_100V_300A_150A_50A_10A_60KW_6:  //jhk_120604
						//case C_SKI_100V_300A_150A_50A_10A_60KW_7:  //jhk_120604
						//case C_SKI_100V_300A_150A_50A_10A_60KW_8:  //jhk_120604 //shh_200709
						case C_HUOJU_125V_200A_100A_50KW:	//jhk_151107
						case C_HUOJU_125V_200A_100A_50KW_2:	//jhk_151107
							if((k >= 24) && (k < 48)) k += 8;
							else if((k >= 32) && (k < 56)) k -= 8;
							break;
						case C_LGC_120V_250A_50A_60KW:	//jhk_160617
							if((k >= 28) && (k < 34)) k += 4;
							else if((k >= 32) && (k < 38)) k -= 4;
							break;
						case C_SKI_120V_400A_100A_192KW:	//kjh_170808
						case C_SKI_120V_400A_100A_192KW_2:	//kjh_170808
						case C_SKI_120V_400A_100A_192KW_3:	//kjh_170808
						//case C_SKI_120V_400A_100A_192KW_4:	//kjh_170808
						case C_SKI_120V_400A_100A_192KW_5:	//kjh_170808
						case C_SKI_120V_400A_100A_192KW_6:	//kjh_170808
						//case C_SKI_120V_400A_100A_192KW_7:	//kjh_170808
						case C_SKI_120V_400A_100A_192KW_8:	//kjh_170808
							if((k >= 50) && (k < 54)) k += 10;
							else if((k >= 54) && (k < 64)) k -= 4;
							break;
						case C_SKI_120V_400A_200A_100A_50A_192KW:	//jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_2:	//jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_3:	//jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_4:	//jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_5:	//jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_6:	//jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_7:	//jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_8:	//jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_9:	//jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_10://jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_11://jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_12://jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_13://jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_14://jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_15://jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_16://jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_17://jhk_180206
						case C_SKI_120V_400A_200A_100A_50A_192KW_18://jhk_180206
							if((k >= 96) && (k < 104)) k += 24;
							else if((k >= 104) && (k < 128)) k -= 8;
							break;
						case C_LGC_150V_250A_100A_150KW:	//jhk_181014
						case C_LGC_150V_250A_100A_150KW_2:	//jhk_181014
						case C_LGC_150V_250A_100A_150KW_3:	//jhk_181014
						case C_LGC_150V_250A_100A_150KW_4:	//jhk_181014
						case C_LGC_150V_250A_100A_150KW_5:	//jhk_181014
						case C_LGC_150V_250A_100A_150KW_6:	//jhk_181014
						case C_LGC_150V_250A_100A_150KW_7:	//jhk_181014
						case C_LGC_150V_250A_100A_150KW_8:	//jhk_181014
						case C_LGC_150V_350A_100A_210KW_4:	//ktg_190421
							if((k >= 120) && (k < 144)) k += 8;
							else if((k >= 144) && (k < 152)) k -= 24;
							break;
						case C_LGC_200V_300A_100A_50A_10A_120KW: //jhkw_120813
							if((k >= 20) && (k < 40)) k += 12;
							else if((k >= 32) && (k < 52)) k -= 12;
							break;
						case C_LGC_250V_350A_250A_100A_175KW: //jhk_181108
							if((k >= 60) && (k < 72)) k += 4;
							else if((k >= 72) && (k < 76)) k -= 12;
							break;
						case C_LGC_400V_100A_10A_40KW:
						case C_3PSYSTEM_400V_100A_10A_40KW_1:	//jhk_120723
						case C_3PSYSTEM_400V_100A_10A_40KW_2:	//jhk_120723
							if((k >= 72) && (k < 82)) k += 24;
							else if((k >= 96) && (k < 106)) k -= 24;
							break;
						case C_LGC_450V_200A_10A_180KW:
							if((k >= 100) && (k < 200)) k += 28;
							else if((k >= 200) && (k < 228)) k -= 100;
							break;
						case C_LGC_450V_250A_50A_225KW:		//jhk_160608
						case C_LGC_450V_250A_50A_225KW_2:	//jhk_160608
							if((k >= 50) && (k < 100)) k += 14;
							else if((k >= 100) && (k < 106)) k += 28;
							else if((k >= 106) && (k < 120)) k -= 56;
							else if((k >= 120) && (k < 134)) k -= 6;
							break;
						case C_LGC_450V_250A_100A_225KW:	//jhk_170211
							if((k >= 100) && (k < 106)) k += 28;
							else if((k >= 128) && (k < 134)) k -= 28;
							break;
						case C_KATECH_450V_150A_10A:
						case C_KATECH_450V_250A_25A_225KW:
						case C_KATECH_450V_250A_25A_225KW_2:
						case C_LGC_500V_250A_100A_50A_250KW:    //jhkw_120809
						case C_DAEHWA_750V_300A_100A_150KW:	//jhk_170203
							if((k >= 16) && (k < 32)) k += 16;
							else if((k >= 32) && (k < 48)) k -= 16;
							break;
						case C_LGCCHINA_500V_150A_75KW:		//jhk_160727
						case C_LGCCHINA_500V_150A_75KW_2:	//jhk_160727
						case C_LGCCHINA_500V_150A_75KW_3:	//jhk_160727
							if((k >= 16) && (k < 20)) k += 16;
							else if((k >= 32) && (k < 36)) k -= 16;
							break;
						case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
							if((ch >= 120) && (ch < 216)) ch += 8;
							else if((ch >= 216) && (ch < 224)) ch -= 96;
							break;
						case C_SEBANG_600V_200A_100A_240KW:	//jhk_160223
							if((k >= 24) && (k < 48)) k += 8;
							else if((k >= 32) && (k < 56)) k -= 8;
							break;
						case C_HYUNDAI_750V_400A_100A_240KW:	//jhk_160909
						case C_HYUNDAI_750V_400A_100A_240KW_2:	//jhk_161111
							if((k >= 15) && (k < 30)) k += 17;
							else if((k >= 32) && (k < 47)) k -= 17;
							break;
						case C_DAEHWA_800V_200A_50A_160KW:	//jhk_160831
							if((k >= 24) && (k < 48)) k += 8;
							else if((k >= 32) && (k < 56)) k -= 8;
							break;
						case C_KATECH_1200V_300A_100A_50A_300KW:	//jhk_150108
							if((k >= 16) && (k < 32)) k += 16;
							else if((k >= 32) && (k < 48)) k -= 16;
							break;
						case C_SKI_1200V_250A_100A_50A_300KW:	//jhk_131209
							if((k >= 10) && (k < 20)) k += 22;
							else if((k >= 32) && (k < 42)) k -= 22;
							break;
						case C_SEBANG_1200V_300A_100A_360KW:	//jhk_160223
							if((k >= 24) && (k < 48)) k += 8;
							else if((k >= 32) && (k < 56)) k -= 8;
							break;
						case C_SKI_1500V_400A_100A_500KW:	//jhk_180610
							if((k >= 40) && (k < 80)) k += 24;
							else if((k >= 80) && (k < 104)) k -= 40;
							break;
						case C_KTL_1500V_400A_200A_700KW:	//jhk_180902
							if((k >= 8) && (k < 16)) k += 24;
							else if((k >= 32) && (k < 40)) k -= 24;
							break;
						case C_KCL_1500V_1000A_500A_600KW:	//jhk_181110
							if((k >= 16) && (k < 32)) k += 16;
							else if((k >= 32) && (k < 48)) k -= 16;
							break;
						default:
							break;
					}

					myData->save_msg_10ms[ch][msg].val[idx].auxData[j].val
						= myData->COM.com_port[1].value[k];
					myData->save_msg_10ms[ch][msg].val[idx].auxData[j].val
						= myData->SubSensV.ch[k].sensV;
					break;
				default:
					myData->save_msg_10ms[ch][msg].val[idx].auxData[j].val = 0;
					break;
			}
			j++;
		}
	}

	j = myData->canReceiveDataCount[ch][0] + myData->canReceiveDataCount[ch][1];
	for(i=0; i < j; i++) {
		myData->save_msg_10ms[ch][msg].val[idx].canData[i].canType
				= myData->canReceiveSetData.normalData[ch][i].canType;
		myData->save_msg_10ms[ch][msg].val[idx].canData[i].data_type
				= myData->canReceiveSetData.normalData[ch][i].data_type;

		switch(myData->AppControl.config.systemModel) {
			case C_SDI_70V_50A_5A_4KW:
			case C_SDI_70V_50A_5A_4KW_2:
			case C_SDI_70V_50A_5A_7KW:
			case C_SDI_70V_50A_5A_7KW_2:
			case C_SDI_70V_250A_25A_18KW:
				k = 0;
				if(i < myData->canReceiveDataCount[ch][0]) {
					if((myData->cData[ch].misc.external_comm_state & 0x04)
						!= 0) k = 10;
				} else {
					if((myData->cData[ch].misc.external_comm_state & 0x08)
						!= 0) k = 10;
				}
				if(k == 0) {
					k = (int)myData->canReceiveSetData
						.normalData[ch][i].data_type;
				}
				break;
			default:
				k = (int)myData->canReceiveSetData.normalData[ch][i].data_type;
				break;
		}

		switch(k) {
			case 0: //unsigned
				myData->save_msg_10ms[ch][msg].val[idx].canData[i].val.ul_val[0]
					= myData->CanData[ch][i].ul_val[0];
				break;
			case 1: //signed
				myData->save_msg_10ms[ch][msg].val[idx].canData[i].val.l_val[0]
					= myData->CanData[ch][i].l_val[0];
				break;
			case 2: //float
				myData->save_msg_10ms[ch][msg].val[idx].canData[i].val.f_val[0]
					= myData->CanData[ch][i].f_val[0];
				break;
			case 3: //string
				memcpy((char *)&myData->save_msg_10ms[ch][msg].val[idx].canData[i].val,
					(char *)&myData->CanData[ch][i], 8);
				break;
			case 4: //hex //kjhw_140811
				myData->save_msg_10ms[ch][msg].val[idx].canData[i].val.ul_val[0]
					= myData->CanData[ch][i].ul_val[0];
				break;
			default:
				memset((char *)&myData->save_msg_10ms[ch][msg].val[idx].canData[i].val,
					0, 8);
				break;
		}
	}

	myData->save_msg_10ms[ch][msg].write_idx = idx;
	myData->save_msg_10ms[ch][msg].total_count++;
	myData->save_msg_10ms[ch][msg].count[ch]++;
	myData->save_msg_10ms[ch][msg].count_100++;
#endif
}

void send_save_msg_3(int ch, int msg)
{
#ifdef __COC__
	unsigned char item, idxStepNo;
	int idx, group, i, j, k;
	long val, val1, val2;

	group = 0; //kjg_w
	item = idxStepNo = 0;
	i = j = k = 0;

	if(myData->save_msg_1[msg].send_flag == 1) { //send save_msg stop
		return;
	}

	if(myData->save_msg_1[msg].total_count >= (MAX_SAVE_MSG - 50)) {
		if(myData->cData[ch].op.select != SAVE_FLAG_SAVING_END) {
			return;
		} else {
			if(myData->save_msg_1[msg].total_count >= (MAX_SAVE_MSG - 5)) {
				return;
			}
		}
	}

	idx = myData->save_msg_1[msg].write_idx[ch];
	idx++;
	if(idx >= MAX_SAVE_MSG) idx = 0;

	myData->save_msg_1[msg].val[idx][ch].ch = (unsigned char)(ch + 1);
	myData->save_msg_1[msg].val[idx][ch].resultIndex
		= myData->cData[ch].op.resultIndex;
	myData->save_msg_1[msg].val[idx][ch].state
		= (unsigned char)convert_ch_state(CONVERT_ORG_TO_P3,
		(long)myData->cData[ch].op.state);
	myData->save_msg_1[msg].val[idx][ch].type
		= (unsigned char)convert_step_type(CONVERT_ORG_TO_P3,
		(long)myData->cData[ch].op.stepType);
	myData->save_msg_1[msg].val[idx][ch].mode
		= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P3,
		(long)myData->cData[ch].op.stepMode);
	
	myData->save_msg_1[msg].val[idx][ch].Vsens
		= myData->cData[ch].op.Vsens;

	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		val = myData->cData[ch].op.Isens;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.Isens;
			}
		}
		myData->save_msg_1[msg].val[idx][ch].Isens = val;

		val = myData->cData[ch].op.charge_AmpareHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_AmpareHour;
			}
		}
		//myData->save_msg_1[msg].val[idx][ch].charge_AmpareHour = val;
		val1 = val;

		val = myData->cData[ch].op.discharge_AmpareHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_AmpareHour;
			}
		}
		//myData->save_msg_1[msg].val[idx][ch].discharge_AmpareHour = val;
		if(myData->cData[ch].op.stepType == STEP_PATTERN) {
			val2 = val;
		} else {
			val2 = val * (-1);
		}
		myData->save_msg_1[msg].val[idx][ch].capacity = val1 + val2;

		val = myData->cData[ch].op.watt;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.watt;
			}
		}
		myData->save_msg_1[msg].val[idx][ch].watt = val;

		val = myData->cData[ch].op.charge_WattHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_WattHour;
			}
		}
		//myData->save_msg_1[msg].val[idx][ch].charge_WattHour = val;
		val1 = val;

		val = myData->cData[ch].op.discharge_WattHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_WattHour;
			}
		}
		//myData->save_msg_1[msg].val[idx][ch].discharge_WattHour = val;
		if(myData->cData[ch].op.stepType == STEP_PATTERN) {
			val2 = val;
		} else {
			val2 = val * (-1);
		}
		myData->save_msg_1[msg].val[idx][ch].wattHour = val1 + val2;

		val = myData->cData[ch].op.meanI;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.meanI;
			}
		}
		myData->save_msg_1[msg].val[idx][ch].avgI = val;
	} else {
		myData->save_msg_1[msg].val[idx][ch].Isens
			= myData->cData[ch].op.Isens;
		//myData->save_msg_1[msg].val[idx][ch].charge_AmpareHour
		val1 = myData->cData[ch].op.charge_AmpareHour;
		//myData->save_msg_1[msg].val[idx][ch].discharge_AmpareHour
		if(myData->cData[ch].op.stepType == STEP_PATTERN) {
			val2 = myData->cData[ch].op.discharge_AmpareHour;
		} else {
			val2 = myData->cData[ch].op.discharge_AmpareHour * (-1);
		}
		myData->save_msg_1[msg].val[idx][ch].capacity = val1 + val2;
		myData->save_msg_1[msg].val[idx][ch].watt
			= myData->cData[ch].op.watt;
		//myData->save_msg_1[msg].val[idx][ch].charge_WattHour
		val1 = myData->cData[ch].op.charge_WattHour;
		//myData->save_msg_1[msg].val[idx][ch].discharge_WattHour
		if(myData->cData[ch].op.stepType == STEP_PATTERN){
			val2 = myData->cData[ch].op.discharge_WattHour;
		} else {
			val2 = myData->cData[ch].op.discharge_WattHour * (-1);
		}
		myData->save_msg_1[msg].val[idx][ch].wattHour = val1 + val2;
		myData->save_msg_1[msg].val[idx][ch].avgI = myData->cData[ch].op.meanI;
	}

//	myData->save_msg_1[msg].val[idx][ch].capacitance
//		= myData->cData[ch].op.capacitance;

	myData->save_msg_1[msg].val[idx][ch].z
		= myData->cData[ch].op.z;
		//= myData->fch.ch[ch].v_power_bus; //kjg_d

	j = 0;
	for(i=0; i < MAX_AUX_DATA; i++) {
		if((ch+1) == myData->auxSetData[i].chNo) {
			k = myData->auxSetData[i].auxChNo - 1;
			myData->save_msg_1[msg].val[idx][ch].temp[j]
				= myData->COM.com_port[1].value[k];
			/*if(j == 0) {
				myData->save_msg_1[msg].val[idx][ch].temp[j]
					= myData->fch.ch[ch].v_output_bus;
			} else if(j == 1) {
				myData->save_msg_1[msg].val[idx][ch].temp[j]
					= myData->cData[ch].misc.d_r1;
			} else if(j == 2) {
				myData->save_msg_1[msg].val[idx][ch].temp[j]
					= myData->cData[ch].misc.d_r2;
			}*/ //kjg_d
			j++;
		}
		if(j == 3) break;
	}

	myData->save_msg_1[msg].val[idx][ch].select
		= myData->cData[ch].op.select;
	myData->save_msg_1[msg].val[idx][ch].code
		= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P3,
		(long)myData->cData[ch].op.code);
	myData->save_msg_1[msg].val[idx][ch].stepNo
		= myData->cData[ch].op.idxStepNo + 1;

	myData->save_msg_1[msg].val[idx][ch].runTime
		= myData->cData[ch].op.runTime;
	myData->save_msg_1[msg].val[idx][ch].totalRunTime
		= myData->cData[ch].op.totalRunTime;
/*	myData->save_msg_1[msg].val[idx][ch].cvTime
		= myData->cData[ch].op.cvTime;
	myData->save_msg_1[msg].val[idx][ch].ccTime
		= myData->cData[ch].op.ccTime;

	myData->save_msg_1[msg].val[idx][ch].realDate
		= myData->mData.real_time[6] * 10000 //year
		+ myData->mData.real_time[5] * 100 //month
		+ myData->mData.real_time[4]; //day
	myData->save_msg_1[msg].val[idx][ch].realClock
		= myData->mData.real_time[3] * 10000000 //hour
		+ myData->mData.real_time[2] * 100000 //min
		+ myData->mData.real_time[1] * 1000 //sec
		+ myData->mData.real_time[0]; //msec
*/
	myData->save_msg_1[msg].val[idx][ch].reservedCmd
		= myData->cData[ch].op.reservedCmd;
/*
	myData->save_msg_1[msg].val[idx][ch].auxDataCount
		= (short int)(myData->auxDataCount[ch][0]
		+ myData->auxDataCount[ch][1]);
	myData->save_msg_1[msg].val[idx][ch].canReceiveDataCount
		= (short int)(myData->canReceiveDataCount[ch][0]
		+ myData->canReceiveDataCount[ch][1]);
*/
	myData->save_msg_1[msg].val[idx][ch].totalCycle
		= myData->cData[ch].misc.totalCycle;
	myData->save_msg_1[msg].val[idx][ch].currentCycle
		= myData->cData[ch].elementCycle.cycle_count;
	myData->save_msg_1[msg].val[idx][ch].gotoCycleCount
		= myData->cData[ch].accCycle[0].cycle_count;
/*	for(i=0; i < MAX_ACC_CYCLE; i++) {
		myData->save_msg_1[msg].val[idx][ch].accCycle[i]
			= myData->cData[ch].accCycle[i].cycle_count;
	}
	for(i=0; i < MAX_MULTI_CYCLE; i++) {
		myData->save_msg_1[msg].val[idx][ch].multiCycle[i]
			= myData->cData[ch].multiCycle[i].cycle_count;
	}
*/
	myData->save_msg_1[msg].val[idx][ch].avgV = myData->cData[ch].op.meanV;

	if(myData->ChAttribute[ch].chamber_control == 0) {
		myData->save_msg_1[msg].val[idx][ch].chamber_control = 0;
	} else {
		myData->save_msg_1[msg].val[idx][ch].chamber_control = 1;
	}
/*
	if(myData->cData[ch].op.select == SAVE_FLAG_SAVING_END
		&& (myData->cData[ch].op.stepType == STEP_CHARGE
		|| myData->cData[ch].op.stepType == STEP_DISCHARGE
		|| myData->cData[ch].op.stepType == STEP_Z
		|| myData->cData[ch].op.stepType == STEP_OCV
		|| myData->cData[ch].op.stepType == STEP_REST)) {
		idxStepNo = myData->cData[ch].op.idxStepNo;
		item = myData->testCond[ch].grade[idxStepNo].item;
		if(item == GRADE_ITEM_V) {
			val = myData->save_msg_1[msg].val[idx][ch].Vsens;
			myData->save_msg_1[msg].val[idx][ch].grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else if(item == GRADE_ITEM_AMPARE_HOUR) {
			val = myData->save_msg_1[msg].val[idx][ch].charge_AmpareHour
				+ (myData->save_msg_1[msg].val[idx][ch].discharge_AmpareHour
				* (-1));
			myData->save_msg_1[msg].val[idx][ch].grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else if(item == GRADE_ITEM_Z) {
			val = myData->save_msg_1[msg].val[idx][ch].z;
			myData->save_msg_1[msg].val[idx][ch].grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else if(item == GRADE_ITEM_CAPACITANCE) {
			val = myData->save_msg_1[msg].val[idx][ch].capacitance;
			myData->save_msg_1[msg].val[idx][ch].grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else {
			myData->save_msg_1[msg].val[idx][ch].grade = 0;
		}
	}*/
	myData->save_msg_1[msg].val[idx][ch].grade = myData->cData[ch].op.grade;

	myData->save_msg_1[msg].write_idx[ch] = idx;
//	myData->save_msg_1[msg].total_count++;
	myData->save_msg_1[msg].count[ch]++;
#endif
}

void send_record_msg(int ch, int index)
{
#ifdef __COC__
	if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COC][0] == P1) {
		if(myData->ChAttribute[ch].chNo_master == 0) return;

		send_record_msg_1(ch, 0, index); //COC1_Client
		send_record_msg_1(ch, 1, index); //DataSave
	}
#endif
}

void send_record_msg_1(int ch, int msg, int index)
{
#ifdef __COC__
	unsigned char item, idxStepNo;
	int idx, group, i, j, k;
	long val, val1, val2;

	group = 0; //kjg_w
	item = idxStepNo = 0;
	i = j = k = 0;

	if(myData->record_msg_1[msg].send_flag == 1) { //send record_msg stop
		return;
	}

	if(myData->record_msg_1[msg].total_count >= (MAX_RECORD_MSG - 10)) {
		if(myData->cData[ch].op.select != SAVE_FLAG_SAVING_END) {
			return;
		} else {
			if(myData->record_msg_1[msg].total_count >= (MAX_RECORD_MSG - 5)) {
				return;
			}
		}
	}

	idx = myData->record_msg_1[msg].write_idx[ch];
	idx++;
	if(idx >= MAX_RECORD_MSG) idx = 0;

	myData->record_msg_1[msg].val[idx][ch].ch = (unsigned char)(ch + 1);
	myData->record_msg_1[msg].val[idx][ch].resultIndex
		= myData->cData[ch].op.resultIndex;
	myData->record_msg_1[msg].val[idx][ch].state
		= (unsigned char)convert_ch_state(CONVERT_ORG_TO_P3,
		(long)myData->cData[ch].op.state);
	myData->record_msg_1[msg].val[idx][ch].type
		= (unsigned char)convert_step_type(CONVERT_ORG_TO_P3,
		(long)myData->cData[ch].op.stepType);
	myData->record_msg_1[msg].val[idx][ch].mode
		= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P3,
		(long)myData->cData[ch].op.stepMode);
	
	myData->record_msg_1[msg].val[idx][ch].Vsens
		= myData->cData[ch].op.Vsens;

	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		val = myData->cData[ch].op.Isens;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.Isens;
			}
		}
		myData->record_msg_1[msg].val[idx][ch].Isens = val;

		val = myData->cData[ch].op.charge_AmpareHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_AmpareHour;
			}
		}
		//myData->record_msg_1[msg].val[idx][ch].charge_AmpareHour = val;
		val1 = val;

		val = myData->cData[ch].op.discharge_AmpareHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_AmpareHour;
			}
		}
		//myData->record_msg_1[msg].val[idx][ch].discharge_AmpareHour = val;
		if(myData->cData[ch].op.stepType == STEP_PATTERN) {
			val2 = val;
		} else {
			val2 = val * (-1);
		}
		myData->record_msg_1[msg].val[idx][ch].capacity = val1 + val2;

		val = myData->cData[ch].op.watt;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.watt;
			}
		}
		myData->record_msg_1[msg].val[idx][ch].watt = val;

		val = myData->cData[ch].op.charge_WattHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_WattHour;
			}
		}
		//myData->record_msg_1[msg].val[idx][ch].charge_WattHour = val;
		val1 = val;

		val = myData->cData[ch].op.discharge_WattHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_WattHour;
			}
		}
		//myData->record_msg_1[msg].val[idx][ch].discharge_WattHour = val;
		if(myData->cData[ch].op.stepType == STEP_PATTERN) {
			val2 = val;
		} else {
			val2 = val * (-1);
		}
		myData->record_msg_1[msg].val[idx][ch].wattHour = val1 + val2;

		val = myData->cData[ch].op.meanI;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.meanI;
			}
		}
		myData->record_msg_1[msg].val[idx][ch].avgI = val;
	} else {
		myData->record_msg_1[msg].val[idx][ch].Isens
			= myData->cData[ch].op.Isens;
		//myData->record_msg_1[msg].val[idx][ch].charge_AmpareHour
		val1 = myData->cData[ch].op.charge_AmpareHour;
		//myData->record_msg_1[msg].val[idx][ch].discharge_AmpareHour
		if(myData->cData[ch].op.stepType == STEP_PATTERN) {
			val2 = myData->cData[ch].op.discharge_AmpareHour;
		} else {
			val2 = myData->cData[ch].op.discharge_AmpareHour * (-1);
		}
		myData->record_msg_1[msg].val[idx][ch].capacity = val1 + val2;
		myData->record_msg_1[msg].val[idx][ch].watt
			= myData->cData[ch].op.watt;
		//myData->record_msg_1[msg].val[idx][ch].charge_WattHour
		val1 = myData->cData[ch].op.charge_WattHour;
		//myData->record_msg_1[msg].val[idx][ch].discharge_WattHour
		if(myData->cData[ch].op.stepType == STEP_PATTERN) {
			val2 = myData->cData[ch].op.discharge_WattHour;
		} else {
			val2 = myData->cData[ch].op.discharge_WattHour * (-1);
		}
		myData->record_msg_1[msg].val[idx][ch].wattHour = val1 + val2;
		myData->record_msg_1[msg].val[idx][ch].avgI
			= myData->cData[ch].op.meanI;
	}

//	myData->record_msg_1[msg].val[idx][ch].capacitance
//		= myData->cData[ch].op.capacitance;

	myData->record_msg_1[msg].val[idx][ch].z
		= myData->cData[ch].op.z;
		//= myData->fch.ch[ch].v_power_bus; //kjg_d

	j = 0;
	for(i=0; i < MAX_AUX_DATA; i++) {
		if((ch+1) == myData->auxSetData[i].chNo) {
			k = myData->auxSetData[i].auxChNo - 1;
			myData->record_msg_1[msg].val[idx][ch].temp[j]
				= myData->COM.com_port[1].value[k];
			/*if(j == 0) {
				myData->record_msg_1[msg].val[idx][ch].temp[j]
					= myData->fch.ch[ch].v_output_bus;
			} else if(j == 1) {
				myData->record_msg_1[msg].val[idx][ch].temp[j]
					= myData->cData[ch].misc.d_r1;
			} else if(j == 2) {
				myData->record_msg_1[msg].val[idx][ch].temp[j]
					= myData->cData[ch].misc.d_r2;
			}*/ //kjg_d
			j++;
		}
		if(j == 3) break;
	}

	myData->record_msg_1[msg].val[idx][ch].select
		= SAVE_FLAG_RECORD_DATA;
	myData->record_msg_1[msg].val[idx][ch].record_index = (unsigned char)index;

	myData->record_msg_1[msg].val[idx][ch].code
		= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P3,
		(long)myData->cData[ch].op.code);
	myData->record_msg_1[msg].val[idx][ch].stepNo
		= myData->cData[ch].op.idxStepNo + 1;

	myData->record_msg_1[msg].val[idx][ch].runTime
		= myData->cData[ch].op.runTime;
	myData->record_msg_1[msg].val[idx][ch].totalRunTime
		= myData->cData[ch].op.totalRunTime;
/*	myData->record_msg_1[msg].val[idx][ch].cvTime
		= myData->cData[ch].op.cvTime;
	myData->record_msg_1[msg].val[idx][ch].ccTime
		= myData->cData[ch].op.ccTime;

	myData->record_msg_1[msg].val[idx][ch].realDate
		= myData->mData.real_time[6] * 10000 //year
		+ myData->mData.real_time[5] * 100 //month
		+ myData->mData.real_time[4]; //day
	myData->record_msg_1[msg].val[idx][ch].realClock
		= myData->mData.real_time[3] * 10000000 //hour
		+ myData->mData.real_time[2] * 100000 //min
		+ myData->mData.real_time[1] * 1000 //sec
		+ myData->mData.real_time[0]; //msec
*/
	myData->record_msg_1[msg].val[idx][ch].reservedCmd
		= myData->cData[ch].op.reservedCmd;
/*
	myData->record_msg_1[msg].val[idx][ch].auxDataCount
		= (short int)(myData->auxDataCount[ch][0]
		+ myData->auxDataCount[ch][1]);
	myData->record_msg_1[msg].val[idx][ch].canReceiveDataCount
		= (short int)(myData->canReceiveDataCount[ch][0]
		+ myData->canReceiveDataCount[ch][1]);
*/
	myData->record_msg_1[msg].val[idx][ch].totalCycle
		= myData->cData[ch].misc.totalCycle;
	myData->record_msg_1[msg].val[idx][ch].currentCycle
		= myData->cData[ch].elementCycle.cycle_count;
	myData->record_msg_1[msg].val[idx][ch].gotoCycleCount
		= myData->cData[ch].accCycle[0].cycle_count;
/*	for(i=0; i < MAX_ACC_CYCLE; i++) {
		myData->record_msg_1[msg].val[idx][ch].accCycle[i]
			= myData->cData[ch].accCycle[i].cycle_count;
	}
	for(i=0; i < MAX_MULTI_CYCLE; i++) {
		myData->record_msg_1[msg].val[idx][ch].multiCycle[i]
			= myData->cData[ch].multiCycle[i].cycle_count;
	}
*/
	myData->record_msg_1[msg].val[idx][ch].avgV = myData->cData[ch].op.meanV;
/*
	if(myData->cData[ch].op.select == SAVE_FLAG_SAVING_END
		&& (myData->cData[ch].op.stepType == STEP_CHARGE
		|| myData->cData[ch].op.stepType == STEP_DISCHARGE
		|| myData->cData[ch].op.stepType == STEP_Z
		|| myData->cData[ch].op.stepType == STEP_OCV
		|| myData->cData[ch].op.stepType == STEP_REST)) {
		idxStepNo = myData->cData[ch].op.idxStepNo;
		item = myData->testCond[ch].grade[idxStepNo].item;
		if(item == GRADE_ITEM_V) {
			val = myData->record_msg_1[msg].val[idx][ch].Vsens;
			myData->record_msg_1[msg].val[idx][ch].grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else if(item == GRADE_ITEM_AMPARE_HOUR) {
			val = myData->record_msg_1[msg].val[idx][ch].charge_AmpareHour
				+ (myData->record_msg_1[msg].val[idx][ch].discharge_AmpareHour
				* (-1));
			myData->record_msg_1[msg].val[idx][ch].grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else if(item == GRADE_ITEM_Z) {
			val = myData->record_msg_1[msg].val[idx][ch].z;
			myData->record_msg_1[msg].val[idx][ch].grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else if(item == GRADE_ITEM_CAPACITANCE) {
			val = myData->record_msg_1[msg].val[idx][ch].capacitance;
			myData->record_msg_1[msg].val[idx][ch].grade
				= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
		} else {
			myData->record_msg_1[msg].val[idx][ch].grade = 0;
		}
	}*/
	myData->record_msg_1[msg].val[idx][ch].grade = myData->cData[ch].op.grade;

	myData->record_msg_1[msg].write_idx[ch] = idx;
//	myData->record_msg_1[msg].total_count++;
	myData->record_msg_1[msg].count[ch]++;
#endif
}

void send_pulse_msg(int ch, int count_flag)
{
#ifdef __COA__
	if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COA][0] == P1) {
		if(myData->ChAttribute[ch].chNo_master == 0) return;

		send_pulse_msg_1(ch, 0, count_flag); //COA1_Client
		send_pulse_msg_1(ch, 1, count_flag); //DataSave
	}
#endif
}

void send_pulse_msg_1(int ch, int msg, int count_flag)
{
#ifdef __COA__
	short int idxStepNo;
	unsigned char item; //, idxStepNo;		//khj_200809
	short int pulse_t1, pulse_t2, pulse_t3;
	int idx, group, i, j, k;
	long val, val1, val2;

	group = 0; //kjg_w
	item = idxStepNo = 0;
	i = j = k = 0;

	if(myData->pulse_msg_1[msg].send_flag == 1) { //send pulse_msg stop
		return;
	}

	if(myData->pulse_msg_1[msg].total_count >= (MAX_PULSE_MSG - 50)) {
		if(myData->cData[ch].op.select != SAVE_FLAG_SAVING_END) {
			return;
		} else {
			if(myData->pulse_msg_1[msg].total_count >= (MAX_PULSE_MSG - 5)) {
				return;
			}
		}
	}

	idx = myData->pulse_msg_1[msg].write_idx[ch];
	idx++;
	if(idx >= MAX_PULSE_MSG) idx = 0;

	//dcr_t1:5000, dcr_count1:400, dcr_t2:100, dcr_count2:100
	pulse_t1 = (myPs->config.dcr_t1/10) - myPs->config.dcr_count1 + 1; //ex)101
	pulse_t2 = (myPs->config.dcr_t1/10) + 1; //ex)501
	pulse_t3 = (myPs->config.dcr_t1/10) + (myPs->config.dcr_t2/10)
		+ myPs->config.dcr_count2; //ex)610

	if(myData->cData[ch].misc.pulse_count <= pulse_t1) {
		return;
	} else if(myData->cData[ch].misc.pulse_count > pulse_t1
		&& myData->cData[ch].misc.pulse_count < pulse_t2) {
		if(msg == 0) {
			i = myData->cData[ch].capa.d_count;
			myData->cData[ch].capa.d_count++;
			myData->cData[ch].capa.d_t[i] = (long)i;
			myData->cData[ch].capa.d_v[i] = myData->cData[ch].misc.tmpVsens;
			myData->cData[ch].capa.d_i[i]
				= myData->cData[ch].misc.total_tmpIsens;
		}
		return;
	} else if(myData->cData[ch].misc.pulse_count == pulse_t2) {
		if(msg == 0) {
			i = myData->cData[ch].capa.d_count;
			myData->cData[ch].capa.d_count++;
			myData->cData[ch].capa.d_t[i] = (long)i;
			myData->cData[ch].capa.d_v[i] = myData->cData[ch].misc.tmpVsens;
			myData->cData[ch].capa.d_i[i]
				= myData->cData[ch].misc.total_tmpIsens;

			calculate_DCR_2(ch, 0); //v1
			myData->cData[ch].capa.d_count = 1;
		}
	} else if(myData->cData[ch].misc.pulse_count < pulse_t3) {
		if(msg == 0) {
			i = myData->cData[ch].capa.d_count;
			myData->cData[ch].capa.d_count++;
			myData->cData[ch].capa.d_t[i] = (long)i;
			myData->cData[ch].capa.d_v[i] = myData->cData[ch].misc.tmpVsens;
			myData->cData[ch].capa.d_i[i]
				= myData->cData[ch].misc.total_tmpIsens;
		}
	} else if(myData->cData[ch].misc.pulse_count == pulse_t3) {
		if(msg == 0) {
			i = myData->cData[ch].capa.d_count;
			myData->cData[ch].capa.d_count++;
			myData->cData[ch].capa.d_t[i] = (long)i;
			myData->cData[ch].capa.d_v[i] = myData->cData[ch].misc.tmpVsens;
			myData->cData[ch].capa.d_i[i]
				= myData->cData[ch].misc.total_tmpIsens;

			calculate_DCR_2(ch, 1); //v2
		}
	}

	myData->pulse_msg_1[msg].val[idx][ch].count_flag
		= (unsigned char)(count_flag);
	myData->pulse_msg_1[msg].val[idx][ch].type
		= (int)convert_step_type(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.stepType);
	
	myData->pulse_msg_1[msg].val[idx][ch].totalCycle
		= (long)myData->cData[ch].misc.totalCycle;
	myData->pulse_msg_1[msg].val[idx][ch].stepNo
		= (long)myData->cData[ch].op.idxStepNo + 1;

	//val = (long)myData->cData[ch].op.runTime;
	//val = (long)myData->cData[ch].misc.pulse_count - 501;
	val = (long)myData->cData[ch].misc.pulse_count - (long)pulse_t2;
	myData->pulse_msg_1[msg].val[idx][ch].runTime = val;

	//if(myData->cData[ch].misc.pulse_count == 501) {
	if(myData->cData[ch].misc.pulse_count == pulse_t2) {
		myData->pulse_msg_1[msg].val[idx][ch].Vsens
			= myData->cData[ch].ccv[1].avg_v;
		myData->pulse_msg_1[msg].val[idx][ch].Isens
			= myData->cData[ch].ccv[1].avg_i;
	} else {
		myData->pulse_msg_1[msg].val[idx][ch].Vsens
			= myData->cData[ch].misc.tmpVsens;
		myData->pulse_msg_1[msg].val[idx][ch].Isens
			= myData->cData[ch].misc.total_tmpIsens;
	}

	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		val = myData->cData[ch].op.charge_AmpareHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_AmpareHour;
			}
		}
		val1 = val;

		val = myData->cData[ch].op.discharge_AmpareHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_AmpareHour;
			}
		}
		if(myData->cData[ch].op.stepType == STEP_PATTERN) {
			val2 = val;
		} else {
			val2 = val * (-1);
		}
		myData->pulse_msg_1[msg].val[idx][ch].capacity = val1 + val2;

		val = myData->cData[ch].op.charge_WattHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_WattHour;
			}
		}
		val1 = val;

		val = myData->cData[ch].op.discharge_WattHour;
		for(i=0; i < 3; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_WattHour;
			}
		}
		if(myData->cData[ch].op.stepType == STEP_PATTERN) {
			val2 = val;
		} else {
			val2 = val * (-1);
		}
		myData->pulse_msg_1[msg].val[idx][ch].wattHour = val1 + val2;
	} else {
		val1 = myData->cData[ch].op.charge_AmpareHour;
		if(myData->cData[ch].op.stepType == STEP_PATTERN) {
			val2 = myData->cData[ch].op.discharge_AmpareHour;
		} else {
			val2 = myData->cData[ch].op.discharge_AmpareHour * (-1);
		}
		myData->pulse_msg_1[msg].val[idx][ch].capacity = val1 + val2;

		val1 = myData->cData[ch].op.charge_WattHour;
		if(myData->cData[ch].op.stepType == STEP_PATTERN) {
			val2 = myData->cData[ch].op.discharge_WattHour;
		} else {
			val2 = myData->cData[ch].op.discharge_WattHour * (-1);
		}
		myData->pulse_msg_1[msg].val[idx][ch].wattHour = val1 + val2;
	}

	myData->pulse_msg_1[msg].write_idx[ch] = idx;
	myData->pulse_msg_1[msg].count[ch]++;
#endif
}

void send_save_msg_formation(int group, int div)
{
#ifdef __COB__
	int toPs;

	toPs = SAVE_GROUP1_TO_DATASAVE + group;
	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
			send_save_msg_formation_2(group, toPs, div);
			break;
		case F_SDI_5V_450A_200A_100A_10A:
			send_save_msg_formation_3(group, toPs, div);
			break;
		default:
			send_save_msg_formation_1(group, toPs, div);
			break;
	}

	if(myData->gData[group].workMode == WORK_TYPE_ONLINE) {
	} else {
		toPs = SAVE_GROUP_TO_COB1 + group;
		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
				send_save_msg_formation_2(group, toPs, div);
				break;
			case F_SDI_5V_450A_200A_100A_10A:
				send_save_msg_formation_3(group, toPs, div);
				break;
			default:
				send_save_msg_formation_1(group, toPs, div);
				break;
		}
	}
#endif
}

void send_save_msg_formation_1(int group, int toPs, int div)
{ //debug_size_cob
/*	int idx, ch, ch_start, i;
	unsigned char item, idxStepNo, attr;
	long val;

	if(myData->f_save_msg[toPs].send_flag == 1) { //send f_save_msg stop
		return;
	}

	idx = myData->f_save_msg[toPs].write_idx;
	idx++;
	if(idx >= MAX_F_SAVE_MSG) idx = 0;

	myData->f_save_msg[toPs].val[idx].state
		= (unsigned char)convert_group_state(CONVERT_ORG_TO_P2,
		(long)myData->gData[group].state);
	myData->f_save_msg[toPs].val[idx].type
		= (unsigned char)convert_step_type(CONVERT_ORG_TO_P2,
		(long)myData->gData[group].stepType);
	myData->f_save_msg[toPs].val[idx].mode
		= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P2,
		(long)myData->gData[group].stepMode);
	myData->f_save_msg[toPs].val[idx].attribute
		= myData->gData[group].attribute;
	attr = myData->gData[group].attribute;

	ch_start = myPs->config.chPerBd * group;
	for(ch=ch_start; ch < ch_start + myPs->config.chInGroup[group]; ch++) {
		myData->f_save_msg[toPs].val[idx].chData[ch].ch
			= (unsigned char)(ch + 1);
		myData->f_save_msg[toPs].val[idx].chData[ch].resultIndex
			= myData->cData[ch].opSave.resultIndex;
		myData->f_save_msg[toPs].val[idx].chData[ch].state
			= (unsigned char)convert_ch_state_org_to_p2(
			(long)myData->cData[ch].opSave.state,
			(long)myData->cData[ch].opSave.stepType,
			(long)attr, (long)myData->cData[ch].opSave.phase);
		myData->f_save_msg[toPs].val[idx].chData[ch].stepType
			= (unsigned char)convert_step_type(CONVERT_ORG_TO_P2,
			(long)myData->cData[ch].opSave.stepType);
		myData->f_save_msg[toPs].val[idx].chData[ch].stepMode
			= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P2,
			(long)myData->cData[ch].opSave.stepMode);
	
		myData->f_save_msg[toPs].val[idx].chData[ch].Vsens
			= myData->cData[ch].opSave.Vsens;

		myData->f_save_msg[toPs].val[idx].chData[ch].Isens
			= myData->cData[ch].opSave.Isens;
		myData->f_save_msg[toPs].val[idx].chData[ch].charge_AmpareHour
			= myData->cData[ch].opSave.charge_AmpareHour;
		myData->f_save_msg[toPs].val[idx].chData[ch].discharge_AmpareHour
			= myData->cData[ch].opSave.discharge_AmpareHour;
		myData->f_save_msg[toPs].val[idx].chData[ch].watt
			= myData->cData[ch].opSave.watt;
		myData->f_save_msg[toPs].val[idx].chData[ch].charge_WattHour
			= myData->cData[ch].opSave.charge_WattHour;
		myData->f_save_msg[toPs].val[idx].chData[ch].discharge_WattHour
			= myData->cData[ch].opSave.discharge_WattHour;
		myData->f_save_msg[toPs].val[idx].chData[ch].avgI
			= myData->cData[ch].opSave.meanI;

		myData->f_save_msg[toPs].val[idx].chData[ch].capacitance
			= myData->cData[ch].opSave.capacitance;
		myData->f_save_msg[toPs].val[idx].chData[ch].z
			= myData->cData[ch].opSave.z;

		myData->f_save_msg[toPs].val[idx].chData[ch].select
			= myData->cData[ch].opSave.select;
		myData->f_save_msg[toPs].val[idx].chData[ch].code
			= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P2,
			(long)myData->cData[ch].opSave.code);
		myData->f_save_msg[toPs].val[idx].chData[ch].stepNo
			= myData->cData[ch].opSave.client_stepNo;

		myData->f_save_msg[toPs].val[idx].chData[ch].runTime
			= myData->cData[ch].opSave.runTime;
		myData->f_save_msg[toPs].val[idx].chData[ch].totalRunTime
			= myData->cData[ch].opSave.totalRunTime;
		myData->f_save_msg[toPs].val[idx].chData[ch].cvTime
			= myData->cData[ch].opSave.cvTime;
		myData->f_save_msg[toPs].val[idx].chData[ch].ccTime
			= myData->cData[ch].opSave.ccTime;

		myData->f_save_msg[toPs].val[idx].chData[ch].reservedCmd
			= myData->cData[ch].opSave.reservedCmd;

		myData->f_save_msg[toPs].val[idx].chData[ch].totalCycle
			= myData->cData[ch].misc.totalCycle;
		myData->f_save_msg[toPs].val[idx].chData[ch].elementCycle
			= myData->cData[ch].elementCycle.cycle_count;
		for(i=0; i < MAX_ACC_CYCLE; i++) {
			myData->f_save_msg[toPs].val[idx].chData[ch].accCycle[i]
				= myData->cData[ch].accCycle[i].cycle_count;
		}
		for(i=0; i < MAX_MULTI_CYCLE; i++) {
			myData->f_save_msg[toPs].val[idx].chData[ch].multiCycle[i]
				= myData->cData[ch].multiCycle[i].cycle_count;
		}

		myData->f_save_msg[toPs].val[idx].chData[ch].avgV
			= myData->cData[ch].opSave.meanV;

		if(myData->cData[ch].opSave.select == SAVE_FLAG_SAVING_END
			&& (myData->cData[ch].opSave.stepType == STEP_CHARGE
			|| myData->cData[ch].opSave.stepType == STEP_DISCHARGE
			|| myData->cData[ch].opSave.stepType == STEP_Z
			|| myData->cData[ch].opSave.stepType == STEP_OCV
			|| myData->cData[ch].opSave.stepType == STEP_REST)) {
			idxStepNo = myData->cData[ch].opSave.idxStepNo;
			item = myData->testCond[ch_start].grade[idxStepNo].item;
			if(item == GRADE_ITEM_V) {
				val = myData->f_save_msg[toPs].val[idx].chData[ch].Vsens;
				myData->f_save_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_AMPARE_HOUR) {
				val = myData->f_save_msg[toPs].val[idx].chData[ch]
					.charge_AmpareHour
					+ (myData->f_save_msg[toPs].val[idx].chData[ch]
					.discharge_AmpareHour * (-1));
				myData->f_save_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_Z) {
				val = myData->f_save_msg[toPs].val[idx].chData[ch].z;
				myData->f_save_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_CAPACITANCE) {
				val = myData->f_save_msg[toPs].val[idx].chData[ch].capacitance;
				myData->f_save_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else {
				myData->f_save_msg[toPs].val[idx].chData[ch].grade = 0;
			}
		}
	}

	myData->f_save_msg[toPs].write_idx = idx;
	myData->f_save_msg[toPs].count++;*/
}

void send_save_msg_formation_2(int group, int toPs, int div)
{ //debug_size_cob
/*	int idx, ch, ch_start, scan_ch, scan_step, i;
	unsigned char item, idxStepNo, attr;
	long val;

	if(myData->f_save_msg[toPs].send_flag == 1) { //send f_save_msg stop
		return;
	}

	idx = myData->f_save_msg[toPs].write_idx;
	idx++;
	if(idx >= MAX_F_SAVE_MSG) idx = 0;

	if(div == 0) {
		scan_ch = myData->gData[group].misc.scan_ch[div];
	} else {
		scan_ch = myData->gData[group].misc.scan_ch[div] + 12;
	}
	scan_step = myData->gData[group].misc.scan_step[div];

	myData->f_save_msg[toPs].val[idx].state
		= (unsigned char)convert_group_state(CONVERT_ORG_TO_P2,
		(long)myData->gData[group].state);

	myData->f_save_msg[toPs].val[idx].type
		= (unsigned char)convert_step_type(CONVERT_ORG_TO_P2,
		(long)myData->COB_opSave[group][scan_ch][scan_step].stepType);
	myData->f_save_msg[toPs].val[idx].mode
		= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P2,
		(long)myData->COB_opSave[group][scan_ch][scan_step].stepMode);
	myData->f_save_msg[toPs].val[idx].attribute
		= myData->COB_opSave[group][scan_ch][scan_step].attribute;
	attr = myData->COB_opSave[group][scan_ch][scan_step].attribute;

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_400A_200A_100A_10A_2:
			ch_start = 0;
			break;
		default:
			ch_start = myPs->config.chPerBd * group;
			break;
	}

	for(ch=ch_start; ch < ch_start + myPs->config.chInGroup[group]; ch++) {
		myData->f_save_msg[toPs].val[idx].chData[ch].ch
			= (unsigned char)(ch + 1);
		myData->f_save_msg[toPs].val[idx].chData[ch].resultIndex
			= myData->COB_opSave[group][ch][scan_step].resultIndex;
		myData->f_save_msg[toPs].val[idx].chData[ch].state
			= (unsigned char)convert_ch_state_org_to_p2(
			(long)myData->COB_opSave[group][ch][scan_step].state,
			(long)myData->COB_opSave[group][ch][scan_step].stepType,
			(long)attr, (long)myData->COB_opSave[group][ch][scan_step].phase);
		myData->f_save_msg[toPs].val[idx].chData[ch].stepType
			= (unsigned char)convert_step_type(CONVERT_ORG_TO_P2,
			(long)myData->COB_opSave[group][ch][scan_step].stepType);
		myData->f_save_msg[toPs].val[idx].chData[ch].stepMode
			= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P2,
			(long)myData->COB_opSave[group][ch][scan_step].stepMode);

		myData->f_save_msg[toPs].val[idx].chData[ch].Vsens
			= myData->COB_opSave[group][ch][scan_step].Vsens;

		myData->f_save_msg[toPs].val[idx].chData[ch].Isens
			= myData->COB_opSave[group][ch][scan_step].Isens;
		myData->f_save_msg[toPs].val[idx].chData[ch].charge_AmpareHour
			= myData->COB_opSave[group][ch][scan_step].charge_AmpareHour;
		myData->f_save_msg[toPs].val[idx].chData[ch].discharge_AmpareHour
			= myData->COB_opSave[group][ch][scan_step].discharge_AmpareHour;
		myData->f_save_msg[toPs].val[idx].chData[ch].watt
			= myData->COB_opSave[group][ch][scan_step].watt;
		myData->f_save_msg[toPs].val[idx].chData[ch].charge_WattHour
			= myData->COB_opSave[group][ch][scan_step].charge_WattHour;
		myData->f_save_msg[toPs].val[idx].chData[ch].discharge_WattHour
			= myData->COB_opSave[group][ch][scan_step].discharge_WattHour;
		myData->f_save_msg[toPs].val[idx].chData[ch].avgI
			= myData->COB_opSave[group][ch][scan_step].meanI;

		myData->f_save_msg[toPs].val[idx].chData[ch].capacitance
			= myData->COB_opSave[group][ch][scan_step].capacitance;
		myData->f_save_msg[toPs].val[idx].chData[ch].z
			= myData->COB_opSave[group][ch][scan_step].z;

		myData->f_save_msg[toPs].val[idx].chData[ch].select
			= myData->COB_opSave[group][ch][scan_step].select;
		myData->f_save_msg[toPs].val[idx].chData[ch].code
			= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P2,
			(long)myData->COB_opSave[group][ch][scan_step].code);
			//= (unsigned char)myData->COB_opSave[group][ch][scan_step].code; //kjg_d
		myData->f_save_msg[toPs].val[idx].chData[ch].stepNo
			= myData->COB_opSave[group][ch][scan_step].client_stepNo;

		myData->f_save_msg[toPs].val[idx].chData[ch].runTime
			= myData->COB_opSave[group][ch][scan_step].runTime;
		myData->f_save_msg[toPs].val[idx].chData[ch].totalRunTime
			= myData->COB_opSave[group][ch][scan_step].totalRunTime;
		myData->f_save_msg[toPs].val[idx].chData[ch].cvTime
			= myData->COB_opSave[group][ch][scan_step].cvTime;
		myData->f_save_msg[toPs].val[idx].chData[ch].ccTime
			= myData->COB_opSave[group][ch][scan_step].ccTime;

		myData->f_save_msg[toPs].val[idx].chData[ch].reservedCmd
			= myData->COB_opSave[group][ch][scan_step].reservedCmd;

		myData->f_save_msg[toPs].val[idx].chData[ch].totalCycle
			= myData->cData[ch].misc.totalCycle;
		myData->f_save_msg[toPs].val[idx].chData[ch].elementCycle
			= myData->cData[ch].elementCycle.cycle_count;
		for(i=0; i < MAX_ACC_CYCLE; i++) {
			myData->f_save_msg[toPs].val[idx].chData[ch].accCycle[i]
				= myData->cData[ch].accCycle[i].cycle_count;
		}
		for(i=0; i < MAX_MULTI_CYCLE; i++) {
			myData->f_save_msg[toPs].val[idx].chData[ch].multiCycle[i]
				= myData->cData[ch].multiCycle[i].cycle_count;
		}

		myData->f_save_msg[toPs].val[idx].chData[ch].avgV
			= myData->COB_opSave[group][ch][scan_step].meanV;

		if(myData->COB_opSave[group][ch][scan_step].select
				== SAVE_FLAG_SAVING_END
			&& (myData->COB_opSave[group][ch][scan_step].stepType == STEP_CHARGE
			|| myData->COB_opSave[group][ch][scan_step].stepType
				== STEP_DISCHARGE
			|| myData->COB_opSave[group][ch][scan_step].stepType == STEP_Z
			|| myData->COB_opSave[group][ch][scan_step].stepType == STEP_OCV
			|| myData->COB_opSave[group][ch][scan_step].stepType
				== STEP_REST)) {
			idxStepNo = myData->COB_opSave[group][ch][scan_step].idxStepNo;
			item = myData->testCond[0].grade[idxStepNo].item;
			if(item == GRADE_ITEM_V) {
				val = myData->f_save_msg[toPs].val[idx].chData[ch].Vsens;
				myData->f_save_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_AMPARE_HOUR) {
				val = myData->f_save_msg[toPs].val[idx].chData[ch]
					.charge_AmpareHour
					+ (myData->f_save_msg[toPs].val[idx].chData[ch]
					.discharge_AmpareHour * (-1));
				myData->f_save_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_Z) {
				val = myData->f_save_msg[toPs].val[idx].chData[ch].z;
				myData->f_save_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_CAPACITANCE) {
				val = myData->f_save_msg[toPs].val[idx].chData[ch].capacitance;
				myData->f_save_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else {
				myData->f_save_msg[toPs].val[idx].chData[ch].grade = 0;
			}
		}
	}

	myData->f_save_msg[toPs].write_idx = idx;
	myData->f_save_msg[toPs].count++;*/
}

void send_save_msg_formation_3(int group, int toPs, int div)
{ //debug_size_cob
/*	int idx, ch, ch_start, scan_ch, scan_step, i;
	unsigned char item, idxStepNo, attr;
	long val;

	if(myData->f_save_msg[toPs].send_flag == 1) { //send f_save_msg stop
		return;
	}

	idx = myData->f_save_msg[toPs].write_idx;
	idx++;
	if(idx >= MAX_F_SAVE_MSG) idx = 0;

	if(div == 0) {
		scan_ch = myData->gData[group].misc.scan_ch[div];
	} else {
		scan_ch = myData->gData[group].misc.scan_ch[div] + 12;
	}
	scan_step = myData->gData[group].misc.scan_step[div];

	myData->f_save_msg[toPs].val[idx].state
		= (unsigned char)convert_group_state(CONVERT_ORG_TO_P2,
		(long)myData->gData[group].state);

	myData->f_save_msg[toPs].val[idx].type
		= (unsigned char)convert_step_type(CONVERT_ORG_TO_P2,
		(long)myData->COB_opSave[group][scan_ch][scan_step].stepType);
	myData->f_save_msg[toPs].val[idx].mode
		= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P2,
		(long)myData->COB_opSave[group][scan_ch][scan_step].stepMode);
	myData->f_save_msg[toPs].val[idx].attribute
		= myData->COB_opSave[group][scan_ch][scan_step].attribute;

	ch_start = myPs->config.chPerBd * group;

	for(ch=ch_start; ch < ch_start + myPs->config.chInGroup[group]; ch++) {
		attr = myData->COB_opSave[group][ch][scan_step].attribute;

		myData->f_save_msg[toPs].val[idx].chData[ch].ch
			= (unsigned char)(ch + 1);
		myData->f_save_msg[toPs].val[idx].chData[ch].resultIndex
			= myData->COB_opSave[group][ch][scan_step].resultIndex;
		myData->f_save_msg[toPs].val[idx].chData[ch].state
			= (unsigned char)convert_ch_state_org_to_p2(
			(long)myData->COB_opSave[group][ch][scan_step].state,
			(long)myData->COB_opSave[group][ch][scan_step].stepType,
			(long)attr, (long)myData->COB_opSave[group][ch][scan_step].phase);
		myData->f_save_msg[toPs].val[idx].chData[ch].stepType
			= (unsigned char)convert_step_type(CONVERT_ORG_TO_P2,
			(long)myData->COB_opSave[group][ch][scan_step].stepType);
		myData->f_save_msg[toPs].val[idx].chData[ch].stepMode
			= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P2,
			(long)myData->COB_opSave[group][ch][scan_step].stepMode);

		myData->f_save_msg[toPs].val[idx].chData[ch].Vsens
			= myData->COB_opSave[group][ch][scan_step].Vsens;

		myData->f_save_msg[toPs].val[idx].chData[ch].Isens
			= myData->COB_opSave[group][ch][scan_step].Isens;
		myData->f_save_msg[toPs].val[idx].chData[ch].charge_AmpareHour
			= myData->COB_opSave[group][ch][scan_step].charge_AmpareHour;
		myData->f_save_msg[toPs].val[idx].chData[ch].discharge_AmpareHour
			= myData->COB_opSave[group][ch][scan_step].discharge_AmpareHour;
		myData->f_save_msg[toPs].val[idx].chData[ch].watt
			= myData->COB_opSave[group][ch][scan_step].watt;
		myData->f_save_msg[toPs].val[idx].chData[ch].charge_WattHour
			= myData->COB_opSave[group][ch][scan_step].charge_WattHour;
		myData->f_save_msg[toPs].val[idx].chData[ch].discharge_WattHour
			= myData->COB_opSave[group][ch][scan_step].discharge_WattHour;
		myData->f_save_msg[toPs].val[idx].chData[ch].avgI
			= myData->COB_opSave[group][ch][scan_step].meanI;

		myData->f_save_msg[toPs].val[idx].chData[ch].capacitance
			= myData->COB_opSave[group][ch][scan_step].capacitance;
		myData->f_save_msg[toPs].val[idx].chData[ch].z
			= myData->COB_opSave[group][ch][scan_step].z;

		myData->f_save_msg[toPs].val[idx].chData[ch].select
			= myData->COB_opSave[group][ch][scan_step].select;
		myData->f_save_msg[toPs].val[idx].chData[ch].code
			= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P2,
			(long)myData->COB_opSave[group][ch][scan_step].code);
			//= (unsigned char)myData->COB_opSave[group][ch][scan_step].code; //kjg_d
		myData->f_save_msg[toPs].val[idx].chData[ch].stepNo
			= myData->COB_opSave[group][ch][scan_step].client_stepNo;

		myData->f_save_msg[toPs].val[idx].chData[ch].runTime
			= myData->COB_opSave[group][ch][scan_step].runTime;
		myData->f_save_msg[toPs].val[idx].chData[ch].totalRunTime
			= myData->COB_opSave[group][ch][scan_step].totalRunTime;
		myData->f_save_msg[toPs].val[idx].chData[ch].cvTime
			= myData->COB_opSave[group][ch][scan_step].cvTime;
		myData->f_save_msg[toPs].val[idx].chData[ch].ccTime
			= myData->COB_opSave[group][ch][scan_step].ccTime;

		myData->f_save_msg[toPs].val[idx].chData[ch].reservedCmd
			= myData->COB_opSave[group][ch][scan_step].reservedCmd;

		myData->f_save_msg[toPs].val[idx].chData[ch].totalCycle
			= myData->cData[ch].misc.totalCycle;
		myData->f_save_msg[toPs].val[idx].chData[ch].elementCycle
			= myData->cData[ch].elementCycle.cycle_count;
		for(i=0; i < MAX_ACC_CYCLE; i++) {
			myData->f_save_msg[toPs].val[idx].chData[ch].accCycle[i]
				= myData->cData[ch].accCycle[i].cycle_count;
		}
		for(i=0; i < MAX_MULTI_CYCLE; i++) {
			myData->f_save_msg[toPs].val[idx].chData[ch].multiCycle[i]
				= myData->cData[ch].multiCycle[i].cycle_count;
		}

		myData->f_save_msg[toPs].val[idx].chData[ch].avgV
			= myData->COB_opSave[group][ch][scan_step].meanV;

		if(myData->COB_opSave[group][ch][scan_step].select
				== SAVE_FLAG_SAVING_END
			&& (myData->COB_opSave[group][ch][scan_step].stepType == STEP_CHARGE
			|| myData->COB_opSave[group][ch][scan_step].stepType
				== STEP_DISCHARGE
			|| myData->COB_opSave[group][ch][scan_step].stepType == STEP_Z
			|| myData->COB_opSave[group][ch][scan_step].stepType == STEP_OCV
			|| myData->COB_opSave[group][ch][scan_step].stepType
				== STEP_REST)) {
			idxStepNo = myData->COB_opSave[group][ch][scan_step].idxStepNo;
			item = myData->testCond[0].grade[idxStepNo].item;
			if(item == GRADE_ITEM_V) {
				val = myData->f_save_msg[toPs].val[idx].chData[ch].Vsens;
				myData->f_save_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_AMPARE_HOUR) {
				val = myData->f_save_msg[toPs].val[idx].chData[ch]
					.charge_AmpareHour
					+ (myData->f_save_msg[toPs].val[idx].chData[ch]
					.discharge_AmpareHour * (-1));
				myData->f_save_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_Z) {
				val = myData->f_save_msg[toPs].val[idx].chData[ch].z;
				myData->f_save_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_CAPACITANCE) {
				val = myData->f_save_msg[toPs].val[idx].chData[ch].capacitance;
				myData->f_save_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else {
				myData->f_save_msg[toPs].val[idx].chData[ch].grade = 0;
			}
		}
	}

	myData->f_save_msg[toPs].write_idx = idx;
	myData->f_save_msg[toPs].count++;*/
}

void send_save_realData_msg(int group)
{
#ifdef __COB__
	int toPs;

	toPs = SAVE_GROUP1_TO_REAL_DATASAVE + group;

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
			send_save_realData_msg_Formation_B(group, toPs);
			break;
		case F_SDI_5V_450A_200A_100A_10A:
			send_save_realData_msg_Formation_C(group, toPs);
			break;
		default:
			send_save_realData_msg_Formation_A(group, toPs);
			break;
	}
#endif
}

void send_save_realData_msg_Formation_A(int group, int toPs)
{
#ifdef __COB__
	int idx, ch, i, j, attr, ch_start;
	unsigned char item, idxStepNo, send_flag, save_flag;
	long val;

	if(myData->save_real_data_msg[toPs].send_flag == 1) { //send f_save_msg stop
		return;
	}

	idx = myData->save_real_data_msg[toPs].write_idx;
	idx++;
	if(idx >= MAX_F_SAVE_MSG) idx = 0;

	attr = (int)myData->gData[group].attribute;
	if((attr >= ATTR_CHECK1_START && attr <= ATTR_CHECK1_END)
		|| (attr >= ATTR_CHECK2_START && attr <= ATTR_CHECK2_END)
		|| (attr >= ATTR_CHECK3_START && attr <= ATTR_CHECK3_END)) return;

	myData->save_real_data_msg[toPs].val[idx].state
		= myData->gData[group].state;
	myData->save_real_data_msg[toPs].val[idx].type
		= myData->gData[group].stepType;
	myData->save_real_data_msg[toPs].val[idx].mode
		= myData->gData[group].stepMode;
	myData->save_real_data_msg[toPs].val[idx].attribute = (unsigned char)attr;

	ch_start = myData->gData[group].misc.chOffset;
	save_flag = P0;
	for(i=ch_start; i < ch_start + myPs->config.chInGroup[group]; i++) {
		ch = i; //myData->mData.config.chPerBd * group + i;
		if(myData->cData[ch].op.save_flag != P0) {
			if((myData->cData[ch].op.state == C_RUN &&
				myData->cData[ch].op.phase == P50)
				|| (myData->cData[ch].op.state == C_RUN
				&& myData->cData[ch].op.phase == P102)) {
				save_flag = P1;
			}
		}
	}
	if(save_flag == P0) return;

	send_flag = P0;
	for(i=ch_start; i < ch_start + myPs->config.chInGroup[group]; i++) {
		ch = i; //myData->mData.config.chPerBd * group + i;
		if(myData->cData[ch].op.save_flag == P0) {
			attr = (int)myData->cData[ch].op.attribute;
			if((attr >= ATTR_CHECK1_START && attr <= ATTR_CHECK1_END)
				|| (attr >= ATTR_CHECK2_START && attr <= ATTR_CHECK2_END)
				|| (attr >= ATTR_CHECK3_START && attr <= ATTR_CHECK3_END)) {
				myData->save_real_data_msg[toPs].val[idx].chData[i].save_flag
					= P0;
				continue;
			}

			if((myData->cData[ch].op.state == C_RUN
				&& myData->cData[ch].op.phase == P50)
				|| (myData->cData[ch].op.state == C_RUN
				&& myData->cData[ch].op.phase == P102)) {
				myData->save_real_data_msg[toPs].val[idx].chData[i].save_flag
					= P1;
			} else {
				myData->save_real_data_msg[toPs].val[idx].chData[i].save_flag
					= P0;
				continue;
			}

			myData->save_real_data_msg[toPs].val[idx].chData[i].ch
				= (unsigned char)(i + 1);

			myData->save_real_data_msg[toPs].val[idx].chData[i].resultIndex
				= myData->cData[ch].op.resultIndex;
			myData->save_real_data_msg[toPs].val[idx].chData[i].state
				= (unsigned char)convert_ch_state_org_to_p2(
				(long)myData->cData[ch].op.state,
				(long)myData->cData[ch].op.stepType,
				(long)attr, (long)myData->cData[ch].op.phase);
			myData->save_real_data_msg[toPs].val[idx].chData[i].stepType
				= (unsigned char)convert_step_type(CONVERT_ORG_TO_P2,
				(long)myData->cData[ch].op.stepType);
			myData->save_real_data_msg[toPs].val[idx].chData[i].stepMode
				= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P2,
				(long)myData->cData[ch].op.stepMode);
	
			myData->save_real_data_msg[toPs].val[idx].chData[i].Vsens
				= myData->cData[ch].op.Vsens;

			myData->save_real_data_msg[toPs].val[idx].chData[i].Isens
				= myData->cData[ch].op.Isens;
			myData->save_real_data_msg[toPs].val[idx].chData[i]
				.charge_AmpareHour
				= myData->cData[ch].op.charge_AmpareHour;
			myData->save_real_data_msg[toPs].val[idx].chData[i]
				.discharge_AmpareHour
				= myData->cData[ch].op.discharge_AmpareHour;
			myData->save_real_data_msg[toPs].val[idx].chData[i].watt
				= myData->cData[ch].op.watt;
			myData->save_real_data_msg[toPs].val[idx].chData[i].charge_WattHour
				= myData->cData[ch].op.charge_WattHour;
			myData->save_real_data_msg[toPs].val[idx].chData[i]
				.discharge_WattHour
				= myData->cData[ch].op.discharge_WattHour;
			myData->save_real_data_msg[toPs].val[idx].chData[i].avgI
				= myData->cData[ch].op.meanI;

			myData->save_real_data_msg[toPs].val[idx].chData[i].capacitance
				= myData->cData[ch].op.capacitance;
			myData->save_real_data_msg[toPs].val[idx].chData[i].z 
				= myData->cData[ch].op.z;

			myData->save_real_data_msg[toPs].val[idx].chData[i].select
				= myData->cData[ch].op.select;
			myData->save_real_data_msg[toPs].val[idx].chData[i].code
				= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P2,
				(long)myData->cData[ch].op.code);
			myData->save_real_data_msg[toPs].val[idx].chData[i].stepNo
				= myData->cData[ch].op.client_stepNo;

			myData->save_real_data_msg[toPs].val[idx].chData[i].runTime
				= myData->cData[ch].op.runTime;
			myData->save_real_data_msg[toPs].val[idx].chData[i].totalRunTime
				= myData->cData[ch].op.totalRunTime;
			myData->save_real_data_msg[toPs].val[idx].chData[i].cvTime
				= myData->cData[ch].op.cvTime;
			myData->save_real_data_msg[toPs].val[idx].chData[i].ccTime
				= myData->cData[ch].op.ccTime;

			myData->save_real_data_msg[toPs].val[idx].chData[i].reservedCmd
				= myData->cData[ch].op.reservedCmd;

			myData->save_real_data_msg[toPs].val[idx].chData[i].totalCycle
				= myData->cData[ch].misc.totalCycle;
			myData->save_real_data_msg[toPs].val[idx].chData[i].elementCycle
				= myData->cData[ch].elementCycle.cycle_count;
			for(j=0; j < MAX_ACC_CYCLE; j++) {
				myData->save_real_data_msg[toPs].val[idx].chData[i].accCycle[i]
					= myData->cData[ch].accCycle[j].cycle_count;
			}
			for(j=0; j < MAX_MULTI_CYCLE; j++) {
				myData->save_real_data_msg[toPs].val[idx].chData[i]
					.multiCycle[i]
					= myData->cData[ch].multiCycle[j].cycle_count;
			}

			myData->save_real_data_msg[toPs].val[idx].chData[i].avgV
				= myData->cData[ch].op.meanV;

			myData->save_real_data_msg[toPs].val[idx].chData[i].temp
				= myData->cData[ch].op.temp;

			if(myData->cData[ch].op.select == SAVE_FLAG_SAVING_END
				&& (myData->cData[ch].op.stepType == STEP_CHARGE
				|| myData->cData[ch].op.stepType == STEP_DISCHARGE
				|| myData->cData[ch].op.stepType == STEP_Z
				|| myData->cData[ch].op.stepType == STEP_OCV
				|| myData->cData[ch].op.stepType == STEP_REST)) {
				idxStepNo = myData->cData[ch].op.idxStepNo;
				item = myData->testCond[ch_start].grade[idxStepNo].item;
				if(item == GRADE_ITEM_V) {
					val = myData->save_real_data_msg[toPs].val[idx].chData[i]
						.Vsens;
					myData->save_real_data_msg[toPs].val[idx].chData[i].grade
						= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
				} else if(item == GRADE_ITEM_AMPARE_HOUR) {
					val = myData->save_real_data_msg[toPs].val[idx].chData[i]
						.charge_AmpareHour
						+ (myData->save_real_data_msg[toPs].val[idx].chData[i]
						.discharge_AmpareHour * (-1));
					myData->save_real_data_msg[toPs].val[idx].chData[i].grade
						= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
				} else if(item == GRADE_ITEM_Z) {
					val = myData->save_real_data_msg[toPs].val[idx].chData[i].z;
					myData->save_real_data_msg[toPs].val[idx].chData[i].grade
						= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
				} else if(item == GRADE_ITEM_CAPACITANCE) {
					val = myData->save_real_data_msg[toPs].val[idx].chData[i]
						.capacitance;
					myData->save_real_data_msg[toPs].val[idx].chData[i].grade
						= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
				} else {
					myData->save_real_data_msg[toPs].val[idx].chData[i].grade
						= 0;
				}
			}
		} else {
			myData->cData[ch].op.save_flag = P0;
			attr = (int)myData->cData[ch].opSave.attribute;
			if((attr >= ATTR_CHECK1_START && attr <= ATTR_CHECK1_END)
				|| (attr >= ATTR_CHECK2_START && attr <= ATTR_CHECK2_END)
				|| (attr >= ATTR_CHECK3_START && attr <= ATTR_CHECK3_END)) {
				myData->save_real_data_msg[toPs].val[idx].chData[i].save_flag
					= P0;
				continue;
			}

			if((myData->cData[ch].opSave.state == C_RUN
				&& myData->cData[ch].opSave.phase == P50)
				|| (myData->cData[ch].opSave.state == C_RUN
				&& myData->cData[ch].opSave.phase == P102)) {
				send_flag = P1;
				myData->save_real_data_msg[toPs].val[idx].chData[i].save_flag
					= P1;
			} else {
				myData->save_real_data_msg[toPs].val[idx].chData[i].save_flag
					= P0;
				continue;
			}

			myData->save_real_data_msg[toPs].val[idx].chData[i].ch
				= (unsigned char)(i + 1);

			myData->save_real_data_msg[toPs].val[idx].chData[i].resultIndex
				= myData->cData[ch].opSave.resultIndex;
			myData->save_real_data_msg[toPs].val[idx].chData[i].state
				= (unsigned char)convert_ch_state_org_to_p2(
				(long)myData->cData[ch].opSave.state,
				(long)myData->cData[ch].opSave.stepType,
				(long)attr, (long)myData->cData[ch].opSave.phase);
			myData->save_real_data_msg[toPs].val[idx].chData[i].stepType
				= (unsigned char)convert_step_type(CONVERT_ORG_TO_P2,
				(long)myData->cData[ch].opSave.stepType);
			myData->save_real_data_msg[toPs].val[idx].chData[i].stepMode
				= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P2,
				(long)myData->cData[ch].opSave.stepMode);
	
			myData->save_real_data_msg[toPs].val[idx].chData[i].Vsens
				= myData->cData[ch].opSave.Vsens;

			myData->save_real_data_msg[toPs].val[idx].chData[i].Isens
				= myData->cData[ch].opSave.Isens;
			myData->save_real_data_msg[toPs].val[idx].chData[i]
				.charge_AmpareHour
				= myData->cData[ch].opSave.charge_AmpareHour;
			myData->save_real_data_msg[toPs].val[idx].chData[i]
				.discharge_AmpareHour
				= myData->cData[ch].opSave.discharge_AmpareHour;
			myData->save_real_data_msg[toPs].val[idx].chData[i].watt
				= myData->cData[ch].opSave.watt;
			myData->save_real_data_msg[toPs].val[idx].chData[i].charge_WattHour
				= myData->cData[ch].opSave.charge_WattHour;
			myData->save_real_data_msg[toPs].val[idx].chData[i]
				.discharge_WattHour
				= myData->cData[ch].opSave.discharge_WattHour;
			myData->save_real_data_msg[toPs].val[idx].chData[i].avgI
				= myData->cData[ch].opSave.meanI;

			myData->save_real_data_msg[toPs].val[idx].chData[i].capacitance
				= myData->cData[ch].opSave.capacitance;
			myData->save_real_data_msg[toPs].val[idx].chData[i].z 
				= myData->cData[ch].opSave.z;

			myData->save_real_data_msg[toPs].val[idx].chData[i].select
				= myData->cData[ch].opSave.select;
			myData->save_real_data_msg[toPs].val[idx].chData[i].code
				= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P2,
				(long)myData->cData[ch].opSave.code);
			myData->save_real_data_msg[toPs].val[idx].chData[i].stepNo
				= myData->cData[ch].opSave.client_stepNo;

			myData->save_real_data_msg[toPs].val[idx].chData[i].runTime
				= myData->cData[ch].opSave.runTime;
			myData->save_real_data_msg[toPs].val[idx].chData[i].totalRunTime
				= myData->cData[ch].opSave.totalRunTime;
			myData->save_real_data_msg[toPs].val[idx].chData[i].cvTime
				= myData->cData[ch].opSave.cvTime;
			myData->save_real_data_msg[toPs].val[idx].chData[i].ccTime
				= myData->cData[ch].opSave.ccTime;

			myData->save_real_data_msg[toPs].val[idx].chData[i].reservedCmd
				= myData->cData[ch].opSave.reservedCmd;

			myData->save_real_data_msg[toPs].val[idx].chData[i].totalCycle
				= myData->cData[ch].misc.totalCycle;
			myData->save_real_data_msg[toPs].val[idx].chData[i].elementCycle
				= myData->cData[ch].elementCycle.cycle_count;
			for(j=0; j < MAX_ACC_CYCLE; j++) {
				myData->save_real_data_msg[toPs].val[idx].chData[i].accCycle[i]
					= myData->cData[ch].accCycle[j].cycle_count;
			}
			for(j=0; j < MAX_MULTI_CYCLE; j++) {
				myData->save_real_data_msg[toPs].val[idx].chData[i]
					.multiCycle[i]
					= myData->cData[ch].multiCycle[j].cycle_count;
			}

			myData->save_real_data_msg[toPs].val[idx].chData[i].avgV
				= myData->cData[ch].opSave.meanV;

			myData->save_real_data_msg[toPs].val[idx].chData[i].temp
				= myData->cData[ch].opSave.temp;

			if(myData->cData[ch].opSave.select == SAVE_FLAG_SAVING_END
				&& (myData->cData[ch].opSave.stepType == STEP_CHARGE
				|| myData->cData[ch].opSave.stepType == STEP_DISCHARGE
				|| myData->cData[ch].opSave.stepType == STEP_Z
				|| myData->cData[ch].opSave.stepType == STEP_OCV
				|| myData->cData[ch].opSave.stepType == STEP_REST)) {
				idxStepNo = myData->cData[ch].opSave.idxStepNo;
				item = myData->testCond[ch_start].grade[idxStepNo].item;
				if(item == GRADE_ITEM_V) {
					val = myData->save_real_data_msg[toPs].val[idx].chData[i]
						.Vsens;
					myData->save_real_data_msg[toPs].val[idx].chData[i].grade
						= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
				} else if(item == GRADE_ITEM_AMPARE_HOUR) {
					val = myData->save_real_data_msg[toPs].val[idx].chData[i]
						.charge_AmpareHour
						+ (myData->save_real_data_msg[toPs].val[idx].chData[i]
						.discharge_AmpareHour * (-1));
					myData->save_real_data_msg[toPs].val[idx].chData[i].grade
						= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
				} else if(item == GRADE_ITEM_Z) {
					val = myData->save_real_data_msg[toPs].val[idx].chData[i].z;
					myData->save_real_data_msg[toPs].val[idx].chData[i].grade
						= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
				} else if(item == GRADE_ITEM_CAPACITANCE) {
					val = myData->save_real_data_msg[toPs].val[idx].chData[i]
						.capacitance;
					myData->save_real_data_msg[toPs].val[idx].chData[i].grade
						= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
				} else {
					myData->save_real_data_msg[toPs].val[idx].chData[i].grade = 0;
				}
			}
		}
	}

	if(send_flag == P0) return;

	myData->save_real_data_msg[toPs].write_idx = idx;
	myData->save_real_data_msg[toPs].count++;
#endif
}

void send_save_realData_msg_Formation_B(int group, int toPs)
{ //debug_size_cob
/*	int idx, ch, attr, scan_ch, scan_step, i, j;
	unsigned char item, idxStepNo, send_flag;
	long val;

	if(myData->save_real_data_msg[toPs].send_flag == 1) { //send f_save_msg stop
		return;
	}

	idx = myData->save_real_data_msg[toPs].write_idx;
	idx++;
	if(idx >= MAX_F_SAVE_MSG) idx = 0;

	scan_ch = myData->gData[group].misc.scan_ch[0];
	scan_step = myData->gData[group].misc.scan_step[0];

	attr = (int)myData->COB_opSave[group][scan_ch][scan_step].attribute;
	if((attr >= ATTR_CHECK1_START && attr <= ATTR_CHECK1_END)
		|| (attr >= ATTR_CHECK2_START && attr <= ATTR_CHECK2_END)
		|| (attr >= ATTR_CHECK3_START && attr <= ATTR_CHECK3_END)) return;

	myData->save_real_data_msg[toPs].val[idx].state
		= myData->COB_opSave[group][scan_ch][scan_step].state;
	myData->save_real_data_msg[toPs].val[idx].type
		= myData->COB_opSave[group][scan_ch][scan_step].stepType;
	myData->save_real_data_msg[toPs].val[idx].mode
		= myData->COB_opSave[group][scan_ch][scan_step].stepMode;
	myData->save_real_data_msg[toPs].val[idx].attribute = (unsigned char)attr;

	send_flag = P0;
	for(i=0; i < myPs->config.chInGroup[group]; i++) {
		//ch = myPs->config.chPerBd * group + i;
		ch = i;

		if(scan_ch != i) {
			myData->save_real_data_msg[toPs].val[idx].chData[i].save_flag = P0;
			continue;
		}

		if(myData->cData[0].op.save_flag == P0) {
			myData->save_real_data_msg[toPs].val[idx].chData[i].save_flag = P0;
			continue;
		} else {
			send_flag = P1;
			myData->cData[0].op.save_flag = P0;
			myData->save_real_data_msg[toPs].val[idx].chData[i].save_flag = P1;
		}
				
		myData->save_real_data_msg[toPs].val[idx].chData[i].scan_ch
			= (unsigned char)scan_ch;
		myData->save_real_data_msg[toPs].val[idx].chData[i].ch
			= (unsigned char)(ch + 1);

		myData->save_real_data_msg[toPs].val[idx].chData[i].resultIndex
			= myData->COB_opSave[group][ch][scan_step].resultIndex;
		myData->save_real_data_msg[toPs].val[idx].chData[i].state
			= (unsigned char)convert_ch_state_org_to_p2(
			(long)myData->COB_opSave[group][ch][scan_step].state,
			(long)myData->COB_opSave[group][ch][scan_step].stepType,
			(long)attr, (long)myData->COB_opSave[group][ch][scan_step].phase);
		myData->save_real_data_msg[toPs].val[idx].chData[i].stepType
			= (unsigned char)convert_step_type(CONVERT_ORG_TO_P2,
			(long)myData->COB_opSave[group][ch][scan_step].stepType);
		myData->save_real_data_msg[toPs].val[idx].chData[i].stepMode
			= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P2,
			(long)myData->COB_opSave[group][ch][scan_step].stepMode);

		myData->save_real_data_msg[toPs].val[idx].chData[i].Vsens
			= myData->COB_opSave[group][ch][scan_step].Vsens;

		myData->save_real_data_msg[toPs].val[idx].chData[i].Isens
			= myData->COB_opSave[group][ch][scan_step].Isens;
		myData->save_real_data_msg[toPs].val[idx].chData[i].charge_AmpareHour
			= myData->COB_opSave[group][ch][scan_step].charge_AmpareHour;
		myData->save_real_data_msg[toPs].val[idx].chData[i]
			.discharge_AmpareHour
			= myData->COB_opSave[group][ch][scan_step].discharge_AmpareHour;
		myData->save_real_data_msg[toPs].val[idx].chData[i].watt
			= myData->COB_opSave[group][ch][scan_step].watt;
		myData->save_real_data_msg[toPs].val[idx].chData[i].charge_WattHour
			= myData->COB_opSave[group][ch][scan_step].charge_WattHour;
		myData->save_real_data_msg[toPs].val[idx].chData[i].discharge_WattHour
			= myData->COB_opSave[group][ch][scan_step].discharge_WattHour;
		myData->save_real_data_msg[toPs].val[idx].chData[i].avgI
			= myData->COB_opSave[group][ch][scan_step].meanI;

		myData->save_real_data_msg[toPs].val[idx].chData[i].capacitance
			= myData->COB_opSave[group][ch][scan_step].capacitance;
		myData->save_real_data_msg[toPs].val[idx].chData[i].z 
			= myData->COB_opSave[group][ch][scan_step].z;

		myData->save_real_data_msg[toPs].val[idx].chData[i].select
			= myData->COB_opSave[group][ch][scan_step].select;
		myData->save_real_data_msg[toPs].val[idx].chData[i].code
			= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P2,
			(long)myData->COB_opSave[group][ch][scan_step].code);
		myData->save_real_data_msg[toPs].val[idx].chData[i].stepNo
			= myData->COB_opSave[group][ch][scan_step].client_stepNo;

		myData->save_real_data_msg[toPs].val[idx].chData[i].runTime
			= myData->COB_opSave[group][ch][scan_step].runTime;
		myData->save_real_data_msg[toPs].val[idx].chData[i].totalRunTime
			= myData->COB_opSave[group][ch][scan_step].totalRunTime;
		myData->save_real_data_msg[toPs].val[idx].chData[i].cvTime
			= myData->COB_opSave[group][ch][scan_step].cvTime;
		myData->save_real_data_msg[toPs].val[idx].chData[i].ccTime
			= myData->COB_opSave[group][ch][scan_step].ccTime;

		myData->save_real_data_msg[toPs].val[idx].chData[i].reservedCmd
			= myData->COB_opSave[group][ch][scan_step].reservedCmd;

		myData->save_real_data_msg[toPs].val[idx].chData[i].totalCycle
			= myData->cData[ch].misc.totalCycle;
		myData->save_real_data_msg[toPs].val[idx].chData[i].elementCycle
			= myData->cData[ch].elementCycle.cycle_count;
		for(j=0; j < MAX_ACC_CYCLE; j++) {
			myData->save_real_data_msg[toPs].val[idx].chData[i].accCycle[i]
				= myData->cData[ch].accCycle[j].cycle_count;
		}
		for(j=0; j < MAX_MULTI_CYCLE; j++) {
			myData->save_real_data_msg[toPs].val[idx].chData[i].multiCycle[i]
				= myData->cData[ch].multiCycle[j].cycle_count;
		}

		myData->save_real_data_msg[toPs].val[idx].chData[i].avgV
			= myData->COB_opSave[group][ch][scan_step].meanV;

		myData->save_real_data_msg[toPs].val[idx].chData[i].temp
			= myData->COB_opSave[group][ch][scan_step].temp;

		if(myData->COB_opSave[group][ch][scan_step].select
				== SAVE_FLAG_SAVING_END
			&& (myData->COB_opSave[group][ch][scan_step].stepType == STEP_CHARGE
			|| myData->COB_opSave[group][ch][scan_step].stepType
				== STEP_DISCHARGE
			|| myData->COB_opSave[group][ch][scan_step].stepType == STEP_Z
			|| myData->COB_opSave[group][ch][scan_step].stepType == STEP_OCV
			|| myData->COB_opSave[group][ch][scan_step].stepType
				== STEP_REST)) {
			idxStepNo = myData->COB_opSave[group][ch][scan_step].idxStepNo;
			item = myData->testCond[0].grade[idxStepNo].item;
			if(item == GRADE_ITEM_V) {
				val = myData->save_real_data_msg[toPs].val[idx].chData[i].Vsens;
				myData->save_real_data_msg[toPs].val[idx].chData[i].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_AMPARE_HOUR) {
				val = myData->save_real_data_msg[toPs].val[idx].chData[i]
					.charge_AmpareHour
					+ (myData->save_real_data_msg[toPs].val[idx].chData[i]
					.discharge_AmpareHour * (-1));
				myData->save_real_data_msg[toPs].val[idx].chData[i].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_Z) {
				val = myData->save_real_data_msg[toPs].val[idx].chData[i].z;
				myData->save_real_data_msg[toPs].val[idx].chData[i].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_CAPACITANCE) {
				val = myData->save_real_data_msg[toPs].val[idx].chData[i].capacitance;
				myData->save_real_data_msg[toPs].val[idx].chData[i].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else {
				myData->save_real_data_msg[toPs].val[idx].chData[i].grade = 0;
			}
		}
	}

	if(send_flag == P0) return;

	myData->save_real_data_msg[toPs].write_idx = idx;
	myData->save_real_data_msg[toPs].count++;*/
}

void send_save_realData_msg_Formation_C(int group, int toPs)
{ //debug_size_cob
/*	int idx, ch, attr, scan_ch, scan_step, i, j;
	unsigned char item, idxStepNo, send_flag, daq_div, save_flag;
	long val;

	if(myData->save_real_data_msg[toPs].send_flag == 1) { //send f_save_msg stop
		return;
	}

	idx = myData->save_real_data_msg[toPs].write_idx;
	idx++;
	if(idx >= MAX_F_SAVE_MSG) idx = 0;

	send_flag = P0;
	for(i=0; i < myPs->config.chInGroup[group]; i++) {
		ch = i;

		if(ch < 12) {
			scan_ch = myData->gData[group].misc.scan_ch[0];
			scan_step = myData->gData[group].misc.scan_step[0];
			attr = (int)myData->COB_opSave[group][scan_ch][scan_step].attribute;
			save_flag = myData->cData[0].op.save_flag;
		} else {
			scan_ch = myData->gData[group].misc.scan_ch[1] + 12;
			scan_step = myData->gData[group].misc.scan_step[1];
			attr = (int)myData->COB_opSave[group][scan_ch][scan_step].attribute;
			save_flag = myData->cData[1].op.save_flag;
		}

		if((attr >= ATTR_CHECK1_START && attr <= ATTR_CHECK1_END)
			|| (attr >= ATTR_CHECK2_START && attr <= ATTR_CHECK2_END)
			|| (attr >= ATTR_CHECK3_START && attr <= ATTR_CHECK3_END)) {
			if(ch < 12) {
				myData->cData[0].op.save_flag = P0;
			} else {
				myData->cData[1].op.save_flag = P0;
			}
			myData->save_real_data_msg[toPs].val[idx].chData[ch].save_flag = P0;
			continue;
		}

		daq_div = 0; //normal Vsens

		if(scan_ch == ch) {
			if(save_flag == P0) {
				myData->save_real_data_msg[toPs].val[idx].chData[ch].save_flag
					= P0;
				daq_div = 1; //daq Vsens
			} else {
				send_flag = P1;
				myData->save_real_data_msg[toPs].val[idx].chData[ch].save_flag
					= P1;
				if(ch < 12) {
					myData->cData[0].op.save_flag = P0;
				} else {
					myData->cData[1].op.save_flag = P0;
				}
			}
		} else {
			myData->save_real_data_msg[toPs].val[idx].chData[ch].save_flag = P0;
			daq_div = 1; //daq Vsens
		}

		myData->save_real_data_msg[toPs].val[idx].chData[ch].ch
			= (unsigned char)(ch + 1);

		myData->save_real_data_msg[toPs].val[idx].chData[ch].resultIndex
			= myData->COB_opSave[group][ch][scan_step].resultIndex;
		myData->save_real_data_msg[toPs].val[idx].chData[ch].state
			= (unsigned char)convert_ch_state_org_to_p2(
			(long)myData->COB_opSave[group][ch][scan_step].state,
			(long)myData->COB_opSave[group][ch][scan_step].stepType,
			(long)attr, (long)myData->COB_opSave[group][ch][scan_step].phase);
		myData->save_real_data_msg[toPs].val[idx].chData[ch].stepType
			= (unsigned char)convert_step_type(CONVERT_ORG_TO_P2,
			(long)myData->COB_opSave[group][ch][scan_step].stepType);
		myData->save_real_data_msg[toPs].val[idx].chData[ch].stepMode
			= (unsigned char)convert_step_mode(CONVERT_ORG_TO_P2,
			(long)myData->COB_opSave[group][ch][scan_step].stepMode);

		if(daq_div == 0) { //normal Vsens
			myData->save_real_data_msg[toPs].val[idx].chData[ch].Vsens
				= myData->COB_opSave[group][ch][scan_step].Vsens;
		} else { //daq Vsens
			if(ch < 12) {
				myData->save_real_data_msg[toPs].val[idx].chData[ch].Vsens
					= myData->SubSensV.ch[ch].sensV;
			} else {
				myData->save_real_data_msg[toPs].val[idx].chData[ch].Vsens
					= myData->SubSensV.ch[ch+20].sensV;
			}
		}

		myData->save_real_data_msg[toPs].val[idx].chData[ch].Isens
			= myData->COB_opSave[group][ch][scan_step].Isens;
		myData->save_real_data_msg[toPs].val[idx].chData[ch].charge_AmpareHour
			= myData->COB_opSave[group][ch][scan_step].charge_AmpareHour;
		myData->save_real_data_msg[toPs].val[idx].chData[ch]
			.discharge_AmpareHour
			= myData->COB_opSave[group][ch][scan_step].discharge_AmpareHour;
		myData->save_real_data_msg[toPs].val[idx].chData[ch].watt
			= myData->COB_opSave[group][ch][scan_step].watt;
		myData->save_real_data_msg[toPs].val[idx].chData[ch].charge_WattHour
			= myData->COB_opSave[group][ch][scan_step].charge_WattHour;
		myData->save_real_data_msg[toPs].val[idx].chData[ch].discharge_WattHour
			= myData->COB_opSave[group][ch][scan_step].discharge_WattHour;
		myData->save_real_data_msg[toPs].val[idx].chData[ch].avgI
			= myData->COB_opSave[group][ch][scan_step].meanI;

		myData->save_real_data_msg[toPs].val[idx].chData[ch].capacitance
			= myData->COB_opSave[group][ch][scan_step].capacitance;
		myData->save_real_data_msg[toPs].val[idx].chData[ch].z 
			= myData->COB_opSave[group][ch][scan_step].z;

		myData->save_real_data_msg[toPs].val[idx].chData[ch].select
			= myData->COB_opSave[group][ch][scan_step].select;
		myData->save_real_data_msg[toPs].val[idx].chData[ch].code
			= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P2,
			(long)myData->COB_opSave[group][ch][scan_step].code);
		myData->save_real_data_msg[toPs].val[idx].chData[ch].stepNo
			= myData->COB_opSave[group][ch][scan_step].client_stepNo;

		myData->save_real_data_msg[toPs].val[idx].chData[ch].runTime
			= myData->COB_opSave[group][ch][scan_step].runTime;
		myData->save_real_data_msg[toPs].val[idx].chData[ch].totalRunTime
			= myData->COB_opSave[group][ch][scan_step].totalRunTime;
		myData->save_real_data_msg[toPs].val[idx].chData[ch].cvTime
			= myData->COB_opSave[group][ch][scan_step].cvTime;
		myData->save_real_data_msg[toPs].val[idx].chData[ch].ccTime
			= myData->COB_opSave[group][ch][scan_step].ccTime;

		myData->save_real_data_msg[toPs].val[idx].chData[ch].reservedCmd
			= myData->COB_opSave[group][ch][scan_step].reservedCmd;

		myData->save_real_data_msg[toPs].val[idx].chData[ch].totalCycle
			= myData->cData[ch].misc.totalCycle;
		myData->save_real_data_msg[toPs].val[idx].chData[ch].elementCycle
			= myData->cData[ch].elementCycle.cycle_count;
		for(j=0; j < MAX_ACC_CYCLE; j++) {
			myData->save_real_data_msg[toPs].val[idx].chData[ch].accCycle[j]
				= myData->cData[ch].accCycle[j].cycle_count;
		}
		for(j=0; j < MAX_MULTI_CYCLE; j++) {
			myData->save_real_data_msg[toPs].val[idx].chData[ch].multiCycle[j]
				= myData->cData[ch].multiCycle[j].cycle_count;
		}

		myData->save_real_data_msg[toPs].val[idx].chData[ch].avgV
			= myData->COB_opSave[group][ch][scan_step].meanV;

		myData->save_real_data_msg[toPs].val[idx].chData[ch].temp
			= myData->COB_opSave[group][ch][scan_step].temp;

		if(myData->COB_opSave[group][ch][scan_step].select
				== SAVE_FLAG_SAVING_END
			&& (myData->COB_opSave[group][ch][scan_step].stepType == STEP_CHARGE
			|| myData->COB_opSave[group][ch][scan_step].stepType
				== STEP_DISCHARGE
			|| myData->COB_opSave[group][ch][scan_step].stepType == STEP_Z
			|| myData->COB_opSave[group][ch][scan_step].stepType == STEP_OCV
			|| myData->COB_opSave[group][ch][scan_step].stepType
				== STEP_REST)) {
			idxStepNo = myData->COB_opSave[group][ch][scan_step].idxStepNo;
			item = myData->testCond[0].grade[idxStepNo].item; //kjg_w
			if(item == GRADE_ITEM_V) {
				val = myData->save_real_data_msg[toPs].val[idx].chData[ch]
					.Vsens;
				myData->save_real_data_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_AMPARE_HOUR) {
				val = myData->save_real_data_msg[toPs].val[idx].chData[ch]
					.charge_AmpareHour
					+ (myData->save_real_data_msg[toPs].val[idx].chData[ch]
					.discharge_AmpareHour * (-1));
				myData->save_real_data_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_Z) {
				val = myData->save_real_data_msg[toPs].val[idx].chData[ch].z;
				myData->save_real_data_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else if(item == GRADE_ITEM_CAPACITANCE) {
				val = myData->save_real_data_msg[toPs].val[idx].chData[ch]
					.capacitance;
				myData->save_real_data_msg[toPs].val[idx].chData[ch].grade
					= (unsigned char)GradeCodeCheck(ch, idxStepNo, val);
			} else {
				myData->save_real_data_msg[toPs].val[idx].chData[ch].grade = 0;
			}
		}
	}

	if(send_flag == P0) return;

	myData->save_real_data_msg[toPs].write_idx = idx;
	myData->save_real_data_msg[toPs].count++;*/
}
