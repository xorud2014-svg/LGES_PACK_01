#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VoltageState 0
#define CurrentState 1

#define Copy_Original 0
#define Copy_Adapt 1
#define Copy_Adapt2 2
#define Machine_eps 0.00001

void RemoveEnd(char* buf) {
	int i = 0;
	while(buf[i]) {
		i++;
	}
	buf[i-1] = '\0';
}

int File_Change(char* oldFile, char* newFile, int org_unit, int new_unit) {
	FILE *pReadFile = NULL;
	FILE *pWriteFile = NULL;

	int TryCnt = 0;
	int Channel = 0;
	int Range = 0;
	int state = VoltageState;
	int copy_state = Copy_Original;
	int diff = (org_unit - new_unit)*3;
	int i, result;

	char TempString[255];
	char *ReadData;

	pReadFile = fopen(oldFile, "rt");
	pWriteFile = fopen(newFile, "wt");

	if(pReadFile == NULL) {
		while(pReadFile == NULL) {
			pReadFile = fopen(oldFile, "rt");
			TryCnt++;
			if(TryCnt > 10) {
				printf("Do not Load ReadFile - %s", oldFile);
				fclose(pWriteFile);
				return -1;
			}
		}
	}

	TryCnt = 0;
	
	if(pWriteFile == NULL) {
		while(pWriteFile == NULL) {
			pWriteFile = fopen(newFile, "wt");
			TryCnt++;
			if(TryCnt > 10) {
				printf("Do not Load WriteFile - %s", newFile);
				fclose(pReadFile);
				return -1;
			}
		}
	}

	for(i = 0; i < 3; i++) {
		ReadData = fgets(TempString, sizeof(TempString), pReadFile);
		fprintf(pWriteFile, "%s", ReadData);
	}

	while(1) {
		char* Token;
		ReadData = fgets(TempString, sizeof(TempString), pReadFile);
		if(feof(pReadFile)) break;
		Token = strtok(ReadData, " ");

		while(Token != NULL) {
			if(strncmp(Token,"voltage",7)==0) {
				fprintf(pWriteFile, "%s", Token); //want to get a new form - insert "//" start of line
				state = VoltageState;
			} else if(strncmp(Token,"current",7)==0) {
				fprintf(pWriteFile, "%s", Token); //want to get a new form - insett "//" start of line
				state = CurrentState;
			} else if(strncmp(Token,"range",5)==0) {
				char* temp = (char*)malloc(sizeof(char)*1);
				fprintf(pWriteFile, "%s", Token); //want to get a new form - insert "//" start of line
				strncpy(temp, Token+5, 1);
				Range = atoi(temp);
			} else if(strncmp(Token,"ch0",3)==0) {
				char* temp = (char*)malloc(sizeof(char)*1);
				fprintf(pWriteFile, "%s", Token); //want to get a new form - insert "//" start of line
				strncpy(temp, Token+3, 1);
				Channel = atoi(temp);
			} else if(strncmp(Token,"setPointNum",11)==0) {
				/*if(state == VoltageState) fprintf(pWriteFile,"voltage\n");
				else fprintf(pWriteFile,"current\n");

				if(new_unit==1) fprintf(pWriteFile,"ratio : n");
				else if(new_unit==2) fprintf(pWriteFile,"ratio : u");
				else if(new_unit==3) fprintf(pWriteFile,"ratio : m");
				else if(new_unit==4) fprintf(pWriteFile,"ratio : ");
				else fprintf(pWriteFile,"ratio : k");

				if(state == VoltageState) fprintf(pWriteFile,"V\n");
				else fprintf(pWriteFile,"A\n");

				fprintf(pWriteFile,"range%d\n",Range);
				fprintf(pWriteFile,"ch0%d\n",Channel);*/
				fprintf(pWriteFile,"setPointNum\n");

				ReadData = fgets(TempString, sizeof(TempString), pReadFile);
				fprintf(pWriteFile,"%s",ReadData);

				copy_state = Copy_Adapt;
				break;
			} else if(strncmp(Token,"checkPointNum",13)==0) {
				fprintf(pWriteFile,"checkPointNum\n");
				ReadData = fgets(TempString, sizeof(TempString), pReadFile);
				fprintf(pWriteFile,"%s",ReadData);
				break;
			} else if(strncmp(Token,"checkPoint",10)==0) {
				fprintf(pWriteFile,"checkPoint");
				copy_state = Copy_Adapt;
			} else if(strncmp(Token,"AD_Ratio",8)==0) {
				fprintf(pWriteFile,"AD_Ratio      ");
				copy_state = Copy_Original;
			} else if(strncmp(Token,"DA_A",4)==0) {
				fprintf(pWriteFile,"DA_A          ");
				copy_state = Copy_Original;
			} else if(strncmp(Token,"AD_A",4)==0) {
				fprintf(pWriteFile,"AD_A          ");
				copy_state = Copy_Original;
			} else if(strncmp(Token,"DA_B",4)==0) {
				fprintf(pWriteFile,"DA_B          ");
				copy_state = Copy_Adapt2;
			} else if(strncmp(Token,"AD_B",4)==0) {
				fprintf(pWriteFile,"AD_B\n");
				copy_state = Copy_Adapt2;
			} else if((strncmp(Token,"set",3)==0)||(strncmp(Token,"check",5)==0)) {
				fprintf(pWriteFile,"%s",Token);
			} else if(strncmp(Token,"ratio",5)==0) {
			} else if(strcmp(Token,"\n")==0) {
				fprintf(pWriteFile,"\n");
			} else if(strcmp(Token," ")==0) {
			} else {
				if(copy_state == Copy_Original) {
					fprintf(pWriteFile,"%s ",Token);
				} else if(copy_state == Copy_Adapt) {
					char* pout = (char*)malloc(sizeof(char)*50);
					int original_number;
					strcpy(pout,Token);
					original_number = atoi(pout);
					
					if(original_number == 0) {
						pout = "0";
					} else {
						if(diff > 0) {
							for(i = 0; i < diff; i++) {
								strcat(pout, "0");
							}
						} else if(diff < 0) {
							for(i = 0; i < (-diff); i++) {
								RemoveEnd(pout);
							}
						}
					}
					original_number = atoi(pout); //kjh_210915
					if(original_number == 0) pout = "0"; //kjh_210915
					fprintf(pWriteFile, "%s ", pout);
				} else if(copy_state == Copy_Adapt2) {
					char* pout = (char*)malloc(sizeof(char)*50);
					float original_number;
					strcpy(pout,Token);
					original_number = atof(pout);
					
					if(diff > 0) {
						for(i = 0; i < diff; i++) {
							original_number *= 10;
						}
					} else if(diff < 0) {
						for(i = 0; i < (-diff); i++) {
							original_number /= 10;
						}
					}
					sprintf(pout, "%f", original_number);
					original_number = atof(pout);
					if((original_number < Machine_eps) && (original_number > (Machine_eps * (-1)))) pout = "0.000000";
					fprintf(pWriteFile, "%s ", pout);
				}
			}

			Token = strtok(NULL," ");
		}		
	}
	fclose(pReadFile);
	fclose(pWriteFile);

	return 0;
}

int main() {
	char oldFileDirectory[100] = "/root/system_data/config/caliData/orignal_CALI_BD1";
	char newFileDirectory[100] = "/root/system_data/config/caliData/new_CALI_BD1";
	
	int original_unit, new_unit;
	
	printf("-------------------Unit Number Information-------------------\n");
	printf("                 n(1) u(2) m(3) base(4) k(5)\n");
	printf("-------------------------------------------------------------\n");
	printf("Select Original Voltage/Current UNIT : ");
	scanf("%d", &original_unit);
	printf("Select New Voltage/Current UNIT : ");
	scanf("%d", &new_unit);

	File_Change(oldFileDirectory, newFileDirectory, original_unit, new_unit);

	return 0;
}
