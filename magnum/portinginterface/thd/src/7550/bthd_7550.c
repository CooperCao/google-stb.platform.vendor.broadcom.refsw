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
#include "bstd.h"
#include "bthd.h"
#include "bthd.h"
#include "bthd_7550.h"
#include "bthd_7550_priv.h"
#include "bthd_7550_platform.h"

BDBG_MODULE(bthd_7550);

static const BTHD_Settings defDevSettings =
{
  {  /* i2c settings */
    0x00, /* chipAddr */
    NULL, /* interruptEnableFunc */
    NULL  /* interruptEnableFuncParam */
  },
  { /* API function table */
    BTHD_7550_P_Open,
    BTHD_7550_P_Close,
    BTHD_7550_P_Init,
    NULL, /*BTHD_7550_P_ReadRegister,*/
    NULL, /* BTHD_7550_P_WriteRegister, */
    NULL, /* BTHD_7550_P_Mi2cWrite, */
    NULL, /* BTHD_7550_P_Mi2cRead, */
    NULL, /* BTHD_SetAcquireParams */
    NULL, /* BTHD_GetAcquireParams */    
    BTHD_7550_P_TuneAcquire,
    BTHD_7550_P_GetThdStatus,
    BTHD_7550_P_ResetStatus, /*BTHD_7550_P_ResetInbandStatus,*/
    NULL, /*BTHD_7550_P_GetChipRevision,*/
    NULL, /*BTHD_7550_P_GetVersion,*/
    BTHD_7550_P_GetInterruptEventHandle,
    NULL, /* BTHD_7550_P_HandleInterrupt_isr */
    BTHD_7550_P_ProcessInterruptEvent,
    BTHD_7550_P_GetLockStateChangeEvent,
    NULL, /* BTHD_7550_P_AcquireIfd,*/
    NULL, /* BTHD_7550_P_GetIfdStatus,*/
    BTHD_7550_P_GetSoftDecisionBuf,
    BTHD_7550_GetDefaultInbandParams,
    NULL, /* BTHD_7550_GetDefaultIfdParams, */
    NULL, /* BTHD_7550_P_ResetIfdStatus,*/
    NULL, /* BTHD_7550_P_SetIfdAudioSettings,*/
    NULL, /* BTHD_7550_P_GetIfdAudioSettings,*/
    NULL, /* BTHD_PowerUp */    
	BTHD_7550_P_PowerDown,
	BTHD_7550_P_GetEWSEventHandle,
	BTHD_7550_P_GetThdLockStatus,
	NULL, /* GetBBSInterruptEventHandle */
	NULL, /* ProcessBBSInterruptEvent */
    NULL, /* BTHD_RequestThdAsyncStatus */
    NULL, /* BTHD_GetThdAsyncStatus */
    NULL /* BTHD_InstallCallback */
    },
    NULL, /* BTMR_Handle */
    false, /* RF Delta Sigma Invert */
    false,  /* IF Delta Sigma Invert */
    0,
	NULL,  /* BMEM_Heap_Handle */
	false, /* IF AGC zero */
    NULL,
	false /* isdbt support */    
};


static const BTHD_InbandParams defInbandParams = 
{
  BTHD_InbandMode_eDvbt,         /* Inband mode */
  BTHD_Bandwidth_8Mhz,            /* Bandwidth */
  666000000,                      /* Tuner Frequency */
  36000000,                       /* IF Frequency */
  BTHD_CCI_Auto,                  /* Co-channel filter options */
  BTHD_PullInRange_eWide,         /* Pull-in range */
  true,                           /* Mode/Guard auto acquire */
  BTHD_TransmissionMode_e2K,      /* Manual Transmission mode (FFT Size) */
  BTHD_GuardInterval_e1_4,        /* Manual guard interval selection */
  BTHD_Decode_Hp,                 /* Decode mode */
  false,                          /* TPS Acquire */
  BTHD_Modulation_e64Qam,         /* Modulation type */
  BTHD_CodeRate_e1_2,             /* Manual coderate LP */
  BTHD_CodeRate_e1_2,             /* Manual coderate HP */
  BTHD_Hierarchy_0,               /* Hierarchy selection */
  true,							  /* ISDB-T TMCC acquire */
  false,						  /* ISDB-T partial reception */
  BTHD_Modulation_e16Qam,		  /* ISDB-T Layer A modulation type */
  BTHD_CodeRate_e1_2,			  /* ISDB-T Layer A code rate */
  BTHD_IsdbtTimeInterleaving_0X,  /* ISDB-T Layer A time interleaving */
  13,                             /*ISDB-T Layer A number of segments*/
  BTHD_Modulation_e16Qam,         /* ISDB-T Layer B modulation type */
  BTHD_CodeRate_e1_2,             /* ISDB-T Layer B code rate */
  BTHD_IsdbtTimeInterleaving_0X,  /* ISDB-T Layer B time interleaving */
  0,                              /*ISDB-T Layer A number of segments*/
  BTHD_Modulation_e16Qam,         /* ISDB-T Layer B modulation type */
  BTHD_CodeRate_e1_2,             /* ISDB-T Layer C code rate */
  BTHD_IsdbtTimeInterleaving_0X,  /* ISDB-T Layer C time interleaving*/
  0,                               /* ISDB-T Layer C number of segments*/
  BTHD_ThdAcquisitionMode_eAuto    /* Auto Acquire */
};




/***************************************************************************
 * BTHD_7550_P_GetDefaultInbandParams
 ***************************************************************************/
BERR_Code BTHD_7550_GetDefaultInbandParams(BTHD_InbandParams *pDefSettings)
{
  *pDefSettings = defInbandParams; 
  return BERR_SUCCESS;
}

/***************************************************************************
 * BTHD_7550_GetDefaultSettings()
 ***************************************************************************/
BERR_Code BTHD_7550_GetDefaultSettings(BTHD_Settings *pDefSettings)
{
  *pDefSettings = defDevSettings;
  return BERR_SUCCESS;
}


