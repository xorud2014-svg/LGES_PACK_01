#ifndef __MODULECONTROL_H__
#define __MODULECONTROL_H__

void	ModuleControl(void);
void 	mSignalCheck(void);

void	mSignalCheck_Cable_Check(void);
void	mSignalCheck_Cell_Check(void);

void 	mSignalCheck_DC_FAN(void);
void 	mSignalCheck_FAN_RELAY(void);
void 	mSignalCheck_TOWER_LAMP(void);
void 	mSignalCheck_TOWER_LAMP1(void);
void 	mSignalCheck_TOWER_LAMP2(void);
void 	mSignalCheck_TOWER_LAMP3(void);
void 	mSignalCheck_TOWER_LAMP4(void);

void 	mSignalCheck_DAQ_ISOLATION_REPLY_DELAY(void);
void	mSignalCheck_DAQ_ISOLATION_REPLY_DELAY_2(void);
void 	mSignalCheck_PACK_ISOLATION_PHASE(void);

void 	mSignalCheck_BMS_EOL_PHASE(void);
void 	mSignalCheck_BMS_EOL_FLASH_PHASE(void);
void 	mSignalCheck_BMS_EOL3_PHASE(void);
void 	mSignalCheck_EOL_PROCEDURE(void);
#endif
