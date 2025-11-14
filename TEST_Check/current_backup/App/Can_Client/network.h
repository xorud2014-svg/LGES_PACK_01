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

void	send_cmd_ch_data(void);
int		send_cmd_pack_cycler_data(void);
int 	send_cmd_can_receive_set(int);
int 	send_cmd_can_transmit_set(int);
int		send_cmd_response(int, int);
int		send_cmd_heartbeat_reply(void);

int		rcv_cmd_heartbeat_req(void);
int		rcv_cmd_response(void);
int		rcv_cmd_standby_req(void);
int		rcv_cmd_can_receive_set_req(void);
int		rcv_cmd_can_transmit_set_req(void);
int 	rcv_cmd_receive_data(void);

int		send_command(char *, int);
void	make_header(char *, int, int, int);
#endif
