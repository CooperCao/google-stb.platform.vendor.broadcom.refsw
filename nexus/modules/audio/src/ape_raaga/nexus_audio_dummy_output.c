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
*   API name: AudioDummyOutput
*    Specific APIs related to audio dummy outputs.  Dummy outputs
*    are used to consume audio data without outputting to a physical
*    output such as I2S or DAC.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_dummy_output);

typedef struct NEXUS_AudioDummyOutput
{
    NEXUS_OBJECT(NEXUS_AudioDummyOutput);
    unsigned index;
    BAPE_DummyOutputHandle apeHandle;
    NEXUS_AudioOutputObject connector;
    NEXUS_AudioDummyOutputSettings settings;
    char name[8];   /* DUMMY %d */
} NEXUS_AudioDummyOutput;

/***************************************************************************
Summary:
Get default settings for an Audio Dummy Output
***************************************************************************/
void NEXUS_AudioDummyOutput_GetDefaultSettings(
    NEXUS_AudioDummyOutputSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    pSettings->maxMultichannelFormat = NEXUS_AudioMultichannelFormat_eNone;
}

/***************************************************************************
Summary:
Open an Audio Dummy Output
***************************************************************************/
NEXUS_AudioDummyOutputHandle NEXUS_AudioDummyOutput_Open( /* attr{destructor=NEXUS_AudioDummyOutput_Close}  */
    unsigned index,
    const NEXUS_AudioDummyOutputSettings *pSettings     /* attr{null_allowed=y} */
    )
{
    NEXUS_AudioDummyOutputHandle handle;
    NEXUS_AudioDummyOutputSettings defaults;
    BAPE_DummyOutputOpenSettings openSettings; 
    BAPE_OutputPort outputPort;
    BERR_Code errCode;

    if ( NULL == pSettings )
    {
        NEXUS_AudioDummyOutput_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

    BAPE_DummyOutput_GetDefaultOpenSettings(&openSettings);
    switch ( pSettings->maxMultichannelFormat )
    {
    default:
    case NEXUS_AudioMultichannelFormat_eNone:
        openSettings.maxMultichannelFormat = BAPE_MultichannelFormat_e2_0;
        break;
    case NEXUS_AudioMultichannelFormat_e5_1:
        openSettings.maxMultichannelFormat = BAPE_MultichannelFormat_e5_1;
        break;
    case NEXUS_AudioMultichannelFormat_e7_1:
        openSettings.maxMultichannelFormat = BAPE_MultichannelFormat_e7_1;
        break;
    }

    handle = BKNI_Malloc(sizeof(NEXUS_AudioDummyOutput));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc_handle;
    }
    NEXUS_OBJECT_INIT(NEXUS_AudioDummyOutput, handle);
    BKNI_Snprintf(handle->name, sizeof(handle->name), "DUMMY %u", index);
    NEXUS_AUDIO_OUTPUT_INIT(&handle->connector, NEXUS_AudioOutputType_eDummy, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioOutput, &handle->connector, Open);
    handle->connector.pName = handle->name;        
    handle->index = index;

    errCode = BAPE_DummyOutput_Open(NEXUS_AUDIO_DEVICE_HANDLE, index, &openSettings, &handle->apeHandle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_open_output;
    }
    
    BAPE_DummyOutput_GetOutputPort(handle->apeHandle, &outputPort);
    handle->connector.port = (size_t)outputPort;
    handle->settings = *pSettings;

    return handle;

err_open_output:
    NEXUS_OBJECT_DESTROY(NEXUS_AudioDummyOutput, handle);
    BKNI_Free(handle);
err_alloc_handle:
    return NULL;
}

/***************************************************************************
Summary:
Close an Audio Dummy Output
    
Description:
Inputs must be removed prior to closing.
***************************************************************************/
static void NEXUS_AudioDummyOutput_P_Finalizer(
    NEXUS_AudioDummyOutputHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioDummyOutput, handle);
    NEXUS_AudioOutput_Shutdown(&handle->connector);
    BAPE_DummyOutput_Close(handle->apeHandle);
    NEXUS_OBJECT_DESTROY(NEXUS_AudioDummyOutput, handle);
    BKNI_Free(handle);    
}

static void NEXUS_AudioDummyOutput_P_Release(NEXUS_AudioDummyOutputHandle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioOutput, &handle->connector, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioDummyOutput, NEXUS_AudioDummyOutput_Close);

/***************************************************************************
Summary:
Get Settings for an Audio Dummy Output
***************************************************************************/
void NEXUS_AudioDummyOutput_GetSettings(
    NEXUS_AudioDummyOutputHandle handle,
    NEXUS_AudioDummyOutputSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDummyOutput);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

/***************************************************************************
Summary:
Set Settings for an Audio Dummy Output
***************************************************************************/
NEXUS_Error NEXUS_AudioDummyOutput_SetSettings(
    NEXUS_AudioDummyOutputHandle handle,
    const NEXUS_AudioDummyOutputSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDummyOutput);
    BDBG_ASSERT(NULL != pSettings);
    if ( handle->settings.maxMultichannelFormat != pSettings->maxMultichannelFormat )
    {
        BDBG_ERR(("Multichannel format can only be set prior to NEXUS_AudioDummyOutput_Open"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    handle->settings = *pSettings;
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Get the audio connector for an Audio Dummy Output
***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioDummyOutput_GetConnector(
    NEXUS_AudioDummyOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioDummyOutput);
    return &handle->connector;
}

