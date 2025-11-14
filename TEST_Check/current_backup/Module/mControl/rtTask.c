#include <linux/module.h>
#include <rtl.h>
#include <rtl_core.h>
#include <pthread.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <linux/slab.h>
//#include "../rt_can/rt_can_ext.h"	//kjg_181121
#ifdef __CAN_FD__	//jhkw_190714s
#include "rt_can_ext.h"	//kjg_181121
#else		//can2.0
#include "../rt_can/rt_can.h"
#endif	//jhkw_190714e
#include "../rt_com/rt_com.h"
#include "../../INC/datastore.h"
#include "local_utils.h"
#include "local_message.h"
#include "ModuleControl.h"
#include "GroupControl.h"
#include "BoardControl.h"
#include "ch_utils.h"
#include "ChannelControl.h"
#include "Analog.h"
#include "FCH_Control.h"
#include "PWM3_Control.h"
#include "InOutControl.h"
#include "SubSensV.h"
#include "CAN.h"
#include "COM.h"
#include "rtTask.h"

pthread_t			thread;
S_SYSTEM_DATA		*myData;
S_MODULE_DATA		*myPs;
S_GROUP_DATA		*myGroup;
S_BD_DATA			*myBd;
S_CH_DATA			*myCh;
S_TEST_CONDITION	*myTestCond;
S_DIO				*myDio;
S_COM_PORT			*myCom;
unsigned char		*dma_buff;
unsigned char		ad_flag; //kjg_190323
unsigned char		can_rx_phase, can_tx_phase; //kjg_2020_1214

void *rt_task(void *arg) 
{
    int	main_slot;
	long long hrt[3];
	struct sched_param p;

	p.sched_priority = 1;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &p);
	pthread_make_periodic_np(pthread_self(), gethrtime(),
		myData->mData.misc.rt_periodic);
	pthread_setfp_np(pthread_self(), 1);

	ad_flag = 0; //kjg_190323
	can_rx_phase = can_tx_phase = 0; //kjg_2020_1214
	
    while(1) {
		hrt[0] = gethrtime();
		main_slot = myPs->misc.main_slot++;

		switch(myPs->config.scan_period) {
			case 25:
				Exec_Slot_25ms(main_slot);
				break;
			case 50:
				Exec_Slot_50ms(main_slot); 
				break;
			case 100:
				Exec_Slot_100ms(main_slot);
				break;
			case 200:
				Exec_Slot_200ms(main_slot);
				break;
			default:
#ifdef __CAN_FD__		//kjhw_190705s
				Exec_Slot_10ms_FD(main_slot);
#else //CAN_2p0b
				Exec_Slot_10ms(main_slot);
#endif //kjhw_190705e
				break;
		}

		hrt[1] = gethrtime();
		pthread_wait_np();
		hrt[2] = gethrtime();
		module_runningTime(main_slot, (long)(hrt[1]-hrt[0]),
			(long)(hrt[2]-hrt[0]));
    }

	return 0;
}

#ifdef __CAN_FD__		//kjhw_190705s
void Exec_Slot_10ms_FD(int main_slot)
{
	int i, max_can_ch;
	long long hrt[10];

	max_can_ch = myPs->config.installedCAN;
	hrt[0] = gethrtime();
	if(main_slot >= 0 && main_slot <= 4) {
		if(myData->dio.config.logic_type == 0) { //kjg_logic_type_140324_s
 	  		Analog_Value_Input(main_slot);
		}
		FCH_Control(main_slot);
		PWM3_Control(main_slot);
	}
	if(main_slot >= 0 && main_slot < 7) {
		if(myData->dio.config.logic_type == 2) {
 	  		Analog_Value_Input_12(main_slot);
		}
	}
	if(myPs->config.daq_type == DAQ_TYPE12) {
		SubSensV_process_12(main_slot);
	}else{
		if(myPs->config.daq_type >= DAQ_TYPE8) {
			SubSensV_process();
		}else{
			SubSensV_process_7(main_slot);
		}
	}
	if(myData->AppControl.misc.Load_Process_CAN[2] == P1) {
	} else {
		if(main_slot >= 0 && main_slot < 4) {
			CAN_Control2(main_slot);	//shh_231004
			//CAN_Control2(main_slot);	
		}
	}

	switch(main_slot) {
		case 0:
		case 1:
		case 2:
		case 3:
			ChannelControl(main_slot);
			//Output_Ch_Switch(main_slot);
			COM_Control_Receive(main_slot);
			COM_Control_Signal(main_slot);
			break;
		case 4:
			if(myData->dio.config.logic_type == 0) { //kjg_logic_type_140324_s
			}else if(myData->dio.config.logic_type == 2) {
			}else{
				ad_flag = inb(0x65C); //kjhw_181218
				if(ad_flag == 0) {
					Analog_Value_Input_3(0); //fifo1(adc1)_read
					outb(0x00, 0x650); //adc_start
				}
				Source_Value_Average(0); //fifo1(adc1)
				Ch_Value_Average(0); //fifo1(adc1)
				bd_sens_count_increment_2();
			}
			PNE_AC_Power_Fail_Detect();
   			ModuleControl();
			GroupControl(MAX_GROUP_4);
#ifdef __SBC_EM104_A5362__
			Sync_RTC(); //kjg_w
#endif
			for(i=0; i < 4; i++) InOutControl(i);
			break;
		case 5:
		case 6:
		case 7:
		case 8:
			Output_Ch_Switch(main_slot - 5); //kjhw_191127
			if(myData->mData.signal[M_SIG_EOL_PROCEDURE_VBF_FILE_DOWNLOAD] == P1
				&& (myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] >= P22
				&& myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] <= P24)) {
				ModuleControl();
			}
			break;
		case 9:
			if(myData->dio.config.watchdogType == 2) { //kjg_logic_type_140324
				CheckWDT();
				EnableWDT();
			}
			RefreshWDT();

			Check_Message(0);
			Check_Message(1);
			Check_Message(2);
			timer_1sec_increment();

			for(i=0; i < 4; i++) Aux_Voltage_Temp_Thermistor_compare(i); 
			C_OutScan();
			myPs->misc.main_slot = 0;
			break;
	}
	for(i=0; i < 4; i++) {
		tx_can_data_3(can_tx_phase);
		can_tx_phase++;
		if(can_tx_phase >= max_can_ch) can_tx_phase = 0;
	}
	for(i=0; i < 4; i++) {
		hrt[4] = gethrtime();
			if((hrt[4] - hrt[0]) < 700000) {
			if(rt_can_fd_rx4(can_rx_phase) == 100) {
				CAN_Control1(can_rx_phase);
			} else {
				if((myPs->misc.timer_1sec
					- myData->CAN.receive_time[can_rx_phase]) > 2) {
					//shh_230827s
					if(myData->mData.config.division_CAN == 1) {
						myData->cData[can_rx_phase].misc.external_comm_state
							|= 0x04;
					} else {
						if((can_rx_phase % 2) == 0) {
							myData->cData[can_rx_phase / 2].misc.external_comm_state
								|= 0x04;
						} else {
							myData->cData[can_rx_phase / 2].misc.external_comm_state
								|= 0x08;
						}
					}
					/*if((can_rx_phase % 2) == 0) {
						myData->cData[can_rx_phase / 2].misc.external_comm_state
							|= 0x04;
					} else {
						myData->cData[can_rx_phase / 2].misc.external_comm_state
							|= 0x08;
					}*/
					//shh_230827e
				}
			}
			can_rx_phase++;
			if(can_rx_phase >= max_can_ch) can_rx_phase = 0;
		} else break;
	}
}
#else //CAN_2p0b
void Exec_Slot_10ms(int main_slot)
{
	int i;

	PNE_AC_Power_Fail_Detect();

	if(main_slot >= 0 && main_slot <= 4) {
		if(main_slot == 0) {
			if(myPs->config.daq_type < DAQ_TYPE8) { //kjg_150225
				SubSensV_adc_start(); //DAQ
			}
		}

		if(myData->dio.config.logic_type == 0) { //kjg_logic_type_140324_s
 	  		Analog_Value_Input(main_slot);
		} else {
			switch(main_slot) {
				case 0:
					//ad_flag = inb(0x65C);
					//if(ad_flag == 0) {
					//	Analog_Value_Input_3(0); //fifo1(adc1)_read
					//}
					break;
				case 1:
					//if(ad_flag == 0) {
					//	outb(0x00, 0x650); //adc_start
					//}
					break;
				case 2:
					//Source_Value_Average(0); //fifo1(adc1)
					break;
				case 3:
					//Ch_Value_Average(0); //fifo1(adc1)
					break;
				case 4:
					//Ch_Value_Average(1); //fifo2(adc2)
					//kjhw_181215
					ad_flag = inb(0x65C); //kjhw_181218
					if(ad_flag == 0) {
						Analog_Value_Input_3(0); //fifo1(adc1)_read
						outb(0x00, 0x650); //adc_start
					}
					Source_Value_Average(0); //fifo1(adc1)
					Ch_Value_Average(0); //fifo1(adc1)
					bd_sens_count_increment_2();
					break;
				default:
					break;
			}
		} //kjg_logic_type_140324_e

		FCH_Control(main_slot);
		PWM3_Control(main_slot);

		if(main_slot < 4) CAN_Control1(main_slot);

		if(myData->mData.signal[M_SIG_EOL_PROCEDURE_VBF_FILE_DOWNLOAD] == P1
			&& (myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] >= P22
			&& myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] <= P24)) {
			ModuleControl();
			CAN_Control2(0);
		}

		COM_Control_Receive(main_slot); //COM
	} else if(main_slot == 5) {
  		ModuleControl();
		GroupControl(MAX_GROUP_4);
#ifdef __SBC_EM104_A5362__
		Sync_RTC(); //kjg_w
#endif
		CAN_Control2(0);
		CAN_Control2(1);

		COM_Control_Signal(0); //COM

		for(i=0; i < 2; i++) ChannelControl(i);
	} else {
		if(main_slot == 6) {
			CAN_Control2(2);
			CAN_Control2(3);

			if(myData->mData.signal[M_SIG_EOL_PROCEDURE_VBF_FILE_DOWNLOAD] == P1
				&& (myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] >= P22
				&& myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] <= P24)) {
				ModuleControl();
				CAN_Control2(0);
			}

			COM_Control_Signal(1); //COM

			for(i=2; i < 4; i++) ChannelControl(i);

			if(myPs->config.daq_type < DAQ_TYPE8) { //kjg_150225
				switch(myPs->config.daq_type) {
					case DAQ_TYPE6: //lki_111010
					case DAQ_TYPE7:
						SubSensV_No_DMA_Read(0); //NO_DMA_DAQ
						break;
					default:
						break;
				}
			}
		} else if(main_slot == 7) {
			if(myData->mData.signal[M_SIG_EOL_PROCEDURE_VBF_FILE_DOWNLOAD] == P1
				&& (myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] >= P22
				&& myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] <= P24)) {
				ModuleControl();
				CAN_Control2(0);
			}

			for(i=4; i < 6; i++) ChannelControl(i);

			if(myPs->config.daq_type < DAQ_TYPE8) { //kjg_150225
				switch(myPs->config.daq_type) {
					case DAQ_TYPE6: //lki_111010
					case DAQ_TYPE7:
						//SubSensV_No_DMA_Read(0); //NO_DMA_DAQ
						break;
					default:
						SubSensV_Control(0); //DAQ
						break;
				}
			}

			for(i=0; i < 4; i++) InOutControl(i);

			COM_Control_Signal(2); //COM

			if(myPs->config.daq_type < DAQ_TYPE8) { //kjg_150225
				switch(myPs->config.daq_type) {
					case DAQ_TYPE6: //lki_111010
					case DAQ_TYPE7:
						SubSensV_No_DMA_Read(1); //NO_DMA_DAQ
						break;
					default:
						SubSensV_Control(1); //DAQ
						break;
				}
			}
		} else if(main_slot == 8) {
			if(myData->mData.signal[M_SIG_EOL_PROCEDURE_VBF_FILE_DOWNLOAD] == P1
				&& (myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] >= P22
				&& myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] <= P24)) {
				ModuleControl();
				CAN_Control2(0);
			}

			for(i=6; i < 8; i++) ChannelControl(i);

			if(myPs->config.daq_type < DAQ_TYPE8) { //kjg_150225
				SubSensV_Control2(0); //DAQ
			}

			COM_Control_Signal(3); //COM
		} else if(main_slot == 9) {
			if(myData->mData.signal[M_SIG_EOL_PROCEDURE_VBF_FILE_DOWNLOAD] == P1
				&& (myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] >= P22
				&& myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] <= P24)) {
				ModuleControl();
				CAN_Control2(0);
			}

			if(myPs->config.daq_type < DAQ_TYPE8) { //kjg_150225
				SubSensV_Control2(1); //DAQ
			}

			COM_Control_Signal(4); //COM

			if(myData->dio.config.watchdogType == 2) { //kjg_logic_type_140324
				CheckWDT();
				EnableWDT();
			}
			RefreshWDT();

			Check_Message(0);
			Check_Message(1);
			Check_Message(2);
			timer_1sec_increment();

			for(i=0; i < 4; i++) Aux_Voltage_Temp_Thermistor_compare(i);

			myPs->misc.main_slot = 0;
		}

		Output_Ch_Switch(main_slot-6);
		C_OutScan();
	}

	if(myPs->config.daq_type == DAQ_TYPE12) {
		SubSensV_process_12(main_slot);
	} else {
		if(myPs->config.daq_type >= DAQ_TYPE8) {
			SubSensV_process(); //kjg_150225
		}
	}
}


#endif //kjhw_190705e

void Exec_Slot_25ms(int main_slot)
{
	int i;

	if(main_slot >= 0 && main_slot <= 9) {
 	  	Analog_Value_Input(main_slot);

		if(main_slot == 0) {
			SubSensV_adc_start(); //DAQ
		}

		if(main_slot >= 0 && main_slot <= 3) {
			CAN_Control1(main_slot);
		}
	} else if(main_slot == 10) {
	} else if(main_slot == 11) {
   		ModuleControl();
	} else if(main_slot == 12) {
		GroupControl(MAX_GROUP_4);
	} else if(main_slot == 13) {
	} else if(main_slot == 14) {
		for(i=0; i < 4; i++) ChannelControl(i);
	} else if(main_slot == 15) {
		for(i=4; i < 8; i++) ChannelControl(i);
	} else if(main_slot == 16) {
		for(i=0; i < 4; i++) InOutControl(i);
	} else if(main_slot == 17) {
		SubSensV_Control(0); //DAQ

		CAN_Control2(0);
	} else if(main_slot == 18) {
		SubSensV_Control(1); //DAQ

		CAN_Control2(1);
	} else if(main_slot == 19) {
		SubSensV_Control2(0); //DAQ

		CAN_Control2(2);
	} else if(main_slot == 20) {
		SubSensV_Control2(1); //DAQ

		CAN_Control2(3);

		RefreshWDT();
	} else {
		if(main_slot == 21) {
			Check_Message(0);
		} else if(main_slot == 22) {
			Check_Message(1);
		} else if(main_slot == 23) {
			Check_Message(2);
		} else if(main_slot == 24) {
			timer_1sec_increment();
			myPs->misc.main_slot = 0;
		}
		Output_Ch_Switch(main_slot-21);
		C_OutScan();
	}
}

void Exec_Slot_50ms(int main_slot)
{
	if(main_slot >= 0 && main_slot <= 9) {
 	  	Analog_Value_Input(main_slot);
	} else if(main_slot == 10) {
		SubSensV_adc_start(); //DAQ
	} else if(main_slot >= 11 && main_slot <= 14) {
		CAN_Control1(main_slot - 11);

		COM_Control_Receive(main_slot - 11); //COM
	} else if(main_slot == 15) {
   		ModuleControl();
	} else if(main_slot == 16) {
		GroupControl(MAX_GROUP_4);
	} else if(main_slot == 17) {
	} else if(main_slot >= 18 && main_slot <= 25) {
		ChannelControl(main_slot - 18);
	} else if(main_slot >= 26 && main_slot <= 29) {
		InOutControl(main_slot - 26);
	} else if(main_slot == 30) {
		SubSensV_Control(0); //DAQ
	} else if(main_slot == 31) {
		CAN_Control2(0);

		COM_Control_Signal(0); //COM
	} else if(main_slot == 32) {
		SubSensV_Control(1); //DAQ
	} else if(main_slot == 33) {
		CAN_Control2(1);

		COM_Control_Signal(1); //COM
	} else if(main_slot == 34) {
		SubSensV_Control2(0); //DAQ
	} else if(main_slot == 35) {
		CAN_Control2(2);
	} else if(main_slot == 36) {
		SubSensV_Control2(1); //DAQ
	} else if(main_slot == 37) {
		CAN_Control2(3);
	} else if(main_slot >= 38 && main_slot <= 41) {
		Output_Ch_Switch(main_slot - 38);
		C_OutScan();
	} else if(main_slot == 42) {
		Check_Message(0);
	} else if(main_slot == 43) {
		Check_Message(1);
	} else if(main_slot == 44) {
		Check_Message(2);
	} else if(main_slot == 49) {
		RefreshWDT();
		timer_1sec_increment();
		myPs->misc.main_slot = 0;
	}
}

void Exec_Slot_100ms(int main_slot)
{
	if(main_slot >= 0 && main_slot <= 39) {
 	  	Analog_Value_Input(main_slot);
	} else if(main_slot == 40) {
		SubSensV_adc_start(); //DAQ
	} else if(main_slot >= 41 && main_slot <= 44) {
		CAN_Control1(main_slot - 41);
	} else if(main_slot >= 45 && main_slot <= 46) {
		COM_Control_Receive(main_slot - 45); //COM
	} else if(main_slot == 47) {
   		ModuleControl();
	} else if(main_slot >= 48 && main_slot <= 49) { //2group
		GroupControl(main_slot - 48);
	} else if(main_slot == 50) {
	} else if(main_slot >= 51 && main_slot <= 74) { //4slot * 6step = 24
		ChannelControl(main_slot - 51);
	} else if(main_slot >= 75 && main_slot <= 78) {
		InOutControl(main_slot - 75);
	} else if(main_slot == 79) {
		SubSensV_Control(0); //DAQ
	} else if(main_slot == 80) {
		CAN_Control2(0);
	} else if(main_slot == 81) {
		SubSensV_Control(1); //DAQ
	} else if(main_slot == 82) {
		CAN_Control2(1);
	} else if(main_slot == 83) {
		SubSensV_Control2(0); //DAQ
	} else if(main_slot == 84) {
		CAN_Control2(2);
	} else if(main_slot == 85) {
		SubSensV_Control2(1); //DAQ
	} else if(main_slot == 86) {
		CAN_Control2(3);
	} else if(main_slot >= 87 && main_slot <= 88) {
		COM_Control_Signal(main_slot - 87); //COM
	} else if(main_slot >= 89 && main_slot <= 92) {
		Output_Ch_Switch(main_slot - 89);
		C_OutScan();
	} else if(main_slot == 96) {
		Check_Message(0);
	} else if(main_slot == 97) {
		Check_Message(1);
	} else if(main_slot == 98) {
		Check_Message(2);
	} else if(main_slot == 99) {
		RefreshWDT();
		timer_1sec_increment();
		myPs->misc.main_slot = 0;
	}
}

void Exec_Slot_200ms(int main_slot)
{
	int i;

	if(main_slot >= 0 && main_slot <= 2) {
		AnalogValue_Operate(0, main_slot); //read_Reference

		COM_Control_Receive(main_slot); //COM
	} else if(main_slot >= 3 && main_slot <= 62) { //10ch * 6bd = 60ch
		AnalogValue_Operate(1, main_slot-3); //read_Channel

		if(main_slot >= 3 && main_slot <= 4) {
			COM_Control_Receive(main_slot); //COM
		}
	} else if(main_slot >= 63 && main_slot <= 70) {
		AnalogValue_Operate(2, main_slot-63); //read_Temp
	} else if(main_slot >= 71 && main_slot <= 72) {
		COM_Control_Signal(main_slot-71); //COM
		COM_Control_Signal(main_slot-71+2); //COM

		if(main_slot == 71) {
			aCalReferenceAverage1(0);
		} else {
			aCalReferenceAverage2(0);
		}
	} else if(main_slot == 73) {
		COM_Control_Signal(main_slot-73+4); //COM

		aCalibratorReference();
   		ModuleControl();
	} else if(main_slot >= 74 && main_slot <= 76) {
		aCalChAverage(main_slot-74);
		aCalChAverage(main_slot-74+3);
	} else if(main_slot >= 77 && main_slot <= 79) {
		GroupControl(main_slot-77);
	} else if(main_slot >= 80 && main_slot <= 82) {
		BoardControl(main_slot-80);
		BoardControl(main_slot-80+3);
	} else if(main_slot >= 83 && main_slot <= 90) {
		ChannelControl(main_slot-83); //kjg_w 0~7
	} else if(main_slot >= 91 && main_slot <= 93) {
		bVICmd_Output(main_slot-91);
		bVICmd_Output(main_slot-91+3);

		bCDSelect_Output(main_slot-91);
		bCDSelect_Output(main_slot-91+3);

		bRangeRelay_Output(main_slot-91);
		bRangeRelay_Output(main_slot-91+3);

		bRunRelay_Output(main_slot-91);
		bRunRelay_Output(main_slot-91+3);

		bParallelRelay_Output(main_slot-91);
		bParallelRelay_Output(main_slot-91+3);
	} else if(main_slot >= 94 && main_slot <= 97) {
		InOutControl(main_slot-94);
		Output_Ch_Switch(main_slot-94);
	} else if(main_slot == 98) {
		C_OutScan();
		RefreshWDT();
	} else if(main_slot == 99) {
		Check_Message(0);
		Check_Message(1);
		Check_Message(2);

		//kjg_110814
		sens_count_increment_group();
		for(i=0; i < myPs->config.installedBd; i++) {
			sens_count_increment_bd(i);
		}

		timer_1sec_increment();
		myPs->misc.main_slot = 0;
	}
}

int init_module(void)
{
#ifdef __CAN_FD__	//jhkw_190629s
	init_rt_can();	//kjg_181121
#endif	//jhkw_190629e
	
	if(Open_SystemMemory(0) < 0) return -1;

	Initialize();

	return pthread_create(&thread, NULL, rt_task, 0);
}

void cleanup_module(void)
{
#ifdef __CAN_FD__	//jhkw_190629s
	cleanup_rt_can(); //kjg_181121
#endif	//jhkw_190629e
	
	Close_DMA_DAQ();
	Close_RT_CAN();
	Close_RT_COM();

	pthread_delete_np(thread);

	DisableWDT();
	Clear_OutPort();
	myData->AppControl.signal[APP_SIG_MODULE_CONTROL_PROCESS] = P3;

	Close_SystemMemory();
}
