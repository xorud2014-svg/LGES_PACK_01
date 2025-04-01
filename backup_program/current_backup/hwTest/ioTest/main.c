#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <asm/io.h>

#define NONE	0
#define OUTPUT	1
#define INPUT	2
#define GET		3
#define AD		4
#define ADJ		5
#define DACHECK	6
#define AD_TEMP	7
#define MAX_AD_COUNT	1000

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

int swapBit(int ch) {
  	int i, result;
	int array[4];
	result &= 0x00;
	if(ch & 0x01)
		array[0] = 0x08;
	else
		array[0] = 0x00;
	if(ch & 0x02)
		array[1] = 0x04;
	else
		array[1] = 0x00;
	if(ch & 0x04)
		array[2] = 0x02;
	else
		array[2] = 0x00;
	if(ch & 0x08)
		array[3] = 0x01;
	else
		array[3] = 0x00;
	
	for(i=0; i<4; i++)
		result |= array[i];
	return result;
}

long sum_ints_from_string(char *string){
	long next;
	char *tail;
	next = strtol(string,&tail,16);

	return next;
}

int main(int argc, char *argv[])
{
    int		retval, addr, val, type, mode, phase;
	int		o_addr, i_addr, count;
    int 	low,i,j, ch;
	short int	tmpVal;
    float	refVal;
    char	tmp[10];
	unsigned char	sync, o_val, i_val, muxVal, muxVal1, muxVal2, adVal, adVal2, Addr;
	union	tmpData txData;
	union	ADData adData;
    struct	timeval tv;
    fd_set	rfds;
	FILE *fp;

	mode = NONE;

    if(iopl(3)) exit(1);

	if(argc !=1 ) {
		o_addr = (int)sum_ints_from_string(argv[1]);
		val = (int)sum_ints_from_string(argv[2]);
		printf("o_addr = %x, val = %x\n", o_addr, val);
		outb(val, o_addr);
		exit(0);
	}

	while(1) {
		printf(">> ");
		scanf("%s", tmp);
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
			outb(o_val, o_addr);
		} else if(strcmp(tmp, "read") == 0) {
			scanf("%x", &addr);
			printf("read %x\n", addr);
			i_addr = addr;
			mode = INPUT;
		} else if(strcmp(tmp, "get") == 0) {
			scanf("%x", &addr);
			printf("get %x\n", addr);
			mode = NONE;
			i_addr = addr;
			i_val = inb(i_addr);
			printf("get data %x %x\n", i_addr, i_val);
		} else if(strcmp(tmp, "adj") == 0) {
			scanf("%x %x", &addr, &val);
			printf("adjust AD %x %x\n", addr, val);
			o_addr = addr;
			o_val = (unsigned char)val;
			mode = ADJ;
		} else if(strcmp(tmp, "aux") == 0) {
			scanf("%x %x", &ch, &val);
			printf("AUX DA %x %x\n", ch, val);
			sync = 0x01;
			sync = sync << (ch / 8);
			outb(sync, 0x625); 
			muxVal = ((ch%8) << 4);
			muxVal |= ((val) >> 4);
			outb(muxVal, 0x626);
			low = val << 4;
			outb(low, 0x627);
			printf("sync : %x, high %02X, low : %02x\n",sync, muxVal, low);
		} else if(strcmp(tmp, "v") == 0) {
			scanf("%f", &refVal);
			tmpVal = (short int)(29491.0*refVal/9.0);
			printf("V reference : %1.3f %02X\n", refVal, tmpVal);
			txData.val = tmpVal;
			outb(txData.data[1], 0x622);	//REF_HIGH
			outb(txData.data[0], 0x623);	//REF_VLT
		} else if(strcmp(tmp, "i") == 0) {
			scanf("%f", &refVal);
			tmpVal = (short int)(29491.0*8.0*refVal/(2000.0*9.0));
			printf("I reference : %4.2f %02X\n", refVal, (short int)tmpVal);
			txData.val = tmpVal;
			outb(txData.data[1], 0x622);	//REF_HIGH
			outb(txData.data[0], 0x624);	//REF_CRRNT
		} else if(strcmp(tmp, "r") == 0) {
			scanf("%d %d", &addr, &val);	//addr : channel val : on off
			printf("relay %d %d\n", addr, val);
			o_addr = 0x620 + (addr - 1)/8;
			o_val = 0x01;
			if(val == 0) o_val = 0x00;
			else o_val = o_val << ((addr - 1)%8);
			outb(0x01, 0x62f);
			outb(o_val, o_addr);
			outb(0x00, 0x62f);
		} else if(strcmp(tmp, "daloop") == 0) {
			scanf("%d", &addr);
			printf("daloop %d\n", addr);
			o_addr = addr - 1;
			o_addr &= 0xFF;
			count = 0x00;
			mode = DACHECK;
		} else if(strcmp(tmp, "t") == 0) {
			scanf("%d", &addr);
			printf("temp. %d\n", addr);
			if(addr < 1 || addr > 16)
				printf("temp mux out of range\n");
			else {
				o_addr = addr - 1;
				muxVal = o_addr & 0x0f;
				outb(muxVal, 0x50a);
				mode = AD_TEMP;
				phase = 0;
			}
		} else if(strcmp(tmp, "quit") == 0) exit(0);

		FD_ZERO(&rfds);
		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
		if(retval == 0) {
			if(mode == OUTPUT)
				outb(o_val, o_addr);
			else if(mode == INPUT)
				i_val = inb(i_addr);
			else if(mode == AD) {
				switch(phase) {
					case 0:
						outb(0x00, 0x521);	//AD_START
						phase = 1;
						break;
					case 1:
						adVal = 0;
						adVal = inb(0x521);
						adVal2 = inb(0x520);
						tmpVal = 0;
						tmpVal = adVal;
						tmpVal = tmpVal << 8;
						tmpVal |= adVal2;
						if(type == 0)
							printf("write %2.3fV %x\n", (tmpVal*9.0/29491),
								adVal);
						else
							printf("write %4.3fmA %x\n",
								(tmpVal*2000.0/16383.88), adVal);
						printf("ad value : %02X\n", adVal);
						//printf("ad value : %x\n", tmpVal);
						//printf("volt : %4.2f\n", (float)tmpVal*30.519441);
						mode = NONE;
						phase = 0;
						break;
				}
			} else if(mode == AD_TEMP) {
				switch(phase) {
					case 0:
						outb(0x00, 0x509);
						printf("kjg1\n");
						phase = 1;
						break;
					case 1:
						adVal = 0;
						adVal = inb(0x509);
						adVal2 = inb(0x508); 
						tmpVal &= 0x00;
						tmpVal |= adVal << 8;
						tmpVal |= adVal2;
						printf("Temperature AD value : %02X\n", tmpVal);
						mode = 0;
						printf("kjg2\n");
						phase = 0;
						break;
				}
			} else if(mode == ADJ) {
				outb(o_addr, 0x529);
				outb(o_val, 0x52a);
			} else if(mode == DACHECK) {
				outb(o_addr, 0x529);
				outb(count, 0x52a);
				count++;
				if(count >= 0xff) count = 0x00;
			}
		}
	}
    exit(0);
}
