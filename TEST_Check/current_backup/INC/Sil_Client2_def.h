#ifndef __SIL_CLIENT2_DEF_H__
#define __SIL_CLIENT2_DEF_H__

#define	PROTOCOL_VERSION_1					20190905

//#define MAX_SENSOR_TYPE						2
//#define MAX_SENSOR_CHANNEL					256

// SilClient Signal
#define SIL_SIG_NET_CONNECTED				0
#define SIL_SIG_NET_CONNECT_RETRY			1
#define SIL_SIG_TEST_STEP_RCV				2
#define SIL_SIG_CLIENT_CONNECT				3

// General Cmd
// SBC -> SIL2
#define SBC_TO_SIL2_CMD_RESPONSE			0x00000001
#define SBC_TO_SIL2_CMD_START				0x00000035
#define SBC_TO_SIL2_CMD_STOP				0x00000045

//SIL2 -> SBC
#define	SIL2_TO_SBC_CMD_HEARTBEAT_REQ		0x00000015
#define SIL2_TO_SBC_CMD_RESPONSE			0x00000001
#define	SIL2_TO_SBC_CMD_DAQ_DATA_RPY		0x00000025

//response code
#define SIL_CLIENT_CD_ACK					0x00000001
#define SIL_CLIENT_CD_NACK					0x00000002

// ethernet protocol
#define SIL_CLIENT_STX			0x0002
#define SIL_CLIENT_ETX			0x0003

#endif
