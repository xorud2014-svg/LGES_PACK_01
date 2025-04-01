#ifndef __COA_CLIENT_STR_H__
#define __COA_CLIENT_STR_H__

#include "SysDefine.h"
#include "P1_Client_str.h"

typedef struct s_p1_client_tag {
	unsigned char		signal[MAX_SIGNAL];

	S_P1_CONFIG			config;
	S_P1_MISC			misc;
	S_P1_TEST_CONFIG	testConfig;

	S_P1_RCV_PACKET		rcvPacket;
	S_P1_RCV_COMMAND	rcvCmd;

	S_P1_REPLY			reply;
	
	S_P1_TEST_CONDITION	testCond;

	S_P1_TEST_CONDITION_TIME	testCond_time; //kjh_160418

	//kjg_170810_s
	S_P1_RCV_CMD_COMMON_SAFETY_COND_UPDATE	common_safety_cond_update;
	S_P1_RCV_CMD_STEP_COND_UPDATE	step_cond_update; //kjg_170810_e

	S_P1_PULSE_DATA		pulse_data[MAX_CH_8];
} S_COA_CLIENT;

#endif
