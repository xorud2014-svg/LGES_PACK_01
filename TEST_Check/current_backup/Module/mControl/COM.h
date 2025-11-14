#ifndef __COM_H__
#define __COM_H__

void	COM_Control_Receive(int);
void	COM_Control_Signal(int);
void	COM_Signal_Check(int);

int		SerialPacket_Receive(int);
void	SerialPacket_Parsing(int);
int		SerialCommand_Receive(int);
int		SerialCommand_Parsing(int);
int		Parsing_SerialEvent(int);

void	COM_Signal_Check_AGILENT_34401A(int);
int		CmdHeader_Check_AGILENT_34401A(int);
int		rcv_cmd_answer_AGILENT_34401A(int);
void	send_cmd_initialize_AGILENT_34401A(int, int);
void	send_cmd_request_AGILENT_34401A(int);

//jhkw_131209s
void    COM_Signal_Check_VITREK_4700(int);
int     CmdHeader_Check_VITREK_4700(int);
int     rcv_cmd_answer_VITREK_4700(int);
void    send_cmd_initialize_VITREK_4700(int, int);
void    send_cmd_request_VITREK_4700(int);
//jhkw_131209e

void	COM_Signal_Check_Display(int);
void	COM_Signal_Check_Display1(int);	//kjhw_121016
int		CmdHeader_Check_Display(int);
int		rcv_cmd_answer_Display(int);
void	send_cmd_value_Display(int, int, int, int);

void	COM_Signal_Check_CB_7018(int);
int		CmdHeader_Check_CB_7018(int);
int		rcv_cmd_answer_CB_7018(int);
void	send_cmd_initialize_CB_7018(int, int, int, int);
void	send_cmd_request_CB_7018(int, int, int);
void	send_cmd_close_CB_7018(int, int);

void	COM_Signal_Check_COMM_A_XL_122(int);
int		CmdHeader_Check_COMM_A_XL_122(int);
int		CheckSum_Check_COMM_A_XL_122(int);
int		rcv_cmd_answer_COMM_A_XL_122(int);
void	send_cmd_request_COMM_A_XL_122(int, int, int);

int		CmdHeader_Check_Calibrator(int);
int		CheckSum_Check_Calibrator(int);
int		rcv_cmd_answer_Calibrator(int);
void	send_cmd_on_off_Calibrator(int, int, int, int);
void	send_cmd_initialize_Calibrator(int, int);
void	send_cmd_request_Calibrator(int);

void	COM_Signal_Check_MS_860_RS_485(int);
int		rcv_cmd_answer_MS_860_RS_485(int);
void	send_cmd_request_MS_860_RS_485(int, unsigned char, unsigned char);
unsigned char	convert_addr_MS_860_RS_485(unsigned char, unsigned char);
void	send_cmd_ack_MS_860_RS_485(int, unsigned char);

void	COM_Signal_Check_MS_860_RS_232(int);
int		rcv_cmd_answer_MS_860_RS_232(int);
void	send_cmd_initialize_MS_860_RS_232(int, int, int);
void	send_cmd_request_MS_860_RS_232(int, int);

void	COM_Signal_Check_COMM_B_MS_860(int);
int		CmdHeader_Check_COMM_B_MS_860(int);
int		CheckSum_Check_COMM_B_MS_860(int);
int		rcv_cmd_answer_COMM_B_MS_860(int);
void	send_cmd_initialize_COMM_B_MS_860(int, int, int);
void	send_cmd_request_COMM_B_MS_860(int, int);

void	COM_Signal_Check_TEMP_880(int);
int		CmdHeader_Check_TEMP_880(int);
int		CheckSum_Check_TEMP_880(int);
int		rcv_cmd_answer_TEMP_880(int);
void	send_cmd_run_stop_TEMP_880(int, int);
void	send_cmd_temp_set_TEMP_880(int, int);
void	send_cmd_time_set_TEMP_880(int, int);
void	send_cmd_time_use_set_TEMP_880(int, int);
void	send_cmd_request_TEMP_880(int, int);

void	COM_Signal_Check_PLC_Q03UD(int);
int		CmdHeader_Check_PLC_Q03UD(int);
int		CheckSum_Check_PLC_Q03UD(int);
int		rcv_cmd_answer_PLC_Q03UD(int);
void	send_cmd_request_PLC_Q03UD(char *, char *, int, int);
void	send_cmd_response_PLC_Q03UD(int, int);
void	PLC_Fault_Check(int);

void	make_header(char *, char, int, int, int);
int		make_check_sum(char *, int);
int		make_check_sum_2(char *, int);
int		make_check_sum_3(char *, int);
int		send_command(char *, int, int, int);

int		Find_comPort(int);
#endif
