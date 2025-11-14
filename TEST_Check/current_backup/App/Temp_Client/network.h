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

int		send_cmd_heartbeat_req(void);
int 	send_cmd_temp_data_req(void);
int		send_cmd_response(int, int);

int		rcv_cmd_heartbeat_rpy(void);
int 	rcv_cmd_temp_data_rpy(void);
int		rcv_cmd_response(void);

int		send_command(char *, int);
void	make_header(char *, int, int);
void 	make_crc(unsigned short , char *, unsigned int);
int 	check_crc(unsigned short , char *, unsigned int); 
unsigned short crc16ccitt_compute_buf(unsigned short , char *, unsigned int);
unsigned short crc16polynomail_buf(unsigned short , char *, unsigned int);
unsigned short crc16modbus_compute_buf(unsigned short , char *, unsigned int);
#endif
