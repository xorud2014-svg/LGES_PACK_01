#ifndef __IOCOMM_DEF_H__
#define __IOCOMM_DEF_H__

//packet
#define	MAX_IO_COMM_PACKET_LENGTH	(256*10)
#define MAX_IO_COMM_PACKET_COUNT		32
#define MAX_IO_COMM_RETRY_CNT			1

//cmd
#define	IO_COMM_SEND_CMD_REQUEST	0x01
#define IO_COMM_SEND_CMD_TEMP		0x02
#define IO_COMM_SEND_CMD_CONTROL	0x03

//Signal
#define IO_COMM_SIG_INITIALIZE		0
#define IO_COMM_SIG_MEASURE		1
#define IO_COMM_SIG_MEASURE_ERROR	2

//readType
#define READ_T							0
#define READ_V							1
#define READ_I							2
#define READ_T_2						3
#define MAX_METER_COUNT					8
#define MAX_METER_CH					8

//Temp ADDR
#define TEMP_ADDR_1					"01"
#define TEMP_ADDR_2					"02"
#define TEMP_ADDR_3					"03"
#define TEMP_ADDR_4					"04"

//IO_COMM ADDR
#define	IO_ADDR_1					"50"
#define	IO_ADDR_2					"51"
#define	IO_ADDR_3					"52"
#define	IO_ADDR_4					"53"
#define	IO_ADDR_5					"54"
#define	IO_ADDR_6					"55"
#define	IO_ADDR_7					"56"
#define	IO_ADDR_8					"57"
#define	IO_ADDR_9					"58"
#define	IO_ADDR_10					"59"
#define	IO_ADDR_11					"5A"
#define	IO_ADDR_12					"5B"
#define	IO_ADDR_13					"5C"
#define	IO_ADDR_14					"5D"
#define	IO_ADDR_15					"5E"
#define	IO_ADDR_16					"5F"
#define CALI_ADDR_1					"60"

//IO_COMM Command
#define	CMD_COMM_CHK				"CHK"
#define	CMD_NACK					"NAK"
#define	CMD_WR1						"WR1"
#define	CMD_RD1						"RD1"
#define	CMD_RESET					"RST"
#define	CMD_IO1						"IO1"
#define	CMD_II1						"II1"
#define CMD_EO1						"EO1"
#define CMD_EO2						"EO2"
#define CMD_EO3						"EO3"
#define CMD_EO4						"EO4"
#define CMD_EO5						"EO5"
#define CMD_EO6						"EO6"
#define CMD_EO7						"EO7"
#define CMD_EO8						"EO8"
#define CMD_EI1						"EI1"
#define CMD_EI2						"EI2"
#define CMD_EI3						"EI3"
#define CMD_EI4						"EI4"
#define CMD_EI5						"EI5"
#define CMD_EI6						"EI6"
#define CMD_EI7						"EI7"
#define CMD_EI8						"EI8"
#define CMD_CO1						"CO1"
#define CMD_INI						"INI"
#define CMD_REQ						"REQ"

//CB_7018, IO_COMM delimiter
#define SND_STX						'@'
#define RCV_STX						'!'

//Output 1
#define O_BIT_J_LATCH_SOL			0x01
#define O_BIT_J_MAIN_SOL			0x02
#define O_BIT_J_CLAMP_SOL			0x04
#define O_BIT_J_FAN_1				0x08
#define O_BIT_J_FAN_2				0x10
#define O_BIT_J_TOWER_GREEN_LAMP	0x20
#define O_BIT_J_TOWER_AMBER_LAMP	0x40
#define O_BIT_J_TOWER_RED_LAMP		0x80

//Jig1
// input data1
#define I_BIT_J_LATCH_CYL_OPEN_L	0x01
#define I_BIT_J_LATCH_CYL_OPEN_R	0x02
#define I_BIT_J_LATCH_CYL_CLOSE_L	0x04
#define I_BIT_J_LATCH_CYL_CLOSE_R	0x08
#define I_BIT_J_MAIN_CYL_UP			0x10
#define I_BIT_J_MAIN_CYL_DOWN		0x20
#define I_BIT_J_CLAMP_CYL_OPEN		0x40
#define I_BIT_J_CLAMP_CYL_CLOSE		0x80

// input data2
#define I_BIT_J_TRAY_IN				0x01
#define I_BIT_J_SMOKE_1				0x02
#define I_BIT_J_SMOKE_2				0x04

//signal
#define IO_COMM_SIG_INIT				0
#define IO_COMM_SIG_MEASURE				1
#define IO_COMM_SIG_MEASURE_ERROR		2

#endif
