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

int		rcv_cmd_version_request(int);
int		rcv_cmd_version_request2(int);
int		rcv_cmd_set_group(int);
int		rcv_cmd_set_line_mode(int);
int		rcv_cmd_line_mode_request(int);
int		rcv_cmd_set_auto_report(int);
int		rcv_cmd_config_request(int);
int		rcv_cmd_set_config(int);
int		rcv_cmd_test_header(int);
int		rcv_cmd_test_precheck(int);
int		rcv_cmd_test_precheck_1(int);
int		rcv_cmd_test_precheck_2(int);
int		rcv_cmd_test_common_safety(int);
int		rcv_cmd_test_ng_condition(int);
int		rcv_cmd_test_step(int);
int		get_test_step_ocv(int);
int		get_test_step_charge(int);
int		get_test_step_discharge(int);
int		get_test_step_z(int);
int		get_test_step_rest(int);
int		get_test_step_long_time_rest(int);
int		get_test_step_end(int);
int		rcv_cmd_test_grade(int);
int		rcv_cmd_test_info_request(int);
int		rcv_cmd_set_sensor_limit(int);
int		rcv_cmd_tray_data(int);
int		rcv_cmd_chamber_set(int);
int		rcv_cmd_job_change_set(int);

int		rcv_cmd_check(int);
int		rcv_cmd_run(int);
int		rcv_cmd_stop(int);
int		rcv_cmd_pause(int);
int		rcv_cmd_continue(int);
int		rcv_cmd_next_step(int);
int		rcv_cmd_reset(int);
int		rcv_cmd_clear(int);
int		rcv_cmd_cali_meter_connect(int);
int		rcv_cmd_cali_set(void);
int		rcv_cmd_cali_voltage(int);
int		rcv_cmd_cali_current(int);
int		rcv_cmd_response(int);
int		rcv_cmd_comm_check_reply(int);
int		rcv_cmd_comm_check(int);
int		rcv_cmd_user_cmd(int);
int		user_cmd_run(int);
int		rcv_cmd_set_tray_ready(int);
int		user_cmd_stop(int);
int		user_cmd_pause(int);
int		user_cmd_continue(int);
int		user_cmd_reset(int);

//kjg_110712
int		rcv_cmd_set_cali_point(int);
int		rcv_cmd_get_main_cali_point(int);
int		rcv_cmd_get_ch_cali_point(int);
int		rcv_cmd_cali_start(int);
int		rcv_cmd_cali_check_start(int);
int		rcv_cmd_cali_stop(int);
int		rcv_cmd_cali_pause(int);
int		rcv_cmd_cali_resume(int);
int		rcv_cmd_cali_update(int);
int		rcv_cmd_real_measure_start(int);
int		rcv_cmd_real_measure_stop(int);
int		rcv_cmd_hw_map_req(int);
int		rcv_cmd_ch_attribute_set(int);
int		rcv_cmd_jig_temp_set_data(int);

int		send_cmd_version_data(char *);
int		send_cmd_version_data2(char *);
int		send_cmd_group_state(void);
int		send_cmd_config_data(char *);
int		send_cmd_response(char *, int);
int		send_cmd_trouble_code(int, int);
int		send_cmd_switch_data(int);
int		send_cmd_line_mode_data(char *);
int		send_cmd_test_info_data(char *);
int		send_cmd_ch_data(void);
int		send_cmd_ch_data_1(void);
int		send_cmd_ch_data_2(void);
int		send_cmd_step_data(void);
int		send_cmd_step_data_1(void);
int		send_cmd_step_data_2(void);
int		send_cmd_check_data(int, int, int);
int		send_cmd_check_data_1(int, int, int);
int		send_cmd_check_data_2(int, int, int);
int		send_cmd_check_data_3(int, int, int);
int		send_cmd_sensor_data(void);
int		send_cmd_chamber_data(void);
int		send_cmd_meter_connect_reply(char *);
int		send_cmd_comm_check_reply(char *);
int		send_cmd_comm_check(void);
int		send_cmd_bcr_info(int);
int		send_cmd_user_cmd(int);
int		send_cmd_dcir_real_time_data(void);
int		send_cmd_all_ng(void);

//kjg_110712
int		send_cmd_test_header(int);
int		send_cmd_test_step(int);
int		send_test_step_ocv(int, int);
int		send_test_step_rest(int, int);
int		send_test_step_charge(int, int);
int		send_test_step_discharge(int, int);
int		send_test_step_end(int, int);
int		send_cmd_test_recipe(int);
int		send_cmd_test_grade(int);
int		send_cmd_tray_data(int);

//kjg_110712
int		send_cmd_get_main_cali_point_reply(char *);
int		send_cmd_get_ch_cali_point_reply(char *);
int		send_cmd_cali_data(int, int);
int		send_cmd_cali_end(int, int);
int 	send_cmd_cali_check_data(int, int);
int		send_cmd_cali_check_end(int, int);
int		send_cmd_real_measure_data(int, int);
int		send_cmd_real_measure_end(int, int);
int		send_cmd_measure_data(void); //kjgw
int		send_cmd_hw_map_req_rpy(int);
int		send_cmd_ch_attribute_set_reply(int);

void	make_header(int, int, char *);
void	make_header2(int, int, char *, char *);
int		send_command(char *);
int		send_command2(char *, int);
int		get_reply_cmdid(char *);

int		Check_NetworkState(void);
void	network_ping(void);
int		check_network_timeout(void);
void	check_cmd_reply_timeout(void);

#endif
