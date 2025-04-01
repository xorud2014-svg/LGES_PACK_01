
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "comm1.h"

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

extern CommParam comm_param1; 
extern char comm_value[256];
static struct termios oldtrm1;
CommParam comm_param1 = {"/dev/ttyS0", TMT_B96, TMT_FXONXOFF};  //Meter(34401A)

int opentty1(void)
{
    int fd;
    struct termios trm;
	
    fd = open(comm_param1.device, O_RDWR | O_NOCTTY);
    if(fd < 0) {
		perror("open");
		return -1;
    }
    tcgetattr(fd, &oldtrm1);	
    bzero(&trm, sizeof(trm));
    switch(comm_param1.bps) {
		case TMT_B3:
			trm.c_cflag = B300 | CS8 | CREAD; break;
		case TMT_B6:
			trm.c_cflag = B600 | CS8 | CREAD; break;
		case TMT_B12:
			trm.c_cflag = B1200 | CS8 | CREAD; break;
		case TMT_B24:
			trm.c_cflag = B2400 | CS8 | CREAD; break;
		case TMT_B48:
			trm.c_cflag = B4800 | CS8 | CREAD; break;
		case TMT_B96:
			trm.c_cflag = B9600 | CS7 | CSTOPB | PARENB | CREAD; break;
		case TMT_B192:
			trm.c_cflag = B19200 | CS8 | CREAD; break;
    }
    switch(comm_param1.flow) {
		case TMT_FRTSCTS:
			trm.c_cflag |= CRTSCTS; break;
		case TMT_FXONXOFF:
			trm.c_iflag |= IXON | IXOFF; break;
		case TMT_FNONE:
			break;
    }
    trm.c_iflag |= IGNPAR;
    trm.c_oflag = 0;
    trm.c_oflag |= NLDLY;
    trm.c_lflag = 0;
    trm.c_cc[VTIME] = 0;
    trm.c_cc[VMIN] = 0;
    tcflush (fd, TCIFLUSH);
    tcsetattr (fd, TCSANOW, &trm);
    return fd;
}

int closetty1(int fd)
{
    tcsetattr (fd, TCSANOW, &oldtrm1);
    if(close(fd) < 0) {
	perror("close");
	return -1;
    } 
    return 0;
}

int Send1(int fd, char *msg, int len)
{
    if(!len) len = strlen(msg);
    if(write(fd, msg, len) == -1) {
		perror("write");
		return -1;
    }
    return 0;
}

int rxsettings1(char *buf,int length)
{
    int fd, rtn, MaxCnt=0;
	
    fd = opentty1();
    if(fd < 0) {
		printf("Error: cannot open tty\n");
		return -1;
    }
    Send1(fd, buf, length);
	tcdrain(fd);
	memset(comm_value, 0x00, sizeof comm_value);
	do {
		rtn = read(fd, comm_value, sizeof(comm_value));
		if(rtn == -1) {
			perror("read"); 
		} else if(comm_value[rtn - 1] == '\n') {
			rtn = 0;
			break; 
		}

		MaxCnt++;
	    if(MaxCnt >= 1000) {
			rtn = -2;
			printf("Meter Read Error\n");
			break;
		}
		//usleep(1000);
		usleep(20000);
	} while(rtn<=0);
    closetty1(fd);
    return rtn;
}

int txsettings1(char *buf,int length)
{
    int fd;

    fd = opentty1();
    if(fd < 0) {
		printf("Error: cannot open tty\n");
		return -1;
    }

    Send1(fd, buf, length);
    tcdrain(fd);
	//usleep(3000);
	usleep(20000);
    closetty1(fd);
    return 0;
}
