#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"
#include "local_message.h"

extern volatile S_SYSTEM_DATA *myData;
extern volatile S_APP_CONTROL *myPs;
extern volatile S_TEST_CONDITION *myTestCond;
extern char psName[PROCESS_NAME_SIZE];

const struct {
	const char *name;
	in_function_no	no;
} DIO_IN_NAME[] = {
	{"I_SPARE", I_SPARE},

	{"I_POWER_SWITCH", I_POWER_SWITCH},
	{"I_EMG_SWITCH", I_EMG_SWITCH},
	{"I_AC_POWER_FAIL", I_AC_POWER_FAIL},
	{"I_UPS_BATTERY_FAIL", I_UPS_BATTERY_FAIL},
	{"I_NO_POWER_OFF", I_NO_POWER_OFF},
	{"I_PS_FAIL", I_PS_FAIL},
	{"I_FAN_ERROR", I_FAN_ERROR},	///phb_221223 //shh_230607
	//{"I_FAN_FAIL", I_FAN_FAIL},

	{"I_FUSE_FAIL", I_FUSE_FAIL},
	{"I_MAIN_MC", I_MAIN_MC},
	{"I_SUB_MC", I_SUB_MC},
	{"I_INVERTER_HV_FAULT", I_INVERTER_HV_FAULT},
	{"I_INVERTER_MODULE_R_FAIL", I_INVERTER_MODULE_R_FAIL},
	{"I_INVERTER_MODULE_S_FAIL", I_INVERTER_MODULE_S_FAIL},
	{"I_INVERTER_MODULE_T_FAIL", I_INVERTER_MODULE_T_FAIL},
	{"I_INVERTER_PRE_CHARGE", I_INVERTER_PRE_CHARGE},
	{"I_INVERTER_OT", I_INVERTER_OT},
	{"I_INVERTER_RUN", I_INVERTER_RUN},
	{"I_POWER_TRANS_OT", I_POWER_TRANS_OT},
	{"I_POWER_REACTOR_OT", I_POWER_REACTOR_OT},
	{"I_MCCB_FAIL", I_MCCB_FAIL},
	{"I_INVERTER_OC", I_INVERTER_OC},
	{"I_INVERTER_ETC", I_INVERTER_ETC},

	{"I_PS1_FAIL", I_PS1_FAIL},
	{"I_PS2_FAIL", I_PS2_FAIL},
	{"I_PS3_FAIL", I_PS3_FAIL},
	{"I_PS4_FAIL", I_PS4_FAIL},
	{"I_PS5_FAIL", I_PS5_FAIL},
	{"I_PS6_FAIL", I_PS6_FAIL},
	{"I_PS7_FAIL", I_PS7_FAIL},
	{"I_PS8_FAIL", I_PS8_FAIL},
	{"I_PS9_FAIL", I_PS9_FAIL},
	{"I_PS10_FAIL", I_PS10_FAIL},
	{"I_PS11_FAIL", I_PS11_FAIL},
	{"I_PS12_FAIL", I_PS12_FAIL},
	{"I_PS13_FAIL", I_PS13_FAIL},
	{"I_PS14_FAIL", I_PS14_FAIL},
	{"I_PS15_FAIL", I_PS15_FAIL},
	{"I_PS16_FAIL", I_PS16_FAIL},
	{"I_OT1_FAIL", I_OT1_FAIL},
	{"I_OT2_FAIL", I_OT2_FAIL},
	{"I_OT3_FAIL", I_OT3_FAIL},
	{"I_OT4_FAIL", I_OT4_FAIL},
	{"I_OT5_FAIL", I_OT5_FAIL},
	{"I_OT6_FAIL", I_OT6_FAIL},
	{"I_OT7_FAIL", I_OT7_FAIL},
	{"I_OT8_FAIL", I_OT8_FAIL},
	{"I_OT9_FAIL", I_OT9_FAIL},
	{"I_OT10_FAIL", I_OT10_FAIL},
	{"I_OT11_FAIL", I_OT11_FAIL},
	{"I_OT12_FAIL", I_OT12_FAIL},
	{"I_OT13_FAIL", I_OT13_FAIL},
	{"I_OT14_FAIL", I_OT14_FAIL},
	{"I_OT15_FAIL", I_OT15_FAIL},
	{"I_OT16_FAIL", I_OT16_FAIL},

	{"I_SUB_EMG", I_SUB_EMG},
	{"I_CONVERTER_OV", I_CONVERTER_OV},
	{"I_CONVERTER_OC", I_CONVERTER_OC},
	{"I_CONVERTER_DCOV", I_CONVERTER_DCOV},
	{"I_CONVERTER_MODULE_FAULT", I_CONVERTER_MODULE_FAULT},
	{"I_CONVERTER_OC_2", I_CONVERTER_OC_2}, //jhkw_170917s
	{"I_CONVERTER_OC_3", I_CONVERTER_OC_3},
	{"I_CONVERTER_OC_4", I_CONVERTER_OC_4},
	{"I_CONVERTER_DCOV_2", I_CONVERTER_DCOV_2},
	{"I_CONVERTER_MODULE_F_2", I_CONVERTER_MODULE_F_2},
	{"I_CONVERTER_MODULE_F_3", I_CONVERTER_MODULE_F_3},
	{"I_CONVERTER_MODULE_F_4", I_CONVERTER_MODULE_F_4}, //jhkw_170917e
	{"I_CONVERTER_OT", I_CONVERTER_OT},
	{"I_CONVERTER_IN_FUSE_FAIL", I_CONVERTER_IN_FUSE_FAIL},
	{"I_RANGE_FAULT", I_RANGE_FAULT},	//jhkw_130416

	{"I_I_RANGE1", I_I_RANGE1},
	{"I_I_RANGE2", I_I_RANGE2},
	{"I_I_RANGE3", I_I_RANGE3},
	{"I_I_RANGE4", I_I_RANGE4},
	{"I_PARALLEL", I_PARALLEL},
	{"I_PARALLEL_SUB", I_PARALLEL_SUB}, //kjhw_150120
	{"I_RIPPLE_SWITCH", I_RIPPLE_SWITCH},
	{"I_MODE_CV_DETECT", I_MODE_CV_DETECT},
	{"I_OUT_FUSE_FAIL", I_OUT_FUSE_FAIL},
	{"I_OUT_REACTOR_OT", I_OUT_REACTOR_OT},
	{"I_OUT_PACK_ISOLATION", I_OUT_PACK_ISOLATION},
	//jhkw_150224s
	{"I_OUT_PACK_ISOLATION2", I_OUT_PACK_ISOLATION2},
	{"I_OUT_PACK_ISOLATION3", I_OUT_PACK_ISOLATION3},
	{"I_OUT_PACK_ISOLATION4", I_OUT_PACK_ISOLATION4},
	{"I_OUT_PACK_ISOLATION5", I_OUT_PACK_ISOLATION5},
	{"I_OUT_PACK_ISOLATION6", I_OUT_PACK_ISOLATION6},
	//jhkw_150224e
	{"I_RUN_RELAY", I_RUN_RELAY}, //kjhw_150120
	{"I_RUN_RELAY_SUB", I_RUN_RELAY_SUB}, //kjhw_150120

	{"I_CABLE_FAIL", I_CABLE_FAIL},

	{"I_EXTERNAL1", I_EXTERNAL1},
	{"I_EXTERNAL2", I_EXTERNAL2},
	{"I_EXTERNAL3", I_EXTERNAL3},
	{"I_EXTERNAL4", I_EXTERNAL4},
	{"I_EXTERNAL5", I_EXTERNAL5},
	{"I_EXTERNAL6", I_EXTERNAL6},
	{"I_EXTERNAL7", I_EXTERNAL7},
	{"I_EXTERNAL8", I_EXTERNAL8},

	{"I_CHARGER_WAKEUP", I_CHARGER_WAKEUP},
	{"I_PACK_SIGNAL", I_PACK_SIGNAL},

	{"I_JIG_LATCH_CYLINDER_L_ON_OFF", I_JIG_LATCH_CYLINDER_L_ON_OFF},
	{"I_JIG_LATCH_CYLINDER_L_ON", I_JIG_LATCH_CYLINDER_L_ON},
	{"I_JIG_LATCH_CYLINDER_L_OFF", I_JIG_LATCH_CYLINDER_L_OFF},
	{"I_JIG_LATCH_CYLINDER_R_ON_OFF", I_JIG_LATCH_CYLINDER_R_ON_OFF},
	{"I_JIG_LATCH_CYLINDER_R_ON", I_JIG_LATCH_CYLINDER_R_ON},
	{"I_JIG_LATCH_CYLINDER_R_OFF", I_JIG_LATCH_CYLINDER_R_OFF},
	{"I_JIG_MAIN_CYLINDER_L_ON_OFF", I_JIG_MAIN_CYLINDER_L_ON_OFF},
	{"I_JIG_MAIN_CYLINDER_L_ON", I_JIG_MAIN_CYLINDER_L_ON},
	{"I_JIG_MAIN_CYLINDER_L_OFF", I_JIG_MAIN_CYLINDER_L_OFF},
	{"I_JIG_MAIN_CYLINDER_R_ON_OFF", I_JIG_MAIN_CYLINDER_R_ON_OFF},
	{"I_JIG_MAIN_CYLINDER_R_ON", I_JIG_MAIN_CYLINDER_R_ON},
	{"I_JIG_MAIN_CYLINDER_R_OFF", I_JIG_MAIN_CYLINDER_R_OFF},
	{"I_JIG_GRIP_CYLINDER_L_ON_OFF", I_JIG_GRIP_CYLINDER_L_ON_OFF},
	{"I_JIG_GRIP_CYLINDER_L_ON", I_JIG_GRIP_CYLINDER_L_ON},
	{"I_JIG_GRIP_CYLINDER_L_OFF", I_JIG_GRIP_CYLINDER_L_OFF},
	{"I_JIG_GRIP_CYLINDER_R_ON_OFF", I_JIG_GRIP_CYLINDER_R_ON_OFF},
	{"I_JIG_GRIP_CYLINDER_R_ON", I_JIG_GRIP_CYLINDER_R_ON},
	{"I_JIG_GRIP_CYLINDER_R_OFF", I_JIG_GRIP_CYLINDER_R_OFF},
	{"I_JIG_STOPPER_STATE", I_JIG_STOPPER_STATE},
	{"I_JIG_TRAY_STATE", I_JIG_TRAY_STATE},
	{"I_JIG_AIR_PRESS", I_JIG_AIR_PRESS},
	{"I_JIG_SW_DOWN", I_JIG_SW_DOWN},
	{"I_JIG_SW_UP", I_JIG_SW_UP},
	{"I_JIG_SW_START", I_JIG_SW_START},
	{"I_JIG_SW_STOP", I_JIG_SW_STOP},
	{"I_JIG_SW_EMG", I_JIG_SW_EMG},
	{"I_JIG_SMOKE_SENS", I_JIG_SMOKE_SENS},
	{"I_JIG_FIRE_SENS", I_JIG_FIRE_SENS},
	{"I_JIG_OT_SENS", I_JIG_OT_SENS},
	{"I_JIG_STACKER_STATE", I_JIG_STACKER_STATE},
	{"I_JIG_ROLL_L_F_ON", I_JIG_ROLL_L_F_ON},
	{"I_JIG_ROLL_L_F_OFF", I_JIG_ROLL_L_F_OFF},
	{"I_JIG_ROLL_L_R_ON", I_JIG_ROLL_L_R_ON},
	{"I_JIG_ROLL_L_R_OFF", I_JIG_ROLL_L_R_OFF},
	{"I_JIG_ROLL_R_F_ON", I_JIG_ROLL_R_F_ON},
	{"I_JIG_ROLL_R_F_OFF", I_JIG_ROLL_R_F_OFF},
	{"I_JIG_ROLL_R_R_ON", I_JIG_ROLL_R_R_ON},
	{"I_JIG_ROLL_R_R_OFF", I_JIG_ROLL_R_R_OFF},
	{"I_JIG_CHAMBER_FIRE", I_JIG_CHAMBER_FIRE},
	{"I_JIG_CHAMBER_SMOKE", I_JIG_CHAMBER_SMOKE},
	{"I_JIG_CHAMBER_DOOR_OPEN", I_JIG_CHAMBER_DOOR_OPEN},
	{"I_JIG_CHAMBER_OVER_TEMP", I_JIG_CHAMBER_OVER_TEMP},
	{"I_JIG_CHAMBER_AIR_PRESS", I_JIG_CHAMBER_AIR_PRESS},
	{"I_JIG_CHAMBER_EMG", I_JIG_CHAMBER_EMG},
	{"I_JIG_CHAMBER_EOCR", I_JIG_CHAMBER_EOCR},
	{"I_JIG_CHAMBER_FAN", I_JIG_CHAMBER_FAN},
	{"I_JIG_CHAMBER_RUN_MODE", I_JIG_CHAMBER_RUN_MODE},
	{"I_JIG_CHAMBER_POWER", I_JIG_CHAMBER_POWER},
	{"I_JIG_CHAMBER_REF_ERROR", I_JIG_CHAMBER_REF_ERROR},
	{"I_JIG_CHAMBER_CP_TRIP", I_JIG_CHAMBER_CP_TRIP},
	{"I_JIG_CHAMBER_LEAK", I_JIG_CHAMBER_LEAK},
	{"I_JIG_CHAMBER_ETC", I_JIG_CHAMBER_ETC},	//jhkw_191216
	{"I_JIG_EXT_OT", I_JIG_EXT_OT},	//jhkw_160219
	{"I_JIG_CHANGE_X", I_JIG_CHANGE_X},
	{"I_JIG_CHANGE_Y", I_JIG_CHANGE_Y},
	{"I_JIG_CHANGE_Z", I_JIG_CHANGE_Z},
	{"I_JIG_CHANGE_LOCK", I_JIG_CHANGE_LOCK},
	{"I_JIG_PITCH", I_JIG_PITCH},

	{"I_MOTOR", I_MOTOR},
	{"I_MOTOR_RUN", I_MOTOR_RUN},
	{"I_MOTOR_ERROR", I_MOTOR_ERROR},
	{"I_MOTOR_READY", I_MOTOR_READY},
	{"I_MOTOR_HOME", I_MOTOR_HOME},

	{"I_REMOTE_CONTROL", I_REMOTE_CONTROL},
	{"I_CHAMBER_FAULT", I_CHAMBER_FAULT},

	{"I_OUT_CONCENT1", I_OUT_CONCENT1}, //jhkw_130319s
	{"I_OUT_CONCENT2", I_OUT_CONCENT2},
	{"I_OUT_CONCENT3", I_OUT_CONCENT3},
	{"I_OUT_CONCENT4", I_OUT_CONCENT4},  //jhkw_130319e
	{"I_OUT_CONCENT5", I_OUT_CONCENT5},
	{"I_OUT_CONCENT6", I_OUT_CONCENT6},
	{"I_OUT_CONCENT7", I_OUT_CONCENT7}, //jhkw_180206
	{"I_OUT_CONCENT8", I_OUT_CONCENT8},	//jhkw_180206

	{"I_MUX_A_P", I_MUX_A_P},  //kjhw_151021s
	{"I_MUX_A_N", I_MUX_A_N},
	{"I_MUX_B_P", I_MUX_B_P},
	{"I_MUX_B_N", I_MUX_B_N},	//kjhw_151021e

	{"I_TEST_01", I_TEST_01},  //kjhw_150401
	{"I_TEST_02", I_TEST_02},  //kjhw_150401
	{"I_TEST_03", I_TEST_03},  //kjhw_150401
	{"I_TEST_04", I_TEST_04},  //kjhw_150401

	{"I_FAN_MC_FAIL", I_FAN_MC_FAIL},  //phb_221221 //shh_230607
	{"I_DC_LINK_FUSE_FAIL", I_DC_LINK_FUSE_FAIL},  //phb_230710
};

const struct {
	const char *name;
	out_function_no	no;
} DIO_OUT_NAME[] = {
	{"O_SPARE", O_SPARE},

	{"O_LOGIC_TYPE", O_LOGIC_TYPE}, //kjg_logic_type_140324
	{"O_RUN_LED", O_RUN_LED},
	{"O_POWER_OFF", O_POWER_OFF},
	{"O_FORCE_POWER_OFF", O_FORCE_POWER_OFF},

	{"O_PS1", O_PS1},
	{"O_PS2", O_PS2},
	{"O_PS3", O_PS3},
	{"O_PS4", O_PS4},
	{"O_PS5", O_PS5},
	{"O_PS6", O_PS6},
	{"O_PS7", O_PS7},
	{"O_PS8", O_PS8},
	{"O_FAN1", O_FAN1},
	{"O_FAN2", O_FAN2},
	{"O_FAN3", O_FAN3},
	{"O_FAN4", O_FAN4},
	{"O_FAN5", O_FAN5},
	{"O_FAN6", O_FAN6},
	{"O_FAN7", O_FAN7},
	{"O_FAN8", O_FAN8},

	{"O_MAIN_MC", O_MAIN_MC},
	{"O_SUB_MC", O_SUB_MC},
	{"O_INVERTER_RUN", O_INVERTER_RUN},
	{"O_INVERTER_RUN2", O_INVERTER_RUN2}, //jhkw_160617
	{"O_INVERTER_RESET", O_INVERTER_RESET},
	{"O_MAIN_MC_IN", O_MAIN_MC_IN},	//jhkw_131209
	{"O_SUB_MC_IN", O_SUB_MC_IN},	//jhkw_131209

	{"O_RUN_RELAY", O_RUN_RELAY},
	{"O_RUN_RELAY_SUB", O_RUN_RELAY_SUB}, //kjhw_150120
	{"O_V_RANGE1", O_V_RANGE1},
	{"O_V_RANGE2", O_V_RANGE2},
	{"O_V_RANGE3", O_V_RANGE3},
	{"O_V_RANGE4", O_V_RANGE4},
	{"O_I_RANGE1", O_I_RANGE1},
	{"O_I_RANGE2", O_I_RANGE2},
	{"O_I_RANGE3", O_I_RANGE3},
	{"O_I_RANGE4", O_I_RANGE4},
	{"O_PARALLEL", O_PARALLEL},
	{"O_PARALLEL_SUB", O_PARALLEL_SUB}, //kjhw_150120
	{"O_PARALLEL_BD", O_PARALLEL_BD}, //kjhw_170630
	{"O_LAMP_CHARGE", O_LAMP_CHARGE},
	{"O_LAMP_DISCHARGE", O_LAMP_DISCHARGE},
	{"O_LAMP_RUN", O_LAMP_RUN},

	{"O_CONVERTER_RUN", O_CONVERTER_RUN},
	{"O_CONVERTER_RESET", O_CONVERTER_RESET},
	{"O_CONVERTER_CHARGE", O_CONVERTER_CHARGE},
	{"O_CONVERTER_DISCHARGE", O_CONVERTER_DISCHARGE},
	{"O_SELECT_VO_VS", O_SELECT_VO_VS},
		//O_SELECT_VO_VS : C_SBL_150V_250A_10A_38KW, 75KW_4, 75KW_5 -> H/L 0
	{"O_SELECT_VREF", O_SELECT_VREF}, //kjhw_120504
	{"O_SELECT_V_CALI_PI", O_SELECT_V_CALI_PI}, //kjhw_130118
	{"O_LAMP_RIPPLE_ON", O_LAMP_RIPPLE_ON},
	{"O_RIPPLE_ON", O_RIPPLE_ON},
	{"O_CONVERTER_FAIL", O_CONVERTER_FAIL},
	{"O_OUT_PRE_CHARGE", O_OUT_PRE_CHARGE},
	{"O_OUT_CAPACITOR", O_OUT_CAPACITOR},
	{"O_OUT_PACK_ISOLATION", O_OUT_PACK_ISOLATION},
	//jhkw_150224s
	{"O_OUT_PACK_ISOLATION2", O_OUT_PACK_ISOLATION2},
	{"O_OUT_PACK_ISOLATION3", O_OUT_PACK_ISOLATION3},
	{"O_OUT_PACK_ISOLATION4", O_OUT_PACK_ISOLATION4},
	{"O_OUT_PACK_ISOLATION5", O_OUT_PACK_ISOLATION5},
	{"O_OUT_PACK_ISOLATION6", O_OUT_PACK_ISOLATION6},
	//jhkw_150224e
	{"O_OUT_VS_ISOLATION", O_OUT_VS_ISOLATION}, //lki_111010_s
	{"O_OUT_VP_ISOLATION", O_OUT_VP_ISOLATION},
	{"O_OUT_VB_ISOLATION", O_OUT_VB_ISOLATION},
	{"O_OUT_VIN_ISOLATION", O_OUT_VIN_ISOLATION}, //lki_111010_e

	{"O_EXTERNAL1", O_EXTERNAL1},
	{"O_EXTERNAL2", O_EXTERNAL2},
	{"O_EXTERNAL3", O_EXTERNAL3},
	{"O_EXTERNAL4", O_EXTERNAL4},
	{"O_EXTERNAL5", O_EXTERNAL5},
	{"O_EXTERNAL6", O_EXTERNAL6},
	{"O_EXTERNAL7", O_EXTERNAL7},
	{"O_EXTERNAL8", O_EXTERNAL8},

	{"O_PACK_SIGNAL", O_PACK_SIGNAL},

	{"O_TOWER_LAMP_RED", O_TOWER_LAMP_RED},
	{"O_TOWER_LAMP_AMBER", O_TOWER_LAMP_AMBER},
	{"O_TOWER_LAMP_GREEN", O_TOWER_LAMP_GREEN},
	{"O_TOWER_LAMP_BLUE", O_TOWER_LAMP_BLUE},
	{"O_TOWER_LAMP_BUZZER", O_TOWER_LAMP_BUZZER},
	{"O_LED_RED", O_LED_RED},
	{"O_LED_AMBER", O_LED_AMBER},
	{"O_LED_GREEN", O_LED_GREEN},

	{"O_CALI_CHARGE_RELAY", O_CALI_CHARGE_RELAY},
	{"O_CALI_DISCHARGE_RELAY", O_CALI_DISCHARGE_RELAY},
	{"O_CALI_SW_1", O_CALI_SW_1},
	{"O_CALI_SW_2", O_CALI_SW_2},
	{"O_CALI_SW_3", O_CALI_SW_3},
	{"O_CALI_SW_4", O_CALI_SW_4},
	{"O_CALI_SW_5", O_CALI_SW_5},
	{"O_CALI_SW_6", O_CALI_SW_6},
	{"O_CALI_SW_7", O_CALI_SW_7},
	{"O_CALI_SW_8", O_CALI_SW_8},
	{"O_CALI_VP_VB", O_CALI_VP_VB},
	{"O_CALI_MODE", O_CALI_MODE},	//jhkw_130121

	{"O_JIG_FAN_RELAY", O_JIG_FAN_RELAY},
	{"O_JIG_LATCH_CYLINDER_ON_OFF", O_JIG_LATCH_CYLINDER_ON_OFF},
	{"O_JIG_LATCH_CYLINDER_ON", O_JIG_LATCH_CYLINDER_ON},
	{"O_JIG_LATCH_CYLINDER_OFF", O_JIG_LATCH_CYLINDER_OFF},
	{"O_JIG_MAIN_CYLINDER_ON_OFF", O_JIG_MAIN_CYLINDER_ON_OFF},
	{"O_JIG_MAIN_CYLINDER_ON", O_JIG_MAIN_CYLINDER_ON},
	{"O_JIG_MAIN_CYLINDER_OFF", O_JIG_MAIN_CYLINDER_OFF},
	{"O_JIG_GRIP_CYLINDER_ON_OFF", O_JIG_GRIP_CYLINDER_ON_OFF},
	{"O_JIG_GRIP_CYLINDER_ON", O_JIG_GRIP_CYLINDER_ON},
	{"O_JIG_GRIP_CYLINDER_OFF", O_JIG_GRIP_CYLINDER_OFF},
	{"O_JIG_TRAY_STATE", O_JIG_TRAY_STATE},
	{"O_JIG_STACKER_SIGNAL", O_JIG_STACKER_SIGNAL},
	{"O_JIG_ROLL_CYLINDER_ON_OFF", O_JIG_ROLL_CYLINDER_ON_OFF},
	{"O_JIG_CHANGE_X", O_JIG_CHANGE_X},
	{"O_JIG_CHANGE_Y", O_JIG_CHANGE_Y},
	{"O_JIG_CHANGE_Z", O_JIG_CHANGE_Z},
	{"O_JIG_CHANGE_LOCK", O_JIG_CHANGE_LOCK},

	{"O_CABLE_CHECK_CH", O_CABLE_CHECK_CH},
	{"O_CABLE_CHECK_SR", O_CABLE_CHECK_SR},
	{"O_CABLE_CHECK_UPPER", O_CABLE_CHECK_UPPER},
	{"O_CABLE_CHECK_LOWER", O_CABLE_CHECK_LOWER},

	{"O_MOTOR", O_MOTOR},
	{"O_MOTOR_RUN", O_MOTOR_RUN},
	{"O_MOTOR_STOP", O_MOTOR_STOP},
	{"O_MOTOR_RESET", O_MOTOR_RESET},
	{"O_MOTOR_HOME", O_MOTOR_HOME},
	{"O_MOTOR_STEP", O_MOTOR_STEP},

	{"O_REMOTE_CONTROL", O_REMOTE_CONTROL},
	{"O_CHAMBER_STOP", O_CHAMBER_STOP},

	{"O_CH_RUN_STATE", O_CH_RUN_STATE}, //kjh_130702

	{"O_BMS_KEY_ON", O_BMS_KEY_ON}, //kjg_101124
	{"O_BMS_CHARGE_ON", O_BMS_CHARGE_ON},
	{"O_BMS_RELAY_ON", O_BMS_RELAY_ON},

	{"O_MARS_LIGHT", O_MARS_LIGHT},	//jhkw_130711

	{"O_OUT_CONCENT1", O_OUT_CONCENT1}, //jhkw_130319s
	//kjh_170808 SK120 ch1 12V
	{"O_OUT_CONCENT2", O_OUT_CONCENT2},
	//kjh_170808 SK120 ch2 12V
	{"O_OUT_CONCENT3", O_OUT_CONCENT3},
	//kjh_170808 SK120 ch1 24V
	{"O_OUT_CONCENT4", O_OUT_CONCENT4},  //jhkw_130319e
	//kjh_170808 SK120 ch2 24V
	{"O_OUT_CONCENT5", O_OUT_CONCENT5},
	{"O_OUT_CONCENT6", O_OUT_CONCENT6},
	{"O_OUT_CONCENT7", O_OUT_CONCENT7},	//jhkw_180206
	{"O_OUT_CONCENT8", O_OUT_CONCENT8},	//jhkw_180206
	
	{"O_OUT_DC_FAN", O_OUT_DC_FAN},  //jhkw_130924
	{"O_OUT_PARALLEL_MODE", O_OUT_PARALLEL_MODE},  //jhkw_130924

	{"O_MUX_A_P", O_MUX_A_P},  //kjhw_151021s
	{"O_MUX_A_N", O_MUX_A_N},
	{"O_MUX_B_P", O_MUX_B_P},
	{"O_MUX_B_N", O_MUX_B_N},	//kjhw_151021e

	{"O_TEST_01", O_TEST_01},  //kjhw_150401
	{"O_TEST_02", O_TEST_02},  //kjhw_150401
	{"O_TEST_03", O_TEST_03},  //kjhw_150401
	{"O_TEST_04", O_TEST_04}   //kjhw_150401
};

int SystemLoader(int argc, char *argv[])
{
	int	rtn;

	if(argc != 2) {
		printf("System loader start fail %d\n", argc);
		return -1;
	}

	rtn = Initialize(atoi(argv[1]));
	if(rtn < 0) {
		if(rtn <= -21) {
			userlog(DEBUG_LOG, psName, "System initialize fail %d\n", rtn);
		} else printf("System initialize fail %d\n", rtn);

		if(rtn < -1) {
			Close_SystemMemory();
			system("rmmod mbuff");
		}
		return -2;
	} else {
		userlog(DEBUG_LOG, psName, "System initialize complete\n");
		return 0;
	}
}

int Initialize(int startType)
{
	int rtn;
	S_MSG_VAL SendMsg;

	rtn = Init_SystemData();
	if(rtn < 0) {
		return rtn;
	} else {
		printf("Init_SystemData complete\n");
	}

	rtn = Read_ConfigFiles(startType);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_ConfigFiles fail %d\n", rtn);
		if(rtn == -1) return -11;
		else return -21;
	}

	Init_SystemMemory_2();
	userlog(DEBUG_LOG, psName, "Init_SystemMemory_2 complete\n");

	if(Load_Process_All() < 0) {
		Close_Process_All();
		return -31;
	} else {
		userlog(DEBUG_LOG, psName, "Load_Process_All complete\n");
	}

	if(myData->AppControl.misc.Load_Process_CAN[0] == P1) {
		sleep(2);

		for(rtn=0; rtn < myData->mData.config.installedCAN; rtn++) {
			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_APP_MODULE_CAN_ABORT_TRANSMISSION;
			SendMsg.val[0] = rtn;
			send_msg(APP_TO_MODULE, (char *)&SendMsg);

			memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
			SendMsg.msg = MSG_APP_MODULE_CAN_BAUDRATE_SET;
			SendMsg.val[0] = rtn;
			switch(myData->mData.config.division_CAN) { //kjhw_140620
				case 1: //1main_ch for 1can_ch
					switch(myData->canReceiveSetData.commonData[rtn][0]
						.can_baudrate) {
						case 0: //125K
							SendMsg.val[1] = 125000;
							break;
						case 1: //250K
							SendMsg.val[1] = 250000;
							break;
						case 2: //500K
							SendMsg.val[1] = 500000;
							break;
						case 3: //1M
							SendMsg.val[1] = 1000000;
							break;
						default: //User
							SendMsg.val[1] = 500000; //kjg_180405 0->500000
							break;
					}
					SendMsg.val[2] = (int)myData->canReceiveSetData
						.commonData[rtn][0].extended_id;
					SendMsg.val[3] = (int)myData->canReceiveSetData
						.commonData[rtn][0].sjw;
#ifdef __CAN_FD__ //jhkw_190714s
					SendMsg.val[4] = (int)myData->canReceiveSetData
						.commonData[rtn][0].can_fd_flag;
					SendMsg.val[5] = (int)myData->canReceiveSetData
						.commonData[rtn][0].can_datarate;
					SendMsg.val[6] = (int)myData->canReceiveSetData
						.commonData[rtn][0].crc_type;
					SendMsg.val[7] = (int)myData->canReceiveSetData
						.commonData[rtn][0].terminal_r;
#endif //jhkw_190714e
					send_msg(APP_TO_MODULE, (char *)&SendMsg);

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_APP_MODULE_CAN_DATA_CLEAR;
					SendMsg.val[0] = rtn;
					send_msg(APP_TO_MODULE, (char *)&SendMsg);
					break;
				default:
					switch(myData->canReceiveSetData.commonData[rtn/2][rtn%2]
						.can_baudrate) {
						case 0: //125K
							SendMsg.val[1] = 125000;
							break;
						case 1: //250K
							SendMsg.val[1] = 250000;
							break;
						case 2: //500K
							SendMsg.val[1] = 500000;
							break;
						case 3: //1M
							SendMsg.val[1] = 1000000;
							break;
						default: //User
							SendMsg.val[1] = 500000; //kjg_180405 0->500000
							break;
					}
					SendMsg.val[2] = (int)myData->canReceiveSetData
						.commonData[rtn/2][rtn%2].extended_id;
					SendMsg.val[3] = (int)myData->canReceiveSetData
						.commonData[rtn/2][rtn%2].sjw;
#ifdef __CAN_FD__ //jhkw_190714s
					SendMsg.val[4] = (int)myData->canReceiveSetData
						.commonData[rtn/2][rtn%2].can_fd_flag;
					SendMsg.val[5] = (int)myData->canReceiveSetData
						.commonData[rtn/2][rtn%2].can_datarate;
					SendMsg.val[6] = (int)myData->canReceiveSetData
						.commonData[rtn/2][rtn%2].crc_type;
					SendMsg.val[7] = (int)myData->canReceiveSetData
						.commonData[rtn/2][rtn%2].terminal_r;
#endif //jhkw_190714e
					send_msg(APP_TO_MODULE, (char *)&SendMsg);

					memset((char *)&SendMsg, 0, sizeof(S_MSG_VAL));
					SendMsg.msg = MSG_APP_MODULE_CAN_DATA_CLEAR;
					SendMsg.val[0] = rtn / 2;
					send_msg(APP_TO_MODULE, (char *)&SendMsg);
					break;
			}
		}
	}

	myPs->signal[APP_SIG_APP_CONTROL_PROCESS] = P1;
	userlog(DEBUG_LOG, psName, "System start\n");
#ifdef __COA__
#ifdef __A_TYPE__
	userlog(DEBUG_LOG, psName, "H/W Control Type : A_TYPE\n");
#elif __B_TYPE__
	userlog(DEBUG_LOG, psName, "H/W Control Type : B_TYPE\n");
#else
	userlog(DEBUG_LOG, psName, "H/W Control Type : Non Control type\n");
#endif
#ifdef __COA_VER_100B__
	userlog(DEBUG_LOG, psName, "S/W Protocol Ver : 100B\n");
#elif __COA_VER_100B2__
	userlog(DEBUG_LOG, psName, "S/W Protocol Ver : 100B2\n");
#elif __COA_VER_100C__
	userlog(DEBUG_LOG, psName, "S/W Protocol Ver : 100C\n");
#elif __COA_VER_100D__
	userlog(DEBUG_LOG, psName, "S/W Protocol Ver : 100D\n");
#elif __COA_VER_100F__ //kjh_160418
	userlog(DEBUG_LOG, psName, "S/W Protocol Ver : 100F\n");
#elif __COA_VER_1011__ //kjh_170629
	userlog(DEBUG_LOG, psName, "S/W Protocol Ver : 1011\n");
#elif __COA_VER_1012__ //jhk_180902
	userlog(DEBUG_LOG, psName, "S/W Protocol Ver : 1012\n");
#elif __COA_VER_1013__ //kjh_181127
	userlog(DEBUG_LOG, psName, "S/W Protocol Ver : 1013\n");
#elif __COA_VER_1014__ //jhkw_190830
	userlog(DEBUG_LOG, psName, "S/W Protocol Ver : 1014\n");
#elif __COA_VER_1015__ //shh_200527
	userlog(DEBUG_LOG, psName, "S/W Protocol Ver : 1015\n");
#elif __COA_VER_1016__ //shh_210204
	userlog(DEBUG_LOG, psName, "S/W Protocol Ver : 1016\n");
#else
	userlog(DEBUG_LOG, psName, "S/W Protocol Ver : Non Protocol\n");
#endif

#ifdef __SBC_EM104_A5362__
	userlog(DEBUG_LOG, psName, "SBC Model : EM104_A5362\n");
#elif __SBC_EMCORE_V621__
	userlog(DEBUG_LOG, psName, "SBC Model : EMCORE_V621\n");
#else
	userlog(DEBUG_LOG, psName, "SBC Model : Non sbc model\n");
#endif
#endif
	switch(myData->mData.config.ratioV) { //231127s
		case 0:
			userlog(DEBUG_LOG, psName, "VOLTAGE Unit		: uV, ex)1 = 1uV\n");
			break;
		case 1:
			userlog(DEBUG_LOG, psName, "VOLTAGE Unit		: mV, ex)1 = 1mV\n");
			break;
		case 2:
			userlog(DEBUG_LOG, psName, "VOLTAGE Unit		: V, ex)1 = 1V\n");
			break;
		case 3:
			userlog(DEBUG_LOG, psName, "VOLTAGE Unit		: kV, ex)1 = 1kV\n");
			break;
		default:
			userlog(DEBUG_LOG, psName, "VOLTAGE Unit		: Non Voltage Unit\n");
			break;
		}
	switch(myData->mData.config.ratioI) {
		case 0:
			userlog(DEBUG_LOG, psName, "CURRENT Unit		: uA, ex)1 = 1uA\n");
			break;
		case 1:
			userlog(DEBUG_LOG, psName, "CURRENT Unit		: mA, ex)1 = 1mA\n");
			break;
		case 2:
			userlog(DEBUG_LOG, psName, "CURRENT Unit		: A, ex)1 = 1A\n");
			break;
		case 3:
			userlog(DEBUG_LOG, psName, "CURRENT Unit		: kA, ex)1 = 1kA\n");
			break;
		default:
			userlog(DEBUG_LOG, psName, "CURRENT Unit		: Non Current Unit\n");
			break;
	}
	switch(myData->mData.config.ratioP) {
		case 0:
			userlog(DEBUG_LOG, psName, "POWER Unit		: No Use\n");
			break;
		case 1:
			userlog(DEBUG_LOG, psName, "POWER Unit		: mW, ex)1 = 1mW\n");
			break;
		case 2:
			userlog(DEBUG_LOG, psName, "POWER Unit		: W, ex)100 = 1W\n");
			break;
		case 3:
			userlog(DEBUG_LOG, psName, "POWER Unit		: kW, ex)100 = 0.001kW\n");
			break;
		default:
			userlog(DEBUG_LOG, psName, "POWER Unit		: Non Power Unit\n");
			break;
	}
	userlog(DEBUG_LOG, psName, "---------------------------------------------------\n");
	//jhkw_231127e

	return 0;
}

int	Init_SystemData(void)
{
	//0:Dont insert mbuff module, 1:insert mbuff module
	if(Open_SystemMemory(1) < 0) return -1;

	myPs = &(myData->AppControl);

	if(Read_SystemMemory("systemMemory") < 0)
		return -2; //saved system memory data load

	Init_SystemMemory_1();

	return 0;
}

//shhw_250409s
void Delete_SystemMemory(void) 
{
	char cmd[256], fileDir[256];

	memset(fileDir, 0, sizeof(fileDir));
	strcpy(fileDir, "/root/system_data/config/sharedMemory/");

	//Delete systemMemory file
	memset(cmd, 0, sizeof(cmd));
	strcpy(cmd, "rm -rf ");
	strcat(cmd, fileDir);
	strcat(cmd, "systemMemory");
	system(cmd);

	//Make systemMemory file
	memset(cmd, 0, sizeof(cmd));
	strcpy(cmd, "touch  ");
	strcat(cmd, fileDir);
	strcat(cmd, "systemMemory");
	system(cmd);
}
//shhw_250409e

int Read_SystemMemory(char *fileName)
{
	int	fp, rtn;
	char cmd[256], tmp[256];

	//systemMemory file check : create - kjg_w
	
	memset(tmp, 0, sizeof tmp);
	strcpy(tmp, "/root/system_data/config/sharedMemory/");

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "touch ");
	strcat(cmd, tmp);
	strcat(cmd, fileName);
	system(cmd);
	// touch /root/system_data/config/sharedMemory/fileName

	memset(cmd, 0, sizeof(cmd));
	strcpy(cmd, tmp);
	strcat(cmd, fileName);
	// /root/system_data/config/sharedMemory/fileName

	if((fp = open(cmd, O_RDONLY)) < 0) {
		printf("Can not open %s file(load)\n", fileName);
		return -1;
	}

	rtn = read(fp, (char *)myData, sizeof(S_SYSTEM_DATA));
	if(rtn != sizeof(S_SYSTEM_DATA)) {
		printf("error %s read\n", fileName);
	}

	Delete_SystemMemory(); //shhw_250409

	close(fp);

	return 0;
}

void Init_SystemMemory_1(void)
{
	memset((char *)&myPs->config, 0, sizeof(S_APP_CONFIG));

	memset((char *)&myPs->misc.path1, 0, 256);
	strcpy((char *)&myPs->misc.path1, "system_data");
	
	memset((char *)&psName[0], 0, PROCESS_NAME_SIZE);
	strcpy(psName, "App");

	memset((char *)&myPs->signal, 0, MAX_SIGNAL);
	myPs->misc.processPointer = (int)&myData;
}

void Init_SystemMemory_2(void)
{
	/*Simplication - shh_250417*/
	unsigned char tmp;
	int i, j;

	//message
	memset((char *)&myData->msg, 0, sizeof(S_MSG) * MAX_MSG_RING);

	//debug
	memset((char *)&myData->test_val_uc[0][0], 0,
		sizeof(unsigned char) * 8 * MAX_TEST_VALUE);
	memset((char *)&myData->test_val_l[0], 0, sizeof(long) * MAX_TEST_VALUE);
	memset((char *)&myData->test_val_ll[0], 0,
		sizeof(long long) * MAX_TEST_VALUE);
	memset((char *)&myData->test_val_f[0], 0, sizeof(float) * MAX_TEST_VALUE);

	//dio
	memset((char *)&myData->dio.misc, 0, sizeof(S_DIO_MISC));
	memset((char *)&myData->dio.in, 0, sizeof(S_DIO_INPUT));
	memset((char *)&myData->dio.out, 0, sizeof(S_DIO_OUTPUT));
	memset((char *)&myData->dio.signal, 0, sizeof(unsigned char) * MAX_DIO_SIGNAL); //phb_230710 
	//memset((char *)&myData->dio.signal, 0, sizeof(unsigned char) * MAX_SIGNAL);

	//mControl
	memset((char *)&myData->mData.misc, 0, sizeof(S_MODULE_MISC));
	myData->mData.misc.main_slot = -1;
	myData->mData.misc.increment_period = (unsigned long)myData->mData
		.config.scan_period / 10; //kjg_w 25ms / 10 = 2
	myData->mData.misc.fan_run_time = 300; //6min
	myData->mData.misc.fan_stop_time = 1800; //30min
	myData->mData.misc.test_internal_r = 1;

	//jhkw_231127s
	if((myData->mData.config.ratioV == MICRO_UNIT) 
		&& (myData->mData.config.ratioI == MICRO_UNIT) 
		&& (myData->mData.config.ratioP == MILLI_UNIT)) {
		myData->mData.ratioV = 1000;	//123 = 123uV
		myData->mData.ratioI = 1000;	//123 = 123uA
		myData->mData.ratioP = 1000.0;	//123 = 123mW
		myData->mData.patt_ratioV = 1000000.0;	//123 = 123000000uV
		myData->mData.patt_ratioI = 1000000.0;	//123 = 123000000uA
		myData->mData.patt_ratioP = 1000.0;	//123 = 123000mW
		myData->mData.cp_to_cc = 1000000000.0;
	} else if((myData->mData.config.ratioV == MILLI_UNIT)
		&& (myData->mData.config.ratioI == MILLI_UNIT)
		&& (myData->mData.config.ratioP == BASE_UNIT)) {
		myData->mData.ratioV = 1000;	//123 = 123mV
		myData->mData.ratioI = 1000;	//123 = 123mA
		myData->mData.ratioP = 0.01;	//123 = 1.23W
		myData->mData.patt_ratioV = 1000.0;	//123 = 123000mV
		myData->mData.patt_ratioI = 1000.0;	//123 = 123000mA
		myData->mData.patt_ratioP = 100.0;	//123 = 123.00W
		myData->mData.cp_to_cc = 10000.0;
	} else {
		myData->mData.ratioV = 1000;	//123 = 123uV
		myData->mData.ratioI = 1000;	//123 = 123uA
		myData->mData.ratioP = 1000.0;	//123 = 123mW
		myData->mData.patt_ratioV = 1000000.0;	//123 = 123000000uV
		myData->mData.patt_ratioI = 1000000.0;	//123 = 123000000uA
		myData->mData.patt_ratioP = 1000.0;	//123 = 123000mW
		myData->mData.cp_to_cc = 0.0;
	}	
	//jhkw_231127e

	switch(myData->AppControl.config.systemModel) {
		case C_SKI_50V_500A_250A_25KW:	//ktg_190715
		case C_SKI_50V_500A_400A_200A_100A_100KW:	//jhk_180119
		case C_SKI_50V_500A_400A_200A_100A_100KW_2:	//jhk_180119
		case C_SKI_50V_1000A_500A_300A_100KW:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_2:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_3:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_4:	//ktg_190711
		case C_SK_60V_300A_10A:
		case C_SKI_60V_400A_200A_100A_96KW:		//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_2:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_3:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_4:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_5:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_6:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_7:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_8:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_9:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_10:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_11:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_12:	//ktg_190319
		case C_NORTHVOLT_60V_400A_200A_96KW:			//khj_191203
		case C_SKI_100V_100A_10A_10KW:			//jhk_120329
		case C_SKI_100V_300A_150A_50A_10A_60KW:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_2:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_3:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_4:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_5:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_6:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_7:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_8:	//kjh_120527
		case C_SKI_120V_400A_100A_192KW:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_2:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_3:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_4:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_5:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_6:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_7:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_8:	//jhk_170628
		case C_SKI_120V_400A_200A_100A_50A_192KW:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_2:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_3:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_4:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_5:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_6:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_7:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_8:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_9:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_10:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_11:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_12:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_13:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_14:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_15:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_16:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_17:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_18:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_192KW:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_2:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_3:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_4:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_5:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_6:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_7:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_8:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_9:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_10:	//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_11:	//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_12:	//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_13:	//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_14:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_15:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_16:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_17:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_18:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_19:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_20:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_21:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_22:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_23:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_24:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_25:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_26:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_27:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_28:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_29:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_30:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_31:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_32:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_33:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_34:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_35:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_36:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_37:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_38:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_39:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_40:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_41:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_42:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_43:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_44:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_45:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_46:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_47:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_48:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_49:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_50:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_51:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_52:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_53:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_54:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_55:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_56:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_57:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_58:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_59:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_60:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_61:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_62:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_63:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_64:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_65:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_66:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_67:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_68:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_69:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_70:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_71:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_72:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_73:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_74:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_75:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_76:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_77:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_78:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_79:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_80:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_81:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_82:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_83:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_84:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_85:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_86:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_87:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_88:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_89:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_90:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_91:	//khj_200308
		////////////////////////////////////////////////////////
		case C_SKI_120V_400A_200A_100A_192KW_92:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_93:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_94:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_95:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_96:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_97:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_98:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_99:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_100:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_101:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_102:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_103:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_104:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_105:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_106:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_107:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_108:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_109:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_110:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_111:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_112:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_113:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_114:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_115:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_116:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_117:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_118:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_119:	//khj_200308
		////////////////////////////////////////////////////////
		//case C_SKI_120V_400A_200A_100A_192KW_169:	//ktg_200410 //shh_240730
		//case C_TEST_120V_400A_200A_100A_192KW:		//shh_200723
		case C_SKI_120V_400A_200A_100A_192KW_170:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_171:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_172:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_173:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_174:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_175:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_176:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_177:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_178:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_179:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_180:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_181:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_182:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_183:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_184:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_185:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_186:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_187:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_188:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_189:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_190:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_191:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_192:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_193:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_194:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_195:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_196:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_197:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_198:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_199:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_200:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_201:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_202:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_203:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_204:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_205:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_206:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_207:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_208:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_209:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_210:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_211:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_212:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_213:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_214:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_215:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_216:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_233:	//ktg_200908
		case C_SKI_120V_400A_200A_100A_192KW_234:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_235:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_236:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_237:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_238:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_239:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_240:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_241:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_242:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_243:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_244:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_245:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_246:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_247:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_248:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_249:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_250:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_251:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_252:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_253:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_254:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_255:	//shh_220113
		////////////////////////////////////////////////////////
		/* 22PPSCSA062  SKON China - 11Set //shh_250417s
		case C_SKI_120V_425A_200A_100A_50A_192KW:		//phb_230206
		case C_SKI_120V_425A_200A_100A_50A_192KW_2:		//phb_230206
		case C_SKI_120V_425A_200A_100A_50A_192KW_3:		//phb_230206
		case C_SKI_120V_425A_200A_100A_50A_192KW_4:		//phb_230206
		case C_SKI_120V_425A_200A_100A_50A_192KW_5:		//phb_230206
		case C_SKI_120V_425A_200A_100A_50A_192KW_6:		//phb_230206
		case C_SKI_120V_425A_200A_100A_50A_192KW_7:		//phb_230206
		case C_SKI_120V_425A_200A_100A_50A_192KW_8:		//phb_230206
		case C_SKI_120V_425A_200A_100A_50A_192KW_9:		//phb_230206
		case C_SKI_120V_425A_200A_100A_50A_192KW_10:	//phb_230206
		case C_SKI_120V_425A_200A_100A_50A_192KW_11:	//phb_230206
		*/									//shh_250417e
		//case C_SKI_120V_425A_300A_200A_100A_192KW:  	//phb_230105
		//case C_SKI_120V_425A_300A_200A_100A_192KW_2:	//phb_230105
		//case C_SKI_120V_425A_300A_200A_100A_192KW_3:	//phb_230105
		case C_SKI_180V_425A_200A_100A_192KW:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_2:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_3:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_4:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_5:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_6:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_7:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_8:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_9:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_10:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_11:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_12:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_13:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_14:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_15:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_16:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_17:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_18:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_19:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_20:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_21:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_22:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_23:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_24:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_25:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_26:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_27:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_28:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_29:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_30:	//sec_221221
		case C_SKI_180V_600A_300A_100A_320KW:		//phb_230220
		//case C_SKI_180V_600A_300A_100A_320KW_2:		//shh_231004
		case C_NORTHVOLT_200V_400A_200A_160KW:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_2:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_3:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_4:	//ktg_191203
		//21PPSCSA052 SKON MOA 9SET - START //shh_250417	
		//case C_SKI_260V_425A_200A_442KW:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_2:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_3:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_4:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_5:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_6:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_7:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_8:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_9:	//shh_211020
		//21PPSCSA052 SKON MOA 9SET - END
		case C_SK_450V_200A_10A_180KW:
		case C_SK_450V_200A_10A_90KW:
		case C_SK_450V_200A_10A_360KW:
		case C_SKI_450V_200A_100A_90KW:		//sec_221223
		case C_SKI_450V_150A_100A_50A_260KW:	//sec_221215
		case C_SKI_450V_150A_100A_50A_260KW_2:	//sec_221215
		case C_SKI_500V_450A_200A_450KW:		//khj_200308
		case C_SKI_500V_450A_200A_450KW_2:		//khj_200308
		case C_SKI_500V_450A_200A_450KW_3:		//khj_200308
		//case C_SKI_500V_450A_200A_450KW_4:		//ktg_200410 //shh_240730s
		//case C_SKI_500V_450A_200A_450KW_5:		//ktg_200410
		//case C_SKI_500V_450A_200A_450KW_6:		//ktg_200410 //shh_240730e	
		case C_SKC_600V_200A_100A_50A_240KW: 	//jhkw_120807
		case C_SKC_600V_200A_100A_50A_240KW_2: 	//jhkw_121019
		case C_SKE_600V_400A_100A_50A_25A_240KW://jhkw_130924
		case C_DAEHWA_600V_400A_200A_100A_200KW:	//jhk_170120
		case C_DAEHWA_750V_300A_100A_150KW:	//jhk_170203
		case C_SKI_1000V_400A_200A_100A_450KW:	//sec_230103
		case C_SKI_1200V_250A_100A_50A_300KW:	//jhk_131209
		//case C_SKI_1200V_500A_400A_300A_200A_600KW:		//phb_230116
	    //case C_SKI_1500V_350A_300A_200A_100A_450KW:		//phb_230117
		case C_SKI_1500V_400A_100A_500KW:	//jhk_180610
		case C_SKI_2000V_300A_200A_100A_50A_600KW:	//phb_230320
		case C_SKI_AUX_BOX:					//shh_211007
			myData->mData.misc.module_type = MODULE_PWM1;
			break;
		/*case C_SDI_5V_60A_10A_1A:
		case C_SDI_5V_250A_25A_5A:
		case C_SBL_5V_250A_25A_5A:
		case C_SBL_5V_250A_60A_10A:
		case C_SDI_5V_300A_60A_10A:
		case C_SDI_5V_450A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A:
		case C_SBL_5V_450A_240A_60A_10A_2:
			myData->mData.misc.module_type = MODULE_FCH;
			break;
		case F_PNE_5V_15A_30AP_SW:
			myData->mData.misc.module_type = MODULE_PWM2;
			break;
		case C_PNE_1000V_300A_300KW:
			myData->mData.misc.module_type = MODULE_PWM3;
			break;*/
		default:
			myData->mData.misc.module_type = MODULE_PWM1;	//shh_240417
			//myData->mData.misc.module_type = MODULE_LINEAR;
			break;
	}

	myData->mData.code = M_CD_NONE;
	tmp = myData->mData.signal[M_SIG_RUNNING_GROUP];
	memset((char *)&myData->mData.signal, 0,
		sizeof(unsigned char) * MAX_SIGNAL);
	myData->mData.signal[M_SIG_RUNNING_GROUP] = tmp;

	for(i=0; i < 6; i++) {
		for(j=0; j < 10; j++) {
			if(i == 3 || i == 5)
				myData->mData.runningTime[i][j] = 9000000; //9ms
			else myData->mData.runningTime[i][j] = 0;
		}
	}

	j = 0;
	for(i=0; i < MAX_GROUP_8; i++) {
		myData->gData[i].workMode = WORK_TYPE_OFFLINE;
		myData->gData[i].misc.chOffset = j;
		j += myData->mData.config.chInGroup[i];
	}

	for(i=0; i < MAX_BD_16; i++) {
		for(j=0; j < 4; j++) {
			myData->bData[i].misc.source2[j].totalV = 0.0;
			myData->bData[i].misc.source2[j].totalI = 0.0;
		}
	}

	for(i=0; i < MAX_CH_256; i++) {
		if(myData->cData[i].op.state == C_CALI) {
			myData->cData[i].op.state = C_IDLE;
			myData->cData[i].op.phase = P0;
		}

		if(myData->cData[i].op.state != C_IDLE) {
			myData->cData[i].signal[C_SIG_OUT_SWITCH] = P1;
			myData->cData[i].signal[C_SIG_SEMI_SWITCH] = P0;
		}

		myData->cData[i].signal[C_SIG_OUT_SWITCH_PHASE] = P0; //kjg_120106
	}

	//daq
	memset((char *)&myData->SubSensV, 0, sizeof(S_SUB_SENS_V));

	//rt_can
	memset((char *)&myData->CAN.receive_time[0], 0,
		sizeof(long) * MAX_CAN_PORT);
	memset((char *)&myData->CAN.transmit_time[0], 0,
		sizeof(long) * MAX_CAN_PORT);

	memset((char *)&myData->CAN.signal[0][0], 0,
		sizeof(unsigned char) * MAX_CAN_PORT * MAX_SIGNAL); //kjg_180523

	//rt_com
	memset((char *)&myData->COM.com_port, 0, sizeof(S_COM_PORT) * MAX_COM_PORT);

	for(i=0; i < MAX_COM_PORT; i++) {
		if(myData->COM.config.functionType[i] == COM_FUNC_TYPE_BCR1) {
			if(myData->AppControl.misc.Load_Process[PROCESS_GROUP_COB][0]
				== P1) {
				myData->COM.com_port[i].misc.externPort_useFlag = P0;
			} else {
				//for Calibration
				myData->COM.com_port[i].misc.externPort_useFlag = P1;
			}

			for(j=0; j < MAX_GROUP_4; j++) {
				if(myData->COM.config.functionModel[i] == 0) {
					myData->COM.com_port[i].misc.bcr_reservation[j] = P0;
				} else if(myData->COM.config.functionModel[i] == 1) {
					myData->COM.com_port[i].misc.bcr_reservation[j] = P11;
				} else if(myData->COM.config.functionModel[i] == 2) {
					myData->COM.com_port[i].misc.bcr_reservation[j] = P11;
				} else {
					myData->COM.com_port[i].misc.bcr_reservation[j] = P0;
				}
			}
		}

		if(myData->COM.config.functionType[i] == COM_FUNC_TYPE_METER2) {
			if(myData->COM.config.autoStart[i] == P1) {
				myData->COM.com_port[i]
					.signal[COM_SIG_ANALOG_METER_REQUEST_PHASE] = P1;
			}
		}

		if(myData->COM.config.functionType[i] == COM_FUNC_TYPE_CHAMBER1) {
			if(myData->COM.config.autoStart[i] == P1) {
				myData->COM.com_port[i].signal[COM_SIG_CHAMBER_COMM] = P1;
			}
		}

		if(myData->COM.config.functionType[i] == COM_FUNC_TYPE_IO_COMM1) {
			if(myData->COM.config.autoStart[i] == P1) {
				switch(myData->COM.config.functionModel[i]) {
					case 0:
						break;
					case 1:
						//myData->COM.com_port[i]
						//	.signal[COM_SIG_PLC_REQUEST_PHASE]
						//	= PLC_FAULT_READ_SEND;
						myData->COM.com_port[i].signal[COM_SIG_PLC_COMM] = P1;
						myData->COM.com_port[i]
							.signal[COM_SIG_PLC_INIT_SEND] = P1;
						break;
					default:
						break;
				}
			}
		}

		if(myData->COM.config.functionType[i] == COM_FUNC_TYPE_DISPLAY1) {
//			myData->AppControl.misc
//				.Load_Process_COM[PROCESS_COM_CALI_METER] = P1;//jhkw_130119
			if(myData->AppControl.misc
				.Load_Process_COM[PROCESS_COM_CALI_METER] == P1) {
				//for Calibration
				myData->COM.com_port[i].misc.externPort_useFlag = P1;
			} else {
				myData->COM.com_port[i].misc.externPort_useFlag = P0;

				if(myData->COM.config.autoStart[i] == P1) {
					myData->COM.com_port[i].signal[COM_SIG_DISPLAY] = P1;
				}
			}
		}
	}

	switch(myData->AppControl.config.systemModel) {
		case F_PNE_5V_15A_30AP_SW:
			myData->mData.misc.rt_periodic = 2000000; //2ms

			myData->mData.config.adRatioV[0] = 204.2675912;
			myData->mData.config.adRatioV[1] = 1.0;
			myData->mData.config.adRatioV[2] = 1.0;
			myData->mData.config.adRatioV[3] = 1.0;

			myData->mData.config.adRatioI[0] = 1008.230954;
			myData->mData.config.adRatioI[1] = 100.8414636;
			myData->mData.config.adRatioI[2] = 504.1154768;
			myData->mData.config.adRatioI[3] = 50.42073178;

			myData->mData.config.daRatioV_P[0] = 1.05;
			myData->mData.config.daRatioV_N[0] = 1.05;
			myData->mData.config.daRatioV_P[1] = 1.0;
			myData->mData.config.daRatioV_N[1] = 1.0;
			myData->mData.config.daRatioV_P[2] = 1.0;
			myData->mData.config.daRatioV_N[2] = 1.0;
			myData->mData.config.daRatioV_P[3] = 1.0;
			myData->mData.config.daRatioV_N[3] = 1.0;

			myData->mData.config.daOffsetV_P[0] = -125000.0;
			myData->mData.config.daOffsetV_N[0] = -125000.0;
			myData->mData.config.daOffsetV_P[1] = 0.0;
			myData->mData.config.daOffsetV_N[1] = 0.0;
			myData->mData.config.daOffsetV_P[2] = 0.0;
			myData->mData.config.daOffsetV_N[2] = 0.0;
			myData->mData.config.daOffsetV_P[3] = 0.0;
			myData->mData.config.daOffsetV_N[3] = 0.0;

			myData->mData.config.daRatioI_P[0] = 0.245049505;
			myData->mData.config.daRatioI_N[0] = 0.242;
			myData->mData.config.daRatioI_P[1] = 2.450048693;
			myData->mData.config.daRatioI_N[1] = 2.419559199;
			myData->mData.config.daRatioI_P[2] = 1.0;
			myData->mData.config.daRatioI_N[2] = 1.0;
			myData->mData.config.daRatioI_P[3] = 1.0;
			myData->mData.config.daRatioI_N[3] = 1.0;

			myData->mData.config.daOffsetI_P[0] = 975247.5248;
			myData->mData.config.daOffsetI_N[0] = 975247.5248;
			myData->mData.config.daOffsetI_P[1] = 975247.5248;
			myData->mData.config.daOffsetI_N[1] = 975247.5248;
			myData->mData.config.daOffsetI_P[2] = 0.0;
			myData->mData.config.daOffsetI_N[2] = 0.0;
			myData->mData.config.daOffsetI_P[3] = 0.0;
			myData->mData.config.daOffsetI_N[3] = 0.0;
			break;
		default:
			myData->mData.misc.rt_periodic = 1000000; //1ms
			break;
	}
}

void Save_SystemMemory(void)
{
	int	fp, rtn;
	char fileName[256];
	
	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/sharedMemory/systemMemory");
	// /root/system_data/config/sharedMemory/systemMemory
	if((fp = open(fileName, O_RDWR)) < 0) {
		userlog(DEBUG_LOG, psName, "Can not open System Memory file(save)\n");
		return;
	}
	rtn = write(fp, (char *)myData, sizeof(S_SYSTEM_DATA));
	if(rtn != sizeof(S_SYSTEM_DATA)) {
		userlog(DEBUG_LOG, psName, "error System Memory write %d\n", rtn);
	}
	close(fp);
}

int Load_Process_All(void)
{
	char cmd[80];
	int i, signal;

	if(myPs->misc.Load_Process_CAN[0] == P1) {
		for(i=0; i < myData->mData.config.installedCAN; i++) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "setserial /dev/ttyS1%d uart none", i);
			system(cmd);
		}
	}
	
	if(myPs->misc.Load_Process_CAN[2] == P1) {
		sprintf(cmd, "Load_CanClient");
		signal = APP_SIG_CAN_CLIENT_PROCESS;
		if(Load_Process(cmd, "./", signal) < 0) return -7;
	}

	if(myPs->misc.Load_Process_COM[0] == P1) {
		for(i=0; i < myData->mData.config.installedCOM; i++) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "setserial /dev/ttyS%d uart none", i);
			system(cmd);
		}
	}

	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COA][i] == P1) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "Load_COA%d_Client", i+1);
			if(Load_Process(cmd, "./", APP_SIG_COA1_CLIENT_PROCESS+i) < 0)
				return -1;
		}
	}

	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COB][i] == P1) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "Load_COB%d_Client", i+1);
			if(Load_Process(cmd, "./", APP_SIG_COB1_CLIENT_PROCESS+i) < 0)
				return -2;
		}
	}

	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COC][i] == P1) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "Load_COC%d_Client", i+1);
			if(Load_Process(cmd, "./", APP_SIG_COC1_CLIENT_PROCESS+i) < 0)
				return -3;
		}
	}

	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COD][i] == P1) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "Load_COD%d_Client", i+1);
			if(Load_Process(cmd, "./", APP_SIG_COD1_CLIENT_PROCESS+i) < 0)
				return -4;
		}
	}

	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_JIG][i] == P1) {
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "Load_Jig%d_Control", i+1);
			if(Load_Process(cmd, "./", APP_SIG_JIG1_CONTROL_PROCESS+i) < 0)
				return -5;
		}
	}

	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_ETC][i] == P1) {
			memset(cmd, 0, sizeof cmd);
			switch(i) {
				case PROCESS_DATA_SAVE:
					sprintf(cmd, "Load_DataSave");
					signal = APP_SIG_DATA_SAVE_PROCESS;
					break;
				case PROCESS_CALI_SERVER:
					sprintf(cmd, "Load_CaliServer");
					signal = APP_SIG_CALI_SERVER_PROCESS;
					break;
				case PROCESS_MODULE_CONTROL:
					sprintf(cmd, "Load_mControl");
					signal = APP_SIG_MODULE_CONTROL_PROCESS;
					break;
				case PROCESS_TIME_SCHEDULE: //kjh_160418
					sprintf(cmd, "Load_TimeSchedule");
					signal = APP_SIG_TIME_SCHEDULE_PROCESS;
					break;
//20190905 KHK-----------------------------------					
				case PROCESS_TEMP1_CLIENT:
					sprintf(cmd, "Load_Temp1Client");
					signal = APP_SIG_TEMP1_CLIENT_PROCESS;
					break;
				case PROCESS_TEMP2_CLIENT:
					sprintf(cmd, "Load_Temp2Client");
					signal = APP_SIG_TEMP2_CLIENT_PROCESS;
					break;
				case PROCESS_TEMP3_CLIENT:
					sprintf(cmd, "Load_Temp3Client");
					signal = APP_SIG_TEMP3_CLIENT_PROCESS;
					break;
				case PROCESS_TEMP4_CLIENT:
					sprintf(cmd, "Load_Temp4Client");
					signal = APP_SIG_TEMP4_CLIENT_PROCESS;
					break;
//------------------------------------------------					
				default:
					return -6;
					//break;
			}
			if(Load_Process(cmd, "./", signal) < 0) return -6;
		}
	}
//shh_231124s
#if defined __DEBUG__
	sprintf(cmd, "Load_SilClient");
	signal = APP_SIG_SIL_CLIENT_PROCESS;
	if(Load_Process(cmd, "./", signal) < 0) return -7;
		
	sprintf(cmd, "Load_SilClient2");
	signal = APP_SIG_SIL_CLIENT2_PROCESS;
	if(Load_Process(cmd, "./", signal) < 0) return -8;
#endif
//shh_231124e
	return 0;
}

int Load_Process(char *process, char *path, int signalNo)
{
	char cmd[256];
	int cnt = 0;

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, path);
	strcat(cmd, process);
	cnt = system(cmd); // ./process

	while(1) {
		usleep(100000); //100mS
		if(myPs->signal[signalNo] == P1) break;
		cnt++;
		userlog(DEBUG_LOG,  psName, "%s load try %d\n", process, cnt);
		if(cnt >= 10) {
			userlog(DEBUG_LOG, psName, "%s load fail\n", process);
			return -1;
		}
	}
	return 0;
}

void Close_Process_All(void)
{
	char cmd[80];
	int i, signalNo, processPointer, rtn;

#if defined __COA__
	//debug_size_coa
	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COA][i] == P1) {
			signalNo = APP_SIG_COA1_CLIENT_PROCESS + i;
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "COA%d_Client", i+1);
			Close_Process(cmd, myData->COA_Client[i].misc.processPointer,
				signalNo);
		}
	}
#endif

#if defined __COB__
	//debug_size_cob
	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COB][i] == P1) {
			signalNo = APP_SIG_COB1_CLIENT_PROCESS + i;
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "COB%d_Client", i+1);
			Close_Process(cmd, myData->COB_Client[i].misc.processPointer,
				signalNo);
		}
	}
#endif

#if defined __COC__
	//debug_size_coc
	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COC][i] == P1) {
			signalNo = APP_SIG_COC1_CLIENT_PROCESS + i;
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "COC%d_Client", i+1);
			Close_Process(cmd, myData->COC_Client[i].misc.processPointer,
				signalNo);
		}
	}
#endif

#if defined __COD__
	//debug_size_cod
	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COD][i] == P1) {
			signalNo = APP_SIG_COD1_CLIENT_PROCESS + i;
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "COD%d_Client", i+1);
			Close_Process(cmd, myData->COD_Client[i].misc.processPointer,
				signalNo);
		}
	}
#endif

#if defined __COB__
	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_JIG][i] == P1) {
			signalNo = APP_SIG_JIG1_CONTROL_PROCESS + i;
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "Jig%d_Control", i+1);
			Close_Process(cmd, myData->jData[i].misc.processPointer,
				signalNo);
		}
	}
#endif

	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_ETC][i] == P1) {
			memset(cmd, 0, sizeof cmd);
			processPointer = signalNo = rtn = 0;
			switch(i) {
				case PROCESS_DATA_SAVE:
					sprintf(cmd, "DataSave");
					signalNo = APP_SIG_DATA_SAVE_PROCESS;
					processPointer = myData->DataSave.misc.processPointer;
					break;
				case PROCESS_TIME_SCHEDULE:
					sprintf(cmd, "TimeSchedule");
					signalNo = APP_SIG_TIME_SCHEDULE_PROCESS;
					processPointer = myData->TimeSch.misc.processPointer;
					break;
//20190905 KHK----------------------------------------------					
				case PROCESS_TEMP1_CLIENT:
					sprintf(cmd, "Temp1Client");
					signalNo = APP_SIG_TEMP1_CLIENT_PROCESS;
					processPointer = myData->TimeSch.misc.processPointer;
					break;
				case PROCESS_TEMP2_CLIENT:
					sprintf(cmd, "Temp2Client");
					signalNo = APP_SIG_TEMP2_CLIENT_PROCESS;
					processPointer = myData->TimeSch.misc.processPointer;
					break;
				case PROCESS_TEMP3_CLIENT:
					sprintf(cmd, "Temp3Client");
					signalNo = APP_SIG_TEMP3_CLIENT_PROCESS;
					processPointer = myData->TimeSch.misc.processPointer;
					break;
				case PROCESS_TEMP4_CLIENT:
					sprintf(cmd, "Temp4Client");
					signalNo = APP_SIG_TEMP4_CLIENT_PROCESS;
					processPointer = myData->TimeSch.misc.processPointer;
					break;
//----------------------------------------------------------					
				/*kjg_w case PROCESS_CALI_SERVER:
					sprintf(cmd, "CaliServer");
					signalNo = APP_SIG_CALI_SERVER_PROCESS;
					processPointer = myData->CaliServer.misc.processPointer;
					break;*/
				default:
					rtn = -1;
					break;
			}
			if(rtn < 0) continue;

			Close_Process(cmd, processPointer, signalNo);
		}
	}
	if(myPs->misc.Load_Process_CAN[2] == P1) {
		sprintf(cmd, "CanClient");
		signalNo = APP_SIG_CAN_CLIENT_PROCESS;
		processPointer = myData->Can_Client.misc.processPointer;
		Close_Process(cmd, processPointer, signalNo);
	}
//shh_231124s
#if defined __DEBUG__
	sprintf(cmd, "Load_SilClient");
	signalNo = APP_SIG_SIL_CLIENT_PROCESS;
	processPointer = myData->Sil_Client.misc.processPointer;
	Close_Process(cmd, processPointer, signalNo);
					
	sprintf(cmd, "Load_SilClient2");
	signalNo = APP_SIG_SIL_CLIENT2_PROCESS;
	processPointer = myData->Sil_Client.misc.processPointer;
	Close_Process(cmd, processPointer, signalNo);
#endif
//shh_231124e
	Unload_Module();
}

void Close_Process(char *process, int pointer, int signalNo)
{
	char buf[32];
	int cnt=0, rtn=0;

	memset(buf, 0, sizeof buf);
	strcpy(buf, process);
	
	while(1) {
		switch(myPs->signal[signalNo]) {
			case P0: rtn = -1;	break;
			case P1: myPs->signal[signalNo] = P2; break;
			case P2:			break;
			case P3: rtn = -2;	break;
			default: rtn = -3;	break;
		}
		if(rtn < 0) break;
		usleep(500000);
		cnt++;
		if(cnt >= 10) {
			userlog(DEBUG_LOG, psName, "ProcessKill %s\n", buf);
			if(pointer > 0) {
				Close_mbuff(pointer);
			}
			Kill_Process(process);
			break;
		}
	}
}

void Close_Process_2(char *process, char *index, int pointer, int signalNo)
{
	char buf[32];
	int cnt=0, rtn=0;

	memset(buf, 0, sizeof buf);
	strcpy(buf, process);
	
	while(1) {
		switch(myPs->signal[signalNo]) {
			case P0: rtn = -1;	break;
			case P1: myPs->signal[signalNo] = P2; break;
			case P2:			break;
			case P3: rtn = -2;	break;
			default: rtn = -3;	break;
		}
		if(rtn < 0) break;
		usleep(500000);
		cnt++;
		if(cnt >= 10) {
			userlog(DEBUG_LOG, psName, "ProcessKill %s\n", buf);
			if(pointer > 0) {
				Close_mbuff(pointer);
			}
			Kill_Process_2(process, index);
			break;
		}
	}
}

void Kill_Process(char *psName)
{
	char buf[32], psKill[32], cmd[256];
	int rtn, i;
    FILE *fp;

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "ps -ax | grep ./");
	strcat(cmd, psName);
	strcat(cmd, " > /root/");
	strcat(cmd, (char *)&myPs->misc.path1);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, psName);
	strcat(cmd, "Kill.txt");
	system(cmd);
	usleep(500000);
	// ps -ax | grep ./psName > /root/system_data/config/tmp/psNameKill.txt
	
	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "/root/");
	strcat(cmd, (char *)&myPs->misc.path1);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, psName);
	strcat(cmd, "Kill.txt");
	// /root/system_data/config/tmp/psNameKill.txt
	
	if((fp = fopen(cmd, "r")) == NULL) {
		printf("Don't open file : %s\n", cmd);
		return;
	}

	i = 0;
	while(1) {
		i++;
		memset(buf, 0, sizeof buf);
	   	rtn = fscanf(fp, "%s", buf);
		if(rtn > 0) {
			if(i == 1) {
				memset(psKill, 0, sizeof psKill);
				strcpy(psKill, "kill ");
				strcat(psKill, buf);
			} else if(i == 5) {
				memset(cmd, 0, sizeof cmd);
				strcpy(cmd, "./");
				strcat(cmd, psName);
				if(strcmp(cmd, buf) == 0) {
					i = 10;
					break;
				}
			}
		}
		if(i >= 5) break;
	}
	fclose(fp);

	if(i == 10) {
		system(psKill);
		printf("%s %s\n", psName, psKill);
	}
}

void Kill_Process_2(char *psName, char *index)
{
	int rtn, i, result;
	char buf[32], psKill[32], cmd[256];
    FILE *fp;

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "ps -ax | grep './");
	strcat(cmd, psName);
	strcat(cmd, " ");
	strcat(cmd, index);
	strcat(cmd, "' > /root/");
	strcat(cmd, (char *)&myPs->misc.path1);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, psName);
	strcat(cmd, index);
	strcat(cmd, "Kill.txt");
	system(cmd);
	usleep(500000);
	// ps -ax | grep './psName #' > /root/system_data/config/tmp/psName#Kill.txt
	
	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "/root/");
	strcat(cmd, (char *)&myPs->misc.path1);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, psName);
	strcat(cmd, index);
	strcat(cmd, "Kill.txt");
	// /root/system_data/config/tmp/psNameKill.txt
	
    if((fp = fopen(cmd, "r")) == NULL) {
		printf("Don't open file : %s\n", cmd);
		return;
	}

	i = 0;
	result = 0;
	while(1) {
		i++;
		memset(buf, 0, sizeof buf);
	   	rtn = fscanf(fp, "%s", buf);
		if(rtn > 0) {
			if(i == 1) {
				memset(psKill, 0, sizeof psKill);
				strcpy(psKill, "kill ");
				strcat(psKill, buf);
			} else if(i == 5) {
				memset(cmd, 0, sizeof cmd);
				strcpy(cmd, "./");
				strcat(cmd, psName);
				if(strcmp(cmd, buf) == 0) {
					result++;
				}
			} else if(i == 6) {
				memset(cmd, 0, sizeof cmd);
				strcpy(cmd, index);
				if(strcmp(cmd, buf) == 0) {
					result++;
				}
			}
		}
		if(i >= 6) break;
	}

	if(result == 2) {
		system(psKill);
		printf("%s %s\n", psName, psKill);
	}
	fclose(fp);
}

void Check_Process(void)
{
	char cmd[80], cmd2[80];
	int rtn, i, signalNo, signalNo2;
	long diff;
	time_t the_time;

	(void)time(&the_time);

	diff = the_time - myPs->misc.processCheckTime;
	if(diff < 0) {
		myPs->misc.processCheckTime = the_time;
		return;
	} else if(diff < 2) return;

	myPs->misc.processCheckTime = the_time;

#ifdef __COA__
	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COA][i] == P1) {
			signalNo = APP_SIG_COA1_CLIENT_PROCESS_CHECK + i;
			signalNo2 = APP_SIG_COA1_CLIENT_PROCESS + i;
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "COA%d_Client", i+1);
			rtn = DieCheck_Process(cmd);
			if(rtn < 0) { //process died
				myPs->signal[signalNo]++;
			} else {
				myPs->signal[signalNo] = P0;
			}
			if(myPs->signal[signalNo] >= P3) {
				myPs->signal[signalNo] = P0;
				memset(cmd, 0, sizeof cmd);
				sprintf(cmd, "Load_COA%d_Client", i+1);
				rtn = Load_Process(cmd, "./", signalNo2);
			}

			//kjg_131002
			if(myData->AppControl.signal[APP_SIG_MODULE_CONTROL_PROCESS] == P1) {
				diff = (myData->mData.misc.timer_1sec
					- myData->COA_Client[i].misc.net_time) * 1000;
				diff += (myData->mData.misc.timer_1000ms
					- myData->COA_Client[i].misc.net_time2);
				if(diff >= myData->COA_Client[i].config.netTimeout
					|| diff < 0) {
					userlog(DEBUG_LOG, psName, "app kill : %s\n", cmd);
					Kill_Process(cmd);
				}
			}
		}
	}
#endif

#ifdef __COB__
	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COB][i] == P1) {
			signalNo = APP_SIG_COB1_CLIENT_PROCESS_CHECK + i;
			signalNo2 = APP_SIG_COB1_CLIENT_PROCESS + i;
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "COB%d_Client", i+1);
			rtn = DieCheck_Process(cmd);
			if(rtn < 0) { //process died
				myPs->signal[signalNo]++;
			} else {
				myPs->signal[signalNo] = P0;
			}
			if(myPs->signal[signalNo] >= P3) {
				myPs->signal[signalNo] = P0;
				memset(cmd, 0, sizeof cmd);
				sprintf(cmd, "Load_COB%d_Client", i+1);
				rtn = Load_Process(cmd, "./", signalNo2);
			}
		}
	}
#endif

#ifdef __COC__
	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COC][i] == P1) {
			signalNo = APP_SIG_COC1_CLIENT_PROCESS_CHECK + i;
			signalNo2 = APP_SIG_COC1_CLIENT_PROCESS + i;
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "COC%d_Client", i+1);
			rtn = DieCheck_Process(cmd);
			if(rtn < 0) { //process died
				myPs->signal[signalNo]++;
			} else {
				myPs->signal[signalNo] = P0;
			}
			if(myPs->signal[signalNo] >= P3) {
				myPs->signal[signalNo] = P0;
				memset(cmd, 0, sizeof cmd);
				sprintf(cmd, "Load_COC%d_Client", i+1);
				rtn = Load_Process(cmd, "./", signalNo2);
			}
		}
	}
#endif

#ifdef __COD__
	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COD][i] == P1) {
			signalNo = APP_SIG_COD1_CLIENT_PROCESS_CHECK + i;
			signalNo2 = APP_SIG_COD1_CLIENT_PROCESS + i;
			memset(cmd, 0, sizeof cmd);
			sprintf(cmd, "COD%d_Client", i+1);
			rtn = DieCheck_Process(cmd);
			if(rtn < 0) { //process died
				myPs->signal[signalNo]++;
			} else {
				myPs->signal[signalNo] = P0;
			}
			if(myPs->signal[signalNo] >= P3) {
				myPs->signal[signalNo] = P0;
				memset(cmd, 0, sizeof cmd);
				sprintf(cmd, "Load_COD%d_Client", i+1);
				rtn = Load_Process(cmd, "./", signalNo2);
			}
		}
	}
#endif
	
	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_ETC][i] == P1) {
			rtn = 0;
			signalNo = signalNo2 = 0;
			memset(cmd, 0, sizeof cmd);
			memset(cmd2, 0, sizeof cmd2);
			switch(i) {
				case PROCESS_DATA_SAVE:
					signalNo = APP_SIG_DATA_SAVE_PROCESS_CHECK;
					signalNo2 = APP_SIG_DATA_SAVE_PROCESS;
					sprintf(cmd, "DataSave");
					sprintf(cmd2, "Load_DataSave");
					break;
				case PROCESS_CALI_SERVER:
					signalNo = APP_SIG_CALI_SERVER_PROCESS_CHECK;
					signalNo2 = APP_SIG_CALI_SERVER_PROCESS;
					sprintf(cmd, "CaliServer");
					sprintf(cmd2, "Load_CaliServer");
					break;
//20190905 KHK----------------------------
				case PROCESS_TEMP1_CLIENT:
					signalNo = APP_SIG_TEMP1_CLIENT_PROCESS_CHECK;
					signalNo2 = APP_SIG_TEMP1_CLIENT_PROCESS;
					sprintf(cmd, "Temp1Client");
					sprintf(cmd2, "Load_Temp1Client");
					break;
				case PROCESS_TEMP2_CLIENT:
					signalNo = APP_SIG_TEMP2_CLIENT_PROCESS_CHECK;
					signalNo2 = APP_SIG_TEMP2_CLIENT_PROCESS;
					sprintf(cmd, "Temp2Client");
					sprintf(cmd2, "Load_Temp2Client");
					break;
				case PROCESS_TEMP3_CLIENT:
					signalNo = APP_SIG_TEMP3_CLIENT_PROCESS_CHECK;
					signalNo2 = APP_SIG_TEMP3_CLIENT_PROCESS;
					sprintf(cmd, "Temp3Client");
					sprintf(cmd2, "Load_Temp3Client");
					break;
				case PROCESS_TEMP4_CLIENT:
					signalNo = APP_SIG_TEMP4_CLIENT_PROCESS_CHECK;
					signalNo2 = APP_SIG_TEMP4_CLIENT_PROCESS;
					sprintf(cmd, "Temp4Client");
					sprintf(cmd2, "Load_Temp4Client");
					break;
//----------------------------------------					
				default:
					rtn = -1;
					break;
			}
			if(rtn < 0) continue;

			rtn = DieCheck_Process(cmd);
			if(rtn < 0) { //process died
				myPs->signal[signalNo]++;
			} else {
				myPs->signal[signalNo] = P0;
			}
			if(myPs->signal[signalNo] >= P3) {
				myPs->signal[signalNo] = P0;
				rtn = Load_Process(cmd2, "./", signalNo2);
			}
		}
	}
	if(myPs->misc.Load_Process_CAN[2] == P1) {
		signalNo = APP_SIG_CAN_CLIENT_PROCESS_CHECK;
		signalNo2 = APP_SIG_CAN_CLIENT_PROCESS;
		sprintf(cmd, "CanClient");
		sprintf(cmd2, "Load_CanClient");
		rtn = DieCheck_Process(cmd);
		if(rtn < 0) { //process died
			myPs->signal[signalNo]++;
		} else {
			myPs->signal[signalNo] = P0;
		}
		if(myPs->signal[signalNo] >= P3) {
			myPs->signal[signalNo] = P0;
			rtn = Load_Process(cmd2, "./", signalNo2);
		}
	}
//shh_231124s
#if defined __DEBUG__
	signalNo = APP_SIG_SIL_CLIENT_PROCESS_CHECK;
	signalNo2 = APP_SIG_SIL_CLIENT_PROCESS;
	sprintf(cmd, "SilClient");
	sprintf(cmd2, "Load_SilClient");
	rtn = DieCheck_Process(cmd);
	if(rtn < 0) { //process died
		myPs->signal[signalNo]++;
	} else {
		myPs->signal[signalNo] = P0;
	}
	if(myPs->signal[signalNo] >= P3) {
		myPs->signal[signalNo] = P0;
		rtn = Load_Process(cmd2, "./", signalNo2);
	}
				
	signalNo = APP_SIG_SIL_CLIENT2_PROCESS_CHECK;
	signalNo2 = APP_SIG_SIL_CLIENT2_PROCESS;
	sprintf(cmd, "SilClient2");
	sprintf(cmd2, "Load_SilClient2");
	rtn = DieCheck_Process(cmd);
	if(rtn < 0) { //process died
		myPs->signal[signalNo]++;
	} else {
		myPs->signal[signalNo] = P0;
	}
	if(myPs->signal[signalNo] >= P3) {
		myPs->signal[signalNo] = P0;
		rtn = Load_Process(cmd2, "./", signalNo2);
	}
#endif
//shh_231124e
}

int DieCheck_Process(char *process)
{
	int rtn;
	char buf[32], cmd[256];
    FILE *fp;

	memset(buf, 0, sizeof buf);

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "rm -rf /root/");
	strcat(cmd, (char *)&myPs->misc.path1);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, process);
	strcat(cmd, "DieCheck.txt");
	system(cmd);
	//usleep(500000);
	sleep(1);
	// rm -rf /root/system_data/config/tmp/processDieCheck.txt

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "ps -ax | grep ./");
	strcat(cmd, process);
	strcat(cmd, " > /root/");
	strcat(cmd, (char *)&myPs->misc.path1);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, process);
	strcat(cmd, "DieCheck.txt");
	system(cmd);
	//usleep(500000);
	sleep(1);
	// ps -ax | grep ./process > /root/system_data/config/tmp/processDieCheck.txt

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "/root/");
	strcat(cmd, (char *)&myPs->misc.path1);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, process);
	strcat(cmd, "DieCheck.txt");
	// /root/system_data/config/tmp/processDieCheck.txt

    if((fp = fopen(cmd, "r")) != NULL) {
		memset(cmd, 0, sizeof cmd);
		strcpy(cmd, "./");
		strcat(cmd, process);

		memset(buf, 0, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		memset(buf, 0, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		memset(buf, 0, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		memset(buf, 0, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		memset(buf, 0, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		if(rtn > 0) {
			if(strcmp(buf, cmd) != 0) { //not equal
				if(strcmp(buf, "sh") != 0) { // != sh
					rtn = -1;
					userlog(DEBUG_LOG, psName, "%s died %d : %s\n",
						process, rtn, buf);
				} else {
					rtn = -2;
					userlog(DEBUG_LOG, psName,
						"process(%s) check - sh : %d\n", process, rtn);
				}
			} else { //equal
				rtn = 0;
			}
		} else {
			userlog(DEBUG_LOG, psName, "process check : %s\n", process);
			rtn = 0;
		}
		fclose(fp);
	} else {
		rtn = -10;
		userlog(DEBUG_LOG, psName, "%s died2 %d\n", process, rtn);
	}
	
	if(rtn == -1) {
		memset(cmd, 0, sizeof cmd);
		strcpy(cmd, "cp -rf /root/");
		strcat(cmd, (char *)&myPs->misc.path1);
		strcat(cmd, "/config/tmp/");
		strcat(cmd, process);
		strcat(cmd, "DieCheck.txt /root/");
		strcat(cmd, (char *)&myPs->misc.path1);
		strcat(cmd, "config/tmp/");
		strcat(cmd, process);
		strcat(cmd, "DieCheck2.txt");
		system(cmd);
		//usleep(500000);
		sleep(1);
		// cp -rf /root/system_data/config/tmp/processDieCheck.txt
		// /root/system_data/config/tmp/processDieCheck2.txt
	}

	return rtn;
}

int DieCheck_Process_2(char *process, char *index)
{
	int rtn;
	char buf[32], cmd[256];
    FILE *fp;

	memset(buf, 0, sizeof buf);

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "rm -rf /root/");
	strcat(cmd, (char *)&myPs->misc.path1);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, process);
	strcat(cmd, index);
	strcat(cmd, "DieCheck.txt");
	system(cmd);
	usleep(500000);
	// rm -rf /root/system_data/config/tmp/process#DieCheck.txt

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "ps -ax | grep './");
	strcat(cmd, process);
	strcat(cmd, " ");
	strcat(cmd, index);
	strcat(cmd, "' > /root/");
	strcat(cmd, (char *)&myPs->misc.path1);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, process);
	strcat(cmd, index);
	strcat(cmd, "DieCheck.txt");
	system(cmd);
	usleep(500000);
	// ps -ax | grep './process #' > /root/system_data/config/tmp/process#DieCheck.txt

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "/root/");
	strcat(cmd, (char *)&myPs->misc.path1);
	strcat(cmd, "/config/tmp/");
	strcat(cmd, process);
	strcat(cmd, index);
	strcat(cmd, "DieCheck.txt");
	// /root/system_data/config/tmp/process#DieCheck.txt

    if((fp = fopen(cmd, "r")) != NULL) {
		memset(cmd, 0, sizeof cmd);
		strcpy(cmd, "./");
		strcat(cmd, process);

		memset(buf, 0, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		memset(buf, 0, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		memset(buf, 0, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		memset(buf, 0, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		memset(buf, 0, sizeof buf);
    	rtn = fscanf(fp, "%s", buf);
		if(rtn > 0) {
			if(strcmp(buf, cmd) != 0) { //not equal
				if(strcmp(buf, "sh") != 0) { // != sh
					rtn = -1;
					userlog(DEBUG_LOG, psName, "%s%s died %d : %s\n",
						process, index, rtn, buf);
				} else {
					rtn = -2;
					userlog(DEBUG_LOG, psName,
						"process(%s%s) check - sh : %d\n", process, index, rtn);
				}
			} else { //equal
				rtn = 0;
			}
		} else {
			userlog(DEBUG_LOG, psName, "process check : %s%s\n",
				process, index);
			rtn = 0;
		}
		fclose(fp);
	} else {
		rtn = -10;
		userlog(DEBUG_LOG, psName, "%s%s died2 %d\n", process, index, rtn);
	}
	
	if(rtn == -1) {
		memset(cmd, 0, sizeof cmd);
		strcpy(cmd, "cp -rf /root/");
		strcat(cmd, (char *)&myPs->misc.path1);
		strcat(cmd, "/config/tmp/");
		strcat(cmd, process);
		strcat(cmd, index);
		strcat(cmd, "DieCheck.txt /root/");
		strcat(cmd, (char *)&myPs->misc.path1);
		strcat(cmd, "/config/tmp/");
		strcat(cmd, process);
		strcat(cmd, index);
		strcat(cmd, "DieCheck2.txt");
		system(cmd);
		usleep(500000);
		// cp -rf /root/system_data/config/tmp/process#DieCheck.txt
		// /root/system_data/config/tmp/process#DieCheck2.txt
	}
	return rtn;
}

int Read_ConfigFiles(int startType)
{
	int rtn, rtn1; //kjhw_150717 rtn1

	rtn = Read_Log_Config();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Log_Config fail %d\n", rtn);
		return -1;
	}

	rtn = Read_AppControl_Config(startType);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_AppControl_Config fail %d\n", rtn);
		return -2;
	}

	rtn = Read_mControl_Config();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_mControl_Config fail %d\n", rtn);
		return -3;
	}

	rtn = Read_Mux_Scan();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Mux_Scan fail %d\n", rtn);
		return -4;
	}

	rtn = Read_Load_Process_Config();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Load_Process_Config fail %d\n", rtn);
		return -5;
	}

	rtn = Read_Addr_Map_AIO();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Addr_Map_AIO fail %d\n", rtn);
		return -11;
	}

	rtn = Read_DIO_Config();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_DIO_Config fail %d\n", rtn);
		return -12;
	}

	rtn = Read_II_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_II_Function_Set fail %d\n", rtn);
		return -13;
	}

	rtn = Read_IO_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_IO_Function_Set fail %d\n", rtn);
		return -14;
	}

	rtn = Read_MI_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_MI_Function_Set fail %d\n", rtn);
		return -15;
	}

	rtn = Read_MO_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_MO_Function_Set fail %d\n", rtn);
		return -16;
	}

	rtn = Read_CI_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_CI_Function_Set fail %d\n", rtn);
		return -17;
	}

	rtn = Read_CO_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_CO_Function_Set fail %d\n", rtn);
		return -18;
	}

	rtn = Read_EI_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_EI_Function_Set fail %d\n", rtn);
		return -19;
	}

	rtn = Read_EO_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_EO_Function_Set fail %d\n", rtn);
		return -20;
	}

	rtn = Read_Calibration_Data();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Calibration_Data fail %d\n", rtn);
		return -21;
	}

	rtn = Read_CellArray_A();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_CellArray_A fail %d\n", rtn);
		return -31;
	}

	//rtn = Read_ChAttribute();
	//if(rtn < 0) {
	//	userlog(DEBUG_LOG, psName, "Read_ChAttribute fail %d\n", rtn);
	//	return -41;
	//}
	rtn = Read_ChAttribute(); //kjhw_180219s
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_ChAttribute fail %d\n", rtn);
		rtn1 = Write_ChAttribute_1();
		if(rtn1 < 0) {
			userlog(DEBUG_LOG, psName,
				"Read_ChAttribute fail after Write fail %d\n", rtn);
			return -41;
		} else {
			rtn = Read_ChAttribute();
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "Read_ChAttribute fail2 %d\n", rtn);
				return -41;
			}
			userlog(DEBUG_LOG, psName, "Read_ChAttribute retry OK\n");
		}
	} //kjhw_180219e

	//jhkw_190714s
	/*rtn = Read_AuxSetData();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_AuxSetData fail %d\n", rtn);
		return -42;
	}*/
	rtn = Read_AuxSetData();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_AuxSetData fail %d\n", rtn);
		
		rtn1 = Write_AuxSetData();
		if(rtn1 < 0) {
			userlog(DEBUG_LOG, psName,
				"Read_AuxSetData fail after Write fail %d\n", rtn1);
			return -42;
		} else {
			rtn = Read_AuxSetData();
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"Read_AuxSetData fail2 %d\n", rtn);
				return -42;
			}
			userlog(DEBUG_LOG, psName, "Read_AuxSetData retry OK\n");
		}
	}
	//jhkw_190714s

	rtn = Read_CanConfig();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_CanConfig fail %d\n", rtn);
		return -45;
	}

	//rtn = Read_CanReceiveSetData();
	//if(rtn < 0) {
	//	userlog(DEBUG_LOG, psName, "Read_CanReceiveSetData fail %d\n", rtn);
	//	return -46;
	//}
	//kjhw_150717s
	rtn = Read_CanReceiveSetData();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_CanReceiveSetData fail %d\n", rtn);

		rtn1 = Write_CanReceiveSetData();
		if(rtn1 < 0) {
			userlog(DEBUG_LOG, psName,
				"Read_CanReceiveSetData fail after Write fail %d\n", rtn1);
			return -46;
		} else {
			rtn = Read_CanReceiveSetData();
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"Read_CanReceiveSetData fail2 %d\n", rtn);
				return -46;
			}

			userlog(DEBUG_LOG, psName, "Read_CanReceiveSetData retry OK\n");
		}
	} //kjhw_150717e

	//rtn = Read_CanTransmitSetData();
	//if(rtn < 0) {
	//	userlog(DEBUG_LOG, psName, "Read_CanTransmitSetData fail %d\n", rtn);
	//	return -47;
	//}
	//kjhw_150716s
	rtn = Read_CanTransmitSetData();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_CanTransmitSetData fail %d\n", rtn);

		rtn1 = Write_CanTransmitSetData();
		if(rtn1 < 0) {
			userlog(DEBUG_LOG, psName,
				"Read_CanTransmitSetData fail after Write fail %d\n", rtn1);
			return -47;
		} else {
			rtn = Read_CanTransmitSetData();
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName,
					"Read_CanTransmitSetData fail2 %d\n", rtn);
				return -47;
			}

			userlog(DEBUG_LOG, psName, "Read_CanTransmitSetData retry OK\n");
		}
	} //kjhw_150716e

	rtn = Read_COM_Config();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_COM_Config fail %d\n", rtn);
		return -50;
	}

//20190902 KHK--------------------------------------
	rtn = Read_DAQ_Config();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_DAQ_Config fail %d\n", rtn);
		return -51;
	}
//--------------------------------------------------

	rtn = Read_Measure_Cali_1();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Measure_Cali_1 fail %d\n", rtn);
		return -52;
	}

	rtn = Read_Th_Table();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Th_Table fail %d\n", rtn);
		return -60;
	}

	rtn = Read_CanFlashFile();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_CanFlashFile fail %d\n", rtn);
		return -70;
	}

	//jhkw_201117s
	rtn = Read_Daq_Map();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Daq_Map fail %d\n", rtn);
		return -22;
	}
	//jhkw_201117e
	
//kjg_d
//	if(Read_KJG_Test_1() < 0) return -100;
//	if(Write_KJG_Test_1() < 0) return -101;
//	if(Read_KJG_Test_2() < 0) return -102;
//	if(Write_KJG_Test_2() < 0) return -103;

	return 0;
}

int Read_Log_Config(void)
{
	char temp[32], buf[32], fileName[256];
	int tmp, i;
	FILE *fp;

	memset(fileName, 0, sizeof fileName);
	sprintf(fileName, "/root/%s/config/parameter/Log_Config", myPs->misc.path1);
	if((fp = fopen(fileName, "r")) == NULL) {
		printf("Log_Config file read error\n");
		return -1;
	}

	//logflag 0:logoff 1:file 2:stdout 3:file&stdout 4:socket 7:all
	for(i=0; i < MAX_LOG; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(atoi(temp) != i) {
			fclose(fp);
			return -2;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		switch(i) {
			case 0:
				if(strcmp(temp, "DEBUG_LOG") != 0) {
					fclose(fp);
					return -2;
				}
				break;
			case 1:
				if(strcmp(temp, "METER_LOG") != 0) {
					fclose(fp);
					return -2;
				}
				break;
			case 2:
				if(strcmp(temp, "METER2_LOG") != 0) {
					fclose(fp);
					return -2;
				}
				break;
			case 3:
				if(strcmp(temp, "COA_LOG") != 0) {
					fclose(fp);
					return -2;
				}
				break;
			case 4:
				if(strcmp(temp, "COB_LOG") != 0) {
					fclose(fp);
					return -2;
				}
				break;
			case 5:
				if(strcmp(temp, "COC_LOG") != 0) {
					fclose(fp);
					return -2;
				}
				break;
			default:
				if(strcmp(temp, "reserved") != 0) {
					fclose(fp);
					return -2;
				}
				break;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		tmp = atoi(buf);
		if(tmp < 0 || tmp > 3) {
			fclose(fp);
			return -2;
		}

		myData->log[i].LogFlag = (unsigned char)tmp;

		if(strcmp(temp, "reserved") != 0) {
			Init_Logfile(temp, i);
		}
	}

	fclose(fp);

	return 0;
}

int Read_AppControl_Config(int startType)
{
	char temp[32], buf[48], fileName[256];
    int tmp;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/AppControl_Config");
	// /root/system_data/config/parameter/AppControl_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "AppControl_Config file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "bootOnStart") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.bootOnStart = (unsigned char)atoi(buf);//0:exit, 1:execute
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "modelName") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
    	fclose(fp);
		return -2;
	}
	tmp = fscanf(fp, "%s", myPs->config.modelName);
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "moduleNo") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.moduleNo = atoi(buf);
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "totalGroup") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.totalGroup = (short int)atoi(buf);
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "systemModel") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.systemModel = atoi(buf);
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "systemType") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:formation, 1:general_cycler, 2:ocv, 3:acir/ocv, 4:aging, 5:grader
	//6:selector, 7:pack_cycler, 8:triangle_cycler
	myPs->config.systemType = (unsigned char)atoi(buf);
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "sbcType") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//1:hs_6637, 2:web_6580, 3:hs_4020, 4:wafer_e669, 5:wafer_lx800
	//6:wafer_mark533, 7:em104_a5362, 8:wafer_mark800, 9:emcore_v621_533
	myPs->config.sbcType = (unsigned char)atoi(buf);
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "osVersion") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:kernel v.2.0, RTLinux v.1.3
	//1:kernel v.2.2, RTLinux v.2.0
	//2:kernel v.2.4, RTLinux v.3.2
	//3:kernel v.2.6, RTLinux v.3.2
	myPs->config.osVersion = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "debugType") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.debugType = (unsigned char)atoi(buf);
	//0:normal
	//1~12 : out_relay_select
	//		F_SDI_5V_400A_200A_100A_10A, F_SDI_5V_400A_200A_100A_10A_2
	//100 : tray_id(K00002) F_SDI_5V
	//110 : software_test

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "versionNo") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.versionNo = atoi(buf);

    fclose(fp);

	if(startType == START_BOOT && myPs->config.bootOnStart == START_FORCE)
		return -3;
	return 0;
}

int Read_mControl_Config(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp, i;
	unsigned char ratioV, ratioI, ratioP;	//jhkw_231127
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/mControl_Config");
	// /root/system_data/config/parameter/mControl_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "mControl_Config file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedBd") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.installedBd = atoi(buf);
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedCh") != 0) {
		fclose(fp);
		return -3;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -3;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.installedCh = atoi(buf);
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "chPerBd") != 0) {
		fclose(fp);
		return -4;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -4;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.chPerBd = atoi(buf);
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "chInGroup") != 0) {
		fclose(fp);
		return -5;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -5;
	}
	for(i=0; i < MAX_GROUP_8; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.chInGroup[i] = atoi(buf);
	}
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "bdInGroup") != 0) {
		fclose(fp);
		return -6;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -6;
	}
	for(i=0; i < MAX_GROUP_8; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.bdInGroup[i] = atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "chInBd") != 0) {
		fclose(fp);
		return -7;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -7;
	}
	for(i=0; i < MAX_BD_16; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.chInBd[i] = atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "chCheckDelayTime") != 0) {
		fclose(fp);
		return -8;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -8;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.chCheckDelayTime = atol(buf);
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "rangeV") != 0) {
		fclose(fp);
		return -9;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -9;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.rangeV = (unsigned char)atoi(buf);
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "rangeI") != 0) {
		fclose(fp);
		return -10;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -10;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.rangeI = (unsigned char)atoi(buf);
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "maxV") != 0) {
		fclose(fp);
		return -11;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -11;
	}
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.maxV[i] = atol(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "minV") != 0) {
		fclose(fp);
		return -12;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -12;
	}
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.minV[i] = atol(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "maxI") != 0) {
		fclose(fp);
		return -13;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -13;
	}
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.maxI[i] = atol(buf);
	}
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "minI") != 0) {
		fclose(fp);
		return -14;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -14;
	}
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.minI[i] = atol(buf);
	}
//kjhw_130903
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "maxP") != 0) {
		fclose(fp);
		return -15;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -15;
	}
	for(i=0; i < MAX_CH_4; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.maxP[i] = atol(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ratioV") != 0) {
		fclose(fp);
		return -16;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -16;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:uV, 1:mV, 2:V
	myData->mData.config.ratioV = (unsigned char)atoi(buf);
	ratioV = myData->mData.config.ratioV;	//jhkw_231127

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ratioI") != 0) {
		fclose(fp);
		return -17;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -17;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:uA, 1:mA, 2:A
	myData->mData.config.ratioI = (unsigned char)atoi(buf);
	ratioI = myData->mData.config.ratioI;	//jhkw_231127

	memset(temp, 0, sizeof temp); //jhkw_231127s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ratioP") != 0) {
		fclose(fp);
		return -18;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -18;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0:Not use, 1:mW, 2:W
	myData->mData.config.ratioP = (unsigned char)atoi(buf);
	ratioP = myData->mData.config.ratioP;
	
	if(((ratioV == MICRO_UNIT) && (ratioI == MICRO_UNIT)
		&& (ratioP == MILLI_UNIT))	//uV,uA,mW
		|| ((ratioV == MILLI_UNIT) && (ratioI == MILLI_UNIT)
		&& (ratioP == BASE_UNIT))) { //mV, mA, W
	} else {
		fclose(fp);
		return -118;
	}	//jhkw_231127e

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "adRatioV") != 0) {
		fclose(fp);
		return -19;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -19;
	}
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.adRatioV[i] = atof(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "adRatioI") != 0) {
		fclose(fp);
		return -20;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -20;
	}
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.adRatioI[i] = atof(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "daRatioV") != 0) {
		fclose(fp);
		return -21;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -21;
	}
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.daRatioV[i] = atof(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "daRatioI") != 0) {
		fclose(fp);
		return -22;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -22;
	}
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.daRatioI[i] = atof(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "shuntR") != 0) {
		fclose(fp);
		return -23;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -23;
	}
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.shuntR[i] = atof(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "scan_period") != 0) {
		fclose(fp);
		return -24;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -24;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.config.scan_period = atol(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ad_count") != 0) {
		fclose(fp);
		return -25;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -25;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//default:3, C_LGC_5V_200A_75A_15A, C_EIG_5V_50A_5A:5
    myData->mData.config.ad_count = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "filter_ad_count") != 0) {
		fclose(fp);
		return -26;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -26;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//default:4, C_LGC_5V_200A_75A_15A, C_EIG_5V_50A_5A:16
    myData->mData.config.filter_ad_count = (unsigned char)atoi(buf);

	//kjg_111006
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "tmp_avg_count") != 0) {
		fclose(fp);
		return -27;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -27;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.config.tmp_avg_count = (unsigned char)atoi(buf);

	//kjg_111006
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "dcr_t1_cnt1_t2_cnt2") != 0) {
		fclose(fp);
		return -28;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -28;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.config.dcr_t1 = (short int)atoi(buf); //ms
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.config.dcr_count1 = (short int)atoi(buf);
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.config.dcr_t2 = (short int)atoi(buf); //ms
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.config.dcr_count2 = (short int)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedTemp") != 0) {
		fclose(fp);
		return -29;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -29;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.config.installedTemp = (short int)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedAuxV") != 0) {
		fclose(fp);
		return -30;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -30;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.config.installedAuxV = (short int)atoi(buf);
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
	//kjh_160610
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedTH") != 0) {
		fclose(fp);
		return -31;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -31;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.config.installedTH = (short int)atoi(buf);
#endif
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedCAN") != 0) {
		fclose(fp);
		return -32;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -32;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.config.installedCAN = (short int)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedCOM") != 0) {
		fclose(fp);
		return -33;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -33;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->mData.config.installedCOM = (short int)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "daq_type") != 0) {
		fclose(fp);
		return -34;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -34;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//1:ver1.0 max_64ch
	//2:ver2.0 max_128ch, isolation
	//3:ver2.1 max_256ch, 128ch*2, isolation
	//4:ver3.0 max_128ch, iso3
	//5:ver3.1 max_256ch, iso3 & 128ch*2
	myData->mData.config.daq_type = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "soft_feed1_v_i_w_r") != 0) {
		fclose(fp);
		return -35;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -35;
	}
	for(i=0; i < 4; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.shuntR[i] = atof(buf);
		myData->mData.config.soft_feedback1[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "soft_feed2_i_w") != 0) {
		fclose(fp);
		return -36;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -36;
	}
	for(i=0; i < 2; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.shuntR[i] = atof(buf);
		myData->mData.config.soft_feedback2[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp); //lki_111111
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "step_0time_save") != 0) {
		fclose(fp);
		return -37;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -37;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.step_0time_save = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp); //kjg_120323
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "cable_check") != 0) {
		fclose(fp);
		return -38;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -38;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.cable_check = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp); //kjhw_140620
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "division_CAN") != 0) {
		fclose(fp);
		return -39;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -39;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.division_CAN = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp); //kjhw_150120
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "out_precharging_V") != 0) {
		fclose(fp);
		return -40;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -40;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.out_precharging_V = atol(buf);
	//0: not use,  
	//ex: uV : 5000000 = +-5V
	//ex: mV : 5000 = +-5V
	
	memset(temp, 0, sizeof temp);	//jhkw_231127s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "da_max") != 0) {
		fclose(fp);
		return -47;
	}
	memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -47;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.da_max = atof(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "main_amp") != 0) {
		fclose(fp);
		return -48;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -48;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.main_amp = atof(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "v_feed(vfb)") != 0) {
		fclose(fp);
		return -49;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -49;
	}
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.v_feed[i] = atof(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "i_feed(ad_HCT)") != 0) {
		fclose(fp);
		return -50;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -50;
	}
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.i_feed[i] = atof(buf);
	}

	memset(temp, 0, sizeof temp);	//kjh_211021s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "i_ref(dcdc)") != 0) {
		fclose(fp);
		return -51;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -51;
	}
	for(i=0; i < MAX_RANGE; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.config.i_ref[i] = atof(buf);
	}

	memset(temp, 0, sizeof temp);	//kjh_211021s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "caliV_rangeI") != 0) {
		fclose(fp);
		return -52;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -52;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.caliV_rangeI = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);	//kjh_211021s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "caliV_cmdI(ratio)") != 0) {
		fclose(fp);
		return -53;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -53;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.caliV_cmdI = (short int)atoi(buf);
	//jhkw_231127e
    fclose(fp);
	return 0;
}

int Read_Mux_Scan(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp, i;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/Mux_Scan");
	// /root/system_data/config/parameter/Mux_Scan
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Mux_Scan file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "total_scan_slot") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	tmp = atoi(buf);
	if(tmp > MAX_SLOT) tmp = MAX_SLOT;
	myData->mData.total_scan_slot = (short int)tmp;
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "total_ch_slot") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.total_ch_slot = (short int)atoi(buf);
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "total_ref_slot") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.total_ref_slot = (short int)atoi(buf);
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "slot") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "mux") != 0) {
		fclose(fp);
		return -2;
	}

	for(i=0; i < (int)myData->mData.total_scan_slot; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		if(atoi(buf) != (i+1)) {
			fclose(fp);
			return -2;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->mData.mux_scan[i] = (short int)atoi(buf);
	}
		
    fclose(fp);
	return 0;
}

int Read_Load_Process_Config(void)
{
	char temp[32], temp2[32], buf[32], fileName[256];
    int tmp, i, j;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/Load_Process_Config");
	// /root/system_data/config/parameter/Load_Process_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Load_Process_Config file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "DAQ") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Load") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Description") != 0) {
    	fclose(fp);
		return -2;
	}
	for(i=0; i < 9; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		switch(i) {
			case 0:
				if(strcmp(temp, "Use") != 0) {
   				 	fclose(fp);
					return -2;
				}
				break;
			default:
				memset(temp2, 0, sizeof temp2);
				sprintf(temp2, "%s%d", "P", i);
				if(strcmp(temp, temp2) != 0) {
   				 	fclose(fp);
					return -2;
				}
				break;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->AppControl.misc.Load_Process_DAQ[i] = (unsigned char)atoi(buf);
		tmp = fscanf(fp, "%s", temp);
	}
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CAN") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Load") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Description") != 0) {
    	fclose(fp);
		return -2;
	}
	for(i=0; i < 9; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		switch(i) {
			case 0:
				if(strcmp(temp, "Use") != 0) {
   				 	fclose(fp);
					return -2;
				}
				break;
			default:
				memset(temp2, 0, sizeof temp2);
				sprintf(temp2, "%s%d", "P", i);
				if(strcmp(temp, temp2) != 0) {
   				 	fclose(fp);
					return -2;
				}
				break;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->AppControl.misc.Load_Process_CAN[i] = (unsigned char)atoi(buf);
		tmp = fscanf(fp, "%s", temp);
	}
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "COM") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Load") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Description") != 0) {
    	fclose(fp);
		return -2;
	}
	for(i=0; i < 9; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		switch(i) {
			case 0:
				if(strcmp(temp, "Use") != 0) {
   				 	fclose(fp);
					return -2;
				}
				break;
			default:
				memset(temp2, 0, sizeof temp2);
				sprintf(temp2, "%s%d", "P", i);
				if(strcmp(temp, temp2) != 0) {
   				 	fclose(fp);
					return -2;
				}
				break;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->AppControl.misc.Load_Process_COM[i] = (unsigned char)atoi(buf);
		tmp = fscanf(fp, "%s", temp);
	}
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Proc(No)") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "COA") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "COB") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "COC") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "COD") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "JIG") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ETC") != 0) {
    	fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Description(ETC)") != 0) {
    	fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_GROUP_8; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "P", i+1);
		if(strcmp(temp, temp2) != 0) {
   		 	fclose(fp);
			return -2;
		}
		for(j=0; j < MAX_PROCESS_GROUP; j++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->AppControl.misc.Load_Process[j][i]
				= (unsigned char)atoi(buf);
		}
		tmp = fscanf(fp, "%s", temp);
	}
		
    fclose(fp);
	return 0;
}

int Read_DIO_Config(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/DIO_Config");
	// /root/system_data/config/parameter/DIO_Setting
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "IO_Setting file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "io_Control_Flag") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
	//0:unuse, 1:use
    myData->dio.config.dio_Control_Flag = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp); //kjg_logic_type_140324_s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "watchdogType") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->dio.config.watchdogType = (unsigned char)atoi(buf); //kjg_logic_type_140324_e

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "sensCount") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.sensCount = atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ioDelay") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.dioDelay = atol(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "scan_period") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.scan_period = atol(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "powerSwitchTimeout") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.powerSwitchTimeout = atol(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "forcePowerSwitchTimeout") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.forcePowerSwitchTimeout = atol(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "resetSwitchTimeout") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.resetSwitchTimeout = atol(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "powerFailTimeout1") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.powerFailTimeout1 = atol(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "powerFailTimeout2") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.powerFailTimeout2 = atol(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "upsBatteryFailTimeout") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.upsBatteryFailTimeout = atol(buf);
	
	memset(temp, 0, sizeof temp); //kjg_logic_type_140324_s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "logic_type") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->dio.config.logic_type = (unsigned char)atoi(buf);

	if(myData->dio.config.logic_type != 0) {
		if(myData->dio.config.watchdogType == 1) {
			myData->dio.config.watchdogType = 2;
		}
	} //kjg_logic_type_140324_e
    memset(temp, 0, sizeof temp); //phb_fan_monitoring_221223s //shh_230607s
    tmp = fscanf(fp, "%s", temp); 
    if(strcmp(temp, "fan_err_detect_time") != 0) {
        fclose(fp);
        return -2;
    }
    memset(temp, 0, sizeof temp);
    tmp = fscanf(fp, "%s", temp);
    if(strcmp(temp, ":") != 0) {
        fclose(fp);
        return -2;
    }
    memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->dio.config.fan_err_detect_time = (unsigned char)atoi(buf);	//phb_221223e //shh_230607e
	
    fclose(fp);
	return 0;
}

int Read_Addr_Map_AIO(void)
{
	char temp[32], temp2[32], buf[32], fileName[256];
    int tmp, i;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/io_map/Addr_Map_AIO");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Addr_Map_AIO file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "BD_BASE_ADDR") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->addr_map.BD_BASE_ADDR = (short int)strtol(buf, (char **)NULL, 16);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "BD_STEP") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->addr_map.BD_STEP = (unsigned char)strtol(buf, (char **)NULL, 16);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "BD_ADDR_DIV") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
    tmp = fscanf(fp, "%s", buf);
    myData->addr_map.BD_ADDR_DIV
		= (unsigned char)strtol(buf, (char **)NULL, 16);

	for(i=0; i < MAX_CH_8; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "MUX_", i+1);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->addr_map.MUX[i] = (unsigned char)strtol(buf, (char **)NULL, 16);
	}

	for(i=0; i < MAX_CH_8; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "DAV_SYNC_", i+1);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->addr_map.DAV_SYNC[i]
			= (unsigned char)strtol(buf, (char **)NULL, 16);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "DAV_DATA_H_", i+1);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->addr_map.DAV_DATA_H[i]
			= (unsigned char)strtol(buf, (char **)NULL, 16);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "DAV_DATA_L_", i+1);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->addr_map.DAV_DATA_L[i]
			= (unsigned char)strtol(buf, (char **)NULL, 16);
	}

	for(i=0; i < MAX_CH_8; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "DAI_SYNC_", i+1);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->addr_map.DAI_SYNC[i]
			= (unsigned char)strtol(buf, (char **)NULL, 16);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "DAI_DATA_H_", i+1);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->addr_map.DAI_DATA_H[i]
			= (unsigned char)strtol(buf, (char **)NULL, 16);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "DAI_DATA_L_", i+1);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->addr_map.DAI_DATA_L[i]
			= (unsigned char)strtol(buf, (char **)NULL, 16);
	}

	for(i=0; i < MAX_CH_8; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "ADV_SYNC_", i+1);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->addr_map.ADV_SYNC[i]
			= (unsigned char)strtol(buf, (char **)NULL, 16);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "ADV_DATA_H_", i+1);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->addr_map.ADV_DATA_H[i]
			= (unsigned char)strtol(buf, (char **)NULL, 16);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "ADV_DATA_L_", i+1);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->addr_map.ADV_DATA_L[i]
			= (unsigned char)strtol(buf, (char **)NULL, 16);
	}

	for(i=0; i < MAX_CH_8; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "ADI_SYNC_", i+1);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->addr_map.ADI_SYNC[i]
			= (unsigned char)strtol(buf, (char **)NULL, 16);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "ADI_DATA_H_", i+1);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->addr_map.ADI_DATA_H[i]
			= (unsigned char)strtol(buf, (char **)NULL, 16);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d", "ADI_DATA_L_", i+1);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->addr_map.ADI_DATA_L[i]
			= (unsigned char)strtol(buf, (char **)NULL, 16);
	}

    fclose(fp);
	return 0;
}

int Read_II_Function_Set(void)
{
	char temp[32], temp2[32], buf[32], fileName[256];
    int tmp, i, j, k, point, byte_index, comp;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/io_map/II_Function_Set");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "II_Function_Set file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "No") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Cnt") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Use") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "H/L") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Ch") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Group") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Function") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Description") != 0) {
		fclose(fp);
		return -2;
	}

	for(i=0; i < MAX_DIO_II_BYTES; i++) {
		byte_index = i;

		memset(temp, 0, sizeof temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d%s", "byte", i+1, "(in)");
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->dio.function_set.in_address[byte_index]
			= (int)strtol(buf, (char **)NULL, 16);

		for(j=0; j < BITS_PER_BYTE; j++) {
			point = byte_index * BITS_PER_BYTE + j;

			//No
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			tmp = atoi(temp);
			if(tmp != (i*8 + j)) {
				fclose(fp);
				return -2;
			}

			//Count
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].count
				= (unsigned char)atoi(buf);

			//Use
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].use
				= (unsigned char)atoi(buf);

			//H/L
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].pn
				= (unsigned char)atoi(buf);

			//Ch
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].ch
				= (short int)atoi(buf);

			//Group
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].group
				= (unsigned char)atoi(buf);

			//Function
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			if(myData->dio.function_set.in_set[point].use == USE) {
				comp = 0;
				for(k=0; k < sizeof(DIO_IN_NAME) / sizeof(*DIO_IN_NAME); k++) {
					if(strcmp(buf, DIO_IN_NAME[k].name) == 0) {
						myData->dio.function_set.in_set[point].function
							= (short int)DIO_IN_NAME[k].no;
						comp = 1;
						break;
					} else if(strcmp(buf, "spare") == 0) {
						myData->dio.function_set.in_set[point].function
							= I_SPARE;
						comp = 2;
						break;
					}
				}

				if(comp == 0) {
					userlog(DEBUG_LOG, psName,
						"II_Function_Set name error %s\n", buf);
					fclose(fp);
					return -2;
				}
			} else {
				myData->dio.function_set.in_set[point].function = I_SPARE;
			}

			//Description
			tmp = fscanf(fp, "%s", temp);
		}
	}

    fclose(fp);
	return 0;
}

int Read_IO_Function_Set(void)
{
	char temp[32], temp2[32], buf[32], fileName[256];
    int tmp, i, j, k, point, byte_index, comp;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/io_map/IO_Function_Set");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "IO_Function_Set file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "No") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Bit") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Use") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "H/L") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Ch") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Group") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Function") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Description") != 0) {
		fclose(fp);
		return -2;
	}

	for(i=0; i < MAX_DIO_IO_BYTES; i++) {
		byte_index = i;

		memset(temp, 0, sizeof temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d%s", "byte", i+1, "(out)");
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->dio.function_set.out_address[byte_index]
			= (int)strtol(buf, (char **)NULL, 16);

		for(j=0; j < BITS_PER_BYTE; j++) {
			point = byte_index * BITS_PER_BYTE + j;

			//No
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			tmp = atoi(temp);
			if(tmp != (i*8 + j)) {
				fclose(fp);
				return -2;
			}

			//Bit
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			tmp = atoi(temp);
			if(tmp != j) {
				fclose(fp);
				return -2;
			}

			//Use
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].use
				= (unsigned char)atoi(buf);

			//H/L
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].pn
				= (unsigned char)atoi(buf);

			//Ch
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].ch
				= (short int)atoi(buf);

			//Group
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].group
				= (unsigned char)atoi(buf);

			//Function
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			if(myData->dio.function_set.out_set[point].use == USE) {
				comp = 0;
				for(k=0; k < sizeof(DIO_OUT_NAME) / sizeof(*DIO_OUT_NAME);
					k++) {
					if(strcmp(buf, DIO_OUT_NAME[k].name) == 0) {
						myData->dio.function_set.out_set[point].function
							= (short int)DIO_OUT_NAME[k].no;
						comp = 1;
						break;
					} else if(strcmp(buf, "spare") == 0) {
						myData->dio.function_set.out_set[point].function
							= O_SPARE;
						comp = 2;
						break;
					}
				}

				if(comp == 0) {
					userlog(DEBUG_LOG, psName,
						"IO_Function_Set name error %s\n", buf);
					fclose(fp);
					return -2;
				}
			} else {
				myData->dio.function_set.out_set[point].function = O_SPARE;
			}

			//Description
			tmp = fscanf(fp, "%s", temp);
		}
	}

    fclose(fp);
	return 0;
}

int Read_MI_Function_Set(void)
{
	char temp[32], temp2[32], buf[32], fileName[256];
    int tmp, i, j, k, point, byte_index, comp;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/io_map/MI_Function_Set");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "MI_Function_Set file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "No") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Cnt") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Use") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "H/L") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Ch") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Group") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Function") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Description") != 0) {
		fclose(fp);
		return -2;
	}

	for(i=0; i < MAX_DIO_MI_BYTES; i++) {
		byte_index = MAX_DIO_II_BYTES + i;

		memset(temp, 0, sizeof temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d%s", "byte", i+1, "(in)");
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->dio.function_set.in_address[byte_index]
			= (int)strtol(buf, (char **)NULL, 16);

		for(j=0; j < BITS_PER_BYTE; j++) {
			point = byte_index * BITS_PER_BYTE + j;

			//No
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			tmp = atoi(temp);
			if(tmp != (i*8 + j)) {
				fclose(fp);
				return -2;
			}

			//Count
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].count
				= (unsigned char)atoi(buf);

			//Use
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].use
				= (unsigned char)atoi(buf);

			//H/L
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].pn
				= (unsigned char)atoi(buf);

			//Ch
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].ch
				= (short int)atoi(buf);

			//Group
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].group
				= (unsigned char)atoi(buf);

			//Function
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			if(myData->dio.function_set.in_set[point].use == USE) {
				comp = 0;
				for(k=0; k < sizeof(DIO_IN_NAME) / sizeof(*DIO_IN_NAME); k++) {
					if(strcmp(buf, DIO_IN_NAME[k].name) == 0) {
						myData->dio.function_set.in_set[point].function
							= (short int)DIO_IN_NAME[k].no;
						comp = 1;
						break;
					} else if(strcmp(buf, "spare") == 0) {
						myData->dio.function_set.in_set[point].function
							= I_SPARE;
						comp = 2;
						break;
					}
				}

				if(comp == 0) {
					userlog(DEBUG_LOG, psName,
						"MI_Function_Set name error %s\n", buf);
					fclose(fp);
					return -2;
				}
			} else {
				myData->dio.function_set.in_set[point].function = I_SPARE;
			}

			//Description
			tmp = fscanf(fp, "%s", temp);
		}
	}

    fclose(fp);
	return 0;
}

int Read_MO_Function_Set(void)
{
	char temp[32], temp2[32], buf[32], fileName[256];
    int tmp, i, j, k, point, byte_index, comp;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/io_map/MO_Function_Set");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "MO_Function_Set file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "No") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Bit") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Use") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "H/L") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Ch") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Group") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Function") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Description") != 0) {
		fclose(fp);
		return -2;
	}

	for(i=0; i < MAX_DIO_MO_BYTES; i++) {
		byte_index = MAX_DIO_IO_BYTES + i;

		memset(temp, 0, sizeof temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d%s", "byte", i+1, "(out)");
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->dio.function_set.out_address[byte_index]
			= (int)strtol(buf, (char **)NULL, 16);

		for(j=0; j < BITS_PER_BYTE; j++) {
			point = byte_index * BITS_PER_BYTE + j;

			//No
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			tmp = atoi(temp);
			if(tmp != (i*8 + j)) {
				fclose(fp);
				return -2;
			}

			//Bit
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			tmp = atoi(temp);
			if(tmp != j) {
				fclose(fp);
				return -2;
			}

			//Use
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].use
				= (unsigned char)atoi(buf);

			//H/L
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].pn
				= (unsigned char)atoi(buf);

			//Ch
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].ch
				= (short int)atoi(buf);

			//Group
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].group
				= (unsigned char)atoi(buf);

			//Function
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			if(myData->dio.function_set.out_set[point].use == USE) {
				comp = 0;
				for(k=0; k < sizeof(DIO_OUT_NAME) / sizeof(*DIO_OUT_NAME);
					k++) {
					if(strcmp(buf, DIO_OUT_NAME[k].name) == 0) {
						myData->dio.function_set.out_set[point].function
							= (short int)DIO_OUT_NAME[k].no;
						comp = 1;
						break;
					} else if(strcmp(buf, "spare") == 0) {
						myData->dio.function_set.out_set[point].function
							= O_SPARE;
						comp = 2;
						break;
					}
				}

				if(comp == 0) {
					userlog(DEBUG_LOG, psName,
						"MO_Function_Set name error %s\n", buf);
					fclose(fp);
					return -2;
				}
			} else {
				myData->dio.function_set.out_set[point].function = O_SPARE;
			}

			//Description
			tmp = fscanf(fp, "%s", temp);
		}
	}

    fclose(fp);
	return 0;
}

int Read_CI_Function_Set(void)
{
	char temp[32], temp2[32], buf[32], fileName[256];
    int tmp, i, j, k, point, byte_index, comp;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/io_map/CI_Function_Set");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "CI_Function_Set file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "No") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Cnt") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Use") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "H/L") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Ch") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Group") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Function") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Description") != 0) {
		fclose(fp);
		return -2;
	}

	for(i=0; i < MAX_DIO_CI_BYTES; i++) {
		byte_index = MAX_DIO_II_BYTES + MAX_DIO_MI_BYTES + i;

		memset(temp, 0, sizeof temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d%s", "byte", i+1, "(in)");
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->dio.function_set.in_address[byte_index]
			= (int)strtol(buf, (char **)NULL, 16);

		for(j=0; j < BITS_PER_BYTE; j++) {
			point = byte_index * BITS_PER_BYTE + j;

			//No
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			tmp = atoi(temp);
			if(tmp != (i*8 + j)) {
				fclose(fp);
				return -2;
			}

			//Count
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].count
				= (unsigned char)atoi(buf);

			//Use
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].use
				= (unsigned char)atoi(buf);

			//H/L
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].pn
				= (unsigned char)atoi(buf);

			//Ch
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].ch
				= (short int)atoi(buf);

			//Group
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].group
				= (unsigned char)atoi(buf);

			//Function
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			if(myData->dio.function_set.in_set[point].use == USE) {
				comp = 0;
				for(k=0; k < sizeof(DIO_IN_NAME) / sizeof(*DIO_IN_NAME); k++) {
					if(strcmp(buf, DIO_IN_NAME[k].name) == 0) {
						myData->dio.function_set.in_set[point].function
							= (short int)DIO_IN_NAME[k].no;
						comp = 1;
						break;
					} else if(strcmp(buf, "spare") == 0) {
						myData->dio.function_set.in_set[point].function
							= I_SPARE;
						comp = 2;
						break;
					}
				}

				if(comp == 0) {
					userlog(DEBUG_LOG, psName,
						"CI_Function_Set name error %s\n", buf);
					fclose(fp);
					return -2;
				}
			} else {
				myData->dio.function_set.in_set[point].function = I_SPARE;
			}

			//Description
			tmp = fscanf(fp, "%s", temp);
		}
	}

    fclose(fp);
	return 0;
}

int Read_CO_Function_Set(void)
{
	char temp[32], temp2[32], buf[32], fileName[256];
    int tmp, i, j, k, point, byte_index, comp;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/io_map/CO_Function_Set");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "CO_Function_Set file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "No") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Bit") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Use") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "H/L") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Ch") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Group") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Function") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Description") != 0) {
		fclose(fp);
		return -2;
	}

	for(i=0; i < MAX_DIO_CO_BYTES; i++) {
		byte_index = MAX_DIO_IO_BYTES + MAX_DIO_MO_BYTES + i;

		memset(temp, 0, sizeof temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d%s", "byte", i+1, "(out)");
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->dio.function_set.out_address[byte_index]
			= (int)strtol(buf, (char **)NULL, 16);

		for(j=0; j < BITS_PER_BYTE; j++) {
			point = byte_index * BITS_PER_BYTE + j;

			//No
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			tmp = atoi(temp);
			if(tmp != (i*8 + j)) {
				fclose(fp);
				return -2;
			}

			//Bit
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			tmp = atoi(temp);
			if(tmp != j) {
				fclose(fp);
				return -2;
			}

			//Use
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].use
				= (unsigned char)atoi(buf);

			//H/L
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].pn
				= (unsigned char)atoi(buf);

			//Ch
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].ch
				= (short int)atoi(buf);

			//Group
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].group
				= (unsigned char)atoi(buf);

			//Function
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			if(myData->dio.function_set.out_set[point].use == USE) {
				comp = 0;
				for(k=0; k < sizeof(DIO_OUT_NAME) / sizeof(*DIO_OUT_NAME);
					k++) {
					if(strcmp(buf, DIO_OUT_NAME[k].name) == 0) {
						myData->dio.function_set.out_set[point].function
							= (short int)DIO_OUT_NAME[k].no;
						comp = 1;
						break;
					} else if(strcmp(buf, "spare") == 0) {
						myData->dio.function_set.out_set[point].function
							= O_SPARE;
						comp = 2;
						break;
					}
				}

				if(comp == 0) {
					userlog(DEBUG_LOG, psName,
						"CO_Function_Set name error %s\n", buf);
					fclose(fp);
					return -2;
				}
			} else {
				myData->dio.function_set.out_set[point].function = O_SPARE;
			}

			//userlog(DEBUG_LOG, psName, "%d %d %d\n", i, j,
			//	myData->dio.function_set.out_set[point].function); //kjg_d

			//Description
			tmp = fscanf(fp, "%s", temp);
		}
	}

    fclose(fp);
	return 0;
}

int Read_EI_Function_Set(void)
{
	char temp[32], temp2[32], buf[32], fileName[256];
    int tmp, i, j, k, point, byte_index, comp;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/io_map/EI_Function_Set");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "EI_Function_Set file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "No") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Cnt") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Use") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "H/L") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Ch") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Group") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Function") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Description") != 0) {
		fclose(fp);
		return -2;
	}

	for(i=0; i < MAX_DIO_EI_BYTES; i++) {
		byte_index = MAX_DIO_II_BYTES + MAX_DIO_MI_BYTES + MAX_DIO_CI_BYTES + i;

		memset(temp, 0, sizeof temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d%s", "byte", i+1, "(in)");
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->dio.function_set.in_address[byte_index]
			= (int)strtol(buf, (char **)NULL, 16);

		for(j=0; j < BITS_PER_BYTE; j++) {
			point = byte_index * BITS_PER_BYTE + j;

			//No
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			tmp = atoi(temp);
			if(tmp != (i*8 + j)) {
				fclose(fp);
				return -2;
			}

			//Count
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].count
				= (unsigned char)atoi(buf);

			//Use
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].use
				= (unsigned char)atoi(buf);

			//H/L
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].pn
				= (unsigned char)atoi(buf);

			//Ch
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].ch
				= (short int)atoi(buf);

			//Group
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.in_set[point].group
				= (unsigned char)atoi(buf);

			//Function
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			if(myData->dio.function_set.in_set[point].use == USE) {
				comp = 0;
				for(k=0; k < sizeof(DIO_IN_NAME) / sizeof(*DIO_IN_NAME); k++) {
					if(strcmp(buf, DIO_IN_NAME[k].name) == 0) {
						myData->dio.function_set.in_set[point].function
							= (short int)DIO_IN_NAME[k].no;
						comp = 1;
						break;
					} else if(strcmp(buf, "spare") == 0) {
						myData->dio.function_set.in_set[point].function
							= I_SPARE;
						comp = 2;
						break;
					}
				}

				if(comp == 0) {
					userlog(DEBUG_LOG, psName,
						"EI_Function_Set name error %s\n", buf);
					fclose(fp);
					return -2;
				}
			} else {
				myData->dio.function_set.in_set[point].function = I_SPARE;
			}

			//Description
			tmp = fscanf(fp, "%s", temp);
		}
	}

    fclose(fp);
	return 0;
}

int Read_EO_Function_Set(void)
{
	char temp[32], temp2[32], buf[32], fileName[256];
    int tmp, i, j, k, point, byte_index, comp;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/io_map/EO_Function_Set");
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "EO_Function_Set file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "No") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Bit") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Use") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "H/L") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Ch") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Group") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Function") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Description") != 0) {
		fclose(fp);
		return -2;
	}

	for(i=0; i < MAX_DIO_EO_BYTES; i++) {
		byte_index = MAX_DIO_IO_BYTES + MAX_DIO_MO_BYTES + MAX_DIO_CO_BYTES + i;

		memset(temp, 0, sizeof temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d%s", "byte", i+1, "(out)");
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->dio.function_set.out_address[byte_index]
			= (int)strtol(buf, (char **)NULL, 16);

		for(j=0; j < BITS_PER_BYTE; j++) {
			point = byte_index * BITS_PER_BYTE + j;

			//No
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			tmp = atoi(temp);
			if(tmp != (i*8 + j)) {
				fclose(fp);
				return -2;
			}

			//Bit
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			tmp = atoi(temp);
			if(tmp != j) {
				fclose(fp);
				return -2;
			}

			//Use
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].use
				= (unsigned char)atoi(buf);

			//H/L
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].pn
				= (unsigned char)atoi(buf);

			//Ch
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].ch
				= (short int)atoi(buf);

			//Group
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->dio.function_set.out_set[point].group
				= (unsigned char)atoi(buf);

			//Function
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			if(myData->dio.function_set.out_set[point].use == USE) {
				comp = 0;
				for(k=0; k < sizeof(DIO_OUT_NAME) / sizeof(*DIO_OUT_NAME);
					k++) {
					if(strcmp(buf, DIO_OUT_NAME[k].name) == 0) {
						myData->dio.function_set.out_set[point].function
							= (short int)DIO_OUT_NAME[k].no;
						comp = 1;
						break;
					} else if(strcmp(buf, "spare") == 0) {
						myData->dio.function_set.out_set[point].function
							= O_SPARE;
						comp = 2;
						break;
					}
				}

				if(comp == 0) {
					userlog(DEBUG_LOG, psName,
						"EO_Function_Set name error %s\n", buf);
					fclose(fp);
					return -2;
				}
			} else {
				myData->dio.function_set.out_set[point].function = O_SPARE;
			}

			//Description
			tmp = fscanf(fp, "%s", temp);
		}
	}

    fclose(fp);
	return 0;
}

int Read_Calibration_Data(void)
{
	int group, bd, rtn;

	switch(myData->AppControl.config.systemModel) {
		case F_PNE_5V_15A_30AP_SW:
			group = 0;
			//debug_size_cob
/*			memset((char *)&myData->f_cali, 0, sizeof(S_F_CALIBRATION));

			rtn = Read_Cali_Set_Main();
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "Read_Cali_Set_Main fail %d\n", rtn);
				return -1;
			}

			rtn = Read_Cali_Set_Ch();
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "Read_Cali_Set_Ch fail %d\n", rtn);
				return -2;
			}

			for(group=0; group < myData->AppControl.config.totalGroup;
				group++) {
				rtn = Read_Main_CaliData(group, 0);
				if(rtn < 0) {
					userlog(DEBUG_LOG, psName,
						"Read_Main_CaliData v fail group:%d %d\n", group, rtn);
					return -3;
				}

				rtn = Read_Main_CaliData(group, 1);
				if(rtn < 0) {
					userlog(DEBUG_LOG, psName,
						"Read_Main_CaliData i fail group:%d %d\n", group, rtn);
					return -4;
				}
			}

			for(bd=0; bd < myData->mData.config.installedBd; bd++) {
				for(group=0; group < myData->mData.config.rangeI; group++) {
					rtn = Read_Ch_CaliData(bd, group);
					if(rtn < 0) {
						userlog(DEBUG_LOG, psName,
							"Read_Ch_CaliData fail bd:%d range:%d %d\n",
							bd, group, rtn);
						return -5;
					}

					rtn = Read_Ch_Check_CaliData(bd, group);
					if(rtn < 0) {
						userlog(DEBUG_LOG, psName,
							"Read_Ch_Check_CaliData fail bd:%d range:%d %d\n",
							bd, group, rtn);
						return -6;
					}
				}
			}

			rtn = Read_MainAD_V_Offset();
			if(rtn < 0) {
				userlog(DEBUG_LOG, psName, "Read_MainAD_V_Offset fail %d\n",
					rtn);
				return -7;
			}*/
			break;
		default:
			memset((char *)&myData->cali, 0, sizeof(S_CALIBRATION));

			for(bd=0; bd < myData->mData.config.installedBd; bd++) {
				rtn = Read_Bd_CaliData(bd);
				if(rtn < 0) {
					userlog(DEBUG_LOG, psName,
						"Read_BdCali file(read) error %d %d\n", bd+1, rtn);
					return -1;
				}
			}
			break;
	}

	return 0;
}

int Read_Bd_CaliData(int bd)
{
	/*Simplication - shh_250417*/
	char fileName[256], temp[32], buf[32];
	int tmp, ch, type, range, point, read_skip=0, ch_start, ch_end;
	S_CALI_DATA data[MAX_CH_8][MAX_TYPE][MAX_RANGE];
	FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/caliData/CALI_BD");
	memset(buf, 0, sizeof buf);
	buf[0] = (char)(49+bd);
	strcat(fileName, buf);
	// /root/system_data/config/caliData/CALI_BD#

	memset(data, 0, sizeof(S_CALI_DATA) * MAX_CH_8 * MAX_TYPE * MAX_RANGE);

	if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"Can not open %d Board CaliData file(read)\n", bd+1);
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "MCTS_Calibration_File") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "KJG001-00") != 0) {
		fclose(fp);
		return -3;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "date") != 0) {
		fclose(fp);
		return -4;
	}
		
	for(type=0; type < MAX_TYPE; type++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(type == 0) {
			if(strcmp(temp, "voltage") != 0) {
				fclose(fp);
				return -5;
			}
		} else {
			if(strcmp(temp, "current") != 0) {
				fclose(fp);
				return -6;
			}
		}
		for(range=0; range < MAX_RANGE; range++) {
			if(read_skip == 0) {
				tmp = fscanf(fp, "%s", temp); //range1, 2, 3, 4
			} else read_skip = 0;

			for(ch=0; ch < MAX_CH_8; ch++) {
				memset(buf, 0, sizeof(buf));
				tmp = fscanf(fp, "%s", buf); //ch
				if(buf[0] == 'c' && buf[1] == 'h') {
					read_skip = 0;
				} else {
					read_skip = 1;
					break;
				}

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "setPointNum") != 0) {
					fclose(fp);
					return -7;
				}
				memset(buf, 0, sizeof(buf));
				tmp = fscanf(fp, "%s", buf);
				data[ch][type][range].point.setPointNum
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "setPoint") != 0) {
					fclose(fp);
					return -8;
				}
				for(point=0; point < MAX_CALI_POINT; point++) {
					memset(buf, 0, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch][type][range].point.setPoint[point]
						= atol(buf);
				}

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "checkPointNum") != 0) {
					fclose(fp);
					return -9;
				}
				memset(buf, 0, sizeof(buf));
				tmp = fscanf(fp, "%s", buf);
				data[ch][type][range].point.checkPointNum
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "checkPoint") != 0) {
					fclose(fp);
					return -10;
				}
				for(point=0; point < MAX_CALI_POINT; point++) {
					memset(buf, 0, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch][type][range].point.checkPoint[point]
						= atol(buf);
				}

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "set_ad") != 0) {
					fclose(fp);
					return -11;
				}
				for(point=0; point < MAX_CALI_POINT; point++) {
					memset(buf, 0, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch][type][range].set_ad[point] = atol(buf);
				}

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "set_meter") != 0) {
					fclose(fp);
					return -12;
				}
				for(point=0; point < MAX_CALI_POINT; point++) {
					memset(buf, 0, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch][type][range].set_meter[point] = atol(buf);
				}

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "check_ad") != 0) {
					fclose(fp);
					return -13;
				}
				for(point=0; point < MAX_CALI_POINT; point++) {
					memset(buf, 0, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch][type][range].check_ad[point] = atol(buf);
				}

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "check_meter") != 0) {
					fclose(fp);
					return -14;
				}
				for(point=0; point < MAX_CALI_POINT; point++) {
					memset(buf, 0, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch][type][range].check_meter[point] = atol(buf);
				}

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "DA_A") != 0) {
					fclose(fp);
					return -15;
				}
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					memset(buf, 0, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch][type][range].DA_A[point] = (double)atof(buf);
				}

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "DA_B") != 0) {
					fclose(fp);
					return -16;
				}
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					memset(buf, 0, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch][type][range].DA_B[point] = (double)atof(buf);
				}
	
				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "AD_A") != 0) {
					fclose(fp);
					return -17;
				}
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					memset(buf, 0, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch][type][range].AD_A[point] = (double)atof(buf);
				}

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "AD_B") != 0) {
					fclose(fp);
					return -18;
				}
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					memset(buf, 0, sizeof(buf));
					tmp = fscanf(fp, "%s", buf);
					data[ch][type][range].AD_B[point] = (double)atof(buf);
				}

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "AD_Ratio") != 0) {
					fclose(fp);
					return -19;
				}
				memset(buf, 0, sizeof(buf));
				tmp = fscanf(fp, "%s", buf);
				data[ch][type][range].AD_Ratio[0] = (double)atof(buf);
				memset(buf, 0, sizeof(buf));
				tmp = fscanf(fp, "%s", buf);
				data[ch][type][range].AD_Ratio[1] = (double)atof(buf);
			}
		}
	}

	fclose(fp);

	switch(myData->AppControl.config.systemModel) {
	
		case C_SKI_50V_500A_250A_25KW:	//ktg_190715
		case C_SKI_50V_500A_400A_200A_100A_100KW:	//jhk_180119
		case C_SKI_50V_500A_400A_200A_100A_100KW_2:	//jhk_180119
		case C_SKI_50V_1000A_500A_300A_100KW:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_2:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_3:	//ktg_190530
		case C_SKI_50V_1000A_500A_300A_100KW_4:	//ktg_190711
		case C_SKI_60V_400A_200A_100A_96KW:		//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_2:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_3:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_4:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_5:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_6:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_7:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_8:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_9:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_10:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_11:	//ktg_190319
		case C_SKI_60V_400A_200A_100A_96KW_12:	//ktg_190319
		case C_NORTHVOLT_60V_400A_200A_96KW:			//khj_191203
		case C_DAEHWA_60V_500A_250A_50A_60KW:	//jhk_160913
		case C_DAEHWA_60V_500A_250A_50A_60KW_2:	//jhk_160913
		case C_SKI_100V_300A_150A_50A_10A_60KW:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_2:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_3:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_4:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_5:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_6:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_7:	//kjh_120527
		case C_SKI_100V_300A_150A_50A_10A_60KW_8:	//kjh_120527
		case C_SKI_120V_400A_100A_192KW:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_2:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_3:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_4:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_5:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_6:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_7:	//jhk_170628
		case C_SKI_120V_400A_100A_192KW_8:	//jhk_170628
		case C_SKI_120V_400A_200A_100A_50A_192KW:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_2:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_3:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_4:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_5:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_6:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_7:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_8:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_9:		//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_10:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_11:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_12:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_13:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_14:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_15:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_16:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_17:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_50A_192KW_18:	//jhk_180206
		case C_SKI_120V_400A_200A_100A_192KW:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_2:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_3:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_4:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_5:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_6:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_7:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_8:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_9:		//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_10:	//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_11:	//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_12:	//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_13:	//ktg_190308
		case C_SKI_120V_400A_200A_100A_192KW_14:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_15:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_16:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_17:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_18:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_19:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_20:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_21:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_22:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_23:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_24:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_25:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_26:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_27:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_28:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_29:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_30:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_31:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_32:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_33:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_34:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_35:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_36:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_37:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_38:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_39:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_40:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_41:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_42:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_43:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_44:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_45:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_46:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_47:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_48:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_49:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_50:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_51:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_52:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_53:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_54:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_55:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_56:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_57:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_58:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_59:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_60:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_61:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_62:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_63:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_64:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_65:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_66:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_67:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_68:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_69:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_70:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_71:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_72:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_73:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_74:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_75:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_76:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_77:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_78:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_79:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_80:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_81:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_82:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_83:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_84:	//ktg_190805
		case C_SKI_120V_400A_200A_100A_192KW_85:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_86:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_87:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_88:	//ktg_191210
		case C_SKI_120V_400A_200A_100A_192KW_89:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_90:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_91:	//khj_200308
		////////////////////////////////////////////////////////
		case C_SKI_120V_400A_200A_100A_192KW_92:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_93:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_94:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_95:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_96:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_97:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_98:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_99:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_100:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_101:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_102:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_103:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_104:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_105:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_106:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_107:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_108:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_109:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_110:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_111:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_112:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_113:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_114:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_115:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_116:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_117:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_118:	//khj_200308
		case C_SKI_120V_400A_200A_100A_192KW_119:	//khj_200308
		////////////////////////////////////////////////////////
		//case C_SKI_120V_400A_200A_100A_192KW_169:	//ktg_200410 //shh_240730
		//case C_TEST_120V_400A_200A_100A_192KW:		//shh_200723
		case C_SKI_120V_400A_200A_100A_192KW_170:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_171:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_172:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_173:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_174:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_175:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_176:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_177:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_178:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_179:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_180:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_181:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_182:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_183:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_184:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_185:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_186:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_187:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_188:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_189:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_190:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_191:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_192:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_193:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_194:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_195:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_196:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_197:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_198:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_199:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_200:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_201:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_202:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_203:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_204:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_205:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_206:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_207:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_208:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_209:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_210:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_211:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_212:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_213:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_214:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_215:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_216:	//shh_200905
		case C_SKI_120V_400A_200A_100A_192KW_233:	//ktg_200908
		case C_SKI_120V_400A_200A_100A_192KW_234:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_235:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_236:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_237:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_238:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_239:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_240:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_241:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_242:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_243:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_244:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_245:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_246:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_247:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_248:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_249:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_250:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_251:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_252:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_253:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_254:	//shh_220113
		case C_SKI_120V_400A_200A_100A_192KW_255:	//shh_220113
		/* 22PPSCSA062  SKON China - 11Set //shh_250417s
		case C_SKI_120V_425A_200A_100A_50A_192KW:       //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_2:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_3:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_4:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_5:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_6:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_7:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_8:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_9:     //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_10:    //phb_230206
        case C_SKI_120V_425A_200A_100A_50A_192KW_11:    //phb_230206
		*/
		//case C_SKI_120V_425A_300A_200A_100A_192KW:      //phb_230105
        //case C_SKI_120V_425A_300A_200A_100A_192KW_2:    //phb_230105
        //case C_SKI_120V_425A_300A_200A_100A_192KW_3:    //phb_230105
		case C_SKI_180V_425A_200A_100A_192KW:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_2:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_3:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_4:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_5:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_6:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_7:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_8:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_9:		//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_10:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_11:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_12:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_13:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_14:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_15:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_16:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_17:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_18:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_19:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_20:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_21:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_22:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_23:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_24:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_25:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_26:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_27:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_28:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_29:	//sec_221221
		case C_SKI_180V_425A_200A_100A_192KW_30:	//sec_221221
		case C_SKI_180V_600A_300A_100A_320KW:       //phb_230220
		//case C_SKI_180V_600A_300A_100A_320KW_2:		//shh_231004
		case C_NORTHVOLT_200V_400A_200A_160KW:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_2:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_3:	//ktg_191203
		case C_NORTHVOLT_200V_400A_200A_160KW_4:	//ktg_191203
		//21PPSCSA052 SKON MOA 9SET - START //shh_250417
		//case C_SKI_260V_425A_200A_442KW:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_2:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_3:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_4:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_5:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_6:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_7:	//shh_211020
		//case C_SKI_260V_425A_200A_442KW_8:	//shh_211020
		//21PPSCSA052 SKON MOA 9SET - END
		case C_SKI_260V_425A_200A_442KW_9:	//shh_211020
		case C_NORTHVOLT_400V_200A_100A_160KW:		//ktg_200415
		case C_DAEWOO_450V_200A_50A_90KW:	//kjh_150519
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
		case C_SKI_450V_200A_100A_90KW:		//sec_221223
		case C_SKI_450V_150A_100A_50A_260KW:	//sec_221215
		case C_SKI_450V_150A_100A_50A_260KW_2:	//sec_221215
		case C_PNE_500V_250A_10A_125KW: //kjh_120510
		case C_SKI_500V_450A_200A_450KW:		//khj_200308
		case C_SKI_500V_450A_200A_450KW_2:		//khj_200308
		case C_SKI_500V_450A_200A_450KW_3:		//khj_200308
		//case C_SKI_500V_450A_200A_450KW_4:		//ktg_200410 //shh_240730s
		//case C_SKI_500V_450A_200A_450KW_5:		//ktg_200410
		//case C_SKI_500V_450A_200A_450KW_6:		//ktg_200410 //shh_240730e	
		case C_SKC_600V_200A_100A_50A_240KW: //jhkw_120807
		case C_SKC_600V_200A_100A_50A_240KW_2: //jhkw_121019
		case C_SKE_600V_400A_100A_50A_25A_240KW:	//jhkw_130924
		case C_DAEHWA_600V_400A_200A_100A_200KW:	//jhk_170120
		case C_DAEHWA_750V_300A_100A_150KW:	//jhk_170203
		case C_SKI_1000V_400A_200A_100A_450KW:	//sec_230103
		case C_SKI_1200V_250A_100A_50A_300KW:	//jhk_131209
		//case C_SKI_1200V_500A_400A_300A_200A_600KW:     //phb_230116
		//case C_SKI_1500V_350A_300A_200A_100A_450KW:     //phb_230117
		case C_SKI_1500V_400A_100A_500KW:	//jhk_180610
		case C_KTL_1500V_400A_200A_700KW:	//jhk_180902
		case C_KCL_1500V_1000A_500A_600KW:	//jhk_181110
		case C_SKI_2000V_300A_200A_100A_50A_600KW:  //phb_230320
		case C_SKI_AUX_BOX:					//shh_211007
			ch_start = 0;
			ch_end = 8;
			break;
		default:
			//shh_240417s
			ch_start = 0;
			ch_end = 8;
			//ch_start = bd * myData->mData.config.chPerBd;
			//ch_end = ch_start + myData->mData.config.chPerBd;
			//shh_240417e
			break;
	}
	tmp = 0;
	for(ch=ch_start; ch < ch_end; ch++) {
		memcpy((char *)&myData->cali[ch].data, (char *)&data[tmp],
			sizeof(S_CALI_DATA) * MAX_TYPE * MAX_RANGE);
		tmp++;
	}

	return 0;
}

int Read_Cali_Set_Main(void)
{
#if defined __COB__
	char temp[32], buf[32], fileName[256];
    int tmp, range, point;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/caliData/Cali_Set_Main");
	// /root/system_data/config/caliData/Cali_Set_Main

    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Cali_Set_Main file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "mainDA") != 0) {
		fclose(fp);
		return -2;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "DAV_set_point") != 0) {
		fclose(fp);
		return -2;
	}

	for(range=0; range < MAX_RANGE; range++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "%s%d", "RANGE", range+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -3;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "cali_point") != 0) {
			fclose(fp);
			return -3;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->cali_set_data.main_dav.set_point_num[range] = atoi(buf);

		for(point=0; point < MAX_F_CALI_POINT; point++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->cali_set_data.main_dav.set_point[range][point] = atol(buf);
		}
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "DAV_check_point") != 0) {
		fclose(fp);
		return -4;
	}

	for(range=0; range < MAX_RANGE; range++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "%s%d", "RANGE", range+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -5;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "cali_point") != 0) {
			fclose(fp);
			return -5;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->cali_set_data.main_dav.check_point_num[range] = atoi(buf);

		for(point=0; point < MAX_F_CALI_CHECK_POINT; point++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->cali_set_data.main_dav.check_point[range][point] =atol(buf);
		}
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "DAI_set_point") != 0) {
		fclose(fp);
		return -6;
	}

	for(range=0; range < MAX_RANGE; range++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "%s%d", "RANGE", range+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -7;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "cali_point") != 0) {
			fclose(fp);
			return -7;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->cali_set_data.main_dai.set_point_num[range] = atoi(buf);

		for(point=0; point < MAX_F_CALI_POINT; point++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->cali_set_data.main_dai.set_point[range][point] = atol(buf);
		}
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "DAI_check_point") != 0) {
		fclose(fp);
		return -8;
	}

	for(range=0; range < MAX_RANGE; range++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "%s%d", "RANGE", range+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -9;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "cali_point") != 0) {
			fclose(fp);
			return -9;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->cali_set_data.main_dai.check_point_num[range] = atoi(buf);

		for(point=0; point < MAX_F_CALI_CHECK_POINT; point++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->cali_set_data.main_dai.check_point[range][point] =atol(buf);
		}
	}

	fclose(fp);
#endif
	return 0;
}

int Read_Cali_Set_Ch(void)
{
#if defined __COB__
	char temp[32], buf[32], fileName[256];
    int tmp, range, point;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/caliData/Cali_Set_Ch");
	// /root/system_data/config/caliData/Cali_Set_Ch

    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Cali_Set_Ch file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ch_ad_da") != 0) {
		fclose(fp);
		return -2;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "V_set_point") != 0) {
		fclose(fp);
		return -2;
	}

	for(range=0; range < MAX_RANGE; range++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "%s%d", "RANGE", range+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -3;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "cali_point") != 0) {
			fclose(fp);
			return -3;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->cali_set_data.ch_v.set_point_num[range] = atoi(buf);

		for(point=0; point < MAX_F_CALI_POINT; point++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->cali_set_data.ch_v.set_point[range][point] = atol(buf);
		}
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "V_check_point") != 0) {
		fclose(fp);
		return -4;
	}

	for(range=0; range < MAX_RANGE; range++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "%s%d", "RANGE", range+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -5;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "cali_point") != 0) {
			fclose(fp);
			return -5;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->cali_set_data.ch_v.check_point_num[range] = atoi(buf);

		for(point=0; point < MAX_F_CALI_CHECK_POINT; point++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->cali_set_data.ch_v.check_point[range][point] = atol(buf);
		}
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "I_set_point") != 0) {
		fclose(fp);
		return -6;
	}

	for(range=0; range < MAX_RANGE; range++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "%s%d", "RANGE", range+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -7;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "cali_point") != 0) {
			fclose(fp);
			return -7;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->cali_set_data.ch_i.set_point_num[range] = atoi(buf);

		for(point=0; point < MAX_F_CALI_POINT; point++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->cali_set_data.ch_i.set_point[range][point] = atol(buf);
		}
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "I_check_point") != 0) {
		fclose(fp);
		return -8;
	}

	for(range=0; range < MAX_RANGE; range++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "%s%d", "RANGE", range+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -9;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "cali_point") != 0) {
			fclose(fp);
			return -9;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->cali_set_data.ch_i.check_point_num[range] = atoi(buf);

		for(point=0; point < MAX_F_CALI_CHECK_POINT; point++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->cali_set_data.ch_i.check_point[range][point] = atol(buf);
		}
	}

	fclose(fp);
#endif
	return 0;
}

int Read_Main_CaliData(int group, int type)
{
#if defined __COB__
	char fileName[256], buf[32], temp[32];
	int	tmp, range, point;
	FILE *fp;

	memset(temp, 0, sizeof temp);
	if(type == P0) { //voltage
		sprintf(temp, "%s%d", "mainDA_V_CaliData", group+1);
	} else { //current
		sprintf(temp, "%s%d", "mainDA_I_CaliData", group+1);
	}

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/caliData/");
	strcat(fileName, temp);

	if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Can not open %s file(read)\n", temp);
		return -1;
	}

	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	if(strcmp(temp, buf) != 0) {
		fclose(fp);
		return -2;
	}

	for(range=0; range < MAX_RANGE; range++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "cali_range") != 0) {
			fclose(fp);
			return -3;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "%d", range+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -4;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "cali_point") != 0) {
			fclose(fp);
			return -5;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->gData[group].cali_main_dac.cali_pointNum[type][range]
			= atol(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "cali_cmd") != 0) {
			fclose(fp);
			return -6;
		}

		for(point=0; point < MAX_CALI_POINT; point++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->gData[group].cali_main_dac.cali_cmd[type][range][point]
				= atol(buf);
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "org_meter") != 0) {
			fclose(fp);
			return -7;
		}

		for(point=0; point < MAX_CALI_POINT; point++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->gData[group].cali_main_dac.org_meter[type][range][point]
				= atol(buf);
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "cal_meter") != 0) {
			fclose(fp);
			return -8;
		}

		for(point=0; point < MAX_CALI_POINT; point++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->gData[group].cali_main_dac.cal_meter[type][range][point]
				= atol(buf);
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "DA_A") != 0) {
			fclose(fp);
			return -9;
		}

		for(point=0; point < MAX_CALI_POINT-1; point++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->gData[group].cali_main_dac.DA_A[type][range][point]
				= (double)atof(buf);
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "DA_B") != 0) {
			fclose(fp);
			return -10;
		}

		for(point=0; point < MAX_CALI_POINT-1; point++) {
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->gData[group].cali_main_dac.DA_B[type][range][point]
				= (double)atof(buf);
		}
	}

	fclose(fp);
#endif
	return 0;
}

int Write_Main_CaliData(int group, int type)
{
#if defined __COB__
	char fileName[256], temp[32];
	int	range=0, point;
	FILE *fp;

	if(type == 1) {
		range = myData->mData.signal[M_SIG_CALI_RANGE_I] - 1;
	}

	if(myData->gData[group].cali_main_dac.tmp_da_caliFlag[type][range]
		== 0) return -1;
	myData->gData[group].cali_main_dac.tmp_da_caliFlag[type][range] = 0;

	printf("Type:%d, Range:%d\n", type, range);

	memset(temp, 0, sizeof temp);
	if(type == P0) { //voltage
		sprintf(temp, "%s%d", "mainDA_V_CaliData", group+1);
	} else { //current
		sprintf(temp, "%s%d", "mainDA_I_CaliData", group+1);
	}

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/caliData/");
	strcat(fileName, temp);

	if((fp = fopen(fileName, "w+")) == NULL) {
		userlog(DEBUG_LOG, psName, "Can not open %s file(write)\n", temp);
		return -1;
	}

	fprintf(fp, "%s\n", temp);

	for(range=0; range < MAX_RANGE; range++) {
		fprintf(fp, "cali_range %d\n", range+1);
		fprintf(fp, "cali_point %ld\n",
			myData->gData[group].cali_main_dac.tmp_cali_pointNum[type][range]);

		fprintf(fp, "cali_cmd ");
		for(point=0; point < MAX_CALI_POINT; point++) {
			fprintf(fp, "%ld ", myData->gData[group].cali_main_dac
				.tmp_cali_cmd[type][range][point]);
		}
		fprintf(fp, "\n");
		fprintf(fp, "org_meter ");
		for(point=0; point < MAX_CALI_POINT; point++) {
			fprintf(fp, "%ld ", myData->gData[group].cali_main_dac
				.tmp_org_meter[type][range][point]);
		}
		fprintf(fp, "\n");
		fprintf(fp, "cal_meter ");
		for(point=0; point < MAX_CALI_POINT; point++) {
			fprintf(fp, "%ld ", myData->gData[group].cali_main_dac
				.tmp_cal_meter[type][range][point]);
		}
		fprintf(fp, "\n");
		fprintf(fp, "DA_A ");
		for(point=0; point < MAX_CALI_POINT-1; point++) {
			fprintf(fp, "%f ", myData->gData[group].cali_main_dac
				.tmp_DA_A[type][range][point]);
		}
		fprintf(fp, "\n");
		fprintf(fp, "DA_B ");
		for(point=0; point < MAX_CALI_POINT-1; point++) {
			fprintf(fp, "%f ", myData->gData[group].cali_main_dac
				.tmp_DA_B[type][range][point]);
		}
		fprintf(fp, "\n");
		fprintf(fp, "\n");
	}

	fclose(fp);

	userlog(DEBUG_LOG, psName, "mainDA(write) group:%d, type:%d\n",
		group+1, type);
#endif
	return 1;
}

int Read_Ch_CaliData(int bd, int range)
{ //debug_size_cob
#if defined __COB__
	char fileName[256], temp[32], buf[32];
	int	tmp, ch, type, point, discharge_pointNum, charge_pointNum;
	FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/caliData/");
	memset(buf, 0, sizeof buf);
	sprintf(buf, "CALI_BD%d_R%d", bd+1, range+1);
	strcat(fileName, buf);
	// /root/system_data/config/caliData/CALI_BD#_R#

	if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"Can not open Board %d Range %d CaliData file(read)\n",
			bd+1, range+1);
		return -1;
	}

	tmp = fscanf(fp, "%s", temp); 
	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);
		
	for(type=0; type < MAX_TYPE; type++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(type == P0) {
			if(strcmp(temp, "Voltage") != 0) {
				fclose(fp);
				return -2;
			}
		} else {
			if(strcmp(temp, "Current") != 0) {
				fclose(fp);
				return -2;
			}
		}

		for(ch=0; ch < MAX_CH_PER_BD_64; ch++) {
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			memset(buf, 0, sizeof buf);
			sprintf(buf, "Ch%02d", ch+1);
			if(strcmp(temp, buf) != 0) {
				fclose(fp);
				return -3;
			}

			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "pointNum") != 0) {
				fclose(fp);
				return -3;
			}
			
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->f_cali.cData[bd][ch].DA_Cali_P_PointNum[type][range]
				= atoi(buf);
			myData->f_cali.tmp_cData[bd][ch].DA_Cali_P_PointNum[type][range]
				= atoi(buf);

			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "Cmd") != 0) {
				fclose(fp);
				return -3;
			}

			for(point=0; point < MAX_CALI_POINT; point++) {
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->f_cali.cData[bd][ch].DA_Cali_P_Cmd[type][range][point]
					= atof(buf);
				myData->f_cali.tmp_cData[bd][ch]
					.DA_Cali_P_Cmd[type][range][point] = atof(buf);
			}

			discharge_pointNum = 0;
			charge_pointNum = 0;
			for(point=0; point < myData->f_cali.cData[bd][ch]
				.DA_Cali_P_PointNum[type][range]; point++) {
				if(myData->f_cali.cData[bd][ch]
					.DA_Cali_P_Cmd[type][range][point] < 0) {
					discharge_pointNum++;
				} else {
					charge_pointNum++;
				}
			}
			myData->f_cali.cData[bd][ch].DA_Cali_D_PointNum[type][range] 
				= discharge_pointNum;
			myData->f_cali.tmp_cData[bd][ch].DA_Cali_D_PointNum[type][range]
				= discharge_pointNum;
			myData->f_cali.cData[bd][ch].DA_Cali_C_PointNum[type][range] 
				= charge_pointNum;
			myData->f_cali.tmp_cData[bd][ch].DA_Cali_C_PointNum[type][range]
				= charge_pointNum;

			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "AD") != 0) {
				fclose(fp);
				return -3;
			}

			for(point=0; point < MAX_CALI_POINT; point++) {
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->f_cali.cData[bd][ch].ad_value_P[type][range][point]
					= atof(buf);
				myData->f_cali.tmp_cData[bd][ch].ad_value_P[type][range][point]
					= atof(buf);
			}

			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "Meter") != 0) {
				fclose(fp);
				return -3;
			}

			for(point=0; point < MAX_CALI_POINT; point++) {
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->f_cali.cData[bd][ch].meter_value_P[type][range][point]
					= atof(buf);
				myData->f_cali.tmp_cData[bd][ch]
					.meter_value_P[type][range][point] = atof(buf);
			}

			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "AD_A") != 0) {
				fclose(fp);
				return -3;
			}

			for(point=0; point < MAX_CALI_POINT-1; point++) {
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->f_cali.cData[bd][ch].AD_A_P[type][range][point]
					= atof(buf);
				myData->f_cali.tmp_cData[bd][ch].AD_A_P[type][range][point]
					= atof(buf);
			}

			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "AD_B") != 0) {
				fclose(fp);
				return -3;
			}

			for(point=0; point < MAX_CALI_POINT-1; point++) {
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->f_cali.cData[bd][ch].AD_B_P[type][range][point]
					= atof(buf);
				myData->f_cali.tmp_cData[bd][ch].AD_B_P[type][range][point]
					= atof(buf);
			}

			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "AUX_DA_A") != 0) {
				fclose(fp);
				return -3;
			}

			for(point=0; point < MAX_CALI_POINT-1; point++) {
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->f_cali.cData[bd][ch].AUX_DA_A_P[type][range][point]
					= atof(buf);
				myData->f_cali.tmp_cData[bd][ch]
					.AUX_DA_A_P[type][range][point] = atof(buf);
			}

			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "AUX_DA_B") != 0) {
				fclose(fp);
				return -3;
			}

			for(point=0; point < MAX_CALI_POINT-1; point++) {
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->f_cali.cData[bd][ch].AUX_DA_B_P[type][range][point]
					= atof(buf);
				myData->f_cali.tmp_cData[bd][ch]
					.AUX_DA_B_P[type][range][point] = atof(buf);
			}
		}
	}

	fclose(fp);
#endif
	return 0;
}

int Write_Ch_CaliData(int bd, int range)
{ //debug_size_cob
#if defined __COB__
	char fileName[256], buf[32];
	int	ch, point, type;
	FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/caliData/");
	memset(buf, 0, sizeof buf);
	sprintf(buf, "CALI_BD%d_R%d", bd+1, range+1);
	strcat(fileName, buf);
	// /root/system_data/config/caliData/CALI_BD#_R#

	if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"Can not open Board %d Range %d CaliData file(read)\n", bd+1,
			range+1);
		return -1;
	}

	fprintf(fp, "PNE_Calibration_File_L03\n"); 
	fprintf(fp, "C71-01\n");
	fprintf(fp, "2011/07/14-16:26:00\n");
	fprintf(fp, "\n");
		
	for(type=0; type < MAX_TYPE; type++) {
		if(type == 0) {
		  fprintf(fp, "Voltage\n");
		} else {
		  fprintf(fp, "\n");
		  fprintf(fp, "Current\n");
		}

		for(ch=0; ch < MAX_CH_PER_BD_64; ch++) {
			fprintf(fp, "Ch%02d\n", ch+1);

			if(myData->f_cali.tmp_cData[bd][ch].caliFlag[type][range] == P1) {
				myData->f_cali.tmp_cData[bd][ch].caliFlag[type][range] = P0;

				fprintf(fp, "pointNum %d\n", myData->f_cali.tmp_bData[bd]
					.DA_Cali_P_PointNum[type][range]);

				fprintf(fp, "Cmd\n");
				for(point=0; point < MAX_CALI_POINT; point++) {
					fprintf(fp, "%f ", (float)myData->f_cali.tmp_cData[bd][ch]
						.DA_Cali_P_Cmd[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD\n");
				for(point=0; point < MAX_CALI_POINT; point++) {
					fprintf(fp, "%f ", (float)myData->f_cali.tmp_cData[bd][ch]
						.ad_value_P[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "Meter\n");
				for(point=0; point < MAX_CALI_POINT; point++) {
					fprintf(fp, "%f ", (float)myData->f_cali.tmp_cData[bd][ch]
						.meter_value_P[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_A\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					fprintf(fp, "%f ", (float)myData->f_cali.tmp_cData[bd][ch]
						.AD_A_P[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_B\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					fprintf(fp, "%f ", (float)myData->f_cali.tmp_cData[bd][ch]
						.AD_B_P[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AUX_DA_A\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					fprintf(fp, "%f ", (float)(myData->f_cali.tmp_cData[bd][ch]
						.AUX_DA_A_P[type][range][point]));
				}
				fprintf(fp, "\n");

				fprintf(fp, "AUX_DA_B\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					fprintf(fp, "%f ", (float)myData->f_cali.tmp_cData[bd][ch]
						.AUX_DA_B_P[type][range][point]);
				}
				fprintf(fp, "\n");
			} else {
				fprintf(fp, "pointNum %d\n", myData->f_cali.cData[bd][ch]
					.DA_Cali_P_PointNum[type][range]);

				fprintf(fp, "Cmd\n");
				for(point=0; point < MAX_CALI_POINT; point++) {
					fprintf(fp, "%f ", (float)myData->f_cali.cData[bd][ch]
						.DA_Cali_P_Cmd[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD\n");
				for(point=0; point < MAX_CALI_POINT; point++) {
					fprintf(fp, "%f ", (float)myData->f_cali.cData[bd][ch]
						.ad_value_P[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "Meter\n");
				for(point=0; point < MAX_CALI_POINT; point++) {
					fprintf(fp, "%f ", (float)myData->f_cali.cData[bd][ch]
						.meter_value_P[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_A\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					fprintf(fp, "%f ", (float)myData->f_cali.cData[bd][ch]
						.AD_A_P[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_B\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					fprintf(fp, "%f ", (float)myData->f_cali.cData[bd][ch]
						.AD_B_P[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AUX_DA_A\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					fprintf(fp, "%f ", (float)myData->f_cali.cData[bd][ch]
						.AUX_DA_A_P[type][range][point]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AUX_DA_B\n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
					fprintf(fp, "%f ", (float)myData->f_cali.cData[bd][ch]
						.AUX_DA_B_P[type][range][point]);
				}
				fprintf(fp, "\n");
			}
		}
	}
	fclose(fp);

	if(Write_Ch_Check_CaliData(bd, range) < 0) return -2;
	if(Read_Ch_Check_CaliData(bd, range) < 0) return -3;
#endif
	return 0;
}

int Write_Ch_Check_CaliData(int bd, int range)
{ //debug_size_cob
#if defined __COB__
	char fileName[256], buf[32];
	int	ch, point, type;
	FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/caliData/");
	memset(buf, 0, sizeof buf);
	sprintf(buf, "CALI_BD%d_R%d_Check", bd+1, range+1);
	strcat(fileName, buf);
	// /root/system_data/config/caliData/CALI_BD#_R#_Check

	if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"Can not open Board %d Range %d CaliData_Check file(read)\n", bd+1,
			range+1);
		return -1;
	}

	fprintf(fp, "PNE_Calibration_Check_File_L03\n"); 
	fprintf(fp, "C71-01\n");
	fprintf(fp, "2011/07/14-16:26:00\n");
	fprintf(fp, "\n");
		
	for(type=0; type < MAX_TYPE; type++) {
		if(type == 0) {
		  fprintf(fp, "Voltage\n");
		} else {
		  fprintf(fp, "\n");
		  fprintf(fp, "Current\n");
		}

		for(ch=0; ch < MAX_CH_PER_BD_64; ch++) {
			fprintf(fp, "Ch%02d\n", ch+1);

			fprintf(fp, "Cmd\n");
			for(point=0; point < MAX_CALI_POINT; point++) {
				fprintf(fp, "%f ", (float)myData->f_cali.tmp_cData[bd][ch]
					.DA_Cali_N_Cmd[type][range][point]);
			}
			fprintf(fp, "\n");

			fprintf(fp, "AD\n");
			for(point=0; point < MAX_CALI_POINT; point++) {
				fprintf(fp, "%f ", (float)myData->f_cali.tmp_cData[bd][ch]
					.ad_value_N[type][range][point]);
			}
			fprintf(fp, "\n");

			fprintf(fp, "Meter\n");
			for(point=0; point < MAX_CALI_POINT; point++) {
				fprintf(fp, "%f ", (float)myData->f_cali.tmp_cData[bd][ch]
					.meter_value_N[type][range][point]);
			}
			fprintf(fp, "\n");
		}
	}

	fclose(fp);
#endif
	return 0;
}

int Read_Ch_Check_CaliData(int bd, int range)
{ //debug_size_cob
#if defined __COB__
	char fileName[256], temp[32], buf[32];
	int	tmp, ch, type, point;
	FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/caliData/");
	memset(buf, 0, sizeof buf);
	sprintf(buf, "CALI_BD%d_R%d_Check", bd+1, range+1);
	strcat(fileName, buf);
	// /root/system_data/config/caliData/CALI_BD#_R#_Check

	if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"Can not open Board %d Range %d CaliData Check file(read)\n", bd+1,
			range+1);
	}

	tmp = fscanf(fp, "%s", temp); 
	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);
		
	for(type=0; type < MAX_TYPE; type++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(type == P0) {
			if(strcmp(temp, "Voltage") != 0) {
				fclose(fp);
				return -2;
			}
		} else {
			if(strcmp(temp, "Current") != 0) {
				fclose(fp);
				return -2;
			}
		}

		for(ch=0; ch < MAX_CH_PER_BD_64; ch++) {
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			memset(buf, 0, sizeof buf);
			sprintf(buf, "Ch%02d", ch+1);
			if(strcmp(temp, buf) != 0) {
				fclose(fp);
				return -3;
			}

			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "Cmd") != 0) {
				fclose(fp);
				return -3;
			}

			for(point=0; point < MAX_CALI_POINT; point++) {
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->f_cali.tmp_cData[bd][ch]
					.DA_Cali_P_Cmd[type][range][point] = atof(buf);
			}

			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "AD") != 0) {
				fclose(fp);
				return -3;
			}

			for(point=0; point < MAX_CALI_POINT; point++) {
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->f_cali.tmp_cData[bd][ch].ad_value_N[type][range][point]
					= atof(buf);
			}

			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "Meter") != 0) {
				fclose(fp);
				return -3;
			}

			for(point=0; point < MAX_CALI_POINT; point++) {
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->f_cali.tmp_cData[bd][ch]
					.meter_value_N[type][range][point] = atof(buf);
			}
		}
	}

	fclose(fp);
#endif
	return 0;
}

int Read_MainAD_V_Offset(void)
{
#if defined __COB__
	char temp[32], buf[32], fileName[256];
    int tmp, group;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/caliData/mainAD_V_Offset");
	// /root/system_data/config/parameter/mainAD_V_Offset
	
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "mainAD_V_Offset file read error\n");
		return -1;
	}

	for(group=0; group < myData->AppControl.config.totalGroup; group++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "group%d(uV)", group+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -2;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}

		memset(buf, 0, sizeof buf);
   		tmp = fscanf(fp, "%s", buf);
		myData->mData.cali_main_dac.adV_offset[group] = atol(buf);
	}

    fclose(fp);
#endif
	return 0;
}

int Read_CellArray_A(void)
{
	char temp[32], buf[32], fileName[256];
	int tmp, i, monitor_no, hw_no, bd, ch;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/CellArray_A");
	// /root/system_data/config/parameter/CellArray_A
	
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "CellArray_A file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "monitor_no") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "hw_no") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "bd") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ch") != 0) {
		fclose(fp);
		return -2;
	}

	for(i=0; i < myData->mData.config.installedCh; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		monitor_no = atoi(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -3;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		hw_no = atoi(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -3;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		bd = atoi(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -3;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		ch = atoi(buf);

		myData->CellArray1[i].number1 = monitor_no;
		myData->CellArray1[i].number2 = hw_no;
		myData->CellArray1[i].bd = bd;
		myData->CellArray1[i].ch = ch;

		myData->CellArray2[hw_no-1].number1 = monitor_no;
		myData->CellArray2[hw_no-1].number2 = hw_no;
		myData->CellArray2[hw_no-1].bd = bd;
		myData->CellArray2[hw_no-1].ch = ch;
	}

    fclose(fp);
	return 0;
}
//jhkw_201117s
int Read_Daq_Map(void)
{
	char temp[32], buf[32], fileName[256];
	int tmp, i, j, no, start, end, type, monitor_ch, installed_daq;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/DAQ_MAP");
	
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "DAQ_MAP file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "No") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Start") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "End") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Type") != 0) {
		fclose(fp);
		return -2;
	}

	installed_daq = 0;
	monitor_ch = 0;
	for(i=0; i < 20; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		no = atoi(buf);
		if(no != (i+1)) {
			fclose(fp);
			return -3;
		}

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -3;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		start = atoi(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -3;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		end = atoi(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -3;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		type = atoi(buf);

		if((start == 0) || end == 0 || type == 0) {
			break;
		} else {
			for(j = start; j <= end; j++) {
				myData->aux_ch_num[monitor_ch].daq_ch = j-1;
				myData->aux_ch_num[j-1].type = type;
				myData->aux_ch_num[j-1].monitor_ch = monitor_ch;
				monitor_ch ++;
			}
			installed_daq += (end - start + 1);
		}
	}
	if((myData->mData.config.installedAuxV 
		+ myData->mData.config.installedTH) != installed_daq) {
		userlog(DEBUG_LOG, psName, "DAQ_installed warning \n");
		return -4;
	}

    fclose(fp);
	return 0;
}
//jhkw_201117e

int Create_BdCaliData_Org(int bd)
{
    int	ch, type, range, point;
	char temp[4], fileName[256];
    FILE *fp;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/caliData_org/CALI_BD");
	memset(temp, 0, sizeof temp);
	temp[0] = (char)(49+bd);
	strcat(fileName, temp);
	// /root/system_data/config/caliData/CALI_BD#

    if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName,
			"Can not open %d Board CaliData file(write)\n", bd+1);
		return -1;
	}
	
	fprintf(fp, "MCTS_Calibration_File\n");
	fprintf(fp, "KJG001-00\n");
	fprintf(fp, "%s\n\n", "date"); //kjg_w
	
	for(type=0; type < MAX_TYPE; type++) {
		if(type == 0) {
			fprintf(fp, "voltage\n");
		} else {
			fprintf(fp, "current\n");
		}
		for(range=0; range < MAX_RANGE; range++) {
			if((range+1) == RANGE1) {
				fprintf(fp, "range1\n");
			} else if((range+1) == RANGE2) {
				fprintf(fp, "range2\n");
			} else if((range+1) == RANGE3) {
				fprintf(fp, "range3\n");
			} else {
				fprintf(fp, "range4\n");
			}

			for(ch=0; ch < MAX_CH_PER_BD; ch++) {
	    		fprintf(fp, "ch%02d  ", ch+1);

				fprintf(fp, "setPointNum   \n");
			   	fprintf(fp, "%d ", 0);
				fprintf(fp, "\n");

				fprintf(fp, "setPoint      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
				   	fprintf(fp, "%f ", 0.0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "checkPointNum   \n");
			   	fprintf(fp, "%d ", 0);
				fprintf(fp, "\n");

				fprintf(fp, "checkPoint      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
				   	fprintf(fp, "%f ", 0.0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_ad        \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
				   	fprintf(fp, "%d ", 0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "set_meter     \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
				   	fprintf(fp, "%d ", 0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_ad      \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
				   	fprintf(fp, "%d ", 0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "check_meter   \n");
				for(point=0; point < MAX_CALI_POINT; point++) {
				   	fprintf(fp, "%d ", 0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "DA_A          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
				   	fprintf(fp, "%f ", 1.0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "DA_B\n        ");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
				   	fprintf(fp, "%f ", 0.0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_A          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
				   	fprintf(fp, "%f ", 1.0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_B          \n");
				for(point=0; point < MAX_CALI_POINT-1; point++) {
				   	fprintf(fp, "%f ", 0.0);
				}
				fprintf(fp, "\n");

				fprintf(fp, "AD_Ratio      \n");
				fprintf(fp, "%f %f", 1.0, 1.0);
				fprintf(fp, "\n\n");
			}
		}
	}
   	fclose(fp);

	return 0;
}

int	Read_ChAttribute(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp, ch;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/ChAttribute");
	// /root/system_data/config/parameter/ChAttribute
   	if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "ChAttribute file read error\n");
		return -1;
	}

	for(ch=0; ch < MAX_CH_256; ch++) {
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "chNo") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		tmp = atoi(temp);
		if(tmp != (ch+1)) {
			fclose(fp);
			return -2;
		}

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "chNo_master") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].chNo_master = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "chNo_slave1") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].chNo_slave[0] = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "chNo_slave2") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].chNo_slave[1] = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "chNo_slave3") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].chNo_slave[2] = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "opType") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].opType = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "chamber_con") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].chamber_control = (unsigned char)atoi(buf);

		//kjhw_141201s
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "can_comm_check") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].can_comm_check = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "chamber_standby") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].chamber_standby = (unsigned char)atoi(buf);
		//kjhw_141223
		//myData->ChAttribute[ch].reserved1 = (unsigned char)atoi(buf);
		//kjhw_141201e
		/*
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "reserved1") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].reserved1[0] = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "reserved2") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].reserved1[1] = (unsigned char)atoi(buf);
		*/
		//kjhw_151021s
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "out_mux_use") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].out_mux_use = (unsigned char)atoi(buf);

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__ || defined __COA_VER_100F__
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "reserved1") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].reserved1[0] = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "reserved2") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].reserved1[1] = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "reserved3") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].reserved1[2] = (unsigned char)atoi(buf);
		//kjhw_151021e
#else //COA_VER_100F01~ //kjhw_170906	
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "chiller_con") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].chiller_control = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "reserved1") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].reserved1[0] = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "reserved2") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->ChAttribute[ch].reserved1[1] = (unsigned char)atoi(buf);
#endif
	}

   	fclose(fp);
	return 0;
}

int	Write_ChAttribute(void)
{
	char fileName[256];
	int ch;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/ChAttribute");
	// /root/system_data/config/parameter/ChAttribute
   	if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "ChAttribute file write error\n");
		return -1;
	}

	for(ch=0; ch < MAX_CH_256; ch++) {
		fprintf(fp, "chNo        : %d\n", ch+1);
		fprintf(fp, "chNo_master : %d\n", myData->ChAttribute[ch].chNo_master);
		fprintf(fp, "chNo_slave1 : %d\n",
			myData->ChAttribute[ch].chNo_slave[0]);
		fprintf(fp, "chNo_slave2 : %d\n",
			myData->ChAttribute[ch].chNo_slave[1]);
		fprintf(fp, "chNo_slave3 : %d\n",
			myData->ChAttribute[ch].chNo_slave[2]);
		fprintf(fp, "opType      : %d\n", myData->ChAttribute[ch].opType);
		fprintf(fp, "chamber_con : %d\n",
			myData->ChAttribute[ch].chamber_control);

		//kjhw_141201s
		fprintf(fp, "can_comm_check  : %d\n",
			myData->ChAttribute[ch].can_comm_check);
		fprintf(fp, "chamber_standby : %d\n",
			myData->ChAttribute[ch].chamber_standby);
		//fprintf(fp, "reserved1   : %d\n", myData->ChAttribute[ch].reserved1);
		/*
		fprintf(fp, "reserved1   : %d\n", myData->ChAttribute[ch].reserved1[0]);
		fprintf(fp, "reserved2   : %d\n", myData->ChAttribute[ch].reserved1[1]);
		*/
		//kjhw_141201e
		
		//kjhw_151021s
		fprintf(fp, "out_mux_use  : %d\n",
			myData->ChAttribute[ch].out_mux_use);
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__ || defined __COA_VER_100F__
		fprintf(fp, "reserved1	  : %d\n",
			myData->ChAttribute[ch].reserved1[0]);
		fprintf(fp, "reserved2	  : %d\n",
			myData->ChAttribute[ch].reserved1[1]);
		fprintf(fp, "reserved3	  : %d\n",
			myData->ChAttribute[ch].reserved1[2]); //kjhw_151021e
#else //COA_VER_100F01~ //kjhw_170906
		fprintf(fp, "chiller_con  : %d\n",
			myData->ChAttribute[ch].chiller_control);
		fprintf(fp, "reserved1	  : %d\n",
			myData->ChAttribute[ch].reserved1[0]);
		fprintf(fp, "reserved2	  : %d\n",
			myData->ChAttribute[ch].reserved1[1]);
#endif
		
		fprintf(fp, "\n");
	}

   	fclose(fp);
	return 0;
}

int	Write_ChAttribute_1(void)
{ //kjhw_180219
	char fileName[256];
	int ch;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/ChAttribute");
	// /root/system_data/config/parameter/ChAttribute
   	if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "ChAttribute file write error\n");
		return -1;
	}

	for(ch=0; ch < MAX_CH_256; ch++) {
		fprintf(fp, "chNo        : %d\n", ch+1);
		fprintf(fp, "chNo_master : %d\n", ch+1);
		fprintf(fp, "chNo_slave1 : %d\n",
			myData->ChAttribute[ch].chNo_slave[0]);
		fprintf(fp, "chNo_slave2 : %d\n",
			myData->ChAttribute[ch].chNo_slave[1]);
		fprintf(fp, "chNo_slave3 : %d\n",
			myData->ChAttribute[ch].chNo_slave[2]);
		fprintf(fp, "opType      : %d\n", myData->ChAttribute[ch].opType);
		fprintf(fp, "chamber_con : %d\n",
			myData->ChAttribute[ch].chamber_control);

		//kjhw_141201s
		fprintf(fp, "can_comm_check  : %d\n",
			myData->ChAttribute[ch].can_comm_check);
		fprintf(fp, "chamber_standby : %d\n",
			myData->ChAttribute[ch].chamber_standby);
		//fprintf(fp, "reserved1   : %d\n", myData->ChAttribute[ch].reserved1);
		/*
		fprintf(fp, "reserved1   : %d\n", myData->ChAttribute[ch].reserved1[0]);
		fprintf(fp, "reserved2   : %d\n", myData->ChAttribute[ch].reserved1[1]);
		*/
		//kjhw_141201e
		
		//kjhw_151021s
		fprintf(fp, "out_mux_use  : %d\n",
			myData->ChAttribute[ch].out_mux_use);
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__ || defined __COA_VER_100F__
		fprintf(fp, "reserved1	  : %d\n",
			myData->ChAttribute[ch].reserved1[0]);
		fprintf(fp, "reserved2	  : %d\n",
			myData->ChAttribute[ch].reserved1[1]);
		fprintf(fp, "reserved3	  : %d\n",
			myData->ChAttribute[ch].reserved1[2]); //kjhw_151021e
#else //COA_VER_100F01~ //kjhw_170906
		fprintf(fp, "chiller_con  : %d\n",
			myData->ChAttribute[ch].chiller_control);
		fprintf(fp, "reserved1	  : %d\n",
			myData->ChAttribute[ch].reserved1[0]);
		fprintf(fp, "reserved2	  : %d\n",
			myData->ChAttribute[ch].reserved1[1]);
#endif
		
		fprintf(fp, "\n");
	}

   	fclose(fp);
	return 0;
}

int	Read_AuxSetData(void)
{
	char temp[32], buf[MAX_AUX_NAME_SIZE], fileName[256], *char_tmp;
    int tmp, i, ch, count1, count2;
	int count3; //kjh_160610
    FILE *fp;

	//memset((char *)&myData->auxDataCount, 0,
	//	sizeof(int) * 512 * MAX_AUX_TYPE);	//kjg_180914
	memset((char *)&myData->auxDataCount, 0,
		sizeof(int) * MAX_CH_256 * MAX_AUX_TYPE);
	memset((char *)&myData->auxSetData[0], 0,
		sizeof(S_AUX_SET_DATA) * MAX_AUX_DATA);

	if(myData->mData.config.installedTemp == 0
		&& myData->mData.config.installedAuxV == 0
		&& myData->mData.config.installedTH == 0) return 0; //kjh_160610

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/AuxSetData");
	// /root/system_data/config/parameter/AuxSetData
   	if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "AuxSetData file read error\n");
		return -1;
	}

	for(i=0; i < MAX_AUX_DATA; i++) {
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "auxChNo") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].auxChNo = (short int)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "auxType") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].auxType = (short int)atoi(buf);

#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
		//kjh_160610s
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "tableNo") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].tableNo = (unsigned char)atoi(buf);
		//kjh_160610e
#endif
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "chNo") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].chNo = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "func_div2") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].function_div2 = (short int)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "func_div3") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].function_div3 = (short int)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "fix_safety_flag") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].fix_safety_flag = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "name") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
		char_tmp = fgets(buf, MAX_AUX_NAME_SIZE, fp);
		memset((char *)&myData->auxSetData[i].name, 0, MAX_AUX_NAME_SIZE);
		if(buf[0] == 0x24) { //$
		} else {
			tmp = strlen(buf) - 2;
			if(tmp > 0) {
				memcpy((char *)&myData->auxSetData[i].name,
					(char *)&buf[1], tmp);
			}
		}
		
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "fault_upper") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
    	myData->auxSetData[i].fault_upper = atol(buf);
		
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "fault_lower") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
   		myData->auxSetData[i].fault_lower = atol(buf);
		
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "end_upper") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
    	myData->auxSetData[i].end_upper = atol(buf);
		
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "end_lower") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
    	myData->auxSetData[i].end_lower = atol(buf);
	
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "func_div") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
   		myData->auxSetData[i].function_div = (short int)atoi(buf);
		
		memset(temp, 0, sizeof temp);
	    tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "reserved2") != 0) {
			fclose(fp);
			return -2;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -2;
		}
		memset(buf, 0, sizeof buf);
  		tmp = fscanf(fp, "%s", buf);
    	myData->auxSetData[i].reserved2 = (short int)atoi(buf);
	}
   	fclose(fp);

	//for(ch=0; ch < 512; ch++) {	//kjg_180914
	for(ch=0; ch < MAX_CH_256; ch++) {
		count1 = count2 = 0;
		count3 = 0; //kjh_160610
		for(i=0; i < MAX_AUX_DATA; i++) {
			if((ch+1) == myData->auxSetData[i].chNo) {
				if(myData->auxSetData[i].auxType == 0) { //temperature
					count1++;
				} else if(myData->auxSetData[i].auxType == 1) { //voltage
					count2++;
				} else if(myData->auxSetData[i].auxType == 2) { //thermistor
					count3++; //kjh_160610
				}
			}
		}
		myData->auxDataCount[ch][0] = count1;
		myData->auxDataCount[ch][1] = count2;
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
		myData->auxDataCount[ch][2] = count3; //kjh_160610
#endif
	}

	return 0;
}

int	Write_AuxSetData(void)
{
	char fileName[256];
	int i, ch, count1, count2;
	int count3; //kjh_160610
    FILE *fp;

	//memset((char *)&myData->auxDataCount, 0,
	//	sizeof(int) * 512 * MAX_AUX_TYPE);	//kjg_180914
	memset((char *)&myData->auxDataCount, 0,
		sizeof(int) * MAX_CH_256 * MAX_AUX_TYPE);

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/AuxSetData");
	// /root/system_data/config/parameter/AuxSetData
   	if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "AuxSetData file write error\n");
		return -1;
	}

	for(i=0; i < MAX_AUX_DATA; i++) {
		fprintf(fp, "auxChNo     : %d\n", myData->auxSetData[i].auxChNo);
		fprintf(fp, "auxType     : %d\n", myData->auxSetData[i].auxType);
		//kjh_160610s
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
		fprintf(fp, "tableNo     : %d\n", myData->auxSetData[i].tableNo);
#endif
		//kjh_160610e
		fprintf(fp, "chNo        : %d\n", myData->auxSetData[i].chNo);
		fprintf(fp, "func_div2   : %d\n", myData->auxSetData[i].function_div2);
		fprintf(fp, "func_div3   : %d\n", myData->auxSetData[i].function_div3);
		fprintf(fp, "fix_safety_flag   : %d\n", myData->auxSetData[i].fix_safety_flag);
		if(myData->auxSetData[i].name[0] == 0) {
			fprintf(fp, "name        : $\n");
		} else {
			fprintf(fp, "name        : %s\n", myData->auxSetData[i].name);
		}
   		fprintf(fp, "fault_upper : %ld\n", myData->auxSetData[i].fault_upper);
    	fprintf(fp, "fault_lower : %ld\n", myData->auxSetData[i].fault_lower);
   		fprintf(fp, "end_upper   : %ld\n", myData->auxSetData[i].end_upper);
   		fprintf(fp, "end_lower   : %ld\n", myData->auxSetData[i].end_lower);
    	fprintf(fp, "func_div    : %d\n", myData->auxSetData[i].function_div);
   		fprintf(fp, "reserved2   : %d\n", myData->auxSetData[i].reserved2);
		fprintf(fp, "\n");
	}
   	fclose(fp);

	//for(ch=0; ch < 512; ch++) {	//kjg_180914
	for(ch=0; ch < MAX_CH_256; ch++) {
		count1 = count2 = 0;
		count3 = 0; //kjh_160610
		for(i=0; i < MAX_AUX_DATA; i++) {
			if((ch+1) == myData->auxSetData[i].chNo) {
				if(myData->auxSetData[i].auxType == 0) { //temperature
					count1++;
				} else if(myData->auxSetData[i].auxType == 1) { //voltage
					count2++;
				} else if(myData->auxSetData[i].auxType == 2) { //thermistor
					count3++; //kjh_160610
				}
			}
		}
		myData->auxDataCount[ch][0] = count1;
		myData->auxDataCount[ch][1] = count2;
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
		myData->auxDataCount[ch][2] = count3; //kjh_160610
#endif
	}
	return 0;
}

int	Read_CanConfig(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp;
    FILE *fp;

	if(myData->mData.config.installedCAN == 0) return 0;

	memset((char *)&myData->CAN.config, 0, sizeof(S_CAN_CONFIG));

	memset(fileName, 0, sizeof fileName);
	sprintf(fileName,
		"/root/%s/config/parameter/can_comm/CanConfig",
		myData->AppControl.misc.path1);
   	if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "CanConfig file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdSendLog") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.CmdSendLog = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdRcvdLog") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.CmdRcvLog = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdSendLog_Hex") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.CmdSendLog_Hex = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdRcvdLog_Hex") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.CmdRcvLog_Hex = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CommCheckLog") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.CommCheckLog = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "commType") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//0 : CAN2.0B Board
	//1 : CAN_FD_A Board //kjg_180405
	myData->CAN.config.commType = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "rx_file_format") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.rx_file_format = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "tx_file_format") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.tx_file_format = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "reserved1") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.reserved1[0] = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "reserved2") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.reserved1[1] = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "reserved3") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.reserved1[2] = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "reserved4") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.reserved1[3] = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "reserved5") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.reserved2[0] = atol(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "reserved6") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.reserved2[1] = atol(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "reserved7") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.reserved2[2] = atol(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "reserved8") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.reserved2[3] = atol(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "reserved9") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.reserved2[4] = atol(buf);

	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "reserved10") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
   	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->CAN.config.reserved2[5] = atol(buf);

	return 0;
}

int	Read_CanReceiveSetData(void)
{
	char temp[32], buf[MAX_CAN_NAME_SIZE], fileName[256], *char_tmp;
    int tmp, i, j, ch, count, rtn;
    FILE *fp;

	memset((char *)&myData->canReceiveDataCount, 0,
		sizeof(int) * MAX_CH_8 * MAX_CAN_TYPE);
	memset((char *)&myData->canReceiveSetData, 0,
		sizeof(S_CAN_RECEIVE_SET_DATA));

	rtn = 0;
	if(myData->mData.config.installedCAN == 0) return rtn;

	for(ch=0; ch < MAX_CH_8; ch++) {
		memset(fileName, 0, sizeof fileName);
		sprintf(fileName,
			"/root/%s/config/parameter/can_comm/CanReceiveSetData%02d",
			myData->AppControl.misc.path1, ch+1);
	   	if((fp = fopen(fileName, "r")) == NULL) {
			userlog(DEBUG_LOG, psName,
				"CanReceiveSetData%02d file read error\n", ch+1);
			rtn = -1;
			return rtn;
		}

		for(i=0; i < MAX_CAN_TYPE; i++) {
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(i == 0) {
				if(strcmp(temp, "master") != 0) {
					rtn = -2;
					fclose(fp);
					return rtn;
				}
			} else {
				if(strcmp(temp, "slave") != 0) {
					rtn = -3;
					fclose(fp);
					return rtn;
				}
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "can_baudrate") != 0) {
				rtn = -4;
				fclose(fp);
				return rtn;
			}
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				rtn = -5;
				fclose(fp);
				return rtn;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.commonData[ch][i].can_baudrate
				= (char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "extended_id") != 0) {
				rtn = -6;
				fclose(fp);
				return rtn;
			}
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				rtn = -7;
				fclose(fp);
				return rtn;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.commonData[ch][i].extended_id
				= (char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "controller_canID") != 0) {
				rtn = -8;
				fclose(fp);
				return rtn;
			}
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				rtn = -9;
				fclose(fp);
				return rtn;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.commonData[ch][i].controller_canID
				= strtol(buf, (char **)NULL, 16);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "mask1") != 0) {
				rtn = -10;
				fclose(fp);
				return rtn;
			}
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				rtn = -11;
				fclose(fp);
				return rtn;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.commonData[ch][i].mask[0]
				= strtol(buf, (char **)NULL, 16);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "mask2") != 0) {
				rtn = -12;
				fclose(fp);
				return rtn;
			}
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				rtn = -13;
				fclose(fp);
				return rtn;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.commonData[ch][i].mask[1]
				= strtol(buf, (char **)NULL, 16);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "filter1") != 0) {
				rtn = -14;
				fclose(fp);
				return rtn;
			}
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				rtn = -15;
				fclose(fp);
				return rtn;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.commonData[ch][i].filter[0]
				= strtol(buf, (char **)NULL, 16);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "filter2") != 0) {
				rtn = -16;
				fclose(fp);
				return rtn;
			}
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				rtn = -17;
				fclose(fp);
				return rtn;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.commonData[ch][i].filter[1]
				= strtol(buf, (char **)NULL, 16);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "filter3") != 0) {
				rtn = -18;
				fclose(fp);
				return rtn;
			}
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				rtn = -19;
				fclose(fp);
				return rtn;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.commonData[ch][i].filter[2]
				= strtol(buf, (char **)NULL, 16);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "filter4") != 0) {
				rtn = -20;
				fclose(fp);
				return rtn;
			}
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				rtn = -21;
				fclose(fp);
				return rtn;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.commonData[ch][i].filter[3]
				= strtol(buf, (char **)NULL, 16);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "filter5") != 0) {
				rtn = -22;
				fclose(fp);
				return rtn;
			}
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				rtn = -23;
				fclose(fp);
				return rtn;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.commonData[ch][i].filter[4]
				= strtol(buf, (char **)NULL, 16);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "filter6") != 0) {
				rtn = -24;
				fclose(fp);
				return rtn;
			}
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				rtn = -25;
				fclose(fp);
				return rtn;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.commonData[ch][i].filter[5]
				= strtol(buf, (char **)NULL, 16);

			if(myData->CAN.config.rx_file_format == 0) {
			} else if(myData->CAN.config.rx_file_format == 1) {
				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "bms_type") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canReceiveSetData.commonData[ch][i].bms_type
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "sjw") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canReceiveSetData.commonData[ch][i].sjw
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved1") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				//kjg_180405 myData->canReceiveSetData.commonData[ch][i].reserved1[0]
				//	= atol(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved2") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				//kjg_180405 myData->canReceiveSetData.commonData[ch][i].reserved1[1]
				//	= atol(buf);
			} else if(myData->CAN.config.rx_file_format == 2) {
				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "bms_type") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canReceiveSetData.commonData[ch][i].bms_type
					= (unsigned char)atoi(buf);

				//kjg_140916
				if(i == 0) {
					if(atoi(buf) == 3) { //E_CAN_2
						myData->cData[ch].signal[C_SIG_EXT_OP_TYPE] = 1;
					} else { //Normal
						myData->cData[ch].signal[C_SIG_EXT_OP_TYPE] = 0;
					}
				}

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "sjw") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canReceiveSetData.commonData[ch][i].sjw
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved1") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				//kjg_180405 myData->canReceiveSetData.commonData[ch][i].reserved1[0]
				//	= atol(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved2") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				//kjg_180405 myData->canReceiveSetData.commonData[ch][i].reserved1[1]
				//	= atol(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved3") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				//kjg_180405 myData->canReceiveSetData.commonData[ch][i].reserved1[2]
				//	= atol(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "func_div") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canReceiveSetData.commonData[ch][i].func_div[j]
						= (short int)atoi(buf);
				}

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "compare_type") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canReceiveSetData.commonData[ch][i].compare_type[j]
						= (unsigned char)atoi(buf);
				}

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_type") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canReceiveSetData.commonData[ch][i].data_type[j]
						= (unsigned char)atoi(buf);
				}

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "value") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canReceiveSetData.commonData[ch][i].value[j]
						= atof(buf);
				}
			} else if(myData->CAN.config.rx_file_format == 3) { //kjg_180405
				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "bms_type") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canReceiveSetData.commonData[ch][i].bms_type
					= (unsigned char)atoi(buf);

				//kjg_140916
				if(i == 0) {
					if(atoi(buf) == 3) { //E_CAN_2
						myData->cData[ch].signal[C_SIG_EXT_OP_TYPE] = 1;
					} else { //Normal
						myData->cData[ch].signal[C_SIG_EXT_OP_TYPE] = 0;
					}
				}

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "sjw") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canReceiveSetData.commonData[ch][i].sjw
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "can_fd_flag") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canReceiveSetData.commonData[ch][i].can_fd_flag
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "can_datarate") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canReceiveSetData.commonData[ch][i].can_datarate
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "terminal_r") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canReceiveSetData.commonData[ch][i].terminal_r
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "crc_type") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canReceiveSetData.commonData[ch][i].crc_type
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "func_div") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canReceiveSetData.commonData[ch][i].func_div[j]
						= (short int)atoi(buf);
				}

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "compare_type") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canReceiveSetData.commonData[ch][i].compare_type[j]
						= (unsigned char)atoi(buf);
				}

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_type") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canReceiveSetData.commonData[ch][i].data_type[j]
						= (unsigned char)atoi(buf);
				}

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "value") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canReceiveSetData.commonData[ch][i].value[j]
						= atof(buf);
				}
			}
		}

		for(i=0; i < MAX_CAN_DATA; i++) {
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "index") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
	    	tmp = fscanf(fp, "%s", buf);
			if(atoi(buf) != i) {
				userlog(DEBUG_LOG, psName,
					"CanReceiveSetData%02d file read error2 %d\n", ch+1, i);
				fclose(fp);
				return -5;
			}

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "canType") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.normalData[ch][i].canType
				= (unsigned char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "byte_order") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.normalData[ch][i].byte_order
				= (unsigned char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "data_type") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.normalData[ch][i].data_type
				= (unsigned char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "factor") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.normalData[ch][i].factor = atof(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "offset") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.normalData[ch][i].offset = atof(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "def_value") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.normalData[ch][i].default_value
				= atof(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "startBit") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.normalData[ch][i].startBit
				= (short int)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "bitCount") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.normalData[ch][i].bitCount
				= (short int)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "canID") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.normalData[ch][i].canID = atol(buf);
				//= strtol(buf, (char **)NULL, 16);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "name") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			char_tmp = fgets(buf, MAX_CAN_NAME_SIZE, fp);
			memset((char *)&myData->canReceiveSetData.normalData[ch][i].name,
				0, MAX_CAN_NAME_SIZE);
			if(buf[0] == 0x24) { //$
			} else {
				tmp = strlen(buf) - 2;
				if(tmp > 0) {
					memcpy((char *)&myData->canReceiveSetData
						.normalData[ch][i].name, (char *)&buf[1], tmp);
				}
			}
		
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "fault_upper") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
  			tmp = fscanf(fp, "%s", buf);
	    	myData->canReceiveSetData.normalData[ch][i].fault_upper = atof(buf);
		
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "fault_lower") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
  			tmp = fscanf(fp, "%s", buf);
	   		myData->canReceiveSetData.normalData[ch][i].fault_lower = atof(buf);
		
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "end_upper") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
  			tmp = fscanf(fp, "%s", buf);
	    	myData->canReceiveSetData.normalData[ch][i].end_upper = atof(buf);
		
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "end_lower") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
  			tmp = fscanf(fp, "%s", buf);
	    	myData->canReceiveSetData.normalData[ch][i].end_lower = atof(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "sent_period") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.normalData[ch][i].sent_period = atol(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "func_div") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canReceiveSetData.normalData[ch][i].function_div
				= (short int)atoi(buf);

			if(myData->CAN.config.rx_file_format == 0) {
				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "user_cont") != 0) {
					fclose(fp);
					return -4;
				}
				memset(temp, 0, sizeof temp);
	    		tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -4;
				}
				memset(buf, 0, sizeof buf);
  				tmp = fscanf(fp, "%s", buf);
	    		myData->canReceiveSetData.normalData[ch][i].function_div2
					= (short int)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "tmp_usr_con") != 0) {
					fclose(fp);
					return -4;
				}
				memset(temp, 0, sizeof temp);
	    		tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -4;
				}
				memset(buf, 0, sizeof buf);
	  			tmp = fscanf(fp, "%s", buf);
		    	myData->canReceiveSetData.normalData[ch][i].function_div3
					= (short int)atoi(buf);
			} else if(myData->CAN.config.rx_file_format == 1
				|| myData->CAN.config.rx_file_format == 2
				|| myData->CAN.config.rx_file_format == 3) { //kjg_180405
				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "func_div2") != 0) {
					fclose(fp);
					return -4;
				}
				memset(temp, 0, sizeof temp);
	    		tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -4;
				}
				memset(buf, 0, sizeof buf);
  				tmp = fscanf(fp, "%s", buf);
	    		myData->canReceiveSetData.normalData[ch][i].function_div2
					= (short int)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "func_div3") != 0) {
					fclose(fp);
					return -4;
				}
				memset(temp, 0, sizeof temp);
	    		tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -4;
				}
				memset(buf, 0, sizeof buf);
	  			tmp = fscanf(fp, "%s", buf);
		    	myData->canReceiveSetData.normalData[ch][i].function_div3
					= (short int)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved1") != 0) {
					fclose(fp);
					return -4;
				}
				memset(temp, 0, sizeof temp);
	    		tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -4;
				}
				memset(buf, 0, sizeof buf);
  				tmp = fscanf(fp, "%s", buf);
		    	myData->canReceiveSetData.normalData[ch][i].reserved1
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved2") != 0) {
					fclose(fp);
					return -4;
				}
				memset(temp, 0, sizeof temp);
	    		tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -4;
				}
				memset(buf, 0, sizeof buf);
  				tmp = fscanf(fp, "%s", buf);
		    	myData->canReceiveSetData.normalData[ch][i].reserved2
					= (short int)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved3") != 0) {
					fclose(fp);
					return -4;
				}
				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -4;
				}
				memset(buf, 0, sizeof buf);
  				tmp = fscanf(fp, "%s", buf);
	    		myData->canReceiveSetData.normalData[ch][i].reserved3[0]
					= atol(buf);
			}
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__
#else //COA_VER_100C~
			//kjhw_131204s
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "startBit2") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
  			tmp = fscanf(fp, "%s", buf);
	   		myData->canReceiveSetData.normalData[ch][i].startBit2
				= (short int)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "bitCount2") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
  			tmp = fscanf(fp, "%s", buf);
	   		myData->canReceiveSetData.normalData[ch][i].bitCount2
				= (short int)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "byte_order2") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
  			tmp = fscanf(fp, "%s", buf);
	   		myData->canReceiveSetData.normalData[ch][i].byte_order2
				= (unsigned char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "data_type2") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
  			tmp = fscanf(fp, "%s", buf);
	   		myData->canReceiveSetData.normalData[ch][i].data_type2
				= (unsigned char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "reserved4") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
  			tmp = fscanf(fp, "%s", buf);
	   		myData->canReceiveSetData.normalData[ch][i].reserved4[0]
				= (unsigned char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "reserved5") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
  			tmp = fscanf(fp, "%s", buf);
	   		myData->canReceiveSetData.normalData[ch][i].reserved4[1]
				= (unsigned char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "compare_value") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
  			tmp = fscanf(fp, "%s", buf);
	   		myData->canReceiveSetData.normalData[ch][i].compare_value
				= atof(buf);
			//kjhw_131204e
#endif
		}
	   	fclose(fp);

		count = 0;
		for(i=0; i < MAX_CAN_DATA; i++) { //master
			if(myData->canReceiveSetData.normalData[ch][i].canType == 1)
				count++;
		}
		myData->canReceiveDataCount[ch][0] = count;

		count = 0;
		for(i=0; i < MAX_CAN_DATA; i++) { //slave
			if(myData->canReceiveSetData.normalData[ch][i].canType == 2)
				count++;
		}
		myData->canReceiveDataCount[ch][1] = count;

		userlog(DEBUG_LOG, psName, "CanReceiveSetData%02d file read end\n",
			ch+1);
	}

	return 0;
}

int	Write_CanReceiveSetData(void)
{
	char fileName[256];
	int i, ch, count;
    FILE *fp;

	//memset((char *)&myData->canReceiveDataCount, 0,
	//	sizeof(int) * MAX_CH_8 * MAX_CAN_TYPE);
	//csk_190331r
	
	for(ch=0; ch < MAX_CH_8; ch++) {
		memset(fileName, 0, sizeof fileName);
		sprintf(fileName,
			"/root/%s/config/parameter/can_comm/CanReceiveSetData%02d",
			myData->AppControl.misc.path1, ch+1);
	   	if((fp = fopen(fileName, "w")) == NULL) {
			userlog(DEBUG_LOG, psName,
				"CanReceiveSetData%02d file write error\n", ch+1);
			return -1;
		}

		for(i=0; i < MAX_CAN_TYPE; i++) {
			if(i == 0) {
				fprintf(fp, "master\n");
				fprintf(fp, "can_baudrate : %d\n",
					myData->canReceiveSetData.commonData[ch][i].can_baudrate);
			} else {
				fprintf(fp, "\nslave\n");
				fprintf(fp, "can_baudrate : %d\n",
					myData->canReceiveSetData.commonData[ch][i].can_baudrate);
			}
			fprintf(fp, "extended_id : %d\n",
				myData->canReceiveSetData.commonData[ch][i].extended_id);
			fprintf(fp, "controller_canID : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].controller_canID);
			fprintf(fp, "mask1   : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].mask[0]);
			fprintf(fp, "mask2   : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].mask[1]);
			fprintf(fp, "filter1 : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].filter[0]);
				//myData->canReceiveSetData.commonData[ch][i].mask[0]);
			fprintf(fp, "filter2 : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].filter[1]);
				//myData->canReceiveSetData.commonData[ch][i].mask[1]);
			fprintf(fp, "filter3 : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].filter[2]);
				//myData->canReceiveSetData.commonData[ch][i].mask[2]);
			fprintf(fp, "filter4 : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].filter[3]);
				//myData->canReceiveSetData.commonData[ch][i].mask[3]);
			fprintf(fp, "filter5 : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].filter[4]);
				//myData->canReceiveSetData.commonData[ch][i].mask[4]);
			fprintf(fp, "filter6 : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].filter[5]);
				//myData->canReceiveSetData.commonData[ch][i].mask[5]);

			if(myData->CAN.config.rx_file_format == 0) {
			} else if(myData->CAN.config.rx_file_format == 1) {
				fprintf(fp, "bms_type : %d\n",
					myData->canReceiveSetData.commonData[ch][i].bms_type);
				fprintf(fp, "sjw      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].sjw);
				fprintf(fp, "reserved1 : 0\n");
					//kjg_180405 myData->canReceiveSetData.commonData[ch][i].reserved1[0]);
				fprintf(fp, "reserved2 : 0\n");
					//kjg_180405 myData->canReceiveSetData.commonData[ch][i].reserved1[1]);
			} else if(myData->CAN.config.rx_file_format == 2) {
				fprintf(fp, "bms_type : %d\n",
					myData->canReceiveSetData.commonData[ch][i].bms_type);

				//kjg_140916
				if(i == 0) {
					if(myData->canReceiveSetData.commonData[ch][i].bms_type
						== 3) { //E_CAN_2
						myData->cData[ch].signal[C_SIG_EXT_OP_TYPE] = 1;
					} else { //Normal
						myData->cData[ch].signal[C_SIG_EXT_OP_TYPE] = 0;
					}
				}

				fprintf(fp, "sjw      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].sjw);
				fprintf(fp, "reserved1 : 0\n");
					//kjg_180405 myData->canReceiveSetData.commonData[ch][i].reserved1[0]);
				fprintf(fp, "reserved2 : 0\n");
					//kjg_180405 myData->canReceiveSetData.commonData[ch][i].reserved1[1]);
				fprintf(fp, "reserved3 : 0\n");
					//kjg_180405 myData->canReceiveSetData.commonData[ch][i].reserved1[2]);

				fprintf(fp,
					"func_div	:	%d %d %d %d %d %d %d %d %d %d\n",
					myData->canReceiveSetData.commonData[ch][i].func_div[0],
					myData->canReceiveSetData.commonData[ch][i].func_div[1],
					myData->canReceiveSetData.commonData[ch][i].func_div[2],
					myData->canReceiveSetData.commonData[ch][i].func_div[3],
					myData->canReceiveSetData.commonData[ch][i].func_div[4],
					myData->canReceiveSetData.commonData[ch][i].func_div[5],
					myData->canReceiveSetData.commonData[ch][i].func_div[6],
					myData->canReceiveSetData.commonData[ch][i].func_div[7],
					myData->canReceiveSetData.commonData[ch][i].func_div[8],
					myData->canReceiveSetData.commonData[ch][i].func_div[9]);

				fprintf(fp,
					"compare_type	:	%d %d %d %d %d %d %d %d %d %d\n",
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[0],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[1],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[2],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[3],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[4],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[5],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[6],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[7],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[8],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[9]);

				fprintf(fp,
					"data_type	:	%d %d %d %d %d %d %d %d %d %d\n",
					myData->canReceiveSetData.commonData[ch][i].data_type[0],
					myData->canReceiveSetData.commonData[ch][i].data_type[1],
					myData->canReceiveSetData.commonData[ch][i].data_type[2],
					myData->canReceiveSetData.commonData[ch][i].data_type[3],
					myData->canReceiveSetData.commonData[ch][i].data_type[4],
					myData->canReceiveSetData.commonData[ch][i].data_type[5],
					myData->canReceiveSetData.commonData[ch][i].data_type[6],
					myData->canReceiveSetData.commonData[ch][i].data_type[7],
					myData->canReceiveSetData.commonData[ch][i].data_type[8],
					myData->canReceiveSetData.commonData[ch][i].data_type[9]);

				fprintf(fp,
					"value	:	%f %f %f %f %f %f %f %f %f %f\n",
					myData->canReceiveSetData.commonData[ch][i].value[0],
					myData->canReceiveSetData.commonData[ch][i].value[1],
					myData->canReceiveSetData.commonData[ch][i].value[2],
					myData->canReceiveSetData.commonData[ch][i].value[3],
					myData->canReceiveSetData.commonData[ch][i].value[4],
					myData->canReceiveSetData.commonData[ch][i].value[5],
					myData->canReceiveSetData.commonData[ch][i].value[6],
					myData->canReceiveSetData.commonData[ch][i].value[7],
					myData->canReceiveSetData.commonData[ch][i].value[8],
					myData->canReceiveSetData.commonData[ch][i].value[9]);
			} else if(myData->CAN.config.rx_file_format == 3) {
				fprintf(fp, "bms_type : %d\n",
					myData->canReceiveSetData.commonData[ch][i].bms_type);

				//kjg_140916
				if(i == 0) {
					if(myData->canReceiveSetData.commonData[ch][i].bms_type
						== 3) { //E_CAN_2
						myData->cData[ch].signal[C_SIG_EXT_OP_TYPE] = 1;
					} else { //Normal
						myData->cData[ch].signal[C_SIG_EXT_OP_TYPE] = 0;
					}
				}

				fprintf(fp, "sjw      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].sjw);
				fprintf(fp, "can_fd_flag : %d\n",
					myData->canReceiveSetData.commonData[ch][i].can_fd_flag);
				fprintf(fp, "can_datarate : %d\n",
					myData->canReceiveSetData.commonData[ch][i].can_datarate);
				fprintf(fp, "terminal_r : %d\n",
					myData->canReceiveSetData.commonData[ch][i].terminal_r);
				fprintf(fp, "crc_type : %d\n",
					myData->canReceiveSetData.commonData[ch][i].crc_type);

				fprintf(fp,
					"func_div	:	%d %d %d %d %d %d %d %d %d %d\n",
					myData->canReceiveSetData.commonData[ch][i].func_div[0],
					myData->canReceiveSetData.commonData[ch][i].func_div[1],
					myData->canReceiveSetData.commonData[ch][i].func_div[2],
					myData->canReceiveSetData.commonData[ch][i].func_div[3],
					myData->canReceiveSetData.commonData[ch][i].func_div[4],
					myData->canReceiveSetData.commonData[ch][i].func_div[5],
					myData->canReceiveSetData.commonData[ch][i].func_div[6],
					myData->canReceiveSetData.commonData[ch][i].func_div[7],
					myData->canReceiveSetData.commonData[ch][i].func_div[8],
					myData->canReceiveSetData.commonData[ch][i].func_div[9]);

				fprintf(fp,
					"compare_type	:	%d %d %d %d %d %d %d %d %d %d\n",
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[0],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[1],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[2],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[3],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[4],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[5],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[6],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[7],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[8],
					myData->canReceiveSetData.commonData[ch][i]
						.compare_type[9]);

				fprintf(fp,
					"data_type	:	%d %d %d %d %d %d %d %d %d %d\n",
					myData->canReceiveSetData.commonData[ch][i].data_type[0],
					myData->canReceiveSetData.commonData[ch][i].data_type[1],
					myData->canReceiveSetData.commonData[ch][i].data_type[2],
					myData->canReceiveSetData.commonData[ch][i].data_type[3],
					myData->canReceiveSetData.commonData[ch][i].data_type[4],
					myData->canReceiveSetData.commonData[ch][i].data_type[5],
					myData->canReceiveSetData.commonData[ch][i].data_type[6],
					myData->canReceiveSetData.commonData[ch][i].data_type[7],
					myData->canReceiveSetData.commonData[ch][i].data_type[8],
					myData->canReceiveSetData.commonData[ch][i].data_type[9]);

				fprintf(fp,
					"value	:	%f %f %f %f %f %f %f %f %f %f\n",
					myData->canReceiveSetData.commonData[ch][i].value[0],
					myData->canReceiveSetData.commonData[ch][i].value[1],
					myData->canReceiveSetData.commonData[ch][i].value[2],
					myData->canReceiveSetData.commonData[ch][i].value[3],
					myData->canReceiveSetData.commonData[ch][i].value[4],
					myData->canReceiveSetData.commonData[ch][i].value[5],
					myData->canReceiveSetData.commonData[ch][i].value[6],
					myData->canReceiveSetData.commonData[ch][i].value[7],
					myData->canReceiveSetData.commonData[ch][i].value[8],
					myData->canReceiveSetData.commonData[ch][i].value[9]);
			}
		}

		for(i=0; i < MAX_CAN_DATA; i++) {
			fprintf(fp, "\nindex : %03d\n", i);
			fprintf(fp, "canType     : %d\n",
				myData->canReceiveSetData.normalData[ch][i].canType);
			fprintf(fp, "byte_order  : %d\n",
				myData->canReceiveSetData.normalData[ch][i].byte_order);
			fprintf(fp, "data_type   : %d\n",
				myData->canReceiveSetData.normalData[ch][i].data_type);
			fprintf(fp, "factor      : %f\n",
				myData->canReceiveSetData.normalData[ch][i].factor);
			fprintf(fp, "offset      : %f\n",
				myData->canReceiveSetData.normalData[ch][i].offset);
			fprintf(fp, "def_value   : %f\n",
				myData->canReceiveSetData.normalData[ch][i].default_value);
			fprintf(fp, "startBit    : %d\n",
				myData->canReceiveSetData.normalData[ch][i].startBit);
			fprintf(fp, "bitCount    : %d\n",
				myData->canReceiveSetData.normalData[ch][i].bitCount);
			fprintf(fp, "canID       : %ld\n",
				myData->canReceiveSetData.normalData[ch][i].canID);

			if(myData->canReceiveSetData.normalData[ch][i].name[0] == 0) {
				fprintf(fp, "name        : $\n");
			} else {
				fprintf(fp, "name        : %s\n",
					myData->canReceiveSetData.normalData[ch][i].name);
			}
   			fprintf(fp, "fault_upper : %f\n",
				myData->canReceiveSetData.normalData[ch][i].fault_upper);
			fprintf(fp, "fault_lower : %f\n",
    			myData->canReceiveSetData.normalData[ch][i].fault_lower);
	   		fprintf(fp, "end_upper   : %f\n",
				myData->canReceiveSetData.normalData[ch][i].end_upper);
			fprintf(fp, "end_lower   : %f\n",
   				myData->canReceiveSetData.normalData[ch][i].end_lower);
			fprintf(fp, "sent_period : %ld\n",
				myData->canReceiveSetData.normalData[ch][i].sent_period);
			fprintf(fp, "func_div    : %d\n",
				myData->canReceiveSetData.normalData[ch][i].function_div);
			if(myData->CAN.config.rx_file_format == 0) {
				fprintf(fp, "user_cont		: %d\n",
					myData->canReceiveSetData.normalData[ch][i].function_div2);
				fprintf(fp, "tmp_usr_con	: %d\n",
					myData->canReceiveSetData.normalData[ch][i].function_div3);
			} else {
				fprintf(fp, "func_div2	: %d\n",
					(short int)myData->canReceiveSetData
					.normalData[ch][i].function_div2);
				fprintf(fp, "func_div3	: %d\n",
					(short int)myData->canReceiveSetData
					.normalData[ch][i].function_div3);
				fprintf(fp, "reserved1	: %d\n",
					(unsigned char)myData->canReceiveSetData
					.normalData[ch][i].reserved1);
				fprintf(fp, "reserved2	: %d\n",
					(short int)myData->canReceiveSetData
					.normalData[ch][i].reserved2);
				fprintf(fp, "reserved3	: %ld\n",
					myData->canReceiveSetData.normalData[ch][i].reserved3[0]);
			}
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__
#else //COA_VER_100C~
			//kjhw_131204s
			fprintf(fp, "startBit2   : %d\n",
				myData->canReceiveSetData.normalData[ch][i].startBit2);
			fprintf(fp, "bitCount2   : %d\n",
				myData->canReceiveSetData.normalData[ch][i].bitCount2);
			fprintf(fp, "byte_order2 : %d\n",
				myData->canReceiveSetData.normalData[ch][i].byte_order2);
			fprintf(fp, "data_type2  : %d\n",
				myData->canReceiveSetData.normalData[ch][i].data_type2);
			fprintf(fp, "reserved4   : %d\n",
				myData->canReceiveSetData.normalData[ch][i].reserved4[0]);
			fprintf(fp, "reserved5   : %d\n",
				myData->canReceiveSetData.normalData[ch][i].reserved4[1]);
			fprintf(fp, "compare_value : %f\n",
				myData->canReceiveSetData.normalData[ch][i].compare_value);
			//kjhw_131204e
#endif
		}
   		fclose(fp);

		count = 0;
		for(i=0; i < MAX_CAN_DATA; i++) { //master
			if(myData->canReceiveSetData.normalData[ch][i].canType == 1)
				count++;
		}
		myData->canReceiveDataCount[ch][0] = count;

		count = 0;
		for(i=0; i < MAX_CAN_DATA; i++) { //slave
			if(myData->canReceiveSetData.normalData[ch][i].canType == 2)
				count++;
		}
		myData->canReceiveDataCount[ch][1] = count;
	}

	return 0;
}

int	Read_CanTransmitSetData(void)
{
	char temp[32], buf[MAX_CAN_NAME_SIZE], fileName[256], *char_tmp;
    int tmp, i, j, ch, count;
    FILE *fp;

	memset((char *)&myData->canTransmitDataCount, 0,
		sizeof(int) * MAX_CH_8 * MAX_CAN_TYPE);
	memset((char *)&myData->canTransmitSetData, 0,
		sizeof(S_CAN_TRANSMIT_SET_DATA));

	if(myData->mData.config.installedCAN == 0) return 0;

	for(ch=0; ch < MAX_CH_8; ch++) {
		memset(fileName, 0, sizeof fileName);
		sprintf(fileName,
			"/root/%s/config/parameter/can_comm/CanTransmitSetData%02d",
			myData->AppControl.misc.path1, ch+1);
	   	if((fp = fopen(fileName, "r")) == NULL) {
			userlog(DEBUG_LOG, psName,
				"CanTransmitSetData%02d file read error\n", ch+1);
			return -1;
		}

		for(i=0; i < MAX_CAN_TYPE; i++) {
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(i == 0) {
				if(strcmp(temp, "master") != 0) {
					fclose(fp);
					return -2;
				}
			} else {
				if(strcmp(temp, "slave") != 0) {
					fclose(fp);
					return -2;
				}
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "can_baudrate") != 0) {
				fclose(fp);
				return -2;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -2;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.commonData[ch][i].can_baudrate
				= (char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "extended_id") != 0) {
				fclose(fp);
				return -2;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -2;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.commonData[ch][i].extended_id
				= (char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "controller_canID") != 0) {
				fclose(fp);
				return -2;
			}
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -2;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.commonData[ch][i].controller_canID
				= strtol(buf, (char **)NULL, 16);

			if(myData->CAN.config.tx_file_format == 0) {
			} else if(myData->CAN.config.tx_file_format == 1) {
				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "bms_type") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.commonData[ch][i].bms_type
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "sjw") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.commonData[ch][i].sjw
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved1") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				//kjg_180405 myData->canTransmitSetData.commonData[ch][i].reserved1[0]
				//	= atol(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved2") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				//kjg_180405 myData->canTransmitSetData.commonData[ch][i].reserved1[1]
				//	= atol(buf);
			} else if(myData->CAN.config.tx_file_format == 2) {
				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "bms_type") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.commonData[ch][i].bms_type
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "sjw") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.commonData[ch][i].sjw
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved1") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				//kjg_180405 myData->canTransmitSetData.commonData[ch][i].reserved1[0]
				//	= atol(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved2") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				//kjg_180405 myData->canTransmitSetData.commonData[ch][i].reserved1[1]
				//	= atol(buf);

				memset(temp, 0, sizeof temp);
			   	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "func_div") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_P1_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canTransmitSetData.commonData[ch][i].func_div[j]
						= (short int)atoi(buf);
				}

				memset(temp, 0, sizeof temp);
			   	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "compare_type") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canTransmitSetData.commonData[ch][i].compare_type[j]
						= (unsigned char)atoi(buf);
				}

				memset(temp, 0, sizeof temp);
			   	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_type") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canTransmitSetData.commonData[ch][i].data_type[j]
						= (unsigned char)atoi(buf);
				}

				memset(temp, 0, sizeof temp);
			   	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "value") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canTransmitSetData.commonData[ch][i].value[j]
						= atof(buf);
				}
			} else if(myData->CAN.config.tx_file_format == 3) { //kjg_180405
				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "bms_type") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.commonData[ch][i].bms_type
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "sjw") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.commonData[ch][i].sjw
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "can_fd_flag") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.commonData[ch][i].can_fd_flag
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "can_datarate") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.commonData[ch][i].can_datarate
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "terminal_r") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.commonData[ch][i].terminal_r
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "crc_type") != 0) {
					fclose(fp);
					return -2;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -2;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.commonData[ch][i].crc_type
					= (unsigned char)atoi(buf);

				memset(temp, 0, sizeof temp);
			   	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "func_div") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_P1_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canTransmitSetData.commonData[ch][i].func_div[j]
						= (short int)atoi(buf);
				}

				memset(temp, 0, sizeof temp);
			   	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "compare_type") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canTransmitSetData.commonData[ch][i].compare_type[j]
						= (unsigned char)atoi(buf);
				}

				memset(temp, 0, sizeof temp);
			   	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_type") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canTransmitSetData.commonData[ch][i].data_type[j]
						= (unsigned char)atoi(buf);
				}

				memset(temp, 0, sizeof temp);
			   	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "value") != 0) {
					fclose(fp);
					return -3;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -3;
				}
				for(j=0; j < MAX_CAN_FUNCTION; j++) {
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canTransmitSetData.commonData[ch][i].value[j]
						= atof(buf);
				}
			}
		}

		for(i=0; i < MAX_CAN_DATA; i++) {
			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "index") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	  	  	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
	  	  	tmp = fscanf(fp, "%s", buf);
			if(atoi(buf) != i) {
				userlog(DEBUG_LOG, psName,
					"CanTransmitSetData%02d file read error2 %d\n", ch+1, i);
				fclose(fp);
				return -3;
			}

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "canType") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	  	  	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.normalData[ch][i].canType
				= (unsigned char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "byte_order") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	  	  	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.normalData[ch][i].byte_order
				= (unsigned char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "data_type") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	  	  	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.normalData[ch][i].data_type
				= (unsigned char)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "factor") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	  	  	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.normalData[ch][i].factor = atof(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "offset") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	  	  	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.normalData[ch][i].offset = atof(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "def_value") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	  	  	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.normalData[ch][i].default_value
				= atof(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "startBit") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	  	  	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.normalData[ch][i].startBit
				= (short int)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "bitCount") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	  	  	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.normalData[ch][i].bitCount
				= (short int)atoi(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "canID") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	  	  	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.normalData[ch][i].canID = atol(buf);
				//= strtol(buf, (char **)NULL, 16);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "send_period") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	  	  	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.normalData[ch][i].send_period
				= atol(buf);

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "name") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	  	  	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			char_tmp = fgets(buf, MAX_CAN_NAME_SIZE, fp);
			memset((char *)&myData->canTransmitSetData.normalData[ch][i].name,
				0, MAX_CAN_NAME_SIZE);
			if(buf[0] == 0x24) { //$
			} else {
				tmp = strlen(buf) - 2;
				if(tmp > 0) {
					memcpy((char *)&myData->canTransmitSetData
						.normalData[ch][i].name, (char *)&buf[1], tmp);
				}
			}

			memset(temp, 0, sizeof temp);
	    	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, "func_div") != 0) {
				fclose(fp);
				return -4;
			}
			memset(temp, 0, sizeof temp);
	  	  	tmp = fscanf(fp, "%s", temp);
			if(strcmp(temp, ":") != 0) {
				fclose(fp);
				return -4;
			}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->canTransmitSetData.normalData[ch][i].function_div
				= (short int)atoi(buf);

			if(myData->CAN.config.tx_file_format == 0) {
				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "user_cont") != 0) {
					fclose(fp);
					return -4;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -4;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.normalData[ch][i].function_div2
					= (short int)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "tmp_usr_con") != 0) {
					fclose(fp);
					return -4;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -4;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.normalData[ch][i].function_div3
					= (short int)atoi(buf);
			} else if(myData->CAN.config.tx_file_format == 1
				|| myData->CAN.config.tx_file_format == 2
				|| myData->CAN.config.tx_file_format == 3) { //kjg_180405
				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "func_div2") != 0) {
					fclose(fp);
					return -4;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -4;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.normalData[ch][i].function_div2
					= (short int)atoi(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "func_div3") != 0) {
					fclose(fp);
					return -4;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -4;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.normalData[ch][i].function_div3
					= (short int)atoi(buf);

				if(myData->CAN.config.tx_file_format == 3) { //kjg_180405
					memset(temp, 0, sizeof temp);
			    	tmp = fscanf(fp, "%s", temp);
					if(strcmp(temp, "dlc") != 0) {
						fclose(fp);
						return -4;
					}
					memset(temp, 0, sizeof temp);
	  			  	tmp = fscanf(fp, "%s", temp);
					if(strcmp(temp, ":") != 0) {
						fclose(fp);
						return -4;
					}
					memset(buf, 0, sizeof buf);
					tmp = fscanf(fp, "%s", buf);
					myData->canTransmitSetData.normalData[ch][i].dlc
						= (unsigned char)atoi(buf);
				}

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved2") != 0) {
					fclose(fp);
					return -4;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -4;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.normalData[ch][i].reserved2
					= atol(buf);

				memset(temp, 0, sizeof temp);
		    	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "reserved3") != 0) {
					fclose(fp);
					return -4;
				}
				memset(temp, 0, sizeof temp);
	  		  	tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, ":") != 0) {
					fclose(fp);
					return -4;
				}
				memset(buf, 0, sizeof buf);
				tmp = fscanf(fp, "%s", buf);
				myData->canTransmitSetData.normalData[ch][i].reserved3
					= (short int)atoi(buf);
			}
		}
	   	fclose(fp);

		count = 0;
		for(i=0; i < MAX_CAN_DATA; i++) { //master
			if(myData->canTransmitSetData.normalData[ch][i].canType == 1)
				count++;
		}
		myData->canTransmitDataCount[ch][0] = count;

		count = 0;
		for(i=0; i < MAX_CAN_DATA; i++) { //slave
			if(myData->canTransmitSetData.normalData[ch][i].canType == 2)
				count++;
		}
		myData->canTransmitDataCount[ch][1] = count;

		userlog(DEBUG_LOG, psName, "CanTransmitSetData%02d file read end\n",
			ch+1);
	}

	return 0;
}

int	Write_CanTransmitSetData(void)
{
	char fileName[256];
	int i, ch, count;
    FILE *fp;

	//memset((char *)&myData->canTransmitDataCount, 0,
	//	sizeof(int) * MAX_CH_8 * MAX_CAN_TYPE);
	//csk_190331r

	for(ch=0; ch < MAX_CH_8; ch++) {
		memset(fileName, 0, sizeof fileName);
		sprintf(fileName,
			"/root/%s/config/parameter/can_comm/CanTransmitSetData%02d",
			myData->AppControl.misc.path1, ch+1);
	   	if((fp = fopen(fileName, "w")) == NULL) {
			userlog(DEBUG_LOG, psName,
				"CanTransmitSetData%02d file write error\n", ch+1);
			return -1;
		}

		for(i=0; i < MAX_CAN_TYPE; i++) {
			if(i == 0) {
				fprintf(fp, "master\n");
				fprintf(fp, "can_baudrate : %d\n",
					myData->canTransmitSetData.commonData[ch][i].can_baudrate);
			} else {
				fprintf(fp, "\nslave\n");
				fprintf(fp, "can_baudrate : %d\n",
					myData->canTransmitSetData.commonData[ch][i].can_baudrate);
			}
			fprintf(fp, "extended_id : %d\n",
				myData->canTransmitSetData.commonData[ch][i].extended_id);
			fprintf(fp, "controller_canID : %lx\n",
				myData->canTransmitSetData.commonData[ch][i].controller_canID);

			if(myData->CAN.config.tx_file_format == 0) {
			} else if(myData->CAN.config.tx_file_format == 1) {
				fprintf(fp, "bms_type	:	%d\n",
					myData->canTransmitSetData.commonData[ch][i].bms_type);
				fprintf(fp, "sjw        :	%d\n",
					myData->canTransmitSetData.commonData[ch][i].sjw);
				fprintf(fp, "reserved1	:	0\n");
					//kjg_180405 myData->canTransmitSetData.commonData[ch][i].reserved1[0]);
				fprintf(fp, "reserved2	:	0\n");
					//kjg_180405 myData->canTransmitSetData.commonData[ch][i].reserved1[1]);
			} else if(myData->CAN.config.tx_file_format == 2) {
				fprintf(fp, "bms_type	:	%d\n",
					myData->canTransmitSetData.commonData[ch][i].bms_type);
				fprintf(fp, "sjw        :	%d\n",
					myData->canTransmitSetData.commonData[ch][i].sjw);
				fprintf(fp, "reserved1	:	0\n");
					//kjg_180405 myData->canTransmitSetData.commonData[ch][i].reserved1[0]);
				fprintf(fp, "reserved2	:	0\n");
					//kjg_180405 myData->canTransmitSetData.commonData[ch][i].reserved1[1]);

				fprintf(fp,
					"func_div	:	%d %d %d %d %d %d %d %d %d %d\n",
					myData->canTransmitSetData.commonData[ch][i].func_div[0],
					myData->canTransmitSetData.commonData[ch][i].func_div[1],
					myData->canTransmitSetData.commonData[ch][i].func_div[2],
					myData->canTransmitSetData.commonData[ch][i].func_div[3],
					myData->canTransmitSetData.commonData[ch][i].func_div[4],
					myData->canTransmitSetData.commonData[ch][i].func_div[5],
					myData->canTransmitSetData.commonData[ch][i].func_div[6],
					myData->canTransmitSetData.commonData[ch][i].func_div[7],
					myData->canTransmitSetData.commonData[ch][i].func_div[8],
					myData->canTransmitSetData.commonData[ch][i].func_div[9]);

				fprintf(fp,
					"compare_type	:	%d %d %d %d %d %d %d %d %d %d\n",
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[0],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[1],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[2],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[3],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[4],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[5],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[6],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[7],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[8],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[9]);

				fprintf(fp,
					"data_type	:	%d %d %d %d %d %d %d %d %d %d\n",
					myData->canTransmitSetData.commonData[ch][i].data_type[0],
					myData->canTransmitSetData.commonData[ch][i].data_type[1],
					myData->canTransmitSetData.commonData[ch][i].data_type[2],
					myData->canTransmitSetData.commonData[ch][i].data_type[3],
					myData->canTransmitSetData.commonData[ch][i].data_type[4],
					myData->canTransmitSetData.commonData[ch][i].data_type[5],
					myData->canTransmitSetData.commonData[ch][i].data_type[6],
					myData->canTransmitSetData.commonData[ch][i].data_type[7],
					myData->canTransmitSetData.commonData[ch][i].data_type[8],
					myData->canTransmitSetData.commonData[ch][i].data_type[9]);

				fprintf(fp, "value	:	%f %f %f %f %f %f %f %f %f %f\n",
					myData->canTransmitSetData.commonData[ch][i].value[0],
					myData->canTransmitSetData.commonData[ch][i].value[1],
					myData->canTransmitSetData.commonData[ch][i].value[2],
					myData->canTransmitSetData.commonData[ch][i].value[3],
					myData->canTransmitSetData.commonData[ch][i].value[4],
					myData->canTransmitSetData.commonData[ch][i].value[5],
					myData->canTransmitSetData.commonData[ch][i].value[6],
					myData->canTransmitSetData.commonData[ch][i].value[7],
					myData->canTransmitSetData.commonData[ch][i].value[8],
					myData->canTransmitSetData.commonData[ch][i].value[9]);
			} else if(myData->CAN.config.tx_file_format == 3) {
				fprintf(fp, "bms_type	:	%d\n",
					myData->canTransmitSetData.commonData[ch][i].bms_type);
				fprintf(fp, "sjw        :	%d\n",
					myData->canTransmitSetData.commonData[ch][i].sjw);
				fprintf(fp, "can_fd_flag :	%d\n",
					myData->canTransmitSetData.commonData[ch][i].can_fd_flag);
				fprintf(fp, "can_datarate	:	%d\n",
					myData->canTransmitSetData.commonData[ch][i].can_datarate);
				fprintf(fp, "terminal_r	:	%d\n",
					myData->canTransmitSetData.commonData[ch][i].terminal_r);
				fprintf(fp, "crc_type	:	%d\n",
					myData->canTransmitSetData.commonData[ch][i].crc_type);

				fprintf(fp,
					"func_div	:	%d %d %d %d %d %d %d %d %d %d\n",
					myData->canTransmitSetData.commonData[ch][i].func_div[0],
					myData->canTransmitSetData.commonData[ch][i].func_div[1],
					myData->canTransmitSetData.commonData[ch][i].func_div[2],
					myData->canTransmitSetData.commonData[ch][i].func_div[3],
					myData->canTransmitSetData.commonData[ch][i].func_div[4],
					myData->canTransmitSetData.commonData[ch][i].func_div[5],
					myData->canTransmitSetData.commonData[ch][i].func_div[6],
					myData->canTransmitSetData.commonData[ch][i].func_div[7],
					myData->canTransmitSetData.commonData[ch][i].func_div[8],
					myData->canTransmitSetData.commonData[ch][i].func_div[9]);

				fprintf(fp,
					"compare_type	:	%d %d %d %d %d %d %d %d %d %d\n",
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[0],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[1],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[2],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[3],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[4],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[5],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[6],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[7],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[8],
					myData->canTransmitSetData.commonData[ch][i]
						.compare_type[9]);

				fprintf(fp,
					"data_type	:	%d %d %d %d %d %d %d %d %d %d\n",
					myData->canTransmitSetData.commonData[ch][i].data_type[0],
					myData->canTransmitSetData.commonData[ch][i].data_type[1],
					myData->canTransmitSetData.commonData[ch][i].data_type[2],
					myData->canTransmitSetData.commonData[ch][i].data_type[3],
					myData->canTransmitSetData.commonData[ch][i].data_type[4],
					myData->canTransmitSetData.commonData[ch][i].data_type[5],
					myData->canTransmitSetData.commonData[ch][i].data_type[6],
					myData->canTransmitSetData.commonData[ch][i].data_type[7],
					myData->canTransmitSetData.commonData[ch][i].data_type[8],
					myData->canTransmitSetData.commonData[ch][i].data_type[9]);

				fprintf(fp, "value	:	%f %f %f %f %f %f %f %f %f %f\n",
					myData->canTransmitSetData.commonData[ch][i].value[0],
					myData->canTransmitSetData.commonData[ch][i].value[1],
					myData->canTransmitSetData.commonData[ch][i].value[2],
					myData->canTransmitSetData.commonData[ch][i].value[3],
					myData->canTransmitSetData.commonData[ch][i].value[4],
					myData->canTransmitSetData.commonData[ch][i].value[5],
					myData->canTransmitSetData.commonData[ch][i].value[6],
					myData->canTransmitSetData.commonData[ch][i].value[7],
					myData->canTransmitSetData.commonData[ch][i].value[8],
					myData->canTransmitSetData.commonData[ch][i].value[9]);
			}
		}

		for(i=0; i < MAX_CAN_DATA; i++) {
			fprintf(fp, "\nindex : %03d\n", i);
			fprintf(fp, "canType     : %d\n",
				myData->canTransmitSetData.normalData[ch][i].canType);
			fprintf(fp, "byte_order  : %d\n",
				myData->canTransmitSetData.normalData[ch][i].byte_order);
			fprintf(fp, "data_type   : %d\n",
				myData->canTransmitSetData.normalData[ch][i].data_type);
			fprintf(fp, "factor      : %f\n",
				myData->canTransmitSetData.normalData[ch][i].factor);
			fprintf(fp, "offset      : %f\n",
				myData->canTransmitSetData.normalData[ch][i].offset);
			fprintf(fp, "def_value   : %f\n",
				myData->canTransmitSetData.normalData[ch][i].default_value);
			fprintf(fp, "startBit    : %d\n",
				myData->canTransmitSetData.normalData[ch][i].startBit);
			fprintf(fp, "bitCount    : %d\n",
				myData->canTransmitSetData.normalData[ch][i].bitCount);
			fprintf(fp, "canID       : %ld\n",
				myData->canTransmitSetData.normalData[ch][i].canID);
			fprintf(fp, "send_period : %ld\n",
				myData->canTransmitSetData.normalData[ch][i].send_period);

			if(myData->canTransmitSetData.normalData[ch][i].name[0] == 0) {
				fprintf(fp, "name        : $\n");
			} else {
				fprintf(fp, "name        : %s\n",
					myData->canTransmitSetData.normalData[ch][i].name);
			}

			fprintf(fp, "func_div    : %d\n",
				myData->canTransmitSetData.normalData[ch][i].function_div);
			if(myData->CAN.config.tx_file_format == 0) {
				fprintf(fp, "user_cont    : %d\n",
					myData->canTransmitSetData.normalData[ch][i].function_div2);
				fprintf(fp, "tmp_usr_con  : %d\n",
					myData->canTransmitSetData.normalData[ch][i].function_div3);
			} else if(myData->CAN.config.tx_file_format == 1
				|| myData->CAN.config.tx_file_format == 2) {
				fprintf(fp, "func_div2	: %d\n",
					(short int)myData->canTransmitSetData
					.normalData[ch][i].function_div2);
				fprintf(fp, "func_div3	: %d\n",
					(short int)myData->canTransmitSetData
					.normalData[ch][i].function_div3);
				fprintf(fp, "reserved2	: %ld\n",
					myData->canTransmitSetData.normalData[ch][i].reserved2);
				fprintf(fp, "reserved3	: %d\n",
					(short int)myData->canTransmitSetData
					.normalData[ch][i].reserved3);
			} else { //kjg_180405 tx_file_format == 3
				fprintf(fp, "func_div2	: %d\n",
					(short int)myData->canTransmitSetData
					.normalData[ch][i].function_div2);
				fprintf(fp, "func_div3	: %d\n",
					(short int)myData->canTransmitSetData
					.normalData[ch][i].function_div3);
				fprintf(fp, "dlc			: %d\n",
					(unsigned char)myData->canTransmitSetData
					.normalData[ch][i].dlc);
				fprintf(fp, "reserved2	: %ld\n",
					myData->canTransmitSetData.normalData[ch][i].reserved2);
				fprintf(fp, "reserved3	: %d\n",
					(short int)myData->canTransmitSetData
					.normalData[ch][i].reserved3);
			}
		}
   		fclose(fp);

		count = 0;
		for(i=0; i < MAX_CAN_DATA; i++) { //master
			if(myData->canTransmitSetData.normalData[ch][i].canType == 1)
				count++;
		}
		myData->canTransmitDataCount[ch][0] = count;

		count = 0;
		for(i=0; i < MAX_CAN_DATA; i++) { //slave
			if(myData->canTransmitSetData.normalData[ch][i].canType == 2)
				count++;
		}
		myData->canTransmitDataCount[ch][1] = count;
	}

	return 0;
}
//jhkw_191108s
void Copy_Th_Table(int table)
{
	char cmd[256];

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "cp -rf ");
	strcat(cmd, "/root/system_data/config/parameter/th_table/");
	strcat(cmd, "th_table_00.txt ");
	strcat(cmd, "/root/system_data/config/parameter/th_table/");
	if(table == 1) {
		strcat(cmd, "th_table_01.txt ");
	} else if(table == 2) {
		strcat(cmd, "th_table_02.txt ");
	} else if(table == 3) {
		strcat(cmd, "th_table_03.txt ");
	} else if(table == 4) {
		strcat(cmd, "th_table_04.txt ");
	} else if(table == 5) {
		strcat(cmd, "th_table_05.txt ");
	} else if(table == 6) {
		strcat(cmd, "th_table_06.txt ");
	} else if(table == 7) {
		strcat(cmd, "th_table_07.txt ");
	} else if(table == 8) {
		strcat(cmd, "th_table_08.txt ");
	} else if(table == 9) {
		strcat(cmd, "th_table_09.txt ");
	} else if(table == 10) {
		strcat(cmd, "th_table_10.txt ");
	} else if(table == 11) {
		strcat(cmd, "th_table_11.txt ");
	} else if(table == 12) {
		strcat(cmd, "th_table_12.txt ");
	} else if(table == 13) {
		strcat(cmd, "th_table_13.txt ");
	} else if(table == 14) {
		strcat(cmd, "th_table_14.txt ");
	} else if(table == 15) {
		strcat(cmd, "th_table_15.txt ");
	} else if(table == 16) {
		strcat(cmd, "th_table_16.txt ");
	} else if(table == 17) {
		strcat(cmd, "th_table_17.txt ");
	} else if(table == 18) {
		strcat(cmd, "th_table_18.txt ");
	} else if(table == 19) {
		strcat(cmd, "th_table_19.txt ");
	}
	system(cmd);
	userlog(DEBUG_LOG, psName, "th_table_copy table No : %d\n", table);
	usleep(500000);
}
//jhkw_191108e

//jhkw_200317s
void Copy_Measure_cali(int mode)
{
	char cmd[256];

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "cp -rf ");
	strcat(cmd, "/root/system_data/config/parameter/");
	strcat(cmd, "Measure_Cali_1_update ");
	strcat(cmd, "/root/system_data/config/parameter/");
	strcat(cmd, "Measure_Cali_1");
	system(cmd);
	userlog(DEBUG_LOG, psName, "Measure_Cali_1_update : %d\n", mode);
	usleep(100000);
}
//jhkw_200317e

void Sync_Time(void)
{
	char temp[32], buf[32], fileName[256];
	int tmp;
	FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "date > /root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/tmp/Sync_Time");
	system(fileName);
	usleep(100000);

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/tmp/Sync_Time");
	if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Sync_Time file read error\n");
		return;
	}

	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//month
	if(strcmp(buf, "Jan") == 0) {
		myData->mData.real_time[5] = 1;
	} else if(strcmp(buf, "Feb") == 0) {
		myData->mData.real_time[5] = 2;
	} else if(strcmp(buf, "Mar") == 0) {
		myData->mData.real_time[5] = 3;
	} else if(strcmp(buf, "Apr") == 0) {
		myData->mData.real_time[5] = 4;
	} else if(strcmp(buf, "May") == 0) {
		myData->mData.real_time[5] = 5;
	} else if(strcmp(buf, "Jun") == 0) {
		myData->mData.real_time[5] = 6;
	} else if(strcmp(buf, "Jul") == 0) {
		myData->mData.real_time[5] = 7;
	} else if(strcmp(buf, "Aug") == 0) {
		myData->mData.real_time[5] = 8;
	} else if(strcmp(buf, "Sep") == 0) {
		myData->mData.real_time[5] = 9;
	} else if(strcmp(buf, "Oct") == 0) {
		myData->mData.real_time[5] = 10;
	} else if(strcmp(buf, "Nov") == 0) {
		myData->mData.real_time[5] = 11;
	} else if(strcmp(buf, "Dec") == 0) {
		myData->mData.real_time[5] = 12;
	}

	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.real_time[4] = atoi(buf); //day

	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	memset(temp, 0, sizeof buf);
	strncat(temp, buf, 2);
	myData->mData.real_time[3] = atoi(temp); //hour

	memset(temp, 0, sizeof buf);
	strncat(temp, buf + 3, 2);
	myData->mData.real_time[2] = atoi(temp); //min

	memset(temp, 0, sizeof buf);
	strncat(temp, buf + 6, 2);
	myData->mData.real_time[1] = atoi(temp); //sec
	myData->mData.real_time[0] = 0; //msec

	tmp = fscanf(fp, "%s", temp);
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.real_time[6] = atoi(buf); //year

	fclose(fp);
}

int Read_COM_Config(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp, i;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/COM_Config");
	// /root/system_data/config/parameter/COM_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "COM_Config file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "functionType") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->COM.config.functionType[i] = (unsigned char)atoi(buf);
		//0 : COM_FUNC_TYPE_NONE
		//1 : COM_FUNC_TYPE_METER1
		//	- 0:Agilent34401A
		//2 : COM_FUNC_TYPE_DISPLAY1
		//	- 0:PNE_PanelMeter_A(232), 1:B(485), 2:C(mcu2,ch4)
		//3 : COM_FUNC_TYPE_METER2(temp)
		//	- 0:CB-7081, 1:XL-122, 2:COMM_A(XL-122)
		//4 : COM_FUNC_TYPE_CALI_SWITCH1
		//	- 0:Calibrator_B, 1:C, 2:D
		//5 : COM_FUNC_TYPE_BCR1
		//	- 0:MS-860(RS-485), 1:COMM_A(MS-860)
		//6 : COM_FUNC_TYPE_IO_COMM
		//	- 0:EIOM_A, 1:PLC_Q03UD
		//7 : COM_FUNC_TYPE_CHAMBER
		//  - 0:TEMP880
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "functionModel") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->COM.config.functionModel[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "comPortId") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->COM.config.comPortId[i] = atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "comBps") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->COM.config.comBps[i] = atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "commType") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
		//0:parity_none 1stop 8data_length
		//1:parity_even 2stop 7data_length
		//2:parity_even 1stop 7data_length
    	myData->COM.config.commType[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "externPort") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->COM.config.externPort[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdSendLog") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->COM.config.CmdSendLog[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdRcvLog") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->COM.config.CmdRcvLog[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdSendLog_Hex") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->COM.config.CmdSendLog_Hex[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CmdRcvLog_Hex") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->COM.config.CmdRcvLog_Hex[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "CommCheckLog") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->COM.config.CommCheckLog[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "autoStart") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->COM.config.autoStart[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "countMeter") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
    	myData->COM.config.countMeter[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "readType") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
		//0:(READ_V_V)v-v,i-v, 1:(READ_V_I)v-v,i-i
    	myData->COM.config.readType[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "measureI") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
	    tmp = fscanf(fp, "%s", buf);
		//1:shunt 0.010ohm, 2:shunt 0.001ohm, 3:DCCT 600A/400mA,
		//4:DCCT 150A/200mA, 5:shunt 10ohm/100ohm, 6:meter DCI,
		//7:shunt 300A/100mV, 8:shunt 600A/100mV, 9:shunt 0.00025ohm,
		//10:DCCT 1000A/666mA
    	myData->COM.config.measureI[i] = (unsigned char)atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "reserved1") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->COM.config.reserved1[i] = atol(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "reserved2") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_COM_PORT; i++) {
		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->COM.config.reserved2[i] = atol(buf);
	}

    fclose(fp);
	return 0;
}

int Read_DAQ_Config(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp, i;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/DAQ_Config");
	// /root/system_data/config/parameter/DAQ_Config
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "DAQ_Config file read error\n");
		return -1;
	}
	memset((char *)&myData->daq.config, 0, sizeof(S_DAQ_CONFIG));

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "daq_use") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -3;
	}

	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->daq.config.daq_use = atoi(buf);
	if(myData->daq.config.daq_use == 0){
	    fclose(fp);
		userlog(DEBUG_LOG, psName, "DAQ not use\n");
		return 0;
	}

	//jhkw_201117s
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Compare_flag") != 0) {
		fclose(fp);
		return -4;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -5;
	}

	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->daq.config.Compare_flag = atoi(buf);
	
	//shhw_230611s
	memset(temp, 0, sizeof temp); 			
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "useDampingResistor") != 0) {
		fclose(fp);
		return -6;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -7;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->daq.config.useDampingResistor = (unsigned char)atoi(buf);
	//damping resistor 1: use, 0: not use
	//shhw_230611e
	
	//jhkw_201117e
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "version") != 0) {
		fclose(fp);
		return -8;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -9;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	memcpy((char *)&myData->daq.config.version, (char *)&buf, 16);
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "daq_type") != 0) {
		fclose(fp);
		return -10;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -11;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->daq.config.daq_type = atoi(buf);
	myData->mData.config.daq_type
		= myData->daq.config.daq_type;
		
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedDAQ") != 0) {
		fclose(fp);
		return -12;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -13;
	}

	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->daq.config.installedDAQ = atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedAuxV") != 0) {
		fclose(fp);
		return -14;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -15;
	}

	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->daq.config.installedAuxV = atoi(buf);
	myData->mData.config.installedAuxV
		= myData->daq.config.installedAuxV;

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedTH") != 0) {
		fclose(fp);
		return -16;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -17;
	}

	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->daq.config.installedTH = atoi(buf);
	myData->mData.config.installedTH
		= myData->daq.config.installedTH;

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Aux_in_DAQ") != 0) {
		fclose(fp);
		return -18;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -19;
	}

	for(i=0; i < MAX_DAQ_BD_NUM; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->daq.config.Aux_in_DAQ[i] = atoi(buf);
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Aux_type") != 0) {
		fclose(fp);
		return -20;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -21;
	}

	for(i=0; i < MAX_DAQ_BD_NUM; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->daq.config.Aux_type[i] = atoi(buf);
	}

    fclose(fp);
	return 0;
}



int Read_Measure_Cali_1(void)
{
    int tmp, i;
	char temp[32], buf[32], fileName[256];
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	strcat(fileName, "/config/parameter/Measure_Cali_1");
	// /root/system_data/config/parameter/Measure_Cali_1
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Measure_Cali_1 file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "index") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "factor") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "offset") != 0) {
		fclose(fp);
		return -2;
	}

	//temperature calibration
	for(i=0; i < MAX_SUB_SENS_V_DATA; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "%03d", i+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -3;
		}

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->measure_cali[0][i].factor = atof(buf);

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->measure_cali[0][i].offset = atof(buf);
	}

    fclose(fp);
	return 0;
}

//jhkw_200317s
int Read_Measure_Cali_2(int mode)
{
    int tmp, i;
	char temp[32], buf[32], fileName[256];
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);
	if(mode == 1) {
		strcat(fileName, "/config/parameter/Measure_Cali_1_zero");
	} else if(mode == 2) {
		strcat(fileName, "/config/parameter/Measure_Cali_1_update");
	} else {
		strcat(fileName, "/config/parameter/Measure_Cali_1");
	}
	// /root/system_data/config/parameter/Measure_Cali_1
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Measure_Cali_1 file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "index") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "factor") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "offset") != 0) {
		fclose(fp);
		return -2;
	}

	//temperature calibration
	for(i=0; i < MAX_SUB_SENS_V_DATA; i++) {
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		memset(buf, 0, sizeof buf);
		sprintf(buf, "%03d", i+1);
		if(strcmp(temp, buf) != 0) {
			fclose(fp);
			return -3;
		}

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->measure_cali[0][i].factor = atof(buf);

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->measure_cali[0][i].offset = atof(buf);
	}

    fclose(fp);
	return 0;
}
//jhkw_200317e

int Read_Th_Table(void)
{
	int i;
	switch(myData->AppControl.config.systemModel) {
		case C_LGC_50V_500A_10A_42KW:
		case C_LGC_50V_500A_10A_42KW_2:
		case C_LGC_50V_500A_10A_42KW_3:
		case C_LGC_50V_500A_10A_42KW_4:
			if(Read_Th_Table_Type(0, 1) < 0) return -1; //103AT
			break;
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
		case C_SDI_70V_250A_25A_18KW:
			if(Read_Th_Table_Type(0, 4) < 0) return -1; //TPW_09M06
			break;
		case C_LGC_70V_250A_50A_35KW_9:		//jhk_160515
		case C_LGC_70V_250A_50A_35KW_10:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_11:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_12:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_13:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_14:	//jhk_160515
		case C_LGC_70V_250A_50A_35KW_15://jhk_160515
		case C_LGC_70V_250A_50A_35KW_16:	//jhk_160515
		case C_LGCUSA_70V_300A_42KW:	//jhk_160802
		case C_LGCUSA_70V_300A_42KW_2:	//jhk_160802
		case C_LGCUSA_70V_350A_24KW:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_2:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_3:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_4:	//jhk_160720
		case C_LGCCHINA_80V_150A_12KW:		//jhk_160725
		case C_LGCCHINA_80V_150A_12KW_2:	//jhk_160725
		case C_LGCCHINA_100V_200A_20KW_3:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_4:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_5:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_6:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_7:	//jhk_160731
		case C_LGC_120V_250A_50A_60KW:	//jhk_160617
			if(Read_Th_Table_Type(0, 1) < 0) return -1; //103AT
			break;
		case C_LGC_400V_100A_10A_40KW:
		case C_3PSYSTEM_400V_100A_10A_40KW_1:	//jhk_120723
		case C_3PSYSTEM_400V_100A_10A_40KW_2:	//jhk_120723
			if(Read_Th_Table_Type(0, 1) < 0) return -1; //103AT
			if(Read_Th_Table_Type(1, 2) < 0) return -2; //103JT_025
			break;
		case C_LGC_450V_200A_20A_180KW:
		case C_LGC_450V_200A_20A_180KW_2:
			if(Read_Th_Table_Type(0, 1) < 0) return -1; //103AT
			if(Read_Th_Table_Type(1, 2) < 0) return -2; //103JT_025
			if(Read_Th_Table_Type(2, 3) < 0) return -3; //KCG103F343F
			break;
		case C_LGC_450V_250A_50A_225KW:		//jhk_160608
		case C_LGC_450V_250A_50A_225KW_2:	//jhk_160608
			if(Read_Th_Table_Type(0, 0) < 0) return -1; //non
			if(Read_Th_Table_Type(1, 1) < 0) return -2; //103AT
			if(Read_Th_Table_Type(2, 2) < 0) return -3; //103JT_025
			if(Read_Th_Table_Type(3, 3) < 0) return -4; //KCG103F343F
			if(Read_Th_Table_Type(4, 4) < 0) return -5; //reserved
			break;
		case C_LGCCHINA_500V_150A_75KW:		//jhk_160727
		case C_LGCCHINA_500V_150A_75KW_2:	//jhk_160727
		case C_LGCCHINA_500V_150A_75KW_3:	//jhk_160727
			if(Read_Th_Table_Type(0, 1) < 0) return -1; //103AT
			break;
		case C_SEBANG_600V_200A_100A_240KW: //jhk_160223
		case C_SEBANG_1200V_300A_100A_360KW: //jhk_160223
			//kjh_160610s
			if(Read_Th_Table_Type(0, 0) < 0) return -1; //non
			if(Read_Th_Table_Type(1, 1) < 0) return -2; //103AT
			if(Read_Th_Table_Type(2, 2) < 0) return -3; //
			if(Read_Th_Table_Type(3, 3) < 0) return -4; //
			if(Read_Th_Table_Type(4, 4) < 0) return -5; //
			//kjh_160610e
			break;
		default:
			//kjh_160610s
			for(i = 0; i < MAX_TH_TABLE; i++) {
				if(Read_Th_Table_Type(i, i) < 0) return (i + 1) * (-1);
			}
			/*if(Read_Th_Table_Type(0, 0) < 0) return -1; //non
			if(Read_Th_Table_Type(1, 1) < 0) return -2; //103AT
			if(Read_Th_Table_Type(2, 2) < 0) return -3; //103JT_025
			if(Read_Th_Table_Type(3, 3) < 0) return -4; //KCG103F343F
			if(Read_Th_Table_Type(4, 4) < 0) return -5; //reserved
			if(Read_Th_Table_Type(5, 5) < 0) return -6; //reserved
			if(Read_Th_Table_Type(6, 6) < 0) return -7; //reserved
			if(Read_Th_Table_Type(7, 7) < 0) return -8; //reserved
			if(Read_Th_Table_Type(8, 8) < 0) return -9; //reserved
			if(Read_Th_Table_Type(9, 9) < 0) return -10; //reserved*/
			//kjh_160610e
			break;
	}

	return 0;
}

int Read_Th_Table_Type(int table, int type)
{
	char temp[32], buf[32], fileName[256];
    int tmp, i;
	float Vref, R1, R2, Rth, val;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	sprintf(fileName,
		"/root/%s/config/parameter/th_table/th_table_%02d.txt",
		myData->AppControl.misc.path1, type);
	// /root/system_data/config/parameter/th_table/th_table_##.txt
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "th_table_%02d file read error\n", type);
		return -1;
	}

	myData->th_table[table].th_type = (unsigned char)type;

	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp); 	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Bias_type") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
   	myData->th_table[table].bias_type = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Vref[mV]") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	Vref = atof(buf); //mV
   	myData->th_table[table].Vref = Vref;

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "R1[ohm]") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	R1 = atof(buf); //ohm
   	myData->th_table[table].R1 = R1;

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "R2[ohm]") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -2;
	}
	memset(buf, 0, sizeof buf);
   	tmp = fscanf(fp, "%s", buf);
	R2 = atof(buf); //ohm
   	myData->th_table[table].R2 = R2;


	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "temp_degreeC") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "R_kOhm") != 0) {
		fclose(fp);
		return -2;
	}
	for(i=0; i < MAX_TH_DATA; i++) {
		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->th_table[table].T_R[i][0] = atof(buf) * 1000.0; //temp * 1000
		if(strcmp(buf, "9999") == 0) { //equal
			if(i == 0) {
			} else {
				i = i - 1;
			}
			myData->th_table[table].th_data_max_index = (short int)i;
			break;
		}

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
		Rth = atof(buf) * 1000.0; //ohm
   		myData->th_table[table].T_R[i][1] = Rth;

		val = Vref * Rth / (R1 + R2 + Rth);
		myData->th_table[table].V_TH[i] = (float)val; //mV
	}

    fclose(fp);
	return 0;
}

int Read_CanFlashFile(void)
{
	char fileName[256], temp[32], buf[256];
	unsigned char flash_file[MAX_CAN_FLASH_FILE_SIZE];
	int	fp2, tmp, rtn, total_block, block_point, block_size, block_count_check;
	FILE *fp1;

	switch(myData->AppControl.config.systemModel) {
		case C_LGC_500V_200A_10A_200KW:
			rtn = 1;
			break;
		default:
			rtn = 0;
			break;
	}
	if(rtn == 0) return 0;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName,
		"/root/system_data/config/parameter/can_comm/can_flash_file");
	if((fp1 = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "can_flash_file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof(temp));
	tmp = fscanf(fp1, "%s", temp);
	if(strcmp(temp, "name") != 0) {
		fclose(fp1);
		return -2;
	}
	memset(temp, 0, sizeof(temp));
	tmp = fscanf(fp1, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp1);
		return -3;
	}
	memset(buf, 0, sizeof(buf));
	tmp = fscanf(fp1, "%s", buf);

	memset(fileName, 0, sizeof(fileName));
	sprintf(fileName, "/root/system_data/config/parameter/can_comm/%s", buf);

	if((fp2 = open(fileName, O_RDONLY)) < 0) {
		userlog(DEBUG_LOG, psName, "Can not open %s file(read)\n", fileName);
		return -4;
	}

	rtn = read(fp2, (char *)flash_file, MAX_CAN_FLASH_FILE_SIZE);
	if(rtn <= 0 || rtn > MAX_CAN_FLASH_FILE_SIZE) {
		userlog(DEBUG_LOG, psName, "Read error %s\n", fileName);
		return -5;
	}
	close(fp2);

	memset((unsigned char *)&myData->CAN.can_flash_file, 0,
		MAX_CAN_FLASH_FILE_SIZE);
	memcpy((unsigned char *)&myData->CAN.can_flash_file,
		(unsigned char *)&flash_file[0], rtn);
	//memcpy((char *)&myData->CAN.can_flash_file_name[0], (char *)&buf[0], 128);

	//printf("kjg_d1\n");
	for(tmp=0; tmp < rtn; tmp++) {
	//	printf("%c", myData->CAN.can_flash_file[tmp]);
		//'{' = 0x7B
		//'}' = 0x7D
		if(myData->CAN.can_flash_file[tmp] == '}') break;
	}
	//printf("\nend %d %d, %d\n", rtn, tmp, rtn-tmp);

	userlog(DEBUG_LOG, psName, "%s : file_size %d = %d + %d\n",
		buf, rtn, tmp, rtn-tmp);

	if(tmp >= MAX_CAN_FLASH_HEADER_SIZE) {
		userlog(DEBUG_LOG, psName, "Hader size error %s\n", fileName);
		return -6;
	}

	total_block = 0;
	tmp++;
	block_count_check = 1;

	while(block_count_check) {
		if(total_block >= 4) {
			userlog(DEBUG_LOG, psName,
				"can_flash total_block error %d\n", total_block);
			return -7;
		}

		block_point = tmp;
		myData->CAN.can_flash_data_block_point[total_block] = block_point;
		block_size = (int)myData->CAN.can_flash_file[block_point + 4]
			* 0x01000000;
		block_size += ((int)myData->CAN.can_flash_file[block_point + 5]
			* 0x00010000);
		block_size += ((int)myData->CAN.can_flash_file[block_point + 6]
			* 0x00000100);
		block_size += (int)myData->CAN.can_flash_file[block_point + 7];
		myData->CAN.can_flash_data_block_size[total_block] = block_size;
		tmp = block_point + 8 + block_size + 2;
		if(tmp >= rtn) block_count_check = 0;
		total_block++;
		if(total_block == 3) break;
	}

	myData->CAN.can_flash_data_total_block = total_block;

	memcpy((char *)&myData->CAN.can_flash_file_name[0], (char *)&buf[0], 128);
	
	userlog(DEBUG_LOG, psName,
		"can_flash total_block:%d, point %d %d %d %d, size %d %d %d %d\n",
			myData->CAN.can_flash_data_total_block,
			myData->CAN.can_flash_data_block_point[0],
			myData->CAN.can_flash_data_block_point[1],
			myData->CAN.can_flash_data_block_point[2],
			myData->CAN.can_flash_data_block_point[3],
			myData->CAN.can_flash_data_block_size[0],
			myData->CAN.can_flash_data_block_size[1],
			myData->CAN.can_flash_data_block_size[2],
			myData->CAN.can_flash_data_block_size[3]);

	return 0;
}

int	Write_VBF_File(char *file_name, char *file_data)
{
	char fileName[128], vbf_name[128];
	int fp2, rtn;
    FILE *fp1;

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName,
		"/root/system_data/config/parameter/can_comm/can_flash_file");
    if((fp1 = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "VBF name file write error\n", psName);
		return -1;
	}

	memcpy((char *)&vbf_name, file_name, 128);
    fprintf(fp1, "name : %s\n", vbf_name);
    	
    fclose(fp1);

	userlog(DEBUG_LOG, psName, "write_vbf_file %s\n", vbf_name);

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName,
		"touch /root/system_data/config/parameter/can_comm/");
	strcat(fileName, vbf_name);

	system(fileName);

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName,
		"/root/system_data/config/parameter/can_comm/");
	strcat(fileName, vbf_name);

    if((fp2 = open(fileName, O_RDWR)) < 0) {
		userlog(DEBUG_LOG, psName, "VBF data file write error\n", psName);
		return -2;
	}

    rtn = write(fp2, file_data, MAX_CAN_FLASH_FILE_SIZE);
	if(rtn != MAX_CAN_FLASH_FILE_SIZE) {
		userlog(DEBUG_LOG, psName, "VBF data file write error2\n", psName);
	    close(fp2);
		return -3;
	} else {
	    close(fp2);
		return 0;
	}
}
/*kjg_d
int Read_KJG_Test_1(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp, i;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	sprintf(fileName,
		"/root/%s/config/parameter/th_table/1min_org.txt",
		myData->AppControl.misc.path1);
	// /root/system_data/config/parameter/th_table/1min_org.txt
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "1min_org file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Step") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "V") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "I") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "C") != 0) {
		fclose(fp);
		return -2;
	}

	for(i=0; i < 300; i++) {
		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
		if(strcmp(buf, "9999") == 0) { //equal
			myData->kjg_table_1[i][0] = -1.0;
			break;
		}
   		myData->kjg_table_1[i][0] = atof(buf);

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->kjg_table_1[i][1] = atof(buf);

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->kjg_table_1[i][2] = atof(buf);

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->kjg_table_1[i][3] = atof(buf);
	}

    fclose(fp);
	return 0;
}

int	Write_KJG_Test_1(void)
{
	char fileName[256];
	int i, j;
	double seed_Ah, sum_Ah, cal_Ah, Isens, diffI, diffT;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/th_table/1min_kjg.txt");
	// /root/system_data/config/parameter/th_table/1min_kjg.txt
   	if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "1min_kjg file write error\n");
		return -1;
	}

	seed_Ah = sum_Ah = cal_Ah = 0.0;

	fprintf(fp, "Step V I C\n");
	for(i=0; i < 300; i++) {
		if(myData->kjg_table_1[i][0] < 0.001) break;

		if(i == 0) {
			for(j=0; j < 5; j++) {
				sum_Ah += (double)myData->kjg_table_1[i][2];
				cal_Ah = seed_Ah + sum_Ah / 360000.0;
				if(sum_Ah > MAX_SUM_DOUBLE) {
					seed_Ah = cal_Ah;
					sum_Ah = 0.0;
				}
				fprintf(fp, "%f %f %f %f\n",
					myData->kjg_table_1[i][0] - (0.04 - (j * 0.01)),
					myData->kjg_table_1[i][1],
					myData->kjg_table_1[i][2],
					(float)cal_Ah);
			}
		} else if(i == 1) {
			for(j=0; j < 5995; j++) {
				sum_Ah += (double)myData->kjg_table_1[i][2];
				cal_Ah = seed_Ah + sum_Ah / 360000.0;
				if(sum_Ah > MAX_SUM_DOUBLE) {
					seed_Ah = cal_Ah;
					sum_Ah = 0.0;
				}
				fprintf(fp, "%f %f %f %f\n",
					myData->kjg_table_1[i][0] - (59.94 - (j * 0.01)),
					myData->kjg_table_1[i][1],
					myData->kjg_table_1[i][2],
					(float)cal_Ah);
			}
		} else {
			if(myData->kjg_table_1[i+1][0] >= 0) {
				for(j=0; j < 6000; j++) {
					diffI = (double)myData->kjg_table_1[i][2]
						- (double)myData->kjg_table_1[i-1][2];
					diffT = (double)myData->kjg_table_1[i][0]
						- (double)myData->kjg_table_1[i-1][0];
					diffT = (double)(j+1) * 0.01 / diffT;
					Isens = diffI * diffT + (double)myData->kjg_table_1[i-1][2];
					sum_Ah += (double)Isens;
					cal_Ah = seed_Ah + sum_Ah / 360000.0;
					if(sum_Ah > MAX_SUM_DOUBLE) {
						seed_Ah = cal_Ah;
						sum_Ah = 0.0;
					}
					fprintf(fp, "%f %f %f %f\n",
						myData->kjg_table_1[i][0] - (59.99 - ((double)j * 0.01)),
						myData->kjg_table_1[i][1],
						(float)Isens,
						(float)cal_Ah);
				}
			} else {
				for(j=0; j < 4799; j++) {
					diffI = (double)myData->kjg_table_1[i][2]
				   		- (double)myData->kjg_table_1[i-1][2];
					diffT = (double)myData->kjg_table_1[i][0]
						- (double)myData->kjg_table_1[i-1][0];
					diffT = (double)(j+1) * 0.01 / diffT;
					Isens = diffI * diffT + (double)myData->kjg_table_1[i-1][2];
					sum_Ah += (double)Isens;
					cal_Ah = seed_Ah + sum_Ah / 360000.0;
					if(sum_Ah > MAX_SUM_DOUBLE) {
						seed_Ah = cal_Ah;
						sum_Ah = 0.0;
					}
					fprintf(fp, "%f %f %f %f\n",
						myData->kjg_table_1[i][0] - (47.98 - ((double)j * 0.01)),
						myData->kjg_table_1[i][1],
						(float)Isens,
						(float)cal_Ah);
				}
			}
		}
	}
   	fclose(fp);

	return 0;
}

int Read_KJG_Test_2(void)
{
	char temp[32], buf[32], fileName[256];
    int tmp, i;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	sprintf(fileName,
		"/root/%s/config/parameter/th_table/10ms_org.txt",
		myData->AppControl.misc.path1);
	// /root/system_data/config/parameter/th_table/10ms_org.txt
    if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "10ms_org file read error\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Step") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "V") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "I") != 0) {
		fclose(fp);
		return -2;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "C") != 0) {
		fclose(fp);
		return -2;
	}

	for(i=0; i < 1240802; i++) {
		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
		if(strcmp(buf, "9999") == 0) { //equal
			myData->kjg_table_2[i][0] = -1.0;
			break;
		}
   		myData->kjg_table_2[i][0] = atof(buf);

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->kjg_table_2[i][1] = atof(buf);

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->kjg_table_2[i][2] = atof(buf);

		memset(buf, 0, sizeof buf);
    	tmp = fscanf(fp, "%s", buf);
   		myData->kjg_table_2[i][3] = atof(buf);
	}

    fclose(fp);
	return 0;
}

int	Write_KJG_Test_2(void)
{
	char fileName[256];
	int i;
	double sum_Ah, cal_Ah, diff_Ah, Isens, diffI, diffT;
    FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myData->AppControl.misc.path1);
	strcat(fileName, "/config/parameter/th_table/10ms_kjg.txt");
	// /root/system_data/config/parameter/th_table/10ms_kjg.txt
   	if((fp = fopen(fileName, "w")) == NULL) {
		userlog(DEBUG_LOG, psName, "10ms_kjg file write error\n");
		return -1;
	}

	sum_Ah = cal_Ah = diff_Ah = 0.0;

	fprintf(fp, "Step V I C C2 diff_C\n");
	for(i=0; i < 1240802; i++) {
		if(myData->kjg_table_2[i][0] < 0.001) break;

		if(i == 0) {
			cal_Ah = (double)myData->kjg_table_2[i][2]
				* (double)myData->kjg_table_2[i][0] / 3600.0;
			sum_Ah = cal_Ah;
		} else {
			diffI = (double)myData->kjg_table_2[i-1][2]
				- (double)myData->kjg_table_2[i][2];
			diffT = (double)myData->kjg_table_2[i][0]
				- (double)myData->kjg_table_2[i-1][0];
			Isens = diffI * diffT;
			diff_Ah = Isens / 3600.0 / 2.0;
			//diff_Ah = 0.0;
			cal_Ah = (double)myData->kjg_table_2[i][2] * diffT / 3600.0;
			sum_Ah = sum_Ah + cal_Ah + diff_Ah;
		}
		fprintf(fp, "%f %f %f %f %f %f\n",
			myData->kjg_table_2[i][0],
			myData->kjg_table_2[i][1],
			myData->kjg_table_2[i][2],
			myData->kjg_table_2[i][3],
			(float)sum_Ah,
			(float)sum_Ah - myData->kjg_table_2[i][3]);
	}
   	fclose(fp);

	return 0;
}
*/

/*kjg_170810 int read_test_cond_step_file_coa(int ch)
{ //kjh_140113
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	char cmd[256];
	int fp, rtn, i, size, checksum;
	S_P1_FILE_TEST_COND test_cond;

	//file check : kjg_w
	
	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "/root/START_INFO/CH%03d/sbc_schedule_info.sch", ch+1);
	// /root/START_INFO/CH00#/sbc_scedule_info.sch

	if((fp = open(cmd, O_RDONLY)) < 0) {
		userlog(DEBUG_LOG, psName, "Can not open %s file(load)\n", cmd);
		return -1;
	}

	rtn = read(fp, (char *)&test_cond, sizeof(S_P1_FILE_TEST_COND));
	if(rtn != (int)myData->COA_Client[0].testCond.test_cond_file_size) {
		userlog(DEBUG_LOG, psName, "error %s size sbc:%d, gui:%d\n",
			cmd, rtn, (int)myData->COA_Client[0].testCond.test_cond_file_size);
		rtn = -2;
	} else rtn = 0;
	close(fp);
	if(rtn < 0) return rtn;

	size = (int)myData->COA_Client[0].testCond.test_cond_file_size;
	checksum = (int)myData->COA_Client[0].testCond.test_cond_file_checksum;
	rtn = checksum_file(psName, ch, size, checksum, (char *)&test_cond);
	if(rtn != checksum) {
		userlog(DEBUG_LOG, psName, "error %s checksum sbc:%d, gui:%d\n",
			cmd, rtn, checksum);
		return -3;
	}
	
	if(myData->COA_Client[0].testCond.header.totalStep
		!= test_cond.header.totalStep) {
		userlog(DEBUG_LOG, psName, "error totalStep %d, %d\n",
			(int)myData->COA_Client[0].testCond.header.totalStep,
			(int)test_cond.header.totalStep);
		return -4;
	}

	if(myData->COA_Client[0].testCond.header.totalPatternCount
		!= test_cond.header.totalPatternCount) {
		userlog(DEBUG_LOG, psName, "error totalPatternCount %d, %d\n",
			(int)myData->COA_Client[0].testCond.header.totalPatternCount,
			(int)test_cond.header.totalPatternCount);
		return -5;
	}

	memcpy((char *)&myData->COA_Client[0].testCond.safety,
		(char *)&test_cond.safety, sizeof(S_P1_TEST_COND_SAFETY));

	for(i=0; i < (int)test_cond.header.totalStep; i++) {
		memcpy((char *)&myData->COA_Client[0].testCond.step[i],
			(char *)&test_cond.step[i], sizeof(S_P1_TEST_COND_STEP));
	}

	userlog(DEBUG_LOG, psName,
		"read step_file completed ch:%d size:%d checksum:%d\n",
		ch+1, size, checksum);
#endif
	return 0;
}

int read_test_cond_time_step_file_coa(int ch)
{ //kjh_160418
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
	char cmd[256];
	int fp, rtn, i, size, checksum;
	S_P1_FILE_TEST_COND_TIME test_cond_time;

	//file check : kjg_w
	
	memset(cmd, 0, sizeof cmd);
	sprintf(cmd,
		"/root/START_INFO/CH%03d/sbc_schedule_info_wait_goto.sch", ch+1);
	// /root/START_INFO/CH00#/sbc_scedule_info.sch

	if((fp = open(cmd, O_RDONLY)) < 0) {
		userlog(DEBUG_LOG, psName, "Can not open %s file(load)\n", cmd);
		return -1;
	}

	rtn = read(fp, (char *)&test_cond_time, sizeof(S_P1_FILE_TEST_COND_TIME));
	if(rtn !=
		(int)myData->COA_Client[0].testCond_time.test_cond_time_file_size) {
		userlog(DEBUG_LOG, psName, "error %s size sbc:%d, gui:%d\n", cmd, rtn,
			(int)myData->COA_Client[0].testCond_time.test_cond_time_file_size);
		rtn = -2;
	} else rtn = 0;
	close(fp);
	if(rtn < 0) return rtn;

	size = (int)myData->COA_Client[0].testCond_time.test_cond_time_file_size;
	checksum
		= (int)myData->COA_Client[0].testCond_time.test_cond_time_file_checksum;
	rtn = checksum_file(psName, ch, size, checksum, (char *)&test_cond_time);
	if(rtn != checksum) {
		userlog(DEBUG_LOG, psName, "error %s checksum sbc:%d, gui:%d\n",
			cmd, rtn, checksum);
		return -3;
	}
	
	if(myData->COA_Client[0].testCond.header.totalTimeSchCount
		!= test_cond_time.header.totalTimeSchCount) {
		userlog(DEBUG_LOG, psName, "error totalTimeSchCount %d, %d\n",
			(int)myData->COA_Client[0].testCond.header.totalTimeSchCount,
			(int)test_cond_time.header.totalTimeSchCount);
		return -4;
	}

	for(i=0; i < (int)myData->COA_Client[0].testCond.header.totalStep; i++) {
		memcpy((char *)&myData->COA_Client[0].testCond_time.time_step[i],
			(char *)&test_cond_time.time_step[i],
			sizeof(S_P1_TEST_COND_TIME_STEP));
	}

	userlog(DEBUG_LOG, psName,
		"read time_step_file completed ch:%d size:%d checksum:%d\n",
			ch+1, size, checksum);
#endif
	return 0;
}

int convert_test_cond_coa(int ch)
{
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int rtn, step, idx, i, fault_count;
	long attr_count, type;

	//kjg_170810 memset((char *)&myData->testCond[ch], 0, sizeof(S_TEST_CONDITION));
	memset((char *)myTestCond, 0, sizeof(S_TEST_CONDITION));

	//common_header
	idx = IDX_COM_OBJ_TOTAL_STEP;
	myTestCond->common_object[idx]
		= (long)myData->COA_Client[0].testCond.header.totalStep;

	idx = IDX_COM_OBJ_TOTAL_PATTERN;
	myTestCond->common_object[idx]
		= (long)myData->COA_Client[0].testCond.header.totalPatternCount;

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = 1;
	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		for(i=0; i < 3; i++) {
			if(myData->ChAttribute[ch].chNo_slave[i] != 0) attr_count++;
		}
	}
	myTestCond->common_object[idx] = attr_count;

	//common_safety
	switch(myData->AppControl.config.systemModel) {
		//for motor inverter application
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_DAEWOO_450V_200A_50A_90KW:
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			fault_count = 50; //500ms delay
			break;
		default:
			fault_count = 1;
			break;
	}

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_V;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_V;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_LOWER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_I;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_I;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_LOWER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_V;
	myTestCond->common_object[idx]
		= myData->COA_Client[0].testCond.safety.faultUpperV;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_V;
	myTestCond->common_object[idx]
		= myData->COA_Client[0].testCond.safety.faultLowerV;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_LOWER_V, fault_count,
		COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_COM_OBJ_FAULT_UPPER_I;
	myTestCond->common_object[idx]
		= myData->COA_Client[0].testCond.safety.faultUpperI;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_P;
	myTestCond->common_object[idx]
		= myData->COA_Client[0].testCond.safety.faultUpperP;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_P, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_WATT_HOUR;
	myTestCond->common_object[idx]
		= myData->COA_Client[0].testCond.safety.faultUpper_WattHour;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_WATT_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myTestCond->common_object[idx]
		= myData->COA_Client[0].testCond.safety.faultUpper_AmpareHour;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_UPPER_TEMP;
	myTestCond->common_object[idx]
		= myData->COA_Client[0].testCond.safety.faultUpperTemp;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_UPPER_TEMP, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_LOWER_TEMP;
	myTestCond->common_object[idx]
		= myData->COA_Client[0].testCond.safety.faultLowerTemp;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_LOWER_TEMP, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_COM_OBJ_FAULT_COMP_AUX_V; //kjhw_150730
	myTestCond->common_object[idx]
		= myData->COA_Client[0].testCond.safety.faultCompAuxV;
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_COMP_AUX_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_DELTA_V; //jhkw_160828
	myTestCond->common_object[idx] = 10000; //10.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_DELTA_V, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>= 1s delay

	//common_cycle_branch

	//common_aux_condition
	convert_test_cond_common_aux_coa(ch);

	//common_can_condition
	convert_test_cond_common_can_coa(ch);

	//step_condition
	for(step=0; step < myData->COA_Client[0].testCond.header.totalStep;
		step++) {
		//header
		idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
		myTestCond->local_object[step][idx]
			= (long)myData->COA_Client[0].testCond.step[step].header.stepNo;

		idx = IDX_LOC_OBJ_STEP_NO;
		myTestCond->local_object[step][idx]
			= (long)myData->COA_Client[0].testCond.step[step].header.stepNo;

		idx = IDX_LOC_OBJ_TYPE;
		type = convert_step_type(CONVERT_P1_TO_ORG,
			(long)myData->COA_Client[0].testCond.step[step].header.type);
		myTestCond->local_object[step][idx] = type;

		switch(type) {
			case STEP_IDLE:
			case STEP_END:
				break;
			case STEP_CYCLE:
				convert_test_cond_cycle_coa(ch, step, type, attr_count);
				break;
			case STEP_LOOP:
				convert_test_cond_loop_coa(ch, step, type, attr_count);
				break;
			default:
				rtn = convert_test_cond_step_default_coa(ch, step, type,
					attr_count);
				if(rtn < 0) return rtn;
				break;
		}
	}
#endif
	return 0;
}

int convert_test_cond_time_coa(int ch)
{ //kjh_160418
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__ || defined __COA_VER_100D__
#else //COA_VER_100F~
	int step, idx, idx2;
	unsigned long tmp_day, tmp_time, tmp_hour, tmp_min, tmp_sec;

	memset((char *)&myData->TimeSch.testCond[ch], 0,
		sizeof(S_TIMESCH_TEST_CONDITION));

	//step_condition
	for(step=0; step < myData->COA_Client[0].testCond.header.totalStep; step++){
		idx = IDX_LOC_OBJ_TIMESCH_STEP_NO;
		myData->TimeSch.testCond[ch].local_object[step][idx]
			= (long)myData->COA_Client[0].testCond_time.time_step[step].stepNo;

		if(myData->TimeSch.testCond[ch].local_object[step][idx] != step+1) {
			return -1;
		}

		idx = IDX_LOC_OBJ_TIMESCH_INIT;
		myData->TimeSch.testCond[ch].local_object[step][idx]
			= (long)myData->COA_Client[0].testCond_time.time_step[step]
			.timeInit;

		idx2 = IDX_LOC_OBJ_END_TIMESCH_DAY;
		tmp_day = (unsigned long)myData->COA_Client[0]
			.testCond_time.time_step[step].wait_day;
		myData->TimeSch.testCond[ch].local_object[step][idx2] = tmp_day;

		if(myData->TimeSch.testCond[ch].local_object[step][idx2] < 0) {
			return -2;
		}

		tmp_hour = (unsigned long)myData->COA_Client[0]
			.testCond_time.time_step[step].wait_hour;

		tmp_min = (unsigned long)myData->COA_Client[0]
			.testCond_time.time_step[step].wait_min;

		tmp_sec	= (unsigned long)myData->COA_Client[0]
			.testCond_time.time_step[step].wait_sec;

		tmp_time = tmp_hour * 360000;
		tmp_time += (tmp_min * 6000);
		tmp_time += (tmp_sec * 100);

		idx = IDX_LOC_OBJ_END_TIMESCH_TIME;
		myData->TimeSch.testCond[ch].local_object[step][idx] = tmp_time;

		if(myData->TimeSch.testCond[ch].local_object[step][idx] < 0) {
			return -3;
		}
	}
#endif
	return 0;
}

void convert_test_cond_common_aux_coa(int ch)
{
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int idx, i, compare_type;
	long func_div, data_type, data_value;
	
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_END_UPPER_TEMP, 3, COMP_GREATER_THAN_OR_EQUAL_TO); //>=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_END_LOWER_TEMP, 3, COMP_LESS_THAN_OR_EQUAL_TO); //<=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_END_UPPER_V, 3, COMP_GREATER_THAN_OR_EQUAL_TO); //>=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_END_LOWER_V, 3, COMP_LESS_THAN_OR_EQUAL_TO); //<=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_FAULT_UPPER_TEMP, 3, COMP_GREATER_THAN_OR_EQUAL_TO); //>=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_FAULT_LOWER_TEMP, 3, COMP_LESS_THAN_OR_EQUAL_TO); //<=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_FAULT_UPPER_V, 3, COMP_GREATER_THAN_OR_EQUAL_TO); //>=
	make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_AUX_FAULT_LOWER_V, 3, COMP_LESS_THAN_OR_EQUAL_TO); //<=

	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		func_div = (long)myData->COA_Client[0].testCond.safety.aux_func_div[i];
		if(func_div < AUX_BRANCH_FUNC_DIV_START
			|| func_div > AUX_BRANCH_FUNC_DIV_END) continue;

		data_type
			= (long)myData->COA_Client[0].testCond.safety.aux_data_type[i];
		idx = IDX_COM_OBJ_AUX_VALUE_TYPE_1 + i;
		myTestCond->common_object[idx] = data_type;

		idx = IDX_COM_OBJ_AUX_VALUE_1 + i;
		if(data_type == 2) { //1:signed, 2:float
			data_value = (long)(myData->COA_Client[0].testCond
				.safety.aux_value[i] * 1000.0);
		} else {
			data_value = (long)myData->COA_Client[0].testCond
				.safety.aux_value[i];
		}

		if(func_div == AUX_FUNC_DIV_CV_SELECT) {
			switch(myData->COA_Client[0].testCond.safety.aux_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_COM_OBJ_AUX_CV_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_COM_OBJ_AUX_CV_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_COM_OBJ_AUX_CV_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_COM_OBJ_AUX_CV_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		if(func_div == AUX_FUNC_DIV_TEMP_PAUSE) { //jhkw_160123s
			switch(myData->COA_Client[0].testCond.safety.aux_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_COM_OBJ_AUX_TEMP_P_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_COM_OBJ_AUX_TEMP_P_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_COM_OBJ_AUX_TEMP_P_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_COM_OBJ_AUX_TEMP_P_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		if(func_div == AUX_FUNC_DIV_TEMP_CONTINUE) {
			switch(myData->COA_Client[0].testCond.safety.aux_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_COM_OBJ_AUX_TEMP_C_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_COM_OBJ_AUX_TEMP_C_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_COM_OBJ_AUX_TEMP_C_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_COM_OBJ_AUX_TEMP_C_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		} //jhkw_160123e

		idx = IDX_COM_OBJ_AUX_FUNC_DIV_1 + i;
		myTestCond->common_object[idx] = func_div;

		idx = IDX_COM_OBJ_AUX_VALUE_1 + i;
		myTestCond->common_object[idx] = data_value;

		switch(myData->COA_Client[0].testCond.safety.aux_compare_type[i]) {
			case P1_COMP_LESS_THAN:
				compare_type = COMP_LESS_THAN; //<
				break;
			case P1_COMP_LESS_THAN_OR_EQUAL_TO:
				compare_type = COMP_LESS_THAN_OR_EQUAL_TO; //<=
				break;
			case P1_COMP_EQUAL_TO:
				compare_type = COMP_EQUAL_TO; //=
				break;
			case P1_COMP_GREATER_THAN:
				compare_type = COMP_GREATER_THAN; //>
				break;
			case P1_COMP_GREATER_THAN_OR_EQUAL_TO:
				compare_type = COMP_GREATER_THAN_OR_EQUAL_TO; //>=
				break;
			case P1_COMP_NOT_EQUAL_TO:
				compare_type = COMP_NOT_EQUAL_TO; //!=
				break;
			default:
				compare_type = COMP_NONE;
				break;
		}

		if(compare_type != COMP_NONE) { //jhkw_130319s
			if(func_div == AUX_FUNC_DIV_CONCENT1_ON
				|| func_div == AUX_FUNC_DIV_CONCENT2_ON
				|| func_div == AUX_FUNC_DIV_CONCENT3_ON
				|| func_div == AUX_FUNC_DIV_CONCENT4_ON
				|| func_div == AUX_FUNC_DIV_CONCENT5_ON
				|| func_div == AUX_FUNC_DIV_CONCENT6_ON
				|| func_div == AUX_FUNC_DIV_CONCENT1_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT2_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT3_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT4_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT5_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT6_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT1_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT2_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT3_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT4_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT5_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT6_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT1_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT2_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT3_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT4_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT5_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT6_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_MUX_A_FORCE_ON //kjhw_151021s
				|| func_div == AUX_FUNC_DIV_MUX_A_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_MUX_B_FORCE_ON
				|| func_div == AUX_FUNC_DIV_MUX_B_FORCE_OFF) { //kjhw_151021e
				make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
					C_CD_COM_AUX_FAULT_FUNC_DIV_1 + i, 100, compare_type);
			} else {
				make_aux_chCode_cond((S_TEST_CONDITION *)myTestCond,
					C_CD_COM_AUX_FAULT_FUNC_DIV_1 + i, 1, compare_type);
			}
		} //jhkw_130319e
	}
#endif
}

void convert_test_cond_common_can_coa(int ch)
{
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int idx, i, compare_type;
	long func_div, data_type, data_value;
	
	make_can_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_CAN_END_UPPER, 1, COMP_GREATER_THAN_OR_EQUAL_TO); //>=
	make_can_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_CAN_END_LOWER, 1, COMP_LESS_THAN_OR_EQUAL_TO); //<=
	make_can_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_CAN_FAULT_UPPER, 1, COMP_GREATER_THAN_OR_EQUAL_TO); //>=
	make_can_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_CAN_FAULT_LOWER, 1, COMP_LESS_THAN_OR_EQUAL_TO); //<=
	
	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		func_div = (long)myData->COA_Client[0].testCond.safety.can_func_div[i];
		if(func_div < CAN_RX_BRANCH_FUNC_DIV_START
			|| func_div > CAN_RX_BRANCH_FUNC_DIV_END) continue;

		data_type
			= (long)myData->COA_Client[0].testCond.safety.can_data_type[i];
		idx = IDX_COM_OBJ_CAN_VALUE_TYPE_1 + i;
		myTestCond->common_object[idx] = (long)data_type;

		idx = IDX_COM_OBJ_CAN_VALUE_1 + i;
		if(data_type == 2) { //1:signed, 2:float
			data_value = (long)(myData->COA_Client[0].testCond
				.safety.can_value[i] * 1000.0);
		} else {
			data_value = (long)myData->COA_Client[0].testCond
				.safety.can_value[i];
		}

		if(func_div == CAN_RX_FUNC_DIV_TEMP_PAUSE) { //jhkw_160123s
			switch(myData->COA_Client[0].testCond.safety.can_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_COM_OBJ_CAN_TEMP_P_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_COM_OBJ_CAN_TEMP_P_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_COM_OBJ_CAN_TEMP_P_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_COM_OBJ_CAN_TEMP_P_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		if(func_div == CAN_RX_FUNC_DIV_TEMP_CONTINUE) {
			switch(myData->COA_Client[0].testCond.safety.can_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_COM_OBJ_CAN_TEMP_C_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_COM_OBJ_CAN_TEMP_C_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_COM_OBJ_CAN_TEMP_C_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_COM_OBJ_CAN_TEMP_C_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		} //jhkw_160123e

		if(func_div == CAN_RX_FUNC_DIV_CV_SELECT) {
			switch(myData->COA_Client[0].testCond.safety.can_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_COM_OBJ_CAN_CV_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_COM_OBJ_CAN_CV_LOWER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_COM_OBJ_CAN_CV_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_COM_OBJ_CAN_CV_UPPER_VALUE;
					myTestCond->common_object[idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		idx = IDX_COM_OBJ_CAN_FUNC_DIV_1 + i;
		myTestCond->common_object[idx] = func_div;

		idx = IDX_COM_OBJ_CAN_VALUE_1 + i;
		myTestCond->common_object[idx] = data_value;

		switch(myData->COA_Client[0].testCond.safety.can_compare_type[i]) {
			case P1_COMP_LESS_THAN:
				compare_type = COMP_LESS_THAN; //<
				break;
			case P1_COMP_LESS_THAN_OR_EQUAL_TO:
				compare_type = COMP_LESS_THAN_OR_EQUAL_TO; //<=
				break;
			case P1_COMP_EQUAL_TO:
				compare_type = COMP_EQUAL_TO; //=
				break;
			case P1_COMP_GREATER_THAN:
				compare_type = COMP_GREATER_THAN; //>
				break;
			case P1_COMP_GREATER_THAN_OR_EQUAL_TO:
				compare_type = COMP_GREATER_THAN_OR_EQUAL_TO; //>=
				break;
			case P1_COMP_NOT_EQUAL_TO:
				compare_type = COMP_NOT_EQUAL_TO; //!=
				break;
			default:
				compare_type = COMP_NONE;
				break;
		}

		if(compare_type != COMP_NONE) { //jhkw_130319s
			if(func_div == CAN_RX_FUNC_DIV_CONCENT1_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_MUX_A_FORCE_ON //kjhw_151021s
				|| func_div == CAN_RX_FUNC_DIV_MUX_A_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_MUX_B_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_MUX_B_FORCE_OFF //kjhw_151021e
				|| func_div == CAN_RX_FUNC_DIV_A_ON //jhkw_170119s
				|| func_div == CAN_RX_FUNC_DIV_A_OFF
				|| func_div == CAN_RX_FUNC_DIV_B_ON
				|| func_div == CAN_RX_FUNC_DIV_B_OFF //jhkw_170119e
				) {
				make_can_chCode_cond((S_TEST_CONDITION *)myTestCond,
					C_CD_COM_CAN_FAULT_FUNC_DIV_1 + i, 100, compare_type);
			} else {
				make_can_chCode_cond((S_TEST_CONDITION *)myTestCond,
					C_CD_COM_CAN_FAULT_FUNC_DIV_1 + i, 1, compare_type);
			}
		} //jhkw_130319e
	}
#endif
}

void convert_test_cond_cycle_coa(int ch, int step, long type, long attr_count)
{
#ifdef __COA_VER_100B__
#elif __COA_VER_100B2__ 
	unsigned long tmp, tmp_day, tmp_time;
#elif __COA_VER_100C__
#else //COA_VER_100D~
	int idx, idx2;
#endif


	idx2 = 0;

	idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_COMPARE;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].AmpareHour_Branch_MultiCycleCountId;

	idx = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].AmpareHour_SumAmpareHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_SUM_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_SUM_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR_TYPE;
	myTestCond->local_object[step][idx] = (long)myData->COA_Client[0].testCond
		.step[step].reference[0].rangeI; //0:all, 1:charge, 2:discharge

	idx = IDX_LOC_OBJ_END_SUM_AMPARE_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_CURRENT_LOOP;

	idx = IDX_LOC_OBJ_END_SUM_WATT_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].WattHour_SumWattHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_SUM_WATT_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_SUM_WATT_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_SUM_WATT_HOUR_TYPE;
	myTestCond->local_object[step][idx] = (long)myData->COA_Client[0].testCond
		.step[step].reference[0].ValueRate_Compare; //0:all,1:charge,2:discharge

	idx = IDX_LOC_OBJ_END_SUM_WATT_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_CURRENT_LOOP;

#ifdef __COA_VER_100B__
	idx = IDX_LOC_OBJ_END_SUM_TIME;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].Time;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
#elif __COA_VER_100B2__
	tmp = myData->COA_Client[0].testCond.step[step].reference[0].Time;
	tmp_day = tmp / (360000 * 24);
	tmp_time = tmp % (360000 * 24);
	idx2 = IDX_LOC_OBJ_END_SUM_TIME_DAY;
	myTestCond->local_object[step][idx2] = tmp_day;
	idx = IDX_LOC_OBJ_END_SUM_TIME;
	myTestCond->local_object[step][idx] = tmp_time;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_SUM_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#else //COA_VER_100C~
	idx2 = IDX_LOC_OBJ_END_SUM_TIME_DAY;
	myTestCond->local_object[step][idx2] = myData->COA_Client[0].testCond
		.step[step].reference[0].endTime_day;
	idx = IDX_LOC_OBJ_END_SUM_TIME;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].endTime;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_SUM_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#endif

	idx = IDX_LOC_OBJ_END_SUM_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_CURRENT_LOOP;
#endif
}

void convert_test_cond_loop_coa(int ch, int step, long type, long attr_count)
{
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int idx;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].V_Lower_Branch_CycleCount;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
	myTestCond->local_object[step][idx] = (long)myData->COA_Client[0].testCond
		.step[step].reference[0].V_Upper_Branch_CycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].V_Lower_MultiCycleCount;

	idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_BRANCH;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].WattHour_Branch_MultiCycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_MULTI_CYCLE_COUNT_COMPARE;
	myTestCond[ch].local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].AmpareHour_Branch_MultiCycleCountId;

	idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].Time_Branch_AccCycleCount;

	idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT_BRANCH;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].AccCycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_ACC_CYCLE_COUNT_COMPARE;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].CVTime_Branch_AccCycleCountId;

	idx = IDX_LOC_OBJ_CYCLE_PAUSE;
	myTestCond->local_object[step][idx] = (long)myData->COA_Client[0].testCond
		.step[step].header.cycle_pause; //0:none, 1:pause(element_cycle count)
#endif
}

int convert_test_cond_step_default_coa(int ch, int step, long type, long attr_count)
{
#ifdef __COA_VER_100B__
#elif __COA_VER_100B2__
	unsigned long tmp, tmp_day, tmp_time;
#elif __COA_VER_100C__
#else //COA_VER_100D~
	int rtn, idx, idx2, i, j, grade, fault_count, use_flag;
#endif


	idx2 = 0;

	//end_condition
	if(type == STEP_OCV) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_OCV, 1, COMP_NONE, 0);
	}

#ifdef __COA_VER_100B__
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].Time;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
#elif __COA_VER_100B2__
	tmp = myData->COA_Client[0].testCond.step[step].reference[0].Time;
	tmp_day = tmp / (360000 * 24);
	tmp_time = tmp % (360000 * 24);
	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = tmp_day;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = tmp_time;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#else //COA_VER_100C~
	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = myData->COA_Client[0].testCond
		.step[step].reference[0].endTime_day;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].endTime;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#endif

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].Time_Branch_AccCycleCount;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;
	
#ifdef __COA_VER_100B__
	idx = IDX_LOC_OBJ_END_CV_TIME;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].CVTime;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
#elif __COA_VER_100B2__
	tmp = myData->COA_Client[0].testCond.step[step].reference[0].CVTime;
	tmp_day = tmp / (360000 * 24);
	tmp_time = tmp % (360000 * 24);
	idx2 = IDX_LOC_OBJ_END_CV_TIME_DAY;
	myTestCond->local_object[step][idx2] = tmp_day;
	idx = IDX_LOC_OBJ_END_CV_TIME;
	myTestCond->local_object[step][idx] = tmp_time;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_CV_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#else //COA_VER_100C~
	idx2 = IDX_LOC_OBJ_END_CV_TIME_DAY;
	myTestCond->local_object[step][idx2] = myData->COA_Client[0].testCond
		.step[step].reference[0].CVTime_day;
	idx = IDX_LOC_OBJ_END_CV_TIME;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].CVTime;
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_CV_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
#endif

	idx = IDX_LOC_OBJ_END_CV_TIME_BRANCH;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].CVTime_Branch_AccCycleCountId;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_V_UPPER;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].V_Upper;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_V_UPPER, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_V_UPPER_BRANCH;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].V_Upper_Branch_CycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_V_LOWER;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].V_Lower_MultiCycleCount;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_V_LOWER, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_END_V_LOWER_BRANCH;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].V_Lower_Branch_CycleCount;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_FAULT_CAN_CHECK_FLAG; //kjhw_141208
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].fault_can_check_flag;

	idx = IDX_LOC_OBJ_FAULT_CAN_TX_STOP; //kjhw_141208
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].can_tx_stop;

	idx = IDX_LOC_OBJ_CHAMBER_STEP_CHECK_FLAG; //kjhw_150821
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].chamber_step_check_flag;

	idx = IDX_LOC_OBJ_CAN_COMM_STEP_CHECK_FLAG; //kjhw_150821
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].can_comm_step_check_flag;

	convert_test_cond_step_aux_coa(ch, step);

	convert_test_cond_step_can_coa(ch, step);

	//record
	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = (long)myData->COA_Client[0].testCond
		.step[step].record.time; //= 1; //kjg_d_10ms katech
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_T, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_V;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].record.deltaV;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_I;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].record.deltaI;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_TEMP;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].record.deltaT;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_TEMP, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_SAVE_DELTA_P;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].record.deltaP;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_P, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//edlc -> record kjg_101221
	switch(myData->AppControl.config.systemModel) {
		case C_SDI_70V_50A_5A_4KW:
		case C_SDI_70V_50A_5A_4KW_2:
		case C_SDI_70V_50A_5A_7KW:
		case C_SDI_70V_50A_5A_7KW_2:
		case C_SDI_70V_250A_25A_18KW:
			use_flag = 1;
			break;
		default:
			use_flag = 0;
			break;
	}

	if(use_flag == 1) {
		idx = IDX_LOC_OBJ_SAVE_RECORD_T1;
		myTestCond->local_object[step][idx]
			= (long)myData->COA_Client[0].testCond.step[step].edlc.startT_Z;

		idx = IDX_LOC_OBJ_SAVE_RECORD_T2;
		myTestCond->local_object[step][idx]
			= (long)myData->COA_Client[0].testCond.step[step].edlc.endT_Z;

		idx = IDX_LOC_OBJ_SAVE_RECORD_T3;
		myTestCond->local_object[step][idx]
			= (long)myData->COA_Client[0].testCond.step[step].edlc.startT_LC;
	}

	//fault
	switch(myData->AppControl.config.systemModel) {
		//for motor inverter application
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_DAEWOO_450V_200A_50A_90KW:
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			fault_count = 50; //500ms delay
			break;
		default:
			fault_count = 1;
			break;
	}

	idx = IDX_LOC_OBJ_FAULT_UPPER_V;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultUpperV;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_V;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultLowerV;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_LOWER_V, fault_count, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	idx = IDX_LOC_OBJ_FAULT_UPPER_TEMP;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultUpperTemp;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_UPPER_TEMP, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_TEMP;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultLowerTemp;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_LOWER_TEMP, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=

	//step grade
	for(grade=0; grade < MAX_P1_GRADE_ITEM; grade++) {
		i = (int)myData->COA_Client[0].testCond.step[step].grade[grade].item;
		if(i != P1_GRADE_ITEM_IDLE) {
			myTestCond->grade[step].item = (unsigned char)i;

			j = (int)myData->COA_Client[0].testCond.step[step].grade[grade]
				.gradeStepCount;
			if(j > MAX_TEST_GRADE) j = MAX_TEST_GRADE;
			myTestCond->grade[step].totalGrade = (unsigned char)j;

			for(i=0; i < j; i++) {
				myTestCond->grade[step].code[i]
					= (long)myData->COA_Client[0].testCond.step[step]
					.grade[grade].gradeStep[i].gradeCode;

				myTestCond->grade[step].value1[i] = myData->COA_Client[0]
					.testCond.step[step].grade[grade].gradeStep[i].lowerValue;

				myTestCond->grade[step].value2[i] = myData->COA_Client[0]
					.testCond.step[step].grade[grade].gradeStep[i].upperValue;
			}
			break;
		}
	}

	switch(type) {
		case STEP_OCV:
		case STEP_REST:
			//kjhw_121031 Vref x 2
			convert_test_cond_ocv_rest_coa(ch, step, type, attr_count);
			break;
		case STEP_CHARGE:
		case STEP_DISCHARGE:
		case STEP_Z:
			convert_test_cond_charge_discharge_z_coa(ch, step, type,attr_count);
			break;
		case STEP_PATTERN:
			rtn = convert_test_cond_pattern_coa(ch, step, type, attr_count);
			if(rtn < 0) return rtn;
			break;
		case STEP_EXTERNAL_CAN:
			convert_test_cond_external_can_coa(ch, step, type, attr_count);
			break;
		case STEP_USERMAP: //kjhw_140828
			rtn = convert_test_cond_usermap_coa(ch, step, type, attr_count);
			if(rtn < 0) return rtn;
			break;
		default: break;
	}
#endif
	return 0;
}

void convert_test_cond_step_aux_coa(int ch, int step)
{
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int idx, i, compare_type, code, branch;
	long func_div, data_type, data_value;
	
	for(i=0; i < MAX_P1_AUX_FUNCTION; i++) {
		func_div = (long)myData->COA_Client[0].testCond.step[step].reference[0]
			.aux_func_div[i];
		if(func_div < AUX_BRANCH_FUNC_DIV_START
			|| func_div > AUX_BRANCH_FUNC_DIV_END) continue;
		
		data_type = (long)myData->COA_Client[0].testCond.step[step].reference[0]
			.aux_data_type[i];
		idx = IDX_LOC_OBJ_AUX_VALUE_TYPE_1 + i;
		myTestCond->local_object[step][idx] = data_type;
	
		if(data_type == 2) { //1:signed, 2:float
			data_value = (long)(myData->COA_Client[0].testCond.step[step]
				.reference[0].aux_value[i] * 1000.0);
		} else {
			data_value = (long)myData->COA_Client[0].testCond.step[step]
				.reference[0].aux_value[i];
		}
		
		if(func_div == AUX_FUNC_DIV_CV_SELECT) {
			switch(myData->COA_Client[0].testCond.step[step].reference[0]
				.aux_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_LOC_OBJ_AUX_CV_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_LOC_OBJ_AUX_CV_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_LOC_OBJ_AUX_CV_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_LOC_OBJ_AUX_CV_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		if(func_div == AUX_FUNC_DIV_TEMP_PAUSE) { //jhkw_160123s
			switch(myData->COA_Client[0].testCond.step[step].reference[0]
				.aux_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_LOC_OBJ_AUX_TEMP_P_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_LOC_OBJ_AUX_TEMP_P_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_LOC_OBJ_AUX_TEMP_P_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_LOC_OBJ_AUX_TEMP_P_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		if(func_div == AUX_FUNC_DIV_TEMP_CONTINUE) {
			switch(myData->COA_Client[0].testCond.step[step].reference[0]
				.aux_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_LOC_OBJ_AUX_TEMP_C_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_LOC_OBJ_AUX_TEMP_C_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_LOC_OBJ_AUX_TEMP_C_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_LOC_OBJ_AUX_TEMP_C_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		} //jhkw_160123e

		idx = IDX_LOC_OBJ_AUX_FUNC_DIV_1 + i;
		myTestCond->local_object[step][idx] = func_div;

		idx = IDX_LOC_OBJ_AUX_BRANCH_1 + i;
		myTestCond->local_object[step][idx] = (long)myData->COA_Client[0]
			.testCond.step[step].reference[0].aux_branch[i];
		branch = (int)myData->COA_Client[0].testCond.step[step].reference[0]
			.aux_branch[i];
		if(branch == ACTIVE_DIV_PAUSE || branch == ACTIVE_DIV_STOP
			|| branch == ACTIVE_DIV_FAULT) {
			code = C_CD_AUX_FAULT_FUNC_DIV_1 + i;
		} else {
			code = C_CD_AUX_END_FUNC_DIV_1 + i;
		}

		idx = IDX_LOC_OBJ_AUX_VALUE_1 + i;
		myTestCond->local_object[step][idx] = data_value;

		switch(myData->COA_Client[0].testCond.step[step].reference[0]
			.aux_compare_type[i]) {
			case P1_COMP_LESS_THAN:
				compare_type = COMP_LESS_THAN; //<
				break;
			case P1_COMP_LESS_THAN_OR_EQUAL_TO:
				compare_type = COMP_LESS_THAN_OR_EQUAL_TO; //<=
				break;
			case P1_COMP_EQUAL_TO:
				compare_type = COMP_EQUAL_TO; //=
				break;
			case P1_COMP_GREATER_THAN:
				compare_type = COMP_GREATER_THAN; //>
				break;
			case P1_COMP_GREATER_THAN_OR_EQUAL_TO:
				compare_type = COMP_GREATER_THAN_OR_EQUAL_TO; //>=
				break;
			case P1_COMP_NOT_EQUAL_TO:
				compare_type = COMP_NOT_EQUAL_TO; //!=
				break;
			default:
				compare_type = COMP_NONE;
				break;
		}

		if(compare_type != COMP_NONE) { //jhkw_130319s
			if(func_div == AUX_FUNC_DIV_CONCENT1_ON
				|| func_div == AUX_FUNC_DIV_CONCENT2_ON
				|| func_div == AUX_FUNC_DIV_CONCENT3_ON
				|| func_div == AUX_FUNC_DIV_CONCENT4_ON
				|| func_div == AUX_FUNC_DIV_CONCENT5_ON
				|| func_div == AUX_FUNC_DIV_CONCENT6_ON
				|| func_div == AUX_FUNC_DIV_CONCENT1_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT2_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT3_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT4_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT5_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT6_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT1_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT2_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT3_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT4_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT5_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT6_FORCE_ON
				|| func_div == AUX_FUNC_DIV_CONCENT1_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT2_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT3_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT4_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT5_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_CONCENT6_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_MUX_A_FORCE_ON //kjhw_151021s
				|| func_div == AUX_FUNC_DIV_MUX_A_FORCE_OFF
				|| func_div == AUX_FUNC_DIV_MUX_B_FORCE_ON
				|| func_div == AUX_FUNC_DIV_MUX_B_FORCE_OFF) { //kjhw_151021e
				make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
					code, 100, compare_type, idx);
			} else {
				make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
					code, 1, compare_type, idx);
			}
		} //jhkw_130319e
	}
#endif
}

void convert_test_cond_step_can_coa(int ch, int step)
{
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int idx, i, compare_type, code, branch;
	long func_div, data_type, data_value;

	for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
		func_div = (long)myData->COA_Client[0].testCond.step[step].reference[0]
			.can_func_div[i];
		if((func_div >= CAN_RX_BRANCH_FUNC_DIV_START
			&& func_div <= CAN_RX_BRANCH_FUNC_DIV_END)
			|| (func_div >= CAN_RX_FUNC_DIV_START
			&& func_div <= CAN_RX_FUNC_DIV_END)) { //kjhw_170303
		} else continue;
		
		data_type = (long)myData->COA_Client[0].testCond.step[step].reference[0]
			.can_data_type[i];
		idx = IDX_LOC_OBJ_CAN_VALUE_TYPE_1 + i;
		myTestCond->local_object[step][idx] = data_type;
		
		if(data_type == 2) { //1:signed, 2:float
			data_value = (long)(myData->COA_Client[0].testCond.step[step]
				.reference[0].can_value[i] * 1000.0);
		} else {
			data_value = (long)myData->COA_Client[0].testCond.step[step]
				.reference[0].can_value[i];
		}

		if(func_div == CAN_RX_FUNC_DIV_TEMP_PAUSE) { //jhkw_160123s
			switch(myData->COA_Client[0].testCond.step[step].reference[0]
				.can_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_LOC_OBJ_CAN_TEMP_P_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_LOC_OBJ_CAN_TEMP_P_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_LOC_OBJ_CAN_TEMP_P_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_LOC_OBJ_CAN_TEMP_P_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}

		if(func_div == CAN_RX_FUNC_DIV_TEMP_CONTINUE) {
			switch(myData->COA_Client[0].testCond.step[step].reference[0]
				.can_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_LOC_OBJ_CAN_TEMP_C_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_LOC_OBJ_CAN_TEMP_C_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_LOC_OBJ_CAN_TEMP_C_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_LOC_OBJ_CAN_TEMP_C_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		} //jhkw_160123e

		if(func_div == CAN_RX_FUNC_DIV_CV_SELECT) {
			switch(myData->COA_Client[0].testCond.step[step].reference[0]
				.can_compare_type[i]) {
				case P1_COMP_LESS_THAN: //<
					idx = IDX_LOC_OBJ_CAN_CV_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_LESS_THAN_OR_EQUAL_TO: //<=
					idx = IDX_LOC_OBJ_CAN_CV_LOWER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_EQUAL_TO: //=
					break;
				case P1_COMP_GREATER_THAN: //>
					idx = IDX_LOC_OBJ_CAN_CV_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_GREATER_THAN_OR_EQUAL_TO: //>=
					idx = IDX_LOC_OBJ_CAN_CV_UPPER_VALUE;
					myTestCond->local_object[step][idx] = data_value;
					break;
				case P1_COMP_NOT_EQUAL_TO: //!=
					break;
				default:
					break;
			}
			continue;
		}
		
		idx = IDX_LOC_OBJ_CAN_FUNC_DIV_1 + i;
		myTestCond->local_object[step][idx] = func_div;
				
		idx = IDX_LOC_OBJ_CAN_BRANCH_1 + i;
		myTestCond->local_object[step][idx] = (long)myData->COA_Client[0]
			.testCond.step[step].reference[0].can_branch[i];
		branch = (int)myData->COA_Client[0].testCond.step[step].reference[0]
			.can_branch[i];
		if(branch == ACTIVE_DIV_PAUSE || branch == ACTIVE_DIV_STOP
			|| branch == ACTIVE_DIV_FAULT) {
			code = C_CD_CAN_FAULT_FUNC_DIV_1 + i;
		} else {
			code = C_CD_CAN_END_FUNC_DIV_1 + i;
		}

		idx = IDX_LOC_OBJ_CAN_VALUE_1 + i;
		myTestCond->local_object[step][idx] = data_value;
		
		switch(myData->COA_Client[0].testCond.step[step].reference[0]
			.can_compare_type[i]) {
			case P1_COMP_LESS_THAN:
				compare_type = COMP_LESS_THAN; //<
				break;
			case P1_COMP_LESS_THAN_OR_EQUAL_TO:
				compare_type = COMP_LESS_THAN_OR_EQUAL_TO; //<=
				break;
			case P1_COMP_EQUAL_TO:
				compare_type = COMP_EQUAL_TO; //=
				break;
			case P1_COMP_GREATER_THAN:
				compare_type = COMP_GREATER_THAN; //>
				break;
			case P1_COMP_GREATER_THAN_OR_EQUAL_TO:
				compare_type = COMP_GREATER_THAN_OR_EQUAL_TO; //>=
				break;
			case P1_COMP_NOT_EQUAL_TO:
				compare_type = COMP_NOT_EQUAL_TO; //!=
				break;
			default:
				compare_type = COMP_NONE;
				break;
		}

		if(compare_type != COMP_NONE) { //jhkw_130319s
			if(func_div == CAN_RX_FUNC_DIV_CHARGING_POWER
				|| func_div == CAN_RX_FUNC_DIV_AVAILABLE_POWER
				|| func_div == CAN_RX_FUNC_DIV_GENERATION_CURRENT //kjhw_150914
				|| func_div == CAN_RX_FUNC_DIV_AVAILABLE_CURRENT //kjhw_150914
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_CONCENT1_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT2_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT3_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT4_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT5_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_CONCENT6_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_MUX_A_FORCE_ON //kjhw_151021s
				|| func_div == CAN_RX_FUNC_DIV_MUX_A_FORCE_OFF
				|| func_div == CAN_RX_FUNC_DIV_MUX_B_FORCE_ON
				|| func_div == CAN_RX_FUNC_DIV_MUX_B_FORCE_OFF //kjhw_151021e
				|| func_div == CAN_RX_FUNC_DIV_A_ON //jhkw_170119s
				|| func_div == CAN_RX_FUNC_DIV_A_OFF
				|| func_div == CAN_RX_FUNC_DIV_B_ON
				|| func_div == CAN_RX_FUNC_DIV_B_OFF //jhkw_170119e
				) {
				make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
					code, 100, compare_type, idx);
			} else {
				make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
					code, 1, compare_type, idx);
			}
		} //jhkw_130319e
	}
#endif
}

void convert_test_cond_ocv_rest_coa(int ch, int step, long type, long attr_count)
{ //kjhw_120517 Vref x 2
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int idx;
	long l_val;

	idx = IDX_COM_OBJ_FAULT_UPPER_V;
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = l_val;
#endif
}

void convert_test_cond_charge_discharge_z_coa(int ch, int step, long type, long attr_count)
{
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int idx, idx2, fault_count, i, func_div;
	long mode, refV1, refV2, refI, l_val, l_val2, tmp, refP, refR;
	double d_val1, d_val2;

	refP = refR = 0;

	//header
	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P1_TO_ORG,
		(long)myData->COA_Client[0].testCond.step[step].header.mode);
	myTestCond->local_object[step][idx] = mode;

	//reference
	idx = IDX_LOC_OBJ_REF_V; //charge ref_v
	refV1 = myData->COA_Client[0].testCond.step[step].reference[0].refV_upper;
	if(refV1 > myData->mData.config.maxV[0]) {
		refV1 = myData->mData.config.maxV[0];
	} else if(refV1 < myData->mData.config.minV[0]) {
		refV1 = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV1;

	idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = l_val; //kjhw_120517e

	idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
	refV2 = myData->COA_Client[0].testCond.step[step].reference[0].refV_lower;
	if(refV2 > myData->mData.config.maxV[0]) {
		refV2 = myData->mData.config.maxV[0];
	} else if(refV2 < myData->mData.config.minV[0]) {
		refV2 = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV2;

	idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V2;
	myTestCond->local_object[step][idx] = l_val; //kjhw_120517e

	if(type == STEP_CHARGE) {
		if(mode == MODE_CP) {
			refP = myData->COA_Client[0].testCond.step[step].reference[0].refP;
			if(refP < 0) refP = 0;
			refI = myData->mData.config.maxI[0] * attr_count;
			d_val1 = (double)(refV1 / 1000) * (double)(refI / 1000) / 1000.0;
			if(refP > (long)d_val1) refP = (long)d_val1;
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refP;
		} else if(mode == MODE_CP_CC) {
			refP = myData->COA_Client[0].testCond.step[step].reference[0].refP;
			if(refP < 0) refP = 0;
			refI = myData->COA_Client[0].testCond.step[step].reference[0].refI;
			if(refI > (myData->mData.config.minI[0] * (-1) * attr_count)) {
				refI = myData->mData.config.minI[0] * (-1) * attr_count;
			} else if(refI < 0) refI = 0;
			d_val1 = (double)(refV1 / 1000) * (double)(refI / 1000) / 1000.0;
			if(refP <= (long)d_val1) d_val1 = (long)refP;
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = (long)d_val1;
			d_val2 = d_val1 / (double)refV1;
			d_val2 *= 1000000000.0;
			if(d_val2 > (double)refI) d_val2 = (double)refI;
			refI = (long)d_val2;
		} else if(mode == MODE_CR) {
			refR = myData->COA_Client[0].testCond.step[step].reference[0].refR;
			if(refR < 1) refR = 1; //0.001ohm
			else if(refR > 1000000000) refR = 1000000000; //1Mohm
			idx = IDX_LOC_OBJ_REF_R;
			myTestCond->local_object[step][idx] = refR;
			d_val1 = (double)refV1 / (double)refR * 1000.0;
			d_val2 = (double)(myData->mData.config.maxI[0] * attr_count);
			if(d_val1 > d_val2) d_val1 = d_val2;
			refI = (long)d_val1;
		} else {
			refI = myData->COA_Client[0].testCond.step[step].reference[0].refI;
		}
	} else { //STEP_DISCHARGE, STEP_Z
		if(mode == MODE_CP) {
			refP = myData->COA_Client[0].testCond.step[step].reference[0].refP;
			refI = myData->mData.config.minI[0] * (-1) * attr_count;
			d_val1 = (double)(myData->mData.config.maxV[0] / 1000)
				* (double)(refI / 1000) / 1000.0;
			if(refP > (long)d_val1) refP = (long)d_val1;
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refP;
		} else if(mode == MODE_CP_CC) {
			refP = myData->COA_Client[0].testCond.step[step].reference[0].refP;
			refI = myData->COA_Client[0].testCond.step[step].reference[0].refI;
			if(refI > (myData->mData.config.minI[0] * (-1) * attr_count)) {
				refI = myData->mData.config.minI[0] * (-1) * attr_count;
			} else if(refI < 0) refI = 0;
			d_val1 = (double)(myData->mData.config.maxV[0] / 1000)
				* (double)(refI / 1000) / 1000.0;
			if(refP > (long)d_val1) refP = (long)d_val1;
			idx = IDX_LOC_OBJ_REF_P;
			myTestCond->local_object[step][idx] = refP;
			if(refV2 <= 1000) { //1mV
				d_val2 = (double)refP / 1000.0; //1mV
			} else {
				d_val2 = (double)refP / (double)refV2;
			}
			d_val2 *= 1000000000.0;
			if(d_val2 > (double)refI) d_val2 = (double)refI;
			refI = (long)d_val2;
		} else if(mode == MODE_CR) {
			refR = myData->COA_Client[0].testCond.step[step].reference[0].refR;
			if(refR < 1) refR = 1; //0.001ohm
			else if(refR > 1000000000) refR = 1000000000; //1Mohm
			idx = IDX_LOC_OBJ_REF_R;
			myTestCond->local_object[step][idx] = refR;
			d_val1 = (double)myData->mData.config.maxV[0]
				/ (double)refR * 1000.0;
			d_val2 = (double)(myData->mData.config.minI[0] * (-1) * attr_count);
			if(d_val1 > d_val2) d_val1 = d_val2;
			refI = (long)d_val1;
		} else {
			refI = myData->COA_Client[0].testCond.step[step].reference[0].refI;
			if(refI > (myData->mData.config.minI[0] * (-1) * attr_count)) {
				refI = myData->mData.config.minI[0] * (-1) * attr_count;
			} else if(refI < 0) refI = 0;
		}
		refI *= (-1);
	}
	idx = IDX_LOC_OBJ_REF_I;
	if(refI > (myData->mData.config.maxI[0] * attr_count)) {
		if(myData->mData.config.maxI[0] != 0) {
			refI = myData->mData.config.maxI[0] * attr_count;
		}
	} else if(refI < (myData->mData.config.minI[0] * attr_count)) {
		if(myData->mData.config.minI[0] != 0) {
			refI = myData->mData.config.minI[0] * attr_count;
		}
	}
	myTestCond->local_object[step][idx] = refI;

	idx = IDX_LOC_OBJ_RANGE_I;
	if(refI > 0) {
		if(refI <= (myData->mData.config.maxI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI <= (myData->mData.config.maxI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI <= (myData->mData.config.maxI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}

		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = (long)myData->COA_Client[0].testCond.step[step].reference[0]
			.rangeI;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else if(refI < 0) {
		if(refI >= (myData->mData.config.minI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI >= (myData->mData.config.minI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI >= (myData->mData.config.minI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}

		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = (long)myData->COA_Client[0].testCond.step[step].reference[0]
			.rangeI;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else {
		if(step == 0) {
			l_val = RANGE1 - 1;
		} else {
			l_val = myTestCond->local_object[step-1][idx];
		}
		myTestCond->local_object[step][idx] = l_val;
	} 

	if(mode == MODE_CC) { //kjh_160623
		for(i=0; i < MAX_P1_CAN_FUNCTION; i++) {
			idx = IDX_LOC_OBJ_CAN_FUNC_DIV_1 + i;
			func_div = (int)myTestCond->local_object[step][idx];
			if(func_div == CAN_RX_FUNC_DIV_BMS_LINK_POWER) {
				idx = IDX_LOC_OBJ_RANGE_I;
				myTestCond->local_object[step][idx] = RANGE1 - 1;
			}
		}
	}

	//end_condition
	idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].AmpareHour_SumAmpareHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].AmpareHour_Branch_MultiCycleCountId;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_WATT_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].WattHour_SumWattHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}
			
	idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].WattHour_Branch_MultiCycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	tmp = (long)myData->COA_Client[0].testCond.step[step].reference[0]
		.ValueRate_Item;
	if(tmp == P1_VALUE_RATE_ITEM_AMPARE_HOUR) {
		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Compare;
	} else if(tmp == P1_VALUE_RATE_ITEM_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Compare;
	}

	if(type == STEP_CHARGE) {
		idx = IDX_LOC_OBJ_END_I;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].Current;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_I, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		idx = IDX_LOC_OBJ_END_I;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].Current * (-1);
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	}

	idx = IDX_LOC_OBJ_END_DELTA_V;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].DeltaV;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_DELTA_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_END_P;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].Power;
	if(type == STEP_CHARGE) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_P, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_P, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	//step edlc
	l_val = myData->COA_Client[0].testCond.step[step].edlc.capacitanceV1;
	l_val2 = myData->COA_Client[0].testCond.step[step].edlc.capacitanceV2;
	if(refI >= 0) {
		if(l_val > l_val2) {
			l_val2 = myData->COA_Client[0].testCond.step[step].edlc
				.capacitanceV1;
			l_val = myData->COA_Client[0].testCond.step[step].edlc
				.capacitanceV2;
		}
	} else {
		if(l_val2 > l_val) {
			l_val2 = myData->COA_Client[0].testCond.step[step].edlc
				.capacitanceV1;
			l_val = myData->COA_Client[0].testCond.step[step].edlc
				.capacitanceV2;
		}
	}

	if(l_val < 0 || l_val2 < 0) {
		l_val = 0;
		l_val2 = 0;
	}

	idx = IDX_LOC_OBJ_CAPACITANCE_V1;
	myTestCond->local_object[step][idx] = l_val;

	idx = IDX_LOC_OBJ_CAPACITANCE_V2;
	myTestCond->local_object[step][idx] = l_val2;

	//fault_condition
	switch(myData->AppControl.config.systemModel) {
		//for motor inverter application
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_DAEWOO_450V_200A_50A_90KW:
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			fault_count = 50; //500ms delay
			break;
		default:
			fault_count = 1;
			break;
	}

	switch(type) {
		case STEP_CHARGE:
			idx = IDX_LOC_OBJ_FAULT_HW_UPPER_V;
			myTestCond->local_object[step][idx] = 5000; //5.0%
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				C_CD_FAULT_HW_UPPER_V, fault_count,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

			switch(myData->AppControl.config.systemModel) {
				case C_SDI_70V_50A_5A_4KW:
				case C_SDI_70V_50A_5A_4KW_2:
				case C_SDI_70V_50A_5A_7KW:
				case C_SDI_70V_50A_5A_7KW_2:
				case C_SDI_70V_250A_25A_18KW:
					idx = IDX_LOC_OBJ_FAULT_LOWER_DELTA_V;
					myTestCond->local_object[step][idx] = myData->COA_Client[0]
						.testConfig.fail_charge_lower_dv;
					idx2 = IDX_LOC_OBJ_FAULT_LOWER_DELTA_V_T;
					myTestCond->local_object[step][idx2] = 100; //1sec
					make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
						C_CD_FAULT_LOWER_DELTA_V, 1,
						COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=
					break;
				default:
					break;
			}
			break;
		case STEP_DISCHARGE:
			idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
			myTestCond->local_object[step][idx] = 5000; //5.0%
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				C_CD_FAULT_HW_LOWER_V, fault_count,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			break;
		case STEP_Z:
			idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
			myTestCond->local_object[step][idx] = 5000; //5.0%
			make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
				C_CD_FAULT_HW_LOWER_V, fault_count,
				COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
			break;
		default: break;
	}

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_I;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_HW_LOWER_I; //kjhw_130912
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_LOWER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_UPPER_I;
	if(type == STEP_CHARGE) {
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].faultUpperI;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_FAULT_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].faultUpperI * (-1);
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_FAULT_UPPER_I, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_FAULT_LOWER_I;
	if(type == STEP_CHARGE) {
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].faultLowerI;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_FAULT_LOWER_I, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	} else if(type == STEP_DISCHARGE || type == STEP_Z) {
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].faultLowerI * (-1);
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_FAULT_LOWER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	}

	if(type == STEP_Z) {
		idx = IDX_LOC_OBJ_FAULT_UPPER_Z;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].faultUpperZ;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_FAULT_UPPER_Z, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_FAULT_LOWER_Z;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].faultLowerZ;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_FAULT_LOWER_Z, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultUpper_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultLower_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_LOWER_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
#endif
}

int convert_test_cond_pattern_coa(int ch, int step, long type, long attr_count)
{
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int idx, i, rtn, fault_count, func_div;
	long pattern_index, mode, l_val, refV, refI, tmp, refI2;

	//header
	idx = IDX_LOC_OBJ_PATTERN_INDEX;
	pattern_index = (long)step;
	myTestCond->local_object[step][idx] = pattern_index;

	idx = IDX_LOC_OBJ_PATTERN_FILE_SIZE;
	myTestCond->local_object[step][idx] = (long)myData->COA_Client[0].testCond
		.step[step].pattern_file_size;
	idx = IDX_LOC_OBJ_PATTERN_FILE_CHECKSUM;
	myTestCond->local_object[step][idx] = (long)myData->COA_Client[0].testCond
		.step[step].pattern_file_checksum;

	rtn = read_test_cond_pattern_file_coa(psName, ch, (int)pattern_index);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "ch%d pattern file error %d\n", ch+1, rtn);
		return rtn;
	}

	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P1_TO_ORG,
		(long)myData->COA_Client[0].testCond.step[step].header.mode);
	myTestCond->local_object[step][idx] = mode;

	//reference
	switch(mode) { //kjhw_150210s
		case MODE_CV:
			refV = 0;
			for(i=1; i < MAX_PATTERN_DATA; i++) {
				l_val = myTestCond->pattern[i].t_val;
				if(l_val < 0) break;

				l_val = myTestCond->pattern[i].cmd_val[0]; //Vref
				if(l_val < 0) {
					userlog(DEBUG_LOG, psName,
						"ch%d pattern file error2 (V)%d\n",
						ch+1, (int)pattern_index);
					return -1;
				}
				if(l_val > refV) refV = l_val;
			}
			idx = IDX_LOC_OBJ_REF_V;
			if(refV > myData->mData.config.maxV[0]) {
				refV = myData->mData.config.maxV[0];
			} else if(refV < myData->mData.config.minV[0]) {
				refV = myData->mData.config.minV[0];
			}
			myTestCond->local_object[step][idx]
				= myData->mData.config.maxV[0]; //kjhw_150507_tt

			idx = IDX_COM_OBJ_FAULT_UPPER_V;
			l_val = myTestCond->common_object[idx];

			if(l_val == 0) {
				l_val = RANGE1 - 1;
			} else if(l_val <= myData->mData.config.maxV[3]) {
				l_val = RANGE4 - 1;
			} else if(l_val <= myData->mData.config.maxV[2]) {
				l_val = RANGE3 - 1;
			} else if(l_val <= myData->mData.config.maxV[1]) {
				l_val = RANGE2 - 1;
			} else {
				l_val = RANGE1 - 1;
			}

			if((l_val+1) > (long)myData->mData.config.rangeV) {
				l_val = (long)myData->mData.config.rangeV - 1;
			}

			idx = IDX_LOC_OBJ_RANGE_V; //kjhw_150507_tt
			myTestCond->local_object[step][idx] = RANGE1 - 1;

			refV = myData->mData.config.maxV[0];
			for(i=1; i < MAX_PATTERN_DATA; i++) {
				l_val = myTestCond->pattern[i].t_val;
				if(l_val < 0) break;

				l_val = myTestCond->pattern[i].cmd_val[0]; //Vref
				if(l_val < refV) refV = l_val;
			}
			idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
			if(refV > myData->mData.config.maxV[0]) {
				refV = myData->mData.config.maxV[0];
			} else if(refV < myData->mData.config.minV[0]) {
				refV = myData->mData.config.minV[0];
			}
			myTestCond->local_object[step][idx]
				= myData->mData.config.minV[0]; //kjhw_150210_tt

			idx = IDX_COM_OBJ_FAULT_UPPER_V;
			l_val = myTestCond->common_object[idx];

			if(l_val == 0) {
				l_val = RANGE1 - 1;
			} else if(l_val <= myData->mData.config.maxV[3]) {
				l_val = RANGE4 - 1;
			} else if(l_val <= myData->mData.config.maxV[2]) {
				l_val = RANGE3 - 1;
			} else if(l_val <= myData->mData.config.maxV[1]) {
				l_val = RANGE2 - 1;
			} else {
				l_val = RANGE1 - 1;
			}

			if((l_val+1) > (long)myData->mData.config.rangeV) {
				l_val = (long)myData->mData.config.rangeV - 1;
			}
			idx = IDX_LOC_OBJ_RANGE_V2; //kjhw_150507_tt
			myTestCond->local_object[step][idx] = RANGE1 - 1;

			refI = 0;
			for(i=1; i < MAX_PATTERN_DATA; i++) {
				l_val = myTestCond->pattern[i].t_val;
				if(l_val < 0) break;

				l_val = myTestCond->pattern[i].cmd_val[1]; //+Iref
				if(l_val < 0) {
					userlog(DEBUG_LOG, psName,
						"ch%d pattern file error2 (+I)%d\n",
						ch+1, (int)pattern_index);
					return -1;
				}
				if(l_val > refI) refI = l_val;
			}
			idx = IDX_LOC_OBJ_REF_I;
			if(refI > (myData->mData.config.maxI[0] * attr_count)) {
				if(myData->mData.config.maxI[0] != 0) {
					refI = myData->mData.config.maxI[0] * attr_count;
				}
			} else if(refI < 0) refI = 0;
			myTestCond->local_object[step][idx] = refI;

			refI2 = 0;
			for(i=1; i < MAX_PATTERN_DATA; i++) {
				l_val = myTestCond->pattern[i].t_val;
				if(l_val < 0) break;

				l_val = myTestCond->pattern[i].cmd_val[2]; //-Iref
				if(l_val > 0) {
					userlog(DEBUG_LOG, psName,
						"ch%d pattern file error2 (-I)%d\n",
						ch+1, (int)pattern_index);
					return -1;
				}
				if(l_val < refI2) refI2 = l_val;
			}
			idx = IDX_LOC_OBJ_REF_I2;
			if(refI2 < (myData->mData.config.minI[0] * attr_count)) {
				if(myData->mData.config.minI[0] != 0) {
					refI2 = myData->mData.config.minI[0] * attr_count;
				}
			} else if(refI2 > 0) refI2 = 0;
			if(refI < refI2 * (-1)) refI = refI2;
			myTestCond->local_object[step][idx] = refI2;

			if(refI <= (myData->mData.config.maxI[3] * attr_count)) {
				l_val = RANGE4 - 1;
			} else if(refI <= (myData->mData.config.maxI[2] * attr_count)) {
				l_val = RANGE3 - 1;
			} else if(refI <= (myData->mData.config.maxI[1] * attr_count)) {
				l_val = RANGE2 - 1;
			} else {
				l_val = RANGE1 - 1;
			}
			if((l_val+1) > (long)myData->mData.config.rangeI) {
				l_val = (long)myData->mData.config.rangeI - 1;
			}
			idx = IDX_LOC_OBJ_RANGE_I;
			myTestCond->local_object[step][idx] = l_val;
			break;
		default: //MODE_CC, MODE_CP
			idx = IDX_LOC_OBJ_REF_V; //charge ref_v
			refV = myData->COA_Client[0].testCond.step[step].reference[0]
				.refV_upper;
			if(refV > myData->mData.config.maxV[0]) {
				refV = myData->mData.config.maxV[0];
			} else if(refV < myData->mData.config.minV[0]) {
				refV = myData->mData.config.minV[0];
			}
			myTestCond->local_object[step][idx] = refV;

			idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
			l_val = myTestCond->common_object[idx];

			if(l_val == 0) {
				l_val = RANGE1 - 1;
			} else if(l_val <= myData->mData.config.maxV[3]) {
				l_val = RANGE4 - 1;
			} else if(l_val <= myData->mData.config.maxV[2]) {
				l_val = RANGE3 - 1;
			} else if(l_val <= myData->mData.config.maxV[1]) {
				l_val = RANGE2 - 1;
			} else {
				l_val = RANGE1 - 1;
			}

			if((l_val+1) > (long)myData->mData.config.rangeV) {
				l_val = (long)myData->mData.config.rangeV - 1;
			}

			idx = IDX_LOC_OBJ_RANGE_V;
			myTestCond->local_object[step][idx] = l_val; //kjhw_120517e
	
			idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
			refV = myData->COA_Client[0].testCond.step[step].reference[0]
				.refV_lower;
			if(refV > myData->mData.config.maxV[0]) {
				refV = myData->mData.config.maxV[0];
			} else if(refV < myData->mData.config.minV[0]) {
				refV = myData->mData.config.minV[0];
			}
			myTestCond->local_object[step][idx] = refV;

			idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
			l_val = myTestCond->common_object[idx];

			if(l_val == 0) {
				l_val = RANGE1 - 1;
			} else if(l_val <= myData->mData.config.maxV[3]) {
				l_val = RANGE4 - 1;
			} else if(l_val <= myData->mData.config.maxV[2]) {
				l_val = RANGE3 - 1;
			} else if(l_val <= myData->mData.config.maxV[1]) {
				l_val = RANGE2 - 1;
			} else {
				l_val = RANGE1 - 1;
			}

			if((l_val+1) > (long)myData->mData.config.rangeV) {
				l_val = (long)myData->mData.config.rangeV - 1;
			}

			idx = IDX_LOC_OBJ_RANGE_V2;
			myTestCond->local_object[step][idx] = l_val; //kjhw_120517e

			if(mode == MODE_CC) {
				refI = 0;
				for(i=1; i < MAX_PATTERN_DATA; i++) {
					//jhkw_161026s
					if(myTestCond->pattern[0].t_val == 2) {
					} else {
						l_val = myTestCond->pattern[i].t_val;
						if(l_val < 0) break;
					} //jhkw_161026e

					l_val = myTestCond->pattern[i].cmd_val[0];
					if(l_val < 0) l_val *= (-1);

					if(l_val > refI) refI = l_val;
				}
			} else { //MODE_CP
				refI = myData->mData.config.maxI[0] * attr_count;
			}

			idx = IDX_LOC_OBJ_REF_I;
			if(refI > (myData->mData.config.maxI[0] * attr_count)) {
				if(myData->mData.config.maxI[0] != 0) {
					refI = myData->mData.config.maxI[0] * attr_count;
				}
			} else if(refI < (myData->mData.config.minI[0] * attr_count)) {
				if(myData->mData.config.minI[0] != 0) {
					refI = myData->mData.config.minI[0] * attr_count;
				}
			}
			myTestCond->local_object[step][idx] = refI;

			if(refI <= (myData->mData.config.maxI[3] * attr_count)) {
				l_val = RANGE4 - 1;
			} else if(refI <= (myData->mData.config.maxI[2] * attr_count)) {
				l_val = RANGE3 - 1;
			} else if(refI <= (myData->mData.config.maxI[1] * attr_count)) {
				l_val = RANGE2 - 1;
			} else {
				l_val = RANGE1 - 1;
			}
			if((l_val+1) > (long)myData->mData.config.rangeI) {
				l_val = (long)myData->mData.config.rangeI - 1;
			}
			idx = IDX_LOC_OBJ_RANGE_I;
			myTestCond->local_object[step][idx] = l_val;

			for(i=0; i < MAX_P1_CAN_FUNCTION; i++) { //kjh_160623
				idx = IDX_LOC_OBJ_CAN_FUNC_DIV_1 + i;
				func_div = (int)myTestCond->local_object[step][idx];
				if(func_div == CAN_RX_FUNC_DIV_BMS_LINK_POWER) {
					idx = IDX_LOC_OBJ_RANGE_I;
					myTestCond->local_object[step][idx] = RANGE1 - 1;
				}
			}
			break;
	} //kjhw_150210e

	//end_condition
	idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].AmpareHour_SumAmpareHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].AmpareHour_Branch_MultiCycleCountId;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_WATT_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].WattHour_SumWattHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}
			
	idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].WattHour_Branch_MultiCycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	tmp = (long)myData->COA_Client[0].testCond.step[step].reference[0]
		.ValueRate_Item;
	if(tmp == P1_VALUE_RATE_ITEM_AMPARE_HOUR) {
		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Compare;
	} else if(tmp == P1_VALUE_RATE_ITEM_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Compare;
	}

	//fault_condition
	switch(myData->AppControl.config.systemModel) {
		//for motor inverter application
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_DAEWOO_450V_200A_50A_90KW:
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			fault_count = 50; //500ms delay
			break;
		default:
			fault_count = 1;
			break;
	}

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_V;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_LOWER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_I;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_UPPER_I;
	myTestCond->local_object[step][idx]
		= myData->COA_Client[0].testCond.step[step].faultUpperI;

	idx = IDX_LOC_OBJ_FAULT_LOWER_I;
	myTestCond->local_object[step][idx]
		= myData->COA_Client[0].testCond.step[step].faultLowerI;

	idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myTestCond->local_object[step][idx]
		= myData->COA_Client[0].testCond.step[step].faultUpper_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
	myTestCond->local_object[step][idx]
		= myData->COA_Client[0].testCond.step[step].faultLower_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_LOWER_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
#endif
	return 0;
}

void convert_test_cond_external_can_coa(int ch, int step, long type, long attr_count)
{
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int idx;
	long mode, refV1, refV2, refI, l_val, l_val2, refP, refR, tmp;
	double d_val1;

	refP = refR = 0;

	//header
	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P1_TO_ORG,
		(long)myData->COA_Client[0].testCond.step[step].header.mode);
	myTestCond->local_object[step][idx] = mode;

	//reference
	idx = IDX_LOC_OBJ_REF_V; //charge ref_v
	refV1 = myData->COA_Client[0].testCond.step[step].reference[0].refV_upper;
	if(refV1 > myData->mData.config.maxV[0]) {
		refV1 = myData->mData.config.maxV[0];
	} else if(refV1 < myData->mData.config.minV[0]) {
		refV1 = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV1;

	idx = IDX_LOC_OBJ_RANGE_V; //kjg_w
	myTestCond->local_object[step][idx] = RANGE1 - 1;
	
	idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
	refV2 = myData->COA_Client[0].testCond.step[step].reference[0].refV_lower;
	if(refV2 > myData->mData.config.maxV[0]) {
		refV2 = myData->mData.config.maxV[0];
	} else if(refV2 < myData->mData.config.minV[0]) {
		refV2 = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV2;

	idx = IDX_LOC_OBJ_RANGE_V2; //kjg_w
	myTestCond->local_object[step][idx] = RANGE1 - 1;

	refP = myData->COA_Client[0].testCond.step[step].reference[0].refP;
	refI = myData->mData.config.maxI[0] * attr_count;
	d_val1 = (double)(myData->mData.config.maxV[0] / 1000)
		* (double)(refI / 1000) / 1000.0;
	if(refP > (long)d_val1) refP = (long)d_val1;
	idx = IDX_LOC_OBJ_REF_P;
	myTestCond->local_object[step][idx] = refP;

	refI = myData->COA_Client[0].testCond.step[step].reference[0].refI;
	idx = IDX_LOC_OBJ_REF_I;
	if(refI > (myData->mData.config.maxI[0] * attr_count)) {
		if(myData->mData.config.maxI[0] != 0) {
			refI = myData->mData.config.maxI[0] * attr_count;
		}
	} else if(refI < (myData->mData.config.minI[0] * attr_count)) {
		if(myData->mData.config.minI[0] != 0) {
			refI = myData->mData.config.minI[0] * attr_count;
		}
	}
	myTestCond->local_object[step][idx] = refI;

	idx = IDX_LOC_OBJ_RANGE_I;
	if(refI > 0) {
		if(refI <= (myData->mData.config.maxI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI <= (myData->mData.config.maxI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI <= (myData->mData.config.maxI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = (long)myData->COA_Client[0].testCond.step[step].reference[0]
			.rangeI;
		if(l_val2 == 0) { //auto range
		} else { //manual range
			if(l_val2 < l_val) {
				l_val = l_val2;
			}
		}
	} else if(refI < 0) {
		if(refI >= (myData->mData.config.minI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI >= (myData->mData.config.minI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI >= (myData->mData.config.minI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = (long)myData->COA_Client[0].testCond.step[step].reference[0]
			.rangeI;
		if(l_val2 == 0) { //auto range
		} else { //manual range
			if(l_val2 < l_val) {
				l_val = l_val2;
			}
		}
	} else {
		if(step == 0) {
			l_val = RANGE1 - 1;
		} else {
			l_val = myTestCond->local_object[step-1][idx];
		}
	} 
	l_val = RANGE1 - 1;
	myTestCond->local_object[step][idx] = l_val;

	//end_condition
	idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].AmpareHour_SumAmpareHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
	myTestCond->local_object[step][idx]
		= myData->COA_Client[0].testCond.step[step].reference[0]
		.AmpareHour_Branch_MultiCycleCountId;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_WATT_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].WattHour_SumWattHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}
			
	idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].WattHour_Branch_MultiCycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	tmp = (long)myData->COA_Client[0].testCond.step[step].reference[0]
		.ValueRate_Item;
	if(tmp == P1_VALUE_RATE_ITEM_AMPARE_HOUR) {
		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Compare;
	} else if(tmp == P1_VALUE_RATE_ITEM_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Compare;
	}

	idx = IDX_LOC_OBJ_END_DELTA_V;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].DeltaV;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_DELTA_V, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//fault_condition
	idx = IDX_LOC_OBJ_FAULT_UPPER_I;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultUpperI;

	idx = IDX_LOC_OBJ_FAULT_LOWER_I;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultLowerI;

	idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultUpper_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultLower_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_LOWER_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
#endif
}

int convert_test_cond_usermap_coa(int ch, int step, long type, long attr_count)
{ //kjhw_140828
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int master, slave, idx, rtn, fault_count;
	int i, k, func_div, func_div1, func_div2, func_div3;
	long usermap_index, mode, l_val, refV, refI, tmp;

	k = 0;
	if(myData->mData.config.division_CAN == 1) {
		master = myData->canReceiveDataCount[ch][0];
		slave = 0;
	} else {
		master = myData->canReceiveDataCount[ch][0];
		slave = myData->canReceiveDataCount[ch][1];
	}

	for(i=0; i < master + slave; i++) {
		func_div1 = myData->canReceiveSetData.normalData[ch][i].function_div;
		func_div2 = myData->canReceiveSetData.normalData[ch][i].function_div2;
		func_div3 = myData->canReceiveSetData.normalData[ch][i].function_div3;
		func_div = CAN_RX_FUNC_DIV_USERMAP_TEMP; //define 1100
		if(func_div1 == func_div || func_div2 == func_div
			|| func_div3 == func_div) {
			if(k == 1 || k > 3) break;
			k += 1;
		}
		func_div = CAN_RX_FUNC_DIV_USERMAP_SOC; //define 1099
		if(func_div1 == func_div || func_div2 == func_div
			|| func_div3 == func_div) {
			if(k == 2 || k > 3) break;
			k += 2;
		}
	}
	if(k != 3) {
		userlog(DEBUG_LOG, psName, "ch%d usermap can_func set error %d\n",
			ch+1, k);
		return -1;
	}

	//header
	idx = IDX_LOC_OBJ_PATTERN_INDEX;
	usermap_index = (long)step;
	myTestCond->local_object[step][idx] = usermap_index;

	idx = IDX_LOC_OBJ_PATTERN_FILE_SIZE;
	myTestCond->local_object[step][idx] = (long)myData->COA_Client[0].testCond
		.step[step].pattern_file_size;
	idx = IDX_LOC_OBJ_PATTERN_FILE_CHECKSUM;
	myTestCond->local_object[step][idx] = (long)myData->COA_Client[0].testCond
		.step[step].pattern_file_checksum;

	rtn = read_test_cond_usermap_file_coa(psName, ch, (int)usermap_index);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "ch%d usermap file error %d\n", ch+1, rtn);
		return rtn;
	}

	idx = IDX_LOC_OBJ_MODE;
	mode = convert_step_mode(CONVERT_P1_TO_ORG,
		(long)myData->COA_Client[0].testCond.step[step].header.mode);
	myTestCond->local_object[step][idx] = mode;

	//reference
	idx = IDX_LOC_OBJ_REF_V; //charge ref_v
	refV = myData->COA_Client[0].testCond.step[step].reference[0].refV_upper;
	if(refV > myData->mData.config.maxV[0]) {
		refV = myData->mData.config.maxV[0];
	} else if(refV < myData->mData.config.minV[0]) {
		refV = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = l_val; //kjhw_120517e
	
	idx = IDX_LOC_OBJ_REF_V2; //discharge ref_v
	refV = myData->COA_Client[0].testCond.step[step].reference[0].refV_lower;
	if(refV > myData->mData.config.maxV[0]) {
		refV = myData->mData.config.maxV[0];
	} else if(refV < myData->mData.config.minV[0]) {
		refV = myData->mData.config.minV[0];
	}
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_COM_OBJ_FAULT_UPPER_V; //kjhw_120517s
	l_val = myTestCond->common_object[idx];

	if(l_val == 0) {
		l_val = RANGE1 - 1;
	} else if(l_val <= myData->mData.config.maxV[3]) {
		l_val = RANGE4 - 1;
	} else if(l_val <= myData->mData.config.maxV[2]) {
		l_val = RANGE3 - 1;
	} else if(l_val <= myData->mData.config.maxV[1]) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}

	if((l_val+1) > (long)myData->mData.config.rangeV) {
		l_val = (long)myData->mData.config.rangeV - 1;
	}

	idx = IDX_LOC_OBJ_RANGE_V2;
	myTestCond->local_object[step][idx] = l_val; //kjhw_120517e

	refI = myData->mData.config.maxI[0] * attr_count;

	idx = IDX_LOC_OBJ_REF_I;
	if(refI > (myData->mData.config.maxI[0] * attr_count)) {
		if(myData->mData.config.maxI[0] != 0) {
			refI = myData->mData.config.maxI[0] * attr_count;
		}
	} else if(refI < (myData->mData.config.minI[0] * attr_count)) {
		if(myData->mData.config.minI[0] != 0) {
			refI = myData->mData.config.minI[0] * attr_count;
		}
	}
	myTestCond->local_object[step][idx] = refI;

	if(refI <= (myData->mData.config.maxI[3] * attr_count)) {
		l_val = RANGE4 - 1;
	} else if(refI <= (myData->mData.config.maxI[2] * attr_count)) {
		l_val = RANGE3 - 1;
	} else if(refI <= (myData->mData.config.maxI[1] * attr_count)) {
		l_val = RANGE2 - 1;
	} else {
		l_val = RANGE1 - 1;
	}
	if((l_val+1) > (long)myData->mData.config.rangeI) {
		l_val = (long)myData->mData.config.rangeI - 1;
	}
	idx = IDX_LOC_OBJ_RANGE_I;
	myTestCond->local_object[step][idx] = l_val;

	//end_condition
	idx = IDX_LOC_OBJ_END_P; //jhkw_141122
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].Power;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_P, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	} else { //<
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_P, 1, COMP_GREATER_THAN, idx); //>
	}

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].AmpareHour_SumAmpareHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}

	idx = IDX_LOC_OBJ_END_AMPARE_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].AmpareHour_Branch_MultiCycleCountId;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_END_WATT_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].WattHour_SumWattHour;
	if(myTestCond->local_object[step][idx] >= 0) {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=
	} else {
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_WATT_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
	}
			
	idx = IDX_LOC_OBJ_END_WATT_HOUR_BRANCH;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].reference[0].WattHour_Branch_MultiCycleCount_Branch;
	if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
		myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	tmp = (long)myData->COA_Client[0].testCond.step[step].reference[0]
		.ValueRate_Item;
	if(tmp == P1_VALUE_RATE_ITEM_AMPARE_HOUR) {
		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_AMPARE_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_AMPARE_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Compare;
	} else if(tmp == P1_VALUE_RATE_ITEM_WATT_HOUR) {
		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate;
		make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
			C_CD_END_VALUE_RATE_WATT_HOUR, 1,
			COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_BRANCH;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Branch;
		if(myTestCond->local_object[step][idx] == ACTIVE_DIV_NONE)
			myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

		idx = IDX_LOC_OBJ_END_WATT_HOUR_RATE_COMPARE;
		myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
			.step[step].reference[0].ValueRate_Compare;
	}

	//fault_condition
	switch(myData->AppControl.config.systemModel) {
		//for motor inverter application
		case C_DAEWOO_450V_200A_10A_90KW:
		case C_DAEWOO_450V_200A_10A_90KW_2:
		case C_DAEWOO_450V_200A_50A_90KW:
		case C_DAEWOO_450V_200A_50A_90KW_2:	//jhk_150710
			fault_count = 50; //500ms delay
			break;
		default:
			fault_count = 1;
			break;
	}

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_V;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_HW_LOWER_V;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_LOWER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_HW_UPPER_I;
	myTestCond->local_object[step][idx] = 5000; //5.0%
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_HW_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_UPPER_I;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultUpperI;

	idx = IDX_LOC_OBJ_FAULT_LOWER_I;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultLowerI;

	idx = IDX_LOC_OBJ_FAULT_UPPER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultUpper_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_UPPER_AMPARE_HOUR, 1,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_FAULT_LOWER_AMPARE_HOUR;
	myTestCond->local_object[step][idx] = myData->COA_Client[0].testCond
		.step[step].faultLower_AmpareHour;
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_FAULT_LOWER_AMPARE_HOUR, 1, COMP_LESS_THAN_OR_EQUAL_TO, idx); //<=
#endif
	return 0;
}

void convert_test_cond_cable_check_coa(int ch)
{ //kjg_101221_w
#if defined __COA_VER_100B__ || defined __COA_VER_100B2__ || defined __COA_VER_100C__
#else //COA_VER_100D~
	int step, idx, idx2, i, fault_count;
	long attr_count, refV, refI, l_val, l_val2;

	//kjg_170810 memset((char *)&myData->testCond[ch], 0, sizeof(S_TEST_CONDITION));
	memset((char *)myTestCond, 0, sizeof(S_TEST_CONDITION));

	//cycle -> charge 1sec -> rest 1sec -> discharge 1sec -> rest 1sec -> loop -> end
	//common_header
	idx = IDX_COM_OBJ_TOTAL_STEP;
	myTestCond->common_object[idx] = 7;

	idx = IDX_COM_OBJ_ATTRIBUTE_COUNT;
	attr_count = 1;
	if(myData->ChAttribute[ch].opType != OP_INDEPENDENT) {
		for(i=0; i < 3; i++) {
			if(myData->ChAttribute[ch].chNo_slave[i] != 0) attr_count++;
		}
	}
	myTestCond->common_object[idx] = attr_count;

	//common_safety
	fault_count = 1;

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_V;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_UPPER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_V;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_LOWER_V, fault_count,
		COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_UPPER_I;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_UPPER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_COM_OBJ_FAULT_HW_LOWER_I;
	myTestCond->common_object[idx] = 5000; //5.0%
	make_com_chCode_cond((S_TEST_CONDITION *)myTestCond,
		C_CD_COM_FAULT_HW_LOWER_I, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//common_cycle_branch

	//step_condition
	//step_cycle
	step = 0;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_CYCLE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_START;

	//step_charge
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_CHARGE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_1;

	idx = IDX_LOC_OBJ_MODE;
	myTestCond->local_object[step][idx] = MODE_CC_CV;

	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = 0;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 10; //0.1sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_T, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_REF_V;
	refV = 4200000; //4.2V
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_LOC_OBJ_REF_V2;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = RANGE1 - 1;

	idx = IDX_LOC_OBJ_REF_I;
	refI = 20000000; //20A
	myTestCond->local_object[step][idx] = refI;

	idx = IDX_LOC_OBJ_RANGE_I;
	if(refI > 0) {
		if(refI <= (myData->mData.config.maxI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI <= (myData->mData.config.maxI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI <= (myData->mData.config.maxI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else if(refI < 0) {
		if(refI >= (myData->mData.config.minI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI >= (myData->mData.config.minI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI >= (myData->mData.config.minI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else {
		if(step == 0) {
			l_val = RANGE1 - 1;
		} else {
			l_val = myTestCond->local_object[step-1][idx];
		}
		myTestCond->local_object[step][idx] = l_val;
	}

	//step_rest
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_REST;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_2;

	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = 0;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 100; //1.0sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_T, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//step_discharge
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_DISCHARGE;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_3;

	idx = IDX_LOC_OBJ_MODE;
	myTestCond->local_object[step][idx] = MODE_CC_CV;

	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = 0;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 10; //0.1sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_T, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	idx = IDX_LOC_OBJ_REF_V;
	refV = 4200000; //4.2V
	myTestCond->local_object[step][idx] = refV;

	idx = IDX_LOC_OBJ_REF_V2;
	myTestCond->local_object[step][idx] = 0;

	idx = IDX_LOC_OBJ_RANGE_V;
	myTestCond->local_object[step][idx] = RANGE1 - 1;

	idx = IDX_LOC_OBJ_REF_I;
	refI = -20000000; //-20A
	myTestCond->local_object[step][idx] = refI;

	idx = IDX_LOC_OBJ_RANGE_I;
	if(refI > 0) {
		if(refI <= (myData->mData.config.maxI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI <= (myData->mData.config.maxI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI <= (myData->mData.config.maxI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else if(refI < 0) {
		if(refI >= (myData->mData.config.minI[3] * attr_count)) {
			l_val = RANGE4 - 1;
		} else if(refI >= (myData->mData.config.minI[2] * attr_count)) {
			l_val = RANGE3 - 1;
		} else if(refI >= (myData->mData.config.minI[1] * attr_count)) {
			l_val = RANGE2 - 1;
		} else {
			l_val = RANGE1 - 1;
		}
		if((l_val+1) > (long)myData->mData.config.rangeI) {
			l_val = (long)myData->mData.config.rangeI - 1;
		}

		l_val2 = 0;
		if(l_val2 == 0) { //auto range
			myTestCond->local_object[step][idx] = l_val;
		} else { //manual range
			if(l_val2 < l_val) {
				myTestCond->local_object[step][idx] = l_val2;
			} else {
				myTestCond->local_object[step][idx] = l_val;
			}
		}
	} else {
		if(step == 0) {
			l_val = RANGE1 - 1;
		} else {
			l_val = myTestCond->local_object[step-1][idx];
		}
		myTestCond->local_object[step][idx] = l_val;
	}

	//step_rest
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_REST;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_4;

	idx2 = IDX_LOC_OBJ_END_TIME_DAY;
	myTestCond->local_object[step][idx2] = 0;
	idx = IDX_LOC_OBJ_END_TIME;
	myTestCond->local_object[step][idx] = 100; //1sec
	make_loc_chCode_cond2((S_TEST_CONDITION *)myTestCond, step,
		C_CD_END_TIME, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx, idx2); //>=

	idx = IDX_LOC_OBJ_END_TIME_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	idx = IDX_LOC_OBJ_SAVE_DELTA_T;
	myTestCond->local_object[step][idx] = 100; //1.0sec
	make_loc_chCode_cond((S_TEST_CONDITION *)myTestCond, step,
		C_CD_SAVE_DELTA_T, 1, COMP_GREATER_THAN_OR_EQUAL_TO, idx); //>=

	//step_loop
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_LOOP;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_5;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT;
	myTestCond->local_object[step][idx] = 1;

	idx = IDX_LOC_OBJ_END_CYCLE_COUNT_BRANCH;
	myTestCond->local_object[step][idx] = ACTIVE_DIV_NEXT;

	//step_end
	step++;

	idx = IDX_LOC_OBJ_CLIENT_STEP_NO;
	myTestCond->local_object[step][idx] = step+1;

	idx = IDX_LOC_OBJ_STEP_NO;
	myTestCond->local_object[step][idx] = step;

	idx = IDX_LOC_OBJ_TYPE;
	myTestCond->local_object[step][idx] = STEP_END;

	idx = IDX_LOC_OBJ_ATTRIBUTE;
	myTestCond->local_object[step][idx] = ATTR_CHECK4_END;
#endif
}*/

