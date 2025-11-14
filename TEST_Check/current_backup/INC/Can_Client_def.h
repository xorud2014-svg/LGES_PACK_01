#ifndef __CAN_CLIENT_DEF_H__
#define __CAN_CLIENT_DEF_H__

#define	PROTOCOL_VERSION_1					20190905

#define MAX_LTC_TYPE						2
//#define MAX_LTC_DATA						256
#define MAX_RX_LTC_DATA						100
#define MAX_TX_LTC_DATA						32
#define MAX_TC_DATA							20
//#define MAX_TC_DATA							15
#define MAX_NTC_DATA						20
#define MAX_AUX_V_DATA						50
#define MAX_TX_FUNC_DATA					10	//shh_test

// CanClient Signal
#define CAN_SIG_NET_CONNECTED				0
#define CAN_SIG_NET_CONNECT_RETRY			1
#define CAN_SIG_TEST_STEP_RCV				2
#define CAN_SIG_CLIENT_CONNECT				3
#define CAN_SIG_DATA_START					4

// General Cmd
// SBC -> CAN
#define SBC_TO_CAN_CMD_RECEIVE_SET			0x00000001
#define SBC_TO_CAN_CMD_TRANSMIT_SET			0x00000002
#define SBC_TO_CAN_CMD_HEARTBEAT_RPY		0x00000003
#define SBC_TO_CAN_CMD_RESPONSE				0x00000004
#define SBC_TO_CAN_CMD_PACK_DATA			0x00000005

//CAN -> SBC
#define	CAN_TO_SBC_CMD_RECEIVE_DATA			0x00010001
#define CAN_TO_SBC_CMD_STANDBY_REQ			0x00010002
#define CAN_TO_SBC_CMD_HEARTBEAT_REQ		0x00010003
#define CAN_TO_SBC_CMD_RESPONSE				0x00010004
#define CAN_TO_SBC_CMD_RECEIVE_SET_REQ		0x00010005
#define	CAN_TO_SBC_CMD_TRANSMIT_SET_REQ		0x00010006

//response code
#define CAN_CLIENT_CD_ACK					0x00000001
#define CAN_CLIENT_CD_NACK					0x00000002

#endif
