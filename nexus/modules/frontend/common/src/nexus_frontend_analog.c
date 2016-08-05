/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2004-2016 Broadcom. All rights reserved.
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Frontend Analog
*    Generic APIs for Analog tuning.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_frontend_module.h"

BDBG_MODULE(nexus_frontend_analog);

/***************************************************************************
Summary:
	Initialize an analog settings structure to defaults
See Also:
    NEXUS_Frontend_TuneAnalog
 ***************************************************************************/
void NEXUS_Frontend_GetDefaultAnalogSettings(
    NEXUS_FrontendAnalogSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_Ifd_P_GetDefaultSettings(&pSettings->ifdSettings);
}

/***************************************************************************
Summary:
	Tune a frontend to an analog channel
See Also:
    NEXUS_Frontend_GetAnalogStatus
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_TuneAnalog(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendAnalogSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->tuneAnalog )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_TuneAnalog(handle->pParentFrontend, pSettings);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->tuneAnalog(handle->pDeviceHandle, pSettings); 
    }
}

/***************************************************************************
Summary:
	Get the analog status of a frontend
See Also:
    NEXUS_Frontend_TuneAnalog
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_GetAnalogStatus(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendAnalogStatus *pStatus /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pStatus);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->getAnalogStatus )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetAnalogStatus(handle->pParentFrontend, pStatus);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->getAnalogStatus(handle->pDeviceHandle, pStatus); 
    }
}

/***************************************************************************
Summary:
	Get the analog video connector from a frontend
Description:
    This function is valid only for internal IF demodulators, and allows
    the decoded video output to be routed directly to the VDEC.
    This routine will return NULL if no direct connection exists between
    the IFD and the audio hardware, in which case an external connection
    such as Ccir656Input will be needed.
See Also:
    NEXUS_AudioInput_Shutdown
 ***************************************************************************/
NEXUS_VideoInputHandle NEXUS_Frontend_GetAnalogVideoConnector(
    NEXUS_FrontendHandle handle
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return NULL;
    }

    if ( NULL == handle->getAnalogVideoConnector )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetAnalogVideoConnector(handle->pParentFrontend);
        }
        else
        {
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            return NULL;
        }
    }
    else
    {
        return handle->getAnalogVideoConnector(handle->pDeviceHandle);
    }
}


/***************************************************************************
Summary:
	Get the analog audio connector from a frontend
Description:
    This function is valid only for internal IF demodulators, and allows
    the decoded audio output to be routed directly to the audio module.
    This routine will return NULL if no direct connection exists between
    the IFD and the audio hardware, in which case an external connection
    such as I2sInput will be needed.
See Also:
    NEXUS_AudioInput_Shutdown
 ***************************************************************************/
NEXUS_AudioInputHandle NEXUS_Frontend_GetAnalogAudioConnector(
    NEXUS_FrontendHandle handle
    )
{
    BDBG_ASSERT(NULL != handle);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return NULL;
    }

    if ( NULL == handle->getAnalogAudioConnector )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_GetAnalogAudioConnector(handle->pParentFrontend);
        }
        else
        {
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            return NULL;
        }
    }
    else
    {
        return handle->getAnalogAudioConnector(handle->pDeviceHandle);
    }
}

/***************************************************************************
Summary:
    Get Analog AGC Settings
***************************************************************************/
void NEXUS_Frontend_GetAnalogAgcSettings(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendAnalogAgcSettings *pSettings      /* [out] */
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        BDBG_ERR(("Device open failed. Cannot get analog agc settings."));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return;
    }

    if ( handle->getAnalogAgcSettings )
    {
        handle->getAnalogAgcSettings(handle->pDeviceHandle, pSettings);
    }
    else if ( handle->pParentFrontend )
    {
        NEXUS_Frontend_GetAnalogAgcSettings(handle->pParentFrontend, pSettings);
    }
}

/***************************************************************************
Summary:
    Set Analog AGC Settings
***************************************************************************/
NEXUS_Error NEXUS_Frontend_SetAnalogAgcSettings(
    NEXUS_FrontendHandle handle,
    const NEXUS_FrontendAnalogAgcSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( NULL == handle->setAnalogAgcSettings )
    {
        if ( handle->pParentFrontend )
        {
            return NEXUS_Frontend_SetAnalogAgcSettings(handle->pParentFrontend, pSettings);
        }
        else
        {
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        return handle->setAnalogAgcSettings(handle->pDeviceHandle, pSettings); 
    }
}

/* Hook for VDEC to inform IFD of current video format */
void NEXUS_Frontend_NotifyAnalogVideoFormat_priv(
    NEXUS_FrontendHandle handle,
    NEXUS_VideoFormat format)
{
    BDBG_ASSERT(NULL != handle);
    NEXUS_ASSERT_MODULE();

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        BDBG_ERR(("Device open failed. Cannot notify analog video format."));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return;
    }

    if ( handle->notifyAnalogVideoFormat )
    {
        handle->notifyAnalogVideoFormat(handle->pDeviceHandle, format);
    }
    else if ( handle->pParentFrontend )
    {
        NEXUS_Frontend_NotifyAnalogVideoFormat_priv(handle->pParentFrontend, format);
    }
}

/* Hook for AASD to retrieve current video format */
void NEXUS_Frontend_GetAnalogVideoFormat_priv(
    NEXUS_FrontendHandle handle,
    NEXUS_VideoFormat *pFormat)
{
    BDBG_ASSERT(NULL != handle);
    NEXUS_ASSERT_MODULE();

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        BDBG_ERR(("Device open failed. Cannot get analog video format."));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return;
    }

    if ( handle->getAnalogVideoFormat )
    {
        handle->getAnalogVideoFormat(handle->pDeviceHandle, pFormat);
    }
    else if ( handle->pParentFrontend )
    {
        NEXUS_Frontend_GetAnalogVideoFormat_priv(handle->pParentFrontend, pFormat);
    }
}

/* Hook for AASD to drive format switches */
NEXUS_Error NEXUS_Frontend_SetAnalogAVFormat_priv(
    NEXUS_FrontendHandle handle,
    NEXUS_VideoFormat videoFormat,
    NEXUS_IfdAudioMode audioMode,
    bool carriersOnly)
{
    BDBG_ASSERT(NULL != handle);
    NEXUS_ASSERT_MODULE();

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);
    }

    if ( handle->setAnalogAVFormat )
    {
        return handle->setAnalogAVFormat(handle->pDeviceHandle, videoFormat, audioMode, carriersOnly);
    }
    else if ( handle->pParentFrontend )
    {
        NEXUS_Frontend_SetAnalogAVFormat_priv(handle->pParentFrontend, videoFormat, audioMode, carriersOnly);
    }

    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_Frontend_SetAnalogAudioInterrupt_priv(
    NEXUS_FrontendHandle handle,
    NEXUS_FrontendAnalogAudioInterruptFunc interruptFunc_isr,
    void *pFuncParam)
{
    BDBG_ASSERT(NULL != handle);
    NEXUS_ASSERT_MODULE();

    if(NEXUS_Frontend_P_CheckDeviceOpen(handle)){
        BDBG_ERR(("Device open failed. Cannot set analog audio interrupt."));
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return;
    }

    if ( handle->setAnalogAudioInterrupt )
    {
        handle->setAnalogAudioInterrupt(handle->pDeviceHandle, interruptFunc_isr, pFuncParam);
    }
    else if ( handle->pParentFrontend )
    {
        NEXUS_Frontend_SetAnalogAudioInterrupt_priv(handle->pParentFrontend, interruptFunc_isr, pFuncParam);
    }
}
