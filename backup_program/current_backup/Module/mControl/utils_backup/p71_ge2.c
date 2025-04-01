
void cOutSwitch_P71_General2(int ch, int slot)
{
	int v_div, i_div, rangeV, rangeI, i, idxStepNo, idx, sysModel;
	long val1, val1_max, val1_0, val1_min, val2, val2_max, val2_0, val2_min;
	long stepTime, rest_relay_time, v_cmd_offset_p, v_cmd_offset_n;

	v_div = (int)myCh->misc.cmd_v_div;
	i_div = (int)myCh->misc.cmd_i_div;
	rangeV = myCh->op.rangeV;
	rangeI = myCh->op.rangeI;

	if(myCh->misc.cmd_i[0] >= 0) {
		val1 = myCh->misc.cmd_v[0];
	} else {
		val1 = myCh->misc.cmd_v[1];
	}
	val1_max = myPs->config.maxV[rangeV];
	val1_0 = 0;
	val1_min = val1_max * (-1);

	val2 = myCh->misc.cmd_i[0];
	val2_max = myPs->config.maxI[rangeI];
	val2_0 = 0;
	val2_min = myPs->config.minI[rangeI];

	idxStepNo = myCh->op.idxStepNo;
	idx = IDX_LOC_OBJ_END_TIME;
	stepTime = myTestCond->local_object[idxStepNo][idx];

	switch(myData->AppControl.config.systemModel) {
		case C_SDI_70V_250A_25A_18KW:
			sysModel = 1;
			v_cmd_offset_p = 20000; //20mV
			v_cmd_offset_n = 55000; //55mV
			break;
		default:
			sysModel = 0;
			v_cmd_offset_p = 0;
			v_cmd_offset_n = 0;
			break;
	}
	rest_relay_time = 1000; //10sec

	if(myCh->signal[C_SIG_SELECTED_RANGE_I] == RANGE0) {
		switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
			case P0:
				if(slot != 0) break;
				if(val2 == 0) {
					cCalCmdV(ch, val1_min, 1, rangeV);
					cCalCmdI(ch, val2_max, 1, rangeI);
					myCh->misc.semiSwitchState = SEMI_I_P;
					myCh->signal[C_SIG_OUT_SWITCH] = P72;
					myCh->signal[C_SIG_SEMI_SWITCH] = P0;
				} else if(val2 > 0) {
					if(myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4) {
						cCalCmdV(ch, val1_0, v_div, rangeV);
						cCalCmdI(ch, val2_min, 1, rangeI);
						myCh->signal[C_SIG_SEMI_SWITCH]++;
					} else {
						cCalCmdV(ch, val1_max, 1, rangeV);
						cCalCmdI(ch, val2_min, 1, rangeI);
						myCh->signal[C_SIG_SEMI_SWITCH] = P10;
					}
				} else {
					if(myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4) {
						cCalCmdV(ch, val1 + v_cmd_offset_n, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_N;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					} else {
						cCalCmdV(ch, val1_min, 1, rangeV);
						cCalCmdI(ch, val2_max, 1, rangeI);
						myCh->signal[C_SIG_SEMI_SWITCH] = P10;
					}
				}
				break;
			case P1:
				if(val2 == 0) {
				} else if(val2 > 0) {
					cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
					cCalCmdI(ch, val2_min, 1, rangeI);
				} else {
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P2:
				if(val2 == 0) {
				} else if(val2 > 0) {
					cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
					cCalCmdI(ch, val2_min, 1, rangeI);
				} else {
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P3:
				if(val2 == 0) {
				} else if(val2 > 0) {
					cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
					cCalCmdI(ch, val2_min, 1, rangeI);
				} else {
				}
				myCh->signal[C_SIG_SEMI_SWITCH]++;
				break;
			case P4:
				if(val2 == 0) {
				} else if(val2 > 0) {
					cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
					cCalCmdI(ch, val2, i_div, rangeI);
					myCh->misc.semiSwitchState = SEMI_I_P;
				} else {
				}
				myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
				myCh->signal[C_SIG_OUT_SWITCH] = P72;
				myCh->signal[C_SIG_SEMI_SWITCH] = P0;
				break;
			case P10:
				if(slot != 0) break;
				if(myData->ChAttribute[ch].chNo_master != 0) { //for master
					if(val2 == 0) {
						if(myCh->op.stepType == STEP_OCV
							|| (myCh->op.stepType == STEP_REST
							&& stepTime < rest_relay_time)) {
							myCh->misc.semiSwitchState = SEMI_IDLE;
							myCh->signal[C_SIG_OUT_SWITCH] = P72;
							myCh->signal[C_SIG_SEMI_SWITCH] = P0;
							break;
						} else {
							cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
						}
					} else cOutSwitch_Select_ON_1(ch, rangeV, rangeI);
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
					} else if(val2 > 0) {
						cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_P;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
					} else {
						cCalCmdV(ch, val1 + v_cmd_offset_n, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_N;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
					}
					myCh->signal[C_SIG_OUT_SWITCH] = P72;
					myCh->signal[C_SIG_SEMI_SWITCH] = P0;
				} else {
					myCh->signal[C_SIG_SEMI_SWITCH]++;
				}
				break;
		}
	} else if(myCh->signal[C_SIG_SELECTED_RANGE_I] == RANGE1) {
		if((rangeI+1) == RANGE1) { //range equal
			switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
				case P0:
					if(slot != 0) break;
					if(myCh->misc.semiSwitchState == SEMI_I_P) {
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							switch(sysModel) {
								case 1:
									cCalCmdI(ch, (long)((float)val2_min * 1.9),
										1, rangeI);
									break;
								default:
									cCalCmdI(ch, (long)((float)val2_min * 1.5),
										1, rangeI);
									break;
							}
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH] = P3;
						}
					}
					break;
				case P1:
					if(myCh->misc.semiSwitchState == SEMI_I_P) {
						if(val2 == 0) {
							if(myCh->op.stepType == STEP_OCV
								|| (myCh->op.stepType == STEP_REST
								&& stepTime < rest_relay_time)) {
								cCalCmdV(ch, val1_min, 1, rangeV);
								cCalCmdI(ch, val2_max, 1, rangeI);
								myCh->misc.semiSwitchState = SEMI_I_P;
								myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
								myCh->signal[C_SIG_OUT_SWITCH] = P72;
								myCh->signal[C_SIG_SEMI_SWITCH] = P0;
							} else {
								cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
								myCh->signal[C_SIG_SEMI_SWITCH] = P11;
							}
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1 + v_cmd_offset_n, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					}
					break;
				case P2:
					if(myCh->misc.semiSwitchState == SEMI_I_P) {
						if(val2 == 0) {
						} else if(val2 > 0) {
							cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
							switch(sysModel) {
								case 1:
									cCalCmdI(ch, (long)((float)val2_min * 1.9),
										1, rangeI);
									break;
								default:
									cCalCmdI(ch, val2_0, i_div, rangeI);
									break;
							}
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1 + v_cmd_offset_n, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					}
					break;
				case P3:
					if(val2 == 0) {
						if(myCh->op.stepType == STEP_OCV
							|| (myCh->op.stepType == STEP_REST
							&& stepTime < rest_relay_time)) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->misc.semiSwitchState = SEMI_I_P;
							myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
							myCh->signal[C_SIG_OUT_SWITCH] = P72;
							myCh->signal[C_SIG_SEMI_SWITCH] = P0;
						} else {
							cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH] = P11;
						}
					} else if(val2 > 0) {
						cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_P;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					} else {
						cCalCmdV(ch, val1 + v_cmd_offset_n, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_N;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					}
					break;
				default:
					if(slot != 0) break;
					i = myCh->signal[C_SIG_SEMI_SWITCH] - P10;
					i *= (int)myPs->config.scan_period;
					if(i >= 200) {
						//200ms wait for Relay Active
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->misc.semiSwitchState = SEMI_I_P;
							myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						} else if(val2 > 0) {
						} else {
						}
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					} else {
						myCh->signal[C_SIG_SEMI_SWITCH]++;
					}
					break;
			}
		} else { //range not equal
			switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
				case P0:
					if(slot != 0) break;
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						}
					}
					break;
				case P1:
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
						if(val2 == 0) {
							if(myCh->op.stepType == STEP_OCV
								|| (myCh->op.stepType == STEP_REST
								&& stepTime < rest_relay_time)) {
								cCalCmdV(ch, val1_min, 1, rangeV);
								cCalCmdI(ch, val2_max, 1, rangeI);
								myCh->misc.semiSwitchState = SEMI_I_P;
								myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
								myCh->signal[C_SIG_OUT_SWITCH] = P72;
								myCh->signal[C_SIG_SEMI_SWITCH] = P0;
							} else {
								cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
								myCh->signal[C_SIG_SEMI_SWITCH] = P11;
							}
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						} else {
						}
					}
					break;
				case P2:
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
						if(val2 == 0) {
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
						} else {
						}
					}
					break;
				case P3:
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
					} else { //SEMI_I_N
						if(val2 == 0) {
							if(myCh->op.stepType == STEP_OCV
								|| (myCh->op.stepType == STEP_REST
								&& stepTime < rest_relay_time)) {
								cCalCmdV(ch, val1_min, 1, rangeV);
								cCalCmdI(ch, val2_max, 1, rangeI);
								myCh->misc.semiSwitchState = SEMI_I_P;
								myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
								myCh->signal[C_SIG_OUT_SWITCH] = P72;
								myCh->signal[C_SIG_SEMI_SWITCH] = P0;
							} else {
								cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
								myCh->signal[C_SIG_SEMI_SWITCH] = P11;
							}
						} else if(val2 > 0) {
						} else {
						}
					}
					break;
				default:
					if(slot != 0) break;
					i = myCh->signal[C_SIG_SEMI_SWITCH] - P10;
					i *= (int)myPs->config.scan_period;
					if(i >= 200) {
						//200ms wait for Relay Active
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->misc.semiSwitchState = SEMI_I_P;
							myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						} else if(val2 > 0) {
						} else {
						}
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					} else {
						myCh->signal[C_SIG_SEMI_SWITCH]++;
					}
					break;
			}
		}
	} else if(myCh->signal[C_SIG_SELECTED_RANGE_I] == RANGE2) {
		if((rangeI+1) == RANGE2) { //range equal
			switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
				case P0:
					if(slot != 0) break;
					if(myCh->misc.semiSwitchState == SEMI_I_P) {
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							switch(sysModel) {
								case 1:
									cCalCmdI(ch, (long)((float)val2_min * 1.9),
										1, rangeI);
									break;
								default:
									cCalCmdI(ch, val2_0, i_div, rangeI);
									break;
							}
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH] = P3;
						}
					}
					break;
				case P1:
					if(myCh->misc.semiSwitchState == SEMI_I_P) {
						if(val2 == 0) {
							if(myCh->op.stepType == STEP_OCV
								|| (myCh->op.stepType == STEP_REST
								&& stepTime < rest_relay_time)) {
								cCalCmdV(ch, val1_min, 1, rangeV);
								cCalCmdI(ch, val2_max, 1, rangeI);
								myCh->misc.semiSwitchState = SEMI_I_P;
								myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
								myCh->signal[C_SIG_OUT_SWITCH] = P72;
								myCh->signal[C_SIG_SEMI_SWITCH] = P0;
							} else {
								cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
								myCh->signal[C_SIG_SEMI_SWITCH] = P11;
							}
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1 + v_cmd_offset_n, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					}
					break;
				case P2:
					if(myCh->misc.semiSwitchState == SEMI_I_P) {
						if(val2 == 0) {
						} else if(val2 > 0) {
							switch(sysModel) {
								case 1:
									cCalCmdV(ch, val1 + v_cmd_offset_p,
										v_div, rangeV);
									cCalCmdI(ch, (long)((float)val2_min * 1.9),
										1, rangeI);
									break;
								default:
									cCalCmdV(ch, val1_max, 1, rangeV);
									cCalCmdI(ch, val2_min, 1, rangeI);
									break;
							}
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1 + v_cmd_offset_n, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					}
					break;
				case P3:
					if(val2 == 0) {
						if(myCh->op.stepType == STEP_OCV
							|| (myCh->op.stepType == STEP_REST
							&& stepTime < rest_relay_time)) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeV);
							myCh->misc.semiSwitchState = SEMI_I_P;
							myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
							myCh->signal[C_SIG_OUT_SWITCH] = P72;
							myCh->signal[C_SIG_SEMI_SWITCH] = P0;
						} else {
							cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH] = P11;
						}
					} else if(val2 > 0) {
						cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_P;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					} else {
						cCalCmdV(ch, val1 + v_cmd_offset_n, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_N;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					}
					break;
				default:
					if(slot != 0) break;
					i = myCh->signal[C_SIG_SEMI_SWITCH] - P10;
					i *= (int)myPs->config.scan_period;
					if(i >= 200) {
						//200ms wait for Relay Active
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->misc.semiSwitchState = SEMI_I_P;
							myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						} else if(val2 > 0) {
						} else {
						}
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					} else {
						myCh->signal[C_SIG_SEMI_SWITCH]++;
					}
					break;
			}
		} else { //range not equal
			switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
				case P0:
					if(slot != 0) break;
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						}
					}
					break;
				case P1:
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
						if(val2 == 0) {
							if(myCh->op.stepType == STEP_OCV
								|| (myCh->op.stepType == STEP_REST
								&& stepTime < rest_relay_time)) {
								cCalCmdV(ch, val1_min, 1, rangeV);
								cCalCmdI(ch, val2_max, 1, rangeI);
								myCh->misc.semiSwitchState = SEMI_I_P;
								myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
								myCh->signal[C_SIG_OUT_SWITCH] = P72;
								myCh->signal[C_SIG_SEMI_SWITCH] = P0;
							} else {
								cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
								myCh->signal[C_SIG_SEMI_SWITCH] = P11;
							}
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							switch(sysModel) {
								case 1:
									cCalCmdV(ch, val1_max, 1, rangeV);
									break;
								default:
									cCalCmdV(ch, val1_0, v_div, rangeV);
									break;
							}
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						} else {
						}
					}
					break;
				case P2:
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
						if(val2 == 0) {
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
						} else {
						}
					}
					break;
				case P3:
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
					} else { //SEMI_N
						if(val2 == 0) {
							if(myCh->op.stepType == STEP_OCV
								|| (myCh->op.stepType == STEP_REST
								&& stepTime < rest_relay_time)) {
								cCalCmdV(ch, val1_min, 1, rangeV);
								cCalCmdI(ch, val2_max, 1, rangeI);
								myCh->misc.semiSwitchState = SEMI_I_P;
								myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
								myCh->signal[C_SIG_OUT_SWITCH] = P72;
								myCh->signal[C_SIG_SEMI_SWITCH] = P0;
							} else {
								cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
								myCh->signal[C_SIG_SEMI_SWITCH] = P11;
							}
						} else if(val2 > 0) {
						} else {
						}
					}
					break;
				default:
					if(slot != 0) break;
					i = myCh->signal[C_SIG_SEMI_SWITCH] - P10;
					i *= (int)myPs->config.scan_period;
					if(i >= 200) {
						//200ms wait for Relay Active
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->misc.semiSwitchState = SEMI_I_P;
							myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						} else if(val2 > 0) {
						} else {
						}
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					} else {
						myCh->signal[C_SIG_SEMI_SWITCH]++;
					}
					break;
			}
		}
	} else if(myCh->signal[C_SIG_SELECTED_RANGE_I] == RANGE3) {
		if((rangeI+1) == RANGE3) { //range equal
			switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
				case P0:
					if(slot != 0) break;
					if(myCh->misc.semiSwitchState == SEMI_I_P) {
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							switch(sysModel) {
								case 1:
									cCalCmdI(ch, (long)((float)val2_min * 1.9),
										1, rangeI);
									break;
								default:
									cCalCmdI(ch, val2_0, i_div, rangeI);
									break;
							}
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH] = P3;
						}
					}
					break;
				case P1:
					if(myCh->misc.semiSwitchState == SEMI_I_P) {
						if(val2 == 0) {
							if(myCh->op.stepType == STEP_OCV
								|| (myCh->op.stepType == STEP_REST
								&& stepTime < rest_relay_time)) {
								cCalCmdV(ch, val1_min, 1, rangeV);
								cCalCmdI(ch, val2_max, 1, rangeI);
								myCh->misc.semiSwitchState = SEMI_I_P;
								myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
								myCh->signal[C_SIG_OUT_SWITCH] = P72;
								myCh->signal[C_SIG_SEMI_SWITCH] = P0;
							} else {
								cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
								myCh->signal[C_SIG_SEMI_SWITCH] = P11;
							}
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1 + v_cmd_offset_n, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					}
					break;
				case P2:
					if(myCh->misc.semiSwitchState == SEMI_I_P) {
						if(val2 == 0) {
						} else if(val2 > 0) {
							switch(sysModel) {
								case 1:
									cCalCmdV(ch, val1 + v_cmd_offset_p,
										v_div, rangeV);
									cCalCmdI(ch, (long)((float)val2_min * 1.9),
										1, rangeI);
									break;
								default:
									cCalCmdV(ch, val1_max, 1, rangeV);
									cCalCmdI(ch, val2_min, 1, rangeI);
									break;
							}
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1 + v_cmd_offset_n, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					}
					break;
				case P3:
					if(val2 == 0) {
						if(myCh->op.stepType == STEP_OCV
							|| (myCh->op.stepType == STEP_REST
							&& stepTime < rest_relay_time)) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeV);
							myCh->misc.semiSwitchState = SEMI_I_P;
							myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
							myCh->signal[C_SIG_OUT_SWITCH] = P72;
							myCh->signal[C_SIG_SEMI_SWITCH] = P0;
						} else {
							cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH] = P11;
						}
					} else if(val2 > 0) {
						cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_P;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					} else {
						cCalCmdV(ch, val1 + v_cmd_offset_n, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_N;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					}
					break;
				default:
					if(slot != 0) break;
					i = myCh->signal[C_SIG_SEMI_SWITCH] - P10;
					i *= (int)myPs->config.scan_period;
					if(i >= 200) {
						//200ms wait for Relay Active
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->misc.semiSwitchState = SEMI_I_P;
							myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						} else if(val2 > 0) {
						} else {
						}
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					} else {
						myCh->signal[C_SIG_SEMI_SWITCH]++;
					}
					break;
			}
		} else { //range not equal
			switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
				case P0:
					if(slot != 0) break;
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						}
					}
					break;
				case P1:
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
						if(val2 == 0) {
							if(myCh->op.stepType == STEP_OCV
								|| (myCh->op.stepType == STEP_REST
								&& stepTime < rest_relay_time)) {
								cCalCmdV(ch, val1_min, 1, rangeV);
								cCalCmdI(ch, val2_max, 1, rangeI);
								myCh->misc.semiSwitchState = SEMI_I_P;
								myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
								myCh->signal[C_SIG_OUT_SWITCH] = P72;
								myCh->signal[C_SIG_SEMI_SWITCH] = P0;
							} else {
								cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
								myCh->signal[C_SIG_SEMI_SWITCH] = P11;
							}
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							switch(sysModel) {
								case 1:
									cCalCmdV(ch, val1_max, 1, rangeV);
									break;
								default:
									cCalCmdV(ch, val1_0, v_div, rangeV);
									break;
							}
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						} else {
						}
					}
					break;
				case P2:
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
						if(val2 == 0) {
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
						} else {
						}
					}
					break;
				case P3:
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
					} else { //SEMI_N
						if(val2 == 0) {
							if(myCh->op.stepType == STEP_OCV
								|| (myCh->op.stepType == STEP_REST
								&& stepTime < rest_relay_time)) {
								cCalCmdV(ch, val1_min, 1, rangeV);
								cCalCmdI(ch, val2_max, 1, rangeI);
								myCh->misc.semiSwitchState = SEMI_I_P;
								myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
								myCh->signal[C_SIG_OUT_SWITCH] = P72;
								myCh->signal[C_SIG_SEMI_SWITCH] = P0;
							} else {
								cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
								myCh->signal[C_SIG_SEMI_SWITCH] = P11;
							}
						} else if(val2 > 0) {
						} else {
						}
					}
					break;
				default:
					if(slot != 0) break;
					i = myCh->signal[C_SIG_SEMI_SWITCH] - P10;
					i *= (int)myPs->config.scan_period;
					if(i >= 200) {
						//200ms wait for Relay Active
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->misc.semiSwitchState = SEMI_I_P;
							myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						} else if(val2 > 0) {
						} else {
						}
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					} else {
						myCh->signal[C_SIG_SEMI_SWITCH]++;
					}
					break;
			}
		}
	} else if(myCh->signal[C_SIG_SELECTED_RANGE_I] == RANGE4) {
		if((rangeI+1) == RANGE4) { //range equal
			switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
				case P0:
					if(slot != 0) break;
					if(myCh->misc.semiSwitchState == SEMI_I_P) {
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							switch(sysModel) {
								case 1:
									cCalCmdI(ch, (long)((float)val2_min * 1.9),
										1, rangeI);
									break;
								default:
									cCalCmdI(ch, val2_0, i_div, rangeI);
									break;
							}
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH] = P3;
						}
					}
					break;
				case P1:
					if(myCh->misc.semiSwitchState == SEMI_I_P) {
						if(val2 == 0) {
							if(myCh->op.stepType == STEP_OCV
								|| (myCh->op.stepType == STEP_REST
								&& stepTime < rest_relay_time)) {
								cCalCmdV(ch, val1_min, 1, rangeV);
								cCalCmdI(ch, val2_max, 1, rangeI);
								myCh->misc.semiSwitchState = SEMI_I_P;
								myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
								myCh->signal[C_SIG_OUT_SWITCH] = P72;
								myCh->signal[C_SIG_SEMI_SWITCH] = P0;
							} else {
								cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
								myCh->signal[C_SIG_SEMI_SWITCH] = P11;
							}
						} else if(val2 > 0) {
							switch(sysModel) {
								case 1:
									cCalCmdV(ch, val1_min, 1, rangeV);
									break;
								default:
									cCalCmdV(ch, val1_max, 1, rangeV);
									break;
							}
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1 + v_cmd_offset_n, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					}
					break;
				case P2:
					if(myCh->misc.semiSwitchState == SEMI_I_P) {
						if(val2 == 0) {
						} else if(val2 > 0) {
							switch(sysModel) {
								case 1:
									cCalCmdV(ch, val1 + v_cmd_offset_p,
										v_div, rangeV);
									cCalCmdI(ch, (long)((float)val2_min * 1.9),
										1, rangeI);
									break;
								default:
									cCalCmdV(ch, val1_max, 1, rangeV);
									cCalCmdI(ch, val2_min, 1, rangeI);
									break;
							}
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1 + v_cmd_offset_n, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1 + v_cmd_offset_p, v_div, rangeV);
							cCalCmdI(ch, val2, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					}
					break;
				case P3:
					if(val2 == 0) {
						if(myCh->op.stepType == STEP_OCV
							|| (myCh->op.stepType == STEP_REST
							&& stepTime < rest_relay_time)) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeV);
							myCh->misc.semiSwitchState = SEMI_I_P;
							myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
							myCh->signal[C_SIG_OUT_SWITCH] = P72;
							myCh->signal[C_SIG_SEMI_SWITCH] = P0;
						} else {
							cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH] = P11;
						}
					} else if(val2 > 0) {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_P;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					} else {
						cCalCmdV(ch, val1, v_div, rangeV);
						cCalCmdI(ch, val2, i_div, rangeI);
						myCh->misc.semiSwitchState = SEMI_I_N;
						myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					}
					break;
				default:
					if(slot != 0) break;
					i = myCh->signal[C_SIG_SEMI_SWITCH] - P10;
					i *= (int)myPs->config.scan_period;
					if(i >= 200) {
						//200ms wait for Relay Active
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->misc.semiSwitchState = SEMI_I_P;
							myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						} else if(val2 > 0) {
						} else {
						}
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					} else {
						myCh->signal[C_SIG_SEMI_SWITCH]++;
					}
					break;
			}
		} else { //range not equal
			switch(myCh->signal[C_SIG_SEMI_SWITCH]) {
				case P0:
					if(slot != 0) break;
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						}
					}
					break;
				case P1:
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
						if(val2 == 0) {
							if(myCh->op.stepType == STEP_OCV
								|| (myCh->op.stepType == STEP_REST
								&& stepTime < rest_relay_time)) {
								cCalCmdV(ch, val1_min, 1, rangeV);
								cCalCmdI(ch, val2_max, 1, rangeI);
								myCh->misc.semiSwitchState = SEMI_I_P;
								myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
								myCh->signal[C_SIG_OUT_SWITCH] = P72;
								myCh->signal[C_SIG_SEMI_SWITCH] = P0;
							} else {
								cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
								myCh->signal[C_SIG_SEMI_SWITCH] = P11;
							}
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							switch(sysModel) {
								case 1:
									cCalCmdV(ch, val1_max, 1, rangeV);
									break;
								default:
									cCalCmdV(ch, val1_0, v_div, rangeV);
									break;
							}
							cCalCmdI(ch, val2_0, i_div, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						} else {
						}
					}
					break;
				case P2:
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
						if(val2 == 0) {
						} else if(val2 > 0) {
							cCalCmdV(ch, val1_max, 1, rangeV);
							cCalCmdI(ch, val2_min, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						} else {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SELECTED_RANGE_I] = RANGE0;
							myCh->signal[C_SIG_SEMI_SWITCH] = P10;
						}
					} else { //SEMI_I_N
						if(val2 == 0) {
							cCalCmdV(ch, val1_0, v_div, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->signal[C_SIG_SEMI_SWITCH]++;
						} else if(val2 > 0) {
						} else {
						}
					}
					break;
				case P3:
					if(myCh->misc.semiSwitchState == SEMI_I_P
						|| (myCh->misc.semiSwitchState >= SEMI_I_RANGE1
						&& myCh->misc.semiSwitchState <= SEMI_I_RANGE4)) {
					} else { //SEMI_N
						if(val2 == 0) {
							if(myCh->op.stepType == STEP_OCV
								|| (myCh->op.stepType == STEP_REST
								&& stepTime < rest_relay_time)) {
								cCalCmdV(ch, val1_min, 1, rangeV);
								cCalCmdI(ch, val2_max, 1, rangeI);
								myCh->misc.semiSwitchState = SEMI_I_P;
								myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
								myCh->signal[C_SIG_OUT_SWITCH] = P72;
								myCh->signal[C_SIG_SEMI_SWITCH] = P0;
							} else {
								cOutSwitch_Select_OFF_1(ch, rangeV, rangeI);
								myCh->signal[C_SIG_SEMI_SWITCH] = P11;
							}
						} else if(val2 > 0) {
						} else {
						}
					}
					break;
				default:
					if(slot != 0) break;
					i = myCh->signal[C_SIG_SEMI_SWITCH] - P10;
					i *= (int)myPs->config.scan_period;
					if(i >= 200) {
						//200ms wait for Relay Active
						if(val2 == 0) {
							cCalCmdV(ch, val1_min, 1, rangeV);
							cCalCmdI(ch, val2_max, 1, rangeI);
							myCh->misc.semiSwitchState = SEMI_I_P;
							myCh->signal[C_SIG_SELECTED_RANGE_I] = rangeI+1;
						} else if(val2 > 0) {
						} else {
						}
						myCh->signal[C_SIG_OUT_SWITCH] = P72;
						myCh->signal[C_SIG_SEMI_SWITCH] = P0;
					} else {
						myCh->signal[C_SIG_SEMI_SWITCH]++;
					}
					break;
			}
		}
	}
}

