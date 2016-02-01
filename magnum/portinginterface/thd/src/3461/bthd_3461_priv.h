/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef _BTHD_3461_PRIV_H__
#define _BTHD_3461_PRIV_H__

#include "bthd_3461.h"
#include "btmr.h"

#define BTHD_CHK_RETCODE(x) \
	{ if ((retCode = (x)) != BERR_SUCCESS) goto done; }

/* Defines raw HAB test mesg hdr (struct) */
#define HAB_MSG_HDR(OPCODE,N,CORE_TYPE,CORE_ID) \
    { ((uint8_t)(((uint16_t)(OPCODE)) >> 2)), \
    (((uint8_t)(0x03 & (OPCODE)) << 6) | ((uint8_t)((N)>>4))), \
    ((((uint8_t)(((N)& 0x0F) << 4))) | ((uint8_t)(0x0F & (CORE_TYPE)))), \
    ((uint8_t)(CORE_ID)) }
    
#define BTHD_CORE_TYPE              0x4
#define BTHD_CORE_ID                0x0
#define CORE_TYPE_GLOBAL            0x0
#define HAB_MEM_SIZE    	        128
#define BTHD_CONFIG_PARAMS_BUF1     0x10
#define BTHD_CONFIG_PARAMS_BUF2     0x40
#define BTHD_CONFIG_PARAMS_BUF3     0

typedef enum BTHD_OpCodesDS{
    BTHD_eAcquire = 0x10,
    BTHD_eAcquireParamsWrite = 0x11,
    BTHD_eAcquireParamsRead = 0x91,
    BTHD_eAcquireParamsWriteInternal = 0x003,
    BTHD_eAcquireParamsReadInternal = 0x103,
    BTHD_eAnnexASymbolRateWrite = 0x12,
    BTHD_eAnnexASymbolRateRead = 0x92,
    BTHD_eScanParamsWrite = 0x13,
    BTHD_eScanParamsRead = 0x93,
    BTHD_eAcqWordsWrite = 0x14,
    BTHD_eAcqWordsRead = 0x94,
    BTHD_eResetStatus = 0x15,
    BTHD_eRequestAsyncStatus = 0x16,
    BTHD_eGetStatus = 0x96,
    BTHD_eGetConstellation = 0xA3,
    BTHD_eGetVersion = 0xB9,
    BTHD_eGetVersionInfo = 0xBA,
    BTHD_ePowerCtrlOn = 0x19,
    BTHD_ePowerCtrlOff = 0x18,
    BTHD_ePowerCtrlRead = 0x98,
    BTHD_eConfigParamsWrite = 0x1A 
}BTHD_OpCodesDS;
    
#ifndef BTHD_POWER_MANAGEMENT
#define BTHD_POWER_MANAGEMENT
#endif

/***************************************************************************
Summary:
	Enumeration for QAM mode return from THD core

Description:
	This enum specifies the type of QAM mode THD can receive

See Also:
	None.

****************************************************************************/
typedef enum BTHD_QamMode
{
	BTHD_QamMode_Qpsk,
	BTHD_QamMode_16Qam,
	BTHD_QamMode_64Qam
} BTDH_QamMode;

/***************************************************************************
Summary:
	Structure for chip-specific THD handle

Description:
	This is the chip-specific component of the BTHD_Handle.

See Also:
	None.

****************************************************************************/
typedef struct BTHD_3461_P_Handle
{
    BREG_Handle  hRegister;         /* handle used to access AP registers */
    BHAB_DevId devId;   
    BHAB_Handle      hHab;     
    BKNI_EventHandle hInterruptEvent;
    BKNI_EventHandle hLockChangeEvent;  /* change of lock status event handle */
    BKNI_EventHandle hInitDoneEvent;    /* AP initialization done event handle */
    BKNI_EventHandle hUnlockEvent;
    BKNI_EventHandle hLockEvent;
    BKNI_EventHandle hHabDoneEvent;     /* HAB done event handle */
    BINT_CallbackHandle hInitApCallback;
    BINT_CallbackHandle hHabDoneCallback;
    BINT_CallbackHandle hUnlockCallback;
    BINT_CallbackHandle hLockCallback;
    BTMR_TimerHandle hTimer;
    bool bPowerdown;
    bool bPowerOffToOn;
    uint32_t			   ulStuckCount;
    BTHD_CallbackFunc pCallback[BTHD_Callback_eLast];
    void *pCallbackParam[BTHD_Callback_eLast];   
    uint32_t magicId;
    bool previousTuneAcquire;
    BTHD_InbandParams previousAcquireParams;
    uint8_t readBuf[HAB_MEM_SIZE];    
} BTHD_3461_P_Handle;

/***************************************************************************
Summary:
	Structure for chip-specific Acquire Parameters

Description:
	This is the chip-specific component of the Thd Acquire command. It maps
    byte for byte to the acquire parameter inside the chip's firmware

See Also:
	None.

****************************************************************************/
typedef struct BTHD_3461_P_AcquireParam
{
    uint32_t AcquireFlag;
    uint8_t TransMode;
    uint8_t Guard;
    uint8_t Setup;
    uint8_t CciMode;
    uint8_t ImpMode;
    uint8_t DivMode;
    uint8_t Decode;
    uint8_t Bandwidth;
    uint32_t CenterFreq;
    uint32_t SampleFreq;
    uint32_t RfFreq;
    int16_t RFTarget;
    int16_t BBTarget;
} BTHD_3461_P_AcquireParam;

/***************************************************************************
Summary:
	Structure for THD status

Description:
	This structure contains THD status that is return by the chip

See Also:
	None.

****************************************************************************/
typedef struct BTHD_P_3461_THDStatus
{
    uint8_t  bytLock; /* 1 */
    uint8_t  bytDecode;
    uint8_t  bytN;
    uint8_t  bytGuard;
    uint8_t  bytQam;
    uint8_t  bytHiearchy;
    uint8_t  bytIDepth;
    uint8_t  bytRateHp;
    uint8_t  bytRateLp; /* 9 */
    int32_t  lCarrierOffset;
    int32_t  lTimingOffset; /* 17 */
    uint16_t  nRAgcGain;
    uint16_t  nIAgcGain;
    uint16_t  nDagc1Gain;
    uint16_t  nDagc2Gain;
    uint16_t  nSnr;
    uint16_t  nSnrData;
    uint16_t  nSnrPilot; /* 31 */
    uint32_t  ulPreVitBer; /* 35 */
    uint32_t  ulVitBer; /* 39 */
    uint32_t  ulTSPer; /* 43 */
    uint32_t  ulTSCerc; 
    uint32_t  ulTSUberc;
    uint32_t  ulTSCberc;
    uint32_t  ulTSNberc;
    uint32_t  ulTSTberc;
    uint32_t  ulTSUferc;
    uint32_t  ulTSNferc;
    uint32_t  ulTSTferc;
    uint32_t  ulReacquireCount;
    uint16_t  nRssi;
    uint8_t   bytTS_Esr;
} BTHD_P_3461_THDStatus;

/******************************************************************************
Summary:
   Initiates THD acquisition on the BCM3461.
Description:
   This function sends the Acquire_Thd HAB command to the AP.
Returns:
   BERR_Code
******************************************************************************/

BERR_Code BTHD_3461_P_AcquireThd(
    BTHD_Handle h,                      /* [in] BTHD handle */
    const BTHD_InbandParams *pParams    /* [in] inband acquisition parameters */
    );


/******************************************************************************
Summary:
   Turn on power to THD block
Description:
   This function enables clocks to THD. 
Returns:
   none
******************************************************************************/
BERR_Code BTHD_3461_P_PowerEnable(
    BTHD_Handle h,   /* [in] BTHD handle */
    uint32_t uiFlags
    );

/******************************************************************************
Summary:
   Turn On power to THD block
Description:
   This function enable clocks to THD.
Returns:
   none
******************************************************************************/
BERR_Code BTHD_3461_P_PowerUp(
    BTHD_Handle h   /* [in] BTHD handle */
    );
    
/******************************************************************************
Summary:
   Turn off power to THD block
Description:
   This function disable clocks to THD.
Returns:
   none
******************************************************************************/
BERR_Code BTHD_3461_P_PowerDown(
    BTHD_Handle h   /* [in] BTHD handle */
    );


/***************************************************************************
Summary:
	Internal HAB command defintion

Description:

See Also:

****************************************************************************/
#define HAB_VERSION_CMD 0x09
#define HAB_ACQUIRE_THD_CMD 0x0A
#define HAB_ACQUIRE_IDLE 0x0B
#define HAB_RESET_THD_STATUS_CMD 0x0D
#define HAB_PAD_CONTROL_CMD 0x0E
#define HAB_CONFIG_READ_CMD 0x12
#define HAB_CONFIG_WRITE_CMD 0x13
#define HAB_CONFIG_EDIT 0x14
#define HAB_THD_STATUS 0x15
#define HAB_ACK_BIT 0x80

/***************************************************************************
Summary:
	THD Acquire flag definition

Description:

See Also:

****************************************************************************/
#define AUTO_ACQUIRE (1<<0)
#define INVERT_SPECTRUM (1<<1)
#define GUARD_ACQUIRE (1<<2)
#define TPS_ACQUIRE (1<<3)
#define WIDE_ACQUIRE (1<<4)
#define PARALLEL_OUT (3<<9)
#define SERIAL_OUT (4<<9)
#define IF_DELSIG_INV (1<<16)
#define RF_DELSIG_INV (1<<17)
#define IF_AGC_ZERO (1<<18)
#define AUTO 0
#define THD_STATUS_LEN 87
#define SAMPLE_RATE 54000000
#define CENTER_FREQ 36000000
#define RF_TARGET -44
#define BB_TARGET -10
#define SETUP_AUTO 0
#define SETUP_FIXED 1
#define AUDIO_SIF_MODE 0x10

/* BCM3461 implementation of API functions */
BERR_Code BTHD_3461_P_Open(BTHD_Handle *, BCHP_Handle, void*, BINT_Handle, const struct BTHD_Settings*);
BERR_Code BTHD_3461_P_Close(BTHD_Handle);
BERR_Code BTHD_3461_P_Init(BTHD_Handle, const uint8_t *, uint32_t);
BERR_Code BTHD_3461_P_SetAcquireParams(BTHD_Handle, const BTHD_InbandParams *);
BERR_Code BTHD_3461_P_GetAcquireParams(BTHD_Handle, BTHD_InbandParams *);
BERR_Code BTHD_3461_P_TuneAcquire(BTHD_Handle, const BTHD_InbandParams *);
BERR_Code BTHD_3461_P_RequestThdAsyncStatus(BTHD_Handle);
BERR_Code BTHD_3461_P_GetThdAsyncStatus(BTHD_Handle, BTHD_THDStatus *);
BERR_Code BTHD_3461_P_GetThdStatus(BTHD_Handle, BTHD_THDStatus *);
BERR_Code BTHD_3461_P_GetThdLockStatus(BTHD_Handle, BTHD_LockStatus *pLockStatus);
BERR_Code BTHD_3461_P_GetChipRevision(BTHD_Handle, uint8_t*);
BERR_Code BTHD_3461_P_GetVersion(BTHD_Handle, uint32_t*, uint32_t*);
BERR_Code BTHD_3461_P_GetVersionInfo(BTHD_Handle, BFEC_VersionInfo *);
BERR_Code BTHD_3461_P_GetInterruptEventHandle(BTHD_Handle, BKNI_EventHandle*);
BERR_Code BTHD_3461_P_ResetInbandStatus(BTHD_Handle);
BERR_Code BTHD_3461_P_ProcessInterruptEvent(BTHD_Handle);
BERR_Code BTHD_3461_P_GetSoftDecisionBuf( BTHD_Handle, int16_t*, int16_t*);
BERR_Code BTHD_3461_P_GetLockStateChangeEvent( BTHD_Handle, BKNI_EventHandle*);
BERR_Code BTHD_3461_P_GetLockEvent(BTHD_Handle, BKNI_EventHandle*);
BERR_Code BTHD_3461_P_GetUnlockEvent(BTHD_Handle, BKNI_EventHandle*);
BERR_Code BTHD_3461_P_InstallCallback(BTHD_Handle h, BTHD_Callback callbackType, BTHD_CallbackFunc pCallbackFunc, void *pParam);

#ifdef __cplusplus
}
#endif

#endif /* BTHD_3461_P_PRIV_H__ */

