#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <math.h>
#include <sys/io.h>
#include "com_socket.h"
#include "com_io.h"
#include "comm.h"
#include "comm1.h"
#include "switch.h"
#include "main.h"

char comm_value[256];
short int tmpValue[CH_READ_COUNT2];

int systemModel, group, cali_type, server_state, scanType, Channel, auxChNum;
int meter, calbox;
int MainBdNum, baseAddr, caliAddr;
int max_ch_per_bd;
int aux_da_sync_v, aux_da_sync_i, aux_haddr, aux_laddr;
int main_dac_sync_v, main_dac_sync_i, main_dac_sync_l, main_dac_high;
int main_dac_low_v, main_dac_low_i, main_dac_low_l;
int div_addr, run_addr;
int mux1_addr, mux2_addr, mux3_addr;
int start_addr, ad_haddr, ad_laddr;

typedef struct avstdev_tag {
    int ave;
    int stdev;
	int data1;
	int data2;
} AVSTDEV;

typedef struct ref_tag {
    int ref_P;
    int ref_0;
    int ref_N;
    int data;
} REF;

typedef struct dac_value_tag {
    int V_N;
    int V_P;
    int I_N;
    int I_P;
} DAC_VALUE;

typedef union u_adda_tag {
    short int val;
    unsigned char data[2];
} U_ADDA;

long calSourceV[3], calSourceI[3];
double Vsource_AD_a, Vsource_AD_b, Isource_AD_a, Isource_AD_b;

double cmdV_DA_a[2], cmdV_DA_b[2], cmdI_DA_a[2], cmdI_DA_b[2];
double cmdL_DA_a[2], cmdL_DA_b[2];
int cmd_adv_org[2][3], cmd_adi_org[2][3], cmd_adl_org[2][3];
double cmd_adv[2][3], cmd_adi[2][3], cmd_adl[2][3];

REF vref;
REF iref;
DAC_VALUE dac_value[2];

int CharToInt(char cmp)
{
	char character[10] = {'0','1','2','3','4','5','6','7','8','9'};
	int num;

	for(num=0; num < 10; num++) {
		if(cmp == character[num]) break;
	}

	return num;
}

long Conversion(void)
{
	int tmp, i;//, exp;
	long real, temp;
	
	temp = 100000;
	tmp = CharToInt(comm_value[1]);
	real = tmp * 1000000;
	for(i=3; i <= 7; i++) {
		tmp = CharToInt(comm_value[i]);
		real += (tmp * temp);
		temp /= 10;
	}
/*
	tmp = CharToInt(comm_value[13]);
	exp = tmp * 10;
	tmp = CharToInt(comm_value[14]);
	exp += tmp;

	temp = 1;
	for(i=0; i < exp; i++) {
		temp *= 10;
	}
	if(comm_value[0] == '-') {
		real = real1 / temp;
	} else {
		real = real1 * temp;
	}*/

	return real;
}

long RealData(void)
{
	int rtn;
	long real;

	rtn = rxsettings1("READ?\n", 0);
	if(comm_value[2] == '.') {
		real = Conversion();
		if(comm_value[0] == '-') real *= (-1);
	} else real = 0;

	return real;
}

void SetAuxDA_V1(int num, unsigned char val)
{ //AD5308
    unsigned char auxDAChannel, low, sync;
    int j;

    if(iopl(3)) exit(1);

    if(num < 0) {
		val = 0x7F;
		for(j=0; j < max_ch_per_bd; j++){
			sync = 0x01;
			sync = sync << (j / 8);
			outb(sync, (baseAddr + aux_da_sync_v));

    		auxDAChannel = (unsigned char)(j % 8); 
	   		auxDAChannel = auxDAChannel << 4;
		   	auxDAChannel |= val >> 4;
			outb(auxDAChannel, (baseAddr + aux_haddr)); //high
			low = val << 4;
			outb(low, (baseAddr + aux_laddr)); //low
			usleep(100);
		}
		printf("all aux da_v set\n");

		return;
    }

	j = num;
	if(j >= max_ch_per_bd) return;

	printf("aux_v cali ch num : %d, val : %x\n", j, val);
	sync = 0x01;
	sync = sync << (j / 8);
	outb(sync, (baseAddr + aux_da_sync_v));

   	auxDAChannel = (unsigned char)(j % 8); 
    auxDAChannel = auxDAChannel << 4;
    auxDAChannel |= val >> 4;
	outb(auxDAChannel, (baseAddr + aux_haddr)); //high
	low = val << 4;
	outb(low, (baseAddr + aux_laddr)); //low
	usleep(100);
}

void SetAuxDA_V2(int num, unsigned short val)
{ //AD5318
    unsigned char auxDAChannel, low, sync;
    int j;

    if(iopl(3)) exit(1);

    if(num < 0) {
		val = 0x1FF;
		for(j=0; j < max_ch_per_bd; j++){
    		auxDAChannel = (unsigned char)(j % 8); 
	   		auxDAChannel = auxDAChannel << 4;
		   	auxDAChannel |= (unsigned char)(val >> 6);
			outb(auxDAChannel, (baseAddr + aux_haddr)); //high
			low = (unsigned char)(val << 2);
			outb(low, (baseAddr + aux_laddr)); //low

			sync = 0x01;
			sync = sync << (j / 8);
			outb(sync, (baseAddr + aux_da_sync_v));
			usleep(50);
		}
		printf("all aux da_v set\n");

		return;
    }

	j = num;
	if(j >= max_ch_per_bd) return;

	printf("aux_v cali ch num : %d, val : %x\n", j, val);
   	auxDAChannel = (unsigned char)(j % 8); 
    auxDAChannel = auxDAChannel << 4;
    auxDAChannel |= (unsigned char)(val >> 6);
	outb(auxDAChannel, (baseAddr + aux_haddr)); //high
	low = (unsigned char)(val << 2);
	outb(low, (baseAddr + aux_laddr)); //low

	sync = 0x01;
	sync = sync << (j / 8);
	outb(sync, (baseAddr + aux_da_sync_v));
	usleep(50);
}

void SetAuxDA_V(int num, unsigned short val)
{
	switch(systemModel) {
		case SW_DEMO_5V3A:
			SetAuxDA_V2(num, val); //AD5318
			break;
		default:
			break;
	}
}

void SetAuxDA_I1(int num, unsigned char val)
{ //AD5308
    unsigned char auxDAChannel, low, sync;
    int j;

    if(iopl(3)) exit(1);

    if(num < 0) {
		val = 0x7F;
		for(j=0; j < max_ch_per_bd; j++){
			sync = 0x01;
			sync = sync << (j / 8);
			outb(sync, (baseAddr + aux_da_sync_i));

    		auxDAChannel = (unsigned char)(j % 8); 
	   		auxDAChannel = auxDAChannel << 4;
		   	auxDAChannel |= val >> 4;
			outb(auxDAChannel, (baseAddr + aux_haddr)); //high
			low = val << 4;
			outb(low, (baseAddr + aux_laddr)); //low
			usleep(100);
		}
		printf("all aux da_i set\n");

		return;
    }

	j = num;
	if(j >= max_ch_per_bd) return;

	printf("aux_i cali ch num : %d, val : %x\n", j, val);
	sync = 0x01;
	sync = sync << (j / 8);
	outb(sync, (baseAddr + aux_da_sync_i));

   	auxDAChannel = (unsigned char)(j % 8); 
    auxDAChannel = auxDAChannel << 4;
    auxDAChannel |= val >> 4;
	outb(auxDAChannel, (baseAddr + aux_haddr)); //high
	low = val << 4;
	outb(low, (baseAddr + aux_laddr)); //low
	usleep(100);
}

void SetAuxDA_I2(int num, unsigned short val)
{ //AD5318
    unsigned char auxDAChannel, low, sync;
    int j;

    if(iopl(3)) exit(1);

    if(num < 0) {
		val = 0x1FF;
		for(j=0; j < max_ch_per_bd; j++){
    		auxDAChannel = (unsigned char)(j % 8); 
	   		auxDAChannel = auxDAChannel << 4;
		   	auxDAChannel |= (unsigned char)(val >> 6);
			outb(auxDAChannel, (baseAddr + aux_haddr)); //high
			low = (unsigned char)(val << 2);
			outb(low, (baseAddr + aux_laddr)); //low

			sync = 0x01;
			sync = sync << (j / 8);
			outb(sync, (baseAddr + aux_da_sync_i));
			usleep(50);
		}
		printf("all aux da_i set\n");

		return;
    }

	j = num;
	if(j >= max_ch_per_bd) return;

	printf("aux_i cali ch num : %d, val : %x\n", j, val);
   	auxDAChannel = (unsigned char)(j % 8); 
    auxDAChannel = auxDAChannel << 4;
    auxDAChannel |= (unsigned char)(val >> 6);
	outb(auxDAChannel, (baseAddr + aux_haddr)); //high
	low = (unsigned char)(val << 2);
	outb(low, (baseAddr + aux_laddr)); //low

	sync = 0x01;
	sync = sync << (j / 8);
	outb(sync, (baseAddr + aux_da_sync_i));
	usleep(50);
}

void SetAuxDA_I(int num, unsigned short val)
{
	switch(systemModel) {
		case LGCE_NJ_5V2A:
		case SW_DEMO_5V3A:
			SetAuxDA_I2(num, val); //AD5318
			break;
		default:
			SetAuxDA_I1(num, (unsigned char)val); //AD5308
			break;
	}
}

void RelayAllOn(void)
{
    int offSetAddr;

    if(iopl(3)) exit(1);

	printf("Relay all on\n");

	switch(systemModel) {
		case LGCE_NJ_5V2A:
			for(offSetAddr=0; offSetAddr < 8; offSetAddr++) {
				outb(0xFF, (baseAddr + run_addr + offSetAddr));
			} 
			break;
		default:
			outb(0x01, (baseAddr + div_addr));	
			for(offSetAddr=0; offSetAddr < 8; offSetAddr++) {
				outb(0xFF, (baseAddr + offSetAddr));
			} 
			outb(0x00, (baseAddr + div_addr));
			break;
	}
}
  
void RelayAllOff(void)
{
    int offSetAddr;

    if(iopl(3)) exit(1);

	printf("Relay all off\n");

	switch(systemModel) {
		case LGCE_NJ_5V2A:
		case SW_DEMO_5V3A:
			for(offSetAddr=0; offSetAddr < (max_ch_per_bd / 8); offSetAddr++) {
				outb(0x00, (baseAddr + run_addr + offSetAddr));
			} 
			break;
		default:
			outb(0x01, (baseAddr + div_addr));	
			for(offSetAddr=0; offSetAddr < 8; offSetAddr++) {
				outb(0x00, (baseAddr + offSetAddr));
			} 
			outb(0x00, (baseAddr + div_addr));	
			break;
	}
}

void RelayOnOff(int num, int type)
{
    unsigned char icVal;
    int offSetAddr;

    if(iopl(3)) exit(1);

  	offSetAddr = num / 8;
   	icVal = 0x01;
    icVal = icVal << (num % 8);

	switch(systemModel) {
		case LGCE_NJ_5V2A:
		case SW_DEMO_5V3A:
			if(type == ON) {
				outb(icVal, (baseAddr + run_addr + offSetAddr));
			} else { //OFF
				outb(0x00, (baseAddr + run_addr + offSetAddr));
			}
			break;
		default:
 		   if(type == ON) {
				outb(0x01, (baseAddr + div_addr));	
				outb(icVal, (baseAddr + offSetAddr));
				outb(0x00, (baseAddr + div_addr));
			} else { //OFF
				outb(0x01, (baseAddr + div_addr));	
				outb(0x00, (baseAddr + offSetAddr));
				outb(0x00, (baseAddr + div_addr));
		    }
			break;
	}
}

void SetMux1(int num, int type)
{
    unsigned char muxVal1, muxVal2, muxVal3;
	int mux_ch;

	if(iopl(3)) exit(1);

	switch(systemModel) {
		case LGCN_PB2:
			mux_ch = 16;
			break;
		default:
			mux_ch = 8;
			break;
	}

	muxVal1 = 0x01;
	muxVal1 = muxVal1 << (num / mux_ch);
	if(type == SCAN_VOLTAGE) {
		muxVal2 = 0x00;
	} else { //SCAN_CURRENT
		muxVal2 = 0x10;
	}
	muxVal3 = (num % mux_ch) & 0x0F;
	
	outb(muxVal1 | 0x40, (baseAddr + mux1_addr));
	outb(muxVal2, (baseAddr + mux2_addr));
	outb(muxVal3, (baseAddr + mux3_addr));
}

void SetMux2(int num, int type)
{
    unsigned char muxVal1, muxVal2;
	int mux_ch;

	if(iopl(3)) exit(1);

	mux_ch = 16;

	muxVal1 = (0x20 | MainBdNum);

	muxVal2 = 0x10;
	muxVal2 = muxVal2 << (num / mux_ch);
	muxVal2 |= (num % mux_ch);
	
	outb(muxVal1, 0x620 + mux1_addr);
	outb(muxVal2, (baseAddr + mux2_addr));
}

void SetMux3(int num, int type)
{
    unsigned char muxVal1, muxVal2;
	int mux_ch;

	if(iopl(3)) exit(1);

	mux_ch = 8;

	if(group == 0) {
		muxVal1 = 0xA0;
	} else {
		muxVal1 = 0xA1;
	}

	muxVal2 = 0x10;
	muxVal2 = muxVal2 << (num / mux_ch);
	muxVal2 |= (num % mux_ch);
	muxVal2 |= 0x80;
	
	outb(muxVal1, 0x620 + mux1_addr);
	outb(muxVal2, (baseAddr + mux2_addr));
}

void SetMux(int num, int type)
{
	switch(systemModel) {
		case LGCE_NJ_5V2A:
			SetMux2(num, type);
			break;
		case SW_DEMO_5V3A:
			SetMux3(num, type);
			break;
		default:
			SetMux1(num, type);
			break;
	}
}

void VoltDACReset(void)
{
	printf("Voltage DAC RESET\n");

	switch(systemModel) {
		case LGCE_NJ_5V2A:
			outb(0x01, (0x620 + main_dac_sync_v));
			outb(0x00, (0x620 + main_dac_high));
			outb(0x00, (0x620 + main_dac_low_v));
			break;
		case SW_DEMO_5V3A:
			outb(0x03, (0x620 + main_dac_sync_v));
			outb(0x00, (0x620 + main_dac_high));
			outb(0x00, (0x620 + main_dac_low_v));
			break;
		default:
			outb(0x00, (baseAddr + main_dac_high));	
			outb(0x00, (baseAddr + main_dac_low_v));
			break;
	}
}

void CurrDACReset(void)
{
	printf("Current DAC RESET\n");

	switch(systemModel) {
		case LGCE_NJ_5V2A:
			outb(0x01, (0x620 + main_dac_sync_i));
			outb(0x00, (0x620 + main_dac_high));
			outb(0x00, (0x620 + main_dac_low_i));
			break;
		case SW_DEMO_5V3A:
			outb(0x03, (0x620 + main_dac_sync_i));
			outb(0x00, (0x620 + main_dac_high));
			outb(0x00, (0x620 + main_dac_low_i));
			break;
		default:
			outb(0x00, (baseAddr + main_dac_high));	
			outb(0x00, (baseAddr + main_dac_low_i));
			break;
	}
}
	
void CreatADFile(int count, int type)
{
	char fname[128];
	int i;
	FILE *fp;

	memset(fname, 0, sizeof fname);

	if(type == SCAN_VOLTAGE) {
		sprintf(fname,"./tmp/ADVALUEVolt_%dB%dC.csv", MainBdNum+1, Channel+1);
	} else { //SCAN_CURRENT
		sprintf(fname,"./tmp/ADVALUECurr_%dB%dC.csv", MainBdNum+1, Channel+1);
	}

	fp = fopen(fname, "w");
	if(fp == NULL) {
		puts("Cannot open file\n");
		return;
	}

	for(i=0; i < count; i++) {
		fprintf(fp, "%d,%d\n", i+1, tmpValue[i]);
	}
	fclose(fp);
}

int AD_stdev(float Average, int count) {
	int i, val;
	float tol=0;

	for(i=0; i < count; i++) {
		tol += (((float)tmpValue[i] - Average)
			* ((float)tmpValue[i] - Average));
	}

	val = (int)(sqrt(tol / count) * 10000);

	return val;
}
	
void Ref_V_Read(void)
{
    int	i, j, tmp;
	double val;
	U_ADDA myVal;

	for(i=0; i < 3; i++) {
		tmp = 0;
		switch(systemModel) {
			case LGCE_NJ_5V2A:
				//mux
				if(i == 0) {
					tmp = 0x10;
				} else if(i == 1) {
					tmp = 0x12;
				} else if(i == 2) {
					tmp = 0x11;
				} else {
					tmp = 0x13;
				}
				outb((unsigned char)tmp, (0x620 + mux1_addr));
				usleep(200000);

				//AD
				val = 0.0;
				for(j=0; j < REF_READ_COUNT; j++) {
					outb(0x03, (0x620 + start_addr));
					usleep(2000);
					myVal.data[1] = inb(0x620 + ad_haddr);
					myVal.data[0] = inb(0x620 + ad_laddr);
					val += (double)myVal.val;
				}
				break;
			case SW_DEMO_5V3A:
				//mux
				if(i == 0) {
					tmp = 0x50;
				} else if(i == 1) {
					tmp = 0x52;
				} else if(i == 2) {
					tmp = 0x51;
				} else {
					tmp = 0x53;
				}
				outb((unsigned char)tmp, (0x620 + mux1_addr));
				usleep(20000); //kjg_081003

				//AD
				val = 0.0;
				for(j=0; j < REF_READ_COUNT; j++) {
					outb(0x03, (0x620 + start_addr));
					usleep(2000);
					if(group == 0) {
						myVal.data[1] = inb(0x620 + ad_haddr);
						myVal.data[0] = inb(0x620 + ad_laddr);
					} else {
						myVal.data[1] = inb(0x620 + ad_haddr + 2);
						myVal.data[0] = inb(0x620 + ad_laddr + 2);
					}
					val += (double)myVal.val;
				}
				break;
			default:
				//mux
				if(i == 0) {
					tmp = 0x00;
				} else if(i == 1) {
					tmp = 0x02;
				} else if(i == 2) {
					tmp = 0x01;
				} else {
					tmp = 0x03;
				}
				outb((unsigned char)tmp, (baseAddr + mux2_addr));
				outb(0x70, (baseAddr + mux1_addr));
				usleep(2000);

				//AD
				val = 0.0;
				for(j=0; j < REF_READ_COUNT; j++) {
					outb(0x00, (baseAddr + start_addr));
					usleep(2000);
					myVal.data[1] = inb(baseAddr + ad_haddr);
					myVal.data[0] = inb(baseAddr + ad_laddr);
					val += (double)myVal.val;
				}
				break;
		}

		val /= REF_READ_COUNT;
		if(val < 0) val -= 0.5;
		else val += 0.5;
   		if(i == 0) {
			vref.ref_P = (int)val;
		} else if(i == 1) {
			vref.ref_0 = (int)val;
		} else if(i == 2) {
			vref.ref_N = (int)val;
		}
		vref.data = 0;
	}
}
				
void Ref_I_Read(void)
{
    int	i, j, tmp;
	double val;
	U_ADDA myVal;

	for(i=0; i < 3; i++) {
		tmp = 0;
		switch(systemModel) {
			case LGCE_NJ_5V2A:
				//mux
				if(i == 0) {
					tmp = 0x10;
				} else if(i == 1) {
					tmp = 0x12;
				} else if(i == 2) {
					tmp = 0x11;
				} else {
					tmp = 0x13;
				}
				outb((unsigned char)tmp, (0x620 + mux1_addr));
				usleep(20000);

				//AD
				val = 0.0;
				for(j=0; j < REF_READ_COUNT; j++) {
					outb(0x03, (0x620 + start_addr));
					usleep(2000);
					myVal.data[1] = inb(0x620 + ad_haddr + 2);
					myVal.data[0] = inb(0x620 + ad_laddr + 2);
					val += (double)myVal.val;
				}
				break;
			case SW_DEMO_5V3A:
				//mux
				if(i == 0) {
					tmp = 0x50;
				} else if(i == 1) {
					tmp = 0x52;
				} else if(i == 2) {
					tmp = 0x51;
				} else {
					tmp = 0x53;
				}
				outb((unsigned char)tmp, (0x620 + mux1_addr));
				usleep(20000);

				//AD
				val = 0.0;
				for(j=0; j < REF_READ_COUNT; j++) {
					outb(0x03, (0x620 + start_addr));
					usleep(2000);
					if(group == 0) {
						myVal.data[1] = inb(0x620 + ad_haddr);
						myVal.data[0] = inb(0x620 + ad_laddr);
					} else {
						myVal.data[1] = inb(0x620 + ad_haddr + 2);
						myVal.data[0] = inb(0x620 + ad_laddr + 2);
					}
					val += (double)myVal.val;
				}
				break;
			default:
				//mux
				if(i == 0) {
					tmp = 0x10;
				} else if(i == 1) {
					tmp = 0x18;
				} else if(i == 2) {
					tmp = 0x14;
				} else {
					tmp = 0x1C;
				}
				outb((unsigned char)tmp, (baseAddr + mux2_addr));
				outb(0x70, (baseAddr + mux1_addr));
				usleep(2000);

				//AD
				val = 0.0;
				for(j=0; j < REF_READ_COUNT; j++) {
					outb(0x00, (baseAddr + start_addr));
					usleep(2000);
					myVal.data[1] = inb(baseAddr + ad_haddr);
					myVal.data[0] = inb(baseAddr + ad_laddr);
					val += (double)myVal.val;
				}
				break;
		}

		val /= REF_READ_COUNT;
		if(val < 0) val -= 0.5;
		else val += 0.5;
   		if(i == 0) {
			iref.ref_P = (int)val;
		} else if(i == 1) {
			iref.ref_0 = (int)val;
		} else if(i == 2) {
			iref.ref_N = (int)val;
		}
		iref.data = 0;
	}
}

void MainADC_Calibration(void)
{
	int i;
	double AD_a, AD_b, val;

	Ref_V_Read();
	Ref_I_Read();

	//AMP -> AD620A
	//G = 1 + 49.4kohm / 100kohm = 1.494
	//10V : 32767 = 5V : V1 -> V1 = 16383.5
	//val = V1 * ratio = 24476.949
	val = 24476.949;

	AD_a = (double)(val - (-val)) / (double)(vref.ref_P - vref.ref_N);
	AD_b = val - (double)vref.ref_P * AD_a;
	Vsource_AD_a = AD_a;
	Vsource_AD_b = AD_b;

	AD_a = (double)(val - (-val)) / (double)(iref.ref_P - iref.ref_N);
	AD_b = val - (double)iref.ref_P * AD_a;
	Isource_AD_a = AD_a;
	Isource_AD_b = AD_b;

	for(i=0; i < 3; i++) {
		if(i == 0) val = (double)vref.ref_P;
		else if(i == 1) val = (double)vref.ref_0;
		else val = (double)vref.ref_N;

		calSourceV[i] = (long)(val * Vsource_AD_a + Vsource_AD_b);

		if(i == 0) val = (double)iref.ref_P;
		else if(i == 1) val = (double)iref.ref_0;
		else val = (double)iref.ref_N;

		calSourceI[i] = (long)(val * Isource_AD_a + Isource_AD_b);
	}

	printf("vref(P):%d, vref(0):%d, vref(N):%d\n",
		vref.ref_P, vref.ref_0, vref.ref_N);
	printf("Vsource_AD a:%f, b:%f\n", Vsource_AD_a, Vsource_AD_b);
	printf("calSourceV %ld %ld %ld\n\n",
		calSourceV[0], calSourceV[1], calSourceV[2]);

	printf("iref(P):%d, iref(0):%d, iref(N):%d\n",
		iref.ref_P, iref.ref_0, iref.ref_N);
	printf("Isource_AD a:%f, b:%f\n", Isource_AD_a, Isource_AD_b);
	printf("calSourceI %ld %ld %ld\n\n",
		calSourceI[0], calSourceI[1], calSourceI[2]);
}

void SetMainDA_V(int val)
{
	U_ADDA myVal;

	myVal.val = (short int)val;
	switch(systemModel) {
		case LGCE_NJ_5V2A:
			outb(0x01, (0x620 + main_dac_sync_v));
			outb(myVal.data[1], (0x620 + main_dac_high));
			outb(myVal.data[0], (0x620 + main_dac_low_v));
			break;
		case SW_DEMO_5V3A:
			printf("test1 %x %d\n", myVal.val, myVal.val);
			outb(0x03, (0x620 + main_dac_sync_v));
			outb(myVal.data[1], (0x620 + main_dac_high));
			outb(myVal.data[0], (0x620 + main_dac_low_v));
			break;
		default:
			outb(myVal.data[1], (baseAddr + main_dac_high));
			outb(myVal.data[0], (baseAddr + main_dac_low_v));
			break;
	}
}

void SetMainDA_I(int val)
{
	U_ADDA myVal;

	myVal.val = (short int)val;
	switch(systemModel) {
		case LGCE_NJ_5V2A:
			outb(0x01, (0x620 + main_dac_sync_i));
			outb(myVal.data[1], (0x620 + main_dac_high));
			outb(myVal.data[0], (0x620 + main_dac_low_i));
			break;
		case SW_DEMO_5V3A:
			printf("test2 %x %d\n", myVal.val, myVal.val);
			outb(0x03, (0x620 + main_dac_sync_i));
			outb(myVal.data[1], (0x620 + main_dac_high));
			outb(myVal.data[0], (0x620 + main_dac_low_i));
			break;
		default:
			outb(myVal.data[1], (baseAddr + main_dac_high));
			outb(myVal.data[0], (baseAddr + main_dac_low_i));
			break;
	}
}

void SetMainDA_L(int val)
{
	U_ADDA myVal;

	myVal.val = (short int)val;
	switch(systemModel) {
		case SW_DEMO_5V3A:
			outb(0x03, (0x620 + main_dac_sync_l));
			outb(myVal.data[1], (0x620 + main_dac_high));
			outb(myVal.data[0], (0x620 + main_dac_low_l));
			break;
		default:
			break;
	}
}

int MainDA_V_Read(void)
{
    int	i, tmp;
	double val;
	U_ADDA myVal;

	switch(systemModel) {
		case SW_DEMO_5V3A:
			//mux
			outb(0xA2, (0x620 + mux1_addr));
			usleep(20000);

			//AD
			val = 0.0;
			for(i=0; i < CMD_READ_COUNT; i++) {
				outb(0x03, (0x620 + start_addr));
				usleep(2000);
				if(group == 0) {
					myVal.data[1] = inb(0x620 + ad_haddr);
					myVal.data[0] = inb(0x620 + ad_laddr);
				} else {
					myVal.data[1] = inb(0x620 + ad_haddr + 2);
					myVal.data[0] = inb(0x620 + ad_laddr + 2);
				}
				val += (double)myVal.val;
			}
			break;
		default:
			//mux
			tmp = 0x13;
			outb((unsigned char)tmp, (0x620 + mux1_addr));
			usleep(20000);

			//AD
			val = 0.0;
			for(i=0; i < CMD_READ_COUNT; i++) {
				myVal.val = 0;
				outb(0x03, (0x620 + start_addr));
				usleep(2000);
				myVal.data[1] = inb(0x620 + ad_haddr);
				myVal.data[0] = inb(0x620 + ad_laddr);
				val += (double)myVal.val;
			}
			break;
	}

	val /= CMD_READ_COUNT;
	if(val < 0) val -= 0.5;
	else val += 0.5;

	return (int)val;
}

int MainDA_I_Read(void)
{
    int	i, tmp;
	double val;
	U_ADDA myVal;

	switch(systemModel) {
		case SW_DEMO_5V3A:
			//mux
			outb(0xA3, (0x620 + mux1_addr));
			usleep(20000);

			//AD
			val = 0.0;
			for(i=0; i < CMD_READ_COUNT; i++) {
				outb(0x03, (0x620 + start_addr));
				usleep(2000);
				if(group == 0) {
					myVal.data[1] = inb(0x620 + ad_haddr);
					myVal.data[0] = inb(0x620 + ad_laddr);
				} else {
					myVal.data[1] = inb(0x620 + ad_haddr + 2);
					myVal.data[0] = inb(0x620 + ad_laddr + 2);
				}
				val += (double)myVal.val;
			}
			break;
		default:
			//mux
			tmp = 0x13;
			outb((unsigned char)tmp, (0x620 + mux1_addr));
			usleep(20000);

			//AD
			val = 0.0;
			for(i=0; i < CMD_READ_COUNT; i++) {
				outb(0x03, (0x620 + start_addr));
				usleep(2000);
				myVal.data[1] = inb(0x620 + ad_haddr + 2);
				myVal.data[0] = inb(0x620 + ad_laddr + 2);
				val += (double)myVal.val;
			}
			break;
	}

	val /= CMD_READ_COUNT;
	if(val < 0) val -= 0.5;
	else val += 0.5;

	return (int)val;
}

void MainDAC_Calibration(void)
{
	int i, val, cmd_dav[3], cmd_dai[3];
	double tmp1, tmp2, value;

/*	cmd_dav[0] = 16383;
	cmd_dav[1] = 0;
	cmd_dav[2] = -16384;*/
	cmd_dav[0] = 13763; // 4.2/10*32768 = 13762.56
	cmd_dav[1] = 8847; // 2.7/10*32768 = 8847.36
	cmd_dav[2] = -13764;

	cmd_dai[0] = 16383;
	cmd_dai[1] = 0;
	cmd_dai[2] = -16384;

	for(i=0; i < 3; i++) {
		SetMainDA_V(cmd_dav[i]);
		usleep(20000);

		val = MainDA_V_Read();
		cmd_adv_org[group][i] = val;
		cmd_adv[group][i] = ((double)val / 1.494 * Vsource_AD_a + Vsource_AD_b);

		SetMainDA_I(cmd_dai[i]);
		usleep(20000);

		val = MainDA_I_Read();
		cmd_adi_org[group][i] = val;
		cmd_adi[group][i] = ((double)val / 1.494 * Isource_AD_a + Isource_AD_b);
	}

	dac_value[group].V_P = cmd_adv_org[group][0];
	dac_value[group].V_N = cmd_adv_org[group][2];
	dac_value[group].I_P = cmd_adi_org[group][0];
	dac_value[group].I_N = cmd_adi_org[group][2];

	printf("cmd_adv_org[%d] %d %d %d\n", group,
		cmd_adv_org[group][0], cmd_adv_org[group][1], cmd_adv_org[group][2]);
	printf("cmd_adv[%d] %f %f %f\n", group,
		cmd_adv[group][0], cmd_adv[group][1], cmd_adv[group][2]);
	printf("cmd_adi_org[%d] %d %d %d\n", group,
		cmd_adi_org[group][0], cmd_adi_org[group][1], cmd_adi_org[group][2]);
	printf("cmd_adi[%d] %f %f %f\n\n", group,
		cmd_adi[group][0], cmd_adi[group][1], cmd_adi[group][2]);

	tmp1 = (double)cmd_dav[0] - (double)cmd_dav[1];
	tmp2 = cmd_adv[group][0] - cmd_adv[group][1];
	cmdV_DA_a[0] = tmp1 / tmp2;
	cmdV_DA_b[0] = (double)cmd_dav[0] - cmd_adv[group][0] * cmdV_DA_a[0];

	tmp1 = (double)cmd_dai[0] - (double)cmd_dai[2];
	tmp2 = cmd_adi[group][0] - cmd_adi[group][2];
	cmdI_DA_a[0] = tmp1 / tmp2;
	cmdI_DA_b[0] = (double)cmd_dai[0] - cmd_adi[group][0] * cmdI_DA_a[0];

	for(i=0; i < 3; i++) {
		value = (double)cmd_dav[i] * cmdV_DA_a[0] + cmdV_DA_b[0];
		if(value < 0) value -= 0.5;
		else value += 0.5;
		val = (int)value;
		SetMainDA_V(val);

		val = MainDA_V_Read();
		cmd_adv[group][i] = ((double)val / 1.494 * Vsource_AD_a + Vsource_AD_b);

		value = (double)cmd_dai[i] * cmdI_DA_a[0] + cmdI_DA_b[0];
		if(value < 0) value -= 0.5;
		else value += 0.5;
		val = (int)value;
		SetMainDA_I(val);
		usleep(20000);

		val = MainDA_I_Read();
		cmd_adi[group][i] = ((double)val / 1.494 * Isource_AD_a + Isource_AD_b);
	}

	printf("cmdV_DA_a a:%f, b:%f\n", cmdV_DA_a[0], cmdV_DA_b[0]);
	printf("cmd_adv[%d] %f %f %f\n\n", group,
		cmd_adv[group][0], cmd_adv[group][1], cmd_adv[group][2]);

	printf("cmdI_DA a:%f, b:%f\n", cmdI_DA_a[0], cmdI_DA_b[0]);
	printf("cmd_adi[%d] %f %f %f\n\n", group,
		cmd_adi[group][0], cmd_adi[group][1], cmd_adi[group][2]);
}

void MainDAC_Calibration2(int type)
{
	int i, val, cmd_dav[3], cmd_dai[3], cmd_dal[3];
	long real;
	double tmp1, tmp2, value;

/*	cmd_dav[0] = 16383;
	cmd_dav[1] = 0;
	cmd_dav[2] = -16384;*/
	cmd_dav[0] = 13763; // 4.2/10*32768 = 13762.56
	cmd_dav[1] = 8847; // 2.7/10*32768 = 8847.36
	cmd_dav[2] = -13764;

	cmd_dai[0] = 16383;
	cmd_dai[1] = 0;
	cmd_dai[2] = -16384;

	cmd_dal[0] = 16383;
	cmd_dal[1] = 0;
	cmd_dal[2] = -16384;

	for(i=0; i < 3; i++) {
		if(type == CALI_VOLTAGE) {
			SetMainDA_V(cmd_dav[i]);
		} else if(type == CALI_CURRENT) {
			SetMainDA_I(cmd_dai[i]);
		} else {
			SetMainDA_L(cmd_dal[i]);
		}
		usleep(200000);

		real = RealData();
		if(type == CALI_VOLTAGE) {
			//printf("Meter value DAC_V(%d) : %ld\n", i, real);
			cmd_adv_org[group][i] = (int)real;
			cmd_adv[group][i] = (double)real * 0.0032768;
		} else if(type == CALI_CURRENT) {
			//printf("Meter value DAC_I(%d) : %ld\n", i, real);
			cmd_adi_org[group][i] = (int)real;
			cmd_adi[group][i] = (double)real * 0.0032768;
		} else {
			//printf("Meter value DAC_L(%d) : %ld\n", i, real);
			cmd_adl_org[group][i] = (int)real;
			cmd_adl[group][i] = (double)real * 0.0032768;
		}
	}

	if(type == CALI_VOLTAGE) {
		dac_value[group].V_P = cmd_adv_org[group][0];
		dac_value[group].V_N = cmd_adv_org[group][2];
		printf("cmd_adv_org[%d] %f %f %f\n", group,
			cmd_adv_org[group-1][0]/1000.0,
			cmd_adv_org[group-1][1]/1000.0,
			cmd_adv_org[group-1][2]/1000.0);
		//printf("cmd_adv %f %f %f\n",
		//	cmd_adv[0], cmd_adv[1], cmd_adv[2]);

		tmp1 = (double)cmd_dav[0] - (double)cmd_dav[1];
		tmp2 = cmd_adv[group][0] - cmd_adv[group][1];
		cmdV_DA_a[group] = tmp1 / tmp2;
		cmdV_DA_b[group] = (double)cmd_dav[0]
			- cmd_adv[group][0] * cmdV_DA_a[group];
	} else if(type == CALI_CURRENT) {
		dac_value[group].I_P = cmd_adi_org[group][0];
		dac_value[group].I_N = cmd_adi_org[group][2];
		printf("cmd_adi_org[%d] %f %f %f\n", group,
			cmd_adi_org[group][0]/1000.0,
			cmd_adi_org[group][1]/1000.0,
			cmd_adi_org[group][2]/1000.0);
		//printf("cmd_adi %f %f %f\n\n",
		//	cmd_adi[0], cmd_adi[1], cmd_adi[2]);

		tmp1 = (double)cmd_dai[0] - (double)cmd_dai[2];
		tmp2 = cmd_adi[group][0] - cmd_adi[group][2];
		cmdI_DA_a[group] = tmp1 / tmp2;
		cmdI_DA_b[group] = (double)cmd_dai[0]
			- cmd_adi[group][0] * cmdI_DA_a[group];
	} else {
		//kjg_081003 dac_value[group].L_P = cmd_adl_org[group][0];
		//kjg_081003 dac_value[group].L_N = cmd_adl_org[group][2];
		printf("cmd_adl_org[%d] %f %f %f\n", group,
			cmd_adl_org[group][0]/1000.0,
			cmd_adl_org[group][1]/1000.0,
			cmd_adl_org[group][2]/1000.0);
		//printf("cmd_adl %f %f %f\n\n",
		//	cmd_adl[0], cmd_adl[1], cmd_adl[2]);

		tmp1 = (double)cmd_dal[0] - (double)cmd_dal[2];
		tmp2 = cmd_adl[group][0] - cmd_adl[group][2];
		cmdL_DA_a[group] = tmp1 / tmp2;
		cmdL_DA_b[group] = (double)cmd_dal[0]
			- cmd_adl[group][0] * cmdL_DA_a[group];
	}

	for(i=0; i < 3; i++) {
		if(type == CALI_VOLTAGE) {
			value = (double)cmd_dav[i] * cmdV_DA_a[group]
				+ cmdV_DA_b[group];
			if(value < 0) value -= 0.5;
			else value += 0.5;
			val = (int)value;
			SetMainDA_V(val);
		} else if(type == CALI_CURRENT) {
			value = (double)cmd_dai[i] * cmdI_DA_a[group]
				+ cmdI_DA_b[group];
			if(value < 0) value -= 0.5;
			else value += 0.5;
			val = (int)value;
			SetMainDA_I(val);
		} else {
			value = (double)cmd_dal[i] * cmdL_DA_a[group]
				+ cmdL_DA_b[group];
			if(value < 0) value -= 0.5;
			else value += 0.5;
			val = (int)value;
			SetMainDA_L(val);
		}
		usleep(200000);

		real = RealData();
		if(type == CALI_VOLTAGE) {
			//printf("Meter value DAC_V2(%d) : %ld\n", i, real);
			cmd_adv[group][i] = (double)real;
		} else if(type == CALI_CURRENT) {
			//printf("Meter value DAC_I2(%d) : %ld\n", i, real);
			cmd_adi[group][i] = (double)real;
		} else {
			//printf("Meter value DAC_L2(%d) : %ld\n", i, real);
			cmd_adl[group][i] = (double)real;
		}
	}

	if(type == CALI_VOLTAGE) {
		printf("cmdV_DA_a[%d] a:%f, b:%f\n", group,
			cmdV_DA_a[group], cmdV_DA_b[group]);
		printf("cmd_adv[%d] %f %f %f\n\n", group,
			cmd_adv[group][0]/1000.0,
			cmd_adv[group][1]/1000.0,
			cmd_adv[group][2]/1000.0);
	} else if(type == CALI_CURRENT) {
		printf("cmdI_DA[%d] a:%f, b:%f\n", group,
			cmdI_DA_a[group], cmdI_DA_b[group]);
		printf("cmd_adi[%d] %f %f %f\n\n", group,
			cmd_adi[group][0]/1000.0,
			cmd_adi[group][1]/1000.0,
			cmd_adi[group][2]/1000.0);
	} else {
		printf("cmdL_DA[%d] a:%f, b:%f\n", group,
			cmdL_DA_a[group], cmdL_DA_b[group]);
		printf("cmd_adl[%d] %f %f %f\n\n", group,
			cmd_adl[group][0]/1000.0,
			cmd_adl[group][1]/1000.0,
			cmd_adl[group][2]/1000.0);
	}
}

void MainDAC_CaliData_Write(int type)
{
    FILE *fp;

	if(type == CALI_VOLTAGE) {
		if(group == 0) {
			system("rm mainDA_V_CaliData1");
		    if((fp = fopen("mainDA_V_CaliData1", "a")) == NULL) {
				printf("mainDA_V1 write error\n");
				return;
			}
		} else {
			system("rm mainDA_V_CaliData2");
		    if((fp = fopen("mainDA_V_CaliData2", "a")) == NULL) {
				printf("mainDA_V2 write error\n");
				return;
			}
		}
	} else if(type == CALI_CURRENT) {
		if(group == 0) {
			system("rm mainDA_I_CaliData1");
		    if((fp = fopen("mainDA_I_CaliData1", "a")) == NULL) {
				printf("mainDA_I1 write error\n");
				return;
			}
		} else {
			system("rm mainDA_I_CaliData2");
		    if((fp = fopen("mainDA_I_CaliData2", "a")) == NULL) {
				printf("mainDA_I2 write error\n");
				return;
			}
		}
	} else {
		if(group == 0) {
			system("rm mainDA_L_CaliData1");
		    if((fp = fopen("mainDA_L_CaliData1", "a")) == NULL) {
				printf("mainDA_L1 write error\n");
				return;
			}
		} else {
			system("rm mainDA_L_CaliData2");
		    if((fp = fopen("mainDA_L_CaliData2", "a")) == NULL) {
				printf("mainDA_L2 write error\n");
				return;
			}
		}
	}

	if(type == CALI_VOLTAGE) {
		fprintf(fp, "MainDA_V%d\n", group+1);
		fprintf(fp, "org_meter %d %d %d\n", cmd_adv_org[group][0],
			cmd_adv_org[group][1], cmd_adv_org[group][2]);
		fprintf(fp, "org_da_value %d %d %d\n",
			(int)((double)cmd_adv_org[group][0] * 0.0032768),
			(int)((double)cmd_adv_org[group][1] * 0.0032768),
			(int)((double)cmd_adv_org[group][2] * 0.0032768));
		fprintf(fp, "cal_meter %f %f %f\n", cmd_adv[group][0] * 0.0032768,
			cmd_adv[group][1] * 0.0032768, cmd_adv[group][2] * 0.0032768);
		fprintf(fp, "cal_da_value %d %d %d\n", (int)(cmd_adv[group][0]),
			(int)(cmd_adv[group][1]), (int)(cmd_adv[group][2]));
		fprintf(fp, "DA_A %f\n", cmdV_DA_a[group]);
		fprintf(fp, "DA_B %f\n", cmdV_DA_b[group]);
	} else if(type == CALI_CURRENT) {
		fprintf(fp, "MainDA_I%d\n", group+1);
		fprintf(fp, "org_meter %d %d %d\n", cmd_adi_org[group][0],
			cmd_adi_org[group][1], cmd_adi_org[group][2]);
		fprintf(fp, "org_da_value %d %d %d\n",
			(int)((double)cmd_adi_org[group][0] * 0.0032768),
			(int)((double)cmd_adi_org[group][1] * 0.0032768),
			(int)((double)cmd_adi_org[group][2] * 0.0032768));
		fprintf(fp, "cal_meter %f %f %f\n", cmd_adi[group][0] * 0.0032768,
			cmd_adi[group][1] * 0.0032768, cmd_adi[group][2] * 0.0032768);
		fprintf(fp, "cal_da_value %f %f %f\n",
			cmd_adi[group][0], cmd_adi[group][1], cmd_adi[group][2]);
		fprintf(fp, "DA_A %f\n", cmdI_DA_a[group]);
		fprintf(fp, "DA_B %f\n", cmdI_DA_b[group]);
	} else {
		fprintf(fp, "MainDA_L%d\n", group+1);
		fprintf(fp, "org_meter %d %d %d\n", cmd_adl_org[group][0],
			cmd_adl_org[group][1], cmd_adl_org[group][2]);
		fprintf(fp, "org_da_value %d %d %d\n",
			(int)((double)cmd_adl_org[group][0] * 0.0032768),
			(int)((double)cmd_adl_org[group][1] * 0.0032768),
			(int)((double)cmd_adl_org[group][2] * 0.0032768));
		fprintf(fp, "cal_meter %f %f %f\n", cmd_adl[group][0] * 0.0032768,
			cmd_adl[group][1] * 0.0032768, cmd_adl[group][2] * 0.0032768);
		fprintf(fp, "cal_da_value %f %f %f\n", cmd_adl[group][0],
			cmd_adl[group][1], cmd_adl[group][2]);
		fprintf(fp, "DA_A %f\n", cmdL_DA_a[group]);
		fprintf(fp, "DA_B %f\n", cmdL_DA_b[group]);
	}

	fclose(fp);
}

void MainDAC_CaliData_Read(int type)
{
	char buf[24];
	int tmp, i;
    FILE *fp;

	if(type == CALI_VOLTAGE) {
		if(group == 0) {
		    if((fp = fopen("mainDA_V_CaliData1", "r")) == NULL) {
				printf("mainDA_V1 read error\n");
				return;
			}
		} else {
		    if((fp = fopen("mainDA_V_CaliData2", "r")) == NULL) {
				printf("mainDA_V2 read error\n");
				return;
			}
		}
	} else if(type == CALI_CURRENT) {
		if(group == 0) {
		    if((fp = fopen("mainDA_I_CaliData1", "r")) == NULL) {
				printf("mainDA_I1 read error\n");
				return;
			}
		} else {
		    if((fp = fopen("mainDA_I_CaliData2", "r")) == NULL) {
				printf("mainDA_I2 read error\n");
				return;
			}
		}
	} else {
		if(group == 0) {
		    if((fp = fopen("mainDA_L_CaliData1", "r")) == NULL) {
				printf("mainDA_L1 read error\n");
				return;
			}
		} else {
		    if((fp = fopen("mainDA_L_CaliData2", "r")) == NULL) {
				printf("mainDA_L2 read error\n");
				return;
			}
		}
	}

	for(i=0; i < 18; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
	}

	if(type == CALI_VOLTAGE) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		cmdV_DA_a[group] = (double)atof(buf);

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		cmdV_DA_b[group] = (double)atof(buf);

		printf("V_DA_A %f, V_DA_B %f\n", cmdV_DA_a[group], cmdV_DA_b[group]);
	} else if(type == CALI_CURRENT) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		cmdI_DA_a[group] = (double)atof(buf);

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		cmdI_DA_b[group] = (double)atof(buf);

		printf("I_DA_A %f, I_DA_B %f\n", cmdI_DA_a[group], cmdI_DA_b[group]);
	} else {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		cmdL_DA_a[group] = (double)atof(buf);

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		cmdL_DA_b[group] = (double)atof(buf);

		printf("L_DA_A %f, L_DA_B %f\n", cmdL_DA_a[group], cmdL_DA_b[group]);
	}

	fclose(fp);
}

int ReadCommand(int fd, int *cmd, int *val)
{
    int	rtn;

    rtn = readn(fd, (char *)cmd, sizeof(int));
    if(rtn != sizeof(int)) {
		printf("Client connection fail1 %d\n", rtn);
		CloseFD(fd);
		server_state = 0;
		return -1;
    }

  	if(server_state == 0) return 0;

    rtn = readn(fd, (char *)val, sizeof(int));
    if(rtn != sizeof(int)) {
		printf("Client connection fail2 %d\n", rtn);
		CloseFD(fd);
		server_state = 0;
		return -2;
    }

    return 1;
}

int DoRead(int fd)
{
    int	rtn, cmd, val, i, stdev, tmp;
	float ave;
	long real;
	double value;
	U_ADDA myVal;

	AVSTDEV avstdev;

    if(iopl(3)) exit(1);

	if(ReadCommand(fd, &cmd, &val) < 0) return 0;

	switch(cmd) {
		case 1:
			printf("Calibration Client connected\n");
			server_state = 1;
			write_ACK(fd);
			break;
    	case 2: //main b'd select
			printf("Set main board number : %d\n", val);
			MainBdNum = val - 1;
			switch(systemModel) {
				case LGCE_NJ_5V2A:
					baseAddr = 0x630 + (0x10 * MainBdNum);
					break;
				case SW_DEMO_5V3A:
					baseAddr = 0x700 + (0x10 * MainBdNum);
					break;
				default:
					baseAddr = 0x620 + (0x10 * MainBdNum);
					break;
			}

			MainADC_Calibration();
			MainDAC_Calibration();

			SetAuxDA_V(-1, 0);
			SetAuxDA_I(-1, 0);
			write_ACK(fd);
			break;
    	case 3:  
			printf("Set current dac : %d\n", val);
			switch(systemModel) {
				case LGCE_NJ_5V2A:
					value = (double)val * cmdI_DA_a[0] + cmdI_DA_b[0];
					if(value < 0) value -= 0.5;
					else value += 0.5;
					val = (int)value;
					SetMainDA_I(val);
					break;
				case SW_DEMO_5V3A:
					//charge/discharge mode select
					if(val >= 0.0) {
						outb(0x00, baseAddr + 1);
						SetMainDA_L(19988);
					} else {
						outb(0x01, baseAddr + 1);
						SetMainDA_L(327);
					}

					value = (double)val * cmdI_DA_a[0] + cmdI_DA_b[0];

					if(val >= 0.0) {
						value = value * 1.1695 + 3024.4864;
					} else {
						value = value * (-0.9896) + 5570.56;
					}

					if(value < 0) value -= 0.5;
					else value += 0.5;
					val = (int)value;
					SetMainDA_I(val);
					break;
				default:
					SetMainDA_I(val);
					break;
			}
			write_ACK(fd);
			break;
    	case 4:
			printf("Set voltage dac : %d\n", val);
			switch(systemModel) {
				case LGCE_NJ_5V2A:
					value = (double)val * cmdV_DA_a[0] + cmdV_DA_b[0];
					//value = (double)val;
					if(value < 0) value -= 0.5;
					else value += 0.5;
					val = (int)value;
					SetMainDA_V(val);
					break;
				case SW_DEMO_5V3A:
					value = (double)val * cmdV_DA_a[0] + cmdV_DA_b[0];

					value = value * 1.0121 + 8159.232;
					if(value < 0) value -= 0.5;
					else value += 0.5;
					val = (int)value;
					SetMainDA_V(val);
					break;
				default:
					SetMainDA_V(val);
					break;
			}
			write_ACK(fd);
			break;
    	case 5:
			printf("Set scan  channel : %d\n", val+1);
			write_ACK(fd);
			break;
	    case 6: //relay, tray switch on
			if(scanType == SCAN_CURRENT) {
				printf("Set relay on : %d\n", val+1);
				RelayOnOff(val, ON);
				if(calbox == CAL_BOX_SBC_CTRL) {
					printf("TraySwitch:On, BdNum:%d, ChNo:%d\n",
						MainBdNum+1, val+1);
					//Calibrator Fixture B'd S/W On
					TraySwitchOnOff(ON, MainBdNum, val);
				}
			} else if(scanType == SCAN_VOLTAGE) {
				if(calbox == CAL_BOX_NONUSE) {
					printf("Set relay on : %d\n", val+1);
					RelayOnOff(val, ON);
				} else if(calbox == CAL_BOX_SBC_CTRL) {
					printf("TraySwitch:On, BdNum:%d, ChNo:%d\n",
						MainBdNum+1, val+1);
					//Calibrator Fixture B'd S/W On
					TraySwitchOnOff(ON, MainBdNum, val);
				}
			}
			usleep(100000);
			write_ACK(fd);
			break;
	    case 7: //relay, tray switch off
			if(scanType == CALI_CURRENT) {
				printf("Set relay off : %d\n", val+1);
				RelayOnOff(val, OFF);
				if(calbox == CAL_BOX_SBC_CTRL) {
					printf("TraySwitch:Off, BdNum:%d, ChNo:%d\n",
						MainBdNum+1, val+1);
					//Calibrator Fixture B'd S/W Off
					TraySwitchOnOff(OFF, MainBdNum, val);
				}
			} else if(scanType == SCAN_VOLTAGE) {
				if(calbox == CAL_BOX_NONUSE) {
					printf("Set relay off : %d\n", val+1);
					RelayOnOff(val, OFF);
				} else if(calbox == CAL_BOX_SBC_CTRL) {
					printf("TraySwitch:Off, BdNum:%d, ChNo:%d\n",
						MainBdNum+1, val+1);
					//Calibrator Fixture B'd S/W Off
					TraySwitchOnOff(OFF, MainBdNum, val);
				}
			}
			write_ACK(fd);
			break;
	    case 8: //scan type select
			scanType = val;
			if(val == 0) {
				printf("Set scan type : VOLTAGE\n");
			} else {
				printf("Set scan type : CURRENT\n");
			}

			if(calbox == CAL_BOX_SBC_CTRL) {
				ScanTypeSetting(val);	
			} else if(calbox == CAL_BOX_NONUSE) {
				switch(systemModel) {
					case LGCE_NJ_5V2A:
						if(scanType == SCAN_VOLTAGE) {
							outb(0x00, 0x612); //CC/CV Relay Togle
						} else { //SCAN_CURRENT
							outb(0x0C, 0x612); //CC/CV Relay Togle
						}
						break;
					default:
						if(scanType == SCAN_VOLTAGE) {
							outb(0x00, 0x610); //CC/CV Relay Togle
						} else { //SCAN_CURRENT
							outb(0x01, 0x610); //CC/CV Relay Togle
						}
						break;
				}
			}

			if(scanType == SCAN_VOLTAGE && calbox != CAL_BOX_NONUSE) {
				RelayAllOn(); //H/W Bugger control only for LG 70set serise
			} else {
				RelayAllOff(); //H/W Bugger control only for LG 70set serise
			}
			write_ACK(fd);
			break;
	    case 9: //mux select
			Channel = val;
			printf("Set mux : %d %d\n", val+1, scanType);
			SetMux(val, scanType);
			write_ACK(fd);
			break;
	    case 10: //Send AD value(low accuracy) 
			write_ACK(fd);
			tmp = 0;
			for(i=0; i < CH_READ_COUNT1; i++) {
				tmp += tmpValue[i];
			}
			ave = (float)tmp / CH_READ_COUNT1;
			stdev = AD_stdev(ave, CH_READ_COUNT1);
			if(ave < 0) {
				avstdev.ave = (int)(ave - 0.5);
			} else {
				avstdev.ave = (int)(ave + 0.5);
			}
			avstdev.stdev = stdev;
			avstdev.data1 = 0;
			avstdev.data2 = 0;
   			printf("AD Average : %d , AD STDEV %d: \n", (int)ave, stdev);
   			rtn = writen(fd, (char *)&avstdev, sizeof(AVSTDEV));
   			if(rtn != sizeof(AVSTDEV))
				printf("ERROR: AD Average don't send\n");
			break;
    	case 11:
			printf("AD Start (high accuracy)\n");
			for(i=0; i < CH_READ_COUNT2; i++) {
				switch(systemModel) {
					case LGCE_NJ_5V2A:
						outb(0x03, (0x620 + start_addr));
						usleep(1000);
						myVal.data[1] = inb(0x620 + ad_haddr + scanType * 2);
						myVal.data[0] = inb(0x620 + ad_laddr + scanType * 2);
						if(scanType == SCAN_VOLTAGE) {
							value = (double)myVal.val
								* Vsource_AD_a + Vsource_AD_b;
						} else { //SCAN_CURRENT
							value = (double)myVal.val
								* Isource_AD_a + Isource_AD_b;
						}
						if(value < 0) {
							myVal.val = (short int)(value - 0.5);
						} else {
							myVal.val = (short int)(value + 0.5);
						}
						break;
					case SW_DEMO_5V3A:
						outb(0x03, (0x620 + start_addr));
						usleep(1000);
						if(group == 0) {
							myVal.data[1] = inb(0x620 + ad_haddr);
							myVal.data[0] = inb(0x620 + ad_laddr);
						} else {
							myVal.data[1] = inb(0x620 + ad_haddr + 2);
							myVal.data[0] = inb(0x620 + ad_laddr + 2);
						}
						if(scanType == SCAN_VOLTAGE) {
							value = (double)myVal.val
								* Vsource_AD_a + Vsource_AD_b;
						} else { //SCAN_CURRENT
							value = (double)myVal.val
								* Isource_AD_a + Isource_AD_b;
						}
						if(value < 0) {
							myVal.val = (short int)(value - 0.5);
						} else {
							myVal.val = (short int)(value + 0.5);
						}
						break;
					default:
						outb(0x00, (baseAddr + start_addr));
						usleep(1000);
						myVal.data[1] = inb(baseAddr + ad_haddr);
						myVal.data[0] = inb(baseAddr + ad_laddr);
						break;
				}
				tmpValue[i] = myVal.val;
			}
			CreatADFile(CH_READ_COUNT2, scanType); //for AD Value confirmation
			write_ACK(fd);
			break;
	    case 12:
			auxChNum = 0;
			printf("Set channel number = 0 for AUX DA_I\n");
			write_ACK(fd);
			break;
	    case 13:
			SetAuxDA_I(auxChNum, (unsigned short)val);
			printf("Set AUX DA_I ch:%d, val:%d\n", auxChNum+1, val);
			auxChNum++;
			write_ACK(fd);
			break;
		case 14: //Read meter value
			write_ACK(fd);
			if(meter == METER_SBC_CTRL) {
				real = RealData();
				printf("Meter value : %ld\n", real);
				rtn = writen(fd, (char *)&real, sizeof(long));
				if(rtn != sizeof(long))
			   		printf("Error : Meter value don't send: %ld\n", real);
			} else { //METER_PC_CTRL
				real = 0;
				rtn = writen(fd, (char *)&real, sizeof(long));
			}
			break;
		case 15: //send AD average
			write_ACK(fd);
			tmp = 0;
			for(i=0; i < CH_READ_COUNT2; i++) {
				tmp += tmpValue[i];
			}
			ave = (float)tmp / CH_READ_COUNT2;
			stdev = AD_stdev(ave, CH_READ_COUNT2);
			if(ave < 0) {
				avstdev.ave = (int)(ave - 0.5);
			} else {
				avstdev.ave = (int)(ave + 0.5);
			}
			avstdev.stdev = stdev;
			avstdev.data1 = 0;
			avstdev.data2 = 0;

   			printf("AD Average : %d , AD STDEV %d: \n", (int)ave, stdev);
   			rtn = writen(fd, (char *)&avstdev, sizeof(AVSTDEV));
   			if(rtn != sizeof(AVSTDEV))
   				printf("ERROR: AD Average don't send\n");
			break;
		case 16: //Meter Control select
			switch(val) {
				case 0:
					printf("Meter controled by PC\n");
					meter = METER_PC_CTRL;
					break;
				case 1:
					printf("Meter controled by SBC\n");
					meter = METER_SBC_CTRL;
					txsettings1("*RST\n", 0);
					txsettings1("SYST:REM\n", 0);
					txsettings1("CONF:VOLT:DC DEF,DEF\n", 0);
					txsettings1("SAMPLE:COUNT 1\n", 0);
					break;
				default:
					printf("Meter controled by PC\n");
					meter = METER_PC_CTRL;
					break;
			}	
			write_ACK(fd);
			break;
		case 17: //Calibration Control Box select
			switch(val) {
				case 0:
					printf("Unused Calibration Box \n");
					calbox = CAL_BOX_NONUSE;
					break;
				case 1:
					printf("Calibration Box controled by SBC\n");
					calbox = CAL_BOX_SBC_CTRL;
					break;
				case 2:
					printf("Calibration Box controled by PC\n");
					calbox = CAL_BOX_PC_CTRL;
					break;
				default:
					printf("Unused Calibration Box\n");
					calbox = CAL_BOX_NONUSE;
					break;
			}
			write_ACK(fd);
			break;
		case 18: //Reset
			//ResetTraySwitch();
			RelayAllOff();
			SetAuxDA_V(-1, 0);
			SetAuxDA_I(-1, 0);
			VoltDACReset();
			CurrDACReset();

			system("rm -rf tmp/*");
			write_ACK(fd);
			break;
		case 19:
			switch(systemModel) {
				case LGCE_NJ_5V2A:
				case SW_DEMO_5V3A:
					tmp = 0x1003; break;
				default:
					tmp = 0x1002; break;
			}
			printf("This program version is : %x\n", tmp); 
			writen(fd, (char *)&tmp, sizeof(int));
			break;
    	case 20:
			printf("AD Start (low accuracy)\n");
			for(i=0; i < CH_READ_COUNT1; i++) {
				switch(systemModel) {
					case LGCE_NJ_5V2A:
						outb(0x03, (0x620 + start_addr));
						usleep(2000);
						myVal.data[1]
							= inb(0x620 + ad_haddr + scanType * 2);
						myVal.data[0]
							= inb(0x620 + ad_laddr + scanType * 2);
						if(scanType == SCAN_VOLTAGE) {
							value = (double)myVal.val
								* Vsource_AD_a + Vsource_AD_b;
						} else { //SCAN_CURRENT
							value = (double)myVal.val
								* Isource_AD_a + Isource_AD_b;
						}
						if(value < 0) {
							myVal.val = (short int)(value - 0.5);
						} else {
							myVal.val = (short int)(value + 0.5);
						}
						break;
					case SW_DEMO_5V3A:
						outb(0x03, (0x620 + start_addr));
						usleep(2000);
						if(group == 0) {
							myVal.data[1] = inb(0x620 + ad_haddr);
							myVal.data[0] = inb(0x620 + ad_laddr);
						} else {
							myVal.data[1] = inb(0x620 + ad_haddr + 2);
							myVal.data[0] = inb(0x620 + ad_laddr + 2);
						}
						if(scanType == SCAN_VOLTAGE) {
							value = (double)myVal.val
								* Vsource_AD_a + Vsource_AD_b;
						} else { //SCAN_CURRENT
							value = (double)myVal.val
								* Isource_AD_a + Isource_AD_b;
						}
						if(value < 0) {
							myVal.val = (short int)(value - 0.5);
						} else {
							myVal.val = (short int)(value + 0.5);
						}
						break;
					default:
						outb(0x00, (baseAddr + start_addr));
						usleep(2000);
						myVal.data[1] = inb(baseAddr + ad_haddr);
						myVal.data[0] = inb(baseAddr + ad_laddr);
						break;
				}
				tmpValue[i] = myVal.val;
			}
			CreatADFile(CH_READ_COUNT1, scanType); //for AD Value confirmation
			write_ACK(fd);
			break;
    	case 21:
			write_ACK(fd);
			switch(systemModel) {
				case LGCE_NJ_5V2A:
				case SW_DEMO_5V3A:
					break;
				default:
					Ref_V_Read();
					break;
			}
				
   			printf("Reference V --> P:%d, 0:%d, N:%d\n",
				vref.ref_P, vref.ref_0, vref.ref_N);
   			rtn = writen(fd, (char *)&vref, sizeof(REF));
   			if(rtn != sizeof(REF)) printf("ERROR: Reference V don't send \n");
			break;
    	case 22:
			write_ACK(fd);
			switch(systemModel) {
				case LGCE_NJ_5V2A:
				case SW_DEMO_5V3A:
					break;
				default:
					Ref_I_Read();
					break;
			}

   			printf("Reference I-> P:%d, 0:%d, N:%d\n",
				iref.ref_P, iref.ref_0, iref.ref_N);
   			rtn = writen(fd, (char *)&iref, sizeof(REF));
   			if(rtn != sizeof(REF)) printf("ERROR: Reference I don't send \n");
			break;
		case 23: //DAC READ
			write_ACK(fd);

   			printf("DAC_VALUE V_P:%d, V_N:%d, I_P:%d, I_N:%d\n",
				dac_value[0].V_P, dac_value[0].V_N,
				dac_value[0].I_P, dac_value[0].I_N);
   			rtn = writen(fd, (char *)&dac_value, sizeof(DAC_VALUE));
   			if(rtn != sizeof(DAC_VALUE))
				printf("ERROR: DAC_VALUE I don't send \n");
			break;
	    case 24:
			auxChNum = 0;
			printf("Set channel number = 0 for AUX DA_V\n");
			write_ACK(fd);
			break;
	    case 25:
			SetAuxDA_V(auxChNum, (unsigned short)val);
			printf("Set AUX DA_V ch:%d, val:%d\n", auxChNum+1, val);
			auxChNum++;
			write_ACK(fd);
			break;
		default:
			printf("rcv cmd:%d, val:%d\n", cmd, val);
			write_ACK(fd);
			break;
	}

    return 1;
}
	
int main(int argc, char *argv[])
{
	unsigned char tmp;
    int	netPort, rtn, i;

    server_state = 0;
    scanType = SCAN_VOLTAGE;

    if(iopl(3)) exit(1);
    
    if(argc != 4) {
		printf("Usage : %s NumNetPort NumUsePort systemModel\n", argv[0]);
		return -1;
    }

	netPort = atoi(argv[1]);
	systemModel = atoi(argv[2]);
	group = atoi(argv[3]) / 10;
	cali_type = atoi(argv[3]) % 10;
	printf("netPort:%d, systemModel:%d, group:%d\n\n",
		netPort, systemModel, group);

	rtn = 0;
	switch(systemModel) {
		case LG_5V10A:
			max_ch_per_bd = 25;
			aux_da_sync_i = 5; aux_haddr = 6; aux_laddr = 7;
			main_dac_high = 2; main_dac_low_v = 3; main_dac_low_i = 4;
			div_addr = 15;
			mux1_addr = 8; mux2_addr = 9; mux3_addr = 10;
			start_addr = 1; ad_haddr = 1; ad_laddr = 0;
			break;
		case VKEIG_5V10A:
			max_ch_per_bd = 32;
			aux_da_sync_i = 5; aux_haddr = 6; aux_laddr = 7;
			main_dac_high = 2; main_dac_low_v = 3; main_dac_low_i = 4;
			div_addr = 15;
			mux1_addr = 8; mux2_addr = 9; mux3_addr = 10;
			start_addr = 1; ad_haddr = 1; ad_laddr = 0;
			break;
		case LGCN_PB2:
			max_ch_per_bd = 64;
			aux_da_sync_i = 5; aux_haddr = 6; aux_laddr = 7;
			main_dac_high = 2; main_dac_low_v = 3; main_dac_low_i = 4;
			div_addr = 15;
			mux1_addr = 8; mux2_addr = 9; mux3_addr = 10;
			start_addr = 1; ad_haddr = 1; ad_laddr = 0;
			break;
		case LGCE_NJ_5V2A:
			max_ch_per_bd = 64;
			aux_da_sync_i = 4; aux_haddr = 5; aux_laddr = 6;
			main_dac_sync_v = 0; main_dac_sync_i = 1; main_dac_high = 2;
			main_dac_low_v = 3; main_dac_low_i = 3;
			run_addr = 8;
			mux1_addr = 6; mux2_addr = 1;
			start_addr = 15; ad_haddr = 10; ad_laddr = 11;

			MainADC_Calibration();
			if(cali_type == CALI_NORMAL) {
				MainDAC_CaliData_Read(CALI_VOLTAGE);
				MainDAC_CaliData_Read(CALI_CURRENT);

				//outb(0x01, 0x604); //7V smps on
			} else {
				txsettings1("*RST\n", 0);
				txsettings1("SYST:REM\n", 0);
				txsettings1("CONF:VOLT:DC DEF,DEF\n", 0);
				txsettings1("SAMPLE:COUNT 1\n", 0);

				MainDAC_Calibration2(cali_type);
				MainDAC_CaliData_Write(cali_type);
				rtn = -1;
			}
			break;
		case SW_DEMO_5V3A:
			max_ch_per_bd = 16;
			aux_da_sync_v = 3; aux_da_sync_i = 4; aux_haddr = 5; aux_laddr = 6;
			main_dac_sync_v = 1; main_dac_sync_i = 2; main_dac_sync_l = 3;
			main_dac_high = 4;
			main_dac_low_v = 5; main_dac_low_i = 5; main_dac_low_l = 5;
			run_addr = 7;
			mux1_addr = 6; mux2_addr = 2;
			start_addr = 15; ad_haddr = 10; ad_laddr = 11;

			outb(0x00, 0x603); //inverter all off
			sleep(1);

			for(i=0; i < 16; i++) {
				//ch run signal off
				outb(0x00, 0x700 + (i * 0x10) + run_addr);
				outb(0x00, 0x700 + (i * 0x10) + run_addr + 1);

				//mode : charge
				outb(0x00, 0x700 + (i * 0x10) + 1);

				//mux initialize
				outb(0x00, 0x700 + (i * 0x10) + mux2_addr);
			}

			outb(0x04, 0x604); //fan on
			sleep(1);

			outb(0x01, 0x603); //inverter reset on
			usleep(100000);
			outb(0x00, 0x603); //inverter reset off
			usleep(100000);
			outb(0x02, 0x603); //inverter run on
			sleep(1);

			//47uF*4 + 22uF*16*8 = 3004uF
			//3004uF*1.67ohm = 5.017ms
			outb(0x07, 0x604); //inverter capacitor relay on, fan on
			sleep(1);

			tmp = inb(0x601);
			if(tmp != 0x02) {
				outb(0x00, 0x603); //inverter all off
				printf("inverter fault : %x\n", tmp);
				rtn = -1;
				break;
			}

			MainADC_Calibration();
			if(cali_type == CALI_NORMAL) {
				MainDAC_CaliData_Read(CALI_VOLTAGE);
				MainDAC_CaliData_Read(CALI_CURRENT);
				MainDAC_CaliData_Read(CALI_LIMIT);
			} else {
				txsettings1("*RST\n", 0);
				txsettings1("SYST:REM\n", 0);
				txsettings1("CONF:VOLT:DC DEF,DEF\n", 0);
				txsettings1("SAMPLE:COUNT 1\n", 0);

				MainDAC_Calibration2(cali_type);
				MainDAC_CaliData_Write(cali_type);
				rtn = -1;
			}
			break;
		default:
			printf("Unknown systemModel:%d\n", systemModel);
   			rtn = -1;
			break;
	}
	if(rtn < 0) return -2;

	CellMapping();

	rtn = ServerEngine(netPort, DoRead);

	return 0;
}
