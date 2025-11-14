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
	//0 : PNE_PanelMeter_A
	//1 : PNE_PanelMeter_B
#define COM_FUNC_TYPE_METER2					3
	//0 : CB-7018
	//1 : XL-122
	//2 : COMM_A(XL-122)
	//3 : I-7018Z
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

//readType
#define READ_V_V								0
#define READ_V_I								1

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

#endif
