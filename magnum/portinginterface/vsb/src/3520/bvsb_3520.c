/***************************************************************************
 *     Copyright (c) 2004-2012, Broadcom Corporation
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
#include "bchp_3520.h"
#include "bvsb.h"
#include "bvsb_priv.h"
#include "bvsb_3520.h"
#include "bvsb_3520_priv.h"


BDBG_MODULE(bvsb_3520);


static const BVSB_Settings defDevSettings =
{
    {  /* i2c settings */
        0x0F, /* chipAddr */
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
        false,  /* bRfiSweep */
        0 /* PGA Gain, not used */
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
        0 /* PGA Gain, not used */
    },
    {  /* NTSC acquisition settings */
        BVSB_PhaseLoopBw_eMedium, 
        true,  /* bFastAcq */
        true,  /* bTerr */
        true,  /* bAutoAcq */
        BVSB_PullInRange_eWide, /* wide pull-in range*/
        45750000, /* not used in 3520 */
        BVSB_AnalogMode_eNTSC, /* not used in 3520 */
        BVSB_BtscCountry_eUS, /* not used in 3520 */
        0 /* PGA Gain, not used */
    },
    {  /* BTSC settings */
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
    { /* placehoder for channel scan parameters, N/A for 3520 */
        0,
        0,
        0
    },
    { /* API function table */
        BVSB_3520_P_Open,
        BVSB_3520_P_Close,
        BVSB_3520_P_InitAp,
        BVSB_3520_P_GetApStatus,
        BVSB_3520_P_GetApVersion,
        BVSB_3520_P_ReadRegister,
        BVSB_3520_P_WriteRegister,
        BVSB_3520_P_Mi2cWrite,
        BVSB_3520_P_Mi2cRead,
        BVSB_3520_P_AcquireInband,
        BVSB_3520_P_GetVsbStatus,
        BVSB_3520_P_GetQamStatus,
        BVSB_3520_P_GetNtscStatus,
        BVSB_3520_P_ResetInbandStatus,
        BVSB_3520_P_AcquireOob,
        BVSB_3520_P_GetOobStatus,
        BVSB_3520_P_ResetOobStatus,
        BVSB_3520_P_ConfigBtsc,
        BVSB_3520_P_SetBtscVolume,
        BVSB_3520_P_GetBtscStatus,
        BVSB_3520_P_SetInbandOi,
        BVSB_3520_P_GetSoftDecisionBuf,
        BVSB_3520_P_GetOobSoftDecisionBuf,
        BVSB_3520_P_SetTmConfig,
        BVSB_3520_P_GetTmConfig,
        BVSB_3520_P_WriteConfig,
        BVSB_3520_P_ReadConfig,
        BVSB_3520_P_GetLockStateChangeEventHandle,
        BVSB_3520_P_GetOobLockStateChangeEventHandle,
        BVSB_3520_P_GetAntennaEventHandle,
        BVSB_3520_P_GetInterruptEventHandle,
        BVSB_3520_P_HandleInterrupt_isr,
        BVSB_3520_P_ProcessInterruptEvent,
        BVSB_3520_P_GetChipRevision,
        NULL, /* DetectChannelSignal, */
        NULL, /* GetUnlockstatusEventHandle */
        NULL, /* GetLockstatusEventHandle*/
        BVSB_3520_P_SetInbandIfFreq, /* SetInbandIfFreq*/
        NULL,  /*SetIfdPullInRange*/
        NULL,  /*PowerDown*/
        NULL,  /*SetPgaGain*/
        BVSB_3520_P_SetOobInterfaceControl 
    },
    NULL, /* Timer */
    false, /* bRfDeltaSigmaInvert */
    false, /* bIfDeltaSigmaInvert */
    BVSB_AgcPinMode_e33V,
    false, /* bIfAgcZero */
    false, /* bRfAgcZero */
    false, /* bRfDeltaSigmaOpenDrain */
    false, /* bIfDeltaSigmaOpenDrain */
    false /* bOpenDrainPinsOnClose */
};


/******************************************************************************
  BVSB_3520_GetDefaultSettings()
 ******************************************************************************/
BERR_Code BVSB_3520_GetDefaultSettings(
        BVSB_Settings *pDefSettings /* [out] default settings */
        )
{
    *pDefSettings = defDevSettings;
    return BERR_SUCCESS;
}


