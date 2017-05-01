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
*   API name: Frontend OFDM
*    Generic APIs for OFDM (Orthogonal Frequency-Division Multiplexing) tuning.
*    This is used in DVB-H and DVB-T environments.
*
***************************************************************************/

#include "nexus_frontend_module.h"

BDBG_MODULE(nexus_frontend_ofdm);

BTRC_MODULE_DECLARE(ChnChange_Tune);

/***************************************************************************
Summary:
    Initialize a OFDM settings structure to defaults
See Also:
    NEXUS_Frontend_TuneOfdm
 ***************************************************************************/
void NEXUS_Frontend_GetDefaultOfdmSettings(
    NEXUS_FrontendOfdmSettings *pSettings   /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_FrontendOfdmSettings));
    pSettings->bandwidth = NEXUS_FrontendOfdmBandwidth_e8Mhz;
    pSettings->pullInRange = NEXUS_FrontendOfdmPullInRange_eWide;
    pSettings->cciMode = NEXUS_FrontendOfdmCciMode_eAuto;
    pSettings->terrestrial = true;
    pSettings->acquisitionMode = NEXUS_FrontendOfdmAcquisitionMode_eAuto;
    pSettings->dvbt2Settings.profile = NEXUS_FrontendDvbt2Profile_eBase;
    pSettings->spectrumMode = NEXUS_FrontendOfdmSpectrumMode_eManual;
    NEXUS_CallbackDesc_Init(&pSettings->lockCallback);
    NEXUS_CallbackDesc_Init(&pSettings->asyncStatusReadyCallback);
    NEXUS_CallbackDesc_Init(&pSettings->ewsCallback);
}

/***************************************************************************
Summary:
    Tune to a OFDM channel
See Also:
    NEXUS_Frontend_InitOfdmSettings
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_TuneOfdm(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendOfdmSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);
    BTRC_TRACE(ChnChange_Tune, START);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }
    NEXUS_Frontend_P_SetTuned(handle);

    if ( handle->tuneOfdm )
    {
        return handle->tuneOfdm(handle->pDeviceHandle, pSettings);
    }
    else
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_TuneOfdm(handle->pParentFrontend, pSettings);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
}

/***************************************************************************
Summary:
    Get the status of a OFDM tuner
See Also:
    NEXUS_Frontend_TuneOfdm
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetOfdmStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendOfdmStatus *pStatus /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( handle->getOfdmStatus )
    {
        return handle->getOfdmStatus(handle->pDeviceHandle, pStatus);
    }
    else
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetOfdmStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
}

/***************************************************************************
Summary:
    Get the status asynchronously of a Ofdm tuner
See Also:
    NEXUS_Frontend_TuneOfdm
    NEXUS_Frontend_RequestOfdmStatus
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetOfdmAsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendOfdmStatus *pStatus /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pStatus);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getOfdmAsyncStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetOfdmAsyncStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getOfdmAsyncStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Request the status asynchronously of a Ofdm tuner
See Also:
    NEXUS_Frontend_GetOfdmAsyncStatus
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestOfdmAsyncStatus(
    NEXUS_FrontendHandle handle
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->requestOfdmAsyncStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_RequestOfdmAsyncStatus(handle->pParentFrontend);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->requestOfdmAsyncStatus(handle->pDeviceHandle);
    }
}

/***************************************************************************
Summary:
    Request the dvbt2 asynchronous status of NEXUS_FrontendDvbt2AsyncStatus type.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestDvbt2AsyncStatus(
    NEXUS_FrontendHandle handle, 
    NEXUS_FrontendDvbt2StatusType type
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->requestDvbt2AsyncStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_RequestDvbt2AsyncStatus(handle->pParentFrontend, type);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->requestDvbt2AsyncStatus(handle->pDeviceHandle, type);
    }
}

/***************************************************************************
Summary:
    Get the dvbt2 asynchronous status ready type.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbt2AsyncStatusReady(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbt2StatusReady *pAsyncStatusReady
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbt2AsyncStatusReady )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbt2AsyncStatusReady(handle->pParentFrontend, pAsyncStatusReady);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbt2AsyncStatusReady(handle->pDeviceHandle, pAsyncStatusReady);
    }
}

/***************************************************************************
Summary:
    Get the dvbt2 asynchronous FEC statisitics
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbt2AsyncFecStatistics (
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbt2StatusType type,
    NEXUS_FrontendDvbt2FecStatistics *pStatistics /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbt2AsyncFecStatistics )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbt2AsyncFecStatistics(handle->pParentFrontend, type, pStatistics);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbt2AsyncFecStatistics(handle->pDeviceHandle, type, pStatistics);
    }
}

/***************************************************************************
Summary:
    Get the dvbt2 asynchronous L1 PRE status
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbt2AsyncL1PreStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbt2L1PreStatus   *pStatus
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbt2AsyncL1PreStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbt2AsyncL1PreStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbt2AsyncL1PreStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Get the dvbt2 L1 asynchronous Post Configurable status
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbt2AsyncL1PostConfigurableStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbt2L1PostConfigurableStatus *pStatus   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbt2AsyncL1PostConfigurableStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbt2AsyncL1PostConfigurableStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbt2AsyncL1PostConfigurableStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Get the dvbt2 asynchronous Post Dynamic status
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbt2AsyncPostDynamicStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbt2L1PostDynamicStatus *pStatus   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbt2AsyncPostDynamicStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbt2AsyncPostDynamicStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbt2AsyncPostDynamicStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Get the dvbt2 asynchronous L1 PLP status
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbt2AsyncL1PlpStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbt2L1PlpStatus *pStatus   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbt2AsyncL1PlpStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbt2AsyncL1PlpStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbt2AsyncL1PlpStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Get the dvbt2 asynchronous basic status
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbt2AsyncBasicStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbt2BasicStatus *pStatus
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbt2AsyncBasicStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbt2AsyncBasicStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbt2AsyncBasicStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Request the dvbc2 asynchronous status of NEXUS_FrontendDvbc2AsyncStatus type.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestDvbc2AsyncStatus(
    NEXUS_FrontendHandle handle, 
    NEXUS_FrontendDvbc2StatusType type
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->requestDvbc2AsyncStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_RequestDvbc2AsyncStatus(handle->pParentFrontend, type);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->requestDvbc2AsyncStatus(handle->pDeviceHandle, type);
    }
}
/***************************************************************************
Summary:
    Get the dvbc2 asynchronous status ready type.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncStatusReady(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2StatusReady *pAsyncStatusReady
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbc2AsyncStatusReady )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbc2AsyncStatusReady(handle->pParentFrontend, pAsyncStatusReady);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbc2AsyncStatusReady(handle->pDeviceHandle, pAsyncStatusReady);
    }
}

/***************************************************************************
Summary:
    Get the dvbc2 asynchronous FEC statistics
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncFecStatistics (
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2StatusType type,
    NEXUS_FrontendDvbc2FecStatistics *pStatistics /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbc2AsyncFecStatistics )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbc2AsyncFecStatistics(handle->pParentFrontend, type, pStatistics);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbc2AsyncFecStatistics (handle->pDeviceHandle, type, pStatistics);
    }
}

/***************************************************************************
Summary:
    Get the dvbc2 asynchronous L1 Part2 asynchronous status
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncL1Part2Status(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2L1Part2Status   *pStatus
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbc2AsyncL1Part2Status )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbc2AsyncL1Part2Status(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbc2AsyncL1Part2Status(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Get the dvbc2 asynchronous L1 Dslice asynchronous status
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncL1DsliceStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2L1DsliceStatus *pStatus   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbc2AsyncL1DsliceStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbc2AsyncL1DsliceStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbc2AsyncL1DsliceStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Get the dvbc2 asynchronous L1 Notch asynchronous status
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncL1NotchStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2L1NotchStatus *pStatus   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbc2AsyncL1NotchStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbc2AsyncL1NotchStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbc2AsyncL1NotchStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Get the dvbt2 asynchronous L1 PLP status
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncL1PlpStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2L1PlpStatus *pStatus   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbc2AsyncL1PlpStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbc2AsyncL1PlpStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbc2AsyncL1PlpStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Get the dvbt2 asynchronous basic status
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbc2AsyncBasicStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbc2BasicStatus *pStatus
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbc2AsyncBasicStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbc2AsyncBasicStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbc2AsyncBasicStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
    Request the Dvbt asynchronous status of NEXUS_FrontendDvbtAsyncStatus type.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestDvbtAsyncStatus(
    NEXUS_FrontendHandle handle, 
    NEXUS_FrontendDvbtStatusType type
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->requestDvbtAsyncStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_RequestDvbtAsyncStatus(handle->pParentFrontend, type);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->requestDvbtAsyncStatus(handle->pDeviceHandle, type);
    }
}
/***************************************************************************
Summary:
    Get the Dvbt asynchronous status ready type.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbtAsyncStatusReady(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbtStatusReady *pAsyncStatusReady
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbtAsyncStatusReady )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbtAsyncStatusReady(handle->pParentFrontend, pAsyncStatusReady);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbtAsyncStatusReady(handle->pDeviceHandle, pAsyncStatusReady);
    }
}

/***************************************************************************
Summary:
    Get the Dvbt asynchronous status.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetDvbtAsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendDvbtStatusType type, 
    NEXUS_FrontendDvbtStatus *pStatus
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getDvbtAsyncStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetDvbtAsyncStatus(handle->pParentFrontend, type, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getDvbtAsyncStatus(handle->pDeviceHandle, type, pStatus);
    }
}

/***************************************************************************
Summary:
    Request the Isdbt asynchronous status of NEXUS_FrontendIsdbtAsyncStatus type.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestIsdbtAsyncStatus(
    NEXUS_FrontendHandle handle, 
    NEXUS_FrontendIsdbtStatusType type
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->requestIsdbtAsyncStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_RequestIsdbtAsyncStatus(handle->pParentFrontend, type);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->requestIsdbtAsyncStatus(handle->pDeviceHandle, type);
    }
}
/***************************************************************************
Summary:
    Get the Isdbt asynchronous status ready type.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetIsdbtAsyncStatusReady(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendIsdbtStatusReady *pAsyncStatusReady
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getIsdbtAsyncStatusReady )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetIsdbtAsyncStatusReady(handle->pParentFrontend, pAsyncStatusReady);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getIsdbtAsyncStatusReady(handle->pDeviceHandle, pAsyncStatusReady);
    }
}
/***************************************************************************
Summary:
    Get the Isdbt asynchronous status.
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetIsdbtAsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendIsdbtStatusType type, 
    NEXUS_FrontendIsdbtStatus *pStatus
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getIsdbtAsyncStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetIsdbtAsyncStatus(handle->pParentFrontend, type, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getIsdbtAsyncStatus(handle->pDeviceHandle, type, pStatus);
    }
}

