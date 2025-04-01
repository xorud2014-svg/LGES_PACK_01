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

int systemModel, server_state, scanType, Channel, auxChNum;
int meter, calbox;
int MainBdNum, baseAddr, caliAddr;
int max_ch_per_bd;
int aux_da_sync, aux_haddr, aux_laddr;
int main_dac_sync_v, main_dac_sync_i, main_dac_high;
int main_dac_low_v, main_dac_low_i;
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

int da_value[2][3];
long calSourceV[3], calSourceI[3];
double Vsource_AD_a, Vsource_AD_b, Isource_AD_a, Isource_AD_b;

double cmdV_DA_a, cmdV_DA_b, cmdI_DA_a, cmdI_DA_b;
int cmd_adv_org[3], cmd_adi_org[3];
double cmd_adv[3], cmd_adi[3];

REF vref;
REF iref;
DAC_VALUE dac_value;

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

void SetAuxDA1(int num, unsigned char val)
{ //AD5308
    unsigned char auxDAChannel, low, sync;
    int j;

    if(iopl(3)) exit(1);

    if(num < 0) {
		val = 0x7F;
		for(j=0; j < max_ch_per_bd; j++){
			sync = 0x01;
			sync = sync << (j / 8);
			outb(sync, (baseAddr + aux_da_sync));

    		auxDAChannel = (unsigned char)(j % 8); 
	   		auxDAChannel = auxDAChannel << 4;
		   	auxDAChannel |= val >> 4;
			outb(auxDAChannel, (baseAddr + aux_haddr)); //high
			low = val << 4;
			outb(low, (baseAddr + aux_laddr)); //low
			usleep(100);
		}
		printf("all aux da set\n");

		return;
    }

	j = num;
	if(j >= max_ch_per_bd) return;

	printf("aux cali ch num : %d, val : %x\n", j, val);
	sync = 0x01;
	sync = sync << (j / 8);
	outb(sync, (baseAddr + aux_da_sync));

   	auxDAChannel = (unsigned char)(j % 8); 
    auxDAChannel = auxDAChannel << 4;
    auxDAChannel |= val >> 4;
	outb(auxDAChannel, (baseAddr + aux_haddr)); //high
	low = val << 4;
	outb(low, (baseAddr + aux_laddr)); //low
	usleep(100);
}

void SetAuxDA2(int num, unsigned short val)
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
			outb(sync, (baseAddr + aux_da_sync));
			usleep(50);
		}
		printf("all aux da set\n");

		return;
    }

	j = num;
	if(j >= max_ch_per_bd) return;

	printf("aux cali ch num : %d, val : %x\n", j, val);
   	auxDAChannel = (unsigned char)(j % 8); 
    auxDAChannel = auxDAChannel << 4;
    auxDAChannel |= (unsigned char)(val >> 6);
	outb(auxDAChannel, (baseAddr + aux_haddr)); //high
	low = (unsigned char)(val << 2);
	outb(low, (baseAddr + aux_laddr)); //low

	sync = 0x01;
	sync = sync << (j / 8);
	outb(sync, (baseAddr + aux_da_sync));
	usleep(50);
}

void SetAuxDA3(int num, unsigned short val)
{//MAS9184 
	unsigned char auxDAChannel, low, sync, aux_ch;
    int j;

    if(iopl(3)) exit(1);

    if(num < 0) {
		val = 0x7F;
		for(j=0; j < max_ch_per_bd; j++){
    		aux_ch = (unsigned char)(j % 12); 
	   		switch(aux_ch) {
				case 0:	auxDAChannel = 0x08; break;
				case 1: auxDAChannel = 0x04; break;
				case 2: auxDAChannel = 0x0C; break;
				case 3: auxDAChannel = 0x02; break;
				case 4: auxDAChannel = 0x0A; break;
				case 5: auxDAChannel = 0x06; break;
				case 6: auxDAChannel = 0x0E; break;
				case 7: auxDAChannel = 0x01; break;
				case 8: auxDAChannel = 0x09; break;
				case 9: auxDAChannel = 0x05; break;
				case 10: auxDAChannel = 0x0D; break;
				case 11: auxDAChannel = 0x03; break;
				default: auxDAChannel = 0x00; break;
			}
			
			outb(auxDAChannel, (baseAddr + aux_haddr)); //high
			low = (unsigned char)val;
			outb(low, (baseAddr + aux_laddr)); //low

			sync = 0x01;
			sync = sync << (j / 12);
			outb(sync, (baseAddr + aux_da_sync));
			usleep(100);
		}
		printf("all aux da set\n");

		return;
    }

	j = num;
	if(j >= max_ch_per_bd) return;

	printf("aux cali ch num : %d, val : %x\n", j, val);
   	aux_ch = (unsigned char)(j % 12); 
	switch(aux_ch) {
		case 0:	auxDAChannel = 0x08; break;
		case 1: auxDAChannel = 0x04; break;
		case 2: auxDAChannel = 0x0C; break;
		case 3: auxDAChannel = 0x02; break;
		case 4: auxDAChannel = 0x0A; break;
		case 5: auxDAChannel = 0x06; break;
		case 6: auxDAChannel = 0x0E; break;
		case 7: auxDAChannel = 0x01; break;
		case 8: auxDAChannel = 0x09; break;
		case 9: auxDAChannel = 0x05; break;
		case 10: auxDAChannel = 0x0D; break;
		case 11: auxDAChannel = 0x03; break;
		default: auxDAChannel = 0x00; break;
	}
	
	outb(auxDAChannel, (baseAddr + aux_haddr)); //high
	low = (unsigned char)val;
	outb(low, (baseAddr + aux_laddr)); //low

	sync = 0x01;
	sync = sync << (j / 12);
	outb(sync, (baseAddr + aux_da_sync));
	usleep(100);

}

void SetAuxDA4(int num, unsigned short val)
{ //AD5318
    unsigned char auxDAChannel, low, sync;
    int j;

    if(iopl(3)) exit(1);

    if(num < 0) {
		val = 0x1FF;
		for(j=0; j < max_ch_per_bd; j++){
			sync = 0x01;
			sync = sync << (j / 8);
			outb(sync, (baseAddr + aux_da_sync));

    		auxDAChannel = (unsigned char)(j % 8); 
	   		auxDAChannel = auxDAChannel << 4;
		   	auxDAChannel |= (unsigned char)(val >> 6);
			outb(auxDAChannel, (baseAddr + aux_haddr)); //high
			low = (unsigned char)(val << 2);
			outb(low, (baseAddr + aux_laddr)); //low

			usleep(100);
		}
		printf("all aux da set\n");

		return;
    }

	j = num;
	if(j >= max_ch_per_bd) return;

	printf("aux cali ch num : %d, val : %x\n", j, val);
	sync = 0x01;
	sync = sync << (j / 8);
	outb(sync, (baseAddr + aux_da_sync));

   	auxDAChannel = (unsigned char)(j % 8); 
    auxDAChannel = auxDAChannel << 4;
    auxDAChannel |= (unsigned char)(val >> 6);
	outb(auxDAChannel, (baseAddr + aux_haddr)); //high
	low = (unsigned char)(val << 2);
	outb(low, (baseAddr + aux_laddr)); //low

	usleep(100);
}

void SetAuxDA(int num, unsigned short val)
{
	switch(systemModel) {
		case LGCE_NJ_5V2A:
			SetAuxDA2(num, val); //AD5318
			break;
		case SK_5V3A:
			SetAuxDA3(num, val); // MAS9184
			break;
		case SK_5V5A:
			SetAuxDA4(num, val); //AD5318
			break;
		default:
			SetAuxDA1(num, (unsigned char)val); //AD5308
			break;
	}
}

void RelayAllOn(void)
{
    int offSetAddr, out_byte;

    if(iopl(3)) exit(1);

	printf("Relay all on\n");

	out_byte = max_ch_per_bd / 8;

	switch(systemModel) {
		case LGCE_NJ_5V2A:
		case SK_5V3A:
			for(offSetAddr=0; offSetAddr < out_byte; offSetAddr++) {
				outb(0xFF, (baseAddr + run_addr + offSetAddr));
			} 
			break;
		default:
			outb(0x01, (baseAddr + div_addr));	
			for(offSetAddr=0; offSetAddr < out_byte; offSetAddr++) {
				outb(0xFF, (baseAddr + offSetAddr));
			} 
			outb(0x00, (baseAddr + div_addr));
			break;
	}
}
  
void RelayAllOff(void)
{
    int offSetAddr, out_byte;

    if(iopl(3)) exit(1);

	printf("Relay all off\n");

	out_byte = max_ch_per_bd / 8;

	switch(systemModel) {
		case LGCE_NJ_5V2A:
		case SK_5V3A:
			for(offSetAddr=0; offSetAddr < out_byte; offSetAddr++) {
				outb(0x00, (baseAddr + run_addr + offSetAddr));
			} 
			break;
		default:
			outb(0x01, (baseAddr + div_addr));	
			for(offSetAddr=0; offSetAddr < out_byte; offSetAddr++) {
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
		case SK_5V3A:
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
	if(type == VOLTAGE) {
		muxVal2 = 0x00;
	} else { //CURRENT
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

	mux_ch = 16;
	muxVal1 = (num % mux_ch);

	if(type == VOLTAGE) {
		muxVal1 |= 0x00;
	} else { //CURRENT
		muxVal1 |= 0x10;
	}

	muxVal2 = 0x01;
	muxVal2 = muxVal2 << (num / mux_ch);
	
	outb(muxVal1, (baseAddr + mux1_addr));
	outb(muxVal2, (baseAddr + mux2_addr));
}

void SetMux(int num, int type)
{
	switch(systemModel) {
		case LGCE_NJ_5V2A:
			SetMux2(num, type);
			break;
		case SK_5V3A:
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

	if(type == VOLTAGE) {
		sprintf(fname,"./tmp/ADVALUEVolt_%dB%dC.csv", MainBdNum+1, Channel+1);
	} else { //CURRENT
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
	//long real;
	double val=0;
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
				//usleep(20000);
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

				//real = RealData();
				//printf("Meter value Ref_V(%d) : %ld\n", i, real);
				break;
			case SK_5V3A:
				//mux
				if(i == 0) {
					tmp = 0x20;		//ref+5v
				} else if(i == 1) {
					tmp = 0x30;		// gnd
				} else {
					tmp = 0x60;		//ref -5v
				}
				outb((unsigned char)tmp, baseAddr + mux1_addr);
				outb(0x00, baseAddr + mux2_addr);
				usleep(2000);

				//AD
				val = 0.0;
				for(j=0; j < REF_READ_COUNT; j++) {
					tmp = (int)inb(baseAddr + start_addr);
					usleep(2000);
					myVal.data[1] = inb(baseAddr + ad_haddr);
					myVal.data[0] = inb(baseAddr + ad_laddr);
					val += (double)myVal.val;
				}
				break;
			case SK_5V5A:
				//mux
				if(i == 0) {
					tmp = 0x30;
				} else if(i == 1) {
					tmp = 0x38;
				} else if(i == 2) {
					tmp = 0x34;
				} else {
					tmp = 0x3C;
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
		} else {
			da_value[0][0] = (int)val;
		}
		vref.data = 0;
	}
}
				
void Ref_I_Read(void)
{
    int	i, j, tmp;
	double val=0;
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
			case SK_5V3A:
				//mux
				if(i == 0) {
					tmp = 0x20;		//ref+5v
				} else if(i == 1) {
					tmp = 0x30;		// gnd
				} else {
					tmp = 0x60;		//ref -5v
				}
				outb((unsigned char)tmp, baseAddr + mux1_addr);
				outb(0x00, baseAddr + mux2_addr);
				usleep(2000);

				//AD
				val = 0.0;
				for(j=0; j < REF_READ_COUNT; j++) {
					tmp = inb(baseAddr + start_addr);
					usleep(2000);
					myVal.data[1] = inb(baseAddr + ad_haddr);
					myVal.data[0] = inb(baseAddr + ad_laddr);
					val += (double)myVal.val;
				}
				break;
			case SK_5V5A:
				//mux
				if(i == 0) {
					tmp = 0x30;
				} else if(i == 1) {
					tmp = 0x38;
				} else if(i == 2) {
					tmp = 0x34;
				} else {
					tmp = 0x3C;
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
		} else {
			da_value[1][0] = (int)val;
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
		default:
			outb(myVal.data[1], (baseAddr + main_dac_high));
			outb(myVal.data[0], (baseAddr + main_dac_low_i));
			break;
	}
}

int MainDA_V_Read(void)
{
    int	i, tmp;
	double val;
	U_ADDA myVal;

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
		SetMainDA_I(cmd_dai[i]);
		usleep(20000);

		val = MainDA_V_Read();
		cmd_adv_org[i] = val;
		cmd_adv[i] = ((double)val / 1.494 * Vsource_AD_a + Vsource_AD_b);
		val = MainDA_I_Read();
		cmd_adi_org[i] = val;
		cmd_adi[i] = ((double)val / 1.494 * Isource_AD_a + Isource_AD_b);
	}

	dac_value.V_P = cmd_adv_org[0];
	dac_value.V_N = cmd_adv_org[2];
	dac_value.I_P = cmd_adi_org[0];
	dac_value.I_N = cmd_adi_org[2];

	printf("cmd_adv_org %d %d %d\n",
		cmd_adv_org[0], cmd_adv_org[1], cmd_adv_org[2]);
	printf("cmd_adv %f %f %f\n",
		cmd_adv[0], cmd_adv[1], cmd_adv[2]);
	printf("cmd_adi_org %d %d %d\n",
		cmd_adi_org[0], cmd_adi_org[1], cmd_adi_org[2]);
	printf("cmd_adi %f %f %f\n\n",
		cmd_adi[0], cmd_adi[1], cmd_adi[2]);

	tmp1 = (double)cmd_dav[0] - (double)cmd_dav[1];
	tmp2 = cmd_adv[0] - cmd_adv[1];
	cmdV_DA_a = tmp1 / tmp2;
	cmdV_DA_b = (double)cmd_dav[0] - cmd_adv[0] * cmdV_DA_a;

	tmp1 = (double)cmd_dai[0] - (double)cmd_dai[2];
	tmp2 = cmd_adi[0] - cmd_adi[2];
	cmdI_DA_a = tmp1 / tmp2;
	cmdI_DA_b = (double)cmd_dai[0] - cmd_adi[0] * cmdI_DA_a;

	for(i=0; i < 3; i++) {
		value = (double)cmd_dav[i] * cmdV_DA_a + cmdV_DA_b;
		if(value < 0) value -= 0.5;
		else value += 0.5;
		val = (int)value;
		SetMainDA_V(val);

		value = (double)cmd_dai[i] * cmdI_DA_a + cmdI_DA_b;
		if(value < 0) value -= 0.5;
		else value += 0.5;
		val = (int)value;
		SetMainDA_I(val);
		usleep(20000);

		val = MainDA_V_Read();
		cmd_adv[i] = ((double)val / 1.494 * Vsource_AD_a + Vsource_AD_b);
		val = MainDA_I_Read();
		cmd_adi[i] = ((double)val / 1.494 * Isource_AD_a + Isource_AD_b);
	}

	printf("cmdV_DA_a a:%f, b:%f\n", cmdV_DA_a, cmdV_DA_b);
	printf("cmd_adv %f %f %f\n\n", cmd_adv[0], cmd_adv[1], cmd_adv[2]);

	printf("cmdI_DA a:%f, b:%f\n", cmdI_DA_a, cmdI_DA_b);
	printf("cmd_adi %f %f %f\n\n", cmd_adi[0], cmd_adi[1], cmd_adi[2]);
}

void MainDAC_Calibration2(int type)
{
	int i, val, cmd_dav[3], cmd_dai[3];
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

	for(i=0; i < 3; i++) {
		if(type == VOLTAGE) {
			SetMainDA_V(cmd_dav[i]);
		} else {
			SetMainDA_I(cmd_dai[i]);
		}
		usleep(200000);

		real = RealData();
		if(type == VOLTAGE) {
			//printf("Meter value DAC_V(%d) : %ld\n", i, real);
			cmd_adv_org[i] = (int)real;
			cmd_adv[i] = (double)real * 0.0032768;
		} else {
			//printf("Meter value DAC_I(%d) : %ld\n", i, real);
			cmd_adi_org[i] = (int)real;
			cmd_adi[i] = (double)real * 0.0032768;
		}
	}

	if(type == VOLTAGE) {
		dac_value.V_P = cmd_adv_org[0];
		dac_value.V_N = cmd_adv_org[2];
		printf("cmd_adv_org %f %f %f\n", cmd_adv_org[0]/1000.0,
			cmd_adv_org[1]/1000.0, cmd_adv_org[2]/1000.0);
		//printf("cmd_adv %f %f %f\n",
		//	cmd_adv[0], cmd_adv[1], cmd_adv[2]);

		tmp1 = (double)cmd_dav[0] - (double)cmd_dav[1];
		tmp2 = cmd_adv[0] - cmd_adv[1];
		cmdV_DA_a = tmp1 / tmp2;
		cmdV_DA_b = (double)cmd_dav[0] - cmd_adv[0] * cmdV_DA_a;
	} else {
		dac_value.I_P = cmd_adi_org[0];
		dac_value.I_N = cmd_adi_org[2];
		printf("cmd_adi_org %f %f %f\n", cmd_adi_org[0]/1000.0,
			cmd_adi_org[1]/1000.0, cmd_adi_org[2]/1000.0);
		//printf("cmd_adi %f %f %f\n\n",
		//	cmd_adi[0], cmd_adi[1], cmd_adi[2]);

		tmp1 = (double)cmd_dai[0] - (double)cmd_dai[2];
		tmp2 = cmd_adi[0] - cmd_adi[2];
		cmdI_DA_a = tmp1 / tmp2;
		cmdI_DA_b = (double)cmd_dai[0] - cmd_adi[0] * cmdI_DA_a;
	}

	for(i=0; i < 3; i++) {
		if(type == VOLTAGE) {
			value = (double)cmd_dav[i] * cmdV_DA_a + cmdV_DA_b;
			if(value < 0) value -= 0.5;
			else value += 0.5;
			val = (int)value;
			SetMainDA_V(val);
		} else {
			value = (double)cmd_dai[i] * cmdI_DA_a + cmdI_DA_b;
			if(value < 0) value -= 0.5;
			else value += 0.5;
			val = (int)value;
			SetMainDA_I(val);
		}
		usleep(200000);

		real = RealData();
		if(type == VOLTAGE) {
			//printf("Meter value DAC_V2(%d) : %ld\n", i, real);
			cmd_adv[i] = (double)real;
		} else {
			//printf("Meter value DAC_I2(%d) : %ld\n", i, real);
			cmd_adi[i] = (double)real;
		}
	}

	if(type == VOLTAGE) {
		printf("cmdV_DA_a a:%f, b:%f\n", cmdV_DA_a, cmdV_DA_b);
		printf("cmd_adv %f %f %f\n\n",
			cmd_adv[0]/1000.0, cmd_adv[1]/1000.0, cmd_adv[2]/1000.0);
	} else {
		printf("cmdI_DA a:%f, b:%f\n", cmdI_DA_a, cmdI_DA_b);
		printf("cmd_adi %f %f %f\n\n",
			cmd_adi[0]/1000.0, cmd_adi[1]/1000.0, cmd_adi[2]/1000.0);
	}
}

void MainDAC_CaliData_Write(int type)
{
    FILE *fp;

	if(type == VOLTAGE) {
		system("rm mainDA_V_CaliData");
	    if((fp = fopen("mainDA_V_CaliData", "a")) == NULL) {
			printf("mainDA_V write error\n");
			return;
		}
	} else {
		system("rm mainDA_I_CaliData");
	    if((fp = fopen("mainDA_I_CaliData", "a")) == NULL) {
			printf("mainDA_I write error\n");
			return;
		}
	}

	if(type == VOLTAGE) {
		fprintf(fp, "MainDA_V\n");
		fprintf(fp, "org_meter %d %d %d\n",
			cmd_adv_org[0], cmd_adv_org[1], cmd_adv_org[2]);
		fprintf(fp, "org_da_value %d %d %d\n",
			(int)((double)cmd_adv_org[0] * 0.0032768),
			(int)((double)cmd_adv_org[1] * 0.0032768),
			(int)((double)cmd_adv_org[2] * 0.0032768));
		fprintf(fp, "cal_meter %f %f %f\n", cmd_adv[0] * 0.0032768,
			cmd_adv[1] * 0.0032768, cmd_adv[2] * 0.0032768);
		fprintf(fp, "cal_da_value %d %d %d\n",
			(int)(cmd_adv[0]), (int)(cmd_adv[1]), (int)(cmd_adv[2]));
		fprintf(fp, "DA_A %f\n", cmdV_DA_a);
		fprintf(fp, "DA_B %f\n", cmdV_DA_b);
	} else {
		fprintf(fp, "MainDA_I\n");
		fprintf(fp, "org_meter %d %d %d\n",
			cmd_adi_org[0], cmd_adi_org[1], cmd_adi_org[2]);
		fprintf(fp, "org_da_value %d %d %d\n",
			(int)((double)cmd_adi_org[0] * 0.0032768),
			(int)((double)cmd_adi_org[1] * 0.0032768),
			(int)((double)cmd_adi_org[2] * 0.0032768));
		fprintf(fp, "cal_meter %f %f %f\n", cmd_adi[0] * 0.0032768,
			cmd_adi[1] * 0.0032768, cmd_adi[2] * 0.0032768);
		fprintf(fp, "cal_da_value %f %f %f\n",
			cmd_adi[0], cmd_adi[1], cmd_adi[2]);
		fprintf(fp, "DA_A %f\n", cmdI_DA_a);
		fprintf(fp, "DA_B %f\n", cmdI_DA_b);
	}

	fclose(fp);
}

void MainDAC_CaliData_Read(int type)
{
	char buf[24];
	int tmp, i;
    FILE *fp;

	if(type == VOLTAGE) {
	    if((fp = fopen("mainDA_V_CaliData", "r")) == NULL) {
			printf("mainDA_V read error\n");
			return;
		}
	} else {
	    if((fp = fopen("mainDA_I_CaliData", "r")) == NULL) {
			printf("mainDA_I read error\n");
			return;
		}
	}

	for(i=0; i < 18; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
	}

	if(type == VOLTAGE) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		cmdV_DA_a = (double)atof(buf);

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		cmdV_DA_b = (double)atof(buf);

		printf("V_DA_A %f, V_DA_B %f\n", cmdV_DA_a, cmdV_DA_b);
	} else {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		cmdI_DA_a = (double)atof(buf);

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		cmdI_DA_b = (double)atof(buf);

		printf("I_DA_A %f, I_DA_B %f\n", cmdI_DA_a, cmdI_DA_b);
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

	printf("cmd = %d \n", cmd);
	
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
				case SK_5V3A:
					baseAddr = 0x530 + (0x10 * MainBdNum);
					break;
				default:
					baseAddr = 0x620 + (0x10 * MainBdNum);
					break;
			}
			SetAuxDA(-1, 0);
			write_ACK(fd);
			break;
    	case 3:  
			printf("Set current dac : %d\n", val);
			switch(systemModel) {
				case LGCE_NJ_5V2A:
					value = (double)val * cmdI_DA_a + cmdI_DA_b;
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
					value = (double)val * cmdV_DA_a + cmdV_DA_b;
					//value = (double)val;
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
			if(scanType == CURRENT) {
				printf("Set relay on : %d\n", val+1);
				RelayOnOff(val, ON);
				if(calbox == CAL_BOX_SBC_CTRL) {
					printf("TraySwitch:On, BdNum:%d, ChNo:%d\n",
						MainBdNum+1, val+1);
					//Calibrator Fixture B'd S/W On
					TraySwitchOnOff(ON, MainBdNum, val);
				}
			} else if(scanType == VOLTAGE) {
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
			if(scanType == CURRENT) {
				printf("Set relay off : %d\n", val+1);
				RelayOnOff(val, OFF);
				if(calbox == CAL_BOX_SBC_CTRL) {
					printf("TraySwitch:Off, BdNum:%d, ChNo:%d\n",
						MainBdNum+1, val+1);
					//Calibrator Fixture B'd S/W Off
					TraySwitchOnOff(OFF, MainBdNum, val);
				}
			} else if(scanType == VOLTAGE) {
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
	    case 8: //mode select
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
						if(scanType == VOLTAGE) {
							outb(0x00, 0x612); //CC/CV Relay Togle
						} else { //CURRENT
							outb(0x0C, 0x612); //CC/CV Relay Togle
						}
						break;
					case SK_5V3A:
						if(scanType == VOLTAGE) {
							outb(0x00, 0x510);
						} else { //CURRENT
							outb(0x0C, 0x510);
						}
						break;
					case SK_5V5A:
						if(scanType == VOLTAGE) {
							outb(0x00, 0x604); //CC/CV Relay Togle
						} else { //CURRENT
							outb(0x08, 0x604); //CC/CV Relay Togle
						}
						break;
					default:
						if(scanType == VOLTAGE) {
							outb(0x00, 0x610); //CC/CV Relay Togle
						} else { //CURRENT
							outb(0x01, 0x610); //CC/CV Relay Togle
						}
						break;
				}
			}

			if(scanType == VOLTAGE && calbox != CAL_BOX_NONUSE) {
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
						if(scanType == VOLTAGE) {
							value = (double)myVal.val
								* Vsource_AD_a + Vsource_AD_b;
						} else { //CURRENT
							value = (double)myVal.val
								* Isource_AD_a + Isource_AD_b;
						}
						if(value < 0) {
							myVal.val = (short int)(value - 0.5);
						} else {
							myVal.val = (short int)(value + 0.5);
						}
						break;
					case SK_5V3A:
						tmp = (int)inb(baseAddr + start_addr);
						usleep(2000);
						myVal.data[1] = inb(baseAddr + ad_haddr);
						myVal.data[0] = inb(baseAddr + ad_laddr);
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
			CreatADFile(CH_READ_COUNT2, scanType); //for AD Value confirmation
			write_ACK(fd);
			break;
	    case 12:
			auxChNum = 0;
			printf("Set channel number = 0 for AUX DA\n");
			write_ACK(fd);
			break;
	    case 13:
			SetAuxDA(auxChNum, (unsigned short)val);
			printf("Set AUX DA ch:%d, val:%d\n", auxChNum+1, val);
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
			SetAuxDA(-1, 0);
			VoltDACReset();
			CurrDACReset();

			system("rm -rf tmp/*");
			write_ACK(fd);
			break;
		case 19:
			switch(systemModel) {
				case LGCE_NJ_5V2A:
					tmp = 0x1003; break;
				case SK_5V3A:
					tmp = 0x1004; break;
				case SK_5V5A:
					tmp = 0x1005; break;
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
						if(scanType == VOLTAGE) {
							value = (double)myVal.val
								* Vsource_AD_a + Vsource_AD_b;
						} else { //CURRENT
							value = (double)myVal.val
								* Isource_AD_a + Isource_AD_b;
						}
						if(value < 0) {
							myVal.val = (short int)(value - 0.5);
						} else {
							myVal.val = (short int)(value + 0.5);
						}
						break;
					case SK_5V3A:
						tmp = (int)inb(baseAddr + start_addr);
						usleep(2000);
						myVal.data[1] = inb(baseAddr + ad_haddr);
						myVal.data[0] = inb(baseAddr + ad_laddr);
						break;
					default:
						outb(0x01, (baseAddr + start_addr));
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
				dac_value.V_P, dac_value.V_N, dac_value.I_P, dac_value.I_N);
   			rtn = writen(fd, (char *)&dac_value, sizeof(DAC_VALUE));
   			if(rtn != sizeof(DAC_VALUE))
				printf("ERROR: DAC_VALUE I don't send \n");
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
    int	netPort, rtn;

    server_state = 0;
    scanType = VOLTAGE;

    if(iopl(3)) exit(1);
    
    if(argc != 4) {
		printf("Usage : %s NumNetPort NumUsePort systemModel\n", argv[0]);
		return -1;
    }

	netPort = atoi(argv[1]);
	systemModel = atoi(argv[2]);
	printf("netPort:%d, systemModel:%d\n\n", netPort, systemModel);

	rtn = 0;
	switch(systemModel) {
		case LG_5V10A:
			max_ch_per_bd = 25;
			aux_da_sync = 5; aux_haddr = 6; aux_laddr = 7;
			main_dac_high = 2; main_dac_low_v = 3; main_dac_low_i = 4;
			div_addr = 15;
			mux1_addr = 8; mux2_addr = 9; mux3_addr = 10;
			start_addr = 1; ad_haddr = 1; ad_laddr = 0;
			break;
		case VKEIG_5V10A:
		case SK_5V5A:
			max_ch_per_bd = 32;
			aux_da_sync = 5; aux_haddr = 6; aux_laddr = 7;
			main_dac_high = 2; main_dac_low_v = 3; main_dac_low_i = 4;
			div_addr = 15;
			mux1_addr = 8; mux2_addr = 9; mux3_addr = 10;
			start_addr = 1; ad_haddr = 1; ad_laddr = 0;
			break;
		case LGCN_PB2:
			max_ch_per_bd = 64;
			aux_da_sync = 5; aux_haddr = 6; aux_laddr = 7;
			main_dac_high = 2; main_dac_low_v = 3; main_dac_low_i = 4;
			div_addr = 15;
			mux1_addr = 8; mux2_addr = 9; mux3_addr = 10;
			start_addr = 1; ad_haddr = 1; ad_laddr = 0;
			break;
		case LGCE_NJ_5V2A:
			max_ch_per_bd = 64;
			aux_da_sync = 4; aux_haddr = 5; aux_laddr = 6;
			main_dac_sync_v = 0; main_dac_sync_i = 1; main_dac_high = 2;
			main_dac_low_v = 3; main_dac_low_i = 3;
			run_addr = 8;
			mux1_addr = 6; mux2_addr = 1;
			start_addr = 15; ad_haddr = 10; ad_laddr = 11;

			MainADC_Calibration();
			if(atoi(argv[3]) == 1) {
				txsettings1("*RST\n", 0);
				txsettings1("SYST:REM\n", 0);
				txsettings1("CONF:VOLT:DC DEF,DEF\n", 0);
				txsettings1("SAMPLE:COUNT 1\n", 0);

				MainDAC_Calibration2(VOLTAGE);
				MainDAC_CaliData_Write(VOLTAGE);
				return 0;
			} else if(atoi(argv[3]) == 2) {
				txsettings1("*RST\n", 0);
				txsettings1("SYST:REM\n", 0);
				txsettings1("CONF:VOLT:DC DEF,DEF\n", 0);
				txsettings1("SAMPLE:COUNT 1\n", 0);

				MainDAC_Calibration2(CURRENT);
				MainDAC_CaliData_Write(CURRENT);
				return 0;
			} else {
				MainDAC_CaliData_Read(VOLTAGE);
				MainDAC_CaliData_Read(CURRENT);
			}

			//outb(0x01, 0x604); //7V smps on
			break;
		case SK_5V3A:
			max_ch_per_bd = 32;
			aux_da_sync = 3; aux_haddr = 4; aux_laddr = 5;
			main_dac_high = 0; main_dac_low_v = 1; main_dac_low_i = 2;
			run_addr = 8;
			mux1_addr = 6; mux2_addr = 7;
			start_addr = 0; ad_haddr = 1; ad_laddr = 2;
			outb(0x01, 0x512); //MC ON
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
