#ifndef __DATASTORE_H__
#define __DATASTORE_H__

/*#include "SysDefine.h"
#include "Message_str.h"
#include "AppControl_str.h"
#include "COA_Client_str.h"
#include "COB_Client_str.h"
#include "COC_Client_str.h"
#include "DataSave_str.h"
#include "ModuleControl_str.h"
#include "InOutControl_str.h"
#include "JigControl_str.h"
#include "SubSensV_str.h"
#include "CAN_str.h" 
#include "COM_str.h"
#include "PLC_def.h"
#include "FCH_Control_str.h"
#include "PWM3_Control_str.h"
#include "TimeSchedule_str.h" //kjh_160418
#include "common_all_str.h" //kjg_171225
#include "common_cob_str.h" //kjg_171225
*/

#include "datastore_common.h"
#include "datastore_coa.h"
/* //kjh_211021
#if defined __COA__
#include "datastore_common.h"
#include "datastore_coa.h"
#elif defined __COB__
#include "datastore_common.h"
#include "datastore_cob.h"
#elif defined __COC__
#include "datastore_common.h"
#include "datastore_coc.h"
#elif defined __COD__
#include "datastore_common.h"
#include "datastore_cod.h"
#else
#include "datastore_common.h"
#endif
*/ //kjh_211021e
#endif
