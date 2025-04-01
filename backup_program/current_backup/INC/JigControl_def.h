#ifndef __JIGCONTROL_DEF_H__
#define __JIGCONTROL_DEF_H__

//jig define
#define MAX_JIG_IN_SIGNAL			32
#define MAX_JIG_OUT_SIGNAL			32
#define MAX_JIG_STATE_PER_GROUP		8

//jig state
#define J_IDLE						0
#define J_DISCONTACT				1
#define J_TO_CONTACT				2
#define J_CONTACT					3
#define J_TO_DISCONTACT				4
#define J_TRAYCHECK					5
#define J_FAIL						6	
#define J_EMG						7 //for SMOKE, TEMP sens

//jig mode
#define J_REMOTE					0
#define J_LOCAL						1

//jigControl Signal
#define J_SIG_TRY_CONTACT				0
#define J_SIG_TRY_DISCONTACT			1
#define J_SIG_TRY_TRAY_CHECK			2
#define J_SIG_LAMP_GREEN				3
#define J_SIG_LAMP_AMBER				4
#define J_SIG_LAMP_RED					5
#define J_SIG_TRY_TRAY_CHECK_COMPLETE	6
#define J_SIG_JOB_CHANGE				7
#define J_SIG_DOOR_OPEN					8
#define J_SIG_DOOR_CLOSE				9
#define J_SIG_PITCH_CHANGE				10
#define J_SIG_PLC_ALARM_AREA_SENSOR		11
#define J_SIG_PLC_ALARM_MANUAL_DOOR_OPEN	12
#define J_SIG_PLC_ALARM_AIR				13
#define J_SIG_PLC_ALARM_APR_S			14
#define J_SIG_PLC_ALARM_REF				15
#define J_SIG_PLC_ALARM_COIL_TRIP		16
#define J_SIG_PLC_ALARM_TEMP			17
#define J_SIG_PLC_ALARM_SMOKE			18
#define J_SIG_PLC_ALARM_FIRE			19
#define J_SIG_PLC_ALARM_EMG				20
#define J_SIG_CHAMBER_FAULT				21
#define J_SIG_PLC_NFB_TRIP				22 //hun_150420_d_s
#define J_SIG_PLC_ALARM_MANUAL_FIRE		23	//hun_160822
#define J_SIG_CHAMBER_ALARM_TEMP		24	//csk_161028_s
#define J_SIG_CHAMBER_ALARM_AIR			25
#define J_SIG_CHAMBER_ALARM_REF			26
#define J_SIG_CHAMBER_ALARM_MOTOR		27
#define J_SIG_CHAMBER_ALARM_LEAK		28
#define J_SIG_CHAMBER_ALARM_DOOR_OPEN	29
#define J_SIG_CHAMBER_ALARM_FIRE		30
#define J_SIG_CHAMBER_ALARM_SMOKE		31
#define J_SIG_CHAMBER_ALARM_EMG			32	//csk_161028_e

//Sensor State Check
#define J_S_JIGSTATE_CHECK			0
#define J_S_TRAYSTATE_CHECK			1
#define J_S_STOPPERSTATE_CHECK		2
#define J_S_DOORSTATE_CHECK			3
#define J_S_IN_BITS1				4
#define J_S_IN_BITS2				5
#define J_S_IN_BITS3				6
#define J_S_IN_BITS4				7

// tray state
#define T_IDLE						0
#define T_UNLOAD					1
#define T_MOVING					2
#define T_LOAD						3
#define T_CHECK						4

// door state
#define D_IDLE						0
#define D_OPEN						1
#define D_MOVING					2
#define D_CLOSE						3

// stopper state
#define S_DISCONTACT				0
#define S_CONTACT					1

//common use jig code
#define J_CD_NONE							0
#define J_CD_MAIN_CYL_UP_TIMEOUT			1
#define J_CD_MAIN_CYL_DOWN_TIMEOUT			2
#define J_CD_LATCH_CYL_BOTH_OPEN_TIMEOUT	3
#define J_CD_LATCH_CYL_R_OPEN_TIMEOUT		4
#define J_CD_LATCH_CYL_L_OPEN_TIMEOUT		5
#define J_CD_LATCH_CYL_BOTH_CLOSE_TIMEOUT	6
#define J_CD_LATCH_CYL_R_CLOSE_TIMEOUT		7
#define J_CD_LATCH_CYL_L_CLOSE_TIMEOUT		8
#define J_CD_GRIP_CYL_OPEN_TIMEOUT			9
#define J_CD_GRIP_CYL_CLOSE_TIMEOUT			10
#define J_CD_MAIN_CYL_UP_ERROR				11
#define J_CD_MAIN_CYL_DOWN_ERROR			12
#define J_CD_LATCH_CYL_R_OPEN_ERROR			13
#define J_CD_LATCH_CYL_L_OPEN_ERROR			14
#define J_CD_LATCH_CYL_R_CLOSE_ERROR		15
#define J_CD_LATCH_CYL_L_CLOSE_ERROR		16
#define J_CD_GRIP_CYL_OPEN_ERROR			17
#define J_CD_GRIP_CYL_CLOSE_ERROR			18
#define J_CD_STOPPER_ERROR					19
#define J_CD_SMOKE_SENS_1					20
#define J_CD_SMOKE_SENS_2					21
#define J_CD_OVER_TEMP_1					22
#define J_CD_OVER_TEMP_2					23
#define J_CD_TRAY_UNLOAD					24
#define J_CD_TRAY_DIR_ERROR					25
#define J_CD_AIR_PRESS_ERROR				26
#define J_CD_EMG_SWITCH						27
#define J_CD_ROLL_CYL_DOWN_ERROR			28
#define J_CD_ROLL_CYL_UP_ERROR				29
#define J_CD_CHAMBER_DOOR_OPEN_ERROR		30
#define J_CD_CHAMBER_FAULT					31
#define J_CD_JOB_CHANGE_ERROR				32
#define J_CD_PITCH_CHANGE_ERROR				33
#define J_CD_CHAMBER_AREA_SENSOR			34
#define J_CD_CHAMBER_MANUAL_DOOR_OPEN		35
#define J_CD_CHAMBER_AIR					36
#define J_CD_CHAMBER_APR_S					37
#define J_CD_CHAMBER_REF					38
#define J_CD_CHAMBER_COIL_TRIP				39
#define J_CD_CHAMBER_FIRE					40
#define J_CD_TRAY_UP1_FAIL					41
#define J_CD_TRAY_UP2_FAIL					42
#define J_CD_PLC_NFB_TRIP					43		//hun_150420
#define J_CD_CHAMBER_MANUAL_FIRE			44		//hun_160822
#define J_CD_JIG_MAINTENANCE_STATE			45		//csk_161017_s
#define J_CD_MAINTENANCE_CYL_TIMEOUT		46
#define J_CD_CHAMBER_MOTOR					47		
#define J_CD_CHAMBER_LEAK					48		//csk_161017_e

//jig1 input signal
#define J1_IN_LATCH_CYL_L_OPEN		0
#define J1_IN_LATCH_CYL_R_OPEN		1
#define J1_IN_MAIN_CYL_UP			2
#define J1_IN_GRIP_CYL_OPEN			3
#define J1_IN_LATCH_CYL_L_CLOSE		4
#define J1_IN_LATCH_CYL_R_CLOSE		5
#define J1_IN_MAIN_CYL_DOWN			6
#define J1_IN_GRIP_CYL_CLOSE		7
#define J1_IN_OVER_TEMP				8
#define J1_IN_TRAY_STATUS			9
#define J1_IN_SMOKE_SENS_1			10
#define J1_IN_SMOKE_SENS_2			11
#define J1_IN_STACKER_JIG_NOT_MOVE	12
#define J1_IN_STACKER_LOADING_END	13
#define J1_IN_TRAY_DIR				14

//jig1 output signal
#define J1_OUT_FAN_RELAY			0
#define J1_OUT_FAN_RELAY1			1
#define J1_OUT_STACKER_TRAY_OUT		2
#define J1_OUT_STACKER_TRAY_IN		3
#define J1_OUT_LATCH_CYL			4
#define J1_OUT_MAIN_CYL				5
#define J1_OUT_GRIP_CYL				6
#define J1_OUT_LED_RED				7
#define J1_OUT_LED_AMBER			8
#define J1_OUT_LED_GREEN			9
#define J1_OUT_TOWER_LAMP_RED		10
#define J1_OUT_TOWER_LAMP_AMBER		11
#define J1_OUT_TOWER_LAMP_GREEN		12

//jig2 input signal
#define J2_IN_LATCH_CYL_L_OPEN		0
#define J2_IN_LATCH_CYL_R_OPEN		1
#define J2_IN_MAIN_CYL_UP			2
#define J2_IN_GRIP_CYL_OPEN			3
#define J2_IN_LATCH_CYL_L_CLOSE		4
#define J2_IN_LATCH_CYL_R_CLOSE		5
#define J2_IN_MAIN_CYL_DOWN			6
#define J2_IN_GRIP_CYL_CLOSE		7
#define J2_IN_OVER_TEMP				8
#define J2_IN_TRAY_STATUS			9
#define J2_IN_SMOKE_SENS_1			10
#define J2_IN_SMOKE_SENS_2			11
#define J2_IN_STACKER_JIG_NOT_MOVE	12
#define J2_IN_STACKER_LOADING_END	13
#define J2_IN_TRAY_DIR				14

//jig2 output signal
#define J2_OUT_FAN_RELAY			0
#define J2_OUT_FAN_RELAY1			1
#define J2_OUT_STACKER_TRAY_OUT		2
#define J2_OUT_STACKER_TRAY_IN		3
#define J2_OUT_LATCH_CYL			4
#define J2_OUT_MAIN_CYL				5
#define J2_OUT_GRIP_CYL				6
#define J2_OUT_LED_RED				7
#define J2_OUT_LED_AMBER			8
#define J2_OUT_LED_GREEN			9
#define J2_OUT_TOWER_LAMP_RED		10
#define J2_OUT_TOWER_LAMP_AMBER		11
#define J2_OUT_TOWER_LAMP_GREEN		12

//jig3 input signal
#define J3_IN_STOPPER_ERROR1		0
#define J3_IN_STOPPER_ERROR2		1
#define J3_IN_STOPPER_ERROR3		2
#define J3_IN_STOPPER_ERROR4		3
#define J3_IN_DOWN1					4
#define J3_IN_DOWN2					5
#define J3_IN_DOWN3					6
#define J3_IN_DOWN4					7
#define J3_IN_TRAY1					8
#define J3_IN_TRAY2					9
#define J3_IN_TRAY3					10
#define J3_IN_TRAY4					11
#define J3_IN_AIR_PRESS				12
#define J3_IN_UP1					13
#define J3_IN_UP2					14
#define J3_IN_SW_UP					15
#define J3_IN_SW_DOWN				16

//jig3 output signal
#define J3_OUT_UP_SOL				0
#define J3_OUT_DOWN_SOL				1

//jig4 input signal
#define J4_IN_TRAY_STATUS_1			0
#define J4_IN_TRAY_STATUS_2			1
#define J4_IN_MAIN_CYL_DOWN			2
#define J4_IN_STOPPER_STATUS		3
#define J4_IN_AIR_SW				4

//jig4 output signal
#define J4_OUT_BCR_LAMP_1			0
#define J4_OUT_BCR_LAMP_2			1
#define J4_OUT_MAIN_CYL_UP			2
#define J4_OUT_MAIN_CYL_DOWN		3
#define J4_OUT_TOWER_LAMP_RED		4
#define J4_OUT_TOWER_LAMP_AMBER		5
#define J4_OUT_TOWER_LAMP_GREEN		6

//jig5 input signal
#define J5_IN_CYL_OPEN_L			0
#define J5_IN_CYL_CLOSE_L			1
#define J5_IN_CYL_CLOSE_R			2
#define J5_IN_CYL_OPEN_R			3
#define J5_IN_TRAY_STATUS_1			4
#define J5_IN_TRAY_STATUS_2			5

//jig5 output signal
#define J5_OUT_CYL_CLOSE			0
#define J5_OUT_CYL_OPEN				1
#define J5_OUT_JIG_FAN				2

//jig6 input signal
#define J6_IN_CYL_OPEN_L			0
#define J6_IN_CYL_CLOSE_L			1
#define J6_IN_CYL_OPEN_R			2
#define J6_IN_CYL_CLOSE_R			3
#define J6_IN_TRAY_1				4
#define J6_IN_TRAY_2				5
#define J6_IN_SMOKE_SENS_1			6
#define J6_IN_SMOKE_SENS_2			7
#define J6_IN_OVER_TEMP_1			8
#define J6_IN_OVER_TEMP_2			9

//jig6 output signal
#define J6_OUT_FAN_RELAY_1			0
#define J6_OUT_FAN_RELAY_2			1
#define J6_OUT_LED_GREEN			2
#define J6_OUT_LED_AMBER			3
#define J6_OUT_LED_RED				4
#define J6_OUT_GRIP_CYL_OPEN		5
#define J6_OUT_GRIP_CYL_CLOSE		6
#define J6_OUT_TRAY_LAMP_1			7
#define J6_OUT_TRAY_LAMP_2			8

//jig7 input signal
#define J7_IN_LATCH_CYL_L_OPEN		0
#define J7_IN_LATCH_CYL_R_OPEN		1
#define J7_IN_LATCH_CYL_L_CLOSE		2
#define J7_IN_LATCH_CYL_R_CLOSE		3
#define J7_IN_MAIN_CYL_UP			4
#define J7_IN_MAIN_CYL_DOWN			5
#define J7_IN_GRIP_CYL_OPEN			6
#define J7_IN_GRIP_CYL_CLOSE		7
#define J7_IN_TRAY_STATUS			8
#define J7_IN_SMOKE_SENS_1			9
#define J7_IN_SMOKE_SENS_2			10
#define J7_IN_START_BUTTON			11
#define J7_IN_EMG_SWITCH			12

//jig7 output signal
#define J7_OUT_LATCH_CYL			0
#define J7_OUT_MAIN_CYL				1
#define J7_OUT_GRIP_CYL				2
#define J7_OUT_FAN_RELAY_1			3
#define J7_OUT_FAN_RELAY_2			4
#define J7_OUT_TOWER_LAMP_GREEN		5
#define J7_OUT_TOWER_LAMP_AMBER		6
#define J7_OUT_TOWER_LAMP_RED		7
#define J7_OUT_JIG_START_LAMP		8

//jig8 input signal
#define J8_IN_GRIP_CYL_OPEN			0
#define J8_IN_GRIP_CYL_CLOSE		1
#define J8_IN_TRAY_1				2
#define J8_IN_TRAY_2				3
#define J8_IN_SMOKE_SENS_1			4
#define J8_IN_SMOKE_SENS_2			5

//jig8 output signal
#define J8_OUT_GRIP_CYL_OPEN		0
#define J8_OUT_GRIP_CYL_CLOSE		1

//job change
#define JOB_CHANGE_IDLE		0
#define JOB_CHANGE_1		1
#define JOB_CHANGE_2		2
#define JOB_CHANGE_3		3
#define JOB_CHANGE_4		4
#define JOB_CHANGE_5		5
#define JOB_CHANGE_6		6
#define JOB_CHANGE_7		7
#define JOB_CHANGE_8		8

//job change end
#define	JOB_CHANGE_IDLE		0
#define	JOB_CHANGE_START	1
#define	JOB_CHANGE_END		2

//pitch change
#define PITCH_CHANGE_IDLE		0
#define PITCH_CHANGE_1		1
#define PITCH_CHANGE_2		2
#define PITCH_CHANGE_3		3
#define PITCH_CHANGE_4		4
#define PITCH_CHANGE_5		5
#define PITCH_CHANGE_6		6
#define PITCH_CHANGE_7		7
#define PITCH_CHANGE_8		8

//pitch change end
#define	PITCH_CHANGE_IDLE	0
#define	PITCH_CHANGE_START	1
#define	PITCH_CHANGE_END	2

#endif
