/***************************************************************************
 *     Copyright (c) 2003-2005, Broadcom Corporation
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
#include "bvsb.h"
#include "bvsb_priv.h"
#include "bvsb_3560.h"
#include "bvsb_3560_priv.h"


BDBG_MODULE(bvsb_3560);


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
   },
   {  /* BTSC settings - not used in 3560 BVSB module */
      BVSB_BtscDecodeMode_eStereo,  /* decode mode */
      BVSB_BtscSampleRate_e48KHz,   /* sample rate */
      true                          /* primary i2s output */
   },
   {  /* OOB acquisition settings */
      BVSB_PhaseLoopBw_eMedium, 
      true,  /* bAutoAcq */
      true,  /* bSpinv */
      false  /* bBypassFEC */
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
      true   /* bSerial */
   },
   { /* API function table */
      BVSB_3560_P_Open,
      BVSB_3560_P_Close,
      BVSB_3560_P_InitAp,
      BVSB_3560_P_GetApStatus,
      BVSB_3560_P_GetApVersion,
      BVSB_3560_P_ReadRegister,
      BVSB_3560_P_WriteRegister,
      NULL,
      NULL,
      BVSB_3560_P_AcquireInband,
      BVSB_3560_P_GetVsbStatus,
      BVSB_3560_P_GetQamStatus,
      BVSB_3560_P_GetNtscStatus,
      BVSB_3560_P_ResetInbandStatus,
      BVSB_3560_P_AcquireOob,
      BVSB_3560_P_GetOobStatus,
      BVSB_3560_P_ResetOobStatus,
      NULL,
      NULL,
      NULL,
      BVSB_3560_P_SetInbandOi,
      BVSB_3560_P_GetSoftDecisionBuf,
      BVSB_3560_P_GetOobSoftDecisionBuf,
      NULL,
      NULL,
      BVSB_3560_P_WriteConfig,
      BVSB_3560_P_ReadConfig,
      BVSB_3560_P_GetLockStateChangeEventHandle,
      NULL,
      NULL,
      BVSB_3560_P_HandleInterrupt_isr,
      NULL,
      BVSB_3560_P_GetSignalChangeEventHandle,
      BVSB_3560_P_GetVideoKillEventHandle
   }
};


/******************************************************************************
 BVSB_3560_GetDefaultSettings()
******************************************************************************/
BERR_Code BVSB_3560_GetDefaultSettings(
   BVSB_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


