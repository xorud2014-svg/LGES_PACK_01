#ifndef _COMM_H_
#define _COMM_H_

int opentty(void);
int closetty(int);
int Send(int, char *, int);
int rxsettings(char *,int);
int txsettings(char *,int);
#endif
