//rt_com
//RT-Linux kernel module for communication across serial lines.

#ifndef RT_COM_P_H
#define RT_COM_P_H

//static unsigned int rt_com_share_isr( unsigned int, struct pt_regs * );
static unsigned int rt_com0_isr( unsigned int, struct pt_regs * );
static unsigned int rt_com1_isr( unsigned int, struct pt_regs * );
#ifdef __SBC_EMCORE_V621__
//static unsigned int rt_com2_isr( unsigned int, struct pt_regs * );
//static unsigned int rt_com3_isr( unsigned int, struct pt_regs * );
#endif
static inline unsigned int rt_com_isr( unsigned int, struct pt_regs * );

spinlock_t rt_com_spinlock = SPIN_LOCK_UNLOCKED;

//internal convinience macros for interrupt handling
#define rt_com_irq_off(state)			rtl_critical(state)
#define rt_com_irq_on(state)			rtl_end_critical(state)
#define rt_com_request_irq(irq, isr)	rtl_request_global_irq(irq, isr); rtl_hard_enable_irq(irq)
#define rt_com_free_irq(irq)			rtl_free_global_irq(irq)

//number of bytes in port FIFO when a DATA_READY interrupt shall occur
#define RT_COM_FIFO_TRIGGER 8
//#define RT_COM_FIFO_TRIGGER 14

//status masks
#define RT_COM_DATA_READY   0x01	//not an error

//port register offsets
#define RT_COM_RXB  0x00
#define RT_COM_TXB  0x00
#define RT_COM_IER  0x01
#define RT_COM_IIR  0x02
#define RT_COM_FCR  0x02
#define RT_COM_LCR  0x03
#define RT_COM_MCR  0x04
#define RT_COM_LSR  0x05
#define RT_COM_MSR  0x06
#define RT_COM_DLL  0x00
#define RT_COM_DLM  0x01

//data buffer - organized as a FIFO
struct rt_buf_struct{
    int		head;
    int		tail;
    char	buf[RT_COM_BUF_SIZ];
};

//information about handled ports
struct rt_com_struct{
    int		magic;
    int		baud_base;
    int		port;
    int		irq;
    int		flag;
    unsigned int	(*isr)(unsigned int, struct pt_regs *);
    int		mode;
		//functioning mode
		//bit 0 : 0 for standard functioning mode (DSR needed on TX)
        //        1 for comunication without hand shake signals (only RXD-TXD-GND)
       	//bit 1 : 1 for hardware flow control (RTS-CTS)
       	//NOTE : When you select a mode that use hand shake signals pay
        //attention that no input signals (CTS,DSR,RI,DCD) must be floating.
    int		used;
		//1 if port region must be requested by init_module (standard)
	    //0 if port region must be requested later by rt_com_set_param
    int error; //last error detected
    int type;
    int ier;  //copy of chip register
    int mcr;  //copy of the MCR internal register
    IRQ_CALLBACK_FN callback;	//called when chars rx'd
    struct rt_buf_struct ibuf;
    struct rt_buf_struct obuf;
};

//Some hardware description
#define RT_COM_BASE_BAUD 115200
//#define RT_COM_BASE_BAUD 230400
#define RT_COM_STD_COM_FLAG 0

#endif
