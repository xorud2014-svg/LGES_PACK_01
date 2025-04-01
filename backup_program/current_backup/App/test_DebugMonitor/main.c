#include <stdio.h>
#include <fcntl.h>
#include "../../INC/datastore_coa.h"
#include "main.h"

volatile S_SYSTEM_DATA *myData;

int main(void)
{
    int	retval;
    struct timeval tv;
    fd_set rfds;

    while(1) {
	    tv.tv_sec = 0;
	    tv.tv_usec = 800000;
	    //tv.tv_usec = 300000;
	    FD_ZERO(&rfds);
	    FD_SET(0, &rfds);
		retval = select(1, &rfds, NULL, NULL, &tv);
	    if(retval == 0) {
			printf("rtn111 %d\n", retval);
			printf("code = %d\n", myData->mData.code);
		//	printf("signal = %d\n", myData->mData.signal[M_SIG_EXIT_PHASE]);
		//	printf("signal2 = %d\n", myData->mData.signal[M_SIG_EXIT_TYPE]);
			/*myData->mData.code = M_CD_FAULT_NORMAL_TERMINAL_QUIT;
			myData->mData.signal[M_SIG_EXIT_PHASE] = P1;
			myData->mData.signal[M_SIG_EXIT_TYPE] = P1;*/
			break;
		} else {
			printf("rtn %d\n", retval);
		}
    }

    return 0;
}
