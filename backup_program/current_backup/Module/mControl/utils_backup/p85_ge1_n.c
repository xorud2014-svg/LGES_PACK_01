
void cOutSwitch_P85_General1(int ch, int slot)
{
	unsigned char idxStepNo;
	int v_div, i_div, rangeV, rangeI, group=0, idx, pattern_count, i, sysModel;
	long val1, val1_max, val1_0, val1_min, val2, val2_max, val2_0, val2_min;
	long cmd_val, attr_count;
	double tmp1;

	switch(myData->AppControl.config.systemModel) {
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
			sysModel = 1;
			break;
		case C_SDI_70V_250A_25A_18KW:
			sysModel = 2;
			break;
		default:
			sysModel = 0;
			break;
	}

	idxStepNo = myCh->op.idxStepNo;
	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = myTestCond->common_object[idx];

	v_div = (int)myCh->misc.cmd_v_div;
	i_div = (int)myCh->misc.cmd_i_div;
	rangeV = myCh->op.rangeV;
	rangeI = myCh->op.rangeI;

	pattern_count = (int)myCh->misc.pattern_count;
	if(pattern_count < 1) pattern_count = 1;
	cmd_val = myTestCond->pattern[pattern_count].cmd_val[0];

	idx = IDX_LOC_OBJ_REF_V;
	myCh->misc.cmd_v[0] = (int)myTestCond->local_object[idxStepNo][idx];
	idx = IDX_LOC_OBJ_REF_V2;
	myCh->misc.cmd_v[1] = (int)myTestCond->local_object[idxStepNo][idx];
	if(cmd_val == 0) {
		val1 = 0;
	} else if(cmd_val > 0) {
		val1 = myCh->misc.cmd_v[0];
	} else {
		val1 = myCh->misc.cmd_v[1];
	}
	val1_max = myPs->config.maxV[rangeV];
	val1_0 = 0;
	val1_min = val1_max * (-1);

	val2_max = myPs->config.maxI[rangeI];
	val2_0 = 0;
	val2_min = myPs->config.minI[rangeI];
	if(myCh->op.stepMode == MODE_CC) {
		val2 = cmd_val / attr_count;
		myCh->misc.cmd_i[0] = val2;
	} else {
		val2 = cmd_val / attr_count;
		if(cmd_val >= 0) {
			tmp1 = (double)(val1_max / 1000) * (double)(val2_max / 1000)
				/ 1000.0;
			if(val2 > (long)tmp1) val2 = (long)tmp1;
			else if(val2 < 0) val2 = 0;
		} else {
			tmp1 = (double)(val1_max / 1000) * (double)(val2_min / 1000)
				/ 1000.0;
			if(val2 < (long)tmp1) val2 = (long)tmp1;
			else if(val2 > 0) val2 = 0;
		}
		myCh->misc.cmd_p[0] = val2;

		tmp1 = (double)myCh->misc.cmd_p[0] / (double)myCh->misc.tmpVsens;
		tmp1 *= 1000000000.0;
		val2 = (long)tmp1;
		if(cmd_val >= 0) {
			if(val2 > val2_max) val2 = val2_max;
			else if(val2 < 0) val2 = 0;
		} else {
			if(val2 < val2_min) val2 = val2_min;
			else if(val2 > 0) val2 = 0;
		}
		myCh->misc.cmd_i[0] = val2;
	}

	if(myCh->signal[C_SIG_SELECTED_RANGE_I] == RANGE0) {
		switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
			case P0:
				if(slot != 0) break;
				cCalCmdV(ch, val1_max, v_div, rangeV);
				cCalCmdI(ch, val2_min, i_div, rangeI);
				myCh->signal[C_SIG_SEMI_SWITCH] = P10;
				break;
			case P10:
				if(slot != 0) break;
				if(myData->ChAttribute[ch].chNo_master != 0) { //for master
					cOutSwitch_Select_ON_1(ch, rangeV, rangeI);
				}
				if(val2 == 0) {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, OFF);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, OFF);
				} else if(val2 > 0) {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, ON);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, OFF);
				} else {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, OFF);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, ON);
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			default:
				if(slot != 0) break;
				i = myCh->signal[C_SIG_SEMI_SWITCH] - P10;
				i *= (int)myPs->config.scan_period;
				if(i >= 200) {
					//200ms wait for Relay Active
					if(val2 == 0) {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_P;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
					} else if(val2 > 0) {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_P;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
					} else {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_N;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
					}
					myCh->signal[C_SIG_OUT_SWITCH] = P86;
					myCh->signal[C_SIG_SEMI_SWITCH] = P0;
				} else {
					myCh->signal[C_SIG_SEMI_SWITCH]++;
				}
				break;
		}
	} else if(myCh->signal[C_SIG_SELECTED_RANGE_I] == RANGE1) {
		switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
			case P0:
				if(slot != 0) break;
				if(myCh->misc.semiSwitchState == SEMI_I_P) {
					if(val2 == 0) {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						switch(sysModel) {
							case 0:
								cCalCmdI(ch, val2_max, i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_0, i_div, rangeI);
								break;
						}
					} else if(val2 > 0) {
						switch(sysModel) {
							case 0:
								cCalCmdV(ch, val1, v_div, rangeV);
								break;
							default:
								cCalCmdV(ch, val1_min, v_div, rangeV);
								cCalCmdI(ch, (long)((float)val2_min * 1.5),
									i_div, rangeI);
								break;
						}
					} else {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						switch(sysModel) {
							case 0:
								cCalCmdI(ch, val2_max, i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_0, i_div, rangeI);
								break;
						}
					}
				} else { //SEMI_I_N
					if(val2 == 0) {
						cCalCmdV(ch, val1_max, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					} else if(val2 > 0) {
						cCalCmdV(ch, val1_max, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					} else {
						switch(sysModel) {
							case 0:
								cCalCmdV(ch, val1, v_div, rangeV);
								break;
							default:
								cCalCmdV(ch, val1_0, v_div, rangeV);
								cCalCmdI(ch, val2_max, i_div, rangeI);
								break;
						}
					}
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P1:
				if(myCh->misc.semiSwitchState == SEMI_I_P) {
					if(val2 == 0) {
						switch(sysModel) {
							case 0:
								break;
							default:
								cCalCmdV(ch, val1_min, v_div, rangeV);
								cCalCmdI(ch, val2_max, i_div, rangeI);
								break;
						}
					} else if(val2 > 0) {
						switch(sysModel) {
							case 0:
								break;
							default:
								cCalCmdV(ch, val1_min, v_div, rangeV);
								break;
						}
						switch(sysModel) {
							case 0:
								cCalCmdI(ch, val2, i_div, rangeI);
								break;
							case 1:
								cCalCmdI(ch, (long)((float)val2_min * 1.5),
									i_div, rangeI);
								break;
							case 2:
								cCalCmdI(ch, (long)((float)val2_min * 1.8),
									i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_min, i_div, rangeI);
								break;
						}
					} else {
						switch(sysModel) {
							case 0:
								cCalCmdV(ch, val1_0, v_div, rangeV);
								cCalCmdI(ch, val2_max, i_div, rangeI);
								break;
							default:
								cCalCmdV(ch, val1, v_div, rangeV);
								cCalCmdI(ch, val2, i_div, rangeI);
								break;
						}
					}
				} else { //SEMI_I_N
					if(val2 == 0) {
						cCalCmdV(ch, val1_max, v_div, rangeV);
						switch(sysModel) {
							case 0:
								cCalCmdI(ch, val2_min, i_div, rangeI);
								break;
							case 2:
								cCalCmdI(ch, (long)((float)val2_max * 0.5),
									i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_0, i_div, rangeI);
								break;
						}
						myCh->signal[C_SIG_SEMI_SWITCH]++;
					} else if(val2 > 0) {
						switch(sysModel) {
							case 0:
								cCalCmdV(ch, val1_max, v_div, rangeV);
								cCalCmdI(ch, val2_min, i_div, rangeI);
								break;
							default:
								cCalCmdV(ch, val1, v_div, rangeV);
								cCalCmdI(ch, val2, i_div, rangeI);
								break;
						}
					} else {
						switch(sysModel) {
							case 0:
								cCalCmdI(ch, val2, i_div, rangeI);
								break;
							default:
								break;
						}
					}
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P2:
				if(myCh->misc.semiSwitchState == SEMI_I_P) {
					if(val2 == 0) {
					} else if(val2 > 0) {
						cCalCmdV(ch, val1, v_div, rangeV);
						switch(sysModel) {
							case 1:
								cCalCmdI(ch, (long)((float)val2_min * 0.9),
									i_div, rangeI);
								break;
							case 2:
								cCalCmdI(ch, (long)((float)val2_min * 1.9),
									i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_min, i_div, rangeI);
								break;
						}
					} else {
						switch(sysModel) {
							case 0:
								cCalCmdV(ch, val1_0, v_div, rangeV);
								cCalCmdI(ch, val2_max, i_div, rangeI);
								break;
							default:
								cCalCmdV(ch, val1, v_div, rangeV);
								cCalCmdI(ch, val2, i_div, rangeI);
								break;
						}
					}
				} else { //SEMI_I_N
					if(val2 == 0) {
						switch(sysModel) {
							case 0:
								break;
							case 2:
								cCalCmdV(ch, val1_min, v_div, rangeV);
								cCalCmdI(ch, val2_max, i_div, rangeI);
								break;
							default:
								cCalCmdV(ch, val1_0, v_div, rangeV);
								cCalCmdI(ch, val2_max, i_div, rangeI);
								break;
						}
					} else if(val2 > 0) {
						switch(sysModel) {
							case 0:
								break;
							default:
								cCalCmdV(ch, val1, v_div, rangeV);
								cCalCmdI(ch, val2, i_div, rangeI);
								break;
						}
					} else {
					}
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P3:
				if(val2 == 0) {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, OFF);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, OFF);
					switch(sysModel) {
						case 0:
							break;
						default:
							cCalCmdV(ch, val1_min, v_div, rangeV);
							cCalCmdI(ch, val2_max, i_div, rangeI);
							break;
					}
					myCh->misc.semiSwitchState = SEMI_I_P;
					myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
				} else if(val2 > 0) {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, ON);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, OFF);
					cCalCmdV(ch, val1, v_div, rangeV);
					cCalCmdI(ch, val2, i_div, rangeI);
					myCh->misc.semiSwitchState = SEMI_I_P;
					myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
				} else {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, OFF);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, ON);
					cCalCmdV(ch, val1, v_div, rangeV);
					cCalCmdI(ch, val2, i_div, rangeI);
					myCh->misc.semiSwitchState = SEMI_I_N;
					myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
				}
				myCh->signal[C_SIG_OUT_SWITCH] = P86;
				myCh->signal[C_SIG_SEMI_SWITCH] = P0;
				break;
		}
	} else if(myCh->signal[C_SIG_SELECTED_RANGE_I] == RANGE2) {
		switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
			case P0:
				if(slot != 0) break;
				if(myCh->misc.semiSwitchState == SEMI_I_P) {
					if(val2 == 0) {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					} else if(val2 > 0) {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						switch(sysModel) {
							case 1:
								cCalCmdI(ch, (long)((float)val2_min * 1.5),
									i_div, rangeI);
								break;
							case 2:
								cCalCmdI(ch, (long)((float)val2_min * 1.9),
									i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_min, i_div, rangeI);
								break;
						}
					} else {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					}
				} else { //SEMI_I_N
					if(val2 == 0) {
						cCalCmdV(ch, val1_max, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					} else if(val2 > 0) {
						cCalCmdV(ch, val1_max, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					} else {
						cCalCmdV(ch, val1_0, v_div, rangeV);
						cCalCmdI(ch, val2_max, i_div, rangeI);
					}
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P1:
				if(myCh->misc.semiSwitchState == SEMI_I_P) {
					if(val2 == 0) {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						cCalCmdI(ch, val2_max, i_div, rangeI);
					} else if(val2 > 0) {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						switch(sysModel) {
							case 1:
								cCalCmdI(ch, (long)((float)val2_min * 1.5),
									i_div, rangeI);
								break;
							case 2:
								cCalCmdI(ch, (long)((float)val2_min * 1.2),
									i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_min, i_div, rangeI);
								break;
						}
					} else {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
					}
				} else { //SEMI_I_N
					if(val2 == 0) {
						cCalCmdV(ch, val1_max, v_div, rangeV);
						switch(sysModel) {
							case 2:
								cCalCmdI(ch, (long)((float)val2_max * 0.5),
									i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_0, i_div, rangeI);
								break;
						}
						myCh->signal[C_SIG_SEMI_SWITCH]++;
					} else if(val2 > 0) {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
					} else {
					}
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P2:
				if(myCh->misc.semiSwitchState == SEMI_I_P) {
					if(val2 == 0) {
					} else if(val2 > 0) {
						cCalCmdV(ch, val1, v_div, rangeV);
						switch(sysModel) {
							case 1:
								cCalCmdI(ch, (long)((float)val2_min * 0.2),
									i_div, rangeI);
								break;
							case 2:
								cCalCmdI(ch, (long)((float)val2_min * 1.9),
									i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_0, i_div, rangeI);
								break;
						}
					} else {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
					}
				} else { //SEMI_I_N
					if(val2 == 0) {
						switch(sysModel) {
							case 2:
								cCalCmdV(ch, val1_min, v_div, rangeV);
								break;
							default:
								cCalCmdV(ch, val1_0, v_div, rangeV);
								break;
						}
						cCalCmdI(ch, val2_max, i_div, rangeI);
					} else if(val2 > 0) {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
					} else {
					}
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P3:
				if(val2 == 0) {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, OFF);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, OFF);
					cCalCmdV(ch, val1_min, v_div, rangeV);
					cCalCmdI(ch, val2_max, i_div, rangeI);
					myCh->misc.semiSwitchState = SEMI_I_P;
					myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
				} else if(val2 > 0) {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, ON);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, OFF);
					cCalCmdV(ch, val1, v_div, rangeV);
					cCalCmdI(ch, val2, i_div, rangeI);
					myCh->misc.semiSwitchState = SEMI_I_P;
					myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
				} else {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, OFF);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, ON);
					cCalCmdV(ch, val1, v_div, rangeV);
					cCalCmdI(ch, val2, i_div, rangeI);
					myCh->misc.semiSwitchState = SEMI_I_N;
					myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
				}
				myCh->signal[C_SIG_OUT_SWITCH] = P86;
				myCh->signal[C_SIG_SEMI_SWITCH] = P0;
				break;
		}
	} else if(myCh->signal[C_SIG_SELECTED_RANGE_I] == RANGE3) {
		switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
			case P0:
				if(slot != 0) break;
				if(myCh->misc.semiSwitchState == SEMI_I_P) {
					if(val2 == 0) {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					} else if(val2 > 0) {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						switch(sysModel) {
							case 1:
							case 2:
								cCalCmdI(ch, (long)((float)val2_min * 1.5),
									i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_min, i_div, rangeI);
								break;
						}
					} else {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					}
				} else { //SEMI_I_N
					if(val2 == 0) {
						cCalCmdV(ch, val1_max, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					} else if(val2 > 0) {
						cCalCmdV(ch, val1_max, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					} else {
						cCalCmdV(ch, val1_0, v_div, rangeV);
						cCalCmdI(ch, val2_max, i_div, rangeI);
					}
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P1:
				if(myCh->misc.semiSwitchState == SEMI_I_P) {
					if(val2 == 0) {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						cCalCmdI(ch, val2_max, i_div, rangeI);
					} else if(val2 > 0) {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						switch(sysModel) {
							case 1:
							case 2:
								cCalCmdI(ch, (long)((float)val2_min * 1.5),
									i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_min, i_div, rangeI);
								break;
						}
					} else {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
					}
				} else { //SEMI_I_N
					if(val2 == 0) {
						cCalCmdV(ch, val1_max, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
						myCh->signal[C_SIG_SEMI_SWITCH]++;
					} else if(val2 > 0) {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
					} else {
					}
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P2:
				if(myCh->misc.semiSwitchState == SEMI_I_P) {
					if(val2 == 0) {
					} else if(val2 > 0) {
						cCalCmdV(ch, val1, v_div, rangeV);
						switch(sysModel) {
							case 1:
							case 2:
								cCalCmdI(ch, (long)((float)val2_min * 0.2),
									i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_0, i_div, rangeI);
								break;
						}
					} else {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
					}
				} else { //SEMI_I_N
					if(val2 == 0) {
						cCalCmdV(ch, val1_0, v_div, rangeV);
						cCalCmdI(ch, val2_max, i_div, rangeI);
					} else if(val2 > 0) {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
					} else {
					}
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P3:
				if(val2 == 0) {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, OFF);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, OFF);
					cCalCmdV(ch, val1_min, v_div, rangeV);
					cCalCmdI(ch, val2_max, i_div, rangeI);
					myCh->misc.semiSwitchState = SEMI_I_P;
					myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
				} else if(val2 > 0) {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, ON);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, OFF);
					cCalCmdV(ch, val1, v_div, rangeV);
					cCalCmdI(ch, val2, i_div, rangeI);
					myCh->misc.semiSwitchState = SEMI_I_P;
					myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
				} else {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, OFF);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, ON);
					cCalCmdV(ch, val1, v_div, rangeV);
					cCalCmdI(ch, val2, i_div, rangeI);
					myCh->misc.semiSwitchState = SEMI_I_N;
					myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
				}
				myCh->signal[C_SIG_OUT_SWITCH] = P86;
				myCh->signal[C_SIG_SEMI_SWITCH] = P0;
				break;
		}
	} else {
		switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
			case P0:
				if(slot != 0) break;
				if(myCh->misc.semiSwitchState == SEMI_I_P) {
					if(val2 == 0) {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					} else if(val2 > 0) {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						switch(sysModel) {
							case 1:
							case 2:
								cCalCmdI(ch, (long)((float)val2_min * 1.5),
									i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_min, i_div, rangeI);
								break;
						}
					} else {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					}
				} else { //SEMI_I_N
					if(val2 == 0) {
						cCalCmdV(ch, val1_max, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					} else if(val2 > 0) {
						cCalCmdV(ch, val1_max, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
					} else {
						cCalCmdV(ch, val1_0, v_div, rangeV);
						cCalCmdI(ch, val2_max, i_div, rangeI);
					}
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P1:
				if(myCh->misc.semiSwitchState == SEMI_I_P) {
					if(val2 == 0) {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						cCalCmdI(ch, val2_max, i_div, rangeI);
					} else if(val2 > 0) {
						cCalCmdV(ch, val1_min, v_div, rangeV);
						switch(sysModel) {
							case 1:
							case 2:
								cCalCmdI(ch, (long)((float)val2_min * 1.5),
									i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_min, i_div, rangeI);
								break;
						}
					} else {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
					}
				} else { //SEMI_I_N
					if(val2 == 0) {
						cCalCmdV(ch, val1_max, v_div, rangeV);
						cCalCmdI(ch, val2_0, i_div, rangeI);
						myCh->signal[C_SIG_SEMI_SWITCH]++;
					} else if(val2 > 0) {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
					} else {
					}
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P2:
				if(myCh->misc.semiSwitchState == SEMI_I_P) {
					if(val2 == 0) {
					} else if(val2 > 0) {
						cCalCmdV(ch, val1, v_div, rangeV);
						switch(sysModel) {
							case 1:
							case 2:
								cCalCmdI(ch, (long)((float)val2_min * 0.2),
									i_div, rangeI);
								break;
							default:
								cCalCmdI(ch, val2_0, i_div, rangeI);
								break;
						}
					} else {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
					}
				} else { //SEMI_I_N
					if(val2 == 0) {
						cCalCmdV(ch, val1_0, v_div, rangeV);
						cCalCmdI(ch, val2_max, i_div, rangeI);
					} else if(val2 > 0) {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
					} else {
					}
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P3:
				if(val2 == 0) {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, OFF);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, OFF);
					cCalCmdV(ch, val1_min, v_div, rangeV);
					cCalCmdI(ch, val2_max, i_div, rangeI);
					myCh->misc.semiSwitchState = SEMI_I_P;
					myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
				} else if(val2 > 0) {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, ON);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, OFF);
					cCalCmdV(ch, val1, v_div, rangeV);
					cCalCmdI(ch, val2, i_div, rangeI);
					myCh->misc.semiSwitchState = SEMI_I_P;
					myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
				} else {
					Select_OutPoint(group, ch+1, O_LAMP_CHARGE, OFF);
					Select_OutPoint(group, ch+1, O_LAMP_DISCHARGE, ON);
					cCalCmdV(ch, val1, v_div, rangeV);
					cCalCmdI(ch, val2, i_div, rangeI);
					myCh->misc.semiSwitchState = SEMI_I_N;
					myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
				}
				myCh->signal[C_SIG_OUT_SWITCH] = P86;
				myCh->signal[C_SIG_SEMI_SWITCH] = P0;
				break;
		}
	}
}

