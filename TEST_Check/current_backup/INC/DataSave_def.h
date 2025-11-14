#ifndef __DATASAVE_DEF_H__
#define __DATASAVE_DEF_H__

//DataSave define
//#define MAX_RESULT_FILE_INDEX				4
#define MAX_RESULT_FILE_INDEX				150 //30files * 5days kjg_100713
#define MAX_RESULT_FILE_INDEX_FORMATION		50
#define MAX_RESULT_FILE_INDEX_GENERAL		300 //30files * 10days
#define MAX_DIVISION_COUNT					60000

// DataSave Signal
#define DATASAVE_SIG_SAVED_FILE_DELETE		0
#define DATASAVE_SIG_SAVED_FILE_DELETE_IDX	1

//code
#define DATASAVE_CD_SAVED_FILE_DELETE_ERROR	0
#define DATASAVE_CD_SEND_SAVE_MSG_ERROR		1
#define DATASAVE_CD_RESULT_SAVE_FILE_ERROR	2

#endif
