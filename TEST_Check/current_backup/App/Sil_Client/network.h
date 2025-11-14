#ifndef __NETWORK_H__
#define __NETWORK_H__

int		InitNetwork(void);
int		NetworkPacket_Receive(void);
void	NetworkPacket_Parsing(void);
int		NetworkCommand_Receive(void);
int		NetworkCommand_Parsing(void);
int		Parsing_NetworkEvent(void);

int		CmdHeaderCheck(char *);
int		CheckReplyCmd(char *);

int 	Check_NetworkState(void);
int 	check_network_ping(void);
int 	network_ping(void);

int 	send_cmd_daq_start(int);
int 	send_cmd_daq_stop(int);
int		send_cmd_response(int, int);
int		send_cmd_module_info(void);
int		send_cmd_heartbeat_reply(void);

int		rcv_cmd_heartbeat_req(void);
int 	rcv_cmd_daq_data_rpy(void);
int		rcv_cmd_response(void);

int		send_command(char *, int);
void	make_header(char *, int, int);
#endif
