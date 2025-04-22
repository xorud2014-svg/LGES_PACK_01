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
//#include "rt_can_ext.h"	//kjg_181121
#ifdef __CAN_FD__		//csk_190617s
#include "rt_can_ext.h"	//kjg_181121
#else	//can2.0
#include "../rt_can/rt_can.h"	//kjg_181121
#endif					//csk_190617e
#include "../rt_com/rt_com.h"
//#include "CAN.h"		//csk_240301
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
			for(i=0; i < j; i++) {	//ktg_231031s
				if(msgParsing(UPDATE_TO_IO) < 0) break;
			}	//ktg_231031e
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
		case UPDATE_TO_IO:	//ktg_231031
			msgParsing_Update_to_IO(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
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
	int manual_can_bitconfig, nominal_brp, nominal_tseg1, nominal_tseg2, nominal_sjw; //ksh_241112
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

			//ksh_241112_s
			manual_can_bitconfig = RecvMsg->val[8];
			nominal_brp = RecvMsg->val[9];
			nominal_tseg1 = RecvMsg->val[10];
			nominal_tseg2 = RecvMsg->val[11];
			nominal_sjw = RecvMsg->val[12];
			//ksh_241112_e
#ifdef __CAN_FD__ //kjh_190706s
			i = RecvMsg->val[2]; //sjw
			j = RecvMsg->val[3]; //ext
			k = RecvMsg->val[4]; //fd_flag			
			/*kjg_240213 rtl_printf("can param1 ch:%d baud:%d sjw:%d ext:%d fd:%d\n",
				ch, val, j, i, k);
			rtl_printf("can param1 data_rate:%d crc:%d terminal_r:%d\n",
				RecvMsg->val[5], RecvMsg->val[6], RecvMsg->val[7]);*/
			rtl_printf("can ch:%d baud:%d sjw:%d ext:%d fd:%d d_rate:%d crc:%d terminal_r:%d\n",
				ch, val, j, i, k,
				RecvMsg->val[5], RecvMsg->val[6], RecvMsg->val[7]);
			rtn = rt_can_set_param_0(ch, val, j, i, k, RecvMsg->val[5], RecvMsg->val[6], RecvMsg->val[7]);			

			myData->CAN.signal[ch][CAN_SIG_ABORT_TRANSMISSION] = 2; //kjg_180523
#else //CAN_2p0b
			//rtn = rt_can_set_param(ch, val, RecvMsg->val[3], RecvMsg->val[2], 1);
			rtn = rt_can_set_param(ch, val, RecvMsg->val[3], RecvMsg->val[2], 1, 
			manual_can_bitconfig, nominal_brp, nominal_tseg1, nominal_tseg2, nominal_sjw); //ksh_241112
			if(rtn < 0) {
				rtl_printf("kjg_d_can error1 %d, ch:%d val:%d sjw:%d ext:%d\n",
					rtn, ch, val, RecvMsg->val[3], RecvMsg->val[2]);
				rtl_printf("kjg_d_can set_param %d, ch:%d manual_can_bitconfig:%d norminal_brp:%d  norminal_tseg1:%d\n norminal_tseg2:%d nominal_sjw:%d\n\n",
					rtn, ch, manual_can_bitconfig, nominal_brp, nominal_tseg1, nominal_tseg2, nominal_sjw); //ksh_241112
			} else { //ksh_241112
				rtl_printf("kjg_d_can set_param %d, ch:%d manual_can_bitconfig:%d sjw:%d ext:%d\n",
					rtn, ch, val, RecvMsg->val[3], RecvMsg->val[2]);
				rtl_printf("kjg_d_can set_param %d, ch:%d manual_can_bitconfig:%d nominal_brp:%d  nominal_tseg1:%d\n nominal_tseg2:%d\n nominal_sjw:%d\n",
					rtn, ch, manual_can_bitconfig, nominal_brp, nominal_tseg1, nominal_tseg2, nominal_sjw); //ksh_241112
			}
#endif //kjh_190706e
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

			//jhkw_181119s
			if(myData->cData[ch].op.state != C_RUN) {
				if(myData->canTransmitSetData.commonData[ch][0]
					.can_lin_flag == 1 ||
					myData->canReceiveSetData.commonData[ch][0]
					.can_lin_flag == 1) { //lin to can
					Select_OutPoint(0, ch+1, O_CAN_OR_LINTOCAN, ON);
				} else { //can
					Select_OutPoint(0, ch+1, O_CAN_OR_LINTOCAN, OFF);
				}

				//csk_190718s
				if(myData->canTransmitSetData.commonData[ch][0]
					.can_lin_flag == 1 ||
					myData->canReceiveSetData.commonData[ch][0]
					.can_lin_flag == 1) { //lin to can
					Select_OutPoint(0, ch+1, O_CAN_SW, OFF);
					Select_OutPoint(0, ch+1, O_LIN_TO_CAN_SW, ON);
					Select_OutPoint(0, ch+1, O_485_TO_CAN_SW, OFF);
					Select_OutPoint(0, ch+1, O_SMBUS_TO_CAN_SW, OFF);	//csk_190910
					Select_OutPoint(0, ch+1, O_MODBUS_TO_CAN_SW, OFF);	//ktg_211001
					Select_OutPoint(0, ch+1, O_CAN_SLAVE_SW, ON);
				} else if(myData->canTransmitSetData.commonData[ch][0]
					.can_lin_flag == 2 ||
					myData->canReceiveSetData.commonData[ch][0]
					.can_lin_flag == 2) { //lin to can
					Select_OutPoint(0, ch+1, O_CAN_SW, OFF);
					Select_OutPoint(0, ch+1, O_LIN_TO_CAN_SW, OFF);
					Select_OutPoint(0, ch+1, O_485_TO_CAN_SW, ON);
					Select_OutPoint(0, ch+1, O_SMBUS_TO_CAN_SW, OFF);	//csk_190910s
					Select_OutPoint(0, ch+1, O_MODBUS_TO_CAN_SW, OFF);	//ktg_211001
					Select_OutPoint(0, ch+1, O_CAN_SLAVE_SW, ON);
				} else if(myData->canTransmitSetData.commonData[ch][0]
					.can_lin_flag == 3 ||
					myData->canReceiveSetData.commonData[ch][0]
					.can_lin_flag == 3) { //smbus to can
					Select_OutPoint(0, ch+1, O_CAN_SW, OFF);
					Select_OutPoint(0, ch+1, O_LIN_TO_CAN_SW, OFF);
					Select_OutPoint(0, ch+1, O_485_TO_CAN_SW, OFF);
					Select_OutPoint(0, ch+1, O_SMBUS_TO_CAN_SW, ON);	//csk_190910e
					Select_OutPoint(0, ch+1, O_MODBUS_TO_CAN_SW, OFF);	//ktg_211001
					Select_OutPoint(0, ch+1, O_CAN_SLAVE_SW, ON);
				} else if(myData->canTransmitSetData.commonData[ch][0]
					.can_lin_flag == 4 ||
					myData->canReceiveSetData.commonData[ch][0]
					.can_lin_flag == 4) { //master 485 to can  slave - modbus to can
					switch(myData->AppControl.config.systemModel) {
						//master - can  slave - modbus to can
						case C_LGC_250V_350A_100A_175KW:	//D10207
						case C_LGC_200V_150A_50A_60KW:	//D10196
							Select_OutPoint(0, ch+1, O_CAN_SW, ON);
							Select_OutPoint(0, ch+1, O_LIN_TO_CAN_SW, OFF);
							Select_OutPoint(0, ch+1, O_485_TO_CAN_SW, OFF);
							Select_OutPoint(0, ch+1, O_SMBUS_TO_CAN_SW, OFF);
							Select_OutPoint(0, ch+1, O_MODBUS_TO_CAN_SW, ON);
							Select_OutPoint(0, ch+1, O_CAN_SLAVE_SW, OFF);
							break;
						default: //master 485 to can  slave - modbus to can
							Select_OutPoint(0, ch+1, O_CAN_SW, OFF);
							Select_OutPoint(0, ch+1, O_LIN_TO_CAN_SW, OFF);
							Select_OutPoint(0, ch+1, O_485_TO_CAN_SW, ON);
							Select_OutPoint(0, ch+1, O_SMBUS_TO_CAN_SW, OFF);
							Select_OutPoint(0, ch+1, O_MODBUS_TO_CAN_SW, ON);
							Select_OutPoint(0, ch+1, O_CAN_SLAVE_SW, OFF);
							break;
					}
				} else { //can
					Select_OutPoint(0, ch+1, O_CAN_SW, ON);
					Select_OutPoint(0, ch+1, O_LIN_TO_CAN_SW, OFF);
					Select_OutPoint(0, ch+1, O_485_TO_CAN_SW, OFF);
					Select_OutPoint(0, ch+1, O_SMBUS_TO_CAN_SW, OFF);	//csk_190910
					Select_OutPoint(0, ch+1, O_MODBUS_TO_CAN_SW, OFF);	//ktg_211001
					Select_OutPoint(0, ch+1, O_CAN_SLAVE_SW, ON);
				}		//csk_190718e
			}	//jhkw_181119e
			switch(myData->AppControl.config.systemModel) { //kjg_141114
				case C_HLGP_500V_300A_100A_300KW:
				case C_HLGP_500V_300A_100A_300KW_2:
				case C_HLGP_500V_300A_100A_300KW_3:
				case C_HLGP_500V_300A_100A_300KW_4:
				case C_HLGP_500V_300A_100A_300KW_5:
				case C_HLGP_500V_300A_100A_300KW_6:	//jhk_141127
				case C_HLGP_500V_300A_100A_300KW_7:	//jhk_141127
				case C_HLGP_500V_300A_100A_300KW_8:	//jhk_141127
				case C_HLGP_500V_300A_100A_300KW_9:	//jhk_141127
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
				myData->CAN.signal[ch][CAN_SIG_BMS_TEST5] = P1;	//ktg_220614
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
					case C_KBIA_500V_250A_100A_250KW:	//shh_200916
					case C_JBTP_500V_250A_100A_500KW:	//shh_200927
					case C_LGC_500V_250A_100A_50A_500KW: //phb_220902
					case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
					case C_LGC_500V_250A_125A_50A_500KW_2: //ktg_190227
					case C_LGC_500V_250A_125A_50A_500KW_3: //ktg_190227
					case C_LGC_500V_250A_125A_50A_500KW_4: //ktg_190227
					case C_LGC_500V_250A_125A_50A_500KW_5: //ktg_190227
					case C_LGC_500V_250A_125A_50A_500KW_6: //ktg_190227
					case C_LGC_500V_250A_125A_50A_500KW_7: //ktg_190227
					case C_LGC_500V_250A_125A_50A_500KW_8: //ktg_190227
					case C_LGC_500V_250A_125A_50A_500KW_9: //ktg_190227
					case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_190615
					case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_190615
					case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
					case C_KCL_600V_200A_100A_480KW: //ktg_190812
					case C_KCL_600V_200A_100A_480KW_2: //ktg_190812
					case C_ADD_800V_350A_200A_50A_380KW:		//shh_200904
					case C_HYUNDAI_1000V_250A_100A_500KW:		//shh_200513
					case C_KOSTA_1000V_250A_100A_600KW:			//shh_201102
					case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
					case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
					case C_KTC_1500V_600A_200A_400KW:	//ktg_200807
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
					myData->CAN.signal[0][CAN_SIG_BMS_TEST5] = P1;	//ktg_220614
					myData->CAN.signal[0][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[1][CAN_SIG_BMS_TEST1] = P1;
					myData->CAN.signal[1][CAN_SIG_BMS_TEST2] = P1;
					myData->CAN.signal[1][CAN_SIG_BMS_TEST5] = P1;	//ktg_220614
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
					myData->CAN.signal[2][CAN_SIG_BMS_TEST5] = P1;	//ktg_220614
					myData->CAN.signal[2][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[3][CAN_SIG_BMS_TEST1] = P1;
					myData->CAN.signal[3][CAN_SIG_BMS_TEST2] = P1;
					myData->CAN.signal[3][CAN_SIG_BMS_TEST5] = P1;	//ktg_220614
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
					myData->CAN.signal[4][CAN_SIG_BMS_TEST5] = P1;	//ktg_220614
					myData->CAN.signal[4][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[5][CAN_SIG_BMS_TEST1] = P1;
					myData->CAN.signal[5][CAN_SIG_BMS_TEST2] = P1;
					myData->CAN.signal[5][CAN_SIG_BMS_TEST5] = P1;	//ktg_220614
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
					myData->CAN.signal[6][CAN_SIG_BMS_TEST5] = P1;	//ktg_220614
					myData->CAN.signal[6][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[7][CAN_SIG_BMS_TEST1] = P1;
					myData->CAN.signal[7][CAN_SIG_BMS_TEST2] = P1;
					myData->CAN.signal[7][CAN_SIG_BMS_TEST5] = P1;	//ktg_220614
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
#ifdef __CAN_FD__ //kjh_190706s
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
#endif //kjh_190706e
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
					myData->cData[ch].signal[C_SIG_CMD_PAUSE] = P1;
				}
				if(RecvMsg->val[2] == 0) {	//shhw_220707s
					myData->cData[ch].misc.wait_type = 0;
				} else if(RecvMsg->val[2] == 1) {
					myData->cData[ch].misc.wait_type = 1;
				} else {
					myData->cData[ch].misc.wait_type = 2;
				}							//shhw_220707e
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
				ch = RecvMsg->val[2];
			if(RecvMsg->val[1] == 1) { //iso
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION] = P1;
			} else {
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION] = P0;
			}

			if(RecvMsg->val[1] == 1) { //iso
				Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION, OFF);
			} else {
				Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION, ON);
			}

			myPs->signal[M_SIG_DAQ_ISOLATION_MAIN_CH] = ch;
			myPs->signal[M_SIG_DAQ_ISOLATION_REPLY_DELAY] = P1;
			break;
		case MSG_COA_MODULE_DAQ_ISOLATION_REQUEST2:	//jhkw_150224s
			ch = RecvMsg->val[3];
			if(RecvMsg->val[1] == 1) { //iso
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION] = P1;
			} else {
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION] = P0;
			}

			//if(myData->cData[0].op.state != C_RUN) {		//jhkw_190709
			if(myData->cData[ch-1].op.state != C_RUN) {
				if(RecvMsg->val[1] == 1) { //iso
					Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION2, OFF);
					Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION3, OFF);
					Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION4, OFF);
					Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION5, OFF);
					Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION6, OFF);
					myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P0;
				} else {
					if(RecvMsg->val[2] == 1) { //div_ch 1
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION2, ON);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION3,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION4,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION5,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION6,OFF);
						myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P1;
					} else if(RecvMsg->val[2] == 2){	//div_ch 2
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION2,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION3, ON);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION4,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION5,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION6,OFF);
						myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P2;
					} else if(RecvMsg->val[2] == 3){	//div_ch 3
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION2,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION3,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION4, ON);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION5,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION6,OFF);
						myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P3;
					} else if(RecvMsg->val[2] == 4){	//div_ch 4
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION2,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION3,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION4,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION5, ON);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION6,OFF);
						myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P4;
					} else if(RecvMsg->val[2] == 5){	//div_ch 5
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION2,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION3,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION4,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION5,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION6, ON);
						myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P5;
					} else {
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION2,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION3,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION4,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION5,OFF);
						Select_OutPoint(0, ch, O_OUT_PACK_ISOLATION6,OFF);
						myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P0;
					}
				}
			} else {
				myPs->signal[M_SIG_DAQ_ISOLATION_DIV_CH] = P10;
			}
			myPs->signal[M_SIG_DAQ_ISOLATION_MAIN_CH2] = ch;
			myPs->signal[M_SIG_DAQ_ISOLATION_REPLY_DELAY2] = P1;
			break;	//jhkw_150224e
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
		case MSG_COA_MODULE_CAN_OR_LINTOCAN_SET: //kjhw_181111
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				if(myData->cData[ch].op.state != C_RUN) {
					group = RecvMsg->val[0];
					if(RecvMsg->val[1] == 1) { //lin to can
						Select_OutPoint(group, ch+1, O_CAN_OR_LINTOCAN, ON);
					} else { //can
						Select_OutPoint(group, ch+1, O_CAN_OR_LINTOCAN, OFF);
					}
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
			myPs->signal[M_SIG_BUZZER_SET] = P2;
			break;
		case MSG_COA_MODULE_ALARM_RESET:
			myPs->signal[M_SIG_ALARM_SET] = P0;
			myPs->signal[M_SIG_BUZZER_SET] = P0;
			break;
		case MSG_COA_MODULE_CHAMBER_VENT_STOP: //kjhw_190820
			//ch = RecvMsg->val[1];
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;
				if(RecvMsg->val[1] == 1) {
					Select_OutPoint(0, ch+1, O_CHAMBER_CH_WARNING, ON);
					myData->cData[ch].op.code = C_CD_FAULT_VENT_OPEN_TEST;
				} else {
					Select_OutPoint(0, ch+1, O_CHAMBER_CH_WARNING, OFF);
				}
			}
			break;
		case MSG_COA_MODULE_CAP_BANK_SET: //kjh_191014
			//ch = RecvMsg->val[1];
			for(i=0; i < myPs->config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[k + i].number2 - 1;

				if(RecvMsg->val[1] == 1) {
					Select_OutPoint(0, ch+1, O_CAP_BANK, ON);
				} else {
					Select_OutPoint(0, ch+1, O_CAP_BANK, OFF);
				}
			}
			break;
		case MSG_COA_MODULE_GUI_EMG_SET:		//khj_201227
			if(RecvMsg->val[1] == 1) {
				myPs->signal[M_SIG_GUI_EMG_SET] = P1;
			} else {
				myPs->signal[M_SIG_GUI_EMG_SET] = P0;
			}
			break;
		case MSG_COA_MODULE_GUI_SHUTDOWN:		//ktg_250410	//jhj_250410
			if(RecvMsg->val[1] == 1) {
				myPs->signal[M_SIG_GUI_SHUTDOWN] = P1;
			} else {
				myPs->signal[M_SIG_GUI_SHUTDOWN] = P0;
			}
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

#endif
}

void msgParsing_COC_to_Module(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
#ifdef __COC__

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
			} else if(myPs->code == M_CD_FAULT_AC_POWER_SHORT
				|| myPs->code == M_CD_FAULT_CHAMBER
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

void msgParsing_Update_to_IO(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{	//ktg_231031
	S_MSG_VAL SendMsg;

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	switch(RecvMsg->msg) {
		case MSG_UPDATE_IO_EXIT:
			//myData->test_val_l1[0] = 11;
			SendMsg.msg = MSG_IO_MODULE_EXIT;
			SendMsg.val[0] = RecvMsg->val[0];
			SendMsg.val[1] = RecvMsg->val[1];
			send_msg(IO_TO_MODULE, (char *)&SendMsg);
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

	switch(myData->AppControl.config.systemModel) {
		case C_CANSYSTEM_60V_350A_100A_21KW: //kjg_140916
		case C_LGE_450V_250A_50A_112KW:		//jhk_150703
		//case C_LGE_450V_250A_50A_225KW:	//jhk_150605	//phb_220628
		//case C_LGE_450V_250A_50A_225KW_2:	//jhk_150605	//phb_220628
		//case C_LGE_450V_250A_50A_225KW_3:	//jhk_150605	//phb_220628
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

	myData->cData[ch].opSave = myData->cData[ch].op;

	if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COA][0] == P1) {
		if(myData->ChAttribute[ch].chNo_master == 0) return;

		myData->cData[ch].op.resultIndex++;
		send_save_msg_2(ch, 0); //COA1_Client
		send_save_msg_2(ch, 1); //DataSave
	}
}

void send_save_msg_2(int ch, int msg)
{
	//unsigned char item, idxStepNo;
	//int idx, group, i, j, k;
	//long val;
	//int idx, idx2, group, i, j, k;
	//long val, val2, refI, refI2, tmp_refI, tmp_i;
	//long val, val2, refI, refI2;
	unsigned char item; //kjh_211021 save_type;
	short int auxTH_offset; //jhkw_211014
	int idx, group, i, j, k, idxStepNo, sub_code_div;	//kjh_211021
	long val, /*//kjh_211021 val2, refI, tmp_refI,*/ v_val, i_val;
	//kjh_211021 long max_refI, min_refI, rangeI_val, tmp_rangeI, min_refI2;
	//kjh_211021 double tmp_val;

	//jhkw_211014s
	auxTH_offset = 0;
	if(myData->cData[ch].misc.minAuxTH_auxChNo != 0
		&& myData->cData[ch].misc.maxAuxTH_auxChNo != 0) {
		auxTH_offset = myData->mData.config.installedAuxV;
	}
	//jhkw_211014s
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
			//= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P1,	//ktg_210807
			= (unsigned short int)convert_ch_code(CONVERT_ORG_TO_P1,
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
	if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
		myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
	 	//0, 10) > 0) {
		0, 15) > 0) { //kjhw_180325
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
		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
			//0, 10) > 0) {
			0, 15) > 0) { //kjhw_180325
			val = myData->cData[ch].misc.tmpIsens;
			for(i=0; i < MAX_SLAVE_CH; i++) {
				j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
				if(j >= 0) {
					val += myData->cData[j].misc.tmpIsens;
				}
			}
//kjhw_150616e
		} else {
			if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
				myData->cData[ch].misc.checkDelayTime_day,
				myData->cData[ch].misc.checkDelayTime,
				0, 5) > 0) { //kjhw_180325
				val = myData->cData[ch].misc.tmpIsens;
				for(i=0; i < MAX_SLAVE_CH; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].misc.tmpIsens;
					}
				}
			} else {
				val = myData->cData[ch].op.Isens;
				for(i=0; i < MAX_SLAVE_CH; i++) {
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
		for(i=0; i < MAX_SLAVE_CH; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_AmpareHour;
			}
		}
		myData->save_msg[msg].val[idx].chData.charge_AmpareHour = val;

		val = myData->cData[ch].op.discharge_AmpareHour;
		for(i=0; i < MAX_SLAVE_CH; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_AmpareHour;
			}
		}
		myData->save_msg[msg].val[idx].chData.discharge_AmpareHour = val;

		/////////////////////////////////		//khj_191007
		switch(myData->cData[ch].op.stepType){
			case STEP_CHARGE:
				myData->save_msg[msg].val[idx].chData.charge_AmpareHour
					+= myData->save_msg[msg].val[idx].chData.discharge_AmpareHour;
				myData->save_msg[msg].val[idx].chData.discharge_AmpareHour = 0;
				break;
			case STEP_DISCHARGE:
			case STEP_Z:
				myData->save_msg[msg].val[idx].chData.discharge_AmpareHour
					+= myData->save_msg[msg].val[idx].chData.charge_AmpareHour;
				myData->save_msg[msg].val[idx].chData.charge_AmpareHour = 0;
				break;
			default:
				break;
		}
		/////////////////////////////////
		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
		 	//0, 10) > 0) {
			0, 15) > 0) { //kjhw_180325
			val = myData->cData[ch].misc.tmpWatt;
			for(i=0; i < MAX_SLAVE_CH; i++) {
				j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
				if(j >= 0) {
					val += myData->cData[j].misc.tmpWatt;
				}
			}
		} else {
			if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
				myData->cData[ch].misc.checkDelayTime_day,
				myData->cData[ch].misc.checkDelayTime,
				0, 5) > 0) { //kjhw_180325
				val = myData->cData[ch].misc.tmpWatt;
				for(i=0; i < MAX_SLAVE_CH; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].misc.tmpWatt;
					}
				}
			} else {
				val = myData->cData[ch].op.watt;
				for(i=0; i < MAX_SLAVE_CH; i++) {
					j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
					if(j >= 0) {
						val += myData->cData[j].op.watt;
					}
				}
			}
		}
		myData->save_msg[msg].val[idx].chData.watt = val;

		val = myData->cData[ch].op.charge_WattHour;
		for(i=0; i < MAX_SLAVE_CH; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_WattHour;
			}
		}
		myData->save_msg[msg].val[idx].chData.charge_WattHour = val;

		val = myData->cData[ch].op.discharge_WattHour;
		for(i=0; i < MAX_SLAVE_CH; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.discharge_WattHour;
			}
		}
		myData->save_msg[msg].val[idx].chData.discharge_WattHour = val;

		////////////////////////////////		/khj_191007
		switch(myData->cData[ch].op.stepType){
			case STEP_CHARGE:
				myData->save_msg[msg].val[idx].chData.charge_WattHour
				+= myData->save_msg[msg].val[idx].chData.discharge_WattHour;
				myData->save_msg[msg].val[idx].chData.discharge_WattHour = 0;
				break;
			case STEP_DISCHARGE:
			case STEP_Z:
				myData->save_msg[msg].val[idx].chData.discharge_WattHour
				+= myData->save_msg[msg].val[idx].chData.charge_WattHour;
				myData->save_msg[msg].val[idx].chData.charge_WattHour = 0;
				break;
			default:
				break;
		}
		////////////////////////////////

		val = myData->cData[ch].op.meanI;
		for(i=0; i < MAX_SLAVE_CH; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.meanI;
			}
		}
		myData->save_msg[msg].val[idx].chData.avgI = val;
	} else {
//kjhw_150616s
		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
		 	//0, 10) > 0) {
			0, 15) > 0) { //kjhw_180325
			val = myData->cData[ch].misc.tmpIsens;
//kjhw_150616e
		} else {
			if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
				myData->cData[ch].misc.checkDelayTime_day,
				myData->cData[ch].misc.checkDelayTime,
				0, 5) > 0) { //kjhw_180325
				val = myData->cData[ch].misc.tmpIsens;
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

		if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
			myData->cData[ch].op.runTime_day, myData->cData[ch].op.runTime,
		 	//0, 10) > 0) {
			0, 15) > 0) { //kjhw_180325
			val = myData->cData[ch].misc.tmpWatt;
		} else {
			if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
				myData->cData[ch].misc.checkDelayTime_day,
				myData->cData[ch].misc.checkDelayTime,
				0, 5) > 0) { //kjhw_180325
				val = myData->cData[ch].misc.tmpWatt;
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
		case C_HLGP_500V_200A_100A_200KW:
		case C_HLGP_500V_200A_100A_200KW_2:
		case C_HLGP_500V_200A_100A_200KW_3:
		case C_HLGP_500V_200A_100A_200KW_4:
		case C_HLGP_500V_200A_100A_200KW_5:
		case C_HLGP_500V_300A_100A_300KW:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_2:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_3:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_4:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_5:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_6:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_7:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_8:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_9:	//jhk_141127
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
	}	//kjhw_130429e
	myData->save_msg[msg].val[idx].chData.capacitance
		= myData->cData[ch].op.capacitance;
	myData->save_msg[msg].val[idx].chData.z = myData->cData[ch].op.z;

	myData->save_msg[msg].val[idx].chData.select
		= myData->cData[ch].op.select;
	myData->save_msg[msg].val[idx].chData.code
		//= (unsigned char)convert_ch_code(CONVERT_ORG_TO_P1,	//ktg_210807
		= (unsigned short int)convert_ch_code(CONVERT_ORG_TO_P1,
		(long)myData->cData[ch].op.code);
	myData->save_msg[msg].val[idx].chData.stepNo
		= myData->cData[ch].op.idxStepNo + 1;

	if(myData->cData[ch].op.stepType == STEP_END) {
		myData->cData[ch].op.runTime_day = 0;
	} //kjhw_180417
	myData->save_msg[msg].val[idx].chData.runTime_day
		= myData->cData[ch].op.runTime_day;
	myData->save_msg[msg].val[idx].chData.totalRunTime_day
		= myData->cData[ch].op.totalRunTime_day;
	myData->save_msg[msg].val[idx].chData.cvTime_day
		= myData->cData[ch].op.cvTime_day;
	myData->save_msg[msg].val[idx].chData.CycleTime_day
		= myData->cData[ch].misc.sum_CycleTime_day;	//ktg_231115
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
	myData->save_msg[msg].val[idx].chData.CycleTime
		= myData->cData[ch].misc.sum_CycleTime;	//ktg_231115
	//myData->save_msg[msg].val[idx].chData.ccTime
	//	= myData->cData[ch].op.ccTime; //kjg_w

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

	//csk_200113
	myData->save_msg[msg].val[idx].chData.auxDataCount
		= (short int)(myData->auxDataCount[ch][0]
		+ myData->auxDataCount[ch][1] + myData->auxDataCount[ch][2]
		+ myData->auxDataCount[ch][3] + myData->auxDataCount[ch][4]);//ktg_221020

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
	//jhkw_211014s
	/*myData->save_msg[msg].val[idx].chData.reserved2[0]
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
		= myData->cData[ch].misc.reserved1[5];*/

	myData->save_msg[msg].val[idx].chData.minAuxV
		= myData->cData[ch].misc.minAuxV;
	myData->save_msg[msg].val[idx].chData.minAuxV_auxChNo
		= myData->cData[ch].misc.minAuxV_auxChNo;
	myData->save_msg[msg].val[idx].chData.maxAuxV
		= myData->cData[ch].misc.maxAuxV;
	myData->save_msg[msg].val[idx].chData.maxAuxV_auxChNo
		= myData->cData[ch].misc.maxAuxV_auxChNo;
	myData->save_msg[msg].val[idx].chData.minAuxTemp
		= myData->cData[ch].misc.minAuxTemp;
	myData->save_msg[msg].val[idx].chData.minAuxTemp_auxChNo
		= myData->cData[ch].misc.minAuxTemp_auxChNo;
	myData->save_msg[msg].val[idx].chData.maxAuxTemp
		= myData->cData[ch].misc.maxAuxTemp;
	myData->save_msg[msg].val[idx].chData.maxAuxTemp_auxChNo
		= myData->cData[ch].misc.maxAuxTemp_auxChNo;
	myData->save_msg[msg].val[idx].chData.minAuxTH
		= myData->cData[ch].misc.minAuxTH;
	myData->save_msg[msg].val[idx].chData.minAuxTH_auxChNo
		= myData->cData[ch].misc.minAuxTH_auxChNo - auxTH_offset;
	myData->save_msg[msg].val[idx].chData.maxAuxTH
		= myData->cData[ch].misc.maxAuxTH;
	myData->save_msg[msg].val[idx].chData.maxAuxTH_auxChNo
		= myData->cData[ch].misc.maxAuxTH_auxChNo - auxTH_offset;
	//myData->save_msg[msg].val[idx].chData.reserved3	= 0;
	//jhkw_211014e
	myData->save_msg[msg].val[idx].chData.freeze_flag
		= myData->cData[ch].misc2.freeze_flag;	//ktg_230728
	myData->save_msg[msg].val[idx].chData.cv_flag
		= myData->cData[ch].op.cv_flag;	//ktg_230728
	myData->save_msg[msg].val[idx].chData.maxFreeze_CANTime
		= myData->cData[ch].misc2.freeze_time;	//ktg_230822
	myData->save_msg[msg].val[idx].chData.maxFreeze_CANChNo
		= myData->cData[ch].misc2.freeze_ChNo;	//ktg_230822
	//jhkw_220103s
	sub_code_div = convert_subcode_division((int)myData->save_msg[msg].val[idx].chData.code);
	switch(sub_code_div) {
		case 0:
			myData->save_msg[msg].val[idx].chData.sub_code[0].si_val[0] = 0;
			myData->save_msg[msg].val[idx].chData.sub_code[1].si_val[0] = 0;
			myData->save_msg[msg].val[idx].chData.sub_code[2].si_val[0] = 0;
			myData->save_msg[msg].val[idx].chData.sub_code[3].si_val[0] = 0;	//ktg_ttttttttttttt
			/*myData->save_msg[msg].val[idx].chData.sub_code[0].si_val[0] = myData->cData[ch].misc.cv_select;
			myData->save_msg[msg].val[idx].chData.sub_code[1].si_val[0] 
				= (short int)(myData->cData[ch].misc.cmd_v[0]/10);
			myData->save_msg[msg].val[idx].chData.sub_code[2].si_val[0] 
				= (short int)(myData->cData[ch].misc.cmd_i[0]/10);
			myData->save_msg[msg].val[idx].chData.sub_code[3].si_val[0]
				= (short int)Read_InPoint(group, ch+1, I_MODE_CV_DETECT);*/
			break;
		case 1:
			myData->save_msg[msg].val[idx].chData.sub_code[0].si_val[0] 
				= myData->cData[ch].op.subCode.end_div_no;
			myData->save_msg[msg].val[idx].chData.sub_code[1].si_val[0] 
				= myData->cData[ch].op.subCode.func_div;
			myData->save_msg[msg].val[idx].chData.sub_code[2].si_val[0] 
				= myData->cData[ch].op.subCode.ch_index;
			myData->save_msg[msg].val[idx].chData.sub_code[3].si_val[0] 
				= myData->cData[ch].op.subCode.reserved1;
			break;
		default: 
			myData->save_msg[msg].val[idx].chData.sub_code[0].si_val[0] = 0;
			myData->save_msg[msg].val[idx].chData.sub_code[1].si_val[0] = 0;
			myData->save_msg[msg].val[idx].chData.sub_code[2].si_val[0] = 0;
			myData->save_msg[msg].val[idx].chData.sub_code[3].si_val[0] = 0;
			break;
	}
	//jhkw_220103e
	myData->save_msg[msg].val[idx].chData.reserved4[0]	//ktg_210807s
		= 0;
	myData->save_msg[msg].val[idx].chData.reserved4[1]
		= 0;
	myData->save_msg[msg].val[idx].chData.reserved4[2]
		= 0;	//ktg_210807e

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
						= myData->COM.com_port[AUX_TEMP].value[k];
					break;
				case 1: //sub sensing voltage
				case 2: //sub sensing thermistor //kjh_160610
				case 3: //sub sensing humidity 	//khj_191205
				case 4: //sub sensing gas	 	//sec_220926
					k = myData->aux_ch_num[k].daq_ch; //jhkw_201117
					if((myData->cData[ch].misc.svdf_drop_flag == 1
						|| myData->cData[ch].misc.svdf_drop_flag == 2)
						&& (myData->auxSetData[i].auxType == 1)) {//ktg_2202214
						myData->save_msg[msg].val[idx].auxData[j].val
							= myData->SubSensV.ch[k].svdfAuxV;
					} else {
						myData->save_msg[msg].val[idx].auxData[j].val
							= myData->SubSensV.ch[k].sensV;
					}
					break;
				default:
					myData->save_msg[msg].val[idx].auxData[j].val = 0;
					break;
			}
			j++;
		}
	}

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

void send_save_msg_3(int ch, int msg)
{
#ifdef __COC__

#endif
}

void send_record_msg(int ch, int index)
{
#ifdef __COC__

#endif
}

void send_record_msg_1(int ch, int msg, int index)
{
#ifdef __COC__

#endif
}

void send_pulse_msg(int ch, int count_flag)
{
	if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COA][0] == P1) {
		if(myData->ChAttribute[ch].chNo_master == 0) return;

		send_pulse_msg_1(ch, 0, count_flag); //COA1_Client
		send_pulse_msg_1(ch, 1, count_flag); //DataSave
	}
}

void send_pulse_msg_1(int ch, int msg, int count_flag)
{

	unsigned char item, idxStepNo;
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
			i = myData->cData[ch].misc.d_count;
			myData->cData[ch].misc.d_count++;
			myData->cData[ch].misc.d_t[i] = (long)i;
			myData->cData[ch].misc.d_v[i] = myData->cData[ch].misc.tmpVsens;
			myData->cData[ch].misc.d_i[i]
				= myData->cData[ch].misc.total_tmpIsens;
		}
		return;
	} else if(myData->cData[ch].misc.pulse_count == pulse_t2) {
		if(msg == 0) {
			i = myData->cData[ch].misc.d_count;
			myData->cData[ch].misc.d_count++;
			myData->cData[ch].misc.d_t[i] = (long)i;
			myData->cData[ch].misc.d_v[i] = myData->cData[ch].misc.tmpVsens;
			myData->cData[ch].misc.d_i[i]
				= myData->cData[ch].misc.total_tmpIsens;

			calculate_DCR_2(ch, 0); //v1
			myData->cData[ch].misc.d_count = 1;
		}
	} else if(myData->cData[ch].misc.pulse_count < pulse_t3) {
		if(msg == 0) {
			i = myData->cData[ch].misc.d_count;
			myData->cData[ch].misc.d_count++;
			myData->cData[ch].misc.d_t[i] = (long)i;
			myData->cData[ch].misc.d_v[i] = myData->cData[ch].misc.tmpVsens;
			myData->cData[ch].misc.d_i[i]
				= myData->cData[ch].misc.total_tmpIsens;
		}
	} else if(myData->cData[ch].misc.pulse_count == pulse_t3) {
		if(msg == 0) {
			i = myData->cData[ch].misc.d_count;
			myData->cData[ch].misc.d_count++;
			myData->cData[ch].misc.d_t[i] = (long)i;
			myData->cData[ch].misc.d_v[i] = myData->cData[ch].misc.tmpVsens;
			myData->cData[ch].misc.d_i[i]
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
		for(i=0; i < MAX_SLAVE_CH; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_AmpareHour;
			}
		}
		val1 = val;

		val = myData->cData[ch].op.discharge_AmpareHour;
		for(i=0; i < MAX_SLAVE_CH; i++) {
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
		for(i=0; i < MAX_SLAVE_CH; i++) {
			j = (int)myData->ChAttribute[ch].chNo_slave[i] - 1;
			if(j >= 0) {
				val += myData->cData[j].op.charge_WattHour;
			}
		}
		val1 = val;

		val = myData->cData[ch].op.discharge_WattHour;
		for(i=0; i < MAX_SLAVE_CH; i++) {
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
}

void send_save_msg_formation(int group, int div)
{
#ifdef __COB__

#endif
}

void send_save_msg_formation_1(int group, int toPs, int div)
{ //debug_size_cob

}

void send_save_msg_formation_2(int group, int toPs, int div)
{ //debug_size_cob

}

void send_save_msg_formation_3(int group, int toPs, int div)
{ //debug_size_cob

}

void send_save_realData_msg(int group)
{
#ifdef __COB__

#endif
}

void send_save_realData_msg_Formation_A(int group, int toPs)
{
#ifdef __COB__

#endif
}

void send_save_realData_msg_Formation_B(int group, int toPs)
{ //debug_size_cob

}

void send_save_realData_msg_Formation_C(int group, int toPs)
{ //debug_size_cob

}
