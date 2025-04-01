#include <asm/io.h>
#include <rtl_core.h>
#include <pthread.h>
#include <math.h>
#include "../../INC/datastore.h"
#include "local_utils.h"
#include "ch_utils.h"
#include "Analog.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;
extern S_DIO *myDio;
extern S_GROUP_DATA *myGroup;
extern S_BD_DATA *myBd;
extern S_CH_DATA *myCh;

void Analog_Value_Input(int main_slot)
{
	switch(myPs->misc.module_type) {
		case MODULE_LINEAR:
			Analog_Value_Input_1(main_slot);
			break;
		case MODULE_PWM1:
			Analog_Value_Input_1(main_slot);
			break;
		case MODULE_FCH:
			break;
		case MODULE_PWM2:
			Analog_Value_Input_2();
			break;
		case MODULE_PWM3:
			break;
		default:
			Analog_Value_Input_1(main_slot);
			break;
	}
}

void Analog_Value_Input_1(int main_slot)
{
	int slot, next_slot, bd, ch, end_slot, next_ch, cnt_end=0, installedBd;

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_50A_5A:
		case F_SDI_5V_200A_100A_50A_10A:		//csk_161007
		case F_SBL_5V_250A_125A_65A_10A:
		case F_SDI_5V_450A_200A_100A_10A:
		case F_SDI_5V_450A_200A_100A_10A_2:
		case F_SDI_5V_450A_200A_100A_10A_3:
		case F_SDI_5V_450A_200A_100A_10A_4:
		case F_SDI_5V_450A_200A_100A_10A_96CH:	//hun_160408
		case F_SDI_5V_450A_200A_100A_10A_96CH_2:	//hun_160808
		case F_SDI_5V_450A_200A_100A_10A_96CH_3:	//hun_161201
		case F_SDI_5V_450A_200A_100A_10A_96CH_4:	//hun_170703
			installedBd = 1;
			break;
		case C_SDI_5V_60A_10A_1A:
		case C_SDI_5V_250A_25A_5A:
		case C_SBL_5V_250A_25A_5A:
		case C_SBL_5V_250A_60A_10A:
		case C_SDI_5V_300A_60A_10A:
		case C_SDI_5V_450A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A_2:
			installedBd = -1;
			break;
		default:
			installedBd = myPs->config.installedBd;
			break;
	}
	if(installedBd < 0) return;

	switch(myPs->config.scan_period) {
		case 25: //25ms
			end_slot = 9; //5slot * 2 = 10 (8ch)
			break;
		case 50: //50ms
			end_slot = 19; //5slot * 4 = 20 (16ch)
			break;
		case 100: //100ms
			end_slot = 39; //5slot * 8 = 40 (32ch)
			break;
		default: //10ms
			end_slot = 4; //5slot * 1 = 5 (4ch)
			break;
	}

	slot = myPs->misc.shift_slot + main_slot;
	slot = slot % (end_slot + 1);
	ch = (slot % 5) + (slot / 5 * 4);

	if(main_slot < end_slot) {
		next_slot = slot + 1;
	} else {
		myPs->misc.shift_slot += 4;
		if(myPs->misc.shift_slot > 16) myPs->misc.shift_slot = 0;
		next_slot = myPs->misc.shift_slot;
	}
	next_slot = next_slot % (end_slot + 1);
	next_ch = (next_slot % 5) + (next_slot / 5 * 4);

	if(myData->AppControl.config.debugType == 0) {
		for(bd=0; bd < installedBd; bd++) Get_AD_Value(slot, bd, ch);

		switch(myData->AppControl.config.systemModel) {
			case C_LGC_5V_200A_75A_15A:
			case F_SDI_5V_50A_5A:
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
			//case F_SDI_5V_450A_200A_100A_10A_3:
			//case F_SDI_5V_450A_200A_100A_10A_4:
			//case F_SDI_5V_450A_200A_100A_10A_96CH:	//hun_160408
			//case F_SDI_5V_450A_200A_100A_10A_96CH_2:	//hun_160808
				break;
			default:
				if(slot == 4) {
					cnt_end = bd_sens_count_increment(); //for org
				}
				break;
		}

		for(bd=0; bd < installedBd; bd++) Set_Mux(next_slot, bd, next_ch);
	}

	if((slot % 5) < 4) {
		for(bd=0; bd < installedBd; bd++) {
			switch(myData->AppControl.config.systemModel) {
				case F_SDI_5V_200A_100A_50A_10A:		//csk_161007
				case F_SBL_5V_250A_125A_65A_10A:
				case F_SDI_5V_450A_200A_100A_10A_2:
				case F_SDI_5V_450A_200A_100A_10A_3:
				case F_SDI_5V_450A_200A_100A_10A_4:
				case F_SDI_5V_450A_200A_100A_10A_96CH:	//hun_160408
				case F_SDI_5V_450A_200A_100A_10A_96CH_2:	//hun_160808
				case F_SDI_5V_450A_200A_100A_10A_96CH_3:	//hun_161201
				case F_SDI_5V_450A_200A_100A_10A_96CH_4:	//hun_170703

				//case C_SKI_20V_400A_200A_100A_16KW:			//csk_171023
				//case C_SKI_20V_400A_200A_100A_16KW_2:		//csk_171023
				//case C_SKI_20V_500A_250A_100A_20KW:		//csk_171108
				//case C_SKI_20V_500A_250A_100A_20KW_2:	//csk_171108
				case C_KATECH_20V_1000A_500A_100A: //csk_120309
				case C_LGC_50V_40A_10A_4A: //csk_120206
				case C_LGC_50V_40A_10A_4A_2:
				case C_LGC_50V_40A_10A_4A_3:
				case C_LGC_50V_40A_10A_4A_4:
				case C_SDI_70V_50A_5A_4KW:
				case C_SDI_70V_50A_5A_4KW_2:
				case C_SDI_70V_50A_5A_7KW:
				case C_SDI_70V_50A_5A_7KW_2:
				case C_SDI_70V_250A_25A_18KW:
				case C_SDI_80V_50A_25A_10A:			//csk_130417_s
				case C_SDI_80V_50A_25A_10A_2:
				case C_SDI_80V_50A_25A_10A_3:
				case C_SDI_80V_50A_25A_10A_4:		//csk_130417_e
				case C_KATECH_100V_30A_15A_5A: //csk_131129
				case C_SBL_150V_250A_10A_38KW: //lki_111010
				case C_SBL_150V_250A_10A_75KW_4:
				case C_SBL_150V_250A_10A_75KW_5:
					CalChAverage(bd+1, ch); //for first read Vpower, Vbus
					break;
				default:
					break;
			}

			CalChAverage(bd, ch);
		}
	} else {
		if(myData->AppControl.config.debugType == 110) return;

		switch(myData->AppControl.config.systemModel) {
			case C_LGC_5V_200A_75A_15A:
			case F_SDI_5V_50A_5A:
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
				cnt_end = bd_sens_count_increment(); //for sbl
				break;
			default:
				break;
		}

		for(bd=0; bd < installedBd; bd++) {
			CalSourceAverage(bd, cnt_end);
			CalSourceAverage2(bd);
			CalibratorSource(bd);

			switch(myData->AppControl.config.systemModel) {
				case F_SDI_5V_200A_100A_50A_10A:		//csk_161007
				case F_SBL_5V_250A_125A_65A_10A:
				case F_SDI_5V_450A_200A_100A_10A_2:
				case F_SDI_5V_450A_200A_100A_10A_3:
				case F_SDI_5V_450A_200A_100A_10A_4:
				case F_SDI_5V_450A_200A_100A_10A_96CH:	//hun_160408
				case F_SDI_5V_450A_200A_100A_10A_96CH_2:	//hun_160808
				case F_SDI_5V_450A_200A_100A_10A_96CH_3:	//hun_161201
				case F_SDI_5V_450A_200A_100A_10A_96CH_4:	//hun_170703

				//case C_SKI_20V_400A_200A_100A_16KW:			//csk_171023
				//case C_SKI_20V_400A_200A_100A_16KW_2:		//csk_171023
				//case C_SKI_20V_500A_250A_100A_20KW:		//csk_171108
				//case C_SKI_20V_500A_250A_100A_20KW_2:	//csk_171108
				case C_KATECH_20V_1000A_500A_100A: //csk_120309
				case C_LGC_50V_40A_10A_4A: //kjg_120209
				case C_LGC_50V_40A_10A_4A_2:
				case C_LGC_50V_40A_10A_4A_3:
				case C_LGC_50V_40A_10A_4A_4:
				case C_SDI_70V_50A_5A_4KW:
				case C_SDI_70V_50A_5A_4KW_2:
				case C_SDI_70V_50A_5A_7KW:
				case C_SDI_70V_50A_5A_7KW_2:
				case C_SDI_70V_250A_25A_18KW:
				case C_SDI_80V_50A_25A_10A:			//csk_130417_s
				case C_SDI_80V_50A_25A_10A_2:
				case C_SDI_80V_50A_25A_10A_3:
				case C_SDI_80V_50A_25A_10A_4:		//csk_130417_e
				case C_KATECH_100V_30A_15A_5A: //csk_131129
				case C_SBL_150V_250A_10A_38KW: //lki_111010
				case C_SBL_150V_250A_10A_75KW_4:
				case C_SBL_150V_250A_10A_75KW_5:
					CalSourceAverage(bd+1, cnt_end);
					CalSourceAverage2(bd+1);
					CalibratorSource(bd+1);
					break;
				default:
					break;
			}
		}
	}
}

void Analog_Value_Input_2(void)
{
#ifdef __COB__
	short int tmp, scan_slot, next_scan_slot;
	int bd, ch, next_ch, cnt_end=0, installedBd;

	//kjgw_120320 installedBd = myPs->config.installedBd;
	installedBd = 1;

	scan_slot = (myPs->misc.scan_slot + myPs->misc.shift_slot)
		% myPs->total_scan_slot;
	tmp = myPs->mux_scan[scan_slot];
	if(tmp < SCAN_DIV_1) {
		myPs->misc.channel_slot = tmp - 1;
		myPs->misc.reference_slot = (-1);
	} else {
		myPs->misc.channel_slot = (-1);
		myPs->misc.reference_slot = tmp - SCAN_DIV_1 - 1;
	}

	myPs->misc.scan_slot++;
	if(myPs->misc.scan_slot >= myPs->total_scan_slot) {
		myPs->misc.scan_slot = 0;
		//myPs->misc.shift_slot += 4;
		//if(myPs->misc.shift_slot > 16) myPs->misc.shift_slot = 0;

		cnt_end = bd_sens_count_increment();
	}

	next_scan_slot = (myPs->misc.scan_slot + myPs->misc.shift_slot)
		% myPs->total_scan_slot;
	tmp = myPs->mux_scan[next_scan_slot];
	if(tmp < SCAN_DIV_1) {
		myPs->misc.next_channel_slot = tmp - 1;
		myPs->misc.next_reference_slot = (-1);
	} else {
		myPs->misc.next_channel_slot = (-1);
		myPs->misc.next_reference_slot = tmp - SCAN_DIV_1 - 1;
	}

	ch = (int)myPs->misc.channel_slot;
	for(bd=0; bd < installedBd; bd++) {
		Get_AD_Value(scan_slot, bd, ch);
	}

	next_ch = (int)myPs->misc.next_channel_slot;
	for(bd=0; bd < installedBd; bd++) {
		Set_Mux(next_scan_slot, bd, next_ch);
	}

	if(myPs->misc.channel_slot >= 0) {
		for(bd=0; bd < installedBd; bd++) {
			CalChAverage(bd, ch);
		}
	} else {
		for(bd=0; bd < installedBd; bd++) {
			CalSourceAverage(bd, cnt_end);
			CalSourceAverage2(bd);
			CalibratorSource(bd);
		}
	}
#endif
}

void Get_AD_Value(int slot, int bd, int ch)
{
	switch(myData->AppControl.config.systemModel) {
		//formation
		case F_PNE_5V_15A_30AP_SW:
			Get_AD_Value_7(bd);
			break;
		case F_SDI_5V_50A_5A:
			Get_AD_Value_5(slot, bd, ch);
			break;
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
			Get_AD_Value_6(slot, bd, ch);
			break;

		//cycler
		case C_ENERLAND_5V_250A:
			Get_AD_Value_2(slot, bd, ch);
			break;
		case C_ENERLAND_5V_250A_50A_5A: //kjg_w
		case C_ENERLAND_5V_250A_50A_5A_2: //kjg_w
			Get_AD_Value_3(slot, bd, ch);
			break;
		case C_EIG_5V_50A_5A:
		case C_EIG_5V_50A_5A_2:
		case C_LGC_5V_200A_75A_15A:
			Get_AD_Value_4(slot, bd, ch);
			break;
		default:
			Get_AD_Value_1(slot, bd, ch);
			break;
	}
}

void Get_AD_Value_1(int slot, int bd, int ch)
{
	int	ad_mux, i, ad_count;

	ad_count = (int)myPs->config.ad_count;

	for(ad_mux=0; ad_mux < 4; ad_mux++) {
		outb((unsigned char)ad_mux, CH_ADDR_AD_START);
		usleep(20);
		for(i=0; i < ad_count; i++) {
			Read_AD_Data_1(slot, bd, ch, ad_mux, i);
			Read_AD_Data_1(slot, bd+1, ch, ad_mux, i);
		}
	}

	//capacitor discharge
	outb(0xAA, CH_ADDR_MUX_2);
	//usleep(100);
	usleep(60);

	for(ad_mux=0; ad_mux < 4; ad_mux++) {
		AD_Data_Calculate(slot, bd, ch, ad_mux);
		AD_Data_Calculate(slot, bd+1, ch, ad_mux);
	}
}

void Read_AD_Data_1(int slot, int bd, int ch, int ad_mux, int ad_count)
{
	int ref_mux, addr_ad_data_h, addr_ad_data_l;
	U_ADDA ADValue;

	if(bd == 0) {
		addr_ad_data_h = CH_ADDR_AD_DATA_H_A1;
		addr_ad_data_l = CH_ADDR_AD_DATA_L_A1;
	} else {
		addr_ad_data_h = CH_ADDR_AD_DATA_H_B1;
		addr_ad_data_l = CH_ADDR_AD_DATA_L_B1;
	}

	addr_ad_data_h += (ad_count * 2);
	addr_ad_data_l += (ad_count * 2);

	ADValue.byte[1] = (unsigned char)inb(addr_ad_data_h);
	ADValue.byte[0] = (unsigned char)inb(addr_ad_data_l);

	if(slot < 4) { //V, I
		ch = slot + bd * 4;
		if(ad_mux == 0) { //SEL_V
			myData->cData[ch].misc.sumV[ad_count] = (long)ADValue.val;
		} else if(ad_mux == 1) { //SEL_I
			myData->cData[ch].misc.sumI[ad_count] = (long)ADValue.val;
		} else if(ad_mux == 2) { //TEMP
		} else { //PV
		}
	} else { //sourceV, sourceI
		ref_mux = (int)myPs->misc.ref_muxVal;
		if(ad_mux == 0) { //SEL_V
			myData->bData[bd].misc.source[ref_mux].sumV[ad_count]
				= (long)ADValue.val;
		} else if(ad_mux == 1) { //SEL_I
			myData->bData[bd].misc.source[ref_mux].sumI[ad_count]
				= (long)ADValue.val;
		} else if(ad_mux == 2) { //TEMP
		} else { //PV
		}
	}
}

void Get_AD_Value_2(int slot, int bd, int ch)
{
	unsigned char ad_mux, tmp_mux;
	int	ad_mux_cnt, i, ad_count;

	ad_count = (int)myPs->config.ad_count;
	ad_mux = myPs->misc.ad_muxVal;
	myPs->misc.ad_muxVal++;
	if(myPs->misc.ad_muxVal == 0x04) myPs->misc.ad_muxVal = 0x00;

	// bits-   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	// mux1-      | M_EN3| M_EN2| M_EN1|      |      | C_EN2| C_EN1|
	// mux2-      |      | M_A6 | M_A5 | M_A4 | M_A3 | M_A2 | M_A1 |
	// mux3-      |      |      |      |      | C_A3 | C_A2 | C_A1 |
	
	for(ad_mux_cnt=0; ad_mux_cnt < 4; ad_mux_cnt++) {
		ad_mux &= 0x03;
		if(slot < 4) {
			tmp_mux = 0x0F;
		} else {
			tmp_mux = myPs->misc.ref_muxVal;
			tmp_mux |= (tmp_mux << 2);
		}
		outb((ad_mux << 4) | tmp_mux, CH_ADDR_MUX_2);
		usleep(15);
		if(ad_mux < 2) {
			for(i=0; i < ad_count; i++) {
				outb(0x00, CH_ADDR_AD_START);
				usleep(15);
				Read_AD_Data_2(slot, bd, ch, (int)ad_mux, i);
			}
		}
		ad_mux++;
	}

	//capacitor discharge
	outb(0x70, CH_ADDR_MUX_1);
	outb(0x3F, CH_ADDR_MUX_2);
	usleep(10);

	for(ad_mux_cnt=0; ad_mux_cnt < 4; ad_mux_cnt++) {
		ad_mux &= 0x03;
		AD_Data_Calculate(slot, bd, ch, (int)ad_mux);
		ad_mux++;
	}
}

void Read_AD_Data_2(int slot, int bd, int ch, int ad_mux, int ad_count)
{
	int ref_mux;
	U_ADDA ADValue;
	
	ADValue.byte[1] = (unsigned char)inb(CH_ADDR_AD_DATA_H_A1);
	ADValue.byte[0] = (unsigned char)inb(CH_ADDR_AD_DATA_L_A1);

	if(slot < 4) { //V, I
		ch = slot + bd * 4;
		if(ad_mux == 0) { //SEL_V
			myData->cData[ch].misc.sumV[ad_count] = (long)ADValue.val;
		} else if(ad_mux == 1) { //SEL_I
			myData->cData[ch].misc.sumI[ad_count] = (long)ADValue.val;
		} else if(ad_mux == 2) { //TEMP
		} else { //PV
		}
	} else { //sourceV, sourceI
		ref_mux = (int)myPs->misc.ref_muxVal;
		if(ad_mux == 0) { //SEL_V
			myData->bData[bd].misc.source[ref_mux].sumV[ad_count]
				= (long)ADValue.val;
		} else if(ad_mux == 1) { //SEL_I
			myData->bData[bd].misc.source[ref_mux].sumI[ad_count]
				= (long)ADValue.val;
		} else if(ad_mux == 2) { //TEMP
		} else { //PV
		}
	}
}

void Get_AD_Value_3(int slot, int bd, int ch)
{
}

void Read_AD_Data_3(int slot, int bd, int ch, int ad_mux, int ad_count)
{
}

void Get_AD_Value_4(int slot, int bd, int ch)
{
	unsigned char mux2;
	int	ad_mux, i, ad_count;

	ad_count = (int)myPs->config.ad_count;

	// bits-   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	// mux1-      | M_EN3| M_EN2| M_EN1|      |      | C_EN2| C_EN1|
	// mux2-      |      | M_A6 | M_A5 | M_A4 | M_A3 | M_A2 | M_A1 |
	// mux3-      |      |      |      |      | C_A3 | C_A2 | C_A1 |

	if((slot % 5) < 4) {
		mux2 = 0x0F;
	} else {
		mux2 = myPs->misc.ref_muxVal;
		mux2 |= (mux2 << 2);
	}

	for(ad_mux=0; ad_mux < 4; ad_mux++) {
		outb(mux2 | (ad_mux << 4), CH_ADDR_MUX_2);
		usleep(40);

		for(i=0; i < ad_count; i++) {
			outb(0, CH_ADDR_AD_START);
			usleep(11);
			Read_AD_Data_4(slot, bd, ch, ad_mux, i);
		}
	}

	//capacitor discharge
	outb(0x70, CH_ADDR_MUX_1);
	outb(0x3F, CH_ADDR_MUX_2);
	usleep(10);

	for(ad_mux=0; ad_mux < 4; ad_mux++) {
		AD_Data_Calculate(slot, bd, ch, ad_mux);
	}
}

void Read_AD_Data_4(int slot, int bd, int ch, int ad_mux, int ad_count)
{
	int ref_mux;
	U_ADDA ADValue;
	
	ADValue.byte[1] = (unsigned char)inb(CH_ADDR_AD_DATA_H_A1);
	ADValue.byte[0] = (unsigned char)inb(CH_ADDR_AD_DATA_L_A1);

	if((slot % 5) < 4) { //V, I
		ch += (bd * myPs->config.chPerBd);
		if(ad_mux == 0) { //SEL_V
			myData->cData[ch].misc.sumV[ad_count] = (long)ADValue.val;
		} else if(ad_mux == 1) { //SEL_I
			myData->cData[ch].misc.sumI[ad_count] = (long)ADValue.val;
		} else if(ad_mux == 2) { //TEMP
		} else { //PV
		}
	} else { //sourceV, sourceI
		ref_mux = (int)myPs->misc.ref_muxVal;
		if(ad_mux == 0) { //SEL_V
			myData->bData[bd].misc.source[ref_mux].sumV[ad_count]
				= (long)ADValue.val;
		} else if(ad_mux == 1) { //SEL_I
			myData->bData[bd].misc.source[ref_mux].sumI[ad_count]
				= (long)ADValue.val;
		} else if(ad_mux == 2) { //TEMP
		} else { //PV
		}
	}
}

void Get_AD_Value_5(int slot, int bd, int ch)
{
	unsigned char mux2;
	int	ad_mux, i, ad_count;

	ad_count = (int)myPs->config.ad_count;

	// bits-   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	// mux1-      | M_EN3| M_EN2| M_EN1| C_EN4| C_EN3| C_EN2| C_EN1|
	// mux2-      |      | M_A6 | M_A5 | M_A4 | M_A3 | M_A2 | M_A1 |
	// mux3-      |      |      |      |      | C_A3 | C_A2 | C_A1 |

	if((slot % 5) < 4) {
		mux2 = 0x0F;
	} else {
		mux2 = myPs->misc.ref_muxVal;
		mux2 |= (mux2 << 2);
	}

	for(ad_mux=0; ad_mux < 4; ad_mux++) {
		outb(mux2 | (ad_mux << 4), CH_ADDR_MUX_2);
		usleep(40);

		for(i=0; i < ad_count; i++) {
			outb(0, CH_ADDR_AD_START);
			usleep(11);
			Read_AD_Data_5(slot, bd, ch, ad_mux, i);
		}
	}

	//capacitor discharge
	outb(0x70, CH_ADDR_MUX_1);
	outb(0x3F, CH_ADDR_MUX_2);
	usleep(10);

	for(ad_mux=0; ad_mux < 4; ad_mux++) {
		AD_Data_Calculate(slot, bd, ch, ad_mux);
	}
}

void Read_AD_Data_5(int slot, int bd, int ch, int ad_mux, int ad_count)
{
	int ref_mux;
	U_ADDA ADValue;

	ADValue.byte[1] = (unsigned char)inb(CH_ADDR_AD_DATA_H_A1);
	ADValue.byte[0] = (unsigned char)inb(CH_ADDR_AD_DATA_L_A1);

	if((slot % 5) < 4) { //V, I
		ch += (bd * myPs->config.chPerBd);
		if(ch >= MAX_CH_PER_MODULE) return;

		if(ad_mux == 0) { //SEL_V
			myData->cData[ch].misc.sumV[ad_count] = (long)ADValue.val;
		} else if(ad_mux == 1) { //SEL_I
			myData->cData[ch].misc.sumI[ad_count] = (long)ADValue.val;
		} else if(ad_mux == 2) { //TEMP
		} else { //PV
		}
	} else { //sourceV, sourceI
		ref_mux = (int)myPs->misc.ref_muxVal;
		if(ad_mux == 0) { //SEL_V
			myData->bData[bd].misc.source[ref_mux].sumV[ad_count]
				= (long)ADValue.val;
		} else if(ad_mux == 1) { //SEL_I
			myData->bData[bd].misc.source[ref_mux].sumI[ad_count]
				= (long)ADValue.val;
		} else if(ad_mux == 2) { //TEMP
		} else { //PV
		}
	}
}

void Get_AD_Value_6(int slot, int bd, int ch)
{
	unsigned char mux2;
	int	ad_mux, i, ad_count;

	ad_count = (int)myPs->config.ad_count;

	// bits-   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	// mux1-      | M_EN3| M_EN2| M_EN1|      |      |      |      |
	// mux2-      |      | M_A6 | M_A5 | M_A4 | M_A3 | M_A2 | M_A1 |

	if((slot % 5) < 4) {
		mux2 = 0x02;
	} else {
		if(myPs->misc.ref_muxVal <= 0x01) {
			mux2 = myPs->misc.ref_muxVal;
		} else if(myPs->misc.ref_muxVal == 0x02) {
			mux2 = 0x03;
		} else {
			mux2 = 0x02;
		}
	}
	mux2 |= (mux2 << 2);

	for(ad_mux=0; ad_mux < 4; ad_mux++) {
		outb(mux2 | (ad_mux << 4), CH_ADDR_MUX_2);
		usleep(40);

		for(i=0; i < ad_count; i++) {
			outb(0, CH_ADDR_AD_START);
			usleep(11);
			Read_AD_Data_6(slot, bd, ch, ad_mux, i);
		}
	}

	//capacitor discharge
	outb(0x70, CH_ADDR_MUX_1);
	outb(0x3F, CH_ADDR_MUX_2);
	usleep(10);

	for(ad_mux=0; ad_mux < 4; ad_mux++) {
		AD_Data_Calculate(slot, bd, ch, ad_mux);
	}
}

void Read_AD_Data_6(int slot, int bd, int ch, int ad_mux, int ad_count)
{
	int ref_mux;
	U_ADDA ADValue;
	
	ADValue.byte[1] = (unsigned char)inb(CH_ADDR_AD_DATA_H_A1);
	ADValue.byte[0] = (unsigned char)inb(CH_ADDR_AD_DATA_L_A1);

	if((slot % 5) < 4) { //V, I
		ch += (bd * myPs->config.chPerBd);
		if(ad_mux == 0) { //SEL_V
			myData->cData[ch].misc.sumV[ad_count] = (long)ADValue.val;
		} else if(ad_mux == 1) { //SEL_I
			myData->cData[ch].misc.sumI[ad_count] = (long)ADValue.val;
		} else if(ad_mux == 2) { //TEMP
		} else { //PV
		}
	} else { //sourceV, sourceI
		ref_mux = (int)myPs->misc.ref_muxVal;
		if(ad_mux == 0) { //SEL_V
			myData->bData[bd].misc.source[ref_mux].sumV[ad_count]
				= (long)ADValue.val;
		} else if(ad_mux == 1) { //SEL_I
			myData->bData[bd].misc.source[ref_mux].sumI[ad_count]
				= (long)ADValue.val;
		} else if(ad_mux == 2) { //TEMP
		} else { //PV
		}
	}
}

void Get_AD_Value_7(int bd)
{
	unsigned char tmp;
	int	ch, i, ad_count, addr, ad_mux;

	ad_count = (int)myPs->config.ad_count;

	// bits-   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	// mux1-      | M_A3 | M_A2 | M_A1 |      | M_EN3| M_EN2| M_EN1|
	// mux2-      | M_A7 | M_A6 | M_A5 |      | M_EN7| M_EN6| M_EN5|
	// mux3-      |      | C_EN2| C_EN1|      | C_A3 | C_A2 | C_A1 |
	// mux4-      |      |      |      |      |      | SEL2 | SEL1 |

	if(myPs->misc.channel_slot >= 0) { //channel
		ch = (int)myPs->misc.channel_slot;

		addr = CH_ADDR_MUX_4 + 0x10 * bd;
		outb(0x03, addr);

		for(ad_mux=0; ad_mux < 2; ad_mux++) {
			tmp = (unsigned char)ad_mux << 4;
			tmp |= 0x02;
			outb(tmp, CH_ADDR_MUX_1);
			usleep(30);

			for(i=0; i < ad_count; i++) {
				outb(0, CH_ADDR_AD_START);
				usleep(10);
				Read_AD_Data_7(bd, ch, ad_mux, i);
			}
		}

		outb(0x00, addr);

		for(ad_mux=0; ad_mux < 2; ad_mux++) {
			AD_Data_Calculate(0, bd, ch, ad_mux);
		}
	} else { //reference
		//kjgw_120320 if(bd > 0) return;

		ch = (int)myPs->misc.reference_slot;

		for(i=0; i < ad_count; i++) {
			outb(0, CH_ADDR_AD_START);
			usleep(10);
			Read_AD_Data_7(bd, ch, 0, i);
		}

		AD_Data_Calculate(0, bd, ch, 0);
	}
}

void Read_AD_Data_7(int bd, int ch, int ad_mux, int ad_count)
{
	U_ADDA ADValue[2];
	
	ADValue[0].byte[1] = (unsigned char)inb(CH_ADDR_AD_DATA_H_A1);
	ADValue[0].byte[0] = (unsigned char)inb(CH_ADDR_AD_DATA_L_A1);

	ADValue[1].byte[1] = (unsigned char)inb(CH_ADDR_AD_DATA_H_A2);
	ADValue[1].byte[0] = (unsigned char)inb(CH_ADDR_AD_DATA_L_A2);

	if(myPs->misc.channel_slot >= 0) { //channel
		if(ad_mux == 0) { //SEL_V, I
			myData->cData[ch].misc.sumV[ad_count] = (long)ADValue[0].val;
			myData->cData[ch].misc.sumI[ad_count] = (long)ADValue[1].val;
		} else if(ad_mux == 1) { //SEL_VP, VB //kjg_w
		} else { //SEL_spare
		}
	} else { //reference
		if(myPs->misc.reference_slot < 4) {
			ad_mux = (int)myPs->misc.reference_slot;
			myData->bData[bd].misc.source[ad_mux].sumV[ad_count]
				= (long)ADValue[0].val;
			myData->bData[bd].misc.source[ad_mux].sumI[ad_count]
				= (long)ADValue[1].val;
		} else { //kjg_w Th
		}
	}
}

void AD_Data_Calculate(int slot, int bd, int ch, int ad_mux)
{
	switch(myData->AppControl.config.systemModel) {
		case F_PNE_5V_15A_30AP_SW:
			AD_Data_Calculate_2(bd, ch, ad_mux);
			break;
		default:
			AD_Data_Calculate_1(slot, bd, ch, ad_mux);
			break;
	}
}

void AD_Data_Calculate_1(int slot, int bd, int ch, int ad_mux)
{
	int ref_mux, i, ad_count;
	float avg, max, min, val;

	ad_count = (int)myPs->config.ad_count;

	if((slot % 5) < 4) { //V, I
		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_200A_100A_50A_10A:		//csk_161007
			case F_SBL_5V_250A_125A_65A_10A:
			case F_SDI_5V_450A_200A_100A_10A_2:
			case F_SDI_5V_450A_200A_100A_10A_3:
			case F_SDI_5V_450A_200A_100A_10A_4:
			case F_SDI_5V_450A_200A_100A_10A_96CH:	//hun_160408
			case F_SDI_5V_450A_200A_100A_10A_96CH_2:	//hun_160808
			case F_SDI_5V_450A_200A_100A_10A_96CH_3:	//hun_161201
			case F_SDI_5V_450A_200A_100A_10A_96CH_4:	//hun_170703
				ch += (bd * 4);
				break;
			default:
				ch += (bd * myPs->config.chPerBd);
				break;
		}
		if(ch >= MAX_CH_PER_MODULE) return;

		if(ad_mux == 0) { //SEL_V
			avg = (float)myData->cData[ch].misc.sumV[0];
			max = min = avg;
			for(i=1; i < ad_count; i++) {
				val = (float)myData->cData[ch].misc.sumV[i];
				avg += val;
				if(val > max) max = val;
				if(val < min) min = val;
			}
			if(ad_count > 3) {
				avg = (avg - max - min) / (float)(ad_count - 2);
			} else {
				avg /= (float)ad_count;
			}

			switch(myData->AppControl.config.systemModel) {
				case C_KATECH_20V_1000A_500A_100A: //csk_120309
				case C_LGC_50V_40A_10A_4A: //csk_120206
				case C_LGC_50V_40A_10A_4A_2:
				case C_LGC_50V_40A_10A_4A_3:
				case C_LGC_50V_40A_10A_4A_4:
				case C_SDI_70V_50A_5A_4KW:
				case C_SDI_70V_50A_5A_4KW_2:
				case C_SDI_70V_50A_5A_7KW:
				case C_SDI_70V_50A_5A_7KW_2:
				case C_SDI_70V_250A_25A_18KW:
				case C_SDI_80V_50A_25A_10A:		//csk_130417_s
				case C_SDI_80V_50A_25A_10A_2:
				case C_SDI_80V_50A_25A_10A_3:
				case C_SDI_80V_50A_25A_10A_4:	//csk_130417_e
				case C_KATECH_100V_30A_15A_5A: //csk_131129
				case C_SBL_150V_250A_10A_38KW: //lki_111010
				case C_SBL_150V_250A_10A_75KW_4:
				case C_SBL_150V_250A_10A_75KW_5:
					if(ch < 4) {
						AD_Data_Calculate_V(bd, ch, ad_mux, avg, 0);
					} else {
						AD_Data_Calculate_V(bd, ch, ad_mux, avg, 1);
					}
					break;
				default:
					AD_Data_Calculate_V(bd, ch, ad_mux, avg, 0);
					break;
			}
		} else if(ad_mux == 1) { //SEL_I
			avg = (float)myData->cData[ch].misc.sumI[0];
			max = min = avg;
			for(i=1; i < ad_count; i++) {
				val = (float)myData->cData[ch].misc.sumI[i];
				avg += val;
				if(val > max) max = val;
				if(val < min) min = val;
			}
			if(ad_count > 3) {
				avg = (avg - max - min) / (float)(ad_count - 2);
			} else {
				avg /= (float)ad_count;
			}

			switch(myData->AppControl.config.systemModel) {
				case C_KATECH_20V_1000A_500A_100A: //csk_120309
				case C_LGC_50V_40A_10A_4A: //csk_120206
				case C_LGC_50V_40A_10A_4A_2:
				case C_LGC_50V_40A_10A_4A_3:
				case C_LGC_50V_40A_10A_4A_4:
				case C_SDI_70V_50A_5A_4KW:
				case C_SDI_70V_50A_5A_4KW_2:
				case C_SDI_70V_50A_5A_7KW:
				case C_SDI_70V_50A_5A_7KW_2:
				case C_SDI_70V_250A_25A_18KW:
				case C_SDI_80V_50A_25A_10A:		//csk_130417_s
				case C_SDI_80V_50A_25A_10A_2:
				case C_SDI_80V_50A_25A_10A_3:
				case C_SDI_80V_50A_25A_10A_4:	//csk_130417_e
				case C_KATECH_100V_30A_15A_5A: //csk_131129
				case C_SBL_150V_250A_10A_38KW: //lki_111010
				case C_SBL_150V_250A_10A_75KW_4:
				case C_SBL_150V_250A_10A_75KW_5:
					if(ch < 4) {
						AD_Data_Calculate_I(bd, ch, ad_mux, avg, 0);
					} else {
						AD_Data_Calculate_V(bd, ch, ad_mux, avg, 2);
					}
					break;
				default:
					AD_Data_Calculate_I(bd, ch, ad_mux, avg, 0);
					break;
			}
		} else if(ad_mux == 2) { //TEMP
		} else { //PV
		}
	} else { //sourceV, sourceI
		ref_mux = (int)myPs->misc.ref_muxVal;
		if(ad_mux == 0) { //SEL_V
			avg = (float)myData->bData[bd].misc.source[ref_mux].sumV[0];
			max = min = avg;
			for(i=1; i < ad_count; i++) {
				val = (float)myData->bData[bd].misc.source[ref_mux].sumV[i];
				avg += val;
				if(val > max) max = val;
				if(val < min) min = val;
			}
			if(ad_count > 3) {
				avg = (avg - max - min) / (float)(ad_count - 2);
			} else {
				avg /= (float)ad_count;
			}
			i = myPs->misc.bd_sensCount;
			myData->bData[bd].misc.source[ref_mux].sensSumV[i] = (long)avg;
		} else if(ad_mux == 1) { //SEL_I
			avg = (float)myData->bData[bd].misc.source[ref_mux].sumI[0];
			max = min = avg;
			for(i=1; i < ad_count; i++) {
				val = (float)myData->bData[bd].misc.source[ref_mux].sumI[i];
				avg += val;
				if(val > max) max = val;
				if(val < min) min = val;
			}
			if(ad_count > 3) {
				avg = (avg - max - min) / (float)(ad_count - 2);
			} else {
				avg /= (float)ad_count;
			}
			i = myPs->misc.bd_sensCount;
			myData->bData[bd].misc.source[ref_mux].sensSumI[i] = (long)avg;
		} else if(ad_mux == 2) { //TEMP
		} else { //PV
		}
	}
}

void AD_Data_Calculate_2(int bd, int ch, int ad_mux)
{
	int ref_mux, i, ad_count;
	float avg, max, min, val;

	ad_count = (int)myPs->config.ad_count;

	if(myPs->misc.channel_slot >= 0) { //channel
		if(ad_mux == 0) { //SEL_V, I
			avg = (float)myData->cData[ch].misc.sumV[0];
			max = min = avg;
			for(i=1; i < ad_count; i++) {
				val = (float)myData->cData[ch].misc.sumV[i];
				avg += val;
				if(val > max) max = val;
				if(val < min) min = val;
			}
			if(ad_count > 3) {
				avg = (avg - max - min) / (float)(ad_count - 2);
			} else {
				avg /= (float)ad_count;
			}

			AD_Data_Calculate_V(0, ch, 0, avg, 0);

			avg = (float)myData->cData[ch].misc.sumI[0];
			max = min = avg;
			for(i=1; i < ad_count; i++) {
				val = (float)myData->cData[ch].misc.sumI[i];
				avg += val;
				if(val > max) max = val;
				if(val < min) min = val;
			}
			if(ad_count > 3) {
				avg = (avg - max - min) / (float)(ad_count - 2);
			} else {
				avg /= (float)ad_count;
			}

			AD_Data_Calculate_I(0, ch, 0, avg, 0);
		} else if(ad_mux == 1) { //SEL_VP, VB
			avg = (float)myData->cData[ch].misc.sumVp[0];
			max = min = avg;
			for(i=1; i < ad_count; i++) {
				val = (float)myData->cData[ch].misc.sumVp[i];
				avg += val;
				if(val > max) max = val;
				if(val < min) min = val;
			}
			if(ad_count > 3) {
				avg = (avg - max - min) / (float)(ad_count - 2);
			} else {
				avg /= (float)ad_count;
			}

			AD_Data_Calculate_Vp(0, ch, 0, avg);

			avg = (float)myData->cData[ch].misc.sumVb[0];
			max = min = avg;
			for(i=1; i < ad_count; i++) {
				val = (float)myData->cData[ch].misc.sumVb[i];
				avg += val;
				if(val > max) max = val;
				if(val < min) min = val;
			}
			if(ad_count > 3) {
				avg = (avg - max - min) / (float)(ad_count - 2);
			} else {
				avg /= (float)ad_count;
			}

			AD_Data_Calculate_Vb(0, ch, 0, avg);
		} else { //SEL_spare
		}
	} else { //reference
		ref_mux = (int)myPs->misc.ref_muxVal;
		avg = (float)myData->bData[bd].misc.source[ref_mux].sumV[0];
		max = min = avg;
		for(i=1; i < ad_count; i++) {
			val = (float)myData->bData[bd].misc.source[ref_mux].sumV[i];
			avg += val;
			if(val > max) max = val;
			if(val < min) min = val;
		}
		if(ad_count > 3) {
			avg = (avg - max - min) / (float)(ad_count - 2);
		} else {
				avg /= (float)ad_count;
		}
		i = myPs->misc.bd_sensCount;
		myData->bData[bd].misc.source[ref_mux].sensSumV[i] = (long)avg;

		avg = (float)myData->bData[bd].misc.source[ref_mux].sumI[0];
		max = min = avg;
		for(i=1; i < ad_count; i++) {
			val = (float)myData->bData[bd].misc.source[ref_mux].sumI[i];
			avg += val;
			if(val > max) max = val;
			if(val < min) min = val;
		}
		if(ad_count > 3) {
			avg = (avg - max - min) / (float)(ad_count - 2);
		} else {
			avg /= (float)ad_count;
		}
		i = myPs->misc.bd_sensCount;
		myData->bData[bd].misc.source[ref_mux].sensSumI[i] = (long)avg;
	}
}

void AD_Data_Calculate_V(int bd, int ch, int ad_mux, float avg, int div)
{
	int i, range, point, tmp_ch;
	float ratio;
	double tmp;
	int minus_cmd;	//jhkw_180124

	range = (int)myData->cData[ch].op.rangeV;
	ratio = Ref_V_Ratio(range);

	tmp = (double)avg * (double)ratio;
	if(tmp >= 0.0) {
		tmp = tmp * myData->bData[bd].misc.Vsource_AD_a
			+ myData->bData[bd].misc.Vsource_AD_b;
	} else {
		tmp = tmp * myData->bData[bd].misc.Vsource_AD_a_N
			+ myData->bData[bd].misc.Vsource_AD_b_N;
	}

	i = myData->cData[ch].misc.sensCount;

	//jhkw_180124s
	minus_cmd = 0;
	switch(myData->AppControl.config.systemModel) {
		case C_SKI_50V_500A_250A_25KW:	//ktg_190715
		case C_SKI_50V_500A_400A_200A_100A_100KW:
		case C_SKI_50V_500A_400A_200A_100A_100KW_2:
		case C_SKI_50V_1000A_500A_300A_100KW:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_2:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_3:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_4:	//ktg_190711
		case C_KTC_100V_500A_300A_100A_100KW:   //phb_221117
		case C_LGC_130V_500A_300A_100A_130KW:   //sec_220906
		case C_KTC_200V_500A_300A_100A_200KW:   //phb_221104
		case C_LGC_500V_300A_100A_300KW:	//ktg_190418
		case C_KTL_600V_200A_100A_20A_400KW:	//jhk_180731
		case C_KCL_600V_200A_100A_20A_240KW:    //phb_230706
		case C_LGC_600V_250A_100A_50A_300KW:    //phb_220907
		case C_SDI_600V_300A_100A_90KW:		//ktg_200809
			if(tmp < 0) {
				minus_cmd = 1;
			} else {
				minus_cmd = 0;
			}
			break;
		default:
			minus_cmd = 0;
			break;
	}
	//jhkw_180124e

	if(div == 0) {
		if(myData->cData[ch].op.state == C_CALI
			&& myData->cali[ch].type == CALI_TYPE_VOLTAGE) {
			if(myData->cData[ch].op.phase <= P10) {
				myData->cData[ch].misc.sensSumV[i] = (long)tmp;
			} else {
				point = cFindADCaliPoint(ch, tmp, ad_mux, range, 0);
				myData->cData[ch].misc.sensSumV[i]
					= (long)(tmp * myData->cali[ch]
					.tmpData[ad_mux][range].AD_A[point]
					+ myData->cali[ch].tmpData[ad_mux][range].AD_B[point]);
			}
		} else {
			point = cFindADCaliPoint(ch, tmp, ad_mux, range, 1);
			//jhkw_180124s
			if(minus_cmd == 1) {
				myData->cData[ch].misc.sensSumV[i]
					= (long)(tmp 
					* myData->cali[ch].data[ad_mux][range].AD_A[point]
					+ (myData->cali[ch].data[ad_mux][range].AD_B[point] * (-1)));
			} else {
			//jhkw_180124e
				myData->cData[ch].misc.sensSumV[i]
					= (long)(tmp 
					* myData->cali[ch].data[ad_mux][range].AD_A[point]
					+ myData->cali[ch].data[ad_mux][range].AD_B[point]);
			}
		}
	} else if(div == 1) {
		switch(myData->AppControl.config.systemModel) {
			case C_SBL_150V_250A_10A_38KW: //lki_111010
				switch(ch) {
					case 4:
						tmp_ch = ch - 4;
						break;
					case 5:
					case 6:
						tmp_ch = ch - 5;
						break;
					case 7:
						tmp_ch = ch - 6;
						break;
					default:
						tmp_ch = ch - 4;
						break;
				}
				break;
			default:
				tmp_ch = ch - 4;
				break;
		}

		if(myData->cData[tmp_ch].op.state == C_CALI
			&& myData->cali[tmp_ch].type == CALI_TYPE_VOLTAGE) {
			if(myData->cData[tmp_ch].op.phase <= P10) {
				myData->cData[ch].misc.sensSumV[i] = (long)tmp;
			} else {
				point = cFindADCaliPoint(ch, tmp, ad_mux, range, 0);
				myData->cData[ch].misc.sensSumV[i]
					= (long)(tmp * myData->cali[ch]
					.tmpData[ad_mux][range].AD_A[point]
					+ myData->cali[ch].tmpData[ad_mux][range].AD_B[point]);
			}
		} else {
			point = cFindADCaliPoint(ch, tmp, ad_mux, range, 1);
			//jhkw_180124s
			if(minus_cmd == 1) {
				myData->cData[ch].misc.sensSumV[i]
					= (long)(tmp
					* myData->cali[ch].data[ad_mux][range].AD_A[point]
					+ (myData->cali[ch].data[ad_mux][range].AD_B[point] * (-1)));
			} else {
			//jhkw_180124e
				myData->cData[ch].misc.sensSumV[i]
					= (long)(tmp 
					* myData->cali[ch].data[ad_mux][range].AD_A[point]
					+ myData->cali[ch].data[ad_mux][range].AD_B[point]);
			}
		}
	} else {
		switch(myData->AppControl.config.systemModel) {
			case C_SBL_150V_250A_10A_38KW: //lki_111010
				switch(ch) {
					case 4:
						tmp_ch = ch - 4;
						break;
					case 5:
					case 6:
						tmp_ch = ch - 5;
						break;
					case 7:
						tmp_ch = ch - 6;
						break;
					default:
						tmp_ch = ch - 4;
						break;
				}
				break;
			default:
				tmp_ch = ch - 4;
				break;
		}

		if(myData->cData[tmp_ch].op.state == C_CALI
			&& myData->cali[tmp_ch].type == CALI_TYPE_VOLTAGE) {
			if(myData->cData[tmp_ch].op.phase <= P10) {
				myData->cData[ch].misc.sensSumI[i] = (long)tmp;
			} else {
				point = cFindADCaliPoint(ch, tmp, ad_mux, range, 0);
				myData->cData[ch].misc.sensSumI[i]
					= (long)(tmp * myData->cali[ch]
					.tmpData[ad_mux][range].AD_A[point]
					+ myData->cali[ch].tmpData[ad_mux][range].AD_B[point]);
			}
		} else {
			point = cFindADCaliPoint(ch, tmp, ad_mux, range, 1);
			myData->cData[ch].misc.sensSumI[i]
				= (long)(tmp * myData->cali[ch].data[ad_mux][range].AD_A[point]
				+ myData->cali[ch].data[ad_mux][range].AD_B[point]);
		}
	}
}

void AD_Data_Calculate_I(int bd, int ch, int ad_mux, float avg, int div)
{
	int i, range, point;
	float ratio;
	double tmp;

	if(myData->cData[ch].op.state == C_RUN) {
		if(myData->cData[ch].misc.semiSwitchState >= SEMI_I_RANGE1
			&& myData->cData[ch].misc.semiSwitchState <= SEMI_I_RANGE4) {
			range = (int)myData->cData[ch].misc.semiSwitchState - 3;
		} else {
			range = (int)myData->cData[ch].op.rangeI;
		}
	} else {
		range = (int)myData->cData[ch].op.rangeI;
	}
	ratio = Ref_I_Ratio(range);

	tmp = (double)avg * (double)ratio;
	if(tmp >= 0.0) {
		tmp = tmp * myData->bData[bd].misc.Isource_AD_a
			+ myData->bData[bd].misc.Isource_AD_b;
	} else {
		tmp = tmp * myData->bData[bd].misc.Isource_AD_a_N
			+ myData->bData[bd].misc.Isource_AD_b_N;
	}

	i = myData->cData[ch].misc.sensCount;
	if(myData->cData[ch].op.state == C_CALI
		&& myData->cali[ch].type == CALI_TYPE_CURRENT) {
		if(myData->cData[ch].op.phase <= P20) {
			myData->cData[ch].misc.sensSumI[i] = (long)tmp;
		} else {
			point = cFindADCaliPoint(ch, tmp, ad_mux, range, 0);
			myData->cData[ch].misc.sensSumI[i]
				= (long)(tmp * myData->cali[ch]
				.tmpData[ad_mux][range].AD_A[point]
				+ myData->cali[ch].tmpData[ad_mux][range].AD_B[point]);
		}
	} else {
		point = cFindADCaliPoint(ch, tmp, ad_mux, range, 1);
		//myData->cData[ch].misc.sensSumI[i]
		//	= (long)(tmp * myData->cali[ch].data[ad_mux][range].AD_A[point]
		//	+ myData->cali[ch].data[ad_mux][range].AD_B[point]);
		//csk_170106d_s
		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_200A_100A_50A_10A:
			case F_SDI_5V_400A_200A_100A_10A:
				if((myData->cData[ch].op.stepType == STEP_OCV) ||
						(myData->cData[ch].op.stepType == STEP_REST)) {
					myData->cData[ch].misc.sensSumI[i] = 0;
				} else {
					myData->cData[ch].misc.sensSumI[i]
					= (long)(tmp 
						* myData->cali[ch].data[ad_mux][range].AD_A[point]
						+ myData->cali[ch].data[ad_mux][range].AD_B[point]);
				}
				break;
			default:
				myData->cData[ch].misc.sensSumI[i]
				= (long)(tmp * myData->cali[ch].data[ad_mux][range].AD_A[point]
					+ myData->cali[ch].data[ad_mux][range].AD_B[point]);
				break;
		}			//csk_170106d_e
	}	
}

void AD_Data_Calculate_Vp(int bd, int ch, int ad_mux, float avg)
{
	int i, range, point;
	float ratio;
	double tmp;

	range = (int)myData->cData[ch].op.rangeV;
	ratio = Ref_V_Ratio(range);

	tmp = (double)avg * (double)ratio;
	if(tmp >= 0.0) {
		tmp = tmp * myData->bData[bd].misc.Vsource_AD_a
			+ myData->bData[bd].misc.Vsource_AD_b;
	} else {
		tmp = tmp * myData->bData[bd].misc.Vsource_AD_a_N
			+ myData->bData[bd].misc.Vsource_AD_b_N;
	}

	i = myData->cData[ch].misc.sensCount;

	if(myData->cData[ch].op.state == C_CALI
		&& myData->cali[ch].type == CALI_TYPE_VOLTAGE) {
		if(myData->cData[ch].op.phase <= P10) {
			myData->cData[ch].misc.sensSumVp[i] = (long)tmp;
		} else {
			point = cFindADCaliPoint(ch, tmp, ad_mux, range, 0);
			myData->cData[ch].misc.sensSumVp[i]
				= (long)(tmp * myData->cali[ch]
				.tmpData[ad_mux][range].AD_A[point]
				+ myData->cali[ch].tmpData[ad_mux][range].AD_B[point]);
		}
	} else {
		point = cFindADCaliPoint(ch, tmp, ad_mux, range, 1);
		myData->cData[ch].misc.sensSumVp[i]
			= (long)(tmp * myData->cali[ch].data[ad_mux][range].AD_A[point]
			+ myData->cali[ch].data[ad_mux][range].AD_B[point]);
	}
}

void AD_Data_Calculate_Vb(int bd, int ch, int ad_mux, float avg)
{
	int i, range, point;
	float ratio;
	double tmp;

	range = (int)myData->cData[ch].op.rangeV;
	ratio = Ref_V_Ratio(range);

	tmp = (double)avg * (double)ratio;
	if(tmp >= 0.0) {
		tmp = tmp * myData->bData[bd].misc.Vsource_AD_a
			+ myData->bData[bd].misc.Vsource_AD_b;
	} else {
		tmp = tmp * myData->bData[bd].misc.Vsource_AD_a_N
			+ myData->bData[bd].misc.Vsource_AD_b_N;
	}

	i = myData->cData[ch].misc.sensCount;

	if(myData->cData[ch].op.state == C_CALI
		&& myData->cali[ch].type == CALI_TYPE_VOLTAGE) {
		if(myData->cData[ch].op.phase <= P10) {
			myData->cData[ch].misc.sensSumVb[i] = (long)tmp;
		} else {
			point = cFindADCaliPoint(ch, tmp, ad_mux, range, 0);
			myData->cData[ch].misc.sensSumVb[i]
				= (long)(tmp * myData->cali[ch]
				.tmpData[ad_mux][range].AD_A[point]
				+ myData->cali[ch].tmpData[ad_mux][range].AD_B[point]);
		}
	} else {
		point = cFindADCaliPoint(ch, tmp, ad_mux, range, 1);
		myData->cData[ch].misc.sensSumVb[i]
			= (long)(tmp * myData->cali[ch].data[ad_mux][range].AD_A[point]
			+ myData->cali[ch].data[ad_mux][range].AD_B[point]);
	}
}

void Set_Mux(int next_slot, int bd, int next_ch)
{
	switch(myData->AppControl.config.systemModel) {
		//formation
		case F_PNE_5V_15A_30AP_SW:
			Set_Mux_7(bd);
			break;
		case F_SDI_5V_50A_5A:
			Set_Mux_5(next_slot, bd, next_ch);
			break;
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
			Set_Mux_6(next_slot, bd, next_ch);
			break;

		//cycler
		case C_ENERLAND_5V_250A:
			Set_Mux_2(next_slot, bd, next_ch);
			break;
		case C_ENERLAND_5V_250A_50A_5A: //kjg_w
		case C_ENERLAND_5V_250A_50A_5A_2: //kjg_w
			Set_Mux_3(next_slot, bd, next_ch);
			break;
		case C_EIG_5V_50A_5A:
		case C_EIG_5V_50A_5A_2:
		case C_LGC_5V_200A_75A_15A:
			Set_Mux_4(next_slot, bd, next_ch);
			break;
		default:
			Set_Mux_1(next_slot, bd, next_ch);
			break;
	}
}

void Set_Mux_1(int next_slot, int bd, int next_ch)
{
	unsigned char mux1, mux2;
	
	// bits-   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	// mux1- C_A8 | C_A7 | C_A6 | C_A5 | C_A4 | C_A3 | C_A2 | C_A1 |
	// mux2- M_A8 | M_A7 | M_A6 | M_A5 | M_A4 | M_A3 | M_A2 | M_A1 |

	if(next_slot < 4) { //V, I
		myPs->misc.ch_muxVal = (unsigned char)(next_slot & 0x03);
		mux2 = 0xFF;
	} else { //sourceV, sourceI
		mux2 = myPs->misc.ref_muxVal;
	}
	mux1 = myPs->misc.ch_muxVal;

	outb(mux1, CH_ADDR_MUX_1);
	outb(mux2, CH_ADDR_MUX_2);
}

void Set_Mux_2(int next_slot, int bd, int next_ch)
{
	unsigned char mux1, mux2, mux3;

	// bits-   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	// mux1-      | M_EN3| M_EN2| M_EN1|      |      | C_EN2| C_EN1|
	// mux2-      |      | M_A6 | M_A5 | M_A4 | M_A3 | M_A2 | M_A1 |
	// mux3-      |      |      |      |      | C_A3 | C_A2 | C_A1 |

	if(next_slot < 4) { //V, I
		myPs->misc.ch_muxVal = (unsigned char)(next_slot & 0x03);
		mux1 = 0x43;
		mux2 = 0x3F;
	} else { //sourceV, sourceI
		mux1 = 0x70;
		mux2 = myPs->misc.ref_muxVal;
		mux2 |= (mux2 << 2);
		mux2 |= 0x30;
	}
	mux3 = myPs->misc.ch_muxVal;

	outb(mux1, CH_ADDR_MUX_1);
	outb(mux2, CH_ADDR_MUX_2);
	outb(mux3, CH_ADDR_MUX_3);
}

void Set_Mux_3(int next_slot, int bd, int next_ch)
{
}

void Set_Mux_4(int next_slot, int bd, int next_ch)
{
	unsigned char mux1, mux2, mux3;
	
	// bits-   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	// mux1-      | M_EN3| M_EN2| M_EN1|      |      | C_EN2| C_EN1|
	// mux2-      |      | M_A6 | M_A5 | M_A4 | M_A3 | M_A2 | M_A1 |
	// mux3-      |      |      |      |      | C_A3 | C_A2 | C_A1 |

	if((next_slot % 5) < 4) { //V, I
		myPs->misc.ch_muxVal = (unsigned char)(next_ch % 8);
		mux1 = 0x43;
		mux2 = 0x3F;
	} else { //sourceV, sourceI
		mux1 = 0x70;
		mux2 = myPs->misc.ref_muxVal;
		mux2 |= (mux2 << 2);
		mux2 |= 0x30;
	}
	mux3 = myPs->misc.ch_muxVal;

	outb(mux1, CH_ADDR_MUX_1);
	outb(mux2, CH_ADDR_MUX_2);
	outb(mux3, CH_ADDR_MUX_3);
}

void Set_Mux_5(int next_slot, int bd, int next_ch)
{
	unsigned char mux1, mux2, mux3;

	// bits-   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	// mux1-      | M_EN3| M_EN2| M_EN1| C_EN4| C_EN3| C_EN2| C_EN1|
	// mux2-      |      | M_A6 | M_A5 | M_A4 | M_A3 | M_A2 | M_A1 |
	// mux3-      |      |      |      |      | C_A3 | C_A2 | C_A1 |

	if((next_slot % 5) < 4) { //V, I
		myPs->misc.ch_muxVal = (unsigned char)(next_ch % 8);
		mux1 = 0x01;
		mux1 = (mux1 << (next_ch / 8));
		mux1 |= 0x40;
		mux2 = 0x3F;
	} else { //sourceV, sourceI
		mux1 = 0x70;
		mux2 = myPs->misc.ref_muxVal;
		mux2 |= (mux2 << 2);
		mux2 |= 0x30;
	}
	mux3 = myPs->misc.ch_muxVal;

	outb(mux1, CH_ADDR_MUX_1);
	outb(mux2, CH_ADDR_MUX_2);
	outb(mux3, CH_ADDR_MUX_3);
}

void Set_Mux_6(int next_slot, int bd, int next_ch)
{
	unsigned char mux1, mux2;

	// bits-   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	// mux1-      | M_EN3| M_EN2| M_EN1|      |      |      |      |
	// mux2-      |      | M_A6 | M_A5 | M_A4 | M_A3 | M_A2 | M_A1 |

	if((next_slot % 5) < 4) { //V, I
		myPs->misc.ch_muxVal = 0x00; //(unsigned char)(next_ch % 8);
		mux1 = 0x70;
		mux2 = 0x3A;
	} else { //sourceV, sourceI
		mux1 = 0x70;
		if(myPs->misc.ref_muxVal <= 0x01) {
			mux2 = myPs->misc.ref_muxVal;
		} else if(myPs->misc.ref_muxVal == 0x02) {
			mux2 = 0x03;
		} else {
			mux2 = 0x02;
		}
		mux2 |= (mux2 << 2);
		mux2 |= 0x30;
	}

	outb(mux1, CH_ADDR_MUX_1);
	outb(mux2, CH_ADDR_MUX_2);
}

void Set_Mux_7(int bd)
{
	unsigned char mux1, mux3, tmp;
	int addr;

	// bits-   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	// mux1-      | M_A3 | M_A2 | M_A1 |      | M_EN3| M_EN2| M_EN1|
	// mux2-      | M_A7 | M_A6 | M_A5 |      | M_EN7| M_EN6| M_EN5|
	// mux3-      |      | C_EN2| C_EN1|      | C_A3 | C_A2 | C_A1 |
	// mux4-      |      |      |      |      |      | SEL2 | SEL1 |

	if(myPs->misc.next_channel_slot >= 0) { //channel
		mux1 = 0x02;
		tmp = 0x10 << (myPs->misc.next_channel_slot / 8);
		mux3 = tmp | (unsigned char)(myPs->misc.next_channel_slot % 8);
	} else { //reference
		if(myPs->misc.next_reference_slot < 4) {
			mux1 = (unsigned char)myPs->misc.next_reference_slot << 4;
			mux1 |= 0x01;
		} else {
			mux1 = (unsigned char)(myPs->misc.next_reference_slot - 4) << 4;
			mux1 |= 0x04;
		}
		mux3 = myPs->misc.ch_muxVal;
	}

	myPs->misc.ref_muxVal = mux1;
	myPs->misc.ch_muxVal = mux3;

	if(bd == 0) {
		outb(mux1, CH_ADDR_MUX_1);
	}

	addr = CH_ADDR_MUX_3 + 0x10 * bd;
	outb(mux3, addr);
}

void CalChAverage(int bd, int ch)
{
	int	cnt, cnt_end, i, j, type, master_ch, acc_type;
	long tmpV, tmpI, min, max;
	double tempV, tempI, acc_time;
	long ratioV, ratioI;	//kjh_220321
	double ratioP;	//kjh_220321

	ch += (bd * myPs->config.chPerBd);

	if(ch >= MAX_CH_8) return;

	myCh = &(myData->cData[ch]);

	if(myPs->config.ratioV == MICRO_UNIT && myPs->config.ratioI == MICRO_UNIT) { //uV,uA
		ratioV = 1;
		ratioI = 1;
	} else if(myPs->config.ratioV == MILLI_UNIT
		&& myPs->config.ratioI == MILLI_UNIT) { //mV,mA
		ratioV = 1000;
		ratioI = 1000;
	} else {
		ratioV = 1;
		ratioI = 1;
	} //kjh_211021

	if(myData->AppControl.config.debugType == 110) {
		//for debugging software
		if(myCh->op.Vsens < 0) {
			myCh->op.Vsens = 10000 / ratioV;
		} else {
			if(myCh->op.Vsens <= (4000000 / ratioV)) {
				myCh->op.Vsens += (100000 / ratioV);
			} else myCh->op.Vsens = 1000000 / ratioV;
		}
		myCh->misc.tmpVsens = myCh->op.Vsens;

		myCh->misc.Vinput = myCh->op.Vsens;
		myCh->misc.Vpower = myCh->op.Vsens + (100000 / ratioV);
		myCh->misc.Vbus = myCh->op.Vsens + (200000 / ratioV);
		if(myCh->op.Isens < (-1000000 / ratioI)) {
			myCh->op.Isens = 1000000 / ratioI;
		} else {
			if(myCh->op.Isens <= (1000000 / ratioI)) {
				myCh->op.Isens += (100000 / ratioI);
			} else myCh->op.Isens = -1000000 / ratioI;
		}
		myCh->misc.tmpIsens = myCh->op.Isens;

		return;
	}

	cnt_end = ch_sens_count_increment(ch);
	cnt = myCh->misc.tmp_sensCount;

	switch(myData->AppControl.config.systemModel) {
		case C_KATECH_20V_1000A_500A_100A: //csk_120309
		case C_LGC_50V_40A_10A_4A: //csk_120206
		case C_LGC_50V_40A_10A_4A_2:
		case C_LGC_50V_40A_10A_4A_3:
		case C_LGC_50V_40A_10A_4A_4:
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
		case C_SDI_70V_250A_25A_18KW:
		case C_SDI_80V_50A_25A_10A:		//csk_130417_s
		case C_SDI_80V_50A_25A_10A_2:
		case C_SDI_80V_50A_25A_10A_3:
		case C_SDI_80V_50A_25A_10A_4:	//csk_130417_e
		case C_KATECH_100V_30A_15A_5A: //csk_131129
			acc_type = 1;
			break;
		default:
			acc_type = 0;
			break;
	}

	type = 0; //voltage
	tmpV = myCh->misc.sensSumV[cnt];
	tempV = myCh->misc.sensSumV[0];
	min = (long)tempV;
	max = (long)tempV;
	for(i=1; i < cnt_end; i++) {
		tempV += (double)myCh->misc.sensSumV[i];
		if(myCh->misc.sensSumV[i] < min) min = myCh->misc.sensSumV[i];
		if(myCh->misc.sensSumV[i] > max) max = myCh->misc.sensSumV[i];
	}
	if(cnt_end < 3) {
		tempV /= cnt_end;
	} else {
		tempV = (tempV - (double)min - (double)max) / (cnt_end - 2);
	}

	switch(myData->AppControl.config.systemModel) {
		case C_KATECH_20V_1000A_500A_100A: //csk_120309
		case C_LGC_50V_40A_10A_4A: //csk_120206
		case C_LGC_50V_40A_10A_4A_2:
		case C_LGC_50V_40A_10A_4A_3:
		case C_LGC_50V_40A_10A_4A_4:
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
		case C_SDI_70V_250A_25A_18KW:
		case C_SDI_80V_50A_25A_10A:		//csk_130417_s
		case C_SDI_80V_50A_25A_10A_2:
		case C_SDI_80V_50A_25A_10A_3:
		case C_SDI_80V_50A_25A_10A_4:	//csk_130417_e
		case C_KATECH_100V_30A_15A_5A: //csk_131129
			if(ch >= 4) {
				myData->cali[ch].orgAD[type] = tempV;
				myData->cData[ch-4].misc.Vpower = (long)tempV;
			}
			break;
		case C_SBL_150V_250A_10A_38KW: //lki_111010
		case C_SBL_150V_250A_10A_75KW_4:
		case C_SBL_150V_250A_10A_75KW_5:
			switch(ch) {
				case 4:
					myData->cali[ch].orgAD[type] = tempV;
					myData->cData[ch-4].misc.Vpower = (long)tempV;
					break;
				case 5:
					myData->cali[ch].orgAD[type] = tempV;
					myData->cData[ch-5].misc.Vbus = (long)tempV;
					break;
				case 6:
					myData->cali[ch].orgAD[type] = tempV;
					myData->cData[ch-5].misc.Vpower = (long)tempV;
					break;
				case 7:
					myData->cali[ch].orgAD[type] = tempV;
					myData->cData[ch-6].misc.Vbus = (long)tempV;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}

	if(myCh->op.state == C_CALI) {
		myData->cali[ch].orgAD[type] = tempV;
		myCh->op.Vsens = (long)tempV;
		myCh->misc.tmpVsens = tmpV;
	} else {
		if(myData->ChAttribute[ch].chNo_master != 0) {
			if(myData->AppControl.config.debugType == 110
				|| myData->AppControl.config.debugType == 111) {
				//for debugging software
				if(myCh->op.Vsens < 0) {
					myCh->op.Vsens = 10000 / ratioV;
				} else {
					if(myCh->op.Vsens <= (100000000 / ratioV)) {
						myCh->op.Vsens += (10000000 / ratioV);
					} else myCh->op.Vsens = 1000000 / ratioV;
				}
				myCh->misc.tmpVsens = myCh->op.Vsens;

				myCh->misc.Vinput = myCh->op.Vsens;
				myCh->misc.Vpower = myCh->op.Vsens + (1000000 / ratioV);
				myCh->misc.Vbus = myCh->op.Vsens + (2000000 / ratioV);
				myCh->misc.reserved1[0] = myCh->misc.Vinput + (1000 / ratioV);
				myCh->misc.reserved1[1] = myCh->misc.Vinput + (2000 / ratioV);
				myCh->misc.reserved1[2] = myCh->misc.Vinput + (3000 / ratioV);
				myCh->misc.reserved1[3] = myCh->misc.Vinput + (4000 / ratioV);
				myCh->misc.reserved1[4] = myCh->misc.Vinput + (5000 / ratioV);
				myCh->misc.reserved1[5] = myCh->misc.Vinput + (6000 / ratioV);
				myCh->misc.reserved1[6] = myCh->misc.Vinput + (7000 / ratioV);
			} else {
				if(acc_type == 0) {
					myCh->op.Vsens = (long)tempV;
					myCh->misc.tmpVsens = tmpV;
					/*
					if(myData->test_val_i[1][0] == 0) {
						myCh->op.Vsens = 50000000; //kjhw_ttttt
						myCh->misc.tmpVsens = 50000000; //kjhw_ttttt
					} else if(myData->test_val_i[1][0] == 1) {
						myCh->op.Vsens += 1000; //kjhw_ttttt
						myCh->misc.tmpVsens += 1000; //kjhw_ttttt
					} else if(myData->test_val_i[1][0] == 2) {
						myCh->op.Vsens -= 1000; //kjhw_ttttt
						myCh->misc.tmpVsens -= 1000; //kjhw_ttttt
					} else {
					}
					*/
				} else {
					if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
						myCh->op.runTime_day, myCh->op.runTime, 0, 9) > 0) {
						myCh->op.Vsens = tmpV;
						myCh->misc.tmpVsens = tmpV;
					} else {
						myCh->op.Vsens = (long)tempV;
						myCh->misc.tmpVsens = tmpV;
					}
				}
			}
		} else {
			master_ch = -1;
			for(i=0; i < myData->mData.config.installedCh; i++) {
				for(j=0; j < MAX_SLAVE_CH; j++) {
					if((myData->ChAttribute[i].chNo_slave[j] - 1) == ch) {
						master_ch = myData->ChAttribute[i].chNo_master - 1;
						break;
					}
				}
				if(master_ch >= 0) break;
			}
			if(acc_type == 0) {
				if(master_ch >= 0) {
					myCh->op.Vsens = myData->cData[master_ch].op.Vsens;
					myCh->misc.tmpVsens
						= myData->cData[master_ch].misc.tmpVsens;
				} else {
					myCh->op.Vsens = (long)tempV;
					myCh->misc.tmpVsens = tmpV;
				}
			} else {
				if(master_ch >= 0) {
					myCh->op.Vsens = myData->cData[master_ch].op.Vsens;
					myCh->misc.tmpVsens
						= myData->cData[master_ch].misc.tmpVsens;
				} else {
					if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
						myCh->op.runTime_day, myCh->op.runTime, 0, 9) > 0) {
						myCh->op.Vsens = tmpV;
						myCh->misc.tmpVsens = tmpV;
					} else {
						myCh->op.Vsens = (long)tempV;
						myCh->misc.tmpVsens = tmpV;
					}
				}
			}
		}
	}
	myCh->misc.tmpVsens_slave = tmpV;	//jhkw_181001

	type = 1; //current
	tmpI = myCh->misc.sensSumI[cnt];
	tempI = myCh->misc.sensSumI[0];
	min = (long)tempI;
	max = (long)tempI;
	for(i=1; i < cnt_end; i++) {
		tempI += (double)myCh->misc.sensSumI[i];
		if(myCh->misc.sensSumI[i] < min) min = myCh->misc.sensSumI[i];
		if(myCh->misc.sensSumI[i] > max) max = myCh->misc.sensSumI[i];
	}
	if(cnt_end < 3) {
		tempI /= cnt_end;
	} else {
		tempI = (tempI - (double)min - (double)max) / (cnt_end - 2);
	}

	switch(myData->AppControl.config.systemModel) {
		case C_KATECH_20V_1000A_500A_100A: //csk_120309
		case C_LGC_50V_40A_10A_4A: //csk_120206
		case C_LGC_50V_40A_10A_4A_2:
		case C_LGC_50V_40A_10A_4A_3:
		case C_LGC_50V_40A_10A_4A_4:
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
		case C_SDI_70V_250A_25A_18KW:
		case C_SDI_80V_50A_25A_10A:		//csk_130417_s
		case C_SDI_80V_50A_25A_10A_2:
		case C_SDI_80V_50A_25A_10A_3:
		case C_SDI_80V_50A_25A_10A_4:	//csk_130417_e
		case C_KATECH_100V_30A_15A_5A: //csk_131129
			if(ch >= 4) {
				myData->cali[ch].orgAD[type] = tempI;
				myData->cData[ch-4].misc.Vbus = (long)tempI;
			}
			break;
		case C_SBL_150V_250A_10A_38KW: //lki_111010
		case C_SBL_150V_250A_10A_75KW_4:
		case C_SBL_150V_250A_10A_75KW_5:
			switch(ch){
				case 4:
					myData->cali[ch].orgAD[type] = tempI;
					myData->cData[ch-4].misc.Vinput = (long)tempI;
					break;
				case 6:
					myData->cali[ch].orgAD[type] = tempI;
					myData->cData[ch-5].misc.Vinput = (long)tempI;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}

	if(myCh->op.state == C_CALI) {
		myData->cali[ch].orgAD[type] = tempI;
		myCh->op.Isens = (long)tempI;
		myCh->misc.tmpIsens = tmpI;
	} else {
		if(myData->AppControl.config.debugType == 110) {
			//for debugging software
			if(myCh->op.Isens < (-100000000 / ratioI)) {
				myCh->op.Isens = (100000000 / ratioI);
			} else {
				if(myCh->op.Isens <= (100000000 / ratioI))
					myCh->op.Isens += (10000000 / ratioI);
				else myCh->op.Isens = (-100000000 / ratioI);
			}
			myCh->misc.tmpIsens = myCh->op.Isens;
		} else if(myData->AppControl.config.debugType == 111) {
			myCh->op.Isens = 0;
			myCh->misc.tmpIsens = 0;
		} else if(myData->AppControl.config.debugType == 220) {
			if(myCh->op.state == C_RUN){
				if(myCh->misc.cmd_i[0] > 0){
					if(myCh->op.Vsens < myCh->misc.cmd_v[0]){
						if(myCh->op.stepType == STEP_PATTERN) {
							myCh->op.Isens = myCh->misc.cmd_i[1];
						} else {
							myCh->op.Isens = myCh->misc.cmd_i[0];
						}
					}else{
						if(myCh->op.stepType == STEP_PATTERN) {
							myCh->op.Isens = myCh->misc.cmd_i[1];
						} else {
							myCh->op.Isens = myCh->misc.cmd_i[0];
						}
					}
				}else{
					if(myCh->op.stepType == STEP_PATTERN) {
						myCh->op.Isens = myCh->misc.cmd_i[1];
					} else {
						myCh->op.Isens = myCh->misc.cmd_i[0];
					}
				}
			}else{
				myCh->op.Isens = 0;
			}
			myCh->misc.tmpIsens = myCh->op.Isens;
		} else {
			if(acc_type == 0) {
				myCh->op.Isens = (long)tempI;
				myCh->misc.tmpIsens = tmpI;
				/*
				if(myData->test_val_i[1][1] == 0) {
					switch(myCh->op.stepType) {
						case STEP_CHARGE:
							myCh->op.Isens = (long)100000000;
							myCh->misc.tmpIsens = 100000000;
							break;
						case STEP_DISCHARGE:
							myCh->op.Isens = (long)-100000000;
							myCh->misc.tmpIsens = -100000000;
							break;
						default:
							break;
					}
				} else if(myData->test_val_i[1][1] == 1) {
					myCh->op.Isens += (long)1000;
					myCh->misc.tmpIsens += 1000;
				} else if(myData->test_val_i[1][1] == 2) {
					myCh->op.Isens -= (long)1000;
					myCh->misc.tmpIsens -= 1000;
				} else {
				}
				*/
			} else {
				if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
					myCh->op.runTime_day, myCh->op.runTime, 0, 9) > 0) {
					myCh->op.Isens = tmpI;
					myCh->misc.tmpIsens = tmpI;
				} else {
					myCh->op.Isens = (long)tempI;
					myCh->misc.tmpIsens = tmpI;
				}
			}
		}
	}

	ratioV = myData->mData.ratioV;	//kjh_220321
	ratioI = myData->mData.ratioI;	//kjh_220321
	ratioP = myData->mData.ratioP;	//kjh_220321

	//cal tmpWatt
	//uA -> watt 1mW/div
	//nA -> watt 1uW/div
	tempI = ((double)myCh->misc.tmpVsens / ratioV)
		* ((double)myCh->misc.tmpIsens / ratioI);
	myCh->misc.tmpWatt = (long)(tempI / ratioP);
	
	//cal watt
	//uA -> watt 1mW/div
	//nA -> watt 1uW/div
	tempI = ((double)myCh->op.Vsens / ratioV)
		* ((double)myCh->op.Isens / ratioI);
	myCh->op.watt = (long)(tempI / ratioP);

	if(myCh->op.state != C_RUN) return;
	if(myCh->op.phase != P50) return;

	acc_time = 3600.0 * (1000.0 / myPs->config.scan_period);

	//cal ampareHour
	//uA -> capacity 1uAh/div
	//nA -> capacity 1nAh/div
	tmpI = myCh->op.Isens;
	if(tmpI >= 0) {
		if(acc_type == 0) {
			myCh->misc.sum_charge_AmpareHour += (double)tmpI;
			myCh->op.charge_AmpareHour
				= (long)(myCh->misc.seed_charge_AmpareHour
				+ myCh->misc.sum_charge_AmpareHour / acc_time);
			if(myCh->misc.sum_charge_AmpareHour > MAX_SUM_DOUBLE) {
				myCh->misc.seed_charge_AmpareHour
					= (double)myCh->op.charge_AmpareHour;
				myCh->misc.sum_charge_AmpareHour = 0.0;
			}
		} else {
			if(time_cal_compare(COMP_LESS_THAN,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
			} else if(time_cal_compare(COMP_EQUAL_TO,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
				myCh->misc.sum_charge_AmpareHour += (double)(tmpI * 8);
				myCh->op.charge_AmpareHour
					= (long)(myCh->misc.seed_charge_AmpareHour
					+ myCh->misc.sum_charge_AmpareHour / acc_time);
				if(myCh->misc.sum_charge_AmpareHour > MAX_SUM_DOUBLE) {
					myCh->misc.seed_charge_AmpareHour
						= (double)myCh->op.charge_AmpareHour;
					myCh->misc.sum_charge_AmpareHour = 0.0;
				}
			} else {
				myCh->misc.sum_charge_AmpareHour += (double)tmpI;
				myCh->op.charge_AmpareHour
					= (long)(myCh->misc.seed_charge_AmpareHour
					+ myCh->misc.sum_charge_AmpareHour / acc_time);
				if(myCh->misc.sum_charge_AmpareHour > MAX_SUM_DOUBLE) {
					myCh->misc.seed_charge_AmpareHour
						= (double)myCh->op.charge_AmpareHour;
					myCh->misc.sum_charge_AmpareHour = 0.0;
				}
			}
		}
	} else {
		if(acc_type == 0) {
			myCh->misc.sum_discharge_AmpareHour += (double)tmpI;
			myCh->op.discharge_AmpareHour
				= (long)(myCh->misc.seed_discharge_AmpareHour
				+ myCh->misc.sum_discharge_AmpareHour / acc_time);
			if(myCh->misc.sum_discharge_AmpareHour > MAX_SUM_DOUBLE) {
				myCh->misc.seed_discharge_AmpareHour
					= (double)myCh->op.discharge_AmpareHour;
				myCh->misc.sum_discharge_AmpareHour = 0.0;
			}
		} else {
			if(time_cal_compare(COMP_LESS_THAN,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
			} else if(time_cal_compare(COMP_EQUAL_TO,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
				myCh->misc.sum_discharge_AmpareHour += (double)(tmpI * 8);
				myCh->op.discharge_AmpareHour
					= (long)(myCh->misc.seed_discharge_AmpareHour
					+ myCh->misc.sum_discharge_AmpareHour / acc_time);
				if(myCh->misc.sum_discharge_AmpareHour > MAX_SUM_DOUBLE) {
					myCh->misc.seed_discharge_AmpareHour
						= (double)myCh->op.discharge_AmpareHour;
					myCh->misc.sum_discharge_AmpareHour = 0.0;
				}
			} else {
				myCh->misc.sum_discharge_AmpareHour += (double)tmpI;
				myCh->op.discharge_AmpareHour
					= (long)(myCh->misc.seed_discharge_AmpareHour
					+ myCh->misc.sum_discharge_AmpareHour / acc_time);
				if(myCh->misc.sum_discharge_AmpareHour > MAX_SUM_DOUBLE) {
					myCh->misc.seed_discharge_AmpareHour
						= (double)myCh->op.discharge_AmpareHour;
					myCh->misc.sum_discharge_AmpareHour = 0.0;
				}
			}
		}
	}

	//cal wattHour
	//uA -> watt 1mWh/div
	//nA -> watt 1uWh/div
	if(tmpI >= 0) {
		if(acc_type == 0) {
			myCh->misc.sum_charge_WattHour += ((double)tempI / ratioP);
			myCh->op.charge_WattHour
				= (long)(myCh->misc.seed_charge_WattHour
				+ myCh->misc.sum_charge_WattHour / acc_time);
			if(myCh->misc.sum_charge_WattHour > MAX_SUM_DOUBLE) {
				myCh->misc.seed_charge_WattHour
					= (double)myCh->op.charge_WattHour;
				myCh->misc.sum_charge_WattHour = 0.0;
			}
		} else {
			if(time_cal_compare(COMP_LESS_THAN,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
			} else if(time_cal_compare(COMP_EQUAL_TO,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
				myCh->misc.sum_charge_WattHour
					+= ((double)tempI / ratioP * 8.0);
				myCh->op.charge_WattHour
					= (long)(myCh->misc.seed_charge_WattHour
					+ myCh->misc.sum_charge_WattHour / acc_time);
				if(myCh->misc.sum_charge_WattHour > MAX_SUM_DOUBLE) {
					myCh->misc.seed_charge_WattHour
						= (double)myCh->op.charge_WattHour;
					myCh->misc.sum_charge_WattHour = 0.0;
				}
			} else {
				myCh->misc.sum_charge_WattHour += ((double)tempI / ratioP);
				myCh->op.charge_WattHour
					= (long)(myCh->misc.seed_charge_WattHour
					+ myCh->misc.sum_charge_WattHour / acc_time);
				if(myCh->misc.sum_charge_WattHour > MAX_SUM_DOUBLE) {
					myCh->misc.seed_charge_WattHour
						= (double)myCh->op.charge_WattHour;
					myCh->misc.sum_charge_WattHour = 0.0;
				}
			}
		}
	} else {
		if(acc_type == 0) {
			myCh->misc.sum_discharge_WattHour += ((double)tempI / ratioP);
			myCh->op.discharge_WattHour
				= (long)(myCh->misc.seed_discharge_WattHour
				+ myCh->misc.sum_discharge_WattHour / acc_time);
			if(myCh->misc.sum_discharge_WattHour > MAX_SUM_DOUBLE) {
				myCh->misc.seed_discharge_WattHour
					= (double)myCh->op.discharge_WattHour;
				myCh->misc.sum_discharge_WattHour = 0.0;
			}
		} else {
			if(time_cal_compare(COMP_LESS_THAN,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
			} else if(time_cal_compare(COMP_EQUAL_TO,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
				myCh->misc.sum_discharge_WattHour
					+= ((double)tempI / ratioP * 8.0);
				myCh->op.discharge_WattHour
					= (long)(myCh->misc.seed_discharge_WattHour
					+ myCh->misc.sum_discharge_WattHour / acc_time);
				if(myCh->misc.sum_discharge_WattHour > MAX_SUM_DOUBLE) {
					myCh->misc.seed_discharge_WattHour
						= (double)myCh->op.discharge_WattHour;
					myCh->misc.sum_discharge_WattHour = 0.0;
				}
			} else {
				myCh->misc.sum_discharge_WattHour += ((double)tempI / ratioP);
				myCh->op.discharge_WattHour
					= (long)(myCh->misc.seed_discharge_WattHour
					+ myCh->misc.sum_discharge_WattHour / acc_time);
				if(myCh->misc.sum_discharge_WattHour > MAX_SUM_DOUBLE) {
					myCh->misc.seed_discharge_WattHour
						= (double)myCh->op.discharge_WattHour;
					myCh->misc.sum_discharge_WattHour = 0.0;
				}
			}
		}
	}

	//cal meanV, meanI
	tempV = (double)myCh->op.meanV * (double)myCh->misc.meanSumCount;
	tempV += (double)myCh->misc.tmpVsens;
	tempI = (double)myCh->op.meanI * (double)myCh->misc.meanSumCount;
	tempI += (double)myCh->misc.tmpIsens;

	myCh->misc.meanSumCount++;
	tempV /= (double)myCh->misc.meanSumCount;
	myCh->op.meanV = (long)tempV;
	tempI /= (double)myCh->misc.meanSumCount;
	myCh->op.meanI = (long)tempI;
}

void CalSourceAverage(int bd, int cnt_end)
{
	int i, ref_mux;
	long min, max;
	double temp, ratio;

	ref_mux = myPs->misc.tmp_ref_muxVal;

	//voltage(ref)
	temp = myData->bData[bd].misc.source[ref_mux].sensSumV[0];
	min = (long)temp;
	max = (long)temp;
	for(i=1; i < cnt_end; i++) {
		temp += myData->bData[bd].misc.source[ref_mux].sensSumV[i];
		if(myData->bData[bd].misc.source[ref_mux].sensSumV[i] < min)
			min = myData->bData[bd].misc.source[ref_mux].sensSumV[i];
		if(myData->bData[bd].misc.source[ref_mux].sensSumV[i] > max)
			max = myData->bData[bd].misc.source[ref_mux].sensSumV[i];
	}
	if(cnt_end < 3) {
		temp /= cnt_end;
	} else {
		temp = (temp - (double)min - (double)max) / (cnt_end - 2);
	}

	ratio = Ref_V_Ratio(0);
	myData->bData[bd].misc.source[ref_mux].sourceV = (long)(temp * ratio);
		
	//current(ref)
	temp = myData->bData[bd].misc.source[ref_mux].sensSumI[0];
	min = (long)temp;
	max = (long)temp;
	for(i=1; i < cnt_end; i++) {
		temp += myData->bData[bd].misc.source[ref_mux].sensSumI[i];
		if(myData->bData[bd].misc.source[ref_mux].sensSumI[i] < min)
			min = myData->bData[bd].misc.source[ref_mux].sensSumI[i];
		if(myData->bData[bd].misc.source[ref_mux].sensSumI[i] > max)
			max = myData->bData[bd].misc.source[ref_mux].sensSumI[i];
	}
	if(cnt_end < 3) {
		temp /= cnt_end;
	} else {
		temp = (temp - (double)min - (double)max) / (cnt_end - 2);
	}

	ratio = Ref_I_Ratio(0);
	myData->bData[bd].misc.source[ref_mux].sourceI = (long)(temp * ratio);
}

void CalSourceAverage2(int bd)
{
	int cnt, cnt_end, ref_mux;
	
	ref_mux = myPs->misc.tmp_ref_muxVal;
	cnt = myPs->misc.source_sensCount;

	if(myPs->misc.timer_1sec < 1) return;

	if(myPs->misc.source_sensCountFlag == P0) {
		cnt_end = cnt + 1;
	} else {
		cnt_end = MAX_SOURCE_SENS_COUNT;

		myData->bData[bd].misc.source2[ref_mux].totalV
			-= (double)myData->bData[bd].misc.source2[ref_mux].sumV[cnt];
		myData->bData[bd].misc.source2[ref_mux].totalI
			-= (double)myData->bData[bd].misc.source2[ref_mux].sumI[cnt];
	}

	myData->bData[bd].misc.source2[ref_mux].sumV[cnt]
		= myData->bData[bd].misc.source[ref_mux].sourceV;
	myData->bData[bd].misc.source2[ref_mux].sumI[cnt]
		= myData->bData[bd].misc.source[ref_mux].sourceI;

	myData->bData[bd].misc.source2[ref_mux].totalV
		+= (double)myData->bData[bd].misc.source[ref_mux].sourceV;
	myData->bData[bd].misc.source2[ref_mux].totalI
		+= (double)myData->bData[bd].misc.source[ref_mux].sourceI;

	myData->bData[bd].misc.source2[ref_mux].sourceV
		= (long)(myData->bData[bd].misc.source2[ref_mux].totalV / cnt_end);
	myData->bData[bd].misc.source2[ref_mux].sourceI
		= (long)(myData->bData[bd].misc.source2[ref_mux].totalI / cnt_end);
}

void CalibratorSource(int bd)
{
	int ref_mux;
	double AD_a, AD_b, val1, val2;
	
	ref_mux = myPs->misc.tmp_ref_muxVal;

	val1 = Ref_V_Value();
	val2 = Ref_I_Value();

	AD_a = (double)(val1 - 0.0)
		/ (double)(myData->bData[bd].misc.source2[0].sourceV
		- myData->bData[bd].misc.source2[2].sourceV);
	AD_b = val1
		- (double)myData->bData[bd].misc.source2[0].sourceV * AD_a;
	myData->bData[bd].misc.Vsource_AD_a = AD_a;
	myData->bData[bd].misc.Vsource_AD_b = AD_b;
	
	AD_a = (double)(0.0 - (-val1))
		/ (double)(myData->bData[bd].misc.source2[2].sourceV
		- myData->bData[bd].misc.source2[1].sourceV);
	AD_b = (double)(0.0)
		- (double)myData->bData[bd].misc.source2[2].sourceV * AD_a;
	myData->bData[bd].misc.Vsource_AD_a_N = AD_a;
	myData->bData[bd].misc.Vsource_AD_b_N = AD_b;
	
	AD_a = (double)(val2 - 0.0)
		/ (double)(myData->bData[bd].misc.source2[0].sourceI
		- myData->bData[bd].misc.source2[2].sourceI);
	AD_b = (double)val2
		- (double)myData->bData[bd].misc.source2[0].sourceI * AD_a;
	myData->bData[bd].misc.Isource_AD_a = AD_a;
	myData->bData[bd].misc.Isource_AD_b = AD_b;
	
	AD_a = (double)(0.0 - (-val2))
		/ (double)(myData->bData[bd].misc.source2[2].sourceI
		- myData->bData[bd].misc.source2[1].sourceI);
	AD_b = (double)(0.0)
		- (double)myData->bData[bd].misc.source2[2].sourceI * AD_a;
	myData->bData[bd].misc.Isource_AD_a_N = AD_a;
	myData->bData[bd].misc.Isource_AD_b_N = AD_b;

	if(myData->bData[bd].misc.source2[ref_mux].sourceV >= 0) {
		myData->bData[bd].misc.source[ref_mux].calSourceV
			= (long)((double)myData->bData[bd].misc.source2[ref_mux].sourceV
			* myData->bData[bd].misc.Vsource_AD_a
			+ myData->bData[bd].misc.Vsource_AD_b);
	} else {
		myData->bData[bd].misc.source[ref_mux].calSourceV
			= (long)((double)myData->bData[bd].misc.source2[ref_mux].sourceV
			* myData->bData[bd].misc.Vsource_AD_a_N
			+ myData->bData[bd].misc.Vsource_AD_b_N);
	}
	if(myData->bData[bd].misc.source2[ref_mux].sourceI >= 0) {
		myData->bData[bd].misc.source[ref_mux].calSourceI
			= (long)((double)myData->bData[bd].misc.source2[ref_mux].sourceI
			* myData->bData[bd].misc.Isource_AD_a
			+ myData->bData[bd].misc.Isource_AD_b);
	} else {
		myData->bData[bd].misc.source[ref_mux].calSourceI
			= (long)((double)myData->bData[bd].misc.source2[ref_mux].sourceI
			* myData->bData[bd].misc.Isource_AD_a_N
			+ myData->bData[bd].misc.Isource_AD_b_N);
	}
}

double Ref_V_Ratio(int range)
{
	double ratio;
	long maxV; //kjh_211021
	double da_max, v_feed, main_amp; //kjh_211021

	//10000000uV / 32768 = 305.1757813uV
	ratio = 305.1757813;
	switch(myData->AppControl.config.systemModel) {
		case C_LGC_5V_150A_10A:
		case C_LGC_5V_600A_10A:
			//AMP -> INA128UA
			ratio /= 1.5;
			break;
		case C_ENERLAND_5V_250A:
			//AMP -> AD620A
			ratio /= 1.494;
			break;
		case C_SKI_20V_400A_200A_100A_16KW:			//csk_171023
		case C_SKI_20V_400A_200A_100A_16KW_2:		//csk_171023
		case C_SKI_20V_500A_250A_100A_20KW:		//csk_171108
		case C_SKI_20V_500A_250A_100A_20KW_2:	//csk_171108
		case C_SKI_20V_600A_300A_60A_24KW:		//jhk_180930
		case C_SKI_20V_600A_300A_60A_24KW_2:	//jhk_180930
		case C_TAESUNG_20V_600A_60A:
		case C_KATECH_20V_1000A_500A_100A: //csk_120309
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (20.0 / 5.0);
			break;
		/*case C_LGC_40V_300A_100A_48KW: //jhk_181018
		case C_LGC_40V_300A_100A_48KW_2: //jhk_181018
		case C_LGC_40V_500A_250A_100A_80KW:	//jhk_181014
		case C_LGC_40V_500A_250A_100A_80KW_2: //jhk_181014
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (40.0 / 5.0);
			} else {
				ratio /= 1.494;
				ratio *= (20.0 / 5.0);
			}
			break;*/
		case C_LGC_50V_120A_10A:
		case C_LGC_50V_200A_10A:
		case C_ENERTECH_55V_100A_10A: //kjg_w
			//AMP -> AD620A
			ratio /= 1.494;
			break;
		case C_LGC_50V_40A_10A_4A:		//csk_120206
		case C_LGC_50V_40A_10A_4A_2:
		case C_LGC_50V_40A_10A_4A_3:
		case C_LGC_50V_40A_10A_4A_4:
		case C_JNU_50V_200A_100A_10KW:	//jhk_161202
		case C_LGC_50V_300A_10A_20KW:	//jhk
		case C_SEBANG_50V_300A_100A_50A_15KW:	//jhkw_130912
		case C_LGC_50V_500A_10A_35KW:	//kjh_111002
		case C_LGC_50V_500A_10A_42KW:
		case C_LGC_50V_500A_10A_42KW_2:
		case C_LGC_50V_500A_10A_42KW_3:
		case C_LGC_50V_500A_10A_42KW_4:
		case C_SKI_50V_500A_250A_25KW:	//ktg_190715
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (50.0 / 5.0);
			break;
		case C_SKI_50V_500A_400A_200A_100A_100KW:	//jhk_180119
		case C_SKI_50V_500A_400A_200A_100A_100KW_2:	//jhk_180119
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (50.0 / 4.5);
			break;
		case C_SKI_50V_1000A_500A_300A_100KW:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_2:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_3:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_4:	//ktg_190711
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (50.0 / 5.0);
			break;
		case C_SNU_60V_200A_100A_50A_1CH_12KW:  //jhkw_130221
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (60.0 / 4.5);
			break;
		case C_LGC_60V_30A_15A_3A_7KW:			//ktg_181113	//shh_231024
		case C_HLGP_60V_200A_10A_12KW:
		case C_HLGP_60V_200A_10A_24KW:
		case C_HLGP_60V_200A_10A_24KW_2:
		case C_LGC_60V_250A_10A_30KW:
		case C_LGC_60V_250A_10A_30KW_2:
		case C_LGC_60V_250A_10A_30KW_3:
		case C_SK_60V_300A_10A:
		case C_KTL_60V_400A_200A_40KW:	//jhk_181218
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (60.0 / 5.0);
			break;
		//case C_LGC_60V_300A_30A_36KW:	//jhk_150406
		//case C_LGC_60V_300A_30A_36KW_2:	//jhk_150406
		//case C_LGC_60V_300A_30A_36KW_3:	//jhk_150406	//shh_231027
		case C_TOPBATT_60V_300A_100A_36KW:	//jhk_150616
		case C_HYUNDAI_60V_300A_100A_36KW:	//jhk_160825
		case C_HYUNDAI_60V_300A_100A_36KW_2:	//jhk_161111
		case C_HYUNDAI_60V_300A_100A_36KW_3:	//jhk_171015
		case C_CANSYSTEM_60V_350A_100A_21KW:	//jhk_140828
		case C_NORTHVOLT_60V_400A_200A_96KW:	//khj_191203
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
		case C_DAEHWA_60V_500A_250A_50A_60KW:	//jhk_160913
		case C_DAEHWA_60V_500A_250A_50A_60KW_2:	//jhk_160913
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (60.0 / 4.5);
			break;
		/*case C_LGC_60V_600A_300A_100A_144KW:	//ktg_190707
		case C_LGC_60V_600A_300A_100A_144KW_2:	//ktg_191222
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (60.0 / 4.5);
			} else {
				ratio /= 1.494;
				ratio *= (40.0 / 4.5);
			}
			break;*/
		/*case C_LGC_70V_100A_28KW:   //ljh_201130
		case C_LGC_70V_100A_28KW_2: //ljh_201130
		    //AMP -> OP270
		    if((range+1) == RANGE1) {
			    ratio /= 1.494;
			    ratio *= (70.0 / 5.25);
			} else {
			    ratio /= 1.494;
			    ratio *= (40.0 / 5.25);
			}
			break;*/	//shh_231024
		case C_LGC_70V_100A_14KW:	//jhk_180104
		case C_LGC_70V_100A_14KW_2:	//jhk_180104
		case C_LGC_70V_100A_14KW_3:	//jhk_180104
		case C_LGC_70V_100A_14KW_4:	//jhk_180104
		//case C_LGC_70V_200A_100A_28KW:		//jhk_170911
		//case C_LGC_70V_200A_100A_28KW_2:	//jhk_170911	//shh_231025
		case C_LGC_70V_250A_10A_35KW:
		case C_LGC_70V_250A_10A_35KW_2:
		case C_LGC_70V_250A_10A_35KW_3:
		//case C_LGC_70V_250A_10A_35KW_4:
		//case C_LGC_70V_250A_10A_35KW_5:	//phb_240429
		//case C_LGC_70V_250A_10A_35KW_6:	//shh_211005
		case C_LGC_70V_250A_10A_35KW_7:
		case C_LGC_70V_250A_10A_35KW_8:
		case C_LGC_70V_250A_10A_35KW_9:
		case C_LGC_70V_250A_10A_35KW_10:
		case C_LGC_70V_250A_10A_35KW_11:
		case C_LGC_70V_250A_10A_35KW_12:
		case C_LGC_70V_250A_10A_35KW_13:
		case C_LGC_70V_250A_10A_35KW_14:
		//case C_LGC_70V_250A_10A_35KW_15:	//ktg_220420
		//case C_LGC_70V_250A_10A_35KW_16:
		//case C_LGC_70V_250A_10A_35KW_17:
		//case C_LGC_70V_250A_10A_35KW_18:
		case C_LGC_70V_250A_10A_35KW_19:	//111202_csk
		case C_LGC_70V_250A_10A_35KW_20:	//111202_csk
		case C_LGC_70V_250A_10A_35KW_21:	//jhk_120229
		case C_LGC_70V_250A_10A_35KW_22:	//jhk_120229
		case C_LGC_70V_250A_10A_35KW_23:	//jhk_120702
		case C_LGC_70V_250A_10A_35KW_24:	//jhk_120702
		//case C_LGC_70V_250A_10A_35KW_25:	//jhk_120702
		case C_LGC_70V_250A_10A_35KW_26:	//jhk_120702
		case C_LGC_70V_250A_10A_35KW_27:	//jhk_120702
		//case C_LGC_70V_250A_10A_35KW_28:	//jhk_120702	//shh_220614
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (70.0 / 5.0);
			break;
		case C_LGC_70V_100A_50A_28KW:			//ktg_190405
		case C_LGC_70V_250A_10A_35KW_4:
		case C_LGC_70V_250A_10A_35KW_25:    //jhk_120702
		case C_LGC_70V_250A_10A_35KW_15:	//ktg_220420
		case C_LGC_70V_250A_10A_35KW_16:
		case C_LGC_70V_250A_10A_35KW_17:	//shh_220214
		case C_LGC_70V_250A_10A_35KW_18:	//shh_220214
		/*case C_LGC_70V_250A_50A_35KW:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_2:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_3:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_4:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_5:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_6:		//jhk_150502
		//case C_LGC_70V_250A_50A_35KW_7:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_8:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_9:		//jhk_160515
		case C_LGC_70V_250A_50A_35KW_10:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_11:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_12:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_13:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_14:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_15:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_16:	//jhk_160515*/	//sec_231026
		case C_LGC_70V_250A_100A_50A_35KW:		//jhkw_121019
		case C_LGC_70V_250A_100A_50A_35KW_2:	//jhkw_121019
		case C_LGC_70V_250A_100A_50A_35KW_3:	//jhkw_121019
		case C_LGC_70V_250A_100A_50A_35KW_4:	//jhkw_121019
		case C_LGC_70V_300A_100A_42KW:	//jhk_170211
		case C_LGC_70V_300A_100A_42KW_2:	//jhk_170519
		//case C_LGC_70V_300A_100A_50A_10A_42KW:	//jhkw_120810
		//case C_LGC_70V_300A_100A_50A_10A_42KW_2:	//jhkw_120813	//shh_231027
		case C_LGCUSA_70V_300A_42KW:	//jhk_160802
		case C_LGCUSA_70V_300A_42KW_2:	//jhk_160802
		case C_LGCUSA_70V_350A_24KW:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_2:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_3:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_4:	//jhk_160720
		case C_LGC_70V_400A_200A_56KW:			//ktg_190711
		case C_LGC_70V_250A_10A_35KW_6:	//shh_211005
		case C_LGC_70V_250A_10A_35KW_28:	//shh_220614
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (70.0 / 4.5);
			break;
		/*case C_LGC_70V_300A_100A_42KW_3:	//jhk_170211
		case C_LGC_70V_350A_100A_49KW:		//jhk_170911
		case C_LGC_70V_350A_100A_49KW_2:	//jhk_170911
		case C_LGC_70V_350A_100A_49KW_3:	//jhk_170911
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (70.0 / 5.0);
			break;
		*/ //shh_231025
		case C_SAEHAN_72V_200A_20A:
			//AMP -> INA128UA
			ratio /= 1.5;
			break;
		case C_NEWPOWER_75V_150A_75A_45KW:	//ljh_201105
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (75.0 / 5.625);
			break;
		case C_SDI_80V_50A_25A_10A:		//csk_130417_s
		case C_SDI_80V_50A_25A_10A_2:
		case C_SDI_80V_50A_25A_10A_3:
		case C_SDI_80V_50A_25A_10A_4:	//csk_130417_e
		case C_LGCCHINA_80V_150A_12KW:		//jhk_160725
		case C_LGCCHINA_80V_150A_12KW_2:	//jhk_160725
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (80.0 / 5.0);
			break;
		/*case C_LGC_90V_300A_100A_108KW:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_2:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_3:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_4:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_5:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_6:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_7:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_8:	//jhk_181016*/
		case C_LGC_90V_500A_250A_100A_90KW:	//jhk_181107
		//case C_LGC_90V_500A_250A_100A_180KW:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_2:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_3:	//jhk_181006	//sec_221027
		//case C_LGC_90V_500A_250A_100A_180KW_4:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_5:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_6:	//jhk_181006	//ktg_231023
		/*case C_LGC_90V_500A_250A_100A_180KW_7:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_8:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_9:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_10:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_11:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_12:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_13:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_14:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_15:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_16:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_17:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_18:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_19:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_20:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_21:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_22:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_23:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_24:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_25:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_26:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_27:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_28:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_29:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_30:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_31:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_32:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_33:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_34:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_35:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_36:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_37:	//ktg_190219 */	//ktg_231124
		case C_LGC_90V_500A_300A_100A_180KW:	//ktg_191106
		case C_LGC_90V_500A_300A_100A_180KW_2:	//ktg_191106
		//case C_LGC_90V_500A_300A_100A_180KW_3:	//ljh_201130
		//case C_LGC_90V_500A_300A_100A_180KW_4:	//ljh_201130
		//case C_LGC_90V_500A_300A_100A_180KW_5:	//ljh_201130	//shh_231027
		//case C_LGC_90V_500A_300A_100A_180KW_6:	//khj_200303	//ktg_231124
		//case C_LGC_90V_500A_300A_100A_180KW_7:	//shh_220324
		//case C_LGC_90V_500A_300A_100A_180KW_8:	//shh_220324
		//case C_LGC_90V_600A_300A_100A_216KW:	//ktg_190707
		//case C_LGC_90V_600A_300A_100A_216KW_2:	//ktg_200709
		//case C_LGC_90V_600A_300A_100A_216KW_3:	//ktg_200709
		//case C_LGC_90V_600A_300A_100A_216KW_4:	//ktg_200709
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (90.0 / 4.5);
			} else {
				ratio /= 1.494;
				ratio *= (40.0 / 4.5);
			}
			break;
		case C_LGC_90V_500A_250A_100A_180KW_38:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_39:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_40:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_41:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_42:	//ktg_200104
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (90.0 / 4.5);
			} else {
				ratio /= 1.494;
				ratio *= (45.0 / 4.5);
			}
			break;
		case C_LGCCHINA_90V_300A_100A_108KW:	//ktg_190319
			//AMP -> AD620A
				ratio /= 1.494;
				ratio *= (90.0 / 4.5);
			break;
		/*
		case C_LGC_100V_100A_50A_20KW:	//jhk_181023	//shh_231024
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (100.0 / 5.0);
			break;
		*/
		case C_JBTP_100V_100A_40KW:  	//shh_200927
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (100.0 / 5.0);
			break;
		case C_GITC_100V_100A_50A_10A_40KW:		//jhk_180917
		case C_SKI_100V_100A_10A_10KW:			//jhk_120329
		case C_POWERLOGICS_100V_100A_50A_40KW:	//jhk_180919
		case C_CORN_100V_100A_50A_40KW:			//ktg_190210
		case C_INCELL_100V_100A_50A_10A_40KW:	//ktg_190210
		case C_SEBANG_100V_100A_50A_10A_40KW:	//ktg_190324
		case C_LGC_100V_100A_10A_10KW:			//jhk_121009
		case C_LGC_100V_100A_10A_10KW_2:		//jhk_121009
		case C_LGC_100V_100A_10A_10KW_3:		//jhk_121009
		case C_LGC_100V_100A_10A_10KW_4:		//jhk_121009
		case C_JFMTECH_100V_100A_50A_40KW:		//csk_190923
		case C_SDI_100V_150A_70A_30KW:			//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_2:		//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_3:		//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_4:		//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_5:		//jhkw_130821
		case C_BOSUNG_100V_200A_100A_40KW:	//jhk_161226
		case C_KIER_100V_200A_100A_20A_40KW:	//jhk_161226
		case C_KATECH_100V_250A_100A_100KW:	//ktg_190120
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (100.0 / 4.5);
			break;
		case C_LGC_100V_150A_100A_30KW:	//jhk_190419
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (100.0 / 4.5);
			} else {
				ratio /= 1.494;
				ratio *= (50.0 / 4.5);
			}
			break;
		case C_KATECH_100V_30A_15A_5A:		//csk_131129
		case C_SAMHWA_100V_100A_10A:
		case C_LGCCHINA_100V_200A_20KW_3:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_4:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_5:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_6:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_7:	//jhk_160731
		case C_LGC_100V_200A_10A_20KW:
		case C_LGC_100V_200A_100A_80KW:		//ktg_200208
		case C_LGC_100V_250A_100A_100KW:	//jhk_181006
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (100.0 / 5.0);
			break;
		case C_POWERLOGICS_100V_150A_60KW:	//shh_200928
		case C_LAONTECH_100V_200A_40KW:		//shh_200927
		case C_HYUNDAI_100V_200A_100A_40KW:	//shh_201005
		case C_HYUNDAI_100V_200A_100A_80KW:	//shh_210311
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (100.0 / 5.0);
			break;
		case C_HCC_100V_300A_100A_60KW:  		//jhkw_131106
		case C_H2_100V_300A_100A_60KW:	//jhk_150726
		case C_SWISS_100V_300A_100A_60KW:	//jhk_160825
		case C_H2_100V_300A_100A_60KW_2:	//jhk_170110
		case C_SKI_100V_300A_150A_50A_10A_60KW: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_2: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_3: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_4: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_5: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_6: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_7: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_8: //kjh_120527
		//case C_LGC_100V_500A_250A_100KW: //jhk_180111
		//case C_LGC_100V_500A_250A_100KW_2: //jhk_180111
		//case C_LGC_100V_500A_250A_100KW_3: //jhk_180111
		//case C_LGC_100V_500A_250A_100KW_4: //jhk_180111	//shh_231030
		case C_SDI_100V_500A_250A_100KW: //ktg_190118
		case C_LGE_100V_30A_3KW:			//khj_191127
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (100.0 / 4.5);
			break;
		case C_KIER_100V_400A_200A_160KW:		//shh_201103
		case C_KTL_100V_500A_250A_50A_200KW:	//jhk_180718
		case C_KTL_100V_500A_100A_100KW:		//shh_200809
		case C_KTL_100V_500A_100A_100KW_2:  //ljh_210319
		case C_KTL_100V_500A_100A_100KW_3:	//ljh_210324
		case C_KTL_100V_500A_100A_100KW_4:	//ljh_210324
		case C_KBTP_100V_300A_100A_60KW:		//shh_210412
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (100.0 / 5.0);
			break;
		case C_VK_100V_200A_20A:
			//AMP -> INA128UA
			ratio /= 1.5;
			break;
		case C_EIG_100V_200A_20A_2:
			//AMP -> INA121
			ratio /= 1.5;
			ratio *= (213.0 / 13.0);
			break;
		case C_LGE_100V_400A_80KW:	//ljh_210331
		case C_LGE_100V_400A_80KW_2:	//ljh_210331
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (100.0 / 5.0);
			break;
		case C_SEINENG_100V_500A_200A_30A_200KW:	//ktg_200116
		case C_SEINENG_100V_500A_200A_30A_200KW_2:	//ktg_200116
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (100.0 / 5.0);
			} else {
				ratio /= 1.494;
				ratio *= (20.0 / 5.0);
			}
			break;
		case C_DAEHWA_100V_500A_250A_50KW:	//jhk_170915
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (100.0 / 4.5);
			break;
		case C_SDI_100V_600A_100A_200KW:	//jhk_181107
		case C_UNINTECH_100V_600A_300A_60A_120KW:	//ktg_200103
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (100.0 / 5.0);
			break;
		/*case C_LGC_110V_600A_300A_100A_216KW:	//ljh_201103
		case C_LGC_110V_600A_300A_100A_216KW_2:	//ljh_201103
		case C_LGC_110V_600A_300A_100A_216KW_3:	//ljh_201103
		case C_LGC_110V_600A_300A_100A_216KW_4:	//ljh_201103
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (110.0 / 5.5);
			break;*/
		/*case C_LGC_110V_600A_300A_100A_264KW:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_2:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_3:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_4:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_5:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_6:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_7:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_8:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_9:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_10:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_11:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_12:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_13:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_14:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_15:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_16:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_17:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_18:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_19:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_20:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_21:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_22:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_23:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_24:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_25:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_26:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_27:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_28:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_29:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_30:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_31:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_32:    //phb_220216
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (120.0 / 6.0);	
			} else if((range+1) == RANGE2) {
				ratio /= 1.494;
				ratio *= (60.0 / 6.0);	
			}
			break;*/
		/*case C_LGC_110V_600A_300A_100A_264KW_33:	//sec_221031
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (110.0 / 5.5);	
			} else if((range+1) == RANGE2) {
				ratio /= 1.494;
				ratio *= (55.0 / 5.5);	
			}
			break;*/
		case C_KATECH_120V_150A_100A_50A_18KW:	//jhkw_130621
		//case C_LGC_120V_250A_50A_60KW:	//jhk_160617	//shh_231027
		case C_LGC_120V_250A_50A_60KW_2:	//jhk_161101
		case C_SKI_120V_400A_100A_192KW:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_2:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_3:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_4:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_5:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_6:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_7:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_8:	//jhk_170628
		case C_SDI_120V_500A_250A_50A_120KW:	//ktg_190201
		case C_SDI_120V_500A_250A_50A_120KW_2:	//ktg_190201
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (120.0 / 4.5);
			break;
		case C_SKI_120V_400A_200A_100A_50A_192KW:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_2:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_3:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_4:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_5:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_6:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_7:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_8:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_9:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_10:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_11:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_12:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_13:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_14:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_15:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_16:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_17:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_18:	//jhk_180206
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
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (120.0 / 6.0);
			} else {
				ratio /= 1.494;
				ratio *= (60.0 / 6.0);
			}
			break;
		/*case C_TEST_120V_400A_200A_100A_192KW:		//shh_200723
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (120.0 / 6.0);
			} else {
				ratio /= 1.494;
				ratio *= (60.0 / 6.0);
			}
			break;*/
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
			//AMP -> AD620A
				ratio /= 1.494;
				ratio *= (120.0 / 4.5);
			break;
		case C_CSR_125V_200A_100A_50A_50KW:		//jhkw_121224
		case C_CSR_125V_200A_100A_50A_50KW_2:	//jhkw_121224
		case C_CSR_125V_200A_100A_50A_50KW_3:	//jhkw_121224
		case C_CSR_125V_200A_100A_50A_50KW_4:	//jhkw_121224
		case C_HUOJU_125V_200A_100A_50KW:	//jhk_151107
		case C_HUOJU_125V_200A_100A_50KW_2:	//jhk_151107
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (125.0 / 5.0);
			break;
		case C_GEELY_130V_300A_150A_78KW:	//shh_210406
		case C_GEELY_130V_300A_150A_78KW_2:	//shh_210406
		case C_GEELY_130V_600A_300A_156KW:	//ljh_210323
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (130.0 / 4.875);
			break;
		case C_SBL_150V_250A_10A_38KW: //csk_111007
		case C_SBL_150V_250A_10A_75KW:
		case C_SBL_150V_250A_10A_75KW_2:
		case C_SBL_150V_250A_10A_75KW_3:
		case C_SBL_150V_250A_10A_75KW_4: //csk_111007
		case C_SBL_150V_250A_10A_75KW_5: //csk_111007
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (150.0 / 5.0);
			break;
		/*case C_LGC_150V_250A_100A_75KW:	//ktg_190507	//shh_231024
		case C_LGC_150V_250A_100A_150KW:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_2:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_3:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_4:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_5:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_6:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_7:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_8:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_9:	//ktg_190514
		case C_LGC_150V_250A_100A_150KW_10:	//ktg_190514
		case C_LGC_150V_250A_100A_150KW_11:	//ktg_190514
		case C_LGC_150V_250A_100A_150KW_12:	//ktg_190514	
		case C_LGC_150V_350A_100A_105KW:	//ktg_190510
		//case C_LGC_150V_350A_100A_210KW:	//ktg_190421
		//case C_LGC_150V_350A_100A_210KW_2:	//ktg_190421
		//case C_LGC_150V_350A_100A_210KW_3:	//ktg_190421
		//case C_LGC_150V_350A_100A_210KW_4:	//ktg_190421	
		case C_LGC_150V_450A_200A_270KW:	//ktg_190421
		case C_LGC_150V_450A_200A_270KW_2:	//ktg_190421
		case C_LGC_150V_450A_200A_270KW_3:	//ktg_190421
		case C_LGC_150V_450A_200A_270KW_4:	//ktg_190421
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (150.0 / 4.5);
			} else {
				ratio /= 1.494;
				ratio *= (60.0 / 4.5);
			}
			break;*/	//shh_231025
		case C_CTP_150V_150A_100A_90KW:	//ktg_191111
		case C_TUV_150V_300A_100A_180KW:	//ktg_190429
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (150.0 / 4.5);
			}
			break;
		case C_LGE_150V_400A_60KW:	//jhk_170721
		case C_HYUNDAICE_150V_500A_250A_150KW:	//ktg_190520
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (150.0 / 4.5);
			break;
		case C_KTL_150V_600A_300A_200KW:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_2:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_3:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_4:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_5:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_6:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_7:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_8:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_9:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_10:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_11:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_12:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_13:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_14:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_15:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_16:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_17:	//ktg_211002
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (150.0 / 4.5);
			break;
		/*case C_LGC_150V_600A_300A_100A_300KW:	//shh_210819
		case C_LGC_150V_600A_300A_100A_300KW_2:	//shh_210819
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (150.0 / 4.5);
			} else {
				ratio /= 1.494;
				ratio *= (75.0 / 4.5);
			}
			break;*/
		case C_KORID_160V_500A_100A_80KW:	//ktg_190412
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (160.0 / 6.0);
			break;
		/*case C_LGC_200V_350A_100A_280KW:	//ktg_190425
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (200.0 / 6.0);
			} else {
				ratio /= 1.494;
				ratio *= (100.0 / 6.0);
			}
			break;*/
		case C_SEBANG_200V_100A_10A:
		case C_SEBANG_200V_100A_10A_2:
		case C_SEBANG_200V_100A_10A_3:
		case C_SEBANG_200V_100A_10A_4:
		case C_SEBANG_200V_100A_10A_5:
		//case C_LGC_200V_150A_50A_60KW:	//jhk_170905	//shh_231025
		case C_SEBANG_200V_200A_10A:
		//case C_LGC_200V_300A_100A_50A_10A_120KW:	//jhkw_120813	//shh_231027
		case C_ECOCAR_200V_200A_100A_50A_40KW:  //jhkw_130827
		case C_NORTHVOLT_200V_400A_200A_160KW:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_2:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_3:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_4:	//ktg_191203
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (200.0 / 5.0);
			break;
		case C_UTP_200V_500A_250A_100KW:	//ljh_201119
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (200.0 / 5.0);
			break;
		case C_OCI_220V_250A_100A_110KW:		//jhk_131220
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (220.0 / 5.5);
			break;
		//case C_LGC_220V_600A_300A_100A_528KW:       //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_2:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_3:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_4:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_5:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_6:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_7:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_8:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_9:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_10:    //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_11:    //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_12:    //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_13:    //phb_220228
		/*	//AMP -> AD620A
			if((range+1) == RANGE1) {
                ratio /= 1.494;
                ratio *= (220.0 / 5.5);
            } else {
                ratio /= 1.494;
                ratio *= (110.0 / 5.5);
            }
            break;*/
		case C_LGC_250V_120A_10A:
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (250.0 / 5.0);
			break;
		/*case C_LGC_250V_150A_100A_75KW:		//ktg_190510
		case C_LGC_250V_150A_100A_150KW:	//ktg_190513
		case C_LGC_250V_150A_100A_150KW_2:	//ktg_190513
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (250.0 / 6.0);
			} else {
				ratio /= 1.494;
				ratio *= (150.0 / 6.0);
			}
			break;*/	//shh_231025
		/*case C_LGC_250V_350A_100A_175KW:	//jhk_171023
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (250.0 / 5.0);
			break;*/	//shh_231025
		case C_LGC_250V_350A_250A_100A_175KW: //jhk_181108
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (250.0 / 5.0);
			} else {
				ratio /= 1.494;
				ratio *= (100.0 / 5.0);
			}
			break;
		case C_SEBANG_300V_250A_10A_150KW:
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (300.0 / 5.0);
			break;
		case C_KTC_300V_100V_250A_100A_25A_150KW:	//jhk_151107
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (300.0 / 4.5);
			} else {
				ratio /= 1.494;
				ratio *= (100.0 / 4.5);
			}
			break;
		case C_KBIA_300V_200A_100A_120KW:	  		//shh_200919	
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (300.0 / 4.5);
			break;
		case C_HYUNDAI_300V_300A_150A_180KW:	//ljh_201106
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (300.0 / 6.0);
			break;
		case C_SDIXIAN_300V_400A_100A_50A_25A:	//jhk_160527
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (300.0 / 6.0);
			break;
		case C_LGC_350V_150A_100A_105KW:	//khj_191230
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (350.0 / 5.0);
			} else {
				ratio /= 1.494;
				ratio *= (175.0 / 5.0);
			}
			break;
		/*case C_LGC_350V_300A_100A_420KW:	//shh_220128
		case C_LGC_350V_300A_100A_420KW_2:	//shh_220128
		case C_LGC_350V_300A_100A_420KW_3:	//shh_220128
		case C_LGC_350V_300A_100A_420KW_4:	//shh_220128
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (350.0 / 6.0);
			} else {
				ratio /= 1.494;
				ratio *= (175.0 / 6.0);
			}
			break;*/
		case C_LGC_400V_60A_10A:
		case C_KATECH_400V_60A_10A_24KW:
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (400.0 / 5.0);
			break;
		case C_ROTEM_400V_60A_10A:
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (500.0 / 5.0);
			break;
		case C_LGC_400V_100A_10A_40KW:
		case C_SKI_400V_100A_10A_40KW: //jhk_120112
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (400.0 / 5.0);
			break;
		case C_3PSYSTEM_400V_100A_10A_40KW_1:	//jhk_120511
		case C_3PSYSTEM_400V_100A_10A_40KW_2:	//jhk_120708
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (400.0 / 4.0);
			break;
		case C_AVL_400V_200A_10A_80KW:
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (400.0 / 5.0);
			break;
		case C_KATECH_400V_250A_10A:
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (400.0 / 6.0);
			break;
		case C_ERAE_400V_300A_100A_30KW:	//jhk_150119
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (400.0 / 4.0);
			break;
		case C_KATECH_450V_150A_10A:
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (450.0 / 4.5);
			break;
		case C_ROTEM_445V_265A_10A:
		//case C_PNE_450V_250A_10A_115KW_2:
		//case C_KATECH_450V_150A_10A:
		case C_LGC_450V_200A_10A:
		case C_LGC_450V_200A_10A_2:
		case C_LGC_450V_200A_10A_3:
		case C_LGC_450V_200A_10A_4:
		case C_LGC_450V_200A_10A_5:
		case C_LGC_450V_200A_10A_6:
		case C_SK_450V_200A_10A_180KW:
		case C_ROTEM_450V_200A_10A:
		case C_SK_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_HYUNDAI_450V_200A_10A_90KW:
		case C_VENS_450V_200A_10A_90KW:
		case C_KEPCO_450V_200A_10A_90KW:
		case C_LGC_450V_200A_20A_180KW:
		case C_LGC_450V_200A_20A_180KW_2:
		case C_SK_450V_200A_10A_360KW:
		case C_LGC_450V_200A_10A_180KW:
		//case C_LGC_450V_200A_10A_180KW_2:	//shh_231024
		case C_LGC_450V_200A_10A_180KW_3:	//kjhw_121102
		//case C_PNE_450V_250A_10A_115KW:
		case C_KATECH_450V_250A_10A_115KW:
		case C_KATECH_450V_250A_25A_225KW:
		case C_KATECH_450V_250A_25A_225KW_2:
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (450.0 / 5.0);
			break;
		case C_LGC_450V_200A_10A_180KW_4:	//jhkw_130909
		case C_HYUNDAI_450V_200A_10A_90KW_2:
		case C_BCST_450V_200A_10A_90KW:
		case C_CTNT_450V_200A_10A_180KW:	//kjh_110929
		case C_DAEWOO_450V_200A_50A_90KW: //kjh_150518
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
		case C_NKT_450V_200A_100A_90KW:	//ktg_190111
		case C_NKT_450V_200A_100A_90KW_2:	//ktg_190111
		case C_SDI_450V_225A_22A_202KW:		//jhkw_130629
		case C_SEM_450V_250A_10A_225KW:
		case C_LGC_450V_250A_10A_225KW: //jhk_140728
		case C_PNE_450V_250A_10A_115KW:		//jhk_131201
		//case C_LGE_450V_250A_50A_112KW:		//jhk_150703
		case C_LGE_450V_250A_50A_225KW:		//jhk_150605
		case C_LGE_450V_250A_50A_225KW_2:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_3:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_4:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_5:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_6:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_7:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_8:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_9:	//jhk_150605
		//case C_LGC_450V_250A_100A_225KW:	//jhk_170211	//shh_231024
		case C_KMI_450V_250A_100A_112KW:	//jhk_171110
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (450.0 / 4.5);
			break;
		/*
		case C_LGC_450V_250A_50A_225KW:		//jhk_160608
		case C_LGC_450V_250A_50A_225KW_2:	//jhk_160608
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (450.0 / 4.5);
			} else {
				ratio /= 1.494;
				ratio *= (100.0 / 4.5);
			}
			break;
		*/	//shh_231024
		case C_PNE_450V_250A_10A_115KW_2:	//jhkw_121005 450V -> 500V
		case C_LGC_500V_20A:
		case C_LGCCHINA_500V_150A_75KW:		//jhk_160727
		case C_LGCCHINA_500V_150A_75KW_2:	//jhk_160727
		case C_LGCCHINA_500V_150A_75KW_3:	//jhk_160727
		case C_KEPCO_500V_200A_10A_100KW:
		case C_KTL_500V_200A_10A_200KW_3:	//jhkw_130629
		case C_LGC_500V_200A_10A:
		case C_NEXCON_500V_200A_10A:
		case C_EIG_500V_200A_10A_200KW:
		case C_HLGP_500V_200A_10A_200KW:
		case C_LGC_500V_200A_10A_200KW:
		case C_NS_500V_250A_10A:
		//case C_PNE_500V_250A_10A_125KW: //kjhw_120507
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (500.0 / 5.0);
			break;
		case C_HLGP_500V_200A_100A_200KW: //kjh_120527
		case C_HLGP_500V_200A_100A_200KW_2: //jhkw_120528
			//AMP -> AD620A (interface b/d)
			//G1 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//100:1 = 500V:x -> x = 5000mV
			//AMP -> AD620A (vfb b/d)
			//G2 = 1
			//V1 = 5000mV * G2 = 5000mV
			//V2 = V1 * G1 = 7470mV
			//500000mV : 7470mV = V1 : 10000mV
			//V1 = 669344.042838mV
			//ratio = 669334042.838uV / 32768 = 20426.75911981
			
			//RANGE2
			//100:1 = 200V:x -> x = 2000mV
			//AMP -> AD620A (vfb b/d)
			//G2 = 1 + 49.4kohm / 32.9kohm = 2.5015197568
			//V1 = 2000mV * G2 = 5003.03951367781mV
			//V2 = V1 * G1 = 7474.5410334346504mV
			//200000mV : 7474.5410334346504mV = V1 : 10000mV
			//V1 = 267574.957586522680434mV
			//ratio = 334672021.419009uV / 32768 = 8165.73967244026
			
			if((range+1) == RANGE1) {
				ratio = 20426.75911981;
			} else {
				ratio = 8165.73967244;
			}
			break;
		case C_HLGP_500V_200A_100A_200KW_3: //jhkw_130223
		case C_HLGP_500V_200A_100A_200KW_4: //jhkw_130223
		case C_HLGP_500V_200A_100A_200KW_5: //jhk_131215
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (500.0 / 5.0);	
			} else {
				ratio /= 1.494;
				ratio *= (200.0 / 5.0);
			}
			break;
		case C_LGC_500V_200A_100A_200KW:    //jhkw_130305
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (500.0 / 5.0);
			break;
		case C_PNE_500V_250A_10A_125KW: //kjh_120520
			//AMP -> AD620A (interface b/d)
			//G1 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//100:1 = 500V:x -> x = 5000mV
			//AMP -> AD620A (vfb b/d)
			//G2 = 1
			//V1 = 5000mV * G2 = 5000mV
			//V2 = V1 * G1 = 7470mV
			//500000mV : 7470mV = V1 : 10000mV
			//V1 = 669344.042838mV
			//ratio = 669334042.838uV / 32768 = 20426.75911981
			
			//RANGE2
			//100:1 = 250V:x -> x = 2500mV
			//AMP -> AD620A (vfb b/d)
			//G2 = 1 + 49.4kohm / 49.4kohm = 2
			//V1 = 2500mV * G2 = 5000mV
			//V2 = V1 * G1 = 7470mV
			//250000mV : 7470mV = V1 : 10000mV
			//V1 = 334672.021419009mV
			//ratio = 334672021.419009uV / 32768 = 10213.379559906
			
			//RANGE3
			//100:1 = 100V:x -> x = 1000mV
			//AMP -> AD620A (vfb b/d)
			//G2 = 1 + 49.4kohm / 12.35kohm = 5
			//V1 = 1000mV * G2 = 5000mV
			//V2 = V1 * G1 = 7470mV
			//100000mV : 7470mV = V1 : 10000mV
			//V1 = 133868.8085676037mV
			//ratio = 133868808.5676037uV / 32768 = 4085.3518239625
			
			//RANGE4
			//100:1 = 50V:x -> x = 500mV
			//AMP -> AD620A (vfb b/d)
			//G2 = 1 + 49.4kohm / 5.49kohm = 9.99817
			//V1 = 500mV * G2 = 4999.0892531mV
			//V2 = V1 * G1 = 7470mV
			//50000mV : 7468.639344262mV = V1 : 10000mV
			//V1 = 66946.59856405mV
			//ratio = 66946598.56405uV / 32768 = 2043.04805188
			
			if((range+1) == RANGE1) {
				ratio = 20426.75911981;
			} else if((range+1) == RANGE2) {
				ratio = 10213.37955990;
			} else if((range+1) == RANGE3) {
				ratio = 4085.35182396;
			} else {
				ratio = 2043.04805188;
			}
			break;
		case C_LGE_500V_250A_50A_250KW:	//jhk_150607
		case C_LGC_500V_250A_100A_50A_250KW:	//jhkw_120809
		case C_KTL_500V_250A_100A_50A_250KW:	//jhkw_130320
		case C_HYUNDAI_500V_250A_125A_25A_250KW: //ktg_190324
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (500.0 / 5.0);
			break;
		case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (500.0 / 5.0);	
			} else if((range+1) == RANGE2) {
				ratio /= 1.494;
				ratio *= (300.0 / 5.0);	
			} else {
				ratio /= 1.494;
				ratio *= (100.0 / 5.0);
			}
			break;
		case C_KBIA_500V_200A_100A_200KW:	//shh_200918
		case C_KBIA_500V_250A_100A_250KW:	//shh_200916
		case C_JBTP_500V_250A_100A_500KW:	//shh_200927
		case C_KBIA_500V_300A_100A_150KW:	//shh_200919
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (500.0 / 5.0);
			break;	
		//case C_LGC_500V_250A_100A_125KW:	//ktg_190620
		/*case C_LGC_500V_250A_125A_50A_500KW_2: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_3: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_4: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_5: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_6: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_7: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_8: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_9: //ktg_190227*/	//ktg_231124
		//case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_190615
		//case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_190615	//shh_231024
		case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (500.0 / 5.0);	
			} else if((range+1) == RANGE2) {
				ratio /= 1.494;
				ratio *= (200.0 / 5.0);	
			}
			break;
		case C_HLGP_500V_300A_100A_300KW:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_2:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_3:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_4:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_5:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_6:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_7:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_8:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_9:	//jhk_141127
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (500.0 / 5.0);	
			} else {
				ratio /= 1.494;
				ratio *= (200.0 / 5.0);
			}
			break;
		case C_LGC_500V_250A_100A_50A_250KW_2:	//ktg_200208
		case C_HLGP_500V_300A_100A_300KW_10:	//jhk_170722
		//case C_LGC_500V_300A_100A_300KW:	//ktg_190418
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (500.0 / 5.0);	
			} else if((range+1) == RANGE2) {
				ratio /= 1.494;
				ratio *= (250.0 / 5.0);	
			}
			break;
		case C_SDI_500V_300A_100A_75KW:		//ktg_200809
		case C_SDI_500V_300A_100A_150KW:	//ktg_200809
		case C_SDI_500V_300A_100A_150KW_2:	//ktg_200809
		case C_SDI_500V_300A_100A_150KW_3:	//ktg_200809
		case C_SDI_500V_300A_100A_150KW_4:	//ktg_200809
		case C_GANGSO_500V_400A_200KW:	//jhk_160518
		case C_HLGP_500V_400A_200KW:	//jhk_170625
		case C_SKI_500V_400A_200KW:	//jhk_171015
		case C_LGC_500V_400A_200A_400KW:	//ktg_190716
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (500.0 / 5.0);
			break;	
		case C_SDI_600V_60A_72KW:			//shh_201202
		case C_SDI_600V_60A_72KW_2:			//shh_201202
		case C_SDI_600V_60A_72KW_3:			//shh_201202
		case C_SDI_600V_60A_72KW_4:			//shh_201202
		case C_SDI_600V_60A_72KW_5:			//shh_201202
		case C_SDI_600V_60A_72KW_6:			//shh_201202
		case C_SDI_600V_60A_72KW_7:			//shh_201202
		case C_SDI_600V_60A_72KW_8:			//shh_201202
		case C_SDI_600V_60A_72KW_9:			//shh_201202
		case C_SDI_600V_60A_72KW_10:		//shh_201202
		case C_SDI_600V_60A_72KW_11:		//shh_201202
		case C_SDI_600V_60A_72KW_12:		//shh_201202
		case C_SDI_600V_60A_72KW_13:		//shh_201202
		case C_SDI_600V_60A_72KW_14:		//shh_201202
		case C_SDI_600V_60A_72KW_15:		//shh_201202
		case C_SDI_600V_60A_72KW_16:		//shh_201202
		case C_SDI_600V_60A_72KW_17:		//shh_201202
		case C_SDI_600V_60A_72KW_18:		//shh_201202
		case C_SDI_600V_60A_72KW_19:		//shh_201202
		case C_SDI_600V_60A_72KW_20:		//shh_201202
		case C_SDI_600V_60A_72KW_21:		//shh_201202
		case C_SDI_600V_60A_72KW_22:		//shh_201202
		case C_SDI_600V_60A_72KW_23:		//shh_201202
		case C_SDI_600V_60A_72KW_24:		//shh_201202
		case C_SDI_600V_60A_72KW_25:		//shh_201202
		case C_SDI_600V_60A_72KW_26:		//shh_201202
		case C_SDI_600V_60A_72KW_27:		//shh_201202
		case C_SDI_600V_60A_72KW_28:		//shh_201202
		case C_SDI_600V_60A_72KW_29:		//shh_201202
		case C_SDI_600V_60A_72KW_30:		//shh_201202
		case C_SDI_600V_60A_72KW_31:		//shh_201202
		case C_SDI_600V_60A_72KW_32:		//shh_201202
		case C_SDI_600V_60A_72KW_33:		//shh_201202
		case C_SDI_600V_60A_72KW_34:		//shh_201202
		case C_SDI_600V_60A_72KW_35:		//shh_201202
		case C_SDI_600V_60A_72KW_36:		//shh_201202
		case C_SDI_600V_60A_72KW_37:		//shh_201202
		case C_SDI_600V_60A_72KW_38:		//shh_201202
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (600.0 / 6.0);
			break;	
		//case C_LGC_600V_150A_100A_180KW: //ktg_190405 //shh_231024
		case C_LGE_600V_150A_100A_180KW: //shh_210426
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (600.0 / 6.0);	
			} else if((range+1) == RANGE2) {
				ratio /= 1.494;
				ratio *= (200.0 / 6.0);	
			}
			break;
		/*case C_LGC_600V_300A_100A_360KW:	//ktg_190620
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (600.0 / 4.5);	
			} else if((range+1) == RANGE2) {
				ratio /= 1.494;
				ratio *= (300.0 / 4.5);	
			}
			break;*/	//shh_231025
		case C_LGC_600V_300A_100A_360KW_2:	//ktg_200104
		case C_LGC_600V_300A_100A_360KW_3:	//ktg_200108
		case C_LGC_600V_300A_100A_360KW_4:	//ktg_200108
		//case C_LGC_600V_300A_100A_720KW:	//shh_220204
		//case C_LGC_600V_300A_100A_720KW_2:	//shh_220204
		//case C_LGC_600V_300A_100A_720KW_3:	//shh_220204
		//case C_LGC_600V_300A_100A_720KW_4:	//shh_220204
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (600.0 / 6.0);	
			} else if((range+1) == RANGE2) {
				ratio /= 1.494;
				ratio *= (300.0 / 6.0);	
			}
			break;
		case C_KCL_600V_200A_100A_480KW: //ktg_190812
		case C_KCL_600V_200A_100A_480KW_2: //ktg_190812
		//case C_SEINENG_600V_300A_100A_30A_500KW:	//ktg_200112
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (600.0 / 4.5);	
			break;
		case C_KTL_600V_200A_100A_20A_400KW:	//jhk_180731
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (600.0 / 6.0);	
			/*} else if((range+1) == RANGE2) {
				ratio /= 1.494;
				ratio *= (300.0 / 6.0);	
			} else {
				ratio /= 1.494;
				ratio *= (60.0 / 6.0);
			}*/	//ktg_200622
			} else {
				ratio /= 1.494;
				ratio *= (100.0 / 6.0);
			}
			break;
		case C_SEINENG_600V_300A_100A_30A_500KW:	//ktg_200112
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (600.0 / 4.5);	
			} else if((range+1) == RANGE2) {
				ratio /= 1.494;
				ratio *= (200.0 / 4.5);	
			}
			break;
		case C_SEBANG_600V_200A_100A_240KW:	//jhk_160223
		case C_SKC_600V_200A_100A_50A_240KW: //jhkw_120807
		case C_LGE_600V_250A_50A_150KW:	//jhk_150520
		case C_SDI_600V_300A_100A_90KW:		//ktg_200809
		case C_INTS_600V_300A_100A_180KW:			//shh_200513 
		case C_UTP_600V_300A_100A_150KW:	//jhk_151107
		case C_SKE_600V_400A_100A_50A_25A_240KW:	//jhkw_130924
		case C_DAEHWA_600V_400A_200A_100A_200KW:	//jhk_170120
		/*case C_HYUNDAI_600V_400A_200A_100A_240KW:	//khj_190805
		//case C_HYUNDAI_600V_400A_200A_100A_240KW_2:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_3:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_4:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_5:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_6:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_7:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_8:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_9:	//khj_190805 */
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (600.0 / 6.0);
			break;
/*		case C_SKC_600V_200A_100A_50A_240KW_2: //jhkw_121019
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (600.0 / 6.0);	
			} else {
				ratio /= 1.494;
				ratio *= (100.0 / 6.0);
			}
			break;
*/
		case C_SKC_600V_200A_100A_50A_240KW_2: //jhkw_121031
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (600.0 / 6.0);	
			} else {
				ratio /= 1.494;
				ratio *= (200.0 / 6.0);
			}
			break;
		case C_DAEHWA_750V_300A_100A_150KW:	//jhk_170203
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (750.0 / 4.5);
			break;
		case C_HANGKE_750V_300A_100A_300KW:		//jhk_131226
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (750.0 / 7.5);
			break;
		case C_HYUNDAI_750V_400A_100A_240KW:	//jhk_160909
		case C_HYUNDAI_750V_400A_100A_240KW_2:	//jhk_161111
		case C_HYUNDAI_750V_400A_100A_240KW_3:	//jhk_171015
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (750.0 / 4.5);
			break;
		case C_STECHWIN_800V_200A_50A_320KW:	//jhk_150812
		case C_DAEHWA_800V_200A_50A_160KW:		//jhk_160831
		case C_CS_800V_250A_50A_250KW:			//jhk_150812
		case C_ADD_800V_350A_200A_50A_380KW:		//shh_200904
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (800.0 / 6.0);
			break;
		case C_POWERLOGICS_1000V_200A_100A_200KW:	//jhk_180921
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (1000.0 / 5.0);
			break;
		case C_HYUNDAI_1000V_250A_100A_500KW:	//shh_200513
		case C_KOSTA_1000V_250A_100A_600KW:		//shh_201102
		case C_KBTP_1000V_300A_100A_300KW:	//ljh_210329
		case C_HLGP_1000V_300A_100A_50A_600KW:	//jhk_180810
		case C_HLGP_1000V_300A_100A_50A_600KW_2:	//jhk_180917
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (1000.0 / 5.0);
			} else {
				ratio /= 1.494;
				ratio *= (500.0 / 5.0);	
			}
			break;
		case C_HYUNDAI_1000V_500A_250A_500KW:	//jhk_180722
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (1000.0 / 5.0);
			break;
		case C_BOSUNG_1200V_200A_100A_200KW:	//jhk_170106
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (1200.0 / 6.0);
			break;
		case C_SKI_1200V_250A_100A_50A_300KW:	//jhk_131209
		case C_KATECH_1200V_300A_100A_50A_300KW:	//jhk_150108
		case C_KCL_1200V_1000A_400A_600KW:		//shh_200809 	
		case C_KCL_1200V_1000A_400A_600KW_2:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_600KW_3:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_600KW_4:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_700KW:		//shh_200809 	
		case C_KCL_1200V_1000A_400A_700KW_2:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_800KW:		//shh_200809 	
		case C_KCL_1200V_1000A_400A_800KW_2:	//shh_200809 	
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_2:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_3:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_4:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_5:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_6:	//shh_210625
		case C_HYUNDAI_1200V_1000A_500A_300A_800KW:	//shh_210915
		case C_HYUNDAI_1200V_1000A_500A_300A_800KW_2:	//shh_211020
		case C_KATECH_1200V_1000A_500A_300A_600KW:		//shh_210611
		case C_KATECH_1200V_1000A_500A_300A_600KW_2:	//shh_210611
		case C_UTP_1200V_1000A_500A_300A_600KW:			//shh_201120
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (1200.0 / 6.0);
			} else {
				ratio /= 1.494;
				ratio *= (600.0 / 6.0);	
			}
			break;
		case C_KTL_1200V_300A_100A_300KW:	//kjh_171013
		case C_SEBANG_1200V_300A_100A_360KW:	//jhk_160223
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (1200.0 / 6.0);
			break;
		case C_NCT_1500V_250A_100A_400KW:	//ljh_201113
			//AMP -> OP270
			if((range+1) == RANGE1) {
					ratio /= 1.494;
					ratio *= (1500.0 / 4.5);
			} else {
					ratio /= 1.494;
					ratio *= (750.0 / 4.5);
			}
			break;
		case C_SKI_1500V_400A_100A_500KW:	//jhk_180610
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (1500.0 / 6.0);
			break;
		case C_KTL_1500V_400A_200A_700KW:	//jhk_180902
		case C_CTP_1500V_600A_300A_60A_600KW:	//ktg_191117
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (1500.0 / 6.0);
			} else {
				ratio /= 1.494;
				ratio *= (750.0 / 6.0);	
			}
			break;
		case C_KTC_1500V_600A_200A_400KW:	//ktg_200807
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (1500.0 / 4.5);
			} else {
				ratio /= 1.494;
				ratio *= (750.0 / 4.5);	
			}
			break;
		case C_KEIR_1500V_600A_300A_200A_100A_720KW:	//ktg_211202
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (1500.0 / 6.0);
			} else {
				ratio /= 1.494;
				ratio *= (600.0 / 6.0);	
			}
			break;
		case C_KCL_1500V_1000A_500A_600KW:	//jhk_181110
		case C_HYUNDAI_1500V_1000A_500A_600KW:	//jhk_170917
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (1500.0 / 6.0);
			break;
		default:
			//AMP -> AD620A
			//ratio /= 1.494;
			//ratio = 305.1757813;
			
			//10000000uV / 32768 = 305.1757813uV
			//10000mV 	 / 32768 = 0.305175781mV
			//10V 		 / 32768 = 0.000305175V
			main_amp = myPs->config.main_amp; //1.494	//kjh_211021s
			maxV = myPs->config.maxV[range];
			da_max = myPs->config.da_max;
			v_feed = myPs->config.v_feed[range];

			if(main_amp <= 0.0 || maxV <= 0 || da_max <= 0.0 || v_feed <= 0.0) {
				ratio = 0.0;
			} else {
				ratio = da_max / 32768.0 / main_amp;
				ratio *= ((double)maxV / v_feed);
			} //kjh_211021e
			break;
	}

	return ratio;
}

double Ref_I_Ratio(int range)
{
	double ratio;
	long maxI; //kjh_210315
	double da_max, i_feed, main_amp; //kjh_210315

	//G_i	: Gain of InterfaceBd
	//G_m	: Gain of MainBd
	//G_s	: Gain of Shunt
	//G_r*	: Gain of CH_Range*

	switch(myData->AppControl.config.systemModel) {
		//shuntR used
		case F_PNE_5V_15A_30AP_SW:
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 1.25kohm) * (1 + 49.4kohm / 100kohm)
			// = 60.53688

			//RANGE1
			//shuntV = 5.0mOhm * 15A = 75mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / xOhm = 1.0
			//V1 = shuntV * G1 = 75000uV
			//V2 = V1 * G2 = 4540266uV
			//15000000uA : 4540266uV = I1 : 10000000uV
			//I1 = 33037711.89uA
			//ratio = 33037711.89uA / 32768 = 1008.230954
		
			//RANGE2
			//shuntV = 5.0mOhm * 1.5A = 7.5mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.49kOhm = 9.9982
			//V1 = shuntV * G1 = 74986.5uV
			//V2 = V1 * G2 = 4539448.752uV
			//1500000uA : 4539448.752uV = I1 : 10000000uV
			//I1 = 3304365.975uA
			//ratio = 3304365.975uA / 32768 = 100.8412468
		
			if((range+1) == RANGE1) ratio = 1008.230954;
			else ratio = 100.8412468;
			break;
		case C_EIG_5V_50A_5A:
		case C_EIG_5V_50A_5A_2:
		case F_SDI_5V_50A_5A:
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 830ohm) * (1 + 49.4kohm / 100kohm) = 90.414

			//RANGE1
			//shuntV = 2.0mOhm * 50A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / xOhm = 1.0
			//V1 = shuntV * G1 = 100000uV
			//V2 = V1 * G2 = 9041400uV
			//50000000uA : 9041400uV = I1 : 10000000uV
			//I1 = 55301170.2uA
			//ratio = 55301170.2uA / 32768 = 1687.658
		
			//RANGE2
			//shuntV = 2.0mOhm * 5A = 10.0mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.49kOhm = 9.9982
			//V1 = shuntV * G1 = 99982uV
			//V2 = V1 * G2 = 9039772.548uV
			//5000000uA : 9039772.548uV = I1 : 10000000uV
			//I1 = 5531112.618uA
			//ratio = 5531112.618uA / 32768 = 168.7962
		
			if((range+1) == RANGE1) ratio = 1687.658;
			else ratio = 168.7962;
			break;
		case C_LGC_5V_150A_10A:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> INA128UA
			//G1 = 1 + 50kohm / 470ohm = 107.3829787
			//AMP -> INA128UA
			//G2 = 1 + 50kohm / 100kohm = 1.5

			//RANGE1
			//V1 = 50000uV * G1 = 5369148.935uV
			//V2 = V1 * G2 = 8053723.403uV
			//150000000uA : 8053723.403uV = I1 : 10000000uV
			//I1 = 186249257uA
			//ratio = 186249257uA / 32768 = 5683.876251
		
			//RANGE2
			//V1 = 50000uV * G1 = 5369148.935uV
			//V2 = V1 * G2 = 8053723.403uV
			//10000000uA : 8053723.403uV = I1 : 10000000uV
			//I1 = 12416617.13uA
			//ratio = 12416617.13uA / 32768 = 378.9250833
			if((range+1) == RANGE1) ratio = 5683.876251;
			else ratio = 378.9250833;
			break;
		case C_LGC_5V_200A_75A_15A:
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 830ohm) * (1 + 49.4kohm / 100kohm) = 90.414

			//RANGE1
			//shuntV = 0.5mOhm * 200A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / xOhm = 1.0
			//V1 = shuntV * G1 = 100000uV
			//V2 = V1 * G2 = 9041400uV
			//200000000uA : 9041400uV = I1 : 10000000uV
			//I1 = 221204680.7uA
			//ratio = 221204680.7uA / 32768 = 6750.631125
		
			//RANGE2
			//shuntV = 0.5mOhm * 75A = 37.5mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 29.76kOhm = 2.65995
			//V1 = shuntV * G1 = 99747.98389uV
			//V2 = V1 * G2 = 9018614.215uV
			//75000000uA : 9018614.215uV = I1 : 10000000uV
			//I1 = 83161335.23uA
			//ratio = 83161335.23uA / 32768 = 2537.882545
		
			//RANGE3
			//shuntV = 0.5mOhm * 15A = 7.5mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4kOhm = 13.35
			//V1 = shuntV * G1 = 100125uV
			//V2 = V1 * G2 = 9052701.75uV
			//15000000uA : 9052701.75uV = I1 : 10000000uV
			//I1 = 16569639uA
			//ratio = 16569639uA / 32768 = 505.6652527
			if((range+1) == RANGE1) ratio = 6750.662527;
			else if((range+1) == RANGE2) ratio = 2537.882545;
			else ratio = 505.6652527;
			break;
		case C_ENERLAND_5V_250A:
		case C_ENERLAND_5V_250A_50A_5A: //kjg_w
		case C_ENERLAND_5V_250A_50A_5A_2: //kjg_w
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 830ohm = 60.51807229
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//shuntV = 250A * 0.4mOhm = 100mV
			//V1 = shuntV * G1 = 6051.807229mV
			//V2 = V1 * G2 = 9041.4mV
			//250000mA : 9041.4mV = I1 : 10000mV
			//I1 = 276505.8509mA
			//ratio = 276505850.9uA / 32768 = 8438.288907
			ratio = 8438.288907;
			break;
		case F_SDI_5V_200A_100A_50A_10A:		//csk_161007
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 100kohm) = 1.494

			//RANGE1
			//shuntV = 0.5mOhm * 200A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 820Ohm = 61.2439
			//V1 = shuntV * G1 = 6124390uV
			//V2 = V1 * G2 = 9149839uV
			//200000000uA : 9149839uV = I1 : 10000000uV
			//I1 = 218583080.5uA
			//ratio = 218583080.5uA / 32768 = 6670.6262358
		
			//RANGE2
			//shuntV = 0.5mOhm * 100A = 50mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / (820Ohm // 820ohm) = 121.4878049
			//V1 = shuntV * G1 = 6074390uV
			//V2 = V1 * G2 = 9075139uV
			//100000000uA : 9075139uV = I1 : 10000000uV
			//I1 = 110191149.4uA
			//ratio = 110191149.4uA / 32768 = 3362.76701
		
			//RANGE3
			//shuntV = 0.5mOhm * 50A = 25mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / (820Ohm // 820Ohm // 237Ohm) = 241.9756098
			//V1 = shuntV * G1 = 6049390uV
			//V2 = V1 * G2 = 9037789uV
			//50000000uA : 9037789uV = I1 : 10000000uV
			//I1 = 55323265.3uA
			//ratio = 55323265.3uA / 32768 = 1688.332071
			
			//RANGE4
			//shuntV = 10mOhm * 10A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 830Ohm = 60.5181
			//V1 = shuntV * G1 = 6051807uV
			//V2 = V1 * G2 = 9041400uV
			//10000000uA : 9041400uV = I1 : 10000000uV
			//I1 = 11060234.03uA
			//ratio = 11060234.03uA / 32768 = 337.5315561
			if((range+1) == RANGE1) ratio = 6770.6262358;
			else if((range+1) == RANGE2) ratio = 3362.76701;
			else if((range+1) == RANGE3) ratio = 1688.332071;
			else ratio = 337.5315561;
			break;
		case F_SBL_5V_250A_125A_65A_10A:
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 100kohm) = 1.494

			//RANGE1
			//shuntV = 0.4mOhm * 250A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 800Ohm = 62.75
			//V1 = shuntV * G1 = 6275000uV
			//V2 = V1 * G2 = 9374850uV
			//250000000uA : 9374850uV = I1 : 10000000uV
			//I1 = 266670933.4uA
			//ratio = 266670933.4uA / 32768 = 8138.151044
		
			//RANGE2
			//shuntV = 0.4mOhm * 125A = 50.0mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 400Ohm = 124.5
			//V1 = shuntV * G1 = 6225000uV
			//V2 = V1 * G2 = 9300150uV
			//125000000uA : 9300150uV = I1 : 10000000uV
			//I1 = 134406434.3uA
			//ratio = 134406434.3uA / 32768 = 4101.758859
		
			//RANGE3
			//shuntV = 0.4mOhm * 65A = 26.0mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 207.229Ohm = 239.3837209
			//V1 = shuntV * G1 = 6223977uV
			//V2 = V1 * G2 = 9298621uV
			//65000000uA : 9298621uV = I1 : 10000000uV
			//I1 = 69902836.4uA
			//ratio = 69902836.4uA / 32768 = 2133.26527
			
			//RANGE4
			//shuntV = 10mOhm * 10A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 800Ohm = 62.75
			//V1 = shuntV * G1 = 6275000uV
			//V2 = V1 * G2 = 9374850uV
			//10000000uA : 9374850uV = I1 : 10000000uV
			//I1 = 10666837.34uA
			//ratio = 10666837.34uA / 32768 = 325.5260419
			if((range+1) == RANGE1) ratio = 8138.151044;
			else if((range+1) == RANGE2) ratio = 4101.758859;
			else if((range+1) == RANGE3) ratio = 2133.26527;
			else ratio = 325.5260419;
			break;
		case F_SDI_5V_400A_200A_100A_10A:
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 100kohm) = 1.494

			//RANGE1
			//shuntV = 0.25mOhm * 420A = 105mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 850Ohm = 59.117647
			//V1 = shuntV * G1 = 6207353uV
			//V2 = V1 * G2 = 9374850uV
			//420000000uA : 9273785uV = I1 : 10000000uV
			//I1 = 452889497.3uA
			//ratio = 452889497.3uA / 32768 = 13821.09062
		
			//RANGE2
			//shuntV = 0.25mOhm * 200A = 50.0mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 412.121Ohm = 120.86771
			//V1 = shuntV * G1 = 6043385uV
			//V2 = V1 * G2 = 9028817uV
			//200000000uA : 9028817uV = I1 : 10000000uV
			//I1 = 221512957.7uA
			//ratio = 221512957.7uA / 32768 = 6760.038991
		
			//RANGE3
			//shuntV = 0.25mOhm * 100A = 25.0mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 202.9851Ohm = 244.367647
			//V1 = shuntV * G1 = 6109191uV
			//V2 = V1 * G2 = 9262800uV
			//100000000uA : 9127131uV = I1 : 10000000uV
			//I1 = 109563450.0uA
			//ratio = 109563450.0uA / 32768 = 3343.611146
			
			//RANGE4
			//shuntV = 10mOhm * 10A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 800Ohm = 62.75
			//V1 = shuntV * G1 = 6275000uV
			//V2 = V1 * G2 = 9374850uV
			//10000000uA : 9374850uV = I1 : 10000000uV
			//I1 = 10666837.34uA
			//ratio = 10666837.34uA / 32768 = 325.5260419
			if((range+1) == RANGE1) ratio = 13821.09062;
			else if((range+1) == RANGE2) ratio = 6760.038991;
			else if((range+1) == RANGE3) ratio = 3343.611146;
			else ratio = 325.5260419;
			break;
		case F_SDI_5V_400A_200A_100A_10A_2:
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 100kohm) = 1.494

			//RANGE1
			//shuntV = 0.25mOhm * 400A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 800Ohm = 62.75
			//V1 = shuntV * G1 = 6275000uV
			//V2 = V1 * G2 = 9374850uV
			//400000000uA : 9374850uV = I1 : 10000000uV
			//I1 = 426673493.4uA
			//ratio = 426673493.4uA / 32768 = 13021.04167
		
			//RANGE2
			//shuntV = 0.25mOhm * 200A = 50.0mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 400Ohm = 124.5
			//V1 = shuntV * G1 = 6225000uV
			//V2 = V1 * G2 = 9300150uV
			//200000000uA : 9300150uV = I1 : 10000000uV
			//I1 = 215050294.9uA
			//ratio = 215050294.9uA / 32768 = 6562.814175
		
			//RANGE3
			//shuntV = 0.25mOhm * 100A = 25.0mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 200Ohm = 248
			//V1 = shuntV * G1 = 6200000uV
			//V2 = V1 * G2 = 9262800uV
			//100000000uA : 9262800uV = I1 : 10000000uV
			//I1 = 107958716.6uA
			//ratio = 107958716.6uA / 32768 = 3294.638568
			
			//RANGE4
			//shuntV = 10mOhm * 10A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 800Ohm = 62.75
			//V1 = shuntV * G1 = 6275000uV
			//V2 = V1 * G2 = 9374850uV
			//10000000uA : 9374850uV = I1 : 10000000uV
			//I1 = 10666837.34uA
			//ratio = 10666837.34uA / 32768 = 325.5260419
			if((range+1) == RANGE1) ratio = 13021.04167;
			else if((range+1) == RANGE2) ratio = 6562.814175;
			else if((range+1) == RANGE3) ratio = 3294.638568;
			else ratio = 325.5260419;
			break;
		case F_SDI_5V_450A_200A_100A_10A:
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 100kohm) = 1.494

			//RANGE1
			//shuntV = 0.25mOhm * 480A = 120mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 980Ohm = 51.408
			//V1 = shuntV * G1 = 6168980uV
			//V2 = V1 * G2 = 9216456uV
			//480000000uA : 9216456uV = I1 : 10000000uV
			//I1 = 520807564.2uA
			//ratio = 520807564.2uA / 32768 = 15893.78553
		
			//RANGE2
			//shuntV = 0.25mOhm * 200A = 50.0mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 401.446Ohm = 124.055
			//V1 = shuntV * G1 = 6202761uV
			//V2 = V1 * G2 = 9266925uV
			//200000000uA : 9266925uV = I1 : 10000000uV
			//I1 = 215821319.4uA
			//ratio = 215821319.4uA / 32768 = 6586.343977
		
			//RANGE3
			//shuntV = 0.25mOhm * 100A = 25.0mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 200.361Ohm = 247.555
			//V1 = shuntV * G1 = 6188881uV
			//V2 = V1 * G2 = 9246188uV
			//100000000uA : 9246188uV = I1 : 10000000uV
			//I1 = 108152684.0uA
			//ratio = 108152684.0uA / 32768 = 3300.557984
			
			//RANGE4
			//shuntV = 10mOhm * 10A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 800Ohm = 62.75
			//V1 = shuntV * G1 = 6275000uV
			//V2 = V1 * G2 = 9374850uV
			//10000000uA : 9374850uV = I1 : 10000000uV
			//I1 = 10666837.34uA
			//ratio = 10666837.34uA / 32768 = 325.5260419
			if((range+1) == RANGE1) ratio = 15893.78553;
			else if((range+1) == RANGE2) ratio = 6586.343977;
			else if((range+1) == RANGE3) ratio = 3300.557984;
			else ratio = 325.5260419;
			break;
		case F_SDI_5V_450A_200A_100A_10A_2:
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 100kohm) = 1.494

			//RANGE1
			//shuntV = 0.222mOhm * 450A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 830Ohm = 60.5181
			//V1 = shuntV * G1 = 6051807uV
			//V2 = V1 * G2 = 9041400uV
			//450000000uA : 9041400uV = I1 : 10000000uV
			//I1 = 497710531.6uA
			//ratio = 497710531.6uA / 32768 = 15188.92003
		
			//RANGE2
			//shuntV = 0.222mOhm * 200A = 44.444mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / (830Ohm // 1kohm) = 109.9180723
			//V1 = shuntV * G1 = 4885248uV
			//V2 = V1 * G2 = 7298560uV
			//200000000uA : 7298560uV = I1 : 10000000uV
			//I1 = 134406434.3uA
			//ratio = 274026657.3uA / 32768 = 8362.629922
		
			//RANGE3
			//shuntV = 0.222mOhm * 100A = 22.222mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / (830Ohm // 318Ohm) = 215.8639842
			//V1 = shuntV * G1 = 4796977uV
			//V2 = V1 * G2 = 7166684uV
			//100000000uA : 7166684uV = I1 : 10000000uV
			//I1 = 139534540.9uA
			//ratio = 139534540.9uA / 32768 = 4258.256253
			
			//RANGE4
			//shuntV = 10mOhm * 10A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 830Ohm = 60.5181
			//V1 = shuntV * G1 = 6051807uV
			//V2 = V1 * G2 = 9041400uV
			//10000000uA : 9041400uV = I1 : 10000000uV
			//I1 = 11060234.03uA
			//ratio = 11060234.03uA / 32768 = 337.5315561
			if((range+1) == RANGE1) ratio = 15188.92003;
			else if((range+1) == RANGE2) ratio = 8362.629922;
			else if((range+1) == RANGE3) ratio = 4258.256253;
			else ratio = 337.5315561;
			break;
		case F_SDI_5V_450A_200A_100A_10A_3:
		case F_SDI_5V_450A_200A_100A_10A_4:
		case F_SDI_5V_450A_200A_100A_10A_96CH:	//hun_160408
		case F_SDI_5V_450A_200A_100A_10A_96CH_2:	//hun_160808
		case F_SDI_5V_450A_200A_100A_10A_96CH_4:	//hun_170703
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 100kohm) = 1.494

			//RANGE1
			//shuntV = 0.222mOhm * 450A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 830Ohm = 60.5181
			//V1 = shuntV * G1 = 6051807uV
			//V2 = V1 * G2 = 9041400uV
			//450000000uA : 9041400uV = I1 : 10000000uV
			//I1 = 497710531.6uA
			//ratio = 497710531.6uA / 32768 = 15188.92003

			//RANGE2
			//shuntV = 0.222mOhm * 200A = 44.4mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / (830Ohm // 649ohm) = 136.6351755
			//V1 = shuntV * G1 = 6066602uV
			//V2 = V1 * G2 = 9063503uV
			//200000000uA : 9063503uV = I1 : 10000000uV
			//I1 = 220665232.9uA
			//ratio = 220665232.9uA / 32768 = 6734.168483

			//RANGE3
			//shuntV = 0.222mOhm * 100A = 22.222mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / (830Ohm // 237Ohm) = 268.9568909
			//V1 = shuntV * G1 = 5976760uV
			//V2 = V1 * G2 = 8929279uV
			//100000000uA : 8929279uV = I1 : 10000000uV
			//I1 = 112409063.4uA
			//ratio = 111991118.9uA / 32768 = 3417.697720

			//RANGE4
			//shuntV = 10mOhm * 10A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 830Ohm = 60.5181
			//V1 = shuntV * G1 = 6051807uV
			//V2 = V1 * G2 = 9041400uV
			//10000000uA : 9041400uV = I1 : 10000000uV
			//I1 = 11060234.03uA
			//ratio = 11060234.03uA / 32768 = 337.5315561
			if((range+1) == RANGE1) ratio = 15188.92003;
			else if((range+1) == RANGE2) ratio = 6734.168483;		//csk_141019
			else if((range+1) == RANGE3) ratio = 3417.697720;		//csk_141019
			else ratio = 337.5315561;
			break;
		case F_SDI_5V_450A_200A_100A_10A_96CH_3:	//hun_161201
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 100kohm) = 1.494

			//RANGE1
			//shuntV = 0.222mOhm * 450A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 820Ohm = 61.2349
			//V1 = shuntV * G1 = 6124390uV
			//V2 = V1 * G2 = 9149838uV
			//450000000uA : 9149838uV = I1 : 10000000uV
			//I1 = 491811986.2uA
			//ratio = 491811986.2uA / 32768 = 15008.91071

			//RANGE2
			//shuntV = 0.222mOhm * 200A = 44.4mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / (820Ohm // 649ohm) = 137.3610056
			//V1 = shuntV * G1 = 6098829uV
			//V2 = V1 * G2 = 9111651uV
			//200000000uA : 9111651uV = I1 : 10000000uV
			//I1 = 219499188.5uA
			//ratio = 219499188.5uA / 32768 = 6698.583633

			//RANGE3
			//shuntV = 0.222mOhm * 100A = 22.222mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / (830Ohm // 232Ohm) = 274.1749369
			//V1 = shuntV * G1 = 6092715uV
			//V2 = V1 * G2 = 9102516uV
			//100000000uA : 9102516uV = I1 : 10000000uV
			//I1 = 109859735.5uA
			//ratio = 109859735.5uA / 32768 = 3352.653061

			//RANGE4
			//shuntV = 10mOhm * 10A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 820Ohm = 61.2349
			//V1 = shuntV * G1 = 6124390uV
			//V2 = V1 * G2 = 9149838uV
			//10000000uA : 9149838uV = I1 : 10000000uV
			//I1 = 10929155.25uA
			//ratio = 10929155.25uA / 32768 = 333.5313492
			if((range+1) == RANGE1) ratio = 15008.91071;
			else if((range+1) == RANGE2) ratio = 6698.583633;
			else if((range+1) == RANGE3) ratio = 3352.653061;
			else ratio = 333.5313492;
			break;
		case C_LGC_5V_600A_10A:
			//AMP -> INA128UA
			//10000000uV / 32768 = 305.1757813uV
			//G1 = 1 + 50kohm / (1.25kohm // 3.3kohm) = 56.15151515
			//G2 = 1 + 50kohm / 100kohm = 1.5

			//RANGE1
			//V1 = 100000uV * G1 = 5615151.515uV
			//V2 = V1 * G2 = 8422727.273uV
			//600000000uA : 8422727.273uV = I1 : 10000000uV
			//I1 = 712358337.8uA
			//ratio = 712358337.8uA / 32768 = 21739.45123

			//RANGE2
			//V1 = 100000uV * G1 = 5615151.515uV
			//V2 = V1 * G2 = 8422727.273uV
			//100000000uA : 8422727.273uV = I1 : 10000000uV
			//I1 = 11872638.96.8uA
			//ratio = 11872638.96uA / 32768 = 362.324187
			if((range+1) == RANGE1) ratio = 21739.45123;
			else ratio = 362.324187;
			break;
		case C_SKI_20V_400A_200A_100A_16KW:		//csk_171023
		case C_SKI_20V_400A_200A_100A_16KW_2:	//csk_171023
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//Rg_2 = 100 kohm
			//G_i = 1 + 49.4Kohm / 100 kohm = 1.494
			//CH_gain
			//G_Ch1 = 100kohm / 1.69kohm = 59.17159763
			//G_Ch2 = (100k+1.69k) / 1.69k = 60.171597633136
			//G_Ch3 = (100k+2k) / 2k = 51
			
			//RANGE1
			//ITB400-S 400A:200mA = 400A:x -> x = 200mA
			//shuntV = 200mA * 0.5ohm = 100mV
			//Rg_r1 = open 
			//G_r1 = 1
			//shuntV1 = shuntV  * G_r1 = 100mV
			//shuntV2 = shuntV1 * G_Ch1 = 5917.1598mV
			//shuntV2 = shuntV1 * G_Ch3 = 5100mV
			//shuntV3 = shuntV2 * G_i = 8840.23669mV
			//shuntV3 = shuntV2 * G_i3 = 7619.4mV
			//400000mA : 8840.23669mV = l1 : 10000mV
			//400000mA : 7619.4mV = l1 : 10000mV
			//I1 = 452476.5729585mA
			//I1 = 524975.7198729mA
			//ratio = 452476572.959uA / 32768 = 13808.48916499
			//ratio = 524975719.873uA / 32768 = 16020.98754495
			
			//RANGE2
			//ITB400-S 400A:200mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 0.5ohm = 50mV
			//Rg_r2 = 49.4kohm
			//G_r2 = 1 + 49.4kohm / 49.4kohm = 2
			//shuntV1 = shuntV  * G_r2 = 100mV
			//shuntV2 = shuntV1 * G_Ch = 5917.1598mV
			//shuntV2 = shuntV1 * G_Ch3 = 5100mV
			//shuntV3 = shuntV2 * G_i = 8840.23669mV
			//shuntV3 = shuntV2 * G_i3 = 7619.4mV
			//200000mA : 8840.23669mV = l1 : 10000mV
			//200000mA : 7619.4mV = l1 : 10000mV
			//I1 = 226238.28647925mA
			//I1 = 262487.85993648mA
			//ratio = 226238286.479uA / 32768 = 6904.2445825
			//ratio = 262487859.936uA / 32768 = 8010.4937725
			
			//RANGEE3
			//ITB400-S 400A:200mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 0.5ohm = 25mV
			//Rg_r3 = 16.467kohm
			//G_r3 = 1 + 49.4kohm / 16.467kohm = 3.99994
			//shuntV1 = shuntV  * G_r3 = 99.9985mV
			//shuntV2 = shuntV1 * G_Ch = 5917.0699mV
			//shuntV2 = shuntV1 * G_Ch3 = 5099.9226mV
			//shuntV3 = shuntV2 * G_i = 8840.10247mV
			//shuntV3 = shuntV2 * G_i = 7619.28432mV
			//100000mA : 8840.10247mV = l1 : 10000mV
			//100000mA : 7619.28432mV = l1 : 10000mV
			//I1 = 113120.86062683mA
			//ratio = 113120860.627uA / 32768 = 3452.1747017
			//if((range+1) == RANGE1) ratio = 13808.48916499;
			if((range+1) == RANGE1) ratio = 16020.98754495;
			//else if((range+1) == RANGE2) ratio = 6904.2445825;
			else if((range+1) == RANGE2) ratio = 8010.4937725;
			//else ratio = 3452.1747017;
			else ratio = 4005.307696;
			break;
		case C_SKI_20V_500A_250A_100A_20KW:		//csk_171108
		case C_SKI_20V_500A_250A_100A_20KW_2:	//csk_171108
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//Rg_2 = 100 kohm
			//G_i = 1 + 49.4Kohm / 100 kohm = 1.494
			//CH_gain
			//G_Ch = (100k+3.3k) / 3.3k = 31.3030303
			
			//RANGE1
			//ITB600-S 600A:400mA = 500A:x -> x = 333.3333mA
			//shuntV = 333.3333mA * 0.5ohm = 166.6667mV
			//Rg_r1 = open 
			//G_r1 = 1
			//shuntV1 = shuntV  * G_r1 = 166.6667mV
			//shuntV2 = shuntV1 * G_Ch = 5217.1717mV
			//shuntV3 = shuntV2 * G_i = 7794.45455mV
			//500000mA : 7794.45455mV = l1 : 10000mV
			//I1 = 641481.705734mA
			//ratio = 641481705.734uA / 32768 = 19576.4680705
			
			//RANGE2
			//ITB400-S 600A:400mA = 250A:x -> x = 166.6667mA
			//shuntV = 166.6667mA * 0.5ohm = 83.3334mV
			//Rg_r2 = 49.4kohm
			//G_r2 = 1 + 49.4kohm / 49.4kohm = 2
			//shuntV1 = shuntV  * G_r2 = 166.667mV
			//shuntV2 = shuntV1 * G_Ch = 5217.1717mV
			//shuntV3 = shuntV2 * G_i = 7794.45455mV
			//250000mA : 7794.45455mV = l1 : 10000mV
			//I1 = 320740.852867mA
			//ratio = 320740852.867uA / 32768 = 9788.234041
			
			//RANGEE3
			//ITB400-S 600A:400mA = 100A:x -> x = 66.6667mA
			//shuntV = 66.6667mA * 0.5ohm = 33.3334mV
			//Rg_r3 = 12.35kohm
			//G_r3 = 1 + 49.4kohm / 12.35kohm = 5
			//shuntV1 = shuntV  * G_r2 = 166.667mV
			//shuntV2 = shuntV1 * G_Ch = 5217.1717mV
			//shuntV3 = shuntV2 * G_i = 7794.45455mV
			//100000mA : 7794.45455mV = l1 : 10000mV
			//I1 = 128296.341222mA
			//ratio = 128296341.222uA / 32768 = 3915.293616
			if((range+1) == RANGE1) ratio = 19576.4680705;
			else if((range+1) == RANGE2) ratio = 9788.234041;
			else ratio = 3915.293616;
			break;
		case C_SKI_20V_600A_300A_60A_24KW:		//jhk_180930
		case C_SKI_20V_600A_300A_60A_24KW_2:	//jhk_180930
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//Rg_2 = 100 kohm
			//G_i = 1 + 49.4Kohm / 100 kohm = 1.494
			//CH_gain
			//G_Ch = 100k:2k = 49.01960784
			
			//RANGE1
			//ITB600-S 600A:400mA = 600A:x -> x = 400mA
			//shuntV = 400mA * 0.25ohm = 100mV
			//Rg_r1 = open 
			//G_r1 = 1
			//shuntV1 = shuntV  * G_r1 = 100mV
			//shuntV2 = shuntV1 * G_Ch = 4901.960784mV
			//shuntV3 = shuntV2 * G_i = 7323.529411mV
			//600000mA : 7323.529411mV = l1 : 10000mV
			//I1 = 819277.11084337mA
			//ratio = 819277110.84337uA / 32768 = 25002.353162
			
			//RANGE2
			//ITB400-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 0.25ohm = 50mV
			//Rg_r2 = 49.4kohm
			//G_r2 = 1 + 49.4kohm / 49.4kohm = 2
			//shuntV1 = shuntV  * G_r2 = 100mV
			//shuntV2 = shuntV1 * G_Ch = 4901.960784mV
			//shuntV3 = shuntV2 * G_i = 7323.529411mV
			//300000mA : 7323.529411mV = l1 : 10000mV
			//I1 = 409638.55421686mA
			//ratio = 409638554.21686uA / 32768 = 12501.176581325
			
			//RANGEE3
			//ITB400-S 600A:400mA = 100A:x -> x = 66.67mA
			//shuntV = 66.67mA * 0.25ohm = 16.667mV
			//Rg_r3 = 9.88kohm
			//G_r3 = 1 + 49.4kohm / 9.88kohm = 6
			//shuntV1 = shuntV  * G_r3 = 100mV
			//shuntV2 = shuntV1 * G_Ch = 4901.960784mV
			//shuntV3 = shuntV2 * G_i = 7322.1954354961mV
			//100000mA : 7322.1954354961mV = l1 : 10000mV
			//I1 = 136571.06107169mA
			//ratio = 136571061.0716uA / 32768 = 4167.8180258695
			if((range+1) == RANGE1) ratio = 25002.353162;
			else if((range+1) == RANGE2) ratio = 12501.176581325;
			else ratio = 4167.8180258695;
			break;
		/*
		case C_SKI_20V_600A_300A_60A_24KW:		//jhk_180930
		case C_SKI_20V_600A_300A_60A_24KW_2:	//jhk_180930
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//Rg_2 = 100 kohm
			//G_i = 1 + 49.4Kohm / 100 kohm = 1.494
			//CH_gain
			//G_Ch = (100k+1.65k) / 1.65k = 61.6060606
			
			//RANGE1
			//ITB600-S 600A:400mA = 600A:x -> x = 400mA
			//shuntV = 400mA * 0.25ohm = 100mV
			//Rg_r1 = open 
			//G_r1 = 1
			//shuntV1 = shuntV  * G_r1 = 100mV
			//shuntV2 = shuntV1 * G_Ch = 6160.60606mV
			//shuntV3 = shuntV2 * G_i = 9203.9mV
			//600000mA : 9203.9mV = l1 : 10000mV
			//I1 = 651897.5651625941mA
			//ratio = 651897565.1625941uA / 32768 = 19894.3348743
			
			//RANGE2
			//ITB400-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 0.25ohm = 50mV
			//Rg_r2 = 49.4kohm
			//G_r2 = 1 + 49.4kohm / 49.4kohm = 2
			//shuntV1 = shuntV  * G_r2 = 100mV
			//shuntV2 = shuntV1 * G_Ch = 6160.60606mV
			//shuntV3 = shuntV2 * G_i = 9203.9mV
			//300000mA : 9203.9mV = l1 : 10000mV
			//I1 = 325948.7825812971mA
			//ratio = 325948782.5812971uA / 32768 = 9947.1674371
			
			//RANGEE3
			//ITB400-S 600A:400mA = 100A:x -> x = 66.666mA
			//shuntV = 66.666mA * 0.25ohm = 16.666mV
			//Rg_r3 = 10kohm
			//G_r3 = 1 + 49.4kohm / 10kohm = 5.94
			//shuntV1 = shuntV  * G_r3 = 98.99604mV
			//shuntV2 = shuntV1 * G_Ch = 6098.75603940002mV
			//shuntV3 = shuntV2 * G_i = 9111.541522863636mV
			//100000mA : 9111.541522863636mV = l1 : 10000mV
			//I1 = 109750.9128933556mA
			//ratio = 109750912.8933556uA / 32768 = 3349.332058513051
			if((range+1) == RANGE1) ratio = 19894.3348743;
			else if((range+1) == RANGE2) ratio = 9947.1674371;
			else ratio = 3349.332058513051;
			break;
		*/
		case C_TAESUNG_20V_600A_60A:
			//AMP -> AD620A
			//10000000uV / 32768 = 305.1757813uV
			//G1 = 1 + 49.4kohm / (830ohm) = 60.51807229
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//V1 = 100000uV * G1 = 6051807.229uV
			//V2 = V1 * G2 = 9041400uV
			//600000000uA : 9041400uV = I1 : 10000000uV
			//I1 = 663614042.1uA
			//ratio = 663614042.1uA / 32768 = 20251.89337

			//RANGE2
			//V1 = 100000uV * G1 = 6051807.229uV
			//V2 = V1 * G2 = 9041400uV
			//60000000uA : 9041400uV = I1 : 10000000uV
			//I1 = 66361404.21uA
			//ratio = 66361404.21uA / 32768 = 2025.189337
			if((range+1) == RANGE1) ratio = 20251.89337;
			else ratio = 2025.189337;
			break;
		case C_KATECH_20V_1000A_500A_100A:  //csk_120309
			//10000000uV / 32768 = 305.17578125uV
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			//G2_2 = 1 + 49.4kohm / 840ohm = 59.8095
			//G2_3 = G2 * G2_2 = 89.35543

			//RANGE1
			//shuntR = 0.1mohm (IRH80F-40A 0.1mohm)
			//G1 = 1 + 49.4kohm / gainR = 1
			//shuntV = 1000A * 0.1mohm = 100mV
			//V1 = shuntV * G1 = 100000uV
			//V2 = V1 * G2_3 = 8935543.0uV
			//1000000000uA : 8935543.0uV = I1 : 10000000uV
			//I1 = 1119126168.38uA
			//ratio = 1119126168.38uA / 32768 = 34153.02

			//RANGE2
			//shuntR = 0.1mohm (IRH80F-40A 0.1mohm)
			//gainR = 49.4kohm
			//G1 = 1 + 49.4kohm / gainR = 2
			//shuntV = 500A * 0.1mOhm = 50mV
			//V1 = shuntV * G1 = 100000uV
			//V2 = V1 * G2_3 = 8935543.0uV
			//500000000uA : 8935543.0uV = I1 : 10000000uV
			//I1 = 559563084.19uA
			//ratio = 559563084.19uA / 32768 = 17076.51

			//RANGE3
			//shuntR = 0.1mohm (IRH80F-40A 0.1mohm)
			//gainR = 5.49kohm
			//G1 = 1 + 49.4kohm / gainR = 9.998179 
			//shuntV = 100A * 0.1mOhm = 10mV
			//V1 = shuntV * G1 = 99981.79uV
			//V2 = V1 * G2_3 = 8933915.838uV
			//100000000uA : 8933915.838uV = I1 : 10000000uV
			//I1 = 111932999.84uA
			//ratio = 111932999.84uA / 32768 = 3415.924
			if((range+1) == RANGE1) ratio = 34153.02;
			else if((range+1) == RANGE2) ratio = 17076.51;
			else ratio = 3415.924;
			break;
		case C_LGC_50V_40A_10A_4A: //csk_120206
		case C_LGC_50V_40A_10A_4A_2:
		case C_LGC_50V_40A_10A_4A_3:
		case C_LGC_50V_40A_10A_4A_4:
			//10000000uV / 32768 = 305.17578125uV
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//shuntR = 2.5mohm (IRH80F-40A 2.5mohm)
			//gainR = 800ohm
			//G1 = 1 + 49.4kohm / gainR = 62.75
			//shuntV = 40A * 2.5mohm = 100mV
			//V1 = shuntV * G1 = 6275000uV
			//V2 = V1 * G2 = 9374850uV
			//40000000uA : 9374850uV = I1 : 10000000uV
			//I1 = 42667349uA
			//ratio = 42667349uA / 32768 = 1302.1042

			//RANGE2
			//shuntR = 2.5mohm (IRH80F-40A 2.5mohm)
			//gainR = 800ohm // 270ohm = 201.8692ohm
			//G1 = 1 + 49.4kohm / gainR = 245.712963
			//shuntV = 10A * 2.5mOhm = 25mV
			//V1 = shuntV * G1 = 6142824.07uV
			//V2 = V1 * G2 = 9177379.167uV
			//10000000uA : 9177379167uV = I1 : 10000000uV
			//I1 = 10896357uA
			//ratio = 10890357uA / 32768 = 332.53043

			//RANGE3
			//shuntR = 2.5mohm (IRH80F-40A 2.5mohm)
			//gainR = 800ohm // 90ohm = 80.8989ohm
			//G1 = 1 + 49.4kohm / gainR = 611.638889
			//shuntV = 4A * 2.5mOhm = 10mV
			//V1 = shuntV * G1 = 6116388.89uV
			//V2 = V1 * G2 = 9137885uV
			//4000000uA : 9137885uV = I1 : 10000000uV
			//I1 = 4377380.543uA
			//ratio = 4377380.543uA / 32768 = 133.5870527
			if((range+1) == RANGE1) ratio = 1302.1042;
			else if((range+1) == RANGE2) ratio = 332.53043;
			else ratio = 133.5870527;
			break;
		case C_LGC_50V_120A_10A:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> INA128UA
			//G1 = 1 + 50kohm / 470ohm = 107.3829787
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//shuntV = 120A / 300A * 100mV = 40mV
			//V1 = 40000uV * G1 = 4295319.148uV
			//V2 = V1 * G2 = 6417206.807uV
			//120000000uA : 6417206.807uV = I1 : 10000000uV
			//I1 = 186997246.0uA
			//ratio = 186997246.0uA / 32768 = 5706.703064

			//RANGE2
			//shuntV = 10A / 5mOhm = 50mV
			//V1 = 50000uV * G1 = 5369148.935uV
			//V2 = V1 * G2 = 8021508.509uV
			//10000000uA : 8021508.509uV = I1 : 10000000uV
			//I1 = 12466483.07uA
			//ratio = 12466483.07uA / 32768 = 380.446871
			if((range+1) == RANGE1) ratio = 5706.703064;
			else ratio = 380.446871;
			break;
		case C_JNU_50V_200A_100A_10KW:	//jhk_161202
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//200000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 223528.6224mA
			//ratio = 223528622.4uA / 32768 = 6821.552209

			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//100000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 111764.31142mA
			//ratio = 111764311.42uA / 32768 = 3410.776105349
			if((range+1) == RANGE1) ratio = 6821.552209;
			else ratio = 3410.776105349;
			break;
		case C_LGC_50V_200A_10A:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> INA128UA
			//G1 = 1 + 50kohm / 560ohm = 90.28571429
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//shuntV = 200A / 300A * 100mV = 66.667mV
			//V1 = shuntV * G1 = 6019047.619uV
			//V2 = V1 * G2 = 8992457.143uV
			//200000000uA : 8992457.143uV = I1 : 10000000uV
			//I1 = 222408621.8uA
			//ratio = 222408621.8uA / 32768 = 6787.372491

			//RANGE2
			//shuntV = 10A * 5mOhm = 50mV
			//V1 = shuntV * G1 = 4514285.715uV
			//V2 = V1 * G2 = 6744342.858uV
			//10000000uA : 6744342.858uV = I1 : 10000000uV
			//I1 = 14827241.45uA
			//ratio = 14827241.45uA / 32768 = 452.4914993
			if((range+1) == RANGE1) ratio = 6787.372491;
			else ratio = 452.4914993;
			break;
		case C_ENERTECH_55V_100A_10A: //kjg_w
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 0.98kohm = 51.408
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//shuntV = 100A * 1mOhm = 100mV
			//V1 = shuntV * G1 = 5140816uV
			//V2 = V1 * G2 = 7680379uV
			//100000000uA : 7680379uV = I1 : 10000000uV
			//I1 = 130201908.0uA
			//ratio = 130201908.0uA / 32768 = 3973.568163
			
			//RANGE2
			//shuntV = 10A * 10mOhm = 100mV
			//V1 = shuntV * G1 = 5140816uV
			//V2 = V1 * G2 = 7680379uV
			//10000000uA : 7680379uV = I1 : 10000000uV
			//I1 = 13020190.8uA
			//ratio = 13020190.8uA / 32768 = 397.3568163
			if((range+1) == RANGE1) ratio = 3973.568163;
			else ratio = 397.3568163;
			break;
		case C_LGC_60V_30A_15A_3A_7KW:	//ktg_181113
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//Rg_2 = 100 kohm
			//G_i = 1 + 49.4Kohm / 100 kohm = 1.494
			//CH_gain
			//G_Ch = 100k:2k = 49.01960784
			
			//RANGE1
			//IT60-S 60A:100mA = 30A:x -> x = 50mA
			//shuntV = 50mA * 2ohm = 100mV
			//Rg_r1 = open
			//G_r1 = 1
			//shuntV1 = shuntV  * G_r1 = 100mV
			//shuntV2 = shuntV1 * G_Ch = 4901.960784mV
			//shuntV3 = shuntV2 * G_i = 7323.529411mV
			//30000mA : 7323.529411mV = l1 : 10000mV
			//I1 = 40963.85542168671mA
			//ratio = 40963855.42168671uA / 32768 = 1250.1176581325
			
			//RANGE2
			//IT60-S 60A:100mA = 15A:x -> x = 25mA
			//shuntV = 25mA * 2ohm = 50mV
			//Rg_r1 = 49.4kohm
			//G_r1 = 2
			//shuntV1 = shuntV  * G_r1 = 100mV
			//shuntV2 = shuntV1 * G_Ch = 4901.960784mV
			//shuntV3 = shuntV2 * G_i = 7323.529411mV
			//15000mA : 7323.529411mV = l1 : 10000mV
			//I1 = 20481.92771084335mA
			//ratio = 20481927.71084335uA / 32768 = 625.0588290662645
			
			//RANGEE3
			//IT60-S 60A:100mA = 3A:x -> x = 5mA
			//shuntV = 50mA * 2ohm = 10mV
			//Rg_r1 = 5.49kohm
			//G_r1 = 9.998178506375228
			//shuntV1 = shuntV  * G_r1 = 99.98178506375228mV
			//shuntV2 = shuntV1 * G_Ch = 4901.067895281979mV
			//shuntV3 = shuntV2 * G_i = 7322.195435551276mV
			//3000mA : 7322.195435551276mV = l1 : 10000mV
			//I1 = 4097.131832119877mA
			//ratio = 4097131.832119877uA / 32768 = 125.0345407751427
			
			if((range+1) == RANGE1) ratio = 1250.1176581325;
			else if((range+1) == RANGE2) ratio = 625.0588290662645;
			else ratio = 125.0345407751427;
			break;
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//shuntR = 2mohm (IRH300F-50A 2mohm)
			//gainR = 900ohm // 12kohm = 837.2093ohm
			//G1 = 1 + 49.4kohm / gainR = 60.005556
			//shuntV = 50A * 2mohm = 100mV
			//V1 = shuntV * G1 = 6000555.6uV
			//V2 = V1 * G2 = 8964830.066uV
			//50000000uA : 8964830.066uV = I1 : 10000000uV
			//I1 = 55773505.61uA
			//ratio = 55773505.61uA / 32768 = 1702.072315

			//RANGE2
			//shuntR = 2mohm (IRH300F-50A 2mohm)
			//gainR = 837.2093ohm // 91ohm = 82.0785ohm
			//G1 = 1 + 49.4kohm / gainR = 602.862697
			//shuntV = 5A * 2mOhm = 10mV
			//V1 = shuntV * G1 = 6028626.97uV
			//V2 = V1 * G2 = 9006768.693uV
			//5000000uA : 9006768.693uV = I1 : 10000000uV
			//I1 = 5551380.49uA
			//ratio = 5551380.49uA / 32768 = 169.4146878
			if((range+1) == RANGE1) ratio = 1702.072315;
			else ratio = 169.4146878;
			break;
/*		case C_SDI_70V_250A_25A_18KW:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//shuntR = 0.4mohm (IRH300F-250A 0.4mohm)
			//gainR = 900ohm // 12kohm = 837.2093ohm
			//G1 = 1 + 49.4kohm / gainR = 60.005556
			//shuntV = 250A * 0.4mohm = 100mV
			//V1 = shuntV * G1 = 6000555.6uV
			//V2 = V1 * G2 = 8964830.066uV
			//250000000uA : 8964830.066uV = I1 : 10000000uV
			//I1 = 278867528.1uA
			//ratio = 278867528.1uA / 32768 = 8510.361575

			//RANGE2
			//shuntR = 0.4mohm (IRH300F-250A 0.4mohm)
			//gainR = 837.2093ohm // 91ohm = 82.0785ohm
			//G1 = 1 + 49.4kohm / gainR = 602.862697
			//shuntV = 25A * 0.4mOhm = 10mV
			//V1 = shuntV * G1 = 6028626.97uV
			//V2 = V1 * G2 = 9006768.693uV
			//25000000uA : 9006768.693uV = I1 : 10000000uV
			//I1 = 27756902.45uA
			//ratio = 27756902.45uA / 32768 = 847.073439
			if((range+1) == RANGE1) ratio = 8510.361575;
			else ratio = 847.073439;
			break;*/
		case C_SDI_70V_250A_25A_18KW:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB-300S 300A/150mA
			//BurnR = 50ohm // 5ohm = 4.545454
			//GainR = 6.8kohm
			//G = 1 + 49.4kohm / 6.8kohm = 8.264706
			//V1 = (250A / 300A * 150mA) * BurnR * G = 4695855.7uV
			//250000000uA * x = V1 -> x = 0.018783423
			//V2 = V1 * G2 = 7015608.416uV
			//250000000uA : 7015608.416uV = I1 : 10000000uV
			//I1 = 356348281.1uA
			//ratio = 356348281.1uA / 32768 = 10874.88651

			//RANGE2
			//BurnR = 50ohm
			//GainR = 6.8kohm
			//G = 1 + 49.4kohm / 6.8kohm = 8.264706
			//V1 = (25A / 300A * 150mA) * BurnR * G = 5165441.3uV
			//25000000uA * x = V1 -> x = 0.20661765
			//V2 = V1 * G2 = 7717169.302uV
			//25000000uA : 7717169.302uV = I1 : 10000000uV
			//I1 = 32395298.1uA
			//ratio = 3239598.1uA / 32768 = 988.6260406
			if((range+1) == RANGE1) ratio = 10874.88651;
			else ratio = 988.6260406;
			break;
		case C_NEWPOWER_75V_150A_75A_45KW:	//ljh_201105
			//10000000uV / 32768 = 305.17578125uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN600-S 150A:100mA = 150A:x -> x = 100mA
			//shuntV = 100mA * 10ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 5940mV * 1.494 = 8874.36mV
			//150000mA : 8874.36mV = I1 : 10000mV
			//I1 = 169026.273443944mA
			//ratio = 169026273.443944 / 32768 = 5158.272505003
			
			//RAGNE2
			//ITN600-S 150A:100mA = 75A:x -> x = 50mA
			//shuntV = 50mA * 20ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 5940mV * 1.494 = 8874.36mV
			//75000mA : 8874.36mV = I1 : 10000mV
			//I1 = 84513.136721972
			//ratio = 84513136.721972 / 32768 = 2579.1362525016
			if((range+1) == RANGE1) ratio = 5158.272505003;
			else ratio = 2579.1362525016;
			break;
		case C_SDI_80V_50A_25A_10A:		//csk_130417_s
		case C_SDI_80V_50A_25A_10A_2:
		case C_SDI_80V_50A_25A_10A_3:
		case C_SDI_80V_50A_25A_10A_4:	//csk_130417_e
			//10000000uV / 32768 = 305.17578125uV
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//shuntR = 2mohm (IRH300F-50A 2mohm)
			//shuntV = 50A * 2mohm = 100mV
			//gainR1 = 820ohm
			//G1 = 1 + 49.4kohm / gainR1 = 61.2439
			//V1 = shuntV * G1 = 6124390uV
			//V2 = V1 * G2 = 9149839uV
			//50000000uA : 9149839uV = I1 : 10000000uV
			//I1 = 54645770uA
			//ratio = 54645770uA / 32768 = 1667.65656

			//RANGE2
			//shuntR = 2mohm (IRH300F-50A 2mohm)
			//shuntV = 25A * 2mOhm = 50mV
			//gainR1 = 820ohm // 820ohm = 410
			//G1 = 1 + 49.4kohm / gainR1 = 121.4878
			//V1 = shuntV * G1 = 6074390uV
			//V2 = V1 * G2 = 9075139uV
			//25000000uA : 9075139uV = I1 : 10000000uV
			//I1 = 27547787uA
			//ratio = 27547787uA / 32768 = 840.69175

			//RANGE3
			//shuntR = 2mohm (IRH300F-50A 2mohm)
			//shuntV = 10A * 2mOhm = 20mV
			//gainR1 = 820ohm // 200ohm = 160.7843137
			//G1 = 1 + 49.4kohm / gainR1 = 308.243902
			//V1 = shuntV * G1 = 6164878uV
			//V2 = V1 * G2 = 9210328uV
			//10000000uA : 9210328uV = I1 : 10000000uV
			//I1 = 10857376uA
			//ratio = 10857376uA / 32768 = 331.3408466
			
			if((range+1) == RANGE1) ratio = 1667.65656;
			else if((range+1) == RANGE2) ratio = 840.69175;
			else ratio = 331.3408466;
			break;
		case C_KATECH_100V_30A_15A_5A: //csk_131129
			//10000000uV / 32768 = 305.17578125uV
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//shuntR = 3.333mohm (IRH80F-30A 3.333mohm)
			//shuntV = 30A * 3.333mohm = 99.99mV
			//gainR1 = 830
			//G1 = 1 + 49.4kohm / gainR1 = 60.518072289
			//V1 = shuntV * G1 = 6051202 uV
			//V2 = V1 * G2 = 9040495.86uV
			//30000000uA : 9040495.86uV = I1 : 10000000uV
			//I1 = 33184021uA
			//ratio = 33184021uA / 32768 = 1012.69594

			//RANGE2
			//shuntR = 3.333mohm (IRH80F-30A 3.333mohm)
			//shuntV = 15A * 3.333mOhm = 49.995mV
			//gainR1 = 830ohm // 830ohm = 415
			//G1 = 1 + 49.4kohm / gainR1 = 120.0361
			//V1 = shuntV * G1 = 6001207uV
			//V2 = V1 * G2 = 8965803uV
			//15000000uA : 8965803uV = I1 : 10000000uV
			//I1 = 16730235uA
			//ratio = 16730235uA / 32768 = 510.566265

			//RANGE3
			//shuntR = 3.333mohm (IRH80F-30A 3.333mohm)
			//shuntV = 5A * 3.333mOhm = 16.665mV
			//gainR1 = 830ohm // 168ohm = 139.7194389
			//G1 = 1 + 49.4kohm / gainR1 = 354.565691
			//V1 = shuntV * G1 = 5908837uV
			//V2 = V1 * G2 = 8827802uV
			//5000000uA : 8802905uV = I1 : 10000000uV
			//I1 = 5663924uA
			//ratio = 5679942uA / 32768 = 172.84922794
			
			if((range+1) == RANGE1) ratio = 1012.69594;
			else if((range+1) == RANGE2) ratio = 510.566265;
			else ratio = 172.84922794;
			break;
		case C_SAEHAN_72V_200A_20A:
		case C_VK_100V_200A_20A: //kjg_w
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> INA128UA
			//G1 = 1 + 50kohm / 2kohm = 26.0
			//AMP -> INA128UA
			//G2 = 1 + 50kohm / 100kohm = 1.5

			//RANGE1
			//shuntV = 200A * 1mOhm = 200mV
			//V1 = shuntV * G1 = 5200000uV
			//V2 = V1 * G2 = 7800000uV
			//200000000uA : 7800000uV = I1 : 10000000uV
			//I1 = 256410256.4uA
			//ratio = 256410256.4uA / 32768 = 7825.020032

			//RANGE2
			//shuntV = 20A * 10mOhm = 200mV
			//V1 = shuntV * G1 = 5200000uV
			//V2 = V1 * G2 = 7800000uV
			//20000000uA : 7800000uV = I1 : 10000000uV
			//I1 = 25641025.64uA
			//ratio = 25641025.64uA / 32768 = 782.5020032
			if((range+1) == RANGE1) ratio = 7825.020032;
			else ratio = 782.5020032;
			break;
		case C_EIG_100V_200A_20A_2:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> INA121
			//G1 = 1 + 50kohm / 1.703kohm = 30.35995302
			//AMP -> INA121
			//G2 = 1 + 50kohm / 100kohm = 1.5

			//RANGE1
			//shuntV = 200A * 1mOhm = 200mV
			//V1 = shuntV * G1 = 6071990.604uV
			//V2 = V1 * G2 = 9107985.906uV
			//200000000uA : 9107985.906uV = I1 : 10000000uV
			//I1 = 219587515.9uA
			//ratio = 219587515.9uA / 32768 = 6701.279172

			//RANGE2
			//shuntV = 20A * 10mOhm = 200mV
			//V1 = shuntV * G1 = 6071990.604uV
			//V2 = V1 * G2 = 9107985.906uV
			//20000000uA : 9107985.906uV = I1 : 10000000uV
			//I1 = 21958751.59uA
			//ratio = 21958751.59uA / 32768 = 670.1279172
			if((range+1) == RANGE1) ratio = 6701.279172;
			else ratio = 670.1279172;
			break;

		//Current Transducer used
		/*case C_LGC_40V_300A_100A_48KW: //jhk_181018
		case C_LGC_40V_300A_100A_48KW_2: //jhk_181018
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 5ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//300000mA : 8874.36mV = I1 : 10000mV
			//I1 = 338052.54688mA
			//ratio = 338052546.88uA / 32768 = 10316.545010006355
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 15ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//100000mA : 8874.36mV = I1 : 10000mV
			//I1 = 112684.18229596mA
			//ratio = 112684182.29596uA / 32768 = 3438.848336668785
			
			if((range+1) == RANGE1) ratio = 10316.545010006355;
			else ratio = 3438.848336668785;
			break;*/
		/*case C_LGC_40V_500A_250A_100A_80KW:	//jhk_181014
		case C_LGC_40V_500A_250A_100A_80KW_2: //jhk_181014
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 500A:x -> x = 333.333mA
			//shuntV = 333.333mA * 2.5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//500000mA : 8932.875mV = I1 : 10000mV
			//I1 = 559730.21003876mA
			//ratio = 559730210.03uA / 32768 = 17081.610413780558
			
			//RANGE2
			//ITN600-S 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//250000mA : 8932.875mV = I1 : 10000mV
			//I1 = 279865.105mA
			//ratio = 279865105.019uA / 32768 = 8540.805206890279
			
			//RANGE3
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 12.5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.16667mV
			//V2 = V1 * G2 = 8932.875mV
			//100000mA : 8932.875mV = I1 : 10000mV
			//I1 = 111946.042mA
			//ratio = 111946042.007uA / 32768 = 3416.322082756112
			
			if((range+1) == RANGE1) ratio = 17081.610413780558;
			else if ((range+1) == RANGE2) ratio = 8540.805206890279;
			else ratio = 3416.322082756112;
			break;*/
		case C_LGC_50V_300A_10A_20KW:	//jhk - 110719
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 8983.333335mV
			//V2 = V1 * G2 = 13421.100002mV
			//300000mA : 13421.100002mV = I1 : 10000mV
			//I1 = 223528.6228mA
			//ratio = 223528622.8uA / 32768 = 6821.552209

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 150ohm = 750mV
			//V1 = 750mV * G1 = 8983.333335mV
			//V2 = V1 * G2 = 13421.100002mV
			//10000mA : 13421.100002mV = I1 : 10000mV
			//I1 = 7450.95409mA
			//ratio = 7450954.09uA / 32768 = 227.385074
			if((range+1) == RANGE1) ratio = 6821.552209;
			else ratio = 227.385074;
			break;
		case C_LGC_50V_500A_10A_35KW:	//kjh_111002
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233333333
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.83333333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.5384136mA
			//ratio = 289968538.4136uA / 32768 = 8849.137525

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 125ohm = 625mV
			//V1 = 625mV * G1 = 5770.8333333mV
			//V2 = V1 * G2 = 8621.625mV
			//10000mA : 8621.625mV = I1 : 10000mV
			//I1 = 11598.741537mA
			//ratio = 11598741.537uA / 32768 = 353.965501
			if((range+1) == RANGE1) ratio = 8849.137525;
			else ratio = 353.965501;
			break;
		case C_SEBANG_50V_300A_100A_50A_15KW:	//jhkw_130912
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.0285714mV
			//300000mA : 9028.0285714mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.9442438785876

			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.0285714mV
			//100000mA : 9028.0285714mV = I1 : 10000mV
			//I1 = 110766.1536611mA
			//ratio = 110766153.6611uA / 32768 = 3380.314747959529
			
			//RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.0285714mV
			//50000mA : 9028.0285714mV = I1 : 10000mV
			//I1 = 55383.0768305689mA
			//ratio = 55383076.8305689uA / 32768 = 1690.15737397976
			
			if((range+1) == RANGE1) ratio = 10140.9442438785876;
			//else if ((range+1) == RANGE2) ratio = 3380.314747959529;
			else ratio = 1690.15737397976;
			break;
		case C_LGC_50V_500A_10A_42KW:
		case C_LGC_50V_500A_10A_42KW_2:
		case C_LGC_50V_500A_10A_42KW_3:
		case C_LGC_50V_500A_10A_42KW_4:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//250000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 289979.007mA
			//ratio = 289979007.0uA / 32768 = 8849.457001

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 125ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//10000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 11599.16028mA
			//ratio = 11599160.28uA / 32768 = 353.97828
			if((range+1) == RANGE1) ratio = 8849.457001;
			else ratio = 353.97828;
			break;
		case C_SKI_50V_500A_250A_25KW:	//ktg_190715
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 500A:x -> x = 333.33mA
			//shuntV = 333.33mA * 2.5ohm = 833.33mV
			//V1 = 833.33mV * G1 = 5979.16667mV
			//V2 = V1 * G2 = 8932.875mV
			//500000mA : 8932.875mV = I1 : 10000mV
			//I1 = 559730.21mA
			//ratio = 559730210.0uA / 32768 = 17081.610413780558
			
			//RANGE2
			//ITN-600 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 5ohm = 833.33mV
			//V1 = 833.33mV * G1 = 5979.1666667mV
			//V2 = V1 * G2 = 8932.875mV
			//250000mA : 8932.875mV = I1 : 10000mV
			//I1 = 279865.10501938mA
			//ratio = 279865105.01938uA / 32768 = 8540.80520689
			
			
			if((range+1) == RANGE1) ratio = 17081.610413780558;
			else ratio = 8540.80520689;
			break;
		case C_SKI_50V_500A_400A_200A_100A_100KW:	//jhk_180119
		case C_SKI_50V_500A_400A_200A_100A_100KW_2:	//jhk_180119
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 500A:x -> x = 333.33mA
			//shuntV = 333.33mA * 2.5ohm = 833.33mV
			//V1 = 833.33mV * G1 = 5979.16667mV
			//V2 = V1 * G2 = 8932.875mV
			//500000mA : 8932.875mV = I1 : 10000mV
			//I1 = 559730.21mA
			//ratio = 559730210.0uA / 32768 = 17081.610413780558
			
			//RANGE2
			//ITN-600 600A:400mA = 400A:x -> x = 266.667mA
			//shuntV = 266.667mA * 3.125ohm = 833.33mV
			//V1 = 833.33mV * G1 = 5979.1666667mV
			//V2 = V1 * G2 = 8932.875mV
			//400000mA : 8932.875mV = I1 : 10000mV
			//I1 = 447784.168mA
			//ratio = 447784168.03uA / 32768 = 13665.288331024446
			
			//RANGE3
			//ITN-600 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 6.25ohm = 833.33mV
			//V1 = 833.33mV * G1 = 5979.16667mV
			//V2 = V1 * G2 = 8932.875mV
			//200000mA : 8932.875mV = I1 : 10000mV
			//I1 = 223892.0840155mA
			//ratio = 223892084.0155uA / 32768 = 6832.644165512223
			
			//RANGE4
			//ITN-600 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 12.5ohm = 833.33mV
			//V1 = 833.33mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//100000mA : 8932.875mV = I1 : 10000mV
			//I1 = 111946.042mA
			//ratio = 111946042uA / 32768 = 3416.322082756112
			
			if((range+1) == RANGE1) ratio = 17081.610413780558;
			else if ((range+1) == RANGE2) ratio = 13665.288331024446;
			else if ((range+1) == RANGE3) ratio = 6832.644165512223;
			else ratio = 3416.322082756112;
			break;
		case C_SKI_50V_1000A_500A_300A_100KW:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_2:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_3:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_4:	//ktg_190711
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.17kohm = 9.00648298
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-1000 600A:400mA = 1000A:x -> x = 666.667mA
			//shuntV = 666.667mA * 1ohm = 666.667mV
			//V1 = 666.667mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.45705mV
			//1000000mA : 8970.45705mV = I1 : 10000mV
			//I1 = 1114770.40066mA
			//ratio = 1114770400.66uA / 32768 = 34020.09279357
			
			//RANGE2
			//ITN-1000 600A:400mA = 500A:x -> x = 333.333mA
			//shuntV = 333.333mA * 2ohm = 666.667mV
			//V1 = 666.667mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.45705mV
			//500000mA : 8970.45705mV = I1 : 10000mV
			//I1 = 557385.20033mA
			//ratio = 557385200.33uA / 32768 = 17010.04639678
			
			//RANGE3
			//ITN-1000 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 3.33ohm = 666.667mV
			//V1 = 666.667mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.45705mV
			//300000mA : 8970.45705mV = I1 : 10000mV
			//I1 = 334431.1201979mA
			//ratio = 334431120.1979uA / 32768 = 10206.02783807
			
			if((range+1) == RANGE1) ratio = 34020.09279357;
			else if ((range+1) == RANGE2) ratio = 17010.04639678;
			else ratio = 10206.02783807;
			break;
		case C_HLGP_60V_200A_10A_12KW:
		case C_HLGP_60V_200A_10A_24KW:
		case C_HLGP_60V_200A_10A_24KW_2:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//200000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 223528.6224mA
			//ratio = 223528622.4uA / 32768 = 6821.552209

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 100ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//10000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 11176.43114mA
			//ratio = 11176431.14uA / 32768 = 341.0776105
			if((range+1) == RANGE1) ratio = 6821.552209;
			else ratio = 341.0776105;
			break;
		case C_SNU_60V_200A_100A_50A_1CH_12KW:  //jhkw_130221
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.8888889mV
			//V2 = V1 * G2 = 8947.400002mV
			//200000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 223528.62284mA
			//ratio = 223528622.84uA / 32768 = 6821.552210698
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//100000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 111764.3114mA
			//ratio = 111764311.4uA / 32768 = 3410.776105
			
			//RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 20ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//50000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 55882.15571mA
			//ratio = 55882155.71uA / 32768 = 1705.38805267
			
			if((range+1) == RANGE1) ratio = 6821.552210698;
			else if((range+1) == RANGE2) ratio = 3410.776105;
			else ratio = 1705.38805267;
			break;
		case C_LGC_60V_250A_10A_30KW:
		case C_LGC_60V_250A_10A_30KW_2:
		case C_LGC_60V_250A_10A_30KW_3:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//250000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 289979.007mA
			//ratio = 289979007.0uA / 32768 = 8849.457001

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 125ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//10000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 11599.16028mA
			//ratio = 11599160.28uA / 32768 = 353.97828
			if((range+1) == RANGE1) ratio = 8849.457001;
			else ratio = 353.97828;
			break;
		case C_SK_60V_300A_10A:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.0kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.857143mV
			//V2 = V1 * G2 = 9028.028572mV
			//300000mA : 9028.028572mV = I1 : 10000mV
			//I1 = 332298.461mA
			//ratio = 332298461uA / 32768 = 10140.94424

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 150ohm = 750mV
			//V1 = 750mV * G1 = 6042.857143mV
			//V2 = V1 * G2 = 9028.028572mV
			//10000mA : 9028.028572mV = I1 : 10000mV
			//I1 = 11076.61537mA
			//ratio = 11076615.37uA / 32768 = 338.0314749
			if((range+1) == RANGE1) ratio = 10140.94424;
			else ratio = 338.0314749;
			break;
		/*case C_LGC_60V_300A_30A_36KW:	//jhk_150406
		case C_LGC_60V_300A_30A_36KW_2:	//jhk_150406
		case C_LGC_60V_300A_30A_36KW_3:	//jhk_150406
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.0kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.857143mV
			//V2 = V1 * G2 = 9028.028572mV
			//300000mA : 9028.028572mV = I1 : 10000mV
			//I1 = 332298.461mA
			//ratio = 332298461uA / 32768 = 10140.94424

			//RANGE2
			//ITB300-S 300A:150mA = 30A:x -> x = 15mA
			//shuntV = 15mA * 50ohm = 750mV
			//V1 = 750mV * G1 = 6042.857143mV
			//V2 = V1 * G2 = 9028.028572mV
			//30000mA : 9028.028572mV = I1 : 10000mV
			//I1 = 33229.84609834mA
			//ratio = 33229846.09834uA / 32768 = 1014.094424387859
			if((range+1) == RANGE1) ratio = 10140.94424;
			else ratio = 1014.094424387859;
			break;*/	//shh_231027
		case C_TOPBATT_60V_300A_100A_36KW:	//jhk_150616
		case C_HYUNDAI_60V_300A_100A_36KW:	//jhk_160825
		case C_HYUNDAI_60V_300A_100A_36KW_2:	//jhk_161111
		case C_HYUNDAI_60V_300A_100A_36KW_3:	//jhk_171015
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.0kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.857143mV
			//V2 = V1 * G2 = 9028.028572mV
			//300000mA : 9028.028572mV = I1 : 10000mV
			//I1 = 332298.461mA
			//ratio = 332298461uA / 32768 = 10140.94424

			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//100000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 110766.1536611mA
			//ratio = 110766153.6611uA / 32768 = 3380.314747959529
			
			if((range+1) == RANGE1) ratio = 10140.94424;
			else ratio = 3380.314747959529;
			break;
		case C_CANSYSTEM_60V_350A_100A_21KW:	//jhk_140828
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.0kohm = 10.88
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 350A:x -> x = 233.33mA
			//shuntV = 233.33mA * 2.5ohm = 583.33333mV
			//V1 = 583.33333mV * G1 = 6346.66667mV
			//V2 = V1 * G2 = 9481.92mV
			//350000mA : 9481.92mV = I1 : 10000mV
			//I1 = 369123.55303567mA
			//ratio = 369123553.03567uA / 32768 = 11264.7568675437
			
			//RANGE2
			//ITN-600 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 8.3ohm = 553.333mV
			//V1 = 553.333mV * G1 = 6020.2666667mV
			//V2 = V1 * G2 = 8994.2784mV
			//100000mA : 8994.2784mV = I1 : 10000mV
			//I1 = 111181.793083mA
			//ratio = 111181793.083uA / 32768 = 3392.999056489
			
			if((range+1) == RANGE1) ratio = 11264.7568675437;
			else ratio = 3392.999056489;
			break;
		case C_KTL_60V_400A_200A_40KW:	//jhk_181218
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677419
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 400A:x -> x = 266.667mA
			//shuntV = 266.667mA * 2.5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.87096774mV
			//400000mA : 8931.8709677mV = I1 : 10000mV
			//I1 = 447834.50348mA
			//ratio = 447834503.48uA / 32768 = 13666.82444706885
			
			//RANGE2
			//ITN-600 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 5ohm = 666.666mV
			//V1 = 666.666mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87mV
			//200000mA : 8931.87mV = I1 : 10000mV
			//I1 = 223917.25174mA
			//ratio = 223917251.74uA / 32768 = 6833.412223534425
			
			if((range+1) == RANGE1) ratio = 13666.82444706885;
			else ratio = 6833.412223534425;
			break;
		case C_DAEHWA_60V_500A_250A_50A_60KW:	//jhk_160913
		case C_DAEHWA_60V_500A_250A_50A_60KW_2:	//jhk_160913
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 500A:x -> x = 333.33mA
			//shuntV = 333.33mA * 2.5ohm = 833.33mV
			//V1 = 833.33mV * G1 = 5979.16667mV
			//V2 = V1 * G2 = 8932.875mV
			//500000mA : 8932.875mV = I1 : 10000mV
			//I1 = 559730.21mA
			//ratio = 559730210.0uA / 32768 = 17081.610413780558
			
			//RANGE2
			//ITN-600 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 5.0ohm = 833.33mV
			//V1 = 833.33mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//250000mA : 8932.875mV = I1 : 10000mV
			//I1 = 279865.10501938mA
			//ratio = 279865105.01938uA / 32768 = 8540.805206890279
			
			//RANGE3
			//ITN-600 600A:400mA = 50A:x -> x = 33.333mA
			//shuntV = 33.333mA * 25.0ohm = 833.33mV
			//V1 = 833.33mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//50000mA : 8932.875mV = I1 : 10000mV
			//I1 = 55973.021mA
			//ratio = 55973021.003876uA / 32768 = 1708.161041378056
			
			if((range+1) == RANGE1) ratio = 17081.610413780558;
			else if ((range+1) == RANGE2) ratio = 8540.805206890279;
			else ratio = 1708.161041378056;
			break;
		/*case C_LGC_70V_100A_28KW:   //ljh_201130
		case C_LGC_70V_100A_28KW_2: //ljh_201130
		    //10000000uV / 32768 = 305.1757813uV
		    //AMP -> AD620A
		    //G1 = 1 + 49.4kohm / 4.5kohm = 11.9778
		    //AMP -> AD620A
		    //G2 = 1 + 49.4kohm / 100kohm = 1.494
		    //RANGE1
		    //ITN300-S 300A:150mA = 100A:x -> x = 50mA
		    //shuntV = 50mA * 10ohm = 500mV
			//V1 = shuntV * G1 = 5988.8889mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142mA
			//ratio = 111764311.4201uA / 32768 = 3410.77610535
			ratio = 3410.77610535;
			break;*/	//shh_231024
		case C_LGC_70V_100A_14KW:	//jhk_180104
		case C_LGC_70V_100A_14KW_2:	//jhk_180104
		case C_LGC_70V_100A_14KW_3:	//jhk_180104
		case C_LGC_70V_100A_14KW_4:	//jhk_180104
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.9777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.8889mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142mA
			//ratio = 111764311.42uA / 32768 = 3410.77610534904
			
			ratio = 3410.77610534904;
			break;
		/*case C_LGC_60V_600A_300A_100A_144KW:	//ktg_190707
		case C_LGC_60V_600A_300A_100A_144KW_2:	//ktg_191222
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 600A:x -> x = 400mA
			//shuntV = 400mA * 2.5ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//600000mA : 8874.36mV = I1 : 10000mV
			//I1 = 676105.0937757765mA
			//ratio = 676105093.7757765uA / 32768 = 20633.09002
			
			//RANGE2
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 5ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//300000mA : 8874.36mV = I1 : 10000mV
			//I1 = 338052.546888mA
			//ratio = 338052546.888uA / 32768 = 10316.54501
			
			//RANGE3
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 15ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//100000mA : 8874.36mV = I1 : 10000mV
			//I1 = 112684.18229596mA
			//ratio = 112684182.29596uA / 32768 = 3438.8483366688
			
			if((range+1) == RANGE1) ratio = 20633.09002;
			else if ((range+1) == RANGE2) ratio = 10316.54501;
			else ratio = 3438.8483366688;
			break;*/
		case C_LGC_70V_100A_50A_28KW:		//ktg_190405
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714mV
			//V2 = V1 * G2 = 9028.0285714mV
			//100000mA : 9028.0285714mV = I1 : 10000mV
			//I1 = 110766.15366113mA
			//ratio = 110766153.66113uA / 32768 = 3380.3147479595

			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714mV
			//V2 = V1 * G2 = 9028.0285714mV
			//50000mA : 9028.0285714mV = I1 : 10000mV
			//I1 = 55383.0768305689mA
			//ratio = 55383076.8305689uA / 32768 = 1690.1573739798
			if((range+1) == RANGE1) ratio = 3380.3147479595;
			else ratio = 1690.1573739798;
			break;
		/*case C_LGC_70V_200A_100A_28KW:		//jhk_170911
		case C_LGC_70V_200A_100A_28KW_2:	//jhk_170911
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.9777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.8889mV
			//V2 = V1 * G2 = 8947.4mV
			//200000mA : 8947.4mV = I1 : 10000mV
			//I1 = 223528.62284mA
			//ratio = 223528622.84uA / 32768 = 6821.55221069808

			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.8889mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142mA
			//ratio = 111764311.42uA / 32768 = 3410.77610534904
			
			if((range+1) == RANGE1) ratio = 6821.55221069808;
			else ratio = 3410.77610534904;
			break;*/	//shh_231025
		case C_LGC_70V_250A_10A_35KW:
		//case C_LGC_70V_250A_10A_35KW_2:
		case C_LGC_70V_250A_10A_35KW_3:
		case C_LGC_70V_250A_10A_35KW_4:
		//case C_LGC_70V_250A_10A_35KW_5:
		//case C_LGC_70V_250A_10A_35KW_6:
		case C_LGC_70V_250A_10A_35KW_7:
		case C_LGC_70V_250A_10A_35KW_8:
		case C_LGC_70V_250A_10A_35KW_9:
		case C_LGC_70V_250A_10A_35KW_10:
		//case C_LGC_70V_250A_10A_35KW_11:
		//case C_LGC_70V_250A_10A_35KW_12:
		//case C_LGC_70V_250A_10A_35KW_13:
		case C_LGC_70V_250A_10A_35KW_14:
		//case C_LGC_70V_250A_10A_35KW_15:
		//case C_LGC_70V_250A_10A_35KW_16:
		//case C_LGC_70V_250A_10A_35KW_17:
		//case C_LGC_70V_250A_10A_35KW_18:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//250000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 289979.007mA
			//ratio = 289979007.0uA / 32768 = 8849.457001

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 125ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//10000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 11599.16028mA
			//ratio = 11599160.28uA / 32768 = 353.97828
			if((range+1) == RANGE1) ratio = 8849.457001;
			else ratio = 353.97828;
			break;
		case C_LGC_70V_250A_10A_35KW_2:	//kjhw_180212
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.75mV
			//V2 = V1 * G2 = 9027.8685mV
			//300000mA : 9027.8685mV = I1 : 10000mV
			//I1 = 332304.352904mA
			//ratio = 332304352.904uA / 32768 = 10141.1240510

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 150ohm = 750mV
			//V1 = 750mV * G1 = 6042.75mV
			//V2 = V1 * G2 = 9027.8685mV
			//10000mA : 9027.8685mV = I1 : 10000mV
			//I1 = 11076.8117634mA
			//ratio = 11076811.763485uA / 32768 = 338.037468368
			if((range+1) == RANGE1) ratio = 10141.1240510;
			else ratio = 338.03746836;
			break;
		//case C_LGC_70V_250A_10A_35KW_5:	//phb_240429
		case C_LGC_70V_250A_10A_35KW_6:
		//case C_LGC_70V_250A_10A_35KW_11:
		case C_LGC_70V_250A_10A_35KW_12:	//jhkw_130829
		//case C_LGC_70V_250A_10A_35KW_13:
		case C_LGC_70V_250A_10A_35KW_16:	//kjhw_131118
		case C_LGC_70V_250A_10A_35KW_17:	//kjhw_131119
		case C_LGC_70V_250A_10A_35KW_18:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//250000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 289979.007mA
			//ratio = 289979007.0uA / 32768 = 8849.457001

			//RANGE2
			//ITB300-S 300A:150mA = 25A:x -> x = 12.5mA
			//shuntV = 12.5mA * 50ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//25000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 28996.85384mA
			//ratio = 28996853.84uA / 32768 = 884.91375248
			if((range+1) == RANGE1) ratio = 8849.457001;
			else ratio = 884.91375248;
			break;
		case C_LGC_70V_250A_10A_35KW_15:	//ktg_220420
		case C_LGC_70V_250A_10A_35KW_28:	//shh_220614
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//250000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 289979.007mA
			//ratio = 289979007.0uA / 32768 = 8849.457001

			ratio = 8849.457001;
			break;
		case C_LGC_70V_250A_10A_35KW_11:
		case C_LGC_70V_250A_10A_35KW_13:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.0285714mV
			//300000mA : 9028.0285714mV = I1 : 10000mV
			//I1 = 332298.46098mA
			//ratio = 332298460.98uA / 32768 = 10140.944243878587
			
			//RANGE2
			//ITB300-S 300A:150mA = 30A:x -> x = 15mA
			//shuntV = 15mA * 50ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.0285714mV
			//30000mA : 9028.0285714mV = I1 : 10000mV
			//I1 = 33229.846098mA
			//ratio = 33229846.098uA / 32768 = 1014.094424387858
			if((range+1) == RANGE1) ratio = 10140.944243878587;
			else ratio = 1014.094424387858;
			break;
		case C_LGC_70V_250A_10A_35KW_19:    //111202_csk
		case C_LGC_70V_250A_10A_35KW_20:    //111202_csk
		case C_LGC_70V_250A_10A_35KW_21:    //jhk_120229
		case C_LGC_70V_250A_10A_35KW_22:    //jhk_120229
		case C_LGC_70V_250A_10A_35KW_23:    //jhk_120702
		case C_LGC_70V_250A_10A_35KW_24:    //jhk_120702
		case C_LGC_70V_250A_10A_35KW_25:    //jhk_120702
		case C_LGC_70V_250A_10A_35KW_26:    //jhk_120702
		case C_LGC_70V_250A_10A_35KW_27:    //jhk_120702
		//case C_LGC_70V_250A_10A_35KW_28:    //jhk_120702	//shh_220614
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.538mA
			//ratio = 289968538.5uA / 32768 = 8849.137525

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 125ohm = 625mV
			//V1 = 625mV * G1 = 5770.833mV
			//V2 = V1 * G2 = 8621.625mV
			//10000mA : 8621.625mV = I1 : 10000mV
			//I1 = 11598.74154mA
			//ratio = 11598741.54uA / 32768 = 353.96550
			if((range+1) == RANGE1) ratio = 8849.137525;
			else ratio = 353.96550;
			break;
		/*case C_LGC_70V_250A_50A_35KW:		//jhk_150502
		//case C_LGC_70V_250A_50A_35KW_2:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_3:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_4:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_5:		//jhk_150502
		//case C_LGC_70V_250A_50A_35KW_6:		//jhk_150502
		//case C_LGC_70V_250A_50A_35KW_7:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_8:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_9:		//jhk_160515
		case C_LGC_70V_250A_50A_35KW_10:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_11:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_12:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_13:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_14:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_15:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_16:	//jhk_160515
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233333333
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 150mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.53841358mA
			//ratio = 289968538.41358uA / 32768 = 8849.137524828556
			
			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 25ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//50000mA : 8621.625mV = I1 : 10000mV
			//I1 = 57993.7076827164mA
			//ratio = 57993707.6827164uA / 32768 = 1769.827504965711
			
			if((range+1) == RANGE1) ratio = 8849.137524828556;
			else ratio = 1769.827504965711;
			break;*/
		/*case C_LGC_70V_250A_50A_35KW_2:		//ktg_191218
		//case C_LGC_70V_250A_50A_35KW_7:		//ktg_191219
		case C_LGC_70V_250A_50A_35KW_6:		//ktg_191218
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.0285714mV
			//300000mA : 9028.0285714mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.98uA / 32768 = 10140.9442438785876
			
			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.0285714mV
			//50000mA : 9028.0285714mV = I1 : 10000mV
			//I1 = 55383.0768305689mA
			//ratio = 55383076.8305689uA / 32768 = 1690.157373979765
			
			if((range+1) == RANGE1) ratio = 10140.9442438785876;
			else ratio = 1690.157373979765;
			break;*/	//sec_231026
		case C_LGC_70V_250A_100A_50A_35KW:		//jhkw_121019
		case C_LGC_70V_250A_100A_50A_35KW_2:	//jhkw_121019
		case C_LGC_70V_250A_100A_50A_35KW_3:	//jhkw_121019
		case C_LGC_70V_250A_100A_50A_35KW_4:	//jhkw_121019
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233333333
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 150mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.53841358mA
			//ratio = 289968538.41358uA / 32768 = 8849.137524828556
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 12.5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//100000mA : 8621.625mV = I1 : 10000mV
			//I1 = 115987.41536543mA
			//ratio = 115987415.36543uA / 32768 = 3539.655009931422
			
			//RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 25ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//50000mA : 8621.625mV = I1 : 10000mV
			//I1 = 57993.7076827164mA
			//ratio = 57993707.6827164uA / 32768 = 1769.827504965711
			
			if((range+1) == RANGE1) ratio = 8849.137524828556;
			else if ((range+1) == RANGE2) ratio = 3539.655009931422;
			else ratio = 1769.827504965711;
			break;
		case C_LGC_70V_300A_100A_42KW:	//jhk_170211
		case C_LGC_70V_300A_100A_42KW_2:	//jhk_170519
		case C_LGC_70V_300A_100A_42KW_3:	//jhk_170211
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714285714
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//300000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.94424387858
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//100000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 110766.1536611mA
			//ratio = 110766153.6611uA / 32768 = 3380.314747959529
			
			if((range+1) == RANGE1) ratio = 10140.944243;
			else ratio = 3380.314747959529;
			break;
		/*case C_LGC_70V_300A_100A_50A_10A_42KW:	//jhkw_120810
		case C_LGC_70V_300A_100A_50A_10A_42KW_2:	//jhkw_120813
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714285714
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//300000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.94424387858
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//100000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 110766.1536611mA
			//ratio = 110766153.6611uA / 32768 = 3380.314747959529
			
			//RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//50000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 55383.0768305689266mA
			//ratio = 55383076.83056892uA / 32768 = 1690.157373979764606
			
			//RANGE4
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 150ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//10000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 11076.615366113785mA
			//ratio = 11076615.366113785uA / 32768 = 338.0314747959529212
			
			if((range+1) == RANGE1) ratio = 10140.944243;
			else if ((range+1) == RANGE2) ratio = 3380.314747959529;
			else if ((range+1) == RANGE3) ratio = 1690.157373;
			else ratio = 338.031474;
			break;*/	//shh_231027
		case C_LGCUSA_70V_300A_42KW:	//jhk_160802
		case C_LGCUSA_70V_300A_42KW_2:	//jhk_160802
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714285714
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//300000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.94424387858
			
			ratio = 10140.944243;
			break;
		case C_LGCUSA_70V_350A_24KW:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_2:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_3:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_4:	//jhk_160720
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.0kohm = 10.88
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 350A:x -> x = 233.33mA
			//shuntV = 233.33mA * 2.5ohm = 583.33333mV
			//V1 = 583.33333mV * G1 = 6346.66667mV
			//V2 = V1 * G2 = 9481.92mV
			//350000mA : 9481.92mV = I1 : 10000mV
			//I1 = 369123.55303567mA
			//ratio = 369123553.03567uA / 32768 = 11264.7568675437
			
			ratio = 11264.7568675437;
			break;
		/*case C_LGC_70V_350A_100A_49KW:		//jhk_170911
		case C_LGC_70V_350A_100A_49KW_2:	//jhk_170911
		case C_LGC_70V_350A_100A_49KW_3:	//jhk_170911
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 12kohm = 5.11666667
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN-600 600A:400mA = 350A:x -> x = 233.33mA
			//shuntV = 233.33mA * 5.0ohm = 1166.667mV
			//V1 = 1166.667mV * G1 = 5969.4444mV
			//V2 = V1 * G2 = 8918.35mV
			//350000mA : 8918.35mV = I1 : 10000mV
			//I1 = 392449.27593mA
			//ratio = 392449275.931uA / 32768 = 11976.601438326596
			
			//RANGE2
			//ITN-600 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 17.5ohm = 1166.667mV
			//V1 = 1166.667mV * G1 = 5969.445mV
			//V2 = V1 * G2 = 8918.35mV
			//100000mA : 8918.35mV = I1 : 10000mV
			//I1 = 112128.36455mA
			//ratio = 112128364.55uA / 32768 = 3421.88612523617

			if((range+1) == RANGE1) ratio = 11976.601438326596;
			else ratio = 3421.88612523617;
			break; */ //shh_231025
		case C_LGC_70V_400A_200A_56KW:			//ktg_190711
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 400A:x -> x = 266.667mA
			//shuntV = 266.667mA * 3ohm = 800mV
			//V1 = 800mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//400000mA : 8575.56mV = I1 : 10000mV
			//I1 = 466441.84169897mA
			//ratio = 466441841.69897uA / 32768 = 14234.6753448
			
			//RANGE2
			//ITN600-S 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 6ohm = 800mV
			//V1 = 800mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//200000mA : 8575.56mV = I1 : 10000mV
			//I1 = 233220.9208495mA
			//ratio = 233220920.8495uA / 32768 = 7117.3376724
			
			if((range+1) == RANGE1) ratio = 14234.6753448;
			else ratio = 7117.3376724;
			break;
		case C_LGCCHINA_80V_150A_12KW:		//jhk_160725
		case C_LGCCHINA_80V_150A_12KW_2:	//jhk_160725
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714285714
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 150A:x -> x = 75mA
			//shuntV = 75mA * 10ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//150000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 166149.230491706mA
			//ratio = 166149230.491706uA / 32768 = 5070.47212193929
			
			ratio = 5070.472121;
			break;
		/*case C_LGC_90V_300A_100A_108KW:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_2:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_3:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_4:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_5:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_6:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_7:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_8:	//jhk_181016*/	//ktg_231023
		case C_LGCCHINA_90V_300A_100A_108KW:	//ktg_190319
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 5ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//300000mA : 8874.36mV = I1 : 10000mV
			//I1 = 338052.54688mA
			//ratio = 338052546.88uA / 32768 = 10316.545010006355
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 15ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//100000mA : 8874.36mV = I1 : 10000mV
			//I1 = 112684.18229596mA
			//ratio = 112684182.29596uA / 32768 = 3438.848336668785
			
			if((range+1) == RANGE1) ratio = 10316.545010006355;
			else ratio = 3438.848336668785;
			break;
		case C_LGC_90V_500A_250A_100A_90KW:	//jhk_181107
		//case C_LGC_90V_500A_250A_100A_180KW:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_2:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_3:	//jhk_181006	//sec_221027
		//case C_LGC_90V_500A_250A_100A_180KW_4:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_5:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_6:	//jhk_181006	//ktg_231023
		/*case C_LGC_90V_500A_250A_100A_180KW_7:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_8:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_9:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_10:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_11:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_12:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_13:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_14:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_15:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_16:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_17:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_18:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_19:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_20:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_21:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_22:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_23:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_24:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_25:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_26:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_27:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_28:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_29:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_30:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_31:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_32:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_33:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_34:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_35:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_36:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_37:	//ktg_190219 */ //ktg_231124
		case C_LGC_90V_500A_250A_100A_180KW_38:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_39:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_40:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_41:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_42:	//ktg_200104
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 500A:x -> x = 333.333mA
			//shuntV = 333.333mA * 2.5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//500000mA : 8932.875mV = I1 : 10000mV
			//I1 = 559730.21003876mA
			//ratio = 559730210.03uA / 32768 = 17081.610413780558
			
			//RANGE2
			//ITN600-S 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//250000mA : 8932.875mV = I1 : 10000mV
			//I1 = 279865.105mA
			//ratio = 279865105.019uA / 32768 = 8540.805206890279
			
			//RANGE3
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 12.5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.16667mV
			//V2 = V1 * G2 = 8932.875mV
			//100000mA : 8932.875mV = I1 : 10000mV
			//I1 = 111946.042mA
			//ratio = 111946042.007uA / 32768 = 3416.322082756112
			
			if((range+1) == RANGE1) ratio = 17081.610413780558;
			else if ((range+1) == RANGE2) ratio = 8540.805206890279;
			else ratio = 3416.322082756112;
			break;
		case C_LGC_90V_500A_300A_100A_180KW:	//ktg_191106
		case C_LGC_90V_500A_300A_100A_180KW_2:	//ktg_191106
		/*case C_LGC_90V_500A_300A_100A_180KW_3:	//ljh_201130
		case C_LGC_90V_500A_300A_100A_180KW_4:	//ljh_201130
		case C_LGC_90V_500A_300A_100A_180KW_5:	//ljh_201130
		case C_LGC_90V_500A_300A_100A_180KW_6:	//khj_200303*/	//shh_231026
		//case C_LGC_90V_500A_300A_100A_180KW_7:	//shh_220324
		//case C_LGC_90V_500A_300A_100A_180KW_8:	//shh_220324
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 500A:x -> x = 333.333mA
			//shuntV = 333.333mA * 3ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//500000mA : 8874.36mV = I1 : 10000mV
			//I1 = 563420.911479814mA
			//ratio = 563420911.479814uA / 32768 = 17194.241683344
			
			//RANGE2
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 5ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//300000mA : 8874.36mV = I1 : 10000mV
			//I1 = 338052.546887888mA
			//ratio = 338052546.887888uA / 32768 = 10316.54501001
			
			//RANGE3
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 15ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//100000mA : 8874.36mV = I1 : 10000mV
			//I1 = 112684.18229596275mA
			//ratio = 112684182.29596275uA / 32768 = 3438.84833667
			
			if((range+1) == RANGE1) ratio = 17194.241683344;
			else if ((range+1) == RANGE2) ratio = 10316.54501001;
			else ratio = 3438.84833667;
			break;
		/*case C_LGC_90V_600A_300A_100A_216KW:	//ktg_190707
		case C_LGC_90V_600A_300A_100A_216KW_2:	//ktg_200709
		case C_LGC_90V_600A_300A_100A_216KW_3:	//ktg_200709
		case C_LGC_90V_600A_300A_100A_216KW_4:	//ktg_200709
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 600A:x -> x = 400mA
			//shuntV = 400mA * 2.5ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//600000mA : 8874.36mV = I1 : 10000mV
			//I1 = 676105.0937757765mA
			//ratio = 676105093.7757765uA / 32768 = 20633.09002
			
			//RANGE2
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 5ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//300000mA : 8874.36mV = I1 : 10000mV
			//I1 = 338052.546888mA
			//ratio = 338052546.888uA / 32768 = 10316.54501
			
			//RANGE3
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 15ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//100000mA : 8874.36mV = I1 : 10000mV
			//I1 = 112684.18229596mA
			//ratio = 112684182.29596uA / 32768 = 3438.8483366688
			
			if((range+1) == RANGE1) ratio = 20633.09002;
			else if ((range+1) == RANGE2) ratio = 10316.54501;
			else ratio = 3438.8483366688;
			break;*/
		case C_LGE_100V_30A_3KW:			//khj_191127
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.2333
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 30A:x -> x = 20mA
			//shuntV = 20mA * 30ohm = 600mV
			//V1 = 600mV * G1 = 5539.98mV
			//V2 = V1 * G2 = 8276.73012mV
			//30000mA : 8276.73012mV = I1 : 10000mV
			//I1 = 36246.1981544mA
			//ratio = 36246198.1544uA / 32768 = 1106.1461839
			
			ratio = 1106.1461839;
			break;
		/*	
		case C_LGC_100V_100A_50A_20KW:	//jhk_181023
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.977777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.8888888889mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142007734mA
			//ratio = 111764311.42007734uA / 32768 = 3410.7761053490
			
			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 20ohm = 500mV
			//V1 = 500mV * G1 = 5988.88888888889mV
			//V2 = V1 * G2 = 8947.4mV
			//50000mA : 8947.4mV = I1 : 10000mV
			//I1 = 55882.1557100mA
			//ratio = 55882155.7100uA / 32768 = 1705.3880526745
			
			if((range+1) == RANGE1) ratio = 3410.7761053490;
			else ratio = 1705.3880526745;
			break;
		*/	//shh_231024
		case C_JBTP_100V_100A_40KW:  	//shh_200927
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 100A:x -> x = 66.66667mA
			//shuntV = 66.66667mA * 10ohm = 666.6667mV
			//V1 = 666.6667 * G1 = 5978.49462365mV
			//V2 = V1 * G2 = 8931.87096774mV
			//100000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 111958.625870388mA
			//ratio = 111958625.870388uA / 32768 = 1106.1461839
			
			ratio = 3416.70611177;
			break;
		case C_POWERLOGICS_100V_100A_50A_40KW:	//jhk_180919
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714285714
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//100000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 110766.1536611mA
			//ratio = 110766153.6611uA / 32768 = 3380.314747959529
			
			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//50000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 55383.0768305689266mA
			//ratio = 55383076.83056892uA / 32768 = 1690.157373979764606
			
			if((range+1) == RANGE1) ratio = 3380.314747959529;
			else ratio = 1690.157373;
			break;
		case C_JFMTECH_100V_100A_50A_40KW:			//csk_190923
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.5kohm = 8.6
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 1500A:1A = 100A:x -> x = 66.6667mA
			//shuntV = 66.6667mA * 10ohm = 666.6667mV
			//V1 = 666.6667mV * G1 = 5733.3334mV
			//V2 = V1 * G2 = 8565.6mV
			//100000mA : 8565.6mV = I1 : 10000mV
			//I1 = 116746.053983375mA
			//ratio = 116746053.983375uA / 32768 = 3562.806823223
			
			//RANGE2
			//ITN600-S 1500A:1A = 50A:x -> x = 33.3333mA
			//shuntV = 33.3333mA * 20ohm = 666.6667mV
			//V1 = 666.6667mV * G1 = 5733.3334mV
			//V2 = V1 * G2 = 8565.6mV
			//50000mA : 8565.6mV = I1 : 10000mV
			//I1 = 58373.0269916mA
			//ratio = 58373026.991uA / 32768 = 1781.40341161156
			
			if((range+1) == RANGE1) ratio = 3562.806823223;
			else ratio = 1781.40341161156;
			break;
		case C_CORN_100V_100A_50A_40KW:	//ktg_190210
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 1500A:1A = 100A:x -> x = 66.6667mA
			//shuntV = 66.6667mA * 12.5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//100000mA : 8932.875mV = I1 : 10000mV
			//I1 = 111946.042007752mA
			//ratio = 111946042.007752uA / 32768 = 3416.3220827561
			
			//RANGE2
			//ITN600-S 1500A:1A = 50A:x -> x = 33.3333mA
			//shuntV = 33.3333mA * 25ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//50000mA : 8932.875mV = I1 : 10000mV
			//I1 = 55973.021004mA
			//ratio = 55973021.004uA / 32768 = 1708.16104137805
			
			if((range+1) == RANGE1) ratio = 3416.3220827561;
			else ratio = 1708.16104137805;
			break;
		case C_SEBANG_100V_100A_50A_10A_40KW:	//ktg_190324
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.5kohm = 7.58667
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 5690mV
			//V2 = V1 * G2 = 8500.86mV
			//100000mA : 8500.86mV = I1 : 10000mV
			//I1 = 117635.156913536mA
			//ratio = 117635156.913536uA / 32768 = 3589.94009135546
			
			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 5690mV
			//V2 = V1 * G2 = 8500.86mV
			//50000mA : 8500.86mV = I1 : 10000mV
			//I1 = 58817.5784567679mA
			//ratio = 58817578.4567679uA / 32768 = 1794.97004567773
			
			//RANGE3
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 150ohm = 750mV
			//V1 = 750mV * G1 = 5690mV
			//V2 = V1 * G2 = 8500.86mV
			//10000mA : 8500.96mV = I1 : 10000mV
			//I1 = 11763.51569135358mA
			//ratio = 11763515.69135358uA / 32768 = 358.994009135546
			
			if((range+1) == RANGE1) ratio = 3589.94009135546;
			else if((range+1) == RANGE2) ratio = 1794.97004567773;
			else ratio = 358.994009135546;
			break;
		case C_INCELL_100V_100A_50A_10A_40KW:	//ktg_190210
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 1500A:1A = 100A:x -> x = 66.6667mA
			//shuntV = 66.6667mA * 12.5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//100000mA : 8932.875mV = I1 : 10000mV
			//I1 = 111946.042007752mA
			//ratio = 111946042.007752uA / 32768 = 3416.3220827561
			
			//RANGE2
			//ITN600-S 1500A:1A = 50A:x -> x = 33.3333mA
			//shuntV = 33.3333mA * 25ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//50000mA : 8932.875mV = I1 : 10000mV
			//I1 = 55973.021004mA
			//ratio = 55973021.004uA / 32768 = 1708.16104137805
			
			//RANGE3
			//ITN600-S 1500A:1A = 10A:x -> x = 6.6667mA
			//shuntV = 6.6667mA * 125ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//10000mA : 8932.875mV = I1 : 10000mV
			//I1 = 11194.6042007752mA
			//ratio = 11194604.2007752uA / 32768 = 341.63220827561
			
			if((range+1) == RANGE1) ratio = 3416.3220827561;
			else if((range+1) == RANGE2) ratio = 1708.16104137805;
			else ratio = 341.63220827561;
			break;
		case C_GITC_100V_100A_50A_10A_40KW:	//jhk_180917
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714285714
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//100000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 110766.1536611mA
			//ratio = 110766153.6611uA / 32768 = 3380.314747959529
			
			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//50000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 55383.0768305689266mA
			//ratio = 55383076.83056892uA / 32768 = 1690.157373979764606
			
			//RANGE3
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 150ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//10000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 11076.615366113785mA
			//ratio = 11076615.366113785uA / 32768 = 338.0314747959529212
			
			if((range+1) == RANGE1) ratio = 3380.314747959529;
			else if ((range+1) == RANGE2) ratio = 1690.157373;
			else ratio = 338.031474;
			break;
		case C_SKI_100V_100A_10A_10KW:  //jhk_120329
		case C_LGC_100V_100A_10A_10KW:  //jhk_121009
		case C_LGC_100V_100A_10A_10KW_2:  //jhk_121009
		case C_LGC_100V_100A_10A_10KW_3:  //jhk_121009
		case C_LGC_100V_100A_10A_10KW_4:  //jhk_121009
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//100000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 111764.3114mA
			//ratio = 111764311.4uA / 32768 = 3410.776105
			
			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 100ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//10000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 11176.43114mA
			//ratio = 11176431.14uA / 32768 = 341.0776105

			if((range+1) == RANGE1) ratio = 3410.776105;
			else ratio = 341.0776105;
			break;
		case C_SAMHWA_100V_100A_10A:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//100000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 111764.3114mA
			//ratio = 111764311.4uA / 32768 = 3410.776105

			//RANGE1
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 100ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//10000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 11176.43114mA
			//ratio = 11176431.14uA / 32768 = 341.0776105
			if((range+1) == RANGE1) ratio = 3410.776105;
			else ratio = 341.0776105;
			break;
		case C_SDI_100V_150A_70A_30KW:			//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_2:		//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_3:		//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_4:		//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_5:		//jhkw_130821
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 150A:x -> x = 75mA
			//shuntV = 75mA * 7ohm = 525mV
			//V1 = 525mV * G1 = 6288.333333mV
			//V2 = V1 * G2 = 9394.77mV
			//150000mA : 9394.77mV = I1 : 10000mV
			//I1 = 159663.30202868mA
			//ratio = 159663302.02868uA / 32768 = 4872.53729335577

			//RANGE2
			//ITB300-S 300A:150mA = 70A:x -> x = 35mA
			//shuntV = 35mA * 15ohm = 525mV
			//V1 = 525mV * G1 = 6288.333333mV
			//V2 = V1 * G2 = 9394.77mV
			//70000mA : 9394.77mV = I1 : 10000mV
			//I1 = 74509.5409467mA
			//ratio = 74509540.946718uA / 32768 = 2273.850736899
			if((range+1) == RANGE1) ratio = 4872.53729335577;
			else ratio = 2273.850736899;
			break;
		case C_POWERLOGICS_100V_150A_60KW:	//shh_200928
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 150A:x -> x = 100mA
			//shuntV = 100mA * 10ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//150000mA : 8874.36mV = I1 : 10000mV
			//I1 = 169026.2734439441mA
			//ratio = 169026273.4439441uA / 32768 = 5158.27250500
			
			ratio = 5158.27250500;
			break;
		case C_LGC_100V_150A_100A_30KW:	//jhk_190419
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 150A:x -> x = 100mA
			//shuntV = 100mA * 8ohm = 800mV
			//V1 = 800mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//150000mA : 8575.56mV = I1 : 10000mV
			//I1 = 174915.690637mA
			//ratio = 174915690.637uA / 32768 = 5338.0032543
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 12ohm = 800mV
			//V1 = 800mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//100000mA : 8575.56mV = I1 : 10000mV
			//I1 = 116610.4604247mA
			//ratio = 116610460.4247uA / 32768 = 3558.6688362
			
			if((range+1) == RANGE1) ratio = 5338.0032543;
			else ratio = 3558.6688362;
			break;
		case C_LGCCHINA_100V_200A_20KW_3:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_4:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_5:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_6:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_7:	//jhk_160731
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//200000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 223528.6228mA
			//ratio = 223528622.8uA / 32768 = 6821.552209

			ratio = 6821.552209;
			break;
		case C_LGC_100V_200A_10A_20KW:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//200000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 223528.6228mA
			//ratio = 223528622.8uA / 32768 = 6821.552209

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 100ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//10000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 11176.43114mA
			//ratio = 11176431.14uA / 32768 = 341.0776105
			if((range+1) == RANGE1) ratio = 6821.552209;
			else ratio = 341.0776105;
			break;
		case C_LAONTECH_100V_200A_40KW:		//shh_200927
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 10ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//200000mA : 8874.36mV = I1 : 10000mV
			//I1 = 225368.3645919255mA
			//ratio = 225368364.5919255uA / 32768 = 6877.69667334

			ratio = 6877.69667334;
			break;
		case C_HYUNDAI_100V_200A_100A_40KW:	//shh_201005
		case C_HYUNDAI_100V_200A_100A_80KW:	//shh_210311
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.967742
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 5ohm = 666.6667mV
			//V1 = 666.6667mV * G1 = 5978.49462365mV
			//V2 = V1 * G2 = 8931.87096774mV
			//200000mA : 8931.87096774mA = I1 : 10000mV
			//I1 =  223917.2517408246mA
			//ratio = 223917251.7408246uA / 32768 = 6833.41222354

			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.66667mA
			//shuntV = 66.66667mA * 10ohm = 666.6667mV
			//V1 = 666.6667mV * G1 = 5978.49462365mV
			//V2 = V1 * G2 = 8931.87096774mV
			//100000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 111958.6258704123mA
			//ratio = 111958625.8704123uA / 32768 = 3416.70611177
			if((range+1) == RANGE1) ratio = 6833.41222354;
			else ratio = 3416.70611177;
			break;
		case C_BOSUNG_100V_200A_100A_40KW:	//jhk_161226
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//200000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 223528.6228mA
			//ratio = 223528622.8uA / 32768 = 6821.552209

			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//100000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 111764.31142mA
			//ratio = 111764311.42uA / 32768 = 3410.7761053490399
			if((range+1) == RANGE1) ratio = 6821.552209;
			else ratio = 3410.776105349;
			break;
		case C_KIER_100V_200A_100A_20A_40KW:	//jhk_161226
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//200000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 223528.6228mA
			//ratio = 223528622.8uA / 32768 = 6821.552209

			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//100000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 111764.31142mA
			//ratio = 111764311.42uA / 32768 = 3410.7761053490399

			//RANGE3
			//ITB300-S 300A:150mA = 20A:x -> x = 10mA
			//shuntV = 10mA * 50ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//20000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 22352.862284mA
			//ratio = 22352862.284uA / 32768 = 682.1552210698
			if((range+1) == RANGE1) ratio = 6821.552209;
			else if ((range+1) == RANGE2) ratio = 3410.776105349;
			else ratio = 682.1552210698;
			break;
		case C_LGC_100V_200A_100A_80KW:		//ktg_200208
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.8709677mV
			//200000mA : 8931.8709677mV = I1 : 10000mV
			//I1 = 223917.251740776mA
			//ratio = 223917251.740776uA / 32768 = 6833.412223534425
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.8709677mV
			//100000mA : 8931.8709677mV = I1 : 10000mV
			//I1 = 111958.625870388mA
			//ratio = 111958625.870388uA / 32768 = 3416.706111767213
			
			if((range+1) == RANGE1) ratio = 6833.4122235;
			else ratio = 3416.706111767;
			break;
		case C_LGC_100V_250A_100A_100KW:	//jhk_181006
		case C_KATECH_100V_250A_100A_100KW:	//ktg_190120
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.8kohm = 9.51724
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5948.275862mV
			//V2 = V1 * G2 = 8886.724137931mV
			//250000mA : 8886.724137931mV = I1 : 10000mV
			//I1 = 281318.510758mA
			//ratio = 281318510.758uA / 32768 = 8585.159630066158

			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 12.5ohm = 625mV
			//V1 = 625mV * G1 = 5948.275862mV
			//V2 = V1 * G2 = 8886.724137931mV
			//100000mA : 8886.724137931mV = I1 : 10000mV
			//I1 = 112527.4043mA
			//ratio = 112527404.3032uA / 32768 = 3434.063852026463
			if((range+1) == RANGE1) ratio = 8585.159630066158;
			else ratio = 3434.063852026463;
			break;
		case C_HCC_100V_300A_100A_60KW:		//jhkw_131106
		case C_H2_100V_300A_100A_60KW:	//jhk_150726
		case C_H2_100V_300A_100A_60KW_2:	//jhk_170110
		case C_SWISS_100V_300A_100A_60KW:	//jhk_160825
		case C_SDI_100V_300A_100A_60KW_3:		//jhk_150206
		case C_SDI_100V_300A_100A_60KW_4:		//jhk_150206
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714285714
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//300000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.94424387858
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//100000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 110766.15366mA
			//ratio = 110766153.6611uA / 32768 = 3380.3147479595
			
			if((range+1) == RANGE1) ratio = 10140.944243;
			else ratio = 3380.3147479595;
			break;
		case C_HYUNDAI_100V_300A_150A_30A_60KW:		//ktg_190322
		case C_HYUNDAI_100V_300A_150A_30A_60KW_2:		//ktg_190322
		case C_HYUNDAI_100V_300A_150A_30A_60KW_3:		//ktg_190322
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 4ohm = 800mV
			//V1 = 800mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//300000mA : 8575.56mV = I1 : 10000mV
			//I1 = 349831.3812742mA
			//ratio = 349831381.2742uA / 32768 = 10676.00650861
			
			//RANGE2
			//ITN600-S 600A:400mA = 150A:x -> x = 100mA
			//shuntV = 100mA * 8ohm = 800mV
			//V1 = 800mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//150000mA : 8575.56mV = I1 : 10000mV
			//I1 = 174915.6906371mA
			//ratio = 174915690.6371uA / 32768 = 5338.003254306
			
			//RANGE3
			//ITN600-S 600A:400mA = 30A:x -> x = 20mA
			//shuntV = 20mA * 40ohm = 800mV
			//V1 = 800mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//30000mA : 8575.56mV = I1 : 10000mV
			//I1 = 34983.1381274226mA
			//ratio = 34983138.1274226uA / 32768 = 1067.600650861
			
			
			if((range+1) == RANGE1) ratio = 10676.00650861;
			else if ((range+1) == RANGE2) ratio = 5338.003254306;
			else ratio = 1067.600650861;
			break;
		case C_SKI_100V_300A_150A_50A_10A_60KW:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_2:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_3:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_4:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_5:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_6:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_7:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_8:	//kjh_120527
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714285714
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//300000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.94424387858
			
			//RANGE2
			//ITB300-S 300A:150mA = 150A:x -> x = 75mA
			//shuntV = 75mA * 10ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//150000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 166149.230491706mA
			//ratio = 166149230.491706uA / 32768 = 5070.47212193929
			
			//RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//50000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 55383.0768305689266mA
			//ratio = 55383076.83056892uA / 32768 = 1690.157373979764606
			
			//RANGE4
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 150ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//10000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 11076.615366113785mA
			//ratio = 11076615.366113785uA / 32768 = 338.0314747959529212
			
			if((range+1) == RANGE1) ratio = 10140.944243;
			else if ((range+1) == RANGE2) ratio = 5070.472121;
			else if ((range+1) == RANGE3) ratio = 1690.157373;
			else ratio = 338.031474;
			break;
		case C_LGE_100V_400A_80KW:	//ljh_210217
		case C_LGE_100V_400A_80KW_2:	//ljh_210217
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.8kohm = 9.5172
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN 600-S 600A:400mA = 400A:x -> x = 266.667mA
			//shuntV = 266.667mA * 2.5ohm = 666.667mV
			//V1 = shuntV * G1 = 6344.8275862mV
			//V2 = V1 * G2 = 9479.17241379mV
			//400000mA : 9479.17241379mV = I1 : 10000mV
			//I1 = 421977.766137mA
			//ratio = 421977766.137uA / 32768 = 12877.7394451
			
			//RANGE2
			//ITN 600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = shuntV * G1 = 6344.8275862mV
			//V2 = V1 * G2 = 9479.1724138mV
			//100000mA : 9479.1724138mV = I1 : 10000mV
			//I1 = 105494.441534253mA
			//ratio = 105494441.534253uA / 32768 = 3219.43486
			if((range+1) == RANGE1) ratio = 12877.7394451;
			else ratio = 3219.43486;
			break;
		case C_KIER_100V_400A_200A_160KW:		//shh_201103
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.7kohm = 7.41558
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB600-S 600A:400mA = 400A:x -> x = 266.667mA
			//shuntV = 266.667mA * 3ohm = 800mV
			//V1 = 800mV * G1 = 5932.46753247mV
			//V2 = V1 * G2 = 8863.10649351mV
			//400000mA : 8863.10649351mV = I1 : 10000mV
			//I1 = 451309.03063509145mA
			//ratio = 451309030.63509144uA / 32768 = 13772.85860092
			
			//RANGE2
			//ITB600-S 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 8ohm = 800mV
			//V1 = 800mV * G1 = 5932.46753247mV
			//V2 = V1 * G2 = 8863.10649351mV
			//200000mA : 8863.10649351mV = I1 : 10000mV
			//I1 = 225654.51531754572mA
			//ratio = 225654515.31754572uA / 32768 = 6886.42930046
			
			if((range+1) == RANGE1) ratio = 13772.85860092;
			else ratio = 6886.42930046;
			break;
		case C_KBTP_100V_300A_100A_60KW:		//shh_210412
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 5ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//300000mA : 8874.36mV = I1 : 10000mV
			//I1 = 338052.5468878883mA
			//ratio = 338052546.8878883uA / 32768 = 10316.54501000
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 15ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//100000mA : 8874.36mV = I1 : 10000mV
			//I1 = 112684.1822959628mA
			//ratio = 112684,182.2959628uA / 32768 = 3438.84833666 
			
			if((range+1) == RANGE1) ratio = 10316.54501000;
			else ratio = 3438.84833666;
			break;
		case C_KTL_100V_500A_100A_100KW:			//shh_200809
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB600-S 600A:400mA = 500A:x -> x = 333.333mA
			//shuntV = 333.333mA * 2ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5,978.4696mV
			//V2 = V1 * G2 = 8931.871mV
			//500000mA : 8931.871mV = I1 : 10000mV
			//I1 = 559793.12935194mA
			//ratio = 559793129.35194uA / 32768 = 17083.5305588
			
			//RANGE2
			//ITB600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.871mV
			//100000mA : 8931.871mV = I1 : 10000mV
			//I1 = 111958.6258704mA
			//ratio = 111958625.8704uA / 32768 = 3416.70611177
			
			if((range+1) == RANGE1) ratio = 17083.5305588;
			else ratio = 3416.70611177;
			break;
		case C_KTL_100V_500A_100A_100KW_2:  //ljh_210319
		case C_KTL_100V_500A_100A_100KW_3:	//ljh_210324
		case C_KTL_100V_500A_100A_100KW_4:	//ljh_210324
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 500A:x -> x = 333.333mA
			//shuntV = 333.333mA * 2ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5,978.4696mV
			//V2 = V1 * G2 = 8931.871mV
			//500000mA : 8931.871mV = I1 : 10000mV
			//I1 = 559793.12935194mA
			//ratio = 559793129.35194uA / 32768 = 17083.5305588
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.871mV
			//100000mA : 8931.871mV = I1 : 10000mV
			//I1 = 111958.6258704mA
			//ratio = 111958625.8704uA / 32768 = 3416.70611177
			
			if((range+1) == RANGE1) ratio = 17083.5305588;
			else ratio = 3416.70611177;
			break;
		case C_SEINENG_100V_500A_200A_30A_200KW:	//ktg_200116
		case C_SEINENG_100V_500A_200A_30A_200KW_2:	//ktg_200116
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10.0kohm = 5.94
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 500A:x -> x = 333.33mA
			//shuntV = 333.33mA * 3.0ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//500000mA : 8874.36mV = I1 : 10000mV
			//I1 = 563420.9114798138mA
			//ratio = 563420911.4798138uA / 32768 = 17194.24168334393
			
			//RANGE2
			//ITN-600 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 7.5ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//200000mA : 8874.36mV = I1 : 10000mV
			//I1 = 225368.3645919255mA
			//ratio = 225368364.5919255uA / 32768 = 6877.69667333757
			
			//RANGE3
			//ITN-600 600A:400mA = 30A:x -> x = 20mA
			//shuntV = 20mA * 50.0ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//30000mA : 8874.36mV = I1 : 10000mV
			//I1 = 33805.25468878883mA
			//ratio = 33805254.68878883uA / 32768 = 1031.654501000636
			
			if((range+1) == RANGE1) ratio = 17194.2416833;
			else if((range+1) == RANGE2) ratio = 6877.696673334;
			else ratio = 1031.6545010006;
			break;
		case C_DAEHWA_100V_500A_250A_50KW:	//jhk_170915
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 500A:x -> x = 333.33mA
			//shuntV = 333.33mA * 2.5ohm = 833.33mV
			//V1 = 833.33mV * G1 = 5979.16667mV
			//V2 = V1 * G2 = 8932.875mV
			//500000mA : 8932.875mV = I1 : 10000mV
			//I1 = 559730.21mA
			//ratio = 559730210.0uA / 32768 = 17081.610413780558
			
			//RANGE2
			//ITN-600 600A:400mA = 250A:x -> x = 166.666mA
			//shuntV = 166.666mA * 5.0ohm = 833.33mV
			//V1 = 833.33mV * G1 = 5979.16667mV
			//V2 = V1 * G2 = 8932.875mV
			//250000mA : 8932.875mV = I1 : 10000mV
			//I1 = 279865.105mA
			//ratio = 279865105.01938uA / 32768 = 8540.80520689028
			
			if((range+1) == RANGE1) ratio = 17081.610413780558;
			else ratio = 8540.80520689028;
			break;
		/*case C_LGC_100V_500A_250A_100KW: //jhk_180111
		case C_LGC_100V_500A_250A_100KW_2: //jhk_180111
		case C_LGC_100V_500A_250A_100KW_3: //jhk_180111
		case C_LGC_100V_500A_250A_100KW_4: //jhk_180111
		case C_SDI_100V_500A_250A_100KW: //ktg_190118
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 500A:x -> x = 333.333mA
			//shuntV = 333.333mA * 2.5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//500000mA : 8932.875mV = I1 : 10000mV
			//I1 = 559730.21003876mA
			//ratio = 559730210.03uA / 32768 = 17081.610413780558
			
			//RANGE2
			//ITN600-S 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//250000mA : 8932.875mV = I1 : 10000mV
			//I1 = 279865.105mA
			//ratio = 279865105.019uA / 32768 = 8540.805206890279
			
			if((range+1) == RANGE1) ratio = 17081.610413780558;
			else ratio = 8540.805206890279;
			break;*/	//shh_231030
		case C_KTL_100V_500A_250A_50A_200KW:	//jhk_180718
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 500A:x -> x = 333.333mA
			//shuntV = 333.333mA * 2.5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//500000mA : 8932.875mV = I1 : 10000mV
			//I1 = 559730.21003876mA
			//ratio = 559730210.03uA / 32768 = 17081.610413780558
			
			//RANGE2
			//ITN600-S 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//250000mA : 8932.875mV = I1 : 10000mV
			//I1 = 279865.105mA
			//ratio = 279865105.019uA / 32768 = 8540.805206890279
			
			//RANGE3
			//ITN600-S 600A:400mA = 50A:x -> x = 33.333mA
			//shuntV = 33.333mA * 25ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.16667mV
			//V2 = V1 * G2 = 8932.875mV
			//50000mA : 8932.875mV = I1 : 10000mV
			//I1 = 55973.021mA
			//ratio = 55973021uA / 32768 = 1708.161041378056
			
			if((range+1) == RANGE1) ratio = 17081.610413780558;
			else if ((range+1) == RANGE2) ratio = 8540.805206890279;
			else ratio = 1708.161041378056;
			break;
		case C_SDI_100V_600A_100A_200KW:	//jhk_181107
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 600A:x -> x = 400mA
			//shuntV = 400mA * 2ohm = 800mV
			//V1 = 800mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//600000mA : 8575.56mV = I1 : 10000mV
			//I1 = 699662.76mA
			//ratio = 699662762.54845uA / 32768 = 21352.013017225697
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 12ohm = 800mV
			//V1 = 800mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//100000mA : 8575.56mV = I1 : 10000mV
			//I1 = 116610.46042474mA
			//ratio = 116610460.42474uA / 32768 = 3558.668836204283
			
			if((range+1) == RANGE1) ratio = 21352.013017225697;
			else ratio = 3558.668836204283;
			break;
		case C_UNINTECH_100V_600A_300A_60A_120KW:	//ktg_200103
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.2333
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 600A:x -> x = 400mA
			//shuntV = 400mA * 1.5ohm = 600mV
			//V1 = 600mV * G1 = 5540mV
			//V2 = V1 * G2 = 8276.76mV
			//600000mA : 8276.76mV = I1 : 10000mV
			//I1 = 724921.3460339553mA
			//ratio = 724921346.0339553uA / 32768 = 22122.84381207139
			
			//RANGE2
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 3ohm = 600mV
			//V1 = 600mV * G1 = 5540mV
			//V2 = V1 * G2 = 8276.76mV
			//300000mA : 8276.76mV = I1 : 10000mV
			//I1 = 362460.6730169777mA
			//ratio = 362460673.0169777uA / 32768 = 11061.4219060357
			
			//RANGE3
			//ITN600-S 600A:400mA = 60A:x -> x = 40mA
			//shuntV = 40mA * 15ohm = 600mV
			//V1 = 600mV * G1 = 5540mV
			//V2 = V1 * G2 = 8276.76mV
			//60000mA : 8276.76mV = I1 : 10000mV
			//I1 = 72492.13460339553mA
			//ratio = 72492134.60339553uA / 32768 = 2212.284381207139
			
			if((range+1) == RANGE1) ratio = 22122.84381207139;
			else if((range+1) == RANGE2) ratio = 11061.4219060357;
			else ratio = 2212.284381207139;
			break;
		/*case C_LGC_110V_600A_300A_100A_216KW:	//ljh_201103
		case C_LGC_110V_600A_300A_100A_216KW_2:	//ljh_201103
		case C_LGC_110V_600A_300A_100A_216KW_3:	//ljh_201103
		case C_LGC_110V_600A_300A_100A_216KW_4:	//ljh_201103
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.7kohm = 7.41558
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN600-S 300A:200mA = 600A:x -> x = 400mA
			//ShuntV = 400mA * 2ohm = 800mV
			//V1 = 800mV * G1 = 5932.4675324675mV
			//V2 = V1 * G2 = 8863.1064935065mV
			//600000mA : 8863.1064935065mV = I1 : 10000mV
			//I1 = 676963.545952637mA
			//ratio = 676963545.952637uA / 32768 = 20659.2879013866
			
			//RANGE2
			//ITN600-S 300A:200mA = 300A:x -> x = 200mA
			//ShuntV = 200mA * 4ohm = 800mV
			//V1 = 800mV * G1 = 5932.4675324675mV
			//V2 = V1 * G2 = 8863.1064935065mV
			//300000mA : 8863.1064935065mV = I1 : 10000mV
			//I1 = 338481.7729763185mA
			//ratio = 338481772.9763185uA / 32768 = 10329.6439506933
			
			//RANGE3
			//ITN600-S 300A:200mA = 100A:x -> x = 66.667mA
			//ShuntV = 66.667mA * 12ohm = 800mV
			//V1 = 800mV * G1 = 5932.4675324675mV
			//V2 = V1 * G2 = 8863.1064935065mV
			//100000mA : 8863.1064935065mV = I1 : 10000mV
			//I1 = 112827.25765877286mA
			//ratio = 112827257.65877286uA / 32768 = 3443.2146502311
			if((range+1) == RANGE1) ratio = 20659.2879013866;
			else if((range+1) == RANGE2) ratio = 10329.6439506933;
			else ratio = 3443.2146502311;
			break;	*/
		/*case C_LGC_110V_600A_300A_100A_264KW:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_2:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_3:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_4:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_5:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_6:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_7:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_8:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_9:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_10:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_11:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_12:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_13:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_14:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_15:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_16:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_17:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_18:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_19:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_20:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_21:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_22:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_23:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_24:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_25:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_26:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_27:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_28:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_29:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_30:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_31:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_32:    //phb_220216
		case C_LGC_110V_600A_300A_100A_264KW_33:	//sec_221031
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN600-S 300A:200mA = 600A:x -> x = 400mA
			//ShuntV = 400mA * 2.5ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//600000mA : 8874.36mV = I1 : 10000mV
			//I1 = 676105.0937757765mA
			//ratio = 676105093.7757765uA / 32768 = 20633.09002001
			
			//RANGE2
			//ITN600-S 300A:200mA = 300A:x -> x = 200mA
			//ShuntV = 200mA * 5ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//300000mA : 8874.36mV = I1 : 10000mV
			//I1 = 338052.5468878883mA
			//ratio = 338052546.8878883uA / 32768 = 10316.545010006
			
			//RANGE3
			//ITN600-S 300A:200mA = 100A:x -> x = 66.667mA
			//ShuntV = 66.667mA * 15ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//100000mA : 8874.36mV = I1 : 10000mV
			//I1 = 112684.1822959628mA
			//ratio = 112684,182.2959628uA / 32768 = 3438.84833666
			if((range+1) == RANGE1) ratio = 20633.09002001;
			else if((range+1) == RANGE2) ratio = 10316.545010006;
			else ratio = 3438.84833666;
			break;*/
		case C_KATECH_120V_150A_100A_50A_18KW:	//jhkw_130621
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 150A:x -> x = 75mA
			//shuntV = 75mA * 10ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.0285714mV
			//150000mA : 9028.0285714mV = I1 : 10000mV
			//I1 = 166149.2304917mA
			//ratio = 166149230.4917uA / 32768 = 5070.47212193929
			
			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//50000mA : 9028.02857142857mV = I1 : 10000mV
			//I1 = 55383.07683mA
			//ratio = 55383076.8305689uA / 32768 = 1690.1573739797646
			
			if((range+1) == RANGE1) ratio = 5070.47212193929;
			else ratio = 1690.1573739797646;
			break;
		/*case C_LGC_120V_250A_50A_60KW:	//jhk_160617
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714285714
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//300000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.94424387858
			
			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//50000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 55383.0768305689266mA
			//ratio = 55383076.83056892uA / 32768 = 1690.157373979764606
						
			if((range+1) == RANGE1) ratio = 10140.944243;
			else ratio = 1690.15737397976;
			break;*/	//shh_231027
		case C_LGC_120V_250A_50A_60KW_2:	//jhk_161101
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233333333
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 150mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.53841358mA
			//ratio = 289968538.41358uA / 32768 = 8849.137524828556
			
			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 25ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//50000mA : 8621.625mV = I1 : 10000mV
			//I1 = 57993.7076827164mA
			//ratio = 57993707.6827164uA / 32768 = 1769.827504965711
			
			if((range+1) == RANGE1) ratio = 8849.137524828556;
			else ratio = 1769.827504965711;
			break;
		case C_SKI_120V_400A_100A_192KW:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_2:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_3:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_4:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_5:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_6:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_7:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_8:	//jhk_170628
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.0kohm = 8.057142857
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 400A:x -> x = 266.7mA
			//shuntV = 266.7mA * 2.5ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5371.42857mV
			//V2 = V1 * G2 = 8024.9142857mV
			//400000mA : 8024.9142857mV = I1 : 10000mV
			//I1 = 498447.691475mA
			//ratio = 498447691.475uA / 32768 = 15211.416365817881
			
			//RANGE2
			//ITN-600 600A:400mA = 100A:x -> x = 66.7mA
			//shuntV = 66.7mA * 10ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5371.42857mV
			//V2 = V1 * G2 = 8024.9142857mV
			//100000mA : 8024.9142857mV = I1 : 10000mV
			//I1 = 124611.92286878mA
			//ratio = 124611922.86878uA / 32768 = 3802.85409145447
			
			if((range+1) == RANGE1) ratio = 15211.416365817881;
			else ratio = 3802.85409145447;
			break;
		case C_SKI_120V_400A_200A_100A_50A_192KW:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_2:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_3:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_4:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_5:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_6:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_7:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_8:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_9:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_10:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_11:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_12:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_13:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_14:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_15:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_16:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_17:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_18:	//jhk_180206
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677419
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 400A:x -> x = 266.7mA
			//shuntV = 266.7mA * 2.5ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//400000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 447834.50348mA
			//ratio = 447834503.48uA / 32768 = 13666.82444706885
			
			//RANGE2
			//ITN-600 600A:400mA = 200A:x -> x = 133.33mA
			//shuntV = 133.33mA * 5.0ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//200000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 223917.25174mA
			//ratio = 223917251.74uA / 32768 = 6833.412223534425
			
			//RANGE3
			//ITN-600 600A:400mA = 100A:x -> x = 66.7mA
			//shuntV = 66.7mA * 10.0ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//100000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 111958.62587mA
			//ratio = 111958625.87uA / 32768 = 3416.7061117672127
			
			//RANGE4
			//ITN-600 600A:400mA = 50A:x -> x = 33.33mA
			//shuntV = 33.33mA * 20.0ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//50000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 55979.312935mA
			//ratio = 55979312.935uA / 32768 = 1708.353055883606
			
			if((range+1) == RANGE1) ratio = 13666.82444706885;
			else if ((range+1) == RANGE2) ratio = 6833.412223534425;
			else if ((range+1) == RANGE3) ratio = 3416.7061117672127;
			else ratio = 1708.353055883606;
			break;
		case C_NORTHVOLT_60V_400A_200A_96KW:			//khj_191203
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677419
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 400A:x -> x = 266.7mA
			//shuntV = 266.7mA * 2.5ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//400000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 447834.50348mA
			//ratio = 447834503.48uA / 32768 = 13666.82444706885
			
			//RANGE2
			//ITN-600 600A:400mA = 200A:x -> x = 133.33mA
			//shuntV = 133.33mA * 5.0ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//200000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 223917.25174mA
			//ratio = 223917251.74uA / 32768 = 6833.412223534425
			
			if((range+1) == RANGE1) ratio = 13666.82444706885;
			else ratio = 6833.412223534425;
			break;			
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
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 400A:x -> x = 266.7mA
			//shuntV = 266.7mA * 2.5ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5371.4285714mV
			//V2 = V1 * G2 = 8024.914285714mV
			//400000mA : 8024.914285714mV = I1 : 10000mV
			//I1 = 498447.69147512mA
			//ratio = 498447691.47512uA / 32768 = 15211.416365818
			
			//RANGE2
			//ITN-600 600A:400mA = 200A:x -> x = 133.33mA
			//shuntV = 133.33mA * 5.0ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5371.4285714mV
			//V2 = V1 * G2 = 8024.914285714mV
			//200000mA : 8024.914285714mV = I1 : 10000mV
			//I1 = 249223.8457376mA
			//ratio = 249223845.7376uA / 32768 = 7605.70818290894
			
			//RANGE3
			//ITN-600 600A:400mA = 100A:x -> x = 66.7mA
			//shuntV = 66.7mA * 10.0ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5371.4285714mV
			//V2 = V1 * G2 = 8024.914285714mV
			//100000mA : 8024.914285714mV = I1 : 10000mV
			//I1 = 124611.922868mA
			//ratio = 124611922.868uA / 32768 = 3802.8540914544704
			
			if((range+1) == RANGE1) ratio = 15211.416365818;
			else if ((range+1) == RANGE2) ratio = 7605.70818290894;
			else ratio = 3802.8540914544704;
			break;
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
		//case C_TEST_120V_400A_200A_100A_192KW:		//shh_200723
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677419
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN-600 600A:400mA = 400A:x -> x = 266.7mA
			//shuntV = 266.7mA * 2.5ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//400000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 447834.50348mA
			//ratio = 447834503.48uA / 32768 = 13666.82444706885

			//RANGE2
			//ITN-600 600A:400mA = 200A:x -> x = 133.33mA
			//shuntV = 133.33mA * 5.0ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//200000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 223917.25174mA
			//ratio = 223917251.74uA / 32768 = 6833.412223534425

			//RANGE3
			//ITN-600 600A:400mA = 100A:x -> x = 66.7mA
			//shuntV = 66.7mA * 10.0ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//100000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 111958.62587mA
			//ratio = 111958625.87uA / 32768 = 3416.7061117672127
			
			if((range+1) == RANGE1) ratio = 13666.82444706885;
			else if ((range+1) == RANGE2) ratio = 6833.412223534425;
			else ratio = 3416.7061117672127;
			break;
		case C_SDI_120V_500A_250A_50A_120KW:	//ktg_190201
		case C_SDI_120V_500A_250A_50A_120KW_2:	//ktg_190201
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 500A:x -> x = 333.333mA
			//shuntV = 333.3mA * 2.5ohm = 833.3mV
			//V1 = 833.3mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//500000mA : 8932.875mV = I1 : 10000mV
			//I1 = 559730.21004mA
			//ratio = 559730210.04uA / 32768 = 17081.610414
			
			//RANGE2
			//ITN-600 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 5.0ohm = 833.3mV
			//V1 = 833.3mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//250000mA : 8932.875mV = I1 : 10000mV
			//I1 = 223892.08401mA
			//ratio = 223892084.01uA / 32768 = 6832.644165
			
			//RANGE3
			//ITN-600 600A:400mA = 50A:x -> x = 33.333mA
			//shuntV = 33.333mA * 25.0ohm = 833.3mV
			//V1 = 833.3mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//50000mA : 8932.875mV = I1 : 10000mV
			//I1 = 55973.021004mA
			//ratio = 55973021.004uA / 32768 = 1708.16104
			
			if((range+1) == RANGE1) ratio = 17081.610414;
			else if ((range+1) == RANGE2) ratio = 6832.644165;
			else ratio = 1708.16104;
			break;
		case C_CSR_125V_200A_100A_50A_50KW:		//jhkw_121224
		case C_CSR_125V_200A_100A_50A_50KW_2:	//jhkw_121224
		case C_CSR_125V_200A_100A_50A_50KW_3:	//jhkw_121224
		case C_CSR_125V_200A_100A_50A_50KW_4:	//jhkw_121224
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.977777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.8888888889mV
			//V2 = V1 * G2 = 8947.4mV
			//200000mA : 8947.4mV = I1 : 10000mV
			//I1 = 223528.62284015468mA
			//ratio = 223528622.84015468uA / 32768 = 6821.552210698
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.8888888889mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142007734mA
			//ratio = 111764311.42007734uA / 32768 = 3410.7761053490
			
			//RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 20ohm = 500mV
			//V1 = 500mV * G1 = 5988.88888888889mV
			//V2 = V1 * G2 = 8947.4mV
			//50000mA : 8947.4mV = I1 : 10000mV
			//I1 = 55882.1557100mA
			//ratio = 55882155.7100uA / 32768 = 1705.3880526745
			
			if((range+1) == RANGE1) ratio = 6821.552210698;
			else if ((range+1) == RANGE2) ratio = 3410.7761053490;
			else ratio = 1705.3880526745;
			break;
		case C_HUOJU_125V_200A_100A_50KW:	//jhk_151107
		case C_HUOJU_125V_200A_100A_50KW_2:	//jhk_151107
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.977777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.8888888889mV
			//V2 = V1 * G2 = 8947.4mV
			//200000mA : 8947.4mV = I1 : 10000mV
			//I1 = 223528.62284015468mA
			//ratio = 223528622.84015468uA / 32768 = 6821.552210698
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.8888888889mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142007734mA
			//ratio = 111764311.42007734uA / 32768 = 3410.7761053490
			
			if((range+1) == RANGE1) ratio = 6821.552210698;
			else ratio = 3410.7761053490;
			break;
		case C_GEELY_130V_300A_150A_78KW:	//ljh_210322
		case C_GEELY_130V_300A_150A_78KW_2:	//ljh_210322
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.977777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 2.5ohm = 500mV
			//V1 = 500mV * G1 = 5988.8888888889mV
			//V2 = V1 * G2 = 8947.4mV
			//300000mA : 8947.4mV = I1 : 10000mV
			//I1 = 335292.934260232mA
			//ratio = 335292934.260232uA / 32768 = 10232.32831604 
			
			//RANGE2
			//ITN600-S 600A:400mA = 150A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.8888888889mV
			//V2 = V1 * G2 = 8947.4mV
			//150000mA : 8947.4mV = I1 : 10000mV
			//I1 = 167646.467130116mA
			//ratio = 167646467.130116uA / 32768 = 5116.16415802
			
			if((range+1) == RANGE1) ratio = 10232.328316;
			else ratio = 5116.164158;
			break;
		case C_GEELY_130V_600A_300A_156KW:	//ljh_210323
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN600-S 600A:400mA = 600A:x -> x = 400mA
			//shuntV = 400mA * 2.5ohm = 1000mV
			//V1 = shuntV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//600000mA : 8874.36mV = I1 : 10000mV
			//I1 = 676105.093776mA
			//ratio = 676105093.776uA / 32768 = 20633.09002
			
			//RANGE2
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 5ohm = 1000mV
			//V1 = shuntV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//300000mA : 8874.36mV = I1 : 10000mV
			//I1 = 338052.546888mA
			//ratio = 338052546.888uA / 32768 = 10316.54501

			if((range+1) == RANGE1) ratio = 20633.09002;
			else ratio = 10316.54501;
			break;
		case C_CTP_150V_150A_100A_90KW:	//ktg_191022
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 150A:x -> x = 100mA
			//shuntV = 100mA * 10ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//150000mA : 8874.36mV = I1 : 10000mV
			//I1 = 169026.2734439mA
			//ratio = 169026273.4439uA / 32768 = 5158.272505003
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 15ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//100000mA : 8874.36mV = I1 : 10000mV
			//I1 = 112684.18229596mA
			//ratio = 112684182.29596uA / 32768 = 3438.84833667
			
			if((range+1) == RANGE1) ratio = 5158.272505003;
			else ratio = 3438.84833667;
			break;
		case C_SBL_150V_250A_10A_38KW: //csk_111007
		case C_SBL_150V_250A_10A_75KW:
		case C_SBL_150V_250A_10A_75KW_2:
		case C_SBL_150V_250A_10A_75KW_3:
		case C_SBL_150V_250A_10A_75KW_4: //csk_111007
		case C_SBL_150V_250A_10A_75KW_5: //csk_111007
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.5384mA
			//ratio = 289968538.4uA / 32768 = 8849.13752482855

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 125ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333mV
			//V2 = V1 * G2 = 8621.625mV
			//10000mA : 8621.625mV = I1 : 10000mV
			//I1 = 11598.741537mA
			//ratio = 11598741.537uA / 32768 = 353.965501
			if((range+1) == RANGE1) ratio = 8849.1375248;
			else ratio = 353.965501;
			break;
		/*case C_LGC_150V_250A_100A_75KW:		//ktg_190507
		case C_LGC_150V_250A_100A_150KW:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_2:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_3:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_4:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_5:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_6:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_7:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_8:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_9:	//ktg_190514
		case C_LGC_150V_250A_100A_150KW_10:	//ktg_190514
		case C_LGC_150V_250A_100A_150KW_11:	//ktg_190514
		case C_LGC_150V_250A_100A_150KW_12:	//ktg_190514
		*///shh_231024
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//250000mA : 8932.875mV = I1 : 10000mV
			//I1 = 279865.105mA
			//ratio = 279865105.019uA / 32768 = 8540.805206890279
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 12.5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.16667mV
			//V2 = V1 * G2 = 8932.875mV
			//100000mA : 8932.875mV = I1 : 10000mV
			//I1 = 111946.042mA
			//ratio = 111946042.007uA / 32768 = 3416.322082756112
		/*	
			if((range+1) == RANGE1) ratio = 8540.805206890279;
			else ratio = 3416.322082756112;
			break;
		*/
		case C_TUV_150V_300A_100A_180KW:	//ktg_190429
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 4ohm = 800mV
			//V1 = 800mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//300000mA : 8575.56mV = I1 : 10000mV
			//I1 = 349831.38127422582315mA
			//ratio = 349831381.27422582315uA / 32768 = 10676.00650861
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 12ohm = 800mV
			//V1 = 800mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//100000mA : 8575.56mV = I1 : 10000mV
			//I1 = 116610.460424742mA
			//ratio = 116610460.424742uA / 32768 = 3558.6688362
			
			if((range+1) == RANGE1) ratio = 10676.00650861;
			else ratio = 3558.6688362;
			break;
		/*case C_LGC_150V_350A_100A_105KW:	//ktg_190510
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 12kohm = 5.11667
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 350A:x -> x = 233.33mA
			//shuntV = 233.33mA * 5ohm = 1166.667mV
			//V1 = 1166.667mV * G1 = 5969.445mV
			//V2 = V1 * G2 = 8918.35mV
			//350000mA : 8918.35mV = I1 : 10000mV
			//I1 = 392449.2759310859mA
			//ratio = 392449275.9310859uA / 32768 = 11976.601438327
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 17.5ohm = 1166.667mV
			//V1 = 1166.667mV * G1 = 5969.445mV
			//V2 = V1 * G2 = 8918.35mV
			//100000mA : 8918.35mV = I1 : 10000mV
			//I1 = 112128.36455174mA
			//ratio = 112128364.55174uA / 32768 = 3421.886125236
			
			if((range+1) == RANGE1) ratio = 11976.601438327;
			else ratio = 3421.886125236;
			break;*/	//shh_231024
		/*case C_LGC_150V_350A_100A_210KW:	//ktg_190421
		case C_LGC_150V_350A_100A_210KW_2:	//ktg_190421
		case C_LGC_150V_350A_100A_210KW_3:	//ktg_190421
		case C_LGC_150V_350A_100A_210KW_4:	//ktg_190421
		case C_LGC_200V_350A_100A_280KW:	//ktg_190425
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.17kohm = 12.84652278
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 350A:x -> x = 233.33mA
			//shuntV = 233.33mA * 2ohm = 466.667mV
			//V1 = 466.667mV * G1 = 5995.0439648mV
			//V2 = V1 * G2 = 8956.59568345mV
			//350000mA : 8956.59568345mV = I1 : 10000mV
			//I1 = 390773.472834777602mA
			//ratio = 390773472.834777602uA / 32768 = 11925.4599864
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 7ohm = 466.667mV
			//V1 = 466.667mV * G1 = 5995.0439648mV
			//V2 = V1 * G2 = 8956.59568345mV
			//100000mA : 8956.59568345mV = I1 : 10000mV
			//I1 = 111649.563667079mA
			//ratio = 111649563.667079uA / 32768 = 3407.27428183
			
			if((range+1) == RANGE1) ratio = 11925.4599864;
			else ratio = 3407.27428183;
			break;*/	//shh_231025
		case C_NORTHVOLT_200V_400A_200A_160KW:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_2:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_3:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_4:	//ktg_191203
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677419
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 400A:x -> x = 266.7mA
			//shuntV = 266.7mA * 2.5ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//400000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 447834.50348mA
			//ratio = 447834503.48uA / 32768 = 13666.82444706885
			
			//RANGE2
			//ITN-600 600A:400mA = 200A:x -> x = 133.33mA
			//shuntV = 133.33mA * 5.0ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//200000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 223917.25174mA
			//ratio = 223917251.74uA / 32768 = 6833.412223534425
			
			if((range+1) == RANGE1) ratio = 13666.82444706885;
			else ratio = 6833.412223534425;
			break;			
		/*case C_LGC_150V_450A_200A_270KW:	//ktg_190421
		case C_LGC_150V_450A_200A_270KW_2:	//ktg_190421
		case C_LGC_150V_450A_200A_270KW_3:	//ktg_190421
		case C_LGC_150V_450A_200A_270KW_4:	//ktg_190421
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.49kohm = 9.9981785
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 450A:x -> x = 300mA
			//shuntV = 300mA * 2ohm = 600mV
			//V1 = 600mV * G1 = 5998.9071038mV
			//V2 = V1 * G2 = 8962.367213115mV
			//450000mA : 8962.367213115mV = I1 : 10000mV
			//I1 = 502099.4892303775mA
			//ratio = 502099489.2303775uA / 32768 = 15322.860389
			
			//RANGE2
			//ITN600-S 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 4.5ohm = 600mV
			//V1 = 600mV * G1 = 5998.9071038mV
			//V2 = V1 * G2 = 8962.367213115mV
			//200000mA : 8962.367213115mV = I1 : 10000mV
			//I1 = 223155.3285468mA
			//ratio = 223155328.5468uA / 32768 = 6810.160172938
			
			if((range+1) == RANGE1) ratio = 15322.860389;
			else ratio = 6810.160172938;
			break;*/	//shh_231025
		/*case C_LGC_150V_600A_300A_100A_300KW:	//shh_210819
		case C_LGC_150V_600A_300A_100A_300KW_2:	//shh_210819
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 600A:x -> x = 400mA
			//shuntV = 400mA * 2.5ohm= 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//600000mA : 8874.36mV = I1 : 10000mV
			//I1 = 676105.0937757765mA
			//ratio = 676105093.7757765uA / 32768 = 20633.09002001

			//RANGE2
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 5ohm= 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//300000mA : 8874.36mV = I1 : 10000mV
			//I1 = 338052.546887888mA
			//ratio = 338052546.887888uA / 32768 = 10316.54501

			//RANGE3
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 15ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//100000mA : 8874.36mV = I1 : 10000mV
			//I1 = 112684.18229596mA
			//ratio = 112684182.29596uA / 32768 = 3438.84833667

			if((range+1) == RANGE1) ratio = 20633.09002001;
			else if ((range+1) == RANGE2) ratio = 10316.54501;
			else ratio = 3438.84833667;
			break;*/
		case C_KORID_160V_500A_100A_80KW:	//ktg_190412
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 500A:x -> x = 333.333mA
			//shuntV = 333.333mA * 2.5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//500000mA : 8932.875mV = I1 : 10000mV
			//I1 = 559730.21004mA
			//ratio = 559730210.04uA / 32768 = 17081.610413781
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 12.5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.16667mV
			//V2 = V1 * G2 = 8932.875mV
			//100000mA : 8932.875mV = I1 : 10000mV
			//I1 = 111946.042mA
			//ratio = 111946042.007uA / 32768 = 3416.322082756112
			
			if((range+1) == RANGE1) ratio = 17081.610413781;
			else ratio = 3416.322082756112;
			break;
		case C_LGE_150V_400A_60KW:	//jhk_170721
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677419
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 400A:x -> x = 266.667mA
			//shuntV = 266.667mA * 2.5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//400000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 447834.50348mA
			//ratio = 447834503.48uA / 32768 = 13666.82444706885
			
			ratio = 13666.82444706885;
			break;
		case C_HYUNDAICE_150V_500A_250A_150KW:	//ktg_190520
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 500A:x -> x = 333.333mA
			//shuntV = 333.3mA * 2.5ohm = 833.3mV
			//V1 = 833.3mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//500000mA : 8932.875mV = I1 : 10000mV
			//I1 = 559730.21004mA
			//ratio = 559730210.04uA / 32768 = 17081.610414
			
			//RANGE2
			//ITN-600 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 5.0ohm = 833.3mV
			//V1 = 833.3mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//250000mA : 8932.875mV = I1 : 10000mV
			//I1 = 279865.10501mA
			//ratio = 279865105.01uA / 32768 = 8540.80520689
			
			if((range+1) == RANGE1) ratio = 17081.610414;
			else ratio = 8540.80520689;
			break;
		case C_KTL_150V_600A_300A_200KW:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_2:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_3:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_4:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_5:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_6:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_7:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_8:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_9:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_10:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_11:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_12:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_13:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_14:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_15:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_16:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_17:	//ktg_211002
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600s 600A:400mA = 600A:x -> x = 400mA
			//shuntV = 400mA * 2.5ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//600000mA : 8874.36mV = I1 : 10000mV
			//I1 = 676105.0937757765067mA
			//ratio = 676105093.7757765067uA / 32768 = 20633.09002
			
			//RANGE2
			//ITN600s 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 5ohm = 1000mV
			//V1 = 1000mV * G1 = 6000mV
			//V2 = V1 * G2 = 8874.36mV
			//300000mA : 8874.36mV = I1 : 10000mV
			//I1 = 338052.546887888mA
			//ratio = 338052546.887888uA / 32768 = 10316.54501
			
			if((range+1) == RANGE1) ratio = 20633.09002;
			else ratio = 10316.54501;
			break;
		case C_SEBANG_200V_100A_10A:
		case C_SEBANG_200V_100A_10A_2:
		case C_SEBANG_200V_100A_10A_3:
		case C_SEBANG_200V_100A_10A_4:
		case C_SEBANG_200V_100A_10A_5:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//100000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 111764.3114mA
			//ratio = 111764311.4uA / 32768 = 3410.776105

			//RANGE1
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 100ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//10000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 11176.43114mA
			//ratio = 11176431.14uA / 32768 = 341.0776105
			if((range+1) == RANGE1) ratio = 3410.776105;
			else ratio = 341.0776105;
			break;
		/*case C_LGC_200V_150A_50A_60KW:	//jhk_170905
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.8kohm = 8.26470588
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 150A:x -> x = 75mA
			//shuntV = 75mA * 10ohm = 750mV
			//V1 = 750mV * G1 = 6198.5294mV
			//V2 = V1 * G2 = 9260.60294mV
			//150000mA : 9260.60294mV = I1 : 10000mV
			//I1 = 161976.49435226mA
			//ratio = 161976494.35226uA / 32768 = 4943.130320808739

			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 6198.5294mV
			//V2 = V1 * G2 = 9260.60294mV
			//50000mA : 9260.60294mV = I1 : 10000mV
			//I1 = 53992.164784mA
			//ratio = 53992164.784uA / 32768 = 1647.710106936246
			if((range+1) == RANGE1) ratio = 4943.130320808739;
			else ratio = 1647.710106936246;
			break;*/	//shh_231025
		case C_SEBANG_200V_200A_10A:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//200000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 223528.6228mA
			//ratio = 223528622.8uA / 32768 = 6821.552209

			//RANGE1
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 100ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//10000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 11176.43114mA
			//ratio = 11176431.14uA / 32768 = 341.0776105
			if((range+1) == RANGE1) ratio = 6821.552209;
			else ratio = 341.0776105;
			break;
		case C_ECOCAR_200V_200A_100A_50A_40KW:	//jhkw_130827
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.888888889mV
			//V2 = V1 * G2 = 8947.4mV
			//200000mA : 8947.4mV = I1 : 10000mV
			//I1 = 223528.62284015468mA
			//ratio = 223528622.84015468uA / 32768 = 6821.552210698

			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.888888889mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142007734mA
			//ratio = 111764311.42007734uA / 32768 = 3410.776105349
			
			//RANGE1
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 20ohm = 500mV
			//V1 = 500mV * G1 = 5988.888888889mV
			//V2 = V1 * G2 = 8947.4mV
			//50000mA : 8947.4mV = I1 : 10000mV
			//I1 = 55882.15571mA
			//ratio = 55882155.71uA / 32768 = 1705.3880526745
			
			if((range+1) == RANGE1) ratio = 6821.552210698;
			else if ((range+1) == RANGE2) ratio = 3410.776105349;
			else ratio = 1705.3880526745;
			break;
		/*case C_LGC_200V_300A_100A_50A_10A_120KW:	//jhkw_120813
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714285714
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//300000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.94424387858
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//100000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 110766.1536611mA
			//ratio = 110766153.661137853uA / 32768 = 3380.3147479595
			
			//RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//50000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 55383.0768305689266mA
			//ratio = 55383076.83056892uA / 32768 = 1690.157373979764606
						
			//RANGE4
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 150ohm = 750mV
			//V1 = 750mV * G1 = 6042.85714285714mV
			//V2 = V1 * G2 = 9028.02857142857mV
			//10000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 11076.615366113785mA
			//ratio = 11076615.366113785uA / 32768 = 338.0314747959529212

			if((range+1) == RANGE1) ratio = 10140.944243;
			else if ((range+1) == RANGE2) ratio = 3380.3147479595;
			else if ((range+1) == RANGE3) ratio = 1690.15737397976;
			else ratio = 338.03147479595;
			break;*/
		case C_UTP_200V_500A_250A_100KW:	//ljh_201119
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN600-S 300A:150mA = 500A:x -> x = 333.3333mA
			//shuntV = 333.3333mA * 3ohm = 1000mV
			//V1 = shuntV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//500000000mA : 8874.36mV = I1 : 10000mV
			//I1 = 563420.9114798mA
			//ratio = 563420911.47981375558uA / 32768 = 17194.24168334393
			
			//RANGE2
			//ITN600-S 300A:150mA = 250A:x -> x = 166.667mA
			//shuntV  166.667mA * 6ohm = 1000mV
			//V1 = shuntV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//250000000mA : 8874.36mV = I1 : 10000mV
			//I1 = 281710.4557399mA
			//ratio = 281710455.739906877792uA / 32768 = 8597.120841672
			if((range+1) == RANGE1) ratio = 17194.24168334393;
			else ratio = 8597.120841672;
			break;
		case C_OCI_220V_250A_100A_110KW:		//jhk_131220
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.23333333
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.5384mA
			//ratio = 289968538.413582uA / 32768 = 8849.137524828556
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 12.5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//100000mA : 8621.625mV = I1 : 10000mV
			//I1 = 115987.41536543mA
			//ratio = 115987415.36543uA / 32768 = 3539.65500993
			
			if((range+1) == RANGE1) ratio = 8849.137524828556;
			else ratio = 3539.65500993;
			break;
		//case C_LGC_220V_600A_300A_100A_528KW:       //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_2:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_3:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_4:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_5:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_6:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_7:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_8:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_9:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_10:    //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_11:    //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_12:    //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_13:    //phb_220228
		/*   //10000000uV / 32768 = 305.1757812uV
           //AMP -> AD620A
           //G1 = 1 + 49.4Kohm / 10.0Kohm = 5.940
           //G2 = 1 + 49.4Kohm / 100Kohm = 1.494


           //RANGE1
           //ITN600-S 1500 : 1 = 600A : 400.000mA
           //shuntV = 400.000mA * 2.5ohm = 1000.000mV
           //V1 = 1000.000mV * G1 = 5940.000mV
           //V2 = V1 * G2 = 8874.360mV
           //600000mA : 8874.360 = I1 : 10000mV
           //I1 = 676105.09377578mA
           //ratio = 676105093.77577651uA / 32768 = 20633.09002001


           //RANGE2
           //ITN600-S 1500 : 1 = 300A : 200.000mA
           //shuntV = 200.000mA * 5.0ohm = 1000.000mV
           //V1 = 1000.000mV * G1 = 5940.000mV
           //V2 = V1 * G2 = 8874.360mV
           //300000mA : 8874.360 = I1 : 10000mV
           //I1 = 338052.54688789mA
           //ratio = 338052546.88788825uA / 32768 = 10316.54501001


           //RANGE3
           //ITN600-S 1500 : 1 = 100A : 66.667mA
           //shuntV = 66.667mA * 15.0ohm = 1000.000mV
           //V1 = 1000.000mV * G1 = 5940.000mV
           //V2 = V1 * G2 = 8874.360mV
           //100000mA : 8874.360 = I1 : 10000mV
           //I1 = 112684.18229596mA
           //ratio = 112684182.29596275uA / 32768 = 3438.84833667


           if((range+1) == RANGE1) ratio = 20633.09002001;
           else if((range+1) == RANGE2) ratio = 10316.54501001;
           else ratio = 3438.84833667;
           break;*/
		case C_LGC_250V_120A_10A:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 3kohm = 17.46666667
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 120A:x -> x = 60mA
			//shuntV = 60mA * 10ohm = 600mV
			//V1 = 600000uV * G1 = 5989090.909uV
			//V2 = V1 * G2 = 8983636.364uV
			//120000000uA : 8983636.364uV = I1 : 10000000uV
			//I1 = 133576199.1uA
			//ratio = 133576199.1uA / 32768 = 4076.422092

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 100ohm = 500mV
			//V1 = 500000uV * G1 = 4990909.091uV
			//V2 = V1 * G2 = 7456418.182uV
			//10000000uA : 7456418.182uV = I1 : 10000000uV
			//I1 = 13411264.97uA
			//ratio = 13411264.97uA / 32768 = 409.2793265
			if((range+1) == RANGE1) ratio = 4076.422092;
			else ratio = 409.2793265;
			break;
		/*case C_LGC_250V_150A_100A_75KW:		//ktg_190510	//shh_231025
		case C_LGC_250V_150A_100A_150KW:	//ktg_190513
		case C_LGC_250V_150A_100A_150KW_2:	//ktg_190513
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.0kohm = 10.88
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 150A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5440mV
			//V2 = V1 * G2 = 8127.36mV
			//150000mA : 8127.36mV = I1 : 10000mV
			//I1 = 184561.776517836mA
			//ratio = 184561776.517836uA / 32768 = 5632.37843377
			
			//RANGE2
			//ITN-600 600A:400mA = 100A:x -> x = 66.7mA
			//shuntV = 66.7mA * 7.5ohm = 500mV
			//V1 = 500mV * G1 = 5440mV
			//V2 = V1 * G2 = 8127.36mV
			//100000mA : 8127.36mV = I1 : 10000mV
			//I1 = 123041.184345224mA
			//ratio = 123041184.345224uA / 32768 = 3754.91895585
			
			if((range+1) == RANGE1) ratio = 5632.37843377;
			else ratio = 3754.91895585;
			break;*/	//shh_231025
		/*case C_LGC_250V_350A_100A_175KW:	//jhk_171023
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.5kohm = 9.981818
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 350A:x -> x = 233.33mA
			//shuntV = 233.33mA * 2.5ohm = 583.33333mV
			//V1 = 583.33333mV * G1 = 5822.72727mV
			//V2 = V1 * G2 = 8699.154545mV
			//350000mA : 8699.154545mV = I1 : 10000mV
			//I1 = 402337.949mA
			//ratio = 402337949.2uA / 32768 = 12278.379798794449
			
			//RANGE2
			//ITN-600 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 9.1ohm = 606.667mV
			//V1 = 606.667mV * G1 = 6055.6363636mV
			//V2 = V1 * G2 = 9047.120727mV
			//100000mA : 9047.120727mV = I1 : 10000mV
			//I1 = 110532.403639mA
			//ratio = 110532403.639uA / 32768 = 3373.181263405068
			
			if((range+1) == RANGE1) ratio = 12278.379798794449;
			else ratio = 3373.181263405068;
			break;*/	//shh_231025
		case C_LGC_250V_350A_250A_100A_175KW: //jhk_181108
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 12kohm = 5.11667
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 350A:x -> x = 233.333mA
			//shuntV = 233.333mA * 5ohm = 1166.666667mV
			//V1 = 1166.666667mV * G1 = 5969.44444mV
			//V2 = V1 * G2 = 8918.35mV
			//350000mA : 8918.35mV = I1 : 10000mV
			//I1 = 392449.27593mA
			//ratio = 392449275.93uA / 32768 = 11976.601438326596

			//RANGE2
			//ITN600-S 600A:400mA = 250A:x -> x = 166.66667mA
			//shuntV = 166.66667mA * 7ohm = 1166.667mV
			//V1 = 1166.667mV * G1 = 5969.44444mV
			//V2 = V1 * G2 = 8918.35mV
			//250000mA : 8918.35mV = I1 : 10000mV
			//I1 = 280320.911379mA
			//ratio = 280320911.379uA / 32768 = 8554.715313090426
			
			//RANGE3
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 17.5ohm = 1166.6667mV
			//V1 = 1166.6667mV * G1 = 5969.444444mV
			//V2 = V1 * G2 = 8918.35mV
			//100000mA : 8918.35mV = I1 : 10000mV
			//I1 = 112128.36455mA
			//ratio = 112128364.55uA / 32768 = 3421.88612523617
			if((range+1) == RANGE1) ratio = 11976.601438326596;
			else if ((range+1) == RANGE2) ratio = 8554.715313090426;
			else ratio = 3421.88612523617;
			break;
		case C_SEBANG_300V_250A_10A_150KW:	//CSK wr
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//250000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 289979.00696979mA
			//ratio = 289979006.9uA / 32768 = 8849.45699768066

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 125ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//10000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 11599.160279mA
			//ratio = 11599160.278uA / 32768 = 353.965501
			if((range+1) == RANGE1) ratio = 8849.1375248;
			else ratio = 353.978280;
			break;
		case C_KTC_300V_100V_250A_100A_25A_150KW:	//jhk_151107
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.8kohm = 9.51724
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5948.275862mV
			//V2 = V1 * G2 = 8886.724137931mV
			//250000mA : 8886.724137931mV = I1 : 10000mV
			//I1 = 281318.510758mA
			//ratio = 281318510.758uA / 32768 = 8585.159630066158

			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 12.5ohm = 625mV
			//V1 = 625mV * G1 = 5948.275862mV
			//V2 = V1 * G2 = 8886.724137931mV
			//100000mA : 8886.724137931mV = I1 : 10000mV
			//I1 = 112527.4043mA
			//ratio = 112527404.3032uA / 32768 = 3434.063852026463
			
			//RANGE3
			//ITB300-S 300A:150mA = 25A:x -> x = 12.5mA
			//shuntV = 12.5mA * 50ohm = 625mV
			//V1 = 625mV * G1 = 5948.275862mV
			//V2 = V1 * G2 = 8886.72413793mV
			//25000mA : 8886.72413793mV = I1 : 10000mV
			//I1 = 28131.8510758mA
			//ratio = 28131851.0758uA / 32768 = 858.515963006616
			if((range+1) == RANGE1) ratio = 8585.159630066158;
			else if ((range+1) == RANGE2) ratio = 3434.063852026463;
			else ratio = 858.515963006616;
			break;
		case C_KBIA_300V_200A_100A_120KW:	  		//shh_200919	
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.87097mV
			//200000mA : 8931.87097mV = I1 : 10000mV
			//I1 = 223917.2517407761mA
			//ratio = 223917251.7407761uA / 32768 = 6833.41222353
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.6667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.87097mV
			//100000mA : 8931.87097mV = I1 : 10000mV
			//I1 = 111958.625870388mA
			//ratio = 111958625.870388uA / 32768 = 3416.7061117
			
			if((range+1) == RANGE1) ratio = 6833.41222353;
			else ratio = 3416.70611177;
			break;
		case C_HYUNDAI_300V_300A_150A_180KW:	//ljh_201106
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN600-S 300A:200mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 5ohm = 1000mV
			//V1 = shuntV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//300000mA : 8874.36mV = I1 : 10000mV
			//I1 = 338052.54688788825mA
			//ratio = 338052546.88788825uA / 32768 = 10316.545010006
			
			//RANGE2
			//ITN600-S 300A:200mA = 150A:x -> x = 100mA
			//shuntV = 100mA * 10ohm = 1000mV
			//V1 = shuntV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//150000mA : 8874.36mV = I1 : 10000mV
			//I1 = 169026.2734439441mA
			//ratio = 169026273.4439441uA / 32768 = 5158.2725050032
			if((range+1) == RANGE1) ratio = 10316.545010006;
			else ratio = 5158.2725050032;
			break;
		case C_SDIXIAN_300V_400A_100A_50A_25A:	//jhk_160527
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.967741935
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN-600 600A:400mA = 400A:x -> x = 266.667mA
			//shuntV = 266.667mA * 2.5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//400000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 447834.50348mA
			//ratio = 447834503.48uA / 32768 = 13666.82444706885

			//RANGE2
			//ITN-600 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//100000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 111958.62587mA
			//ratio = 111958625.87uA / 32768 = 3416.706111767213
			
			//RANGE3
			//ITN-600 600A:400mA = 50A:x -> x = 33.334mA
			//shuntV = 33.334mA * 20ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//50000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 55979.312935mA
			//ratio = 55979312.935uA / 32768 = 1708.353055883606
			
			//RANGE4
			//ITN-600 600A:400mA = 25A:x -> x = 16.667mA
			//shuntV = 16.667mA * 40ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//25000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 27989.656467mA
			//ratio = 27989656.467uA / 32768 = 854.176527941803
			if((range+1) == RANGE1) ratio = 13666.82444706885;
			else if ((range+1) == RANGE2) ratio = 3416.706111767213;
			else if ((range+1) == RANGE3) ratio = 1708.353055883606;
			else ratio = 854.176527941803;
			break;
		case C_LGC_350V_150A_100A_105KW:	//khj_191230
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.0kohm = 10.88
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN-600 600A:400mA = 150A:x -> x = 100.000mA
			//shuntV = 100.000mA * 5.0ohm = 500.00mV
			//V1 = 500.000mV * G1 = 5440.000mV
			//V2 = V1 * G2 = 8127.36mV
			//150000mA : 8127.36mV = I1 : 10000mV
			//I1 = 184561.7765178mA
			//ratio = 184561776.5178uA / 32768 = 5632.37843377

			//RANGE2
			//ITN-600 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 7.5ohm = 500.000mV
			//V1 = 500.000mV * G1 = 5440.000mV
			//V2 = V1 * G2 = 8127.36mV
			//100000mA : 8127.36mV = I1 : 10000mV
			//I1 = 123041.184345224mA
			//ratio = 123041184.345224uA / 32768 = 3754.91895584
			
			if((range+1) == RANGE1) ratio = 5632.37843377;
			else ratio = 3754.91895584;
			break;
		/*case C_LGC_350V_300A_100A_420KW:	//shh_220128
		case C_LGC_350V_300A_100A_420KW_2:	//shh_220128
		case C_LGC_350V_300A_100A_420KW_3:	//shh_220128
		case C_LGC_350V_300A_100A_420KW_4:	//shh_220128
		case C_LGC_600V_300A_100A_720KW:	//shh_220204
		case C_LGC_600V_300A_100A_720KW_2:	//shh_220204
		case C_LGC_600V_300A_100A_720KW_3:	//shh_220204
		case C_LGC_600V_300A_100A_720KW_4:	//shh_220204
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.17kohm = 9.0064
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN-600 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 3.33ohm = 666.6667mV
			//V1 = 666.6667mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.45705mV
			//300000mA : 8970.45705mV = I1 : 10000mV
			//I1 = 334431.1201979060mA
			//ratio = 334431120.19790600 uA / 32768 = 10206.02783807

			//RANGE2
			//ITN-600 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.6667mV
			//V1 = 666.6667mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.45705mV
			//100000mA : 8970.45705mV = I1 : 10000mV
			//I1 = 111477.0400659690mA
			//ratio = 111477040.06596900uA / 32768 = 3402.00927936
			
			if((range+1) == RANGE1) ratio = 10206.02783807;
			else ratio = 3402.00927936;
			break;*/
		case C_LGC_400V_60A_10A:
		case C_ROTEM_400V_60A_10A:
		case C_KATECH_400V_60A_10A_24KW:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.5kohm = 9.981818182
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 60A:x -> x = 30mA
			//shuntV = 30mA * 20ohm = 600mV
			//V1 = 600mV * G1 = 5989.090909mV
			//V2 = V1 * G2 = 8947.701818mV
			//60000mA : 8947.701818mV = I1 : 10000mV
			//I1 = 67056.32487mA
			//ratio = 67056324.87uA / 32768 = 2046.396633

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 120ohm = 600mV
			//V1 = 600mV * G1 = 5989.090909mV
			//V2 = V1 * G2 = 8947.701818mV
			//10000mA : 8947.701818mV = I1 : 10000mV
			//I1 = 11176.05415mA
			//ratio = 11176.05415uA / 32768 = 341.0661055
			if((range+1) == RANGE1) ratio = 2046.396633;
			else ratio = 341.0661055;
			break;
		case C_LGC_400V_100A_10A_40KW:
		case C_SKI_400V_100A_10A_40KW: //jhk_120112
		case C_3PSYSTEM_400V_100A_10A_40KW_1:	//jhk_120511
		case C_3PSYSTEM_400V_100A_10A_40KW_2:	//jhk_120708
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//100000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 111764.3114mA
			//ratio = 111764311.4uA / 32768 = 3410.776105

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 100ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//10000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 11176.43114mA
			//ratio = 11176431.14uA / 32768 = 341.0776105
			if((range+1) == RANGE1) ratio = 3410.776105;
			else ratio = 341.0776105;
			break;
		case C_AVL_400V_200A_10A_80KW:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//200000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 223528.6224mA
			//ratio = 223528622.4uA / 32768 = 6821.552209

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 100ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//10000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 11176.43114mA
			//ratio = 11176431.14uA / 32768 = 341.0776105
			if((range+1) == RANGE1) ratio = 6821.552209;
			else ratio = 341.0776105;
			break;
		case C_KATECH_400V_250A_10A:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//250000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 289979.007mA
			//ratio = 289979007.0uA / 32768 = 8849.457001

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 110ohm = 550mV
			//V1 = 550mV * G1 = 5078.15mV
			//V2 = V1 * G2 = 7586.7561mV
			//10000mA : 7586.7561mV = I1 : 10000mV
			//I1 = 13180.86395mA
			//ratio = 13180863.95uA / 32768 = 402.2480453
			if((range+1) == RANGE1) ratio = 8849.457001;
			else ratio = 402.2480453;
			break;
		case C_ERAE_400V_300A_100A_30KW:	//jhk_150119
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//300000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.944243879
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//100000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 110766.15366mA
			//ratio = 110766153.66uA / 32768 = 3380.3147479595
			
			if((range+1) == RANGE1) ratio = 10140.944243879;
			else ratio = 3380.3147479595;
			break;
		case C_ROTEM_445V_265A_10A:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.1kohm = 9.098360656
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 265A:x -> x = 132.5mA
			//shuntV = 132.5mA * 5ohm = 662.5mV
			//V1 = 662.5mV * G1 = 6027.663935mV
			//V2 = V1 * G2 = 9005.329919mV
			//265000mA : 9005.329919mV = I1 : 10000mV
			//I1 = 294270.1738mA
			//ratio = 294270173.8uA / 32768 = 8980.413019

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 130ohm = 650mV
			//V1 = 650mV * G1 = 5913.934426mV
			//V2 = V1 * G2 = 8835.418032mV
			//10000mA : 8835.418032mV = I1 : 10000mV
			//I1 = 11318.08361mA
			//ratio = 11318083.61uA / 32768 = 345.4005008
			if((range+1) == RANGE1) ratio = 8980.413019;
			else ratio = 345.4005008;
			break;
//lki_111029		case C_PNE_450V_250A_10A_115KW_2: //csk_110711
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//KOHSHIN 300A:200mA = 250A:x -> x = 166.66mA
			//shuntV = 166.66mA * 5ohm = 833mV
			//V1 = 833mV * G1 = 5976.775mV
			//V2 = V1 * G2 = 8929.301mV
			//250000mA : 8929.301mV = I1 : 10000mV
			//I1 = 279977.122509mA
			//ratio = 279977122.509uA / 32768 = 8544.223709

			//RANGE2
			//KOHSHIN 300A:200mA = 10A:x -> x = 6.66mA
			//shuntV = 6.66mA * 125ohm = 833mV
			//V1 = 833mV * G1 = 5976.775mV
			//V2 = V1 * G2 = 8929.301mV
			//10000mA : 8929.301mV = I1 : 10000mV
			//I1 = 11199.084900mA
			//ratio = 11199084.900uA / 32768 = 341.768948
//			if((range+1) == RANGE1) ratio = 8544.223709;
//			else ratio = 341.768948;
//			break;
		case C_KATECH_450V_150A_10A:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 150A:x -> x = 75mA
			//shuntR = (5//10)+(5//10) = 6.666666667ohm
			//shuntV = 75mA * 6.666666667ohm = 500mV
			//V1 = 500mV * G1 = 5988.888889mV
			//V2 = V1 * G2 = 8947.4mV
			//150000mA : 8947.4mV = I1 : 10000mV
			//I1 = 167646.4671mA
			//ratio = 167646467.1uA / 32768 = 5116.164158

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntR = 100ohm
			//shuntV = 5mA * 100ohm = 500mV
			//V1 = 500mV * G1 = 5988.888889mV
			//V2 = V1 * G2 = 8947.4mV
			//10000mA : 8947.4mV = I1 : 10000mV
			//I1 = 11176.43114mA
			//ratio = 11176431.14uA / 32768 = 341.0776105
			if((range+1) == RANGE1) ratio = 5116.164158;
			else ratio = 341.0776105;
			break;
		case C_LGC_450V_200A_10A:
		case C_LGC_450V_200A_10A_2:
		case C_LGC_450V_200A_10A_3:
		case C_LGC_450V_200A_10A_4:
		case C_LGC_450V_200A_10A_5:
		case C_LGC_450V_200A_10A_6:
		case C_SK_450V_200A_10A_180KW:
		case C_ROTEM_450V_200A_10A:
		case C_SK_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_HYUNDAI_450V_200A_10A_90KW:
		case C_HYUNDAI_450V_200A_10A_90KW_2:
		case C_VENS_450V_200A_10A_90KW:
		case C_KEPCO_450V_200A_10A_90KW:
		case C_SK_450V_200A_10A_360KW:
		case C_LGC_450V_200A_10A_180KW:
		//case C_LGC_450V_200A_10A_180KW_2:	//ktg_210222
		case C_LGC_450V_200A_10A_180KW_3:	//kjhw_121102
		case C_LGC_450V_200A_10A_180KW_4:	//jhkw_130909
			//10000000uV / 32768 = 305.1757813uV(AD974ARS)
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//200000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 223528.6228mA
			//ratio = 223528622.8uA / 32768 = 6821.552209

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 100ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//10000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 11176.43114mA
			//ratio = 11176431.14uA / 32768 = 341.0776105
			if((range+1) == RANGE1) ratio = 6821.552209;
			else ratio = 341.0776105;
			break;
		/*case C_LGC_450V_200A_10A_180KW_2:	//200A->250A	//ktg_210222
			//10000000uV / 32768 = 305.1757813uV(AD974ARS)
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.2333333
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.538413mA
			//ratio = 289968538.413uA / 32768 = 8849.13752482

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 125ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333mV
			//V2 = V1 * G2 = 8621.625mV
			//10000mA : 8621.625mV = I1 : 10000mV
			//I1 = 11598.741536mA
			//ratio = 11598741.536uA / 32768 = 353.96550099
			if((range+1) == RANGE1) ratio = 8849.13752;
			else ratio = 353.9655009;
			break;*/	//shh_231024
		case C_LGC_450V_200A_20A_180KW:
		case C_LGC_450V_200A_20A_180KW_2:
		case C_CTNT_450V_200A_10A_180KW:	//kjh_110929 10A->20A
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//200000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 223528.6228mA
			//ratio = 223528622.8uA / 32768 = 6821.552209

			//RANGE2
			//ITB300-S 300A:150mA = 20A:x -> x = 10mA
			//shuntV = 10mA * (100ohm // 100ohm) = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//20000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 22352.862279mA
			//ratio = 22352.862279uA / 32768 = 682.1552209
			if((range+1) == RANGE1) ratio = 6821.552209;
			else ratio = 682.1552209;
			break;
		case C_DAEWOO_450V_200A_50A_90KW: //kjh_150518
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			//10000000uV / 32768 = 305.1757813uV(AD974ARS)
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//200000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 223528.6228mA
			//ratio = 223528622.8uA / 32768 = 6821.552209

			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 20ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//50000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 55882.15569754741mA
			//ratio = 55882155.69754741uA / 32768 = 1705.388052293317
			if((range+1) == RANGE1) ratio = 6821.552209;
			else ratio = 1705.388052;
			break;
		case C_NKT_450V_200A_100A_90KW:	//ktg_190111
		case C_NKT_450V_200A_100A_90KW_2:	//ktg_190111
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5kohm = 10.88
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5440mV
			//V2 = V1 * G2 = 8127.36mV
			//200000mA : 8127.36mV = I1 : 10000mV
			//I1 = 246082.3686904mA
			//ratio = 246082368.6904uA / 32768 = 7509.8379116

			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5440mV
			//V2 = V1 * G2 = 8127.36mV
			//100000mA : 8127.36mV = I1 : 10000mV
			//I1 = 123041.1843452240mA
			//ratio = 123041184.3452240uA / 32768 = 3754.918955847901
			if((range+1) == RANGE1) ratio = 7509.8379116;
			else ratio = 3754.9189558;
			break;
		case C_SDI_450V_225A_22A_202KW:		//jhkw_130629
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.2kohm = 10.5
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 225A:x -> x = 112.5mA
			//shuntV = 112.5mA * 5ohm = 562.5mV
			//V1 = 562.5mV * G1 = 5906.25mV
			//V2 = V1 * G2 = 8823.9375mV
			//225000mA : 8823.9375mV = I1 : 10000mV
			//I1 = 254988.2067954357mA
			//ratio = 254988206.7954357uA / 32768 = 7781.622521833365

			//RANGE2
			//ITB300-S 300A:150mA = 75A:x -> x = 37.5mA
			//shuntV = 37.5mA * 15ohm = 562.5mV
			//V1 = 562.5mV * G1 = 5906.25mV
			//V2 = V1 * G2 = 8823.9375mV
			//75000mA : 8823.9375mV = I1 : 10000mV
			//I1 = 84996.0689318119mA
			//ratio = 84996068.9318119uA / 32768 = 2593.874173944455

			//RANGE3
			//ITB300-S 300A:150mA = 25A:x -> x = 12.5mA
			//shuntV = 12.5mA * 45ohm = 562.5mV
			//V1 = 562.5mV * G1 = 5906.25mV
			//V2 = V1 * G2 = 8823.9375mV
			//25000mA : 8823.9375mV = I1 : 10000mV
			//I1 = 28332.02297727mA
			//ratio = 28332022.97727uA / 32768 = 864.62472464815
			if((range+1) == RANGE1) ratio = 7781.622521833365;
			else if ((range+1) == RANGE2) ratio = 2593.874173944455;
			else ratio = 864.62472464815;
			break;
		/*
		case C_PNE_450V_250A_10A_115KW:	//jhk_131201
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.83333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.538413582mA
			//ratio = 289968538.41358uA / 32768 = 8849.137524828556

			//RANGE2
			//ITB300-S 300A:150mA = 125A:x -> x = 62.5mA
			//shuntV = 62.5mA * 10ohm = 625mV
			//V1 = 625mV * G1 = 5770.83333mV
			//V2 = V1 * G2 = 8621.625mV
			//125000mA : 8621.625mV = I1 : 10000mV
			//I1 = 144984.269206791mA
			//ratio = 144984269.206791uA / 32768 = 4424.5687624

			//RANGE3
			//ITB300-S 300A:150mA = 62.5A:x -> x = 31.25mA
			//shuntV = 31.25mA * 20ohm = 625mV
			//V1 = 625mV * G1 = 5770.83333mV
			//V2 = V1 * G2 = 8621.625mV
			//62500mA : 8621.625mV = I1 : 10000mV
			//I1 = 724921.3460339553mA
			//ratio = 724921346.0339553uA / 32768 = 2212.284381207
			if((range+1) == RANGE1) ratio = 8849.137524828556;
			else if ((range+1) == RANGE2) ratio = 4424.5687624;
			else ratio = 2212.284381207;
			break;
		*/
		case C_PNE_450V_250A_10A_115KW:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.83333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.538413582mA
			//ratio = 289968538.41358uA / 32768 = 8849.137524828556

			//RANGE2
			//ITB300-S 300A:150mA = 125A:x -> x = 62.5mA
			//shuntV = 62.5mA * 10ohm = 625mV
			//V1 = 625mV * G1 = 5770.83333mV
			//V2 = V1 * G2 = 8621.625mV
			//125000mA : 8621.625mV = I1 : 10000mV
			//I1 = 144984.269206791mA
			//ratio = 144984269.206791uA / 32768 = 4424.5687624

			//RANGE3
			//ITB300-S 300A:150mA = 62.5A:x -> x = 31.25mA
			//shuntV = 31.25mA * 20ohm = 625mV
			//V1 = 625mV * G1 = 5770.83333mV
			//V2 = V1 * G2 = 8621.625mV
			//62500mA : 8621.625mV = I1 : 10000mV
			//I1 = 724921.3460339553mA
			//ratio = 724921346.0339553uA / 32768 = 2212.284381207
			if((range+1) == RANGE1) ratio = 8849.137524828556;
			else if ((range+1) == RANGE2) ratio = 4424.5687624;
			else ratio = 2212.284381207;
			break;
		//case C_PNE_450V_250A_10A_115KW:	//jhk_131206
		case C_PNE_450V_250A_10A_115KW_2: //lki_111029
		case C_KATECH_450V_250A_10A_115KW:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//250000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 289979.007mA
			//ratio = 289979007.0uA / 32768 = 8849.457001

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 125ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//10000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 11599.16028mA
			//ratio = 11599160.28uA / 32768 = 353.97828
			if((range+1) == RANGE1) ratio = 8849.457001;
			else ratio = 353.97828;
			break;
		case C_LGC_450V_250A_10A_225KW: //jhk_140728
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//250000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 289979.007mA
			//ratio = 289979007.0uA / 32768 = 8849.457001

			//RANGE2 10A->25A
			//ITB300-S 300A:150mA = 25A:x -> x = 12.5mA
			//shuntV = 12.5mA * 50ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//25000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 28997.90069698mA
			//ratio = 28997900.69698uA / 32768 = 884.94569998
			if((range+1) == RANGE1) ratio = 8849.457001;
			else ratio = 884.94569998;
			break;
		case C_KATECH_450V_250A_25A_225KW:
		case C_KATECH_450V_250A_25A_225KW_2:
		case C_SEM_450V_250A_10A_225KW:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//250000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 289979.007mA
			//ratio = 289979007.0uA / 32768 = 8849.457001

			//RANGE2
			//ITB300-S 300A:150mA = 25A:x -> x = 12.5mA
			//shuntV = 12.5mA * 50ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//25000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 28997.90069698mA
			//ratio = 28997900.69698uA / 32768 = 884.94569998
			if((range+1) == RANGE1) ratio = 8849.457001;
			else ratio = 884.94569998;
			break;
		//case C_LGE_450V_250A_50A_112KW:		//jhk_150703
		case C_LGE_450V_250A_50A_225KW:		//jhk_150605
		case C_LGE_450V_250A_50A_225KW_2:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_3:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_4:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_5:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_6:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_7:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_8:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_9:	//jhk_150605
		//case C_LGC_450V_250A_50A_225KW:		//jhk_160608
		//case C_LGC_450V_250A_50A_225KW_2:	//jhk_160608
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.83333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.5384mA
			//ratio = 289968538.4uA / 32768 = 8849.137524828556

			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 25ohm = 625mV
			//V1 = 625mV * G1 = 5770.83333mV
			//V2 = V1 * G2 = 8621.625mV
			//50000mA : 8621.625mV = I1 : 10000mV
			//I1 = 57993.7076827mA
			//ratio = 57993707.6827uA / 32768 = 1769.8275049657

			if((range+1) == RANGE1) ratio = 8849.137524828556;
			else ratio = 1769.8275049657;
			break;
		//case C_LGC_450V_250A_50A_225KW:		//jhk_160608
		//case C_LGC_450V_250A_50A_225KW_2:	//jhk_160608	//shh_231024
		case C_KMI_450V_250A_100A_112KW:	//jhk_171110
		//case C_LGC_450V_250A_100A_225KW:	//jhk_170211	//shh_231024
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.83333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.5384mA
			//ratio = 289968538.4uA / 32768 = 8849.137524828556

			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 12.5ohm = 625mV
			//V1 = 625mV * G1 = 5770.83333mV
			//V2 = V1 * G2 = 8621.625mV
			//100000mA : 8621.625mV = I1 : 10000mV
			//I1 = 115987.415mA
			//ratio = 115987415.365uA / 32768 = 3539.655009931422

			if((range+1) == RANGE1) ratio = 8849.137524828556;
			else ratio = 3539.655009931422;
			break;
		case C_LGC_500V_20A:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ES100C 100A:100mA = 20A:x -> x = 20mA
			//shuntV = 20mA * 25ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//20000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 22352.86228mA
			//ratio = 22352862.28uA / 32768 = 682.1552209
			ratio = 682.1552209;
			break;
		case C_LGCCHINA_500V_150A_75KW:		//jhk_160727
		case C_LGCCHINA_500V_150A_75KW_2:	//jhk_160727
		case C_LGCCHINA_500V_150A_75KW_3:	//jhk_160727
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 150A:x -> x = 75mA
			//shuntV = 75mA * 10ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.0285714mV
			//150000mA : 9028.0285714mV = I1 : 10000mV
			//I1 = 166149.2304917mA
			//ratio = 166149230.4917uA / 32768 = 5070.47212193929
			
			ratio = 5070.47212193929;
			break;	
		case C_KEPCO_500V_200A_10A_100KW:
		case C_KTL_500V_200A_10A_200KW_3:		//jhk_130629
		case C_LGC_500V_200A_10A:
		case C_NEXCON_500V_200A_10A:
		case C_EIG_500V_200A_10A_200KW:
		case C_HLGP_500V_200A_10A_200KW:
		case C_LGC_500V_200A_10A_200KW:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//200000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 223528.6228mA
			//ratio = 223528622.8uA / 32768 = 6821.552209

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 100ohm = 500mV
			//V1 = 500mV * G1 = 5988.88889mV
			//V2 = V1 * G2 = 8947.400002mV
			//10000mA : 8947.400002mV = I1 : 10000mV
			//I1 = 11176.43114mA
			//ratio = 11176431.14uA / 32768 = 341.0776105
			if((range+1) == RANGE1) ratio = 6821.552209;
			else ratio = 341.0776105;
			break;
		case C_HLGP_500V_200A_100A_200KW: //kjhw_120527
		case C_HLGP_500V_200A_100A_200KW_2: //jhkw_120528
		case C_HLGP_500V_200A_100A_200KW_3: //jhkw_130223
		case C_HLGP_500V_200A_100A_200KW_4: //jhkw_130223
		case C_HLGP_500V_200A_100A_200KW_5: //jhk_131215
		case C_LGC_500V_200A_100A_200KW:    //jhkw_130305
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.399999mV
			//200000mA : 8947.39999mV = I1 : 10000mV
			//I1 = 223528.62284015468mA
			//ratio = 223528622.84015468uA / 32768 = 6821.5522106
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.399999mV
			//100000mA : 8947.39999mV = I1 : 10000mV
			//I1 = 111764.31142007mA
			//ratio = 111764311.42007uA / 32768 = 3410.776105349
			
			if((range+1) == RANGE1) ratio = 6821.5522106;
			else ratio = 3410.776105;
			break;
		case C_NS_500V_250A_10A:
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.625mV
			//V2 = V1 * G2 = 8621.31375mV
			//250000mA : 8621.31375mV = I1 : 10000mV
			//I1 = 289979.007mA
			//ratio = 289979007.0uA / 32768 = 8849.457001

			//RANGE2
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 125ohm = 625mV
			//V1 = 625mV * G1 = 5770.83mV
			//V2 = V1 * G2 = 8621.625mV
			//10000mA : 8621.625mV = I1 : 10000mV
			//I1 = 11598.74154mA
			//ratio = 11598741.54uA / 32768 = 353.9655010
			if((range+1) == RANGE1) ratio = 8849.457001;
			else ratio = 353.9655010;
			break;
		case C_PNE_500V_250A_10A_125KW: //kjhw_120507
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.74kohm = 9.606
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 6003.919mV
			//V2 = V1 * G2 = 8969.85627mV
			//250000mA : 8969.85627mV = I1 : 10000mV
			//I1 = 278711.26629599mA
			//ratio = 278711266.29599uA / 32768 = 8505.592843
			//
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 12.5ohm = 625mV
			//V1 = 625mV * G1 = 6003.919mV
			//V2 = V1 * G2 = 8969.85627mV
			//100000mA : 8969.85627mV = I1 : 10000mV
			//I1 = 111484.5065mA
			//ratio = 111484506.5uA / 32768 = 3402.237137
			
			//RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 25ohm = 625mV
			//V1 = 625mV * G1 = 6003.919mV
			//V2 = V1 * G2 = 8969.85627mV
			//50000mA : 8969.85627mV = I1 : 10000mV
			//I1 = 55742.25327mA
			//ratio = 55742253.27uA / 32768 = 1701.118569
			
			//RANGE4
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 125ohm = 625mV
			//V1 = 625mV * G1 = 6003.919mV
			//V2 = V1 * G2 = 8969.85627mV
			//10000mA : 8969.85627mV = I1 : 10000mV
			//I1 = 11148.45065mA
			//ratio = 11148450.65uA / 32768 = 340.2237137
			
			if((range+1) == RANGE1) ratio = 8505.592843;
			else if ((range+1) == RANGE2) ratio = 3402.237137;
			else if ((range+1) == RANGE3) ratio = 1701.118569;
			else ratio = 340.2237137;
			break;
		case C_LGE_500V_250A_50A_250KW:	//jhk_150607
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.538413582mA
			//ratio = 289968538.413582uA / 32768 = 8849.137524829
			
			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 25ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//50000mA : 8621.625mV = I1 : 10000mV
			//I1 = 57993.707682716mA
			//ratio = 57993707.682716uA / 32768 = 1769.8275049657

			if((range+1) == RANGE1) ratio = 8849.137524829;
			else ratio = 1769.8275049657;
			break;
		case C_LGC_500V_250A_100A_50A_250KW:    //jhkw_120822
		case C_KTL_500V_250A_100A_50A_250KW:	//jhkw_130320
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.538413582mA
			//ratio = 289968538.413582uA / 32768 = 8849.137524829
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 12.5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//100000mA : 8621.625mV = I1 : 10000mV
			//I1 = 115987.41536543285mA
			//ratio = 115987415.36543285uA / 32768 = 3539.655009931
			                                                                                //RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 25ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//50000mA : 8621.625mV = I1 : 10000mV
			//I1 = 57993.707682716mA
			//ratio = 57993707.682716uA / 32768 = 1769.8275049657

			if((range+1) == RANGE1) ratio = 8849.137524829;
			else if ((range+1) == RANGE2) ratio = 3539.655009931;
			else ratio = 1769.8275049657;
			break;
		//case C_LGC_500V_250A_100A_125KW:	//ktg_190620
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 500mV * G1 = 5770.833mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.53841mA
			//ratio = 289968538.41uA / 32768 = 8849.1375248
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 12.5ohm = 625mV
			//V1 = 500mV * G1 = 5770.833mV
			//V2 = V1 * G2 = 8621.625mV
			//100000mA : 8621.625mV = I1 : 10000mV
			//I1 = 115987.415365433mA
			//ratio = 115987415.365433uA / 32768 = 3539.65500993
			
			if((range+1) == RANGE1) ratio = 8849.1375248;
			else ratio = 3539.65500993;
			break;
		case C_KBIA_500V_200A_100A_200KW:	//shh_200918
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.87097mV
			//200000mA : 8931.87097mV = I1 : 10000mV
			//I1 = 223917.2517407761mA
			//ratio = 223917251.7407761uA / 32768 = 6833.41222353
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.6667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.87097mV
			//100000mA : 8931.87097mV = I1 : 10000mV
			//I1 = 111958.625870388mA
			//ratio = 111958625.870388uA / 32768 = 3416.7061117
			
			if((range+1) == RANGE1) ratio = 6833.41222353;
			else ratio = 3416.70611177;
			break;
		case C_KBIA_500V_250A_100A_250KW:	//shh_200916
		case C_JBTP_500V_250A_100A_500KW:	//shh_200927
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 4ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.87097mV
			//250000mA : 8931.87097mV = I1 : 10000mV
			//I1 = 279896.56467597mA
			//ratio = 279896564.67597uA / 32768 = 8541.76528
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.6667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.87097mV
			//100000mA : 8931.87097mV = I1 : 10000mV
			//I1 = 111958.625870388mA
			//ratio = 111958625.870388uA / 32768 = 3416.7061117
			
			if((range+1) == RANGE1) ratio = 8541.76528;
			else ratio = 3416.70611177;
			break;
	/*	case C_LGC_500V_250A_100A_50A_250KW:	//jhkw_120809
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 4ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//250000mA : 8947.4mV = I1 : 10000mV
			//I1 = 279410.778550193mA
			//ratio = 279410778.550193uA / 32768 = 8526.94026337
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142008mA
			//ratio = 111764311.42008uA / 32768 = 3410.776105349
			
			//RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 20ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//50000mA : 8947.4mV = I1 : 10000mV
			//I1 = 55882.155710039mA
			//ratio = 55882155.710039uA / 32768 = 1705.3880526745
			
			if((range+1) == RANGE1) ratio = 8526.94026337;
			else if ((range+1) == RANGE2) ratio = 3410.776105349;
			else ratio = 1705.3880526745;
			break;
	*/
		case C_LGC_500V_250A_100A_50A_250KW_2:	//ktg_200208
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 4ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.87097mV
			//250000mA : 8931.87097mV = I1 : 10000mV
			//I1 = 279896.56467597mA
			//ratio = 279896564.67597uA / 32768 = 8541.76528
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.87097mV
			//100000mA : 8931.87097mV = I1 : 10000mV
			//I1 = 111958.625870388mA
			//ratio = 111958625.870388uA / 32768 = 3416.70611177
			
			//RANGE3
			//ITN600-S 600A:400mA = 50A:x -> x = 33.333mA
			//shuntV = 33.333mA * 20ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.87097mV
			//50000mA : 8931.87097mV = I1 : 10000mV
			//I1 = 55979.31293519401mA
			//ratio = 55979312.93519401uA / 32768 = 1708.35305588
			
			if((range+1) == RANGE1) ratio = 8541.76528;
			else if ((range+1) == RANGE2) ratio = 3416.70611177;
			else ratio = 1708.35305588;
			break;
		case C_HYUNDAI_500V_250A_125A_25A_250KW: //ktg_190324
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//250000mA : 8932.875mV = I1 : 10000mV
			//I1 = 279865.105mA
			//ratio = 279865105uA / 32768 = 8540.805206890279
			
			//RANGE2
			//ITN600-S 600A:400mA = 125A:x -> x = 83.333mA
			//shuntV = 83.333mA * 10ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//125000mA : 8932.875mV = I1 : 10000mV
			//I1 = 139932.5525mA
			//ratio = 139932552.5uA / 32768 = 4270.402603445139
			
			//RANGE3
			//ITN600-S 600A:400mA = 25A:x -> x = 16.6667mA
			//shuntV = 16.6667mA * 50ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//25000mA : 8932.875mV = I1 : 10000mV
			//I1 = 27986.5105019mA
			//ratio = 27986510.5019A / 32768 =854.0805206890279 
			
			if((range+1) == RANGE1) ratio = 8540.805206890279;
			else if ((range+1) == RANGE2) ratio = 4270.402603445139;
			else ratio = 854.0805206890279;
			break;
		case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
		/*case C_LGC_500V_250A_125A_50A_500KW_2: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_3: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_4: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_5: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_6: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_7: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_8: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_9: //ktg_190227 */	//ktg_231124
		//case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_190615
		//case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_190615	//shh_231024
		case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 5ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//250000mA : 8932.875mV = I1 : 10000mV
			//I1 = 279865.105mA
			//ratio = 279865105uA / 32768 = 8540.805206890279
			
			//RANGE2
			//ITN600-S 600A:400mA = 125A:x -> x = 83.333mA
			//shuntV = 83.333mA * 10ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.166667mV
			//V2 = V1 * G2 = 8932.875mV
			//125000mA : 8932.875mV = I1 : 10000mV
			//I1 = 139932.5525mA
			//ratio = 139932552.5uA / 32768 = 4270.402603445139
			
			//RANGE3
			//ITN600-S 600A:400mA = 50A:x -> x = 33.333mA
			//shuntV = 33.333mA * 25ohm = 833.333mV
			//V1 = 833.333mV * G1 = 5979.1667mV
			//V2 = V1 * G2 = 8932.875mV
			//50000mA : 8932.875mV = I1 : 10000mV
			//I1 = 55973.021mA
			//ratio = 55973021uA / 32768 = 1708.161041378056
			
			if((range+1) == RANGE1) ratio = 8540.805206890279;
			else if ((range+1) == RANGE2) ratio = 4270.402603445139;
			else ratio = 1708.161041378056;
			break;
		case C_KBIA_500V_300A_100A_150KW:	//shh_200919
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.7kohm = 7.415584
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 4ohm = 800mV
			//V1 = 800mV * G1 = 5932.4675mV
			//V2 = V1 * G2 = 8863.10649mV
			//300000mA : 8863.10649mV = I1 : 10000mV
			//I1 = 338481.7731102315mA
			//ratio = 338481773.1102315uA / 32768 = 10329.64395478
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.6667mA * 10ohm = 800mV
			//V1 = 800mV * G1 = 5932.4675mV
			//V2 = V1 * G2 = 8863.10649mV
			//100000mA : 8863.10649mV = I1 : 10000mV
			//I1 = 112827.2576587729mA
			//ratio = 112827257.6587729uA / 32768 = 3443.21465023
			
			if((range+1) == RANGE1) ratio = 10329.64395478;
			else ratio = 3443.21465023;
			break;
		case C_HLGP_500V_300A_100A_300KW:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_2:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_3:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_4:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_5:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_6:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_7:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_8:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_9:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_10:	//jhk_170722
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//300000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.944243879
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//100000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 110766.15366mA
			//ratio = 110766153.66uA / 32768 = 3380.3147479595
			
			if((range+1) == RANGE1) ratio = 10140.944243879;
			else ratio = 3380.3147479595;
			break;
		case C_SDI_500V_300A_100A_75KW:		//ktg_200809
		case C_SDI_500V_300A_100A_150KW:	//ktg_200809
		case C_SDI_500V_300A_100A_150KW_2:	//ktg_200809
		case C_SDI_500V_300A_100A_150KW_3:	//ktg_200809
		case C_SDI_500V_300A_100A_150KW_4:	//ktg_200809
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.9777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 2.5ohm = 500mV
			//V1 = 500mV * G1 = 5988.8889mV
			//V2 = V1 * G2 = 8947.4mV
			//300000mA : 8947.4mV = I1 : 10000mV
			//I1 = 335292.9342602mA
			//ratio = 335292934.2602uA / 32768 = 10232.328316047
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 7.5ohm = 500mV
			//V1 = 500mV * G1 = 5988.8889mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.3114200773mA
			//ratio = 111764311.4200773uA / 32768 = 3410.776105349
			
			if((range+1) == RANGE1) ratio = 10232.328316047;
			else ratio = 3410.776105349;
			break;
		/*case C_LGC_500V_300A_100A_300KW:	//ktg_190418
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 4ohm = 800mV
			//V1 = 800mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//300000mA : 8575.56mV = I1 : 10000mV
			//I1 = 349831.381274226mA
			//ratio = 349831381.274226uA / 32768 = 10676.00651
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 12ohm = 800mV
			//V1 = 800mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//100000mA : 8575.56mV = I1 : 10000mV
			//I1 = 116610.460424741941mA
			//ratio = 116610460.424741941uA / 32768 = 3558.6688362
			
			if((range+1) == RANGE1) ratio = 10676.00651;
			else ratio = 3558.6688362;
			break;*/
		case C_GANGSO_500V_400A_200KW:	//jhk_160518
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 400A:x -> x = 266.667mA
			//shuntV = 266.667mA * 2.5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5371.42857mV
			//V2 = V1 * G2 = 8024.9142857mV
			//400000mA : 8024.9142857mV = I1 : 10000mV
			//I1 = 498447.69147512mA
			//ratio = 498447691.47512uA / 32768 = 15211.41636581788
			
			ratio = 15211.41636581788;
			break;
		case C_HLGP_500V_400A_200KW:	//jhk_170625
		case C_SKI_500V_400A_200KW:	//jhk_171015
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677419
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN600-S 600A:400mA = 400A:x -> x = 266.667mA
			//shuntV = 266.667mA * 2.5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//400000mA : 8931.87096774mV = I1 : 10000mV
			///I1 = 447834.50348mA
			//ratio = 447834503.48uA / 32768 = 13666.82444706885
			
			ratio = 13666.82444706885;
			break;
		case C_LGC_500V_400A_200A_400KW:	//ktg_190716
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 400A:x -> x = 266.667mA
			//shuntV = 266.667mA * 3ohm= 800mV
			//V1 = 1000mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//400000mA : 8575.56mV = I1 : 10000mV
			//I1 = 466441.841699mA
			//ratio = 466441841.699uA / 32768 = 14234.6753448
			
			//RANGE2
			//ITN600-S 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 6ohm = 800mV
			//V1 = 1000mV * G1 = 5740mV
			//V2 = V1 * G2 = 8575.56mV
			//200000mA : 8575.56mV = I1 : 10000mV
			//I1 = 233220.9208495mA
			//ratio = 233220920.8495uA / 32768 = 7117.337672
			
			if((range+1) == RANGE1) ratio = 14234.6753448;
			else ratio = 7117.337672;
			break;
		case C_SDI_600V_60A_72KW:			//shh_201202
		case C_SDI_600V_60A_72KW_2:			//shh_201202
		case C_SDI_600V_60A_72KW_3:			//shh_201202
		case C_SDI_600V_60A_72KW_4:			//shh_201202
		case C_SDI_600V_60A_72KW_5:			//shh_201202
		case C_SDI_600V_60A_72KW_6:			//shh_201202
		case C_SDI_600V_60A_72KW_7:			//shh_201202
		case C_SDI_600V_60A_72KW_8:			//shh_201202
		case C_SDI_600V_60A_72KW_9:			//shh_201202
		case C_SDI_600V_60A_72KW_10:		//shh_201202
		case C_SDI_600V_60A_72KW_11:		//shh_201202
		case C_SDI_600V_60A_72KW_12:		//shh_201202
		case C_SDI_600V_60A_72KW_13:		//shh_201202
		case C_SDI_600V_60A_72KW_14:		//shh_201202
		case C_SDI_600V_60A_72KW_15:		//shh_201202
		case C_SDI_600V_60A_72KW_16:		//shh_201202
		case C_SDI_600V_60A_72KW_17:		//shh_201202
		case C_SDI_600V_60A_72KW_18:		//shh_201202
		case C_SDI_600V_60A_72KW_19:		//shh_201202
		case C_SDI_600V_60A_72KW_20:		//shh_201202
		case C_SDI_600V_60A_72KW_21:		//shh_201202
		case C_SDI_600V_60A_72KW_22:		//shh_201202
		case C_SDI_600V_60A_72KW_23:		//shh_201202
		case C_SDI_600V_60A_72KW_24:		//shh_201202
		case C_SDI_600V_60A_72KW_25:		//shh_201202
		case C_SDI_600V_60A_72KW_26:		//shh_201202
		case C_SDI_600V_60A_72KW_27:		//shh_201202
		case C_SDI_600V_60A_72KW_28:		//shh_201202
		case C_SDI_600V_60A_72KW_29:		//shh_201202
		case C_SDI_600V_60A_72KW_30:		//shh_201202
		case C_SDI_600V_60A_72KW_31:		//shh_201202
		case C_SDI_600V_60A_72KW_32:		//shh_201202
		case C_SDI_600V_60A_72KW_33:		//shh_201202
		case C_SDI_600V_60A_72KW_34:		//shh_201202
		case C_SDI_600V_60A_72KW_35:		//shh_201202
		case C_SDI_600V_60A_72KW_36:		//shh_201202
		case C_SDI_600V_60A_72KW_37:		//shh_201202
		case C_SDI_600V_60A_72KW_38:		//shh_201202
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN600-S 600A:400mA = 60A:x -> x = 40mA
			//shuntV = 40mA * 25ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//60000mA : 8874.36mV = I1 : 10000mV
			//I1 = 67610.50937758mA
			//ratio = 67610509.37758uA / 32768 = 2063.309002
			ratio = 2063.309002;
			break;
		/*case C_LGC_600V_150A_100A_180KW:	//ktg_190405
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 150A:x -> x = 75mA
			//shuntV = 75mA * 10ohm = 750mV
			//V1 = 750mV * G1 = 6042.8571428571mV
			//V2 = V1 * G2 = 9028.02857142mV
			//150000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 166149.23049170678mA
			//ratio = 166149230.49170678uA / 32768 = 5070.472122
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.8571428571mV
			//V2 = V1 * G2 = 9028.02857142mV
			//100000mA : 9028.02857142mV = I1 : 10000mV
			//I1 = 110766.15366114mA
			//ratio = 110766153.66114uA / 32768 = 3380.314748
			
			if((range+1) == RANGE1) ratio = 5070.472122;
			else ratio = 3380.314748;
			break;*/	//shh_231024
		case C_LGE_600V_150A_100A_180KW: //shh_210426
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 150A:x -> x = 100mA
			//shuntV = 100mA * 10ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//150000mA : 8874.36mV = I1 : 10000mV
			//I1 = 169026.2734439441mA
			//ratio = 169026273.4439441uA / 32768 = 5158.27250500
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.6667mA
			//shuntV = 66.6667mA * 15ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//100000mA : 8874.36mV = I1 : 10000mV
			//I1 = 112684.1822959628mA
			//ratio = 112684182.2959628uA / 32768 = 3438.84833666
			
			if((range+1) == RANGE1) ratio = 5158.27250500;
			else ratio = 3438.84833666;
			break;
		case C_KCL_600V_200A_100A_480KW: //ktg_190812
		case C_KCL_600V_200A_100A_480KW_2: //ktg_190812
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.5kohm = 8.6
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 200mA * 5ohm= 666.667mV
			//V1 = 666.667mV * G1 = 5733.333mV
			//V2 = V1 * G2 = 8565.6mV
			//200000mA : 8565.6mV = I1 : 10000mV
			//I1 = 233492.10796675mA
			//ratio = 233492107.96675uA / 32768 = 7125.613646
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5733.333mV
			//V2 = V1 * G2 = 8565.6mV
			//100000mA : 8565.6mV = I1 : 10000mV
			//I1 = 116746.053983375mA
			//ratio = 116746053.983375uA / 32768 = 3562.806823
			
			if((range+1) == RANGE1) ratio = 7125.613646;
			else ratio = 3562.806823;
			break;
		case C_KTL_600V_200A_100A_20A_400KW:	//jhk_180731
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//200000mA : 8947.4mV = I1 : 10000mV
			//I1 = 223528.62284015468mA
			//ratio = 223528622.84015468uA / 32768 = 6821.5522106
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142008mA
			//ratio = 111764311.42008uA / 32768 = 3410.776105349
			
			//RANGE3
			//ITB300-S 300A:150mA = 20A:x -> x = 10mA
			//shuntV = 10mA * 50ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//20000mA : 8947.4mV = I1 : 10000mV
			//I1 = 22352.862284mA
			//ratio = 22352862.284uA / 32768 = 682.155221069808
			
			if((range+1) == RANGE1) ratio = 6821.5522106;
			else if ((range+1) == RANGE2) ratio = 3410.776105349;
			else ratio = 682.155221069808;
			break;
		case C_SEBANG_600V_200A_100A_240KW:	//jhk_160223
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//200000mA : 8947.4mV = I1 : 10000mV
			//I1 = 223528.62284015468mA
			//ratio = 223528622.84015468uA / 32768 = 6821.5522106
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142008mA
			//ratio = 111764311.42008uA / 32768 = 3410.776105349
			
			if((range+1) == RANGE1) ratio = 6821.5522106;
			else ratio = 3410.776105349;
			break;
		case C_SKC_600V_200A_100A_50A_240KW: //jhkw_120807
		case C_SKC_600V_200A_100A_50A_240KW_2: //jhkw_121019
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//200000mA : 8947.4mV = I1 : 10000mV
			//I1 = 223528.62284015468mA
			//ratio = 223528622.84015468uA / 32768 = 6821.5522106
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142008mA
			//ratio = 111764311.42008uA / 32768 = 3410.776105349
			
			//RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 20ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//50000mA : 8947.4mV = I1 : 10000mV
			//I1 = 55882.155710039mA
			//ratio = 55882155.710039uA / 32768 = 1705.3880526745
			
			if((range+1) == RANGE1) ratio = 6821.5522106;
			else if ((range+1) == RANGE2) ratio = 3410.776105349;
			else ratio = 1705.3880526745;
			break;
		case C_LGE_600V_250A_50A_150KW:	//jhk_150520
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.538413582mA
			//ratio = 289968538.413582uA / 32768 = 8849.137524829
			                                                                                //RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 25ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//50000mA : 8621.625mV = I1 : 10000mV
			//I1 = 57993.707682716mA
			//ratio = 57993707.682716uA / 32768 = 1769.8275049657

			if((range+1) == RANGE1) ratio = 8849.137524829;
			else ratio = 1769.8275049657;
			break;
		case C_SDI_600V_300A_100A_90KW:		//ktg_200809
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.9777778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 2.5ohm = 500mV
			//V1 = 500mV * G1 = 5988.8889mV
			//V2 = V1 * G2 = 8947.4mV
			//300000mA : 8947.4mV = I1 : 10000mV
			//I1 = 335292.9342602mA
			//ratio = 335292934.2602uA / 32768 = 10232.328316047
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 7.5ohm = 500mV
			//V1 = 500mV * G1 = 5988.8889mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.3114200773mA
			//ratio = 111764311.4200773uA / 32768 = 3410.776105349
			
			if((range+1) == RANGE1) ratio = 10232.328316047;
			else ratio = 3410.776105349;
			break;
		case C_UTP_600V_300A_100A_150KW:	//jhk_151107
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//300000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.944243878588
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//100000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 110766.15366mA
			//ratio = 110766153.66uA / 32768 = 3380.314747959529

			if((range+1) == RANGE1) ratio = 10140.944243878588;
			else ratio = 3380.314747959529;
			break;
		//case C_LGC_600V_300A_100A_360KW:	//ktg_190620	//shh_231025
		case C_INTS_600V_300A_100A_180KW:			//shh_200513 
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 5ohm= 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//300000mA : 8874.36mV = I1 : 10000mV
			//I1 = 338052.546887888mA
			//ratio = 338052546.887888uA / 32768 = 10316.54501
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 15ohm = 1000mV
			//V1 = 1000mV * G1 = 5940mV
			//V2 = V1 * G2 = 8874.36mV
			//100000mA : 8874.36mV = I1 : 10000mV
			//I1 = 112684.18229596mA
			//ratio = 112684182.29596uA / 32768 = 3438.84833667
			
			if((range+1) == RANGE1) ratio = 10316.54501;
			else ratio = 3438.84833667;
			break;
		case C_LGC_600V_300A_100A_360KW_2:	//ktg_200104
		case C_LGC_600V_300A_100A_360KW_3:	//ktg_200108
		case C_LGC_600V_300A_100A_360KW_4:	//ktg_200108
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5kohm = 10.88
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 2.5ohm= 500mV
			//V1 = 500mV * G1 = 5440mV
			//V2 = V1 * G2 = 8127.36mV
			//300000mA : 8127.36mV = I1 : 10000mV
			//I1 = 369123.5530356721mA
			//ratio = 369123553.0356721uA / 32768 = 11264.7568675437
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 7.5ohm = 500mV
			//V1 = 500mV * G1 = 5440mV
			//V2 = V1 * G2 = 8127.36mV
			//100000mA : 8127.36mV = I1 : 10000mV
			//I1 = 123041.184345224mA
			//ratio = 123041184.345224uA / 32768 = 3754.918955847901
			
			if((range+1) == RANGE1) ratio = 11264.7568675437;
			else ratio = 3754.918955847901;
			break;
		case C_SEINENG_600V_300A_100A_30A_500KW:	//ktg_200112
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5kohm = 10.88
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 2.5ohm= 500mV
			//V1 = 500mV * G1 = 5440mV
			//V2 = V1 * G2 = 8127.36mV
			//300000mA : 8127.36mV = I1 : 10000mV
			//I1 = 369123.5530356721mA
			//ratio = 369123553.0356721uA / 32768 = 11264.7568675437
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 7.5ohm = 500mV
			//V1 = 500mV * G1 = 5440mV
			//V2 = V1 * G2 = 8127.36mV
			//100000mA : 8127.36mV = I1 : 10000mV
			//I1 = 123041.184345224mA
			//ratio = 123041184.345224uA / 32768 = 3754.918955847901
			
			//RANGE3
			//ITN600-S 600A:400mA = 30A:x -> x = 20mA
			//shuntV = 20mA * 25ohm = 500mV
			//V1 = 500mV * G1 = 5440mV
			//V2 = V1 * G2 = 8127.36mV
			//30000mA : 8127.36mV = I1 : 10000mV
			//I1 = 36912.35530356721mA
			//ratio = 36912355.30356721uA / 32768 = 1126.47568675437
			
			if((range+1) == RANGE1) ratio = 11264.7568675437;
			else if((range+1) == RANGE2) ratio = 3754.918955847901;
			else ratio = 1126.47568675437;
			break;
		case C_SKE_600V_400A_100A_50A_25A_240KW:	//jhkw_130924
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774193548
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//MH-500 600A:400mA = 400A:x -> x = 266.67mA
			//shuntV = 266.67mA * 2.5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//400000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 447834.5034815521mA
			//ratio = 447834503.4815521uA / 32768 = 13666.82444706885
			
			//RANGE2
			//MH-500 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//100000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 111958.625870388mA
			//ratio = 111958625.870388uA / 32768 = 3416.706111767
			
			//RANGE3
			//MH-500 600A:400mA = 50A:x -> x = 33.33mA
			//shuntV = 33.33mA * 20ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//50000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 55979.312935194mA
			//ratio = 55979312.935194uA / 32768 = 1708.3530558836
			
			//RANGE4
			//MH-500 600A:400mA = 25A:x -> x = 16.667mA
			//shuntV = 16.667mA * 40ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//25000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 27989.656467597mA
			//ratio = 27989656.467597uA / 32768 = 854.1765279418
			
			if((range+1) == RANGE1) ratio = 13666.82444706885;
			else if ((range+1) == RANGE2) ratio = 3416.706111767;
			else if ((range+1) == RANGE3) ratio = 1708.3530558836;
			else ratio = 854.1765279418;
			break;
		case C_DAEHWA_600V_400A_200A_100A_200KW:	//jhk_170120
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774193548
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN-600 600A:400mA = 400A:x -> x = 266.67mA
			//shuntV = 266.67mA * 2.5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//400000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 447834.5034815521mA
			//ratio = 447834503.4815521uA / 32768 = 13666.82444706885
			
			//RANGE2
			//ITN-600 600A:400mA = 200A:x -> x = 133.33mA
			//shuntV = 133.33mA * 5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//200000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 223917.25174mA
			//ratio = 223917251.74uA / 32768 = 6833.412223534425
			
			//RANGE3
			//ITN-600 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//100000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 111958.625870388mA
			//ratio = 111958625.870388uA / 32768 = 3416.706111767
			
			if((range+1) == RANGE1) ratio = 13666.82444706885;
			else if ((range+1) == RANGE2) ratio = 6833.412223534425;
			else ratio = 3416.706111767;
			break;
		/*
		case C_HYUNDAI_600V_400A_200A_100A_240KW:	//khj_190805	
		//case C_HYUNDAI_600V_400A_200A_100A_240KW_2:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_3:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_4:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_5:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_6:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_7:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_8:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_9:	//khj_190805
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.17kohm = 9.00648298
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN-600 600A:400mA = 400A:x -> x = 266.67mA
			//shuntV = 266.67mA * 2.5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 6004.3249902755mV
			//V2 = V1 * G2 = 8970.4615354716mV
			//400000mA : 8970.4615354716mV = I1 : 10000mV
			//I1 = 445907.9373099mA
			//ratio = 445907937.3099uA / 32768 = 13608.0303134126
			
			//RANGE2
			//ITN-600 600A:400mA = 200A:x -> x = 133.33mA
			//shuntV = 133.33mA * 5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 6004.32499mV
			//V2 = V1 * G2 = 8970.461535mV
			//200000mA : 8970.461535mV = I1 : 10000mV
			//I1 = 222953.96866667mA
			//ratio = 222953968.66667uA / 32768 = 6804.015157064
			
			//RANGE3
			//ITN-600 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 6004.32499mV
			//V2 = V1 * G2 = 8970.461535mV
			//100000mA : 8970.461535mV = I1 : 10000mV
			//I1 = 111476.9843296mA
			//ratio = 111476984.3296uA / 32768 = 3402.007578
			
			if((range+1) == RANGE1) ratio = 13608.0303134126;
			else if ((range+1) == RANGE2) ratio = 6804.015157064;
			else ratio = 3402.007578;
			break;
			*/
		case C_DAEHWA_750V_300A_100A_150KW:	//jhk_170203
		case C_HANGKE_750V_300A_100A_300KW:		//jhk_131226
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//300000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.9442438786
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//100000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 110766.15366mA
			//ratio = 110766153.66uA / 32768 = 3380.3147479595

			if((range+1) == RANGE1) ratio = 10140.9442438786;
			else ratio = 3380.3147479595;
			break;
		case C_HYUNDAI_750V_400A_100A_240KW:	//jhk_160909
		case C_HYUNDAI_750V_400A_100A_240KW_2:	//jhk_161111
		case C_HYUNDAI_750V_400A_100A_240KW_3:	//jhk_171015
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774193548
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//MH-500 600A:400mA = 400A:x -> x = 266.67mA
			//shuntV = 266.67mA * 2.5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//400000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 447834.5034815521mA
			//ratio = 447834503.4815521uA / 32768 = 13666.82444706885
			
			//RANGE2
			//MH-500 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//100000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 111958.625870388mA
			//ratio = 111958625.870388uA / 32768 = 3416.706111767
			
			if((range+1) == RANGE1) ratio = 13666.82444706885;
			else ratio = 3416.706111767;
			break;
		case C_STECHWIN_800V_200A_50A_320KW:	//jhk_150812
		case C_DAEHWA_800V_200A_50A_160KW:	//jhk_160831
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//200000mA : 8947.4mV = I1 : 10000mV
			//I1 = 223528.62284015468mA
			//ratio = 223528622.84015468uA / 32768 = 6821.5522106
			
			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 20ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//50000mA : 8947.4mV = I1 : 10000mV
			//I1 = 55882.155710039mA
			//ratio = 55882155.710039uA / 32768 = 1705.3880526745
			
			if((range+1) == RANGE1) ratio = 6821.5522106;
			else ratio = 1705.3880526745;
			break;
		case C_CS_800V_250A_50A_250KW:	//jhk_150812
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.8kohm = 9.5172413793
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5948.275862mV
			//V2 = V1 * G2 = 8886.72413793mV
			//250000mA : 8886.72413793mV = I1 : 10000mV
			//I1 = 281318.510758mA
			//ratio = 281318510.758uA / 32768 = 8585.159630066
			
			//RANGE2
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 25ohm = 625mV
			//V1 = 625mV * G1 = 5948.275862mV
			//V2 = V1 * G2 = 8886.72413793mV
			//50000mA : 8886.72413793mV = I1 : 10000mV
			//I1 = 56263.7021516mA
			//ratio = 56263702.1516uA / 32768 = 1717.031926

			if((range+1) == RANGE1) ratio = 8585.159630066;
			else ratio = 1717.031926;
			break;
		case C_ADD_800V_350A_200A_50A_380KW:			//shh_200904	//ktg_201028
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.967742
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 350A:x -> x = 233.333mA
			//shuntV = 233.333mA * 3ohm = 700mV
			//V1 = 700mV * G1 = 6277.419355mV
			//V2 = V1 * G2 = 9378.464516mV
			//350000mA : 9378.464516129mV = I1 : 10000mV
			//I1 = 373195.41956796mA
			//ratio = 373195419.56796uA / 32768 = 11389.02037256
			
			//RANGE2
			//ITN600-S 600A:400mA = 200A:x -> x = 133.3333mA
			//shuntV = 133.3333mA * 5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//200000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 223917.251740776mA
			//ratio = 223917251.740776uA / 32768 = 6833.412223
			
			//RANGE3
			//ITN600-S 600A:400mA = 50A:x -> x = 33.3333333mA
			//shuntV = 33.3333mA * 20ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//50000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 55979.3129352mA
			//ratio = 55979312.9352uA / 32768 = 1708.35305588
			
			if((range+1) == RANGE1) ratio = 11389.02037256;
			else if((range+1) == RANGE2) ratio = 6833.412223;
			else ratio = 1708.35305588;
			break;
		case C_POWERLOGICS_1000V_200A_100A_200KW:	//jhk_180921
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//200000mA : 8947.4mV = I1 : 10000mV
			//I1 = 223528.62284015468mA
			//ratio = 223528622.84015468uA / 32768 = 6821.5522106
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142008mA
			//ratio = 111764311.42008uA / 32768 = 3410.776105349
			
			if((range+1) == RANGE1) ratio = 6821.5522106;
			else ratio = 3410.776105349;
			break;
		case C_HYUNDAI_1000V_250A_100A_500KW:	//shh_200513
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 3ohm = 500mV
			//V1 = 500mV * G1 = 5988.89mV
			//V2 = V1 * G2 = 8947.40166mV
			//250000mA : 8947.4mV = I1 : 10000mV
			//I1 = 279410.7267114685mA
			//ratio = 279410726.7114685uA / 32768 = 8526.938681380266 
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 7.5ohm = 500mV
			//V1 = 500mV * G1 = 5988.89mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.2906845874mA 
			//ratio = 111764290.6845874uA / 32768 = 3410.775472552106
			
			if((range+1) == RANGE1) ratio = 8526.938681380266;
			else ratio = 3410.775472552106;
			break;
		case C_KOSTA_1000V_250A_100A_600KW:		//shh_201102
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600-S 600A:400mA = 250A:x -> x = 166.667mA
			//shuntV = 166.667mA * 3ohm = 500mV
			//V1 = 500mV * G1 = 5988.89mV
			//V2 = V1 * G2 = 8947.40166mV
			//250000mA : 8947.4mV = I1 : 10000mV
			//I1 = 279410.7267114685mA
			//ratio = 279410726.7114685uA / 32768 = 8526.938681380266 
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 7.5ohm = 500mV
			//V1 = 500mV * G1 = 5988.89mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.2906845874mA 
			//ratio = 111764290.6845874uA / 32768 = 3410.775472552106
			
			if((range+1) == RANGE1) ratio = 8526.938681380266;
			else ratio = 3410.775472552106;
			break;
		case C_KBTP_1000V_300A_100A_300KW:	//ljh_210329
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN600-S 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 2.5ohm = 500mV
			//V1 = shuntV * G1 = 5988.89mV
			//V2 = V1 * G2 = 8947.4mV
			//300000mA : 8947.4mV = I1 : 10000mV
			//I1 = 335292.93426mA
			//ratio = 335292934.26uA / 32768 = 10232.328316
			
			//RANGE2
			//ITN600-S 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 7.5ohm = 500mV
			//V1 = shuntV * G1 = 5988.89mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142mA
			//ratio = 111764344.42uA / 32768 = 3410.77610535
			
			if((range+1) == RANGE1) ratio = 10232.328316;
			else ratio = 3410.77610535;
			break;
		case C_HLGP_1000V_300A_100A_50A_600KW:	//jhk_180810
		case C_HLGP_1000V_300A_100A_50A_600KW_2:	//jhk_180917
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//300000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.9442438786
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//100000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 110766.15366mA
			//ratio = 110766153.66uA / 32768 = 3380.3147479595
			
			//RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//50000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 55383.07683mA
			//ratio = 55383076.83uA / 32768 = 1690.1573739797646
			
			if((range+1) == RANGE1) ratio = 10140.9442438786;
			else if((range+1) == RANGE2) ratio = 3380.3147479595;
			else ratio = 1690.1573739797646;
			break;
		case C_HYUNDAI_1000V_500A_250A_500KW:	//jhk_180722
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.8kohm = 9.5172413793
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5948.275862mV
			//V2 = V1 * G2 = 8886.72413793mV
			//250000mA : 8886.72413793mV = I1 : 10000mV
			//I1 = 281318.51mA
			//ratio = 281318510.758uA / 32768 = 8585.159630066158
			
			//RANGE2
			//ITB300-S 300A:150mA = 125A:x -> x = 62.5mA
			//shuntV = 62.5mA * 10ohm = 625mV
			//V1 = 625mV * G1 = 5948.275862mV
			//V2 = V1 * G2 = 8886.72413793mV
			//125000mA : 8886.72413793mV = I1 : 10000mV
			//I1 = 140659.255379mA
			//ratio = 140659255.379uA / 32768 = 4292.579815033079
			
			if((range+1) == RANGE1) ratio = 8585.159630066158;
			else ratio = 4292.579815033079;
			break;
		case C_BOSUNG_1200V_200A_100A_200KW:	//jhk_170106
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITB300-S 300A:150mA = 200A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//200000mA : 8947.4mV = I1 : 10000mV
			//I1 = 223528.62284015468mA
			//ratio = 223528622.84015468uA / 32768 = 6821.5522106
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 10ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142008mA
			//ratio = 111764311.42008uA / 32768 = 3410.776105349
			
			if((range+1) == RANGE1) ratio = 6821.5522106;
			else ratio = 3410.776105349;
			break;
		case C_SKI_1200V_250A_100A_50A_300KW:	//jhk_131209
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.233
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB300-S 300A:150mA = 250A:x -> x = 125mA
			//shuntV = 125mA * 5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//250000mA : 8621.625mV = I1 : 10000mV
			//I1 = 289968.538413582mA
			//ratio = 289968538.413582uA / 32768 = 8849.137524829
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 12.5ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//100000mA : 8621.625mV = I1 : 10000mV
			//I1 = 115987.41536543285mA
			//ratio = 115987415.36543285uA / 32768 = 3539.655009931
			                                                                                //RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 25ohm = 625mV
			//V1 = 625mV * G1 = 5770.833333333mV
			//V2 = V1 * G2 = 8621.625mV
			//50000mA : 8621.625mV = I1 : 10000mV
			//I1 = 57993.707682716mA
			//ratio = 57993707.682716uA / 32768 = 1769.8275049657

			if((range+1) == RANGE1) ratio = 8849.137524829;
			else if ((range+1) == RANGE2) ratio = 3539.655009931;
			else ratio = 1769.8275049657;
			break;
		case C_KTL_1200V_300A_100A_300KW:	//kjh_171013
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//300000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.9442438786
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//100000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 110766.15366mA
			//ratio = 110766153.66uA / 32768 = 3380.3147479595
			
			//RANGE3
			//ITB300-S 300A:150mA = 50A:x -> x = 25mA
			//shuntV = 25mA * 30ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//50000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 55383.07683mA
			//ratio = 55383076.83uA / 32768 = 1690.1573739797646
			
			//RANGE4
			//ITB300-S 300A:150mA = 10A:x -> x = 5mA
			//shuntV = 5mA * 150ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//10000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 11076.615366mA
			//ratio = 11076615.366uA / 32768 = 338.03147479595

			if((range+1) == RANGE1) ratio = 10140.9442438786;
			else if((range+1) == RANGE2) ratio = 3380.3147479595;
			else if((range+1) == RANGE3) ratio = 1690.1573739797646;
			else ratio = 338.03147479595;
			break;
		case C_KATECH_1200V_300A_100A_50A_300KW:	//jhk_150108
		case C_SEBANG_1200V_300A_100A_360KW:	//jhk_160223
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITB300-S 300A:150mA = 300A:x -> x = 150mA
			//shuntV = 150mA * 5ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//300000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 332298.4609834mA
			//ratio = 332298460.9834uA / 32768 = 10140.9442438786
			
			//RANGE2
			//ITB300-S 300A:150mA = 100A:x -> x = 50mA
			//shuntV = 50mA * 15ohm = 750mV
			//V1 = 750mV * G1 = 6042.857142857mV
			//V2 = V1 * G2 = 9028.02857mV
			//100000mA : 9028.02857mV = I1 : 10000mV
			//I1 = 110766.15366mA
			//ratio = 110766153.66uA / 32768 = 3380.3147479595
			
			if((range+1) == RANGE1) ratio = 10140.9442438786;
			else ratio = 3380.3147479595;
			break;
		case C_KATECH_1200V_1000A_500A_300A_600KW:	//shh_210521
		case C_KATECH_1200V_1000A_500A_300A_600KW_2:	//shh_210521
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.978
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN1000-S 1500A:1000mA = 1000A:x -> x = 666.667mA
			//shuntV = 666.667mA * 1ohm = 666.667mV
			//V1 = 666.667mV * G1 = 7985.18518518mV
			//V2 = V1 * G2 = 11929.86666666mV
			//1000000mA : 11929.86666666mV = I1 : 10000mV
			//I1 = 838232.3356505798mA
			//ratio = 838232335.6505798uA / 32768 = 25580.82079011 
			
			//RANGE2
			//ITN1000-S 1500A:1000mA = 500A:x -> x = 333.333mA
			//shuntV = 333.333mA * 2ohm = 666.667mV
			//V1 = 666.667mV * G1 = 7985.18518518mV
			//V2 = V1 * G2 = 11929.86666666mV
			//500000mA : 11929.86666666mV = I1 : 10000mV
			//I1 = 419116.1678252899mA
			//ratio = 419116167.8252899uA / 32768 = 12790.41039505 
			
			//RANGE3
			//ITN1000-S 1500A:1000mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 3.33ohm = 666.667mV
			//V1 = 666.667mV * G1 = 7985.18518518mV
			//V2 = V1 * G2 =  11929.86666666mV
			//300000mA :  11929.86666666mV = I1 : 10000mV
			//I1 = 251469.7006951739mA
			//ratio = 251469700.6951739uA / 32768 = 7674.24623703 
			
			if((range+1) == RANGE1) ratio = 25580.82079011;
			else if((range+1) == RANGE2) ratio = 12790.41039505;
			else ratio = 7674.24623703;
			break;
		case C_KCL_1200V_1000A_400A_600KW:		//shh_200809 	
		case C_KCL_1200V_1000A_400A_600KW_2:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_600KW_3:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_600KW_4:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_700KW:		//shh_200809 	
		case C_KCL_1200V_1000A_400A_700KW_2:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_800KW:		//shh_200809 	
		case C_KCL_1200V_1000A_400A_800KW_2:	//shh_200809 
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN1000-S 1000A:1000mA = 250A:x -> x = 250mA
			//shuntV = 250mA * 2ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//250000mA : 8947.4mV = I1 : 10000mV
			//I1 = 279410.7784983546mA
			//ratio = 279410778.49835uA / 32768 = 8526.94026179 
			
			//RANGE2
			//ITN1000-S 1000A:1000mA = 100A:x -> x = 100mA
			//shuntV = 100mA * 5ohm = 500mV
			//V1 = 500mV * G1 = 5988.888mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.3113993418mA
			//ratio = 111764311.3993uA / 32768 = 3410.7761047162
			
			if((range+1) == RANGE1) ratio = 8526.94026179;
			else ratio = 3410.7761047162;
			break;
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_2:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_3:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_4:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_5:	//ktg_190814
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.17kohm = 9.00648
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN1000-S 1500A:1000mA = 1000A:x -> x = 666.667mA
			//shuntV = 666.667mA * 1ohm = 666.667mV
			//V1 = 666.667mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.45705mV
			//1000000mA : 8970.45705mV = I1 : 10000mV
			//I1 = 1114770.40066mA
			//ratio = 1114770400.66uA / 32768 = 34020.092793569
			
			//RANGE2
			//ITN1000-S 1500A:1000mA = 500A:x -> x = 333.333mA
			//shuntV = 333.333mA * 2ohm = 666.667mV
			//V1 = 666.667mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.45705mV
			//500000mA : 8970.45705mV = I1 : 10000mV
			//I1 = 557385.2003298mA
			//ratio = 557385200.3298uA / 32768 = 17010.04639678
			
			//RANGE3
			//ITN1000-S 1500A:1000mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 3.33ohm = 666.667mV
			//V1 = 666.667mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.45705mV
			//300000mA : 8970.45705mV = I1 : 10000mV
			//I1 = 334431.120197905mA
			//ratio = 334431120.197905uA / 32768 = 10206.027838
			
			if((range+1) == RANGE1) ratio = 34020.092793569;
			else if((range+1) == RANGE2) ratio = 17010.04639678;
			else ratio = 10206.027838;
			break;
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_6:	//shh_210625
		case C_HYUNDAI_1200V_1000A_500A_300A_800KW:	//shh_210915
		case C_HYUNDAI_1200V_1000A_500A_300A_800KW_2:	//shh_211020
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.18kohm = 8.99352750
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN1000-S 1500A:1000mA = 1000A:x -> x = 666.667mA
			//shuntV = 666.667mA * 1ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5995.685005393mV
			//V2 = V1 * G2 = 8957.55339805mV
			//1000000mA : 8957.55339805mV = I1 : 10000mV
			//I1 = 1116376.264323216mA
			//ratio = 1116376264.323216uA / 32768 = 34069.09986337
			
			//RANGE2
			//ITN1000-S 1500A:1000mA = 500A:x -> x = 333.333mA
			//shuntV = 333.333mA * 2ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5995.685005393mV
			//V2 = V1 * G2 = 8957.55339805mV
			//500000mA : 8957.55339805mV = I1 : 10000mV
			//I1 = 558188.1321616079mA
			//ratio = 557385200.3298uA / 32768 = 17034.549931689
			
			//RANGE3
			//ITN1000-S 1500A:1000mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 3.33ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5995.685005393mV
			//V2 = V1 * G2 = 8957.55339805mV
			//300000mA : 8957.55339805mV = I1 : 10000mV
			//I1 = 334912.8792972733mA
			//ratio = 334431120.197905uA / 32768 = 10220.729959023
			
			if((range+1) == RANGE1) ratio = 34069.09986337;
			else if((range+1) == RANGE2) ratio = 17034.549931689;
			else ratio = 10220.729959023;
			break;
		case C_UTP_1200V_1000A_500A_300A_600KW:			//shh_201120
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN1000-S 1500A:1000mA = 1000A:x -> x = 666.667mA
			//shuntV = 666.667mA * 1ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.49462365mV
			//V2 = V1 * G2 = 8931.87096774mV
			//1000000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 1119586.25870388mA
			//ratio = 1119586258.70388026uA / 32768 = 34167.06111767
			
			//RANGE2
			//ITN1000-S 1500A:1000mA = 500A:x -> x = 333.333mA
			//shuntV = 333.333mA * 2ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.49462365mV
			//V2 = V1 * G2 = 8931.87096774mV
			//500000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 559793.12935194mA
			//ratio = 559793129.35194013uA / 32768 = 17083.53055883
			
			//RANGE3
			//ITN1000-S 1500A:1000mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 3.33ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.49462365mV
			//V2 = V1 * G2 = 8931.87096774mV
			//300000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 335875.877611165mA
			//ratio = 335875877.61116408uA / 32768 = 10250.118335301
			
			if((range+1) == RANGE1) ratio = 34167.06111767;
			else if((range+1) == RANGE2) ratio = 17083.53055883;
			else ratio = 10250.118335301;
			break;
		case C_NCT_1500V_250A_100A_400KW:	//ljh_201113
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN600-S 300A:200mA = 250A:x -> x = 166.67mA
			//shuntV = 166.67mA * 3ohm = 500mV
			//V1 = shuntV * G1 = 5988.89mV
			//V2 = V1 * G2 = 8947.4mV
			//250000mA : 8947.4mV = I1 : 10000mV
			//I1 = 279410.77855019335mA
			//ratio = 279410778.55019335uA / 32768 = 8526.94026337259986
			
			//RANGE2
			//ITN600-S 300A:200mA = 100A:x -> x = 66.67mA
			//shuntV = 66.67mA * 7.5ohm = 500mV
			//V1 = shuntV * G1 = 5988.89mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.311420077341mA
			//ratio = 111764311.420077341uA / 32768 = 3410.77610534904
			if((range+1) == RANGE1) ratio = 8526.94026337259986;
			else ratio = 3410.77610534904;
			break;
		case C_SKI_1500V_400A_100A_500KW:	//jhk_180610
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774193548
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600s 600A:400mA = 400A:x -> x = 266.67mA
			//shuntV = 266.67mA * 2.5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//400000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 447834.5034815521mA
			//ratio = 447834503.4815521uA / 32768 = 13666.82444706885
			
			//RANGE2
			//ITN600s 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//100000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 111958.625870388mA
			//ratio = 111958625.870388uA / 32768 = 3416.706111767
			
			if((range+1) == RANGE1) ratio = 13666.82444706885;
			else ratio = 3416.706111767;
			break;
		case C_KTL_1500V_400A_200A_700KW:	//jhk_180902
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774193548
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600s 600A:400mA = 400A:x -> x = 266.67mA
			//shuntV = 266.67mA * 2.5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//400000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 447834.5034815521mA
			//ratio = 447834503.4815521uA / 32768 = 13666.82444706885
			
			//RANGE2
			//ITN600s 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 5ohm = 666.667mV
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//200000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 223917.251740776mA
			//ratio = 223917251.74uA / 32768 = 6833.412223534425
			
			if((range+1) == RANGE1) ratio = 13666.82444706885;
			else ratio = 6833.412223534425;
			break;
		case C_KTC_1500V_600A_200A_400KW:	//ktg_200807
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600s 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 2.5ohm = 500mV
			//V1 = 500mV * G1 = 5988.8889mV
			//V2 = V1 * G2 = 8947.4mV
			//300000mA : 8947.4mV = I1 : 10000mV
			//I1 = 335292.934260232mA
			//ratio = 335292934.260232uA / 32768 = 10232.328316047
			
			//RANGE2
			//ITN600s 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 7.5ohm = 500mV
			//V1 = 500mV * G1 = 5988.8889mV
			//V2 = V1 * G2 = 8947.4mV
			//100000mA : 8947.4mV = I1 : 10000mV
			//I1 = 111764.31142007734mA
			//ratio = 111764311.42007734uA / 32768 = 3410.776105349
			
			if((range+1) == RANGE1) ratio = 10232.328316047;
			else ratio = 3410.776105349;
			break;
		case C_CTP_1500V_600A_300A_60A_600KW:	//ktg_191117
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.6kohm = 7.5
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600s 600A:400mA = 600A:x -> x = 400mA
			//shuntV = 400mA * 2ohm = 800mV
			//V1 = 800mV * G1 = 6000mV
			//V2 = V1 * G2 = 8964mV
			//600000mA : 8964mV = I1 : 10000mV
			//I1 = 669344.042838mA
			//ratio = 669344042.838018uA / 32768 = 20426.7591198
			
			//RANGE2
			//ITN600s 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 4ohm = 800mV
			//V1 = 800mV * G1 = 6000mV
			//V2 = V1 * G2 = 8964mV
			//300000mA : 8964mV = I1 : 10000mV
			//I1 = 334672.021419009mA
			//ratio = 334672021.419009uA / 32768 = 10213.37955991
			
			//RANGE3
			//ITN600s 600A:400mA = 60A:x -> x = 40mA
			//shuntV = 40mA * 20ohm = 800mV
			//V1 = 800mV * G1 = 6000mV
			//V2 = V1 * G2 = 8964mV
			//60000mA : 8964mV = I1 : 10000mV
			//I1 = 66934.4042838mA
			//ratio = 66934404.2838uA / 32768 = 2042.675911981
			
			if((range+1) == RANGE1) ratio = 20426.7591198;
			else if((range+1) == RANGE2) ratio = 10213.37955991;
			else ratio = 2042.675911981;
			break;
		case C_KEIR_1500V_600A_300A_200A_100A_720KW:	//ktg_211202
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.6kohm = 7.5
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600s 600A:400mA = 600A:x -> x = 400mA
			//shuntV = 400mA * 2ohm = 800mV
			//V1 = 800mV * G1 = 6000mV
			//V2 = V1 * G2 = 8964mV
			//600000mA : 8964mV = I1 : 10000mV
			//I1 = 669344.042838mA
			//ratio = 669344042.838018uA / 32768 = 20426.7591198
			
			//RANGE2
			//ITN600s 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 4ohm = 800mV
			//V1 = 800mV * G1 = 6000mV
			//V2 = V1 * G2 = 8964mV
			//300000mA : 8964mV = I1 : 10000mV
			//I1 = 334672.021419009mA
			//ratio = 334672021.419009uA / 32768 = 10213.37955991
			
			//RANGE3
			//ITN600s 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 6ohm = 800mV
			//V1 = 800mV * G1 = 6000mV
			//V2 = V1 * G2 = 8964mV
			//200000mA : 8964mV = I1 : 10000mV
			//I1 = 223114.6809460062mA
			//ratio = 223114680.9460062uA / 32768 = 6808.919706604195
			
			//RANGE4
			//ITN600s 600A:400mA = 100A:x -> x = 66.666mA
			//shuntV = 66.666mA * 12ohm = 800mV
			//V1 = 800mV * G1 = 6000mV
			//V2 = V1 * G2 = 8964mV
			//100000mA : 8964mV = I1 : 10000mV
			//I1 = 111557.3404730031mA
			//ratio = 111557340.4730031uA / 32768 = 3404.459853302097
			
			if((range+1) == RANGE1) ratio = 20426.7591198;
			else if((range+1) == RANGE2) ratio = 10213.37955991;
			else if((range+1) == RANGE3) ratio = 6808.919706604195;
			else ratio = 3404.459853302097;
			break;
		case C_KCL_1500V_1000A_500A_600KW:	//jhk_181110
		case C_HYUNDAI_1500V_1000A_500A_600KW:	//jhk_170917
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.49kohm = 12.002227
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//IT1000-S 1000A:1000mA = 1000A:x -> x = 1000mA
			//shuntV = 1000mA * 0.5ohm = 500mV
			//V1 = 500mV * G1 = 6001.113585746mV
			//V2 = V1 * G2 = 8965.663697mV
			//1000000mA : 8965.663697mV = I1 : 10000mV
			//I1 = 1115366.395mA
			//ratio = 1115366395.37uA / 32768 = 34038.281108910188
			
			//RANGE2
			//IT1000-S 1000A:1000mA = 500A:x -> x = 500mA
			//shuntV = 500mA * 1ohm = 500mV
			//V1 = 500mV * G1 = 6001.113585746mV
			//V2 = V1 * G2 = 8965.663697mV
			//500000mA : 8965.663697mV = I1 : 10000mV
			//I1 = 557683.197688mA
			//ratio = 557683197.688uA / 32768 = 17019.140554455094
			
			if((range+1) == RANGE1) ratio = 34038.281108910188;
			else ratio = 17019.140554455094;
			break;
		default:
			//AMP -> INA128UA
			//ratio = 1.0;

			main_amp = myPs->config.main_amp;	//1.494	//kjh_211021s
			maxI = myPs->config.maxI[range];
			da_max = myPs->config.da_max;
			i_feed = myPs->config.i_feed[range];

			if(main_amp <= 0.0 || maxI <= 0 || da_max <= 0.0 || i_feed <= 0.0) {
				ratio = 0.0;
			} else {
				ratio = i_feed * main_amp;
				ratio = (double)maxI * (da_max / ratio);
				ratio /= 32768;
			} //kjh_211021e
			break;
	}

	return ratio;
}

double Ref_V_Value(void)
{
	double ret_val;
	long maxV;  //kjh_211021
	double v_feed, ref; //kjh_211021

	switch(myData->AppControl.config.systemModel) {
		case C_LGC_50V_120A_10A:
		case C_LGC_50V_200A_10A:
			//ret_val = 5000000.0 * 218.0 / 18.0;
			ret_val = (double)60555555.56;
			break;
		case C_SKI_50V_500A_400A_200A_100A_100KW:	//jhk_180119
		case C_SKI_50V_500A_400A_200A_100A_100KW_2:	//jhk_180119
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_ENERTECH_55V_100A_10A: //kjg_w
			//ret_val = 5000000.0 * 220.0 / 20.0;
			ret_val = (double)55000000.0;
			break;
		case C_SNU_60V_200A_100A_50A_1CH_12KW:  //jhkw_130221
		//case C_LGC_60V_300A_30A_36KW:	//jhk_150406
		//case C_LGC_60V_300A_30A_36KW_2:	//jhk_150406
		//case C_LGC_60V_300A_30A_36KW_3:	//jhk_150406	//shh_231027
		case C_TOPBATT_60V_300A_100A_36KW:	//jhk_150616
		case C_HYUNDAI_60V_300A_100A_36KW:	//jhk_160825
		case C_HYUNDAI_60V_300A_100A_36KW_2:	//jhk_161111
		case C_HYUNDAI_60V_300A_100A_36KW_3:	//jhk_171015
		case C_CANSYSTEM_60V_350A_100A_21KW:	//jhk_140828
		case C_NORTHVOLT_60V_400A_200A_96KW:	//khj_191203
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
		case C_DAEHWA_60V_500A_250A_50A_60KW:	//jhk_160913
		case C_DAEHWA_60V_500A_250A_50A_60KW_2:	//jhk_160913
		//case C_LGC_60V_600A_300A_100A_144KW:	//ktg_190707
		//case C_LGC_60V_600A_300A_100A_144KW_2:	//ktg_191222
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
			//feedback section
			//R1 = 100kohm, R2 = 10kohm // 60kohm = 8.571429kohm
			//V1 = 70000mV * R2 / (R1 + R2) = 5526.316044mV
			//Vmax * Vref / V1 = 70000000uV * 5000000uV / 5526316.044uV
			// = 63333330.42
			ret_val = (double)63333330.42;
			break;
		/*case C_LGC_70V_100A_28KW:	//ljh_201130
		case C_LGC_70V_100A_28KW_2:	//ljh_201130
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 5.25;
			break;*/	//shh_231024
		case C_SDI_70V_250A_25A_18KW:
			//feedback section
			//R1 = 100kohm, R2 = 3kohm + 5.5kohm = 8.5kohm
			//V1 = 70000mV * R2 / (R1 + R2) = 5483.870968mV
			//Vmax * Vref / V1 = 70000000uV * 5000000uV / 5483870.968uV
			// = 63823529.41
			ret_val = (double)63823529.41;
			break;
		case C_LGC_70V_100A_50A_28KW:		//ktg_190405
		case C_LGC_70V_250A_10A_35KW_4:
		case C_LGC_70V_250A_10A_35KW_15:	//ktg_220420
		case C_LGC_70V_250A_10A_35KW_16:
		case C_LGC_70V_250A_10A_35KW_17:	//shh_220214
		case C_LGC_70V_250A_10A_35KW_18:	//shh_220214
		/*case C_LGC_70V_250A_50A_35KW:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_2:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_3:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_4:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_5:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_6:		//jhk_150502
		//case C_LGC_70V_250A_50A_35KW_7:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_8:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_9:		//jhk_160515
		case C_LGC_70V_250A_50A_35KW_10:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_11:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_12:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_13:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_14:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_15:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_16:	//jhk_160515*/	//sec_231026
		case C_LGC_70V_300A_100A_42KW:	//jhk_170211
		case C_LGC_70V_300A_100A_42KW_2:	//jhk_170519
		//case C_LGC_70V_300A_100A_50A_10A_42KW:   //jhkw_120820
		//case C_LGC_70V_300A_100A_50A_10A_42KW_2: //jhkw_120820	//shh_231027
		case C_LGCUSA_70V_300A_42KW:	//jhk_160802
		case C_LGCUSA_70V_300A_42KW_2:	//jhk_160802
		case C_LGCUSA_70V_350A_24KW:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_2:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_3:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_4:	//jhk_160720
		case C_LGC_70V_400A_200A_56KW:			//ktg_190711
		case C_LGC_70V_250A_10A_35KW_6:			//shh_211005
		case C_LGC_70V_250A_10A_35KW_28:	//shh_220614
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_SAEHAN_72V_200A_20A:
			//ret_val = 5000000.0 * 215.0 / 15.0;
			ret_val = (double)71666666.67;
			break;
		case C_NEWPOWER_75V_150A_75A_45KW:	//ljh_201105
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 5.625;
			break;
		case C_SDI_80V_50A_25A_10A:
		case C_SDI_80V_50A_25A_10A_2:
		case C_SDI_80V_50A_25A_10A_3:
		case C_SDI_80V_50A_25A_10A_4:
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 5.714;
			break;
		case C_LGC_90V_300A_100A_108KW:	//jhk_181016
		/*case C_LGC_90V_300A_100A_108KW_2:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_3:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_4:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_5:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_6:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_7:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_8:	//jhk_181016*/
		case C_LGCCHINA_90V_300A_100A_108KW:	//ktg_190319
		case C_LGC_90V_500A_250A_100A_90KW:	//jhk_181107
		//case C_LGC_90V_500A_250A_100A_180KW:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_2:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_3:	//jhk_181006	//sec_221027
		//case C_LGC_90V_500A_250A_100A_180KW_4:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_5:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_6:	//jhk_181006	//ktg_231023
		/*case C_LGC_90V_500A_250A_100A_180KW_7:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_8:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_9:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_10:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_11:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_12:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_13:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_14:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_15:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_16:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_17:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_18:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_19:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_20:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_21:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_22:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_23:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_24:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_25:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_26:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_27:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_28:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_29:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_30:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_31:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_32:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_33:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_34:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_35:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_36:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_37:	//ktg_190219	*/ //ktg_231124
		case C_LGC_90V_500A_250A_100A_180KW_38:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_39:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_40:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_41:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_42:	//ktg_200104
		case C_LGC_90V_500A_300A_100A_180KW:	//ktg_191106
		case C_LGC_90V_500A_300A_100A_180KW_2:	//ktg_191106
		//case C_LGC_90V_500A_300A_100A_180KW_3:	//ljh_201130
		//case C_LGC_90V_500A_300A_100A_180KW_4:	//ljh_201130
		//case C_LGC_90V_500A_300A_100A_180KW_5:	//ljh_201130	//shh_231027
		//case C_LGC_90V_500A_300A_100A_180KW_6:	//khj_200303	//ktg_231124
		//case C_LGC_90V_500A_300A_100A_180KW_7:	//shh_220324
		//ase C_LGC_90V_500A_300A_100A_180KW_8:	//shh_220324
		//case C_LGC_90V_600A_300A_100A_216KW:	//ktg_190707
		//case C_LGC_90V_600A_300A_100A_216KW_2:	//ktg_200709
		//case C_LGC_90V_600A_300A_100A_216KW_3:	//ktg_200709
		//case C_LGC_90V_600A_300A_100A_216KW_4:	//ktg_200709
		case C_GITC_100V_100A_50A_10A_40KW:	//jhk_180917
		case C_POWERLOGICS_100V_100A_50A_40KW:	//jhk_180919
		case C_CORN_100V_100A_50A_40KW:	//ktg_190210
		case C_SEBANG_100V_100A_50A_10A_40KW:	//ktg_190324
		case C_INCELL_100V_100A_50A_10A_40KW:	//ktg_190210
		case C_SDI_100V_150A_70A_30KW:			//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_2:		//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_3:		//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_4:		//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_5:		//jhkw_130821
		case C_LGC_100V_150A_100A_30KW:	//jhk_190419
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_VK_100V_200A_20A: //kjg_w
			//ret_val = 5000000.0 * 210.0 / 10.0;
			ret_val = (double)105000000.0;
			break;
		case C_EIG_100V_200A_20A_2:
			//ret_val = 5000000.0 * 213.0 / 13.0;
			ret_val = (double)81923076.92;
			break;
		case C_SEINENG_100V_500A_200A_30A_200KW:	//ktg_200116
			ret_val = (double)100000000;
			break;
		case C_BOSUNG_100V_200A_100A_40KW:	//jhk_161226
		case C_KIER_100V_200A_100A_20A_40KW:	//jhk_161226
		case C_KATECH_100V_250A_100A_100KW:	//ktg_190120
		case C_SDI_100V_300A_100A_60KW_3:		//jhk_150206
		case C_SDI_100V_300A_100A_60KW_4:		//jhk_150206
		case C_HCC_100V_300A_100A_60KW:		//jhkw_131106
		case C_H2_100V_300A_100A_60KW:	//jhk_150726
		case C_H2_100V_300A_100A_60KW_2:	//jhk_170110
		case C_SWISS_100V_300A_100A_60KW:	//jhk_160825		
		case C_HYUNDAI_100V_300A_150A_30A_60KW:		//ktg_190322
		case C_HYUNDAI_100V_300A_150A_30A_60KW_2:		//ktg_190322
		case C_HYUNDAI_100V_300A_150A_30A_60KW_3:		//ktg_190322
		case C_SKI_100V_300A_150A_50A_10A_60KW:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_2:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_3:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_4:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_5:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_6:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_7:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_8:	//kjhw_120531
		case C_LGE_100V_400A_80KW:	//ljh_210217
		case C_LGE_100V_400A_80KW_2:	//ljh_210217
		case C_DAEHWA_100V_500A_250A_50KW:	//jhk_170915
		//case C_LGC_100V_500A_250A_100KW: //jhk_180111
		//case C_LGC_100V_500A_250A_100KW_2: //jhk_180111
		//case C_LGC_100V_500A_250A_100KW_3: //jhk_180111
		//case C_LGC_100V_500A_250A_100KW_4: //jhk_180111	//shh_231030
		case C_SDI_100V_500A_250A_100KW: //ktg_190118
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		/*case C_LGC_110V_600A_300A_100A_216KW:	//ljh_201103					
		case C_LGC_110V_600A_300A_100A_216KW_2:	//ljh_201103					
		case C_LGC_110V_600A_300A_100A_216KW_3:	//ljh_201103					
		case C_LGC_110V_600A_300A_100A_216KW_4:	//ljh_201103
		//case C_LGC_110V_600A_300A_100A_264KW_33:	//sec_221031
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 5.5;
			break;*/			
		/*case C_LGC_110V_600A_300A_100A_264KW:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_2:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_3:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_4:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_5:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_6:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_7:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_8:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_9:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_10:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_11:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_12:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_13:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_14:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_15:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_16:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_17:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_18:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_19:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_20:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_21:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_22:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_23:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_24:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_25:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_26:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_27:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_28:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_29:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_30:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_31:    //phb_220216
		case C_LGC_110V_600A_300A_100A_264KW_32:    //phb_220216
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 6.0;
			break;*/
		case C_KATECH_120V_150A_100A_50A_18KW:	//jhkw_130621
		//case C_LGC_120V_250A_50A_60KW:	//jhk_160617	//shh_231027
		case C_LGC_120V_250A_50A_60KW_2:	//jhk_161101
		case C_SKI_120V_400A_100A_192KW:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_2:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_3:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_4:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_5:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_6:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_7:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_8:	//jhk_170628
		case C_SDI_120V_500A_250A_50A_120KW:	//ktg_190201
		case C_SDI_120V_500A_250A_50A_120KW_2:	//ktg_190201
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_SKI_120V_400A_200A_100A_50A_192KW:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_2:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_3:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_4:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_5:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_6:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_7:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_8:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_9:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_10:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_11:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_12:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_13:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_14:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_15:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_16:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_17:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_18:	//jhk_180206
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
		case C_KORID_160V_500A_100A_80KW:			//ktg_190412
		case C_INTS_600V_300A_100A_180KW:			//shh_200513 
		//case C_TEST_120V_400A_200A_100A_192KW:		//shh_200723
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 6.0;
			break;
		case C_GEELY_130V_300A_150A_78KW:	//shh_210406
		case C_GEELY_130V_300A_150A_78KW_2:	//shh_210406
		case C_GEELY_130V_600A_300A_156KW:  //ljh_210323
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.875;
			break;
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
		case C_CTP_150V_150A_100A_90KW:	//ktg_191111
		case C_SBL_150V_250A_10A_38KW: //lki_111010
		case C_SBL_150V_250A_10A_75KW_4:
		case C_SBL_150V_250A_10A_75KW_5:
		/*case C_LGC_150V_250A_100A_75KW:		//ktg_190507	//shh_231024
		case C_LGC_150V_250A_100A_150KW:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_2:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_3:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_4:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_5:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_6:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_7:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_8:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_9:	//ktg_190514
		case C_LGC_150V_250A_100A_150KW_10:	//ktg_190514
		case C_LGC_150V_250A_100A_150KW_11:	//ktg_190514
		case C_LGC_150V_250A_100A_150KW_12:	//ktg_190514*/
		case C_TUV_150V_300A_100A_180KW:	//ktg_190429
		//case C_LGC_150V_350A_100A_105KW:	//ktg_190510 //shh_231024
		//case C_LGC_150V_350A_100A_210KW:	//ktg_190421
		//case C_LGC_150V_350A_100A_210KW_2:	//ktg_190421
		//case C_LGC_150V_350A_100A_210KW_3:	//ktg_190421
		//case C_LGC_150V_350A_100A_210KW_4:	//ktg_190421	//shh_231025
		case C_LGE_150V_400A_60KW:	//jhk_170721
		//case C_LGC_150V_450A_200A_270KW:	//ktg_190421
		//case C_LGC_150V_450A_200A_270KW_2:	//ktg_190421
		//case C_LGC_150V_450A_200A_270KW_3:	//ktg_190421
		//case C_LGC_150V_450A_200A_270KW_4:	//ktg_190421	//shh_231025
		case C_HYUNDAICE_150V_500A_250A_150KW:	//ktg_190520
		case C_KTL_150V_600A_300A_200KW:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_2:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_3:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_4:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_5:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_6:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_7:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_8:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_9:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_10:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_11:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_12:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_13:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_14:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_15:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_16:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_17:	//ktg_211002
		//case C_LGC_150V_600A_300A_100A_300KW:	//shh_210819
		//case C_LGC_150V_600A_300A_100A_300KW_2:	//shh_210819
		case C_NORTHVOLT_200V_400A_200A_160KW:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_2:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_3:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_4:	//ktg_191203
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_UTP_200V_500A_250A_100KW:	//ljh201119
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 5.0;
			break;
		case C_KTC_300V_100V_250A_100A_25A_150KW:	//jhk_151107
		case C_KBIA_300V_200A_100A_120KW:	  		//shh_200919	
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		//case C_LGC_220V_600A_300A_100A_528KW:       //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_2:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_3:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_4:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_5:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_6:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_7:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_8:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_9:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_10:    //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_11:    //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_12:    //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_13:    //phb_220228
		//	ret_val = (double)myPs->config.maxV[0] * 5.0 / 5.5;
        //    break;
		//case C_LGC_200V_350A_100A_280KW:	//ktg_190425
		//case C_LGC_250V_150A_100A_75KW:		//ktg_190510	
		//case C_LGC_250V_150A_100A_150KW:	//ktg_190513
		//case C_LGC_250V_150A_100A_150KW_2:	//ktg_190513	//shh_231025
		case C_HYUNDAI_300V_300A_150A_180KW:	//ljh_201106
		case C_SDIXIAN_300V_400A_100A_50A_25A:	//jhk_160527
		//case C_LGC_350V_300A_100A_420KW:	//shh_220128
		//case C_LGC_350V_300A_100A_420KW_2:	//shh_220128
		//case C_LGC_350V_300A_100A_420KW_3:	//shh_220128
		//case C_LGC_350V_300A_100A_420KW_4:	//shh_220128
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 6.0;
			break;
		case C_LGC_350V_150A_100A_105KW:	//khj_191230
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 5.0;
			break;
		case C_ROTEM_400V_60A_10A: //kjg_w
			ret_val = (double)500000000.0;//(double)myPs->config.maxV[0];
			break;
		case C_KATECH_400V_250A_10A:
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 6.0;
			break;
		case C_ERAE_400V_300A_100A_30KW:	//jhk_150119
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.0;
			break;
		case C_KATECH_450V_150A_10A:
		case C_HYUNDAI_450V_200A_10A_90KW_2:
		case C_BCST_450V_200A_10A_90KW:
		case C_CTNT_450V_200A_10A_180KW:	//kjh_110929
		case C_LGC_450V_200A_10A_180KW_4:	//jhkw_130909
		case C_DAEWOO_450V_200A_50A_90KW: //kjh_150518
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
		case C_NKT_450V_200A_100A_90KW:	//ktg_190111
		case C_NKT_450V_200A_100A_90KW_2:	//ktg_190111
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_SDI_450V_225A_22A_202KW:		//jhkw_130629
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_PNE_450V_250A_10A_115KW: //jhk_131201
		case C_SEM_450V_250A_10A_225KW:
		case C_LGC_450V_250A_10A_225KW: //jhk_140728
		//case C_LGE_450V_250A_50A_112KW:		//jhk_150703
		case C_LGE_450V_250A_50A_225KW:		//jhk_150605
		case C_LGE_450V_250A_50A_225KW_2:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_3:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_4:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_5:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_6:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_7:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_8:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_9:	//jhk_150605
		//case C_LGC_450V_250A_50A_225KW:		//jhk_160608
		//case C_LGC_450V_250A_50A_225KW_2:	//jhk_160608	//shh_231024
		//case C_LGC_450V_250A_100A_225KW:	//jhk_170211	//shh_231024
		case C_KMI_450V_250A_100A_112KW:	//jhk_171110
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_SDI_600V_60A_72KW:			//shh_201202
		case C_SDI_600V_60A_72KW_2:			//shh_201202
		case C_SDI_600V_60A_72KW_3:			//shh_201202
		case C_SDI_600V_60A_72KW_4:			//shh_201202
		case C_SDI_600V_60A_72KW_5:			//shh_201202
		case C_SDI_600V_60A_72KW_6:			//shh_201202
		case C_SDI_600V_60A_72KW_7:			//shh_201202
		case C_SDI_600V_60A_72KW_8:			//shh_201202
		case C_SDI_600V_60A_72KW_9:			//shh_201202
		case C_SDI_600V_60A_72KW_10:		//shh_201202
		case C_SDI_600V_60A_72KW_11:		//shh_201202
		case C_SDI_600V_60A_72KW_12:		//shh_201202
		case C_SDI_600V_60A_72KW_13:		//shh_201202
		case C_SDI_600V_60A_72KW_14:		//shh_201202
		case C_SDI_600V_60A_72KW_15:		//shh_201202
		case C_SDI_600V_60A_72KW_16:		//shh_201202
		case C_SDI_600V_60A_72KW_17:		//shh_201202
		case C_SDI_600V_60A_72KW_18:		//shh_201202
		case C_SDI_600V_60A_72KW_19:		//shh_201202
		case C_SDI_600V_60A_72KW_20:		//shh_201202
		case C_SDI_600V_60A_72KW_21:		//shh_201202
		case C_SDI_600V_60A_72KW_22:		//shh_201202
		case C_SDI_600V_60A_72KW_23:		//shh_201202
		case C_SDI_600V_60A_72KW_24:		//shh_201202
		case C_SDI_600V_60A_72KW_25:		//shh_201202
		case C_SDI_600V_60A_72KW_26:		//shh_201202
		case C_SDI_600V_60A_72KW_27:		//shh_201202
		case C_SDI_600V_60A_72KW_28:		//shh_201202
		case C_SDI_600V_60A_72KW_29:		//shh_201202
		case C_SDI_600V_60A_72KW_30:		//shh_201202
		case C_SDI_600V_60A_72KW_31:		//shh_201202
		case C_SDI_600V_60A_72KW_32:		//shh_201202
		case C_SDI_600V_60A_72KW_33:		//shh_201202
		case C_SDI_600V_60A_72KW_34:		//shh_201202
		case C_SDI_600V_60A_72KW_35:		//shh_201202
		case C_SDI_600V_60A_72KW_36:		//shh_201202
		case C_SDI_600V_60A_72KW_37:		//shh_201202
		case C_SDI_600V_60A_72KW_38:		//shh_201202
		//case C_LGC_600V_150A_100A_180KW:	//ktg_190405	//shh_231024
		case C_LGE_600V_150A_100A_180KW: 	//shh_210426
		case C_KTL_600V_200A_100A_20A_400KW:	//jhk_180731
		case C_SEBANG_600V_200A_100A_240KW:	//jhk_160223
		case C_SKC_600V_200A_100A_50A_240KW: //jhkw_120807
		case C_SKC_600V_200A_100A_50A_240KW_2: //jhkw_121019
		case C_LGE_600V_250A_50A_150KW:	//jhk_150520
		case C_UTP_600V_300A_100A_150KW:	//jhk_151107
		case C_LGC_600V_300A_100A_360KW_2:	//ktg_200104
		case C_LGC_600V_300A_100A_360KW_3:	//ktg_200108
		case C_LGC_600V_300A_100A_360KW_4:	//ktg_200108
		//case C_LGC_600V_300A_100A_720KW:	//shh_220204
		//case C_LGC_600V_300A_100A_720KW_2:	//shh_220204
		//case C_LGC_600V_300A_100A_720KW_3:	//shh_220204
		//case C_LGC_600V_300A_100A_720KW_4:	//shh_220204
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 6.0;
			break;
		case C_KCL_600V_200A_100A_480KW: //ktg_190812
		case C_KCL_600V_200A_100A_480KW_2: //ktg_190812
		//case C_LGC_600V_300A_100A_360KW:	//ktg_190620	//shh_231025
		case C_SEINENG_600V_300A_100A_30A_500KW:	//ktg_200112
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_SDI_600V_300A_100A_90KW:		//ktg_200809
		case C_SKE_600V_400A_100A_50A_25A_240KW:	//jhkw_130924
		case C_DAEHWA_600V_400A_200A_100A_200KW:	//jhk_170120
		/*
		case C_HYUNDAI_600V_400A_200A_100A_240KW:	//khj_190805
		//case C_HYUNDAI_600V_400A_200A_100A_240KW_2:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_3:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_4:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_5:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_6:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_7:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_8:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_9:	//khj_190805
		*/
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 6.0;
			break;
		case C_DAEHWA_750V_300A_100A_150KW:	//jhk_170203
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_HANGKE_750V_300A_100A_300KW:		//jhk_131226
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 7.5;
			break;
		case C_HYUNDAI_750V_400A_100A_240KW:	//jhk_160909
		case C_HYUNDAI_750V_400A_100A_240KW_2:	//jhk_161111
		case C_HYUNDAI_750V_400A_100A_240KW_3:	//jhk_171015
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_STECHWIN_800V_200A_50A_320KW:	//jhk_150812
		case C_DAEHWA_800V_200A_50A_160KW:		//jhk_160831
		case C_CS_800V_250A_50A_250KW:			//jhk_150812
		case C_ADD_800V_350A_200A_50A_380KW:	//shh_200904
		case C_BOSUNG_1200V_200A_100A_200KW:	//jhk_170106
		case C_SKI_1200V_250A_100A_50A_300KW:	//jhk_131209
		case C_KTL_1200V_300A_100A_300KW:	//kjh_171013
		case C_KATECH_1200V_300A_100A_50A_300KW:	//jhk_150108
		case C_SEBANG_1200V_300A_100A_360KW:	//jhk_160223
		case C_KCL_1200V_1000A_400A_600KW:		//shh_200809 	
		case C_KCL_1200V_1000A_400A_600KW_2:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_600KW_3:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_600KW_4:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_700KW:		//shh_200809 	
		case C_KCL_1200V_1000A_400A_700KW_2:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_800KW:		//shh_200809 	
		case C_KCL_1200V_1000A_400A_800KW_2:	//shh_200809 
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_2:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_3:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_4:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_5:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_6:	//shh_210625
		case C_HYUNDAI_1200V_1000A_500A_300A_800KW:	//shh_210915
		case C_HYUNDAI_1200V_1000A_500A_300A_800KW_2:	//shh_211020
		case C_KATECH_1200V_1000A_500A_300A_600KW:		//shh_210611
		case C_KATECH_1200V_1000A_500A_300A_600KW_2:	//shh_210611
		case C_UTP_1200V_1000A_500A_300A_600KW:			//shh_201120
		case C_SKI_1500V_400A_100A_500KW:	//jhk_180610
		case C_KTL_1500V_400A_200A_700KW:	//jhk_180902
		case C_CTP_1500V_600A_300A_60A_600KW:	//ktg_191117
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 6.0;
			break;
		case C_NCT_1500V_250A_100A_400KW:	//ljh_201113
		case C_KTC_1500V_600A_200A_400KW:	//ktg_200807
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_KEIR_1500V_600A_300A_200A_100A_720KW:	//ktg_211202
		case C_KCL_1500V_1000A_500A_600KW:	//jhk_181110
		case C_HYUNDAI_1500V_1000A_500A_600KW:	//jhk_170917
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 6.0;
			break;
		default:
			//ret_val = (double)myPs->config.maxV[0];
			//REF_VAL : config.maxV = 5000 : v_feed
			//REF_VAL = config_maxV * 5000 / v_feed

			maxV = myPs->config.maxV[0];	//kjh_211021s
			v_feed = myPs->config.v_feed[0];

			if(myPs->config.ratioV == MICRO_UNIT) {	//uV
				ref = 5000000.0;
			} else if(myPs->config.ratioV == MILLI_UNIT) {	//mV
				ref = 5000.0;
			} else {	//V
				ref = 5.0;
			}

			ret_val = (double)maxV * ref / v_feed;	//kjh_211021e
			break;
	}

	return ret_val;
}

double Ref_I_Value(void)
{
	double ret_val;
	long maxI; //kjh_211021
	double i_feed, ref; //kjh_211021

	switch(myData->AppControl.config.systemModel) {
		//shuntR used
		case C_EIG_5V_50A_5A:
		case C_EIG_5V_50A_5A_2:
		case F_SDI_5V_50A_5A:
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 830ohm) * (1 + 49.4kohm / 100kohm) = 90.414
			//G2 = (1 + 49.4kohm / 830ohm) = 60.51807

			//RANGE1
			//shuntV = 2.0mOhm * 50A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / xOhm = 1.0
			//V1 = shuntV * G1 = 100000uV
			//V2 = V1 * G2 = 6051807uV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.051807;
			break;
		case C_LGC_5V_200A_75A_15A:
			//AMP -> AD620A
			//kjg_090613 G2 = (1 + 49.4kohm / 830ohm) * (1 + 49.4kohm / 100kohm) = 62.0121
			//G2 = (1 + 49.4kohm / 830ohm) = 60.51807

			//RANGE1
			//shuntV = 0.5mOhm * 200A = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / xOhm = 1.0
			//V1 = shuntV * G1 = 100000uV
			//V2 = V1 * G2 = 6201210uV
			//kjg_090613 ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.20121;
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.051807;
			break;
		case C_ENERLAND_5V_250A:
		case C_ENERLAND_5V_250A_50A_5A: //kjg_w
		case C_ENERLAND_5V_250A_50A_5A_2: //kjg_w
		case C_TAESUNG_20V_600A_60A:
			//shuntV = 100mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 830ohm = 60.51807229
			//V1 = shuntV * G1 = 6051.807229mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.051807229;
			break;
		case C_KATECH_20V_1000A_500A_100A:  //csk_120309
			//shuntR = 0.1mohm (IRH80F-40A 0.1mohm)
			//G1 = 1 + 49.4kohm / gainR = 1
			//V1 = 1000000000uA * shuntR * G1 = 100000uV
			//V2 = V1 * G2 = 100000uV * (1 + 49.4kohm / 840ohm) = 5980952.38uV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.98095238;
			break;
		case C_LGC_50V_40A_10A_4A: //csk_120206
		case C_LGC_50V_40A_10A_4A_2:
		case C_LGC_50V_40A_10A_4A_3:
		case C_LGC_50V_40A_10A_4A_4:
			//shuntR = 2.5mohm (IRH80F-40A 2.5mohm)
			//gainR = 800ohm
			//G1 = 1 + 49.4kohm / 800ohm = 62.75
			//V1 = 40000000uA * shuntR * G1 = 6275000uV
			//Imax * Vref / V1 = 40000000uA * 5000000uV / 6275000uV
			// = 31872509.96
			ret_val = (double)31872509.96;
			break;
		case C_SKI_50V_500A_250A_25KW:	//ktg_190715
		case C_SKI_50V_500A_400A_200A_100A_100KW:	//jhk_180119
		case C_SKI_50V_500A_400A_200A_100A_100KW_2:	//jhk_180119
			//shuntV = 833.3mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5979.1666666667mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9791666667;
			break;
		case C_SKI_50V_1000A_500A_300A_100KW:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_2:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_3:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_4:	//ktg_190711
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.17kohm = 9.00648298
			//V1 = shuntV * G1 = 6004.321988mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.004321988;
			break;
		case C_ENERTECH_55V_100A_10A: //kjg_w
			ret_val = (double)myPs->config.minI[0] * (-1.0);
			break;
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
			//shuntR = 2mohm (IRH300F-50A 2mohm)
			//gainR = 900ohm // 12kohm = 837.2093ohm
			//G1 = 1 + 49.4kohm / 837.2093 = 60.005556
			//V1 = 50000000uA * shuntR * G1 = 6000555.6uV
			//Imax * Vref / V1 = 50000000uA * 5000000uV / 6000555.6uV
			// = 41662808.69
			ret_val = (double)41662808.69;
			break;
		case C_SDI_70V_250A_25A_18KW:
			//ITB-300S 300A/150mA
			//BurnR = 50ohm // 5ohm = 4.545454
			//GainR = 6.8kohm
			//G = 1 + 49.4kohm / 6.8kohm = 8.264706
			//V1 = (250A / 300A * 150mA) * BurnR * G = 4695855.7uV
			//Imax * Vref / V1 = 250000000uA * 5000000uV / 4695855.7uV
			// = 266192166
			ret_val = (double)266192166.0;
			break;
		case C_SAEHAN_72V_200A_20A:
			ret_val = (double)myPs->config.minI[0] * (-1.0);
			break;
		case C_SDI_80V_50A_25A_10A:		//csk_130417_s
		case C_SDI_80V_50A_25A_10A_2:
		case C_SDI_80V_50A_25A_10A_3:
		case C_SDI_80V_50A_25A_10A_4:	//csk_130417_e
			//RANGE1
			//shuntR = 2mohm (IRH300F-50A 2mohm)
			//shuntV = 50A * 2mohm = 100mV
			//gainR1 = 820ohm
			//G1 = 1 + 49.4kohm / gainR1 = 61.2439
			//V1 = shuntV * G1 = 6124390uV

			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.124;
			break;
		case C_KATECH_100V_30A_15A_5A: //csk_131129
			//RANGE1
			//shuntR = 3.333mohm (IRH80F-30A 3.333mohm)
			//shuntV = 30A * 3.333mohm = 99.99mV
			//gainR1 = 830ohm
			//G1 = 1 + 49.4kohm / gainR1 = 60.518
			//V1 = shuntV * G1 = 6051202uV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.051;
			break;
		case C_EIG_100V_200A_20A_2:
			//shuntV = 200mV
			//AMP -> INA121
			//G1 = 1 + 50kohm / 1.703kohm = 30.35995302
			//V1 = shuntV * G1 = 6071.990604mV
			//val2 = (double)myPs->config.maxI[0];
			ret_val = (double)myPs->config.maxI[0] * 5000.0 / 6071.990604;
			break;
		case F_SDI_5V_200A_100A_50A_10A:		//csk_161007
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 830ohm) * (1 + 49.4kohm / 100kohm) = 90.414
			//G2 = (1 + 49.4kohm / 820ohm) = 60.51807

			//RANGE1
			//shuntV = 0.5mOhm * 200A = 100mV
			//AMP -> AD620A
			//G1 = (1 + 49.4kohm / 820ohm) = 61.2439
			//V1 = shuntV * G1 = 6124390uV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.12439;
			break;
		case F_SDI_5V_450A_200A_100A_10A_3:
		case F_SDI_5V_450A_200A_100A_10A_4:
		case F_SDI_5V_450A_200A_100A_10A_96CH:	//hun_160408
		case F_SDI_5V_450A_200A_100A_10A_96CH_2:	//hun_160808
		case F_SDI_5V_450A_200A_100A_10A_96CH_4:	//hun_170703
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 820ohm) * (1 + 49.4kohm / 100kohm) = 91.498
			//G2 = (1 + 49.4kohm / 820ohm) = 61.24390

			//RANGE1
			//shuntV = 0.222mOhm * 450A = 99.9mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / xOhm = 1.0
			//V1 = shuntV * G1 = 100000uV
			//V2 = V1 * G2 = 6124390uV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.124390;
			break;

		case F_SDI_5V_450A_200A_100A_10A_96CH_3:	//hun_161201
			//AMP -> AD620A
			//G2 = (1 + 49.4kohm / 830ohm) * (1 + 49.4kohm / 100kohm) = 90.414
			//G2 = (1 + 49.4kohm / 830ohm) = 60.51807

			//RANGE1
			//shuntV = 0.222mOhm * 450A = 99.9mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / xOhm = 1.0
			//V1 = shuntV * G1 = 100000uV
			//V2 = V1 * G2 = 6051807uV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.051807;
			break;

		//Current Transducer used
		/*case C_LGC_40V_300A_100A_48KW: //jhk_181018
		case C_LGC_40V_300A_100A_48KW_2: //jhk_181018
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10.0kohm = 5.94
			//V1 = shuntV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.94;
			break;
		case C_LGC_40V_500A_250A_100A_80KW:	//jhk_181014
		case C_LGC_40V_500A_250A_100A_80KW_2: //jhk_181014
			//shuntV = 833.333mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5979.1666667mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9791666667;
			break;*/
		case C_SEBANG_50V_300A_100A_50A_15KW:	//jhkw_130912
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.0kohm = 8.057142857
			//V1 = shuntV * G1 = 6042.857143mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		case C_LGC_50V_300A_10A_20KW:	//jhk_110719
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 8983.333333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 8.983333333;
			break;	
		case C_JNU_50V_200A_100A_10KW:	//jhk_161202
		case C_HLGP_60V_200A_10A_12KW:
		case C_HLGP_60V_200A_10A_24KW:
		case C_HLGP_60V_200A_10A_24KW_2:
		case C_SNU_60V_200A_100A_50A_1CH_12KW:  //jhkw_130221
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_SK_60V_300A_10A:
		//case C_LGC_60V_300A_30A_36KW:	//jhk_150406
		//case C_LGC_60V_300A_30A_36KW_2:	//jhk_150406
		//case C_LGC_60V_300A_30A_36KW_3:	//jhk_150406
		case C_TOPBATT_60V_300A_100A_36KW:	//jhk_150616
		case C_HYUNDAI_60V_300A_100A_36KW:	//jhk_160825
		case C_HYUNDAI_60V_300A_100A_36KW_2:	//jhk_161111
		case C_HYUNDAI_60V_300A_100A_36KW_3:	//jhk_171015
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.0kohm = 8.057142857
			//V1 = shuntV * G1 = 6042.857143mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		case C_CANSYSTEM_60V_350A_100A_21KW:	//jhk_140828
			//shuntV = 583.333mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.0kohm = 10.88
			//V1 = shuntV * G1 = 6346.66667mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.34666667;
			break;
		case C_KTL_60V_400A_200A_40KW:	//jhk_181218
			//shuntV = 666.7mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9784946236559;
			break;
		case C_DAEHWA_60V_500A_250A_50A_60KW:	//jhk_160913
		case C_DAEHWA_60V_500A_250A_50A_60KW_2:	//jhk_160913
			//shuntV = 833.3mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5979.1666666667mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9791666667;
			break;
		//case C_LGC_60V_600A_300A_100A_144KW:	//ktg_190707
		//case C_LGC_60V_600A_300A_100A_144KW_2:	//ktg_191222
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//V1 = shuntV * G1 = 5940mV
		//	ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.94;
		//	break;
		case C_LGC_50V_500A_10A_35KW:	//kjh_111002
		case C_LGC_50V_500A_10A_42KW:
		case C_LGC_50V_500A_10A_42KW_2:
		case C_LGC_50V_500A_10A_42KW_3:
		case C_LGC_50V_500A_10A_42KW_4:
		case C_LGC_60V_250A_10A_30KW:
		case C_LGC_60V_250A_10A_30KW_2:
		case C_LGC_60V_250A_10A_30KW_3:
			//ITB300-S 300A:150mA = 250A:x -> x =125mA
			//shuntV = x * 5ohm = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.0kohm = 9.233333
			//V1 = shuntV * G1 = 5770.83333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.77083333;
			break;
		case C_NORTHVOLT_60V_400A_200A_96KW:		//khj_191203
			//shuntV = 666.7mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9784946236559;
			break;
		case C_LGC_70V_100A_14KW:	//jhk_180104
		case C_LGC_70V_100A_14KW_2:	//jhk_180104
		case C_LGC_70V_100A_14KW_3:	//jhk_180104
		case C_LGC_70V_100A_14KW_4:	//jhk_180104
		//case C_LGC_70V_100A_28KW:	//ljh_201130
		//case C_LGC_70V_100A_28KW_2:	//ljh_201130	//shh_231024
		//case C_LGC_70V_200A_100A_28KW:		//jhk_170911
		//case C_LGC_70V_200A_100A_28KW_2:	//jhk_170911	//shh_231025
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.977778
			//V1 = 500mV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_LGC_70V_250A_10A_35KW:
		//case C_LGC_70V_250A_10A_35KW_2:
		case C_LGC_70V_250A_10A_35KW_3:
		case C_LGC_70V_250A_10A_35KW_4:
		//case C_LGC_70V_250A_10A_35KW_5:	//phb_240429
		case C_LGC_70V_250A_10A_35KW_6:
		case C_LGC_70V_250A_10A_35KW_7:
		case C_LGC_70V_250A_10A_35KW_8:
		case C_LGC_70V_250A_10A_35KW_9:
		case C_LGC_70V_250A_10A_35KW_10:
		//case C_LGC_70V_250A_10A_35KW_11:
		case C_LGC_70V_250A_10A_35KW_12:
		//case C_LGC_70V_250A_10A_35KW_13:
		case C_LGC_70V_250A_10A_35KW_14:
		case C_LGC_70V_250A_10A_35KW_15:
		case C_LGC_70V_250A_10A_35KW_16:
		case C_LGC_70V_250A_10A_35KW_17:
		case C_LGC_70V_250A_10A_35KW_18:
		case C_LGC_70V_250A_10A_35KW_19:    //111202_csk
		case C_LGC_70V_250A_10A_35KW_20:    //111202_csk
		case C_LGC_70V_250A_10A_35KW_21:    //jhk_120222
		case C_LGC_70V_250A_10A_35KW_22:    //jhk_120229
		case C_LGC_70V_250A_10A_35KW_23:    //jhk_120702
		case C_LGC_70V_250A_10A_35KW_24:    //jhk_120702
		case C_LGC_70V_250A_10A_35KW_25:    //jhk_120702
		case C_LGC_70V_250A_10A_35KW_26:    //jhk_120702
		case C_LGC_70V_250A_10A_35KW_27:    //jhk_120702
		case C_LGC_70V_250A_10A_35KW_28:    //jhk_120702
		/*case C_LGC_70V_250A_50A_35KW:		//jhk_150502
		//case C_LGC_70V_250A_50A_35KW_2:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_3:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_4:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_5:		//jhk_150502
		//case C_LGC_70V_250A_50A_35KW_6:		//jhk_150502
		//case C_LGC_70V_250A_50A_35KW_7:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_8:		//jhk_150502
		case C_LGC_70V_250A_50A_35KW_9:		//jhk_160515
		case C_LGC_70V_250A_50A_35KW_10:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_11:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_12:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_13:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_14:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_15:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_16:	//jhk_160515*/	//sec_231026
		case C_LGC_70V_250A_100A_50A_35KW:		//jhkw_121019
		case C_LGC_70V_250A_100A_50A_35KW_2:	//jhkw_121019
		case C_LGC_70V_250A_100A_50A_35KW_3:	//jhkw_121019
		case C_LGC_70V_250A_100A_50A_35KW_4:	//jhkw_121019
			//ITB300-S 300A:150mA = 250A:x -> x =125mA
			//shuntV = x * 5ohm = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.0kohm = 9.233333
			//V1 = shuntV * G1 = 5770.83333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.77083333;
			break;
		case C_LGC_70V_100A_50A_28KW:		//ktg_190405
		case C_LGC_70V_250A_10A_35KW_2:		//kjhw_180212
		//case C_LGC_70V_250A_50A_35KW_2:		//ktg_191218
		//case C_LGC_70V_250A_50A_35KW_6:		//ktg_191218
		//case C_LGC_70V_250A_50A_35KW_7:		//ktg_191219
		case C_LGC_70V_250A_10A_35KW_11:
		case C_LGC_70V_250A_10A_35KW_13:
		case C_LGC_70V_300A_100A_42KW:	//jhk_170211
		case C_LGC_70V_300A_100A_42KW_2:	//jhk_170519
		case C_LGC_70V_300A_100A_42KW_3:	//jhk_170211
		//case C_LGC_70V_300A_100A_50A_10A_42KW:	//jhkw_120810
		//case C_LGC_70V_300A_100A_50A_10A_42KW_2:	//jhkw_120813	//shh_231027
		case C_LGCUSA_70V_300A_42KW:	//jhk_160802
		case C_LGCUSA_70V_300A_42KW_2:	//jhk_160802
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 750mV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		case C_LGCUSA_70V_350A_24KW:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_2:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_3:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_4:	//jhk_160720
			//shuntV = 583.333mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.0kohm = 10.88
			//V1 = shuntV * G1 = 6346.66667mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.34666667;
			break;
		/*case C_LGC_70V_350A_100A_49KW:		//jhk_170911
		case C_LGC_70V_350A_100A_49KW_2:	//jhk_170911
		case C_LGC_70V_350A_100A_49KW_3:	//jhk_170911
			//shuntV = 1166.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 12kohm = 5.11666667
			//V1 = shuntV * G1 = 5969.4444mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.969444444;
			break;
		*/	//shh_231025
		case C_LGC_70V_400A_200A_56KW:			//ktg_190711
			//shuntV = 800mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5740mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.74;
			break;
		case C_NEWPOWER_75V_150A_75A_45KW:	//ljh_201105
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//V1 = shuntV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.940;
			break;
		case C_LGCCHINA_80V_150A_12KW:		//jhk_160725
		case C_LGCCHINA_80V_150A_12KW_2:	//jhk_160725
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 750mV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		/*case C_LGC_90V_300A_100A_108KW:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_2:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_3:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_4:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_5:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_6:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_7:	//jhk_181016
		case C_LGC_90V_300A_100A_108KW_8:	//jhk_181016*/	//ktg_231023
		case C_LGCCHINA_90V_300A_100A_108KW:	//ktg_190319
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10.0kohm = 5.94
			//V1 = shuntV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.94;
			break;
		case C_LGC_90V_500A_250A_100A_90KW:	//jhk_181107
		//case C_LGC_90V_500A_250A_100A_180KW:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_2:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_3:	//jhk_181006	//sec_221027
		//case C_LGC_90V_500A_250A_100A_180KW_4:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_5:	//jhk_181006
		//case C_LGC_90V_500A_250A_100A_180KW_6:	//jhk_181006	//ktg_231023
		/*case C_LGC_90V_500A_250A_100A_180KW_7:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_8:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_9:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_10:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_11:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_12:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_13:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_14:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_15:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_16:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_17:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_18:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_19:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_20:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_21:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_22:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_23:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_24:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_25:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_26:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_27:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_28:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_29:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_30:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_31:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_32:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_33:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_34:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_35:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_36:	//ktg_190219
		case C_LGC_90V_500A_250A_100A_180KW_37:	//ktg_190219 */ //ktg_231124
		case C_LGC_90V_500A_250A_100A_180KW_38:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_39:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_40:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_41:	//ktg_200104
		case C_LGC_90V_500A_250A_100A_180KW_42:	//ktg_200104
		case C_CORN_100V_100A_50A_40KW:	//ktg_190210
		case C_INCELL_100V_100A_50A_10A_40KW:	//ktg_190210
			//shuntV = 833.333mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5979.1666667mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9791666667;
			break;
		case C_LGC_90V_500A_300A_100A_180KW:	//ktg_191106
		case C_LGC_90V_500A_300A_100A_180KW_2:	//ktg_191106
		//case C_LGC_90V_500A_300A_100A_180KW_3:	//ljh_201130
		//case C_LGC_90V_500A_300A_100A_180KW_4:	//ljh_201130
		//case C_LGC_90V_500A_300A_100A_180KW_5:	//ljh_201130	//shh_231027
		//case C_LGC_90V_500A_300A_100A_180KW_6:	//khj_200303	//ktg_231124
		//case C_LGC_90V_500A_300A_100A_180KW_7:	//shh_220324
		//case C_LGC_90V_500A_300A_100A_180KW_8:	//shh_220324
		//case C_LGC_90V_600A_300A_100A_216KW:	//ktg_190707
		//case C_LGC_90V_600A_300A_100A_216KW_2:	//ktg_200709
		//case C_LGC_90V_600A_300A_100A_216KW_3:	//ktg_200709
		//case C_LGC_90V_600A_300A_100A_216KW_4:	//ktg_200709
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//V1 = shuntV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.94;
			break;
		case C_LGE_100V_30A_3KW:			//khj_191127
			//shuntV = 600mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6kohm = 9.2333
			//V1 = shuntV * G1 = 5539.98mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.53998;
			break;
		case C_JBTP_100V_100A_40KW:  	//shh_200927
			//shuntV = 666mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677
			//V1 = shuntV * G1 = 5978.49462366mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.978494623;
			break;
		case C_SEBANG_100V_100A_50A_10A_40KW:	//ktg_190324
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.5kohm = 7.58667
			//V1 = shuntV * G1 = 5690mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.69;
			break;
		case C_GITC_100V_100A_50A_10A_40KW:	//jhk_180917
		case C_POWERLOGICS_100V_100A_50A_40KW:	//jhk_180919
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 750mV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		//case C_LGC_100V_100A_50A_20KW:	//jhk_181023	//shh_231024
		case C_SAMHWA_100V_100A_10A:
		case C_SKI_100V_100A_10A_10KW:  //jhk_120329
		case C_LGC_100V_100A_10A_10KW:  //jhk_121009
		case C_LGC_100V_100A_10A_10KW_2:  //jhk_121009
		case C_LGC_100V_100A_10A_10KW_3:  //jhk_121009
		case C_LGC_100V_100A_10A_10KW_4:  //jhk_121009
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_JFMTECH_100V_100A_50A_40KW:			//csk_190923
			//shuntV = 666.6667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.5kohm = 8.6
			//V1 = shuntV * G1 = 5733.333333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.733333333;
			break;
		case C_SDI_100V_150A_70A_30KW:			//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_2:		//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_3:		//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_4:		//jhkw_130821
		case C_SDI_100V_150A_70A_30KW_5:		//jhkw_130821
			//shuntV = 525mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 6288.333333333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6288.333333333;
			break;
		case C_LGCCHINA_100V_200A_20KW_3:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_4:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_5:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_6:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_7:	//jhk_160731
		case C_LGC_100V_200A_10A_20KW:
		case C_BOSUNG_100V_200A_100A_40KW:	//jhk_161226
		case C_KIER_100V_200A_100A_20A_40KW: //jhk_161226
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_LAONTECH_100V_200A_40KW:		//shh_200927
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//V1 = shuntV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.94;
			break;
		case C_HYUNDAI_100V_200A_100A_40KW:	//shh_201005
		case C_HYUNDAI_100V_200A_100A_80KW:	//shh_210311
		case C_LGC_100V_200A_100A_80KW:		//ktg_200208
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.967741935483871
			//V1 = shuntV * G1 = 5978.494623655914mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.978494624;
			break;
		case C_LGC_100V_250A_100A_100KW:	//jhk_181006
		case C_KATECH_100V_250A_100A_100KW:	//ktg_190120
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.8kohm = 9.5172413793
			//V1 = shuntV * G1 = 5948.2758620689mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.948275862;
			break;
		case C_SDI_100V_300A_100A_60KW_3:		//jhk_150206
		case C_SDI_100V_300A_100A_60KW_4:		//jhk_150206
		case C_HCC_100V_300A_100A_60KW:		//jhkw_131106
		case C_H2_100V_300A_100A_60KW:	//jhk_150726
		case C_H2_100V_300A_100A_60KW_2:	//jhk_170110
		case C_SWISS_100V_300A_100A_60KW:	//jhk_160825
		case C_SKI_100V_300A_150A_50A_10A_60KW:	//kjhw_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_2:	//kjhw_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_3:	//kjhw_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_4:	//kjhw_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_5:	//kjhw_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_6:	//kjhw_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_7:	//kjhw_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_8:	//kjhw_120527
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 750mV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		case C_LGE_100V_400A_80KW:	//ljh_210217
		case C_LGE_100V_400A_80KW_2:	//ljh_210217
			//shuntV = 666.7mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.8kohm = 9.51724138
			//V1 = shuntV * G1 = 6344.827586mV
			ret_val = (double)myPs->config.maxI[0] *5.0 / 6.344827586;
			break;
		case C_KIER_100V_400A_200A_160KW:		//shh_201103
			//shuntV = 800mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.7kohm = 7.41558
			//V1 = 800mV * 7.41558 = 5932.467532467mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.93246753;
			break;
		case C_KBTP_100V_300A_100A_60KW:		//shh_210412
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//V1 = 10000mV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.940;
			break;
		case C_KTL_100V_500A_100A_100KW:			//shh_200809
		case C_KTL_100V_500A_100A_100KW_2:  //ljh_210319
		case C_KTL_100V_500A_100A_100KW_3:	//ljh_210324
		case C_KTL_100V_500A_100A_100KW_4:	//ljh_210324
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774
			//V1 = 666.667mV * G1 = 5978.4696559mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9784696559;
			break;
		case C_POWERLOGICS_100V_150A_60KW:			//shh_200928
		case C_SEINENG_100V_500A_200A_30A_200KW:	//ktg_200116
		case C_SEINENG_100V_500A_200A_30A_200KW_2:	//ktg_200116
		/*case C_LGC_110V_600A_300A_100A_264KW:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_2:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_3:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_4:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_5:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_6:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_7:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_8:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_9:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_10:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_11:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_12:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_13:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_14:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_15:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_16:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_17:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_18:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_19:	//shh_220215
		case C_LGC_110V_600A_300A_100A_264KW_20:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_21:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_22:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_23:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_24:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_25:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_26:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_27:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_28:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_29:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_30:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_31:    //phb_220216
        case C_LGC_110V_600A_300A_100A_264KW_32:    //phb_220216*/
		//case C_LGC_110V_600A_300A_100A_264KW_33:	//sec_221031
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10.0kohm = 5.94
			//V1 = shuntV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.94;
			break;
		//case C_LGC_100V_500A_250A_100KW: //jhk_180111
		//case C_LGC_100V_500A_250A_100KW_2: //jhk_180111
		//case C_LGC_100V_500A_250A_100KW_3: //jhk_180111
		//case C_LGC_100V_500A_250A_100KW_4: //jhk_180111
		case C_KTL_100V_500A_250A_50A_200KW:	//jhk_180718
		case C_SDI_100V_500A_250A_100KW: //ktg_190118
		case C_DAEHWA_100V_500A_250A_50KW:	//jhk_170915
			//shuntV = 833.333mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5979.1666667mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9791666667;
			break;
		case C_LGC_100V_150A_100A_30KW:	//jhk_190419
		case C_SDI_100V_600A_100A_200KW:	//jhk_181107
		case C_HYUNDAI_100V_300A_150A_30A_60KW:		//ktg_190322
		case C_HYUNDAI_100V_300A_150A_30A_60KW_2:		//ktg_190322
		case C_HYUNDAI_100V_300A_150A_30A_60KW_3:		//ktg_190322
			//shuntV = 800mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5740mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.74;
			break;
		case C_UNINTECH_100V_600A_300A_60A_120KW:	//ktg_200103
			//shuntV = 600mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.0kohm = 9.2333
			//V1 = shuntV * G1 = 5540mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.54;
			break;
		/*case C_LGC_110V_600A_300A_100A_216KW:	//ljh_201103					
		case C_LGC_110V_600A_300A_100A_216KW_2:	//ljh_201103					
		case C_LGC_110V_600A_300A_100A_216KW_3:	//ljh_201103					
		case C_LGC_110V_600A_300A_100A_216KW_4:	//ljh_201103					
			//ShuntV = 800mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.7kohm = 7.41558
			//V1 = ShuntV * G1 = 5932.4675324675mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5932.4675324675;
			break;*/
		case C_KATECH_120V_150A_100A_50A_18KW:	//jhkw_130621
		//case C_LGC_120V_250A_50A_60KW:	//jhk_160617	//shh_231027
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 750mV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		case C_LGC_120V_250A_50A_60KW_2:	//jhk_161101
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.0kohm = 9.233333
			//V1 = shuntV * G1 = 5770.83333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.77083333;
			break;
		case C_SKI_120V_400A_100A_192KW:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_2:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_3:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_4:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_5:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_6:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_7:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_8:	//jhk_170628
			//shuntV = 666.7mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 666.7mV * G1 = 5371.42857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.37142857;
			break;
		case C_SKI_120V_400A_200A_100A_50A_192KW:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_2:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_3:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_4:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_5:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_6:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_7:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_8:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_9:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_10:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_11:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_12:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_13:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_14:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_15:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_16:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_17:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_18:	//jhk_180206
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
		//case C_TEST_120V_400A_200A_100A_192KW:		//shh_200723
			//shuntV = 666.7mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9784946236559;
			ret_val = (double)400000000 * 5.0 / 5.9784946236559; //kjhw_190304
			break;
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
			//shuntV = 666.7mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714
			//V1 = 666.7mV * G1 = 5371.4285714mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.3714285714;
			break;
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
			//shuntV = 666.7mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.05714
			//V1 = 666.7mV * G1 = 5371.4285714mV
			//ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.3714285714;
			ret_val = (double)400000000 * 5.0 / 5.3714285714;
			break;
		case C_SDI_120V_500A_250A_50A_120KW:	//ktg_190201
		case C_SDI_120V_500A_250A_50A_120KW_2:	//ktg_190201
			//shuntV = 833.333mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5979.1666667mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9791666667;
			break;
		case C_CSR_125V_200A_100A_50A_50KW:		//jhkw_121224
		case C_CSR_125V_200A_100A_50A_50KW_2:	//jhkw_121224
		case C_CSR_125V_200A_100A_50A_50KW_3:	//jhkw_121224
		case C_CSR_125V_200A_100A_50A_50KW_4:	//jhkw_121224
		case C_HUOJU_125V_200A_100A_50KW:	//jhk_151107
		case C_HUOJU_125V_200A_100A_50KW_2:	//jhk_151107
		case C_GEELY_130V_300A_150A_78KW:	//shh_210406
		case C_GEELY_130V_300A_150A_78KW_2:	//shh_210406
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_GEELY_130V_600A_300A_156KW:   //shh_210406
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//V1 = shuntV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.94;
			break;
		case C_CTP_150V_150A_100A_90KW:	//ktg_191111
		//case C_LGC_150V_600A_300A_100A_300KW:	//shh_210819
		//case C_LGC_150V_600A_300A_100A_300KW_2:	//shh_210819
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//V1 = shuntV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.94;
			break;
		case C_SBL_150V_250A_10A_38KW: //csk_111007
		case C_SBL_150V_250A_10A_75KW:
		case C_SBL_150V_250A_10A_75KW_2:
		case C_SBL_150V_250A_10A_75KW_3:
		case C_SBL_150V_250A_10A_75KW_4: //csk_111007
		case C_SBL_150V_250A_10A_75KW_5: //csk_111007
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.0kohm = 9.233333
			//V1 = shuntV * G1 = 5770.83333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.77083333;
			break;
		/*case C_LGC_150V_250A_100A_75KW:		//ktg_190507	//shh_231024
		case C_LGC_150V_250A_100A_150KW:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_2:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_3:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_4:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_5:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_6:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_7:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_8:	//jhk_181014
		case C_LGC_150V_250A_100A_150KW_9:	//ktg_190514
		case C_LGC_150V_250A_100A_150KW_10:	//ktg_190514
		case C_LGC_150V_250A_100A_150KW_11:	//ktg_190514
		case C_LGC_150V_250A_100A_150KW_12:	//ktg_190514*/	//shh_231024
		case C_KORID_160V_500A_100A_80KW:	//ktg_190412
			//shuntV = 833.333mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5979.1666667mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9791666667;
			break;
		case C_TUV_150V_300A_100A_180KW:	//ktg_190429
			//shuntV = 800mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5740mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.74;
			break;
		/*case C_LGC_150V_350A_100A_105KW:	//ktg_190510
			//shuntV = 1166.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 12.0kohm = 5.11667
			//V1 = shuntV * G1 = 5969.445mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9694444445;
			break;*/
		/*case C_LGC_150V_350A_100A_210KW:	//ktg_190421
		case C_LGC_150V_350A_100A_210KW_2:	//ktg_190421
		case C_LGC_150V_350A_100A_210KW_3:	//ktg_190421
		case C_LGC_150V_350A_100A_210KW_4:	//ktg_190421	//shh_231025
		case C_LGC_200V_350A_100A_280KW:	//ktg_190425
			//shuntV = 466.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.17kohm = 12.84652278
			//V1 = shuntV * G1 = 5995.043964mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.995043964;
			break;*/	
		case C_LGE_150V_400A_60KW:	//jhk_170721
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774193548
			//V1 = 666.667mV * G1 = 5978.4946237mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9784946237;
			break;
		/*case C_LGC_150V_450A_200A_270KW:	//ktg_190421
		case C_LGC_150V_450A_200A_270KW_2:	//ktg_190421
		case C_LGC_150V_450A_200A_270KW_3:	//ktg_190421
		case C_LGC_150V_450A_200A_270KW_4:	//ktg_190421
			//shuntV = 600mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.49kohm = 9.9981785064
			//V1 = shuntV * G1 = 5998.907103825mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.998907103825;
			break;*/	//shh_231025
		case C_HYUNDAICE_150V_500A_250A_150KW:	//ktg_190520
			//shuntV = 833.333mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5979.1666667mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9791666667;
			break;
		case C_KTL_150V_600A_300A_200KW:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_2:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_3:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_4:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_5:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_6:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_7:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_8:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_9:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_10:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_11:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_12:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_13:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_14:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_15:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_16:	//ktg_211002
		case C_KTL_150V_600A_300A_200KW_17:	//ktg_211002
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10.0kohm = 5.94
			//V1 = shuntV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.94;
			break;
		case C_SEBANG_200V_100A_10A:
		case C_SEBANG_200V_100A_10A_2:
		case C_SEBANG_200V_100A_10A_3:
		case C_SEBANG_200V_100A_10A_4:
		case C_SEBANG_200V_100A_10A_5:
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		/*case C_LGC_200V_150A_50A_60KW:	//jhk_170905
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.8kohm = 8.26470588
			//V1 = shuntV * G1 = 6198.5294mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.198529412;
			break;*/	//shh_231025
		case C_SEBANG_200V_200A_10A:
		case C_ECOCAR_200V_200A_100A_50A_40KW:  //jhkw_130827
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_NORTHVOLT_200V_400A_200A_160KW:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_2:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_3:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_4:	//ktg_191203
			//shuntV = 666.7mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9784946236559;
			break;
		/*case C_LGC_200V_300A_100A_50A_10A_120KW:	//jhkw_120813
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 750mV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;*/
		case C_UTP_200V_500A_250A_100KW:	//ljh_201119
			//shuntV = 1000mV
			//AMP -> OP270
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//V1 = 1000mV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.940;
			break;
		//case C_LGC_220V_600A_300A_100A_528KW:       //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_2:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_3:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_4:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_5:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_6:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_7:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_8:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_9:     //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_10:    //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_11:    //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_12:    //phb_220228
        //case C_LGC_220V_600A_300A_100A_528KW_13:    //phb_220228
		/*	//shuntV = 1000.0000mV
            //AMP = AD620A
            //G1 = 1 + 49.4Kohm / 10.00Kohm = 5.9400
            //V1 = shuntV * G1 = 5940.00000000mV
            ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.94000000;
            break;*/
		//case C_LGC_250V_150A_100A_75KW:		//ktg_190510	//shh_231025
		//case C_LGC_250V_150A_100A_150KW:	//ktg_190513
		//case C_LGC_250V_150A_100A_150KW_2:	//ktg_190513	//shh_231025
		case C_LGC_350V_150A_100A_105KW:	//khj_191230
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5kohm = 10.88
			//V1 = mV * G1 = 5440mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.440;
			break;
		/*case C_LGC_350V_300A_100A_420KW:	//shh_220128
		case C_LGC_350V_300A_100A_420KW_2:	//shh_220128
		case C_LGC_350V_300A_100A_420KW_3:	//shh_220128
		case C_LGC_350V_300A_100A_420KW_4:	//shh_220128
		case C_LGC_600V_300A_100A_720KW:	//shh_220204
		case C_LGC_600V_300A_100A_720KW_2:	//shh_220204
		case C_LGC_600V_300A_100A_720KW_3:	//shh_220204
		case C_LGC_600V_300A_100A_720KW_4:	//shh_220204
			//shuntV = 666.6667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.17kohm = 9.0064
			//V1 = shuntV * G1 = 6004.321988mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.004321988;
			break;*/
		/*case C_LGC_250V_350A_100A_175KW:	//jhk_171023
			//shuntV = 583.333mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.5kohm = 9.98181818
			//V1 = shuntV * G1 = 5822.7272727mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.8227272727;
			break;*/	//shh_231025
		case C_LGC_250V_350A_250A_100A_175KW: //jhk_181108
			//shuntV = 1166.67mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 12kohm = 5.1166666667
			//V1 = mV * G1 = 5969.4444mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.96944444445;
			break;
		case C_OCI_220V_250A_100A_110KW:		//jhk_131220
		case C_SEBANG_300V_250A_10A_150KW:
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.0kohm = 9.233333
			//V1 = shuntV * G1 = 5770.83333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.77083333;
			break;
		case C_KTC_300V_100V_250A_100A_25A_150KW:	//jhk_151107
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.8kohm = 9.5172413793
			//V1 = shuntV * G1 = 5948.2758620689mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.948275862;
			break;
		case C_HYUNDAI_300V_300A_150A_180KW:	//ljh_201106
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//V1 = shuntV * G1 = 1000mV * 5.94 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.94;
			break;
		case C_KBIA_300V_200A_100A_120KW:	  	//shh_200919	
		case C_SDIXIAN_300V_400A_100A_50A_25A:	//jhk_160527
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.967741935
			//V1 = shuntV * G1 = 5978.4946236559mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9784946237;
			break;
		case C_LGC_400V_60A_10A:
		case C_ROTEM_400V_60A_10A:
		case C_KATECH_400V_60A_10A_24KW:
			//shuntV = 600mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.5kohm = 9.981818182
			//V1 = shuntV * G1 = 5989.090909mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.989090909;
			break;
		case C_LGC_400V_100A_10A_40KW:
		case C_SKI_400V_100A_10A_40KW: //jhk_120112
		case C_3PSYSTEM_400V_100A_10A_40KW_1:	//jhk_120511
		case C_3PSYSTEM_400V_100A_10A_40KW_2:	//jhk_120708
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_AVL_400V_200A_10A_80KW:
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_ERAE_400V_300A_100A_30KW:	//jhk_150119
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 750mV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
//lki_111029		case C_PNE_450V_250A_10A_115KW_2: //csk_110711
			//shuntV = 833mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5976.775mV
//			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.976775;
//			break;
		case C_KATECH_450V_150A_10A:
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_LGC_450V_200A_10A:
		case C_LGC_450V_200A_10A_2:
		case C_LGC_450V_200A_10A_3:
		case C_LGC_450V_200A_10A_4:
		case C_LGC_450V_200A_10A_5:
		case C_LGC_450V_200A_10A_6:
		case C_SK_450V_200A_10A_180KW:
		case C_ROTEM_450V_200A_10A:
		case C_SK_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_HYUNDAI_450V_200A_10A_90KW:
		case C_HYUNDAI_450V_200A_10A_90KW_2:
		case C_VENS_450V_200A_10A_90KW:
		case C_KEPCO_450V_200A_10A_90KW:
		case C_LGC_450V_200A_20A_180KW:
		case C_LGC_450V_200A_20A_180KW_2:
		case C_SK_450V_200A_10A_360KW:
		case C_LGC_450V_200A_10A_180KW:
		//case C_LGC_450V_200A_10A_180KW_2:	//ktg_210222
		case C_CTNT_450V_200A_10A_180KW:	//kjh_110929
		case C_LGC_450V_200A_10A_180KW_3:	//kjhw_121102
		case C_LGC_450V_200A_10A_180KW_4:	//jhkw_130909
		case C_DAEWOO_450V_200A_50A_90KW:	//kjh_150518
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_NKT_450V_200A_100A_90KW:	//ktg_190111
		case C_NKT_450V_200A_100A_90KW_2:	//ktg_190111
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5440mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.440;
			break;
		case C_SDI_450V_225A_22A_202KW:		//jhkw_130629
			//shuntV = 562.5mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.2kohm = 10.5
			//V1 = shuntV * G1 = 5906.25mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.90625;
			break;
		//case C_LGC_450V_200A_10A_180KW_2:	//ktg_210222	//shh_231024
		case C_KATECH_450V_250A_10A_115KW:
		//case C_PNE_450V_250A_10A_115KW: //kjg_111212_w?
		case C_PNE_450V_250A_10A_115KW: //jhk_131201
		case C_PNE_450V_250A_10A_115KW_2: //lki_111029
		case C_SEM_450V_250A_10A_225KW:
		case C_LGC_450V_250A_10A_225KW: //jhk_140728
		case C_KATECH_450V_250A_25A_225KW:
		case C_KATECH_450V_250A_25A_225KW_2:
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.0kohm = 9.233333
			//V1 = shuntV * G1 = 5770.83333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.77083333;
			break;
		case C_KEPCO_500V_200A_10A_100KW:
		case C_NEXCON_500V_200A_10A:
		case C_KTL_500V_200A_10A_200KW_3:	//jhkw_130629
		case C_EIG_500V_200A_10A_200KW:
		case C_HLGP_500V_200A_10A_200KW:
		case C_LGC_500V_200A_10A_200KW:
		case C_HLGP_500V_200A_100A_200KW: //kjhw_120527
		case C_HLGP_500V_200A_100A_200KW_2: //jhkw_120528
		case C_HLGP_500V_200A_100A_200KW_3: //jhkw_130223
		case C_HLGP_500V_200A_100A_200KW_4: //jhkw_130223
		case C_HLGP_500V_200A_100A_200KW_5: //jhk_131215
		case C_LGC_500V_200A_100A_200KW:    //jhkw_130305
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_NS_500V_250A_10A:
		case C_PNE_500V_250A_10A_125KW:
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.74kohm = 9.606271
			//V1 = shuntV * G1 = 6003.919861mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.003919861;
			break;
		//case C_LGE_450V_250A_50A_112KW:		//jhk_150703
		case C_LGE_450V_250A_50A_225KW:		//jhk_150605
		case C_LGE_450V_250A_50A_225KW_2:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_3:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_4:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_5:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_6:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_7:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_8:	//jhk_150605
		case C_LGE_450V_250A_50A_225KW_9:	//jhk_150605
		//case C_LGC_450V_250A_50A_225KW:		//jhk_160608
		//case C_LGC_450V_250A_50A_225KW_2:	//jhk_160608	//shh_231024
		//case C_LGC_450V_250A_100A_225KW:	//jhk_170211	//shh_231024
		case C_KMI_450V_250A_100A_112KW:	//jhk_171110
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.0kohm = 9.233333
			//V1 = shuntV * G1 = 5770.83333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.77083333;
			break;
		case C_LGCCHINA_500V_150A_75KW:		//jhk_160727
		case C_LGCCHINA_500V_150A_75KW_2:	//jhk_160727
		case C_LGCCHINA_500V_150A_75KW_3:	//jhk_160727
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 750mV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		case C_LGE_500V_250A_50A_250KW:	//jhk_150607
		//case C_LGC_500V_250A_100A_125KW:	//ktg_190620
		case C_LGC_500V_250A_100A_50A_250KW:    //jhkw_120822
		case C_KTL_500V_250A_100A_50A_250KW:	//jhkw_130320
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.0kohm = 9.233333
			//V1 = shuntV * G1 = 5770.83333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.77083333;
			break;
		case C_KBIA_500V_200A_100A_200KW:		//shh_200918
		case C_KBIA_500V_250A_100A_250KW:		//shh_200916
		case C_JBTP_500V_250A_100A_500KW:		//shh_200927
		case C_LGC_500V_250A_100A_50A_250KW_2:	//ktg_200208
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.967741
			//V1 = shuntV * G1 = 5978.4946mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9784946;
			break;
		case C_HYUNDAI_500V_250A_125A_25A_250KW: //ktg_190324
		case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
		/*case C_LGC_500V_250A_125A_50A_500KW_2: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_3: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_4: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_5: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_6: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_7: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_8: //ktg_190227
		case C_LGC_500V_250A_125A_50A_500KW_9: //ktg_190227*/	//ktg_231124
		//case C_LGC_500V_250A_125A_50A_500KW_10: //ktg_190615
		//case C_LGC_500V_250A_125A_50A_500KW_11: //ktg_190615	//shh_231024
		case C_LGC_500V_250A_125A_50A_500KW_12: //ktg_191108
			//shuntV = 833.333mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5979.1666667mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9791666667;
			break;
		case C_SDI_500V_300A_100A_75KW:		//ktg_200809
		case C_SDI_500V_300A_100A_150KW:	//ktg_200809
		case C_SDI_500V_300A_100A_150KW_2:	//ktg_200809
		case C_SDI_500V_300A_100A_150KW_3:	//ktg_200809
		case C_SDI_500V_300A_100A_150KW_4:	//ktg_200809
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = 750mV * G1 = 5988.8889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_KBIA_500V_300A_100A_150KW:	//shh_200919
			//shuntV = 800mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.7kohm = 7.41558442
			//V1 = 800mV * G1 = 5932.46753246mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.932467532;
			break;
		case C_HLGP_500V_300A_100A_300KW:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_2:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_3:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_4:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_5:	//jhk_140305
		case C_HLGP_500V_300A_100A_300KW_6:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_7:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_8:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_9:	//jhk_141127
		case C_HLGP_500V_300A_100A_300KW_10:	//jhk_170722
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 750mV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		//case C_LGC_500V_300A_100A_300KW:	//ktg_190418
		case C_LGC_500V_400A_200A_400KW:	//ktg_190716
			//shuntV = 800mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8kohm = 7.175
			//V1 = 800mV * G1 = 5740mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.740;
			break;
		case C_GANGSO_500V_400A_200KW:	//jhk_160518
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 666.667mV * G1 = 5371.42857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.3714285714;
			break;
		case C_HLGP_500V_400A_200KW:	//jhk_170625
		case C_SKI_500V_400A_200KW:	//jhk_171015
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774193548
			//V1 = 666.667mV * G1 = 5978.4946237mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9784946237;
			break;
		case C_SDI_600V_60A_72KW:			//shh_201202
		case C_SDI_600V_60A_72KW_2:			//shh_201202
		case C_SDI_600V_60A_72KW_3:			//shh_201202
		case C_SDI_600V_60A_72KW_4:			//shh_201202
		case C_SDI_600V_60A_72KW_5:			//shh_201202
		case C_SDI_600V_60A_72KW_6:			//shh_201202
		case C_SDI_600V_60A_72KW_7:			//shh_201202
		case C_SDI_600V_60A_72KW_8:			//shh_201202
		case C_SDI_600V_60A_72KW_9:			//shh_201202
		case C_SDI_600V_60A_72KW_10:		//shh_201202
		case C_SDI_600V_60A_72KW_11:		//shh_201202
		case C_SDI_600V_60A_72KW_12:		//shh_201202
		case C_SDI_600V_60A_72KW_13:		//shh_201202
		case C_SDI_600V_60A_72KW_14:		//shh_201202
		case C_SDI_600V_60A_72KW_15:		//shh_201202
		case C_SDI_600V_60A_72KW_16:		//shh_201202
		case C_SDI_600V_60A_72KW_17:		//shh_201202
		case C_SDI_600V_60A_72KW_18:		//shh_201202
		case C_SDI_600V_60A_72KW_19:		//shh_201202
		case C_SDI_600V_60A_72KW_20:		//shh_201202
		case C_SDI_600V_60A_72KW_21:		//shh_201202
		case C_SDI_600V_60A_72KW_22:		//shh_201202
		case C_SDI_600V_60A_72KW_23:		//shh_201202
		case C_SDI_600V_60A_72KW_24:		//shh_201202
		case C_SDI_600V_60A_72KW_25:		//shh_201202
		case C_SDI_600V_60A_72KW_26:		//shh_201202
		case C_SDI_600V_60A_72KW_27:		//shh_201202
		case C_SDI_600V_60A_72KW_28:		//shh_201202
		case C_SDI_600V_60A_72KW_29:		//shh_201202
		case C_SDI_600V_60A_72KW_30:		//shh_201202
		case C_SDI_600V_60A_72KW_31:		//shh_201202
		case C_SDI_600V_60A_72KW_32:		//shh_201202
		case C_SDI_600V_60A_72KW_33:		//shh_201202
		case C_SDI_600V_60A_72KW_34:		//shh_201202
		case C_SDI_600V_60A_72KW_35:		//shh_201202
		case C_SDI_600V_60A_72KW_36:		//shh_201202
		case C_SDI_600V_60A_72KW_37:		//shh_201202
		case C_SDI_600V_60A_72KW_38:		//shh_201202
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//V1 = 1000mV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.94;
			break;
		case C_KCL_600V_200A_100A_480KW: //ktg_190812
		case C_KCL_600V_200A_100A_480KW_2: //ktg_190812
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.5kohm = 8.6
			//V1 = 666.667mV * G1 = 5733.333333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.733333333;
			break;
		//case C_LGC_500V_250A_100A_50A_250KW:	//jhkw_120809
		case C_KTL_600V_200A_100A_20A_400KW:	//jhk_180731
		case C_SEBANG_600V_200A_100A_240KW:	//jhk_160223
		case C_SKC_600V_200A_100A_50A_240KW: //jhkw_120807
		case C_SKC_600V_200A_100A_50A_240KW_2: //jhkw_121019
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_LGE_600V_250A_50A_150KW:	//jhk_150520
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.0kohm = 9.233333
			//V1 = shuntV * G1 = 5770.83333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.77083333;
			break;
		case C_SDI_600V_300A_100A_90KW:		//ktg_200809
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = 750mV * G1 = 5988.8889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_UTP_600V_300A_100A_150KW:	//jhk_151107
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = shuntV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		//case C_LGC_600V_300A_100A_360KW:	//ktg_190620	//shh_231025
		case C_INTS_600V_300A_100A_180KW:			//shh_200513 
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10.0kohm = 5.94
			//V1 = shuntV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.940;
			break;
		case C_LGC_600V_300A_100A_360KW_2:	//ktg_200104
		case C_LGC_600V_300A_100A_360KW_3:	//ktg_200108
		case C_LGC_600V_300A_100A_360KW_4:	//ktg_200108
		case C_SEINENG_600V_300A_100A_30A_500KW:	//ktg_200112
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.0kohm = 10.88
			//V1 = shuntV * G1 = 5440mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.440;
			break;
		case C_SKE_600V_400A_100A_50A_25A_240KW:	//jhkw_130924
		case C_DAEHWA_600V_400A_200A_100A_200KW:	//jhk_170120
		case C_HYUNDAI_750V_400A_100A_240KW:	//jhk_160909
		case C_HYUNDAI_750V_400A_100A_240KW_2:	//jhk_161111
		case C_HYUNDAI_750V_400A_100A_240KW_3:	//jhk_171015
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774193548
			//V1 = shuntV * G1 = 5978.4946236559mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.978494624;
			break;
		/*
		case C_HYUNDAI_600V_400A_200A_100A_240KW:	//khj_190805
		//case C_HYUNDAI_600V_400A_200A_100A_240KW_2:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_3:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_4:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_5:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_6:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_7:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_8:	//khj_190805
		case C_HYUNDAI_600V_400A_200A_100A_240KW_9:	//khj_190805
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.17kohm = 9.00648298
			//V1 = shuntV * G1 = 6004.32499027mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.00432499;
			break;*/
		//case C_LGC_600V_150A_100A_180KW:	//ktg_190405	//shh_231024
		case C_DAEHWA_750V_300A_100A_150KW:	//jhk_170203
		case C_HANGKE_750V_300A_100A_300KW:		//jhk_131226
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 750mV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		case C_LGE_600V_150A_100A_180KW: //shh_210426
			//shuntV = 1000mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 10kohm = 5.94
			//V1 = 1000mV * G1 = 5940mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.94;
			break;
		case C_STECHWIN_800V_200A_50A_320KW:	//jhk_150812
		case C_DAEHWA_800V_200A_50A_160KW:	//jhk_160831
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_CS_800V_250A_50A_250KW:	//jhk_150812
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.8kohm = 9.5172413793
			//V1 = 625mV * G1 = 5948.275862mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.948275862;
			break;
		case C_ADD_800V_350A_200A_50A_380KW:		//shh_200904	//ktg_201028
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677419355
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9784946236559;
			break;
		case C_POWERLOGICS_1000V_200A_100A_200KW:	//jhk_180921
		case C_HYUNDAI_1000V_250A_100A_500KW:		//shh_200513
		case C_KOSTA_1000V_250A_100A_600KW:			//shh_201102
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_HLGP_1000V_300A_100A_50A_600KW:	//jhk_180810
		case C_HLGP_1000V_300A_100A_50A_600KW_2:	//jhk_180917
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 750mV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		case C_KBTP_1000V_300A_100A_300KW:	//ljh_210329
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//V1 = 500mV * G1 = 5988.8889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_HYUNDAI_1000V_500A_250A_500KW:	//jhk_180722
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 5.8kohm = 9.5172413793
			//V1 = 625mV * G1 = 5948.275862mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.948275862;
			break;
		case C_BOSUNG_1200V_200A_100A_200KW:	//jhk_170106
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_SKI_1200V_250A_100A_50A_300KW:	//jhk_131209
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.0kohm = 9.233333
			//V1 = shuntV * G1 = 5770.83333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.77083333;
			break;
		case C_KTL_1200V_300A_100A_300KW:	//kjh_171013
		case C_KATECH_1200V_300A_100A_50A_300KW:	//jhk_150108
		case C_SEBANG_1200V_300A_100A_360KW:	//jhk_160223
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 750mV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		case C_KCL_1200V_1000A_400A_600KW:		//shh_200809 	
		case C_KCL_1200V_1000A_400A_600KW_2:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_600KW_3:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_600KW_4:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_700KW:		//shh_200809 	
		case C_KCL_1200V_1000A_400A_700KW_2:	//shh_200809 	
		case C_KCL_1200V_1000A_400A_800KW:		//shh_200809 	
		case C_KCL_1200V_1000A_400A_800KW_2:	//shh_200809 
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_2:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_3:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_4:	//ktg_190814
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_5:	//ktg_190814
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.17kohm = 9.00648298
			//V1 = 750mV * G1 = 6004.321988mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.004321988;
			break;
		case C_HYUNDAI_1200V_1000A_500A_300A_600KW_6:	//shh_210625
		case C_HYUNDAI_1200V_1000A_500A_300A_800KW:	//shh_210915
		case C_HYUNDAI_1200V_1000A_500A_300A_800KW_2:	//shh_211020
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.18kohm = 8.99352750 
			//V1 = shuntV * G1 = 5995.685005393mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.995685005393;
			break;
		case C_KATECH_1200V_1000A_500A_300A_600KW:		//shh_210611
		case C_KATECH_1200V_1000A_500A_300A_600KW_2:	//shh_210611
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5978.49462365mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.97849462365;
			break;
		case C_UTP_1200V_1000A_500A_300A_600KW:			//shh_201120
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677419355
			//V1 = 666.667mV * G1 = 5978.4946236559mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9784946236559;
			break;
		case C_NCT_1500V_250A_100A_400KW:	//ljh_201113
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97778
			//V1 = shuntV * G1 = 5988.888888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888888889;
			break;
		case C_SKI_1500V_400A_100A_500KW:	//jhk_180610
		case C_KTL_1500V_400A_200A_700KW:	//jhk_180902
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774193548
			//V1 = shuntV * G1 = 5978.4946236559mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.978494624;
			break;
		case C_KTC_1500V_600A_200A_400KW:	//ktg_200807
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.977778
			//V1 = shuntV * G1 = 5988.8888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988889;
			break;
		case C_CTP_1500V_600A_300A_60A_600KW:	//ktg_191117
		case C_KEIR_1500V_600A_300A_200A_100A_720KW:	//ktg_211202
			//shuntV = 800mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.6kohm = 7.5
			//V1 = shuntV * G1 = 6000mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.0;
			break;
		case C_KCL_1500V_1000A_500A_600KW:	//jhk_181110
		case C_HYUNDAI_1500V_1000A_500A_600KW:	//jhk_170917
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.49kohm = 12.00222717
			//V1 = 500mV * G1 = 6001.1135857461mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.001113585746;
			break;
		default:
			//ret_val = (double)myPs->config.maxI[0];
			//REF_VAL : config.maxI = 5000 : i_feed
			//REF_VAL = config_maxI * 5000 / i_feed

			maxI = myPs->config.maxI[0];	//kjh_211021s
			i_feed = myPs->config.i_feed[0];

			if(myPs->config.ratioI == MICRO_UNIT) {	//uA
				ref = 5000000.0;
			} else if(myPs->config.ratioI == MILLI_UNIT) {	//mA
				ref = 5000.0;
			} else {	//A
				ref = 5.0;
			}

			ret_val = (double)maxI * ref / i_feed;	//kjh_211021e
			break;
	}

	return ret_val;
}

void AnalogValue_Operate(int ad_type, int slot)
{
	int next_slot, ch;

	if(ad_type == 0) {
		aSetMux_Ground();
		aGetAD_Reference(slot);

		next_slot = slot + 1;
		if(next_slot < 3) {
			aSetMux_Reference(next_slot);
		} else {
			aSetMux_Channel(0);
		}
	} else if(ad_type == 1) {
		ch = slot;
		aSetMux_Ground();
		aGetAD_Channel(ch);
		aSetAuxDA(ch);

		next_slot = slot + 1;
		if(next_slot < 60) {
			ch = next_slot;
			aSetMux_Channel(ch);
		} else {
			aSetMux_Temp(0);
		}
	} else if(ad_type == 2) {
		aSetMux_Ground();
		aGetAD_Temp(slot);

		next_slot = slot + 1;
		if(next_slot < 8) {
			aSetMux_Temp(next_slot);
		} else {
			aSetMux_Reference(0);
			myPs->temp[0].sens_count1_inc_flag = P1;
			myPs->temp[0].temp_slot = 0;
		}
	}

	outb(0x00, 0x617); //ad_start
}

void aGetAD_Reference(int ch)
{
	int group=0, addr, type, i;
	long tmp;
	U_ADDA ADValue;

	addr = 0x618 + 0x04 * group;
	ADValue.byte[1] = (unsigned char)inb(addr);
	ADValue.byte[0] = (unsigned char)inb(addr + 0x01);
	tmp = (long)ADValue.val;
	type = 0;
	myData->gData[group].auto_cali_ref[type].sens_AD[ch][0] = tmp;
	i = myData->gData[group].auto_cali_ref[type].ref_sens_count1;
	myData->gData[group].auto_cali_ref[type].avgSens_AD1[ch][i] = (long)tmp;

	ADValue.byte[1] = (unsigned char)inb(addr + 0x02);
	ADValue.byte[0] = (unsigned char)inb(addr + 0x03);
	tmp = (long)ADValue.val;
	type = 1;
	myData->gData[group].auto_cali_ref[type].sens_AD[ch][0] = tmp;
	i = myData->gData[group].auto_cali_ref[type].ref_sens_count1;
	myData->gData[group].auto_cali_ref[type].avgSens_AD1[ch][i] = (long)tmp;
}

void aGetAD_Channel(int ch)
{
	int group=0, i, bd, addr, type, channel;
	long tmp;
	U_ADDA ADValue;

	bd = ch / myPs->config.chPerBd;
	channel = ch % myPs->config.chPerBd;
	if(channel >= myPs->config.chInBd[bd]) return;

	addr = 0x618 + 0x04 * group;

	type = 0;
	ADValue.byte[1] = (unsigned char)inb(addr);
	ADValue.byte[0] = (unsigned char)inb(addr + 0x01);
	tmp = (long)ADValue.val;
	myData->cData[ch].misc.sens_AD[type][0] = tmp;
	i = myData->bData[bd].misc.ch_sens_count;
	myData->cData[ch].misc.avgSens_AD1[type][i] = (long)tmp;

	type = 1;
	ADValue.byte[1] = (unsigned char)inb(addr + 0x02);
	ADValue.byte[0] = (unsigned char)inb(addr + 0x03);
	tmp = (long)ADValue.val;
	myData->cData[ch].misc.sens_AD[type][0] = tmp;
	i = myData->bData[bd].misc.ch_sens_count;
	myData->cData[ch].misc.avgSens_AD1[type][i] = (long)tmp;

	addr = 0x620 + bd * 0x10;
	outb(0x00, addr + 0x01); //bd select mux off
}

void aGetAD_Temp(int ch)
{
	int group=0, i, addr, type;
	long tmp;
	U_ADDA ADValue;

	addr = 0x618;

	type = 0;
	ADValue.byte[1] = (unsigned char)inb(addr);
	ADValue.byte[0] = (unsigned char)inb(addr + 0x01);
	tmp = (long)ADValue.val;

	i = myData->gData[group].ch_sens_count;
	myData->gData[group].temp[ch].sumSens_TempAD_BD[i] = (long)ADValue.val;

	type = 1;
	ADValue.byte[1] = (unsigned char)inb(addr + 0x02);
	ADValue.byte[0] = (unsigned char)inb(addr + 0x03);
	tmp = (long)ADValue.val;

	i = myData->gData[group].ch_sens_count;
	myData->gData[group].temp[ch].sumSens_TempAD_Jig[i] = (long)ADValue.val;
}

void aSetMux_Ground(void)
{
	//capacitor discharge
	outb(0x12, 0x615);

	usleep(5);
}

void aSetMux_Reference(int ch)
{
	unsigned char mux;

	switch(ch) {
		case 0: //ref_P
			mux = 0x01; break;
		case 1: //ref_N
			mux = 0x11; break;
		case 2: //ref_0
			mux = 0x21; break;
		default: //cmd
			mux = 0x31; break;
	}

	outb(mux, 0x615);
}

void aSetMux_Channel(int ch)
{
	unsigned char mux;
	int addr, bd, i;

	bd = ch / myPs->config.chPerBd;
	ch = ch % myPs->config.chPerBd;

	if(bd >= myPs->config.installedBd) return;
	if(ch >= myPs->config.chInBd[bd]) return;

	for(i=0; i < myPs->config.installedBd; i++) {
		outb(0x00, 0x620 + i * 0x10 + 0x01);
	}

	addr = 0x620 + bd * 0x10;

	mux = 0x10;
	mux = mux << (ch / 8);
	mux |= (unsigned char)(ch % 8);
	outb(mux, addr);

	outb(0x03, addr + 0x01); //bd select mux enable

	outb(0x02, 0x615);
}

void aSetMux_Temp(int ch)
{
	unsigned char mux;

	outb(0x00, 0x620 + 0x10 * (myPs->config.installedBd - 1) + 0x01);

	mux = (unsigned char)(ch % 8);
	mux = mux << 4;
	mux |= 0x04;

	outb(mux, 0x615);
}

void aSetAuxDA(int ch)
{
	int bd, channel;

	bd = ch / myPs->config.chPerBd;
	channel = ch % myPs->config.chPerBd;

	if(bd >= myPs->config.installedBd) return;
	if(channel >= myPs->config.chInBd[bd]) return;

	aSetAuxDA_V(bd, channel, ch);
	aSetAuxDA_I(bd, channel, ch);
}

void aSetAuxDA_V(int bd, int channel, int ch)
{ //debug_size_cob
/*	unsigned char sync;
	int group=0, type=0, range, point, rtn;
	int addr, base_addr, addr_step, aux_da_type, aux_da_sync;
	int aux_da_h, aux_da_l;
	long diff=0, val=0;
	U_AUXDA_10BIT auxda_10bit;

	base_addr = 0x620;
	addr_step = 0x10;
	aux_da_sync = 0x04;
	aux_da_h = 0x05;
	aux_da_l = 0x06;
	aux_da_type = 0x03;

	if(myData->cData[ch].op.state == C_PAUSE) return;

	range = myData->bData[bd].misc.rangeV;
	point = 0;

	switch(myData->cData[ch].signal[C_SIG_AUX_DAV]) {
		case P0:
			rtn = 0;
			break;
		case P1:
			if(myData->gData[group].stepMode == MODE_CP) {
				myData->cData[ch].signal[C_SIG_AUX_DAV] = P13;
			} else {
				myData->cData[ch].signal[C_SIG_AUX_DAV]++;
			}

			val = myData->bData[bd].misc.refV
				* myData->f_cali.cData[bd][channel]
				.AUX_DA_A_P[type][range][point] / 1000000.0
				+ myData->f_cali.cData[bd][channel]
				.AUX_DA_B_P[type][range][point];
			myData->cData[ch].misc.auxDaV = (unsigned short)val + 1;
			rtn = 1;
			break;
		case P13:
			if(myData->gData[group].stepMode == MODE_CP) {
				myData->cData[ch].signal[C_SIG_AUX_DAV] = P13;
				diff = myData->cData[ch].misc.tmpVsens
					- myData->bData[bd].misc.refV;
			} else {
				myData->cData[ch].signal[C_SIG_AUX_DAV] = P2;
				if(myData->bData[bd].misc.ch_sens_count_flag == P1) {
					diff = myData->cData[ch].op.Vsens
						- myData->bData[bd].misc.refV;
				} else {
					diff = myData->cData[ch].misc.tmpVsens
						- myData->bData[bd].misc.refV;
				}
			}

			if(labs(diff) < (myPs->config.maxV[range] * 0.0003)) { //0.03%
				rtn = 0;
			} else rtn = 2;
			break;
		case P20:
			//myData->cData[ch].signal[C_SIG_AUX_DAV] = P0;
			val = 0x1FF;
			myData->cData[ch].misc.auxDaV = (unsigned short)val;
			rtn = 1;
			break;
		case P21:
			myData->cData[ch].signal[C_SIG_AUX_DAV] = P0;
			val = (long)myData->cData[ch].misc.auxDaV;
			if(val > 1023) val = 1023;
			else if(val < 0) val = 0;
			myData->cData[ch].misc.auxDaV = (unsigned short)val;
			rtn = 1;
			break;
		default:
			myData->cData[ch].signal[C_SIG_AUX_DAV]++;
			rtn = 0;
			break;
	}
	if(rtn == 0) return;
	else if(rtn == 2) return;

	addr = base_addr + addr_step * bd;
	outb(0x00, addr + aux_da_type);

	sync = 0x01;
	sync = sync << (channel / 8);
	outb(sync, addr + aux_da_sync);

	auxda_10bit.data.addr = (unsigned short)(channel % 8);
	auxda_10bit.data.val = (unsigned short)val;
	auxda_10bit.data.control = 0;
	outb(auxda_10bit.byte[1], addr + aux_da_h);
	outb(auxda_10bit.byte[0], addr + aux_da_l);

	usleep(25);*/
}

void aSetAuxDA_I(int bd, int channel, int ch)
{ //debug_size_cob
/*	unsigned char sync;
	int group=0, type=1, range, point, rtn, deltaT;
	int addr, base_addr, addr_step, aux_da_type, aux_da_sync;
	int aux_da_h, aux_da_l;
	long diff=0, tmp, val=0;
	U_AUXDA_10BIT auxda_10bit;

	memset((char *)&auxda_10bit, 0, sizeof(U_AUXDA_10BIT));

	base_addr = 0x620;
	addr_step = 0x10;
	aux_da_sync = 0x04;
	aux_da_h = 0x05;
	aux_da_l = 0x06;
	aux_da_type = 0x03;

	if(myData->cData[ch].op.state == C_PAUSE) return;

	range = myData->bData[bd].misc.rangeI;
	point = 0;
	deltaT = 300;

	switch(myData->cData[ch].signal[C_SIG_AUX_DAI]) {
		case P0:
			myData->cData[ch].misc.auxCnt = 0;
			rtn = 0;
			break;
		case P1:
			//if(myData->cData[ch].op.runTime < 50) { //0.5sec
			//	myData->cData[ch].misc.auxCnt = 0;
			//	rtn = 0;
			//	break;
			//}
			val = myData->bData[bd].misc.refI;

			if(myData->gData[group].stepMode == MODE_CP) {
				myData->cData[ch].signal[C_SIG_AUX_DAI] = P13;
			} else {
				//myData->cData[ch].signal[C_SIG_AUX_DAI] = P6;
				//myData->cData[ch].signal[C_SIG_AUX_DAI] = P2;

				if(myData->cData[ch].op.runTime < 50) { //0.5sec
					myData->cData[ch].misc.auxCnt = 0;
					myData->cData[ch].signal[C_SIG_AUX_DAI] = P1;
				} else {
					myData->cData[ch].misc.auxCnt = 0;
					myData->cData[ch].signal[C_SIG_AUX_DAI] = P2;
				}
			}

			point = aFindDACaliPoint(bd, channel, val, type, range);
			if(val >= 0) {
				point = 1; //kjg_110821_d
				val = val * myData->f_cali.cData[bd][channel]
					.AUX_DA_A_P[type][range][point] / 1000000.0
					+ myData->f_cali.cData[bd][channel]
					.AUX_DA_B_P[type][range][point];
			} else {
				point = 0; //kjg_110821_d
				val = val * (-1.0) * myData->f_cali.cData[bd][channel]
					.AUX_DA_A_P[type][range][point] / 1000000.0
					+ myData->f_cali.cData[bd][channel]
					.AUX_DA_B_P[type][range][point];
			}

			myData->cData[ch].misc.auxDaI = (unsigned short)val + 1;
			rtn = 1;
			break;
		case P7:
			diff = myData->cData[ch].op.Vsens - myData->bData[bd].misc.refV;
			if(labs(diff) < 1500) { //1.5mV
				myData->cData[ch].signal[C_SIG_AUX_DAI] = P2;
				return;
			}
			
			if(myData->cData[ch].op.runTime < deltaT) { //3sec
				diff = myData->cData[ch].misc.tmpIsens
					- myData->bData[bd].misc.refI;
				val = myData->bData[bd].misc.refI;
				myData->cData[ch].signal[C_SIG_AUX_DAI] = P2;
			} else {
				myData->cData[ch].signal[C_SIG_AUX_DAI] = P2;
				diff = myData->cData[ch].op.Isens
					- myData->bData[bd].misc.refI;
				val = myData->bData[bd].misc.refI;
			}
			if(val < 0) diff = (-1) * diff;
			diff /= 2;
			rtn = 2;
			break;
		case P13:
			if(myData->gData[group].stepMode == MODE_CP) {
				myData->cData[ch].signal[C_SIG_AUX_DAI] = P13;
				diff = myData->cData[ch].misc.tmpIsens
					- myData->bData[bd].misc.refI;

				val = myData->bData[bd].misc.refI;
				if(val < 0) diff = (-1) * diff;
				diff = diff / 2;
			}
			rtn = 2;
			break;
		case P20:
			//myData->cData[ch].signal[C_SIG_AUX_DAI] = P0;
			val = 0x1FF;
			myData->cData[ch].misc.auxDaI = (unsigned short)val;
			rtn = 1;
			break;
		case P21:
			myData->cData[ch].signal[C_SIG_AUX_DAI] = P0;
			val = (long)myData->cData[ch].misc.auxDaI + 1;
			if(val > 1023) val = 1023;
			else if(val < 0) val = 0;
			myData->cData[ch].misc.auxDaI = (unsigned short)val;
			rtn = 1;
			break;
		default:
			myData->cData[ch].signal[C_SIG_AUX_DAI]++;
			rtn = 0;
			break;
	}
	if(rtn == 0) return;
	else if(rtn == 2) {
		tmp = (long)(myPs->config.maxI[range] * 0.0002);
		if(labs(diff) < tmp) {
			myData->cData[ch].misc.auxCnt = 0;
			return;
		}
		myData->cData[ch].misc.auxCnt++;
		//if(myData->cData[ch].misc.auxCnt >= 3) {
		if(myData->cData[ch].misc.auxCnt >= 1) {
			myData->cData[ch].misc.auxCnt = 0;
			//2500mV * 100k / (100k / 1M) = 227.2727273mV
			val = myData->bData[bd].misc.refI;
			if(val >= 0) { //charge
				switch(range) {
					case 0:
						//1020202.02uA / 512 = 1992.58207
						tmp = (long)((double)diff / 1992.58207);
						break;
					case 1:
						//102038.788uA / 512 = 199.2945078
						tmp = (long)((double)diff / 199.2945078);
						break;
					default:
						//1020202.02uA / 512 = 1992.58207
						tmp = (long)((double)diff / 1992.58207);
						break;
				}
			} else { //discharge
				switch(range) {
					case 0:
						//1033057.851uA / 512 = 2017.691115
						tmp = (long)((double)diff / 2017.691115);
						break;
					case 1:
						//103324.606uA / 512 = 201.8058711
						tmp = (long)((double)diff / 201.8058711);
						break;
					default:
						//1033057.851uA / 512 = 2017.691115
						tmp = (long)((double)diff / 2017.691115);
						break;
				}
			}
			val = myData->cData[ch].misc.auxDaI - tmp;

			if(val > 1023) val = 1023;
			else if(val < 0) val = 0;
		} else {
			return;
		}

		myData->cData[ch].misc.auxDaI = (unsigned short)val;
	}

	addr = base_addr + addr_step * bd;
	outb(0x01, addr + aux_da_type);

	sync = 0x01;
	sync = sync << (channel / 8);
	outb(sync, addr + aux_da_sync);

	auxda_10bit.data.addr = (unsigned short)(channel % 8);
	auxda_10bit.data.val = (unsigned short)val;
	auxda_10bit.data.control = 0;
	outb(auxda_10bit.byte[1], addr + aux_da_h);
	outb(auxda_10bit.byte[0], addr + aux_da_l);

	usleep(25);*/
}

void aCalChAverage(int bd)
{
	int group=0, stepNo, ch, rangeV, rangeI, channel;

	if(bd >= myPs->config.installedBd) return;

	myGroup = &(myData->gData[group]);

	if(myGroup->state == G_CALI) {
		rangeV = myData->mData.signal[M_SIG_CALI_RANGE_V] - 1;
		rangeI = myData->mData.signal[M_SIG_CALI_RANGE_I] - 1;
	} else {
		stepNo = myGroup->stepNo;
		rangeV = myData->testCond[group]
			.local_object[stepNo][IDX_LOC_OBJ_RANGE_V] - 1;
		rangeI = myData->testCond[group]
			.local_object[stepNo][IDX_LOC_OBJ_RANGE_I] - 1;
	}
	if(rangeV < 0) rangeV = 0;
	if(rangeI < 0) rangeI = 0;

	for(channel=0; channel < myPs->config.chInBd[bd]; channel++) {
		ch = myPs->config.chPerBd * bd + channel;
		myCh = &(myData->cData[ch]);

		aCalCh_Voltage(bd, channel, ch, rangeV);
		aCalCh_Current(bd, channel, ch, rangeI);
		aCalCh_Capacity();
	}
}

void aCalCh_Voltage(int bd, int channel, int ch, int range)
{ //debug_size_cob
/*	int type, cnt, end, i, point, group;
	long tmp, sum, max, min, refV, refI, stepNo, maxV;
	double value, value1, value2, ratio, psum, avg, std; //, diff;

	type = 0;
	//5000mV * 1.494(AD620A 49.4/100k+1) = 7470mV
	//adRatioV = 5000000uV / 24477.696 = 204.2675912
	
	group = 0;
	stepNo = myGroup->stepNo;

	refV = myData->testCond[group].local_object[stepNo][IDX_LOC_OBJ_REF_V];
	refI = myData->testCond[group].local_object[stepNo][IDX_LOC_OBJ_REF_I];
	maxV = myPs->config.maxV[range];

	ratio = myPs->config.adRatioV[range];
	cnt = myData->bData[bd].misc.ch_sens_count;
	end = MAX_CH_SENS_COUNT;

	tmp = myCh->misc.avgSens_AD1[type][cnt];
	value = (double)tmp;
	sum = tmp;
	max = tmp;
	min = tmp;
	psum = (double)tmp;

	for(i=1; i < end; i++) {
		tmp = myCh->misc.avgSens_AD1[type][i];
		sum += tmp;
		if(tmp > max) max = tmp;
		else if(tmp < min) min = tmp;
		psum += (double)tmp;
	}

	sum = sum - max - min;
	avg = (double)sum / (end - 2);
	psum = psum / end;
	avg = (psum + avg) / 2.0;

	psum = 0.0;
	std = 0.0;
//	for(i=0; i < end; i++) {
//		value = (double)myCh->misc.avgSens_AD1[type][i];
//		psum += ((value - avg) * (value - avg));
//	}
//	std = sqrt(psum / end);
//	diff = std - myCh->misc.stdev_v;
//
//	if(std < 0.5 && fabs(diff) < 1.0) {
//		if(myGroup->state == G_CALI) {
//			myCh->misc.tmpAvgSens_AD2[type] = (long)(avg * ratio);
//			myCh->misc.avgSens_AD2[type] = (long)(avg * ratio);
//		}
//	} else {
//		myCh->misc.tmpAvgSens_AD2[type] = (long)(avg * ratio);
//		myCh->misc.avgSens_AD2[type] = (long)(avg * ratio);
//	}

	myCh->misc.tmpAvgSens_AD2[type] = (long)(value * ratio);
	myCh->misc.avgSens_AD2[type] = (long)(avg * ratio);
	myCh->misc.stdev_v = std;

	point = 0;
	cnt = 0;
	if(myGroup->state == G_CALI) {
		if(myPs->signal[M_SIG_CALI_VI_SELECT] == P0) { //voltage
			if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) cnt = 1;
			else cnt = 2;
		} else { //current
			cnt = 3;
		}
	} else {
		cnt = 3;
	}

	value1 = (double)myCh->misc.tmpAvgSens_AD2[type]
		* myGroup->auto_cali_ref[type].AD_a
		+ myGroup->auto_cali_ref[type].AD_b;

	value2 = (double)myCh->misc.avgSens_AD2[type]
		* myGroup->auto_cali_ref[type].AD_a
		+ myGroup->auto_cali_ref[type].AD_b;

	if(cnt == 1) {
		myCh->misc.tmpVsens = (long)value1;
		myCh->op.Vsens = (long)value2;
	} else if(cnt == 2) {
		point = aFindADCaliPoint(bd, channel, value1, type, range);
		myCh->misc.tmpVsens = (long)(value1
			* myData->f_cali.tmp_cData[bd][channel].AD_A_P[type][range][point]
			+ myData->f_cali.tmp_cData[bd][channel].AD_B_P[type][range][point]);

		point = aFindADCaliPoint(bd, channel, value2, type, range);
		myCh->op.Vsens = (long)(value2
			* myData->f_cali.tmp_cData[bd][channel].AD_A_P[type][range][point]
			+ myData->f_cali.tmp_cData[bd][channel].AD_B_P[type][range][point]);
	} else {
		point = aFindADCaliPoint(bd, channel, value1, type, range);
		myCh->misc.tmpVsens = (long)(value1
			* myData->f_cali.cData[bd][channel].AD_A_P[type][range][point]
			+ myData->f_cali.cData[bd][channel].AD_B_P[type][range][point]);

		point = aFindADCaliPoint(bd, channel, value2, type, range);
		myCh->op.Vsens = (long)(value2
			* myData->f_cali.cData[bd][channel].AD_A_P[type][range][point]
			+ myData->f_cali.cData[bd][channel].AD_B_P[type][range][point]);
//		myCh->op.Vsens = myCh->misc.tmpVsens;
	}*/
}

void aCalCh_Current(int bd, int channel, int ch, int range)
{ //debug_size_cob
/*	int type, cnt, end, i, point, group, cmd;
	long tmp, sum, max, min;
	double value1, value2, ratio, psum, avg, std; //, diff;
	
	type = 1;
	group = 0;

	//odden channel
	//15000mA * 0.005ohm * 40.52(AD620A 49.4k/1.25k+1) = 3039.0mV
	//3039000uV * 1.494(AD620A 49.4k/100k+1) = 4540.266mV
	//(4540.266mV/10000mV)*32768 = 14877.54363
	//adRatioI = 15000000uA / 14877.54363 = 1008.230954

	//even channel
	//15000mA * 0.010ohm * 40.52(AD620A 49.4k/1.25k+1) = 6078.0mV
	//6078000uV * 1.494(AD620A 49.4k/100k+1) = 9080.532mV
	//(9080.532mV/10000mV)*32768 = 29755.08726
	//adRatioI = 15000000uA / 29755.08726 = 504.1154768

	ratio = myPs->config.adRatioI[range];

	cnt = myData->bData[bd].misc.ch_sens_count;
	if(myData->bData[bd].misc.ch_sens_count_flag == P0 
		|| myData->bData[bd].misc.ch_sens_count_flag == P2) {
		end = cnt + 1;
	} else {
		end = MAX_CH_SENS_COUNT;
	}

	tmp = myCh->misc.avgSens_AD1[type][cnt];
	value1 = (double)tmp;
	myCh->misc.tmpAvgSens_AD2[type] = (long)value1;

	tmp = myCh->misc.avgSens_AD1[type][0];
	sum = tmp;
	max = tmp;
	min = tmp;
	psum = (double)tmp;

	for(i=1; i < end; i++) {
		tmp = myCh->misc.avgSens_AD1[type][i];
		sum += tmp;
		if(tmp > max) max = tmp;
		else if(tmp < min) min = tmp;
		psum += (double)tmp;
	}
	if(end < 3) {
		avg = (double)sum / end;
	} else {
		sum = sum - max - min;
		avg = (double)sum  / (end - 2);
	}
	psum = psum / end;
	avg = (psum + avg) / 2.0;

	psum = 0.0;
	std = 0.0;
//	for(i=0; i < end; i++) {
//		value2 = (double)myCh->misc.avgSens_AD1[type][i];
//		psum += ((value2 - avg) * (value2 - avg));
//	}
//	std = sqrt(psum / end);
//	diff = std - myCh->misc.stdev_i;
//	if(std < 1.5 && fabs(diff) < 1.0) {
//	} else {
//		myCh->misc.avgSens_AD2[type] = (long)avg;
//	}
	myCh->misc.avgSens_AD2[type] = (long)avg;
	myCh->misc.stdev_i = std;

	point = 0;
	cnt = 0;
	if(myGroup->state == G_CALI) {
		if(myPs->signal[M_SIG_CALI_VI_SELECT] == P0) { //voltage
			cnt = 3;
		} else { //current
			if(myPs->signal[M_SIG_CALI_SEQUENCE] == P2) cnt = 1;
			else cnt = 2;
		}
	} else {
		cnt = 3;
	}

	cmd = myData->bData[bd].misc.VICmd[type].value;
	value1 = (double)myCh->misc.tmpAvgSens_AD2[type] * ratio;
	value1 = value1 * myGroup->auto_cali_ref[type].AD_a
			+ myGroup->auto_cali_ref[type].AD_b;

	value2 = (double)myCh->misc.avgSens_AD2[type] * ratio;
	value2 = value2 * myGroup->auto_cali_ref[type].AD_a
		+ myGroup->auto_cali_ref[type].AD_b;

	if(cnt == 1) {
		myCh->misc.tmpIsens = (long)value1;
		myCh->op.Isens = (long)value2;
	} else if(cnt == 2) {
		point = aFindADCaliPoint(bd, channel, value1, type, range);
		myCh->misc.tmpIsens = (long)(value1
			* myData->f_cali.tmp_cData[bd][channel].AD_A_P[type][range][point]
			+ myData->f_cali.tmp_cData[bd][channel].AD_B_P[type][range][point]);

		point = aFindADCaliPoint(bd, channel, value2, type, range);
		myCh->op.Isens = (long)(value2
			* myData->f_cali.tmp_cData[bd][channel].AD_A_P[type][range][point]
			+ myData->f_cali.tmp_cData[bd][channel].AD_B_P[type][range][point]);
	} else {
		point = aFindADCaliPoint(bd, channel, value1, type, range);
		myCh->misc.tmpIsens = (long)(value1
			* myData->f_cali.cData[bd][channel].AD_A_P[type][range][point]
			+ myData->f_cali.cData[bd][channel].AD_B_P[type][range][point]);

		point = aFindADCaliPoint(bd, channel, value2, type, range);
		myCh->op.Isens = (long)(value2
			* myData->f_cali.cData[bd][channel].AD_A_P[type][range][point]
			+ myData->f_cali.cData[bd][channel].AD_B_P[type][range][point]);
//		myCh->op.Isens = myCh->misc.tmpIsens;
	}*/
}

void aCalCh_Capacity(void)
{
	long tmpI;
	double tempV, tempI, acc_time;
	
	//cal tmpWatt
	//uA -> watt 1mW/div
	//nA -> watt 1uW/div
	tempI = ((double)myCh->misc.tmpVsens / 1000.0)
		* ((double)myCh->misc.tmpIsens / 1000.0);
	myCh->misc.tmpWatt = (long)(tempI / 1000.0);

	//cal watt
	//uA -> watt 1mW/div
	//nA -> watt 1uW/div
	tempI = ((double)myCh->op.Vsens / 1000.0)
		* ((double)myCh->op.Isens / 1000.0);
	myCh->op.watt = (long)(tempI / 1000.0);

	if(myCh->op.state != C_RUN) return;
	if(myCh->op.phase != P50) return;

	acc_time = 3600.0 * (1000.0 / myPs->config.scan_period);

	//cal ampareHour
	//uA -> capacity 1uAh/div
	//nA -> capacity 1nAh/div
	tmpI = myCh->op.Isens;
	if(tmpI >= 0) {
		myCh->misc.sum_charge_AmpareHour += (double)tmpI;
		myCh->op.charge_AmpareHour
			= (long)(myCh->misc.seed_charge_AmpareHour
			+ myCh->misc.sum_charge_AmpareHour / acc_time);
		if(myCh->misc.sum_charge_AmpareHour > MAX_SUM_DOUBLE) {
			myCh->misc.seed_charge_AmpareHour
				= (double)myCh->op.charge_AmpareHour;
			myCh->misc.sum_charge_AmpareHour = 0.0;
		}
	} else {
		myCh->misc.sum_discharge_AmpareHour += (double)tmpI;
		myCh->op.discharge_AmpareHour
			= (long)(myCh->misc.seed_discharge_AmpareHour
			+ myCh->misc.sum_discharge_AmpareHour / acc_time);
		if(myCh->misc.sum_discharge_AmpareHour > MAX_SUM_DOUBLE) {
			myCh->misc.seed_discharge_AmpareHour
				= (double)myCh->op.discharge_AmpareHour;
			myCh->misc.sum_discharge_AmpareHour = 0.0;
		}
	}

	//cal wattHour
	//uA -> watt 1mWh/div
	//nA -> watt 1uWh/div
	if(tmpI >= 0) {
		myCh->misc.sum_charge_WattHour += ((double)tempI / 1000.0);
		myCh->op.charge_WattHour
			= (long)(myCh->misc.seed_charge_WattHour
			+ myCh->misc.sum_charge_WattHour / acc_time);
		if(myCh->misc.sum_charge_WattHour > MAX_SUM_DOUBLE) {
			myCh->misc.seed_charge_WattHour
				= (double)myCh->op.charge_WattHour;
			myCh->misc.sum_charge_WattHour = 0.0;
		}
	} else {
		myCh->misc.sum_discharge_WattHour += ((double)tempI / 1000.0);
		myCh->op.discharge_WattHour
			= (long)(myCh->misc.seed_discharge_WattHour
			+ myCh->misc.sum_discharge_WattHour / acc_time);
		if(myCh->misc.sum_discharge_WattHour > MAX_SUM_DOUBLE) {
			myCh->misc.seed_discharge_WattHour
				= (double)myCh->op.discharge_WattHour;
			myCh->misc.sum_discharge_WattHour = 0.0;
		}
	}

	//cal meanV, meanI
	tempV = (double)myCh->op.meanV * (double)myCh->misc.meanSumCount;
	tempV += (double)myCh->misc.tmpVsens;
	tempI = (double)myCh->op.meanI * (double)myCh->misc.meanSumCount;
	tempI += (double)myCh->misc.tmpIsens;

	myCh->misc.meanSumCount++;
	tempV /= (double)myCh->misc.meanSumCount;
	myCh->op.meanV = (long)tempV;
	tempI /= (double)myCh->misc.meanSumCount;
	myCh->op.meanI = (long)tempI;
}

void aCalReferenceAverage1(int slot)
{
	int group=0, i, cnt, end, ref_ch, type;
	long min1, max1, sum1, tmp1;
	double value1;

	type = 0;
	if(myData->gData[group].auto_cali_ref[type].ref_sens_count_flag1 == P0) {
		end = myData->gData[group].auto_cali_ref[type].ref_sens_count1 + 1;
	} else {
		end = MAX_REF_SENS_COUNT1;
	}

	for(ref_ch=0; ref_ch < MAX_REF_CH; ref_ch++) {
		tmp1 = myData->gData[group].auto_cali_ref[type].avgSens_AD1[ref_ch][0];
		min1 = tmp1;
		max1 = tmp1;
		sum1 = tmp1;

		for(i=1; i < end; i++) {
			tmp1 = myData->gData[group].auto_cali_ref[type].avgSens_AD1[ref_ch][i];
			sum1 += tmp1;
			if(tmp1 < min1) min1 = tmp1;
			else if(tmp1 > max1) max1 = tmp1;
		}

		if(end < MAX_REF_SENS_COUNT1) {
			value1 = (double)sum1 / end;
		} else {
			value1 = (double)(sum1 - min1 - max1) / (end - 2);
		}

		if(value1 < 0) value1 = ceil(value1 -0.5);
		else value1 = floor(value1+0.5);
		cnt = myData->gData[group].auto_cali_ref[type].ref_sens_count2;
		myData->gData[group].auto_cali_ref[type].avgSens_AD2[ref_ch][cnt]
			= (long)value1;
	}

	type = 1;
	if(myData->gData[group].auto_cali_ref[type].ref_sens_count_flag1 == P0) {
		end = myData->gData[group].auto_cali_ref[type].ref_sens_count1 + 1;
	} else {
		end = MAX_REF_SENS_COUNT1;
	}
	for(ref_ch=0; ref_ch < MAX_REF_CH; ref_ch++) {
		tmp1 = myData->gData[group].auto_cali_ref[type].avgSens_AD1[ref_ch][0];
		min1 = tmp1;
		max1 = tmp1;
		sum1 = tmp1;

		for(i=1; i < end; i++) {
			tmp1 = myData->gData[group].auto_cali_ref[type].avgSens_AD1[ref_ch][i];
			sum1 += tmp1;
			if(tmp1 < min1) min1 = tmp1;
			else if(tmp1 > max1) max1 = tmp1;
		}

		if(end < MAX_REF_SENS_COUNT1) {
			value1 = (double)sum1 / end;
		} else {
			value1 = (double)(sum1 - min1 - max1) / (end - 2);
		}

		if(value1 < 0) value1 = ceil(value1 - 0.5);
		else value1 = floor(value1 + 0.5);
		cnt = myData->gData[group].auto_cali_ref[type].ref_sens_count2;
		myData->gData[group].auto_cali_ref[type].avgSens_AD2[ref_ch][cnt]
			= (long)value1;
	}
}

void aCalReferenceAverage2(int slot)
{
	int group=0, i, j, end, cnt, type;
	long min, max;
	double tmp;

	type = 0;
	cnt = myData->gData[group].auto_cali_ref[type].ref_sens_count2;
	if(myData->gData[group].auto_cali_ref[type].ref_sens_count_flag2 == P0) {
		end = cnt + 1;
	} else {
		end = MAX_REF_SENS_COUNT2;
	}

	for(i=0; i < 4; i++) {
		min = myData->gData[group].auto_cali_ref[type].avgSens_AD2[i][0];
		max = min;
		tmp = (double)min;
		for(j=1; j < end; j++) {
			tmp += (double)myData->gData[group].auto_cali_ref[type].avgSens_AD2[i][j];
			if(myData->gData[group].auto_cali_ref[type].avgSens_AD2[i][j] < min)
				min = myData->gData[group].auto_cali_ref[type].avgSens_AD2[i][j];
			else if(myData->gData[group].auto_cali_ref[type].avgSens_AD2[i][j] > max)
				max = myData->gData[group].auto_cali_ref[type].avgSens_AD2[i][j];
		}

		if(end < 3) {
			tmp = tmp / end;
		} else {
			tmp = (tmp - (double)min - (double)max) / (end - 2);
		}
		myData->gData[group].auto_cali_ref[type].tmp_value[i] = (long)tmp;
	}

	type = 1;
	cnt = myData->gData[group].auto_cali_ref[type].ref_sens_count2;
	if(myData->gData[group].auto_cali_ref[type].ref_sens_count_flag2 == P0) {
		end = cnt + 1;
	} else {
		end = MAX_REF_SENS_COUNT2;
	}

	for(i=0; i < 4; i++) {
		min = myData->gData[group].auto_cali_ref[type].avgSens_AD2[i][0];
		max = min;
		tmp = (double)min;
		for(j=1; j < end; j++) {
			tmp += (double)myData->gData[group].auto_cali_ref[type].avgSens_AD2[i][j];
			if(myData->gData[group].auto_cali_ref[type].avgSens_AD2[i][j] < min)
				min = myData->gData[group].auto_cali_ref[type].avgSens_AD2[i][j];
			else if(myData->gData[group].auto_cali_ref[type].avgSens_AD2[i][j] > max)
				max = myData->gData[group].auto_cali_ref[type].avgSens_AD2[i][j];
		}

		if(end < 3) {
			tmp = tmp / end;
		} else {
			tmp = (tmp - (double)min - (double)max) / (end - 2);
		}
		myData->gData[group].auto_cali_ref[type].tmp_value[i] = (long)tmp;
	}
}

void aCalibratorReference(void)
{
	int group=0, ref_ch, type;
	double AD_a, AD_b, val, tmp_val1, tmp_val2, tmp_val3;

	//AMP -> AD620A
	//G = 1 + 49.4kohm / 100kohm = 1.494
	//10V : 32767 = 5V : V1 -> V1 = 16383.5
	//val = V1 * G = 24476.949

	type = 0;
	val = 24476.949;
	
	tmp_val1 = (double)myData->gData[group].auto_cali_ref[type].tmp_value[0];
	tmp_val2 = (double)myData->gData[group].auto_cali_ref[type].tmp_value[1];
	tmp_val3 = (double)myData->gData[group].auto_cali_ref[type].tmp_value[2];
//	AD_a = (val - 0) / (tmp_val1 - tmp_val3);
//	AD_b = -tmp_val3 * AD_a;
	AD_a = (val - (-val)) / (tmp_val1 - tmp_val2);
	AD_b = val -tmp_val1 * AD_a;

	myData->gData[group].auto_cali_ref[type].AD_a = AD_a;
	myData->gData[group].auto_cali_ref[type].AD_b = AD_b;
	
	for(ref_ch=0; ref_ch < MAX_REF_CH; ref_ch++) {
		val = (double)myData->gData[group].auto_cali_ref[type].tmp_value[ref_ch]
			* myData->gData[group].auto_cali_ref[type].AD_a
			+ myData->gData[group].auto_cali_ref[type].AD_b;
		if(val < 0) val = ceil(val -0.5);
		else val = floor(val+0.5);
		myData->gData[group].auto_cali_ref[type].cal_value[ref_ch] = (long)val;
	}

	type = 1;
	val = 24476.949;
	
	tmp_val1 = (double)myData->gData[group].auto_cali_ref[type].tmp_value[0];
	tmp_val2 = (double)myData->gData[group].auto_cali_ref[type].tmp_value[1];
	tmp_val3 = (double)myData->gData[group].auto_cali_ref[type].tmp_value[2];
//	AD_a = (val - 0) / (tmp_val1 - tmp_val3);
//	AD_b = -tmp_val3 * AD_a;
	AD_a = (val - (-val)) / (tmp_val1 - tmp_val2);
	AD_b = val -tmp_val1 * AD_a;

	myData->gData[group].auto_cali_ref[type].AD_a = AD_a;
	myData->gData[group].auto_cali_ref[type].AD_b = AD_b;
	
	for(ref_ch=0; ref_ch < MAX_REF_CH; ref_ch++) {
		val = (double)myData->gData[group].auto_cali_ref[type].tmp_value[ref_ch]
			* myData->gData[group].auto_cali_ref[type].AD_a
			+ myData->gData[group].auto_cali_ref[type].AD_b;
		if(val < 0) val = ceil(val -0.5);
		else val = floor(val+0.5);
		myData->gData[group].auto_cali_ref[type].cal_value[ref_ch] = (long)val;
	}
}

void aSetMainDA_V(int bd, long val)
{
	int group=0;
	U_ADDA DAValue;

	DAValue.val = (short int)val;

	outb(DAValue.byte[1], 0x610);
	outb(DAValue.byte[0], (0x610 + 0x01 + 0x02 * group));
}

void aSetMainDA_I(int bd, long val)
{
	int group=0;
	U_ADDA DAValue;

	DAValue.val = (short int)val;

	outb(DAValue.byte[1], 0x610);
	outb(DAValue.byte[0], (0x610 + 0x02 + 0x02 * group));
}

int aFindADCaliPoint(int bd, int ch, long value, int type, int range)
{
	int point;

	if(type == 0) {
		point = aFindADCaliPoint_1(bd, ch, value, type, range);
	} else {
		value = (long)myData->bData[bd].misc.VICmd[type].value;
		point = aFindADCaliPoint_2(bd, ch, value, type, range);
	}

	return point;
}

int aFindADCaliPoint_1(int bd, int ch, long value, int type, int range)
{ //debug_size_cob
/*	int point, point1, pointNum;
	double ad;

	if(myData->gData[0].state == G_CALI) {
		pointNum = myData->f_cali.tmp_bData[bd].DA_Cali_P_PointNum[type][range];
		for(point=0; point < pointNum; point++) {
			ad = myData->f_cali.tmp_cData[bd][ch]
				.ad_value_P[type][range][point];
			if(ad >= value) {
				if(point <= 0)	point = 0;
				else point -=1;
				return point;
			}
		}
		point1 = pointNum-1;
		ad = myData->f_cali.tmp_cData[bd][ch].ad_value_P[type][range][point1];
		if(ad < value) {
			if(point1 <= 0)  point = 0;
			else point = point1 -1;
		}
	} else {
		pointNum = myData->f_cali.cData[bd][ch].DA_Cali_P_PointNum[type][range];
		for(point=0; point < pointNum; point++) {
			ad = myData->f_cali.cData[bd][ch].ad_value_P[type][range][point];
			if(ad >= value) {
				if(point <= 0)	point = 0;
				else point -=1;
				return point;
			}
		}
		point1 = pointNum-1;
		ad = myData->f_cali.cData[bd][ch].ad_value_P[type][range][point1];
		if(ad < value) {
			if(point1 <= 0)  point = 0;
			else point = point1 -1;
		}
	}
	return point;*/
	return 0;
}

int aFindADCaliPoint_2(int bd, int ch, long value, int type, int range)
{ //debug_size_cob
/*	int point, point1, pointNum;
	double ad;

	if(myData->gData[0].state == G_CALI) {
		pointNum = myData->f_cali.tmp_bData[bd].DA_Cali_D_PointNum[type][range];
		if(value < 0){
			for(point=0; point < pointNum; point++) {
				ad = myData->f_cali.tmp_cData[bd][ch].ad_value_P[type][range][point];
				if(ad > value) {
					if(point <= 0)	point = 0;
					else point -=1;
					return point;
				}
			}
			point1 = pointNum-1;
			ad = myData->f_cali.tmp_cData[bd][ch].ad_value_P[type][range][point1];
			if(ad <= value) {
				if(point1 <= 0)  point = 0;
				else point = point1 -1;
			}
		}else{
			for(point = pointNum; 
				point < myData->f_cali.tmp_bData[bd].DA_Cali_P_PointNum[type][range]; point++){
				ad = myData->f_cali.tmp_cData[bd][ch].ad_value_P[type][range][point];
				if(ad > value) {
					if(point <= pointNum)	point = pointNum-1;
					else	point -= 2;
					return point;
				}
			}
			point1 = myData->f_cali.tmp_bData[bd].DA_Cali_P_PointNum[type][range] -1;
			ad = myData->f_cali.tmp_cData[bd][ch].ad_value_P[type][range][point1];

			if(ad <= value) {
				if(point1 <= 0)	point= 0;
				else point = point1 -2;
			}
		}
	} else {
		pointNum = myData->f_cali.cData[bd][ch].DA_Cali_D_PointNum[type][range];
		if(value < 0){
			for(point=0; point < pointNum; point++) {
				ad = myData->f_cali.cData[bd][ch].ad_value_P[type][range][point];
				if(ad > value) {
					if(point <= 0)	point = 0;
					else point -=1;
					return point;
				}
			}
			point1 = pointNum-1;
			ad = myData->f_cali.cData[bd][ch].ad_value_P[type][range][point1];
			if(ad <= value) {
				if(point1 <= 0)  point = 0;
				else point = point1 -1;
			}
		}else{
			for(point = pointNum; 
				point < myData->f_cali.cData[bd][ch].DA_Cali_P_PointNum[type][range]; point++){
				ad = myData->f_cali.cData[bd][ch].ad_value_P[type][range][point];
				if(ad > value) {
					if(point <= pointNum)	point = pointNum-1;
					else	point -= 2;
					return point;
				}
			}
			point1 = myData->f_cali.cData[bd][ch].DA_Cali_P_PointNum[type][range]-1;
			ad = myData->f_cali.cData[bd][ch].ad_value_P[type][range][point1];

			if(ad <= value) {
				if(point1 <= 0)	point= 0;
				else point = point1 - 2;
			}
		}
	}
	return point;*/
	return 0;
}

int	aFindDACaliPoint(int bd, int ch, long value, int type, int range)
{
	int point;

	if(type == 0) {
		point = aFindDACaliPoint_1(bd, ch, value, type, range);
	} else {
		point = aFindDACaliPoint_2(bd, ch, value, type, range);
	}

	return point;
}

int	aFindDACaliPoint_1(int bd, int ch, long value, int type, int range)
{ //debug_size_cob
/*	int	point, point1, pointNum;
	long da;

	if(myData->gData[0].state == G_CALI) {
		pointNum = myData->f_cali.tmp_bData[bd].DA_Cali_P_PointNum[type][range];
		for(point=0; point < pointNum; point++) {
			da = myData->f_cali.tmp_bData[bd].DA_Cali_P_Cmd[type][range][point];
			if(da >= value) {
				if(point <= 0) point = 0;
				else point -= 1;

				return point;
			}
		}

		point1 = pointNum - 1;
		da = myData->f_cali.tmp_bData[bd].DA_Cali_P_Cmd[type][range][point1];
		if(da < value) {
			if(point1 <= 0)	point = 0;
			else point = point1 - 1;
		}
	} else {
		pointNum = myData->f_cali.cData[bd][ch].DA_Cali_P_PointNum[type][range];
		for(point=0; point < pointNum; point++) {
			da = myData->f_cali.cData[bd][ch].DA_Cali_P_Cmd[type][range][point];
			if(da >= value) {
				if(point <= 0) point = 0;
				else point -= 1;

				return point;
			}
		}

		point1 = pointNum - 1;
		da = myData->f_cali.cData[bd][ch].DA_Cali_P_Cmd[type][range][point1];
		if(da < value) {
			if(point1 <= 0)	point= 0;
			else point = point1 - 1;
		}
	}

	return point;*/
	return 0;
}

int	aFindDACaliPoint_2(int bd, int ch, long value, int type, int range)
{ //debug_size_cob
/*	int	point, point1, pointNum;
	long da;

	if(myData->gData[0].state == G_CALI) {
		pointNum = myData->f_cali.tmp_bData[bd].DA_Cali_D_PointNum[type][range];
		if(value < 0) {
			for(point=0; point < pointNum; point++) {
				da = myData->f_cali.tmp_bData[bd]
					.DA_Cali_P_Cmd[type][range][point];
				if(da > value) {
					if(point <= 0) point = 0;
					else point -= 1;

					return point;
				}
			}

			point1 = pointNum - 1;
			da = myData->f_cali.tmp_bData[bd]
				.DA_Cali_P_Cmd[type][range][point1];
			if(da <= value) {
				if(point1 <= 0) point = 0;
				else point = point1 - 1;
			}
		} else {
			for(point=pointNum; point < myData->f_cali.tmp_bData[bd]
				.DA_Cali_P_PointNum[type][range]; point++) {
				da = myData->f_cali.tmp_bData[bd]
					.DA_Cali_P_Cmd[type][range][point];
				if(da > value) {
					if(point <= pointNum) point = pointNum - 1;
					else point -= 2;

					return point;
				}
			}

			point1 = myData->f_cali.tmp_bData[bd]
				.DA_Cali_P_PointNum[type][range] - 1;
			da = myData->f_cali.tmp_bData[bd]
				.DA_Cali_P_Cmd[type][range][point1];
			if(da <= value) {
				if(point1 <= 0)	point= 0;
				else point = point1 - 2;
			}
		}
	} else {
		pointNum = myData->f_cali.cData[bd][ch].DA_Cali_D_PointNum[type][range];
		if(value < 0) {
			for(point=0; point < pointNum; point++) {
				da = myData->f_cali.cData[bd][ch]
					.DA_Cali_P_Cmd[type][range][point];
				if(da > value) {
					if(point <= 0) point = 0;
					else point -= 1;

					return point;
				}
			}

			point1 = pointNum - 1;
			da = myData->f_cali.cData[bd][ch]
				.DA_Cali_P_Cmd[type][range][point1];
			if(da < value) {
				if(point1 <= 0)	point= 0;
				else point = point1 - 1;
			}
		} else {
			for(point=pointNum; point < myData->f_cali.cData[bd][ch]
				.DA_Cali_P_PointNum[type][range]-1; point++) {
				da = myData->f_cali.cData[bd][ch]
					.DA_Cali_P_Cmd[type][range][point];
				if(da > value) {
					if(point <= pointNum) point = pointNum - 1;
					else point -= 2;

					return point;
				}
			}

			point1 = myData->f_cali.cData[bd][ch]
				.DA_Cali_P_PointNum[type][range] - 1;
			da = myData->f_cali.cData[bd][ch]
				.DA_Cali_P_Cmd[type][range][point1];
			if(da <= value) {
				if(point1 <= 0)	point= 0;
				else point = point1 - 2;
			}
		}
	}

	return point;*/
	return 0;
}

//kjg_logic_type_140325_s
void Analog_Value_Input_3(int fifo)
{
	int addr, i, j, k;
	U_ADDA ADValue;

	if(myPs->misc.bd_sensCountFlag == P0) return;

	addr = 0x660 + fifo;

	for(i=0; i < 7; i++) {
		inb(addr);
	}

	for(i=0; i < 4; i++) {
		for(j=0; j < 7; j++) {
			for(k=0; k < 4; k++) {
				ADValue.byte[1] = (unsigned char)inb(addr); //high byte
				ADValue.byte[0] = (unsigned char)inb(addr); //low byte
				myPs->fifo_data[fifo][i][j][k] = (short int)ADValue.val;
			}
		}
	}
}

void Analog_Value_Input_3a(int fifo)
{
	int addr, i, j, k;
	U_ADDA ADValue;

	if(myPs->misc.bd_sensCountFlag == P0) return;

	addr = 0x660 + fifo;

	for(i=0; i < 7; i++) {
		inb(addr);
	}

	for(i=0; i < 2; i++) {
		for(j=0; j < 7; j++) {
			for(k=0; k < 4; k++) {
				ADValue.byte[1] = (unsigned char)inb(addr); //high byte
				ADValue.byte[0] = (unsigned char)inb(addr); //low byte
				myPs->fifo_data[fifo][i][j][k] = (short int)ADValue.val;
			}
		}
	}
}

void Analog_Value_Input_3b(int fifo)
{
	int addr, i, j, k;
	U_ADDA ADValue;

	if(myPs->misc.bd_sensCountFlag == P0) return;

	addr = 0x660 + fifo;

	for(i=2; i < 4; i++) {
		for(j=0; j < 7; j++) {
			for(k=0; k < 4; k++) {
				ADValue.byte[1] = (unsigned char)inb(addr); //high byte
				ADValue.byte[0] = (unsigned char)inb(addr); //low byte
				myPs->fifo_data[fifo][i][j][k] = (short int)ADValue.val;
			}
		}
	}
}

void Source_Value_Average(int fifo)
{
	int i, bd, ad_mux, ref_mux, cnt, cnt_end;
	long tmp;
	double ratio, AD_a, AD_b, val1;

	if(myPs->misc.bd_sensCountFlag == P0) return;

	bd = fifo;

	//CalSourceAverage
	for(ad_mux=0; ad_mux < 4; ad_mux++) {
		for(ref_mux=0; ref_mux < 3; ref_mux++) {
			tmp = (long)myPs->fifo_data[fifo][0][ref_mux][ad_mux];
			for(i=1; i < 4; i++) {
				tmp += (long)myPs->fifo_data[fifo][i][ref_mux][ad_mux];
			}

			switch(ad_mux) {
				case 0:
					ratio = Ref_V_Ratio(0);
					val1 = (double)tmp * ratio / 4.0;
					myData->bData[bd].misc.source[ref_mux].sourceV = (long)val1;
					break;
				case 1:
					ratio = Ref_I_Ratio(0);
					val1 = (double)tmp * ratio / 4.0;
					myData->bData[bd].misc.source[ref_mux].sourceI = (long)val1;
					break;
				case 2:
					ratio = Ref_V_Ratio(0);
					val1 = (double)tmp * ratio / 4.0;
					myData->bData[bd].misc.source[ref_mux].sourceV2
						= (long)val1;
					break;
				case 3:
					ratio = Ref_I_Ratio(0);
					val1 = (double)tmp * ratio / 4.0;
					myData->bData[bd].misc.source[ref_mux].sourceI2
						= (long)val1;
					break;
				default:
					break;
			}
		}
	}

	//CalSourceAverage2
	cnt = myPs->misc.source_sensCount;

	if(myPs->misc.source_sensCountFlag == P0) {
		cnt_end = cnt + 1;
	} else {
		cnt_end = MAX_SOURCE_SENS_COUNT;

		for(ad_mux=0; ad_mux < 4; ad_mux++) {
			for(ref_mux=0; ref_mux < 3; ref_mux++) {
				switch(ad_mux) {
					case 0:
						myData->bData[bd].misc.source2[ref_mux].totalV
							-= (double)myData->bData[bd].misc
							.source2[ref_mux].sumV[cnt];
						break;
					case 1:
						myData->bData[bd].misc.source2[ref_mux].totalI
							-= (double)myData->bData[bd].misc
							.source2[ref_mux].sumI[cnt];
						break;
					case 2:
						myData->bData[bd].misc.source2[ref_mux].totalV2
							-= (double)myData->bData[bd].misc
							.source2[ref_mux].sumV2[cnt];
						break;
					case 3:
						myData->bData[bd].misc.source2[ref_mux].totalI2
							-= (double)myData->bData[bd].misc
							.source2[ref_mux].sumI2[cnt];
						break;
					default:
						break;
				}
			}
		}
	}

	for(ad_mux=0; ad_mux < 4; ad_mux++) {
		for(ref_mux=0; ref_mux < 3; ref_mux++) {
			switch(ad_mux) {
				case 0:
					myData->bData[bd].misc.source2[ref_mux].sumV[cnt]
						= myData->bData[bd].misc.source[ref_mux].sourceV;
					myData->bData[bd].misc.source2[ref_mux].totalV
						+= (double)myData->bData[bd].misc
						.source[ref_mux].sourceV;
					myData->bData[bd].misc.source2[ref_mux].sourceV
						= (long)(myData->bData[bd].misc.source2[ref_mux].totalV
						/ cnt_end);
					break;
				case 1:
					myData->bData[bd].misc.source2[ref_mux].sumI[cnt]
						= myData->bData[bd].misc.source[ref_mux].sourceI;
					myData->bData[bd].misc.source2[ref_mux].totalI
						+= (double)myData->bData[bd].misc
						.source[ref_mux].sourceI;
					myData->bData[bd].misc.source2[ref_mux].sourceI
						= (long)(myData->bData[bd].misc.source2[ref_mux].totalI
						/ cnt_end);
					break;
				case 2:
					myData->bData[bd].misc.source2[ref_mux].sumV2[cnt]
						= myData->bData[bd].misc.source[ref_mux].sourceV2;
					myData->bData[bd].misc.source2[ref_mux].totalV2
						+= (double)myData->bData[bd].misc
						.source[ref_mux].sourceV2;
					myData->bData[bd].misc.source2[ref_mux].sourceV2
						= (long)(myData->bData[bd].misc.source2[ref_mux].totalV2
						/ cnt_end);
					break;
				case 3:
					myData->bData[bd].misc.source2[ref_mux].sumI2[cnt]
						= myData->bData[bd].misc.source[ref_mux].sourceI2;
					myData->bData[bd].misc.source2[ref_mux].totalI2
						+= (double)myData->bData[bd].misc
						.source[ref_mux].sourceI2;
					myData->bData[bd].misc.source2[ref_mux].sourceI2
						= (long)(myData->bData[bd].misc.source2[ref_mux].totalI2
						/ cnt_end);
					break;
				default:
					break;
			}
		}
	}

	//CalibratorSource
	val1 = Ref_V_Value();

	AD_a = (double)(val1 - 0.0)
		/ (double)(myData->bData[bd].misc.source2[0].sourceV
		- myData->bData[bd].misc.source2[2].sourceV);
	AD_b = val1
		- (double)myData->bData[bd].misc.source2[0].sourceV * AD_a;
	myData->bData[bd].misc.Vsource_AD_a = AD_a;
	myData->bData[bd].misc.Vsource_AD_b = AD_b;

	AD_a = (double)(0.0 - (-val1))
		/ (double)(myData->bData[bd].misc.source2[2].sourceV
		- myData->bData[bd].misc.source2[1].sourceV);
	AD_b = (double)(0.0)
		- (double)myData->bData[bd].misc.source2[2].sourceV * AD_a;
	myData->bData[bd].misc.Vsource_AD_a_N = AD_a;
	myData->bData[bd].misc.Vsource_AD_b_N = AD_b;

	for(ref_mux=0; ref_mux < 3; ref_mux++) {
		if(myData->bData[bd].misc.source2[ref_mux].sourceV >= 0) {
			myData->bData[bd].misc.source[ref_mux].calSourceV
				= (long)((double)myData->bData[bd].misc.source2[ref_mux].sourceV
				* myData->bData[bd].misc.Vsource_AD_a
				+ myData->bData[bd].misc.Vsource_AD_b);
		} else {
			myData->bData[bd].misc.source[ref_mux].calSourceV
				= (long)((double)myData->bData[bd].misc.source2[ref_mux].sourceV
				* myData->bData[bd].misc.Vsource_AD_a_N
				+ myData->bData[bd].misc.Vsource_AD_b_N);
		}
	}

	val1 = Ref_I_Value();

	AD_a = (double)(val1 - 0.0)
		/ (double)(myData->bData[bd].misc.source2[0].sourceI
		- myData->bData[bd].misc.source2[2].sourceI);
	AD_b = val1
		- (double)myData->bData[bd].misc.source2[0].sourceI * AD_a;
	myData->bData[bd].misc.Isource_AD_a = AD_a;
	myData->bData[bd].misc.Isource_AD_b = AD_b;

	AD_a = (double)(0.0 - (-val1))
		/ (double)(myData->bData[bd].misc.source2[2].sourceI
		- myData->bData[bd].misc.source2[1].sourceI);
	AD_b = (double)(0.0)
		- (double)myData->bData[bd].misc.source2[2].sourceI * AD_a;
	myData->bData[bd].misc.Isource_AD_a_N = AD_a;
	myData->bData[bd].misc.Isource_AD_b_N = AD_b;

	for(ref_mux=0; ref_mux < 3; ref_mux++) {
		if(myData->bData[bd].misc.source2[ref_mux].sourceI >= 0) {
			myData->bData[bd].misc.source[ref_mux].calSourceI
				= (long)((double)myData->bData[bd].misc.source2[ref_mux].sourceI
				* myData->bData[bd].misc.Isource_AD_a
				+ myData->bData[bd].misc.Isource_AD_b);
		} else {
			myData->bData[bd].misc.source[ref_mux].calSourceI
				= (long)((double)myData->bData[bd].misc.source2[ref_mux].sourceI
				* myData->bData[bd].misc.Isource_AD_a_N
				+ myData->bData[bd].misc.Isource_AD_b_N);
		}
	}

	val1 = Ref_V_Value();

	AD_a = (double)(val1 - 0.0)
		/ (double)(myData->bData[bd].misc.source2[0].sourceV2
		- myData->bData[bd].misc.source2[2].sourceV2);
	AD_b = val1
		- (double)myData->bData[bd].misc.source2[0].sourceV2 * AD_a;
	myData->bData[bd].misc.Vsource2_AD_a = AD_a;
	myData->bData[bd].misc.Vsource2_AD_b = AD_b;

	AD_a = (double)(0.0 - (-val1))
		/ (double)(myData->bData[bd].misc.source2[2].sourceV2
		- myData->bData[bd].misc.source2[1].sourceV2);
	AD_b = (double)(0.0)
		- (double)myData->bData[bd].misc.source2[2].sourceV2 * AD_a;
	myData->bData[bd].misc.Vsource2_AD_a_N = AD_a;
	myData->bData[bd].misc.Vsource2_AD_b_N = AD_b;

	for(ref_mux=0; ref_mux < 3; ref_mux++) {
		if(myData->bData[bd].misc.source2[ref_mux].sourceV2 >= 0) {
			myData->bData[bd].misc.source[ref_mux].calSourceV2
				= (long)((double)myData->bData[bd].misc
				.source2[ref_mux].sourceV2
				* myData->bData[bd].misc.Vsource2_AD_a
				+ myData->bData[bd].misc.Vsource2_AD_b);
		} else {
			myData->bData[bd].misc.source[ref_mux].calSourceV2
				= (long)((double)myData->bData[bd].misc
				.source2[ref_mux].sourceV2
				* myData->bData[bd].misc.Vsource2_AD_a_N
				+ myData->bData[bd].misc.Vsource2_AD_b_N);
		}
	}

	val1 = Ref_I_Value();

	AD_a = (double)(val1 - 0.0)
		/ (double)(myData->bData[bd].misc.source2[0].sourceI2
		- myData->bData[bd].misc.source2[2].sourceI2);
	AD_b = val1
		- (double)myData->bData[bd].misc.source2[0].sourceI2 * AD_a;
	myData->bData[bd].misc.Isource2_AD_a = AD_a;
	myData->bData[bd].misc.Isource2_AD_b = AD_b;

	AD_a = (double)(0.0 - (-val1))
		/ (double)(myData->bData[bd].misc.source2[2].sourceI2
		- myData->bData[bd].misc.source2[1].sourceI2);
	AD_b = (double)(0.0)
		- (double)myData->bData[bd].misc.source2[2].sourceI2 * AD_a;
	myData->bData[bd].misc.Isource2_AD_a_N = AD_a;
	myData->bData[bd].misc.Isource2_AD_b_N = AD_b;

	for(ref_mux=0; ref_mux < 3; ref_mux++) {
		if(myData->bData[bd].misc.source2[ref_mux].sourceI2 >= 0) {
			myData->bData[bd].misc.source[ref_mux].calSourceI2
				= (long)((double)myData->bData[bd].misc
				.source2[ref_mux].sourceI2
				* myData->bData[bd].misc.Isource2_AD_a
				+ myData->bData[bd].misc.Isource2_AD_b);
		} else {
			myData->bData[bd].misc.source[ref_mux].calSourceI2
				= (long)((double)myData->bData[bd].misc
				.source2[ref_mux].sourceI2
				* myData->bData[bd].misc.Isource2_AD_a_N
				+ myData->bData[bd].misc.Isource2_AD_b_N);
		}
	}
}

void Ch_Value_Average(int fifo)
{
	int ch, i, j, k, bd, ad_mux, ch_mux, cnt, cnt_end, range, point;
	int type, master_ch, acc_type;
	long tmpV, tmpI, min, max;
	double ratio, val1, tempV, tempI, acc_time;
	long ratioV, ratioI; //kjh_220321
	double ratioP; //kjh_220321

	if(myPs->misc.bd_sensCountFlag == P0) return;

	bd = fifo;

	//AD_Data_Calculate
	for(i=0; i < 4; i++) {
		ad_mux = 0;

		for(ch_mux=3; ch_mux < 7; ch_mux++) {
			ch = bd * 4 + (ch_mux - 3);

			range = (int)myData->cData[ch].op.rangeV;
			ratio = (double)Ref_V_Ratio(range);

			val1 = (double)myPs->fifo_data[fifo][i][ch_mux][ad_mux] * ratio;
			if(val1 >= 0.0) {
				val1 = val1 * myData->bData[bd].misc.Vsource_AD_a
					+ myData->bData[bd].misc.Vsource_AD_b;
			} else {
				val1 = val1 * myData->bData[bd].misc.Vsource_AD_a_N
					+ myData->bData[bd].misc.Vsource_AD_b_N;
			}

			j = myData->cData[ch].misc.sensCount + i;

			if(myData->cData[ch].op.state == C_CALI
				&& myData->cali[ch].type == CALI_TYPE_VOLTAGE) {
				if(myData->cData[ch].op.phase <= P10) {
					myData->cData[ch].misc.sensSumV[j] = (long)val1;
				} else {
					point = cFindADCaliPoint(ch, val1, ad_mux, range, 0);
					myData->cData[ch].misc.sensSumV[j]
						= (long)(val1 * myData->cali[ch]
						.tmpData[ad_mux][range].AD_A[point]
						+ myData->cali[ch].tmpData[ad_mux][range].AD_B[point]);
				}
			} else {
				point = cFindADCaliPoint(ch, val1, ad_mux, range, 1);
				myData->cData[ch].misc.sensSumV[j]
					= (long)(val1 * myData->cali[ch].data[ad_mux][range]
					.AD_A[point]
					+ myData->cali[ch].data[ad_mux][range].AD_B[point]);
			}
		}
	}

	for(i=0; i < 4; i++) {
		ad_mux = 1;

		for(ch_mux=3; ch_mux < 7; ch_mux++) {
			ch = bd * 4 + (ch_mux - 3);

			if(myData->cData[ch].op.state == C_RUN) {
				if(myData->cData[ch].misc.semiSwitchState >= SEMI_I_RANGE1
					&& myData->cData[ch].misc.semiSwitchState
					<= SEMI_I_RANGE4) {
					range = (int)myData->cData[ch].misc.semiSwitchState - 3;
				} else {
					range = (int)myData->cData[ch].op.rangeI;
				}
			} else {
				range = (int)myData->cData[ch].op.rangeI;
			}
			ratio = (double)Ref_I_Ratio(range);

			val1 = (double)myPs->fifo_data[fifo][i][ch_mux][ad_mux] * ratio;
			if(val1 >= 0.0) {
				val1 = val1 * myData->bData[bd].misc.Isource_AD_a
					+ myData->bData[bd].misc.Isource_AD_b;
			} else {
				val1 = val1 * myData->bData[bd].misc.Isource_AD_a_N
					+ myData->bData[bd].misc.Isource_AD_b_N;
			}

			j = myData->cData[ch].misc.sensCount + i;

			if(myData->cData[ch].op.state == C_CALI
				&& myData->cali[ch].type == CALI_TYPE_CURRENT) {
				if(myData->cData[ch].op.phase <= P20) {
					myData->cData[ch].misc.sensSumI[j] = (long)val1;
				} else {
					point = cFindADCaliPoint(ch, val1, ad_mux, range, 0);
					myData->cData[ch].misc.sensSumI[j]
						= (long)(val1 * myData->cali[ch]
						.tmpData[ad_mux][range].AD_A[point]
						+ myData->cali[ch].tmpData[ad_mux][range].AD_B[point]);
				}
			} else {
				point = cFindADCaliPoint(ch, val1, ad_mux, range, 1);
				myData->cData[ch].misc.sensSumI[j]
					= (long)(val1 * myData->cali[ch].data[ad_mux][range]
					.AD_A[point]
					+ myData->cali[ch].data[ad_mux][range].AD_B[point]);
			}
		}
	}

	//CalChAverage
	for(k=0; k < 4; k++) {
		ch = bd * 4 + k;
		if(ch >= MAX_CH_8) return;

		myCh = &(myData->cData[ch]);

		cnt_end = ch_sens_count_increment_2(ch);
		cnt = myCh->misc.tmp_sensCount;

		switch(myData->AppControl.config.systemModel) {
			case C_KATECH_20V_1000A_500A_100A: //csk_120309
			case C_LGC_50V_40A_10A_4A: //csk_120206
			case C_LGC_50V_40A_10A_4A_2:
			case C_LGC_50V_40A_10A_4A_3:
			case C_LGC_50V_40A_10A_4A_4:
			case C_SDI_70V_50A_5A_4KW:
			case C_SDI_70V_50A_5A_4KW_2:
			case C_SDI_70V_50A_5A_7KW:
			case C_SDI_70V_50A_5A_7KW_2:
			case C_SDI_70V_250A_25A_18KW:
			//case C_LGC_100V_500A_250A_100KW_2:	//kjhw_180213
				acc_type = 1;
				break;
			default:
				acc_type = 0;
				break;
		}
	
		type = 0; //voltage
		tmpV = myCh->misc.sensSumV[cnt];
		tempV = myCh->misc.sensSumV[0];
		min = (long)tempV;
		max = (long)tempV;
		for(i=1; i < cnt_end; i++) {
			tempV += (double)myCh->misc.sensSumV[i];
			if(myCh->misc.sensSumV[i] < min) min = myCh->misc.sensSumV[i];
			if(myCh->misc.sensSumV[i] > max) max = myCh->misc.sensSumV[i];
		}
		if(cnt_end < 3) {
			tempV /= cnt_end;
		} else {
			tempV = (tempV - (double)min - (double)max) / (cnt_end - 2);
		}
	
		switch(myData->AppControl.config.systemModel) {
			case C_KATECH_20V_1000A_500A_100A: //csk_120309
			case C_LGC_50V_40A_10A_4A: //csk_120206
			case C_LGC_50V_40A_10A_4A_2:
			case C_LGC_50V_40A_10A_4A_3:
			case C_LGC_50V_40A_10A_4A_4:
			case C_SDI_70V_50A_5A_4KW:
			case C_SDI_70V_50A_5A_4KW_2:
			case C_SDI_70V_50A_5A_7KW:
			case C_SDI_70V_50A_5A_7KW_2:
			case C_SDI_70V_250A_25A_18KW:
				if(ch >= 4) {
					myData->cali[ch].orgAD[type] = tempV;
					myData->cData[ch-4].misc.Vpower = (long)tempV;
				}
				break;
			case C_SBL_150V_250A_10A_38KW: //lki_111010
			case C_SBL_150V_250A_10A_75KW_4:
			case C_SBL_150V_250A_10A_75KW_5:
				switch(ch) {
					case 4:
						myData->cali[ch].orgAD[type] = tempV;
						myData->cData[ch-4].misc.Vpower = (long)tempV;
						break;
					case 5:
						myData->cali[ch].orgAD[type] = tempV;
						myData->cData[ch-5].misc.Vbus = (long)tempV;
						break;
					case 6:
						myData->cali[ch].orgAD[type] = tempV;
						myData->cData[ch-5].misc.Vpower = (long)tempV;
						break;
					case 7:
						myData->cali[ch].orgAD[type] = tempV;
						myData->cData[ch-6].misc.Vbus = (long)tempV;
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}

		if(myPs->config.ratioV == MICRO_UNIT
			&& myPs->config.ratioI == MICRO_UNIT) { //uV,uA
			ratioV = 1;
			ratioI = 1;
		} else if(myPs->config.ratioV == MILLI_UNIT
			&& myPs->config.ratioI == MILLI_UNIT) { //mV,mA
			ratioV = 1000;
			ratioI = 1000;
		} else {
			ratioV = 1;
			ratioI = 1;
		} //kjh_211021

		if(myCh->op.state == C_CALI) {
			myData->cali[ch].orgAD[type] = tempV;
			myCh->op.Vsens = (long)tempV;
			myCh->misc.tmpVsens = tmpV;
		} else {
			if(myData->ChAttribute[ch].chNo_master != 0) {
				if(myData->AppControl.config.debugType == 110
					|| myData->AppControl.config.debugType == 111) {
					//for debugging software
					if(myCh->op.Vsens < 0) {
						myCh->op.Vsens = 10000 / ratioV;
					} else {
						if(myCh->op.Vsens <= (100000000 / ratioV)) {
							myCh->op.Vsens += (10000000 / ratioV);
						} else myCh->op.Vsens = 1000000 / ratioV;
					}
					myCh->misc.tmpVsens = myCh->op.Vsens;
	
					myCh->misc.Vinput = myCh->op.Vsens;
					myCh->misc.Vpower = myCh->op.Vsens + (1000000 / ratioV);
					myCh->misc.Vbus = myCh->op.Vsens + (2000000 / ratioV);
					myCh->misc.reserved1[0] = myCh->misc.Vinput + (1000 / ratioV);
					myCh->misc.reserved1[1] = myCh->misc.Vinput + (2000 / ratioV);
					myCh->misc.reserved1[2] = myCh->misc.Vinput + (3000 / ratioV);
					myCh->misc.reserved1[3] = myCh->misc.Vinput + (4000 / ratioV);
					myCh->misc.reserved1[4] = myCh->misc.Vinput + (5000 / ratioV);
					myCh->misc.reserved1[5] = myCh->misc.Vinput + (6000 / ratioV);
					myCh->misc.reserved1[6] = myCh->misc.Vinput + (7000 / ratioV);
				} else if(myData->AppControl.config.debugType == 220) {
					if(myCh->op.state == C_RUN){
						if(myCh->misc.cmd_i[0] > 0){
							if(myCh->op.Vsens >= myCh->misc.cmd_v[0]) {
								myCh->op.Vsens = myCh->misc.cmd_v[0];
							} else myCh->op.Vsens += 10000;
						}else if(myCh->misc.cmd_i[0] < 0){
							if(myCh->op.Vsens >= myCh->misc.cmd_v[0]) {
								myCh->op.Vsens -= 10000;
							}else{
								myCh->op.Vsens = myCh->misc.cmd_v[0];
							}
						}
					}else{
						myCh->op.Vsens = myPs->config.maxV[0]/5;
					}
					myCh->misc.tmpVsens = myCh->op.Vsens;
				} else {
					if(acc_type == 0) {
						myCh->op.Vsens = (long)tempV;
						myCh->misc.tmpVsens = tmpV;
						//myCh->op.Vsens = 50000000; //kjhw_ttttt 180213
						//myCh->misc.tmpVsens = 50000000; //kjhw_ttttt 180213
					} else {
						if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
							myCh->op.runTime_day, myCh->op.runTime, 0, 9) > 0) {
							//<= 9
							myCh->op.Vsens = tmpV;
							myCh->misc.tmpVsens = tmpV;
						} else {
							myCh->op.Vsens = (long)tempV;
							myCh->misc.tmpVsens = tmpV;
						}
					}
				}
			} else {
				master_ch = -1;
				for(i=0; i < myData->mData.config.installedCh; i++) {
					for(j=0; j < MAX_SLAVE_CH; j++) {
						if((myData->ChAttribute[i].chNo_slave[j] - 1) == ch) {
							master_ch = myData->ChAttribute[i].chNo_master - 1;
							break;
						}
					}
					if(master_ch >= 0) break;
				}
				if(acc_type == 0) {
					if(master_ch >= 0) {
						myCh->op.Vsens = myData->cData[master_ch].op.Vsens;
						myCh->misc.tmpVsens
							= myData->cData[master_ch].misc.tmpVsens;
					} else {
						myCh->op.Vsens = (long)tempV;
						myCh->misc.tmpVsens = tmpV;
					}
				} else {
					if(master_ch >= 0) {
						myCh->op.Vsens = myData->cData[master_ch].op.Vsens;
						myCh->misc.tmpVsens
							= myData->cData[master_ch].misc.tmpVsens;
					} else {
						if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
							myCh->op.runTime_day, myCh->op.runTime, 0, 9) > 0) {
							//<= 9
							myCh->op.Vsens = tmpV;
							myCh->misc.tmpVsens = tmpV;
						} else {
							myCh->op.Vsens = (long)tempV;
							myCh->misc.tmpVsens = tmpV;
						}
					}
				}
			}
		}
		myCh->misc.tmpVsens_slave = tmpV; //jhkw_181001

		type = 1; //current
		tmpI = myCh->misc.sensSumI[cnt];
		tempI = myCh->misc.sensSumI[0];
		min = (long)tempI;
		max = (long)tempI;
		for(i=1; i < cnt_end; i++) {
			tempI += (double)myCh->misc.sensSumI[i];
			if(myCh->misc.sensSumI[i] < min) min = myCh->misc.sensSumI[i];
			if(myCh->misc.sensSumI[i] > max) max = myCh->misc.sensSumI[i];
		}
		if(cnt_end < 3) {
			tempI /= cnt_end;
		} else {
			tempI = (tempI - (double)min - (double)max) / (cnt_end - 2);
		}

		switch(myData->AppControl.config.systemModel) {
			case C_KATECH_20V_1000A_500A_100A: //csk_120309
			case C_LGC_50V_40A_10A_4A: //csk_120206
			case C_LGC_50V_40A_10A_4A_2:
			case C_LGC_50V_40A_10A_4A_3:
			case C_LGC_50V_40A_10A_4A_4:
			case C_SDI_70V_50A_5A_4KW:
			case C_SDI_70V_50A_5A_4KW_2:
			case C_SDI_70V_50A_5A_7KW:
			case C_SDI_70V_50A_5A_7KW_2:
			case C_SDI_70V_250A_25A_18KW:
				if(ch >= 4) {
					myData->cali[ch].orgAD[type] = tempI;
					myData->cData[ch-4].misc.Vbus = (long)tempI;
				}
				break;
			case C_SBL_150V_250A_10A_38KW: //lki_111010
			case C_SBL_150V_250A_10A_75KW_4:
			case C_SBL_150V_250A_10A_75KW_5:
				switch(ch) {
					case 4:
						myData->cali[ch].orgAD[type] = tempI;
						myData->cData[ch-4].misc.Vinput = (long)tempI;
						break;
					case 6:
						myData->cali[ch].orgAD[type] = tempI;
						myData->cData[ch-5].misc.Vinput = (long)tempI;
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}

		if(myCh->op.state == C_CALI) {
			myData->cali[ch].orgAD[type] = tempI;
			myCh->op.Isens = (long)tempI;
			myCh->misc.tmpIsens = tmpI;
		} else {
			if(myData->AppControl.config.debugType == 110) {
				//for debugging software
				if(myCh->op.Isens < (-100000000 / ratioI)) {
					myCh->op.Isens = 100000000 / ratioI;
				} else {
					if(myCh->op.Isens <= (100000000 / ratioI)) {
						myCh->op.Isens += (10000000 / ratioI);
					} else myCh->op.Isens = -100000000 / ratioI;
				}
				myCh->misc.tmpIsens = myCh->op.Isens;
			} else if(myData->AppControl.config.debugType == 111) {
				myCh->op.Isens = 0;
				myCh->misc.tmpIsens = 0;
			} else if(myData->AppControl.config.debugType == 220) {
				if(myCh->op.state == C_RUN){
					if(myCh->misc.cmd_i[0] > 0){
						if(myCh->op.Vsens < myCh->misc.cmd_v[0]){
							if(myCh->op.stepType == STEP_PATTERN) {
								myCh->op.Isens = myCh->misc.cmd_i[1];
							} else {
								myCh->op.Isens = myCh->misc.cmd_i[0];
							}
						}else{
							if(myCh->op.stepType == STEP_PATTERN) {
								myCh->op.Isens = myCh->misc.cmd_i[1];
							} else {
								myCh->op.Isens = myCh->misc.cmd_i[0];
							}
						}
					}else{
						if(myCh->op.stepType == STEP_PATTERN) {
							myCh->op.Isens = myCh->misc.cmd_i[1];
						} else {
							myCh->op.Isens = myCh->misc.cmd_i[0];
						}
					}
				}else{
					myCh->op.Isens = 0;
				}
				myCh->misc.tmpIsens = myCh->op.Isens;
			} else {
				if(acc_type == 0) {
					myCh->op.Isens = (long)tempI;
					myCh->misc.tmpIsens = tmpI;
					//myCh->op.Isens = 45000; //kjhw_ttttt 180213
					//myCh->misc.tmpIsens = 450000; //kjhw_ttttt 180213
					/*if(myData->test_val_i[1][0] == 0) {
						myCh->op.Isens = 50000; //kjhw_ttttt
						myCh->misc.tmpIsens = 50000; //kjhw_ttttt
					} else if(myData->test_val_i[1][0] == 1) {
						myCh->op.Isens += 1; //kjhw_ttttt
						myCh->misc.tmpIsens += 1; //kjhw_ttttt
					} else if(myData->test_val_i[1][0] == 2) {
						myCh->op.Isens -= 1; //kjhw_ttttt
						myCh->misc.tmpIsens -= 1; //kjhw_ttttt
					} else {
					}*/

				} else {
					if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
						myCh->op.runTime_day, myCh->op.runTime, 0, 9) > 0) {
						//<= 9
						myCh->op.Isens = tmpI;
						myCh->misc.tmpIsens = tmpI;
					} else {
						myCh->op.Isens = (long)tempI;
						myCh->misc.tmpIsens = tmpI;
					}
				}
			}
		}
	
		ratioV = myData->mData.ratioV;	//kjh_220321
		ratioI = myData->mData.ratioI;	//kjh_220321
		ratioP = myData->mData.ratioP;	//kjh_220321

		//cal tmpWatt
		//uA -> watt 1mW/div
		//nA -> watt 1uW/div
		tempI = ((double)myCh->misc.tmpVsens / ratioV)
			* ((double)myCh->misc.tmpIsens / ratioI);
		myCh->misc.tmpWatt = (long)(tempI / ratioP);
	
		//cal watt
		//uA -> watt 1mW/div
		//nA -> watt 1uW/div
		tempI = ((double)myCh->op.Vsens / ratioV)
			* ((double)myCh->op.Isens / ratioI);
		myCh->op.watt = (long)(tempI / ratioP);

		if(myCh->op.state != C_RUN) continue;
		if(myCh->op.phase != P50) continue;

		acc_time = 3600.0 * (1000.0 / myPs->config.scan_period);

		//cal ampareHour
		//uA -> capacity 1uAh/div
		//nA -> capacity 1nAh/div
		tmpI = myCh->op.Isens;
		if(tmpI >= 0) {
			if(acc_type == 0) {
				myCh->misc.sum_charge_AmpareHour += (double)tmpI;
				myCh->op.charge_AmpareHour
					= (long)(myCh->misc.seed_charge_AmpareHour
					+ myCh->misc.sum_charge_AmpareHour / acc_time);
				if(myCh->misc.sum_charge_AmpareHour > MAX_SUM_DOUBLE) {
					myCh->misc.seed_charge_AmpareHour
						= (double)myCh->op.charge_AmpareHour;
					myCh->misc.sum_charge_AmpareHour = 0.0;
				}
			} else {
				if(time_cal_compare(COMP_LESS_THAN,
					myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
					//< 8
				} else if(time_cal_compare(COMP_EQUAL_TO,
					myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
					//== 8
					myCh->misc.sum_charge_AmpareHour += (double)(tmpI * 8);
					myCh->op.charge_AmpareHour
						= (long)(myCh->misc.seed_charge_AmpareHour
						+ myCh->misc.sum_charge_AmpareHour / acc_time);
					if(myCh->misc.sum_charge_AmpareHour > MAX_SUM_DOUBLE) {
						myCh->misc.seed_charge_AmpareHour
							= (double)myCh->op.charge_AmpareHour;
						myCh->misc.sum_charge_AmpareHour = 0.0;
					}
				} else {
					myCh->misc.sum_charge_AmpareHour += (double)tmpI;
					myCh->op.charge_AmpareHour
						= (long)(myCh->misc.seed_charge_AmpareHour
						+ myCh->misc.sum_charge_AmpareHour / acc_time);
					if(myCh->misc.sum_charge_AmpareHour > MAX_SUM_DOUBLE) {
						myCh->misc.seed_charge_AmpareHour
							= (double)myCh->op.charge_AmpareHour;
						myCh->misc.sum_charge_AmpareHour = 0.0;
					}
				}
			}
		} else {
			if(acc_type == 0) {
				myCh->misc.sum_discharge_AmpareHour += (double)tmpI;
				myCh->op.discharge_AmpareHour
					= (long)(myCh->misc.seed_discharge_AmpareHour
					+ myCh->misc.sum_discharge_AmpareHour / acc_time);
				if(myCh->misc.sum_discharge_AmpareHour > MAX_SUM_DOUBLE) {
					myCh->misc.seed_discharge_AmpareHour
						= (double)myCh->op.discharge_AmpareHour;
					myCh->misc.sum_discharge_AmpareHour = 0.0;
				}
			} else {
				if(time_cal_compare(COMP_LESS_THAN,
					myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
					//< 8
				} else if(time_cal_compare(COMP_EQUAL_TO,
					myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
					//== 8
					myCh->misc.sum_discharge_AmpareHour += (double)(tmpI * 8);
					myCh->op.discharge_AmpareHour
						= (long)(myCh->misc.seed_discharge_AmpareHour
						+ myCh->misc.sum_discharge_AmpareHour / acc_time);
					if(myCh->misc.sum_discharge_AmpareHour > MAX_SUM_DOUBLE) {
						myCh->misc.seed_discharge_AmpareHour
							= (double)myCh->op.discharge_AmpareHour;
						myCh->misc.sum_discharge_AmpareHour = 0.0;
					}
				} else {
					myCh->misc.sum_discharge_AmpareHour += (double)tmpI;
					myCh->op.discharge_AmpareHour
						= (long)(myCh->misc.seed_discharge_AmpareHour
						+ myCh->misc.sum_discharge_AmpareHour / acc_time);
					if(myCh->misc.sum_discharge_AmpareHour > MAX_SUM_DOUBLE) {
						myCh->misc.seed_discharge_AmpareHour
							= (double)myCh->op.discharge_AmpareHour;
						myCh->misc.sum_discharge_AmpareHour = 0.0;
					}
				}
			}
		}

		//cal wattHour
		//uA -> watt 1mWh/div
		//nA -> watt 1uWh/div
		if(tmpI >= 0) {
			if(acc_type == 0) {
				myCh->misc.sum_charge_WattHour += ((double)tempI / ratioP);
				myCh->op.charge_WattHour
					= (long)(myCh->misc.seed_charge_WattHour
					+ myCh->misc.sum_charge_WattHour / acc_time);
				if(myCh->misc.sum_charge_WattHour > MAX_SUM_DOUBLE) {
					myCh->misc.seed_charge_WattHour
						= (double)myCh->op.charge_WattHour;
					myCh->misc.sum_charge_WattHour = 0.0;
				}
			} else {
				if(time_cal_compare(COMP_LESS_THAN,
					myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
					//< 8
				} else if(time_cal_compare(COMP_EQUAL_TO,
					myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
					//== 8
					myCh->misc.sum_charge_WattHour
						+= ((double)tempI / ratioP * 8.0);
					myCh->op.charge_WattHour
						= (long)(myCh->misc.seed_charge_WattHour
						+ myCh->misc.sum_charge_WattHour / acc_time);
					if(myCh->misc.sum_charge_WattHour > MAX_SUM_DOUBLE) {
						myCh->misc.seed_charge_WattHour
							= (double)myCh->op.charge_WattHour;
						myCh->misc.sum_charge_WattHour = 0.0;
					}
				} else {
					myCh->misc.sum_charge_WattHour += ((double)tempI / ratioP);
					myCh->op.charge_WattHour
						= (long)(myCh->misc.seed_charge_WattHour
						+ myCh->misc.sum_charge_WattHour / acc_time);
					if(myCh->misc.sum_charge_WattHour > MAX_SUM_DOUBLE) {
						myCh->misc.seed_charge_WattHour
							= (double)myCh->op.charge_WattHour;
						myCh->misc.sum_charge_WattHour = 0.0;
					}
				}
			}
		} else {
			if(acc_type == 0) {
				myCh->misc.sum_discharge_WattHour += ((double)tempI / ratioP);
				myCh->op.discharge_WattHour
					= (long)(myCh->misc.seed_discharge_WattHour
					+ myCh->misc.sum_discharge_WattHour / acc_time);
				if(myCh->misc.sum_discharge_WattHour > MAX_SUM_DOUBLE) {
					myCh->misc.seed_discharge_WattHour
						= (double)myCh->op.discharge_WattHour;
					myCh->misc.sum_discharge_WattHour = 0.0;
				}
			} else {
				if(time_cal_compare(COMP_LESS_THAN,
					myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
					//< 8
				} else if(time_cal_compare(COMP_EQUAL_TO,
					myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
					//== 8
					myCh->misc.sum_discharge_WattHour
						+= ((double)tempI / ratioP * 8.0);
					myCh->op.discharge_WattHour
						= (long)(myCh->misc.seed_discharge_WattHour
						+ myCh->misc.sum_discharge_WattHour / acc_time);
					if(myCh->misc.sum_discharge_WattHour > MAX_SUM_DOUBLE) {
						myCh->misc.seed_discharge_WattHour
							= (double)myCh->op.discharge_WattHour;
						myCh->misc.sum_discharge_WattHour = 0.0;
					}
				} else {
					myCh->misc.sum_discharge_WattHour
						+= ((double)tempI / ratioP);
					myCh->op.discharge_WattHour
						= (long)(myCh->misc.seed_discharge_WattHour
						+ myCh->misc.sum_discharge_WattHour / acc_time);
					if(myCh->misc.sum_discharge_WattHour > MAX_SUM_DOUBLE) {
						myCh->misc.seed_discharge_WattHour
							= (double)myCh->op.discharge_WattHour;
						myCh->misc.sum_discharge_WattHour = 0.0;
					}
				}
			}
		}

		//cal meanV, meanI
		tempV = (double)myCh->op.meanV * (double)myCh->misc.meanSumCount;
		tempV += (double)myCh->misc.tmpVsens;
		tempI = (double)myCh->op.meanI * (double)myCh->misc.meanSumCount;
		tempI += (double)myCh->misc.tmpIsens;

		myCh->misc.meanSumCount++;
		tempV /= (double)myCh->misc.meanSumCount;
		myCh->op.meanV = (long)tempV;
		tempI /= (double)myCh->misc.meanSumCount;
		myCh->op.meanI = (long)tempI;
	}
} //kjg_logic_type_140325_e

