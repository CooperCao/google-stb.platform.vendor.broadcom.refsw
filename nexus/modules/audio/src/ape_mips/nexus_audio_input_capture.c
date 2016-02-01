/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
*   API name: Audio Module
*    Module includes
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_input_capture);


/***************************************************************************
Summary:
Get default Open-time settings for AudioInputCapture
***************************************************************************/
void NEXUS_AudioInputCapture_GetDefaultOpenSettings(
    NEXUS_AudioInputCaptureOpenSettings *pSettings  /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
Open an AudioInputCapture handle
***************************************************************************/
NEXUS_AudioInputCaptureHandle NEXUS_AudioInputCapture_Open(
    unsigned index,
    const NEXUS_AudioInputCaptureOpenSettings *pSettings
    )
{
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

/***************************************************************************
Summary:
Close an AudioInputCapture handle
***************************************************************************/
void NEXUS_AudioInputCapture_Close(
    NEXUS_AudioInputCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
}

/***************************************************************************
Summary:
Get Default Start-time settings for AudioInputCapture
***************************************************************************/
void NEXUS_AudioInputCapture_GetDefaultStartSettings(
    NEXUS_AudioInputCaptureStartSettings *pSettings /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
Start capturing input data
***************************************************************************/
NEXUS_Error NEXUS_AudioInputCapture_Start(
    NEXUS_AudioInputCaptureHandle handle,
    const NEXUS_AudioInputCaptureStartSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


/***************************************************************************
Summary:
Stop capturing input data
***************************************************************************/
void NEXUS_AudioInputCapture_Stop(
    NEXUS_AudioInputCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
}

/***************************************************************************
Summary:
Get current settings for AudioInputCapture
***************************************************************************/
void NEXUS_AudioInputCapture_GetSettings(
    NEXUS_AudioInputCaptureHandle handle,
    NEXUS_AudioInputCaptureSettings *pSettings  /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
Set settings for AudioInputCapture
***************************************************************************/
NEXUS_Error NEXUS_AudioInputCapture_SetSettings(
    NEXUS_AudioInputCaptureHandle handle,
    const NEXUS_AudioInputCaptureSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Get current status for AudioInputCapture
***************************************************************************/
NEXUS_Error NEXUS_AudioInputCapture_GetStatus(
    NEXUS_AudioInputCaptureHandle handle,
    NEXUS_AudioInputCaptureStatus *pStatus  /* [out] */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/***************************************************************************
Summary:
Get audio connector to connect InputCapture to other objects
***************************************************************************/
NEXUS_AudioInput NEXUS_AudioInputCapture_GetConnector(
    NEXUS_AudioInputCaptureHandle handle
    )
{
    BSTD_UNUSED(handle);
    return NULL;
}

NEXUS_Error NEXUS_AudioInputCapture_GetBuffer(
    NEXUS_AudioInputCaptureHandle handle,
    void **ppBuffer,    /* [out] attr{memory=cached} pointer to memory mapped
                                 region that contains captured data. */
    size_t *pSize       /* [out] total number of readable, contiguous bytes which the buffers are pointing to */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(ppBuffer);
    BSTD_UNUSED(pSize);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioInputCapture_ReadComplete(
    NEXUS_AudioInputCaptureHandle handle,
    size_t amountWritten            /* The number of bytes read from the buffer */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(amountWritten);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

