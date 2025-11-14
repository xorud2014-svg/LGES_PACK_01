#include <asm/io.h>
#include <rtl_core.h>
#include <pthread.h>
#include <math.h>
#include "../../INC/datastore.h"
#include "BoardControl.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;
extern S_DIO *myDio;
extern S_GROUP_DATA *myGroup;
extern S_BD_DATA *myBd;
extern S_CH_DATA *myCh;

void BoardControl(int bd)
{
#ifdef __COB__
	//int group=0;

	//if(bd >= myPs->config.installedBd) return;

	//myBd = &(myData->bData[bd]);

	//switch(myBd->state) {
	//	case B_IDLE:	bIdle(group, bd);		break;
	//	case B_STANDBY:	bStandby(group, bd);	break;
	//	case B_RUN:		bRun(group, bd);		break;
	//	case B_PAUSE:	bPause(group, bd);		break;
	//	case B_CALI:	bCali(group, bd);		break;
	//	default: break;
	//}
#endif
}

void bIdle(int group, int bd)
{
#ifdef __COB__
	int ch;

	switch(myBd->phase) {
		case P0:
			for(ch = 0; ch < MAX_CH_PER_BD; ch++){
				myCh = &(myData->cData[ch]);
				myCh->op.state = C_IDLE;
				myCh->op.phase = P0;
			}
			myBd->misc.faultChCnt = 0;
			myBd->misc.checkChOVP = 0;
			myBd->misc.runTime = 0;
			memset((char *)&myBd->cmd, 0, sizeof(char)*4);
			memset((char *)&myBd->signal, 0, sizeof(char)*MAX_SIGNAL);
			myBd->misc.semiSwitchState = SEMI_IDLE;
			myBd->phase++;
			break;
		case P5:
			bSemiSwitch(group, bd);
			bState(myBd, B_STANDBY, P0);
			break;
		default:
			myBd->phase++;
			break;
	}
#endif
}

void bStandby(int group, int bd)
{
#ifdef __COB__
	switch(myBd->phase) {
		case P0:
			myBd->misc.semiSwitchState = SEMI_PRE;
			bSemiSwitch(group, bd);

			myBd->misc.faultChCnt = 0;
			myBd->misc.checkChOVP = 0;
			myBd->misc.runTime = 0;
			myBd->phase++;
			break;
		case P1:
			if(myBd->cmd[B_USER_CMD] == B_CMD_RUN) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;

				myBd->signal[B_SIG_V_RANGE_SELECT] = P1;
				myBd->signal[B_SIG_I_RANGE_SELECT] = P1;
				bSendCmd_to_Ch(bd, C_USER_CMD, C_CMD_RUN);
				bState(myBd, B_RUN, P0);
			} else if(myBd->cmd[B_USER_CMD] == B_CMD_RESET) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bSendCmd_to_Ch(bd, C_USER_CMD, C_CMD_RESET);
			} else if(myBd->cmd[B_STEP_CMD] == B_STEP_CMD_RESET) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bSendCmd_to_Ch(bd, C_STEP_CMD, C_STEP_CMD_RESET);
			}else{
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
			}
			break;
		default: break;
	}
#endif
}

void bRun(int group, int bd)
{
#ifdef __COB__
	int stepNo, range, div, i, j, k, l, f, code;
	long val;
	
	stepNo = myData->gData[group].stepNo;

	switch(myBd->phase) {
		case P0:// Channel SSR ON Wait
			j = 0;
			for(i=0; i < myPs->config.chInBd[bd]; i++) {
				if(myData->cData[i].op.state == C_RUN) {
					if(myData->cData[i].op.phase == P3 // test run
					|| myData->cData[i].op.phase == P21)// test end 
						j++;
				} else if(myData->cData[i].op.state == C_FAULT) j++;
			}
			if(j == myPs->config.chInBd[bd]) {
				myBd->phase = P1;
			}
			break;
		case P1:
			if(myData->gData[group].stepType == STEP_CHARGE
				|| myData->gData[group].stepType == STEP_DISCHARGE) {
				div = 5;
				val = myData->testCond[group]
					.local_object[stepNo][IDX_LOC_OBJ_REF_V];
				range = myData->testCond[group]
					.local_object[stepNo][IDX_LOC_OBJ_RANGE_V] - 1;
				bVICmd_Setting(bd, val, range, div, 0);

				range = myData->testCond[group]
					.local_object[stepNo][IDX_LOC_OBJ_RANGE_I] - 1;
				if(myData->testCond[group].local_object[stepNo][IDX_LOC_OBJ_MODE]
					== MODE_CP) {
					val = 0;
					bVICmd_Setting(bd, val, range, div, 1);
				} else {
					val = myData->testCond[group]
							.local_object[stepNo][IDX_LOC_OBJ_REF_I];
					bVICmd_Setting(bd, val, range, div, 1);
				}
				myBd->misc.ch_sens_count = 0;
				myBd->misc.ch_sens_count_flag = P0;
			}
			for(i=0; i < myPs->config.chInBd[bd]; i++) {
				if(myData->cData[i].op.state == C_RUN
					&& myData->cData[i].op.phase == P3) {
					myData->cData[i].op.phase = P4;
				}
			}
			myBd->misc.avg_bd_Vsens = 0;
			myBd->misc.avg_bd_Isens = 0;
			myBd->misc.runTime = 0;
			myBd->phase = P10;
			break;
		case P10:
			myBd->misc.runTime += myPs->config.scan_period;

			if(myBd->cmd[B_USER_CMD] == B_CMD_STOP) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bSendCmd_to_Ch(bd, C_USER_CMD, C_CMD_STOP);
				break;
			} else if(myBd->cmd[B_USER_CMD] == B_CMD_PAUSE) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bSendCmd_to_Ch(bd, C_USER_CMD, C_CMD_PAUSE);
				break;
			} else if(myBd->cmd[B_USER_CMD] == B_CMD_NEXT_STEP) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bSendCmd_to_Ch(bd, C_USER_CMD, C_CMD_NEXT_STEP);
				break;
			}else{
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
			}
			
			j = 0; k = 0, f = 0, l = 0;
			for(i=0; i < myPs->config.chInBd[bd]; i++) {
				if(myData->cData[i].op.state == C_RUN
					&& myData->cData[i].op.phase == P21) {
					j++;
				} else if(myData->cData[i].op.state == C_FAULT) {
					j++;
					f++;
					code = (int)myData->cData[i].op.code;
					if((code >= C_CD_FAULT_CHECK_START
							&& code <= C_CD_FAULT_CHECK_END)
							|| code == C_CD_FAULT_PAUSE_CMD
							|| code == C_CD_FAULT_STOP_CMD){
					}else{
						l++;
					}
				} else if(myData->cData[i].op.state == C_PAUSE) k++;
				myBd->misc.faultChCnt = l;
			}
			if(j == myPs->config.chInBd[bd]) {
				for(i=0; i < myPs->config.chInBd[bd]; i++) {
					if(myData->cData[i].op.state == C_RUN
						&& myData->cData[i].op.phase == P21) {
						myData->cData[i].op.phase = P30;
					}
				}
				myBd->phase = P20;
			} else if(k == myPs->config.chInBd[bd]) { //pause + fault
				myBd->phase = P21;
			}

			bCalculateAverage(bd);
			break;
		case P20:
			myBd->phase = P30;
			break;
		case P21:
			if(myData->gData[group].stepType == STEP_CHARGE
				|| myData->gData[group].stepType == STEP_DISCHARGE) {
				bSemiSwitch(group, bd);
			}
			bState(myBd, B_PAUSE, P0);
			break;
		case P30:
			if(myBd->cmd[B_USER_CMD] == B_CMD_RUN) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bSemiSwitch(group, bd);
				myBd->signal[B_SIG_V_RANGE_SELECT] = P1;
				myBd->signal[B_SIG_I_RANGE_SELECT] = P1;
				bSendCmd_to_Ch(bd, C_USER_CMD, C_CMD_RUN);
				myBd->phase = P0;
			}else if(myBd->cmd[B_STEP_CMD] == B_STEP_CMD_RUN) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bSemiSwitch(group, bd);
				myBd->signal[B_SIG_V_RANGE_SELECT] = P1;
				myBd->signal[B_SIG_I_RANGE_SELECT] = P1;
				bSendCmd_to_Ch(bd, C_STEP_CMD, C_STEP_CMD_RUN);
				myBd->phase = P0;
			} else if(myBd->cmd[B_STEP_CMD] == B_STEP_CMD_STOP) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bState(myBd, B_STANDBY, P0);
				bSendCmd_to_Ch(bd, C_STEP_CMD, C_STEP_CMD_STOP);
			} else if(myBd->cmd[B_USER_CMD] == B_CMD_STOP) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bSendCmd_to_Ch(bd, C_USER_CMD, C_CMD_STOP);
			} else if(myBd->cmd[B_USER_CMD] == B_CMD_PAUSE) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bState(myBd, B_PAUSE, P0);
				bSendCmd_to_Ch(bd, C_USER_CMD, C_CMD_PAUSE);
			} else if(myBd->cmd[B_STEP_CMD] == B_STEP_CMD_CHECK_PAUSE) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bState(myBd, B_PAUSE, P0);
				bSendCmd_to_Ch(bd, C_STEP_CMD, C_STEP_CMD_CHECK_PAUSE);
			}else{
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
			}
			break;
		default: break;
	}
#endif
}

void bPause(int group, int bd)
{
#ifdef __COB__
	switch(myBd->phase) {
		case P0:
			myBd->cmd[B_USER_CMD] = B_CMD_NONE;
			myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
			myBd->phase = P1;
			break;
		case P1:
			if(myBd->cmd[B_USER_CMD] == B_CMD_CONTINUE) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bSemiSwitch(group, bd);
				myBd->signal[B_SIG_V_RANGE_SELECT] = P1;
				myBd->signal[B_SIG_I_RANGE_SELECT] = P1;
				bSendCmd_to_Ch(bd, C_USER_CMD, C_CMD_CONTINUE);
				bState(myBd, B_RUN, P0);
			} else if(myBd->cmd[B_USER_CMD] == B_CMD_STOP) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bState(myBd, B_STANDBY, P0);
				bSendCmd_to_Ch(bd, C_USER_CMD, C_CMD_STOP);
			} else if(myBd->cmd[B_USER_CMD] == B_CMD_RESET) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bState(myBd, B_IDLE, P0);
			} else if(myBd->cmd[B_USER_CMD] == B_CMD_CLEAR) {
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
				bState(myBd, B_IDLE, P0);
			}else{
				myBd->cmd[B_USER_CMD] = B_CMD_NONE;
				myBd->cmd[B_STEP_CMD] = B_CMD_NONE;
			}
			break;
		default: break;
	}
#endif
}

void bCali(int group, int bd)
{
#ifdef __COB__
	switch(myBd->phase) {
		case P0://cali
			break;
		case P1: //check
			break;
		default: break;
	}
#endif
}

void bCalculateAverage(int bd)
{
#ifdef __COB__
	int i, count;
	double sumV, sumI;

	count = 0; sumV = 0.0; sumI = 0.0;

	for(i=0; i < myPs->config.chInBd[bd]; i++) {
		if(myData->cData[i].op.state == C_RUN) {
			count++;
			sumV += myData->cData[i].misc.tmpVsens;
			sumI += myData->cData[i].misc.tmpIsens;
		}
	}

	if(count != 0) {
		sumV /= (double)count;
		sumI /= (double)count;
		myBd->misc.avg_bd_Vsens = (long)sumV;
		myBd->misc.avg_bd_Isens = (long)sumI;

		sumV = (sumV / 1000.0) * (sumI /  1000.0);
		if(sumV < 0.0) sumV *= (-1.0);
		myBd->misc.avg_bd_watt = (long)(sumV / 1000.0); //(1mW/div)
	} else {
		myBd->misc.avg_bd_Vsens = 0;
		myBd->misc.avg_bd_Isens = 0;
		myBd->misc.avg_bd_watt = 0;
	}
#endif
}

void bReadOTFault(int bd)
{
#ifdef __COB__
	unsigned char val, mask;
	int addr, base_addr, addr_step, ot_in;

	if(myPs->misc.timer_1sec 
		< myData->dio.config.dioDelay) {
		return;
	}

	base_addr = 0x620;
	addr_step = 0x10;
	ot_in = 0x01;

	addr = base_addr + addr_step * bd;

	val = inb(addr + ot_in);

	mask = 0x01; 
/*kjg_110813_w
	if((val & mask) != P0) {
		if(myBd->signal[B_SIG_OT_DETECT] == P0){
			myBd->signal[B_SIG_OT_DETECT] = P1;
			send_msg(BD_TO_DIO, MSG_BD_DIO_OT, bd,
				M_CD_FAULT_OT);
		}
	} else {
		myBd->signal[B_SIG_OT_DETECT] = P0;
	}*/
#endif
}

void bSemiSwitch(int group, int bd)
{
#ifdef __COB__
	int stepNo, rangeV, rangeI, stepType, div;
	long val1, max_val2, min_val2;

	stepNo = myData->gData[group].stepNo;
	stepType = myData->testCond[group].local_object[stepNo][IDX_LOC_OBJ_TYPE];
	if(myBd->state == B_RUN){
		rangeV = myData->testCond[group].local_object[stepNo][IDX_LOC_OBJ_RANGE_V] - 1;
		rangeI = myData->testCond[group].local_object[stepNo][IDX_LOC_OBJ_RANGE_I] - 1;
	}else{
		rangeV = 0;
		rangeI = 0;
	}

	val1 = myPs->config.maxV[rangeV];
	max_val2 = myPs->config.maxI[rangeI];
	min_val2 = myPs->config.minI[rangeI];
	div = 5;

	if(myBd->misc.semiSwitchState == SEMI_IDLE) {
		bVICmd_Setting(bd, val1*(-1), rangeV, div, 0);
		bVICmd_Setting(bd, max_val2, rangeI, div, 1);
		myBd->misc.semiSwitchState = SEMI_PRE;
	} else if(myBd->misc.semiSwitchState == SEMI_PRE) {
		if(stepType == STEP_CHARGE) {
			bVICmd_Setting(bd, val1, rangeV, div, 0);
			bVICmd_Setting(bd, min_val2, rangeI, div, 1);
			myBd->misc.semiSwitchState = SEMI_V_P;
		} else if(stepType == STEP_DISCHARGE || stepType == STEP_Z) {
			bVICmd_Setting(bd, val1*(-1), rangeV, div, 0);
			bVICmd_Setting(bd, max_val2, rangeI, div, 1);
			myBd->misc.semiSwitchState = SEMI_V_N;
		}
	} else if(myBd->misc.semiSwitchState == SEMI_V_P) {
		//kjg_110813_w if(stepType == STEP_CHARGE || stepType == STEP_Z_CHARGE) {
		if(stepType == STEP_CHARGE) {
			bVICmd_Setting(bd, val1, rangeV, div, 0);
			bVICmd_Setting(bd, min_val2, rangeI, div, 1);
			myBd->misc.semiSwitchState = SEMI_PRE;
		//kjg_110813_w } else if(stepType == STEP_DISCHARGE || stepType == STEP_Z_DISCHARGE) {
		} else if(stepType == STEP_DISCHARGE) {
			bVICmd_Setting(bd, val1*(-1), rangeV, div, 0);
			bVICmd_Setting(bd, max_val2, rangeI, div, 1);
			myBd->misc.semiSwitchState = SEMI_V_N;
		} else {
			bVICmd_Setting(bd, val1, rangeV, div, 0);
			bVICmd_Setting(bd, min_val2, rangeI, div, 1);
			myBd->misc.semiSwitchState = SEMI_PRE;
		}
	} else if(myBd->misc.semiSwitchState == SEMI_V_N) {
		//kjg_110813_w if(stepType == STEP_CHARGE || stepType == STEP_Z_CHARGE) {
		if(stepType == STEP_CHARGE) {
			bVICmd_Setting(bd, val1, rangeV, div, 0);
			bVICmd_Setting(bd, min_val2, rangeI, div, 1);
			myBd->misc.semiSwitchState = SEMI_V_P;
		//kjg_110813_w } else if(stepType == STEP_DISCHARGE || stepType == STEP_Z_DISCHARGE) {
		} else if(stepType == STEP_DISCHARGE) {
			bVICmd_Setting(bd, val1*(-1), rangeV, div, 0);
			bVICmd_Setting(bd, max_val2, rangeI, div, 1);
			myBd->misc.semiSwitchState = SEMI_PRE;
		} else {
			bVICmd_Setting(bd, val1*(-1), rangeV, div, 0);
			bVICmd_Setting(bd, max_val2, rangeI, div, 1);
			myBd->misc.semiSwitchState = SEMI_PRE;
		}
	}
#endif
}

void bSendCmd_to_Ch(int bd, int signal, int val)
{
#ifdef __COB__
	int ch;

	for(ch=0; ch < myPs->config.chInBd[bd]; ch++) {
		myData->cData[ch].cmd[signal] = (unsigned char)val;
	}
#endif
}

void bState(S_BD_DATA *myBd, unsigned char state, unsigned char phase)
{
#ifdef __COB__
	myBd->state = state;
	myBd->phase = phase;
#endif
}

void bSendSignal_to_Ch(int bd, int signal, int val)
{
#ifdef __COB__
	int ch;

	for(ch=0; ch < myPs->config.chInBd[bd]; ch++) {
		myData->cData[ch].signal[signal] = (unsigned char)val;
	}
#endif
}

void bVICmd_Setting(int bd, long value, int range, int div, int type)
{
#ifdef __COB__
	if(type == 0) { //V
		myData->bData[bd].signal[B_SIG_V_CMD_OUTPUT] = P1;
	} else { //I
		myData->bData[bd].signal[B_SIG_I_CMD_OUTPUT] = P1;
		if(value >= 0) myData->bData[bd].signal[B_SIG_CD_SELECT] = P1;
		else myData->bData[bd].signal[B_SIG_CD_SELECT] = P11;
	}

	myData->bData[bd].misc.VICmd[type].value = value;
	myData->bData[bd].misc.VICmd[type].range = range;
	myData->bData[bd].misc.VICmd[type].div = div;
#endif
}

void bVICmd_Output(int bd)
{
#ifdef __COB__
	int group, val, type;

	if(bd >= myPs->config.installedBd) return;

	group = 0;
	val = 0;

	if(myData->gData[group].state == G_CALI) {
		if(myData->gData[group].phase < P30) {
			val = 1;
		} else {
			val = 0;
		}
	} else {
		if(bd == 0) val = 1;
	}
	if(val == 0) return;

	myBd = &(myData->bData[bd]);

	myGroup = &(myData->gData[group]);

	if(myBd->signal[B_SIG_V_CMD_OUTPUT] == P1) {
		myBd->signal[B_SIG_V_CMD_OUTPUT] = P0;
		type = 0;
		val = bVCmd_Calculate(bd);

		bDA_Output(bd, val, type);
	}

	if(myBd->signal[B_SIG_I_CMD_OUTPUT] == P1) {
		myBd->signal[B_SIG_I_CMD_OUTPUT] = P0;
		type = 1;
		val = bICmd_Calculate(bd);

		bDA_Output(bd, val, type);
	}
#endif
}

int bVCmd_Calculate(int bd)
{
	double tmp=0.0;
#ifdef __COB__
    int type, range, div, point;//, tmp_bd;
    double  value, ratio, aux_offset=0;//, max, min;

	type = 0;
	point = 0;
	range = myBd->misc.VICmd[type].range;
	div = myBd->misc.VICmd[type].div;

	value = (double)myBd->misc.VICmd[type].value;

	if(div < 10) {
		tmp = value;
	} else {
		div -= 10;
		tmp = value;
	}

	//S_5V_3A : (1M+100k) / 1M = 1.1
	//S_LGC_5V_3A : (2M+100k) / 2M = 1.05 offset -130mV
	//S_LGC_5V_3A_25KW : (2M+100k) / 2M = 1.05 offset -130mV
	//L_5V_15A_10A : (2M+100k) / 2M = 1.05 offset -130mV

	if(myBd->misc.VICmd[1].value >= 0) {
		ratio = myPs->config.daRatioV_P[range];
		tmp = tmp * ratio + myPs->config.daOffsetV_P[range];
	} else {
		ratio = myPs->config.daRatioV_N[range];
		tmp = tmp * ratio + myPs->config.daOffsetV_N[range];
	}
	aux_offset = myPs->config.maxV[range]
		* myPs->config.auxRateV[range] / 2.0;

	tmp -= aux_offset;

	tmp = tmp * myGroup->cali_main_dac.DA_A[type][0][0]
		+ myGroup->cali_main_dac.DA_B[type][0][0];

	//0.0032767 = 32767 / 10000000uV
	tmp *= 0.0032767;
	if(div != 5) tmp = tmp / 5.0 * div;
	if(tmp < 0) tmp = ceil(tmp-0.5);
	else tmp = floor(tmp+0.5);

	myData->bData[bd].misc.preVref = tmp;
#endif
	return (int)tmp;
}

int bICmd_Calculate(int bd)
{
	double tmp=0.0;
#ifdef __COB__
    int type, range, div, point;//, tmp_bd, point=0;
	double value, aux_offset, ratio;//, max, min;

	type = 1;
	point = 0;
	range = myBd->misc.VICmd[type].range;
	div = myBd->misc.VICmd[type].div;

	value = (double)myBd->misc.VICmd[type].value;

	if(div < 10) {
		tmp = value;
	} else {
		div -= 10;
		tmp = value;
	}

	if(myBd->misc.VICmd[type].value >= 0) { //charge
		tmp = tmp * myGroup->cali_main_dac.DA_A[type][range][1]
			+ myGroup->cali_main_dac.DA_B[type][range][1];
		ratio = myPs->config.daRatioI_P[range];
		tmp = tmp * ratio + myPs->config.daOffsetI_P[range];
	} else {
		tmp = tmp * myGroup->cali_main_dac.DA_A[type][range][0]
			+ myGroup->cali_main_dac.DA_B[type][range][0];
		ratio = myPs->config.daRatioI_N[range];
		tmp = tmp * (-1.0) * ratio + myPs->config.daOffsetI_N[range];
	}

	aux_offset = 0;

	tmp -= aux_offset;
	tmp *= 0.0032767;

	if(div != 5) tmp = tmp / 5.0 * div;
	if(tmp < 0) tmp = ceil(tmp-0.5);
	else tmp = floor(tmp +0.5);

	myData->bData[bd].misc.preIref = tmp;
#endif
	return (int)tmp;
}

void bDA_Output(int bd, int val, int type)
{
#ifdef __COB__
	int base_addr, da_h_byte, da_l, group;
	U_ADDA	value;

	group = 0;

	base_addr = 0x610;
	da_h_byte = 0x00;

	if(type == 0) { //V
		da_l = 0x01 + 0x02 * group;
	} else { //I
		da_l = 0x02 + 0x02 * group;
	}

	value.val = (short int)val;
	
	outb(value.byte[1], base_addr + da_h_byte); //high
	outb(value.byte[0], base_addr + da_l); //low
#endif
}

void bCDSelect_Output(int bd)
{
#ifdef __COB__
	unsigned char cd_flag;
	int	addr, base_addr, addr_step, cd_cs;
	
	if(bd >= myPs->config.installedBd) return;

	myBd = &(myData->bData[bd]);

	if(myBd->signal[B_SIG_CD_SELECT] == P0) return;

	base_addr = 0x620;
	addr_step = 0x10;
	cd_cs = 0x07;

	if(myBd->signal[B_SIG_CD_SELECT] == P1) {
		cd_flag = 0x00; //charge
	} else {
		cd_flag = 0x01; //discharge
	}
	myBd->signal[B_SIG_CD_SELECT] = P0;

	addr = base_addr + addr_step * bd + cd_cs;
	outb(cd_flag, addr);
#endif
}

void bRangeRelay_Output(int bd)
{
#ifdef __COB__
	unsigned char out_flag;
	int group, stepNo, rangeI, rangeV, type, ch;
	int	addr, base_addr, addr_step, range_cs, addr_div;

	if(bd >= myPs->config.installedBd) return;

	myBd = &(myData->bData[bd]);
	group = 0;

	base_addr = 0x620;
	addr_step = 0x10;
	range_cs = 0x00;
	addr_div = 0x0F;

	if(myData->gData[group].state == G_CALI) {
		type = myPs->signal[M_SIG_CALI_VI_SELECT];
		if(type == 0) {
			rangeI = RANGE1;
		} else {
			rangeI = myPs->signal[M_SIG_CALI_RANGE_I];
		}
	} else {
		stepNo = myData->gData[group].stepNo;
		rangeV = myData->testCond[group]
			.local_object[stepNo][IDX_LOC_OBJ_RANGE_V];
		if(rangeV < 1) rangeV = 1; //kjg_110814
		myData->bData[bd].misc.rangeV = (unsigned char)rangeV-1;

		rangeI = myData->testCond[group]
			.local_object[stepNo][IDX_LOC_OBJ_RANGE_I];
		if(rangeI < 1) rangeI = 1; //kjg_110814
		myData->bData[bd].misc.rangeI = (unsigned char)rangeI-1;
		myData->bData[bd].misc.refV
			= myData->testCond[group].local_object[stepNo][IDX_LOC_OBJ_REF_V];
		myData->bData[bd].misc.refI
			= myData->testCond[group].local_object[stepNo][IDX_LOC_OBJ_REF_I];
	}

	out_flag = 0x00;
	ch = myPs->config.chPerBd * bd;
	if(myData->cData[ch].signal[C_SIG_RANGE_RELAY_ON] == P1) {
		myData->cData[ch].signal[C_SIG_RANGE_RELAY_ON] = P0;
		switch(rangeI) {
			case RANGE2:
				myData->bData[bd].rangeRelayState[0][0] = 0x07;
				break;
			default:
				myData->bData[bd].rangeRelayState[0][0] = 0x00;
				break;
		}
		out_flag = 0x01;
	} else if(myData->cData[ch].signal[C_SIG_RANGE_RELAY_OFF] == P1) {
		myData->cData[ch].signal[C_SIG_RANGE_RELAY_OFF] = P0;
		myData->bData[bd].rangeRelayState[0][0] = 0x00;
		out_flag = 0x01;
	}

	if(out_flag == 0x01) {
		addr = base_addr + addr_step * bd;
		outb(0x10, addr + addr_div);
		outb(myData->bData[bd].rangeRelayState[0][0], addr + range_cs);
		outb(0x00, addr + addr_div);
	}
#endif
}

void bRunRelay_Output(int bd)
{
#ifdef __COB__
	unsigned char bit_flag, out_flag[8];
	int ch, channel, div, shift;
	int	addr, base_addr, addr_step, run_cs, addr_div;
	
	if(bd >= myPs->config.installedBd) return;

	myBd = &(myData->bData[bd]);

	base_addr = 0x620;
	addr_step = 0x10;
	run_cs = 0x00;
	addr_div = 0x0F;

	for(div=0; div < 8; div++) out_flag[div] = 0x00;
	
	for(channel=0; channel < myPs->config.chInBd[bd]; channel++) {
		ch = myPs->config.chPerBd * bd + channel;
		div = channel / 8;
		shift = channel % 8;
		bit_flag = 0x01;
		if(myData->cData[ch].signal[C_SIG_RUN_RELAY_ON] == P1) {
			myData->cData[ch].signal[C_SIG_RUN_RELAY_ON] = P0;
			bit_flag = bit_flag << shift;
			myData->bData[bd].runRelayState[div] |= bit_flag;
			out_flag[div] = 0x01;
		} else if(myData->cData[ch].signal[C_SIG_RUN_RELAY_OFF]
			== P1) {
			myData->cData[ch].signal[C_SIG_RUN_RELAY_OFF] = P0;
			bit_flag = bit_flag << shift;
			myData->bData[bd].runRelayState[div] &= ~bit_flag;
			out_flag[div] = 0x01;
		}
	}

	channel = myPs->config.chInBd[bd] / 8;
	if((myPs->config.chInBd[bd] % 8) != 0) channel++;

	addr = base_addr + addr_step * bd;

	outb(0x01, addr + addr_div);
	for(div=0; div < channel; div++) {
		if(out_flag[div] == 0x01) {
			outb(myData->bData[bd].runRelayState[div],
				addr + run_cs + div);
		}
	}
	outb(0x00, addr + addr_div);
#endif
}

void bParallelRelay_Output(int bd)
{
#ifdef __COB__
	unsigned char bit_flag;
	int	group, addr, base_addr, addr_step, parallel_cs, addr_div;
	
	if(bd >= myPs->config.installedBd) return;

	myBd = &(myData->bData[bd]);
	group = 0;

	base_addr = 0x620;
	addr_step = 0x10;
	parallel_cs = 0x00;
	addr_div = 0x0F;

	if(myData->gData[group].state == G_CALI) {
		if(myData->mData.signal[M_SIG_CALI_PARALLEL] == P0) {
			bit_flag = 0x00;
		} else {
			bit_flag = 0x1F;
		}
	} else {
		if(myData->ChAttribute[0].opType == 0) { //independent
			bit_flag = 0x00;
		} else { //parallel
			bit_flag = 0x00;
			if((myData->bData[bd].runRelayState[0] & 0x01) != 0x00) {
				bit_flag |= 0x01;
			}
			if((myData->bData[bd].runRelayState[0] & 0x04) != 0x00) {
				bit_flag |= 0x02;
			}
			if((myData->bData[bd].runRelayState[0] & 0x10) != 0x00) {
				bit_flag |= 0x04;
			}
			if((myData->bData[bd].runRelayState[0] & 0x40) != 0x00) {
				bit_flag |= 0x08;
			}
			if((myData->bData[bd].runRelayState[1] & 0x01) != 0x00) {
				bit_flag |= 0x10;
			}
		}
	}

	addr = base_addr + addr_step * bd;
	outb(0x20, addr + addr_div);
	outb(bit_flag, addr + parallel_cs);
	outb(0x00, addr + addr_div);
#endif
}
