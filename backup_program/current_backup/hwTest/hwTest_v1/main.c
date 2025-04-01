#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/io.h>

#define NONE	0
#define OUTPUT	1
#define INPUT	2
#define AD		3
#define ADJ		4
#define DACHECK	5
#define MAX_AD_COUNT	10

short REFIC_P5V[MAX_AD_COUNT];
short REFIC_G0V[MAX_AD_COUNT];
short REFIC_N5V[MAX_AD_COUNT];
short REFIC_P5I[MAX_AD_COUNT];
short REFIC_G0I[MAX_AD_COUNT];
short REFIC_N5I[MAX_AD_COUNT];
short REFIC[MAX_AD_COUNT];

union tmpData {
	char	data[2];
	short int	val;
};

union ADData {
	unsigned char	data[2];
	short int	val;
};

int main(void)
{
    int		retval, addr, val, type, mode, phase1;
	int		ch, val1;
	int		o_addr, i_addr, count;
    int 	i,j;
	short int	Vfeedback[4]={0,},Ifeedback[4]={0,},PlusVref[3],MinusVref[3];
	short int		FeedTmp=0;
	short int		FBRead[2]={0,};
	int		FBCalc=0;
	short int	tmpVal;
    float	refVal;
    char	tmp[10];
	unsigned char	o_val, i_val; 
	unsigned char muxVal1, muxVal2;
    unsigned char	adVal, adVal2;
	union	tmpData txData;
	union	ADData adData;
    struct	timeval tv;
    fd_set	rfds;
	FILE *fp;

	phase1 = type = o_addr = i_addr = count = 0;
	o_val = muxVal2 = 0;
	mode = NONE;

    if(iopl(3)) exit(1);
	

	while(1) {
		tmp[0] = 0;
		if(mode == NONE) {
			printf("scan1>> ");
			scanf("%s", tmp);
		}

		if(strcmp(tmp, "vref") == 0) {
			//scanf("%d", &val);
			scanf("%d %d", &ch, &val);
			if(val == 0) {
				txData.val = 0;
			} else {
				refVal = (float)val;
				tmpVal = (short int)(refVal/10.0*32768.0);
				printf("vref : %1.3f %02X\n", refVal, tmpVal);
				txData.val = tmpVal;
			}
			ch = ch - 1;
			val1 = 0x01;
			val1 = val1 << ch;
			//outb(0x0f, 0x640);
			outb(val1, 0x640);
			outb(txData.data[1], 0x642);	//REF_HIGH
			outb(txData.data[0], 0x643);
		} else if(strcmp(tmp, "iref") == 0) {
			//scanf("%d", &val);
			scanf("%d %d", &ch, &val);
			if(val == 0) {
				txData.val = 0;
			} else {
				refVal = (float)val;
				tmpVal = (short int)(refVal/10.0*32768.0);
				printf("iref : %1.3f %02X\n", refVal, tmpVal);
				txData.val = tmpVal;
			}
			ch = ch - 1;
			val1 = 0x01;
			val1 = val1 << ch;
			//outb(0x0f, 0x641);
			outb(val1, 0x641);
			outb(txData.data[1], 0x642);	//REF_HIGH
			outb(txData.data[0], 0x643);
		} else if(strcmp(tmp, "lref") == 0) {
			scanf("%x", &val);
			if(val == 0) {
				txData.val = 0;
			} else {
				refVal = (float)val;
				tmpVal = (short int)(refVal/10.0*32768.0);
				printf("lref : %1.3f %02X\n", refVal, tmpVal);
				txData.val = tmpVal;
			}
			outb(0x01, 0x640);
			outb(txData.data[1], 0x641);	//REF_HIGH
			outb(txData.data[0], 0x64a);
		} else if(strcmp(tmp, "run") == 0) {
			scanf("%x", &val);
			printf("run %d\n", val);
			if(val == 1) {
				outb(0x01, 0x630);
			} else {
				outb(0x00, 0x630);
			}
		} else if(strcmp(tmp, "range") == 0) {
			scanf("%x", &val);
			printf("range %d\n", val);
			if(val == 0) {
				outb(0x00, 0x634);
				outb(0x00, 0x632);
			} else if(val == 1) {
				outb(0x00, 0x634);
				outb(0x01, 0x632);
			} else if(val == 2) {
				outb(0x00, 0x632);
				outb(0x01, 0x634);
			}
		}

		if(strcmp(tmp, "write") == 0) {
			scanf("%x %x", &addr, &val);
			printf("write %x %x\n", addr, val);
			o_addr = addr;
			o_val = (unsigned char)val;
			mode = OUTPUT;
		} else if(strcmp(tmp, "wr") == 0) {
			scanf("%x %x", &addr, &val);
			printf("wr %x %x\n", addr, val);
			mode = NONE;
			o_addr = addr;
			o_val = (unsigned char)val;
			outb(o_val,o_addr);
		} else if(strcmp(tmp, "wr_R") == 0) {
			scanf("%x %x", &addr, &val);
			printf("wr %x %x\n", addr, val);
			mode = NONE;
			o_addr = addr;
			o_val = (unsigned char)val;
			for(i=0;i<8;i++)
			{
				outb((0x01 << i), o_addr);
				sleep(1);
			}
			outb(0x00,o_addr);
		} else if(strcmp(tmp, "read16") == 0) {
			scanf("%x", &addr);
			printf("read 0x%04X\n", addr);
			i_addr = addr;
			for(i=0;i<16;i++)
			{
				printf(" ==> 0x%02X\n",inb(i_addr));
				sleep(1);
			}
			//mode = INPUT;
		} else if(strcmp(tmp, "read") == 0) {
			scanf("%x", &addr);
			printf("read 0x%04X\n", addr);
			i_addr = addr;
			mode = INPUT;
		} else if(strcmp(tmp, "read1") == 0) {
			scanf("%x", &addr);
			printf("read1 %x\n", addr);
			mode = NONE;
			i_addr = addr;
			//for(i=0; i < 184; i++) {
			for(i=0; i < 360; i++) {
				i_val = inb(i_addr);
				//printf("%x ", i_val);
			}
			printf("\n");
		} else if(strcmp(tmp, "get") == 0) {
			scanf("%x", &addr);
			printf("get %x\n", addr);
			mode = NONE;
			i_addr = addr;
			i_val = inb(i_addr);
			printf("get data %x %x\n", i_addr, i_val);
		} else if(strcmp(tmp, "mux") == 0) {
			scanf("%d %d", &addr, &val);
			printf("set mux to channel %d ", addr);
			if(val == 0) {
				type = 0; printf("voltage\n");
			} else {
				type = 1; printf("current\n");
			}
			if(addr < 1 || addr > 64) 
				printf("channel out of range\n");
			else {
				o_addr = addr - 1;
				o_val = val;
				muxVal1 = (o_addr%16) & 0x0F;
				if(o_val == 0) {
					muxVal1 |= 0x00;
				} else {
					muxVal1 |= 0x10;
				}
				switch(o_addr/16) {
					case 0 : muxVal2 = 0x01; break;
					case 1 : muxVal2 = 0x02; break;
					case 2 : muxVal2 = 0x04; break;
					case 3 : muxVal2 = 0x08; break;
				}
				outb(muxVal1, 0x527);	// MUX1_ADDR
				outb(muxVal2, 0x528);	// MUX2_ADDR
			}
		} else if(strcmp(tmp, "FeedBack") == 0) {
			//V,I Feedback Read
			outb(0x0F,0x631);	//MUX Select V,I
			for(j=0;j<10;j++)
			{
				for(val=0;val<4;val++)
				{
					outb(((val + (val << 2)) & 0x0F), 0x630);	//Ch Select V,I
					usleep(100000);
					outb(0x00,0x650);	// Voltage Line
					usleep(50);
					FBCalc = 0;
					for(i=0;i<3;i++)
					{
						FBRead[0] = inb(0x650 + i*2);
						FBRead[1] = inb(0x651 + i*2);
						FeedTmp = (FBRead[0] << 8) + FBRead[1];
						FBCalc += (long)FeedTmp;
					}
					Vfeedback[val] = (int)(FBCalc / 3L);
					
					outb(0x01,0x650);	// Current Line
					usleep(50);
					FBCalc = 0;
					for(i=0;i<3;i++)
					{
						FBRead[0] = inb(0x650 + i*2);
						FBRead[1] = inb(0x651 + i*2);
						FeedTmp = (FBRead[0] << 8) + FBRead[1];
						FBCalc += (long)FeedTmp;
					}
					Ifeedback[val] = (int)(FBCalc / 3L);	
					usleep(10000);
				}
					printf("\n");
					for(i=0;i<4;i++)
					{
						printf("Voltage[%1d] = %7.4f, Current[%1d] = %7.4f\n",i,(float)Vfeedback[i]*10.0/32767.0,i,(float)Ifeedback[i]*10.0/32767.0);
					}
					sleep(1);
			}

			// Ref Check
			for(j=0;j<3;j++)
			{
				outb( ( (j << 2) + j),0x631);	//MUX Select V,I
				usleep(100000);
				outb(0x00,0x650);	// Voltage Line
				usleep(50);
				FBCalc = 0;
				for(i=0;i<3;i++)
				{
					FBRead[0] = inb(0x650 + i*2);
					FBRead[1] = inb(0x651 + i*2);
					FeedTmp = (FBRead[0] << 8) + FBRead[1];
					FBCalc += (long)FeedTmp;
				}
				PlusVref[j] = (int)(FBCalc / 3L);
					
				outb(0x01,0x650);	// Current Line
				usleep(50);
				FBCalc = 0;
				for(i=0;i<3;i++)
				{
					FBRead[0] = inb(0x650 + i*2);
					FBRead[1] = inb(0x651 + i*2);
					FeedTmp = (FBRead[0] << 8) + FBRead[1];
					FBCalc += (long)FeedTmp;
				}
				MinusVref[j] = (int)(FBCalc / 3L);	
				usleep(10000);
			}
			
			printf("V-Plus-Ref = %7.4f,\nV-Minus-Ref = %7.4f,\nV-Gnd-Ref = %7.4f\n",(float)PlusVref[0]*10.0/32767.0,(float)PlusVref[1]*10.0/32767.0,(float)PlusVref[2]*10.0/32767.0);
			printf("I-Plus-Ref = %7.4f,\nI-Minus-Ref = %7.4f,\nI-Gnd-Ref = %7.4f\n",(float)MinusVref[0]*10.0/32767.0,(float)MinusVref[1]*10.0/32767.0,(float)MinusVref[2]*10.0/32767.0);
			
			sleep(1);
		} else if(strcmp(tmp, "adv") == 0) {
			type = 0;
			mode = AD;
			phase1 = 0;
		} else if(strcmp(tmp, "adi") == 0) {
			type = 1;
			mode = AD;
			phase1 = 0;
		} else if(strcmp(tmp, "adj") == 0) {
			scanf("%x %x", &addr, &val);
			printf("adjust AD %x %x\n", addr, val);
			o_addr = addr;
			o_val = (unsigned char)val;
			mode = ADJ;
		} else if(strcmp(tmp, "dav") == 0) {
			scanf("%x %f",&val, &refVal);
			tmpVal = (short int)(32767.0*refVal/2.5);
			printf("V reference : %1.3f %04X\n", refVal, tmpVal);
			txData.val = (short int)tmpVal;
			outb((val & 0xFF), 0x640);	//REF_HIGH
			outb(txData.data[1], 0x642);	//REF_HIGH
			outb(txData.data[0], 0x643);	//REF_VLT
		} else if(strcmp(tmp, "dai") == 0) {
			scanf("%x %f",&val, &refVal);
			tmpVal = (short int)(32767.0*refVal/2.5);
			printf("I reference : %1.3f %04X\n", refVal, tmpVal);
			txData.val = (short int)tmpVal;
			outb((val & 0xFF), 0x641);	//REF_HIGH
			outb(txData.data[1], 0x642);	//REF_HIGH
			outb(txData.data[0], 0x643);	//REF_VLT
		} else if(strcmp(tmp, "v") == 0) {
			scanf("%f", &refVal);
			tmpVal = (short int)(29491.0*refVal/9.0);
			printf("V reference : %1.3f %04X\n", refVal, tmpVal);
			txData.val = tmpVal;
			outb(txData.data[1], 0x522);	//REF_HIGH
			outb(txData.data[0], 0x523);	//REF_VLT
		} else if(strcmp(tmp, "i") == 0) {
			scanf("%f", &refVal);
			tmpVal = (short int)(29491.0*5.0*refVal/(2000.0*9.0));
			printf("I reference : %4.2f %02X\n", refVal, (short int)tmpVal);
			txData.val = tmpVal;
			outb(txData.data[1], 0x522);	//REF_HIGH
			outb(txData.data[0], 0x524);	//REF_CRRNT
		} else if(strcmp(tmp, "daloop") == 0) {
			scanf("%d", &addr);
			printf("daloop %d\n", addr);
			o_addr = addr - 1;
			o_addr &= 0xFF;
			count = 0x00;
			mode = DACHECK;
		} else if(strcmp(tmp, "r") == 0) {
			scanf("%d %d", &addr, &val);	//addr : channel val : on off
			printf("relay %d %d\n", addr, val);
			o_addr = 0x520 + (addr - 1)/8;
			o_val = 0x01;
			if(val == 0) o_val = 0x00;
			else o_val = o_val << ((addr - 1)%8);
			outb(o_val, o_addr);
		} else if(strcmp(tmp, "pv") == 0) {//LG_Cycler
			system("rm -rf refic.csv");
			muxVal1=0x10; //5V
			muxVal2=0x10;

			outb(muxVal2, 0x5A8);//MUX_CHAR
			usleep(50);
			outb(muxVal1, 0x5A9);//MUX_CHEN
			usleep(50);

			outb(0x00, 0x5AB);	//AD_START
			adData.data[1]=0;
			adData.data[0]=0;
			usleep(10000);
			for(j=0; j<MAX_AD_COUNT; j++){
				adData.data[1] = inb(0x5AB);//AD_HADDR 
				adData.data[0] = inb(0x5AA);//AD_LADDR 
				REFIC[j] = adData.val;
			}
			printf("read %2.3fV adVal: %x\n", (adData.val*9.0/29491),
					adData.val);
			fp=fopen("refic.csv","a");
			if(fp==NULL){
			   printf("refic.csv read fail\n");
			   fclose(fp);
			}

			for(j=0; j<MAX_AD_COUNT; j++){
				fprintf(fp,"%d,	%2.3f\n",
						j+1,
						REFIC[j]*9.0/29491);
			}//Num, 5V
			fclose(fp);
		} else if(strcmp(tmp, "quit") == 0) exit(0);

		FD_ZERO(&rfds);
		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
		if(retval != 0) continue;

		switch(mode) {
			case OUTPUT:
				outb(o_val, o_addr);
				break;
			case INPUT:
				i_val = inb(i_addr);
				printf("%02X",i_val);
				break;
			case AD:
				switch(phase1) {
					case 0:
						if(type == 0) {
							outb(0x00, 0x650);	//AD_START
						} else {
							outb(0x01, 0x650);	//AD_START
						}
						phase1 = 1;
						break;
					case 1:
						for(i=0; i < 3; i++) {
							j = 0x650 + 2 * i;
							adVal = 0;
							adVal = inb(j);	//AD_HADDR
							adVal2 = inb(j+1);//AD_LADDR
							tmpVal = 0;
							tmpVal = adVal;
							tmpVal = tmpVal << 8;
							tmpVal |= adVal2;
							if(type == 0) {
								printf("adv %d %f\n", tmpVal,
									(float)tmpVal/32768.0*10000.0);
								} else {
								printf("adi %d %f\n", tmpVal,
									(float)tmpVal/32768.0*190618.0);
							}
						}
						mode = NONE;
						phase1 = 0;
						break;
					default: break;
				}
				break;
			case ADJ:
				outb(o_addr, 0x529);
				outb(o_val, 0x52a);
				break;
			case DACHECK:
				outb(o_addr, 0x529);
				outb(count, 0x52a);
				count++;
				if(count >= 0xff) count = 0x00;
				break;
			default: break;
		}
	}
    exit(0);
}

