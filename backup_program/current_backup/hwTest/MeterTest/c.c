
#include <time.h>
#include "comm.h"

/* ADEX Meter */
extern CommParam comm_param; 
extern char value[256];
static struct termios oldtrm;

int 
opentty ()
{
	int fd;
	struct termios trm;

	fd = open (comm_param.device, O_RDWR | O_NOCTTY);
	if (fd < 0){
		printf("Cannot Open tty0\n");
		perror("open");
		return -1;
	}

	tcgetattr (fd, &oldtrm);	/* salva il setup attuale della porta */
	bzero (&trm, sizeof (trm));
	switch (comm_param.bps) {
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
//	case TMT_B96:
//		trm.c_cflag = B9600 | CS8 | CREAD; break;
	case TMT_B96:
		trm.c_cflag = B9600 | CS7 | CREAD; break;
	case TMT_B192:
		trm.c_cflag = B19200 | CS8 | CREAD; break;
	}
	switch (comm_param.flow) {
	case TMT_FRTSCTS:
		trm.c_cflag |= CRTSCTS ; break;
	case TMT_FXONXOFF:
		trm.c_iflag |= IXON | IXOFF ; break;
	case TMT_FNONE:
		break;
	}
	trm.c_iflag |= IGNPAR;
	trm.c_cflag |= PARENB | CSTOPB;
	trm.c_oflag = 0;
	trm.c_oflag |= NLDLY;
	trm.c_lflag = 0;
	trm.c_cc[VTIME] = 0;
	trm.c_cc[VMIN] = 0;
	tcflush (fd, TCIFLUSH);
	tcsetattr (fd, TCSANOW, &trm);
	return fd;
}

int 
closetty (int fd)
{
	tcsetattr (fd, TCSANOW, &oldtrm);
	if (close (fd) < 0) {
			perror("close");
		return -1;
	} 
	return 0;
}

int 
send (int fd, char *msg, int len)
{
	if (!len) len = strlen (msg);
	if(write(fd, msg, len) == -1) {
		printf("Write error\n");
		perror ("write");
		return -1;
	}
	return 0;
}

int rxsettings (char* cmd)
{
    int fd, rtn, cnt;
	
    fd = opentty();
    if(fd < 0) {
		printf("Error: cannot open tty\n");
		return -1;
    }
    send(fd, cmd, 0); 
	tcdrain(fd);
	//usleep(500000);
	sleep(1);
    cnt = 0;
	do {
	   cnt++;	
	   if(cnt > 3) break;
		   
	   rtn = read(fd, value, sizeof(value));
	   if(rtn == -1){
	   		printf("Can not Meter Read\n");
			perror("read"); 
		}
	   if(value[rtn-1] == '\n') break;// ricevuto carattere End Of Line 
	   usleep(500);
	} while(1);
	closetty(fd);
    return 0;
}

int txsettings (char* buf)
{
	int fd;
	fd = opentty();
	if (fd < 0) {
		printf("Error: cannot open tty\n");
		return -1;
	}
	send(fd, buf, 0);
	//tcdrain (fd);
	//usleep(1000000);
	sleep(1);
	closetty (fd);
	return 0;
}

void 
delay(int sec)
{
	time_t t = time(NULL);
	while ( time(NULL) - t < sec ) { }
}
