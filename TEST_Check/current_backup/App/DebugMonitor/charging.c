
//20181219 KHK------------------------------------------------------
int Read_User_Define_Mode_Charging_Count_File(char *psName1, int ch)
{ 
	char cmd[256], *in_delimiter = ",\t\n\r", *token;
	char temp[256], buf[256];
	int tmp, i, j, k;
	FILE *fp;

	memset(cmd, 0, sizeof cmd);
	sprintf(cmd, "/root/START_INFO/CH%03d/ChargingCount_data.csv",ch+1);
	// /root/START_INFO/CH00#/ChargingCount_data.csv
	
	if((fp = fopen(cmd, "r")) == NULL) {
//		userlog(DEBUG_LOG, psName1, "%s file(load){Open Error}\n", cmd);
		return -1;
	}

	j = k = 0;
	i = 0;
	while(i == 0) {
		j++;
		if(j >= 100) {
			i = -1;
			break;
		}
		memset(temp, 0, sizeof temp);
		tmp = fscanf(fp, "%s", temp);

	//	userlog2(DEBUG_LOG, psName1, " %02x:%02x:%02x:%02x",
	//		(unsigned char)temp[0], (unsigned char)temp[1],
	//		(unsigned char)temp[2], (unsigned char)temp[3]);

//		if(strncmp(buf, "STX_CHGCOUNT", 12) == 0) {
		if(temp[0] == 'S' && temp[1] == 'T' && temp[2] == 'X'
			&&temp[3] == '_' && temp[7] == 'C'){
			i = 1;
			break;
		}
	}
	//userlog2(DEBUG_LOG, psName1, ":end\n");

	if(i <= 0) {
		userlog(DEBUG_LOG, psName1, "%s file(load){STX Error}\n", cmd);
    	fclose(fp);
		return -2;
	}

	memset((char *)&myData->testCond[ch].user_define_mode.charging_count_set, 0,
		sizeof(S_TEST_COND_CHARGING_COUNT_SET_DATA));
	
	memset(temp, 0, sizeof temp);
	tmp = fscanf(fp, "%s", temp);
	token = strtok(temp, in_delimiter);
	if(token == NULL) {
		userlog(DEBUG_LOG, psName1, "%s file(load){Index Error}\n", cmd);
    	fclose(fp);
		return -3;
	}

	for(i=0; i < 16; i++) {
		if(i > 0){
			memset(temp, 0, sizeof temp);
			tmp = fscanf(fp, "%s", temp);
			token = strtok(temp, in_delimiter);
		}
		//Charging Count
		if(token == NULL){
		}else{
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
		}
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		}else{
			if(token != NULL){
				myData->testCond[ch].user_define_mode.charging_count_set.counter[i] = atol(buf);
			}
		}
		token = strtok(NULL, in_delimiter);
		//Charging Mode
		if(token == NULL){
		}else{
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
		}
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		}else{
			if(token != NULL){
				myData->testCond[ch].user_define_mode.charging_count_set.mode[i] = atoi(buf);
			}
		}
		token = strtok(NULL, in_delimiter);
		//SOC Tracking Use Flag
		if(token == NULL){
		}else{
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
		}
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		}else{
			if(token != NULL){
				myData->testCond[ch].user_define_mode.charging_count_set.soc_tracking_use[i] = atoi(buf);
			}
		}
		token = strtok(NULL, in_delimiter);
		//Power
		if(token == NULL){
		}else{
			memset(buf, 0, sizeof buf);
			strcpy(buf, token);
		}
		if((strncmp(buf, "EOF", 3) == 0) || (strncmp(buf, "eof", 3) == 0)) {
		}else{
			if(token != NULL){
				myData->testCond[ch].user_define_mode.charging_count_set.refP[i] = atol(buf);
			}
		}
		token = strtok(NULL, in_delimiter);
	}

    fclose(fp);
	userlog(DEBUG_LOG, psName1,
		"ChargingCount_data file read completed ch:%d \n",ch+1);
	return 0;
}
//--------------------------------------------------------------------

