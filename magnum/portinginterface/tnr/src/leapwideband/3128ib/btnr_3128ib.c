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
#include "btnr_3128ib.h"
#include "btnr_312xib_priv.h"

BDBG_MODULE(btnr_3128ib);

#define DEV_MAGIC_ID                    ((BERR_TNR_ID<<16) | 0xFACE)
#define BTNR_3128Ib_SETTINGS_I2CADDR    (0x66)          /* 7bit addr */
#define CHIP_ID_3123                    0x3123
#define CHIP_ID_3124                    0x3124
#define CHIP_ID_3462                    0x3462
#define CHIP_ID_3127                    0x3127
#define CHIP_ID_3147                    0x3147
#define CHIP_ID_3144                    0x3144
#define CHIP_ID_3145                    0x3145
#define CHIP_ID_3184                    0x3184
#define BTNR_MAX_3462_CHANNELS          (1)
#define BTNR_MAX_3127_CHANNELS          (4)
#define BTNR_MAX_3147_CHANNELS          (4)
#define BTNR_MAX_3184_CHANNELS          (8)
/* Chips with DAC */
#define BTNR_MAX_3145_CHANNELS          (4)
#define BTNR_MAX_3144_CHANNELS          (5)
#define BTNR_MAX_3124_CHANNELS          (5)
#define BTNR_MAX_3123_CHANNELS          (4)

/*******************************************************************************
*
*   Default IF DAC Settings
*
*******************************************************************************/
static const BTNR_IfDacSettings defIfdacSettings =
{
    BTNR_Standard_eQam,
    765000000,
    6000000,
    10000000,
    0
};

/*******************************************************************************/
BERR_Code BTNR_3128Ib_Open(
    BTNR_Handle *phDev,                 /* [output] Returns handle */
    BTNR_3128Ib_Settings *pSettings, /* [Input] settings structure */
    BHAB_Handle hHab                  /* [Input] Hab Handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_312xIb_Handle h312xIbDev;
    BTNR_P_312xIb_Settings *pTnrImplData;
    BTNR_Handle hDev;
    BHAB_Capabilities capabilities={0, NULL};
    uint16_t chipVer;
    uint32_t familyId, chipId;
    uint8_t apVer, minApVer, i=0, totalTunerChannels=0;

    BDBG_ENTER(BTNR_312xIb_Open);
    BDBG_ASSERT( hHab );

    hDev = NULL;
    /* Alloc memory from the system heap */
    h312xIbDev = (BTNR_312xIb_Handle) BKNI_Malloc( sizeof( BTNR_P_312xIb_Handle ) );
    if( h312xIbDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_312xIb_Open: BKNI_malloc() failed"));
        goto done;
    }
    BKNI_Memset( h312xIbDev, 0x00, sizeof( BTNR_P_312xIb_Handle ) );

    hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_312xIb_Open: BKNI_malloc() failed"));
        BKNI_Free( h312xIbDev );
        goto done;
    }
    BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

    h312xIbDev->magicId = DEV_MAGIC_ID;
    pTnrImplData = &h312xIbDev->settings;
    pTnrImplData->rfFreq = 0;
    pTnrImplData->tunerMode = BTNR_TunerMode_eLast;
    h312xIbDev->bPowerdown = true;

    h312xIbDev->hHab = hHab;
    hDev->hDevImpl = (void *) h312xIbDev;
    hDev->magicId = DEV_MAGIC_ID;
    h312xIbDev->devId = BHAB_DevId_eTNR0; /* Here the device id is always defaulted to channel 0. */
    hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_312xIb_SetRfFreq;
    hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_312xIb_GetRfFreq;
    hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_P_312xIb_GetAgcRegVal;
    hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) BTNR_312xIb_SetAgcRegVal;
    hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_312xIb_GetInfo;
    hDev->pClose = (BTNR_CloseFunc) BTNR_312xIb_Close;
    hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc) BTNR_312xIb_GetPowerSaver;
    hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc) BTNR_312xIb_SetPowerSaver;
    hDev->pGetSettings = (BTNR_GetSettingsFunc) BTNR_312xIb_GetSettings;
    hDev->pSetSettings = (BTNR_SetSettingsFunc) BTNR_312xIb_SetSettings;
    hDev->pRequestSpectrumAnalyzerData = (BTNR_RequestSpectrumAnalyzerDataFunc) BTNR_312xIb_RequestSpectrumAnalyzerData;
    hDev->pGetSpectrumAnalyzerData = (BTNR_GetSpectrumAnalyzerDataFunc) BTNR_312xIb_GetSpectrumAnalyzerData;
    hDev->pInstallCallback = (BTNR_InstallCallbackFunc) BTNR_312xIb_InstallCallback;
    hDev->pVersionInfo = (BTNR_GetVersionInfoFunc) BTNR_312xIb_GetVersionInfo;
    hDev->pTuneIfDac = (BTNR_TuneIfDacFunc) BTNR_312xIb_TuneIfDac;
    hDev->pResetIfDacStatus = (BTNR_ResetIfDacStatusFunc) BTNR_312xIb_ResetIfDacStatus;
    hDev->pRequestIfDacStatus = (BTNR_RequestIfDacStatusFunc) BTNR_312xIb_RequestIfDacStatus;
    hDev->pGetIfDacStatus = (BTNR_GetIfDacStatusFunc) BTNR_312xIb_GetIfDacStatus;

    retCode = BHAB_GetApVersion(h312xIbDev->hHab, &familyId, &chipId, &chipVer, &apVer, &minApVer);
    if((chipId == 0x00) && (familyId == 0x3128))
        chipId = 0x3128;

    retCode =  BHAB_GetTunerChannels(h312xIbDev->hHab, &capabilities.totalTunerChannels);
    if(retCode){retCode = BERR_TRACE(retCode); goto done;}

    capabilities.channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc(capabilities.totalTunerChannels*sizeof(BHAB_ChannelCapability));
    if(!capabilities.channelCapabilities){
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;
    }

    retCode =  BHAB_GetCapabilities(h312xIbDev->hHab, &capabilities);
    if(retCode){retCode = BERR_TRACE(retCode); goto done;}
    for(i=0; i<capabilities.totalTunerChannels; i++)
    {
        if((capabilities.channelCapabilities[i].demodCoreType.ads) | (capabilities.channelCapabilities[i].demodCoreType.ifdac))
            totalTunerChannels++;
    }
    h312xIbDev->mxChnNo = totalTunerChannels;
    h312xIbDev->channelNo = capabilities.channelCapabilities[pSettings->channelNo].tunerChannelNumber;

    if(pSettings->channelNo > h312xIbDev->mxChnNo)
    {
        retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        BDBG_ERR(("Only %d TNR channels supported on on %x", h312xIbDev->mxChnNo, chipId));
    }
    else
        *phDev = hDev;

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
    BDBG_LEAVE(BTNR_312xIb_Open);
    return( retCode );
}

BERR_Code BTNR_3128Ib_GetDefaultSettings(
    BTNR_3128Ib_Settings *pDefSettings  /* [out] Returns default setting */
    )
{
    BDBG_ASSERT(NULL != pDefSettings);

    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));
    pDefSettings->i2cAddr = BTNR_3128Ib_SETTINGS_I2CADDR;

    return BERR_SUCCESS;
}

BERR_Code BTNR_312xIb_GetDefaultIfDacSettings(
    BTNR_IfDacSettings *pDefIfdacSettings  /* [out] Returns default setting */
    )
{
    BDBG_ASSERT(NULL != pDefIfdacSettings);

    BKNI_Memset(pDefIfdacSettings, 0, sizeof(*pDefIfdacSettings));
    *pDefIfdacSettings = defIfdacSettings;

    return BERR_SUCCESS;
}
