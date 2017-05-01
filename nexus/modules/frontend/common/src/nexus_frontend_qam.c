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

BDBG_MODULE(nexus_frontend_qam);

BTRC_MODULE_DECLARE(ChnChange_Tune);

/***************************************************************************
Summary:
    Initialize a QAM settings structure to defaults
See Also:
    NEXUS_Frontend_TuneQam
 ***************************************************************************/
void NEXUS_Frontend_GetDefaultQamSettings(
    NEXUS_FrontendQamSettings *pSettings
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->mode = NEXUS_FrontendQamMode_e64;
    pSettings->annex = NEXUS_FrontendQamAnnex_eB;
    pSettings->symbolRate = 5056900;
    pSettings->bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
    pSettings->spectrumMode = NEXUS_FrontendQamSpectrumMode_eAuto;
    pSettings->frequencyOffset = NEXUS_FrontendQamFrequencyOffset_e180Khz;
    pSettings->acquisitionMode = NEXUS_FrontendQamAcquisitionMode_eAuto;
    pSettings->autoAcquire = true;
    pSettings->scan.upperBaudSearch = 5360537;
    pSettings->scan.lowerBaudSearch = 5056941;
    pSettings->scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e64] = true;
    pSettings->scan.mode[NEXUS_FrontendQamAnnex_eB][NEXUS_FrontendQamMode_e256] = true;
    pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e64] = true;
    pSettings->scan.mode[NEXUS_FrontendQamAnnex_eA][NEXUS_FrontendQamMode_e256] = true;
    NEXUS_CallbackDesc_Init(&pSettings->lockCallback);
    NEXUS_CallbackDesc_Init(&pSettings->asyncStatusReadyCallback);
}

/***************************************************************************
Summary:
    Tune to a QAM channel
See Also:
    NEXUS_Frontend_InitQamSettings
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_TuneQam(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendQamSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);
    BTRC_TRACE(ChnChange_Tune, START);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }
    NEXUS_Frontend_P_SetTuned(handle);

    if ( NULL == handle->tuneQam )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_TuneQam(handle->pParentFrontend, pSettings);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->tuneQam(handle->pDeviceHandle, pSettings);
    }
}

/***************************************************************************
Summary:
    Get the status synchronously of a QAM tuner
See Also:
    NEXUS_Frontend_TuneQam
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetQamStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendQamStatus *pStatus /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pStatus);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getQamStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetQamStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getQamStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Get the status asynchronously of a QAM tuner
See Also:
    NEXUS_Frontend_TuneQam
    NEXUS_Frontend_RequestQamStatus
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetQamAsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendQamStatus *pStatus /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pStatus);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getQamAsyncStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetQamAsyncStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getQamAsyncStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Request the status asynchronously of a QAM tuner
See Also:
    NEXUS_Frontend_GetQamAsyncStatus
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestQamAsyncStatus(
    NEXUS_FrontendHandle handle
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->requestQamAsyncStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_RequestQamAsyncStatus(handle->pParentFrontend);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->requestQamAsyncStatus(handle->pDeviceHandle);
    }
}

/***************************************************************************
Summary:
    Request the status asynchronously of a QAM tuner
See Also:
    NEXUS_Frontend_GetQamAsyncStatus
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetQamScanStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendQamScanStatus *pScanStatus /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getQamScanStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetQamScanStatus(handle->pParentFrontend, pScanStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getQamScanStatus(handle->pDeviceHandle, pScanStatus);
    }
}

unsigned NEXUS_Frontend_P_GetDefaultQamSymbolRate(NEXUS_FrontendQamMode mode, NEXUS_FrontendQamAnnex annex)
{
    if (annex == NEXUS_FrontendQamAnnex_eA) {
        switch (mode) {
        case NEXUS_FrontendQamMode_e64:
        case NEXUS_FrontendQamMode_e256:
            return 6952000;
        case NEXUS_FrontendQamMode_e16:
        case NEXUS_FrontendQamMode_e32:
        case NEXUS_FrontendQamMode_e128:
        default:
            return 6952000; /* TODO */
        }

    }
    else {
        switch (mode) {
        case NEXUS_FrontendQamMode_e64:
            return 5056941;
        case NEXUS_FrontendQamMode_e256:
            return 5360537;
        case NEXUS_FrontendQamMode_e16:
        case NEXUS_FrontendQamMode_e32:
        case NEXUS_FrontendQamMode_e128:
        default:
            return 5056941; /* TODO */
        }
    }
}
