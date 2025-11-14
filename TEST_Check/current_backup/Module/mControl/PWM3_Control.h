#ifndef __PWM3_CONTROL_H__
#define __PWM3_CONTROL_H__

void	PWM3_Control(int);
void	PWM3_Write(int, int);
void	PWM3_Read(int, int);
void	PWM3_send_cali_data(int);
void	PWM3_CalChAverage(int);

#endif
