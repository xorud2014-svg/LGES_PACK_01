#ifndef _MAIN_H_
#define _MAIN_H_

int		main(void);
int		Initialize(void);
void	DataBase_kill(void);
void	NvRam_kill(void);
void	InitUserlog(void);
void	InitSyslog(void);
int		Open_SharedMemory(void);
void	CloseClient(void);
void	SaveTmpData(void);
int		LoadTmpData(void);
void	PauseCommand(void);
#endif
