#ifndef __LOCAL_MESSAGE_H__
#define __LOCAL_MESSAGE_H__

void	Check_Message(void);
void	msgParsing(int);
void	msgParsing_App_to_COC(int, int, S_MSG_VAL *);
void	msgParsing_Module_to_COC(int, int, S_MSG_VAL *);
void	msgParsing_DataSave_to_COC(int, int, S_MSG_VAL *);
void	msgParsing_Meter_to_COC(int, int, S_MSG_VAL *);
#endif
