#ifndef __COMMON_UTILS_H__
#define __COMMON_UTILS_H__

void	Unload_Module(void);

int		Open_SystemMemory(int);
void	Close_SystemMemory(void);
void	Close_mbuff(int);
//int		my_system(const char *);

int		bdNo_to_groupNo(int);
int		groupNo_to_jigNo(int);
int		jigNo_to_groupNo(int);

void	make_com_chCode_cond(S_TEST_CONDITION *, int, int, int, int);
void	make_loc_chCode_cond(S_TEST_CONDITION *, int, int, int, int, int);
void	make_loc_chCode_cond2(S_TEST_CONDITION *, int, int, int, int, int, int);
void	make_aux_chCode_cond(S_TEST_CONDITION *, int, int, int);
void	make_can_chCode_cond(S_TEST_CONDITION *, int, int, int);
//jhkw_220103s
void	make_sub_code(int, int, short int, short int, short int, short int, long, long);
int		convert_subcode_division(int);
//jhkw_220103e

long	convert_group_state(int, long);

long	convert_step_type(int, long);
long	convert_step_mode(int, long);
long	convert_ch_state(int, long);
long	convert_ch_state_org_to_p2(long, long, long, long);

long	convert_ch_code(int, long);
long	convert_ch_code_org_to_p1(long);
long	convert_ch_code_org_to_p2(long);
long	convert_ch_code_org_to_p3(long);

int		convert_group_trouble_code(int, int);
int		convert_group_trouble_code_org_to_p1(int);
int		convert_group_trouble_code_org_to_p2(int);
int		convert_group_trouble_code_org_to_p3(int);

int		GradeCodeCheck(int, unsigned char, long);

unsigned char	Read_InPoint(int, int, short int);
void	Select_OutPoint(int, int, short int, unsigned char);

void    cFind_Temp_Cali_Data(long, int);        //khj_210802
#ifdef __KERNEL__
#else
long	string_to_long(char *, int, int);
int		Read_Pattern_File_1(char *, int, int);
int		Write_Pattern_File_1(char *, int, int, char *);
int		Read_Pattern_File_2(char *, int, int);
int		Write_Pattern_File_2(char *, int, int, char *);
int		checksum_file(char *, int, int, int, char *);

int		read_test_cond_pattern_file_coa(char *, int, int, unsigned char);	//ktg_220512 //shh_220607
int		read_test_cond_usermap_file_coa(char *, int, int); //kjhw_140828
#endif

#endif
