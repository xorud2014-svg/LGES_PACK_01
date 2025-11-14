//rt_can_fd
//RT-Linux kernel module for CAN_FD.

//kjg_181121 #include <linux/config.h>
//kjg_181121 #include <linux/errno.h>
#include <linux/ioport.h>
//kjg_181121 #include <linux/kernel.h>
#include <linux/module.h>
//kjg_181121 #include <linux/version.h>

//kjg_181121 #include <asm/system.h>
#include <asm/io.h>

//kjg_181121 #include <rtl_conf.h>
#include <rtl_core.h>
#include <rtl_sync.h>
#include <pthread.h>
#include <rtl.h> //kjg_180521
#include <rtl_core.h> //kjg_180521

#include "rt_can.h"
#include "rt_can_ext.h"

#include "../../INC/datastore.h"
extern S_SYSTEM_DATA	*myData;

//kjg_181121 pthread_t thread; //kjg_180521
//kjg_2020_1212 int set_parameter[RT_CAN_FD_CNT][8]; //kjg_180523
int set_parameter[RT_CAN_FD_CNT][16];
unsigned char ba_rxd[RT_CAN_FD_CNT][MAX_MSG_SIZE];
unsigned int rx_can_id[RT_CAN_FD_CNT][100];
long long rx_can_time[RT_CAN_FD_CNT][100];

//used = 0 - port and irq setting by rt_can_set_param.
//If you want to work like a standard rt_can_fd you can set used = 1.
#ifdef __SBC_EM104_A5362__
struct rt_can_fd_struct rt_can_fd_table[RT_CAN_FD_CNT] =
{ //sbc : a5362
    {0, RT_CAN_FD_BASE_BAUD, 0x810, 5, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {1, RT_CAN_FD_BASE_BAUD, 0x818, 5, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {2, RT_CAN_FD_BASE_BAUD, 0x820, 5, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {3, RT_CAN_FD_BASE_BAUD, 0x828, 5, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {4, RT_CAN_FD_BASE_BAUD, 0x830, 5, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {5, RT_CAN_FD_BASE_BAUD, 0x838, 5, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {6, RT_CAN_FD_BASE_BAUD, 0x840, 5, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {7, RT_CAN_FD_BASE_BAUD, 0x848, 5, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1}
};
#else
struct rt_can_fd_struct rt_can_fd_table[RT_CAN_FD_CNT] =
{ //sbc : mark533, mark800, v621
    {0, RT_CAN_FD_BASE_BAUD, 0x810, 9, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {1, RT_CAN_FD_BASE_BAUD, 0x818, 9, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {2, RT_CAN_FD_BASE_BAUD, 0x820, 9, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {3, RT_CAN_FD_BASE_BAUD, 0x828, 9, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {4, RT_CAN_FD_BASE_BAUD, 0x830, 9, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {5, RT_CAN_FD_BASE_BAUD, 0x838, 9, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {6, RT_CAN_FD_BASE_BAUD, 0x840, 9, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1},
    {7, RT_CAN_FD_BASE_BAUD, 0x848, 9, RT_CAN_FD_CLASSIC_FLAG, rt_can_fd_share_isr, 0, 1}
};
#endif

/*kjg_181121 void *rt_can_task(void *arg)
{ //kjg_180521
	int slot=0;
	struct sched_param sch_p;

	sch_p.sched_priority = 2;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &sch_p);
	//kjg_180522 pthread_make_periodic_np(pthread_self(), gethrtime(), 250000); //250us
	//kjg_181110 pthread_make_periodic_np(pthread_self(), gethrtime(), 1500000); //1500us
	//kjg_181120 pthread_make_periodic_np(pthread_self(), gethrtime(), 1000000); //1000us kjg_181110
	pthread_make_periodic_np(pthread_self(), gethrtime(), 250000); //250us
	pthread_setfp_np(pthread_self(), 1);

	while(1) {
		tx_can_data(slot);

		slot++;
		if(slot >= RT_CAN_FD_CNT) slot = 0;
		//kjg_180522 if(slot >= 4) slot = 0;

		pthread_wait_np();
	}

	return 0;
}*/

void tx_can_data(int num)
{ //kjg_180521
	char error, debug;
	unsigned char tmp, attempts = MAX_TXQUEUE_ATTEMPTS;
	int idx, delay;
	unsigned int base;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	if(p->status != 0) { //kjg_180523
		p->status++;
		if(p->status >= 150) { //1.5ms * 8 * 150 = 1800ms
			rt_can_set_param_2(num);

			p->status = 0;
		}
		return;
	}

	if(p->tx_msg_count <= 0) return;

	p->tx_msg_read_idx++;
	if(p->tx_msg_read_idx >= RT_CAN_FD_TX_MSG_BUF_SIZE)
		p->tx_msg_read_idx = 0;
	idx = p->tx_msg_read_idx;
	p->tx_msg_count--;

	//Initialize ID and Control bits
	p->txObj.word[0] = 0;
	p->txObj.word[1] = 0;

	/*kjg_181110 if(p->type == 0) {
		p->txObj.bF.id.SID = p->tx_buf[idx].id; //Standard or Base ID
		p->txObj.bF.id.EID = 0; //Extended ID
		p->txObj.bF.ctrl.IDE = 0; //Standard frame
	} else {
		p->txObj.bF.id.SID = p->tx_buf[idx].id; //Standard or Base ID
		p->txObj.bF.id.EID = p->tx_buf[idx].id; //Extended ID
		p->txObj.bF.ctrl.IDE = 1; //Extended frame
	}*/
	//if(p->tx_buf[idx].id < 0x800) {
	if(p->type == 0) {
		p->txObj.bF.id.SID = p->tx_buf[idx].id; //Standard or Base ID
		p->txObj.bF.id.EID = 0; //Extended ID
		p->txObj.bF.ctrl.IDE = 0; //Standard frame
	} else {
		p->txObj.bF.id.SID = (p->tx_buf[idx].id >> 18) & 0x7FF;
		p->txObj.bF.id.EID = p->tx_buf[idx].id & 0x3FFFF;
		p->txObj.bF.ctrl.IDE = 1; //Extended frame
	}
	//printk(KERN_WARNING
	//	"rt_can_fd: kjgd1 %d %x %x %x\n", p->type, p->tx_buf[idx].id, p->txObj.bF.id.SID, p->txObj.bF.id.EID);

	if(p->tx_buf[idx].can_fd_flag == 0) {
		p->txObj.bF.ctrl.FDF = 0;
		p->txObj.bF.ctrl.BRS = 0;
	} else {
		p->txObj.bF.ctrl.FDF = 1; //CAN FD frame
		p->txObj.bF.ctrl.BRS = 1; //Switch bit rate
	}
	p->txObj.bF.ctrl.DLC = CHIP_DataBytesToDlc(p->tx_buf[idx].length);
	if(p->txObj.bF.ctrl.DLC == 0) {
		p->txObj.bF.ctrl.RTR = 1; //A remote frame request
	} else {
		p->txObj.bF.ctrl.RTR = 0; //Not a remote frame request
	}
	//Sequenct: doesn't get transmitted, but will be stored in TEF
	//kjg_180405 p->txObj.bF.ctrl.SEQ = 1; //kjgw_2020_0323

	rtl_hard_disable_irq(p->irq);

	base = RT_CAN_FD_BASE_ADDR;
	tmp = 0x01;
	tmp = tmp << num;
	outb(tmp, base + RT_CAN_FD_PORT_SELECT);

	attempts = 2; //kjgd_180522

	do { //Check if FIFO is not full
		CHIP_TransmitChannelEventGet(num, CHIP_FIFO_CH2, &p->txFlags);

		if(attempts == 0) {
			debug = 0;
			//debug = 1;
			if(debug == 1) { //kjg_180522
				for(delay=0; delay < DELAY_10US; delay++) {}
				CHIP_ErrorCountStateGet(num, &p->tec, &p->rec, &p->errorFlags);

				printk(KERN_WARNING
					"rt_can_fd: fd_chip_write error1 %d, %x, %x, %x, %d, %d\n",
					num, p->tec, p->rec, p->errorFlags, p->txObj.bF.ctrl.DLC,
					p->tx_buf[idx].length);
			}
			
			break;
		}

		attempts--;
	} while(!(p->txFlags & CHIP_TX_FIFO_NOT_FULL_EVENT));

	if(attempts != 0) {
		//Load message and transmit
		error = CHIP_TransmitChannelLoad(num, CHIP_FIFO_CH2, true, p, idx);

		//Message Sent
	}

	rtl_hard_enable_irq(p->irq);
}

void tx_can_data_2(int num)
{ //kjg_181121
	char error, debug;
	unsigned char tmp, attempts = MAX_TXQUEUE_ATTEMPTS;
	int idx, delay;
	unsigned int base;
	struct rt_can_fd_struct *p;
	//long long hrt[10];

	p = &(rt_can_fd_table[num]);

	/*if(p->status != 0) { //kjg_180523
		p->status++;
		if(p->status >= 750) { //750 / 5 = 150 -> 1500ms
			rt_can_set_param_2(num);

			p->status = 0;
		}
		return;
	}*/

	if(p->status != 0) { //kjg_2020_1212
		p->status++;
		if(p->status >= 750) { //750 / 5 = 150 -> 1500ms
			rt_can_set_param_3(num); //750~781
		}
		return;
	}


	if(p->tx_msg_count <= 0) return;

	//hrt[0] = gethrtime(); //kjg_2020_0330

	p->tx_msg_read_idx++;
	if(p->tx_msg_read_idx >= RT_CAN_FD_TX_MSG_BUF_SIZE)
		p->tx_msg_read_idx = 0;
	idx = p->tx_msg_read_idx;
	p->tx_msg_count--;

	//Initialize ID and Control bits
	p->txObj.word[0] = 0;
	p->txObj.word[1] = 0;

	if(p->type == 0) {
		p->txObj.bF.id.SID = p->tx_buf[idx].id; //Standard or Base ID
		p->txObj.bF.id.EID = 0; //Extended ID
		p->txObj.bF.ctrl.IDE = 0; //Standard frame
	} else {
		p->txObj.bF.id.SID = (p->tx_buf[idx].id >> 18) & 0x7FF;
		p->txObj.bF.id.EID = p->tx_buf[idx].id & 0x3FFFF;
		p->txObj.bF.ctrl.IDE = 1; //Extended frame
	}
	//printk(KERN_WARNING
	//	"rt_can_fd: kjgd1 %d %x %x %x\n", p->type, p->tx_buf[idx].id, p->txObj.bF.id.SID, p->txObj.bF.id.EID);

	if(p->tx_buf[idx].can_fd_flag == 0) {
		p->txObj.bF.ctrl.FDF = 0;
		p->txObj.bF.ctrl.BRS = 0;
	} else {
		p->txObj.bF.ctrl.FDF = 1; //CAN FD frame
		p->txObj.bF.ctrl.BRS = 1; //Switch bit rate
	}
	p->txObj.bF.ctrl.DLC = CHIP_DataBytesToDlc(p->tx_buf[idx].length);
	if(p->txObj.bF.ctrl.DLC == 0) {
		p->txObj.bF.ctrl.RTR = 1; //A remote frame request
	} else {
		p->txObj.bF.ctrl.RTR = 0; //Not a remote frame request
	}
	//Sequenct: doesn't get transmitted, but will be stored in TEF
	//kjg_180405 p->txObj.bF.ctrl.SEQ = 1; //kjgw_2020_0323

	//rtl_hard_disable_irq(p->irq);

	base = RT_CAN_FD_BASE_ADDR;
	tmp = 0x01;
	tmp = tmp << num;
	outb(tmp, base + RT_CAN_FD_PORT_SELECT);

	attempts = 2; //kjgd_180522

	do { //Check if FIFO is not full
		CHIP_TransmitChannelEventGet(num, CHIP_FIFO_CH2, &p->txFlags);

		if(attempts == 0) {
			debug = 0;
			if(debug == 1) { //kjg_180522
				delay=0; //kjg_181121_2 for(delay=0; delay < DELAY_10US; delay++) {}
				CHIP_ErrorCountStateGet(num, &p->tec, &p->rec, &p->errorFlags);

				printk(KERN_WARNING
					"rt_can_fd: fd_chip_write error2 %d, %x, %x, %x, %d, %d\n",
					num, p->tec, p->rec, p->errorFlags, p->txObj.bF.ctrl.DLC,
					p->tx_buf[idx].length);
			}
			
			break;
		}

		attempts--;
	} while(!(p->txFlags & CHIP_TX_FIFO_NOT_FULL_EVENT));

	//hrt[1] = gethrtime(); //kjg_2020_0330

	if(attempts != 0) {
		//Load message and transmit
		error = CHIP_TransmitChannelLoad(num, CHIP_FIFO_CH2, true, p, idx);

		//Message Sent
	}

	//rtl_hard_enable_irq(p->irq);
	
	//hrt[2] = gethrtime(); //kjg_2020_0330
	//hrt[9] = hrt[2] - hrt[1];
	//if(hrt[9] > myData->test_val_ll[num]) {
	//	myData->test_val_ll[num] = hrt[9];
	//}
}

/*void tx_can_data_2(int num)
{ //kjg_181121, kjg_2020_1206
	char error, debug;
	unsigned char tmp, attempts = MAX_TXQUEUE_ATTEMPTS;
	int idx, delay;
	unsigned int base;
	struct rt_can_fd_struct *p;
	//long long hrt[10];

	p = &(rt_can_fd_table[num]);

	if(p->status != 0) { //kjg_180523
		p->status++;
		if(p->status >= 750) { //750 / 6 = 125 -> 1250ms
			rt_can_set_param_2(num);

			p->status = 0;
		}
		return;
	}

	if(p->tx_msg_count <= 0) return;

	CHIP_TransmitChannelEventGet(num, CHIP_FIFO_CH2, &p->txFlags);
	if(!(p->txFlags & CHIP_TX_FIFO_NOT_FULL_EVENT)) return;

	//hrt[0] = gethrtime(); //kjg_2020_0330

	p->tx_msg_read_idx++;
	if(p->tx_msg_read_idx >= RT_CAN_FD_TX_MSG_BUF_SIZE)
		p->tx_msg_read_idx = 0;
	idx = p->tx_msg_read_idx;
	p->tx_msg_count--;

	//Initialize ID and Control bits
	p->txObj.word[0] = 0;
	p->txObj.word[1] = 0;

	if(p->type == 0) {
		p->txObj.bF.id.SID = p->tx_buf[idx].id; //Standard or Base ID
		p->txObj.bF.id.EID = 0; //Extended ID
		p->txObj.bF.ctrl.IDE = 0; //Standard frame
	} else {
		p->txObj.bF.id.SID = (p->tx_buf[idx].id >> 18) & 0x7FF;
		p->txObj.bF.id.EID = p->tx_buf[idx].id & 0x3FFFF;
		p->txObj.bF.ctrl.IDE = 1; //Extended frame
	}
	//printk(KERN_WARNING
	//	"rt_can_fd: kjgd1 %d %x %x %x\n", p->type, p->tx_buf[idx].id, p->txObj.bF.id.SID, p->txObj.bF.id.EID);

	if(p->tx_buf[idx].can_fd_flag == 0) {
		p->txObj.bF.ctrl.FDF = 0;
		p->txObj.bF.ctrl.BRS = 0;
	} else {
		p->txObj.bF.ctrl.FDF = 1; //CAN FD frame
		p->txObj.bF.ctrl.BRS = 1; //Switch bit rate
	}
	p->txObj.bF.ctrl.DLC = CHIP_DataBytesToDlc(p->tx_buf[idx].length);
	if(p->txObj.bF.ctrl.DLC == 0) {
		p->txObj.bF.ctrl.RTR = 1; //A remote frame request
	} else {
		p->txObj.bF.ctrl.RTR = 0; //Not a remote frame request
	}
	//Sequenct: doesn't get transmitted, but will be stored in TEF
	//kjg_180405 p->txObj.bF.ctrl.SEQ = 1; //kjgw_2020_0323

	//rtl_hard_disable_irq(p->irq);

	base = RT_CAN_FD_BASE_ADDR;
	tmp = 0x01;
	tmp = tmp << num;
	outb(tmp, base + RT_CAN_FD_PORT_SELECT);

	attempts = 2; //kjgd_180522

	do { //Check if FIFO is not full
		CHIP_TransmitChannelEventGet(num, CHIP_FIFO_CH2, &p->txFlags);

		if(attempts == 0) {
			debug = 0;
			if(debug == 1) { //kjg_180522
				delay=0; //kjg_181121_2 for(delay=0; delay < DELAY_10US; delay++) {}
				CHIP_ErrorCountStateGet(num, &p->tec, &p->rec, &p->errorFlags);

				printk(KERN_WARNING
					"rt_can_fd: fd_chip_write error2 %d, %x, %x, %x, %d, %d\n",
					num, p->tec, p->rec, p->errorFlags, p->txObj.bF.ctrl.DLC,
					p->tx_buf[idx].length);
			}
			
			break;
		}

		attempts--;
	} while(!(p->txFlags & CHIP_TX_FIFO_NOT_FULL_EVENT));

	//hrt[1] = gethrtime(); //kjg_2020_0330

	if(attempts != 0) {
		//Load message and transmit
		error = CHIP_TransmitChannelLoad(num, CHIP_FIFO_CH2, true, p, idx);

		//Message Sent
	}

	//rtl_hard_enable_irq(p->irq);
	
	//hrt[2] = gethrtime(); //kjg_2020_0330
	//hrt[9] = hrt[2] - hrt[1];
	//if(hrt[9] > myData->test_val_ll[num]) {
	//	myData->test_val_ll[num] = hrt[9];
	//}
}*/

void tx_can_data_3(int num)
{ //kjg_2020_1212
	char error, debug;
	unsigned char attempts = MAX_TXQUEUE_ATTEMPTS;
	int idx, read_idx;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	if(p->status != 0) { //kjg_2020_1212
		p->status++;
		if(p->status >= 750) { //750 / 5 = 150 -> 1500ms
			rt_can_set_param_3(num); //750~781
		}
		set_parameter[num][10] = 0;
		return;
	}


	if(p->tx_msg_count <= 0) {
		set_parameter[num][10] = 0;
		return;
	}

	if(set_parameter[num][12] != 0) return; //kjg_2020_1213

	switch(set_parameter[num][10]) {
		case 0:
			read_idx = p->tx_msg_read_idx + 1;
			if(read_idx >= RT_CAN_FD_TX_MSG_BUF_SIZE) read_idx = 0;
			idx = read_idx;

			//Initialize ID and Control bits
			p->txObj.word[0] = 0;
			p->txObj.word[1] = 0;

			if(p->type == 0) {
				p->txObj.bF.id.SID = p->tx_buf[idx].id; //Standard or Base ID
				p->txObj.bF.id.EID = 0; //Extended ID
				p->txObj.bF.ctrl.IDE = 0; //Standard frame
			} else {
				p->txObj.bF.id.SID = (p->tx_buf[idx].id >> 18) & 0x7FF;
				p->txObj.bF.id.EID = p->tx_buf[idx].id & 0x3FFFF;
				p->txObj.bF.ctrl.IDE = 1; //Extended frame
			}

			if(p->tx_buf[idx].can_fd_flag == 0) {
				p->txObj.bF.ctrl.FDF = 0;
				p->txObj.bF.ctrl.BRS = 0;
			} else {
				p->txObj.bF.ctrl.FDF = 1; //CAN FD frame
				p->txObj.bF.ctrl.BRS = 1; //Switch bit rate
			}
			p->txObj.bF.ctrl.DLC = CHIP_DataBytesToDlc(p->tx_buf[idx].length);
			if(p->txObj.bF.ctrl.DLC == 0) {
				p->txObj.bF.ctrl.RTR = 1; //A remote frame request
			} else {
				p->txObj.bF.ctrl.RTR = 0; //Not a remote frame request
			}
			//Sequenct: doesn't get transmitted, but will be stored in TEF
			//kjg_180405 p->txObj.bF.ctrl.SEQ = 1; //kjgw_2020_0323

			attempts = 2; //kjgd_180522

			do { //Check if FIFO is not full
				CHIP_TransmitChannelEventGet(num, CHIP_FIFO_CH2, &p->txFlags);

				if(attempts == 0) {
					debug = 0;
					if(debug == 1) { //kjg_180522
						CHIP_ErrorCountStateGet(num, &p->tec, &p->rec, &p->errorFlags);

						printk(KERN_WARNING
							"rt_can_fd: fd_chip_write error2 %d, %x, %x, %x, %d, %d\n",
							num, p->tec, p->rec, p->errorFlags, p->txObj.bF.ctrl.DLC,
							p->tx_buf[idx].length);
					}
			
					//break;
					return;
				}

				attempts--;
			} while(!(p->txFlags & CHIP_TX_FIFO_NOT_FULL_EVENT));
			set_parameter[num][10] = 1;
			break;
		case 1:
			//if(set_parameter[num][12] != 0) break; //kjg_2020_1213

			read_idx = p->tx_msg_read_idx + 1;
			if(read_idx >= RT_CAN_FD_TX_MSG_BUF_SIZE) read_idx = 0;
			idx = read_idx;

			//Load message and transmit
			error = CHIP_TransmitChannelLoad_3a(num, CHIP_FIFO_CH2, true, p, idx);
			//Message Sent
			if(error == 0) {
				error = CHIP_TransmitChannelLoad_3b(num, CHIP_FIFO_CH2, true);
				p->tx_msg_read_idx++;
				if(p->tx_msg_read_idx >= RT_CAN_FD_TX_MSG_BUF_SIZE)
					p->tx_msg_read_idx = 0;
				p->tx_msg_count--;
				set_parameter[num][10] = 0;
			} else {
				set_parameter[num][10] = 2;
			}
			break;
		case 2:
			//if(set_parameter[num][12] != 0) break; //kjg_2020_1213

			error = SPI_Transfer_3b(num,(unsigned short)set_parameter[num][11]);
			if(error == 0) {
				error = CHIP_TransmitChannelLoad_3b(num, CHIP_FIFO_CH2, true);
				p->tx_msg_read_idx++;
				if(p->tx_msg_read_idx >= RT_CAN_FD_TX_MSG_BUF_SIZE)
					p->tx_msg_read_idx = 0;
				p->tx_msg_count--;
				set_parameter[num][10] = 0;
			}
			break;
		default:
			break;
	}
}

int rt_can_write(int num, const char *msg)
{ //kjg_180521
	int idx;
	struct rt_can_fd_struct *p;

	if(max_can_fd_ch == 0) return 0; //kjg_180405

    if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING "rt_can_fd: rt_can_write error1 %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

	if(p->used <= 0) {
		printk(KERN_WARNING "rt_can_fd: rt_can_write error2 %d, %d\n",
			num, p->used);
		return -2;
	}

	if(p->tx_msg_count >= RT_CAN_FD_TX_MSG_BUF_SIZE) {
		p->tx_msg_error++;
		if(p->tx_msg_error < 10) {
			//kjg_180522 printk(KERN_WARNING
			//	"rt_can_fd: rt_can_write error3 %d, %d, %d\n", num,
			//	p->tx_msg_count, p->tx_msg_error);
			return -3;
		} else {
			//printk(KERN_WARNING
			//	"rt_can_fd: rt_can_write error4 %d, %d, %d\n", num,
			//	p->tx_msg_count, p->tx_msg_error); //kjg_180522

			//printk(KERN_WARNING
			//	"rt_can_fd: rt_can_write error4a %d %d %d %d %d : %d %d %d\n",
			//	num, p->tx_msg_count, p->tx_msg_error,
			//	set_parameter[num][10], p->tx_msg_read_idx,
			//	set_parameter[num][12],
			//	set_parameter[num][13],
			//	set_parameter[num][14]); //kjg_2020_1213

			p->tx_msg_error = 0;
			p->tx_msg_count = 0;
			p->tx_msg_read_idx = p->tx_msg_write_idx;
		}
	}

	idx = p->tx_msg_write_idx;
	idx++;
	if(idx >= RT_CAN_FD_TX_MSG_BUF_SIZE) idx = 0;

	memcpy((char *)&p->tx_buf[idx], msg, sizeof(struct rt_can_fd_msg));

	if(p->flag == RT_CAN_FD_CLASSIC_FLAG) {
		p->tx_buf[idx].can_fd_flag = 0;
	} else { //RT_CAN_FD_NORMAL_FLAG
		p->tx_buf[idx].can_fd_flag = 1;
	}

	p->tx_msg_write_idx = idx;
	p->tx_msg_count++;

	return 0;
}
/*kjg_180521
int rt_can_write(int num, const char *msg)
{
	unsigned char tmp, mask;
	int idx, tmp2;
	unsigned int base;
	//kjg_180405 long state;
	struct rt_can_fd_struct *p;

	if(max_can_fd_ch == 0) return 0; //kjg_180405

    if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING "rt_can_fd: rt_can_write error1 %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

	if(p->used <= 0) {
		printk(KERN_WARNING "rt_can_fd: rt_can_write error2 %d, %d\n",
			num, p->used);
		return -2;
	}

	//kjg_180405 rt_can_fd_irq_off(state);
	rtl_hard_disable_irq(p->irq); //kjg_180405

	base = RT_CAN_FD_BASE_ADDR;
	tmp = 0x01;
	tmp = tmp << num;
	outb(tmp, base + RT_CAN_FD_PORT_SELECT);

	mask = inb(base + RT_CAN_FD_PORT_IRQ_MASK);
//	if((mask & tmp) == 0) {
//		rt_can_fd_irq_on(state);
//		printk(KERN_WARNING "rt_can_fd: rt_can_write error3 %d, %x, %x\n",
//			num, mask, tmp);
//		return -3;
//	}

	tmp = 0;
	if(p->tx_msg_count >= RT_CAN_FD_TX_MSG_BUF_SIZE) {
		p->tx_msg_error++;
		if(p->tx_msg_error < 10) {
			printk(KERN_WARNING
				"rt_can_fd: rt_can_write error4 %d, %d, %d\n", num,
				p->tx_msg_count, p->tx_msg_error);
			return -4;
		} else {
			tmp = 1;
		}
	}

	if(tmp == 1) {
		p->tx_msg_error = 0;
		p->tx_msg_count = 0;
		p->tx_msg_read_idx = p->tx_msg_write_idx;
	}

	idx = p->tx_msg_write_idx;
	idx++;
	if(idx >= RT_CAN_FD_TX_MSG_BUF_SIZE) idx = 0;

	memcpy((char *)&p->tx_buf[idx], msg, sizeof(struct rt_can_fd_msg));

	if(p->flag == RT_CAN_FD_CLASSIC_FLAG) {
		p->tx_buf[idx].can_fd_flag = 0;
	} else { //RT_CAN_FD_NORMAL_FLAG
		p->tx_buf[idx].can_fd_flag = 1;
	}

	p->tx_msg_write_idx = idx;
	p->tx_msg_count++;

	if(p->tx_msg_count > 0) {
		if(p->tx_msg_write_idx == p->tx_msg_read_idx) {
			printk(KERN_WARNING
				"rt_can_fd: rt_can_write error5 %d, %d, %d\n",
				num, p->tx_msg_write_idx, p->tx_msg_read_idx);
			return -5;
		} else {
			tmp2 = fd_chip_write(num, p);
		}
	}

    //kjg_180405 rt_can_fd_irq_on(state);
	rtl_hard_enable_irq(p->irq); //kjg_180405

	return 0;
}*/

int fd_chip_write(int num, struct rt_can_fd_struct *p) 
{
	char error;
	unsigned char attempts = MAX_TXQUEUE_ATTEMPTS;
	int idx, delay;
  
	p->tx_msg_read_idx++;
	if(p->tx_msg_read_idx >= RT_CAN_FD_TX_MSG_BUF_SIZE)
		p->tx_msg_read_idx = 0;
	idx = p->tx_msg_read_idx;
	p->tx_msg_count--;

	//Initialize ID and Control bits
	p->txObj.word[0] = 0;
	p->txObj.word[1] = 0;

	/*kjg_181110 if(p->type == 0) {
		p->txObj.bF.id.SID = p->tx_buf[idx].id; //Standard or Base ID
		p->txObj.bF.id.EID = 0; //Extended ID
    	p->txObj.bF.ctrl.IDE = 0; //Standard frame
	} else {
		p->txObj.bF.id.SID = p->tx_buf[idx].id; //Standard or Base ID
		p->txObj.bF.id.EID = p->tx_buf[idx].id; //Extended ID
    	p->txObj.bF.ctrl.IDE = 1; //Extended frame
	}*/
	//if(p->tx_buf[idx].id < 0x800) {
	if(p->type == 0) {
		p->txObj.bF.id.SID = p->tx_buf[idx].id; //Standard or Base ID
		p->txObj.bF.id.EID = 0; //Extended ID
    	p->txObj.bF.ctrl.IDE = 0; //Standard frame
	} else {
		p->txObj.bF.id.SID = (p->tx_buf[idx].id >> 18) & 0x7FF;
		p->txObj.bF.id.EID = p->tx_buf[idx].id & 0x3FFFF;
    	p->txObj.bF.ctrl.IDE = 1; //Extended frame
	}
	//printk(KERN_WARNING
	//	"rt_can_fd: kjgd2 %x %x %x\n", p->tx_buf[idx].id, p->txObj.bF.id.SID, p->txObj.bF.id.EID);

	if(p->tx_buf[idx].can_fd_flag == 0) {
		p->txObj.bF.ctrl.FDF = 0;
		p->txObj.bF.ctrl.BRS = 0;
	} else {
		p->txObj.bF.ctrl.FDF = 1; //CAN FD frame
		p->txObj.bF.ctrl.BRS = 1; //Switch bit rate
	}
    p->txObj.bF.ctrl.DLC = CHIP_DataBytesToDlc(p->tx_buf[idx].length);
	if(p->txObj.bF.ctrl.DLC == 0) {
    	p->txObj.bF.ctrl.RTR = 1; //A remote frame request
	} else {
    	p->txObj.bF.ctrl.RTR = 0; //Not a remote frame request
	}
    //Sequence: doesn't get transmitted, but will be stored in TEF
    //kjg_180405 p->txObj.bF.ctrl.SEQ = 1; //kjgw_2020_0323

    do { //Check if FIFO is not full
        CHIP_TransmitChannelEventGet(num, CHIP_FIFO_CH2, &p->txFlags);

        if(attempts == 0) {
			for(delay=0; delay < DELAY_10US; delay++) {}
            CHIP_ErrorCountStateGet(num, &p->tec, &p->rec, &p->errorFlags);
//kjg_180405
//			printk(KERN_WARNING
//				"rt_can_fd: fd_chip_write error3 %d, %x, %x, %x, %d, %d\n",
//				num, p->tec, p->rec, p->errorFlags, p->txObj.bF.ctrl.DLC,
//				p->tx_buf[idx].length);
            return -1;
        }

        attempts--;
    } while(!(p->txFlags & CHIP_TX_FIFO_NOT_FULL_EVENT));

    //Load message and transmit
    error = CHIP_TransmitChannelLoad(num, CHIP_FIFO_CH2, true, p, idx);
    
    //Message Sent

	return 0;
}

int rt_can_read(int num, char *ptr)
{
	int idx;
	struct rt_can_fd_struct *p;

    if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING "rt_can_fd: rt_can_read error1 %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

	if(p->used <= 0) {
		printk(KERN_WARNING "rt_can_fd: rt_can_read error2 %d, %d\n",
			num, p->used);
		return -2;
	}
	if(p->rx_msg_count == 0) {
		//kjg_180405 printk(KERN_WARNING "rt_can_fd: rt_can_read error3a %d, %d\n",
		//	num, p->rx_msg_count);
		return -3;
	} else if(p->rx_msg_count < 0) { //kjg_181110
		p->rx_msg_count = 0;
		printk(KERN_WARNING "rt_can_fd: rt_can_read error3b %d, %d\n",
			num, p->rx_msg_count);
		return -3;
	}

	if(p->rx_msg_write_idx == p->rx_msg_read_idx) {
		//kjg_180522 printk(KERN_WARNING "rt_can_fd: rt_can_read error4 %d, %d, %d\n",
		//	num, p->rx_msg_write_idx, p->rx_msg_read_idx);
		p->rx_msg_count = 0; //kjg_181110
		return -4;
	}

	p->rx_msg_read_idx++;
	if(p->rx_msg_read_idx >= RT_CAN_FD_RX_MSG_BUF_SIZE) p->rx_msg_read_idx = 0;

	idx = p->rx_msg_read_idx;
	memcpy(ptr, (char *)&p->rx_buf[idx], sizeof(struct rt_can_fd_msg));
	p->rx_msg_count--;

    return p->rx_msg_count;
}
/*kjg_180522
int rt_can_read(int num, char *ptr)
{
	unsigned char tmp, mask;
	int idx;
	unsigned int base;
	//kjg_180405 long state;
	struct rt_can_fd_struct *p;

    if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING "rt_can_fd: rt_can_read error1 %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

	if(p->used <= 0) {
		printk(KERN_WARNING "rt_can_fd: rt_can_read error2 %d, %d\n",
			num, p->used);
		return -2;
	}
	if(p->rx_msg_count <= 0) {
		//kjg_180405 printk(KERN_WARNING "rt_can_fd: rt_can_read error3 %d, %d\n",
		//	num, p->rx_msg_count);
		return -3;
	}

	if(p->rx_msg_write_idx == p->rx_msg_read_idx) {
		printk(KERN_WARNING "rt_can_fd: rt_can_read error4 %d, %d, %d\n",
			num, p->rx_msg_write_idx, p->rx_msg_read_idx);
		return -4;
	}

	//kjg_180405 rt_can_fd_irq_off(state);
	rtl_hard_disable_irq(p->irq); //kjg_180405

	base = RT_CAN_FD_BASE_ADDR;
	tmp = 0x01;
	tmp = tmp << num;
	outb(tmp, base + RT_CAN_FD_PORT_SELECT);

	mask = inb(base + RT_CAN_FD_PORT_IRQ_MASK);
//	if((mask & tmp) == 0) {
//		rt_can_fd_irq_on(state);
//		printk(KERN_WARNING "rt_can_fd: rt_can_read error5 %d, %x, %x\n",
//			num, mask, tmp);
//		return -5;
//	}

	p->rx_msg_read_idx++;
	if(p->rx_msg_read_idx >= RT_CAN_FD_RX_MSG_BUF_SIZE) p->rx_msg_read_idx = 0;

	idx = p->rx_msg_read_idx;
	memcpy(ptr, (char *)&p->rx_buf[idx], sizeof(struct rt_can_fd_msg));
	p->rx_msg_count--;

    //kjg_180405 rt_can_fd_irq_on(state);
	rtl_hard_enable_irq(p->irq); //kjg_180405

    return p->rx_msg_count;
}*/

//Registered interrupt handlers.
//These simply call the general interrupt handler for the current
//line to do the work.
static unsigned int rt_can_fd_share_isr(unsigned int irq_no, struct pt_regs *reg)
{
	unsigned char data, tmp;
	int i, j;

	j = 0;
	while((data = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_PORT_IRQ)) != 0) {
		tmp = 0x01;
		//kjg_180405 for(i=0; i < RT_CAN_FD_CNT; i++) {
		for(i=0; i < max_can_fd_ch; i++) {
			if((data & tmp) != 0x00) {
				rt_can_fd_isr((unsigned int)i, NULL);
			}
			tmp = tmp << 1;
		}

		j++;
		//kjg_180524 if(j >= 32) break; //kjg_180405 16->32
		//kjg_181120 if(j >= 300) break; //kjg_180405 16->32
		if(j >= 16) { //kjg_181120
			printk(KERN_WARNING "rt_can_fd: fd_isr loop %d %d\n",
				j, data);
			break;
		}
	}

//kjg_180524
#ifdef __SBC_EM104_A5362__
    rtl_hard_enable_irq(5);
#else
    rtl_hard_enable_irq(9);
#endif

	return 0;
}

//Real interrupt handler.
//Called by the registered ISRs to do the actual work.
//param can Port to use corresponding to internal numbering scheme.
unsigned int rt_can_fd_isr(unsigned int num, struct pt_regs *reg)
{
	char error;
    unsigned char port_sel, tmp;
    unsigned int base, count;
    struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	base = RT_CAN_FD_BASE_ADDR;
	port_sel = inb(base + RT_CAN_FD_PORT_SELECT);

	tmp = 0x01;
	tmp = tmp << num;
	outb(tmp, base + RT_CAN_FD_PORT_SELECT);

	count = 0;
//kjg_180524	while(1) { //Check if FIFO is not empty
		error = CHIP_ReceiveChannelEventGet(num, CHIP_FIFO_CH1, &p->rxFlags);

		if(p->rxFlags & CHIP_RX_FIFO_NOT_EMPTY_EVENT) {
			CHIP_ReceiveMessageGet(num, CHIP_FIFO_CH1, &p->rxObj);

			//kjg_180522
//			count++;
//			if(count >= 3) break;
//		} else break;
	}

	outb(port_sel, base + RT_CAN_FD_PORT_SELECT);

    //kjg_180524 rtl_hard_enable_irq(p->irq);
    return 0;
}

unsigned int rt_can_fd_rx(void)
{
	unsigned char data, tmp;
	int i, j;

	j = 0;
	while((data = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_PORT_IRQ)) != 0) {
		tmp = 0x01;
		for(i=0; i < max_can_fd_ch; i++) {
			if((data & tmp) != 0x00) {
				rt_can_fd_rx2((unsigned int)i);
			}
			tmp = tmp << 1;
		}

		j++;
		//kjg_180524 if(j >= 32) break; //kjg_180405 16->32
		//kjg_181120 if(j >= 300) break; //kjg_180405 16->32
		if(j >= 16) { //kjg_181120
			printk(KERN_WARNING "rt_can_fd: fd_isr loop %d %d\n",
				j, data);
			break;
		}
	}

//kjg_180524
#ifdef __SBC_EM104_A5362__
    //rtl_hard_enable_irq(5);
#else
    //rtl_hard_enable_irq(9);
#endif

	return 0;
}

void rt_can_fd_rx2(unsigned int can_ch)
{
	char error;
    unsigned char port_sel, tmp;
    unsigned int base, i, module_ch, master_slave;
    struct rt_can_fd_struct *p;

	//kjg_190418_s
	if(myData->AppControl.misc.Load_Process_CAN[0] != P1) return;
	if(can_ch >= myData->mData.config.installedCAN) return;
	i = myData->mData.config.division_CAN;
	if(i == 0) {
		switch(myData->AppControl.config.systemModel) {
			case C_SKI_120V_400A_100A_192KW:
			case C_SKI_120V_400A_100A_192KW_2:
			case C_SKI_120V_400A_100A_192KW_3:
			case C_SKI_120V_400A_100A_192KW_4:
			case C_SKI_120V_400A_100A_192KW_5:
			case C_SKI_120V_400A_100A_192KW_6:
			case C_SKI_120V_400A_100A_192KW_7:
			case C_SKI_120V_400A_100A_192KW_8:
			case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
				module_ch = can_ch / 2;
				if(module_ch == 1) module_ch = 2;
				master_slave = can_ch % 2;
				break;
			case C_SK_450V_200A_10A_360KW:
				module_ch = can_ch / 2;
				if(module_ch == 1) module_ch = 3;
				master_slave = can_ch % 2;
				break;
			default:
				module_ch = can_ch / 2;
				master_slave = can_ch % 2;
				break;
			}
		if(myData->canReceiveDataCount[module_ch][master_slave] == 0) return;
	} else { //division_CAN == 1
		module_ch = can_ch;
		master_slave = 0;
		if(myData->canReceiveDataCount[module_ch][master_slave] == 0) return;
	} //kjg_190418_e

	p = &(rt_can_fd_table[can_ch]);

	base = RT_CAN_FD_BASE_ADDR;
	port_sel = inb(base + RT_CAN_FD_PORT_SELECT);

	tmp = 0x01;
	tmp = tmp << can_ch;
	outb(tmp, base + RT_CAN_FD_PORT_SELECT);

	for(i=0; i < 3; i++) {
		//Check if FIFO is not empty
		error = CHIP_ReceiveChannelEventGet(can_ch, CHIP_FIFO_CH1, &p->rxFlags);

		if(p->rxFlags & CHIP_RX_FIFO_NOT_EMPTY_EVENT) {
			CHIP_ReceiveMessageGet(can_ch, CHIP_FIFO_CH1, &p->rxObj);
			break;
		}
	}

	outb(port_sel, base + RT_CAN_FD_PORT_SELECT);

    //kjg_180524 rtl_hard_enable_irq(p->irq);
}

int rt_can_fd_rx3(unsigned int can_ch)
{ //kjg_2020_1212
	char error;
    unsigned int i, module_ch, master_slave;
    struct rt_can_fd_struct *p;
	long long hrt[4];

	if(myData->AppControl.misc.Load_Process_CAN[0] != P1) return 0;
	if(can_ch >= myData->mData.config.installedCAN) return 0;
	i = myData->mData.config.division_CAN;
	if(i == 0) {
		switch(myData->AppControl.config.systemModel) {
			case C_SKI_120V_400A_100A_192KW:
			case C_SKI_120V_400A_100A_192KW_2:
			case C_SKI_120V_400A_100A_192KW_3:
			case C_SKI_120V_400A_100A_192KW_4:
			case C_SKI_120V_400A_100A_192KW_5:
			case C_SKI_120V_400A_100A_192KW_6:
			case C_SKI_120V_400A_100A_192KW_7:
			case C_SKI_120V_400A_100A_192KW_8:
			case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
				module_ch = can_ch / 2;
				if(module_ch == 1) module_ch = 2;
				master_slave = can_ch % 2;
				break;
			case C_SK_450V_200A_10A_360KW:
				module_ch = can_ch / 2;
				if(module_ch == 1) module_ch = 3;
				master_slave = can_ch % 2;
				break;
			default:
				module_ch = can_ch / 2;
				master_slave = can_ch % 2;
				break;
			}
		if(myData->canReceiveDataCount[module_ch][master_slave] == 0) return 0;
	} else { //division_CAN == 1
		module_ch = can_ch;
		master_slave = 0;
		if(myData->canReceiveDataCount[module_ch][master_slave] == 0) return 0;
	}

	if(set_parameter[can_ch][10] == 2) return (-1);

	p = &(rt_can_fd_table[can_ch]);

	hrt[0] = gethrtime();

	//Check if FIFO is not empty
	error = CHIP_ReceiveChannelEventGet(can_ch, CHIP_FIFO_CH1, &p->rxFlags);
	hrt[1] = gethrtime();

	if(p->rxFlags & CHIP_RX_FIFO_NOT_EMPTY_EVENT) {
		CHIP_ReceiveMessageGet(can_ch, CHIP_FIFO_CH1, &p->rxObj);

//		hrt[2] = gethrtime();
//		myData->test_val_l[can_ch] = hrt[1] - hrt[0];
//		myData->test_val_l[can_ch+8] = hrt[2] - hrt[0];


		return 100;
	}

	return 0;
}

int rt_can_fd_rx4(unsigned int can_ch)
{ //kjg_2020_1213
	char error;
	unsigned short crcFromSpiSlave = 0, crcAtController = 0, tx_length, nBytes;
    unsigned int i, module_ch, master_slave, idx;
    struct rt_can_fd_struct *p;
	//long long hrt[4];

	T_CHIP_reg myReg;
	struct rt_can_fd_msg tmp_msg;

	p = &(rt_can_fd_table[can_ch]);

	if(p->status != 0) return 0; //kjg_2020_1213

	if(myData->AppControl.misc.Load_Process_CAN[0] != P1) {
		set_parameter[can_ch][12] = 0;
		return 0;
	}

	if(can_ch >= myData->mData.config.installedCAN) {
		set_parameter[can_ch][12] = 0;
		return 0;
	}

	i = myData->mData.config.division_CAN;
	if(i == 0) {
		switch(myData->AppControl.config.systemModel) {
			case C_SKI_120V_400A_100A_192KW:
			case C_SKI_120V_400A_100A_192KW_2:
			case C_SKI_120V_400A_100A_192KW_3:
			case C_SKI_120V_400A_100A_192KW_4:
			case C_SKI_120V_400A_100A_192KW_5:
			case C_SKI_120V_400A_100A_192KW_6:
			case C_SKI_120V_400A_100A_192KW_7:
			case C_SKI_120V_400A_100A_192KW_8:
			case C_LGC_500V_250A_125A_50A_500KW: //jhk_181109
				module_ch = can_ch / 2;
				if(module_ch == 1) module_ch = 2;
				master_slave = can_ch % 2;
				break;
			case C_SK_450V_200A_10A_360KW:
				module_ch = can_ch / 2;
				if(module_ch == 1) module_ch = 3;
				master_slave = can_ch % 2;
				break;
			default:
				module_ch = can_ch / 2;
				master_slave = can_ch % 2;
				break;
			}
		if(myData->canReceiveDataCount[module_ch][master_slave] == 0) {
			set_parameter[can_ch][12] = 0;
			return 0;
		}
	} else { //division_CAN == 1
		module_ch = can_ch;
		master_slave = 0;
		if(myData->canReceiveDataCount[module_ch][master_slave] == 0) {
			set_parameter[can_ch][12] = 0;
			return 0;
		}
	}

	if(set_parameter[can_ch][10] == 2) return (-1);

	//hrt[0] = gethrtime();

	switch(set_parameter[can_ch][12]) {
		case 0:
			//Check if FIFO is not empty
			error = CHIP_ReceiveChannelEventGet(can_ch, CHIP_FIFO_CH1,
				&p->rxFlags);
			//hrt[1] = gethrtime();

			if(p->rxFlags & CHIP_RX_FIFO_NOT_EMPTY_EVENT) {
				error = CHIP_ReceiveMessageGet_4(can_ch, CHIP_FIFO_CH1,
					&p->rxObj);
				if(error == 0) {
					return 100;
				} else if(error == 123) {
					set_parameter[can_ch][12] = 1;

					//hrt[2] = gethrtime();
					//myData->test_val_ll2[0][can_ch] = hrt[1] - hrt[0];
					//myData->test_val_ll2[0][can_ch + 8] = hrt[2] - hrt[0];
				}
			}
			break;
		case 1:
			error = SPI_Transfer_4b(can_ch, p->spi_rx_buf,
				set_parameter[can_ch][13]);
			if(error != 0) return 0;

			//hrt[1] = gethrtime();

			set_parameter[can_ch][12] = 0;

			tx_length = set_parameter[can_ch][13];
			nBytes = tx_length - 5;

			//Get CRC from controller
			crcFromSpiSlave = (unsigned short)(p->spi_rx_buf[tx_length - 2]
				<< 8) + (unsigned short)(p->spi_rx_buf[tx_length - 1]);

			//User the receive buffer to calculate CRC
			//First three bytes need to be command
			p->spi_rx_buf[0] = p->spi_tx_buf[0];
			p->spi_rx_buf[1] = p->spi_tx_buf[1];
			p->spi_rx_buf[2] = p->spi_tx_buf[2];
			crcAtController = CHIP_CalculateCRC16(p->spi_rx_buf, nBytes + 3);

			//Compare CRC readings
			if(crcFromSpiSlave == crcAtController) { //kjgw_2020_0322
				//Update data
				for(i=0; i < nBytes; i++) {
					ba_rxd[can_ch][i] = p->spi_rx_buf[i + 3];
				}
			} else {
				printk(KERN_WARNING
					"rt_can_fd: CHIP_ReceiveMessageGet error4a %d\n", can_ch);
				return 0;
			}

			//Assign message header
			myReg.byte[0] = ba_rxd[can_ch][0];
			myReg.byte[1] = ba_rxd[can_ch][1];
			myReg.byte[2] = ba_rxd[can_ch][2];
			myReg.byte[3] = ba_rxd[can_ch][3];
			p->rxObj.word[0] = myReg.word;

			myReg.byte[0] = ba_rxd[can_ch][4];
			myReg.byte[1] = ba_rxd[can_ch][5];
			myReg.byte[2] = ba_rxd[can_ch][6];
			myReg.byte[3] = ba_rxd[can_ch][7];
			p->rxObj.word[1] = myReg.word;

			if(p->rxObj.bF.ctrl.RTR) tmp_msg.type = MSGTYPE_RTR;
			else tmp_msg.type = MSGTYPE_STANDARD;
			if(p->rxObj.bF.ctrl.IDE == 0) { //Standard frame
				tmp_msg.id = (unsigned int)p->rxObj.bF.id.SID;
			} else { //Extended frame
				tmp_msg.type |= MSGTYPE_EXTENDED;
				tmp_msg.id = (unsigned int)p->rxObj.bF.id.EID;
				tmp_msg.id |= ((unsigned int)p->rxObj.bF.id.SID << 18);
			}

			if(p->rxObj.bF.ctrl.FDF == 0) {
				tmp_msg.can_fd_flag = 0;
			} else { //CAN FD frame
				tmp_msg.can_fd_flag = 1;
			}

			tmp_msg.length = (unsigned char)CHIP_DlcToDataBytes(p->rxObj
				.bF.ctrl.DLC);
			if(set_parameter[can_ch][14]) {
				myReg.byte[0] = ba_rxd[can_ch][8];
				myReg.byte[1] = ba_rxd[can_ch][9];
				myReg.byte[2] = ba_rxd[can_ch][10];
				myReg.byte[3] = ba_rxd[can_ch][11];
				p->rxObj.word[2] = myReg.word;

				//Assign message data
				for(i=0; i < tmp_msg.length; i++) {
					tmp_msg.data[i] = ba_rxd[can_ch][i + 12];
				}
			} else {
				p->rxObj.word[2] = 0;

				//Assign message data
				for(i=0; i < tmp_msg.length; i++) {
					tmp_msg.data[i] = ba_rxd[can_ch][i + 8];
				}
			}

			//UINC channel
			error = CHIP_ReceiveChannelUpdate(can_ch, CHIP_FIFO_CH1);
			if(error) {
				printk(KERN_WARNING
					"rt_can_fd: CHIP_ReceiveMessageGet error5a %d, %d\n",
					can_ch, error);
				return 0;
			}

			tmp_msg.time = gethrtime();

			//kjg_2020_1213
			for(i=0; i < 100; i++) {
				if(rx_can_id[can_ch][i] == tmp_msg.id) {
					//if((tmp_msg.time - rx_can_time[can_ch][i]) < 25000000) {
					//if((tmp_msg.time - rx_can_time[can_ch][i]) < 55000000) {
					if((tmp_msg.time - rx_can_time[can_ch][i]) < 75000000) {
						return 0;
					}
				}
			}
			if(i == 100) {
				for(i=0; i < 100; i++) {
					if(rx_can_id[can_ch][i] == 0) {
						rx_can_id[can_ch][i] = tmp_msg.id;
						rx_can_time[can_ch][i] = tmp_msg.time;
						break;
					}
				}
			}

			//step forward in fifo
			if(p->rx_msg_count >= RT_CAN_FD_RX_MSG_BUF_SIZE) {
				printk(KERN_WARNING
					"rt_can_fd: CHIP_ReceiveMessageGet error6a %d, %d\n",
					can_ch, p->rx_msg_count);
				return 0;
			}

			idx = p->rx_msg_write_idx;
			idx++;
			if(idx >= RT_CAN_FD_RX_MSG_BUF_SIZE) idx = 0;

			p->rx_buf[idx].id = tmp_msg.id;
			p->rx_buf[idx].type = tmp_msg.type;
			p->rx_buf[idx].length = tmp_msg.length;
			for(i=0; i < tmp_msg.length; i++) {
				p->rx_buf[idx].data[i] = tmp_msg.data[i];
			}
			p->rx_buf[idx].time = tmp_msg.time;
			p->rx_buf[idx].timeStamp = tmp_msg.timeStamp;

			p->rx_msg_write_idx = idx;
			p->rx_msg_count++;

			//hrt[2] = gethrtime();
			//myData->test_val_ll2[1][can_ch] = hrt[1] - hrt[0];
			//myData->test_val_ll2[1][can_ch + 8] = hrt[2] - hrt[0];

			return 100;
			break;
		default:
			break;
	}

	return 0;
}

int CAN_FD_A_Initialize(int num, int baud, int sjw, int ext, int fd_flag, int data_rate, int crc)
{ //kjg_180405
	int i, j, can_ch, can_type;
	long filter_id, mask_id; //kjg_191124
	struct rt_can_fd_struct *p;
	//long long hrt[20];
	//hrt[0] = gethrtime();

	p = &(rt_can_fd_table[num]);
	p->rxFlags = 0;
	
	CHIP_Reset(num);
	//kjg_2020_1212 8.1us
	//hrt[1] = gethrtime();

	//Enable ECC and initialize RAM
	CHIP_EccEnable(num);
	//kjg_2020_1212 26.9us
	//hrt[2] = gethrtime();
	CHIP_RamInit(num, 0xFF); //kjg_2020_0325
	//kjg_2020_1212 4873.7us
	//hrt[3] = gethrtime();

	//Configure device
	CHIP_ConfigureObjectReset(num);
	//kjg_2020_1212 1.3us
	//hrt[4] = gethrtime();
	if(crc == 0) p->config.IsoCrcEnable = 0;
	else p->config.IsoCrcEnable = 1; //ISO_CRC
	p->config.StoreInTEF = 0;
	CHIP_Configure(num);
	//kjg_2020_1212 18.6us
	//hrt[5] = gethrtime();

	//Setup TX FIFO
	CHIP_TransmitChannelConfigureObjectReset(num);
	//kjg_2020_1212 1.0us
	//hrt[6] = gethrtime();
	p->txConfig.FifoSize = 7;
	p->txConfig.PayLoadSize = CHIP_PLSIZE_64;
	p->txConfig.TxPriority = 1;
	CHIP_TransmitChannelConfigure(num, CHIP_FIFO_CH2);
	//kjg_2020_1212 17.5us
	//hrt[7] = gethrtime();

	//Setup RX FIFO
	CHIP_ReceiveChannelConfigureObjectReset(num);
	//kjg_2020_1212 1.2us
	//hrt[8] = gethrtime();
	p->rxConfig.FifoSize = 15; //kjg_191124 15->20, after 20->15
	p->rxConfig.PayLoadSize = CHIP_PLSIZE_64;
	CHIP_ReceiveChannelConfigure(num, CHIP_FIFO_CH1);
	//kjg_2020_1212 17.4us
	//hrt[9] = gethrtime();
/*kjg_191124
	//Setup RX Filter
	p->fObj.word = 0;
	p->fObj.bF.SID = 0x00;
	p->fObj.bF.SID11 = 0; //kjg_181031
	//kjg_181031 p->fObj.bF.EXIDE = 0; //Only execpt Standard frames
	p->fObj.bF.EXIDE = 1;
	p->fObj.bF.EID = 0x00;
	CHIP_FilterObjectConfigure(num, CHIP_FILT0);

	//Setup RX Mask
	p->mObj.word = 0;
	p->mObj.bF.MSID = 0x00;
	p->mObj.bF.MSID11 = 0; //kjg_181031
	//kjg_181031 if(ext == 0) {
	//	p->mObj.bF.MIDE = 1; //Only allow standard frame
	//} else {
		p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
	//}
	p->mObj.bF.MEID = 0x00;
	CHIP_FilterMaskConfigure(num, CHIP_FILT0);

	//Link FIFO and Filter
	CHIP_FilterToFifoLink(num, CHIP_FILT0, CHIP_FIFO_CH1, true);*/

	/*kjg_191124_1
	//filter_id = 0x1E010000;
	filter_id = 0x1EF00000;
	mask_id = 0x1FFF0000;
	//Setup RX Filter
	p->fObj.word = 0;
	p->fObj.bF.SID = (filter_id >> 18) & 0x7FF;
	p->fObj.bF.SID11 = 0; //kjg_181031
	//kjg_181031 p->fObj.bF.EXIDE = 0; //Only execpt Standard frames
	p->fObj.bF.EXIDE = 1;
	p->fObj.bF.EID = filter_id & 0x3FFFF;
	CHIP_FilterObjectConfigure(num, CHIP_FILT0);

	//Setup RX Mask
	p->mObj.word = 0;
	p->mObj.bF.MSID = (mask_id >> 18) & 0x7FF;
	p->mObj.bF.MSID11 = 0; //kjg_181031
	//kjg_181031 if(ext == 0) {
	//	p->mObj.bF.MIDE = 1; //Only allow standard frame
	//} else {
		p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
	//}
	p->mObj.bF.MEID = mask_id & 0x3FFFF;
	CHIP_FilterMaskConfigure(num, CHIP_FILT0);

	//Link FIFO and Filter
	CHIP_FilterToFifoLink(num, CHIP_FILT0, CHIP_FIFO_CH1, true);*/

	/*kjg_191124_2
	//filter_id = 0x1E010000;
	//filter_id = 0x1EF00000;
	//mask_id = 0x1FFF0000;
	filter_id = myData->canReceiveSetData.commonData[num][0].filter[0];
	mask_id = myData->canReceiveSetData.commonData[num][0].mask[0];
	printk(KERN_WARNING "kjgd_191124 %d, %lx, %lx\n", num,
		//myData->canReceiveSetData.commonData[num][0].filter[0],
		//myData->canReceiveSetData.commonData[num][0].mask[0]);
		filter_id, mask_id);
	if(ext == 0) {
		//Setup RX Filter
		p->fObj.word = 0;
		p->fObj.bF.SID = filter_id;
		p->fObj.bF.SID11 = 0; //kjg_181031
		//kjg_181031 p->fObj.bF.EXIDE = 0; //Only execpt Standard frames
		p->fObj.bF.EXIDE = 1;
		p->fObj.bF.EID = 0;
		CHIP_FilterObjectConfigure(num, CHIP_FILT0);

		//Setup RX Mask
		p->mObj.word = 0;
		p->mObj.bF.MSID = mask_id;
		p->mObj.bF.MSID11 = 0; //kjg_181031
		//kjg_181031 if(ext == 0) {
		//	p->mObj.bF.MIDE = 1; //Only allow standard frame
		//} else {
			p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
		//}
		p->mObj.bF.MEID = 0x00;
		CHIP_FilterMaskConfigure(num, CHIP_FILT0);
	} else {
		//Setup RX Filter
		p->fObj.word = 0;
		p->fObj.bF.SID = (filter_id >> 18) & 0x7FF;
		p->fObj.bF.SID11 = 0; //kjg_181031
		//kjg_181031 p->fObj.bF.EXIDE = 0; //Only execpt Standard frames
		p->fObj.bF.EXIDE = 1;
		p->fObj.bF.EID = filter_id & 0x3FFFF;
		CHIP_FilterObjectConfigure(num, CHIP_FILT0);

		//Setup RX Mask
		p->mObj.word = 0;
		p->mObj.bF.MSID = (mask_id >> 18) & 0x7FF;
		p->mObj.bF.MSID11 = 0; //kjg_181031
		//kjg_181031 if(ext == 0) {
		//	p->mObj.bF.MIDE = 1; //Only allow standard frame
		//} else {
			p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
		//}
		p->mObj.bF.MEID = mask_id & 0x3FFFF;
		CHIP_FilterMaskConfigure(num, CHIP_FILT0);
	}

	//Link FIFO and Filter
	CHIP_FilterToFifoLink(num, CHIP_FILT0, CHIP_FIFO_CH1, true);*/

	//kjg_191124_3a
	j = 0;
	for(i=0; i < 2; i++) {
	//for(i=0; i < 1; i++) {
		switch(myData->mData.config.division_CAN) { //ktg_230612s
			case 1:
				can_ch = num;
				can_type = 0;
				break;
			default:
				switch(myData->AppControl.config.systemModel) { //kjhw_170629s
					case C_SKI_120V_400A_100A_192KW:		//shh_230612s
					case C_SKI_120V_400A_100A_192KW_2:
					case C_SKI_120V_400A_100A_192KW_3:
					case C_SKI_120V_400A_100A_192KW_4:
					case C_SKI_120V_400A_100A_192KW_5:
					case C_SKI_120V_400A_100A_192KW_6:
					case C_SKI_120V_400A_100A_192KW_7:
					case C_SKI_120V_400A_100A_192KW_8:
					case C_LGC_500V_250A_125A_50A_500KW: 	
					case C_SKI_500V_450A_200A_450KW:	
					case C_SKI_500V_450A_200A_450KW_2:	
					case C_SKI_500V_450A_200A_450KW_3:		
					case C_SKI_500V_450A_200A_450KW_4:	
					case C_SKI_500V_450A_200A_450KW_5:		
					case C_SKI_500V_450A_200A_450KW_6:		//shh_230612e
						can_ch = num / 2;
						if(can_ch == 1) can_ch = 2;
						can_type = num % 2;
						break;
					default:
						can_ch = num / 2;
						can_type = num % 2;
						break;
				}
				break;
		}	//ktg_230612e
		//can_ch = num / 2;		//ktg_230612
		//can_type = num % 2;	//ktg_230612
		filter_id = myData->canReceiveSetData.commonData[can_ch][can_type]
			.filter[i];
		if(filter_id == 0) {
			//Link FIFO and Filter
			if(i == 0) CHIP_FilterToFifoLink(num, CHIP_FILT0, CHIP_FIFO_CH1,
				false);
			else CHIP_FilterToFifoLink(num, CHIP_FILT1, CHIP_FIFO_CH1, false);
			continue;
		}
		j++;
		mask_id = myData->canReceiveSetData.commonData[can_ch][can_type]
			.mask[0];
		printk(KERN_WARNING "kjgd_191124a ch:%d, mask:%lx, filter:%lx\n", num,
			mask_id, filter_id);

		if(ext == 0) {
			//Setup RX Filter
			p->fObj.word = 0;
			p->fObj.bF.SID = filter_id;
			p->fObj.bF.SID11 = 0; //kjg_181031
			//p->fObj.bF.EXIDE = 0; //Only execpt Standard frames
			p->fObj.bF.EXIDE = 1; //Both Standard and Extended frames will be accepted	//ktg_201221
			p->fObj.bF.EID = 0;
			if(i == 0) CHIP_FilterObjectConfigure(num, CHIP_FILT0);
			else CHIP_FilterObjectConfigure(num, CHIP_FILT1);

			//Setup RX Mask
			p->mObj.word = 0;
			p->mObj.bF.MSID = mask_id;
			p->mObj.bF.MSID11 = 0; //kjg_181031
			//p->mObj.bF.MIDE = 1; //Only allow standard frame
			p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted	//ktg_201221
			p->mObj.bF.MEID = 0x00;
			if(i == 0) CHIP_FilterMaskConfigure(num, CHIP_FILT0);
			else CHIP_FilterMaskConfigure(num, CHIP_FILT1);
		} else {
			//Setup RX Filter
			p->fObj.word = 0;
			p->fObj.bF.SID = (filter_id >> 18) & 0x7FF;
			p->fObj.bF.SID11 = 0; //kjg_181031
			p->fObj.bF.EXIDE = 1;
			p->fObj.bF.EID = filter_id & 0x3FFFF;
			if(i == 0) CHIP_FilterObjectConfigure(num, CHIP_FILT0);
			else CHIP_FilterObjectConfigure(num, CHIP_FILT1);

			//Setup RX Mask
			p->mObj.word = 0;
			p->mObj.bF.MSID = (mask_id >> 18) & 0x7FF;
			p->mObj.bF.MSID11 = 0; //kjg_181031
			p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
			p->mObj.bF.MEID = mask_id & 0x3FFFF;
			if(i == 0) CHIP_FilterMaskConfigure(num, CHIP_FILT0);
			else CHIP_FilterMaskConfigure(num, CHIP_FILT1);
		}

		//Link FIFO and Filter
		if(i == 0) CHIP_FilterToFifoLink(num, CHIP_FILT0, CHIP_FIFO_CH1, true);
		else CHIP_FilterToFifoLink(num, CHIP_FILT1, CHIP_FIFO_CH1, true);
	}
	//kjg_2020_1212 22.3us
	//hrt[10] = gethrtime();

	//kjg_191124_3b
	for(i=0; i < 4; i++) {
		switch(myData->mData.config.division_CAN) { //ktg_230612s
			case 1:
				can_ch = num;
				can_type = 0;
				break;
			default:
				switch(myData->AppControl.config.systemModel) { //kjhw_170629s
					case C_SKI_120V_400A_100A_192KW:		//shh_230612s
					case C_SKI_120V_400A_100A_192KW_2:
					case C_SKI_120V_400A_100A_192KW_3:
					case C_SKI_120V_400A_100A_192KW_4:
					case C_SKI_120V_400A_100A_192KW_5:
					case C_SKI_120V_400A_100A_192KW_6:
					case C_SKI_120V_400A_100A_192KW_7:
					case C_SKI_120V_400A_100A_192KW_8:
					case C_LGC_500V_250A_125A_50A_500KW: 	
					case C_SKI_500V_450A_200A_450KW:	
					case C_SKI_500V_450A_200A_450KW_2:	
					case C_SKI_500V_450A_200A_450KW_3:		
					case C_SKI_500V_450A_200A_450KW_4:	
					case C_SKI_500V_450A_200A_450KW_5:		
					case C_SKI_500V_450A_200A_450KW_6:		//shh_230612e
						can_ch = num / 2;
						if(can_ch == 1) can_ch = 2;
						can_type = num % 2;
						break;
					default:
						can_ch = num / 2;
						can_type = num % 2;
						break;
				}
				break;
		}	//ktg_230612e
		//can_ch = num / 2;		//ktg_230612
		//can_type = num % 2;	//ktg_230612
		filter_id = myData->canReceiveSetData.commonData[can_ch][can_type]
			.filter[i+2];
		if(filter_id == 0) {
			//Link FIFO and Filter
			switch(i) {
				case 0: CHIP_FilterToFifoLink(num, CHIP_FILT2, CHIP_FIFO_CH1,
						false);
					break;
				case 1: CHIP_FilterToFifoLink(num, CHIP_FILT3, CHIP_FIFO_CH1,
						false);
					break;
				case 2: CHIP_FilterToFifoLink(num, CHIP_FILT4, CHIP_FIFO_CH1,
						false);
					break;
				case 3: CHIP_FilterToFifoLink(num, CHIP_FILT5, CHIP_FIFO_CH1,
						false);
					break;
				default: break;
			}
			continue;
		}
		j++;
		mask_id = myData->canReceiveSetData.commonData[can_ch][can_type]
			.mask[1];
		printk(KERN_WARNING "kjgd_191124b ch:%d, mask:%lx, filter:%lx\n", num,
			mask_id, filter_id);

		if(ext == 0) {
			//Setup RX Filter
			p->fObj.word = 0;
			p->fObj.bF.SID = filter_id;
			p->fObj.bF.SID11 = 0; //kjg_181031
			//p->fObj.bF.EXIDE = 0; //Only execpt Standard frames
			p->fObj.bF.EXIDE = 1; //Both Standard and Extended frames will be accpted	//ktg_201221
			p->fObj.bF.EID = 0;
			switch(i) {
				case 0: CHIP_FilterObjectConfigure(num, CHIP_FILT2); break;
				case 1: CHIP_FilterObjectConfigure(num, CHIP_FILT3); break;
				case 2: CHIP_FilterObjectConfigure(num, CHIP_FILT4); break;
				case 3: CHIP_FilterObjectConfigure(num, CHIP_FILT5); break;
				default: break;
			}

			//Setup RX Mask
			p->mObj.word = 0;
			p->mObj.bF.MSID = mask_id;
			p->mObj.bF.MSID11 = 0; //kjg_181031
			//p->mObj.bF.MIDE = 1; //Only allow standard frame
			p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted	//ktg_201221
			p->mObj.bF.MEID = 0x00;
			switch(i) {
				case 0: CHIP_FilterMaskConfigure(num, CHIP_FILT2); break;
				case 1: CHIP_FilterMaskConfigure(num, CHIP_FILT3); break;
				case 2: CHIP_FilterMaskConfigure(num, CHIP_FILT4); break;
				case 3: CHIP_FilterMaskConfigure(num, CHIP_FILT5); break;
				default: break;
			}
		} else {
			//Setup RX Filter
			p->fObj.word = 0;
			p->fObj.bF.SID = (filter_id >> 18) & 0x7FF;
			p->fObj.bF.SID11 = 0; //kjg_181031
			p->fObj.bF.EXIDE = 1;
			p->fObj.bF.EID = filter_id & 0x3FFFF;
			switch(i) {
				case 0: CHIP_FilterObjectConfigure(num, CHIP_FILT2); break;
				case 1: CHIP_FilterObjectConfigure(num, CHIP_FILT3); break;
				case 2: CHIP_FilterObjectConfigure(num, CHIP_FILT4); break;
				case 3: CHIP_FilterObjectConfigure(num, CHIP_FILT5); break;
				default: break;
			}

			//Setup RX Mask
			p->mObj.word = 0;
			p->mObj.bF.MSID = (mask_id >> 18) & 0x7FF;
			p->mObj.bF.MSID11 = 0; //kjg_181031
			p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
			p->mObj.bF.MEID = mask_id & 0x3FFFF;
			switch(i) {
				case 0: CHIP_FilterMaskConfigure(num, CHIP_FILT2); break;
				case 1: CHIP_FilterMaskConfigure(num, CHIP_FILT3); break;
				case 2: CHIP_FilterMaskConfigure(num, CHIP_FILT4); break;
				case 3: CHIP_FilterMaskConfigure(num, CHIP_FILT5); break;
				default: break;
			}
		}

		//Link FIFO and Filter
		switch(i) {
			case 0: CHIP_FilterToFifoLink(num, CHIP_FILT2, CHIP_FIFO_CH1, true);
				break;
			case 1: CHIP_FilterToFifoLink(num, CHIP_FILT3, CHIP_FIFO_CH1, true);
				break;
			case 2: CHIP_FilterToFifoLink(num, CHIP_FILT4, CHIP_FIFO_CH1, true);
				break;
			case 3: CHIP_FilterToFifoLink(num, CHIP_FILT5, CHIP_FIFO_CH1, true);
				break;
			default: break;
		}
	}
	//kjg_2020_1212 42.8us
	//hrt[11] = gethrtime();

	if(j == 0) {
		switch(myData->mData.config.division_CAN) { //ktg_230612s
			case 1:
				can_ch = num;
				can_type = 0;
				break;
			default:
				switch(myData->AppControl.config.systemModel) { //kjhw_170629s
					case C_SKI_120V_400A_100A_192KW:		//shh_230612s
					case C_SKI_120V_400A_100A_192KW_2:
					case C_SKI_120V_400A_100A_192KW_3:
					case C_SKI_120V_400A_100A_192KW_4:
					case C_SKI_120V_400A_100A_192KW_5:
					case C_SKI_120V_400A_100A_192KW_6:
					case C_SKI_120V_400A_100A_192KW_7:
					case C_SKI_120V_400A_100A_192KW_8:
					case C_LGC_500V_250A_125A_50A_500KW: 	
					case C_SKI_500V_450A_200A_450KW:	
					case C_SKI_500V_450A_200A_450KW_2:	
					case C_SKI_500V_450A_200A_450KW_3:		
					case C_SKI_500V_450A_200A_450KW_4:	
					case C_SKI_500V_450A_200A_450KW_5:		
					case C_SKI_500V_450A_200A_450KW_6:		//shh_230612e
						can_ch = num / 2;
						if(can_ch == 1) can_ch = 2;
						can_type = num % 2;
						break;
					default:
						can_ch = num / 2;
						can_type = num % 2;
						break;
				}
				break;
		}	//ktg_230612e
		//can_ch = num / 2;		//ktg_230612
		//can_type = num % 2;	//ktg_230612
		mask_id = 0; filter_id = 0;
		printk(KERN_WARNING "kjgd_191124c ch:%d, mask:%lx, filter:%lx\n", num,
			mask_id, filter_id);

		if(ext == 0) {
			//Setup RX Filter
			p->fObj.word = 0;
			p->fObj.bF.SID = filter_id;
			p->fObj.bF.SID11 = 0; //kjg_181031
			//p->fObj.bF.EXIDE = 0; //Only execpt Standard frames
			p->fObj.bF.EXIDE = 1; //Both Standard and Extended frames will be accepted	//ktg_201221
			p->fObj.bF.EID = 0;
			CHIP_FilterObjectConfigure(num, CHIP_FILT0);

			//Setup RX Mask
			p->mObj.word = 0;
			p->mObj.bF.MSID = mask_id;
			p->mObj.bF.MSID11 = 0; //kjg_181031
			//p->mObj.bF.MIDE = 1; //Only allow standard frame
			p->mObj.bF.MIDE = 0; //Both Standard Extended frames will be accepted		//ktg_201221
			p->mObj.bF.MEID = 0x00;
			CHIP_FilterMaskConfigure(num, CHIP_FILT0);
		} else {
			//Setup RX Filter
			p->fObj.word = 0;
			p->fObj.bF.SID = (filter_id >> 18) & 0x7FF;
			p->fObj.bF.SID11 = 0; //kjg_181031
			p->fObj.bF.EXIDE = 1;
			p->fObj.bF.EID = filter_id & 0x3FFFF;
			CHIP_FilterObjectConfigure(num, CHIP_FILT0);

			//Setup RX Mask
			p->mObj.word = 0;
			p->mObj.bF.MSID = (mask_id >> 18) & 0x7FF;
			p->mObj.bF.MSID11 = 0; //kjg_181031
			p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
			p->mObj.bF.MEID = mask_id & 0x3FFFF;
			CHIP_FilterMaskConfigure(num, CHIP_FILT0);
		}
		CHIP_FilterToFifoLink(num, CHIP_FILT0, CHIP_FIFO_CH1, true);
	}
	//kjg_2020_1212 109.6us
	//hrt[12] = gethrtime();

	//Setup Bit Time
	if(fd_flag == 0) {
		switch(baud) {
			case 125000: p->selectedBitTime = CHIP_125K_500K; break;
			case 250000: p->selectedBitTime = CHIP_250K_500K; break;
			case 500000: p->selectedBitTime = CHIP_500K_1M; break;
			case 1000000: p->selectedBitTime = CHIP_1000K_4M; break;
			default: p->selectedBitTime = CHIP_500K_1M; break;
		}
	} else {
		switch(baud) {
			case 125000: p->selectedBitTime = CHIP_125K_500K; break;
			case 250000:
				if(data_rate == 0) { //500K
					p->selectedBitTime = CHIP_250K_500K;
				} else if(data_rate == 1) { //833K
					p->selectedBitTime = CHIP_250K_833K;
				} else if(data_rate == 2) { //1M
					p->selectedBitTime = CHIP_250K_1M;
				} else if(data_rate == 3) { //1M5
					p->selectedBitTime = CHIP_250K_1M5;
				} else if(data_rate == 4) { //2M
					p->selectedBitTime = CHIP_250K_2M;
				} else if(data_rate == 5) { //3M
					p->selectedBitTime = CHIP_250K_3M;
				} else if(data_rate == 6) { //4M
					p->selectedBitTime = CHIP_250K_4M;
				} else { //2M
					p->selectedBitTime = CHIP_250K_2M;
				}
				break;
			case 500000:
				if(data_rate == 0) { //500K
					p->selectedBitTime = CHIP_500K_1M;
				} else if(data_rate == 1) { //833K
					p->selectedBitTime = CHIP_500K_1M;
				} else if(data_rate == 2) { //1M
					p->selectedBitTime = CHIP_500K_1M;
				} else if(data_rate == 3) { //1M5
					p->selectedBitTime = CHIP_500K_1M;
				} else if(data_rate == 4) { //2M
					p->selectedBitTime = CHIP_500K_2M;
				} else if(data_rate == 5) { //3M
					p->selectedBitTime = CHIP_500K_3M;
				} else if(data_rate == 6) { //4M
					p->selectedBitTime = CHIP_500K_4M;
				} else { //2M
					p->selectedBitTime = CHIP_500K_2M;
				}
				break;
			case 1000000: p->selectedBitTime = CHIP_1000K_4M; break;
			default: p->selectedBitTime = CHIP_500K_1M; break;
		}
	}
	CHIP_BitTimeConfigure(num, CHIP_SSP_MODE_AUTO, CHIP_SYSCLK_40M);
	//kjg_2020_1212 82.1us
	//hrt[13] = gethrtime();

	//Setup Transmit and Receive Interrupts
	CHIP_GpioModeConfigure(num, CHIP_PINMODE_INT, CHIP_PINMODE_INT);
	//kjg_2020_1212 26.6us
	//hrt[14] = gethrtime();
	//kjg_180405 CHIP_TransmitChannelEventEnable(num, CHIP_FIFO_CH2, CHIP_TX_FIFO_NOT_FULL_EVENT);
	CHIP_TransmitChannelEventDisable(num, CHIP_FIFO_CH2, CHIP_TX_FIFO_NOT_FULL_EVENT);
	//kjg_2020_1212 24.0us
	//hrt[15] = gethrtime();
	CHIP_ReceiveChannelEventEnable(num, CHIP_FIFO_CH1, CHIP_RX_FIFO_NOT_EMPTY_EVENT);
	//kjg_2020_1212 26.4us
	//hrt[16] = gethrtime();
	//kjg_180405 CHIP_ModuleEventEnable(num, CHIP_TX_EVENT | CHIP_RX_EVENT);
	CHIP_ModuleEventEnable(num, CHIP_RX_EVENT);
	//kjg_2020_1212 33.4us
	//hrt[17] = gethrtime();

	//Select Normal Mode
	if(fd_flag == 0) {
		p->flag = RT_CAN_FD_CLASSIC_FLAG;
		CHIP_OperationModeSelect(num, CHIP_CLASSIC_MODE);
	} else {
		p->flag = RT_CAN_FD_NORMAL_FLAG;
		CHIP_OperationModeSelect(num, CHIP_NORMAL_MODE);
	}
	//kjg_2020_1212 26.1us
	//hrt[18] = gethrtime();
	//kjg_2020_1212 5349.7us
/*
	rtl_printf(
		"kjgd_2020_1212_1 %d %lld %lld %lld %lld %lld %lld %lld %lld %lld\n",
		num,
		hrt[1]-hrt[0], hrt[2]-hrt[1], hrt[3]-hrt[2], hrt[4]-hrt[3],
		hrt[5]-hrt[4], hrt[6]-hrt[5], hrt[7]-hrt[6], hrt[8]-hrt[7],
		hrt[9]-hrt[8]);

	rtl_printf(
		"kjgd_2020_1212_2 %d %lld %lld %lld %lld %lld %lld %lld %lld %lld, %lld\n",
		num,
		hrt[10]-hrt[9], hrt[11]-hrt[10], hrt[12]-hrt[11], hrt[13]-hrt[12],
		hrt[14]-hrt[13], hrt[15]-hrt[14], hrt[16]-hrt[15], hrt[17]-hrt[16],
		hrt[18]-hrt[17], hrt[18]-hrt[0]);
*/
	return 0;
}

int CAN_FD_A_Initialize_3(int num, int phase, int baud, int sjw, int ext, int fd_flag, int data_rate, int crc)
{ //kjg_2020_1212
	int i, can_ch, can_type;
	long filter_id, mask_id;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	switch(phase) {
		case 1:
			p->rxFlags = 0;
	
			CHIP_Reset(num);
			//kjg_2020_1212 8.1us
			
			CHIP_EccEnable(num);
			//kjg_2020_1212 26.9us
			break;
		case 23:
			//Configure device
			CHIP_ConfigureObjectReset(num);
			//kjg_2020_1212 1.3us
	
			if(crc == 0) p->config.IsoCrcEnable = 0;
			else p->config.IsoCrcEnable = 1; //ISO_CRC
			p->config.StoreInTEF = 0;
			CHIP_Configure(num);
			//kjg_2020_1212 18.6us
			break;
		case 24:
			//Setup TX FIFO
			CHIP_TransmitChannelConfigureObjectReset(num);
			//kjg_2020_1212 1.0us
	
			p->txConfig.FifoSize = 7;
			p->txConfig.PayLoadSize = CHIP_PLSIZE_64;
			p->txConfig.TxPriority = 1;
			CHIP_TransmitChannelConfigure(num, CHIP_FIFO_CH2);
			//kjg_2020_1212 17.5us

			//Setup RX FIFO
			CHIP_ReceiveChannelConfigureObjectReset(num);
			//kjg_2020_1212 1.2us
	
			p->rxConfig.FifoSize = 15;
			p->rxConfig.PayLoadSize = CHIP_PLSIZE_64;
			CHIP_ReceiveChannelConfigure(num, CHIP_FIFO_CH1);
			//kjg_2020_1212 17.4us
			break;
		case 25:
			//j = 0;
			set_parameter[num][9] = 0;
			for(i=0; i < 2; i++) {
				switch(myData->mData.config.division_CAN) { //ktg_230612s
					case 1:
						can_ch = num;
						can_type = 0;
						break;
					default:
						switch(myData->AppControl.config.systemModel) { //kjhw_170629s
							case C_SKI_120V_400A_100A_192KW:		//shh_230612s
							case C_SKI_120V_400A_100A_192KW_2:
							case C_SKI_120V_400A_100A_192KW_3:
							case C_SKI_120V_400A_100A_192KW_4:
							case C_SKI_120V_400A_100A_192KW_5:
							case C_SKI_120V_400A_100A_192KW_6:
							case C_SKI_120V_400A_100A_192KW_7:
							case C_SKI_120V_400A_100A_192KW_8:
							case C_LGC_500V_250A_125A_50A_500KW: 	
							case C_SKI_500V_450A_200A_450KW:	
							case C_SKI_500V_450A_200A_450KW_2:	
							case C_SKI_500V_450A_200A_450KW_3:		
							case C_SKI_500V_450A_200A_450KW_4:	
							case C_SKI_500V_450A_200A_450KW_5:		
							case C_SKI_500V_450A_200A_450KW_6:		//shh_230612e
								can_ch = num / 2;
								if(can_ch == 1) can_ch = 2;
								can_type = num % 2;
								break;
							default:
								can_ch = num / 2;
								can_type = num % 2;
								break;
						}
						break;
				}	//ktg_230612e
				//can_ch = num / 2;		//ktg_230612
				//can_type = num % 2;	//ktg_230612
				filter_id = myData->canReceiveSetData.commonData[can_ch]
					[can_type].filter[i];
				if(filter_id == 0) {
					//Link FIFO and Filter
					if(i == 0) CHIP_FilterToFifoLink(num, CHIP_FILT0,
						CHIP_FIFO_CH1, false);
					else CHIP_FilterToFifoLink(num, CHIP_FILT1, CHIP_FIFO_CH1,
						false);
					continue;
				}
				//j++;
				set_parameter[num][9]++;
				mask_id = myData->canReceiveSetData.commonData[can_ch]
					[can_type].mask[0];
				printk(KERN_WARNING
					"kjgd_191124a ch:%d, mask:%lx, filter:%lx\n",
					num, mask_id, filter_id);

				if(ext == 0) {
					//Setup RX Filter
					p->fObj.word = 0;
					p->fObj.bF.SID = filter_id;
					p->fObj.bF.SID11 = 0;
					//p->fObj.bF.EXIDE = 0; //Only execpt Standard frames
					p->fObj.bF.EXIDE = 1; //Both Standard and Extended frames will be accepted	//ktg_201221
					p->fObj.bF.EID = 0;
					if(i == 0) CHIP_FilterObjectConfigure(num, CHIP_FILT0);
					else CHIP_FilterObjectConfigure(num, CHIP_FILT1);

					//Setup RX Mask
					p->mObj.word = 0;
					p->mObj.bF.MSID = mask_id;
					p->mObj.bF.MSID11 = 0;
					//p->mObj.bF.MIDE = 1; //Only allow standard frame
					p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted	//ktg_201221
					p->mObj.bF.MEID = 0x00;
					if(i == 0) CHIP_FilterMaskConfigure(num, CHIP_FILT0);
					else CHIP_FilterMaskConfigure(num, CHIP_FILT1);
				} else {
					//Setup RX Filter
					p->fObj.word = 0;
					p->fObj.bF.SID = (filter_id >> 18) & 0x7FF;
					p->fObj.bF.SID11 = 0;
					p->fObj.bF.EXIDE = 1;
					p->fObj.bF.EID = filter_id & 0x3FFFF;
					if(i == 0) CHIP_FilterObjectConfigure(num, CHIP_FILT0);
					else CHIP_FilterObjectConfigure(num, CHIP_FILT1);

					//Setup RX Mask
					p->mObj.word = 0;
					p->mObj.bF.MSID = (mask_id >> 18) & 0x7FF;
					p->mObj.bF.MSID11 = 0;
					p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
					p->mObj.bF.MEID = mask_id & 0x3FFFF;
					if(i == 0) CHIP_FilterMaskConfigure(num, CHIP_FILT0);
					else CHIP_FilterMaskConfigure(num, CHIP_FILT1);
				}

				//Link FIFO and Filter
				if(i == 0) {
					CHIP_FilterToFifoLink(num, CHIP_FILT0, CHIP_FIFO_CH1, true);
				} else {
					CHIP_FilterToFifoLink(num, CHIP_FILT1, CHIP_FIFO_CH1, true);
				}
			}
			//kjg_2020_1212 22.3us
			break;
		case 26:
			for(i=0; i < 4; i++) {
				switch(myData->mData.config.division_CAN) { //ktg_230612s
					case 1:
						can_ch = num;
						can_type = 0;
						break;
					default:
						switch(myData->AppControl.config.systemModel) { //kjhw_170629s
							case C_SKI_120V_400A_100A_192KW:		//shh_230612s
							case C_SKI_120V_400A_100A_192KW_2:
							case C_SKI_120V_400A_100A_192KW_3:
							case C_SKI_120V_400A_100A_192KW_4:
							case C_SKI_120V_400A_100A_192KW_5:
							case C_SKI_120V_400A_100A_192KW_6:
							case C_SKI_120V_400A_100A_192KW_7:
							case C_SKI_120V_400A_100A_192KW_8:
							case C_LGC_500V_250A_125A_50A_500KW: 	
							case C_SKI_500V_450A_200A_450KW:	
							case C_SKI_500V_450A_200A_450KW_2:	
							case C_SKI_500V_450A_200A_450KW_3:		
							case C_SKI_500V_450A_200A_450KW_4:	
							case C_SKI_500V_450A_200A_450KW_5:		
							case C_SKI_500V_450A_200A_450KW_6:		//shh_230612e
								can_ch = num / 2;
								if(can_ch == 1) can_ch = 2;
								can_type = num % 2;
								break;
							default:
								can_ch = num / 2;
								can_type = num % 2;
								break;
						}
						break;
				}	//ktg_230612e
				//can_ch = num / 2;		//ktg_230612
				//can_type = num % 2;	//ktg_230612
				filter_id = myData->canReceiveSetData.commonData[can_ch]
					[can_type].filter[i+2];
				if(filter_id == 0) {
					//Link FIFO and Filter
					switch(i) {
						case 0:
							CHIP_FilterToFifoLink(num, CHIP_FILT2,
								CHIP_FIFO_CH1, false);
							break;
						case 1:
							CHIP_FilterToFifoLink(num, CHIP_FILT3,
								CHIP_FIFO_CH1, false);
							break;
						case 2:
							CHIP_FilterToFifoLink(num, CHIP_FILT4,
								CHIP_FIFO_CH1, false);
							break;
						case 3:
							CHIP_FilterToFifoLink(num, CHIP_FILT5,
								CHIP_FIFO_CH1, false);
							break;
						default:
							break;
					}
					continue;
				}
				//j++;
				set_parameter[num][9]++;
				mask_id = myData->canReceiveSetData.commonData[can_ch]
					[can_type].mask[1];
				printk(KERN_WARNING
					"kjgd_191124b ch:%d, mask:%lx, filter:%lx\n",
					num, mask_id, filter_id);

				if(ext == 0) {
					//Setup RX Filter
					p->fObj.word = 0;
					p->fObj.bF.SID = filter_id;
					p->fObj.bF.SID11 = 0;
					//p->fObj.bF.EXIDE = 0; //Only execpt Standard frames
					p->fObj.bF.EXIDE = 1; //Both Standard and Extended frames will be accepted	//ktg_201221
					p->fObj.bF.EID = 0;
					switch(i) {
						case 0:
							CHIP_FilterObjectConfigure(num, CHIP_FILT2);
							break;
						case 1:
							CHIP_FilterObjectConfigure(num, CHIP_FILT3);
							break;
						case 2:
							CHIP_FilterObjectConfigure(num, CHIP_FILT4);
							break;
						case 3:
							CHIP_FilterObjectConfigure(num, CHIP_FILT5);
							break;
						default:
							break;
					}

					//Setup RX Mask
					p->mObj.word = 0;
					p->mObj.bF.MSID = mask_id;
					p->mObj.bF.MSID11 = 0;
					//p->mObj.bF.MIDE = 1; //Only allow standard frame
					p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted	//ktg_201221
					p->mObj.bF.MEID = 0x00;
					switch(i) {
						case 0:
							CHIP_FilterMaskConfigure(num, CHIP_FILT2);
							break;
						case 1:
							CHIP_FilterMaskConfigure(num, CHIP_FILT3);
							break;
						case 2:
							CHIP_FilterMaskConfigure(num, CHIP_FILT4);
							break;
						case 3:
							CHIP_FilterMaskConfigure(num, CHIP_FILT5);
							break;
						default:
							break;
					}
				} else {
					//Setup RX Filter
					p->fObj.word = 0;
					p->fObj.bF.SID = (filter_id >> 18) & 0x7FF;
					p->fObj.bF.SID11 = 0;
					p->fObj.bF.EXIDE = 1;
					p->fObj.bF.EID = filter_id & 0x3FFFF;
					switch(i) {
						case 0:
							CHIP_FilterObjectConfigure(num, CHIP_FILT2);
							break;
						case 1:
							CHIP_FilterObjectConfigure(num, CHIP_FILT3);
							break;
						case 2:
							CHIP_FilterObjectConfigure(num, CHIP_FILT4);
							break;
						case 3:
							CHIP_FilterObjectConfigure(num, CHIP_FILT5);
							break;
						default:
							break;
					}

					//Setup RX Mask
					p->mObj.word = 0;
					p->mObj.bF.MSID = (mask_id >> 18) & 0x7FF;
					p->mObj.bF.MSID11 = 0;
					p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
					p->mObj.bF.MEID = mask_id & 0x3FFFF;
					switch(i) {
						case 0:
							CHIP_FilterMaskConfigure(num, CHIP_FILT2);
							break;
						case 1:
							CHIP_FilterMaskConfigure(num, CHIP_FILT3);
							break;
						case 2:
							CHIP_FilterMaskConfigure(num, CHIP_FILT4);
							break;
						case 3:
							CHIP_FilterMaskConfigure(num, CHIP_FILT5);
							break;
						default:
							break;
					}
				}

				//Link FIFO and Filter
				switch(i) {
					case 0:
						CHIP_FilterToFifoLink(num, CHIP_FILT2,
							CHIP_FIFO_CH1, true);
						break;
					case 1:
						CHIP_FilterToFifoLink(num, CHIP_FILT3,
							CHIP_FIFO_CH1, true);
						break;
					case 2:
						CHIP_FilterToFifoLink(num, CHIP_FILT4,
							CHIP_FIFO_CH1, true);
						break;
					case 3:
						CHIP_FilterToFifoLink(num, CHIP_FILT5,
							CHIP_FIFO_CH1, true);
						break;
					default: break;
				}
			}
			//kjg_2020_1212 42.8us
			break;
		case 27:
			//if(j == 0) {
			if(set_parameter[num][9] == 0) {
				switch(myData->mData.config.division_CAN) { //ktg_230612s
					case 1:
						can_ch = num;
						can_type = 0;
						break;
					default:
						switch(myData->AppControl.config.systemModel) { //kjhw_170629s
							case C_SKI_120V_400A_100A_192KW:		//shh_230612s
							case C_SKI_120V_400A_100A_192KW_2:
							case C_SKI_120V_400A_100A_192KW_3:
							case C_SKI_120V_400A_100A_192KW_4:
							case C_SKI_120V_400A_100A_192KW_5:
							case C_SKI_120V_400A_100A_192KW_6:
							case C_SKI_120V_400A_100A_192KW_7:
							case C_SKI_120V_400A_100A_192KW_8:
							case C_LGC_500V_250A_125A_50A_500KW: 	
							case C_SKI_500V_450A_200A_450KW:	
							case C_SKI_500V_450A_200A_450KW_2:	
							case C_SKI_500V_450A_200A_450KW_3:		
							case C_SKI_500V_450A_200A_450KW_4:	
							case C_SKI_500V_450A_200A_450KW_5:		
							case C_SKI_500V_450A_200A_450KW_6:		//shh_230612e
								can_ch = num / 2;
								if(can_ch == 1) can_ch = 2;
								can_type = num % 2;
								break;
							default:
								can_ch = num / 2;
								can_type = num % 2;
								break;
						}
						break;
				}	//ktg_230612e
				//can_ch = num / 2;		//ktg_230612
				//can_type = num % 2;	//ktg_230612
				mask_id = 0; filter_id = 0;
				printk(KERN_WARNING
					"kjgd_191124c ch:%d, mask:%lx, filter:%lx\n",
					num, mask_id, filter_id);

				if(ext == 0) {
					//Setup RX Filter
					p->fObj.word = 0;
					p->fObj.bF.SID = filter_id;
					p->fObj.bF.SID11 = 0;
					//p->fObj.bF.EXIDE = 0; //Only execpt Standard frames
					p->fObj.bF.EXIDE = 1; //Both Standard and Extended frames will be accpted	//ktg_201221
					p->fObj.bF.EID = 0;
					CHIP_FilterObjectConfigure(num, CHIP_FILT0);

					//Setup RX Mask
					p->mObj.word = 0;
					p->mObj.bF.MSID = mask_id;
					p->mObj.bF.MSID11 = 0;
					//p->mObj.bF.MIDE = 1; //Only allow standard frame
					p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accpted	//ktg_201221
					p->mObj.bF.MEID = 0x00;
					CHIP_FilterMaskConfigure(num, CHIP_FILT0);
				} else {
					//Setup RX Filter
					p->fObj.word = 0;
					p->fObj.bF.SID = (filter_id >> 18) & 0x7FF;
					p->fObj.bF.SID11 = 0;
					p->fObj.bF.EXIDE = 1;
					p->fObj.bF.EID = filter_id & 0x3FFFF;
					CHIP_FilterObjectConfigure(num, CHIP_FILT0);

					//Setup RX Mask
					p->mObj.word = 0;
					p->mObj.bF.MSID = (mask_id >> 18) & 0x7FF;
					p->mObj.bF.MSID11 = 0;
					p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
					p->mObj.bF.MEID = mask_id & 0x3FFFF;
					CHIP_FilterMaskConfigure(num, CHIP_FILT0);
				}
				CHIP_FilterToFifoLink(num, CHIP_FILT0, CHIP_FIFO_CH1, true);
			}
			//kjg_2020_1212 109.6us
			break;
		case 28:
			//Setup Bit Time
			if(fd_flag == 0) {
				switch(baud) {
					case 125000: p->selectedBitTime = CHIP_125K_500K; break;
					case 250000: p->selectedBitTime = CHIP_250K_500K; break;
					case 500000: p->selectedBitTime = CHIP_500K_1M; break;
					case 1000000: p->selectedBitTime = CHIP_1000K_4M; break;
					default: p->selectedBitTime = CHIP_500K_1M; break;
				}
			} else {
				switch(baud) {
					case 125000: p->selectedBitTime = CHIP_125K_500K; break;
					case 250000:
						if(data_rate == 0) { //500K
							p->selectedBitTime = CHIP_250K_500K;
						} else if(data_rate == 1) { //833K
							p->selectedBitTime = CHIP_250K_833K;
						} else if(data_rate == 2) { //1M
							p->selectedBitTime = CHIP_250K_1M;
						} else if(data_rate == 3) { //1M5
							p->selectedBitTime = CHIP_250K_1M5;
						} else if(data_rate == 4) { //2M
							p->selectedBitTime = CHIP_250K_2M;
						} else if(data_rate == 5) { //3M
							p->selectedBitTime = CHIP_250K_3M;
						} else if(data_rate == 6) { //4M
							p->selectedBitTime = CHIP_250K_4M;
						} else { //2M
							p->selectedBitTime = CHIP_250K_2M;
						}
						break;
					case 500000:
						if(data_rate == 0) { //500K
							p->selectedBitTime = CHIP_500K_1M;
						} else if(data_rate == 1) { //833K
							p->selectedBitTime = CHIP_500K_1M;
						} else if(data_rate == 2) { //1M
							p->selectedBitTime = CHIP_500K_1M;
						} else if(data_rate == 3) { //1M5
							p->selectedBitTime = CHIP_500K_1M;
						} else if(data_rate == 4) { //2M
							p->selectedBitTime = CHIP_500K_2M;
						} else if(data_rate == 5) { //3M
							p->selectedBitTime = CHIP_500K_3M;
						} else if(data_rate == 6) { //4M
							p->selectedBitTime = CHIP_500K_4M;
						} else { //2M
							p->selectedBitTime = CHIP_500K_2M;
						}
						break;
					case 1000000: p->selectedBitTime = CHIP_1000K_4M; break;
					default: p->selectedBitTime = CHIP_500K_1M; break;
				}
			}
			CHIP_BitTimeConfigure(num, CHIP_SSP_MODE_AUTO, CHIP_SYSCLK_40M);
			//kjg_2020_1212 82.1us
			break;
		case 29:
			//Setup Transmit and Receive Interrupts
			CHIP_GpioModeConfigure(num, CHIP_PINMODE_INT, CHIP_PINMODE_INT);
			//kjg_2020_1212 26.6us
	
			//kjg_180405 CHIP_TransmitChannelEventEnable(num, CHIP_FIFO_CH2, CHIP_TX_FIFO_NOT_FULL_EVENT);
			CHIP_TransmitChannelEventDisable(num, CHIP_FIFO_CH2,
				CHIP_TX_FIFO_NOT_FULL_EVENT);
			//kjg_2020_1212 24.0us
	
			CHIP_ReceiveChannelEventEnable(num, CHIP_FIFO_CH1,
				CHIP_RX_FIFO_NOT_EMPTY_EVENT);
			//kjg_2020_1212 26.4us
			break;
		case 30:
			//kjg_180405 CHIP_ModuleEventEnable(num, CHIP_TX_EVENT | CHIP_RX_EVENT);
			CHIP_ModuleEventEnable(num, CHIP_RX_EVENT);
			//kjg_2020_1212 33.4us

			//Select Normal Mode
			if(fd_flag == 0) {
				p->flag = RT_CAN_FD_CLASSIC_FLAG;
				CHIP_OperationModeSelect(num, CHIP_CLASSIC_MODE);
			} else {
				p->flag = RT_CAN_FD_NORMAL_FLAG;
				CHIP_OperationModeSelect(num, CHIP_NORMAL_MODE);
			}
			//kjg_2020_1212 26.1us
			break;
		default: //2~22
			CHIP_RamInit_3(num, 0xFF, phase);
			//kjg_2020_1212 4873.7us / 21 = 232us
			break;
	}

	return 0;
}

/*kjg_180523
int rt_can_set_param(int num, int baud, int sjw, int ext, int fd_flag, int data_rate, int crc, int terminal_r)
{
	unsigned char tmp1, mask;
	unsigned int base;
	struct rt_can_fd_struct *p;

	if(max_can_fd_ch == 0) return 0; //kjg_180405

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING "rt_can_fd: rt_can_set_param error %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

	tmp1 = 0x01;
	if(terminal_r == 0) {
		if(num <= 3) {
			terminal_resistor[0] &= ~(tmp1 << num);
		} else {
			terminal_resistor[1] &= ~(tmp1 << (num - 4));
		}
	} else {
		if(num <= 3) {
			terminal_resistor[0] |= (tmp1 << num);
		} else {
			terminal_resistor[1] |= (tmp1 << (num - 4));
		}
	}
	base = RT_CAN_FD_BASE_ADDR;
	outb(terminal_resistor[0], base + RT_CAN_FD_BD1_OUT_1);
	outb(terminal_resistor[1], base + RT_CAN_FD_BD2_OUT_1);

	tmp1 = 0x01;
	tmp1 = tmp1 << num;
	outb(tmp1, base + RT_CAN_FD_PORT_SELECT);

	mask = inb(base + RT_CAN_FD_PORT_IRQ_MASK);
	mask &= (~tmp1); //IRQ_OFF
	outb(mask, base + RT_CAN_FD_PORT_IRQ_MASK);

	p->rx_msg_write_idx = 0;
	p->rx_msg_read_idx = 0;
	p->rx_msg_count = 0;
	p->tx_msg_write_idx = 0;
	p->tx_msg_read_idx = 0;
	p->tx_msg_count = 0;
	p->tx_msg_error = 0;
	p->error = 0; //init. last error code
	p->error_count = 0;
	p->type = ext;
	
	CAN_FD_A_Initialize(num, baud, sjw, ext, fd_flag, data_rate, crc);

	p->used |= 0x02; //mark setup done

	mask = inb(base + RT_CAN_FD_PORT_IRQ_MASK);
	mask |= tmp1; //IRQ_ON
	outb(mask, base + RT_CAN_FD_PORT_IRQ_MASK);

	return 0;
}*/

int rt_can_set_param(int num, int baud, int sjw, int ext, int fd_flag, int data_rate, int crc, int terminal_r)
{ //kjg_180523
	int i;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	if(max_can_fd_ch == 0) return 0;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING "rt_can_fd: rt_can_set_param error %d\n", num);
		return -1;
	}

	p->status = 1;
	set_parameter[num][1] = baud;
	set_parameter[num][2] = sjw;
	set_parameter[num][3] = ext;
	set_parameter[num][4] = fd_flag;
	set_parameter[num][5] = data_rate;
	set_parameter[num][6] = crc;
	set_parameter[num][7] = terminal_r;

	for(i=8; i < 16; i++) { //kjg_2020_1213
		set_parameter[num][i] = 0;
	}

	for(i=0; i < 100; i++) { //kjg_2020_1213
		rx_can_id[num][i] = 0;
	}

	return 0;
}

int rt_can_set_param_2(int num)
{
	unsigned char tmp1, mask;
	int baud, sjw, ext, fd_flag, data_rate, crc, terminal_r;
	unsigned int base;
	struct rt_can_fd_struct *p;

	//kjg_180523
	baud = set_parameter[num][1];
	sjw = set_parameter[num][2];
	ext = set_parameter[num][3];
	fd_flag = set_parameter[num][4];
	data_rate = set_parameter[num][5];
	crc = set_parameter[num][6];
	terminal_r = set_parameter[num][7];

	p = &(rt_can_fd_table[num]);

	tmp1 = 0x01;
	if(terminal_r == 0) {
		if(num <= 3) {
			terminal_resistor[0] &= ~(tmp1 << num);
		} else {
			terminal_resistor[1] &= ~(tmp1 << (num - 4));
		}
	} else {
		if(num <= 3) {
			terminal_resistor[0] |= (tmp1 << num);
		} else {
			terminal_resistor[1] |= (tmp1 << (num - 4));
		}
	}
	base = RT_CAN_FD_BASE_ADDR;
	outb(terminal_resistor[0], base + RT_CAN_FD_BD1_OUT_1);
	outb(terminal_resistor[1], base + RT_CAN_FD_BD2_OUT_1);

	tmp1 = 0x01;
	tmp1 = tmp1 << num;
	outb(tmp1, base + RT_CAN_FD_PORT_SELECT);

	mask = inb(base + RT_CAN_FD_PORT_IRQ_MASK);
	mask &= (~tmp1); //IRQ_OFF
	outb(mask, base + RT_CAN_FD_PORT_IRQ_MASK);

	p->rx_msg_write_idx = 0;
	p->rx_msg_read_idx = 0;
	p->rx_msg_count = 0;
	p->tx_msg_write_idx = 0;
	p->tx_msg_read_idx = 0;
	p->tx_msg_count = 0;
	p->tx_msg_error = 0;
	p->error = 0; //init. last error code
	p->error_count = 0;
	p->type = ext;
	
	CAN_FD_A_Initialize(num, baud, sjw, ext, fd_flag, data_rate, crc);

	p->used |= 0x02; //mark setup done

	mask = inb(base + RT_CAN_FD_PORT_IRQ_MASK);
	mask |= tmp1; //IRQ_ON
	outb(mask, base + RT_CAN_FD_PORT_IRQ_MASK);

	return 0;
}

int rt_can_set_param_3(int num)
{ //kjg_2020_1212
	unsigned char tmp1;//, mask;
	int baud, sjw, ext, fd_flag, data_rate, crc, terminal_r, i;
	unsigned int base, phase;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	phase = p->status - 750;

	switch(phase) {
		case 0:
			for(i=8; i < 16; i++) { //kjg_2020_1213
				set_parameter[num][i] = 0;
			}

			for(i=0; i < 100; i++) { //kjg_2020_1213
				rx_can_id[num][i] = 0;
			}

			terminal_r = set_parameter[num][7];

			tmp1 = 0x01;
			if(terminal_r == 0) {
				if(num <= 3) {
					terminal_resistor[0] &= ~(tmp1 << num);
				} else {
					terminal_resistor[1] &= ~(tmp1 << (num - 4));
				}
			} else {
				if(num <= 3) {
					terminal_resistor[0] |= (tmp1 << num);
				} else {
					terminal_resistor[1] |= (tmp1 << (num - 4));
				}
			}
			base = RT_CAN_FD_BASE_ADDR;
			outb(terminal_resistor[0], base + RT_CAN_FD_BD1_OUT_1);
			outb(terminal_resistor[1], base + RT_CAN_FD_BD2_OUT_1);

/*			tmp1 = 0x01;
			tmp1 = tmp1 << num;
			outb(tmp1, base + RT_CAN_FD_PORT_SELECT);

			mask = inb(base + RT_CAN_FD_PORT_IRQ_MASK);
			mask &= (~tmp1); //IRQ_OFF
			outb(mask, base + RT_CAN_FD_PORT_IRQ_MASK);
			set_parameter[num][8] = tmp1;
*/
			p->rx_msg_write_idx = 0;
			p->rx_msg_read_idx = 0;
			p->rx_msg_count = 0;
			p->tx_msg_write_idx = 0;
			p->tx_msg_read_idx = 0;
			p->tx_msg_count = 0;
			p->tx_msg_error = 0;
			p->error = 0; //init. last error code
			p->error_count = 0;
			//p->type = ext;
			p->type = set_parameter[num][3];
			break;
		case 31:
			p->used |= 0x02; //mark setup done
/*
			tmp1 = set_parameter[num][8];
			base = RT_CAN_FD_BASE_ADDR;
			mask = inb(base + RT_CAN_FD_PORT_IRQ_MASK);
			mask |= tmp1; //IRQ_ON
			outb(mask, base + RT_CAN_FD_PORT_IRQ_MASK);
*/
			p->status = 0;
			break;
		default: //1~30
			baud = set_parameter[num][1];
			sjw = set_parameter[num][2];
			ext = set_parameter[num][3];
			fd_flag = set_parameter[num][4];
			data_rate = set_parameter[num][5];
			crc = set_parameter[num][6];

			CAN_FD_A_Initialize_3(num, phase,
				baud, sjw, ext, fd_flag, data_rate, crc);
			break;
	}

	return 0;
}

int rt_can_check_status(int num)
{ //kjg_180523
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	return (int)p->status;
}

int rt_can_setup(int num, int baud)
{
	unsigned char tmp;
	unsigned int base;
	struct rt_can_fd_struct *p;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING "rt_can_fd: rt_can_setup error1 %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);
	
	if(p->used == 0) {
		printk(KERN_WARNING "rt_can_fd: rt_can_setup error2 %d, %d\n",
			num, p->used);
		return -2;
	}

	base = RT_CAN_FD_BASE_ADDR;
	tmp = 0x01;
	tmp = tmp << num;
	outb(tmp, base + RT_CAN_FD_PORT_SELECT);

	p->error = 0; //init. last error code

	switch(baud) {
		case 1:
			p->status = 0; //kjg_180523
			p->rx_msg_write_idx = 0;
			p->rx_msg_read_idx = 0;
			p->rx_msg_count = 0;
			p->tx_msg_write_idx = 0;
			p->tx_msg_read_idx = 0;
			p->tx_msg_count = 0;
			p->tx_msg_error = 0;
			p->type = 0;
			CHIP_Reset(num);
			break;
		case 2:
			//kjg_181121 MOD_INC_USE_COUNT;
			p->used |= 0x02; //mark setup done
			break;
		case (-1):
			CHIP_Reset(num);
			break;
		case (-2):
			//kjg_181121 MOD_DEC_USE_COUNT;
			CHIP_Reset(num);
			break;
		default:
			CHIP_Reset(num);
			break;
	}

	return 0;
}

/*char SPI_Transfer(int num, unsigned char *pIN, unsigned char *pOUT, unsigned short len, unsigned char spi_type, int debug)
{
	unsigned char i, j, k, tmp;
	unsigned int addr, delay, base;
	long long hrt[10];

	//kjg_2020_1212
	base = RT_CAN_FD_BASE_ADDR;
	tmp = 0x01;
	tmp = tmp << num;
	outb(tmp, base + RT_CAN_FD_PORT_SELECT);

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_1;
	outb((unsigned char)len, addr);

	if(debug == 100) k = 8; //kjg_2020_0330
	else k = 1;

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_2;
	for(i=0; i < len; i++) {
		outb(*pIN, addr);
		pIN++;

		if(i == k && spi_type == RT_CAN_FD_SPI_RX_ONLY) { //kjg_180522
			break;
		}
	}

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_3;
	outb(0x00, addr);

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_1;
	j = 0;
	hrt[0] = gethrtime(); //kjg_2020_1206
	hrt[1] = gethrtime(); //kjg_2020_1206
	while(1) {
		delay=0; //kjg_181121_2 for(delay=0; delay < DELAY_2US; delay++) {}
		i = inb(addr);
		if(i == (unsigned char)len) break;
		j++;
		//kjg_180522 if(j > 20) {
		//if(j > 50) {
		if(j > 100) { //kjg_2020_1206 50 -> 100
			printk(KERN_WARNING "rt_can_fd: SPI_Transfer %d %d %d %d %d\n",
				num, i, len, debug, j);
			return -1;
		}
	}
	hrt[2] = gethrtime(); //kjg_2020_1206
	hrt[3] = hrt[1] - hrt[0];
	hrt[4] = hrt[2] - hrt[0];
	if(num == 0 && hrt[4] > 5000) {
		//printk(KERN_WARNING "rt_can_fd: kjg_2020_1206 %lld, %lld, %d\n", hrt[3], hrt[4], debug);
	}

	if(spi_type == RT_CAN_FD_SPI_TX_ONLY) { //kjg_180405
		return 0;
	}

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_2;
	for(i=0; i < len; i++) {
		*pOUT = inb(addr);
		pOUT++;
	}

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_3;
	i = inb(addr);

	return 0;
}*/

char SPI_Transfer(int num, unsigned char *pIN, unsigned char *pOUT, unsigned short len, unsigned char spi_type, int debug)
{ //kjg_2020_1212
	unsigned char i, j, k;
	unsigned int addr, addr2;

	switch(num) {
		case 0:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD1_1;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_1;
			break;
		case 1:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD1_4;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_4;
			break;
		case 2:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD1_7;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_7;
			break;
		case 3:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD1_A;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_A;
			break;
		case 4:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD2_1;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_1;
			break;
		case 5:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD2_4;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_4;
			break;
		case 6:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD2_7;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_7;
			break;
		case 7:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD2_A;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_A;
			break;
		default:
			addr = RT_CAN_FD_BASE_ADDR;
			addr2 = RT_CAN_FD_BASE_ADDR;
			break;
	}
	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_1;
	outb((unsigned char)len, addr);

	if(debug == 100) k = 8; //kjg_2020_0330
	else k = 1;

	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_2;
	for(i=0; i < len; i++) {
		outb(*pIN, addr + 1);
		pIN++;

		if(i == k && spi_type == RT_CAN_FD_SPI_RX_ONLY) { //kjg_180522
			break;
		}
	}

	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_3;
	outb(0x00, addr + 2);

	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_1;
	j = 0;
	while(1) {
		i = inb(addr2);
		if(i == (unsigned char)len) break;
		j++;
		if(j > 100) { //kjg_2020_1206 50 -> 100
			printk(KERN_WARNING "rt_can_fd: SPI_Transfer %d %d %d %d %d\n",
				num, i, len, debug, j);
			return -1;
		}
	}

	if(spi_type == RT_CAN_FD_SPI_TX_ONLY) { //kjg_180405
		return 0;
	}

	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_2;
	for(i=0; i < len; i++) {
		*pOUT = inb(addr2 + 1);
		pOUT++;
	}

	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_3;
	//kjg_2020_1213 i = inb(addr2 + 2);

	return 0;
}

char SPI_Transfer_3a(int num, unsigned char *pIN, unsigned char *pOUT, unsigned short len, unsigned char spi_type, int debug)
{ //kjg_2020_1212
	unsigned char i, k;
	unsigned int addr, addr2;

	switch(num) {
		case 0:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD1_1;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_1;
			break;
		case 1:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD1_4;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_4;
			break;
		case 2:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD1_7;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_7;
			break;
		case 3:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD1_A;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_A;
			break;
		case 4:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD2_1;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_1;
			break;
		case 5:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD2_4;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_4;
			break;
		case 6:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD2_7;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_7;
			break;
		case 7:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD2_A;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_A;
			break;
		default:
			addr = RT_CAN_FD_BASE_ADDR;
			addr2 = RT_CAN_FD_BASE_ADDR;
			break;
	}
	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_1;
	outb((unsigned char)len, addr);

	if(debug == 100) k = 8; //kjg_2020_0330
	else k = 1;

	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_2;
	for(i=0; i < len; i++) {
		outb(*pIN, addr + 1);
		pIN++;

		if(i == k && spi_type == RT_CAN_FD_SPI_RX_ONLY) { //kjg_180522
			break;
		}
	}

	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_3;
	outb(0x00, addr + 2);

	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_1;
	i = inb(addr2);
	if(i != (unsigned char)len) return 123;

	return 0;
}

char SPI_Transfer_3b(int num, unsigned short len)
{ //kjg_2020_1212
	unsigned char i;
	unsigned int addr2;

	switch(num) {
		case 0:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_1;
			break;
		case 1:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_4;
			break;
		case 2:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_7;
			break;
		case 3:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_A;
			break;
		case 4:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_1;
			break;
		case 5:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_4;
			break;
		case 6:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_7;
			break;
		case 7:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_A;
			break;
		default:
			addr2 = RT_CAN_FD_BASE_ADDR;
			break;
	}

	i = inb(addr2);
	if(i != (unsigned char)len) return 123;

	return 0;
}

char SPI_Transfer_4a(int num, unsigned char *pIN, unsigned char *pOUT, unsigned short len, unsigned char spi_type, int debug)
{ //kjg_2020_1213
	unsigned char i, k;
	unsigned int addr, addr2;

	switch(num) {
		case 0:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD1_1;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_1;
			break;
		case 1:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD1_4;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_4;
			break;
		case 2:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD1_7;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_7;
			break;
		case 3:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD1_A;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_A;
			break;
		case 4:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD2_1;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_1;
			break;
		case 5:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD2_4;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_4;
			break;
		case 6:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD2_7;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_7;
			break;
		case 7:
			addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_BD2_A;
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_A;
			break;
		default:
			addr = RT_CAN_FD_BASE_ADDR;
			addr2 = RT_CAN_FD_BASE_ADDR;
			break;
	}
	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_1;
	outb((unsigned char)len, addr);

	if(debug == 100) k = 8; //kjg_2020_0330
	else k = 1;

	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_2;
	for(i=0; i < len; i++) {
		outb(*pIN, addr + 1);
		pIN++;

		if(i == k && spi_type == RT_CAN_FD_SPI_RX_ONLY) { //kjg_180522
			break;
		}
	}

	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_3;
	outb(0x00, addr + 2);

	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_1;
	i = inb(addr2);
	if(i != (unsigned char)len) {
		//printk(KERN_WARNING "rt_can_fd: SPI_Transfer_4a %d %d\n", num, i);
		return 123;
	}

	//printk(KERN_WARNING "rt_can_fd: SPI_Transfer_4a ok %d %d\n", num, len);

	return 0;
}

char SPI_Transfer_4b(int num, unsigned char *pOUT, unsigned short len)
{ //kjg_2020_1213
	unsigned char i;
	unsigned int addr2;
	//long long hrt[4];

	switch(num) {
		case 0:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_1;
			break;
		case 1:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_4;
			break;
		case 2:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_7;
			break;
		case 3:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD1_A;
			break;
		case 4:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_1;
			break;
		case 5:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_4;
			break;
		case 6:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_7;
			break;
		case 7:
			addr2 = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_BD2_A;
			break;
		default:
			addr2 = RT_CAN_FD_BASE_ADDR;
			break;
	}

	//hrt[0] = gethrtime();

	i = inb(addr2);
	if(i != (unsigned char)len) {
		//printk(KERN_WARNING "rt_can_fd: SPI_Transfer_4b %d %d\n", num, i);
		return 123;
	}

	//hrt[1] = gethrtime();

	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_2;
	for(i=0; i < len; i++) {
		*pOUT = inb(addr2 + 1);
		pOUT++;
	}

	//hrt[2] = gethrtime();
	//myData->test_val_ll2[3][num] = hrt[1] - hrt[0];
	//myData->test_val_ll2[3][num + 8] = hrt[2] - hrt[0];
	//myData->test_val_ll2[4][num] = len;

	//addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_3;
	//kjg_2020_1213 i = inb(addr2 + 2);

	//printk(KERN_WARNING "rt_can_fd: SPI_Transfer_4b ok %d\n", num);

	return 0;
}

/*kjg_180522
char SPI_Transfer(int num, unsigned char *pIN, unsigned char *pOUT, unsigned short len, unsigned char spi_type, int debug)
{
	unsigned char *in = pIN, *out = pOUT, i, j;
	unsigned int addr, delay;

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_1;
	outb((unsigned char)len, addr);

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_2;
	for(i=0; i < len; i++) {
		outb(*in, addr);
		in++;
	}

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_3;
	outb(0x00, addr);

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_1;
	j = 0;
	while(1) {
		for(delay=0; delay < DELAY_10US; delay++) {}
		i = inb(addr);
		if(i == (unsigned char)len) break;
		j++;
		if(j > 20) {
			printk(KERN_WARNING "rt_can_fd: SPI_Transfer %d %d %d %d %d\n",
				num, i, len, debug, j);
			return -1;
		}
	}

	if(spi_type == RT_CAN_FD_SPI_TX_ONLY) { //kjg_180405
		return 0;
	}

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_2;
	for(i=0; i < len; i++) {
		*out = inb(addr);
		out++;
	}

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_3;
	i = inb(addr);

	return 0;
}*/

//Public Function Definitions
char CHIP_Reset(int num)
{
	char error;
	unsigned short tx_length = 2;
	struct rt_can_fd_struct *p;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING "rt_can_fd: CHIP_Reset error %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

	p->spi_tx_buf[0] = (unsigned char)(_CHIP_INS_RESET << 4);
	p->spi_tx_buf[1] = 0;

	error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_ONLY, 1);

	return error;
}

char CHIP_ReadByte(int num, unsigned short address, unsigned char *rxd)
{
    char error;
    unsigned short tx_length = 3;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_READ << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);
    p->spi_tx_buf[2] = 0;

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_RX, 2);

    //Update data
    *rxd = p->spi_rx_buf[2];

    return error;
}

char CHIP_WriteByte(int num, unsigned short address, unsigned char txd)
{
    char error;
    unsigned short tx_length = 3;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_WRITE << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);
    p->spi_tx_buf[2] = txd;

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_ONLY, 3);

    return error;
}

char CHIP_ReadWord(int num, unsigned short address, unsigned int *rxd)
{
    char error;
    unsigned char i;
    unsigned short tx_length = 6;
    unsigned int x;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_READ << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_RX, 4);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_ReadWord error %d, %d\n",
			num, error);
		return error;
	}

    //Update data
    *rxd = 0;
    for(i=2; i < 6; i++) {
        x = (unsigned int)p->spi_rx_buf[i];
        *rxd += x << ((i - 2) * 8);
    }

    return error;
}

char CHIP_WriteWord(int num, unsigned short address, unsigned int txd)
{
    char error;
    unsigned char i;
    unsigned short tx_length = 6;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_WRITE << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

    //Split word into 4 bytes and add them to buffer
    for(i=0; i < 4; i++) {
        p->spi_tx_buf[i + 2] = (unsigned char)((txd >> (i * 8)) & 0xFF);
    }

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_ONLY, 5);

    return error;
}

char CHIP_ReadHalfWord(int num, unsigned short address, unsigned short *rxd)
{
    char error;
    unsigned char i;
    unsigned short tx_length = 4;
    unsigned int x;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_READ << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_RX, 6);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_ReadHalfWord error %d, %d\n",
			num, error);
		return error;
	}

    //Update data
    *rxd = 0;
    for(i=2; i < 4; i++) {
        x = (unsigned int)p->spi_rx_buf[i];
        *rxd += x << ((i - 2) * 8);
    }

    return error;
}

char CHIP_WriteHalfWord(int num, unsigned short address, unsigned short txd)
{
    char error;
    unsigned char i;
    unsigned short tx_length = 4;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_WRITE << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

    //Split word into 2 bytes and add them to buffer
    for(i=0; i < 2; i++) {
        p->spi_tx_buf[i + 2] = (unsigned char)((txd >> (i * 8)) & 0xFF);
    }

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_ONLY, 7);

    return error;
}

char CHIP_WriteByteSafe(int num, unsigned short address, unsigned char txd)
{
    char error;
    unsigned short crcResult = 0, tx_length = 5;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_WRITE_SAFE << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);
    p->spi_tx_buf[2] = txd;

    //Add CRC
    crcResult = CHIP_CalculateCRC16(p->spi_tx_buf, 3);
    p->spi_tx_buf[3] = (crcResult >> 8) & 0xFF;
    p->spi_tx_buf[4] = crcResult & 0xFF;

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_ONLY, 8);

    return error;
}

char CHIP_WriteWordSafe(int num, unsigned short address, unsigned int txd)
{
    char error;
    unsigned char i;
    unsigned short crcResult = 0, tx_length = 8;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_WRITE_SAFE << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

    //Split word into 4 bytes and add them to buffer
    for(i=0; i < 4; i++) {
        p->spi_tx_buf[i + 2] = (unsigned char)((txd >> (i * 8)) & 0xFF);
    }

    //Add CRC
    crcResult = CHIP_CalculateCRC16(p->spi_tx_buf, 6);
    p->spi_tx_buf[6] = (crcResult >> 8) & 0xFF;
    p->spi_tx_buf[7] = crcResult & 0xFF;

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_ONLY, 9);

    return error;
}

char CHIP_ReadByteArray(int num, unsigned short address, unsigned char *rxd, unsigned short nBytes)
{
    char error;
    unsigned short i, tx_length = nBytes + 2;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_READ << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

    //Clear data
    for(i=2; i < tx_length; i++) {
        p->spi_tx_buf[i] = 0;
    }

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_RX_ONLY, 10);

    //Update data
    for(i=0; i < nBytes; i++) {
        rxd[i] = p->spi_rx_buf[i + 2];
    }

    return error;
}

/*kjg_2020_0322 char CHIP_ReadByteArrayWithCRC(int num, unsigned short address, unsigned char *rxd, unsigned short nBytes)
{
    char error;
    unsigned char i;
    unsigned short crcFromSpiSlave = 0, crcAtController = 0, tx_length;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	if(p->flag == RT_CAN_FD_CLASSIC_FLAG) {
		nBytes = 20;
	} else { //RT_CAN_FD_NORMAL_FLAG;
	}

	tx_length = nBytes + 5; //first two bytes for sending command & address, third for size, last two bytes for CRC

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_READ_CRC << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);
    //if(fromRam) {
        p->spi_tx_buf[2] = nBytes >> 2;
    //} else {
    //    p->spi_tx_buf[2] = nBytes;
    //}

    //Clear data
    for(i=3; i < tx_length; i++) {
        p->spi_tx_buf[i] = 0;
    }

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_RX_ONLY, 100);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReadByteArrayWithCRC error1 %d, %d\n", num, error);
		return error;
	}

    //Get CRC from controller
    crcFromSpiSlave = (unsigned short)(p->spi_rx_buf[tx_length - 2] << 8)
		+ (unsigned short)(p->spi_rx_buf[tx_length - 1]);

    //Use the receive buffer to calculate CRC
    //First three bytes need to be command
    p->spi_rx_buf[0] = p->spi_tx_buf[0];
    p->spi_rx_buf[1] = p->spi_tx_buf[1];
    p->spi_rx_buf[2] = p->spi_tx_buf[2];
    crcAtController = CHIP_CalculateCRC16(p->spi_rx_buf, nBytes + 3);

    //Compare CRC readings
    if(crcFromSpiSlave == crcAtController) {
 	   //Update data
    	for(i=0; i < nBytes; i++) {
        	rxd[i] = p->spi_rx_buf[i + 3];
    	}
    } else {
		//kjgw_190323 printk(KERN_WARNING
		//	"rt_can_fd: CHIP_ReadByteArrayWithCRC error2 %d, %x, %x\n",
		//	num, crcFromSpiSlave, crcAtController);
    }

    return error;
}
*/
char CHIP_ReadByteArrayWithCRC(int num, unsigned short address, unsigned char *rxd, unsigned short nBytes, bool fromRam, bool *crcIsCorrect)
{
    char error;
    unsigned char i;
    unsigned short crcFromSpiSlave = 0, crcAtController = 0, tx_length;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	/*if(p->flag == RT_CAN_FD_CLASSIC_FLAG) { //kjgw_2020_0330
		nBytes = 20;
	} else { //RT_CAN_FD_NORMAL_FLAG;
	}*/

	tx_length = nBytes + 5; //first two bytes for sending command & address, third for size, last two bytes for CRC

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_READ_CRC << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);
    if(fromRam) {
        p->spi_tx_buf[2] = nBytes >> 2;
    } else { //kjgw_2020_0322
        p->spi_tx_buf[2] = nBytes;
    }

    //Clear data
    for(i=3; i < tx_length; i++) {
        p->spi_tx_buf[i] = 0;
    }

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_RX_ONLY, 100);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReadByteArrayWithCRC error1 %d, %d\n", num, error);
		return error;
	}

    //Get CRC from controller
    crcFromSpiSlave = (unsigned short)(p->spi_rx_buf[tx_length - 2] << 8)
		+ (unsigned short)(p->spi_rx_buf[tx_length - 1]);

    //Use the receive buffer to calculate CRC
    //First three bytes need to be command
    p->spi_rx_buf[0] = p->spi_tx_buf[0];
    p->spi_rx_buf[1] = p->spi_tx_buf[1];
    p->spi_rx_buf[2] = p->spi_tx_buf[2];
    crcAtController = CHIP_CalculateCRC16(p->spi_rx_buf, nBytes + 3);

    //Compare CRC readings
    if(crcFromSpiSlave == crcAtController) { //kjgw_2020_0322
		*crcIsCorrect = true;

	 	//Update data
   		for(i=0; i < nBytes; i++) {
       		rxd[i] = p->spi_rx_buf[i + 3];
  	 	}

    } else {
		*crcIsCorrect = false;

		//kjgw_190323 printk(KERN_WARNING
		//	"rt_can_fd: CHIP_ReadByteArrayWithCRC error2 %d, %x, %x\n",
		//	num, crcFromSpiSlave, crcAtController);
    }

    return error;
}

char CHIP_ReadByteArrayWithCRC_4(int num, unsigned short address, unsigned short nBytes, bool fromRam, bool *crcIsCorrect)
{
    char error;
    unsigned char i;
    unsigned short crcFromSpiSlave = 0, crcAtController = 0, tx_length;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	/*if(p->flag == RT_CAN_FD_CLASSIC_FLAG) { //kjgw_2020_0330
		nBytes = 20;
	} else { //RT_CAN_FD_NORMAL_FLAG;
	}*/

	tx_length = nBytes + 5; //first two bytes for sending command & address, third for size, last two bytes for CRC

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_READ_CRC << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);
    if(fromRam) {
        p->spi_tx_buf[2] = nBytes >> 2;
    } else { //kjgw_2020_0322
        p->spi_tx_buf[2] = nBytes;
    }

    //Clear data
    for(i=3; i < tx_length; i++) {
        p->spi_tx_buf[i] = 0;
    }

	set_parameter[num][13] = tx_length;
    error = SPI_Transfer_4a(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_RX_ONLY, 100);
	if(error == 123) {
		return error;
	} else if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReadByteArrayWithCRC error1 %d, %d\n",
			num, error);
		return error;
	}

    //Get CRC from controller
    crcFromSpiSlave = (unsigned short)(p->spi_rx_buf[tx_length - 2] << 8)
		+ (unsigned short)(p->spi_rx_buf[tx_length - 1]);

    //Use the receive buffer to calculate CRC
    //First three bytes need to be command
    p->spi_rx_buf[0] = p->spi_tx_buf[0];
    p->spi_rx_buf[1] = p->spi_tx_buf[1];
    p->spi_rx_buf[2] = p->spi_tx_buf[2];
    crcAtController = CHIP_CalculateCRC16(p->spi_rx_buf, nBytes + 3);

    //Compare CRC readings
    if(crcFromSpiSlave == crcAtController) { //kjgw_2020_0322
		*crcIsCorrect = true;

	 	//Update data
   		for(i=0; i < nBytes; i++) {
       		ba_rxd[num][i] = p->spi_rx_buf[i + 3];
  	 	}

    } else {
		*crcIsCorrect = false;

		//kjgw_190323 printk(KERN_WARNING
		//	"rt_can_fd: CHIP_ReadByteArrayWithCRC error2 %d, %x, %x\n",
		//	num, crcFromSpiSlave, crcAtController);
    }

	//kjgw_2020_1213 printk(KERN_WARNING
	//	"rt_can_fd: CHIP_ReadByteArrayWithCRC ok %d\n", num);

    //kjg_2020_1213 return error;
    return 0;
}

char CHIP_WriteByteArray(int num, unsigned short address, unsigned char *txd, unsigned short nBytes)
{
    char error;
    unsigned short i, tx_length = nBytes + 2;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_WRITE << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

    //Add data
    for(i=2; i < tx_length; i++) {
        p->spi_tx_buf[i] = txd[i - 2];
    }

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_ONLY, 11);

    return error;
}

char CHIP_WriteByteArray_3(int num, unsigned short address, unsigned char *txd, unsigned short nBytes)
{
    char error;
    unsigned short i, tx_length = nBytes + 2;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_WRITE << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);

    //Add data
    for(i=2; i < tx_length; i++) {
        p->spi_tx_buf[i] = txd[i - 2];
    }

    error = SPI_Transfer_3a(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_ONLY, 11);

    return error;
}

char CHIP_WriteByteArrayWithCRC(int num, unsigned short address, unsigned char *txd, unsigned short nBytes, bool fromRam)
{
    char error;
    unsigned short i, crcResult = 0, tx_length = nBytes + 5;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_WRITE_CRC << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);
    if(fromRam) {
        p->spi_tx_buf[2] = nBytes >> 2;
    } else {
        p->spi_tx_buf[2] = nBytes;
    }

    //Add data
    for(i=0; i < nBytes; i++) {
        p->spi_tx_buf[i + 3] = txd[i];
    }

    //Add CRC
    crcResult = CHIP_CalculateCRC16(p->spi_tx_buf, tx_length - 2);
    p->spi_tx_buf[tx_length - 2] = (unsigned char)((crcResult >> 8) & 0xFF);
    p->spi_tx_buf[tx_length - 1] = (unsigned char)(crcResult & 0xFF);

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_ONLY, 12);

    return error;
}

char CHIP_ReadWordArray(int num, unsigned short address, unsigned int *rxd, unsigned short nWords)
{
    char error;
    unsigned short i, j, n, tx_length = nWords * 4 + 2;
    T_CHIP_reg w;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (_CHIP_INS_READ << 4) + ((address >> 8) & 0xF);
    p->spi_tx_buf[1] = address & 0xFF;

    //Clear data
    for(i=2; i < tx_length; i++) {
        p->spi_tx_buf[i] = 0;
    }

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_RX, 13);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReadWordArray error %d, %d\n", num, error);
		return error;
	}

    //Convert Byte array to Word array
    n = 2;
    for(i=0; i < nWords; i++) {
        w.word = 0;
        for(j=0; j < 4; j++, n++) {
            w.byte[j] = p->spi_rx_buf[n];
        }
        rxd[i] = w.word;
    }

    return error;
}

char CHIP_WriteWordArray(int num, unsigned short address, unsigned int *txd, unsigned short nWords)
{
    char error;
    unsigned short i, j, n, tx_length = nWords * 4 + 2;
    T_CHIP_reg w;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (_CHIP_INS_WRITE << 4) + ((address >> 8) & 0xF);
    p->spi_tx_buf[1] = address & 0xFF;

    //Convert ByteArray to word array
    n = 2;
    for(i=0; i < nWords; i++) {
        w.word = txd[i];
        for(j=0; j < 4; j++, n++) {
            p->spi_tx_buf[n] = w.byte[j];
        }
    }

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_ONLY, 14);

    return error;
}

//Section : Configuration
char CHIP_Configure(int num)
{
    char error;
    T_CHIP_ctl ciCon;
	struct rt_can_fd_struct *p;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING "rt_can_fd: CHIP_Configure error1 %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

    ciCon.word = canControlResetValues[_CHIP_REG_CiCON / 4];

    ciCon.bF.DNetFilterCount = p->config.DNetFilterCount;
    ciCon.bF.IsoCrcEnable = p->config.IsoCrcEnable;
    ciCon.bF.ProtocolExceptionEventDisable
		= p->config.ProtocolExpectionEventDisable;
    ciCon.bF.WakeUpFilterEnable = p->config.WakeUpFilterEnable;
    ciCon.bF.WakeUpFilterTime = p->config.WakeUpFilterTime;
    ciCon.bF.BitRateSwitchDisable = p->config.BitRateSwitchDisable;
    ciCon.bF.RestrictReTxAttempts = p->config.RestrictReTxAttempts;
    ciCon.bF.EsiInGatewayMode = p->config.EsiInGatewayMode;
    ciCon.bF.SystemErrorToListenOnly = p->config.SystemErrorToListenOnly;
    ciCon.bF.StoreInTEF = p->config.StoreInTEF;
    ciCon.bF.TXQEnable = p->config.TXQEnable;
    ciCon.bF.TxBandWidthSharing = p->config.TxBandWidthSharing;

    error = CHIP_WriteWord(num, _CHIP_REG_CiCON, ciCon.word);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_Configure error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_ConfigureObjectReset(int num)
{
    T_CHIP_ctl ciCon;
	struct rt_can_fd_struct *p;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ConfigureObjectReset error %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

    ciCon.word = canControlResetValues[_CHIP_REG_CiCON / 4];

    p->config.DNetFilterCount = ciCon.bF.DNetFilterCount;
    p->config.IsoCrcEnable = ciCon.bF.IsoCrcEnable;
    p->config.ProtocolExpectionEventDisable
		= ciCon.bF.ProtocolExceptionEventDisable;
    p->config.WakeUpFilterEnable = ciCon.bF.WakeUpFilterEnable;
    p->config.WakeUpFilterTime = ciCon.bF.WakeUpFilterTime;
    p->config.BitRateSwitchDisable = ciCon.bF.BitRateSwitchDisable;
    p->config.RestrictReTxAttempts = ciCon.bF.RestrictReTxAttempts;
    p->config.EsiInGatewayMode = ciCon.bF.EsiInGatewayMode;
    p->config.SystemErrorToListenOnly = ciCon.bF.SystemErrorToListenOnly;
    p->config.StoreInTEF = ciCon.bF.StoreInTEF;
    p->config.TXQEnable = ciCon.bF.TXQEnable;
    p->config.TxBandWidthSharing = ciCon.bF.TxBandWidthSharing;

    return 0;
}

//Section : Operating mode
char CHIP_OperationModeSelect(int num, T_CHIP_opMode opMode)
{
    char error;
    unsigned char d = 0;

    error = CHIP_ReadByte(num, _CHIP_REG_CiCON + 3, &d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_OperationModeSelect error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    d &= ~0x07;
    d |= opMode;

    error = CHIP_WriteByte(num, _CHIP_REG_CiCON + 3, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_OperationModeSelect error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

T_CHIP_opMode CHIP_OperationModeGet(int num)
{
    char error;
    unsigned char d = 0;
    T_CHIP_opMode mode = CHIP_INVALID_MODE;

    //Read Opmode
    error = CHIP_ReadByte(num, _CHIP_REG_CiCON + 2, &d);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_OperationModeGet error %d\n", num);
		return CHIP_INVALID_MODE;
	}

    //Get Opmode bits
    d = (d >> 5) & 0x7;

    //Decode Opmode
    switch(d) {
        case CHIP_NORMAL_MODE:
            mode = CHIP_NORMAL_MODE;
            break;
        case CHIP_SLEEP_MODE:
            mode = CHIP_SLEEP_MODE;
            break;
        case CHIP_INT_LOOP_MODE:
            mode = CHIP_INT_LOOP_MODE;
            break;
        case CHIP_EXT_LOOP_MODE:
            mode = CHIP_EXT_LOOP_MODE;
            break;
        case CHIP_LISTEN_ONLY_MODE:
            mode = CHIP_LISTEN_ONLY_MODE;
            break;
        case CHIP_CONFIG_MODE:
            mode = CHIP_CONFIG_MODE;
            break;
        case CHIP_CLASSIC_MODE:
            mode = CHIP_CLASSIC_MODE;
            break;
        case CHIP_RESTRICT_MODE:
            mode = CHIP_RESTRICT_MODE;
            break;
        default:
            mode = CHIP_INVALID_MODE;
            break;
    }

    return mode;
}

char CHIP_LowPowerModeEnable(int num)
{ //kjg_2020_0322
	char error;

#ifdef CAN_FD_A_CHIP1
	//LPM not implemented
	error = -100;

	printk(KERN_WARNING
		"rt_can_fd: CHIP_LowPowerModeEnable error1 %d\n", num);
#else
	unsigned char d = 0;

	//Read
    error = CHIP_ReadByte(num, _CHIP_REG_OSC, &d);
	if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_LowPowerModeEnable error2 %d, %d\n", num, error);

		return -1;
	}

	//Modify
	d |= 0x08;

	//Write
	error = CHIP_WriteByte(num, _CHIP_REG_OSC, d);
	if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_LowPowerModeEnable error3 %d, %d\n", num, error);

		return -2;
	}
#endif

	return error;
}

char CHIP_LowPowerModeDisable(int num)
{ //kjg_2020_0322
	char error;

#ifdef CAN_FD_A_CHIP1
	//LPM not implemented
	error = -100;

	printk(KERN_WARNING
		"rt_can_fd: CHIP_LowPowerModeDisable error1 %d\n", num);
#else
	unsigned char d = 0;

	//Read
    error = CHIP_ReadByte(num, _CHIP_REG_OSC, &d);
	if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_LowPowerModeDisable error2 %d, %d\n", num, error);

		return -1;
	}

	//Modify
	d &= ~0x08;

	//Write
	error = CHIP_WriteByte(num, _CHIP_REG_OSC, d);
	if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_LowPowerModeDisable error3 %d, %d\n", num, error);

		return -2;
	}
#endif

	return error;
}

//Section : CAN Transmit
char CHIP_TransmitChannelConfigure(int num, T_CHIP_fifoChannel channel)
{
    char error;
    unsigned short a = 0;
    T_CHIP_fifoCtl ciFifoCon;
	struct rt_can_fd_struct *p;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelConfigure error %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

    //Setup FIFO
    ciFifoCon.word = canFifoResetValues[0];
    ciFifoCon.txBF.TxEnable = 1;
    ciFifoCon.txBF.FifoSize = p->txConfig.FifoSize;
    ciFifoCon.txBF.PayLoadSize = p->txConfig.PayLoadSize;
    ciFifoCon.txBF.TxAttempts = p->txConfig.TxAttempts;
    ciFifoCon.txBF.TxPriority = p->txConfig.TxPriority;
    ciFifoCon.txBF.RTREnable = p->txConfig.RTREnable;

    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);

    error = CHIP_WriteWord(num, a, ciFifoCon.word);

    return error;
}

char CHIP_TransmitChannelConfigureObjectReset(int num)
{
    T_CHIP_fifoCtl ciFifoCon;
	struct rt_can_fd_struct *p;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_TransmitChannelConfigureObjectReset error %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

    ciFifoCon.word = canFifoResetValues[0];

    p->txConfig.RTREnable = ciFifoCon.txBF.RTREnable;
    p->txConfig.TxPriority = ciFifoCon.txBF.TxPriority;
    p->txConfig.TxAttempts = ciFifoCon.txBF.TxAttempts;
    p->txConfig.FifoSize = ciFifoCon.txBF.FifoSize;
    p->txConfig.PayLoadSize = ciFifoCon.txBF.PayLoadSize;

    return 0;
}

char CHIP_TransmitQueueConfigure(int num, T_CHIP_txQueCfg *config)
{
    char error;
    unsigned short a = 0;
    T_CHIP_txQueCtl ciFifoCon;

    //Setup FIFO
    ciFifoCon.word = canFifoResetValues[0]; //kjg_2020_1206 0->1

    ciFifoCon.txBF.TxEnable = 1;
    ciFifoCon.txBF.FifoSize = config->FifoSize;
    ciFifoCon.txBF.PayLoadSize = config->PayLoadSize;
    ciFifoCon.txBF.TxAttempts = config->TxAttempts;
    ciFifoCon.txBF.TxPriority = config->TxPriority;

    a = _CHIP_REG_CiTXQCON;
    error = CHIP_WriteWord(num, a, ciFifoCon.word);

    return error;
}

char CHIP_TransmitQueueConfigureObjectReset(T_CHIP_txQueCfg *config)
{
    T_CHIP_fifoCtl ciFifoCon;
    //T_CHIP_txQueCtl ciFifoCon; //kjg_2020_1206

    ciFifoCon.word = canFifoResetValues[0]; //kjg_2020_1206 0->1

    config->TxPriority = ciFifoCon.txBF.TxPriority;
    config->TxAttempts = ciFifoCon.txBF.TxAttempts;
    config->FifoSize = ciFifoCon.txBF.FifoSize;
    config->PayLoadSize = ciFifoCon.txBF.PayLoadSize;

    return 0;
}

char CHIP_TransmitChannelLoad(int num, T_CHIP_fifoChannel channel, bool flush, struct rt_can_fd_struct *p, int idx)
{
    char error;
    unsigned char j=0, i, txBuffer[MAX_MSG_SIZE];
    unsigned short a, n = 0;
    unsigned int fifoReg[3], tx_length, txdNumBytes;
    T_CHIP_fifoCtl ciFifoCon;
    //kjg_2020_0322 T_CHIP_fifoStat ciFifoSta;
    __attribute__((unused)) T_CHIP_fifoStat ciFifoSta;
    T_CHIP_fifoUserCfg ciFifoUa;
	//kjg_2020_1206 long long hrt[10]; //kjg_2020_0331

    //Get FIFO registers
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);

	//kjg_2020_1206 hrt[0] = gethrtime(); //kjg_2020_0331

    error = CHIP_ReadWordArray(num, a, fifoReg, 3);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelLoad error1 %d, %d\n", num, error);
		return -1;
	}
	//kjg_2020_1206 hrt[1] = gethrtime(); //kjg_2020_0331

    //Check that it is a transmit buffer
    ciFifoCon.word = fifoReg[0];
    if(!ciFifoCon.txBF.TxEnable) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelLoad error2 %d, %d\n",
			num, ciFifoCon.txBF.TxEnable);
		return -2;
	}

	//Check that DLC is big enough for data
    tx_length = CHIP_DlcToDataBytes(p->txObj.bF.ctrl.DLC);
	txdNumBytes = tx_length;
	/*kjg_2020_0323 if(tx_length < txdNumBytes) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelLoad error3 %d, %d, %d\n",
			num, tx_length, txdNumBytes);
		return -3;
	}*/

    //Get status
    ciFifoSta.word = fifoReg[1];

    //Get address
    ciFifoUa.word = fifoReg[2];
/*kjg_2020_0326 #ifdef USERADDRESS_TIMES_FOUR
    a = 4 * ciFifoUa.bF.UserAddress;
#else*/
    a = ciFifoUa.bF.UserAddress;
//#endif
    a += _CHIP_RAMADDR_START;

    txBuffer[0] = p->txObj.byte[0];
		//not using 'for' to reduce no of instructions
    txBuffer[1] = p->txObj.byte[1];
    txBuffer[2] = p->txObj.byte[2];
    txBuffer[3] = p->txObj.byte[3];

    txBuffer[4] = p->txObj.byte[4];
    txBuffer[5] = p->txObj.byte[5];
    txBuffer[6] = p->txObj.byte[6];
    txBuffer[7] = p->txObj.byte[7];

    for(i=0; i < txdNumBytes; i++) {
        txBuffer[i + 8] = p->tx_buf[idx].data[i];
    }

    //Make sure we write a multiple of 4 bytes to RAM
    if(txdNumBytes % 4) {
        //Need to add bytes
        n = 4 - (txdNumBytes % 4);
        i = txdNumBytes + 8;

        for(j=0; j < n; j++) {
            txBuffer[i + 8 + j] = 0;
        }
    }
	//kjg_2020_1206 hrt[2] = gethrtime(); //kjg_2020_0331

    error = CHIP_WriteByteArray(num, a, txBuffer, txdNumBytes + 8 + n);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelLoad error4 %d, %d\n", num, error);
		return -3;
	}
	//kjg_2020_1206 hrt[3] = gethrtime(); //kjg_2020_0331

    //Set UINC and TXREQ
    error = CHIP_TransmitChannelUpdate(num, channel, flush);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelLoad error5 %d, %d\n", num, error);
		return -4;
	}
	/*kjg_2020_1206 hrt[4] = gethrtime(); //kjg_2020_0331

	hrt[5] = hrt[1] - hrt[0];
	hrt[6] = hrt[2] - hrt[1];
	hrt[7] = hrt[3] - hrt[2];
	hrt[8] = hrt[4] - hrt[3];
	if(hrt[5] > myData->test_val_ll2[0][num])
		myData->test_val_ll2[0][num] = hrt[5];
	if(hrt[6] > myData->test_val_ll2[1][num])
		myData->test_val_ll2[1][num] = hrt[6];
	if(hrt[7] > myData->test_val_ll2[2][num])
		myData->test_val_ll2[2][num] = hrt[7];
	if(hrt[8] > myData->test_val_ll2[3][num])
		myData->test_val_ll2[3][num] = hrt[8];*/
	
/*kjgd_180405
	printk(KERN_WARNING
		"rt_can_fd: CHIP_TransmitChannelLoad %d, %x %x %x %x %x %x %x %x\n",
		num,
		txBuffer[0], txBuffer[1], txBuffer[2], txBuffer[3],
		txBuffer[4], txBuffer[5], txBuffer[6], txBuffer[7]);
	printk(KERN_WARNING
		"rt_can_fd: CHIP_TransmitChannelLoad %d, %x %x %x %x %x %x %x %x\n",
		num,
		txBuffer[8], txBuffer[9], txBuffer[10], txBuffer[11],
		txBuffer[12], txBuffer[13], txBuffer[14], txBuffer[15]);
	printk(KERN_WARNING
		"rt_can_fd: CHIP_TransmitChannelLoad %d, %x %x %x %x %x %x %x %x\n",
		num,
		txBuffer[56], txBuffer[57], txBuffer[58], txBuffer[59],
		txBuffer[60], txBuffer[61], txBuffer[62], txBuffer[63]);
	printk(KERN_WARNING
		"rt_can_fd: CHIP_TransmitChannelLoad %d, %x %x %x %x %x %x %x %x\n",
		num,
		txBuffer[64], txBuffer[65], txBuffer[66], txBuffer[67],
		txBuffer[68], txBuffer[69], txBuffer[70], txBuffer[71]);
*/
    return error;
}

char CHIP_TransmitChannelLoad_3a(int num, T_CHIP_fifoChannel channel, bool flush, struct rt_can_fd_struct *p, int idx)
{ //kjg_2020_1212
    char error;
    unsigned char j=0, i, txBuffer[MAX_MSG_SIZE];
    unsigned short a, n=0;
    unsigned int fifoReg[3], tx_length, txdNumBytes;
    T_CHIP_fifoCtl ciFifoCon;
    //kjg_2020_0322 T_CHIP_fifoStat ciFifoSta;
    __attribute__((unused)) T_CHIP_fifoStat ciFifoSta;
    T_CHIP_fifoUserCfg ciFifoUa;

    //Get FIFO registers
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);

    error = CHIP_ReadWordArray(num, a, fifoReg, 3);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelLoad error1 %d, %d\n", num, error);
		return -1;
	}

    //Check that it is a transmit buffer
    ciFifoCon.word = fifoReg[0];
    if(!ciFifoCon.txBF.TxEnable) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelLoad error2 %d, %d\n",
			num, ciFifoCon.txBF.TxEnable);
		return -2;
	}

	//Check that DLC is big enough for data
    tx_length = CHIP_DlcToDataBytes(p->txObj.bF.ctrl.DLC);
	txdNumBytes = tx_length;
	/*kjg_2020_0323 if(tx_length < txdNumBytes) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelLoad error3 %d, %d, %d\n",
			num, tx_length, txdNumBytes);
		return -3;
	}*/

    //Get status
    ciFifoSta.word = fifoReg[1];

    //Get address
    ciFifoUa.word = fifoReg[2];
/*kjg_2020_0326 #ifdef USERADDRESS_TIMES_FOUR
    a = 4 * ciFifoUa.bF.UserAddress;
#else*/
    a = ciFifoUa.bF.UserAddress;
//#endif
    a += _CHIP_RAMADDR_START;

    txBuffer[0] = p->txObj.byte[0];
		//not using 'for' to reduce no of instructions
    txBuffer[1] = p->txObj.byte[1];
    txBuffer[2] = p->txObj.byte[2];
    txBuffer[3] = p->txObj.byte[3];

    txBuffer[4] = p->txObj.byte[4];
    txBuffer[5] = p->txObj.byte[5];
    txBuffer[6] = p->txObj.byte[6];
    txBuffer[7] = p->txObj.byte[7];

    for(i=0; i < txdNumBytes; i++) {
        txBuffer[i + 8] = p->tx_buf[idx].data[i];
    }

    //Make sure we write a multiple of 4 bytes to RAM
    if(txdNumBytes % 4) {
        //Need to add bytes
        n = 4 - (txdNumBytes % 4);
        i = txdNumBytes + 8;

        for(j=0; j < n; j++) {
            txBuffer[i + 8 + j] = 0;
        }
    }

	set_parameter[num][11] = txdNumBytes + 8 + n + 2;
    error = CHIP_WriteByteArray_3(num, a, txBuffer, txdNumBytes + 8 + n);
    //if(error) {
	//	printk(KERN_WARNING
	//		"rt_can_fd: CHIP_TransmitChannelLoad error4 %d, %d\n", num, error);
	//	return -3;
	//}

    return error;
}

char CHIP_TransmitChannelLoad_3b(int num, T_CHIP_fifoChannel channel, bool flush)
{ //kjg_2020_1212
    char error;

    //Set UINC and TXREQ
    error = CHIP_TransmitChannelUpdate(num, channel, flush);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelLoad error5 %d, %d\n", num, error);
		return -4;
	}
	
    return error;
}

char CHIP_TransmitChannelFlush(int num, T_CHIP_fifoChannel channel)
{
    char error;
    unsigned char d = 0;
    unsigned short a = 0;

    //Address of TXREQ
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
    a += 1;

    //Set TXREQ
    d = 0x02;

    error = CHIP_WriteByte(num, a, d);

    return error;
}

char CHIP_TransmitChannelStatusGet(int num, T_CHIP_fifoChannel channel, T_CHIP_txFifoStatus *status)
{
    char error;
    unsigned short a = 0;
    unsigned int sta = 0, fifoReg[2];
    T_CHIP_fifoStat ciFifoSta;
    T_CHIP_fifoCtl ciFifoCon;

    //Get FIFO registers
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);

    error = CHIP_ReadWordArray(num, a, fifoReg, 2);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelStatusGet error %d\n", num);
		return -1;
	}

    //Update data
    ciFifoCon.word = fifoReg[0];
    ciFifoSta.word = fifoReg[1];

    //Update status
    sta = ciFifoSta.byte[0];

    if(ciFifoCon.txBF.TxRequest) {
        sta |= CHIP_TX_FIFO_TRANSMITTING;
    }

    *status = (T_CHIP_txFifoStatus)(sta & CHIP_TX_FIFO_STATUS_MASK);

    return error;
}

char CHIP_TransmitChannelReset(int num, T_CHIP_fifoChannel channel)
{
    return CHIP_ReceiveChannelReset(num, channel);
}

char CHIP_TransmitChannelUpdate(int num, T_CHIP_fifoChannel channel, bool flush)
{
    char error;
    unsigned short a;
    T_CHIP_fifoCtl ciFifoCon;

    //Set UINC
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET) + 1; //Byte that contains FRESET
    ciFifoCon.word = 0;
    ciFifoCon.txBF.UINC = 1;

    //Set TXREQ
    if(flush) {
        ciFifoCon.txBF.TxRequest = 1;
    }

    error = CHIP_WriteByte(num, a, ciFifoCon.byte[1]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelUpdate error %d, %d\n", num, error);
        return -1;
    }

    return error;
}

char CHIP_TransmitRequestSet(int num, T_CHIP_txReqChannel txreq)
{
    char error;
    unsigned int w = txreq; //Write TXREQ register

    error = CHIP_WriteWord(num, _CHIP_REG_CiTXREQ, w);

    return error;
}

char CHIP_TransmitRequestGet(int num, unsigned int *txreq)
{
    char error;

    error = CHIP_ReadWord(num, _CHIP_REG_CiTXREQ, txreq);

    return error;
}

char CHIP_TransmitChannelAbort(int num, T_CHIP_fifoChannel channel)
{
    char error;
    unsigned char d;
    unsigned short a;

    //Address
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
    a += 1; // byte address of TXREQ

    //Clear TXREQ
    d = 0x00;

    error = CHIP_WriteByte(num, a, d);

    return error;
}

char CHIP_TransmitAbortAll(int num)
{
    char error;
    unsigned char d;

    //Read CiCON byte 3
    error = CHIP_ReadByte(num, (_CHIP_REG_CiCON + 3), &d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitAbortAll error1 %d, %d\n", num, error);
        return -1;
    }

    //Modify
    d |= 0x8;

    error = CHIP_WriteByte(num, (_CHIP_REG_CiCON + 3), d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitAbortAll error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_TransmitBandWidthSharingSet(int num, T_CHIP_txbws txbws)
{
    char error;
    unsigned char d = 0;

    //Read CiCON byte 3
    error = CHIP_ReadByte(num, (_CHIP_REG_CiCON + 3), &d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitBandWidthSharingSet error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    d &= 0x0f;
    d |= (txbws << 4);

    error = CHIP_WriteByte(num, (_CHIP_REG_CiCON + 3), d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitBandWidthSharingSet error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

//Section : CAN Receive
char CHIP_FilterObjectConfigure(int num, T_CHIP_filter filter)
{
    char error;
    unsigned short a;
    T_CHIP_filtObj fObj;
	struct rt_can_fd_struct *p;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_FilterObjectConfigure error %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

    //Setup
    fObj.word = 0;
    fObj.bF = p->fObj.bF;
    a = _CHIP_REG_CiFLTOBJ + (filter * _CHIP_FILTER_OFFSET);

    error = CHIP_WriteWord(num, a, fObj.word);

    return error;
}

char CHIP_FilterMaskConfigure(int num, T_CHIP_filter filter)
{
    char error;
    unsigned short a;
    T_CHIP_maskObj mObj;
	struct rt_can_fd_struct *p;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_FilterMaskConfigure error %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

    //Setup
    mObj.word = 0;
    mObj.bF = p->mObj.bF;
    a = _CHIP_REG_CiMASK + (filter * _CHIP_FILTER_OFFSET);

    error = CHIP_WriteWord(num, a, mObj.word);

    return error;
}

char CHIP_FilterToFifoLink(int num, T_CHIP_filter filter, T_CHIP_fifoChannel channel, bool enable)
{
    char error;
    unsigned short a;
    T_CHIP_filtCtl fCtrl;
	struct rt_can_fd_struct *p;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_FilterToFifoLink error %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

    if(enable) {
        fCtrl.bF.Enable = 1;
    } else {
        fCtrl.bF.Enable = 0;
    }

    //Link
    fCtrl.bF.BufferPointer = channel;
    a = _CHIP_REG_CiFLTCON + filter;

    error = CHIP_WriteByte(num, a, fCtrl.byte);

    return error;
}

char CHIP_FilterEnable(int num, T_CHIP_filter filter)
{
    char error;
    unsigned short a;
    T_CHIP_filtCtl fCtrl;

    a = _CHIP_REG_CiFLTCON + filter;
    error = CHIP_ReadByte(num, a, &fCtrl.byte);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_FilterEnable error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    fCtrl.bF.Enable = 1;

    error = CHIP_WriteByte(num, a, fCtrl.byte);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_FilterEnable error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_FilterDisable(int num, T_CHIP_filter filter)
{
    char error;
    unsigned short a;
    T_CHIP_filtCtl fCtrl;

    a = _CHIP_REG_CiFLTCON + filter;
    error = CHIP_ReadByte(num, a, &fCtrl.byte);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_FilterDisable error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    fCtrl.bF.Enable = 0;

    error = CHIP_WriteByte(num, a, fCtrl.byte);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_FilterDisable error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_DeviceNetFilterCountSet(int num, T_CHIP_dnetFiltSize dnfc)
{
    char error;
    unsigned char d = 0;

    //Read CiCON byte 0
    error = CHIP_ReadByte(num, _CHIP_REG_CiCON, &d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_DeviceNetFilterCountSet error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    d &= 0x1f;
    d |= dnfc;

    error = CHIP_WriteByte(num, _CHIP_REG_CiCON, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_DeviceNetFilterCountSet error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_ReceiveChannelConfigure(int num, T_CHIP_fifoChannel channel)
{
    char error;
    unsigned short a = 0;
    T_CHIP_fifoCtl ciFifoCon;
	struct rt_can_fd_struct *p;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelConfigure error1 %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

    if(channel == CAN_TXQUEUE_CH0) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelConfigure error2 %d\n", num);
		return -100;
	}

    //Setup FIFO
    ciFifoCon.word = canFifoResetValues[0];

    ciFifoCon.rxBF.TxEnable = 0;
    ciFifoCon.rxBF.FifoSize = p->rxConfig.FifoSize;
    ciFifoCon.rxBF.PayLoadSize = p->rxConfig.PayLoadSize;
    ciFifoCon.rxBF.RxTimeStampEnable = p->rxConfig.RxTimeStampEnable;

    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);

    error = CHIP_WriteWord(num, a, ciFifoCon.word);

    return error;
}

char CHIP_ReceiveChannelConfigureObjectReset(int num)
{
    T_CHIP_fifoCtl ciFifoCon;
	struct rt_can_fd_struct *p;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_ReceiveChannelConfigureObjectReset error %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

    ciFifoCon.word = canFifoResetValues[0];

    p->rxConfig.FifoSize = ciFifoCon.rxBF.FifoSize;
    p->rxConfig.PayLoadSize = ciFifoCon.rxBF.PayLoadSize;
    p->rxConfig.RxTimeStampEnable = ciFifoCon.rxBF.RxTimeStampEnable;

    return 0;
}

char CHIP_ReceiveChannelStatusGet(int num, T_CHIP_fifoChannel channel, T_CHIP_rxFifoStatus *status)
{
    char error;
    unsigned short a;
    T_CHIP_fifoStat ciFifoSta;

    ciFifoSta.word = 0;
    a = _CHIP_REG_CiFIFOSTA + (channel * _CHIP_FIFO_OFFSET);

    error = CHIP_ReadByte(num, a, &ciFifoSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelStatusGet error %d, %d\n",
			num, error);
		return -1;
	}

    //Update data
    *status = (T_CHIP_rxFifoStatus)(ciFifoSta.byte[0] & 0x0F);

    return error;
}

char CHIP_ReceiveMessageGet(int num, T_CHIP_fifoChannel channel, T_CHIP_rxMsgObj *rxObj)
{
	bool crcIsCorrect;
    char error;
    unsigned char n, i, ba[MAX_MSG_SIZE], nBytes;
    unsigned short a;
    unsigned int fifoReg[3], idx;
    T_CHIP_fifoCtl ciFifoCon;
    //kjg_2020_0323 T_CHIP_fifoStat ciFifoSta;
    __attribute__((unused)) T_CHIP_fifoStat ciFifoSta;
    T_CHIP_fifoUserCfg ciFifoUa;
    T_CHIP_reg myReg;
	struct rt_can_fd_msg tmp_msg;
    struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Get FIFO registers
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
    error = CHIP_ReadWordArray(num, a, fifoReg, 3);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error1 %d, %d\n", num, error);
		return -1;
	}

    //Check that it is a receive buffer
    ciFifoCon.word = fifoReg[0];
    if(ciFifoCon.txBF.TxEnable) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error2 %d, %d\n",
			num, ciFifoCon.txBF.TxEnable);
		return -2;
	}

    //Get Status
    ciFifoSta.word = fifoReg[1];

    //Get address
    ciFifoUa.word = fifoReg[2];
/*kjg_2020_0326 #ifdef USERADDRESS_TIMES_FOUR
    a = 4 * ciFifoUa.bF.UserAddress;
#else*/
    a = ciFifoUa.bF.UserAddress;
//#endif
    a += _CHIP_RAMADDR_START;

    //Number of bytes to read
	if(set_parameter[num][4] == 0) { //fd_flag
		nBytes = 8;
	} else {
		nBytes = MAX_DATA_BYTES; //kjgw_2020_1213
	}
    n = nBytes + 8; //Add 8 header bytes

    if(ciFifoCon.rxBF.RxTimeStampEnable) {
        n += 4; //Add 4 time stamp bytes
    }

    //Make sure we read a multiple of 4 bytes from RAM
    if(n % 4) {
        n = n + 4 - (n % 4);
    }

    //Read rxObj using one access
    if(n > MAX_MSG_SIZE) n = MAX_MSG_SIZE;

	crcIsCorrect = true;
    error = CHIP_ReadByteArrayWithCRC(num, a, ba, n, true, &crcIsCorrect);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error3 %d, %d\n", num, error);
		return -3;
	}

	if(crcIsCorrect == false) { //kjg_2020_0330
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error4 %d, %d\n", num, error);
		return -4;
	}

    //Assign message header
    myReg.byte[0] = ba[0];
    myReg.byte[1] = ba[1];
    myReg.byte[2] = ba[2];
    myReg.byte[3] = ba[3];
    rxObj->word[0] = myReg.word;

    myReg.byte[0] = ba[4];
    myReg.byte[1] = ba[5];
    myReg.byte[2] = ba[6];
    myReg.byte[3] = ba[7];
    rxObj->word[1] = myReg.word;

	//kjgw_2020_0323_s
	if(rxObj->bF.ctrl.RTR) tmp_msg.type = MSGTYPE_RTR;
	else tmp_msg.type = MSGTYPE_STANDARD;
	if(rxObj->bF.ctrl.IDE == 0) { //Standard frame
		tmp_msg.id = (unsigned int)rxObj->bF.id.SID;
	} else { //Extended frame
		tmp_msg.type |= MSGTYPE_EXTENDED;
		//kjg_181110 tmp_msg.id = (unsigned int)rxObj->bF.id.EID;
		tmp_msg.id = (unsigned int)rxObj->bF.id.EID;
		tmp_msg.id |= ((unsigned int)rxObj->bF.id.SID << 18);
	}

	if(p->rxObj.bF.ctrl.FDF == 0) {
		tmp_msg.can_fd_flag = 0;
	} else { //CAN FD frame
		tmp_msg.can_fd_flag = 1;
	}

	tmp_msg.length = (unsigned char)CHIP_DlcToDataBytes(rxObj->bF.ctrl.DLC);
	//kjgw_2020_0323_e
    if(ciFifoCon.rxBF.RxTimeStampEnable) {
        myReg.byte[0] = ba[8];
        myReg.byte[1] = ba[9];
        myReg.byte[2] = ba[10];
        myReg.byte[3] = ba[11];
        rxObj->word[2] = myReg.word;

        //Assign message data
        for(i=0; i < tmp_msg.length; i++) {
            tmp_msg.data[i] = ba[i + 12];
        }
    } else {
        rxObj->word[2] = 0;

        //Assign message data
        for(i=0; i < tmp_msg.length; i++) {
            tmp_msg.data[i] = ba[i + 8];
        }
    }

    //UINC channel
    error = CHIP_ReceiveChannelUpdate(num, channel);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error5 %d, %d\n", num, error);
		return -5;
	}

	tmp_msg.time = gethrtime();

	//step forward in fifo
	if(p->rx_msg_count >= RT_CAN_FD_RX_MSG_BUF_SIZE) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error6 %d, %d\n",
			num, p->rx_msg_count);
		return -6;
	}

	idx = p->rx_msg_write_idx;
	idx++;
	if(idx >= RT_CAN_FD_RX_MSG_BUF_SIZE) idx = 0;

	p->rx_buf[idx].id = tmp_msg.id;
	p->rx_buf[idx].type = tmp_msg.type;
	p->rx_buf[idx].length = tmp_msg.length;
	for(i=0; i < tmp_msg.length; i++) p->rx_buf[idx].data[i] = tmp_msg.data[i];
	p->rx_buf[idx].time = tmp_msg.time;
	p->rx_buf[idx].timeStamp = tmp_msg.timeStamp;

	p->rx_msg_write_idx = idx;
	p->rx_msg_count++;
/*kjg_180405
	printk(KERN_WARNING
		"rt_can_fd: rx kjg_d0 %d %x %d %d : %x %x %x\n", num, tmp_msg.id,
		tmp_msg.length, tmp_msg.can_fd_flag,
		rxObj->word[0], rxObj->word[1], rxObj->word[2]);
	printk(KERN_WARNING
		"rt_can_fd: rx kjg_d1 %x %x %x %x %x %x %x %x\n",
		tmp_msg.data[0], tmp_msg.data[1], tmp_msg.data[2], tmp_msg.data[3],
		tmp_msg.data[4], tmp_msg.data[5], tmp_msg.data[6], tmp_msg.data[7]);
	printk(KERN_WARNING
		"rt_can_fd: rx kjg_d2 %x %x %x %x %x %x %x %x\n",
		tmp_msg.data[8], tmp_msg.data[9], tmp_msg.data[10], tmp_msg.data[11],
		tmp_msg.data[12], tmp_msg.data[13], tmp_msg.data[14], tmp_msg.data[15]);
	printk(KERN_WARNING
		"rt_can_fd: rx kjg_d3 %x %x %x %x %x %x %x %x\n",
		tmp_msg.data[16], tmp_msg.data[17], tmp_msg.data[18], tmp_msg.data[19],
		tmp_msg.data[20], tmp_msg.data[21], tmp_msg.data[22], tmp_msg.data[23]);

	printk(KERN_WARNING
		"rt_can_fd: rx kjg_d4 %x %x %x %x %x %x %x %x\n",
		tmp_msg.data[56], tmp_msg.data[57], tmp_msg.data[58], tmp_msg.data[59],
		tmp_msg.data[60], tmp_msg.data[61], tmp_msg.data[62], tmp_msg.data[63]);
*/
    return error;
}

char CHIP_ReceiveMessageGet_4(int num, T_CHIP_fifoChannel channel, T_CHIP_rxMsgObj *rxObj)
{
	bool crcIsCorrect;
    char error;
    //unsigned char n, i, ba[MAX_MSG_SIZE], nBytes;
    unsigned char n, i, nBytes;
    unsigned short a;
    unsigned int fifoReg[3], idx;
    T_CHIP_fifoCtl ciFifoCon;
    //kjg_2020_0323 T_CHIP_fifoStat ciFifoSta;
    __attribute__((unused)) T_CHIP_fifoStat ciFifoSta;
    T_CHIP_fifoUserCfg ciFifoUa;
    T_CHIP_reg myReg;
	struct rt_can_fd_msg tmp_msg;
    struct rt_can_fd_struct *p;
	//long long hrt[4];

	//hrt[0] = gethrtime();

	p = &(rt_can_fd_table[num]);

    //Get FIFO registers
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
    error = CHIP_ReadWordArray(num, a, fifoReg, 3);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error1 %d, %d\n", num, error);
		return -1;
	}

    //Check that it is a receive buffer
    ciFifoCon.word = fifoReg[0];
    if(ciFifoCon.txBF.TxEnable) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error2 %d, %d\n",
			num, ciFifoCon.txBF.TxEnable);
		return -2;
	}

    //Get Status
    ciFifoSta.word = fifoReg[1];

    //Get address
    ciFifoUa.word = fifoReg[2];
/*kjg_2020_0326 #ifdef USERADDRESS_TIMES_FOUR
    a = 4 * ciFifoUa.bF.UserAddress;
#else*/
    a = ciFifoUa.bF.UserAddress;
//#endif
    a += _CHIP_RAMADDR_START;

    //Number of bytes to read
	if(set_parameter[num][4] == 0) { //fd_flag
		nBytes = 8;
	} else {
		nBytes = MAX_DATA_BYTES; //kjgw_2020_1213
	}	
    n = nBytes + 8; //Add 8 header bytes

    if(ciFifoCon.rxBF.RxTimeStampEnable) {
        n += 4; //Add 4 time stamp bytes
    }
	set_parameter[num][14] = ciFifoCon.rxBF.RxTimeStampEnable;

    //Make sure we read a multiple of 4 bytes from RAM
    if(n % 4) {
        n = n + 4 - (n % 4);
    }

    //Read rxObj using one access
    if(n > MAX_MSG_SIZE) n = MAX_MSG_SIZE;

	//hrt[1] = gethrtime();

	crcIsCorrect = true;
    error = CHIP_ReadByteArrayWithCRC_4(num, a, n, true, &crcIsCorrect);
    if(error == 123) {
		//hrt[2] = gethrtime();
		//myData->test_val_ll2[2][num] = hrt[1] - hrt[0];
		//myData->test_val_ll2[2][num + 8] = hrt[2] - hrt[0];

		return error;
	} else if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error3 %d, %d\n", num, error);
		return -3;
	}

	if(crcIsCorrect == false) { //kjg_2020_0330
		//kjgw_2020_1213 printk(KERN_WARNING
		//	"rt_can_fd: CHIP_ReceiveMessageGet error4 %d, %d\n", num, error);
		return -4;
	}

    //Assign message header
    myReg.byte[0] = ba_rxd[num][0]; //ba[0];
    myReg.byte[1] = ba_rxd[num][1]; //ba[1];
    myReg.byte[2] = ba_rxd[num][2]; //ba[2];
    myReg.byte[3] = ba_rxd[num][3]; //ba[3];
    rxObj->word[0] = myReg.word;

    myReg.byte[0] = ba_rxd[num][4]; //ba[4];
    myReg.byte[1] = ba_rxd[num][5]; //ba[5];
    myReg.byte[2] = ba_rxd[num][6]; //ba[6];
    myReg.byte[3] = ba_rxd[num][7]; //ba[7];
    rxObj->word[1] = myReg.word;

	//kjgw_2020_0323_s
	if(rxObj->bF.ctrl.RTR) tmp_msg.type = MSGTYPE_RTR;
	else tmp_msg.type = MSGTYPE_STANDARD;
	if(rxObj->bF.ctrl.IDE == 0) { //Standard frame
		tmp_msg.id = (unsigned int)rxObj->bF.id.SID;
	} else { //Extended frame
		tmp_msg.type |= MSGTYPE_EXTENDED;
		//kjg_181110 tmp_msg.id = (unsigned int)rxObj->bF.id.EID;
		tmp_msg.id = (unsigned int)rxObj->bF.id.EID;
		tmp_msg.id |= ((unsigned int)rxObj->bF.id.SID << 18);
	}

	if(p->rxObj.bF.ctrl.FDF == 0) {
		tmp_msg.can_fd_flag = 0;
	} else { //CAN FD frame
		tmp_msg.can_fd_flag = 1;
	}

	tmp_msg.length = (unsigned char)CHIP_DlcToDataBytes(rxObj->bF.ctrl.DLC);
	//kjgw_2020_0323_e
    if(ciFifoCon.rxBF.RxTimeStampEnable) {
        myReg.byte[0] = ba_rxd[num][8]; //ba[8];
        myReg.byte[1] = ba_rxd[num][9]; //ba[9];
        myReg.byte[2] = ba_rxd[num][10]; //ba[10];
        myReg.byte[3] = ba_rxd[num][11]; //ba[11];
        rxObj->word[2] = myReg.word;

        //Assign message data
        for(i=0; i < tmp_msg.length; i++) {
            tmp_msg.data[i] = ba_rxd[num][i + 12]; //ba[i + 12];
        }
    } else {
        rxObj->word[2] = 0;

        //Assign message data
        for(i=0; i < tmp_msg.length; i++) {
            tmp_msg.data[i] = ba_rxd[num][i + 8]; //ba[i + 8];
        }
    }

    //UINC channel
    error = CHIP_ReceiveChannelUpdate(num, channel);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error5 %d, %d\n", num, error);
		return -5;
	}

	tmp_msg.time = gethrtime();

	//step forward in fifo
	if(p->rx_msg_count >= RT_CAN_FD_RX_MSG_BUF_SIZE) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error6 %d, %d\n",
			num, p->rx_msg_count);
		return -6;
	}

	idx = p->rx_msg_write_idx;
	idx++;
	if(idx >= RT_CAN_FD_RX_MSG_BUF_SIZE) idx = 0;

	p->rx_buf[idx].id = tmp_msg.id;
	p->rx_buf[idx].type = tmp_msg.type;
	p->rx_buf[idx].length = tmp_msg.length;
	for(i=0; i < tmp_msg.length; i++) p->rx_buf[idx].data[i] = tmp_msg.data[i];
	p->rx_buf[idx].time = tmp_msg.time;
	p->rx_buf[idx].timeStamp = tmp_msg.timeStamp;

	p->rx_msg_write_idx = idx;
	p->rx_msg_count++;

	printk(KERN_WARNING
		"rt_can_fd: CHIP_ReceiveMessageGet ok %d\n", num);

    //kjg_2020_1213 return error;
    return 100;
}

char CHIP_ReceiveChannelReset(int num, T_CHIP_fifoChannel channel)
{
    char error;
    unsigned short a = 0;
    T_CHIP_fifoCtl ciFifoCon;

    //Address and data
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET) + 1; //Byte that contains FRESET
    ciFifoCon.word = 0;
    ciFifoCon.rxBF.FRESET = 1;

    error = CHIP_WriteByte(num, a, ciFifoCon.byte[1]);

    return error;
}

char CHIP_ReceiveChannelUpdate(int num, T_CHIP_fifoChannel channel)
{
    char error;
    unsigned short a;
    T_CHIP_fifoCtl ciFifoCon;

    ciFifoCon.word = 0;

    //Set UINC
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET) + 1;
		//Byte that contains FRESET
    ciFifoCon.rxBF.UINC = 1;

    error = CHIP_WriteByte(num, a, ciFifoCon.byte[1]);

    return error;
}

//Section : Transmit Event FIFO
char CHIP_TefStatusGet(int num, T_CHIP_tefFifoStatus *status)
{
    char error;
    unsigned short a = 0;
    T_CHIP_tef ciTefSta;

    ciTefSta.word = 0;
    a = _CHIP_REG_CiTEFSTA;
    error = CHIP_ReadByte(num, a, &ciTefSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefStatusGet error %d, %d\n", num, error);
		return -1;
	}

    //Update data
    *status = (T_CHIP_tefFifoStatus)(ciTefSta.byte[0]
		& CHIP_TEF_FIFO_STATUS_MASK);

    return error;
}

char CHIP_TefMessageGet(int num, T_CHIP_tefMsgObj *tefObj)
{
    char error;
    unsigned char n = 0, ba[12];
    unsigned short a = 0;
    unsigned int fifoReg[3];
    T_CHIP_reg myReg;
    T_CHIP_tefCtl ciTefCon;
    //kjg_2020_0323 T_CHIP_tef ciTefSta;
    __attribute__((unused)) T_CHIP_tef ciTefSta;
    T_CHIP_fifoUserCfg ciTefUa;

    //Get FIFO registers
    a = _CHIP_REG_CiTEFCON;
    error = CHIP_ReadWordArray(num, a, fifoReg, 3);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefMessageGet error1 %d, %d\n", num, error);
		return -1;
	}

    //Get control
    ciTefCon.word = fifoReg[0];

    //Get status
    ciTefSta.word = fifoReg[1];

    //Get address
    ciTefUa.word = fifoReg[2];
/*kjg_2020_0326 #ifdef USERADDRESS_TIMES_FOUR
    a = 4 * ciTefUa.bF.UserAddress;
#else*/
    a = ciTefUa.bF.UserAddress;
//#endif
    a += _CHIP_RAMADDR_START;

    //Number of bytes to read
    n = 8; //8 header bytes

    if(ciTefCon.bF.TimeStampEnable) {
        n += 4; //Add 4 time stamp bytes
    }

    //Read rxObj using one access
    error = CHIP_ReadByteArray(num, a, ba, n);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefMessageGet erro2 %d, %d\n", num, error);
		return -2;
	}

    //Assign message header
    myReg.byte[0] = ba[0];
    myReg.byte[1] = ba[1];
    myReg.byte[2] = ba[2];
    myReg.byte[3] = ba[3];
    tefObj->word[0] = myReg.word;

    myReg.byte[0] = ba[4];
    myReg.byte[1] = ba[5];
    myReg.byte[2] = ba[6];
    myReg.byte[3] = ba[7];
    tefObj->word[1] = myReg.word;

    if(ciTefCon.bF.TimeStampEnable) {
        myReg.byte[0] = ba[8];
        myReg.byte[1] = ba[9];
        myReg.byte[2] = ba[10];
        myReg.byte[3] = ba[11];
        tefObj->word[2] = myReg.word;
    } else {
        tefObj->word[2] = 0;
    }

    //Set UINC
    error = CHIP_TefUpdate(num);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefMessageGet error3 %d, %d\n", num, error);
		return -3;
	}

    return error;
}

char CHIP_TefReset(int num)
{
    char error;
    unsigned short a = 0;
    T_CHIP_tefCtl ciTefCon;

    //Set FRESET
    a = _CHIP_REG_CiTEFCON + 1;
    ciTefCon.word = 0;
    ciTefCon.bF.FRESET = 1;

    error = CHIP_WriteByte(num, a, ciTefCon.byte[1]);

    return error;
}

char CHIP_TefUpdate(int num)
{
    char error;
    unsigned short a = 0;
    T_CHIP_tefCtl ciTefCon;

    //Set UINC
    a = _CHIP_REG_CiTEFCON + 1;
    ciTefCon.word = 0;
    ciTefCon.bF.UINC = 1;

    error = CHIP_WriteByte(num, a, ciTefCon.byte[1]);

    return error;
}

char CHIP_TefConfigure(int num, T_CHIP_tefCfg *config)
{
    char error;
    T_CHIP_tefCtl ciTefCon;

    //Setup FIFO
    ciTefCon.word = canFifoResetValues[0]; //kjg_2020_1206 0->2

    ciTefCon.bF.FifoSize = config->FifoSize;
    ciTefCon.bF.TimeStampEnable = config->TimeStampEnable;

    error = CHIP_WriteWord(num, _CHIP_REG_CiTEFCON, ciTefCon.word);

    return error;
}

char CHIP_TefConfigureObjectReset(T_CHIP_tefCfg *config)
{
    T_CHIP_tefCtl ciTefCon;

    ciTefCon.word = canFifoResetValues[0]; //kjg_2020_1206 0->2

    config->FifoSize = ciTefCon.bF.FifoSize;
    config->TimeStampEnable = ciTefCon.bF.TimeStampEnable;

    return 0;
}

//Section : Module Events
char CHIP_ModuleEventGet(int num, T_CHIP_moduleEvent *flags)
{
    char error;
    T_CHIP_intFlag intFlags;

    //Read Interrupt flags
    intFlags.word = 0;
    error = CHIP_ReadHalfWord(num, _CHIP_REG_CiINTFLAG, &intFlags.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ModuleEventGet error %d, %d\n", num, error);
		return -1;
	}

    //Update data
    *flags = (T_CHIP_moduleEvent)(intFlags.word & CHIP_ALL_EVENTS);

    return error;
}

char CHIP_ModuleEventEnable(int num, T_CHIP_moduleEvent flags)
{
    char error;
    unsigned short a = 0;
    T_CHIP_intCfg intEnables;

    //Read Interrupt Enables
    a = _CHIP_REG_CiINTENABLE;
    intEnables.word = 0;
    error = CHIP_ReadHalfWord(num, a, &intEnables.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ModuleEventEnable error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    intEnables.word |= (flags & CHIP_ALL_EVENTS);

    error = CHIP_WriteHalfWord(num, a, intEnables.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ModuleEventEnable error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_ModuleEventDisable(int num, T_CHIP_moduleEvent flags)
{
    char error;
    unsigned short a = 0;
    T_CHIP_intCfg intEnables;

    //Read Interrupt Enables
    a = _CHIP_REG_CiINTENABLE;
    intEnables.word = 0;
    error = CHIP_ReadHalfWord(num, a, &intEnables.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ModuleEventDisable error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    intEnables.word &= ~(flags & CHIP_ALL_EVENTS);

    error = CHIP_WriteHalfWord(num, a, intEnables.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ModuleEventDisable error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_ModuleEventClear(int num, T_CHIP_moduleEvent flags)
{
    char error;
    unsigned short a = 0;
    T_CHIP_intFlag intFlags;

    //Read Interrupt flags
    a = _CHIP_REG_CiINTFLAG;
    intFlags.word = 0;

    //Write 1 to all flags except the ones that we want to clear
    //Writing a 1 will not set the flag
    //Only writing a 0 will clear it
    //The flags are HS/C
    intFlags.word = CHIP_ALL_EVENTS;
    intFlags.word &= ~flags;

    error = CHIP_WriteHalfWord(num, a, intFlags.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ModuleEventClear error %d, %d\n", num, error);
		return -1;
	}

    return error;
}

char CHIP_ModuleEventRxCodeGet(int num, T_CHIP_rxcode *rxCode)
{
    char error;
    unsigned char rxCodeByte = 0;
    unsigned short a = 0;

    a = _CHIP_REG_CiVEC + 3;
    error = CHIP_ReadByte(num, a, &rxCodeByte);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ModuleEventRxCodeGet error %d, %d\n", num, error);
		return -1;
	}

    //Decode data
    //0x40 = "no interrupt" (CHIP_FIFO_CIVEC_NOINTERRUPT)
    if((rxCodeByte < CHIP_RXCODE_TOTAL_CHANNELS)
		|| (rxCodeByte == CHIP_RXCODE_NO_INT)) {
        *rxCode = (T_CHIP_rxcode)rxCodeByte;
    } else {
        *rxCode = CHIP_RXCODE_RESERVED; //shouldn't get here
    }

    return error;
}

char CHIP_ModuleEventTxCodeGet(int num, T_CHIP_txcode *txCode)
{
    char error;
    unsigned char txCodeByte = 0;
    unsigned short a = 0;

    a = _CHIP_REG_CiVEC + 2;
    error = CHIP_ReadByte(num, a, &txCodeByte);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ModuleEventTxCodeGet error %d, %d\n", num, error);
		return -1;
	}

    //Decode data_
    //0x40 = "no interrupt" (CHIP_FIFO_CIVEC_NOINTERRUPT)
    if((txCodeByte < CHIP_TXCODE_TOTAL_CHANNELS)
		|| (txCodeByte == CHIP_TXCODE_NO_INT)) {
        *txCode = (T_CHIP_txcode)txCodeByte;
    } else {
        *txCode = CHIP_TXCODE_RESERVED; //shouldn't get here
    }

    return error;
}

char CHIP_ModuleEventFilterHitGet(int num, T_CHIP_filter *filterHit)
{
    char error;
    unsigned char filterHitByte = 0;
    unsigned short a = 0;

    a = _CHIP_REG_CiVEC + 1;
    error = CHIP_ReadByte(num, a, &filterHitByte);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ModuleEventFilterHitGet error %d, %d\n",
			num, error);
		return -1;
	}

    //Update data
    *filterHit = (T_CHIP_filter)filterHitByte;

    return error;
}

char CHIP_ModuleEventIcodeGet(int num, T_CHIP_icode *icode)
{
    char error;
    unsigned char icodeByte = 0;
    unsigned short a = 0;

    a = _CHIP_REG_CiVEC;
    error = CHIP_ReadByte(num, a, &icodeByte);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ModuleEventIcodeGet error %d, %d\n", num, error);
		return -1;
	}

    //Decode
    if((icodeByte < CHIP_ICODE_RESERVED)
		&& ((icodeByte < CHIP_ICODE_TOTAL_CHANNELS)
		|| (icodeByte >= CHIP_ICODE_NO_INT))) {
        *icode = (T_CHIP_icode)icodeByte;
    } else {
        *icode = CHIP_ICODE_RESERVED; //shouldn't get here
    }

    return error;
}

//Section : Transmit FIFO Events
char CHIP_TransmitChannelEventGet(int num, T_CHIP_fifoChannel channel, T_CHIP_txFifoEvent *flags)
{
    char error;
    unsigned short a = 0;
    T_CHIP_fifoStat ciFifoSta;

    //Read Interrupt flags
    ciFifoSta.word = 0;
    a = _CHIP_REG_CiFIFOSTA + (channel * _CHIP_FIFO_OFFSET);
    error = CHIP_ReadByte(num, a, &ciFifoSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelEventGet error %d, %d\n",
			num, error);
		return -1;
	}

    //Update data
    *flags = (T_CHIP_txFifoEvent)(ciFifoSta.byte[0] & CHIP_TX_FIFO_ALL_EVENTS);

    return error;
}

char CHIP_TransmitEventGet(int num, unsigned int *txif)
{
    char error;

    error = CHIP_ReadWord(num, _CHIP_REG_CiTXIF, txif);

    return error;
}

char CHIP_TransmitEventAttemptGet(int num, unsigned int *txatif)
{
    char error;

    error = CHIP_ReadWord(num, _CHIP_REG_CiTXATIF, txatif);

    return error;
}

char CHIP_TransmitChannelIndexGet(int num, T_CHIP_fifoChannel channel, unsigned char *idx)
{
    char error;
    unsigned short a = 0;
    T_CHIP_fifoStat ciFifoSta;

    //Read index
    ciFifoSta.word = 0;
    a = _CHIP_REG_CiFIFOSTA + (channel * _CHIP_FIFO_OFFSET);
    error = CHIP_ReadWord(num, a, &ciFifoSta.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelIndexGet error %d, %d\n",
			num, error);
		return -1;
	}

    //Update data
    *idx = ciFifoSta.txBF.FifoIndex;

    return error;
}

char CHIP_TransmitChannelEventEnable(int num, T_CHIP_fifoChannel channel, T_CHIP_txFifoEvent flags)
{
    char error;
    unsigned short a = 0;
    T_CHIP_fifoCtl ciFifoCon;

    //Read Interrupt Enables
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
    ciFifoCon.word = 0;
    error = CHIP_ReadByte(num, a, &ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelEventEnable error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciFifoCon.byte[0] |= (flags & CHIP_TX_FIFO_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelEventEnable error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_TransmitChannelEventDisable(int num, T_CHIP_fifoChannel channel, T_CHIP_txFifoEvent flags)
{
    char error;
    unsigned short a = 0;
    T_CHIP_fifoCtl ciFifoCon;

    //Read Interrupt Enables
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
    ciFifoCon.word = 0;
    error = CHIP_ReadByte(num, a, &ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelEventDisable error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciFifoCon.byte[0] &= ~(flags & CHIP_TX_FIFO_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelEventDisable error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_TransmitChannelEventAttemptClear(int num, T_CHIP_fifoChannel channel)
{
    char error;
    unsigned short a = 0;
    T_CHIP_fifoStat ciFifoSta;
    
    //Read Interrupt Enables
    a = _CHIP_REG_CiFIFOSTA + (channel * _CHIP_FIFO_OFFSET);
    ciFifoSta.word = 0;
    error = CHIP_ReadByte(num, a, &ciFifoSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelEventAttemptClear error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciFifoSta.byte[0] &= ~CHIP_TX_FIFO_ATTEMPTS_EXHAUSTED_EVENT;

    error = CHIP_WriteByte(num, a, ciFifoSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelEventAttemptClear error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

//Section : Receive FIFO Events
char CHIP_ReceiveChannelEventGet(int num, T_CHIP_fifoChannel channel, T_CHIP_rxFifoEvent *flags)
{
    char error;
    unsigned short a=0;
    T_CHIP_fifoStat ciFifoSta;

    if(channel == CAN_TXQUEUE_CH0) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventGet error1) %d, %d\n",
			num, channel);
		return -100;
	}

    //Read Interrupt flags
    ciFifoSta.word = 0;
    a = _CHIP_REG_CiFIFOSTA + (channel * _CHIP_FIFO_OFFSET);
    error = CHIP_ReadByte(num, a, &ciFifoSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventGet error2 %d, %d\n",
			num, error);
		return -1;
	}

    //Update data
    *flags = (T_CHIP_rxFifoEvent)(ciFifoSta.byte[0] & CHIP_RX_FIFO_ALL_EVENTS);

    return error;
}

char CHIP_ReceiveEventGet(int num, unsigned int *rxif)
{
    char error;

    error = CHIP_ReadWord(num, _CHIP_REG_CiRXIF, rxif);

    return error;
}

char CHIP_ReceiveEventOverflowGet(int num, unsigned int *rxovif)
{
    char error;

    error = CHIP_ReadWord(num, _CHIP_REG_CiRXOVIF, rxovif);

    return error;
}

char CHIP_ReceiveChannelIndexGet(int num, T_CHIP_fifoChannel channel, unsigned char *idx)
{
    return CHIP_TransmitChannelIndexGet(num, channel, idx);
}

char CHIP_ReceiveChannelEventEnable(int num, T_CHIP_fifoChannel channel, T_CHIP_rxFifoEvent flags)
{
    char error;
    unsigned short a = 0;
    T_CHIP_fifoCtl ciFifoCon;

    if(channel == CAN_TXQUEUE_CH0) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventEnable error1 %d\n", num);
		return -100;
	}

    //Read Interrupt Enables
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
    ciFifoCon.word = 0;
    error = CHIP_ReadByte(num, a, &ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventEnable error2 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciFifoCon.byte[0] |= (flags & CHIP_RX_FIFO_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventEnable error3 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_ReceiveChannelEventDisable(int num, T_CHIP_fifoChannel channel, T_CHIP_rxFifoEvent flags)
{
    char error;
    unsigned short a = 0;
    T_CHIP_fifoCtl ciFifoCon;

    if(channel == CAN_TXQUEUE_CH0) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventDisable error1 %d, %d\n",
			num, channel);
		return -100;
	}

    //Read Interrupt Enables
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
    ciFifoCon.word = 0;
    error = CHIP_ReadByte(num, a, &ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventDisable error2 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciFifoCon.byte[0] &= ~(flags & CHIP_RX_FIFO_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventDisable error3 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_ReceiveChannelEventOverflowClear(int num, T_CHIP_fifoChannel channel)
{
    char error;
    unsigned short a = 0;
    T_CHIP_fifoStat ciFifoSta;

    if(channel == CAN_TXQUEUE_CH0) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_ReceiveChannelEventOverflowClear error1 %d,%d\n",
			num, channel);
		return -100;
	}

    //Read Interrupt Flags
    ciFifoSta.word = 0;
    a = _CHIP_REG_CiFIFOSTA + (channel * _CHIP_FIFO_OFFSET);
    error = CHIP_ReadByte(num, a, &ciFifoSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_ReceiveChannelEventOverflowClear error2 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciFifoSta.byte[0] &= ~(CHIP_RX_FIFO_OVERFLOW_EVENT);

    error = CHIP_WriteByte(num, a, ciFifoSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_ReceiveChannelEventOverflowClear error3 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

//Section : Transmit Event FIFO Events
char CHIP_TefEventGet(int num, T_CHIP_tefFifoEvent *flags)
{
    char error;
    unsigned short a = 0;
    T_CHIP_tef ciTefSta;

    //Read Interrupt flags
    ciTefSta.word = 0;
    a = _CHIP_REG_CiTEFSTA;
    error = CHIP_ReadByte(num, a, &ciTefSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefEventGet error %d, %d\n", num, error);
		return -1;
	}

    //Update data
    *flags = (T_CHIP_tefFifoEvent)(ciTefSta.byte[0] & CHIP_TEF_FIFO_ALL_EVENTS);

    return error;
}

char CHIP_TefEventEnable(int num, T_CHIP_tefFifoEvent flags)
{
    char error;
    unsigned short a = 0;
    T_CHIP_tefCtl ciTefCon;

    //Read Interrupt Enables
    a = _CHIP_REG_CiTEFCON;
    ciTefCon.word = 0;
    error = CHIP_ReadByte(num, a, &ciTefCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefEventEnable error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    ciTefCon.byte[0] |= (flags & CHIP_TEF_FIFO_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, ciTefCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefEventEnable error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_TefEventDisable(int num, T_CHIP_tefFifoEvent flags)
{
    char error;
    unsigned short a = 0;
    T_CHIP_tefCtl ciTefCon;

    //Read Interrupt Enables
    a = _CHIP_REG_CiTEFCON;
    ciTefCon.word = 0;
    error = CHIP_ReadByte(num, a, &ciTefCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefEventDisable error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    ciTefCon.byte[0] &= ~(flags & CHIP_TEF_FIFO_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, ciTefCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefEventDisable error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_TefEventOverflowClear(int num)
{
    char error;
    unsigned short a = 0;
    T_CHIP_tef ciTefSta;

    //Read Interrupt Flags
    ciTefSta.word = 0;
    a = _CHIP_REG_CiTEFSTA;
    error = CHIP_ReadByte(num, a, &ciTefSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefEventOverflowClear error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciTefSta.byte[0] &= ~(CHIP_TEF_FIFO_OVERFLOW_EVENT);

    error = CHIP_WriteByte(num, a, ciTefSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefEventOverflowClear error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

//Section : Error Handling
char CHIP_ErrorCountTransmitGet(int num, unsigned char *tec)
{
    char error;
    unsigned short a = 0;

    //Read Error count
    a = _CHIP_REG_CiTREC + 1;
    error = CHIP_ReadByte(num, a, tec);

    return error;
}

char CHIP_ErrorCountReceiveGet(int num, unsigned char *rec)
{
    char error;
    unsigned short a = 0;

    //Read Error count
    a = _CHIP_REG_CiTREC;
    error = CHIP_ReadByte(num, a, rec);

    return error;
}

char CHIP_ErrorStateGet(int num, T_CHIP_errorState *flags)
{
    char error;
    unsigned char f = 0;
    unsigned short a;

    //Read Error state
    a = _CHIP_REG_CiTREC + 2;
    error = CHIP_ReadByte(num, a, &f);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ErrorStateGet error %d, %d\n", num, error);
		return -1;
	}

    //Update data
    *flags = (T_CHIP_errorState)(f & CHIP_ERROR_ALL);

    return error;
}

char CHIP_ErrorCountStateGet(int num, unsigned char *tec, unsigned char *rec, T_CHIP_errorState *flags)
{
    char error;
    unsigned short a = 0;
    T_CHIP_trec ciTrec;

    //Read Error
    a = _CHIP_REG_CiTREC;
    ciTrec.word = 0;
    error = CHIP_ReadWord(num, a, &ciTrec.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ErrorCountStateGet error %d, %d\n", num, error);
		return -1;
	}

    //Update data
    *tec = ciTrec.byte[1];
    *rec = ciTrec.byte[0];
    *flags = (T_CHIP_errorState)(ciTrec.byte[2] & CHIP_ERROR_ALL);

    return error;
}

char CHIP_BusDiagnosticsGet(int num, T_CHIP_busDiag *bd)
{
    char error;
    unsigned short a = 0;
    unsigned int w[2];
    T_CHIP_busDiag b;

    //Read diagnostic registers all in one shot
    a = _CHIP_REG_CiBDIAG0;
    error = CHIP_ReadWordArray(num, a, w, 2);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BusDiagnosticsGet error %d, %d\n", num, error);
		return -1;
	}

    //Update data
    b.word[0] = w[0];
    b.word[1] = w[1] & 0x0000ffff;
    b.word[2] = (w[1] >> 16) & 0x0000ffff;
    *bd = b;

    return error;
}

char CHIP_BusDiagnosticsClear(int num)
{
    char error;
    unsigned char a = 0;
    unsigned int w[2];

    //Clear diagnostic registers all in one shot
    a = _CHIP_REG_CiBDIAG0;
    w[0] = 0;
    w[1] = 0;

    error = CHIP_WriteWordArray(num, a, w, 2);

    return error;
}

//Section : ECC
char CHIP_EccEnable(int num)
{
    char error;
    unsigned char d = 0;

    error = CHIP_ReadByte(num, _CHIP_REG_ECCCON, &d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccEnable error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    d |= 0x01;

    error = CHIP_WriteByte(num, _CHIP_REG_ECCCON, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccEnable error2 %d, %d\n", num, error);
		return -2;
	}

    return 0;
}

char CHIP_EccDisable(int num)
{
    char error;
    unsigned char d = 0;

    error = CHIP_ReadByte(num, _CHIP_REG_ECCCON, &d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccDisable error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    d &= ~0x01;

    error = CHIP_WriteByte(num, _CHIP_REG_ECCCON, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccDisable error2 %d, %d\n", num, error);
		return -2;
	}

    return 0;
}

char CHIP_EccEventGet(int num, T_CHIP_eccEvent *flags)
{
    char error;
    unsigned char eccStatus = 0;
    unsigned short a;

    //Read Interrupt flags
    a = _CHIP_REG_ECCSTA;
    error = CHIP_ReadByte(num, a, &eccStatus);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccEventGet error %d, %d\n", num, error);
		return -1;
	}

    //Update data
    *flags = (T_CHIP_eccEvent)(eccStatus & CHIP_ECC_ALL_EVENTS);

    return error;
}

char CHIP_EccParitySet(int num, unsigned char parity)
{
    char error;

    error = CHIP_WriteByte(num, _CHIP_REG_ECCCON + 1, parity);

    return error;
}

char CHIP_EccParityGet(int num, unsigned char *parity)
{
    char error;

    error = CHIP_ReadByte(num, _CHIP_REG_ECCCON + 1, parity);

    return error;
}

char CHIP_EccErrorAddressGet(int num, unsigned short *a)
{
    char error;
    T_CHIP_eccSta reg;

    error = CHIP_ReadWord(num, _CHIP_REG_ECCSTA, &reg.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccErrorAddressGet error %d, %d\n", num, error);
		return -1;
	}

    //Update data
    *a = reg.bF.ErrorAddress;

    return error;
}

char CHIP_EccEventEnable(int num, T_CHIP_eccEvent flags)
{
    char error;
    unsigned char eccInterrupts = 0;
    unsigned short a;

    a = _CHIP_REG_ECCCON;
    error = CHIP_ReadByte(num, a, &eccInterrupts);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccErrorEnable error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    eccInterrupts |= (flags & CHIP_ECC_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, eccInterrupts);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccErrorEnable error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_EccEventDisable(int num, T_CHIP_eccEvent flags)
{
    char error;
    unsigned char eccInterrupts = 0;
    unsigned short a;

    a = _CHIP_REG_ECCCON;
    error = CHIP_ReadByte(num, a, &eccInterrupts);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccErrorDisable error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    eccInterrupts &= ~(flags & CHIP_ECC_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, eccInterrupts);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccErrorDisable error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_EccEventClear(int num, T_CHIP_eccEvent flags)
{
    char error;
    unsigned char eccStat = 0;
    unsigned short a;

    a = _CHIP_REG_ECCSTA;
    error = CHIP_ReadByte(num, a, &eccStat);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccEventClear error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    eccStat &= ~(flags & CHIP_ECC_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, eccStat);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccEventClear error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

//Section : CRC
char CHIP_CrcEventEnable(int num, T_CHIP_crcEvent flags)
{
    char error;
    unsigned char crc;
    unsigned short a;

    //Read interrupt control bits of CRC Register
    a = _CHIP_REG_CRC + 3;
    error = CHIP_ReadByte(num, a, &crc);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_CrcEventEnable error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    crc |= (flags & CHIP_CRC_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, crc);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_CrcEventEnable error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_CrcEventDisable(int num, T_CHIP_crcEvent flags)
{
    char error;
    unsigned char crc;
    unsigned short a;

    //Read interrupt control bits of CRC Register
    a = _CHIP_REG_CRC + 3;
    error = CHIP_ReadByte(num, a, &crc);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_CrcEventDisable error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    crc &= ~(flags & CHIP_CRC_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, crc);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_CrcEventDisable error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_CrcEventClear(int num, T_CHIP_crcEvent flags)
{
    char error;
    unsigned char crc;
    unsigned short a;

    //Read interrupt flags of CRC Register
    a = _CHIP_REG_CRC + 2;
    error = CHIP_ReadByte(num, a, &crc);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_CrcEventClear error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    crc &= ~(flags & CHIP_CRC_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, crc);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_CrcEventClear error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_CrcEventGet(int num, T_CHIP_crcEvent *flags)
{
    char error;
    unsigned char crc;
    unsigned short a;

    //Read interrupt flags of CRC Register
    a = _CHIP_REG_CRC + 2;
    error = CHIP_ReadByte(num, a, &crc);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_CrcEventGet error %d, %d\n", num, error);
		return -1;
	}

    //Update data
    *flags = (T_CHIP_crcEvent)(crc & CHIP_CRC_ALL_EVENTS);

    return error;
}

char CHIP_CrcValueGet(int num, unsigned short *crc)
{
    char error;

    //Read CRC value from CRC Register
    error = CHIP_ReadHalfWord(num, _CHIP_REG_CRC, crc);

    return error;
}

char CHIP_RamInit(int num, unsigned char d)
{
    char error;
    unsigned char txd[CHIP_SPI_BUFFER_LENGTH];
    unsigned short a = _CHIP_RAMADDR_START;
    unsigned int k;
	//long long hrt[24];

    //Prepare data
    for(k=0; k < CHIP_SPI_BUFFER_LENGTH; k++) txd[k] = d;

	//hrt[0] = gethrtime();
    for(k=0; k < (_CHIP_RAM_SIZE / CHIP_SPI_BUFFER_LENGTH); k++) {
        error = CHIP_WriteByteArray(num, a, txd, CHIP_SPI_BUFFER_LENGTH);
        if(error) {
			printk(KERN_WARNING
				"rt_can_fd: CHIP_RamInit error %d, %d\n", num, error);
			return -1;
		}

        a += CHIP_SPI_BUFFER_LENGTH;
		//hrt[k+1] = gethrtime();
    }

/*	rtl_printf("kjgd_2020_1212_3 %d\n", num);
	for(a=0; a < k; a++) {
		rtl_printf("(%d)%lld ", a, hrt[a+1]-hrt[a]);
	}
	rtl_printf("end\n");
*/
    return error;
}

char CHIP_RamInit_3(int num, unsigned char d, int phase)
{ //kjg_2020_1212
    char error;
    unsigned char txd[CHIP_SPI_BUFFER_LENGTH];
    unsigned short a;
    unsigned int k;

    //Prepare data
    for(k=0; k < CHIP_SPI_BUFFER_LENGTH; k++) txd[k] = d;

	a = _CHIP_RAMADDR_START + (CHIP_SPI_BUFFER_LENGTH * (phase - 2));

    error = CHIP_WriteByteArray(num, a, txd, CHIP_SPI_BUFFER_LENGTH);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_RamInit error %d, %d\n", num, error);
		return -1;
	}

    return error;
}

//Section : Time Stamp
char CHIP_TimeStampEnable(int num)
{
    char error;
    unsigned char d = 0;

    error = CHIP_ReadByte(num, _CHIP_REG_CiTSCON + 2, &d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TimeStampEnable error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    d |= 0x01;

    error = CHIP_WriteByte(num, _CHIP_REG_CiTSCON + 2, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TimeStampEnable error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_TimeStampDisable(int num)
{
    char error;
    unsigned char d = 0;

    error = CHIP_ReadByte(num, _CHIP_REG_CiTSCON + 2, &d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TimeStampDisable error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    d &= 0x06;

    error = CHIP_WriteByte(num, _CHIP_REG_CiTSCON + 2, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TimeStampDisable error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_TimeStampGet(int num, unsigned int *ts)
{
    char error;

    error = CHIP_ReadWord(num, _CHIP_REG_CiTBC, ts);

    return error;
}

char CHIP_TimeStampSet(int num, unsigned int ts)
{
    char error;

    error = CHIP_WriteWord(num, _CHIP_REG_CiTBC, ts);

    return error;
}

char CHIP_TimeStampModeConfigure(int num, T_CHIP_tsMode mode)
{
    char error;
    unsigned char d = 0;

    error = CHIP_ReadByte(num, _CHIP_REG_CiTSCON + 2, &d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TimeStampModeConfigure error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    d &= 0x01;
    d |= mode << 1;

    error = CHIP_WriteByte(num, _CHIP_REG_CiTSCON + 2, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TimeStampModeConfigure error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_TimeStampPrescalerSet(int num, unsigned short ps)
{
    char error;

    error = CHIP_WriteHalfWord(num, _CHIP_REG_CiTSCON, ps);

    return error;
}

//Section : Oscillator and Bit Time
char CHIP_OscillatorEnable(int num)
{
    char error;
    unsigned char d = 0;

    error = CHIP_ReadByte(num, _CHIP_REG_OSC, &d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_OscillatorEnable error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    d &= ~0x4;

    error = CHIP_WriteByte(num, _CHIP_REG_OSC, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_OscillatorEnable error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_OscillatorControlSet(int num, T_CHIP_oscCtrl ctrl)
{
    char error;
    T_CHIP_osc osc;

    osc.word = 0;

    osc.bF.PllEnable = ctrl.PllEnable;
    osc.bF.OscDisable = ctrl.OscDisable;
    osc.bF.SCLKDIV = ctrl.SclkDivide;
    osc.bF.CLKODIV = ctrl.ClkOutDivide;
#ifndef CAN_FD_A_CHIP1
	osc.bF.LowPowerModeEnable = ctrl.LowPowerModeEnable;
#endif

    error = CHIP_WriteByte(num, _CHIP_REG_OSC, osc.byte[0]);

    return error;
}

char CHIP_OscillatorControlObjectReset(T_CHIP_oscCtrl *ctrl)
{
    T_CHIP_osc osc;

    osc.word = chipControlResetValues[0];

    ctrl->PllEnable = osc.bF.PllEnable;
    ctrl->OscDisable = osc.bF.OscDisable;
    ctrl->SclkDivide = osc.bF.SCLKDIV;
    ctrl->ClkOutDivide = osc.bF.CLKODIV;

    return 0;
}

char CHIP_OscillatorStatusGet(int num, T_CHIP_oscSta *status)
{
    char error;
    T_CHIP_oscSta stat;
    T_CHIP_osc osc;
    
    osc.word = 0;
    error = CHIP_ReadByte(num, _CHIP_REG_OSC + 1, &osc.byte[1]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_OscillatorStatusGet error %d, %d\n", num, error);
		return -1;
	}

    stat.PllReady = osc.bF.PllReady;
    stat.OscReady = osc.bF.OscReady;
    stat.SclkReady = osc.bF.SclkReady;

    *status = stat;

    return error;
}

char CHIP_BitTimeConfigure(int num, T_CHIP_sspMode sspMode, T_CHIP_sysClkSpeed clk)
{
    char error;

    //Decode clk
    switch(clk) {
        case CHIP_SYSCLK_40M:
            error = CHIP_BitTimeConfigureNominal40MHz(num);
            if(error) return error;

            error = CHIP_BitTimeConfigureData40MHz(num, sspMode);
            break;
        case CHIP_SYSCLK_20M:
            error = CHIP_BitTimeConfigureNominal20MHz(num);
            if(error) return error;

            error = CHIP_BitTimeConfigureData20MHz(num, sspMode);
            break;
        case CHIP_SYSCLK_10M:
            error = CHIP_BitTimeConfigureNominal10MHz(num);
            if(error) return error;

            error = CHIP_BitTimeConfigureData10MHz(num, sspMode);
            break;
        default:
			printk(KERN_WARNING
				"rt_can_fd: CHIP_BitTimeConfigure error %d, %d\n", num, clk);
            error = -1;
            break;
    }

    return error;
}

char CHIP_BitTimeConfigureNominal40MHz(int num)
{
    char error;
    T_CHIP_nbtCfg ciNbtcfg;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    ciNbtcfg.word = canControlResetValues[_CHIP_REG_CiNBTCFG / 4];

    switch(p->selectedBitTime) { //Arbitration Bit rate
        case CHIP_125K_500K:
            ciNbtcfg.bF.BRP = 0;
            ciNbtcfg.bF.TSEG1 = 254;
            ciNbtcfg.bF.TSEG2 = 63;
            ciNbtcfg.bF.SJW = 63;
            break;
        case CHIP_250K_500K:
        case CHIP_250K_833K:
        case CHIP_250K_1M:
        case CHIP_250K_1M5:
        case CHIP_250K_2M:
        case CHIP_250K_3M:
        case CHIP_250K_4M:
            ciNbtcfg.bF.BRP = 0;
            ciNbtcfg.bF.TSEG1 = 126;
            ciNbtcfg.bF.TSEG2 = 31;
            ciNbtcfg.bF.SJW = 31;
            break;
        case CHIP_500K_1M:
        case CHIP_500K_2M:
        case CHIP_500K_3M:
        case CHIP_500K_4M:
        case CHIP_500K_5M:
        case CHIP_500K_6M7:
        case CHIP_500K_8M:
        case CHIP_500K_10M:
            ciNbtcfg.bF.BRP = 0;
            ciNbtcfg.bF.TSEG1 = 62;
            ciNbtcfg.bF.TSEG2 = 15;
            ciNbtcfg.bF.SJW = 15;
            break;
        case CHIP_1000K_4M:
        case CHIP_1000K_8M:
            ciNbtcfg.bF.BRP = 0;
            ciNbtcfg.bF.TSEG1 = 30;
            ciNbtcfg.bF.TSEG2 = 7;
            ciNbtcfg.bF.SJW = 7;
            break;
        default:
			printk(KERN_WARNING
				"rt_can_fd: CHIP_BitTimeConfigureNominal40MHz error %d, %d\n",
				num, p->selectedBitTime);
            return -1;
            break;
    }

    //Write Bit time registers
    error = CHIP_WriteWord(num, _CHIP_REG_CiNBTCFG, ciNbtcfg.word);

	printk(KERN_INFO
		"rt_can_fd: CHIP_BitTimeConfigureNominal40MHz %d, %d, %x, %d\n",
		num, p->selectedBitTime, ciNbtcfg.word, error);

    return error;
}

char CHIP_BitTimeConfigureData40MHz(int num, T_CHIP_sspMode sspMode)
{
    char error;
    unsigned int tdcValue = 0;
    T_CHIP_dbtCfg ciDbtcfg;
    T_CHIP_tdcCfg ciTdc;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);
    
    //sspMode;
    ciDbtcfg.word = canControlResetValues[_CHIP_REG_CiDBTCFG / 4];
    ciTdc.word = 0;

    //Configure Bit time and sample point
    ciTdc.bF.TDCMode = CHIP_SSP_MODE_AUTO;

    switch(p->selectedBitTime) { //Data Bit rate and SSP
        case CHIP_125K_500K:
        case CHIP_250K_500K:
			//Data BR
            ciDbtcfg.bF.BRP = 1;
            ciDbtcfg.bF.TSEG1 = 30;
            ciDbtcfg.bF.TSEG2 = 7;
            ciDbtcfg.bF.SJW = 7;
            //SSP
            ciTdc.bF.TDCOffset = 31;
            ciTdc.bF.TDCValue = tdcValue;
            ciTdc.bF.TDCMode = CHIP_SSP_MODE_OFF;
            break;
        case CHIP_250K_833K:
			//Data BR
            ciDbtcfg.bF.BRP = 1;
            ciDbtcfg.bF.TSEG1 = 17;
            ciDbtcfg.bF.TSEG2 = 4;
            ciDbtcfg.bF.SJW = 4;
            //SSP
            ciTdc.bF.TDCOffset = 18;
            ciTdc.bF.TDCValue = tdcValue;
            ciTdc.bF.TDCMode = CHIP_SSP_MODE_OFF;
            break;
        case CHIP_250K_1M:
        case CHIP_500K_1M:
			//Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 30;
            ciDbtcfg.bF.TSEG2 = 7;
            ciDbtcfg.bF.SJW = 7;
            //SSP
            ciTdc.bF.TDCOffset = 31;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_250K_1M5:
			//Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 18;
            ciDbtcfg.bF.TSEG2 = 5;
            ciDbtcfg.bF.SJW = 5;
            //SSP
            ciTdc.bF.TDCOffset = 19;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_250K_2M:
        case CHIP_500K_2M:
			//Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 14;
            ciDbtcfg.bF.TSEG2 = 3;
            ciDbtcfg.bF.SJW = 3;
            //SSP
            ciTdc.bF.TDCOffset = 15;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_250K_3M:
        case CHIP_500K_3M:
			//Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 8;
            ciDbtcfg.bF.TSEG2 = 2;
            ciDbtcfg.bF.SJW = 2;
            //SSP
            ciTdc.bF.TDCOffset = 9;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_250K_4M:
        case CHIP_500K_4M:
        case CHIP_1000K_4M:
			//Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 6;
            ciDbtcfg.bF.TSEG2 = 1;
            ciDbtcfg.bF.SJW = 1;
            //SSP
            ciTdc.bF.TDCOffset = 7;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_500K_5M:
			//Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 4;
            ciDbtcfg.bF.TSEG2 = 1;
            ciDbtcfg.bF.SJW = 1;
            //SSP
            ciTdc.bF.TDCOffset = 5;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_500K_6M7:
			//Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 3;
            ciDbtcfg.bF.TSEG2 = 0;
            ciDbtcfg.bF.SJW = 0;
            //SSP
            ciTdc.bF.TDCOffset = 4;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_500K_8M:
        case CHIP_1000K_8M:
			//Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 2;
            ciDbtcfg.bF.TSEG2 = 0;
            ciDbtcfg.bF.SJW = 0;
            //SSP
            ciTdc.bF.TDCOffset = 3;
            ciTdc.bF.TDCValue = 1;
            break;
        case CHIP_500K_10M:
			//Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 1;
            ciDbtcfg.bF.TSEG2 = 0;
            ciDbtcfg.bF.SJW = 0;
            //SSP
            ciTdc.bF.TDCOffset = 2;
            ciTdc.bF.TDCValue = 0;
            break;
        default:
			printk(KERN_WARNING
				"rt_can_fd: CHIP_BitTimeConfigureData40MHz error1 %d, %d\n",
				num, p->selectedBitTime);
            return -1;
            break;
    }

    //Write Bit time registers
    error = CHIP_WriteWord(num, _CHIP_REG_CiDBTCFG, ciDbtcfg.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureData40MHz error2 %d, %d, %d\n",
			num, p->selectedBitTime, error);
		return -2;
	}

    //Write Transmitter Delay Compensation
#ifdef REV_A
    ciTdc.bF.TDCOffset = 0;
    ciTdc.bF.TDCValue = 0;
#endif

    error = CHIP_WriteWord(num, _CHIP_REG_CiTDC, ciTdc.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureData40MHz error3 %d, %d, %d\n",
			num, p->selectedBitTime, error);
		return -3;
	}

	printk(KERN_INFO
		"rt_can_fd: CHIP_BitTimeConfigureData40MHz %d, %d, %x, %x\n",
		num, p->selectedBitTime, ciDbtcfg.word, ciTdc.word);

    return error;
}

char CHIP_BitTimeConfigureNominal20MHz(int num)
{
    char error;
    T_CHIP_nbtCfg ciNbtcfg;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    ciNbtcfg.word = canControlResetValues[_CHIP_REG_CiNBTCFG / 4];

    //Arbitration Bit rate
    switch(p->selectedBitTime) {
        //All 500K
        case CHIP_500K_1M:
        case CHIP_500K_2M:
        case CHIP_500K_4M:
        case CHIP_500K_5M:
        case CHIP_500K_6M7:
        case CHIP_500K_8M:
        case CHIP_500K_10M:
            ciNbtcfg.bF.BRP = 0;
            ciNbtcfg.bF.TSEG1 = 30;
            ciNbtcfg.bF.TSEG2 = 7;
            ciNbtcfg.bF.SJW = 7;
            break;
        //All 250K
        case CHIP_250K_500K:
        case CHIP_250K_833K:
        case CHIP_250K_1M:
        case CHIP_250K_1M5:
        case CHIP_250K_2M:
        case CHIP_250K_3M:
        case CHIP_250K_4M:
            ciNbtcfg.bF.BRP = 0;
            ciNbtcfg.bF.TSEG1 = 62;
            ciNbtcfg.bF.TSEG2 = 15;
            ciNbtcfg.bF.SJW = 15;
            break;
        case CHIP_1000K_4M:
        case CHIP_1000K_8M:
            ciNbtcfg.bF.BRP = 0;
            ciNbtcfg.bF.TSEG1 = 14;
            ciNbtcfg.bF.TSEG2 = 3;
            ciNbtcfg.bF.SJW = 3;
            break;
        case CHIP_125K_500K:
            ciNbtcfg.bF.BRP = 0;
            ciNbtcfg.bF.TSEG1 = 126;
            ciNbtcfg.bF.TSEG2 = 31;
            ciNbtcfg.bF.SJW = 31;
            break;
        default:
			printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureNominal20MHz error1 %d, %d\n",
				num, p->selectedBitTime);
            return -1;
            break;
    }

    //Write Bit time registers
    error = CHIP_WriteWord(num, _CHIP_REG_CiNBTCFG, ciNbtcfg.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureNominal20MHz error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_BitTimeConfigureData20MHz(int num, T_CHIP_sspMode sspMode)
{
    char error;
    unsigned int tdcValue = 0;
    T_CHIP_dbtCfg ciDbtcfg;
    T_CHIP_tdcCfg ciTdc;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);
    
    //sspMode;
    ciDbtcfg.word = canControlResetValues[_CHIP_REG_CiDBTCFG / 4];
    ciTdc.word = 0;

    //Configure Bit time and sample point
    ciTdc.bF.TDCMode = CHIP_SSP_MODE_AUTO;

    //Data Bit rate and SSP
    switch(p->selectedBitTime) {
        case CHIP_500K_1M:
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 14;
            ciDbtcfg.bF.TSEG2 = 3;
            ciDbtcfg.bF.SJW = 3;
            //SSP
            ciTdc.bF.TDCOffset = 15;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_500K_2M:
            //Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 6;
            ciDbtcfg.bF.TSEG2 = 1;
            ciDbtcfg.bF.SJW = 1;
            //SSP
            ciTdc.bF.TDCOffset = 7;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_500K_4M:
        case CHIP_1000K_4M:
            //Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 2;
            ciDbtcfg.bF.TSEG2 = 0;
            ciDbtcfg.bF.SJW = 0;
            //SSP
            ciTdc.bF.TDCOffset = 3;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_500K_5M:
            //Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 1;
            ciDbtcfg.bF.TSEG2 = 0;
            ciDbtcfg.bF.SJW = 0;
            //SSP
            ciTdc.bF.TDCOffset = 2;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_500K_6M7:
        case CHIP_500K_8M:
        case CHIP_500K_10M:
        case CHIP_1000K_8M:
            //Data Bitrate not feasible with this clock!
			printk(KERN_WARNING
				"rt_can_fd: CHIP_BitTimeConfigureData20MHz error1 %d\n", num);
            return -11;
            break;

        case CHIP_250K_500K:
        case CHIP_125K_500K:
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 30;
            ciDbtcfg.bF.TSEG2 = 7;
            ciDbtcfg.bF.SJW = 7;
            //SSP
            ciTdc.bF.TDCOffset = 31;
            ciTdc.bF.TDCValue = tdcValue;
            ciTdc.bF.TDCMode = CHIP_SSP_MODE_OFF;
            break;
        case CHIP_250K_833K:
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 17;
            ciDbtcfg.bF.TSEG2 = 4;
            ciDbtcfg.bF.SJW = 4;
            //SSP
            ciTdc.bF.TDCOffset = 18;
            ciTdc.bF.TDCValue = tdcValue;
            ciTdc.bF.TDCMode = CHIP_SSP_MODE_OFF;
            break;
        case CHIP_250K_1M:
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 14;
            ciDbtcfg.bF.TSEG2 = 3;
            ciDbtcfg.bF.SJW = 3;
            //SSP
            ciTdc.bF.TDCOffset = 15;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_250K_1M5:
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 8;
            ciDbtcfg.bF.TSEG2 = 2;
            ciDbtcfg.bF.SJW = 2;
            //SSP
            ciTdc.bF.TDCOffset = 9;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_250K_2M:
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 6;
            ciDbtcfg.bF.TSEG2 = 1;
            ciDbtcfg.bF.SJW = 1;
            //SSP
            ciTdc.bF.TDCOffset = 7;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_250K_3M:
            //Data Bitrate not feasible with this clock!
			printk(KERN_WARNING
				"rt_can_fd: CHIP_BitTimeConfigureData20MHz error2 %d\n", num);
            return -12;
            break;
        case CHIP_250K_4M:
            //Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 2;
            ciDbtcfg.bF.TSEG2 = 0;
            ciDbtcfg.bF.SJW = 0;
            //SSP
            ciTdc.bF.TDCOffset = 3;
            ciTdc.bF.TDCValue = tdcValue;
            break;

        default:
			printk(KERN_WARNING
				"rt_can_fd: CHIP_BitTimeConfigureData20MHz error3 %d, %d\n",
				num, p->selectedBitTime);
            return -1;
            break;
    }

    //Write Bit time registers
    error = CHIP_WriteWord(num, _CHIP_REG_CiDBTCFG, ciDbtcfg.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureData20MHz error4 %d, %d\n",
			num, error);
		return -2;
	}

    //Write Transmitter Delay Compensation
#ifdef REV_A
    ciTdc.bF.TDCOffset = 0;
    ciTdc.bF.TDCValue = 0;
#endif

    error = CHIP_WriteWord(num, _CHIP_REG_CiTDC, ciTdc.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureData20MHz error5 %d, %d\n",
			num, error);
		return -3;
	}

    return error;
}

char CHIP_BitTimeConfigureNominal10MHz(int num)
{
    char error;
    T_CHIP_nbtCfg ciNbtcfg;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    ciNbtcfg.word = canControlResetValues[_CHIP_REG_CiNBTCFG / 4];

    //Arbitration Bit rate
    switch(p->selectedBitTime) {
        //All 500K
        case CHIP_500K_1M:
        case CHIP_500K_2M:
        case CHIP_500K_4M:
        case CHIP_500K_5M:
        case CHIP_500K_6M7:
        case CHIP_500K_8M:
        case CHIP_500K_10M:
            ciNbtcfg.bF.BRP = 0;
            ciNbtcfg.bF.TSEG1 = 14;
            ciNbtcfg.bF.TSEG2 = 3;
            ciNbtcfg.bF.SJW = 3;
            break;
        //All 250K
        case CHIP_250K_500K:
        case CHIP_250K_833K:
        case CHIP_250K_1M:
        case CHIP_250K_1M5:
        case CHIP_250K_2M:
        case CHIP_250K_3M:
        case CHIP_250K_4M:
            ciNbtcfg.bF.BRP = 0;
            ciNbtcfg.bF.TSEG1 = 30;
            ciNbtcfg.bF.TSEG2 = 7;
            ciNbtcfg.bF.SJW = 7;
            break;
        case CHIP_1000K_4M:
        case CHIP_1000K_8M:
            ciNbtcfg.bF.BRP = 0;
            ciNbtcfg.bF.TSEG1 = 7;
            ciNbtcfg.bF.TSEG2 = 2;
            ciNbtcfg.bF.SJW = 2;
            break;
        case CHIP_125K_500K:
            ciNbtcfg.bF.BRP = 0;
            ciNbtcfg.bF.TSEG1 = 62;
            ciNbtcfg.bF.TSEG2 = 15;
            ciNbtcfg.bF.SJW = 15;
            break;
        default:
			printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureNominal10MHz error1 %d, %d\n",
				num, p->selectedBitTime);
            return -1;
            break;
    }

    //Write Bit time registers
    error = CHIP_WriteWord(num, _CHIP_REG_CiNBTCFG, ciNbtcfg.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureNominal10MHz error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_BitTimeConfigureData10MHz(int num, T_CHIP_sspMode sspMode)
{
    char error;
    unsigned int tdcValue = 0;
    T_CHIP_dbtCfg ciDbtcfg;
    T_CHIP_tdcCfg ciTdc;
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //sspMode;
    ciDbtcfg.word = canControlResetValues[_CHIP_REG_CiDBTCFG / 4];
    ciTdc.word = 0;

    //Configure Bit time and sample point
    ciTdc.bF.TDCMode = CHIP_SSP_MODE_AUTO;

    //Data Bit rate and SSP
    switch(p->selectedBitTime) {
        case CHIP_500K_1M:
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 6;
            ciDbtcfg.bF.TSEG2 = 1;
            ciDbtcfg.bF.SJW = 1;
            //SSP
            ciTdc.bF.TDCOffset = 7;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_500K_2M:
            //Data BR
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 2;
            ciDbtcfg.bF.TSEG2 = 0;
            ciDbtcfg.bF.SJW = 0;
            //SSP
            ciTdc.bF.TDCOffset = 3;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_500K_4M:
        case CHIP_500K_5M:
        case CHIP_500K_6M7:
        case CHIP_500K_8M:
        case CHIP_500K_10M:
        case CHIP_1000K_4M:
        case CHIP_1000K_8M:
            //Data Bitrate not feasible with this clock!
			printk(KERN_WARNING
				"rt_can_fd: CHIP_BitTimeConfigureData10MHz error1 %d\n", num);
            return -11;
            break;
        case CHIP_250K_500K:
        case CHIP_125K_500K:
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 14;
            ciDbtcfg.bF.TSEG2 = 3;
            ciDbtcfg.bF.SJW = 3;
            //SSP
            ciTdc.bF.TDCOffset = 15;
            ciTdc.bF.TDCValue = tdcValue;
            ciTdc.bF.TDCMode = CHIP_SSP_MODE_OFF;
            break;
        case CHIP_250K_833K:
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 7;
            ciDbtcfg.bF.TSEG2 = 2;
            ciDbtcfg.bF.SJW = 2;
            //SSP
            ciTdc.bF.TDCOffset = 8;
            ciTdc.bF.TDCValue = tdcValue;
            ciTdc.bF.TDCMode = CHIP_SSP_MODE_OFF;
            break;
        case CHIP_250K_1M:
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 6;
            ciDbtcfg.bF.TSEG2 = 1;
            ciDbtcfg.bF.SJW = 1;
            //SSP
            ciTdc.bF.TDCOffset = 7;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_250K_1M5:
            //Data Bitrate not feasible with this clock!
			printk(KERN_WARNING
				"rt_can_fd: CHIP_BitTimeConfigureData10MHz error2 %d\n", num);
            return -12;
            break;
        case CHIP_250K_2M:
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 2;
            ciDbtcfg.bF.TSEG2 = 0;
            ciDbtcfg.bF.SJW = 0;
            //SSP
            ciTdc.bF.TDCOffset = 3;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_250K_3M:
        case CHIP_250K_4M:
            //Data Bitrate not feasible with this clock!
			printk(KERN_WARNING
				"rt_can_fd: CHIP_BitTimeConfigureData10MHz error3 %d\n", num);
            return -13;
            break;
        default:
			printk(KERN_WARNING
				"rt_can_fd: CHIP_BitTimeConfigureData10MHz error4 %d, %d\n",
				num, p->selectedBitTime);
            return -1;
            break;
    }

    //Write Bit time registers
    error = CHIP_WriteWord(num, _CHIP_REG_CiDBTCFG, ciDbtcfg.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureData10MHz error5 %d, %d\n",
			num, error);
		return -2;
	}

    //Write Transmitter Delay Compensation
#ifdef REV_A
    ciTdc.bF.TDCOffset = 0;
    ciTdc.bF.TDCValue = 0;
#endif

    error = CHIP_WriteWord(num, _CHIP_REG_CiTDC, ciTdc.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureData10MHz error6 %d, %d\n",
			num, error);
		return -3;
	}

    return error;
}

//Section : GPIO
char CHIP_GpioModeConfigure(int num, T_CHIP_pinMode gpio0, T_CHIP_pinMode gpio1)
{
    char error;
    unsigned short a;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON + 3;
    iocon.word = 0;
    error = CHIP_ReadByte(num, a, &iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioModeConfigure error1 %d, %d\n", num, error);
		return -1;
	}

    //Modify
    iocon.bF.PinMode0 = gpio0;
    iocon.bF.PinMode1 = gpio1;

    error = CHIP_WriteByte(num, a, iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioModeConfigure error2 %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioDirectionConfigure(int num, T_CHIP_pinDir gpio0, T_CHIP_pinDir gpio1)
{
    char error;
    unsigned short a;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON;
    iocon.word = 0;
    error = CHIP_ReadByte(num, a, &iocon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioDirectionConfigure error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    iocon.bF.TRIS0 = gpio0;
    iocon.bF.TRIS1 = gpio1;

    error = CHIP_WriteByte(num, a, iocon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioDirectionConfigure error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioStandbyControlEnable(int num)
{
    char error;
    unsigned short a;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON;
    iocon.word = 0;
    error = CHIP_ReadByte(num, a, &iocon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioStandbyControlEnable error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    iocon.bF.XcrSTBYEnable = 1;

    error = CHIP_WriteByte(num, a, iocon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioStandbyControlEnable error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioStandbyControlDisable(int num)
{
    char error;
    unsigned short a;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON;
    iocon.word = 0;
    error = CHIP_ReadByte(num, a, &iocon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioStandbyControlDisable error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    iocon.bF.XcrSTBYEnable = 0;

    error = CHIP_WriteByte(num, a, iocon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioStandbyControlDisable error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioInterruptPinsOpenDrainConfigure(int num, T_CHIP_odm mode)
{
    char error;
    unsigned short a;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON + 3;
    iocon.word = 0;
    error = CHIP_ReadByte(num, a, &iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_GpioInterruptPinsOpenDrainConfigure error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    iocon.bF.INTPinOpenDrain = mode;

    error = CHIP_WriteByte(num, a, iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_GpioInterruptPinsOpenDrainConfigure error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioTransmitPinOpenDrainConfigure(int num, T_CHIP_odm mode)
{
    char error;
    unsigned short a;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON + 3;
    iocon.word = 0;
    error = CHIP_ReadByte(num, a, &iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_GpioTransmitPinsOpenDrainConfigure error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    iocon.bF.TXCANOpenDrain = mode;

    error = CHIP_WriteByte(num, a, iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_GpioTransmitPinsOpenDrainConfigure error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioPinSet(int num, T_CHIP_pinPos pos, T_CHIP_pinState latch)
{
    char error;
    unsigned short a;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON + 1;
    iocon.word = 0;
    error = CHIP_ReadByte(num, a, &iocon.byte[1]);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_GpioPinSet error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    switch(pos) {
        case CHIP_PIN_0:
            iocon.bF.LAT0 = latch;
            break;
        case CHIP_PIN_1:
            iocon.bF.LAT1 = latch;
            break;
        default:
			printk(KERN_WARNING "rt_can_fd: CHIP_GpioPinSet error2 %d, %d\n",
				num, pos);
            return -1;
            break;
    }

    error = CHIP_WriteByte(num, a, iocon.byte[1]);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_GpioPinSet errror3 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioPinRead(int num, T_CHIP_pinPos pos, T_CHIP_pinState *state)
{
    char error;
    unsigned short a;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON + 2;
    iocon.word = 0;
    error = CHIP_ReadByte(num, a, &iocon.byte[2]);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_GpioPinRead error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Update data
    switch(pos) {
        case CHIP_PIN_0:
            *state = (T_CHIP_pinState)iocon.bF.GPIO0;
            break;
        case CHIP_PIN_1:
            *state = (T_CHIP_pinState)iocon.bF.GPIO1;
            break;
        default:
			printk(KERN_WARNING
				"rt_can_fd: CHIP_GpioPinRead error2 %d, %d\n", num, pos);
            return -2;
            break;
    }

    return error;
}

char CHIP_GpioClockOutputConfigure(int num, T_CHIP_clkoMode mode)
{
    char error;
    unsigned short a;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON + 3;
    iocon.word = 0;
    error = CHIP_ReadByte(num, a, &iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioClockOutputConfigure error1 %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    iocon.bF.SOFOutputEnable = mode;

    error = CHIP_WriteByte(num, a, iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioClockOutputConfigure error2 %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

//Section : Miscellaneous
unsigned int CHIP_DlcToDataBytes(T_CHIP_dlc dlc)
{
    unsigned int dataBytesInObject;

	/*kjg_2020_0323 Nop();
	Nop();*/

    if(dlc < CHIP_DLC_12) {
        dataBytesInObject = dlc;
    } else {
        switch(dlc) {
            case CHIP_DLC_12: dataBytesInObject = 12; break;
            case CHIP_DLC_16: dataBytesInObject = 16; break;
            case CHIP_DLC_20: dataBytesInObject = 20; break;
            case CHIP_DLC_24: dataBytesInObject = 24; break;
            case CHIP_DLC_32: dataBytesInObject = 32; break;
            case CHIP_DLC_48: dataBytesInObject = 48; break;
            case CHIP_DLC_64: dataBytesInObject = 64; break;
            default: dataBytesInObject = 0; break;
        }
    }

    return dataBytesInObject;
}

char CHIP_FifoIndexGet(int num, T_CHIP_fifoChannel channel, unsigned char *mi)
{
    char error;
    unsigned char b = 0;
    unsigned short a;

    //Read Status register
    a = _CHIP_REG_CiFIFOSTA + (channel * _CHIP_FIFO_OFFSET);
    a += 1; //byte[1]
    error = CHIP_ReadByte(num, a, &b);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_FifoIndexGet error %d, %d\n",
			num, error);
		return -1;
	}

    //Update data
    *mi = b & 0x1f;

    return error;
}

unsigned short CHIP_CalculateCRC16(unsigned char *data_, unsigned short size)
{
    unsigned char index;
    unsigned short init = CRCBASE;

    while(size-- != 0) {
        index = ((unsigned char *)&init)[CRCUPPER] ^ *data_++;
        init = (init << 8) ^ crc16Table[index];
    }

    return init;
}

T_CHIP_dlc CHIP_DataBytesToDlc(unsigned char n)
{
    T_CHIP_dlc dlc;

    if(n <= 8) {
		switch(n) {
			case 0: dlc = CHIP_DLC_0; break;
			case 1: dlc = CHIP_DLC_1; break;
			case 2: dlc = CHIP_DLC_2; break;
			case 3: dlc = CHIP_DLC_3; break;
			case 4: dlc = CHIP_DLC_4; break;
			case 5: dlc = CHIP_DLC_5; break;
			case 6: dlc = CHIP_DLC_6; break;
			case 7: dlc = CHIP_DLC_7; break;
			case 8: dlc = CHIP_DLC_8; break;
			default: dlc = CHIP_DLC_0; break;
		}
    } else if(n <= 12) {
        dlc = CHIP_DLC_12;
    } else if(n <= 16) {
        dlc = CHIP_DLC_16;
    } else if(n <= 20) {
        dlc = CHIP_DLC_20;
    } else if(n <= 24) {
        dlc = CHIP_DLC_24;
    } else if(n <= 32) {
        dlc = CHIP_DLC_32;
    } else if(n <= 48) {
        dlc = CHIP_DLC_48;
    } else if(n <= 64) {
        dlc = CHIP_DLC_64;
    } else {
		dlc = CHIP_DLC_0;
	}

    return dlc;
}

//Request port memory and register ISRs, if we cannot get the memory
//of all ports, release all already requested ports and return an error.
//return Success status, zero on success.
/*kjg_181121 int init_module(void)
{
	unsigned char info[6];
    int errorcode=0, i, j;
    struct rt_can_fd_struct *p;

	terminal_resistor[0] = terminal_resistor[1] = 0x0F;
	max_can_fd_ch = 0;
	info[0] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_INFO_1);
	info[1] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_INFO_2);
	info[2] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_INFO_3);
	if(info[0] == 0x41
		&& (info[1] >= 0x30 && info[1] <= 0x39)
		&& (info[2] >= 0x30 && info[2] <= 0x39)) {
		max_can_fd_ch = 4;
	}

	info[3] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_INFO_1);
	info[4] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_INFO_2);
	info[5] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_INFO_3);
	if(info[3] == 0x41
		&& (info[4] >= 0x30 && info[4] <= 0x39)
		&& (info[5] >= 0x30 && info[5] <= 0x39)) {
		max_can_fd_ch += 4;
	}

    for(i=0; i < max_can_fd_ch; i++) {
		p = &(rt_can_fd_table[i]);
		if(p->used > 0) {
		    if(-EBUSY == check_region(p->port, 8)) {
				errorcode = 10;
				break;
	    	}

		    request_region(p->port, 8, "rt_can_fd");
		    rt_can_fd_request_irq(p->irq, p->isr);
	    	rt_can_setup(i, 1);
		}
    }

	printk(KERN_INFO "rt_can_fd: board1 %x %x %x, board2 %x %x %x, %d\n",
		info[0], info[1], info[2], info[3], info[4], info[5], max_can_fd_ch);

    if(errorcode == 0) {
		if(max_can_fd_ch == 4) {
			outb(0x0F, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_PORT_IRQ_MASK);
		} else if(max_can_fd_ch == 8) {
			outb(0xFF, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_PORT_IRQ_MASK);
		}

		printk(KERN_INFO
		"rt_can_fd: CAN_FD driver (version " VERSION ") sucessfully loaded.\n");

		return pthread_create(&thread, NULL, rt_can_task, 0);
    } else {
		printk(KERN_WARNING
			"rt_can_fd: cannot request all port regions, giving up.\n");

		outb(0x00, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_PORT_IRQ_MASK);

		for(j=0; j < i; j++) {
		    p = &(rt_can_fd_table[j]);
	    	if(p->used > 0) {
	    		rt_can_setup(i, -1);
				rt_can_fd_free_irq(p->irq);
				release_region(p->port, 8);
	    	}
		}

    	return errorcode;
    }
}

//Unregister ISR and releases memory for all ports
void cleanup_module(void)
{
    int i;
    struct rt_can_fd_struct *p;

	outb(0x00, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_PORT_IRQ_MASK);

	pthread_delete_np(thread);

    for(i=0; i < max_can_fd_ch; i++) {
		p = &(rt_can_fd_table[i]);
		if(p->used > 0) {
		    rt_can_setup(i, -1);
		    rt_can_fd_free_irq(p->irq);
	    	release_region(p->port, 8);
		}
    }

    printk(KERN_INFO "rt_can_fd: unloaded.\n");
    return;
}*/

int init_rt_can(void)
{ //kjg_181121
	unsigned char info[6];
    int errorcode=0, i, j;
    struct rt_can_fd_struct *p;

	terminal_resistor[0] = terminal_resistor[1] = 0x0F;
	max_can_fd_ch = 0;
	info[0] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_INFO_1);
	info[1] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_INFO_2);
	info[2] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD1_INFO_3);
	//0x41=A, 0x61=a, 0x74=t, 0x54=T
	if((info[0] == 0x41 || info[0] == 0x61
		|| info[0] == 0x74 || info[0] == 0x54)
		&& (info[1] >= 0x30 && info[1] <= 0x39)
		&& (info[2] >= 0x30 && info[2] <= 0x39)) {
		max_can_fd_ch = 4;
	}

	info[3] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_INFO_1);
	info[4] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_INFO_2);
	info[5] = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_BD2_INFO_3);
	if((info[3] == 0x41 || info[3] == 0x61
		|| info[3] == 0x74 || info[3] == 0x54)
		&& (info[4] >= 0x30 && info[4] <= 0x39)
		&& (info[5] >= 0x30 && info[5] <= 0x39)) {
		max_can_fd_ch += 4;
	}

    for(i=0; i < max_can_fd_ch; i++) {
		p = &(rt_can_fd_table[i]);
		if(p->used > 0) {
		    //if(-EBUSY == check_region(p->port, 8)) {
			//	errorcode = 10;
			//	break;
	    	//}

		    //request_region(p->port, 8, "rt_can_fd");
		    //rt_can_fd_request_irq(p->irq, p->isr);
	    	rt_can_setup(i, 1);
		}
    }

	printk(KERN_INFO "rt_can_fd: board1 %x %x %x, board2 %x %x %x, %d\n",
		info[0], info[1], info[2], info[3], info[4], info[5], max_can_fd_ch);

    if(errorcode == 0) {
		if(max_can_fd_ch == 4) {
			outb(0x0F, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_PORT_IRQ_MASK);
		} else if(max_can_fd_ch == 8) {
			outb(0xFF, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_PORT_IRQ_MASK);
		}

		printk(KERN_INFO
		//"rt_can_fd: CAN_FD driver (version " VERSION ") sucessfully loaded.\n");
		"rt_can_fd: CAN_FD driver (version 181121) sucessfully loaded.\n");

		return 0;//pthread_create(&thread, NULL, rt_can_task, 0);
    } else {
		printk(KERN_WARNING
			"rt_can_fd: cannot request all port regions, giving up.\n");

		outb(0x00, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_PORT_IRQ_MASK);

		for(j=0; j < i; j++) {
		    p = &(rt_can_fd_table[j]);
	    	if(p->used > 0) {
	    		rt_can_setup(i, -1);
				//rt_can_fd_free_irq(p->irq);
				//release_region(p->port, 8);
	    	}
		}

    	return errorcode;
    }
}

void cleanup_rt_can(void)
{
    int i;
    struct rt_can_fd_struct *p;

	outb(0x00, RT_CAN_FD_BASE_ADDR + RT_CAN_FD_PORT_IRQ_MASK);

	//pthread_delete_np(thread);

    for(i=0; i < max_can_fd_ch; i++) {
		p = &(rt_can_fd_table[i]);
		if(p->used > 0) {
		    rt_can_setup(i, -1);
		    //rt_can_fd_free_irq(p->irq);
	    	//release_region(p->port, 8);
		}
    }

    printk(KERN_INFO "rt_can_fd: unloaded.\n");
    return;
}
