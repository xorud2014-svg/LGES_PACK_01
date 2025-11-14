#include "../../INC/datastore.h"
#include "common_utils.h"
#include "CAN.h"

extern S_SYSTEM_DATA	*myData;

//shhw_231004s
float FindCanChangeData_LTC(int module_ch, float default_value, int function_div)
{ //kjhw_170911
	int idx, j, k, ch, idxStepNo, func_div;

	ch = module_ch;
	idxStepNo = (int)myData->cData[ch].op.idxStepNo; //kjhw_171108
	j = k = 0;

	//default_value = myData->canTransmitSetData.normalData[module_ch][i]
		//.default_value;
	if(myData->cData[ch].op.state == C_RUN) {
		for(j=0; j < MAX_CAN_FUNCTION; j++) {
			idx = IDX_LOC_OBJ_CAN_FUNC_DIV_1 + j;
			if(myData->testCond[ch].local_object[idxStepNo][idx]
				== function_div) {
				k++;
				break;
			}
		}
	} else {
	}

	if(k == 0) {
		for(j=0; j < MAX_CAN_TRANSMIT_CHANGE_DATA; j++) {
			func_div = (int)myData->canTransmitChange.changeData[module_ch][j]
				.function_div; //kjhw_171108
			if(func_div == function_div) {
				default_value 
					= myData->canTransmitChange.changeData[module_ch][j].default_value;
				k++;
				break;
			} 
		}
		if(k == 0) {
			//default_value = myData->canTransmitSetData.normalData[module_ch][i]
				//.default_value;
		}
	} else if(k == 1) {
		idx = IDX_LOC_OBJ_CAN_VALUE_1 + j;
		default_value = (float)myData->testCond[ch].local_object[idxStepNo][idx]; //kjhw_170811
		default_value /= 1000.0;

		for(j=0; j < MAX_CAN_TRANSMIT_CHANGE_DATA; j++) {
			func_div = myData->canTransmitChange.changeData[module_ch][j]
				.function_div;
			if(func_div == 0) {
				myData->canTransmitChange.changeData[module_ch][j].function_div 
					= function_div;
				myData->canTransmitChange.changeData[module_ch][j].default_value 
					= default_value;
				break;
			} else if(func_div == function_div) {
				myData->canTransmitChange.changeData[module_ch][j].default_value 
					= default_value;
				break;
			}
		}
	} else {
		//default_value = myData->canTransmitSetData.normalData[module_ch][i]
			//.default_value;
	}

	return default_value;
}
//shhw_231004e
