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
*   API name: Frontend VSB
*    Generic APIs for VSB tuning.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_frontend_module.h"

BDBG_MODULE(nexus_frontend_vsb);

BTRC_MODULE_DECLARE(ChnChange_Tune);

/***************************************************************************
Summary:
	Initialize a VSB settings structure to defaults
See Also:
    NEXUS_Frontend_TuneVsb
 ***************************************************************************/
void NEXUS_Frontend_GetDefaultVsbSettings(
    NEXUS_FrontendVsbSettings *pSettings
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->autoAcquire = true;
    pSettings->terrestrial = true;
    pSettings->ifFrequency = 44000000; /* 44 MHz*/
}

/***************************************************************************
Summary:
	Tune to a VSB channel
See Also:
    NEXUS_Frontend_InitVsbSettings
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_TuneVsb(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendVsbSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);
    BTRC_TRACE(ChnChange_Tune, START);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    NEXUS_Time_Get(&handle->resetStatusTime);

    if ( NULL == handle->tuneVsb )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_TuneVsb(handle->pParentFrontend, pSettings);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->tuneVsb(handle->pDeviceHandle, pSettings);
    }
}

/***************************************************************************
Summary:
	Get the status of a VSB tuner
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetVsbStatus( NEXUS_FrontendHandle handle, NEXUS_FrontendVsbStatus *pStatus )
{
    NEXUS_Error rc;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pStatus);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getVsbStatus )
    {
        if ( handle->pParentFrontend )
        {
            rc = NEXUS_Frontend_GetVsbStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        rc = handle->getVsbStatus(handle->pDeviceHandle, pStatus);
    }
    if (!rc) {
        NEXUS_Time now;
        NEXUS_Time_Get(&now);
        pStatus->timeElapsed = NEXUS_Time_Diff(&now, &handle->resetStatusTime);
    }
    return rc;
}

/***************************************************************************
Summary:
    Request the status asynchronously of a VSB tuner
See Also:
    NEXUS_Frontend_GetVsbAsyncStatus
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_RequestVsbAsyncStatus( NEXUS_FrontendHandle handle )
{
    NEXUS_Error rc;

    BDBG_ASSERT(NULL != handle);
    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->requestVsbAsyncStatus )
    {
        if ( handle->pParentFrontend )
        {
            rc = NEXUS_Frontend_RequestVsbAsyncStatus(handle->pParentFrontend);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        rc = handle->requestVsbAsyncStatus(handle->pDeviceHandle);
    }

    return rc;
}

/***************************************************************************
Summary:
    Get the status asynchronously of a VSB tuner
See Also:
    NEXUS_Frontend_TuneVsb
    NEXUS_Frontend_RequestVsbStatus
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetVsbAsyncStatus( NEXUS_FrontendHandle handle, NEXUS_FrontendVsbStatus *pStatus )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pStatus);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getVsbAsyncStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetVsbAsyncStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getVsbAsyncStatus(handle->pDeviceHandle, pStatus);
    }
}
