#include <asm/io.h>
#include <rtl_core.h>
#include <pthread.h>
#include "../../INC/datastore.h"
#include "ch_utils.h"
#include "PWM3_Control.h"

extern S_SYSTEM_DATA *myData;
extern S_MODULE_DATA *myPs;
extern S_CH_DATA *myCh;

void PWM3_Control(int mainSlot)
{
	int slot, cmd, ch, max_ch, time_delay;

	if(myData->mData.misc.module_type != MODULE_PWM3) return;

	slot = mainSlot;
	max_ch = 8;
	switch(myData->AppControl.config.sbcType) {
		case SBC_WAFER_MARK533:
		case SBC_WAFER_MARK800:
			time_delay = 1; //1us
			break;
		case SBC_EM104_A5362:
			time_delay = 0;
			break;
		default:
			time_delay = 1; //1us
			break;
	}

	if(slot == 0) {
		cmd = PWM3_CMD_RD1;
		for(ch=0; ch < max_ch; ch++) PWM3_Write(ch, cmd);
		if(time_delay != 0) usleep(time_delay);
		for(ch=0; ch < max_ch; ch++) PWM3_Read(ch, 0);
		if(time_delay != 0) usleep(time_delay);

		cmd = PWM3_CMD_AD_VALUE3;
		for(ch=0; ch < max_ch; ch++) PWM3_Write(ch, cmd);
		if(time_delay != 0) usleep(time_delay);
		for(ch=0; ch < max_ch; ch++) PWM3_Read(ch, 0);

		for(ch=0; ch < myPs->config.installedCh; ch++) PWM3_CalChAverage(ch);
	} else if(slot == 1) {
		switch(myData->pwm3_ch.signal[PWM3_SIG_GET_PHASE1]) {
			case P0:
				cmd = PWM3_CMD_AD_VALUE1;
				myData->pwm3_ch.signal[PWM3_SIG_GET_PHASE1]++;
				break;
			case P1:
				cmd = PWM3_CMD_AD_VALUE2;
				myData->pwm3_ch.signal[PWM3_SIG_GET_PHASE1]++;
				break;
			case P2:
				cmd = PWM3_CMD_AD_VALUE4;
				myData->pwm3_ch.signal[PWM3_SIG_GET_PHASE1]++;
				break;
			case P3:
				cmd = PWM3_CMD_AD_VALUE5;
				myData->pwm3_ch.signal[PWM3_SIG_GET_PHASE1]++;
				break;
			default:
				cmd = PWM3_CMD_AH_WH;
				myData->pwm3_ch.signal[PWM3_SIG_GET_PHASE1] = P0;
				break;
		}
		for(ch=0; ch < max_ch; ch++) PWM3_Write(ch, cmd);
		if(time_delay != 0) usleep(time_delay);
		for(ch=0; ch < max_ch; ch++) PWM3_Read(ch, 0);
		if(time_delay != 0) usleep(time_delay);

		cmd = PWM3_CMD_EMG1;
		for(ch=0; ch < max_ch; ch++) PWM3_Write(ch, cmd);
		if(time_delay != 0) usleep(time_delay);
		for(ch=0; ch < max_ch; ch++) PWM3_Read(ch, 0);
	} else if(slot == 2) {
		switch(myData->pwm3_ch.signal[PWM3_SIG_GET_PHASE2]) {
			case P0:
				cmd = PWM3_CMD_AVG_V_I;
				myData->pwm3_ch.signal[PWM3_SIG_GET_PHASE2]++;
				break;
			case P1:
				cmd = PWM3_CMD_MAX_V_I;
				myData->pwm3_ch.signal[PWM3_SIG_GET_PHASE2]++;
				break;
			case P2:
				cmd = PWM3_CMD_MIN_V_I;
				myData->pwm3_ch.signal[PWM3_SIG_GET_PHASE2]++;
				break;
			case P3:
				cmd = PWM3_CMD_ETC1;
				myData->pwm3_ch.signal[PWM3_SIG_GET_PHASE2]++;
				break;
			default:
				cmd = PWM3_CMD_ETC2;
				myData->pwm3_ch.signal[PWM3_SIG_GET_PHASE2] = P0;
				break;
		}
		for(ch=0; ch < max_ch; ch++) PWM3_Write(ch, cmd);
		if(time_delay != 0) usleep(time_delay);
		for(ch=0; ch < max_ch; ch++) PWM3_Read(ch, 0);
		if(time_delay != 0) usleep(time_delay);

		cmd = PWM3_CMD_TEST_COND1;
		for(ch=0; ch < max_ch; ch++) PWM3_Write(ch, cmd);
		if(time_delay != 0) usleep(time_delay);
		for(ch=0; ch < max_ch; ch++) PWM3_Read(ch, 0);
	} else if(slot == 3) {
		cmd = PWM3_CMD_TEST_COND2;
		for(ch=0; ch < max_ch; ch++) PWM3_Write(ch, cmd);
		if(time_delay != 0) usleep(time_delay);
		for(ch=0; ch < max_ch; ch++) PWM3_Read(ch, 0);
		if(time_delay != 0) usleep(time_delay);

		cmd = PWM3_CMD_TEST_COND3;
		for(ch=0; ch < max_ch; ch++) PWM3_Write(ch, cmd);
		if(time_delay != 0) usleep(time_delay);
		for(ch=0; ch < max_ch; ch++) PWM3_Read(ch, 0);
	} else if(slot == 4) {
		cmd = PWM3_CMD_TEST_COND4;
		for(ch=0; ch < max_ch; ch++) PWM3_Write(ch, cmd);
		if(time_delay != 0) usleep(time_delay);
		for(ch=0; ch < max_ch; ch++) PWM3_Read(ch, 0);
		if(time_delay != 0) usleep(time_delay);

		cmd = PWM3_CMD_TEST_COND5;
		for(ch=0; ch < max_ch; ch++) PWM3_Write(ch, cmd);
		if(time_delay != 0) usleep(time_delay);
		for(ch=0; ch < max_ch; ch++) PWM3_Read(ch, 0);
	}
}

void PWM3_Write(int ch, int cmd)
{
	unsigned char wr_data[10];
	int i, range, j, max_count;
	double tmp;

	myCh = &(myData->cData[ch]);

	if(cmd == PWM3_CMD_TEST_COND1) {
		if(myCh->op.state == C_STANDBY || myCh->op.state == C_PAUSE) {
			if(myCh->signal[C_SIG_CALI_UPDATE] != P0) {
				if(myCh->signal[C_SIG_CALI_UPDATE] == P61) {
					cmd = PWM3_CMD_SYSTEM_MODEL;
					myCh->signal[C_SIG_CALI_UPDATE] = P62;
				} else if(myCh->signal[C_SIG_CALI_UPDATE] == P62) {
					cmd = PWM3_CMD_FIRMWARE_VERSION;
					myCh->signal[C_SIG_CALI_UPDATE] = P0;
				} else {
					cmd = PWM3_CMD_CALI_UPDATE;
					PWM3_send_cali_data(ch);
				}
			}
		}
	}

	if(cmd == PWM3_CMD_TEST_COND2) {
		if(myCh->op.state == C_STANDBY || myCh->op.state == C_PAUSE) {
			if(myCh->signal[C_SIG_CALI_UPDATE] != P0) {
				if(myCh->signal[C_SIG_CALI_UPDATE] == P61) {
					cmd = PWM3_CMD_SYSTEM_MODEL;
					myCh->signal[C_SIG_CALI_UPDATE] = P62;
				} else if(myCh->signal[C_SIG_CALI_UPDATE] == P62) {
					cmd = PWM3_CMD_FIRMWARE_VERSION;
					myCh->signal[C_SIG_CALI_UPDATE] = P0;
				} else {
					cmd = PWM3_CMD_CALI_UPDATE;
					PWM3_send_cali_data(ch);
				}
			}
		}
	}

	if(cmd == PWM3_CMD_TEST_COND3) {
		if(myCh->op.state == C_STANDBY || myCh->op.state == C_PAUSE) {
			if(myCh->signal[C_SIG_CALI_UPDATE] != P0) {
				if(myCh->signal[C_SIG_CALI_UPDATE] == P61) {
					cmd = PWM3_CMD_SYSTEM_MODEL;
					myCh->signal[C_SIG_CALI_UPDATE] = P62;
				} else if(myCh->signal[C_SIG_CALI_UPDATE] == P62) {
					cmd = PWM3_CMD_FIRMWARE_VERSION;
					myCh->signal[C_SIG_CALI_UPDATE] = P0;
				} else {
					cmd = PWM3_CMD_CALI_UPDATE;
					PWM3_send_cali_data(ch);
				}
			}
		}
	}

	if(cmd == PWM3_CMD_TEST_COND4) {
		if(myCh->op.state == C_CALI) {
			if(myCh->op.phase == P10 && myCh->signal[C_SIG_CALI_PHASE] == P1) {
				cmd = PWM3_CMD_DA_VALUE3;
				myData->pwm3_ch.ch[ch].da_val[0] = myCh->misc.cmd_v[0];
				myData->pwm3_ch.ch[ch].da_val[1] = myCh->misc.cmd_v[1];
			}
			if(myCh->op.phase == P11 && myCh->signal[C_SIG_CALI_PHASE] == P1) {
				cmd = PWM3_CMD_DA_VALUE3;
				range = myCh->op.rangeV;
				i = cFindDACaliPoint(ch, myCh->misc.cmd_v[range], 0, range);
				tmp = (double)myCh->misc.cmd_v[range]
					* myData->cali[ch].tmpData[0][range].DA_A[i]
					+ myData->cali[ch].tmpData[0][range].DA_B[i];
				if(range == (RANGE1-1)) {
					myData->pwm3_ch.ch[ch].da_val[0] = (long)tmp;
					myData->pwm3_ch.ch[ch].da_val[1] = myCh->misc.cmd_v[1];
				} else {
					myData->pwm3_ch.ch[ch].da_val[0] = myCh->misc.cmd_v[0];
					myData->pwm3_ch.ch[ch].da_val[1] = (long)tmp;
				}
			}
			if(myCh->op.phase == P20 && myCh->signal[C_SIG_CALI_PHASE] == P1) {
				cmd = PWM3_CMD_DA_VALUE3;
				myData->pwm3_ch.ch[ch].da_val[0] = myCh->misc.cmd_v[0];
				myData->pwm3_ch.ch[ch].da_val[1] = myCh->misc.cmd_v[1];
			}
			if(myCh->op.phase == P21 && myCh->signal[C_SIG_CALI_PHASE] == P1) {
				cmd = PWM3_CMD_DA_VALUE3;
				myData->pwm3_ch.ch[ch].da_val[0] = myCh->misc.cmd_v[0];
				myData->pwm3_ch.ch[ch].da_val[1] = myCh->misc.cmd_v[1];
			}
		} else if(myCh->op.state == C_STANDBY || myCh->op.state == C_PAUSE) {
			if(myCh->signal[C_SIG_CALI_UPDATE] != P0) {
				if(myCh->signal[C_SIG_CALI_UPDATE] == P61) {
					cmd = PWM3_CMD_SYSTEM_MODEL;
					myCh->signal[C_SIG_CALI_UPDATE] = P62;
				} else if(myCh->signal[C_SIG_CALI_UPDATE] == P62) {
					cmd = PWM3_CMD_FIRMWARE_VERSION;
					myCh->signal[C_SIG_CALI_UPDATE] = P0;
				} else {
					cmd = PWM3_CMD_CALI_UPDATE;
					PWM3_send_cali_data(ch);
				}
			}
		}
	}

	if(cmd == PWM3_CMD_TEST_COND5) {
		if(myCh->op.state == C_CALI) {
			if(myCh->op.phase == P10 && myCh->signal[C_SIG_CALI_PHASE] == P1) {
				cmd = PWM3_CMD_DA_VALUE4;
				myData->pwm3_ch.ch[ch].da_val[2] = myCh->misc.cmd_i[0];
				myData->pwm3_ch.ch[ch].da_val[3] = 0;
			}
			if(myCh->op.phase == P11 && myCh->signal[C_SIG_CALI_PHASE] == P1) {
				cmd = PWM3_CMD_DA_VALUE4;
				myData->pwm3_ch.ch[ch].da_val[2] = myCh->misc.cmd_i[0];
				myData->pwm3_ch.ch[ch].da_val[3] = 0;
			}
			if(myCh->op.phase == P20 && myCh->signal[C_SIG_CALI_PHASE] == P1) {
				cmd = PWM3_CMD_DA_VALUE4;
				myData->pwm3_ch.ch[ch].da_val[2] = myCh->misc.cmd_i[0];
				myData->pwm3_ch.ch[ch].da_val[3] = 0;
			}
			if(myCh->op.phase == P21 && myCh->signal[C_SIG_CALI_PHASE] == P1) {
				cmd = PWM3_CMD_DA_VALUE4;
				range = myCh->op.rangeI;
				i = cFindDACaliPoint(ch, myCh->misc.cmd_i[0], 1, range);
				tmp = (double)myCh->misc.cmd_i[0]
					* myData->cali[ch].tmpData[1][range].DA_A[i]
					+ myData->cali[ch].tmpData[1][range].DA_B[i];
				myData->pwm3_ch.ch[ch].da_val[2] = (long)tmp;
				myData->pwm3_ch.ch[ch].da_val[3] = 0;
			}
		} else if(myCh->op.state == C_STANDBY || myCh->op.state == C_PAUSE) {
			if(myCh->signal[C_SIG_CALI_UPDATE] != P0) {
				if(myCh->signal[C_SIG_CALI_UPDATE] == P61) {
					cmd = PWM3_CMD_SYSTEM_MODEL;
					myCh->signal[C_SIG_CALI_UPDATE] = P62;
				} else if(myCh->signal[C_SIG_CALI_UPDATE] == P62) {
					cmd = PWM3_CMD_FIRMWARE_VERSION;
					myCh->signal[C_SIG_CALI_UPDATE] = P0;
				} else {
					cmd = PWM3_CMD_CALI_UPDATE;
					PWM3_send_cali_data(ch);
				}
			}
		}
	}

	wr_data[0] = (unsigned char)((cmd & 0xFF00) >> 8);
	wr_data[1] = (unsigned char)(cmd & 0x00FF);

	switch(cmd) {
		case PWM3_CMD_SYSTEM_MODEL:
			memcpy((char *)&wr_data[2],
				(char *)&myData->AppControl.config.systemModel, 4);
			break;
		case PWM3_CMD_FIRMWARE_VERSION:
			break;
		case PWM3_CMD_WR1:
			wr_data[2] = myData->pwm3_ch.ch[ch].o_val[0];
			wr_data[3] = myData->pwm3_ch.ch[ch].o_val[1];
			break;
		case PWM3_CMD_DA_VALUE3:
			memcpy((char *)&wr_data[2],
				(char *)&myData->pwm3_ch.ch[ch].da_val[0], 4);
			memcpy((char *)&wr_data[6],
				(char *)&myData->pwm3_ch.ch[ch].da_val[1], 4);
			break;
		case PWM3_CMD_DA_VALUE4:
			memcpy((char *)&wr_data[2],
				(char *)&myData->pwm3_ch.ch[ch].da_val[2], 4);
			memcpy((char *)&wr_data[6],
				(char *)&myData->pwm3_ch.ch[ch].da_val[3], 4);
			break;
		case PWM3_CMD_ETC1:
			break;
		case PWM3_CMD_ETC2:
			break;
		case PWM3_CMD_TEST_COND1:
			memcpy((char *)&wr_data[2], (char *)&myCh->misc.cmd_v[0], 4);
			memcpy((char *)&wr_data[6], (char *)&myCh->misc.cmd_v[1], 4);
			break;
		case PWM3_CMD_TEST_COND2:
			memcpy((char *)&wr_data[2], (char *)&myCh->misc.cmd_i[0], 4);
			memcpy((char *)&wr_data[6], (char *)&myCh->misc.cmd_i[1], 4);
			break;
		case PWM3_CMD_TEST_COND3:
			memcpy((char *)&wr_data[2], (char *)&myCh->misc.cmd_p[0], 4);
			memcpy((char *)&wr_data[6], (char *)&myCh->misc.cmd_p[1], 4);
			break;
		case PWM3_CMD_TEST_COND4:
			memcpy((char *)&wr_data[2], (char *)&myCh->misc.cmd_t[0], 4);
			memcpy((char *)&wr_data[6], (char *)&myCh->misc.cmd_t[1], 4);
			break;
		case PWM3_CMD_TEST_COND5:
			wr_data[2] = myCh->signal[C_SIG_OUT_SWITCH];
			wr_data[3] = myCh->op.state;
			wr_data[4] = myCh->op.rangeI;
			wr_data[5] = myCh->op.stepType;
			wr_data[6] = myCh->op.stepMode;
			wr_data[7] = myCh->misc.waveform_type;
			wr_data[8] = myCh->signal[C_SIG_CABLE_CHECK];
			break;
		case PWM3_CMD_CALI_UPDATE:
			wr_data[2] = myData->pwm3_ch.ch[ch].cali_type;
			wr_data[3] = myData->pwm3_ch.ch[ch].cali_range;
			wr_data[4] = myData->pwm3_ch.ch[ch].cali_mode;
			wr_data[5] = myData->pwm3_ch.ch[ch].cali_index;
			memcpy((char *)&wr_data[6],
				(char *)&myData->pwm3_ch.ch[ch].cali_data, 4);
			break;
		default:
			break;
	}

	switch(myData->AppControl.config.sbcType) {
		case SBC_WAFER_MARK533:
		case SBC_WAFER_MARK800:
			max_count = 0;
			break;
		case SBC_EM104_A5362:
			max_count = 240;
			break;
		default:
			max_count = 0;
			break;
	}

	outb((unsigned char)ch, 0x75F);
	for(j=0; j < max_count; j++) {}
	for(i=0; i < 10; i++) {
		outb(wr_data[i], 0x750 + i);
		for(j=0; j < max_count; j++) {}
	}
}

void PWM3_Read(int ch, int cmd)
{
	unsigned char rd_data[10];
	int i, j, max_count;
	long tmpV, tempV;//, min, max;
	double tmp_val;

	myCh = &(myData->cData[ch]);

	switch(myData->AppControl.config.sbcType) {
		case SBC_WAFER_MARK533:
		case SBC_WAFER_MARK800:
			max_count = 0;
			break;
		case SBC_EM104_A5362:
			//max_count = 120;
			max_count = 200;
			break;
		default:
			max_count = 0;
			break;
	}

	outb((unsigned char)ch, 0x75E);
	for(j=0; j < max_count; j++) {}
	for(i=0; i < 10; i++) {
		rd_data[i] = inb(0x750 + i);
		for(j=0; j < max_count; j++) {}
	}

	if(cmd == 0) {
		cmd = (int)rd_data[0];
		cmd = cmd << 8;
		cmd |= (int)rd_data[1];
	}

	switch(cmd) {
		case PWM3_CMD_SYSTEM_MODEL_REPLY:
			break;
		case PWM3_CMD_FIRMWARE_VERSION_REPLY:
			memcpy((char *)&myData->pwm3_ch.ch[ch].firmware_version[0],
				(char *)&rd_data[2], 8);
			break;
		case PWM3_CMD_RD1_REPLY:
			myData->pwm3_ch.ch[ch].i_val[0] = rd_data[2];
			myData->pwm3_ch.ch[ch].i_val[1] = rd_data[3];
			break;
		case PWM3_CMD_AD_VALUE1_REPLY:
			memcpy((char *)&myData->pwm3_ch.ch[ch].ad_val[0],
				(char *)&rd_data[2], 4);
			memcpy((char *)&myData->pwm3_ch.ch[ch].ad_val[1],
				(char *)&rd_data[6], 4);
			break;
		case PWM3_CMD_AD_VALUE2_REPLY:
			memcpy((char *)&myData->pwm3_ch.ch[ch].ad_val[2],
				(char *)&rd_data[2], 4);
			memcpy((char *)&myData->pwm3_ch.ch[ch].ad_val[3],
				(char *)&rd_data[6], 4);
			break;
		case PWM3_CMD_AD_VALUE3_REPLY:
			memcpy((char *)&myData->pwm3_ch.ch[ch].ad_val[4],
				(char *)&rd_data[2], 4);
			memcpy((char *)&myData->pwm3_ch.ch[ch].ad_val[5],
				(char *)&rd_data[6], 4);

			tmpV = myData->pwm3_ch.ch[ch].ad_val[4];
			i = myCh->misc.sensCount;
			myCh->misc.sensSumV[i] = tmpV;
			//min = tmpV;
			//max = tmpV;
			i++;
			if(i >= 4) i = 0;
			myCh->misc.sensCount = (unsigned char)i;
			tmp_val = (double)myCh->misc.sensSumV[0];
			for(i=1; i < 4; i++) {
				tmp_val += (double)myCh->misc.sensSumV[i];
				//if(myCh->misc.sensSumV[i] < min) min = myCh->misc.sensSumV[i];
				//if(myCh->misc.sensSumV[i] > max) max = myCh->misc.sensSumV[i];
			}
			//tmp_val = (tmp_val - min - max) / 2.0;
			tmp_val /= 4.0;
			tempV = (long)tmp_val;

			myCh->op.Vsens = tempV;
			myCh->misc.tmpVsens = tmpV;
			myCh->op.Isens = myData->pwm3_ch.ch[ch].ad_val[5];
			myCh->misc.tmpIsens = myData->pwm3_ch.ch[ch].ad_val[5];
			if(myCh->op.state == C_CALI) {
				myData->cali[ch].orgAD[0] = tempV;
				myData->cali[ch].orgAD[1] = myCh->op.Isens;
			}
			break;
		case PWM3_CMD_AD_VALUE4_REPLY:
			memcpy((char *)&myData->pwm3_ch.ch[ch].ad_val[6],
				(char *)&rd_data[2], 4);
			memcpy((char *)&myData->pwm3_ch.ch[ch].ad_val[7],
				(char *)&rd_data[6], 4);
			break;
		case PWM3_CMD_AD_VALUE5_REPLY:
			memcpy((char *)&myData->pwm3_ch.ch[ch].ad_val[8],
				(char *)&rd_data[2], 4);
			memcpy((char *)&myData->pwm3_ch.ch[ch].ad_val[9],
				(char *)&rd_data[6], 4);
			break;
		case PWM3_CMD_AH_WH_REPLY:
			memcpy((char *)&myData->pwm3_ch.ch[ch].ah, (char *)&rd_data[2], 4);
			memcpy((char *)&myData->pwm3_ch.ch[ch].wh, (char *)&rd_data[6], 4);
			break;
		case PWM3_CMD_AVG_V_I_REPLY:
			memcpy((char *)&myData->pwm3_ch.ch[ch].avg_v,
				(char *)&rd_data[2], 4);
			memcpy((char *)&myData->pwm3_ch.ch[ch].avg_i,
				(char *)&rd_data[6], 4);
			break;
		case PWM3_CMD_MAX_V_I_REPLY:
			memcpy((char *)&myData->pwm3_ch.ch[ch].max_v,
				(char *)&rd_data[2], 4);
			memcpy((char *)&myData->pwm3_ch.ch[ch].max_i,
				(char *)&rd_data[6], 4);
			break;
		case PWM3_CMD_MIN_V_I_REPLY:
			memcpy((char *)&myData->pwm3_ch.ch[ch].min_v,
				(char *)&rd_data[2], 4);
			memcpy((char *)&myData->pwm3_ch.ch[ch].min_i,
				(char *)&rd_data[6], 4);
			break;
		case PWM3_CMD_ETC1_REPLY:
			break;
		case PWM3_CMD_TEST_COND3_REPLY:
			memcpy((char *)&myData->pwm3_ch.ch[ch].v_power_bus,
				(char *)&rd_data[2], 4);
			memcpy((char *)&myData->pwm3_ch.ch[ch].v_output_bus,
				(char *)&rd_data[6], 4);
			break;
		case PWM3_CMD_TEST_COND5_REPLY:
			if(myCh->signal[C_SIG_OUT_SWITCH] == P1) {
				if(rd_data[2] == P2) myCh->signal[C_SIG_OUT_SWITCH] = P2;
			} else if(myCh->signal[C_SIG_OUT_SWITCH] == P11) {
				if(rd_data[2] == P12) myCh->signal[C_SIG_OUT_SWITCH] = P12;
			} else if(myCh->signal[C_SIG_OUT_SWITCH] == P51) {
				if(rd_data[2] == P52) myCh->signal[C_SIG_OUT_SWITCH] = P52;
			} else if(myCh->signal[C_SIG_OUT_SWITCH] == P61) {
				if(rd_data[2] == P62) myCh->signal[C_SIG_OUT_SWITCH] = P62;
			} else if(myCh->signal[C_SIG_OUT_SWITCH] == P71) {
				if(rd_data[2] == P72) myCh->signal[C_SIG_OUT_SWITCH] = P72;
			} else if(myCh->signal[C_SIG_OUT_SWITCH] == P81) {
				if(rd_data[2] == P82) myCh->signal[C_SIG_OUT_SWITCH] = P82;
			} else if(myCh->signal[C_SIG_OUT_SWITCH] == P85) {
				if(rd_data[2] == P86) myCh->signal[C_SIG_OUT_SWITCH] = P86;
			} else if(myCh->signal[C_SIG_OUT_SWITCH] == P91) {
				if(rd_data[2] == P92) myCh->signal[C_SIG_OUT_SWITCH] = P92;
			}
			break;
		default:
			break;
	}

	if(myData->AppControl.config.debugType == 1) {
		if(myCh->signal[C_SIG_OUT_SWITCH] == P1) {
			myCh->signal[C_SIG_OUT_SWITCH] = P2;
		} else if(myCh->signal[C_SIG_OUT_SWITCH] == P11) {
			myCh->signal[C_SIG_OUT_SWITCH] = P12;
		} else if(myCh->signal[C_SIG_OUT_SWITCH] == P51) {
			myCh->signal[C_SIG_OUT_SWITCH] = P52;
		} else if(myCh->signal[C_SIG_OUT_SWITCH] == P61) {
			myCh->signal[C_SIG_OUT_SWITCH] = P62;
		} else if(myCh->signal[C_SIG_OUT_SWITCH] == P71) {
			myCh->signal[C_SIG_OUT_SWITCH] = P72;
		} else if(myCh->signal[C_SIG_OUT_SWITCH] == P81) {
			myCh->signal[C_SIG_OUT_SWITCH] = P82;
		} else if(myCh->signal[C_SIG_OUT_SWITCH] == P85) {
			myCh->signal[C_SIG_OUT_SWITCH] = P86;
		} else if(myCh->signal[C_SIG_OUT_SWITCH] == P91) {
			myCh->signal[C_SIG_OUT_SWITCH] = P92;
		}
	}
}

void PWM3_send_cali_data(int ch)
{
	unsigned char phase, type, range, mode;
	float value;

	phase = myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE];
	mode = myData->cData[ch].signal[C_SIG_CALI_UPDATE];

	switch(mode) {
		case P1:
			type = 0; //voltage
			range = 0;
			if(phase == P0) {
				myData->pwm3_ch.ch[ch].cali_type = type;
				myData->pwm3_ch.ch[ch].cali_range = range;
				myData->pwm3_ch.ch[ch].cali_mode = mode; //setPointNum
				myData->pwm3_ch.ch[ch].cali_index = phase;
				myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
					.data[type][range].point.setPointNum;
			} else {
				myData->pwm3_ch.ch[ch].cali_type = type;
				myData->pwm3_ch.ch[ch].cali_range = range;
				myData->pwm3_ch.ch[ch].cali_mode = mode; //set_ad
				myData->pwm3_ch.ch[ch].cali_index = phase;
				myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
					.data[type][range].set_ad[phase - 1];
			}
			phase++;
			if(phase == P16) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P2:
			type = 0; //voltage
			range = 0;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //AD_A
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].AD_A[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P3:
			type = 0; //voltage
			range = 0;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //AD_B
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].AD_B[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P4:
			type = 0; //voltage
			range = 0;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //setPoint
			myData->pwm3_ch.ch[ch].cali_index = phase;
			myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
				.data[type][range].point.setPoint[phase];
			phase++;
			if(phase == P15) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P5:
			type = 0; //voltage
			range = 0;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //DA_A
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].DA_A[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P6:
			type = 0; //voltage
			range = 0;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //DA_B
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].DA_B[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode = P11;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P11:
			type = 0; //voltage
			range = 1;
			if(phase == P0) {
				myData->pwm3_ch.ch[ch].cali_type = type;
				myData->pwm3_ch.ch[ch].cali_range = range;
				myData->pwm3_ch.ch[ch].cali_mode = mode; //setPointNum
				myData->pwm3_ch.ch[ch].cali_index = phase;
				myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
					.data[type][range-1].point.setPointNum;
			} else {
				myData->pwm3_ch.ch[ch].cali_type = type;
				myData->pwm3_ch.ch[ch].cali_range = range;
				myData->pwm3_ch.ch[ch].cali_mode = mode; //set_ad
				myData->pwm3_ch.ch[ch].cali_index = phase;
				myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
					.data[type][range-1].set_ad[phase - 1];
			}
			phase++;
			if(phase == P16) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P12:
			type = 0; //voltage
			range = 1;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //AD_A
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range-1].AD_A[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P13:
			type = 0; //voltage
			range = 1;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //AD_B
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range-1].AD_B[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P14:
			type = 0; //voltage
			range = 1;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //setPoint
			myData->pwm3_ch.ch[ch].cali_index = phase;
			myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
				.data[type][range-1].point.setPoint[phase];
			phase++;
			if(phase == P15) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P15:
			type = 0; //voltage
			range = 1;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //DA_A
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].DA_A[phase];
			//value = (float)myData->cali[ch].data[type][range-1].DA_A[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P16:
			type = 0; //voltage
			range = 1;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //DA_B
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].DA_B[phase];
			//value = (float)myData->cali[ch].data[type][range-1].DA_B[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode = P21;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P21:
			type = 1; //current
			range = 0;
			if(phase == P0) {
				myData->pwm3_ch.ch[ch].cali_type = type;
				myData->pwm3_ch.ch[ch].cali_range = range;
				myData->pwm3_ch.ch[ch].cali_mode = mode; //setPointNum
				myData->pwm3_ch.ch[ch].cali_index = phase;
				myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
					.data[type][range].point.setPointNum;
			} else {
				myData->pwm3_ch.ch[ch].cali_type = type;
				myData->pwm3_ch.ch[ch].cali_range = range;
				myData->pwm3_ch.ch[ch].cali_mode = mode; //set_ad
				myData->pwm3_ch.ch[ch].cali_index = phase;
				myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
					.data[type][range].set_ad[phase - 1];
			}
			phase++;
			if(phase == P16) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P22:
			type = 1; //current
			range = 0;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //AD_A
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].AD_A[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P23:
			type = 1; //current
			range = 0;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //AD_B
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].AD_B[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P24:
			type = 1; //current
			range = 0;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //setPoint
			myData->pwm3_ch.ch[ch].cali_index = phase;
			myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
				.data[type][range].point.setPoint[phase];
			phase++;
			if(phase == P15) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P25:
			type = 1; //current
			range = 0;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //DA_A
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].DA_A[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P26:
			type = 1; //current
			range = 0;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //DA_B
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].DA_B[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode = P31;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P31:
			type = 1; //current
			range = 1;
			if(phase == P0) {
				myData->pwm3_ch.ch[ch].cali_type = type;
				myData->pwm3_ch.ch[ch].cali_range = range;
				myData->pwm3_ch.ch[ch].cali_mode = mode; //setPointNum
				myData->pwm3_ch.ch[ch].cali_index = phase;
				myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
					.data[type][range].point.setPointNum;
			} else {
				myData->pwm3_ch.ch[ch].cali_type = type;
				myData->pwm3_ch.ch[ch].cali_range = range;
				myData->pwm3_ch.ch[ch].cali_mode = mode; //set_ad
				myData->pwm3_ch.ch[ch].cali_index = phase;
				myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
					.data[type][range].set_ad[phase - 1];
			}
			phase++;
			if(phase == P16) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P32:
			type = 1; //current
			range = 1;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //AD_A
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].AD_A[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P33:
			type = 1; //current
			range = 1;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //AD_B
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].AD_B[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P34:
			type = 1; //current
			range = 1;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //setPoint
			myData->pwm3_ch.ch[ch].cali_index = phase;
			myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
				.data[type][range].point.setPoint[phase];
			phase++;
			if(phase == P15) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P35:
			type = 1; //current
			range = 1;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //DA_A
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].DA_A[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P36:
			type = 1; //current
			range = 1;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //DA_B
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].DA_B[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode = P41;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P41:
			type = 1; //current
			range = 2;
			if(phase == P0) {
				myData->pwm3_ch.ch[ch].cali_type = type;
				myData->pwm3_ch.ch[ch].cali_range = range;
				myData->pwm3_ch.ch[ch].cali_mode = mode; //setPointNum
				myData->pwm3_ch.ch[ch].cali_index = phase;
				myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
					.data[type][range].point.setPointNum;
			} else {
				myData->pwm3_ch.ch[ch].cali_type = type;
				myData->pwm3_ch.ch[ch].cali_range = range;
				myData->pwm3_ch.ch[ch].cali_mode = mode; //set_ad
				myData->pwm3_ch.ch[ch].cali_index = phase;
				myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
					.data[type][range].set_ad[phase - 1];
			}
			phase++;
			if(phase == P16) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P42:
			type = 1; //current
			range = 2;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //AD_A
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].AD_A[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P43:
			type = 1; //current
			range = 2;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //AD_B
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].AD_B[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P44:
			type = 1; //current
			range = 2;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //setPoint
			myData->pwm3_ch.ch[ch].cali_index = phase;
			myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
				.data[type][range].point.setPoint[phase];
			phase++;
			if(phase == P15) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P45:
			type = 1; //current
			range = 2;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //DA_A
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].DA_A[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P46:
			type = 1; //current
			range = 2;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //DA_B
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].DA_B[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode = P51;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P51:
			type = 1; //current
			range = 3;
			if(phase == P0) {
				myData->pwm3_ch.ch[ch].cali_type = type;
				myData->pwm3_ch.ch[ch].cali_range = range;
				myData->pwm3_ch.ch[ch].cali_mode = mode; //setPointNum
				myData->pwm3_ch.ch[ch].cali_index = phase;
				myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
					.data[type][range].point.setPointNum;
			} else {
				myData->pwm3_ch.ch[ch].cali_type = type;
				myData->pwm3_ch.ch[ch].cali_range = range;
				myData->pwm3_ch.ch[ch].cali_mode = mode; //set_ad
				myData->pwm3_ch.ch[ch].cali_index = phase;
				myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
					.data[type][range].set_ad[phase - 1];
			}
			phase++;
			if(phase == P16) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P52:
			type = 1; //current
			range = 3;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //AD_A
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].AD_A[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P53:
			type = 1; //current
			range = 3;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //AD_B
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].AD_B[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P54:
			type = 1; //current
			range = 3;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //setPoint
			myData->pwm3_ch.ch[ch].cali_index = phase;
			myData->pwm3_ch.ch[ch].cali_data = (long)myData->cali[ch]
				.data[type][range].point.setPoint[phase];
			phase++;
			if(phase == P15) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P55:
			type = 1; //current
			range = 3;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //DA_A
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].DA_A[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode++;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		case P56:
			type = 1; //current
			range = 3;
			myData->pwm3_ch.ch[ch].cali_type = type;
			myData->pwm3_ch.ch[ch].cali_range = range;
			myData->pwm3_ch.ch[ch].cali_mode = mode; //DA_B
			myData->pwm3_ch.ch[ch].cali_index = phase;
			value = (float)myData->cali[ch].data[type][range].DA_B[phase];
			memcpy((char *)&myData->pwm3_ch.ch[ch].cali_data,
				(char *)&value, 4);
			phase++;
			if(phase == P14) {
				phase = P0;
				mode = P61;
				myData->cData[ch].signal[C_SIG_CALI_UPDATE] = mode;
			}
			myData->cData[ch].signal[C_SIG_CALI_UPDATE_PHASE] = phase;
			break;
		default:
			break;
	}
}

void PWM3_CalChAverage(int ch)
{
	long tmpI;
    double tempV, tempI, acc_time;

	myCh = &(myData->cData[ch]);

	if(myCh->op.state != C_RUN) return;
	if(myCh->op.phase != P50) return;

	/*kjg_d myCh->op.Vsens = myCh->op.runTime * 10000;
	myCh->misc.tmpVsens = myCh->op.runTime * 10000;
	myCh->op.Isens = myCh->op.runTime * 10000;
	myCh->misc.tmpIsens = myCh->op.runTime * 10000;
*/
	acc_time = 3600.0 * (1000.0 / myPs->config.scan_period);

	//cal ampareHour
	//uA -> capacity 1uAh/div
	//nA -> capacity 1nAh/div
//	myCh->op.charge_AmpareHour = myData->pwm3_ch.ch[ch].ah;

	tmpI = myCh->op.Isens;
	if(tmpI >= 0) {
		if(myCh->op.runTime == 4) tmpI *= 2;
		myCh->misc.sum_charge_AmpareHour += (double)tmpI;
		myCh->op.charge_AmpareHour
			= (long)(myCh->misc.seed_charge_AmpareHour
			+ myCh->misc.sum_charge_AmpareHour / acc_time);
	   	if(myCh->misc.sum_charge_AmpareHour > MAX_SUM_DOUBLE) {
			myCh->misc.seed_charge_AmpareHour
				= (double)myCh->op.charge_AmpareHour;
			myCh->misc.sum_charge_AmpareHour = 0.0;
		}
	} else {
		if(myCh->op.runTime == 4) tmpI *= 2;
		myCh->misc.sum_discharge_AmpareHour += (double)tmpI;
		myCh->op.discharge_AmpareHour
			= (long)(myCh->misc.seed_discharge_AmpareHour
			+ myCh->misc.sum_discharge_AmpareHour / acc_time);
	   	if(myCh->misc.sum_discharge_AmpareHour > MAX_SUM_DOUBLE) {
			myCh->misc.seed_discharge_AmpareHour
				= (double)myCh->op.discharge_AmpareHour;
			myCh->misc.sum_discharge_AmpareHour = 0.0;
		}
	}

	//cal tmpWatt
	//uA -> watt 1mW/div
	//nA -> watt 1uW/div
   	tempI = ((double)myCh->misc.tmpVsens / 1000.0)
		* ((double)myCh->misc.tmpIsens / 1000.0);
	myCh->misc.tmpWatt = (long)(tempI / 1000.0);

	//cal watt
	//uA -> watt 1mW/div
	//nA -> watt 1uW/div
   	tempI = ((double)myCh->op.Vsens / 1000.0)
		* ((double)myCh->op.Isens / 1000.0);
	myCh->op.watt = (long)(tempI / 1000.0);

	//cal wattHour
	//uA -> watt 1mWh/div
	//nA -> watt 1uWh/div
//	myCh->op.charge_WattHour = myData->pwm3_ch.ch[ch].wh;

	if(tmpI >= 0) {
		if(myCh->op.runTime == 4) (double)tempI *= 2.1;
		myCh->misc.sum_charge_WattHour += ((double)tempI / 1000.0);
		myCh->op.charge_WattHour
			= (long)(myCh->misc.seed_charge_WattHour
			+ myCh->misc.sum_charge_WattHour / acc_time);
	   	if(myCh->misc.sum_charge_WattHour > MAX_SUM_DOUBLE) {
			myCh->misc.seed_charge_WattHour = (double)myCh->op.charge_WattHour;
			myCh->misc.sum_charge_WattHour = 0.0;
		}
	} else {
		if(myCh->op.runTime == 4) (double)tempI *= 2.1;
		myCh->misc.sum_discharge_WattHour += ((double)tempI / 1000.0);
		myCh->op.discharge_WattHour
			= (long)(myCh->misc.seed_discharge_WattHour
			+ myCh->misc.sum_discharge_WattHour / acc_time);
	   	if(myCh->misc.sum_discharge_WattHour > MAX_SUM_DOUBLE) {
			myCh->misc.seed_discharge_WattHour
   				= (double)myCh->op.discharge_WattHour;
			myCh->misc.sum_discharge_WattHour = 0.0;
		}
	}

	//cal meanV, meanI
	tempV = (double)myCh->op.meanV * (double)myCh->misc.meanSumCount;
	tempV += (double)myCh->misc.tmpVsens;
	tempI = (double)myCh->op.meanI * (double)myCh->misc.meanSumCount;
	tempI += (double)myCh->misc.tmpIsens;

	myCh->misc.meanSumCount++;
	tempV /= (double)myCh->misc.meanSumCount;
	myCh->op.meanV = (long)tempV;
	tempI /= (double)myCh->misc.meanSumCount;
	myCh->op.meanI = (long)tempI;
}

