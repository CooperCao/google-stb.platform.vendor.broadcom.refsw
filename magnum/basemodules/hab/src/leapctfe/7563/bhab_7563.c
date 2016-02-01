/***************************************************************************
 *     (c)2003-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#include "bhab.h"
#include "bhab_priv.h"
#include "bhab_7563.h"
#include "bhab_7563_priv.h"

BDBG_MODULE(bhab_7563);


static const BHAB_Settings defDevSettings =
{
    0x6c, /* chipAddr */
    NULL, /* interruptEnableFunc */
    NULL, /* interruptEnableFuncParam */
    /* API function table */
    {
        BHAB_7563_Open,
        BHAB_7563_Close,
        BHAB_7563_InitAp,
        BHAB_7563_GetApStatus,
        BHAB_7563_GetApVersion,
        BHAB_7563_GetVersionInfo,
        BHAB_7563_ReadRegister,
        BHAB_7563_WriteRegister,
        BHAB_7563_ReadMemory,
        BHAB_7563_WriteMemory,
        NULL,
        NULL,
        BHAB_7563_HandleInterrupt_isr,
        BHAB_7563_ProcessInterruptEvent,
        BHAB_7563_EnableLockInterrupt,
        BHAB_7563_InstallInterruptCallback,
        BHAB_7563_UnInstallInterruptCallback,
        BHAB_7563_SendHabCommand,
        BHAB_7563_GetInterruptEventHandle,
        NULL,
        NULL,
        NULL,
        NULL,
        BHAB_7563_GetConfigSettings,
        BHAB_7563_SetConfigSettings,
        NULL,
        NULL,
        BHAB_7563_GetInternalGain,
        BHAB_7563_GetExternalGain,
        BHAB_7563_SetExternalGain,
        NULL, /* BHAB_GetAvsData */
        BHAB_7563_GetTunerChannels,
        BHAB_7563_GetCapabilities,
        BHAB_7563_Reset,
        NULL, /* BHAB_GetRecalibrateSettings */
        NULL, /* BHAB_SetRecalibrateSettings */
        NULL /* BHAB_GetLnaStatus */
    },
    0x6c, /* slaveChipAddr */
    false, /* isSpi */
    false, /* isMtsif */
    NULL, /* pImgInterface */
    NULL, /* pImgContext */
    NULL  /* pChp */
};


/******************************************************************************
 BHAB_7563_GetDefaultSettings()
******************************************************************************/
BERR_Code BHAB_7563_GetDefaultSettings(
	BHAB_Settings *pDefSettings /* [out] default settings */
)
{
	*pDefSettings = defDevSettings;
	return BERR_SUCCESS;
}
