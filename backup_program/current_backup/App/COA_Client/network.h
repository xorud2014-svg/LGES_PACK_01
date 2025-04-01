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
int		rcv_cmd_ch_can_receive_set(void);
int		rcv_cmd_ch_can_transmit_set(void);
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
int		rcv_cmd_goto_step(void); //kjhw_151211
int		rcv_cmd_chamber_flag(void);
int		rcv_cmd_chamber_continue(void);
int		rcv_cmd_cable_check(void);
int		rcv_cmd_cell_check(void);
int		rcv_cmd_cycle_continue(void);
int		rcv_cmd_can_comm_check(void); //kjhw_141201
int		rcv_cmd_out_mux_use(void); //kjhw_141201
int		rcv_cmd_reset(void);
int		rcv_cmd_testcond_start(void);
int		rcv_cmd_testcond_safety(void);
int		rcv_cmd_testcond_step(void);
int		rcv_cmd_testcond_end(void);
int		rcv_cmd_step_cond_request(void);
int		rcv_cmd_step_cond_update(void);
int		rcv_cmd_common_safety_cond_request(void);
int		rcv_cmd_common_safety_cond_update(void);
int		rcv_cmd_reset_reserved_cmd(void);
int		rcv_cmd_testcond_update_start(void);
int		rcv_cmd_testcond_update_end(void);
int		rcv_cmd_testcond_pattern_update(void);
int		rcv_cmd_sensor_limit_set(void);
int		rcv_cmd_comm_check_reply(void);
int		rcv_cmd_comm_check(void);
int		rcv_cmd_testcond_pattern(void);
int		rcv_cmd_real_time_reply(void);
int		rcv_cmd_bms_comm_request(void);
int		rcv_cmd_can_transmit_change(void);
int		rcv_cmd_bms_eol_data1_request(void);
int		rcv_cmd_bms_eol_data2_request(void);
int		rcv_cmd_bms_eol_pack_id_write(void);
int		rcv_cmd_bms_eol_pack_id_check(void);
int		rcv_cmd_bms_eol_hi_pot_test(void);
int		rcv_cmd_uds_vbf_info_request(void);
int		rcv_cmd_uds_vbf_change_request(void);
int		rcv_cmd_bms_eol_data3_request(void);
int		rcv_cmd_bms_eol_data3_led_request(void);
int		rcv_cmd_bms_eol_cvtn_id_request(void);
int		rcv_cmd_bms_eol_dtc_clear(void);
int		rcv_cmd_bms_eol_reset_micro(void);
int		rcv_cmd_bms_eol_3p_system_request(void);
int		rcv_cmd_daq_isolation_request(void);
int		rcv_cmd_daq_isolation_request2(void);	//jhkw_150224
int		rcv_cmd_out_mux_select_request(void);	//kjhw_151021
int		rcv_cmd_can_or_lintocan_set(void);	//kjhw_181111
int		rcv_cmd_th_table_set(void);	//kjhw_181111
int		rcv_cmd_humidity_table_set(void);	//csk_200207
int		rcv_cmd_can_th_table_set(void);	//ktg_220614
int		rcv_cmd_eol_procedure_request(void);
int		rcv_cmd_bms_procedure_request(void);
int		rcv_cmd_calimode_request(void);
int		rcv_cmd_cali_temp_request(void);			//khj_210802
int		rcv_cmd_load_type_set(void);
int		rcv_cmd_ch_io_set(void);
int		rcv_cmd_buzzer_stop(void);
int		rcv_cmd_alarm_reset(void);
int		rcv_cmd_chamber_vent_stop(void); //kjhw_190820
int		rcv_cmd_cap_bank_set(void); //kjh_191014
int		rcv_cmd_gui_emg_set(void);	//khj_201227s

int		send_cmd_response(char *, int);
int		send_cmd_test_cond_conversion_end(int);
int		send_cmd_test_cond_conversion_update_end(int);
int		send_cmd_module_info_reply(int);
int		send_cmd_aux_info_reply(void);
int		send_cmd_can_receive_info_reply(void);
int		send_cmd_can_transmit_info_reply(void);
int		send_cmd_ch_attribute_reply(void);

void	send_cmd_ch_data2(void);
void	send_monitoring_data(int);
void	send_save_data(int);
void	send_cmd_pulse_data(void);

int		send_cmd_step_cond_reply(char *, int, int);
int		send_cmd_common_safety_cond_reply(char *, int);
void	send_cmd_meter_connect_reply(void);
void	send_cmd_cali_start_reply(int, int);
void	send_cmd_cali_normal_result(int);
void	send_cmd_cali_normal_result_1(int, int); //kjhw_120504 Vref x 2
void	send_cmd_cali_check_result(int);
void	send_cmd_comm_check_reply(void);
void	send_cmd_comm_check(void);
void	send_cmd_trouble_code(int, int, int);
void	send_cmd_unknown(int, int);
void	send_cmd_real_time_request(void);
void	send_cmd_bms_comm_reply(int, int);
void	send_cmd_bms_comm_end(int, int);
void	send_cmd_bms_eol_data1_reply(void);
void	send_cmd_bms_eol_data2_reply(void);
void	send_cmd_bms_eol_pack_id_write_reply(int);
void	send_cmd_bms_eol_pack_id_check_reply(int);
void	send_cmd_bms_eol_hi_pot_test_reply(int, int);
void	send_cmd_uds_vbf_info_reply(void);
void	send_cmd_uds_vbf_change_reply(int, char *);
void	send_cmd_bms_eol_data3_reply(void);
void	send_cmd_bms_eol_cvtn_id_reply(void);
void	send_cmd_bms_eol_3p_system_reply(void);
void	send_cmd_daq_isolation_reply(int, int);
void	send_cmd_daq_isolation_reply2(int, int, int);	//jhkw_150224
void	send_cmd_eol_procedure_reply(int);
void	send_cmd_bms_procedure_reply(int, int, int, int);
void	send_cmd_out_mux_select_reply(int,int);	//kjhw_151021
void	send_cmd_timesch_coa_next_step(int);	//kjh_160418
void	send_cmd_th_table_set_reply(int);	//kjhw_181111
void	send_cmd_humidity_table_set_reply(int);	//csk_200207
void	send_cmd_can_th_table_set_reply(int);	//ktg_220614
void	send_cmd_cali_temp_reply(int);			//khj_210802

void	make_header(char *, char, int, int, int);
void	make_header_2(char *, char, int, int, int, int);
int		send_command(char *, int, int, int);
int		get_reply_cmdid(char *);

int		Check_NetworkState(void);
void	network_ping(void);
void	check_cmd_reply_timeout(void);
int		check_network_timeout(void);

#endif
