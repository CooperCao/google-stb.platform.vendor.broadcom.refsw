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
#ifndef _BTHD_7550_PRIV_H__
#define _BTHD_7550_PRIV_H__

#include "bthd_7550.h"
#include "btmr.h"

#define BTHD_CHK_RETCODE(x) \
  { if ((retCode = (x)) != BERR_SUCCESS) goto done; }

typedef uint32_t BTHD_RESULT;

/***************************************************************************
 * Register Field Combinations Definition
 ***************************************************************************/
#define BCHP_THD_CORE_EQ_EXP_USE_FORMAT_OV_SHIFT BCHP_THD_CORE_EQ_FORMAT_OV_SHIFT 
#define BCHP_THD_CORE_EQ_EXP_USE_FORMAT_OV_MASK  (BCHP_THD_CORE_EQ_FORMAT_OV_MASK | BCHP_THD_CORE_EQ_USE_FORMAT_OV_MASK) 

#define BCHP_THD_CORE_BYP_EXP_AVG_NSE_SHIFT BCHP_THD_CORE_BYP_EXP_NSE_BYP_SHIFT 
#define BCHP_THD_CORE_BYP_EXP_AVG_NSE_MASK  (BCHP_THD_CORE_BYP_EXP_NSE_BYP_MASK | BCHP_THD_CORE_BYP_EXP_AVG_NSE_BYP_MASK) 

#define BCHP_THD_CORE_CE_ACE_ACT_SHIFT BCHP_THD_CORE_CE_ACE_SHIFT 
#define BCHP_THD_CORE_CE_ACE_ACT_MASK  (BCHP_THD_CORE_CE_ACE_MASK | BCHP_THD_CORE_CE_ACT_MASK)

#define BCHP_THD_CORE_RST_RS_FEC_BCH_TPS_SNR_RST_SHIFT BCHP_THD_CORE_RST_SNR_RST_SHIFT 
#define BCHP_THD_CORE_RST_RS_FEC_BCH_TPS_SNR_RST_MASK  (BCHP_THD_CORE_RST_SNR_RST_MASK | BCHP_THD_CORE_RST_TPS_RST_MASK  | BCHP_THD_CORE_RST_TPS_SAVE_RST_MASK | BCHP_THD_CORE_RST_BCH_ERC_RST_MASK | BCHP_THD_CORE_RST_FEC_RST_MASK | BCHP_THD_CORE_RST_RS_RST_MASK) 

#define BCHP_THD_CORE_FRZ_NOTCH_FRZ_SHIFT BCHP_THD_CORE_FRZ_NOTCH0M_FRZ_SHIFT 
#define BCHP_THD_CORE_FRZ_NOTCH_FRZ_MASK  (BCHP_THD_CORE_FRZ_NOTCH0M_FRZ_MASK | BCHP_THD_CORE_FRZ_NOTCH0_FRZ_MASK  | BCHP_THD_CORE_FRZ_NOTCH1_FRZ_MASK | BCHP_THD_CORE_FRZ_NOTCH2_FRZ_MASK | BCHP_THD_CORE_FRZ_NOTCH3_FRZ_MASK | BCHP_THD_CORE_FRZ_NOTCH4_FRZ_MASK | BCHP_THD_CORE_FRZ_NOTCH_DDFS_FRZ_MASK)

#define BCHP_THD_CORE_TPS_OV_RX_INFO_SHIFT BCHP_THD_CORE_TPS_OV_QAM_SHIFT
#define BCHP_THD_CORE_TPS_OV_RX_INFO_MASK  (BCHP_THD_CORE_TPS_OV_QAM_MASK | BCHP_THD_CORE_TPS_OV_reserved5_MASK  | BCHP_THD_CORE_TPS_OV_HIERARCHY_MASK | BCHP_THD_CORE_TPS_OV_INDEPTH_MASK | BCHP_THD_CORE_TPS_OV_reserved4_MASK | BCHP_THD_CORE_TPS_OV_CRATE_HP_MASK)

#define BCHP_THD_CORE_TPS_RX_INFO_SHIFT BCHP_THD_CORE_TPS_QAM_SHIFT
#define BCHP_THD_CORE_TPS_RX_INFO_MASK  (BCHP_THD_CORE_TPS_QAM_MASK | BCHP_THD_CORE_TPS_reserved5_MASK  | BCHP_THD_CORE_TPS_HIERARCHY_MASK | BCHP_THD_CORE_TPS_INDEPTH_MASK | BCHP_THD_CORE_TPS_reserved4_MASK | BCHP_THD_CORE_TPS_CRATE_HP_MASK)

#define BCHP_THD_CORE_BYP_EQ_BYP_SHIFT BCHP_THD_CORE_BYP_EQ_BIT_SCALING_BYP_SHIFT
#define BCHP_THD_CORE_BYP_EQ_BYP_MASK  (BCHP_THD_CORE_BYP_EQ_BIT_SCALING_BYP_MASK | BCHP_THD_CORE_BYP_NSE_AVG_BYP_MASK  | BCHP_THD_CORE_BYP_EXP_CHAN_BYP_MASK | BCHP_THD_CORE_BYP_EXP_NSE_BYP_MASK | BCHP_THD_CORE_BYP_EXP_AVG_NSE_BYP_MASK) 

/***************************************************************************
 * Enumerated Type Definitions (General)
 ***************************************************************************/
typedef enum THD_Standard
{
  THD_Standard_DVBT=0,
  THD_Standard_ISDBT=1
} THD_Standard_t;

typedef enum THD_FrontEndMode
{
  THD_FrontEndMode_Baseband=0,
  THD_FrontEndMode_IF=1
} THD_FrontEndMode_t;

typedef enum THD_ChannelEstimatorMode
{
  THD_ChannelEstimatorMode_Fixed=0,
  THD_ChannelEstimatorMode_Pedestrian=1,
  THD_ChannelEstimatorMode_Mobile=2,
  THD_ChannelEstimatorMode_Auto=3
} THD_ChannelEstimatorMode_t;

typedef enum THD_FFTWindowMode
{
  THD_FFTWindowMode_InSpan=0,
  THD_FFTWindowMode_OutOfSpanPost=1,
  THD_FFTWindowMode_OutOfSpanPre=2,
  THD_FFTWindowMode_Auto=3
} THD_FFTWindowMode_t;

typedef enum THD_CoChannelMode
{
  THD_CoChannelMode_None=0,
  THD_CoChannelMode_Auto=1,
  THD_CoChannelMode_Other=2
} THD_CoChannelMode_t;

typedef enum THD_ImpulseMode
{
  THD_ImpulseMode_None=0,
  THD_ImpulseMode_Auto=1,
  THD_ImpulseMode_Other=2
} THD_ImpulseMode_t;

typedef enum THD_SpectrumMode
{
  THD_SpectrumMode_Normal=0,
  THD_SpectrumMode_Inverted=1
} THD_SpectrumMode_t;

typedef enum THD_DiversityMode
{
  THD_DiversityMode_None=0,
  THD_DiversityMode_Master=1,
  THD_DiversityMode_Slave=2
} THD_DiversityMode_t;

typedef enum THD_TransGuardMode
{
  THD_TransGuardMode_Manual=0,
  THD_TransGuardMode_Auto=1,
  THD_TransGuardMode_Auto_DVBT=2,
  THD_TransGuardMode_Auto_ISDBT=3
} THD_TransGuardMode_t;

typedef enum THD_TPSMode
{
  THD_TPSMode_Manual=0,
  THD_TPSMode_Auto=1
} THD_TPSMode_t;

typedef enum THD_CarrierRange
{
  THD_CarrierRange_Narrow=0,
  THD_CarrierRange_Wide=1
} THD_CarrierRange_t;

typedef enum THD_PriorityMode
{
  THD_PriorityMode_High=0,
  THD_PriorityMode_Low=1
} THD_PriorityMode_t;

typedef enum THD_FrequencyInterpolatorMode
{
  THD_FrequencyInterpolatorMode_InSpan=0,
  THD_FrequencyInterpolatorMode_OutOfSpanPost=1,
  THD_FrequencyInterpolatorMode_OutOfSpanPre_1_4=2,
  THD_FrequencyInterpolatorMode_OutOfSpanPre_1_8=3,
  THD_FrequencyInterpolatorMode_OutOfSpanPre_1_16=4
} THD_FrequencyInterpolatorMode_t;

typedef enum THD_TSMode
{
  THD_TSMode_None=0,
  THD_TSMode_Serial=1,
  THD_TSMode_Parallel=2
} THD_TSMode_t;

typedef enum THD_AcquireMode
{
  THD_AcquireMode_Manual=0,
  THD_AcquireMode_Auto=1
} THD_AcquireMode_t;

/***************************************************************************
 * Enumerated Type Definitions (TPS Parameters, Matches register definitions)
 ***************************************************************************/
typedef enum THD_Bandwidth
{
  THD_Bandwidth_8MHz=0,
  THD_Bandwidth_7MHz=1,
  THD_Bandwidth_6MHz=2,
  THD_Bandwidth_5MHz=3
} THD_Bandwidth_t;

typedef enum THD_TransmissionMode
{
  THD_TransmissionMode_2k=0,
  THD_TransmissionMode_8k=1,
  THD_TransmissionMode_4k=2
} THD_TransmissionMode_t;

typedef enum THD_GuardInterval
{
  THD_GuardInterval_1_32=0,
  THD_GuardInterval_1_16=1,
  THD_GuardInterval_1_8=2,
  THD_GuardInterval_1_4=3
} THD_GuardInterval_t;

typedef enum THD_CodeRate
{
  THD_CodeRate_1_2=0,
  THD_CodeRate_2_3=1,
  THD_CodeRate_3_4=2,
  THD_CodeRate_5_6=3,
  THD_CodeRate_7_8=4
} THD_CodeRate_t;

typedef enum THD_Interleaving
{
  THD_Interleaving_Native=0,
  THD_Interleaving_InDepth=1
} THD_Interleaving_t;

typedef enum THD_Hierarchy
{
  THD_Hierarchy_None=0,
  THD_Hierarchy_1=1,
  THD_Hierarchy_2=2,
  THD_Hierarchy_4=3
} THD_Hierarchy_t;

typedef enum THD_Qam
{
  THD_Qam_Qpsk=0,
  THD_Qam_16Qam=1,
  THD_Qam_64Qam=2,
  THD_Qam_Dqpsk=3
} THD_Qam_t;

/***************************************************************************
 * Enumerated Type Definitions (TMCC Parameters, Matches register definitions)
 ***************************************************************************/
typedef enum THD_TMCCMode
{
  THD_TMCCMode_Manual=0,
  THD_TMCCMode_Auto=1
} THD_TMCCMode_t;

typedef enum THD_TimeInt
{
  THD_TimeInt_0X=0,
  THD_TimeInt_1X=1,
  THD_TimeInt_2X=2,
  THD_TimeInt_4X=3
} THD_TimeInt_t;

/***************************************************************************
 * Miscellaneous Definitions
 ***************************************************************************/
typedef enum THD_TransGuardResult
{
  THD_TransGuardResult_None=0,
  THD_TransGuardResult_NoSignal=1,
  THD_TransGuardResult_CoChannelPresent=2
} THD_TransGuardResult_t;

typedef enum THD_AcquireResult
{
  THD_AcquireResult_Lock=0,
  THD_AcquireResult_NoSignal=1,
  THD_AcquireResult_NoOFDMFound=2,
  THD_AcquireResult_NoFFTLock=3,
  THD_AcquireResult_NoCarrierLock=4,
  THD_AcquireResult_NoSPLock=5,
  THD_AcquireResult_NoTPSLock=6,
  THD_AcquireResult_NoFECLock=7,
  THD_AcquireResult_BadBER=8,
  THD_AcquireResult_InitLockState=9,
  THD_AcquireResult_NoDVBTSignal=10
} THD_AcquireResult_t;

typedef enum THD_LockStatusBit
{
  THD_LockStatusBit_TPSLock=0,
  THD_LockStatusBit_VitLock=1,
  THD_LockStatusBit_FECLock=2,
  THD_LockStatusBit_SystemLock=3,
  THD_LockStatusBit_NoDVBTSignal=4
} THD_LockStatusBit_t;


typedef enum THD_Pr
{
  THD_Pr_Disable=0,
  THD_Pr_Enable=1
} THD_Pr_t;


typedef enum THD_Ews
{
  THD_Ews_Disable=0,
  THD_Ews_Enable=1
} THD_Ews_t;

typedef enum THD_AcquireStartMode
{
  THD_AcquireStartMode_Auto = 1,
	THD_AcquireStartMode = 2,
	THD_AcquireStartMode_ResetStatus = 4,
	THD_AcquireStartMode_Scan = 8
} THD_AcquireStartMode_t;

typedef enum THD_NexusStatusMode
{
  THD_NexusStatusMode_EnableStatusForNexus = 1
} THD_NexusStatusMode_t;

typedef enum THD_Event
{
  THD_Event_LockUpdate = 1,
  THD_Event_UnlockStopEWS = 2, /* if this channel is unlock, stop reporting EWS */
  THD_Event_LockStartEWS = 4     /* if this channel is lock, start reporting EWS */
} THD_Event_t;


#define THD_Qam_N           3
#define THD_GuardInterval_N 4
#define THD_CodeRate_N      5

#define THD_FrequencyInterpolatorCoefLength   396
#define THD_SNRAlpha_N 1
#define THD_SNRAlpha_D 16
#define THD_StatusFramesForReset 10
#define THD_ICESpectrumInversionThreshold 8

  
/*************************************************************************
 * Summary:
 *   Structure for chip-specific Acquire Parameters
 *    
 * Description:
 *   This is the chip-specific component of the Thd Acquire command. It maps
 *   byte for byte to the acquire parameter inside the chip's firmware
 *     
 * See Also:
 *   None.
 *      
 ***************************************************************************/

typedef struct BTHD_7550_P_AcquireParam
{
  uint32_t                        StatusStructureAddress;
  volatile THD_AcquireStartMode_t AcquireStartMode;
  volatile THD_NexusStatusMode_t  NexusStatusMode;
  THD_Standard_t                  Standard;
  THD_FrontEndMode_t              FrontEndMode;
  THD_ChannelEstimatorMode_t      ChannelEstimatorMode;
  THD_FFTWindowMode_t             FFTWindowMode;
  THD_CoChannelMode_t             CoChannelMode;
  THD_ImpulseMode_t               ImpulseMode;
  THD_TransGuardMode_t            TransGuardMode;
  THD_TPSMode_t                   TPSMode;
  THD_CarrierRange_t              CarrierRange;
  THD_TSMode_t                    TSMode;
  THD_PriorityMode_t              PriorityMode;
  THD_AcquireMode_t               AcquireMode;
  THD_TransmissionMode_t          TransmissionMode;
  THD_GuardInterval_t             GuardInterval;
  THD_Qam_t                       Qam;
  THD_Hierarchy_t                 Hierarchy;
  THD_CodeRate_t                  CodeRateHP;
  THD_CodeRate_t                  CodeRateLP;
  THD_Pr_t                        ISDBT_PR;
  THD_Qam_t                       ISDBT_A_Qam;
  THD_Qam_t                       ISDBT_B_Qam;
  THD_Qam_t                       ISDBT_C_Qam;
  THD_CodeRate_t                  ISDBT_A_CodeRate;
  THD_CodeRate_t                  ISDBT_B_CodeRate;
  THD_CodeRate_t                  ISDBT_C_CodeRate;
  THD_TimeInt_t                   ISDBT_A_TimeInt;
  THD_TimeInt_t                   ISDBT_B_TimeInt;
  THD_TimeInt_t                   ISDBT_C_TimeInt;
  uint32_t                        ISDBT_A_Segments;
  uint32_t                        ISDBT_B_Segments;
  uint32_t                        ISDBT_C_Segments;
  THD_Bandwidth_t                 Bandwidth;
  uint32_t                        CenterFreq;
  uint32_t                        SampleFreq;
  uint32_t                        RfFreq;
  uint32_t                        TransGuardMaxThreshold;
  uint32_t                        TransGuardMaxMinRatioThreshold[4];
  uint32_t                        SoftwareWorkaroundMode;
  THD_TMCCMode_t                  ISDBT_TMCCMode;
  THD_Event_t					  EventMode;
} BTHD_7550_P_AcquireParam;
    
/***************************************************************************
 * Summary:
 *   Structure for THD status
 *    
 * Description:
 *   This structure contains THD status that is return by the chip
 *     
 * See Also:
 *   None.
 *      
 ***************************************************************************/


typedef struct BTHD_P_7550_THDStatus
{
  uint32_t               Lock;
  THD_PriorityMode_t     Priority;
  THD_TransmissionMode_t TransmissionMode;
  THD_GuardInterval_t    GuardInterval;
  THD_Qam_t              Qam;
  THD_Hierarchy_t        Hierarchy;
  THD_Interleaving_t     Interleaving;
  THD_CodeRate_t         CodeRateHP;
  THD_CodeRate_t         CodeRateLP;
  uint32_t               CellID;
  THD_Ews_t              ISDBT_EWS;
  THD_Pr_t               ISDBT_PR;
  THD_Qam_t              ISDBT_A_Qam;
  THD_CodeRate_t         ISDBT_A_CodeRate;
  THD_TimeInt_t          ISDBT_A_TimeInt;
  uint32_t               ISDBT_A_Segments;
  THD_Qam_t              ISDBT_B_Qam;
  THD_CodeRate_t         ISDBT_B_CodeRate;
  THD_TimeInt_t          ISDBT_B_TimeInt;
  uint32_t               ISDBT_B_Segments;
  THD_Qam_t              ISDBT_C_Qam;
  THD_CodeRate_t         ISDBT_C_CodeRate;
  THD_TimeInt_t          ISDBT_C_TimeInt;
  uint32_t               ISDBT_C_Segments;
  int32_t                CarrierOffset;
  int32_t                TimingOffset;
  uint32_t               RagcGain;
  uint32_t               IagcGain;
  int32_t                Dagc1Gain;
  int32_t                Dagc2Gain;
  int32_t                SNR;
  int32_t                SNRData;
  int32_t                ISDBT_A_SNRData;
  int32_t                ISDBT_B_SNRData;
  int32_t                ISDBT_C_SNRData;
  int32_t                SNRPilot;
  uint32_t               PreVitBER;
  uint32_t               VitBER;
  uint32_t               TS_PER;
  uint32_t               ISDBT_A_VitBER;   
  uint32_t               ISDBT_A_TS_PER;
  uint32_t               ISDBT_B_VitBER;   
  uint32_t               ISDBT_B_TS_PER;
  uint32_t               ISDBT_C_VitBER;   
  uint32_t               ISDBT_C_TS_PER;
  uint32_t               TS_ESR;
  uint32_t               TS_CERC;
  uint32_t               TS_UBERC;
  uint32_t               TS_CBERC;
  uint32_t               TS_NBERC;
  uint32_t               TS_TBERC;
  uint32_t               TS_UFERC;
  uint32_t               TS_NFERC;
  uint32_t               TS_TFERC;
  uint32_t               ISDBT_A_TS_CERC;
  uint32_t               ISDBT_A_TS_UBERC;
  uint32_t               ISDBT_A_TS_CBERC;
  uint32_t               ISDBT_A_TS_NBERC;
  uint32_t               ISDBT_A_TS_TBERC;
  uint32_t               ISDBT_A_TS_UFERC;
  uint32_t               ISDBT_A_TS_NFERC;
  uint32_t               ISDBT_A_TS_TFERC;
  uint32_t               ISDBT_B_TS_CERC;
  uint32_t               ISDBT_B_TS_UBERC;
  uint32_t               ISDBT_B_TS_CBERC;
  uint32_t               ISDBT_B_TS_NBERC;
  uint32_t               ISDBT_B_TS_TBERC;
  uint32_t               ISDBT_B_TS_UFERC;
  uint32_t               ISDBT_B_TS_NFERC;
  uint32_t               ISDBT_B_TS_TFERC;
  uint32_t               ISDBT_C_TS_CERC;
  uint32_t               ISDBT_C_TS_UBERC;
  uint32_t               ISDBT_C_TS_CBERC;
  uint32_t               ISDBT_C_TS_NBERC;
  uint32_t               ISDBT_C_TS_TBERC;
  uint32_t               ISDBT_C_TS_UFERC;
  uint32_t               ISDBT_C_TS_NFERC;
  uint32_t               ISDBT_C_TS_TFERC;
  uint32_t               ReacquireCount;
  uint32_t               RSSI;
  uint32_t               TS_CERC_ref;
  uint32_t               TS_UBERC_ref;
  uint32_t               TS_CBERC_ref;
  uint32_t               TS_NBERC_ref;
  uint32_t               TS_TBERC_ref;
  uint32_t               TS_UFERC_ref;
  uint32_t               TS_NFERC_ref;
  uint32_t               TS_TFERC_ref;
  uint32_t               ISDBT_A_TS_CERC_ref;
  uint32_t               ISDBT_A_TS_UBERC_ref;
  uint32_t               ISDBT_A_TS_CBERC_ref;
  uint32_t               ISDBT_A_TS_NBERC_ref;
  uint32_t               ISDBT_A_TS_TBERC_ref;
  uint32_t               ISDBT_B_TS_CERC_ref;
  uint32_t               ISDBT_B_TS_UBERC_ref;
  uint32_t               ISDBT_B_TS_CBERC_ref;
  uint32_t               ISDBT_B_TS_NBERC_ref;
  uint32_t               ISDBT_B_TS_TBERC_ref;
  uint32_t               ISDBT_C_TS_CERC_ref;
  uint32_t               ISDBT_C_TS_UBERC_ref;
  uint32_t               ISDBT_C_TS_CBERC_ref;
  uint32_t               ISDBT_C_TS_NBERC_ref;
  uint32_t               ISDBT_C_TS_TBERC_ref;
  uint32_t               AcquisitionTime;
  THD_ChannelEstimatorMode_t ChannelEstimatorMode;
  THD_FFTWindowMode_t        FFTWindowMode;
  THD_CoChannelMode_t        CoChannelMode;
  THD_Bandwidth_t            Bandwidth;
  uint32_t					    CoChannelPresent;
  uint32_t				      spare1;
  uint32_t				      spare2;
  uint32_t				      spare3;
  uint32_t				      spare4;
  uint32_t				      spare5;
  uint32_t				      spare6;
  uint32_t				      spare7;
  uint32_t				      spare8;
  uint32_t              LowICECount;
} BTHD_P_7550_THDStatus;

/***************************************************************************
 * Summary:
 *   Structure for chip-specific THD handle
 *    
 * Description:
 *   This is the chip-specific component of the BTHD_Handle.
 *     
 * See Also:
 *   None.
 *      
 ***************************************************************************/
typedef struct BTHD_7550_P_Handle
{
  BREG_Handle  hRegister;         /* handle used to access AP registers */
  BTHD_7550_P_AcquireParam  *pAcquireParam;
  BTHD_P_7550_THDStatus *pStatus;
#if (BCHP_VER >= BCHP_VER_A1)
  uint32_t *pIsdbtMemory;
#endif
  BKNI_EventHandle hLockEvent;
  BKNI_EventHandle hInterruptEvent;
  BKNI_EventHandle hEWSEvent;
  BTMR_TimerHandle hTimer;
  BTHD_AnalogInput eAnalogInput;
  uint32_t ThdLockStatus;
  
  /* THD code callbacks */
  BKNI_EventHandle hFwCorrMaxEvent;
  BKNI_EventHandle hFwSyncEvent;
  BKNI_EventHandle hSpSyncEvent;
  BKNI_EventHandle hTpsSyncEvent;
  BKNI_EventHandle hTmccSyncEvent;
  BKNI_EventHandle hFecSyncEvent;
  BKNI_EventHandle hFbcntZeroEvent;
  BINT_CallbackHandle hFwCorrMaxCallback;
  BINT_CallbackHandle hFwSyncCallback;
  BINT_CallbackHandle hSpSyncCallback;
  BINT_CallbackHandle hTpsSyncCallback;
  BINT_CallbackHandle hTmccSyncCallback;
  BINT_CallbackHandle hFecSyncCallback;
  BINT_CallbackHandle hFbcCntCallback;
} BTHD_7550_P_Handle;

/***************************************************************************
 * Summary:
 *   Reads from the RAM space.
 *    
 * Description:
 *   This function is normally used for verifying the contents of the RAM 
 *   after microcode download.  
 *      
 * Returns:
 *   BERR_Code
 *        
 ***************************************************************************/
BERR_Code BTHD_7550_P_ReadMemory(BTHD_Handle h, uint16_t addr, uint8_t *buf, uint16_t n);
    
/***************************************************************************
 * Summary:
 *   Writes to the AP RAM space.
 *      
 * Description:
 *   This function is normally used for downloading the microcode.  This function
 *   assumes that the AP is already in idle or reset state.
 *       
 * Returns:
 *   BERR_Code
 *         
 ***************************************************************************/
BERR_Code BTHD_7550_P_WriteMemory(BTHD_Handle h, uint16_t addr,  uint8_t *buf, uint16_t n);
    
/***************************************************************************
 * Summary:
 *   Initiates THD acquisition on the BCM7550.
 *      
 * Description:
 *   This function sends the Acquire_Thd HAB command to the AP.
 *       
 * Returns:
 *   BERR_Code
 *        
 ***************************************************************************/
BERR_Code BTHD_7550_P_AcquireThd(BTHD_Handle h);
    
/***************************************************************************
 * Summary:
 *   Turn on power to THD/IFD block
 *     
 * Description:
 *   This function enables clocks to AFE, PLL, THD, and IFD.
 *       
 * Returns:
 *   none
 *        
 ***************************************************************************/
BERR_Code BTHD_7550_P_PowerEnable(BTHD_Handle h, uint32_t uiFlags);
    
/***************************************************************************
 * Summary:
 *   Turn off power to THD/IFD block
 *
 * Description:
 *   This function disable clocks to AFE, PLL, THD, and IFD. 
 * 
 * Returns:
 *   none
 *  
 ***************************************************************************/
BERR_Code BTHD_7550_P_PowerDown(BTHD_Handle h);

/******************************************************************************
Summary:
   Update Aquire parameters
Description:
  
Returns:
   none
******************************************************************************/
BERR_Code BTHD_7550_P_DynamicAcquireParam(BTHD_Handle h);
    
/***************************************************************************
 * Summary:
 *   THD Acquire flag definition
 *      
 * Description:
 *       
 * See Also:
 *        
 ***************************************************************************/
#define AUTO_ACQUIRE (1<<0)
#define INVERT_SPECTRUM (1<<1)
#define GUARD_ACQUIRE (1<<2)
#define TPS_ACQUIRE (1<<3)
#define WIDE_ACQUIRE (1<<4)
#define PARALLEL_OUT (3<<9)
#define SERIAL_OUT (4<<9)
#define AUTO 0
#define THD_STATUS_LEN 85
#define SAMPLE_RATE 54000000
#define CENTER_FREQ 36000000
#define RF_TARGET -44
#define SETUP_AUTO 0
#define SETUP_FIXED 1
#define AUDIO_SIF_MODE 0x10
    
/* BCM7550 implementation of API functions */

BERR_Code BTHD_7550_P_GetThdStatus(BTHD_Handle, BTHD_THDStatus *);
BERR_Code BTHD_7550_P_ResetInbandStatus(BTHD_Handle);
void BTHD_7550_P_ISDBT_Softwareworkaround (BTHD_Handle);
BERR_Code BTHD_7550_P_GetThdLockStatus(BTHD_Handle, bool *);

    
/* Internal THD Functions */
void BTHD_7550_P_SetMode( BTHD_Handle, THD_TransmissionMode_t, THD_GuardInterval_t, THD_Qam_t, THD_CodeRate_t, THD_CodeRate_t, THD_Hierarchy_t);
void BTHD_7550_P_SetOI(BTHD_Handle);
BTHD_RESULT BTHD_7550_P_SetTPS(BTHD_Handle h, THD_TransmissionMode_t, THD_GuardInterval_t);
BTHD_RESULT BTHD_7550_P_SetTMCC(BTHD_Handle h, THD_TransmissionMode_t, THD_GuardInterval_t);
BTHD_RESULT BTHD_7550_P_SetFEC(BTHD_Handle h);
void BTHD_7550_P_SetFrequency(BTHD_Handle);
void BTHD_7550_P_SetFrontEnd(BTHD_Handle);
BTHD_RESULT  BTHD_7550_P_GetNotchISDBT(BTHD_Handle, THD_TransmissionMode_t);
BTHD_RESULT  BTHD_7550_P_GetNotch(BTHD_Handle, THD_TransmissionMode_t);
void BTHD_7550_P_SetNotch(BTHD_Handle);
THD_TransGuardResult_t BTHD_7550_P_SetTransGuard(BTHD_Handle);
void BTHD_7550_P_SetFW( BTHD_Handle, THD_FFTWindowMode_t, THD_TransmissionMode_t, THD_GuardInterval_t);
void BTHD_7550_P_SetCE( BTHD_Handle, THD_ChannelEstimatorMode_t);
BTHD_RESULT BTHD_7550_P_SetICE(BTHD_Handle, THD_TransmissionMode_t, THD_GuardInterval_t);
void BTHD_7550_P_WriteFICoef( BTHD_Handle, THD_FrequencyInterpolatorMode_t);
void BTHD_7550_P_WriteTICoef( BTHD_Handle);
void BTHD_7550_P_WriteLpf1Coef( BTHD_Handle);
void BTHD_7550_P_WriteLpf1CoefAcq( BTHD_Handle);
void BTHD_7550_P_SetEq( BTHD_Handle, THD_CoChannelMode_t);
void BTHD_7550_P_SetViterbi( BTHD_Handle, THD_CodeRate_t, THD_CodeRate_t);
void BTHD_7550_P_SetFrame( BTHD_Handle );
void BTHD_7550_P_Status( BTHD_Handle );
BTHD_RESULT BTHD_7550_P_Lock( BTHD_Handle, int );
BTHD_RESULT BTHD_7550_P_LockOld( BTHD_Handle );
void BTHD_7550_P_ResetAll(BTHD_Handle);
void BTHD_7550_P_ResetAcquire(BTHD_Handle);
BERR_Code BTHD_7550_P_ResetStatus(BTHD_Handle);
void BTHD_7550_P_ResetStatusHW(BTHD_Handle);
void BTHD_7550_P_Config(BTHD_Handle);
BERR_Code BTHD_7550_P_Acquire(BTHD_Handle);
BERR_Code BTHD_7550_P_EnableTimer(void);
BERR_Code BTHD_7550_P_ReacquireThd(BTHD_Handle h);
BERR_Code BTHD_7550_P_ThdTop(BTHD_Handle);
void BTHD_7550_P_SetTs(BTHD_Handle);
void BTHD_7550_P_Init_clk(BTHD_Handle);
BERR_Code BTHD_7550_P_GetSoftDecisionBuf(BTHD_Handle, int16_t *,int16_t *);
    

#ifdef NOOS
extern void Frontend_P_7550_LockEvent_tnr(void);
#endif
#ifdef __cplusplus
}
#endif

#endif /* BTHD_7550_P_PRIV_H__ */

