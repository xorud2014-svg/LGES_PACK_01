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
	switch(myPs->config.daq_type) {
		case DAQ_TYPE12:
			SubSensV_adc_start_12();
			break;
		default:
			SubSensV_adc_start_Default();
			break;
	}
}

void SubSensV_adc_start_Default(void)
{
	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1) return;
	if(myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0) return; //kjh_160610
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

void SubSensV_adc_start_12(void)
{
	int bd, base_addr;

	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1) return;
	if(myData->daq.config.daq_use != P1) return;
	if(myPs->misc.timer_1sec < 3) return;

	for(bd=0; bd < myData->daq.config.installedDAQ; bd++) {
		base_addr = 0x710 + 0x10 * bd;
		outb(0x00, base_addr);
	}
}

void SubSensV_Control(int phase)
{
	int dma_size;
	//unsigned long flags=0;

	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1) return;
	if(myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0) return; //kjh_160610

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
	
			memcpy((char *)&myData->SubSensV.org_data[0].zero_val,
				(char *)&org_data1.zero_val, SUB_SENS_V_ZERO_VAL);
			for(i = 0; i < MAX_SUB_SENS_V_REF_INDEX; i++) {
				for(j = 0; j < MAX_SUB_SENS_V_AD_SLOT; j++) {
					for(k = 0; j < MAX_SUB_SENS_V_ADC_INDEX_1; j++) {
						myData->SubSensV.org_data[0].ref_val[i][j][k]
							= org_data1.ref_val[i][j][k];
					}
				}
			}
			for(i = 0; i < MAX_SUB_SENS_V_REF_INDEX; i++) {
				for(j = 0; j < MAX_SUB_SENS_V_AD_SLOT; j++) {
					for(k = 0; j < MAX_SUB_SENS_V_ADC_INDEX_1; j++) {
						myData->SubSensV.org_data[0].sub_val[i][j][k]
							= org_data1.sub_val[i][j][k];
					}
				}
			}
			memcpy((char *)&myData->SubSensV.org_data[0].equal_val,
				(char *)&org_data1.equal_val, 2);
		} else {
			memcpy((char *)&myData->SubSensV.org_data[0], dma_buff, dma_size);
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
		&& myPs->config.installedTH == 0) return; //kjh_160610

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
		memcpy((char *)&myData->SubSensV.org_data[0], buff, dma_size);
	}
}
//kjhw_140528e
//
void SubSensV_Control2(int phase)
{
	int max_adc_index;

	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1) return;
	if(myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0) return; //kjh_160610

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
	int bd;
	switch(myPs->config.daq_type) {
		case DAQ_TYPE12:
			for(bd=0; bd < myData->daq.config.installedDAQ; bd++) {
				Cal_SubSensV_Source1_12_Filter1(max_adc_index, bd);
				Cal_SubSensV_Source1_12(max_adc_index, bd);
			}
			break;
		default:
			Cal_SubSensV_Source1_Default(max_adc_index);
			break;
	}
}

void Cal_SubSensV_Source1_Default(int max_adc_index)
{
	short int tmp;
	int sensCount, end, ref_index, ad_slot, adc_index;
	long sum;
	double ratio;
	S_SUB_SENS_V_SOURCE_DIV *mySubSensSource;

	if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P0) {
		mySubSensSource = &(myData->SubSensV.source_div[0]);
	} else {
		mySubSensSource = &(myData->SubSensV.source_div[1]);
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
				tmp	= myData->SubSensV.org_data[0]
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


void Cal_SubSensV_Source1_12_Filter1(int max_adc_index, int bd)
{
	short int tmp;
	int sensCount, end, ref_index, ad_slot, adc_index, i, j;
	long sum;
	long list[MAX_SUB_SENS_V_SOURCE_SENS_COUNT];

	S_SUB_SENS_V_SOURCE_DIV *mySubSensSource;

	if(bd >= MAX_DAQ_BD_NUM) return;

	mySubSensSource = &(myData->SubSensV.source_div[bd]);

	sensCount = myPs->misc.SubSensV_SourceSensCount;
	if(myPs->misc.SubSensV_SourceSensCountFlag < P1) {
		end = sensCount + 1;
	} else {
		end = MAX_SUB_SENS_V_SOURCE_SENS_COUNT;
	}

	for(ref_index=0; ref_index < MAX_SUB_SENS_V_REF_INDEX; ref_index++) {
		for(ad_slot=0; ad_slot < MAX_SUB_SENS_V_AD_SLOT; ad_slot++) {
			for(adc_index=0; adc_index < max_adc_index; adc_index++) {
				tmp	= myData->SubSensV.org_data[bd]
					.ref_val[ref_index][ad_slot][adc_index].val;
				mySubSensSource->source[ad_slot][adc_index]
					.tmpV_f1[ref_index][sensCount] = tmp;
			}
		}
	}
	
	sum = 0;
	for(ad_slot=0; ad_slot < MAX_SUB_SENS_V_AD_SLOT; ad_slot++) {
		for(adc_index=0; adc_index < max_adc_index; adc_index++) {
			for(ref_index=0; ref_index < MAX_SUB_SENS_V_REF_INDEX;
				ref_index++) {
				if(myPs->misc.SubSensV_SourceSensCountFlag < P1) {
					sensCount = myPs->misc.SubSensV_SourceSensCount;
					sum = mySubSensSource->source[ad_slot][adc_index]
						.tmpV_f1[ref_index][sensCount];
				}else{
					sum = 0;
					memset(list, 0x00, sizeof(list));
					for(sensCount=0; sensCount < end; sensCount++) {
						tmp = mySubSensSource->source[ad_slot][adc_index]
							.tmpV_f1[ref_index][sensCount];
						sum += tmp;
						list[sensCount] = tmp;
					}
					for(i = 0; i < end -1; i++){
						for(j = i+1; j < end; j++){
							if(list[i] < list[j]){
								SWAP(list[i], list[j], tmp);
							}
						}
					}
					if(end < 6){
						sum /= end;
					}else{
						sum = sum 
							- list[0]
							- list[1]
							- list[end-2]
							- list[end-1];
						sum = (double)(sum / (end - 4));
					}
				}
				sensCount = myPs->misc.SubSensV_SourceSensCount;
				mySubSensSource->source[ad_slot][adc_index]
					.tmpV[ref_index][sensCount] = sum;
			}
		}
	}
}

void Cal_SubSensV_Source1_12(int max_adc_index, int bd)
{
	short int tmp;
	int sensCount, end, ref_index, ad_slot, adc_index, i, j;
	long sum;
	double ratio;
	long list[MAX_SUB_SENS_V_SOURCE_SENS_COUNT];

	S_SUB_SENS_V_SOURCE_DIV *mySubSensSource;

//	if(bd >= 2) return; //KHKW
	if(bd >= MAX_DAQ_BD_NUM) return;

	mySubSensSource = &(myData->SubSensV.source_div[bd]);

	sensCount = myPs->misc.SubSensV_SourceSensCount;
	if(myPs->misc.SubSensV_SourceSensCountFlag < P1) {
		end = sensCount + 1;
	} else {
		end = MAX_SUB_SENS_V_SOURCE_SENS_COUNT;
	}

	for(ref_index=0; ref_index < MAX_SUB_SENS_V_REF_INDEX; ref_index++) {
		for(ad_slot=0; ad_slot < MAX_SUB_SENS_V_AD_SLOT; ad_slot++) {
			for(adc_index=0; adc_index < max_adc_index; adc_index++) {
				tmp	= myData->SubSensV.org_data[bd]
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
				if(myPs->misc.SubSensV_SourceSensCountFlag < P1) {
					sensCount = myPs->misc.SubSensV_SourceSensCount;
					sum = mySubSensSource->source[ad_slot][adc_index]
							.tmpV[ref_index][sensCount];
				}else{
					sum = 0;
					memset(list, 0x00, sizeof(list));
					for(sensCount=0; sensCount < end; sensCount++) {
						tmp = mySubSensSource->source[ad_slot][adc_index]
							.tmpV[ref_index][sensCount];
						sum += tmp;
						list[sensCount] = tmp;
					}
					for(i = 0; i < end -1; i++){
						for(j = i+1; j < end; j++){
							if(list[i] < list[j]){
								SWAP(list[i], list[j], tmp);
							}
						}
					}
					if(end < 6){
						sum /= end;
					}else{
						sum = sum 
							- list[0]
							- list[1]
							- list[end-2]
							- list[end-1];
						sum = (double)(sum / (end - 4));
					}
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
					= (long)((double)sum * ratio);
			}
		}
	}
}


/*
void Cal_SubSensV_Source1_12(int max_adc_index, int bd)
{
	short int tmp;
	int sensCount, end, ref_index, ad_slot, adc_index;
	long sum, max1=0, max2=0, min1=0, min2=0;
	double ratio;
	S_SUB_SENS_V_SOURCE_DIV *mySubSensSource;

//	if(bd >= 2) return; //KHKW
	if(bd >= MAX_DAQ_BD_NUM) return;

	mySubSensSource = &(myData->SubSensV.source_div[bd]);

	sensCount = myPs->misc.SubSensV_SourceSensCount;
	if(myPs->misc.SubSensV_SourceSensCountFlag < P1) {
		end = sensCount + 1;
	} else {
		end = MAX_SUB_SENS_V_SOURCE_SENS_COUNT;
	}

	for(ref_index=0; ref_index < MAX_SUB_SENS_V_REF_INDEX; ref_index++) {
		for(ad_slot=0; ad_slot < MAX_SUB_SENS_V_AD_SLOT; ad_slot++) {
			for(adc_index=0; adc_index < max_adc_index; adc_index++) {
				tmp	= myData->SubSensV.org_data[bd]
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
					tmp = mySubSensSource->source[ad_slot][adc_index]
						.tmpV[ref_index][sensCount];
					sum += tmp;
					if(sensCount == 0){
						max1 = tmp;
						max2 = tmp;
						min1 = tmp;
						min2 = tmp;
					}else{
						if(max1 < tmp){
							if(max2 < tmp){
								max2 = tmp;
							}else{
								max1 = tmp;
							}
						}
						if(min1 > tmp){
							if(min2 > tmp){
								min2 = tmp;
							}else{
								min1 = tmp;
							}
						}
					}
				}
				if(end > 4){
					sum = (sum - max1 - max2 - min1 - min2)/(end -4);
				}else{
					sum = sum / end;
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
//					= (long)((double)sum * ratio / end);
					= (long)((double)sum * ratio);
			}
		}
	}
}
*/

void Cal_SubSensV_Source2(int max_adc_index)
{
	int bd;
	switch(myPs->config.daq_type) {
		case DAQ_TYPE12:
			for(bd=0; bd < myData->daq.config.installedDAQ; bd++) {
				Cal_SubSensV_Source2_12(max_adc_index, bd);
			}
			break;
		default:
			Cal_SubSensV_Source2_Default(max_adc_index);
			break;
	}
}

void Cal_SubSensV_Source2_Default(int max_adc_index)
{
	int ref_index, ad_slot, adc_index;
	double AD_a, AD_b, val1;
	S_SUB_SENS_V_SOURCE_DIV *mySubSensSource;

	if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P0) {
		mySubSensSource = &(myData->SubSensV.source_div[0]);
	} else {
		mySubSensSource = &(myData->SubSensV.source_div[1]);
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

void Cal_SubSensV_Source2_12(int max_adc_index, int bd)
{
	int ref_index, ad_slot, adc_index;
	double AD_a, AD_b, val1;
	S_SUB_SENS_V_SOURCE_DIV *mySubSensSource;

//	if(bd >= 2) return; //KHKW
	if(bd >= MAX_DAQ_BD_NUM) return;
	
	mySubSensSource = &(myData->SubSensV.source_div[bd]);
	
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
	int bd;
	switch(myPs->config.daq_type) {
		case DAQ_TYPE12:
			//jhkw_211002s
			if(myData->SubSensV.signal[TH_TABLE_PHASE] >= 6) {	//shhw_230228s	MAX_TH_DATA: 300
			//if(myData->SubSensV.signal[TH_TABLE_PHASE] >= 7) {
				myData->SubSensV.signal[TH_TABLE_PHASE] = 1;
			} else {
				myData->SubSensV.signal[TH_TABLE_PHASE]++;
			}
			//jhkw_211002e
			for(bd=0; bd < myData->daq.config.installedDAQ; bd++) {
				Cal_SubSensV_Ch_12(max_adc_index, bd);
			}
			break;
		default:
			Cal_SubSensV_Ch_Default(max_adc_index);
			break;
	}
}

void Cal_SubSensV_Ch_Default(int max_adc_index)
{
	short int th_type;
	int sensCount, end, ch_index, ad_slot, adc_index, data_index, data_offset;
	int monitor_ch;	//jhkw_201117
	int filter_sensCount;	//jhkw_151028
//	int ch, k, j;
	long tmp;
	double ratio, sum;
	S_SUB_SENS_V_SOURCE_DIV *mySubSensSource;

	if(myData->AppControl.config.debugType == 110
		|| myData->AppControl.config.debugType == 111) {
		mySubSensSource = &(myData->SubSensV.source_div[0]);
		data_offset = 0;
		sum = 0.0;

		for(ch_index=0; ch_index < MAX_SUB_SENS_V_CH_INDEX; ch_index++) {
			for(ad_slot=0; ad_slot < MAX_SUB_SENS_V_AD_SLOT; ad_slot++) {
				for(adc_index=0; adc_index < max_adc_index; adc_index++) {
					data_index = ch_index + ad_slot * MAX_SUB_SENS_V_CH_INDEX
						+ (adc_index * MAX_SUB_SENS_V_CH_INDEX
						* MAX_SUB_SENS_V_AD_SLOT) + data_offset;
					if(myData->SubSensV.ch[data_index].sensV <= 5000000) {
						myData->SubSensV.ch[data_index].sensV += 10000;
					} else myData->SubSensV.ch[data_index].sensV = 10000;
				}
			}
		}
		return;
	}

	if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P0) {
		mySubSensSource = &(myData->SubSensV.source_div[0]);
		data_offset = 0;
	} else {
		mySubSensSource = &(myData->SubSensV.source_div[1]);
		data_offset = 128;
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
				tmp = (long)myData->SubSensV.org_data[0]
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
				if(myData->daq.config.useDampingResistor == 1) sum *= 1.0025072;  //shhw_230611
				//sum *= 1.0025072;	//phb_230214
				//jhkw_201117s
				if(myData->AppControl.config.debugType == 220) {
					sum = (data_index+1) * 100000;
#if defined __DEBUG__	//shh_231124s
				} else if(myData->AppControl.config.debugType >= 230) {	//shhw_240104 
				//} else if(myData->AppControl.config.debugType == 230) { 
					monitor_ch = myData->aux_ch_num[data_index].monitor_ch;
					sum = myData->sil_daq_val[monitor_ch];
				} else {
#endif					//shh_231124e
				}
				monitor_ch = myData->aux_ch_num[data_index].monitor_ch;
				monitor_ch += myData->mData.config.installedTemp;
				switch(myData->aux_ch_num[data_index].type) {
					case 2:
						th_type = myData->auxSetData[monitor_ch].tableNo;
						sum = (double)Convert_Vth_to_Temp((float)sum, th_type);
						//sum = sum * (100 * th_type);
						break;
					case 5:
						th_type = myData->auxSetData[monitor_ch].tableNo;
						sum = (double)Convert_Vth_to_Ohm((float)sum, th_type);
						//sum = sum * (100 * th_type);
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
					case 17:	//shh_211007
						sum = sum * 20 + 15000;
						break;
					default:
						break;
				}
				//jhkw_201117e
				myData->SubSensV.ch[data_index].sensV = (long)sum;
				//myData->SubSensV.ch[data_index].sensV = data_index * 100000;
			}
		}
	}
}

void Cal_SubSensV_Ch_12(int max_adc_index, int bd)
{
	short int th_type;
	int sensCount, end, ch_index, ad_slot, adc_index, data_index, data_offset;
	int monitor_ch;	//jhkw_201117
	int i, j;
	long tmp;
	double ratio, sum;
	long list[MAX_SUB_SENS_V_FILTER_AD_COUNT];

	S_SUB_SENS_V_SOURCE_DIV *mySubSensSource;

//	if(bd >= 2) return; //KHKW
	if(bd >= MAX_DAQ_BD_NUM) return;

	/*	//shh_230228s                                  
	//jhkw_211002s
	if(myData->SubSensV.signal[TH_TABLE_PHASE] >= 6) {	//shhw_221114 	
	//if(myData->SubSensV.signal[TH_TABLE_PHASE] >= 7) {
		myData->SubSensV.signal[TH_TABLE_PHASE] = 1;
	} else {
		myData->SubSensV.signal[TH_TABLE_PHASE]++;
	}
	//jhkw_211002e  
	*/	//shh_230228e                                  
	
	if(myData->AppControl.config.debugType == 110
		|| myData->AppControl.config.debugType == 111) {
		mySubSensSource = &(myData->SubSensV.source_div[0]);
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

	mySubSensSource = &(myData->SubSensV.source_div[bd]);
	data_offset = 128 * bd;
	
	sensCount = myPs->misc.SubSensV_ChSensCount;
	if(myPs->misc.SubSensV_ChSensCountFlag < P1) {
		end = sensCount + 1;
	} else {
		end = MAX_SUB_SENS_V_FILTER_AD_COUNT;
	}

	for(ch_index=0; ch_index < MAX_SUB_SENS_V_CH_INDEX; ch_index++) {
		for(ad_slot=0; ad_slot < MAX_SUB_SENS_V_AD_SLOT; ad_slot++) {
			for(adc_index=0; adc_index < max_adc_index; adc_index++) {
				tmp = (long)myData->SubSensV.org_data[bd]
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
/*				
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
*/				
			}
			
		}
	}

	for(ch_index=0; ch_index < MAX_SUB_SENS_V_CH_INDEX; ch_index++) {
		for(ad_slot=0; ad_slot < MAX_SUB_SENS_V_AD_SLOT; ad_slot++) {
			for(adc_index=0; adc_index < max_adc_index; adc_index++) {
				data_index = ch_index + ad_slot * MAX_SUB_SENS_V_CH_INDEX
					+ (adc_index * MAX_SUB_SENS_V_CH_INDEX
					* MAX_SUB_SENS_V_AD_SLOT) + data_offset;

				if(myPs->misc.SubSensV_ChSensCountFlag == P0) {
					sensCount = myPs->misc.SubSensV_ChSensCount;
					sum = myData->SubSensV.ch[data_index].tmpV[sensCount];
				}else{
					sum = 0;
					memset(list, 0x00, sizeof(list));
					for(sensCount = 0;sensCount < end; sensCount++){
						sum += myData->SubSensV.ch[data_index].tmpV[sensCount];
						list[sensCount] = myData->SubSensV.ch[data_index].tmpV[sensCount];
					}

					for(i = 0; i < end -1; i++){
						for(j = i+1; j < end; j++){
							if(list[i] < list[j]){
								SWAP(list[i], list[j], tmp);
							}
						}
					}
					if(end < 7){
						sum /= end;
					}else{
						sum = sum 
							- list[0]
							- list[1]
							- list[2]
							- list[end-3]
							- list[end-2]
							- list[end-1];
						sum = (double)(sum / (end - 6));
					}
				}
				//jhkw_201117s
				if(myData->daq.config.useDampingResistor == 1) sum *= 1.0025072;  //shhw_230611
				//sum *= 1.0025072; //phb_230214
				
				//monitor_ch = myData->aux_ch_num[data_index].monitor_ch; //shhw_231127	
				//monitor_ch += myData->mData.config.installedTemp;	
				
				if(myData->AppControl.config.debugType == 220) {
					sum = ((data_index+1) * 100000) + myData->test_val_l[9];
					if(sum > 30000000) {
						myData->test_val_l[9] =0;
					} else if(sum < 1000) {
						myData->test_val_l[9] = 0;
					} else {
						myData->test_val_l[9]++;
					}
					//sum = ((data_index+1) * 10000) 
					//	+ (myData->auxSetData[monitor_ch].tableNo * 1000000);
					//sum = 3000000; 
#if defined __DEBUG__
				} else if(myData->AppControl.config.debugType >= 230) { //shhw_240104
				//} else if(myData->AppControl.config.debugType == 230) {
					monitor_ch = myData->aux_ch_num[data_index].monitor_ch;
					sum = myData->sil_daq_val[monitor_ch];
				} else {
#endif
				}
				
				monitor_ch = myData->aux_ch_num[data_index].monitor_ch; //shhw_231127
				monitor_ch += myData->mData.config.installedTemp;		
				
				switch(myData->aux_ch_num[data_index].type) {
					case 2:
						th_type = myData->auxSetData[monitor_ch].tableNo;
						sum = (double)Convert_Vth_to_Temp((float)sum, th_type);
						//sum = sum * (100 * th_type);
						break;
					case 5:
						th_type = myData->auxSetData[monitor_ch].tableNo;
						sum = (double)Convert_Vth_to_Ohm((float)sum, th_type);
						//sum = sum * (100 * th_type);
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
					case 17:	//shh_211007
						sum = sum * 20 + 15000;
						break;
					default:
						break;
				}
				//jhkw_201117e
				//myData->SubSensV.ch[data_index].sensV = (long)sum;
				if(myData->aux_ch_num[data_index].type == 2) {
					if(myData->SubSensV.signal[TH_TABLE_READ_FLAG] == 1) {
						myData->SubSensV.ch[data_index].sensV = (long)sum; 
						myData->SubSensV.signal[TH_TABLE_READ_FLAG] = 0;
					}
				} else {
					myData->SubSensV.ch[data_index].sensV = (long)sum;
				}
			}
		}
	}
}

float Convert_Vth_to_Temp(float Vth, int type)
{
	int table, i;
	long max_th_data, start_data;	//jhkw_211002
	float V_TH, TEMP, ret_temp;

	Vth = Vth / 1000.0;
	//Only use DampingResistor && debugType == 0
	if(myData->daq.config.useDampingResistor == 1 
		&& myData->AppControl.config.debugType == 0) Vth /= 1.0025072; //shhw_230104
	
	//jhkw_211002s
	switch(myPs->config.daq_type) {
		case DAQ_TYPE12:
			max_th_data = myData->SubSensV.signal[TH_TABLE_PHASE] * 50;
			start_data = max_th_data - 50;
			break;
		default:
			max_th_data = MAX_TH_DATA;
			start_data = 0;
			break;
	}
	//jhkw_211002e
	table=0;
	for(table=0; table < MAX_TH_TABLE; table++) {
		if((int)myData->th_table[table].th_type == type) {
			break;
		}
	}
	ret_temp = 999999.0; //999.999degreeC
	if(table == MAX_TH_TABLE) {
		myData->SubSensV.signal[TH_TABLE_READ_FLAG] = 1;
		return ret_temp * 1000.0;
	}
	//kjh_160610s
	if(type == 0) {
		ret_temp = 123456.0; //123.456degreeC
		myData->SubSensV.signal[TH_TABLE_READ_FLAG] = 1;
		return ret_temp * 1000.0;
	}
	//kjh_160610e

	//for(i=0; i < MAX_TH_DATA; i++) {
	for(i=start_data; i < max_th_data; i++) {	//jhkw_211002
		if(myData->th_table[table].T_R[i+1][0] >= 999000.0) {
			ret_temp = 999000.0;
			myData->SubSensV.signal[TH_TABLE_READ_FLAG] = 1;
			break;
		}

		V_TH = myData->th_table[table].V_TH[i];
		TEMP = myData->th_table[table].T_R[i][0];

		if(Vth > V_TH) {
			ret_temp = -999999.0;
			if(myData->SubSensV.signal[TH_TABLE_PHASE] == 1 && i == 0) {
				myData->SubSensV.signal[TH_TABLE_READ_FLAG] = 1;
			}
			break;
		} else if(Vth > myData->th_table[table].V_TH[i+1]) {
			ret_temp = TEMP -
				((V_TH - Vth) / (V_TH - myData->th_table[table].V_TH[i+1]))
				* (TEMP - myData->th_table[table].T_R[i+1][0]);
			myData->SubSensV.signal[TH_TABLE_READ_FLAG] = 1;
			break;
		}
	}
	if(i == MAX_TH_DATA) ret_temp = 998000.0;

	return ret_temp * 1000.0;
}

float Convert_Vth_to_Ohm(float Vth, int type)
{ //lki_110821
	int table, i;
	float ret_temp;

	Vth = Vth / 1000.0;
	//Only use DampingResistor && debugType == 0
	if(myData->daq.config.useDampingResistor == 1 
		&& myData->AppControl.config.debugType == 0) Vth /= 1.0025072; //shhw_230104

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

	//ret_temp = (390000 * ret_temp) / (390000 - ret_temp);

	return ret_temp * 1000.0;
}

void SubSensV_process(void)
{ //kjg_150225
	unsigned char state;
	int i, j, k;
	//unsigned long flags=0;

	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1)  return;
	if(myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0) return; //kjh_160610
	if(myPs->misc.timer_1sec < 3) return;

	switch(myData->SubSensV.phase) {
		case P0:
			outb(0x03, 0x710); //daq1, daq2 board mux start
			if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_ISOLATION] == P0) {
				outb(0x03, 0x711);
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
						myData->SubSensV.phase = P10;
					}
				}
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
				}
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
						*((char *)&myData->SubSensV.org_data[0] + i) = inb(0x710);
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
						memcpy((char *)&myData->SubSensV.org_data[0], dma_buff,
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
						*((char *)&myData->SubSensV.org_data[0] + i) = inb(0x710);
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
				*((char *)&myData->SubSensV.org_data[0] + i) = inb(0x710);
			}
			myData->SubSensV.phase++;
			break;
		case P5:
			if(myData->SubSensV.signal[SUB_SENS_SIG_DAQ_DIVISION] == P0) {
				outb(0x01, 0x713); //daq1 fifo reset
			} else {
				outb(0x02, 0x713); //daq2 fifo reset
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

void SubSensV_process_7(main_slot)
{ 
	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1)  return;
	if(myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0) return; //kjh_160610
	if(myPs->misc.timer_1sec < 3) return;
	if(myData->daq.config.daq_use == P0) return;

	switch(main_slot) {
		case P0:
			SubSensV_adc_start(); //DAQ
			break;
		case P6:
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
			break;
		case P7:
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
			break;
		case P8:
			if(myPs->config.daq_type < DAQ_TYPE8) { //kjg_150225
				SubSensV_Control2(0); //DAQ
			}
			break;
		case P9:
			if(myPs->config.daq_type < DAQ_TYPE8) { //kjg_150225
				SubSensV_Control2(1); //DAQ
			}
			break;
		default:
			break;
	}
}



void SubSensV_process_12(main_slot)
{ 
	unsigned char state;
	int i, k, j,  bd, base_addr, adc_index=0;
	short int byte1=0;

	if(myData->AppControl.misc.Load_Process_DAQ[0] != P1)  return;
	if(myPs->config.installedAuxV == 0
		&& myPs->config.installedTH == 0) return; //kjh_160610
	if(myPs->misc.timer_1sec < 3) return;
	if(myData->daq.config.daq_use == P0) return;

	switch(main_slot) {
		case P4:
			adc_index = 0;
			for(bd=0; bd < myData->daq.config.installedDAQ; bd++) {
				base_addr = 0x710 + 0x10 * bd;
				state = inb(base_addr + 0x04);
//				if(state == 1){
					k = SUB_SENS_V_ZERO_VAL + 1;
					for(i=0; i < k; i++) {
						inb(base_addr);
					}
					k = 2* (MAX_SUB_SENS_V_REF_INDEX
						+ MAX_SUB_SENS_V_CH_INDEX) * MAX_SUB_SENS_V_AD_SLOT;
					for(i=0; i < k; i++) {
						if(i % 2 == 0)
							byte1 = inb(base_addr) << 8;
						else{
							byte1 |= inb(base_addr);
							if(i < 2 * MAX_SUB_SENS_V_REF_INDEX * MAX_SUB_SENS_V_AD_SLOT){
								myData->SubSensV.org_data[bd]
								.ref_val[(i/8)%3][(i/2)%4][adc_index].val = byte1;
							}else{
								j = i - 2 * MAX_SUB_SENS_V_REF_INDEX * MAX_SUB_SENS_V_AD_SLOT;
								myData->SubSensV.org_data[bd]
								.sub_val[(j/8)%8][(j/2)%4][adc_index].val = byte1;
							}	
						}
					}
//				}
			}
			break;
		case P5:
			adc_index = 1;
			for(bd=0; bd < myData->daq.config.installedDAQ; bd++) {
				base_addr = 0x710 + 0x10 * bd;
				state = inb(base_addr + 0x04);
//				if(state == 1){
					k = SUB_SENS_V_ZERO_VAL + 1;
					for(i=0; i < k; i++) {
						inb(base_addr + 0x01);
					}
					k = 2* (MAX_SUB_SENS_V_REF_INDEX
						+ MAX_SUB_SENS_V_CH_INDEX) * MAX_SUB_SENS_V_AD_SLOT;
					for(i=0; i < k; i++) {
						if(i % 2 == 0)
							byte1 = inb(base_addr + 0x01) << 8;
						else{
							byte1 |= inb(base_addr + 0x01);
							if(i < 2 * MAX_SUB_SENS_V_REF_INDEX * MAX_SUB_SENS_V_AD_SLOT){
								myData->SubSensV.org_data[bd]
								.ref_val[(i/8)%3][(i/2)%4][adc_index].val = byte1;
							}else{
								j = i - 2 * MAX_SUB_SENS_V_REF_INDEX * MAX_SUB_SENS_V_AD_SLOT;
								myData->SubSensV.org_data[bd]
								.sub_val[(j/8)%8][(j/2)%4][adc_index].val = byte1;
							}	
						}
					}
//				}
			}
			break;
		case P6:
			adc_index = 2;
			for(bd=0; bd < myData->daq.config.installedDAQ; bd++) {
				base_addr = 0x710 + 0x10 * bd;
				state = inb(base_addr + 0x04);
//				if(state == 1){
					k = SUB_SENS_V_ZERO_VAL + 1;
					for(i=0; i < k; i++) {
						inb(base_addr + 0x02);
					}
					k = 2* (MAX_SUB_SENS_V_REF_INDEX
						+ MAX_SUB_SENS_V_CH_INDEX) * MAX_SUB_SENS_V_AD_SLOT;
					for(i=0; i < k; i++) {
						if(i % 2 == 0)
							byte1 = inb(base_addr + 0x02) << 8;
						else{
							byte1 |= inb(base_addr + 0x02);
							if(i < 2 * MAX_SUB_SENS_V_REF_INDEX * MAX_SUB_SENS_V_AD_SLOT){
								myData->SubSensV.org_data[bd]
								.ref_val[(i/8)%3][(i/2)%4][adc_index].val = byte1;
							}else{
								j = i - 2 * MAX_SUB_SENS_V_REF_INDEX * MAX_SUB_SENS_V_AD_SLOT;
								myData->SubSensV.org_data[bd]
								.sub_val[(j/8)%8][(j/2)%4][adc_index].val = byte1;
							}	
						}
					}
//				}
			}
			break;
		case P7:
			adc_index = 3;
			for(bd=0; bd < myData->daq.config.installedDAQ; bd++) {
				base_addr = 0x710 + 0x10 * bd;
				state = inb(base_addr + 0x04);
//				if(state == 1){
					k = SUB_SENS_V_ZERO_VAL + 1;
					for(i=0; i < k; i++) {
						inb(base_addr + 0x03);
					}
					k = 2* (MAX_SUB_SENS_V_REF_INDEX
						+ MAX_SUB_SENS_V_CH_INDEX) * MAX_SUB_SENS_V_AD_SLOT;
					for(i=0; i < k; i++) {
						if(i % 2 == 0)
							byte1 = inb(base_addr + 0x03) << 8;
						else{
							byte1 |= inb(base_addr + 0x03);
							if(i < 2 * MAX_SUB_SENS_V_REF_INDEX * MAX_SUB_SENS_V_AD_SLOT){
								myData->SubSensV.org_data[bd]
								.ref_val[(i/8)%3][(i/2)%4][adc_index].val = byte1;
							}else{
								j = i - 2 * MAX_SUB_SENS_V_REF_INDEX * MAX_SUB_SENS_V_AD_SLOT;
								myData->SubSensV.org_data[bd]
								.sub_val[(j/8)%8][(j/2)%4][adc_index].val = byte1;
							}	
						}
					}
//				}
			}
			for(bd=0; bd < myData->daq.config.installedDAQ; bd++) {
				base_addr = 0x710 + 0x10 * bd + 0x01;
				outb(0x01, base_addr); //iso enable
			}
			SubSensV_adc_start();
			break;
		case P8:
			Cal_SubSensV_Source1(MAX_SUB_SENS_V_ADC_INDEX_2);
			Cal_SubSensV_Source2(MAX_SUB_SENS_V_ADC_INDEX_2);
			break;
		case P9:
			Cal_SubSensV_Ch(MAX_SUB_SENS_V_ADC_INDEX_2);
			sub_sens_v_count_increment();
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
