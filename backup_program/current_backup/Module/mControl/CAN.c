#include <rtl_core.h>
#include <asm/io.h>
#include <pthread.h>
#ifdef __CAN_FD__		//csk_190617s
#include "rt_can_ext.h"	//kjg_181121
#else
#include "../rt_can/rt_can.h"
#endif					//csk_190617e
#include "../../INC/datastore.h"
#include "common_utils.h"
#include "local_message.h"
#include "CAN.h"

extern S_SYSTEM_DATA	*myData;
extern S_MODULE_DATA	*myPs;

void CAN_Control1(int slot)
{
	unsigned char tmp;
	//kjh_211021 unsigned char delay; //kjhw_170905
	int index_offset, can_ch, start, end, module_ch, master_slave;
	int count, comm_state, rtn, max_count, i, function_div;
	S_RT_CAN_MSG can_msg;

	if(myData->AppControl.misc.Load_Process_CAN[0] != P1) return;
	if(slot >= myPs->config.installedCAN) return;

	start = slot;
	end = start + 1;
#ifdef __CAN_FD__ //kjh_190706s
	max_count = 80; //kjg_180405 40->80
	//max_count = 60; //ktg_210825
#else //CAN_2p0b
	max_count = 40;
#endif //kjh_190706e
	for(can_ch=start; can_ch < end; can_ch++) {
		if(can_ch >= myPs->config.installedCAN) return;

		switch(myData->mData.config.division_CAN) { //kjhw_140620
			case 1:
				module_ch = can_ch;
				master_slave = 0;
				if(master_slave == 0) index_offset = 0;
				else index_offset = myData->canReceiveDataCount[module_ch][0];
				break;
			default:
				switch(myData->AppControl.config.systemModel) { //kjhw_170629s
					case C_KBIA_500V_250A_100A_250KW:	 //shh_200916
					case C_JBTP_500V_250A_100A_500KW:	 //shh_200927
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
					case C_KCL_600V_200A_100A_480KW: 		//ktg_190812
					case C_KCL_600V_200A_100A_480KW_2: 		//ktg_190812
					case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
					case C_ADD_800V_350A_200A_50A_380KW:	//shh_200904
					case C_HYUNDAI_1000V_250A_100A_500KW:	//shh_200513
					case C_KOSTA_1000V_250A_100A_600KW:		//shh_201102
					case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
					case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
					case C_KTC_1500V_600A_200A_400KW:		//ktg_200807
						module_ch = can_ch / 2;
						if(module_ch == 1) module_ch = 2;
						master_slave = can_ch % 2;
						break;
					case C_SK_450V_200A_10A_360KW:
						module_ch = can_ch / 2;
						if(module_ch == 1) module_ch = 3;
						master_slave = can_ch % 2;
						break;
					default:
						module_ch = can_ch / 2;
						master_slave = can_ch % 2;
						break;
				}
				if(master_slave == 0) {
					index_offset = 0;
				} else {
					index_offset = myData->canReceiveDataCount[module_ch][0];
				} //kjhw_170629e
				break;
		}

		comm_state = 0;
		for(count=0; count < max_count; count++) {
			rtn = rt_can_read(can_ch, (char *)&can_msg);
			if(rtn < 0) {
				if(count == 0) comm_state = (-1);
				break;
			}
#ifdef __CAN_FD__
			/*	//csk_240301s
			if((count % 10) == 0) { //ktg_210825
				comm_state = rcv_can_data_parsing(can_ch, module_ch, master_slave,
					index_offset, (S_RT_CAN_MSG *)&can_msg);
			}
			*/		//csk_240301e
			comm_state = rcv_can_data_parsing(can_ch, module_ch, master_slave,
				index_offset, (S_RT_CAN_MSG *)&can_msg);	//csk_240301
#else //CAN_2p0b
			comm_state = rcv_can_data_parsing(can_ch, module_ch, master_slave,
				index_offset, (S_RT_CAN_MSG *)&can_msg);
#endif
		}

		if(can_ch == 0) { //kjg_120619
			for(i=0; i <myData->canReceiveDataCount[module_ch][master_slave];
				i++) {
				function_div = myData->canReceiveSetData
					.normalData[module_ch][i].function_div;
				switch(function_div) {
					case CAN_RX_FUNC_DIV_BMS_EOL_RX_TIME:
						myData->CanData[module_ch][i].f_val[0]
							= (float)((double)myData->CAN.rx_time[can_ch]
							/ 1000000.0);
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL_RX_TIME_MAX:
						myData->CanData[module_ch][i].f_val[0]
							= (float)((double)myData->CAN.rx_time[can_ch]
							/ 1000000.0);
						break;
					case CAN_RX_FUNC_DIV_BMS_EOL_RX_TIME_MIN:
						myData->CanData[module_ch][i].f_val[0]
							= (float)((double)myData->CAN.rx_time[can_ch]
							/ 1000000.0);
						break;
					default:
						break;
				}
			}
		}

		rtn = rcv_can_data_clear(can_ch, module_ch, master_slave, index_offset);
		if(rtn < 0) continue;

		if(comm_state < 0) {
		} else {
			myData->CAN.receive_time[can_ch] = myPs->misc.timer_1sec;
		}
		tmp = myData->cData[module_ch].misc.external_comm_state;
		//if((myPs->misc.timer_1sec - myData->CAN.receive_time[can_ch]) > 2) {
		if((myPs->misc.timer_1sec - myData->CAN.receive_time[can_ch])
			> myData->CAN.config.can_delay) {	//kjh_211021
			if(master_slave == 0) tmp |= 0x04;
			else tmp |= 0x08;
		} else {
			if(master_slave == 0) tmp &= ~(0x04);
			else tmp &= ~(0x08);
		}
		if(myData->mData.config.division_CAN == 1) { //kjhw_140620
			tmp |= 0x08;
		} else {
		}
		myData->cData[module_ch].misc.external_comm_state = tmp;
	}
}

int rcv_can_data_clear(int can_ch, int module_ch, int master_slave, int index_offset)
{
	int i, index, tmp, rtn=0;

	for(i=0; i < myData->canReceiveDataCount[module_ch][master_slave]; i++) {
		switch(myData->AppControl.config.systemModel) {
			case C_LGC_400V_60A_10A:
			case C_ROTEM_400V_60A_10A:
			//kjg_120221 case C_LGC_100V_200A_10A_20KW:
				tmp = 1;
				break;
			default:
				tmp = 0;
				break;
		}

		index = i + index_offset;
		if(myData->AppControl.config.debugType == 110) {
			if(myData->CanData[module_ch][index].f_val[0] <= 5000000.0) {
				myData->CanData[module_ch][index].f_val[0] += 100000.0;
			} else {
				myData->CanData[module_ch][index].f_val[0] = 100000.0;
			}
			rtn = -1;
			continue;
		}

		if(tmp == 1) {
			if(myData->dio.signal[DIO_SIG_IN_EXTERNAL1] == P0) {
				memset((char *)&myData->CanData[module_ch][index].c_val[0],
					0, 8);
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_DEBUG_ON] = P10;
				rtn = (-2);
			} else {
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_DEBUG_ON]
					>= P10) {
					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_DEBUG_ON]++;
					if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_DEBUG_ON]
						> P20) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_DEBUG_ON]
							= P1;
					}
				}
			}
		}
	}

	return rtn;
}

int rcv_can_data_parsing(int can_ch, int module_ch, int master_slave, int index_offset, S_RT_CAN_MSG *msg)
{
	unsigned char tmp_val, val, sign, function_phase;
	short int bit, startBit, bitCount, bitCount2, function_div, comp_count;
	int i, index, tmp, div, div2, comm_state, j, k, div3, div4;
	unsigned long tmp_val2, tmp_val3;
	long canID;
	short int startBit2, bitCount3;
	int tmp2; //kjhw_131204
	double compare_value; //kjhw_131204
	unsigned char byte_order2, data_type2; //kjhw_131204

	U_CAN_VAL can_val;//, can_val2;

	unsigned char comp_type, tmp_index; //kjg_141114_s
	unsigned char idx1, idx2, idx3; //kjhw_170303
	unsigned long tmp_l_val1, tmp_l_val2, tmp_l_val3; //kjhw_170303
	int func_div, func_div1, func_div2, func_div3; //kjhw_170303
	float tmp_f_val, tmp_f_val_o, tmp_f_val_n, comp_val; //kjhw_181220
	double tmp_d_val;
	S_MSG_VAL SendMsg; //kjg_141114_e

	unsigned char flag=0;	//csk_190808
	long checkComm_canID;	//csk_190808

	float ratioV, ratioI, ratioP; //kjh_211021

	canID = (long)msg->id;

	startBit2 = bitCount3 = 0;	//kjhw_131204
	byte_order2 = data_type2 = 0;	//kjhw_131204
	compare_value = 0;	//kjhw_131204

	//csk_190808s
	//LIN	(can_id : 0x1FF00012)
	//485	(can_id : 0x1EF00012)
	//SMbus	(can_id : 0x1FF2FF03)	//csk_190903
	if(myData->canTransmitSetData.commonData[module_ch][master_slave]
			.can_lin_flag == 1 ||
		myData->canReceiveSetData.commonData[module_ch][master_slave]
			.can_lin_flag == 1) { //lin to can
		flag = 1;
		checkComm_canID = 0x1FF00012;
	} else if(myData->canTransmitSetData.commonData[module_ch][master_slave]
			.can_lin_flag == 2 ||
		myData->canReceiveSetData.commonData[module_ch][master_slave]
			.can_lin_flag == 2) { //485 to can
		flag = 2;
		checkComm_canID = 0x1EF00012;
	} else if(myData->canTransmitSetData.commonData[module_ch][master_slave]
			.can_lin_flag == 3 ||
		myData->canReceiveSetData.commonData[module_ch][master_slave]
			.can_lin_flag == 3) { //SMbus to can
		flag = 3;
		checkComm_canID = 0x1FF2FF03;
	//ktg_211001s
	} else if(myData->canTransmitSetData.commonData[module_ch][master_slave]
			.can_lin_flag == 4 ||
		myData->canReceiveSetData.commonData[module_ch][master_slave]
			.can_lin_flag == 4) { //Modbus to can
		if(master_slave == 0) {	//master
			switch(myData->AppControl.config.systemModel) {
				//master - can  slave - modbus to can
				case C_LGC_250V_350A_100A_175KW:	//D10207
				case C_LGC_200V_150A_50A_60KW:	//D10196
					flag = 0;
					checkComm_canID = 0x00000000;	//ktg_211027
					break;
				default: //master - modbus to can  slave - modbus to can
					flag = 4;
					checkComm_canID = 0x1EF00012;
					break;
			}
		} else {	//slave
			flag = 4;	//ktg_211027
			checkComm_canID = 0x12000012;
		}	//ktg_211001e
	} else {		//CAN
		flag = 0;
		checkComm_canID = 0x00000000;
	}	//csk_190808e

	comm_state = 0;
	for(i=0; i < myData->canReceiveDataCount[module_ch][master_slave]; i++) {
		index = i + index_offset;

		if(myData->canReceiveSetData.normalData[module_ch][index].canID
			!= canID) {
			//if(canID != 0) comm_state = 1;
			if((canID != 0) && (flag == 0)) comm_state = 1; //kjhw_190820
			switch(myData->AppControl.config.systemModel) {
				case C_SNU_60V_200A_100A_50A_1CH_12KW: //kjhw_130228
					can_kokam_bms_data_parsing(module_ch, index);
					break;
				default: break;
			}
			continue;
		}

		tmp = 0;
		function_div = myData->canReceiveSetData
			.normalData[module_ch][index].function_div;
		function_phase = function_div % 50;
		if((function_div >= CAN_RX_FUNC_DIV_FAN_SPEED)
			&& (function_div < (CAN_RX_FUNC_DIV_FAN_SPEED + 50))) {
			if(myData->CAN.signal[can_ch][CAN_SIG_RX_PHASE_FAN_SPEED]
				!= function_phase) tmp = 1;
		} else if((function_div >= CAN_RX_FUNC_DIV_ECU_ID)
			&& (function_div < (CAN_RX_FUNC_DIV_ECU_ID + 50))) {
			tmp_val = myData->CAN.signal[can_ch][CAN_SIG_ECU_ID_REQUEST];
			if(tmp_val == P2) {
				if(function_phase == 1 && msg->data[0] == 0x10) {
				} else if(function_phase == 2 && msg->data[0] == 0x10) {
					myData->CAN.signal[can_ch][CAN_SIG_ECU_ID_REQUEST]++;
				} else tmp = 1;
			} else if(tmp_val == P4) {
				if(function_phase == 3 && msg->data[0] == 0x21) {
				} else if(function_phase == 4 && msg->data[0] == 0x21) {
				} else if(function_phase == 5 && msg->data[0] == 0x22) {
				} else if(function_phase == 6 && msg->data[0] == 0x22) {
				} else if(function_phase == 7 && msg->data[0] == 0x23) {
				} else if(function_phase == 8 && msg->data[0] == 0x23) {
				} else if(function_phase == 9 && msg->data[0] == 0x24) {
				} else if(function_phase == 10 && msg->data[0] == 0x24) {
				} else if(function_phase == 11 && msg->data[0] == 0x25) {
				} else if(function_phase == 12 && msg->data[0] == 0x25) {
				} else if(function_phase == 13 && msg->data[0] == 0x26) {
				} else if(function_phase == 14 && msg->data[0] == 0x26) {
				} else if(function_phase == 15 && msg->data[0] == 0x27) {
				} else if(function_phase == 16 && msg->data[0] == 0x27) {
				} else if(function_phase == 17 && msg->data[0] == 0x28) {
				} else if(function_phase == 18 && msg->data[0] == 0x28) {
					myData->CAN.signal[can_ch][CAN_SIG_ECU_ID_REQUEST]++;
				} else tmp = 1;
			} else if(tmp_val == P6) {
				if(function_phase == 19 && msg->data[0] == 0x29) {
				} else if(function_phase == 20 && msg->data[0] == 0x29) {
					myData->CAN.signal[can_ch][CAN_SIG_ECU_ID_REQUEST]++;
				} else tmp = 1;
			} else tmp = 1;
		}
		if(tmp == 1) continue;
		//comm_state = 1;
		if(flag == 0) comm_state = 1; //kjhw_190820

		//kjhw_131204s
		memset((char *)&can_val, 0, sizeof(U_CAN_VAL));
		//memset((char *)&myData->CanData[module_ch][index].c_val, 0, 8);

		startBit2 = myData->canReceiveSetData.normalData[module_ch][index]
			.startBit2;
		bitCount3 = myData->canReceiveSetData.normalData[module_ch][index]
			.bitCount2;
		byte_order2 = myData->canReceiveSetData.normalData[module_ch][index]
			.byte_order2;
		data_type2 = myData->canReceiveSetData.normalData[module_ch][index]
			.data_type2;
		compare_value = myData->canReceiveSetData.normalData[module_ch][index]
			.compare_value;

		tmp = 0;
		if((startBit2 == 0) && (bitCount3 == 0)) tmp = 1;

		if(tmp == 0) {
			bitCount2 = 0;
			div2 = 0;
			div3 = 0;
			div4 = 0;
			tmp_val = 0x01;
			val = 0x00;
			sign = 0x00;

			if(myData->canReceiveSetData.normalData[module_ch][index]
				.byte_order2 == 0) { //little_endian(intel)
				for(bit=0; bit < (startBit2 + bitCount3); bit++) {
					div = bit / 8;
					if((bit % 8) == 0) tmp_val = 0x01;
					if(bit >= startBit2) {
						val = val >> 1;
						if((tmp_val & msg->data[div]) != 0) {
							val |= 0x80;
							sign = 0x01;
						} else {
							sign = 0x00;
						}
						bitCount2++;
						if(bitCount2 >= 8) {
							can_val.uc_val[div2] = val;
							val = 0x00;
							div2++;
							bitCount2 = 0;
						}
					}
					tmp_val = tmp_val << 1;
				}
				if(bitCount2 != 0) {
					val = val >> (8 - bitCount2);
					can_val.uc_val[div2] = val;
				}
			} else { //big_endian(motorola)
				for(bit=0; bit < (startBit2 + bitCount3); bit++) {
					div = bit / 8;
					if((bit % 8) == 0) tmp_val = 0x01;
					if(bit >= startBit2) {
						if(bit == startBit2) {
							div3 = div;
							div4 = div;
						} else {
							if(div != div4) {
								div3 = div3 - 1;
								if(div3 < 0) div3 = 0;
								div4 = div;
							}
						}
						val = val >> 1;
						if((tmp_val & msg->data[div3]) != 0) {
							val |= 0x80;
							sign = 0x01;
						} else {
							sign = 0x00;
						}
						bitCount2++;
						if(bitCount2 >= 8) {
							can_val.uc_val[div2] = val;
							val = 0x00;
							div2++;
							bitCount2 = 0;
						}
					}
					tmp_val = tmp_val << 1;
				}
				if(bitCount2 != 0) {
					val = val >> (8 - bitCount2);
					can_val.uc_val[div2] = val;
				}
			}

			tmp2 = 0;
			switch(myData->canReceiveSetData.normalData[module_ch][index]
				.data_type2) {
				case 0: //unsigned
					if(bitCount3 <= 8) {
						if(myData->canReceiveSetData
							.normalData[module_ch][index].compare_value
							!= (float)can_val.uc_val[0]) {
							tmp2 = 1;
						}
					} else if(bitCount3 <= 32) {
						if(myData->canReceiveSetData
							.normalData[module_ch][index].compare_value
							!= (float)can_val.ul_val[0]) {
							tmp2 = 1;
						}
					}
					break;
				case 1: //signed //kkkkkkkkkkkkkkkkkkkkkkkkkkk
					if(sign == 0x00) {
						if(bitCount3 <= 32) {
							if(myData->canReceiveSetData
								.normalData[module_ch][index].compare_value
								!= (float)can_val.ul_val[0]) {
								tmp2 = 1;
							}
						} else {
						}
					} else {
						tmp_val2 = 0x7FFFFFFF;
						tmp_val3 = ~can_val.ul_val[0];
						for(bit=32; bit > bitCount3; bit--) {
							tmp_val3 &= tmp_val2;
							tmp_val2 = tmp_val2 >> 1;
						}
						tmp_val3 += 1; //kjhw_181115
						if(bitCount3 <= 32) {
							if(myData->canReceiveSetData
								.normalData[module_ch][index].compare_value
								!= (float)tmp_val3 * (-1.0)) {
								tmp2 = 1;
							}
						} else {
						}
					}
					break;
				case 2: //float
					if(bitCount3 <= 32) {
						if(myData->canReceiveSetData
							.normalData[module_ch][index].compare_value
							!= can_val.f_val[0]) {
							tmp2 = 1;
						}
					} else {
					}
					break;
				default:
					memcpy((char *)&myData->CanData[module_ch][index].c_val[0],
						(char *)&can_val.c_val[0], sizeof(U_CAN_VAL));
					break;
			}

			if(tmp2 == 1) {
				//memset((char *)&can_val, 0, sizeof(U_CAN_VAL));
				//memset((char *)&myData->CanData[module_ch][index].c_val, 0, 8);
				continue;
			}
		}	//kjhw_131204e

		memset((char *)&can_val, 0, sizeof(U_CAN_VAL));
		memset((char *)&myData->CanData[module_ch][index].c_val, 0, sizeof(U_CAN_VAL)); //kjg_180405 8->sizeof(U_CAN_VAL)

		startBit = myData->canReceiveSetData.normalData[module_ch][index]
			.startBit;
		bitCount = myData->canReceiveSetData.normalData[module_ch][index]
			.bitCount;

		bitCount2 = 0;
		div2 = 0;
		div3 = 0;
		div4 = 0;
		tmp_val = 0x01;
		val = 0x00;
		sign = 0x00;

		//kjg_110908
		if(myData->canReceiveSetData.normalData[module_ch][index].byte_order
			== 0) { //little_endian(intel)
			for(bit=0; bit < (startBit + bitCount); bit++) {
				div = bit / 8;
				if((bit % 8) == 0) tmp_val = 0x01;
				if(bit >= startBit) {
					val = val >> 1;
					if((tmp_val & msg->data[div]) != 0) {
						val |= 0x80;
						sign = 0x01;
					} else {
						sign = 0x00;
					}
					bitCount2++;
					if(bitCount2 >= 8) {
						can_val.uc_val[div2] = val;
						val = 0x00;
						div2++;
						bitCount2 = 0;
					}
				}
				tmp_val = tmp_val << 1;
			}
			if(bitCount2 != 0) {
				val = val >> (8 - bitCount2);
				can_val.uc_val[div2] = val;
			}
		} else { //big_endian(motorola)
			for(bit=0; bit < (startBit + bitCount); bit++) {
				div = bit / 8;
				if((bit % 8) == 0) tmp_val = 0x01;
				if(bit >= startBit) {
					if(bit == startBit) {
						div3 = div;
						div4 = div;
					} else {
						if(div != div4) {
							div3 = div3 - 1;
							if(div3 < 0) div3 = 0;
							div4 = div;
						}
					}
					val = val >> 1;
					if((tmp_val & msg->data[div3]) != 0) {
						val |= 0x80;
						sign = 0x01;
					} else {
						sign = 0x00;
					}
					bitCount2++;
					if(bitCount2 >= 8) {
						can_val.uc_val[div2] = val;
						val = 0x00;
						div2++;
						bitCount2 = 0;
					}
				}
				tmp_val = tmp_val << 1;
			}
			if(bitCount2 != 0) {
				val = val >> (8 - bitCount2);
				can_val.uc_val[div2] = val;
			}
		}

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
				comp_type = 1;
				break;
			case C_LGC_450V_250A_50A_225KW:	//kjhw_170303
			case C_LGC_450V_250A_50A_225KW_2: //kjhw_170601
			case C_UTP_600V_300A_100A_150KW:	//jhk_151107
				comp_type = 2;
				break;
			default:
				comp_type = 0;
				break;
		}

		if(comp_type == 0) {
			switch(myData->canReceiveSetData
				.normalData[module_ch][index].data_type) {
				case 0: //unsigned
					if(bitCount <= 32) {
						myData->CanData[module_ch][index].f_val[0]
							= (float)can_val.ul_val[0]
							* myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					} else {
						myData->CanData[module_ch][index].d_val[0]
							= can_val.d_val[0]
							* myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					}
					break;
				case 1: //signed
					if(sign == 0x00) {
						if(bitCount <= 32) {
							myData->CanData[module_ch][index].f_val[0]
								= (float)can_val.ul_val[0]
								* myData->canReceiveSetData
									.normalData[module_ch][index].factor
								+ myData->canReceiveSetData
									.normalData[module_ch][index].offset;
						} else {
							myData->CanData[module_ch][index].d_val[0]
								= can_val.d_val[0]
								* myData->canReceiveSetData
									.normalData[module_ch][index].factor
								+ myData->canReceiveSetData
									.normalData[module_ch][index].offset;
						}
					} else {
						tmp_val2 = 0x7FFFFFFF;
						tmp_val3 = ~can_val.ul_val[0];
						for(bit=32; bit > bitCount; bit--) {
							tmp_val3 &= tmp_val2;
							tmp_val2 = tmp_val2 >> 1;
						}
						tmp_val3 += 1; //kjhw_181115
						if(bitCount <= 32) {
							myData->CanData[module_ch][index].f_val[0]
								= (float)tmp_val3 * (-1.0)
								* myData->canReceiveSetData
									.normalData[module_ch][index].factor
								+ myData->canReceiveSetData
									.normalData[module_ch][index].offset;
						} else {
							myData->CanData[module_ch][index].d_val[0]
								= (double)tmp_val3 * (-1.0)
								* myData->canReceiveSetData
									.normalData[module_ch][index].factor
								+ myData->canReceiveSetData
									.normalData[module_ch][index].offset;
						}
					}
					break;
				case 2: //float
					if(bitCount <= 32) {
						myData->CanData[module_ch][index].f_val[0]
							= can_val.f_val[0]
							* (float)myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ (float)myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					} else {
						myData->CanData[module_ch][index].d_val[0]
							= can_val.d_val[0]
							* myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					}
					break;
				case 3: //string					//kjhw_140811s
					memcpy((char *)&myData->CanData[module_ch][index].c_val[0],
						(char *)&can_val.c_val[0], sizeof(U_CAN_VAL));
					break;
				case 4: //hex
					if(bitCount <= 32) {
						myData->CanData[module_ch][index].f_val[0]
							= (float)can_val.ul_val[0]
							* myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					} else {
						myData->CanData[module_ch][index].d_val[0]
							= can_val.d_val[0]
							* myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					}
					break;
				default:
					memcpy((char *)&myData->CanData[module_ch][index].c_val[0],
						(char *)&can_val.c_val[0], sizeof(U_CAN_VAL));
					break;
			}	//kjhw_140811e
		//} else { //kjg_141114
		} else if(comp_type == 1) { //kjhw_170303
			j = (int)myData->canReceiveSetData
				.normalData[module_ch][0].sent_period;
			if(j == 0) {
				j = 3;
			} else {
				j /= 10;
				if(j < 3) j = 3;
				else if(j > 9) j = 9;
			}
			myData->tmpCanData_sort_count[module_ch] = (unsigned char)j;

			tmp_index = myData->tmpCanData_index[module_ch][index];
			myData->tmpCanData_index[module_ch][index]++;
			if(myData->tmpCanData_index[module_ch][index]
				> myData->tmpCanData_sort_count[module_ch]) {
				myData->tmpCanData_index[module_ch][index] = 0;
			}

			switch(myData->canReceiveSetData
				.normalData[module_ch][index].data_type) {
				case 0: //unsigned
					if(bitCount <= 32) {
						myData->tmpCanData[module_ch][index][tmp_index].f_val[0]
							= (float)can_val.ul_val[0]
							* myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					} else {
						myData->tmpCanData[module_ch][index][tmp_index]
							.d_val[0] = can_val.d_val[0]
							* myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					}
					break;
				case 1: //signed
					if(sign == 0x00) {
						if(bitCount <= 32) {
							myData->tmpCanData[module_ch][index][tmp_index]
								.f_val[0] = (float)can_val.ul_val[0]
								* myData->canReceiveSetData
									.normalData[module_ch][index].factor
								+ myData->canReceiveSetData
									.normalData[module_ch][index].offset;
						} else {
							myData->tmpCanData[module_ch][index][tmp_index]
								.d_val[0] = can_val.d_val[0]
								* myData->canReceiveSetData
									.normalData[module_ch][index].factor
								+ myData->canReceiveSetData
									.normalData[module_ch][index].offset;
						}
					} else {
						tmp_val2 = 0x7FFFFFFF;
						tmp_val3 = ~can_val.ul_val[0];
						for(bit=32; bit > bitCount; bit--) {
							tmp_val3 &= tmp_val2;
							tmp_val2 = tmp_val2 >> 1;
						}
						tmp_val3 += 1; //kjhw_181115
						if(bitCount <= 32) {
							myData->tmpCanData[module_ch][index][tmp_index]
								.f_val[0] = (float)tmp_val3 * (-1.0)
								* myData->canReceiveSetData
									.normalData[module_ch][index].factor
								+ myData->canReceiveSetData
									.normalData[module_ch][index].offset;
						} else {
							myData->tmpCanData[module_ch][index][tmp_index]
								.d_val[0] = (double)tmp_val3 * (-1.0)
								* myData->canReceiveSetData
									.normalData[module_ch][index].factor
								+ myData->canReceiveSetData
									.normalData[module_ch][index].offset;
						}
					}
					break;
				case 2: //float
					if(bitCount <= 32) {
						myData->tmpCanData[module_ch][index][tmp_index].f_val[0]
							= can_val.f_val[0]
							* (float)myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ (float)myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					} else {
						myData->tmpCanData[module_ch][index][tmp_index]
							.d_val[0]
							= can_val.d_val[0] * myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					}
					break;
				default:
					memcpy((char *)&myData->
						tmpCanData[module_ch][index][tmp_index].c_val[0],
						(char *)&can_val.c_val[0], sizeof(U_CAN_VAL));
					break;
			}

			switch(myData->canReceiveSetData
				.normalData[module_ch][index].data_type) {
				case 0: //unsigned
				case 1: //signed
				case 2: //float
					if(bitCount <= 32) {
						if(myData->tmpCanData_count[module_ch][index]
							< myData->tmpCanData_sort_count[module_ch]) {
							myData->CanData[module_ch][index].f_val[0]
								= myData->tmpCanData[module_ch][index]
								[tmp_index].f_val[0];
						} else {
							tmp_f_val = myData->tmpCanData
								[module_ch][index][0].f_val[0];
							for(k=0; k < myData->tmpCanData_sort_count
								[module_ch]; k++) {
								for(j=0; j < myData->tmpCanData_sort_count
									[module_ch]; j++) {
									if(myData->tmpCanData_sort_flag[module_ch]
										[index][j] == 0) {
										tmp_f_val = myData->tmpCanData
											[module_ch][index][j].f_val[0];
										j = myData->tmpCanData_sort_count
											[module_ch];
									}
								}
								for(j=0; j < myData->tmpCanData_sort_count
									[module_ch]; j++) {
									if((myData->tmpCanData_sort_flag[module_ch]
										[index][j] == 0)
										&& (tmp_f_val
										>= myData->tmpCanData[module_ch][index]
										[j].f_val[0])) { //min
										tmp_f_val = myData->tmpCanData
											[module_ch][index][j].f_val[0];
									}
								}
								for(j=0; j < myData->tmpCanData_sort_count
									[module_ch]; j++) {
									if((myData->tmpCanData_sort_flag[module_ch]
										[index][j] == 0)
										&& (tmp_f_val
										== myData->tmpCanData[module_ch][index]
										[j].f_val[0])) { //min
										myData->tmpCanData_sort_flag[module_ch]
											[index][j] = 1;
										myData->tmpCanData_sort[module_ch]
											[index][k].f_val[0]
											= myData->tmpCanData[module_ch]
											[index][j].f_val[0];
										j = myData->tmpCanData_sort_count
											[module_ch];
									}
								}
							}
							for(j=0; j < myData->tmpCanData_sort_count
								[module_ch]; j++) {
								myData->tmpCanData_sort_flag[module_ch]
									[index][j] = 0;
							}
							k = myData->tmpCanData_sort_count[module_ch] / 2;
							myData->CanData[module_ch][index].f_val[0]
								= myData->tmpCanData_sort[module_ch][index]
								[k].f_val[0];

							k = myData->tmpCanData_sort_count[module_ch] - 1;
							if((myData->tmpCanData_sort[module_ch][index][0]
								.f_val[0] < myData->CanData[module_ch][index]
								.f_val[0] / 2.0)
								|| (myData->tmpCanData_sort[module_ch][index]
								[k].f_val[0] > myData->CanData[module_ch]
								[index].f_val[0] * 2.0)) { //kjg_d
								memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
								SendMsg.msg = MSG_MODULE_APP_CAN_DATA_DEBUG;
								SendMsg.val[0] = module_ch;
								SendMsg.val[1] = index;
								SendMsg.val[2] = (int)myData->tmpCanData_sort
									[module_ch][index][0].f_val[0];
								SendMsg.val[3] = (int)myData->tmpCanData_sort
									[module_ch][index][k].f_val[0];
								send_msg(MODULE_TO_APP, (char *)&SendMsg);
							}
						}
					} else {
						if(myData->tmpCanData_count[module_ch][index]
							< myData->tmpCanData_sort_count[module_ch]) {
							myData->CanData[module_ch][index].d_val[0]
								= myData->tmpCanData[module_ch][index]
								[tmp_index].d_val[0];
						} else {
							tmp_d_val = myData->tmpCanData
								[module_ch][index][0].d_val[0];
							for(k=0; k < myData->tmpCanData_sort_count
								[module_ch]; k++) {
								for(j=0; j < myData->tmpCanData_sort_count
									[module_ch]; j++) {
									if(myData->tmpCanData_sort_flag[module_ch]
										[index][j] == 0) {
										tmp_d_val = myData->tmpCanData
											[module_ch][index][j].d_val[0];
										j = myData->tmpCanData_sort_count
											[module_ch];
									}
								}
								for(j=0; j < myData->tmpCanData_sort_count
									[module_ch]; j++) {
									if((myData->tmpCanData_sort_flag[module_ch]
										[index][j] == 0)
										&& (tmp_d_val
										>= myData->tmpCanData[module_ch][index]
										[j].d_val[0])) { //min
										tmp_d_val = myData->tmpCanData
											[module_ch][index][j].d_val[0];
									}
								}
								for(j=0; j < myData->tmpCanData_sort_count
									[module_ch]; j++) {
									if((myData->tmpCanData_sort_flag[module_ch]
										[index][j] == 0)
										&& (tmp_d_val
										== myData->tmpCanData[module_ch][index]
										[j].d_val[0])) { //min
										myData->tmpCanData_sort_flag[module_ch]
											[index][j] = 1;
										myData->tmpCanData_sort[module_ch]
											[index][k].d_val[0]
											= myData->tmpCanData[module_ch]
											[index][j].d_val[0];
										j = myData->tmpCanData_sort_count
											[module_ch];
									}
								}
							}
							for(j=0; j < myData->tmpCanData_sort_count
								[module_ch]; j++) {
								myData->tmpCanData_sort_flag[module_ch]
									[index][j] = 0;
							}
							k = myData->tmpCanData_sort_count[module_ch] / 2;
							myData->CanData[module_ch][index].d_val[0]
								= myData->tmpCanData_sort[module_ch][index]
								[k].d_val[0];

							k = myData->tmpCanData_sort_count[module_ch] - 1;
							if((myData->tmpCanData_sort[module_ch][index][0]
								.d_val[0] < myData->CanData[module_ch][index]
								.d_val[0] / 2.0)
								|| (myData->tmpCanData_sort[module_ch][index]
								[k].d_val[0] > myData->CanData[module_ch]
								[index].d_val[0] * 2.0)) { //kjg_d
								memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
								SendMsg.msg = MSG_MODULE_APP_CAN_DATA_DEBUG;
								SendMsg.val[0] = module_ch;
								SendMsg.val[1] = index;
								SendMsg.val[2] = (int)myData->tmpCanData_sort
									[module_ch][index][0].d_val;
								SendMsg.val[3] = (int)myData->tmpCanData_sort
									[module_ch][index][k].d_val;
								send_msg(MODULE_TO_APP, (char *)&SendMsg);
							}
						}
					}
					break;
				default:
					memcpy((char *)&myData->CanData[module_ch][index].c_val[0],
						(char *)&myData->tmpCanData[module_ch][index][tmp_index]
						.c_val[0], sizeof(U_CAN_VAL));
					break;
			}

			if(myData->tmpCanData_count[module_ch][index] < 4) {
				myData->tmpCanData_count[module_ch][index]++;
			}
		} else { //kjhw_170303s
			func_div = myData->canReceiveSetData
				.normalData[module_ch][index].function_div3;
			func_div1 = CAN_RX_FUNC_DIV_FILTER3;
			func_div2 = CAN_RX_FUNC_DIV_FILTER4;
			func_div3 = CAN_RX_FUNC_DIV_FILTER5;
			if(func_div == func_div1 || func_div == func_div2
				|| func_div == func_div3) {
				idx1 = myData->tmpCanData_index[module_ch][0];
				idx2 = myData->tmpCanData_index[module_ch][1];
				idx3 = myData->tmpCanData_index[module_ch][2];
				tmp_l_val1 = myData->tmpCanData[module_ch][idx1][0].ul_val[0];
				tmp_l_val2 = myData->tmpCanData[module_ch][idx2][1].ul_val[0];
				tmp_l_val3 = myData->tmpCanData[module_ch][idx3][2].ul_val[0];
				if((50 == tmp_l_val1)
					&& (260 == tmp_l_val2)
					&& (260 == tmp_l_val3)) {
				} else {
					if(func_div == func_div1) {
						myData->CanData[module_ch][index].f_val[0]
							= myData->tmpCanData[module_ch][index][3].f_val[0];
					} else if(func_div == func_div2) {
						myData->CanData[module_ch][index].f_val[0]
							= myData->tmpCanData[module_ch][index][4].f_val[0];
					} else if(func_div == func_div3) {
						myData->CanData[module_ch][index].f_val[0]
							= myData->tmpCanData[module_ch][index][5].f_val[0];
					}
					continue;
				}
			}
			switch(myData->canReceiveSetData
				.normalData[module_ch][index].data_type) {
				case 0: //unsigned
					if(bitCount <= 32) {
						myData->CanData[module_ch][index].f_val[0]
							= (float)can_val.ul_val[0]
							* myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ myData->canReceiveSetData
								.normalData[module_ch][index].offset;
						//kjhw_170303s
						if(func_div == CAN_RX_FUNC_DIV_FILTER) {
							myData->tmpCanData[module_ch][index][0].ul_val[0]
								= can_val.ul_val[0];
							myData->tmpCanData_index[module_ch][0] = index;
						} else if(func_div == CAN_RX_FUNC_DIV_FILTER1) {
							myData->tmpCanData[module_ch][index][1].ul_val[0]
								= can_val.ul_val[0];
							myData->tmpCanData_index[module_ch][1] = index;
						} else if(func_div == CAN_RX_FUNC_DIV_FILTER2) {
							myData->tmpCanData[module_ch][index][2].ul_val[0]
								= can_val.ul_val[0];
							myData->tmpCanData_index[module_ch][2] = index;
						}
						if(func_div == CAN_RX_FUNC_DIV_FILTER3) {
							myData->tmpCanData[module_ch][index][3].f_val[0]
								= myData->CanData[module_ch][index].f_val[0];
						} else if(func_div == CAN_RX_FUNC_DIV_FILTER4) {
							myData->tmpCanData[module_ch][index][4].f_val[0]
								= myData->CanData[module_ch][index].f_val[0];
						} else if(func_div == CAN_RX_FUNC_DIV_FILTER5) {
							myData->tmpCanData[module_ch][index][5].f_val[0]
								= myData->CanData[module_ch][index].f_val[0];
						}
						//kjhw_170303e
					} else {
						myData->CanData[module_ch][index].d_val[0]
							= can_val.d_val[0] * myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					}
					break;
				case 1: //signed
					if(sign == 0x00) {
						if(bitCount <= 32) {
							myData->CanData[module_ch][index].f_val[0]
								= (float)can_val.ul_val[0]
								* myData->canReceiveSetData
									.normalData[module_ch][index].factor
								+ myData->canReceiveSetData
									.normalData[module_ch][index].offset;
						} else {
							myData->CanData[module_ch][index].d_val[0]
								= can_val.d_val[0]
								* myData->canReceiveSetData
									.normalData[module_ch][index].factor
								+ myData->canReceiveSetData
									.normalData[module_ch][index].offset;
						}
					} else {
						tmp_val2 = 0x7FFFFFFF;
						tmp_val3 = ~can_val.ul_val[0];
						for(bit=32; bit > bitCount; bit--) {
							tmp_val3 &= tmp_val2;
							tmp_val2 = tmp_val2 >> 1;
						}
						tmp_val3 += 1; //kjhw_181115
						if(bitCount <= 32) {
							myData->CanData[module_ch][index].f_val[0]
								= (float)tmp_val3 * (-1.0)
								* myData->canReceiveSetData
									.normalData[module_ch][index].factor
								+ myData->canReceiveSetData
									.normalData[module_ch][index].offset;
						} else {
							myData->CanData[module_ch][index].d_val[0]
								= (double)tmp_val3 * (-1.0)
								* myData->canReceiveSetData
									.normalData[module_ch][index].factor
								+ myData->canReceiveSetData
									.normalData[module_ch][index].offset;
						}
					}
					break;
				case 2: //float
					if(bitCount <= 32) {
						myData->CanData[module_ch][index].f_val[0]
							= can_val.f_val[0]
							* (float)myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ (float)myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					} else {
						myData->CanData[module_ch][index].d_val[0]
							= can_val.d_val[0]
							* myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					}
					break;
				/*default:
					memcpy((char *)&myData->CanData[module_ch][index].c_val[0],
						(char *)&can_val.c_val[0], sizeof(U_CAN_VAL));
					break;
				*/	 //kjhw_140811
				//kjhw_140811s
				case 3: //string
					memcpy((char *)&myData->CanData[module_ch][index].c_val[0],
						(char *)&can_val.c_val[0], sizeof(U_CAN_VAL));
					break;
				case 4: //hex
					if(bitCount <= 32) {
						myData->CanData[module_ch][index].f_val[0]
							= (float)can_val.ul_val[0]
							* myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					} else {
						myData->CanData[module_ch][index].d_val[0]
							= can_val.d_val[0] * myData->canReceiveSetData
								.normalData[module_ch][index].factor
							+ myData->canReceiveSetData
								.normalData[module_ch][index].offset;
					}
					break;
				default:
					memcpy((char *)&myData->CanData[module_ch][index].c_val[0],
						(char *)&can_val.c_val[0], sizeof(U_CAN_VAL));
					break;
			}
		} //kjhw_170303e

		if(flag != 0) {		//kjh_211005s
			if((canID == checkComm_canID)
				&& (myData->CanData[module_ch][index].f_val[0] >= 1.0)) {
				comm_state = 1;
			} else if((canID == checkComm_canID)
				&& (myData->CanData[module_ch][index].f_val[0] < 1.0)) {
				comm_state = 0;
			}
		}	//kjh_211005e

		if(master_slave != 0) continue;

		func_div3 = myData->canReceiveSetData
			.normalData[module_ch][index].function_div3;
		switch(func_div3) { //kjhw_181220s
			case CAN_RX_FUNC_DIV_TEMP_FILTER: //kjhw_181220
				if(myData->canReceiveSetData.normalData[module_ch][index]
					.factor < 1) {
					comp_val = 0.3;
				} else {
					comp_val = 10;
				}
				//jhkw_190128s
				if(myData->canReceiveSetData.normalData[module_ch][index]
					.bitCount == 1) {
					comp_val = 0.5;
					//comp_count = 3;
				} else if(myData->canReceiveSetData.normalData
					[module_ch][index].bitCount == 3) {
					comp_val = 1;
					//comp_count = 3;
				} else {
					//comp_count = 100;
				}
				comp_count = 5; //5s
				//jhkw_190128s
				tmp_f_val_n = myData->CanData[module_ch][index].f_val[0];
				tmp_f_val_o = myData->tmpCanData[module_ch][index][0].f_val[0];
				tmp_f_val = tmp_f_val_o - tmp_f_val_n;
				if(tmp_f_val < 0) tmp_f_val *= -1;
				if(tmp_f_val > comp_val) {
					myData->CanData[module_ch][index].f_val[0]
						= tmp_f_val_o;
					myData->tmpCanData_sort_flag[module_ch][index][0]++;
				} else {
					myData->tmpCanData[module_ch][index][0].f_val[0]
						= tmp_f_val_n;
					myData->tmpCanData_sort_flag[module_ch][index][0] = 0;
					myData->tmpCanData_sort_flag[module_ch][index][1] = 0;
					//kjhw_190131
				}
				/*if(myData->tmpCanData_sort_flag[module_ch][index][0]
					>= 200) {
					myData->tmpCanData_sort_flag[module_ch][index][0] = 0;
					myData->tmpCanData[module_ch][index][0].f_val[0]
						= tmp_f_val_n;
					myData->CanData[module_ch][index].f_val[0]
						= tmp_f_val_n;
				}*/
				if(myData->tmpCanData_sort_flag[module_ch][index][0]
					>= 100) { //1s
					myData->tmpCanData_sort_flag[module_ch][index][0] = 0;
					myData->tmpCanData_sort_flag[module_ch][index][1]++;
				}
				if(myData->tmpCanData_sort_flag[module_ch][index][1]
					>= comp_count) {
					myData->tmpCanData_sort_flag[module_ch][index][1] = 0;
					myData->tmpCanData[module_ch][index][0].f_val[0]
						= tmp_f_val_n;
					myData->CanData[module_ch][index].f_val[0]
						= tmp_f_val_n;
				} //kjhw_190131
				break;
			default: break;
		} //kjhw_181220e

		ratioV = myData->mData.patt_ratioV;	//kjh_220321
		ratioI = myData->mData.patt_ratioI;	//kjh_220321
		ratioP = myData->mData.patt_ratioP;	//kjh_220321

		switch(function_div) {
			case CAN_RX_FUNC_DIV_CAN_TH_TABLE_01:	//ktg_220614s
				myData->CanData[module_ch][index].f_val[0]
					= Convert_CAN_V_to_Temp(myData->CanData[module_ch][index].f_val[0],1);
				break;	//ktg_220614e
			case CAN_RX_FUNC_DIV_CAN_TH_TABLE_02:	//ktg_220614s
				myData->CanData[module_ch][index].f_val[0]
					= Convert_CAN_V_to_Temp(myData->CanData[module_ch][index].f_val[0],2);
				break;	//ktg_220614e
			case CAN_RX_FUNC_DIV_CAN_TH_TABLE_03:	//ktg_220614s
				myData->CanData[module_ch][index].f_val[0]
					= Convert_CAN_V_to_Temp(myData->CanData[module_ch][index].f_val[0],3);
				break;	//ktg_220614e
			case CAN_RX_FUNC_DIV_CAN_TH_TABLE_04:	//ktg_220614s
				myData->CanData[module_ch][index].f_val[0]
					= Convert_CAN_V_to_Temp(myData->CanData[module_ch][index].f_val[0],4);
				break;	//ktg_220614e
			case CAN_RX_FUNC_DIV_CAN_TH_TABLE_05:	//ktg_220614s
				myData->CanData[module_ch][index].f_val[0]
					= Convert_CAN_V_to_Temp(myData->CanData[module_ch][index].f_val[0],5);
				break;	//ktg_220614e
			case CAN_RX_FUNC_DIV_CAN_TH_TABLE_06:	//ktg_220614s
				myData->CanData[module_ch][index].f_val[0]
					= Convert_CAN_V_to_Temp(myData->CanData[module_ch][index].f_val[0],6);
				break;	//ktg_220614e
			case CAN_RX_FUNC_DIV_CAN_TH_TABLE_07:	//ktg_220614s
				myData->CanData[module_ch][index].f_val[0]
					= Convert_CAN_V_to_Temp(myData->CanData[module_ch][index].f_val[0],7);
				break;	//ktg_220614e
			case CAN_RX_FUNC_DIV_CAN_TH_TABLE_08:	//ktg_220614s
				myData->CanData[module_ch][index].f_val[0]
					= Convert_CAN_V_to_Temp(myData->CanData[module_ch][index].f_val[0],8);
				break;	//ktg_220614e
			case CAN_RX_FUNC_DIV_CAN_TH_TABLE_09:	//ktg_220614s
				myData->CanData[module_ch][index].f_val[0]
					= Convert_CAN_V_to_Temp(myData->CanData[module_ch][index].f_val[0],9);
				break;	//ktg_220614e
			case CAN_RX_FUNC_DIV_EXT_CHARGE_V:
				myData->testCond[module_ch].external_data[0].cmd_v[0]
					= (long)(myData->CanData[module_ch][index].f_val[0]
					* ratioV);
				break;
			case CAN_RX_FUNC_DIV_EXT_DISCHARGE_V:
				myData->testCond[module_ch].external_data[0].cmd_v[1]
					= (long)(myData->CanData[module_ch][index].f_val[0]
					* ratioV);
				break;
			case CAN_RX_FUNC_DIV_EXT_SET_VERSION:
				myData->testCond[module_ch].external_data[0].version
					= (long)myData->CanData[module_ch][index].f_val[0];
				break;
			case CAN_RX_FUNC_DIV_EXT_SET_MODE:
				myData->testCond[module_ch].external_data[0].mode
					= (long)myData->CanData[module_ch][index].f_val[0];
				break;
			case CAN_RX_FUNC_DIV_EXT_SET_WAVEFORM:
				myData->testCond[module_ch].external_data[0].waveform_type
					= (long)myData->CanData[module_ch][index].f_val[0];
				break;
			case CAN_RX_FUNC_DIV_EXT_CMD_I_START:
				myData->testCond[module_ch].external_data[0].cmd_i[0]
					= (long)(myData->CanData[module_ch][index].f_val[0]
					* ratioI);
				break;
			case CAN_RX_FUNC_DIV_EXT_CMD_I_END:
				myData->testCond[module_ch].external_data[0].cmd_i[1]
					= (long)(myData->CanData[module_ch][index].f_val[0]
					* ratioI);

				memcpy((char *)&myData->testCond[module_ch].external_data[1],
					(char *)&myData->testCond[module_ch].external_data[0],
					sizeof(S_TEST_COND_EXTERNAL_DATA));

				//kjg_140923_s
				for(j=0; j < MAX_SLAVE_CH; j++) {
					k = (int)myData->ChAttribute[module_ch].chNo_slave[j] - 1;
					if(k >= 0) {
						memcpy((char *)&myData->testCond[k].external_data[1],
							(char *)&myData->testCond[module_ch]
							.external_data[0],
							sizeof(S_TEST_COND_EXTERNAL_DATA));
						myData->CAN.signal[k * 2][CAN_SIG_SET_CONTROL] = P1;
					}
				}

				myData->CAN.signal[can_ch][CAN_SIG_SET_CONTROL] = P1;
				//kjg_140923_e
				break;
			case CAN_RX_FUNC_DIV_EXT_CMD_P_START:
				myData->testCond[module_ch].external_data[0].cmd_p[0]
					= (long)(myData->CanData[module_ch][index].f_val[0]
					* ratioP);
				break;
			case CAN_RX_FUNC_DIV_EXT_CMD_P_END:
				myData->testCond[module_ch].external_data[0].cmd_p[1]
					= (long)(myData->CanData[module_ch][index].f_val[0]
					* ratioP);

				memcpy((char *)&myData->testCond[module_ch].external_data[1],
					(char *)&myData->testCond[module_ch].external_data[0],
					sizeof(S_TEST_COND_EXTERNAL_DATA)); //kjhw_160517

				//kjg_140923_s
				for(j=0; j < MAX_SLAVE_CH; j++) {
					k = (int)myData->ChAttribute[module_ch].chNo_slave[j] - 1;
					if(k >= 0) {
						memcpy((char *)&myData->testCond[k].external_data[1],
							(char *)&myData->testCond[module_ch]
							.external_data[0],
							sizeof(S_TEST_COND_EXTERNAL_DATA));
						myData->CAN.signal[k * 2][CAN_SIG_SET_CONTROL] = P1;
					}
				}

				myData->CAN.signal[can_ch][CAN_SIG_SET_CONTROL] = P1;
				//kjg_140923_e
				break;
			case CAN_RX_FUNC_DIV_EXT_CMD_T:
				myData->testCond[module_ch].external_data[0].t_val
					= (long)myData->CanData[module_ch][index].f_val[0];
				break;
			case CAN_RX_FUNC_DIV_EXT_SET_CONTROL:
				myData->testCond[module_ch].external_data[0].control
					= (long)myData->CanData[module_ch][index].f_val[0];
				break;
			case CAN_RX_FUNC_DIV_EXT_RECEIVE_PARSING_PERIOD:
				myData->testCond[module_ch].external_data[0]
					.receive_parsing_period
					= (long)myData->CanData[module_ch][index].f_val[0];
				break;
			case CAN_RX_FUNC_DIV_EXT_CAN_TRANSMIT_PERIOD:
				myData->testCond[module_ch].external_data[0]
					.can_transmit_period
					= (long)myData->CanData[module_ch][index].f_val[0];
				break;
			case CAN_RX_FUNC_DIV_EXT_RESERVED_C3:
				memcpy((char *)&myData->testCond[module_ch].external_data[1],
					(char *)&myData->testCond[module_ch].external_data[0],
					sizeof(S_TEST_COND_EXTERNAL_DATA));

				for(j=0; j < MAX_SLAVE_CH; j++) {
					k = (int)myData->ChAttribute[module_ch].chNo_slave[j] - 1;
					if(k >= 0) {
						memcpy((char *)&myData->testCond[k].external_data[1],
							(char *)&myData->testCond[module_ch]
							.external_data[0],
							sizeof(S_TEST_COND_EXTERNAL_DATA));
						myData->CAN.signal[k * 2][CAN_SIG_SET_CONTROL] = P1;
					}
				}

				myData->CAN.signal[can_ch][CAN_SIG_SET_CONTROL] = P1;
				break;
			case CAN_RX_FUNC_DIV_EXT_FAULT_CLEAR:
				if((long)myData->CanData[module_ch][index].f_val[0] == 1) {
					if(myData->cData[module_ch].op.state == C_PAUSE
						&& myData->cData[module_ch]
						.signal[C_SIG_EXT_CAN_FAULT_CLEAR] == P0) {
						myData->cData[module_ch]
							.signal[C_SIG_EXT_CAN_FAULT_CLEAR] = P1;
						myData->cData[module_ch].signal[C_SIG_CMD_STOP] = P1;
					}
				}
				break;
			case CAN_RX_FUNC_DIV_EXT_E12_STAT_ERROR:
				if((long)myData->CanData[module_ch][index].f_val[0] != 0) {
					if(myData->cData[module_ch]
						.signal[C_SIG_EXT_CAN_E12_NM_STAT] == P1) {
						myData->cData[module_ch]
							.signal[C_SIG_EXT_CAN_E12_STAT_ERROR] = P1;
					}
				} else {
					myData->cData[module_ch]
						.signal[C_SIG_EXT_CAN_E12_STAT_ERROR] = P0;
				}
				break;
			case CAN_RX_FUNC_DIV_EXT_E48_STAT_ERROR:
				if((long)myData->CanData[module_ch][index].f_val[0] != 0) {
					if(myData->cData[module_ch]
						.signal[C_SIG_EXT_CAN_E48_NM_STAT] == P1) {
						myData->cData[module_ch]
							.signal[C_SIG_EXT_CAN_E48_STAT_ERROR] = P1;
					}
				} else {
					myData->cData[module_ch]
						.signal[C_SIG_EXT_CAN_E48_STAT_ERROR] = P0;
				}
				break;
			case CAN_RX_FUNC_DIV_EXT_CHM_STAT_ERROR:
				if((long)myData->CanData[module_ch][index].f_val[0] != 0) {
					if(myData->cData[module_ch]
						.signal[C_SIG_EXT_CAN_CHM_NM_STAT] == P1) {
						myData->cData[module_ch]
							.signal[C_SIG_EXT_CAN_CHM_STAT_ERROR] = P1;
					}
				} else {
					myData->cData[module_ch]
						.signal[C_SIG_EXT_CAN_CHM_STAT_ERROR] = P0;
				}
				break;
			case CAN_RX_FUNC_DIV_EXT_CHM_NM_STAT:
				myData->cData[module_ch].signal[C_SIG_EXT_CAN_CHM_NM_STAT]
					= (unsigned char)myData->CanData[module_ch][index].f_val[0];
				break;
			case CAN_RX_FUNC_DIV_EXT_E12_NM_STAT:
				myData->cData[module_ch].signal[C_SIG_EXT_CAN_E12_NM_STAT]
					= (unsigned char)myData->CanData[module_ch][index].f_val[0];
				break;
			case CAN_RX_FUNC_DIV_EXT_E48_NM_STAT:
				myData->cData[module_ch].signal[C_SIG_EXT_CAN_E48_NM_STAT]
					= (unsigned char)myData->CanData[module_ch][index].f_val[0];
				break;
			case CAN_RX_FUNC_DIV_EXT_PC_NM_STAT:
				myData->cData[module_ch].signal[C_SIG_EXT_CAN_NM_FLAG_1] = P1;
				myData->cData[module_ch].signal[C_SIG_EXT_CAN_NM_STAT_1]
					= (unsigned char)myData->CanData[module_ch][index].f_val[0];
				break;
			case CAN_RX_FUNC_DIV_EXT_RP_NM_STAT:
				myData->cData[module_ch].signal[C_SIG_EXT_CAN_NM_FLAG_2] = P1;
				myData->cData[module_ch].signal[C_SIG_EXT_CAN_NM_STAT_2]
					= (unsigned char)myData->CanData[module_ch][index].f_val[0];
				break; //kjg_140916_e
			//kjhw_150216s
			case CAN_RX_FUNC_DIV_EXT_PACK_ISOLATION:
				if(myData->cData[module_ch].op.state == C_RUN) break;
				myData->cData[module_ch].signal[C_SIG_EXT_CAN_PACK_ISOLATION]
					= (unsigned char)myData->CanData[module_ch][index].f_val[0];
				if(myData->cData[module_ch]
					.signal[C_SIG_EXT_CAN_PACK_ISOLATION] >= 2) {
					if(Read_InPoint(0, module_ch+1, I_OUT_PACK_ISOLATION)
						== OFF) {
						Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION, ON);
					}
				} else if(myData->cData[module_ch]
					.signal[C_SIG_EXT_CAN_PACK_ISOLATION] < 2) {
					if(Read_InPoint(0, module_ch+1, I_OUT_PACK_ISOLATION)
						== ON) {
						Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION, OFF);
					}
				}
				break;
			case CAN_RX_FUNC_DIV_EXT_CH_ATTRIBUTE:
				if(myData->mData.config.installedCh == 1) break;
				if(myData->cData[module_ch].op.state == C_RUN) break;
				if(module_ch != 0) break;
				if(myData->mData.signal[C_SIG_EXT_CAN_CH_ATTRIBUTE] != P0) {
					break;
				}
				if((unsigned char)myData->CanData[module_ch][index]
					.f_val[0] >= 2) {
					myData->ChAttribute[module_ch].opType = 1;
					myData->ChAttribute[module_ch].chNo_slave[0] = 2;
					myData->ChAttribute[module_ch+1].chNo_master = 0;
				} else {
					myData->ChAttribute[module_ch].opType = 0;
					myData->ChAttribute[module_ch].chNo_slave[0] = 0;
					myData->ChAttribute[module_ch+1].chNo_master = 2;
				}

				myData->mData.signal[C_SIG_EXT_CAN_CH_ATTRIBUTE] = P1;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_APP_WRITE_CH_ATTRIBUTE;
				//SendMsg.val[0] = module_ch;
				send_msg(MODULE_TO_APP, (char *)&SendMsg);
				break;	//kjhw_150216e
			case CAN_RX_FUNC_DIV_EXT_CH_DIV:	//jhkw_150611s
				if(myData->cData[module_ch].op.state == C_RUN) break;
				myData->cData[module_ch].signal[C_SIG_EXT_CAN_CH_DIV]
					= (unsigned char)myData->CanData[module_ch][index].f_val[0];
				if(myData->cData[module_ch]
					.signal[C_SIG_EXT_CAN_CH_DIV] == 0) {
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION2, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION3, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION4, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION5, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION6, OFF);
				} else if(myData->cData[module_ch]
					.signal[C_SIG_EXT_CAN_CH_DIV] == 1) {
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION2, ON);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION3, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION4, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION5, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION6, OFF);
				} else if(myData->cData[module_ch]
					.signal[C_SIG_EXT_CAN_CH_DIV] == 2) {
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION2, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION3, ON);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION4, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION5, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION6, OFF);
				} else if(myData->cData[module_ch]
					.signal[C_SIG_EXT_CAN_CH_DIV] == 3) {
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION2, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION3, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION4, ON);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION5, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION6, OFF);
				} else if(myData->cData[module_ch]
					.signal[C_SIG_EXT_CAN_CH_DIV] == 4) {
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION2, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION3, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION4, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION5, ON);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION6, OFF);
				} else if(myData->cData[module_ch]
					.signal[C_SIG_EXT_CAN_CH_DIV] == 5) {
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION2, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION3, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION4, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION5, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION6, ON);
				} else {
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION2, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION3, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION4, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION5, OFF);
					Select_OutPoint(0, module_ch+1, O_OUT_PACK_ISOLATION6, OFF);
				}
				break;
			case CAN_RX_FUNC_DIV_EXT_NEXTSTEP:	//jhkw_151106s
				myData->CAN.signal[can_ch][CAN_SIG_USER_NEXT]
					= (long)myData->CanData[module_ch][index].f_val[0];
				break;	//jhkw_151106e
			case CAN_RX_FUNC_DIV_FRAME_NUMBER_DATA1:	//kjhw_130228s
			case CAN_RX_FUNC_DIV_FRAME_NUMBER_DATA2:
			case CAN_RX_FUNC_DIV_FRAME_NUMBER_DATA3:
			case CAN_RX_FUNC_DIV_FRAME_NUMBER_DATA4:
			case CAN_RX_FUNC_DIV_FRAME_NUMBER_DATA5:
			case CAN_RX_FUNC_DIV_FRAME_NUMBER_DATA6:
			case CAN_RX_FUNC_DIV_FRAME_NUMBER_DATA7:
			case CAN_RX_FUNC_DIV_FRAME_NUMBER_DATA8:
				j = function_div - CAN_RX_FUNC_DIV_FRAME_NUMBER_DATA1;
				myData->CAN.frame_number_rx[can_ch].uc_val[j]
					= (unsigned char)myData->CanData[module_ch][index].f_val[0];
				if(can_ch == 0
					&& function_div == CAN_RX_FUNC_DIV_FRAME_NUMBER_DATA8) {
					if(myData->CAN.frame_number_rx[can_ch].uc_val[0] == 0x00) {
						myPs->bms_frame_data.frame_number[0]
							= (unsigned char)myData->CAN.frame_number_rx[can_ch]
							.uc_val[0];
						myPs->bms_frame_data.data[0]
							= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[1] * 256;
						myPs->bms_frame_data.data[0]
							+= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[2];
						myPs->bms_frame_data.data[1]
							= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[3] * 256;
						myPs->bms_frame_data.data[1]
							+= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[4];
						myPs->bms_frame_data.data[2]
							= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[5] * 256;
						myPs->bms_frame_data.data[2]
							+= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[6];
						myPs->bms_frame_data.data[3]
							= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[7] * 256;
					} else if(myData->CAN.frame_number_rx[can_ch].uc_val[0]
						== 0x01) {
						myPs->bms_frame_data.frame_number[1]
							= (unsigned char)myData->CAN.frame_number_rx[can_ch]
							.uc_val[0];
						myPs->bms_frame_data.data[3]
							+= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[1];
						myPs->bms_frame_data.data[4]
							= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[2] * 256;
						myPs->bms_frame_data.data[4]
							+= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[3];
						myPs->bms_frame_data.data[5]
							= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[4] * 256;
						myPs->bms_frame_data.data[5]
							+= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[5];
						myPs->bms_frame_data.data[6]
							= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[6] * 256;
						myPs->bms_frame_data.data[6]
							+= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[7];
					} else if(myData->CAN.frame_number_rx[can_ch].uc_val[0]
						== 0x02) {
						myPs->bms_frame_data.frame_number[2]
							= (unsigned char)myData->CAN.frame_number_rx[can_ch]
							.uc_val[0];
						myPs->bms_frame_data.data[7]
							= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[1] * 256;
						myPs->bms_frame_data.data[7]
							+= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[2];
						myPs->bms_frame_data.data[8]
							= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[3] * 256;
						myPs->bms_frame_data.data[8]
							+= (short int)myData->CAN.frame_number_rx[can_ch]
							.uc_val[4];
						myPs->bms_frame_data.flag
							= (unsigned char)myData->CAN.frame_number_rx[can_ch]
							.uc_val[5];
						myPs->bms_frame_data.Vmax_module_number_hi_nibble
							= (unsigned char)myData->CAN.frame_number_rx[can_ch]
							.uc_val[6] & 0xF0;
						myPs->bms_frame_data.Vmin_module_number_lo_nibble
							= (unsigned char)myData->CAN.frame_number_rx[can_ch]
							.uc_val[6] & 0x0F;
						myPs->bms_frame_data.Tmax_module_number_hi_nibble
							= (unsigned char)myData->CAN.frame_number_rx[can_ch]
							.uc_val[7] & 0xF0;
						myPs->bms_frame_data.Tmin_module_number_lo_nibble
							= (unsigned char)myData->CAN.frame_number_rx[can_ch]
							.uc_val[7] & 0x0F;
					}
				}
				break;	//kjhw_130228e
			case CAN_RX_FUNC_DIV_SUM_AMPARE_HOUR: //ktg_220829s
				myData->cData[module_ch].misc2.sum_CanAmpareHour
					= myData->CanData[module_ch][index].f_val[0] * 1000;
				break;
			case CAN_RX_FUNC_DIV_SUM_WATT_HOUR:
				myData->cData[module_ch].misc2.sum_CanWattHour
					= myData->CanData[module_ch][index].f_val[0] * 1000;
				break;	//ktg_220829e
			case CAN_RX_FUNC_DIV_UDS_DATA1:
			case CAN_RX_FUNC_DIV_UDS_DATA2:
			case CAN_RX_FUNC_DIV_UDS_DATA3:
			case CAN_RX_FUNC_DIV_UDS_DATA4:
			case CAN_RX_FUNC_DIV_UDS_DATA5:
			case CAN_RX_FUNC_DIV_UDS_DATA6:
			case CAN_RX_FUNC_DIV_UDS_DATA7:
			case CAN_RX_FUNC_DIV_UDS_DATA8:
				j = function_div - CAN_RX_FUNC_DIV_UDS_DATA1;
				myData->CAN.uds_rx[can_ch].uc_val[j]
					= (unsigned char)myData->CanData[module_ch][index].f_val[0];

				if(myData->canTransmitSetData.commonData[module_ch][0]
					.bms_type == 0) { //kjg_161207
					switch(myData->AppControl.config.systemModel) {
						case C_LGC_500V_200A_10A_200KW:
							if(can_ch == 0
								&& function_div == CAN_RX_FUNC_DIV_UDS_DATA8) {
								can_signal_bms_eol_phase(can_ch, module_ch,
									master_slave, index_offset, msg);

								can_signal_bms_eol_flash_phase(can_ch, module_ch,
									master_slave, index_offset, msg);

								can_signal_eol_procedure(can_ch, module_ch,
									master_slave, index_offset, msg); //kjg_120709
							}
							break;
						case C_LGC_450V_200A_10A_4:
						case C_LGC_450V_200A_10A_5:
						case C_LGC_450V_200A_10A_6:
						case C_LGC_450V_200A_10A_180KW:
						case C_LGC_450V_200A_10A_180KW_2:
						case C_LGC_450V_200A_10A_180KW_3:	//kjhw_120827
						case C_LGC_450V_200A_10A_180KW_4:	//jhkw_130909
						case C_LGC_450V_200A_20A_180KW:
						case C_LGC_450V_200A_20A_180KW_2:
						case C_LGC_450V_250A_10A_225KW:	//jhk_140728
						case C_LGC_500V_250A_100A_50A_250KW: //kjhw_121119
						case C_LGC_450V_250A_50A_225KW:	//kjh_170221
						case C_LGC_450V_250A_50A_225KW_2:	//kjh_170221
						case C_KTL_500V_200A_10A_200KW_3:	//jhk_170125
						case C_UTP_600V_300A_100A_150KW:	//jhk_151107
						case C_KTL_1200V_300A_100A_300KW:	//kjh_171013
							//kjg_120425
							if(function_div == CAN_RX_FUNC_DIV_UDS_DATA8) {
								can_signal_bms_uds_phase(can_ch, module_ch,
									master_slave, index_offset, msg);
							}
							break;
						case C_LGC_70V_250A_10A_35KW_3: //kjhw_150803
						case C_LGC_70V_250A_10A_35KW_4: //kjhw_150803
						case C_LGC_70V_250A_10A_35KW_5:
						case C_LGC_70V_250A_10A_35KW_6:
						case C_LGC_70V_250A_10A_35KW_7: //kjhw_150803
						case C_LGC_70V_250A_10A_35KW_8: //kjhw_150803
						case C_LGC_70V_250A_10A_35KW_16:
						case C_LGC_70V_250A_10A_35KW_17:
						case C_LGC_70V_250A_10A_35KW_18:
							//kjh_130628
							//for audi pack
							if(function_div == CAN_RX_FUNC_DIV_UDS_DATA8) {
								can_signal_bms_uds_phase1(can_ch, module_ch,
									master_slave, index_offset, msg);
							}
							break;
						default:
							break;
					}
				} else if(myData->canTransmitSetData.commonData[module_ch][0]
					.bms_type == 50) { //kjg_161207 Porsche_PAG12V_BMS
					if(function_div == CAN_RX_FUNC_DIV_UDS_DATA8) {
						can_signal_bms_procedure_50(can_ch, module_ch,
							master_slave, index_offset, msg);
					}
				}
				break;
			case CAN_RX_FUNC_DIV_UDS_DATA1_2: //kjg_130314_s
			case CAN_RX_FUNC_DIV_UDS_DATA2_2:
			case CAN_RX_FUNC_DIV_UDS_DATA3_2:
			case CAN_RX_FUNC_DIV_UDS_DATA4_2:
			case CAN_RX_FUNC_DIV_UDS_DATA5_2:
			case CAN_RX_FUNC_DIV_UDS_DATA6_2:
			case CAN_RX_FUNC_DIV_UDS_DATA7_2:
			case CAN_RX_FUNC_DIV_UDS_DATA8_2:
				j = function_div - CAN_RX_FUNC_DIV_UDS_DATA1_2;
				myData->CAN.uds_rx[can_ch].uc_val[j]
					= (unsigned char)myData->CanData[module_ch][index].f_val[0];
				switch(myData->AppControl.config.systemModel) {
					case C_LGC_450V_200A_10A_4:
					case C_LGC_450V_200A_10A_6:
					case C_LGC_450V_200A_10A_180KW:
					case C_LGC_450V_200A_10A_180KW_2:
					case C_LGC_450V_200A_10A_180KW_3:	//kjg_130415
					case C_LGC_450V_200A_10A_180KW_4:	//jhkw_130909
					case C_LGC_450V_200A_20A_180KW:
					case C_LGC_450V_200A_20A_180KW_2:
					case C_LGC_450V_250A_10A_225KW:	//jhk_140728
					case C_KTL_500V_200A_10A_200KW_3:	//jhk_170125
						if(function_div == CAN_RX_FUNC_DIV_UDS_DATA8_2) {
							can_signal_bms_eol_phase(can_ch, module_ch,
								master_slave, index_offset, msg);

							can_signal_bms_eol_flash_phase(can_ch, module_ch,
								master_slave, index_offset, msg);

							can_signal_eol_procedure(can_ch, module_ch,
								master_slave, index_offset, msg);
						}
						break;
					default:
						break;
				}
				break; //kjg_130314_e
			//case CAN_RX_FUNC_DIV_BMS_LINK_POWER: //kjh_tttttt
			//	myData->CanData[module_ch][index].f_val[0]
			//		= (float)myData->test_val_l[1];
			//	break;
			default:
				break;
		}
	}

	if(comm_state == 0
		&& myData->canReceiveDataCount[module_ch][master_slave] > 0) {
		comm_state = -1;
	}

	return comm_state;
}

void can_signal_bms_eol_phase(int can_ch, int module_ch, int master_slave, int index_offset, S_RT_CAN_MSG *msg)
{
	int tmp, j, k, index2, l, m;

	switch(myPs->signal[M_SIG_BMS_EOL_PHASE]) {
		case P2:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x50
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x40
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x00
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x32
				&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x01
				&& myData->CAN.uds_rx[can_ch].uc_val[6] == 0xF4
				) {
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 10;
			}
			break;
		case P3:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xF1
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xB2
				) {
				myPs->bms_eol_data.cvtn_id
					= (short int)myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				myPs->bms_eol_data.cvtn_id
					+= (short int)myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P5:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xFD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x00
				) {
				myPs->bms_eol_data.sw_version[0]
					= myData->CAN.uds_rx[can_ch].uc_val[4];
				myPs->bms_eol_data.sw_version[1]
					= myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.sw_version[2]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myPs->bms_eol_data.sw_version[3]
					= myData->CAN.uds_rx[can_ch].uc_val[7];
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P7:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x59
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xFF
				) {
				myPs->bms_eol_data.DTC[0]
					= myData->CAN.uds_rx[can_ch].uc_val[4];
				myPs->bms_eol_data.DTC[1]
					= myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.DTC[2]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;

				myData->CAN.uds_rx_message_count = 0;
				myData->CAN.uds_rx_dtc_count = 0;
			}
			break;
		case P9:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x03
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x59
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x02
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xFF
				) {
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x59
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x02
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xFF
				) {
				myPs->bms_eol_data.DTC_List[0][0]
					= myData->CAN.uds_rx[can_ch].uc_val[4];
				myPs->bms_eol_data.DTC_List[0][1]
					= myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.DTC_List[0][2]
					= myData->CAN.uds_rx[can_ch].uc_val[6];

				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			} else if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x10) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] = 14;
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = P0;

				k = (int)(myData->CAN.uds_rx[can_ch].uc_val[0] & 0x0F) * 256;
				k += (int)myData->CAN.uds_rx[can_ch].uc_val[1];
				myData->CAN.uds_rx_message_count = (short int)((k - 6) / 7 + 1);

				myPs->bms_eol_data.DTC_List[0][0]
					= myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.DTC_List[0][1]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myPs->bms_eol_data.DTC_List[0][2]
					= myData->CAN.uds_rx[can_ch].uc_val[7];
			} else if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x20) {
				k = myPs->misc.dtc_data_index;
				myPs->misc.dtc_data_index += 7;
				memcpy((char *)&myPs->misc.dtc_data[k],
					(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);

				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;

				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== myData->CAN.uds_rx_message_count
					|| myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== 255) {
					l = 1;
					m = 0;
					for(k=1; k < myPs->misc.dtc_data_index; k++) {
						if(myPs->misc.dtc_data[k] == 0x00) {
							break;
						}
						if(m < 3) {
							myPs->bms_eol_data.DTC_List[l][m]
								= myPs->misc.dtc_data[k];
						}
						m++;
						if(m == 4) {
							l++;
							m = 0;
						}
					}
					myPs->signal[M_SIG_BMS_EOL_PHASE]++;
				}
			}
			break;
		case P11:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xF1
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xB6
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.module_delta_temp = (float)tmp * 10.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P13:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x46
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.thermistor_sensor
					= ((float)tmp * 0.01 - 50.0) * 1000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P15:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x3F
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.cell_delta_v = (float)tmp * 1000.0;

				for(j=0; j < myData->canReceiveDataCount
					[module_ch][master_slave]; j++) {
					index2 = j + index_offset;

					if(myData->canReceiveSetData.normalData
						[module_ch][index2].function_div
						== CAN_RX_FUNC_DIV_BMS_EOL_HVIL) {
						myPs->bms_eol_data.HvBattHvil_D_Fault
							= (unsigned char)myData->CanData
							[module_ch][index2].f_val[0];
					}
				}
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P19:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x0D
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.DID_480D_Pack_V[0] = (float)tmp * 10000.0;

				for(j=0; j < myData->canReceiveDataCount
					[module_ch][master_slave]; j++) {
					index2 = j + index_offset;

					if(myData->canReceiveSetData.normalData
						[module_ch][index2].function_div
						== CAN_RX_FUNC_DIV_BMS_EOL_HV_BATT_V) {
						myPs->bms_eol_data.HvBatt_U_Actl[0]
							= myData->CanData[module_ch][index2].f_val[0]
							* 1000000.0;
					}
				}
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P21:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x07
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Link_V[0] = (float)tmp * 10000.0;

				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P23:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x0E
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Charge_V[0] = (float)tmp * 10000.0;

				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P25:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x84
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Pack2_V[0] = (float)tmp * 100000.0;

				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P27:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x86
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Charger_Fuse_V[0] = (float)tmp * 10000.0;

				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P29:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x85
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Elac_Fuse_V[0] = (float)tmp * 10000.0;

				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P35:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x0D
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.DID_480D_Pack_V[1] = (float)tmp * 10000.0;

				for(j=0; j < myData->canReceiveDataCount
					[module_ch][master_slave]; j++) {
					index2 = j + index_offset;

					if(myData->canReceiveSetData.normalData
						[module_ch][index2].function_div
						== CAN_RX_FUNC_DIV_BMS_EOL_HV_BATT_V) {
						myPs->bms_eol_data.HvBatt_U_Actl[1]
							= myData->CanData[module_ch][index2].f_val[0]
							* 1000000.0;
					}
				}
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P37:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x07
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Link_V[1] = (float)tmp * 10000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P39:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x0E
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Charge_V[1] = (float)tmp * 10000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P41:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x84
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Pack2_V[1] = (float)tmp * 100000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P43:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x86
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Charger_Fuse_V[1] = (float)tmp * 10000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P45:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x85
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Elac_Fuse_V[1] = (float)tmp * 10000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P51:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x0D
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.DID_480D_Pack_V[2] = (float)tmp * 10000.0;

				for(j=0; j < myData->canReceiveDataCount
					[module_ch][master_slave]; j++) {
					index2 = j + index_offset;

					if(myData->canReceiveSetData.normalData
						[module_ch][index2].function_div
						== CAN_RX_FUNC_DIV_BMS_EOL_HV_BATT_V) {
						myPs->bms_eol_data.HvBatt_U_Actl[2]
							= myData->CanData[module_ch][index2].f_val[0]
							* 1000000.0;
					}
				}
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P53:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x07
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Link_V[2] = (float)tmp * 10000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P55:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x0E
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Charge_V[2] = (float)tmp * 10000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P57:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x84
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Pack2_V[2] = (float)tmp * 100000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P59:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x86
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Charger_Fuse_V[2] = (float)tmp * 10000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P61:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x85
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Elac_Fuse_V[2] = (float)tmp * 10000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P68:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x0D
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.DID_480D_Pack_V[3] = (float)tmp * 10000.0;

				for(j=0; j < myData->canReceiveDataCount
					[module_ch][master_slave]; j++) {
					index2 = j + index_offset;

					if(myData->canReceiveSetData.normalData
						[module_ch][index2].function_div
						== CAN_RX_FUNC_DIV_BMS_EOL_HV_BATT_V) {
						myPs->bms_eol_data.HvBatt_U_Actl[3]
							= myData->CanData[module_ch][index2].f_val[0]
							* 1000000.0;
					}
				}
				myPs->signal[M_SIG_BMS_EOL_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_DELAY2] = 0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P70:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x07
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Link_V[3] = (float)tmp * 10000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P72:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x0E
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Charge_V[3] = (float)tmp * 10000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P74:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x84
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Pack2_V[3] = (float)tmp * 100000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P76:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x86
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Charger_Fuse_V[3] = (float)tmp * 10000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P78:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x85
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.Elac_Fuse_V[3] = (float)tmp * 10000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P102:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x13
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256 * 256 * 256;
				tmp += (myData->CAN.uds_rx[can_ch].uc_val[5] * 256 * 256);
				tmp += (myData->CAN.uds_rx[can_ch].uc_val[6] * 256);
				tmp += myData->CAN.uds_rx[can_ch].uc_val[7];
				myPs->bms_eol_data.isolation_resistance = (float)tmp;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P104:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xFD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x03
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.isolation_Va = (float)tmp * 1000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P106:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xFD
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x04
				) {
				tmp = myData->CAN.uds_rx[can_ch].uc_val[4] * 256;
				tmp += myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data.isolation_Vb = (float)tmp * 1000.0;
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P151:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x50
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x40
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x00
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x32
				&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x01
				&& myData->CAN.uds_rx[can_ch].uc_val[6] == 0xF4
				) {
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P153:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
				&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
				) {
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P155:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x1C
				&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
				&& myData->CAN.uds_rx[can_ch].uc_val[6] == 0x00
				) {
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		case P161:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
				&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
				) {
				myPs->signal[M_SIG_BMS_EOL_PHASE]++;
			}
			break;
		default:
			break;
	}
}

void can_signal_bms_eol_flash_phase(int can_ch, int module_ch, int master_slave, int index_offset, S_RT_CAN_MSG *msg)
{
	int i, k, block_count, block_point, data_index;
	unsigned int chhb, chlb, A_reg, B_reg, seed, R_bytes;

	switch(myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]) {
		case P3:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x04
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xF1
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x62
				) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			}
			break;
		case P5:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x50
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x02
				) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			}
			break;
		case P7:
		case P9:
		case P11:
		case P13:
		case P15:
			if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x10) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] = 14;
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = P1;

				k = (int)(myData->CAN.uds_rx[can_ch].uc_val[0] & 0x0F) * 256;
				k += (int)myData->CAN.uds_rx[can_ch].uc_val[1];
				myData->CAN.uds_rx_message_count = (short int)(k / 7 + 1);

				if(myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] == P11) {
					memset((char *)&myPs->bms_eol_data2.pack_id[1][0], 0, 40);
					myPs->bms_eol_data2.pack_id[1][0]
						= myData->CAN.uds_rx[can_ch].uc_val[5];
					myPs->bms_eol_data2.pack_id[1][1]
						= myData->CAN.uds_rx[can_ch].uc_val[6];
					myPs->bms_eol_data2.pack_id[1][2]
						= myData->CAN.uds_rx[can_ch].uc_val[7];
				}
			} else if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x20) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;

				if(myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] == P11) {
					if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
						== 2) {
						memcpy((char *)&myPs->bms_eol_data2.pack_id[1][3],
							(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);
					} else {
						memcpy((char *)&myPs->bms_eol_data2.pack_id[1][10],
							(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);
					}
				}

				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== myData->CAN.uds_rx_message_count
					|| myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== 255) {
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
				}
			}
			break;
		case P17: //seed
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_KEY]
				!= P0) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_KEY] = P0;
			}

			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x67
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
				) {
				chhb = 0xFFFFFFFF;
				A_reg = 0x00C541A9;
				seed = myData->CAN.uds_rx[can_ch].uc_val[5] << 16;
				seed += myData->CAN.uds_rx[can_ch].uc_val[4] << 8;
				seed += myData->CAN.uds_rx[can_ch].uc_val[3];
				chlb = 0xFF000000 | seed;
				for(i=0; i < 32; i++) {
					B_reg = (chlb % 2) ^ (A_reg % 2);
					chlb >>= 1;
					A_reg >>= 1;
					if(B_reg != 0) {
						B_reg <<= 23;
						B_reg |= A_reg;
						B_reg ^= 0x109028;
						A_reg = B_reg;
					}
				}
				for(i=0; i < 32; i++) {
					B_reg = (chhb % 2) ^ (A_reg % 2);
					chhb >>= 1;
					A_reg >>= 1;
					if(B_reg != 0) {
						B_reg <<= 23;
						B_reg |= A_reg;
						B_reg ^= 0x109028;
						A_reg = B_reg;
					}
				}
				R_bytes = (A_reg << 12) & 0xFF0000;
				R_bytes |= (A_reg & 0xF000) | ((A_reg >> 12) & 0xF00);
				R_bytes |= ((A_reg << 4) & 0xF0) | ((A_reg >> 16) & 0x0F);
				R_bytes &= 0x00FFFFFF;

				myData->CAN.uds_tx[can_ch].uc_val[3] = (R_bytes >> 16) & 0xFF;
				myData->CAN.uds_tx[can_ch].uc_val[4] = (R_bytes >> 8 ) & 0xFF;
				myData->CAN.uds_tx[can_ch].uc_val[5] = R_bytes & 0xFF;
				myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			}
			break;
		case P19: //key
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_KEY]
				!= P0) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_KEY] = P0;
			}

			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x02
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x67
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x02
				) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			}
			break;
		case P21:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x30
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x00
				) {
				myData->CAN.uds_ff_count = 0; //kjg_120927
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;
			} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x04
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x74
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x20
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x0C
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x62
				) {
				block_count = myData->CAN.block_count;
				myData->CAN.block_point = myData->CAN
					.can_flash_data_block_point[block_count];
				myData->CAN.uds_ff_count = 0;
				myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			}
			break;
		case P23:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x30
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x00
				) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;
			} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x02
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x76
				&& myData->CAN.uds_rx[can_ch].uc_val[2]
				== (unsigned char)myData->CAN.uds_ff_count) {
				if(myData->CAN.uds_tx_block_size == 3170) {
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P22;
					myData->CAN.block_point -= 8;
				} else {
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
				}
			}
			break;
		case P25:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x03
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x77
				) {
				block_count = myData->CAN.block_count;
				block_point = myData->CAN
					.can_flash_data_block_size[block_count] + 8
					+ myData->CAN.can_flash_data_block_point[block_count];

				if(myData->CAN.uds_rx[can_ch].uc_val[2]
					== myData->CAN.can_flash_file[block_point]
					&& myData->CAN.uds_rx[can_ch].uc_val[3]
					== myData->CAN.can_flash_file[block_point+1]) {
					if((block_count+1)
						== myData->CAN.can_flash_data_total_block) {
						myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
					} else {
						myData->CAN.block_count++;
						myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P20;
					}
				}
			}
			break;
		case P27:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x30
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x00
				) {
				//myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
				myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P29;
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA3] = 12;
				myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY1] = 0;
				myPs->signal[M_SIG_BMS_EOL_FLASH_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
			}
			break;
		case P29:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x03
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x01
				&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x10
				) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			}
			break;
		case P31:
			if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x10) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] = 14;
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = P1;

				k = (int)(myData->CAN.uds_rx[can_ch].uc_val[0] & 0x0F) * 256;
				k += (int)myData->CAN.uds_rx[can_ch].uc_val[1];
				myData->CAN.uds_rx_message_count = (short int)(k / 7 + 1);

				memset((char *)&myPs->bms_eol_data2.pack_id[2][0], 0, 40);
				myPs->bms_eol_data2.pack_id[2][0]
					= myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data2.pack_id[2][1]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myPs->bms_eol_data2.pack_id[2][2]
					= myData->CAN.uds_rx[can_ch].uc_val[7];
			} else if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x20) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;

				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== 2) {
					memcpy((char *)&myPs->bms_eol_data2.pack_id[2][3],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);
				} else {
					memcpy((char *)&myPs->bms_eol_data2.pack_id[2][10],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);
				}

				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== myData->CAN.uds_rx_message_count
					|| myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== 255) {
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
				}
			}
			break;
		case P33:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x30
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x00
				) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			}
			break;
		case P36:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x03
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x6E
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xF1
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x8C
				) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x03
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x7F
				) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE] = P39;
			}
			break;
		case P102:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x04
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xF1
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x62
				) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			}
			break;
		case P104:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x50
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x02
				) {
				myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
			}
			break;
		case P106:
			if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x10) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] = 14;
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = P1;

				k = (int)(myData->CAN.uds_rx[can_ch].uc_val[0] & 0x0F) * 256;
				k += (int)myData->CAN.uds_rx[can_ch].uc_val[1];
				myData->CAN.uds_rx_message_count = (short int)(k / 7 + 1);

				memset((char *)&myPs->bms_eol_data2.pack_id[3][0], 0, 40);
				myPs->bms_eol_data2.pack_id[3][0]
					= myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->bms_eol_data2.pack_id[3][1]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myPs->bms_eol_data2.pack_id[3][2]
					= myData->CAN.uds_rx[can_ch].uc_val[7];
			} else if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x20) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;

				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== 2) {
					memcpy((char *)&myPs->bms_eol_data2.pack_id[3][3],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);
				} else {
					memcpy((char *)&myPs->bms_eol_data2.pack_id[3][10],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);
				}

				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== myData->CAN.uds_rx_message_count
					|| myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== 255) {
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
				}
			}
			break;
		case P111: //kjg_120817
			if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x10) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] = 14;
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = P1;

				k = (int)(myData->CAN.uds_rx[can_ch].uc_val[0] & 0x0F) * 256;
				k += (int)myData->CAN.uds_rx[can_ch].uc_val[1];
				myData->CAN.uds_rx_message_count = (short int)(k / 7 + 1);

				data_index = (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
				myPs->eol_data[data_index].string_value[0]
					= myData->CAN.uds_rx[can_ch].uc_val[5];
				myPs->eol_data[data_index].string_value[1]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myPs->eol_data[data_index].string_value[2]
					= myData->CAN.uds_rx[can_ch].uc_val[7];
			} else if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x20) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;

				data_index = (int)myPs->signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== 2) {
					memcpy((char *)&myPs->eol_data[data_index].string_value[3],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);
				} else if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== 3) {
					memcpy((char *)&myPs->eol_data[data_index].string_value[10],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);
				} else if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== 4) {
					memcpy((char *)&myPs->eol_data[data_index].string_value[17],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);
				} else {
					memcpy((char *)&myPs->eol_data[data_index].string_value[24],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);
				}

				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== myData->CAN.uds_rx_message_count
					|| myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== 255) {
					myPs->signal[M_SIG_BMS_EOL_FLASH_PHASE]++;
				}
			}
			break;
		default:
			break;
	}
}

void can_signal_bms_uds_phase(int can_ch, int module_ch, int master_slave, int index_offset, S_RT_CAN_MSG *msg)
{
	int k;

	switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]) {
		case P3:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x10
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x7E
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x61
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x41
				) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] = 14;
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;

				k = (int)(myData->CAN.uds_rx[can_ch].uc_val[0] & 0x0F) * 256;
				k += (int)myData->CAN.uds_rx[can_ch].uc_val[1];
				myData->CAN.bms_uds[can_ch].rx_message_count
					= (short int)((k - 6) / 7 + 1);

				myData->CAN.bms_uds[can_ch].cell_v1[0]
					= myData->CAN.uds_rx[can_ch].uc_val[4];
				myData->CAN.bms_uds[can_ch].cell_v1[1]
					= myData->CAN.uds_rx[can_ch].uc_val[5];
				myData->CAN.bms_uds[can_ch].cell_v1[2]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myData->CAN.bms_uds[can_ch].cell_v1[3]
					= myData->CAN.uds_rx[can_ch].uc_val[7];
				myData->CAN.bms_uds[can_ch].cell_v1_index = 4;
			} else if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x20) {
				k = myData->CAN.bms_uds[can_ch].cell_v1_index;
				myData->CAN.bms_uds[can_ch].cell_v1_index += 7;
				memcpy((char *)&myData->CAN.bms_uds[can_ch].cell_v1[k],
					(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);

				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;

				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== myData->CAN.bms_uds[can_ch].rx_message_count
					|| myData->CAN.signal[can_ch]
					[CAN_SIG_TX_PHASE_UDS_DATA_INDEX] == 255) {
					can_data_convert(can_ch, module_ch, 0); //cell_v1
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
				}
			}
			break;
		case P5:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x10
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x4A
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x61
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x42
				) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] = 14;
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;

				k = (int)(myData->CAN.uds_rx[can_ch].uc_val[0] & 0x0F) * 256;
				k += (int)myData->CAN.uds_rx[can_ch].uc_val[1];
				myData->CAN.bms_uds[can_ch].rx_message_count
					= (short int)((k - 6) / 7 + 1);

				myData->CAN.bms_uds[can_ch].cell_v2[0]
					= myData->CAN.uds_rx[can_ch].uc_val[4];
				myData->CAN.bms_uds[can_ch].cell_v2[1]
					= myData->CAN.uds_rx[can_ch].uc_val[5];
				myData->CAN.bms_uds[can_ch].cell_v2[2]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myData->CAN.bms_uds[can_ch].cell_v2[3]
					= myData->CAN.uds_rx[can_ch].uc_val[7];
				myData->CAN.bms_uds[can_ch].cell_v2_index = 4;
			} else if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x20) {
				k = myData->CAN.bms_uds[can_ch].cell_v2_index;
				myData->CAN.bms_uds[can_ch].cell_v2_index += 7;
				memcpy((char *)&myData->CAN.bms_uds[can_ch].cell_v2[k],
					(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);

				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;

				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== myData->CAN.bms_uds[can_ch].rx_message_count
					|| myData->CAN.signal[can_ch]
					[CAN_SIG_TX_PHASE_UDS_DATA_INDEX] == 255) {
					can_data_convert(can_ch, module_ch, 1); //cell_v2
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
				}
			}
			break;
		case P7:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x10
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x32
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x61
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x43
				) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] = 14;
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;

				k = (int)(myData->CAN.uds_rx[can_ch].uc_val[0] & 0x0F) * 256;
				k += (int)myData->CAN.uds_rx[can_ch].uc_val[1];
				myData->CAN.bms_uds[can_ch].rx_message_count
					= (short int)((k - 6) / 7 + 1);

				myData->CAN.bms_uds[can_ch].busbar_v[0]
					= myData->CAN.uds_rx[can_ch].uc_val[4];
				myData->CAN.bms_uds[can_ch].busbar_v[1]
					= myData->CAN.uds_rx[can_ch].uc_val[5];
				myData->CAN.bms_uds[can_ch].busbar_v[2]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myData->CAN.bms_uds[can_ch].busbar_v[3]
					= myData->CAN.uds_rx[can_ch].uc_val[7];
				myData->CAN.bms_uds[can_ch].busbar_v_index = 4;
			} else if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x20) {
				k = myData->CAN.bms_uds[can_ch].busbar_v_index;
				myData->CAN.bms_uds[can_ch].busbar_v_index += 7;
				memcpy((char *)&myData->CAN.bms_uds[can_ch].busbar_v[k],
					(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);

				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;

				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== myData->CAN.bms_uds[can_ch].rx_message_count
					|| myData->CAN.signal[can_ch]
					[CAN_SIG_TX_PHASE_UDS_DATA_INDEX] == 255) {
					can_data_convert(can_ch, module_ch, 2); //busbar_v
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
				}
			}
			break;
		case P9:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x10
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x4D
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x61
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x04
				) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] = 14;
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;

				k = (int)(myData->CAN.uds_rx[can_ch].uc_val[0] & 0x0F) * 256;
				k += (int)myData->CAN.uds_rx[can_ch].uc_val[1];
				myData->CAN.bms_uds[can_ch].rx_message_count
					= (short int)((k - 6) / 7 + 1);

				myData->CAN.bms_uds[can_ch].temp[0]
					= myData->CAN.uds_rx[can_ch].uc_val[4];
				myData->CAN.bms_uds[can_ch].temp[1]
					= myData->CAN.uds_rx[can_ch].uc_val[5];
				myData->CAN.bms_uds[can_ch].temp[2]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myData->CAN.bms_uds[can_ch].temp[3]
					= myData->CAN.uds_rx[can_ch].uc_val[7];
				myData->CAN.bms_uds[can_ch].temp_index = 4;
			} else if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x20) {
				k = myData->CAN.bms_uds[can_ch].temp_index;
				myData->CAN.bms_uds[can_ch].temp_index += 7;
				memcpy((char *)&myData->CAN.bms_uds[can_ch].temp[k],
					(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);

				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;

				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== myData->CAN.bms_uds[can_ch].rx_message_count
					|| myData->CAN.signal[can_ch]
					[CAN_SIG_TX_PHASE_UDS_DATA_INDEX] == 255) {
					can_data_convert(can_ch, module_ch, 3); //temp
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
				}
			}
			break;
			//kjhw_141110s
		case P11:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x10
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x1D
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x61
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x03
				) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] = 14;
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;

				k = (int)(myData->CAN.uds_rx[can_ch].uc_val[0] & 0x0F) * 256;
				k += (int)myData->CAN.uds_rx[can_ch].uc_val[1];
				myData->CAN.bms_uds[can_ch].rx_message_count
					= (short int)((k - 6) / 7 + 1);

				myData->CAN.bms_uds[can_ch].battery_soc[0]
					= myData->CAN.uds_rx[can_ch].uc_val[4];
				myData->CAN.bms_uds[can_ch].battery_soc[1]
					= myData->CAN.uds_rx[can_ch].uc_val[5];
				myData->CAN.bms_uds[can_ch].battery_soc[2]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myData->CAN.bms_uds[can_ch].battery_soc[3]
					= myData->CAN.uds_rx[can_ch].uc_val[7];
				myData->CAN.bms_uds[can_ch].battery_soc_index = 4;
			} else if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0) == 0x20) {
				k = myData->CAN.bms_uds[can_ch].battery_soc_index;
				myData->CAN.bms_uds[can_ch].battery_soc_index += 7;
				memcpy((char *)&myData->CAN.bms_uds[can_ch].battery_soc[k],
					(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);

				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;

				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== myData->CAN.bms_uds[can_ch].rx_message_count
					|| myData->CAN.signal[can_ch]
					[CAN_SIG_TX_PHASE_UDS_DATA_INDEX] == 255) {
					can_data_convert(can_ch, module_ch, 4); //battery_soc
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
				}
			}
			break;	//kjhw_141110e
		default:
			break;
	}
}

void can_signal_bms_uds_phase1(int can_ch, int module_ch, int master_slave, int index_offset, S_RT_CAN_MSG *msg)
{ //kjhw_130628s
	int k;
	unsigned short int val1;
	int i;

	switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]) {
		case P3:
		case P5:
		case P7:
		case P9:
		case P11:
		case P13:
		case P15:
		case P17:
		case P19:
		case P21:
		case P23:
		case P25:
		case P27:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x03
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x63
				) {
				val1 = (int)(myData->CAN.uds_rx[can_ch].uc_val[2] & 0xFF) * 256;
				val1 += (int)myData->CAN.uds_rx[can_ch].uc_val[3];

				i = (myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] / 2) - 1;
				k = myData->canReceiveDataCount[module_ch][0] + i;
				myData->CanData[module_ch][k].f_val[0] = (float)val1 * 0.001;
				//myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++; //kjh_test
			}
			break;
		default:
			break;
	}
} //kjhw_130628e

void can_signal_bms_procedure_50(int can_ch, int module_ch, int master_slave, int index_offset, S_RT_CAN_MSG *msg)
{ //kjg_161207
	int i, k, val1, index_offset2;

	index_offset2 = 8;
	switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]) {
		case P3:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x04
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x16
				) {
				val1 = (int)myData->CAN.uds_rx[can_ch].uc_val[5];

				i = index_offset2 + 0;
				k = myData->canReceiveDataCount[module_ch][0] + i;
				myData->CanData[module_ch][k].f_val[0] = (float)val1 - 85.0;

				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x7F) { //kjg_161216
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P5:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x04
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x17
				) {
				val1 = (int)myData->CAN.uds_rx[can_ch].uc_val[5];

				i = index_offset2 + 1;
				k = myData->canReceiveDataCount[module_ch][0] + i;
				myData->CanData[module_ch][k].f_val[0] = (float)val1 - 85.0;

				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x7F) { //kjg_161216
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P7:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x04
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x18
				) {
				val1 = (int)myData->CAN.uds_rx[can_ch].uc_val[5];

				i = index_offset2 + 2;
				k = myData->canReceiveDataCount[module_ch][0] + i;
				myData->CanData[module_ch][k].f_val[0] = (float)val1 - 85.0;

				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x7F) { //kjg_161216
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P9:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x04
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x19
				) {
				val1 = (int)myData->CAN.uds_rx[can_ch].uc_val[5];

				i = index_offset2 + 3;
				k = myData->canReceiveDataCount[module_ch][0] + i;
				myData->CanData[module_ch][k].f_val[0] = (float)val1 - 85.0;

				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x7F) { //kjg_161216
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P11:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x10
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x15
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0xD0
				&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x43
				) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;

				k = (int)myData->CAN.uds_rx[can_ch].uc_val[2];
				myData->CAN.bms_uds[can_ch].rx_message_count
					= (short int)((k + 6) / 6 - 1);

				myData->CAN.bms_uds[can_ch].cell_v1[0]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myData->CAN.bms_uds[can_ch].cell_v1[1]
					= myData->CAN.uds_rx[can_ch].uc_val[7];
				myData->CAN.bms_uds[can_ch].cell_v1_index = 2;
			} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& (myData->CAN.uds_rx[can_ch].uc_val[1] & 0xF0) == 0x20) {
				if(myData->CAN.uds_rx[can_ch].uc_val[1] == 0x21) {
					k = myData->CAN.bms_uds[can_ch].cell_v1_index;
					//myData->CAN.bms_uds[can_ch].cell_v1_index += 6;
					memcpy((char *)&myData->CAN.bms_uds[can_ch].cell_v1[k],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[2], 6);
				} else if(myData->CAN.uds_rx[can_ch].uc_val[1] == 0x22) {
					k = myData->CAN.bms_uds[can_ch].cell_v1_index + 6;
					memcpy((char *)&myData->CAN.bms_uds[can_ch].cell_v1[k],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[2], 6);
				} else if(myData->CAN.uds_rx[can_ch].uc_val[1] == 0x23) {
					k = myData->CAN.bms_uds[can_ch].cell_v1_index + 12;
					memcpy((char *)&myData->CAN.bms_uds[can_ch].cell_v1[k],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[2], 6);
					can_data_convert(can_ch, module_ch, 5); //bms meas.

					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
				}

				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;

				/*if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== myData->CAN.bms_uds[can_ch].rx_message_count) {
					can_data_convert(can_ch, module_ch, 5); //bms meas.

					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
				}*/
			} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x7F) { //kjg_161216
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			} else {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					>= 8) { //kjg_161213 16 -> 8
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
				}
			}
			break;
		case P13:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x04
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x4D
				) {
				val1 = (int)myData->CAN.uds_rx[can_ch].uc_val[5];

				i = index_offset2 + 12;
				k = myData->canReceiveDataCount[module_ch][0] + i;
				myData->CanData[module_ch][k].f_val[0] = (float)val1;

				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x7F) { //kjg_161216
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P15:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x7F) { //kjg_161216
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
				break;
			}

			if(myData->CAN.uds_rx[can_ch].uc_val[0] != 0x47) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== 8) { //kjg_161213 16 -> 8
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
				}
				break;
			}

			val1 = myData->CAN.uds_rx[can_ch].uc_val[1] & 0xF0;
			if(val1 == 0x00
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x40) {
				myData->CAN.bms_uds[can_ch].cell_v2[0]
					= myData->CAN.uds_rx[can_ch].uc_val[5];
				myData->CAN.bms_uds[can_ch].cell_v2[1]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myData->CAN.bms_uds[can_ch].cell_v2[2]
					= myData->CAN.uds_rx[can_ch].uc_val[7];

				can_data_convert(can_ch, module_ch, 6); //error monitor

				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			} else if(val1 == 0x10
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0xD0
				&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x40) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;

				k = (int)myData->CAN.uds_rx[can_ch].uc_val[2];
				myData->CAN.bms_uds[can_ch].rx_message_count
					= (short int)((k + 6) / 6 - 1);

				myData->CAN.bms_uds[can_ch].cell_v2[0]
					= myData->CAN.uds_rx[can_ch].uc_val[6];
				myData->CAN.bms_uds[can_ch].cell_v2[1]
					= myData->CAN.uds_rx[can_ch].uc_val[7];
				myData->CAN.bms_uds[can_ch].cell_v2_index = 2;
			} else if(val1 == 0x20) {
				if(myData->CAN.uds_rx[can_ch].uc_val[1] == 0x21) {
					k = myData->CAN.bms_uds[can_ch].cell_v2_index;
					//myData->CAN.bms_uds[can_ch].cell_v2_index += 6;
					memcpy((char *)&myData->CAN.bms_uds[can_ch].cell_v2[k],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[2], 6);

					if(myData->CAN.bms_uds[can_ch].rx_message_count == 1) {
						can_data_convert(can_ch, module_ch, 6); //error monitor

						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
				} else if(myData->CAN.uds_rx[can_ch].uc_val[1] == 0x22) {
					k = myData->CAN.bms_uds[can_ch].cell_v2_index + 6;
					//myData->CAN.bms_uds[can_ch].cell_v2_index += 6;
					memcpy((char *)&myData->CAN.bms_uds[can_ch].cell_v2[k],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[2], 6);

					if(myData->CAN.bms_uds[can_ch].rx_message_count == 2) {
						can_data_convert(can_ch, module_ch, 6); //error monitor

						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
				} else if(myData->CAN.uds_rx[can_ch].uc_val[1] == 0x23) {
					k = myData->CAN.bms_uds[can_ch].cell_v2_index + 12;
					//myData->CAN.bms_uds[can_ch].cell_v2_index += 6;
					memcpy((char *)&myData->CAN.bms_uds[can_ch].cell_v2[k],
						(char *)&myData->CAN.uds_rx[can_ch].uc_val[2], 6);

					//if(myData->CAN.bms_uds[can_ch].rx_message_count == 3) {
						can_data_convert(can_ch, module_ch, 6); //error monitor

						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					//}
				}

				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;

				/*if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== myData->CAN.bms_uds[can_ch].rx_message_count) {
					can_data_convert(can_ch, module_ch, 6); //error monitor

					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
				}*/
			} else {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
					== 8) { //kjg_161213 16 -> 8
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
				}
			}
			break;
		case P17:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x05
				&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x62
				&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
				&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x06
				) {
				val1 = ((int)myData->CAN.uds_rx[can_ch].uc_val[6] * 256);
				val1 += (int)myData->CAN.uds_rx[can_ch].uc_val[5];

				i = index_offset2 + 22;
				k = myData->canReceiveDataCount[module_ch][0] + i;
				myData->CanData[module_ch][k].f_val[0] = (float)val1 * 0.1;

				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x7F) { //kjg_161216
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P19:
			if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
				&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x7F) { //kjg_161216
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
				break;
			}

			switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_PROCEDURE_INDEX]) {
				case 1: //BMS WakeUp 0xD005, periodic transmit start
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x04
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x05
						) {
						val1 = myData->CAN.uds_rx[can_ch].uc_val[5];

						i = 0;
						k = myData->canReceiveDataCount[module_ch][0] + i;
						myData->CanData[module_ch][k].f_val[0] = (float)val1;

						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case 2: //Sleep 0x00 FF FF FF FF FF FF FF, periodic transmit stop
					break;
				case 3: //Relay ON 0xD045
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x03
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x6E
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x45
						) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case 4: //Relay OFF 0xD045
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x03
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x6E
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x45
						) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case 5: //Read deep sleep flag memory 0xD046
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x04
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x46
						) {
						val1 = myData->CAN.uds_rx[can_ch].uc_val[5];

						i = 4;
						k = myData->canReceiveDataCount[module_ch][0] + i;
						myData->CanData[module_ch][k].f_val[0] = (float)val1;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case 6: //BMS deep sleep 0xD046
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x03
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x6E
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x46
						) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case 7: //Relay ON(force) 0xD045 kjg_161213
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x03
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x6E
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x45
						) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case 8: //Clear error flag 0xD048 kjg_161214
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x47
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x03
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x6E
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xD0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x48
						) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}

void can_data_convert(int can_ch, int module_ch, int type)
{
	char tmp1[2];
	unsigned short int val1;
	int i, j, k, index_offset;
	long val2;

	switch(type) {
		case 0: //cell_v1
			for(i=0; i < 62; i++) {
				j = i * 2;
				tmp1[0] = myData->CAN.bms_uds[can_ch].cell_v1[j+1];
				tmp1[1] = myData->CAN.bms_uds[can_ch].cell_v1[j];
				memcpy((char *)&val1, (char *)&tmp1[0], 2);
				k = myData->canReceiveDataCount[module_ch][0] + i;
				myData->CanData[module_ch][k].f_val[0] = (float)val1 * 0.001;
			}
			break;
		case 1: //cell_v2
			for(i=62; i < 98; i++) {
				j = (i - 62) * 2;
				tmp1[0] = myData->CAN.bms_uds[can_ch].cell_v2[j+1];
				tmp1[1] = myData->CAN.bms_uds[can_ch].cell_v2[j];
				memcpy((char *)&val1, (char *)&tmp1[0], 2);
				k = myData->canReceiveDataCount[module_ch][0] + i;
				if(i < 96) {
					myData->CanData[module_ch][k].f_val[0]
						= (float)val1 * 0.001;
				} else {
					myData->CanData[module_ch][k].f_val[0] = (float)val1 * 0.01;
				}
			}
			break;
		case 2: //busbar_v
			for(i=0; i < 24; i++) {
				j = i * 2;
				tmp1[0] = myData->CAN.bms_uds[can_ch].busbar_v[j+1];
				tmp1[1] = myData->CAN.bms_uds[can_ch].busbar_v[j];
				memcpy((char *)&val1, (char *)&tmp1[0], 2);
				k = myData->canReceiveDataCount[module_ch][0] + i + 98 + 27;
				myData->CanData[module_ch][k].f_val[0] = (float)val1 * 0.001;
			}
			break;
		case 3: //temp
			for(i=0; i < 24; i++) {
				j = i * 3 + 2;
				tmp1[0] = myData->CAN.bms_uds[can_ch].temp[j];
				k = myData->canReceiveDataCount[module_ch][0] + i + 98;
				myData->CanData[module_ch][k].f_val[0] = (float)tmp1[0] - 40;
			}

			tmp1[0] = myData->CAN.bms_uds[can_ch].temp[j+1];
			k = myData->canReceiveDataCount[module_ch][0] + i + 98;
			myData->CanData[module_ch][k].f_val[0] = (float)tmp1[0] - 40;

			tmp1[0] = myData->CAN.bms_uds[can_ch].temp[j+2];
			k = myData->canReceiveDataCount[module_ch][0] + i + 98 + 1;
			myData->CanData[module_ch][k].f_val[0] = (float)tmp1[0] - 40;

			tmp1[0] = myData->CAN.bms_uds[can_ch].temp[j+3];
			k = myData->canReceiveDataCount[module_ch][0] + i + 98 + 2;
			myData->CanData[module_ch][k].f_val[0] = (float)tmp1[0] - 40;
			break;
		case 4: //kjhw_141110 battery_soc
			tmp1[0] = myData->CAN.bms_uds[can_ch].battery_soc[23];
			tmp1[1] = myData->CAN.bms_uds[can_ch].battery_soc[22];
			memcpy((char *)&val1, (char *)&tmp1[0], 2);
			k = myData->canReceiveDataCount[module_ch][0] + 98 + 27 + 24;
			myData->CanData[module_ch][k].f_val[0] = (float)val1 * 0.01;
			break;
		case 5: //kjg_161207 bms meas.
			index_offset = 8 + 4;
			for(i=0; i < 4; i++) {
				j = i * 2;
				tmp1[0] = myData->CAN.bms_uds[can_ch].cell_v1[j];
				tmp1[1] = myData->CAN.bms_uds[can_ch].cell_v1[j+1];
				memcpy((char *)&val1, (char *)&tmp1[0], 2);
				k = myData->canReceiveDataCount[module_ch][0] + index_offset + i;
				myData->CanData[module_ch][k].f_val[0] = (float)val1 * 0.001; //cell_v
			}

			j = 8;
			tmp1[0] = myData->CAN.bms_uds[can_ch].cell_v1[j];
			tmp1[1] = myData->CAN.bms_uds[can_ch].cell_v1[j+1];
			memcpy((char *)&val1, (char *)&tmp1[0], 2);
			k = myData->canReceiveDataCount[module_ch][0] + index_offset + 4;
			myData->CanData[module_ch][k].f_val[0] = (float)val1 * 0.001; //pack_v

			j = 10;
			tmp1[0] = myData->CAN.bms_uds[can_ch].cell_v1[j];
			tmp1[1] = myData->CAN.bms_uds[can_ch].cell_v1[j+1];
			memcpy((char *)&val1, (char *)&tmp1[0], 2);
			k = myData->canReceiveDataCount[module_ch][0] + index_offset + 5;
			myData->CanData[module_ch][k].f_val[0] = (float)val1 * 0.001; //fuse_v

			j = 12;
			tmp1[0] = myData->CAN.bms_uds[can_ch].cell_v1[j];
			tmp1[1] = myData->CAN.bms_uds[can_ch].cell_v1[j+1];
			memcpy((char *)&val1, (char *)&tmp1[0], 2);
			k = myData->canReceiveDataCount[module_ch][0] + index_offset + 6;
			myData->CanData[module_ch][k].f_val[0] = (float)val1 * 0.001; //terminal_v

			j = 14;
			val2 = ((long)myData->CAN.bms_uds[can_ch].cell_v1[j+2] * 65536);
			val2 += ((long)myData->CAN.bms_uds[can_ch].cell_v1[j+1] * 256);
			val2 += (long)myData->CAN.bms_uds[can_ch].cell_v1[j];
			k = myData->canReceiveDataCount[module_ch][0] + index_offset + 7;
			myData->CanData[module_ch][k].f_val[0] = (float)val2 * 0.001 - 2000.0; //current
			break;
		case 6: //kjg_161207 error monitor
			index_offset = 8 + 13;
			for(i=0; i < 9; i++) {
				val1 = myData->CAN.bms_uds[can_ch].cell_v2[i];
				k = myData->canReceiveDataCount[module_ch][0] + index_offset + i;
				myData->CanData[module_ch][k].f_val[0] = (float)val1;
			}
			break;
		default:
			break;
	}
}

void can_signal_eol_procedure(int can_ch, int module_ch, int master_slave, int index_offset, S_RT_CAN_MSG *msg)
{
	int data_index, k, l, m;

	switch(myPs->signal[M_SIG_EOL_PROCEDURE_INDEX]) {
		case P1: //EOL_MODE(1429)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x50
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x40
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x00
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x32
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[6] == 0xF4
						) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P2:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2: //HVIL_Return Check(1436)
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x04
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[6]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[7])
							* 7.446289;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				case P4: //AUX_HVIL_Return Check(1560)
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x04
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[1]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[6]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[7])
							* 7.446289;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P3: //CVTN_ID Assign(1401)
			break;
		case P4: //CVTN_ID Check(1402)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xEE
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x29
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].byte_value[0]
							= myData->CAN.uds_rx[can_ch].uc_val[4];
						myPs->eol_data[data_index].byte_value[1]
							= myData->CAN.uds_rx[can_ch].uc_val[5];
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P5: //SW_VERSION Check(1403)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDA
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x01
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].byte_value[0]
							= myData->CAN.uds_rx[can_ch].uc_val[4];
						myPs->eol_data[data_index].byte_value[1]
							= myData->CAN.uds_rx[can_ch].uc_val[5];
						myPs->eol_data[data_index].byte_value[2]
							= myData->CAN.uds_rx[can_ch].uc_val[6];
						myPs->eol_data[data_index].byte_value[3]
							= myData->CAN.uds_rx[can_ch].uc_val[7];
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P6: //Fine_Current_Sense_Wire Check(1438)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x04
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[6]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[7])
							* 12.207 / 1000.0 - 25.0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P7: //Coarse_Current_Sense_Wire Check(1439)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x04
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[6]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[7])
							* 305.176 / 1000.0 - 625.0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P8: //Disable_ISO(1431)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x1C
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						&& myData->CAN.uds_rx[can_ch].uc_val[6] == 0x00
						) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P9:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //Main+, Charger Precharge Close(1440)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P2:
							if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
								&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
								&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
								&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
								&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
								) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							}
							break;
						default:
							break;
					}
					break;
				case P1: //Main+, Charger Precharge Open(1441)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P2:
							if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
								&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
								&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
								&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
								&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
								) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							}
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P10:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //Charge+, Precharge Close(1442)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P2:
							if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
								&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
								&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
								&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
								&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
								) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							}
							break;
						default:
							break;
					}
					break;
				case P1: //Charge+, Precharge Open(1443)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P2:
							if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
								&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
								&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
								&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
								&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
								) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							}
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P11:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //Main+ Close(1444)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P2:
							if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
								&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
								&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
								&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
								&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
								) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							}
							break;
						default:
							break;
					}
					break;
				case P1: //Main+ Open(1445)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P2:
							if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
								&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
								&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
								&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
								&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
								) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							}
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P12:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //Charge+ Close(1446)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P2:
							if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
								&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
								&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
								&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
								&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
								) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							}
							break;
						default:
							break;
					}
					break;
				case P1: //Charge+ Open(1447)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P2:
							if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
								&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
								&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
								&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
								&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
								) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							}
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P13:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //Main- Close(1448)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P2:
							if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
								&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
								&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
								&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
								&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
								) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							}
							break;
						default:
							break;
					}
					break;
				case P1: //Main- Open(1449)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P2:
							if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
								&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
								&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
								&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
								&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
								) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							}
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P14:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2: //Max_Cell_Temp Check(1551)
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xD9
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x05
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[4]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[5])
							* 0.01 - 50.0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				case P4: //Min_Cell_Temp Check(1552)
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xD9
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x06
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[1]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[4]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[5])
							* 0.01 - 50.0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P15: //Coolant Lebel Open(1553)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDA
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x05
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].byte_value[0]
							= myData->CAN.uds_rx[can_ch].uc_val[4];
						myPs->eol_data[data_index].byte_value[1]
							= myData->CAN.uds_rx[can_ch].uc_val[5];
						myPs->eol_data[data_index].byte_value[2]
							= myData->CAN.uds_rx[can_ch].uc_val[6];
						myPs->eol_data[data_index].byte_value[3]
							= myData->CAN.uds_rx[can_ch].uc_val[7];
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P16: //CIRCULATION PUMP Check(1554)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDA
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x0F
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].long_value[0]
							= (long)myData->CAN.uds_rx[can_ch].uc_val[4] * 256
							+ (long)myData->CAN.uds_rx[can_ch].uc_val[5];
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P17: //COOLING PUMP Check(1555)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDA
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x0E
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].long_value[0]
							= (long)myData->CAN.uds_rx[can_ch].uc_val[4] * 256
							+ (long)myData->CAN.uds_rx[can_ch].uc_val[5];
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P18: //3Way Valve Check(1556)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x04
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[6]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[7])
							* 4.049;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P19: //DTC Check(1404)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x59
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xFF
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].long_value[0]
							= (long)myData->CAN.uds_rx[can_ch].uc_val[4]
							* 256 * 256
							+ (long)myData->CAN.uds_rx[can_ch].uc_val[5] * 256
							+ (long)myData->CAN.uds_rx[can_ch].uc_val[6];
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;

						myData->CAN.uds_rx_message_count = 0;
						myData->CAN.uds_rx_dtc_count = 0;
					}
					break;
				default:
					break;
			}
			break;
		case P20: //DTC List(1419)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					data_index = (int)myPs->
						signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x03
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x59
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x02
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xFF
						) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x59
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x02
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xFF
						) {
						myPs->eol_data[data_index].string_value[0]
							= myData->CAN.uds_rx[can_ch].uc_val[4];
						myPs->eol_data[data_index].string_value[1]
							= myData->CAN.uds_rx[can_ch].uc_val[5];
						myPs->eol_data[data_index].string_value[2]
							= myData->CAN.uds_rx[can_ch].uc_val[6];

						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					} else if((myData->CAN.uds_rx[can_ch].uc_val[0]
						& 0xF0) == 0x10) {
						myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_FC]
							= 14;
						myData->CAN.signal[0][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
							= P0;

						k = (int)(myData->CAN.uds_rx[can_ch].uc_val[0] & 0x0F)
							* 256;
						k += (int)myData->CAN.uds_rx[can_ch].uc_val[1];
						myData->CAN.uds_rx_message_count
							= (short int)((k - 6) / 7 + 1);

						myPs->eol_data[data_index].string_value[0]
							= myData->CAN.uds_rx[can_ch].uc_val[5];
						myPs->eol_data[data_index].string_value[1]
							= myData->CAN.uds_rx[can_ch].uc_val[6];
						myPs->eol_data[data_index].string_value[2]
							= myData->CAN.uds_rx[can_ch].uc_val[7];
					} else if((myData->CAN.uds_rx[can_ch].uc_val[0] & 0xF0)
						== 0x20) {
						k = myPs->misc.dtc_data_index;
						myPs->misc.dtc_data_index += 7;
						memcpy((char *)&myPs->misc.dtc_data[k],
							(char *)&myData->CAN.uds_rx[can_ch].uc_val[1], 7);

						myData->CAN
							.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]++;

						l = 1;

						if(myData->CAN
							.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
							== myData->CAN.uds_rx_message_count
							|| myData->CAN
							.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_INDEX]
							== 255) {
							//l = 1;
							m = 0;
							for(k=1; k < myPs->misc.dtc_data_index; k++) {
								//if(myPs->misc.dtc_data[k] == 0x00) {
								//	break;
								//}
								if(m < 3) {
									myPs->eol_data[data_index]
										.string_value[l*3+m]
										= myPs->misc.dtc_data[k];
								}
								m++;
								if(m == 4) {
									l++;
									m = 0;
								}
							}
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
						}
					}
					break;
				default:
					break;
			}
			break;
		case P21: //Cell delta voltage Check(1407)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x3F
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[4]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[5]);
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P22: //Module delta temp Check(1405)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xD9
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF6
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[4]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[5])
							* 10.0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P23: //Inlet temp Check(1406)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05) {
						if(myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x46
						) {
							data_index = (int)myPs->
								signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].float_value[0]
								= (float)((int)myData->CAN.uds_rx[can_ch]
									.uc_val[4]
								* 256 + (int)myData->CAN.uds_rx[can_ch]
								.uc_val[5])
								* 0.01 - 50.0;
							myPs->eol_data[data_index].float_value[0] *= 1000.0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
						}
					} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x04) {
						if(myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x46
						) {
							data_index = (int)myPs->
								signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
							myPs->eol_data[data_index].float_value[0]
								= (float)myData->CAN.uds_rx[can_ch].uc_val[4]
								- 50.0;
							myPs->eol_data[data_index].float_value[0] *= 1000.0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
							myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
						}
					}
					break;
				default:
					break;
			}
			break;
		case P25: //Pack voltage Check(1410)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x0D
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[4]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[5])
							* 10000.0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P26: //Link voltage Check(1411)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x07
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[4]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[5])
							* 10000.0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P28: //Charger voltage Check(1412)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x0E
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[4]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[5])
							* 10000.0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P29: //Elac_Fuse_V Check(1415)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xD9
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x01
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[4]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[5])
							* 10000.0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P30:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_SUB_INDEX]) {
				case P0: //MAIN+, CHARGER+ close(1557)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P2:
							if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
								&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
								&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
								&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
								&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
								) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							}
							break;
						default:
							break;
					}
					break;
				case P1: //MAIN+, CHARGER+ open(1558)
					switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
						case P2:
							if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
								&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
								&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
								&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
								&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x02
								) {
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
								myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
							}
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
			break;
		case P31: //Charger_Fuse_V Check(1414)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xD9
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x02
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[4]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[5])
							* 10000.0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P32: //Isolation resistance check(1416)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xEE
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x3C
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x00
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[6]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[7])
							* 1000.0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P33:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2: //HVIL_Return Check(1436)
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x04
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[6]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[7])
							* 7.446289;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				case P4: //HVIL_Source Check(1559)
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x04
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[1]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[6]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[7])
							* 7.446289;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P34:
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2: //AUX_HVIL_Return Check(1560)
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x04
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[6]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[7])
							* 7.446289;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				case P4: //AUX_HVIL_Source Check(1437)
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x04
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[1]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[6]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[7])
							* 7.446289;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P37: //Clear_DTCs(1433)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x54
						) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P38: //SAVE_EEPROM(1434)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x10
						//&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x65
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x08
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x08
						&& myData->CAN.uds_rx[can_ch].uc_val[6] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[7] == 0x00
						) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P39: //ECU_Reset(1435)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x59
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xFF
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x00
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						&& myData->CAN.uds_rx[can_ch].uc_val[6] == 0x00
						) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P42: //Max_Cell_V(1561)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xD9
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF5
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x0B
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[5]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[6]);
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P43: //Min_Cell_V(1562)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x40
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x09
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[5]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[6]);
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P44: //All_Cell_V(1563)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P3:
					k = (int)myData->CAN.uds_rx[can_ch].uc_val[2] * 256
						+ (int)myData->CAN.uds_rx[can_ch].uc_val[3];
					l = (int)0xDA3E
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_CHECK_INDEX] -1;
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& (k == l)
						//&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0xDA
						//&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x3E
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[4]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[5]);
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P45: //All_Module_Temp(1564)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P3:
					k = (int)myData->CAN.uds_rx[can_ch].uc_val[2] * 256
						+ (int)myData->CAN.uds_rx[can_ch].uc_val[3];
					l = (int)0xDAA2
						+ (int)myPs->signal[M_SIG_EOL_PROCEDURE_CHECK_INDEX] -1;
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x05
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& (k == l)
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[4]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[5])
							* 0.01 - 50.0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P46: //Set_Pump1_PWM(1565)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x0B
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].byte_value[0]
							= myData->CAN.uds_rx[can_ch].uc_val[6];
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P48: //Set_Pump2_PWM(1567)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x0B
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].byte_value[0]
							= myData->CAN.uds_rx[can_ch].uc_val[6];
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P50: //Set_Valve_On(1569)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x0D
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].byte_value[0]
							= myData->CAN.uds_rx[can_ch].uc_val[6];
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P52: //NORMAL_MODE(1571)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x06
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x50
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P53: //CIE_VOLTAGE(1572)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x07
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0x04
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x00
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].float_value[0]
							= (float)((int)myData->CAN.uds_rx[can_ch].uc_val[6]
							* 256 + (int)myData->CAN.uds_rx[can_ch].uc_val[7])
							* 7.324219;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P55: //DISABLE_DIAG(1573)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x10
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x09
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x0A
						) {
						myData->CAN
							.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] = 14;
					} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x21) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P56: //ISO_RESULT_WRITE(1574)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x03
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x6E
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x95
						) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P57: //ISO_RESULT_CHECK(1575)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x04
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x62
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x48
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x95
						//&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0xBB
						) {
						data_index = (int)myPs->
							signal[M_SIG_EOL_PROCEDURE_DATA_INDEX];
						myPs->eol_data[data_index].byte_value[0]
							= myData->CAN.uds_rx[can_ch].uc_val[4];
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		case P58: //DUMP_NVM_WRITE(1576)
			switch(myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]) {
				case P2:
					if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x10
						&& myData->CAN.uds_rx[can_ch].uc_val[1] == 0x09
						&& myData->CAN.uds_rx[can_ch].uc_val[2] == 0x71
						&& myData->CAN.uds_rx[can_ch].uc_val[3] == 0x01
						&& myData->CAN.uds_rx[can_ch].uc_val[4] == 0xF0
						&& myData->CAN.uds_rx[can_ch].uc_val[5] == 0x09
						) {
						myData->CAN
							.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] = 14;
					} else if(myData->CAN.uds_rx[can_ch].uc_val[0] == 0x21) {
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY1] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_DELAY2] = 0;
						myPs->signal[M_SIG_EOL_PROCEDURE_PHASE]++;
					}
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}

void can_kokam_bms_data_parsing(int module_ch, int index)
{
	short int function_div;

	function_div = myData->canReceiveSetData.normalData[module_ch][index]
		.function_div;
	if(function_div == CAN_RX_FUNC_DIV_KOKAM_SOC) {
		myData->CanData[module_ch][index].f_val[0]
			= (float)myPs->bms_frame_data.data[0]
			* myData->canReceiveSetData.normalData[module_ch][index].factor
			+ myData->canReceiveSetData.normalData[module_ch][index].offset;
	} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_SOH) {
		myData->CanData[module_ch][index].f_val[0]
			= (float)myPs->bms_frame_data.data[1]
			* myData->canReceiveSetData.normalData[module_ch][index].factor
			+ myData->canReceiveSetData.normalData[module_ch][index].offset;
	} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_VOLTAGE) {
		myData->CanData[module_ch][index].f_val[0]
			= (float)myPs->bms_frame_data.data[2]
			* myData->canReceiveSetData.normalData[module_ch][index].factor
			+ myData->canReceiveSetData.normalData[module_ch][index].offset;
	} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_CURRENT) {
		myData->CanData[module_ch][index].f_val[0]
			= (float)myPs->bms_frame_data.data[3]
			* myData->canReceiveSetData.normalData[module_ch][index].factor
			+ myData->canReceiveSetData.normalData[module_ch][index].offset;
	} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_MAX_CELL_VOLTAGE) {
		myData->CanData[module_ch][index].f_val[0]
			= (float)myPs->bms_frame_data.data[4]
			* myData->canReceiveSetData.normalData[module_ch][index].factor
			+ myData->canReceiveSetData.normalData[module_ch][index].offset;
	} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_MIN_CELL_VOLTAGE) {
		myData->CanData[module_ch][index].f_val[0]
			= (float)myPs->bms_frame_data.data[5]
			* myData->canReceiveSetData.normalData[module_ch][index].factor
			+ myData->canReceiveSetData.normalData[module_ch][index].offset;
	} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_AVG_CELL_TEMP) {
		myData->CanData[module_ch][index].f_val[0]
			= (float)myPs->bms_frame_data.data[6]
			* myData->canReceiveSetData.normalData[module_ch][index].factor
			+ myData->canReceiveSetData.normalData[module_ch][index].offset;
	} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_MAX_CELL_TEMP) {
		myData->CanData[module_ch][index].f_val[0]
			= (float)myPs->bms_frame_data.data[7]
			* myData->canReceiveSetData.normalData[module_ch][index].factor
			+ myData->canReceiveSetData.normalData[module_ch][index].offset;
	} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_MIN_CELL_TEMP) {
		myData->CanData[module_ch][index].f_val[0]
			= (float)myPs->bms_frame_data.data[8]
			* myData->canReceiveSetData.normalData[module_ch][index].factor
			+ myData->canReceiveSetData.normalData[module_ch][index].offset;
	} else if((function_div >= CAN_RX_FUNC_DIV_KOKAM_FLAG1)
		&& (function_div <= CAN_RX_FUNC_DIV_KOKAM_FLAG8)) {
		if(function_div == CAN_RX_FUNC_DIV_KOKAM_FLAG1) {
			if((myPs->bms_frame_data.flag & 0x01) == 0x01) {
				myData->CanData[module_ch][index].f_val[0]
					= (float)myPs->bms_frame_data.flag
					* myData->canReceiveSetData
						.normalData[module_ch][index].factor
					+ myData->canReceiveSetData
						.normalData[module_ch][index].offset;
			} else {
				myData->CanData[module_ch][index].f_val[0] = 0;
			}
		} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_FLAG2) {
			if((myPs->bms_frame_data.flag & 0x02) == 0x02) {
				myData->CanData[module_ch][index].f_val[0]
					= (float)(myPs->bms_frame_data.flag >> 1)
					* myData->canReceiveSetData
						.normalData[module_ch][index].factor
					+ myData->canReceiveSetData
						.normalData[module_ch][index].offset;
			} else {
				myData->CanData[module_ch][index].f_val[0] = 0;
			}
		} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_FLAG3) {
			if((myPs->bms_frame_data.flag & 0x04) == 0x04) {
				myData->CanData[module_ch][index].f_val[0]
					= (float)(myPs->bms_frame_data.flag >> 2)
					* myData->canReceiveSetData
						.normalData[module_ch][index].factor
					+ myData->canReceiveSetData
						.normalData[module_ch][index].offset;
			} else {
				myData->CanData[module_ch][index].f_val[0] = 0;
			}
		} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_FLAG4) {
			if((myPs->bms_frame_data.flag & 0x08) == 0x08) {
				myData->CanData[module_ch][index].f_val[0]
					= (float)(myPs->bms_frame_data.flag >> 3)
					* myData->canReceiveSetData
						.normalData[module_ch][index].factor
					+ myData->canReceiveSetData
						.normalData[module_ch][index].offset;
			} else {
				myData->CanData[module_ch][index].f_val[0] = 0;
			}
		} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_FLAG5) {
			if((myPs->bms_frame_data.flag & 0x10) == 0x10) {
				myData->CanData[module_ch][index].f_val[0]
					= (float)(myPs->bms_frame_data.flag >> 4)
					* myData->canReceiveSetData
						.normalData[module_ch][index].factor
					+ myData->canReceiveSetData
						.normalData[module_ch][index].offset;
			} else {
				myData->CanData[module_ch][index].f_val[0] = 0;
			}
		} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_FLAG6) {
			if((myPs->bms_frame_data.flag & 0x20) == 0x20) {
				myData->CanData[module_ch][index].f_val[0]
					= (float)(myPs->bms_frame_data.flag >> 5)
					* myData->canReceiveSetData
						.normalData[module_ch][index].factor
					+ myData->canReceiveSetData
						.normalData[module_ch][index].offset;
			} else {
				myData->CanData[module_ch][index].f_val[0] = 0;
			}
		} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_FLAG7) {
			if((myPs->bms_frame_data.flag & 0x40) == 0x40) {
				myData->CanData[module_ch][index].f_val[0]
					= (float)(myPs->bms_frame_data.flag >> 6)
					* myData->canReceiveSetData
						.normalData[module_ch][index].factor
					+ myData->canReceiveSetData
						.normalData[module_ch][index].offset;
			} else {
				myData->CanData[module_ch][index].f_val[0] = 0;
			}
		} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_FLAG8) {
			if((myPs->bms_frame_data.flag & 0x80) == 0x80) {
				myData->CanData[module_ch][index].f_val[0]
					= (float)(myPs->bms_frame_data.flag >> 7)
					* myData->canReceiveSetData
						.normalData[module_ch][index].factor
					+ myData->canReceiveSetData
						.normalData[module_ch][index].offset;
			} else {
				myData->CanData[module_ch][index].f_val[0] = 0;
			}
		}
	} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_VMAX_MODULE_NUMBER) {
		myData->CanData[module_ch][index].f_val[0]
			= (float)(myPs->bms_frame_data.Vmax_module_number_hi_nibble > 4)
			* myData->canReceiveSetData.normalData[module_ch][index].factor
			+ myData->canReceiveSetData.normalData[module_ch][index].offset;
	} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_VMIN_MODULE_NUMBER) {
		myData->CanData[module_ch][index].f_val[0]
			= (float)myPs->bms_frame_data.Vmin_module_number_lo_nibble
			* myData->canReceiveSetData.normalData[module_ch][index].factor
			+ myData->canReceiveSetData.normalData[module_ch][index].offset;
	} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_TMAX_MODULE_NUMBER) {
		myData->CanData[module_ch][index].f_val[0]
			= (float)(myPs->bms_frame_data.Tmax_module_number_hi_nibble > 4)
			* myData->canReceiveSetData.normalData[module_ch][index].factor
			+ myData->canReceiveSetData.normalData[module_ch][index].offset;
	} else if(function_div == CAN_RX_FUNC_DIV_KOKAM_TMIN_MODULE_NUMBER) {
		myData->CanData[module_ch][index].f_val[0]
			= (float)myPs->bms_frame_data.Tmin_module_number_lo_nibble
			* myData->canReceiveSetData.normalData[module_ch][index].factor
			+ myData->canReceiveSetData.normalData[module_ch][index].offset;
	}
}

/*void CAN_Control2(int slot)
{
	int i;

	if(myData->AppControl.misc.Load_Process_CAN[0] != P1) return;
	if((slot + 1) > myPs->config.installedCAN) return;

	for(i=0; i < myPs->config.installedCAN; i++) {
		if(i != slot) continue;

		canSignalCheck(i);
	}
}*/

//kjg_240213 void canSignalCheck(int can_ch)
void CAN_Control2(int can_ch)
{
	char function_phase; //kjhw_170410
	int i, module_ch;
	int idx=0; //kjhw_141212
	int count=0; //kjhw_170410
	unsigned char master_slave; //kjh_210913

	if(myData->AppControl.misc.Load_Process_CAN[0] != P1) return;
	if(can_ch >= myPs->config.installedCAN) return;

	i = 0;
#ifdef __CAN_FD__		//kjh_190706s
	switch(myData->CAN.signal[can_ch][CAN_SIG_ABORT_TRANSMISSION]) {
		case P2:
			if(rt_can_check_status(can_ch) == 100)
				myData->CAN.signal[can_ch][CAN_SIG_ABORT_TRANSMISSION] = P3;
			break;
		case P3:
			if(rt_can_check_status(can_ch) == 100) i = 1;
			break;
		default: break;
	}
	if(i == 0) return;
#else //CAN_2p0b
	if(myData->CAN.signal[can_ch][CAN_SIG_ABORT_TRANSMISSION] == 0) {
		return;
	} else if(myData->CAN.signal[can_ch][CAN_SIG_ABORT_TRANSMISSION] < 110) {
		myData->CAN.signal[can_ch][CAN_SIG_ABORT_TRANSMISSION]++;
		if(myData->CAN.signal[can_ch][CAN_SIG_ABORT_TRANSMISSION] > 100) {
			myData->CAN.signal[can_ch][CAN_SIG_ABORT_TRANSMISSION] = 110;
		}

		return;
	}
#endif //kjh_190706e

	switch(myData->mData.config.division_CAN) {
		case 1:
			module_ch = can_ch;
			master_slave = 0;
			break;
		default:
			//kjhw_170630s
			//module_ch = can_ch / 2;
			switch(myData->AppControl.config.systemModel) {
				case C_SKI_120V_400A_100A_192KW:
				case C_SKI_120V_400A_100A_192KW_2:
				case C_SKI_120V_400A_100A_192KW_3:
				case C_SKI_120V_400A_100A_192KW_4:
				case C_SKI_120V_400A_100A_192KW_5:
				case C_SKI_120V_400A_100A_192KW_6:
				case C_SKI_120V_400A_100A_192KW_7:
				case C_SKI_120V_400A_100A_192KW_8:
				case C_KBIA_500V_250A_100A_250KW:	 //shh_200916
				case C_JBTP_500V_250A_100A_500KW:	 //shh_200927
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
				case C_KCL_600V_200A_100A_480KW: 	  //ktg_190812
				case C_KCL_600V_200A_100A_480KW_2: 	  //ktg_190812
				case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
				case C_ADD_800V_350A_200A_50A_380KW:  //shh_200904
				case C_HYUNDAI_1000V_250A_100A_500KW: //shh_200513
				case C_KOSTA_1000V_250A_100A_600KW:	  //shh_201102
				case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
				case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
				case C_KTC_1500V_600A_200A_400KW:	  //ktg_200807
					module_ch = can_ch / 2;
					if(module_ch == 1) module_ch = 2;
					master_slave = can_ch % 2;
					break;
				case C_SK_450V_200A_10A_360KW:
					module_ch = can_ch / 2;
					if(module_ch == 1) module_ch = 3;
					master_slave = can_ch % 2;
					break;
				default:
					module_ch = can_ch / 2; //kjhw_171108
					master_slave = can_ch % 2;
					break;
			}
			break;
			//kjhw_170630e
	}

	if(myData->canTransmitSetData.commonData[module_ch][0].bms_type
		== 0) { //kjg_161207
		canSignalCheck_0(can_ch);
	} else if(myData->canTransmitSetData.commonData[module_ch][0].bms_type
		== 3) { //kjh_180821	//rolling
		//canSignalCheck_3(can_ch);
		canSignalCheck_3(module_ch, can_ch, master_slave); //kjh_210913
	} else if(myData->canTransmitSetData.commonData[module_ch][0].bms_type
		== 50) { //kjg_161207 Porsche_PAG12V_BMS
		canSignalCheck_50(can_ch);
	}

	if(myData->CAN.signal[can_ch][CAN_SIG_TEST1] == P1) {
		can_transmit1(can_ch);
		return;
	}

	switch(myData->AppControl.config.systemModel) {
		case C_LGC_400V_60A_10A:
		case C_ROTEM_400V_60A_10A:
			can_transmit2(can_ch);
			break;
		case C_KEPCO_500V_200A_10A_100KW:
			module_ch = can_ch / 2;

			if(myData->dio.signal[DIO_SIG_IN_CHARGER_WAKEUP] == P0) {
				myData->CAN.signal[module_ch][CAN_SIG_CHARGER_PHASE] = P0;
				if(myData->dio.signal[DIO_SIG_IN_CHARGER_PHASE] == P0) {
					myData->dio.signal[DIO_SIG_IN_CHARGER_PHASE] = P1;
					memset((char *)&myData->canTransmitChange
						.changeData[module_ch][0],
						0, sizeof(S_CAN_TRANSMIT_CHANGE_DATA)
						* MAX_CAN_TRANSMIT_CHANGE_DATA);
				}
			} else {
				myData->dio.signal[DIO_SIG_IN_CHARGER_PHASE] = P0;
				if(myData->CAN.signal[module_ch][CAN_SIG_CHARGER_PHASE] == P0) {
					myData->CAN.signal[module_ch][CAN_SIG_CHARGER_PHASE] = P1;
				} else {
					if(myData->cData[0].op.state == C_RUN) {
						if(myData->CAN.signal[module_ch][CAN_SIG_CHARGER_PHASE]
							== P1) {
							myData->CAN.signal[module_ch][CAN_SIG_CHARGER_PHASE]
								= P2;
						}
					} else {
						if(myData->CAN.signal[module_ch][CAN_SIG_CHARGER_PHASE]
							== P2) {
							myData->CAN.signal[module_ch][CAN_SIG_CHARGER_PHASE]
								= P3;
							myData->CAN.signal[module_ch][CAN_SIG_CHARGER_DELAY]
								= P0;
						} else if(myData->CAN.signal[module_ch]
							[CAN_SIG_CHARGER_PHASE] == P3) {
							myData->CAN.signal[module_ch][CAN_SIG_CHARGER_DELAY]
								+= 1;
							if(myData->CAN.signal[module_ch]
								[CAN_SIG_CHARGER_DELAY] > 20) {
								myData->CAN.signal[module_ch]
									[CAN_SIG_CHARGER_PHASE] = P4;
							}
						}
					}
				}
			}

			i = myData->CAN.signal[module_ch][CAN_SIG_CHARGER_PHASE];
			myData->CAN.signal[module_ch][CAN_SIG_TX_PHASE_CHARGER]
				= (CAN_TX_FUNC_DIV_CHARGER_PHASE1 + i) % 50;

			can_transmit3(can_ch);
			break;
		default:
			//kjhw_170410s
			function_phase
				= myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MIN] - 3;
			if(function_phase >= 0) {
				switch(function_phase) {
					case 1: //increment
						myData->CAN.can_timer_1min_count[can_ch]++;
						count = (int)myData->CAN.can_timer_1min_count[can_ch]
							* (int)myPs->config.scan_period;
						if(count >= 1000) {
							myData->CAN.can_timer_1min_count[can_ch] = 0;
							myData->CAN.can_timer_1min_count2[can_ch]++;
						}
						if(myData->CAN.can_timer_1min_count2[can_ch] >= 60) {
							myData->CAN.can_timer_1min_count2[can_ch] = 0;
							myData->CAN.can_timer_1min[can_ch]++;
						}
						if(myData->CAN.can_timer_1min[can_ch] >= 16770000) {
							myData->CAN.can_timer_1min[can_ch] = 16770000;
						}
						break;
					case 2: //increment_pause
						break;
					case 3: //increment_clear
						myData->CAN.can_timer_1min_count[can_ch] = 0;
						myData->CAN.can_timer_1min_count2[can_ch] = 0;
						myData->CAN.can_timer_1min[can_ch] = 0;
						break;
					default: break;
				}
			}	//kjhw_170410e
			switch(myData->mData.config.division_CAN) {
				case 1:
					module_ch = can_ch;
					break;
				default:
					//kjhw_170630s
					//module_ch = can_ch / 2;
					switch(myData->AppControl.config.systemModel) {
						case C_SKI_120V_400A_100A_192KW:
						case C_SKI_120V_400A_100A_192KW_2:
						case C_SKI_120V_400A_100A_192KW_3:
						case C_SKI_120V_400A_100A_192KW_4:
						case C_SKI_120V_400A_100A_192KW_5:
						case C_SKI_120V_400A_100A_192KW_6:
						case C_SKI_120V_400A_100A_192KW_7:
						case C_SKI_120V_400A_100A_192KW_8:
						case C_KBIA_500V_250A_100A_250KW:	 //shh_200916
						case C_JBTP_500V_250A_100A_500KW:	 //shh_200927
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
						case C_KCL_600V_200A_100A_480KW: 	  //ktg_190812
						case C_KCL_600V_200A_100A_480KW_2: 	  //ktg_190812
						case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
						case C_ADD_800V_350A_200A_50A_380KW:  //shh_200904
						case C_HYUNDAI_1000V_250A_100A_500KW: //shh_200513
						case C_KOSTA_1000V_250A_100A_600KW:	  //shh_201102
						case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
						case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
						case C_KTC_1500V_600A_200A_400KW:	  //ktg_200807
							module_ch = can_ch / 2;
							if(module_ch == 1) module_ch = 2;
							break;
						case C_SK_450V_200A_10A_360KW:
							module_ch = can_ch / 2;
							if(module_ch == 1) module_ch = 3;
							break;
						default:
							module_ch = can_ch / 2; //kjhw_171108
							break;
					}
					break;
					//kjhw_170630e
			}
			idx = IDX_LOC_OBJ_FAULT_CAN_TX_STOP;
			if(myData->testCond[module_ch].local_object
				[myData->cData[module_ch].op.idxStepNo][idx] == 1) {
				break;
			} else {
				can_transmit3(can_ch);
			}	//kjhw_141212e
			break;
	}
}

void canSignalCheck_0(int can_ch)
{
	switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST1]) {
		case P1:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= 100) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1] = P2;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST1]++;
			}
			break;
		case P2:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= 100) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1] = P3;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST1]++;
			}
			break;
		case P3:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= 100) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1] = P4;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST1] = P0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P1;
			}
			break;
		default:
			break;
	}

	switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST2]) {
		case P1:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_DELAY1] >= 100) {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST2]++;
			}
			break;
		case P2:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_DELAY1] >= 1) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST4] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST2]++;
			}
			break;
		case P3:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_DELAY1] >= 1) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST4] = 1;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST2] = P2;
			}
			break;
		default:
			break;
	}

	switch(myData->AppControl.config.systemModel) {
		case C_LGC_70V_250A_10A_35KW_3: //kjhw_150803
		case C_LGC_70V_250A_10A_35KW_4: //kjhw_150803
		case C_LGC_70V_250A_10A_35KW_5:
		case C_LGC_70V_250A_10A_35KW_6:
		case C_LGC_70V_250A_10A_35KW_7: //kjhw_150803
		case C_LGC_70V_250A_10A_35KW_8: //kjhw_150803
		case C_LGC_70V_250A_10A_35KW_16:
		case C_LGC_70V_250A_10A_35KW_17:
		case C_LGC_70V_250A_10A_35KW_18:
			switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]) {
				case P1:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 100) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P2:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 2) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 29;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P3:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 8) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P4:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 2) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 30;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P5:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 8) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P6:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 2) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 31;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P7:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 8) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P8:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 2) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 32;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P9:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 8) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P10:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 2) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST3] = 33;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P11:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 8) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P12:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 2) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST3] = 34;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P13:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 8) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P14:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 2) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST3] = 35;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P15:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 8) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P16:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 2) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST3] = 36;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P17:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 8) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P18:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 2) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST3] = 37;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P19:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 8) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P20:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 2) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST3] = 38;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P21:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 8) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P22:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 2) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST3] = 39;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P23:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 8) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P24:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 2) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST3] = 40;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P25:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 8) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P26:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 2) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST3] = 41;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P27:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 8) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P28:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P2;
					break;
				default:
					break;
			}
			break;
		case C_LGC_70V_250A_100A_50A_35KW_4: //audi_module oh_chang
			switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]) {
				case P1:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 100) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P2:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 1) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST4] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P3:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 1) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST4] = 1;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;

						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P2;
					}
					break;
				default: break;
			}
			break;
		default:
			switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]) {
				case P1:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 100) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P2:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 20) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 29;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P3:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 100) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P4:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 20) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 30;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P5:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 100) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P6:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 20) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 31;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P7:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 100) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P8:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 20) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 32;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P9:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 100) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P10:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY1] >= 20) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 33;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P11:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
					if(myData->CAN.signal[can_ch]
						[CAN_SIG_BMS_UDS_DELAY2] >= 100) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
					break;
				case P12:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P2;
					//kjg_d myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = 0;
					//kjhw_141110e
					break;
				default:
					break;
			}
			break;
	}
}

void canSignalCheck_3(int module_ch, int can_ch, unsigned char master_slave)
{ //kjh_210913
	unsigned char function_phase;
	short int function_div;
	int cnt_start, cnt_end, i;

	if(master_slave == 0) { //master
		cnt_start = 0;
	} else { //slave
		cnt_start = myData->canTransmitDataCount[module_ch][0];
	}
	cnt_end = cnt_start + myData->canTransmitDataCount[module_ch][master_slave];

	switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST1]) {
		case P1:
			myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1] = P0;
			myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST1] = P2;
			break;
		case P2:
			function_phase = myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1];
			for(i=cnt_start; i < cnt_end; i++) {
				function_div
					= myData->canTransmitSetData.normalData[module_ch][i].function_div;
				if(function_div >= CAN_TX_FUNC_DIV_TEST3_PHASE1
					&& function_div <= CAN_TX_FUNC_DIV_TEST3_PHASE31) {
					if(function_phase == (function_div % 50)) {
						if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1] >= P30) {
							myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1] = P0;
							break;
						}
						break;
					}
				} 
				if(i == cnt_end - 1) {
					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1] = P0;
				}
			}
			break;
		default: break;
	}
	switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST2]) {
		case P1:
			myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = P0;
			myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST2] = P2;
			break;
		case P2:
			function_phase = myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2];
			for(i=cnt_start; i < cnt_end; i++) {
				function_div
					= myData->canTransmitSetData.normalData[module_ch][i].function_div;
				if(function_div >= CAN_TX_FUNC_DIV_TEST2_PHASE1
					&& function_div <= CAN_TX_FUNC_DIV_TEST2_PHASE31) {
					if(function_phase == (function_div % 50)) {
						if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] >= P30) {
							myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = P0;
							break;
						}
						break;
					}
				} 
				if(i == cnt_end - 1) {
					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = P0;
				}
			}
			break;
		default: break;
	}
	switch(myData->cData[module_ch].signal[C_SIG_CAN_TEST5_RUN]) {	//ktg_220724s
		case P1:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST5] = P1;
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST5] == P1) {
				myData->cData[module_ch].signal[C_SIG_CAN_TEST5_RUN] = P2;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST5] = P2;
			}
			break;
		default: break;
 	}	//ktg_220724e
	switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST5]) {	//ktg_220614s
		case P1:
			myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST5] = P0;
			myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST5] = P2;
			break;
		default: break;
 	}	//ktg_220614e
	
}

void canSignalCheck_50(int can_ch)
{ //kjg_161207
	int i, function_phase, delay_time;
	S_MSG_VAL SendMsg;

	delay_time = 20; //kjg_161219 100(1000ms) -> 20(200ms)

	switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST1]) {
		case P1:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= 100) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1] = P2;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST1]++;
			}
			break;
		case P2:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= 100) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1] = P3;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST1]++;
			}
			break;
		case P3:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= 100) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1] = P4;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST1] = P0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P1;
			}
			break;
		default:
			break;
	}

	switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]) {
		case P1:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= delay_time) {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P2:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= 1) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 50;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P3:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] >= delay_time) {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P4:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= 1) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 51;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P5:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] >= 100) {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P6:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P10;
			break;
		case P7:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] >= delay_time) {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P8:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= 1) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 53;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P9:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] >= delay_time) {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P10:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= 1) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 54;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P11:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] >= delay_time) {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P12:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P16;
			break;
		case P13:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] >= delay_time) {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P14:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= 1) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 56;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P15:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] >= delay_time) {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P16:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= 1) {
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = 57;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P17:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] >= delay_time) {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P18:
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_PROCEDURE_PHASE] == P0) {
				function_phase = 0;
				for(i=0; i < myData->canTransmitDataCount[can_ch/2][0]; i++) {
					if(myData->canTransmitSetData.normalData[can_ch/2][i].canID == 0x1FE00000) {
						if(myData->canTransmitSetData.normalData[can_ch/2][i+5].default_value == 1) {
							function_phase = 1;
						}
						break;
					}
				}
				if(function_phase == 0) {
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P2;
				} else {
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P18;
				}
			} else {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_PROCEDURE_PHASE] = P0;

				function_phase = 0;
				switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_PROCEDURE_INDEX]) {
					case 0: //Reserved
						break;
					case 1: //BMS WakeUp
						function_phase = 29;
						break;
					case 2: //Sleep
						function_phase = 30;
						break;
					case 3: //Relay ON
						function_phase = 31;
						break;
					case 4: //Relay OFF
						function_phase = 32;
						break;
					case 5: //Read deep sleep flag
						function_phase = 33;
						break;
					case 6: //BMS deep sleep
						function_phase = 34;
						break;
					case 7: //Relay ON(force) kjg_161213
						function_phase = 35;
						break;
					case 8: //Clear error flag 0xD048 kjg_161214
						function_phase = 36;
						break;
					default:
						break;
				}

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_COA_BMS_PROCEDURE_REPLY;
				SendMsg.val[0] = can_ch / 2;
				SendMsg.val[1] = myData->canTransmitSetData.commonData[can_ch/2][0].bms_type;
				SendMsg.val[2] = myData->CAN.signal[can_ch][CAN_SIG_BMS_PROCEDURE_INDEX];

				if(function_phase == 0) {
					SendMsg.val[3] = 3; //error code 0:ok, 1~:ng
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P2;
				} else {
					SendMsg.val[3] = 0; //error code 0:ok, 1~:ng

					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1]++;
					if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] >= 1) {
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = function_phase;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
						myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
					}
				}

				send_msg(MODULE_TO_COA1, (char *)&SendMsg);
			}
			break;
		case P19:
			myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2]++;
			if(myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] >= delay_time) {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY1] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_DELAY2] = 0;
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE]++;
			}
			break;
		case P20:
			switch(myData->CAN.signal[can_ch][CAN_SIG_BMS_PROCEDURE_INDEX]) {
				case 1: //BMS WakeUp
					for(i=0; i < myData->canTransmitDataCount[can_ch/2][0]; i++) {
						if(myData->canTransmitSetData.normalData[can_ch/2][i].canID == 0x1FE00000) {
							myData->canTransmitSetData.normalData[can_ch/2][i+5].default_value = 0;
							break;
						}
					}

					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P2;
					break;
				case 2: //Sleep
					for(i=0; i < myData->canTransmitDataCount[can_ch/2][0]; i++) {
						if(myData->canTransmitSetData.normalData[can_ch/2][i].canID == 0x1FE00000) {
							myData->canTransmitSetData.normalData[can_ch/2][i+5].default_value = 1;
							break;
						}
					}

					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P18;
					break;
				default:
					myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P2;
					break;
			}

			function_phase = 0;
			for(i=0; i < myData->canTransmitDataCount[can_ch/2][0]; i++) {
				if(myData->canTransmitSetData.normalData[can_ch/2][i].canID == 0x1FE00000) {
					if(myData->canTransmitSetData.normalData[can_ch/2][i+5].default_value == 1) {
						function_phase = 1;
					}
					break;
				}
			}
			if(function_phase == 0) {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P2;
			} else {
				myData->CAN.signal[can_ch][CAN_SIG_BMS_UDS_PHASE] = P18;
			}
			break;
		default:
			break;
	}
}

void can_transmit1(int can_ch)
{
	//int i;
	S_RT_CAN_MSG can_msg;

	switch(myData->mData.config.division_CAN) {
		case 1:
			if(can_ch == 0 || can_ch == 2) { //ch1, ch3
				if((myPs->misc.timer_1sec_count % 2) == 0) { //20ms
					can_msg.id = 0x7CF; //1999d
					can_msg.type = 0;
					can_msg.length = 8;
					can_msg.data[0]
						= (unsigned char)(myPs->misc.timer_1sec_count % 255);
					can_msg.data[1] = 0xAA;
					can_msg.data[2] = 0xAA;
					can_msg.data[3] = 0xAA;
					can_msg.data[4]
						= (unsigned char)(myPs->misc.timer_1sec_count % 255);
					can_msg.data[5] = 0xAA;
					can_msg.data[6] = 0xAA;
					can_msg.data[7] = 0xAA;

					rt_can_write(can_ch, (char *)&can_msg);
					//for(i=0; i < 10; i++) { //2000d~
					//	can_msg.id = 0x7D0 + i;
					//	rt_can_write(can_ch, (char *)&can_msg);
					//}
					can_msg.id = 0x7D0;		//csk_191204
					rt_can_write(can_ch, (char *)&can_msg);	//csk_191204
				}
			} else { //ch2, ch4
				if((myPs->misc.timer_1sec_count % 2) == 0) { //20ms
					can_msg.id = 0x3E7; //999d
					can_msg.type = 0;
					can_msg.length = 8;
					can_msg.data[0]
						= (unsigned char)(myPs->misc.timer_1sec_count % 255);
					can_msg.data[1] = 0xAA;
					can_msg.data[2] = 0xAA;
					can_msg.data[3] = 0xAA;
					can_msg.data[4]
						= (unsigned char)(myPs->misc.timer_1sec_count % 255);
					can_msg.data[5] = 0xAA;
					can_msg.data[6] = 0xAA;
					can_msg.data[7] = 0xAA;

					rt_can_write(can_ch, (char *)&can_msg);
					//for(i=0; i < 10; i++) { //1000d
					//	can_msg.id = 0x3E8 + i;
					//	rt_can_write(can_ch, (char *)&can_msg);
					//}	//csk_191204r
					can_msg.id = 0x3E8;		//csk_191204
					rt_can_write(can_ch, (char *)&can_msg);	//csk_191204

				}
			}
			break;
		default:
			if((can_ch % 2) == 0) { //master
				if((myPs->misc.timer_1sec_count % 2) == 0) { //20ms
					can_msg.id = 0x7CF; //1999d
					can_msg.type = 0;
					can_msg.length = 8;
					can_msg.data[0]
						= (unsigned char)(myPs->misc.timer_1sec_count % 255);
					can_msg.data[1] = 0xAA;
					can_msg.data[2] = 0xAA;
					can_msg.data[3] = 0xAA;
					can_msg.data[4]
						= (unsigned char)(myPs->misc.timer_1sec_count % 255);
					can_msg.data[5] = 0xAA;
					can_msg.data[6] = 0xAA;
					can_msg.data[7] = 0xAA;
					can_msg.length = 8; //kjh_181127

					rt_can_write(can_ch, (char *)&can_msg);
					//for(i=0; i < 10; i++) { //2000d~			//ktg_191222s
					//	can_msg.id = 0x7D0 + i;
					//	rt_can_write(can_ch, (char *)&can_msg);
					//}
					can_msg.id = 0x7D0;
					rt_can_write(can_ch, (char *)&can_msg);		//ktg_191222e
				}
			} else { //slave
				if((myPs->misc.timer_1sec_count % 2) == 0) { //20ms
					can_msg.id = 0x3E7; //999d
					can_msg.type = 0;
					can_msg.length = 8;
					can_msg.data[0]
						= (unsigned char)(myPs->misc.timer_1sec_count % 255);
					can_msg.data[1] = 0xAA;
					can_msg.data[2] = 0xAA;
					can_msg.data[3] = 0xAA;
					can_msg.data[4]
						= (unsigned char)(myPs->misc.timer_1sec_count % 255);
					can_msg.data[5] = 0xAA;
					can_msg.data[6] = 0xAA;
					can_msg.data[7] = 0xAA;
					can_msg.length = 8; //kjh_181127

					rt_can_write(can_ch, (char *)&can_msg);
					//for(i=0; i < 10; i++) { //1000d			//ktg_191222s
					//	can_msg.id = 0x3E8 + i;
					//	rt_can_write(can_ch, (char *)&can_msg);
					//}
					can_msg.id = 0x3E8;
					rt_can_write(can_ch, (char *)&can_msg);		//ktg_191222e
				}
			}
			break;
	}
}

void can_transmit2(int can_ch)
{
	unsigned char tmp3, function_phase, msg_count, msg_skip;
	short int function_div;
	int i, j, cnt_start, cnt_end, start_point, end_point, group, ch, k, l;
	int baudrate, sjw, extended_id, rtn;
	int manual_can_bitconfig, nominal_brp, nominal_tseg1, nominal_tseg2, nominal_sjw;
	long canID, bitCount;
	long long val1, period;
	unsigned long long tmp1, tmp2;
	float val2;
	U_CAN_VAL can_val, can_val2;
	S_RT_CAN_MSG can_msg;
	S_MSG_VAL SendMsg;

	group = 0; ch = 0;

	if(myData->dio.signal[DIO_SIG_IN_EXTERNAL1] == P2) {
		if(myData->CAN.signal[can_ch][CAN_SIG_START_TRANSMISSION] != P1) {
			memset((char *)&myData->canTransmitTime[can_ch/2][0], 0,
				sizeof(long) * MAX_CAN_DATA);
			myData->CAN.signal[can_ch][CAN_SIG_START_TRANSMISSION] = P1;

			switch(myData->canTransmitSetData.commonData[0][0]
				.can_baudrate) { //kjg_091125_s
				case 0: //125K
					baudrate = 125000;
					break;
				case 1: //250K
					baudrate = 250000;
					break;
				case 2: //500K
					baudrate = 500000;
					break;
				case 3: //1M
					baudrate = 1000000;
					break;
				default: //User
					baudrate = 500000; //kjg_180405 0->500000
					break;
			}
			sjw = (int)myData->canTransmitSetData.commonData[0][0].sjw;
			extended_id
				= (int)myData->canTransmitSetData.commonData[0][0].extended_id;
			//ksh_241112_s
			manual_can_bitconfig = (int)myData->canTransmitSetData.commonData[0][0].manual_can_bitconfig;
			nominal_brp = (int)myData->canTransmitSetData.commonData[0][0].nominal_brp;
			nominal_tseg1 = (int)myData->canTransmitSetData.commonData[0][0].nominal_tseg1;
			nominal_tseg2 = (int)myData->canTransmitSetData.commonData[0][0].nominal_tseg2;
			nominal_sjw = (int)myData->canTransmitSetData.commonData[0][0].nominal_sjw;
			//ksh_241112_e
#ifdef __CAN_FD__ //kjh_190706s
			i = (int)myData->canTransmitSetData.commonData[0][0].can_fd_flag;
			j =	(int)myData->canTransmitSetData.commonData[0][0].can_datarate;
			k =	(int)myData->canTransmitSetData.commonData[0][0].crc_type;
			l =	(int)myData->canTransmitSetData.commonData[0][0].terminal_r;
			
			rtl_printf("can param2 ch:%d baud:%d sjw:%d ext:%d fd_flag:%d\n",
				can_ch, baudrate, sjw, extended_id, i);
			rtl_printf("can param2 data_rate:%d crc:%d terminal_r:%d\n",
				j, k, l);

			//rtn = rt_can_set_param(0, baudrate, sjw, extended_id, i, j, k, l);
			rtn = rt_can_set_param_0(0, baudrate, sjw, extended_id, i, j, k, l);	//csk_24001
#else //CAN_2p0b
			//rtn = rt_can_set_param(0, baudrate, sjw, extended_id, 1);
			rtn = rt_can_set_param(0, baudrate, sjw, extended_id, 1, 
			manual_can_bitconfig, nominal_brp, nominal_tseg1, nominal_tseg2, nominal_sjw); //ksh_241112

			if(rtn < 0) {
				rtl_printf("kjg_d_can error2 %d, ch:%d val:%d ext:%d\n",
					rtn, can_ch, baudrate, extended_id);
				if(manual_can_bitconfig == 1) {
				rtl_printf("kjg_d_can error2 %d, ch:%d manual_can_bitconfig:%d norminal_brp:%d norminal_tseg1:%d norminal_tseg2:%d nominal_sjw:%d\n",
					rtn, can_ch, manual_can_bitconfig, nominal_brp,nominal_tseg1, nominal_tseg2, nominal_sjw);//ksh_241112
					}
			} else {
				rtl_printf("kjg_d_can set_param %d, ch:%d val:%d ext:%d\n",
					rtn, can_ch, baudrate, extended_id);
				if(manual_can_bitconfig == 1) {
				rtl_printf("kjg_d_can set_param %d, ch:%d manual_can_bitconfig:%d norminal_brp:%d norminal_tseg1:%d norminal_tseg2:%d nominal_sjw:%d\n",
					rtn, can_ch, manual_can_bitconfig, nominal_brp,nominal_tseg1, nominal_tseg2, nominal_sjw);//ksh_241112
					}
			}
#endif //kjh_190706e
			return; //kjg_091125_e
		}
	} else if(myData->dio.signal[DIO_SIG_IN_EXTERNAL1] == P3) {
		if(myData->CAN.signal[can_ch][CAN_SIG_START_TRANSMISSION] != P2) {
			myData->CAN.signal[can_ch][CAN_SIG_START_TRANSMISSION] = P2;
			myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_INV_CAP_V] = P2;
			myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MOTOR_RPM] = P2;
		}
	} else {
		myData->CAN.signal[can_ch][CAN_SIG_START_TRANSMISSION] = P0;
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_INV_CAP_V] = P1;
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MOTOR_RPM] = P1;
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MCU_TEMP] = P1;
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_ECU_ID] = P0;
		myData->CAN.signal[can_ch][CAN_SIG_RX_PHASE_FAN_SPEED] = P1;
		myData->CAN.signal[can_ch][CAN_SIG_RX_PHASE_ECU_ID] = P0;
		myData->CAN.signal[can_ch][CAN_SIG_ECU_ID_REQUEST] = P0;
	}

	switch(myData->CAN.signal[can_ch][CAN_SIG_FAN_TEST]) {
		case P0:
			break;
		case P1:
			myData->CAN.signal[can_ch][CAN_SIG_RX_PHASE_FAN_SPEED] = P2;
			myData->CAN.signal[can_ch][CAN_SIG_FAN_TEST]++;
			break;
		case P20:
			Select_OutPoint(group, ch, O_EXTERNAL1, ON);
			myData->CAN.signal[can_ch][CAN_SIG_FAN_TEST]++;
			myData->CAN.tmp_val = 0;
			break;
		case P21:
			myData->CAN.tmp_val++;
			if(myData->CAN.tmp_val >= 100) { //1sec
				myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MCU_TEMP] = P2;
				myData->CAN.signal[can_ch][CAN_SIG_RX_PHASE_FAN_SPEED] = P3;
				myData->CAN.signal[can_ch][CAN_SIG_FAN_TEST]++;
				myData->CAN.tmp_val = 0;
			}
			break;
		case P40:
			myData->CAN.tmp_val++;
			if(myData->CAN.tmp_val >= 1500) { //15sec
				Select_OutPoint(group, ch, O_EXTERNAL1, OFF);
				myData->CAN.signal[can_ch][CAN_SIG_RX_PHASE_FAN_SPEED] = P4;
				myData->CAN.signal[can_ch][CAN_SIG_FAN_TEST]++;
			}
			break;
		case P41:
			myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MCU_TEMP] = P3;
			myData->CAN.signal[can_ch][CAN_SIG_FAN_TEST]++;
			myData->CAN.tmp_val = 0;
			break;
		case P60:
			myData->CAN.tmp_val++;
			if(myData->CAN.tmp_val >= 500) { //5sec
				myData->CAN.signal[can_ch][CAN_SIG_RX_PHASE_FAN_SPEED] = P1;
				myData->CAN.signal[can_ch][CAN_SIG_FAN_TEST]++;
			}
			break;
		case P61:
			myData->CAN.tmp_val = 0;
			myData->CAN.signal[can_ch][CAN_SIG_FAN_TEST] = P0;

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_MODULE_COA_BMS_COMM_END;
			SendMsg.val[0] = can_ch;
			SendMsg.val[1] = CAN_RX_FUNC_DIV_FAN_SPEED;
			send_msg(MODULE_TO_COA1, (char *)&SendMsg);
			break;
		default:
			myData->CAN.signal[can_ch][CAN_SIG_FAN_TEST]++;
			break;
	}

	switch(myData->CAN.signal[can_ch][CAN_SIG_ECU_ID_REQUEST]) {
		case P0:
			break;
		case P1:
			myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_ECU_ID] = P1;
			myData->CAN.signal[can_ch][CAN_SIG_ECU_ID_REQUEST]++;
			break;
		case P2: //wait response
			break;
		case P3:
			myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_ECU_ID] = P2;
			myData->CAN.signal[can_ch][CAN_SIG_ECU_ID_REQUEST]++;
			break;
		case P4: //wait response
			break;
		case P5:
			myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_ECU_ID] = P3;
			myData->CAN.signal[can_ch][CAN_SIG_ECU_ID_REQUEST]++;
			break;
		case P6: //wait response
			break;
		case P7:
			myData->CAN.tmp_val = 0;
			myData->CAN.signal[can_ch][CAN_SIG_ECU_ID_REQUEST]++;
			break;
		case P8:
			myData->CAN.tmp_val++;
			if(myData->CAN.tmp_val >= 500) { //5sec
				myData->CAN.tmp_val = 0;
				myData->CAN.signal[can_ch][CAN_SIG_ECU_ID_REQUEST] = P0;

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				SendMsg.msg = MSG_MODULE_COA_BMS_COMM_END;
				SendMsg.val[0] = can_ch;
				SendMsg.val[1] = CAN_RX_FUNC_DIV_ECU_ID;
				send_msg(MODULE_TO_COA1, (char *)&SendMsg);
			}
			break;
		default: break;
	}

	if(myData->CAN.signal[can_ch][CAN_SIG_START_TRANSMISSION] == P0) return;

	if((can_ch % 2) == 0) { //master
		cnt_start = 0;
		cnt_end = myData->canTransmitDataCount[can_ch/2][0];
	} else { //slave
		cnt_start = myData->canTransmitDataCount[can_ch/2][0];
		cnt_end = cnt_start + myData->canTransmitDataCount[can_ch/2][1];
	}

	canID = -1;
	msg_count = 0;
	msg_skip = 0;
	for(i=cnt_start; i < cnt_end; i++) {
		if(canID != myData->canTransmitSetData.normalData[can_ch/2][i].canID) {
			memset((char *)&can_msg, 0, sizeof(S_RT_CAN_MSG));
			canID = myData->canTransmitSetData.normalData[can_ch/2][i].canID;
			can_msg.id = (unsigned int)canID;
			can_msg.type = 0;
			can_msg.length = 8;
			msg_count = 0;
		}
		msg_skip = 0;

		val2 = 0.0;
		function_div
			= myData->canTransmitSetData.normalData[can_ch/2][i].function_div;
		function_phase = function_div % 50;
		if(function_div == CAN_TX_FUNC_DIV_NONE) {
			val2 = myData->canTransmitSetData
				.normalData[can_ch/2][i].default_value
				/ myData->canTransmitSetData.normalData[can_ch/2][i].factor
				- myData->canTransmitSetData.normalData[can_ch/2][i].offset;
			msg_count++;
		} else if((function_div >= CAN_TX_FUNC_DIV_INV_CAP_V)
			&& (function_div < (CAN_TX_FUNC_DIV_INV_CAP_V+50))) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_INV_CAP_V]
				== function_phase) {
				val2 = myData->canTransmitSetData
					.normalData[can_ch/2][i].default_value
					/ myData->canTransmitSetData.normalData[can_ch/2][i].factor
					- myData->canTransmitSetData.normalData[can_ch/2][i].offset;
				msg_count++;
			} else msg_skip = 1;
		} else if((function_div >= CAN_TX_FUNC_DIV_MCU_TEMP)
			&& (function_div < (CAN_TX_FUNC_DIV_MCU_TEMP+50))) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MCU_TEMP]
				== function_phase) {
				val2 = myData->canTransmitSetData
					.normalData[can_ch/2][i].default_value
					/ myData->canTransmitSetData.normalData[can_ch/2][i].factor
					- myData->canTransmitSetData.normalData[can_ch/2][i].offset;
				msg_count++;
			} else msg_skip = 1;
		} else if((function_div >= CAN_TX_FUNC_DIV_ECU_ID)
			&& (function_div < (CAN_TX_FUNC_DIV_ECU_ID+50))) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_ECU_ID]
				== function_phase) {
				val2 = myData->canTransmitSetData
					.normalData[can_ch/2][i].default_value
					/ myData->canTransmitSetData.normalData[can_ch/2][i].factor
					- myData->canTransmitSetData.normalData[can_ch/2][i].offset;
				msg_count++;
			} else msg_skip = 1;
		} else if((function_div >= CAN_TX_FUNC_DIV_MOTOR_RPM)
			&& (function_div < (CAN_TX_FUNC_DIV_MOTOR_RPM+50))) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MOTOR_RPM]
				== function_phase) {
				val2 = myData->canTransmitSetData
					.normalData[can_ch/2][i].default_value
					/ myData->canTransmitSetData.normalData[can_ch/2][i].factor
					- myData->canTransmitSetData.normalData[can_ch/2][i].offset;
				msg_count++;
			} else msg_skip = 1;
		}

		if(msg_skip != 1) {
			val1 = (long long)val2;
			if(myData->canTransmitSetData.normalData[can_ch/2][i].data_type
				== 0) { //unsigned
				if(val1 < 0) val1 *= (-1);
			}

			start_point = myData->canTransmitSetData.normalData[can_ch/2][i]
				.startBit;
			bitCount = myData->canTransmitSetData.normalData[can_ch/2][i]
				.bitCount;
			end_point = start_point + bitCount;

			memcpy((char *)&tmp1, (char *)&val1, sizeof(long long));
			//kjg_101028
			if(myData->canTransmitSetData.normalData[can_ch/2][i].byte_order
				== 0) { //little_endian(intel)
			} else { //big_endian(motorola)
				memset((char *)&can_val, 0, sizeof(U_CAN_VAL));
				memcpy((char *)&can_val2, (char *)&tmp1, sizeof(U_CAN_VAL));
				k = (bitCount - 1) / 8;
				if(k < 0) k = 0;
				for(l=0; l <= k; l++) {
					can_val.c_val[k-l-1] = can_val2.c_val[l];
				}
				memcpy((char *)&tmp1, (char *)&can_val, sizeof(long long));
			}
			tmp1 = tmp1 << start_point;

			tmp2 = 0x01; tmp3 = 0;
			for(j=0; j < 64; j++) {
				if((j % 8) == 0) tmp3 = 0x01;
				if((j >= start_point) && (j < end_point)) {
					if((tmp1 & tmp2) != 0) {
						can_msg.data[j / 8] |= tmp3;
					}
				}
				tmp2 = tmp2 << 1;
				tmp3 = tmp3 << 1;
			}
		}

		j = 0;
		if(myData->canTransmitSetData.normalData[can_ch/2][i+1].canID
			!= canID) {
			j = 1;
		}

		tmp3 = 0;
		if((i == (cnt_end - 1)) || (j == 1)) {
			if(myData->canTransmitSetData.normalData[can_ch/2][i].send_period
				== 0) {
				tmp3 = 0;
				myData->canTransmitTime[can_ch/2][i] = 0;
			} else if(myData->canTransmitTime[can_ch/2][i] == 0) {
				tmp3 = 1;
				myData->canTransmitTime[can_ch/2][i] += 1;
			} else {
				//ktg_211111s
				if(myData->canTransmitSetData.normalData[can_ch/2][i].send_period <
					myPs->config.scan_period) {
						myData->canTransmitSetData.normalData[can_ch/2][i].send_period
							= myPs->config.scan_period;
				}	//ktg_211111e
				period = ((myData->canTransmitTime[can_ch/2][i] * 10)
					% myData->canTransmitSetData.normalData[can_ch/2][i]
					.send_period);
				if(period == 0) {
					tmp3 = 2;
					myData->canTransmitTime[can_ch/2][i] = 0;
				}
				myData->canTransmitTime[can_ch/2][i] += 1;
			}
		}
		if((tmp3 != 0) && (msg_count != 0)) {
			rt_can_write(can_ch, (char *)&can_msg);

			if((function_div >= CAN_TX_FUNC_DIV_ECU_ID)
				&& (function_div < (CAN_TX_FUNC_DIV_ECU_ID+50))) {
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_ECU_ID] != P0)
					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_ECU_ID] = P0;
			}
		}
	}
}

void can_transmit3(int can_ch)
{
	unsigned char tmp3, function_phase, msg_count, msg_skip;
	short int function_div;
	int i, j, cnt_start, cnt_end, start_point, end_point, k, ch; //l;
	int module_ch, master_slave, start_point2;
	unsigned int msg_count2;
	long canID, bitCount, tmp_val1;
	long long val1, period;
	unsigned long long tmp1, tmp2;
	float default_value, val2;
	float tmp_val2[MAX_CAN_DATA / 2]; //kjhw_131022
	int flag1, flag2, flag3, flag4, flag5;	//jhkw_150611
	float ratioV, ratioI, ratioP; //kjh_211021

	U_CAN_VAL can_val;//, can_val2;
	S_RT_CAN_MSG can_msg;

	switch(myData->mData.config.division_CAN) { //kjhw_140620
		case 1:
			module_ch = can_ch;
			master_slave = 0;
			break;
		default:
			switch(myData->AppControl.config.systemModel) {
				case C_KBIA_500V_250A_100A_250KW:	 //shh_200916
				case C_JBTP_500V_250A_100A_500KW:	 //shh_200927
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
				case C_KCL_600V_200A_100A_480KW: 	  //ktg_190812
				case C_KCL_600V_200A_100A_480KW_2: 	  //ktg_190812
				case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
				case C_ADD_800V_350A_200A_50A_380KW:  //shh_200904
				case C_HYUNDAI_1000V_250A_100A_500KW: //shh_200513
				case C_KOSTA_1000V_250A_100A_600KW:	  //shh_201102
				case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
				case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
				case C_KTC_1500V_600A_200A_400KW:	  //ktg_200807
					module_ch = can_ch / 2;
					if(module_ch == 1) module_ch = 2;
					master_slave = can_ch % 2;
					break;
				case C_SK_450V_200A_10A_360KW:
					module_ch = can_ch / 2;
					if(module_ch == 1) module_ch = 3;
					master_slave = can_ch % 2;
					break;
				default:
					module_ch = can_ch / 2;
					master_slave = can_ch % 2;
					break;
			}	//kjhw_170630e
			break;
	}

	if(master_slave == 0) { //master
		cnt_start = 0;
		cnt_end = myData->canTransmitDataCount[module_ch][0];
	} else { //slave
		cnt_start = myData->canTransmitDataCount[module_ch][0];
		cnt_end = cnt_start + myData->canTransmitDataCount[module_ch][1];
	}

	ratioV = myData->mData.patt_ratioV; //kjh_220321
	ratioI = myData->mData.patt_ratioI; //kjh_220321
	ratioP = myData->mData.patt_ratioP; //kjh_220321

	canID = -1;
	msg_count = 0;
	msg_count2 = 0;
	msg_skip = 0;
	for(i=cnt_start; i < cnt_end; i++) {
		if(canID != myData->canTransmitSetData.normalData[module_ch][i].canID) {
			memset((char *)&can_msg, 0, sizeof(S_RT_CAN_MSG));
			canID = myData->canTransmitSetData.normalData[module_ch][i].canID;
			can_msg.id = (unsigned int)canID;
			can_msg.type = 0;
			function_div = myData->canTransmitSetData.normalData[module_ch][i]
				.function_div2;
			if(function_div >= CAN_TX_FUNC_DIV_DLC_1
				&& function_div <= CAN_TX_FUNC_DIV_DLC_8) { //kjg_120227
				can_msg.length = (function_div % 550);
			} else {
				if(myData->CAN.config.commType == 0) {
					if(myData->canTransmitSetData.normalData[module_ch][i].dlc > 8) {
						can_msg.length = 8;
					} else {
						can_msg.length = myData->canTransmitSetData
							.normalData[module_ch][i].dlc;
					}
				} else {
					can_msg.length = myData->canTransmitSetData
						.normalData[module_ch][i].dlc; //kjg_180405 8->dlc
				}
			}
			msg_count = 0;
			msg_count2 = 0;
		}
		msg_skip = 0;

		val2 = 0.0;
		function_div
			= myData->canTransmitSetData.normalData[module_ch][i].function_div;
		if(myData->canTransmitSetData.commonData[module_ch][0].bms_type
			== 0) {
			function_phase = function_div % 50;
		} else if(myData->canTransmitSetData.commonData[module_ch][0].bms_type
			== 50) { //kjg_161207
			function_phase = function_div % 100;
		} else {
			function_phase = function_div % 50;
		}

		if(function_div == CAN_TX_FUNC_DIV_NONE) {
			default_value = FindCanChangeData(module_ch, i);
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if((function_div >= CAN_TX_FUNC_DIV_CHARGER_DC_V)
			&& (function_div < (CAN_TX_FUNC_DIV_CHARGER_DC_V+50))) {
			switch(function_div) {
				case CAN_TX_FUNC_DIV_CHARGER_DC_V:
					val2 = (float)myData->cData[0].op.Vsens / ratioV
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_CHARGER_DC_I:
					val2 = (float)myData->cData[0].op.Isens / ratioI
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				default:
					default_value = FindCanChangeData(module_ch, i);
					val2 = default_value
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
			}
			msg_count++;
		} else if((function_div >= CAN_TX_FUNC_DIV_CHARGER_PHASE1)
			&& (function_div < (CAN_TX_FUNC_DIV_CHARGER_PHASE1+50))) {
			if(myData->CAN.signal[module_ch][CAN_SIG_TX_PHASE_CHARGER]
				== function_phase) {
				default_value = FindCanChangeData(module_ch, i);
				val2 = default_value
					/ myData->canTransmitSetData.normalData[module_ch][i].factor
					- myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else msg_skip = 1;
		} else if(function_div == CAN_TX_FUNC_DIV_DEBUG_ON) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_DEBUG_ON] == P1) {
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					/ myData->canTransmitSetData.normalData[module_ch][i].factor
					- myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else msg_skip = 1;
		} else if(function_div == CAN_TX_FUNC_DIV_CALIBRATION_CHARGE) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_CALIBRATION]
				== P1) {
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					/ myData->canTransmitSetData.normalData[module_ch][i].factor
					- myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else msg_skip = 1;
		} else if(function_div == CAN_TX_FUNC_DIV_CALIBRATION_DISCHARGE) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_CALIBRATION]
				== P3) {
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					/ myData->canTransmitSetData.normalData[module_ch][i].factor
					- myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else msg_skip = 1;
		} else if(function_div == CAN_TX_FUNC_DIV_FLASH_REQUEST) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_FLASH_CMD] == P1) {
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					/ myData->canTransmitSetData.normalData[module_ch][i].factor
					- myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else msg_skip = 1;
		} else if(function_div == CAN_TX_FUNC_DIV_FLASH_RESET) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_FLASH_CMD] == P3) {
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					/ myData->canTransmitSetData.normalData[module_ch][i].factor
					- myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else msg_skip = 1;
		} else if((function_div >= CAN_TX_FUNC_DIV_DAQ_MUX)
			&& (function_div < (CAN_TX_FUNC_DIV_DAQ_MUX+50))) {
			//kjg_110921 if(myData->CAN.signal[module_ch][CAN_SIG_TX_PHASE_DAQ_MUX]
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_DAQ_MUX]
				== function_phase) {
				default_value = FindCanChangeData(module_ch, i);
				val2 = default_value
					/ myData->canTransmitSetData.normalData[module_ch][i].factor
					- myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else msg_skip = 1;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_CHARGE_V) {
			if(master_slave == 0) { //master
				default_value = FindExternalCanData(module_ch, function_div);
			} else { //slave
				default_value = FindCanChangeData(module_ch, i);
			}
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_DISCHARGE_V) {
			if(master_slave == 0) { //master
				default_value = FindExternalCanData(module_ch, function_div);
			} else { //slave
				default_value = FindCanChangeData(module_ch, i);
			}
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_SET_VERSION) {
			if(master_slave == 0) { //master
				default_value = FindExternalCanData(module_ch, function_div);
			} else { //slave
				default_value = FindCanChangeData(module_ch, i);
			}
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_SET_MODE) {
			if(master_slave == 0) { //master
				default_value = FindExternalCanData(module_ch, function_div);
			} else { //slave
				default_value = FindCanChangeData(module_ch, i);
			}
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_SET_WAVEFORM) {
			if(master_slave == 0) { //master
				default_value = FindExternalCanData(module_ch, function_div);
			} else { //slave
				default_value = FindCanChangeData(module_ch, i);
			}
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_CMD_I_START) {
			if(master_slave == 0) { //master
				default_value = FindExternalCanData(module_ch, function_div);
			} else { //slave
				default_value = FindCanChangeData(module_ch, i);
			}
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_CMD_I_END) {
			if(master_slave == 0) { //master
				default_value = FindExternalCanData(module_ch, function_div);
			} else { //slave
				default_value = FindCanChangeData(module_ch, i);
			}
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_CMD_P_START) {
			if(master_slave == 0) { //master
				default_value = FindExternalCanData(module_ch, function_div);
			} else { //slave
				default_value = FindCanChangeData(module_ch, i);
			}
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_CMD_P_END) {
			if(master_slave == 0) { //master
				default_value = FindExternalCanData(module_ch, function_div);
			} else { //slave
				default_value = FindCanChangeData(module_ch, i);
			}
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_CMD_T) {
			if(master_slave == 0) { //master
				default_value = FindExternalCanData(module_ch, function_div);
			} else { //slave
				default_value = FindCanChangeData(module_ch, i);
			}
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_SET_CONTROL) {
			if(master_slave == 0) { //master
				default_value = FindExternalCanData(module_ch, function_div);
			} else { //slave
				default_value = FindCanChangeData(module_ch, i);
			}
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_RECEIVE_PARSING_PERIOD) {
			if(master_slave == 0) { //master
				default_value = FindExternalCanData(module_ch, function_div);
			} else { //slave
				default_value = FindCanChangeData(module_ch, i);
			}
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_CAN_TRANSMIT_PERIOD) {
			//kjg_140916
			if(master_slave == 0) { //master
				default_value = FindExternalCanData(module_ch, function_div);
			} else { //slave
				default_value = FindCanChangeData(module_ch, i);
			}
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_V_SENS) {
			//val2 = (float)((double)myData->cData[module_ch].op.Vsens
			//kjhw_150129s
			if(myData->cData[module_ch].op.state == C_RUN
				&& myData->cData[module_ch].op.phase != P50) {
			} else {
				myData->cData[module_ch].misc.tmpVsens2
					= myData->cData[module_ch].misc.tmpVsens;
			}
			val2 = (float)((double)myData->cData[module_ch].misc.tmpVsens2
				/ ratioV
				* (double)myData->canTransmitSetData.normalData[module_ch][i]
				.factor)
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;	//kjhw_150129e
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_I_SENS) {
			//tmp_val1 = myData->cData[module_ch].op.Isens;
			if(myData->cData[module_ch].op.state == C_RUN	//kjhw_150129s
				&& myData->cData[module_ch].op.phase != P50) {
			} else {
				myData->cData[module_ch].misc.tmpIsens2
					= myData->cData[module_ch].misc.tmpIsens;
			}	//kjhw_150129e
			tmp_val1 = myData->cData[module_ch].misc.tmpIsens2;
			for(j=0; j < MAX_SLAVE_CH; j++) {
				k = (int)myData->ChAttribute[module_ch].chNo_slave[j] - 1;
				if(k >= 0) {
					//tmp_val1 += myData->cData[k].op.Isens;
					//kjhw_150129s
					if(myData->cData[k].op.state == C_RUN
						&& myData->cData[k].op.phase != P50) {
					} else {
						myData->cData[k].misc.tmpIsens2
							= myData->cData[k].misc.tmpIsens;
					}
					//kjhw_150129e
					tmp_val1 += myData->cData[k].misc.tmpIsens2;
				}
			}
			val2 = (float)((double)tmp_val1 / ratioI
				* (double)myData->canTransmitSetData.normalData[module_ch][i]
				.factor)
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_P) {
			//tmp_val1 = myData->cData[module_ch].op.watt;
			if(myData->cData[module_ch].op.state == C_RUN //kjhw_150129s
				&& myData->cData[module_ch].op.phase != P50) {
			} else {
				myData->cData[module_ch].misc.tmpWatt2
					= myData->cData[module_ch].misc.tmpWatt;
			}	//kjhw_150129e
			tmp_val1 = myData->cData[module_ch].misc.tmpWatt2;
			for(j=0; j < MAX_SLAVE_CH; j++) {
				k = (int)myData->ChAttribute[module_ch].chNo_slave[j] - 1;
				if(k >= 0) {
					//tmp_val1 += myData->cData[k].op.watt;
					//kjhw_150129s
					if(myData->cData[k].op.state == C_RUN
						&& myData->cData[k].op.phase != P50) {
					} else {
						myData->cData[k].misc.tmpWatt2
							= myData->cData[k].misc.tmpWatt;
					}
					//kjhw_150129e
					tmp_val1 += myData->cData[k].misc.tmpWatt2;
				}
			}
			val2 = (float)((double)tmp_val1 / ratioP
				* (double)myData->canTransmitSetData.normalData[module_ch][i]
				.factor)
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_WH) {
			tmp_val1 = myData->cData[module_ch].op.charge_WattHour;
			tmp_val1 += myData->cData[module_ch].op.discharge_WattHour;
			for(j=0; j < MAX_SLAVE_CH; j++) {
				k = (int)myData->ChAttribute[module_ch].chNo_slave[j] - 1;
				if(k >= 0) {
					tmp_val1 += myData->cData[k].op.charge_WattHour;
					tmp_val1 += myData->cData[k].op.discharge_WattHour;
				}
			}
			val2 = (float)((double)tmp_val1 / ratioP
				* (double)myData->canTransmitSetData.normalData[module_ch][i]
				.factor)
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_AH) {
			tmp_val1 = myData->cData[module_ch].op.charge_AmpareHour;
			tmp_val1 += myData->cData[module_ch].op.discharge_AmpareHour;
			for(j=0; j < MAX_SLAVE_CH; j++) {
				k = (int)myData->ChAttribute[module_ch].chNo_slave[j] - 1;
				if(k >= 0) {
					tmp_val1 += myData->cData[k].op.charge_AmpareHour;
					tmp_val1 += myData->cData[k].op.discharge_AmpareHour;
				}
			}
			switch(myData->AppControl.config.systemModel) {
				case C_CANSYSTEM_60V_350A_100A_21KW: //kjg_140916
				case C_LGE_450V_250A_50A_112KW:		//jhk_150703
				//case C_LGE_450V_250A_50A_225KW:		//jhk_150605 //phb_220628
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
					//kjh_220321 val2 = (float)((double)tmp_val1 / 1000.0 * ratioI	//mAh
					val2 = (float)((double)tmp_val1 / ratioI * 1000	//mAh
						* (double)myData->canTransmitSetData
						.normalData[module_ch][i].factor)
						+ myData->canTransmitSetData
						.normalData[module_ch][i].offset;
					break;
				default:
					val2 = (float)((double)tmp_val1 / ratioI	//Ah
						* (double)myData->canTransmitSetData
						.normalData[module_ch][i].factor)
						+ myData->canTransmitSetData
						.normalData[module_ch][i].offset;
					break;
			}
			msg_count++;
		//jhkw_150418s
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_CABLE_FAIL) {
			if(Read_InPoint(0, module_ch+1, I_CABLE_FAIL) == ON) {
				val2 = 1
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			} else {
				val2 = 0
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			}
			msg_count++;	//jhkw_150418e
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_STATE) {
			val2 = (float)myData->cData[module_ch].op.state
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			val2 = (float)convert_ch_state(CONVERT_ORG_TO_P1, (long)val2);
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_STEP_TYPE) {
			val2 = (float)myData->cData[module_ch].op.stepType
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			val2 = (float)convert_step_type(CONVERT_ORG_TO_P1, (long)val2);
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_CODE) {
			val2 = (float)myData->cData[module_ch].op.code
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			val2 = (float)convert_ch_code(CONVERT_ORG_TO_P1, (long)val2);
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_STEP_NO) {
			val2 = (float)myData->cData[module_ch].op.idxStepNo
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_CURRENT_CYCLE) {
			val2 = (float)myData->cData[module_ch].elementCycle.cycle_count
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_TOTAL_CYCLE) {
			val2 = (float)myData->cData[module_ch].misc.totalCycle
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_GROUP_CYCLE1) {
			val2 = (float)myData->cData[module_ch].accCycle[0].cycle_count
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_GROUP_CYCLE2) {
			val2 = (float)myData->cData[module_ch].accCycle[1].cycle_count
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_GROUP_CYCLE3) {
			val2 = (float)myData->cData[module_ch].accCycle[2].cycle_count
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_GROUP_CYCLE4) {
			val2 = (float)myData->cData[module_ch].accCycle[3].cycle_count
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_GROUP_CYCLE5) {
			val2 = (float)myData->cData[module_ch].accCycle[4].cycle_count
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_MULTI_CYCLE1) {
			val2 = (float)myData->cData[module_ch].multiCycle[0].cycle_count
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_MULTI_CYCLE2) {
			val2 = (float)myData->cData[module_ch].multiCycle[1].cycle_count
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_MULTI_CYCLE3) {
			val2 = (float)myData->cData[module_ch].multiCycle[2].cycle_count
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_MULTI_CYCLE4) {
			val2 = (float)myData->cData[module_ch].multiCycle[3].cycle_count
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_MULTI_CYCLE5) {
			val2 = (float)myData->cData[module_ch].multiCycle[4].cycle_count
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_AUX_DATA) { //kjhw_131022s
			if(master_slave == 0) { //master
			} else { //slave
				k = ch = 0;
				for(j=0; j < MAX_AUX_DATA; j++) {
					if(module_ch == myData->auxSetData[j].chNo - 1) {
						ch = myData->auxSetData[j].auxChNo - 1;
						if(myData->auxSetData[j].auxType == 0) { //temperature
							//tmp_val2[k] = (float)myData->COM.com_port[1]
							//	.value[ch] / 1000.0;
							tmp_val2[k] = (float)myData->COM.com_port[AUX_TEMP]
								.value[ch] / 1000.0; //KHK_190903
						} else {
							//csk_200303s
							switch(myData->AppControl.config.systemModel) {
								/*case C_LGC_500V_250A_125A_50A_500KW_10:
								case C_LGC_500V_250A_125A_50A_500KW_11:
									if((ch >= 240) && (ch < 272)) ch += 16;
									else if((ch >= 256) && (ch < 288)) ch -= 16;
									break;*/
								case C_LGC_600V_300A_100A_360KW_2:
									if((ch >= 120) && (ch < 132)) ch += 8;
									break;
								default: break;
							}	//csk_200303e
							tmp_val2[k] = (float)myData->SubSensV.ch[ch]
								.sensV / 1000000.0;
						}
						k++;
					}
				}
				k = i - myData->canTransmitDataCount[module_ch][0];
				val2 = (float)((double)tmp_val2[k]
					* (double)myData->canTransmitSetData
					.normalData[module_ch][i].factor)
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
			}
			msg_count++;
		} else if((function_div == CAN_TX_FUNC_DIV_AUXV_DATA1)
			|| (function_div == CAN_TX_FUNC_DIV_AUXV_DATA2)
			|| (function_div == CAN_TX_FUNC_DIV_AUXV_DATA3)) { //ktg_220818s
			k = ch = 0;
			for(j=0; j < MAX_AUX_DATA; j++) {
				if(module_ch == myData->auxSetData[j].chNo - 1) {
					ch = myData->auxSetData[j].auxChNo - 1;
					if(myData->auxSetData[j].auxType == 1) {
						/*switch(myData->AppControl.config.systemModel) {
							case C_SEINENG_200V_300A_150A_200KW:	//shh_220119
							case C_SEINENG_200V_300A_150A_200KW_2:	//shh_220119
							case C_SEINENG_200V_300A_150A_200KW_3:	//shh_220119
							case C_SEINENG_200V_300A_150A_200KW_4:	//shh_220119
							case C_SEINENG_200V_300A_150A_200KW_5:	//shh_220119
								if((ch >= 24) && (ch < 48)) ch += 8;
								else if((ch >= 48) && (ch < 72)) ch += 16;
								else if((ch >= 72) && (ch < 96)) ch += 24;
								else if((ch >= 96) && (ch < 136)) ch += 32;
								else if((ch >= 32) && (ch < 56)) ch -= 8;
								else if((ch >= 64) && (ch < 88)) ch -= 16;
								else if((ch >= 96) && (ch < 120)) ch -= 24;
								else if((ch >= 128) && (ch < 168)) ch -= 32;
								break;
							default: break;
 						}*/
						tmp_val2[k] = (float)myData->SubSensV.ch[ch]
							.sensV / 1000000.0;
						k++;
					}
					if(k>3) break;
				}
			}
			val2 = (float)((double)tmp_val2[function_div % 10]
				* (double)myData->canTransmitSetData
				.normalData[module_ch][i].factor)
				+ myData->canTransmitSetData.normalData[module_ch][i]
				.offset;
			msg_count++;
		} else if((function_div == CAN_TX_FUNC_DIV_AUXTH_DATA1)
			|| (function_div == CAN_TX_FUNC_DIV_AUXTH_DATA2)) {
			k = ch = 0;
			for(j=0; j < MAX_AUX_DATA; j++) {
				if(module_ch == myData->auxSetData[j].chNo - 1) {
					ch = myData->auxSetData[j].auxChNo - 1;
					if(myData->auxSetData[j].auxType == 2) {
						/*switch(myData->AppControl.config.systemModel) {
							case C_SEINENG_200V_300A_150A_200KW:	//shh_220119
							case C_SEINENG_200V_300A_150A_200KW_2:	//shh_220119
							case C_SEINENG_200V_300A_150A_200KW_3:	//shh_220119
							case C_SEINENG_200V_300A_150A_200KW_4:	//shh_220119
							case C_SEINENG_200V_300A_150A_200KW_5:	//shh_220119
								if((ch >= 24) && (ch < 48)) ch += 8;
								else if((ch >= 48) && (ch < 72)) ch += 16;
								else if((ch >= 72) && (ch < 96)) ch += 24;
								else if((ch >= 96) && (ch < 136)) ch += 32;
								else if((ch >= 32) && (ch < 56)) ch -= 8;
								else if((ch >= 64) && (ch < 88)) ch -= 16;
								else if((ch >= 96) && (ch < 120)) ch -= 24;
								else if((ch >= 128) && (ch < 168)) ch -= 32;
								break;
							default: break;
						}*/
						tmp_val2[k] = (float)myData->SubSensV.ch[ch]
							.sensV / 1000000.0;
						k++;
					}
					if(k>2) break;
				}
			}
			val2 = (float)((double)tmp_val2[function_div % 10]
				* (double)myData->canTransmitSetData
				.normalData[module_ch][i].factor)
				+ myData->canTransmitSetData.normalData[module_ch][i]
				.offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_AUXV_MAX) {
			tmp_val2[0] = (float)myData->cData[module_ch].misc.maxAuxV / 1000000.0;
			val2 = (float)((double)tmp_val2[0]
				* (double)myData->canTransmitSetData
				.normalData[module_ch][i].factor)
				+ myData->canTransmitSetData.normalData[module_ch][i]
				.offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_AUXV_MIN) {
			tmp_val2[0] = (float)myData->cData[module_ch].misc.minAuxV / 1000000.0;
			val2 = (float)((double)tmp_val2[0]
				* (double)myData->canTransmitSetData
				.normalData[module_ch][i].factor)
				+ myData->canTransmitSetData.normalData[module_ch][i]
				.offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_AUXTH_MAX) {
			tmp_val2[0] = (float)myData->cData[module_ch].misc.maxAuxTH / 1000000.0;
			val2 = (float)((double)tmp_val2[0]
				* (double)myData->canTransmitSetData
				.normalData[module_ch][i].factor)
				+ myData->canTransmitSetData.normalData[module_ch][i]
				.offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_AUXTH_MIN) {
			tmp_val2[0] = (float)myData->cData[module_ch].misc.minAuxTH / 1000000.0;
			val2 = (float)((double)tmp_val2[0]
				* (double)myData->canTransmitSetData
				.normalData[module_ch][i].factor)
				+ myData->canTransmitSetData.normalData[module_ch][i]
				.offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_STOP_FLAG) {
			if(myData->cData[module_ch].op.state == C_PAUSE) {
				val2 = (10 * (double)myData->canTransmitSetData
					.normalData[module_ch][i].factor)
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
			} else {
				val2 = 0;
			}
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_STEP_MODE) {
			val2 = (float)myData->cData[module_ch].op.stepMode
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;	//ktg_220818e
		} else if((function_div == CAN_TX_FUNC_DIV_EXT_TEMP_DATA1)
			|| (function_div == CAN_TX_FUNC_DIV_EXT_TEMP_DATA2)	//khj_200325
			|| (function_div == CAN_TX_FUNC_DIV_EXT_TEMP_DATA3)	//khj_200325
			|| (function_div == CAN_TX_FUNC_DIV_EXT_TEMP_DATA4)	//khj_200325
			|| (function_div == CAN_TX_FUNC_DIV_EXT_TEMP_DATA5)) {	//khj_200325
			k = 0;
			for(j = 0; j < myPs->config.installedTemp; j++) {
				if(k == 5) break;

				if(module_ch == myData->auxSetData[j].chNo - 1) {	//CH 1,2,3,4
					tmp_val2[k] = (float)myData->COM.com_port[AUX_TEMP].value[j] / 1000.0;	
					k++;
				}
			}
			switch(function_div) {
				case CAN_TX_FUNC_DIV_EXT_TEMP_DATA1 : 
					val2 = (float)((double)tmp_val2[0]
						* myData->canTransmitSetData.normalData[module_ch][i].factor)
						+ myData->canTransmitSetData.normalData[module_ch][i].offset;
					myData->test_val_f[0] = tmp_val2[0];
					msg_count++;
					break;			
				case CAN_TX_FUNC_DIV_EXT_TEMP_DATA2 : 
					val2 = (float)((double)tmp_val2[1]
						* myData->canTransmitSetData.normalData[module_ch][i].factor)
						+ myData->canTransmitSetData.normalData[module_ch][i].offset;
					myData->test_val_f[0] = tmp_val2[0];
					msg_count++;
					break;			
				case CAN_TX_FUNC_DIV_EXT_TEMP_DATA3 : 
					val2 = (float)((double)tmp_val2[2]
						* myData->canTransmitSetData.normalData[module_ch][i].factor)
						+ myData->canTransmitSetData.normalData[module_ch][i].offset;
					myData->test_val_f[0] = tmp_val2[0];
					msg_count++;
					break;			
				case CAN_TX_FUNC_DIV_EXT_TEMP_DATA4 : 
					val2 = (float)((double)tmp_val2[3]
						* myData->canTransmitSetData.normalData[module_ch][i].factor)
						+ myData->canTransmitSetData.normalData[module_ch][i].offset;
					myData->test_val_f[0] = tmp_val2[0];
					msg_count++;
					break;			
				case CAN_TX_FUNC_DIV_EXT_TEMP_DATA5 : 
					val2 = (float)((double)tmp_val2[4]
						* myData->canTransmitSetData.normalData[module_ch][i].factor)
						+ myData->canTransmitSetData.normalData[module_ch][i].offset;
					myData->test_val_f[0] = tmp_val2[0];
					msg_count++;
					break;	
				default: break;
			}		
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_I_DATA) {		//khj_200325
			val2 = (float)myData->cData[module_ch].op.Isens / ratioI	//A
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_MODULE_STATE) { //kjg_140916_s
			val2 = (float)myData->mData.state
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_EMG_CODE) {
			val2 = (float)myData->mData.code
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			val2 = (float)convert_group_trouble_code(CONVERT_ORG_TO_P1, (int)val2);
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_OP_TYPE) {
			val2 = (float)myData->cData[module_ch].signal[C_SIG_EXT_OP_TYPE]
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_OP_STATE) {
			if(myData->cData[module_ch].op.state == C_RUN
				&& myData->cData[module_ch].op.phase == P50) {
				j = 1; //Standby
			} else {
				j = 0; //Idle
			}
			val2 = (float)j
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_RUN_T) {
			val2 = (float)myData->cData[module_ch].op.runTime
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_TOTAL_RUN_T) {
			val2 = (float)(myData->cData[module_ch].op.totalRunTime_day
				* 86400 + myData->cData[module_ch].op.totalRunTime / 100)
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset;
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_CHG_NM_STAT) {
			if(myData->cData[module_ch].op.state == C_RUN) {
				val2 = (float)2.0
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData
					.normalData[module_ch][i].offset;
			} else {
				val2 = (float)0.0
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData
					.normalData[module_ch][i].offset;
			}
			msg_count++;
		//kjhw_150216s
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_PACK_ISOLATION) {
			if(Read_InPoint(0, module_ch+1, I_OUT_PACK_ISOLATION) == ON) {
				val2 = (float)2.0
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData
					.normalData[module_ch][i].offset;
			} else {
				val2 = (float)1.0
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData
					.normalData[module_ch][i].offset;
			}
			msg_count++;
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_CH_ATTRIBUTE) {
			if(module_ch != 0) break;
			if(myData->mData.signal[C_SIG_EXT_CAN_CH_ATTRIBUTE] != P0) break;
			if(myData->mData.config.installedCh == 1) break;
			if(myData->ChAttribute[module_ch].opType == OP_PARALLEL) {
				val2 = (float)2.0
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData
					.normalData[module_ch][i].offset;
			} else {
				val2 = (float)1.0
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData
					.normalData[module_ch][i].offset;
			}
			msg_count++;	//kjhw_150216e
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_CH_DIV) {	//jhkw_150611s
			flag1 = Read_InPoint(0, module_ch+1, I_OUT_PACK_ISOLATION2);
			flag2 = Read_InPoint(0, module_ch+1, I_OUT_PACK_ISOLATION3);
			flag3 = Read_InPoint(0, module_ch+1, I_OUT_PACK_ISOLATION4);
			flag4 = Read_InPoint(0, module_ch+1, I_OUT_PACK_ISOLATION5);
			flag5 = Read_InPoint(0, module_ch+1, I_OUT_PACK_ISOLATION6);
			if(flag1 == OFF && flag2 == OFF && flag3 == OFF && flag4 == OFF
					&& flag5 == OFF) {
				val2 = (float)0.0
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData
					.normalData[module_ch][i].offset;
			} else if(flag1 == ON && flag2 == OFF && flag3 == OFF
					&& flag4 == OFF && flag5 == OFF) {
				val2 = (float)1.0
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData
					.normalData[module_ch][i].offset;
			} else if(flag1 == OFF && flag2 == ON && flag3 == OFF
					&& flag4 == OFF && flag5 == OFF) {
				val2 = (float)2.0
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData
					.normalData[module_ch][i].offset;
			} else if(flag1 == OFF && flag2 == OFF && flag3 == ON
					&& flag4 == OFF && flag5 == OFF) {
				val2 = (float)3.0
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData
					.normalData[module_ch][i].offset;
			} else if(flag1 == OFF && flag2 == OFF && flag3 == OFF
					&& flag4 == ON && flag5 == OFF) {
				val2 = (float)4.0
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData
					.normalData[module_ch][i].offset;
			} else if(flag1 == OFF && flag2 == OFF && flag3 == OFF
					&& flag4 == OFF && flag5 == ON) {
				val2 = (float)5.0
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData
					.normalData[module_ch][i].offset;
			} else {
				val2 = (float)10.0
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData
					.normalData[module_ch][i].offset;
			}
			msg_count++;	//jhkw_150611e
		//kjhw_160519s
		} else if(function_div == CAN_TX_FUNC_DIV_EXT_OUT_RELAY_STATE) {
			if(myData->ChAttribute[module_ch].opType == OP_PARALLEL) {
				flag1 = Read_InPoint(0, module_ch+1, I_I_RANGE1);
				flag2 = Read_InPoint(0, module_ch+1, I_PARALLEL);
				flag3 = Read_InPoint(0, module_ch+2, I_I_RANGE1);

				if(flag1 == ON || flag2 == ON || flag3 == ON) {
					val2 = (float)1.0
						* myData->canTransmitSetData.normalData[module_ch][i]
							.factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
							.offset;
				} else {
					val2 = (float)0.0
						* myData->canTransmitSetData.normalData[module_ch][i]
							.factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
							.offset;
				}
			} else {
				flag1 = Read_InPoint(0, module_ch+1, I_I_RANGE1);
				flag2 = Read_InPoint(0, module_ch+1, I_PARALLEL);
				if(flag1 == ON || flag2 == ON) {
					val2 = (float)1.0
						* myData->canTransmitSetData.normalData[module_ch][i]
							.factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
							.offset;
				} else {
					val2 = (float)0.0
						* myData->canTransmitSetData.normalData[module_ch][i]
							.factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
							.offset;
				}
			}
			msg_count++;	//kjhw_160519e
		} else if((function_div >= CAN_TX_FUNC_DIV_EXT_TEST_FAULT_CLEAR)
			&& (function_div <= CAN_TX_FUNC_DIV_EXT_TEST_CAN_TRANSMIT_PERIOD)) {
			switch(function_div) {
				case CAN_TX_FUNC_DIV_EXT_TEST_FAULT_CLEAR:
					//kjg_240213 val2 = (float)myData->test_val_l1[1]
					val2 = (float)myData->test_can_val_l[1]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_EXT_TEST_E48_STAT_ERROR:
					//kjg_240213 val2 = (float)myData->test_val_l1[2]
					val2 = (float)myData->test_can_val_l[2]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_EXT_TEST_E48_NM:
					//kjg_240213 val2 = (float)myData->test_val_l1[3]
					val2 = (float)myData->test_can_val_l[3]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_EXT_TEST_PC_NM:
					//kjg_240213 val2 = (float)myData->test_val_l1[4]
					val2 = (float)myData->test_can_val_l[4]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_EXT_TEST_RP_NM:
					//kjg_240213 val2 = (float)myData->test_val_l1[5]
					val2 = (float)myData->test_can_val_l[5]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_EXT_TEST_CHARGE_V:
					//kjg_240213 val2 = (float)myData->test_val_l1[6]
					val2 = (float)myData->test_can_val_l[6]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_EXT_TEST_DISCHARGE_V:
					//kjg_240213 val2 = (float)myData->test_val_l1[7]
					val2 = (float)myData->test_can_val_l[7]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_EXT_TEST_SET_MODE:
					//kjg_240213 val2 = (float)myData->test_val_l1[8]
					val2 = (float)myData->test_can_val_l[8]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_EXT_TEST_CMD_I_START:
					//kjg_240213 val2 = (float)myData->test_val_l1[9]
					val2 = (float)myData->test_can_val_l[9]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_EXT_TEST_CMD_P_START:
					//kjg_240213 val2 = (float)myData->test_val_l1[10]
					val2 = (float)myData->test_can_val_l[10]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_EXT_TEST_CMD_T:
					//kjg_240213 val2 = (float)myData->test_val_l1[11]
					val2 = (float)myData->test_can_val_l[11]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_EXT_TEST_SET_CONTROL:
					//kjg_240213 val2 = (float)myData->test_val_l1[12]
					val2 = (float)myData->test_can_val_l[12]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_EXT_TEST_RECEIVE_PARSING_PERIOD:
					//kjg_240213 val2 = (float)myData->test_val_l1[13]
					val2 = (float)myData->test_can_val_l[13]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				case CAN_TX_FUNC_DIV_EXT_TEST_CAN_TRANSMIT_PERIOD:
					//kjg_240213 val2 = (float)myData->test_val_l1[14]
					val2 = (float)myData->test_can_val_l[14]
						/ myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					break;
				default:
					break;
			}
			msg_count++;
		//kjg_140916_e
		} else if((function_div >= CAN_TX_FUNC_DIV_TEST_PHASE1)
			&& (function_div <= CAN_TX_FUNC_DIV_TEST_PHASE3)) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1]
				== function_phase) { //kjg_110831 1902, 1903, 1904
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else msg_skip = 1;
		} else if((function_div >= CAN_TX_FUNC_DIV_OP_TYPE1)
			&& (function_div <= CAN_TX_FUNC_DIV_OP_TYPE5)) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_OP_TYPE]
				== function_phase) { //kjg_111119 1906~1910
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
				if(myData->CAN.config.commType == 0) {
					if(myData->canTransmitSetData.normalData[module_ch][i].dlc > 8) {
						can_msg.length = 8;
					} else {
						can_msg.length = myData->canTransmitSetData
							.normalData[module_ch][i].dlc;
					}
				} else {
					can_msg.length = myData->canTransmitSetData
						.normalData[module_ch][i].dlc;
				}	//kjh_220519
			} else msg_skip = 1;
		} else if(((function_div >= CAN_TX_FUNC_DIV_WAKE_UP1)
			&& (function_div <= CAN_TX_FUNC_DIV_WAKE_UP2))
			|| ((function_div >= CAN_TX_FUNC_DIV_WAKE_UP3)
			&& (function_div <= CAN_TX_FUNC_DIV_WAKE_UP4))) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_WAKE_UP]
				== function_phase) {
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
				if(myData->CAN.config.commType == 0) {
					if(myData->canTransmitSetData.normalData[module_ch][i].dlc > 8) {
						can_msg.length = 8;
					} else {
						can_msg.length = myData->canTransmitSetData
							.normalData[module_ch][i].dlc;
					}
				} else {
					can_msg.length = myData->canTransmitSetData
						.normalData[module_ch][i].dlc;
				}	//kjh_220519
			} else msg_skip = 1;
		} else if((function_div >= CAN_TX_FUNC_DIV_TEST_PHASE4)
			&& (function_div <= CAN_TX_FUNC_DIV_TEST_PHASE8)) { //kjhw_141110
			if(myData->canTransmitSetData.commonData[module_ch][0].bms_type
				== 0) { //kjg_161207
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2]
					== function_phase) {
					val2 = myData->canTransmitSetData
						.normalData[module_ch][i].default_value
						* myData->canTransmitSetData.normalData[module_ch][i].factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					msg_count++;
				} else msg_skip = 1;
			} else if(myData->canTransmitSetData.commonData[module_ch][0]
				.bms_type == 50) { //kjg_161207 Porsche_PAG12V_BMS
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2]
					== function_phase) {
					val2 = myData->canTransmitSetData
						.normalData[module_ch][i].default_value
						* myData->canTransmitSetData.normalData[module_ch][i].factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					msg_count++;
				} else msg_skip = 1;
			}
		} else if((function_div >= CAN_TX_FUNC_DIV_TEST_PHASE9) //kjh_130628s
			&& (function_div <= CAN_TX_FUNC_DIV_TEST_PHASE18)) {
			if(myData->canTransmitSetData.commonData[module_ch][0]
				.bms_type == 0) { //kjg_161207
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST3]
					== function_phase) {
					val2 = myData->canTransmitSetData
						.normalData[module_ch][i].default_value
						* myData->canTransmitSetData.normalData[module_ch][i].factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					msg_count++;
				} else msg_skip = 1; //kjh_130628_e
			} else if(myData->canTransmitSetData.commonData[module_ch][0]
				.bms_type == 50) { //kjg_161207 Porsche_PAG12V_BMS
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2]
					== function_phase) {
					val2 = myData->canTransmitSetData
						.normalData[module_ch][i].default_value
						* myData->canTransmitSetData.normalData[module_ch][i].factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					msg_count++;
				} else msg_skip = 1;
			}
		} else if((function_div >= CAN_TX_FUNC_DIV_TEST3_PHASE1)
			&& (function_div <= CAN_TX_FUNC_DIV_TEST3_PHASE31)) { //kjh_210913
			//&& (function_div <= CAN_TX_FUNC_DIV_TEST3_PHASE16)) {
			if(myData->canTransmitSetData.commonData[module_ch][0].bms_type
				== 3) { //kjh_180821
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1]
					== function_phase) {
					val2 = myData->canTransmitSetData
						.normalData[module_ch][i].default_value
						* myData->canTransmitSetData.normalData[module_ch][i].factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					msg_count++;
				} else msg_skip = 1;
			}
		} else if((function_div >= CAN_TX_FUNC_DIV_TEST2_PHASE1) //kjh_140103s
			&& (function_div <= CAN_TX_FUNC_DIV_TEST2_PHASE31)) { //kjh_210913
			//&& (function_div <= CAN_TX_FUNC_DIV_TEST2_PHASE16)) {
			if(myData->canTransmitSetData.commonData[module_ch][0].bms_type
				== 0) { //kjg_161207
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST4]
					== function_phase) {
					val2 = myData->canTransmitSetData
						.normalData[module_ch][i].default_value
						* myData->canTransmitSetData.normalData[module_ch][i].factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					msg_count++;
				} else msg_skip = 1; //kjh_130628_e
			} else if(myData->canTransmitSetData.commonData[module_ch][0]
				.bms_type == 3) { //kjh_180821
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2]
					== function_phase) {
					val2 = myData->canTransmitSetData
						.normalData[module_ch][i].default_value
						* myData->canTransmitSetData.normalData[module_ch][i].factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					msg_count++;
				} else msg_skip = 1; //kjh_130628_e
			} else if(myData->canTransmitSetData.commonData[module_ch][0]
				.bms_type == 50) { //kjg_161207 Porsche_PAG12V_BMS
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2]
					== function_phase) {
					val2 = myData->canTransmitSetData
						.normalData[module_ch][i].default_value
						* myData->canTransmitSetData.normalData[module_ch][i].factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					msg_count++;
				} else msg_skip = 1;
			}
		} else if((function_div >= CAN_TX_FUNC_DIV_TEST5_PHASE1)
			&& (function_div <= CAN_TX_FUNC_DIV_TEST5_PHASE10)) {	//ktg_220614s
			if(myData->canTransmitSetData.commonData[module_ch][0]
				.bms_type == 3) {
				if(myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST5] != P0) {
					if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST5]
						== function_phase) {
						val2 = myData->canTransmitSetData
							.normalData[module_ch][i].default_value
							* myData->canTransmitSetData.normalData[module_ch][i].factor
 							+ myData->canTransmitSetData.normalData[module_ch][i]
							.offset;
						if(myData->CAN.config.commType == 0) {
							if(myData->canTransmitSetData.normalData[module_ch][i].dlc > 8) {
								can_msg.length = 8;
							} else {
								can_msg.length = myData->canTransmitSetData
									.normalData[module_ch][i].dlc;
							}
						} else {
							can_msg.length = myData->canTransmitSetData
								.normalData[module_ch][i].dlc;
						}
						msg_count++;
					} else msg_skip = 1; //ktg_220614e
				} else msg_skip = 1;
			}
		} else if((function_div >= 1401)
			&& (function_div <= 1450)) { //kjg_111120
			function_phase = (unsigned char)(function_div - 1400);

			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MAIN_ON]
				== function_phase) {
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MAIN_OFF]
				== function_phase) {
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_AUX_ON]
				== function_phase) {
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_AUX_OFF]
				== function_phase) {
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA1]
				== function_phase) {
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else msg_skip = 1;
		} else if((function_div >= 1451)
			&& (function_div <= 1500)) {
			function_phase = (unsigned char)(function_div - 1450);

			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA3]
				== function_phase) {
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					* myData->canTransmitSetData.normalData[module_ch][i]
					.factor
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
					if(function_phase == 9) {
					msg_count2++;
					if(msg_count2 == 4) {
						val2 = (float)myData->CAN.uds_tx[can_ch].uc_val[3];
					} else if(msg_count2 == 5) {
						val2 = (float)myData->CAN.uds_tx[can_ch].uc_val[4];
					} else if(msg_count2 == 6) {
						val2 = (float)myData->CAN.uds_tx[can_ch].uc_val[5];
					}
				} else if(function_phase == 13) {
					switch(myData->CAN.signal[can_ch]
						[CAN_SIG_TX_PHASE_UDS_DATA_INDEX]) {
						case 0:
							msg_count2++;
							if(msg_count2 == 6) {
								val2 = (float)myPs->bms_eol_data2
									.pack_id[0][0];
							} else if(msg_count2 == 7) {
								val2 = (float)myPs->bms_eol_data2
									.pack_id[0][1];
							} else if(msg_count2 == 8) {
								val2 = (float)myPs->bms_eol_data2
									.pack_id[0][2];
							}
							break;
						case 1:
							msg_count2++;
							if(msg_count2 == 1) {
								val2 = (float)(0x21);
							} else if(msg_count2 >= 2 && msg_count2 <= 8) {
								val2 = (float)myPs->bms_eol_data2
									.pack_id[0][msg_count2+1];
							}
							break;
						case 2:
							msg_count2++;
							if(msg_count2 == 1) {
								val2 = (float)(0x22);
							} else if(msg_count2 >= 2 && msg_count2 <= 8) {
								val2 = (float)myPs->bms_eol_data2
									.pack_id[0][msg_count2+8];
							}
							break;
						default:
							break;
					}
				}
			} else msg_skip = 1;

			if(myData->AppControl.config.systemModel
				== C_LGC_100V_200A_10A_20KW) {
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_EOL_DATA3]
					== function_phase) {
					val2 = myData->canTransmitSetData
						.normalData[module_ch][i].default_value
						* myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					msg_count++;
				} else msg_skip = 1;
			}
		} else if((function_div >= 1501)
			&& (function_div <= 1550)) {
			switch(myData->AppControl.config.systemModel) {
				case C_3PSYSTEM_500V_200A_10A_100KW: //kjg_120619
					if(function_div != CAN_TX_FUNC_DIV_BMS_EOL_INV_INPUT_V) {
						break;
					}
					default_value
						= FindExternalCanData(module_ch, function_div);
					if(myData->cData[0].op.state != C_RUN) {
						if(myData->CAN.percent_input_v[0] > 1) {
							default_value = default_value
								/ myData->CAN.percent_input_v[0] * 100.0;
						}
					}
					val2 = default_value / myData->canTransmitSetData
						.normalData[module_ch][i].factor
						- myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					msg_count++;
					break;
				default:
					function_phase = (unsigned char)(function_div - 1500);

					if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA2]
						== function_phase) {
						val2 = myData->canTransmitSetData
							.normalData[module_ch][i].default_value
							* myData->canTransmitSetData
							.normalData[module_ch][i].factor
							+ myData->canTransmitSetData
							.normalData[module_ch][i].offset;
						msg_count++;
					} else msg_skip = 1;
					break;
			}
		} else if((function_div >= 1551)
			&& (function_div <= 1600)) {
			if(function_div == CAN_TX_FUNC_DIV_EOL_PROCEDURE_ALL_CELL_V) {
				function_phase = (unsigned char)(function_div - 1550);
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_EOL_DATA2]
					== function_phase) { // 1563
					msg_count++;

					if(msg_count2 >= 0 && msg_count2 < 8) {
						val2 = (float)myData->CAN.uds_tx[can_ch]
							.uc_val[msg_count2];
					} else {
						val2 = 0.0;
					}
					msg_count2++;
				} else msg_skip = 1;
			} else if(function_div
				== CAN_TX_FUNC_DIV_EOL_PROCEDURE_ALL_MODULE_TEMP) {
				function_phase = (unsigned char)(function_div - 1550);
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_EOL_DATA2]
					== function_phase) { // 1564
					msg_count++;

					if(msg_count2 >= 0 && msg_count2 < 8) {
						val2 = (float)myData->CAN.uds_tx[can_ch]
							.uc_val[msg_count2];
					} else {
						val2 = 0.0;
					}
					msg_count2++;
				} else msg_skip = 1;
			} else {
				function_phase = (unsigned char)(function_div - 1550);
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_EOL_DATA2]
					== function_phase) {
					val2 = myData->canTransmitSetData
						.normalData[module_ch][i].default_value
						* myData->canTransmitSetData.normalData[module_ch][i]
						.factor
						+ myData->canTransmitSetData.normalData[module_ch][i]
						.offset;
					msg_count++;
				} else msg_skip = 1;
			}
		} else if(function_div == CAN_TX_FUNC_DIV_UDS_SF) { //kjh_120119_s
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_SF]
				== function_phase) { // 1911
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else msg_skip = 1;
		} else if(function_div == CAN_TX_FUNC_DIV_UDS_FF) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FF]
				== function_phase) { // 1912
				msg_count++;

				if(msg_count2 >= 0 && msg_count2 < 8) {
					val2 = (float)myData->CAN.uds_tx[can_ch].uc_val[msg_count2];
				} else {
					val2 = 0.0;
				}
				msg_count2++;
			} else msg_skip = 1;
		} else if(function_div == CAN_TX_FUNC_DIV_UDS_CF) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_CF]
				== function_phase) { // 1913
				msg_count++;

				if(msg_count2 >= 0 && msg_count2 < 8) {
					val2 = (float)myData->CAN.uds_tx[can_ch].uc_val[msg_count2];
				} else {
					val2 = 0.0;
				}
				msg_count2++;
			} else msg_skip = 1; //kjh_120119_e
		} else if(function_div == CAN_TX_FUNC_DIV_UDS_FC) {
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC]
				== function_phase) {
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;
			} else msg_skip = 1;
		} else if(function_div == CAN_TX_FUNC_DIV_UDS_KEY) { //kjh_120119
			if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_KEY]
				== function_phase) { //1915
				val2 = myData->canTransmitSetData
					.normalData[module_ch][i].default_value
					* myData->canTransmitSetData.normalData[module_ch][i].factor
					+ myData->canTransmitSetData.normalData[module_ch][i]
					.offset;
				msg_count++;

				msg_count2++;
				if(msg_count2 == 4) {
					val2 = (float)myData->CAN.uds_tx[can_ch].uc_val[0];
				} else if(msg_count2 == 5) {
					val2 = (float)myData->CAN.uds_tx[can_ch].uc_val[1];
				} else if(msg_count2 == 6) {
					val2 = (float)myData->CAN.uds_tx[can_ch].uc_val[2];
				}
			} else msg_skip = 1;
		} else if(function_div == CAN_TX_FUNC_DIV_INCRE_MIN) {
			val2 = myData->canTransmitSetData
				.normalData[module_ch][i].default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i].offset
				+ (float)myData->CAN.can_timer_1min[can_ch];
			if(val2 >= 16770000.0) val2 = 16770000.0;
			msg_count++;
		} else if((function_div >= CAN_TX_FUNC_DIV_VALUE_CHANGE)
			&& (function_div <= CAN_TX_FUNC_DIV_VALUE_CHANGE5)) {	//kjhw_170911s
			default_value = FindCanChangeData_2(module_ch, i, function_div);
			val2 = default_value
				* myData->canTransmitSetData.normalData[module_ch][i].factor
				+ myData->canTransmitSetData.normalData[module_ch][i]
				.offset;
			msg_count++;	//kjhw_170911e
		} else {
			msg_skip = 1;
		}

		if(msg_skip != 1) {
			switch(myData->canTransmitSetData
				.normalData[module_ch][i].data_type) {
				case 0: //unsigned
					val1 = (long long)val2;
					if(val1 < 0) val1 *= (-1);
					break;
				case 1: //signed
					val1 = (long long)val2;
					break;
				case 2: //float
					memcpy((char *)&val1, (char *)&val2, 8);
					break;
				default: //string
					memcpy((char *)&val1, (char *)&val2, 8);
					break;
			}

			start_point = myData->canTransmitSetData.normalData[module_ch][i]
				.startBit;
			bitCount = myData->canTransmitSetData.normalData[module_ch][i]
				.bitCount;
			end_point = start_point + bitCount;

#ifdef __CAN_FD__ //kjh_190706s
			if(myData->canTransmitSetData.normalData[module_ch][i]
				.byte_order == 0) { //little_endian(intel) kjg_190411
				memcpy((char *)&tmp1, (char *)&val1, sizeof(long long));
				tmp1 = tmp1 << (start_point % 64);

				tmp2 = 0x01;
				tmp2 = tmp2 << (start_point % 64);
				tmp3 = 0x01 << (start_point % 8);
				for(j=start_point; j < end_point; j++) {
					if((j % 8) == 0) tmp3 = 0x01;
					if((tmp1 & tmp2) != 0) {
						can_msg.data[j / 8] |= tmp3;
					}
					tmp2 = tmp2 << 1;
					tmp3 = tmp3 << 1;
				}

			} else { //big_endian(motorola) kjg_190411
				memcpy((char *)&tmp1, (char *)&val1, sizeof(long long));
				start_point2 = ((7 - (start_point % 64) / 8) * 8)
					+ (start_point % 8);
				end_point = start_point2 + bitCount;
				tmp1 = tmp1 << start_point2;

				memset((char *)&can_val, 0, sizeof(U_CAN_VAL));
				tmp2 = 0x01;
				tmp2 = tmp2 << start_point2;
				tmp3 = 0x01 << (start_point2 % 8);
				for(j=start_point2; j < end_point; j++) {
					if((j % 8) == 0) tmp3 = 0x01;
					if((tmp1 & tmp2) != 0) {
						can_val.uc_val[j / 8] |= tmp3;
					}
					tmp2 = tmp2 << 1;
					tmp3 = tmp3 << 1;
				}
				j = (start_point / 64) * 8;
				for(k=0; k < 8; k++) {
					can_msg.data[j + k] |= can_val.uc_val[7 - k];
				}
			}
#else //CAN_2p0b
			start_point2 = 0;
			if(myData->canTransmitSetData.normalData[module_ch][i].byte_order
				== 0) { //little_endian(intel)
				memcpy((char *)&tmp1, (char *)&val1, sizeof(long long));
				tmp1 = tmp1 << start_point;

				tmp2 = 0x01; tmp3 = 0;
				for(j=0; j < 64; j++) {
					if((j % 8) == 0) tmp3 = 0x01;
					if((j >= start_point) && (j < end_point)) {
						if((tmp1 & tmp2) != 0) {
							can_msg.data[j / 8] |= tmp3;
						}
					}
					tmp2 = tmp2 << 1;
					tmp3 = tmp3 << 1;
				}
			} else { //big_endian(motorola)
				memcpy((char *)&tmp1, (char *)&val1, sizeof(long long));
				start_point = ((7 - start_point / 8) * 8) + (start_point % 8);
				end_point = start_point + bitCount;
				tmp1 = tmp1 << start_point;

				memset((char *)&can_val, 0, sizeof(U_CAN_VAL));
				tmp2 = 0x01; tmp3 = 0;
				for(j=0; j < 64; j++) {
					if((j % 8) == 0) tmp3 = 0x01;
					if((j >= start_point) && (j < end_point)) {
						if((tmp1 & tmp2) != 0) {
							can_val.uc_val[j / 8] |= tmp3;
						}
					}
					tmp2 = tmp2 << 1;
					tmp3 = tmp3 << 1;
				}
				for(k=0; k < 8; k++) {
					//can_msg.data[k] = can_val.uc_val[7-k];
					can_msg.data[k] += can_val.uc_val[7-k]; //kjhw_141111
				}
			}
#endif //kjh_190706e
		}

		j = 0;
		if(myData->canTransmitSetData.normalData[module_ch][i+1].canID
			!= canID) {
			j = 1;
		}

		tmp3 = 0;
		if((i == (cnt_end - 1)) || (j >= 1)) {
			if(myData->mData.signal[M_SIG_EOL_PROCEDURE_VBF_FILE_DOWNLOAD] == P1
				&& canID == 0x7E4
				&& (myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] >= P22
				&& myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] <= P24)) {
				tmp3 = 1;				//kjg_121004
				myData->canTransmitTime[module_ch][i] = 0;
			} else {
				if(myData->canTransmitSetData.normalData[module_ch][i].send_period
					== 0) {
					tmp3 = 0;
					myData->canTransmitTime[module_ch][i] = 0;
				} else if(myData->canTransmitTime[module_ch][i] == 0) {
					tmp3 = 1;
					myData->canTransmitTime[module_ch][i] += 1;
				} else {
					//period = (myData->canTransmitTime[module_ch][i]
					//	% (myData->canTransmitSetData.normalData[module_ch][i]
					//	.send_period / 10));
					//ktg_211111s
					if(myData->canTransmitSetData.normalData[module_ch][i].send_period <
						myPs->config.scan_period) {
						myData->canTransmitSetData.normalData[module_ch][i].send_period
							= myPs->config.scan_period;
					}	//ktg_211111e
					period = (myData->canTransmitTime[module_ch][i]
						% (myData->canTransmitSetData.normalData[module_ch][i]
						.send_period / myPs->config.scan_period));
					//kjhw_210303
					if(period == 0) {
						tmp3 = 2;
						myData->canTransmitTime[module_ch][i] = 0;
					}
					myData->canTransmitTime[module_ch][i] += 1;
				}
			}
		}

		if((tmp3 != 0) && (msg_count != 0)) {
			msg_count = 0;
			rt_can_write(can_ch, (char *)&can_msg);

			if(function_div == CAN_TX_FUNC_DIV_DEBUG_ON) {
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_DEBUG_ON]
					== P1) {
					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_DEBUG_ON]++;
				}
			}
			if(function_div == CAN_TX_FUNC_DIV_CALIBRATION_CHARGE
				|| function_div == CAN_TX_FUNC_DIV_CALIBRATION_DISCHARGE) {
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_CALIBRATION]
					== P1) {
					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_CALIBRATION]++;
				}
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_CALIBRATION]
					== P3) {
					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_CALIBRATION]++;
				}
			}
			if(function_div == CAN_TX_FUNC_DIV_FLASH_REQUEST
				|| function_div == CAN_TX_FUNC_DIV_FLASH_RESET) {
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_FLASH_CMD]
					== P1) {
					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_FLASH_CMD]++;
				}
				if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_FLASH_CMD]
					== P3) {
					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_FLASH_CMD]++;
				}
			}
			if(myData->canTransmitSetData.commonData[module_ch][0].bms_type == 3) {
				if(function_div >= CAN_TX_FUNC_DIV_TEST3_PHASE1
					&& function_div <= CAN_TX_FUNC_DIV_TEST3_PHASE31) { //kjh_210913
					//&& function_div <= CAN_TX_FUNC_DIV_TEST3_PHASE16) {
					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1]++;
				}
				if(function_div >= CAN_TX_FUNC_DIV_TEST2_PHASE1
					&& function_div <= CAN_TX_FUNC_DIV_TEST2_PHASE31) { //kjh_210913
					//&& function_div <= CAN_TX_FUNC_DIV_TEST2_PHASE16) {
					myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2]++;
				}
				if(myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST5] != P0) {
					if(function_div >= CAN_TX_FUNC_DIV_TEST5_PHASE1
						&& function_div <= CAN_TX_FUNC_DIV_TEST5_PHASE10) {	//ktg_220614
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST5]++;
					}
					if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST5] >= P10) {
						myData->CAN.signal[can_ch][CAN_SIG_BMS_TEST5] = P0;
						myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST5] = P0;	//ktg_220724
					}
				}
			}
		}
	}

	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_AUX_ON] != P0) {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_AUX_ON] = P0;
	}
	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_AUX_OFF] != P0) {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_AUX_OFF] = P0;
	}
	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MAIN_ON] != P0) {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MAIN_ON] = P0;
	}
	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MAIN_OFF] != P0) {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_MAIN_OFF] = P0;
	}

	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA1] == 1) {
	} else {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA1] = P0;
	}

	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA2] != P0) {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA2] = P0;
	}
	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA3] != P0) {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA3] = P0;
	}
	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FF] != P0) {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FF] = P0;
	}
	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_CF] != P0) {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_CF] = P0;
	}
	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] != P0) {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_UDS_DATA_FC] = P0;
	}

	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_EOL_DATA2] != P0) {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_EOL_DATA2] = P0;
	}

	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_EOL_DATA3] != P0) {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_EOL_DATA3] = P0;
	}

	if(myData->canTransmitSetData.commonData[module_ch][0].bms_type == 3) {
	} else { //kjh_180821
		if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1] != P0) {
			myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST1] = P0;
		}
	}

	if(myData->canTransmitSetData.commonData[module_ch][0].bms_type == 3) {
	} else { //kjh_180821
		if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] != P0) {
			myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST2] = P0;
		}
	}

	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST3] != P0) {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST3] = P0;
	}	//kjhw_130628

	if(myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST4] != P0) {
		myData->CAN.signal[can_ch][CAN_SIG_TX_PHASE_TEST4] = P0;
	}	//kjhw_140103
}

float FindCanChangeData(int module_ch, int i)
{
	int j, k;
	float default_value;

	k = 0;
	for(j=0; j < MAX_CAN_TRANSMIT_CHANGE_DATA; j++) {
		k = 0;
		if(myData->canTransmitSetData.normalData[module_ch][i].canID == 0) {
			break;
		}
		if(myData->canTransmitSetData.normalData[module_ch][i].canID
			== myData->canTransmitChange.changeData[module_ch][j].canID) {
			k++; //1
		} else {
			continue;
		}
		if(myData->canTransmitSetData.normalData[module_ch][i].canType
			== myData->canTransmitChange.changeData[module_ch][j].canType) {
			k++; //2
		} else {
			continue;
		}
		if(myData->canTransmitSetData.normalData[module_ch][i].startBit
			== myData->canTransmitChange.changeData[module_ch][j]
			.startBit) {
			k++; //3
		} else {
			continue;
		}
		if(myData->canTransmitSetData.normalData[module_ch][i].function_div
			== myData->canTransmitChange.changeData[module_ch][j]
			.function_div) {
			k++; //4
		} else {
			continue;
		}
		if(myData->canTransmitSetData.normalData[module_ch][i].function_div2
			== myData->canTransmitChange.changeData[module_ch][j]
			.function_div2) {
			k++; //5
		} else {
			continue;
		}
		if(myData->canTransmitSetData.normalData[module_ch][i].function_div3
			== myData->canTransmitChange.changeData[module_ch][j]
			.function_div3) {
			k++; //6
		} else {
			continue;
		}
		if(k == 6) {
			break;
		}
	}

	if(k != 6) {
		default_value = myData->canTransmitSetData.normalData[module_ch][i]
			.default_value;
	} else {
		default_value = myData->canTransmitChange.changeData[module_ch][j]
			.default_value;
	}

	return default_value;
}

float FindExternalCanData(int module_ch, short int function_div)
{
	int j;
	float default_value;

	default_value = 0.0;

	for(j=0; j < myData->canReceiveDataCount[module_ch][0]; j++) {
		if(myData->canReceiveSetData.normalData[module_ch][j].function_div
			== function_div) {
			default_value = myData->CanData[module_ch][j].f_val[0];
			break;
		}
	}

	return default_value;
}

float FindCanChangeData_2(int module_ch, int i, int function_div)
{ //kjhw_170911
	int idx, j, k, ch, idxStepNo, func_div;
	float default_value;

	ch = module_ch;
	//idxStepNo = myData->cData[ch].op.idxStepNo;
	idxStepNo = (int)myData->cData[ch].op.idxStepNo; //kjhw_171108
	j = k = 0;

	default_value = myData->canTransmitSetData.normalData[module_ch][i]
		.default_value;
	if(myData->cData[ch].op.state == C_RUN) {
		for(j=0; j < MAX_CAN_FUNCTION; j++) {
			idx = IDX_LOC_OBJ_CAN_FUNC_DIV_1 + j;
			if(myData->testCond[ch].local_object[idxStepNo][idx]
				== function_div) {
				k++;
				break;
			}
		}
	} else {
	}

	if(k == 0) {
		for(j=0; j < MAX_CAN_TRANSMIT_CHANGE_DATA; j++) {
			func_div = (int)myData->canTransmitChange.changeData[module_ch][j]
				.function_div; //kjhw_171108
			if(func_div == function_div) {
				default_value = myData->canTransmitChange
					.changeData[module_ch][j].default_value;
				k++;
				break;
			}
		}
		if(k == 0) {
			default_value = myData->canTransmitSetData.normalData[module_ch][i]
				.default_value;
		}
	} else if(k == 1) {
		idx = IDX_LOC_OBJ_CAN_VALUE_1 + j;
		//default_value = myData->testCond[ch].local_object[idxStepNo][idx];
		default_value = (float)myData->testCond[ch].local_object[idxStepNo][idx]; //kjhw_170811
		default_value /= 1000.0;

		for(j=0; j < MAX_CAN_TRANSMIT_CHANGE_DATA; j++) {
			func_div = myData->canTransmitChange.changeData[module_ch][j]
				.function_div;
			if(func_div == 0) {
				myData->canTransmitChange.changeData[module_ch][j]
					.function_div = function_div;
				myData->canTransmitChange.changeData[module_ch][j]
					.default_value = default_value;
				break;
			} else if(func_div == function_div) {
				myData->canTransmitChange.changeData[module_ch][j]
					.default_value = default_value;
				break;
			}
		}
	} else {
		default_value = myData->canTransmitSetData.normalData[module_ch][i]
			.default_value;
	}

	return default_value;
}

float Convert_CAN_V_to_Temp(float Vcan, int type)
{	//ktg_220614s
	int table, i;
	float V_CANTemp_1, V_CANTemp_2, CANTemp1, CANTemp2, ret_CANTemp;

	table=0;
	for(table=0; table < MAX_CAN_TH_TABLE; table++) {
		if((int)myData->can_th_table[table].can_th_type == type) {
			break;
		}
	}
	ret_CANTemp = 999.999; //999.999%
	if(table == MAX_CAN_TH_TABLE) {
		return ret_CANTemp * 1000.0;
	}
						
	if(type == 0) {
		ret_CANTemp = 654.321; //654.321%
		return ret_CANTemp; // * 1000.0;
	}
	for(i=0; i < MAX_CAN_TH_DATA; i++) {
		if(myData->can_th_table[table].CT_V[i+1][0] >= 999000.0) {
			ret_CANTemp = 999000.0;
			break;
		}

		V_CANTemp_1 = myData->can_th_table[table].CT_V[i][1];
		V_CANTemp_2 = myData->can_th_table[table].CT_V[i+1][1];
		CANTemp1 = myData->can_th_table[table].CT_V[i][0];
		CANTemp2 = myData->can_th_table[table].CT_V[i+1][0];
		if(myData->can_th_table[table].type == 1) { //NTC //kjh_210317s
			if(Vcan > V_CANTemp_1) {
				ret_CANTemp = -999999;
				break;
			} else if(Vcan > V_CANTemp_2) {	// < : asceding order, > : deasceding order
				ret_CANTemp = CANTemp1 -
					((V_CANTemp_1 - Vcan) /(V_CANTemp_1 - V_CANTemp_2))
					* (CANTemp1 - CANTemp2);
 				break;
			}
		} else if (myData->can_th_table[table].type == 2) {	//PTC //kjh_210317e
			if(Vcan < V_CANTemp_1) { 	// < : asceding order, > : deasceding order
				ret_CANTemp = -999999;
				break;
			} else if(Vcan < V_CANTemp_2) {	// < : asceding order, > : deasceding order
				ret_CANTemp = CANTemp1 -
					((Vcan - V_CANTemp_1) / (V_CANTemp_2 - V_CANTemp_1))
					* (CANTemp1 - CANTemp2);
				break;
			}
		}else {
			break;	//None //kjh_210317e
		}
	}
	if(i == MAX_CAN_TH_DATA) ret_CANTemp = 998000.0;

	return ret_CANTemp / 1000;
	//ktg_220614e
}
