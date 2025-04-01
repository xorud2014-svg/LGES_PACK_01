#include <rtl_core.h>
#include <pthread.h>
//#include <inttypes.h>
#include "../rt_com/rt_com.h"
#include "../../INC/datastore.h"
#include "common_utils.h"
#include "local_message.h"
#include "COM.h"
#include "../../INC/IOComm_def.h"

extern S_SYSTEM_DATA	*myData;
extern S_MODULE_DATA	*myPs;
extern S_COM_PORT		*myCom;

void COM_Control_Receive(int slot)
{
	int rtn, comPort, start, end, count=0, comPort2;

	if(myData->AppControl.misc.Load_Process_COM[0] != P1) return;
//	if((slot + 1) > myPs->config.installedCOM) return;
	if(myData->COM.config.functionType[slot] == COM_FUNC_TYPE_NONE) {
		return;
	} else if(myData->COM.config.functionType[slot] == COM_FUNC_TYPE_DISPLAY1) {
		if(myData->AppControl.misc.Load_Process_COM[PROCESS_COM_CALI_METER]
			== P1) {
			return;
		}
	} else if(myData->COM.config.functionType[slot] == COM_FUNC_TYPE_METER1) {
		if(myData->AppControl.misc.Load_Process_COM[PROCESS_COM_CALI_METER]
			== P0) {
			return;
		}
	}

	start = slot;
	end = start + 1;

	for(comPort=start; comPort < end; comPort++) {
//		if(comPort >= myPs->config.installedCOM) return;

		if(myData->COM.com_port[comPort].misc.externPort_useFlag == P0) {
			comPort2 = comPort;
		} else {
			comPort2 = myData->COM.config.externPort[comPort] - 1;
		}

		myCom = &(myData->COM.com_port[comPort2]);

		for(count=0; count < 10; count++) {
			rtn = SerialPacket_Receive(comPort2);
			if(rtn >= 0) {
				rtn = Parsing_SerialEvent(comPort2);
				if(rtn < 0) {
					break;
				}
			} else {
				break;
			}
		}
	}
}

void COM_Control_Signal(int slot)
{
	int comPort, start, end, comPort2;

	if(myData->AppControl.misc.Load_Process_COM[0] != P1) return;
//	if((slot + 1) > myPs->config.installedCOM) return;
	if(myData->COM.config.functionType[slot] == COM_FUNC_TYPE_NONE) {
		return;
	} else if(myData->COM.config.functionType[slot] == COM_FUNC_TYPE_DISPLAY1) {
		if(myData->AppControl.misc.Load_Process_COM[PROCESS_COM_CALI_METER]
			== P1) {
			return;
		}
	} else if(myData->COM.config.functionType[slot] == COM_FUNC_TYPE_METER1) {
		if(myData->AppControl.misc.Load_Process_COM[PROCESS_COM_CALI_METER]
			== P0) {
			return;
		}
	}

	start = slot;
	end = start + 1;

	for(comPort=start; comPort < end; comPort++) {
//		if(comPort >= myPs->config.installedCOM) return;

		if(myData->COM.com_port[comPort].misc.externPort_useFlag == P0) {
			comPort2 = comPort;
		} else {
			comPort2 = myData->COM.config.externPort[comPort] - 1;
		}

		myCom = &(myData->COM.com_port[comPort2]);

		COM_Signal_Check(comPort2);
	}
}

void COM_Signal_Check(int comPort)
{
	int functionType, functionModel, i;

	functionType = (int)myData->COM.config.functionType[comPort];
	functionModel = (int)myData->COM.config.functionModel[comPort];

	switch(functionType) {
		case COM_FUNC_TYPE_METER1:
			//COM_Signal_Check_AGILENT_34401A(comPort);
			//break;
			//jhkw_131209s
			switch(functionModel) {
				case 0:	//AGILENT_34401A
					COM_Signal_Check_AGILENT_34401A(comPort);
					break;
				case 1: //VITREK_4700
					COM_Signal_Check_VITREK_4700(comPort);
					break;
				case 2: //Keysight_34461A   //csk_170912
					COM_Signal_Check_KEYSIGHT_34461A(comPort);
					break;
				default: break;
			}
			//jhkw_131209e
			break;
		case COM_FUNC_TYPE_DISPLAY1:
			//COM_Signal_Check_Display(comPort);	//kjhw_121015
			//kjhw_121015s
			switch(functionModel) {
				case 0:	//PNE_PannelMeter_A(232)
				case 1:	//B(485)
					COM_Signal_Check_Display(comPort);
					break;
				case 2:	//C(mcu2,ch4)
					COM_Signal_Check_Display1(comPort);
					break;
				default: break;
			}
			//kjhw_121015e
			break;
		case COM_FUNC_TYPE_METER2:
			switch(functionModel) {
				case 0: //CB-7018
					COM_Signal_Check_CB_7018(comPort);
					if(myData->AppControl.config.debugType == 110) {
						for(i=0; i < 32; i++) {
							if(myCom->value[i] <= 5500000) {
								myCom->value[i] += 100000;
							} else myCom->value[i] = 100000;
						}
					}
					break;
				case 1:
					break;
				case 2:
					COM_Signal_Check_COMM_A_XL_122(comPort);
					break;
				case 3: //I-7018Z
#if TEMP_CONVERTER
					break;
#else
					COM_Signal_Check_CB_7018(comPort);
					if(myData->AppControl.config.debugType == 110) {
						for(i=0; i < 32; i++) {
							if(myCom->value[i] <= 5500000) {
								myCom->value[i] += 100000;
							} else myCom->value[i] = 100000;
						}
					}
					break;
#endif
				case 4: //I-7033	//ktg_190516s
					COM_Signal_Check_CB_7018(comPort);
					if(myData->AppControl.config.debugType == 110) {
						for(i=0; i < 32; i++) {
							if(myCom->value[i] <= 5500000) {
								myCom->value[i] += 100000;
							} else myCom->value[i] = 100000;
						}
					}
					break;
					//ktg_190516e
				default: break;
			}
			break;
		case COM_FUNC_TYPE_BCR1:
			switch(functionModel) {
				case 0:
					COM_Signal_Check_MS_860_RS_485(comPort);
					break;
				case 1:
					COM_Signal_Check_MS_860_RS_232(comPort);
					break;
				case 2:
					COM_Signal_Check_COMM_B_MS_860(comPort);
					break;
				default: break;
			}
			break;
		case COM_FUNC_TYPE_IO_COMM1:
			switch(functionModel) {
				case 0: //EIOM_A
					break;
				case 1: //PLC_Q03UD
					COM_Signal_Check_PLC_Q03UD(comPort);
					break;
				default: break;
			}
			break;
		case COM_FUNC_TYPE_CHAMBER1:
			COM_Signal_Check_TEMP_880(comPort);
			break;
		default: break;
	}
}

int SerialPacket_Receive(int comPort)
{
	char packetBuf[MAX_COM_PACKET_LENGTH];
	int rcv_size, read_size, i, start, index, debug, hwComPort;

	memset(packetBuf, 0, MAX_COM_PACKET_LENGTH);

	hwComPort = myData->COM.config.comPortId[comPort] - 1;
	rcv_size = rt_com_read(hwComPort, packetBuf, MAX_COM_PACKET_LENGTH);

	if(rcv_size > MAX_COM_PACKET_LENGTH) {
		rtl_printf("com:%d max packet size over\n", comPort);
		return -1;
	} else if(rcv_size > (MAX_COM_PACKET_LENGTH
		- myCom->rcvPacket.usedBufSize)) {
		rtl_printf("com:%d packet buffer overflow\n", comPort);
		return -2;
	} else if(rcv_size < 0) {
		memset((char *)&myCom->rcvPacket, 0, sizeof(S_COM_RCV_PACKET));
		memset((char *)&myCom->rcvCmd, 0, sizeof(S_COM_RCV_COMMAND));
		rtl_printf("com:%d packet sock_rcv error1 %d\n", comPort, rcv_size);
		return -3;
	} else if(rcv_size == 0) {
		//rtl_printf("com:%d packet sock_rcv error2\n", comPort); //kjg_d
		//kjg_w return -4;
		return 0;
	}

	debug = 0;
	if(debug != 0 && comPort == 0) {
		rtl_printf("com:%d recvCmd_d", comPort);
		for(i=0; i < rcv_size; i++) {
	   		rtl_printf(" %02x", (unsigned char)packetBuf[i]);
		}
		rtl_printf(":end\n");
	}

	read_size = rcv_size;

	i = myCom->rcvPacket.rcvCount;
	myCom->rcvPacket.rcvCount++;
	if(myCom->rcvPacket.rcvCount > (MAX_COM_PACKET_COUNT-1))
		myCom->rcvPacket.rcvCount = 0;

	if(i == 0) index = MAX_COM_PACKET_COUNT - 1;
	else index = i - 1;
	start = myCom->rcvPacket.rcvStartPoint[index]
		+ myCom->rcvPacket.rcvSize[index];
	if(start >= MAX_COM_PACKET_LENGTH) {
		myCom->rcvPacket.rcvStartPoint[i] = abs(start - MAX_COM_PACKET_LENGTH);
	} else {
		myCom->rcvPacket.rcvStartPoint[i] = start;
	}

	myCom->rcvPacket.rcvSize[i] = read_size;
	myCom->rcvPacket.usedBufSize += read_size;

	start = myCom->rcvPacket.rcvStartPoint[i];
	if((start + read_size) > MAX_COM_PACKET_LENGTH) {
		index = MAX_COM_PACKET_LENGTH - start;
		memcpy((char *)&myCom->rcvPacket.rcvPacketBuf[start],
			(char *)&packetBuf, index);
		memcpy((char *)&myCom->rcvPacket.rcvPacketBuf,
			(char *)&packetBuf[index], read_size - index);
	} else {
		memcpy((char *)&myCom->rcvPacket.rcvPacketBuf[start],
			(char *)&packetBuf, read_size);
	}
	return 0;
}

void SerialPacket_Parsing(int comPort)
{
	int i, j, k, cmdBuf_index, start_point, debug;

	if(myCom->rcvPacket.rcvCount == myCom->rcvPacket.parseCount) return;

	i = myCom->rcvPacket.parseCount;
	myCom->rcvPacket.parseCount++;
	if(myCom->rcvPacket.parseCount > (MAX_COM_PACKET_COUNT-1))
		myCom->rcvPacket.parseCount = 0;

	cmdBuf_index = myCom->rcvCmd.cmdBufSize;
	myCom->rcvCmd.cmdBufSize += myCom->rcvPacket.rcvSize[i];

	debug = 0;
	if(debug != 0) {
		rtl_printf("com:%d, rcvCount:%d, parseCount:%d, cmdBufSize:%d, cmdBuf_index:%d, i:%d\n",
			comPort, myCom->rcvPacket.rcvCount, myCom->rcvPacket.parseCount,
			myCom->rcvCmd.cmdBufSize, cmdBuf_index, i);
	}

	if(myCom->rcvCmd.cmdBufSize >= (MAX_COM_PACKET_LENGTH - 32)) {
		rtl_printf("com:%d cmdBufSize error %d\n", comPort,
			myCom->rcvCmd.cmdBufSize);
		memset((char *)&myCom->rcvPacket, 0, sizeof(S_COM_RCV_PACKET));
		memset((char *)&myCom->rcvCmd, 0, sizeof(S_COM_RCV_COMMAND));
		return;
	}

	start_point = myCom->rcvPacket.parseStartPoint[i];

	j = start_point + myCom->rcvPacket.rcvSize[i];
	if(j <= MAX_COM_PACKET_LENGTH) {
		memcpy((char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myCom->rcvPacket.rcvPacketBuf[start_point],
			myCom->rcvPacket.rcvSize[i]);
	} else {
		k = MAX_COM_PACKET_LENGTH - start_point;
		memcpy((char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myCom->rcvPacket.rcvPacketBuf[start_point], k);
		cmdBuf_index += k;
		start_point = 0;
		k = j - MAX_COM_PACKET_LENGTH;
		memcpy((char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index],
			(char *)&myCom->rcvPacket.rcvPacketBuf[start_point], k);
	}

	start_point = myCom->rcvPacket.parseStartPoint[i]
		+ myCom->rcvPacket.rcvSize[i];
	if(start_point >= MAX_COM_PACKET_LENGTH) {
		j = i + 1;
		if(j >= MAX_COM_PACKET_COUNT) j = 0;
		myCom->rcvPacket.parseStartPoint[j]
			= abs(start_point - MAX_COM_PACKET_LENGTH);
	} else {
		j = i + 1;
		if(j >= MAX_COM_PACKET_COUNT) j = 0;
		myCom->rcvPacket.parseStartPoint[j] = start_point;
	}

	myCom->rcvPacket.usedBufSize -= myCom->rcvPacket.rcvSize[i];
}

int SerialCommand_Receive(int comPort)
{
	int cmd_size=0, cmdBuf_index, i, debug, functionType, functionModel;
	int packetType;

	if(myCom->rcvCmd.cmdBufSize < 1) return -1;

	debug = 0;
	if(debug != 0) {
		rtl_printf("com:%d cmdBuf", comPort);
		for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
	   		rtl_printf(" %02x", (unsigned char)myCom->rcvCmd.cmdBuf[i]);
		}
		rtl_printf(":end\n");
	}

	functionType = myData->COM.config.functionType[comPort];
	functionModel = myData->COM.config.functionModel[comPort];

	//packetType
	//01 : Agilent34401A
	//02 : PNE_PanelMeter_A, B, C(mcu2,ch4)
	//03 : CB-7018, EIOM_A, COMM_A(XL-122), Calibrator_B, C, D, COMM_B(MS-860)
	//	EIOM_B
	//04 : XL-122
	//05 : MS-860(RS-485)
	//06 : MS-860(RS-232)
	//07 : TEMP-880
	//08 : PLC_Q03UD
	//09 : EIOM_B(test)
	
	switch(functionType) {
		case COM_FUNC_TYPE_METER1:
			packetType = 1;
			break;
		case COM_FUNC_TYPE_DISPLAY1:
			packetType = 2;
			break;
		case COM_FUNC_TYPE_METER2:
			switch(functionModel) {
				case 0: //CB-7018
					packetType = 3;
					break;
				case 1: //XL-122
					packetType = 4;
					break;
				case 2: //COMM_A(XL-122)
					packetType = 3;
					break;
				case 3: //I-7018Z
#if TEMP_CONVERTER
					packetType = -1;	//KHK_190903
#else					
					packetType = 3;
#endif					
					break;
				case 4:	//I-7033 ktg_190516
					packetType = 3;
					break;
				default:
					packetType = -1;
					break;
			}
			break;
		case COM_FUNC_TYPE_CALIBRATOR1:
			packetType = 3;
			break;
		case COM_FUNC_TYPE_BCR1:
			switch(functionModel) {
				case 0: //MS-860(RS-485)
					packetType = 5;
					break;
				case 1: //MS-860(RS-232)
					packetType = 6;
					break;
				case 2: //COMM_B(MS-860)
					packetType = 3;
					break;
				default:
					packetType = -1;
					break;
			}
			break;
		case COM_FUNC_TYPE_IO_COMM1:
			switch(functionModel) {
				case 0: //EIOM_A
					packetType = 3;
					break;
				case 1: //PLC_Q03UD
					packetType = 8;
					break;
				default:
					packetType = -1;
					break;
			}
			break;
		case COM_FUNC_TYPE_CHAMBER1:
			packetType = 7;
			break;
		default:
			packetType = -1;
			break;
	}
	if(packetType < 0) return -11;

	if(packetType == 1) {
		//Agilent34401A
		if(myCom->rcvCmd.cmdBuf[0] == 0x2B //+
			|| myCom->rcvCmd.cmdBuf[0] == 0x2D) { //-
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == 0x0A) { //\n
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -21;
		} else {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == 0x0A) { //\n
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -22;

			cmdBuf_index = cmd_size;
			myCom->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myCom->rcvCmd.cmdBufSize;
			memset((char *)&myCom->rcvCmd.tmpBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.tmpBuf,
				(char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myCom->rcvCmd.cmdBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.cmdBuf,
				(char *)&myCom->rcvCmd.tmpBuf, cmd_size);

			return 1;
		}
	} else if(packetType == 2) {
		//PNE_PanelMeter_A, B, C(mcu2,ch4)
		if(myCom->rcvCmd.cmdBuf[0] == S_STX) {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == S_ETX) {
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -21;
			else if(cmd_size != 11) {
				cmdBuf_index = cmd_size;
				myCom->rcvCmd.cmdBufSize -= cmd_size;
				cmd_size = myCom->rcvCmd.cmdBufSize;
				memset((char *)&myCom->rcvCmd.tmpBuf, 0, MAX_COM_PACKET_LENGTH);
				memcpy((char *)&myCom->rcvCmd.tmpBuf,
					(char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
				memset((char *)&myCom->rcvCmd.cmdBuf, 0, MAX_COM_PACKET_LENGTH);
				memcpy((char *)&myCom->rcvCmd.cmdBuf,
					(char *)&myCom->rcvCmd.tmpBuf, cmd_size);

				return 2;
			}
		} else {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == S_ETX) {
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -22;

			cmdBuf_index = cmd_size;
			myCom->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myCom->rcvCmd.cmdBufSize;
			memset((char *)&myCom->rcvCmd.tmpBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.tmpBuf,
				(char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myCom->rcvCmd.cmdBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.cmdBuf,
				(char *)&myCom->rcvCmd.tmpBuf, cmd_size);

			return 1;
		}
	} else if(packetType == 3) {
		//CB-7018, EIOM_A, COMM_A(XL-122), Calibrator_B, C, D, COMM_B(MS-860),
		//EIOM_B, I-7018Z , I-7033
		if(myCom->rcvCmd.cmdBuf[0] == 0x00) {
			for(i=1; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] != 0x00) {
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -21;

			cmdBuf_index = cmd_size;
			myCom->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myCom->rcvCmd.cmdBufSize;
			memset((char *)&myCom->rcvCmd.tmpBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.tmpBuf,
				(char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myCom->rcvCmd.cmdBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.cmdBuf,
				(char *)&myCom->rcvCmd.tmpBuf, cmd_size);

			return 1;
		} else if(myCom->rcvCmd.cmdBuf[0] == 0x21 //!
			|| myCom->rcvCmd.cmdBuf[0] == 0x3F //?
			|| myCom->rcvCmd.cmdBuf[0] == 0x3E) { //>
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == 0x0D) { //\r
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -22;
		} else if(myCom->rcvCmd.cmdBuf[0] == 0x25 //%
			|| myCom->rcvCmd.cmdBuf[0] == 0x24 //$
			|| myCom->rcvCmd.cmdBuf[0] == 0x23) { //#
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == 0x0D) { //\r
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -23;

			cmdBuf_index = cmd_size;
			myCom->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myCom->rcvCmd.cmdBufSize;
			memset((char *)&myCom->rcvCmd.tmpBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.tmpBuf,
				(char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myCom->rcvCmd.cmdBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.cmdBuf,
				(char *)&myCom->rcvCmd.tmpBuf, cmd_size);

			return 1;
		} else {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == 0x0D) { //\r
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -24;

			cmdBuf_index = cmd_size;
			myCom->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myCom->rcvCmd.cmdBufSize;
			memset((char *)&myCom->rcvCmd.tmpBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.tmpBuf,
				(char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myCom->rcvCmd.cmdBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.cmdBuf,
				(char *)&myCom->rcvCmd.tmpBuf, cmd_size);
			return 2;
		}
	} else if(packetType == 4) {
		//XL-122
	} else if(packetType == 5) {
		//MS-860(RS-485)
		if(myCom->rcvCmd.cmdBuf[0] == 0x1C || myCom->rcvCmd.cmdBuf[0] == 0x1D
			|| myCom->rcvCmd.cmdBuf[0] == 0x1E
			|| myCom->rcvCmd.cmdBuf[0] == 0x1F) {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == S_ACK
					|| myCom->rcvCmd.cmdBuf[i] == S_ETX
					|| myCom->rcvCmd.cmdBuf[i] == S_RES
					|| myCom->rcvCmd.cmdBuf[i] == S_NULL) {
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -21;
		} else {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == S_ACK
					|| myCom->rcvCmd.cmdBuf[i] == S_ETX
					|| myCom->rcvCmd.cmdBuf[i] == S_RES
					|| myCom->rcvCmd.cmdBuf[i] == S_NULL
					|| myCom->rcvCmd.cmdBuf[i] == 0x14) {
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -22;

			cmdBuf_index = cmd_size;
			myCom->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myCom->rcvCmd.cmdBufSize;
			memset((char *)&myCom->rcvCmd.tmpBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.tmpBuf,
				(char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myCom->rcvCmd.cmdBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.cmdBuf,
				(char *)&myCom->rcvCmd.tmpBuf, cmd_size);

			return 1;
		}
	} else if(packetType == 6) {
		//MS-860(RS-232)
		if(myCom->rcvCmd.cmdBuf[0] == 0x00) {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == 0x0A) { //\n
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -21;

			cmdBuf_index = cmd_size;
			myCom->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myCom->rcvCmd.cmdBufSize;
			memset((char *)&myCom->rcvCmd.tmpBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.tmpBuf,
				(char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myCom->rcvCmd.cmdBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.cmdBuf,
				(char *)&myCom->rcvCmd.tmpBuf, cmd_size);

			return 1;
		} else {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == 0x0A) { //\n
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -22;
		}
	} else if(packetType == 7) {
		//TEMP-880
		if(myCom->rcvCmd.cmdBuf[0] == S_STX) {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == 0x0A) { //\n
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -21;
		} else {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == 0x0A) { //\n
					cmd_size = i+1; //one base
					break;
				}
			}
			if(cmd_size == 0) return -22;

			cmdBuf_index = cmd_size;
			myCom->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myCom->rcvCmd.cmdBufSize;
			memset((char *)&myCom->rcvCmd.tmpBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.tmpBuf,
				(char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myCom->rcvCmd.cmdBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.cmdBuf,
				(char *)&myCom->rcvCmd.tmpBuf, cmd_size);

			return 1;
		}
	} else if(packetType == 8) {
		//PLC_Q03UD
		if(myCom->rcvCmd.cmdBuf[0] == STX) {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == ETX) {
					cmd_size = i+1; // ETX
					break;
				}
			}
			if(cmd_size == 0) return -21;
		} else if(myCom->rcvCmd.cmdBuf[0] == PLC_ACK) {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(i < 3) continue;
				if(myCom->rcvCmd.cmdBuf[i-1] == 0x46
					&& myCom->rcvCmd.cmdBuf[i] == 0x46) {
					cmd_size = i+1; // ETX
					break;
				}
			}
			if(cmd_size < 5) return -22;
		} else if(myCom->rcvCmd.cmdBuf[0] == PLC_NACK) {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(i < 5) continue;
				if(myCom->rcvCmd.cmdBuf[i-3] == 0x46
					&& myCom->rcvCmd.cmdBuf[i-2] == 0x46
					&& (myCom->rcvCmd.cmdBuf[i-1] >= '0'
						&& myCom->rcvCmd.cmdBuf[i-1] <= '9')
					&& (myCom->rcvCmd.cmdBuf[i-1] >= 'A'
						&& myCom->rcvCmd.cmdBuf[i-1] <= 'F')
					&& (myCom->rcvCmd.cmdBuf[i] >= '0'
						&& myCom->rcvCmd.cmdBuf[i] <= '9')
					&& (myCom->rcvCmd.cmdBuf[i] >= 'A'
						&& myCom->rcvCmd.cmdBuf[i] <= 'F')) {
					cmd_size = i+1; // ETX
					break;
				}
			}
			if(cmd_size < 7) return -23;
		} else {
			for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
				if(myCom->rcvCmd.cmdBuf[i] == ETX) {
					cmd_size = i+1; // ETX 
					break;
				}
			}
			if(cmd_size == 0) return -24;

			cmdBuf_index = cmd_size;
			myCom->rcvCmd.cmdBufSize -= cmd_size;
			cmd_size = myCom->rcvCmd.cmdBufSize;
			memset((char *)&myCom->rcvCmd.tmpBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.tmpBuf,
				(char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
			memset((char *)&myCom->rcvCmd.cmdBuf, 0, MAX_COM_PACKET_LENGTH);
			memcpy((char *)&myCom->rcvCmd.cmdBuf,
				(char *)&myCom->rcvCmd.tmpBuf, cmd_size);

			return 1;
		}
	} else {
		for(i=0; i < myCom->rcvCmd.cmdBufSize; i++) {
			if(myCom->rcvCmd.cmdBuf[i] == 0x0D //\r
				|| myCom->rcvCmd.cmdBuf[i] == 0x0A) { //\n
				cmd_size = i+1; //one base
				break;
			}
		}
		if(cmd_size == 0) return -21;

		cmdBuf_index = cmd_size;
		myCom->rcvCmd.cmdBufSize -= cmd_size;
		cmd_size = myCom->rcvCmd.cmdBufSize;
		memset((char *)&myCom->rcvCmd.tmpBuf, 0, MAX_COM_PACKET_LENGTH);
		memcpy((char *)&myCom->rcvCmd.tmpBuf,
			(char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
		memset((char *)&myCom->rcvCmd.cmdBuf, 0, MAX_COM_PACKET_LENGTH);
		memcpy((char *)&myCom->rcvCmd.cmdBuf,
			(char *)&myCom->rcvCmd.tmpBuf, cmd_size);

		return 1;
	}

	memset((char *)&myCom->rcvCmd.cmd, 0, MAX_COM_PACKET_LENGTH);
	memcpy((char *)&myCom->rcvCmd.cmd, (char *)&myCom->rcvCmd.cmdBuf, cmd_size);
	myCom->rcvCmd.cmdSize = cmd_size;

	debug = 0;
	if(debug != 0) {
		rtl_printf("com:%d cmd", comPort);
		for(i=0; i < cmd_size; i++) {
	   		rtl_printf(" %02x", (unsigned char)myCom->rcvCmd.cmd[i]);
		}
		rtl_printf(":end\n");
	}

	cmdBuf_index = cmd_size;
	myCom->rcvCmd.cmdBufSize -= cmd_size;
	cmd_size = myCom->rcvCmd.cmdBufSize;
	memset((char *)&myCom->rcvCmd.tmpBuf, 0, MAX_COM_PACKET_LENGTH);
	memcpy((char *)&myCom->rcvCmd.tmpBuf,
		(char *)&myCom->rcvCmd.cmdBuf[cmdBuf_index], cmd_size);
	memset((char *)&myCom->rcvCmd.cmdBuf, 0, MAX_COM_PACKET_LENGTH);
	memcpy((char *)&myCom->rcvCmd.cmdBuf,
		(char *)&myCom->rcvCmd.tmpBuf, cmd_size);

	return 0;
}

int SerialCommand_Parsing(int comPort)
{
	unsigned char tmp, log_flag;
	int rtn=0, i, functionType, functionModel;

	if(myData->COM.config.CmdRcvLog[comPort] == P1) {
		log_flag = 0;
		if(myData->COM.config.CommCheckLog[comPort] == P1) {
			log_flag = 1;
		} else {
			log_flag = 1;
		}

		if(log_flag == 1) {
			rtl_printf("com:%d recvCmd %s:end\n", comPort, myCom->rcvCmd.cmd);
		}
	}

	if(myData->COM.config.CmdRcvLog_Hex[comPort] == P1) {
		log_flag = 0;
		if(myData->COM.config.CommCheckLog[comPort] == P1) {
			log_flag = 1;
		} else {
			log_flag = 1;
		}

		if(log_flag == 1) {
			rtl_printf("com:%d recvCmd", comPort);
			for(i=0; i < myCom->rcvCmd.cmdSize; i++) {
				tmp = myCom->rcvCmd.cmd[i];
		   		rtl_printf(" %02x", tmp);
			}
			rtl_printf(":end\n");
		}
	}

	functionType = myData->COM.config.functionType[comPort];
	functionModel = myData->COM.config.functionModel[comPort];

	switch(functionType) {
		case COM_FUNC_TYPE_METER1:
			//jhkw_131209s
			/*
			rtn = CmdHeader_Check_AGILENT_34401A(comPort);
			if(rtn < 0) return -11;

			rtn = rcv_cmd_answer_AGILENT_34401A(comPort);
			break;
			*/
			switch(functionModel) {
				case 0: //34401A
					rtn = CmdHeader_Check_AGILENT_34401A(comPort);
					if(rtn < 0) return -11;
	
					rtn = rcv_cmd_answer_AGILENT_34401A(comPort);
					break;
				case 1: //4700
					rtn = CmdHeader_Check_VITREK_4700(comPort);
					if(rtn < 0) return -11;
					
					rtn = rcv_cmd_answer_VITREK_4700(comPort);
					break;
				case 2: //34461A	//csk_170912
					rtn = CmdHeader_Check_KEYSIGHT_34461A(comPort);
					if(rtn < 0) return -11;

					rtn = rcv_cmd_answer_KEYSIGHT_34461A(comPort);
					break;
				default:
					break;
			}
			break;
			//jhkw_131209e
		case COM_FUNC_TYPE_DISPLAY1:
			rtn = CmdHeader_Check_Display(comPort);
			if(rtn < 0) return -11;

			rtn = rcv_cmd_answer_Display(comPort);
			break;
		case COM_FUNC_TYPE_METER2:
			switch(functionModel) {
				case 0: //CB-7018
					rtn = CmdHeader_Check_CB_7018(comPort);
					if(rtn < 0) return -11;

					rtn = rcv_cmd_answer_CB_7018(comPort);
					break;
				case 1: //XL-122
					break;
				case 2: //COMM_A(XL-122)
					rtn = CmdHeader_Check_COMM_A_XL_122(comPort);
					if(rtn < 0) return -11;

					rtn = CheckSum_Check_COMM_A_XL_122(comPort);
					if(rtn < 0) return -12;

					rtn = rcv_cmd_answer_COMM_A_XL_122(comPort);
					break;
				case 3: //I-7018Z
				case 4: //I-7033 ktg_190516
					rtn = CmdHeader_Check_CB_7018(comPort);
					if(rtn < 0) return -11;

					rtn = rcv_cmd_answer_CB_7018(comPort);
					break;
				default:
					break;
			}
			break;
		case COM_FUNC_TYPE_CALIBRATOR1:
			rtn = CmdHeader_Check_Calibrator(comPort);
			if(rtn < 0) return -11;

			rtn = CheckSum_Check_Calibrator(comPort);
			if(rtn < 0) return -12;

			rtn = rcv_cmd_answer_Calibrator(comPort);
			break;
		case COM_FUNC_TYPE_BCR1:
			switch(functionModel) {
				case 0: //MS-860(RS-485)
					rtn = rcv_cmd_answer_MS_860_RS_485(comPort);
					break;
				case 1: //MS-860(RS-232)
					rtn = rcv_cmd_answer_MS_860_RS_232(comPort);
					break;
				case 2: //COMM_B(MS-860)
					rtn = CmdHeader_Check_COMM_B_MS_860(comPort);
					if(rtn < 0) return -11;

					rtn = CheckSum_Check_COMM_B_MS_860(comPort);
					if(rtn < 0) return -12;

					rtn = rcv_cmd_answer_COMM_B_MS_860(comPort);
					break;
				default:
					break;
			}
			break;
		case COM_FUNC_TYPE_IO_COMM1:
			switch(functionModel) {
				case 0: //EIOM_A
					break;
				case 1: //PLC
					rtn = CmdHeader_Check_PLC_Q03UD(comPort);
					if(rtn < 0) return -11;

//					rtn = CheckSum_Check_PLC_Q03UD(comPort);
//					if(rtn < 0) return -12;

					rtn = rcv_cmd_answer_PLC_Q03UD(comPort);
					break;
				default:
					break;
			}
			break;
		case COM_FUNC_TYPE_CHAMBER1:
			rtn = CmdHeader_Check_TEMP_880(comPort);
			if(rtn < 0) return -11;

			rtn = CheckSum_Check_TEMP_880(comPort);
			if(rtn < 0) return -12;

			rtn = rcv_cmd_answer_TEMP_880(comPort);
			break;
		default:
			break;
	}

	return rtn;
}

int Parsing_SerialEvent(int comPort)
{
	int rtn, count=0;

	SerialPacket_Parsing(comPort);

	while(1) {
		count++;
		if(count > 10) break;

		rtn = SerialCommand_Receive(comPort);
		if(rtn < 0) break;
		else if(rtn > 0) continue;
		else {
			if(SerialCommand_Parsing(comPort) < 0) {
				myCom->rcvCmd.cmdFail++;
				if(myCom->rcvCmd.cmdFail >= 3) {
					myCom->rcvCmd.cmdFail = 0;
					myCom->rcvCmd.cmdBufSize = 0;
					memset((char *)&myCom->rcvCmd.cmdBuf, 0,
						MAX_COM_PACKET_LENGTH);
					return -1;
				}
			} else {
				myCom->rcvCmd.cmdFail = 0;
			}
		}
	}
	return 0;
}

void COM_Signal_Check_AGILENT_34401A(int comPort)
{
	S_MSG_VAL SendMsg;

	//jhkw_130121s
	switch(myCom->signal[COM_SIG_CALI_MODE_INITIALIZE]) {
		case P1:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;

				myData->AppControl.misc.Load_Process_COM[1] = 1;
				myData->AppControl.misc.Load_Process_COM[2] = 0;
				myData->COM.config.autoStart[0] = 0;
				myData->AppControl.misc
					.Load_Process_COM[PROCESS_COM_CALI_METER] = P1;
				//myData->COM.com_port[0].misc.externPort_useFlag = P1;
				rt_com_setup(0, 9600, RT_COM_PARITY_EVEN, 2, 7);

				//send_cmd_request_AGILENT_34401A(comPort);
				send_cmd_initialize_AGILENT_34401A(comPort, 1);

				myCom->signal[COM_SIG_CALI_MODE_INITIALIZE]++;
			}
			break;
		case P2:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_AGILENT_34401A(comPort, 2);
				myCom->signal[COM_SIG_CALI_MODE_INITIALIZE]++;
			}
			break;
		case P3:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_AGILENT_34401A(comPort, 3);
				myCom->signal[COM_SIG_CALI_MODE_INITIALIZE]++;
			}
			break;
		case P4:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_AGILENT_34401A(comPort, 4);
				myCom->signal[COM_SIG_CALI_MODE_INITIALIZE]++;
			}
			break;
		case P5:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_request_AGILENT_34401A(comPort);
				myCom->signal[COM_SIG_CALI_MODE_INITIALIZE] = P0;
			}
			break;
		default:
			break;
	}
	//jhkw_130121e
	switch(myCom->signal[COM_SIG_CALI_METER_INITIALIZE]) {
		case P1:
			send_cmd_initialize_AGILENT_34401A(comPort, 1);
			myCom->misc.delay_time = 0;
			myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			break;
		case P2:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_AGILENT_34401A(comPort, 2);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P3:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_AGILENT_34401A(comPort, 3);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P4:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_AGILENT_34401A(comPort, 4);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		case P11:
			send_cmd_initialize_AGILENT_34401A(comPort, 1);
			myCom->misc.delay_time = 0;
			myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			break;
		case P12:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_AGILENT_34401A(comPort, 2);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P13:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				if(myData->COM.config.readType[comPort] == READ_V_V) {
					send_cmd_initialize_AGILENT_34401A(comPort, 3);
				} else if(myData->COM.config.readType[comPort] == READ_V_I) {
					send_cmd_initialize_AGILENT_34401A(comPort, 5);
				}
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P14:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_AGILENT_34401A(comPort, 4);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		case P21:
			send_cmd_initialize_AGILENT_34401A(comPort, 1);
			myCom->misc.delay_time = 0;
			myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			break;
		case P22:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_AGILENT_34401A(comPort, 2);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P23:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_AGILENT_34401A(comPort, 3);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P24:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_AGILENT_34401A(comPort, 4);

				//kjg_100417_w memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				//SendMsg.msg = MSG_METER_COA_INITIALIZE_REPLY;
				//SendMsg.val[0] = 0;
				//send_msg(METER_TO_COA1, (char *)&SendMsg);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		case P31:
			send_cmd_initialize_AGILENT_34401A(comPort, 1);
			myCom->misc.delay_time = 0;
			myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			break;
		case P32:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_AGILENT_34401A(comPort, 2);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P33:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				if(myData->COM.config.readType[comPort] == READ_V_V) {
					send_cmd_initialize_AGILENT_34401A(comPort, 3);
				} else if(myData->COM.config.readType[comPort] == READ_V_I) {
					send_cmd_initialize_AGILENT_34401A(comPort, 5);
				}
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P34:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_AGILENT_34401A(comPort, 4);

				//kjg_100417_w memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				//SendMsg.msg = MSG_METER_COA_INITIALIZE_REPLY;
				//SendMsg.val[0] = 0;
				//send_msg(METER_TO_COA1, (char *)&SendMsg);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		default:	break;
	}

	switch(myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE]) {
		case P1:
			send_cmd_request_AGILENT_34401A(comPort);
			myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE]++;
			break;
		case P3:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_METER_MODULE_REQUEST_REPLY;
			SendMsg.val[0] = myPs->misc.receivedCh;
			send_msg(METER_TO_MODULE, (char *)&SendMsg);
			myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE] = P0;
			break;
		default:
			break;
	}
}

int CmdHeader_Check_AGILENT_34401A(int comPort)
{
	int length, i;
	S_COM_CALI_METER1_CMD_HEADER header;

	length = sizeof(S_COM_CALI_METER1_CMD_HEADER);
	memcpy((char *)&header, (char *)&myCom->rcvCmd.cmd, length);

	if(length != myCom->rcvCmd.cmdSize) {
		rtl_printf("com:%d RcvCmd size error length:%d rcv:%d\n",
			comPort, length, myCom->rcvCmd.cmdSize);
		return -1;
	}

	if(header.sign1 != '+' && header.sign1 != '-') {
		rtl_printf("com:%d RcvCmd sign1 error 0x%x\n", comPort, header.sign1);
		return -2;
	}

	if(header.digit1 < '0' || header.digit1 > '9') {
		rtl_printf("com:%d RcvCmd digit1 error 0x%x\n", comPort, header.digit1);
		return -3;
	}

	if(header.dot != '.') {
		rtl_printf("com:%d RcvCmd dot error 0x%x\n", comPort, header.dot);
		return -4;
	}

	for(i=0; i < 8; i++) {
		if(header.digit2[i] < '0' || header.digit2[i] > '9') {
			rtl_printf("com:%d RcvCmd digit2 error 0x%x\n", comPort,
				header.digit2[i]);
			return -5;
		}
	}

	if(header.exponent != 'E' && header.exponent != 'e') {
		rtl_printf("com:%d RcvCmd exponent error 0x%x\n", comPort,
			header.exponent);
		return -6;
	}

	if(header.sign2 != '+' && header.sign2 != '-') {
		rtl_printf("com:%d RcvCmd sign2 error 0x%x\n", comPort, header.sign2);
		return -7;
	}

	for(i=0; i < 2; i++) {
		if(header.digit3[i] < '0' || header.digit3[i] > '9') {
			rtl_printf("com:%d RcvCmd digit3 error 0x%x\n", comPort,
				header.digit3[i]);
			return -8;
		}
	}

	if(header.cr != 0x0D) {
		rtl_printf("com:%d RcvCmd cr error 0x%x\n", comPort, header.cr);
		return -9;
	}

	if(header.nl != '\n') {
		rtl_printf("com:%d RcvCmd nl error 0x%x\n", comPort, header.nl);
		return -10;
	}

	return 0;
}

int rcv_cmd_answer_AGILENT_34401A(int comPort)
{
	int i, tmp1, tmp2;
	double val;
	S_COM_CALI_METER1_CMD_HEADER header;

	memcpy((char *)&header, (char *)&myCom->rcvCmd.cmd,
		sizeof(S_COM_CALI_METER1_CMD_HEADER));
	
	tmp1 = (int)(header.digit1 - 0x30);

	for(i=0; i < 8; i++) {
		tmp1 = tmp1 * 10 + (int)(header.digit2[i] - 0x30);
	}

	tmp2 = (int)(header.digit3[0] - 0x30);
	tmp2 = tmp2 * 10 + (int)(header.digit3[1] - 0x30);

	val = (double)tmp1;
	if(header.sign2 == '-') {
		for(i=0; i < tmp2; i++)  val = val / 10.0;
	} else {
		for(i=0; i < tmp2; i++)  val = val * 10.0;
	}

	val /= 100.0;
	if(header.sign1 == '-') {
		val = val * (-1.0);
	}

//	rtl_printf("com:%d rcv value %d\n", comPort, (int)val); //kjg_d
	myPs->misc.meter_value = val;

	if(myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE] == P2) {
		myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE] = P3;
	}

	return 0;
}

void send_cmd_initialize_AGILENT_34401A(int comPort, int type)
{
	char cmd[32];
	int rtn, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	switch(type) {
		case 1:
			cmd_size = 5;
			memcpy((char *)&cmd, "*RST\n", cmd_size);
			break;
		case 2:
			cmd_size = 9;
			memcpy((char *)&cmd, "SYST:REM\n", cmd_size);
			break;
		case 3:
			cmd_size = 22;
			memcpy((char *)&cmd, "CONF:VOLT:DC DEF, DEF\n", cmd_size);
			break;
		case 4:
			cmd_size = 15;
			memcpy((char *)&cmd, "SAMPLE:COUNT 1\n", cmd_size);
			break;
		case 5:
			cmd_size = 22;
			memcpy((char *)&cmd, "CONF:CURR:DC DEF, DEF\n", cmd_size);
			break;
		default:
			rtl_printf("com:%d Unknown cmd\n", comPort);
			return;
	}

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void send_cmd_request_AGILENT_34401A(int comPort)
{
	char cmd[8];
	int rtn, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	cmd_size = 6;
	memcpy((char *)&cmd[0], "READ?\n", cmd_size);

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void COM_Signal_Check_VITREK_4700(int comPort)
{
	S_MSG_VAL SendMsg;

	switch(myCom->signal[COM_SIG_CALI_METER_INITIALIZE]) {
		case P1:
			send_cmd_initialize_VITREK_4700(comPort, 1);
			myCom->misc.delay_time = 0;
			myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			break;
		case P2:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_VITREK_4700(comPort, 2);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P3:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_VITREK_4700(comPort, 3);
				//myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		/*
		case P4:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_VITREK_4700(comPort, 4);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		*/
		case P11:
			send_cmd_initialize_VITREK_4700(comPort, 1);
			myCom->misc.delay_time = 0;
			myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			break;
		case P12:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_VITREK_4700(comPort, 2);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P13:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_VITREK_4700(comPort, 3);
				//myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		/*
		case P14:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_VITREK_4700(comPort, 4);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		*/
		case P21:
			send_cmd_initialize_VITREK_4700(comPort, 1);
			myCom->misc.delay_time = 0;
			myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			break;
		case P22:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_VITREK_4700(comPort, 2);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P23:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_VITREK_4700(comPort, 3);
				//myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		/*
		case P24:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_VITREK_4700(comPort, 4);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		*/
		case P31:
			send_cmd_initialize_VITREK_4700(comPort, 1);
			myCom->misc.delay_time = 0;
			myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			break;
		case P32:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_VITREK_4700(comPort, 2);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P33:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_VITREK_4700(comPort, 3);
				//myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		/*
		case P34:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_VITREK_4700(comPort, 4);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		*/
		default:	break;
	}

	switch(myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE]) {
		case P1:
			send_cmd_request_VITREK_4700(comPort);
			myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE]++;
			break;
		case P3:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_METER_MODULE_REQUEST_REPLY;
			SendMsg.val[0] = myPs->misc.receivedCh;
			send_msg(METER_TO_MODULE, (char *)&SendMsg);
			myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE] = P0;
			myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P21;
			break;
		default:
			break;
	}
}

int CmdHeader_Check_VITREK_4700(int comPort)
{
	int length, i;
	S_COM_CALI_METER1_4700_CMD_HEADER header;

	length = sizeof(S_COM_CALI_METER1_4700_CMD_HEADER);
	memcpy((char *)&header, (char *)&myCom->rcvCmd.cmd, length);

	if(length != myCom->rcvCmd.cmdSize) {
		rtl_printf("com:%d RcvCmd size error length:%d rcv:%d\n",
			comPort, length, myCom->rcvCmd.cmdSize);
		return -1;
	}

	if(header.sign1 != '+' && header.sign1 != '-') {
		rtl_printf("com:%d RcvCmd sign1 error 0x%x\n", comPort, header.sign1);
		return -2;
	}

	if(header.exponent != 'E' && header.exponent != 'e') {
		rtl_printf("com:%d RcvCmd exponent error 0x%x\n", comPort,
			header.exponent);
		return -6;
	}

	if(header.sign2 != '+' && header.sign2 != '-') {
		rtl_printf("com:%d RcvCmd sign2 error 0x%x\n", comPort, header.sign2);
		return -7;
	}

	for(i=0; i < 2; i++) {
		if(header.digit3[i] < '0' || header.digit3[i] > '9') {
			rtl_printf("com:%d RcvCmd digit3 error 0x%x\n", comPort,
				header.digit3[i]);
			return -8;
		}
	}

	if(header.cr != 0x0D) {
		rtl_printf("com:%d RcvCmd cr error 0x%x\n", comPort, header.cr);
		return -9;
	}

	if(header.nl != '\n') {
		rtl_printf("com:%d RcvCmd nl error 0x%x\n", comPort, header.nl);
		return -10;
	}

	return 0;
}

int rcv_cmd_answer_VITREK_4700(int comPort)
{
	int i, tmp1, tmp2;
	int dot = 0;
	int dotcnt = 0;
	double val;
	S_COM_CALI_METER1_4700_CMD_HEADER header;

	memcpy((char *)&header, (char *)&myCom->rcvCmd.cmd,
		sizeof(S_COM_CALI_METER1_4700_CMD_HEADER));
	tmp1 = 0;
	for(i=0; i < 7; i++) {
		if(header.digit1[i] == '.') {
			dot = 1;
			dotcnt = i - 1;
			rtl_printf("dot\n"); //kjg_d
		}
		if(dot != 1) {
			tmp1 = tmp1 * 10 + (int)(header.digit1[i] - 0x30);
//	rtl_printf("com:%d rcv value digit1 %d\n", comPort, (int)(header.digit1[i] - 0x30)); //kjg_d
//	rtl_printf("com:%d rcv value tmp1 %d\n", comPort, (int)tmp1); //kjg_d
		} else {
			dot = 0;
		}
	}
	for(i=0; i < dotcnt; i++)	tmp1 = tmp1 * 10;
	tmp2 = (int)(header.digit3[0] - 0x30);
	tmp2 = tmp2 * 10 + (int)(header.digit3[1] - 0x30);

	val = (double)tmp1;
	
//	rtl_printf("com:%d rcv value tmp2 %d\n", comPort, (int)tmp2); //kjg_d
	if(header.sign2 == '-') {
		for(i=0; i < tmp2; i++)  val = val / 10.0;
	} else {
		for(i=0; i < tmp2; i++)  val = val * 10.0;
	}
//	rtl_printf("com:%d rcv value sign2 %d\n", comPort, header.sign2); //kjg_d
	
	//val /= 100.0;
	if(header.sign1 == '-') {
		val = val * (-1.0);
	}
	val *= 10;
	rtl_printf("com:%d rcv value %d\n", comPort, (int)val); //kjg_d
//	rtl_printf("===============================================\n"); //kjg_d
	myPs->misc.meter_value = val;

	if(myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE] == P2) {
		myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE] = P3;
	}

	return 0;
}

void send_cmd_initialize_VITREK_4700(int comPort, int type)
{
	char cmd[32];
	int rtn, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	switch(type) {
		case 1:
			cmd_size = 5;
			memcpy((char *)&cmd, "*RST\n", cmd_size);
			break;
		case 2:
			cmd_size = 13;
			memcpy((char *)&cmd, "MODE,PRECISE\n", cmd_size);
			break;
		case 3:
			cmd_size = 9;
			memcpy((char *)&cmd, "DIGITS,6\n", cmd_size);
			break;
		/*
		case 4:
			cmd_size = 7;
			memcpy((char *)&cmd, "DCZERO\n", cmd_size);
			break;
		*/
		default:
			rtl_printf("com:%d Unknown cmd\n", comPort);
			return;
	}

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void send_cmd_request_VITREK_4700(int comPort)
{
	char cmd[8];
	int rtn, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	cmd_size = 6;
	memcpy((char *)&cmd[0], "DCV?\n", cmd_size);

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

//csk_170912s
void COM_Signal_Check_KEYSIGHT_34461A(int comPort)
{
	S_MSG_VAL SendMsg;

	//jhkw_130121s
	switch(myCom->signal[COM_SIG_CALI_MODE_INITIALIZE]) {
		case P1:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;

				myData->AppControl.misc.Load_Process_COM[1] = 1;
				myData->AppControl.misc.Load_Process_COM[2] = 0;
				myData->COM.config.autoStart[0] = 0;
				myData->AppControl.misc
					.Load_Process_COM[PROCESS_COM_CALI_METER] = P1;
				//myData->COM.com_port[0].misc.externPort_useFlag = P1;
				rt_com_setup(0, 9600, RT_COM_PARITY_EVEN, 2, 7);

				send_cmd_initialize_KEYSIGHT_34461A(comPort, 1);

				myCom->signal[COM_SIG_CALI_MODE_INITIALIZE]++;
			}
			break;
		case P2:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_KEYSIGHT_34461A(comPort, 2);
				myCom->signal[COM_SIG_CALI_MODE_INITIALIZE]++;
			}
			break;
		case P3:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_KEYSIGHT_34461A(comPort, 3);
				myCom->signal[COM_SIG_CALI_MODE_INITIALIZE]++;
			}
			break;
		case P4:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_KEYSIGHT_34461A(comPort, 4);
				myCom->signal[COM_SIG_CALI_MODE_INITIALIZE]++;
			}
			break;
		case P5:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_request_KEYSIGHT_34461A(comPort);
				myCom->signal[COM_SIG_CALI_MODE_INITIALIZE] = P0;
			}
			break;
		default:
			break;
	}	//jhkw_130121e
	switch(myCom->signal[COM_SIG_CALI_METER_INITIALIZE]) {
		case P1:
			send_cmd_initialize_KEYSIGHT_34461A(comPort, 1);
			myCom->misc.delay_time = 0;
			myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			break;
		case P2:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_KEYSIGHT_34461A(comPort, 2);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P3:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_KEYSIGHT_34461A(comPort, 3);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P4:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_KEYSIGHT_34461A(comPort, 4);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		case P11:
			send_cmd_initialize_KEYSIGHT_34461A(comPort, 1);
			myCom->misc.delay_time = 0;
			myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			break;
		case P12:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_KEYSIGHT_34461A(comPort, 2);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P13:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				if(myData->COM.config.readType[comPort] == READ_V_V) {
					send_cmd_initialize_KEYSIGHT_34461A(comPort, 3);
				} else if(myData->COM.config.readType[comPort] == READ_V_I) {
					send_cmd_initialize_KEYSIGHT_34461A(comPort, 5);
				}
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P14:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_KEYSIGHT_34461A(comPort, 4);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		case P21:
			send_cmd_initialize_KEYSIGHT_34461A(comPort, 1);
			myCom->misc.delay_time = 0;
			myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			break;
		case P22:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_KEYSIGHT_34461A(comPort, 2);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P23:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_KEYSIGHT_34461A(comPort, 3);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P24:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_KEYSIGHT_34461A(comPort, 4);

				//kjgw100417 memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				//SendMsg.msg = MSG_METER_COA_INITIALIZE_REPLY;
				//SendMsg.val[0] = 0;
				//send_msg(METER_TO_COA1, (char *)&SendMsg);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		case P31:
			send_cmd_initialize_KEYSIGHT_34461A(comPort, 1);
			myCom->misc.delay_time = 0;
			myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			break;
		case P32:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_KEYSIGHT_34461A(comPort, 2);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P33:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				if(myData->COM.config.readType[comPort] == READ_V_V) {
					send_cmd_initialize_KEYSIGHT_34461A(comPort, 3);
				} else if(myData->COM.config.readType[comPort] == READ_V_I) {
					send_cmd_initialize_KEYSIGHT_34461A(comPort, 5);
				}
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE]++;
			}
			break;
		case P34:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 500) { //500ms
				myCom->misc.delay_time = 0;
				send_cmd_initialize_KEYSIGHT_34461A(comPort, 4);

				//kjgw100417 memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				//SendMsg.msg = MSG_METER_COA_INITIALIZE_REPLY;
				//SendMsg.val[0] = 0;
				//send_msg(METER_TO_COA1, (char *)&SendMsg);
				myCom->signal[COM_SIG_CALI_METER_INITIALIZE] = P0;
			}
			break;
		default:	break;
	}

	switch(myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE]) {
		case P1:
			send_cmd_request_KEYSIGHT_34461A(comPort);
			myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE]++;
			break;
		case P3:
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_METER_MODULE_REQUEST_REPLY;
			SendMsg.val[0] = myPs->misc.receivedCh;
			send_msg(METER_TO_MODULE, (char *)&SendMsg);
			myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE] = P0;
			break;
		default:
			break;
	}
}

int CmdHeader_Check_KEYSIGHT_34461A(int comPort)
{
	int length, i;
	S_COM_CALI_METER1_34461A_CMD_HEADER header;

	length = sizeof(S_COM_CALI_METER1_34461A_CMD_HEADER);
	memcpy((char *)&header, (char *)&myCom->rcvCmd.cmd, length);

	if(length != myCom->rcvCmd.cmdSize) {
		rtl_printf("com:%d RcvCmd size error length:%d rcv:%d\n",
			comPort, length, myCom->rcvCmd.cmdSize);
		return -1;
	}

	if(header.sign1 != '+' && header.sign1 != '-') {
		rtl_printf("com:%d RcvCmd sign1 error 0x%x\n", comPort, header.sign1);
		return -2;
	}

	if(header.digit1 < '0' || header.digit1 > '9') {
		rtl_printf("com:%d RcvCmd digit1 error 0x%x\n", comPort, header.digit1);
		return -3;
	}

	if(header.dot != '.') {
		rtl_printf("com:%d RcvCmd dot error 0x%x\n", comPort, header.dot);
		return -4;
	}

	for(i=0; i < 8; i++) {
		if(header.digit2[i] < '0' || header.digit2[i] > '9') {
			rtl_printf("com:%d RcvCmd digit2 error 0x%x\n", comPort,
				header.digit2[i]);
			return -5;
		}
	}

	if(header.exponent != 'E' && header.exponent != 'e') {
		rtl_printf("com:%d RcvCmd exponent error 0x%x\n", comPort,
			header.exponent);
		return -6;
	}

	if(header.sign2 != '+' && header.sign2 != '-') {
		rtl_printf("com:%d RcvCmd sign2 error 0x%x\n", comPort, header.sign2);
		return -7;
	}

	for(i=0; i < 2; i++) {
		if(header.digit3[i] < '0' || header.digit3[i] > '9') {
			rtl_printf("com:%d RcvCmd digit3 error 0x%x\n", comPort,
				header.digit3[i]);
			return -8;
		}
	}

	if(header.nl != 0x0A) {		//\n
		rtl_printf("com:%d RcvCmd nl error 0x%x\n", comPort, header.nl);
		return -11;
	}

	return 0;
}

int rcv_cmd_answer_KEYSIGHT_34461A(int comPort)
{
	int i, tmp1, tmp2;
	double val;
	S_COM_CALI_METER1_34461A_CMD_HEADER header;

	memcpy((char *)&header, (char *)&myCom->rcvCmd.cmd,
		sizeof(S_COM_CALI_METER1_34461A_CMD_HEADER));
	
	tmp1 = (int)(header.digit1 - 0x30);

	for(i=0; i < 8; i++) {
		tmp1 = tmp1 * 10 + (int)(header.digit2[i] - 0x30);
	}

	tmp2 = (int)(header.digit3[0] - 0x30);
	tmp2 = tmp2 * 10 + (int)(header.digit3[1] - 0x30);

	val = (double)tmp1;
	if(header.sign2 == '-') {
		for(i=0; i < tmp2; i++)  val = val / 10.0;
	} else {
		for(i=0; i < tmp2; i++)  val = val * 10.0;
	}

	val /= 100.0;
	if(header.sign1 == '-') {
		val = val * (-1.0);
	}

	rtl_printf("com:%d rcv value %d\n", comPort, (int)val); //kjgd
	myPs->misc.meter_value = val;

	if(myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE] == P2) {
		myCom->signal[COM_SIG_CALI_METER_REQUEST_PHASE] = P3;
	}

	return 0;
}

void send_cmd_initialize_KEYSIGHT_34461A(int comPort, int type)
{
	char cmd[32];
	int rtn, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	switch(type) {
		case 1:
			cmd_size = 5;
			memcpy((char *)&cmd, "*RST\n", cmd_size);
			break;
		case 2:
			cmd_size = 10;									//for_34461A
			memcpy((char *)&cmd, "SYST:PRES\n", cmd_size);	//for_34461A
			break;
		case 3:
			cmd_size = 22;
			memcpy((char *)&cmd, "CONF:VOLT:DC DEF, DEF\n", cmd_size);
			break;
		case 4:
			cmd_size = 15;
			memcpy((char *)&cmd, "SAMPLE:COUNT 1\n", cmd_size);
			break;
		case 5:
			cmd_size = 22;
			memcpy((char *)&cmd, "CONF:CURR:DC DEF, DEF\n", cmd_size);
			break;
		default:
			rtl_printf("com:%d Unknown cmd\n", comPort);
			return;
	}

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void send_cmd_request_KEYSIGHT_34461A(int comPort)
{
	char cmd[8];
	int rtn, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	cmd_size = 6;
	memcpy((char *)&cmd[0], "READ?\n", cmd_size);

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}
//csk_170912e

void COM_Signal_Check_Display(int comPort)
{
	int i, test=0;

	switch(myCom->signal[COM_SIG_DISPLAY]) {
		case P1: //ch1
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(0, 1, test, comPort);
			break;
		case P2:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(0, 2, test, comPort);
			break;
		case P3:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(0, 0, test, comPort);
			break;
		case P4:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(0, 3, test, comPort);
			break;
		case P5: //ch2
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			if(myData->COM.config.functionModel[comPort] == 0) {
				send_cmd_value_Display(0, 11, test, comPort);
			} else {
				send_cmd_value_Display(1, 1, test, comPort);
			}
			break;
		case P6:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			if(myData->COM.config.functionModel[comPort] == 0) {
				send_cmd_value_Display(0, 12, test, comPort);
			} else {
				send_cmd_value_Display(1, 2, test, comPort);
			}
			break;
		case P7:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			if(myData->COM.config.functionModel[comPort] == 0) {
				send_cmd_value_Display(0, 10, test, comPort);
			} else {
				send_cmd_value_Display(1, 0, test, comPort);
			}
			break;
		case P8:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			if(myData->COM.config.functionModel[comPort] == 0) {
				send_cmd_value_Display(0, 13, test, comPort);
			} else {
				send_cmd_value_Display(1, 3, test, comPort);
			}
			break;
		case P9: //ch3
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(2, 1, test, comPort);
			break;
		case P10:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(2, 2, test, comPort);
			break;
		case P11:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(2, 0, test, comPort);
			break;
		case P12:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(2, 3, test, comPort);
			break;
		case P13: //ch4
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(3, 1, test, comPort);
			break;
		case P14:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(3, 2, test, comPort);
			break;
		case P15:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(3, 0, test, comPort);
			break;
		case P16:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(3, 3, test, comPort);
			break;
		case P21:
		case P22:
		case P23:
		case P24:
		case P25:
		case P26:
		case P27:
		case P28:
		case P29:
		case P30:
		case P31:
		case P32:
		case P33:
		case P34:
		case P35:
		case P36:
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time
				* (int)myPs->config.scan_period;
			if(i < 30) break;
			myCom->misc.retry_time = 0;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				if(myData->COM.config.countMeter[comPort]
					//>= (myCom->signal[COM_SIG_DISPLAY] - P20)
					> (myCom->signal[COM_SIG_DISPLAY] - P20)	//kjhw_130118
					/ 4) {
					myCom->signal[COM_SIG_DISPLAY] -= P19;
					//kjg_d myCom->signal[COM_SIG_DISPLAY] = P1;
				} else {
					myCom->signal[COM_SIG_DISPLAY] = P1;
				}
			}
			break;
		default: break;
	}
}

void COM_Signal_Check_Display1(int comPort)
{ //kjhw_121016
	int i, test=0;

	switch(myCom->signal[COM_SIG_DISPLAY]) {
		case P1: //ch1
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(0, 1, test, comPort);
			break;
		case P2:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(0, 2, test, comPort);
			break;
		case P3:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(0, 0, test, comPort);
			break;
		case P4:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			send_cmd_value_Display(0, 3, test, comPort);
			break;
		case P5: //ch2
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			//if(myData->COM.config.functionModel[comPort] == 0) {
				send_cmd_value_Display(0, 11, test, comPort);
			//} else {
			//	send_cmd_value_Display(1, 1, test, comPort);
			//}
			break;
		case P6:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			//if(myData->COM.config.functionModel[comPort] == 0) {
				send_cmd_value_Display(0, 12, test, comPort);
			//} else {
			//	send_cmd_value_Display(1, 2, test, comPort);
			//}
			break;
		case P7:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			//if(myData->COM.config.functionModel[comPort] == 0) {
				send_cmd_value_Display(0, 10, test, comPort);
			//} else {
			//	send_cmd_value_Display(1, 0, test, comPort);
			//}
			break;
		case P8:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			//if(myData->COM.config.functionModel[comPort] == 0) {
				send_cmd_value_Display(0, 13, test, comPort);
			//} else {
			//	send_cmd_value_Display(1, 3, test, comPort);
			//}
			break;
		case P9: //ch3
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			//send_cmd_value_Display(2, 1, test, comPort);
			send_cmd_value_Display(1, 1, test, comPort);
			break;
		case P10:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			//send_cmd_value_Display(2, 2, test, comPort);
			send_cmd_value_Display(1, 2, test, comPort);
			break;
		case P11:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			//send_cmd_value_Display(2, 0, test, comPort);
			send_cmd_value_Display(1, 0, test, comPort);
			break;
		case P12:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			//send_cmd_value_Display(2, 3, test, comPort);
			send_cmd_value_Display(1, 3, test, comPort);
			break;
		case P13: //ch4
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			//send_cmd_value_Display(3, 1, test, comPort);
			send_cmd_value_Display(1, 11, test, comPort);
			break;
		case P14:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			//send_cmd_value_Display(3, 2, test, comPort);
			send_cmd_value_Display(1, 12, test, comPort);
			break;
		case P15:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			//send_cmd_value_Display(3, 0, test, comPort);
			send_cmd_value_Display(1, 10, test, comPort);
			break;
		case P16:
			myCom->signal[COM_SIG_DISPLAY] += P20;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			//send_cmd_value_Display(3, 3, test, comPort);
			send_cmd_value_Display(1, 13, test, comPort);
			break;
		case P21:
		case P22:
		case P23:
		case P24:
		case P25:
		case P26:
		case P27:
		case P28:
		case P29:
		case P30:
		case P31:
		case P32:
		case P33:
		case P34:
		case P35:
		case P36:
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time
				* (int)myPs->config.scan_period;
			if(i < 30) break;
			myCom->misc.retry_time = 0;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				if(myData->COM.config.countMeter[comPort]
					> (myCom->signal[COM_SIG_DISPLAY] - P20)
					/ 8) {
					// >= (myCom->signal[COM_SIG_DISPLAY] - P20)
					// / 4) {
					myCom->signal[COM_SIG_DISPLAY] -= P19;
					//kjg_d myCom->signal[COM_SIG_DISPLAY] = P1;
				} else {
					myCom->signal[COM_SIG_DISPLAY] = P1;
				}
			}
			break;
		default: break;
	}
}

int CmdHeader_Check_Display(int comPort)
{
	int length;
	S_COM_DISPLAY_CMD_HEADER header;

	length = 11;
	memcpy((char *)&header, (char *)&myCom->rcvCmd.cmd,
		sizeof(S_COM_DISPLAY_CMD_HEADER));

	if(length != myCom->rcvCmd.cmdSize) {
		//rtl_printf("com:%d RcvCmd size error length:%d rcv:%d\n",
		//	comPort, length, myCom->rcvCmd.cmdSize);
		return -1;
	}

	if((header.addr[0] >= 0x30 && header.addr[0] <= 0x39)
		|| (header.addr[0] >= 0x41 && header.addr[0] <= 0x46)) {
	} else {
		rtl_printf("com:%d RcvCmd addr[0] error 0x%x\n", comPort,
			header.addr[0]);
		return -2;
	}

	if((header.addr[1] >= 0x30 && header.addr[1] <= 0x39)
		|| (header.addr[1] >= 0x41 && header.addr[1] <= 0x46)) {
	} else {
		rtl_printf("com:%d RcvCmd addr[1] error 0x%x\n", comPort,
			header.addr[1]);
		return -3;
	}

	if((header.cmd[0] >= 0x30 && header.cmd[0] <= 0x39)
		|| (header.cmd[0] >= 0x41 && header.cmd[0] <= 0x46)) {
	} else {
		rtl_printf("com:%d RcvCmd cmd[0] error 0x%x\n", comPort,
			header.cmd[0]);
		return -4;
	}

	if((header.cmd[1] >= 0x30 && header.cmd[1] <= 0x39)
		|| (header.cmd[1] >= 0x41 && header.cmd[1] <= 0x46)) {
	} else {
		rtl_printf("com:%d RcvCmd cmd[1] error 0x%x\n", comPort,
			header.cmd[1]);
		return -5;
	}

	return 0;
}

int rcv_cmd_answer_Display(int comPort)
{
	int i, checksum, cmp_checksum, checksum_type, tmp;
	S_COM_RCV_CMD_ANSWER_DISPLAY recvCmd;
	
	memset((char *)&recvCmd, 0, sizeof(S_COM_RCV_CMD_ANSWER_DISPLAY));
	memcpy((char *)&recvCmd, (char *)&myCom->rcvCmd.cmd, myCom->rcvCmd.cmdSize);

	checksum = 0;
	checksum_type = 1; //0:dec, 1:hex
	for(i=1; i < 6; i++) {
		checksum += (int)recvCmd.data[i];
	}

	if(checksum_type == 0) { //dec
		cmp_checksum = ((int)recvCmd.data[6] << 12);
		cmp_checksum |= ((int)recvCmd.data[7] << 8);
		cmp_checksum |= ((int)recvCmd.data[8] << 4);
		cmp_checksum |= (int)recvCmd.data[9];
	} else { //hex
		tmp = (int)recvCmd.data[6];
		if(tmp <= 0x39) tmp -= 0x30;
		else tmp -= 0x37;
		cmp_checksum = (tmp << 12);
		tmp = (int)recvCmd.data[7];
		if(tmp <= 0x39) tmp -= 0x30;
		else tmp -= 0x37;
		cmp_checksum |= (tmp << 8);
		tmp = (int)recvCmd.data[8];
		if(tmp <= 0x39) tmp -= 0x30;
		else tmp -= 0x37;
		cmp_checksum |= (tmp << 4);
		tmp = (int)recvCmd.data[9];
		if(tmp <= 0x39) tmp -= 0x30;
		else tmp -= 0x37;
		cmp_checksum |= tmp;
	}

	if(checksum != cmp_checksum) {
		rtl_printf(
			"com:%d RcvCmd checksum error : %02x %02x %02x %02x : %x : %x\n",
			comPort, recvCmd.data[6], recvCmd.data[7], recvCmd.data[8],
			recvCmd.data[9], checksum, cmp_checksum);
	}
	
	if(recvCmd.data[5] != 0x06) {
		rtl_printf( "com:%d RcvCmd nack %02x\n", comPort, recvCmd.data[5]);
	} else {
		switch((int)myCom->signal[COM_SIG_DISPLAY]) {
			case P21: //ch1
				if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x31) {
					myCom->signal[COM_SIG_DISPLAY] -= P19;
					//kjg_d myCom->signal[COM_SIG_DISPLAY] = P1;
				}
				break;
			case P22:
				if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x32) {
					myCom->signal[COM_SIG_DISPLAY] -= P19;
				}
				break;
			case P23:
				if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x30) {
					myCom->signal[COM_SIG_DISPLAY] -= P19;
				}
				break;
			case P24:
				if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x33) {
					myCom->signal[COM_SIG_DISPLAY] -= P19;
				}
				break;
			case P25: //ch2
				/* //kjhw_121016
				if(myData->COM.config.functionModel[comPort] == 0) {
					if(recvCmd.data[3] == 0x31 && recvCmd.data[4] == 0x31) {
						myCom->signal[COM_SIG_DISPLAY] -= P19;
					}
				} else {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x31) {
						myCom->signal[COM_SIG_DISPLAY] -= P19;
					}
				}*/
				if((myData->COM.config.functionModel[comPort] == 0)
					|| (myData->COM.config.functionModel[comPort] == 2)) {
					if(recvCmd.data[3] == 0x31 && recvCmd.data[4] == 0x31) {
						myCom->signal[COM_SIG_DISPLAY] -= P19;
					}
				} else {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x31) {
						myCom->signal[COM_SIG_DISPLAY] -= P19;
					}
				}
				break;
			case P26:
				if((myData->COM.config.functionModel[comPort] == 0)
					|| (myData->COM.config.functionModel[comPort] == 2)) {
					if(recvCmd.data[3] == 0x31 && recvCmd.data[4] == 0x32) {
						myCom->signal[COM_SIG_DISPLAY] -= P19;
					}
				} else {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x32) {
						myCom->signal[COM_SIG_DISPLAY] -= P19;
					}
				}
				break;
			case P27:
				if((myData->COM.config.functionModel[comPort] == 0)
					|| (myData->COM.config.functionModel[comPort] == 2)) {
					if(recvCmd.data[3] == 0x31 && recvCmd.data[4] == 0x30) {
						myCom->signal[COM_SIG_DISPLAY] -= P19;
					}
				} else {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x30) {
						myCom->signal[COM_SIG_DISPLAY] -= P19;
					}
				}
				break;
			case P28:
				if((myData->COM.config.functionModel[comPort] == 0)
					|| (myData->COM.config.functionModel[comPort] == 2)) {
					if(recvCmd.data[3] == 0x31 && recvCmd.data[4] == 0x33) {
						myCom->signal[COM_SIG_DISPLAY] = P1;
					}
				} else {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x33) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 4) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				}
				break;
			case P29: //ch3
				if(myData->COM.config.functionModel[comPort] == 2) {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x31) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 8) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				} else {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x31) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 4) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				}
				break;
			case P30:
				if(myData->COM.config.functionModel[comPort] == 2) {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x32) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 8) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				} else {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x32) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 4) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				}
				break;
			case P31:
				if(myData->COM.config.functionModel[comPort] == 2) {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x30) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 8) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				} else {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x30) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 4) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				}
				break;
			case P32:
				if(myData->COM.config.functionModel[comPort] == 2) {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x33) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 8) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				} else {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x33) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 4) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				}
				break;
			case P33: //ch4
				if(myData->COM.config.functionModel[comPort] == 2) {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x31) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 8) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				} else {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x31) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 4) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				}
				break;
			case P34:
				if(myData->COM.config.functionModel[comPort] == 2) {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x32) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 8) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				} else {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x32) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 4) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				}
				break;
			case P35:
				if(myData->COM.config.functionModel[comPort] == 2) {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x30) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 8) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				} else {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x30) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 4) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				}
				break;
			case P36:
				if(myData->COM.config.functionModel[comPort] == 2) {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x33) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 8) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				} else {
					if(recvCmd.data[3] == 0x30 && recvCmd.data[4] == 0x33) {
						if(myData->COM.config.countMeter[comPort]
							> (myCom->signal[COM_SIG_DISPLAY] - P20) / 4) {
							myCom->signal[COM_SIG_DISPLAY] -= P19;
						} else {
							myCom->signal[COM_SIG_DISPLAY] = P1;
						}
					}
				}
				break;
			default: break;
		}
	}

	return 0;
}

void send_cmd_value_Display(int addr, int type, int test, int comPort)
{
	unsigned char flag[12], val3;
	int rtn, cmd_size, i, checksum, tmp, checksum_type, group=0;
	int j; //kjhw_130917
	long val, val2;

	S_COM_SEND_CMD_DISPLAY_VALUE cmd;

	memset((char *)&cmd, 0, sizeof(S_COM_SEND_CMD_DISPLAY_VALUE));

	if((type % 10) == 0) {
		cmd_size = 12; //LED
	} else {
		cmd_size = 16; //V, I, P
	}

	j = 0; //kjhw_130917
	cmd.data[0] = 0x02; //STX
	cmd.data[1] = '0'; //addr1
	cmd.data[2] = (char)(0x30 + addr);
	switch(type % 10) {
		case 0: //LED
			if(type < 10) {
				cmd.data[3] = '0';
				cmd.data[4] = '0';
			} else {
				cmd.data[3] = '1';
				cmd.data[4] = '0';
			}

			val2 = 0;
			if(test == 1 || test == 2) {
				val = 0xFF;
				break;
			}
			if(myData->COM.config.functionModel[comPort] == 0 && type < 10) {
				//A rs232 ch1 LED
				val = 0;
				flag[0] = Read_InPoint(group, 1, I_FUSE_FAIL);
				flag[1] = Read_InPoint(group, 1, I_INVERTER_HV_FAULT);
				flag[2] = Read_InPoint(group, 1, I_INVERTER_MODULE_R_FAIL);
				flag[3] = Read_InPoint(group, 1, I_INVERTER_MODULE_S_FAIL);
				flag[4] = Read_InPoint(group, 1, I_INVERTER_MODULE_T_FAIL);
				flag[5] = Read_InPoint(group, 1, I_INVERTER_OT);
				flag[6] = Read_InPoint(group, 1, I_POWER_TRANS_OT);
				flag[7] = Read_InPoint(group, 1, I_POWER_REACTOR_OT);
				flag[8] = Read_InPoint(group, 1, I_MCCB_FAIL); //lki_111010
				flag[9] = Read_InPoint(group, 1, I_INVERTER_OC); //jhkw_131011
				flag[10] = Read_InPoint(group, 1, I_INVERTER_ETC); //jhkw_131011
				flag[11] = Read_InPoint(group, 1, I_DC_LINK_FUSE_FAIL); //phb_230710
				val3 = 0;
				for(i=0; i < 12; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x02;
				}

				if(myData->dio.signal[DIO_SIG_REMOTE_PS] == P100) {
					val |= 0x01;
				}

				flag[0] = Read_InPoint(group, 1, I_CONVERTER_OV);
				flag[1] = Read_InPoint(group, 1, I_CONVERTER_OC);
				flag[2] = Read_InPoint(group, 1, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(group, 1, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(group, 1, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				switch(myData->AppControl.config.systemModel) {
					case C_ADD_800V_350A_200A_50A_380KW:	//shh_200904
						break;
					default:
						if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
							val |= 0x40;
						}
						break;
				}

				if(myData->cData[0].op.state == C_RUN) {
					val |= 0x08;
					if(myData->cData[0].op.stepType == STEP_CHARGE) {
						val |= 0x10;
					} else if(myData->cData[0].op.stepType == STEP_DISCHARGE
						|| myData->cData[0].op.stepType == STEP_Z) {
						val |= 0x20;
					} else if(myData->cData[0].op.stepType == STEP_PATTERN
						|| myData->cData[0].op.stepType == STEP_EXTERNAL_CAN
						|| myData->cData[0].op.stepType == STEP_EXTERNAL_CAN_2
						|| myData->cData[0].op.stepType == STEP_USERMAP) { //kjhw_140828
						if(myData->cData[0].op.Isens >= 0) {
							val |= 0x10;
						} else {
							val |= 0x20;
						}
					}
				}

				val |= 0x04; //power led
			} else if(myData->COM.config.functionModel[comPort] == 0
				&& type >= 10) {
				//A rs232 ch2 LED
				val = 0;
				flag[0] = Read_InPoint(group, 2, I_CONVERTER_OV);
				flag[1] = Read_InPoint(group, 2, I_CONVERTER_OC);
				flag[2] = Read_InPoint(group, 2, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(group, 2, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(group, 2, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				//kjhw_130917s
				//if(myData->ChAttribute[1].opType != OP_INDEPENDENT) {
				if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
					val |= 0x40;
				}

				if(myData->ChAttribute[1].chNo_master == 0) {
				//ch1,ch2 parallel if ch2 zero
				} else {
					if(myData->cData[1].op.state == C_RUN) {
						val |= 0x08;
						if(myData->cData[1].op.stepType == STEP_CHARGE) {
							val |= 0x10;
						} else if(myData->cData[1].op.stepType == STEP_DISCHARGE
							|| myData->cData[1].op.stepType == STEP_Z) {
							val |= 0x20;
						} else if(myData->cData[1].op.stepType == STEP_PATTERN
							|| myData->cData[1].op.stepType == STEP_EXTERNAL_CAN
							|| myData->cData[1].op.stepType == STEP_EXTERNAL_CAN_2
							|| myData->cData[1].op.stepType == STEP_USERMAP) { //kjhw_140828
							if(myData->cData[1].op.Isens >= 0) {
								val |= 0x10;
							} else {
								val |= 0x20;
							}
						}
					}
				}
				//kjhw_130917e
				val |= 0x04; //power led
			} else if(myData->COM.config.functionModel[comPort] == 1
				&& addr == 0) {
				//B rs485 ch1 LED
				val = 0;
				flag[0] = Read_InPoint(group, 1, I_FUSE_FAIL);
				flag[1] = Read_InPoint(group, 1, I_INVERTER_HV_FAULT);
				flag[2] = Read_InPoint(group, 1, I_INVERTER_MODULE_R_FAIL);
				flag[3] = Read_InPoint(group, 1, I_INVERTER_MODULE_S_FAIL);
				flag[4] = Read_InPoint(group, 1, I_INVERTER_MODULE_T_FAIL);
				flag[5] = Read_InPoint(group, 1, I_INVERTER_OT);
				flag[6] = Read_InPoint(group, 1, I_POWER_TRANS_OT);
				flag[7] = Read_InPoint(group, 1, I_POWER_REACTOR_OT);
				flag[8] = Read_InPoint(group, 1, I_INVERTER_OC); //jhkw_131011
				flag[9] = Read_InPoint(group, 1, I_INVERTER_ETC); //jhkw_131011
				flag[10] = Read_InPoint(group, 1, I_DC_LINK_FUSE_FAIL); //phb_230710
				val3 = 0;
				for(i=0; i < 11; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x02;
				}

				if(myData->dio.signal[DIO_SIG_REMOTE_PS] == P100) {
					val |= 0x01;
				}

				flag[0] = Read_InPoint(group, 1, I_CONVERTER_OV);
				flag[1] = Read_InPoint(group, 1, I_CONVERTER_OC);
				flag[2] = Read_InPoint(group, 1, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(group, 1, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(group, 1, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
					val |= 0x40;
				}

				if(myData->cData[0].op.state == C_RUN) {
					val |= 0x08;
					if(myData->cData[0].op.stepType == STEP_CHARGE) {
						val |= 0x10;
					} else if(myData->cData[0].op.stepType == STEP_DISCHARGE
						|| myData->cData[0].op.stepType == STEP_Z) {
						val |= 0x20;
					} else if(myData->cData[0].op.stepType == STEP_PATTERN
						|| myData->cData[0].op.stepType == STEP_EXTERNAL_CAN
						|| myData->cData[0].op.stepType == STEP_EXTERNAL_CAN_2
						|| myData->cData[0].op.stepType == STEP_USERMAP) { //kjhw_140828
						if(myData->cData[0].op.Isens >= 0) {
							val |= 0x10;
						} else {
							val |= 0x20;
						}
					}
				}

				val |= 0x04; //power led
			} else if(myData->COM.config.functionModel[comPort] == 1
				&& addr == 1) {
				//B rs485 ch2 LED
				val = 0;
				flag[0] = Read_InPoint(group, 2, I_FUSE_FAIL);
				flag[1] = Read_InPoint(group, 2, I_INVERTER_HV_FAULT);
				flag[2] = Read_InPoint(group, 2, I_INVERTER_MODULE_R_FAIL);
				flag[3] = Read_InPoint(group, 2, I_INVERTER_MODULE_S_FAIL);
				flag[4] = Read_InPoint(group, 2, I_INVERTER_MODULE_T_FAIL);
				flag[5] = Read_InPoint(group, 2, I_INVERTER_OT);
				flag[6] = Read_InPoint(group, 2, I_POWER_TRANS_OT);
				flag[7] = Read_InPoint(group, 2, I_POWER_REACTOR_OT);
				flag[8] = Read_InPoint(group, 2, I_INVERTER_OC); //jhkw_131011
				flag[9] = Read_InPoint(group, 2, I_INVERTER_ETC); //jhkw_131011
				flag[10] = Read_InPoint(group, 2, I_DC_LINK_FUSE_FAIL); //phb_230710
				val3 = 0;
				for(i=0; i < 11; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x02;
				}

				if(myData->dio.signal[DIO_SIG_REMOTE_PS] == P100) {
					val |= 0x01;
				}

				flag[0] = Read_InPoint(group, 2, I_CONVERTER_OV);
				flag[1] = Read_InPoint(group, 2, I_CONVERTER_OC);
				flag[2] = Read_InPoint(group, 2, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(group, 2, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(group, 2, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
					val |= 0x40;
				}

				if(myData->cData[1].op.state == C_RUN) {
					val |= 0x08;
					if(myData->cData[1].op.stepType == STEP_CHARGE) {
						val |= 0x10;
					} else if(myData->cData[1].op.stepType == STEP_DISCHARGE
						|| myData->cData[1].op.stepType == STEP_Z) {
						val |= 0x20;
					} else if(myData->cData[1].op.stepType == STEP_PATTERN
						|| myData->cData[1].op.stepType == STEP_EXTERNAL_CAN
						|| myData->cData[1].op.stepType == STEP_EXTERNAL_CAN_2
						|| myData->cData[1].op.stepType == STEP_USERMAP) { //kjhw_140828
						if(myData->cData[1].op.Isens >= 0) {
							val |= 0x10;
						} else {
							val |= 0x20;
						}
					}
				}

				val |= 0x04; //power led
			} else if(myData->COM.config.functionModel[comPort] == 1
				&& addr == 2) {
				//B rs485 ch3 LED
				val = 0;
				flag[0] = Read_InPoint(group, 3, I_FUSE_FAIL);
				flag[1] = Read_InPoint(group, 3, I_INVERTER_HV_FAULT);
				flag[2] = Read_InPoint(group, 3, I_INVERTER_MODULE_R_FAIL);
				flag[3] = Read_InPoint(group, 3, I_INVERTER_MODULE_S_FAIL);
				flag[4] = Read_InPoint(group, 3, I_INVERTER_MODULE_T_FAIL);
				flag[5] = Read_InPoint(group, 3, I_INVERTER_OT);
				flag[6] = Read_InPoint(group, 3, I_POWER_TRANS_OT);
				flag[7] = Read_InPoint(group, 3, I_POWER_REACTOR_OT);
				flag[8] = Read_InPoint(group, 3, I_INVERTER_OC); //jhkw_131011
				flag[9] = Read_InPoint(group, 3, I_INVERTER_ETC); //jhkw_131011
				flag[10] = Read_InPoint(group, 3, I_DC_LINK_FUSE_FAIL); //phb_230710
				val3 = 0;
				for(i=0; i < 11; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x02;
				}

				if(myData->dio.signal[DIO_SIG_REMOTE_PS] == P100) {
					val |= 0x01;
				}

				flag[0] = Read_InPoint(group, 3, I_CONVERTER_OV);
				flag[1] = Read_InPoint(group, 3, I_CONVERTER_OC);
				flag[2] = Read_InPoint(group, 3, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(group, 3, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(group, 3, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				if(myData->ChAttribute[2].opType != OP_INDEPENDENT
					|| myData->ChAttribute[2].chNo_master == 0) {
					val |= 0x40;
				}

				if(myData->cData[2].op.state == C_RUN) {
					val |= 0x08;
					if(myData->cData[2].op.stepType == STEP_CHARGE) {
						val |= 0x10;
					} else if(myData->cData[2].op.stepType == STEP_DISCHARGE
						|| myData->cData[2].op.stepType == STEP_Z) {
						val |= 0x20;
					} else if(myData->cData[2].op.stepType == STEP_PATTERN
						|| myData->cData[2].op.stepType == STEP_EXTERNAL_CAN
						|| myData->cData[2].op.stepType == STEP_EXTERNAL_CAN_2
						|| myData->cData[2].op.stepType == STEP_USERMAP) { //kjhw_140828
						if(myData->cData[2].op.Isens >= 0) {
							val |= 0x10;
						} else {
							val |= 0x20;
						}
					}
				}

				val |= 0x04; //power led
			} else if(myData->COM.config.functionModel[comPort] == 1
				&& addr == 3) {
				//B rs485 ch4 LED
				val = 0;
				flag[0] = Read_InPoint(group, 4, I_FUSE_FAIL);
				flag[1] = Read_InPoint(group, 4, I_INVERTER_HV_FAULT);
				flag[2] = Read_InPoint(group, 4, I_INVERTER_MODULE_R_FAIL);
				flag[3] = Read_InPoint(group, 4, I_INVERTER_MODULE_S_FAIL);
				flag[4] = Read_InPoint(group, 4, I_INVERTER_MODULE_T_FAIL);
				flag[5] = Read_InPoint(group, 4, I_INVERTER_OT);
				flag[6] = Read_InPoint(group, 4, I_POWER_TRANS_OT);
				flag[7] = Read_InPoint(group, 4, I_POWER_REACTOR_OT);
				flag[8] = Read_InPoint(group, 4, I_INVERTER_OC); //jhkw_131011
				flag[9] = Read_InPoint(group, 4, I_INVERTER_ETC); //jhkw_131011
				flag[10] = Read_InPoint(group, 4, I_DC_LINK_FUSE_FAIL); //phb_230710
				val3 = 0;
				for(i=0; i < 11; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x02;
				}

				if(myData->dio.signal[DIO_SIG_REMOTE_PS] == P100) {
					val |= 0x01;
				}

				flag[0] = Read_InPoint(group, 4, I_CONVERTER_OV);
				flag[1] = Read_InPoint(group, 4, I_CONVERTER_OC);
				flag[2] = Read_InPoint(group, 4, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(group, 4, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(group, 4, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				if(myData->ChAttribute[2].opType != OP_INDEPENDENT
					|| myData->ChAttribute[3].chNo_master == 0) {
					val |= 0x40;
				}

				if(myData->cData[3].op.state == C_RUN) {
					val |= 0x08;
					if(myData->cData[3].op.stepType == STEP_CHARGE) {
						val |= 0x10;
					} else if(myData->cData[3].op.stepType == STEP_DISCHARGE
						|| myData->cData[3].op.stepType == STEP_Z) {
						val |= 0x20;
					} else if(myData->cData[3].op.stepType == STEP_PATTERN
						|| myData->cData[3].op.stepType == STEP_EXTERNAL_CAN
						|| myData->cData[3].op.stepType == STEP_EXTERNAL_CAN_2
						|| myData->cData[3].op.stepType == STEP_USERMAP) { //kjhw_140828
						if(myData->cData[3].op.Isens >= 0) {
							val |= 0x10;
						} else {
							val |= 0x20;
						}
					}
				}

				val |= 0x04; //power led
			//kjhw_121016s
			} else if(myData->COM.config.functionModel[comPort] == 2
				&& addr == 0 && type < 10) {
				//C(mcu2_ch4) 1bd ch1 rs485
				val = 0;
				flag[0] = Read_InPoint(group, 1, I_FUSE_FAIL);
				flag[1] = Read_InPoint(group, 1, I_INVERTER_HV_FAULT);
				flag[2] = Read_InPoint(group, 1, I_INVERTER_MODULE_R_FAIL);
				flag[3] = Read_InPoint(group, 1, I_INVERTER_MODULE_S_FAIL);
				flag[4] = Read_InPoint(group, 1, I_INVERTER_MODULE_T_FAIL);
				flag[5] = Read_InPoint(group, 1, I_INVERTER_OT);
				flag[6] = Read_InPoint(group, 1, I_POWER_TRANS_OT);
				flag[7] = Read_InPoint(group, 1, I_POWER_REACTOR_OT);
				flag[8] = Read_InPoint(group, 1, I_MCCB_FAIL); //lki_111010
				flag[9] = Read_InPoint(group, 1, I_DC_LINK_FUSE_FAIL); //phb_230710
				val3 = 0;
				for(i=0; i < 10; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x02;
				}

				if(myData->dio.signal[DIO_SIG_REMOTE_PS] == P100) {
					val |= 0x01;
				}

				flag[0] = Read_InPoint(group, 1, I_CONVERTER_OV);
				flag[1] = Read_InPoint(group, 1, I_CONVERTER_OC);
				flag[2] = Read_InPoint(group, 1, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(group, 1, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(group, 1, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				switch(myData->AppControl.config.systemModel) {
					case C_KBIA_500V_250A_100A_250KW:	//shh_200916
					case C_JBTP_500V_250A_100A_500KW:	//shh_200927
					case C_LGC_500V_250A_100A_50A_500KW: //phb_220902
					case C_LGC_500V_250A_125A_50A_500KW: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_2: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_3: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_4: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_5: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_6: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_7: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_8: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_9: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_210222
					case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_210222
					case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
					case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
					case C_ADD_800V_350A_200A_50A_380KW:	//shh_200904
					case C_HYUNDAI_1000V_250A_100A_500KW:	//shh_200513
					case C_KOSTA_1000V_250A_100A_600KW:		//shh_201102
					case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
					case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
					case C_KTC_1500V_600A_200A_400KW:		//ktg_200807
						if(myData->ChAttribute[0].opType != OP_INDEPENDENT
							&& myData->ChAttribute[1].chNo_master == 0
							&& myData->ChAttribute[2].chNo_master == 0
							&& myData->ChAttribute[3].chNo_master == 0) {
							val |= 0x40;
						}
						break;
					default:
						if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
							val |= 0x40;
						}
						break;
				}//kjhw_170630e

				if(myData->cData[0].op.state == C_RUN) {
					val |= 0x08;
					if(myData->cData[0].op.stepType == STEP_CHARGE) {
						val |= 0x10;
					} else if(myData->cData[0].op.stepType == STEP_DISCHARGE
						|| myData->cData[0].op.stepType == STEP_Z) {
						val |= 0x20;
					} else if(myData->cData[0].op.stepType == STEP_PATTERN
						|| myData->cData[0].op.stepType == STEP_EXTERNAL_CAN) {
						if(myData->cData[0].op.Isens >= 0) {
							val |= 0x10;
						} else {
							val |= 0x20;
						}
					}
				}

				val |= 0x04; //power led
			} else if(myData->COM.config.functionModel[comPort] == 2
				&& addr == 0 && type >= 10) {
				//C(mcu2_ch4) 1bd ch2 rs485
				val = 0;
				flag[0] = Read_InPoint(group, 2, I_CONVERTER_OV);
				flag[1] = Read_InPoint(group, 2, I_CONVERTER_OC);
				flag[2] = Read_InPoint(group, 2, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(group, 2, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(group, 2, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				//jhkw_180213s
				if(myData->ChAttribute[1].opType != OP_INDEPENDENT
					|| myData->ChAttribute[1].chNo_master == 0) {
				//jhkw_180213e
					val |= 0x40;
				}

				if(myData->cData[1].op.state == C_RUN) {
					val |= 0x08;
					if(myData->cData[1].op.stepType == STEP_CHARGE) {
						val |= 0x10;
					} else if(myData->cData[1].op.stepType == STEP_DISCHARGE
						|| myData->cData[1].op.stepType == STEP_Z) {
						val |= 0x20;
					} else if(myData->cData[1].op.stepType == STEP_PATTERN
						|| myData->cData[1].op.stepType == STEP_EXTERNAL_CAN) {
						if(myData->cData[1].op.Isens >= 0) {
							val |= 0x10;
						} else {
							val |= 0x20;
						}
					}
				}

				val |= 0x04; //power led
			} else if(myData->COM.config.functionModel[comPort] == 2
				&& addr == 1 && type < 10) {
				//C(mcu2_ch4) 2bd ch1 rs485
				val = 0;
				flag[0] = Read_InPoint(group, 2, I_FUSE_FAIL);	//kjhw_170630s
				flag[1] = Read_InPoint(group, 2, I_INVERTER_HV_FAULT);
				flag[2] = Read_InPoint(group, 2, I_INVERTER_MODULE_R_FAIL);
				flag[3] = Read_InPoint(group, 2, I_INVERTER_MODULE_S_FAIL);
				flag[4] = Read_InPoint(group, 2, I_INVERTER_MODULE_T_FAIL);
				flag[5] = Read_InPoint(group, 2, I_INVERTER_OT);
				flag[6] = Read_InPoint(group, 2, I_POWER_TRANS_OT);
				flag[7] = Read_InPoint(group, 2, I_POWER_REACTOR_OT);
				flag[8] = Read_InPoint(group, 2, I_DC_LINK_FUSE_FAIL); //phb_230710
				//flag[8] = Read_InPoint(group, 1, I_MCCB_FAIL); //lki_111010
				val3 = 0;
				for(i=0; i < 9; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x02;
				}

				if(myData->dio.signal[DIO_SIG_REMOTE_PS] == P100) {
					val |= 0x01;
				} //kjhw_170630e

				flag[0] = Read_InPoint(group, 3, I_CONVERTER_OV);
				flag[1] = Read_InPoint(group, 3, I_CONVERTER_OC);
				flag[2] = Read_InPoint(group, 3, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(group, 3, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(group, 3, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				switch(myData->AppControl.config.systemModel) {
					case C_KBIA_500V_250A_100A_250KW:	//shh_200916
					case C_JBTP_500V_250A_100A_500KW:	//shh_200927
					case C_LGC_500V_250A_100A_50A_500KW: //phb_220902
					case C_LGC_500V_250A_125A_50A_500KW: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_2: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_3: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_4: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_5: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_6: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_7: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_8: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_9: //kjh_190627
					case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_210222
					case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_210222
					case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
					case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
					case C_ADD_800V_350A_200A_50A_380KW:	//shh_200904
					case C_HYUNDAI_1000V_250A_100A_500KW:	//shh_200513
					case C_KOSTA_1000V_250A_100A_600KW:		//shh_201102
					case C_HLGP_1000V_300A_100A_50A_600KW:		//ktg_200811
					case C_HLGP_1000V_300A_100A_50A_600KW_2:	//ktg_200811
					case C_KTC_1500V_600A_200A_400KW:		//ktg_200807
						if(myData->ChAttribute[0].opType != OP_INDEPENDENT
							&& myData->ChAttribute[1].chNo_master == 0
							&& myData->ChAttribute[2].chNo_master == 0
							&& myData->ChAttribute[3].chNo_master == 0) {
							val |= 0x40;
						}
						break;
					default:
						if(myData->ChAttribute[2].opType != OP_INDEPENDENT
							|| myData->ChAttribute[2].chNo_master == 0) {
							val |= 0x40;
						}
						break;
				}//kjhw_170630e
				//kjhw_170630s	//SKI_120V_400A 4ch
				if(myData->cData[2].op.state == C_RUN) {
					val |= 0x08;
					if(myData->cData[2].op.stepType == STEP_CHARGE) {
						val |= 0x10;
					} else if(myData->cData[2].op.stepType == STEP_DISCHARGE
						|| myData->cData[2].op.stepType == STEP_Z) {
						val |= 0x20;
					} else if(myData->cData[2].op.stepType == STEP_PATTERN
						|| myData->cData[2].op.stepType == STEP_EXTERNAL_CAN) {
						if(myData->cData[2].op.Isens >= 0) {
							val |= 0x10;
						} else {
							val |= 0x20;
						}
					}
				}//kjhw_170630e

				val |= 0x04; //power led
			//} else if(myData->COM.config.functionModel[comPort] == 2
			//	&& addr == 1 && type >= 10) {
			} else {
				//C(mcu2_ch4) 2bd ch2 rs485
				val = 0;
				flag[0] = Read_InPoint(group, 4, I_CONVERTER_OV);
				flag[1] = Read_InPoint(group, 4, I_CONVERTER_OC);
				flag[2] = Read_InPoint(group, 4, I_CONVERTER_DCOV);
				flag[3] = Read_InPoint(group, 4, I_CONVERTER_MODULE_FAULT);
				flag[4] = Read_InPoint(group, 4, I_CONVERTER_OT);
				val3 = 0;
				for(i=0; i < 5; i++) {
					val3 += flag[i];
				}
				if(val3 != 0) {
					val |= 0x80;
				}

				if(myData->ChAttribute[2].opType != OP_INDEPENDENT
					|| myData->ChAttribute[3].chNo_master == 0) {
					val |= 0x40;
				}

				if(myData->cData[3].op.state == C_RUN) {
					val |= 0x08;
					if(myData->cData[3].op.stepType == STEP_CHARGE) {
						val |= 0x10;
					} else if(myData->cData[3].op.stepType == STEP_DISCHARGE
						|| myData->cData[3].op.stepType == STEP_Z) {
						val |= 0x20;
					} else if(myData->cData[3].op.stepType == STEP_PATTERN
						|| myData->cData[3].op.stepType == STEP_EXTERNAL_CAN) {
						if(myData->cData[3].op.Isens >= 0) {
							val |= 0x10;
						} else {
							val |= 0x20;
						}
					}
				}

				val |= 0x04; //power led
			}	//kjhw_121016e
			break;
		case 1: //V
			if(type < 10) {
				cmd.data[3] = '0';
				cmd.data[4] = '1';
			} else {
				cmd.data[3] = '1';
				cmd.data[4] = '1';
			}

			val2 = 1;
			if(test == 1) {
				val = -888888888; //-888.888888V
				break;
			} else if(test == 2) {
				val = myData->mData.misc.timer_1sec * 1000000; //kjg_d
				break;
			}

			//kjhw_121016s
			if(myData->COM.config.functionModel[comPort] == 2) {
				//C(mcu2 and 4ch)
				if(addr == 0) {
					if(type < 10) {	//ch1
						if(myData->ChAttribute[0].chNo_master == 0) val = 0.0;
						else val = myData->cData[0].op.Vsens;
					} else {	//ch2
						if(myData->ChAttribute[1].chNo_master == 0) val = 0.0;
						else val = myData->cData[1].op.Vsens;
					}
				} else {
					if(type < 10) {	//ch3
						if(myData->ChAttribute[2].chNo_master == 0) val = 0.0;
						else val = myData->cData[2].op.Vsens;
					} else {	//ch4
						if(myData->ChAttribute[3].chNo_master == 0) val = 0.0;
						else val = myData->cData[3].op.Vsens;
					}
				}
			} else {
				//A(rs232),B(rs485)
				if(addr == 0) {
					if(type < 10) {
						val = myData->cData[0].op.Vsens;
					} else {
						val = myData->cData[1].op.Vsens;
					}
				} else {
					val = myData->cData[addr].op.Vsens;
				}
			}	//kjhw_121016e

			//kjhw_130917s
			//rs232 ch2 or rs485 ch2
			//ch1,ch2 parallel if ch2 zero
			if((myData->COM.config.functionModel[comPort] == 0 && type >= 10)
				|| (myData->COM.config.functionModel[comPort] == 1
				&& addr == 1)) {
				if(myData->ChAttribute[1].chNo_master == 0) {
					val = 0.0;
				}
			}	//kjhw_130917e

			break;
		case 2: //I
			if(type < 10) {
				cmd.data[3] = '0';
				cmd.data[4] = '2';
			} else {
				cmd.data[3] = '1';
				cmd.data[4] = '2';
			}

			val2 = 1;
			if(test == 1) {
				val = 999999999; //999.999999A
				break;
			} else if(test == 2) {
				val = myData->mData.misc.timer_1sec * 1000000; //kjg_d
				break;
			}

			//kjhw_121016s
			if(myData->COM.config.functionModel[comPort] == 2) {
				//C(mcu2 and 4ch)
				if(addr == 0) {
					if(type < 10) {	//ch1
						if(myData->ChAttribute[0].chNo_master == 0) val = 0.0;
						else val = myData->cData[0].op.Isens;
					} else {	//ch2
						if(myData->ChAttribute[1].chNo_master == 0) val = 0.0;
						else val = myData->cData[1].op.Isens;
					}
				} else {
					if(type < 10) {	//ch3
						if(myData->ChAttribute[2].chNo_master == 0) val = 0.0;
						else val = myData->cData[2].op.Isens;
					} else {	//ch4
						if(myData->ChAttribute[3].chNo_master == 0) val = 0.0;
						else val = myData->cData[3].op.Isens;
					}
				}
			} else {
				//A(rs232),B(rs485)
				if(addr == 0) {
					if(type < 10) {
						val = myData->cData[0].op.Isens;
					} else {
						val = myData->cData[1].op.Isens;
					}
				} else {
					val = myData->cData[addr].op.Isens;
				}
			}	//kjhw_121016e

			//kjhw_130917s
			if(myData->COM.config.functionModel[comPort] == 0) {
				if(type < 10) { //rs232 ch1 parallel
					if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
						val = myData->cData[0].op.Isens;
						for(i=0; i < MAX_SLAVE_CH; i++) {
							j = (int)myData->ChAttribute[0].chNo_slave[i] - 1;
							if(j >= 0) {
								val += myData->cData[j].op.Isens;
							}
						}
					}
				} else {	//rs232 ch2
					if(myData->ChAttribute[1].chNo_master == 0) {
						val = 0.0;
					}
				}
			} else if(myData->COM.config.functionModel[comPort] == 1) {
				if(addr == 0) { //rs485 ch1 parallel
					if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
						val = myData->cData[0].op.Isens;
						for(i=0; i < MAX_SLAVE_CH; i++) {
							j = (int)myData->ChAttribute[0].chNo_slave[i] - 1;
							if(j >= 0) {
								val += myData->cData[j].op.Isens;
							}
						}
					}
				} else {	//rs485 ch2,ch3,ch4
					if(myData->ChAttribute[addr].chNo_master == 0) {
						val = 0.0;
					}
				}
			//kjhw_130917e
			//SKI_120V_400A 4ch	//kjhw_170630s
			} else if(myData->COM.config.functionModel[comPort] == 2) {
				if((addr == 0) && (type < 10)) { //rs485 ch1,ch2 parallel
					if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
						val = myData->cData[0].op.Isens;
						for(i=0; i < MAX_SLAVE_CH; i++) {
							j = (int)myData->ChAttribute[0].chNo_slave[i] - 1;
							if(j >= 0) {
								val += myData->cData[j].op.Isens;
							}
						}
					}
				} else if((addr == 1) && (type < 10)) {	//rs485 ch3,ch4 parallel
					if(myData->ChAttribute[2].opType != OP_INDEPENDENT) {
						val = myData->cData[2].op.Isens;
						for(i=0; i < MAX_SLAVE_CH; i++) {
							j = (int)myData->ChAttribute[2].chNo_slave[i] - 1;
							if(j >= 0) {
								val += myData->cData[j].op.Isens;
							}
						}
					}
				}
			}//kjhw_170630e
			break;
		case 3: //P
			if(type < 10) {
				cmd.data[3] = '0';
				cmd.data[4] = '3';
			} else {
				cmd.data[3] = '1';
				cmd.data[4] = '3';
			}

			//kjh_220321 val2 = 1;
			val2 = 2;	//kjh_220321
			if(test == 1) {
				val = -789789789; //-789.789789kW
				break;
			} else if(test == 2) {
				val = myData->mData.misc.timer_1sec * 1000000; //kjg_d
				break;
			}
			//kjhw_121016s
			if(myData->COM.config.functionModel[comPort] == 2) {
				//C(mcu2 and 4ch)
				if(addr == 0) {
					if(type < 10) {	//ch1
						if(myData->ChAttribute[0].chNo_master == 0) val = 0.0;
						else val = myData->cData[0].op.watt;
					} else {	//ch2
						if(myData->ChAttribute[1].chNo_master == 0) val = 0.0;
						else val = myData->cData[1].op.watt;
					}
				} else {
					if(type < 10) {	//ch3
						if(myData->ChAttribute[2].chNo_master == 0) val = 0.0;
						else val = myData->cData[2].op.watt;
					} else {	//ch4
						if(myData->ChAttribute[3].chNo_master == 0) val = 0.0;
						else val = myData->cData[3].op.watt;
					}
				}
			} else {
				//A(rs232),B(rs485)
				if(addr == 0) {
					if(type < 10) {
						val = myData->cData[0].op.watt;
					} else {
						val = myData->cData[1].op.watt;
					}
				} else {
					val = myData->cData[addr].op.watt;
				}
			}
			//kjhw_121016e

			//kjhw_130917s
			if(myData->COM.config.functionModel[comPort] == 0) {
				if(type < 10) { //rs232 ch1 parallel
					if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
						val = myData->cData[0].op.watt;
						for(i=0; i < MAX_SLAVE_CH; i++) {
							j = (int)myData->ChAttribute[0].chNo_slave[i] - 1;
							if(j >= 0) {
								val += myData->cData[j].op.watt;
							}
						}
					}
				} else {	//rs232 ch2
					if(myData->ChAttribute[1].chNo_master == 0) {
						val = 0.0;
					}
				}
			} else if(myData->COM.config.functionModel[comPort] == 1) {
				if(addr == 0) { //rs485 ch1 parallel
					if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
						val = myData->cData[0].op.watt;
						for(i=0; i < MAX_SLAVE_CH; i++) {
							j = (int)myData->ChAttribute[0].chNo_slave[i] - 1;
							if(j >= 0) {
								val += myData->cData[j].op.watt;
							}
						}
					}
				} else {	//rs485 ch2,ch3,ch4
					if(myData->ChAttribute[addr].chNo_master == 0) {
						val = 0.0;
					}
				}
			//kjhw_130917e
			//SKI_120V_400A 4ch	//kjhw_170630s
			} else if(myData->COM.config.functionModel[comPort] == 2) {
				if((addr == 0) && (type < 10)) { //rs485 ch1,ch2 parallel
					if(myData->ChAttribute[0].opType != OP_INDEPENDENT) {
						val = myData->cData[0].op.watt;
						for(i=0; i < MAX_SLAVE_CH; i++) {
							j = (int)myData->ChAttribute[0].chNo_slave[i] - 1;
							if(j >= 0) {
								val += myData->cData[j].op.watt;
							}
						}
					}
				} else if((addr == 1) && (type < 10)) { //rs485 ch3,ch4 parallel_jhkw_190514
					if(myData->ChAttribute[2].opType != OP_INDEPENDENT) {
						val = myData->cData[2].op.watt;
						for(i=0; i < MAX_SLAVE_CH; i++) {
							j = (int)myData->ChAttribute[2].chNo_slave[i] - 1;
							if(j >= 0) {
								val += myData->cData[j].op.watt;
							}
						}
					}
				}
			}//kjhw_170630e
			if(myData->mData.config.ratioP == BASE_UNIT) val = (long)(val / 100);	//kjh_220321
			break;
		default:
			val = 0;
			val2 = -1;
			break;
	}

	if(val2 == 0) { //flag
		val3 = (val & 0xF0) >> 4;
		if(val3 < 10) val3 += 0x30;
		else val3 += 0x37;
		cmd.data[5] = val3;

		val3 = val & 0x0F;
		if(val3 < 10) val3 += 0x30;
		else val3 += 0x37;
		cmd.data[6] = val3;

		val3 = 0;
	} else { //if(val2 == 1) { //uV, uA, mW
		if(val >= 0) cmd.data[5] = ' ';
		else {
			cmd.data[5] = '-';
			val = val * (-1);
		}
		
		if(val == 0x80000000) val = 2100000000;

		if((myData->mData.config.ratioV == MICRO_UNIT)
			&& (myData->mData.config.ratioI == MICRO_UNIT)) {
			if(val >= 2100000000) {
				cmd.data[6] = 'F';
				cmd.data[7] = 'F';
				cmd.data[8] = 'F';
				cmd.data[9] = 'F';
				cmd.data[10] = 'F';
			} else if(val >= 1000000000) { //1000.
				cmd.data[6] = val / 1000000000 + 0x30;
				val2 = val % 1000000000;
				cmd.data[7] = val2 / 100000000 + 0x30;
				val2 = val2 % 100000000;
				cmd.data[8] = val2 / 10000000 + 0x30;
				val2 = val2 % 10000000;
				cmd.data[9] = val2 / 1000000 + 0x30;
				val2 = val2 % 1000000;
				if(val2 >= 500000 && cmd.data[9] < 0x39) cmd.data[9] += 1;	//u_unit 0.5V
				cmd.data[10] = '.';
			} else if(val >= 100000000) { //100.0
				cmd.data[6] = val / 100000000 + 0x30;
				val2 = val % 100000000;
				cmd.data[7] = val2 / 10000000 + 0x30;
				val2 = val2 % 10000000;
				cmd.data[8] = val2 / 1000000 + 0x30;
				val2 = val2 % 1000000;
				cmd.data[9] = '.';
				cmd.data[10] = val2 / 100000 + 0x30;
				val2 = val2 % 100000;
				if(val2 >= 50000 && cmd.data[10] < 0x39) cmd.data[10] += 1; //u_unit 0.05V
			} else if(val >= 10000000) { //10.00
				cmd.data[6] = val / 10000000 + 0x30;
				val2 = val % 10000000;
				cmd.data[7] = val2 / 1000000 + 0x30;
				val2 = val2 % 1000000;
				cmd.data[8] = '.';
				cmd.data[9] = val2 / 100000 + 0x30;
				val2 = val2 % 100000;
				cmd.data[10] = val2 / 10000 + 0x30;
				val2 = val2 % 10000;
				if(val2 >= 5000 && cmd.data[10] < 0x39) cmd.data[10] += 1;
			} else if(val >= 1000000) { //1.000
				cmd.data[6] = val / 1000000 + 0x30;
				val2 = val % 1000000;
				cmd.data[7] = '.';
				cmd.data[8] = val2 / 100000 + 0x30;
				val2 = val2 % 100000;
				cmd.data[9] = val2 / 10000 + 0x30;
				val2 = val2 % 10000;
				cmd.data[10] = val2 / 1000 + 0x30;
				val2 = val2 % 1000;
				if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
			} else if(val >= 100000) { //0.100
				cmd.data[6] = 0x30;
				cmd.data[7] = '.';
				cmd.data[8] = val / 100000 + 0x30;
				val2 = val % 100000;
				cmd.data[9] = val2 / 10000 + 0x30;
				val2 = val2 % 10000;
				cmd.data[10] = val2 / 1000 + 0x30;
				val2 = val2 % 1000;
				if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
			} else if(val >= 10000) { //0.010
				cmd.data[6] = 0x30;
				cmd.data[7] = '.';
				cmd.data[8] = 0x30;
				cmd.data[9] = val / 10000 + 0x30;
				val2 = val % 10000;
				cmd.data[10] = val2 / 1000 + 0x30;
				val2 = val2 % 1000;
				if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
			} else { //if(val >= 1000) { //0.001
				cmd.data[6] = 0x30;
				cmd.data[7] = '.';
				cmd.data[8] = 0x30;
				cmd.data[9] = 0x30;
				cmd.data[10] = val / 1000 + 0x30;
				val2 = val % 1000;
				if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
			}
		} else if((myData->mData.config.ratioV == MILLI_UNIT)
			&& (myData->mData.config.ratioI == MILLI_UNIT)) {
			if(val >= 2100000000) {	//kjh_220321s
				cmd.data[6] = 'F';
				cmd.data[7] = 'F';
				cmd.data[8] = 'F';
				cmd.data[9] = 'F';
				cmd.data[10] = 'F';
			} else if(val >= 1000000) { //1000.
				cmd.data[6] = val / 1000000 + 0x30;
				val2 = val % 1000000;
				cmd.data[7] = val2 / 100000 + 0x30;
				val2 = val2 % 100000;
				cmd.data[8] = val2 / 10000 + 0x30;
				val2 = val2 % 10000;
				cmd.data[9] = val2 / 1000 + 0x30;
				val2 = val2 % 1000;
				if(val2 >= 500 && cmd.data[9] < 0x39) cmd.data[9] += 1;
				cmd.data[10] = '.';
			} else if(val >= 100000) { //100.0
				cmd.data[6] = val / 100000 + 0x30;
				val2 = val % 100000;
				cmd.data[7] = val2 / 10000 + 0x30;
				val2 = val2 % 10000;
				cmd.data[8] = val2 / 1000 + 0x30;
				val2 = val2 % 1000;
				cmd.data[9] = '.';
				cmd.data[10] = val2 / 100 + 0x30;
				val2 = val2 % 100;
				if(val2 >= 50 && cmd.data[10] < 0x39) cmd.data[10] += 1;
			} else if(val >= 10000) { //10.00
				cmd.data[6] = val / 10000 + 0x30;
				val2 = val % 10000;
				cmd.data[7] = val2 / 1000 + 0x30;
				val2 = val2 % 1000;
				cmd.data[8] = '.';
				cmd.data[9] = val2 / 100 + 0x30;
				val2 = val2 % 100;
				cmd.data[10] = val2 / 10 + 0x30;
				val2 = val2 % 10;
				if(val2 >= 5 && cmd.data[10] < 0x39) cmd.data[10] += 1;
			} else if(val >= 1000) { //1.000
				cmd.data[6] = val / 1000 + 0x30;
				val2 = val % 1000;
				cmd.data[7] = '.';
				cmd.data[8] = val2 / 100 + 0x30;
				val2 = val2 % 100;
				cmd.data[9] = val2 / 10 + 0x30;
				val2 = val2 % 10;
				cmd.data[10] = val2 / 1 + 0x30;
				//val2 = val2 % 1000;
				//if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
			} else if(val >= 100) { //0.100
				cmd.data[6] = 0x30;
				cmd.data[7] = '.';
				cmd.data[8] = val / 100 + 0x30;
				val2 = val % 100;
				cmd.data[9] = val2 / 10 + 0x30;
				val2 = val2 % 10;
				cmd.data[10] = val2 / 1 + 0x30;
				//val2 = val2 % 1000;
				//if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
			} else if(val >= 10) { //0.010
				cmd.data[6] = 0x30;
				cmd.data[7] = '.';
				cmd.data[8] = 0x30;
				cmd.data[9] = val / 10 + 0x30;
				val2 = val % 10;
				cmd.data[10] = val2 / 1 + 0x30;
				//val2 = val2 % 1000;
				//if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
			} else { //if(val >= 1000) { //0.001
				cmd.data[6] = 0x30;
				cmd.data[7] = '.';
				cmd.data[8] = 0x30;
				cmd.data[9] = 0x30;
				cmd.data[10] = val / 1 + 0x30;
				//val2 = val % 1000;
				//if(val2 >= 500 && cmd.data[10] < 0x39) cmd.data[10] += 1;
			}
		}//kjh_220321e
		val3 = 1;
	}

	checksum = 0;
	checksum_type = 1; //0:dec, 1:hex
	for(i=1; i < (cmd_size - 5); i++) {
		checksum += (int)cmd.data[i];
	}

	if(checksum_type == 0) { //dec
		tmp = (checksum / 1000) + 0x30;
		cmd.data[i] = (char)tmp;

		checksum = checksum % 1000;
		tmp = (checksum / 100) + 0x30;
		i++;
		cmd.data[i] = (char)tmp;

		checksum = checksum % 100;
		tmp = (checksum / 10) + 0x30;
		i++;
		cmd.data[i] = (char)tmp;

		tmp = (checksum % 10) + 0x30;
		i++;
		cmd.data[i] = (char)tmp;
	} else { //hex
		tmp = (checksum & 0xF000) >> 12;
		if(tmp < 10) {
			tmp += 0x30;
		} else {
			tmp += 0x37;
		}
		cmd.data[i] = (char)tmp;

		tmp = (checksum & 0x0F00) >> 8;
		if(tmp < 10) {
			tmp += 0x30;
		} else {
			tmp += 0x37;
		}
		i++;
		cmd.data[i] = (char)tmp;

		tmp = (checksum & 0x00F0) >> 4;
		if(tmp < 10) {
			tmp += 0x30;
		} else {
			tmp += 0x37;
		}
		i++;
		cmd.data[i] = (char)tmp;

		tmp = checksum & 0x000F;
		if(tmp < 10) {
			tmp += 0x30;
		} else {
			tmp += 0x37;
		}
		i++;
		cmd.data[i] = (char)tmp;
	}

	i++;
	cmd.data[i] = 0x03; //ETX

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void COM_Signal_Check_CB_7018(int comPort)
{
	int i;

	switch(myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]) {
		case P1:
			send_cmd_initialize_CB_7018(1, 1, 1, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P2:
			send_cmd_initialize_CB_7018(2, 1, 1, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P3:
			send_cmd_initialize_CB_7018(1, 1, 2, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P4:
			send_cmd_initialize_CB_7018(2, 1, 2, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P5:
			send_cmd_initialize_CB_7018(1, 1, 3, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P6:
			send_cmd_initialize_CB_7018(2, 1, 3, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P7:
			send_cmd_initialize_CB_7018(1, 1, 4, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P8:
			send_cmd_initialize_CB_7018(2, 1, 4, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P11:
			send_cmd_initialize_CB_7018(1, 2, 1, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P12:
			send_cmd_initialize_CB_7018(2, 2, 1, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P13:
			send_cmd_initialize_CB_7018(1, 2, 2, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P14:
			send_cmd_initialize_CB_7018(2, 2, 2, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P15:
			send_cmd_initialize_CB_7018(1, 2, 3, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P16:
			send_cmd_initialize_CB_7018(2, 2, 3, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P17:
			send_cmd_initialize_CB_7018(1, 2, 4, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P18:
			send_cmd_initialize_CB_7018(2, 2, 4, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P21:
			send_cmd_initialize_CB_7018(1, 3, 1, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P22:
			send_cmd_initialize_CB_7018(2, 3, 1, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P23:
			send_cmd_initialize_CB_7018(1, 3, 2, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P24:
			send_cmd_initialize_CB_7018(2, 3, 2, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P25:
			send_cmd_initialize_CB_7018(1, 3, 3, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P26:
			send_cmd_initialize_CB_7018(2, 3, 3, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P27:
			send_cmd_initialize_CB_7018(1, 3, 4, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P28:
			send_cmd_initialize_CB_7018(2, 3, 4, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P31:
			send_cmd_initialize_CB_7018(1, 4, 1, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P32:
			send_cmd_initialize_CB_7018(2, 4, 1, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P33:
			send_cmd_initialize_CB_7018(1, 4, 2, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P34:
			send_cmd_initialize_CB_7018(2, 4, 2, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P35:
			send_cmd_initialize_CB_7018(1, 4, 3, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P36:
			send_cmd_initialize_CB_7018(2, 4, 3, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		case P37:
			send_cmd_initialize_CB_7018(1, 4, 4, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE]++;
			break;
		case P38:
			send_cmd_initialize_CB_7018(2, 4, 4, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_INITIALIZE] = P0;
			break;
		default: break;
	}

	switch(myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE]) {
		case P1:
			send_cmd_request_CB_7018(0, 1, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P21;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			break;
		case P2:
			send_cmd_request_CB_7018(0, 2, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P22;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			break;
		case P3:
			send_cmd_request_CB_7018(0, 3, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P23;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			break;
		case P4:
			send_cmd_request_CB_7018(0, 4, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P24;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			break;
		case P5:
			send_cmd_request_CB_7018(0, 5, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P25;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			break;
		case P6:
			send_cmd_request_CB_7018(0, 6, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P26;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			break;
		case P7:
			send_cmd_request_CB_7018(0, 7, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P27;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			break;
		case P8:
			send_cmd_request_CB_7018(0, 8, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P28;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			break;
		case P9:
			send_cmd_request_CB_7018(0, 9, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P29;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			break;
		case P10:
			send_cmd_request_CB_7018(0, 10, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P30;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			break;
		case P11:
		//jhkw_181109s
			send_cmd_request_CB_7018(0, 11, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P31;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			break;
		case P12:
			send_cmd_request_CB_7018(0, 12, comPort);
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P32;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			break;
		case P13:
		//jhkw_181109e
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P0;
			myCom->misc.retry_count = 0;
			myCom->misc.retry_time = 0;
			break;
		case P21:
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time * (int)myPs->config.scan_period;
			if(i < 250) break;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				if(myData->COM.config.countMeter[comPort] > 1) {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P2;
				} else {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
				}
			}
			break;
		case P22:
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time * (int)myPs->config.scan_period;
			if(i < 250) break;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				if(myData->COM.config.countMeter[comPort] > 2) {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P3;
				} else {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
				}
			}
			break;
		case P23:
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time * (int)myPs->config.scan_period;
			if(i < 250) break;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				if(myData->COM.config.countMeter[comPort] > 3) {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P4;
				} else {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
				}
			}
			break;
		case P24:
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time * (int)myPs->config.scan_period;
			if(i < 250) break;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				if(myData->COM.config.countMeter[comPort] > 4) {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P5;
				} else {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
				}
			}
			break;
		case P25:
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time * (int)myPs->config.scan_period;
			if(i < 250) break;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				if(myData->COM.config.countMeter[comPort] > 5) {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P6;
				} else {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
				}
			}
			break;
		case P26:
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time * (int)myPs->config.scan_period;
			if(i < 250) break;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				if(myData->COM.config.countMeter[comPort] > 6) {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P7;
				} else {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
				}
			}
			break;
		case P27:
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time * (int)myPs->config.scan_period;
			if(i < 250) break;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				if(myData->COM.config.countMeter[comPort] > 7) {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P8;
				} else {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
				}
			}
			break;
		case P28:
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time * (int)myPs->config.scan_period;
			if(i < 250) break;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				if(myData->COM.config.countMeter[comPort] > 8) {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P9;
				} else {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
				}
			}
			break;
		case P29:
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time * (int)myPs->config.scan_period;
			if(i < 250) break;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				if(myData->COM.config.countMeter[comPort] > 9) {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P10;
				} else {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
				}
			}
			break;
		case P30:
			//jhkw_181109s
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time * (int)myPs->config.scan_period;
			if(i < 250) break;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				if(myData->COM.config.countMeter[comPort] > 10) {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P11;
				} else {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
				}
			}
			break;
		case P31:
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time * (int)myPs->config.scan_period;
			if(i < 250) break;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				if(myData->COM.config.countMeter[comPort] > 11) {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P12;
				} else {
					myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
				}
			}
			break;
		case P32:
			//jhkw_181109e
			myCom->misc.retry_time++;
			i = (int)myCom->misc.retry_time * (int)myPs->config.scan_period;
			if(i < 250) break;

			myCom->misc.retry_count++;
			if(myCom->misc.retry_count >= 3) {
				myCom->misc.retry_count = 0;
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
			break;
		default:
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P0;
			break;
	}
}

int CmdHeader_Check_CB_7018(int comPort)
{
	int length;
	S_COM_ANALOG_METER_CMD_HEADER header;

	length = sizeof(S_COM_ANALOG_METER_CMD_HEADER);
	memcpy((char *)&header, (char *)&myCom->rcvCmd.cmd, length);

	if(length >= myCom->rcvCmd.cmdSize) {
		rtl_printf("com:%d RcvCmd size error length:%d rcv:%d\n",
			comPort, length, myCom->rcvCmd.cmdSize);
		return -1;
	}

	if(header.stx != '!' && header.stx != '?' && header.stx != '>') {
		rtl_printf("com:%d RcvCmd stx error 0x%x\n", comPort, header.stx);
		return -2;
	}

	if(header.stx == '!') {
		if(strncmp(header.addr,"01",2) != 0
			&& strncmp(header.addr,"02",2) != 0
			&& strncmp(header.addr,"03",2) != 0
			&& strncmp(header.addr,"04",2) != 0
			&& strncmp(header.addr,"05",2) != 0
			&& strncmp(header.addr,"06",2) != 0	//jhkw_151129s
			&& strncmp(header.addr,"07",2) != 0
			&& strncmp(header.addr,"08",2) != 0
			&& strncmp(header.addr,"09",2) != 0
			//&& strncmp(header.addr,"10",2) != 0) {	//jhkw_151129e jhkw_181109s
			&& strncmp(header.addr,"0A",2) != 0
			&& strncmp(header.addr,"0B",2) != 0
			&& strncmp(header.addr,"0C",2) != 0) {	//jhkw_181109e
			rtl_printf("com:%d RcvCmd addr error 0x%02x 0x%02x\n", comPort,
				(unsigned char)header.addr[0], (unsigned char)header.addr[1]);
			return -3;
		}
	}

	return 0;
}

int rcv_cmd_answer_CB_7018(int comPort)
{
	char answer[100];
	int i, index=0, temp_bd=0, temp_ch=-1, is_plus=1;//kjh_211021 group;
	long temp[32], cmd_size;
	float val1, val2;
	int functionModel, module_per_ch; //kjhw_131011
	
	memset((char *)&answer, 0, sizeof(answer));
	cmd_size = myCom->rcvCmd.cmdSize;
	memcpy((char *)&answer, (char *)&myCom->rcvCmd.cmd, cmd_size);
	
	//kjhw_131011
	module_per_ch = 0;
	functionModel = (int)myData->COM.config.functionModel[comPort];

	if(answer[0] != '>') return 0;

	for(i=0; i < 100; i++) {
		if(answer[i] == S_CR) {
			index = i;
			break;
		} else {
		}
	}

	//jhkw_151129
	//temp_bd = (int)(myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] % 10) - 1;
	temp_bd = (int)(myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] % 20) - 1;
	if(temp_bd < 0) return 0;

	if(answer[1] == '+') {	//kjg_121105_s
		is_plus = 1;
	} else {
		is_plus = -1;
	}
	temp_ch = 0;
	temp[temp_ch] = 0;
	index = -1;

	for(i=2; i < 100; i++) {
		if(answer[i] == '+') {
			switch(index) {
				case 1:
					temp[temp_ch] *= 100;
					break;
				case 2:
					temp[temp_ch] *= 10;
					break;
				case 3:
					temp[temp_ch] *= 1;
					break;
				default:
					break;
			}
			temp[temp_ch] *= is_plus;

			is_plus = 1;
			temp_ch++;
			temp[temp_ch] = 0;
			index = -1;
		} else if(answer[i] == '-') {
			switch(index) {
				case 1:
					temp[temp_ch] *= 100;
					break;
				case 2:
					temp[temp_ch] *= 10;
					break;
				case 3:
					temp[temp_ch] *= 1;
					break;
				default:
					break;
			}
			temp[temp_ch] *= is_plus;

			is_plus = -1;
			temp_ch++;
			temp[temp_ch] = 0;
			index = -1;
		} else if(answer[i] == '.') {
			index = 0;
		} else if(answer[i] == S_CR) {
			switch(index) {
				case 1:
					temp[temp_ch] *= 100;
					break;
				case 2:
					temp[temp_ch] *= 10;
					break;
				case 3:
					temp[temp_ch] *= 1;
					break;
				default:
					break;
			}
			temp[temp_ch] *= is_plus;
			break;
		} else {
			temp[temp_ch] *= 10;
			temp[temp_ch] += (answer[i] - 0x30);
			if(index >= 0) {
				index++;
			}
		}
	} //kjg_121105_e

	if(functionModel == 0) { //CB-7018	//kjhw_131011s
		module_per_ch = 8;
	} else if(functionModel == 3) { //I-7018Z
		module_per_ch = 10;
	} else if(functionModel == 4) { //I-7033 //ktg_190516
		module_per_ch = 3;
	} //kjhw_131011e

	val2 = 0.0;
	//for(temp_ch=0; temp_ch < 8; temp_ch++) {
	//	i = temp_bd * 8 + temp_ch;
	for(temp_ch=0; temp_ch < module_per_ch; temp_ch++) {
		i = temp_bd * module_per_ch + temp_ch;
		if(myPs->config.multi_temp_cali == 1) { //multi temp cali	//shh_211221s
			if(temp[temp_ch] != 9999900) {
				cFind_Temp_Cali_Data(temp[temp_ch], i);		//khj_210802
			}
		}	//shh_211221e
		val1 = (float)temp[temp_ch] * myData->measure_cali[0][i].factor
			+ myData->measure_cali[0][i].offset;
		//myCom->value[i] = (long)val1;
		//kjhw_141027s HLGreen500V300A
		myCom->test_value[i] = (long)val1;
		if(((myCom->test_value[i] - myCom->value[i]) >= 5000)
			|| ((myCom->test_value[i] - myCom->value[i]) <= -5000)) {
			//if(myCom->fault_count[i] < 2) {
			if(myCom->fault_count[i] < 5) {
				myCom->fault_count[i]++;
			} else {
				myCom->value[i] = myCom->test_value[i];
				myCom->fault_count[i] = 0;
			}
		} else {
			myCom->value[i] = myCom->test_value[i];
			myCom->fault_count[i] = 0;
		}	//kjhw_141027e
	}

	switch((int)myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE]) {
		case P21:
			if(myData->COM.config.countMeter[comPort] > 1) {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P2;
			} else {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
			myCom->signal[COM_SIG_CALI_METER_ERROR] = 0;
			break;
		case P22:
			if(myData->COM.config.countMeter[comPort] > 2) {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P3;
			} else {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
			myCom->signal[COM_SIG_CALI_METER_ERROR] = 0;
			break;
		case P23:
			if(myData->COM.config.countMeter[comPort] > 3) {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P4;
			} else {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
			myCom->signal[COM_SIG_CALI_METER_ERROR] = 0;
			break;
		case P24:
			if(myData->COM.config.countMeter[comPort] > 4) {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P5;
			} else {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
			myCom->signal[COM_SIG_CALI_METER_ERROR] = 0;
			break;
		case P25:
			if(myData->COM.config.countMeter[comPort] > 5) {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P6;
			} else {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
			myCom->signal[COM_SIG_CALI_METER_ERROR] = 0;
			break;
		case P26:
			if(myData->COM.config.countMeter[comPort] > 6) {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P7;
			} else {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
			myCom->signal[COM_SIG_CALI_METER_ERROR] = 0;
			break;
		case P27:
			if(myData->COM.config.countMeter[comPort] > 7) {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P8;
			} else {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
			myCom->signal[COM_SIG_CALI_METER_ERROR] = 0;
			break;
		case P28:
			if(myData->COM.config.countMeter[comPort] > 8) {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P9;
			} else {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
			myCom->signal[COM_SIG_CALI_METER_ERROR] = 0;
			break;
		case P29:
			if(myData->COM.config.countMeter[comPort] > 9) {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P10;
			} else {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
			myCom->signal[COM_SIG_CALI_METER_ERROR] = 0;
			break;
		case P30:
			if(myData->COM.config.countMeter[comPort] > 10) {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P11;
			} else {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
			myCom->signal[COM_SIG_CALI_METER_ERROR] = 0;
			break;
		case P31:	//jhkw_181109s
			if(myData->COM.config.countMeter[comPort] > 11) {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P12;
			} else {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
			myCom->signal[COM_SIG_CALI_METER_ERROR] = 0;
			break;
		case P32:
			if(myData->COM.config.countMeter[comPort] > 12) {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P13;
			} else {
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
			myCom->signal[COM_SIG_CALI_METER_ERROR] = 0;
			break;	//jhkw_181109e
		}
	return 0;
}

void send_cmd_initialize_CB_7018(int type, int addr, int val, int comPort)
{
	char cmd[20];
	int rtn, cmd_size;

	memset(cmd, 0, 20);

	switch(type) {
		case 1: //set module configuration
			cmd_size = 12;
			if(addr == 1) {
				if(val == 1) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				} else if(val == 2) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '2'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '2'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '2'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				} else if(val == 3) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '3'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '3'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '3'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				} else if(val == 4) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '4'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '4'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '4'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				}
			} else if(addr == 2) {
				if(val == 1) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '2'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '2'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '2'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				} else if(val == 2) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '2'; cmd[3] = '0';
						cmd[4] = '2'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '2'; cmd[3] = '0';
						cmd[4] = '2'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '2'; cmd[3] = '0';
						cmd[4] = '2'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				} else if(val == 3) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '2'; cmd[3] = '0';
						cmd[4] = '3'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '2'; cmd[3] = '0';
						cmd[4] = '3'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '2'; cmd[3] = '0';
						cmd[4] = '3'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				} else if(val == 4) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '2'; cmd[3] = '0';
						cmd[4] = '4'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '2'; cmd[3] = '0';
						cmd[4] = '4'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '2'; cmd[3] = '0';
						cmd[4] = '4'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				}
			} else if(addr == 3) {
				if(val == 1) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '3'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '3'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '3'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				} else if(val == 2) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '3'; cmd[3] = '0';
						cmd[4] = '2'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '3'; cmd[3] = '0';
						cmd[4] = '2'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '3'; cmd[3] = '0';
						cmd[4] = '2'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				} else if(val == 3) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '3'; cmd[3] = '0';
						cmd[4] = '3'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '3'; cmd[3] = '0';
						cmd[4] = '3'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '3'; cmd[3] = '0';
						cmd[4] = '3'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				} else if(val == 4) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '3'; cmd[3] = '0';
						cmd[4] = '4'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '3'; cmd[3] = '0';
						cmd[4] = '4'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '3'; cmd[3] = '0';
						cmd[4] = '4'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				}
			} else if(addr == 4) {
				if(val == 1) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '4'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '4'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '4'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				} else if(val == 2) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '4'; cmd[3] = '0';
						cmd[4] = '2'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '4'; cmd[3] = '0';
						cmd[4] = '2'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '4'; cmd[3] = '0';
						cmd[4] = '2'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				} else if(val == 3) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '4'; cmd[3] = '0';
						cmd[4] = '3'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '4'; cmd[3] = '0';
						cmd[4] = '3'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '4'; cmd[3] = '0';
						cmd[4] = '3'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				} else if(val == 4) {
					if(myData->COM.config.readType[comPort] == READ_T) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '4'; cmd[3] = '0';
						cmd[4] = '4'; cmd[5] = '0'; cmd[6] = 'F'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					} else if(myData->COM.config.readType[comPort] == READ_V) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '4'; cmd[3] = '0';
						cmd[4] = '4'; cmd[5] = '0'; cmd[6] = '5'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516s
					} else if(myData->COM.config.readType[comPort] == READ_T_2) {
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '0';
						cmd[4] = '1'; cmd[5] = '8'; cmd[6] = '0'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
						//ktg_190516e
					} else { //READ_I
						cmd[0] = '%'; cmd[1] = '0'; cmd[2] = '4'; cmd[3] = '0';
						cmd[4] = '4'; cmd[5] = '0'; cmd[6] = '6'; cmd[7] = '0';
						cmd[8] = '6'; cmd[9] = '0'; cmd[10] = '0';
						cmd[11] = 0x0D; //\r(CR)
					}
				}
			}
			break;
		case 2: //set channel enable
			cmd_size = 7;
			if(val == 1) {
				cmd[0] = '$'; cmd[1] = '0'; cmd[2] = '1'; cmd[3] = '5';
				cmd[4] = 'F'; cmd[5] = 'F'; cmd[6] = 0x0D; //\r(CR)
			} else if(val == 2) {
				cmd[0] = '$'; cmd[1] = '0'; cmd[2] = '2'; cmd[3] = '5';
				cmd[4] = 'F'; cmd[5] = 'F'; cmd[6] = 0x0D; //\r(CR)
			} else if(val == 3) {
				cmd[0] = '$'; cmd[1] = '0'; cmd[2] = '3'; cmd[3] = '5';
				cmd[4] = 'F'; cmd[5] = 'F'; cmd[6] = 0x0D; //\r(CR)
			} else if(val == 4) {
				cmd[0] = '$'; cmd[1] = '0'; cmd[2] = '4'; cmd[3] = '5';
				cmd[4] = 'F'; cmd[5] = 'F'; cmd[6] = 0x0D; //\r(CR)
			}
			break;
		default:
//		userlog(DEBUG_LOG, psName, "Unknown cmd\n");
			return;
	}

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
//		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void send_cmd_request_CB_7018(int type, int bd, int comPort)
{
	int rtn, cmd_size;
	S_COM_ANALOG_METER_SEND_CMD_REQUEST	cmd;
	
	memset((char *)&cmd, 0, sizeof(S_COM_ANALOG_METER_SEND_CMD_REQUEST));

	switch(type) {
		case 0: //Read Analog Input
			cmd_size = 4;
			cmd.data[0] = '#';
			cmd.data[1] = '0';
			if(bd == 10) {
				cmd.data[2] = 'A';
			} else if(bd == 11) {
				cmd.data[2] = 'B';
			} else if(bd == 12) {
				cmd.data[2] = 'C';
			} else {
				cmd.data[2] = '0' + bd;
			}
			cmd.data[3] = 0x0D; //\r
			break;
		case 1: //Read CJC Temperature
			cmd_size = 5;
			if(bd == 1) {
				memcpy((char *)&cmd.data, "$013\r", cmd_size);
			} else if(bd == 2) {
				memcpy((char *)&cmd.data, "$023\r", cmd_size);
			} else if(bd == 3) {
				memcpy((char *)&cmd.data, "$033\r", cmd_size);
			} else if(bd == 4) {
				memcpy((char *)&cmd.data, "$043\r", cmd_size);
			//jhkw_151129s
			} else if(bd == 5) {
				memcpy((char *)&cmd.data, "$053\r", cmd_size);
			} else if(bd == 6) {
				memcpy((char *)&cmd.data, "$063\r", cmd_size);
			} else if(bd == 7) {
				memcpy((char *)&cmd.data, "$073\r", cmd_size);
			} else if(bd == 8) {
				memcpy((char *)&cmd.data, "$083\r", cmd_size);
			} else if(bd == 9) {
				memcpy((char *)&cmd.data, "$093\r", cmd_size);
			} else if(bd == 10) {
				memcpy((char *)&cmd.data, "$0A3\r", cmd_size);
			//jhkw_151129e
			//jhkw_181109s
			} else if(bd == 11) {
				memcpy((char *)&cmd.data, "$0B3\r", cmd_size);
			} else if(bd == 12) {
				memcpy((char *)&cmd.data, "$0C3\r", cmd_size);
			}
			//jhkw_181109s
			break;
		default:
//			userlog(DEBUG_LOG, psName, "Unknown cmd\n");
			return;
	}
	
	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
//		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

void send_cmd_close_CB_7018(int bd, int comPort)
{
	char buf[12];
	int rtn, cmd_size;

	cmd_size = 7;
	memset(buf, 0, 12);
	buf[0] = 0x1B; //ESC
	buf[1] = 'C';
	buf[2] = ' ';
	buf[3] = '0';
	buf[4] = '0' + bd;
	buf[5] = 0x0D; //\r(CR)
	buf[6] = 0x0A; //\n

	rtn = send_command((char *)&buf, cmd_size, 0, comPort);
	if(rtn < 0) {
//		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}

	memset((char *)&myCom->rcvPacket, 0, sizeof(S_COM_RCV_PACKET));
	memset((char *)&myCom->rcvCmd, 0, sizeof(S_COM_RCV_COMMAND));
}

void COM_Signal_Check_COMM_A_XL_122(int comPort)
{
	switch(myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE]) {
		case P1:
			send_cmd_request_COMM_A_XL_122(1, 1, comPort); //bd1 open
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE]++;
			myCom->misc.delay_time = 0;
			break;
		case P2:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 100) { //100ms
				//>= 1000) { //1000ms
				send_cmd_request_COMM_A_XL_122(1, 2, comPort); //bd2 open
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE]++;
				myCom->misc.delay_time = 0;
			}
			break;
		case P3:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 100) { //100ms
				//>= 1000) { //1000ms
				send_cmd_request_COMM_A_XL_122(1, 1, comPort); //bd1 open
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE]++;
				myCom->misc.delay_time = 0;
			}
			break;
		case P4:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 100) { //100ms
				//>= 1000) { //1000ms
				send_cmd_request_COMM_A_XL_122(1, 2, comPort); //bd2 open
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE]++;
				myCom->misc.delay_time = 0;
			}
			break;
		case P5:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 100) { //100ms
				//>= 1000) { //1000ms
				send_cmd_request_COMM_A_XL_122(3, 1, comPort); //bd1 scan start
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE]++;
				myCom->misc.delay_time = 0;
			}
			break;
		case P6:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 100) { //100ms
				//>= 1000) { //1000ms
				send_cmd_request_COMM_A_XL_122(3, 2, comPort); //bd2 scan start
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE]++;
				myCom->misc.delay_time = 0;
			}
			break;
		case P7:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 100) { //100ms
				//>= 1000) { //1000ms
				send_cmd_request_COMM_A_XL_122(3, 1, comPort); //bd1 scan start
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE]++;
				myCom->misc.delay_time = 0;
			}
			break;
		case P8:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 100) { //100ms
				//>= 1000) { //1000ms
				send_cmd_request_COMM_A_XL_122(3, 2, comPort); //bd2 scan start
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P11;
				myCom->misc.delay_time = 0;
			}
			break;
		case P11:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 1000) { //1sec
				//>= 2000) { //2sec
				send_cmd_request_COMM_A_XL_122(0, 1, comPort); //bd1 value request
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE]++;
				myCom->misc.delay_time = 0;
			}
			break;
		case P12:
			myCom->misc.delay_time++;
			if((myCom->misc.delay_time * myPs->config.scan_period)
				>= 1000) { //1sec
				//>= 2000) { //2sec
				send_cmd_request_COMM_A_XL_122(0, 2, comPort); //bd2 value request
				myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P11;
				myCom->misc.delay_time = 0;
			}
			break;
		default:
			myCom->signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P0;
			break;
	}
}

int CmdHeader_Check_COMM_A_XL_122(int comPort)
{
	int length;
	S_COM_ANALOG_METER_CMD_HEADER header;

	length = sizeof(S_COM_ANALOG_METER_CMD_HEADER);
	memcpy((char *)&header, (char *)&myCom->rcvCmd.cmd, length);

	if(length >= myCom->rcvCmd.cmdSize) {
		rtl_printf("com:%d RcvCmd size error length:%d rcv:%d\n",
			comPort, length, myCom->rcvCmd.cmdSize);
		return -1;
	}

	if(header.stx != '!') {
		rtl_printf("com:%d RcvCmd stx error 0x%x\n", comPort, header.stx);
		return -2;
	}

	if(strncmp(header.addr, "70", 2) != 0
		&& strncmp(header.addr, "71", 2) != 0) {
		rtl_printf("com:%d RcvCmd addr error 0x%02x 0x%02x\n", comPort,
			(unsigned char)header.addr[0],
			(unsigned char)header.addr[1]);
		return -3;
	}

	return 0;
}

int CheckSum_Check_COMM_A_XL_122(int comPort) //COMM_A
{
	char buf[64];
	int bcc, i, cmd_size, rcv_bcc;

	cmd_size = myCom->rcvCmd.cmdSize;

	memset((char *)&buf, 0, sizeof buf);
	memcpy((char *)&buf, (char *)&myCom->rcvCmd.cmd, cmd_size);

	if(buf[0] != '!') {
		return -1;
	}

	bcc = 0;
	for(i=0; i < (cmd_size - 5); i++) {
		bcc += buf[i];
	}

	rcv_bcc = (buf[cmd_size - 5] - 0x30) * 1000
		+ (buf[cmd_size - 4] - 0x30) * 100
		+ (buf[cmd_size - 3] - 0x30) * 10
		+ (buf[cmd_size - 2] - 0x30);

	if(bcc == rcv_bcc) {
		return 0;
	} else {
		//rtl_printf("com:%d RcvCmd CheckSum error (%04d:%04d)\n", comPort,
		//	bcc, rcv_bcc);
		return -2;
	}
}

int rcv_cmd_answer_COMM_A_XL_122(int comPort)
{
	char answer[128], buf[12];
	int i, temp_bd, temp_ch;
	long temp, temp2, cmd_size;
	double val1;
	
	memset((char *)&answer, 0, sizeof(answer));
	cmd_size = myCom->rcvCmd.cmdSize;
	memcpy((char *)&answer, (char *)&myCom->rcvCmd.cmd, cmd_size);
	
	if(answer[0] != '!') return 0;

	temp_bd = (int)(answer[1] - 0x30) * 10 + (int)(answer[2] - 0x30);
	temp_bd -= 70;
	if(temp_bd < 0 || temp_bd > 1) return 0;

	if(answer[3] != 'M' && answer[4] != 'D' && answer[5] != '1') return 0;
	if(answer[6] != '2' && answer[7] != '5') return 0;

	temp_ch = (int)(answer[11] - 0x30) * 10 + (int)(answer[12] - 0x30);
	temp_ch--;
	if(temp_ch < 0 || temp_ch > 15) return 0;

	if(answer[8] == 'N') { //Status bit
		memset(buf, 0, sizeof(buf));
		memcpy((char *)&buf, (char *)&answer[23], 10); //Sign bit start
		//temp = atoi(buf) * 100;

		temp = (long)(answer[24] - 0x30);
		for(i=1; i < 5; i++) {
			temp = temp * 10 + (long)(answer[24+i] - 0x30);
		}

		temp2 = (long)(answer[31] - 0x30);
		temp2 = temp2 * 10 + (long)(answer[32] - 0x30);

		val1 = (double)temp;
		if(answer[30] == '-') {
			for(i=0; i < temp2; i++)  val1 = val1 / 10.0;
		} else {
			for(i=0; i < temp2; i++)  val1 = val1 * 10.0;
		}

		val1 *= 1000.0;
		if(answer[23] == '-') {
			val1 = val1 * (-1.0);
		}
	} else if(answer[8] == 'O') {
		val1 = 9999000.0;
	} else if(answer[8] == 'S' || answer[8] == 'E') {
		val1 = -9999000.0;
	} else val1 = -9999000.0;

	i = temp_bd * 16 + temp_ch;
	val1 = (double)val1 * (double)myData->measure_cali[0][i].factor
		+ (double)myData->measure_cali[0][i].offset;
	myCom->value[i] = (long)val1;

	return 0;
}

void send_cmd_request_COMM_A_XL_122(int type, int bd, int comPort)
{
	int rtn, cmd_size, data_size;
	char cmd[32];
	
	memset((char *)&cmd, 0, 32);

	switch(type) {
		case 0: //analog value request
			cmd[0] = '@'; //start_delimiter
			cmd[1] = '7'; //addr
			cmd[2] = '0' + (bd - 1);
			cmd[3] = 'M'; //cmd
			cmd[4] = 'D';
			cmd[5] = '1';

			data_size = 0;
			cmd[6] = (data_size / 10) + 0x30;
			cmd[7] = (data_size % 10) + 0x30;

			cmd_size = make_check_sum((char *)&cmd, data_size + 8);
			break;
		case 1: //open
			cmd[0] = '@'; //start_delimiter
			cmd[1] = '7'; //addr
			cmd[2] = '0' + (bd - 1);
			cmd[3] = 'E'; //cmd
			cmd[4] = 'S';
			cmd[5] = 'O';

			data_size = 0;
			cmd[6] = (data_size / 10) + 0x30;
			cmd[7] = (data_size % 10) + 0x30;

			cmd_size = make_check_sum((char *)&cmd, data_size + 8);
			break;
		case 2: //close
			cmd[0] = '@'; //start_delimiter
			cmd[1] = '7'; //addr
			cmd[2] = '0' + (bd - 1);
			cmd[3] = 'E'; //cmd
			cmd[4] = 'S';
			cmd[5] = 'C';

			data_size = 0;
			cmd[6] = (data_size / 10) + 0x30;
			cmd[7] = (data_size % 10) + 0x30;

			cmd_size = make_check_sum((char *)&cmd, data_size + 8);
			break;
		case 3: //xl122 scan start
			cmd[0] = '@'; //start_delimiter
			cmd[1] = '7'; //addr
			cmd[2] = '0' + (bd - 1);
			cmd[3] = 'M'; //cmd
			cmd[4] = 'D';
			cmd[5] = 'S';

			data_size = 0;
			cmd[6] = (data_size / 10) + 0x30;
			cmd[7] = (data_size % 10) + 0x30;

			cmd_size = make_check_sum((char *)&cmd, data_size + 8);
			break;
		case 4: //xl122 scan stop
			cmd[0] = '@'; //start_delimiter
			cmd[1] = '7'; //addr
			cmd[2] = '0' + (bd - 1);
			cmd[3] = 'M'; //cmd
			cmd[4] = 'D';
			cmd[5] = 'E';

			data_size = 0;
			cmd[6] = (data_size / 10) + 0x30;
			cmd[7] = (data_size % 10) + 0x30;

			cmd_size = make_check_sum((char *)&cmd, data_size + 8);
			break;
		default:
//			userlog(DEBUG_LOG, psName, "Unknown cmd\n");
			return;
	}
	
	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
//		userlog(DEBUG_LOG, psName, "send cmd error !!!\n");
	}
}

int CmdHeader_Check_Calibrator(int comPort)
{
	int length;
	S_COM_CALI_SWITCH1_CMD_HEADER header;

	length = sizeof(S_COM_CALI_SWITCH1_CMD_HEADER);
	memcpy((char *)&header, (char *)&myCom->rcvCmd.cmd, length);

	if(length >= myCom->rcvCmd.cmdSize) {
		rtl_printf("com:%d RcvCmd size error length:%d rcv:%d\n",
			comPort, length, myCom->rcvCmd.cmdSize);
		return -1;
	}

	if(header.stx != '!' && header.stx != '?' && header.stx != '>') {
		rtl_printf("com:%d RcvCmd stx error 0x%x\n", comPort, header.stx);
		return -2;
	}

	if(header.stx == '!') {
		if(header.addr[0] == '6' && header.addr[1] == '0') {
		} else {
			rtl_printf("com:%d RcvCmd addr error 0x%02x 0x%02x\n", comPort,
				(unsigned char)header.addr[0], (unsigned char)header.addr[1]);
			return -3;
		}
	}

	return 0;
}

int CheckSum_Check_Calibrator(int comPort)
{
	char buf[64];
	int bcc, i, cmd_size, rcv_bcc;

	cmd_size = myCom->rcvCmd.cmdSize;

	memset((char *)&buf, 0, sizeof buf);
	memcpy((char *)&buf, (char *)&myCom->rcvCmd.cmd, cmd_size);

	if(buf[0] != '!') {
		return -1;
	}

	bcc = 0;
	for(i=0; i < (cmd_size - 5); i++) {
		bcc += buf[i];
	}

	rcv_bcc = (buf[cmd_size - 5] - 0x30) * 1000
		+ (buf[cmd_size - 4] - 0x30) * 100
		+ (buf[cmd_size - 3] - 0x30) * 10
		+ (buf[cmd_size - 2] - 0x30);

	if(bcc == rcv_bcc) {
		return 0;
	} else {
		rtl_printf("com:%d RcvCmd CheckSum error (%04d:%04d)\n", comPort,
			bcc, rcv_bcc);
		return -2;
	}
}

int rcv_cmd_answer_Calibrator(int comPort)
{
	char buf[64];
	int cmd_size, i, tmp1, tmp2;
	double val;

	cmd_size = myCom->rcvCmd.cmdSize;

	memset((char *)&buf, 0, sizeof buf);
	memcpy((char *)&buf, (char *)&myCom->rcvCmd.cmd, cmd_size);

	if(buf[3] == 'C' && buf[4] == 'O' && buf[5] == '1') {
	} else if(buf[3] == 'I' && buf[4] == 'N' && buf[5] == 'I') {
	} else if(buf[3] == 'R' && buf[4] == 'E' && buf[5] == 'Q') {
		tmp1 = (int)(buf[9] - 0x30); //digit1

		for(i=0; i < 8; i++) {
			tmp1 = tmp1 * 10 + (int)(buf[11+i] - 0x30); //digit2
		}

		tmp2 = (int)(buf[21] - 0x30); //digit3_1
		tmp2 = tmp2 * 10 + (int)(buf[22] - 0x30); //digit3_2

		val = (double)tmp1;
		if(buf[20] == '-') { //sign2
			for(i=0; i < tmp2; i++)  val = val / 10.0;
		} else {
			for(i=0; i < tmp2; i++)  val = val * 10.0;
		}

		val /= 100.0;
		if(buf[8] == '-') { //sign1
			val = val * (-1.0);
		}

		//rtl_printf("com:%d rcv value %d\n", comPort, (int)val); //kjg_d
		myPs->misc.meter_value = val;
		myPs->signal[M_SIG_CALIBRATION] = P50;
	} else {
		rtl_printf("com:%d rcv NAK\n", comPort);
		return -1;
	}

//	rtl_printf("com:%d rcv value %x %x %x\n", comPort,
//		buf[3], buf[4], buf[5]); //kjg_d

	return 0;
}

void send_cmd_on_off_Calibrator(int comPort, int val1, int val2, int val3)
{
	char cmd[32], data[6];
	unsigned char val[3];
	int rtn, data_size, i, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	cmd[0] = SND_STX; //stx
	memcpy((char *)&cmd[1], (char *)&CALI_ADDR_1, 2); //addr
	memcpy((char *)&cmd[3], (char *)&CMD_CO1, 3); //cmd

	data_size = 6;
	cmd[6] = (data_size / 10) + 0x30;
	cmd[7] = (data_size % 10) + 0x30;

	val[0] = (unsigned char)val1;
	val[1] = (unsigned char)val2;
	val[2] = (unsigned char)val3;

	for(i=0; i < data_size; i++) {
		if((i % 2) == 0) {
			data[i] = ((val[i/2] & 0xF0) >> 4);
		} else {
			data[i] = (val[i/2] & 0x0F);
		}
		if(data[i] < 0x0A) {
			data[i] += 0x30;
		} else {
			data[i] += 0x37;
		}
	}
	memcpy((char *)&cmd[8], (char *)&data[0], data_size); //data

	cmd_size = make_check_sum((char *)&cmd, data_size + 8);

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void send_cmd_initialize_Calibrator(int comPort, int val1)
{
	char cmd[32], data[6];
	unsigned char val[3];
	int rtn, data_size, i, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	cmd[0] = SND_STX; //stx
	memcpy((char *)&cmd[1], (char *)&CALI_ADDR_1, 2); //addr
	memcpy((char *)&cmd[3], (char *)&CMD_INI, 3); //cmd

	data_size = 2;
	cmd[6] = (data_size / 10) + 0x30;
	cmd[7] = (data_size % 10) + 0x30;

	val[0] = (unsigned char)val1;

	for(i=0; i < data_size; i++) {
		if((i % 2) == 0) {
			data[i] = ((val[i/2] & 0xF0) >> 4);
		} else {
			data[i] = (val[i/2] & 0x0F);
		}
		if(data[i] < 0x0A) {
			data[i] += 0x30;
		} else {
			data[i] += 0x37;
		}
	}
	memcpy((char *)&cmd[8], (char *)&data[0], data_size); //data

	cmd_size = make_check_sum((char *)&cmd, data_size + 8);

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void send_cmd_request_Calibrator(int comPort)
{
	char cmd[32];
	int rtn, data_size, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	cmd[0] = SND_STX; //stx
	memcpy((char *)&cmd[1], (char *)&CALI_ADDR_1, 2); //addr
	memcpy((char *)&cmd[3], (char *)&CMD_REQ, 3); //cmd

	data_size = 0;
	cmd[6] = (data_size / 10) + 0x30;
	cmd[7] = (data_size % 10) + 0x30;

	cmd_size = make_check_sum((char *)&cmd, data_size + 8);

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void COM_Signal_Check_MS_860_RS_485(int comPort)
{
	int	i, group, toPs;
	long wait_time, wait_time_limit;
	S_MSG_VAL SendMsg;

	wait_time_limit = 1000; //1sec
	group = myCom->signal[COM_SIG_BCR_COMM_GROUP];

	switch(myCom->signal[COM_SIG_BCR_REQUEST_PHASE]) {
		case P0:
			for(i=0; i < MAX_GROUP_4; i++) {
				if(myCom->misc.bcr_reservation[i] != P0) {
					myCom->misc.bcr_reservation[i] = P0;
					myCom->signal[COM_SIG_BCR_COMM_GROUP] = i;
					myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P1;
					myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
					myCom->signal[COM_SIG_BCR_READ_COUNT] = P0;
					break;
				}
			}
			break;
		case P1:
			send_cmd_request_MS_860_RS_485(comPort, group, P0);
			myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			break;
		case P2:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				myCom->signal[COM_SIG_BCR_RETRY_COUNT]++;
				if(myCom->signal[COM_SIG_BCR_RETRY_COUNT] >= 3) {
					myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
					myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P0; //kjg_w
				} else {
					myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P1;
				}
			}
			break;
		case P3:
			//send_cmd_request_MS_860_RS_485(comPort, group, P1);
			//myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
			//myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P5;
			break;
		case P4:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				myCom->signal[COM_SIG_BCR_RETRY_COUNT]++;
				if(myCom->signal[COM_SIG_BCR_RETRY_COUNT] >= 3) {
					myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
					myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P0; //kjg_w
				} else {
					myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P1;
				}
			}
			break;
		case P5:
			send_cmd_request_MS_860_RS_485(comPort, group, P2);
			myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			break;
		case P6:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P7:
			send_cmd_request_MS_860_RS_485(comPort, group, P3);
			myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			break;
		case P8:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				myCom->signal[COM_SIG_BCR_RETRY_COUNT]++;
				if(myCom->signal[COM_SIG_BCR_RETRY_COUNT] >= 3) {
					myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
					myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P0; //kjg_w
				} else {
					myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
					myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P6;
				}
			}
			break;
		case P9:
			send_cmd_request_MS_860_RS_485(comPort, group, P4);

			myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;

			myCom->signal[COM_SIG_BCR_READ_COUNT]++;
			if(myCom->signal[COM_SIG_BCR_READ_COUNT] >= 3) {
				//myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P0;

				memcpy((char *)&myCom->misc.tray_id[group][0],
					(char *)&myCom->misc.tmp_tray_id[2][0], COM_BCR_SIZE);
				memcpy((char *)&myData->gData[group].tray_id[0],
					(char *)&myCom->misc.tmp_tray_id[2][0], BCR_SIZE);

				memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
				toPs = MODULE_TO_COB1 + group;
				SendMsg.msg = MSG_MODULE_COB_TRAY_ID;
				SendMsg.val[0] = group;
				send_msg(toPs, (char *)&SendMsg);
			} else {
				myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P11;
			}
			break;
		case P10:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				myCom->signal[COM_SIG_BCR_RETRY_COUNT]++;
				if(myCom->signal[COM_SIG_BCR_RETRY_COUNT] >= 3) {
					myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
					myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P0; //kjg_w
				} else {
					myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
					myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P6;
				}
			}
			break;
		case P11:
			myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P1;
			break;
		default: break;
	}
}

int rcv_cmd_answer_MS_860_RS_485(int comPort)
{
	int i, count, start_point;

	switch(myCom->signal[COM_SIG_BCR_REQUEST_PHASE]) {
		case P2:
			if(myCom->rcvCmd.cmd[0] == 0x1D && myCom->rcvCmd.cmd[1] == S_ACK) {
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P3;
			} else if(myCom->rcvCmd.cmd[0] == 0x1F
				&& myCom->rcvCmd.cmd[1] == S_ACK) {
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P3;
			}
			break;
		case P4:
			if(myCom->rcvCmd.cmd[0] == 0x1D && myCom->rcvCmd.cmd[1] == S_ACK) {
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P5;
			} else if(myCom->rcvCmd.cmd[0] == 0x1F
				&& myCom->rcvCmd.cmd[1] == S_ACK) {
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P5;
			}
			break;
		case P8:
			count = 0;
			if(myCom->rcvCmd.cmd[0] == 0x1C && myCom->rcvCmd.cmd[1] == S_STX) {
				count = 1;
			} else if(myCom->rcvCmd.cmd[0] == 0x1E
				&& myCom->rcvCmd.cmd[1] == S_STX) {
				count = 2;
			}
			if(count == 0) break;

			start_point = 2;
			count = 0;
			for(i=2; i < (COM_BCR_SIZE+2); i++) { //tray_id size
				if(i == 2 && myCom->rcvCmd.cmd[i] == 0x0D) {
					start_point++;
					continue;
				} else if(myCom->rcvCmd.cmd[i] != S_CR) {
					count++;
				} else break;
			}
	
			i = myCom->signal[COM_SIG_BCR_READ_COUNT];
			memset((char *)&myCom->misc.tmp_tray_id[i][0], 0,
				sizeof(char) * COM_BCR_SIZE);
			memcpy((char *)&myCom->misc.tmp_tray_id[i][0], 
				(char *)&myCom->rcvCmd.cmd[start_point], count);

			myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P9;
			break;
		case P10:
			if(myCom->rcvCmd.cmd[0] == S_RES) {
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P0;
			}
			break;
		default:	break;
	}

	return 0;
}

void send_cmd_request_MS_860_RS_485(int comPort, unsigned char ch, unsigned char type)
{
	char cmd[8];
	unsigned char	addr;
	int rtn, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	switch(type) {
		case P0:
			cmd_size = 3;
			*cmd = S_RES;
			addr = convert_addr_MS_860_RS_485(ch, type);
			*(cmd+1) = addr;
			*(cmd+2) = S_REQ;
			break;
		case P1:
			cmd_size = 3;
			*cmd = S_STX;
			addr = convert_addr_MS_860_RS_485(ch, type);
			*(cmd+1) = addr;
			*(cmd+2) = S_ETX;
			break;
		case P2:
			cmd_size = 1;
			*cmd = S_RES;
			break;
		case P3:
			cmd_size = 3;
			*cmd = S_RES;
			addr = convert_addr_MS_860_RS_485(ch, type);
			*(cmd+1) = addr;
			*(cmd+2) = S_REQ;
			break;
		case P4:
			cmd_size = 1;
			*cmd = S_ACK;
			break;
		default:	return;
	}

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

unsigned char convert_addr_MS_860_RS_485(unsigned char ch, unsigned char type)
{
	unsigned char addr=0;

	if(type <= P2) {
		addr = BCR1_SELECT_1 + 0x02 * ch;
	} else {
		addr = BCR1_POLLING_1 + 0x02 * ch;
	}

	return addr;
}

void send_cmd_ack_MS_860_RS_485(int comPort, unsigned char ch)
{
	char cmd[8];
	int rtn, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	cmd_size = 1;
	*cmd = S_ACK;

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void COM_Signal_Check_MS_860_RS_232(int comPort)
{
	int	i, group, toPs;
	long wait_time, wait_time_limit, time_l2;
	S_MSG_VAL SendMsg;

	wait_time_limit = 500; //0.5sec
	time_l2 = 100; //100ms
	group = myCom->signal[COM_SIG_BCR_COMM_GROUP];

	switch(myCom->signal[COM_SIG_BCR_REQUEST_PHASE]) {
		case P0:
			for(i=0; i < MAX_GROUP_4; i++) {
				if(myCom->misc.bcr_reservation[i] == P1) {
					myCom->misc.bcr_reservation[i] = P0;
					myCom->signal[COM_SIG_BCR_COMM_GROUP] = i;
					myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P1;
					myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
					myCom->signal[COM_SIG_BCR_READ_COUNT] = P0;
					break;
				} else if(myCom->misc.bcr_reservation[i] == P11) {
					myCom->misc.bcr_reservation[i] = P0;
					myCom->signal[COM_SIG_BCR_COMM_GROUP] = i;
					myCom->signal[COM_SIG_BCR_REQUEST_PHASE] =P21;
					myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
					myCom->signal[COM_SIG_BCR_READ_COUNT] = P0;
					break;
				}
			}
			break;
		case P1:
			send_cmd_initialize_MS_860_RS_232(comPort, group, P10);
			myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			break;
		case P2:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P11);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P3:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P12);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P4:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P13);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P5:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit * 2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P10);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P6:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P11);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P7:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P12);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P8:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P13);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P9:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit * 2) {
				send_cmd_request_MS_860_RS_232(comPort, group);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P10:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit * 2) {
				send_cmd_request_MS_860_RS_232(comPort, group);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P12:
			memcpy((char *)&myCom->misc.tray_id[group][0],
				(char *)&myCom->misc.tmp_tray_id[0][0], COM_BCR_SIZE);
			memcpy((char *)&myData->gData[group].tray_id[0],
				(char *)&myCom->misc.tmp_tray_id[0][0], BCR_SIZE);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			toPs = MODULE_TO_COB1 + group;
			SendMsg.msg = MSG_MODULE_COB_TRAY_ID;
			SendMsg.val[0] = group;
			send_msg(toPs, (char *)&SendMsg);

			myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P21;
			break;
		case P21:
			send_cmd_initialize_MS_860_RS_232(comPort, group, P0);
			myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			break;
		case P22:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P1);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P23:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P2);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P24:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P3);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P25:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P4);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P26:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P0);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P27:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P1);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P28:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P2);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P29:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P3);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P30:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= time_l2) {
				send_cmd_initialize_MS_860_RS_232(comPort, group, P4);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P31:
			myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P0;
			break;
		default: break;
	}
}

int rcv_cmd_answer_MS_860_RS_232(int comPort)
{
	int i, count, start_point;

	switch(myCom->signal[COM_SIG_BCR_REQUEST_PHASE]) {
		case P11:
			start_point = 0;
			count = 0;
			for(i=0; i < COM_BCR_SIZE; i++) { //tray_id size
				if(myCom->rcvCmd.cmd[i] != S_CR) {
					count++;
				} else break;
			}
	
			i = myCom->signal[COM_SIG_BCR_READ_COUNT];
			memset((char *)&myCom->misc.tmp_tray_id[i][0], 0,
				sizeof(char) * COM_BCR_SIZE);
			memcpy((char *)&myCom->misc.tmp_tray_id[i][0], 
				(char *)&myCom->rcvCmd.cmd[start_point], count);

			myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P12;
			break;
		default:	break;
	}

	return 0;
}

void send_cmd_initialize_MS_860_RS_232(int comPort, int group, int type)
{
	char cmd[32];
	int rtn, cmd_size, index;

	memset((char *)&cmd, 0, sizeof cmd);

	cmd_size = 0;
	index = 0;
	*(cmd+index) = 0x3C; //<
	
	switch(type) {
		case 0: //Trigger (Serial Data)
			index++;
			*(cmd+index) = 0x4B; //K
			index++;
			*(cmd+index) = 0x32; //2
			index++;
			*(cmd+index) = 0x30; //0
			index++;
			*(cmd+index) = 0x30; //0
			index++;
			*(cmd+index) = 0x2C; //,
			index++;
			*(cmd+index) = 0x34; //4
			break;
		case 1: //Raster OFF
			index++;
			*(cmd+index) = 0x4B; //K
			index++;
			*(cmd+index) = 0x35; //5
			index++;
			*(cmd+index) = 0x30; //0
			index++;
			*(cmd+index) = 0x36; //6
			index++;
			*(cmd+index) = 0x2C; //,
			index++;
			*(cmd+index) = 0x30; //0
			break;
		case 2: //Laser OFF
			index++;
			*(cmd+index) = 0x4B; //K
			index++;
			*(cmd+index) = 0x37; //7
			index++;
			*(cmd+index) = 0x30; //0
			index++;
			*(cmd+index) = 0x30; //0
			index++;
			*(cmd+index) = 0x2C; //,
			index++;
			*(cmd+index) = 0x31; //1
			break;
		case 3: //Motor OFF
			index++;
			*(cmd+index) = 0x4B; //K
			index++;
			*(cmd+index) = 0x46; //F
			break;
		case 4: //Code128 Enable
			index++;
			*(cmd+index) = 0x4B; //K
			index++;
			*(cmd+index) = 0x34; //4
			index++;
			*(cmd+index) = 0x37; //7
			index++;
			*(cmd+index) = 0x34; //4
			index++;
			*(cmd+index) = 0x2C; //,
			index++;
			*(cmd+index) = 0x31; //1
			break;
		case 10: //Motor ON
			index++;
			*(cmd+index) = 0x4B; //K
			index++;
			*(cmd+index) = 0x45; //E
			break;
		case 11: //Laser ON
			index++;
			*(cmd+index) = 0x4B; //K
			index++;
			*(cmd+index) = 0x37; //7
			index++;
			*(cmd+index) = 0x30; //0
			index++;
			*(cmd+index) = 0x30; //0
			index++;
			*(cmd+index) = 0x2C; //,
			index++;
			*(cmd+index) = 0x30; //0
			break;
		case 12: //Raster ON
			index++;
			*(cmd+index) = 0x4B; //K
			index++;
			*(cmd+index) = 0x35; //5
			index++;
			*(cmd+index) = 0x30; //0
			index++;
			*(cmd+index) = 0x36; //6
			index++;
			*(cmd+index) = 0x2C; //,
			index++;
			*(cmd+index) = 0x31; //1
			break;
		case 13: //Code128 Enable
			index++;
			*(cmd+index) = 0x4B; //K
			index++;
			*(cmd+index) = 0x34; //4
			index++;
			*(cmd+index) = 0x37; //7
			index++;
			*(cmd+index) = 0x34; //4
			index++;
			*(cmd+index) = 0x2C; //,
			index++;
			*(cmd+index) = 0x31; //1
			break;
		default: break;
	}

	index++;
	*(cmd+index) = 0x3E; //>
	index++;
	*(cmd+index) = 0x0D; //CR
	index++;
	*(cmd+index) = 0x0A; //LF
	cmd_size = index + 1;

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void send_cmd_request_MS_860_RS_232(int comPort, int group)
{
	char cmd[32];
	int rtn, cmd_size, index;

	memset((char *)&cmd, 0, sizeof cmd);

	cmd_size = 0;
	index = 0;
	*(cmd+index) = 0x3C; //<
	
	index++;
	*(cmd+index) = 0x1D; //GS

	index++;
	*(cmd+index) = 0x3E; //>
	index++;
	*(cmd+index) = 0x0D; //CR
	index++;
	*(cmd+index) = 0x0A; //LF
	cmd_size = index + 1;

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void COM_Signal_Check_COMM_B_MS_860(int comPort)
{
	int	i, group, toPs;
	long wait_time, wait_time_limit, time_l2;
	S_MSG_VAL SendMsg;

	wait_time_limit = 1000; //1sec
	time_l2 = 500; //500ms
	group = myCom->signal[COM_SIG_BCR_COMM_GROUP];

	switch(myCom->signal[COM_SIG_BCR_REQUEST_PHASE]) {
		case P0:
			for(i=0; i < MAX_GROUP_4; i++) {
				if(myCom->misc.bcr_reservation[i] == P1) {
					myCom->misc.bcr_reservation[i] = P0;
					myCom->signal[COM_SIG_BCR_COMM_GROUP] = i;
					myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P1;
					myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
					myCom->signal[COM_SIG_BCR_READ_COUNT] = P0;
					break;
				} else if(myCom->misc.bcr_reservation[i] == P11) {
					myCom->misc.bcr_reservation[i] = P0;
					myCom->signal[COM_SIG_BCR_COMM_GROUP] = i;
					myCom->signal[COM_SIG_BCR_REQUEST_PHASE] =P21;
					myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
					myCom->signal[COM_SIG_BCR_READ_COUNT] = P0;
					break;
				}
			}
			break;
		case P1:
			send_cmd_initialize_COMM_B_MS_860(comPort, group, P1);
			myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			break;
		case P2:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				send_cmd_initialize_COMM_B_MS_860(comPort, group, P1);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P3:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit * 2) {
				memset((char *)&myCom->misc.tmp_tray_id[group][0], 0,
					sizeof(char) * COM_BCR_SIZE);
				send_cmd_request_COMM_B_MS_860(comPort, group);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P4:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit * 2) {
				send_cmd_request_COMM_B_MS_860(comPort, group);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P5:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit * 5) {
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P6:
			memcpy((char *)&myCom->misc.tray_id[group][0],
				(char *)&myCom->misc.tmp_tray_id[0][0], COM_BCR_SIZE);
			memcpy((char *)&myData->gData[group].tray_id[0],
				(char *)&myCom->misc.tmp_tray_id[0][0], BCR_SIZE);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			toPs = MODULE_TO_COB1 + group;
			SendMsg.msg = MSG_MODULE_COB_TRAY_ID;
			SendMsg.val[0] = group;
			send_msg(toPs, (char *)&SendMsg);

			myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P21;
			break;
		case P21:
			send_cmd_initialize_COMM_B_MS_860(comPort, group, P0);
			myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			break;
		case P22:
			myCom->signal[COM_SIG_BCR_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_BCR_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				send_cmd_initialize_COMM_B_MS_860(comPort, group, P0);
				myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_BCR_REQUEST_PHASE]++;
			}
			break;
		case P23:
			myCom->signal[COM_SIG_BCR_RETRY_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_WAIT_COUNT] = P0;
			myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P0;
			break;
		default: break;
	}
}

int CmdHeader_Check_COMM_B_MS_860(int comPort)
{
	int length;
	S_COM_COMM_B_CMD_HEADER header;

	length = sizeof(S_COM_COMM_B_CMD_HEADER);
	memcpy((char *)&header, (char *)&myCom->rcvCmd.cmd, length);

	if(length >= myCom->rcvCmd.cmdSize) {
		rtl_printf("com:%d RcvCmd size error length:%d rcv:%d\n",
			comPort, length, myCom->rcvCmd.cmdSize);
		return -1;
	}

	if(header.stx != '!' && header.stx != '?' && header.stx != '>') {
		rtl_printf("com:%d RcvCmd stx error 0x%x\n", comPort, header.stx);
		return -2;
	}

	if(header.stx == '!') {
		if((header.addr[0] == '8' && header.addr[1] == '0')
			|| (header.addr[0] == '8' && header.addr[1] == '1')) {
		} else {
			rtl_printf("com:%d RcvCmd addr error 0x%02x 0x%02x\n", comPort,
				(unsigned char)header.addr[0], (unsigned char)header.addr[1]);
			return -3;
		}
	}

	return 0;
}

int CheckSum_Check_COMM_B_MS_860(int comPort)
{
	char buf[64];
	int bcc, i, cmd_size, rcv_bcc;

	cmd_size = myCom->rcvCmd.cmdSize;

	memset((char *)&buf, 0, sizeof buf);
	memcpy((char *)&buf, (char *)&myCom->rcvCmd.cmd, cmd_size);

	if(buf[0] != '!') {
		return -1;
	}

	bcc = 0;
	for(i=0; i < (cmd_size - 5); i++) {
		bcc += buf[i];
	}

	rcv_bcc = (buf[cmd_size - 5] - 0x30) * 1000
		+ (buf[cmd_size - 4] - 0x30) * 100
		+ (buf[cmd_size - 3] - 0x30) * 10
		+ (buf[cmd_size - 2] - 0x30);

	if(bcc == rcv_bcc) {
		return 0;
	} else {
		rtl_printf("com:%d RcvCmd CheckSum error (%04d:%04d)\n", comPort,
			bcc, rcv_bcc);
		return -2;
	}
}

int rcv_cmd_answer_COMM_B_MS_860(int comPort)
{
	int i, count, start_point;

	switch(myCom->signal[COM_SIG_BCR_REQUEST_PHASE]) {
		case P5:
			start_point = 8;
			count = (int)(myCom->rcvCmd.cmd[6] - 0x30) * 10;
			count += (int)(myCom->rcvCmd.cmd[7] - 0x30);

			i = myCom->signal[COM_SIG_BCR_READ_COUNT];
			memset((char *)&myCom->misc.tmp_tray_id[i][0], 0,
				sizeof(char) * COM_BCR_SIZE);
			memcpy((char *)&myCom->misc.tmp_tray_id[i][0], 
				(char *)&myCom->rcvCmd.cmd[start_point], count);

			myCom->signal[COM_SIG_BCR_REQUEST_PHASE] = P6;
			break;
		default:	break;
	}

	return 0;
}

void send_cmd_initialize_COMM_B_MS_860(int comPort, int group, int type)
{
	char cmd[32];
	int rtn, data_size, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	data_size = 10;
	*cmd = 0x40;
	*(cmd+1) = 0x38;
	*(cmd+2) = 0x30 + group;

	*(cmd+3) = 0x49; //I
	*(cmd+4) = 0x4E; //N
	*(cmd+5) = 0x49; //I

	*(cmd+6) = 0x30;
	*(cmd+7) = 0x32;

	*(cmd+8) = 0x30;
	*(cmd+9) = 0x30 + type;

	cmd_size = make_check_sum((char *)&cmd, data_size);

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void send_cmd_request_COMM_B_MS_860(int comPort, int group)
{
	char cmd[32];
	int rtn, data_size, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	data_size = 8;
	*cmd = 0x40;
	*(cmd+1) = 0x38;
	*(cmd+2) = 0x30 + group;

	*(cmd+3) = 0x52; //R
	*(cmd+4) = 0x45; //E
	*(cmd+5) = 0x51; //Q

	*(cmd+6) = 0x30;
	*(cmd+7) = 0x30;

	cmd_size = make_check_sum((char *)&cmd, data_size);

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void COM_Signal_Check_PLC_Q03UD(int comPort)
{
	long wait_time, wait_time_limit, comm_period;

	comm_period = 500; //0.5sec

	switch(myCom->signal[COM_SIG_PLC_COMM]) {
		case P0:
			break;
		case P1:
			myCom->signal[COM_SIG_PLC_COMM_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_PLC_COMM_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= comm_period) {
				myCom->signal[COM_SIG_PLC_REQUEST_PHASE] = P1;
				myCom->signal[COM_SIG_PLC_COMM_COUNT] = P0;
				myCom->signal[COM_SIG_PLC_COMM] = P2;
			} else {
				if(myCom->signal[COM_SIG_PLC_DOOR_OPEN] == P1) {
					myCom->signal[COM_SIG_PLC_DOOR_OPEN] = P0;
					send_cmd_request_PLC_Q03UD(JW, PLC_INIT, 1, comPort);
					send_cmd_request_PLC_Q03UD(JW, PLC_DOOR_OPEN, 1, comPort);
				} else if(myCom->signal[COM_SIG_PLC_DOOR_CLOSE] == P1) {
					myCom->signal[COM_SIG_PLC_DOOR_CLOSE] = P0;
					send_cmd_request_PLC_Q03UD(JW, PLC_INIT, 1, comPort);
					send_cmd_request_PLC_Q03UD(JW, PLC_DOOR_CLOSE, 1, comPort);
				} else if(myCom->signal[COM_SIG_PLC_INIT_SEND] == P1) {
					myCom->signal[COM_SIG_PLC_INIT_SEND] = P0;
					send_cmd_request_PLC_Q03UD(JW, PLC_INIT, 1, comPort);
				}
			}
			break;
		case P2:
			break;
		default: break;
	}

	wait_time_limit = 300; //300ms

	switch(myCom->signal[COM_SIG_PLC_REQUEST_PHASE]) {
		case P0:
			break;
		case P1:
			myCom->signal[COM_SIG_PLC_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_PLC_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				send_cmd_request_PLC_Q03UD(QR, PLC_FAULT_READ, 1, comPort);
				myCom->signal[COM_SIG_PLC_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_PLC_REQUEST_PHASE] = P2;
			}
			break;
		case P2:
			break;
		default: break;
	}
}

int CmdHeader_Check_PLC_Q03UD(int comPort)
{
	int length;
	S_COM_PLC_Q03UD_CMD_HEADER header;

	length = sizeof(S_COM_PLC_Q03UD_CMD_HEADER);
	memcpy((char *)&header, (char *)&myCom->rcvCmd.cmd, length);

	if(length > myCom->rcvCmd.cmdSize) {
		rtl_printf("com:%d RcvCmd size error length:%d rcv:%d\n",
			comPort, length, myCom->rcvCmd.cmdSize);
		return -1;
	}

	if(header.stx != PLC_ACK
		&& header.stx != PLC_NACK
		&& header.stx != STX) {
		rtl_printf("com:%d RcvCmd stx error 0x%x\n", comPort, header.stx);
		return -2;
	}

	if(header.stx == PLC_ACK
		|| header.stx == PLC_NACK
		|| header.stx == STX) {
		if(header.plcNo[0] == 'F' && header.plcNo[1] == 'F') {
		} else {
			rtl_printf("com:%d RcvCmd plcNo error 0x%02x 0x%02x\n", comPort,
				(unsigned char)header.plcNo[0], (unsigned char)header.plcNo[1]);
			return -3;
		}
	}

	return 0;
}

int CheckSum_Check_PLC_Q03UD(int comPort)
{
	char buf[64];
	int i, cmd_size;
	unsigned char	bcc, rcv_bcc;

	cmd_size = myCom->rcvCmd.cmdSize;

	memset((char *)&buf, 0, sizeof buf);
	memcpy((char *)&buf, (char *)&myCom->rcvCmd.cmd, cmd_size+2);

	if(buf[0] != PLC_ACK
		|| buf[0] != STX) {
		return -1;
	}

	bcc = 0;
	for(i=1; i < cmd_size; i++) {
		bcc += buf[i];
	}

	if((buf[cmd_size+1]) >= 0x31 && buf[cmd_size+1] <= 0x39){
		rcv_bcc = (buf[cmd_size+1]-0x30) << 4;
	}else if((buf[cmd_size+1]) >= 0x41 && buf[cmd_size+1] <= 0x46){
		rcv_bcc = (buf[cmd_size+1]-0x37) << 4;
	}

	if((buf[cmd_size+2]) >= 0x31 && buf[cmd_size+2] <= 0x39){
		rcv_bcc += ((buf[cmd_size+2]-0x30) << 4);
	}else if((buf[cmd_size+2]) >= 0x41 && buf[cmd_size+2] <= 0x46){
		rcv_bcc += ((buf[cmd_size+2]-0x37) << 4);
	}

	if(bcc == rcv_bcc) {
		return 0;
	} else {
		rtl_printf("com:%d RcvCmd CheckSum error (%04d:%04d)\n", comPort,
			bcc, rcv_bcc);
		return -2;
	}
}

int rcv_cmd_answer_PLC_Q03UD(int comPort)
{
	switch(myCom->signal[COM_SIG_PLC_REQUEST_PHASE]) {
		case P2:
			send_cmd_response_PLC_Q03UD(0, comPort);
			if(myPs->misc.timer_1sec > 5) {
				PLC_Fault_Check(comPort);
			}
			myCom->signal[COM_SIG_PLC_REQUEST_PHASE] = P0;
			myCom->signal[COM_SIG_PLC_COMM] = P1;
			break;
		default:	break;
	}

	return 0;
}

void PLC_Fault_Check(int comPort)
{
	char cmd[32], temp;
	int size;
	S_MSG_VAL SendMsg;

	size = sizeof(S_COM_PLC_Q03UD_CMD_HEADER);
	memset((char *)&cmd, 0, 32);
	memcpy((char *)&cmd, (char *)&myCom->rcvCmd.cmd[size],4);

	temp = cmd[0];
	if(temp >= '1' && temp <='9'){
		temp -=0x30;
	}else{
		temp -=0x37;
	}
	if((temp & PLC_ALARM_AREA_SENSOR) != 0){
		if(myCom->signal[COM_SIG_PLC_ALARM_AREA_SENSOR] == P0){
			myCom->signal[COM_SIG_PLC_ALARM_AREA_SENSOR] = P1;
			SendMsg.msg = MSG_MODULE_JIGM_PLC_ALARM_AREA_SENSOR;
			SendMsg.val[0] = 0;
			send_msg(MODULE_TO_JIGM,(char *)&SendMsg); 
		}
	}else{
		if(myCom->signal[COM_SIG_PLC_ALARM_AREA_SENSOR] == P1){
			myCom->signal[COM_SIG_PLC_ALARM_AREA_SENSOR] = P0;
		}
	}
	if((temp & PLC_ALARM_MANUAL_DOOR_OPEN) != 0){
		if(myCom->signal[COM_SIG_PLC_ALARM_MANUAL_DOOR_OPEN] == P1){
			myCom->signal[COM_SIG_PLC_ALARM_MANUAL_DOOR_OPEN] = P0;
		}
	}else{
		if(myCom->signal[COM_SIG_PLC_ALARM_MANUAL_DOOR_OPEN] == P0){
			myCom->signal[COM_SIG_PLC_ALARM_MANUAL_DOOR_OPEN] = P1;
			SendMsg.msg = MSG_MODULE_JIGM_PLC_ALARM_MANUAL_DOOR_OPEN;
			SendMsg.val[0] = 0;
			send_msg(MODULE_TO_JIGM,(char *)&SendMsg); 
		}
	}

	temp = cmd[1];
	if(temp >= '1' && temp <='9'){
		temp -=0x30;
	}else{
		temp -=0x37;
	}
	if((temp & PLC_ALARM_CHAMBER_RUN) != 0){
		if(myCom->signal[COM_SIG_PLC_ALARM_CHAMBER_RUN] == P0){
			myCom->signal[COM_SIG_PLC_ALARM_CHAMBER_RUN] = P1;
		}
	}else{
		if(myCom->signal[COM_SIG_PLC_ALARM_CHAMBER_RUN] == P1){
			myCom->signal[COM_SIG_PLC_ALARM_CHAMBER_RUN] = P0;
		}
	}
	if((temp & PLC_ALARM_DOOR_CLOSE) != 0){
		if(myCom->signal[COM_SIG_PLC_ALARM_DOOR_CLOSE] == P0){
			myCom->signal[COM_SIG_PLC_ALARM_DOOR_CLOSE] = P1;
			SendMsg.msg = MSG_MODULE_JIGM_DOOR_CLOSE;
			SendMsg.val[0] = 0;
			send_msg(MODULE_TO_JIGM,(char *)&SendMsg); 
		}
	}else{
		if(myCom->signal[COM_SIG_PLC_ALARM_DOOR_CLOSE] == P1){
			myCom->signal[COM_SIG_PLC_ALARM_DOOR_CLOSE] = P0;
		}
	}
	if((temp & PLC_ALARM_DOOR_OPEN) != 0){
		if(myCom->signal[COM_SIG_PLC_ALARM_DOOR_OPEN] == P0){
			myCom->signal[COM_SIG_PLC_ALARM_DOOR_OPEN] = P1;
			SendMsg.msg = MSG_MODULE_JIGM_DOOR_OPEN;
			SendMsg.val[0] = 0;
			send_msg(MODULE_TO_JIGM,(char *)&SendMsg); 
		}
	}else{
		if(myCom->signal[COM_SIG_PLC_ALARM_DOOR_OPEN] == P1){
			myCom->signal[COM_SIG_PLC_ALARM_DOOR_OPEN] = P0;
		}
	}
	if((temp & PLC_ALARM_AIR) != 0){
		if(myCom->signal[COM_SIG_PLC_ALARM_AIR] == P0){
			myCom->signal[COM_SIG_PLC_ALARM_AIR] = P1;
			SendMsg.msg = MSG_MODULE_JIGM_PLC_ALARM_AIR;
			SendMsg.val[0] = 0;
			send_msg(MODULE_TO_JIGM,(char *)&SendMsg); 
		}
	}else{
		if(myCom->signal[COM_SIG_PLC_ALARM_AIR] == P1){
			myCom->signal[COM_SIG_PLC_ALARM_AIR] = P0;
		}
	}

	temp = cmd[2];
	if(temp >= '1' && temp <='9'){
		temp -=0x30;
	}else{
		temp -=0x37;
	}
	if((temp & PLC_ALARM_APR_S) != 0){
		if(myCom->signal[COM_SIG_PLC_ALARM_APR_S] == P1){
			myCom->signal[COM_SIG_PLC_ALARM_APR_S] = P0;
		}
	}else{
		if(myCom->signal[COM_SIG_PLC_ALARM_APR_S] == P0){
			myCom->signal[COM_SIG_PLC_ALARM_APR_S] = P1;
			SendMsg.msg = MSG_MODULE_JIGM_PLC_ALARM_APR_S;
			SendMsg.val[0] = 0;
			send_msg(MODULE_TO_JIGM,(char *)&SendMsg); 
		}
	}
	if((temp & PLC_ALARM_REF) != 0){
		if(myCom->signal[COM_SIG_PLC_ALARM_REF] == P0){
			myCom->signal[COM_SIG_PLC_ALARM_REF] = P1;
			SendMsg.msg = MSG_MODULE_JIGM_PLC_ALARM_REF;
			SendMsg.val[0] = 0;
			send_msg(MODULE_TO_JIGM,(char *)&SendMsg); 
		}
	}else{
		if(myCom->signal[COM_SIG_PLC_ALARM_REF] == P1){
			myCom->signal[COM_SIG_PLC_ALARM_REF] = P0;
		}
	}
	if((temp & PLC_ALARM_COIL_TRIP) != 0){
		if(myCom->signal[COM_SIG_PLC_ALARM_COIL_TRIP] == P0){
			myCom->signal[COM_SIG_PLC_ALARM_COIL_TRIP] = P1;
			SendMsg.msg = MSG_MODULE_JIGM_PLC_ALARM_COIL_TRIP;
			SendMsg.val[0] = 0;
			send_msg(MODULE_TO_JIGM,(char *)&SendMsg); 
		}
	}else{
		if(myCom->signal[COM_SIG_PLC_ALARM_COIL_TRIP] == P1){
			myCom->signal[COM_SIG_PLC_ALARM_COIL_TRIP] = P0;
		}
	}
	if((temp & PLC_ALARM_TEMP) != 0){
		if(myCom->signal[COM_SIG_PLC_ALARM_TEMP] == P1){
			myCom->signal[COM_SIG_PLC_ALARM_TEMP] = P0;
		}
	}else{
		if(myCom->signal[COM_SIG_PLC_ALARM_TEMP] == P0){
			myCom->signal[COM_SIG_PLC_ALARM_TEMP] = P1;
			SendMsg.msg = MSG_MODULE_JIGM_PLC_ALARM_TEMP;
			SendMsg.val[0] = 0;
			send_msg(MODULE_TO_JIGM,(char *)&SendMsg); 
		}
	}

	temp = cmd[3];
	if(temp >= '1' && temp <='9'){
		temp -=0x30;
	}else{
		temp -=0x37;
	}
	if((temp & PLC_ALARM_SMOKE) != 0){
		if(myCom->signal[COM_SIG_PLC_ALARM_SMOKE] == P1){
			myCom->signal[COM_SIG_PLC_ALARM_SMOKE] = P0;
		}
	}else{
		if(myCom->signal[COM_SIG_PLC_ALARM_SMOKE] == P0){
			myCom->signal[COM_SIG_PLC_ALARM_SMOKE] = P1;
			SendMsg.msg = MSG_MODULE_JIGM_PLC_ALARM_SMOKE;
			SendMsg.val[0] = 0;
			send_msg(MODULE_TO_JIGM,(char *)&SendMsg); 
		}
	}
	if((temp & PLC_ALARM_FIRE) != 0){
		if(myCom->signal[COM_SIG_PLC_ALARM_FIRE] == P0){
			myCom->signal[COM_SIG_PLC_ALARM_FIRE] = P1;
			SendMsg.msg = MSG_MODULE_JIGM_PLC_ALARM_FIRE;
			SendMsg.val[0] = 0;
			send_msg(MODULE_TO_JIGM,(char *)&SendMsg); 
		}
	}else{
		if(myCom->signal[COM_SIG_PLC_ALARM_FIRE] == P1){
			myCom->signal[COM_SIG_PLC_ALARM_FIRE] = P0;
		}
	}
	if((temp & PLC_ALARM_EMG) != 0){
		if(myCom->signal[COM_SIG_PLC_ALARM_EMG] == P0){
			myCom->signal[COM_SIG_PLC_ALARM_EMG] = P1;
			SendMsg.msg = MSG_MODULE_JIGM_PLC_ALARM_EMG;
			SendMsg.val[0] = 0;
			send_msg(MODULE_TO_JIGM,(char *)&SendMsg); 
		}
	}else{
		if(myCom->signal[COM_SIG_PLC_ALARM_EMG] == P1){
			myCom->signal[COM_SIG_PLC_ALARM_EMG] = P0;
		}
	}
}

void send_cmd_request_PLC_Q03UD(char *plc_cmd, char *addr, int size, int comPort)
{
	int rtn, cmd_size, data_size;
	char cmd[32];
	char len[2];
	
	sprintf(len,"%02d",size);
	
	memset((char *)&cmd, 0, 32);
	
	cmd[0] = ENQ;
  	cmd[1] = '0';
  	cmd[2] = '0';
  	cmd[3] = 'F';
  	cmd[4] = 'F';
  	cmd[5] = *plc_cmd;
  	cmd[6] = *(plc_cmd+1);
	cmd[7] = 'A'; //wait time 100mS
  	cmd[8] = *addr;
  	cmd[9] = *(addr+1);
  	cmd[10] = *(addr+2);
  	cmd[11] = *(addr+3);
  	cmd[12] = *(addr+4);
  	cmd[13] = *(addr+5);
  	cmd[14] = *(addr+6);
	cmd[15] = len[0];
	cmd[16] = len[1];
	if(!strcmp(plc_cmd,"JW")){
		cmd[17] = '1';//bit on
	}

	if(!strcmp(plc_cmd, "JW")){
		data_size = 20;
	}else{
		data_size = 19;
	}

//	cmd_size = make_check_sum_3((char *)&cmd, data_size);
	cmd_size = data_size;
	
	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}else{
		usleep(60);
		if(!strcmp(plc_cmd,"JW")){
			cmd[17] = '0';//bit off
			cmd_size = make_check_sum_3((char *)&cmd, 20);
			rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
			if(rtn < 0) {
				rtl_printf("com:%d send cmd error\n", comPort);
			}
		}
	}
}

void send_cmd_response_PLC_Q03UD(int response, int comPort)
{
	int rtn, cmd_size;
	char cmd[32];
	
	memset((char *)&cmd, 0, 32);
	
	if(response == 0){
		cmd[0] = PLC_ACK;
	}else{
		cmd[0] = PLC_NACK;
	}
  	cmd[1] = '0';
  	cmd[2] = '0';
  	cmd[3] = 'F';
  	cmd[4] = 'F';
	cmd_size = 5;
	
	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void COM_Signal_Check_TEMP_880(int comPort)
{ //debug_size_cob
/*	long wait_time, wait_time_limit, comm_period;

	comm_period = 1500; //1.5sec

	switch(myCom->signal[COM_SIG_CHAMBER_COMM]) {
		case P0:
			break;
		case P1:
			myCom->signal[COM_SIG_CHAMBER_COMM_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_CHAMBER_COMM_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= comm_period) {
				myCom->signal[COM_SIG_CHAMBER_REQUEST_PHASE] = P1;
				myCom->signal[COM_SIG_CHAMBER_COMM_COUNT] = P0;
				myCom->signal[COM_SIG_CHAMBER_COMM] = P2;
			} else {
				if(myCom->signal[COM_SIG_CHAMBER_STOP] == P1) {
					myCom->signal[COM_SIG_CHAMBER_STOP] = P0;
					myCom->signal[COM_SIG_CHAMBER_RUN] = P0;
					send_cmd_run_stop_TEMP_880(comPort, 0);
				} else if(myCom->signal[COM_SIG_CHAMBER_RUN] == P1) {
					myCom->signal[COM_SIG_CHAMBER_RUN] = P0;
					send_cmd_run_stop_TEMP_880(comPort, 1);
				} else if(myCom->signal[COM_SIG_CHAMBER_SET] == P1) {
					myCom->signal[COM_SIG_CHAMBER_SET] = P0;
					myCom->signal[COM_SIG_CHAMBER_REQUEST_PHASE] = P11;
					myCom->signal[COM_SIG_CHAMBER_COMM_COUNT] = P0;
					myCom->signal[COM_SIG_CHAMBER_COMM] = P2;
				}
			}
			break;
		case P2:
			break;
		default:	break;
	}

	wait_time_limit = 300; //300msec

	switch(myCom->signal[COM_SIG_CHAMBER_REQUEST_PHASE]) {
		case P0:
			break;
		case P1:
			myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				send_cmd_request_TEMP_880(comPort, 1);
				myCom->signal[COM_SIG_CHAMBER_RSD] = P1;
				myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_CHAMBER_REQUEST_PHASE]++;
			}
			break;
		case P2:
			myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				send_cmd_request_TEMP_880(comPort, 3);
				myCom->signal[COM_SIG_CHAMBER_RSD] = P3;
				myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_CHAMBER_REQUEST_PHASE]++;
			}
			break;
		case P3:
			myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				send_cmd_request_TEMP_880(comPort, 5);
				myCom->signal[COM_SIG_CHAMBER_RSD] = P5;
				myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_CHAMBER_REQUEST_PHASE]++;
			}
			break;
		case P4:
			myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_CHAMBER_REQUEST_PHASE] = P0;
				myCom->signal[COM_SIG_CHAMBER_COMM] = P1;
			}
			break;
		case P11:
			myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				send_cmd_temp_set_TEMP_880(comPort,
					(int)myData->COB_Client[0].chamber_set.set_temp);
				myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_CHAMBER_REQUEST_PHASE]++;
			}
			break;
		case P12:
			myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				//send_cmd_time_set_TEMP_880(comPort,
				//	(int)myData->COB_Client[0].chamber_set.set_time);
				myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_CHAMBER_REQUEST_PHASE]++;
			}
			break;
		case P13:
			myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				send_cmd_time_use_set_TEMP_880(comPort, 0);
				myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_CHAMBER_REQUEST_PHASE]++;
			}
			break;
		case P14:
			myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT]++;
			wait_time = (long)myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT];
			wait_time *= myPs->config.scan_period;
			if(wait_time >= wait_time_limit) {
				myCom->signal[COM_SIG_CHAMBER_WAIT_COUNT] = P0;
				myCom->signal[COM_SIG_CHAMBER_REQUEST_PHASE] = P0;
				myCom->signal[COM_SIG_CHAMBER_COMM] = P1;
			}
			break;
		default: break;
	}*/
}

int CmdHeader_Check_TEMP_880(int comPort)
{
	char tmp;

	if(myCom->rcvCmd.cmdSize < 5) {
		rtl_printf("com:%d RcvCmd size error %d\n",
			comPort, myCom->rcvCmd.cmdSize);
		return -1;
	}

	tmp = myCom->rcvCmd.cmd[0];
	if(tmp != S_STX) {
		rtl_printf("com:%d RcvCmd stx error 0x%x\n", comPort, tmp);
		return -2;
	}

	tmp = myCom->rcvCmd.cmd[1];
	if(tmp != 0x30) {
		rtl_printf("com:%d RcvCmd addr1 error 0x%x\n", comPort, tmp);
		return -3;
	}

	tmp = myCom->rcvCmd.cmd[2];
	if(tmp != 0x31) {
		rtl_printf("com:%d RcvCmd addr2 error 0x%x\n", comPort, tmp);
		return -4;
	}

	return 0;
}

int CheckSum_Check_TEMP_880(int comPort)
{
	char tmp, make_sum1, make_sum2, recv_sum1, recv_sum2;
	int i, sum=0;

	for(i=1; i < (myCom->rcvCmd.cmdSize - 4); i++) {
		sum += (unsigned char)(myCom->rcvCmd.cmd[i]);
	}

	tmp = (char)((sum % 0x100) / 0x10);
	if(tmp <= 9) tmp += 48;
	else tmp += 55;
	make_sum1 = tmp;

	tmp = (char)(sum % 0x10);
	if(tmp <= 9) tmp += 48;
	else tmp += 55;
	make_sum2 = tmp;

	recv_sum1 = (char)myCom->rcvCmd.cmd[myCom->rcvCmd.cmdSize - 4];
	recv_sum2 = (char)myCom->rcvCmd.cmd[myCom->rcvCmd.cmdSize - 3];

	if((recv_sum1 != make_sum1) || (recv_sum2 != make_sum2)) {
		rtl_printf(
			"com:%d RcvCmd CheckSum error recv(%02x:%02x) make(%02x:%02x)\n",
			comPort, recv_sum1, recv_sum2, make_sum1, make_sum2);
		return -1;
	}

	return 0;
}

int rcv_cmd_answer_TEMP_880(int comPort)
{
	char rcv_cmd[8], tmp;
	int type, val, index, i;

	memcpy((char *)&rcv_cmd, (char *)&myCom->rcvCmd.cmd[3], 6);

	if(rcv_cmd[0] == 'N' && rcv_cmd[1] == 'G') {
		rtl_printf("com:%d rcv_cmd_error %02x %02x\n",
			comPort, rcv_cmd[0], rcv_cmd[1]);
		return -1;
	} else if(rcv_cmd[0] == 'W' && rcv_cmd[1] == 'S' && rcv_cmd[2] == 'D') {
		if(rcv_cmd[4] == 'O' && rcv_cmd[5] == 'K') {
			/*if(myCom->signal[COM_SIG_CHAMBER_RUN] == P11) {
				myCom->signal[COM_SIG_CHAMBER_RUN] = P0;
			}
			if(myCom->signal[COM_SIG_CHAMBER_STOP] == P11) {
				myCom->signal[COM_SIG_CHAMBER_STOP] = P0;
			}
			if(myCom->signal[COM_SIG_CHAMBER_HOLD] == P11) {
				myCom->signal[COM_SIG_CHAMBER_HOLD] = P0;
			}*/
		} else {
			rtl_printf("com:%d rcv_cmd_error wsd %02x %02x\n",
				comPort, rcv_cmd[4], rcv_cmd[5]);
			return -1;
		}
	} else if(rcv_cmd[0] == 'R' && rcv_cmd[1] == 'S' && rcv_cmd[2] == 'D') {
		if(rcv_cmd[4] == 'O' && rcv_cmd[5] == 'K') {
			type = (int)myCom->signal[COM_SIG_CHAMBER_RSD];
			switch(type) {
				case 1:
					if(myCom->rcvCmd.cmdSize != 28) {
						rtl_printf("com:%d rcv_cmd_error rsd %d %d\n",
							comPort, type, myCom->rcvCmd.cmdSize);
						return -1;
					}

					val = 0;
					index = 10;
					for(i=0; i < 4; i++) {
						tmp = myCom->rcvCmd.cmd[index + i];
						if(tmp <= 57) tmp -= 48;
						else tmp -= 55;
						if(i == 0) val += (tmp * 0x1000);
						else if(i == 1) val += (tmp * 0x100);
						else if(i == 2) val += (tmp * 0x10);
						else val += tmp;
					}
					myCom->misc.TEMP_NPV = val;

					val = 0;
					index += 5;
					for(i=0; i < 4; i++) {
						tmp = myCom->rcvCmd.cmd[index + i];
						if(tmp <= 57) tmp -= 48;
						else tmp -= 55;
						if(i == 0) val += (tmp * 0x1000);
						else if(i == 1) val += (tmp * 0x100);
						else if(i == 2) val += (tmp * 0x10);
						else val += tmp;
					}
					myCom->misc.TEMP_NSP = val;
/*
					val = 0;
					index += 5;
					for(i=0; i < 4; i++) {
						tmp = myCom->rcvCmd.cmd[index + i];
						if(tmp <= 57) tmp -= 48;
						else tmp -= 55;
						if(i == 0) val += (tmp * 0x1000);
						else if(i == 1) val += (tmp * 0x100);
						else if(i == 2) val += (tmp * 0x10);
						else val += tmp;
					}
					myCom->misc.DAMP_NSP = val;*/
					break;
				case 2:
					if(myCom->rcvCmd.cmdSize != 33) {
						rtl_printf("com:%d rcv_cmd_error rsd %d %d\n",
							comPort, type, myCom->rcvCmd.cmdSize);
						return -1;
					}
					break;
				case 3:
					if(myCom->rcvCmd.cmdSize != 38) {
						rtl_printf("com:%d rcv_cmd_error rsd %d %d\n",
							comPort, type, myCom->rcvCmd.cmdSize);
						return -1;
					}

					val = 0;
					index = 10;
					for(i=0; i < 4; i++) {
						tmp = myCom->rcvCmd.cmd[index + i];
						if(tmp <= 57) tmp -= 48;
						else tmp -= 55;
						/*if(i == 0) {
							val = (int)tmp;
						} else if(i == 1) {
							val |= ((int)tmp << 4);
						} else if(i == 2) {
						} else {
						}*/
						if(i == 0) val += (tmp * 0x1000);
						else if(i == 1) val += (tmp * 0x100);
						else if(i == 2) val += (tmp * 0x10);
						else val += tmp;
					}
					myCom->misc.NOWSTS = val;

					val = 0;
					index += 5;
					for(i=0; i < 4; i++) {
						tmp = myCom->rcvCmd.cmd[index + i];
						if(tmp <= 57) tmp -= 48;
						else tmp -= 55;
						/*if(i == 0) {
							val = (int)tmp;
						} else if(i == 1) {
							val |= ((int)tmp << 4);
						} else if(i == 2) {
						} else {
						}*/
						if(i == 0) val += (tmp * 0x1000);
						else if(i == 1) val += (tmp * 0x100);
						else if(i == 2) val += (tmp * 0x10);
						else val += tmp;
					}
					myCom->misc.OTHERSTS = val;
					break;
				case 4:
					if(myCom->rcvCmd.cmdSize != 38) {
						rtl_printf("com:%d rcv_cmd_error rsd %d %d\n",
							comPort, type, myCom->rcvCmd.cmdSize);
						return -1;
					}
					break;
				case 5:
					if(myCom->rcvCmd.cmdSize != 23) {
						rtl_printf("com:%d rcv_cmd_error rsd %d %d\n",
							comPort, type, myCom->rcvCmd.cmdSize);
						return -1;
					}

					val = 0;
					index = 10;
					for(i=0; i < 4; i++) {
						tmp = myCom->rcvCmd.cmd[index + i];
						if(tmp <= 57) tmp -= 48;
						else tmp -= 55;
						if(i == 0) val += (tmp * 0x1000);
						else if(i == 1) val += (tmp * 0x100);
						else if(i == 2) val += (tmp * 0x10);
						else val += tmp;
					}
					myCom->misc.PROC_TIME_H = val;

					val = 0;
					index += 5;
					for(i=0; i < 4; i++) {
						tmp = myCom->rcvCmd.cmd[index + i];
						if(tmp <= 57) tmp -= 48;
						else tmp -= 55;
						if(i == 0) val += (tmp * 0x1000);
						else if(i == 1) val += (tmp * 0x100);
						else if(i == 2) val += (tmp * 0x10);
						else val += tmp;
					}
					myCom->misc.PROC_TIME_L = val;
					break;
				case 6:
					if(myCom->rcvCmd.cmdSize != 33) {
						rtl_printf("com:%d rcv_cmd_error rsd %d %d\n",
							comPort, type, myCom->rcvCmd.cmdSize);
						return -1;
					}
					break;
				case 7:
					if(myCom->rcvCmd.cmdSize != 23) {
						rtl_printf("com:%d rcv_cmd_error rsd %d %d\n",
							comPort, type, myCom->rcvCmd.cmdSize);
						return -1;
					}
					break;
				case 8:
					if(myCom->rcvCmd.cmdSize != 33) {
						rtl_printf("com:%d rcv_cmd_error rsd %d %d\n",
							comPort, type, myCom->rcvCmd.cmdSize);
						return -1;
					}
					break;
				case 9:
					if(myCom->rcvCmd.cmdSize != 38) {
						rtl_printf("com:%d rcv_cmd_error rsd %d %d\n",
							comPort, type, myCom->rcvCmd.cmdSize);
						return -1;
					}
					break;
				default:
					break;
			}

			myCom->signal[COM_SIG_CHAMBER_RSD] = P0;
		} else {
			rtl_printf("com:%d rcv_cmd_error rsd %02x %02x\n",
				comPort, rcv_cmd[4], rcv_cmd[5]);
			return -1;
		}
	}

	return 0;
}

void send_cmd_run_stop_TEMP_880(int comPort, int type)
{
	char cmd[32];
	int rtn, data_size, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	data_size = 19;
	*cmd = S_STX; //0x02
	*(cmd+1) = 0x30; //addr 0
	*(cmd+2) = 0x31; //addr 1

	*(cmd+3) = 0x57; //W
	*(cmd+4) = 0x53; //S
	*(cmd+5) = 0x44; //D
	*(cmd+6) = 0x2C; //,

	*(cmd+7) = 0x30; //0
	*(cmd+8) = 0x31; //1
	*(cmd+9) = 0x2C; //,

	*(cmd+10) = 0x30; //0
	*(cmd+11) = 0x31; //1
	*(cmd+12) = 0x30; //0
	*(cmd+13) = 0x31; //1
	*(cmd+14) = 0x2C; //,

	*(cmd+15) = 0x30; //0
	*(cmd+16) = 0x30; //0
	*(cmd+17) = 0x30; //0
	if(type == 0) { //stop
		*(cmd+18) = 0x34; //4
	} else if(type == 1) { //run
		*(cmd+18) = 0x31; //1
	} else { //hold
		*(cmd+18) = 0x32; //2
	}

	cmd_size = make_check_sum_2((char *)&cmd, data_size);

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void send_cmd_temp_set_TEMP_880(int comPort, int temp)
{
	char cmd[32], tmp;
	int rtn, data_size, cmd_size, val1, val2;

	memset((char *)&cmd, 0, sizeof cmd);

	data_size = 19;
	*cmd = S_STX; //0x02
	*(cmd+1) = 0x30; //addr 0
	*(cmd+2) = 0x31; //addr 1

	*(cmd+3) = 0x57; //W
	*(cmd+4) = 0x53; //S
	*(cmd+5) = 0x44; //D
	*(cmd+6) = 0x2C; //,

	*(cmd+7) = 0x30; //0
	*(cmd+8) = 0x31; //1
	*(cmd+9) = 0x2C; //,

	*(cmd+10) = 0x30; //0
	*(cmd+11) = 0x31; //1
	*(cmd+12) = 0x30; //0
	*(cmd+13) = 0x32; //2
	*(cmd+14) = 0x2C; //,

	temp /= 10;
	val1 = temp / 256;
	val2 = temp % 256;

	tmp = val1 / 16;
	if(tmp < 10) {
		*(cmd+15) = 0x30 + tmp;
	} else {
		*(cmd+15) = 0x30 + tmp + 7;
	}

	tmp = val1 % 16;
	if(tmp < 10) {
		*(cmd+16) = 0x30 + tmp;
	} else {
		*(cmd+16) = 0x30 + tmp + 7;
	}

	tmp = val2 / 16;
	if(tmp < 10) {
		*(cmd+17) = 0x30 + tmp;
	} else {
		*(cmd+17) = 0x30 + tmp + 7;
	}

	tmp = val2 % 16;
	if(tmp < 10) {
		*(cmd+18) = 0x30 + tmp;
	} else {
		*(cmd+18) = 0x30 + tmp + 7;
	}

	cmd_size = make_check_sum_2((char *)&cmd, data_size);

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void send_cmd_time_set_TEMP_880(int comPort, int time)
{
	char cmd[32], tmp;
	int rtn, data_size, cmd_size, val1, val2;

	memset((char *)&cmd, 0, sizeof cmd);

	data_size = 29;
	*cmd = S_STX; //0x02
	*(cmd+1) = 0x30; //addr 0
	*(cmd+2) = 0x31; //addr 1

	*(cmd+3) = 0x57; //W
	*(cmd+4) = 0x53; //S
	*(cmd+5) = 0x44; //D
	*(cmd+6) = 0x2C; //,

	*(cmd+7) = 0x30; //0
	*(cmd+8) = 0x32; //2
	*(cmd+9) = 0x2C; //,

	*(cmd+10) = 0x30; //0
	*(cmd+11) = 0x31; //1
	*(cmd+12) = 0x31; //1
	*(cmd+13) = 0x31; //1
	*(cmd+14) = 0x2C; //,

	val1 = (time / 3600) / 256;
	val2 = (time / 3600) % 256;

	tmp = val1 / 16;
	if(tmp < 10) {
		*(cmd+15) = 0x30 + tmp;
	} else {
		*(cmd+15) = 0x30 + tmp + 7;
	}

	tmp = val1 % 16;
	if(tmp < 10) {
		*(cmd+16) = 0x30 + tmp;
	} else {
		*(cmd+16) = 0x30 + tmp + 7;
	}

	tmp = val2 / 16;
	if(tmp < 10) {
		*(cmd+17) = 0x30 + tmp;
	} else {
		*(cmd+17) = 0x30 + tmp + 7;
	}

	tmp = val2 % 16;
	if(tmp < 10) {
		*(cmd+18) = 0x30 + tmp;
	} else {
		*(cmd+18) = 0x30 + tmp + 7;
	}
	*(cmd+19) = 0x2C; //,

	*(cmd+20) = 0x30; //0
	*(cmd+21) = 0x31; //1
	*(cmd+22) = 0x31; //1
	*(cmd+23) = 0x32; //2
	*(cmd+24) = 0x2C; //,

	val1 = ((time % 3600) / 60) / 256;
	val2 = ((time % 3600) / 60) % 256;

	tmp = val1 / 16;
	if(tmp < 10) {
		*(cmd+25) = 0x30 + tmp;
	} else {
		*(cmd+25) = 0x30 + tmp + 7;
	}

	tmp = val1 % 16;
	if(tmp < 10) {
		*(cmd+26) = 0x30 + tmp;
	} else {
		*(cmd+26) = 0x30 + tmp + 7;
	}

	tmp = val2 / 16;
	if(tmp < 10) {
		*(cmd+27) = 0x30 + tmp;
	} else {
		*(cmd+27) = 0x30 + tmp + 7;
	}

	tmp = val2 % 16;
	if(tmp < 10) {
		*(cmd+28) = 0x30 + tmp;
	} else {
		*(cmd+28) = 0x30 + tmp + 7;
	}

	cmd_size = make_check_sum_2((char *)&cmd, data_size);

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void send_cmd_time_use_set_TEMP_880(int comPort, int type)
{
	char cmd[32];
	int rtn, data_size, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	data_size = 19;
	*cmd = S_STX; //0x02
	*(cmd+1) = 0x30; //addr 0
	*(cmd+2) = 0x31; //addr 1

	*(cmd+3) = 0x57; //W
	*(cmd+4) = 0x53; //S
	*(cmd+5) = 0x44; //D
	*(cmd+6) = 0x2C; //,

	*(cmd+7) = 0x30; //0
	*(cmd+8) = 0x31; //1
	*(cmd+9) = 0x2C; //,

	*(cmd+10) = 0x30; //0
	*(cmd+11) = 0x31; //1
	*(cmd+12) = 0x31; //1
	*(cmd+13) = 0x30; //0
	*(cmd+14) = 0x2C; //,

	*(cmd+15) = 0x30; //0
	*(cmd+16) = 0x30; //0
	*(cmd+17) = 0x30; //0
	if(type == 0) { //don't use
		*(cmd+18) = 0x30;
	} else { //use
		*(cmd+18) = 0x31;
	}

	cmd_size = make_check_sum_2((char *)&cmd, data_size);

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void send_cmd_request_TEMP_880(int comPort, int type)
{
	char cmd[32];
	int rtn, data_size, cmd_size;

	memset((char *)&cmd, 0, sizeof cmd);

	data_size = 14;
	*cmd = S_STX; //0x02
	*(cmd+1) = 0x30; //addr 0
	*(cmd+2) = 0x31; //addr 1

	*(cmd+3) = 0x52; //R
	*(cmd+4) = 0x53; //S
	*(cmd+5) = 0x44; //D
	*(cmd+6) = 0x2C; //,

	switch(type) {
		case 1:
			*(cmd+7) = 0x30; //0
			*(cmd+8) = 0x33; //3
			*(cmd+9) = 0x2C; //,
			*(cmd+10) = 0x30; //0
			*(cmd+11) = 0x30; //0
			*(cmd+12) = 0x30; //0
			*(cmd+13) = 0x31; //1
			break;
		case 2:
			*(cmd+7) = 0x30; //0
			*(cmd+8) = 0x33; //4
			*(cmd+9) = 0x2C; //,
			*(cmd+10) = 0x30; //0
			*(cmd+11) = 0x30; //0
			*(cmd+12) = 0x30; //0
			*(cmd+13) = 0x36; //6
			break;
		case 3:
			*(cmd+7) = 0x30; //0
			*(cmd+8) = 0x35; //5
			*(cmd+9) = 0x2C; //,
			*(cmd+10) = 0x30; //0
			*(cmd+11) = 0x30; //0
			*(cmd+12) = 0x31; //1
			*(cmd+13) = 0x30; //0
			break;
		case 4:
			*(cmd+7) = 0x30; //0
			*(cmd+8) = 0x35; //5
			*(cmd+9) = 0x2C; //,
			*(cmd+10) = 0x30; //0
			*(cmd+11) = 0x30; //0
			*(cmd+12) = 0x31; //1
			*(cmd+13) = 0x35; //5
			break;
		case 5:
			*(cmd+7) = 0x30; //0
			*(cmd+8) = 0x32; //2
			*(cmd+9) = 0x2C; //,
			*(cmd+10) = 0x30; //0
			*(cmd+11) = 0x30; //0
			*(cmd+12) = 0x32; //2
			*(cmd+13) = 0x30; //0
			break;
		case 6:
			*(cmd+7) = 0x30; //0
			*(cmd+8) = 0x34; //4
			*(cmd+9) = 0x2C; //,
			*(cmd+10) = 0x30; //0
			*(cmd+11) = 0x30; //0
			*(cmd+12) = 0x32; //2
			*(cmd+13) = 0x35; //5
			break;
		case 7:
			*(cmd+7) = 0x30; //0
			*(cmd+8) = 0x32; //2
			*(cmd+9) = 0x2C; //,
			*(cmd+10) = 0x30; //0
			*(cmd+11) = 0x30; //0
			*(cmd+12) = 0x32; //2
			*(cmd+13) = 0x39; //9
			break;
		case 8:
			*(cmd+7) = 0x30; //0
			*(cmd+8) = 0x34; //4
			*(cmd+9) = 0x2C; //,
			*(cmd+10) = 0x30; //0
			*(cmd+11) = 0x30; //0
			*(cmd+12) = 0x33; //3
			*(cmd+13) = 0x31; //1
			break;
		case 9:
			*(cmd+7) = 0x30; //0
			*(cmd+8) = 0x35; //5
			*(cmd+9) = 0x2C; //,
			*(cmd+10) = 0x30; //0
			*(cmd+11) = 0x30; //0
			*(cmd+12) = 0x33; //3
			*(cmd+13) = 0x35; //5
			break;
		default:
			break;
	}

	cmd_size = make_check_sum_2((char *)&cmd, data_size);

	rtn = send_command((char *)&cmd, cmd_size, 0, comPort);
	if(rtn < 0) {
		rtl_printf("com:%d send cmd error\n", comPort);
	}
}

void make_header(char *cmd, char reply, int cmd_id, int seqno, int cmd_size)
{/*kjg_w
	S_COM_CMD_HEADER	header;

	memset((char *)&header, 0, sizeof(S_COM_CMD_HEADER));
	header.sync = (char)0xF0;
	header.pid = (char)cmd_id;
	header.length = (char)(cmd_size - 5);

	memcpy(cmd, (char *)&header, sizeof(S_COM_CMD_HEADER));*/
}

int make_check_sum(char *cmd, int count)
{
	char len[4];
	int i, bcc;

	bcc = 0;
	for(i=0; i < count; i++) {
		bcc += *(cmd + i);
	}

	len[0] = (bcc / 1000) + 0x30;
	i = bcc % 1000;
	len[1] = (i / 100) + 0x30; 
	i = i % 100;
	len[2] = (i / 10) + 0x30; 
	i = i % 10;
	len[3] = i + 0x30; 
	memcpy((char *)&cmd[count], (char *)&len[0], sizeof(len));

	i = count + sizeof(len);
	*(cmd + i) = S_CR;

	return (i+1);
}

int make_check_sum_2(char *cmd, int count)
{
	char check_sum;
	int i, bcc;

	bcc = 0;
	for(i=1; i < count; i++) {
		bcc += *(cmd + i);
	}

	check_sum = (char)((bcc % 0x100) / 0x10);
	if(check_sum <= 9) check_sum += 48;
	else check_sum += 55;
	*(cmd + count) = check_sum;

	check_sum = (char)(bcc % 0x10);
	if(check_sum <= 9) check_sum += 48;
	else check_sum += 55;
	*(cmd + count + 1) = check_sum;

	*(cmd + count + 2) = S_CR; //0x0D
	*(cmd + count + 3) = S_LF; //0x0A

	return (count+4);
}
/*
int make_check_sum_3(char *cmd, int count)
{
	int i;
   	unsigned char bcc;

	bcc = 0;
	for(i=1; i < count-2; i++) {
		bcc += *(cmd + i);
	}

	*(cmd + count - 2) = ((bcc >> 4)&0xF) + 0x30;
	*(cmd + count - 1) = ((bcc << 4)&0xF) + 0x30;
	return count;
}*/

int make_check_sum_3(char *cmd, int count)
{
	int i, bcc;
	unsigned check;

	bcc = 0;
	for(i=1; i < count-2; i++) {
		bcc += *(cmd + i);
	}

	check = (unsigned char)(bcc & 0xF0) >> 4;
	if(check >= 0x01 && check <= 0x09){
		*(cmd + count - 2) = check + 0x30;
	}else if(check >= 0x0A && check <= 0x0F){
		*(cmd + count - 2) = check + 0x37;
	}else{
		*(cmd + count - 2) = 0x30;
	}
	check = (unsigned char)(bcc & 0x0F);
	if(check >= 0x01 && check <= 0x09){
		*(cmd + count - 1) = check + 0x30;
	}else if(check >= 0x0A && check <= 0x0F){
		*(cmd + count - 1) = check + 0x37;
	}else{
		*(cmd + count - 1) = 0x30;
	}

	return count;
}

int send_command(char *cmd, int size, int cmd_id, int comPort)
{
	unsigned char log_flag;
	char packet[MAX_COM_PACKET_LENGTH];
	int i, hwComPort;

	if(size > MAX_COM_PACKET_LENGTH) {
		rtl_printf("com:%d CMD SEND FAIL!! TOO LARGE SIZE:%d\n", comPort, size);
		return -1;
	}

	memset((char *)&packet, 0, MAX_COM_PACKET_LENGTH);
	memcpy((char *)&packet, cmd, size);

	if(myData->COM.config.CmdSendLog[comPort] == P1) {
		log_flag = 0;
		if(myData->COM.config.CommCheckLog[comPort] == P1) {
			log_flag = 1;
		} else {
			log_flag = 1;
		}

		if(log_flag == 1) {
			rtl_printf("com:%d sendCmd %s:end\n", comPort, packet);
		}
	}

	if(myData->COM.config.CmdSendLog_Hex[comPort] == P1) {
		log_flag = 0;
		if(myData->COM.config.CommCheckLog[comPort] == P1) {
			log_flag = 1;
		} else {
			log_flag = 1;
		}

		if(log_flag == 1) {
			rtl_printf("com:%d sendCmd", comPort);
			for(i=0; i < size; i++) {
		   		rtl_printf(" %02x", (unsigned char)*(cmd + i));
			}
			rtl_printf(":end\n");
		}
	}

	hwComPort = myData->COM.config.comPortId[comPort] - 1;
	rt_com_write(hwComPort, packet, size);

	return 0;
}

int	Find_comPort(int functionType)
{
	int i, comPort;

	for(i=0; MAX_COM_PORT; i++) {
		if(functionType == myData->COM.config.functionType[i]) break;
	}

	if(i >= MAX_COM_PORT) comPort = -1;
	else comPort = i;

	return comPort;
}

