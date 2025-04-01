#ifndef _COMM_H_
#define _COMM_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


int opentty(void);
int closetty(int);
int Send(int, char *, int);
int rxsettings(char *,int);
int txsettings(char *,int);
#endif
