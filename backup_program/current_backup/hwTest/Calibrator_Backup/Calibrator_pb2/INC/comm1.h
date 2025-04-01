#ifndef _COMM1_H_
#define _COMM1_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef enum {
	TMT_FRTSCTS,
	TMT_FXONXOFF,
	TMT_FNONE
} Flow;

typedef enum {
	TMT_B3,
	TMT_B6,
	TMT_B12,
	TMT_B24,
	TMT_B48,
	TMT_B96,
	TMT_B192
} Bps;

typedef struct {
	char	device[11];
	Bps		bps;
	Flow	flow;
} CommParam;

int opentty1(void);
int closetty1(int);
int Send1(int, char *, int);
int rxsettings1(char *,int);
int txsettings1(char *,int);
#endif
