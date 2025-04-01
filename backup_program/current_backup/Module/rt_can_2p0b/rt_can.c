//rt_can
//RT-Linux kernel module for CAN.

#include <linux/config.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

#include <asm/system.h>
#include <asm/io.h>

#include <rtl_conf.h>
#include <rtl_core.h>
#include <rtl_sync.h>
#include <pthread.h>
#include <rtl.h> //kjg_180723
#include <rtl_core.h> //kjg_180723

#include "rt_can.h"
#include "rt_can_ext.h"

pthread_t thread; //kjg_180723_s
long long tx_ht3[RT_CAN_CNT];
long long rx_ht3[RT_CAN_CNT];
int set_parameter[RT_CAN_CNT][8]; //kjg_180723_e
int thread_state; //kjg_180810

//used = 0 - port and irq setting by rt_can_set_param.
//If you want to work like a standard rt_can you can set used = 1.
#ifdef __SBC_EM104_A5362__
struct rt_can_struct rt_can_table[RT_CAN_CNT] =
{ //sbc : a5362
    {0, RT_CAN_BASE_BAUD, 0x890, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x898, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8A0, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8A8, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8B0, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8B8, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8C0, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8C8, 5, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1}
};
#else
struct rt_can_struct rt_can_table[RT_CAN_CNT] =
{ //sbc : mark533, mark800, v621
    {0, RT_CAN_BASE_BAUD, 0x890, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x898, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8A0, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8A8, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8B0, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8B8, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8C0, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1},
    {0, RT_CAN_BASE_BAUD, 0x8C8, 9, RT_CAN_STD_FLAG, rt_can_share_isr, 0, 1}
};
#endif

void *rt_can_task(void *arg)
{ //kjg_180723
	int slot=0;
	long long ht0, ht1, ht2;//, ht3[RT_CAN_CNT];
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
	//pthread_make_periodic_np(pthread_self(), gethrtime(), 10000000); //10000us
	//pthread_make_periodic_np(pthread_self(), gethrtime(), 5000000); //5000us
	//pthread_make_periodic_np(pthread_self(), gethrtime(), 2000000); //2000us
	//pthread_make_periodic_np(pthread_self(), gethrtime(), 1900000); //1900us
	pthread_make_periodic_np(pthread_self(), gethrtime(), 1500000); //1500us
	pthread_setfp_np(pthread_self(), 1);

	while(1) {
		ht0 = gethrtime();
		tx_can_data(slot);

		ht1 = gethrtime();
		ht2 = ht1 - ht0;
		if(ht2 >= 2000 && ht2 > tx_ht3[slot]) {
			tx_ht3[slot] = ht2;
			printk(KERN_WARNING "rt_can: time_slot(%d) %lld\n",
				slot, ht2);
		}

		slot++;
		if(slot >= RT_CAN_CNT) slot = 0;
		//kjg_180522 if(slot >= 4) slot = 0;

		pthread_wait_np();
	}

	return 0;
}

void tx_can_data(int num)
{ //kjg_180723
	unsigned char tmp, attempts = MAX_TXQUEUE_ATTEMPTS;
	int idx, tmp2;
	unsigned int base;
	struct rt_can_struct *p;
	unsigned char txfi, ucLen, ucWriteoutBase;
	int	i=0, nRtrFrame;
	unsigned int id;

	p = &(rt_can_table[num]);

	if(p->used <= 0) return; //kjg_180810

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
	if(p->tx_msg_read_idx >= RT_CAN_TX_MSG_BUF_SIZE)
		p->tx_msg_read_idx = 0;
	idx = p->tx_msg_read_idx;
	p->tx_msg_count--;
/*
	//Initialize ID and Control bits
	p->txObj.word[0] = 0;
	p->txObj.word[1] = 0;

	if(p->type == 0) {
		p->txObj.bF.id.SID = p->tx_buf[idx].id; //Standard or Base ID
		p->txObj.bF.id.EID = 0; //Extended ID
		p->txObj.bF.ctrl.IDE = 0; //Standard frame
	} else {
		p->txObj.bF.id.SID = p->tx_buf[idx].id; //Standard or Base ID
		p->txObj.bF.id.EID = p->tx_buf[idx].id; //Extended ID
		p->txObj.bF.ctrl.IDE = 1; //Extended frame
	}

	if(p->tx_buf[idx].can_fd_flag == 0) {
		p->txObj.bF.ctrl.FDF = 0; //CAN FD frame
		p->txObj.bF.ctrl.BRS = 0; //Switch bit rate
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
	//kjg_180405 p->txObj.bF.ctel.SEQ = 1;
*/
	rtl_hard_disable_irq(p->irq);

	base = RT_CAN_BASE_ADDR;
	tmp = 0x01;
	tmp = tmp << num;
	//outb(tmp, base + RT_CAN_PORT_SELECT);
	outb((unsigned char)(num + 1), base + RT_CAN_PORT_SELECT);

	attempts = 2; //kjgd_180522

	do {
		tmp2 = inb(base + CHIPSTATUS) & TRANS_BUFFER_STATUS;
		if(tmp2 != 0) {
			//tmp2 = sja1000_write(num, p);
  
			id = p->tx_buf[idx].id;
			txfi = ucLen = p->tx_buf[idx].length;  
			nRtrFrame = p->tx_buf[idx].type & MSGTYPE_RTR;  
  
			if((p->tx_buf[idx].type & MSGTYPE_EXTENDED) || (p->type == 1)) {
    			txfi |= BUFFER_EFF;
				id <<= 3;

				outb((unsigned char)((id & 0xFF000000) >> 24),
					base + TRANSMIT_FRAME_BASE + 1);
				outb((unsigned char)((id & 0x00FF0000) >> 16),
					base + TRANSMIT_FRAME_BASE + 2);
				outb((unsigned char)((id & 0x0000FF00) >> 8),
					base + TRANSMIT_FRAME_BASE + 3);
				outb((unsigned char)(id & 0x000000FF),
					base + TRANSMIT_FRAME_BASE + 4);
  
				ucWriteoutBase = TRANSMIT_FRAME_BASE + 5;
			} else {
    			id <<= 5;
  
				outb((unsigned char)((id & 0xFF00) >> 8),
					base + TRANSMIT_FRAME_BASE + 1);
				outb((unsigned char)(id & 0xFF),
					base + TRANSMIT_FRAME_BASE + 2);
  
				ucWriteoutBase = TRANSMIT_FRAME_BASE + 3;
  			}

			//add data only if it is a RTR frame
			if(nRtrFrame) txfi |= BUFFER_RTR;
			else {
				while(ucLen--) {
					outb(p->tx_buf[idx].data[i], base + ucWriteoutBase + i);
					i++;
				}
			}
  
			//finish message construction
			outb(txfi, base + TRANSMIT_FRAME_BASE);

			outb(TRANSMISSION_REQUEST, base + COMMAND);
			break;
		}

		attempts--;

		printk(KERN_WARNING "rt_can: tx_can_data error (-1) %d, %d\n",
			num, attempts);
	} while(attempts > 0);

	rtl_hard_enable_irq(p->irq);
}

int rt_can_write(int num, const char *msg)
{ //kjg_180723
	int idx;
	struct rt_can_struct *p;

	if(max_can_ch == 0) return 0;

    if(num >= RT_CAN_CNT) {
		printk(KERN_WARNING "rt_can: rt_can_write error (-1) %d\n", num);
		return -1;
	}

	p = &(rt_can_table[num]);

	if(p->used <= 0) {
		printk(KERN_WARNING "rt_can: rt_can_write error (-2) %d, %d\n",
			num, p->used);
		return -2;
	}

	if(p->tx_msg_count >= RT_CAN_TX_MSG_BUF_SIZE) {
		p->tx_msg_error++;
		if(p->tx_msg_error < 10) {
			//kjg_180723 printk(KERN_WARNING
			//	"rt_can: rt_can_write error (-3) %d, %d, %d\n", num,
			//	p->tx_msg_count, p->tx_msg_error);
			return -3;
		} else {
			printk(KERN_WARNING
				"rt_can: rt_can_write error (-4) %d, %d, %d\n", num,
				p->tx_msg_count, p->tx_msg_error); //kjg_180723

			p->tx_msg_error = 0;
			p->tx_msg_count = 0;
			p->tx_msg_read_idx = p->tx_msg_write_idx;
		}
	}

	idx = p->tx_msg_write_idx;
	idx++;
	if(idx >= RT_CAN_TX_MSG_BUF_SIZE) idx = 0;

	memcpy((char *)&p->tx_buf[idx], msg, sizeof(struct rt_can_msg));

	if(p->flag == RT_CAN_FD_CLASSIC_FLAG) { //kjgw_180723
		p->tx_buf[idx].can_fd_flag = 0;
	} else { //RT_CAN_FD_NORMAL_FLAG
		p->tx_buf[idx].can_fd_flag = 1;
	}

	p->tx_msg_write_idx = idx;
	p->tx_msg_count++;

	return 0;
}
/*kjg_180723
int rt_can_write(int num, const char *msg)
{
	unsigned char tmp, mask;
	int idx, tmp2;
	unsigned int base;
	//kjg_180405 long state;
	struct rt_can_struct *p;

	if(max_can_ch == 0) return 0; //kjg_180405

    if(num >= RT_CAN_CNT) {
		printk(KERN_WARNING "rt_can: rt_can_write error (-1) %d\n", num);
		return -1;
	}

	p = &(rt_can_table[num]);

	if(p->used <= 0) {
		printk(KERN_WARNING "rt_can: rt_can_write error (-2) %d, %d\n",
			num, p->used);
		return -2;
	}

	//kjg_180405 rt_can_irq_off(state);
	rtl_hard_disable_irq(p->irq); //kjg_180405

	base = RT_CAN_BASE_ADDR;
	outb((unsigned char)(num + 1), base + RT_CAN_PORT_SELECT);

	tmp = 0x01;
	tmp = tmp << num;
	mask = inb(base + RT_CAN_PORT_IRQ_MASK);
//	if((mask & tmp) == 0) {
//		rt_can_irq_on(state);
//		printk(KERN_WARNING "rt_can: rt_can_write error (-3) %d, %x, %x\n",
//			num, mask, tmp);
//		return -3;
//	}

	tmp = 0;
	if(p->tx_msg_count >= RT_CAN_TX_MSG_BUF_SIZE) {
		p->tx_msg_error++;
		if(p->tx_msg_error < 10) {
			printk(KERN_WARNING
				"rt_can: rt_can_write error (-4) %d, %d, %d\n",
				num, mask, tmp);
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
	if(idx >= RT_CAN_TX_MSG_BUF_SIZE) idx = 0;

	memcpy((char *)&p->tx_buf[idx], msg, sizeof(struct rt_can_msg));

	p->tx_msg_write_idx = idx;
	p->tx_msg_count++;

	if(p->tx_msg_count > 0) {
		if(p->tx_msg_write_idx == p->tx_msg_read_idx) {
			printk(KERN_WARNING
				"rt_can: rt_can_write error (-5) %d, %d, %d\n",
				num, p->tx_msg_write_idx, p->tx_msg_read_idx);
			return -5;
		} else {
			tmp2 = inb(base + CHIPSTATUS) & TRANS_BUFFER_STATUS;
			if(tmp2 != 0) {
				tmp2 = sja1000_write(num, p);
			}
		}
	}

    //kjg_180405 rt_can_irq_on(state);
	rtl_hard_enable_irq(p->irq); //kjg_180405

	return 0;
}*/

int sja1000_write(int num, struct rt_can_struct *p) 
{
	unsigned char txfi, ucLen, ucWriteoutBase;
	int	i=0, nRtrFrame, idx;
	unsigned int base, id;
  
	base = RT_CAN_BASE_ADDR;

	if(p->tx_msg_count == 0) return -1;

	if(p->tx_msg_write_idx == p->tx_msg_read_idx) return -2;

	p->tx_msg_read_idx++;
	if(p->tx_msg_read_idx >= RT_CAN_TX_MSG_BUF_SIZE)
		p->tx_msg_read_idx = 0;
	idx = p->tx_msg_read_idx;

	p->tx_msg_count--;
	id = p->tx_buf[idx].id;
	txfi = ucLen = p->tx_buf[idx].length;  
	nRtrFrame = p->tx_buf[idx].type & MSGTYPE_RTR;  
  
	if((p->tx_buf[idx].type & MSGTYPE_EXTENDED) || (p->type == 1)) {
    	txfi |= BUFFER_EFF;
		id <<= 3;

		outb((unsigned char)((id & 0xFF000000) >> 24),
			base + TRANSMIT_FRAME_BASE + 1);
		outb((unsigned char)((id & 0x00FF0000) >> 16),
			base + TRANSMIT_FRAME_BASE + 2);
		outb((unsigned char)((id & 0x0000FF00) >> 8),
			base + TRANSMIT_FRAME_BASE + 3);
		outb((unsigned char)(id & 0x000000FF),
			base + TRANSMIT_FRAME_BASE + 4);
  
		ucWriteoutBase = TRANSMIT_FRAME_BASE + 5;
	} else {
    	id <<= 5;
  
		outb((unsigned char)((id & 0xFF00) >> 8),
			base + TRANSMIT_FRAME_BASE + 1);
		outb((unsigned char)(id & 0xFF),
			base + TRANSMIT_FRAME_BASE + 2);
  
		ucWriteoutBase = TRANSMIT_FRAME_BASE + 3;
  }

	//add data only if it is a RTR frame
	if(nRtrFrame) txfi |= BUFFER_RTR;
	else {
		while(ucLen--) {
			outb(p->tx_buf[idx].data[i], base + ucWriteoutBase + i);
			i++;
		}
	}
  
	//finish message construction
	outb(txfi, base + TRANSMIT_FRAME_BASE);

	outb(TRANSMISSION_REQUEST, base + COMMAND);
	
	return 0;
}

int rt_can_read(int num, char *ptr)
{
	unsigned char tmp, mask;
	int idx;
	unsigned int base;
	//kjg_180405 long state;
	struct rt_can_struct *p;

    if(num >= RT_CAN_CNT) {
		printk(KERN_WARNING "rt_can: rt_can_read error (-1) %d\n", num);
		return -1;
	}

	p = &(rt_can_table[num]);

	if(p->used <= 0) {
		printk(KERN_WARNING "rt_can: rt_can_read error (-2) %d, %d\n",
			num, p->used);
		return -2;
	}

	if(p->rx_msg_count <= 0) {
		//kjg_180405 printk(KERN_WARNING "rt_can: rt_can_read error (-3) %d, %d\n",
		//	num, p->rx_msg_count);
		return -3;
	}

	if(p->rx_msg_write_idx == p->rx_msg_read_idx) {
		printk(KERN_WARNING "rt_can: rt_can_read error (-4) %d, %d, %d\n",
			num, p->rx_msg_write_idx, p->rx_msg_read_idx);
		return -4;
	}

	//kjg_180405 rt_can_irq_off(state);
	rtl_hard_disable_irq(p->irq); //kjg_180405

	base = RT_CAN_BASE_ADDR;
	outb((unsigned char)(num + 1), base + RT_CAN_PORT_SELECT);

	tmp = 0x01;
	tmp = tmp << num;
	mask = inb(base + RT_CAN_PORT_IRQ_MASK);
/*	if((mask & tmp) == 0) {
		rt_can_irq_on(state);
		printk(KERN_WARNING "rt_can: rt_can_read error (-5) %d, %x, %x\n",
			num, mask, tmp);
		return -5;
	}*/

	p->rx_msg_read_idx++;
	if(p->rx_msg_read_idx >= RT_CAN_RX_MSG_BUF_SIZE) p->rx_msg_read_idx = 0;

	idx = p->rx_msg_read_idx;
	memcpy(ptr, (char *)&p->rx_buf[idx], sizeof(struct rt_can_msg));
	p->rx_msg_count--;

    //kjg_180405 rt_can_irq_on(state);
	rtl_hard_enable_irq(p->irq); //kjg_180405

    return p->rx_msg_count;
}

//read CAN-data from chip, supposed a message is available
int sja1000_read(int can, struct rt_can_struct *p) 
{
	unsigned char tmp, rxfi, ucLen;
	int i=MAX_MESSAGES_PER_INTERRUPT, j, idx;
	unsigned int base, id;
	struct rt_can_msg tmp_msg;
  
	base = RT_CAN_BASE_ADDR;

	do {
		unsigned char ucReadoutBase;
		int k, nRtrFrame;

		rxfi = inb(base + RECEIVE_FRAME_BASE);
    
		ucLen = rxfi & BUFFER_DLC_MASK;    
		if(ucLen > 8) ucLen = 8;
				
		nRtrFrame = rxfi & BUFFER_RTR;
  
		if(nRtrFrame) tmp_msg.type = MSGTYPE_RTR;
		else tmp_msg.type = MSGTYPE_STANDARD;
			
		j = 0;
		if((p->type == 1) || ((rxfi & BUFFER_EFF) != 0)) {
			tmp_msg.type |= MSGTYPE_EXTENDED;
  
			tmp = inb(base + RECEIVE_FRAME_BASE + 1);
			id = (unsigned int)tmp << 24;
			tmp = inb(base + RECEIVE_FRAME_BASE + 2);
			id |= ((unsigned int)tmp << 16);
			tmp = inb(base + RECEIVE_FRAME_BASE + 3);
			id |= ((unsigned int)tmp << 8);
			tmp = inb(base + RECEIVE_FRAME_BASE + 4);
			id |= (unsigned int)tmp;
			id >>= 3;
  
			ucReadoutBase = RECEIVE_FRAME_BASE + 5;
		} else {
			tmp = inb(base + RECEIVE_FRAME_BASE + 1);
			id = (unsigned int)tmp << 3;
			tmp = inb(base + RECEIVE_FRAME_BASE + 2);
			id |= ((unsigned int)tmp >> 5);
  
			ucReadoutBase = RECEIVE_FRAME_BASE + 3;
  		}
		tmp_msg.id = id;

		k = ucLen;
		if(nRtrFrame) {
			//only for beauty, replace useless telegram content with zeros
			while(k--) tmp_msg.data[j++] = 0;
		} else {
			while(k--) {
				tmp = inb(base + ucReadoutBase + j);
				tmp_msg.data[j] = tmp;
				j++;
			}
		}

		tmp_msg.time = gethrtime(); //kjg_120619

		//step forward in fifo
		if(p->rx_msg_count >= RT_CAN_RX_MSG_BUF_SIZE) return -1;

		idx = p->rx_msg_write_idx;
		idx++;
		if(idx >= RT_CAN_RX_MSG_BUF_SIZE) idx = 0;

		p->rx_buf[idx].id = tmp_msg.id;
		p->rx_buf[idx].type = tmp_msg.type;
		for(j=0; j < 8; j++) p->rx_buf[idx].data[j] = tmp_msg.data[j];
		p->rx_buf[idx].time = tmp_msg.time; //kjg_120619

		p->rx_msg_write_idx = idx;
		p->rx_msg_count++;
	   
		outb(RELEASE_RECEIVE_BUFFER, base + COMMAND);
		udelay(1); //give time to settle
	} while((inb(base + CHIPSTATUS) & RECEIVE_BUFFER_STATUS) && (i--));
      
	return 0;
}

//Registered interrupt handlers.
//These simply call the general interrupt handler for the current
//line to do the work.
static unsigned int rt_can_share_isr(unsigned int irq_no, struct pt_regs *r)
{
	unsigned char data, tmp;
	int i, j;
	long long ht0, ht1, ht2; //kjg_180723

	j = 0;
	while(0 != (data = inb(RT_CAN_BASE_ADDR + RT_CAN_PORT_IRQ))) {
		tmp = 0x01;
		//kjg_180405 for(i=0; i < RT_CAN_CNT; i++) {
		for(i=0; i < max_can_ch; i++) {
			if((data & tmp) != 0x00) {
				ht0 = gethrtime(); //kjg_180723
				rt_can_isr((unsigned int)i, NULL);

				ht1 = gethrtime();
				ht2 = ht1 - ht0;
				if(ht2 >= rx_ht3[i]) {
					rx_ht3[i] = ht2;
					printk(KERN_WARNING "rt_can: isr(%d) %lld\n",
						i, ht2);
				}
			}
			tmp = tmp << 1;
		}

		j++;
		//kjg_180723 if(j >= 32) break; //kjg_180405 16->32
		if(j >= 300) break; //kjg_180723
	}

//kjg_180723
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
unsigned int rt_can_isr(unsigned int num, struct pt_regs *r)
{
    char loop=10;
    unsigned char status, port_sel;
    int err;
    unsigned int base;
    struct rt_can_struct *p;

	p = &(rt_can_table[num]);

	base = RT_CAN_BASE_ADDR;
	port_sel = inb(base + RT_CAN_PORT_SELECT);
	outb((unsigned char)(num + 1), base + RT_CAN_PORT_SELECT);

	while((status = inb(base + INTERRUPT_STATUS)) && (loop--)) {
		if(status & DATA_OVERRUN_INTERRUPT) {
			p->status |= CAN_ERR_OVERRUN;
			p->error_count++;
			outb(CLEAR_DATA_OVERRUN, base + COMMAND);
		}

		if(status & RECEIVE_INTERRUPT) {
			if((err = sja1000_read(num, p))) {
				p->error = err;
				p->error_count++;
				p->status |= CAN_ERR_QOVERRUN;
				//throw away last message which was refused by fifo
				outb(RELEASE_RECEIVE_BUFFER, base + COMMAND);
			}
		}

		/*kjg_180723 if(status & TRANSMIT_INTERRUPT) {
			if((err = sja1000_write(num, p))) {
				if(err == -ENODATA) {
				} else {
					p->error = err;
					p->error_count++;
					p->status |= CAN_ERR_QXMTFULL; //fatal error!!!
				}
			}
		}*/

		if(status & ERROR_PASSIV_INTERRUPT) {
			p->status |= CAN_ERR_BUSHEAVY;
			p->error_count++;
		}

		if(status & BUS_ERROR_INTERRUPT) {
			p->status |= CAN_ERR_BUSOFF;
			p->error_count++;
		}
    }

	outb(port_sel, base + RT_CAN_PORT_SELECT);

    //kjg_180723 rtl_hard_enable_irq(p->irq);
    return 0;
}

int CAN_Initialize(int num, int baud, int sjw, int ext, int fd_flag, int data_rate, int crc)
{ //kjg_180723
	unsigned char clkdivider, tmp;
	unsigned short btr0btr1;
	int rtn;
	unsigned int base;
	struct rt_can_struct *p;

	p = &(rt_can_table[num]);
	base = RT_CAN_BASE_ADDR;

	outb(ABORT_TRANSMISSION, base + COMMAND);
	outb(0, base + INTERRUPT_ENABLE);
	outb(RELEASE_RECEIVE_BUFFER, base + COMMAND);

	rtn = set_reset_mode(num);
	clkdivider = PELICAN_SINGLE;
	outb(clkdivider, base + CLKDIVIDER);

	//configure output control registers
	outb(OUTPUT_CONTROL_SETUP, base + OUTPUT_CONTROL);

	//configure acceptance code registers
	outb(0, base + ACCEPTANCE_CODE_BASE + 0);
	outb(0, base + ACCEPTANCE_CODE_BASE + 1);
	outb(0, base + ACCEPTANCE_CODE_BASE + 2);
	outb(0, base + ACCEPTANCE_CODE_BASE + 3);

	//configure all acceptance mask registers to don't care
	outb(0xFF, base + ACCEPTANCE_MASK_BASE + 0);
	outb(0xFF, base + ACCEPTANCE_MASK_BASE + 1);
	outb(0xFF, base + ACCEPTANCE_MASK_BASE + 2);
	outb(0xFF, base + ACCEPTANCE_MASK_BASE + 3);

	//configure bus timing registers
	btr0btr1 = (unsigned short)sja1000_baud_rate(baud, 0, sjw);
	outb((unsigned char)((btr0btr1 >> 8) & 0xFF), base + TIMING0);
	outb((unsigned char)((btr0btr1) & 0xFF), base + TIMING1);

	//clear any pending interrupt
	tmp = inb(base + INTERRUPT_STATUS);

	rtn = set_normal_mode(num, NORMAL_MODE);
	outb(INTERRUPT_ENABLE_SETUP, base + INTERRUPT_ENABLE);

	outb(RELEASE_RECEIVE_BUFFER, base + COMMAND);

	return 0;
}

/*kjg_180723 int CAN_Initialize(int num, int baud, int sjw, int ext, int fd_flag, int data_rate, int crc)
{ //kjg_180405
	unsigned char clkdivider, tmp;
	unsigned short btr0btr1;
	int rtn;
	unsigned int base;
	struct rt_can_struct *p;

	p = &(rt_can_table[num]);
	base = RT_CAN_BASE_ADDR;

	outb(ABORT_TRANSMISSION, base + COMMAND);
	outb(0, base + INTERRUPT_ENABLE);
	outb(RELEASE_RECEIVE_BUFFER, base + COMMAND);

	rtn = set_reset_mode(num);
	clkdivider = PELICAN_SINGLE;
	outb(clkdivider, base + CLKDIVIDER);

	//configure output control registers
	outb(OUTPUT_CONTROL_SETUP, base + OUTPUT_CONTROL);

	//configure acceptance code registers
	outb(0, base + ACCEPTANCE_CODE_BASE + 0);
	outb(0, base + ACCEPTANCE_CODE_BASE + 1);
	outb(0, base + ACCEPTANCE_CODE_BASE + 2);
	outb(0, base + ACCEPTANCE_CODE_BASE + 3);

	//configure all acceptance mask registers to don't care
	outb(0xFF, base + ACCEPTANCE_MASK_BASE + 0);
	outb(0xFF, base + ACCEPTANCE_MASK_BASE + 1);
	outb(0xFF, base + ACCEPTANCE_MASK_BASE + 2);
	outb(0xFF, base + ACCEPTANCE_MASK_BASE + 3);

	//configure bus timing registers
	btr0btr1 = (unsigned short)sja1000_baud_rate(baud, 0, sjw);
	outb((unsigned char)((btr0btr1 >> 8) & 0xFF), base + TIMING0);
	outb((unsigned char)((btr0btr1) & 0xFF), base + TIMING1);

	//clear any pending interrupt
	tmp = inb(base + INTERRUPT_STATUS);

	rtn = set_normal_mode(num, NORMAL_MODE);
	outb(INTERRUPT_ENABLE_SETUP, base + INTERRUPT_ENABLE);

	outb(RELEASE_RECEIVE_BUFFER, base + COMMAND);

	return 0;
}*/

int rt_can_set_param(int num, int baud, int sjw, int ext, int fd_flag, int data_rate, int crc, int terminal_r)
{ //kjg_180723
	struct rt_can_struct *p;

	p = &(rt_can_table[num]);

	if(max_can_ch == 0) return 0;

	if(num >= RT_CAN_CNT) {
		printk(KERN_WARNING "rt_can: rt_can_set_param error (-1) %d\n", num);
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
{ //kjg_180723
	unsigned char tmp1, mask;
	int baud, sjw, ext, fd_flag, data_rate, crc, terminal_r;
	unsigned int base;
	struct rt_can_struct *p;

	baud = set_parameter[num][1];
	sjw = set_parameter[num][2];
	ext = set_parameter[num][3];
	fd_flag = set_parameter[num][4];
	data_rate = set_parameter[num][5];
	crc = set_parameter[num][6];
	terminal_r = set_parameter[num][7];

	tx_ht3[num] = rx_ht3[num] = 0;

	p = &(rt_can_table[num]);

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
	base = RT_CAN_BASE_ADDR;
	//outb(terminal_resistor([0], base + RT_CAN_BD1_OUT_1);
	//outb(terminal_resistor([1], base + RT_CAN_BD2_OUT_1);

	tmp1 = 0x01;
	tmp1 = tmp1 << num;
	outb((unsigned char)(num + 1), base + RT_CAN_PORT_SELECT);

	mask = inb(base + RT_CAN_PORT_IRQ_MASK);
	mask &= (~tmp1); //IRQ_OFF
	outb(mask, base + RT_CAN_PORT_IRQ_MASK);

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

	CAN_Initialize(num, baud, sjw, ext, fd_flag, data_rate, crc);

	p->used |= 0x02; //mark setup done

	mask = inb(base + RT_CAN_PORT_IRQ_MASK);
	mask |= tmp1; //IRQ_ON
	outb(mask, base + RT_CAN_PORT_IRQ_MASK);

	return 0;
}

int rt_can_check_status(int num)
{ //kjg_180723
	struct rt_can_struct *p;

	p = &(rt_can_table[num]);

	return (int)p->status;
}

int rt_can_setup(int num, int baud)
{
	unsigned char tmp; //kjg_180723
	unsigned int base;
	struct rt_can_struct *p;

	if(num >= RT_CAN_CNT) {
		printk(KERN_WARNING "rt_can: rt_can_setup error (-1) %d\n", num);
		return -1;
	}

	p = &(rt_can_table[num]);
	
	if(p->used == 0) {
		printk(KERN_WARNING "rt_can: rt_can_setup error (-2) %d, %d\n",
			num, p->used);
		return -2;
	}

	tx_ht3[num] = rx_ht3[num] = 0; //kjg_180723

	base = RT_CAN_BASE_ADDR;
	tmp = 0x01;
	tmp = tmp << num;
	outb((unsigned char)(num + 1), base + RT_CAN_PORT_SELECT);

	p->error = 0; //init. last error code

	switch(baud) {
		case 1:
			p->status = 0; //kjg_180723
			p->rx_msg_write_idx = 0;
			p->rx_msg_read_idx = 0;
			p->rx_msg_count = 0;
			p->tx_msg_write_idx = 0;
			p->tx_msg_read_idx = 0;
			p->tx_msg_count = 0;
			p->tx_msg_error = 0;
			p->type = 0;
			set_reset_mode(num);
			break;
		case 2:
			MOD_INC_USE_COUNT;
			p->used |= 0x02; //mark setup done
			break;
		case (-1):
			set_reset_mode(num);
			break;
		case (-2):
			MOD_DEC_USE_COUNT;
			set_reset_mode(num);
			break;
		default:
			set_reset_mode(num);
			break;
	}

	return 0;
}

//switches the chip onto reset mode
int set_reset_mode(int num)
{
	unsigned char tmp, tmp2;
	unsigned int base;
	struct rt_can_struct *p;

	p = &(rt_can_table[num]);
	base = RT_CAN_BASE_ADDR;

	tmp2 = 0;
	tmp = inb(base + MODE);
	while((tmp != RESET_MODE) && tmp2 < 100) {
		outb(RESET_MODE, base + MODE);
		tmp2++;
		tmp = inb(base + MODE);
	}

	if(tmp != RESET_MODE) return -1;
	else return 0;
}

//switches the chip back from reset mode
int set_normal_mode(int num, unsigned char ucModifier)
{
	unsigned char tmp, tmp2;
	unsigned int base;
	struct rt_can_struct *p;

	p = &(rt_can_table[num]);
	base = RT_CAN_BASE_ADDR;

	tmp2 = 0;
	tmp = inb(base + MODE);
	while((tmp != ucModifier) && tmp2 < 100) {
		outb(ucModifier, base + MODE);
		tmp2++;
		tmp = inb(base + MODE);
	}

	if(tmp != ucModifier) return -1;
	else return 0;
}

//calculate BTR0BTR1 for odd bitrates
//Set communication parameters.
//param rate baud rate in Hz
//param clock frequency of sja1000 clock in Hz
//param sjw synchronization jump width (0-3) prescaled clock cycles
//param sampl_pt sample point in % (0-100) sets (TSEG1+2)/(TSEG1+TSEG2+3) ratio
//param flags fields BTR1_SAM, OCMODE, OCPOL, OCTP, OCTN, CLK_OFF, CBP
int sja1000_baud_rate(int rate, int flags, int set_sjw)
{
	int best_error = 1000000000;
	int error;
	int best_tseg=0, best_brp=0, best_rate=0, brp=0;
	int tseg=0, tseg1=0, tseg2=0;
	int clock = CLOCK_HZ / 2;
	u16 wBTR0BTR1;
	int sjw;
	int sampl_pt = 90;

	//some heuristic specials
	if (rate > ((1000000 + 500000) / 2)) sampl_pt = 75; //750000
	if (rate < ((12500 + 10000) / 2)) sampl_pt = 75; //11250
//	if (rate < ((100000 + 125000) / 2)) sjw = 1; //112500

	switch(set_sjw) { //kjg_100826
		case 0:
			sjw = 2;
			if(rate < ((100000 + 125000) / 2)) sjw = 1; //112500
			break;
		default:
			if(set_sjw < 0 || set_sjw > 4) set_sjw = 3;
			sjw = set_sjw - 1;
			break;
	}

	//tseg even = round down, odd = round up
	for(tseg = (0 + 0 + 2) * 2; tseg <= (MAX_TSEG2 + MAX_TSEG1 + 2) * 2 + 1;
		tseg++) {
		brp = clock / ((1 + tseg / 2) * rate) + tseg % 2;
		if((brp == 0) || (brp > 64)) continue;
		
		error = rate - clock / (brp * (1 + tseg / 2));
		if(error < 0) error = -error;
			
		if(error <= best_error) {
			best_error = error;
			best_tseg = tseg/2;
			best_brp = brp-1;
			best_rate = clock/(brp*(1+tseg/2));
		}
	}
	
	if(best_error && (rate / best_error < 10)) return 0;
	
	tseg2 = best_tseg - (sampl_pt * (best_tseg + 1)) / 100;
	
	if(tseg2 < 0) tseg2 = 0;
		
	if(tseg2 > MAX_TSEG2) tseg2 = MAX_TSEG2;
		
	tseg1 = best_tseg - tseg2 - 2;
	
	if(tseg1 > MAX_TSEG1) {
		tseg1 = MAX_TSEG1;
		tseg2 = best_tseg-tseg1-2;
	}

	wBTR0BTR1 = ((sjw<<6 | best_brp) << 8)
		| (((flags & BTR1_SAM) != 0)<<7 | tseg2<<4 | tseg1);

	return wBTR0BTR1;	
}

//Request port memory and register ISRs, if we cannot get the memory
//of all ports, release all already requested ports and return an error.
//return Success status, zero on success.
int init_module(void)
{
	unsigned char info[6];
    int errorcode=0, i, j;
    struct rt_can_struct *p;

	terminal_resistor[0] = terminal_resistor[1] = 0x0F;
	max_can_ch = 0;
	info[0] = inb(RT_CAN_BASE_ADDR + RT_CAN_BD1_INFO_1);
	info[1] = inb(RT_CAN_BASE_ADDR + RT_CAN_BD1_INFO_2);
	info[2] = inb(RT_CAN_BASE_ADDR + RT_CAN_BD1_INFO_3);
	if(info[0] == 0x41
		&& (info[1] >= 0x30 && info[1] <= 0x39)
		&& (info[2] >= 0x30 && info[2] <= 0x39)) {
	} else {
		outb(0x0F, RT_CAN_BASE_ADDR + RT_CAN_PORT_IRQ_MASK);
		info[3] = inb(RT_CAN_BASE_ADDR + RT_CAN_PORT_IRQ_MASK);
		outb(0xAF, RT_CAN_BASE_ADDR + RT_CAN_PORT_IRQ_MASK);
		info[4] = inb(RT_CAN_BASE_ADDR + RT_CAN_PORT_IRQ_MASK);
		outb(0x0F, RT_CAN_BASE_ADDR + RT_CAN_PORT_IRQ_MASK);
		info[5] = inb(RT_CAN_BASE_ADDR + RT_CAN_PORT_IRQ_MASK);

		if(info[3] == 0x0F && info[4] == 0xAF && info[5] == 0x0F) {
			max_can_ch = 4;
		}
	}

    for(i=0; i < max_can_ch; i++) {
		p = &(rt_can_table[i]);
		if(p->used > 0) {
		    if(-EBUSY == check_region(p->port, 8)) {
				errorcode = 10;
				break;
	    	}

		    request_region(p->port, 8, "rt_can_2p0b");
		    rt_can_request_irq(p->irq, p->isr);
	    	rt_can_setup(i, 1);
		}
    }

	printk(KERN_INFO "rt_can: board %x %x %x, %x %x %x, %d\n",
		info[0], info[1], info[2], info[3], info[4], info[5], max_can_ch);

    if(errorcode == 0 && max_can_ch >= 4) {
		printk(KERN_INFO
			"rt_can: CAN driver (version " VERSION ") sucessfully loaded.\n");

		thread_state = 1; //kjg_180810

		return pthread_create(&thread, NULL, rt_can_task, 0);
    } else {
		printk(KERN_WARNING
			"rt_can: cannot request all port regions,\nrt_can: giving up.\n");

		thread_state = 0; //kjg_180810

		outb(0x00, RT_CAN_BASE_ADDR + RT_CAN_PORT_IRQ_MASK);

		for(j=0; j < i; j++) {
		    p = &(rt_can_table[j]);
	    	if(p->used > 0) {
				rt_can_setup(i, -1);
				rt_can_free_irq(p->irq);
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
    struct rt_can_struct *p;

	outb(0x00, RT_CAN_BASE_ADDR + RT_CAN_PORT_IRQ_MASK);

	if(thread_state == 1) { //kjg_180810
		pthread_delete_np(thread); //kjg_180723
	}

    for(i=0; i < max_can_ch; i++) {
		p = &(rt_can_table[i]);
		if(p->used > 0) {
		    rt_can_free_irq(p->irq); //kjg_180810
		    rt_can_setup(i, -1);
	    	release_region(p->port, 8);
		}
    }

    printk(KERN_INFO "rt_can: unloaded.\n");
    return;
}
