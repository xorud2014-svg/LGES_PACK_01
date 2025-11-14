#ifndef __RTTASK_H__
#define __RTTASK_H__

void  	*rt_task(void *);
void	Exec_Slot_10ms_FD(int); //jhkw_190714
void	Exec_Slot_10ms(int);
void	Exec_Slot_25ms(int);
void	Exec_Slot_50ms(int);
void	Exec_Slot_100ms(int);
void	Exec_Slot_200ms(int);

int		init_module(void);
void	cleanup_module(void);
#endif
