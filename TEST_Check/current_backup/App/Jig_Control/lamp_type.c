#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "common_utils.h"
#include "local_utils.h"
#include "local_message.h"
#include "lamp_type.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_JIG_DATA		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Check_Lamp(void)
{
	if(myPs->config.jigUseFlag == UNUSE) {
		return;
	}

	switch(myPs->config.jigType) {
		case 1:
		case 2:
		case 4:
		case 6:
		case 7:
			Check_Lamp_1_2_4_6_7(); break;
		default: break;
	}
}

void Check_Lamp_1_2_4_6_7(void)
{
	int ch=0;

	if(myPs->signal[J_SIG_LAMP_GREEN] == P1){
		myPs->signal[J_SIG_LAMP_GREEN] = P0;
		Select_OutPoint(myPs->misc.JigNo, ch, O_TOWER_LAMP_GREEN, ON);
		Select_OutPoint(myPs->misc.JigNo, ch, O_TOWER_LAMP_AMBER, OFF);
		Select_OutPoint(myPs->misc.JigNo, ch, O_TOWER_LAMP_RED, OFF);
		Select_OutPoint(myPs->misc.JigNo, ch, O_LED_GREEN, ON);
		Select_OutPoint(myPs->misc.JigNo, ch, O_LED_AMBER, OFF);
		Select_OutPoint(myPs->misc.JigNo, ch, O_LED_RED, OFF);
	}else if(myPs->signal[J_SIG_LAMP_AMBER] == P1){
		myPs->signal[J_SIG_LAMP_AMBER] = P0;
		Select_OutPoint(myPs->misc.JigNo, ch, O_TOWER_LAMP_GREEN, OFF);
		Select_OutPoint(myPs->misc.JigNo, ch, O_TOWER_LAMP_AMBER, ON);
		Select_OutPoint(myPs->misc.JigNo, ch, O_TOWER_LAMP_RED, OFF);
		Select_OutPoint(myPs->misc.JigNo, ch, O_LED_GREEN, OFF);
		Select_OutPoint(myPs->misc.JigNo, ch, O_LED_AMBER, ON);
		Select_OutPoint(myPs->misc.JigNo, ch, O_LED_RED, OFF);
	}else if(myPs->signal[J_SIG_LAMP_RED] == P1){
		myPs->signal[J_SIG_LAMP_RED] = P0;
		Select_OutPoint(myPs->misc.JigNo, ch, O_TOWER_LAMP_GREEN, OFF);
		Select_OutPoint(myPs->misc.JigNo, ch, O_TOWER_LAMP_AMBER, OFF);
		Select_OutPoint(myPs->misc.JigNo, ch, O_TOWER_LAMP_RED, ON);
		Select_OutPoint(myPs->misc.JigNo, ch, O_LED_GREEN, OFF);
		Select_OutPoint(myPs->misc.JigNo, ch, O_LED_AMBER, OFF);
		Select_OutPoint(myPs->misc.JigNo, ch, O_LED_RED, ON);
	}
}

