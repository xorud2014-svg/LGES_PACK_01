#ifndef __LOCAL_MESSAGE_H__
#define __LOCAL_MESSAGE_H__

#include "common_message.h"

void	Check_Message(int);
int		msgParsing(int);
void	msgParsing_App_to_Module(int, int, S_MSG_VAL *);
void	msgParsing_DataSave_to_Module(int, int, S_MSG_VAL *);
void	msgParsing_COA_to_Module(int, int, S_MSG_VAL *);
void	msgParsing_COB_to_Module(int, int, S_MSG_VAL *);
void	msgParsing_COC_to_Module(int, int, S_MSG_VAL *);
void	msgParsing_IO_to_Module(int, int, S_MSG_VAL *);
void	msgParsing_Meter_to_Module(int, int, S_MSG_VAL *);

void	msgParsing_JigM_to_Module(int, int, S_MSG_VAL *);
void	msgParsing_App_to_JigM(int, int, S_MSG_VAL *);
void	msgParsing_Module_to_JigM(int, int, S_MSG_VAL *);
void	msgParsing_JigC_to_JigM(int, int, S_MSG_VAL *);

void	msgParsing_App_to_IO(int, int, S_MSG_VAL *);
void	msgParsing_Update_to_IO(int, int, S_MSG_VAL *);	//ktg_231031
void	msgParsing_Module_to_IO(int, int, S_MSG_VAL *);
void	msgParsing_JigC_to_IO(int, int, S_MSG_VAL *);

void	msgParsing_App_to_Meter(int, int, S_MSG_VAL *);
void	msgParsing_COA_to_Meter(int, int, S_MSG_VAL *);
void	msgParsing_COC_to_Meter(int, int, S_MSG_VAL *);
void	msgParsing_Module_to_Meter(int, int, S_MSG_VAL *);

void	msgParsing_App_to_Meter2(int, int, S_MSG_VAL *);

void	send_save_msg(int);
void	send_save_msg_2(int, int);
void	send_save_msg_3(int, int);

void	send_record_msg(int, int);
void	send_record_msg_1(int, int, int);

void	send_pulse_msg(int, int);
void	send_pulse_msg_1(int, int, int);

void	send_save_msg_formation(int, int);
void	send_save_msg_formation_1(int, int, int);
void	send_save_msg_formation_2(int, int, int);
void	send_save_msg_formation_3(int, int, int);

void	send_save_realData_msg(int);
void	send_save_realData_msg_Formation_A(int, int);
void	send_save_realData_msg_Formation_B(int, int);
void	send_save_realData_msg_Formation_C(int, int);
#endif
