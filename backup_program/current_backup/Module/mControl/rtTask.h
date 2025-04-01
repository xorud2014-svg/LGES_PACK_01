#ifndef __RTTASK_H__
#define __RTTASK_H__

void	*rt_task(void *);
void	Exec_Slot_10ms_FD(int); //kjhw_190705
void	Exec_Slot_10ms(int);
void	Exec_Slot_25ms(int);
void	Exec_Slot_50ms(int);
void	Exec_Slot_100ms(int);
void	Exec_Slot_200ms(int);

int		init_module(void);
void	cleanup_module(void);

void	rt_can_control(void);	//csk_240301
void	spi_test_1a(int);		//csk_240301
void	spi_test_1b(int);		//csk_240301
void	spi_test_2(int);		//kjg_240503
void	spi_test_3(int);		//kjg_240503
#endif
