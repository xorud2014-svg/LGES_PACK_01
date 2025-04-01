//rt_com
//RT-Linux kernel module for communication across serial lines.

#ifndef RT_COM_H
#define RT_COM_H

typedef void (*IRQ_CALLBACK_FN) (unsigned int com_port);

extern void cleanup_module( void );
extern int  init_module( void );

//amount of free space remaining in out buffer
extern int  rt_com_bout_free( unsigned int);

extern int  rt_com_clr_in( unsigned int);
extern int  rt_com_clr_out( unsigned int);

//read input signal from modem (CTS,DSR,RI,DCD)
extern int  rt_com_rd_modem (unsigned int com, int segnal);

extern int  rt_com_read( unsigned int, char *, int );
extern int  rt_com_setup( unsigned int, int, unsigned int, unsigned int, unsigned int );

//set call-back function for when data rx'd
extern IRQ_CALLBACK_FN rt_com_set_callback (unsigned int com, IRQ_CALLBACK_FN fn);
//port & irq setting for a specified com
extern int  rt_com_set_param( unsigned int com, int port, int irq );

//set functioning mode
extern int  rt_com_set_mode( unsigned int com, int mode );

extern void rt_com_write( unsigned int com, const char *buffer, int count );

//set output signal for modem control (DTR,RTS)
extern int  rt_com_wr_modem( unsigned int com, int signal, int value );

//return last error detected
extern int  rt_com_error( unsigned int com );

//parity flags
#define RT_COM_PARITY_EVEN    0x18
#define RT_COM_PARITY_NONE    0x00
#define RT_COM_PARITY_ODD     0x08

//size of internal queues (power of 2)
#define RT_COM_BUF_SIZ      0x1000

//modem control signal and modem status signal definition
#define RT_COM_DTR            0x01      //data Terminal Ready
#define RT_COM_RTS            0x02      //Request To Send
#define RT_COM_Out1           0x04
#define RT_COM_Out2           0x08
#define RT_COM_CTS            0x10
#define RT_COM_LoopBack       0x10
#define RT_COM_DSR            0x20
#define RT_COM_RI             0x40
#define RT_COM_DCD            0x80

//error code masks
#define RT_COM_BUFFER_FULL    0x01
#define RT_COM_OVERRUN        0x02
#define RT_COM_PARITY         0x04
#define RT_COM_FRAME          0x08
#define RT_COM_BREAK          0x10

//functioning mode
#define RT_COM_DSR_ON_TX      0x00
#define RT_COM_NO_HAND_SHAKE  0x01
#define RT_COM_HW_FLOW	      0x02

#endif
