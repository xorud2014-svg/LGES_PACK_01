#include <linux/module.h>
#include <rtl.h>
#include <rtl_core.h>
#include <pthread.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <linux/slab.h>
//#include "rt_can_ext.h"	//kjg_181121
#ifdef __CAN_FD__			//csk_190617s
#include "rt_can_ext.h"	//kjg_181121
#else		//can2.0
#include "../rt_can/rt_can.h"
#endif						//csk_190617e
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
//csk_240301	unsigned char		can_rx_phase, can_tx_phase;

void *rt_task(void *arg) 
{
	int	main_slot;
//csk_240301	long long hrt[3];
	struct sched_param p;

	p.sched_priority = 1;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &p);
	pthread_make_periodic_np(pthread_self(), gethrtime(),
		myData->mData.misc.rt_periodic);
	pthread_setfp_np(pthread_self(), 1);

	ad_flag = 0; //kjg_190323
//csk_240301	can_rx_phase = can_tx_phase = 0;

	while(1) {
		//hrt[0] = gethrtime();
		myData->mData.misc.hrt[0] = gethrtime();	//csk_240301

		myPs->misc.main_slot++;
		main_slot = (int)myPs->misc.main_slot;

		module_runningTime3(); //kjg_240213

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
			case 20:
#ifdef __CAN_FD__		//kjhw_190705s
				Exec_Slot_10ms_FD(main_slot);
#else //CAN_2p0b
				Exec_Slot_10ms(main_slot);
#endif //kjhw_190705e
				break;
			default:
#ifdef __CAN_FD__		//kjhw_190705s
				Exec_Slot_10ms_FD(main_slot);
#else //CAN_2p0b
				Exec_Slot_10ms(main_slot);
#endif //kjhw_190705e
				break;
		}

		myData->mData.misc.timer_1tick_count++; //kjg_240213 1ms

		//hrt[1] = gethrtime();
		myData->mData.misc.hrt[1] = gethrtime();	//csk_240301
		pthread_wait_np();
		//hrt[2] = gethrtime();
		myData->mData.misc.hrt[2] = gethrtime();	//csk_240301
		//module_runningTime(main_slot, (long)(hrt[1]-hrt[0]),
		//	(long)(hrt[2]-hrt[0]));
		module_runningTime2(main_slot);				//csk_240301
	}

	return 0;
}

#ifdef __CAN_FD__		//kjhw_190705s
void Exec_Slot_10ms_FD(int main_slot)
{
	int i, max_can_ch;
//csk_240301	long long hrt[10];

	//shh_250122s SWEGPROD-1488
	if(SBC_TIME_FLAG == 1) {
		slot_tic_timer2();//KHK 20191121
	}else {
		slot_tic_timer();
		//myPs->misc.slot_tic_timer ++;
		//myPs->misc.slot_tic_timer += 1;
	}
	//slot_tic_timer();//KHK 20191121
	//shh_250122e
	max_can_ch = myPs->config.installedCAN;
//csk_240301	hrt[0] = gethrtime();
	if(main_slot >= 0 && main_slot <= 3) {
		i = main_slot * 2;
		if(i < max_can_ch) {
			CAN_Control2(i);
		}
		i++;
		if(i < max_can_ch) {
			CAN_Control2(i);
		}
	}

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
					//Analog_Value_Input_3(0); //fifo1(adc1)_read
					/*kjg_190327 ad_flag = inb(0x65C); //kjhw_181218
					if(ad_flag == 0) {
						Analog_Value_Input_3(0); //fifo1(adc1)_read
					}*/
					break;
				case 1:
					//Analog_Value_Input_3(1); //fifo2(adc2)_read
					//kjhw_181215
					/*kjg_190327 if(ad_flag == 0) { //kjhw_181218
						outb(0x00, 0x650); //adc_start
					}*/
					break;
				case 2:
					//kjg_190327 Source_Value_Average(0); //fifo1(adc1)
					//Source_Value_Average(1); //fifo2(adc2)
					//kjhw_181215
					break;
				case 3:
					//kjg_190327 Ch_Value_Average(0); //fifo1(adc1)
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

		if(myData->mData.signal[M_SIG_EOL_PROCEDURE_VBF_FILE_DOWNLOAD] == P1
			&& (myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] >= P22
			&& myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] <= P24)) {
			ModuleControl();

			CAN_Control2(0);
		}

		COM_Control_Receive(main_slot); //COM

		if(main_slot == 4) {
			PNE_AC_Power_Fail_Detect();

			ModuleControl();
			GroupControl(MAX_GROUP_4);

#ifdef __SBC_EM104_A5362__
			//shh_250122s SWEGPROD-1488
			if(SBC_TIME_FLAG == 1) {
				Sync_RTC2(); //kjg_w
			} else {
				Sync_RTC();
			} //shh_250122_e
#endif

			for(i=0; i < 4; i++) InOutControl(i);
		}
	} else if(main_slot == 5) {
		ChannelControl(0);
	} else {
		if(main_slot == 6) {
			if(myData->mData.signal[M_SIG_EOL_PROCEDURE_VBF_FILE_DOWNLOAD] == P1
				&& (myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] >= P22
				&& myData->mData.signal[M_SIG_BMS_EOL_FLASH_PHASE] <= P24)) {
				ModuleControl();
				CAN_Control2(0);
			}

			ChannelControl(1);

			COM_Control_Signal(1); //COM

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

			ChannelControl(2);

			if(myPs->config.daq_type < DAQ_TYPE8) { //kjg_150225
				switch(myPs->config.daq_type) {
					case DAQ_TYPE6: //lki_111010
					case DAQ_TYPE7:
						//SubSensV_No_DMA_Read(0); //NO_DMA_DAQ
						break;
					default:
						SubSensV_Control1(0); //DAQ
						break;
				}
			}

			COM_Control_Signal(2); //COM

			if(myPs->config.daq_type < DAQ_TYPE8) { //kjg_150225
				switch(myPs->config.daq_type) {
					case DAQ_TYPE6: //lki_111010
					case DAQ_TYPE7:
						SubSensV_No_DMA_Read(1); //NO_DMA_DAQ
						break;
					default:
						SubSensV_Control1(1); //DAQ
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

			ChannelControl(3);

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

			COM_Control_Signal(0); //COM
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
			for(i=0; i < 4; i++) CAN_function_compare(i);	//ktg_210706

			myPs->misc.main_slot = (-1);
		}

		//Output_Ch_Switch(main_slot-6);
		//if(main_slot >= 8) Output_Ch_Switch(main_slot-8);		//csk_190507
		if(main_slot == 8) Output_Ch_Switch(0);					//csk_190617s
		if(main_slot == 9) {
			Output_Ch_Switch(1);
			Output_Ch_Switch(2);
			Output_Ch_Switch(3);
		}														//csk_190617e
		C_OutScan();
	}

	if(myPs->config.daq_type >= DAQ_TYPE8
		&& myPs->config.daq_type < DAQ_TYPE20) {
		SubSensV_process(); //kjg_150225
	} else if(myPs->config.daq_type == DAQ_TYPE20) { //kjg_180914
		SubSensV_process_CAN_DAQ();
	}
/*	if(myPs->config.daq_type >= DAQ_TYPE8
		&& myPs->config.daq_type < DAQ_TYPE20) {
		SubSensV_process(); //kjg_150225
	} else if(myPs->config.daq_type == DAQ_TYPE20) { //kjg_180914
		SubSensV_process_CAN_DAQ();
	}*/

	rt_can_control(); //kjg_240213
	/*	//csk_240301
	//kjg_2020_1214
	for(i=0; i < 16; i++) {
		tx_can_data_3(can_tx_phase);
		can_tx_phase++;
		if(can_tx_phase >= max_can_ch) can_tx_phase = 0;
	}
	if(main_slot != 4 ) {
		for(i=0; i < 64; i++) {
			hrt[4] = gethrtime();
			if((hrt[4] - hrt[0]) < (myPs->config.scan_period * 70000)) {
				if(rt_can_fd_rx4(can_rx_phase) == 100) {
					CAN_Control1(can_rx_phase);
				} else {
					if((myPs->misc.timer_1sec
						- myData->CAN.receive_time[can_rx_phase]) > 2) {
						switch(myData->AppControl.config.systemModel) {
							case C_LGC_500V_250A_125A_50A_500KW_10:	//ktg_210222
							case C_LGC_500V_250A_125A_50A_500KW_11:	//ktg_210222
								if(can_rx_phase == 0) {
									myData->cData[0].
										misc.external_comm_state |= 0x04;
								} else if(can_rx_phase == 1) {
									myData->cData[0].
										misc.external_comm_state |= 0x08;
								} else if(can_rx_phase == 2) {
									myData->cData[2].
										misc.external_comm_state |= 0x04;
								} else if(can_rx_phase == 3) {
									myData->cData[2].
										misc.external_comm_state |= 0x08;
								}
								break;
							default :	
								if(myData->mData.config.division_CAN == 1) {	//shh_220823
									myData->cData[can_rx_phase].misc.external_comm_state |= 0x04;
								} else {
									if((can_rx_phase % 2) == 0) {
										myData->cData[can_rx_phase / 2].misc.external_comm_state
											|= 0x04;
									} else {
										myData->cData[can_rx_phase / 2].misc.external_comm_state
											|= 0x08;
									}	
								}			
								break;
						}
					}
				}
				can_rx_phase++;
				if(can_rx_phase >= max_can_ch) can_rx_phase = 0;
			} else break;
		}
	}
	*/	//csk_240301
}
#else //CAN_2p0b
void Exec_Slot_10ms(int main_slot)
{
	int i;

	//kjh_211021 PNE_AC_Power_Fail_Detect();
	slot_tic_timer();//KHK 20191121

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
						SubSensV_Control1(0); //DAQ
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
						SubSensV_Control1(1); //DAQ
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
			for(i=0; i < 4; i++) CAN_function_compare(i);	//ktg_210706
			PNE_AC_Power_Fail_Detect();	//kjh_211021

			myPs->misc.main_slot = (-1);
		}

		Output_Ch_Switch(main_slot-6);
		C_OutScan();
	}

	/*if(myPs->config.daq_type >= DAQ_TYPE8) {
		SubSensV_process(); //kjg_150225
	}*/
	if(myPs->config.daq_type >= DAQ_TYPE8
		&& myPs->config.daq_type < DAQ_TYPE20) {
		SubSensV_process(); //kjg_150225
	} else if(myPs->config.daq_type == DAQ_TYPE20) { //kjg_180914
		SubSensV_process_CAN_DAQ();
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
		SubSensV_Control1(0); //DAQ

		CAN_Control2(0);
	} else if(main_slot == 18) {
		SubSensV_Control1(1); //DAQ

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
			myPs->misc.main_slot = -1;
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
		SubSensV_Control1(0); //DAQ
	} else if(main_slot == 31) {
		CAN_Control2(0);

		COM_Control_Signal(0); //COM
	} else if(main_slot == 32) {
		SubSensV_Control1(1); //DAQ
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
		myPs->misc.main_slot = (-1);
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
		SubSensV_Control1(0); //DAQ
	} else if(main_slot == 80) {
		CAN_Control2(0);
	} else if(main_slot == 81) {
		SubSensV_Control1(1); //DAQ
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
		myPs->misc.main_slot = -1;
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
		myPs->misc.main_slot = (-1);
	}
}

int init_module(void)
{
/*	//csk_240301
#ifdef __CAN_FD__		//csk_190617
	init_rt_can();	//kjg_181121
#endif					//csk_190617
*/	//csk_240301

	if(Open_SystemMemory(0) < 0) return -1;

//csk_240301	Initialize();
	if(Initialize() < 0) return -2;

	return pthread_create(&thread, NULL, rt_task, 0);
}

void cleanup_module(void)
{
#ifdef __CAN_FD__
	//cleanup_rt_can();
	if(myData->AppControl.misc.Load_Process_CAN[0] == P1) cleanup_rt_can();	//csk_240301
#endif
	
	Close_DMA_DAQ();
#ifdef __CAN_FD__
#else
	Close_RT_CAN();
#endif
	Close_RT_COM();

	pthread_delete_np(thread);

	DisableWDT();
	Clear_OutPort();
	myData->AppControl.signal[APP_SIG_MODULE_CONTROL_PROCESS] = P3;

	Close_SystemMemory();
}

#ifdef __CAN_FD__
void rt_can_control(void)
{
	unsigned char rt_can_ch_phase;
	unsigned char debug = 1; //csk_240507
	int i, max_can_ch, can_ch, rtn;
	long long diff_hrt;

	max_can_ch = myData->mData.config.installedCAN;

	for(i=0; i < 16; i++) { //kjg_240624 8 -> 16
		diff_hrt = gethrtime() - myData->mData.misc.hrt[0];
		if(diff_hrt > (myPs->config.scan_period * 70000)) break;

		rt_can_ch_phase = myData->CAN.rt_can_ch_phase;
		myData->CAN.rt_can_ch_phase++;
		if(myData->CAN.rt_can_ch_phase >= 8) myData->CAN.rt_can_ch_phase = 0;

		if(max_can_ch == 4) {
			switch(rt_can_ch_phase) {
				case 0: can_ch = 0; break;
				case 1: can_ch = 8; break;
				case 2: can_ch = 1; break;
				case 3: can_ch = 8; break;
				case 4: can_ch = 2; break;
				case 5: can_ch = 8; break;
				case 6: can_ch = 3; break;
				case 7: can_ch = 8; break;
				default: can_ch = 8; break;
			}
		} else { //max_can_ch == 8
			switch(rt_can_ch_phase) {
				case 0: can_ch = 0; break;
				case 1: can_ch = 4; break;
				case 2: can_ch = 1; break;
				case 3: can_ch = 5; break;
				case 4: can_ch = 2; break;
				case 5: can_ch = 6; break;
				case 6: can_ch = 3; break;
				case 7: can_ch = 7; break;
				default: can_ch = 8; break;
			}
		}
		if(can_ch == 8) continue;

		switch(myData->CAN.rt_can_control_phase[can_ch]) {
			case PH0:
				if(myData->AppControl.misc.Load_Process_CAN[0] == P1
					&& myData->mData.misc.timer_1sec >= 2) {
					myData->CAN.rt_can_control_phase[can_ch]++;
				}
				break;
			case PH1:
				rtn = rt_can_set_param_11(can_ch);
				if(rtn == 1) {
					myData->CAN.chip_diff_1tick_count[can_ch]
						= myData->mData.misc.timer_1tick_count;

					myData->CAN.rt_can_control_phase[can_ch] = PH11;
				}
				break;
			case PH11:
				rtn = rx_can_data_11(can_ch);
				if(rtn == 10) { //kjg_240624
					myData->CAN.rt_can_control_phase[can_ch] = PH12;

					rtn = tx_can_data_11(can_ch);
					if(rtn == 10 || rtn == 11 || rtn == 12) {
						myData->CAN.rt_can_control_phase[can_ch] = PH11;
					} else if(rtn == 50) {
						myData->CAN.rt_can_control_phase[can_ch] = PH50;
					}
				} else if(rtn == 11 || rtn == 12) { //kjg_240624
					myData->CAN.rt_can_control_phase[can_ch] = PH12;
				} else if(rtn == 49) {
					CAN_Control1(can_ch);
					myData->CAN.rt_can_control_phase[can_ch] = PH12;
				} else if(rtn == 50) {
					myData->CAN.rt_can_control_phase[can_ch] = PH50;
				}
				break;
			case PH12:
				rtn = tx_can_data_11(can_ch);
				if(rtn == 10) { //kjg_240624
					myData->CAN.rt_can_control_phase[can_ch] = PH11;

					rtn = rx_can_data_11(can_ch);
					if(rtn == 10 || rtn == 11 || rtn == 12) { 
						myData->CAN.rt_can_control_phase[can_ch] = PH12;
					} else if(rtn == 49) {
						CAN_Control1(can_ch);
						myData->CAN.rt_can_control_phase[can_ch] = PH12;
					} else if(rtn == 50) {
						myData->CAN.rt_can_control_phase[can_ch] = PH50;
					}
				} else if(rtn == 11 || rtn == 12) { //kjg_240624
					myData->CAN.rt_can_control_phase[can_ch] = PH11;
				} else if(rtn == 50) {
					myData->CAN.rt_can_control_phase[can_ch] = PH50;
				}
				break;
			case PH20:
				if(myData->mData.signal[M_SIG_WDT_PHASE] != P2) {
					if(can_ch == myData->test_val_ll1[15]) {
						if(myData->test_val_ll1[14] < myData->test_val_ll1[13]) {
							myData->test_val_ll1[14]++;
							//spi_test_1a(can_ch);
							spi_test_2(can_ch);		//kjg_240503
						}
					}
					//spi_test_1b(can_ch);
				}

				if((debug == 1) && (myData->test_val_ll1[12] == 0))	//csk_240507
					myData->CAN.rt_can_control_phase[can_ch] = PH11;
				break;
			case PH21:	//kjg_240503
				spi_test_3(can_ch);
				myData->CAN.rt_can_control_phase[can_ch]++;
				break;
			case PH22:	//kjg_240503
				if((debug == 0) ||(myData->test_val_ll1[12] == 1)) //csk_240507
					myData->CAN.rt_can_control_phase[can_ch] = PH11;
				break;
			case PH31:	//kjg_240503
				if((debug == 0) || (myData->test_val_ll1[12] == 1))	//csk_240507
					myData->CAN.rt_can_control_phase[can_ch] = PH11;
				break;
			case PH50:
				myData->CAN.chip_diff_1tick_count[can_ch]
					= myData->mData.misc.timer_1tick_count;
				myData->CAN.rt_can_control_phase[can_ch]++;
				break;
			case PH51:
				if((myData->mData.misc.timer_1tick_count
					- myData->CAN.chip_diff_1tick_count[can_ch]) >= 50) { //50ms
					myData->CAN.rt_can_control_phase[can_ch] = PH1;
				}
				break;
			default:
				break;
		}
	}

	for(i=0; i < 8; i++) {
		if((myData->mData.misc.timer_1sec - myData->CAN.receive_time[i]) > 2) {
			switch(myData->AppControl.config.systemModel) {
				case C_LGC_500V_250A_125A_50A_500KW_10:	//ktg_210222
				case C_LGC_500V_250A_125A_50A_500KW_11:	//ktg_210222
					if(i == 0) {
						myData->cData[0].
							misc.external_comm_state |= 0x04;
					} else if(i == 1) {
						myData->cData[0].
							misc.external_comm_state |= 0x08;
					} else if(i == 2) {
						myData->cData[2].
							misc.external_comm_state |= 0x04;
					} else if(i == 3) {
						myData->cData[2].
							misc.external_comm_state |= 0x08;
					}
					break;
				default :	
					if(myData->mData.config.division_CAN == 1) {	//shh_220823
						myData->cData[i].misc.external_comm_state |= 0x04;
					} else {
						if((i % 2) == 0) {
							myData->cData[i / 2].misc.external_comm_state
								|= 0x04;
						} else {
							myData->cData[i / 2].misc.external_comm_state
								|= 0x08;
						}	
					}			
					break;
			}	
			/*myData->cData[i / 2].misc.external_comm_state |= 0x04;
			} else {
				myData->cData[i / 2].misc.external_comm_state |= 0x08;
			}*/
		}
	}
}
#endif

void spi_test_1a(int can_ch)
{
	unsigned char tx_count, rx_count, tx_cmd;
	unsigned char kjg_val[16], kjg_val2[4];
	int addr, i, j, k, l;

	tx_count = 3 + 2;
	rx_count = 1;
	tx_cmd = 0x00; //chip command
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	outb(tx_count, addr+1);
	outb(rx_count, addr+2);
	outb(tx_cmd, addr+3);

	outb(0x00, addr+4); //reset
	outb(0x00, addr+4);

	outb(0x00, addr+5); //spi_start

	i = k = l = 0;
	while(1) {
		kjg_val2[0] = inb(addr);
		kjg_val2[1] = inb(addr+2);
		kjg_val2[2] = inb(addr+3) & 0x0F;
		kjg_val2[3] = inb(addr+4);
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[0] == kjg_val2[1]
			&& kjg_val2[2] == 0x02 && kjg_val2[3] == rx_count) {
			break;
		}
		i++;
		if(i > 100) { //84~85
			k = 1;
			break;
		}
	}

	for(j=0; j < 4; j++) {
		kjg_val[j] = inb(addr+1);
	}

	for(j=4; j < (tx_count + rx_count); j++) {
		kjg_val[j] = inb(addr+1);
	}
/*
		printk(KERN_WARNING "a ch%d, %d %d %02x %d, %02x %02X %02x %02x %02x %02x, %d\n",
			can_ch,
			kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3],
			kjg_val[0], kjg_val[1], kjg_val[2],
			kjg_val[3], kjg_val[4], kjg_val[5], i);

		kjg_val2[0] = inb(addr);
		kjg_val2[1] = inb(addr+2);
		kjg_val2[2] = inb(addr+3) & 0x0F;
		kjg_val2[3] = inb(addr+4);
*/
	outb(0x00, addr);

	if(kjg_val[0] == 0x00 && kjg_val[1] == 0x05 && kjg_val[2] == 0x01
		&& kjg_val[3] == 0x00 && kjg_val[4] == 0x00 && kjg_val[5] == 0x00) {
		l = 0;
	} else l = 1;

	if(k == 1 || l == 1) {
		printk(KERN_WARNING "b ch%d, %d %d %02x %d, %02x %02X %02x %02x %02x %02x, %d %d %d\n",
			can_ch,
			kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3],
			kjg_val[0], kjg_val[1], kjg_val[2],
			kjg_val[3], kjg_val[4], kjg_val[5], i, k, l);
	}
}

void spi_test_1b(int can_ch)
{
	unsigned char tx_count, rx_count, tx_cmd;
	unsigned char kjg_val2[4];
	int addr, i, j;

	tx_count = 3 + 2;
	rx_count = 1;
	tx_cmd = 0x00; //chip command
	addr = 0x840 + (0x10 * can_ch);

	switch(myData->test_val_i[can_ch][10]) {
		case 0:
			outb(0x00, addr); //clear

			outb(tx_count, addr+1);
			outb(rx_count, addr+2);
			outb(tx_cmd, addr+3);

			outb(0x00, addr+4); //reset
			outb(0x00, addr+4);

			outb(0x00, addr+5); //spi_start
			myData->test_val_i[can_ch][10]++;
			break;
		case 1:
			kjg_val2[0] = inb(addr);
			kjg_val2[1] = inb(addr+2);
			kjg_val2[2] = inb(addr+3) & 0x0F;
			kjg_val2[3] = inb(addr+4);
			if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[0] == kjg_val2[1]
				&& kjg_val2[2] == 0x02 && kjg_val2[3] == rx_count) {
				for(j=0; j < 4; j++) {
					myData->test_val_i[can_ch][j] = inb(addr+1);
				}

				for(j=4; j < (tx_count + rx_count); j++) {
					myData->test_val_i[can_ch][j] = inb(addr+1);
				}

				outb(0x00, addr);

				if(myData->test_val_i[can_ch][0] == 0x00
					&& myData->test_val_i[can_ch][1] == 0x05
					&& myData->test_val_i[can_ch][2] == 0x01
					&& myData->test_val_i[can_ch][3] == 0x00
					&& myData->test_val_i[can_ch][4] == 0x00
					&& myData->test_val_i[can_ch][5] == 0x00) {
					i = 0;
					if(myData->test_val_i[can_ch][15] == 1) {
						i = 1;
						myData->test_val_i[can_ch][15] = 0;
						myData->test_val_i[can_ch][0] = 1;
					}
				} else {
					i = 1;
				}

				if(i == 1) {
					printk(KERN_WARNING "ch%d, %d %d %02x %d, %02x %02X %02x %02x %02x %02x, %d\n",
						can_ch,
						kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3],
						myData->test_val_i[can_ch][0],
						myData->test_val_i[can_ch][1],
						myData->test_val_i[can_ch][2],
						myData->test_val_i[can_ch][3],
						myData->test_val_i[can_ch][4],
						myData->test_val_i[can_ch][5],
						myData->test_val_i[can_ch][11]);
				}

				myData->test_val_i[can_ch][10] = 0;
			} else {
				myData->test_val_i[can_ch][11]++;
			}
			break;
		default:
			break;
	}
}

void spi_test_2(int can_ch)
{	//kjg_240503
	unsigned char tx_count, rx_count, tx_cmd, tx_len, rx_len;
	unsigned char kjg_val[16], kjg_val2[4];
	int addr, i1, i2, i3, i4, i5;

	tx_len = 0;
	rx_len = 6;
	tx_count = 3 + tx_len;
	rx_count = 1 + rx_len;
	tx_cmd = 0x03; //mcu command
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	outb(tx_count, addr+1);
	outb(rx_count, addr+2);
	outb(tx_cmd, addr+3);

	if(tx_len != 0) { //kjgw_240223
		outb(0x00, addr+4);
	}

	outb(0x00, addr+5); //spi_start

	i1 = i2 = i3 = i4 = i5 = 0;
	while(1) {
		kjg_val2[0] = inb(addr);
		kjg_val2[1] = inb(addr+2);
		kjg_val2[2] = inb(addr+3) & 0x03;
		kjg_val2[3] = inb(addr+4);
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[0] == kjg_val2[1]
			&& kjg_val2[2] == 0x02 && kjg_val2[3] == rx_count) {
			break;
		}
		i1++;
		if(i1 > 100) { //48~51, 56~64
			break;
		}
	}

	for(i2=0; i2 < tx_count; i2++) {
		kjg_val[i2] = inb(addr+1);
	}

	for(i2=tx_count; i2 < (tx_count + rx_count); i2++) {
		kjg_val[i2] = inb(addr+1);
	}

	while(1) {
		kjg_val2[2] = inb(addr+3) & 0x0F;
		if(kjg_val2[2] == 0x0A) break;
		i3++;
		if(i3 > 100) break; //0
	}

	printk(KERN_WARNING "a ch%d %d %d %02x %d, %02x %02X %02x %02x %02x, %02x %02x %02x %02x %02x, %d %d %d %d %d\n",
		can_ch, kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3],
		kjg_val[0], kjg_val[1], kjg_val[2], kjg_val[3], kjg_val[4],
		kjg_val[5], kjg_val[6], kjg_val[7], kjg_val[8], kjg_val[9],
		i1, i2, i3, i4, i5);

	outb(0x00, addr); //clear

	if(kjg_val[0] == 0x00 && kjg_val[1] == 0x03 && kjg_val[2] == 0x07
		&& kjg_val[3] == 0x03 && kjg_val[4] == 0x00 && kjg_val[5] == 0x00
		&& kjg_val[6] == 0x00 && kjg_val[7] == 0x00 && kjg_val[8] == 0x00
		&& kjg_val[9] == 0x00) {
		i4 = 0;
	} else i4 = 1;

	if(i1 == 101 || i3 == 101 || i4 == 1) {
		printk(KERN_WARNING "b ch%d %d %d %02x %d, %02x %02X %02x %02x %02x, %02x %02x %02x %02x %02x, %d %d %d %d %d\n",
			can_ch, kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3],
			kjg_val[0], kjg_val[1], kjg_val[2], kjg_val[3], kjg_val[4],
			kjg_val[5], kjg_val[6], kjg_val[7], kjg_val[8], kjg_val[9],
			i1, i2, i3, i4, i5);
	}
}

void spi_test_3(int can_ch)
{	//kjg_240503
	unsigned char tx_count, rx_count, tx_cmd, tx_len, rx_len;
	unsigned char kjg_val[16], kjg_val2[4];
	int addr, i1, i2, i3, i4, i5;

	tx_len = 2;
	rx_len = 4;
	tx_count = 3 + tx_len;
	rx_count = 1 + rx_len;
	tx_cmd = 0x00; //chip command
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	outb(tx_count, addr+1);
	outb(rx_count, addr+2);
	outb(tx_cmd, addr+3);

	//_CHIP_REG_CiTREC = 0x034
	outb(0x30, addr+4); //read byte
	outb(0x34, addr+4);

	outb(0x00, addr+5); //spi_start

	i1 = i2 = i3 = i4 = i5 = 0;
	while(1) {
		kjg_val2[0] = inb(addr);
		kjg_val2[1] = inb(addr+2);
		kjg_val2[2] = inb(addr+3) & 0x03;
		kjg_val2[3] = inb(addr+4);
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[0] == kjg_val2[1]
			&& kjg_val2[2] == 0x02 && kjg_val2[3] == rx_count) {
			break;
		}
		i1++;
		if(i1 > 200) { //84~92
			break;
		}
	}

	for(i2=0; i2 < tx_count; i2++) {
		kjg_val[i2] = inb(addr+1);
	}

	for(i2=tx_count; i2 < (tx_count + rx_count); i2++) {
		kjg_val[i2] = inb(addr+1);
	}

	while(1) {
		kjg_val2[2] = inb(addr+3) & 0x0F;
		if(kjg_val2[2] == 0x0A) break;
		i3++;
		if(i3 > 200) break; //0
	}

	printk(KERN_WARNING "a ch%d %d %d %02x %d, %02x %02X %02x %02x %02x, %02x %02x %02x %02x %02x, %d %d %d %d %d\n",
		can_ch, kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3],
		kjg_val[0], kjg_val[1], kjg_val[2], kjg_val[3], kjg_val[4],
		kjg_val[5], kjg_val[6], kjg_val[7], kjg_val[8], kjg_val[9],
		i1, i2, i3, i4, i5);

	outb(0x00, addr); //clear

	if(kjg_val[0] == 0x00 && kjg_val[1] == 0x05 && kjg_val[2] == 0x05
		&& kjg_val[3] == 0x00 && kjg_val[4] == 0x30 && kjg_val[5] == 0x34
		&& kjg_val[6] == 0x00 && kjg_val[7] == 0x00 && kjg_val[8] == 0x00
		&& kjg_val[9] == 0x00) {
		i4 = 0;
	} else i4 = 1;

	if(i1 == 201 || i3 == 201 || i4 == 1) {
		printk(KERN_WARNING "b ch%d %d %d %02x %d, %02x %02X %02x %02x %02x, %02x %02x %02x %02x %02x, %d %d %d %d %d\n",
			can_ch, kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3],
			kjg_val[0], kjg_val[1], kjg_val[2], kjg_val[3], kjg_val[4],
			kjg_val[5], kjg_val[6], kjg_val[7], kjg_val[8], kjg_val[9],
			i1, i2, i3, i4, i5);
	}
}
