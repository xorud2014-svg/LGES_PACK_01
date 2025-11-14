#ifndef __RT_CAN_EXT_H__
#define __RT_CAN_EXT_H__

extern int	rt_can_write(int, const char *);
extern int  rt_can_read(int, char *);

extern int  rt_can_set_param(int, int, int, int, int, int, int, int);
extern int	rt_can_check_status(int);
extern int  rt_can_setup(int, int);

#endif
