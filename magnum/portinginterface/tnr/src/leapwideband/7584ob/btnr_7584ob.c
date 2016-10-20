/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
 *****************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_7584ob.h"
#include "btnr_7584ob_priv.h"

BDBG_MODULE(btnr_7584ob);

#define DEV_MAGIC_ID                    ((BERR_TNR_ID<<16) | 0xFACE)
#define BTNR_7584Ob_SETTINGS_I2CADDR    (0x66)          /* 7bit addr */
#define BTNR_7584Ob_IF_FREQ             44000000            /* 7bit addr */

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/

BERR_Code BTNR_7584Ob_Open(
    BTNR_Handle *phDev,                 /* [output] Returns handle */
    BTNR_7584Ob_Settings *pSettings, /* [Input] settings structure */
    BHAB_Handle hHab                  /* [Input] Hab Handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_7584Ob_Handle h7584ObDev;
    BTNR_P_7584Ob_Settings *pTnrImplData;
    BTNR_Handle hDev;
    uint16_t chipVer;
    uint32_t familyId, chipId;
    uint8_t apVer, minApVer, i=0;
    uint8_t hab[9] = HAB_MSG_HDR(BTNR_OOB_RF_INPUT_MODE, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_Capabilities capabilities={0, NULL};
    bool isOobSupported = false;

    BDBG_ENTER(BTNR_7584Ob_Open);
    BDBG_ASSERT( hHab );

    hDev = NULL;
    /* Alloc memory from the system heap */
    h7584ObDev = (BTNR_7584Ob_Handle) BKNI_Malloc( sizeof( BTNR_P_7584Ob_Handle ) );
    if( h7584ObDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7584_Open: BKNI_malloc() failed"));
        goto done;
    }
    BKNI_Memset( h7584ObDev, 0x00, sizeof( BTNR_P_7584Ob_Handle ) );

    hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7584Ob_Open: BKNI_malloc() failed"));
        BKNI_Free( h7584ObDev );
        goto done;
    }
    BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

    h7584ObDev->magicId = DEV_MAGIC_ID;
    h7584ObDev->bPowerdown = true;
    pTnrImplData = &h7584ObDev->settings;
    pTnrImplData->rfFreq = 0;
    pTnrImplData->tunerMode = BTNR_TunerMode_eDigital;
    pTnrImplData->ifFreq = pSettings->ifFreq;

    h7584ObDev->hHab = hHab;
    hDev->hDevImpl = (void *) h7584ObDev;
    hDev->magicId = DEV_MAGIC_ID;
    hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_7584Ob_SetRfFreq;
    hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_7584Ob_GetRfFreq;
    hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_P_7584Ob_GetAgcRegVal;
    hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) BTNR_7584Ob_SetAgcRegVal;
    hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_7584Ob_GetInfo;
    hDev->pClose = (BTNR_CloseFunc) BTNR_7584Ob_Close;
    hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc) BTNR_7584Ob_GetPowerSaver;
    hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc) BTNR_7584Ob_SetPowerSaver;
    hDev->pGetSettings = (BTNR_GetSettingsFunc) BTNR_7584Ob_GetSettings;
    hDev->pSetSettings = (BTNR_SetSettingsFunc) BTNR_7584Ob_SetSettings;
    hDev->pVersionInfo = (BTNR_GetVersionInfoFunc) BTNR_7584Ob_GetVersionInfo;

    hab[4] = pSettings->inputMode;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(h7584ObDev->hHab, hab, 9, hab, 0, false, true, 9));

    CHK_RETCODE(retCode, BHAB_GetApVersion(h7584ObDev->hHab, &familyId, &chipId, &chipVer, &apVer, &minApVer));

    if((chipId == 0x00) && (familyId == 0x7584))
        chipId = 0x75840;

    retCode =  BHAB_GetTunerChannels(h7584ObDev->hHab, &capabilities.totalTunerChannels);
    if(retCode){retCode = BERR_TRACE(retCode); goto done;}

    capabilities.channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc(capabilities.totalTunerChannels*sizeof(BHAB_ChannelCapability));
    if(!capabilities.channelCapabilities){
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;
    }

    retCode =  BHAB_GetCapabilities(h7584ObDev->hHab, &capabilities);
    if(retCode){retCode = BERR_TRACE(retCode); goto done;}
    for(i=0; i<capabilities.totalTunerChannels; i++)
    {
        if(capabilities.channelCapabilities[i].demodCoreType.aob) {
            isOobSupported = true;
            break;
        }
    }
    if(isOobSupported)
        *phDev = hDev;
    else
    {
        retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        BDBG_ERR(("AOB not supported on %x", chipId));
    }

done:
    if(capabilities.channelCapabilities)
        BKNI_Free(capabilities.channelCapabilities);
    capabilities.channelCapabilities = NULL;
    if( retCode != BERR_SUCCESS )
    {
        if( hDev != NULL )
        {
            BKNI_Free( hDev );
        }
        *phDev = NULL;
    }
    BDBG_LEAVE(BTNR_7584Ob_Open);
    return( retCode );
}

BERR_Code BTNR_7584Ob_GetDefaultSettings(
    BTNR_7584Ob_Settings *pDefSettings  /* [out] Returns default setting */
    )
{
    BDBG_ASSERT(NULL != pDefSettings);

    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));
    pDefSettings->i2cAddr = BTNR_7584Ob_SETTINGS_I2CADDR;
    pDefSettings->ifFreq = BTNR_7584Ob_IF_FREQ;

    return BERR_SUCCESS;
}
