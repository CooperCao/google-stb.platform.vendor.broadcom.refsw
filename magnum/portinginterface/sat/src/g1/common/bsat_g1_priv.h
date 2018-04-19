/******************************************************************************
* Copyright (C) 2018 Broadcom.
* The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to
* the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied),
* right to use, or waiver of any kind with respect to the Software, and
* Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
* THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
* IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization,
* constitutes the valuable trade secrets of Broadcom, and you shall use all
* reasonable efforts to protect the confidentiality thereof, and to use this
* information only in connection with your use of Broadcom integrated circuit
* products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
* "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
* OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
* RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
* IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
* A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
* ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
* THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
* OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
* INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
* RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
* HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
* EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
* WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
* FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* Module Description:
*
*****************************************************************************/
#ifndef _BSAT_G1_PRIV_H_
#define _BSAT_G1_PRIV_H_

#include "bstd.h"
#include "bsat_g1.h"
#if (BCHP_CHIP==7366)
#ifndef BYPASS_LEAP
   #define BSAT_HAS_LEAP
#endif
   #define BSAT_EXCLUDE_MI2C
   /* #define BSAT_EXCLUDE_SPUR_CANCELLER */
   #include "bsat_7366_priv.h"
#elif (BCHP_CHIP==45216)
   #define BSAT_HAS_LEAP
   #define BSAT_EXCLUDE_MI2C
   /* #define BSAT_EXCLUDE_SPUR_CANCELLER */
   #include "bsat_45216_priv.h"
#elif (BCHP_CHIP==45308)
   #define BSAT_HAS_LEAP
   #define BSAT_EXCLUDE_MI2C
   #define BSAT_HAS_DVBS2X
   #define BSAT_HAS_DUAL_TFEC
   #define BSAT_HAS_ACM
   #include "bsat_45308_priv.h"
#elif (BCHP_CHIP==4538)
   #define BSAT_HAS_LEAP
   #define BSAT_EXCLUDE_MI2C
   #define BSAT_EXCLUDE_SPUR_CANCELLER
   #include "bsat_4538_priv.h"
#elif (BCHP_CHIP==7364)
   #define BSAT_EXCLUDE_MI2C
   #include "bsat_7364_priv.h"
#elif (BCHP_CHIP==45402)
   #define BSAT_HAS_LEAP
   #define BSAT_EXCLUDE_MI2C
   #define BSAT_HAS_DVBS2X
   #define BSAT_HAS_DUAL_TFEC
   #define BSAT_HAS_ACM
   #include "bsat_45402_priv.h"
#else
   #error "unsupported BCHP_CHIP"
#endif

/* define any of the following to limit features in the SAT PI:
   BSAT_EXCLUDE_AFEC
   BSAT_EXCLUDE_TFEC
   BSAT_EXCLUDE_MI2C
   BSAT_EXCLUDE_SPUR_CANCELLER
*/


#define BSAT_G1_MAX_NOTCH 16

#define BSAT_CHK_RETCODE(x) { if ((retCode = (x)) != BERR_SUCCESS) goto done; }

typedef BERR_Code (*BSAT_g1_FUNCT)(BSAT_ChannelHandle);


/* timers */
typedef enum BSAT_TimerSelect
{
   BSAT_TimerSelect_eBaud = 0,
   BSAT_TimerSelect_eBaudUsec,
   BSAT_TimerSelect_eBer,  /* used for delays between successive register accesses in bclk domain */
   BSAT_TimerSelect_eGen1, /* used to implement reacquisition timeout */
   BSAT_TimerSelect_eGen2, /* used for stable_lock timer and lock monitor activities */
   BSAT_TimerSelect_eGen3  /* used to measure acquisition time */
} BSAT_TimerSelect;

#define BSAT_TimerSelect_eReacqTimer       BSAT_TimerSelect_eGen1
#define BSAT_TimerSelect_eStableLockTimer  BSAT_TimerSelect_eGen2
#define BSAT_TimerSelect_eMonitorLockTimer BSAT_TimerSelect_eGen2


/* reason for reacquisition */
typedef enum BSAT_ReacqCause
{
   BSAT_ReacqCause_eOK = 0,
   BSAT_ReacqCause_eTimingLoopNotLocked,         /* 1=timing loop could not be locked */
   BSAT_ReacqCause_eFecNotStableLock,            /* 2=reacquisition timer expired due to FEC not able to stay in lock */
   BSAT_ReacqCause_eInvalidMode,                 /* 3=restricted mode */
   BSAT_ReacqCause_eCarrierOffsetOutOfRange,     /* 4=carrier offset drifted beyond search range */
   BSAT_ReacqCause_eInvalidModcod,               /* 5=decoded modcod is invalid */
   BSAT_ReacqCause_eModcodNotStable,             /* 6=modcod not stable */
   BSAT_ReacqCause_eTimerError,                  /* 7=BSAT_EnableTimer() returned error */
   BSAT_ReacqCause_eHpLostLock,                  /* 8=HP fell out of lock */
   BSAT_ReacqCause_eHpCouldNotLock,              /* 9=HP could not reach state 5 */
   BSAT_ReacqCause_eNoAfec,                      /* 10=LDPC acquisition initiated but chip does not have AFEC */
   BSAT_ReacqCause_eNoTfec,                      /* 11=turbo acquisition initiated but chip does not have TFEC */
   BSAT_ReacqCause_eVitNotLock,                  /* 12=no Viterbi lock in legacy QPSK mode */
   BSAT_ReacqCause_eUnableToDetermineActualMode, /* 13=legacy qpsk mode: puncture rate status not stable */
   BSAT_ReacqCause_eTurboSyncTimeout,            /* 14=no turbo sync seen */
   BSAT_ReacqCause_eCodeRateChanged,             /* 15=legacy qpsk mode: detected code rate changed */
   BSAT_ReacqCause_eDftTimeout,                  /* 16=DFT done timeout */
   BSAT_ReacqCause_eInvalidCondition,            /* 17=invalid condition (sanity check failed) */
   BSAT_ReacqCause_eAfecFlushFailed,             /* 18=AFEC flush timeout */
   BSAT_ReacqCause_eFrameLengthCannotMix,        /* 19=dual AFEC core cannot mix normal/short frames */
   BSAT_ReacqCause_eDvbs2xNotSupported           /* 20=DVB-S2X not supported */
} BSAT_ReacqCause;


/* acquisition states */
typedef enum BSAT_AcqState
{
   BSAT_AcqState_eIdle = 0,
   BSAT_AcqState_eTuning,
   BSAT_AcqState_eAcquiring,
   BSAT_AcqState_eWaitForInitialLock,
   BSAT_AcqState_eWaitForStableLock,
   BSAT_AcqState_eMonitorLock
} BSAT_AcqState;


/* type of acquisition */
typedef enum BSAT_Operation
{
   BSAT_Operation_eAcq = 0,
   BSAT_Operation_eSignalDetect,
   BSAT_Operation_eSymbolRateScan,
   BSAT_Operation_eToneDetect,
   BSAT_Operation_ePsd,
   BSAT_Operation_eSa
} BSAT_Operation;


typedef enum BSAT_AcqType
{
   BSAT_AcqType_eQpsk = 0,
   BSAT_AcqType_eDvbs2,
   BSAT_AcqType_eTurbo
} BSAT_AcqType;


/* bit definitions for turboScanState */
#define BSAT_TURBO_SCAN_STATE_HP_INIT        0x01
#define BSAT_TURBO_SCAN_STATE_HP_LOCKED      0x02
#define BSAT_TURBO_SCAN_STATE_8PSK_HP_LOCKED 0x04
#define BSAT_TURBO_SCAN_STATE_8PSK_FAILED    0x08
#define BSAT_TURBO_SCAN_STATE_FIRST_ACQ      0x20
#define BSAT_TURBO_SCAN_STATE_SYNC_ACQUIRED  0x40
#define BSAT_TURBO_SCAN_STATE_FIRST_PASS     0x80

/* bit definitions for ldpcScanState */
#define BSAT_DVBS2_SCAN_STATE_QPSK          0x02
#define BSAT_DVBS2_SCAN_STATE_PILOT         0x01
#define BSAT_DVBS2_SCAN_STATE_ENABLED       0x80
#define BSAT_DVBS2_SCAN_STATE_FOUND         0x40
#define BSAT_DVBS2_SCAN_STATE_MASK          0xC0

/* BSAT_g1_P_Handle is the data type pointed to by BSAT_P_Handle->pImpl */
typedef struct BSAT_g1_P_Handle
{
   BCHP_Handle           hChip;                 /* chip handle */
   BREG_Handle           hRegister;             /* register handle */
   BINT_Handle           hInterrupt;            /* interrupt handle */
   BKNI_EventHandle      hInitDoneEvent;        /* initialization done event handle */
   BSAT_NotchSettings    notchSettings[BSAT_G1_MAX_NOTCH]; /* notch settings for spur cancellation */
#ifndef BSAT_EXCLUDE_SPUR_CANCELLER
   uint32_t              cwcFreq[6];
#endif
   uint32_t              xtalFreq;              /* crystal freq in Hz */
   uint32_t              chipFamily;            /* FAMILY_ID */
   BSAT_NetworkSpec      networkSpec;           /* network spec */
   uint32_t              *pSaBuf;               /* memory area for SA data */
   uint32_t              afecRampSettings;      /* BSAT_g1_CONFIG_AFEC_RAM configuration parameter */
   bool                  bResetDone;            /* true if BSAT PI was just completed reset */
   uint16_t              afecRampChanMask;          /* bitmask indicating which channels have afec lock */
   uint16_t              afecRampLowThreshChanMask; /* bitmask indicating which afec locked channels have ave iter below threshold */
   uint8_t               sdsRevId;              /* SDS core revision ID */
   uint8_t               nNotch;                /* number of spurs cancelled by DCO notch filter */
   uint8_t               acqDoneThreshold;      /* number of failed reacqs before setting acq done event */
} BSAT_g1_P_Handle;


typedef struct BSAT_g1_ModeFunct
{
   BERR_Code (*Acquire)(BSAT_ChannelHandle);
   bool (*IsSpinv)(BSAT_ChannelHandle);
   bool (*IsValidMode)(BSAT_ChannelHandle);
   BERR_Code (*OnLock)(BSAT_ChannelHandle);
   BERR_Code (*OnLostLock)(BSAT_ChannelHandle);
   BERR_Code (*OnStableLock)(BSAT_ChannelHandle);
   BERR_Code (*EnableLockInterrupts)(BSAT_ChannelHandle, bool bEnable);
   BERR_Code (*OnMonitorLock)(BSAT_ChannelHandle);
   bool (*IsLocked)(BSAT_ChannelHandle);
   BERR_Code (*Reacquire)(BSAT_ChannelHandle);
} BSAT_g1_ModeFunct;


typedef struct BSAT_g1_P_AcmStatus
{
   BSAT_Mode mode;
   bool    bSpinv;
   bool    bPilot;
   uint8_t modcod;
   uint8_t type;
} BSAT_g1_P_AcmStatus;


/* BSAT_g1_P_ChannelHandle is the data type pointed to by BSAT_P_ChannelHandle->pImpl */
typedef struct BSAT_g1_P_ChannelHandle
{
   BKNI_EventHandle     hLockChangeEvent;      /* change of lock status event handle */
   BKNI_EventHandle     hAcqDoneEvent;         /* peak scan event handle */
   BKNI_EventHandle     hSignalNotificationEvent;    /* signal notification event handle */
   BKNI_EventHandle     hReadyEvent;           /* ready event handle */
   BINT_CallbackHandle  hBaudTimerCb;          /* callback handle for baud clock timer interrupt */
   BINT_CallbackHandle  hBerTimerCb;           /* callback handle for BER timer interrupt */
   BINT_CallbackHandle  hGen1TimerCb;          /* callback handle for general timer 1 interrupt */
   BINT_CallbackHandle  hGen2TimerCb;          /* callback handle for general timer 2 interrupt */
   BINT_CallbackHandle  hGen3TimerCb;          /* callback handle for general timer 3 interrupt */
   BINT_CallbackHandle  hQpskLockCb;           /* callback handle for Legacy mode lock interrupt */
   BINT_CallbackHandle  hQpskNotLockCb;        /* callback handle for Legacy mode not lock interrupt */
   BINT_CallbackHandle  hHpFrameBoundaryCb;    /* callback handle for HP frame boundary */
   BINT_CallbackHandle  hHpStateMatchCb;       /* callback handle for HP state match interrupt */
   BINT_CallbackHandle  hHpStateChangeCb;      /* callback handle for HP state change interrupt */
   BINT_CallbackHandle  hDftDoneCb;            /* callback handle for DFT done interrupt */
   BSAT_g1_FUNCT        baudTimerIsr;          /* function for handling baud timer interrupt */
   BSAT_g1_FUNCT        berTimerIsr;           /* function for handling BER timer interrupt */
   BSAT_g1_FUNCT        gen1TimerIsr;          /* function for handling general timer 1 interrupt */
   BSAT_g1_FUNCT        gen2TimerIsr;          /* function for handling general timer 2 interrupt */
   BSAT_g1_FUNCT        gen3TimerIsr;          /* function for handling general timer 3 interrupt */
   BSAT_g1_FUNCT        nextFunct;             /* general function pointer used in acquisition state machine */
   BSAT_g1_FUNCT        next2Funct;
   BSAT_g1_FUNCT        passFunct;             /* general function pointer used in acquisition state machine */
   uint32_t             configParam[BSAT_g1_CONFIG_MAX]; /* configuration parameters */
   uint32_t             trace[BSAT_TraceEvent_eMax]; /* trace buffer */
   uint32_t             dftBinPower[32];       /* used for symbol rate scan */
   uint32_t             fecFreq;               /* FEC (Turbo/LDPC) clock frequency in Hz */
   uint32_t             searchRange;           /* search range in Hz */
   uint32_t             outputBitrate;         /* output bitrate in bps */
   uint32_t             reacqCount;            /* number of reacquisitions */
   uint32_t             sampleFreq;            /* sample frequency in Hz */
   uint32_t             aciBandwidth;          /* ACI filter bandwidth */
   uint32_t             notification;          /* signal notification indicators */
   uint32_t             rainFadeSnrAve;        /* ave snr in 1/8 dB during rain fade window */
   uint32_t             rainFadeSnrMax;        /* max snr in 1/8 dB during rain fade window */
   uint32_t             stableLockTimeout;     /* timeout in usecs for determining stable lock */
   uint32_t             maxStableLockTimeout;  /* maximum timeout in usecs for determining stable lock */
   uint32_t             actualTunerFreq;       /* actual tuner frequency in Hz */
   uint32_t             count1;                /* general counter used in acquisition */
   uint32_t             count2;                /* general counter used in acquisition */
   uint32_t             count3;                /* general counter used in acquisition */
   uint32_t             maxCount1;             /* general counter used in acquisition */
   uint32_t             maxCount2;             /* general counter used in acquisition */
   uint32_t             functState;            /* specifies current state in acquisition state machine */
   uint32_t             tunerIfStepSize;       /* internal tuner IF step size in Hz used in freq scan algorithm */
   uint32_t             opll_N;                /* calculated OPLL N value */
   uint32_t             opll_D;                /* calculated OPLL D value */
   uint32_t             mpegErrorCount;        /* accumulated MPEG errors */
   uint32_t             mpegFrameCount;        /* accumulated MPEG frames */
   uint32_t             carrierAcq1Bw;         /* acquisition carrier loop bandwidth in Hz */
   uint32_t             carrierAcq1Damp;       /* acquisition carrier loop damping factor scaled 2^2 */
   uint32_t             carrierAcq2Bw;         /* viterbi carrier loop bandwidth in Hz */
   uint32_t             carrierAcq2Damp;       /* viterbi carrier loop damping factor scaled 2^2 */
   uint32_t             carrierAcq3Bw;         /* narrow carrier loop bandwidth in Hz, previously cmin */
   uint32_t             carrierAcq3Damp;       /* narrow carrier loop damping factor scaled 2^2 */
   uint32_t             carrierTrkBw;          /* final tracking carrier loop bandwidth in Hz */
   uint32_t             carrierTrkDamp;        /* final tracking carrier loop damping factor scaled 2^2 */
   uint32_t             baudAcqBw;             /* baud loop bandwidth in Hz */
   uint32_t             baudAcqDamp;           /* baud loop damping factor scaled 2^2 */
   uint32_t             baudTrkBw;             /* final baud loop bandwidth in Hz, previous bmin */
   uint32_t             baudTrkDamp;           /* final baud loop damping factor scaled 2^2 */
   uint32_t             carrierDelay;          /* LockViterbi dwell time */
   uint32_t             totalBlocks;           /* total block count */
   uint32_t             badBlocks;             /* bad block count */
   uint32_t             corrBlocks;            /* corrected block count */
   uint32_t             timeSinceStableLock;   /* in units of 100 msecs */
#ifdef BSAT_HAS_ACM
   uint32_t             mpegFrameCountMs[8];   /* multi-stream accumulated MPEG frames */
   uint32_t             mpegErrorCountMs[8];   /* multi-stream accumulated MPEG errors */
#endif
   int32_t              lockFilterRamp;        /* used in lock filtering */
   int32_t              tunerIfStep;           /* tuner offset */
   int32_t              tunerIfStepMax;        /* used in freq scan */
   int32_t              freqTransferInt;       /* used in pli-to-fli leak */
   int32_t              initFreqOffset;        /* used in carrier freq drift threshold */
   int32_t              sum;
   BSAT_AcqSettings     acqSettings;           /* acquisition settings from most recent call to BSAT_Acquire() */
   BSAT_BertSettings    bertSettings;          /* BERT settings */
   BSAT_OutputTransportSettings xportSettings; /* transport settings */
   BSAT_SignalNotificationSettings sigNotSettings;   /* signal notification settings */
   BSAT_LegacyQpskAcqSettings qpskSettings;    /* legacy QPSK acquisition settings */
   BSAT_Dvbs2AcqSettings dvbs2Settings;        /* DVB-S2 acquisition settings */
   BSAT_Dvbs2xAcqSettings dvbs2xSettings;      /* DVB-S2X acquisition settings */
   BSAT_TurboAcqSettings turboSettings;        /* Turbo acquisition settings */
   BSAT_ExtAcqSettings  miscSettings;          /* extended acquisition settings */
   BSAT_ScanSpectrumSettings saSettings;       /* input parameters for spectrum scan */
   BSAT_SpectrumStatus  saStatus;              /* spectrum scan status */
#ifdef BSAT_HAS_ACM
   BSAT_AcmSettings     acmSettings;           /* ACM settings */
#endif
   BSAT_Mode            actualMode;            /* actual mode locked */
   BSAT_SignalDetectStatus   signalDetectStatus;   /* signal detect status */
   BSAT_SymbolRateScanStatus symbolRateScanStatus; /* symbol rate scan status */
   BSAT_ToneDetectStatus     toneDetectStatus;     /* tone detect status */
   BSAT_PsdScanStatus   psdScanStatus;         /* PSD status */
   BSAT_QpskStatus      qpskStatus;            /* legacy qpsk error counters */
   BSAT_ReacqCause      reacqCause;            /* cause for reacquisition */
   BSAT_AcqState        acqState;              /* acquisition state */
   BSAT_AcqType         acqType;               /* type of acquisition to perform */
   BSAT_Operation       operation;             /* type of acquisition performed */
#ifndef BSAT_EXCLUDE_AFEC
   BINT_CallbackHandle  hAfecLockCb;           /* callback handle for AFEC lock interrupt */
   BINT_CallbackHandle  hAfecNotLockCb;        /* callback handle for AFEC not lock interrupt */
   BINT_CallbackHandle  hAfecMpegLockCb;       /* callback handle for AFEC MPEG lock interrupt */
   BINT_CallbackHandle  hAfecMpegNotLockCb;    /* callback handle for AFEC MPEG not lock interrupt */
   BSAT_AmcScramblingSeq scramblingSeq;        /* AMC scrambling sequence */
#ifdef BSAT_HAS_DVBS2X
   uint8_t              lutMode;
#else
   bool                 bAfecFlushFailed;
#endif
#endif
#ifndef BSAT_EXCLUDE_TFEC
   BINT_CallbackHandle  hTfecLockCb;           /* callback handle for TFEC lock interrupt */
   BINT_CallbackHandle  hTfecNotLockCb;        /* callback handle for TFEC not lock interrupt */
   BINT_CallbackHandle  hTfecSyncCb;           /* callback handle for TFEC sync detected */
   uint32_t             turboScanCurrMode;     /* used in turbo scan mode */
   uint32_t             turboScanLockedMode;
#endif
#ifndef BSAT_EXCLUDE_MI2C
   BINT_CallbackHandle  hMi2cCb;               /* callback handle for MI2C interrupt */
   BKNI_EventHandle     hMi2cEvent;            /* mi2c event handle */
#endif
#ifndef BSAT_EXCLUDE_SPUR_CANCELLER
   bool                 bCwcActive[6];
#endif
   bool                 bHasAfec;              /* true if this channel supports DVB-S2 */
   bool                 bHasTfec;              /* true if this channel supports Turbo */
   bool                 bAcqDone;              /* true if acq done event has been set for current acquisition */
   bool                 bPlcTracking;          /* true if PLC tracking bw set */
   bool                 bLocked;               /* true if demod and fec are locked */
   bool                 bLastLocked;           /* filtered last lock status */
   bool                 bEverStableLock;       /* true if current acquisition was ever stable locked */
   bool                 bStableLock;           /* true if currently stable locked */
   bool                 bBlindScan;            /* true if blind scan is enabled */
   bool                 bUndersample;          /* true if in undersample mode */
   bool                 bForceReacq;           /* true if reacquisition needs to be done */
   bool                 bTimingLoopLock;       /* true if timing loop is locked */
   bool                 bShortFrame;           /* true if DVB-S2 short frames detected */
   bool                 bOverrideAciBandwidth; /* true if manually set by the host */
   bool                 bReacqTimerExpired;    /* true if reacquisition timer expired */
   bool                 bAbortAcq;             /* true if acquisition stopped by host */
   bool                 bFlag1;                /* general purpose */
   bool                 bEnableFineFreqEst;    /* true if fine frequency estimation is enabled */
   bool                 bAfecRampEnabled;      /* true if AFEC ramping is enabled */
   uint8_t              blindScanCurrMode;     /* used in blind acquisition */
   uint8_t              relockCount;           /* number of times relocked without doing reacquisition */
   uint8_t              dvbs2ScanState;        /* used in DVB-S2 modcod scan mode */
   uint8_t              dtvScanState;          /* used for manual DTV 1/2 scan in DTV scan mode */
   uint8_t              lockIsrFlag;           /* last lock isr state */
#ifndef BSAT_EXCLUDE_TFEC
   uint8_t              turboScanState;        /* used in Turbo scan mode */
   uint8_t              turboScanLockedModeFailures;
#endif
   int8_t               notchState;
} BSAT_g1_P_ChannelHandle;


/* API implementation functions */
BERR_Code BSAT_g1_P_Open(BSAT_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInterrupt, const BSAT_Settings *pSettings);
BERR_Code BSAT_g1_P_Close(BSAT_Handle h);
BERR_Code BSAT_g1_P_GetTotalChannels(BSAT_Handle h, uint32_t *totalChannels);
BERR_Code BSAT_g1_P_OpenChannel(BSAT_Handle h, BSAT_ChannelHandle *pChannelHandle, uint32_t chan, const BSAT_ChannelSettings *pSettings);
BERR_Code BSAT_g1_P_CloseChannel(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_GetDevice(BSAT_ChannelHandle h, BSAT_Handle *pDev);
BERR_Code BSAT_g1_P_GetVersionInfo(BSAT_Handle h, BFEC_VersionInfo *pVersion);
BERR_Code BSAT_g1_P_Reset(BSAT_Handle h);
BERR_Code BSAT_g1_P_PowerDownChannel(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_PowerUpChannel(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_IsChannelOn(BSAT_ChannelHandle h, bool *pOn);
BERR_Code BSAT_g1_P_Acquire(BSAT_ChannelHandle h, BSAT_AcqSettings *pParams);
BERR_Code BSAT_g1_P_GetLockStatus(BSAT_ChannelHandle h, bool *pbLocked, bool *pbAcqDone);
BERR_Code BSAT_g1_P_GetChannelStatus(BSAT_ChannelHandle h, BSAT_ChannelStatus *pStatus);
BERR_Code BSAT_g1_P_ResetChannelStatus(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_GetSoftDecisions(BSAT_ChannelHandle h, uint32_t n, int16_t *pI, int16_t *pQ);
BERR_Code BSAT_g1_P_ResetChannel(BSAT_ChannelHandle h, bool bDisableDemod);
BERR_Code BSAT_g1_P_SetBertSettings(BSAT_ChannelHandle h, BSAT_BertSettings *pSettings);
BERR_Code BSAT_g1_P_GetBertSettings(BSAT_ChannelHandle h, BSAT_BertSettings *pSettings);
BERR_Code BSAT_g1_P_GetBertStatus(BSAT_ChannelHandle h, BSAT_BertStatus *pStatus);
BERR_Code BSAT_g1_P_SetSearchRange(BSAT_ChannelHandle h, uint32_t searchRange);
BERR_Code BSAT_g1_P_GetSearchRange(BSAT_ChannelHandle h, uint32_t *pSearchRange);
#ifndef BSAT_EXCLUDE_AFEC
BERR_Code BSAT_g1_P_SetAmcScramblingSeq(BSAT_ChannelHandle h, BSAT_AmcScramblingSeq *pSeq);
#endif
BERR_Code BSAT_g1_P_SetNetworkSpec(BSAT_Handle h, BSAT_NetworkSpec networkSpec);
BERR_Code BSAT_g1_P_GetNetworkSpec_isrsafe(BSAT_Handle h, BSAT_NetworkSpec *pNetworkSpec);
BERR_Code BSAT_g1_P_SetOutputTransportSettings(BSAT_ChannelHandle h, BSAT_OutputTransportSettings *pSettings);
BERR_Code BSAT_g1_P_GetOutputTransportSettings(BSAT_ChannelHandle h, BSAT_OutputTransportSettings *pSettings);
BERR_Code BSAT_g1_P_GetInitDoneEventHandle(BSAT_Handle h, BKNI_EventHandle *hEvent);
BERR_Code BSAT_g1_P_GetLockStateChangeEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BSAT_g1_P_GetAcqDoneEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BSAT_g1_P_GetSignalNotificationEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BSAT_g1_P_GetReadyEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BSAT_g1_P_StartToneDetect(BSAT_ChannelHandle h, uint32_t freq, uint32_t adcSelect);
BERR_Code BSAT_g1_P_GetToneDetectStatus(BSAT_ChannelHandle h, BSAT_ToneDetectStatus *pStatus);
BERR_Code BSAT_g1_P_StartSymbolRateScan(BSAT_ChannelHandle h, uint32_t freq, uint32_t minSymbolRate, uint32_t maxSymbolRate, uint32_t adcSelect);
BERR_Code BSAT_g1_P_GetSymbolRateScanStatus(BSAT_ChannelHandle h, BSAT_SymbolRateScanStatus *pStatus);
BERR_Code BSAT_g1_P_SetConfig(BSAT_Handle h, uint32_t addr, uint32_t val);
BERR_Code BSAT_g1_P_GetConfig(BSAT_Handle h, uint32_t addr, uint32_t *pVal);
BERR_Code BSAT_g1_P_SetChannelConfig(BSAT_ChannelHandle h, uint32_t addr, uint32_t val);
BERR_Code BSAT_g1_P_GetChannelConfig(BSAT_ChannelHandle h, uint32_t addr, uint32_t *pVal);
BERR_Code BSAT_g1_P_SetAcqDoneEventSettings(BSAT_Handle h, uint8_t reacqs);
BERR_Code BSAT_g1_P_SetSignalNotificationSettings(BSAT_ChannelHandle h, BSAT_SignalNotificationSettings *pSettings);
BERR_Code BSAT_g1_P_GetSignalNotificationSettings(BSAT_ChannelHandle h, BSAT_SignalNotificationSettings *pSettings);
BERR_Code BSAT_g1_P_SetACIBandwidth(BSAT_ChannelHandle h, uint32_t bw);
BERR_Code BSAT_g1_P_StartSignalDetect(BSAT_ChannelHandle h, uint32_t symbolRate, uint32_t freq, uint32_t adcSelect);
BERR_Code BSAT_g1_P_GetSignalDetectStatus(BSAT_ChannelHandle h, BSAT_SignalDetectStatus *pStatus);
BERR_Code BSAT_g1_P_GetLegacyQpskAcqSettings(BSAT_ChannelHandle h, BSAT_LegacyQpskAcqSettings *pSettings);
BERR_Code BSAT_g1_P_SetLegacyQpskAcqSettings(BSAT_ChannelHandle h, BSAT_LegacyQpskAcqSettings *pSettings);
BERR_Code BSAT_g1_P_StartPsdScan(BSAT_ChannelHandle h, uint32_t freq, uint32_t adcSelect);
BERR_Code BSAT_g1_P_GetPsdScanStatus(BSAT_ChannelHandle h, BSAT_PsdScanStatus *pStatus);
BERR_Code BSAT_g1_P_SetAcmSettings(BSAT_ChannelHandle h, BSAT_AcmSettings *pSettings);
BERR_Code BSAT_g1_P_GetAcmSettings(BSAT_ChannelHandle h, BSAT_AcmSettings *pSettings);
BERR_Code BSAT_g1_P_GetStreamList(BSAT_ChannelHandle h, int bufsize, int *pNumStreams, uint8_t *pStreamIdList);
BERR_Code BSAT_g1_P_GetStreamStatus(BSAT_ChannelHandle h, uint8_t streamId, BSAT_StreamStatus *pStatus);
BERR_Code BSAT_g1_P_ScanSpectrum(BSAT_ChannelHandle h, BSAT_ScanSpectrumSettings *pSettings);
BERR_Code BSAT_g1_P_GetSpectrumStatus(BSAT_ChannelHandle h, BSAT_SpectrumStatus *pStatus, uint8_t *pData);

#ifndef BSAT_EXCLUDE_AFEC
BERR_Code BSAT_g1_P_GetDvbs2AcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2AcqSettings *pSettings);
BERR_Code BSAT_g1_P_SetDvbs2AcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2AcqSettings *pSettings);
#endif
#ifndef BSAT_EXCLUDE_TFEC
BERR_Code BSAT_g1_P_GetTurboAcqSettings(BSAT_ChannelHandle h, BSAT_TurboAcqSettings *pSettings);
BERR_Code BSAT_g1_P_SetTurboAcqSettings(BSAT_ChannelHandle h, BSAT_TurboAcqSettings *pSettings);
#endif
BERR_Code BSAT_g1_P_GetExtAcqSettings(BSAT_ChannelHandle h, BSAT_ExtAcqSettings *pSettings);
BERR_Code BSAT_g1_P_SetExtAcqSettings(BSAT_ChannelHandle h, BSAT_ExtAcqSettings *pSettings);
BERR_Code BSAT_g1_P_SetNotchSettings(BSAT_Handle, uint32_t, BSAT_NotchSettings*);
BERR_Code BSAT_g1_P_GetNotchSettings(BSAT_Handle, uint32_t*, BSAT_NotchSettings*);
#ifndef BSAT_EXCLUDE_SPUR_CANCELLER
BERR_Code BSAT_g1_P_SetCwc(BSAT_Handle h, uint32_t n, uint32_t *pFreqs);
#endif
#ifdef BSAT_HAS_DVBS2X
BERR_Code BSAT_g1_P_GetDvbs2xAcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2xAcqSettings *pSettings);
BERR_Code BSAT_g1_P_SetDvbs2xAcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2xAcqSettings *pSettings);
#endif

/* functions that are required to be implemented in bsat_<chip>_priv.c */
uint32_t BSAT_g1_P_ReadRegister_isrsafe(BSAT_ChannelHandle h, uint32_t reg);
void BSAT_g1_P_WriteRegister_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t val);
BERR_Code BSAT_g1_P_InitHandleExt(BSAT_Handle h);
BERR_Code BSAT_g1_P_InitChannelHandleExt(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_TunerPowerUp(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_TunerPowerDown(BSAT_ChannelHandle h);
bool BSAT_g1_P_IsSdsOn(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_SdsPowerUp(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_SdsPowerDown(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_GetAgcStatus(BSAT_ChannelHandle h, BSAT_AgcStatus *pStatus);
BERR_Code BSAT_g1_P_ValidateAcqParams(BSAT_ChannelHandle h, BSAT_AcqSettings *pParams);
#ifndef BSAT_EXCLUDE_AFEC
BERR_Code BSAT_g1_P_AfecPowerUp_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecPowerDown_isrsafe(BSAT_ChannelHandle h);
bool BSAT_g1_P_IsAfecOn_isrsafe(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_GetAfecClock_isrsafe(BSAT_ChannelHandle h, uint32_t *pFreq);
#ifndef BSAT_HAS_DVBS2X
BSAT_ChannelHandle BSAT_g1_P_AfecGetOtherChannelHandle_isrsafe(BSAT_ChannelHandle h);
#endif /* ifndef BSAT_HAS_DVBS2X */
#endif
#ifndef BSAT_EXCLUDE_TFEC
BERR_Code BSAT_g1_P_TfecPowerUp_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_TfecPowerDown_isrsafe(BSAT_ChannelHandle h);
bool BSAT_g1_P_IsTfecOn_isrsafe(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_GetTfecClock_isrsafe(BSAT_ChannelHandle h, uint32_t *pFreq);
#endif

/* functions in bsat_g1_priv_acq.c */
BERR_Code BSAT_g1_P_InitHandle(BSAT_Handle h);
BERR_Code BSAT_g1_P_InitChannelHandle(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_InitChannel(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DisableInterrupts(BSAT_Handle h);
BERR_Code BSAT_g1_P_DisableChannelInterrupts(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_StartBackgroundAcq(BSAT_ChannelHandle h);
void BSAT_g1_P_IndicateLocked_isrsafe(BSAT_ChannelHandle h);
void BSAT_g1_P_IndicateNotLocked_isrsafe(BSAT_ChannelHandle h);
void BSAT_g1_P_IndicateAcqDone_isr(BSAT_ChannelHandle h);
void BSAT_g1_P_Lock_isr(void *p, int int_id);
void BSAT_g1_P_NotLock_isr(void *p, int int_id);
BERR_Code BSAT_g1_P_SetDefaultSampleFreq(BSAT_ChannelHandle h);
void BSAT_g1_P_PowerDownOpll_isrsafe(BSAT_ChannelHandle h);
void BSAT_g1_P_PowerUpOpll_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_GetSymbolRateError(BSAT_ChannelHandle h, int32_t *pSymbolRateError);
BERR_Code BSAT_g1_P_GetCarrierError_isrsafe(BSAT_ChannelHandle h, int32_t *pCarrierError);
BERR_Code BSAT_g1_P_UpdateMpegCount(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_ResetMpegCount_isrsafe(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_PrepareNewAcquisition(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_ResetSignalNotification_isrsafe(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_ResetSignalDetectStatus(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_ResetSymbolRateScanStatus(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_ResetToneDetectStatus(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_Acquire0_isr(BSAT_ChannelHandle h);
bool BSAT_g1_P_IsValidFreq(uint32_t freq);
bool BSAT_g1_P_IsValidSymbolRate(uint32_t Fb);
uint32_t BSAT_g1_P_GetNumDecimatingFilters_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_SetFlifOffset_isr(BSAT_ChannelHandle h, int32_t offset);
BERR_Code BSAT_g1_P_Reacquire_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DisableChannelInterrupts_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_NonLegacyModeAcquireInit_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_SetAgcTrackingBw_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_ConfigOif_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_InitBert_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_StartTracking_isr(BSAT_ChannelHandle h);
void BSAT_g1_P_IncrementReacqCount_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DisableChannelInterrupts_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_SetBaudBw_isr(BSAT_ChannelHandle h, uint32_t bw, uint32_t damp);
BERR_Code BSAT_g1_P_SetCarrierBw_isr(BSAT_ChannelHandle h, uint32_t bw, uint32_t damp);
BERR_Code BSAT_g1_P_GetActualMode_isr(BSAT_ChannelHandle h, BSAT_Mode *pActualMode);
BERR_Code BSAT_g1_P_SetFfeMainTap_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecUpdateBlockCount_isrsafe(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_SetDecimationFilters_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_SetBfos_isr(BSAT_ChannelHandle h);
const BSAT_g1_ModeFunct* BSAT_g1_P_GetModeFunct_isrsafe(BSAT_AcqType t);

/* functions in bsat_g1_priv_dft.c */
void BSAT_g1_P_DftDone_isr(void *p, int int_id);
BERR_Code BSAT_g1_P_DftEnableInterrupt_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DftDisableInterrupt_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DftSearchCarrier_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT funct);
BERR_Code BSAT_g1_P_DftSymbolRateScan_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DftPsdScan_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DftToneDetect_isr(BSAT_ChannelHandle h);

/* functions in bsat_g1_priv_hp.c */
void BSAT_g1_P_HpStateMatch_isr(void *p, int int_id);
void BSAT_g1_P_HpStateChange_isr(void *p, int int_id);
void BSAT_g1_P_HpFrameBoundary_isr(void *p, int int_id);
BERR_Code BSAT_g1_P_HpDisableInterrupts_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_HpEnable(BSAT_ChannelHandle h, bool bEnable);
BERR_Code BSAT_g1_P_HpEnable_isr(BSAT_ChannelHandle h, bool bEnable);
bool BSAT_g1_P_HpIsSpinv_isrsafe(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_HpGetFreqOffsetEstimate(BSAT_ChannelHandle h, int32_t *pFreq);
BERR_Code BSAT_g1_P_HpAcquire_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT funct);
bool BSAT_g1_P_IsHpLocked_isr(BSAT_ChannelHandle h);
#ifdef BSAT_HAS_DVBS2X
BERR_Code BSAT_g1_P_GetDvbs2Plscode_isrsafe(BSAT_Mode mode, uint8_t *pPlscode);
BERR_Code BSAT_g1_P_GetDvbs2ModeFromPls_isrsafe(uint8_t pls, BSAT_Mode *pMode);
#endif

/* functions in bsat_g1_priv_qpsk.c */
BERR_Code BSAT_g1_P_QpskAcquire_isr(BSAT_ChannelHandle h);
bool BSAT_g1_P_QpskIsSpinv_isrsafe(BSAT_ChannelHandle);
bool BSAT_g1_P_QpskIsValidMode_isr(BSAT_ChannelHandle);
BERR_Code BSAT_g1_P_QpskOnLock_isr(BSAT_ChannelHandle);
BERR_Code BSAT_g1_P_QpskOnLostLock_isr(BSAT_ChannelHandle);
BERR_Code BSAT_g1_P_QpskOnStableLock_isr(BSAT_ChannelHandle);
BERR_Code BSAT_g1_P_QpskEnableLockInterrupts_isr(BSAT_ChannelHandle, bool bEnable);
bool BSAT_g1_P_QpskIsLocked_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_QpskUpdateErrorCount_isrsafe(BSAT_ChannelHandle h);

/* functions in bsat_g1_priv_tfec.c */
#ifndef BSAT_EXCLUDE_TFEC
BERR_Code BSAT_g1_P_TfecAcquire_isr(BSAT_ChannelHandle h);
bool BSAT_g1_P_TfecIsValidMode_isr(BSAT_ChannelHandle);
BERR_Code BSAT_g1_P_TfecOnLock_isr(BSAT_ChannelHandle);
BERR_Code BSAT_g1_P_TfecOnLostLock_isr(BSAT_ChannelHandle);
BERR_Code BSAT_g1_P_TfecOnStableLock_isr(BSAT_ChannelHandle);
BERR_Code BSAT_g1_P_TfecEnableLockInterrupts_isr(BSAT_ChannelHandle, bool bEnable);
BERR_Code BSAT_g1_P_TfecEnableSyncInterrupt_isr(BSAT_ChannelHandle h, bool bEnable);
void BSAT_g1_P_TfecSync_isr(void *p, int int_id);
BERR_Code BSAT_g1_P_TfecGetStatus(BSAT_ChannelHandle h, BSAT_TurboStatus *pStatus);
BERR_Code BSAT_g1_P_TfecOnMonitorLock_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_TfecResetBlockCount_isrsafe(BSAT_ChannelHandle h);
#ifdef BSAT_HAS_DUAL_TFEC
bool BSAT_g1_P_TfecIsOtherChannelBusy_isrsafe(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_TfecReacquire_isr(BSAT_ChannelHandle h);
#endif
#endif

/* functions in bsat_g1_priv_afec.c */
#ifndef BSAT_EXCLUDE_AFEC
BERR_Code BSAT_g1_P_AfecAcquire_isr(BSAT_ChannelHandle h);
bool BSAT_g1_P_AfecIsValidMode_isr(BSAT_ChannelHandle);
bool BSAT_g1_P_AfecIsPilot_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecOnLock_isr(BSAT_ChannelHandle);
BERR_Code BSAT_g1_P_AfecOnLostLock_isr(BSAT_ChannelHandle);
BERR_Code BSAT_g1_P_AfecOnStableLock_isr(BSAT_ChannelHandle);
BERR_Code BSAT_g1_P_AfecEnableLockInterrupts_isr(BSAT_ChannelHandle, bool bEnable);
BERR_Code BSAT_g1_P_AfecEnableMpegInterrupts_isr(BSAT_ChannelHandle h, bool bEnable);
void BSAT_g1_P_AfecMpegLock_isr(void *p, int int_id);
void BSAT_g1_P_AfecMpegNotLock_isr(void *p, int int_id);
BERR_Code BSAT_g1_P_AfecGetStatus(BSAT_ChannelHandle h, BSAT_Dvbs2Status *pStatus);
BERR_Code BSAT_g1_P_AfecOnMonitorLock_isr(BSAT_ChannelHandle h);
bool BSAT_g1_P_AfecIsLocked_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecResetBlockCount_isrsafe(BSAT_ChannelHandle h);
bool BSAT_g1_P_AfecIsChannelEnabled_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecReacquire_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecResetChannel_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_AfecEnableChannel_isrsafe(BSAT_ChannelHandle h, bool bEnable);
BERR_Code BSAT_g1_P_HpGetAcmStatus(BSAT_ChannelHandle h, BSAT_g1_P_AcmStatus *pStatus);
BERR_Code BSAT_g1_P_AfecSetScramblingSeq_isr(BSAT_ChannelHandle h);
bool BSAT_g1_P_AfecIsMpegLocked_isr(BSAT_ChannelHandle h);
#if defined(BSAT_HAS_DVBS2X) && defined(BSAT_HAS_ACM)
BERR_Code BSAT_g1_P_GetStreamRegStat_isrsafe(BSAT_ChannelHandle h, int idx, uint32_t *pStat);
BERR_Code BSAT_g1_P_AfecSetEqsftctl_isrsafe(BSAT_ChannelHandle h);
void BSAT_g1_P_AfecUpdateStreamIdsForMpegCounters_isrsafe(BSAT_ChannelHandle h);
#endif
#ifndef BSAT_HAS_DVBS2X
bool BSAT_g1_P_AfecIsOtherChannelBusy_isrsafe(BSAT_ChannelHandle h);
#endif
#endif

/* functions in bsat_g1_priv_timer.c */
void BSAT_g1_P_BaudTimer_isr(void *p, int param);
void BSAT_g1_P_BerTimer_isr(void *p, int param);
void BSAT_g1_P_Gen1Timer_isr(void *p, int param);
void BSAT_g1_P_Gen2Timer_isr(void *p, int param);
void BSAT_g1_P_Gen3Timer_isr(void *p, int param);
BERR_Code BSAT_g1_P_DisableTimer_isr(BSAT_ChannelHandle h, BSAT_TimerSelect t);
BERR_Code BSAT_g1_P_EnableTimer(BSAT_ChannelHandle h, BSAT_TimerSelect t, uint32_t delay, BSAT_g1_FUNCT func);
BERR_Code BSAT_g1_P_EnableTimer_isr(BSAT_ChannelHandle h, BSAT_TimerSelect t, uint32_t delay, BSAT_g1_FUNCT func);
bool BSAT_g1_P_IsTimerRunning_isr(BSAT_ChannelHandle h, BSAT_TimerSelect t);
bool BSAT_g1_P_IsTimerExpired_isr(BSAT_ChannelHandle h, BSAT_TimerSelect t);

/* functions in bsat_g1_priv_misc.c */
void BSAT_g1_P_ReadModifyWriteRegister_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t and_mask, uint32_t or_mask);
void BSAT_g1_P_OrRegister_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t or_mask);
void BSAT_g1_P_AndRegister_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t and_mask);
void BSAT_g1_P_ToggleBit_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t mask);
void BSAT_g1_P_GetRegisterWriteWaitTime_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t *wait_time);
BERR_Code BSAT_g1_P_ClearTraceBuffer(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_LogTraceBuffer_isr(BSAT_ChannelHandle h, BSAT_TraceEvent event);
BERR_Code BSAT_g1_P_GetTraceInfo(BSAT_ChannelHandle h, BSAT_TraceInfo *pBuffer);
BERR_Code BSAT_g1_P_ResetAcquisitionTimer_isr(BSAT_ChannelHandle h);
void BSAT_g1_P_GetAcquisitionTimerValue_isr(BSAT_ChannelHandle h, uint32_t *pVal);
uint32_t BSAT_g1_P_GCF_isrsafe(uint32_t m, uint32_t n);

/* functions in bsat_g1_priv_snr.c */
BERR_Code BSAT_g1_P_GetSnr(BSAT_ChannelHandle h, uint32_t *pSnr);
BERR_Code BSAT_g1_P_GetSnr_isr(BSAT_ChannelHandle h, uint32_t *pSnr);

/* functions in bsat_g1_priv_plc.c */
BERR_Code BSAT_g1_P_SetPlc_isr(BSAT_ChannelHandle h, uint32_t bw, uint32_t damp);
BERR_Code BSAT_g1_P_ConfigPlc_isr(BSAT_ChannelHandle h, bool bAcq);

/* functions in bsat_g1_priv_sa.c */
BERR_Code BSAT_g1_P_SaStateMachine_isr(BSAT_ChannelHandle h);

#ifndef BSAT_EXCLUDE_SPUR_CANCELLER
/* functions in bsat_g1_priv_cwc.c */
BERR_Code BSAT_g1_P_CwcInit_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT funct);
BERR_Code BSAT_g1_P_CwcDisable_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_CwcReset_isr(BSAT_ChannelHandle h);
#endif

/* tuner functions */
BERR_Code BSAT_g1_P_TunerInit(BSAT_ChannelHandle h, BSAT_g1_FUNCT nextFunct);
BERR_Code BSAT_g1_P_TunerSetFreq_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT nextFunct);
BERR_Code BSAT_g1_P_TunerQuickTune_isr(BSAT_ChannelHandle h, BSAT_g1_FUNCT nextFunct);
BERR_Code BSAT_g1_ResetXbarFifo(BSAT_Handle h, uint8_t adcSelect);

/* channelizer functions */
BERR_Code BSAT_g1_P_ChanSetAciCoeff_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_DisableAllNotch_isrsafe(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_UpdateNotch_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_ConfigChanAgc_isr(BSAT_ChannelHandle h, bool bTracking);
BERR_Code BSAT_g1_P_SetNotch_isr(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_P_SetAdcSelect(BSAT_ChannelHandle h, uint8_t adcSelect);
BERR_Code BSAT_g1_P_GetAdcSelect(BSAT_ChannelHandle h, uint8_t *pAdcSelect);
BERR_Code BSAT_g1_P_FreezeChanAgc_isr(BSAT_ChannelHandle h, bool bFreeze);
void BSAT_g1_P_LoadAciFilterCoeff_isr(BSAT_ChannelHandle h, const int16_t *pCoeff);

#if (!defined(BSAT_EXCLUDE_AFEC) && defined(BSAT_HAS_DVBS2X))
const uint8_t* BSAT_g1_P_AfecGetPdLut_isr(uint8_t lutMode);
#endif

#endif /* _BSAT_7366_PRIV_H_ */
