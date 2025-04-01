#ifndef __COB_CLIENT_STR_H__
#define __COB_CLIENT_STR_H__

#include "SysDefine.h"
#include "P2_Client_str.h"

typedef struct s_p2_client_tag {
	unsigned char		signal[MAX_SIGNAL];

	S_P2_CONFIG			config;
	S_P2_CONFIG_DATA	configData;
	S_P2_MISC			misc;
	S_P2_TEST_CONFIG	testConfig;

	S_P2_RCV_PACKET		rcvPacket;
	S_P2_RCV_COMMAND	rcvCmd;
	S_P2_CMD_BCR_INFO	bcr;

	S_P2_REPLY			reply;
	
	S_P2_SENSOR_LIMIT	sensor_limit;
	S_P2_CHAMBER_SET	chamber_set;

	S_P2_TEST_CONDITION	testCond;
} S_COB_CLIENT;

#endif
