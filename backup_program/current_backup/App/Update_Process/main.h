#ifndef __MAIN_H__
#define __MAIN_H__

int 	main(void);
int		Initialize(void);
void	Update_Process_Control(void);
int		Update_file_state_check(void);
int		RollBack_file_state_check(void);
void	Update_Execute(void);
void	RollBack_Execute(void);


void	Close_Process(void);
#endif
