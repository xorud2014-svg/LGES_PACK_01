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

pthread_t thread; //kjg_180521
long long tx_ht3[RT_CAN_FD_CNT]; //kjg_180522
long long rx_ht3[RT_CAN_FD_CNT]; //kjg_180522
int set_parameter[RT_CAN_FD_CNT][8]; //kjg_180523

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

void *rt_can_task(void *arg)
{ //kjg_180521
	int slot=0;
	long long ht0, ht1, ht2;//, ht3[RT_CAN_FD_CNT];
	struct sched_param sch_p;

//	ht3[0] = ht3[1] = ht3[2] = ht3[3] = 0;
//	ht3[4] = ht3[5] = ht3[6] = ht3[7] = 0;

	tx_ht3[0] = tx_ht3[1] = tx_ht3[2] = tx_ht3[3] = 0;
	tx_ht3[4] = tx_ht3[5] = tx_ht3[6] = tx_ht3[7] = 0;

	rx_ht3[0] = rx_ht3[1] = rx_ht3[2] = rx_ht3[3] = 0;
	rx_ht3[4] = rx_ht3[5] = rx_ht3[6] = rx_ht3[7] = 0;

	sch_p.sched_priority = 2;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &sch_p);
	//kjg_180522 pthread_make_periodic_np(pthread_self(), gethrtime(), 250000); //250us
	//kjg_181110 pthread_make_periodic_np(pthread_self(), gethrtime(), 1500000); //1500us
	//kjg_181120 pthread_make_periodic_np(pthread_self(), gethrtime(), 1000000); //1000us kjg_181110
	pthread_make_periodic_np(pthread_self(), gethrtime(), 250000); //250us
	pthread_setfp_np(pthread_self(), 1);

	while(1) {
		ht0 = gethrtime();
		tx_can_data(slot);

		ht1 = gethrtime();
		ht2 = ht1 - ht0;
		if(ht2 >= 2000 && ht2 > tx_ht3[slot]) {
			tx_ht3[slot] = ht2;
			printk(KERN_WARNING "rt_can_fd: time_slot(%d) %lld\n",
				slot, ht2);
		}

		slot++;
		if(slot >= RT_CAN_FD_CNT) slot = 0;
		//kjg_180522 if(slot >= 4) slot = 0;

		pthread_wait_np();
	}

	return 0;
}

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
	//kjg_180405 p->txObj.bF.ctrl.SEQ = 1;

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
					"rt_can_fd: fd_chip_write error (-1) %d, tec:%x rec:%x errorFlags:%x DLC:%d(%d)\n",
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

int rt_can_write(int num, const char *msg)
{ //kjg_180521
	int idx;
	struct rt_can_fd_struct *p;

	if(max_can_fd_ch == 0) return 0; //kjg_180405

    if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING "rt_can_fd: rt_can_write error (-1) %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

	if(p->used <= 0) {
		printk(KERN_WARNING "rt_can_fd: rt_can_write error (-2) %d, %d\n",
			num, p->used);
		return -2;
	}

	if(p->tx_msg_count >= RT_CAN_FD_TX_MSG_BUF_SIZE) {
		p->tx_msg_error++;
		if(p->tx_msg_error < 10) {
			//kjg_180522 printk(KERN_WARNING
			//	"rt_can_fd: rt_can_write error (-3) %d, %d, %d\n", num,
			//	p->tx_msg_count, p->tx_msg_error);
			return -3;
		} else {
			printk(KERN_WARNING
				"rt_can_fd: rt_can_write error (-4) %d, %d, %d\n", num,
				p->tx_msg_count, p->tx_msg_error); //kjg_180522

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
		printk(KERN_WARNING "rt_can_fd: rt_can_write error (-1) %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

	if(p->used <= 0) {
		printk(KERN_WARNING "rt_can_fd: rt_can_write error (-2) %d, %d\n",
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
//		printk(KERN_WARNING "rt_can_fd: rt_can_write error (-3) %d, %x, %x\n",
//			num, mask, tmp);
//		return -3;
//	}

	tmp = 0;
	if(p->tx_msg_count >= RT_CAN_FD_TX_MSG_BUF_SIZE) {
		p->tx_msg_error++;
		if(p->tx_msg_error < 10) {
			printk(KERN_WARNING
				"rt_can_fd: rt_can_write error (-4) %d, %d, %d\n", num,
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
				"rt_can_fd: rt_can_write error (-5) %d, %d, %d\n",
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
    //kjg_180405 p->txObj.bF.ctrl.SEQ = 1;

    do { //Check if FIFO is not full
        CHIP_TransmitChannelEventGet(num, CHIP_FIFO_CH2, &p->txFlags);

        if(attempts == 0) {
			for(delay=0; delay < DELAY_10US; delay++) {}
            CHIP_ErrorCountStateGet(num, &p->tec, &p->rec, &p->errorFlags);
//kjg_180405
//			printk(KERN_WARNING
//				"rt_can_fd: fd_chip_write error (-1) %d, tec:%x rec:%x errorFlags:%x DLC:%d(%d)\n",
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
		printk(KERN_WARNING "rt_can_fd: rt_can_read error (-1) %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

	if(p->used <= 0) {
		printk(KERN_WARNING "rt_can_fd: rt_can_read error (-2) %d, %d\n",
			num, p->used);
		return -2;
	}
	if(p->rx_msg_count == 0) {
		//kjg_180405 printk(KERN_WARNING "rt_can_fd: rt_can_read error (-3a) %d, %d\n",
		//	num, p->rx_msg_count);
		return -3;
	} else if(p->rx_msg_count < 0) { //kjg_181110
		p->rx_msg_count = 0;
		printk(KERN_WARNING "rt_can_fd: rt_can_read error (-3b) %d, %d\n",
			num, p->rx_msg_count);
		return -3;
	}

	if(p->rx_msg_write_idx == p->rx_msg_read_idx) {
		//kjg_180522 printk(KERN_WARNING "rt_can_fd: rt_can_read error (-4) %d, %d, %d\n",
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
		printk(KERN_WARNING "rt_can_fd: rt_can_read error (-1) %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

	if(p->used <= 0) {
		printk(KERN_WARNING "rt_can_fd: rt_can_read error (-2) %d, %d\n",
			num, p->used);
		return -2;
	}
	if(p->rx_msg_count <= 0) {
		//kjg_180405 printk(KERN_WARNING "rt_can_fd: rt_can_read error (-3) %d, %d\n",
		//	num, p->rx_msg_count);
		return -3;
	}

	if(p->rx_msg_write_idx == p->rx_msg_read_idx) {
		printk(KERN_WARNING "rt_can_fd: rt_can_read error (-4) %d, %d, %d\n",
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
//		printk(KERN_WARNING "rt_can_fd: rt_can_read error (-5) %d, %x, %x\n",
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
static unsigned int rt_can_fd_share_isr(unsigned int irq_no, struct pt_regs *r)
{
	unsigned char data, tmp;
	int i, j;
	long long ht0, ht1, ht2;

	j = 0;
	while((data = inb(RT_CAN_FD_BASE_ADDR + RT_CAN_FD_PORT_IRQ)) != 0) {
		tmp = 0x01;
		//kjg_180405 for(i=0; i < RT_CAN_FD_CNT; i++) {
		for(i=0; i < max_can_fd_ch; i++) {
			if((data & tmp) != 0x00) {
				ht0 = gethrtime();
				rt_can_fd_isr((unsigned int)i, NULL);

				ht1 = gethrtime();
				ht2 = ht1 - ht0;
				if(ht2 >= rx_ht3[i]) {
					rx_ht3[i] = ht2;
					printk(KERN_WARNING "rt_can_fd: fd_isr(%d) %lld\n",
						i, ht2);
				}
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
unsigned int rt_can_fd_isr(unsigned int num, struct pt_regs *r)
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

int CAN_FD_A_Initialize(int num, int baud, int sjw, int ext, int fd_flag, int data_rate, int crc)
{ //kjg_180405
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);
	p->rxFlags = 0;
	
	CHIP_Reset(num);

	//Enable ECC and initialize RAM
	CHIP_EccEnable(num);
	//CHIP_RamInit(0xFF);

	//Configure device
	CHIP_ConfigureObjectReset(num);
	if(crc == 0) p->config.IsoCrcEnable = 0;
	else p->config.IsoCrcEnable = ISO_CRC;
	p->config.StoreInTEF = 0;
	CHIP_Configure(num);

	//Setup TX FIFO
	CHIP_TransmitChannelConfigureObjectReset(num);
	p->txConfig.FifoSize = 7;
	p->txConfig.PayLoadSize = CHIP_PLSIZE_64;
	p->txConfig.TxPriority = 1;
	CHIP_TransmitChannelConfigure(num, CHIP_FIFO_CH2);

	//Setup RX FIFO
	CHIP_ReceiveChannelConfigureObjectReset(num);
	p->rxConfig.FifoSize = 15;
	p->rxConfig.PayLoadSize = CHIP_PLSIZE_64;
	CHIP_ReceiveChannelConfigure(num, CHIP_FIFO_CH1);

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
	/*kjg_181031 if(ext == 0) {
		p->mObj.bF.MIDE = 1; //Only allow standard frame
	} else {*/
		p->mObj.bF.MIDE = 0; //Both Standard and Extended frames will be accepted
	//}
	p->mObj.bF.MEID = 0x00;
	CHIP_FilterMaskConfigure(num, CHIP_FILT0);

	//Link FIFO and Filter
	CHIP_FilterToFifoLink(num, CHIP_FILT0, CHIP_FIFO_CH1, true);

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

	//Setup Transmit and Receive Interrupts
	CHIP_GpioModeConfigure(num, CHIP_PINMODE_INT, CHIP_PINMODE_INT);
	//kjg_180405 CHIP_TransmitChannelEventEnable(num, CHIP_FIFO_CH2, CHIP_TX_FIFO_NOT_FULL_EVENT);
	CHIP_TransmitChannelEventDisable(num, CHIP_FIFO_CH2, CHIP_TX_FIFO_NOT_FULL_EVENT);
	CHIP_ReceiveChannelEventEnable(num, CHIP_FIFO_CH1, CHIP_RX_FIFO_NOT_EMPTY_EVENT);
	//kjg_180405 CHIP_ModuleEventEnable(num, CHIP_TX_EVENT | CHIP_RX_EVENT);
	CHIP_ModuleEventEnable(num, CHIP_RX_EVENT);

	//Select Normal Mode
	if(fd_flag == 0) {
		p->flag = RT_CAN_FD_CLASSIC_FLAG;
		CHIP_OperationModeSelect(num, CHIP_CLASSIC_MODE);
	} else {
		p->flag = RT_CAN_FD_NORMAL_FLAG;
		CHIP_OperationModeSelect(num, CHIP_NORMAL_MODE);
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
		printk(KERN_WARNING "rt_can_fd: rt_can_set_param error (-1) %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

	tx_ht3[num] = rx_ht3[num] = 0; //kjg_180522

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
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

	if(max_can_fd_ch == 0) return 0;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING "rt_can_fd: rt_can_set_param error (-1) %d\n", num);
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

	tx_ht3[num] = rx_ht3[num] = 0; //kjg_180522

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
		printk(KERN_WARNING "rt_can_fd: rt_can_setup error (-1) %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);
	
	if(p->used == 0) {
		printk(KERN_WARNING "rt_can_fd: rt_can_setup error (-2) %d, %d\n",
			num, p->used);
		return -2;
	}

	tx_ht3[num] = rx_ht3[num] = 0; //kjg_180522

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
			MOD_INC_USE_COUNT;
			p->used |= 0x02; //mark setup done
			break;
		case (-1):
			CHIP_Reset(num);
			break;
		case (-2):
			MOD_DEC_USE_COUNT;
			CHIP_Reset(num);
			break;
		default:
			CHIP_Reset(num);
			break;
	}

	return 0;
}

char SPI_Transfer(int num, unsigned char *pIN, unsigned char *pOUT, unsigned short len, unsigned char spi_type, int debug)
{
	unsigned char i, j;
	unsigned int addr, delay;

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_1;
	outb((unsigned char)len, addr);

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_2;
	for(i=0; i < len; i++) {
		outb(*pIN, addr);
		pIN++;

		if(i == 1 && spi_type == RT_CAN_FD_SPI_RX_ONLY) { //kjg_180522
			break;
		}
	}

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_TX_3;
	outb(0x00, addr);

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_1;
	j = 0;
	while(1) {
		for(delay=0; delay < DELAY_2US; delay++) {}
		i = inb(addr);
		if(i == (unsigned char)len) break;
		j++;
		//kjg_180522 if(j > 20) {
		if(j > 50) {
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
		*pOUT = inb(addr);
		pOUT++;
	}

	addr = RT_CAN_FD_BASE_ADDR + RT_CAN_FD_SPI_RX_3;
	i = inb(addr);

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

	error =  SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
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
		printk(KERN_WARNING "rt_can_fd: CHIP_ReadWord error (-1) %d, %d\n",
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
    p->spi_tx_buf[0]
		= (unsigned char)((_CHIP_INS_WRITE << 4) + ((address >> 8) & 0xF));
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
		printk(KERN_WARNING "rt_can_fd: CHIP_ReadHalfWord error (-1) %d, %d\n",
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

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_tx_buf, tx_length,
		//kjg_180522 RT_CAN_FD_SPI_TX_RX, 8);
		RT_CAN_FD_SPI_RX_ONLY, 8);

    //Update data
    for(i=0; i < nBytes; i++) {
        rxd[i] = p->spi_tx_buf[i + 2];
    }

    return error;
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
		RT_CAN_FD_SPI_TX_ONLY, 9);

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

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_tx_buf, tx_length,
		RT_CAN_FD_SPI_TX_ONLY, 10);

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

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_tx_buf, tx_length,
		RT_CAN_FD_SPI_TX_ONLY, 11);

    return error;
}

char CHIP_ReadByteArrayWithCRC(int num, unsigned short address, unsigned char *rxd, unsigned short nBytes, bool fromRam, bool* crcIsCorrect)
{
    char error;
    unsigned char i;
    unsigned short crcFromSpiSlave = 0, crcAtController = 0;
    unsigned short tx_length = nBytes + 5;
	//first two bytes for sending command & address, third for size, last two bytes for CRC
	struct rt_can_fd_struct *p;

	p = &(rt_can_fd_table[num]);

    //Compose command
    p->spi_tx_buf[0] = (unsigned char)((_CHIP_INS_READ_CRC << 4)
		+ ((address >> 8) & 0xF));
    p->spi_tx_buf[1] = (unsigned char)(address & 0xFF);
    if(fromRam) {
        p->spi_tx_buf[2] = nBytes >> 2;
    } else {
        p->spi_tx_buf[2] = nBytes;
    }

    //Clear data
    for(i=3; i < tx_length; i++) {
        p->spi_tx_buf[i] = 0;
    }

    error = SPI_Transfer(num, p->spi_tx_buf, p->spi_rx_buf, tx_length,
		RT_CAN_FD_SPI_TX_RX, 12);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReadByteArrayWithCRC error (-1) %d, %d\n",
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
    if(crcFromSpiSlave == crcAtController) {
        *crcIsCorrect = true;
    } else {
        *crcIsCorrect = false;
    }

    //Update data
    for(i=0; i < nBytes; i++) {
        rxd[i] = p->spi_rx_buf[i + 3];
    }

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
		RT_CAN_FD_SPI_TX_ONLY, 13);

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
		RT_CAN_FD_SPI_TX_RX, 14);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReadWordArray error (-1) %d, %d\n", num, error);
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
		RT_CAN_FD_SPI_TX_ONLY, 15);

    return error;
}

//Section : Configuration
char CHIP_Configure(int num)
{
    char error;
    T_CHIP_ctl ciCon;
	struct rt_can_fd_struct *p;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING "rt_can_fd: CHIP_Configure error (-1) %d\n", num);
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
		printk(KERN_WARNING "rt_can_fd: CHIP_Configure error (-2) %d, %d\n",
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
			"rt_can_fd: CHIP_ConfigureObjectReset error (-1) %d\n", num);
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
			"rt_can_fd: CHIP_OperationModeSelect error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    d &= ~0x07;
    d |= opMode;

    error = CHIP_WriteByte(num, _CHIP_REG_CiCON + 3, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_OperationModeSelect error (-2) %d, %d\n",
			num, error);
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
		printk(KERN_WARNING "rt_can_fd: CHIP_OperationModeGet error (-1) %d\n",
			num);
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

//Section : CAN Transmit
char CHIP_TransmitChannelConfigure(int num, T_CHIP_fifoChannel channel)
{
    char error;
    unsigned short a = 0;
    T_CHIP_fifoCtl ciFifoCon;
	struct rt_can_fd_struct *p;

	if(num >= RT_CAN_FD_CNT) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelConfigure error (-1) %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

    //Setup FIFO
    ciFifoCon.word = canControlResetValues[_CHIP_REG_CiFIFOCON / 4];
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
		"rt_can_fd: CHIP_TransmitChannelConfigureObjectReset error (-1) %d\n",
			num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

    ciFifoCon.word = canControlResetValues[_CHIP_REG_CiFIFOCON / 4];

    p->txConfig.RTREnable = ciFifoCon.txBF.RTREnable;
    p->txConfig.TxPriority = ciFifoCon.txBF.TxPriority;
    p->txConfig.TxAttempts = ciFifoCon.txBF.TxAttempts;
    p->txConfig.FifoSize = ciFifoCon.txBF.FifoSize;
    p->txConfig.PayLoadSize = ciFifoCon.txBF.PayLoadSize;

    return 0;
}

char CHIP_TransmitQueueConfigure(int num, T_CHIP_txQueCfg *config)
{
#ifndef CAN_TXQUEUE_IMPLEMENTED
    config;

	printk(KERN_WARNING
		"rt_can_fd: CHIP_TransmitQueueConfigure error (-100) %d\n", num);
    return -100;
#else
    char error;
    unsigned short a = 0;
    T_CHIP_txQueCtl ciFifoCon;

    //Setup FIFO
    ciFifoCon.word = canControlResetValues[_CHIP_REG_CiFIFOCON / 4];

    ciFifoCon.txBF.TxEnable = 1;
    ciFifoCon.txBF.FifoSize = config->FifoSize;
    ciFifoCon.txBF.PayLoadSize = config->PayLoadSize;
    ciFifoCon.txBF.TxAttempts = config->TxAttempts;
    ciFifoCon.txBF.TxPriority = config->TxPriority;

    a = _CHIP_REG_CiTXQCON;
    error = CHIP_WriteWord(num, a, ciFifoCon.word);

    return error;
#endif    
}

char CHIP_TransmitQueueConfigureObjectReset(T_CHIP_txQueCfg *config)
{
    T_CHIP_fifoCtl ciFifoCon;

    ciFifoCon.word = canControlResetValues[_CHIP_REG_CiFIFOCON / 4];

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
    unsigned int fifoReg[3], tx_length;
    T_CHIP_fifoCtl ciFifoCon;
    T_CHIP_fifoStat ciFifoSta;
    T_CHIP_fifoUserCfg ciFifoUa;

    //Get FIFO registers
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);

    error = CHIP_ReadWordArray(num, a, fifoReg, 3);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelLoad error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Check that it is a transmit buffer
    ciFifoCon.word = fifoReg[0];
    if(!ciFifoCon.txBF.TxEnable) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelLoad error (-2) %d, %d\n",
			num, ciFifoCon.txBF.TxEnable);
		return -2;
	}

    //Get status
    ciFifoSta.word = fifoReg[1];

    //Get address
    ciFifoUa.word = fifoReg[2];
#ifdef USERADDRESS_TIMES_FOUR
    a = 4 * ciFifoUa.bF.UserAddress;
#else
    a = ciFifoUa.bF.UserAddress;
#endif
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

    tx_length = CHIP_DlcToDataBytes(p->txObj.bF.ctrl.DLC);
    for(i=0; i < tx_length; i++) {
        txBuffer[i + 8] = p->tx_buf[idx].data[i];
    }

    //Make sure we write a multiple of 4 bytes to RAM
    if(tx_length % 4) {
        //Need to add bytes
        n = 4 - (tx_length % 4);
        i = tx_length + 8;

        for(j=0; j < n; j++) {
            txBuffer[i + 8 + j] = 0;
        }
    }

    error = CHIP_WriteByteArray(num, a, txBuffer, tx_length + 8 + n);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelLoad error (-3) %d, %d\n",
			num, error);
		return -3;
	}

    //Set UINC and TXREQ
    error = CHIP_TransmitChannelUpdate(num, channel, flush);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelLoad error (-4) %d, %d\n",
			num, error);
		return -4;
	}
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
			"rt_can_fd: CHIP_TransmitChannelStatusGet error (-1) %d\n", num);
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
			"rt_can_fd: CHIP_TransmitChannelUpdate error (-1) %d, %d\n",
			num, error);
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
			"rt_can_fd: CHIP_TransmitAbortAll error (-1) %d, %d\n", num, error);
        return -1;
    }

    //Modify
    d |= 0x8;

    error = CHIP_WriteByte(num, (_CHIP_REG_CiCON + 3), d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitAbortAll error (-2) %d, %d\n", num, error);
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
			"rt_can_fd: CHIP_TransmitBandWidthSharingSet error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    d &= 0x0f;
    d |= (txbws << 4);

    error = CHIP_WriteByte(num, (_CHIP_REG_CiCON + 3), d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitBandWidthSharingSet error (-2) %d, %d\n",
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
			"rt_can_fd: CHIP_FilterObjectConfigure error (-1) %d\n", num);
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
			"rt_can_fd: CHIP_FilterMaskConfigure error (-1) %d\n", num);
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
			"rt_can_fd: CHIP_FilterToFifoLink error (-1) %d\n", num);
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
		printk(KERN_WARNING "rt_can_fd: CHIP_FilterEnable error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    fCtrl.bF.Enable = 1;

    error = CHIP_WriteByte(num, a, fCtrl.byte);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_FilterEnable error (-2) %d, %d\n",
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
		printk(KERN_WARNING "rt_can_fd: CHIP_FilterDisable error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    fCtrl.bF.Enable = 0;

    error = CHIP_WriteByte(num, a, fCtrl.byte);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_FilterDisable error (-2) %d, %d\n",
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
			"rt_can_fd: CHIP_DeviceNetFilterCountSet error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    d &= 0x1f;
    d |= dnfc;

    error = CHIP_WriteByte(num, _CHIP_REG_CiCON, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_DeviceNetFilterCountSet error (-2) %d, %d\n",
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
			"rt_can_fd: CHIP_ReceiveChannelConfigure error (-1) %d\n", num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

#ifdef CAN_TXQUEUE_IMPLEMENTED
    if(channel == CAN_TXQUEUE_CH0) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelConfigure error (-100) %d\n", num);
		return -100;
	}
#endif

    //Setup FIFO
    ciFifoCon.word = canControlResetValues[_CHIP_REG_CiFIFOCON / 4];

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
		"rt_can_fd: CHIP_ReceiveChannelConfigureObjectReset error (-1) %d\n",
			num);
		return -1;
	}

	p = &(rt_can_fd_table[num]);

    ciFifoCon.word = canControlResetValues[_CHIP_REG_CiFIFOCON / 4];

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
			"rt_can_fd: CHIP_ReceiveChannelStatusGet error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Update data
    *status = (T_CHIP_rxFifoStatus)(ciFifoSta.byte[0] & 0x0F);

    return error;
}

char CHIP_ReceiveMessageGet(int num, T_CHIP_fifoChannel channel, T_CHIP_rxMsgObj *rxObj)
{
    char error;
    unsigned char n, i, ba[MAX_MSG_SIZE], nBytes;
    unsigned short a;
    unsigned int fifoReg[3], idx, delay;
    T_CHIP_fifoCtl ciFifoCon;
    T_CHIP_fifoStat ciFifoSta;
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
			"rt_can_fd: CHIP_ReceiveMessageGet error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Check that it is a receive buffer
    ciFifoCon.word = fifoReg[0];
    if(ciFifoCon.rxBF.TxEnable) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error (-2) %d, %d\n",
			num, ciFifoCon.rxBF.TxEnable);
		return -2;
	}

    //Get Status
    ciFifoSta.word = fifoReg[1];

    //Get address
    ciFifoUa.word = fifoReg[2];
#ifdef USERADDRESS_TIMES_FOUR
    a = 4 * ciFifoUa.bF.UserAddress;
#else
    a = ciFifoUa.bF.UserAddress;
#endif
    a += _CHIP_RAMADDR_START;

    //Number of bytes to read
	nBytes = MAX_DATA_BYTES; //kjg_180405
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

    error = CHIP_ReadByteArray(num, a, ba, n);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error (-3) %d, %d\n",
			num, error);
		return -3;
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

	for(delay=0; delay < DELAY_8US; delay++) {}

    //UINC channel
    error = CHIP_ReceiveChannelUpdate(num, channel);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error (-4) %d, %d\n",
			num, error);
		return -4;
	}

	tmp_msg.time = gethrtime();

	//step forward in fifo
	if(p->rx_msg_count >= RT_CAN_FD_RX_MSG_BUF_SIZE) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveMessageGet error (-5) %d, %d\n",
			num, p->rx_msg_count);
		return -5;
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
    unsigned short a = 0;
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
		printk(KERN_WARNING "rt_can_fd: CHIP_TefStatusGet error (-1) %d, %d\n",
			num, error);
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
    T_CHIP_tef ciTefSta;
    T_CHIP_fifoUserCfg ciTefUa;

    //Get FIFO registers
    a = _CHIP_REG_CiTEFCON;

    error = CHIP_ReadWordArray(num, a, fifoReg, 3);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_TefMessageGet error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Get control
    ciTefCon.word = fifoReg[0];

    //Get status
    ciTefSta.word = fifoReg[1];

    //Get address
    ciTefUa.word = fifoReg[2];
#ifdef USERADDRESS_TIMES_FOUR
    a = 4 * ciTefUa.bF.UserAddress;
#else
    a = ciTefUa.bF.UserAddress;
#endif
    a += _CHIP_RAMADDR_START;

    //Number of bytes to read
    n = 8; //8 header bytes

    if(ciTefCon.bF.TimeStampEnable) {
        n += 4; //Add 4 time stamp bytes
    }

    //Read rxObj using one access
    error = CHIP_ReadByteArray(num, a, ba, n);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_TefMessageGet error (-2) %d, %d\n",
			num, error);
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
		printk(KERN_WARNING "rt_can_fd: CHIP_TefMessageGet error (-3) %d, %d\n",
			num, error);
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
    ciTefCon.word = canControlResetValues[_CHIP_REG_CiTEFCON / 4];

    ciTefCon.bF.FifoSize = config->FifoSize;
    ciTefCon.bF.TimeStampEnable = config->TimeStampEnable;

    error = CHIP_WriteWord(num, _CHIP_REG_CiTEFCON, ciTefCon.word);

    return error;
}

char CHIP_TefConfigureObjectReset(T_CHIP_tefCfg *config)
{
    T_CHIP_tefCtl ciTefCon;

    ciTefCon.word = canControlResetValues[_CHIP_REG_CiFIFOCON / 4];

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
			"rt_can_fd: CHIP_ModuleEventGet error (-1) %d, %d\n", num, error);
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
			"rt_can_fd: CHIP_ModuleEventEnable error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    intEnables.word |= (flags & CHIP_ALL_EVENTS);

    error = CHIP_WriteHalfWord(num, a, intEnables.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ModuleEventEnable error (-2) %d, %d\n",
			num, error);
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
			"rt_can_fd: CHIP_ModuleEventDisable error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    intEnables.word &= ~(flags & CHIP_ALL_EVENTS);

    error = CHIP_WriteHalfWord(num, a, intEnables.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ModuleEventDisable error (-2) %d, %d\n",
			num, error);
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
			"rt_can_fd: CHIP_ModuleEventClear error (-1) %d, %d\n", num, error);
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
			"rt_can_fd: CHIP_ModuleEventRxCodeGet error (-1) %d, %d\n",
			num, error);
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
			"rt_can_fd: CHIP_ModuleEventTxCodeGet error (-1) %d, %d\n",
			num, error);
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
			"rt_can_fd: CHIP_ModuleEventFilterHitGet error (-1) %d, %d\n",
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
			"rt_can_fd: CHIP_ModuleEventIcodeGet error (-1) %d, %d\n",
			num, error);
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
		printk(KERN_WARNING "rt_can_fd: CHIP_TransmitChannelEventGet %d, %d\n",
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
			"rt_can_fd: CHIP_TransmitChannelIndexGet error (-1) %d, %d\n",
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
			"rt_can_fd: CHIP_TransmitChannelEventEnable error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciFifoCon.byte[0] |= (flags & CHIP_TX_FIFO_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelEventEnable error (-2) %d, %d\n",
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
			"rt_can_fd: CHIP_TransmitChannelEventDisable error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciFifoCon.byte[0] &= ~(flags & CHIP_TX_FIFO_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TransmitChannelEventDisable error (-2) %d, %d\n",
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
		"rt_can_fd: CHIP_TransmitChannelEventAttemptClear error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciFifoSta.byte[0] &= ~CHIP_TX_FIFO_ATTEMPTS_EXHAUSTED_EVENT;

    error = CHIP_WriteByte(num, a, ciFifoSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_TransmitChannelEventAttemptClear error (-2) %d, %d\n",
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

#ifdef CAN_TXQUEUE_IMPLEMENTED
    if(channel == CAN_TXQUEUE_CH0) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventGet error (-100) %d, %d\n",
			num, channel);
		return -100;
	}
#endif

    //Read Interrupt flags
    ciFifoSta.word = 0;
    a = _CHIP_REG_CiFIFOSTA + (channel * _CHIP_FIFO_OFFSET);

    error = CHIP_ReadByte(num, a, &ciFifoSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventGet error (-1) %d, %d\n",
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

#ifdef CAN_TXQUEUE_IMPLEMENTED
    if(channel == CAN_TXQUEUE_CH0) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventEnable error (-100) %d\n",
			num);
		return -100;
	}
#endif

    //Read Interrupt Enables
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
    ciFifoCon.word = 0;

    error = CHIP_ReadByte(num, a, &ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventEnable error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciFifoCon.byte[0] |= (flags & CHIP_RX_FIFO_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventEnable error (-2) %d, %d\n",
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

#ifdef CAN_TXQUEUE_IMPLEMENTED
    if(channel == CAN_TXQUEUE_CH0) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventDisable error (-100) %d, %d\n",
			num, channel);
		return -100;
	}
#endif

    //Read Interrupt Enables
    a = _CHIP_REG_CiFIFOCON + (channel * _CHIP_FIFO_OFFSET);
    ciFifoCon.word = 0;

    error = CHIP_ReadByte(num, a, &ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventDisable error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciFifoCon.byte[0] &= ~(flags & CHIP_RX_FIFO_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, ciFifoCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_ReceiveChannelEventDisable error (-2) %d, %d\n",
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

#ifdef CAN_TXQUEUE_IMPLEMENTED
    if(channel == CAN_TXQUEUE_CH0) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_ReceiveChannelEventOverflowClear error (-100) %d,%d\n",
			num, channel);
		return -100;
	}
#endif

    //Read Interrupt Flags
    ciFifoSta.word = 0;
    a = _CHIP_REG_CiFIFOSTA + (channel * _CHIP_FIFO_OFFSET);

    error = CHIP_ReadByte(num, a, &ciFifoSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_ReceiveChannelEventOverflowClear error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciFifoSta.byte[0] &= ~(CHIP_RX_FIFO_OVERFLOW_EVENT);

    error = CHIP_WriteByte(num, a, ciFifoSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_ReceiveChannelEventOverflowClear error (-2) %d, %d\n",
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
		printk(KERN_WARNING "rt_can_fd: CHIP_TefEventGet error (-1) %d, %d\n",
			num, error);
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
			"rt_can_fd: CHIP_TefEventEnable error (-1) %d, %d\n", num, error);
		return -1;
	}

    //Modify
    ciTefCon.byte[0] |= (flags & CHIP_TEF_FIFO_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, ciTefCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefEventEnable error (-2) %d, %d\n", num, error);
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
			"rt_can_fd: CHIP_TefEventDisable error (-1) %d, %d\n", num, error);
		return -1;
	}

    //Modify
    ciTefCon.byte[0] &= ~(flags & CHIP_TEF_FIFO_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, ciTefCon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefEventDisable error (-2) %d, %d\n", num, error);
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
			"rt_can_fd: CHIP_TefEventOverflowClear error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    ciTefSta.byte[0] &= ~(CHIP_TEF_FIFO_OVERFLOW_EVENT);

    error = CHIP_WriteByte(num, a, ciTefSta.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TefEventOverflowClear error (-2) %d, %d\n",
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
    unsigned short a = 0;

    //Read Error state
    a = _CHIP_REG_CiTREC + 2;

    error = CHIP_ReadByte(num, a, &f);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_ErrorStateGet %d, %d\n", num,
			error);
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
			"rt_can_fd: CHIP_ErrorCountStateGet error (-1) %d, %d\n",
			num, error);
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
			"rt_can_fd: CHIP_BusDiagnosticsGet error (-1) %d, %d\n",
			num, error);
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
		printk(KERN_WARNING "rt_can_fd: CHIP_EccEnable error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    d |= 0x01;

    error = CHIP_WriteByte(num, _CHIP_REG_ECCCON, d);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_EccEnable error (-2) %d, %d\n",
			num, error);
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
		printk(KERN_WARNING "rt_can_fd: CHIP_EccDisable error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    d &= ~0x01;

    error = CHIP_WriteByte(num, _CHIP_REG_ECCCON, d);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_EccDisable error (-2) %d, %d\n",
			num, error);
		return -2;
	}

    return 0;
}

char CHIP_EccEventGet(int num, T_CHIP_eccEvent *flags)
{
    char error;
    unsigned char eccStatus = 0;
    unsigned short a = 0;

    //Read Interrupt flags
    a = _CHIP_REG_ECCSTA;

    error = CHIP_ReadByte(num, a, &eccStatus);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_EccEventGet error (-1) %d, %d\n",
			num, error);
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
			"rt_can_fd: CHIP_EccErrorAddressGet error (-1) %d, %d\n",
			num, error);
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
    unsigned short a = 0;

    a = _CHIP_REG_ECCCON;

    error = CHIP_ReadByte(num, a, &eccInterrupts);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccErrorEnable error (-1) %d, %d\n", num, error);
		return -1;
	}

    //Modify
    eccInterrupts |= (flags & CHIP_ECC_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, eccInterrupts);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccErrorEnable error (-2) %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_EccEventDisable(int num, T_CHIP_eccEvent flags)
{
    char error;
    unsigned char eccInterrupts = 0;
    unsigned short a = 0;

    a = _CHIP_REG_ECCCON;

    error = CHIP_ReadByte(num, a, &eccInterrupts);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccErrorDisable error (-1) %d, %d\n", num, error);
		return -1;
	}

    //Modify
    eccInterrupts &= ~(flags & CHIP_ECC_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, eccInterrupts);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_EccErrorDisable error (-2) %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_EccEventClear(int num, T_CHIP_eccEvent flags)
{
    char error;
    unsigned char eccStat = 0;
    unsigned short a = 0;

    a = _CHIP_REG_ECCSTA;

    error = CHIP_ReadByte(num, a, &eccStat);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_EccEventClear error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    eccStat &= ~(flags & CHIP_ECC_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, eccStat);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_EccEventClear error (-2) %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

//Section : CRC
char CHIP_CrcEventEnable(int num, T_CHIP_crcEvent flags)
{
    char error;
    unsigned char crc;
    unsigned short a = 0;

    //Read interrupt control bits of CRC Register
    a = _CHIP_REG_CRC + 3;

    error = CHIP_ReadByte(num, a, &crc);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_CrcEventEnable error (-1) %d, %d\n", num, error);
		return -1;
	}

    //Modify
    crc |= (flags & CHIP_CRC_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, crc);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_CrcEventEnable error (-2) %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_CrcEventDisable(int num, T_CHIP_crcEvent flags)
{
    char error;
    unsigned char crc;
    unsigned short a = 0;

    //Read interrupt control bits of CRC Register
    a = _CHIP_REG_CRC + 3;

    error = CHIP_ReadByte(num, a, &crc);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_CrcEventDisable error (-1) %d, %d\n", num, error);
		return -1;
	}

    //Modify
    crc &= ~(flags & CHIP_CRC_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, crc);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_CrcEventDisable error (-2) %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_CrcEventClear(int num, T_CHIP_crcEvent flags)
{
    char error;
    unsigned char crc;
    unsigned short a = 0;

    //Read interrupt flags of CRC Register
    a = _CHIP_REG_CRC + 2;

    error = CHIP_ReadByte(num, a, &crc);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_CrcEventClear error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    crc &= ~(flags & CHIP_CRC_ALL_EVENTS);

    error = CHIP_WriteByte(num, a, crc);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_CrcEventClear error (-2) %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_CrcEventGet(int num, T_CHIP_crcEvent *flags)
{
    char error;
    unsigned char crc;
    unsigned short a = 0;

    //Read interrupt flags of CRC Register
    a = _CHIP_REG_CRC + 2;

    error = CHIP_ReadByte(num, a, &crc);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_CrcEventGet error (-1) %d, %d\n",
			num, error);
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
    unsigned char txd[SPI_DEFAULT_BUFFER_LENGTH];
    unsigned short a = _CHIP_RAMADDR_START;
    unsigned int k;

    //Prepare data
    for(k=0; k < SPI_DEFAULT_BUFFER_LENGTH; k++) txd[k] = d;

    for(k=0; k < (_CHIP_RAM_SIZE / SPI_DEFAULT_BUFFER_LENGTH); k++) {
        error = CHIP_WriteByteArray(num, a, txd, SPI_DEFAULT_BUFFER_LENGTH);
        if(error) {
			printk(KERN_WARNING "rt_can_fd: CHIP_RamInit error (-1) %d, %d\n",
				num, error);
			return -1;
		}

        a += SPI_DEFAULT_BUFFER_LENGTH;
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
			"rt_can_fd: CHIP_TimeStampEnable error (-1) %d, %d\n", num, error);
		return -1;
	}

    //Modify
    d |= 0x01;

    error = CHIP_WriteByte(num, _CHIP_REG_CiTSCON + 2, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TimeStampEnable error (-2) %d, %d\n", num, error);
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
			"rt_can_fd: CHIP_TimeStampDisable error (-1) %d, %d\n", num, error);
		return -1;
	}

    //Modify
    d &= 0x06;

    error = CHIP_WriteByte(num, _CHIP_REG_CiTSCON + 2, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TimeStampDisable error (-2) %d, %d\n", num, error);
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
			"rt_can_fd: CHIP_TimeStampModeConfigure error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    d &= 0x01;
    d |= mode << 1;

    error = CHIP_WriteByte(num, _CHIP_REG_CiTSCON + 2, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_TimeStampModeConfigure error (-2) %d, %d\n",
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
			"rt_can_fd: CHIP_OscillatorEnable error (-1) %d, %d\n", num, error);
		return -1;
	}

    //Modify
    d &= ~0x4;

    error = CHIP_WriteByte(num, _CHIP_REG_OSC, d);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_OscillatorEnable error (-2) %d, %d\n", num, error);
		return -2;
	}

    return error;
}

char CHIP_OscillatorControlSet(int num, T_CHIP_divCtl ctrl)
{
    char error;
    T_CHIP_oscCtl osc;

    osc.word = 0;

    osc.bF.PllEnable = ctrl.PllEnable;
    osc.bF.OscDisable = ctrl.OscDisable;
    osc.bF.SCLKDIV = ctrl.SclkDivide;
    osc.bF.CLKODIV = ctrl.ClkOutDivide;

    error = CHIP_WriteByte(num, _CHIP_REG_OSC, osc.byte[0]);

    return error;
}

char CHIP_OscillatorControlObjectReset(T_CHIP_divCtl *ctrl)
{
    T_CHIP_oscCtl osc;

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
    T_CHIP_oscCtl osc;
    
    osc.word = 0;

    error = CHIP_ReadByte(num, _CHIP_REG_OSC + 1, &osc.byte[1]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_OscillatorStatusGet error (-1) %d, %d\n",
			num, error);
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
				"rt_can_fd: CHIP_BitTimeConfigure error (-1) %d, %d\n",
				num, clk);
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
			"rt_can_fd: CHIP_BitTimeConfigureNominal40MHz error (-1) %d, %d\n",
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
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 30;
            ciDbtcfg.bF.TSEG2 = 7;
            ciDbtcfg.bF.SJW = 7;
            //SSP
            ciTdc.bF.TDCOffset = 31;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_250K_1M5:
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
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 6;
            ciDbtcfg.bF.TSEG2 = 1;
            ciDbtcfg.bF.SJW = 1;
            //SSP
            ciTdc.bF.TDCOffset = 7;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_500K_5M:
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 4;
            ciDbtcfg.bF.TSEG2 = 1;
            ciDbtcfg.bF.SJW = 1;
            //SSP
            ciTdc.bF.TDCOffset = 5;
            ciTdc.bF.TDCValue = tdcValue;
            break;
        case CHIP_500K_6M7:
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
            ciDbtcfg.bF.BRP = 0;
            ciDbtcfg.bF.TSEG1 = 2;
            ciDbtcfg.bF.TSEG2 = 0;
            ciDbtcfg.bF.SJW = 0;
            //SSP
            ciTdc.bF.TDCOffset = 3;
            ciTdc.bF.TDCValue = 1;
            break;
        case CHIP_500K_10M:
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
				"rt_can_fd: CHIP_BitTimeConfigureData40MHz error (-1) %d, %d\n",
				num, p->selectedBitTime);
            return -1;
            break;
    }

    //Write Bit time registers
    error = CHIP_WriteWord(num, _CHIP_REG_CiDBTCFG, ciDbtcfg.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureData40MHz error (-2) %d, %d, %d\n",
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
			"rt_can_fd: CHIP_BitTimeConfigureData40MHz error (-3) %d, %d, %d\n",
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
			"rt_can_fd: CHIP_BitTimeConfigureNominal20MHz error (-1) %d, %d\n",
				num, p->selectedBitTime);
            return -1;
            break;
    }

    //Write Bit time registers
    error = CHIP_WriteWord(num, _CHIP_REG_CiNBTCFG, ciNbtcfg.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureNominal20MHz error (-2) %d, %d\n",
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
				"rt_can_fd: CHIP_BitTimeConfigureData20MHz error (-11) %d\n",
				num);
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
				"rt_can_fd: CHIP_BitTimeConfigureData20MHz error (-12) %d\n",
				num);
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
				"rt_can_fd: CHIP_BitTimeConfigureData20MHz error (-1) %d, %d\n",
				num, p->selectedBitTime);
            return -1;
            break;
    }

    //Write Bit time registers
    error = CHIP_WriteWord(num, _CHIP_REG_CiDBTCFG, ciDbtcfg.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureData20MHz error (-2) %d, %d\n",
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
			"rt_can_fd: CHIP_BitTimeConfigureData20MHz error (-3) %d, %d\n",
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
			"rt_can_fd: CHIP_BitTimeConfigureNominal10MHz error (-1) %d, %d\n",
				num, p->selectedBitTime);
            return -1;
            break;
    }

    //Write Bit time registers
    error = CHIP_WriteWord(num, _CHIP_REG_CiNBTCFG, ciNbtcfg.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureNominal10MHz error (-2) %d, %d\n",
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
				"rt_can_fd: CHIP_BitTimeConfigureData10MHz error (-11) %d\n",
				num);
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
				"rt_can_fd: CHIP_BitTimeConfigureData10MHz error (-12) %d\n",
				num);
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
				"rt_can_fd: CHIP_BitTimeConfigureData10MHz error (-13) %d\n",
				num);
            return -13;
            break;

        default:
			printk(KERN_WARNING
				"rt_can_fd: CHIP_BitTimeConfigureData10MHz error (-1) %d, %d\n",
				num, p->selectedBitTime);
            return -1;
            break;
    }

    //Write Bit time registers
    error = CHIP_WriteWord(num, _CHIP_REG_CiDBTCFG, ciDbtcfg.word);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_BitTimeConfigureData10MHz error (-2) %d, %d\n",
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
			"rt_can_fd: CHIP_BitTimeConfigureData10MHz error (-3) %d, %d\n",
			num, error);
		return -3;
	}

    return error;
}

//Section : GPIO
char CHIP_GpioModeConfigure(int num, T_CHIP_pinMode gpio0, T_CHIP_pinMode gpio1)
{
    char error;
    unsigned short a = 0;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON + 3;
    iocon.word = 0;

    error = CHIP_ReadByte(num, a, &iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioModeConfigure error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    iocon.bF.PinMode0 = gpio0;
    iocon.bF.PinMode1 = gpio1;

    error = CHIP_WriteByte(num, a, iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioModeConfigure error (-2) %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioDirectionConfigure(int num, T_CHIP_pinDir gpio0, T_CHIP_pinDir gpio1)
{
    char error;
    unsigned short a = 0;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON;
    iocon.word = 0;

    error = CHIP_ReadByte(num, a, &iocon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioDirectionConfigure error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    iocon.bF.TRIS0 = gpio0;
    iocon.bF.TRIS1 = gpio1;

    error = CHIP_WriteByte(num, a, iocon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioDirectionConfigure error (-2) %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioStandbyControlEnable(int num)
{
    char error;
    unsigned short a = 0;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON;
    iocon.word = 0;

    error = CHIP_ReadByte(num, a, &iocon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioStandbyControlEnable error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    iocon.bF.XcrSTBYEnable = 1;

    error = CHIP_WriteByte(num, a, iocon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioStandbyControlEnable error (-2) %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioStandbyControlDisable(int num)
{
    char error;
    unsigned short a = 0;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON;
    iocon.word = 0;

    error = CHIP_ReadByte(num, a, &iocon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioStandbyControlDisable error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    iocon.bF.XcrSTBYEnable = 0;

    error = CHIP_WriteByte(num, a, iocon.byte[0]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioStandbyControlDisable error (-2) %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioInterruptPinsOpenDrainConfigure(int num, T_CHIP_odm mode)
{
    char error;
    unsigned short a = 0;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON + 3;
    iocon.word = 0;

    error = CHIP_ReadByte(num, a, &iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_GpioInterruptPinsOpenDrainConfigure error (-1)%d,%d\n",
			num, error);
		return -1;
	}

    //Modify
    iocon.bF.INTPinOpenDrain = mode;

    error = CHIP_WriteByte(num, a, iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_GpioInterruptPinsOpenDrainConfigure error (-2)%d,%d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioTransmitPinOpenDrainConfigure(int num, T_CHIP_odm mode)
{
    char error;
    unsigned short a = 0;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON + 3;
    iocon.word = 0;

    error = CHIP_ReadByte(num, a, &iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_GpioTransmitPinsOpenDrainConfigure error (-1) %d,%d\n",
			num, error);
		return -1;
	}

    //Modify
    iocon.bF.TXCANOpenDrain = mode;

    error = CHIP_WriteByte(num, a, iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
		"rt_can_fd: CHIP_GpioTransmitPinsOpenDrainConfigure error (-2) %d,%d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioPinSet(int num, T_CHIP_pinPos pos, T_CHIP_pinState latch)
{
    char error;
    unsigned short a = 0;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON + 1;
    iocon.word = 0;

    error = CHIP_ReadByte(num, a, &iocon.byte[1]);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_GpioPinSet error (-1) %d, %d\n",
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
            return -1;
            break;
    }

    error = CHIP_WriteByte(num, a, iocon.byte[1]);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_GpioPinSet errror (-2) %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

char CHIP_GpioPinRead(int num, T_CHIP_pinPos pos, T_CHIP_pinState *state)
{
    char error;
    unsigned short a = 0;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON + 2;
    iocon.word = 0;

    error = CHIP_ReadByte(num, a, &iocon.byte[2]);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_GpioPinRead error (-1) %d, %d\n",
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
				"rt_can_fd: CHIP_GpioPinRead error (-2) %d, %d\n", num, error);
            return -2;
            break;
    }

    return error;
}

char CHIP_GpioClockOutputConfigure(int num, T_CHIP_clkoMode mode)
{
    char error;
    unsigned short a = 0;
    T_CHIP_ioCtl iocon;

    a = _CHIP_REG_IOCON + 3;
    iocon.word = 0;

    error = CHIP_ReadByte(num, a, &iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioClockOutputConfigure error (-1) %d, %d\n",
			num, error);
		return -1;
	}

    //Modify
    iocon.bF.SOFOutputEnable = mode;

    error = CHIP_WriteByte(num, a, iocon.byte[3]);
    if(error) {
		printk(KERN_WARNING
			"rt_can_fd: CHIP_GpioClockOutputConfigure error (-2) %d, %d\n",
			num, error);
		return -2;
	}

    return error;
}

//Section : Miscellaneous
unsigned int CHIP_DlcToDataBytes(T_CHIP_dlc dlc)
{
    unsigned int dataBytesInObject;

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

T_CHIP_dlc CHIP_DataBytesToDlc(unsigned char n)
{
    T_CHIP_dlc dlc;

    if(n <= 8) {
		switch(n) {
			case 0: dlc = CHIP_DLC_0; break;
			case 1: dlc = CHIP_DLC_1; break;
			//case 1: dlc = CHIP_DLC_4; break;
			case 2: dlc = CHIP_DLC_2; break;
			//case 2: dlc = CHIP_DLC_4; break;
			case 3: dlc = CHIP_DLC_3; break;
			//case 3: dlc = CHIP_DLC_4; break;
			case 4: dlc = CHIP_DLC_4; break;
			case 5: dlc = CHIP_DLC_5; break;
			//case 5: dlc = CHIP_DLC_8; break;
			case 6: dlc = CHIP_DLC_6; break;
			//case 6: dlc = CHIP_DLC_8; break;
			case 7: dlc = CHIP_DLC_7; break;
			//case 7: dlc = CHIP_DLC_8; break;
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

char CHIP_FifoIndexGet(int num, T_CHIP_fifoChannel channel, unsigned char *mi)
{
    char error;
    unsigned char b = 0;
    unsigned short a = 0;

    //Read Status register
    a = _CHIP_REG_CiFIFOSTA + (channel * _CHIP_FIFO_OFFSET);
    a += 1; //byte[1]

    error = CHIP_ReadByte(num, a, &b);
    if(error) {
		printk(KERN_WARNING "rt_can_fd: CHIP_FifoIndexGet error (-1) %d, %d\n",
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

//Request port memory and register ISRs, if we cannot get the memory
//of all ports, release all already requested ports and return an error.
//return Success status, zero on success.
int init_module(void)
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
}

