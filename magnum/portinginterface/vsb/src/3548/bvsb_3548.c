/***************************************************************************
 *     Copyright (c) 2007-2010, Broadcom Corporation
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
#include "bvsb_3548.h"
#include "bvsb_3548_priv.h"

BDBG_MODULE(bvsb_3548);

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
      false, /* bRfiSweep */
      BVSB_PgaGain_e6dB
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
      false,  /* select DVB unmapper */
      BVSB_PgaGain_e6dB
   },
   {  /* NTSC acquisition settings */
      BVSB_PhaseLoopBw_eMedium, 
      true,  /* bFastAcq */
      true,  /* bTerr */
      true,  /* bAutoAcq */
	  BVSB_PullInRange_eWide,
	  45750000, /* will need this when we integrate 3563 changes */
	  BVSB_AnalogMode_eNTSC,
      BVSB_BtscCountry_eUS,
      BVSB_PgaGain_e6dB
#ifdef NOT_USED
      100, /* delay */
      0xA, /* RF Bandwidth */
      0x2, /* IF Bandwidth */
      0x5A80, /* TOP Bandwidth */
      false /* bSpectrumInvert */
#endif
   },
   {  /* BTSC settings - not used in 3548 BVSB module */
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
      BVSB_P_UNCHANGED_THRESHOLD,
      BVSB_P_UNCHANGED_THRESHOLD,
      BVSB_P_UNCHANGED_THRESHOLD
   },
   { /* API function table */
      BVSB_3548_P_Open,
      BVSB_3548_P_Close,
      BVSB_3548_P_InitAp,
      BVSB_3548_P_GetApStatus,
      BVSB_3548_P_GetApVersion,
      BVSB_3548_P_ReadRegister,
      BVSB_3548_P_WriteRegister,
      NULL,
      NULL,
      BVSB_3548_P_AcquireInband,
      BVSB_3548_P_GetVsbStatus,
      BVSB_3548_P_GetQamStatus,
      BVSB_3548_P_GetNtscStatus,
      BVSB_3548_P_ResetInbandStatus,
      NULL, /*BVSB_3548_P_AcquireOob*/
      NULL, /*BVSB_3548_P_GetOobStatus, */
      NULL, /*BVSB_3548_P_ResetOobStatus,*/
      NULL,
      NULL,
      BVSB_3548_P_GetBtscStatus,
      BVSB_3548_P_SetInbandOi,
      BVSB_3548_P_GetSoftDecisionBuf,
      NULL, /*BVSB_3548_P_GetOobSoftDecisionBuf,*/
      NULL, /*BVSB_3548_P_SetTmConfig */
      NULL, /*BVSB_3548_P_GetTmConfig, */
      BVSB_3548_P_WriteConfig,
      BVSB_3548_P_ReadConfig,
      BVSB_3548_P_GetLockStateChangeEventHandle,
      NULL, /* GetOobLockStateChangeEventHandle */
      NULL,
      NULL,
      NULL, /* BVSB_3548_P_HandleInterrupt_isr,*/
      NULL,
      NULL,
      BVSB_3548_P_DetectChannelSignal,
      BVSB_3548_P_GetUnlockstatusEventHandle,
      BVSB_3548_P_GetLockstatusEventHandle,
      BVSB_3548_P_SetInbandIfFreq,
      BVSB_3548_P_SetIfdPullInRange,
      BVSB_3548_P_PowerDown,
      BVSB_3548_P_SetPgaGain,
      NULL /* BVSB_SetOobInterfaceControl */
#ifdef NEW_BVSB_SETTINGS
      ,
      BVSB_3548_P_SetQamSettings,
	  BVSB_3548_P_SetIfdPixCarrier
#endif
   },
   NULL, /* BTMR_Handle */
   false, /* bRfDeltaSigmaInvert */
   false, /* bIfDeltaSigmaInvert */
   BVSB_AgcPinMode_e25V,
   false, /* bIfAgcZero */
   false, /* bRfAgcZero */
   false, /* bRfDeltaSigmaOpenDrain */
   false, /* bIfDeltaSigmaOpenDrain */
   false /* bOpenDrainPinsOnClose */
};


/******************************************************************************
 BVSB_3548_GetDefaultSettings()
******************************************************************************/
BERR_Code BVSB_3548_GetDefaultSettings(
   BVSB_Settings *pDefSettings /* [out] default settings */
)
{
   *pDefSettings = defDevSettings;
   return BERR_SUCCESS;
}


