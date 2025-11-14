#ifndef _COMM1_H_
#define _COMM1_H_

int opentty1(void);
int closetty1(int);
int Send1(int, char *, int);
int rxsettings1(char *,int);
int txsettings1(char *,int);
#endif
