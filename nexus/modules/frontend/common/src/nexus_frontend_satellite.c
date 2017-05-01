/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
* API Description:
*   API name: Frontend QAM
*    Generic APIs for QAM tuning.
*
***************************************************************************/

#include "nexus_frontend_module.h"

BDBG_MODULE(nexus_frontend_satellite);

BTRC_MODULE_DECLARE(ChnChange_Tune);

void NEXUS_Frontend_GetDefaultSatelliteSettings( NEXUS_FrontendSatelliteSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->mode = NEXUS_FrontendSatelliteMode_eDvb;
    pSettings->symbolRate = 20000000;
    pSettings->searchRange = 5000000;
    /* Set the equivalent of BAST_ACQSETTINGS_DEFAULT - 0x00036004 */
    pSettings->spectralInversion = NEXUS_FrontendSatelliteInversion_eScan;
    pSettings->ldpcPilot = true;
    pSettings->ldpcPilotPll = true;
    pSettings->nyquist20 = true;
    pSettings->nyquistRolloff = NEXUS_FrontendSatelliteNyquistFilter_e20;
    NEXUS_CallbackDesc_Init(&pSettings->lockCallback);
}

void NEXUS_Frontend_GetDefaultSatelliteSettingsForMode( NEXUS_FrontendSatelliteMode mode, NEXUS_FrontendSatelliteSettings *pSettings )
{
    NEXUS_Frontend_GetDefaultSatelliteSettings(pSettings);
    pSettings->mode = mode;
    switch (mode) {
    default:
    case NEXUS_FrontendSatelliteMode_eDss:
    case NEXUS_FrontendSatelliteMode_eDvb:
        /* acq_ctl 0x0600C */
        pSettings->bertEnable = true;
        pSettings->nyquist20 = true;
        break;

    case NEXUS_FrontendSatelliteMode_eDcii:
    case NEXUS_FrontendSatelliteMode_eQpskTurbo:
    case NEXUS_FrontendSatelliteMode_e8pskTurbo:
    case NEXUS_FrontendSatelliteMode_eTurbo:
        break;

    case NEXUS_FrontendSatelliteMode_eQpskLdpc:
    case NEXUS_FrontendSatelliteMode_e8pskLdpc:
        /* acq_ctl 0x3600C */
        pSettings->bertEnable = true;
        pSettings->nyquist20 = true;
        pSettings->ldpcPilot = true;
        pSettings->ldpcPilotPll = true;
        break;

    case NEXUS_FrontendSatelliteMode_eLdpc:
        break;
    }
}

NEXUS_Error NEXUS_Frontend_TuneSatellite( NEXUS_FrontendHandle handle, const NEXUS_FrontendSatelliteSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }
    NEXUS_Frontend_P_SetTuned(handle);

    NEXUS_Time_Get(&handle->resetStatusTime);

    BDBG_ASSERT(NULL != pSettings);
    BTRC_TRACE(ChnChange_Tune, START);

    if ( NULL == handle->tuneSatellite )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_TuneSatellite(handle->pParentFrontend, pSettings);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->tuneSatellite(handle->pDeviceHandle, pSettings);
    }
}

/***************************************************************************
Summary:
    Get the status of a satellite tuner
See Also:
    NEXUS_Frontend_TuneSatellite
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetSatelliteStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendSatelliteStatus *pStatus
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pStatus);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getSatelliteStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetSatelliteStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getSatelliteStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Get the current DiSEqC settings for a satellite tuner
See Also:
    NEXUS_Frontend_SetDiseqcSettings
 ***************************************************************************/
void NEXUS_Frontend_GetDiseqcSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDiseqcSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pSettings);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        BDBG_ERR(("Device open failed. Cannot get diseqc settings."));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return;
    }

    if ( NULL == handle->getDiseqcSettings )
    {
        if ( handle->pParentFrontend )
        {
            NEXUS_Frontend_GetDiseqcSettings(handle->pParentFrontend, pSettings);
        }
        else
        {
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        handle->getDiseqcSettings(handle->pDeviceHandle, pSettings);
    }
}

/***************************************************************************
Summary:
    Set the current DiSEqC settings for a satellite tuner
See Also:
    NEXUS_Frontend_GetDiseqcSettings
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_SetDiseqcSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendDiseqcSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pSettings);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->setDiseqcSettings )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_SetDiseqcSettings(handle->pParentFrontend, pSettings);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->setDiseqcSettings(handle->pDeviceHandle, pSettings);
    }
}

NEXUS_Error NEXUS_Frontend_GetDiseqcStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDiseqcStatus *pStatus
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDiseqcStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDiseqcStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDiseqcStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Send a DiSEqC message
Description:
    This routine will send the number of bytes specified in a DiSEqC message.
    The callback provided will be called when the transfer is complete, at
    which point the reply/status can be read.
See Also:
    NEXUS_Frontend_GetDiseqcReply
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_SendDiseqcMessage(
    NEXUS_FrontendHandle handle,
    const uint8_t *pSendData,
    size_t sendDataSize,                    /* In Bytes */
    const NEXUS_CallbackDesc *pSendComplete /* Callback will be received when message status is ready */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pSendData);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->sendDiseqcMessage )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_SendDiseqcMessage(handle->pParentFrontend, pSendData, sendDataSize, pSendComplete);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->sendDiseqcMessage(handle->pDeviceHandle, pSendData, sendDataSize, pSendComplete);
    }
}

/***************************************************************************
Summary:
    Reset DiSEqC
Description:
    This routine will reset the diseqc.
See Also:
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_ResetDiseqc(
    NEXUS_FrontendHandle handle,
    uint8_t options
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->resetDiseqc )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_ResetDiseqc(handle->pParentFrontend, options);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->resetDiseqc(handle->pDeviceHandle, options);
    }
}

/***************************************************************************
Summary:
    Read a DiSEqC message reply
Description:
    This routine will read up to the number of bytes specified from a DiSEqC
    message reply.  Any remaining bytes will be discarded.
See Also:
    NEXUS_Frontend_SendDiseqcMessage
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDiseqcReply(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDiseqcMessageStatus *pStatus,
    uint8_t *pReplyBuffer,              /* [out] */
    size_t replyBufferSize,            /* in bytes */
    size_t *pReplyLength                /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pStatus);
    BDBG_ASSERT(NULL != pReplyBuffer);
    BDBG_ASSERT(NULL != pReplyLength);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDiseqcReply )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDiseqcReply(handle->pParentFrontend, pStatus, pReplyBuffer, replyBufferSize, pReplyLength);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDiseqcReply(handle->pDeviceHandle, pStatus, pReplyBuffer, replyBufferSize, pReplyLength);
    }
}

/***************************************************************************
Summary:
    Send a DiSEqC Auto Control Word
See Also:
    NEXUS_Frontend_SendDiseqcMessage
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_SendDiseqcAcw(
    NEXUS_FrontendHandle handle,
    uint8_t codeWord
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->sendDiseqcAcw )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_SendDiseqcAcw(handle->pParentFrontend, codeWord);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->sendDiseqcAcw(handle->pDeviceHandle, codeWord);
    }
}

NEXUS_Error NEXUS_Frontend_ReadSatelliteConfig( NEXUS_FrontendHandle handle, unsigned id, void *buffer, unsigned bufferSize )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->readSatelliteConfig )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_ReadSatelliteConfig(handle->pParentFrontend, id, buffer, bufferSize);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->readSatelliteConfig(handle->pDeviceHandle, id, buffer, bufferSize);
    }
}

NEXUS_Error NEXUS_Frontend_WriteSatelliteConfig( NEXUS_FrontendHandle handle, unsigned id, const void *buffer, unsigned bufferSize )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->writeSatelliteConfig )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_WriteSatelliteConfig(handle->pParentFrontend, id, buffer, bufferSize);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->writeSatelliteConfig(handle->pDeviceHandle, id, buffer, bufferSize);
    }
}

void NEXUS_Frontend_GetDefaultSatellitePeakscanSettings(NEXUS_FrontendSatellitePeakscanSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->frequency= 0;
    pSettings->minSymbolRate = 15 * 1000000;
    pSettings->maxSymbolRate = 30 * 1000000;
    pSettings->frequencyRange = 5 * 1000000;
    pSettings->frequencyStep = 1 * 1000000;
    NEXUS_CallbackDesc_Init(&pSettings->peakscanCallback);
}

void NEXUS_Frontend_GetDefaultSatelliteToneSearchSettings(NEXUS_FrontendSatelliteToneSearchSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->frequency= 0;
    pSettings->frequencyRange = 5 * 1000000;
    pSettings->minRatio.numerator = 5; /* default: 2.5  (2 1/2 = 5/2) */
    pSettings->minRatio.denominator = 2;
    NEXUS_CallbackDesc_Init(&pSettings->completionCallback);
}

NEXUS_Error NEXUS_Frontend_SatellitePeakscan( NEXUS_FrontendHandle handle, const NEXUS_FrontendSatellitePeakscanSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->satellitePeakscan )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_SatellitePeakscan(handle->pParentFrontend, pSettings);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->satellitePeakscan(handle->pDeviceHandle, pSettings);
    }
}

NEXUS_Error NEXUS_Frontend_GetSatellitePeakscanResult( NEXUS_FrontendHandle handle, NEXUS_FrontendSatellitePeakscanResult *pResult )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getSatellitePeakscanResult )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetSatellitePeakscanResult(handle->pParentFrontend, pResult);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getSatellitePeakscanResult(handle->pDeviceHandle, pResult);
    }
}

NEXUS_Error NEXUS_Frontend_SatelliteToneSearch( NEXUS_FrontendHandle handle, const NEXUS_FrontendSatelliteToneSearchSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->satelliteToneSearch )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_SatelliteToneSearch(handle->pParentFrontend, pSettings);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->satelliteToneSearch(handle->pDeviceHandle, pSettings);
    }
}

NEXUS_Error NEXUS_Frontend_GetSatelliteToneSearchResult( NEXUS_FrontendHandle handle, NEXUS_FrontendSatelliteToneSearchResult *pResult )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getSatelliteToneSearchResult )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetSatelliteToneSearchResult(handle->pParentFrontend, pResult);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getSatelliteToneSearchResult(handle->pDeviceHandle, pResult);
    }
}

NEXUS_Error NEXUS_Frontend_GetSatelliteSignalDetectStatus( NEXUS_FrontendHandle handle, NEXUS_FrontendSatelliteSignalDetectStatus *pStatus)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getSatelliteSignalDetectStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetSatelliteSignalDetectStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getSatelliteSignalDetectStatus(handle->pDeviceHandle, pStatus);
    }
}

static void NEXUS_Frontend_P_GetSatelliteDefaultStatus(NEXUS_FrontendSatelliteAgcStatus *pStatus)
{
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
}

NEXUS_Error NEXUS_Frontend_GetSatelliteAgcStatus(NEXUS_FrontendHandle handle, NEXUS_FrontendSatelliteAgcStatus *pStatus)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getSatelliteAgcStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetSatelliteAgcStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        NEXUS_Frontend_P_GetSatelliteDefaultStatus(pStatus);
        return handle->getSatelliteAgcStatus(handle->pDeviceHandle, pStatus);
    }
}

NEXUS_Error NEXUS_Frontend_GetSatelliteRuntimeSettings(NEXUS_FrontendHandle handle, NEXUS_FrontendSatelliteRuntimeSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getSatelliteRuntimeSettings )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetSatelliteRuntimeSettings(handle->pParentFrontend, pSettings);
        }
        else
        {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getSatelliteRuntimeSettings(handle->pDeviceHandle, pSettings);
    }
}

NEXUS_Error NEXUS_Frontend_SetSatelliteRuntimeSettings(NEXUS_FrontendHandle handle, const NEXUS_FrontendSatelliteRuntimeSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->setSatelliteRuntimeSettings )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_SetSatelliteRuntimeSettings(handle->pParentFrontend, pSettings);
        }
        else
        {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->setSatelliteRuntimeSettings(handle->pDeviceHandle, pSettings);
    }
}
