#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "local_message.h"
#include "stopper_type.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_JIG_DATA		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Stopper(void)
{
	if(myPs->config.jigUseFlag == UNUSE) {
		myPs->stopperState = S_DISCONTACT;
		return;
	}

	switch(myPs->config.jigType) {
		case 4:
			Check_Stopper_4();
			break;
		default:
			Check_Stopper_1(); 
			break;
	}
}


void Check_Stopper_1(void)
{
	myPs->stopperState = S_DISCONTACT;
}

void Check_Stopper_4(void)
{
	int flag1, ch=0;

	flag1 = Read_InPoint(myPs->misc.JigNo, ch, I_JIG_STOPPER_STATE);

	if(flag1 == P1) {
		myPs->stopperState = S_CONTACT;
	} else {
		myPs->stopperState = S_DISCONTACT;
	}
}
