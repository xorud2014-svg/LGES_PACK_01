#ifndef __RT_CAN_EXT_H__
#define __RT_CAN_EXT_H__

extern int	rt_can_write(int, const char *);
extern int	rt_can_read(int, char *);

extern int	rt_can_set_param_0(int, int, int, int, int, int, int, int);
extern int	rt_can_set_param_11(int);
extern int	rt_can_check_status(int);

extern int	init_rt_can(void);
extern void	cleanup_rt_can(void);

extern int	rx_can_data_11(int);
extern int	tx_can_data_11(int);

//shh_250114s
void printCanLogic(unsigned char, unsigned char *, int);
void checkCanLogic(unsigned char *, int);
//shh_250114e


#endif
