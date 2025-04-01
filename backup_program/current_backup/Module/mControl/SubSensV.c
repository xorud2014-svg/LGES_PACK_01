#include <rtl_core.h>
#include <pthread.h>
#include <asm/io.h>
#include <asm/dma.h>
#include "../../INC/datastore.h"
#include "local_utils.h"
#include "SubSensV.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;
extern unsigned char *dma_buff;
unsigned char buff[MAX_SUB_SENS_V_BUFF]; //kjhw_140528

void SubSensV_adc_start(void)
{
	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1) return;
	if(myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0	 //kjh_160610
		&& myPs->config.installedHumidity == 0
		&& myPs->config.installedGas == 0) return;	//sec_220926
	if(myPs->misc.timer_1sec < 3) return;

	if(myData->SubSensV.phase != P0) return;
	myData->SubSensV.phase = P1;
	myData->SubSensV.delay_count = 0;

	switch(myPs->config.daq_type) {
		case DAQ_TYPE1:
		case DAQ_TYPE2:
			outb(0x00, 0x710);
			break;
		case DAQ_TYPE3:
		case DAQ_TYPE4:
		case DAQ_TYPE5:
		case DAQ_TYPE6: //lki_111010
			if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P0) {
				outb(0x00, 0x710);
			} else {
				outb(0x01, 0x710);
			}
			if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION] == P0) {
				outb(0x01, 0x711);
			} else {
				outb(0x00, 0x711);
			}
			break;
		case DAQ_TYPE7: //lki_111010
			if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P0) {
				outb(0x01, 0x710);
			} else {
				outb(0x00, 0x710);
			}
			if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION] == P0) {
				outb(0x01, 0x711);
			} else {
				outb(0x00, 0x711);
			}
			break;
		default:	break;
	}
}

void SubSensV_Control1(int phase)
{
	int dma_size;
	//unsigned long flags=0;

	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1) return;
	if(myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0
		&& myPs->config.installedHumidity == 0	//kjh_211021
		&& myPs->config.installedGas == 0) return;	//sec_220926

	if(myPs->config.daq_type == DAQ_TYPE1) {
		dma_size = sizeof(S_SUB_SENS_V_ORG1);
	} else {
		dma_size = sizeof(S_SUB_SENS_V_ORG2);
	}

	if(phase == 0) {
		if(myData->SubSensV.phase != P1) return;
		myData->SubSensV.delay_count++;
		//if(myData->SubSensV.delay_count <= 2) return;
		if(myData->SubSensV.delay_count <= 4) return;
		myData->SubSensV.phase = P2;
		myData->SubSensV.delay_count = 0;

		if(myPs->config.daq_type == DAQ_TYPE1) {
			int i, j, k;
			S_SUB_SENS_V_ORG1	org_data1;

			memcpy((char *)&org_data1, dma_buff, dma_size);

			memcpy((char *)&myData->SubSensV.org_data.zero_val,
				(char *)&org_data1.zero_val, SUB_SENS_V_ZERO_VAL);
			for(i = 0; i < MAX_SUB_SENS_V_REF_INDEX; i++) {
				for(j = 0; j < MAX_SUB_SENS_V_AD_SLOT; j++) {
					for(k = 0; j < MAX_SUB_SENS_V_ADC_INDEX_1; j++) {
						myData->SubSensV.org_data.ref_val[i][j][k]
							= org_data1.ref_val[i][j][k];
					}
				}
			}
			for(i = 0; i < MAX_SUB_SENS_V_REF_INDEX; i++) {
				for(j = 0; j < MAX_SUB_SENS_V_AD_SLOT; j++) {
					for(k = 0; j < MAX_SUB_SENS_V_ADC_INDEX_1; j++) {
						myData->SubSensV.org_data.sub_val[i][j][k]
							= org_data1.sub_val[i][j][k];
					}
				}
			}
			memcpy((char *)&myData->SubSensV.org_data.equal_val,
				(char *)&org_data1.equal_val, 2);
		} else {
			memcpy((char *)&myData->SubSensV.org_data, dma_buff, dma_size);
		}
	} else {
		if(myData->SubSensV.phase != P2) return;
		//myData->SubSensV.delay_count++;
		//if(myData->SubSensV.delay_count <= 2) return;
		myData->SubSensV.phase = P3;
		myData->SubSensV.delay_count = 0;

		//flags = claim_dma_lock();
		disable_dma(1);
		clear_dma_ff(1);
		set_dma_mode(1, DMA_MODE_READ);
		set_dma_addr(1, virt_to_bus(dma_buff));
		//kjg_d (long)myData-> = virt_to_bus(dma_buff);
		set_dma_count(1, dma_size);
		outb(0x01, 0x720); //dma request
		enable_dma(1);
		//release_dma_lock(flags);
	}
}
/*
void SubSensV_No_DMA_Read(int phase)
{ //lki_111010
	int i, j, k;

	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1) return;
	if(myPs->config.installedAuxV == 0) return;

	if(phase == 0) {
		if(myData->SubSensV.phase != P1) return;
		myData->SubSensV.delay_count++;
		//if(myData->SubSensV.delay_count <= 2) return;
		if(myData->SubSensV.delay_count <= 4) return;
		//if(myData->SubSensV.delay_count <= 6) return;
		//if(myData->SubSensV.delay_count <= 8) return;
		//if(myData->SubSensV.delay_count <= 10) return;
		myData->SubSensV.phase = P2;
		myData->SubSensV.delay_count = 0;

		for(i = 0; i < SUB_SENS_V_ZERO_VAL; i++) {
			myData->SubSensV.org_data.zero_val[i] = inb(0x710);
		}

		for(i = 0; i < MAX_SUB_SENS_V_REF_INDEX; i++) {
			for(j = 0; j < MAX_SUB_SENS_V_AD_SLOT; j++) {
				for(k = 0; k < MAX_SUB_SENS_V_ADC_INDEX; k++) {
					myData->SubSensV.org_data.ref_val[i][j][k].byte[0]
						= inb(0x710);
					myData->SubSensV.org_data.ref_val[i][j][k].byte[1]
						= inb(0x710);
				}
			}
		}

		for(i = 0; i < MAX_SUB_SENS_V_CH_HALF_INDEX; i++) {
			for(j = 0; j < MAX_SUB_SENS_V_AD_SLOT; j++) {
				for(k = 0; k < MAX_SUB_SENS_V_ADC_INDEX; k++) {
					myData->SubSensV.org_data.sub_val[i][j][k].byte[0]
						= inb(0x710);
					myData->SubSensV.org_data.sub_val[i][j][k].byte[1]
						= inb(0x710);
				}
			}
		}

	} else {
		if(myData->SubSensV.phase != P2) return;
		//myData->SubSensV.delay_count++;
		//if(myData->SubSensV.delay_count <= 2) return;
		myData->SubSensV.phase = P3;
		myData->SubSensV.delay_count = 0;

		for(i = MAX_SUB_SENS_V_CH_HALF_INDEX;
			i < MAX_SUB_SENS_V_CH_INDEX; i++) {
			for(j = 0; j < MAX_SUB_SENS_V_AD_SLOT; j++) {
				for(k = 0; k < MAX_SUB_SENS_V_ADC_INDEX; k++) {
					myData->SubSensV.org_data.sub_val[i][j][k].byte[0]
						= inb(0x710);
					myData->SubSensV.org_data.sub_val[i][j][k].byte[1]
						= inb(0x710);
				}
			}
		}

		for(i = 0; i < 2; i++) {
			myData->SubSensV.org_data.equal_val[i] = inb(0x710);
		}
	}
}*/

//kjhw_140528s
void SubSensV_No_DMA_Read(int phase)
{
	int i;
	int dma_size;

	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1) return;
	if(myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0
		&& myPs->config.installedHumidity == 0//) return;	//kjh_211021
		&& myPs->config.installedGas == 0) return;	//sec_220926

	dma_size = sizeof(S_SUB_SENS_V_ORG2);

	if(phase == 0) {
		if(myData->SubSensV.phase != P1) return;
		myData->SubSensV.delay_count++;
		//if(myData->SubSensV.delay_count <= 2) return;
		if(myData->SubSensV.delay_count <= 4) return;
		myData->SubSensV.phase = P2;
		myData->SubSensV.delay_count = 0;

		for(i = 0; i < MAX_SUB_SENS_V_BUFF; i++) {
			//myData->SubSensV.buff[i] = inb(0x710);
			buff[i] = inb(0x710);
		}
	} else {
		if(myData->SubSensV.phase != P2) return;
		//myData->SubSensV.delay_count++;
		//if(myData->SubSensV.delay_count <= 2) return;
		myData->SubSensV.phase = P3;
		myData->SubSensV.delay_count = 0;

		//memcpy((char *)&myData->SubSensV.org_data, myData->SubSensV.buff
		//	, dma_size);
		memcpy((char *)&myData->SubSensV.org_data, buff, dma_size);
	}
}
//kjhw_140528e

void SubSensV_Control2(int phase)
{
	int max_adc_index;

	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1) return;
	if(myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0
		&& myPs->config.installedHumidity == 0//) return;	//kjh_211021
		&& myPs->config.installedGas == 0) return;	//sec_220926

	switch(myPs->config.daq_type) {
		case DAQ_TYPE1:
			max_adc_index = MAX_SUB_SENS_V_ADC_INDEX_1;
			break;
		default:
			max_adc_index = MAX_SUB_SENS_V_ADC_INDEX_2;
			break;
	}

	if(phase == 0) {
		switch(myPs->config.daq_type) {
			case DAQ_TYPE6: //lki_111010
			case DAQ_TYPE7:
				break;
			default:
				if(get_dma_residue(1) != 0) return;
				break;
		}

		if(myData->SubSensV.phase != P3) return;
		//myData->SubSensV.delay_count++;
		//if(myData->SubSensV.delay_count <= 2) return;
		myData->SubSensV.phase = P4;
		myData->SubSensV.delay_count = 0;

		Cal_SubSensV_Source1(max_adc_index);
		Cal_SubSensV_Source2(max_adc_index);
	} else {
		if(myData->AppControl.config.debugType == 110
			|| myData->AppControl.config.debugType == 111) {
		} else {
			if(myData->SubSensV.phase != P4) return;
		}
		//myData->SubSensV.delay_count++;
		//if(myData->SubSensV.delay_count <= 2) return;
		myData->SubSensV.phase = P0;
		myData->SubSensV.delay_count = 0;

		Cal_SubSensV_Ch(max_adc_index);

		switch(myPs->config.daq_type) {
			case DAQ_TYPE3:
				if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P0) {
					myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] = P1;
				} else {
					myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] = P0;
					sub_sens_v_count_increment();
				}
				break;
			case DAQ_TYPE5:
			case DAQ_TYPE7: //lki_111010
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION]++;
				if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] >= P2) {
					myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] = P0;
					sub_sens_v_count_increment();
				}
				break;
			default:
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] = P0;
				sub_sens_v_count_increment();
				break;
		}
	}
}

void Cal_SubSensV_Source1(int max_adc_index)
{
	short int tmp;
	int sensCount, end, ref_index, ad_slot, adc_index;
	long sum;
	double ratio;
	S_SUB_SENS_V_SOURCE_DIV *mySubSensSource;

	if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P0) {
		mySubSensSource = &(myData->SubSensV.source_div1);
	//} else {
	//	mySubSensSource = &(myData->SubSensV.source_div2);
	} else if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P1) {	//csk_191016s
		mySubSensSource = &(myData->SubSensV.source_div2);
	} else if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P2) {
		mySubSensSource = &(myData->SubSensV.source_div3);
	} else {
		mySubSensSource = &(myData->SubSensV.source_div4);					//csk_191016e
	}

	sensCount = myPs->misc.SubSensV_SourceSensCount;
	if(myPs->misc.SubSensV_SourceSensCountFlag < P1) {
		end = sensCount + 1;
	} else {
		end = MAX_SUB_SENS_V_SOURCE_SENS_COUNT;
	}

	for(ref_index=0; ref_index < MAX_SUB_SENS_V_REF_INDEX; ref_index++) {
		for(ad_slot=0; ad_slot < MAX_SUB_SENS_V_AD_SLOT; ad_slot++) {
			for(adc_index=0; adc_index < max_adc_index; adc_index++) {
				tmp	= myData->SubSensV.org_data
					.ref_val[ref_index][ad_slot][adc_index].val;
				mySubSensSource->source[ad_slot][adc_index]
					.tmpV[ref_index][sensCount] = tmp;
			}
		}
	}

	for(ad_slot=0; ad_slot < MAX_SUB_SENS_V_AD_SLOT; ad_slot++) {
		for(adc_index=0; adc_index < max_adc_index; adc_index++) {
			for(ref_index=0; ref_index < MAX_SUB_SENS_V_REF_INDEX;
				ref_index++) {
				sum = 0;
				for(sensCount=0; sensCount < end; sensCount++) {
					sum += (long)mySubSensSource->source[ad_slot][adc_index]
						.tmpV[ref_index][sensCount];
				}

				//10000000uV / 32768 = 305.1757813uV
				switch(myData->AppControl.config.systemModel) {
					case C_LGC_5V_150A_10A:
					case C_LGC_5V_600A_10A:
					case C_VK_100V_200A_20A:
					case C_EIG_100V_200A_20A_2:
						//AMP -> INA128UA
						//ratio = 305.1757813uV / 1.5
						ratio = 203.4505209;
						break;
					case C_SEBANG_200V_100A_10A:
					case C_LGC_450V_200A_10A_4:
					case C_LGC_450V_200A_10A_5:
						//AMP -> INA121U
						//ratio = 305.1757813uV / 1.5
						ratio = 203.4505209;
						break;
					default:
						//AMP -> AD620A
						//ratio = 305.1757813uV / 1.494
						ratio = 204.2675912;
						break;
				}
				mySubSensSource->source[ad_slot][adc_index].sensV[ref_index]
					= (long)((double)sum * ratio / end);
			}
		}
	}
}

void Cal_SubSensV_Source2(int max_adc_index)
{
	int ref_index, ad_slot, adc_index;
	double AD_a, AD_b, val1;
	S_SUB_SENS_V_SOURCE_DIV *mySubSensSource;

	if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P0) {
		mySubSensSource = &(myData->SubSensV.source_div1);
	//} else {
	//	mySubSensSource = &(myData->SubSensV.source_div2);
	} else if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P1) {	//csk_191015s
		mySubSensSource = &(myData->SubSensV.source_div2);
	} else if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P2) {
		mySubSensSource = &(myData->SubSensV.source_div3);
	} else {
		mySubSensSource = &(myData->SubSensV.source_div4);					//csk_191015e
	}

	for(ad_slot=0; ad_slot < MAX_SUB_SENS_V_AD_SLOT; ad_slot++) {
		for(adc_index=0; adc_index < max_adc_index; adc_index++) {
			val1 = 5000000; //5000mV

			AD_a = (double)(val1 - 0.0)
				/ (double)(mySubSensSource->source[ad_slot][adc_index].sensV[0]
				- mySubSensSource->source[ad_slot][adc_index].sensV[2]);
			AD_b = val1 - (double)mySubSensSource->
				source[ad_slot][adc_index].sensV[0] * AD_a;
			mySubSensSource->source[ad_slot][adc_index].V_AD_a = AD_a;
			mySubSensSource->source[ad_slot][adc_index].V_AD_b = AD_b;

			AD_a = (double)(0.0 - (-val1))
				/ (double)(mySubSensSource->source[ad_slot][adc_index].sensV[2]
				- mySubSensSource->source[ad_slot][adc_index].sensV[1]);
			AD_b = (double)(0.0) - (double)mySubSensSource->
				source[ad_slot][adc_index].sensV[2] * AD_a;
			mySubSensSource->source[ad_slot][adc_index].V_AD_a_N = AD_a;
			mySubSensSource->source[ad_slot][adc_index].V_AD_b_N = AD_b;
//kjhw_140620s //test
/*
			val1 = 5092186; //5092mV

			AD_a = (double)(val1 - 0.0)
				/ (double)(mySubSensSource->source[ad_slot][adc_index].sensV[0]
				- (mySubSensSource->source[ad_slot][adc_index].sensV[2]
				- mySubSensSource->source[ad_slot][adc_index].sensV[2]));
			AD_b = val1 - (double)mySubSensSource->
				source[ad_slot][adc_index].sensV[0] * AD_a;
			mySubSensSource->source[ad_slot][adc_index].V_AD_a = AD_a;
			mySubSensSource->source[ad_slot][adc_index].V_AD_b = AD_b;

			AD_a = (double)(0.0 - (-val1))
				/ (double)(mySubSensSource->source[ad_slot][adc_index].sensV[2]
				- mySubSensSource->source[ad_slot][adc_index].sensV[2])
				- (mySubSensSource->source[ad_slot][adc_index].sensV[1] * (-1));
			AD_b = (double)(0.0)
				- (double)(mySubSensSource->source[ad_slot][adc_index].sensV[2]
				* (-1)) * AD_a;
			mySubSensSource->source[ad_slot][adc_index].V_AD_a_N = AD_a;
			mySubSensSource->source[ad_slot][adc_index].V_AD_b_N = AD_b;
*/
//kjhw_140620e //test

			for(ref_index=0; ref_index < MAX_SUB_SENS_V_REF_INDEX;
				ref_index++) {
				val1 = mySubSensSource->source[ad_slot][adc_index]
					.sensV[ref_index];
				if(val1 >= 0) {
					mySubSensSource->source[ad_slot][adc_index].calV[ref_index]
						= (long)((double)val1
						* mySubSensSource->source[ad_slot][adc_index].V_AD_a
						+ mySubSensSource->source[ad_slot][adc_index].V_AD_b);
				} else {
					mySubSensSource->source[ad_slot][adc_index].calV[ref_index]
						= (long)((double)val1
						* mySubSensSource->source[ad_slot][adc_index].V_AD_a_N
						+ mySubSensSource->source[ad_slot][adc_index].V_AD_b_N);
				}
			}
		}
	}
}

void Cal_SubSensV_Ch(int max_adc_index)
{
	short int th_type;
	short int installedTemp, installedAuxV, installedTH, installedHumidity, installedGas;//csk_200113 sec_220926
	int sensCount, end, ch_index, ad_slot, adc_index, data_index, data_offset;
	int monitor_ch; //jhkw_201117
	int filter_sensCount;	//jhkw_151028
//	int ch, k, j;
	long tmp;
	double ratio, sum;
	S_SUB_SENS_V_SOURCE_DIV *mySubSensSource;
	
	installedTemp = myData->mData.config.installedTemp;		//csk_200113s
	installedAuxV = myData->mData.config.installedAuxV;
	installedTH = myData->mData.config.installedTH;
	installedHumidity = myData->mData.config.installedHumidity;
	installedGas = myData->mData.config.installedGas;	//sec_220926
	//csk_2001113e

	if(myData->AppControl.config.debugType == 110
		|| myData->AppControl.config.debugType == 111) {
		mySubSensSource = &(myData->SubSensV.source_div1);
		data_offset = 0;
		sum = 0.0;

		for(ch_index=0; ch_index < MAX_SUB_SENS_V_CH_INDEX; ch_index++) {
			for(ad_slot=0; ad_slot < MAX_SUB_SENS_V_AD_SLOT; ad_slot++) {
				for(adc_index=0; adc_index < max_adc_index; adc_index++) {
					data_index = ch_index + ad_slot * MAX_SUB_SENS_V_CH_INDEX
						+ (adc_index * MAX_SUB_SENS_V_CH_INDEX
						* MAX_SUB_SENS_V_AD_SLOT) + data_offset;
					if(myData->SubSensV.ch[data_index].sensV <= 5000000) {
						myData->SubSensV.ch[data_index].sensV += 100000;
					} else myData->SubSensV.ch[data_index].sensV = 100000;
				}
			}
		}
		return;
	}

	if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P0) {
		mySubSensSource = &(myData->SubSensV.source_div1);
		data_offset = 0;
	//} else {
	//	mySubSensSource = &(myData->SubSensV.source_div2);
	//	data_offset = 128;
	} else if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P1) {	//csk_191015s
		mySubSensSource = &(myData->SubSensV.source_div2);
		data_offset = 128;
	} else if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P2) {
		mySubSensSource = &(myData->SubSensV.source_div3);
		data_offset = 256;
	} else {
		mySubSensSource = &(myData->SubSensV.source_div4);
		data_offset = 384;													//csk_191015e
	}

	sensCount = myPs->misc.SubSensV_ChSensCount;
	if(myPs->misc.SubSensV_ChSensCountFlag < P1) {
		end = sensCount + 1;
	} else {
		end = MAX_SUB_SENS_V_FILTER_AD_COUNT;
	}

	for(ch_index=0; ch_index < MAX_SUB_SENS_V_CH_INDEX; ch_index++) {
		for(ad_slot=0; ad_slot < MAX_SUB_SENS_V_AD_SLOT; ad_slot++) {
			for(adc_index=0; adc_index < max_adc_index; adc_index++) {
				tmp = (long)myData->SubSensV.org_data
					.sub_val[ch_index][ad_slot][adc_index].val;

				//10000000uV / 32768 = 305.1757813uV
				switch(myData->AppControl.config.systemModel) {
					case C_LGC_5V_150A_10A:
					case C_LGC_5V_600A_10A:
					case C_VK_100V_200A_20A:
					case C_EIG_100V_200A_20A_2:
						//AMP -> INA128UA
						//ratio = 305.1757813uV / 1.5
						ratio = 203.4505209;
						break;
					case C_SEBANG_200V_100A_10A:
					case C_LGC_450V_200A_10A_4:
					case C_LGC_450V_200A_10A_5:
						//AMP -> INA121U
						//ratio = 305.1757813uV / 1.5
						ratio = 203.4505209;
						break;
					default:
						//AMP -> AD620A
						//ratio = 305.1757813uV / 1.494
						ratio = 204.2675912;
						break;
				}

				if(tmp >= 0) {
					tmp = (long)((double)tmp * ratio
						* mySubSensSource->source[ad_slot][adc_index].V_AD_a
						+ mySubSensSource->source[ad_slot][adc_index].V_AD_b);
				} else {
					tmp = (long)((double)tmp * ratio
						* mySubSensSource->source[ad_slot][adc_index].V_AD_a_N
						+ mySubSensSource->source[ad_slot][adc_index].V_AD_b_N);
				}

				data_index = ch_index + ad_slot * MAX_SUB_SENS_V_CH_INDEX
					+ (adc_index * MAX_SUB_SENS_V_CH_INDEX
					* MAX_SUB_SENS_V_AD_SLOT) + data_offset;

				myData->SubSensV.ch[data_index].tmpV[sensCount] = tmp;
				myData->SubSensV.ch[data_index].tmpSensV = tmp;
				//jhkw_151028s
				if(sensCount == 0) {
					filter_sensCount = 3;
				} else {
					filter_sensCount = sensCount - 1;
				}
				if(myData->SubSensV.ch[data_index].tmpV[sensCount]
					- myData->SubSensV.ch[data_index]
					.tmpV[filter_sensCount] >= 500000
					|| myData->SubSensV.ch[data_index].tmpV[sensCount]
					- myData->SubSensV.ch[data_index]
					.tmpV[filter_sensCount] <= -500000) {
					myData->SubSensV.ch[data_index].fault_count++;
					if(myData->SubSensV.ch[data_index].fault_count <= 5){
						myData->SubSensV.ch[data_index].tmpV[sensCount]
							= myData->SubSensV.ch[data_index]
							.tmpV[filter_sensCount];
					} else {
						myData->SubSensV.ch[data_index].fault_count = 0;
					}
				} else {
					myData->SubSensV.ch[data_index].fault_count = 0;
				}
				//jhkw_151028e
			}
		}
	}

	for(ch_index=0; ch_index < MAX_SUB_SENS_V_CH_INDEX; ch_index++) {
		for(ad_slot=0; ad_slot < MAX_SUB_SENS_V_AD_SLOT; ad_slot++) {
			for(adc_index=0; adc_index < max_adc_index; adc_index++) {
				data_index = ch_index + ad_slot * MAX_SUB_SENS_V_CH_INDEX
					+ (adc_index * MAX_SUB_SENS_V_CH_INDEX
					* MAX_SUB_SENS_V_AD_SLOT) + data_offset;
				sum = 0.0;
				for(sensCount=0; sensCount < end; sensCount++) {
					sum += (double)myData->SubSensV.ch[data_index]
						.tmpV[sensCount];
				}

				sum /= (double)end;
				sum *= 1.0025072;	//shh_220404
				//jhkw_201117s4
				if(myData->AppControl.config.debugType == 220) {
					sum = (data_index+1) * 100000;
				}
				monitor_ch = myData->aux_ch_num[data_index].monitor_ch;
				monitor_ch += myData->mData.config.installedTemp;
				switch(myData->aux_ch_num[data_index].type) {
					case 2:
						th_type = myData->auxSetData[monitor_ch].tableNo;
						sum = (double)Convert_Vth_to_Temp((float)sum, th_type);
						//sum = sum * (100 * th_type);
						break;
					case 3:
						th_type = myData->auxSetData[monitor_ch].tableNo;
						sum = (double)Convert_Vwet_to_Humidity((float)sum, th_type);
						break;
					case 10:
						break;
					case 11:
						sum = sum  * 2;
						break;
					case 12:
						sum = sum  * 3;
						break;
					case 13:
						sum = sum  * 4;
						break;
					case 14:
						sum = sum  * 6;
						break;
					case 15:
						sum = sum * 4.402136 + (-132114.1);
						break;
					case 16:
						sum = sum * 4.438871 + (-59974.75);
						break;
					case 17:
						sum = sum * 8.33333333;
						break;
					default:
						break;
				}
				//jhkw_201117e
				myData->SubSensV.ch[data_index].sensV = (long)sum;
				/*if(data_index <8) {	//ktg_ttttttttttttttttttt
					myData->SubSensV.ch[data_index].sensV = 3000000;
				} else if(data_index <16) {
					myData->SubSensV.ch[data_index].sensV = 3100000;
				} else if(data_index <24) {
					myData->SubSensV.ch[data_index].sensV = 3200000;
				} else if(data_index <32) {
					myData->SubSensV.ch[data_index].sensV = 3300000;
				} else if(data_index <40) {
					myData->SubSensV.ch[data_index].sensV = 3400000;
				} else if(data_index <48) {
					myData->SubSensV.ch[data_index].sensV = 3500000;
				} else if(data_index <56) {
					myData->SubSensV.ch[data_index].sensV = 3600000;
				} else if(data_index <64) {
					myData->SubSensV.ch[data_index].sensV = 3700000;
				} else if(data_index <72) {
					myData->SubSensV.ch[data_index].sensV = 3800000;
				} else if(data_index <80) {
					myData->SubSensV.ch[data_index].sensV = 3900000;
				} else if(data_index <88) {
					myData->SubSensV.ch[data_index].sensV = 4000000;
				} else if(data_index <96) {
					myData->SubSensV.ch[data_index].sensV = 4100000;
				}*/
				/*if(myData->test_val_l1[6] == 1) {
					if(myData->SubSensV.ch[data_index].sensV >= 4000000) {
						myData->SubSensV.ch[data_index].sensV = 1000000;
					}
					myData->SubSensV.ch[data_index].sensV += 1000;
				} else {
					if(myData->SubSensV.ch[data_index].sensV <= 500000) {
						myData->SubSensV.ch[data_index].sensV = 3500000;
					}
					if(myData->SubSensV.ch[data_index].sensV >= 4000000) {
						myData->SubSensV.ch[data_index].sensV = 1000000;
					}
					myData->SubSensV.ch[data_index].sensV += 2000;
				}*/
			}
		}
	}
}

float Convert_Vth_to_Temp(float Vth, int type)
{
	int table, i;
	float V_TH, TEMP, ret_temp;

	Vth = Vth / 1000.0;
	Vth /= 1.0025072;	//shh_220404

	table=0;
	for(table=0; table < MAX_TH_TABLE; table++) {
		if((int)myData->th_table[table].th_type == type) {
			break;
		}
	}
	ret_temp = 999999.0; //999.999degreeC
	if(table == MAX_TH_TABLE) {
		return ret_temp * 1000.0;
	}
	//kjh_160610s
	if(type == 0) {
		ret_temp = 123456.0; //123.456degreeC
		return ret_temp * 1000.0;
	}
	//kjh_160610e

	for(i=0; i < MAX_TH_DATA; i++) {
		if(myData->th_table[table].T_R[i+1][0] >= 999000.0) {
			ret_temp = 999000.0;
			break;
		}

		V_TH = myData->th_table[table].V_TH[i];
		TEMP = myData->th_table[table].T_R[i][0];

		/*if(Vth > V_TH) {
			ret_temp = -999999.0;
			break;
		} else if(Vth > myData->th_table[table].V_TH[i+1]) {
			ret_temp = TEMP -
				((V_TH - Vth) / (V_TH - myData->th_table[table].V_TH[i+1]))
				* (TEMP - myData->th_table[table].T_R[i+1][0]);
			break;
		}*/
		if(myData->th_table[table].type == 1) { //NTC //kjh_210317s
			if(Vth > V_TH) {
				ret_temp = -999999.0;
				break;
			} else if(Vth > myData->th_table[table].V_TH[i+1]) {
				ret_temp = TEMP -
					((V_TH - Vth) / (V_TH - myData->th_table[table].V_TH[i+1]))
					* (TEMP - myData->th_table[table].T_R[i+1][0]);
				break;
			}
		} else if(myData->th_table[table].type == 2) { //PTC
			if(Vth < V_TH) {
				ret_temp = -999999.0;
				break;
			} else if(Vth < myData->th_table[table].V_TH[i+1]) {
				ret_temp = TEMP -
					((Vth - V_TH) / (myData->th_table[table].V_TH[i+1] - V_TH))
					* (TEMP - myData->th_table[table].T_R[i+1][0]);
				break;
			}
		} else break; //Non //kjh_210317e
	}
	if(i == MAX_TH_DATA) ret_temp = 998000.0;

	return ret_temp * 1000.0;
}

float Convert_Vth_to_Ohm(float Vth, int type)
{ //lki_110821
	int table, i;
	float ret_temp;

	Vth = Vth / 1000.0;

	table=0;
	for(table=0; table < MAX_TH_TABLE; table++) {
		if((int)myData->th_table[table].th_type == type) {
			break;
		}
	}
	ret_temp = 999999.0; //999.999degreeC
	if(table == MAX_TH_TABLE) {
		return ret_temp * 1000.0;
	}

	if(Vth > myData->th_table[table].V_TH[0]) {
		ret_temp = -999999.0;
		return ret_temp * 1000.0;
	}
	i = (int)myData->th_table[table].th_data_max_index;
	if(Vth < myData->th_table[table].V_TH[i]) {
		ret_temp = 999000.0;
		return ret_temp * 1000.0;
	}

	ret_temp = (Vth * (myData->th_table[table].R1 + myData->th_table[table].R2)
		/ (myData->th_table[table].Vref - Vth));

	ret_temp = (390000 * ret_temp) / (390000 - ret_temp);

	return ret_temp * 1000.0;
}

//khj_191205
float Convert_Vwet_to_Humidity(float Vwet, int type)
{
	int table, i;
	float V_Humidity_1, V_Humidity_2, Humidity1, Humidity2, ret_humidity;

	//Vwet = Vwet / 1000.0;
	Vwet /= 1.0025072;	//shh_220404

	table=0;
	for(table=0; table < MAX_HUMIDITY_TABLE; table++) {
		if((int)myData->humidity_table[table].humidity_type == type) {
			break;
		}
	}
	ret_humidity = 999.999; //999.999%
	if(table == MAX_HUMIDITY_TABLE) {
		return ret_humidity * 1000.0;
	}
	//kjh_160610s
	if(type == 0) {
		ret_humidity = 654.321; //654.321%
		return ret_humidity; // * 1000.0;
	}
	//kjh_160610e

	for(i=0; i < MAX_HUMIDITY_DATA; i++) {
		if(myData->humidity_table[table].H_V[i+1][0] >= 999000.0) {
			ret_humidity = 999000.0;
			break;
		}

		V_Humidity_1 = myData->humidity_table[table].H_V[i][1];
		V_Humidity_2 = myData->humidity_table[table].H_V[i+1][1];
		Humidity1 = myData->humidity_table[table].H_V[i][0];
		Humidity2 = myData->humidity_table[table].H_V[i+1][0];

		if(Vwet < V_Humidity_1) { 	// < : asceding order, > : deasceding order
			ret_humidity = -999.999;
			break;
		} else if(Vwet < V_Humidity_2) {	// < : asceding order, > : deasceding order
			ret_humidity = ((Humidity2 - Humidity1) / (V_Humidity_2 - V_Humidity_1))
					* (Vwet - V_Humidity_1);
			ret_humidity = ret_humidity + Humidity1;

			break;
		}
	}
	if(i == MAX_HUMIDITY_DATA) ret_humidity = 998000.0;

	return ret_humidity * 1000.0;
}	//khj_191205e

void SubSensV_process(void)
{ //kjg_150225
	unsigned char state;
	int i, j, k;
	//unsigned long flags=0;

	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1)  return;
	//if(myPs->config.installedAuxV == 0
	//	&& myPs->config.installedTH == 0) return; //kjh_160610

	if(myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0
		&& myPs->config.installedHumidity == 0//) return; //csk_200113
		&& myPs->config.installedGas == 0) return; //sec_220926

	if(myPs->misc.timer_1sec < 3) return;

	switch(myData->SubSensV.phase) {
		case P0:
			//outb(0x03, 0x710); //daq1, daq2 board mux start
			outb(0x0F, 0x710); //daq1, daq2, daq3, daq4 board mux start	//csk_191015
			if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION] == P0) {
				//outb(0x03, 0x711);
				outb(0x0F, 0x711);	//csk_191015
			} else {
				outb(0x00, 0x711);
			}
			myData->SubSensV.delay_count = 0;
			myData->SubSensV.phase++;
			break;
		case P1:
			if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P0) {
				state = inb(0x701);
				if(state == 0) { //daq1 mux end
					outb(0x01, 0x712); //daq1 fifo read enable
					myData->SubSensV.delay_count = 0;
					myData->SubSensV.phase++;
				} else {
					myData->SubSensV.delay_count++;
					if(myData->SubSensV.delay_count > 100) { //100ms
						if(myData->SubSensV.check_count < 100) {
							//kjg_180522 rtl_printf("daq mux do not end %d\n",
							//	myData->SubSensV
							//	.signal[SUB_SENS_SIG_DAQ_DIVISION]);
						}
						myData->SubSensV.delay_count = 0;
						if(myData->AppControl.config.debugType == 220) {
							myData->SubSensV.phase = P2;
						} else {
							myData->SubSensV.phase = P10;
							//myData->SubSensV.phase = P2;	//ktg_tttttttttttttttt
						}
					}
				}
			/*		//csk_191015s_r
			} else {
				state = inb(0x702);
				if(state == 0) { //daq2 mux end
					outb(0x02, 0x712); //daq2 fifo read enable
					myData->SubSensV.delay_count = 0;
					myData->SubSensV.phase++;
				} else {
					myData->SubSensV.delay_count++;
					if(myData->SubSensV.delay_count > 100) { //100ms
						if(myData->SubSensV.check_count < 100) {
							//kjg_180522 rtl_printf("daq mux do not end %d\n",
							//	myData->SubSensV
							//	.signal[SUB_SENS_SIG_DAQ_DIVISION]);
						}
						myData->SubSensV.delay_count = 0;
						myData->SubSensV.phase = P10;
					}
				}	*/
			} else if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P1) {	//csk_191015s
				state = inb(0x702);
				if(state == 0) { //daq2 mux end
					outb(0x02, 0x712); //daq2 fifo read enable
					myData->SubSensV.delay_count = 0;
					myData->SubSensV.phase++;
				} else {
					myData->SubSensV.delay_count++;
					if(myData->SubSensV.delay_count > 100) { //100ms
						if(myData->SubSensV.check_count < 100) {
							//kjg_180522 rtl_printf("daq mux do not end %d\n",
							//	myData->SubSensV
							//	.signal[SUB_SENS_SIG_DAQ_DIVISION]);
						}
						myData->SubSensV.delay_count = 0;
						myData->SubSensV.phase = P10;
					}
				}
			} else if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P2) {
				state = inb(0x703);
				if(state == 0) { //daq3 mux end
					outb(0x04, 0x712); //daq3 fifo read enable
					myData->SubSensV.delay_count = 0;
					myData->SubSensV.phase++;
				} else {
					myData->SubSensV.delay_count++;
					if(myData->SubSensV.delay_count > 100) { //100ms
						if(myData->SubSensV.check_count < 100) {
							//kjg_180522 rtl_printf("daq mux do not end %d\n",
							//	myData->SubSensV
							//	.signal[SUB_SENS_SIG_DAQ_DIVISION]);
						}
						myData->SubSensV.delay_count = 0;
						myData->SubSensV.phase = P10;
					}
				}
			} else {
				state = inb(0x704);
				if(state == 0) { //daq4 mux end
					outb(0x08, 0x712); //daq4 fifo read enable
					myData->SubSensV.delay_count = 0;
					myData->SubSensV.phase++;
				} else {
					myData->SubSensV.delay_count++;
					if(myData->SubSensV.delay_count > 100) { //100ms
						if(myData->SubSensV.check_count < 100) {
							//kjg_180522 rtl_printf("daq mux do not end %d\n",
							//	myData->SubSensV
							//	.signal[SUB_SENS_SIG_DAQ_DIVISION]);
						}
						myData->SubSensV.delay_count = 0;
						myData->SubSensV.phase = P10;
					}
				}	//csk_191015e
			}
			break;
		case P2:
			switch(myPs->config.daq_type) {
				case DAQ_TYPE8: //DMA
				case DAQ_TYPE9:
					//flags = claim_dma_lock();
					disable_dma(1);
					clear_dma_ff(1);
					set_dma_mode(1, DMA_MODE_READ);
					set_dma_addr(1, virt_to_bus(dma_buff));
					//kjg_d (long)myData-> = virt_to_bus(dma_buff);
					set_dma_count(1, sizeof(S_SUB_SENS_V_ORG2));
					outb(0x01, 0x720); //dma request
					enable_dma(1);
					//release_dma_lock(flags);
					myData->SubSensV.phase++;
					break;
				default: //NO_DMA
					j = 0;
					k = SUB_SENS_V_ZERO_VAL + 2 * MAX_SUB_SENS_V_REF_INDEX
						* MAX_SUB_SENS_V_AD_SLOT * MAX_SUB_SENS_V_ADC_INDEX;
					for(i=j; i < k; i++) {
						*((char *)&myData->SubSensV.org_data + i) = inb(0x710);
					}
					myData->SubSensV.phase++;
					break;
			}
			break;
		case P3:
			switch(myPs->config.daq_type) {
				case DAQ_TYPE8: //DMA
				case DAQ_TYPE9:
					if(get_dma_residue(1) == 0) {
						memcpy((char *)&myData->SubSensV.org_data, dma_buff,
							sizeof(S_SUB_SENS_V_ORG2));
						myData->SubSensV.delay_count = 0;
						myData->SubSensV.phase = P5;
					} else {
						myData->SubSensV.delay_count++;
						if(myData->SubSensV.delay_count > 100) { //100ms
							if(myData->SubSensV.check_count < 100) {
								rtl_printf("daq dma do not end %d\n",
									myData->SubSensV
									.signal[SUB_SENS_SIG_DAQ_DIVISION]);
							}
							myData->SubSensV.delay_count = 0;
							myData->SubSensV.phase = P10;
						}
					}
					break;
				default: //NO_DMA
					j = SUB_SENS_V_ZERO_VAL + 2 * MAX_SUB_SENS_V_REF_INDEX
						* MAX_SUB_SENS_V_AD_SLOT * MAX_SUB_SENS_V_ADC_INDEX;
					k = j + 2 * (MAX_SUB_SENS_V_CH_INDEX / 2)
						* MAX_SUB_SENS_V_AD_SLOT * MAX_SUB_SENS_V_ADC_INDEX;
					for(i=j; i < k; i++) {
						*((char *)&myData->SubSensV.org_data + i) = inb(0x710);
					}
					myData->SubSensV.delay_count = 0;
					myData->SubSensV.phase++;
					break;
			}
			break;
		case P4:
			j = SUB_SENS_V_ZERO_VAL + 2 * MAX_SUB_SENS_V_REF_INDEX
				* MAX_SUB_SENS_V_AD_SLOT * MAX_SUB_SENS_V_ADC_INDEX
				+ 2 * (MAX_SUB_SENS_V_CH_INDEX / 2)
				* MAX_SUB_SENS_V_AD_SLOT * MAX_SUB_SENS_V_ADC_INDEX;
			k = sizeof(S_SUB_SENS_V_ORG2);
			for(i=j; i < k; i++) {
				*((char *)&myData->SubSensV.org_data + i) = inb(0x710);
			}
			myData->SubSensV.phase++;
			break;
		case P5:
			if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P0) {
				outb(0x01, 0x713); //daq1 fifo reset
			//} else {
			//	outb(0x02, 0x713); //daq2 fifo reset
			} else if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P1) {	//csk_191015s
				outb(0x02, 0x713); //daq2 fifo reset
			} else if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P2) {
				outb(0x04, 0x713); //daq3 fifo reset
			} else {
				outb(0x08, 0x713); //daq4 fifo reset								//csk_191015e
			}

			Cal_SubSensV_Source1(MAX_SUB_SENS_V_ADC_INDEX_2);
			myData->SubSensV.phase++;
			break;
		case P6:
			Cal_SubSensV_Source2(MAX_SUB_SENS_V_ADC_INDEX_2);
			myData->SubSensV.phase++;
			break;
		case P7:
			Cal_SubSensV_Ch(MAX_SUB_SENS_V_ADC_INDEX_2);
			myData->SubSensV.phase++;
			break;
		case P8:
			switch(myPs->config.daq_type) {
				case DAQ_TYPE8: //128ch daq
				case DAQ_TYPE10:
					myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] = P0;
					sub_sens_v_count_increment();
					myData->SubSensV.phase = P0;
					break;
				case DAQ_TYPE13:	//384ch daq		//csk_191015s
					myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION]++;
					if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION]
						>= P3) {
						myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] = P0;
						sub_sens_v_count_increment();
						myData->SubSensV.phase = P0;
					} else {
						myData->SubSensV.phase = P1;
					}
					break;
				case DAQ_TYPE14:	//512ch daq
					myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION]++;
					if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION]
						>= P4) {
						myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] = P0;
						sub_sens_v_count_increment();
						myData->SubSensV.phase = P0;
					} else {
						myData->SubSensV.phase = P1;
					}
					break;			//csk_191015e
				default: //256ch daq
					myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION]++;
					if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION]
						>= P2) {
						myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] = P0;
						sub_sens_v_count_increment();
						myData->SubSensV.phase = P0;
					} else {
						myData->SubSensV.phase = P1;
					}
					break;
			}
			break;
		case P10:
			if(myData->SubSensV.check_count < 100) { //100ms * 100 = 10sec
				myData->SubSensV.check_count++;
			}

			switch(myPs->config.daq_type) {
				case DAQ_TYPE8: //128ch daq
				case DAQ_TYPE10:
					myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] = P0;
					myData->SubSensV.phase = P0;
					break;
				case DAQ_TYPE13:	//384ch daq		//csk_191015s
					myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION]++;
					if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION]
						>= P3) {
						myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] = P0;
						myData->SubSensV.phase = P0;
					} else {
						myData->SubSensV.phase = P1;
					}
					break;
				case DAQ_TYPE14:	//512ch daq
					myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION]++;
					if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION]
						>= P4) {
						myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] = P0;
						myData->SubSensV.phase = P0;
					} else {
						myData->SubSensV.phase = P1;
					}
					break;			//csk_191015e
				default: //256ch daq
					myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION]++;
					if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION]
						>= P2) {
						myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] = P0;
						myData->SubSensV.phase = P0;
					} else {
						myData->SubSensV.phase = P1;
					}
					break;
			}
			break;
		default:
			break;
	}
}
//kjg_180914
/*void SubSensV_process_CAN_DAQ(void)
{ //kjg_180920
	unsigned char sel, val;
	int bd, sub_rack, ch, daq_ch;
	long long_value;

	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1)  return;
	if(myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0) return; //kjh_160610
	if(myPs->misc.timer_1sec < 3) return;

	switch(myData->SubSensV.phase) {
		case P0:
			val = inb(0x791);
			if((val & 0x80) != 0) break;

			bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
			sub_rack = bd / 4;
			bd = bd % 4;

			sel = (unsigned char)(sub_rack << 4) & 0x70;
			sel |= (unsigned char)bd;
			sel |= 0x80; //ram write stop
			outb(sel, 0x790);

			myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_SEL] = sel;
			myData->SubSensV.phase++;
			break;
		case P1:
		case P2:
			val = inb(0x791);
			if((val & 0x80) != 0) break;
			myData->SubSensV.phase++;
			break;
		case P3:
			bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
			sub_rack = bd / 4;
			bd = bd % 4;

			for(ch=0; ch < 32; ch++) {
				daq_ch = (long)(sub_rack * 128);
				daq_ch += (long)(bd * 32);
				daq_ch += (long)ch;

				val = inb(ch * 4 + 0x700);
				long_value = (long)val;

				val = inb(ch * 4 + 0x701);
				long_value |= ((long)val << 8);

				val = inb(ch * 4 + 0x702);
				long_value |= ((long)val << 16);

				val = inb(ch * 4 + 0x703);
				long_value |= ((long)val << 24);

				//myData->SubSensV.ch[daq_ch].sensV = long_value * 1000;
				myData->SubSensV.ch[daq_ch].sensV = long_value;
				//myData->SubSensV.ch[daq_ch].sensV = 4499000;//long_value;
			}

			sel = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_SEL];
			sel &= 0x7F; //ram write run
			outb(sel, 0x790);

			myData->SubSensV.phase++;
			break;
		case P4:
		case P5:
		case P6:
			myData->SubSensV.phase++;
			break;
		case P7:
			bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
			if(bd == 15) {
			//if(bd == 7) {
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE] = 0;
			} else {
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE]++;
			}

			myData->SubSensV.phase = P0;
			break;
		default:
			break;
	}
}*/

void SubSensV_process_CAN_DAQ(void)
{ //kjg_190822
	unsigned char sel, val;
	int bd, sub_rack, ch, daq_ch, fail_ch;
	long long_value[32], test_val[8];

	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1)  return;
	if(myPs->config.installedAuxV == 0 && myPs->config.installedTH == 0
		&& myPs->config.installedHumidity == 0
		&& myPs->config.installedGas == 0) return;	//sec_220926
	if(myPs->misc.timer_1sec < 3) return;

	switch(myData->SubSensV.phase) {
		case P0:
			val = inb(0x7A1);
			if((val & 0x80) == 0) {
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE] = 0;
				bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
				sub_rack = bd / 4;
				bd = bd % 4;

				sel = (unsigned char)(sub_rack << 2);
				sel |= (unsigned char)bd;
				sel |= 0x80; //ram1 write stop
				outb(sel, 0x7A0);
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_SEL] = sel;

				myData->SubSensV.phase = P10;
			} else if((val & 0x40) == 0) {
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE] = 8;
				bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
				sub_rack = bd / 4;
				bd = bd % 4;

				sel = (unsigned char)(sub_rack << 2);
				sel |= (unsigned char)bd;
				sel |= 0x40; //ram2 write stop
				outb(sel, 0x7A0);
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_SEL] = sel;

				myData->SubSensV.phase = P20;
			}
			break;
		case P10:
		case P11:
			myData->SubSensV.phase++;
			break;
		case P12:
			bd = inb(0x700) - 1;
			val = inb(0x701);
			val = inb(0x702);
			val = inb(0x703);

			//bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
			sub_rack = bd / 4;
			bd = bd % 4;

			fail_ch = 0;
			for(ch=0; ch < 32; ch++) {
				val = inb(ch * 4 + 0x700 + 4);
				long_value[ch] = (long)val;

				val = inb(ch * 4 + 0x701 + 4);
				long_value[ch] |= ((long)val << 8);

				val = inb(ch * 4 + 0x702 + 4);
				long_value[ch] |= ((long)val << 16);

				val = inb(ch * 4 + 0x703 + 4);
				long_value[ch] |= ((long)val << 24);

				if(long_value[ch] == (-1)) fail_ch++;
			}

			if(fail_ch != 32 && (sub_rack >= 0 && sub_rack <= 3)) {
				for(ch=0; ch < 32; ch++) {
					daq_ch = (long)(sub_rack * 128);
					daq_ch += (long)(bd * 32);
					daq_ch += (long)ch;

					myData->SubSensV.ch[daq_ch].tmpSensV = long_value[ch];
				}
			}


			bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
			if(bd == 7) {
				outb(0x00, 0x7A0); //ram write run

				myData->SubSensV.phase++;
			} else {
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE]++;
				bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
				sub_rack = bd / 4;
				bd = bd % 4;

				sel = (unsigned char)(sub_rack << 2);
				sel |= (unsigned char)bd;
				sel |= 0x80; //ram1 write stop
				outb(sel, 0x7A0);
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_SEL] = sel;

				myData->SubSensV.phase = P11;
				//myData->SubSensV.phase = P10;
			}
			break;
		case P13:
			val = inb(0x7A1);
			if((val & 0x40) == 0) {
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE] = 8;
				bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
				sub_rack = bd / 4;
				bd = bd % 4;

				sel = (unsigned char)(sub_rack << 2);
				sel |= (unsigned char)bd;
				sel |= 0x40; //ram2 write stop
				outb(sel, 0x7A0);
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_SEL] = sel;

				myData->SubSensV.phase++;
			}
			break;
		case P14:
		case P15:
			myData->SubSensV.phase++;
			break;
		case P16:
			bd = inb(0x700) - 1;
			val = inb(0x701);
			val = inb(0x702);
			val = inb(0x703);

			//bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
			sub_rack = bd / 4;
			bd = bd % 4;

			fail_ch = 0;
			for(ch=0; ch < 32; ch++) {
				val = inb(ch * 4 + 0x700 + 4);
				long_value[ch] = (long)val;

				val = inb(ch * 4 + 0x701 + 4);
				long_value[ch] |= ((long)val << 8);

				val = inb(ch * 4 + 0x702 + 4);
				long_value[ch] |= ((long)val << 16);

				val = inb(ch * 4 + 0x703 + 4);
				long_value[ch] |= ((long)val << 24);

				if(long_value[ch] == (-1)) fail_ch++;
			}

			if(fail_ch != 32 && (sub_rack >= 0 && sub_rack <= 3)) {
				for(ch=0; ch < 32; ch++) {
					daq_ch = (long)(sub_rack * 128);
					daq_ch += (long)(bd * 32);
					daq_ch += (long)ch;

					myData->SubSensV.ch[daq_ch].tmpSensV = long_value[ch];
				}
			}


			bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
			if(bd == 15) {
				outb(0x00, 0x7A0); //ram write run

				myData->SubSensV.phase = P30;
			} else {
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE]++;
				bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
				sub_rack = bd / 4;
				bd = bd % 4;

				sel = (unsigned char)(sub_rack << 2);
				sel |= (unsigned char)bd;
				sel |= 0x40; //ram2 write stop
				outb(sel, 0x7A0);
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_SEL] = sel;

				myData->SubSensV.phase = P15;
				//myData->SubSensV.phase = P14;
			}
			break;
		case P20:
		case P21:
			myData->SubSensV.phase++;
			break;
		case P22:
			bd = inb(0x700) - 1;
			val = inb(0x701);
			val = inb(0x702);
			val = inb(0x703);

			//bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
			sub_rack = bd / 4;
			bd = bd % 4;

			fail_ch = 0;
			for(ch=0; ch < 32; ch++) {
				val = inb(ch * 4 + 0x700 + 4);
				long_value[ch] = (long)val;

				val = inb(ch * 4 + 0x701 + 4);
				long_value[ch] |= ((long)val << 8);

				val = inb(ch * 4 + 0x702 + 4);
				long_value[ch] |= ((long)val << 16);

				val = inb(ch * 4 + 0x703 + 4);
				long_value[ch] |= ((long)val << 24);

				if(long_value[ch] == (-1)) fail_ch++;
			}

			if(fail_ch != 32 && (sub_rack >= 0 && sub_rack <= 3)) {
				for(ch=0; ch < 32; ch++) {
					daq_ch = (long)(sub_rack * 128);
					daq_ch += (long)(bd * 32);
					daq_ch += (long)ch;

					myData->SubSensV.ch[daq_ch].tmpSensV = long_value[ch];
				}
			}


			bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
			if(bd == 15) {
				outb(0x00, 0x7A0); //ram write run

				myData->SubSensV.phase++;
			} else {
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE]++;
				bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
				sub_rack = bd / 4;
				bd = bd % 4;

				sel = (unsigned char)(sub_rack << 2);
				sel |= (unsigned char)bd;
				sel |= 0x40; //ram2 write stop
				outb(sel, 0x7A0);
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_SEL] = sel;

				myData->SubSensV.phase = P21;
				//myData->SubSensV.phase = P20;
			}
			break;
		case P23:
			val = inb(0x7A1);
			if((val & 0x80) == 0) {
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE] = 0;
				bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
				sub_rack = bd / 4;
				bd = bd % 4;

				sel = (unsigned char)(sub_rack << 2);
				sel |= (unsigned char)bd;
				sel |= 0x80; //ram1 write stop
				outb(sel, 0x7A0);
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_SEL] = sel;

				myData->SubSensV.phase++;
			}
			break;
		case P24:
		case P25:
			myData->SubSensV.phase++;
			break;
		case P26:
			bd = inb(0x700) - 1;
			val = inb(0x701);
			val = inb(0x702);
			val = inb(0x703);

			//bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
			sub_rack = bd / 4;
			bd = bd % 4;

			fail_ch = 0;
			for(ch=0; ch < 32; ch++) {
				val = inb(ch * 4 + 0x700 + 4);
				long_value[ch] = (long)val;

				val = inb(ch * 4 + 0x701 + 4);
				long_value[ch] |= ((long)val << 8);

				val = inb(ch * 4 + 0x702 + 4);
				long_value[ch] |= ((long)val << 16);

				val = inb(ch * 4 + 0x703 + 4);
				long_value[ch] |= ((long)val << 24);

				if(long_value[ch] == (-1)) fail_ch++;
			}

			if(fail_ch != 32 && (sub_rack >= 0 && sub_rack <= 3)) {
				for(ch=0; ch < 32; ch++) {
					daq_ch = (long)(sub_rack * 128);
					daq_ch += (long)(bd * 32);
					daq_ch += (long)ch;

					myData->SubSensV.ch[daq_ch].tmpSensV = long_value[ch];
				}
			}


			bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
			if(bd == 7) {
				outb(0x00, 0x7A0); //ram write run

				myData->SubSensV.phase = P30;
			} else {
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE]++;
				bd = myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_PHASE];
				sub_rack = bd / 4;
				bd = bd % 4;

				sel = (unsigned char)(sub_rack << 2);
				sel |= (unsigned char)bd;
				sel |= 0x80; //ram1 write stop
				outb(sel, 0x7A0);
				myData->SubSensV.signal[SUB_SENS_SIG_DAQ_CH_DATA_SEL] = sel;

				myData->SubSensV.phase = P25;
				//myData->SubSensV.phase = P24;
			}
			break;
		case P30:
			test_val[0] = myData->SubSensV.ch[0].tmpSensV;
			test_val[1] = myData->SubSensV.ch[0].tmpSensV;
			test_val[2] = myData->SubSensV.ch[128].tmpSensV;
			test_val[3] = myData->SubSensV.ch[128].tmpSensV;
			test_val[4] = myData->SubSensV.ch[128].tmpSensV;
			test_val[5] = myData->SubSensV.ch[128].tmpSensV;
			test_val[7] = myData->SubSensV.ch[128].tmpSensV;
			test_val[8] = myData->SubSensV.ch[128].tmpSensV;

			for(daq_ch=0; daq_ch < MAX_SUB_SENS_V_DATA; daq_ch++) {
				myData->SubSensV.ch[daq_ch].sensV
					= myData->SubSensV.ch[daq_ch].tmpSensV;
			}

			myData->SubSensV.phase++;
			break;
		default:
			//if(myData->SubSensV.phase == P85) {
			//if(myData->SubSensV.phase == P70) {
			if(myData->SubSensV.phase == P60) {
			//if(myData->SubSensV.phase == P50) {
			//if(myData->SubSensV.phase == P45) {
				myData->SubSensV.phase = P0;
			} else {
				myData->SubSensV.phase++;
			}
			break;
	}
}
