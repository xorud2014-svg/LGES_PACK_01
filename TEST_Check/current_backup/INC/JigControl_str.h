#ifndef __JIGCONTROL_STR_H__
#define __JIGCONTROL_STR_H__

#include "SysDefine.h"
#include "JigControl_def.h"

typedef struct s_jig_misc_tag {
	int					psSignal;
	int					JigNo;
	long				cylMoveTimer;
	long				jobChangeMoveTimer;
	long				pitchChangeMoveTimer;
	long				contactTime;
	int					processPointer;
	unsigned int		retryCnt;
} S_JIG_MISC;

typedef struct s_jig_config_tag {
	unsigned char		jigUseFlag;
	unsigned char		jigType;
	unsigned char		trayType;
	unsigned char		totalTrayNo;

	unsigned char		localRemote;
	unsigned char		reserved[3];

	long				cylMoveTimeout;
	long				cylMoveDelay;

	unsigned char		inSignalNo[MAX_JIG_IN_SIGNAL];
	unsigned char		outSignalNo[MAX_JIG_OUT_SIGNAL];
} S_JIG_CONFIG;

typedef struct s_jig_data_tag {
	unsigned char		jigState;
	unsigned char		stopperState;
	unsigned char		loadingEnd;
	unsigned char		microState;
	
	unsigned char		doorState[MAX_TRAY_16];
	unsigned char		trayState[MAX_TRAY_16];
	unsigned char		trayPreLoadState[MAX_TRAY_16];
	unsigned char		tmpTrayState[MAX_TRAY_16];
	unsigned char		trayStateCheck[MAX_TRAY_16];

	unsigned char		code;
	unsigned char		jigNotMove;
	unsigned char		emgPhase;
	unsigned char		troublePos;

	long				jigNotMoveTimer;
	long				loadingEnd_waitTime; 
	
	long				traySensingDelay;
	long				trayCheckDelay;
	long				emgCheckDelay;

	unsigned char		signal[MAX_SIGNAL];

	unsigned char		timerStart;
	unsigned char		preTrayFlag;
	unsigned char		curTrayFlag;
	unsigned char		startButtonFlag;

	unsigned char		emgSwitchFlag;
	unsigned char		reserved[3];

	unsigned char		jobChangePhase;
	unsigned char		jobChangeEnd;
	unsigned char		pitchChangePhase;
	unsigned char		pitchChangeEnd;

	S_JIG_MISC			misc;
	S_JIG_CONFIG		config;
} S_JIG_DATA;
#endif
