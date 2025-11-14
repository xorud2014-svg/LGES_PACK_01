#ifndef __RT_CAN_H__
#define __RT_CAN_H__

//kjg_180723
#define DELAY_2US						500		//2us
#define DELAY_4US						1000	//4us
#define DELAY_8US						2000	//8us
#define DELAY_10US						2500	//10us
#define DELAY_40US						10000	//40us

spinlock_t rt_can_spinlock = SPIN_LOCK_UNLOCKED;

//internal convinience macros for interrupt handling
#define rt_can_irq_off(state)			rtl_critical(state)
#define rt_can_irq_on(state)			rtl_end_critical(state)
#define rt_can_request_irq(irq, isr)	rtl_request_global_irq(irq, isr); rtl_hard_enable_irq(irq)
#define rt_can_free_irq(irq)			rtl_free_global_irq(irq)

//Number and descriptions of serial ports to manage.  You also need
//to create an ISR (rt_canN_isr()) for each port.
#define RT_CAN_CNT 						8

#define RT_CAN_NAME						"rt_can_2p0b"
#define RT_CAN_BASE_BAUD 				500000
#define RT_CAN_STD_FLAG 				0
#define RT_CAN_FD_CLASSIC_FLAG			0 //kjg_180723
#define RT_CAN_FD_NORMAL_FLAG			1 //kjg_180723
#define RT_CAN_RX_MSG_BUF_SIZE			200
//kjg_180405 #define RT_CAN_TX_MSG_BUF_SIZE			50
#define RT_CAN_TX_MSG_BUF_SIZE			200	//kjg_180723 20->200

#define RT_CAN_BASE_ADDR				0x800
#define RT_CAN_PORT_SELECT				0x80
#define RT_CAN_PORT_IRQ					0x81
#define RT_CAN_PORT_IRQ_MASK			0x82
#define RT_CAN_BD1_INFO_1				0x85
#define RT_CAN_BD1_INFO_2				0x86
#define RT_CAN_BD1_INFO_3				0x87
#define RT_CAN_BD2_INFO_1				0x8A
#define RT_CAN_BD2_INFO_2				0x8B
#define RT_CAN_BD2_INFO_3				0x8C

//error codes
#define CAN_ERR_OK             	0x0000 //no error
#define CAN_ERR_XMTFULL        	0x0001 //transmit buffer full
#define CAN_ERR_OVERRUN        	0x0002 //overrun in receive buffer
#define CAN_ERR_BUSLIGHT       	0x0004 //bus error, errorcounter limit reached
#define CAN_ERR_BUSHEAVY       	0x0008 //bus error, errorcounter limit reached
#define CAN_ERR_BUSOFF         	0x0010 //bus error, 'bus off' state entered
#define CAN_ERR_QRCVEMPTY      	0x0020 //receive queue is empty
#define CAN_ERR_QOVERRUN       	0x0040 //receive queue overrun
#define CAN_ERR_QXMTFULL       	0x0080 //transmit queue full 
#define CAN_ERR_REGTEST        	0x0100 //test of controller registers failed
#define CAN_ERR_NOVXD          	0x0200 //Win95/98/ME only
#define CAN_ERR_RESOURCE       	0x2000 //can't create resource
#define CAN_ERR_ILLPARAMTYPE   	0x4000 //illegal parameter
#define CAN_ERR_ILLPARAMVAL    	0x8000 //value out of range
#define CAN_ERRMASK_ILLHANDLE	0x1C00 //wrong handle, handle error

//MSGTYPE
#define MSGTYPE_STATUS					0x80//used to mark pending status
#define MSGTYPE_EXTENDED				0x02//declares a extended frame
#define MSGTYPE_RTR						0x01//marks a remote frame
#define MSGTYPE_STANDARD				0x00//marks a standard frame

//kjg_180405_s
#include <stdbool.h> //kjg_180723

#define RT_CAN_TX_RX					0
#define RT_CAN_TX_ONLY					1
#define RT_CAN_RX_ONLY					2 //kjg_180723

#define MAX_TXQUEUE_ATTEMPTS			50 //kjg_180723

//sja1000 registers
#define CONTROL							0 //control(BASIC)
#define MODE                   			0 //mode(PELICAN)
#define COMMAND                			1
#define CHIPSTATUS             			2
#define INTERRUPT_STATUS       			3
#define INTERRUPT_ENABLE       			4 //acceptance code
#define TIMING0                			6 //bus timing 0
#define TIMING1                			7 //bus timing 1
#define OUTPUT_CONTROL         			8 //output control
#define TESTREG                			9

#define ARBIT_LOST_CAPTURE    			11 //transmit buffer: Identifier
#define ERROR_CODE_CAPTURE    			12 //RTR bit und data length code
#define ERROR_WARNING_LIMIT   			13 //start byte of data field
#define RX_ERROR_COUNTER      			14
#define TX_ERROR_COUNTER      			15

#define ACCEPTANCE_CODE_BASE  			16
#define RECEIVE_FRAME_BASE    			16
#define TRANSMIT_FRAME_BASE   			16

#define ACCEPTANCE_MASK_BASE  			20

#define RECEIVE_MSG_COUNTER   			29
#define RECEIVE_START_ADDRESS 			30

#define CLKDIVIDER            			31 //set bit rate and pelican mode

//important sja1000 register contents, MODE register
#define SLEEP_MODE             			0x10
#define ACCEPT_FILTER_MODE     			0x08
#define SELF_TEST_MODE         			0x04
#define LISTEN_ONLY_MODE       			0x02
#define RESET_MODE             			0x01
#define NORMAL_MODE            			0x00

//COMMAND register
#define CLEAR_DATA_OVERRUN     			0x08
#define RELEASE_RECEIVE_BUFFER 			0x04
#define ABORT_TRANSMISSION     			0x02
#define TRANSMISSION_REQUEST   			0x01

//CHIPSTATUS register
#define BUS_STATUS             			0x80
#define ERROR_STATUS           			0x40
#define TRANSMIT_STATUS        			0x20
#define RECEIVE_STATUS         			0x10
#define TRANS_COMPLETE_STATUS  			0x08
#define TRANS_BUFFER_STATUS    			0x04
#define DATA_OVERRUN_STATUS    			0x02
#define RECEIVE_BUFFER_STATUS  			0x01

//INTERRUPT STATUS register
#define BUS_ERROR_INTERRUPT    			0x80
#define ARBIT_LOST_INTERRUPT   			0x40
#define ERROR_PASSIV_INTERRUPT 			0x20
#define WAKE_UP_INTERRUPT      			0x10
#define DATA_OVERRUN_INTERRUPT 			0x08
#define ERROR_WARN_INTERRUPT   			0x04
#define TRANSMIT_INTERRUPT     			0x02
#define RECEIVE_INTERRUPT      			0x01

//INTERRUPT ENABLE register
#define BUS_ERROR_INTERRUPT_ENABLE    	0x80
#define ARBIT_LOST_INTERRUPT_ENABLE   	0x40
#define ERROR_PASSIV_INTERRUPT_ENABLE 	0x20
#define WAKE_UP_INTERRUPT_ENABLE      	0x10
#define DATA_OVERRUN_INTERRUPT_ENABLE 	0x08
#define ERROR_WARN_INTERRUPT_ENABLE   	0x04
#define TRANSMIT_INTERRUPT_ENABLE     	0x02
#define RECEIVE_INTERRUPT_ENABLE      	0x01

//OUTPUT CONTROL register
#define OUTPUT_CONTROL_TRANSISTOR_P1  	0x80
#define OUTPUT_CONTROL_TRANSISTOR_N1  	0x40
#define OUTPUT_CONTROL_POLARITY_1     	0x20
#define OUTPUT_CONTROL_TRANSISTOR_P0  	0x10
#define OUTPUT_CONTROL_TRANSISTOR_N0  	0x08
#define OUTPUT_CONTROL_POLARITY_0     	0x04
#define OUTPUT_CONTROL_MODE_1         	0x02
#define OUTPUT_CONTROL_MODE_0         	0x01

//TRANSMIT or RECEIVE BUFFER
#define BUFFER_EFF                    	0x80 //set for 29 bit identifier
#define BUFFER_RTR                    	0x40 //set for RTR request
#define BUFFER_DLC_MASK               	0x0F

//CLKDIVIDER register
#define CAN_MODE                      	0x80
#define CAN_BYPASS                    	0x40
#define RXINT_OUTPUT_ENABLE           	0x20
#define CLOCK_OFF                     	0x08
#define CLOCK_DIVIDER_MASK            	0x07

//additional informations
#define CLOCK_HZ                  		24000000 //crystal frequency

//time for mode register to change mode
#define MODE_REGISTER_SWITCH_TIME		100 //msec 

//some CLKDIVIDER register contents, hardware architecture dependend 
#define PELICAN_SINGLE  (CAN_MODE | CAN_BYPASS | 0x07 | CLOCK_OFF)
#define PELICAN_MASTER  (CAN_MODE | CAN_BYPASS | 0x07            )
#define PELICAN_DEFAULT (CAN_MODE |              0x07 | CLOCK_OFF)

//hardware depended setup for OUTPUT_CONTROL register
#define OUTPUT_CONTROL_SETUP 	( OUTPUT_CONTROL_TRANSISTOR_P1 | OUTPUT_CONTROL_TRANSISTOR_N1 | OUTPUT_CONTROL_TRANSISTOR_P0 | OUTPUT_CONTROL_TRANSISTOR_N0 | OUTPUT_CONTROL_MODE_1)

//the interrupt enables
#define INTERRUPT_ENABLE_SETUP	(RECEIVE_INTERRUPT_ENABLE | TRANSMIT_INTERRUPT_ENABLE | DATA_OVERRUN_INTERRUPT_ENABLE | BUS_ERROR_INTERRUPT_ENABLE | ERROR_PASSIV_INTERRUPT_ENABLE)

//the maximum number of handled messages in one interrupt 
#define MAX_MESSAGES_PER_INTERRUPT		8

//the maximum number of handled sja1000 interrupts in 1 handler entry
#define MAX_INTERRUPTS_PER_ENTRY		4

#define MAX_TSEG1  						15
#define MAX_TSEG2  						7
#define BTR1_SAM   						(1 << 1)

unsigned char terminal_resistor[2];
int max_can_ch;
//kjg_180405_e

struct rt_can_msg {
	unsigned int		id;

	unsigned char		type;
	unsigned char		length;
	unsigned char		can_fd_flag; //0:can_2.0B(default), 1:can_fd
	unsigned char		reserved1;

	unsigned char		data[64]; //kjg_180405 8->64

	long long			time; //kjg_120619
	unsigned int		timeStamp; //kjg_180405
};

//information about handled ports
struct rt_can_struct {
    int					magic;
    int					baud_base;
    int					port;
    int					irq;
    int					flag;
    unsigned int		(*isr)(unsigned int, struct pt_regs *);
    int					mode;
    int					used;
	    //0 if port region must be requested later by rt_can_set_param
		//1 if port region must be requested by init_module (standard)
    int 				error; //last error detected
	int					error_count;
	unsigned short 		status;
	unsigned short 		reserved1;
    int					type; //0:standard, 1:extedned

	int					rx_msg_write_idx;
	int					rx_msg_read_idx;
	int					rx_msg_count;
	int					tx_msg_write_idx;
	int					tx_msg_read_idx;
	int					tx_msg_count;
	int					tx_msg_error;

	struct rt_can_msg	rx_buf[RT_CAN_RX_MSG_BUF_SIZE];
	struct rt_can_msg	tx_buf[RT_CAN_TX_MSG_BUF_SIZE];
};

void *rt_can_task(void *); //kjg_180723
void tx_can_data(int); //kjg_180723

int rt_can_write(int, const char *);
int sja1000_write(int, struct rt_can_struct *);
int rt_can_read(int, char *);
int sja1000_read(int, struct rt_can_struct *);

static unsigned int rt_can_share_isr(unsigned int, struct pt_regs *);
//kjg_180405 static inline unsigned int rt_can_isr(unsigned int, struct pt_regs *);
unsigned int rt_can_isr(unsigned int, struct pt_regs *);

int CAN_Initialize(int, int, int, int, int, int, int);

int rt_can_set_param(int, int, int, int, int, int, int, int);
int rt_can_set_param_2(int); //kjg_180723
int rt_can_check_status(int);
int rt_can_setup(int, int);

int	set_reset_mode(int);
int	set_normal_mode(int, unsigned char);
int	sja1000_baud_rate(int, int, int);

int	init_module(void);
void cleanup_module(void);

#endif
