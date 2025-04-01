#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "common_message.h"
#include "local_utils.h"
#include "local_message.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_JIG_DATA *myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Message(void)
{
	switch(myPs->misc.JigNo) {
		case 0:	
				msgParsing(JIGM_TO_JIGC1);	
				msgParsing(COB1_TO_JIGC1);	
				break;
		case 1:	
				msgParsing(JIGM_TO_JIGC2);	
				msgParsing(COB2_TO_JIGC2);	
				break;
/*		case 2:	msgParsing(JIGM_TO_JIGC3);	break;
		case 3:	msgParsing(JIGM_TO_JIGC4);	break;
		case 4:	msgParsing(JIGM_TO_JIGC5);	break;
		case 5:	msgParsing(JIGM_TO_JIGC6);	break;
		case 6:	msgParsing(JIGM_TO_JIGC7);	break;
		case 7:	msgParsing(JIGM_TO_JIGC8);	break;*/
		default: break;
	}
}

void msgParsing(int fromPs)
{
	int idx;
	S_MSG_VAL  RecvMsg;
	
	idx = rcv_msg(fromPs, (S_MSG_VAL *)&RecvMsg);
	if(idx < 0) return;
	
	switch(fromPs) {
		case JIGM_TO_JIGC1:
		case JIGM_TO_JIGC2:
/*		case JIGM_TO_JIGC3:
		case JIGM_TO_JIGC4:
		case JIGM_TO_JIGC5:
		case JIGM_TO_JIGC6:
		case JIGM_TO_JIGC7:
		case JIGM_TO_JIGC8:*/
			msgParsing_JigM_to_JigC(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		case COB1_TO_JIGC1:
		case COB2_TO_JIGC2:
			msgParsing_COB_to_JigC(fromPs, idx, (S_MSG_VAL *)&RecvMsg);
			break;
		default:
			userlog(DEBUG_LOG, psName,
				"Msg Direction UnKnown : %d %d %d\n", fromPs, idx, RecvMsg.msg);
			break;
	}
}

void msgParsing_JigM_to_JigC(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	unsigned char state;

	switch(RecvMsg->msg) {
		case MSG_JIGM_JIGC_TRY_CONTACT:
			myPs->signal[J_SIG_TRY_CONTACT] = P1;
			break;
		case MSG_JIGM_JIGC_TRY_DISCONTACT:
			myPs->signal[J_SIG_TRY_DISCONTACT] = P1;
			break;
		case MSG_JIGM_JIGC_TRY_TRAY_CHECK:
			myPs->signal[J_SIG_TRY_TRAY_CHECK] = P1;
			break;
		case MSG_JIGM_JIGC_LAMP:
			state = (unsigned char)RecvMsg->val[0];
			switch(state){
				case G_RUN:
					myPs->signal[J_SIG_LAMP_GREEN] = P1;
					break;
				case G_PAUSE:
					myPs->signal[J_SIG_LAMP_RED] = P1;
					break;
				default:
					myPs->signal[J_SIG_LAMP_AMBER] = P1;
					break;
			}
			break;
		case MSG_JIGM_JIGC_DOOR_OPEN:
			myPs->signal[J_SIG_DOOR_OPEN] = P1;
			userlog(DEBUG_LOG, psName, "Door Open : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
		case MSG_JIGM_JIGC_DOOR_CLOSE:
			myPs->signal[J_SIG_DOOR_OPEN] = P0;
			userlog(DEBUG_LOG, psName, "Door Close : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
		case MSG_JIGM_JIGC_PLC_ALARM_AREA_SENSOR:
			//myPs->signal[J_SIG_PLC_ALARM_AREA_SENSOR] = P1;
			userlog(DEBUG_LOG, psName, "area_sensor\n");
			break;
		case MSG_JIGM_JIGC_PLC_ALARM_MANUAL_DOOR_OPEN:
			//myPs->signal[J_SIG_PLC_ALARM_MANUAL_DOOR_OPEN] = P1;
			userlog(DEBUG_LOG, psName, "manual_door\n");
			break;
		case MSG_JIGM_JIGC_PLC_ALARM_AIR:
			//myPs->signal[J_SIG_PLC_ALARM_AIR] = P1;
			userlog(DEBUG_LOG, psName, "air\n");
			break;
		case MSG_JIGM_JIGC_PLC_ALARM_APR_S:
			myPs->signal[J_SIG_PLC_ALARM_APR_S] = P1;
			userlog(DEBUG_LOG, psName, "apr_s\n");
			break;
		case MSG_JIGM_JIGC_PLC_ALARM_REF:
			myPs->signal[J_SIG_PLC_ALARM_REF] = P1;
			userlog(DEBUG_LOG, psName, "ref\n");
			break;
		case MSG_JIGM_JIGC_PLC_ALARM_COIL_TRIP:
			myPs->signal[J_SIG_PLC_ALARM_COIL_TRIP] = P1;
			userlog(DEBUG_LOG, psName, "coil_trip\n");
			break;
		case MSG_JIGM_JIGC_PLC_ALARM_TEMP:
			myPs->signal[J_SIG_PLC_ALARM_TEMP] = P1;
			userlog(DEBUG_LOG, psName, "temp\n");
			break;
		case MSG_JIGM_JIGC_PLC_ALARM_SMOKE:
			myPs->signal[J_SIG_PLC_ALARM_SMOKE] = P1;
			userlog(DEBUG_LOG, psName, "smoke\n");
			break;
		case MSG_JIGM_JIGC_PLC_ALARM_FIRE:
			myPs->signal[J_SIG_PLC_ALARM_FIRE] = P1;
			userlog(DEBUG_LOG, psName, "fire\n");
			break;
		case MSG_JIGM_JIGC_PLC_ALARM_EMG:
			myPs->signal[J_SIG_PLC_ALARM_EMG] = P1;
			userlog(DEBUG_LOG, psName, "emg\n");
			break;
		default:
			userlog(DEBUG_LOG, psName, "JigM to JigC Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}

void msgParsing_COB_to_JigC(int fromPs, int idx, S_MSG_VAL *RecvMsg)
{
	switch(RecvMsg->msg) {
		case MSG_COB_JIGC_JOB_CHANGE:
			myPs->signal[J_SIG_JOB_CHANGE] = RecvMsg->val[0];
			myPs->jobChangePhase = P0;
			myPs->jobChangeEnd = JOB_CHANGE_START;

			switch(myPs->signal[J_SIG_JOB_CHANGE]) {
				case 0:
					break;
				case 1:
					myPs->signal[J_SIG_PITCH_CHANGE] = 2;
					break;
				case 2:
					break;
				case 3:
					break;
				case 4:
					break;
				case 5:
					break;
				case 6:
					break;
				case 7:
					myPs->signal[J_SIG_PITCH_CHANGE] = 1;
					break;
				case 8:
					break;
				default: break;
			}
			myPs->pitchChangePhase = P0;
			myPs->pitchChangeEnd = PITCH_CHANGE_START;
			break;
		case MSG_COB_JIGC_PITCH_CHANGE:
			myPs->signal[J_SIG_PITCH_CHANGE] = RecvMsg->val[0];
			myPs->pitchChangePhase = P0;
			myPs->pitchChangeEnd = PITCH_CHANGE_START;
			break;
		case MSG_COB_JIGC_TRY_CONTACT:
			myPs->signal[J_SIG_TRY_CONTACT] = P1;
			break;
		case MSG_COB_JIGC_TRY_DISCONTACT:
			myPs->signal[J_SIG_TRY_DISCONTACT] = P1;
			break;
		default:
			userlog(DEBUG_LOG, psName, "COB to JigC Msg Unknown : %d %d %d\n",
				fromPs, idx, RecvMsg->msg);
			break;
	}
}
