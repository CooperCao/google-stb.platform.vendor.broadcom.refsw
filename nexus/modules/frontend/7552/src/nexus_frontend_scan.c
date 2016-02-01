/***************************************************************************
*     (c)2010-2011 Broadcom Corporation
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
*   API name: Frontend channel scan
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

/* base was /nexus/modules/frontend/35230/src/nexus_frontend_scan.c@@/main/1 */
/* TODO: this could be a symlink */

#include "nexus_frontend_module.h"
#include "nexus_frontend_scan.h"

BDBG_MODULE(nexus_frontend_scan);

/***************************************************************************
Summary:
    Get default options for NEXUS_Frontend_ScanFrequency
***************************************************************************/
void NEXUS_Frontend_GetDefaultScanSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendScanSettings *pSettings   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    if ( handle->capabilities.scan )
    {
        if ( handle->capabilities.vsb )
        {
            pSettings->signalTypes[NEXUS_FrontendSignalType_eVsb] = true;
        }
        if ( handle->capabilities.qam )
        {
            pSettings->signalTypes[NEXUS_FrontendSignalType_eQam] = true;
        }
        if ( handle->capabilities.analog )
        {
            pSettings->signalTypes[NEXUS_FrontendSignalType_eAnalog] = true;
        }
        if ( handle->capabilities.ofdm )
        {
            pSettings->signalTypes[NEXUS_FrontendSignalType_eDvbt] = true;
            if ( handle->capabilities.ofdmModes[NEXUS_FrontendOfdmMode_eIsdbt] )
            {
                pSettings->signalTypes[NEXUS_FrontendSignalType_eIsdbt] = true;
            }
        }
    }
}

/***************************************************************************
Summary:
    Scan a frequency for specified signal types

Description:
    This is a blocking function.  It will return NEXUS_SUCCESS if a signal
    is detected matching the specified settings.  If no signal is
    detected, NEXUS_TIMEOUT will be returned.
***************************************************************************/
NEXUS_Error NEXUS_Frontend_ScanFrequency(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendScanSettings *pSettings,
    NEXUS_FrontendScanResults *pResults             /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pResults);

    if ( NULL == handle->scanFrequency )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_ScanFrequency(handle->pParentFrontend, pSettings, pResults);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }

    return handle->scanFrequency(handle->pDeviceHandle, pSettings, pResults);
}

/***************************************************************************
Summary:
    Get thresholds for NEXUS_Frontend_ScanFrequency operation
***************************************************************************/
void NEXUS_Frontend_GetScanThresholds(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendScanThresholds *pThresholds   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pThresholds);

    if ( handle->getScanThresholds )
    {
        handle->getScanThresholds(handle->pDeviceHandle, pThresholds);
    }
    else if ( handle->pParentFrontend )
    {
        NEXUS_Frontend_GetScanThresholds(handle->pParentFrontend, pThresholds);
    }
}

/***************************************************************************
Summary:
    Set thresholds for NEXUS_Frontend_ScanFrequency operation
***************************************************************************/
NEXUS_Error NEXUS_Frontend_SetScanThresholds(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendScanThresholds *pThresholds
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pThresholds);

    if ( NULL == handle->setScanThresholds )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_SetScanThresholds(handle->pParentFrontend, pThresholds);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->setScanThresholds(handle->pDeviceHandle, pThresholds);
    }
}

