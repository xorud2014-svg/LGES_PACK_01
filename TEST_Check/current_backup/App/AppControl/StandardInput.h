#ifndef __STANDARDINPUT_H__
#define __STANDARDINPUT_H__

void	StandardInput_Receive(void);
int		WaitKeyInput(void);
int		Parsing_StandardInput(char *);
void	UserCommand_Parsing(int, int);
int		SysProcessing(int, int);
int		ExecProcessing(int, int);
int		CaliProcessing(int, int);
int		AnalogProcessing(int, int);
int		ComProcessing(int, int);
int		CanProcessing(int, int);
int		COAProcessing(int, int);
int		COBProcessing(int, int);
#endif
