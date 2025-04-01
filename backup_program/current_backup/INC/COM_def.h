#ifndef __COM_DEF_H__
#define __COM_DEF_H__

//COM define
#define MAX_COM_PORT							8

//packet
#define	MAX_COM_PACKET_LENGTH					(320*16)
#define MAX_COM_PACKET_COUNT					64

//functionType
#define COM_FUNC_TYPE_NONE						0
#define COM_FUNC_TYPE_METER1					1
	//0 :Agilent34401A
#define COM_FUNC_TYPE_DISPLAY1					2
	//0 : PNE_PanelMeter_A(232)
	//1 : PNE_PanelMeter_B(485)
	//2 : mcu2, ch4
#define COM_FUNC_TYPE_METER2					3
	//0 : CB-7018
	//1 : XL-122
	//2 : COMM_A(XL-122)
	//3 : I-7018Z
	//4 : I-7033
#define COM_FUNC_TYPE_CALIBRATOR1				4
	//0 : Calibrator_B
	//1 : Calibrator_C
	//2 : Calibrator_D
#define COM_FUNC_TYPE_BCR1						5
	//0 : MS-860(RS-485)
	//1 : MS-860(RS-232)
	//2 : COMM_B(MS-860)
#define COM_FUNC_TYPE_IO_COMM1					6
	//0 : EIOM_A
	//1 : PLC_Q03UD
#define COM_FUNC_TYPE_CHAMBER1					7
	//0 : TEMP-880
	//1 : TEMP-2000
#define COM_FUNC_TYPE_IO_COMM2					8
	//0 : EIOM_B

//34401A readType
#define READ_V_V								0
#define READ_V_I								1

//CB_7018 readType
#define READ_T									0
#define READ_V									1
#define READ_I									2

//measureI
#define MEASURE_I_1								1	//shunt 0.010ohm
#define MEASURE_I_2								2	//shunt 0.001ohm
#define MEASURE_I_3								3	//DCCT 600A/400mA
#define MEASURE_I_4								4	//DCCT 150A/200mA
#define MEASURE_I_5								5	//shunt 10ohm, 100ohm
#define MEASURE_I_6								6	//meter DCI
#define MEASURE_I_7								7	//shunt 300A/100mV
#define MEASURE_I_8								8	//shunt 600A/100mV
#define MEASURE_I_9								9	//shunt 0.25ohm
#define MEASURE_I_10							10	//DCCT 1000A/666.6mA
#define MEASURE_I_11							11	//shunt 0.0002ohm
#define MEASURE_I_12							12	//shunt 0.0005ohm
#define MEASURE_I_13							13	//shunt 0.0001ohm

//cali type
#define CALI_TYPE_VOLTAGE						0
#define CALI_TYPE_CURRENT						1

//cali mode
#define CALI_MODE_NORMAL						0
#define CALI_MODE_CHECK							1
#define CALI_MODE_ONLY_CHECK					2

//IO_COMM ADDR
#define	IO_ADDR_1					"50"
#define	IO_ADDR_2					"51"
#define	IO_ADDR_3					"52"
#define	IO_ADDR_4					"53"
#define	IO_ADDR_5					"54"
#define	IO_ADDR_6					"55"
#define	IO_ADDR_7					"56"
#define	IO_ADDR_8					"57"
#define	IO_ADDR_9					"58"
#define	IO_ADDR_10					"59"
#define	IO_ADDR_11					"5A"
#define	IO_ADDR_12					"5B"
#define	IO_ADDR_13					"5C"
#define	IO_ADDR_14					"5D"
#define	IO_ADDR_15					"5E"
#define	IO_ADDR_16					"5F"
#define CALI_ADDR_1					"60"
#define COMM_A_ADDR_1				"70"
#define COMM_B_ADDR_1				"80"
#define EIOM_B_ADDR_1				"90"
#define EIOM_B_ADDR_2				"91"

//IO_COMM Command
#define	CMD_RESET					"RST" //0
#define	CMD_NACK					"NAK"
#define	CMD_COMM_CHK				"CHK"
#define	CMD_WR1						"WR1"
#define	CMD_RD1						"RD1"
#define	CMD_IO1						"IO1"
#define	CMD_II1						"II1"
#define CMD_INI						"INI"
#define CMD_EO1						"EO1"
#define CMD_EO2						"EO2"
#define CMD_EO3						"EO3" //10
#define CMD_EO4						"EO4"
#define CMD_EO5						"EO5"
#define CMD_EO6						"EO6"
#define CMD_EO7						"EO7"
#define CMD_EO8						"EO8"
#define CMD_EI1						"EI1"
#define CMD_EI2						"EI2"
#define CMD_EI3						"EI3"
#define CMD_EI4						"EI4"
#define CMD_EI5						"EI5" //20
#define CMD_EI6						"EI6"
#define CMD_EI7						"EI7"
#define CMD_EI8						"EI8"
#define CMD_CO1						"CO1"
#define CMD_ESC						"ESC"
#define CMD_ESO						"ESO"
#define CMD_MDS						"MDS"
#define CMD_MDE						"MDE"
#define CMD_MDT						"MDT"
#define CMD_MD1						"MD1" //30
#define CMD_REQ						"REQ"
#define CMD_IO2						"IO2"
#define CMD_IO3						"IO3"
#define CMD_IO4						"IO4"
#define CMD_IO5						"IO5"
#define CMD_IO6						"IO6"
#define CMD_IO7						"IO7"
#define CMD_IO8						"IO8"
#define CMD_II2						"II2"
#define CMD_II3						"II3" //40
#define CMD_II4						"II4"
#define CMD_II5						"II5"
#define CMD_II6						"II6"
#define CMD_II7						"II7"
#define CMD_II8						"II8"
#define CMD_ADS						"ADS"
#define CMD_A01						"A01"
#define CMD_A02						"A02"
#define CMD_A03						"A03"
#define CMD_A04						"A04" //50
#define CMD_A05						"A05"
#define CMD_A06						"A06"
#define CMD_A07						"A07"
#define CMD_A08						"A08"
#define CMD_A09						"A09"
#define CMD_A10						"A10"
#define CMD_A11						"A11"
#define CMD_A12						"A12"
#define CMD_A13						"A13"
#define CMD_A14						"A14" //60
#define CMD_A15						"A15"
#define CMD_A16						"A16"
#define CMD_A17						"A17"
#define CMD_A18						"A18"
#define CMD_A19						"A19"
#define CMD_A20						"A20"
#define CMD_A21						"A21"
#define CMD_A22						"A22"
#define CMD_A23						"A23"
#define CMD_A24						"A24" //70
#define CMD_A25						"A25"
#define CMD_A26						"A26"
#define CMD_A27						"A27"
#define CMD_A28						"A28"
#define CMD_A29						"A29"
#define CMD_A30						"A30"
#define CMD_A31						"A31"
#define CMD_A32						"A32"
#define CMD_A33						"A33"
#define CMD_A34						"A34" //80
#define CMD_A35						"A35"
#define CMD_A36						"A36"
#define CMD_A37						"A37"
#define CMD_A38						"A38"
#define CMD_A39						"A39"
#define CMD_A40						"A40"
#define CMD_A41						"A41"
#define CMD_A42						"A42"
#define CMD_A43						"A43"
#define CMD_A44						"A44" //90
#define CMD_A45						"A45"
#define CMD_A46						"A46"

//CB_7018, IO_COMM delimiter
#define SND_STX						'@' //0x40 host -> module
#define RCV_STX						'!' //0x21 host <- module

//bcr define
#define COM_BCR_SIZE							64

//MS-860 select address
#define BCR1_SELECT_1							0x1D
#define BCR1_SELECT_2							0x1F

//MS-860 polling address
#define BCR1_POLLING_1							0x1C
#define BCR1_POLLING_2							0x1E

//COM Signal
#define COM_SIG_CALI_METER_INITIALIZE			0
#define COM_SIG_CALI_METER_REQUEST_PHASE		1
#define COM_SIG_CALI_METER_ERROR				2
#define COM_SIG_CALI_CURRENT_RANGE				3
#define COM_SIG_CALI_VOLTAGE_RANGE				4
#define COM_SIG_CALI_MODE_INITIALIZE			5	//jhkw_130121

#define COM_SIG_BCR_REQUEST_PHASE				10
#define COM_SIG_BCR_COMM_GROUP					11
#define COM_SIG_BCR_WAIT_COUNT					12
#define COM_SIG_BCR_RETRY_COUNT					13
#define COM_SIG_BCR_READ_COUNT					14

#define COM_SIG_DISPLAY							20

#define COM_SIG_ANALOG_METER_INITIALIZE			30
#define COM_SIG_ANALOG_METER_REQUEST_PHASE		31
#define COM_SIG_ANALOG_METER_ERROR				32
#define COM_SIG_ANALOG_METER_MEASURE			33

#define COM_SIG_CHAMBER_RUN						40
#define COM_SIG_CHAMBER_STOP					41
#define COM_SIG_CHAMBER_HOLD					42
#define COM_SIG_CHAMBER_RSD						43
#define COM_SIG_CHAMBER_REQUEST_PHASE			44
#define COM_SIG_CHAMBER_WAIT_COUNT				45
#define COM_SIG_CHAMBER_COMM					46
#define COM_SIG_CHAMBER_COMM_COUNT				47
#define COM_SIG_CHAMBER_SET						48

#define COM_SIG_PLC_REQUEST_PHASE				50
#define COM_SIG_PLC_ALARM_AREA_SENSOR			51
#define COM_SIG_PLC_ALARM_MANUAL_DOOR_OPEN		52
#define COM_SIG_PLC_ALARM_CHAMBER_RUN			53
#define COM_SIG_PLC_ALARM_DOOR_CLOSE			54
#define COM_SIG_PLC_ALARM_DOOR_OPEN				55
#define COM_SIG_PLC_ALARM_AIR					56
#define COM_SIG_PLC_ALARM_APR_S					57
#define COM_SIG_PLC_ALARM_REF					58
#define COM_SIG_PLC_ALARM_COIL_TRIP				59
#define COM_SIG_PLC_ALARM_TEMP					60
#define COM_SIG_PLC_ALARM_SMOKE					61
#define COM_SIG_PLC_ALARM_FIRE					62
#define COM_SIG_PLC_ALARM_EMG					63
#define COM_SIG_PLC_COMM						64
#define COM_SIG_PLC_COMM_COUNT					65
#define COM_SIG_PLC_WAIT_COUNT					66
#define COM_SIG_PLC_DOOR_OPEN					67
#define COM_SIG_PLC_DOOR_CLOSE					68
#define COM_SIG_PLC_INIT_SEND					69
#define COM_SIG_PLC_FAULT_READ					70
#define COM_SIG_PLC_ALARM_AREA_SENSOR_COUNT		71
#define COM_SIG_PLC_ALARM_MANUAL_DOOR_OPEN_COUNT	72
#define COM_SIG_PLC_ALARM_CHAMBER_RUN_COUNT		73
#define COM_SIG_PLC_ALARM_DOOR_CLOSE_COUNT		74
#define COM_SIG_PLC_ALARM_DOOR_OPEN_COUNT		75
#define COM_SIG_PLC_ALARM_AIR_COUNT				76
#define COM_SIG_PLC_ALARM_APR_S_COUNT			77
#define COM_SIG_PLC_ALARM_REF_COUNT				78
#define COM_SIG_PLC_ALARM_COIL_TRIP_COUNT		79
#define COM_SIG_PLC_ALARM_TEMP_COUNT			80
#define COM_SIG_PLC_ALARM_SMOKE_COUNT			81
#define COM_SIG_PLC_ALARM_FIRE_COUNT			82
#define COM_SIG_PLC_ALARM_EMG_COUNT				83
#define COM_SIG_PLC_ALARM_NFB_TRIP				84	//hun_150421_csk
#define COM_SIG_PLC_COMM_IDLE_COUNT				85	//hun_150421_csk
#define COM_SIG_PLC_ALARM_MANUAL_FIRE_COUNT		86	//hun_160822
#define COM_SIG_PLC_ALARM_MANUAL_FIRE			87	//hun_160822

#define COM_SIG_EIOM_B_REQUEST_PHASE			90
#define COM_SIG_EIOM_B_ERROR					91
#define COM_SIG_EIOM_B_COMM						92
#define COM_SIG_EIOM_B_COMM_COUNT				93
#define COM_SIG_EIOM_B_WAIT_COUNT				94
#define COM_SIG_EIOM_B_SERVO_PHASE				95
#define COM_SIG_EIOM_B_SERVO_COUNT				96
#define COM_SIG_EIOM_B_SERVO_TABLE_ID			97
#define COM_SIG_EIOM_B_STEP_PHASE				98
#define COM_SIG_EIOM_B_STEP_COUNT				99
#define COM_SIG_EIOM_B_STEP_TABLE_ID			100
#define COM_SIG_EIOM_B_ADS_VAL					101
#define COM_SIG_EIOM_B_AD_PHASE					102
#define COM_SIG_EIOM_B_COMM_PHASE				103
#define COM_SIG_EIOM_B_COMM_PHASE2				104

#define	COM_SIG_CHAMBER_ALARM_TEMP				120		//csk_161028_s
#define	COM_SIG_CHAMBER_ALARM_TEMP_COUNT		121
#define COM_SIG_CHAMBER_ALARM_DOOR_OPEN			122
#define COM_SIG_CHAMBER_ALARM_DOOR_OPEN_COUNT	123
#define COM_SIG_CHAMBER_ALARM_AIR				124
#define COM_SIG_CHAMBER_ALARM_AIR_COUNT			125
#define COM_SIG_CHAMBER_ALARM_REF				126
#define COM_SIG_CHAMBER_ALARM_REF_COUNT			127
#define COM_SIG_CHAMBER_ALARM_MOTOR				128
#define COM_SIG_CHAMBER_ALARM_MOTOR_COUNT		129
#define COM_SIG_CHAMBER_ALARM_LEAK				130
#define COM_SIG_CHAMBER_ALARM_LEAK_COUNT		131
#define COM_SIG_CHAMBER_ALARM_FIRE				132
#define COM_SIG_CHAMBER_ALARM_FIRE_COUNT		133
#define COM_SIG_CHAMBER_ALARM_SMOKE				134
#define COM_SIG_CHAMBER_ALARM_SMOKE_COUNT		135
#define COM_SIG_CHAMBER_ALARM_EMG				136
#define COM_SIG_CHAMBER_ALARM_EMG_COUNT			137		//csk_161028_e

#endif
