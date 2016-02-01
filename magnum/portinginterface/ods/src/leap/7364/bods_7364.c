/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include "bhab.h"
#include "bods.h"
#include "bods_priv.h"
#include "bods_7364.h"
#include "../bods_leap_priv.h"

BDBG_MODULE(bods_7364);

/*******************************************************************************
*
*   Default Module Settings
*
*******************************************************************************/
static const BODS_Settings defDevSettings =
{
    NULL, /* BHAB handle, must be provided by application*/
    {
        BODS_Leap_Open,
        BODS_Leap_Close,
        BODS_Leap_Init,
        BODS_Leap_GetVersion,
        BODS_Leap_GetVersionInfo,
        BODS_Leap_GetTotalChannels,
        BODS_Leap_GetChannelDefaultSettings,
        BODS_Leap_OpenChannel,
        BODS_Leap_CloseChannel,
        BODS_Leap_GetDevice,
        BODS_Leap_GetCapabilities,
        BODS_Leap_GetLockStatus,
        BODS_Leap_GetSoftDecision,
        BODS_Leap_InstallCallback,
        BODS_Leap_GetDefaultAcquireParams,
        BODS_Leap_SetAcquireParams,
        BODS_Leap_GetAcquireParams,
        BODS_Leap_Acquire,
        BODS_Leap_EnablePowerSaver,
        BODS_Leap_DisablePowerSaver,
        BODS_Leap_ResetStatus,
        BODS_Leap_RequestSelectiveAsyncStatus,
        BODS_Leap_GetSelectiveAsyncStatusReadyType,
        BODS_Leap_GetSelectiveAsyncStatus
    }
};

/***************************************************************************
Summary:
    This function returns the default settings for ODS module.

Description:
    This function is responsible for returns the default setting for
    BODS module. The returning default setting should be when
    opening the device.

Returns:
    TODO:

See Also:
    BODS_Leap_Open()

****************************************************************************/
BERR_Code BODS_7364_GetDefaultSettings(
    BODS_Settings *pDefSettings,        /* [out] Returns default setting */
    BCHP_Handle hChip                   /* [in] Chip handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BODS_Leap_GetDefaultSettings);
    BSTD_UNUSED(hChip);

    *pDefSettings = defDevSettings;

    BDBG_LEAVE(BODS_Leap_GetDefaultSettings);
    return( retCode );
}
