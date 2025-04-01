#ifndef __NETWORK_H__
#define __NETWORK_H__

int		InitNetwork(void);
int		NetworkPacket_Receive(void);
void	NetworkPacket_Parsing(void);
int		NetworkCommand_Receive(void);
int		NetworkCommand_Parsing(void);
int		Parsing_NetworkEvent(void);

int		CmdHeader_Check(char *);
int		Check_ReplyCmd(char *);

int		rcv_cmd_module_info_request(void);
int		rcv_cmd_aux_info_request(void);
int		rcv_cmd_can_receive_info_request(void);
int		rcv_cmd_can_transmit_info_request(void);
int		rcv_cmd_module_set_data(void);
int		rcv_cmd_ch_attribute_set(void);
int		rcv_cmd_aux_set(void);
int		rcv_cmd_can_receive_set(void);
int		rcv_cmd_can_transmit_set(void);
int		rcv_cmd_ch_attribute_request(void);
int		rcv_cmd_cali_meter_connect(void);
int		rcv_cmd_cali_start(void);
int		rcv_cmd_cali_update(void);
int		rcv_cmd_response(void);
int		rcv_cmd_run(void);
int		rcv_cmd_stop(void);
int		rcv_cmd_pause(void);
int		rcv_cmd_continue(void);
int		rcv_cmd_next_step(void);
int		rcv_cmd_chamber_flag(void);
int		rcv_cmd_chamber_continue(void);
int		rcv_cmd_cable_check(void);
int		rcv_cmd_cell_check(void);
int		rcv_cmd_cycle_continue(void);
int		rcv_cmd_init(void);
int		rcv_cmd_reset(void);
int		rcv_cmd_testcond_start(void);
int		rcv_cmd_testcond_safety(void);
int		rcv_cmd_testcond_step(void);
int		rcv_cmd_testcond_end(void);
int		rcv_cmd_step_cond_request(void);
int		rcv_cmd_step_cond_update(void);
int		rcv_cmd_safety_cond_request(void);
int		rcv_cmd_safety_cond_update(void);
int		rcv_cmd_reset_reserved_cmd(void);
int		rcv_cmd_sensor_limit_set(void);
int		rcv_cmd_comm_check_reply(void);
int		rcv_cmd_comm_check(void);
int		rcv_cmd_testcond_pattern(void);
int		rcv_cmd_real_time_reply(void);
int		rcv_cmd_bms_comm_request(void);
int		rcv_cmd_daq_isolation_request(void);
int		rcv_cmd_set_measure_data(void);

int		send_cmd_response(char *, int);
int		send_cmd_module_info_reply(void);
int		send_cmd_aux_info_reply(void);
int		send_cmd_can_receive_info_reply(void);
int		send_cmd_can_transmit_info_reply(void);
int		send_cmd_ch_attribute_reply(void);

//void	send_cmd_ch_data(void);
//void	send_monitoring_data(int);
//void	send_save_data(int);

void	send_cmd_monitor_data(void);
int		send_cmd_ch_data(void);
//int		send_cmd_sensor_data(void);
//int		send_cmd_ch_pulse_data(int);
//void	send_cmd_ch_pulse_data_2(int, int);
//int		send_cmd_fadm_pulse_data(int);
//void	send_cmd_fadm_pulse_data_2(int, int);
int		send_cmd_record_data(void);

int		send_cmd_step_cond_reply(void);
int		send_cmd_safety_cond_reply(void);
void	send_cmd_meter_connect_reply(void);
void	send_cmd_cali_start_reply(int, int);
void	send_cmd_cali_normal_result(int);
void	send_cmd_cali_check_result(int);
void	send_cmd_comm_check_reply(void);
void	send_cmd_comm_check(void);
void	send_cmd_trouble_code(int, int);
void	send_cmd_unknown(int, int);
void	send_cmd_real_time_request(void);
void	send_cmd_bms_comm_reply(int, int);
void	send_cmd_bms_comm_end(int, int);
void	send_cmd_daq_isolation_reply(int);
int		send_cmd_set_measure_data(int);
void	send_cmd_cable_check_reply(void);
void	send_cmd_cell_check_reply(void);

void	make_header(char *, char, int, int, int);
void	make_header_2(char *, char, int, int, int, int);
int		send_command(char *, int, int);
int		get_reply_cmdid(char *);

int		Check_NetworkState(void);
void	network_ping(void);
void	check_cmd_reply_timeout(void);
int		check_network_timeout(void);

#endif
