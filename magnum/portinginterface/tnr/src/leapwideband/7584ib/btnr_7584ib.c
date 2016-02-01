/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "btnr.h"
#include "btnr_priv.h"
#include "btnr_7584ib.h"
#include "btnr_7584ib_priv.h"

BDBG_MODULE(btnr_7584ib);

#define DEV_MAGIC_ID                    ((BERR_TNR_ID<<16) | 0xFACE)
#define BTNR_7584Ib_SETTINGS_I2CADDR    (0x6C)          /* 7bit addr */
#define BTNR_MAX_7584_CHANNELS          5
#define BTNR_MAX_7583_CHANNELS          4


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
BERR_Code BTNR_7584Ib_Open(
    BTNR_Handle *phDev,                 /* [output] Returns handle */
    BTNR_7584Ib_Settings *pSettings, /* [Input] settings structure */
    BHAB_Handle hHab                  /* [Input] Hab Handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_7584Ib_Handle h7584IbDev;
    BTNR_P_7584Ib_Settings *pTnrImplData;
    BTNR_Handle hDev;
    BHAB_Capabilities capabilities={0, NULL};
    uint16_t chipVer;
    uint32_t familyId, chipId;
    uint8_t apVer, minApVer, i=0, totalTunerChannels=0;

    BDBG_ENTER(BTNR_7584Ib_Open);
    BDBG_ASSERT( hHab );

    hDev = NULL;
    /* Alloc memory from the system heap */
    h7584IbDev = (BTNR_7584Ib_Handle) BKNI_Malloc( sizeof( BTNR_P_7584Ib_Handle ) );
    if( h7584IbDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7584Ib_Open: BKNI_malloc() failed\n"));
        goto done;
    }
    BKNI_Memset( h7584IbDev, 0x00, sizeof( BTNR_P_7584Ib_Handle ) );

    hDev = (BTNR_Handle) BKNI_Malloc( sizeof( BTNR_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BTNR_7584Ib_Open: BKNI_malloc() failed\n"));
        BKNI_Free( h7584IbDev );
        goto done;
    }
    BKNI_Memset( hDev, 0x00, sizeof( BTNR_P_Handle ) );

    h7584IbDev->magicId = DEV_MAGIC_ID;
    pTnrImplData = &h7584IbDev->settings;
    pTnrImplData->rfFreq = 0;
    pTnrImplData->tunerMode = BTNR_TunerMode_eLast;
    h7584IbDev->bPowerdown = true;

    h7584IbDev->hHab = hHab;
    hDev->hDevImpl = (void *) h7584IbDev;
    hDev->magicId = DEV_MAGIC_ID;
    h7584IbDev->devId = BHAB_DevId_eTNR0; /* Here the device id is always defaulted to channel 0. */
    hDev->pSetRfFreq = (BTNR_SetRfFreqFunc) BTNR_7584Ib_SetRfFreq;
    hDev->pGetRfFreq = (BTNR_GetRfFreqFunc) BTNR_7584Ib_GetRfFreq;
    hDev->pGetAgcRegVal = (BTNR_GetAgcRegValFunc) BTNR_P_7584Ib_GetAgcRegVal;
    hDev->pSetAgcRegVal = (BTNR_SetAgcRegValFunc) BTNR_7584Ib_SetAgcRegVal;
    hDev->pGetInfo = (BTNR_GetInfoFunc) BTNR_7584Ib_GetInfo;
    hDev->pClose = (BTNR_CloseFunc) BTNR_7584Ib_Close;
    hDev->pGetPowerSaver = (BTNR_GetPowerSaverFunc) BTNR_7584Ib_GetPowerSaver;
    hDev->pSetPowerSaver = (BTNR_SetPowerSaverFunc) BTNR_7584Ib_SetPowerSaver;
    hDev->pGetSettings = (BTNR_GetSettingsFunc) BTNR_7584Ib_GetSettings;
    hDev->pSetSettings = (BTNR_SetSettingsFunc) BTNR_7584Ib_SetSettings;
    hDev->pRequestSpectrumAnalyzerData = (BTNR_RequestSpectrumAnalyzerDataFunc) BTNR_7584Ib_RequestSpectrumAnalyzerData;
    hDev->pGetSpectrumAnalyzerData = (BTNR_GetSpectrumAnalyzerDataFunc) BTNR_7584Ib_GetSpectrumAnalyzerData;
    hDev->pInstallCallback = (BTNR_InstallCallbackFunc) BTNR_7584Ib_InstallCallback;
    hDev->pVersionInfo = (BTNR_GetVersionInfoFunc) BTNR_7584Ib_GetVersionInfo;
    hDev->pTuneIfDac = (BTNR_TuneIfDacFunc) BTNR_7584Ib_TuneIfDac;
    hDev->pResetIfDacStatus = (BTNR_ResetIfDacStatusFunc) BTNR_7584Ib_ResetIfDacStatus;
    hDev->pRequestIfDacStatus = (BTNR_RequestIfDacStatusFunc) BTNR_7584Ib_RequestIfDacStatus;
    hDev->pGetIfDacStatus = (BTNR_GetIfDacStatusFunc) BTNR_7584Ib_GetIfDacStatus;

    retCode = BHAB_GetApVersion(h7584IbDev->hHab, &familyId, &chipId, &chipVer, &apVer, &minApVer);

    if((chipId == 0x00) && (familyId == 0x7584))
        chipId = 0x75840;

    retCode =  BHAB_GetTunerChannels(h7584IbDev->hHab, &capabilities.totalTunerChannels);
    if(retCode){retCode = BERR_TRACE(retCode); goto done;}

    capabilities.channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc(capabilities.totalTunerChannels*sizeof(BHAB_ChannelCapability));
    if(!capabilities.channelCapabilities){
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;
    }

    retCode =  BHAB_GetCapabilities(h7584IbDev->hHab, &capabilities);
    if(retCode){retCode = BERR_TRACE(retCode); goto done;}
    for(i=0; i<capabilities.totalTunerChannels; i++)
    {
        if((capabilities.channelCapabilities[i].demodCoreType.ads) | (capabilities.channelCapabilities[i].demodCoreType.ifdac))
            totalTunerChannels++;
    }
    h7584IbDev->mxChnNo = totalTunerChannels;
    h7584IbDev->channelNo = capabilities.channelCapabilities[pSettings->channelNo].tunerChannelNumber;

    if(pSettings->channelNo > h7584IbDev->mxChnNo)
    {
        retCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        BDBG_ERR(("Only %d TNR channels supported on %x", h7584IbDev->mxChnNo, chipId));
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
    BDBG_LEAVE(BTNR_7584Ib_Open);
    return( retCode );
}

BERR_Code BTNR_7584Ib_GetDefaultSettings(
    BTNR_7584Ib_Settings *pDefSettings  /* [out] Returns default setting */
    )
{
    BDBG_ASSERT(NULL != pDefSettings);

    BKNI_Memset(pDefSettings, 0, sizeof(*pDefSettings));
    pDefSettings->i2cAddr = BTNR_7584Ib_SETTINGS_I2CADDR;

    return BERR_SUCCESS;
}

BERR_Code BTNR_7584Ib_GetDefaultIfDacSettings(
    BTNR_IfDacSettings *pDefIfdacSettings  /* [out] Returns default setting */
    )
{
    BDBG_ASSERT(NULL != pDefIfdacSettings);

    BKNI_Memset(pDefIfdacSettings, 0, sizeof(*pDefIfdacSettings));
    *pDefIfdacSettings = defIfdacSettings;

    return BERR_SUCCESS;
}
