#include <stdio.h>
//#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/io.h>

#define F_REF_ERROR 10
#define F_AD_ERROR 15
#define WRITE_WAIT 1000
#define AD_WAIT 10000

void dio_test1(int);
void fch_dio_test(void);
void fch_ref_ad_test(void);
void fch_ad_test1(int, int, int);
void fch_ad_test2(int, int);

unsigned char wr_data[10], rd_data[10];

int main(void)
{
	char tmp[10];

	if(iopl(3)) exit(1);

	memset((char *)&tmp[0], 0, 10);

	while(1) {
		printf("scan1>> ");
		scanf("%s", tmp);

		if(strcmp(tmp, "dio") == 0) {
			dio_test1(0x610);
			dio_test1(0x611);
			dio_test1(0x620);
			dio_test1(0x621);
			dio_test1(0x622);
			dio_test1(0x623);
		} else if(strcmp(tmp, "fch_dio") == 0) {
			fch_dio_test();
		} else if(strcmp(tmp, "fch_ad_da") == 0) {
			fch_ref_ad_test();
			fch_ad_test1(5, -5, 0);
			fch_ad_test1(-5, 5, 0);
			fch_ad_test1(5, -5, 1);
			fch_ad_test1(-5, 5, 1);
			fch_ad_test2(5, -5);
			fch_ad_test2(-5, 5);
		} else if(strcmp(tmp, "quit") == 0) {
			exit(0);
		}
	}
}

void dio_test1(int o_addr)
{
	unsigned char o_val, i_val, cnt=0, i;
	
	o_val = 0x01;
	for(i=0; i < 8; i++) {
		outb(o_val, o_addr);
		usleep(WRITE_WAIT);

		i_val = inb(o_addr);
		if(o_val != i_val) {
			printf("fail address %x, out %x, in %x\n", o_addr, o_val, i_val);
			cnt++;
		}
		usleep(WRITE_WAIT);

		o_val = o_val << 0x01;
	}

	outb(0x00, o_addr);

	if(cnt == 0) {
		printf("%x ADDRESS GOOD!!!!\n\n", o_addr);
	} else {
		printf("%x ADDRESS FAIL!!!!\n\n", o_addr);
	}
}

void fch_dio_test(void)
{
	unsigned char o_val, cnt=0, i, j, k;
	int tx_addr;

	tx_addr = 0x750;

	o_val = 0x01;
	for(i=0; i < 8; i++) {
		memset((char *)&wr_data[0], 0, 10);
		wr_data[0] = 0x00;
		wr_data[1] = 0x11;
		wr_data[2] = o_val;
		wr_data[3] = o_val;

		for(k=0; k < 3; k++) {
			outb(0x00, 0x75F);
			for(j=0; j < 10; j++) {
				outb(wr_data[j], tx_addr + j);
			}
			usleep(WRITE_WAIT);
		}

		memset((char *)&wr_data[0], 0, 10);
		wr_data[0] = 0x00;
		wr_data[1] = 0x12;

		for(k=0; k < 3; k++) {
			outb(0x00, 0x75F);
			for(j=0; j < 10; j++) {
				outb(wr_data[j], tx_addr + j);
			}
			usleep(WRITE_WAIT);
		}
	
		outb(0x00, 0x75E);
		for(j=0; j < 10; j++) {
			rd_data[j] = inb(tx_addr + j);
		}
		if(o_val != rd_data[2]) {
			printf("fail out %x, in %x\n", o_val, rd_data[2]);
			cnt++;
		}
		usleep(500000);

		o_val = o_val << 0x01;
	}

	o_val = 0x00;
	memset((char *)&wr_data[0], 0, 10);
	wr_data[0] = 0x00;
	wr_data[1] = 0x11;
	wr_data[2] = o_val;
	wr_data[3] = o_val;

	for(k=0; k < 3; k++) {
		outb(0x00, 0x75F);
		for(j=0; j < 10; j++) {
			outb(wr_data[j], tx_addr + j);
		}
		usleep(WRITE_WAIT);
	}

	if(cnt == 0) {
		printf("\nFCH ADDRESS GOOD!!!!\n\n");
	} else {
		printf("\nFCH ADDRESS FAIL!!!!\n\n");
	}
}

void fch_ref_ad_test(void)
{
	int i, j, tx_addr, val1, val2, val3;

	tx_addr = 0x750;

	memset((char *)&wr_data[0], 0, 10);
	wr_data[0] = 0x00;
	wr_data[1] = 0x31;

	for(j=0; j < 3; j++) {
		outb(0x00, 0x75F);
	    for(i=0; i < 10; i++) {
		    outb(wr_data[i], tx_addr + i);
		}
		usleep(WRITE_WAIT);
	}

	outb(0x00, 0x75E);
   	for(i=0; i < 10; i++) {
		rd_data[i] = inb(tx_addr + i);
	}
	usleep(WRITE_WAIT);

	memcpy((char *)&val1, (char *)&rd_data[2], 4);
	memcpy((char *)&val2, (char *)&rd_data[6], 4);
	val1 /= 1000;
	val2 /= 1000;

	memset((char *)&wr_data[0], 0, 10);
	wr_data[0] = 0x00;
	wr_data[1] = 0x32;

	for(j=0; j < 3; j++) {
		outb(0x00, 0x75F);
	    for(i=0; i < 10; i++) {
		    outb(wr_data[i], tx_addr + i);
		}
		usleep(WRITE_WAIT);
	}

	outb(0x00, 0x75E);
   	for(i=0; i < 10; i++) {
		rd_data[i] = inb(tx_addr + i);
		usleep(WRITE_WAIT);
   	}

	memcpy((char *)&val3, (char *)&rd_data[2], 4);
	val3 /= 1000;

	printf("\n");
	printf("+5V : %dmV, -5V : %dmV, 0V : %dmV\n", val1, val2, val3);
	val1 -= 5000;
	val2 += 5000;
	
	if((abs(val1) > F_REF_ERROR) || (abs(val2) > F_REF_ERROR)
		|| (abs(val3) > F_REF_ERROR)) {
		printf("FCH REF AD FAIL!!!!\n");
	} else {
		printf("FCH REF AD GOOD!!!!\n");
	}
}

void fch_ad_test1(int o_val1, int o_val2, int div)
{
	int i, j, tx_addr, val1, val2;

	tx_addr = 0x750;

	memset((char *)&wr_data[0], 0, 10);
	wr_data[0] = 0x00;
	wr_data[1] = 0x21;

	val1 = o_val1 * 1000000;
	if(div == 0) {
		val2 = o_val2 * 1000000;
	} else {
		val2 = (int)((float)o_val2 * 1000000.0 / 42.1667);
	}
	memcpy((char *)&wr_data[2], (char *)&val1, 4);
	memcpy((char *)&wr_data[6], (char *)&val2, 4);

	for(j=0; j < 3; j++) {
		outb(0x00, 0x75F);
		for(i=0; i < 10; i++) {
		    outb(wr_data[i], tx_addr + i);
		}
		usleep(WRITE_WAIT);
	}

	usleep(100000);

	memset((char *)&wr_data[0], 0, 10);
	wr_data[0] = 0x00;
	wr_data[1] = 0x33;

	for(j=0; j < 3; j++) {
		outb(0x00, 0x75F);
		tx_addr = 0x750;
		for(i=0; i < 10; i++) {
			outb(wr_data[i], tx_addr + i);
		}
		usleep(WRITE_WAIT);
	}

	outb(0x00, 0x75E);
   	for(i=0; i < 10; i++) {
		rd_data[i] = inb(tx_addr + i);
    }
	usleep(WRITE_WAIT);

	memcpy((char *)&val1, (char *)&rd_data[2], 4);
	memcpy((char *)&val2, (char *)&rd_data[6], 4);
	val1 /= 1000;
	val2 /= 1000;

	printf("\n");
	printf("DA1 %dmV, %dmV : AD1 %dmV, %dmV\n",
		o_val1*1000, o_val2*1000, val1, val2);

	val1 -= (o_val1*1000);
	if(div == 0) {
		val2 -= (o_val2*1000);

		if(abs(val1) > F_AD_ERROR) {
			printf("FCH DA1 AD1 FAIL!!!!\n");
		} else {
			printf("FCH DA1 AD1 GOOD!!!\n");
		}
	} else {
		val2 -= (int)((float)o_val2 * 1000.0 / 42.1667);

		if(abs(val2) > F_AD_ERROR) {
			printf("FCH DA1 AD3 FAIL!!!!\n");
		} else {
			printf("FCH DA1 AD3 GOOD!!!\n");
		}
	}

	if(div != 0) return;
	
	memset((char *)&wr_data[0], 0, 10);
	wr_data[0] = 0x00;
	wr_data[1] = 0x34;

	for(j=0; j < 3; j++) {
		outb(0x00, 0x75F);
		tx_addr = 0x750;
		for(i=0; i < 10; i++) {
			outb(wr_data[i], tx_addr + i);
		}
		usleep(WRITE_WAIT);
	}

	outb(0x00, 0x75E);
   	for(i=0; i < 10; i++) {
		rd_data[i] = inb(tx_addr + i);
    }
	usleep(WRITE_WAIT);

	memcpy((char *)&val1, (char *)&rd_data[2], 4);
	memcpy((char *)&val2, (char *)&rd_data[6], 4);
	val1 /= 1000;
	val2 /= 1000;

	printf("\n");
	printf("DA1 %dmV, %dmV : AD2 %dmV, %dmV\n",
		o_val1*1000, o_val2*1000, val1, val2);

	val1 -= (o_val1*1000);
	val2 -= (o_val2*1000);

	if((abs(val1) > F_AD_ERROR) || (abs(val2) > F_AD_ERROR)) {
		printf("FCH DA1 AD2 FAIL!!!!\n");
	} else {
		printf("FCH DA1 AD2 GOOD!!!\n");
	}
}

void fch_ad_test2(int o_val1, int o_val2)
{
	int i, j, tx_addr, val1, val2;

	tx_addr = 0x750;

	memset((char *)&wr_data[0], 0, 10);
	wr_data[0] = 0x00;
	wr_data[1] = 0x22;

	val1 = o_val1 * 1000000;
	val2 = o_val2 * 1000000;
	memcpy((char *)&wr_data[2], (char *)&val1, 4);
	memcpy((char *)&wr_data[6], (char *)&val2, 4);

	for(j=0; j < 3; j++) {
		outb(0x00, 0x75F);
		for(i=0; i < 10; i++) {
		    outb(wr_data[i], tx_addr + i);
		}
		usleep(WRITE_WAIT);
	}

	usleep(100000);

	memset((char *)&wr_data[0], 0, 10);
	wr_data[0] = 0x00;
	wr_data[1] = 0x35;

	for(j=0; j < 3; j++) {
		outb(0x00, 0x75F);
		tx_addr = 0x750;
		for(i=0; i < 10; i++) {
			outb(wr_data[i], tx_addr + i);
		}
		usleep(WRITE_WAIT);
	}

	outb(0x00, 0x75E);
   	for(i=0; i < 10; i++) {
		rd_data[i] = inb(tx_addr + i);
    }
	usleep(WRITE_WAIT);

	memcpy((char *)&val1, (char *)&rd_data[2], 4);
	memcpy((char *)&val2, (char *)&rd_data[6], 4);
	val1 /= 1000;
	val2 /= 1000;

	printf("\n");
	printf("DA2 %dmV, %dmV : AD3 %dmV, %dmV\n",
		o_val1*1000, o_val2*1000, val1, val2);

	val1 -= (o_val1*1000);
	val2 -= (o_val2*1000);
	//if((abs(val1) > F_AD_ERROR) || (abs(val2) > F_AD_ERROR)) {
	if(abs(val1) > F_AD_ERROR) {
		printf("FCH DA2 AD3 FAIL!!!!\n");
	} else {
		printf("FCH DA2 AD3 GOOD!!!\n");
	}
}
