#ifndef __DAQ_DEF_H__
#define __DAQ_DEF_H__

#define MAX_INSTALLED_AUX		256
#define MAX_AUX_PER_DAQ			128
#define MAX_DAQ_BD_NUM			8

//Aux_Type Define
#define	AUX_V					0
#define	AUX_TH					1

//daq types
#define DAQ_TYPE1						1 //ver1.0 max_64ch
#define DAQ_TYPE2						2 //ver2.0 max_128ch, isolation
#define DAQ_TYPE3						3 //ver2.1 max_256ch, iso & 128ch*2
#define DAQ_TYPE4						4 //ver3.0 max_128ch, iso3
#define DAQ_TYPE5						5 //ver3.1 max_256ch, iso3 & 128ch*2
#define DAQ_TYPE6						6 //ver3.2 max_128ch, iso3 & NO_DMA
#define DAQ_TYPE7						7 //ver3.3 max_256ch, iso3 & NO_DMA
#define DAQ_TYPE8						8 //ver4.0 max_128ch, iso3, DMA //kjg_150225_s
#define DAQ_TYPE9						9 //ver4.0 max_256ch, iso3, DMA
#define DAQ_TYPE10						10 //ver4.0 max_128ch, iso3, NO_DMA
#define DAQ_TYPE11						11 //ver4.0 max_256ch, iso3, NO_DMA //kjg_150225_e
#define DAQ_TYPE12						12 //ver8.0 mux_128, iso, NO_DMA 10mS
//#define DAQ_TYPE20						20 //can_daq kjg_180914

#endif
