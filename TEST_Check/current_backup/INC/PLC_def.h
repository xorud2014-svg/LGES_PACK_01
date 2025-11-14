#ifndef __PLC_DEF_H__
#define __PLC_DEF_H__

//PLC ADDRESS define
#define PLC_DOOR_OPEN  				  "M000002"
#define PLC_DOOR_OPEN_STATE 		  "Y000028"
#define PLC_DOOR_CLOSE  			  "M000003"
#define PLC_INIT		  			  "M000004"
#define PLC_DOOR_CLOSE_STATE    	  "Y000029"
#define PLC_FAULT_READ   			  "D001000"

//PLC RS232C CONTROL CODE
#define NUL         0x00    //Null
#define STX         0x02    //start of text
#define ETX         0x03    //end of text
#define EOT         0x04    //end of transmission
#define ENQ         0x05    //enquiry
#define PLC_ACK     0x06    //Acknowledge
#define PLC_LF      0x0A    //line feed
#define PLC_CL      0x0C    //clear
#define PLC_CR      0x0D    //camiage return
#define PLC_NACK    0x15    //negative Acknowledge
#define GOOD        0x47    //Good
#define NO_GOOD     0x4E    //No Good

//PLC Bit ON/OFF Define
#define BIT_ON  '1'
#define BIT_OFF '0'
//#define DEFAULT '2'

//PLC COMMAND Define
#define JR  "JR"    //bit단위  read
#define JW  "JW"    //bit단위  write
#define QR  "QR"    //word단위  read
#define QW  "QW"    //word단위  write

#define	PLC_IDLE				0
#define	PLC_DOOR_OPEN_SEND		1
#define	PLC_DOOR_CLOSE_SEND		2
#define	PLC_FAULT_READ_SEND		3
#define	PLC_INIT_SEND			4

#define	PLC_DOOR_OPEN_SEND_CHECK		11
#define	PLC_DOOR_CLOSE_SEND_CHECK		12
#define	PLC_FAULT_READ_SEND_CHECK		13
#define	PLC_INIT_SEND_CHECK				14

#define	PLC_DOOR_OPEN_STATE_CHECK		21
#define	PLC_DOOR_CLOSE_STATE_CHECK		22

#define	PLC_DOOR_OPEN_STATE_CHECK_RES		31
#define	PLC_DOOR_CLOSE_STATE_CHECK_RES		32

#define PLC_ALARM_AREA_SENSOR				0x02
#define PLC_ALARM_MANUAL_DOOR_OPEN			0x01

#define PLC_ALARM_CHAMBER_RUN				0x08
#define PLC_ALARM_DOOR_CLOSE				0x04
#define PLC_ALARM_DOOR_OPEN					0x02
#define PLC_ALARM_AIR						0x01

#define PLC_ALARM_APR_S						0x08//Power Reverse Phase
#define PLC_ALARM_REF						0x04
#define PLC_ALARM_COIL_TRIP					0x02
#define PLC_ALARM_TEMP						0x01

#define PLC_ALARM_SMOKE						0x08
#define PLC_ALARM_FIRE						0x04
#define PLC_ALARM_EMG						0x02
#define PLC_ALARM_SPARE						0x01

#define PLC_CD_NONE							0
#define PLC_CD_CHAMBER_RUN					1
#define PLC_CD_DOOR_CLOSE					2
#define PLC_CD_DOOR_OPEN					3
#define PLC_CD_AIR							4
#define PLC_CD_APR_S						5
#define PLC_CD_REF							6
#define PLC_CD_COIL_TRIP					7
#define PLC_CD_TEMP							8
#define PLC_CD_SMOKE						9
#define PLC_CD_FIRE							10
#define PLC_CD_EMG							11

#define	PLC_SIG_DOOR_OPEN					0
#define	PLC_SIG_DOOR_CLOSE					1
#endif
