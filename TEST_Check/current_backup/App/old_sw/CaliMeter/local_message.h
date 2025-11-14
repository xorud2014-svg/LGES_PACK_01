#ifndef __LOCAL_MESSAGE_H__
#define __LOCAL_MESSAGE_H__

#include "common_message.h"

void	Check_Message(void);
void	msgParsing(int);
void	msgParsing_App_to_Meter(int, int, S_MSG_VAL *);
void	msgParsing_COA_to_Meter(int, int, S_MSG_VAL *);
void	msgParsing_Module_to_Meter(int, int, S_MSG_VAL *);
#endif
