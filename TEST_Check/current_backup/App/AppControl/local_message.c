#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"
#include "local_message.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_APP_CONTROL *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	msgParsing(COA1_TO_APP);
	msgParsing(COA2_TO_APP);
	msgParsing(COB1_TO_APP);
	msgParsing(COB2_TO_APP);
	msgParsing(COC1_TO_APP);
	msgParsing(COC2_TO_APP);
	msgParsing(COD1_TO_APP);
	msgParsing(COD2_TO_APP);
	msgParsing(MODULE_TO_APP);
}

void msgParsing(int fromPs)
{
	int idx;
	S_MSG_VAL RecvMsg;
	
	idx = rcv_msg(fromPs, (S_MSG_VAL *)&RecvMsg);
	if(idx < 0) return;

	userlog(DEBUG_LOG, psName, "kjg_d message : %d %d %d\n",
		fromPs, idx, RecvMsg.msg);

	switch(fromPs) {
		case COA1_TO_APP:
		case COA2_TO_APP:
			msgParsing_COA_to_App(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case COB1_TO_APP:
		case COB2_TO_APP:
			msgParsing_COB_to_App(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case COC1_TO_APP:
		case COC2_TO_APP:
			msgParsing_COC_to_App(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case MODULE_TO_APP:
			msgParsing_Module_to_App(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", fromPs, idx, RecvMsg.msg);
			break;
	}
}

void msgParsing_COA_to_App(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int rtn, tmp, group, tmp2;
	S_MSG_VAL SendMsg;

/*kjg_170810 #if defined _COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	unsigned char chFlag, chFlag1;
	int i, j, ch;
	S_MSG_CH_FLAG ch_flag;
#endif*/

	switch(RecvMsg->msg) {
		case MSG_COA_APP_PROCESS_KILL:
			group = RecvMsg->val[0];
			if(myData->COA_Client[group].misc.processPointer > 0) {
				Close_mbuff(myData->COA_Client[group].misc.processPointer);
			}
			switch(group) {
				case 0: Kill_Process("COA1_Client"); break;
				case 1: Kill_Process("COA2_Client"); break;
				case 2: Kill_Process("COA3_Client"); break;
				case 3: Kill_Process("COA4_Client"); break;
			}
			break;
		case MSG_COA_APP_WRITE_CH_ATTRIBUTE:
			rtn = Write_ChAttribute();
			break;
		case MSG_COA_APP_WRITE_AUX_SET_DATA:
			rtn = Write_AuxSetData();
			break;
		case MSG_COA_APP_WRITE_CAN_RECEIVE_SET_DATA:
			tmp = RecvMsg->val[1];

			switch(myData->AppControl.config.systemModel) {
				case C_SKI_120V_400A_100A_192KW: //kjhw_170629
				case C_SKI_120V_400A_100A_192KW_2:
				case C_SKI_120V_400A_100A_192KW_3:
				case C_SKI_120V_400A_100A_192KW_4:
				case C_SKI_120V_400A_100A_192KW_5:
				case C_SKI_120V_400A_100A_192KW_6:
				case C_SKI_120V_400A_100A_192KW_7:
				case C_SKI_120V_400A_100A_192KW_8:
				case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
				case C_SK_450V_200A_10A_360KW:
					if(tmp == 0) tmp2 = tmp;
					else tmp2 = 1;
					break;
				default:
					tmp2 = tmp;
					break;
			}

			switch(myData->mData.config.division_CAN) {
				case 1: //kjhw_140620
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_APP_MODULE_CAN_ABORT_TRANSMISSION;
					SendMsg.val[0] = tmp2;
					send_msg(APP_TO_MODULE, (char *)&SendMsg);

					rtn = Write_CanReceiveSetData();
					if(rtn < 0) break;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_APP_MODULE_CAN_BAUDRATE_SET;
					SendMsg.val[0] = tmp2;
					switch(myData->canReceiveSetData.commonData[tmp][0]
							.can_baudrate) {
						case 0: //125K
							SendMsg.val[1] = 125000;
							break;
						case 1: //250K
							SendMsg.val[1] = 250000;
							break;
						case 2: //500K
							SendMsg.val[1] = 500000;
							break;
						case 3: //1M
							SendMsg.val[1] = 1000000;
							break;
						default: //User
							SendMsg.val[1] = 500000; //kjg_180405 0->500000
							break;
					}
					SendMsg.val[2] = (int)myData->canReceiveSetData
						.commonData[tmp][0].extended_id;
					SendMsg.val[3] = (int)myData->canReceiveSetData
						.commonData[tmp][0].sjw;
#ifdef __CAN_FD__ //jhkw_190714s
					SendMsg.val[4] = (int)myData->canReceiveSetData
						.commonData[tmp][0].can_fd_flag;
					SendMsg.val[5] = (int)myData->canReceiveSetData
						.commonData[tmp][0].can_datarate;
					SendMsg.val[6] = (int)myData->canReceiveSetData
						.commonData[tmp][0].crc_type;
					SendMsg.val[7] = (int)myData->canReceiveSetData
						.commonData[tmp][0].terminal_r;
#endif //jhkw_190714e
					send_msg(APP_TO_MODULE, (char *)&SendMsg);
					break;
				default:
					for(rtn=0; rtn < 2; rtn++) {
						memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
						SendMsg.msg = MSG_APP_MODULE_CAN_ABORT_TRANSMISSION;
						SendMsg.val[0] = tmp2 * 2 + rtn;
						send_msg(APP_TO_MODULE, (char *)&SendMsg);
					}

					rtn = Write_CanReceiveSetData();
					if(rtn < 0) break;

					for(rtn=0; rtn < 2; rtn++) {
						memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
						SendMsg.msg = MSG_APP_MODULE_CAN_BAUDRATE_SET;
						SendMsg.val[0] = tmp2 * 2 + rtn;
						switch(myData->canReceiveSetData.commonData[tmp][rtn]
							.can_baudrate) {
							case 0: //125K
								SendMsg.val[1] = 125000;
								break;
							case 1: //250K
								SendMsg.val[1] = 250000;
								break;
							case 2: //500K
								SendMsg.val[1] = 500000;
								break;
							case 3: //1M
								SendMsg.val[1] = 1000000;
								break;
							default: //User
								SendMsg.val[1] = 500000; //kjg_180405 0->500000
								break;
						}
						SendMsg.val[2] = (int)myData->canReceiveSetData
							.commonData[tmp][rtn].extended_id;
						SendMsg.val[3] = (int)myData->canReceiveSetData
							.commonData[tmp][rtn].sjw;
#ifdef __CAN_FD__ //jhkw_190714s
						SendMsg.val[4] = (int)myData->canReceiveSetData
							.commonData[tmp][rtn].can_fd_flag;
						SendMsg.val[5] = (int)myData->canReceiveSetData
							.commonData[tmp][rtn].can_datarate;
						SendMsg.val[6] = (int)myData->canReceiveSetData
							.commonData[tmp][rtn].crc_type;
						SendMsg.val[7] = (int)myData->canReceiveSetData
							.commonData[tmp][rtn].terminal_r;
#endif //jhkw_190714e
						send_msg(APP_TO_MODULE, (char *)&SendMsg);
					}
					break;
			}

		//	if(myData->AppControl.misc.Load_Process_CAN[2] == P1) {
		//		memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
		//		SendMsg.msg = MSG_APP_CAN_RECEIVE_SET;
		//		SendMsg.val[0] = tmp;
		//		send_msg(APP_TO_CAN, (char *)&SendMsg);
		//		break;
		//	} else {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_APP_MODULE_CAN_DATA_CLEAR;
				SendMsg.val[0] = tmp;
				send_msg(APP_TO_MODULE, (char *)&SendMsg);
				break;
		//	}
		case MSG_COA_APP_WRITE_CAN_TRANSMIT_SET_DATA:
			tmp = RecvMsg->val[1];

			switch(myData->AppControl.config.systemModel) {
				case C_SKI_120V_400A_100A_192KW: //kjhw_170629
				case C_SKI_120V_400A_100A_192KW_2:
				case C_SKI_120V_400A_100A_192KW_3:
				case C_SKI_120V_400A_100A_192KW_4:
				case C_SKI_120V_400A_100A_192KW_5:
				case C_SKI_120V_400A_100A_192KW_6:
				case C_SKI_120V_400A_100A_192KW_7:
				case C_SKI_120V_400A_100A_192KW_8:
				case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
				case C_SK_450V_200A_10A_360KW:
					if(tmp == 0) tmp2 = tmp;
					else tmp2 = 1;
					break;
				default:
					tmp2 = tmp;
					break;
			}

			switch(myData->mData.config.division_CAN) {
				case 1: //kjhw_140620
					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_APP_MODULE_CAN_ABORT_TRANSMISSION;
					SendMsg.val[0] = tmp2;
					send_msg(APP_TO_MODULE, (char *)&SendMsg);

					rtn = Write_CanTransmitSetData();
					if(rtn < 0) break;

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_APP_MODULE_CAN_BAUDRATE_SET;
					SendMsg.val[0] = tmp2;
					switch(myData->canTransmitSetData.commonData[tmp][0]
						.can_baudrate) {
						case 0: //125K
							SendMsg.val[1] = 125000;
							break;
						case 1: //250K
							SendMsg.val[1] = 250000;
							break;
						case 2: //500K
							SendMsg.val[1] = 500000;
							break;
						case 3: //1M
							SendMsg.val[1] = 1000000;
							break;
						default: //User
							SendMsg.val[1] = 500000; //kjg_180405 0->500000
							break;
					}
					SendMsg.val[2] = (int)myData->canTransmitSetData
						.commonData[tmp][0].extended_id;
					SendMsg.val[3] = (int)myData->canTransmitSetData
						.commonData[tmp][0].sjw;
#ifdef __CAN_FD__ //jhkw_190714s
					SendMsg.val[4] = (int)myData->canTransmitSetData
						.commonData[tmp][0].can_fd_flag;
					SendMsg.val[5] = (int)myData->canTransmitSetData
						.commonData[tmp][0].can_datarate;
					SendMsg.val[6] = (int)myData->canTransmitSetData
						.commonData[tmp][0].crc_type;
					SendMsg.val[7] = (int)myData->canTransmitSetData
						.commonData[tmp][0].terminal_r;
#endif //jhkw_190714e
					send_msg(APP_TO_MODULE, (char *)&SendMsg);
					break;
				default:
					for(rtn=0; rtn < 2; rtn++) {
						memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
						SendMsg.msg = MSG_APP_MODULE_CAN_ABORT_TRANSMISSION;
						SendMsg.val[0] = tmp2 * 2 + rtn;
						send_msg(APP_TO_MODULE, (char *)&SendMsg);
					}

					rtn = Write_CanTransmitSetData();
					if(rtn < 0) break;

					for(rtn=0; rtn < 2; rtn++) {
						memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
						SendMsg.msg = MSG_APP_MODULE_CAN_BAUDRATE_SET;
						SendMsg.val[0] = tmp2 * 2 + rtn;
						switch(myData->canTransmitSetData.commonData[tmp][rtn]
							.can_baudrate) {
							case 0: //125K
								SendMsg.val[1] = 125000;
								break;
							case 1: //250K
								SendMsg.val[1] = 250000;
								break;
							case 2: //500K
								SendMsg.val[1] = 500000;
								break;
							case 3: //1M
								SendMsg.val[1] = 1000000;
								break;
							default: //User
								SendMsg.val[1] = 500000; //kjg_180405 0->500000
								break;
						}
						SendMsg.val[2] = (int)myData->canTransmitSetData
							.commonData[tmp][rtn].extended_id;
						SendMsg.val[3] = (int)myData->canTransmitSetData
							.commonData[tmp][rtn].sjw;
#ifdef __CAN_FD__ //jhkw_190714s
						SendMsg.val[4] = (int)myData->canTransmitSetData
							.commonData[tmp][rtn].can_fd_flag;
						SendMsg.val[5] = (int)myData->canTransmitSetData
							.commonData[tmp][rtn].can_datarate;
						SendMsg.val[6] = (int)myData->canTransmitSetData
							.commonData[tmp][rtn].crc_type;
						SendMsg.val[7] = (int)myData->canTransmitSetData
							.commonData[tmp][rtn].terminal_r;
#endif //jhkw_190714e
						send_msg(APP_TO_MODULE, (char *)&SendMsg);
					}
					break;
			}

			if(myData->AppControl.misc.Load_Process_CAN[2] == P1) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_APP_CAN_TRANSMIT_SET;
				SendMsg.val[0] = tmp;
				send_msg(APP_TO_CAN, (char *)&SendMsg);
				break;
			} else {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_APP_MODULE_CAN_DATA_CLEAR;
				SendMsg.val[0] = tmp;
				send_msg(APP_TO_MODULE, (char *)&SendMsg);
				break;
			}
		case MSG_COA_APP_WRITE_VBF_DATA:
			group = RecvMsg->val[0];
			tmp = APP_TO_COA1 + group;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_APP_COA_WRITE_VBF_DATA_REPLY;
			SendMsg.val[0] = group;

			rtn = Write_VBF_File((char *)&myData->CAN
				.tmp_can_flash_file_name[0], (char *)&myData->CAN
				.tmp_can_flash_file[0]);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "vbf write error %d\n", rtn);
				SendMsg.val[1] = 0; //0:NG, 1:OK
			} else {
				rtn = Read_CanFlashFile();
				if(rtn < 0) {
					userlog(DEBUG_LOG, psName, "vbf read error %d\n", rtn);
					SendMsg.val[1] = 0; //0:NG, 1:OK
				} else {
					SendMsg.val[1] = 1; //0:NG, 1:OK
				}
			}

			send_msg(tmp, (char *)&SendMsg);
			break;
/*kjg_170810
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
		case MSG_COA_APP_TEST_COND_CONVERSION_START:
			group = RecvMsg->val[0];
			chFlag = 0x01;

			memcpy((char *)&ch_flag,(char *)&myData->msg[fromPs]
				.msg_ch_flag[idx], sizeof(S_MSG_CH_FLAG));

			rtn = 0;
			for(i=0; i < myData->mData.config.chInGroup[group]; i++) {
				j = i / 8;
				chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
				if(chFlag1 == 0) continue;

				ch = myData->CellArray1[i].number2 - 1;

				userlog(DEBUG_LOG, psName,
					"test_cond_converstion start %d, %d\n", ch+1, rtn);

				rtn = read_test_cond_step_file_coa(ch);
				if(rtn < 0) {
					userlog(DEBUG_LOG, psName,
						"read_test_cond_step_file error %d, %d\n", ch, rtn);
					break;
				}

				rtn = convert_test_cond_coa(ch);
				if(rtn < 0) {
					userlog(DEBUG_LOG, psName,
						"convert_test_cond error %d, %d\n", ch, rtn);
					break;
				}

				userlog(DEBUG_LOG, psName,
					"test_cond_converstion end %d, %d\n", ch+1, rtn);

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~ //kjh_160418
				if(myData->COA_Client[0].testCond.header.totalTimeSchCount
					> 0) {
					userlog(DEBUG_LOG, psName,
						"test_cond_converstion_time start %d, %d\n", ch+1, rtn);

					rtn = read_test_cond_time_step_file_coa(ch);
					if(rtn < 0) {
						userlog(DEBUG_LOG, psName,
							"read_test_cond_time_step_file error %d, %d\n",
							ch, rtn);
						break;
					}

					rtn = convert_test_cond_time_coa(ch);
					if(rtn < 0) {
						userlog(DEBUG_LOG, psName,
							"convert_test_cond_time error %d, %d\n", ch, rtn);
						break;
					}

					userlog(DEBUG_LOG, psName,
						"test_cond_time_converstion end %d, %d\n", ch+1, rtn);
				} else {
					//kjh_161109
					memset((char *)&myData->TimeSch.testCond[ch], 0, 
						sizeof(S_TIMESCH_TEST_CONDITION));
				}
#endif
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_APP_COA_TEST_COND_CONVERSION_END;
			SendMsg.val[0] = group;
			if(rtn < 0) {
				SendMsg.val[1] = 0; //nack

				for(i=0; i < myData->mData.config.chInGroup[group]; i++) {
					j = i / 8;
					chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
					if(chFlag1 == 0) continue;

					ch = myData->CellArray1[i].number2 - 1;

					myData->COA_Client[group]
						.signal[P1_SIG_TEST_CONVERSION_END_1 + ch] = P0;
				}
			} else {
				SendMsg.val[1] = 1; //ack

				for(i=0; i < myData->mData.config.chInGroup[group]; i++) {
					j = i / 8;
					chFlag1 = (chFlag << (i % 8)) & ch_flag.flag[j];
					if(chFlag1 == 0) continue;

					ch = myData->CellArray1[i].number2 - 1;

					myData->COA_Client[group]
						.signal[P1_SIG_TEST_CONVERSION_END_1 + ch] = P1;
				}
			}
			send_msg(APP_TO_COA1+group, (char *)&SendMsg);

			userlog(DEBUG_LOG, psName,
				"test_cond_converstion end send %d ch_flag %x %x\n", rtn,
				ch_flag.flag[0], ch_flag.flag[1]);
			break;
#endif*/
		case MSG_COA_APP_READ_TH_TABLE_SET: //jhkw_191108
			group = RecvMsg->val[0];
			tmp2 = RecvMsg->val[1];
			tmp = APP_TO_COA1 + group;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_APP_COA_READ_TH_TABLE_SET_REPLY;
			SendMsg.val[0] = group;

			rtn = Read_Th_Table_Type(tmp2,tmp2);
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "Read_Th_Table_Type read error %d\n", rtn);
				SendMsg.val[1] = 0; //0:NG, 1:OK
				Copy_Th_Table(tmp2);
			} else {
				SendMsg.val[1] = 1; //0:NG, 1:OK
			}

			SendMsg.val[2] = tmp2; //table num
			send_msg(tmp, (char *)&SendMsg);
			break;
		case MSG_COA_APP_CMD_AUTOCALI_TEMP: //jhkw_200317s
			group = RecvMsg->val[0];
			tmp2 = RecvMsg->val[1];
			tmp = APP_TO_COA1 + group;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_APP_COA_CMD_AUTOCALI_TEMP_REPLY;
			SendMsg.val[0] = group;

			rtn = 0;
			switch(tmp2) {
				case 0:
					rtn = 0;
					userlog(DEBUG_LOG, psName, "mode request %d, reply %d\n", tmp2, rtn);
					break;
				case 1:
					rtn = Read_Measure_Cali_2(tmp2);
					userlog(DEBUG_LOG, psName, "mode request %d, reply %d\n", tmp2, rtn);
					break;
				case 2:
					rtn = Read_Measure_Cali_2(tmp2);
					if(rtn < 0) {
					} else {
						Copy_Measure_cali(tmp2);
					}
					userlog(DEBUG_LOG, psName, "mode request %d, reply %d\n", tmp2, rtn);
					break;
				default:
					break;
			}
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "Measure_Cali_2 read error %d\n", rtn);
				SendMsg.val[1] = 0; //0:NG, 1:OK
			} else {
				SendMsg.val[1] = 1; //0:NG, 1:OK
			}

			send_msg(tmp, (char *)&SendMsg);
			break;	
			//jhkw_200317e
		default:
			userlog(DEBUG_LOG, psName, "COA%d to App Msg Unknown : %d\n",
				fromPs+1, RecvMsg->msg);
			break;
	}
}

void msgParsing_COB_to_App(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{ //debug_size_cob
/*	int group;

	switch(RecvMsg->msg) {
		case MSG_COB_APP_PROCESS_KILL:
			group = RecvMsg->val[0];
			if(myData->COB_Client[group].misc.processPointer > 0) {
				Close_mbuff(myData->COB_Client[group].misc.processPointer);
			}
			switch(group) {
				case 0: Kill_Process("COB1_Client"); break;
				case 1: Kill_Process("COB2_Client"); break;
				case 2: Kill_Process("COB3_Client"); break;
				case 3: Kill_Process("COB4_Client"); break;
			}
			break;
		case MSG_COB_APP_WRITE_CH_ATTRIBUTE:
			group = Write_ChAttribute();
			break;
		case MSG_COB_APP_CALI_UPDATE:
			myPs->signal[APP_SIG_CALI_UPDATE] = P50;
			break;
		default:
			userlog(DEBUG_LOG, psName, "COB%d to App Msg Unknown : %d\n",
				fromPs+1, RecvMsg->msg);
			break;
	}*/
}

void msgParsing_COC_to_App(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{ //debug_size_coc
/*	int rtn, tmp, group, tmp2;
	S_MSG_VAL SendMsg;

	switch(RecvMsg->msg) {
		case MSG_COC_APP_PROCESS_KILL:
			group = RecvMsg->val[0];
			if(myData->COC_Client[group].misc.processPointer > 0) {
				Close_mbuff(myData->COC_Client[group].misc.processPointer);
			}
			switch(group) {
				case 0: Kill_Process("COC1_Client"); break;
				case 1: Kill_Process("COC2_Client"); break;
				case 2: Kill_Process("COC3_Client"); break;
				case 3: Kill_Process("COC4_Client"); break;
			}
			break;
		case MSG_COC_APP_WRITE_CH_ATTRIBUTE:
			rtn = Write_ChAttribute();
			break;
		case MSG_COC_APP_WRITE_AUX_SET_DATA:
			rtn = Write_AuxSetData();
			break;
		case MSG_COC_APP_WRITE_CAN_RECEIVE_SET_DATA:
			tmp = RecvMsg->val[1];
			tmp2 = tmp;

			for(rtn=0; rtn < 2; rtn++) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_APP_MODULE_CAN_ABORT_TRANSMISSION;
				SendMsg.val[0] = tmp2 * 2 + rtn;
				send_msg(APP_TO_MODULE, (char *)&SendMsg);
			}

			rtn = Write_CanReceiveSetData();
			if(rtn < 0) break;
			for(rtn=0; rtn < 2; rtn++) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_APP_MODULE_CAN_BAUDRATE_SET;
				SendMsg.val[0] = tmp2 * 2 + rtn;
				switch(myData->canReceiveSetData.commonData[tmp][rtn]
					.can_baudrate) {
					case 0: //125K
						SendMsg.val[1] = 125000;
						break;
					case 1: //250K
						SendMsg.val[1] = 250000;
						break;
					case 2: //500K
						SendMsg.val[1] = 500000;
						break;
					case 3: //1M
						SendMsg.val[1] = 1000000;
						break;
					default: //User
						SendMsg.val[1] = 500000; //kjg_180405 0->500000
						break;
				}
				SendMsg.val[2] = (int)myData->canReceiveSetData
					.commonData[tmp][rtn].extended_id;
				SendMsg.val[3] = (int)myData->canReceiveSetData
					.commonData[tmp][rtn].sjw;
				SendMsg.val[4] = (int)myData->canReceiveSetData
					.commonData[tmp][rtn].can_fd_flag;
				SendMsg.val[5] = (int)myData->canReceiveSetData
					.commonData[tmp][rtn].can_datarate;
				SendMsg.val[6] = (int)myData->canReceiveSetData
					.commonData[tmp][rtn].crc_type;
				SendMsg.val[7] = (int)myData->canReceiveSetData
					.commonData[tmp][rtn].terminal_r;
				send_msg(APP_TO_MODULE, (char *)&SendMsg);
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_APP_MODULE_CAN_DATA_CLEAR;
			SendMsg.val[0] = tmp;
			send_msg(APP_TO_MODULE, (char *)&SendMsg);
			break;
		case MSG_COC_APP_WRITE_CAN_TRANSMIT_SET_DATA:
			tmp = RecvMsg->val[1];
			tmp2 = tmp;

			for(rtn=0; rtn < 2; rtn++) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_APP_MODULE_CAN_ABORT_TRANSMISSION;
				SendMsg.val[0] = tmp2 * 2 + rtn;
				send_msg(APP_TO_MODULE, (char *)&SendMsg);
			}

			rtn = Write_CanTransmitSetData();
			if(rtn < 0) break;
			for(rtn=0; rtn < 2; rtn++) {
				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_APP_MODULE_CAN_BAUDRATE_SET;
				SendMsg.val[0] = tmp2 * 2 + rtn;
				switch(myData->canTransmitSetData.commonData[tmp][rtn]
					.can_baudrate) {
					case 0: //125K
						SendMsg.val[1] = 125000;
						break;
					case 1: //250K
						SendMsg.val[1] = 250000;
						break;
					case 2: //500K
						SendMsg.val[1] = 500000;
						break;
					case 3: //1M
						SendMsg.val[1] = 1000000;
						break;
					default: //User
						SendMsg.val[1] = 500000; //kjg_180405 0->500000
						break;
				}
				SendMsg.val[2] = (int)myData->canTransmitSetData
					.commonData[tmp][rtn].extended_id;
				SendMsg.val[3] = (int)myData->canTransmitSetData
					.commonData[tmp][rtn].sjw;
				SendMsg.val[4] = (int)myData->canTransmitSetData
					.commonData[tmp][rtn].can_fd_flag;
				SendMsg.val[5] = (int)myData->canTransmitSetData
					.commonData[tmp][rtn].can_datarate;
				SendMsg.val[6] = (int)myData->canTransmitSetData
					.commonData[tmp][rtn].crc_type;
				SendMsg.val[7] = (int)myData->canTransmitSetData
					.commonData[tmp][rtn].terminal_r;
				send_msg(APP_TO_MODULE, (char *)&SendMsg);
			}

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_APP_MODULE_CAN_DATA_CLEAR;
			SendMsg.val[0] = tmp;
			send_msg(APP_TO_MODULE, (char *)&SendMsg);
			break;
		default:
			userlog(DEBUG_LOG, psName, "COC%d to App Msg Unknown : %d\n",
				fromPs+1, RecvMsg->msg);
			break;
	}*/
}

void msgParsing_Module_to_App(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	int group, type, i;
	int rtn; //kjhw_150216

	rtn = 0; //kjhw_150216
	switch(RecvMsg->msg) {
		case MSG_MODULE_APP_EXIT:
			myPs->signal[APP_SIG_QUIT] = P1;
			myPs->signal[APP_SIG_QUIT_TYPE] = (unsigned char)RecvMsg->val[1];
			myPs->signal[APP_SIG_QUIT_VALUE] = (unsigned char)RecvMsg->val[2];
			break;
		case MSG_MODULE_APP_PROCESS_KILL:
			for(group=0; group < myPs->config.totalGroup; group++) {
				if(myData->COA_Client[group].misc.processPointer > 0) {
					Close_mbuff(myData->COA_Client[group].misc.processPointer);
				}
				switch(group) {
					case 0: Kill_Process_2("COA_Client", "1"); break;
					case 1: Kill_Process_2("COA_Client", "2"); break;
					case 2: Kill_Process_2("COA_Client", "3"); break;
					case 3: Kill_Process_2("COA_Client", "4"); break;
				}
			}
			break;
		case MSG_MODULE_APP_CALI_DA_END:
			group = RecvMsg->val[0];
			type = RecvMsg->val[1];
			if(type == 0) {
				i = 0; //range
			} else {
				i = myData->mData.signal[M_SIG_CALI_RANGE_I] - 1;
			}

			userlog(DEBUG_LOG, psName,
				"Calibration da end group:%d type:%d, range:%d\n",
				group+1, type, i);
			userlog(DEBUG_LOG, psName,
				"org_meter:%ld, %ld, cali_da:%f, %f, cali_meter:%ld, %ld\n",
				myData->gData[group].cali_main_dac.tmp_org_meter[type][i][0],
				myData->gData[group].cali_main_dac.tmp_org_meter[type][i][1],
				myData->gData[group].cali_main_dac.tmp_DA_A[type][i][0],
				myData->gData[group].cali_main_dac.tmp_DA_B[type][i][0],
				myData->gData[group].cali_main_dac.tmp_cal_meter[type][i][0],
				myData->gData[group].cali_main_dac.tmp_cal_meter[type][i][1]);
			userlog(DEBUG_LOG, psName,
				"org_meter:%ld, %ld, cali_da:%f, %f, cali_meter:%ld, %ld\n",
				myData->gData[group].cali_main_dac.tmp_org_meter[type][i][2],
				myData->gData[group].cali_main_dac.tmp_org_meter[type][i][3],
				myData->gData[group].cali_main_dac.tmp_DA_A[type][i][1],
				myData->gData[group].cali_main_dac.tmp_DA_B[type][i][1],
				myData->gData[group].cali_main_dac.tmp_cal_meter[type][i][2],
				myData->gData[group].cali_main_dac.tmp_cal_meter[type][i][3]);
			break;
		case MSG_MODULE_APP_CALI_END:
			userlog(DEBUG_LOG, psName, "Calibration end bd:%d range:%d\n",
				RecvMsg->val[0]+1, RecvMsg->val[1]+1);
			break;
		case MSG_MODULE_APP_FAULT_HW_CURRENT: //jhkw_140313 //kjhw_140219
			//shh_230518t
			/*userlog(DEBUG_LOG, psName, "FAULT_HW_CURRENT ch:%d count:%d\n",
				RecvMsg->val[0], RecvMsg->val[1]);*/
			userlog(DEBUG_LOG, psName, "FAULT_HW_CURRENT ch:%d\n",RecvMsg->val[0]);
			userlog(DEBUG_LOG, psName, "sensSumI[0]:%d\n",RecvMsg->val[1]);
			userlog(DEBUG_LOG, psName, "sensSumI[1]:%d\n",RecvMsg->val[2]);
			userlog(DEBUG_LOG, psName, "sensSumI[2]:%d\n",RecvMsg->val[3]);
			userlog(DEBUG_LOG, psName, "sensSumI[3]:%d\n",RecvMsg->val[4]);
			userlog(DEBUG_LOG, psName, "sensCount:%d\n",RecvMsg->val[5]);
			//userlog(DEBUG_LOG, psName, "ref.val:%d\n",RecvMsg->val[5]);
			userlog(DEBUG_LOG, psName, "code:%d\n",RecvMsg->val[6]);
			userlog(DEBUG_LOG, psName, "total_Isens:%d\n",RecvMsg->val[7]);
			//shh_230518t
			break;
		//jhkw_170917s
		case MSG_MODULE_APP_FAULT_HW_OC:
			userlog(DEBUG_LOG, psName, "FAULT_HW_OC ch:%d dcdc_ch:%d\n",
				RecvMsg->val[0], RecvMsg->val[1]);
			break;
		case MSG_MODULE_APP_FAULT_HW_DCOV:
			userlog(DEBUG_LOG, psName, "FAULT_HW_DCOV ch:%d dcdc_ch:%d\n",
				RecvMsg->val[0], RecvMsg->val[1]);
			break;
		case MSG_MODULE_APP_FAULT_HW_MOD_FAULT:
			userlog(DEBUG_LOG, psName, "FAULT_HW_MOD_FAULT ch:%d dcdc_ch:%d\n",
				RecvMsg->val[0], RecvMsg->val[1]);
			break;
		//jhkw_170917e
		case MSG_MODULE_APP_CAN_DATA_DEBUG: //kjg_141114
			userlog(DEBUG_LOG, psName,
				"CAN_DATA_DEBUG ch:%d, index:%d, min:%d, max:%d\n",
				RecvMsg->val[0], RecvMsg->val[1],
				RecvMsg->val[2], RecvMsg->val[3]);
			break;
		case MSG_MODULE_APP_WRITE_CH_ATTRIBUTE: //kjhw_150216
			rtn = Write_ChAttribute();
			myData->mData.signal[C_SIG_EXT_CAN_CH_ATTRIBUTE] = P0;
			userlog(DEBUG_LOG, psName, "EXT_CAN_CH_ATTRIBUTE OK\n");
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Module to App Msg Unknown : %d %d %d\n",
				RecvMsg->msg, RecvMsg->val[0], RecvMsg->val[1]);
			break;
	}
}
