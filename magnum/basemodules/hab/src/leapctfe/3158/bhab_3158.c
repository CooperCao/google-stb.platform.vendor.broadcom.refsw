/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bhab.h"
#include "bhab_priv.h"
#include "bhab_3158.h"
#include "bhab_leap_priv.h"
#include "bchp_3158_leap_host_l1.h"

BDBG_MODULE(bhab_3158);

static const BHAB_Settings defDevSettings =
{
    0x6c, /* chipAddr */
    NULL, /* interruptEnableFunc */
    NULL, /* interruptEnableFuncParam */
    /* API function table */
    {
        BHAB_Leap_Open,
        BHAB_Leap_Close,
        BHAB_Leap_InitAp,
        BHAB_Leap_GetApStatus,
        BHAB_Leap_GetApVersion,
        BHAB_Leap_GetVersionInfo,
        BHAB_Leap_ReadRegister,
        BHAB_Leap_WriteRegister,
        BHAB_Leap_ReadMemory,
        BHAB_Leap_WriteMemory,
        NULL,
        NULL,
        BHAB_Leap_HandleInterrupt_isr,
        BHAB_Leap_ProcessInterruptEvent,
        BHAB_Leap_EnableLockInterrupt,
        BHAB_Leap_InstallInterruptCallback,
        BHAB_Leap_UnInstallInterruptCallback,
        BHAB_Leap_SendHabCommand,
        BHAB_Leap_GetInterruptEventHandle,
        NULL,
        NULL,
        NULL,
        NULL,
        BHAB_Leap_GetConfigSettings,
        BHAB_Leap_SetConfigSettings,
        NULL,
        NULL,
        BHAB_Leap_GetInternalGain,
        BHAB_Leap_GetExternalGain,
        BHAB_Leap_SetExternalGain,
        BHAB_Leap_GetAvsData,
        BHAB_Leap_GetTunerChannels,
        BHAB_Leap_GetCapabilities,
        BHAB_Leap_Reset,
        BHAB_Leap_GetRecalibrateSettings,
        BHAB_Leap_SetRecalibrateSettings,
        BHAB_Leap_GetLnaStatus
    },
    0x6c, /* slaveChipAddr */
    false, /* isSpi */
    false, /* isMtsif */
    NULL, /* pImgInterface */
    NULL, /* pImgContext */
    NULL  /* pChp */
};

static const BHAB_RecalibrateSettings defRecalibrateSettings =
{
    {
        true,
        260,
        50
    }
};

/******************************************************************************
 BHAB_3158_GetDefaultSettings()
******************************************************************************/
BERR_Code BHAB_3158_GetDefaultSettings(
	BHAB_Settings *pDefSettings /* [out] default settings */
)
{
	*pDefSettings = defDevSettings;
	return BERR_SUCCESS;
}

/******************************************************************************
 BHAB_3158_GetDefaultRecalibrateSettings()
******************************************************************************/
BERR_Code BHAB_3158_GetDefaultRecalibrateSettings(
    BHAB_RecalibrateSettings *pRecalibrateSettings /* [out] default recalibrate settings */
)
{
	*pRecalibrateSettings = defRecalibrateSettings;
	return BERR_SUCCESS;
}


/***************************************************************************
BHAB_3158_P_GetStandbySettings()
***************************************************************************/
BERR_Code BHAB_3158_P_GetStandbySettings(
    BHAB_Handle handle,                         /* [in] Device handle */
    BHAB_3158_StandbySettings *pSettings        /* [out] Standby Settings */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_Leap_P_Handle *pLeap;
    BDBG_ASSERT(handle);

    pLeap = (BHAB_Leap_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(pLeap);

    *pSettings = pLeap->standbySettings;
    return retCode;
}

/***************************************************************************
BHAB_3158_P_SetStandbySettings()
***************************************************************************/
BERR_Code BHAB_3158_P_SetStandbySettings(
    BHAB_Handle handle,                         /* [in] Device handle */
    const BHAB_3158_StandbySettings *pSettings  /* [in] Standby Settings */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[60] = HAB_MSG_HDR(BHAB_SET_STANDYBY_MODE, 56, BHAB_GLOBAL_CORE_TYPE, BHAB_CORE_ID);
    uint32_t value;
    BHAB_Leap_P_Handle *pLeap;
    BDBG_ASSERT(handle);

    pLeap = (BHAB_Leap_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(pLeap);

    switch(pSettings->mode) {
        case BHAB_3158_StandbyMode_eOn:
            value = BCHP_LEAP_HOST_L1_INTR_W0_STATUS_PERIPH_DEMOD_XPT_WAKEUP_INTR_MASK;
            BHAB_CHK_RETCODE(BHAB_Leap_WriteRegister(handle, BCHP_LEAP_HOST_L1_INTR_W0_MASK_SET, &value));
            break;
        case BHAB_3158_StandbyMode_eDeepSleep:
            value = BCHP_LEAP_HOST_L1_INTR_W0_STATUS_PERIPH_DEMOD_XPT_WAKEUP_INTR_MASK;
            BHAB_CHK_RETCODE(BHAB_Leap_WriteRegister(handle, BCHP_LEAP_HOST_L1_INTR_W0_MASK_CLEAR, &value));
            break;
        default:
            break;
    }
    buf[16]=pSettings->mode;
    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 60, pLeap->inBuf, 0, false, true, 0));
    pLeap->standbySettings = *pSettings;

done:
    return retCode;
}
