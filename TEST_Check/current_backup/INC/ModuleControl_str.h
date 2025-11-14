#ifndef __MODULECONTROL_STR_H__
#define __MODULECONTROL_STR_H__

#include "SysDefine.h"
#include "ModuleControl_def.h"

typedef union u_adda_tag {
    short int 			val;
    unsigned char		byte[2];
} U_ADDA;

typedef struct s_aux_10bit_tag {
	unsigned short		control : 2;
	unsigned short		val : 10;
	unsigned short		addr : 4;
} S_AUX_10BIT;

typedef union u_auxda_10_bit_tag {
	S_AUX_10BIT			data;
	unsigned char		byte[2];
} U_AUXDA_10BIT;
#ifdef __COA_VER_100B__
typedef struct s_ch_misc_tag {
	unsigned char		tmpState;
	unsigned char		tmpPhase;
	unsigned char		semiSwitchState;
	unsigned char		ripple_out; //for C_LGC_500V_20A

	int					tmpCode;
	int					saveCode;

	long				maxV;
	long				minV;
	long				maxI;
	long				minI;
	long				tmpVsens;
	long				tmpIsens;
   	long				tmpWatt;

	long				ccv1;
	long				ccv2;

	unsigned char		sensCount;
	unsigned char		tmp_sensCount;
	unsigned char		sensCountFlag;
	unsigned char		waveform_type;

	unsigned char		nonCell;
	unsigned char		errorCode;
	unsigned char		d_r_count;
	unsigned char		external_comm_state;
							//0:normal, 1:error
							//0bit:aux_temp, 1bit:aux_v,
							//2bit:can_master, 3bit:can_slave

   	long				sumV[MAX_AD_COUNT];
   	long				sumI[MAX_AD_COUNT];
   	long				sumVi[MAX_AD_COUNT];
   	long				sumVp[MAX_AD_COUNT];
   	long				sumVb[MAX_AD_COUNT];

   	double				sum_charge_AmpareHour;
   	double				seed_charge_AmpareHour;
   	double				sum_discharge_AmpareHour;
   	double				seed_discharge_AmpareHour;
   	double				sum_charge_WattHour;
   	double				seed_charge_WattHour;
   	double				sum_discharge_WattHour;
   	double				seed_discharge_WattHour;

	long				total_tmpIsens;
	long				total_Isens;
	long				total_AmpareHour;
	long				total_WattHour;
	long				total_watt;
	long				sum_AmpareHour;
	long				sum_AmpareHourSOC;	//20181219 KHK
	long				sum_WattHour;

	long				sum_CycleAmpareHour;
	long				seed_CycleAmpareHour;
	long				sum_CycleWattHour;
	long				seed_CycleWattHour;
	unsigned long		sum_CycleTime;

	long				meanSumCount;

	unsigned char		common_codeCount[MAX_COM_CODE];
	unsigned char		local_codeCount[MAX_LOC_CODE];	//jhkw_180823
	//int					local_codeCount[MAX_LOC_CODE];	//jhkw_180823
	unsigned char		aux_codeCount[MAX_AUX_CODE];
	unsigned char		can_codeCount[MAX_CAN_CODE];

	long				upper_delta_v_t;
	long				lower_delta_v_t;
	long				upper_delta_i_t;
	long				lower_delta_i_t;
	long				upper_delta_c_t;
	long				lower_delta_c_t;

	long				sensSumV[MAX_FILTER_AD_COUNT];
	long				sensSumI[MAX_FILTER_AD_COUNT];
	long				sensSumVi[MAX_FILTER_AD_COUNT];
	long				sensSumVp[MAX_FILTER_AD_COUNT];
	long				sensSumVb[MAX_FILTER_AD_COUNT];

	long				tmpVsens_avg; //kjg_111006_s
	long				tmpVsens_avg_sum[MAX_AVERAGE_COUNT];
	long				tmpIsens_avg;
	long				tmpIsens_avg_sum[MAX_AVERAGE_COUNT];
	unsigned char		tmp_avg_index;
	unsigned char		tmp_avg_flag;
	unsigned short int	dcr_delay_count; //kjg_111006_e

	long				c_v1;
	long				c_v2;
	unsigned long		c_t1;
	unsigned long		c_t2;
	unsigned int		d_count;
	int					d_flag;
	long				d_v[1000];
	long				d_i[1000];
	long				d_t[1000];
	long				d_voltage;
	
	short int			sendDataCount;
	//kjg_190323 short int			reserved0; //kjg_171219
	unsigned char		can_fault_check_phase; //kjg_190323
	unsigned char		reserved0;

	//short int			pattern_count;
	unsigned int		pattern_count;	//jhkw_130619
	unsigned long		pattern_time;
   	unsigned long		saveDt;
   	unsigned long		saveDv;
   	unsigned long		saveDi;
   	long				saveDtemp;
	long				actualCapacity[MAX_TEST_STEP];
	long				actualWattHour[MAX_TEST_STEP]; //kjg_171219
	unsigned char		update_ampareHour_flag[MAX_TEST_STEP]; //kjg_171219
	unsigned char		update_wattHour_flag[MAX_TEST_STEP];

	short int			user_branch_stepNo;		//1base
	short int			active_division;		//1base stepNo

	int					active_code;
	unsigned long		totalCycle;				//1base

	short int			fbCountV;
	short int			fbCountI;
	long				fbV;
	long				fbI;

   	unsigned long		checkDelayTime;
	unsigned long		tmpDelayTime;

	double				pid_ui1[MAX_TYPE];
	double				pid_error1[MAX_TYPE];

	long				cmd_v[2];
	long				cmd_i[2];
	short int			cmd_v_div;
	short int			cmd_i_div;
	long				cmd_p[2];
	long				cmd_t[2];

	long				vs_value;

	long				groupTemp;

	long				d_r1;
	long				d_r2;

	unsigned char		fch_fault_count_ov;
	unsigned char		fch_fault_count_oc;
	unsigned char		fch_fault_count_ot;
	unsigned char		fch_fault_count_v_p;

	unsigned char		fch_fault_count_v_n;
	unsigned char		cv_select;
	unsigned char		ch_output_state; //kjg_101219
							//0:off, 1:on
							//0bit:key_on, 1bit:charge_on, 2bit:pack_relay_on
	unsigned char		ch_input_state; //kjg_101219

	long				Vinput;
	long				Vpower;
	long				Vbus;
	long				delay_ch_output_state; //kjg_110322

	long				update_stepNo; //kjg_170810
	//long				reserved1[5];	//jhkw_181001s
	long				reserved1[4];
	long				tmpVsens_slave; //jhkw_181001e

	long				tmp_val[4][10];
		//[0]:Vsens, [1]:Vinput, [2]:Vpower, [3]:Vbus
	
	unsigned char		can_warnning_count;
	unsigned char		can_error_count;
	unsigned char		tmp_ch_output_state; //kjg_110322
	unsigned char		internal_skip;

	unsigned short int	pulse_count;
	short int			sub_code; //kjg_120328

	unsigned long		multiCycleCount[MAX_TEST_STEP];

	short int			auxDaV; //kjg_110813_s
	short int			auxDaI;
	unsigned short		auxCnt;
	unsigned short		hw_current_check_phase; //kjg_170810 reserved2

	short int			sens_AD[MAX_TYPE][MAX_CH_AD_COUNT];
	long				avgSens_AD1[MAX_TYPE][MAX_CH_SENS_COUNT];
	long				avgSens_AD2[MAX_TYPE];
	long				tmpAvgSens_AD2[MAX_TYPE];

	long				capacitySumI;
	unsigned int		capacityCnt;
	long				preVref;
	long				preIref;
	float				stdev_v;
	float				stdev_i; //kjg_110813_e
} S_CH_MISC;
#else //COA_VER_100B2~
typedef struct s_ch_misc_tag {
	unsigned char		tmpState;
	unsigned char		tmpPhase;
	unsigned char		semiSwitchState;
	unsigned char		ripple_out; //for C_LGC_500V_20A

	int					tmpCode;
	int					saveCode;

	long				maxV;
	long				minV;
	long				maxI;
	long				minI;
	long				tmpVsens;
	long				tmpIsens;
   	long				tmpWatt;

	long				ccv1;
	long				ccv2;

	unsigned char		sensCount;
	unsigned char		tmp_sensCount;
	unsigned char		sensCountFlag;
	unsigned char		waveform_type;

	unsigned char		nonCell;
	unsigned char		errorCode;
	unsigned char		d_r_count;
	unsigned char		external_comm_state;
							//0:normal, 1:error
							//0bit:aux_temp, 1bit:aux_v,
							//2bit:can_master, 3bit:can_slave

   	long				sumV[MAX_AD_COUNT];
   	long				sumI[MAX_AD_COUNT];
   	long				sumVi[MAX_AD_COUNT];
   	long				sumVp[MAX_AD_COUNT];
   	long				sumVb[MAX_AD_COUNT];

   	double				sum_charge_AmpareHour;
   	double				seed_charge_AmpareHour;
   	double				sum_discharge_AmpareHour;
   	double				seed_discharge_AmpareHour;
   	double				sum_charge_WattHour;
   	double				seed_charge_WattHour;
   	double				sum_discharge_WattHour;
   	double				seed_discharge_WattHour;

	long				total_tmpIsens;
	long				total_Isens;
	long				total_AmpareHour;
	long				total_WattHour;
	long				total_watt;
	long				sum_AmpareHour;
	long				sum_AmpareHourSOC;	//20181219 KHK
	long				sum_WattHour;
	long				sum_CycleAmpareHour;
	long				seed_CycleAmpareHour;
	long				sum_CycleWattHour;
	long				seed_CycleWattHour;

	long				sum_CycleAmpareHourSOC; //20181219 KHK

	unsigned long		sum_CycleTime_day;
	unsigned long		sum_CycleTime;

	long				meanSumCount;
//83
	unsigned char		common_codeCount[MAX_COM_CODE];
	unsigned char		local_codeCount[MAX_LOC_CODE];	//jhkw_180823
	//int					local_codeCount[MAX_LOC_CODE];	//jhkw_180823
	unsigned char		aux_codeCount[MAX_AUX_CODE];
	unsigned char		can_codeCount[MAX_CAN_CODE];

	long				upper_delta_v_t;
	long				lower_delta_v_t;
	long				upper_delta_i_t;
	long				lower_delta_i_t;
	long				upper_delta_c_t;
	long				lower_delta_c_t;

	long				sensSumV[MAX_FILTER_AD_COUNT];
	long				sensSumI[MAX_FILTER_AD_COUNT];
	long				sensSumVi[MAX_FILTER_AD_COUNT];
	long				sensSumVp[MAX_FILTER_AD_COUNT];
	long				sensSumVb[MAX_FILTER_AD_COUNT];

	short int			sendDataCount;
	short int			reserved0; //kjg_171219
	//short int			pattern_count;
	unsigned int		pattern_count;	//jhkw_130619
	unsigned long		pattern_time_day;
	unsigned long		pattern_time;
   	unsigned long		saveDt;
   	unsigned long		saveDv;
   	unsigned long		saveDi;
   	long				saveDtemp;
	long				actualCapacity[MAX_TEST_STEP];
	long				actualWattHour[MAX_TEST_STEP]; //kjg_171219
	unsigned char		update_ampareHour_flag[MAX_TEST_STEP]; //kjg_171219
	unsigned char		update_wattHour_flag[MAX_TEST_STEP];

	short int			user_branch_stepNo;		//1base
	short int			active_division;		//1base stepNo

	int					active_code;
	unsigned long		totalCycle;				//1base

	short int			fbCountV;
	short int			fbCountI;
	long				fbV;
	long				fbI;

   	unsigned long		checkDelayTime_day;
   	unsigned long		checkDelayTime;
	unsigned long		tmpDelayTime;

	double				pid_ui1[MAX_TYPE];
	double				pid_error1[MAX_TYPE];

	long				cmd_v[2];
	long				cmd_i[2];
	short int			cmd_v_div;
	short int			cmd_i_div;
	long				cmd_p[2];
	long				cmd_t[2];

	long				vs_value;

	long				groupTemp;

	long				d_r1;
	long				d_r2;

	unsigned char		fch_fault_count_ov;
	unsigned char		fch_fault_count_oc;
	unsigned char		fch_fault_count_ot;
	unsigned char		fch_fault_count_v_p;

	unsigned char		fch_fault_count_v_n;
	unsigned char		cv_select;
	unsigned char		ch_output_state; //kjg_101219
							//0:off, 1:on
							//0bit:key_on, 1bit:charge_on, 2bit:pack_relay_on
	unsigned char		ch_input_state; //kjg_101219

	long				Vinput;
	long				Vpower;
	long				Vbus;
	long				delay_ch_output_state; //kjg_110322

	long				update_stepNo; //kjg_170810
	//long				reserved1[5];	//jhkw_181001s
	//long				reserved1[4];
	long				reserved1[3];
	long				update_step_count;	//jhkw_191229
	long				tmpVsens_slave; //jhkw_181001e

	long				tmp_val[4][10];
		//[0]:Vsens, [1]:Vinput, [2]:Vpower, [3]:Vbus
	
	unsigned char		can_warnning_count;
	unsigned char		can_error_count;
	unsigned char		tmp_ch_output_state; //kjg_110322
	unsigned char		internal_skip;

	unsigned short int	pulse_count;
	short int			sub_code; //kjg_120328

	unsigned long		multiCycleCount[MAX_TEST_STEP];

	short int			auxDaV; //kjg_110813_s
	short int			auxDaI;
	unsigned short		auxCnt;
	unsigned short		hw_current_check_phase; //kjg_170810 reserved2

	short int			sens_AD[MAX_TYPE][MAX_CH_AD_COUNT];
	long				avgSens_AD1[MAX_TYPE][MAX_CH_SENS_COUNT];
	long				avgSens_AD2[MAX_TYPE];
	long				tmpAvgSens_AD2[MAX_TYPE];

	long				capacitySumI;
	unsigned int		capacityCnt;
	long				preVref;
	long				preIref;
	float				stdev_v;
	float				stdev_i; //kjg_110813_e

	long				tmpVsens2; //kjhw_150129s
	long				tmpIsens2;
	long				tmpWatt2;
	//long				reserved3; //kjhw_150129e
	long				check_runTime;	//jhkw_220812
	
	long				maxAuxV; //kjhw_150730s
	long				minAuxV;
	long				diffAuxV;
	short int			maxAuxV_auxChNo;
	short int			minAuxV_auxChNo; //kjhw_150730e

	unsigned char		out_mux; //kjhw_151021s
	unsigned char		out_mux_backup;
	unsigned char		out_mux_fault;
	unsigned char		socSetFlag; //20181219 KHK => using CP SOC shh_t240629(241022)

	unsigned char		bms_link_flag; //kjh_160623
	//jhkw_170917s
	//unsigned char		reserved6[3];
	//unsigned char		reserved6;				//csk_190328r
	unsigned char		can_fault_check_phase; //kjg_190323
	short int			out_delay;
	//jhkw_170917s

	long				maxAuxTemp; //kjhw_170820s
	long				minAuxTemp;
	long				diffAuxTemp;
	short int			maxAuxTemp_auxChNo;
	short int			minAuxTemp_auxChNo; //kjhw_170820e

	long				maxAuxTH; //kjhw_170820s
	long				minAuxTH;
	long				diffAuxTH;
	short int			maxAuxTH_auxChNo;
	short int			minAuxTH_auxChNo; //kjhw_170820e

	long				maxAuxT; //kjhw_181223s
	long				minAuxT;
	long				diffAuxT;
	long				avgAuxT; //kjhw_181223
	//jhkw_190629s
	unsigned char		delay_count1;
	unsigned char		Aux_Func_Count;			//jhkw_210513
	unsigned char		sequence_count;	//jhkw_221205;
	unsigned char		sequence_end_flag; //jhkw_221205
	long				can_data_l[4];
	//jhkw_190629e
	unsigned char		comm_state;	//jhkw_230607
	unsigned char		reserved[3];
	//kjhw_170904s
//kjg_171223	unsigned short int	local_codeCount_aux[MAX_AUX_DATA][MAX_AUX_FUNCTION];
//kjg_171223	unsigned char		reserved7[3];
	//kjhw_170904e
	//shhw_230605s
	long				dMaxAuxV;		//dMaxAuxV[2] -> dMaxAuxV			//shhw_230614s
	long				dMinAuxV;		//dMinAuxV[2] -> dMinAuxV	
	short int			dMaxAuxVChNo;   //dMaxAuxVChNo[2] -> dMaxAuxVChNo
	short int			dMinAuxVChNo;	//dMinAuxVChNo[2] -> dMinAuxVChNo	//shhw_230614e
	/*
	long				dMaxAuxV[2];
	long				dMinAuxV[2];
	short int			dMinAuxVChNo[2];
	short int			dMaxAuxVChNo[2];
	*/
	//shhw_230605e
	long				sil_v_i_val[2]; //0:Voltage, 1: Current //shh_231124
	//shht_240629s(241022)
	long 				socRefI;
	unsigned char		reserved2[4];
	//shht_240629e(241022)
} S_CH_MISC;
#endif

typedef struct s_ch_cycle_tag {
	short int			cycle_idxStepNo;
	short int			loop_idxStepNo;

	unsigned long		cycle_count;
} S_CH_CYCLE;

typedef struct s_ch_ccv_tag {
	int					index;
	unsigned int		count;
	long				ad_ccv[20]; //kjg_111002 10 -> 20
	long				ad_cci[20]; //kjg_111002 10 -> 20
	long				avg_v;
	long				avg_i;
} S_CH_CCV;

#ifdef __COA_VER_100B__
typedef struct s_ch_operation_data_tag {
   	unsigned char		state;
   	unsigned char		phase;
   	unsigned char		stepType;
   	unsigned char		stepMode;

	unsigned char		grade;
	unsigned char		tray_cell_code;
	unsigned char		select;
	unsigned char		reservedCmd;	//0:normal, 1:stop, 2:pause

	short int			idxStepNo; //0base
	short int			client_stepNo;

   	unsigned long		totalRunTime;
   	unsigned long		runTime;

	int					code;

   	long				Vsens;
	long				ocv;
	long				meanV;
   	long				Isens;
   	long				meanI;
   	long				watt;

   	long				charge_WattHour;
   	long				discharge_WattHour;
	long				charge_AmpareHour;
	long				discharge_AmpareHour;
   	long				capacitance;

   	long 				z;
	long				temp;
	unsigned long		ccTime;
	unsigned long		cvTime;

	long				resultIndex;

	unsigned char		rangeV;
	unsigned char		rangeI;
	unsigned char		attribute;
	unsigned char		save_flag;

	unsigned long		checkDelayTime; //kjg_110813
} S_CH_OP_DATA;
#else //COA_VER_100B2~
typedef struct s_ch_operation_data_tag {
   	unsigned char		state;
   	unsigned char		phase;
   	unsigned char		stepType;
   	unsigned char		stepMode;

	unsigned char		grade;
	unsigned char		tray_cell_code;
	unsigned char		select;
	unsigned char		reservedCmd;	//0:normal, 1:stop, 2:pause

	short int			idxStepNo; //0base
	short int			client_stepNo;

   	unsigned long		totalRunTime_day;
   	unsigned long		totalRunTime;
   	unsigned long		runTime_day;
   	unsigned long		runTime;

	int					code;

   	long				Vsens;
	long				ocv;
	long				meanV;
   	long				Isens;
   	long				meanI;
   	long				watt;
   	long				charge_WattHour;
   	long				discharge_WattHour;
	long				charge_AmpareHour;
	long				discharge_AmpareHour;
   	long				capacitance;
   	long 				z;
	long				temp;
	unsigned long		ccTime_day;
	unsigned long		ccTime;
	unsigned long		cvTime_day;
	unsigned long		cvTime;

	long				resultIndex;

	unsigned char		rangeV;
	unsigned char		rangeI;
	unsigned char		attribute;
	unsigned char		save_flag;

	unsigned long		checkDelayTime; //kjg_110813
	float				SOC; //20181219 KHK
	float				rptSOC;//20181219 KHK
	long				seed_CycleAmpareHourSOC; //20181219 KHK
	unsigned long		charging_counter; //20181219 KHK
	unsigned long		charging_counter_rpt_soc; //20181219 KHK
	float				link_rptSOC;	//jhkw_191108
	float				current_rptSOC;	//jhkw_191108
	unsigned char		gui_error_code;	//jhkw_201102s
	unsigned char		reserved1[3];	//jhkw_201102e
	long				dIsens;		//shhw_230605 
	long				preIsens;	//shhw_230605 
	unsigned char		savePeriodFlag;		//shhw_230616
	unsigned char		reserved2[3];		//shhw_230616
} S_CH_OP_DATA;
#endif

typedef struct s_ch_capacitance_tag {
	long				tmpVsens_avg; //kjg_111006_s
	long				tmpVsens_avg_sum[MAX_AVERAGE_COUNT];
	long				tmpIsens_avg;
	long				tmpIsens_avg_sum[MAX_AVERAGE_COUNT];
	unsigned char		tmp_avg_index;
	unsigned char		tmp_avg_flag;
	unsigned short int	dcr_delay_count; //kjg_111006_e

	long				c_v1;
	long				c_v2;
	unsigned long		c_t1;
	unsigned long		c_t2;
	unsigned int		d_count;
	int					d_flag;
	long				d_v[1000];
	long				d_i[1000];
	long				d_t[1000];
	long				d_voltage;
} S_CH_CAPACITANCE;

typedef struct s_ch_data_tag {
	S_CH_OP_DATA		op;
	S_CH_OP_DATA		opSave;

	S_CH_CCV			ccv[2];
	S_CH_CYCLE			elementCycle;
	S_CH_CYCLE			multiCycle[MAX_MULTI_CYCLE];
	S_CH_CYCLE			accCycle[MAX_ACC_CYCLE];
   	S_CH_MISC			misc;
	S_CH_CAPACITANCE	capa;

   	unsigned char		signal[MAX_SIGNAL];
   	unsigned char		cmd[MAX_CMD_MODE];
} S_CH_DATA;

typedef struct s_auto_cali_reference_tag { //kjg_110813_s
	short int			sens_AD[MAX_REF_CH][MAX_REF_AD_COUNT];
	long				avgSens_AD1[MAX_REF_CH][MAX_REF_SENS_COUNT1];
	long				avgSens_AD2[MAX_REF_CH][MAX_REF_SENS_COUNT2];
	long				tmp_value[MAX_REF_CH];
	long				cal_value[MAX_REF_CH];
	unsigned short		ref_sens_count1;
	unsigned short		ref_sens_count2;
	unsigned char		ref_sens_count_flag1;
	unsigned char		ref_sens_count_flag2;
	unsigned char		reserved1[2];

	double				AD_a;
	double				AD_b;
} S_AUTO_CALI_REFERENCE;

typedef struct s_temp_data_tag {
	unsigned long		temp_sens_cnt;
	long				sumSens_TempAD_BD[MAX_CH_SENS_COUNT];
	long				temp_BD;
	long				tempStart_BD;
	long				tempEnd_BD;
	long				tempAvr_BD;
	long				tempMin_BD;
	long				tempMax_BD;
	double				tempMeanSum_BD;

	long				sumSens_TempAD_Jig[MAX_CH_SENS_COUNT];
	long				temp_Jig;
	long				tempStart_Jig;
	long				tempEnd_Jig;
	long				tempAvr_Jig;
	long				tempMin_Jig;
	long				tempMax_Jig;
	double				tempMeanSum_Jig;
} S_TEMP_DATA;

typedef struct s_module_temp_tag {
	int					temp_slot;
	unsigned short		sens_count1;
	unsigned short		sens_count2;

	unsigned char		sens_count1_inc_flag;
	unsigned char		sens_count2_inc_flag;
	unsigned char		reserved1[2]; //kjg_171219

	short int			sens_AD[MAX_TEMP_CH][MAX_TEMP_AD_COUNT];
	long				avgSens_AD1[MAX_TEMP_CH][MAX_TEMP_SENS_COUNT1];
	long				avgSens_AD2[MAX_TEMP_CH][MAX_TEMP_SENS_COUNT2];
	long				tmp_value[MAX_TEMP_CH];
	long				cal_value[MAX_TEMP_CH];
	S_TEMP_DATA			data[MAX_TEMP_CH];
	S_AUTO_CALI_REFERENCE	auto_cali_ref[MAX_TYPE];
} S_MODULE_TEMP; //kjg_110813_e

typedef struct s_board_source_tag {
	long				sumV[MAX_AD_COUNT];
	long				sensSumV[MAX_FILTER_AD_COUNT];
	long				sourceV;
	long				calSourceV;
	long				sumI[MAX_AD_COUNT];
	long				sensSumI[MAX_FILTER_AD_COUNT];
	long				sourceI;
	long				calSourceI;

	//kjg_logic_type_140325_s
	long				sourceV2;
	long				calSourceV2;
	long				sourceI2;
	long				calSourceI2;
	//kjg_logic_type_140325_e
} S_BD_SOURCE;

typedef struct s_board_source2_tag {
	long				sumV[MAX_SOURCE_SENS_COUNT];
	double				totalV;
	long				sourceV;
	long				sumI[MAX_SOURCE_SENS_COUNT];
	double				totalI;
	long				sourceI;

	//kjg_logic_type_140325_s
	long				sumV2[MAX_SOURCE_SENS_COUNT];
	double				totalV2;
	long				sourceV2;
	long				sumI2[MAX_SOURCE_SENS_COUNT];
	double				totalI2;
	long				sourceI2;
	//kjg_logic_type_140325_e
} S_BD_SOURCE2;

typedef struct s_vi_command_tag {
	long				value;
	unsigned char		range;
	unsigned char		div;
	unsigned char		reserved1[2];
} S_VI_CMD;

#ifdef __COA_VER_100B__
typedef struct s_board_misc_tag {
	unsigned char		semiSwitchState;
	unsigned char		rangeV;
	unsigned char		rangeI;
	unsigned char		reserved1;

	long				refV;
	long				refI;
	long				avg_bd_Vsens;
	long				avg_bd_Isens;
	long				avg_bd_watt;

	double				Vsource_AD_a;
	double				Vsource_AD_b;
	double				Vsource_AD_a_N;
	double				Vsource_AD_b_N;
	double				Isource_AD_a;
	double				Isource_AD_b;
	double				Isource_AD_a_N;
	double				Isource_AD_b_N;

	S_BD_SOURCE			source[4];
	S_BD_SOURCE2		source2[4];
	S_VI_CMD			VICmd[MAX_TYPE];

	long				preVref; //kjg_110813_s
	long				preIref;
	unsigned int		ch_sens_count;

	unsigned char		ch_sens_count_flag;
	unsigned char		reserved2[3];

	unsigned int		faultChCnt;
	unsigned int		checkChOVP;
	unsigned long		runTime;
	long				BD_AD_Offset;

	unsigned int		checkChMinus; //kjg_110813_e
} S_BD_MISC;
#else //COA_VER_100B2~
typedef struct s_board_misc_tag {
	unsigned char		semiSwitchState;
	unsigned char		rangeV;
	unsigned char		rangeI;
	unsigned char		reserved1;

	long				refV;
	long				refI;
	long				avg_bd_Vsens;
	long				avg_bd_Isens;
	long				avg_bd_watt;

	double				Vsource_AD_a;
	double				Vsource_AD_b;
	double				Vsource_AD_a_N;
	double				Vsource_AD_b_N;
	double				Isource_AD_a;
	double				Isource_AD_b;
	double				Isource_AD_a_N;
	double				Isource_AD_b_N;

	//kjg_logic_type_140325_s
	double				Vsource2_AD_a;
	double				Vsource2_AD_b;
	double				Vsource2_AD_a_N;
	double				Vsource2_AD_b_N;
	double				Isource2_AD_a;
	double				Isource2_AD_b;
	double				Isource2_AD_a_N;
	double				Isource2_AD_b_N;
	//kjg_logic_type_140325_e
	S_BD_SOURCE			source[4];
	S_BD_SOURCE2		source2[4];
	S_VI_CMD			VICmd[MAX_TYPE];

	long				preVref; //kjg_110813_s
	long				preIref;
	unsigned int		ch_sens_count;

	unsigned char		ch_sens_count_flag;
	unsigned char		reserved2[3];

	unsigned int		faultChCnt;
	unsigned int		checkChOVP;
	unsigned long		runTime_day;
	unsigned long		runTime;
	long				BD_AD_Offset;

	unsigned int		checkChMinus; //kjg_110813_e
} S_BD_MISC;
#endif

typedef struct s_cali_main_dac_tag {
	unsigned char		sens_count_flag1;
	unsigned char		reserved1[3];

	unsigned short int	sens_count1;
	short int			reserved2;
	long				cmdV_offset[MAX_GROUP_8];
	long				adV_offset[MAX_GROUP_8];

	short int			sens_AD[MAX_MAIN_DAC_CH][MAX_MAIN_DAC_AD_COUNT];
	long				avgSens_AD1[MAX_MAIN_DAC_CH][MAX_MAIN_DAC_SENS_COUNT1];

	unsigned char		tmp_da_caliFlag[MAX_TYPE][MAX_RANGE];
	long				tmp_value[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	long				cal_value[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];

	long				tmp_cali_pointNum[MAX_TYPE][MAX_RANGE];
	long				tmp_cali_cmd[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	long				tmp_org_meter[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	long				tmp_cal_meter[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];

	double				tmp_DA_A[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	double				tmp_DA_B[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];

	long				cali_pointNum[MAX_TYPE][MAX_RANGE];
	long				cali_cmd[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	long				org_meter[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	long				cal_meter[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];

	double				DA_A[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
	double				DA_B[MAX_TYPE][MAX_RANGE][MAX_CALI_POINT];
} S_CALI_MAIN_DAC;

typedef struct s_board_data_tag {
	unsigned char		state;
	unsigned char		phase;
	unsigned char		reserved1[2];
	int					code;

   	unsigned char		signal[MAX_SIGNAL];
	unsigned char		cmd[MAX_CMD_MODE];

	unsigned char		outputSwitch;
	unsigned char		C_D_Select;
	unsigned char		readOtFault;
	unsigned char		readHwFault;

	//unsigned char		rangeSelectV[MAX_RANGE]; //kjg_110814
	//unsigned char		rangeSelectI[MAX_RANGE]; //kjg_110814
		
	S_BD_MISC			misc;

	S_AUTO_CALI_REFERENCE	auto_cali_ref[MAX_TYPE]; //kjg_110814_s
	S_CALI_MAIN_DAC		cali_main_dac;

	unsigned char		runRelayState[8];
	unsigned char		rangeRelayState[MAX_RANGE][8];
	unsigned char		rangeSelectV[MAX_RANGE][8];
	unsigned char		rangeSelectI[MAX_RANGE][8];
	unsigned char		readOTFault[8];
	unsigned char		hardwareFault[8];
	unsigned char		ssrState[8]; //kjg_110814_e
} S_BD_DATA;

typedef struct s_ch_grade_tag {
	unsigned char		item;
	unsigned char		totalGrade;
	unsigned char		reserved1[2];

	short int			clientStepNo;
	short int			groupStepNo;

	long				code[MAX_TEST_GRADE];
	long				value1[MAX_TEST_GRADE];
	long				value2[MAX_TEST_GRADE];
} S_CH_GRADE;

typedef struct s_ch_code_tag {
	long				compCount; //kjg_130509 unsigned char -> long

	unsigned char		compType;
	unsigned char		check_delay_count; //kjg_170810
	unsigned char		reserved1[2];

	short int			compIndex1; //jhkw_130319 unsigned char -> short int
	short int			compIndex2; //jhkw_130319 unsigned char -> short int
} S_CH_CODE;

typedef struct s_test_cond_reserved_tag {
	unsigned char		reserved_cmd;	//0:normal, 1:stop, 2:pause
	unsigned char		select_run;		//0:normal, 1:select_run
	unsigned char		reserved1[2];
	unsigned long		reserved_stepNo;
	unsigned long		reserved_cycleNo;
	unsigned long		select_stepNo;
	unsigned long		select_cycleNo;
	//unsigned long		select_advCycleStep; //jhkw_201127s
	short int			select_advCycleStep;
	short int			select_advLoopStep;	//jhkw_201127e
} S_TEST_COND_RESERVED;

typedef struct s_test_cond_pattern_data_tag {
	long				t_val;
	//long				cmd_val[2];
	long				cmd_val[4]; //kjhw_150914 pattern_cv

	unsigned char		waveform_type; //0:rectangle, 1:triangle
	unsigned char		reserved1[3];
} S_TEST_COND_PATTERN_DATA;

//20181219 KHK---------------------------------
typedef struct s_test_cond_charging_rpt_soc_set_data_tag {
	unsigned long	counter[16];
	unsigned long	endSOC[16];
	unsigned long	maxCounter;
	unsigned char	from_endsoc_to_rptsoc_set[16];
} S_TEST_COND_CHARGING_RPT_SOC_SET_DATA;

typedef struct s_user_define_mode_tag {
	long	ref;
	long	rptSOC;
	long	endSOC;
	unsigned char	org_mode;
	unsigned char	mode;
	unsigned char	soc_tracking_use;
	unsigned char	from_endsoc_to_rptsoc_set;

	unsigned long	charging_counter;
	unsigned long	charging_counter_rpt_soc;
} S_USER_DEFINE_MODE;

typedef struct s_test_cond_charging_count_set_data_tag {
	unsigned long	counter[16];
	long	refP[16];
	unsigned char	mode[16];
	unsigned char	soc_tracking_use[16];
	unsigned long	maxCounter;
} S_TEST_COND_CHARGING_COUNT_SET_DATA;

typedef struct s_test_cond_user_define_mode_data_tag {
	S_TEST_COND_CHARGING_RPT_SOC_SET_DATA	charging_rpt_soc_set;
	S_TEST_COND_CHARGING_COUNT_SET_DATA	charging_count_set;
} S_TEST_COND_USER_DEFINE_MODE_DATA;
//---------------------------------------------

//20181219 KHK---------------------------------
typedef struct s_test_cond_soc_tracking_data_tag {
	unsigned char		row;
	unsigned char		col;
	unsigned char		interpolation;
	unsigned char		reserved;
	unsigned short int				temp_num;
	unsigned short int				soc_num;
	long							maxI;
	long							minI;
	long				temp[MAX_SOC_TRACKING_DATA];
	long				SOC[MAX_SOC_TRACKING_DATA];
	long				limit_current[MAX_SOC_TRACKING_DATA][MAX_SOC_TRACKING_DATA];
	float				tracking_data_A[MAX_SOC_TRACKING_DATA][MAX_SOC_TRACKING_DATA];
	float				tracking_data_B[MAX_SOC_TRACKING_DATA][MAX_SOC_TRACKING_DATA];
} S_TEST_COND_SOC_TRACKING_DATA;
//---------------------------------------------
//jhkw_221205s
typedef struct s_test_cond_sequence_charge_data_tag {
	unsigned char		row;
	unsigned char		col;
	unsigned char		row2;
	unsigned char		row_num;
	unsigned char		col_num;
	unsigned char		row2_num;
	unsigned char		reserved[2];
	long				maxI;
	long				minI;
	long				ROW[MAX_SQ_ROW_DATA];
	long				COL[MAX_SQ_COL_DATA];
	long				div_voltage[MAX_SQ_ROW_DATA][MAX_SQ_COL_DATA];
	long				limit_current[MAX_SQ_ROW_DATA][MAX_SQ_COL_DATA];
} S_TEST_COND_SEQUENCE_CHARGE_DATA;
//jhkw_221205e

typedef struct s_test_cond_external_data_tag {
	long				cmd_v[2];
	long				cmd_i[2];
	long				cmd_p[2];
	long				t_val;
	long				receive_parsing_period;
	long				can_transmit_period; //kjg_140916

	unsigned char		version;		//1:default, 2:hmc
	unsigned char		mode;			//2:MODE_CC, 6:MODE_CP
	unsigned char		waveform_type;	//0:rectangle, 1:triangle
	unsigned char		control;	//0:IDLE, 1:RUN, 2:PAUSE, 3:CONTINUE, 4:END
} S_TEST_COND_EXTERNAL_DATA;

typedef struct s_test_cond_usermap_data_tag { //kjhw_140828
	long				t_val[2];
	long				cmd_val[2];

	long				table[MAX_USERMAP_ROW_DATA][MAX_USERMAP_COL_DATA];
	long				reserved1[3];
} S_TEST_COND_USERMAP_DATA;

typedef struct s_test_condition_tag {
	long				common_object[MAX_COM_TEST_OBJECT];
	long				local_object[MAX_TEST_STEP][MAX_LOC_TEST_OBJECT];

	S_CH_GRADE			grade[MAX_TEST_STEP];
	S_CH_CODE			common_chCode[MAX_COM_CODE];
	S_CH_CODE			local_chCode[MAX_TEST_STEP][MAX_LOC_CODE];
	S_CH_CODE			aux_chCode[MAX_AUX_CODE];
	S_CH_CODE			can_chCode[MAX_CAN_CODE];
	S_TEST_COND_RESERVED	reserved;
	S_TEST_COND_PATTERN_DATA	pattern[MAX_PATTERN_DATA];
	//S_TEST_COND_SOC_TRACKING_DATA	SOC_tracking; //20181219 KHK
	S_TEST_COND_SOC_TRACKING_DATA	SOC_tracking[2]; //jhkw_201102
	S_TEST_COND_SEQUENCE_CHARGE_DATA	SQ_Charge;	//jhkw_221205
	S_TEST_COND_USER_DEFINE_MODE_DATA user_define_mode; //20181219 KHK
	S_TEST_COND_EXTERNAL_DATA	external_data[2];
	S_TEST_COND_USERMAP_DATA	usermap; //kjhw_140828
	unsigned char		fault_can_check_flag; //kjhw_141208
	unsigned char		reserved1[3]; //kjhw_141208
} S_TEST_CONDITION;

typedef struct s_group_misc_tag {
	unsigned char		semiSwitchState;
	unsigned char		scan_end_step_flag; //for formation 400A, 450A
	unsigned char		cell_check_count;
	unsigned char		cell_check_result;

	unsigned char		long_rest_area1[MAX_CH_PER_MODULE];
	unsigned char		long_rest_area2[MAX_CH_PER_MODULE];

	short int			scan_ch[2];
	short int			scan_step[2];
	short int			tmp_scan_step[2];
	short int			scan_idxStepNo_offset[2];
	short int			total_ng_ch;
	short int			reserved1;

	unsigned long		long_rest_time1[MAX_CH_PER_MODULE];
	unsigned long		long_rest_time2[MAX_CH_PER_MODULE];

	int					chOffset;

	S_VI_CMD			VICmd[MAX_TYPE];
} S_GROUP_MISC;

typedef struct s_jig_temp_set_data_tag { //kjg_110713
	unsigned short int	curJigAvgTemp;
	unsigned short int	checkTempTime;

	unsigned short int	targetTemp;
	unsigned char		useJigTempSetData;
	unsigned char		reserved1;
} S_JIG_TEMP_SET_DATA;

#ifdef __COA_VER_100B__
typedef struct s_group_data_tag {
	unsigned char		state;
	unsigned char		phase;
	unsigned char		rangeV;
	unsigned char		rangeI;

	unsigned char		workMode;
	unsigned char		stepType;
	unsigned char		stepMode;
	unsigned char		attribute;

	unsigned char		group_control_mode;
	unsigned char		reserved1[3]; //kjg_171219

	int					code;
	int					stepNo;
	int					advStepNo;
	int					cycleNo;
	int					advCycleNo;
	int					start_stepNo;
	int					client_stepNo;
	unsigned long		runTime;
	unsigned long		saveDt; //kjg_w

	unsigned char		signal[MAX_SIGNAL];
	unsigned char		cmd[MAX_CMD_MODE];

	char				tray_id[BCR_SIZE];

	S_GROUP_MISC		misc;

	unsigned char		measureFlag; //kjg_110713_s
	unsigned char		selectMap;
	unsigned char		reserved1[2];

	unsigned short int	ch_sens_count;
	unsigned char		ch_sens_count_flag;
	unsigned char		reserved2;

	S_AUTO_CALI_REFERENCE	auto_cali_ref[MAX_TYPE];
	S_TEMP_DATA			temp[MAX_CH_256];
	S_JIG_TEMP_SET_DATA	jigTempSetData;
	S_CALI_MAIN_DAC		cali_main_dac; //kjg_110713_e
} S_GROUP_DATA;
#else //COA_VER_100B2~
typedef struct s_group_data_tag {
	unsigned char		state;
	unsigned char		phase;
	unsigned char		rangeV;
	unsigned char		rangeI;

	unsigned char		workMode;
	unsigned char		stepType;
	unsigned char		stepMode;
	unsigned char		attribute;

	unsigned char		group_control_mode;
	unsigned char		reserved0[3]; //kjg_171219

	int					code;
	int					stepNo;
	int					advStepNo;
	int					cycleNo;
	int					advCycleNo;
	int					start_stepNo;
	int					client_stepNo;
	unsigned long		runTime_day;
	unsigned long		runTime;
	unsigned long		saveDt; //kjg_w

	unsigned char		signal[MAX_SIGNAL];
	unsigned char		cmd[MAX_CMD_MODE];

	char				tray_id[BCR_SIZE];

	S_GROUP_MISC		misc;

	unsigned char		measureFlag; //kjg_110713_s
	unsigned char		selectMap;
	unsigned char		reserved1[2];

	unsigned short int	ch_sens_count;
	unsigned char		ch_sens_count_flag;
	unsigned char		reserved2;

	S_AUTO_CALI_REFERENCE	auto_cali_ref[MAX_TYPE];
	S_TEMP_DATA			temp[MAX_CH_256];
	S_JIG_TEMP_SET_DATA	jigTempSetData;
	S_CALI_MAIN_DAC		cali_main_dac; //kjg_110713_e
} S_GROUP_DATA;
#endif

typedef struct s_cable_check_tag {
	unsigned char		scan_ch;
	unsigned char		reserved1[3];

	unsigned char		code[MAX_CH_PER_MODULE];

	long				Vsens[MAX_CH_PER_MODULE][7];
	long				Isens[MAX_CH_PER_MODULE][7];
	long				v_power_bus[MAX_CH_PER_MODULE][7];
	long				v_output_bus[MAX_CH_PER_MODULE][7];
} S_CABLE_CHECK;

#ifdef __COA_VER_100B__
typedef struct s_module_misc_tag {
	long long			rt_periodic;

	unsigned short int	main_slot;
	short int			shift_slot;
	short int			scan_slot;
	short int			channel_slot;
	short int			reference_slot;
	short int			next_channel_slot;
	short int			next_reference_slot;
	short int			reserved1;

	int					processPointer;
	unsigned long		increment_period;
	unsigned long		timer_1sec;

	unsigned short int	source_sensCount;
	unsigned short int	SubSensV_SourceSensCount;
	unsigned short int	timer_1000ms;
	unsigned short int	SubSensV_ChSensCount;

	unsigned char		bd_sensCountFlag;
	unsigned char		source_sensCountFlag;
	unsigned char		SubSensV_SourceSensCountFlag;
	unsigned char		SubSensV_ChSensCountFlag;

	unsigned char		ad_muxVal;
	unsigned char		ref_muxVal;
	unsigned char		tmp_ref_muxVal;
	unsigned char		ch_muxVal;

	unsigned char		bd_sensCount;
	unsigned char		timer_1sec_count;
	unsigned char		timer_1000ms_count;
	unsigned char		module_type;

	long				fan_delay_time;
	long				fan_active_time;
	long				fan_fail_detect_time;
	long				fan_fail_detect_time2;
	long				fan_delay_time2;  //phb_221223 //shh_230607
	long				fan_run_time;
	long				fan_stop_time;
	long				meter_value;
	long				meter_wait_time;

	int					receivedBd;
	int					receivedCh;

	long				test_internal_r;

	S_CABLE_CHECK		cable_check;
	S_CABLE_CHECK		cell_check;

	unsigned long		cali_delay; //kjg_110813_s

	int					runCh;
	int					runBd;
	int					runGroup;
	long				measureMeterValue;
	unsigned long		meterRunTime;
	unsigned long		runTime;
	unsigned long		recordTime;

	S_AUTO_CALI_REFERENCE	auto_cali_ref[MAX_TYPE];
	S_VI_CMD			VICmd[MAX_TYPE];
	S_TEMP_DATA			temp[MAX_CH_256]; //kjg_110813_e
	
	unsigned char		dtc_data[2048];
	int					dtc_data_index;

	int					exit_value; //kjg_130127
	int					exit_value_2; //kjg_130205
} S_MODULE_MISC;
#else //COA_VER_100B2~
typedef struct s_module_misc_tag {
	long long			rt_periodic;

	unsigned short int	main_slot;
	short int			shift_slot;

	short int			scan_slot;
	short int			channel_slot;

	short int			reference_slot;
	short int			next_channel_slot;

	short int			next_reference_slot;
	unsigned char		can_ch;
	unsigned char		can_ch_check_data;

	int					processPointer;
	unsigned long		increment_period;
	unsigned long		timer_1sec;

	unsigned short int	timer_1000ms;
	unsigned short int	source_sensCount;
	unsigned short int	SubSensV_SourceSensCount;
	unsigned short int	SubSensV_ChSensCount;

	unsigned char		bd_sensCountFlag;
	unsigned char		source_sensCountFlag;
	unsigned char		SubSensV_SourceSensCountFlag;
	unsigned char		SubSensV_ChSensCountFlag;

	unsigned char		ad_muxVal;
	unsigned char		ref_muxVal;
	unsigned char		tmp_ref_muxVal;
	unsigned char		ch_muxVal;

	unsigned char		bd_sensCount;
	unsigned char		timer_1sec_count;
	unsigned char		timer_1000ms_count;
	unsigned char		module_type;

	unsigned long		fan_delay_time;
	unsigned long		fan_active_time;
	unsigned long		fan_fail_detect_time;
	unsigned long		fan_fail_detect_time2;
	unsigned long       fan_delay_time2;	//phb_221223 //shh_230607
	unsigned long		fan_run_time;
	unsigned long		fan_stop_time;
	//jhkw_180304s
	unsigned char		fan_active_flag_run;
	unsigned long		fan_run_count;
	unsigned long		fan_stop_count;
	//jhkw_180304e
	long				meter_value;
	long				meter_wait_time;

	int					receivedBd;
	int					receivedCh;

	long				test_internal_r;

	S_CABLE_CHECK		cable_check;
	S_CABLE_CHECK		cell_check;

	unsigned long		cali_delay; //kjg_110813_s

	int					runCh;
	int					runBd;
	int					runGroup;
	long				measureMeterValue;
	unsigned long		meterRunTime;
	unsigned long		runTime_day;
	unsigned long		runTime;
	unsigned long		recordTime;

	S_AUTO_CALI_REFERENCE	auto_cali_ref[MAX_TYPE];
	S_VI_CMD			VICmd[MAX_TYPE];
	S_TEMP_DATA			temp[MAX_CH_256]; //kjg_110813_e
	
	unsigned char		dtc_data[2048];
	int					dtc_data_index;

	unsigned long		tmp_inc_time_day; //kjg_121001_s
	unsigned long		tmp_inc_time;
	unsigned long		tmp_dec_time_day;
	unsigned long		tmp_dec_time; //kjg_121001_e

	int					exit_value; //kjg_130127
	int					exit_value_2; //kjg_130205
} S_MODULE_MISC;
#endif

typedef struct s_module_config_tag {
	int					installedBd;
	int					installedCh;
	int					chPerBd;

	int					chInGroup[MAX_GROUP_8];
	int					bdInGroup[MAX_GROUP_8];
	int					chInBd[MAX_BD_16];

	long				chCheckDelayTime;

	long				maxV[MAX_RANGE];
	long				minV[MAX_RANGE];
	long				maxI[MAX_RANGE];
	long				minI[MAX_RANGE];
	long				maxP[MAX_CH_4]; //kjhw_130903

	unsigned char		rangeV;
	unsigned char		rangeI;
	unsigned char		ratioV;	//0:uV, 1:mV, 2:V
	unsigned char		ratioI;	//0:uA, 1:mA, 2:A

	unsigned char		daq_type;
	unsigned char		ad_count;
	unsigned char		filter_ad_count;
	unsigned char		tmp_avg_count; //kjg_111006

	short int			dcr_t1; //kjg_111006
	unsigned short int	dcr_count1;
	short int			dcr_t2;
	unsigned short int	dcr_count2;

	float				adRatioV[MAX_RANGE];
	float				adRatioI[MAX_RANGE];
	float				daRatioV[MAX_RANGE];
	float				daRatioI[MAX_RANGE];
	float				shuntR[MAX_RANGE];

	long				scan_period;

	short int			installedTemp;
	short int			installedAuxV;
	short int			installedTH; //kjh_160610
	short int			installedCAN;
	short int			installedCOM;
	short int			reserved1; //kjg_171219

	unsigned char		soft_feedback1[4]; //step : cv, cc, cp, cr
	unsigned char		soft_feedback2[2]; //pattern : cc, cp
	unsigned char		step_0time_save; //lki_111111
	unsigned char		cable_check; //kjg_120323
	unsigned char		division_CAN; //kjhw_140620
	unsigned char		reserved[3]; //kjhw_140620 //kjg_171219
	long				out_precharging_V; //kjhw_150120
	//unsigned char		reserved[7]; //kjhw_140620
	
	unsigned char		ratioP;	//0:no, 1:mW, 2:W	//jhkw_231127s
	unsigned char		reserved3;
	unsigned char		caliV_rangeI;
	unsigned char		reserved4;
				
	short int			caliV_cmdI;	
	double				da_max;	
	double				main_amp;	//1.494	
	double				v_feed[MAX_RANGE];	
	double				i_feed[MAX_RANGE];	
	double				i_ref[MAX_RANGE];	//jhkw_231127e

	double				daRatioV_P[MAX_RANGE]; //kjg_110814_s
	double				daRatioV_N[MAX_RANGE];
	double				daOffsetV_P[MAX_RANGE];
	double				daOffsetV_N[MAX_RANGE];
	double				daRatioI_P[MAX_RANGE];
	double				daRatioI_N[MAX_RANGE];
	double				daOffsetI_P[MAX_RANGE];
	double				daOffsetI_N[MAX_RANGE];
	double				AD_GainV[MAX_RANGE];
	double				AD_GainI[MAX_RANGE];
	double				DA_GainV[MAX_RANGE];
	double				DA_GainI[MAX_RANGE];
	double				AD_CommGain[MAX_RANGE];
	double				auxRateV[MAX_RANGE];
	double				auxRateI[MAX_RANGE];
	double				resAuxDA_V[MAX_RANGE];
	double				resAuxDA_I[MAX_RANGE];
	long				auxDaV[MAX_RANGE];
	long				auxDaI[MAX_RANGE]; //kjg_110814_e
} S_MODULE_CONFIG;

typedef struct s_bms_eol_data_tag {
	short int			cvtn_id; //uds
	short int			reserved1;

	unsigned char		sw_version[4]; //uds

	unsigned char		HvBattHvil_D_Fault; //can
	unsigned char		DTC[3]; //uds

	float				module_delta_temp; //uds
	float				thermistor_sensor; //uds
	float				cell_delta_v; //uds

	//0:fail, 1:ok
	unsigned char		HvBattAuxCntct_D_Actl[4]; //aux_contactor can
	unsigned char		HvBattCntct_D_Actl[4]; //main_contactor can
	unsigned char		HvBattNeg_D_Actl[4]; //negative_contactor can

	//0:all off, 1:aux on, 2:aux & main on, 3:all off
	float				Cycler_Pack_V[4]; //cycler
	float				HvBatt_U_Actl[4]; //can
	float				DID_480D_Pack_V[4]; //uds
	float				Link_V[4]; //uds
	float				Charge_V[4]; //uds
	float				Pack2_V[4]; //uds
	float				Charger_Fuse_V[4]; //uds
	float				Elac_Fuse_V[4]; //uds

	float				isolation_resistance; //uds
	float				isolation_Va; //uds
	float				isolation_Vb; //uds

	unsigned char		DTC_List[256][4]; //uds
} S_BMS_EOL_DATA;

typedef struct s_bms_eol_data2_tag {
	char				pack_id[4][40]; //0:write id, 1:blank id(vbf write),
										//2:blank id(vbf end), 3:read id(check)
} S_BMS_EOL_DATA2;

typedef struct s_bms_eol_data3_tag {
	unsigned char		UBMS_State;
	unsigned char		HW_Version;
	unsigned char		SW_Version;
	unsigned char		UBMS_ID;

	unsigned char		reserved1[4];
	unsigned char		reserved2[4];

	int					cali_v[4];
	int					cell_v[4];
	int					temp;
	int					module_v;
	int					obd;

	unsigned char		cali_val[4];
	int					reserved3[3];
} S_BMS_EOL_DATA3;

typedef struct s_eol_data_tag {
	unsigned char		index;
	unsigned char		sub_index;
	short int			response; //0:ok, 1~:fail code

	long				long_value[4];
	char				string_value[256];
	float				float_value[4];
	unsigned char		byte_value[4];
} S_EOL_DATA;

typedef struct s_bms_frame_data_tag { //kjhw_130220s
    unsigned char       frame_number[4];

	unsigned char       flag;
    unsigned char       reserved1[3];

    unsigned char       Vmax_module_number_hi_nibble;
    unsigned char       Vmin_module_number_lo_nibble;
    unsigned char       Tmax_module_number_hi_nibble;
    unsigned char       Tmin_module_number_lo_nibble;

	short int           data[10];
} S_BMS_FRAME_DATA; //kjhw_130220e

typedef struct s_module_data_tag {
    unsigned char		state;
    unsigned char		phase;
	unsigned char		debug_op_mode; //0:normal, 1:ch11_select, 2:ch1_select
	unsigned char		reserved1;

    int					code;

	//mux_scan
	short int			total_scan_slot;
	short int			total_ch_slot;
	short int			total_ref_slot;
	short int			reserved2;
	short int			mux_scan[MAX_SLOT];

	unsigned char		signal[MAX_SIGNAL];
	unsigned char		cmd[MAX_CMD_MODE];
	
	S_MODULE_MISC		misc;
	S_MODULE_CONFIG		config;

	long				runningTime[6][100];
	long				real_time[7];

	unsigned char		measureState; //kjg_110813_s
	unsigned char		reserved3[3];

	S_CALI_MAIN_DAC		cali_main_dac;
	S_MODULE_TEMP		temp[MAX_GROUP_4]; //kjg_110813_e
	
	S_BMS_EOL_DATA		bms_eol_data;
	S_BMS_EOL_DATA2		bms_eol_data2;
	S_BMS_EOL_DATA3		bms_eol_data3;

	S_EOL_DATA			eol_data[MAX_EOL_DATA]; //kjg_120709

	S_BMS_FRAME_DATA    bms_frame_data; //kjhw_130220

	short int			fifo_data[2][4][7][4]; //kjg_logic_type_140325

	long				ratioV;	//jhkw_231127s
	long				ratioI;	
	double				ratioP;	
	double				cp_to_cc;	
	float				patt_ratioV;	
	float				patt_ratioI;	
	float				patt_ratioP;	//jhkw_231127e
} S_MODULE_DATA;

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
typedef struct s_aux_set_data_tag {
	unsigned char		chNo;			//1base : machine channel number
	unsigned char		reserved1[3];
	short int			auxChNo;		//1base : aux channel number
	short int			auxType;		//0:temperature, 1:v
	char				name[MAX_AUX_NAME_SIZE];
	long				fault_upper;
	long				fault_lower;
	long				end_upper;
	long				end_lower;
	
	short int			function_div;
	short int			function_div2;
	short int			function_div3;
	short int			reserved2;
} S_AUX_SET_DATA;
#else //COA_VER_100F~
//kjh_160610s
typedef struct s_aux_set_data_tag {
	unsigned char		chNo;			//1base : machine channel number
	//unsigned char		reserved1[3];
	unsigned char		tableNo;		//auxType=2 0:Non, 1:Atype, 2:Btype, 3:Ctype
	//unsigned char		reserved1[2];
	unsigned char		fix_safety_flag;
	unsigned char		reserved1;
	short int			auxChNo;		//1base : aux channel number
	short int			auxType;		//0:temperature, 1:v, 2:thermistor
	char				name[MAX_AUX_NAME_SIZE];
	long				fault_upper;
	long				fault_lower;
	long				end_upper;
	long				end_lower;
	
	short int			function_div;
	short int			function_div2;
	short int			function_div3;
	short int			reserved2;
} S_AUX_SET_DATA;
//kjh_160610e
#endif

typedef struct s_ch_attribute_tag {
	unsigned char		chNo_master;	//1base
	unsigned char		chNo_slave[MAX_SLAVE_CH];	//1base

	unsigned char		opType;				//0:independent, 1:parallel
	unsigned char		chamber_control;	//0:none, 1:pc_control

	//unsigned char		reserved1[2]; //kjhw_141201s
	unsigned char		can_comm_check; //0:non-check, 1:M+Scheck, 2:M, 3:S
	//unsigned char		reserved1;
	unsigned char		chamber_standby; //0:non-standby, 1:standby
	//unsigned char		reserved1[4]; //kjhw_141201e
	
	unsigned char		out_mux_use; //kjhw_151021s
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__ || defined __COA_VER_100F__
	unsigned char		reserved1[3]; //kjhw_151021e
#else //COA_VER_100F01~ //kjhw_170906
	unsigned char		chiller_control; //0:none, 1:pc_control //kjhw_170906
	unsigned char		reserved1[2];
#endif
} S_CH_ATTRIBUTE;

typedef struct s_th_table_tag {
	unsigned char		th_type;
	unsigned char		bias_type;
	short int			th_data_max_index;

	float				Vref; //mV
	float				R1; //ohm
	float				R2; //ohm

	float				T_R[MAX_TH_DATA][2]; //temp, ohm
	float				V_TH[MAX_TH_DATA]; //mV
} S_TH_TABLE;

typedef struct s_ch_temp_tag { //kjg_110712
	short				number1;
	short				number2;
	short				bd;
	short				ch;
} S_CH_TEMP;

#endif
