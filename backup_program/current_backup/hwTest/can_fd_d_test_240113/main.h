#ifndef _MAIN_H_
#define _MAIN_H_

int exec_cmd1(int); //get parameter
int exec_reset(int); //chip command(reset)
int exec_cmd2(int); //set parameter
int exec_cmd3(int); //get status
int exec_t1_wr(int);
int exec_t1_rd(int);
unsigned short CHIP_CalculateCRC16(unsigned char *, unsigned short);
int exec_t2_wr(int);
int exec_t2_rd(int);
int exec_cmd_error_count_state_get(int);
int exec_cmd_bus_diagnostics_get(int);
int exec_cmd_bus_diagnostics_clear(int);
void exec_T3(void);
int main(void);
#endif
