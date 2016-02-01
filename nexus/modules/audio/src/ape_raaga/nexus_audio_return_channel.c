/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
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
*   API name: Audio Return Channel (ARC)
*    Specific APIs related to Audio Return Channel output.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_return_channel);

typedef struct NEXUS_AudioReturnChannel
{
    NEXUS_OBJECT(NEXUS_AudioReturnChannel);
    bool opened;
    unsigned index;
    BAPE_AudioReturnChannelHandle bapeArcHandle;

    NEXUS_AudioReturnChannelSettings settings;
    NEXUS_AudioOutputObject connector;
    char name[6];   /* ARC %d */
} NEXUS_AudioReturnChannel;

#if NEXUS_NUM_AUDIO_RETURN_CHANNEL
static NEXUS_AudioReturnChannel g_arc[NEXUS_NUM_AUDIO_RETURN_CHANNEL];
#endif

/***************************************************************************
Summary:
    Get default settings for an Audio Return Channel Output
 ***************************************************************************/
void NEXUS_AudioReturnChannel_GetDefaultSettings(
    NEXUS_AudioReturnChannelSettings *pSettings   /* [out] default settings */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->muted = false;
}


/***************************************************************************
Summary:
    Open an Audio Return Channel Output
 ***************************************************************************/
NEXUS_AudioReturnChannelHandle NEXUS_AudioReturnChannel_Open(
    unsigned index,
    const NEXUS_AudioReturnChannelSettings *pSettings     /* Pass NULL for default settings */
    )
{
#if NEXUS_NUM_AUDIO_RETURN_CHANNEL
    NEXUS_AudioReturnChannelHandle handle;
    BAPE_AudioReturnChannelHandle bapeArcHandle;
    NEXUS_Error errCode;

    /* Sanity check */
    if (index >= NEXUS_NUM_AUDIO_RETURN_CHANNEL )
    {
        BDBG_ERR(("Arc Index %u out of range", index));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    handle = &g_arc[index];
    if ( handle->opened )
    {
        BDBG_ERR(("Arc %u already open", index));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    errCode = BAPE_AudioReturnChannel_Open( NEXUS_AUDIO_DEVICE_HANDLE, index, NULL, &bapeArcHandle );
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        return NULL;
    }

    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioReturnChannel));
    NEXUS_OBJECT_INIT(NEXUS_AudioReturnChannel, handle);
    handle->opened = true;
    handle->index = index;
    handle->bapeArcHandle = bapeArcHandle;

    /* Initialize connector */
    BKNI_Snprintf(handle->name, sizeof(handle->name), "ARC %u", index);
    NEXUS_AUDIO_OUTPUT_INIT(&handle->connector, NEXUS_AudioOutputType_eArc, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioOutput, &handle->connector, Open);
    handle->connector.pName = handle->name;

    errCode = NEXUS_AudioReturnChannel_SetSettings( handle, pSettings );

    return handle;
#else
    BSTD_UNUSED(pSettings);
    BDBG_ERR(("Arc Index %u out of range", index));
    return NULL;
#endif
}


/***************************************************************************
Summary:
    Close an Audio Return Channel Output
 ***************************************************************************/
static void NEXUS_AudioReturnChannel_P_Finalizer(
    NEXUS_AudioReturnChannelHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioReturnChannel, handle);
    BDBG_ASSERT(handle->opened);

    NEXUS_AudioOutput_Shutdown(&handle->connector);
    BAPE_AudioReturnChannel_Close(handle->bapeArcHandle);
    /* This is equivalent to BDBG_OBJECT_UNSET and also clears other flags */
    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioReturnChannel));
}

static void NEXUS_AudioReturnChannel_P_Release(NEXUS_AudioReturnChannelHandle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioOutput, &handle->connector, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioReturnChannel, NEXUS_AudioReturnChannel_Close);

/***************************************************************************
Summary:
    Get the settings of an Audio Return Channel Output
 ***************************************************************************/
void NEXUS_AudioReturnChannel_GetSettings(
    NEXUS_AudioReturnChannelHandle handle,
    NEXUS_AudioReturnChannelSettings *pSettings    /* [out] Settings */
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioReturnChannel, handle);
    BDBG_ASSERT(NULL != pSettings);

    *pSettings = handle->settings;
}


/***************************************************************************
Summary:
    Set the settings of an Audio Return Channel Output
 ***************************************************************************/
NEXUS_Error NEXUS_AudioReturnChannel_SetSettings(
    NEXUS_AudioReturnChannelHandle handle,
    const NEXUS_AudioReturnChannelSettings *pSettings    /* [in] Settings */
    )
{
    NEXUS_AudioReturnChannelSettings settings;
    BAPE_AudioReturnChannelSettings bapeArcSettings;
    NEXUS_Error errCode;

    /* Sanity Check */
    NEXUS_OBJECT_ASSERT(NEXUS_AudioReturnChannel, handle);

    if ( NULL == pSettings )
    {
        NEXUS_AudioReturnChannel_GetDefaultSettings(&settings);
        pSettings = &settings;
    }
    BAPE_AudioReturnChannel_GetSettings(handle->bapeArcHandle, &bapeArcSettings);

    bapeArcSettings.muted = pSettings->muted;

    errCode = BAPE_AudioReturnChannel_SetSettings(handle->bapeArcHandle, &bapeArcSettings);
    if (errCode)
    {
        return BERR_TRACE(errCode);
    }

    handle->settings = *pSettings;

    /* success */
    return BERR_SUCCESS;
}


/***************************************************************************
Summary:
    Get the NEXUS_AudioOutputHandle connector for an Audio Return Channel Output
 ***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioReturnChannel_GetConnector(
    NEXUS_AudioReturnChannelHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_AudioReturnChannel, handle);
    return &handle->connector;
}


/***************************************************************************
Summary:
    Set the master Spdif source for an Audio Return Channel output
 ***************************************************************************/
NEXUS_Error NEXUS_AudioReturnChannel_P_SetMaster(
    NEXUS_AudioOutputHandle slaveHandle,      /* handle of ARC (slave) output device */
    NEXUS_AudioOutputHandle sourceHandle      /* handle of SPDIF master */
    )
{
    NEXUS_Error errCode;
    BAPE_AudioReturnChannelHandle apeArcHandle;
    BAPE_AudioReturnChannelSettings apeArcSettings;
    NEXUS_AudioReturnChannelHandle nexusArcHandle;

    BDBG_ASSERT(slaveHandle->objectType == NEXUS_AudioOutputType_eArc);
    BDBG_ASSERT(NULL != slaveHandle->pObjectHandle);

    nexusArcHandle = slaveHandle->pObjectHandle;
    apeArcHandle = nexusArcHandle->bapeArcHandle;

    BAPE_AudioReturnChannel_GetSettings(apeArcHandle, &apeArcSettings);

    if ( NULL == sourceHandle )
    {
        apeArcSettings.master = NULL;
    }
    else
    {
        BDBG_ASSERT(sourceHandle->objectType == NEXUS_AudioOutputType_eSpdif);
        apeArcSettings.master = (BAPE_OutputPort)sourceHandle->port;
    }

    errCode = BAPE_AudioReturnChannel_SetSettings(apeArcHandle, &apeArcSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

