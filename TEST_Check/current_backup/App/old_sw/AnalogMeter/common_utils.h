#ifndef __COMMON_UTILS_H__
#define __COMMON_UTILS_H__

void	Unload_Module(void);

int		Open_SystemMemory(int);
void	Close_SystemMemory(void);
void	Close_mbuff(int);
//int		my_system(const char *);

int		bdNo_to_groupNo(int);
int		groupNo_to_jigNo(int);
int		jigNo_to_groupNo(int, int);

void	make_com_chCode_cond(int, int, int, char[], int);
void	make_loc_chCode_cond(int, int, int, int, char[], int);
void	make_loc_chCode_cond2(int, int, int, int, char[], int, int);
void	make_aux_chCode_cond(int, int, int, char[]);
void	make_can_chCode_cond(int, int, int, char[]);

void	make2_com_chCode_cond(int, int, int, int, int);
void	make2_loc_chCode_cond(int, int, int, int, int, int);
void	make2_loc_chCode_cond2(int, int, int, int, int, int, int);
void	make2_aux_chCode_cond(int, int, int, int);
void	make2_can_chCode_cond(int, int, int, int);

long	convert_group_state(int, long);

long	convert_step_type(int, long);
long	convert_step_mode(int, long);
long	convert_ch_state(int, long);
long	convert_ch_state_org_to_p2(long, long, long, long);

long	convert_ch_code(int, long);
long	convert_ch_code_org_to_p1(long);
long	convert_ch_code_org_to_p2(long);

int		convert_group_trouble_code(int, int);
int		convert_group_trouble_code_org_to_p1(int);
int		convert_group_trouble_code_org_to_p2(int);

int		GradeCodeCheck(int, unsigned char, long);

unsigned char	Read_InPoint(int, int, short int);
void	Select_OutPoint(int, int, short int, unsigned char);

#ifdef __KERNEL__
#else
long	string_to_long(char *, int, int);
int		Read_Pattern_File(char *, int, int);
int		Write_Pattern_File(char *, int, int, char *);
#endif

#endif
