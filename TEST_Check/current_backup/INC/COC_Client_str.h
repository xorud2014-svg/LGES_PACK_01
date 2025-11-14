#ifndef __COC_CLIENT_STR_H__
#define __COC_CLIENT_STR_H__

#include "SysDefine.h"
#include "P3_Client_str.h"

typedef struct s_p3_client_tag {
	unsigned char		signal[MAX_SIGNAL];

	S_P3_CONFIG			config;
	S_P3_MISC			misc;
	S_P3_TEST_CONFIG	testConfig;

	S_P3_RCV_PACKET		rcvPacket;
	S_P3_RCV_COMMAND	rcvCmd;

	S_P3_REPLY			reply;
	
	S_P3_TEST_CONDITION	testCond;

//	S_P3_CH_PULSE_DATA	chPulseData[MAX_CH_PER_MODULE];
//	S_P3_CH_PULSE_DATA	fadmPulseData[MAX_CH_PER_MODULE];
} S_COC_CLIENT;

#endif
