//rt_com
//RT-Linux kernel module for communication across serial lines.

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

#include "rt_com.h"
#include "rt_comP.h"

#define RT_COM_NAME "rt_com"

//amount of free space on input buffer for RTS reset
#define RT_COM_BUF_LOW  RT_COM_BUF_SIZ / 3

//amount of free space on input buffer for RTS set
#define RT_COM_BUF_HI   RT_COM_BUF_SIZ * 2 / 3

//amount of free space on input buffer for buffer full error
#define RT_COM_BUF_FULL 20

//Number and descriptions of serial ports to manage.  You also need
//to create an ISR (rt_comN_isr()) for each port.
#ifdef __SBC_EMCORE_V621__
#define RT_COM_CNT 4

//Default: mode=0 - DSR needed on TX, no hw flow control
//         used=0 - port and irq setting by rt_com_set_param.
//           	If you want to work like
//              	a standard rt_com you can set used=1.
struct rt_com_struct rt_com_table[RT_COM_CNT] =
{
	{0, RT_COM_BASE_BAUD, 0x3F8, 4, RT_COM_STD_COM_FLAG, rt_com0_isr, 1, 1},
	{0, RT_COM_BASE_BAUD, 0x2F8, 3, RT_COM_STD_COM_FLAG, rt_com1_isr, 1, 1}
//	{0, RT_COM_BASE_BAUD, 0x3E8, 10, RT_COM_STD_COM_FLAG, rt_com2_isr, 1, 1},
//	{0, RT_COM_BASE_BAUD, 0x2E8, 11, RT_COM_STD_COM_FLAG, rt_com3_isr, 1, 1}
};
#else //mark533, mark800, a5362
#define RT_COM_CNT 2

//Default: mode=0 - DSR needed on TX, no hw flow control
//         used=0 - port and irq setting by rt_com_set_param.
//           	If you want to work like
//              	a standard rt_com you can set used=1.
struct rt_com_struct rt_com_table[RT_COM_CNT] =
{
	{0, RT_COM_BASE_BAUD, 0x3F8, 4, RT_COM_STD_COM_FLAG, rt_com0_isr, 1, 1},
	{0, RT_COM_BASE_BAUD, 0x2F8, 3, RT_COM_STD_COM_FLAG, rt_com1_isr, 1, 1}
};
#endif

//Remaining free space of buffer
//@return amount of free space remaining in a buffer (input or output)
//Note that, although the buffer is RT_COM_BUF_SIZ bytes long, it can
//only hold RT_COM_BUF_SIZ - 1 bytes.
#define rt_com_buff_free(head, tail) (RT_COM_BUF_SIZ - 1 - ((head - tail) & (RT_COM_BUF_SIZ - 1)))

//Enable hardware fifo buffers.
//This requires an 16550A or better !
//@param base    Port base address to work on.
//@param trigger Bytecount to buffer before an interrupt.
//@param clr     Clear fifos: 0= no reset, 2= RCVR fifo reset, 4= XMIT fifo reset
static inline void rt_com_enable_fifo(int base, int trigger, int clr)
{
    switch(trigger) {
		case 0:
			outb(0x00, base + RT_COM_FCR);	break;
    	case 1:
			outb(0x01 | clr, base + RT_COM_FCR);	break;
    	case 4:
			outb(0x41 | clr, base + RT_COM_FCR);	break;
    	case 8:
			outb(0x81 | clr, base + RT_COM_FCR);	break;
    	case 14:
			outb(0xC1 | clr, base + RT_COM_FCR);	break;
    	default:	break;
    }
}

//Returns amount of free space remaining in the output buffer.
//@param com    Port to use; corresponding to internal numbering scheme.
//@return       Number of bytes or -1 on error
int rt_com_bout_free(unsigned int com)
{
    if(com < RT_COM_CNT) {
		struct rt_com_struct *p = &(rt_com_table[com]);
		struct rt_buf_struct *b = &(p->obuf);

		if(p->used > 0) return (rt_com_buff_free(b->head, b->tail));
   	}

    return -1;
}

//Clear input buffer.
//@param com    Port to use; corresponding to internal numbering scheme.
//@return       -1 on error or 0 if all right
int rt_com_clr_in(unsigned int com)
{
    if(com < RT_COM_CNT) {
		long state;
		struct rt_com_struct *p = &(rt_com_table[com]);
		struct rt_buf_struct *b = &(p->ibuf);

		if(p->used > 0) {
		    rt_com_irq_off(state);
	    	b->tail = b->head;
		    if(RT_COM_FIFO_TRIGGER > 0)
				rt_com_enable_fifo(p->port, RT_COM_FIFO_TRIGGER, 0x02);
	    	rt_com_irq_on(state);
		    if(p->mode & 0x02) { //if hardware flow set RTS
				p->mcr |= 0x02;
				outb(p->mcr, p->port + RT_COM_MCR);
	    	}
	    	return 0;
		}
    }

    return -1;
}

//Clear output buffer.
//@param com    Port to use; corresponding to internal numbering scheme.
//@return       -1 on error or 0 if all right
int rt_com_clr_out(unsigned int com)
{
    if(com < RT_COM_CNT) {
		long state;
		struct rt_com_struct *p = &(rt_com_table[com]);
		struct rt_buf_struct *b = &(p->obuf);
	
		if(p->used > 0) {
		    rt_com_irq_off(state);
	    	p->ier &= ~0x02;
		    outb (p->ier, p->port + RT_COM_IER);
		    b->tail = b->head;
	    	if(RT_COM_FIFO_TRIGGER > 0)
				rt_com_enable_fifo(p->port, RT_COM_FIFO_TRIGGER, 0x04);
	    	rt_com_irq_on(state);
	    	return 0;
		}
    }

    return -1;
}

//Set functioning mode.
//@param com    Port to use; corresponding to internal numbering scheme.
//@param mode   functioning mode
//@return       -1 on error or 0 if all right
int rt_com_set_mode(unsigned int com, int mode)
{
    if(com < RT_COM_CNT) {
		struct rt_com_struct *p = &(rt_com_table[com]);

		if(p->used > 0) {
		    p->mode = mode;
		    if(p->used & 0x02) { //if com setup done
				if(mode & 0x01) {
					//if no hs signals disable modem interrupts
					p->ier &= ~0x08;
				} else {
					//else enable it
				    p->ier |= 0x08;
				}
				outb(p->ier, p->port + RT_COM_IER);
	    	}
	    	return 0;
		}
    }
    return( -1 );
}

//Set output signal for modem control (DTR, RTS).
//@param com    Port to use; corresponding to internal numbering scheme.
//@param signal Output signals: DTR or RTS.
//@param value  Status: 0 or 1.
//@return       -1 on error or 0 if all right
int rt_com_wr_modem(unsigned int com, int signal, int value)
{
    if(com < RT_COM_CNT) {
		struct rt_com_struct *p = &(rt_com_table[com]);

		if(p->used > 0) {
		    if(value) p->mcr |= signal;
	    	else p->mcr &= ~signal;
	    	outb(p->mcr, p->port+RT_COM_MCR);
	    	return 0;
		}
    }

    return -1;
}

//Read input signal from modem (CTS, DSR, RI, DCD).
//@param com    Port to use; corresponding to internal numbering scheme.
//@param signal Input signals: CTS, DSR, RI, DCD or any combination.
//@return       -1 on error or input signal status (0 or 1) as follows:
//               bit4 = CTS; bit5 = DSR; bit6 = RI; bit7 = DCD.
int rt_com_rd_modem(unsigned int com, int signal)
{
    if(com < RT_COM_CNT) {
		struct rt_com_struct *p = &(rt_com_table[com]);
		if(0 < p->used) {
	    	return	(inb(p->port + RT_COM_MSR) & signal);
		}
    }

    return -1;
}

//Return last error detected.
//@param com    Port to use; corresponding to internal numbering scheme.
//@return       bit 0 :1 = Input buffer overflow
//              bit 1 :1 = Receive data overrun
//              bit 2 :1 = Parity error
//              bit 3 :1 = Framing error
//              bit 4 :1 = Break detected
int rt_com_error(unsigned int com)
{
    int tmp=0;

    if(com < RT_COM_CNT) {
		struct rt_com_struct *p = &(rt_com_table[com]);

		tmp = p->error;
		p->error = 0;
    }

    return tmp;
}

//Write data to a line.
//@param com    Port to use; corresponding to internal numbering scheme.
//@param buffer Address of data.
//@param count  Number of bytes to write.
void rt_com_write(unsigned int com, const char *buffer, int count)
{
    const char *data = buffer;

    if(com < RT_COM_CNT) {
		//kjg_t unsigned char sta;
		long state;
		struct rt_com_struct *p = &(rt_com_table[com]);
		struct rt_buf_struct *b = &(p->obuf);

		if(p->used > 0) {
			/*kjg_t p->mcr |= 0x02;
			outb(p->mcr, p->port+RT_COM_MCR);*/

		    rt_com_irq_off(state);
		    while(--count >= 0) {
				//put byte into buffer, move pointers to next elements
				b->buf[b->head++] = *data++;
				//if( head == RT_COM_BUF_SIZ ),
				//wrap head to the first buffer element
				b->head &= (RT_COM_BUF_SIZ - 1);
	    	}
		    p->ier |= 0x02;
		    outb(p->ier, p->port + RT_COM_IER);
		    rt_com_irq_on(state);

            /*kjg_t while((sta = inb(p->port + RT_COM_LSR)) & 0x20) {}
			p->mcr &= ~0x02;
			outb(p->mcr, p->port+RT_COM_MCR);*/
		}
    }
}

//Read data we got from a line.
//@param com  Port to use corresponding to internal numbering scheme.
//@param ptr  Address of data buffer. Needs to be of size > cnt !
//@param cnt  Number of bytes to read.
//@return     Number of bytes actually read.
int rt_com_read(unsigned int com, char *ptr, int cnt)
{
    int done=0;

    if(com < RT_COM_CNT) {
		long state;
		struct rt_com_struct *p = &(rt_com_table[com]);
		struct rt_buf_struct *b = &(p->ibuf);

		if(p->used > 0) {
		    rt_com_irq_off(state);
		    while ((b->head != b->tail) && (--cnt >= 0)) {
				done++;
				*ptr++ = b->buf[b->tail++];
				b->tail &= (RT_COM_BUF_SIZ - 1);
			}
			rt_com_irq_on(state);
			if((p->mode & 0x02)
				&& (rt_com_buff_free(b->head, b->tail) > RT_COM_BUF_HI)) {
				//if hardware flow and enough free space on input buffer
				//then set RTS
				p->mcr |= 0x02;
				outb(p->mcr, p->port+RT_COM_MCR);
			}
		}
    }

    return done;
}

//Get first byte from the write buffer.
//@param p  rt_com_struct of the line we are writing to.
//@param c  Address to put the char in.
//@return   Number of characters we actually got.
static inline int rt_com_irq_get(struct rt_com_struct *p, unsigned char *c)
{
    struct rt_buf_struct *b = &(p->obuf);

    if(b->head != b->tail) {
		*c = b->buf[b->tail++];
		b->tail &= (RT_COM_BUF_SIZ - 1);
		return 1;
    }

    return 0;
}

//Concatenate a byte to the read buffer.
//@param p   rt_com_struct of the line we are writing to.
//@param ch  Byte to put into buffer.
static inline void rt_com_irq_put(struct rt_com_struct *p, unsigned char ch)
{
    struct rt_buf_struct *b = &(p->ibuf);

    b->buf[b->head++] = ch;
    b->head &= (RT_COM_BUF_SIZ - 1);
}

//Registered interrupt handlers.
//These simply call the general interrupt handler for the current
//line to do the work.
/*static unsigned int rt_com_share_isr(unsigned int num, struct pt_regs *r)
{
	unsigned char data, tmp;
	int i, j;

	j = 0;
	while(0 != (data = inb(0x801))) {
		tmp = 0x01;
		for(i=0; i < RT_COM_CNT; i++) {
			if((data & tmp) != 0x00) {
				rt_com_isr((unsigned int)i, NULL);
			}
			tmp = tmp << 1;
		}
		j++;
		if(j > 10) break;
	}

	return 0;
}*/

static unsigned int rt_com0_isr(unsigned int num, struct pt_regs *r)
{
	return rt_com_isr(0, NULL);
}

static unsigned int rt_com1_isr(unsigned int num, struct pt_regs *r)
{
	return rt_com_isr(1, NULL);
}
#ifdef __SBC_EMCORE_V621__
//static unsigned int rt_com2_isr(unsigned int num, struct pt_regs *r)
//{
//	return rt_com_isr(2, NULL);
//}
//
//static unsigned int rt_com3_isr(unsigned int num, struct pt_regs *r)
//{
//	return rt_com_isr(3, NULL);
//}
#endif

//Real interrupt handler.
//Called by the registered ISRs to do the actual work.
//@param com Port to use corresponding to internal numbering scheme.
unsigned int rt_com_isr(unsigned int com, struct pt_regs *r)
{
    char loop=4;
    char toFifo=16;
    unsigned char data, sta;
    int buff, trans;
    int rxd_bytes=0;
    struct rt_com_struct *p;
    struct rt_buf_struct *b;
    unsigned int B;

	p = &(rt_com_table[com]);
	b = &(p->ibuf);
	B = p->port;

    do {
		//get available data from port
		sta = inb(B + RT_COM_LSR);
		if(0x1e & sta) p->error = sta & 0x1e;

		while(RT_COM_DATA_READY & sta) {
		    data = inb(B + RT_COM_RXB);
		    rt_com_irq_put(p, data);
	    	rxd_bytes = 1;
		    sta = inb(B + RT_COM_LSR);
		    if(0x1e & sta) p->error = sta & 0x1e;
		}

		//controls on buffer full and RTS clear on hardware flow control
		buff = rt_com_buff_free(b->head, b->tail);
		if(buff < RT_COM_BUF_FULL) p->error = RT_COM_BUFFER_FULL;

		if((p->mode & 0x02) && (buff < RT_COM_BUF_LOW)) {
		    p->mcr &= ~0x02;
		    outb(p->mcr, p->port+RT_COM_MCR);
		}

		//if possible, put data to port
		sta = inb( B + RT_COM_MSR );
		if(((0x20 & sta)
	       && ((0x10 & sta) || (0 == (p->mode & 0x02)))) || (p->mode & 0x01)) {
		    //(DSR && (CTS || Mode==no hw flow)) or Mode==no hand shake
            if((sta = inb(B + RT_COM_LSR)) & 0x20) {
				// if (THRE==1) i.e. transmitter empty
	    		if(0 != (trans = rt_com_irq_get(p, &data))) {
					//if data in output buffer
				    do {
						outb(data, B + RT_COM_TXB);
			    	} while((--toFifo > 0)
						&& (0 != (trans = rt_com_irq_get(p, &data))));
				}

				if(!trans) {
				    //no more data in output buffer,
					//disable Transmitter Holding Register Empty Interrupt
		    		p->ier &= ~0x02;
			    	outb(p->ier, B + RT_COM_IER);
		    	}
	    	}
		}

		//check the low nibble of IIR to check
		//if there is another pending interrupt
	    //Note: why is it done at most 4 times ?
    } while(1 != ((inb(B + RT_COM_IIR) & 0x0f)) && (--loop > 0));

    if(rxd_bytes) {
		//We received 1+ bytes. If user requested a callback, invoke it.
		if(p->callback) (*(p->callback))(com);
    }

    rtl_hard_enable_irq(p->irq);
    return 0;
}

//Setup one port
//Calls from init_module + cleanup_module have baud == 0; in these
//cases we only do some cleanup.
//To allocate a port, give usefull setup parameter, to deallocate
//give negative baud.
//@param com        Number corresponding to internal port numbering scheme.
//                  This is esp. the index of the rt_com_table to use.
//@param baud       Data transmission rate to use [Byte/s].
//@param parity     Parity for transmission protocol.
//                  (RT_COM_PARITY_EVEN, RT_COM_PARITY_ODD or RT_COM_PARITY_NONE)
//@param stopbits   Number of stopbits to use. 1 gives you one stopbit, 2
//                  actually gives really two stopbits for wordlengths of
//                  6 - 8 bit, but 1.5 stopbits for a wordlength of 5 bits.
//@param wordlength Number of bits per word (5 - 8 bits).
//@return           -1 if error, 0 if all right
int rt_com_setup(unsigned int com, int baud, unsigned int parity,
	unsigned int stopbits, unsigned int wordlength)
{
	if(com < RT_COM_CNT) { //!!
		struct rt_com_struct *p = &(rt_com_table[com]);
		unsigned int base = p->port, divider, par = parity;

		if(0 == p->used) return (-EBUSY);

		//Stop everything, set DLAB
		outb(0x00, base + RT_COM_IER);
		outb(0x80, base + RT_COM_LCR);

		//clear irq
		inb(base + RT_COM_IIR);
		inb(base + RT_COM_LSR);
		inb(base + RT_COM_RXB);
		inb(base + RT_COM_MSR);

		p->error = 0; //!! init. last error code
		p->callback = 0;

		if(0 == baud) {
			//return
		} else if(0 > baud) {
			MOD_DEC_USE_COUNT;
		} else {
			MOD_INC_USE_COUNT;
			divider = p->baud_base / baud;
			outb(divider % 256, base + RT_COM_DLL);
			outb(divider / 256, base + RT_COM_DLM);
			//bits 3,4 + 5 determine parity - clear all other bits
			par &= 0x38;
			//set transmission parameters and clear DLAB
			outb((wordlength - 5) + ((stopbits - 1) << 2) + par,
				base + RT_COM_LCR);
			p->mcr = RT_COM_DTR + RT_COM_RTS + RT_COM_Out1 + RT_COM_Out2;
			outb(p->mcr, base + RT_COM_MCR);
			if(p->mode & 0x01) {
				//if no hs signals enable only receiver interrupts
				p->ier = 0x05;
			} else {
				//else enable receiver and modem interrupts
				p->ier = 0x0D;
			}
			outb(p->ier, base + RT_COM_IER);
			rt_com_enable_fifo(base, RT_COM_FIFO_TRIGGER, 0x06);
			p->used |= 0x02; //mark setup done
		}

		return 0;
    } else {
    	return (-ENODEV);
	}
}

//Optional callback function.
//If the user supplies a callback function, it is called whenever
//characters are received.
//*** NOTE *** The callback function is called in the context of the
//interrupt handler!
//@param com	Port to use; corresponding to internal numbering scheme.
//@param fn	Callback function (0 = de-register callback function).
//@return	Pointer to previously-installed callback function.
IRQ_CALLBACK_FN rt_com_set_callback (unsigned int com, IRQ_CALLBACK_FN fn)
{
    struct rt_com_struct *p;
    IRQ_CALLBACK_FN old_fn;

    if(com >= RT_COM_CNT) return 0;

    p = &(rt_com_table[com]);

    old_fn = p->callback;
    p->callback = fn;

    return old_fn;
}

//Port and irq setting for a specified COM.
//This must run in standard Linux context !
//@param com    Port to use; corresponding to internal numbering scheme.
//@param port   port address, if zero, use standard value from rt_com_table,
//              if negative, deinitialize.
//@param irq    irq address, if zero, use standard value from rt_com_table
//@return       0 if all right, -1 if port is used yet, -2 if error on port region request
int rt_com_set_param(unsigned int com, int port, int irq)
{
    if(com >= RT_COM_CNT) {
		return (-ENODEV);
    } else {
		struct rt_com_struct *p = &(rt_com_table[com]);

		if(0 == p->used) {
	    	if(0 != port) p->port = port;
	    	if(0 != irq) p->irq = irq;
	    	if(-EBUSY == check_region(p->port, 8)) {
				return (-EBUSY);
	    	}
	    	request_region(p->port, 8, "rt_com");
	    	rt_com_request_irq(p->irq, p->isr);
	    	rt_com_setup(com, 0, 0, 0, 0);
	    	p->used = 1;
		} else {
		    if(0 > port) {
				rt_com_setup(com, 0, 0, 0, 0);
				rt_com_free_irq(p->irq);
				release_region(p->port, 8);
				p->used = 0;
		    } else {
				return (-EBUSY);
	    	}
		}
    }

    return 0;
}

//Initialization
//Request port memory and register ISRs, if we cannot get the memory
//of all ports, release all already requested ports and return an error.
//@return Success status, zero on success.
int init_module(void)
{
    int errorcode=0, i, j;
    struct rt_com_struct *p;

    for(i=0; i < RT_COM_CNT; i++) {
		p = &(rt_com_table[i]);
		if(p->used > 0) {
		    if(-EBUSY == check_region(p->port, 8)) {
				errorcode = 1;
				break;
	    	}
		    request_region(p->port, 8, "rt_com");
		    rt_com_request_irq(p->irq, p->isr);
	    	rt_com_setup(i, 0, 0, 0, 0);
		}
    }

    if(errorcode != 0) {
		printk(KERN_WARNING
			"rt_com: cannot request all port regions, giving up.\n");
		for(j=0; j < i; j++) {
		    p = &(rt_com_table[j]);
	    	if(0 < p->used) {
				rt_com_free_irq(p->irq);
				release_region(p->port, 8);
	    	}
		}
    	return errorcode;
    }

	printk(KERN_INFO
	"rt_com: serial port driver (version " VERSION ") sucessfully loaded.\n");
    return errorcode;
}

//Cleanup
//Unregister ISR and releases memory for all ports
void cleanup_module(void)
{
    int i;
    struct rt_com_struct *p;

    for(i=0; i < RT_COM_CNT; i++) {
		p = &(rt_com_table[i]);
		if(p->used > 0) {
		    rt_com_free_irq(p->irq);
		    rt_com_setup(i, 0, 0, 0, 0);
	    	release_region(p->port, 8);
		}
    }
    printk(KERN_INFO "rt_com: unloaded.\n");

    return;
}
