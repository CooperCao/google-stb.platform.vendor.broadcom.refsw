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
#include "btnr_7584ib_priv.h"
#include "bhab.h"

BDBG_MODULE(btnr_7584_priv);

#define DEV_MAGIC_ID                    ((BERR_TNR_ID<<16) | 0xFACE)

/*******************************************************************************
*
*   Private Module Functions
*
*******************************************************************************/
BERR_Code BTNR_7584_P_EventCallback_isr(
    void * pParam1, int param2
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_7584Ib_Handle hDev = (BTNR_7584Ib_Handle) pParam1;
    BHAB_InterruptType event = (BHAB_InterruptType) param2;

    BDBG_ENTER(BTNR_7584_P_EventCallback_isr);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->hHab);

    switch (event) {
        case BHAB_Interrupt_eSpectrumAnalyzerDataReady:
            {
                if( hDev->pCallback[BTNR_Callback_eSpectrumDataReady] != NULL )
                {
                    (hDev->pCallback[BTNR_Callback_eSpectrumDataReady])(hDev->pCallbackParam[BTNR_Callback_eSpectrumDataReady] );
                }
            }
            break;
        case BHAB_Interrupt_eIfDacAcquireComplete:
            {
                if( hDev->pCallback[BTNR_Callback_eIfDacAcquireComplete] != NULL )
                {
                    (hDev->pCallback[BTNR_Callback_eIfDacAcquireComplete])(hDev->pCallbackParam[BTNR_Callback_eIfDacAcquireComplete] );
                }
            }
            break;
        case BHAB_Interrupt_eIfDacStatusReady:
            {
                if( hDev->pCallback[BTNR_Callback_eIfDacStatusReady] != NULL )
                {
                    (hDev->pCallback[BTNR_Callback_eIfDacStatusReady])(hDev->pCallbackParam[BTNR_Callback_eIfDacStatusReady] );
                }
            }
            break;
        default:
            BDBG_WRN((" unknown event code from 7584"));
            break;
    }

    BDBG_LEAVE(BTNR_7584_P_EventCallback_isr);
    return retCode;
}

/*******************************************************************************
*
*   Public Module Functions
*
*******************************************************************************/
BERR_Code BTNR_7584Ib_InstallCallback(
    BTNR_7584Ib_Handle hDev,     /* [in] Device handle */
    BTNR_Callback callbackType, /* [in] Type of callback */
    BTNR_CallbackFunc pCallback, /* [in] Function Ptr to callback */
    void *pParam                 /* [in] Generic parameter send on callback */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_7584Ib_InstallCallback);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    switch( callbackType )
    {
        case BTNR_Callback_eSpectrumDataReady:
        case BTNR_Callback_eIfDacAcquireComplete:
        case BTNR_Callback_eIfDacStatusReady:
            hDev->pCallback[callbackType] = pCallback;
            hDev->pCallbackParam[callbackType] = pParam;
            break;
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    hDev->devId = BHAB_DevId_eTNR0 + hDev->channelNo;
    BHAB_InstallInterruptCallback( hDev->hHab, hDev->devId, BTNR_7584_P_EventCallback_isr , (void *)hDev, callbackType);

    BDBG_LEAVE(BTNR_7584Ib_InstallCallback);
    return retCode;
}

BERR_Code BTNR_7584Ib_Close(
    BTNR_Handle hDev                    /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_7584Ib_Handle btnr_7584ib_handle;
    BTNR_P_7584Ib_Settings btnr_7584ib_settings;


    BDBG_ENTER(BTNR_7584Ib_Close);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    /* verify the handle is good before using it */
    btnr_7584ib_handle = (BTNR_7584Ib_Handle) hDev->hDevImpl;
    btnr_7584ib_settings = btnr_7584ib_handle ->settings;

    hDev->magicId = 0x00;       /* clear it to catch inproper use */
    BKNI_Free( (void *) hDev->hDevImpl );
    BKNI_Free( (void *) hDev );

    BDBG_LEAVE(BTNR_7584Ib_Close);
    return retCode;
}

BERR_Code BTNR_7584Ib_SetRfFreq(
    BTNR_7584Ib_Handle hDev,            /* [in] Device handle */
    uint32_t rfFreq,                    /* [in] Requested tuner freq., in Hertz */
    BTNR_TunerMode tunerMode            /* [in] Requested tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_P_7584Ib_Settings *pTnrImplData;
    uint8_t hab[13] = HAB_MSG_HDR(BTNR_ACQUIRE_PARAMS_WRITE, 0x8, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BSTD_UNUSED(tunerMode);

    BDBG_ENTER(BTNR_7584Ib_SetRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;

    if(hDev->bPowerdown)
    {
        BDBG_ERR(("TNR core %d Powered Off", hDev->channelNo));
        retCode = BERR_TRACE(BTNR_ERR_POWER_DOWN);
    }
    else
    {
        hab[4] = hDev->channelNo;
        hab[5] = BTNR_CORE_TYPE_TO_FEED;
        hab[6] = 0x80;
        hab[8] = (rfFreq >> 24);
        hab[9] = (rfFreq >> 16);
        hab[10] = (rfFreq >> 8);
        hab[11] = rfFreq;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, hab, 13, hab, 0, false, true, 13));
    }

done:
    BDBG_LEAVE(BTNR_7584Ib_SetRfFreq);
    return retCode;
}

BERR_Code BTNR_7584Ib_GetRfFreq(
    BTNR_7584Ib_Handle hDev,            /* [in] Device handle */
    uint32_t *rfFreq,                   /* [output] Returns tuner freq., in Hertz */
    BTNR_TunerMode *tunerMode           /* [output] Returns tuner mode */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BTNR_P_7584Ib_Settings *pTnrImplData;


    BDBG_ENTER(BTNR_7584Ib_GetRfFreq);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;

    *rfFreq = pTnrImplData->rfFreq;
    *tunerMode = pTnrImplData->tunerMode;

    BDBG_LEAVE(BTNR_7584Ib_GetRfFreq);
    return retCode;
}

BERR_Code BTNR_P_7584Ib_GetAgcRegVal(
    BTNR_7584Ib_Handle hDev,            /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [out] output value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hDev);
    BSTD_UNUSED(regOffset);
    BSTD_UNUSED(agcVal);

    return retCode;
}

BERR_Code BTNR_7584Ib_SetAgcRegVal(
    BTNR_7584Ib_Handle hDev,            /* [in] Device handle */
    uint32_t regOffset,                 /* [in] AGC register offset */
    uint32_t *agcVal                    /* [in] input value */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(hDev);
    BSTD_UNUSED(regOffset);
    BSTD_UNUSED(agcVal);

    return retCode;
}

BERR_Code BTNR_7584Ib_GetInfo(
    BTNR_7584Ib_Handle hDev,            /* [in] Device handle */
    BTNR_TunerInfo *tnrInfo             /* [out] Tuner information */
    )
{
    BTNR_P_7584Ib_Settings *pTnrImplData;
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_7584Ib_GetInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    pTnrImplData = &hDev->settings;
    tnrInfo->tunerMaker = BRCM_TUNER_MAKER_ID;
    tnrInfo->tunerId = 0x7584;
    tnrInfo->tunerMajorVer = pTnrImplData->iRevLetter;
    tnrInfo->tunerMinorVer = pTnrImplData->iRevNumber;

    BDBG_LEAVE(BTNR_7584Ib_GetInfo);
    return retCode;
}

BERR_Code BTNR_7584Ib_GetPowerSaver(
    BTNR_7584Ib_Handle hDev,                    /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings        /* [in] Power saver settings. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BTNR_POWER_STATUS_READ, 0x0, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_7584Ib_GetPowerSaver);

    buf[3] = hDev->channelNo;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 5, buf, 9, false, true, 9));

    if(buf[4]){
        pwrSettings->enable = false;
    }
    else {
        pwrSettings->enable = true;
    }

done:
    BDBG_LEAVE(BTNR_7584Ib_GetPowerSaver);
    return retCode;
}

BERR_Code BTNR_7584Ib_SetPowerSaver(
    BTNR_7584Ib_Handle hDev,                    /* [in] Device handle */
    BTNR_PowerSaverSettings *pwrSettings /* [in] Power saver settings. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[5] = HAB_MSG_HDR(BTNR_POWER_ON, 0x0, BTNR_CORE_TYPE, BTNR_CORE_ID);
    uint8_t buf1[5] = HAB_MSG_HDR(BTNR_POWER_OFF, 0x0, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_7584Ib_SetPowerSaver);

    buf[3] = hDev->channelNo;
    buf1[3] = hDev->channelNo;

    if(pwrSettings->enable) {
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf1, 5, buf1, 0, false, true, 5));
        hDev->bPowerdown = true;
    }
    else {
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 5, buf, 0, false, true, 5));
        hDev->bPowerdown = false;
    }

done:
    BDBG_LEAVE(BTNR_7584Ib_SetPowerSaver);
    return retCode;
}

BERR_Code BTNR_7584Ib_GetSettings(
    BTNR_7584Ib_Handle hDev,    /* [in] Device handle */
    BTNR_Settings *settings     /* [out] TNR settings. */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_7584Ib_GetSettings);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( settings );

    BDBG_LEAVE(BTNR_7584Ib_GetSettings);
    return retCode;
}

BERR_Code BTNR_7584Ib_SetSettings(
    BTNR_7584Ib_Handle hDev,    /* [in] Device handle */
    BTNR_Settings *settings     /* [in] TNR settings. */
    )

{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BTNR_7584Ib_SetSettings);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( settings );

    BDBG_LEAVE(BTNR_7584Ib_SetSettings);
    return retCode;
}

BERR_Code BTNR_7584Ib_RequestSpectrumAnalyzerData(
    BTNR_7584Ib_Handle hDev,     /* [in] Device handle */
    BTNR_SpectrumSettings *pSettings /* [in] spectrum settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;

    uint8_t buf[21] = HAB_MSG_HDR(BTNR_REQUEST_SPECTRUM_DATA, 0x10, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_7584Ib_RequestSpectrumAnalyzerData);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( pSettings );

    buf[3] = hDev->channelNo;
    buf[4] = pSettings->startFreq >> 24;
    buf[5] = pSettings->startFreq >> 16;
    buf[6] = pSettings->startFreq >> 8;
    buf[7] = pSettings->startFreq;
    buf[8] = pSettings->stopFreq >> 24;
    buf[9] = pSettings->stopFreq >> 16;
    buf[10] = pSettings->stopFreq >> 8;
    buf[11] = pSettings->stopFreq;
    buf[12] = pSettings->fftSize;
    buf[16] = pSettings->numSamples >> 24;
    buf[17] = pSettings->numSamples >> 16;
    buf[18] = pSettings->numSamples >> 8;
    buf[19] = pSettings->numSamples;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 21, buf, 0, false, true, 21));


done:
    BDBG_LEAVE(BTNR_7584Ib_RequestSpectrumAnalyzerData);
    return( retCode );
}

BERR_Code BTNR_7584Ib_GetSpectrumAnalyzerData(
    BTNR_7584Ib_Handle hDev,     /* [in] Device handle */
    BTNR_SpectrumData  *pSpectrumData /* [out] spectrum Data*/
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[512] = HAB_MSG_HDR(BTNR_GET_SPECTRUM_DATA, 0x8, BTNR_CORE_TYPE, BTNR_CORE_ID);
    uint16_t i;

    BDBG_ENTER(BTNR_7584Ib_GetSpectrumAnalyzerData);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    BDBG_ASSERT( pSpectrumData );


    buf[3] = hDev->channelNo;
    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 13, buf, 512, false, true, 512));

    pSpectrumData->datalength = ((((buf[1] & 0x3F) << 4) | (buf[2] >> 4) ) - 8)/4;
    pSpectrumData->moreData = (buf[4] >> 7) & 0x1;

    if(pSpectrumData->data != NULL)
    {

        for(i=0; i < pSpectrumData->datalength; i++)
        {
            *pSpectrumData->data++ = ((buf[12 + i*4] << 24) | (buf[13 + i*4] << 16)| (buf[14 + i*4] << 8)| buf[15 + i*4]);
        }
    }

done:
    BDBG_LEAVE(BTNR_7584Ib_GetSpectrumAnalyzerData);
    return( retCode );
}

BERR_Code BTNR_7584Ib_GetVersionInfo(
    BTNR_7584Ib_Handle hDev,        /* [in] Device handle */
    BFEC_VersionInfo *pVersionInfo  /* [out] Returns version Info */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[29] = HAB_MSG_HDR(BTNR_SYS_VERSION, 0, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_7584Ib_GetVersionInfo);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 5, buf, 29, false, true, 29));
    pVersionInfo->majorVersion = (buf[4] << 8) | buf[5];
    pVersionInfo->minorVersion = (buf[6] << 8) | buf[7];
    pVersionInfo->buildType = (buf[8] << 8) | buf[9];
    pVersionInfo->buildId = (buf[10] << 8) | buf[11];

done:
    BDBG_LEAVE(BTNR_7584Ib_GetVersionInfo);
    return( retCode );
}

BERR_Code BTNR_7584Ib_TuneIfDac(
    BTNR_7584Ib_Handle hDev,        /* [in] Device handle */
    BTNR_IfDacSettings *pSettings   /* [in] IF DAC Settings */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[35] = HAB_MSG_HDR(BTNR_TUNE_IFDAC, 0x1E, BTNR_CORE_TYPE, BTNR_CORE_ID);
    uint8_t bandwidth=0;

    BDBG_ENTER(BTNR_7584Ib_TuneIfDac);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if(hDev->bPowerdown)
    {
        BDBG_ERR(("TNR core %d Powered Off", hDev->channelNo));
        retCode = BERR_TRACE(BTNR_ERR_POWER_DOWN);
    }
    else
    {
        if(pSettings->bandwidth == 6000000)
            bandwidth = 1;
        buf[3] = hDev->channelNo;
        buf[4] = hDev->channelNo;
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
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 35, buf, 0, false, true, 35));
        hDev->ifDacSettings = *pSettings;
    }

done:
    BDBG_LEAVE(BTNR_7584Ib_TuneIfDac);
    return( retCode );
}

BERR_Code BTNR_7584Ib_ResetIfDacStatus(
    BTNR_7584Ib_Handle hDev        /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[17] = HAB_MSG_HDR(BTNR_RESET_IFDAC_STATUS, 0xc, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_7584Ib_ResetIfDacStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );

    if(hDev->bPowerdown)
    {
        BDBG_ERR(("TNR core %d Powered Off", hDev->channelNo));
        retCode = BERR_TRACE(BTNR_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hDev->channelNo;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 17, buf, 0, false, true, 17));
    }

done:
    BDBG_LEAVE(BTNR_7584Ib_ResetIfDacStatus);
    return( retCode );
}

BERR_Code BTNR_7584Ib_RequestIfDacStatus(
    BTNR_7584Ib_Handle hDev        /* [in] Device handle */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[17] = HAB_MSG_HDR(BTNR_REQ_IFDAC_STATUS, 0xc, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_7584Ib_RequestIfDacStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    if(hDev->bPowerdown)
    {
        BDBG_ERR(("TNR core %d Powered Off", hDev->channelNo));
        retCode = BERR_TRACE(BTNR_ERR_POWER_DOWN);
    }
    else
    {
        buf[3] = hDev->channelNo;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 17, buf, 0, false, true, 17));
    }

done:
    BDBG_LEAVE(BTNR_7584Ib_RequestIfDacStatus);
    return( retCode );
}

BERR_Code BTNR_7584Ib_GetIfDacStatus(
    BTNR_7584Ib_Handle hDev,               /* [in] Device handle */
    BTNR_IfDacStatus *pStatus       /* [out] Returns status */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[121] = HAB_MSG_HDR(BTNR_GET_TUNER_STATUS, 0xc, BTNR_CORE_TYPE, BTNR_CORE_ID);

    BDBG_ENTER(BTNR_7584Ib_GetIfDacStatus);
    BDBG_ASSERT( hDev );
    BDBG_ASSERT( hDev->magicId == DEV_MAGIC_ID );
    if(hDev->bPowerdown)
    {
        BDBG_ERR(("TNR core %d Powered Off", hDev->channelNo));
        retCode = BERR_TRACE(BTNR_ERR_POWER_DOWN);
    }
    else
    {
        pStatus->ifDacSettings = hDev->ifDacSettings;
        buf[3] = hDev->channelNo;
        CHK_RETCODE(retCode, BHAB_SendHabCommand(hDev->hHab, buf, 17, buf, 121, false, true, 121));
        pStatus->rssi = (int16_t)((buf[26] << 8) | buf[27])*100/256;
    }

done:
    BDBG_LEAVE(BTNR_7584Ib_GetIfDacStatus);
    return( retCode );
}
