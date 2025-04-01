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
    unsigned char adVal, adVal2;
	unsigned char kjg_val[4], tx_count;
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
		} else if(strcmp(tmp, "test1") == 0) {
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
				outb(0x2E, 0x802);
				outb(0x0C, 0x802);
				outb(can_ch, 0x802);
				outb(can_ch+1, 0x802);
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
				outb(0x3E, 0x802);
				outb(0x0C, 0x802);
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
		} else if(strcmp(tmp, "t1") == 0) {
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
		} else if(strcmp(tmp, "quit") == 0) {
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

