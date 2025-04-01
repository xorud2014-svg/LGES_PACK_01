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
	{"I_FAN_FAIL", I_FAN_FAIL},

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
	{"I_JIG_SUB_CYLINDER_L_ON", I_JIG_SUB_CYLINDER_L_ON},		//csk_161017_s
	{"I_JIG_SUB_CYLINDER_L_OFF", I_JIG_SUB_CYLINDER_L_OFF},
	{"I_JIG_SUB_CYLINDER_R_ON", I_JIG_SUB_CYLINDER_R_ON},
	{"I_JIG_SUB_CYLINDER_R_OFF", I_JIG_SUB_CYLINDER_R_OFF},
	{"I_JIG_FIXTURE_L_ON", I_JIG_FIXTURE_L_ON},
	{"I_JIG_FIXTURE_L_OFF", I_JIG_FIXTURE_L_OFF},
	{"I_JIG_FIXTURE_R_ON", I_JIG_FIXTURE_R_ON},
	{"I_JIG_FIXTURE_R_OFF", I_JIG_FIXTURE_R_OFF},				//csk_161017_e
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
	{"I_JIG_CHAMBER_MANUAL_FIRE", I_JIG_CHAMBER_MANUAL_FIRE},	//csk_161115
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
	{"I_FRAME_DOOR_OPEN", I_FRAME_DOOR_OPEN},	//csk_160408

	{"I_OUT_CONCENT1", I_OUT_CONCENT1},	//jhkw_130319s
	{"I_OUT_CONCENT2", I_OUT_CONCENT2},
	{"I_OUT_CONCENT3", I_OUT_CONCENT3},
	{"I_OUT_CONCENT4", I_OUT_CONCENT4},	//jhkw_130319e
	{"I_OUT_CONCENT5", I_OUT_CONCENT5},
	{"I_OUT_CONCENT6", I_OUT_CONCENT6},
	{"I_OUT_CONCENT7", I_OUT_CONCENT7}, //jhkw_180206
	{"I_OUT_CONCENT8", I_OUT_CONCENT8},	//jhkw_180206

	{"I_MUX_A_P", I_MUX_A_P},  //kjhw_151021s
	{"I_MUX_A_N", I_MUX_A_N},
	{"I_MUX_B_P", I_MUX_B_P},
	{"I_MUX_B_N", I_MUX_B_N},	//kjhw_151021e

	{"I_TEST_01", I_TEST_01},	//kjhw_150401
	{"I_TEST_02", I_TEST_02},	//kjhw_150401
	{"I_TEST_03", I_TEST_03},	//kjhw_150401
	{"I_TEST_04", I_TEST_04},	//kjhw_150401

	{"I_CAP_BANK", I_CAP_BANK},	//kjh_191014
	{"I_DC_LINK_FUSE_FAIL", I_DC_LINK_FUSE_FAIL},	//phb_230710
	{"I_BREAKDOWN_VOLTAGE", I_BREAKDOWN_VOLTAGE}	//ktg_240930
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
	{"O_SELECT_V_CALI_PI2", O_SELECT_V_CALI_PI2}, //kjh_191029
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
	{"O_JIG_SUB_CYLINDER_ON", O_JIG_SUB_CYLINDER_ON},			//csk_161017_s
	{"O_JIG_SUB_CYLINDER_OFF", O_JIG_SUB_CYLINDER_OFF},
	{"O_JIG_SUB_CYLINDER_ON_OFF", O_JIG_SUB_CYLINDER_ON_OFF},
	{"O_JIG_FIXTURE_CYLINDER_ON", O_JIG_FIXTURE_CYLINDER_ON},
	{"O_JIG_FIXTURE_CYLINDER_OFF", O_JIG_FIXTURE_CYLINDER_OFF},	//csk_161017_e
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
	{"O_CHAMBER_CH_WARNING", O_CHAMBER_CH_WARNING},		//csk_190620

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
	{"O_TEST_04", O_TEST_04},  //kjhw_150401
	{"O_CAN_OR_LINTOCAN", O_CAN_OR_LINTOCAN},   //kjhw_181111
	{"O_CAN_SW", O_CAN_SW},						//csk_190718s
	{"O_LIN_TO_CAN_SW", O_LIN_TO_CAN_SW},
	{"O_485_TO_CAN_SW", O_485_TO_CAN_SW},		//csk_190718e
	{"O_SMBUS_TO_CAN_SW", O_SMBUS_TO_CAN_SW},	//csk_190910
	{"O_MODBUS_TO_CAN_SW", O_MODBUS_TO_CAN_SW},	//ktg_211001
	{"O_CAN_SLAVE_SW", O_CAN_SLAVE_SW},			//ktg_211001
	{"O_CAP_BANK", O_CAP_BANK},					//kjh_191014
	{"O_BREAKDOWN_VOLTAGE", O_BREAKDOWN_VOLTAGE}	//ktg_240930
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
#ifdef __CAN_FD__ //kjh_190706s
					SendMsg.val[4] = (int)myData->canReceiveSetData
						.commonData[rtn][0].can_fd_flag;
					SendMsg.val[5] = (int)myData->canReceiveSetData
						.commonData[rtn][0].can_datarate;
					SendMsg.val[6] = (int)myData->canReceiveSetData
						.commonData[rtn][0].crc_type;
					SendMsg.val[7] = (int)myData->canReceiveSetData
						.commonData[rtn][0].terminal_r;
#endif //kjh_190706e
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
#ifdef __CAN_FD__ //kjh_190706s
					SendMsg.val[4] = (int)myData->canReceiveSetData
						.commonData[rtn/2][rtn%2].can_fd_flag;
					SendMsg.val[5] = (int)myData->canReceiveSetData
						.commonData[rtn/2][rtn%2].can_datarate;
					SendMsg.val[6] = (int)myData->canReceiveSetData
						.commonData[rtn/2][rtn%2].crc_type;
					SendMsg.val[7] = (int)myData->canReceiveSetData
						.commonData[rtn/2][rtn%2].terminal_r;
#endif //kjh_190706e
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
	userlog(DEBUG_LOG, psName, "System start\n");		//csk_190830_c PACK_MERGE

	userlog(DEBUG_LOG, psName, "-----------System Parameter Info(Module)-----------\n");
	userlog(DEBUG_LOG, psName, "versionNo		: %s\n", myData->AppControl.config.versionNo);
	userlog(DEBUG_LOG, psName, "update_date		: %s\n", myData->AppControl.config.update_date);

#ifdef __A_TYPE__
	userlog(DEBUG_LOG, psName, "H/W Control Type	: A_TYPE\n");
#elif __B_TYPE__
	userlog(DEBUG_LOG, psName, "H/W Control Type	: B_TYPE\n");
#else
	userlog(DEBUG_LOG, psName, "H/W Control Type	: Non Control Type\n");
#endif

#ifdef __CAN_FD__
	userlog(DEBUG_LOG, psName, "CAN Type		: CAN_FD\n");
#elif __CAN_2P0B__
	userlog(DEBUG_LOG, psName, "CAN Type		: CAN_2.0\n");
#else
	userlog(DEBUG_LOG, psName, "CAN Type		: Non CAN Type\n");
#endif
	
#ifdef __SBC_EM104_A5362__
	userlog(DEBUG_LOG, psName, "SBC Model		: EM104_A5362\n");
#elif __SBC_EMCORE_V621__
	userlog(DEBUG_LOG, psName, "SBC Model		: EMCORE_V621\n");
#else
	userlog(DEBUG_LOG, psName, "SBC Model		: Non SBC Model\n");
#endif

	switch(myData->mData.config.ratioV) { //kjh_211021s
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
	//kjh_211021e

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
	unsigned char tmp;
	int i, j;

	//message
	memset((char *)&myData->msg, 0, sizeof(S_MSG) * MAX_MSG_RING);

#if defined __COC__
	//fch
	memset((char *)&myData->fch, 0, sizeof(S_FCH_DATA));
#endif

	//debug
	memset((char *)&myData->test_val_uc[0][0], 0,
		sizeof(unsigned char) * 8 * MAX_TEST_VALUE);
	//memset((char *)&myData->test_val_l[0], 0, sizeof(long) * MAX_TEST_VALUE);
	memset((char *)&myData->test_val_l1[0], 0, sizeof(long) * MAX_TEST_VALUE);	//csk_240301
	//memset((char *)&myData->test_val_ll[0], 0,
	memset((char *)&myData->test_val_ll1[0], 0,
		sizeof(long long) * MAX_TEST_VALUE);									//csk_240301
	memset((char *)&myData->test_val_f[0], 0, sizeof(float) * MAX_TEST_VALUE);

	//dio
	memset((char *)&myData->dio.misc, 0, sizeof(S_DIO_MISC));
	memset((char *)&myData->dio.in, 0, sizeof(S_DIO_INPUT));
	memset((char *)&myData->dio.out, 0, sizeof(S_DIO_OUTPUT));
	memset((char *)&myData->dio.signal, 0, sizeof(unsigned char) * MAX_DIO_SIGNAL); //shh_231016
	//memset((char *)&myData->dio.signal, 0, sizeof(unsigned char) * MAX_SIGNAL);

	//mControl
	memset((char *)&myData->mData.misc, 0, sizeof(S_MODULE_MISC));
	myData->mData.misc.main_slot = -1;
	myData->mData.misc.increment_period = (unsigned long)myData->mData
		.config.scan_period / 10; //kjg_w 25ms / 10 = 2
	myData->mData.misc.fan_run_time = 300; //6min
	myData->mData.misc.fan_stop_time = 1800; //30min
	myData->mData.misc.test_internal_r = 1;

	if((myData->mData.config.ratioV == MICRO_UNIT)
		&& (myData->mData.config.ratioI == MICRO_UNIT)
		&& (myData->mData.config.ratioP == MILLI_UNIT)) {	//kjh_220321s
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
	}	//kjh_220321e

	switch(myData->AppControl.config.systemModel) {
		case C_SDI_80V_50A_25A_10A:
		case C_SDI_80V_50A_25A_10A_2:
		case C_SDI_80V_50A_25A_10A_3:
		case C_SDI_80V_50A_25A_10A_4:
		case C_LGC_60V_30A_15A_3A_7KW:	//shh_231024
			myData->mData.misc.module_type = MODULE_LINEAR;
			break;
		case C_SDI_5V_60A_10A_1A:
		case C_SBL_5V_60A_10A_1A:				//csk_121205
		case C_SBL_5V_200A_120A_60A_10A:		//csk_120820
		case C_SDI_5V_250A_25A_5A:
		case C_SBL_5V_250A_25A_5A:
		case C_SBL_5V_250A_60A_10A:
		case C_SDI_5V_300A_60A_10A:
		case C_SBL_5V_300A_240A_60A_10A:		//csk_120820
		case C_SBL_5V_300A_240A_60A_10A_2:		//kjg_120826
		case C_SBL_5V_300A_240A_60A_10A_3:		//kjg_120919
		case C_SBL_5V_300A_240A_60A_10A_4:		//csk_121205
		case C_SBL_5V_400A_240A_60A_10A:		//csk_120820
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
			break;
		default:
			myData->mData.misc.module_type = MODULE_PWM1;
			break;
	}

	myData->mData.code = M_CD_NONE;
	tmp = myData->mData.signal[M_SIG_RUNNING_GROUP];
	memset((char *)&myData->mData.signal, 0,
		sizeof(unsigned char) * MAX_SIGNAL);
	myData->mData.signal[M_SIG_RUNNING_GROUP] = tmp;

	for(i=0; i < 6; i++) {
		for(j=0; j < 10; j++) {
			//if(i == 3 || i == 5)
			//	myData->mData.runningTime[i][j] = 9000000; //9ms
			//else myData->mData.runningTime[i][j] = 0;
			//csk_240523
			if((i == 5) && (j == 2 || j == 7))
				myData->mData.runningTime[i][j]	= 90000000;
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

		if(myData->COM.config.functionType[i] == COM_FUNC_TYPE_IO_COMM2) {
			if(myData->COM.config.autoStart[i] == P1) {
				switch(myData->COM.config.functionModel[i]) {
					case 0: //EIOM_B
						myData->COM.com_port[i]
							.signal[COM_SIG_EIOM_B_COMM] = P1;
						myData->COM.com_port[i]
							.signal[COM_SIG_EIOM_B_COMM_COUNT] = P0;
						break;
					default:
						break;
				}
			}
		}
	}

	/* //kjh_211021 switch(myData->AppControl.config.systemModel) {
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
			//myData->mData.misc.rt_periodic = 2000000; //2ms
			break;
	}*/
	if(myData->mData.config.scan_period == 10) {	//kjh_211021
		myData->mData.misc.rt_periodic = 1000000; //1ms
	} else if(myData->mData.config.scan_period == 20) {
		myData->mData.misc.rt_periodic = 2000000; //2ms
	} else {
		myData->mData.misc.rt_periodic = 1000000; //1ms
	}
}

void Save_SystemMemory(void)
{
	char fileName[256];
	int	fp, rtn;
	
	memset(fileName, 0, sizeof(char) * 256);
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

	memset(cmd, 0, sizeof cmd);	//ktg_231031s
	sprintf(cmd, "Load_Update_Process");
	if(Load_Process(cmd, "./", APP_SIG_UPDATE_PROCESS) < 0)
		return -7;	//ktg_231031e

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

	//signalNo = APP_SIG_UPDATE_PROCESS;	//ktg_231031s
	//memset(cmd, 0, sizeof cmd);
	//sprintf(cmd, "Update_Process");
	//Close_Process(cmd, myData->UpdateProcess.misc.processPointer,
	//	signalNo);	//ktg_231031e
	
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
		//printf("Don't open file : %s\n", cmd);
		userlog(DEBUG_LOG, psName, "kill : Don't open file : %s\n", cmd);
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
		//printf("%s %s\n", psName, psKill);
		userlog(DEBUG_LOG, psName, "kill : %s %s\n", psName, psKill);
	}
}

void Kill_Process_2(char *psName, char *index)
{
	char buf[32], psKill[32], cmd[256];
	int rtn, i, result;
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
		//printf("Don't open file : %s\n", cmd);
		userlog(DEBUG_LOG, psName, "kill2 : Don't open file : %s\n", cmd);
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
		//printf("%s %s\n", psName, psKill);
		userlog(DEBUG_LOG, psName, "kill2 : %s %s\n", psName, psKill);
	}
	fclose(fp);
}

void Check_Process(void)
{
	char cmd[80], cmd2[80];
	int rtn, i, signalNo, signalNo2;
	long long diff;
	time_t the_time;

	(void)time(&the_time);

	diff = the_time - myPs->misc.processCheckTime;
	if(diff < 0) {
		myPs->misc.processCheckTime = the_time;
		return;
	} else if(diff < 2) return;

	myPs->misc.processCheckTime = the_time;

	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COA][i] == P1) {
			signalNo = APP_SIG_COA1_CLIENT_PROCESS_CHECK + i;
			signalNo2 = APP_SIG_COA1_CLIENT_PROCESS + i;
			memset(cmd, 0, sizeof(char) * 80);
			sprintf(cmd, "COA%d_Client", i+1);
			rtn = DieCheck_Process(cmd);
			if(rtn < 0) { //process died
				myPs->signal[signalNo]++;
			} else {
				myPs->signal[signalNo] = P0;
			}
			if(myPs->signal[signalNo] >= P3) {
				myPs->signal[signalNo] = P0;
				memset(cmd, 0, sizeof(char) * 80);
				sprintf(cmd, "Load_COA%d_Client", i+1);
				rtn = Load_Process(cmd, "./", signalNo2);
			}

			//kjg_131002
			if(myData->AppControl.signal[APP_SIG_MODULE_CONTROL_PROCESS] == P1) {
				diff = (myData->mData.misc.timer_1sec
					- myData->COA_Client[i].misc.net_time) * 1000;
				diff += (myData->mData.misc.timer_1000ms
					- myData->COA_Client[i].misc.net_time2);
				//if(diff >= myData->COA_Client[i].config.netTimeout
				//	|| diff < 0) {		
				if(diff >= myData->COA_Client[i].config.netTimeout) { //ksh_250203
					userlog(DEBUG_LOG, psName, "app kill : %s\n", cmd);
					Kill_Process(cmd);
				}
			}
		}
	}

#ifdef __COB__
	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_COB][i] == P1) {
			signalNo = APP_SIG_COB1_CLIENT_PROCESS_CHECK + i;
			signalNo2 = APP_SIG_COB1_CLIENT_PROCESS + i;
			memset(cmd, 0, sizeof(char) * 80);
			sprintf(cmd, "COB%d_Client", i+1);
			rtn = DieCheck_Process(cmd);
			if(rtn < 0) { //process died
				myPs->signal[signalNo]++;
			} else {
				myPs->signal[signalNo] = P0;
			}
			if(myPs->signal[signalNo] >= P3) {
				myPs->signal[signalNo] = P0;
				memset(cmd, 0, sizeof(char) * 80);
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
			memset(cmd, 0, sizeof(char) * 80);
			sprintf(cmd, "COC%d_Client", i+1);
			rtn = DieCheck_Process(cmd);
			if(rtn < 0) { //process died
				myPs->signal[signalNo]++;
			} else {
				myPs->signal[signalNo] = P0;
			}
			if(myPs->signal[signalNo] >= P3) {
				myPs->signal[signalNo] = P0;
				memset(cmd, 0, sizeof(char) * 80);
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
			memset(cmd, 0, sizeof(char) * 80);
			sprintf(cmd, "COD%d_Client", i+1);
			rtn = DieCheck_Process(cmd);
			if(rtn < 0) { //process died
				myPs->signal[signalNo]++;
			} else {
				myPs->signal[signalNo] = P0;
			}
			if(myPs->signal[signalNo] >= P3) {
				myPs->signal[signalNo] = P0;
				memset(cmd, 0, sizeof(char) * 80);
				sprintf(cmd, "Load_COD%d_Client", i+1);
				rtn = Load_Process(cmd, "./", signalNo2);
			}
		}
	}
#endif

	/*signalNo = APP_SIG_UPDATE_PROCESS_CHECK;	//ktg_231031s
	signalNo2 = APP_SIG_UPDATE_PROCESS;
	memset(cmd, 0, sizeof(char) * 80);
	sprintf(cmd, "Load_Update_Process");
	rtn = DieCheck_Process(cmd);
	if(rtn < 0) { //process died
		myPs->signal[signalNo]++;
	} else {
		myPs->signal[signalNo] = P0;
	}
	if(myPs->signal[signalNo] >= P3) {
		myPs->signal[signalNo] = P0;
		memset(cmd, 0, sizeof(char) * 80);
		sprintf(cmd, "Load_Update_Process");
		rtn = Load_Process(cmd, "./", signalNo2);	//ktg_231031e
	}*/

	for(i=0; i < MAX_GROUP_8; i++) {
		if(myPs->misc.Load_Process[PROCESS_GROUP_ETC][i] == P1) {
			rtn = 0;
			signalNo = signalNo2 = 0;
			memset(cmd, 0, sizeof(char) * 80);
			memset(cmd2, 0, sizeof(char) * 80);
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
}

int DieCheck_Process(char *process)
{
	char buf[32], cmd[256];
	int rtn;
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

		memset(buf, 0, sizeof(char) * 32);
		rtn = fscanf(fp, "%s", buf);
		if(rtn == EOF) {
			userlog(DEBUG_LOG, psName, "process check eof1 : %s\n", process);
			//kjg_121008 fclose(fp);
			return (-11);
		}
		memset(buf, 0, sizeof(char) * 32);
		rtn = fscanf(fp, "%s", buf);
		if(rtn == EOF) {
			userlog(DEBUG_LOG, psName, "process check eof2 : %s\n", process);
			fclose(fp);
			return (-11);
		}
		memset(buf, 0, sizeof(char) * 32);
		rtn = fscanf(fp, "%s", buf);
		if(rtn == EOF) {
			userlog(DEBUG_LOG, psName, "process check eof3 : %s\n", process);
			fclose(fp);
			return (-11);
		}
		memset(buf, 0, sizeof(char) * 32);
		rtn = fscanf(fp, "%s", buf);
		if(rtn == EOF) {
			userlog(DEBUG_LOG, psName, "process check eof4 : %s\n", process);
			fclose(fp);
			return (-11);
		}
		memset(buf, 0, sizeof(char) * 32);
		rtn = fscanf(fp, "%s", buf);
		if(rtn == EOF) {
			userlog(DEBUG_LOG, psName, "process check eof5 : %s\n", process);
			fclose(fp);
			return (-11);
		} else {
		//if(rtn > 0) {
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
		//} else {
		//	userlog(DEBUG_LOG, psName, "process check : %s\n", process);
		//	rtn = 0;
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
	char buf[32], cmd[256];
	int rtn;
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
	userlog(DEBUG_LOG, psName, "Read_Log_Config complete\n");

	rtn = Read_AppControl_Config(startType);
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_AppControl_Config fail %d\n", rtn);
		return -2;
	}
	userlog(DEBUG_LOG, psName, "Read_AppControl_Config complete\n");

	rtn = Read_mControl_Config();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_mControl_Config fail %d\n", rtn);
		return -3;
	}
	userlog(DEBUG_LOG, psName, "Read_mControl_Config complete\n");

	rtn = Read_Mux_Scan();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Mux_Scan fail %d\n", rtn);
		return -4;
	}
	userlog(DEBUG_LOG, psName, "Read_Mux_Scan complete\n");

	rtn = Read_Load_Process_Config();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Load_Process_Config fail %d\n", rtn);
		return -5;
	}
	userlog(DEBUG_LOG, psName, "Read_Load_Process_Config complete\n");

	rtn = Read_Addr_Map_AIO();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Addr_Map_AIO fail %d\n", rtn);
		return -11;
	}
	userlog(DEBUG_LOG, psName, "Read_Addr_Map_AIO complete\n");

	rtn = Read_DIO_Config();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_DIO_Config fail %d\n", rtn);
		return -12;
	}
	userlog(DEBUG_LOG, psName, "Read_DIO_Config complete\n");

	rtn = Read_II_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_II_Function_Set fail %d\n", rtn);
		return -13;
	}
	userlog(DEBUG_LOG, psName, "Read_II_Function_Set complete\n");

	rtn = Read_IO_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_IO_Function_Set fail %d\n", rtn);
		return -14;
	}
	userlog(DEBUG_LOG, psName, "Read_IO_Function_Set complete\n");

	rtn = Read_MI_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_MI_Function_Set fail %d\n", rtn);
		return -15;
	}
	userlog(DEBUG_LOG, psName, "Read_MI_Function_Set complete\n");

	rtn = Read_MO_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_MO_Function_Set fail %d\n", rtn);
		return -16;
	}
	userlog(DEBUG_LOG, psName, "Read_MO_Function_Set complete\n");

	rtn = Read_CI_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_CI_Function_Set fail %d\n", rtn);
		return -17;
	}
	userlog(DEBUG_LOG, psName, "Read_CI_Function_Set complete\n");

	rtn = Read_CO_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_CO_Function_Set fail %d\n", rtn);
		return -18;
	}
	userlog(DEBUG_LOG, psName, "Read_CO_Function_Set complete\n");

	rtn = Read_EI_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_EI_Function_Set fail %d\n", rtn);
		return -19;
	}
	userlog(DEBUG_LOG, psName, "Read_EI_Function_Set complete\n");

	rtn = Read_EO_Function_Set();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_EO_Function_Set fail %d\n", rtn);
		return -20;
	}
	userlog(DEBUG_LOG, psName, "Read_EO_Function_Set complete\n");

	rtn = Read_Calibration_Data();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Calibration_Data fail %d\n", rtn);
		return -21;
	}
	userlog(DEBUG_LOG, psName, "Read_Calibration_Data complete\n");

	rtn = Read_CellArray_A();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_CellArray_A fail %d\n", rtn);
		return -31;
	}
	userlog(DEBUG_LOG, psName, "Read_CellArray_A complete\n");

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
	userlog(DEBUG_LOG, psName, "Read_ChAttribute complete\n");

	//rtn = Read_AuxSetData();
	//if(rtn < 0) {
	//	userlog(DEBUG_LOG, psName, "Read_AuxSetData fail %d\n", rtn);
	//	return -42;
	//}
	//kjhw_190627s
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
	} //kjhw_190627e
	userlog(DEBUG_LOG, psName, "Read_AuxSetData complete\n");

	rtn = Read_CanConfig();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_CanConfig fail %d\n", rtn);
		return -45;
	}
	userlog(DEBUG_LOG, psName, "Read_CanConfig complete\n");

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
	userlog(DEBUG_LOG, psName, "Read_CanReceiveSetData complete\n");

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
	userlog(DEBUG_LOG, psName, "Read_CanTransmitSetData complete\n");

	rtn = Read_COM_Config();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_COM_Config fail %d\n", rtn);
		return -50;
	}
	userlog(DEBUG_LOG, psName, "Read_COM_Config complete\n");

	rtn = Read_Measure_Cali_1();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Measure_Cali_1 fail %d\n", rtn);
		return -51;
	}
	userlog(DEBUG_LOG, psName, "Read_Measure_Cali_1 complete\n");

	rtn = Read_Th_Table();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Th_Table fail %d\n", rtn);
		return -60;
	}
	userlog(DEBUG_LOG, psName, "Read_Th_Table complete\n");

	rtn = Read_CanFlashFile();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_CanFlashFile fail %d\n", rtn);
		return -70;
	}
	userlog(DEBUG_LOG, psName, "Read_CanFlashFile complete\n");

	rtn = Read_Humidity_Table();		//khj_191205s
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Humidity_Table fail %d\n", rtn);
		return -80;
	}									//khj_191205e
	rtn = Read_CAN_Th_Table();		//ktg_220614s
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_CAN_Th_Table fail %d\n", rtn);
		return -100;
	}									//ktg_220614e
	userlog(DEBUG_LOG, psName, "Read_Can_Th_Table complete\n");
	
	//jhkw_201117s
	rtn = Read_Daq_Map();
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Daq_Map fail %d\n", rtn);
		return -22;
	}
	//jhkw_201117e
	rtn = Read_Measure_Cali_Temp(1);	//khj_210802
	if(rtn < 0) {
		userlog(DEBUG_LOG, psName, "Read_Measure_Cali_Temp fail %d\n", rtn);
		return -90;
	}
	userlog(DEBUG_LOG, psName, "Read_Measure_Cali_Temp complete\n");
	
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
		return -3;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myPs->config.bootOnStart = (unsigned char)atoi(buf);//0:exit, 1:execute

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "modelName") != 0) {
		fclose(fp);
		return -4;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -5;
	}
	tmp = fscanf(fp, "%s", myPs->config.modelName);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "moduleNo") != 0) {
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
	myPs->config.moduleNo = atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "totalGroup") != 0) {
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
	myPs->config.totalGroup = (short int)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "systemModel") != 0) {
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
	myPs->config.systemModel = atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "systemType") != 0) {
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
	//0:formation, 1:general_cycler, 2:ocv, 3:acir/ocv, 4:aging, 5:grader
	//6:selector, 7:pack_cycler, 8:triangle_cycler
	//myPs->config.systemType = (unsigned char)atoi(buf);
	myPs->config.systemType = (short int)atoi(buf);		//csk_190830_c PACK_MERGE
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "sbcType") != 0) {
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
	//1:hs_6637, 2:web_6580, 3:hs_4020, 4:wafer_e669, 5:wafer_lx800
	//6:wafer_mark533, 7:em104_a5362, 8:wafer_mark800, 9:emcore_v621_533
	//10:emcore_a5364			//csk_140723
	myPs->config.sbcType = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "osVersion") != 0) {
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
	//0:kernel v.2.0, RTLinux v.1.3
	//1:kernel v.2.2, RTLinux v.2.0
	//2:kernel v.2.4, RTLinux v.3.2
	//3:kernel v.2.6, RTLinux v.3.2
	myPs->config.osVersion = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "debugType") != 0) {
		fclose(fp);
		return -18;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -19;
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
		return -20;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -21;
	}
	tmp = fscanf(fp, "%s", myPs->config.versionNo);
	//memset(buf, 0, sizeof buf);
	//tmp = fscanf(fp, "%s", buf);
	//myPs->config.versionNo = atoi(buf);
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "update_date") != 0) {
		fclose(fp);
		return -22;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -23;
	}
	tmp = fscanf(fp, "%s", myPs->config.update_date);

	fclose(fp);

	if(startType == START_BOOT && myPs->config.bootOnStart == START_FORCE)
		return -24;
	return 0;
}

int Read_mControl_Config(void)
{
	unsigned char ratioV, ratioI, ratioP; //kjh_211021
	char temp[32], buf[32], fileName[256];
	int tmp, i;
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
	ratioV = myData->mData.config.ratioV;	//kjh_211021

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
	ratioI = myData->mData.config.ratioI; //kjh_211021

	memset(temp, 0, sizeof temp); //kjh_211021s
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
	}	//kjh_211021e

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

	//COA_VER_100F~	//kjh_160610s
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
	//kjh_160610e

	//COA_VER_1016~	//khj_191205s
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedHumidity") != 0) {
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
	myData->mData.config.installedHumidity = (short int)atoi(buf);
	//khj_191205e

	memset(temp, 0, sizeof temp);	//sec_220926s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedGas") != 0) {
		fclose(fp);
		return -56;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -56;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.installedGas = (short int)atoi(buf);   //sec_220926e	//shhw_221018
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedCAN") != 0) {
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
	myData->mData.config.installedCAN = (short int)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "installedCOM") != 0) {
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
	myData->mData.config.installedCOM = (short int)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "daq_type") != 0) {
		fclose(fp);
		return -35;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -35;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	//1:ver1.0 max_64ch
	//2:ver2.0 max_128ch, isolation
	//3:ver2.1 max_256ch, 128ch*2, isolation
	//4:ver3.0 max_128ch, iso3
	//5:ver3.1 max_256ch, iso3 & 128ch*2
	//6:ver3.2 max_128ch, iso3 & NO_DMA
	//7:ver3.3 max_256ch, iso3 & NO_DMA
	//8:ver4.0 max_128ch, iso3, DMA
	//9:ver4.0 max_256ch, iso3, DMA
	//10:ver4.0 max_128ch, iso3, NO_DMA
	//11:ver4.0 max_256ch, iso3, NO_DMA
	//13:ver4.0 max_384ch, iso3, NO_DMA		//csk_191015
	//14:ver4.0 max_512ch, iso3, NO_DMA		//csk_191015
	//20: can_daq
	myData->mData.config.daq_type = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "soft_feed1_v_i_w_r") != 0) {
		fclose(fp);
		return -36;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -36;
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
		return -37;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -37;
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
	myData->mData.config.step_0time_save = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp); //kjg_120323
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "cable_check") != 0) {
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
	myData->mData.config.cable_check = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp); //kjhw_140620
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "division_CAN") != 0) {
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
	myData->mData.config.division_CAN = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp); //kjhw_150120
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "out_precharging_V") != 0) {
		fclose(fp);
		return -41;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -41;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.out_precharging_V = atol(buf);
	//0: not use,
	//ex: uV : 5000000 = +-5V
	//ex: mV : 5000 = +-5V

	memset(temp, 0, sizeof temp); //kjh_211021s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "out_capacitance") != 0) {
		fclose(fp);
		return -42;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -42;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.out_capacitance = atol(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "out_prechg_ratio") != 0) {
		fclose(fp);
		return -43;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -43;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.out_prechg_ratio = atof(buf);	//kjh_211021e

	memset(temp, 0, sizeof temp); //ktg_211104s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "use_leakage_curr") != 0) {
		fclose(fp);
		return -44;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -44;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.use_leakage_curr = (unsigned char)atoi(buf);
	//0: not use,  1: use

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "leakage_curr_I") != 0) {
		fclose(fp);
		return -45;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -45;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.leakage_curr_I = atol(buf);	//ktg_211104e

	memset(temp, 0, sizeof temp); //kjh_211021s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "out_relay_delay") != 0) {
		fclose(fp);
		return -46;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -46;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.out_relay_delay = (short int)atoi(buf);	//kjh_211021e
	
	memset(temp, 0, sizeof temp);	//kjh_211021s
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
	myData->mData.config.da_max = atof(buf);	//kjh_211021e

	memset(temp, 0, sizeof temp);	//kjh_211021s
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
	myData->mData.config.main_amp = atof(buf);	//kjh_211021e

	memset(temp, 0, sizeof temp);	//kjh_211021s
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
	}	//kjh_211021e

	memset(temp, 0, sizeof temp);	//kjh_211021s
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
	}	//kjh_211021e

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
	}	//kjh_211021e

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
	myData->mData.config.caliV_rangeI = (unsigned char)atoi(buf);	//kjh_211021e

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
	myData->mData.config.caliV_cmdI = (short int)atoi(buf);	//kjh_211021e

	memset(temp, 0, sizeof temp);	//shh_211221s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "multi_temp_cali") != 0) {
		fclose(fp);
		return -54;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -55;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.multi_temp_cali = atol(buf);	//shh_211221e

	memset(temp, 0, sizeof temp);	//ktg_240930s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "BreakDownCheck") != 0) {
		fclose(fp);
		return -54;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -55;
	}
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.BreakDownCheck = atol(buf);	//ktg_240930e

	fclose(fp);

	//COA_VER_1016~		//csk_200113
	if((myData->mData.config.installedTemp
		+ myData->mData.config.installedAuxV
		+ myData->mData.config.installedTH
		+ myData->mData.config.installedHumidity	//sec_220926
		+ myData->mData.config.installedGas) > MAX_AUX_DATA) return -70;

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

	memset(temp, 0, sizeof temp);	//kjh_211021s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "upsBatteryFail_TypeFlag") != 0) {
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
	myData->dio.config.upsBatteryFail_TypeFlag = (unsigned char)atoi(buf);
	//kjh_211021e

	memset(temp, 0, sizeof temp);	//kjh_211021s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "acPowerFail_TypeFlag") != 0) {
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
	myData->dio.config.acPowerFail_TypeFlag = (unsigned char)atoi(buf);
	//kjh_211021e

	if(myData->dio.config.logic_type != 0) {
		if(myData->dio.config.watchdogType == 1) {
			myData->dio.config.watchdogType = 2;
		}
	} //kjg_logic_type_140324_e

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
		return -3;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Use") != 0) {
		fclose(fp);
		return -4;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "H/L") != 0) {
		fclose(fp);
		return -5;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Ch") != 0) {
		fclose(fp);
		return -6;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Group") != 0) {
		fclose(fp);
		return -7;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Function") != 0) {
		fclose(fp);
		return -8;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Description") != 0) {
		fclose(fp);
		return -9;
	}

	for(i=0; i < MAX_DIO_II_BYTES; i++) {
		byte_index = i;

		memset(temp, 0, sizeof temp);
		memset(temp2, 0, sizeof temp2);
		sprintf(temp2, "%s%d%s", "byte", i+1, "(in)");
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, temp2) != 0) {
			fclose(fp);
			return -10;
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

#ifdef __B_TYPE__	//kjh_220321
	ch_start = 0;
	ch_end = 8;
#else	//__A_TYPE__
	ch_start = bd * myData->mData.config.chPerBd;
	ch_end = ch_start + myData->mData.config.chPerBd;
#endif

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

int Read_Daq_Map(void)
{	//jhkw_201117
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
	if(strcmp(temp, "Compare_flag") != 0) {
		fclose(fp);
		return -18;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0) {
		fclose(fp);
		return -19;
	}

	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp, "%s", buf);
	myData->mData.config.Compare_flag = atoi(buf);

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
		+ myData->mData.config.installedTH + myData->mData.config.installedHumidity
		+ myData->mData.config.installedGas)	//sec_220926 
		!= installed_daq) {
		userlog(DEBUG_LOG, psName, "DAQ_installed warning \n");
		return -4;
	}

    fclose(fp);
	return 0;
}

int Create_BdCaliData_Org(int bd)
{
	char temp[4], fileName[256];
	int	ch, type, range, point;
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

	}

	fclose(fp);
	return 0;
}

int	Write_ChAttribute(void)
{
	char fileName[256];
	int ch;
	FILE *fp;

	userlog(DEBUG_LOG, psName, "ChAttribute file write start\n");

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
		fprintf(fp, "chiller_con  : %d\n",
			myData->ChAttribute[ch].chiller_control);
		fprintf(fp, "reserved1	  : %d\n",
			myData->ChAttribute[ch].reserved1[0]);
		fprintf(fp, "reserved2	  : %d\n",
			myData->ChAttribute[ch].reserved1[1]);

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
		fprintf(fp, "chiller_con  : %d\n",
			myData->ChAttribute[ch].chiller_control);
		fprintf(fp, "reserved1	  : %d\n",
			myData->ChAttribute[ch].reserved1[0]);
		fprintf(fp, "reserved2	  : %d\n",
			myData->ChAttribute[ch].reserved1[1]);
		
		fprintf(fp, "\n");
	}

	fclose(fp);
	userlog(DEBUG_LOG, psName, "ChAttribute file write end\n");
	return 0;
}

int	Read_AuxSetData(void)
{
	char temp[32], buf[MAX_AUX_NAME_SIZE], fileName[256], *char_tmp;
	int tmp, i, ch, count1, count2;
	int count3, count4, count5; //kjh_160610
	FILE *fp;

	//memset((char *)&myData->auxDataCount, 0,
	//	sizeof(int) * MAX_CH_512 * MAX_AUX_TYPE);	//kjg_180914
	//memset((char *)&myData->auxDataCount, 0,
	//	sizeof(int) * MAX_CH_256 * MAX_AUX_TYPE);
	memset((char *)&myData->auxDataCount, 0,
		sizeof(int) * MAX_CH_4 * MAX_AUX_TYPE);	//ktg_191126
	memset((char *)&myData->auxSetData[0], 0,
		sizeof(S_AUX_SET_DATA) * MAX_AUX_DATA);

	if(myData->mData.config.installedTemp == 0
		&& myData->mData.config.installedAuxV == 0
		&& myData->mData.config.installedTH == 0					//khj_191205
		&& myData->mData.config.installedHumidity == 0//) return 0;	//khj_191205 //sec_220926
		&& myData->mData.config.installedGas == 0) return 0;		//sec_220926

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
			return -3;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].auxChNo = (short int)atoi(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "auxType") != 0) {
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
		myData->auxSetData[i].auxType = (short int)atoi(buf);

		memset(temp, 0, sizeof temp);	//kjh_160610s
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "tableNo") != 0) {
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
		myData->auxSetData[i].tableNo = (unsigned char)atoi(buf);
		//kjh_160610e

		memset(temp, 0, sizeof temp);	//csk_200113s
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "vent_use_flag") != 0) {
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
		myData->auxSetData[i].vent_use_flag = (unsigned char)atoi(buf);
		//csk_200113e

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "chNo") != 0) {
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
		myData->auxSetData[i].chNo = (unsigned char)atoi(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "func_div2") != 0) {
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
		myData->auxSetData[i].function_div2 = (short int)atoi(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "func_div3") != 0) {
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
		myData->auxSetData[i].function_div3 = (short int)atoi(buf);

		memset(temp, 0, sizeof temp);		//ktg_220220s
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "GroupNo") != 0) {
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
		myData->auxSetData[i].GroupNo = (unsigned char)atoi(buf);	//ktg_220220e

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "name") != 0) {
			fclose(fp);
			return -18;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -19;
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
			return -20;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -21;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].fault_upper = atol(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "fault_lower") != 0) {
			fclose(fp);
			return -22;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -23;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].fault_lower = atol(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "end_upper") != 0) {
			fclose(fp);
			return -24;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -25;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].end_upper = atol(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "end_lower") != 0) {
			fclose(fp);
			return -26;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -27;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].end_lower = atol(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "vent_upper") != 0) {
			fclose(fp);
			return -28;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -29;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].vent_upper = atol(buf);
		
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "vent_lower") != 0) {
			fclose(fp);
			return -30;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -31;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].vent_lower = atol(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "func_div") != 0) {
			fclose(fp);
			return -32;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -33;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].function_div = (short int)atoi(buf);

		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, "reserved2") != 0) {
			fclose(fp);
			return -34;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);
		if(strcmp(temp, ":") != 0) {
			fclose(fp);
			return -35;
		}
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->auxSetData[i].reserved2 = (short int)atoi(buf);
	}
	fclose(fp);

	userlog(DEBUG_LOG, psName, "Read_AuxSetData END:\n");	//csk_200113
	//for(ch=0; ch < MAX_CH_512; ch++) {	//kjg_180914
	//for(ch=0; ch < MAX_CH_256; ch++) {
	for(ch=0; ch < MAX_CH_4; ch++) {	//kjh_191115
		count1 = count2 = 0;
		count3 = 0; //kjh_160610
		count4 = 0; //khj_191205
		count5 = 0; //sec_220926
		for(i=0; i < MAX_AUX_DATA; i++) {
			if((ch+1) == myData->auxSetData[i].chNo) {
				if(myData->auxSetData[i].auxType == 0) { //temperature
					count1++;
				} else if(myData->auxSetData[i].auxType == 1) { //voltage
					count2++;
				} else if(myData->auxSetData[i].auxType == 2) { //thermistor
					count3++; //kjh_160610
				} else if(myData->auxSetData[i].auxType == 3) { //humidity
					count4++; //khj_191205
				} else if(myData->auxSetData[i].auxType == 4) { //gas
					count5++; //sec_220926
				}
			}
		}
		myData->auxDataCount[ch][0] = count1;
		myData->auxDataCount[ch][1] = count2;
		myData->auxDataCount[ch][2] = count3; //kjh_160610
		myData->auxDataCount[ch][3] = count4; //khj_191205
		myData->auxDataCount[ch][4] = count5; //sec_220926
		userlog(DEBUG_LOG, psName, "ch%d: Temp(%d), AuxV(%d), AuxTH(%d), AuxHumidity(%d)\n, AuxGas(%d)\n", ch+1, count1, count2, count3, count4, count5);	//csk_200113 //sec_220926
	}
	//userlog(DEBUG_LOG, psName, "\n");	//csk_200113

	return 0;
}

int	Write_AuxSetData(void)
{
	char fileName[256];
	int i, ch, count1, count2;
	int count3, count4, count5; //kjh_160610
	FILE *fp;

	//memset((char *)&myData->auxDataCount, 0,
	//	sizeof(int) * MAX_CH_512 * MAX_AUX_TYPE);	//kjg_180914
	//memset((char *)&myData->auxDataCount, 0,
	//	sizeof(int) * MAX_CH_256 * MAX_AUX_TYPE);
	memset((char *)&myData->auxDataCount, 0,
		sizeof(int) * MAX_CH_4 * MAX_AUX_TYPE);	//ktg_191126

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
		fprintf(fp, "tableNo     : %d\n", myData->auxSetData[i].tableNo);
		//kjh_160610e
		fprintf(fp, "vent_use_flag     : %d\n", myData->auxSetData[i].vent_use_flag);
		//csk_200113e
		fprintf(fp, "chNo        : %d\n", myData->auxSetData[i].chNo);
		fprintf(fp, "func_div2   : %d\n", myData->auxSetData[i].function_div2);
		fprintf(fp, "func_div3   : %d\n", myData->auxSetData[i].function_div3);

		fprintf(fp, "GroupNo 	  : %d\n", myData->auxSetData[i].GroupNo);	//ktg_220220
		if(myData->auxSetData[i].name[0] == 0) {
			fprintf(fp, "name        : $\n");
		} else {
			fprintf(fp, "name        : %s\n", myData->auxSetData[i].name);
		}
		fprintf(fp, "fault_upper : %ld\n", myData->auxSetData[i].fault_upper);
		fprintf(fp, "fault_lower : %ld\n", myData->auxSetData[i].fault_lower);
		fprintf(fp, "end_upper   : %ld\n", myData->auxSetData[i].end_upper);
		fprintf(fp, "end_lower   : %ld\n", myData->auxSetData[i].end_lower);

		fprintf(fp, "vent_upper : %ld\n", myData->auxSetData[i].vent_upper);
		fprintf(fp, "vent_lower : %ld\n", myData->auxSetData[i].vent_lower);

		fprintf(fp, "func_div    : %d\n", myData->auxSetData[i].function_div);
		fprintf(fp, "reserved2   : %d\n", myData->auxSetData[i].reserved2);
		fprintf(fp, "\n");
	}
	fclose(fp);

	userlog(DEBUG_LOG, psName, "Write_AuxSetData END:\n");	//csk_200113
	//for(ch=0; ch < MAX_CH_512; ch++) {	//kjg_180914
	//for(ch=0; ch < MAX_CH_256; ch++) {
	for(ch=0; ch < MAX_CH_4; ch++) {
		count1 = count2 = 0;
		count3 = 0; //kjh_160610
		count4 = 0; //khj_191205
		count5 = 0; //sec_220926
		for(i=0; i < MAX_AUX_DATA; i++) {
			if((ch+1) == myData->auxSetData[i].chNo) {
				if(myData->auxSetData[i].auxType == 0) { //temperature
					count1++;
				} else if(myData->auxSetData[i].auxType == 1) { //voltage
					count2++;
				} else if(myData->auxSetData[i].auxType == 2) { //thermistor
					count3++; //kjh_160610
				} else if(myData->auxSetData[i].auxType == 3) { //wetness
					count4++; //khj_191205
				} else if(myData->auxSetData[i].auxType == 4) { //gas
					count5++; //sec_220926
				}
			}
		}
		myData->auxDataCount[ch][0] = count1;
		myData->auxDataCount[ch][1] = count2;
		myData->auxDataCount[ch][2] = count3; //kjh_160610
		myData->auxDataCount[ch][3] = count4; //khj_191205
		myData->auxDataCount[ch][4] = count5; //sec_220926
		userlog(DEBUG_LOG, psName, "ch%d: Temp(%d), AuxV(%d), AuxTH(%d), AuxHumidity(%d)\n, AuxGas(%d)\n", ch+1, count1, count2, count3, count4, count5);	//csk_200113	//sec_220926
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

	memset(temp, 0, sizeof temp);	//kjh_211021s
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "can_delay") != 0) {
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
	myData->CAN.config.can_delay = (unsigned char)atoi(buf);	//kjh_211021e

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
	myData->CAN.config.reserved1[0] = (unsigned char)atoi(buf);

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
	myData->CAN.config.reserved1[1] = (unsigned char)atoi(buf);

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
	myData->CAN.config.reserved1[2] = (unsigned char)atoi(buf);

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
	int tmp, i, j, ch, count, rtn, cell_count;
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

				//ksh_241112_s
				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "manual_can_bitconfig") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].manual_can_bitconfig
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_brp") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].nominal_brp
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_tseg1") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].nominal_tseg1
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_tseg2") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].nominal_tseg2
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_sjw") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].nominal_sjw
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_sample_point") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].nominal_sample_point
					= atof(buf);//ksh_241112
				//ksh_241112_e
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
				//jhkw_181119s
				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "can_lin_flag") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].can_lin_flag
					= (unsigned char)atoi(buf); //kjhw_181111
				//jhkw_181119e

				//ktg_210706s
				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "cell_check_use_flag") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].cell_check_use_flag
					= (unsigned char)atoi(buf); //ktg_210706
				//ktg_210706e

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

				//ksh_241112_s
				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "manual_can_bitconfig") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].manual_can_bitconfig
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_brp") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].nominal_brp
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_tseg1") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].nominal_tseg1
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_tseg2") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].nominal_tseg2
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_sjw") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].nominal_sjw
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_sample_point") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].nominal_sample_point
					= atof(buf);//ksh_241112
					
				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_brp") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].data_brp
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_tseg1") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].data_tseg1
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_tseg2") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].data_tseg2
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_sjw") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].data_sjw
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_sample_point") != 0) {
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
				myData->canReceiveSetData.commonData[ch][i].data_sample_point
					= atof(buf);//ksh_241112
				//ksh_241112_e
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

			memset(temp, 0, sizeof temp);	//kjhw_131204s
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
			//if(strcmp(temp, "reserved4") != 0) {	//ktg_210706
			if(strcmp(temp, "cell_check") != 0) {
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
			//myData->canReceiveSetData.normalData[ch][i].reserved4	//ktg_210706
			myData->canReceiveSetData.normalData[ch][i].cell_check
				= (unsigned char)atoi(buf);

			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			//if(strcmp(temp, "reserved5") != 0) {	//ktg_210706
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
			//myData->canReceiveSetData.normalData[ch][i].reserved4[1]	//ktg_210706
			myData->canReceiveSetData.normalData[ch][i].reserved4
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
		}
		fclose(fp);

		count = 0;
		cell_count = 0;	//ktg_210706
		for(i=0; i < MAX_CAN_DATA; i++) { //master
			//if(myData->canReceiveSetData.normalData[ch][i].canType == CAN_TYPE_MASTER)	//ktg_210706
			if(myData->canReceiveSetData.normalData[ch][i].canType == CAN_TYPE_MASTER) {
				count++;
				if(myData->canReceiveSetData.normalData[ch][i].cell_check == 1)
					cell_count++;
			}
		}
		myData->canReceiveDataCount[ch][0] = count;
		myData->canReceiveCellCount[ch][0] = cell_count;	//ktg_210706

		count = 0;
		cell_count = 0;	//ktg_210706
		for(i=0; i < MAX_CAN_DATA; i++) { //slave
			//if(myData->canReceiveSetData.normalData[ch][i].canType == CAN_TYPE_SLAVE)	//ktg_210706
			if(myData->canReceiveSetData.normalData[ch][i].canType == CAN_TYPE_SLAVE) {
				count++;
				if(myData->canReceiveSetData.normalData[ch][i].cell_check == 1)
					cell_count++;
			}
		}
		myData->canReceiveDataCount[ch][1] = count;
		myData->canReceiveCellCount[ch][1] = cell_count;	//ktg_210706

		if(( myData->canReceiveCellCount[ch][0] + myData->canReceiveCellCount[ch][1])
			> MAX_CAN_CELL_COUNT) {	//ktg_210706
			return -5;
		}	//ktg_210706
		userlog(DEBUG_LOG, psName, "CanReceiveSetData%02d file read end\n",
			ch+1);
	}

	return 0;
}

int	Write_CanReceiveSetData(void)
{
	char fileName[256];
	int i, ch, count, cell_count;	//ktg_210706
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
				myData->canReceiveSetData.commonData[ch][i].filter[0]);	//ktg_210706	mask filter
			fprintf(fp, "filter2 : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].filter[1]);
			fprintf(fp, "filter3 : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].filter[2]);
			fprintf(fp, "filter4 : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].filter[3]);
			fprintf(fp, "filter5 : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].filter[4]);
			fprintf(fp, "filter6 : %lx\n",
				myData->canReceiveSetData.commonData[ch][i].filter[5]);

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
				//ksh_241112_s
				fprintf(fp, "manual_can_bitconfig      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].manual_can_bitconfig); 
				fprintf(fp, "nominal_brp      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_brp);
				fprintf(fp, "nominal_tseg1      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_tseg1);
				fprintf(fp, "nominal_tseg2      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_tseg2);
				fprintf(fp, "nominal_sjw      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_sjw);
				fprintf(fp, "nominal_sample_point      : %f\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_sample_point);
				//ksh_241112_e
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
				//jhkw_181119s
				fprintf(fp, "can_lin_flag : %d\n",
					myData->canReceiveSetData.commonData[ch][i].can_lin_flag);
				//jhkw_181119e

				//ktg_210706s
				fprintf(fp, "cell_check_use_flag : %d\n",
					myData->canReceiveSetData.commonData[ch][i].cell_check_use_flag);
				//ktg_210706e
				
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
				//ksh_241112_s
				fprintf(fp, "manual_can_bitconfig      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].manual_can_bitconfig); 
				fprintf(fp, "nominal_brp      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_brp);
				fprintf(fp, "nominal_tseg1      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_tseg1);
				fprintf(fp, "nominal_tseg2      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_tseg2);
				fprintf(fp, "nominal_sjw      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_sjw);
				fprintf(fp, "nominal_sample_point      : %f\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_sample_point);
				fprintf(fp, "data_brp      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].data_brp);
				fprintf(fp, "data_tseg1      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].data_tseg1);
				fprintf(fp, "data_tseg2      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].data_tseg2);
				fprintf(fp, "data_sjw      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].data_sjw);
				fprintf(fp, "data_sample_point      : %f\n",
					myData->canReceiveSetData.commonData[ch][i].data_sample_point);
				//fprintf(fp, "reserved1 : 0\n");
					//ksh_241112 myData->canReceiveSetData.commonData[ch][i].reserved1);
				//fprintf(fp, "reserved3 : 0\n");
					//ksh_241112 myData->canReceiveSetData.commonData[ch][i].reserved3);
				//fprintf(fp, "reserved4 : 0\n");
					//ksh_241112 myData->canReceiveSetData.commonData[ch][i].reserved4);
				//ksh_241112_e
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
			//kjhw_131204s
			fprintf(fp, "startBit2   : %d\n",
				myData->canReceiveSetData.normalData[ch][i].startBit2);
			fprintf(fp, "bitCount2   : %d\n",
				myData->canReceiveSetData.normalData[ch][i].bitCount2);
			fprintf(fp, "byte_order2 : %d\n",
				myData->canReceiveSetData.normalData[ch][i].byte_order2);
			fprintf(fp, "data_type2  : %d\n",
				myData->canReceiveSetData.normalData[ch][i].data_type2);
			/*fprintf(fp, "reserved4   : %d\n",	//ktg_210706s
				myData->canReceiveSetData.normalData[ch][i].reserved4[0]);
			fprintf(fp, "reserved5   : %d\n",
				myData->canReceiveSetData.normalData[ch][i].reserved4[1]);*/
			fprintf(fp, "cell_check   : %d\n",
				myData->canReceiveSetData.normalData[ch][i].cell_check);
			fprintf(fp, "reserved4   : %d\n",
				myData->canReceiveSetData.normalData[ch][i].reserved4);	//ktg_210706e
			fprintf(fp, "compare_value : %f\n",
				myData->canReceiveSetData.normalData[ch][i].compare_value);
			//kjhw_131204e
		}
		fclose(fp);

		count = 0;
		cell_count = 0; //ktg_210706
		for(i=0; i < MAX_CAN_DATA; i++) { //master
			//if(myData->canReceiveSetData.normalData[ch][i].canType == CAN_TYPE_MASTER)	//ktg_210706
			if(myData->canReceiveSetData.normalData[ch][i].canType == CAN_TYPE_MASTER) {
				count++;
				if(myData->canReceiveSetData.normalData[ch][i].cell_check == 1)
					cell_count++;
			}
		}
		myData->canReceiveDataCount[ch][0] = count;
		myData->canReceiveCellCount[ch][0] = cell_count;	//ktg_210706
		
		count = 0;
		cell_count = 0; //ktg_210706
		for(i=0; i < MAX_CAN_DATA; i++) { //slave
			//if(myData->canReceiveSetData.normalData[ch][i].canType == CAN_TYPE_SLAVE)	//ktg_210706
			if(myData->canReceiveSetData.normalData[ch][i].canType == CAN_TYPE_SLAVE) {
				count++;
				if(myData->canReceiveSetData.normalData[ch][i].cell_check == 1)
					cell_count++;
			}
		}
		myData->canReceiveDataCount[ch][1] = count;
		myData->canReceiveCellCount[ch][1] = cell_count;	//ktg_210706
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

				//ksh_241112_s
				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "manual_can_bitconfig") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].manual_can_bitconfig
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_brp") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].nominal_brp
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_tseg1") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].nominal_tseg1
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_tseg2") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].nominal_tseg2
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_sjw") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].nominal_sjw
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_sample_point") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].nominal_sample_point
					= atof(buf);//ksh_241112
				//ksh_241112_e
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

				//jhkw_181119s
				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "can_lin_flag") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].can_lin_flag
					= (unsigned char)atoi(buf);
				//jhkw_181119e

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

				//ksh_241112_s
				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "manual_can_bitconfig") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].manual_can_bitconfig
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "norminal_brp") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].nominal_brp
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "norminal_tseg1") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].nominal_tseg1
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "norminal_tseg2") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].nominal_tseg2
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_sjw") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].nominal_sjw
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "nominal_sample_point") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].nominal_sample_point
					= atof(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_brp") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].data_brp
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_tseg1") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].data_tseg1
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_tseg2") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].data_tseg2
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_sjw") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].data_sjw
					= (unsigned char)atoi(buf);//ksh_241112

				memset(temp, 0, sizeof temp);
				tmp = fscanf(fp, "%s", temp);
				if(strcmp(temp, "data_sample_point") != 0) {
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
				myData->canTransmitSetData.commonData[ch][i].data_sample_point
                    = atof(buf);//ksh_241112
				//ksh_241112_e
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
			if(myData->canTransmitSetData.normalData[ch][i].canType == CAN_TYPE_MASTER)
				count++;
		}
		myData->canTransmitDataCount[ch][0] = count;

		count = 0;
		for(i=0; i < MAX_CAN_DATA; i++) { //slave
			if(myData->canTransmitSetData.normalData[ch][i].canType == CAN_TYPE_SLAVE)
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
	int i, ch; //, count;	csk_220818
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
				fprintf(fp, "reserved1 : 0\n");
					//kjg_180405 myData->canReceiveSetData.commonData[ch][i].reserved1[0]);
				fprintf(fp, "reserved2 : 0\n");
					//kjg_180405 myData->canReceiveSetData.commonData[ch][i].reserved1[1]);
				fprintf(fp, "reserved3 : 0\n");
					//kjg_180405 myData->canReceiveSetData.commonData[ch][i].reserved1[2]);

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
				//ksh_241112_s
				fprintf(fp, "manual_can_bitconfig      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].manual_can_bitconfig); 
				fprintf(fp, "nominal_brp      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_brp);
				fprintf(fp, "nominal_tseg1      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_tseg1);
				fprintf(fp, "nominal_tseg2      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_tseg2);
				fprintf(fp, "nominal_sjw      : %d\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_sjw);
				fprintf(fp, "nominal_sample_point      : %f\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_sample_point);
				//ksh_241112_e
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
				//jhkw_181119s
				fprintf(fp, "can_lin_flag :	%d\n",
					myData->canTransmitSetData.commonData[ch][i].can_lin_flag);
				//jhkw_181119e

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
				//ksh_241112_s
				fprintf(fp, "manual_can_bitconfig      : %d\n",
					myData->canTransmitSetData.commonData[ch][i].manual_can_bitconfig);
				fprintf(fp, "norminal_brp      : %d\n",
					myData->canTransmitSetData.commonData[ch][i].nominal_brp);
				fprintf(fp, "norminal_tseg1      : %d\n",
					myData->canTransmitSetData.commonData[ch][i].nominal_tseg1);
				fprintf(fp, "norminal_tseg2      : %d\n",
					myData->canTransmitSetData.commonData[ch][i].nominal_tseg2);
				fprintf(fp, "nominal_sjw      : %d\n",
					myData->canTransmitSetData.commonData[ch][i].nominal_sjw);
				fprintf(fp, "nominal_sample_point      : %f\n",
					myData->canReceiveSetData.commonData[ch][i].nominal_sample_point);
				fprintf(fp, "data_brp      : %d\n",
					myData->canTransmitSetData.commonData[ch][i].data_brp);
				fprintf(fp, "data_tseg1      : %d\n",
					myData->canTransmitSetData.commonData[ch][i].data_tseg1);
				fprintf(fp, "data_tseg2      : %d\n",
					myData->canTransmitSetData.commonData[ch][i].data_tseg2);
				fprintf(fp, "data_sjw      : %d\n",
					myData->canTransmitSetData.commonData[ch][i].data_sjw);
				fprintf(fp, "data_sample_point      : %f\n",
					myData->canTransmitSetData.commonData[ch][i].data_sample_point);
				//fprintf(fp, "reserved1 : 0\n");
					//ksh_241112 myData->canReceiveSetData.commonData[ch][i].reserved1);
				//fprintf(fp, "reserved3 : 0\n");
					//ksh_241112 myData->canReceiveSetData.commonData[ch][i].reserved3);
				//fprintf(fp, "reserved4 : 0\n");
					//ksh_241112 myData->canReceiveSetData.commonData[ch][i].reserved4);
				//ksh_241112_e
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

		/*count = 0;	//220818s
		for(i=0; i < MAX_CAN_DATA; i++) { //master
			if(myData->canTransmitSetData.normalData[ch][i].canType == CAN_TYPE_MASTER)
				count++;
		}
		myData->canTransmitDataCount[ch][0] = count;

		count = 0;
		for(i=0; i < MAX_CAN_DATA; i++) { //slave
			if(myData->canTransmitSetData.normalData[ch][i].canType == CAN_TYPE_SLAVE)
				count++;
		}
		myData->canTransmitDataCount[ch][1] = count;*/	//csk_220818e
	}

	return 0;
}

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
	}
	system(cmd);
	userlog(DEBUG_LOG, psName, "th_table_copy table No : %d\n", table);
	usleep(500000);
}

void Copy_Humidity_Table(int table)
{	//csk_200207
	char cmd[256];

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "cp -rf ");
	strcat(cmd, "/root/system_data/config/parameter/humidity_table/");
	strcat(cmd, "Humidity_table_00.txt ");
	strcat(cmd, "/root/system_data/config/parameter/humidity_table/");
	if(table == 1) {
		strcat(cmd, "Humidity_table_01.txt ");
	} else if(table == 2) {
		strcat(cmd, "Humidity_table_02.txt ");
	} else if(table == 3) {
		strcat(cmd, "Humidity_table_03.txt ");
	} else if(table == 4) {
		strcat(cmd, "Humidity_table_04.txt ");
	} else if(table == 5) {
		strcat(cmd, "Humidity_table_05.txt ");
	} else if(table == 6) {
		strcat(cmd, "Humidity_table_06.txt ");
	} else if(table == 7) {
		strcat(cmd, "Humidity_table_07.txt ");
	} else if(table == 8) {
		strcat(cmd, "Humidity_table_08.txt ");
	} else if(table == 9) {
		strcat(cmd, "Humidity_table_09.txt ");
	}
	system(cmd);
	userlog(DEBUG_LOG, psName, "humidity_table_copy table No : %d\n", table);
	usleep(500000);
}
void Copy_CAN_Th_Table(int table)
{	//ktg_220614
	char cmd[256];

	memset(cmd, 0, sizeof cmd);
	strcpy(cmd, "cp -rf ");
	strcat(cmd, "/root/system_data/config/parameter/can_th_table/");
	strcat(cmd, "th_table_00.txt ");
	strcat(cmd, "/root/system_data/config/parameter/can_th_table/");
	if(table == 1) {
		strcat(cmd, "can_th_table_01.txt ");
	} else if(table == 2) {
		strcat(cmd, "can_th_table_02.txt ");
	} else if(table == 3) {
		strcat(cmd, "can_th_table_03.txt ");
	} else if(table == 4) {
		strcat(cmd, "can_th_table_04.txt ");
	} else if(table == 5) {
		strcat(cmd, "can_th_table_05.txt ");
	} else if(table == 6) {
		strcat(cmd, "can_th_table_06.txt ");
	} else if(table == 7) {
		strcat(cmd, "can_th_table_07.txt ");
	} else if(table == 8) {
		strcat(cmd, "can_th_table_08.txt ");
	} else if(table == 9) {
		strcat(cmd, "can_th_table_09.txt ");
	}
	system(cmd);
	userlog(DEBUG_LOG, psName, "can_th_table_copy table No : %d\n", table);
	usleep(500000);
}

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
		//7 : COM_FUNC_TYPE_CHAMBER1
		//  - 0:TEMP880, 1:TEMP2000_2500
		//8 : COM_FUNC_TYPE_IO_COMM2
		//	- 0:EIOM_B
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

int Read_CAN_Th_Table(void)	//ktg_220614s
{
	int rtn = 0;

	switch(myData->AppControl.config.systemModel) {
		default:
			rtn = Read_CAN_Th_Table_Type(0, 0); 
			if(rtn < 0) return (10 * (-1) + rtn); 
			rtn = Read_CAN_Th_Table_Type(1, 1); 
			if(rtn < 0) return (20 * (-1) + rtn); 
			rtn = Read_CAN_Th_Table_Type(2, 2); 
			if(rtn < 0) return (30 * (-1) + rtn); 
			rtn = Read_CAN_Th_Table_Type(3, 3); 
			if(rtn < 0) return (40 * (-1) + rtn); 
			rtn = Read_CAN_Th_Table_Type(4, 4); 
			if(rtn < 0) return (50 * (-1) + rtn); 
			rtn = Read_CAN_Th_Table_Type(5, 5); 
			if(rtn < 0) return (60 * (-1) + rtn); 
			rtn = Read_CAN_Th_Table_Type(6, 6); 
			if(rtn < 0) return (70 * (-1) + rtn); 
			rtn = Read_CAN_Th_Table_Type(7, 7); 
			if(rtn < 0) return (80 * (-1) + rtn); 
			rtn = Read_CAN_Th_Table_Type(8, 8); 
			if(rtn < 0) return (90 * (-1) + rtn); 
			rtn = Read_CAN_Th_Table_Type(9, 9); 
			if(rtn < 0) return (100 * (-1) + rtn); 
			break;
	}
	return 0;
}

int Read_CAN_Th_Table_Type(int table, int type)
{
	char temp[32], buf[32], fileName[256];
	int tmp, i;
	float CT_V, temp_val;	//kjh_211021
	FILE *fp;

	memset(fileName, 0, sizeof fileName);
	sprintf(fileName,
		"/root/%s/config/parameter/can_th_table/can_th_table_%02d.txt",
		myData->AppControl.misc.path1, type);
	if((fp = fopen(fileName, "r")) == NULL){
		userlog(DEBUG_LOG, psName, 
			"can_th_table_%02d file read error\n", type);
		return -1;
	}
	
	myData->can_th_table[table].can_th_type = (unsigned char)type;
	
	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Bias_type") != 0){
		fclose(fp);
		return -2;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0){
		fclose(fp);
		return -3;
	}
	
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp," %s", buf);
	myData->can_th_table[table].bias_type = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "temp_degreeC") != 0) {
		fclose(fp);
		return -4;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "V_Out") != 0) {
		fclose(fp);
		return -5;
	}
	for(i=0; i < MAX_CAN_TH_DATA; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		temp_val = atof(buf) * 1000.0; //degreeC * 1000	//ktg_220706
		myData->can_th_table[table].CT_V[i][0] = temp_val;
		if(strcmp(buf, "9999") == 0) { //equal
			if(i == 0) {
			} else {
				i = i - 1;
			}
			myData->can_th_table[table].can_th_data_max_index = (short int)i;
			break;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		CT_V = atof(buf);	//V 
		myData->can_th_table[table].CT_V[i][1] = (float)CT_V;

		if(i == 1) {
			if((myData->can_th_table[table].CT_V[i-1][0] < temp_val)
				&& (myData->can_th_table[table].CT_V[i-1][1] > CT_V)) {
				myData->can_th_table[table].type = 1; //NTC temp:up, voltage:down
			} else if((myData->can_th_table[table].CT_V[i-1][0] < temp_val)
				&& (myData->can_th_table[table].CT_V[i-1][1] < CT_V)) {
				myData->can_th_table[table].type = 2; //NTC temp:up, voltage:up
			} else {
				myData->can_th_table[table].type = 0; //None
			}
		}
	}
	fclose(fp);
	return 0;	
}	//ktg_220614e

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

int Read_Th_Table(void)
{
	switch(myData->AppControl.config.systemModel) {
		case F_SBL_5V_250A_125A_65A_10A:
		case F_SDI_5V_450A_200A_100A_10A_2:
		case F_SDI_5V_450A_200A_100A_10A_3:
		case F_SDI_5V_450A_200A_100A_10A_4:
		case F_SDI_5V_450A_200A_100A_10A_96CH:	//hun_160408
		case F_SDI_5V_450A_200A_100A_10A_96CH_2:	//hun_160808
		case F_SDI_5V_450A_200A_100A_10A_96CH_3:	//hun_161201
		case F_SDI_5V_450A_200A_100A_10A_96CH_4:	//hun_170703
			if(Read_Th_Table_Type(0, 5) < 0) return -1; //NTC-103F343F
			break;
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
		//case C_LGC_70V_250A_50A_35KW_9:		//jhk_160515
		//case C_LGC_70V_250A_50A_35KW_10:	//jhk_160515	//shh_220105
		//case C_LGC_70V_250A_50A_35KW_11:	//jhk_160515
		//case C_LGC_70V_250A_50A_35KW_12:	//jhk_160515
		//case C_LGC_70V_250A_50A_35KW_13:	//jhk_160515
		//case C_LGC_70V_250A_50A_35KW_14:	//jhk_160515
		//case C_LGC_70V_250A_50A_35KW_15://jhk_160515
		//case C_LGC_70V_250A_50A_35KW_16:	//jhk_160515
		case C_LGCUSA_70V_300A_42KW:	//jhk_160802
		case C_LGCUSA_70V_300A_42KW_2:	//jhk_160802
		case C_LGCUSA_70V_350A_24KW:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_2:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_3:	//jhk_160720
		case C_LGCUSA_70V_350A_24KW_4:	//jhk_160720
		//case C_LGC_70V_300A_100A_42KW:  //ktg_210222
		case C_LGCCHINA_80V_150A_12KW:		//jhk_160725
		case C_LGCCHINA_80V_150A_12KW_2:	//jhk_160725
		case C_LGCCHINA_100V_200A_20KW_3:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_4:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_5:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_6:	//jhk_160731
		case C_LGCCHINA_100V_200A_20KW_7:	//jhk_160731
		//case C_LGC_120V_250A_50A_60KW:	//jhk_160617
			if(Read_Th_Table_Type(0, 1) < 0) return -1; //103AT
			break;
		case C_LGE_150V_400A_60KW:	//jhk_170721
			if(Read_Th_Table_Type(0, 4) < 0) return -4; //CEC NTC Thermistor
			break;
		case C_LGC_400V_100A_10A_40KW:
		case C_3PSYSTEM_400V_100A_10A_40KW_1:	//jhk_120723
		case C_3PSYSTEM_400V_100A_10A_40KW_2:	//jhk_120723
			if(Read_Th_Table_Type(0, 1) < 0) return -1; //103AT
			if(Read_Th_Table_Type(1, 2) < 0) return -2; //103JT_025
			break;
		//case C_LGC_450V_200A_20A_180KW:	//ktg_210222
		//case C_LGC_450V_200A_20A_180KW_2:	//ktg_210222
		//	if(Read_Th_Table_Type(0, 1) < 0) return -1; //103AT
		//	if(Read_Th_Table_Type(1, 2) < 0) return -2; //103JT_025
		//	if(Read_Th_Table_Type(2, 3) < 0) return -3; //KCG103F343F
		//	break;
		//case C_LGC_450V_250A_50A_225KW:		//jhk_160608	//ktg_210222
		//case C_LGC_450V_250A_50A_225KW_2:		//jhk_160608	//ktg_210222
		//	if(Read_Th_Table_Type(0, 0) < 0) return -1; //non
		//	if(Read_Th_Table_Type(1, 1) < 0) return -2; //103AT
		//	if(Read_Th_Table_Type(2, 2) < 0) return -3; //103JT_025
		//	if(Read_Th_Table_Type(3, 3) < 0) return -4; //KCG103F343F
		//	if(Read_Th_Table_Type(4, 4) < 0) return -5; //reserved
		//	break;
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
			if(Read_Th_Table_Type(0, 0) < 0) return -1; //non
			if(Read_Th_Table_Type(1, 1) < 0) return -2; //103AT
			if(Read_Th_Table_Type(2, 2) < 0) return -3; //103JT_025
			if(Read_Th_Table_Type(3, 3) < 0) return -4; //KCG103F343F
			if(Read_Th_Table_Type(4, 4) < 0) return -5; //reserved
			if(Read_Th_Table_Type(5, 5) < 0) return -6; //reserved
			if(Read_Th_Table_Type(6, 6) < 0) return -7; //reserved
			if(Read_Th_Table_Type(7, 7) < 0) return -8; //reserved
			if(Read_Th_Table_Type(8, 8) < 0) return -9; //reserved
			if(Read_Th_Table_Type(9, 9) < 0) return -10; //reserved
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
	float temp_val;	//kjh_211021
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
		//myData->th_table[table].T_R[i][0] = atof(buf) * 1000.0; //temp * 1000
		temp_val = atof(buf) * 1000.0; //temp * 1000
		myData->th_table[table].T_R[i][0] = temp_val;
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

		if(i == 1) { //kjh_210317s
			if((myData->th_table[table].T_R[i-1][0] < temp_val)
				&& (myData->th_table[table].T_R[i-1][1] > Rth)) {
				myData->th_table[table].type = 1; //NTC temp:up, reg:down
			} else if((myData->th_table[table].T_R[i-1][0] < temp_val)
				&& (myData->th_table[table].T_R[i-1][1] < Rth)) {
				myData->th_table[table].type = 2; //PTC temp:up, reg:up
			} else {
				myData->th_table[table].type = 0; //Non
			}
		}  //kjh_210317e
	}

	fclose(fp);
	return 0;
}

int Read_Humidity_Table(void)
{
	int rtn = 0;
	
	switch(myData->AppControl.config.systemModel) {
		default:
			//rtn = Read_Humidity_Table_Type(0, 0); 
			//rtn = Read_Humidity_Table_Type(1, 1); 
			//rtn = Read_Humidity_Table_Type(2, 2); 
			//rtn = Read_Humidity_Table_Type(3, 3); 
			//if(rtn < 0) return rtn; 
			rtn = Read_Humidity_Table_Type(0, 0); 
			if(rtn < 0) return (10 * (-1) + rtn); 
			rtn = Read_Humidity_Table_Type(1, 1); 
			if(rtn < 0) return (20 * (-1) + rtn); 
			rtn = Read_Humidity_Table_Type(2, 2); 
			if(rtn < 0) return (30 * (-1) + rtn); 
			rtn = Read_Humidity_Table_Type(3, 3); 
			if(rtn < 0) return (40 * (-1) + rtn); 
			rtn = Read_Humidity_Table_Type(4, 4); 
			if(rtn < 0) return (50 * (-1) + rtn); 
			rtn = Read_Humidity_Table_Type(5, 5); 
			if(rtn < 0) return (60 * (-1) + rtn); 
			rtn = Read_Humidity_Table_Type(6, 6); 
			if(rtn < 0) return (70 * (-1) + rtn); 
			rtn = Read_Humidity_Table_Type(7, 7); 
			if(rtn < 0) return (80 * (-1) + rtn); 
			rtn = Read_Humidity_Table_Type(8, 8); 
			if(rtn < 0) return (90 * (-1) + rtn); 
			rtn = Read_Humidity_Table_Type(9, 9); 
			if(rtn < 0) return (100 * (-1) + rtn); 
			break;
	}
	return 0;
}

int Read_Humidity_Table_Type(int table, int type)
{	//khj_191205
	char temp[32], buf[32], fileName[256];
	int tmp, i;
	float H_V;
	FILE *fp;

	memset(fileName, 0, sizeof fileName);
	sprintf(fileName,
					"/root/%s/config/parameter/humidity_table/Humidity_%02d.txt",
					myData->AppControl.misc.path1, type);
	if((fp = fopen(fileName, "r")) == NULL){
		userlog(DEBUG_LOG, psName, 
						"humidity_table_%02d file read error\n", type);
		return -1;
	}

	myData->humidity_table[table].humidity_type = (unsigned char)type;
	
	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);
	tmp = fscanf(fp, "%s", temp);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Bias_type") != 0){
		fclose(fp);
		return -2;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, ":") != 0){
		fclose(fp);
		return -3;
	}
	
	memset(buf, 0, sizeof buf);
	tmp = fscanf(fp," %s", buf);
	myData->humidity_table[table].bias_type = (unsigned char)atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Humidity") != 0) {
		fclose(fp);
		return -4;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "V_Out") != 0) {
		fclose(fp);
		return -5;
	}
	for(i=0; i < MAX_HUMIDITY_DATA; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		myData->humidity_table[table].H_V[i][0] = atof(buf) * 1000.0; //wetness * 1000
		if(strcmp(buf, "9999") == 0) { //equal
			if(i == 0) {
			} else {
				i = i - 1;
			}
			myData->humidity_table[table].humidity_data_max_index = (short int)i;
			break;
		}

		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
		H_V = atof(buf) * 1000000.0;	//V 
		myData->humidity_table[table].H_V[i][1] = (float)H_V;

	}

	fclose(fp);
	return 0;	
}

int Read_Measure_Cali_Temp(int mode)
{	//khj_210802
	char fileName[256], temp[32], buf[32];
	int tmp, i, j;
	int point_set, range_set;
	FILE *fp;

	memset(fileName, 0, sizeof fileName);
	strcpy(fileName, "/root/");
	strcat(fileName, (char *)&myPs->misc.path1);

	if(mode == 0) {
		strcat(fileName, "/config/parameter/Measure_Cali_zero");
	} else if(mode == 1) {
		strcat(fileName, "/config/parameter/Measure_Cali_Temp");
	} else if(mode == 2) {
		strcat(fileName, "/config/parameter/Measure_Cali_Temp");
	} else {

	}
	
	if((fp = fopen(fileName, "r")) == NULL) {
		userlog(DEBUG_LOG, psName, "Measure_Cali_Temp\n");
		return -1;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Temp_point_count") != 0) {
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
	point_set = atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "Temp_range_count") != 0) {
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
	range_set = atoi(buf);
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "index") != 0) {
		fclose(fp);
		return -6;
	}
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "point") != 0) {
		fclose(fp);
		return -7;
	}
	
//	memset(buf, 0, sizeof buf);
//	tmp = fscanf(fp, "%s", buf);
//	point[0] = atoi(buf);

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ratio") != 0) {
		fclose(fp);
		return -8;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "offset") != 0) {
		fclose(fp);
		return -9;
	}
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "point") != 0) {
		fclose(fp);
		return -10;
	}
	
//	memset(buf, 0, sizeof buf);
//	tmp = fscanf(fp, "%s", buf);
//	point[1] = atoi(buf);
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ratio") != 0) {
		fclose(fp);
		return -11;
	}
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "offset") != 0) {
		fclose(fp);
		return -12;
	}
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "point") != 0) {
		fclose(fp);
		return -13;
	}
	
//	memset(buf, 0, sizeof buf);
//	tmp = fscanf(fp, "%s", buf);
//	point[2] = atoi(buf);
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ratio") != 0) {
		fclose(fp);
		return -14;
	}
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "offset") != 0) {
		fclose(fp);
		return -15;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "point") != 0) {
		fclose(fp);
		return -16;
	}
	
//	memset(buf, 0, sizeof buf);
//	tmp = fscanf(fp, "%s", buf);
//	point[3] = atoi(buf);
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "ratio") != 0) {
		fclose(fp);
		return -17;
	}

	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "offset") != 0) {
		fclose(fp);
		return -18;
	}
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	if(strcmp(temp, "point") != 0) {
		fclose(fp);
		return -19;
	}
	
//	memset(buf, 0, sizeof buf);
//	tmp = fscanf(fp, "%s", buf);
//	point[4] = atoi(buf);
	
	for(i=0; i < MAX_AUX_DATA; i++) {
		memset(buf, 0, sizeof buf);
		tmp = fscanf(fp, "%s", buf);
	
		for(j=0; j < 4; j++) {	
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->measure_cali_temp[0][i].temp[j] = atoi(buf);
			
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->measure_cali_temp[0][i].factor[j] = atof(buf);
			
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->measure_cali_temp[0][i].offset[j] = atof(buf);
		}
			memset(buf, 0, sizeof buf);
			tmp = fscanf(fp, "%s", buf);
			myData->measure_cali_temp[0][i].temp[4] = atoi(buf);
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

