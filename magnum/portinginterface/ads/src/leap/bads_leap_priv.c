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
#include "bstd.h"
#include "bads.h"
#include "bhab.h"
#include "bads_priv.h"
#include "bads_leap_priv.h"

BDBG_MODULE(bads_Leap_priv);

static const BADS_InbandParam defInbandParams =
{
    BADS_ModulationType_eAnnexBQam256,
    5360537,
    BADS_InvertSpectrum_eNoInverted,
    BADS_SpectrumMode_eAuto,
    BADS_DpmMode_Disabled,
    true,
    false,
    150000,
    BADS_AcquireType_eAuto,
    false,
    765000000
};

/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/
BERR_Code BADS_Leap_P_EventCallback_isr(
    void * pParam1, int param2
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_ChannelHandle hChn = (BADS_ChannelHandle) pParam1;
    BADS_Leap_ChannelHandle hImplChnDev;
    BHAB_InterruptType event = (BHAB_InterruptType) param2;

    BDBG_ENTER(BADS_Leap_ProcessNotification);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab);

    switch (event) {
        case BHAB_Interrupt_eLockChange:
            {
                if( hImplChnDev->pCallback[BADS_Callback_eLockChange] != NULL )
                {
                    (hImplChnDev->pCallback[BADS_Callback_eLockChange])(hImplChnDev->pCallbackParam[BADS_Callback_eLockChange] );
                }
            }
            break;
        case BHAB_Interrupt_eUpdateGain:
            {
                if( hImplChnDev->pCallback[BADS_Callback_eUpdateGain] != NULL )
                {
                    (hImplChnDev->pCallback[BADS_Callback_eUpdateGain])(hImplChnDev->pCallbackParam[BADS_Callback_eUpdateGain] );
                }
            }
            break;
        case BHAB_Interrupt_eNoSignal:
            {
                if( hImplChnDev->pCallback[BADS_Callback_eNoSignal] != NULL )
                {
                    (hImplChnDev->pCallback[BADS_Callback_eNoSignal])(hImplChnDev->pCallbackParam[BADS_Callback_eNoSignal] );
                }
            }
            break;
        case BHAB_Interrupt_eQamAsyncStatusReady:
            {
                if( hImplChnDev->pCallback[BADS_Callback_eAsyncStatusReady] != NULL )
                {
                    (hImplChnDev->pCallback[BADS_Callback_eAsyncStatusReady])(hImplChnDev->pCallbackParam[BADS_Callback_eAsyncStatusReady] );
                }
            }
            break;
        case BHAB_Interrupt_eSpectrumAnalyzerDataReady:
            {
                if( hImplChnDev->pCallback[BADS_Callback_eSpectrumDataReady] != NULL )
                {
                    (hImplChnDev->pCallback[BADS_Callback_eSpectrumDataReady])(hImplChnDev->pCallbackParam[BADS_Callback_eSpectrumDataReady] );
                }
            }
            break;
#if BADS_CHIP==3158
        case BHAB_Interrupt_eIfDacAcquireComplete:
            {
                if( hImplChnDev->pCallback[BADS_Callback_eIfDacAcquireComplete] != NULL )
                {
                    (hImplChnDev->pCallback[BADS_Callback_eIfDacAcquireComplete])(hImplChnDev->pCallbackParam[BADS_Callback_eIfDacAcquireComplete] );
                }
            }
            break;
        case BHAB_Interrupt_eIfDacStatusReady:
            {
                if( hImplChnDev->pCallback[BADS_Callback_eIfDacStatusReady] != NULL )
                {
                    (hImplChnDev->pCallback[BADS_Callback_eIfDacStatusReady])(hImplChnDev->pCallbackParam[BADS_Callback_eIfDacStatusReady] );
                }
            }
            break;
#endif
        default:
            BDBG_WRN((" unknown event code from leap"));
            break;
    }

    BDBG_LEAVE(BADS_Leap_P_EventCallback_isr);
    return retCode;
}

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BADS_Leap_Open(
    BADS_Handle *pAds,                          /* [out] Returns handle */
    BCHP_Handle hChip,                          /* [in] Chip handle */
    BREG_Handle hRegister,                      /* [in] Register handle */
    BINT_Handle hInterrupt,                     /* [in] Interrupt handle */
    const struct BADS_Settings *pDefSettings    /* [in] Default settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Handle hDev;
    unsigned int chnIdx;
    BADS_Leap_Handle hImplDev = NULL;
    uint16_t chipVer;
    BHAB_Capabilities capabilities={0, NULL};
    uint8_t i=0, maxAdsChannels=0;

    BDBG_ENTER(BADS_Leap_Open);

    /* Alloc memory from the system heap */
    hDev = (BADS_Handle) BKNI_Malloc( sizeof( BADS_P_Handle ) );
    if( hDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BADS_Open: BKNI_malloc() failed"));
        goto done;
    }

    BKNI_Memset( hDev, 0x00, sizeof( BADS_P_Handle ) );
    hDev->magicId = DEV_MAGIC_ID;
    hDev->settings = *pDefSettings;
#if BADS_CHIP==3158
    hDev->pTuneIfDac = (BADS_TuneIfDacFunc) BADS_Leap_TuneIfDac;
    hDev->pResetIfDacStatus = (BADS_ResetIfDacStatusFunc) BADS_Leap_ResetIfDacStatus;
    hDev->pRequestIfDacStatus = (BADS_RequestIfDacStatusFunc) BADS_Leap_RequestIfDacStatus;
    hDev->pGetIfDacStatus = (BADS_GetIfDacStatusFunc) BADS_Leap_GetIfDacStatus;
#endif
    hImplDev = (BADS_Leap_Handle) BKNI_Malloc( sizeof( BADS_P_Leap_Handle ) );
    if( hImplDev == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BADS_Open: BKNI_malloc() failed"));
        goto done;
    }

    BKNI_Memset( hImplDev, 0x00, sizeof( BADS_P_Leap_Handle ) );
    hImplDev->magicId = DEV_MAGIC_ID;
    hImplDev->hChip = hChip;
    hImplDev->hRegister = hRegister;
    hImplDev->hInterrupt = hInterrupt;
    hImplDev->hHab = (BHAB_Handle) pDefSettings->hGeneric;    /* For this device, we need the HAB handle */
    hImplDev->devId = BHAB_DevId_eADS0; /* Here the device id is always defaulted to channel 0. */

    hImplDev->habBuffer = (uint8_t *)BKNI_Malloc(BHAB_BUFFER_SIZE);
    if( hImplDev->habBuffer == NULL )
    {
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BDBG_ERR(("BADS_Open: BKNI_malloc() failed"));
        goto done;
    }
    retCode = BHAB_GetApVersion(hImplDev->hHab, &hImplDev->verInfo.familyId, &hImplDev->verInfo.chipId, &chipVer, &hImplDev->verInfo.apVer, &hImplDev->verInfo.minApVer);
    hImplDev->chipId = hImplDev->verInfo.chipId;
    if((hImplDev->verInfo.chipId == 0x00) && (hImplDev->verInfo.familyId != 0))
        hImplDev->chipId = hImplDev->verInfo.familyId;

    if(hImplDev->chipId == CHIP_ID_3462)
        hImplDev->mxChnNo = MAX_3462_ADS_CHANNELS;
    else if(hImplDev->chipId == CHIP_ID_3461)
        hImplDev->mxChnNo = MAX_3462_ADS_CHANNELS;
    else {
        retCode =  BHAB_GetTunerChannels(hImplDev->hHab, &capabilities.totalTunerChannels);
        if(retCode){retCode = BERR_TRACE(retCode); goto done;}

        capabilities.channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc(capabilities.totalTunerChannels*sizeof(BHAB_ChannelCapability));
        if(!capabilities.channelCapabilities){
            retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;
        }
        BKNI_Memset(capabilities.channelCapabilities, 0, capabilities.totalTunerChannels*sizeof(BHAB_ChannelCapability));

        retCode =  BHAB_GetCapabilities(hImplDev->hHab, &capabilities);
        if(retCode){retCode = BERR_TRACE(retCode); goto done;}
        for(i=0; i<capabilities.totalTunerChannels; i++)
        {
            if(capabilities.channelCapabilities[i].demodCoreType.ads)
                maxAdsChannels++;
#if BADS_CHIP==3158
            if(capabilities.channelCapabilities[i].demodCoreType.ifdac) {
                hImplDev->ifdacChannelNo=i;
                maxAdsChannels++;
                }
#endif
        }
        hImplDev->mxChnNo = maxAdsChannels;
        if (hImplDev->mxChnNo == 0) {
            BDBG_WRN((">>>>> No ADS Channels, defaulting to %d <<<<<", MX_ADS_CHANNELS));
            hImplDev->mxChnNo = MX_ADS_CHANNELS;
        }
    }

    for( chnIdx = 0; chnIdx < MX_ADS_CHANNELS; chnIdx++ )
    {
        hImplDev->hAdsChn[chnIdx] = NULL;
    }
    hDev->pImpl = hImplDev;
    *pAds = hDev;

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
        if( hImplDev->habBuffer != NULL )
        {
            BKNI_Free( hImplDev->habBuffer );
        }
        if( hImplDev != NULL )
        {
            BKNI_Free( hImplDev );
        }
        *pAds = NULL;
    }
    BDBG_LEAVE(BADS_Leap_Open);
    return retCode;
}

BERR_Code BADS_Leap_Close(
    BADS_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_Handle hImplDev;
    BDBG_ENTER(BADS_Leap_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Leap_Handle) hDev->pImpl;

    if( (void *)hImplDev->habBuffer )
        BKNI_Free( (void *)hImplDev->habBuffer );
    if( hDev->pImpl )
        BKNI_Free( (void *) hDev->pImpl );
    hDev->magicId = 0x00;       /* clear it to catch improper use */
    if( hDev )
        BKNI_Free( (void *) hDev );

    BDBG_LEAVE(BADS_Leap_Close);
    return retCode;
}

BERR_Code BADS_Leap_Init(
    BADS_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_Handle hImplDev;
    uint16_t chipVer=0;
    BFEC_SystemVersionInfo  versionInfo;

    BDBG_ENTER(BADS_Leap_Init);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Leap_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );
    BDBG_ASSERT( hImplDev->hHab );

    retCode = BHAB_GetApVersion(hImplDev->hHab, &hImplDev->verInfo.familyId, &hImplDev->verInfo.chipId, &chipVer, &hImplDev->verInfo.apVer, &hImplDev->verInfo.minApVer);

    switch(chipVer >> 8)
    {
        case 0:
            hImplDev->verInfo.majVer = 0xA;
            break;
        case 1:
            hImplDev->verInfo.majVer = 0xB;
            break;
        case 2:
            hImplDev->verInfo.majVer = 0xC;
            break;
        default:
            retCode = BERR_INVALID_PARAMETER;
            goto done;
    }

    hImplDev->verInfo.minVer = chipVer & 0xFF;

    switch(hImplDev->verInfo.familyId)
    {
        case CHIP_ID_3461:
            if((hImplDev->verInfo.apVer >= 6) || ((hImplDev->verInfo.apVer >= 5) & ( hImplDev->verInfo.minApVer>= 3)))
            {
                BHAB_GetVersionInfo(hImplDev->hHab, &versionInfo);
                BDBG_WRN(("LEAP FW version is %d.%d.%d.%d", versionInfo.firmware.majorVersion, versionInfo.firmware.minorVersion, versionInfo.firmware.buildType, versionInfo.firmware.buildId));
            }
            else
                BDBG_WRN(("LEAP FW version is %d.%d", hImplDev->verInfo.apVer, hImplDev->verInfo.minApVer));
            break;
        case CHIP_ID_3128:
            if((hImplDev->verInfo.apVer >= 6) || ((hImplDev->verInfo.apVer >= 5) & ( hImplDev->verInfo.minApVer>= 7)))
            {
                BHAB_GetVersionInfo(hImplDev->hHab, &versionInfo);
                BDBG_WRN(("LEAP FW version is %d.%d.%d.%d", versionInfo.firmware.majorVersion, versionInfo.firmware.minorVersion, versionInfo.firmware.buildType, versionInfo.firmware.buildId));
            }
            else
                BDBG_WRN(("LEAP FW version is %d.%d", hImplDev->verInfo.apVer, hImplDev->verInfo.minApVer));
            break;
        default:
            BHAB_GetVersionInfo(hImplDev->hHab, &versionInfo);
            BDBG_WRN(("LEAP FW version is %d.%d.%d.%d", versionInfo.firmware.majorVersion, versionInfo.firmware.minorVersion, versionInfo.firmware.buildType, versionInfo.firmware.buildId));
            break;
    }

    BDBG_LEAVE(BADS_Leap_Init);
done:
    return retCode;
}

BERR_Code BADS_Leap_GetVersion(
    BADS_Handle hDev,                   /* [in] Device handle */
    BADS_Version *pVersion              /* [out] Returns version */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_Handle hImplDev;

    BDBG_ENTER(BADS_Leap_GetVersion);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Leap_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    *pVersion = hImplDev->verInfo;      /* use saved data */

    BDBG_LEAVE(BADS_Leap_GetVersion);
    return retCode;
}

BERR_Code BADS_Leap_GetVersionInfo(
    BADS_Handle hDev,              /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo /* [out] Returns version Info */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[29] = HAB_MSG_HDR(BADS_eGetVersionInfo, 0, BADS_CORE_TYPE);
    BADS_Leap_Handle hImplDev;

    BDBG_ENTER(BADS_Leap_GetVersionInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Leap_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    buf[3] = 0;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplDev->hHab, buf, 5, buf, 29, false, true, 29 ));
    pVersionInfo->majorVersion = (buf[4] << 8) | buf[5];
    pVersionInfo->minorVersion = (buf[6] << 8) | buf[7];
    pVersionInfo->buildType = (buf[8] << 8) | buf[9];
    pVersionInfo->buildId = (buf[10] << 8) | buf[11];

    BDBG_LEAVE(BADS_Leap_GetVersionInfo);

done:
    return retCode;
}

BERR_Code BADS_Leap_GetTotalChannels(
    BADS_Handle hDev,                   /* [in] Device handle */
    unsigned int *totalChannels         /* [out] Returns total number downstream channels supported */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_Handle hImplDev;

    BDBG_ENTER(BADS_Leap_GetTotalChannels);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Leap_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );

    *totalChannels = hImplDev->mxChnNo; /* use saved data */

    BDBG_LEAVE(BADS_Leap_GetTotalChannels);
    return retCode;
}

BERR_Code BADS_Leap_GetChannelDefaultSettings(
    BADS_Handle hDev,                       /* [in] Device handle */
    unsigned int channelNo,                 /* [in] Channel number to default setting for */
    BADS_ChannelSettings *pChnDefSettings   /* [out] Returns channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_Handle hImplDev;

    BDBG_ENTER(BADS_Leap_GetChannelDefaultSettings);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Leap_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );
    BDBG_ASSERT( hImplDev->hHab );

    if( channelNo < hImplDev->mxChnNo )
    {
        pChnDefSettings->ifFreq = 0;    /*The if freq for leap is set to 0 as the internal tuner does not spit out seperate if frequency. */
        pChnDefSettings->fastAcquire = false;
    }
    else{
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_LEAVE(BADS_Leap_GetChannelDefaultSettings);
    return retCode;
}

BERR_Code BADS_Leap_OpenChannel(
    BADS_Handle hDev,                                   /* [in] Device handle */
    BADS_ChannelHandle *phChn,                          /* [out] Returns channel handle */
    unsigned int channelNo,                             /* [in] Channel number to open */
    const struct BADS_ChannelSettings *pChnDefSettings  /* [in] Channel default setting */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_Handle hImplDev;
    BADS_Leap_ChannelHandle hImplChnDev = NULL;
    BADS_ChannelHandle hChnDev;

    BDBG_ENTER(BADS_Leap_OpenChannel);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    hImplDev = (BADS_Leap_Handle) hDev->pImpl;
    BDBG_ASSERT( hImplDev );
    BDBG_ASSERT( hImplDev->hHab);

    hChnDev = NULL;
    if( channelNo < hImplDev->mxChnNo )
    {
        if( hImplDev->hAdsChn[channelNo] == NULL )
        {
            /* Alloc memory from the system heap */
            hChnDev = (BADS_ChannelHandle) BKNI_Malloc( sizeof( BADS_P_ChannelHandle ) );
            if( hChnDev == NULL )
            {
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BADS_OpenChannel: BKNI_malloc() failed"));
                goto done;
            }

            BKNI_Memset( hChnDev, 0x00, sizeof( BADS_P_ChannelHandle ) );
            hChnDev->magicId = DEV_MAGIC_ID;
            hChnDev->hAds = hDev;

            hImplChnDev = (BADS_Leap_ChannelHandle) BKNI_Malloc( sizeof( BADS_P_Leap_ChannelHandle ) );
            if( hImplChnDev == NULL )
            {
                retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                BDBG_ERR(("BADS_OpenChannel: BKNI_malloc() failed"));
                goto done;
            }
            BKNI_Memset( hImplChnDev, 0x00, sizeof( BADS_P_Leap_ChannelHandle ) );
            hImplChnDev->chnNo = channelNo;
            hImplChnDev->devId = (BHAB_DevId) channelNo;
            /* set the modType to an invalid value so that the acquire parameters get sent */
            hImplChnDev->previousAcquireParams.modType = BADS_ModulationType_eLast;

            if (pChnDefSettings) hImplChnDev->settings = *pChnDefSettings;
            hImplChnDev->hHab = hImplDev->hHab;
            if(hImplDev->habBuffer)
                hImplChnDev->habBuffer = hImplDev->habBuffer;
            CHK_RETCODE(retCode, BKNI_CreateMutex(&hImplChnDev->mutex));
            hImplDev->hAdsChn[channelNo] = hChnDev;
            hImplChnDev->bPowerdown = true;
            hChnDev->pImpl = hImplChnDev;

            *phChn = hChnDev;
        }
        else
        {
            retCode = BERR_TRACE(BADS_ERR_NOTAVAIL_CHN_NO);
        }
    }
    else
    {
        BDBG_ERR(("Maximum number of ADS channels supported on %x is %d", hImplDev->chipId, hImplDev->mxChnNo));
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
    }

done:
    if( retCode != BERR_SUCCESS )
    {
        if( hChnDev != NULL )
        {
            BKNI_Free( hChnDev );
            hImplDev->hAdsChn[channelNo] = NULL;
        }
        if( hImplChnDev != NULL )
        {
            BKNI_Free( hImplChnDev );
        }
        *phChn = NULL;
    }
    BDBG_LEAVE(BADS_Leap_OpenChannel);
    return retCode;
}

BERR_Code BADS_Leap_CloseChannel(
    BADS_ChannelHandle hChn             /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_Handle hImplDev;
    BADS_Leap_ChannelHandle hImplChnDev;
    BADS_Handle hAds;
    unsigned int chnNo;

    BDBG_ENTER(BADS_Leap_CloseChannel);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    hAds = hChn->hAds;
    hImplDev = (BADS_Leap_Handle) hAds->pImpl;
    BDBG_ASSERT( hImplDev );

    BHAB_UnInstallInterruptCallback(hImplChnDev->hHab, hImplChnDev->devId);

    BKNI_DestroyMutex(hImplChnDev->mutex);
    chnNo = hImplChnDev->chnNo;
    hChn->magicId = 0x00;       /* clear it to catch inproper use */
    BKNI_Free( hChn->pImpl );
    BKNI_Free( hChn );
    hImplDev->hAdsChn[chnNo] = NULL;

    BDBG_LEAVE(BADS_Leap_CloseChannel);
    return retCode;
}

BERR_Code BADS_Leap_GetDevice(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Handle *phDev                  /* [out] Returns Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BADS_Leap_GetDevice);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    *phDev = hChn->hAds;

    BDBG_LEAVE(BADS_Leap_GetDevice);
    return retCode;
}

BERR_Code BADS_Leap_GetDefaultAcquireParams(
    BADS_InbandParam *ibParams          /* [out] default Inband Parameters */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BADS_3462_GetDefaultAcquireParams);

    *ibParams = defInbandParams;

    BDBG_LEAVE(BADS_3462_GetDefaultAcquireParams);
    return( retCode );
}

BERR_Code BADS_Leap_SetAcquireParams(
    BADS_ChannelHandle hChn ,           /* [in] Device channel handle */
    const BADS_InbandParam *ibParams          /* [in] Inband Parameters to use */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t bps;
    uint8_t buf[13] = HAB_MSG_HDR(BADS_eAcquireParamsWrite, 0x8, BADS_CORE_TYPE);

    BDBG_ENTER(BADS_Leap_SetAcquireParams);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );
    BDBG_ASSERT( ibParams );

    BKNI_AcquireMutex(hImplChnDev->mutex);
    hImplChnDev->isLock = false;
    BKNI_ReleaseMutex(hImplChnDev->mutex);

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("ADS core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        if(!((hImplChnDev->previousAcquireParams.tuneAcquire == ibParams->tuneAcquire) &&
            (hImplChnDev->previousAcquireParams.modType == ibParams->modType) &&
            (hImplChnDev->previousAcquireParams.frequencyOffset == ibParams->frequencyOffset) &&
            (hImplChnDev->previousAcquireParams.symbolRate == ibParams->symbolRate) &&
            (hImplChnDev->previousAcquireParams.invertSpectrum == ibParams->invertSpectrum) &&
            (hImplChnDev->previousAcquireParams.autoAcquire == ibParams->autoAcquire) &&
            (hImplChnDev->previousAcquireParams.acquisitionType == ibParams->acquisitionType)))
        {
            /* set Acquire Parameters */
            buf[3] = hImplChnDev->chnNo;
            if(ibParams->autoAcquire)
               buf[4] |= 0x80;
            buf[4] |= (ibParams->acquisitionType << 5);
            if(ibParams->invertSpectrum)
               buf[4] |= 0x10;
            if(ibParams->modType > BADS_ModulationType_eAnnexAQam4096)
               buf[4] |= 0x8;

            switch ( ibParams->modType )
            {
                case BADS_ModulationType_eAnnexAQam16:
                    bps = 0x0;
                    break;
                case BADS_ModulationType_eAnnexAQam32:
                    bps = 0x1;
                    break;
                case BADS_ModulationType_eAnnexAQam64:
                    bps = 0x2;
                    break;
                case BADS_ModulationType_eAnnexAQam128:
                    bps = 0x3;
                    break;
                case BADS_ModulationType_eAnnexAQam256:
                    bps = 0x4;
                    break;
                case BADS_ModulationType_eAnnexAQam512:
                    bps = 0x5;
                    break;
                case BADS_ModulationType_eAnnexAQam1024:
                    bps = 0x6;
                    break;
                case BADS_ModulationType_eAnnexBQam64:
                    bps = 0x2;
                    break;
                case BADS_ModulationType_eAnnexBQam256:
                    bps = 0x4;
                    break;
                case BADS_ModulationType_eAnnexBQam1024:
                    bps = 0x6;
                    break;
                default:
                    retCode = BERR_INVALID_PARAMETER;
                    goto done;
            }

            buf[4] |= bps;
            buf[5] = (ibParams->tuneAcquire << 7);
            buf[5] |= ((ibParams->enableNullPackets) ? 0x1 : 0x00);
            buf[6] = (ibParams->frequencyOffset/256 >> 8);  /* Carrier Range [15:8] */
            buf[7] = ibParams->frequencyOffset/256;  /* Carrier Range [7:0] */

    /*        if(ibParams->modType <= BADS_ModulationType_eAnnexAQam4096)*/
            if(1)

            {
                /* set AnnexA SymbolRate */
                buf[8] = ibParams->symbolRate >> 24;
                buf[9] = ibParams->symbolRate >> 16;
                buf[10] = ibParams->symbolRate >> 8;
                buf[11] = ibParams->symbolRate;
            }
            else
            {
                buf[8] = 0;  /* Sym_Rate [31:24] */
                buf[9] = 0;  /* Sym_Rate [23:16] */
                buf[10] = 0;  /* Sym_Rate [15:8] */
                buf[11] = 0;  /* Sym_Rate [7:0] */
            }

            CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 13, buf, 0, false, true, 13 ));
            hImplChnDev->previousAcquireParams = *ibParams;
        }
    }
done:
    BDBG_LEAVE(BADS_Leap_SetAcquireParams);
    return retCode;
}

BERR_Code BADS_Leap_GetAcquireParams(
    BADS_ChannelHandle hChn ,           /* [in] Device channel handle */
    BADS_InbandParam *ibParams          /* [out] Inband Parameters to use */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[13] = HAB_MSG_HDR(BADS_eAcquireParamsRead, 0, BADS_CORE_TYPE);

    BDBG_ENTER(BADS_Leap_GetAcquireParams);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );
    BDBG_ASSERT( ibParams );

    BKNI_AcquireMutex(hImplChnDev->mutex);
    hImplChnDev->isLock = false;
    BKNI_ReleaseMutex(hImplChnDev->mutex);

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("ADS core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hImplChnDev->chnNo;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 5, buf, 13, false, true, 13 ));

        ibParams->autoAcquire = (buf[4] & 0x80) >> 7;
        ibParams->acquisitionType = (buf[4] & 0x60) >> 5;
        ibParams->invertSpectrum = (buf[4] & 0x10) >> 4;

        if((buf[4] & 0x8) >> 3)
        {
            switch(buf[4] & 0x7)
            {
                case 2:
                    ibParams->modType = BADS_ModulationType_eAnnexBQam64;
                    break;
                case 4:
                    ibParams->modType = BADS_ModulationType_eAnnexBQam256;
                    break;
                case 6:
                    ibParams->modType = BADS_ModulationType_eAnnexBQam1024;
                    break;
                default:
                    ibParams->modType = BADS_ModulationType_eAnnexBQam256;
            }
        }
        else
        {
            switch(buf[4] & 0x7)
            {
                case 0:
                    ibParams->modType = BADS_ModulationType_eAnnexAQam16;
                    break;
                case 1:
                    ibParams->modType = BADS_ModulationType_eAnnexAQam32;
                    break;
                case 2:
                    ibParams->modType = BADS_ModulationType_eAnnexAQam64;
                    break;
                case 3:
                    ibParams->modType = BADS_ModulationType_eAnnexAQam128;
                    break;
                case 4:
                    ibParams->modType = BADS_ModulationType_eAnnexAQam256;
                    break;
                case 5:
                    ibParams->modType = BADS_ModulationType_eAnnexAQam512;
                    break;
                case 6:
                    ibParams->modType = BADS_ModulationType_eAnnexAQam1024;
                    break;
                default:
                    ibParams->modType = BADS_ModulationType_eAnnexAQam256;
            }
        }

        ibParams->tuneAcquire = (buf[5] & 0x80) >> 7;
        ibParams->enableNullPackets = buf[5] & 0x1;
        ibParams->frequencyOffset = ((buf[6] << 8) | buf[7])/256;
        /* set AnnexA SymbolRate */
        ibParams->symbolRate = (buf[8] << 24) | (buf[9] << 16) | (buf[10] << 8) | buf[11];

    }
done:
    BDBG_LEAVE(BADS_Leap_GetAcquireParams);
    return retCode;
}


BERR_Code BADS_Leap_Acquire(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_InbandParam *ibParam           /* [in] Inband Parameters to use */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
#if BADS_CHIP==3158
    uint8_t buf[13] = HAB_MSG_HDR(BADS_eAcquire, 0x8, BADS_CORE_TYPE);
#else
    uint8_t buf[5] = HAB_MSG_HDR(BADS_eAcquire, 0, BADS_CORE_TYPE);
#endif
    BDBG_ENTER(BADS_Leap_Acquire);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );
    BSTD_UNUSED( ibParam );

    BKNI_AcquireMutex(hImplChnDev->mutex);
    hImplChnDev->isLock = false;
    BKNI_ReleaseMutex(hImplChnDev->mutex);

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("ADS core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        /* Acquire */
        buf[3] = hImplChnDev->chnNo;
#if BADS_CHIP==3158
        buf[8] = ibParam->frequency >> 24;
        buf[9] = ibParam->frequency >> 16;
        buf[10] = ibParam->frequency >> 8;
        buf[11] = ibParam->frequency;

        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 13, buf, 0, false, true, 13 ));
#else

        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 5, buf, 0, false, true, 5 ));
#endif
        CHK_RETCODE(retCode, BHAB_EnableLockInterrupt(hImplChnDev->hHab, hImplChnDev->devId, true));
    }

done:
    BDBG_LEAVE(BADS_Leap_Acquire);
    return retCode;
}


BERR_Code BADS_Leap_GetStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Status *pStatus                /* [out] Returns status */
    )
{
    BSTD_UNUSED(hChn);
    BSTD_UNUSED(pStatus);

    BDBG_ERR(("BADS_GetStatus not supported on this platform, please use BADS_GetAsyncStatus"));

    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


BERR_Code BADS_Leap_RequestAsyncStatus(
    BADS_ChannelHandle hChn            /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[5] = HAB_MSG_HDR(BADS_eRequestAsyncStatus, 0, BADS_CORE_TYPE);

    BDBG_ENTER(BADS_Leap_RequestAsyncStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("ADS core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hImplChnDev->chnNo;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 5, buf, 0, false, true, 5));
    }

done:
    BDBG_LEAVE(BADS_Leap_RequestAsyncStatus);
    return retCode;
}

BERR_Code BADS_Leap_GetAsyncStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Status *pStatus                /* [out] Returns status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t val = 0;
#if BADS_CHIP==3158
    uint8_t buf[133] = HAB_MSG_HDR(BADS_eGetAsyncStatus, 0, BADS_CORE_TYPE);
#else
    uint8_t buf[112] = HAB_MSG_HDR(BADS_eGetAsyncStatus, 0, BADS_CORE_TYPE);
#endif
    BDBG_ENTER(BADS_Leap_GetAsyncStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("ADS core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hImplChnDev->chnNo;
#if BADS_CHIP==3158
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 5, buf, 133, false, true, 133));
#else
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 5, buf, 112, false, true, 112));
#endif

        pStatus->isPowerSaverEnabled = false;

        val = (buf[4] & 0x7);

        pStatus->ifFreq = 0; /* Not supported */
        pStatus->symbolRate = (int32_t)((buf[0x1C] << 24) | (buf[0x1D] << 16) | (buf[0x1E] << 8) | buf[0x1F]);
        pStatus->isFecLock = (buf[7] & 0x40) >> 6;
        pStatus->isQamLock = (buf[7] & 0x80) >> 7;
        pStatus->correctedCount = 0;/* Not supported */
        pStatus->uncorrectedCount = 0;/* Not supported */
        pStatus->snrEstimate = (buf[0xE] << 8) | buf[0xF];
        pStatus->agcIntLevel = 0;/* Not supported */
        pStatus->agcExtLevel =  0;/* Not supported */
        pStatus->carrierFreqOffset = (int32_t)(((buf[0x20] << 24) | (buf[0x21] << 16) | (buf[0x22] << 8) | buf[0x23]) +
                                               ((buf[0x54] << 24) | (buf[0x55] << 16) | (buf[0x56] << 8) | buf[0x57])) * 1000;
        pStatus->symbolRateError = (int32_t)((buf[0x24] << 24) | (buf[0x25] << 16) | (buf[0x26] << 8) | buf[0x27]);
        pStatus->carrierPhaseOffset = (int32_t)((buf[0x28] << 24) | (buf[0x29] << 16) | (buf[0x2A] << 8) | buf[0x2B]) * 1000;
        pStatus->rxSymbolRate = (int32_t)((buf[0x1c] << 24) | (buf[0x1d] << 16) | (buf[0x1e] << 8) | buf[0x1f]);
        pStatus->interleaveDepth = 0; /* Not supported */
        pStatus->goodRsBlockCount = 0; /* Not supported */
        pStatus->berRawCount = (int32_t)((buf[0x40] << 24) | (buf[0x41] << 16) | (buf[0x42] << 8) | buf[0x43]);
        pStatus->dsChannelPower = (int32_t)(((((int8_t)buf[0x4a] << 8) | buf[0x4b])) + 12480 /*48.75*256*/)*10/256; /* in 10th of dBmV unit */
        pStatus->isSpectrumInverted = (buf[7] & 0x10) >> 4;
        pStatus->mainTap = (int32_t)((buf[0x18] << 24) | (buf[0x19] << 16) | (buf[0x1a] << 8) | buf[0x1b]);
        pStatus->feGain = ((buf[0x12] << 8) | buf[0x13])*100/256;
        pStatus->digitalAgcGain = ((buf[0x14] << 8) | buf[0x15])*100/256;
        pStatus->equalizerGain = ((buf[0x16] << 8) | buf[0x17])*100/256;
        pStatus->postRsBER = 0; /* Not supported */
        pStatus->elapsedTimeSec = 0; /* Not supported */
        pStatus->correctedBits = (int32_t)((buf[0x2c] << 24) | (buf[0x2d] << 16) | (buf[0x2e] << 8) | buf[0x2f]);
        pStatus->accCorrectedCount = (int32_t)((buf[0x30] << 24) | (buf[0x31] << 16) | (buf[0x32] << 8) | buf[0x33]);
        pStatus->accUncorrectedCount = (int32_t)((buf[0x34] << 24) | (buf[0x35]<< 16) | (buf[0x36]<< 8) | buf[0x37]);
        pStatus->accCleanCount = (int32_t)((buf[0x38] << 24) | (buf[0x39] << 16) | (buf[0x3a] << 8) | buf[0x3b]);
        pStatus->cleanCount = 0; /* Not supported */

        BDBG_MSG(("ADS Channel %d QamLock = %d, FecLock = %d, : ElapsedTime_u32= %d ms TotalTime= %d ms", hImplChnDev->chnNo, pStatus->isQamLock, pStatus->isFecLock, ((buf[0x0c] << 8) | buf[0x0d]), ((buf[0x48] << 8) | buf[0x49])));
    }

done:
    BDBG_LEAVE(BADS_Leap_GetAsyncStatus);
    return retCode;
}

BERR_Code BADS_Leap_GetScanStatus(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_ScanStatus *pScanStatus        /* [out] Returns status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[19] = HAB_MSG_HDR(BADS_eGetScanStatus, 0, BADS_CORE_TYPE);
    uint8_t annex, modulation;

    BDBG_ENTER(BADS_Leap_GetScanStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("ADS core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hImplChnDev->chnNo;
#if BADS_CHIP==3158
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 5, buf, 19, false, true, 19));
#else
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 5, buf, 17, false, true, 17));
#endif

        switch (buf[0x4])
        {
            case 0:
                pScanStatus->acquisitionStatus= BADS_AcquisitionStatus_eNoSignal;
                break;
            case 1:
                pScanStatus->acquisitionStatus = BADS_AcquisitionStatus_eNoSignal;
                break;
            case 2:
                pScanStatus->acquisitionStatus = BADS_AcquisitionStatus_eUnlocked;
                break;
            case 3:
                pScanStatus->acquisitionStatus = BADS_AcquisitionStatus_eUnlocked;
                break;
            case 4:
                pScanStatus->acquisitionStatus = BADS_AcquisitionStatus_eLockedFast;
                break;
            case 5:
                pScanStatus->acquisitionStatus = BADS_AcquisitionStatus_eLockedSlow;
                break;
            case 6:
                pScanStatus->acquisitionStatus = BADS_AcquisitionStatus_eNoSignal;
                break;
            default:
                BDBG_WRN(("Unrecognized Acquisition Status"));
                break;
        }

        modulation = (buf[6] & 0xFF) >> 4;
        annex = (buf[6] & 0x08) >> 3;

        if(annex)
        {
            switch(modulation)
            {
                case 2:
                    pScanStatus->modType = BADS_ModulationType_eAnnexBQam64;
                    break;
                case 4:
                    pScanStatus->modType = BADS_ModulationType_eAnnexBQam256;
                    break;
                case 6:
                    pScanStatus->modType = BADS_ModulationType_eAnnexBQam1024;
                    break;
                default:
                    BDBG_WRN(("Unrecognized modulation"));
                    break;
            }
        }
        else
        {
            switch(modulation)
            {
                case 0:
                    pScanStatus->modType = BADS_ModulationType_eAnnexAQam16;
                    break;
                case 1:
                    pScanStatus->modType = BADS_ModulationType_eAnnexAQam32;
                    break;
                case 2:
                    pScanStatus->modType = BADS_ModulationType_eAnnexAQam64;
                    break;
                case 3:
                    pScanStatus->modType = BADS_ModulationType_eAnnexAQam128;
                    break;
                case 4:
                    pScanStatus->modType = BADS_ModulationType_eAnnexAQam256;
                    break;
                case 5:
                    pScanStatus->modType = BADS_ModulationType_eAnnexAQam512;
                    break;
                case 6:
                    pScanStatus->modType = BADS_ModulationType_eAnnexAQam1024;
                    break;
                default:
                    BDBG_WRN(("Unrecognized modulation"));
                    break;
            }
        }

        pScanStatus->isSpectrumInverted = (buf[6] & 0x04) >> 2;
        pScanStatus->autoAcquire = (buf[6] & 0x02) >> 1;

        switch (buf[7])
        {
            case 0:
                pScanStatus->interleaver = BADS_Interleaver_eI128_J1;
                break;
            case 1:
                pScanStatus->interleaver = BADS_Interleaver_eI128_J2;
                break;
            case 2:
                pScanStatus->interleaver = BADS_Interleaver_eI128_J3;
                break;
            case 3:
                pScanStatus->interleaver = BADS_Interleaver_eI128_J4;
                break;
            case 4:
                pScanStatus->interleaver = BADS_Interleaver_eI64_J2;
                break;
            case 5:
                pScanStatus->interleaver = BADS_Interleaver_eI32_J4;
                break;
            case 6:
                pScanStatus->interleaver = BADS_Interleaver_eI16_J8;
                break;
            case 7:
                pScanStatus->interleaver = BADS_Interleaver_eI8_J16;
                break;
            case 8:
                pScanStatus->interleaver = BADS_Interleaver_eI4_J32;
                break;
            case 9:
                pScanStatus->interleaver = BADS_Interleaver_eI2_J64;
                break;
            case 10:
                pScanStatus->interleaver = BADS_Interleaver_eI1_J128;
                break;
            case 11:
                pScanStatus->interleaver = BADS_Interleaver_eI12_J17;
                break;
            case 12:
                pScanStatus->interleaver = BADS_Interleaver_eUnsupported;
                break;
            default:
                BDBG_WRN(("Unrecognized interleaver Status"));
                break;
        }
        pScanStatus->symbolRate = (uint32_t)((buf[8] << 24) | (buf[9] << 16) | (buf[10] << 8) | buf[11]);
        pScanStatus->carrierFreqOffset = (int32_t)((buf[12] << 24) | (buf[13] << 16) | (buf[14] << 8) | buf[15]);
    }

done:
    BDBG_LEAVE(BADS_Leap_GetScanStatus);
    return retCode;
}


BERR_Code BADS_Leap_GetSoftDecision(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    int16_t nbrToGet,                   /* [in] Number values to get */
    int16_t *iVal,                      /* [out] Ptr to array to store output I soft decision */
    int16_t *qVal,                      /* [out] Ptr to array to store output Q soft decision */
    int16_t *nbrGotten                  /* [out] Number of values gotten/read */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t i;
    uint8_t buf[0x7D] = HAB_MSG_HDR(BADS_eGetConstellation, 0, BADS_CORE_TYPE);

    BDBG_ENTER(BADS_Leap_GetSoftDecision);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("ADS core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hImplChnDev->chnNo;

        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 5, buf, 0x7D, false, true, 0x7D));

        for (i = 0; i < 30 && i < nbrToGet; i++)
        {
            iVal[i] = (buf[4+(4*i)] << 8) | (buf[5+(4*i)]);
            qVal[i] = (buf[6+(4*i)] << 8) | (buf[7+(4*i)]);
        }

        *nbrGotten = i;
    }

done:


    BDBG_LEAVE(BADS_Leap_GetSoftDecision);
    return retCode;
}


BERR_Code BADS_Leap_EnablePowerSaver(
    BADS_ChannelHandle hChn             /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[5] = HAB_MSG_HDR(BADS_ePowerCtrlOff, 0, BADS_CORE_TYPE);

    BDBG_ENTER(BADS_Leap_EnablePowerSaver);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if(!hImplChnDev->bPowerdown)
    {
        buf[3] = hImplChnDev->chnNo;
#if BADS_CHIP==3158
        if(hImplChnDev->chnNo == BADS_IFDAC_CHANNEL_NUMBER) {
            buf[2] = (buf[2] & 0xF0) | BADS_IFDAC_CORE_TYPE;
            buf[3] = 0;
        }
#endif
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 5, buf, 0, false, true, 5));
        hImplChnDev->bPowerdown = true;

        /* set the modType to an invalid value so that the acquire parameters get sent */
        hImplChnDev->previousAcquireParams.modType = BADS_ModulationType_eLast;
    }

done:
    BDBG_LEAVE(BADS_Leap_EnablePowerSaver);
    return retCode;
}

BERR_Code BADS_Leap_DisablePowerSaver(
    BADS_ChannelHandle hChn             /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[5] = HAB_MSG_HDR(BADS_ePowerCtrlOn, 0, BADS_CORE_TYPE);

    BDBG_ENTER(BADS_Leap_DisablePowerSaver);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if(hImplChnDev->bPowerdown)
    {
        buf[3] = hImplChnDev->chnNo;
#if BADS_CHIP==3158
        if(hImplChnDev->chnNo == BADS_IFDAC_CHANNEL_NUMBER) {
            buf[2] = (buf[2] & 0xF0) | BADS_IFDAC_CORE_TYPE;
            buf[3] = 0;
        }
#endif
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 5, buf, 0, false, true, 5));
        hImplChnDev->bPowerdown = false;
    }

done:
    BDBG_LEAVE(BADS_Leap_DisablePowerSaver);
    return retCode;
}

BERR_Code BADS_Leap_ProcessNotification(
    BADS_ChannelHandle hChn,                /* [in] Device channel handle */
    unsigned int event                      /* [in] Event code and event data*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hChn);
    BSTD_UNUSED(event);

    BDBG_WRN(("Not supported for this frontend chip."));
    return retCode;
}

BERR_Code BADS_Leap_InstallCallback(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_Callback callbackType,         /* [in] Type of callback */
    BADS_CallbackFunc pCallback,        /* [in] Function Ptr to callback */
    void *pParam                        /* [in] Generic parameter send on callback */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;

    BDBG_ENTER(BADS_Leap_InstallCallback);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );

    switch( callbackType )
    {
        case BADS_Callback_eLockChange:
        case BADS_Callback_eUpdateGain:
        case BADS_Callback_eNoSignal:
        case BADS_Callback_eAsyncStatusReady:
        case BADS_Callback_eSpectrumDataReady:
#if BADS_CHIP==3158
        case BADS_Callback_eIfDacAcquireComplete:
        case BADS_Callback_eIfDacStatusReady:
#endif
            hImplChnDev->pCallback[callbackType] = pCallback;
            hImplChnDev->pCallbackParam[callbackType] = pParam;
            break;
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    BHAB_InstallInterruptCallback( hImplChnDev->hHab,   hImplChnDev->devId, BADS_Leap_P_EventCallback_isr , (void *)hChn, callbackType);

    BDBG_LEAVE(BADS_Leap_InstallCallback);
    return retCode;
}

BERR_Code BADS_Leap_SetDaisyChain(
    BADS_Handle hDev,       /* [in] Returns handle */
    bool enableDaisyChain   /* [in] Eanble/disable daisy chain. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hDev);
    BSTD_UNUSED(enableDaisyChain);

    return retCode ;
}

BERR_Code BADS_Leap_GetDaisyChain(
    BADS_Handle hDev,           /* [in] Returns handle */
    bool *isEnableDaisyChain    /* [in] Eanble/disable daisy chain. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hDev);
    BSTD_UNUSED(isEnableDaisyChain);

    return retCode;
}

BERR_Code BADS_Leap_ResetStatus(
    BADS_ChannelHandle hChn     /* [in] Device channel handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[5] = HAB_MSG_HDR(BADS_eResetStatus, 0, BADS_CORE_TYPE);

    BDBG_ENTER(BADS_Leap_ResetStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("ADS core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hImplChnDev->chnNo;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 5, buf, 0, false, true, 5));
    }

done:
    BDBG_LEAVE(BADS_Leap_ResetStatus);
    return retCode;
}

BERR_Code BADS_Leap_WriteSlave(
    BADS_ChannelHandle hChn,     /* [in] Device channel handle */
    uint8_t chipAddr,            /* [in] chip addr of the i2c slave device */
    uint32_t subAddr,            /* [in] sub addr of the register to read from the slave device */
    uint8_t subAddrLen,          /* [in] how many bytes is the sub addr? one to four*/
    uint32_t *data,              /* [in] ptr to the data that we will write to the slave device */
    uint8_t dataLen              /* [in] how many bytes are we going to write? one to four*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hChn);
    BSTD_UNUSED(chipAddr);
    BSTD_UNUSED(subAddr);
    BSTD_UNUSED(subAddrLen);
    BSTD_UNUSED(data);
    BSTD_UNUSED(dataLen);

    return( retCode );
}

BERR_Code BADS_Leap_ReadSlave(
    BADS_ChannelHandle hChn,     /* [in] Device channel handle */
    uint8_t chipAddr,            /* [in] chip addr of the i2c slave device */
    uint32_t subAddr,            /* [in] sub addr of the register to read from the slave device */
    uint8_t subAddrLen,          /* [in] how many bytes is the sub addr? one to four*/
    uint32_t *data,              /* [out] ptr to the data that we will read from the slave device */
    uint8_t dataLen              /* [in] how many bytes are we going to read? one to four*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hChn);
    BSTD_UNUSED(chipAddr);
    BSTD_UNUSED(subAddr);
    BSTD_UNUSED(subAddrLen);
    BSTD_UNUSED(data);
    BSTD_UNUSED(dataLen);


    return( retCode );
}

BERR_Code BADS_Leap_SetScanParam(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_ChannelScanSettings *pChnScanSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[21] = HAB_MSG_HDR(BADS_eScanParamsWrite, 0x10, BADS_CORE_TYPE);

    BDBG_ENTER(BADS_Leap_SetScanParam);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("ADS core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hImplChnDev->chnNo;
        buf[4] = pChnScanSettings->A512;
        buf[4] |= (pChnScanSettings->A1024 << 1);
        buf[4] |= (pChnScanSettings->TO << 4);
        buf[4] |= (pChnScanSettings->CO << 5);
        buf[4] |= (pChnScanSettings->QM << 6);
        buf[4] |= (pChnScanSettings->AI << 7);
        buf[5] = pChnScanSettings->B64;
        buf[5] |= (pChnScanSettings->B256 << 1);
        buf[5] |= (pChnScanSettings->B1024 << 2);
        buf[5] |= (pChnScanSettings->A16 << 3);
        buf[5] |= (pChnScanSettings->A32 << 4);
        buf[5] |= (pChnScanSettings->A64 << 5);
        buf[5] |= (pChnScanSettings->A128 << 6);
        buf[5] |= (pChnScanSettings->A256 << 7);
        buf[10] = (pChnScanSettings->carrierSearch >> 8);
        buf[11] = pChnScanSettings->carrierSearch;
        buf[12] = (pChnScanSettings->upperBaudSearch >> 24);
        buf[13] = (pChnScanSettings->upperBaudSearch >> 16);
        buf[14] = (pChnScanSettings->upperBaudSearch >> 8);
        buf[15] = pChnScanSettings->upperBaudSearch;
        buf[16] = (pChnScanSettings->lowerBaudSearch >> 24);
        buf[17] = (pChnScanSettings->lowerBaudSearch >> 16);
        buf[18] = (pChnScanSettings->lowerBaudSearch >> 8);
        buf[19] = pChnScanSettings->lowerBaudSearch;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 21, buf, 0, false, true, 21));
    }

done:
    BDBG_LEAVE(BADS_Leap_SetScanParam);
    return( retCode );
}

BERR_Code BADS_Leap_GetScanParam(
    BADS_ChannelHandle hChn,            /* [in] Device channel handle */
    BADS_ChannelScanSettings *pChnScanSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[21] = HAB_MSG_HDR(BADS_eScanParamsRead, 0x0, BADS_CORE_TYPE);

    BDBG_ENTER(BADS_Leap_GetScanParam);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("ADS core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hImplChnDev->chnNo;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 5, buf, 21, false, true, 21));

        pChnScanSettings->A512 = (buf[4] & 0x1);
        pChnScanSettings->A1024 = (buf[4] & 0x2) >> 1;
        pChnScanSettings->TO = (buf[4] & 0x10) >> 4;
        pChnScanSettings->CO = (buf[4] & 0x20) >> 5;
        pChnScanSettings->QM = (buf[4] & 0x40) >> 6;
        pChnScanSettings->AI = (buf[4] & 0x80) >> 7;
        pChnScanSettings->B64 =  (buf[5] & 0x1);
        pChnScanSettings->B256 =  (buf[5] & 0x2) >> 1;
        pChnScanSettings->B1024 =  (buf[5] & 0x4) >> 2;
        pChnScanSettings->A16 =  (buf[5] & 0x8) >> 3;
        pChnScanSettings->A32 =  (buf[5] & 0x10) >> 4;
        pChnScanSettings->A64 =  (buf[5] & 0x20) >> 5;
        pChnScanSettings->A128 =  (buf[5] & 0x40) >> 6;
        pChnScanSettings->A256  =  (buf[5] & 0x80) >> 7;
        pChnScanSettings->carrierSearch  =  ((buf[10] << 8) | buf[11]);
        pChnScanSettings->upperBaudSearch  =  ((buf[12] << 24) | (buf[13] << 16) | (buf[14] << 8) | buf[15]);
        pChnScanSettings->lowerBaudSearch  =  ((buf[16] << 24) | (buf[17] << 16) | (buf[18] << 8) | buf[19]);
    }
done:
    BDBG_LEAVE(BADS_Leap_GetScanParam);
    return( retCode );
}
#if BADS_CHIP==3158
BERR_Code BADS_Leap_RequestSpectrumAnalyzerData(
    BADS_ChannelHandle hChn,     /* [in] Device channel handle */
    BADS_SpectrumSettings *pSettings /* [in] spectrum settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[21] = HAB_MSG_HDR(BADS_eReqSpectrumAnalyzerData, 0x10, BADS_SPECA_CORE_TYPE);

    BDBG_ENTER(BADS_Leap_RequestSpectrumAnalyzerData);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( pSettings );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("ADS core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = BADS_SPECA_CORE_ID;
        buf[4] = pSettings->startFreq >> 24;
        buf[5] = pSettings->startFreq >> 16;
        buf[6] = pSettings->startFreq >> 8;
        buf[7] = pSettings->startFreq;
        buf[8] = pSettings->stopFreq >> 24;
        buf[9] = pSettings->stopFreq >> 16;
        buf[10] = pSettings->stopFreq >> 8;
        buf[11] = pSettings->stopFreq;
        buf[12] = pSettings->fftSize;
        buf[13] = pSettings->binAverage;
        buf[16] = pSettings->numSamples >> 24;
        buf[17] = pSettings->numSamples >> 16;
        buf[18] = pSettings->numSamples >> 8;
        buf[19] = pSettings->numSamples;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 21, buf, 0, false, true, 21));
    }

done:
    BDBG_LEAVE(BADS_Leap_RequestSpectrumAnalyzerData);
    return( retCode );
}

BERR_Code BADS_Leap_GetSpectrumAnalyzerData(
    BADS_ChannelHandle hChn,     /* [in] Device channel handle */
    BADS_SpectrumData  *pSpectrumData /* [out] spectrum Data*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[13] = HAB_MSG_HDR(BADS_eGetSpectrumAnalyzerData, 0x8, BADS_SPECA_CORE_TYPE);
    uint16_t i;

    BDBG_ENTER(BADS_Leap_GetSpectrumAnalyzerData);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("ADS core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        if(hImplChnDev->habBuffer == NULL) {
            BDBG_ERR(("HAB Buffer is not allocated, make sure hab buffer is allocated first"));
            retCode = BERR_TRACE(BERR_NOT_INITIALIZED);
            goto done;
        }

        buf[3] = BADS_SPECA_CORE_ID;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 13, hImplChnDev->habBuffer, (uint16_t)BHAB_BUFFER_SIZE, false, true, 0));

        pSpectrumData->datalength = ((((hImplChnDev->habBuffer[1] & 0x3F) << 4) | (hImplChnDev->habBuffer[2] >> 4) ) - 8)/4;
        pSpectrumData->moreData = (hImplChnDev->habBuffer[4] >> 7) & 0x1;

        if(pSpectrumData->data != NULL)
        {

            for(i=0; i < pSpectrumData->datalength; i++)
            {
                *pSpectrumData->data++ = ((hImplChnDev->habBuffer[12 + i*4] << 24) | (hImplChnDev->habBuffer[13 + i*4] << 16)| (hImplChnDev->habBuffer[14 + i*4] << 8)| hImplChnDev->habBuffer[15 + i*4]);
            }
        }
    }
done:
    BDBG_LEAVE(BADS_Leap_GetSpectrumAnalyzerData);
    return( retCode );
}

BERR_Code BADS_Leap_TuneIfDac(
    BADS_ChannelHandle hChn,        /* [in] Device handle */
    BADS_IfDacSettings *pSettings   /* [in] IF DAC Settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[35] = HAB_MSG_HDR(BADS_SET_IFDAC_PARAMETERS, 0x1E, BADS_IFDAC_CORE_TYPE);
    uint8_t bandwidth=0;

    BDBG_ENTER(BADS_Leap_TuneIfDac);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("IFDAC core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        if(pSettings->bandwidth == 6000000)
            bandwidth = 1;
        buf[5] = (bandwidth << 4) | 0x8;
        buf[6] = 0x80;
        buf[8] = (pSettings->frequency >> 24);
        buf[9] = (pSettings->frequency >> 16);
        buf[10] = (pSettings->frequency >> 8);
        buf[11] = pSettings->frequency;
        buf[12] = (pSettings->outputFrequency >> 24);
        buf[13] = (pSettings->outputFrequency >> 16);
        buf[14] = (pSettings->outputFrequency >> 8);
        buf[15] = pSettings->outputFrequency;
        buf[16] = (pSettings->dacAttenuation >> 8);
        buf[17] = pSettings->dacAttenuation;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 35, buf, 0, false, true, 35));
        hImplChnDev->ifDacSettings = *pSettings;
    }

done:
    BDBG_LEAVE(BADS_Leap_TuneIfDac);
    return( retCode );
}

BERR_Code BADS_Leap_ResetIfDacStatus(
    BADS_ChannelHandle hChn        /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[17] = HAB_MSG_HDR(BADS_RESET_IFDAC_STATUS, 0xc, BADS_IFDAC_CORE_TYPE);

    BDBG_ENTER(BADS_Leap_ResetIfDacStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );

    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("IFDAC core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = 0;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 17, buf, 0, false, true, 17));
    }

done:
    BDBG_LEAVE(BADS_Leap_ResetIfDacStatus);
    return( retCode );
}

BERR_Code BADS_Leap_RequestIfDacStatus(
    BADS_ChannelHandle hChn        /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[17] = HAB_MSG_HDR(BADS_REQ_IFDAC_STATUS, 0xc, BADS_IFDAC_CORE_TYPE);

    BDBG_ENTER(BADS_Leap_RequestIfDacStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );
    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("IFDAC core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = 0;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 17, buf, 0, false, true, 17));
    }

done:
    BDBG_LEAVE(BADS_Leap_RequestIfDacStatus);
    return( retCode );
}

BERR_Code BADS_Leap_GetIfDacStatus(
    BADS_ChannelHandle hChn,               /* [in] Device handle */
    BADS_IfDacStatus *pStatus       /* [out] Returns status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BADS_Leap_ChannelHandle hImplChnDev;
    uint8_t buf[69] = HAB_MSG_HDR(BADS_GET_IFDAC_STATUS, 0xc, BADS_IFDAC_CORE_TYPE);

    BDBG_ENTER(BADS_Leap_GetIfDacStatus);
    BDBG_ASSERT( hChn );
    BDBG_ASSERT( hChn->magicId == DEV_MAGIC_ID );

    hImplChnDev = (BADS_Leap_ChannelHandle) hChn->pImpl;
    BDBG_ASSERT( hImplChnDev );
    BDBG_ASSERT( hImplChnDev->hHab );
    if(hImplChnDev->bPowerdown)
    {
        BDBG_ERR(("IFDAC core %d Powered Off", hImplChnDev->chnNo));
        retCode = BERR_TRACE(BADS_ERR_POWER_DOWN);
    }
    else
    {
        pStatus->ifDacSettings = hImplChnDev->ifDacSettings;
        buf[3] = 0;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hImplChnDev->hHab, buf, 17, buf, 69, false, true, 69));
        pStatus->rssi = (int16_t)((buf[20] << 8) | buf[21])*100/256;
    }

done:

    BDBG_LEAVE(BADS_Leap_GetIfDacStatus);
    return( retCode );
}
#endif