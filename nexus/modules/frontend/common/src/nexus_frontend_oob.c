/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
* API Description:
*   API name: Frontend OutOfBand
*    Generic APIs for Out-of-band tuning.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_frontend_module.h"

BDBG_MODULE(nexus_frontend_oob);

BTRC_MODULE_DECLARE(ChnChange_Tune);

/***************************************************************************
Summary:
	Initialize an out-of-band settings structure to defaults
See Also:
    NEXUS_Frontend_TuneOutOfBand
 ***************************************************************************/
void NEXUS_Frontend_GetDefaultOutOfBandSettings(
    NEXUS_FrontendOutOfBandSettings *pSettings
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->mode = NEXUS_FrontendOutOfBandMode_ePod_Dvs178Qpsk;
    pSettings->spectrum = NEXUS_FrontendOutOfBandSpectrum_eAuto;
    pSettings->autoAcquire = true;
    pSettings->openDrain = true;
    pSettings->bertSource = NEXUS_FrontendOutOfBandBertSource_eFecOutput;
}

/***************************************************************************
Summary:
	Tune a frontend to an out-of-band channel
See Also:
    NEXUS_Frontend_InitOutOfBandSettings
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_TuneOutOfBand(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendOutOfBandSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);
    BTRC_TRACE(ChnChange_Tune, START);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->tuneOutOfBand )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_TuneOutOfBand(handle->pParentFrontend, pSettings);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->tuneOutOfBand(handle->pDeviceHandle, pSettings);
    }
}

/***************************************************************************
Summary:
	Get the out-of-band status synchronouslyof a frontend
See Also:
    NEXUS_Frontend_TuneOutOfBand
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetOutOfBandStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendOutOfBandStatus *pStatus /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pStatus);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getOutOfBandStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetOutOfBandStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getOutOfBandStatus(handle->pDeviceHandle, pStatus);
    }
}

/***************************************************************************
Summary:
	Get the out-of-band status asynchronously of a frontend
See Also:
    NEXUS_Frontend_TuneOutOfBand
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetOutOfBandAsyncStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendOutOfBandStatus *pStatus /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pStatus);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getOutOfBandStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetOutOfBandAsyncStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getOutOfBandAsyncStatus(handle->pDeviceHandle, pStatus);
    }
}


NEXUS_Error NEXUS_Frontend_RequestOutOfBandAsyncStatus(
    NEXUS_FrontendHandle handle
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->requestOutOfBandAsyncStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_RequestOutOfBandAsyncStatus(handle->pParentFrontend);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->requestOutOfBandAsyncStatus(handle->pDeviceHandle);
    }
}

