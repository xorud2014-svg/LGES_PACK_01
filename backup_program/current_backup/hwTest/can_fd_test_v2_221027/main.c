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
    char tmp[16];
	unsigned char o_val, i_val; 
	unsigned char muxVal1, muxVal2;
    unsigned char adVal, adVal2, tx_count;
	//unsigned char kjg_val[4];
	unsigned char kjg_val[128];
	short int tmpVal;
    int	retval, addr, val, type, mode, phase1;
	int	o_addr, i_addr, count;
    int	i, j, k, can_ch, tmp1, tmp2;
//	union tmpData txData;
	union ADData adData;
    struct timeval tv;
    fd_set rfds;
	FILE *fp;

	phase1 = type = o_addr = i_addr = count = 0;
	o_val = muxVal2 = 0;
	tmp1 = tmp2 = 0;

	mode = NONE;

    if(iopl(3)) exit(1);

	while(1) {
		tmp[0] = 0;
		if(mode == NONE) {
			printf("scan1>> ");
			scanf("%s", tmp);
		}

		if(strcmp(tmp, "ver") == 0) {
			mode = NONE;

			kjg_val[0] = inb(0x885);
			kjg_val[1] = inb(0x886);
			kjg_val[2] = inb(0x887);
			printf("bd1 ver %x %x %x\n", kjg_val[0], kjg_val[1], kjg_val[2]);

			kjg_val[0] = inb(0x88A);
			kjg_val[1] = inb(0x88B);
			kjg_val[2] = inb(0x88C);
			printf("bd2 ver %x %x %x\n", kjg_val[0], kjg_val[1], kjg_val[2]);
		} else if(strcmp(tmp, "reset") == 0) {
			printf("\n");
			mode = NONE;

			outb(0, 0x884); //kjg_220618
			outb(0, 0x889); //kjg_220618
			usleep(100000);

			kjg_val[0] = inb(0x885);
			kjg_val[1] = inb(0x886);
			kjg_val[2] = inb(0x887);
			printf("bd1 ver %x %x %x\n", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x41 || kjg_val[0] == 'a'
				|| kjg_val[0] == 'T' || kjg_val[0] == 't') { //'A'
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x88A);
			kjg_val[1] = inb(0x88B);
			kjg_val[2] = inb(0x88C);
			printf("bd2 ver %x %x %x\n", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x41 || kjg_val[0] == 'a'
				|| kjg_val[0] == 'T' || kjg_val[0] == 't') { //'A'
				tmp2 = 8;
			} else tmp2 = 4;

			tx_count = 2;

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				o_val = 0x01;
				o_val = o_val << can_ch;
				outb(o_val, 0x880);

				outb(tx_count, 0x801);
				outb(0x00, 0x802);
				outb(0x00, 0x802);
				outb(0x00, 0x803);

				for(i=0; i < tx_count; i++) {
					kjg_val[i] = 0xAA;
					usleep(5);
				}

				i_val = inb(0x801);
				printf("can_ch : %d, get tx_count %d\n", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(0x802);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				}

				i_val = inb(0x803);
				printf("0x803 : %02x\n", i_val);

				if(kjg_val[0] == 0x00 && kjg_val[1] == 0x00) {
					k++;
				}
			}

			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d %d\n", tmp1, tmp2);
				else printf("NG %d %d\n", tmp1, tmp2);
			} else {
				if(k == 8) printf("OK %d %d\n", tmp1, tmp2);
				else printf("NG %d %d\n", tmp1, tmp2);
			}
		} else if(strcmp(tmp, "test1") == 0
			|| (strcmp(tmp, "t1") == 0)) {
			printf("\n");
			mode = NONE;

			if((tmp1 == 0 && tmp2 == 0) || (tmp1 == 4 && tmp2 == 4)) {
				printf("first request reset : %d, %d\n", tmp1, tmp2);
			}

			tx_count = 4;
			k = 0;

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				o_val = 0x01;
				o_val = o_val << can_ch;
				outb(o_val, 0x880);

				outb(tx_count, 0x801);
				outb(0x2E, 0x802); outb(0x0C, 0x802);
				outb(can_ch, 0x802); outb(can_ch+1, 0x802);
				outb(0x00, 0x803);

				for(i=0; i < tx_count; i++) {
					kjg_val[i] = 0xAA;
					usleep(5);
				}

				i_val = inb(0x801);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(0x802);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n, ");
				}

				i_val = inb(0x803);
				printf("0x803 : %02x\n", i_val);

				if(kjg_val[0] == 0x00 && kjg_val[1] == 0x00
					&& kjg_val[2] == 0x00 && kjg_val[3] == 0x00) {
					k++;
				}
			}

			if(k != 0) printf("\n read\n");

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				o_val = 0x01;
				o_val = o_val << can_ch;
				outb(o_val, 0x880);

				outb(tx_count, 0x801);
				outb(0x3E, 0x802); outb(0x0C, 0x802);
				outb(0x00, 0x802); outb(0x00, 0x802);
				outb(0x00, 0x803);

				for(i=0; i < tx_count; i++) {
					kjg_val[i] = 0xAA;
					usleep(5);
				}

				i_val = inb(0x801);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(0x802);
						printf("%02x ", kjg_val[i]);
					}
					printf(", ");
				}

				i_val = inb(0x803);
				printf("0x803 : %02x\n", i_val);

				if(kjg_val[0] == 0x00 && kjg_val[1] == 0x00
					&& kjg_val[2] == can_ch && (kjg_val[3] == can_ch+1)) {
					k++;
				}
			}

			printf("test1 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 8) printf("OK %d\n", k);
				else printf("NG %d\n", k);
			} else {
				if(k == 16) printf("OK %d\n", k);
				else printf("NG %d\n", k);
			}
		} else if(strcmp(tmp, "test2") == 0
			|| (strcmp(tmp, "t2") == 0)) {
			printf("\n");
			mode = NONE;

			tmp1 = 0; tmp2 = 8;

			tx_count = 4;
			k = 0;

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				o_val = 0x01;
				o_val = o_val << can_ch;
				outb(o_val, 0x880);

				outb(tx_count, 0x801);
				//outb(0x2E, 0x802); outb(0x0C, 0x802); //ok

				//outb(0x20, 0x802); outb(0x68, 0x802); //0x68:ok, 0x69:04
				//outb(0x20, 0x802); outb(0x6C, 0x802); //0x6C:00, 0x6D:00
				//outb(0x20, 0x802); outb(0x5C, 0x802); //0x5C:ok, 0x5D:04
				outb(0x20, 0x802); outb(0x58, 0x802); //0x58:08, 0x59:00

				outb(can_ch, 0x802); outb(can_ch+1, 0x802);
				outb(0x00, 0x803);

				for(i=0; i < tx_count; i++) {
					kjg_val[i] = 0xAA;
					usleep(5);
				}

				i_val = inb(0x801);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(0x802);
						printf("%02x ", kjg_val[i]);
					}
					printf(", ");
				}

				i_val = inb(0x803);
				printf("0x803 : %02x\n", i_val);

				if(kjg_val[0] == 0x00 && kjg_val[1] == 0x00
					&& kjg_val[2] == 0x00 && kjg_val[3] == 0x00) {
					k++;
				}
			}

			if(k != 0) printf("\n read\n");

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				o_val = 0x01;
				o_val = o_val << can_ch;
				outb(o_val, 0x880);

				outb(tx_count, 0x801);
				//outb(0x3E, 0x802); outb(0x0C, 0x802);
				
				//outb(0x30, 0x802); outb(0x68, 0x802);
				//outb(0x30, 0x802); outb(0x6C, 0x802);
				//outb(0x30, 0x802); outb(0x5C, 0x802);
				outb(0x30, 0x802); outb(0x58, 0x802);

				outb(0x00, 0x802); outb(0x00, 0x802);
				outb(0x00, 0x803);

				for(i=0; i < tx_count; i++) {
					kjg_val[i] = 0xAA;
					usleep(5);
				}

				i_val = inb(0x801);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(0x802);
						printf("%02x ", kjg_val[i]);
					}
					printf(", ");
				}

				i_val = inb(0x803);
				printf("0x803 : %02x\n", i_val);

				if(kjg_val[0] == 0x00 && kjg_val[1] == 0x00
					&& kjg_val[2] == can_ch && (kjg_val[3] == can_ch+1)) {
					k++;
				}
			}

			printf("test2 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 8) printf("OK %d\n", k);
				else printf("NG %d\n", k);
			} else {
				if(k == 16) printf("OK %d\n", k);
				else printf("NG %d\n", k);
			}
		} else if(strcmp(tmp, "test3") == 0
			|| (strcmp(tmp, "t3") == 0)) {
			printf("\n");
			mode = NONE;

			tmp1 = 0; tmp2 = 8;

			tx_count = 4;
			k = 0;

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				o_val = 0x01;
				o_val = o_val << can_ch;
				outb(o_val, 0x880);

				outb(tx_count, 0x801);
				//outb(0x3E, 0x802); outb(0x0C, 0x802);

				//outb(0x30, 0x802); outb(0x68, 0x802);
				//outb(0x30, 0x802); outb(0x6C, 0x802);
				//outb(0x30, 0x802); outb(0x5C, 0x802);
				outb(0x30, 0x802); outb(0x58, 0x802);

				outb(0x00, 0x802); outb(0x00, 0x802);
				outb(0x00, 0x803);

				for(i=0; i < tx_count; i++) {
					kjg_val[i] = 0xAA;
					usleep(5);
				}

				i_val = inb(0x801);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(0x802);
						printf("%02x ", kjg_val[i]);
					}
					printf(", ");
				}

				i_val = inb(0x803);
				printf("0x803 : %02x\n", i_val);

				if(kjg_val[0] == 0x00 && kjg_val[1] == 0x00
					&& kjg_val[2] == can_ch && (kjg_val[3] == can_ch+1)) {
					k++;
				}
			}

			printf("test3 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 8) printf("OK %d\n", k);
				else printf("NG %d\n", k);
			} else {
				if(k == 16) printf("OK %d\n", k);
				else printf("NG %d\n", k);
			}
		} else if(strcmp(tmp, "test9") == 0) {
			printf("\n");
			mode = NONE;

			if((tmp1 == 0 && tmp2 == 0) || (tmp1 == 4 && tmp2 == 4)) {
				printf("first request reset : %d, %d\n", tmp1, tmp2);
			}

			tx_count = 4;
			k = 0;

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x811 + (3 * can_ch);
				} else {
					o_addr = 0x821 + (3 * (can_ch - 4));
				}
				outb(tx_count, o_addr);
				outb(0x2E, o_addr + 1);
				outb(0x0C, o_addr + 1);
				outb(can_ch, o_addr + 1);
				outb(can_ch+1, o_addr + 1);
				outb(0x00, o_addr + 2);

				for(i=0; i < tx_count; i++) {
					kjg_val[i] = 0xAA;
					usleep(5);
				}

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d\n", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr + 1);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				}

				i_val = inb(o_addr + 2);
				printf("0x%03x : %02x\n", o_addr + 2, i_val);

				if(kjg_val[0] == 0x00 && kjg_val[1] == 0x00
					&& kjg_val[2] == 0x00 && kjg_val[3] == 0x00) {
					k++;
				}
			}

			if(k != 0) printf("\n read\n");

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x811 + (3 * can_ch);
				} else {
					o_addr = 0x821 + (3 * (can_ch - 4));
				}
				outb(tx_count, o_addr);
				outb(0x3E, o_addr + 1);
				outb(0x0C, o_addr + 1);
				outb(0x00, o_addr + 1);
				outb(0x00, o_addr + 1);
				outb(0x00, o_addr + 2);

				for(i=0; i < tx_count; i++) {
					kjg_val[i] = 0xAA;
					usleep(5);
				}

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d\n", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr + 1);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				}

				i_val = inb(o_addr + 2);
				printf("0x%03x : %02x\n", o_addr + 2, i_val);

				if(kjg_val[0] == 0x00 && kjg_val[1] == 0x00
					&& kjg_val[2] == can_ch && (kjg_val[3] == can_ch+1)) {
					k++;
				}
			}

			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 8) printf("OK %d\n", k);
				else printf("NG %d\n", k);
			} else {
				if(k == 16) printf("OK %d\n", k);
				else printf("NG %d\n", k);
			}
		} else if(strcmp(tmp, "test11") == 0) {
			printf("\n");
			mode = NONE;

			outb(1, 0x884);
			outb(1, 0x889);

			if((tmp1 == 0 && tmp2 == 0) || (tmp1 == 4 && tmp2 == 4)) {
				printf("first request reset : %d, %d\n", tmp1, tmp2);
			}

			tx_count = 4;
			k = 0;

			i_addr = 0x838;
			i_val = inb(i_addr);
			if(i_val == 0xF5) printf("OK");
			else printf("NG");
			printf(" : flag01 : %02x\n", i_val);

			o_addr = 0x830;
			outb(tx_count, o_addr);
			//j = 1000;
			//usleep(j);
			i_val = inb(i_addr);
			if(i_val == 0xF4) printf("OK");
			else printf("NG");
			printf(" : flag02 : %02x\n", i_val);

			/*while(1) {
				if(i_val == 0xF4) break;
				usleep(j);
				i_val = inb(i_addr);
				if(i_val == 0xF4) printf("OK");
				else printf("NG");
				printf(" : flag02 : %02x\n", i_val);
			}*/

			kjg_val[0] = 0x74; kjg_val[1] = 0x31; kjg_val[2] = 0x33;
			outb(kjg_val[0], o_addr + 1);
			//usleep(j);
			i_val = inb(i_addr);
			if(i_val == 0xF4) printf("OK");
			else printf("NG");
			printf(" : flag03 : %02x\n", i_val);

			outb(kjg_val[1], o_addr + 1);
			//usleep(j);
			i_val = inb(i_addr);
			if(i_val == 0xF4) printf("OK");
			else printf("NG");
			printf(" : flag04 : %02x\n", i_val);

			outb(kjg_val[2], o_addr + 1);
			//j = 25000; //ok 30ms, 25ms, ng 20ms, 10ms
			j = 5100; //ok 30ms, 25ms, middle 5.1ms, 4.5ms, ng 4ms, 3ms, 2ms, 1ms
			usleep(j);
			i_val = inb(i_addr);
			if(i_val == 0xF9) printf("OK");
			else printf("NG");
			printf(" : flag05 : %02x\n", i_val);

			while(1) {
				if(i_val == 0xF9) break;
				usleep(j);
				i_val = inb(i_addr);
				if(i_val == 0xF9) printf("OK");
				else printf("NG");
				printf(" : flag05 : %02x\n", i_val);
			}

			//j = 1000;
			//usleep(j);
			i_val = inb(o_addr);
			if(i_val == 4) printf("OK");
			else printf("NG");
			printf(" : flag11 : %02x\n", i_val);

			//usleep(j);
			i_val = inb(i_addr);
			if(i_val == 0xF9) printf("OK");
			else printf("NG");
			printf(" : flag12 : %02x\n", i_val);

			//usleep(j);
			i_val = inb(o_addr);
			if(i_val == kjg_val[0]) printf("OK");
			else printf("NG");
			printf(" : flag13 : %02x : %02x\n", i_val, kjg_val[0]);

			//usleep(j);
			i_val = inb(i_addr);
			if(i_val == 0xF9) printf("OK");
			else printf("NG");
			printf(" : flag14 : %02x\n", i_val);

			//usleep(j);
			i_val = inb(o_addr);
			if(i_val == kjg_val[1]) printf("OK");
			else printf("NG");
			printf(" : flag15 : %02x : %02x\n", i_val, kjg_val[1]);

			//usleep(j);
			i_val = inb(i_addr);
			if(i_val == 0xF9) printf("OK");
			else printf("NG");
			printf(" : flag16 : %02x\n", i_val);

			//usleep(j);
			i_val = inb(o_addr);
			if(i_val == kjg_val[2]) printf("OK");
			else printf("NG");
			printf(" : flag17 : %02x : %02x\n", i_val, kjg_val[2]);

			//j = 6000; //ok 30ms, 10ms, 6ms, middle 5ms, ng 3ms
			j = 2000; //ok 30ms, 10ms, 2ms, middle 1ms, 1us, ng 0us
			usleep(j);
			i_val = inb(i_addr);
			if(i_val == 0xF5) printf("OK");
			else printf("NG");
			printf(" : flag18 : %02x\n", i_val);
		} else if(strcmp(tmp, "reset20") == 0
			|| strcmp(tmp, "r20") == 0) {
			mode = NONE;

			sleep(1);

			outb(0x00, 0x884); //MCU_LOCK
			outb(0x00, 0x889);
			usleep(2000);

			outb(0x01, 0x884); //MCU_UNLOCK
			outb(0x01, 0x889);
			sleep(1);

			outb(0xFF, 0x885); //CAN_RESET set
			outb(0xFF, 0x88A);
			usleep(2000);
			outb(0x00, 0x885); //CAN_RESET clear
			outb(0x00, 0x88A);
			usleep(2000);

			kjg_val[0] = inb(0x885);
			kjg_val[1] = inb(0x886);
			kjg_val[2] = inb(0x887);
			printf("bd1 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x88A);
			kjg_val[1] = inb(0x88B);
			kjg_val[2] = inb(0x88C);
			printf("bd2 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp2 = 8;
			} else tmp2 = 4;

			if(tmp1 == 0 && tmp2 == 8) {
				printf("tmp1 %d, tmp2 %d\n", tmp1, tmp2);
			} else {
				printf("tmp1 %d, tmp2 %d, reset20 NG!!!\n", tmp1, tmp2);
				continue;
			}

			tx_count = 3; //(spi_type << 4) || cmd, data1, data2

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x830 + (can_ch * 2);
					i_addr = 0x838 + (can_ch * 2);
				} else {
					o_addr = 0x840 + ((can_ch - 4) * 2);
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					/*if(i >= 10) {
						printf("reset20 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("reset20 fail1 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("reset20 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("reset20 received a : %d, %02x, %d\n", can_ch, i_val, i);

				outb(tx_count, o_addr);
				outb(0x10, o_addr+1); //(spi_type << 4) || cmd
					//spi_tx_rx 0, spi_tx_only 1, spi_rx_only 2
				outb(0x00, o_addr+1); //data1
				outb(0x00, o_addr+1); //data2
				i_val = inb(o_addr+1); //tx_start
				if(tx_count != i_val) printf("reset20 fail_a %d, %d\n", can_ch, i_val);

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0x45) break;
					i++;
					/*if(i >= 10) {
						printf("reset20 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("reset20 fail2 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("reset20 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("reset20 received b : %d, %02x, %d\n", can_ch, i_val, i);

				for(i=0; i < tx_count; i++) kjg_val[i] = 0xAA;

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				//if(i_val == tx_count || i_val == 0) { //kjgw_220622
				if(i_val == 0) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				} else {
					printf("reset20 fail3 : %d, %02x\n", can_ch, i_val);
					continue;
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0x45) break;
					i++;
					/*if(i >= 10) {
						printf("reset20 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("reset20 fail4 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("reset20 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("reset20 received d : %d, %02x, %d\n", can_ch, i_val, i);

				if(kjg_val[1] == 0x00 && kjg_val[2] == 0x00) {
					k++;
				}
			}

			printf("reset20 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("%d %d OK\n", tmp1, tmp2);
				else printf("%d %d NG!!!\n", tmp1, tmp2);
			} else {
				if(k == 8) printf("%d %d OK\n", tmp1, tmp2);
				else printf("%d %d NG!!!\n", tmp1, tmp2);
			}
		} else if(strcmp(tmp, "status20") == 0
			|| strcmp(tmp, "s20") == 0
			|| strcmp(tmp, "s") == 0) {
			printf("status ");
			for(can_ch=0; can_ch < 8; can_ch++) {
				if(can_ch < 4) {
					if(can_ch == 0) printf("0x884=%02xh, ", inb(0x884));
					i_addr = 0x838 + (can_ch * 2);
				} else {
					if(can_ch == 4) printf("0x889=%02xh, ", inb(0x889));
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i_val = inb(i_addr);
				printf("%02x ", i_val);
				if(can_ch == 3) printf(", ");
			}
			printf("\n");
		} else if(strcmp(tmp, "test20") == 0
			|| strcmp(tmp, "t20") == 0) {
			mode = NONE;

			kjg_val[0] = inb(0x885);
			kjg_val[1] = inb(0x886);
			kjg_val[2] = inb(0x887);
			printf("bd1 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x88A);
			kjg_val[1] = inb(0x88B);
			kjg_val[2] = inb(0x88C);
			printf("bd2 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp2 = 8;
			} else tmp2 = 4;

			if(tmp1 == 0 && tmp2 == 8) {
				printf("tmp1 %d, tmp2 %d\n", tmp1, tmp2);
			} else {
				printf("tmp1 %d, tmp2 %d, test20 NG!!!\n", tmp1, tmp2);
				continue;
			}

			tx_count = 5; //(spi_type << 4) || cmd, data1~4

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x830 + (can_ch * 2);
					i_addr = 0x838 + (can_ch * 2);
				} else {
					o_addr = 0x840 + ((can_ch - 4) * 2);
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5 || i_val == 0x45) break;
					i++;
					/*if(i >= 10) {
						printf("test20 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test20 fail1 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test20 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test20 received a : %d, %02x, %d\n", can_ch, i_val, i);

				outb(tx_count, o_addr);
				outb(0x00, o_addr+1); //(spi_type << 4) || cmd
					//spi_tx_rx 0, spi_tx_only 1, spi_rx_only 2

				outb(0x2E, o_addr+1); outb(0x0C, o_addr+1); //data1, data2

				outb(can_ch, o_addr+1); outb(can_ch+1, o_addr+1); //data3, data4
				i_val = inb(o_addr+1); //tx_start
				if(tx_count != i_val) printf("test20 fail_a %d, %d\n", can_ch, i_val);

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xE3) break;
					i++;
					/*if(i >= 10) {
						printf("test20 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test20 fail2 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test20 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test20 received b : %d, %02x, %d\n", can_ch, i_val, i);

				for(i=0; i < tx_count; i++) kjg_val[i] = 0xAA;

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				} else {
					printf("test20 fail3 : %d, %02x\n", can_ch, i_val);
					continue;
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					/*if(i >= 10) {
						printf("test20 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test20 fail4 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test20 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test20 received d : %d, %02x, %d\n", can_ch, i_val, i);

				if(kjg_val[0] == 0x00
					&& ((kjg_val[1] == 0x00 && kjg_val[2] == 0x00
					&& kjg_val[3] == 0x00 && kjg_val[4] == 0x00)
					|| (kjg_val[1] == 0xFF && kjg_val[2] == 0xFF
					&& kjg_val[3] == 0xFF && kjg_val[4] == 0xFF))) {
					k++;
				}
			}

			printf("k=%d, read\n", k);

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x830 + (can_ch * 2);
					i_addr = 0x838 + (can_ch * 2);
				} else {
					o_addr = 0x840 + ((can_ch - 4) * 2);
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					/*if(i >= 10) {
						printf("test20 fail5 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test20 fail5 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test20 fail5 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test20 received e : %d, %02x, %d\n", can_ch, i_val, i);

				outb(tx_count, o_addr);
				outb(0x00, o_addr+1); //(spi_type << 4) || cmd
					//spi_tx_rx 0, spi_tx_only 1, spi_rx_only 2

				outb(0x3E, o_addr+1); outb(0x0C, o_addr+1); //data1, data2

				outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data3, data4
				i_val = inb(o_addr+1); //tx_start
				if(tx_count != i_val) printf("test20 fail_b %d, %d\n", can_ch, i_val);

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xE3) break;
					i++;
					/*if(i >= 10) {
						printf("test20 fail6 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test20 fail6 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test20 fail6 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test20 received f : %d, %02x, %d\n", can_ch, i_val, i);

				for(i=0; i < tx_count; i++) kjg_val[i] = 0xAA;

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				} else {
					printf("test20 fail7 : %d, %02x\n", can_ch, i_val);
					continue;
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					/*if(i >= 10) {
						printf("test20 fail8 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test20 fail8 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test20 fail8 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test20 received h : %d, %02x, %d\n", can_ch, i_val, i);

				if(kjg_val[0] == 0x00
					&& ((kjg_val[1] == 0x00 && kjg_val[2] == 0x00)
					|| (kjg_val[1] == 0xFF && kjg_val[2] == 0xFF))
					&& kjg_val[3] == can_ch && kjg_val[4] == (can_ch+1)) {
					k++;
				}
			}

			printf("test20 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 8) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			} else {
				if(k == 16) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			}
		} else if(strcmp(tmp, "test21") == 0
			|| strcmp(tmp, "t21") == 0) {
			mode = NONE;

			kjg_val[0] = inb(0x885);
			kjg_val[1] = inb(0x886);
			kjg_val[2] = inb(0x887);
			printf("bd1 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x88A);
			kjg_val[1] = inb(0x88B);
			kjg_val[2] = inb(0x88C);
			printf("bd2 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp2 = 8;
			} else tmp2 = 4;

			if(tmp1 == 0 && tmp2 == 8) {
				printf("tmp1 %d, tmp2 %d\n", tmp1, tmp2);
			} else {
				printf("tmp1 %d, tmp2 %d, test21 NG!!!\n", tmp1, tmp2);
				continue;
			}

			tx_count = 5; //(spi_type << 4) || cmd, data1~4

			k = 0;

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x830 + (can_ch * 2);
					i_addr = 0x838 + (can_ch * 2);
				} else {
					o_addr = 0x840 + ((can_ch - 4) * 2);
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5 || i_val == 0x45) break;
					i++;
					/*if(i >= 10) {
						printf("test21 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test21 fail1 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test21 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test21 received a : %d, %02x, %d\n", can_ch, i_val, i);

				outb(tx_count, o_addr);
				outb(0x00, o_addr+1); //(spi_type << 4) || cmd
					//spi_tx_rx 0, spi_tx_only 1, spi_rx_only 2
				//_CHIP_REG_ECCCON = 0xE0C
				outb(0x3E, o_addr+1); outb(0x0C, o_addr+1); //data1, 2
				
				//_CHIP_REG_CiFIFOSTA + (2 * 12) => TransmitChannelEventGet
				//outb(0x30, o_addr+1); outb(0x6C, o_addr+1); //data1, 2

				//_CHIP_REG_CiCON + 3 => OperationModeSelect
				//outb(0x30, o_addr+1); outb(0x03, o_addr+1); //data1, 2

				//_CHIP_REG_CiINTENABLE => ModuleEventEnable
				//outb(0x30, o_addr+1); outb(0x1E, o_addr+1); //data1, 2

				//_CHIP_REG_CiFIFOCON + (1 * 12) => ReceiveChannelEventEnable
				//outb(0x30, o_addr+1); outb(0x5C, o_addr+1); //data1, 2

				//_CHIP_REG_CiFIFOCON + (2 * 12) => TransmitChannelEventEnable
				//outb(0x30, o_addr+1); outb(0x68, o_addr+1); //data1, 2

				//_CHIP_REG_IOCON + 3 => GpioModeConfigure
				//outb(0x3E, o_addr+1); outb(0x07, o_addr+1); //data1, 2

				//outb(0x00, o_addr+1); //data3
				outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data3, 4
				i_val = inb(o_addr+1); //tx_start
				if(tx_count != i_val) printf("test21 fail_a %d, %d\n", can_ch, i_val);

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xE3) break;
					i++;
					/*printf("test21 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test21 fail2 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test21 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test21 received b : %d, %02x, %d\n", can_ch, i_val, i);

				for(i=0; i < tx_count; i++) kjg_val[i] = 0xAA;

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				} else {
					printf("test21 fail3 : %d, %02x\n", can_ch, i_val);
					continue;
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					/*if(i >= 10) {
						printf("test21 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test21 fail4 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test21 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test21 received d : %d, %02x, %d\n", can_ch, i_val, i);

				if(kjg_val[0] == 0x00
					&& ((kjg_val[1] == 0x00 && kjg_val[2] == 0x00)
					|| (kjg_val[1] == 0xFF && kjg_val[2] == 0xFF))
					&& kjg_val[3] == can_ch && kjg_val[4] == (can_ch+1)) {
					k++;
				}
			}

			printf("test21 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			}
		} else if(strcmp(tmp, "test22") == 0
			|| strcmp(tmp, "t22") == 0) {
			mode = NONE;

			kjg_val[0] = inb(0x885);
			kjg_val[1] = inb(0x886);
			kjg_val[2] = inb(0x887);
			printf("bd1 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x88A);
			kjg_val[1] = inb(0x88B);
			kjg_val[2] = inb(0x88C);
			printf("bd2 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp2 = 8;
			} else tmp2 = 4;

			if(tmp1 == 0 && tmp2 == 8) {
				printf("tmp1 %d, tmp2 %d\n", tmp1, tmp2);
			} else {
				printf("tmp1 %d, tmp2 %d, test22 NG!!!\n", tmp1, tmp2);
				continue;
			}

			//tx_count = 7; //(spi_type << 4) || cmd, data1~4
			tx_count = 5; //(spi_type << 4) || cmd, data1~4
			//tx_count = 4; //(spi_type << 4) || cmd, data1~4
			k = 0;

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x830 + (can_ch * 2);
					i_addr = 0x838 + (can_ch * 2);
				} else {
					o_addr = 0x840 + ((can_ch - 4) * 2);
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5 || i_val == 0x45) break;
					i++;
					/*printf("test22 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test22 fail1 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test22 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test22 received a : %d, %02x, %d\n", can_ch, i_val, i);

				outb(tx_count, o_addr);
				outb(0x00, o_addr+1); //(spi_type << 4) || cmd
					//spi_tx_rx 0, spi_tx_only 1, spi_rx_only 2

				//outb(0x2E, o_addr+1); outb(0x0C, o_addr+1); //data1, data2

				outb(0x20, o_addr+1); outb(0x68, o_addr+1); //data1, data2
				//outb(0x20, o_addr+1); outb(0x6C, o_addr+1); //data1, data2
				//outb(0x20, o_addr+1); outb(0x5C, o_addr+1); //data1, data2
				//outb(0x20, o_addr+1); outb(0x58, o_addr+1); //data1, data2

				//outb(0x20, o_addr+1); outb(0x00, o_addr+1); //data1, data2
				//outb(0x24, o_addr+1); outb(0x00, o_addr+1); //data1, data2

				//outb(0x03, o_addr+1); //data3
				outb(can_ch, o_addr+1); outb(can_ch+2, o_addr+1); //data3, data4
				//outb(0x03, o_addr+1); outb(0x04, o_addr+1); //data5, data6
				i_val = inb(o_addr+1); //tx_start
				if(tx_count != i_val) printf("test22 fail_a %d, %d\n", can_ch, i_val);

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xE3) break;
					i++;
					/*printf("test22 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test22 fail2 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test22 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test22 received b : %d, %02x, %d\n", can_ch, i_val, i);

				for(i=0; i < tx_count; i++) kjg_val[i] = 0xAA;

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				} else {
					printf("test22 fail3 : %d, %02x\n", can_ch, i_val);
					continue;
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					/*printf("test22 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test22 fail4 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test22 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test22 received d : %d, %02x, %d\n", can_ch, i_val, i);

				if(kjg_val[0] == 0x00
					&& ((kjg_val[1] == 0x00 && kjg_val[2] == 0x00
					&& kjg_val[3] == 0x00 && kjg_val[4] == 0x00)
					|| (kjg_val[1] == 0xFF && kjg_val[2] == 0xFF
					&& kjg_val[3] == 0xFF && kjg_val[4] == 0xFF))) {
					k++;
				}
			}

			printf("k=%d, read\n", k);

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x830 + (can_ch * 2);
					i_addr = 0x838 + (can_ch * 2);
				} else {
					o_addr = 0x840 + ((can_ch - 4) * 2);
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					/*if(i >= 10) {
						printf("test22 fail5 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test22 fail5 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test22 fail5 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test22 received e : %d, %02x, %d\n", can_ch, i_val, i);

				outb(tx_count, o_addr);
				outb(0x00, o_addr+1); //(spi_type << 4) || cmd
					//spi_tx_rx 0, spi_tx_only 1, spi_rx_only 2

				//outb(0x3E, o_addr+1); outb(0x0C, o_addr+1); //data1, data2

				outb(0x30, o_addr+1); outb(0x68, o_addr+1); //data1, data2
				//outb(0x30, o_addr+1); outb(0x6C, o_addr+1); //data1, data2
				//outb(0x30, o_addr+1); outb(0x5C, o_addr+1); //data1, data2
				//outb(0x30, o_addr+1); outb(0x58, o_addr+1); //data1, data2

				//outb(0x30, o_addr+1); outb(0x00, o_addr+1); //data1, data2
				//outb(0x34, o_addr+1); outb(0x00, o_addr+1); //data1, data2

				//outb(0x00, o_addr+1); //data3
				outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data3, data4
				//outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data5, data6
				i_val = inb(o_addr+1); //tx_start
				if(tx_count != i_val) printf("test22 fail_b %d, %d\n", can_ch, i_val);

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xE3) break;
					i++;
					/*if(i >= 10) {
						printf("test22 fail6 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test22 fail6 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test22 fail6 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test22 received f : %d, %02x, %d\n", can_ch, i_val, i);

				for(i=0; i < tx_count; i++) kjg_val[i] = 0xAA;

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				} else {
					printf("test22 fail7 : %d, %02x\n", can_ch, i_val);
					continue;
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					/*printf("test22 fail8 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test22 fail8 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test22 fail8 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test22 received h : %d, %02x, %d\n", can_ch, i_val, i);

				if(kjg_val[0] == 0x00
					&& ((kjg_val[1] == 0x00 && kjg_val[2] == 0x00)
					|| (kjg_val[1] == 0xFF && kjg_val[2] == 0xFF))
					&& kjg_val[3] == can_ch && kjg_val[4] == 0x04) {
					k++;
				}
			}

			printf("test22 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 8) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			} else {
				if(k == 16) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			}
		} else if(strcmp(tmp, "test23") == 0
			|| strcmp(tmp, "t23") == 0) {
			mode = NONE;

			kjg_val[0] = inb(0x885);
			kjg_val[1] = inb(0x886);
			kjg_val[2] = inb(0x887);
			printf("bd1 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x88A);
			kjg_val[1] = inb(0x88B);
			kjg_val[2] = inb(0x88C);
			printf("bd2 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp2 = 8;
			} else tmp2 = 4;

			if(tmp1 == 0 && tmp2 == 8) {
				printf("tmp1 %d, tmp2 %d\n", tmp1, tmp2);
			} else {
				printf("tmp1 %d, tmp2 %d, test23 NG!!!\n", tmp1, tmp2);
				continue;
			}

			//tx_count = 7; //(spi_type << 4) || cmd, data1~4
			tx_count = 5; //(spi_type << 4) || cmd, data1~4
			//tx_count = 4; //(spi_type << 4) || cmd, data1~4
			k = 0;

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x830 + (can_ch * 2);
					i_addr = 0x838 + (can_ch * 2);
				} else {
					o_addr = 0x840 + ((can_ch - 4) * 2);
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5 || i_val == 0x45) break;
					i++;
					/*printf("test23 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test23 fail1 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test23 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test23 received a : %d, %02x, %d\n", can_ch, i_val, i);

				outb(tx_count, o_addr);
				outb(0x00, o_addr+1); //(spi_type << 4) || cmd
					//spi_tx_rx 0, spi_tx_only 1, spi_rx_only 2
				//_CHIP_REG_ECCCON = 0xE0C
				//outb(0x3E, o_addr+1); outb(0x0C, o_addr+1); //data1, 2

				//_CHIP_REG_CiFIFOCON + (2 * 12) => TransmitChannelEventEnable
				outb(0x30, o_addr+1); outb(0x68, o_addr+1); //data1, 2

				//_CHIP_REG_CiFIFOSTA + (2 * 12) => TransmitChannelEventGet
				//outb(0x30, o_addr+1); outb(0x6C, o_addr+1); //data1, 2

				//_CHIP_REG_CiFIFOCON + (1 * 12) => ReceiveChannelEventEnable
				//outb(0x30, o_addr+1); outb(0x5C, o_addr+1); //data1, 2

				//outb(0x30, o_addr+1); outb(0x58, o_addr+1); //data1, 2
				
				//outb(0x30, o_addr+1); outb(0x00, o_addr+1); //data1, 2
				//outb(0x34, o_addr+1); outb(0x00, o_addr+1); //data1, 2
				
				//_CHIP_REG_CiCON + 3 => OperationModeSelect
				//outb(0x30, o_addr+1); outb(0x03, o_addr+1); //data1, 2

				//_CHIP_REG_CiINTENABLE => ModuleEventEnable
				//outb(0x30, o_addr+1); outb(0x1E, o_addr+1); //data1, 2

				//_CHIP_REG_IOCON + 3 => GpioModeConfigure
				//outb(0x3E, o_addr+1); outb(0x07, o_addr+1); //data1, 2

				//outb(0x00, o_addr+1); //data3
				outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data3, 4
				//outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data5, 6
				i_val = inb(o_addr+1); //tx_start
				if(tx_count != i_val) printf("test23 fail_a %d, %d\n", can_ch, i_val);

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xE3) break;
					i++;
					/*printf("test23 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test23 fail2 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test23 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test23 received b : %d, %02x, %d\n", can_ch, i_val, i);

				for(i=0; i < tx_count; i++) kjg_val[i] = 0xAA;

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				} else {
					printf("test23 fail3 : %d, %02x\n", can_ch, i_val);
					continue;
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					/*printf("test23 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test23 fail4 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test23 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test23 received d : %d, %02x, %d\n", can_ch, i_val, i);

				if(kjg_val[0] == 0x00
					&& ((kjg_val[1] == 0x00 && kjg_val[2] == 0x00)
					|| (kjg_val[1] == 0xFF && kjg_val[2] == 0xFF))
					&& kjg_val[3] == can_ch && kjg_val[4] == 0x04) {
					k++;
				}
			}

			printf("test23 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			}
		} else if(strcmp(tmp, "test24") == 0 || strcmp(tmp, "t24") == 0) {
			mode = NONE;

			kjg_val[0] = inb(0x885);
			kjg_val[1] = inb(0x886);
			kjg_val[2] = inb(0x887);
			printf("bd1 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x88A);
			kjg_val[1] = inb(0x88B);
			kjg_val[2] = inb(0x88C);
			printf("bd2 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp2 = 8;
			} else tmp2 = 4;

			if(tmp1 == 0 && tmp2 == 8) {
				printf("tmp1 %d, tmp2 %d\n", tmp1, tmp2);
			} else {
				printf("tmp1 %d, tmp2 %d, test24 NG!!!\n", tmp1, tmp2);
				continue;
			}

			tx_count = 3; //(spi_type << 4) || cmd, data1, data2
			k = 0;

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x830 + (can_ch * 2);
					i_addr = 0x838 + (can_ch * 2);
				} else {
					o_addr = 0x840 + ((can_ch - 4) * 2);
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5 || i_val == 0x45) break;
					i++;
					/*printf("test24 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test24 fail1 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test24 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test24 received a : %d, %02x, %d\n", can_ch, i_val, i);

				outb(tx_count, o_addr);
				outb(0x01, o_addr+1); //(spi_type << 4) || cmd
					//spi_tx_rx 0, spi_tx_only 1, spi_rx_only 2
				outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data1, data2
				i_val = inb(o_addr+1); //tx_start
				if(tx_count != i_val) printf("test24 fail_a %d, %d\n",
					can_ch, i_val);

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xE3) break;
					i++;
					/*printf("test24 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test24 fail2 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test24 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test24 received b : %d, %02x, %d\n", can_ch, i_val, i);

				tx_count = 45;
				for(i=0; i < tx_count; i++) kjg_val[i] = 0xAA;

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr);
						if(i >= 33) printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				} else {
					printf("test24 fail3 : %d, %02x\n", can_ch, i_val);
					continue;
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					/*printf("test24 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test24 fail4 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test24 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test24 received d : %d, %02x, %d\n", can_ch, i_val, i);

				if(kjg_val[0] == 0x01
					&& (kjg_val[1] == 'S' && kjg_val[2] == 'Y'
					&& kjg_val[3] == 'S' && kjg_val[4] == 'T'
					&& kjg_val[5] == 'E' && kjg_val[6] == 'M'
					&& kjg_val[7] == '_' && kjg_val[8] == 'C'
					&& kjg_val[9] == 'A' && kjg_val[10] == 'N'
					&& kjg_val[11] == '_' && kjg_val[12] == 'F'
					&& kjg_val[13] == 'D' && kjg_val[14] == '_'
					&& kjg_val[15] == 'B' && kjg_val[16] == '2')
					&& (kjg_val[33] == '2') //&& kjg_val[34] == '2'
					//&& kjg_val[35] == '1' && kjg_val[36] == '0'
					//&& kjg_val[37] == '2' && kjg_val[38] == '8'
					//&& kjg_val[39] == '0' && kjg_val[40] == '3')
					) {
					k++;
				}
			}

			printf("test24 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			}
		} else if(strcmp(tmp, "test25") == 0 || strcmp(tmp, "t25") == 0
			|| strcmp(tmp, "t25b") == 0) {
			mode = NONE;

			kjg_val[0] = inb(0x885);
			kjg_val[1] = inb(0x886);
			kjg_val[2] = inb(0x887);
			printf("bd1 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x88A);
			kjg_val[1] = inb(0x88B);
			kjg_val[2] = inb(0x88C);
			printf("bd2 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp2 = 8;
			} else tmp2 = 4;

			if(tmp1 == 0 && tmp2 == 8) {
				printf("tmp1 %d, tmp2 %d\n", tmp1, tmp2);
			} else {
				printf("tmp1 %d, tmp2 %d, test25 NG!!!\n", tmp1, tmp2);
				continue;
			}

			tx_count = 5; //(spi_type << 4) || cmd, data1, data2
			k = 0;

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
			//for(can_ch=tmp1; can_ch < 4; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x830 + (can_ch * 2);
					i_addr = 0x838 + (can_ch * 2);
				} else {
					o_addr = 0x840 + ((can_ch - 4) * 2);
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5 || i_val == 0x45) break;
					i++;
					/*printf("test25 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test25 fail1 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test25 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test25 received a : %d, %02x, %d\n", can_ch, i_val, i);

				outb(tx_count, o_addr);
				outb(0x12, o_addr+1); //(spi_type << 4) || cmd
					//spi_tx_rx 0, spi_tx_only 1, spi_rx_only 2
				if(strcmp(tmp, "t25b") == 0) {
					adData.val = 0;
				} else {
					adData.val = 80;
				}
				outb(adData.data[0], o_addr+1); outb(adData.data[1], o_addr+1); //data1, data2
				outb(0x01, o_addr+1); outb(0x00, o_addr+1); //data3, data4
				i_val = inb(o_addr+1); //tx_start
				if(tx_count != i_val) printf("test25 fail_a %d, %d\n",
					can_ch, i_val);
				else k++;

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					/*printf("test25 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test25 fail2 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test25 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test25 received b : %d, %02x, %d\n", can_ch, i_val, i);

				/*for(i=0; i < tx_count; i++) kjg_val[i] = 0xAA;

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				} else {
					printf("test25 fail3 : %d, %02x\n", can_ch, i_val);
					continue;
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					//if(i_val == 0xC5) break;
					if(i_val == 0x45) break;
					i++;
					printf("test25 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test25 fail4 end\n");
						i = 100;
						break;
					}
				}
				if(i == 100) continue;

				if(kjg_val[0] == 0x02) k++;*/
			}

			printf("test25 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			}
		} else if(strcmp(tmp, "test26") == 0 || strcmp(tmp, "t26") == 0) {
			mode = NONE;

			kjg_val[0] = inb(0x885);
			kjg_val[1] = inb(0x886);
			kjg_val[2] = inb(0x887);
			printf("bd1 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x88A);
			kjg_val[1] = inb(0x88B);
			kjg_val[2] = inb(0x88C);
			printf("bd2 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp2 = 8;
			} else tmp2 = 4;

			if(tmp1 == 0 && tmp2 == 8) {
				printf("tmp1 %d, tmp2 %d\n", tmp1, tmp2);
			} else {
				printf("tmp1 %d, tmp2 %d, test26 NG!!!\n", tmp1, tmp2);
				continue;
			}

			tx_count = 3; //(spi_type << 4) || cmd, data1, data2
			k = 0;

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x830 + (can_ch * 2);
					i_addr = 0x838 + (can_ch * 2);
				} else {
					o_addr = 0x840 + ((can_ch - 4) * 2);
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5 || i_val == 0x45) break;
					i++;
					/*printf("test26 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test26 fail1 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test26 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test26 received a : %d, %02x, %d\n", can_ch, i_val, i);

				outb(tx_count, o_addr);
				outb(0x03, o_addr+1); //(spi_type << 4) || cmd
					//spi_tx_rx 0, spi_tx_only 1, spi_rx_only 2
				outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data1, data2
				i_val = inb(o_addr+1); //tx_start
				if(tx_count != i_val) printf("test26 fail_a %d, %d\n",
					can_ch, i_val);

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xE3) break;
					i++;
					/*printf("test26 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test26 fail2 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test26 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test26 received b : %d, %02x, %d\n", can_ch, i_val, i);

				tx_count = 7;
				for(i=0; i < tx_count; i++) kjg_val[i] = 0xAA;

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				} else {
					printf("test26 fail3 : %d, %02x\n", can_ch, i_val);
					continue;
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					/*printf("test26 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test26 fail4 end\n");
						i = 100;
						break;
					}*/
					if(i == 1000) {
						printf("test26 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
						break;
					}
				}
				//if(i == 100) continue;
				if(i == 1000) continue;
				if(i != 0) printf("test26 received c : %d, %02x, %d\n", can_ch, i_val, i);

				if(kjg_val[0] == 0x03) k++;
			}

			printf("test26 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			}
		} else if(strcmp(tmp, "test27") == 0 || strcmp(tmp, "t27") == 0) {
			mode = NONE;

			kjg_val[0] = inb(0x885);
			kjg_val[1] = inb(0x886);
			kjg_val[2] = inb(0x887);
			printf("bd1 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x88A);
			kjg_val[1] = inb(0x88B);
			kjg_val[2] = inb(0x88C);
			printf("bd2 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp2 = 8;
			} else tmp2 = 4;

			if(tmp1 == 0 && tmp2 == 8) {
				printf("tmp1 %d, tmp2 %d\n", tmp1, tmp2);
			} else {
				printf("tmp1 %d, tmp2 %d, test27 NG!!!\n", tmp1, tmp2);
				continue;
			}

			tx_count = 3; //(spi_type << 4) || cmd, data1, data2
			k = 0;

			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x830 + (can_ch * 2);
					i_addr = 0x838 + (can_ch * 2);
				} else {
					o_addr = 0x840 + ((can_ch - 4) * 2);
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5 || i_val == 0x45) break;
					i++;
					printf("test27 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test27 fail1 end\n");
						i = 100;
						break;
					}
				}
				if(i == 100) continue;

				outb(tx_count, o_addr);
				outb(0x0B, o_addr+1); //(spi_type << 4) || cmd
					//spi_tx_rx 0, spi_tx_only 1, spi_rx_only 2
				outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data1, data2
				i_val = inb(o_addr+1); //tx_start
				if(tx_count != i_val) printf("test27 fail_a %d, %d\n",
					can_ch, i_val);

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xE3) break;
					i++;
					printf("test27 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test27 fail2 end\n");
						i = 100;
						break;
					}
				}
				if(i == 100) continue;

				i_val = inb(o_addr);
				tx_count = 5;
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if((i_val == tx_count) || (i_val > 9)) {
					printf("rcv data ");
					for(i=0; i < i_val; i++) {
						kjg_val[i] = inb(o_addr);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				} else {
					printf("test27 fail3 : %d, %02x\n", can_ch, i_val);
					continue;
				}

				i = 0;
				while(1) {
					usleep(100);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					printf("test27 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
					if(i >= 10) {
						printf("test27 fail4 end\n");
						i = 100;
						break;
					}
				}
				if(i == 100) continue;

				if(kjg_val[0] == 0x0B) k++;
			}

			printf("test27 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!\n", tmp1, tmp2, k);
			}
		} else if(strcmp(tmp, "test30") == 0
			|| strcmp(tmp, "t30") == 0) {
			mode = NONE;

			kjg_val[0] = inb(0x885);
			kjg_val[1] = inb(0x886);
			kjg_val[2] = inb(0x887);
			printf("bd1 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x88A);
			kjg_val[1] = inb(0x88B);
			kjg_val[2] = inb(0x88C);
			printf("bd2 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp2 = 8;
			} else tmp2 = 4;

			printf("tmp1 %d, tmp2 %d\n", tmp1, tmp2);

			//tx_count = 7; //(spi_type << 4) || cmd, data1~4
			//tx_count = 5; //(spi_type << 4) || cmd, data1~4
			tx_count = 4; //(spi_type << 4) || cmd, data1~4
			k = 0;

			tmp1 = 0; tmp2 = 1;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x830 + (can_ch * 2);
					i_addr = 0x838 + (can_ch * 2);
				} else {
					o_addr = 0x840 + ((can_ch - 4) * 2);
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i = 0;
				while(1) {
					//usleep(100);
					usleep(10000);
					i_val = inb(i_addr);
					if(i_val == 0xC5 || i_val == 0x45) break;
					i++;
					if(i >= 10) {
						printf("test30 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test30 fail1 end\n");
						i = 100;
						break;
					}
				}
				if(i == 100) continue;

				outb(tx_count, o_addr);
				outb(0x00, o_addr+1); //(spi_type << 4) || cmd
					//spi_tx_rx 0, spi_tx_only 1, spi_rx_only 2

				//_CHIP_REG_CiFIFOSTA + (1 * _CHIP_FIFO_OFFSET)
				//0x54 + (1 * 12) = 0x60
				outb(0x30, o_addr+1); outb(0x60, o_addr+1); //data1, 2

				outb(0x00, o_addr+1); //data3
				i_val = inb(o_addr+1); //tx_start
				if(tx_count != i_val) printf("test30 fail_a %d, %d\n", can_ch, i_val);

				i = 0;
				while(1) {
					//usleep(100);
					usleep(10000);
					i_val = inb(i_addr);
					if(i_val == 0xE3) break;
					i++;
					if(i >= 10) {
						printf("test30 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test30 fail2 end\n");
						i = 100;
						break;
					}
				}
				if(i == 100) continue;

				for(i=0; i < tx_count; i++) kjg_val[i] = 0xAA;

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				} else {
					printf("test30 fail3 : %d, %02x\n", can_ch, i_val);
					continue;
				}

				i = 0;
				while(1) {
					//usleep(100);
					usleep(10000);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					if(i >= 10) {
						printf("test30 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test30 fail4 end\n");
						i = 100;
						break;
					}
				}
				if(i == 100) continue;

				if(kjg_val[1] == 0x00 && kjg_val[2] == 0x00
					&& kjg_val[3] == can_ch && kjg_val[4] == (can_ch+1)) {
					k++;
				}
			}

			printf("test30 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d %d : k=%d\n", tmp1, tmp2, k);
				else printf("NG %d %d : k=%d\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("OK %d %d : k=%d\n", tmp1, tmp2, k);
				else printf("NG %d %d : k=%d\n", tmp1, tmp2, k);
			}
		} else if(strcmp(tmp, "test31") == 0
			|| strcmp(tmp, "t31") == 0) {
			mode = NONE;

			kjg_val[0] = inb(0x885);
			kjg_val[1] = inb(0x886);
			kjg_val[2] = inb(0x887);
			printf("bd1 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x88A);
			kjg_val[1] = inb(0x88B);
			kjg_val[2] = inb(0x88C);
			printf("bd2 ver %x %x %x, ", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x42 || kjg_val[0] == 0x62) { //0x42=B, 0x62=b
				tmp2 = 8;
			} else tmp2 = 4;

			printf("tmp1 %d, tmp2 %d\n", tmp1, tmp2);

			//tx_count = 7; //(spi_type << 4) || cmd, data1~4
			//tx_count = 5; //(spi_type << 4) || cmd, data1~4
			//tx_count = 4; //(spi_type << 4) || cmd, data1~4
			tx_count = 15; //(spi_type << 4) || cmd, data1~4
			k = 0;

			tmp1 = 0; tmp2 = 1;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(can_ch < 4) {
					o_addr = 0x830 + (can_ch * 2);
					i_addr = 0x838 + (can_ch * 2);
				} else {
					o_addr = 0x840 + ((can_ch - 4) * 2);
					i_addr = 0x848 + ((can_ch - 4) * 2);
				}

				i = 0;
				while(1) {
					//usleep(100);
					usleep(10000);
					i_val = inb(i_addr);
					if(i_val == 0xC5 || i_val == 0x45) break;
					i++;
					if(i >= 10) {
						printf("test31 fail1 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test31 fail1 end\n");
						i = 100;
						break;
					}
				}
				if(i == 100) continue;

				outb(tx_count, o_addr);
				outb(0x00, o_addr+1); //(spi_type << 4) || cmd
					//spi_tx_rx 0, spi_tx_only 1, spi_rx_only 2

				//_CHIP_REG_CiFIFOCON + (1 * _CHIP_FIFO_OFFSET)
				//0x50 + (1 * 12) = 0x5C
				outb(0x30, o_addr+1); outb(0x5C, o_addr+1); //data1, 2

				outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data3, data4
				outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data3, data4

				outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data3, data4
				outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data3, data4

				outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data3, data4
				outb(0x00, o_addr+1); outb(0x00, o_addr+1); //data3, data4
				i_val = inb(o_addr+1); //tx_start
				if(tx_count != i_val) printf("test31 fail_a %d, %d\n", can_ch, i_val);

				i = 0;
				while(1) {
					//usleep(100);
					usleep(10000);
					i_val = inb(i_addr);
					if(i_val == 0xE3) break;
					i++;
					if(i >= 10) {
						printf("test31 fail2 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test31 fail2 end\n");
						i = 100;
						break;
					}
				}
				if(i == 100) continue;

				for(i=0; i < tx_count; i++) kjg_val[i] = 0xAA;

				i_val = inb(o_addr);
				printf("can_ch : %d, get tx_count %d, ", can_ch, i_val);
				if(i_val == tx_count) {
					printf("rcv data ");
					for(i=0; i < tx_count; i++) {
						kjg_val[i] = inb(o_addr);
						printf("%02x ", kjg_val[i]);
					}
					printf("\n");
				} else {
					printf("test31 fail3 : %d, %02x\n", can_ch, i_val);
					continue;
				}

				i = 0;
				while(1) {
					//usleep(100);
					usleep(10000);
					i_val = inb(i_addr);
					if(i_val == 0xC5) break;
					i++;
					if(i >= 10) {
						printf("test31 fail4 : %d, %02x, %d\n", can_ch, i_val, i);
						printf("test31 fail4 end\n");
						i = 100;
						break;
					}
				}
				if(i == 100) continue;

				if(kjg_val[1] == 0x00 && kjg_val[2] == 0x00
					&& kjg_val[3] == can_ch && kjg_val[4] == (can_ch+1)) {
					k++;
				}
			}

			printf("test31 ");
			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d %d : k=%d\n", tmp1, tmp2, k);
				else printf("NG %d %d : k=%d\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("OK %d %d : k=%d\n", tmp1, tmp2, k);
				else printf("NG %d %d : k=%d\n", tmp1, tmp2, k);
			}
		} else if(strcmp(tmp, "tt1") == 0) {
			mode = NONE;
			for(i=0; i < 60; i++) {
				i_val = inb(0x883);
				printf("t1 %d %x\n", i, i_val);
				sleep(1);
			}
		} else if(strcmp(tmp, "r0") == 0) {
			mode = NONE;
			outb(0x00, 0x883);
			outb(0x00, 0x888);
			printf("relay off (terminal resistor disconnected)\n");
		} else if(strcmp(tmp, "r1") == 0) {
			mode = NONE;
			outb(0x0F, 0x883);
			outb(0x0F, 0x888);
			printf("relay on (terminal resistor connected)\n");
		} else if(strcmp(tmp, "create_file_rx") == 0) {
			fp = fopen("CanReceiveSetData01_org", "w");
			if(fp == NULL) {
			   printf("CanReceiveSetData01_org write fail\n");
			} else {
				for(i=0; i < 2; i++) {
					if(i == 0) {
						fprintf(fp, "master\n");
					} else {
						fprintf(fp, "\nslave\n");
					}
					fprintf(fp, "can_baudrate : 2\n");
					fprintf(fp, "extended_id : 0\n");
					fprintf(fp, "controller_canID : 0\n");
					fprintf(fp, "mask1 : 0\n");
					fprintf(fp, "mask2 : 0\n");
					fprintf(fp, "filter1 : 0\n");
					fprintf(fp, "filter2 : 0\n");
					fprintf(fp, "filter3 : 0\n");
					fprintf(fp, "filter4 : 0\n");
					fprintf(fp, "filter5 : 0\n");
					fprintf(fp, "filter6 : 0\n");
					fprintf(fp, "bms_type : 0\n");
					fprintf(fp, "sjw : 0\n");
					fprintf(fp, "can_fd_flag : 1\n");
					fprintf(fp, "can_datarate : 4\n");
					fprintf(fp, "terminal_r : 1\n");
					fprintf(fp, "crc_type : 1\n");
					fprintf(fp, "func_div : 0 0 0 0 0 0 0 0 0 0\n");
					fprintf(fp, "compare_type : 0 0 0 0 0 0 0 0 0 0\n");
					fprintf(fp, "data_type : 0 0 0 0 0 0 0 0 0 0\n");
					fprintf(fp, "value : 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0\n");
				}

				for(i=0; i < 512; i++) {
					fprintf(fp, "\nindex : %03d\n", i);
					fprintf(fp, "canType : 0\n");
					fprintf(fp, "byte_order : 0\n");
					fprintf(fp, "data_type : 0\n");
					fprintf(fp, "factor : 0.0\n");
					fprintf(fp, "offset : 0.0\n");
					fprintf(fp, "def_value : 0.0\n");
					fprintf(fp, "startBit : 0\n");
					fprintf(fp, "bitCount : 0\n");
					fprintf(fp, "canID : 0\n");
					fprintf(fp, "name : $\n");
					fprintf(fp, "fault_upper : 0.0\n");
					fprintf(fp, "fault_lower : 0.0\n");
					fprintf(fp, "end_upper : 0.0\n");
					fprintf(fp, "end_lower : 0.0\n");
					fprintf(fp, "sent_period : 0\n");
					fprintf(fp, "func_div : 0\n");
					fprintf(fp, "func_div2 : 0\n");
					fprintf(fp, "func_div3 : 0\n");
					fprintf(fp, "reserved1 : 0\n");
					fprintf(fp, "reserved2 : 0\n");
					fprintf(fp, "reserved3 : 0\n");
					fprintf(fp, "startBit2 : 0\n");
					fprintf(fp, "bitCount2 : 0\n");
					fprintf(fp, "byte_order2 : 0\n");
					fprintf(fp, "data_type2 : 0\n");
					fprintf(fp, "reserved4 : 0\n");
					fprintf(fp, "reserved5 : 0\n");
					fprintf(fp, "compare_value : 0.0\n");
				}
			}
			fclose(fp);
		} else if(strcmp(tmp, "create_file_tx") == 0) {
			fp = fopen("CanTransmitSetData01_org", "w");
			if(fp == NULL) {
			   printf("CanTransmitSetData01_org write fail\n");
			} else {
				for(i=0; i < 2; i++) {
					if(i == 0) {
						fprintf(fp, "master\n");
					} else {
						fprintf(fp, "\nslave\n");
					}
					fprintf(fp, "can_baudrate : 2\n");
					fprintf(fp, "extended_id : 0\n");
					fprintf(fp, "controller_canID : 0\n");
					fprintf(fp, "bms_type : 0\n");
					fprintf(fp, "sjw : 0\n");
					fprintf(fp, "can_fd_flag : 1\n");
					fprintf(fp, "can_datarate : 4\n");
					fprintf(fp, "terminal_r : 1\n");
					fprintf(fp, "crc_type : 1\n");
					fprintf(fp, "func_div : 0 0 0 0 0 0 0 0 0 0\n");
					fprintf(fp, "compare_type : 0 0 0 0 0 0 0 0 0 0\n");
					fprintf(fp, "data_type : 0 0 0 0 0 0 0 0 0 0\n");
					fprintf(fp, "value : 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0\n");
				}

				for(i=0; i < 512; i++) {
					fprintf(fp, "\nindex : %03d\n", i);
					fprintf(fp, "canType : 0\n");
					fprintf(fp, "byte_order : 0\n");
					fprintf(fp, "data_type : 0\n");
					fprintf(fp, "factor : 0.0\n");
					fprintf(fp, "offset : 0.0\n");
					fprintf(fp, "def_value : 0.0\n");
					fprintf(fp, "startBit : 0\n");
					fprintf(fp, "bitCount : 0\n");
					fprintf(fp, "canID : 0\n");
					fprintf(fp, "send_period : 0\n");
					fprintf(fp, "name : $\n");
					fprintf(fp, "func_div : 0\n");
					fprintf(fp, "func_div2 : 0\n");
					fprintf(fp, "func_div3 : 0\n");
					fprintf(fp, "dlc : 8\n");
					fprintf(fp, "reserved2 : 0\n");
					fprintf(fp, "reserved3 : 0\n");
				}
			}
			fclose(fp);
		} else if(strcmp(tmp, "wr") == 0) {
			scanf("%x %x", &addr, &val);
			printf("wr %x %x\n", addr, val);
			mode = NONE;
			o_addr = addr;
			o_val = (unsigned char)val;
			outb(o_val, o_addr);
		} else if(strcmp(tmp, "write") == 0) {
			scanf("%x %x", &addr, &val);
			printf("write %x %x\n", addr, val);
			mode = OUTPUT;
			o_addr = addr;
			o_val = (unsigned char)val;
		} else if(strcmp(tmp, "read16") == 0) {
			scanf("%x", &addr);
			printf("read 0x%04X\n", addr);
			//mode = INPUT;
			i_addr = addr;
			for(i=0; i < 16; i++) {
				printf(" ==> 0x%02X\n", inb(i_addr));
				sleep(1);
			}
		} else if(strcmp(tmp, "read") == 0) {
			scanf("%x", &addr);
			printf("read 0x%04X\n", addr);
			mode = INPUT;
			i_addr = addr;
		} else if(strcmp(tmp, "get") == 0) {
			scanf("%x", &addr);
			printf("get %x\n", addr);
			mode = NONE;
			i_addr = addr;
			i_val = inb(i_addr);
			printf("get data %x %x\n", i_addr, i_val);
		} else if(strcmp(tmp, "adv") == 0) {
			mode = AD;
			type = 0;
			phase1 = 0;
		} else if(strcmp(tmp, "adi") == 0) {
			mode = AD;
			type = 1;
			phase1 = 0;
		} else if(strcmp(tmp, "adj") == 0) {
			scanf("%x %x", &addr, &val);
			printf("adjust AD %x %x\n", addr, val);
			o_addr = addr;
			o_val = (unsigned char)val;
			mode = ADJ;
		} else if(strcmp(tmp, "daloop") == 0) {
			scanf("%d", &addr);
			printf("daloop %d\n", addr);
			o_addr = addr - 1;
			o_addr &= 0xFF;
			count = 0x00;
			mode = DACHECK;
		} else if(strcmp(tmp, "pv") == 0) { //LG_Cycler
			muxVal1 = 0x10; //5V
			muxVal2 = 0x10;

			outb(muxVal2, 0x5A8); //MUX_CHAR
			usleep(50);
			outb(muxVal1, 0x5A9); //MUX_CHEN
			usleep(50);

			outb(0x00, 0x5AB); //AD_START
			usleep(10000);
			for(j=0; j < MAX_AD_COUNT; j++) {
				adData.data[1] = inb(0x5AB); //AD_HADDR 
				adData.data[0] = inb(0x5AA); //AD_LADDR 
//				REFIC[j] = adData.val;
			}
			printf("read %2.3fV, adVal : %x\n",
				adData.val * 9.0 / 29491, adData.val);

			system("rm -rf refic.csv");

			fp = fopen("refic.csv","a");
			if(fp == NULL) {
			   printf("refic.csv read fail\n");
			} else {
				for(j=0; j < MAX_AD_COUNT; j++) {
//					fprintf(fp,"%d,	%2.3f\n", j + 1, REFIC[j] * 9.0 / 29491);
				} //Num, 5V
			}
			fclose(fp);
		} else if(strcmp(tmp, "quit") == 0 || strcmp(tmp, "q") == 0) {
			exit(0);
		}

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
						//AD_START
						if(type == 0) outb(0x00, 0x650);
						else outb(0x01, 0x650);
						phase1 = 1;
						break;
					case 1:
						for(i=0; i < 3; i++) {
							j = 0x650 + 2 * i;
							adVal = inb(j);	//AD_HADDR
							adVal2 = inb(j+1); //AD_LADDR
							tmpVal = adVal;
							tmpVal = tmpVal << 8;
							tmpVal |= adVal2;
							if(type == 0) {
								printf("adv %d %f\n", tmpVal,
									(float)tmpVal / 32768.0 * 10000.0);
							} else {
								printf("adi %d %f\n", tmpVal,
									(float)tmpVal / 32768.0 * 190618.0);
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
				if(count >= 0xFF) count = 0x00;
				break;
			default: break;
		}
	}

    exit(0);
}

