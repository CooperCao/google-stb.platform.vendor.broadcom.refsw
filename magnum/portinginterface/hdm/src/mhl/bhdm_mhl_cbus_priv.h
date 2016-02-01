/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef BHDM_MHL_CBUS_PRIV_H__
#define BHDM_MHL_CBUS_PRIV_H__

#include "bchp.h"       /* Chip Info */
#include "breg_mem.h"   /* Chip register access. */
#include "bkni.h"       /* Kernel Interface */
#include "bint.h"       /* Interrupt */
#include "btmr.h"   	/* Timer Handle  */

#include "berr_ids.h"   /* Error codes */
#include "bdbg.h"       /* Debug Support */

#include "bchp_mt_cbus.h"
#include "bchp_mt_msc_resp.h"
#include "bchp_dvp_mt_aon_top.h"
#include "bchp_cbus_intr2_0.h"
#include "bchp_cbus_intr2_1.h"

#include "bhdm_mhl_const_priv.h"
#include "bhdm_mhl_common_priv.h"
#include "bhdm_mhl_cbus_cmd_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CBUS Clock frequency in Hz */
#define BHDM_P_MHL_CBUS_CLK_FREQ            54000000

#define BHDM_P_MHL_CBUS_CLOCK_FACTOR        (1000000UL)

/* CBUS wakeup & discovery time in ms should be around 2s */
#ifndef BHDM_P_MHL_CBUS_WND_WAIT_TIME_IN_MS
#define BHDM_P_MHL_CBUS_WND_WAIT_TIME_IN_MS    2000
#endif

/* CBUS wakeup & discovery retry interval in ms */
#ifndef BHDM_P_MHL_CBUS_WND_RETRY_TIME_IN_MS
#define BHDM_P_MHL_CBUS_WND_RETRY_TIME_IN_MS   150
#endif

/* Timer tick duration in us */
#ifndef BHDM_P_MHL_CBUS_TICK_DURATION
#define BHDM_P_MHL_CBUS_TICK_DURATION          10
#endif

/* No. of ticks of delay between the transmission
   of successive CBUS command */
#ifndef BHDM_P_MHL_CBUS_CMD_SEND_TICK_INTERVAL
#define BHDM_P_MHL_CBUS_CMD_SEND_TICK_INTERVAL 1000
#endif

/* Max. EDID read attempt per block */
#define BHDM_P_MHL_CBUS_MAX_EDID_READ_ATTEMPT  3

/* Heartbeat duration in ticks
   ~26 ticks per ms
   If we have debug level > error,
   decrease the heartbeat interval so the
   logging does not get swamped by the heartbeat.
   MHL 3 specifies a heartbeat interval of 50ms
   and heartbeat packets failed duration of 200ms
   (so 4 packets) before declaring the link is dead.
 */
#ifndef BHDM_P_MHL_CBUS_HEARTBEAT_TICKS
#if MT_DBG_LEVEL > MT_DBG_LEVEL_ERROR
#define BHDM_P_MHL_CBUS_HEARTBEAT_TICKS       (4000*26)
#else
#define BHDM_P_MHL_CBUS_HEARTBEAT_TICKS       (50*26)
#endif
#endif

#ifndef BHDM_P_MHL_CBUS_HEARTBEAT_DEAD_PKTS
#define BHDM_P_MHL_CBUS_HEARTBEAT_DEAD_PKTS    4
#endif

/* Constants for CBUS DCAP content */
#define BHDM_P_MHL_CBUS_DCAP_MHL_VER        (0x21)     /* MHL version */
#define BHDM_P_MHL_CBUS_DCAP_DEV_CAT_TYPE   (0x2)      /* DEV_TYPE source */
#define BHDM_P_MHL_CBUS_DCAP_DEV_CAT_POW    (0x0 << 4) /* Source cannot output power */
#define BHDM_P_MHL_CBUS_DCAP_DEV_CAT_PLIM   (0x0 << 5) /* Power limit */
#define BHDM_P_MHL_CBUS_DCAP_VID_LINK_MODE  (0x3F)     /* RGB444 (bit0), YCC422 (bit2), Packed Pixel (bit3), island (bit4), VGA (bit5) [0x31: RGB444 only with data island/VGA ]*/
#define BHDM_P_MHL_CBUS_DCAP_AUD_LINK_MODE  (0x3)      /* 2ch and 8ch [0x1: Stereo PCM only] */
#define BHDM_P_MHL_CBUS_DCAP_VIDEO_TYPE     (0x0)      /* No content type */
#define BHDM_P_MHL_CBUS_DCAP_LOG_DEV_MAP    (0x6)      /* Video and audio output */
#define BHDM_P_MHL_CBUS_DCAP_BANDWIDTH      (0xF)      /* Max. pixel clock 75MHz */
#define BHDM_P_MHL_CBUS_DCAP_FEATURE        (0x17)     /* RCP/RAP/SP and UCP_RECV support */

/* INT and STAT registers 4 bytes each */
#define BHDM_P_MHL_CBUS_DCAP_INT_STAT_SIZE  (((BHDM_P_MHL_CBUS_STAT_REG_SIZE-1) << 4) |	\
				  ((BHDM_P_MHL_CBUS_INT_REG_SIZE-1)  << 0))


/* CBUS impedance measure time in ms */
#ifndef BHDM_P_MHL_CBUS_Z_MTIME_IN_MS
#define BHDM_P_MHL_CBUS_Z_MTIME_IN_MS 500
#endif

typedef enum
{
	BHDM_P_Mhl_CbusHpd_eDown = 0,
	BHDM_P_Mhl_CbusHpd_eUp   = 1
} BHDM_P_Mhl_CbusHpd;

/* Link state in MT_CBUS_LINK_STATUS register */
typedef enum
{
	BHDM_P_Mhl_CbusLinkState_eReset                = 0,
	BHDM_P_Mhl_CbusLinkState_eWakeup               = 1,
	BHDM_P_Mhl_CbusLinkState_eDiscovery_ePending   = 2,
	BHDM_P_Mhl_CbusLinkState_eDiscovery_eFailed    = 3,
	BHDM_P_Mhl_CbusLinkState_eDiscovery_eSucceeded = 4,
	BHDM_P_Mhl_CbusLinkState_eActive               = 5
} BHDM_P_Mhl_CbusLinkState;

/* Discovery result */
typedef enum
{
	BHDM_P_Mhl_CbusDiscovery_eDisconnected = 0,
	BHDM_P_Mhl_CbusDiscovery_eSucceeded    = 1,
	BHDM_P_Mhl_CbusDiscovery_eFailed       = 2,
	BHDM_P_Mhl_CbusDiscovery_eTimeout      = 3
} BHDM_P_Mhl_CbusDiscovery;

/* MSC error code */
typedef enum
{
	BHDM_P_Mhl_CbusMscErr_eNone          = 0x00, /* no error */
	BHDM_P_Mhl_CbusMscErr_eRetryFailed   = 0x01, /* retry threshold exceeded */
	BHDM_P_Mhl_CbusMscErr_eProtocolError = 0x02, /* Protocol error */
	BHDM_P_Mhl_CbusMscErr_eTimeout       = 0x04, /* Peer timed out */
	BHDM_P_Mhl_CbusMscErr_eInvalidOp     = 0x08, /* Invalid OP code */
	BHDM_P_Mhl_CbusMscErr_eBadOffset     = 0x10, /* bad offset in command */
	BHDM_P_Mhl_CbusMscErr_eBusy          = 0x20 /* peer is busy */
} BHDM_P_Mhl_CbusMscErr;

/* DDC error code */
typedef enum
{
	BHDM_P_Mhl_CbusDdcErr_eNone          = 0x00, /* No error */
	BHDM_P_Mhl_CbusDdcErr_eRetryFailed   = 0x01, /* Retry threshold exceeded */
	BHDM_P_Mhl_CbusDdcErr_eProtocolError = 0x02, /* Protocol error */
	BHDM_P_Mhl_CbusDdcErr_eTimeout       = 0x04 /* Peer timed out */
} BHDM_P_Mhl_CbusDdcErr;

/* Processor state */
typedef enum
{
	BHDM_P_Mhl_CbusMpmState_eIdle        = 0,
	BHDM_P_Mhl_CbusMpmState_eActive      = 1,
	BHDM_P_Mhl_CbusMpmState_eStopping    = 2 /* During hand over */
} BHDM_P_Mhl_CbusMpmState;

/* State for sending PATH_EN=1 to sink */
typedef enum
{
	BHDM_P_Mhl_CbusPathEnSentState_eNotNeeded = 0, /* No need to send PATH_EN yet */
	BHDM_P_Mhl_CbusPathEnSentState_ePending   = 1, /* Sink sent us PATH_EN=1 but we cannot send it yet */
	BHDM_P_Mhl_CbusPathEnSentState_eSent      = 2  /* Sent PATH_EN=1 to sink */
} BHDM_P_Mhl_CbusPathEnSentState;


/* Interrupt status. Just storing the raw interrupt status bits */
typedef struct BHDM_P_Mhl_CbusIntr
{
	uint32_t ulStatus0;
	uint32_t ulStatus1;
} BHDM_P_Mhl_CbusIntr;

typedef struct BHDM_P_Mhl_MpmHostIntr
{
	uint32_t ulStatus;
} BHDM_P_Mhl_MpmHostIntr;

/* CBUS register space/other state (hardware independent),
   We probably need some sort of lock here for the
   interrupt registers and events to signal they have
   changed values via the SET_INT command
 */
typedef struct BHDM_P_Mhl_CbusState
{
	/* Note that these values are stored in the mailbox:
	   DCAP (sink + source)
	   Sink device INT registers
	   Source/Sink STAT registers
	   MSC/DDC errorocodes (sink + source)
	   EDID
	*/

	/* States from mailbox which we need to know
	   the previous state */
	BHDM_P_Mhl_CbusHpd eLastHpdState;

	/* Link mode */
	uint8_t            ucSrcLinkMode;

	/* Initial source DCAP used to initialised the mailbox */
	uint8_t            aucCbusSrcDcap[BHDM_P_MHL_CBUS_CAP_REG_SIZE];
	uint8_t            aucScratchReg[BHDM_P_MHL_MAILBOX_SCRATCHPAD_SIZE]; /* Scratch pad register */

	uint8_t            ucTxVendorId; /* This is NOT adopter ID */
	uint8_t            ucRxVendorId;
} BHDM_P_Mhl_CbusState;


/* Initialization */
void BHDM_P_Mhl_Cbus_Init
	( BREG_Handle                  hRegister );

bool BHDM_P_Mhl_Cbus_GetIntrStatus
	( BREG_Handle                  hRegister,
	  BHDM_P_Mhl_CbusIntr         *pIntrStat );

void BHDM_P_Mhl_Cbus_ClearAllInt_isr
	( BREG_Handle                  hRegister );

void BHDM_P_Mhl_Cbus_ClearInt_isr
	( BREG_Handle                  hRegister,
	  BHDM_P_Mhl_CbusIntr         *pIntStat );

/********************************
  CBUS translation layer command functions
 ********************************/
void BHDM_P_Mhl_Cbus_CmdCopy_isr
	( const BHDM_P_Mhl_CbusCmd    *pstSrcCmd,
	  BHDM_P_Mhl_CbusCmd          *pstDestCmd );

/* packing/unpacking packet to and from a format in the packet register */
void BHDM_P_Mhl_Cbus_PktPack_isr
	( uint16_t                    *pusPktVal,
	  BHDM_P_Mhl_CbusPktType       ePktType,
	  BHDM_P_Mhl_CbusPktDirection  ePktDir,
	  uint8_t                      ucPktData );

void BHDM_P_Mhl_Cbus_PktUnpack_isr
	( uint16_t                     usPktVal,
	  BHDM_P_Mhl_CbusPktType      *pePktType,
	  BHDM_P_Mhl_CbusPktDirection *pePktDir,
	  uint8_t                     *pucPktData );

bool BHDM_P_Mhl_Cbus_PacketMatched_isr
	( BHDM_P_Mhl_CbusPkt          *pPacket,
	  BHDM_P_Mhl_CbusPktDirection  ePktDir,
	  BHDM_P_Mhl_CbusPktType       ePktType,
	  uint8_t                      ucPktData );

void BHDM_P_Mhl_Cbus_SendPkts_isr
	( BREG_Handle                  hRegister,
	  uint32_t                     ulOutboundFifoReg,
	  BHDM_P_Mhl_CbusCmd          *pCmd,
	  BHDM_P_Mhl_CbusCmdQueueType  eQueueType );

bool BHDM_P_Mhl_Cbus_CmdIsMsc_isr
	( BHDM_P_Mhl_CbusCmd          *pCmd );

bool BHDM_P_Mhl_Cbus_CmdIsDdc_isr
	( BHDM_P_Mhl_CbusCmd          *pCmd );

/******************************
       CBUS Phy functions
 ******************************/
void BHDM_P_Mhl_Cbus_Disconnect_isr
	( BREG_Handle                  hRegister );

void BHDM_P_Mhl_Cbus_PhyWaitRcalDone
	( BREG_Handle                  hRegister );

void BHDM_P_Mhl_Cbus_ConfigPhyImpDetectMode
	( BREG_Handle                  hRegister );

bool BHDM_P_Mhl_Cbus_PhyMeasureImp_isr
	( BREG_Handle                  hRegister );

void BHDM_P_Mhl_Cbus_ConfigPhyNormalMode
	( BREG_Handle                  hRegister );

#if BHDM_MHL_CTS
void BHDM_P_Mhl_Cbus_PhyInit
	( BREG_Handle                  hRegister );

void BHDM_P_Mhl_Cbus_WakeupAndDiscovery_isr
	( BREG_Handle                  hRegister,
	  uint32_t                     ulDiscoveryTimeout,
	  BHDM_P_Mhl_CbusDiscovery    *peDiscovery );

void BHDM_P_Mhl_Cbus_GetDiscoveryStatus
	( BREG_Handle                  hRegister,
	  BHDM_P_Mhl_CbusDiscovery    *peDiscovery );
#endif

#ifdef __cplusplus
}
#endif

#endif /* BHDM_MHL_CBUS_PRIV_H__ */
