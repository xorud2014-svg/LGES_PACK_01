#ifndef __RT_CAN_H__
#define __RT_CAN_H__

//Number and descriptions of serial ports to manage.  You also need
//to create an ISR (rt_canN_isr()) for each port.
#define RT_CAN_FD_CNT 					8

#include <stdbool.h>

//*** Section : Implementation
//Device selection
//#define CAN_FD_A_CHIP1
#define CAN_FD_A_CHIP2 //kjg_200321

//Maximum Size of TX/RX Object
#define MAX_MSG_SIZE					76

//Maximum number of data_ bytes in message
#define MAX_DATA_BYTES					64

//*** Section : Object definitions
typedef enum {
    CHIP_FIFO_CH0, //CAN_TXQUEUE_CH0
    CHIP_FIFO_CH1,
    CHIP_FIFO_CH2,
    CHIP_FIFO_CH3,
    CHIP_FIFO_CH4,
    CHIP_FIFO_CH5,
    CHIP_FIFO_CH6,
    CHIP_FIFO_CH7,
    CHIP_FIFO_CH8,
    CHIP_FIFO_CH9,
    CHIP_FIFO_CH10,
    CHIP_FIFO_CH11,
    CHIP_FIFO_CH12,
    CHIP_FIFO_CH13,
    CHIP_FIFO_CH14,
    CHIP_FIFO_CH15,
    CHIP_FIFO_CH16,
    CHIP_FIFO_CH17,
    CHIP_FIFO_CH18,
    CHIP_FIFO_CH19,
    CHIP_FIFO_CH20,
    CHIP_FIFO_CH21,
    CHIP_FIFO_CH22,
    CHIP_FIFO_CH23,
    CHIP_FIFO_CH24,
    CHIP_FIFO_CH25,
    CHIP_FIFO_CH26,
    CHIP_FIFO_CH27,
    CHIP_FIFO_CH28,
    CHIP_FIFO_CH29,
    CHIP_FIFO_CH30,
    CHIP_FIFO_CH31,
    CHIP_FIFO_TOTAL_CHANNELS
} T_CHIP_fifoChannel; //CAN FIFO Channels

//FIFO_0 is a special FIFO, the TX Queue
#define CAN_TXQUEUE_CH0         		CHIP_FIFO_CH0

typedef enum {
    CHIP_FILT0,
    CHIP_FILT1,
    CHIP_FILT2,
    CHIP_FILT3,
    CHIP_FILT4,
    CHIP_FILT5,
    CHIP_FILT6,
    CHIP_FILT7,
    CHIP_FILT8,
    CHIP_FILT9,
    CHIP_FILT10,
    CHIP_FILT11,
    CHIP_FILT12,
    CHIP_FILT13,
    CHIP_FILT14,
    CHIP_FILT15,
    CHIP_FILT16,
    CHIP_FILT17,
    CHIP_FILT18,
    CHIP_FILT19,
    CHIP_FILT20,
    CHIP_FILT21,
    CHIP_FILT22,
    CHIP_FILT23,
    CHIP_FILT24,
    CHIP_FILT25,
    CHIP_FILT26,
    CHIP_FILT27,
    CHIP_FILT28,
    CHIP_FILT29,
    CHIP_FILT30,
    CHIP_FILT31,
    CHIP_FILT_TOTAL
} T_CHIP_filter; //CAN Filter Channels

typedef enum {
    CHIP_NORMAL_MODE = 0x00,
    CHIP_SLEEP_MODE = 0x01,
    CHIP_INT_LOOP_MODE = 0x02,
    CHIP_LISTEN_ONLY_MODE = 0x03,
    CHIP_CONFIG_MODE = 0x04,
    CHIP_EXT_LOOP_MODE = 0x05,
    CHIP_CLASSIC_MODE = 0x06,
    CHIP_RESTRICT_MODE = 0x07,
    CHIP_INVALID_MODE = 0xFF
} T_CHIP_opMode; //CAN Operation Modes

typedef enum {
    CHIP_TXBWS_NO_DELAY,
    CHIP_TXBWS_2,
    CHIP_TXBWS_4,
    CHIP_TXBWS_8,
    CHIP_TXBWS_16,
    CHIP_TXBWS_32,
    CHIP_TXBWS_64,
    CHIP_TXBWS_128,
    CHIP_TXBWS_256,
    CHIP_TXBWS_512,
    CHIP_TXBWS_1024,
    CHIP_TXBWS_2048,
    CHIP_TXBWS_4096
} T_CHIP_txbws; //Transmit Bandwidth Sharing

typedef enum {
    CHIP_WFT00,
    CHIP_WFT01,
    CHIP_WFT10,
    CHIP_WFT11
} T_CHIP_wft; //Wake-up Filter Time

typedef enum {
    CHIP_DNET_FILT_DISABLE = 0,
    CHIP_DNET_FILT_SIZE_1_BIT,
    CHIP_DNET_FILT_SIZE_2_BIT,
    CHIP_DNET_FILT_SIZE_3_BIT,
    CHIP_DNET_FILT_SIZE_4_BIT,
    CHIP_DNET_FILT_SIZE_5_BIT,
    CHIP_DNET_FILT_SIZE_6_BIT,
    CHIP_DNET_FILT_SIZE_7_BIT,
    CHIP_DNET_FILT_SIZE_8_BIT,
    CHIP_DNET_FILT_SIZE_9_BIT,
    CHIP_DNET_FILT_SIZE_10_BIT,
    CHIP_DNET_FILT_SIZE_11_BIT,
    CHIP_DNET_FILT_SIZE_12_BIT,
    CHIP_DNET_FILT_SIZE_13_BIT,
    CHIP_DNET_FILT_SIZE_14_BIT,
    CHIP_DNET_FILT_SIZE_15_BIT,
    CHIP_DNET_FILT_SIZE_16_BIT,
    CHIP_DNET_FILT_SIZE_17_BIT,
    CHIP_DNET_FILT_SIZE_18_BIT
} T_CHIP_dnetFiltSize; //Data Byte Filter Number

typedef enum {
    CHIP_PLSIZE_8,
    CHIP_PLSIZE_12,
    CHIP_PLSIZE_16,
    CHIP_PLSIZE_20,
    CHIP_PLSIZE_24,
    CHIP_PLSIZE_32,
    CHIP_PLSIZE_48,
    CHIP_PLSIZE_64
} T_CHIP_plsize; //FIFO Payload Size

typedef struct {
    unsigned int	DNetFilterCount : 5;
    unsigned int	IsoCrcEnable : 1;
    unsigned int	ProtocolExpectionEventDisable : 1;
    unsigned int	WakeUpFilterEnable : 1;
    unsigned int	WakeUpFilterTime : 2;
    unsigned int	BitRateSwitchDisable : 1;
    unsigned int	RestrictReTxAttempts : 1;
    unsigned int	EsiInGatewayMode : 1;
    unsigned int	SystemErrorToListenOnly : 1;
    unsigned int	StoreInTEF : 1;
    unsigned int	TXQEnable : 1;
    unsigned int	TxBandWidthSharing : 4;
} T_CHIP_cfg; //CAN Configure

typedef struct {
    unsigned int	RTREnable : 1;
    unsigned int	TxPriority : 5;
    unsigned int	TxAttempts : 2;
    unsigned int	FifoSize : 5;
    unsigned int	PayLoadSize : 3;
} T_CHIP_txFifoCfg; //CAN Transmit Channel Configure

typedef struct {
    unsigned int	TxPriority : 5;
    unsigned int	TxAttempts : 2;
    unsigned int	FifoSize : 5;
    unsigned int	PayLoadSize : 3;
} T_CHIP_txQueCfg; //CAN Transmit Queue Configure

typedef struct {
    unsigned int	RxTimeStampEnable : 1;
    unsigned int	FifoSize : 5;
    unsigned int	PayLoadSize : 3;
} T_CHIP_rxFifoCfg; //CAN Receive Channel Configure

typedef struct {
    unsigned int	TimeStampEnable : 1;
    unsigned int	FifoSize : 5;
} T_CHIP_tefCfg; //CAN Transmit Event FIFO Configure

typedef struct {
    unsigned int	SID : 11;
    unsigned int	EID : 18;
    unsigned int	SID11 : 1;
    unsigned int	unimplemented1 : 2;
} T_CHIP_msgObjId; //CAN Message Object ID

typedef enum {
    CHIP_DLC_0,
    CHIP_DLC_1,
    CHIP_DLC_2,
    CHIP_DLC_3,
    CHIP_DLC_4,
    CHIP_DLC_5,
    CHIP_DLC_6,
    CHIP_DLC_7,
    CHIP_DLC_8,
    CHIP_DLC_12,
    CHIP_DLC_16,
    CHIP_DLC_20,
    CHIP_DLC_24,
    CHIP_DLC_32,
    CHIP_DLC_48,
    CHIP_DLC_64
} T_CHIP_dlc; //CAN Data Length Code

typedef struct {
	unsigned int	DLC : 4;
    unsigned int	IDE : 1;
    unsigned int	RTR : 1;
    unsigned int	BRS : 1;
    unsigned int	FDF : 1;
    unsigned int	esi : 1;
#ifdef CAN_FD_A_CHIP1
    unsigned int	SEQ : 7;
    unsigned int	unimplemented1 : 16;
#else
    unsigned int	SEQ : 23;
#endif
} T_CHIP_txMsgObjCtl; //CAN TX Message Object Control

typedef struct {
    unsigned int	DLC : 4;
    unsigned int	IDE : 1;
    unsigned int	RTR : 1;
    unsigned int	BRS : 1;
    unsigned int	FDF : 1;
    unsigned int	esi : 1;
    unsigned int	unimplemented1 : 2;
    unsigned int	FilterHit : 5;
    unsigned int	unimplemented2 : 16;
} T_CHIP_rxMsgObjCtl; //CAN RX Message Object Control

typedef unsigned int T_CHIP_timeStamp;

typedef union {
    struct {
        T_CHIP_msgObjId id;
        T_CHIP_txMsgObjCtl ctrl;
		T_CHIP_timeStamp timeStamp;
    } bF;

    unsigned int	word[3];
    unsigned char	byte[12];
} T_CHIP_txMsgObj; //CAN TX Message Object

typedef union {
    struct {
        T_CHIP_msgObjId id;
        T_CHIP_rxMsgObjCtl ctrl;
		T_CHIP_timeStamp timeStamp;
    } bF;

    unsigned int	word[3];
    unsigned char	byte[12];
} T_CHIP_rxMsgObj; //CAN RX Message Object

typedef union {
    struct {
        T_CHIP_msgObjId id;
        T_CHIP_txMsgObjCtl ctrl;
		T_CHIP_timeStamp timeStamp;
    } bF;

    unsigned int	word[3];
    unsigned char	byte[12];
} T_CHIP_tefMsgObj; //CAN TEF Message Object

typedef struct {
    unsigned int	SID : 11;
    unsigned int	EID : 18;
    unsigned int	SID11 : 1;
    unsigned int	EXIDE : 1;
    unsigned int	unimplemented1 : 1;
} T_CHIP_filtObjId; //CAN Filter Object ID

typedef struct {
    unsigned int	MSID : 11;
    unsigned int	MEID : 18;
    unsigned int	MSID11 : 1;
    unsigned int	MIDE : 1;
    unsigned int	unimplemented1 : 1;
} T_CHIP_maskObjId; //CAN Mask Object ID

typedef enum {
    CHIP_RX_FIFO_EMPTY = 0,
    CHIP_RX_FIFO_STATUS_MASK = 0x0F,
    CHIP_RX_FIFO_NOT_EMPTY = 0x01,
    CHIP_RX_FIFO_HALF_FULL = 0x02,
    CHIP_RX_FIFO_FULL = 0x04,
    CHIP_RX_FIFO_OVERFLOW = 0x08
} T_CHIP_rxFifoStatus; //CAN RX FIFO Status

typedef enum {
    CHIP_TX_FIFO_FULL = 0,
    CHIP_TX_FIFO_STATUS_MASK = 0x1F7,
    CHIP_TX_FIFO_NOT_FULL = 0x01,
    CHIP_TX_FIFO_HALF_FULL = 0x02,
    CHIP_TX_FIFO_EMPTY = 0x04,
    CHIP_TX_FIFO_ATTEMPTS_EXHAUSTED = 0x10,
    CHIP_TX_FIFO_ERROR = 0x20,
    CHIP_TX_FIFO_ARBITRATION_LOST = 0x40,
    CHIP_TX_FIFO_ABORTED = 0x80,
    CHIP_TX_FIFO_TRANSMITTING = 0x100
} T_CHIP_txFifoStatus; //CAN TX FIFO Status

typedef enum {
    CHIP_TEF_FIFO_EMPTY = 0,
    CHIP_TEF_FIFO_STATUS_MASK = 0x0F,
    CHIP_TEF_FIFO_NOT_EMPTY = 0x01,
    CHIP_TEF_FIFO_HALF_FULL = 0x02,
    CHIP_TEF_FIFO_FULL = 0x04,
    CHIP_TEF_FIFO_OVERFLOW = 0x08
} T_CHIP_tefFifoStatus; //CAN TEF FIFO Status

typedef enum {
    CHIP_NO_EVENT = 0,
    CHIP_ALL_EVENTS = 0xFF1F,
    CHIP_TX_EVENT = 0x0001,
    CHIP_RX_EVENT = 0x0002,
    CHIP_TIME_BASE_COUNTER_EVENT = 0x0004,
    CHIP_OPERATION_MODE_CHANGE_EVENT = 0x0008,
    CHIP_TEF_EVENT = 0x0010,

    CHIP_RAM_ECC_EVENT = 0x0100,
    CHIP_SPI_CRC_EVENT = 0x0200,
    CHIP_TX_ATTEMPTS_EVENT = 0x0400,
    CHIP_RX_OVERFLOW_EVENT = 0x0800,
    CHIP_SYSTEM_ERROR_EVENT = 0x1000,
    CHIP_BUS_ERROR_EVENT = 0x2000,
    CHIP_BUS_WAKEUP_EVENT = 0x4000,
    CHIP_RX_INVALID_MESSAGE_EVENT = 0x8000
} T_CHIP_moduleEvent; //CAN Module Event (Interrupts)

typedef enum {
    CHIP_TX_FIFO_NO_EVENT = 0,
    CHIP_TX_FIFO_ALL_EVENTS = 0x17,
    CHIP_TX_FIFO_NOT_FULL_EVENT = 0x01,
    CHIP_TX_FIFO_HALF_FULL_EVENT = 0x02,
    CHIP_TX_FIFO_EMPTY_EVENT = 0x04,
    CHIP_TX_FIFO_ATTEMPTS_EXHAUSTED_EVENT = 0x10
} T_CHIP_txFifoEvent; //CAN TX FIFO Event (Interrupts)

typedef enum {
    CHIP_RX_FIFO_NO_EVENT = 0,
    CHIP_RX_FIFO_ALL_EVENTS = 0x0F,
    CHIP_RX_FIFO_NOT_EMPTY_EVENT = 0x01,
    CHIP_RX_FIFO_HALF_FULL_EVENT = 0x02,
    CHIP_RX_FIFO_FULL_EVENT = 0x04,
    CHIP_RX_FIFO_OVERFLOW_EVENT = 0x08
} T_CHIP_rxFifoEvent; //CAN RX FIFO Event (Interrupts)

typedef enum {
    CHIP_TEF_FIFO_NO_EVENT = 0,
    CHIP_TEF_FIFO_ALL_EVENTS = 0x0F,
    CHIP_TEF_FIFO_NOT_EMPTY_EVENT = 0x01,
    CHIP_TEF_FIFO_HALF_FULL_EVENT = 0x02,
    CHIP_TEF_FIFO_FULL_EVENT = 0x04,
    CHIP_TEF_FIFO_OVERFLOW_EVENT = 0x08
} T_CHIP_tefFifoEvent; //CAN TEF FIFO Event (Interrupts)

typedef enum {
    CHIP_500K_1M,    	//0x00
    CHIP_500K_2M,    	//0x01
    CHIP_500K_3M,
    CHIP_500K_4M,
    CHIP_500K_5M,    	//0x04
    CHIP_500K_6M7,
    CHIP_500K_8M,    	//0x06
    CHIP_500K_10M,
    CHIP_250K_500K,		//0x08
    CHIP_250K_833K,
    CHIP_250K_1M,
    CHIP_250K_1M5,
    CHIP_250K_2M,
    CHIP_250K_3M,
    CHIP_250K_4M,
    CHIP_1000K_4M,   	//0x0F
    CHIP_1000K_8M,
    CHIP_125K_500K,   	//0x11
	CHIP_500K_3M2 //kjg_220714
} T_CHIP_bitTimeSetup; //CAN Bit Time Setup: Arbitration/Data Bit Phase

typedef enum {
    CHIP_NBT_125K,
    CHIP_NBT_250K,
    CHIP_NBT_500K,
    CHIP_NBT_1M
} T_CHIP_nbtSetup; //CAN Nominal Bit Time Setup

typedef enum {
    CHIP_DBT_500K,
    CHIP_DBT_833K,
    CHIP_DBT_1M,
    CHIP_DBT_1M5,
    CHIP_DBT_2M,
    CHIP_DBT_3M,
    CHIP_DBT_4M,
    CHIP_DBT_5M,
    CHIP_DBT_6M7,
    CHIP_DBT_8M,
    CHIP_DBT_10M
} T_CHIP_dbtSetup; //CAN Data Bit Time Setup

typedef enum {
    CHIP_SSP_MODE_OFF,
    CHIP_SSP_MODE_MANUAL,
    CHIP_SSP_MODE_AUTO
} T_CHIP_sspMode; //Secondary Sample Point Mode

typedef enum {
    CHIP_ERROR_FREE_STATE = 0,
    CHIP_ERROR_ALL = 0x3F,
    CHIP_TX_RX_WARNING_STATE = 0x01,
    CHIP_RX_WARNING_STATE = 0x02,
    CHIP_TX_WARNING_STATE = 0x04,
    CHIP_RX_BUS_PASSIVE_STATE = 0x08,
    CHIP_TX_BUS_PASSIVE_STATE = 0x10,
    CHIP_TX_BUS_OFF_STATE = 0x20
} T_CHIP_errorState; //CAN Error State

typedef enum {
    CHIP_TS_SOF = 0x00,
    CHIP_TS_EOF = 0x01,
    CHIP_TS_RES = 0x02
} T_CHIP_tsMode; //CAN Time Stamp Mode Select

typedef enum {
    CHIP_ECC_NO_EVENT = 0x00,
    CHIP_ECC_ALL_EVENTS = 0x06,
    CHIP_ECC_SEC_EVENT = 0x02,
    CHIP_ECC_DED_EVENT = 0x04
} T_CHIP_eccEvent; //CAN ECC EVENT

typedef enum {
    CHIP_CRC_NO_EVENT = 0x00,
    CHIP_CRC_ALL_EVENTS = 0x03,
    CHIP_CRC_CRCERR_EVENT = 0x01,
    CHIP_CRC_FORMERR_EVENT = 0x02
} T_CHIP_crcEvent; //CAN CRC EVENT

typedef enum {
    CHIP_PIN_0,
    CHIP_PIN_1
} T_CHIP_pinPos; //GPIO Pin Position

typedef enum {
    CHIP_PINMODE_INT,
    CHIP_PINMODE_GPIO
} T_CHIP_pinMode; //GPIO Pin Modes

typedef enum {
    CHIP_PINOUT,
    CHIP_PININ
} T_CHIP_pinDir; //GPIO Pin Directions

typedef enum {
    CHIP_PUSHPULL,
    CHIP_OPENDRAIN
} T_CHIP_odm; //GPIO Open Drain Mode

typedef enum {
    CHIP_PINLOW,
    CHIP_PINHIGH
} T_CHIP_pinState; //GPIO Pin State

typedef enum {
    CHIP_CLKO_CLOCK,
    CHIP_CLKO_SOF
} T_CHIP_clkoMode; //Clock Output Mode

typedef struct {
    unsigned int	NBIT0_ERR : 1;
    unsigned int	NBIT1_ERR : 1;
    unsigned int	NACK_ERR : 1;
    unsigned int	NFORM_ERR : 1;
    unsigned int	NSTUFF_ERR : 1;
    unsigned int	NCRC_ERR : 1;
    unsigned int	unimplemented1 : 1;
    unsigned int	TXBO_ERR : 1;
    unsigned int	DBIT0_ERR : 1;
    unsigned int	DBIT1_ERR : 1;
    unsigned int	unimplemented2 : 1;
    unsigned int	DFORM_ERR : 1;
    unsigned int	DSTUFF_ERR : 1;
    unsigned int	DCRC_ERR : 1;
    unsigned int	esi : 1;
    unsigned int	DLC_MISMATCH : 1;
} T_CHIP_busDiagFlags; //CAN Bus Diagnostic flags

typedef struct {
    unsigned char	NREC;
    unsigned char	NTEC;
    unsigned char	DREC;
    unsigned char	DTEC;
} T_CHIP_busErrorCount; //CAN Bus Diagnostic Error Counts

typedef union {
    struct {
        T_CHIP_busErrorCount errorCount;
        unsigned short	errorFreeMsgCount;
        T_CHIP_busDiagFlags flag;
    } bF;

    unsigned int	word[2];
    unsigned char	byte[8];
} T_CHIP_busDiag; //CAN BUS DIAGNOSTICS

typedef enum {
    CHIP_TXREQ_CH0 = 0x00000001,
    CHIP_TXREQ_CH1 = 0x00000002,
    CHIP_TXREQ_CH2 = 0x00000004,
    CHIP_TXREQ_CH3 = 0x00000008,
    CHIP_TXREQ_CH4 = 0x00000010,
    CHIP_TXREQ_CH5 = 0x00000020,
    CHIP_TXREQ_CH6 = 0x00000040,
    CHIP_TXREQ_CH7 = 0x00000080,

    CHIP_TXREQ_CH8 = 0x00000100,
    CHIP_TXREQ_CH9 = 0x00000200,
    CHIP_TXREQ_CH10 = 0x00000400,
    CHIP_TXREQ_CH11 = 0x00000800,
    CHIP_TXREQ_CH12 = 0x00001000,
    CHIP_TXREQ_CH13 = 0x00002000,
    CHIP_TXREQ_CH14 = 0x00004000,
    CHIP_TXREQ_CH15 = 0x00008000,

    CHIP_TXREQ_CH16 = 0x00010000,
    CHIP_TXREQ_CH17 = 0x00020000,
    CHIP_TXREQ_CH18 = 0x00040000,
    CHIP_TXREQ_CH19 = 0x00080000,
    CHIP_TXREQ_CH20 = 0x00100000,
    CHIP_TXREQ_CH21 = 0x00200000,
    CHIP_TXREQ_CH22 = 0x00400000,
    CHIP_TXREQ_CH23 = 0x00800000,

    CHIP_TXREQ_CH24 = 0x01000000,
    CHIP_TXREQ_CH25 = 0x02000000,
    CHIP_TXREQ_CH26 = 0x04000000,
    CHIP_TXREQ_CH27 = 0x08000000,
    CHIP_TXREQ_CH28 = 0x10000000,
    CHIP_TXREQ_CH29 = 0x20000000,
    CHIP_TXREQ_CH30 = 0x40000000,
    CHIP_TXREQ_CH31 = 0x80000000
} T_CHIP_txReqChannel; //TXREQ Channel Bits

typedef struct {
    unsigned int	PllEnable : 1;
    unsigned int	OscDisable : 1;
    unsigned int	SclkDivide : 1;
    unsigned int	ClkOutDivide : 2;
#ifndef CAN_FD_A_CHIP1
	unsigned int	LowPowerModeEnable : 1;
#endif
} T_CHIP_oscCtrl; //Oscillator Control

typedef struct {
    unsigned int	PllReady : 1;
    unsigned int	OscReady : 1;
    unsigned int	SclkReady : 1;
} T_CHIP_oscSta; //Oscillator Status

typedef enum {
    CHIP_ICODE_FIFO_CH0,
    CHIP_ICODE_FIFO_CH1,
    CHIP_ICODE_FIFO_CH2,
    CHIP_ICODE_FIFO_CH3,
    CHIP_ICODE_FIFO_CH4,
    CHIP_ICODE_FIFO_CH5,
    CHIP_ICODE_FIFO_CH6,
    CHIP_ICODE_FIFO_CH7,
    CHIP_ICODE_FIFO_CH8,
    CHIP_ICODE_FIFO_CH9,
    CHIP_ICODE_FIFO_CH10,
    CHIP_ICODE_FIFO_CH11,
    CHIP_ICODE_FIFO_CH12,
    CHIP_ICODE_FIFO_CH13,
    CHIP_ICODE_FIFO_CH14,
    CHIP_ICODE_FIFO_CH15,
    CHIP_ICODE_FIFO_CH16,
    CHIP_ICODE_FIFO_CH17,
    CHIP_ICODE_FIFO_CH18,
    CHIP_ICODE_FIFO_CH19,
    CHIP_ICODE_FIFO_CH20,
    CHIP_ICODE_FIFO_CH21,
    CHIP_ICODE_FIFO_CH22,
    CHIP_ICODE_FIFO_CH23,
    CHIP_ICODE_FIFO_CH24,
    CHIP_ICODE_FIFO_CH25,
    CHIP_ICODE_FIFO_CH26,
    CHIP_ICODE_FIFO_CH27,
    CHIP_ICODE_FIFO_CH28,
    CHIP_ICODE_FIFO_CH29,
    CHIP_ICODE_FIFO_CH30,
    CHIP_ICODE_FIFO_CH31,
    CHIP_ICODE_TOTAL_CHANNELS,
    CHIP_ICODE_NO_INT = 0x40,
    CHIP_ICODE_CERRIF,
    CHIP_ICODE_WAKIF,
    CHIP_ICODE_RXOVIF,
    CHIP_ICODE_ADDRERR_SERRIF,
    CHIP_ICODE_MABOV_SERRIF,
    CHIP_ICODE_TBCIF,
    CHIP_ICODE_MODIF,
    CHIP_ICODE_IVMIF,
    CHIP_ICODE_TEFIF,
    CHIP_ICODE_TXATIF,
    CHIP_ICODE_RESERVED
} T_CHIP_icode; //ICODE

typedef enum {
    CHIP_RXCODE_FIFO_CH1=1,
    CHIP_RXCODE_FIFO_CH2,
    CHIP_RXCODE_FIFO_CH3,
    CHIP_RXCODE_FIFO_CH4,
    CHIP_RXCODE_FIFO_CH5,
    CHIP_RXCODE_FIFO_CH6,
    CHIP_RXCODE_FIFO_CH7,
    CHIP_RXCODE_FIFO_CH8,
    CHIP_RXCODE_FIFO_CH9,
    CHIP_RXCODE_FIFO_CH10,
    CHIP_RXCODE_FIFO_CH11,
    CHIP_RXCODE_FIFO_CH12,
    CHIP_RXCODE_FIFO_CH13,
    CHIP_RXCODE_FIFO_CH14,
    CHIP_RXCODE_FIFO_CH15,
    CHIP_RXCODE_FIFO_CH16,
    CHIP_RXCODE_FIFO_CH17,
    CHIP_RXCODE_FIFO_CH18,
    CHIP_RXCODE_FIFO_CH19,
    CHIP_RXCODE_FIFO_CH20,
    CHIP_RXCODE_FIFO_CH21,
    CHIP_RXCODE_FIFO_CH22,
    CHIP_RXCODE_FIFO_CH23,
    CHIP_RXCODE_FIFO_CH24,
    CHIP_RXCODE_FIFO_CH25,
    CHIP_RXCODE_FIFO_CH26,
    CHIP_RXCODE_FIFO_CH27,
    CHIP_RXCODE_FIFO_CH28,
    CHIP_RXCODE_FIFO_CH29,
    CHIP_RXCODE_FIFO_CH30,
    CHIP_RXCODE_FIFO_CH31,
    CHIP_RXCODE_TOTAL_CHANNELS,
    CHIP_RXCODE_NO_INT = 0x40,
    CHIP_RXCODE_RESERVED
} T_CHIP_rxcode; //RXCODE

typedef enum {
    CHIP_TXCODE_FIFO_CH0,
    CHIP_TXCODE_FIFO_CH1,
    CHIP_TXCODE_FIFO_CH2,
    CHIP_TXCODE_FIFO_CH3,
    CHIP_TXCODE_FIFO_CH4,
    CHIP_TXCODE_FIFO_CH5,
    CHIP_TXCODE_FIFO_CH6,
    CHIP_TXCODE_FIFO_CH7,
    CHIP_TXCODE_FIFO_CH8,
    CHIP_TXCODE_FIFO_CH9,
    CHIP_TXCODE_FIFO_CH10,
    CHIP_TXCODE_FIFO_CH11,
    CHIP_TXCODE_FIFO_CH12,
    CHIP_TXCODE_FIFO_CH13,
    CHIP_TXCODE_FIFO_CH14,
    CHIP_TXCODE_FIFO_CH15,
    CHIP_TXCODE_FIFO_CH16,
    CHIP_TXCODE_FIFO_CH17,
    CHIP_TXCODE_FIFO_CH18,
    CHIP_TXCODE_FIFO_CH19,
    CHIP_TXCODE_FIFO_CH20,
    CHIP_TXCODE_FIFO_CH21,
    CHIP_TXCODE_FIFO_CH22,
    CHIP_TXCODE_FIFO_CH23,
    CHIP_TXCODE_FIFO_CH24,
    CHIP_TXCODE_FIFO_CH25,
    CHIP_TXCODE_FIFO_CH26,
    CHIP_TXCODE_FIFO_CH27,
    CHIP_TXCODE_FIFO_CH28,
    CHIP_TXCODE_FIFO_CH29,
    CHIP_TXCODE_FIFO_CH30,
    CHIP_TXCODE_FIFO_CH31,
    CHIP_TXCODE_TOTAL_CHANNELS,
    CHIP_TXCODE_NO_INT = 0x40,
    CHIP_TXCODE_RESERVED
} T_CHIP_txcode; //TXCODE

typedef enum {
    CHIP_SYSCLK_40M,
    CHIP_SYSCLK_20M,
    CHIP_SYSCLK_10M
} T_CHIP_sysClkSpeed; //System Clock Selection

typedef enum {
    CHIP_CLKO_DIV1,
    CHIP_CLKO_DIV2,
    CHIP_CLKO_DIV4,
    CHIP_CLKO_DIV10
} T_CHIP_oscClkoDiv; //CLKO Divide

//*** CHIP Registers
//SPI Instruction Set
#define _CHIP_INS_RESET            		0x00
#define _CHIP_INS_READ             		0x03
#define _CHIP_INS_READ_CRC         		0x0B
#define _CHIP_INS_WRITE            		0x02
#define _CHIP_INS_WRITE_CRC        		0x0A
#define _CHIP_INS_WRITE_SAFE       		0x0C

//Register Addresses
#define _CHIP_REG_CiCON            		0x000
#define _CHIP_REG_CiNBTCFG         		0x004
#define _CHIP_REG_CiDBTCFG         		0x008
#define _CHIP_REG_CiTDC            		0x00C

#define _CHIP_REG_CiTBC            		0x010
#define _CHIP_REG_CiTSCON          		0x014
#define _CHIP_REG_CiVEC            		0x018
#define _CHIP_REG_CiINT            		0x01C
#define _CHIP_REG_CiINTFLAG        		_CHIP_REG_CiINT
#define _CHIP_REG_CiINTENABLE      		(_CHIP_REG_CiINT + 2)

#define _CHIP_REG_CiRXIF           		0x020
#define _CHIP_REG_CiTXIF           		0x024
#define _CHIP_REG_CiRXOVIF         		0x028
#define _CHIP_REG_CiTXATIF         		0x02C

#define _CHIP_REG_CiTXREQ          		0x030
#define _CHIP_REG_CiTREC           		0x034
#define _CHIP_REG_CiBDIAG0         		0x038
#define _CHIP_REG_CiBDIAG1         		0x03C

#define _CHIP_REG_CiTEFCON         		0x040
#define _CHIP_REG_CiTEFSTA         		0x044
#define _CHIP_REG_CiTEFUA          		0x048
#define _CHIP_REG_CiFIFOBA         		0x04C

#define _CHIP_REG_CiFIFOCON        		0x050
#define _CHIP_REG_CiFIFOSTA        		0x054
#define _CHIP_REG_CiFIFOUA         		0x058
#define _CHIP_FIFO_OFFSET          		(3 * 4)
#define _CHIP_FILTER_OFFSET        		(2 * 4)

#define _CHIP_REG_CiTXQCON         		0x050
#define _CHIP_REG_CiTXQSTA         		0x054
#define _CHIP_REG_CiTXQUA          		0x058

//The filters start right after the FIFO control/status registers
#define _CHIP_REG_CiFLTCON				(_CHIP_REG_CiFIFOCON + (_CHIP_FIFO_OFFSET * CHIP_FIFO_TOTAL_CHANNELS))
#define _CHIP_REG_CiFLTOBJ				(_CHIP_REG_CiFLTCON + CHIP_FIFO_TOTAL_CHANNELS)
#define _CHIP_REG_CiMASK				(_CHIP_REG_CiFLTOBJ + 4)

//CHIP Specific
#define _CHIP_REG_OSC              		0xE00
#define _CHIP_REG_IOCON            		0xE04
#define _CHIP_REG_CRC              		0xE08
#define _CHIP_REG_ECCCON           		0xE0C
#define _CHIP_REG_ECCSTA           		0xE10
#ifndef CAN_FD_A_CHIP1
#define _CHIP_REG_DEVID					0xE14
#endif

//RAM addresses
#if defined(CAN_FD_A_CHIP1) || defined(CAN_FD_A_CHIP2)
#define _CHIP_RAM_SIZE					2048
#endif
#define _CHIP_RAMADDR_START				0x400
#define _CHIP_RAMADDR_END				(_CHIP_RAMADDR_START + _CHIP_RAM_SIZE)

//*** Register Structures
typedef union {
    unsigned char	byte[4];
    unsigned int	word;
} T_CHIP_reg; //CHIP General 32-bit Registers

typedef union {
    struct {
        unsigned int	DNetFilterCount : 5;
        unsigned int	IsoCrcEnable : 1;
        unsigned int	ProtocolExceptionEventDisable : 1;
        unsigned int	unimplemented1 : 1;
        unsigned int	WakeUpFilterEnable : 1;
        unsigned int	WakeUpFilterTime : 2;
        unsigned int	unimplemented2 : 1;
        unsigned int	BitRateSwitchDisable : 1;
        unsigned int	unimplemented3 : 3;
        unsigned int	RestrictReTxAttempts : 1;
        unsigned int	EsiInGatewayMode : 1;
        unsigned int	SystemErrorToListenOnly : 1;
        unsigned int	StoreInTEF : 1;
        unsigned int	TXQEnable : 1;
        unsigned int	OpMode : 3;
        unsigned int	RequestOpMode : 3;
        unsigned int	AbortAllTx : 1;
        unsigned int	TxBandWidthSharing : 4;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_ctl; //CAN Control Register

typedef union {
    struct {
        unsigned int	SJW : 7;
        unsigned int	unimplemented1 : 1; //can_fd_ubp
        unsigned int	TSEG2 : 7;
        unsigned int	unimplemented2 : 1;
        unsigned int	TSEG1 : 8;
        unsigned int	BRP : 8;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_nbtCfg; //Nominal Bit Time Configuration Register

typedef union {
    struct {
        unsigned int	SJW : 4;
        unsigned int	unimplemented1 : 4;
        unsigned int	TSEG2 : 4;
        unsigned int	unimplemented2 : 4;
        unsigned int	TSEG1 : 5;
        unsigned int	unimplemented3 : 3;
        unsigned int	BRP : 8;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_dbtCfg; //Data Bit Time Configuration Register

typedef union {
    struct {
        unsigned int	TDCValue : 6;
        unsigned int	unimplemented1 : 2;
        unsigned int	TDCOffset : 7;
        unsigned int	unimplemented2 : 1;
        unsigned int	TDCMode : 2;
        unsigned int	unimplemented3 : 6;
        unsigned int	SID11Enable : 1;
        unsigned int	EdgeFilterEnable : 1;
        unsigned int	unimplemented4 : 6;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_tdcCfg; //Transmitter Delay Compensation Register

typedef union {
    struct {
        unsigned int	TBCPrescaler : 10;
        unsigned int	unimplemented1 : 6;
        unsigned int	TBCEnable : 1;
        unsigned int	TimeStampEOF : 1;
        unsigned int	unimplemented2 : 14;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_tsCfg; //Time Stamp Configuration Register

typedef union {
    struct {
        unsigned int	ICODE : 7;
        unsigned int	unimplemented1 : 1;
        unsigned int	FilterHit : 5;
        unsigned int	unimplemented2 : 3;
        unsigned int	TXCODE : 7;
        unsigned int	unimplemented3 : 1;
        unsigned int	RXCODE : 7;
        unsigned int	unimplemented4 : 1;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_intVec; //Interrupt Vector Register

typedef struct {
    unsigned int	TXIF : 1;
    unsigned int	RXIF : 1;
    unsigned int	TBCIF : 1;
    unsigned int	MODIF : 1;
    unsigned int	TEFIF : 1;
    unsigned int	unimplemented1 : 3;

    unsigned int	ECCIF : 1;
    unsigned int	SPICRCIF : 1;
    unsigned int	TXATIF : 1;
    unsigned int	RXOVIF : 1;
    unsigned int	SERRIF : 1;
    unsigned int	CERRIF : 1;
    unsigned int	WAKIF : 1;
    unsigned int	IVMIF : 1;
} T_CHIP_intFlagsStat; //Interrupt Flags

typedef struct {
    unsigned int	TXIE : 1;
    unsigned int	RXIE : 1;
    unsigned int	TBCIE : 1;
    unsigned int	MODIE : 1;
    unsigned int	TEFIE : 1;
    unsigned int	unimplemented2 : 3;

    unsigned int	ECCIE : 1;
    unsigned int	SPICRCIE : 1;
    unsigned int	TXATIE : 1;
    unsigned int	RXOVIE : 1;
    unsigned int	SERRIE : 1;
    unsigned int	CERRIE : 1;
    unsigned int	WAKIE : 1;
    unsigned int	IVMIE : 1;
} T_CHIP_intEn; //Interrupt Enables

typedef union {
    struct {
        T_CHIP_intFlagsStat IF;
        T_CHIP_intEn IE;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_int; //Interrupt Register

typedef union {
    T_CHIP_intFlagsStat IF;

    unsigned short	word;
    unsigned char	byte[2];
} T_CHIP_intFlag; //Interrupt Flag Register

typedef union {
    T_CHIP_intEn IE;

    unsigned short	word;
    unsigned char	byte[2];
} T_CHIP_intCfg; //Interrupt Configuration

typedef union {
    struct {
        unsigned int	RxErrorCount : 8;
        unsigned int	TxErrorCount : 8;
        unsigned int	ErrorStateWarning : 1;
        unsigned int	RxErrorStateWarning : 1;
        unsigned int	TxErrorStateWarning : 1;
        unsigned int	RxErrorStatePassive : 1;
        unsigned int	TxErrorStatePassive : 1;
        unsigned int	TxErrorStateBusOff : 1;
        unsigned int	unimplemented1 : 10;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_trec; //Transmit/Receive Error Count Register

typedef union {
    struct {
        unsigned int	NRxErrorCount : 8;
        unsigned int	NTxErrorCount : 8;
        unsigned int	DRxErrorCount : 8;
        unsigned int	DTxErrorCount : 8;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_busDiag0; //Diagnostic Register 0

typedef union {
    struct {
        unsigned int	ErrorFreeMsgCount : 16;

        unsigned int	NBit0Error : 1;
        unsigned int	NBit1Error : 1;
        unsigned int	NAckError : 1;
        unsigned int	NFormError : 1;
        unsigned int	NStuffError : 1;
        unsigned int	NCRCError : 1;
        unsigned int	unimplemented1 : 1;
        unsigned int	TXBOError : 1;
        unsigned int	DBit0Error : 1;
        unsigned int	DBit1Error : 1;
        unsigned int	DAckError : 1;
        unsigned int	DFormError : 1;
        unsigned int	DStuffError : 1;
        unsigned int	DCRCError : 1;
        unsigned int	esi : 1;
        unsigned int	unimplemented2 : 1;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_busDiag1; //Diagnostic Register 1

typedef union {
    struct {
        unsigned int	TEFNEIE : 1;
        unsigned int	TEFHFIE : 1;
        unsigned int	TEFFULIE : 1;
        unsigned int	TEFOVIE : 1;
        unsigned int	unimplemented1 : 1;
        unsigned int	TimeStampEnable : 1;
        unsigned int	unimplemented2 : 2;
        unsigned int	UINC : 1;
        unsigned int	unimplemented3 : 1;
        unsigned int	FRESET : 1;
        unsigned int	unimplemented4 : 13;
        unsigned int	FifoSize : 5;
        unsigned int	unimplemented5 : 3;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_tefCtl; //Transmit Event FIFO Control Register

typedef union {
    struct {
        unsigned int	TEFNotEmptyIF : 1;
        unsigned int	TEFHalfFullIF : 1;
        unsigned int	TEFFullIF : 1;
        unsigned int	TEFOVIF : 1;
        unsigned int	unimplemented1 : 28;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_tef; //Transmit Event FIFO Status Register

typedef union {
    struct {
        unsigned int	TxNotFullIE : 1;
        unsigned int	unimplemented1 : 1;
        unsigned int	TxEmptyIE : 1;
        unsigned int	unimplemented2 : 1;
        unsigned int	TxAttemptIE : 1;
        unsigned int	unimplemented3 : 2;
        unsigned int	TxEnable : 1;
        unsigned int	UINC : 1;
        unsigned int	TxRequest : 1;
        unsigned int	FRESET : 1;
        unsigned int	unimplemented4 : 5;
        unsigned int	TxPriority : 5;
        unsigned int	TxAttempts : 2;
        unsigned int	unimplemented5 : 1;
        unsigned int	FifoSize : 5;
        unsigned int	PayLoadSize : 3;
    } txBF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_txQueCtl; //Transmit Queue Control Register

typedef union {
    struct {
        unsigned int	TxNotFullIF : 1;
        unsigned int	unimplemented1 : 1;
        unsigned int	TxEmptyIF : 1;
        unsigned int	unimplemented2 : 1;
        unsigned int	TxAttemptIF : 1;
        unsigned int	TxError : 1;
        unsigned int	TxLostArbitration : 1;
        unsigned int	TxAborted : 1;
        unsigned int	FifoIndex : 5;
        unsigned int	unimplemented3 : 19;
    } txBF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_txQueStat; //Transmit Queue Status Register

typedef union {
    struct { //Receive FIFO
        unsigned int	RxNotEmptyIE : 1;
        unsigned int	RxHalfFullIE : 1;
        unsigned int	RxFullIE : 1;
        unsigned int	RxOverFlowIE : 1;
        unsigned int	unimplemented1 : 1;
        unsigned int	RxTimeStampEnable : 1;
        unsigned int	unimplemented2 : 1;
        unsigned int	TxEnable : 1;
        unsigned int	UINC : 1;
        unsigned int	unimplemented3 : 1;
        unsigned int	FRESET : 1;
        unsigned int	unimplemented4 : 13;
        unsigned int	FifoSize : 5;
        unsigned int	PayLoadSize : 3;
    } rxBF;

    struct { //Transmit FIFO
        unsigned int	TxNotFullIE : 1;
        unsigned int	TxHalfFullIE : 1;
        unsigned int	TxEmptyIE : 1;
        unsigned int	unimplemented1 : 1;
        unsigned int	TxAttemptIE : 1;
        unsigned int	unimplemented2 : 1;
        unsigned int	RTREnable : 1;
        unsigned int	TxEnable : 1;
        unsigned int	UINC : 1;
        unsigned int	TxRequest : 1;
        unsigned int	FRESET : 1;
        unsigned int	unimplemented3 : 5;
        unsigned int	TxPriority : 5;
        unsigned int	TxAttempts : 2;
        unsigned int	unimplemented4 : 1;
        unsigned int	FifoSize : 5;
        unsigned int	PayLoadSize : 3;
    } txBF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_fifoCtl; //FIFO Control Register

typedef union {
    struct { //Receive FIFO
        unsigned int	RxNotEmptyIF : 1;
        unsigned int	RxHalfFullIF : 1;
        unsigned int	RxFullIF : 1;
        unsigned int	RxOverFlowIF : 1;
        unsigned int	unimplemented1 : 4;
        unsigned int	FifoIndex : 5;
        unsigned int	unimplemented2 : 19;
    } rxBF;

    struct { //Transmit FIFO
        unsigned int	TxNotFullIF : 1;
        unsigned int	TxHalfFullIF : 1;
        unsigned int	TxEmptyIF : 1;
        unsigned int	unimplemented1 : 1;
        unsigned int	TxAttemptIF : 1;
        unsigned int	TxError : 1;
        unsigned int	TxLostArbitration : 1;
        unsigned int	TxAborted : 1;
        unsigned int	FifoIndex : 5;
        unsigned int	unimplemented2 : 19;
    } txBF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_fifoStat; //FIFO Status Register

typedef union {
    struct {
        unsigned int	UserAddress : 12;
        unsigned int	unimplemented1 : 20;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_fifoUserCfg; //FIFO User Address Register

typedef union {
    struct {
        unsigned int	BufferPointer : 5;
        unsigned int	unimplemented1 : 2;
        unsigned int	Enable : 1;
    } bF;

    unsigned char	byte;
} T_CHIP_filtCtl; //Filter Control Register

typedef union {
    T_CHIP_filtObjId bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_filtObj; //Filter Object Register

typedef union {
    T_CHIP_maskObjId bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_maskObj; //Mask Object Register

//*** CAN_FD_A_CHIP Specific
typedef union {
    struct {
        unsigned int	PllEnable : 1;
        unsigned int	unimplemented1 : 1;
        unsigned int	OscDisable : 1;
#ifdef CAN_FD_A_CHIP1
        unsigned int	unimplemented2 : 1;
#else
        unsigned int	LowPowerModeEnable : 1;
#endif
        unsigned int	SCLKDIV : 1;
        unsigned int	CLKODIV : 2;
        unsigned int	unimplemented3 : 1;
        unsigned int	PllReady : 1;
        unsigned int	unimplemented4 : 1;
        unsigned int	OscReady : 1;
        unsigned int	unimplemented5 : 1;
        unsigned int	SclkReady : 1;
        unsigned int	unimplemented6 : 19;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_osc; //Oscillator Control Register

typedef union {
    struct {
        unsigned int	TRIS0 : 1;
        unsigned int	TRIS1 : 1;
        unsigned int	unimplemented1 : 2;
        unsigned int	ClearAutoSleepOnMatch : 1;
        unsigned int	AutoSleepEnable : 1;
        unsigned int	XcrSTBYEnable : 1;
        unsigned int	unimplemented2 : 1;
        unsigned int	LAT0 : 1;
        unsigned int	LAT1 : 1;
        unsigned int	unimplemented3 : 5;
        unsigned int	HVDETSEL : 1;
        unsigned int	GPIO0 : 1;
        unsigned int	GPIO1 : 1;
        unsigned int	unimplemented4 : 6;
        unsigned int	PinMode0 : 1;
        unsigned int	PinMode1 : 1;
        unsigned int	unimplemented5 : 2;
        unsigned int	TXCANOpenDrain : 1;
        unsigned int	SOFOutputEnable : 1;
        unsigned int	INTPinOpenDrain : 1;
        unsigned int	unimplemented6 : 1;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_ioCtl; //I/O Control Register

typedef union {
    struct {
        unsigned int	CRC : 16;
        unsigned int	CRCERRIF : 1;
        unsigned int	FERRIF : 1;
        unsigned int	unimplemented1 : 6;
        unsigned int	CRCERRIE : 1;
        unsigned int	FERRIE : 1;
        unsigned int	unimplemented2 : 6;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_crc; //CRC Regsiter

typedef union {
    struct {
        unsigned int	EccEn : 1;
        unsigned int	SECIE : 1;
        unsigned int	DEDIE : 1;
        unsigned int	unimplemented1 : 5;
        unsigned int	Parity : 7;
        unsigned int	unimplemented2 : 17;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_eccCtl; //ECC Control Register

typedef union {
    struct {
        unsigned int	unimplemented1 : 1;
        unsigned int	SECIF : 1;
        unsigned int	DEDIF : 1;
        unsigned int	unimplemented2 : 13;
        unsigned int	ErrorAddress : 12;
        unsigned int	unimplemented3 : 4;
    } bF;

    unsigned int	word;
    unsigned char	byte[4];
} T_CHIP_eccSta; //ECC Status Register

#ifndef CAN_FD_A_CHIP1
typedef union {
	struct {
		unsigned int	REV : 4;
		unsigned int	DEV : 4;
		unsigned int	unimplemented : 24;
	} bF;

	unsigned int	word;
	unsigned char	byte[4];
} T_CHIP_devId;
#endif

//Reset Values
//Control Register Reset Values up to FIFOs
#define N_CAN_CTRL_REGS					20

static unsigned int canControlResetValues[] = {
    //Address 0x000 to 0x00C
    0x04980760, 0x003E0F0F, 0x000E0303, 0x00021000,

    //Address 0x010 to 0x01C
    0x00000000, 0x00000000, 0x40400040, 0x00000000,

    //Address 0x020 to 0x02C
    0x00000000, 0x00000000, 0x00000000, 0x00000000,

    //Address 0x030 to 0x03C
    0x00000000, 0x00200000, 0x00000000, 0x00000000,

    //Address 0x040 to 0x04C
    0x00000400, 0x00000000, 0x00000000, 0x00000000
};

//FIFO Register Reset Values
#define N_CHIP_FIFO_REGS				(CHIP_FIFO_TOTAL_CHANNELS * _CHIP_FIFO_OFFSET)

static unsigned int canFifoResetValues[] = {
    0x00600400, 0x00000000, 0x00000000
};

//*** api
#define CRCBASE							0xFFFF
#define CRCUPPER						1

const unsigned char bitReverseTable256[256] = { //Reverse order of bits in byte
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0,
    0x30, 0xB0, 0x70, 0xF0, 0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
    0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 0x04, 0x84, 0x44, 0xC4,
    0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC,
    0x3C, 0xBC, 0x7C, 0xFC, 0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
    0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 0x0A, 0x8A, 0x4A, 0xCA,
    0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6,
    0x36, 0xB6, 0x76, 0xF6, 0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
    0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01, 0x81, 0x41, 0xC1,
    0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9,
    0x39, 0xB9, 0x79, 0xF9, 0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
    0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5, 0x0D, 0x8D, 0x4D, 0xCD,
    0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3,
    0x33, 0xB3, 0x73, 0xF3, 0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 0x07, 0x87, 0x47, 0xC7,
    0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF,
    0x3F, 0xBF, 0x7F, 0xFF
};

const unsigned short crc16Table[256] = { //Look-up table for CRC calculation
    0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
    0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
    0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
    0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
    0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
    0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
    0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
    0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
    0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
    0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
    0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
    0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
    0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
    0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
    0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
    0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
    0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
    0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
    0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
    0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
    0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
    0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
    0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
    0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
    0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
    0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
    0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
    0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
    0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
    0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
    0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
    0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

#define RT_CAN_FD_NAME					"rt_can_fd"
#define RT_CAN_FD_BASE_BAUD 			500000
#define RT_CAN_FD_CLASSIC_FLAG 			0
#define RT_CAN_FD_NORMAL_FLAG 			1
#define RT_CAN_FD_RX_MSG_BUF_SIZE		200
#define RT_CAN_FD_TX_MSG_BUF_SIZE		200 //kjg_180522 50->200

#define RT_CAN_FD_BASE_ADDR				0x800
#define RT_CAN_FD_BD1_IN_MASK			0x01
#define RT_CAN_FD_BD1_OUT_MASK			0x01
#define RT_CAN_FD_BD1_IN_IRQ			0x02
#define RT_CAN_FD_BD1_IN_1				0x03
#define RT_CAN_FD_BD1_OUT_1				0x03
#define RT_CAN_FD_BD1_IN_2				0x04
#define RT_CAN_FD_BD1_OUT_2				0x04
#define RT_CAN_FD_BD1_IN_3				0x05
#define RT_CAN_FD_BD1_OUT_3				0x05
#define RT_CAN_FD_BD1_INFO_1			0x06
#define RT_CAN_FD_BD1_INFO_2			0x07
#define RT_CAN_FD_BD1_INFO_3			0x08
#define RT_CAN_FD_BD2_IN_MASK			0x21
#define RT_CAN_FD_BD2_OUT_MASK			0x21
#define RT_CAN_FD_BD2_IN_IRQ			0x22
#define RT_CAN_FD_BD2_IN_1				0x23
#define RT_CAN_FD_BD2_OUT_1				0x23
#define RT_CAN_FD_BD2_IN_2				0x24
#define RT_CAN_FD_BD2_OUT_2				0x24
#define RT_CAN_FD_BD2_IN_3				0x25
#define RT_CAN_FD_BD2_OUT_3				0x25
#define RT_CAN_FD_BD2_INFO_1			0x26
#define RT_CAN_FD_BD2_INFO_2			0x27
#define RT_CAN_FD_BD2_INFO_3			0x28

//error codes
#define CAN_ERR_OK             	0x0000 //no error
#define CAN_ERR_XMTFULL        	0x0001 //transmit buffer full
#define CAN_ERR_OVERRUN        	0x0002 //overrun in receive buffer
#define CAN_ERR_BUSLIGHT       	0x0004 //bus error, errorcounter limit reached
#define CAN_ERR_BUSHEAVY       	0x0008 //bus error, errorcounter limit reached
#define CAN_ERR_BUSOFF         	0x0010 //bus error, 'bus off' state entered
#define CAN_ERR_QRCVEMPTY      	0x0020 //receive queue is empty
#define CAN_ERR_QOVERRUN       	0x0040 //receive queue overrun
#define CAN_ERR_QXMTFULL       	0x0080 //transmit queue full 
#define CAN_ERR_REGTEST        	0x0100 //est of controller registers failed
#define CAN_ERR_NOVXD          	0x0200 //Win95/98/ME only
#define CAN_ERR_RESOURCE       	0x2000 //can't create resource
#define CAN_ERR_ILLPARAMTYPE   	0x4000 //illegal parameter
#define CAN_ERR_ILLPARAMVAL    	0x8000 //value out of range
#define CAN_ERRMASK_ILLHANDLE  	0x1C00 //wrong handle, handle error

//MSGTYPE
#define MSGTYPE_STATUS					0x80 //used to mark pending status
#define MSGTYPE_EXTENDED				0x02 //declares a extended frame
#define MSGTYPE_RTR						0x01 //marks a remote frame
#define MSGTYPE_STANDARD				0x00 //marks a standard frame

#define CHIP_SPI_BUFFER_LENGTH			96 //kjg_200322
#define FPGA_SPI_BUFFER_LENGTH			104 //kjg_200326 100->104
#define MAX_TXQUEUE_ATTEMPTS			50

unsigned char terminal_resistor[2];
int max_can_fd_ch;

struct rt_can_fd_msg {
	unsigned int		id;

	unsigned char		type;
	unsigned char		length;
	unsigned char		can_fd_flag; //0:can_2.0B(default), 1:can_fd
	unsigned char		reserved1;

	unsigned char		data[64]; //kjg_180405 8->64

	long long			time; //kjg_120619
	unsigned int		timeStamp; //kjg_180405
};

//information about handled ports
struct rt_can_fd_struct {
    int					magic;
    int					baud_base;
    int					port;
    int					irq;
    int					flag;
    int					mode;
    int					used;
	    //0 if port region must be requested later by rt_can_fd_set_param
		//1 if port region must be requested by init_module (standard)
    int 				error; //last error detected
	int					error_count;
	unsigned short 		status;

    int					type; //0:standard, 1:extended

	int					set_param_baud;
	unsigned char		set_param_sjw;
	unsigned char		set_param_ext;
	unsigned char		set_param_fd_flag;
	unsigned char		set_param_data_rate;
	unsigned char		set_param_crc;
	unsigned char		set_param_terminal_r;
		
	unsigned char 		set_phase;
	unsigned char 		tmp_set_phase;
	unsigned char		chip_op_phase;
	unsigned char		chip_tx_phase;
	unsigned char		chip_rx_phase;
	unsigned char		chip_tx_rx_phase;

	unsigned char		chip_transfer_phase;
	unsigned char		chip_transfer_count;
	unsigned char		chip_transfer_tx_length;
	unsigned char		chip_transfer_rx_length;
	unsigned char		chip_transfer_cmd;
	unsigned char		chip_transfer_received_length;
	unsigned char		chip_filter_phase;

	unsigned char		error_print_flag;
	unsigned char		chip_tmp_data_uc1;
	unsigned short		chip_tmp_data_us1;
	unsigned int		chip_tmp_data_ui1;

	unsigned int		can_fd_mcu_ver;
	unsigned short		mcu_filter_id_count;
	unsigned char		mcu_chip_op_phase;
	unsigned char		mcu_rx_msg_count;
	unsigned char		mcu_rx_msg_wr_idx;
	unsigned char		mcu_rx_msg_rd_idx;

	int					rx_msg_write_idx;
	int					rx_msg_read_idx;
	int					rx_msg_count;
	int					tx_msg_write_idx;
	int					tx_msg_read_idx;
	int					tx_msg_count;
	int					tx_msg_error;

	struct rt_can_fd_msg	tmp_msg;
	struct rt_can_fd_msg	rx_buf[RT_CAN_FD_RX_MSG_BUF_SIZE];
	struct rt_can_fd_msg	tx_buf[RT_CAN_FD_TX_MSG_BUF_SIZE];

	unsigned char		spi_tx_buf[FPGA_SPI_BUFFER_LENGTH];
	unsigned char		spi_rx_buf[FPGA_SPI_BUFFER_LENGTH];

	T_CHIP_cfg			config;

	//Transmit objects
	T_CHIP_txFifoCfg	txConfig;
	T_CHIP_txFifoEvent	txFlags;
	T_CHIP_txMsgObj		txObj;

	//Receive objects
	T_CHIP_rxFifoCfg	rxConfig;
	T_CHIP_filtObj		fObj;
	T_CHIP_maskObj		mObj;
	T_CHIP_rxFifoEvent	rxFlags;
	T_CHIP_rxMsgObj		rxObj;

	T_CHIP_bitTimeSetup	selectedBitTime;
	unsigned char		tec, rec;
	T_CHIP_errorState	errorFlags;
};

int rt_can_read(int, char *);
int rt_can_write(int, const char *);

int rx_can_data_11(int);
int tx_can_data_11(int);

int rt_can_set_param_0(int, int, int, int, int, int, int, int);
int rt_can_set_param_11(int);
int rt_can_set_filter_1(int);
int rt_can_set_filter_2(int);
int rt_can_set_filter_3(int);
int rt_can_set_etc_1(int);
int rt_can_set_etc_2(int);
int rt_can_set_etc_3(int);
int rt_can_check_status(int);

int SPI_Transfer_11(int, struct rt_can_fd_struct *, unsigned char);
int SPI_Transfer_TX(int, struct rt_can_fd_struct *, unsigned char);
int SPI_Transfer_RX(int, struct rt_can_fd_struct *, unsigned char);

//CHIP HW Driver Functions
char CHIP_MCU_set_parameter(int, unsigned char, unsigned char);
char CHIP_MCU_get_parameter(int, unsigned char);
char CHIP_MCU_get_status(int, unsigned char);
char CHIP_MCU_ReceiveMessageGet(int);

char CHIP_Reset_11(int, unsigned char);

//SPI Access Functions
char CHIP_ReadByte_11(int, unsigned short, unsigned char *);
char CHIP_WriteByte_11(int, unsigned short, unsigned char);
char CHIP_ReadWord_11(int, unsigned short, unsigned int *);
char CHIP_WriteWord_11(int, unsigned short, unsigned int);
char CHIP_ReadHalfWord_11(int, unsigned short, unsigned short *);
char CHIP_WriteHalfWord_11(int, unsigned short, unsigned short);
char CHIP_WriteByteArray_11(int, unsigned short, unsigned char *, unsigned short);
char CHIP_ReadByteArrayWithCRC_11(int, unsigned short, unsigned short, bool, bool *);
char CHIP_ReadWordArray_11(int, unsigned short, unsigned int *, unsigned short);

//Configuration
//CAN Control register configuration
char CHIP_Configure_11(int, unsigned char);

//Reset Configure object to reset values
char CHIP_ConfigureObjectReset(int);

//Operating mode
char CHIP_OperationModeSelect_11(int, T_CHIP_opMode, unsigned char);

//CAN Transmit
//Configure Transmit FIFO
char CHIP_TransmitChannelConfigure_11(int, T_CHIP_fifoChannel, unsigned char);

//Reset TransmitChannelConfigure object to reset values
char CHIP_TransmitChannelConfigureObjectReset(int);

//TX Channel Load
char CHIP_MCU_TransmitMessage(int);

//CAN Receive
//Filter Object Configuration
//Configures ID of filter object
char CHIP_FilterObjectConfigure_11(int, T_CHIP_filter, unsigned char);

//Filter Mask Configuration
//Configures Mask of filter object
char CHIP_FilterMaskConfigure_11(int, T_CHIP_filter, unsigned char);

//Link Filter to FIFO
//Initializes the Pointer from Filter to FIFO
//Enables or disables the Filter
char CHIP_FilterToFifoLink_11(int, T_CHIP_filter, T_CHIP_fifoChannel, bool, unsigned char);

//Configure Receive FIFO
char CHIP_ReceiveChannelConfigure_11(int, T_CHIP_fifoChannel, unsigned char);

//Reset ReceiveChannelConfigure object to reset value
char CHIP_ReceiveChannelConfigureObjectReset(int);

//Module Event Enable, Enables interrupts
char CHIP_ModuleEventEnable_11(int, T_CHIP_moduleEvent, unsigned char);

//Transmit FIFO Event Get
//Reads Transmit FIFO interrupt Flags
char CHIP_TransmitChannelEventGet_11(int, T_CHIP_fifoChannel, T_CHIP_txFifoEvent *, unsigned char);

//Transmit FIFO Event Enable
//Enables Transmit FIFO interrupts
char CHIP_TransmitChannelEventEnable_11(int, T_CHIP_fifoChannel, T_CHIP_txFifoEvent, unsigned char);

//Transmit FIFO Event Disable
//Disables Transmit FIFO interrupts
char CHIP_TransmitChannelEventDisable_11(int, T_CHIP_fifoChannel, T_CHIP_txFifoEvent, unsigned char);

//Receive FIFO Event Get
//Reads Receive FIFO interrupt Flags
char CHIP_ReceiveChannelEventGet_11(int, T_CHIP_fifoChannel, T_CHIP_rxFifoEvent *, unsigned char);

//Receive FIFO Event Enable
//Enables Receive FIFO interrupts
char CHIP_ReceiveChannelEventEnable_11(int, T_CHIP_fifoChannel, T_CHIP_rxFifoEvent, unsigned char);

//Error Counts and Error State Get
//Returns content of complete CiTREC
char CHIP_ErrorCountStateGet_11(int, unsigned char *, unsigned char *, T_CHIP_errorState *);

//ECC
char CHIP_EccEnable_11(int, unsigned char);

//Initialize RAM
char CHIP_RamInit_11(int, unsigned char);

//Bit Time
//Configure Bit Time registers (based on CAN clock speed)
//Configure Nominal bit time for 40MHz system clock
char CHIP_BitTimeConfigureNominal40MHz_11(int, unsigned char);

//Configure Data bit time for 40MHz system clock
char CHIP_BitTimeConfigureData40MHz_11(int, T_CHIP_sspMode, unsigned char);

//GPIO
//Initialize GPIO Mode
char CHIP_GpioModeConfigure_11(int, T_CHIP_pinMode, T_CHIP_pinMode, unsigned char);

//Miscellaneous
unsigned int CHIP_DlcToDataBytes(T_CHIP_dlc);
unsigned short CHIP_CalculateCRC16(unsigned char *, unsigned short);
T_CHIP_dlc CHIP_DataBytesToDlc(unsigned char);

int rt_can_exec_cmd_reset(int);
int rt_can_exec_cmd1(int);

int init_rt_can(void);
void cleanup_rt_can(void);

//shh_250114s
void printCanLogic(unsigned char, unsigned char *, int);
void checkCanLogic(unsigned char *, int);
//shh_250114e

#endif
