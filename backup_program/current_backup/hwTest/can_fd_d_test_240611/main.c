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

const unsigned short crc16Table[256] = { //Look-up table for CRC calculation
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
    0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
    0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
    0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
    0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
    0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
    0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
    0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
    0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
    0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
    0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
    0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
    0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
    0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
    0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
    0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
    0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

union short_int_tmpData {
	unsigned char	data[2];
	short int		val;
};

union unsigned_short_tmpData {
	unsigned char	data[2];
	unsigned short	val;
};

unsigned char kjg_val[128], kjg_val2[40];
int kjg_val3;

int exec_cmd1(int can_ch)
{
    unsigned char tx_count, rx_count, tx_cmd;
	int rtn = 0, addr, i, j;

	tx_count = 3;
	rx_count = 1 + 45;
	tx_cmd = 0x01; //get parameter
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	/*kjg_val2[0] = inb(addr); //spi_tx_index
	kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
	kjg_val2[2] = inb(addr+3); //fifo_status
	kjg_val2[3] = inb(addr+4); //rx_length
	printf("ch %d status %d %d %d %d\n", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);*/

	outb(tx_count, addr+1);
	outb(rx_count, addr+2);
	outb(tx_cmd, addr+3);

	outb(0x00, addr+5); //spi start

	/*kjg_val2[0] = inb(addr); //spi_tx_index
	kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
	kjg_val2[2] = inb(addr+3); //fifo_status
	kjg_val2[3] = inb(addr+4); //rx_length
	printf("ch %d status %d %d %d %d\n", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);*/

	i = 0;
	while(1) {
		kjg_val2[0] = inb(addr); //spi_tx_index
		//kjg_val2[2] = inb(addr+3); //fifo_status
		//kjg_val2[2] = (inb(addr+3) & 0x03); //fifo_status
		kjg_val2[2] = (inb(addr+3) & 0x0F); //fifo_status
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[2] == 0x02) break;
		usleep(100);
		i++;
		if(i > 10) {
			//outb(0x00, addr); //clear
			printf("ch%d fail1 %d %d, %02x\n", can_ch, kjg_val2[0], tx_count + rx_count, kjg_val2[2]);
			return 0;
			//break;
		}
	}
	i = 0;
	while(1) {
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[1] == kjg_val2[0] && kjg_val2[3] == rx_count) break;
		usleep(100);
		i++;
		if(i > 10) {
			//outb(0x00, addr); //clear
			printf("ch%d fail2 %d %d, %d %d\n", can_ch, kjg_val2[0], kjg_val2[1], kjg_val2[3], rx_count);
			return 0;
			//break;
		}
	}

	printf("ch%d status %d %d %d %d, ", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);

	j = 0;
	printf("rxd");
	for(i=0; i < 4; i++) {
		kjg_val[i] = inb(addr+1);
		j += kjg_val[i];
		printf(" %02x", kjg_val[i]);
	}
	printf(",");
	for(i=4; i < tx_count + rx_count; i++) {
		kjg_val[i] = inb(addr+1);
		j += kjg_val[i];
		if(i <= 43) printf("%c", kjg_val[i]);
		else printf("%02x ", kjg_val[i]);
		if(i == 35 || i == 43) printf(",");
	}
	printf("\n");
	kjg_val2[4] = inb(addr+5); //checksum_h
	kjg_val2[5] = inb(addr+6); //checksum_l
	kjg_val2[6] = inb(addr+7); //fifo_rx_rd_index
	if(j != ((int)kjg_val2[4] * 0x100 + kjg_val2[5]) || kjg_val2[0] != kjg_val2[6]) {
		printf("ch%d fail3 %04x %02x %02x, %d %d\n", can_ch, j, kjg_val2[4], kjg_val2[5], kjg_val2[0], kjg_val2[6]);
	}
	/*kjg_val2[4] = inb(addr+5); //checksum_h
	kjg_val2[5] = inb(addr+6); //checksum_l
	kjg_val2[6] = inb(addr+7); //fifo_rx_rd_index
	printf("ch%d checksum %04x %02x %02x, %d %d\n", can_ch, j, kjg_val2[4], kjg_val2[5], kjg_val2[0], kjg_val2[6]);*/
	outb(0x00, addr); //clear

	/*kjg_val2[0] = inb(addr); //spi_tx_index
	kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
	kjg_val2[2] = inb(addr+3); //fifo_status
	kjg_val2[3] = inb(addr+4); //rx_length
	printf("ch%d status %d %d %d %d, ", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);

	printf("rxd");
	for(i=0; i < 4; i++) {
		//kjg_val[i] = inb(addr+1);
		printf(" %02x", kjg_val[i]);
	}
	printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		if(i <= 43) printf("%c", kjg_val[i]);
		else printf("%02x ", kjg_val[i]);
		if(i == 35 || i == 43) printf(", ");
	}
	printf("\n");*/

	i = 0;
	if(kjg_val2[0] == 49 && kjg_val2[1] == kjg_val2[0]
		&& kjg_val2[2] == 0x02 && kjg_val2[3] == 46) {
		i++;
	} else {
		printf("ch%d fail4 %d %d %d %d\n", can_ch, kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);
	}
	if(kjg_val[1] == 3 && kjg_val[2] == 46 && kjg_val[3] == 1
		&& kjg_val[4] == 0x53 && kjg_val[5] == 0x59 //S, Y
		&& kjg_val[6] == 0x53 && kjg_val[7] == 0x54 //S, T
		&& kjg_val[8] == 0x45 && kjg_val[9] == 0x4D //E, M
		&& kjg_val[10] == 0x5F && kjg_val[11] == 0x43 //_, C
		&& kjg_val[12] == 0x41 && kjg_val[13] == 0x4E //A, N
		&& kjg_val[14] == 0x5F && kjg_val[15] == 0x46 //_, F
		&& kjg_val[16] == 0x44 && kjg_val[17] == 0x5F //D, _
		&& kjg_val[18] == 0x44 //D
		&& (kjg_val[19] >= 0x31 && kjg_val[19] <= 0x39) //1~9
		&& kjg_val[20] == 0x00 //sp
		&& (kjg_val[36] >= 0x32 && kjg_val[36] <= 0x34) //2~4
		&& (kjg_val[37] >= 0x30 && kjg_val[37] <= 0x39) //0~9
		&& (kjg_val[38] >= 0x30 && kjg_val[38] <= 0x31) //0~1
		&& (kjg_val[39] >= 0x30 && kjg_val[39] <= 0x39) //0~9
		&& (kjg_val[40] >= 0x30 && kjg_val[40] <= 0x33) //0~3
		&& (kjg_val[41] >= 0x30 && kjg_val[41] <= 0x39) //0~9
		&& (kjg_val[42] >= 0x30 && kjg_val[42] <= 0x39) //0~9
		&& (kjg_val[43] >= 0x30 && kjg_val[43] <= 0x39) //0~9
		&& (kjg_val[46] == 0x00 || kjg_val[46] == 0x01) //0~1
		&& (kjg_val[47] == 0x00 || kjg_val[47] == 0x01) //0~1
		) {
		i++;
	} else {
		printf("ch%d fail5\n", can_ch);
	}
	if(i == 2) rtn = 1;

	return rtn;
}

int exec_cmd_reset(int can_ch)
{
    unsigned char tx_count, rx_count, tx_cmd;
	int rtn = 0, addr, i, c1, c2;

	tx_count = 3 + 2;
	rx_count = 1;
	tx_cmd = 0x00; //chip command
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear
	
	outb(tx_count, addr+1);
	outb(rx_count, addr+2);
	outb(tx_cmd, addr+3);

	outb(0x00, addr+4); //reset
	outb(0x00, addr+4);

	outb(0x00, addr+5); //spi_start

	i = 0;
	while(1) {
		usleep(100);
		kjg_val2[0] = inb(addr); //spi_tx_index
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[2] = (inb(addr+3) & 0x0F); //fifo_status
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[2] == 0x02) {
			kjg_val3++;
			break;
		}
		i++;
		if(i > 10) {
			printf("ch%d reset_a status %d %d, %d %d %02x %d, %d %d\n", can_ch,
				tx_count, rx_count, kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3], i, kjg_val3);
			outb(0x00, addr); //clear
			return 0;
		}
	}
	c1 = i;
	i = 0;
	while(1) {
		usleep(100);
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[1] == kjg_val2[0] && kjg_val2[3] == rx_count) break;
		i++;
		if(i > 10) {
			printf("ch%d reset_b status %d %d, %d %d %d %d\n", can_ch,
				tx_count, rx_count,
				kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);
			outb(0x00, addr); //clear
			return 0;
		}
	}
	c2 = i;
	//printf("%02x ", inb(addr+3));
	printf("ch%d %d %d status %d %d %d %d, ", can_ch, c1, c2,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);

	printf("rxd");
	for(i=0; i < 4; i++) {
		kjg_val[i] = inb(addr+1);
		printf(" %02x", kjg_val[i]);
	}
	printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		kjg_val[i] = inb(addr+1);
		printf("%02x ", kjg_val[i]);
	}
	//printf("ex%02x ", inb(addr+3));
	outb(0x00, addr); //clear
	printf("\n");

	i = 0;
	if(kjg_val2[0] == 6 && kjg_val2[1] == kjg_val2[0]
		&& kjg_val2[2] == 0x02 && kjg_val2[3] == 1) i++;
	if(kjg_val[1] == 0x05 && kjg_val[2] == 0x01 && kjg_val[3] == 0x00
		&& kjg_val[4] == 0x00 && kjg_val[5] == 0x00) i++;
	if(i == 2) rtn = 1;
	else {
		printf("ch%d reset_c status %d %d, %d %d %02x %d\n",
			can_ch, tx_count, rx_count,
			kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);
		printf("%02x %02x %02x, %02x %02x\n",
			kjg_val[1], kjg_val[2], kjg_val[3], kjg_val[4], kjg_val[5]);
	}

	return rtn;
}

int exec_cmd2(int can_ch)
{
    unsigned char tx_count, rx_count, tx_cmd;
	int rtn = 0, addr, i;
	union short_int_tmpData txData;

	tx_count = 3 + 4;
	rx_count = 1;
	tx_cmd = 0x02; //set parameter
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	outb(tx_count, addr+1);
	outb(rx_count, addr+2);
	outb(tx_cmd, addr+3);

	txData.val = 80; //80ms
	outb(txData.data[0], addr+4); //set_can_filter_time
	outb(txData.data[1], addr+4);
	outb(0x00, addr+4); //set_can_fd_flag
	outb(0x00, addr+4); //set_can_enable (0:disable, 1:enable)

	outb(0x00, addr+5); //spi start

	i = 0;
	while(1) {
		kjg_val2[0] = inb(addr); //spi_tx_index
		//kjg_val2[2] = inb(addr+3); //fifo_status
		//kjg_val2[2] = (inb(addr+3) & 0x03); //fifo_status
		kjg_val2[2] = (inb(addr+3) & 0x0F); //fifo_status
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[2] == 0x02) break;
		usleep(100);
		i++;
		if(i > 10) {
			outb(0x00, addr); //clear
			return 0;
		}
	}
	i = 0;
	while(1) {
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[1] == kjg_val2[0] && kjg_val2[3] == rx_count) break;
		usleep(100);
		i++;
		if(i > 10) {
			outb(0x00, addr); //clear
			return 0;
		}
	}
	printf("ch%d status %d %d %d %d, ", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);

	printf("rxd");
	for(i=0; i < 4; i++) {
		kjg_val[i] = inb(addr+1);
		printf(" %02x", kjg_val[i]);
	}
	printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		kjg_val[i] = inb(addr+1);
		printf("%02x ", kjg_val[i]);
	}
	printf("\n");
	outb(0x00, addr); //clear

	i = 0;
	if(kjg_val2[0] == 8 && kjg_val2[1] == kjg_val2[0]
		&& kjg_val2[2] == 0x02 && kjg_val2[3] == 1) i++;
	if(kjg_val[1] == 0x07 && kjg_val[2] == 0x01
		&& kjg_val[3] == 0x02 && kjg_val[4] == 0x50
		&& kjg_val[5] == 0x00 && kjg_val[6] == 0x00
		&& kjg_val[7] == 0x00) i++;
	if(i == 2) rtn = 1;

	return rtn;
}

int exec_cmd3(int can_ch)
{
    unsigned char tx_count, rx_count, tx_cmd;
	int rtn = 0, addr, i;

	tx_count = 3;
	//rx_count = 1 + 10;
	rx_count = 1 + 15; //kjg_240617
	tx_cmd = 0x03; //get status
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	outb(tx_count, addr+1);
	outb(rx_count, addr+2);
	outb(tx_cmd, addr+3);

	outb(0x00, addr+5); //spi_start

	i = 0;
	while(1) {
		kjg_val2[0] = inb(addr); //spi_tx_index
		//kjg_val2[2] = inb(addr+3); //fifo_status
		//kjg_val2[2] = (inb(addr+3) & 0x03); //fifo_status
		kjg_val2[2] = (inb(addr+3) & 0x0F); //fifo_status
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[2] == 0x02) break;
		usleep(100);
		i++;
		if(i > 10) {
			printf("ch%d fail1 %d %d %d %d\n", can_ch, kjg_val2[0], tx_count, rx_count, kjg_val2[2]);
			outb(0x00, addr); //clear
			return 0;
		}
	}
	i = 0;
	while(1) {
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[1] == kjg_val2[0] && kjg_val2[3] == rx_count) break;
		usleep(100);
		i++;
		if(i > 10) {
			printf("ch%d fail2 %d %d %d %d %d\n", can_ch, kjg_val2[0], tx_count, rx_count, kjg_val2[1], kjg_val2[3]);
			outb(0x00, addr); //clear
			return 0;
		}
	}

	printf("ch%d status %d %d %d %d, ", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);

	printf("rxd");
	for(i=0; i < 4; i++) {
		kjg_val[i] = inb(addr+1);
		printf(" %02x", kjg_val[i]);
	}
	printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		kjg_val[i] = inb(addr+1);
		printf("%02x ", kjg_val[i]);
	}
	printf("\n");
	outb(0x00, addr); //clear

	i = 0;
	//kjg_240617 if(kjg_val2[0] == 14 && kjg_val2[1] == kjg_val2[0] && kjg_val2[2] == 0x02 && kjg_val2[3] == 11) {
	if(kjg_val2[0] == 19 && kjg_val2[1] == kjg_val2[0] && kjg_val2[2] == 0x02 && kjg_val2[3] == 16) {
		i++;
	} else {
		printf("ch%d fail3 %d %d %d %d\n", can_ch, kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);
	}
	if(kjg_val[1] == 0x03 && kjg_val[2] == 0x10 && kjg_val[3] == 0x03) {
		i++;
	} else {
		printf("ch%d fail4 %d %d %d %d\n", can_ch, kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);
	}
	if(i == 2) rtn = 1;

	return rtn;
}

int exec_t1_wr(int can_ch)
{
    unsigned char tx_count, rx_count, tx_cmd;
	int rtn = 0, addr, i;

	//kjg_240503
	unsigned char uc_tmp, checksum_enable, checksum_h, checksum_l;
	unsigned int checksum_val, ui_tmp;
	checksum_enable = 1; //0:checksum unuse, 1:checksum use
	checksum_val = 0;

	tx_count = 3 + 6;
	rx_count = 1;
	tx_cmd = 0x00; //chip command
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	uc_tmp = tx_count;
	outb(uc_tmp, addr+1);
	checksum_val += uc_tmp;
	uc_tmp = rx_count;
	outb(uc_tmp, addr+2);
	checksum_val += uc_tmp;
	uc_tmp = tx_cmd;
	outb(uc_tmp, addr+3);
	checksum_val += uc_tmp;

	uc_tmp = 0x20; //write byte
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = 0x10;
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = can_ch;
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = can_ch+1;
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = can_ch+2;
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = can_ch+3;
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;

	if(checksum_enable == 1) {
		checksum_h = ((checksum_val & 0x0000FF00) >> 8);
		checksum_l = (checksum_val & 0x000000FF);
		outb(checksum_h, addr+6);
		outb(checksum_l, addr+7);
	}

	outb(0x00, addr+5); //spi_start

	i = 0;
	while(1) {
		kjg_val2[0] = inb(addr); //spi_tx_index
		//kjg_val2[2] = inb(addr+3); //fifo_status
		//kjg_val2[2] = (inb(addr+3) & 0x03); //fifo_status
		kjg_val2[2] = (inb(addr+3) & 0x0F); //fifo_status
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[2] == 0x02) break;
		usleep(100);
		i++;
		if(i > 10) {
			//outb(0x00, addr); //clear
			break;//return 0;
		}
	}
	i = 0;
	while(1) {
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[1] == kjg_val2[0] && kjg_val2[3] == rx_count) break;
		usleep(100);
		i++;
		if(i > 10) {
			//outb(0x00, addr); //clear
			break;//return 0;
		}
	}
	printf("ch%d status %d %d %d %d, ", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);

	checksum_val = 0;
	printf("rxd");
	for(i=0; i < 4; i++) {
		kjg_val[i] = inb(addr+1);
		checksum_val += kjg_val[i];
		printf(" %02x", kjg_val[i]);
	}
	printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		kjg_val[i] = inb(addr+1);
		checksum_val += kjg_val[i];
		printf("%02x ", kjg_val[i]);
	}
	printf("\n");
	if(checksum_enable == 1) {
		checksum_h = inb(addr+5);
		checksum_l = inb(addr+6);
		ui_tmp = ((unsigned int)checksum_h << 8) + checksum_l;
		kjg_val2[6] = inb(addr+7);
		if(checksum_val != ui_tmp) {
			printf("checksum fail %d %04x, %02x %02x %d, %d %d\n", can_ch,
				checksum_val, checksum_h, checksum_l, checksum_val-ui_tmp, kjg_val2[0], kjg_val2[6]);
		}
	}
	outb(0x00, addr); //clear

	i = 0;
	if(kjg_val2[0] == 10 && kjg_val2[1] == kjg_val2[0]
		&& kjg_val2[2] == 0x02 && kjg_val2[3] == 1) i++;
	if(kjg_val[1] == 0x09 && kjg_val[2] == 0x01 && kjg_val[3] == 0x00
		&& kjg_val[4] == 0x20 && kjg_val[5] == 0x10
		&& kjg_val[6] == (can_ch) && kjg_val[7] == (can_ch+1)
		&& kjg_val[8] == (can_ch+2) && kjg_val[9] == (can_ch+3)) i++;
	if(i == 2) rtn = 1;

	if(i != 2) printf("fail ch%d %d\n", can_ch, i);

	return rtn;
}

int exec_t1_rd(int can_ch)
{
    unsigned char tx_count, rx_count, tx_cmd;
	int rtn = 0, addr, i;

	//kjg_240503
	unsigned char uc_tmp, checksum_enable, checksum_h, checksum_l;
	unsigned int checksum_val, ui_tmp;
	checksum_enable = 1; //0:checksum unuse, 1:checksum use
	checksum_val = 0;

	tx_count = 3 + 2;
	rx_count = 1 + 4;
	tx_cmd = 0x00; //chip command
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	uc_tmp = tx_count;
	outb(uc_tmp, addr+1);
	checksum_val += uc_tmp;
	uc_tmp = rx_count;
	outb(uc_tmp, addr+2);
	checksum_val += uc_tmp;
	uc_tmp = tx_cmd;
	outb(uc_tmp, addr+3);
	checksum_val += uc_tmp;

	uc_tmp = 0x30; //read byte
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = 0x10;
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;

	if(checksum_enable == 1) {
		checksum_h = ((checksum_val & 0x0000FF00) >> 8);
		checksum_l = (checksum_val & 0x000000FF);
		outb(checksum_h, addr+6);
		outb(checksum_l, addr+7);
	}

	outb(0x00, addr+5); //spi_start

	i = 0;
	while(1) {
		kjg_val2[0] = inb(addr); //spi_tx_index
		//kjg_val2[2] = inb(addr+3); //fifo_status
		//kjg_val2[2] = (inb(addr+3) & 0x03); //fifo_status
		kjg_val2[2] = (inb(addr+3) & 0x0F); //fifo_status
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[2] == 0x02) break;
		usleep(100);
		i++;
		if(i > 10) {
			//outb(0x00, addr); //clear
			break;//return 0;
		}
	}
	i = 0;
	while(1) {
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[1] == kjg_val2[0] && kjg_val2[3] == rx_count) break;
		usleep(100);
		i++;
		if(i > 10) {
			//outb(0x00, addr); //clear
			break;//return 0;
		}
	}
	printf("ch%d status %d %d %d %d, ", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);

	checksum_val = 0;
	printf("rxd");
	for(i=0; i < 4; i++) {
		kjg_val[i] = inb(addr+1);
		checksum_val += kjg_val[i];
		printf(" %02x", kjg_val[i]);
	}
	printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		kjg_val[i] = inb(addr+1);
		checksum_val += kjg_val[i];
		printf("%02x ", kjg_val[i]);
	}
	printf("\n");
	if(checksum_enable == 1) {
		checksum_h = inb(addr+5);
		checksum_l = inb(addr+6);
		ui_tmp = ((unsigned int)checksum_h << 8) + checksum_l;
		kjg_val2[6] = inb(addr+7);
		if(checksum_val != ui_tmp) {
			printf("checksum fail %d %04x, %02x %02x %d, %d %d\n", can_ch,
				checksum_val, checksum_h, checksum_l, checksum_val-ui_tmp, kjg_val2[0], kjg_val2[6]);
		}
	}
	outb(0x00, addr); //clear

	i = 0;
	if(kjg_val2[0] == 10 && kjg_val2[1] == kjg_val2[0]
		&& kjg_val2[2] == 0x02 && kjg_val2[3] == 5) i++;
	if(kjg_val[1] == 0x05 && kjg_val[2] == 0x05 && kjg_val[3] == 0x00
		&& kjg_val[4] == 0x30 && kjg_val[5] == 0x10
		&& kjg_val[6] == (can_ch) && kjg_val[7] == (can_ch+1)
		&& kjg_val[8] == (can_ch+2) && kjg_val[9] == (can_ch+3)) i++;
	if(i == 2) rtn = 1;

	if(i != 2) printf("fail ch%d %d\n", can_ch, i);

	return rtn;
}

unsigned short CHIP_CalculateCRC16(unsigned char *data, unsigned short size)
{
	unsigned char index;
	unsigned short init = 0xFFFF;

	while(size-- != 0) {
		index = ((unsigned char *)&init)[1] ^ *data++;
		init = (init << 8) ^ crc16Table[index];
	}

	return init;
}

int exec_t2_wr(int can_ch)
{
    unsigned char tx_count, rx_count, tx_cmd, tx_data[12];
	int rtn = 0, addr, i;
	union unsigned_short_tmpData crc;

	//kjg_240503
	unsigned char uc_tmp, checksum_enable, checksum_h, checksum_l;
	unsigned int checksum_val, ui_tmp;
	checksum_enable = 1; //0:checksum unuse, 1:checksum use
	checksum_val = 0;

	tx_count = 3 + 9;
	rx_count = 1;
	tx_cmd = 0x00; //chip command
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	uc_tmp = tx_count;
	outb(uc_tmp, addr+1);
	checksum_val += uc_tmp;
	uc_tmp = rx_count;
	outb(uc_tmp, addr+2);
	checksum_val += uc_tmp;
	uc_tmp = tx_cmd;
	outb(uc_tmp, addr+3);
	checksum_val += uc_tmp;

	tx_data[0] = 0xA0;
	tx_data[1] = 0x10;
	tx_data[2] = 0x04;
	tx_data[3] = can_ch+10;
	tx_data[4] = can_ch+11;
	tx_data[5] = can_ch+12;
	tx_data[6] = can_ch+13;

	uc_tmp = tx_data[0]; //write with crc
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = tx_data[1];
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = tx_data[2];
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = tx_data[3];
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = tx_data[4];
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = tx_data[5];
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = tx_data[6];
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;

	crc.val = CHIP_CalculateCRC16(tx_data, 7);
	uc_tmp = crc.data[1]; //h
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = crc.data[0]; //l
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;

	if(checksum_enable == 1) {
		checksum_h = ((checksum_val & 0x0000FF00) >> 8);
		checksum_l = (checksum_val & 0x000000FF);
		outb(checksum_h, addr+6);
		outb(checksum_l, addr+7);
	}

	outb(0x00, addr+5); //spi_start

	i = 0;
	while(1) {
		kjg_val2[0] = inb(addr); //spi_tx_index
		//kjg_val2[2] = inb(addr+3); //fifo_status
		//kjg_val2[2] = (inb(addr+3) & 0x03); //fifo_status
		kjg_val2[2] = (inb(addr+3) & 0x0F); //fifo_status
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[2] == 0x02) break;
		usleep(100);
		i++;
		if(i > 10) {
			//outb(0x00, addr); //clear
			break;//return 0;
		}
	}
	i = 0;
	while(1) {
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[1] == kjg_val2[0] && kjg_val2[3] == rx_count) break;
		usleep(100);
		i++;
		if(i > 10) {
			//outb(0x00, addr); //clear
			break;//return 0;
		}
	}
	printf("ch%d status %d %d %d %d, ", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);

	checksum_val = 0;
	printf("rxd");
	for(i=0; i < 4; i++) {
		kjg_val[i] = inb(addr+1);
		checksum_val += kjg_val[i];
		printf(" %02x", kjg_val[i]);
	}
	printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		kjg_val[i] = inb(addr+1);
		checksum_val += kjg_val[i];
		printf("%02x ", kjg_val[i]);
	}
	printf("\n");
	if(checksum_enable == 1) {
		checksum_h = inb(addr+5);
		checksum_l = inb(addr+6);
		ui_tmp = ((unsigned int)checksum_h << 8) + checksum_l;
		if(checksum_val != ui_tmp) {
			printf("checksum fail %d %04x, %02x %02x, %d\n", can_ch,
				checksum_val, checksum_h, checksum_l, checksum_val-ui_tmp);
		}
	}
	outb(0x00, addr); //clear

	i = 0;
	if(kjg_val2[0] == 13 && kjg_val2[1] == kjg_val2[0]
		&& kjg_val2[2] == 0x02 && kjg_val2[3] == 1) i++;
	if(kjg_val[1] == 0x0C && kjg_val[2] == 0x01 && kjg_val[3] == 0x00
		&& kjg_val[4] == 0xA0 && kjg_val[5] == 0x10 && kjg_val[6] == 0x04
		&& kjg_val[7] == (can_ch+10) && kjg_val[8] == (can_ch+11)
		&& kjg_val[9] == (can_ch+12) && kjg_val[10] == (can_ch+13)) i++;
	if(i == 2) rtn = 1;

	if(i != 2) printf("fail ch%d %d\n", can_ch, i);

	return rtn;
}

int exec_t2_rd(int can_ch)
{
    unsigned char tx_count, rx_count, tx_cmd, rx_data[12];
	int rtn = 0, addr, i;
	union unsigned_short_tmpData crc;

	//kjg_240503
	unsigned char uc_tmp, checksum_enable, checksum_h, checksum_l;
	unsigned int checksum_val, ui_tmp;
	checksum_enable = 1; //0:checksum unuse, 1:checksum use
	checksum_val = 0;

	tx_count = 3 + 3;
	rx_count = 1 + 6;
	tx_cmd = 0x00; //chip command
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	uc_tmp = tx_count;
	outb(uc_tmp, addr+1);
	checksum_val += uc_tmp;
	uc_tmp = rx_count;
	outb(uc_tmp, addr+2);
	checksum_val += uc_tmp;
	uc_tmp = tx_cmd;
	outb(uc_tmp, addr+3);
	checksum_val += uc_tmp;

	rx_data[0] = 0xB0;
	rx_data[1] = 0x10;
	rx_data[2] = 0x04;

	uc_tmp = rx_data[0]; //read with crc
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = rx_data[1];
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;
	uc_tmp = rx_data[2];
	outb(uc_tmp, addr+4);
	checksum_val += uc_tmp;

	if(checksum_enable == 1) {
		checksum_h = ((checksum_val & 0x0000FF00) >> 8);
		checksum_l = (checksum_val & 0x000000FF);
		outb(checksum_h, addr+6);
		outb(checksum_l, addr+7);
	}

	outb(0x00, addr+5); //spi_start

	i = 0;
	while(1) {
		kjg_val2[0] = inb(addr); //spi_tx_index
		//kjg_val2[2] = inb(addr+3); //fifo_status
		//kjg_val2[2] = (inb(addr+3) & 0x03); //fifo_status
		kjg_val2[2] = (inb(addr+3) & 0x0F); //fifo_status
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[2] == 0x02) break;
		usleep(100);
		i++;
		if(i > 10) {
			//outb(0x00, addr); //clear
			break;//return 0;
		}
	}
	i = 0;
	while(1) {
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[1] == kjg_val2[0] && kjg_val2[3] == rx_count) break;
		usleep(100);
		i++;
		if(i > 10) {
			//outb(0x00, addr); //clear
			break;//return 0;
		}
	}
	printf("ch%d status %d %d %d %d, ", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);

	checksum_val = 0;
	printf("rxd");
	for(i=0; i < 4; i++) {
		kjg_val[i] = inb(addr+1);
		checksum_val += kjg_val[i];
		printf(" %02x", kjg_val[i]);
	}
	printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		kjg_val[i] = inb(addr+1);
		checksum_val += kjg_val[i];
		printf("%02x ", kjg_val[i]);
	}
	printf("\n");
	if(checksum_enable == 1) {
		checksum_h = inb(addr+5);
		checksum_l = inb(addr+6);
		ui_tmp = ((unsigned int)checksum_h << 8) + checksum_l;
		if(checksum_val != ui_tmp) {
			printf("checksum fail %d %04x, %02x %02x, %d\n", can_ch,
				checksum_val, checksum_h, checksum_l, checksum_val-ui_tmp);
		}
	}
	outb(0x00, addr); //clear

	for(i=3; i < rx_count; i++) rx_data[i] = kjg_val[i+4];
	crc.val = CHIP_CalculateCRC16((unsigned char *)&rx_data[0], 7);
	//for(i=0; i < 7; i++) printf("%02x ", rx_data[i]);
	//printf("crc %02x %02x\n", crc.data[1], crc.data[0]);

	i = 0;
	if(kjg_val2[0] == 13 && kjg_val2[1] == kjg_val2[0]
		&& kjg_val2[2] == 0x02 && kjg_val2[3] == 7) i++;
	if(kjg_val[1] == 0x06 && kjg_val[2] == 0x07 && kjg_val[3] == 0x00
		&& kjg_val[4] == 0xB0 && kjg_val[5] == 0x10 && kjg_val[6] == 0x04
		&& kjg_val[7] == (can_ch+10) && kjg_val[8] == (can_ch+11)
		&& kjg_val[9] == (can_ch+12) && kjg_val[10] == (can_ch+13)) i++;
	if(crc.data[1] == kjg_val[11] && crc.data[0] == kjg_val[12]) i++;
	if(i == 3) rtn = 1;

	if(i != 3) printf("fail ch%d %d\n", can_ch, i);

	return rtn;
}


int exec_cmd_error_count_state_get(int can_ch)
{
    unsigned char tx_count, rx_count, tx_cmd;
	int rtn = 0, addr, i;

	tx_count = 3 + 2;
	rx_count = 1 + 4;
	tx_cmd = 0x00; //chip command
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	outb(tx_count, addr+1);
	outb(rx_count, addr+2);
	outb(tx_cmd, addr+3);

	//_CHIP_REG_CiTREC = 0x034
	outb(0x30, addr+4); //read byte
	outb(0x34, addr+4);

	outb(0x00, addr+5); //spi_start

	i = 0;
	while(1) {
		kjg_val2[0] = inb(addr); //spi_tx_index
		//kjg_val2[2] = inb(addr+3); //fifo_status
		//kjg_val2[2] = (inb(addr+3) & 0x03); //fifo_status
		kjg_val2[2] = (inb(addr+3) & 0x0F); //fifo_status
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[2] == 0x02) break;
		usleep(100);
		i++;
		if(i > 10) {
			outb(0x00, addr); //clear
			return 0;
		}
	}
	i = 0;
	while(1) {
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[1] == kjg_val2[0] && kjg_val2[3] == rx_count) break;
		usleep(100);
		i++;
		if(i > 10) {
			outb(0x00, addr); //clear
			return 0;
		}
	}
	printf("ch%d status %d %d %d %d, ", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);

	printf("rxd");
	for(i=0; i < 4; i++) {
		kjg_val[i] = inb(addr+1);
		printf(" %02x", kjg_val[i]);
	}
	printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		kjg_val[i] = inb(addr+1);
		printf("%02x ", kjg_val[i]);
	}
	printf("\n");
	outb(0x00, addr); //clear

	i = 0;
	if(kjg_val2[0] == 10 && kjg_val2[1] == kjg_val2[0]
		&& kjg_val2[2] == 0x02 && kjg_val2[3] == 5) i++;
	if(kjg_val[1] == 0x05 && kjg_val[2] == 0x05 && kjg_val[3] == 0x00
		&& kjg_val[4] == 0x30 && kjg_val[5] == 0x10
		&& kjg_val[6] == (can_ch) && kjg_val[7] == (can_ch+1)
		&& kjg_val[8] == (can_ch+2) && kjg_val[9] == (can_ch+3)) i++;
	if(i == 2) rtn = 1;

	return rtn;
}

int exec_cmd_bus_diagnostics_get(int can_ch)
{
    unsigned char tx_count, rx_count, tx_cmd;
	int rtn = 0, addr, i;

	tx_count = 3 + 2;
	rx_count = 1 + 8;
	tx_cmd = 0x00; //chip command
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	outb(tx_count, addr+1);
	outb(rx_count, addr+2);
	outb(tx_cmd, addr+3);

	//_CHIP_REG_CiBDIAG0 = 0x038
	outb(0x30, addr+4); //read byte
	outb(0x38, addr+4);

	outb(0x00, addr+5); //spi_start

	i = 0;
	while(1) {
		kjg_val2[0] = inb(addr); //spi_tx_index
		//kjg_val2[2] = inb(addr+3); //fifo_status
		//kjg_val2[2] = (inb(addr+3) & 0x03); //fifo_status
		kjg_val2[2] = (inb(addr+3) & 0x0F); //fifo_status
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[2] == 0x02) break;
		usleep(100);
		i++;
		if(i > 10) {
			outb(0x00, addr); //clear
			return 0;
		}
	}
	i = 0;
	while(1) {
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[1] == kjg_val2[0] && kjg_val2[3] == rx_count) break;
		usleep(100);
		i++;
		if(i > 10) {
			outb(0x00, addr); //clear
			return 0;
		}
	}
	printf("ch%d status %d %d %d %d, ", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);

	printf("rxd");
	for(i=0; i < 4; i++) {
		kjg_val[i] = inb(addr+1);
		printf(" %02x", kjg_val[i]);
	}
	printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		kjg_val[i] = inb(addr+1);
		printf("%02x ", kjg_val[i]);
	}
	printf("\n");
	outb(0x00, addr); //clear

	i = 0;
	if(kjg_val2[0] == 10 && kjg_val2[1] == kjg_val2[0]
		&& kjg_val2[2] == 0x02 && kjg_val2[3] == 5) i++;
	if(kjg_val[1] == 0x05 && kjg_val[2] == 0x05 && kjg_val[3] == 0x00
		&& kjg_val[4] == 0x30 && kjg_val[5] == 0x10
		&& kjg_val[6] == (can_ch) && kjg_val[7] == (can_ch+1)
		&& kjg_val[8] == (can_ch+2) && kjg_val[9] == (can_ch+3)) i++;
	if(i == 2) rtn = 1;

	return rtn;
}

int exec_cmd_bus_diagnostics_clear(int can_ch)
{
    unsigned char tx_count, rx_count, tx_cmd;
	int rtn = 0, addr, i;

	tx_count = 3 + 10;
	rx_count = 1 + 0;
	tx_cmd = 0x00; //chip command
	addr = 0x840 + (0x10 * can_ch);

	outb(0x00, addr); //clear

	outb(tx_count, addr+1);
	outb(rx_count, addr+2);
	outb(tx_cmd, addr+3);

	//_CHIP_REG_CiBDIAG0 = 0x038
	outb(0x20, addr+4); //write byte
	outb(0x38, addr+4);

	outb(0x00, addr+4);
	outb(0x00, addr+4);
	outb(0x00, addr+4);
	outb(0x00, addr+4);

	outb(0x00, addr+4);
	outb(0x00, addr+4);
	outb(0x00, addr+4);
	outb(0x00, addr+4);

	outb(0x00, addr+5); //spi_start

	i = 0;
	while(1) {
		kjg_val2[0] = inb(addr); //spi_tx_index
		//kjg_val2[2] = inb(addr+3); //fifo_status
		//kjg_val2[2] = (inb(addr+3) & 0x03); //fifo_status
		kjg_val2[2] = (inb(addr+3) & 0x0F); //fifo_status
		if(kjg_val2[0] == (tx_count + rx_count) && kjg_val2[2] == 0x02) break;
		usleep(100);
		i++;
		if(i > 10) {
			outb(0x00, addr); //clear
			return 0;
		}
	}
	i = 0;
	while(1) {
		kjg_val2[1] = inb(addr+2); //fifo_rx_wr_index
		kjg_val2[3] = inb(addr+4); //rx_length
		if(kjg_val2[1] == kjg_val2[0] && kjg_val2[3] == rx_count) break;
		usleep(100);
		i++;
		if(i > 10) {
			outb(0x00, addr); //clear
			return 0;
		}
	}
	printf("ch%d status %d %d %d %d, ", can_ch,
		kjg_val2[0], kjg_val2[1], kjg_val2[2], kjg_val2[3]);

	printf("rxd");
	for(i=0; i < 4; i++) {
		kjg_val[i] = inb(addr+1);
		printf(" %02x", kjg_val[i]);
	}
	printf(", ");
	for(i=4; i < tx_count + rx_count; i++) {
		kjg_val[i] = inb(addr+1);
		printf("%02x ", kjg_val[i]);
	}
	printf("\n");
	outb(0x00, addr); //clear

	i = 0;
	if(kjg_val2[0] == 10 && kjg_val2[1] == kjg_val2[0]
		&& kjg_val2[2] == 0x02 && kjg_val2[3] == 5) i++;
	if(kjg_val[1] == 0x05 && kjg_val[2] == 0x05 && kjg_val[3] == 0x00
		&& kjg_val[4] == 0x30 && kjg_val[5] == 0x10
		&& kjg_val[6] == (can_ch) && kjg_val[7] == (can_ch+1)
		&& kjg_val[8] == (can_ch+2) && kjg_val[9] == (can_ch+3)) i++;
	if(i == 2) rtn = 1;

	return rtn;
}

void exec_T3(void)
{
	int i;

	for(i=0; i < 1000000; i++) {
		outb(0xAA, 0x804);
		outb(0x55, 0x805);
		kjg_val[0] = inb(0x806);
		kjg_val[1] = inb(0x807);
		kjg_val[2] = inb(0x808);
		if(kjg_val[0] == 0x64 && kjg_val[1] == 0x33
			&& kjg_val[2] == 0x31) {
		} else {
			printf("%02x %02x %02x : %d\n",
				kjg_val[0], kjg_val[1], kjg_val[2], i);
		}
	}

	printf("T3 end\n");
}

int main(void)
{
    char tmp[16];
	unsigned char o_val, i_val; 
    unsigned char adVal, adVal2, tx_count;
	short int tmpVal;
    int	retval, addr, val, type, mode, phase1;
	int	o_addr, i_addr, count;
    int	i, j, k, can_ch, tmp1, tmp2;
    struct timeval tv;
    fd_set rfds;
	FILE *fp;

	phase1 = type = o_addr = i_addr = count = 0;
	o_val = 0;
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

			kjg_val[0] = inb(0x806);
			kjg_val[1] = inb(0x807);
			kjg_val[2] = inb(0x808);
			printf("bd1 ver %x %x %x\n", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x44 || kjg_val[0] == 0x64
				|| kjg_val[0] == 0x54 || kjg_val[0] == 0x74) { //D,d,T,t
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x826);
			kjg_val[1] = inb(0x827);
			kjg_val[2] = inb(0x828);
			printf("bd2 ver %x %x %x\n", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x44 || kjg_val[0] == 0x64
				|| kjg_val[0] == 0x54 || kjg_val[0] == 0x74) { //D,d,T,t
				tmp2 = 8;
			} else tmp2 = 4;
			//printf("tmp1:%d, tmp2:%d\n", tmp1, tmp2);

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
			//can_ch = 0;
				if(exec_cmd1(can_ch) > 0) k++;
			}

			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d %d, %d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d, %d\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("OK %d %d, %d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d, %d\n", tmp1, tmp2, k);
			}
			printf("\n");
		} else if(strcmp(tmp, "reset") == 0) {
			mode = NONE;

			outb(0xFF, 0x805);
			outb(0xFF, 0x825);
			usleep(1000);
			outb(0x00, 0x805);
			outb(0x00, 0x825);

			kjg_val[0] = inb(0x806);
			kjg_val[1] = inb(0x807);
			kjg_val[2] = inb(0x808);
			printf("bd1 ver %x %x %x\n", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x44 || kjg_val[0] == 0x64
				|| kjg_val[0] == 0x54 || kjg_val[0] == 0x74) { //D,d,T,t
				tmp1 = 0;
			} else tmp1 = 4;

			kjg_val[0] = inb(0x826);
			kjg_val[1] = inb(0x827);
			kjg_val[2] = inb(0x828);
			printf("bd2 ver %x %x %x\n", kjg_val[0], kjg_val[1], kjg_val[2]);
			if(kjg_val[0] == 0x44 || kjg_val[0] == 0x64
				|| kjg_val[0] == 0x54 || kjg_val[0] == 0x74) { //D,d,T,t
				tmp2 = 8;
			} else tmp2 = 4;

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(exec_cmd_reset(can_ch) > 0) k++;
			}
			kjg_val3 = 0;

			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d %d, %d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d, %d\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("OK %d %d, %d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d, %d\n", tmp1, tmp2, k);
			}
			printf("\n");
		} else if(strcmp(tmp, "cmd2") == 0) {
			mode = NONE;

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
			//can_ch = 0;
				if(exec_cmd2(can_ch) > 0) k++;
			}

			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d %d, %d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d, %d\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("OK %d %d, %d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d, %d\n", tmp1, tmp2, k);
			}
			printf("\n");
		} else if(strcmp(tmp, "cmd3") == 0) {
			mode = NONE;

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
			//can_ch = 0;
				if(exec_cmd3(can_ch) > 0) k++;
			}

			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d %d, %d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d, %d\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("OK %d %d, %d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d, %d\n", tmp1, tmp2, k);
			}
			printf("\n");
		} else if(strcmp(tmp, "t1w") == 0) { //write
			mode = NONE;

			if((tmp1 == 0 && tmp2 == 0) || (tmp1 == 4 && tmp2 == 4)) {
				printf("first request reset : %d, %d\n", tmp1, tmp2);
			}

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
			//can_ch = 0;
				if(exec_t1_wr(can_ch) > 0) k++;
			}


			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d\n", k);
				else printf("NG!!!! %d\n", k);
			} else {
				if(k == 8) printf("OK %d\n", k);
				else printf("NG!!!! %d\n", k);
			}
			printf("\n");
		} else if(strcmp(tmp, "t1r") == 0) { //read
			mode = NONE;

			if((tmp1 == 0 && tmp2 == 0) || (tmp1 == 4 && tmp2 == 4)) {
				printf("first request reset : %d, %d\n", tmp1, tmp2);
			}

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
			//can_ch = 0;
				if(exec_t1_rd(can_ch) > 0) k++;
			}

			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d\n", k);
				else printf("NG!!!! %d\n", k);
			} else {
				if(k == 8) printf("OK %d\n", k);
				else printf("NG!!!! %d\n", k);
			}
			printf("\n");
		} else if(strcmp(tmp, "t1a") == 0) { //all(write & read)
			mode = NONE;

			if((tmp1 == 0 && tmp2 == 0) || (tmp1 == 4 && tmp2 == 4)) {
				printf("first request reset : %d, %d\n", tmp1, tmp2);
			}

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
			//can_ch = 0;
				j = exec_t1_wr(can_ch);
				if(j > 0) {
					k++;
				} else {
					//printf("wr ch%d %d %d\n", can_ch, j, k);
				}
			}

			if(k != 0) printf("read\n");
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
			//can_ch = 0;
				j = exec_t1_rd(can_ch);
				if(j > 0) {
					k++;
				} else {
					//printf("rd ch%d %d %d\n", can_ch, j, k);
				}
			}

			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 8) printf("OK %d\n", k);
				else printf("NG!!!! %d\n", k);
			} else {
				if(k == 16) printf("OK %d\n", k);
				else printf("NG!!!! %d\n", k);
			}
			printf("\n");
		} else if(strcmp(tmp, "t2w") == 0) { //write with crc
			mode = NONE;

			if((tmp1 == 0 && tmp2 == 0) || (tmp1 == 4 && tmp2 == 4)) {
				printf("first request reset : %d, %d\n", tmp1, tmp2);
			}

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
			//can_ch = 0;
				if(exec_t2_wr(can_ch) > 0) k++;
			}


			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d\n", k);
				else printf("NG!!!! %d\n", k);
			} else {
				if(k == 8) printf("OK %d\n", k);
				else printf("NG!!!! %d\n", k);
			}
			printf("\n");
		} else if(strcmp(tmp, "t2r") == 0) { //read with crc
			mode = NONE;

			if((tmp1 == 0 && tmp2 == 0) || (tmp1 == 4 && tmp2 == 4)) {
				printf("first request reset : %d, %d\n", tmp1, tmp2);
			}

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
			//can_ch = 0;
				if(exec_t2_rd(can_ch) > 0) k++;
			}

			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d\n", k);
				else printf("NG!!!! %d\n", k);
			} else {
				if(k == 8) printf("OK %d\n", k);
				else printf("NG!!!! %d\n", k);
			}
			printf("\n");
		} else if(strcmp(tmp, "t2a") == 0) { //all(write & read with crc)
			mode = NONE;

			if((tmp1 == 0 && tmp2 == 0) || (tmp1 == 4 && tmp2 == 4)) {
				printf("first request reset : %d, %d\n", tmp1, tmp2);
			}

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
			//can_ch = 0;
				if(exec_t2_wr(can_ch) > 0) k++;
			}

			if(k != 0) printf("read\n");
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
			//can_ch = 0;
				if(exec_t2_rd(can_ch) > 0) k++;
			}

			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 8) printf("OK %d\n", k);
				else printf("NG!!!! %d\n", k);
			} else {
				if(k == 16) printf("OK %d\n", k);
				else printf("NG!!!! %d\n", k);
			}
			printf("\n");
		} else if(strcmp(tmp, "t3") == 0) {
			mode = NONE;

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(exec_cmd_error_count_state_get(can_ch) > 0) k++;
			}

			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d %d : k=%d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d : k=%d\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("OK %d %d : k=%d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d : k=%d\n", tmp1, tmp2, k);
			}
			printf("\n");
		} else if(strcmp(tmp, "t4") == 0) {
			mode = NONE;

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(exec_cmd_bus_diagnostics_get(can_ch) > 0) k++;
			}

			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d %d : k=%d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d : k=%d\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("OK %d %d : k=%d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d : k=%d\n", tmp1, tmp2, k);
			}
			printf("\n");
		} else if(strcmp(tmp, "t5") == 0) {
			mode = NONE;

			k = 0;
			for(can_ch=tmp1; can_ch < tmp2; can_ch++) {
				if(exec_cmd_bus_diagnostics_clear(can_ch) > 0) k++;
			}

			if((tmp1 == 0 && tmp2 == 4) || (tmp1 == 4 && tmp2 == 8)) {
				if(k == 4) printf("OK %d %d : k=%d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d : k=%d\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("OK %d %d : k=%d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d : k=%d\n", tmp1, tmp2, k);
			}
			printf("\n");
		} else if(strcmp(tmp, "T3") == 0) {
			exec_T3();
		} else if(strcmp(tmp, "t21") == 0) {
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
				printf("tmp1 %d, tmp2 %d, test21 NG!!!!\n", tmp1, tmp2);
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
				else printf("%d %d : k=%d NG!!!!\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!!\n", tmp1, tmp2, k);
			}
		} else if(strcmp(tmp, "t23") == 0) {
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
				printf("tmp1 %d, tmp2 %d, test23 NG!!!!\n", tmp1, tmp2);
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
				else printf("%d %d : k=%d NG!!!!\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("%d %d : k=%d OK\n", tmp1, tmp2, k);
				else printf("%d %d : k=%d NG!!!!\n", tmp1, tmp2, k);
			}
		} else if(strcmp(tmp, "t30") == 0) {
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
				else printf("NG!!!! %d %d : k=%d\n", tmp1, tmp2, k);
			} else {
				if(k == 8) printf("OK %d %d : k=%d\n", tmp1, tmp2, k);
				else printf("NG!!!! %d %d : k=%d\n", tmp1, tmp2, k);
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
