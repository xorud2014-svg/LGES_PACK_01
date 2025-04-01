#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "local_message.h"
#include "jig_type.h"
#include "tray_type.h"
#include "door_type.h"
#include "stopper_type.h"
#include "stacker_type.h"
#include "lamp_type.h"
#include "emg_type.h"
#include "job_change.h"
#include "motor_control.h"
#include "main.h"

volatile S_SYSTEM_DATA	*myData;
volatile S_JIG_DATA		*myPs; //my process : Jig_Control[?]
char psName[PROCESS_NAME_SIZE];
	
int main(int argc, char *argv[])
{
    int rtn;
    struct timeval tv;
    fd_set rfds;
    
	rtn = Initialize(argc, argv);
	if(rtn < 0) {
		if(rtn > (-10)) {
			printf("Jig_Control Initialize fail(%d) %d, %s\n",
				rtn, argc, argv[0]);
		} else {
			userlog(DEBUG_LOG, psName,
				"Jig_Control Initialize fail(%d) %d, %s\n",
				rtn, argc, argv[0]);
		}
		return 0;
	}

    while(myData->AppControl.signal[myPs->misc.psSignal] == P1) {
    	tv.tv_sec = 0;
		tv.tv_usec = 250000;
		FD_ZERO(&rfds);

		rtn = select(0, &rfds, NULL, NULL, &tv);
		if(rtn == 0) {
			Jig_Control();
		}
    }

	Close_JigControl();
    return 0;
}

void Jig_Control(void)
{
	Check_Message();
	Check_Signal();
	Check_JobChange();
	Check_Motor_Control();
	Check_Tray();
	Check_Door();
	Check_Stopper();
	Check_Lamp();
	Check_StackerCrane();
	Check_EMG();
	
	switch(myPs->jigState) {
		case J_IDLE:			jIdle();			break;
		case J_DISCONTACT: 		jDisContact();		break;
		case J_TO_CONTACT: 		jTo_Contact();		break;
		case J_CONTACT: 		jContact();			break;
		case J_TO_DISCONTACT: 	jTo_DisContact();	break;
		case J_TRAYCHECK:		jTrayCheck();		break;
		case J_FAIL:			jFail();			break;
		case J_EMG:				jEMG();				break;
		default: break;
	}
}

void Check_Signal(void)
{
	if(myPs->signal[J_SIG_TRY_CONTACT] == P1) {
		myPs->signal[J_SIG_TRY_CONTACT] = P0;
		myPs->jigState = J_TO_CONTACT;
		myPs->microState = P0;
		myPs->code = J_CD_NONE;
	} else if(myPs->signal[J_SIG_TRY_DISCONTACT] == P1) {
		myPs->signal[J_SIG_TRY_DISCONTACT] = P0;
		myPs->jigState = J_TO_DISCONTACT;
		myPs->microState = P0;
		myPs->code = J_CD_NONE;
	} else if(myPs->signal[J_SIG_TRY_TRAY_CHECK] == P1) {
		myPs->signal[J_SIG_TRY_TRAY_CHECK] = P0;
		myPs->jigState = J_TRAYCHECK;
		myPs->microState = P0;
		myPs->code = J_CD_NONE;
	}
}

void Close_JigControl(void)
{
    myData->AppControl.signal[myPs->misc.psSignal] = P3;
	
	Close_SystemMemory();
}
