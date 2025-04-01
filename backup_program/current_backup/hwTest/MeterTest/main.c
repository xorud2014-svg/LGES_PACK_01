#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <stdlib.h>
#include "comm.h"
#include "../../INC/datastore.h"
#include "main.h"

//S_SYSTEM_DATA *myData;
CommParam comm_param = { "/dev/ttyS0", TMT_B96, TMT_FXONXOFF};

/* ADEX Meter */
char value[256];
long z, acZ;

int main(void)
{
	int fd, rtn, sig, meas;
	struct timeval tv;
	fd_set rfds;

/*	if((fd = open("/dev/mem", O_RDWR)) < 0) {
		printf("Can not open shared memory(Meter)\n");
		return 0;
	}
	myData = (S_SYSTEM_DATA *) mmap(0, sizeof(S_SYSTEM_DATA),
		PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, BASE_ADDRESS); 
   	if(myData == ((S_SYSTEM_DATA *)-1)) {
	   	printf("Can not create shared memory(Meter)\n");
	   	close(fd);
	   	return 0;
   	}
   	close(fd);
  */ 	
	fd = 0;
   	sig = PHASE1;
	meas = PHASE1;
	while(sig == PHASE1) {
		tv.tv_sec = 0;
		tv.tv_usec = 500000; //500ms wkjg
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		
		rtn = select(0, &rfds, NULL, NULL, &tv);
		if(rtn == 0) {
				fd++;
				if(fd > 10) fd = 10;
				if(fd == 1) {
					printf("kjg1\n");
					txsettings("\x1B\x52\x0D\x0A\n", 0);
   					txsettings("R2\x0D\x0A\n", 0);
   					txsettings("W0\x0D\x0A\n", 0);
   					txsettings("T0\x0D\x0A\n", 0);
   					txsettings("E\x0D\x0A\n", 0);
					printf("kjg2\n");
					meas = PHASE2;
				}
			if(meas == PHASE2) {
				printf("kjg3\n");
				acZ = SaveData();
				meas = PHASE3;
				sig = PHASE2;
			}
		}
	} //end while
	return 0;
}

int Conversion(int dvn)
{
    int tmp;
    switch(dvn) {
       case 1:
		   tmp = CharToInt(value[1]);
		   z = tmp * 100000000L;
		   tmp = CharToInt(value[2]);
		   z += tmp * 10000000L;
		   tmp = CharToInt(value[3]);
		   z += tmp * 1000000L;
		   tmp = CharToInt(value[4]);
		   z += tmp * 100000L;
		   break;
       case 2:
		   tmp = CharToInt(value[1]);
		   z = tmp * 10000000L;
		   tmp = CharToInt(value[2]);
		   z += tmp * 1000000L;
		   tmp = CharToInt(value[3]);
		   z += tmp * 100000L;
		   tmp = CharToInt(value[4]);
		   z += tmp * 10000L;
		   break;
       case 3:
		   tmp = CharToInt(value[1]);
		   z = tmp * 1000000L;
		   tmp = CharToInt(value[2]);
		   z += tmp * 100000L;
		   tmp = CharToInt(value[3]);
		   z += tmp * 10000L;
		   tmp = CharToInt(value[4]);
		   z += tmp * 1000L;
		   break;
       case 4:
		   tmp = CharToInt(value[1]);
		   z = tmp * 100000L;
		   tmp = CharToInt(value[2]);
		   z += tmp * 10000L;
		   tmp = CharToInt(value[3]);
		   z += tmp * 1000L;
		   tmp = CharToInt(value[4]);
		   z += tmp * 100L;
		   break;
       case 5:
		   tmp = CharToInt(value[1]);
		   z = tmp * 10000L;
		   tmp = CharToInt(value[2]);
		   z += tmp * 1000L;
		   tmp = CharToInt(value[3]);
		   z += tmp * 100L;
		   tmp = CharToInt(value[4]);
		   z += tmp * 10L;
		   break;
       default: dvn=-1; break;
   }
   return dvn;
}

int CharToInt(char cmp)
{
    int integer;
    char character[10]={'0','1','2','3','4','5','6','7','8','9'};
    
    for(integer=0; integer < 10; integer++) {
		if(cmp == character[integer]) break;	
    }	
    return integer;
}

long SaveData(void)
{
	int rtn;
	txsettings("E\x0D\x0A\n", 0);	//Start Measurement
   	usleep(100000);
	rtn = rxsettings("\x1B\x44\x0D\x0A\n", 0);	// Set Remote Mode (Must assert)
    
	if(value[0]=='X' && value[5]=='E' && value[6]=='-') {
		switch(value[7]) {
			case '1':	rtn = Conversion(1);	break;
			case '2':	rtn = Conversion(2);	break;
			case '3':	rtn = Conversion(3);	break;
			case '4':	rtn = Conversion(4);	break;
			case '5':	rtn = Conversion(5);	break;
			default:	z = 0;	break;
		}
	} else z = 0;
	printf("kjg %ld : ", z);
	for(rtn = 0; rtn < 7; rtn++) printf(" %c", value[rtn]);
	printf("\n");
	return z;
}	
