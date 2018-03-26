/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef _BAST_G3_PRIV_H__
#define _BAST_G3_PRIV_H__

/* exclude flags for code savings */
/*
#define BAST_EXCLUDE_TURBO
#define BAST_EXCLUDE_STATUS_EVENTS
#define BAST_EXCLUDE_POWERDOWN
#define BAST_EXCLUDE_PEAK_SCAN
#define BAST_EXCLUDE_MI2C
#define BAST_EXCLUDE_BCM3445
#define BAST_EXCLUDE_LDPC
#define BAST_EXCLUDE_FTM
*/
#define BAST_EXCLUDE_SPUR_CANCELLER /* compile out spur canceller by default */

#include "bast_g3.h"
#if (BCHP_CHIP==7358)
   #include "bast_7358_priv.h"
#elif (BCHP_CHIP==7346)
   #include "bast_7346_priv.h"
#elif (BCHP_CHIP==73465)
   #include "bast_73465_priv.h"
#elif (BCHP_CHIP==7344)
   #include "bast_7344_priv.h"
#elif (BCHP_CHIP==4528)
   #define BAST_HAS_WFE
   #define BAST_HAS_LEAP
   #define BAST_EXCLUDE_MI2C
   #define BAST_EXCLUDE_BCM3445
   #include "bast_4528_priv.h"
#elif (BCHP_CHIP==4517)
   #define BAST_HAS_LEAP
   #define BAST_EXCLUDE_LDPC
   #define BAST_DONT_USE_MI2C_IRQ
   #undef BAST_EXCLUDE_SPUR_CANCELLER
   #include "bast_4517_priv.h"
#elif (BCHP_CHIP==7360)
   #undef BAST_EXCLUDE_SPUR_CANCELLER
   #include "bast_7360_priv.h"
#elif (BCHP_CHIP==7362)
   #include "bast_7362_priv.h"
#elif (BCHP_CHIP==73625)
   #include "bast_73625_priv.h"
#elif (BCHP_CHIP==7228)
   #include "bast_7228_priv.h"
#elif (BCHP_CHIP==4538)
   #define BAST_HAS_WFE
   #define BAST_HAS_LEAP
   #define BAST_EXCLUDE_BCM3445
   #define BAST_EXCLUDE_MI2C
   #define BAST_EXCLUDE_STATUS_EVENTS
   #define BAST_EXCLUDE_EXT_TUNER
   #define BAST_EXCLUDE_ACW
   /* disable notch for now... #undef BAST_EXCLUDE_SPUR_CANCELLER */
#if BCHP_VER < BCHP_VER_B0
   #define BAST_FROF2_WORKAROUND
#endif
   #include "bast_4538_priv.h"
#else
   #error "unsupported BCHP_CHIP"
#endif


#define BAST_G3_RELEASE_VERSION 14


#define BAST_CHK_RETCODE(x) \
   { if ((retCode = (x)) != BERR_SUCCESS) goto done; }

#define BAST_SCRIPT_OPCODE          0xF0000000
#define BAST_SCRIPT_OPCODE_WRITE    0x00000000
#define BAST_SCRIPT_OPCODE_AND      0x10000000
#define BAST_SCRIPT_OPCODE_OR       0x20000000
#define BAST_SCRIPT_OPCODE_AND_OR   0x30000000
#define BAST_SCRIPT_OPCODE_DEBUG    0x40000000
#define BAST_SCRIPT_OPCODE_EXIT     0xF0000000
#define BAST_SCRIPT_WRITE(reg,val)  (BAST_SCRIPT_OPCODE_WRITE | reg), val
#define BAST_SCRIPT_AND(reg,val)    (BAST_SCRIPT_OPCODE_AND | reg), val
#define BAST_SCRIPT_OR(reg,val)     (BAST_SCRIPT_OPCODE_OR | reg), val
#define BAST_SCRIPT_AND_OR(reg,a,o) (BAST_SCRIPT_OPCODE_AND_OR | reg), a, o
#define BAST_SCRIPT_EXIT            BAST_SCRIPT_OPCODE_EXIT
#define BAST_SCRIPT_DEBUG(val)      BAST_SCRIPT_OPCODE_DEBUG, val

/* bit definitions for ldpcScanState */
#define BAST_LDPC_SCAN_STATE_QPSK          0x02
#define BAST_LDPC_SCAN_STATE_PILOT         0x01
#define BAST_LDPC_SCAN_STATE_ENABLED       0x80
#define BAST_LDPC_SCAN_STATE_FOUND         0x40
#define BAST_LDPC_SCAN_STATE_MASK          0xC0

/* bit definitions for turboScanState */
#define BAST_TURBO_SCAN_STATE_HP_INIT        0x01
#define BAST_TURBO_SCAN_STATE_HP_LOCKED      0x02
#define BAST_TURBO_SCAN_STATE_8PSK_HP_LOCKED 0x04
#define BAST_TURBO_SCAN_STATE_8PSK_FAILED    0x08
#define BAST_TURBO_SCAN_STATE_FIRST_TIME     0x80

/* bit definitions for signalDetectStatus */
#define BAST_SIGNAL_DETECT_ENABLED     0x80
#define BAST_SIGNAL_DETECT_IN_PROGRESS 0x40
#define BAST_SIGNAL_DETECT_TIMING_LOCK 0x01

/* bit definitions for peakScanStatus */
#define BAST_PEAK_SCAN_STATUS_ENABLED   0x80
#define BAST_PEAK_SCAN_STATUS_DONE      0x40
#define BAST_PEAK_SCAN_STATUS_ERROR     0x20
#define BAST_PEAK_SCAN_STATUS_DUMP_BINS 0x10

#define BAST_BCM3445_DEFAULT_ADDRESS 0xD8

#define BAST_TUNER_KVCO_CAL_TABLE_SIZE 8

typedef BERR_Code (*BAST_g3_FUNCT)(BAST_ChannelHandle);
typedef BERR_Code (*BAST_g3_DEV_FUNCT)(BAST_Handle);
typedef void (*BAST_g3_LOCK_FUNCT)(BAST_ChannelHandle, bool*);

/* timers */
typedef enum BAST_TimerSelect
{
   BAST_TimerSelect_eBaud = 0,
   BAST_TimerSelect_eBaudUsec,
   BAST_TimerSelect_eBer,  /* used for delays between successive register accesses in bclk domain */
   BAST_TimerSelect_eGen1, /* used for reacquisition timeout */
   BAST_TimerSelect_eGen2, /* used for stable_lock timer and lock monitor activities */
   BAST_TimerSelect_eGen3, /* used to measure acquisition time */
   BAST_TimerSelect_eDiseqc1,
   BAST_TimerSelect_eDiseqc2
} BAST_TimerSelect;

#if (BCHP_CHIP==4528)
#define BAST_TimerSelect_eReacqTimer       BAST_TimerSelect_eBaudUsec
#define BAST_TimerSelect_eStableLockTimer  BAST_TimerSelect_eBaudUsec
#define BAST_TimerSelect_eMonitorLockTimer BAST_TimerSelect_eBaudUsec
#else
#define BAST_TimerSelect_eReacqTimer       BAST_TimerSelect_eGen1
#define BAST_TimerSelect_eStableLockTimer  BAST_TimerSelect_eGen2
#define BAST_TimerSelect_eMonitorLockTimer BAST_TimerSelect_eGen2
#endif

/* acquisition states */
typedef enum BAST_AcqState
{
   BAST_AcqState_eIdle = 0,
   BAST_AcqState_eTuning,
   BAST_AcqState_eAcquiring,
   BAST_AcqState_eWaitForInitialLock,
   BAST_AcqState_eWaitForStableLock,
   BAST_AcqState_eMonitorLock
} BAST_AcqState;

/* tuner indirect register groups */
typedef enum BAST_TunerIndirectRegGroup
{
   BAST_TunerIndirectRegGroup_eRfagc,
   BAST_TunerIndirectRegGroup_eBbagc,
   BAST_TunerIndirectRegGroup_ePreDcoI,
   BAST_TunerIndirectRegGroup_ePreDcoQ,
   BAST_TunerIndirectRegGroup_ePostDcoI,
   BAST_TunerIndirectRegGroup_ePostDcoQ
} BAST_TunerIndirectRegGroup;

/* tuner calibration type */
typedef enum BAST_TunerCalType
{
   BAST_TunerCalType_eFga,
   BAST_TunerCalType_eLpf
} BAST_TunerCalType;

/* reason for reacquisition */
typedef enum BAST_ReacqCause
{
   BAST_ReacqCause_eOK = 0,
   BAST_ReacqCause_eTimingLoopNotLocked, /* timing loop could not be locked */
   BAST_ReacqCause_eFecNotStableLock, /* reacquisition timer expired due to FEC not able to stay in lock */
   BAST_ReacqCause_eInvalidMode,  /* restricted mode */
   BAST_ReacqCause_eCarrierOffsetOutOfRange, /* carrier offset drifted beyond search range */
   BAST_ReacqCause_eInvalidModcod, /* decoded modcod is invalid */
   BAST_ReacqCause_eModcodNotStable, /* modcod not stable */
   BAST_ReacqCause_eTimerError1,  /* BAST_EnableTimer() returned error */
   BAST_ReacqCause_eTimerError2,  /* BAST_EnableTimer() returned error */
   BAST_ReacqCause_eHpLostLock1,  /* HP not locked in frame boundary isr */
   BAST_ReacqCause_eHpLostLock2,  /* HP lost lock while waiting for turbo sync */
   BAST_ReacqCause_eHpLostLock3,  /* HP lost lock during LDPC tracking */
   BAST_ReacqCause_eHpLostLock4,  /* HP lost lock during Turbo tracking */
   BAST_ReacqCause_eHpCouldNotLock, /* HP could not reach state 5 */
   BAST_ReacqCause_eNoAfec, /* LDPC acquisition initiated but chip does not have AFEC */
   BAST_ReacqCause_eNoTfec, /* turbo acquisition initiated but chip does not have TFEC */
   BAST_ReacqCause_eInvalidCondition1, /* actualMode is not LDPC in LDPC function */
   BAST_ReacqCause_eVitNotLock, /* no Viterbi lock in legacy QPSK mode */
   BAST_ReacqCause_eUnableToDetermineActualMode, /* legacy qpsk mode: puncture rate status not stable */
   BAST_ReacqCause_eTurboSyncTimeout, /* no turbo sync seen */
   BAST_ReacqCause_eCodeRateChanged   /* legacy qpsk mode: detected code rate changed */
} BAST_ReacqCause;

/* function pointer for ftm idle funct */
typedef BERR_Code (*BAST_FTM_ISR)(BAST_Handle);


#ifdef BAST_HAS_WFE
#define BAST_MAX_SPURS 16

typedef enum BAST_NotchSelect
{
   BAST_NotchSelect_e0 = 0,
   BAST_NotchSelect_e1
} BAST_NotchSelect;


typedef struct BAST_NotchSettings
{
   int32_t  spurFreq;
   uint8_t  alpha; /* depth of DC notch filter */
} BAST_NotchSettings;
#endif /* BAST_HAS_WFE */


#ifndef BAST_EXCLUDE_FTM
/******************************************************************************
BAST_FtmCbInfo
Summary: Interrupt Callback Table to store ID, callback handle, and enable state
*******************************************************************************/
typedef struct BAST_FtmCbInfo
{
   BINT_Id       	      intID;
   BINT_CallbackHandle  hCb;
	bool                 bEnable;
} BAST_FtmCbInfo;


/***************************************************************************
Summary:
   Structure for FTM device of the BAST_g3_P_Handle
Description:
   This is the FTM device component of the BAST_g3_P_Handle.
See Also:
   None.
****************************************************************************/
typedef struct
{
   BAST_FskChannel      txChannel;           /* channel for fsk transmit  */
   BAST_FskChannel      rxChannel;           /* channel for fsk receive  */
   uint32_t             txFreqHz;            /* transmit frequency in Hz */
   uint32_t             rxFreqHz;            /* receive frequency in Hz */
   uint32_t             txDevHz;             /* transmit deviation in Hz */
#ifndef BAST_ENABLE_GENERIC_FSK
   BAST_FTM_ISR         idle_funct_isr;
   BKNI_EventHandle     event;
   BAST_FtmCbInfo       cbInfo[32];             /* callback info */
   uint32_t             rid;                    /* receiver id */
   uint32_t             reg_time_left;          /* registration time remaining */
   bool                 bForwardPacket;         /* determine whether to forward rcvd network packet to host */
   uint16_t             options;                /* current option bits */
   uint16_t             req_options;            /* requested option bits */
   uint16_t             rx_bit_mask;            /* rx address mask */
   uint16_t             status;
   uint16_t             polled_addr;            /* monitor polled addresses */
   uint8_t              tx_buffer[65];
   uint8_t              rx_buffer[65];
   uint8_t              host_buffer[65];
   uint8_t              err_buffer[65];
   uint8_t              buf_status;
   uint8_t              err_status;
   uint8_t              rx_packet_cmd;          /* received command id */
   uint8_t              rx_packet_src_addr;     /* received source address */
   uint8_t              rx_packet_dest_addr;    /* received destination address */
   uint8_t              rx_packet_data_len;     /* received data length */
   uint8_t              rx_packet_header_crc;   /* received header crc */
   uint8_t              sp_packet_src_addr;     /* used to store src address of outgoing sp packet */
   uint8_t              num_offers;
   uint8_t              network_activity;
#else
   BINT_CallbackHandle  hCallback[32];       /* ftm interrupts callback array */
   bool                 bLnbDetected;        /* indicates LNB detection */
   uint16_t             status;
   uint16_t             timecount_max;       /* TC max calculated from N_channel in 0.1ms units */
   uint16_t             slot_length;         /* TS slot length varies from 3000us to 6500us in 500us steps */
   uint16_t             tdma_cycle;          /* (N_Channel + 1) * slot_length / 100 in 0.1ms units */
   uint8_t              tx_buffer[65];
   uint8_t              rx_buffer[65];
   uint8_t              buf_status;
   uint8_t              err_status;
   uint8_t              n_channel;           /*N_Channel is the number of channels in the LNB, used to define number of time slots in system */
   uint8_t              rxLen;               /* receive packet length */
#endif
   uint8_t              txPower;          /* config param for transmit power */
} BAST_g3_P_FtmDevice;
#endif


/***************************************************************************
Summary:
   Structure for diseqc device of the BAST_g3_P_ChannelHandle
Description:
   This is the diseqc device component of the BAST_g3_P_ChannelHandle.  This
   struct is allocated only if the channel has diseqc capability.
See Also:
   None.
****************************************************************************/
typedef struct BAST_g3_P_DiseqcDevice
{
   BKNI_EventHandle     hDiseqcEvent;             /* diseqc event handle */
   BKNI_EventHandle     hDiseqcOverVoltageEvent;  /* diseqc over-voltage event handle */
   BKNI_EventHandle     hDiseqcUnderVoltageEvent; /* diseqc under-voltage event handle */
   BINT_CallbackHandle  hDiseqcDoneCb;            /* callback handle for diseqc done interrupt */
   BINT_CallbackHandle  hDiseqcOverVoltageCb;     /* callback handle for diseqc over voltage interrupt */
   BINT_CallbackHandle  hDiseqcUnderVoltageCb;    /* callback handle for diseqc under voltage interrupt */
   BINT_CallbackHandle  hDiseqc1TimerCb;          /* callback handle for diseqc 1 timer interrupt */
   BINT_CallbackHandle  hDiseqc2TimerCb;          /* callback handle for diseqc 2 timer interrupt */
   BINT_CallbackHandle  hDiseqcTxFifoAlmostEmptyCb;   /* callback handle for diseqc tx fifo almost empty interrupt */
   BAST_g3_FUNCT        diseqc1TimerIsr;          /* function for handling diseqc timer 1 interrupt */
   BAST_g3_FUNCT        diseqc2TimerIsr;          /* function for handling diseqc timer 2 interrupt */
   BAST_DiseqcStatus    diseqcStatus;             /* status of last diseqc transaction */
   BAST_DiseqcSettings  dsecSettings;             /* diseqc settings */
   uint8_t              diseqcSendBuffer[128];    /* diseqc transmit buffer */
   uint8_t              diseqcSendLen;            /* diseqc transmit length */
   uint8_t              diseqcSendCount;          /* diseqc transmit count */
#ifndef BAST_EXCLUDE_ACW
   uint8_t              diseqcAcw;                /* auto-control word */
#endif
   bool                 bDiseqcToneOn;            /* true if diseqc tone is enabled */
} BAST_g3_P_DiseqcDevice;


/***************************************************************************
Summary:
   Structure for chip-specific portion of the BAST handle
Description:
   This is the chip-specific component of the BAST_Handle.
See Also:
   None.
****************************************************************************/
typedef struct BAST_g3_P_Handle
{
   BREG_Handle           hRegister;          /* register handle */
   BINT_Handle           hInterrupt;         /* interrupt handle */
   BCHP_Handle           hChip;              /* chip handle */
   BKNI_EventHandle      hInitDoneEvent;     /* init done event handle */
#ifndef BAST_EXCLUDE_FTM
   BKNI_EventHandle      hFtmEvent;          /* FTM event handle */
   BAST_g3_P_FtmDevice   hFtmDev;            /* FTM device handle */
#endif
   BAST_g3_DEV_FUNCT     postInitApFunct;    /* chip-specific function to call after BAST_InitAp() */
#ifdef BAST_HAS_WFE
   BAST_NotchSettings    spurs[BAST_MAX_SPURS]; /* stores all the spur information */
#else
   BAST_TunerLnaSettings lnaSetting;         /* lna setting */
#endif
   uint32_t              xtalFreq;           /* crystal freq in Hz */
   uint32_t              searchRange;        /* tuner search range in Hz */
   bool                  bInit;              /* true when AST PI has completely initialized */
#ifndef BAST_EXCLUDE_FTM
   bool                  bFtmLocalReset;     /* distinguish between bast reset or local reset */
   bool                  bFtmPoweredDown;    /* indicates if ftm core powered down */
#endif
#ifndef BAST_EXCLUDE_BCM3445
   bool                  bBcm3445;           /* TRUE = has bcm3445 LNA */
#endif
   bool                  bOpen;              /* true if AST PI has been openned successfully */
   uint16_t              tuner_kvco_cal_capcntl_table[BAST_TUNER_KVCO_CAL_TABLE_SIZE];
   uint8_t               tuner_kvco_cal_kvcocntl_table[BAST_TUNER_KVCO_CAL_TABLE_SIZE];
   uint8_t               numInternalTuners;  /* number of internal tuners on this chip */
#ifndef BAST_EXCLUDE_BCM3445
   uint8_t               bcm3445Address;     /* i2c address of bcm3445 */
#endif
#ifdef BAST_HAS_WFE
   uint8_t               numSpurs;           /* number of spurs stored in spurs[] */
#endif
   uint8_t               sdsRevId;           /* SDS core revision ID */
   uint8_t               counter;            /* used during channel initialization */
   uint8_t               dftMinN;            /* number of times to run DFT to minimize bin power */
} BAST_g3_P_Handle;


/***************************************************************************
Summary:
   Structure for chip-specific portion of the BAST channel handle
Description:
   This is the chip-specific component of the BAST_ChannelHandle.
See Also:
   None.
****************************************************************************/
typedef struct BAST_g3_P_ChannelHandle
{
   BKNI_EventHandle     hLockChangeEvent;    /* change of lock status event handle */
   BKNI_EventHandle     hPeakScanEvent;      /* peak scan event handle */
#ifndef BAST_EXCLUDE_MI2C
   BKNI_EventHandle     hMi2cEvent;          /* mi2c event handle */
#endif
#ifndef BAST_EXCLUDE_STATUS_EVENTS
   BKNI_EventHandle     hStatusEvent;        /* status event handle */
#endif
   BKNI_EventHandle     hTunerInitDoneEvent; /* tuner init done event handle */
   BAST_AcqSettings     acqParams;           /* input acquisition parameters */
   BAST_AcqState        acqState;            /* acquisition state */
   BINT_CallbackHandle  hBaudTimerCb;           /* callback handle for baud clock timer interrupt */
   BINT_CallbackHandle  hBerTimerCb;            /* callback handle for BER timer interrupt */
   BINT_CallbackHandle  hGen1TimerCb;           /* callback handle for general timer 1 interrupt */
   BINT_CallbackHandle  hGen2TimerCb;           /* callback handle for general timer 2 interrupt */
   BINT_CallbackHandle  hGen3TimerCb;           /* callback handle for general timer 3 interrupt */
   BINT_CallbackHandle  hQpskLockCb;            /* callback handle for Legacy mode lock interrupt */
   BINT_CallbackHandle  hQpskNotLockCb;         /* callback handle for Legacy mode not lock interrupt */
   BINT_CallbackHandle  hHpLockCb;              /* callback handle for HP state reaching rcvr lock */
   BINT_CallbackHandle  hHpFrameBoundaryCb;     /* callback handle for HP frame boundary */
#if BCHP_CHIP==4538
   BINT_CallbackHandle  hDftDoneCb;             /* callback handle for DFT done interrupt */
#endif
#ifndef BAST_EXCLUDE_LDPC
   BINT_CallbackHandle  hLdpcLockCb;            /* callback handle for LDPC lock interrupt */
   BINT_CallbackHandle  hLdpcNotLockCb;         /* callback handle for LDPC not lock interrupt */
#endif
#ifndef BAST_EXCLUDE_TURBO
   BINT_CallbackHandle  hTurboLockCb;           /* callback handle for turbo lock interrupt */
   BINT_CallbackHandle  hTurboNotLockCb;        /* callback handle for turbo not lock interrupt */
   BINT_CallbackHandle  hTurboSyncCb;           /* callback handle for turbo sync detected */
#endif
#ifndef BAST_EXCLUDE_MI2C
   BINT_CallbackHandle  hMi2cCb;                /* callback handle for MI2C interrupt */
#endif
   BAST_g3_FUNCT        baudTimerIsr;        /* function for handling baud timer interrupt */
   BAST_g3_FUNCT        berTimerIsr;         /* function for handling BER timer interrupt */
   BAST_g3_FUNCT        gen1TimerIsr;        /* function for handling general timer 1 interrupt */
   BAST_g3_FUNCT        gen2TimerIsr;        /* function for handling general timer 2 interrupt */
   BAST_g3_FUNCT        gen3TimerIsr;        /* function for handling general timer 3 interrupt */
   BAST_g3_FUNCT        passFunct;           /* general function pointer used in acquisition state machine */
   BAST_g3_FUNCT        failFunct;           /* general function pointer used in acquisition state machine */
   BAST_g3_FUNCT        postTuneFunct;       /* function to call after tuning */
   BAST_g3_FUNCT        acqFunct;            /* mode-specific acquisition function pointer */
   BAST_g3_FUNCT        checkModeFunct;      /* virtual function to check valid mode */
   BAST_g3_FUNCT        onLockFunct;         /* mode-specific function to handle lock interrupt */
   BAST_g3_FUNCT        onLostLockFunct;     /* mode-specific function to handle lost lock interrupt */
   BAST_g3_FUNCT        onStableLockFunct;   /* mode-specific function to process stable lock event */
   BAST_g3_FUNCT        onMonitorLockFunct;  /* mode-specific function to do periodic lock monitoring */
   BAST_g3_FUNCT        enableLockInterrupts; /* virtual function to enable lock/lost_lock interrupts */
   BAST_g3_FUNCT        disableLockInterrupts; /* virtual function to disable lock/lost_lock interrupts */
   BAST_g3_LOCK_FUNCT   getLockStatusFunct;  /* virtual function to get lock status */
   BAST_g3_P_DiseqcDevice *diseqc;           /* pointer to the diseqc info for this channel */
   BAST_Mode            actualMode;          /* actual mode found during code rate scan */
#ifndef BAST_EXCLUDE_LDPC
   BAST_Mode            acmMaxMode;          /* maximum mode in ACM */
#endif
   BAST_OutputTransportSettings xportSettings;     /* mpeg transport settings */
#ifndef BAST_HAS_WFE
   BAST_TunerLnaSettings      tunerLnaSettings;    /* crossbar configuration of tuner LNA */
#endif
#ifndef BAST_EXCLUDE_SPUR_CANCELLER
   BAST_SpurCancellerConfig   spurConfig[6];
#endif
#ifndef BAST_EXCLUDE_BCM3445
   BAST_Mi2cChannel           bcm3445Mi2cChannel;  /* identifies which mi2c channel controls the BCM3445 associated with this channel's tuner */
   BAST_Bcm3445OutputChannel  bcm3445TunerInput;   /* output port of the BCM3445 feeding this channel's tuner */
   BAST_Bcm3445Settings       bcm3445Settings;     /* configuration of BCM3445 controlled on this channel's MI2C */
#endif
   BAST_ReacqCause      reacqCause;          /* cause for reacquisition */
   uint32_t             vcoRefClock;         /* VCO reference clock frequency in Hz */
   uint32_t             sampleFreq;          /* ADC sampling frequency in Hz */
   uint32_t             fecFreq;             /* FEC (Turbo/LDPC) clock frequency in Hz */
   uint32_t             tunerFreq;           /* requested tuner frequency in Hz */
   uint32_t             actualTunerFreq;     /* actual tuner frequency in Hz */
   uint32_t             tunerIfStepSize;     /* internal tuner IF step size in Hz used in freq scan algorithm */
   uint32_t             outputBitRate;       /* output transport bit rate in bps */
   uint32_t             funct_state;         /* specifies current state in acquisition state machine */
   uint32_t             dft_funct_state;     /* current state in dft carrier search state machine */
   uint32_t             prev_state;          /* previous HP state */
   uint32_t             acqCount;            /* number of reacquisitions */
   uint32_t             carrierAcq1Bw;       /* acquisition carrier loop bandwidth in Hz */
   uint32_t             carrierAcq1Damp;     /* acquisition carrier loop damping factor scaled 2^2 */
   uint32_t             carrierAcq2Bw;       /* viterbi carrier loop bandwidth in Hz */
   uint32_t             carrierAcq2Damp;     /* viterbi carrier loop damping factor scaled 2^2 */
   uint32_t             carrierAcq3Bw;       /* narrow carrier loop bandwidth in Hz, previously cmin */
   uint32_t             carrierAcq3Damp;     /* narrow carrier loop damping factor scaled 2^2 */
   uint32_t             carrierTrkBw;        /* final tracking carrier loop bandwidth in Hz */
   uint32_t             carrierTrkDamp;      /* final tracking carrier loop damping factor scaled 2^2 */
   uint32_t             baudAcqBw;           /* baud loop bandwidth in Hz */
   uint32_t             baudAcqDamp;         /* baud loop damping factor scaled 2^2 */
   uint32_t             baudTrkBw;           /* final baud loop bandwidth in Hz, previous bmin */
   uint32_t             baudTrkDamp;         /* final baud loop damping factor scaled 2^2 */
   uint32_t             mpegErrorCount;      /* accumulated MPEG errors */
   uint32_t             mpegFrameCount;      /* accumulated MPEG frames */
   uint32_t             berErrors;           /* accumulated BER error count */
   uint32_t             debug1;              /* debug config param 1 */
   uint32_t             debug2;              /* debug config param 2 */
   uint32_t             debug3;              /* debug config param 3 */
   uint32_t             rsCorr;              /* accumulated RS correctable errors */
   uint32_t             rsUncorr;            /* accumulated RS uncorrectable errors */
   uint32_t             preVitErrors;        /* accumulated pre-Viterbi error count */
   uint32_t             peakScanSymRateMin;  /* minimum symbol rate in peak scan */
   uint32_t             peakScanSymRateMax;  /* maximum symbol rate in peak scan */
   uint32_t             peakScanOutput;      /* peak scan output status */
   uint32_t             acqTime;             /* acquisition time in usecs */
   uint32_t             irqCount[BAST_g3_MaxIntID]; /* interrupt counter */
   uint32_t             altPlcAcqBw;         /* Acquisition PLC BW override value */
   uint32_t             altPlcTrkBw;         /* Tracking PLC BW override value */
   uint32_t             dft_count1;          /* general counter used in dft carrier search */
   uint32_t             dft_current_max_pow; /* used in dft */
   uint32_t             count1;              /* general counter used in acquisition */
   uint32_t             count2;              /* general counter used in acquisition */
   uint32_t             maxCount1;           /* general counter used in acquisition */
   uint32_t             stableLockTimeout;   /* timeout in usecs for determining stable lock */
   uint32_t             maxStableLockTimeout;/* maximum timeout in usecs for determining stable lock */
   uint32_t             lockFilterIncr;      /* used in lock filtering */
   uint32_t             trace[BAST_TraceEvent_eMax]; /* trace buffer */
   uint32_t             carrierDelay;        /* LockViterbi dwell time */
   uint32_t             checkLockInterval;   /* LockViterbi interval */
   uint32_t             opll_N;              /* calculated OPLL N value */
   uint32_t             opll_D;              /* calculated OPLL D value */
   uint32_t             bfos;                /* baud loop frequency control word */
   uint32_t             peakScanIfStepSize;  /* IF step size used in peak scan */
   uint32_t             peakScanMaxPower;    /* max peak power found during peak scan */
   uint32_t             peakScanSymRateEst;  /* symbol rate where max peak power occurs */
   uint32_t             peakScanMaxRatioSymRate; /* symbol rate where max peak/total occurs */
   uint32_t             peakScanMaxRatioPower; /* max peak power where peak/total is maximized */
   uint32_t             timeSinceStableLock; /* in units of 100 msecs */
   uint32_t             hpFrameCount;        /* current number of frames */
#ifndef BAST_EXCLUDE_LDPC
   uint32_t             ldpcTotalBlocks;     /* accumulated total LDPC block count */
   uint32_t             ldpcCorrBlocks;      /* accumulated LDPC correctable block count */
   uint32_t             ldpcBadBlocks;       /* accumulated LDPC bad block count */
#endif
#ifndef BAST_EXCLUDE_TURBO
   uint32_t             turboTotalBlocks;    /* accumulated Turbo block count */
   uint32_t             turboCorrBlocks;     /* accumulated Turbo correctable block count */
   uint32_t             turboBadBlocks;      /* accumulated Turbo bad block count */
#endif
#ifndef BAST_EXCLUDE_STATUS_EVENTS
   uint32_t             freqDriftThreshold;  /* carrier freq drift threshold in Hz */
   uint32_t             rainFadeSnrAve;      /* ave snr in 1/8 dB during rain fade window */
   uint32_t             rainFadeSnrMax;      /* max snr in 1/8 dB during rain fade window */
   uint32_t             rainFadeWindow;      /* time window for rain fade indicator in units of 100 msecs */
#endif
#ifndef BAST_HAS_WFE
   uint32_t             fullTuneFreq;        /* freq at which full tune was done */
   uint32_t             tunerFddfs;          /* internal tuner DDFS frequency in Hz */
   uint32_t             tunerLoDivider;      /* internal tuner LO divider scaled 2^6 */
   uint32_t             tunerVcRefLow;       /* tuner VcRef low threshold */
   uint32_t             tunerVcRefHigh;      /* tuner VcRef high threshold */
   uint32_t             tunerAgcThreshold;   /* value of BB/LNA AGC clip detector thresholds */
   uint32_t             tunerAgcWinLength;   /* value of BB/LNA AGC window lengths */
   uint32_t             dftBinPower[32];
#else
   int8_t               notchState;
#endif
   int32_t              tunerIfStep;         /* used in freq scan */
   int32_t              tunerIfStepMax;      /* used in freq scan */
   int32_t              dftFreqEstimate;     /* used in freq scan */
   int32_t              initFreqOffset;      /* used in carrier freq drift threshold */
   int32_t              lockFilterRamp;      /* used in lock filtering */
   int32_t              freqTransferInt;     /* used in pli-to-fli leak */
   int32_t              tunerVcoAvoidanceLpfChange;
   bool                 bHasDiseqc;          /* true if channel supports diseqc */
   bool                 bHasAfec;            /* true if channel supports DVB-S2 */
   bool                 bHasTfec;            /* true if channel supports Turbo */
   bool                 bVitLocked;          /* true if viterbi locked in legacy acquisition */
   bool                 bLocked;             /* true if demod and fec are locked */
   bool                 bLastLocked;         /* filtered last lock status */
   bool                 bFsNotDefault;       /* true if sample freq has changed */
   bool                 bBlindScan;          /* true if blind scan is enabled */
   bool                 bReacqTimerExpired;  /* true if reacquisition timer expired */
   bool                 bEverStableLock;     /* true if current transponder was ever locked */
   bool                 bStableLock;         /* true if currently stable locked */
   bool                 bTimingLock;         /* true if timing loop is locked */
   bool                 bPlcTracking;        /* true if tracking plc values are used */
   bool                 bUndersample;        /* true if in undersample mode */
   bool                 bMonitorLock;        /* false if in powered down standby mode */
   bool                 bEnableFineFreq;     /* true if fine freq is enabled */
   bool                 bVcoAvoidance;       /* true if VCO avoidance is triggered */
#ifndef BAST_EXCLUDE_EXT_TUNER
   bool                 bExternalTuner;      /* true if channel uses external tuner */
#endif
#ifndef BAST_HAS_WFE
   bool                 bHasTunerRefPll;     /* true if channel controls internal tuner REF PLL */
   bool                 bOverrideKvco;       /* override kvco */
   bool                 bCalibrateKvco;      /* calibrating kvco */
#endif
#ifndef BAST_EXCLUDE_SPUR_CANCELLER
   bool                 bCwcActive[6];
#endif
#ifndef BAST_EXCLUDE_MI2C
   bool                 bMi2cInProgress;     /* true if MI2C is busy */
#endif
#ifndef BAST_EXCLUDE_STATUS_EVENTS
   bool                 bStatusInterrupts;   /* true if host wants PI to monitor status indicators */
#endif
   uint16_t             dftRangeStart;       /* configuration parameter used in tone detect */
   uint16_t             dftRangeEnd;         /* configuration parameter used in tone detect */
   uint16_t             peakScanDftSize;     /* DFT size used in peak scan */
   uint16_t             peakScanMaxPeakBin;  /* bin where max peak power occurs */
   uint16_t             peakScanMaxRatio;    /* max peak/total */
   uint16_t             peakScanMaxRatioBin; /* bin where max peak/total occurs */
#ifndef BAST_EXCLUDE_TURBO
   bool                 bTurboSpinv;         /* spectral inversion setting for HP */
   uint16_t             turboScanModes;      /* specifies which turbo modes to search in scan mode */
   uint16_t             turboScanCurrMode;   /* used in turbo scan mode */
#endif
#ifndef BAST_EXCLUDE_LDPC
   uint16_t             ldpcScanModes;       /* specifies which ldpc modes to search in scan mode */
#endif
#ifndef BAST_HAS_WFE
   uint16_t             tunerCapCntl;        /* tuner mixer PLL VCO cap setting */
   uint16_t             tunerCapMask;        /* search mask for tuner mixer PLL VCO cap */
   uint16_t             tunerVcoFreqMhz;     /* vco calibration freq in MHz */
   uint16_t             tunerAgcAmpThresh;   /* value of BB/LNA AGC amplitude thresholds */
   uint16_t             tunerAgcLoopCoeff;   /* value of BB/LNA AGC loop coefficients */
#endif
   uint8_t              dftFreqEstimateStatus; /* used in DFT freq estimate algorithm */
   uint8_t              blindScanCurrMode;   /* used in blind acquisition */
   uint8_t              blindScanModes;      /* config parameter used to specify which modes to scan */
   uint8_t              tunerCtl;            /* TUNER_CTL configuration parameter */
   uint8_t              tunerLpfToCalibrate; /* tuner LPF freq to calibrate */
   uint8_t              tunerCutoff;         /* desired internal tuner cutoff frequency in MHz */
   uint8_t              agcCtl;              /* AGC_CTL configuration parameter */
   uint8_t              dtvScanState;        /* used for manual DTV 1/2 scan in DTV scan mode */
   uint8_t              coresPoweredDown;
   uint8_t              lockIsrFlag;
   uint8_t              dtvScanCodeRates;    /* specifies which DTV code rates to search in scan mode */
   uint8_t              dvbScanCodeRates;    /* specifies which DVB code rates to search in scan mode */
   uint8_t              dciiScanCodeRates;   /* specifies which DCII code rates to search in scan mode */
   uint8_t              stuffBytes;            /* number of null stuff bytes */
   uint8_t              reacqCtl;              /* reacquisition control flags */
   uint8_t              altPlcAcqDamp;         /* Acquisition PLC damping factor override value, scaled 2^3 */
   uint8_t              altPlcTrkDamp;         /* Tracking PLC damping factor override value, scaled 2^3 */
   uint8_t              plcCtl;                /* PLC_CTL configuration parameter */
   uint8_t              signalDetectStatus;    /* status of signal detect mode */
   uint8_t              peakScanStatus;        /* status of peak scan */
   uint8_t              dft_done_timeout;      /* used to timeout dft done */
   uint8_t              miscCtl;                /* MISC_CTL configuration parameter */
   uint8_t              relockCount;         /* number of times relocked without doing reacquisition */
   uint8_t              peakScanMaxPeakHb;
   uint8_t              peakScanMaxRatioHb;
#ifndef BAST_EXCLUDE_TURBO
   uint8_t              turboCtl;            /* TURBO_CTL configuration parameter */
   uint8_t              turboScanState;      /* used in Turbo scan mode */
#endif
#ifndef BAST_EXCLUDE_LDPC
   uint8_t              ldpcCtl;             /* LDPC_CTL configuration parameter */
   uint8_t              ldpcScanState;       /* used in LDPC scan mode */
   uint8_t              modcod;              /* saved modcod */
   uint8_t              ldpcScramblingSeq[16]; /* XSEED+PLHDRCFGx sequence */
#endif
#ifndef BAST_EXCLUDE_BCM3445
   uint8_t              bcm3445Status;         /* see BAST_BCM3445_STATUS_* macros in bast.h */
   uint8_t              bcm3445Ctl;            /* BCM3445_CTL configuration parameter */
#endif
#ifndef BAST_HAS_WFE
   uint8_t              tunerLnaStatus;        /* see BAST_TUNER_LNA_STATUS_* macros in bast.h */
   uint8_t              tunerFgaCal;         /* calibrated value for FGA */
   uint8_t              tunerLpfCal;         /* calibrated value for LPF */
   uint8_t              tunerFgaCalManual;
   uint8_t              tunerLpfCalManual;
   uint8_t              tunerFilCalUpperThresh; /* upper threshold for tuner filter calibration as percentage of half tone scaled by 100 */
   uint8_t              tunerFilCalLowerThresh; /* lower threshold for tuner filter calibration as percentage of half tone scaled by 100 */
   uint8_t              tunerLsRange;           /* tuner linear cap search range */
   uint8_t              tunerKvcoCntl;          /* tuner kvco_cntl override */
   uint8_t              tunerKvcoCntl_high;     /* high tuner kvco_cntl */
   uint8_t              tunerKvcoCntl_low;      /* low tuner kvoc_cntl */
   uint8_t              tempAmbient;            /* configurable ambient temperature for tuner Vc adjustment */
   uint8_t              tunerDaisyGain;         /* daisy output gain */
   uint8_t              tunerRefPllChannel;     /* parent channel controlling REF PLL */
#endif
#ifndef BAST_EXCLUDE_STATUS_EVENTS
   uint8_t              rainFadeThreshold;   /* rain fade threshold in 1/8 dB */
   uint8_t              statusEventIndicators;
#endif
} BAST_g3_P_ChannelHandle;


/* implementation of API functions */
BERR_Code BAST_g3_P_Open(BAST_Handle *, BCHP_Handle, void*, BINT_Handle, const BAST_Settings *pDefSettings);
BERR_Code BAST_g3_P_Close(BAST_Handle);
BERR_Code BAST_g3_P_GetTotalChannels(BAST_Handle, uint32_t *);
BERR_Code BAST_g3_P_OpenChannel(BAST_Handle, BAST_ChannelHandle *, uint32_t chnNo, const BAST_ChannelSettings *pChnDefSettings);
BERR_Code BAST_g3_P_CloseChannel(BAST_ChannelHandle);
BERR_Code BAST_g3_P_GetDevice(BAST_ChannelHandle, BAST_Handle *);
BERR_Code BAST_g3_P_InitAp(BAST_Handle, const uint8_t *);
BERR_Code BAST_g3_P_SoftReset(BAST_Handle);
BERR_Code BAST_g3_P_ResetChannel(BAST_ChannelHandle, bool);
#ifndef BAST_HAS_LEAP
BERR_Code BAST_g3_P_GetApVersion(BAST_Handle, uint16_t*, uint8_t*, uint32_t*, uint8_t*, uint8_t*);
BERR_Code BAST_g3_P_GetVersionInfo(BAST_Handle h, BFEC_VersionInfo *pVersion);
#endif
BERR_Code BAST_g3_P_TuneAcquire(BAST_ChannelHandle, const uint32_t, const BAST_AcqSettings *);
BERR_Code BAST_g3_P_GetChannelStatus(BAST_ChannelHandle, BAST_ChannelStatus *);
BERR_Code BAST_g3_P_GetLockStatus(BAST_ChannelHandle, bool *);
BERR_Code BAST_g3_P_ResetStatus(BAST_ChannelHandle);
BERR_Code BAST_g3_P_SetDiseqcTone(BAST_ChannelHandle, bool);
BERR_Code BAST_g3_P_GetDiseqcTone(BAST_ChannelHandle, bool*);
BERR_Code BAST_g3_P_SetDiseqcVoltage(BAST_ChannelHandle, bool);
BERR_Code BAST_g3_P_GetDiseqcVoltage(BAST_ChannelHandle, uint8_t*);
BERR_Code BAST_g3_P_EnableDiseqcLnb(BAST_ChannelHandle, bool);
BERR_Code BAST_g3_P_EnableVsenseInterrupts(BAST_ChannelHandle, bool);
#ifndef BAST_EXCLUDE_ACW
BERR_Code BAST_g3_P_SendACW(BAST_ChannelHandle, uint8_t);
BERR_Code BAST_g3_P_DoneACW(BAST_ChannelHandle);
#endif
BERR_Code BAST_g3_P_SendDiseqcCommand(BAST_ChannelHandle, const uint8_t *pSendBuf, uint8_t sendBufLen);
BERR_Code BAST_g3_P_GetDiseqcStatus(BAST_ChannelHandle, BAST_DiseqcStatus *pStatus);
BERR_Code BAST_g3_P_ResetDiseqc(BAST_ChannelHandle, uint8_t options);
#ifndef BAST_EXCLUDE_MI2C
BERR_Code BAST_g3_P_WriteMi2c(BAST_ChannelHandle, uint8_t, uint8_t*, uint8_t);
BERR_Code BAST_g3_P_ReadMi2c(BAST_ChannelHandle, uint8_t, uint8_t*, uint8_t, uint8_t *, uint8_t);
#endif
BERR_Code BAST_g3_P_GetSoftDecisionBuf(BAST_ChannelHandle, int16_t*, int16_t*);
BERR_Code BAST_g3_P_FreezeEq(BAST_ChannelHandle, bool);
BERR_Code BAST_g3_P_PowerDown(BAST_ChannelHandle, uint32_t);
BERR_Code BAST_g3_P_PowerUp(BAST_ChannelHandle, uint32_t);
BERR_Code BAST_g3_P_ReadRegister_isrsafe(BAST_ChannelHandle, uint32_t, uint32_t*);
BERR_Code BAST_g3_P_WriteRegister_isrsafe(BAST_ChannelHandle, uint32_t, uint32_t*);
BERR_Code BAST_g3_P_ReadConfig(BAST_ChannelHandle, uint16_t, uint8_t*, uint8_t);
BERR_Code BAST_g3_P_WriteConfig(BAST_ChannelHandle, uint16_t, uint8_t*, uint8_t);
BERR_Code BAST_g3_P_GetLockChangeEventHandle(BAST_ChannelHandle, BKNI_EventHandle*);
#ifndef BAST_EXCLUDE_FTM
BERR_Code BAST_g3_P_InitCbInfo(BAST_Handle h);
BERR_Code BAST_g3_P_GetFtmEventHandle(BAST_Handle, BKNI_EventHandle*);
BERR_Code BAST_g3_P_ResetFtm(BAST_Handle);
BERR_Code BAST_g3_P_ReadFtm(BAST_Handle, uint8_t *pBuf, uint8_t *n);
BERR_Code BAST_g3_P_WriteFtm(BAST_Handle, uint8_t *pBuf, uint8_t n);
BERR_Code BAST_g3_P_PowerDownFtm(BAST_Handle);
BERR_Code BAST_g3_P_PowerUpFtm(BAST_Handle);
#endif
BERR_Code BAST_g3_P_GetDiseqcEventHandle(BAST_ChannelHandle, BKNI_EventHandle*);
BERR_Code BAST_g3_P_GetDiseqcVsenseEventHandles(BAST_ChannelHandle, BKNI_EventHandle*, BKNI_EventHandle*);
BERR_Code BAST_g3_P_AbortAcq(BAST_ChannelHandle);
BERR_Code BAST_g3_P_PeakScan(BAST_ChannelHandle h, uint32_t tunerFreq);
BERR_Code BAST_g3_P_GetPeakScanStatus(BAST_ChannelHandle h, BAST_PeakScanStatus*);
BERR_Code BAST_g3_P_GetPeakScanEventHandle(BAST_ChannelHandle, BKNI_EventHandle*);
#ifndef BAST_EXCLUDE_STATUS_EVENTS
BERR_Code BAST_g3_P_EnableStatusInterrupts(BAST_ChannelHandle, bool);
BERR_Code BAST_g3_P_GetStatusEventHandle(BAST_ChannelHandle h, BKNI_EventHandle *pEvent);
#endif
BERR_Code BAST_g3_P_EnableSpurCanceller(BAST_ChannelHandle h, uint8_t n, BAST_SpurCancellerConfig *pConfig);
BERR_Code BAST_g3_P_SetSearchRange(BAST_Handle h, uint32_t searchRange);
BERR_Code BAST_g3_P_GetSearchRange(BAST_Handle h, uint32_t *pSearchRange);
#ifndef BAST_EXCLUDE_LDPC
BERR_Code BAST_g3_P_SetAmcScramblingSeq(BAST_ChannelHandle h, uint32_t xseed, uint32_t plhdrscr1, uint32_t plhdrscr2, uint32_t plhdrscr3);
#endif
BERR_Code BAST_g3_P_SetTunerFilter(BAST_ChannelHandle h, uint32_t cutoffHz);
BERR_Code BAST_g3_P_GetSignalDetectStatus(BAST_ChannelHandle h, BAST_SignalDetectStatus *pStatus);
BERR_Code BAST_g3_P_SetOutputTransportSettings(BAST_ChannelHandle, BAST_OutputTransportSettings*);
BERR_Code BAST_g3_P_GetOutputTransportSettings(BAST_ChannelHandle, BAST_OutputTransportSettings*);
BERR_Code BAST_g3_P_SetDiseqcSettings(BAST_ChannelHandle, BAST_DiseqcSettings*);
BERR_Code BAST_g3_P_GetDiseqcSettings(BAST_ChannelHandle, BAST_DiseqcSettings*);
BERR_Code BAST_g3_P_SetNetworkSpec(BAST_Handle, BAST_NetworkSpec);
BERR_Code BAST_g3_P_GetNetworkSpec(BAST_Handle, BAST_NetworkSpec *);
BERR_Code BAST_g3_P_SetFskChannel(BAST_Handle h, BAST_FskChannel fskTxChannel, BAST_FskChannel fskRxChannel);
BERR_Code BAST_g3_P_GetFskChannel(BAST_Handle h, BAST_FskChannel *fskTxChannel, BAST_FskChannel *fskRxChannel);
BERR_Code BAST_g3_P_SetPeakScanSymbolRateRange(BAST_ChannelHandle, uint32_t, uint32_t);
BERR_Code BAST_g3_P_GetPeakScanSymbolRateRange(BAST_ChannelHandle, uint32_t*, uint32_t*);
#ifdef BAST_HAS_WFE
BERR_Code BAST_g3_P_SetAdcSelect(BAST_ChannelHandle h, uint8_t);
BERR_Code BAST_g3_P_GetAdcSelect(BAST_ChannelHandle h, uint8_t*, uint8_t*);
#else
BERR_Code BAST_g3_P_ReadAgc(BAST_ChannelHandle, BAST_Agc, uint32_t*);
BERR_Code BAST_g3_P_WriteAgc(BAST_ChannelHandle, BAST_Agc, uint32_t*);
BERR_Code BAST_g3_P_FreezeAgc(BAST_ChannelHandle, BAST_Agc, bool);
#endif

/* private functions */

/* bast_<chip>.c */
BERR_Code BAST_g3_P_InitHandle(BAST_Handle h);
BERR_Code BAST_g3_P_InitConfig(BAST_ChannelHandle h);

/* bast_g3_priv.c */
BERR_Code BAST_g3_P_DisableChannelInterrupts_isr(BAST_ChannelHandle h, bool bDisableDiseqc, bool bDisableMi2c);
BERR_Code BAST_g3_P_DisableChannelInterrupts(BAST_ChannelHandle h, bool bDisableDiseqc, bool bDisableMi2c);

/* bast_g3_priv_acq.c */
BERR_Code BAST_g3_P_InitAllChannels(BAST_Handle h);
BERR_Code BAST_g3_P_InitAllChannels_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_InitChannel_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_WriteVerifyRegister(BAST_ChannelHandle h, uint32_t reg, uint32_t *val);
BERR_Code BAST_g3_P_LogTraceBuffer_isr(BAST_ChannelHandle h, BAST_TraceEvent event);
BERR_Code BAST_g3_P_ProcessScript_isrsafe(BAST_ChannelHandle hChn, uint32_t const *pScr);
uint32_t BAST_g3_P_GCF_isr(uint32_t m, uint32_t n);
BERR_Code BAST_g3_P_SetFlifOffset_isr(BAST_ChannelHandle h, int32_t offset);
BERR_Code BAST_g3_P_SetDefaultSampleFreq_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_UpdateErrorCounters_isrsafe(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_GetSymbolRateError(BAST_ChannelHandle h, int32_t *pSymRateError);
BERR_Code BAST_g3_P_GetCarrierError_isrsafe(BAST_ChannelHandle h, int32_t *pCarrierError);
BERR_Code BAST_g3_P_GetActualMode(BAST_ChannelHandle h, BAST_Mode *pActualMode);
BERR_Code BAST_g3_P_GetActualMode_isr(BAST_ChannelHandle h, BAST_Mode *pActualMode);
BERR_Code BAST_g3_P_ResetErrorCounters(BAST_ChannelHandle h);
void BAST_g3_P_IncrementInterruptCounter_isr(BAST_ChannelHandle h, BAST_g3_IntID idx);
BERR_Code BAST_g3_P_InitChannelHandle(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_PrepareNewAcquisition(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_TuneAcquire0_isr(BAST_ChannelHandle h);
void BAST_g3_P_IndicateLocked_isrsafe(BAST_ChannelHandle h);
void BAST_g3_P_IndicateNotLocked_isrsafe(BAST_ChannelHandle h);
void BAST_g3_P_Lock_isr(void *p, int param);
void BAST_g3_P_NotLock_isr(void *p, int param);
BERR_Code BAST_g3_P_InitHandleDefault(BAST_Handle h);
void BAST_g3_P_ToggleBit_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t mask);
BERR_Code BAST_g3_P_SetSampleFreq_isr(BAST_ChannelHandle h, uint32_t ndiv, uint32_t mdiv);
BERR_Code BAST_g3_P_GetSampleFreq_isrsafe(BAST_ChannelHandle h, uint32_t *pSampleFreq);
void BAST_g3_P_ReadModifyWriteRegister_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t and_mask, uint32_t or_mask);
void BAST_g3_P_OrRegister_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t or_mask);
void BAST_g3_P_AndRegister_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t and_mask);
BERR_Code BAST_g3_P_SdsPowerUp(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_SdsPowerDown(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_SetFunctTable_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_StartTracking_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_Reacquire_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_SetBaudBw_isr(BAST_ChannelHandle h, uint32_t bw, uint32_t damp);
BERR_Code BAST_g3_P_SetCarrierBw_isr(BAST_ChannelHandle h, uint32_t bw, uint32_t damp);
BERR_Code BAST_g3_P_ConfigOif_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_InitBert_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_StartBert_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_SetAgcTrackingBw_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_SetFfeMainTap_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_ConfigAgc_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_NonLegacyModeAcquireInit_isr(BAST_ChannelHandle h);
void BAST_g3_P_GetRegisterWriteWaitTime_isrsafe(BAST_ChannelHandle h, uint32_t reg, uint32_t *wait_time);
BERR_Code BAST_g3_P_Acquire1_isr(BAST_ChannelHandle h);
void BAST_g3_P_GetCarrierErrorFli_isrsafe(BAST_ChannelHandle h, int32_t *pCarrierErrorFli);
void BAST_g3_P_GetCarrierErrorPli_isrsafe(BAST_ChannelHandle h, int32_t *pCarrierErrorPli);
BERR_Code BAST_g3_P_SdsDisableOif_isrsafe(BAST_ChannelHandle h);
uint32_t BAST_g3_P_GetNumDecimatingFilters_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_SetDecimationFilters_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_SetBfos_isr(BAST_ChannelHandle h);
void BAST_g3_P_SdsPowerDownOpll_isrsafe(BAST_ChannelHandle h);
void BAST_g3_P_SdsPowerUpOpll_isr(BAST_ChannelHandle h);
#ifndef BAST_EXCLUDE_STATUS_EVENTS
BERR_Code BAST_g3_P_CheckStatusIndicators_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_ResetStatusIndicators(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_ResetStatusIndicators_isr(BAST_ChannelHandle h);
#endif
#ifndef BAST_HAS_WFE
void BAST_g3_P_GetSampleFreqMN_isr(BAST_ChannelHandle h, uint32_t *pN, uint32_t *pM);
#endif

/* bast_g3_priv_cwc.c */
#ifndef BAST_EXCLUDE_SPUR_CANCELLER
BERR_Code BAST_g3_P_ClearSpurCancellerConfig(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_DisableSpurCanceller(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_DisableSpurCanceller_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_InitCWC_isr(BAST_ChannelHandle h, BAST_g3_FUNCT funct);
BERR_Code BAST_g3_P_ResetCWC_isr(BAST_ChannelHandle h);
#endif

/* bast_g3_priv_dft.c */
BERR_Code BAST_g3_P_DftSearchCarrier_isr(BAST_ChannelHandle h, BAST_g3_FUNCT funct);
BERR_Code BAST_g3_P_DftPeakScan_isr(BAST_ChannelHandle h);
#if BCHP_CHIP==4538
void BAST_g3_P_DftDone_isr(void *p, int param);
#else
BERR_Code BAST_g3_P_DftDumpBins_isr(BAST_ChannelHandle h);
#endif

/* bast_g3_priv_diseqc.c */
BERR_Code BAST_g3_P_DiseqcPowerUp(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_DiseqcPowerDown(BAST_ChannelHandle h);
void BAST_g3_P_DiseqcDone_isr(void *p, int param);
void BAST_g3_P_DiseqcOverVoltage_isr(void *p, int param);
void BAST_g3_P_DiseqcUnderVoltage_isr(void *p, int param);
void BAST_g3_P_DiseqcTxFifoAlmostEmpty_isr(void *p, int param);
void BAST_g3_P_DisableDiseqcInterrupts_isr(BAST_ChannelHandle h);

/* bast_g3_priv_fsk.c */
#ifdef BAST_ENABLE_GENERIC_FSK
BERR_Code BAST_g3_P_ListenFsk(BAST_Handle h, uint8_t n);
BERR_Code BAST_g3_Fsk_P_InitDevice(BAST_Handle);
BERR_Code BAST_g3_Fsk_P_EnableFskInterrupts(BAST_Handle h, bool bEnable);
void BAST_g3_Fsk_P_HandleInterrupt_isr(void *p, int param);
#endif

/* bast_g3_priv_ftm.c */
BERR_Code BAST_g3_P_PowerUpFskphy_isrsafe(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_PowerDownFskphy_isrsafe(BAST_ChannelHandle h);
#ifndef BAST_EXCLUDE_FTM
#ifndef BAST_ENABLE_GENERIC_FSK
BERR_Code BAST_g3_Ftm_P_InitDevice_isrsafe(BAST_Handle);
BERR_Code BAST_g3_Ftm_P_EnableFtmInterrupts(BAST_Handle h, bool bEnable);
void BAST_g3_Ftm_P_HandleInterrupt_isr(void *p, int param);
#else
#define BAST_g3_Ftm_P_InitDevice_isrsafe(h) BAST_g3_Fsk_P_InitDevice(h)
#define BAST_g3_Ftm_P_EnableFtmInterrupts(h, bEnable) BAST_g3_Fsk_P_EnableFskInterrupts(h, bEnable)
#define BAST_g3_Ftm_P_HandleInterrupt_isr BAST_g3_Fsk_P_HandleInterrupt_isr
#endif
#endif

/* bast_g3_priv_hp.c */
void BAST_g3_P_HpLock_isr(void *p, int param);
void BAST_g3_P_HpFrameBoundary_isr(void *p, int param);
BERR_Code BAST_g3_P_HpEnable(BAST_ChannelHandle h, bool bEnable);
BERR_Code BAST_g3_P_HpEnable_isr(BAST_ChannelHandle h, bool bEnable);
BERR_Code BAST_g3_P_HpAcquire_isr(BAST_ChannelHandle h, BAST_g3_FUNCT funct);
BERR_Code BAST_g3_P_OnHpTimeOut_isr(BAST_ChannelHandle h);
bool BAST_g3_P_IsHpLocked_isrsafe(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_HpDisableInterrupts_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_HpIsSpinv_isrsafe(BAST_ChannelHandle h, bool *pSpinv);
BERR_Code BAST_g3_P_HpGetFreqOffsetEstimate_isrsafe(BAST_ChannelHandle h, int32_t *pFreq);

/* bast_g3_priv_ldpc.c */
#ifndef BAST_EXCLUDE_LDPC
BERR_Code BAST_g3_P_LdpcSetFunctTable_isr(BAST_ChannelHandle h);
bool BAST_g3_P_IsLdpcPilotOn_isrsafe(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_LdpcPowerUp_isrsafe(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_LdpcPowerDown(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_GetAfecClock_isrsafe(BAST_ChannelHandle h, uint32_t *pFreq);
bool BAST_g3_P_IsLdpcOn_isrsafe(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_LdpcUpdateBlockCounters_isrsafe(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_LdpcDisableLockInterrupts_isr(BAST_ChannelHandle h);
#if (BCHP_CHIP==4528) || (BCHP_CHIP==4538)
BERR_Code BAST_g3_P_LdpcEnableDynamicPowerShutDown_isrsafe(BAST_ChannelHandle h, bool bEnable);
#endif
#endif

/* bast_g3_priv_lna.c */
#ifndef BAST_EXCLUDE_BCM3445
BERR_Code BAST_g3_P_ConfigBcm3445(BAST_Handle, BAST_Bcm3445Settings*);
BERR_Code BAST_g3_P_MapBcm3445ToTuner(BAST_ChannelHandle, BAST_Mi2cChannel, BAST_Bcm3445OutputChannel);
BERR_Code BAST_g3_P_GetBcm3445Status(BAST_ChannelHandle, BAST_Bcm3445Status*);
#endif

/* bast_g3_priv_mi2c.c */
#ifndef BAST_EXCLUDE_MI2C
void BAST_g3_P_Mi2c_isr(void *p, int param);
BERR_Code BAST_g3_P_Mi2cReset(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_Mi2cDisableInterrupts_isr(BAST_ChannelHandle h);
#endif

/* bast_g3_priv_plc.c */
BERR_Code BAST_g3_P_SetPlc_isr(BAST_ChannelHandle h, uint32_t bw, uint32_t damp);
BERR_Code BAST_g3_P_ConfigPlc_isr(BAST_ChannelHandle h, bool bAcq);

/* bast_g3_priv_qpsk.c */
BERR_Code BAST_g3_P_QpskSetFunctTable_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskUpdateErrorCounters_isrsafe(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_QpskDisableLockInterrupts_isr(BAST_ChannelHandle h);

/* bast_g3_priv_snr.c */
BERR_Code BAST_g3_P_GetSnr(BAST_ChannelHandle h, uint32_t *pSnr);
BERR_Code BAST_g3_P_GetSnr_isr(BAST_ChannelHandle h, uint32_t *pSnr);

/* bast_g3_priv_timer.c */
BERR_Code BAST_g3_P_EnableTimer(BAST_ChannelHandle h, BAST_TimerSelect t, uint32_t delay, BAST_g3_FUNCT func);
BERR_Code BAST_g3_P_EnableTimer_isr(BAST_ChannelHandle h, BAST_TimerSelect t, uint32_t delay, BAST_g3_FUNCT func);
BERR_Code BAST_g3_P_DisableTimer_isr(BAST_ChannelHandle h, BAST_TimerSelect t);
bool BAST_g3_P_IsTimerRunning_isrsafe(BAST_ChannelHandle h, BAST_TimerSelect t);
bool BAST_g3_P_IsTimerExpired_isrsafe(BAST_ChannelHandle h, BAST_TimerSelect t);
void BAST_g3_P_BaudTimer_isr(void *p, int param);
void BAST_g3_P_BerTimer_isr(void *p, int param);
void BAST_g3_P_Gen1Timer_isr(void *p, int param);
void BAST_g3_P_Gen2Timer_isr(void *p, int param);
void BAST_g3_P_Gen3Timer_isr(void *p, int param);
void BAST_g3_P_Diseqc1Timer_isr(void *p, int param);
void BAST_g3_P_Diseqc2Timer_isr(void *p, int param);

/* bast_g3_priv_tuner.c */
BERR_Code BAST_g3_P_TunerQuickTune_isr(BAST_ChannelHandle h, BAST_g3_FUNCT nextFunct);
BERR_Code BAST_g3_P_TunerSetFreq_isr(BAST_ChannelHandle h, BAST_g3_FUNCT nextFunct);
BERR_Code BAST_g3_P_TunerInit(BAST_ChannelHandle h, BAST_g3_FUNCT nextFunct);
BERR_Code BAST_g3_P_TunerInit_isr(BAST_ChannelHandle h, BAST_g3_FUNCT nextFunct);
BERR_Code BAST_g3_P_TunerPowerUp(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_TunerPowerDown(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_TunerUpdateActualTunerFreq_isr(BAST_ChannelHandle h);
#ifndef BAST_HAS_WFE
BERR_Code BAST_g3_P_TunerBypassLna_isrsafe(BAST_ChannelHandle h, bool bBypass, bool bPgaMode);
BERR_Code BAST_g3_P_TunerSetFilter_isr(BAST_ChannelHandle h, bool bTracking);
BERR_Code BAST_g3_P_TunerConfigLna(BAST_Handle h, BAST_TunerLnaSettings *pSettings);
BERR_Code BAST_g3_P_TunerConfigLna_isr(BAST_Handle h, BAST_TunerLnaSettings *pSettings);
BERR_Code BAST_g3_P_TunerGetLnaStatus(BAST_ChannelHandle h, BAST_TunerLnaStatus *pStatus);
BERR_Code BAST_g3_P_TunerGetLockStatus_isrsafe(BAST_ChannelHandle h, bool *bRefPllLocked, bool *bMixPllLocked);
BERR_Code BAST_g3_P_TunerIndirectWrite_isrsafe(BAST_ChannelHandle h, BAST_TunerIndirectRegGroup type, uint8_t addr, uint32_t val);
BERR_Code BAST_g3_P_TunerIndirectRead_isrsafe(BAST_ChannelHandle h, BAST_TunerIndirectRegGroup type, uint8_t addr, uint32_t *pVal);
BERR_Code BAST_g3_P_TunerRunDco_isr(BAST_ChannelHandle h, BAST_TimerSelect t, uint16_t runtime);
BERR_Code BAST_g3_P_TunerRunDco1_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_TunerGetActualLOFreq_isrsafe(BAST_ChannelHandle h, uint32_t *pFreq);
BERR_Code BAST_g3_P_TunerGetLoVco_isr(BAST_ChannelHandle h, uint32_t *pFreqMhz, uint32_t *pDiv);
#endif

/* bast_g3_priv_turbo.c */
#ifndef BAST_EXCLUDE_TURBO
BERR_Code BAST_g3_P_TurboPowerUp_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_TurboPowerDown(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_TurboUpdateErrorCounters_isrsafe(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_GetTfecClock_isrsafe(BAST_ChannelHandle h, uint32_t *pFreq);
BERR_Code BAST_g3_P_TurboSetFunctTable_isr(BAST_ChannelHandle h);
bool BAST_g3_P_IsTurboOn_isrsafe(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_TurboDisableLockInterrupts_isr(BAST_ChannelHandle h);
void BAST_g3_P_TurboSync_isr(void *p, int param);
BERR_Code BAST_g3_P_TurboDisableSyncInterrupt_isr(BAST_ChannelHandle h);
#endif

#ifdef BAST_HAS_WFE
BERR_Code BAST_g3_P_SetChanDecFcw_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_GetChanAddress(BAST_ChannelHandle h, uint32_t reg, uint32_t *pAddr);
BERR_Code BAST_g3_P_ConfigChanAgc_isr(BAST_ChannelHandle h, bool bTracking);
BERR_Code BAST_g3_P_GetChanAciCoeff_isr(BAST_ChannelHandle h, int16_t **p);
BERR_Code BAST_g3_P_SetChanAciCoeff_isr(BAST_ChannelHandle h, int16_t *pCoeff);
BERR_Code BAST_g3_P_EnableNotch_isr(BAST_ChannelHandle h, BAST_NotchSelect n, int32_t freqHz, BAST_NotchSettings *pSettings);
BERR_Code BAST_g3_P_DisableNotch_isrsafe(BAST_ChannelHandle h, BAST_NotchSelect n);
BERR_Code BAST_g3_P_UpdateNotch_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_SetNotchSettings(BAST_Handle h, uint8_t n, BAST_NotchSettings *pSettings);
BERR_Code BAST_g3_GetNotchSettings(BAST_Handle h, uint8_t *n, BAST_NotchSettings *pSettings);
BERR_Code BAST_g3_P_SetNotch_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_UpdateNotch_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_P_DisableAllNotch_isr(BAST_ChannelHandle h);
BERR_Code BAST_g3_ResetXbarFifo(BAST_Handle h, uint8_t adcSelect);
#endif /* BAST_HAS_WFE */


#ifdef __cplusplus
}
#endif

#endif /* BAST_g3_PRIV_H__ */
