#ifndef __RT_CAN_EXT_H__
#define __RT_CAN_EXT_H__

extern int	rt_can_write(int, const char *);
extern int  rt_can_read(int, char *);

extern int  rt_can_set_param(int, int, int, int, int, int, int, int);
extern int	rt_can_check_status(int);
extern int  rt_can_setup(int, int);

extern int init_rt_can(void); //kjg_181121
extern void cleanup_rt_can(void); //kjg_181121

extern unsigned int rt_can_fd_rx(void); //kjg_181121
extern void rt_can_fd_rx2(unsigned int); //kjg_181121
extern int rt_can_fd_rx3(unsigned int);
extern int rt_can_fd_rx4(unsigned int);
extern void tx_can_data_2(int); //kjg_181121
extern void tx_can_data_3(int);
#endif
