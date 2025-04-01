#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"
#include "local_utils.h"

extern volatile S_SYSTEM_DATA	*myData;
extern volatile S_DATA_SAVE		*myPs;
extern char psName[PROCESS_NAME_SIZE];

void Init_SystemMemory(void)
{
	memset((char *)&psName[0], 0, PROCESS_NAME_SIZE);
	strcpy(psName, "UpdateProcess");

	memset((char *)&myPs->signal, 0, MAX_SIGNAL);

	myPs->misc.processPointer = (int)&myData;
}


