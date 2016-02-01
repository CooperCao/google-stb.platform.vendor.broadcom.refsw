/***************************************************************************
 *     Copyright (c) 2003-2007, Broadcom Corporation
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
 ***************************************************************************/

#include "bstd.h"
#include "bvsb.h"
#include "bvsb_priv.h"
#include "bvsb_3563.h"
#include "bvsb_3563_priv.h"


BDBG_MODULE(bvsb_3563);


static const BVSB_Settings defDevSettings =
{
   {  /* i2c settings */
      0x00, /* chipAddr */
      NULL, /* interruptEnableFunc */
      NULL  /* interruptEnableFuncParam */
   },
   {  /* VSB acquisition settings */
      BVSB_PhaseLoopBw_eMedium,  /* BW */
      true,  /* bAutoAcq */
      true,  /* bFastAcq */
      true,  /* bTei */
      true,  /* bTerr */
      false, /* bNtscSweep */
      false  /* bRfiSweep */
   },
   {  /* QAM acquisition settings */
      BVSB_PhaseLoopBw_eMedium, 
      BVSB_IDepth_e12_17,  /* idepth */
      BVSB_NyquistFilter_e12, /* nyquist filter rolloff */
      true,  /* bAutoAcq */
      true,  /* bFastAcq */
      true,  /* bTerr */
      true,  /* bEq */
      false, /* use 6MHz channelization */
      true,  /* bTei */
      true,  /* bSpinv */
      false  /* select DVB unmapper */
   },
   {  /* NTSC acquisition settings */
      BVSB_PhaseLoopBw_eMedium, 
      true,  /* bFastAcq */
      true,  /* bTerr */
      true,  /* bAutoAcq */
	  false, /* bPalInput */
	  BVSB_PullInRange_eWide,	
	  45750000,
	  BVSB_PalMode_ePalD
   },
   {  /* BTSC settings - not used in 3563 BVSB module */
      BVSB_BtscDecodeMode_eStereo,  /* decode mode */
      BVSB_BtscSampleRate_e48KHz,   /* sample rate */
      true                          /* primary i2s output */
   },
   {  /* OOB acquisition settings */
      BVSB_PhaseLoopBw_eMedium, 
      true,  /* bAutoAcq */
      true,  /* bSpinv */
      true  /* bBypassFEC */ 
   },
   {  /* Inband Transport output interface settings */
      false, /* bHead4 */
      false,  /* bSync1 */
      false, /* bXBERT */
      false, /* bErrinv */
      false, /* bSyncinv */
      false, /* bVldinv */
      false, /* bClksup */
      true,  /* bClkinv */
      false   /* bSerial */
   },
   {  /* set threshold for signal detection */
      BVSB_P_UNCHANGED_THRESHOLD,       /* QAM param 4 */
      BVSB_P_UNCHANGED_THRESHOLD,       /* AGF */
      BVSB_P_UNCHANGED_THRESHOLD        /* AGC */
   },
   { /* API function table */
      BVSB_3563_P_Open,
      BVSB_3563_P_Close,
      BVSB_3563_P_InitAp,
      BVSB_3563_P_GetApStatus,
      BVSB_3563_P_GetApVersion,
      BVSB_3563_P_ReadRegister,
      BVSB_3563_P_WriteRegister,
      NULL,
      NULL,
      BVSB_3563_P_AcquireInband,
      BVSB_3563_P_GetVsbStatus,
      BVSB_3563_P_GetQamStatus,
      BVSB_3563_P_GetNtscStatus,
      BVSB_3563_P_ResetInbandStatus,
      BVSB_3563_P_AcquireOob,
      BVSB_3563_P_GetOobStatus,
      BVSB_3563_P_ResetOobStatus,
      NULL,
      NULL,
      BVSB_3563_P_GetBtscStatus,
      BVSB_3563_P_SetInbandOi,
      BVSB_3563_P_GetSoftDecisionBuf,
      BVSB_3563_P_GetOobSoftDecisionBuf,
      BVSB_3563_P_SetTmConfig,
      BVSB_3563_P_GetTmConfig,
      BVSB_3563_P_WriteConfig,
      BVSB_3563_P_ReadConfig,
      BVSB_3563_P_GetLockStateChangeEventHandle,
      NULL,
      NULL,
      BVSB_3563_P_HandleInterrupt_isr,
      NULL,
      NULL,
      BVSB_3563_P_DetectChannelSignal,
      BVSB_3563_P_GetUnlockstatusEventHandle,
      BVSB_3563_P_GetLockstatusEventHandle,
      BVSB_3563_P_SetInbandIfFreq,
      BVSB_3563_P_SetIfdPullInRange,
      BVSB_3563_P_SetQamSettings,
	  BVSB_3563_P_SetIfdPixCarrier
   }
};


/******************************************************************************
 BVSB_3563_GetDefaultSettings()
******************************************************************************/
BERR_Code BVSB_3563_GetDefaultSettings(
   BVSB_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


