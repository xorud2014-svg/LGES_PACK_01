#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <sys/time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"
#include "local_message.h"
#include "StandardInput.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_APP_CONTROL *myPs;
extern char psName[16];
char p_argv[8][32];
char *in_delimiter = " \t\n\r";

void StandardInput_Receive(void)
{
	char msg[256], *tmp;
	int	rtn, index;

	memset((char *)&p_argv[0][0], 0, (8 * 32));

	memset(msg, 0, sizeof(msg));
	tmp = fgets(msg, sizeof(msg), stdin);
	if(tmp == NULL) {
		//kjg_w userlog(DEBUG_LOG, psName, "error occurred reading from stdin\n");
		return;
	}

	rtn = Parsing_StandardInput(msg);
	if(rtn < 0) return;
	
	if(rtn >= 2) {
		index = 0;
	} else {
		userlog(DEBUG_LOG, psName, "command > ", rtn);
		if(WaitKeyInput() < 0) return;

		memset(msg, 0, sizeof(msg));
		tmp = fgets(msg, sizeof(msg), stdin);
		if(tmp == NULL) {
			//kjg_w userlog(DEBUG_LOG, psName, "error occurred reading from stdin\n");
			return;
		}
	
		rtn = Parsing_StandardInput(msg);
		if(rtn < 0) return;
		index = 0;
	}

	UserCommand_Parsing(rtn, index);
}

int WaitKeyInput(void)
{
	int 	rtn, count=0;
	struct	timeval	tv;
	fd_set	rfds;
	
	while(1) {
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		
		rtn = select(1, &rfds, NULL, NULL, &tv);
		if(rtn != 0) {
			break;
		} else {
			count++;
			if(count >= 20) {
				userlog(DEBUG_LOG, psName, "StandardInput timeover!!!\n");
				return -1; //20sec
			}
		}
	}
	return 0;
}

int Parsing_StandardInput(char *buf)
{
	char *token, tmpbuf[256];
	int	p_argc=0;

	memset((char *)&p_argv[0][0], 0, 8 * 32);
	memset((char *)&tmpbuf[0], 0, sizeof(tmpbuf));

	strcpy((char *)&tmpbuf[0], buf);

	token = strtok(tmpbuf, in_delimiter);
	if(token == NULL) {
		*buf = 0;
		userlog(DEBUG_LOG, psName, "there are nothing to get\n");
		return -1;
	}
	
	while(1) {
		strncpy((char *)&p_argv[p_argc][0], token, 32);
		userlog(DEBUG_LOG, psName, "token is %s \n", p_argv[p_argc]);
		
		p_argc++;
		token = strtok(NULL, in_delimiter);
		if(token) {
			if(p_argc >= 8) {
				userlog(DEBUG_LOG, psName, "Error!!! : p_argc over\n");
				return -2;
			}
		} else break;
	}
	return p_argc;
}

void UserCommand_Parsing(int p_argc, int index)
{
	int rtn;

	if(strcmp((char *)&p_argv[index][0], "sys") == 0
		|| strcmp((char *)&p_argv[index][0], "SYS") == 0) {
		index++;
		rtn = SysProcessing(p_argc, index);
	} else if(strcmp((char *)&p_argv[index][0], "exec") == 0
		|| strcmp((char *)&p_argv[index][0], "EXEC") == 0) {
		index++;
		rtn = ExecProcessing(p_argc, index);
	} else if(strcmp((char *)&p_argv[index][0], "cali") == 0
		|| strcmp((char *)&p_argv[index][0], "CALI") == 0) {
		index++;
		rtn = CaliProcessing(p_argc, index);
	} else if(strcmp((char *)&p_argv[index][0], "analog") == 0
		|| strcmp((char *)&p_argv[index][0], "ANALOG") == 0) {
		index++;
		rtn = AnalogProcessing(p_argc, index);
	} else if(strcmp((char *)&p_argv[index][0], "com") == 0
		|| strcmp((char *)&p_argv[index][0], "COM") == 0) {
		index++;
		rtn = ComProcessing(p_argc, index);
	} else if(strcmp((char *)&p_argv[index][0], "can") == 0
		|| strcmp((char *)&p_argv[index][0], "CAN") == 0) {
		index++;
		rtn = CanProcessing(p_argc, index);
	} else if(strcmp((char *)&p_argv[index][0], "coa") == 0
		|| strcmp((char *)&p_argv[index][0], "COA") == 0) {
		index++;
		rtn = COAProcessing(p_argc, index);
	} else if(strcmp((char *)&p_argv[index][0], "cob") == 0
		|| strcmp((char *)&p_argv[index][0], "COB") == 0) {
		index++;
		rtn = COBProcessing(p_argc, index);
	} else {
		userlog(DEBUG_LOG, psName, "unknown command : %s\n", p_argv[index]);
	}
}

int SysProcessing(int p_argc, int index)
{
	int rtn=0;
	S_MSG_VAL SendMsg;

	memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));

	if(strcmp((char *)&p_argv[index][0], "quit") == 0
		|| strcmp((char *)&p_argv[index][0], "QUIT") == 0) {
		if(myData->mData.state != M_RUN) {
			SendMsg.msg = MSG_APP_IO_EXIT;
			SendMsg.val[0] = M_CD_FAULT_NORMAL_TERMINAL_QUIT;
			send_msg(APP_TO_IO, (char *)&SendMsg);
		} else {
			userlog(DEBUG_LOG, psName, "Don't quit! Module is running.\n");
		}
	} else if(strcmp((char *)&p_argv[index][0], "f_quit") == 0
		|| strcmp((char *)&p_argv[index][0], "F_QUIT") == 0) {
		SendMsg.msg = MSG_APP_IO_EXIT;
		SendMsg.val[0] = M_CD_FAULT_FORCE_TERMINAL_QUIT;
		send_msg(APP_TO_IO, (char *)&SendMsg);
	} else if(strcmp((char *)&p_argv[index][0], "halt") == 0
		|| strcmp((char *)&p_argv[index][0], "HALT") == 0) {
		if(myData->mData.state != M_RUN) {
			SendMsg.msg = MSG_APP_IO_EXIT;
			SendMsg.val[0] = M_CD_FAULT_NORMAL_TERMINAL_HALT;
			send_msg(APP_TO_IO, (char *)&SendMsg);
		} else {
			userlog(DEBUG_LOG, psName, "Don't halt! Module is running.\n");
		}
	} else if(strcmp((char *)&p_argv[index][0], "f_halt") == 0
		|| strcmp((char *)&p_argv[index][0], "F_HALT") == 0) {
		SendMsg.msg = MSG_APP_IO_EXIT;
		SendMsg.val[0] = M_CD_FAULT_FORCE_TERMINAL_HALT;
		send_msg(APP_TO_IO, (char *)&SendMsg);
	} else {
		userlog(DEBUG_LOG, psName, "unknown item : %s\n", p_argv[index]);
		rtn = -1;
	}
	return rtn;
}

int ExecProcessing(int p_argc, int index)
{
	char buf[12];
	S_MSG_CH_FLAG ch_flag;
	S_MSG_VAL msg_val;

	memset((char *)&ch_flag, 0, sizeof(S_MSG_CH_FLAG));
	memset((char *)&msg_val, 0, sizeof(S_MSG_VAL));
	
	if(strcmp((char *)&p_argv[index][0], "test_pause") == 0) {
		msg_val.msg = MSG_APP_MODULE_TEST_PAUSE;
		msg_val.val[0] = 0; //group
		msg_val.val[1] = G_SIG_CMD_PAUSE;
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "test_continue") == 0) {
		msg_val.msg = MSG_APP_MODULE_TEST_CONTINUE;
		msg_val.val[0] = 0; //group
		msg_val.val[1] = G_SIG_CMD_CONTINUE;
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "test_stop") == 0) {
		msg_val.msg = MSG_APP_MODULE_TEST_STOP;
		msg_val.val[0] = 0; //group
		msg_val.val[1] = G_SIG_CMD_STOP;
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "test_bcr") == 0) {
		msg_val.msg = MSG_APP_MODULE_TEST_BCR_READ;
		msg_val.val[0] = 0; //group
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "ch_io_set") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			ch_flag.flag[0] = 0x01;
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			ch_flag.flag[0] = 0x02;
		} else {
			userlog(DEBUG_LOG, psName, "fail_ch_no %s\n", p_argv[index]);
			return 0;
		}

		msg_val.msg = MSG_APP_MODULE_CH_IO_SET;
		msg_val.val[0] = 0; //group

		index++;
		memset(buf, 0, sizeof buf);
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[1] = atoi(buf);

		index++;
		memset(buf, 0, sizeof buf);
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[2] = atoi(buf);
		send_msg_ch_flag(APP_TO_MODULE, (char *)&ch_flag);
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else {
		userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -10;
	}

	return 0;
}

int CaliProcessing(int p_argc, int index)
{
	char buf[12];
	int rtn, ch;
	S_MSG_VAL msg_val;

	memset((char *)&msg_val, 0, sizeof(S_MSG_VAL));
	memset(buf, 0, sizeof buf);

	if(strcmp((char *)&p_argv[index][0], "connect") == 0
		|| strcmp((char *)&p_argv[index][0], "CONNECT") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "v") == 0
			|| strcmp((char *)&p_argv[index][0], "V") == 0) {
			msg_val.msg = MSG_APP_METER_INITIALIZE;
			msg_val.val[0] = CALI_TYPE_VOLTAGE;
			send_msg(APP_TO_METER, (char *)&msg_val);
		} else if(strcmp((char *)&p_argv[index][0], "i") == 0
			|| strcmp((char *)&p_argv[index][0], "I") == 0) {
			msg_val.msg = MSG_APP_METER_INITIALIZE;
			msg_val.val[0] = CALI_TYPE_CURRENT;
			send_msg(APP_TO_METER, (char *)&msg_val);
		}
	} else if(strcmp((char *)&p_argv[index][0], "start") == 0
		|| strcmp((char *)&p_argv[index][0], "START") == 0) {
		index++;
		ch = 0;
		if(strcmp((char *)&p_argv[index][0], "v") == 0
			|| strcmp((char *)&p_argv[index][0], "V") == 0) {
			myData->cali[ch].type = CALI_TYPE_VOLTAGE;
			myData->cali[ch].range = 0;
			myData->cali[ch].mode = CALI_MODE_NORMAL;
			msg_val.msg = MSG_APP_MODULE_CH_CALI;
			msg_val.val[0] = 0;
			send_msg(APP_TO_MODULE, (char *)&msg_val);
		} else if(strcmp((char *)&p_argv[index][0], "i") == 0
			|| strcmp((char *)&p_argv[index][0], "I") == 0) {
			myData->cali[ch].type = CALI_TYPE_CURRENT;
			myData->cali[ch].range = 0;
			myData->cali[ch].mode = CALI_MODE_NORMAL;
			msg_val.msg = MSG_APP_MODULE_CH_CALI;
			msg_val.val[0] = 0;
			send_msg(APP_TO_MODULE, (char *)&msg_val);
		}
	} else if(strcmp((char *)&p_argv[index][0], "read") == 0
		|| strcmp((char *)&p_argv[index][0], "READ") == 0) {
		msg_val.msg = MSG_APP_METER_REQUEST;
		send_msg(APP_TO_METER, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "org") == 0
		|| strcmp((char *)&p_argv[index][0], "ORG") == 0) {
		rtn = Create_BdCaliData_Org(0);
	} else if(strcmp((char *)&p_argv[index][0], "test") == 0
		|| strcmp((char *)&p_argv[index][0], "TEST") == 0) {
		msg_val.msg = MSG_APP_METER_TEST;
		index++;
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] = atoi(buf);
		send_msg(APP_TO_METER, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "update") == 0
		|| strcmp((char *)&p_argv[index][0], "UPDATE") == 0) {
		myPs->signal[APP_SIG_CALI_UPDATE] = P0;
	} else if(strcmp((char *)&p_argv[index][0], "dav") == 0
		|| strcmp((char *)&p_argv[index][0], "DAV") == 0) {
		msg_val.msg = MSG_APP_METER_INITIALIZE;
		msg_val.val[0] = CALI_TYPE_VOLTAGE;
		send_msg(APP_TO_METER, (char *)&msg_val);

		memset((char *)&msg_val, 0, sizeof(S_MSG_VAL));
		index++;
		//bd 1base -> 1 single, 2 parallel
		msg_val.val[1] = atoi((char *)&p_argv[index][0]);
		index++;
		msg_val.val[0] = atoi((char *)&p_argv[index][0]); //group 1base
		msg_val.msg = MSG_APP_MODULE_CALI_DAV;
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "dai") == 0
		|| strcmp((char *)&p_argv[index][0], "DAI") == 0) {
		msg_val.msg = MSG_APP_METER_INITIALIZE;
		msg_val.val[0] = CALI_TYPE_CURRENT;
		send_msg(APP_TO_METER, (char *)&msg_val);

		memset((char *)&msg_val, 0, sizeof(S_MSG_VAL));
		index++;
		//bd 1base -> 1 single, 2 parallel
		msg_val.val[1] = atoi((char *)&p_argv[index][0]);
		index++;
		msg_val.val[0] = atoi((char *)&p_argv[index][0]); //group 1base
		msg_val.msg = MSG_APP_MODULE_CALI_DAI;
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "dai_s") == 0
		|| strcmp((char *)&p_argv[index][0], "DAI_S") == 0) {
		msg_val.msg = MSG_APP_METER_INITIALIZE;
		msg_val.val[0] = CALI_TYPE_CURRENT;
		send_msg(APP_TO_METER, (char *)&msg_val);

		memset((char *)&msg_val, 0, sizeof(S_MSG_VAL));
		index++;
		//bd 1base -> 1 single, 2 parallel
		msg_val.val[0] = atoi((char *)&p_argv[index][0]);
		index++;
		msg_val.val[1] = atoi((char *)&p_argv[index][0]); //range
		msg_val.msg = MSG_APP_MODULE_CALI_DAI_SW;
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "range") == 0
		|| strcmp((char *)&p_argv[index][0], "RANGE") == 0) {
		index++;
		msg_val.val[0] = atoi((char *)&p_argv[index][0]); //type
		index++;
		msg_val.val[1] = atoi((char *)&p_argv[index][0]); //range
		msg_val.msg = MSG_APP_MODULE_CALI_RANGE;
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "da_update") == 0
		|| strcmp((char *)&p_argv[index][0], "DA_UPDATE") == 0) {
		index++;
		myPs->signal[APP_SIG_CALI_DA_UPDATE]
			= (unsigned char)atoi((char *)&p_argv[index][0]); //group 1base
		if(myPs->signal[APP_SIG_CALI_DA_UPDATE] == P0) { //all group
			myPs->signal[APP_SIG_CALI_DA_UPDATE] = P50;
		}
	} else {
		userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -10;
	}

	return 0;
}

int AnalogProcessing(int p_argc, int index)
{
	S_MSG_VAL msg_val;

	memset((char *)&msg_val, 0, sizeof(S_MSG_VAL));

	if(strcmp((char *)&p_argv[index][0], "connect") == 0
		|| strcmp((char *)&p_argv[index][0], "CONNECT") == 0) {
		index++;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			index++;
			msg_val.msg = MSG_APP_METER2_INITIALIZE;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				msg_val.val[0] = 1;
				msg_val.val[1] = 1;
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				msg_val.val[0] = 1;
				msg_val.val[1] = 2;
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				msg_val.val[0] = 1;
				msg_val.val[1] = 3;
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				msg_val.val[0] = 1;
				msg_val.val[1] = 4;
			//csk_161207_s
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				msg_val.val[0] = 1;
				msg_val.val[1] = 5;
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				msg_val.val[0] = 1;
				msg_val.val[1] = 6;
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				msg_val.val[0] = 1;
				msg_val.val[1] = 7;
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				msg_val.val[0] = 1;
				msg_val.val[1] = 8;
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				msg_val.val[0] = 1;
				msg_val.val[1] = 9;
			} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 
				|| (strcmp((char *)&p_argv[index][0], "a") == 0)) {
				msg_val.val[0] = 1;
				msg_val.val[1] = 10;
			//csk_161207_e
			}
			send_msg(APP_TO_METER2, (char *)&msg_val);
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			index++;
			msg_val.msg = MSG_APP_METER2_INITIALIZE;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				msg_val.val[0] = 2;
				msg_val.val[1] = 1;
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				msg_val.val[0] = 2;
				msg_val.val[1] = 2;
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				msg_val.val[0] = 2;
				msg_val.val[1] = 3;
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				msg_val.val[0] = 2;
				msg_val.val[1] = 4;
			//csk_161207_s
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				msg_val.val[0] = 2;
				msg_val.val[1] = 5;
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				msg_val.val[0] = 2;
				msg_val.val[1] = 6;
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				msg_val.val[0] = 2;
				msg_val.val[1] = 7;
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				msg_val.val[0] = 2;
				msg_val.val[1] = 8;
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				msg_val.val[0] = 2;
				msg_val.val[1] = 9;
			} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 
				|| (strcmp((char *)&p_argv[index][0], "a") == 0)) {
				msg_val.val[0] = 2;
				msg_val.val[1] = 10;
			//csk_161207_e
			}
			send_msg(APP_TO_METER2, (char *)&msg_val);
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			index++;
			msg_val.msg = MSG_APP_METER2_INITIALIZE;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				msg_val.val[0] = 3;
				msg_val.val[1] = 1;
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				msg_val.val[0] = 3;
				msg_val.val[1] = 2;
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				msg_val.val[0] = 3;
				msg_val.val[1] = 3;
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				msg_val.val[0] = 3;
				msg_val.val[1] = 4;
			//csk_161207_s
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				msg_val.val[0] = 3;
				msg_val.val[1] = 5;
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				msg_val.val[0] = 3;
				msg_val.val[1] = 6;
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				msg_val.val[0] = 3;
				msg_val.val[1] = 7;
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				msg_val.val[0] = 3;
				msg_val.val[1] = 8;
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				msg_val.val[0] = 3;
				msg_val.val[1] = 9;
			} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 
				|| (strcmp((char *)&p_argv[index][0], "a") == 0)) {
				msg_val.val[0] = 3;
				msg_val.val[1] = 10;
			//csk_161207_e
			}
			send_msg(APP_TO_METER2, (char *)&msg_val);
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			index++;
			msg_val.msg = MSG_APP_METER2_INITIALIZE;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				msg_val.val[0] = 4;
				msg_val.val[1] = 1;
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				msg_val.val[0] = 4;
				msg_val.val[1] = 2;
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				msg_val.val[0] = 4;
				msg_val.val[1] = 3;
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				msg_val.val[0] = 4;
				msg_val.val[1] = 4;
			//csk_161207_s
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				msg_val.val[0] = 4;
				msg_val.val[1] = 5;
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				msg_val.val[0] = 4;
				msg_val.val[1] = 6;
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				msg_val.val[0] = 4;
				msg_val.val[1] = 7;
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				msg_val.val[0] = 4;
				msg_val.val[1] = 8;
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				msg_val.val[0] = 4;
				msg_val.val[1] = 9;
			} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 
				|| (strcmp((char *)&p_argv[index][0], "a") == 0)) {
				msg_val.val[0] = 4;
				msg_val.val[1] = 10;
			}
			send_msg(APP_TO_METER2, (char *)&msg_val);
		} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
			index++;
			msg_val.msg = MSG_APP_METER2_INITIALIZE;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				msg_val.val[0] = 5;
				msg_val.val[1] = 1;
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				msg_val.val[0] = 5;
				msg_val.val[1] = 2;
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				msg_val.val[0] = 5;
				msg_val.val[1] = 3;
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				msg_val.val[0] = 5;
				msg_val.val[1] = 4;
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				msg_val.val[0] = 5;
				msg_val.val[1] = 5;
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				msg_val.val[0] = 5;
				msg_val.val[1] = 6;
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				msg_val.val[0] = 5;
				msg_val.val[1] = 7;
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				msg_val.val[0] = 5;
				msg_val.val[1] = 8;
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				msg_val.val[0] = 5;
				msg_val.val[1] = 9;
			} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 
				|| (strcmp((char *)&p_argv[index][0], "a") == 0)) {
				msg_val.val[0] = 5;
				msg_val.val[1] = 10;
			}
			send_msg(APP_TO_METER2, (char *)&msg_val);
		} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
			index++;
			msg_val.msg = MSG_APP_METER2_INITIALIZE;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				msg_val.val[0] = 6;
				msg_val.val[1] = 1;
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				msg_val.val[0] = 6;
				msg_val.val[1] = 2;
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				msg_val.val[0] = 6;
				msg_val.val[1] = 3;
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				msg_val.val[0] = 6;
				msg_val.val[1] = 4;
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				msg_val.val[0] = 6;
				msg_val.val[1] = 5;
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				msg_val.val[0] = 6;
				msg_val.val[1] = 6;
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				msg_val.val[0] = 6;
				msg_val.val[1] = 7;
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				msg_val.val[0] = 6;
				msg_val.val[1] = 8;
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				msg_val.val[0] = 6;
				msg_val.val[1] = 9;
			} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 
				|| (strcmp((char *)&p_argv[index][0], "a") == 0)) {
				msg_val.val[0] = 6;
				msg_val.val[1] = 10;
			}
			send_msg(APP_TO_METER2, (char *)&msg_val);
		} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
			index++;
			msg_val.msg = MSG_APP_METER2_INITIALIZE;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				msg_val.val[0] = 7;
				msg_val.val[1] = 1;
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				msg_val.val[0] = 7;
				msg_val.val[1] = 2;
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				msg_val.val[0] = 7;
				msg_val.val[1] = 3;
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				msg_val.val[0] = 7;
				msg_val.val[1] = 4;
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				msg_val.val[0] = 7;
				msg_val.val[1] = 5;
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				msg_val.val[0] = 7;
				msg_val.val[1] = 6;
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				msg_val.val[0] = 7;
				msg_val.val[1] = 7;
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				msg_val.val[0] = 7;
				msg_val.val[1] = 8;
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				msg_val.val[0] = 7;
				msg_val.val[1] = 9;
			} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 
				|| (strcmp((char *)&p_argv[index][0], "a") == 0)) {
				msg_val.val[0] = 7;
				msg_val.val[1] = 10;
			}
			send_msg(APP_TO_METER2, (char *)&msg_val);
		} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
			index++;
			msg_val.msg = MSG_APP_METER2_INITIALIZE;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				msg_val.val[0] = 8;
				msg_val.val[1] = 1;
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				msg_val.val[0] = 8;
				msg_val.val[1] = 2;
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				msg_val.val[0] = 8;
				msg_val.val[1] = 3;
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				msg_val.val[0] = 8;
				msg_val.val[1] = 4;
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				msg_val.val[0] = 8;
				msg_val.val[1] = 5;
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				msg_val.val[0] = 8;
				msg_val.val[1] = 6;
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				msg_val.val[0] = 8;
				msg_val.val[1] = 7;
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				msg_val.val[0] = 8;
				msg_val.val[1] = 8;
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				msg_val.val[0] = 8;
				msg_val.val[1] = 9;
			} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 
				|| (strcmp((char *)&p_argv[index][0], "a") == 0)) {
				msg_val.val[0] = 8;
				msg_val.val[1] = 10;
			}
			send_msg(APP_TO_METER2, (char *)&msg_val);
		} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
			index++;
			msg_val.msg = MSG_APP_METER2_INITIALIZE;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				msg_val.val[0] = 9;
				msg_val.val[1] = 1;
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				msg_val.val[0] = 9;
				msg_val.val[1] = 2;
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				msg_val.val[0] = 9;
				msg_val.val[1] = 3;
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				msg_val.val[0] = 9;
				msg_val.val[1] = 4;
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				msg_val.val[0] = 9;
				msg_val.val[1] = 5;
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				msg_val.val[0] = 9;
				msg_val.val[1] = 6;
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				msg_val.val[0] = 9;
				msg_val.val[1] = 7;
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				msg_val.val[0] = 9;
				msg_val.val[1] = 8;
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				msg_val.val[0] = 9;
				msg_val.val[1] = 9;
			} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 
				|| (strcmp((char *)&p_argv[index][0], "a") == 0)) {
				msg_val.val[0] = 9;
				msg_val.val[1] = 10;
			}
			send_msg(APP_TO_METER2, (char *)&msg_val);
		} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 
			|| (strcmp((char *)&p_argv[index][0], "a") == 0)) {
			index++;
			msg_val.msg = MSG_APP_METER2_INITIALIZE;
			if(strcmp((char *)&p_argv[index][0], "1") == 0) {
				msg_val.val[0] = 10;
				msg_val.val[1] = 1;
			} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
				msg_val.val[0] = 10;
				msg_val.val[1] = 2;
			} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
				msg_val.val[0] = 10;
				msg_val.val[1] = 3;
			} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
				msg_val.val[0] = 10;
				msg_val.val[1] = 4;
			} else if(strcmp((char *)&p_argv[index][0], "5") == 0) {
				msg_val.val[0] = 10;
				msg_val.val[1] = 5;
			} else if(strcmp((char *)&p_argv[index][0], "6") == 0) {
				msg_val.val[0] = 10;
				msg_val.val[1] = 6;
			} else if(strcmp((char *)&p_argv[index][0], "7") == 0) {
				msg_val.val[0] = 10;
				msg_val.val[1] = 7;
			} else if(strcmp((char *)&p_argv[index][0], "8") == 0) {
				msg_val.val[0] = 10;
				msg_val.val[1] = 8;
			} else if(strcmp((char *)&p_argv[index][0], "9") == 0) {
				msg_val.val[0] = 10;
				msg_val.val[1] = 9;
			} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 
				|| (strcmp((char *)&p_argv[index][0], "a") == 0)) {
				msg_val.val[0] = 10;
				msg_val.val[1] = 10;
			}
			send_msg(APP_TO_METER2, (char *)&msg_val);
			//csk_161207_e
		} else {
			userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
			return -10;
		}
	} else if(strcmp((char *)&p_argv[index][0], "read") == 0
		|| strcmp((char *)&p_argv[index][0], "READ") == 0) {
		index++;
		msg_val.msg = MSG_APP_METER2_REQUEST;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			msg_val.val[0] = 0;
			msg_val.val[1] = 1;
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			msg_val.val[0] = 0;
			msg_val.val[1] = 2;
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			msg_val.val[0] = 0;
			msg_val.val[1] = 3;
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			msg_val.val[0] = 0;
			msg_val.val[1] = 4;
		} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 		//csk_161207_s
				|| (strcmp((char *)&p_argv[index][0], "a")) == 0) {
			msg_val.val[0] = 0;
			msg_val.val[1] = 10;										//csk_161207_e
		} else {
			msg_val.val[0] = 0;
			msg_val.val[1] = atoi(p_argv[index]);
		}
		send_msg(APP_TO_METER2, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "read2") == 0
		|| strcmp((char *)&p_argv[index][0], "READ2") == 0) {
		index++;
		msg_val.msg = MSG_APP_METER2_REQUEST;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			msg_val.val[0] = 1;
			msg_val.val[1] = 1;
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			msg_val.val[0] = 1;
			msg_val.val[1] = 2;
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			msg_val.val[0] = 1;
			msg_val.val[1] = 3;
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			msg_val.val[0] = 1;
			msg_val.val[1] = 4;
		} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 		//csk_161207_s
				|| (strcmp((char *)&p_argv[index][0], "a")) == 0) {
			msg_val.val[0] = 1;
			msg_val.val[1] = 10;										//csk_161207_e		
		} else {
			msg_val.val[0] = 1;
			msg_val.val[1] = atoi(p_argv[index]);
		}
		send_msg(APP_TO_METER2, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "start") == 0
		|| strcmp((char *)&p_argv[index][0], "START") == 0) {
		index++;
		msg_val.msg = MSG_APP_METER2_MEASURE;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			msg_val.val[0] = 0;
			msg_val.val[1] = 1;
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			msg_val.val[0] = 0;
			msg_val.val[1] = 2;
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			msg_val.val[0] = 0;
			msg_val.val[1] = 3;
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			msg_val.val[0] = 0;
			msg_val.val[1] = 4;
		} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 		//csk_161207_s
				|| (strcmp((char *)&p_argv[index][0], "a")) == 0) {
			msg_val.val[0] = 0;
			msg_val.val[1] = 10;										//csk_161207_e
		} else {
			msg_val.val[0] = 0;
			msg_val.val[1] = atoi(p_argv[index]);
		}
		send_msg(APP_TO_METER2, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "stop") == 0
		|| strcmp((char *)&p_argv[index][0], "STOP") == 0) {
		index++;
		msg_val.msg = MSG_APP_METER2_MEASURE;
		if(strcmp((char *)&p_argv[index][0], "1") == 0) {
			msg_val.val[0] = 1;
			msg_val.val[1] = 1;
		} else if(strcmp((char *)&p_argv[index][0], "2") == 0) {
			msg_val.val[0] = 1;
			msg_val.val[1] = 2;
		} else if(strcmp((char *)&p_argv[index][0], "3") == 0) {
			msg_val.val[0] = 1;
			msg_val.val[1] = 3;
		} else if(strcmp((char *)&p_argv[index][0], "4") == 0) {
			msg_val.val[0] = 1;
			msg_val.val[1] = 4;
		} else if((strcmp((char *)&p_argv[index][0], "A") == 0) 		//csk_161207_s
				|| (strcmp((char *)&p_argv[index][0], "a")) == 0) {
			msg_val.val[0] = 1;
			msg_val.val[1] = 10;										//csk_161207_e
		} else {
			msg_val.val[0] = 1;
			msg_val.val[1] = atoi(p_argv[index]);
		}
		send_msg(APP_TO_METER2, (char *)&msg_val);
	} else {
		userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -11;
	}

	return 0;
}

int ComProcessing(int p_argc, int index)
{
	char buf[12];
	S_MSG_VAL msg_val;

	memset((char *)&msg_val, 0, sizeof(S_MSG_VAL));
	memset(buf, 0, sizeof buf);

	if(strcmp((char *)&p_argv[index][0], "rs232_baudrate") == 0
		|| strcmp((char *)&p_argv[index][0], "RS232_BAUDRATE") == 0) {
		index++;
		msg_val.msg = MSG_APP_MODULE_RS232_BAUDRATE_SET;
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] = atoi(buf);

		index++;
		memset(buf, 0, sizeof buf);
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[1] = atoi(buf);
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "bcr") == 0
		|| strcmp((char *)&p_argv[index][0], "BCR") == 0) {
		index++;
		msg_val.msg = MSG_APP_MODULE_READ_BCR;
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] = atoi(buf); //1base groupNo
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else {
	    userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -11;
    }

	return 0;
}

int CanProcessing(int p_argc, int index)
{
	char buf[12], tmp[12];
	int rtn;
	S_MSG_VAL msg_val;

	memset((char *)&msg_val, 0, sizeof(S_MSG_VAL));
	memset(buf, 0, sizeof buf);

	if(strcmp((char *)&p_argv[index][0], "init") == 0
		|| strcmp((char *)&p_argv[index][0], "INIT") == 0) {
		index++;
		msg_val.msg = MSG_APP_MODULE_CAN_INITIALIZE;
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] = atoi(buf);
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "status") == 0
		|| strcmp((char *)&p_argv[index][0], "STATUS") == 0) {
		index++;
		msg_val.msg = MSG_APP_MODULE_CAN_STATUS_REQUEST;
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] = atoi(buf);
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "can_baudrate") == 0
		|| strcmp((char *)&p_argv[index][0], "CAN_BAUDRATE") == 0) {
		index++;
		msg_val.msg = MSG_APP_MODULE_CAN_BAUDRATE_SET;
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] = atoi(buf); //can_ch

		index++;
		memset(buf, 0, sizeof buf);
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[1] = atoi(buf); //baud

		msg_val.val[2] = 0; //ext
		msg_val.val[3] = 0; //sjw
		msg_val.val[4] = 0; //fd_flag
		msg_val.val[5] = 2; //data_rate
		msg_val.val[6] = 0; //crc_type
		msg_val.val[7] = 1; //terminal_r
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "can_error_clear") == 0
		|| strcmp((char *)&p_argv[index][0], "CAN_ERROR_CLEAR") == 0) {
		index++;
		msg_val.msg = MSG_APP_MODULE_CAN_ERROR_CLEAR;
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] = atoi(buf);
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "test_start") == 0
		|| strcmp((char *)&p_argv[index][0], "TEST_START") == 0) {
		msg_val.msg = MSG_APP_MODULE_CAN_TEST1;
		msg_val.val[0] = 1;
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "test_stop") == 0
		|| strcmp((char *)&p_argv[index][0], "TEST_STOP") == 0) {
		msg_val.msg = MSG_APP_MODULE_CAN_TEST1;
		msg_val.val[0] = 0;
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "test2") == 0
		|| strcmp((char *)&p_argv[index][0], "TEST2") == 0) {
		index++;
		memset(tmp, 0, sizeof tmp);
		strcpy(tmp, (char *)&p_argv[index][0]);

		index++;
		memset(buf, 0, sizeof buf);
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] = atoi(buf); //ch (0 base)

		if(strcmp((char *)&tmp[0], "run") == 0
			|| strcmp((char *)&tmp[0], "RUN") == 0) {
			msg_val.val[1] = CAN_TX_FUNC_USER_RUN;
		} else if(strcmp((char *)&tmp[0], "stop") == 0
			|| strcmp((char *)&tmp[0], "STOP") == 0) {
			msg_val.val[1] = CAN_TX_FUNC_USER_STOP;
		} else if(strcmp((char *)&tmp[0], "pause") == 0
			|| strcmp((char *)&tmp[0], "PAUSE") == 0) {
			msg_val.val[1] = CAN_TX_FUNC_USER_PAUSE;
		} else if(strcmp((char *)&tmp[0], "continue") == 0
			|| strcmp((char *)&tmp[0], "CONTINUE") == 0) {
			msg_val.val[1] = CAN_TX_FUNC_USER_CONTINUE;
		} else if(strcmp((char *)&tmp[0], "next") == 0
			|| strcmp((char *)&tmp[0], "NEXT") == 0) {
			msg_val.val[1] = CAN_TX_FUNC_USER_NEXT;
		} else if(strcmp((char *)&tmp[0], "goto") == 0
			|| strcmp((char *)&tmp[0], "GOTO") == 0) {
			msg_val.val[1] = CAN_TX_FUNC_USER_GOTO;
			msg_val.val[2] = 4; //step_no
		} else if(strcmp((char *)&tmp[0], "value") == 0
			|| strcmp((char *)&tmp[0], "VALUE") == 0) {
			msg_val.val[1] = CAN_TX_FUNC_USER_VALUE1;
			index++;
			memset(buf, 0, sizeof buf);
			strcpy(buf, (char *)&p_argv[index][0]);
			msg_val.val[2] = atoi(buf); //value
		} else {
			index = -1;
		}

		if(index < 0) {
			userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		} else {
			msg_val.msg = MSG_APP_MODULE_CAN_TEST2;
			send_msg(APP_TO_MODULE, (char *)&msg_val);

			usleep(10000);
			msg_val.val[1] = 0;
			msg_val.val[2] = 0;
			send_msg(APP_TO_MODULE, (char *)&msg_val);
		}
	} else if(strcmp((char *)&p_argv[index][0], "test3") == 0
		|| strcmp((char *)&p_argv[index][0], "TEST3") == 0) {
		index++;
		memset(buf, 0, sizeof buf);
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] = atoi(buf); //ch
		msg_val.msg = MSG_APP_MODULE_CAN_TEST3;
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "fd_test") == 0
		|| strcmp((char *)&p_argv[index][0], "FD_TEST") == 0) { //kjg_180405
		index++;
		memset(buf, 0, sizeof buf);
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] = atoi(buf); //ch
		msg_val.msg = MSG_APP_MODULE_CAN_FD_TEST;
		send_msg(APP_TO_MODULE, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "file_write") == 0
		|| strcmp((char *)&p_argv[index][0], "FILE_WRITE") == 0) {
		rtn = Write_CanReceiveSetData();
		rtn = Write_CanTransmitSetData();
	} else {
		userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -11;
	}

	return 0;
}

int COAProcessing(int p_argc, int index)
{
	char buf[12];
	S_MSG_VAL msg_val;

	memset((char *)&msg_val, 0, sizeof(S_MSG_VAL));
	memset(buf, 0, sizeof buf);

	if(strcmp((char *)&p_argv[index][0], "real_time") == 0
		|| strcmp((char *)&p_argv[index][0], "REAL_TIME") == 0) {
		msg_val.msg = MSG_APP_COA_REAL_TIME_REQUEST;
		send_msg(APP_TO_COA1, (char *)&msg_val);
	} else {
		userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -11;
	}

	return 0;
}

int COBProcessing(int p_argc, int index)
{
	int dir;
	char buf[12];
	S_MSG_VAL msg_val;

	memset((char *)&msg_val, 0, sizeof(S_MSG_VAL));
	memset(buf, 0, sizeof buf);

	if(strcmp((char *)&p_argv[index][0], "start") == 0
		|| strcmp((char *)&p_argv[index][0], "START") == 0) {
		index++;
		msg_val.msg = MSG_APP_COB_TEST_FUNCTION;
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] = 1;
		send_msg(APP_TO_COB1 + atoi(buf) - 1, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "stop") == 0
		|| strcmp((char *)&p_argv[index][0], "STOP") == 0) {
		index++;
		msg_val.msg = MSG_APP_COB_TEST_FUNCTION;
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] = 2;
		send_msg(APP_TO_COB1 + atoi(buf) - 1, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "door_o") == 0
		|| strcmp((char *)&p_argv[index][0], "DOOR_O") == 0) {
		index++;
		msg_val.msg = MSG_APP_COB_PLC_DOOR_OPEN;
		strcpy(buf, (char *)&p_argv[index][0]);
		send_msg(APP_TO_COB1 + atoi(buf) - 1, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "door_c") == 0
		|| strcmp((char *)&p_argv[index][0], "DOOR_C") == 0) {
		index++;
		msg_val.msg = MSG_APP_COB_PLC_DOOR_CLOSE;
		strcpy(buf, (char *)&p_argv[index][0]);
		send_msg(APP_TO_COB1 + atoi(buf) - 1, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "fault_r") == 0
		|| strcmp((char *)&p_argv[index][0], "FAULT_R") == 0) {
		index++;
		msg_val.msg = MSG_APP_COB_PLC_FAULT_READ;
		strcpy(buf, (char *)&p_argv[index][0]);
		send_msg(APP_TO_COB1 + atoi(buf) - 1, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "job") == 0
		|| strcmp((char *)&p_argv[index][0], "JOB") == 0) {
		index++;
		msg_val.msg = MSG_APP_COB_JOB_CHANGE;
		strcpy(buf, (char *)&p_argv[index][0]);
		dir = atoi(buf);
		index++;
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] =  atoi(buf);
		send_msg(APP_TO_COB1 + dir - 1, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "pitch") == 0
		|| strcmp((char *)&p_argv[index][0], "PITCH") == 0) {
		index++;
		msg_val.msg = MSG_APP_COB_PITCH_CHANGE;
		strcpy(buf, (char *)&p_argv[index][0]);
		dir = atoi(buf);
		index++;
		strcpy(buf, (char *)&p_argv[index][0]);
		msg_val.val[0] =  atoi(buf);
		send_msg(APP_TO_COB1 + dir - 1, (char *)&msg_val);
	} else if(strcmp((char *)&p_argv[index][0], "jig_c") == 0
		|| strcmp((char *)&p_argv[index][0], "JIG_C") == 0) {
		index++;
		msg_val.msg = MSG_APP_COB_TRY_CONTACT;
		strcpy(buf, (char *)&p_argv[index][0]);
		send_msg(APP_TO_COB1 + atoi(buf) - 1, (char *)&msg_val);
		userlog(DEBUG_LOG, psName, "MSG_APP_COB%d_TRY_CONTACT\n", atoi(buf) - 1);		//csk_140220
	} else if(strcmp((char *)&p_argv[index][0], "jig_o") == 0
		|| strcmp((char *)&p_argv[index][0], "JIG_O") == 0) {
		index++;
		msg_val.msg = MSG_APP_COB_TRY_DISCONTACT;
		strcpy(buf, (char *)&p_argv[index][0]);
		send_msg(APP_TO_COB1 + atoi(buf) - 1, (char *)&msg_val);
		userlog(DEBUG_LOG, psName, "MSG_APP_COB%d_TRY_DISCONTACT\n", atoi(buf) - 1);		//csk_140220
	} else {
		userlog(DEBUG_LOG, psName, "unknown item :%s\n", p_argv[index]);
		return -11;
	}

	return 0;
}
