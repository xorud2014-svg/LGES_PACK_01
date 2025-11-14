#ifndef __CAN_H__
#define __CAN_H__

void	CAN_Control1(int);
int		rcv_can_data_clear(int, int, int, int);
int		rcv_can_data_parsing(int, int, int, int, S_RT_CAN_MSG *);
int		rcv_can_data_parsing_2(int, int, int, int, int);
void    can_signal_bms_eol_phase(int, int, int, int, S_RT_CAN_MSG *);
void    can_signal_bms_eol_flash_phase(int, int, int, int, S_RT_CAN_MSG *);
void    can_signal_bms_uds_phase(int, int, int, int, S_RT_CAN_MSG *);
void    can_signal_bms_uds_phase1(int, int, int, int, S_RT_CAN_MSG *);
void    can_signal_bms_procedure_50(int, int, int, int, S_RT_CAN_MSG *);
void    can_data_convert(int, int, int);

void    can_signal_eol_procedure(int, int, int, int, S_RT_CAN_MSG *);
void    can_kokam_bms_data_parsing(int, int); //kjhw_130228

void	CAN_Control2(int);
void	canSignalCheck(int);
void	canSignalCheck_0(int);
void	canSignalCheck_3(int);	//kjh_180821
void	canSignalCheck_50(int);
void	can_transmit1(int);
void	can_transmit2(int);
void	can_transmit3(int);

float	FindCanChangeData(int, int);
float	FindExternalCanData(int, short int);
float	FindCanChangeData_2(int, int, int); //kjhw_170911

#endif
