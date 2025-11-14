#ifndef _COMM_H_
#define _COMM_H_

//#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define DATA_START        1
#define DATA_STOP         2500
#define DATA_LENGHT       2500
#define BYTES_TO_RECEIVE  2500

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
	char device[11];
	Bps   bps;
	Flow  flow;
} CommParam;

int opentty (void);
int closetty (int fd);
int send (int fd, char *msg, int len);
int rxsettings (char*, int);
int txsettings (char*, int);
void delay (int sec);
#endif
