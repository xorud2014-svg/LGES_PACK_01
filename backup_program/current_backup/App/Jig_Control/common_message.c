#include "../../INC/datastore.h"
#include "local_utils.h"
#include "common_message.h"

extern volatile S_SYSTEM_DATA *myData;

int rcv_msg(int fromPs, S_MSG_VAL *msg_val)
{
	int idx;
	
	if(myData->msg[fromPs].write_idx == myData->msg[fromPs].read_idx)
		return -1;

	myData->msg[fromPs].read_idx++;
	if(myData->msg[fromPs].read_idx >= MAX_MSG)
		myData->msg[fromPs].read_idx = 0;
	
	idx = myData->msg[fromPs].read_idx;
	memcpy(msg_val, (char *)&myData->msg[fromPs].msg_val[idx],
		sizeof(S_MSG_VAL));

	return idx;
}
	
void send_msg(int toPs, char *msg_val)
{
	int idx;
	
	idx = myData->msg[toPs].write_idx;
	idx++;
	if(idx >= MAX_MSG) idx = 0;
	
	memcpy((char *)&myData->msg[toPs].msg_val[idx],
		msg_val, sizeof(S_MSG_VAL));

	myData->msg[toPs].write_idx = idx;
}

void send_msg_ch_flag(int toPs, char *ch_flag)
{
	int idx;
	
	idx = myData->msg[toPs].write_idx;
	idx++;
	if(idx >= MAX_MSG) idx = 0;
	
	memcpy((char *)&myData->msg[toPs].msg_ch_flag[idx],
		ch_flag, sizeof(S_MSG_CH_FLAG));
}

