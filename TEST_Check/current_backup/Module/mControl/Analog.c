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
	switch(myData->AppControl.config.systemModel) {
		case F_PNE_5V_15A_30AP_SW:
			Analog_Value_Input_2();
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
		case F_SDI_5V_450A_200A_100A_10A:
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

	for(bd=0; bd < installedBd; bd++) Get_AD_Value(slot, bd, ch);

	switch(myData->AppControl.config.systemModel) {
		case C_LGC_5V_200A_75A_15A:
		case F_SDI_5V_50A_5A:
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
			break;
		default:
			if(slot == 4) {
				cnt_end = bd_sens_count_increment(); //for org
			}
			break;
	}

	for(bd=0; bd < installedBd; bd++) Set_Mux(next_slot, bd, next_ch);

	if((slot % 5) < 4) {
		for(bd=0; bd < installedBd; bd++) {
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
	short int tmp, scan_slot, next_scan_slot;
	int bd, ch, next_ch, cnt_end=0, installedBd;

	installedBd = myPs->config.installedBd;

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
		if(bd > 0) return;

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
		ch += (bd * myPs->config.chPerBd);
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
		case C_NORTHVOLT_400V_200A_100A_160KW:	//jhkw_200526
		case C_KTL_600V_200A_100A_20A_400KW:	//jhk_180731
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
			&& myData->cali[ch].mode != CALI_MODE_ONLY_CHECK //jhkw_200317
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
			&& myData->cali[ch].mode != CALI_MODE_ONLY_CHECK //jhkw_200317
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
			&& myData->cali[ch].mode != CALI_MODE_ONLY_CHECK //jhkw_200317
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
		&& myData->cali[ch].mode != CALI_MODE_ONLY_CHECK //jhkw_200317
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
		myData->cData[ch].misc.sensSumI[i]
			= (long)(tmp * myData->cali[ch].data[ad_mux][range].AD_A[point]
			+ myData->cali[ch].data[ad_mux][range].AD_B[point]);
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
		&& myData->cali[ch].mode != CALI_MODE_ONLY_CHECK //jhkw_200317
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
		&& myData->cali[ch].mode != CALI_MODE_ONLY_CHECK //jhkw_200317
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
    int	cnt, cnt_end, i, j, type, master_ch, acc_type, filter_ad_count;
	long tmpV, tmpI, min, max;
    double tempV, tempI, acc_time;
	long ratioV, ratioI;	//jhkw_231127
	double ratioP;	//jhkw_231127
	
	ch += (bd * myPs->config.chPerBd);
	if(ch >= MAX_CH_PER_MODULE) return;

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
	}	//jhkw_231127

	filter_ad_count = (int)myPs->config.filter_ad_count;

	cnt = myCh->misc.sensCount;
	if(myCh->misc.sensCountFlag == P0) {
		cnt_end = cnt + 1;
	} else {
		cnt_end = filter_ad_count;
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
		case C_SKI_AUX_BOX:	//jhkw_211012
			tempV = myData->SubSensV.ch[20].sensV;
			tmpV = tempV;
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
//20181219 KHK Test-------------------------------------				
				if(myCh->op.state == C_RUN){
					if(myCh->misc.cmd_i[0] > 0){
						if(myCh->op.Vsens >= myCh->misc.cmd_v[0]) {
							myCh->op.Vsens = myCh->misc.cmd_v[0];
						} else myCh->op.Vsens += (10000 / ratioV);
					}else if(myCh->misc.cmd_i[0] < 0){
						if(myCh->op.Vsens >= myCh->misc.cmd_v[0]) {
							myCh->op.Vsens -= (10000 / ratioV);
						}else{
							myCh->op.Vsens = myCh->misc.cmd_v[0];
						}
					}
				}else{
					myCh->op.Vsens = myPs->config.maxV[0]/5;
				}
//20181219 KHK Test-------------------------------------				

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
						} else myCh->op.Vsens += (10000 / ratioV);
					}else if(myCh->misc.cmd_i[0] < 0){
						if(myCh->op.Vsens >= myCh->misc.cmd_v[0]) {
							myCh->op.Vsens -= (10000 / ratioV);
						}else{
							myCh->op.Vsens = myCh->misc.cmd_v[0];
						}
					}
				}else{
					myCh->op.Vsens = myPs->config.maxV[0]/5;
				}
				myCh->misc.tmpVsens = myCh->op.Vsens;
#if defined __DEBUG__	//shh_231124s
			} else if(myData->AppControl.config.debugType == 230) {
				myCh->op.Vsens = myCh->misc.sil_v_i_val[0];
				myCh->misc.tmpVsens = myCh->op.Vsens;
#endif					//shh_231124e
			} else {
				if(acc_type == 0) {
					myCh->op.Vsens = (long)tempV;
					myCh->misc.tmpVsens = tmpV;
				} else {
#ifdef __COA_VER_100B__
					if(myCh->op.runTime <= 9) {
#else //COA_VER_100B2~
					if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
						myCh->op.runTime_day, myCh->op.runTime, 0, 9) > 0) {
#endif
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
				for(j=0; j < 3; j++) {
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
#ifdef __COA_VER_100B__
					if(myCh->op.runTime <= 9) {
#else //COA_VER_100B2~
					if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
						myCh->op.runTime_day, myCh->op.runTime, 0, 9) > 0) {
#endif
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

	type = 1; //current
	cnt = myCh->misc.sensCount;
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
//20181219 KHK Test-------------------------------------				
			if(myCh->op.state == C_RUN){
				if(myCh->misc.cmd_i[0] > 0){
					if(myCh->op.Vsens < myCh->misc.cmd_v[0]){
						myCh->op.Isens = myCh->misc.cmd_i[0];
					}else{
						myCh->op.Isens = myCh->misc.cmd_i[0];
								
					}
				}else{
					myCh->op.Isens = myCh->misc.cmd_i[0];
				}
			}else{
				myCh->op.Isens = 0;
			}
//------------------------------------------------------			
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
#if defined __DEBUG__	//shh_231124s
		} else if(myData->AppControl.config.debugType == 230) {
			myCh->op.Isens = myCh->misc.sil_v_i_val[1];
			myCh->misc.tmpIsens = myCh->op.Isens;
#endif					//shh_231124e
		} else {
			if(acc_type == 0) {
				myCh->op.Isens = (long)tempI;
				myCh->misc.tmpIsens = tmpI;
			} else {
#ifdef __COA_VER_100B__
				if(myCh->op.runTime <= 9) {
#else //COA_VER_100B2~
				if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
					myCh->op.runTime_day, myCh->op.runTime, 0, 9) > 0) {
#endif
					myCh->op.Isens = tmpI;
					myCh->misc.tmpIsens = tmpI;
				} else {
					myCh->op.Isens = (long)tempI;
					myCh->misc.tmpIsens = tmpI;
				}
			}
		}
	}

	ratioV = myData->mData.ratioV;	//jhkw_231127s
	ratioI = myData->mData.ratioI;
	ratioP = myData->mData.ratioP;	//jhkw_231127e

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
#ifdef __COA_VER_100B__
			if(myCh->op.runTime <= 8) {
			} else if(myCh->op.runTime == 8) {
#else //COA_VER_100B2~
			if(time_cal_compare(COMP_LESS_THAN,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
			} else if(time_cal_compare(COMP_EQUAL_TO,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
#endif
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
#ifdef __COA_VER_100B__
			if(myCh->op.runTime < 8) {
			} else if(myCh->op.runTime == 8) {
#else //COA_VER_100B2~
			if(time_cal_compare(COMP_LESS_THAN,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
			} else if(time_cal_compare(COMP_EQUAL_TO,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
#endif
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
#ifdef __COA_VER_100B__
			if(myCh->op.runTime < 8) {
			} else if(myCh->op.runTime == 8) {
#else //COA_VER_100B2~
			if(time_cal_compare(COMP_LESS_THAN,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
			} else if(time_cal_compare(COMP_EQUAL_TO,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
#endif
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
#ifdef __COA_VER_100B__
			if(myCh->op.runTime < 8) {
			} else if(myCh->op.runTime == 8) {
#else //COA_VER_100B2~
			if(time_cal_compare(COMP_LESS_THAN,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
			} else if(time_cal_compare(COMP_EQUAL_TO,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
#endif
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

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
			j = 0;
			break;
		case F_SDI_5V_450A_200A_100A_10A:
			j = 1;
			break;
		default:
			break;
	}
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
	switch(myData->AppControl.config.systemModel) {
		case C_EIG_5V_50A_5A:
		case C_EIG_5V_50A_5A_2:
		case C_LGC_5V_200A_75A_15A:
		case F_SDI_5V_50A_5A:
			//sensSumI == sensSumV
			break;
		default:
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
			break;
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
	/*Simplication - shh_250417*/
	double ratio;
	long maxV; //jhkw_231127
	double da_max, v_feed, main_amp;	//jhkw_231127

	//10000000uV / 32768 = 305.1757813uV
	ratio = 305.1757813;
	switch(myData->AppControl.config.systemModel) {
		case C_SKI_20V_300A_200A_100A_12KW:		//shh_231204
		case C_SKI_20V_300A_200A_100A_12KW_2:	//shh_231204
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (20.0 / 5.0);
			break;
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
		case C_SK_60V_300A_10A:
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (60.0 / 5.0);
			break;
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
		case C_SKI_100V_100A_10A_10KW:  //jhk_120329
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (100.0 / 4.5);
			break;
		case C_SKI_100V_300A_150A_50A_10A_60KW: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_2: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_3: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_4: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_5: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_6: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_7: //kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_8: //kjh_120527
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (100.0 / 4.5);
			break;
		case C_SKI_120V_400A_100A_192KW:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_2:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_3:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_4:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_5:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_6:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_7:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_8:	//jhk_170628
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
		case C_SKI_120V_400A_200A_100A_192KW_85:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_86:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_87:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_88:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_89:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_90:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_91:	//khj_200308
		////////////////////////////////////////////////////////
		case C_SKI_120V_400A_200A_100A_192KW_92:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_93:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_94:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_95:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_96:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_97:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_98:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_99:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_100:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_101:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_102:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_103:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_104:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_105:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_106:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_107:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_108:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_109:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_110:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_111:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_112:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_113:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_114:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_115:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_116:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_117:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_118:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_119:	//khj_200308
		////////////////////////////////////////////////////////
		//case C_SKI_120V_400A_200A_100A_192KW_169:	//ktg_200410 //shh_240730
		case C_SKI_120V_400A_200A_100A_192KW_170:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_171:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_172:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_173:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_174:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_175:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_176:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_177:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_178:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_179:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_180:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_181:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_182:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_183:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_184:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_185:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_186:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_187:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_188:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_189:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_190:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_191:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_192:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_193:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_194:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_195:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_196:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_197:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_198:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_199:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_200:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_201:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_202:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_203:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_204:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_205:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_206:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_207:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_208:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_209:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_210:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_211:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_212:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_213:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_214:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_215:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_216:	//shh_200905
		//case C_SKI_120V_400A_200A_100A_192KW_232:	//shh_200914 //shh_240730
		case C_SKI_120V_400A_200A_100A_192KW_233:	//ktg_200908
		case C_SKI_120V_400A_200A_100A_192KW_234:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_235:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_236:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_237:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_238:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_239:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_240:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_241:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_242:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_243:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_244:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_245:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_246:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_247:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_248:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_249:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_250:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_251:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_252:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_253:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_254:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_255:	//shh_220113
		case C_SKI_120V_425A_200A_100A_50A_192KW:       //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_2:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_3:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_4:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_5:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_6:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_7:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_8:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_9:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_10:    //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_11:    //phb_230206
		//case C_SKI_120V_425A_300A_200A_100A_192KW:      //phb_230105
        //case C_SKI_120V_425A_300A_200A_100A_192KW_2:    //phb_230105
        //case C_SKI_120V_425A_300A_200A_100A_192KW_3:    //phb_230105
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (120.0 / 6.0);
			} else {
				ratio /= 1.494;
				ratio *= (60.0 / 6.0);
			}
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
			//AMP -> AD620A
				ratio /= 1.494;
				ratio *= (120.0 / 4.5);
			break;
		case C_SKI_180V_425A_200A_100A_192KW:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_2:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_3:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_4:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_5:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_6:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_7:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_8:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_9:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_10:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_11:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_12:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_13:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_14:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_15:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_16:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_17:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_18:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_19:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_20:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_21:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_22:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_23:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_24:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_25:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_26:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_27:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_28:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_29:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_30:	//sec_221221
		case C_SKI_180V_600A_300A_100A_320KW:       //phb_230220
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (180.0 / 4.5);
			} else {
				ratio /= 1.494;
				ratio *= (90.0 / 4.5);
			}
			break;
		case C_NORTHVOLT_200V_400A_200A_160KW:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_2:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_3:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_4:	//ktg_191203
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (200.0 / 5.0);
			break;
		case C_SKI_260V_425A_200A_442KW:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_2:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_3:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_4:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_5:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_6:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_7:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_8:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_9:	//shh_211020
			//AMP -> AD620A
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (260.0 / 4.875);
			} else {
				ratio /= 1.494;
				ratio *= (130.0 / 4.875);
			}
			break;
		case C_SKI_400V_100A_10A_40KW: //jhk_120112
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (400.0 / 5.0);
			break;
		case C_NORTHVOLT_400V_200A_100A_160KW:		//ktg_200415
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (400.0 / 5.0);
			break;
		case C_SK_450V_200A_10A_180KW:
		case C_SK_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_SK_450V_200A_10A_360KW:
			//AMP -> AD620A
			ratio /= 1.494;
			ratio *= (450.0 / 5.0);
			break;
		case C_SKI_450V_200A_100A_90KW:			//sec_221223
		case C_SKI_450V_150A_100A_50A_260KW:	//sec_221215
		case C_SKI_450V_150A_100A_50A_260KW_2:	//sec_221215
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (450.0 / 4.5);
			} else {
				ratio /= 1.494;
				ratio *= (225.0 / 4.5);
			}
			break;
		case C_SKI_500V_450A_200A_450KW:		//khj_200308
		case C_SKI_500V_450A_200A_450KW_2:		//khj_200308
		case C_SKI_500V_450A_200A_450KW_3:		//khj_200308
		//case C_SKI_500V_450A_200A_450KW_4:	//ktg_200410 //shh_240730s
		//case C_SKI_500V_450A_200A_450KW_5:	//ktg_200410
		//case C_SKI_500V_450A_200A_450KW_6:	//ktg_200410 //shh_240730e
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (500.0 / 5.0);
			break;	
		case C_SKC_600V_200A_100A_50A_240KW: //jhkw_120807
		case C_SKE_600V_400A_100A_50A_25A_240KW:	//jhkw_130924
		case C_DAEHWA_600V_400A_200A_100A_200KW:	//jhk_170120
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (600.0 / 6.0);
			break;
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
		case C_DAEHWA_800V_200A_50A_160KW:	//jhk_160831
		
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (800.0 / 6.0);
			break;
		case C_SKI_1000V_400A_200A_100A_450KW:	//sec_230103
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (1000.0 / 6.4);
			} else {
				ratio /= 1.494;
				ratio *= (500.0 / 6.4);	
			}
			break;
		case C_SKI_1200V_250A_100A_50A_300KW:	//jhk_131209
		//case C_SKI_1200V_500A_400A_300A_200A_600KW:     //phb_230116
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (1200.0 / 6.0);
			} else {
				ratio /= 1.494;
				ratio *= (600.0 / 6.0);	
			}
			break;
		case C_SKI_1500V_400A_100A_500KW:	//jhk_180610
			//AMP -> OP270
			ratio /= 1.494;
			ratio *= (1500.0 / 6.0);
			break;
		case C_PNE_2000V_300A_300KW:	//jhk_200313
		case C_PNE_2000V_300A_600KW:	//jhk_200313
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (2000.0 / 6.0);
			} else if((range+1) == RANGE2) {
				ratio /= 1.494;
				ratio *= (1000.0 / 6.0);
			} else {
				ratio /= 1.494;
				ratio *= (500.0 / 6.0);	
			}
			break;
		case C_SKI_2000V_300A_200A_100A_50A_600KW:  //phb_230320
			//AMP -> OP270
			if((range+1) == RANGE1) {
				ratio /= 1.494;
				ratio *= (2000.0 / 6.0);
			} else {
				ratio /= 1.494;
				ratio *= (1000.0 / 6.0);
			}
			break;
		default:
			//AMP -> AD620A
			//ratio /= 1.494;
			main_amp = myPs->config.main_amp; //1.494	//jhkw_231127s
			maxV = myPs->config.maxV[range];
			da_max = myPs->config.da_max;	//10000000uV = 10000mV = 10V
			v_feed = myPs->config.v_feed[range];

			if(main_amp <= 0.0 || maxV <= 0 || da_max <= 0.0 || v_feed <= 0.0) {
				ratio = 0.0;
			} else {
				ratio = da_max / 32768.0 / main_amp;
				ratio *= ((double)maxV / v_feed);
			}	//jhkw_231127
			break;
	}

	return ratio;
}

double Ref_I_Ratio(int range)
{
	/*Simplication - shh_250417*/
	double ratio;
	long maxI; //jhkw_231127
	double da_max, i_feed, main_amp;	//jhkw_231127

	switch(myData->AppControl.config.systemModel) {
		case C_SKI_20V_300A_200A_100A_12KW:		//shh_231204
		case C_SKI_20V_300A_200A_100A_12KW_2:	//shh_231204
			//10000000uV / 32768 = 305.1757812uV
            //AMP -> AD620A
            //G1 = 1 + 49.4Kohm / 7.6Kohm = 7.5
            //G2 = 1 + 49.4Kohm / 100Kohm = 1.494

            //RANGE1
            //ITN600-S 1500 : 1 = 300A : 400.000mA
            //shuntV = 200.000mA * 4.0ohm = 800.000mV
            //V1 = 800.000mV * G1 = 6000mV
            //V2 = V1 * G2 = 8964mV
            //300000mA : 8964 = I1 : 10000mV
           //I1 = 334672.0214190094mA
            //ratio = 334672021.4190094uA / 32768 = 10213.37955990

            //RANGE2
            //ITN600-S 1500 : 1 = 200A : 133.333mA
            //shuntV = 133.333mA * 6.0ohm = 800.000mV
            //V1 = 800.000mV * G1 = 6000mV
            //V2 = V1 * G2 = 8964mV
            //200000mA : 8964 = I1 : 10000mV
            //I1 = 223114.6809460062mA
            //ratio = 223114680.9460062uA / 32768 = 6808.91970660

            //RANGE3
            //ITN600-S 1500 : 1 = 100A : 66.667mA
            //shuntV = 66.667mA * 12.0ohm = 800.000mV
            //V1 = 800.000mV * G1 = 6000mV
            //V2 = V1 * G2 = 8964mV
            //100000mA : 8964 = I1 : 10000mV
            //I1 = 111557.3404730031mA
            //ratio = 111557340.4730031uA / 32768 = 3404.45985330

            if((range+1) == RANGE1) ratio = 10213.37955990;
            else if((range+1) == RANGE2) ratio = 6808.91970660;
            else ratio = 3404.45985330;
            break;
		//Current Transducer used
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
		case C_SKI_100V_100A_10A_10KW:  //jhk_120329
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
		case C_SKI_120V_400A_200A_100A_192KW_85:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_86:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_87:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_88:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_89:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_90:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_91:	//khj_200308
		////////////////////////////////////////////////////////
		case C_SKI_120V_400A_200A_100A_192KW_92:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_93:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_94:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_95:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_96:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_97:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_98:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_99:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_100:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_101:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_102:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_103:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_104:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_105:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_106:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_107:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_108:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_109:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_110:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_111:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_112:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_113:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_114:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_115:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_116:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_117:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_118:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_119:	//khj_200308
		////////////////////////////////////////////////////////
		//case C_SKI_120V_400A_200A_100A_192KW_169:	//ktg_200410 //shh_240730
		case C_SKI_120V_400A_200A_100A_192KW_170:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_171:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_172:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_173:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_174:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_175:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_176:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_177:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_178:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_179:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_180:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_181:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_182:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_183:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_184:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_185:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_186:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_187:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_188:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_189:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_190:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_191:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_192:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_193:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_194:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_195:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_196:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_197:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_198:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_199:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_200:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_201:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_202:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_203:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_204:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_205:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_206:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_207:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_208:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_209:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_210:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_211:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_212:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_213:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_214:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_215:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_216:	//shh_200905
		//case C_SKI_120V_400A_200A_100A_192KW_232:	//shh_200914 //shh_240730
		case C_SKI_120V_400A_200A_100A_192KW_233:	//ktg_200908
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
		case C_SKI_120V_400A_200A_100A_192KW_234:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_235:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_236:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_237:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_238:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_239:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_240:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_241:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_242:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_243:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_244:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_245:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_246:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_247:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_248:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_249:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_250:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_251:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_252:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_253:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_254:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_255:	//shh_220113
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
			//I1 = 55979.31293520614mA
			//ratio = 55979312.93520614uA / 32768 = 1708.353055883
			
			if((range+1) == RANGE1) ratio = 13666.82444706885;
			else if ((range+1) == RANGE2) ratio = 6833.412223534425;
			else if ((range+1) == RANGE3) ratio = 3416.7061117672127;
			else ratio = 1708.353055883;
			break;
		case C_SKI_120V_425A_200A_100A_50A_192KW:       //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_2:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_3:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_4:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_5:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_6:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_7:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_8:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_9:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_10:    //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_11:    //phb_230206
			//10000000uV / 32768 = 305.1757812uV
            //AMP -> AD620A
            //G1 = 1 + 49.4Kohm / 6.2Kohm = 8.968
            //G2 = 1 + 49.4Kohm / 100Kohm = 1.494

            //RANGE1
            //ITN600-S 1500 : 1 = 425A : 283.333mA
            //shuntV = 283.333mA * 2.5ohm = 666.000mV
            //V1 = 666.000mV * G1 = 5972.516mV
            //V2 = V1 * G2 = 8922.939mV
            //425000mA : 8922.939 = I1 : 10000mV
            //I1 = 476300.46040956mA
            //ratio = 476300460.40955859uA / 32768 = 14535.53651152

            //RANGE2
            //ITN600-S 1500 : 1 = 200A : 133.333mA
            //shuntV = 133.333mA * 5.0ohm = 666.000mV
            //V1 = 666.000mV * G1 = 5972.516mV
            //V2 = V1 * G2 = 8922.939mV
            //200000mA : 8922.939 = I1 : 10000mV
            //I1 = 224141.39313391mA
            //ratio = 224141393.13390994uA / 32768 = 6840.25247601

            //RANGE3
            //ITN600-S 1500 : 1 = 100A : 66.667mA
            //shuntV = 66.667mA * 10.0ohm = 666.000mV
            //V1 = 666.000mV * G1 = 5972.516mV
            //V2 = V1 * G2 = 8922.939mV
            //100000mA : 8922.939 = I1 : 10000mV
            //I1 = 112070.69656695mA
            //ratio = 112070696.56695497uA / 32768 = 3420.12623801

            //RANGE4
            //ITN600-S 1500 : 1 = 50A : 33.333mA
            //shuntV = 33.333mA * 20.0ohm = 666.000mV
            //V1 = 666.000mV * G1 = 5972.516mV
            //V2 = V1 * G2 = 8922.939mV
            //50000mA : 8922.939 = I1 : 10000mV
            //I1 = 56035.34828348mA
            //ratio = 56035348.28347749uA / 32768 = 1710.06311900

            if((range+1) == RANGE1) ratio = 14535.53651152;
            else if((range+1) == RANGE2) ratio = 6840.25247601;
            else if((range+1) == RANGE3) ratio = 3420.12623801;
            else ratio = 1710.06311900;
            break;
		/*
		case C_SKI_120V_425A_300A_200A_100A_192KW:      //phb_230105
        case C_SKI_120V_425A_300A_200A_100A_192KW_2:    //phb_230105
        case C_SKI_120V_425A_300A_200A_100A_192KW_3:    //phb_230105
 			//10000000uV / 32768 = 305.1757812uV
            //AMP -> AD620A
            //G1 = 1 + 49.4Kohm / 10.0Kohm = 5.940
            //G2 = 1 + 49.4Kohm / 100Kohm = 1.494

            //RANGE1
            //ITN600 1500 : 1 = 425A : 283.333mA
            //shuntV = 283.333mA * 3.5ohm = 999.000mV
            //V1 = 999.000mV * G1 = 5934.060mV
            //V2 = V1 * G2 = 8865.486mV
            //425000mA : 8865.486 = I1 : 10000mV
            //I1 = 479387.16191976mA
            //ratio = 479387161.91976142uA / 32768 = 14629.73516601

            //RANGE2
            //ITN600 1500 : 1 = 300A : 200.000mA
            //shuntV = 200.000mA * 5.0ohm = 999.000mV
            //V1 = 999.000mV * G1 = 5934.060mV
            //V2 = V1 * G2 = 8865.486mV
            //300000mA : 8865.486 = I1 : 10000mV
            //I1 = 338390.93782571mA
            //ratio = 338390937.82571393uA / 32768 = 10326.87188189

            //RANGE3
            //ITN600 1500 : 1 = 200A : 133.333mA
            //shuntV = 133.333mA * 7.5ohm = 999.000mV
            //V1 = 999.000mV * G1 = 5934.060mV
            //V2 = V1 * G2 = 8865.486mV
            //200000mA : 8865.486 = I1 : 10000mV
            //I1 = 225593.95855048mA
            //ratio = 225593958.55047596uA / 32768 = 6884.58125459

            //RANGE4
            //ITN600 1500 : 1 = 100A : 66.667mA
            //shuntV = 66.667mA * 15.0ohm = 999.000mV
            //V1 = 999.000mV * G1 = 5934.060mV
            //V2 = V1 * G2 = 8865.486mV
            //100000mA : 8865.486 = I1 : 10000mV
            //I1 = 112796.97927524mA
            //ratio = 112796979.27523798uA / 32768 = 3442.29062730

            if((range+1) == RANGE1) ratio = 14629.73516601;
            else if((range+1) == RANGE2) ratio = 10326.87188189;
            else if((range+1) == RANGE3) ratio = 6884.58125459;
            else ratio = 3442.29062730;
            break;
		*/
		case C_SKI_180V_425A_200A_100A_192KW:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_2:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_3:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_4:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_5:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_6:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_7:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_8:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_9:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_10:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_11:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_12:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_13:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_14:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_15:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_16:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_17:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_18:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_19:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_20:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_21:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_22:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_23:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_24:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_25:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_26:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_27:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_28:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_29:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_30:	//sec_221221
           //10000000uV / 32768 = 305.1757812uV
           //AMP -> AD620A
           //G1 = 1 + 49.4Kohm / 6.2Kohm = 8.968
           //G2 = 1 + 49.4Kohm / 100Kohm = 1.494

           //RANGE1
           //ITN600 1500 : 1 = 425A : 283.333mA
           //shuntV = 283.333mA * 2.4ohm = 666.000mV
           //V1 = 666.000mV * G1 = 5972.516mV
           //V2 = V1 * G2 = 8922.939mV
           //425000mA : 8922.939 = I1 : 10000mV
           //I1 = 476300.46040956mA
           //ratio = 476300460.40955859uA / 32768 = 14535.53651152

           //RANGE2
           //ITN600 1500 : 1 = 200A : 133.333mA
           //shuntV = 133.333mA * 5.0ohm = 666.000mV
           //V1 = 666.000mV * G1 = 5972.516mV
           //V2 = V1 * G2 = 8922.939mV
           //200000mA : 8922.939 = I1 : 10000mV
           //I1 = 224141.39313391mA
           //ratio = 224141393.13390994uA / 32768 = 6840.25247601

           //RANGE3
           //ITN600 1500 : 1 = 100A : 66.667mA
           //shuntV = 66.667mA * 10.0ohm = 666.000mV
           //V1 = 666.000mV * G1 = 5972.516mV
           //V2 = V1 * G2 = 8922.939mV
           //100000mA : 8922.939 = I1 : 10000mV
           //I1 = 112070.69656695mA
           //ratio = 112070696.56695497uA / 32768 = 3420.12623801

           //RANGE4
           //ITN600 1500 : 1 = 50A : 33.333mA
           //shuntV = 33.333mA * 20.0ohm = 666.000mV
           //V1 = 666.000mV * G1 = 5972.516mV
           //V2 = V1 * G2 = 8922.939mV
           //50000mA : 8922.939 = I1 : 10000mV
           //I1 = 56035.34828348mA
           //ratio = 56035348.28347749uA / 32768 = 1710.06311900
     	   if((range+1) == RANGE1) ratio = 14535.53651152;
           else if((range+1) == RANGE2) ratio = 6840.25247601;
           else if((range+1) == RANGE3) ratio = 3420.12623801;
           else ratio = 1710.06311900;
           break;
		case C_SKI_180V_600A_300A_100A_320KW:       //phb_230220
		//case C_SKI_180V_600A_300A_100A_320KW_2:		//shh_231004
			//10000000uV / 32768 = 305.1757812uV
            //AMP -> AD620A
            //G1 = 1 + 49.4Kohm / 7.7Kohm = 7.416
            //G2 = 1 + 49.4Kohm / 100Kohm = 1.494

            //RANGE1
            //ITN600-S 1500 : 1 = 600A : 400.000mA
            //shuntV = 400.000mA * 2.0ohm = 800.000mV
            //V1 = 800.000mV * G1 = 5932.468mV
            //V2 = V1 * G2 = 8863.106mV
            //600000mA : 8863.106 = I1 : 10000mV
            //I1 = 676963.54595264mA
            //ratio = 676963545.95263720uA / 32768 = 20659.28790139

            //RANGE2
            //ITN600-S 1500 : 1 = 300A : 200.000mA
            //shuntV = 200.000mA * 4.0ohm = 800.000mV
            //V1 = 800.000mV * G1 = 5932.468mV
            //V2 = V1 * G2 = 8863.106mV
            //300000mA : 8863.106 = I1 : 10000mV
            //I1 = 338481.77297632mA
            //ratio = 338481772.97631860uA / 32768 = 10329.64395069

            //RANGE3
            //ITN600-S 1500 : 1 = 100A : 66.667mA
            //shuntV = 66.667mA * 12.0ohm = 800.000mV
            //V1 = 800.000mV * G1 = 5932.468mV
            //V2 = V1 * G2 = 8863.106mV
            //100000mA : 8863.106 = I1 : 10000mV
            //I1 = 112827.25765877mA
            //ratio = 112827257.65877287uA / 32768 = 3443.21465023

            if((range+1) == RANGE1) ratio = 20659.28790139;
            else if((range+1) == RANGE2) ratio = 10329.64395069;
            else ratio = 3443.21465023;
            break;
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
		case C_SKI_260V_425A_200A_442KW:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_2:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_3:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_4:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_5:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_6:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_7:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_8:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_9:	//shh_211020
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.17kohm = 9.006483
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 400A:x -> x = 266.666mA
			//shuntV = 266.666mA * 2.5ohm = 666.7mV
			//V1 = 666.7mV * G1 = 6004.32198811mV
			//V2 = V1 * G2 = 8970.45705024mV
			//400000mA : 8970.45705024mV = I1 : 10000mV
			//I1 = 445908.1602640282mA
			//ratio = 4459081602.640282uA / 32768 = 13608.03711743
			
			//RANGE2
			//ITN-600 600A:400mA = 200A:x -> x = 133.33mA
			//shuntV = 133.33mA * 5.0ohm = 666.7mV
			//V1 = 666.7mV * G1 = 6004.32198811mV
			//V2 = V1 * G2 = 8970.45705024mV
			//200000mA : 8970.45705024mV = I1 : 10000mV
			//I1 = 222954.0801320141mA
			//ratio = 222954080.1320141uA / 32768 = 6804.018558716
			
			//RANGE3
			//ITN-600 600A:400mA = 100A:x -> x = 66.6667mA
			//shuntV = 66.6667mA * 10ohm = 666.7mV
			//V1 = 666.7mV * G1 = 6004.32198811mV
			//V2 = V1 * G2 = 8970.45705024mV
			//100000mA : 8970.45705024mV = I1 : 10000mV
			//I1 = 111477.0400660071mA
			//ratio = 111477040.0660071uA / 32768 = 3402.00927935
			
			if((range+1) == RANGE1) ratio = 13608.03711743;
			else if ((range+1) == RANGE2) ratio = 6804.008558716;
			else ratio = 3402.00927935;
			break;
		case C_SKI_400V_100A_10A_40KW: //jhk_120112
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
		
		case C_NORTHVOLT_400V_200A_100A_160KW:		//ktg_200415
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677419
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 200A:x -> x = 133.33mA
			//shuntV = 133.3mA * 5ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//200000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 223917.251740776mA
			//ratio =223917251.740776uA / 32768 = 6833.412223534425
			
			//RANGE2
			//ITN-600 600A:400mA = 100A:x -> x = 66.7mA
			//shuntV = 66.7mA * 10ohm = 667mV
			//V1 = 667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.8709324mV
			//100000mA : 8931.8709324mV = I1 : 10000mV
			//I1 = 111958.6263133898mA
			//ratio = 111958626.3133898uA / 32768 = 3416.706125286554
			
			if((range+1) == RANGE1) ratio = 6833.41222353;
			else ratio = 3416.70612529;
			break;
		case C_SK_450V_200A_10A_180KW:
		case C_SK_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_SK_450V_200A_10A_360KW:
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
		case C_SKI_450V_200A_100A_90KW:			//sec_221223
           //10000000uV / 32768 = 305.1757812uV
           //AMP -> AD620A
           //G1 = 1 + 49.4Kohm / 6.2Kohm = 8.968
           //G2 = 1 + 49.4Kohm / 100Kohm = 1.494

           //RANGE1
           //ITN600-S 1500 : 1 = 200A : 133.333mA
           //shuntV = 133.333mA * 5.0ohm = 666.000mV
           //V1 = 666.000mV * G1 = 5972.516mV
           //V2 = V1 * G2 = 8922.939mV
           //200000mA : 8922.939 = I1 : 10000mV
           //I1 = 224141.39313391mA
           //ratio = 224141393.13390994uA / 32768 = 6840.25247601

           //RANGE2
           //ITN600-S 1500 : 1 = 100A : 66.667mA
           //shuntV = 66.667mA * 10.0ohm = 666.000mV
           //V1 = 666.000mV * G1 = 5972.516mV
           //V2 = V1 * G2 = 8922.939mV
           //100000mA : 8922.939 = I1 : 10000mV
           //I1 = 112070.69656695mA
           //ratio = 112070696.56695497uA / 32768 = 3420.12623801
           if((range+1) == RANGE1) ratio = 6840.25247601;
           else ratio = 3420.12623801;
           break;
		case C_SKI_450V_150A_100A_50A_260KW:	//sec_221215
		case C_SKI_450V_150A_100A_50A_260KW_2:	//sec_221215
           //10000000uV / 32768 = 305.1757812uV
           //AMP -> AD620A
           //G1 = 1 + 49.4Kohm / 10.0Kohm = 5.940
           //G2 = 1 + 49.4Kohm / 100Kohm = 1.494

           //RANGE1
           //ITN600-S 1500 : 1 = 150A : 100.000mA
           //shuntV = 100.000mA * 10.0ohm = 999.000mV
           //V1 = 999.000mV * G1 = 5934.060mV
           //V2 = V1 * G2 = 8865.486mV
           //150000mA : 8865.486 = I1 : 10000mV
           //I1 = 169195.46891286mA
           //ratio = 169195468.91285697uA / 32768 = 5163.43594094

           //RANGE2
           //ITN600-S 1500 : 1 = 100A : 66.667mA
           //shuntV = 66.667mA * 15.0ohm = 999.000mV
           //V1 = 999.000mV * G1 = 5934.060mV
           //V2 = V1 * G2 = 8865.486mV
           //100000mA : 8865.486 = I1 : 10000mV
           //I1 = 112796.97927524mA
           //ratio = 112796979.27523798uA / 32768 = 3442.29062730

           //RANGE3
           //ITN600-S 1500 : 1 = 50A : 33.333mA
           //shuntV = 33.333mA * 30.0ohm = 999.000mV
           //V1 = 999.000mV * G1 = 5934.060mV
           //V2 = V1 * G2 = 8865.486mV
           //50000mA : 8865.486 = I1 : 10000mV
           //I1 = 56398.48963762mA
           //ratio = 56398489.63761899uA / 32768 = 1721.14531365
           if((range+1) == RANGE1) ratio = 5163.43594094;
           else if((range+1) == RANGE2) ratio = 3442.29062730;
           else ratio = 1721.14531365;
           break;
		case C_SKI_500V_450A_200A_450KW:		//khj_200308
		case C_SKI_500V_450A_200A_450KW_2:		//khj_200308
		case C_SKI_500V_450A_200A_450KW_3:		//khj_200308
		//case C_SKI_500V_450A_200A_450KW_4:	//ktg_200410  //shh_240730s
		//case C_SKI_500V_450A_200A_450KW_5:	//ktg_200410
		//case C_SKI_500V_450A_200A_450KW_6:	//ktg_200410  //shh_240730e
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.9677419
			//AMP -> INA121
			//G2 = 1 + 49.4kohm / 100kohm = 1.494
			
			//RANGE1
			//ITN-600 600A:400mA = 250A:x -> x = 166.67mA
			//shuntV = 166.6mA * 4ohm = 666.7mV
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//V2 = V1 * G2 = 8931.87096774mV
			//250000mA : 8931.87096774mV = I1 : 10000mV
			//I1 = 279896.5646760307mA
			//ratio =279896564.6760307uA / 32768 = 8541.765279419883
			
			//RANGE2
			//ITN-600 600A:400mA = 100A:x -> x = 66.7mA
			//shuntV = 66.7mA * 10ohm = 667mV
			//V1 = 667mV * G1 = 5978.4946mV
			//V2 = V1 * G2 = 8931.8709324mV
			//100000mA : 8931.8709324mV = I1 : 10000mV
			//I1 = 111958.6263133898mA
			//ratio = 111958626.3133898uA / 32768 = 3416.706125286554
			
			if((range+1) == RANGE1) ratio = 8541.765279419883;
			else ratio = 3416.70612528655;
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
		case C_DAEHWA_750V_300A_100A_150KW:	//jhk_170203
	
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
		case C_SKI_1000V_400A_200A_100A_450KW:	//sec_230103
           //10000000uV / 32768 = 305.1757812uV
           //AMP -> AD620A
           //G1 = 1 + 49.4Kohm / 6.2Kohm = 9.006
           //G2 = 1 + 49.4Kohm / 100Kohm = 1.494

           //RANGE1
           //ITN600-S 1500 : 1 = 400A : 266.667mA
           //shuntV = 266.667mA * 2.5ohm = 666.666mV
           //V1 = 666.666mV * G1 = 6004.316mV
           //V2 = V1 * G2 = 8970.448mV
           //400000mA : 8970.448 = I1 : 10000mV
           //I1 = 445908.60617248mA
           //ratio = 445908606.17247975uA / 32768 = 13608.05072548

           //RANGE2
           //ITN600-S 1500 : 1 = 200A : 133.333mA
           //shuntV = 133.333mA * 5.0ohm = 666.666mV
           //V1 = 666.666mV * G1 = 6004.316mV
           //V2 = V1 * G2 = 8970.448mV
           //200000mA : 8970.448 = I1 : 10000mV
           //I1 = 222954.30308624mA
           //ratio = 222954303.08623987uA / 32768 = 6804.02536274

           //RANGE3
           //ITN600-S 1500 : 1 = 100A : 66.667mA
           //shuntV = 66.667mA * 10.0ohm = 666.666mV
           //V1 = 666.666mV * G1 = 6004.316mV
           //V2 = V1 * G2 = 8970.448mV
           //100000mA : 8970.448 = I1 : 10000mV
           //I1 = 111477.15154312mA
           //ratio = 111477151.54311994uA / 32768 = 3402.01268137
           if((range+1) == RANGE1) ratio = 13608.05072548;
           else if((range+1) == RANGE2) ratio = 6804.02536274;
           else ratio = 3402.01268137;
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
		/*
		case C_SKI_1200V_500A_400A_300A_200A_600KW:     //phb_230116
			//10000000uV / 32768 = 305.1757812uV
            //AMP -> AD620A
            //G1 = 1 + 49.4Kohm / 7.6Kohm = 7.500
            //G2 = 1 + 49.4Kohm / 100Kohm = 1.494

            //RANGE1
            //ITN600-S 1500 : 1 = 500A : 333.333mA
            //shuntV = 333.333mA * 2.4ohm = 800.000mV
            //V1 = 800.000mV * G1 = 6000.000mV
            //V2 = V1 * G2 = 8964.000mV
            //500000mA : 8964.000 = I1 : 10000mV
            //I1 = 557786.70236502mA
            //ratio = 557786702.36501563uA / 32768 = 17022.29926651

            //RANGE2
            //ITN600-S 1500 : 1 = 400A : 266.667mA
            //shuntV = 266.667mA * 3.0ohm = 800.000mV
            //V1 = 800.000mV * G1 = 6000.000mV
            //V2 = V1 * G2 = 8964.000mV
            //400000mA : 8964.000 = I1 : 10000mV
            //I1 = 446229.36189201mA
            //ratio = 446229361.89201248uA / 32768 = 13617.83941321

            //RANGE3
            //ITN600-S 1500 : 1 = 300A : 200.000mA
            //shuntV = 200.000mA * 4.0ohm = 800.000mV
            //V1 = 800.000mV * G1 = 6000.000mV
            //V2 = V1 * G2 = 8964.000mV
            //300000mA : 8964.000 = I1 : 10000mV
            //I1 = 334672.02141901mA
            //ratio = 334672021.41900939uA / 32768 = 10213.37955991

            //RANGE4
            //ITN600-S 1500 : 1 = 200A : 133.333mA
            //shuntV = 133.333mA * 6.0ohm = 800.000mV
            //V1 = 800.000mV * G1 = 6000.000mV
            //V2 = V1 * G2 = 8964.000mV
            //200000mA : 8964.000 = I1 : 10000mV
            //I1 = 223114.68094601mA
            //ratio = 223114680.94600624uA / 32768 = 6808.91970660

            if((range+1) == RANGE1) ratio = 17022.29926651;
            else if((range+1) == RANGE2) ratio = 13617.83941321;
            else if((range+1) == RANGE3) ratio = 10213.37955991;
            else ratio = 6808.91970660;
            break;
		*/
		/* 
		case C_SKI_1500V_350A_300A_200A_100A_450KW:     //phb_230117
			//10000000uV / 32768 = 305.1757812uV
            //AMP -> AD620A
            //G1 = 1 + 49.4Kohm / 7.6Kohm = 7.500
            //G2 = 1 + 49.4Kohm / 100Kohm = 1.494

            //RANGE1
            //ITN600-S 1500 : 1 = 350A : 233.333mA
            //shuntV = 233.333mA * 3.4ohm = 800.000mV
            //V1 = 800.000mV * G1 = 6000.000mV
            //V2 = V1 * G2 = 8964.000mV
            //350000mA : 8964.000 = I1 : 10000mV
            //I1 = 390450.69165551mA
            //ratio = 390450691.65551090uA / 32768 = 11915.60948656

            //RANGE2
            //ITN600-S 1500 : 1 = 300A : 200.000mA
            //shuntV = 200.000mA * 4.0ohm = 800.000mV
            //V1 = 800.000mV * G1 = 6000.000mV
            //V2 = V1 * G2 = 8964.000mV
            //300000mA : 8964.000 = I1 : 10000mV
            //I1 = 334672.02141901mA
            //ratio = 334672021.41900939uA / 32768 = 10213.37955991

            //RANGE3
            //ITN600-S 1500 : 1 = 200A : 133.333mA
            //shuntV = 133.333mA * 6.0ohm = 800.000mV
            //V1 = 800.000mV * G1 = 6000.000mV
            //V2 = V1 * G2 = 8964.000mV
            //200000mA : 8964.000 = I1 : 10000mV
            //I1 = 223114.68094601mA
            //ratio = 223114680.94600624uA / 32768 = 6808.91970660

            //RANGE4
            //ITN600-S 1500 : 1 = 100A : 66.667mA
            //shuntV = 66.667mA * 12.0ohm = 800.000mV
            //V1 = 800.000mV * G1 = 6000.000mV
            //V2 = V1 * G2 = 8964.000mV
            //100000mA : 8964.000 = I1 : 10000mV
            //I1 = 111557.34047300mA
            //ratio = 111557340.47300312uA / 32768 = 3404.45985330

            if((range+1) == RANGE1) ratio = 11915.60948656;
            else if((range+1) == RANGE2) ratio = 10213.37955991;
            else if((range+1) == RANGE3) ratio = 6808.91970660;
            else ratio = 3404.45985330;
            break;
		*/
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
		case C_PNE_2000V_300A_300KW:	//jhk_200313
		case C_PNE_2000V_300A_600KW:	//jhk_200313
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.17kohm = 9.00648298
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600s 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 3.33ohm = 666.666mV
			//V1 = 666.666mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.457050243mV
			//300000mA : 8970.457050243mV = I1 : 10000mV
			//I1 = 334431.1201979mA
			//ratio = 334431120.1979uA / 32768 = 10206.027838070836
			
			//RANGE2
			//ITN600s 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 5ohm = 666.666mV
			//V1 = 666.666mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.457050243mV
			//200000mA : 8970.457050243mV = I1 : 10000mV
			//I1 = 222954.08013mA
			//ratio = 222954080.1319uA / 32768 = 6804.018558713891
			
			//RANGE3
			//ITN600s 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = 666.666mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.457050243mV
			//100000mA : 8970.457050243mV = I1 : 10000mV
			//I1 = 111477.040065968mA
			//ratio = 111477040.06uA / 32768 = 3402.0092793569455
			
			//RANGE4
			//ITN600s 600A:400mA = 50A:x -> x = 33.333mA
			//shuntV = 33.333mA * 20ohm = 666.667mV
			//V1 = 666.666mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.457050243mV
			//50000mA : 8970.457050243mV = I1 : 10000mV
			//I1 = 55738.520032984mA
			//ratio = 55738520.032984uA / 32768 = 1701.004639678473
			
			if((range+1) == RANGE1) ratio = 10206.027838070836;
			else if((range+1) == RANGE2) ratio = 6804.018558713891;
			else if((range+1) == RANGE3) ratio = 3402.0092793569455;
			else ratio = 1701.004639678473;
			break;
		case C_SKI_2000V_300A_200A_100A_50A_600KW:  //phb_230320
			//10000000uV / 32768 = 305.1757813uV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.17kohm = 9.00648298
			//AMP -> AD620A
			//G2 = 1 + 49.4kohm / 100kohm = 1.494

			//RANGE1
			//ITN600s 600A:400mA = 300A:x -> x = 200mA
			//shuntV = 200mA * 3.33ohm = 666.666mV
			//V1 = 666.666mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.457050243mV
			//300000mA : 8970.457050243mV = I1 : 10000mV
			//I1 = 334431.1201979mA
			//ratio = 334431120.1979uA / 32768 = 10206.027838070836
			
			//RANGE2
			//ITN600s 600A:400mA = 200A:x -> x = 133.333mA
			//shuntV = 133.333mA * 5ohm = 666.666mV
			//V1 = 666.666mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.457050243mV
			//200000mA : 8970.457050243mV = I1 : 10000mV
			//I1 = 222954.08013mA
			//ratio = 222954080.1319uA / 32768 = 6804.018558713891
			
			//RANGE3
			//ITN600s 600A:400mA = 100A:x -> x = 66.667mA
			//shuntV = 66.667mA * 10ohm = 666.667mV
			//V1 = 666.666mV * G1 = 6004.321988mV
			//V2 = V1 * G2 = 8970.457050243mV
			//100000mA : 8970.457050243mV = I1 : 10000mV
			//I1 = 111477.040065968mA
			//ratio = 111477040.06uA / 32768 = 3402.0092793569455
			
			if((range+1) == RANGE1) ratio = 10206.027838070836;
			else if((range+1) == RANGE2) ratio = 6804.018558713891;
			else ratio = 3402.0092793569455;
			break;
		default:
			//AMP -> INA128UA
			//ratio = 1.0;
			main_amp = myPs->config.main_amp;	//1.494	//jhkw_231127s
			maxI = myPs->config.maxI[range];
			da_max = myPs->config.da_max;
			i_feed = myPs->config.i_feed[range];
	
			if(main_amp <= 0.0 || maxI <= 0 || da_max <= 0.0 || i_feed <= 0.0) {
				ratio = 0.0;
			} else {
				ratio = i_feed * main_amp;
				ratio = (double)maxI * (da_max / ratio);
				ratio /= 32768;
			}	//jhkw_231127e
			break;
	}

	return ratio;
}

double Ref_V_Value(void)
{
	/*Simplication - shh_250417*/
	double ret_val;
	long maxV;  //jhkw_231127
	double v_feed, ref;	//jhkw_231127

	switch(myData->AppControl.config.systemModel) {
		case C_SKI_50V_500A_400A_200A_100A_100KW:	//jhk_180119
		case C_SKI_50V_500A_400A_200A_100A_100KW_2:	//jhk_180119
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
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
		    ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
		    break;
		case C_SKI_100V_300A_150A_50A_10A_60KW:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_2:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_3:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_4:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_5:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_6:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_7:	//kjhw_120531
		case C_SKI_100V_300A_150A_50A_10A_60KW_8:	//kjhw_120531
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_SKI_120V_400A_100A_192KW:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_2:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_3:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_4:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_5:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_6:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_7:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_8:	//jhk_170628
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
		case C_SKI_120V_400A_200A_100A_192KW_85:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_86:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_87:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_88:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_89:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_90:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_91:	//khj_200308
		////////////////////////////////////////////////////////
		case C_SKI_120V_400A_200A_100A_192KW_92:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_93:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_94:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_95:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_96:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_97:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_98:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_99:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_100:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_101:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_102:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_103:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_104:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_105:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_106:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_107:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_108:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_109:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_110:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_111:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_112:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_113:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_114:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_115:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_116:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_117:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_118:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_119:	//khj_200308
		////////////////////////////////////////////////////////
		//case C_SKI_120V_400A_200A_100A_192KW_169:	//ktg_200410 //shh_240730
		//case C_TEST_120V_400A_200A_100A_192KW:		//shh_200723
		case C_SKI_120V_400A_200A_100A_192KW_170:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_171:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_172:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_173:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_174:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_175:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_176:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_177:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_178:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_179:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_180:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_181:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_182:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_183:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_184:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_185:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_186:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_187:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_188:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_189:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_190:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_191:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_192:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_193:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_194:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_195:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_196:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_197:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_198:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_199:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_200:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_201:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_202:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_203:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_204:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_205:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_206:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_207:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_208:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_209:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_210:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_211:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_212:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_213:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_214:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_215:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_216:	//shh_200905
		//case C_SKI_120V_400A_200A_100A_192KW_232:	//shh_200914 //shh_240730
		case C_SKI_120V_400A_200A_100A_192KW_233:	//ktg_200908
		case C_SKI_120V_400A_200A_100A_192KW_234:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_235:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_236:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_237:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_238:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_239:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_240:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_241:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_242:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_243:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_244:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_245:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_246:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_247:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_248:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_249:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_250:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_251:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_252:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_253:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_254:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_255:	//shh_220113
		case C_SKI_120V_425A_200A_100A_50A_192KW:       //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_2:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_3:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_4:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_5:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_6:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_7:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_8:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_9:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_10:    //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_11:    //phb_230206
		//case C_SKI_120V_425A_300A_200A_100A_192KW:      //phb_230105
		//case C_SKI_120V_425A_300A_200A_100A_192KW_2:    //phb_230105
        //case C_SKI_120V_425A_300A_200A_100A_192KW_3:    //phb_230105
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 6.0;
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
		case C_SKI_180V_425A_200A_100A_192KW:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_2:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_3:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_4:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_5:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_6:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_7:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_8:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_9:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_10:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_11:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_12:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_13:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_14:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_15:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_16:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_17:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_18:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_19:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_20:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_21:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_22:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_23:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_24:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_25:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_26:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_27:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_28:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_29:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_30:	//sec_221221
		case C_SKI_180V_600A_300A_100A_320KW:       //phb_230220
		case C_NORTHVOLT_200V_400A_200A_160KW:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_2:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_3:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_4:	//ktg_191203
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_SKI_260V_425A_200A_442KW:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_2:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_3:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_4:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_5:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_6:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_7:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_8:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_9:	//shh_211020
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.875;
			break;
		case C_DAEWOO_450V_200A_50A_90KW: //kjh_150518
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_SKI_450V_200A_100A_90KW:			//sec_221223
		case C_SKI_450V_150A_100A_50A_260KW:	//sec_221215
		case C_SKI_450V_150A_100A_50A_260KW_2:	//sec_221215
			ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
			break;
		case C_SKC_600V_200A_100A_50A_240KW: //jhkw_120807
		case C_SKC_600V_200A_100A_50A_240KW_2: //jhkw_121019
		    ret_val = (double)myPs->config.maxV[0] * 5.0 / 6.0;
		    break;
		case C_SKE_600V_400A_100A_50A_25A_240KW:	//jhkw_130924
		case C_DAEHWA_600V_400A_200A_100A_200KW:	//jhk_170120
		    ret_val = (double)myPs->config.maxV[0] * 5.0 / 6.0;
		    break;
		case C_DAEHWA_750V_300A_100A_150KW:	//jhk_170203
		//case C_SKI_1500V_350A_300A_200A_100A_450KW:     //phb_230117
		    ret_val = (double)myPs->config.maxV[0] * 5.0 / 4.5;
		    break;
		case C_DAEHWA_800V_200A_50A_160KW:	//jhk_160831
		case C_SKI_1200V_250A_100A_50A_300KW:	//jhk_131209
		//case C_SKI_1200V_500A_400A_300A_200A_600KW:     //phb_230116
		case C_SKI_1500V_400A_100A_500KW:	//jhk_180610
		case C_PNE_2000V_300A_300KW:	//jhk_200313
		case C_PNE_2000V_300A_600KW:	//jhk_200313
		case C_SKI_2000V_300A_200A_100A_50A_600KW:  //phb_230320
		    ret_val = (double)myPs->config.maxV[0] * 5.0 / 6.0;
		    break;
		case C_SKI_1000V_400A_200A_100A_450KW:	//sec_230103
		    ret_val = (double)myPs->config.maxV[0] * 5.0 / 6.4;
		    break;
		default:
			//ret_val = (double)myPs->config.maxV[0];
			maxV = myPs->config.maxV[0];	//kjh_211021s
			v_feed = myPs->config.v_feed[0];
			
			if(myPs->config.ratioV == MICRO_UNIT) {	//uV
				ref = 5000000.0;
			} else if(myPs->config.ratioV == MILLI_UNIT) {	//mV
				ref = 5000.0;
			} else {	//V
				ref = 5.0;
			}

			ret_val = (double)maxV * ref / v_feed;
			break;
	}

	return ret_val;
}

double Ref_I_Value(void)
{
	/*Simplication - shh_250417*/
	double ret_val;
	long maxI; //jhkw_231127
	double i_feed, ref;	//jhkw_231127
	
	switch(myData->AppControl.config.systemModel) {
		case C_SKI_20V_300A_200A_100A_12KW:		//shh_231204
		case C_SKI_20V_300A_200A_100A_12KW_2:	//shh_231204
			//shuntV = 800.0000mV
            //AMP = AD620A
            //G1 = 1 + 49.4Kohm / 7.6Kohm = 7.5
            //V1 = shuntV * G1 = 6000.000mV
            ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.0;
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
		//Current Transducer used
		case C_SK_60V_300A_10A:
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7.0kohm = 8.057142857
			//V1 = shuntV * G1 = 6042.857143mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		case C_DAEHWA_60V_500A_250A_50A_60KW:	//jhk_160913
		case C_DAEHWA_60V_500A_250A_50A_60KW_2:	//jhk_160913
			//shuntV = 833.3mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 8.0kohm = 7.175
			//V1 = shuntV * G1 = 5979.1666666667mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9791666667;
			break;
		case C_NORTHVOLT_60V_400A_200A_96KW:		//khj_191203
			//shuntV = 666.7mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9784946236559;
			break;
		case C_SKI_100V_100A_10A_10KW:  //jhk_120329
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
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
		case C_SKI_120V_400A_200A_100A_192KW_85:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_86:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_87:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_88:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_89:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_90:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_91:	//khj_200308
		////////////////////////////////////////////////////////
		case C_SKI_120V_400A_200A_100A_192KW_92:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_93:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_94:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_95:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_96:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_97:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_98:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_99:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_100:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_101:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_102:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_103:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_104:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_105:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_106:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_107:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_108:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_109:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_110:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_111:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_112:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_113:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_114:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_115:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_116:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_117:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_118:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_119:	//khj_200308
		////////////////////////////////////////////////////////
		//case C_SKI_120V_400A_200A_100A_192KW_169:	//ktg_200410 //shh_240730
		//case C_TEST_120V_400A_200A_100A_192KW:		//shh_200723
		case C_SKI_120V_400A_200A_100A_192KW_170:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_171:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_172:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_173:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_174:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_175:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_176:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_177:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_178:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_179:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_180:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_181:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_182:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_183:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_184:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_185:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_186:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_187:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_188:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_189:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_190:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_191:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_192:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_193:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_194:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_195:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_196:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_197:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_198:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_199:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_200:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_201:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_202:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_203:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_204:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_205:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_206:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_207:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_208:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_209:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_210:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_211:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_212:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_213:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_214:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_215:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_216:	//shh_200905
		//case C_SKI_120V_400A_200A_100A_192KW_232:	//shh_200914 //shh_240730
		case C_SKI_120V_400A_200A_100A_192KW_233:	//ktg_200908
		case C_SKI_120V_400A_200A_100A_192KW_234:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_235:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_236:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_237:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_238:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_239:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_240:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_241:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_242:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_243:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_244:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_245:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_246:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_247:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_248:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_249:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_250:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_251:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_252:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_253:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_254:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_255:	//shh_220113
			//shuntV = 666.7mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774
			//V1 = 666.7mV * G1 = 5978.4946236559mV
			//ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.9784946236559;
			ret_val = (double)400000000 * 5.0 / 5.9784946236559; //kjhw_190304
			break;
		case C_SKI_120V_425A_200A_100A_50A_192KW:       //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_2:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_3:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_4:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_5:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_6:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_7:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_8:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_9:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_10:    //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_11:    //phb_230206
			//shuntV = 666.0000mV
            //AMP = AD620A
            //G1 = 1 + 49.4Kohm / 6.20Kohm = 8.9677
            //V1 = shuntV * G1 = 5972.51612903mV
            ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.97251613;
            break;
		/*
		case C_SKI_120V_425A_300A_200A_100A_192KW:      //phb_230105
        case C_SKI_120V_425A_300A_200A_100A_192KW_2:    //phb_230105
        case C_SKI_120V_425A_300A_200A_100A_192KW_3:    //phb_230105
			//shuntV = 999.0000mV
            //AMP = AD620A
            //G1 = 1 + 49.4Kohm / 10.00Kohm = 5.9400
            //V1 = shuntV * G1 = 5934.06mV
            ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.93406;
            break;
		*/
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
			ret_val = (double)400000000 * 5.0 / 5.3714285714;
			break;
		case C_SKI_180V_425A_200A_100A_192KW:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_2:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_3:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_4:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_5:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_6:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_7:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_8:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_9:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_10:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_11:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_12:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_13:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_14:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_15:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_16:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_17:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_18:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_19:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_20:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_21:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_22:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_23:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_24:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_25:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_26:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_27:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_28:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_29:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_30:	//sec_221221
           //shuntV = 666.0000mV
           //AMP = AD620A
           //G1 = 1 + 49.4Kohm / 6.20Kohm = 8.9677
           //V1 = shuntV * G1 = 5972.51612903mV
           ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.97251613;
           break;
		case C_SKI_180V_600A_300A_100A_320KW:       //phb_230220
		//case C_SKI_180V_600A_300A_100A_320KW_2:		//shh_231004
			//shuntV = 800.0000mV
            //AMP = AD620A
            //G1 = 1 + 49.4Kohm / 7.70Kohm = 7.4156
            //V1 = shuntV * G1 = 5932.46753247mV
            ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.93246753;
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
		case C_SKI_260V_425A_200A_442KW:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_2:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_3:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_4:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_5:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_6:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_7:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_8:	//shh_211020
		case C_SKI_260V_425A_200A_442KW_9:	//shh_211020
			//shuntV = 666.7mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.17kohm = 9.006483
			//V1 = shuntV * G1 = 6004.32198811mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 /6.00432198811 ;
			break;
		case C_SKI_400V_100A_10A_40KW: //jhk_120112
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_NORTHVOLT_400V_200A_100A_160KW:		//ktg_200415
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.967741935483871
			//V1 = shuntV * G1 = 5978.494623655914mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.978494624;
			break;
		case C_SK_450V_200A_10A_180KW:
		case C_SK_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_SK_450V_200A_10A_360KW:
		case C_DAEWOO_450V_200A_50A_90KW:	//kjh_150518
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_SKI_450V_200A_100A_90KW:			//sec_221223
           //shuntV = 666.0000mV
           //AMP = AD620A
           //G1 = 1 + 49.4Kohm / 6.20Kohm = 8.9677
           //V1 = shuntV * G1 = 5972.51612903mV
           ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.97251613;
           break;
		case C_SKI_450V_150A_100A_50A_260KW:	//sec_221215
		case C_SKI_450V_150A_100A_50A_260KW_2:	//sec_221215
           //shuntV = 999.0000mV
           //AMP = AD620A
           //G1 = 1 + 49.4Kohm / 10.00Kohm = 5.9400
           //V1 = shuntV * G1 = 5934.06000000mV
           ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.93406000;
           break;
		case C_SKI_500V_450A_200A_450KW:		//khj_200308
		case C_SKI_500V_450A_200A_450KW_2:		//khj_200308
		case C_SKI_500V_450A_200A_450KW_3:		//khj_200308
		//case C_SKI_500V_450A_200A_450KW_4:	//ktg_200410 //shh_240730s
		//case C_SKI_500V_450A_200A_450KW_5:	//ktg_200410
		//case C_SKI_500V_450A_200A_450KW_6:	//ktg_200410 //shh_240730e
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.967741935483871
			//V1 = 666.667mV * G1 = 5,978.497612903226mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.978497612903226;
			break;
		case C_SKC_600V_200A_100A_50A_240KW: //jhkw_120807
		case C_SKC_600V_200A_100A_50A_240KW_2: //jhkw_121019
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_DAEHWA_600V_400A_200A_100A_200KW:	//jhk_170120
	
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774193548
			//V1 = shuntV * G1 = 5978.4946236559mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.978494624;
			break;
		case C_DAEHWA_750V_300A_100A_150KW:	//jhk_170203
	
			//shuntV = 750mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 7kohm = 8.057142857
			//V1 = 750mV * G1 = 6042.857142857mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.042857143;
			break;
		case C_DAEHWA_800V_200A_50A_160KW:	//jhk_160831
			//shuntV = 500mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 4.5kohm = 11.97777778
			//V1 = shuntV * G1 = 5988.888889mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.988888889;
			break;
		case C_SKI_1000V_400A_200A_100A_450KW:	//sec_230103
           ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.00432198;
           break;
		case C_SKI_1200V_250A_100A_50A_300KW:	//jhk_131209
			//shuntV = 625mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.0kohm = 9.233333
			//V1 = shuntV * G1 = 5770.83333mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.77083333;
			break;
		/*
		case C_SKI_1200V_500A_400A_300A_200A_600KW:     //phb_230116
		case C_SKI_1500V_350A_300A_200A_100A_450KW:     //phb_230117
			//shuntV = 800.0000mV
            //AMP = AD620A
            //G1 = 1 + 49.4Kohm / 7.60Kohm = 7.5000
            //V1 = shuntV * G1 = 6000.00000000mV
            ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.00000000;
            break;
		*/
		case C_SKI_1500V_400A_100A_500KW:	//jhk_180610
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.2kohm = 8.96774193548
			//V1 = shuntV * G1 = 5978.4946236559mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 5.978494624;
			break;
		case C_PNE_2000V_300A_300KW:	//jhk_200313
		case C_PNE_2000V_300A_600KW:	//jhk_200313
		case C_SKI_2000V_300A_200A_100A_50A_600KW:  //phb_230320
			//shuntV = 666.667mV
			//AMP -> AD620A
			//G1 = 1 + 49.4kohm / 6.17kohm = 9.00648298
			//V1 = 666.666mV * G1 = 6004.321988mV
			ret_val = (double)myPs->config.maxI[0] * 5.0 / 6.004321988115;
			break;
		default:
			//ret_val = (double)myPs->config.maxI[0];
			maxI = myPs->config.maxI[0];	//jhkw_231127s
			i_feed = myPs->config.i_feed[0];
			
			if(myPs->config.ratioI == MICRO_UNIT) {	//uA
				ref = 5000000.0;
			} else if(myPs->config.ratioI == MILLI_UNIT) {	//mA
				ref = 5000.0;
			} else {	//A
				ref = 5.0;
			}

			ret_val = (double)maxI * ref / i_feed;	//jhkw_231127e
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
}

void aSetAuxDA_I(int bd, int channel, int ch)
{ //debug_size_cob
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
}

void aCalCh_Current(int bd, int channel, int ch, int range)
{ //debug_size_cob
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
	return 0;
}

int aFindADCaliPoint_2(int bd, int ch, long value, int type, int range)
{ //debug_size_cob
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
	return 0;
}

int	aFindDACaliPoint_2(int bd, int ch, long value, int type, int range)
{ //debug_size_cob
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
	long ratioV, ratioI; //jhkw_231127
	double ratioP;	//jhkw_231127

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
				&& myData->cali[ch].mode != CALI_MODE_ONLY_CHECK //jhkw_200317
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
				&& myData->cali[ch].mode != CALI_MODE_ONLY_CHECK //jhkw_200317
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
	//for(k=0; k < 4; k++) {
	//	ch = bd * 4 + k;
	//}
	//if(ch >= MAX_CH_8) return;
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
			case C_SKI_AUX_BOX: //jhkw_211012
				tempV = myData->SubSensV.ch[20].sensV;
				tmpV = tempV;
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
		}	//jhkw_231127

		if(myCh->op.state == C_CALI) {
			myData->cali[ch].orgAD[type] = tempV;
			myCh->op.Vsens = (long)tempV;
			myCh->misc.tmpVsens = tmpV;
		} else {
			if(myData->ChAttribute[ch].chNo_master != 0) {
				if(myData->AppControl.config.debugType == 110
					|| myData->AppControl.config.debugType == 111) {
					//for debugging software
//20181219 KHK Test-------------------------------------				
					if(myCh->op.state == C_RUN){
						if(myCh->misc.cmd_i[0] > 0){
							if(myCh->op.Vsens >= myCh->misc.cmd_v[0]) {
								myCh->op.Vsens = myCh->misc.cmd_v[0];
							} else myCh->op.Vsens += (10000 / ratioV);
						}else if(myCh->misc.cmd_i[0] < 0){
							if(myCh->op.Vsens >= myCh->misc.cmd_v[0]) {
								myCh->op.Vsens -= (10000 / ratioV);
							}else{
								myCh->op.Vsens = myCh->misc.cmd_v[0];
							}
						}
					}else{
						myCh->op.Vsens = myPs->config.maxV[0]/5;
					}
//-----------------------------------------------------------------				
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
							} else myCh->op.Vsens += (10000 / ratioV);
						}else if(myCh->misc.cmd_i[0] < 0){
							if(myCh->op.Vsens >= myCh->misc.cmd_v[0]) {
								myCh->op.Vsens -= (10000 / ratioV);
							}else{
								myCh->op.Vsens = myCh->misc.cmd_v[0];
							}
						}
					}else{
						myCh->op.Vsens = myPs->config.maxV[0]/5;
					}
					myCh->misc.tmpVsens = myCh->op.Vsens;
#if defined __DEBUG__	//shh_231124s
				} else if(myData->AppControl.config.debugType == 230) {
					myCh->op.Vsens = myCh->misc.sil_v_i_val[0];
					myCh->misc.tmpVsens = myCh->op.Vsens;
#endif					//shh_231124e
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
					for(j=0; j < 3; j++) {
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
//20181219 KHK Test-------------------------------------				
				if(myCh->op.state == C_RUN){
					if(myCh->misc.cmd_i[0] > 0){
						if(myCh->op.Vsens < myCh->misc.cmd_v[0]){
							myCh->op.Isens = myCh->misc.cmd_i[0];
						}else{
							myCh->op.Isens = myCh->misc.cmd_i[0];
						}
					}else{
						myCh->op.Isens = myCh->misc.cmd_i[0];
					}
				}else{
					myCh->op.Isens = 0;
				}
//------------------------------------------------------
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
#if defined __DEBUG__	//shh_231124s
			} else if(myData->AppControl.config.debugType == 230) {
				myCh->op.Isens = myCh->misc.sil_v_i_val[1];
				myCh->misc.tmpIsens = myCh->op.Isens;
#endif					//shh_231124e
			} else {
				if(acc_type == 0) {
					myCh->op.Isens = (long)tempI;
					myCh->misc.tmpIsens = tmpI;
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

		ratioV = myData->mData.ratioV;	//jhkw_231127s
		ratioI = myData->mData.ratioI;
		ratioP = myData->mData.ratioP;	//jhkw_231127e
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

		switch(myData->AppControl.config.systemModel) {
			case F_SDI_5V_400A_200A_100A_10A:
			case F_SDI_5V_400A_200A_100A_10A_2:
				j = 0;
				break;
			case F_SDI_5V_450A_200A_100A_10A:
				j = 1;
				break;
			default:
				break;
		}
	}
} //kjg_logic_type_140325_e


//20191008 KHK----------------------------------------------------
void Analog_Value_Input_12(int main_slot)
{
	int ch, end_slot, next_ch, cnt_end=0, sel_ad, i, type, filter_ad_count;

	if(main_slot >= 7) return;

	switch(myPs->config.scan_period) {
		default: //10ms
			end_slot = 4;
			break;
	}
	ch = main_slot % 4;

	next_ch = main_slot + 1;
	if(next_ch >= 7) next_ch = 0;

	if(main_slot < end_slot) {
		type = 0; // AD_0: Ch 0, 1, 2, 3, AD_1: Ch 4, 5, 6, 7
	} else {
		type = 1; //Reference AD_0: 5V, -5V, GND, AD_1: 5V, -5V, GND
	}

	sel_ad = 1;
	if(myPs->config.chPerBd > 4){
		sel_ad = 2;
	}

	for(i=0; i < sel_ad; i++){
		Get_AD_Value_12(type, i, ch); //AD_0
	}
	Set_Mux_12(next_ch);
	if(type == 0) {
		for(i=0; i < sel_ad; i++){
			CalChAverage_12(i, ch);
		}
	} else {
		filter_ad_count = (int)myPs->config.filter_ad_count;
		cnt_end = myPs->misc.bd_sensCount;
		if(myPs->misc.bd_sensCountFlag == P0) {
			cnt_end = cnt_end + 1;
		} else {
			cnt_end = filter_ad_count;
		}
		for(i=0; i < sel_ad; i++){
			myPs->misc.tmp_ref_muxVal = ch;
			CalSourceAverage(i, cnt_end);
			CalSourceAverage2(i);
			CalibratorSource(i);
		}
		if(next_ch == 0) bd_sens_count_increment_12();
	}
}

void Get_AD_Value_12(int type, int sel_ad, int ch)
{
 	Read_AD_Data_12(type, sel_ad, ch);
	AD_Data_Calculate_12(type, sel_ad, ch);
}

void Read_AD_Data_12(int type, int sel_ad, int ch)
{
	int real_ch;
	int addr_ad_data_v_h, addr_ad_data_v_l;
	int addr_ad_data_i_h, addr_ad_data_i_l;
	U_ADDA ADValue_V, ADValue_I;
	
	if(sel_ad == 0){ //Ch 0, 1, 2, 3
		addr_ad_data_v_h = CH_ADDR_AD_DATA_H_A1;
		addr_ad_data_v_l = CH_ADDR_AD_DATA_L_A1;
		addr_ad_data_i_h = CH_ADDR_AD_DATA_H_A2;
		addr_ad_data_i_l = CH_ADDR_AD_DATA_L_A2;
	}else{ //Ch 4, 5, 6, 7
		addr_ad_data_v_h = CH_ADDR_AD_DATA_H_A1+4;
		addr_ad_data_v_l = CH_ADDR_AD_DATA_L_A1+4;
		addr_ad_data_i_h = CH_ADDR_AD_DATA_H_A2+4;
		addr_ad_data_i_l = CH_ADDR_AD_DATA_L_A2+4;
	}

	ADValue_V.byte[1] = (unsigned char)inb(addr_ad_data_v_h);
	ADValue_V.byte[0] = (unsigned char)inb(addr_ad_data_v_l);

	ADValue_I.byte[1] = (unsigned char)inb(addr_ad_data_i_h);
	ADValue_I.byte[0] = (unsigned char)inb(addr_ad_data_i_l);

	if(type == 0) { //CH V, I
		real_ch = ch + sel_ad * 4;
//		ad_count = myData->cData[real_ch].misc.sensCount;
		myData->cData[real_ch].misc.sumV[0] = (long)ADValue_V.val;
		myData->cData[real_ch].misc.sumI[0] = (long)ADValue_I.val;
	} else { //sourceV, sourceI
		myData->bData[sel_ad].misc.source[ch].sumV[0]
			= (long)ADValue_V.val;
		myData->bData[sel_ad].misc.source[ch].sumI[0]
			= (long)ADValue_I.val;
	}
}

void Set_Mux_12(int next_ch)
{
    unsigned char mux;
	
	mux = next_ch & 0x0f;
	outb(mux, CH_ADDR_MUX_1); //Mux Select

	outb(0x00, CH_ADDR_AD_START); //AD Start
}

void AD_Data_Calculate_12(int type, int sel_ad, int ch)
{
	int i, ad_count, bd;
	float avg;

	ad_count = (int)myPs->config.filter_ad_count;
	bd = sel_ad;

	if(type == 0) { //Ch V, I
		ch += (sel_ad * myPs->config.chPerBd);
		if(ch >= MAX_CH_PER_MODULE) return;
		
		avg = (float)myData->cData[ch].misc.sumV[0];
		AD_Data_Calculate_V(bd, ch, 0, avg, 0);

		avg = (float)myData->cData[ch].misc.sumI[0];
		AD_Data_Calculate_I(bd, ch, 1, avg, 0);
	} else { //sourceV, sourceI
		avg = (float)myData->bData[bd].misc.source[ch].sumV[0];
		i = myPs->misc.bd_sensCount;
		myData->bData[bd].misc.source[ch].sensSumV[i] = (long)avg;

		avg = (float)myData->bData[bd].misc.source[ch].sumI[0];
		i = myPs->misc.bd_sensCount;
		myData->bData[bd].misc.source[ch].sensSumI[i] = (long)avg;
	}
}

void CalChAverage_12(int bd, int ch)
{
    int	cnt, cnt_end, i, j, type, master_ch, acc_type;
	int filter_ad_count;
	long tmpV, tmpI, min, max;
    double tempV, tempI, acc_time;
	long ratioV, ratioI;	//jhkw_231127
	double ratioP;	//jhkw_231127
	
	ch += (bd * myPs->config.chPerBd);
	if(ch >= MAX_CH_PER_MODULE) return;

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
	}	//jhkw_231127
//	cnt_end = ch_sens_count_increment(ch);
	filter_ad_count = (int)myPs->config.filter_ad_count;

	cnt = myCh->misc.sensCount;
	if(myCh->misc.sensCountFlag == P0) {
		cnt_end = cnt + 1;
	} else {
		cnt_end = filter_ad_count;
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
	
	//jhkw_211012s
	switch(myData->AppControl.config.systemModel) {
		case C_SKI_AUX_BOX:
			tempV = myData->SubSensV.ch[20].sensV;
			tmpV = tempV;
			break;
		default:
			break;
	}
	//jhkw_211012e
	if(myCh->op.state == C_CALI) {
		myData->cali[ch].orgAD[type] = tempV;
		myCh->op.Vsens = (long)tempV;
		myCh->misc.tmpVsens = tmpV;
	} else {
		if(myData->ChAttribute[ch].chNo_master != 0) {
			if(myData->AppControl.config.debugType == 110
				|| myData->AppControl.config.debugType == 111) {
//20181219 KHK Test-------------------------------------				
				if(myCh->op.state == C_RUN){
					if(myCh->misc.cmd_i[0] > 0){
//						if(myCh->op.Vsens >= myPs->config.maxV[0]) {
						if(myCh->op.Vsens >= myCh->misc.cmd_v[0]) {
							myCh->op.Vsens = myCh->misc.cmd_v[0];
//							myCh->op.Vsens = myPs->config.maxV[0]/5;
						} else myCh->op.Vsens += (10000 / ratioV);
					}else if(myCh->misc.cmd_i[0] < 0){
						if(myCh->op.Vsens >= myCh->misc.cmd_v[0]) {
							myCh->op.Vsens -= (10000 / ratioV);
						}else{
							myCh->op.Vsens = myCh->misc.cmd_v[0];
						}
					}
				}else{
					myCh->op.Vsens = myPs->config.maxV[0]/5;
				}
//20181219 KHK Test-------------------------------------				

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
			} else if(myData->AppControl.config.debugType == 220
				|| myData->AppControl.config.debugType == 235) { //shhw_240104
				if(myCh->op.state == C_RUN){
					if(myCh->misc.cmd_i[0] > 0){
						if(myCh->op.Vsens >= myCh->misc.cmd_v[0]) {
							myCh->op.Vsens = myCh->misc.cmd_v[0];
						} else myCh->op.Vsens += (10000 / ratioV);
					}else if(myCh->misc.cmd_i[0] < 0){
						if(myCh->op.Vsens >= myCh->misc.cmd_v[0]) {
							myCh->op.Vsens -= (10000 / ratioV);
						}else{
							myCh->op.Vsens = myCh->misc.cmd_v[0];
						}
					}
				}else{
					myCh->op.Vsens = myPs->config.maxV[0]/5;
				}
#if defined __DEBUG__	//shh_231124s
			} else if(myData->AppControl.config.debugType == 230) {
				myCh->op.Vsens = myCh->misc.sil_v_i_val[0];
				myCh->misc.tmpVsens = myCh->op.Vsens;
#endif					//shh_231124e
			} else {
				if(acc_type == 0) {
					myCh->op.Vsens = (long)tempV;
					myCh->misc.tmpVsens = tmpV;
				} else {
#ifdef __COA_VER_100B__
					if(myCh->op.runTime <= 9) {
#else //COA_VER_100B2~
					if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
						myCh->op.runTime_day, myCh->op.runTime, 0, 9) > 0) {
#endif
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
				for(j=0; j < 3; j++) {
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
#ifdef __COA_VER_100B__
					if(myCh->op.runTime <= 9) {
#else //COA_VER_100B2~
					if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
						myCh->op.runTime_day, myCh->op.runTime, 0, 9) > 0) {
#endif
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
	cnt = myCh->misc.sensCount;
	tmpI = myCh->misc.sensSumI[cnt];
	tempI = myCh->misc.sensSumI[0];
//	min = (long)tempI;
//	max = (long)tempI;
	min = (long)tmpI;
	max = (long)tmpI;
	myData->test_val_l[14] = myCh->misc.sensCountFlag;
	if(myCh->misc.sensCountFlag == P0) {
		if(myCh->misc.sensCount == 0) {
			myData->test_val_l[15] = myCh->misc.sensSumI[0];
			for(i=1; i < (int)myPs->config.filter_ad_count; i++) {
				myCh->misc.sensSumI[i] = 0;
			}
		}
	}
	for(i=1; i < cnt_end; i++) {
		tempI += (double)myCh->misc.sensSumI[i];
		if(myCh->misc.sensSumI[i] < min) min = myCh->misc.sensSumI[i];
		if(myCh->misc.sensSumI[i] > max) max = myCh->misc.sensSumI[i];
	}
	if(myCh->misc.sensCountFlag == P0) {
		tempI = (double)myCh->misc.sensSumI[cnt];
	} else {
		if(cnt_end < 3) {
			tempI /= cnt_end;
		} else {
			tempI = (tempI - (double)min - (double)max) / (cnt_end - 2);
		}
	}
	if(myCh->op.state == C_CALI) {
		myData->cali[ch].orgAD[type] = tempI;
		myCh->op.Isens = (long)tempI;
		myCh->misc.tmpIsens = tmpI;
	} else {
		if(myData->AppControl.config.debugType == 110) {
			//for debugging software
//20181219 KHK Test-------------------------------------				
			if(myCh->op.state == C_RUN){
				if(myCh->misc.cmd_i[0] > 0){
					if(myCh->op.Vsens < myCh->misc.cmd_v[0]){
						myCh->op.Isens = myCh->misc.cmd_i[0];
					}else{
						myCh->op.Isens = myCh->misc.cmd_i[0];
					}
				}else{
					myCh->op.Isens = myCh->misc.cmd_i[0];
				}
			}else{
				myCh->op.Isens = 0;
			}
//------------------------------------------------------			
			myCh->misc.tmpIsens = myCh->op.Isens;
		} else if(myData->AppControl.config.debugType == 111) {
			myCh->op.Isens = 0;
			myCh->misc.tmpIsens = 0;
		} else if(myData->AppControl.config.debugType == 220
			|| myData->AppControl.config.debugType == 235) { //shhw_240104
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
#if defined __DEBUG__	//shh_231124s
		} else if(myData->AppControl.config.debugType == 230) {
			myCh->op.Isens = myCh->misc.sil_v_i_val[1];
			myCh->misc.tmpIsens = myCh->op.Isens;
#endif					//shh_231124e
		} else {
			if(acc_type == 0) {
			//	myCh->op.Isens = (long)tempI;
				myCh->op.Isens = (long)tmpI;
				myCh->misc.tmpIsens = tmpI;
			} else {
#ifdef __COA_VER_100B__
				if(myCh->op.runTime <= 9) {
#else //COA_VER_100B2~
				if(time_cal_compare(COMP_LESS_THAN_OR_EQUAL_TO,
					myCh->op.runTime_day, myCh->op.runTime, 0, 9) > 0) {
#endif
					myCh->op.Isens = tmpI;
					myCh->misc.tmpIsens = tmpI;
				} else {
					myCh->op.Isens = (long)tempI;
					myCh->misc.tmpIsens = tmpI;
				}
			}
		}
	}

	switch(myCh->op.stepType){
		case STEP_CHARGE:
		case STEP_DISCHARGE:
		case STEP_Z:
		case STEP_PATTERN:
		case STEP_EXTERNAL_CAN:
		case STEP_EXTERNAL_CAN_2:
		case STEP_USERMAP:
			break;
		default:
			return;
	}
	if(myCh->op.state != C_RUN) return;
	if(myCh->op.phase != P50) return;

	ratioV = myData->mData.ratioV;	//jhkw_231127s
	ratioI = myData->mData.ratioI;
	ratioP = myData->mData.ratioP;	//jhkw_231127e

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
#ifdef __COA_VER_100B__
			if(myCh->op.runTime <= 8) {
			} else if(myCh->op.runTime == 8) {
#else //COA_VER_100B2~
			if(time_cal_compare(COMP_LESS_THAN,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
			} else if(time_cal_compare(COMP_EQUAL_TO,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
#endif
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
#ifdef __COA_VER_100B__
			if(myCh->op.runTime < 8) {
			} else if(myCh->op.runTime == 8) {
#else //COA_VER_100B2~
			if(time_cal_compare(COMP_LESS_THAN,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
			} else if(time_cal_compare(COMP_EQUAL_TO,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
#endif
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
#ifdef __COA_VER_100B__
			if(myCh->op.runTime < 8) {
			} else if(myCh->op.runTime == 8) {
#else //COA_VER_100B2~
			if(time_cal_compare(COMP_LESS_THAN,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
			} else if(time_cal_compare(COMP_EQUAL_TO,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
#endif
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
#ifdef __COA_VER_100B__
			if(myCh->op.runTime < 8) {
			} else if(myCh->op.runTime == 8) {
#else //COA_VER_100B2~
			if(time_cal_compare(COMP_LESS_THAN,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
			} else if(time_cal_compare(COMP_EQUAL_TO,
				myCh->op.runTime_day, myCh->op.runTime, 0, 8) > 0) {
#endif
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

	switch(myData->AppControl.config.systemModel) {
		case F_SDI_5V_400A_200A_100A_10A:
		case F_SDI_5V_400A_200A_100A_10A_2:
			j = 0;
			break;
		case F_SDI_5V_450A_200A_100A_10A:
			j = 1;
			break;
		default:
			break;
	}
}
//-----------------------------------------------------------


