//////////////////////////////////////////////////////////////////////
////	Library	: log writer
////	Date	: 2007-09-29
////	Composer: Jun-Gu Kwon
////	Comments: Write normal/error log to file/stdout
//////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>
#include "../../INC/datastore.h"
#include "userlog.h"

extern volatile S_SYSTEM_DATA *myData;

void Init_Logfile(char *fname, int type)
{
	char path[256], pathname[256], file[32], filename[32], *token;
	
	memset(path, 0, sizeof path);
	strcpy(path, "/root/");
	strcat(path, (char *)&myData->AppControl.misc.path1);
	strcat(path, "/log");
	
	memset(pathname, 0, sizeof pathname);
	token = strtok(path, " \t\n\r");
	if(token == NULL) strcpy(pathname, "./");
	else strcpy(pathname, token);
	
	memset(file, 0, sizeof file);
	strcpy(file, fname);

	memset(filename, 0, sizeof filename);
	token = strtok(file, " \t\n\r");
	if(token == NULL) strcpy(filename, "null");
	else strcpy(filename, token);

	memset((char *)&myData->log[type].LogPath, 0, 256);
	strcpy((char *)&myData->log[type].LogPath, pathname);
	
	memset((char *)&myData->log[type].LogFile, 0, 256);
	strcpy((char *)&myData->log[type].LogFile, filename);

	if(myData->log[type].LogFlag != LOG_OFF) {
    	logopen(pathname, filename, LOG_FNAME_DATE, type);
	}
}

/////////////////////////////////////////////////////////////
////	Declaration	: Set log-filename
////	Description	: You can manipulate log-filepath
////	  adding absolute path.
/////////////////////////////////////////////////////////////
void logopen(char *pathname, char *filename, int postfix, int type)
{
	int 	rtn, i;
	char	LogFile[256], buf[256];
	struct	tm *tm;
	time_t	t;

	time(&t);
	tm = localtime(&t);
	if(postfix) {
		memset(LogFile, 0, sizeof LogFile);
		sprintf(LogFile, "%s/%s%02d%02d", pathname, filename,
			tm->tm_mon+1, tm->tm_mday);
	} else {
		memset(LogFile, 0, sizeof LogFile);
		sprintf(LogFile, "%s/%s", pathname, filename);
	}
	strcat(LogFile, ".log");

	rtn = strcmp((char *)&myData->log[type].OpenLogFile, LogFile);
	if(rtn != 0) {//is not equal
		if(myData->log[type].OpenLogFile[0] == 0) {
			memset((char *)&myData->log[type].OpenLogFile, 0, 256);
			strcpy((char *)&myData->log[type].OpenLogFile, LogFile);
		} else {
			for(i=1; i <= 12; i++) {
				memset(buf, 0, sizeof buf);
				sprintf(buf, "rm -rf %s/%s%02d%02d.log",
					pathname, filename, i, tm->tm_mday);
				system(buf);
			}
			memset((char *)&myData->log[type].OpenLogFile, 0, 256);
			strcpy((char *)&myData->log[type].OpenLogFile, LogFile);
		}
	}
}

/////////////////////////////////////////////////////////////////////////
////	Declaration	: Write the message to destination
////	  (file|stdout)
////	Description	: The arguments and usage is equal
////	  to th function "printf".
////	  Only one difference is the prefix of datetime by each call.
/////////////////////////////////////////////////////////////////////////
void userlog(int type, char *name, char *fmt, ...)
{
	char	*asciitime, tmp[32], psName[PROCESS_NAME_SIZE];
	time_t	curtime;
	va_list	args;
    FILE	*log_fp;

	memcpy((char *)&psName, name, PROCESS_NAME_SIZE);

    logopen((char *)&myData->log[type].LogPath,
		(char *)&myData->log[type].LogFile, LOG_FNAME_DATE, type);

	time(&curtime);
	asciitime = ctime(&curtime);
	asciitime[24] = 0;

	va_start(args, fmt);
	if(myData->log[type].LogFlag & LOG_2FILE) {
		if((log_fp = fopen((char *)&myData->log[type].OpenLogFile, "a"))
			== NULL) return;
		memset(tmp, 0, sizeof tmp);
		memcpy((char *)&tmp[0], psName, PROCESS_NAME_SIZE);
		fprintf(log_fp, "m%03d:%s:%s: ",
			myData->AppControl.config.moduleNo, asciitime, tmp);
		vfprintf(log_fp, fmt, args);
		fflush(log_fp);
		fclose(log_fp);
		chmod((char *)&myData->log[type].OpenLogFile, 0666);
	}

	if(myData->log[type].LogFlag & LOG_2STDOUT) {
		memset(tmp, 0, sizeof tmp);
		memcpy((char *)&tmp[0], psName, PROCESS_NAME_SIZE);
		printf("m%03d:%s:%s: ", myData->AppControl.config.moduleNo,
			asciitime, tmp);
		vprintf(fmt, args);
		fflush(stdout);
	}
	va_end(args);
}

void userlog2(int type, char *name, char *fmt, ...)
{
	char	*asciitime, psName[PROCESS_NAME_SIZE];
	time_t	curtime;
	va_list	args;
    FILE	*log_fp;

	time(&curtime);
	asciitime = ctime(&curtime);
	asciitime[24] = 0;

	memcpy((char *)&psName, name, PROCESS_NAME_SIZE);

	va_start(args, fmt);
	if(myData->log[type].LogFlag & LOG_2FILE) {
		if((log_fp = fopen((char *)&myData->log[type].OpenLogFile, "a"))
			== NULL) return;
		vfprintf(log_fp, fmt, args);
		fflush(log_fp);
		fclose(log_fp);
		chmod((char *)&myData->log[type].OpenLogFile, 0666);
	}

	if(myData->log[type].LogFlag & LOG_2STDOUT) {
		vprintf(fmt, args);
		fflush(stdout);
	}
	va_end(args);
}
