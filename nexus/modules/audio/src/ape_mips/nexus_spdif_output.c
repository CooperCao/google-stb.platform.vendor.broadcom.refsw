/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
*   API name: SpdifOutput
*    Specific APIs related to SPDIF audio outputs.
*
***************************************************************************/
#include "nexus_audio_module.h"

BDBG_MODULE(nexus_spdif_output);

typedef struct NEXUS_SpdifOutput
{
    NEXUS_OBJECT(NEXUS_SpdifOutput);
    bool opened;
    bool rawStatus;
    BAPE_SpdifOutputHandle handle;
    NEXUS_SpdifOutputSettings settings;
    NEXUS_AudioOutputObject connector;
    NEXUS_AudioRawChannelStatus rawChannelStatus;
} NEXUS_SpdifOutput;

static NEXUS_SpdifOutput g_spdifOutputs[NEXUS_NUM_SPDIF_OUTPUTS];

/***************************************************************************
Summary:
    Get default settings for a SPDIF Output
See Also:
    NEXUS_SpdifOutput_Get
 ***************************************************************************/
void NEXUS_SpdifOutput_GetDefaultSettings(
    NEXUS_SpdifOutputSettings *pSettings   /* [out] default settings */
    )
{
    BAPE_SpdifOutputSettings defaults;
    BSTD_UNUSED(pSettings);
    BAPE_SpdifOutput_GetDefaultSettings(&defaults);
    pSettings->limitTo16Bits = defaults.limitTo16Bits;
    pSettings->dither = defaults.ditherEnabled;
    BDBG_CASSERT(NEXUS_SpdifOutputBurstType_eMax == BAPE_SpdifBurstType_eMax);
    pSettings->burstType = defaults.underflowBurst;
    pSettings->channelStatusInfo.professionalMode = defaults.channelStatus.professional;
    pSettings->channelStatusInfo.swCopyRight = defaults.channelStatus.copyright;
    pSettings->channelStatusInfo.categoryCode = defaults.channelStatus.categoryCode;
    pSettings->channelStatusInfo.clockAccuracy = defaults.channelStatus.clockAccuracy;
}

/***************************************************************************
Summary:
    Open a SPDIF Output
See Also:
    NEXUS_SpdifOutput_Close
 ***************************************************************************/
NEXUS_SpdifOutputHandle NEXUS_SpdifOutput_Open(
    unsigned index,
    const NEXUS_SpdifOutputSettings *pSettings     /* Pass NULL for default settings */
    )
{
    BAPE_SpdifOutputHandle spdifHandle;
    BERR_Code errCode;
    BAPE_MixerOutput connector;

    /* Sanity check */
    if ( index >= NEXUS_NUM_SPDIF_OUTPUTS )
    {
        BDBG_ERR(("SpdifOutput Index %u out of range", index));
        return NULL;
    }

    if ( g_spdifOutputs[index].opened )
    {
        BDBG_ERR(("SpdifOutput %d already open", index));
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_SpdifOutput, &g_spdifOutputs[index]);

    errCode = BAPE_SpdifOutput_Open(NEXUS_AUDIO_DEVICE_HANDLE, index, NULL, &spdifHandle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        return NULL;
    }

    g_spdifOutputs[index].opened = true;
    /* Initialize connector */
    g_spdifOutputs[index].handle = spdifHandle;
    g_spdifOutputs[index].connector.pObjectHandle = &g_spdifOutputs[index];
    g_spdifOutputs[index].connector.objectType = NEXUS_AudioOutputType_eSpdif;
    NEXUS_SpdifOutput_SetSettings(&g_spdifOutputs[index], pSettings);

    BAPE_SpdifOutput_GetConnector(spdifHandle, &connector);
    g_spdifOutputs[index].connector.port = (uint32_t)connector;

    /* Success */
    return &g_spdifOutputs[index];
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_SpdifOutput, NEXUS_SpdifOutput_Close);

/***************************************************************************
Summary:
    Close a SPDIF Output
Description:
    Input to the SPDIF output must be removed prior to closing.
See Also:
    NEXUS_SpdifOutput_Open
    NEXUS_AudioOutput_RemoveInput
    NEXUS_AudioOutput_RemoveAllInputs
 ***************************************************************************/
static void NEXUS_SpdifOutput_P_Finalizer(
    NEXUS_SpdifOutputHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_SpdifOutput, handle);

    if ( NULL != handle->connector.pMixerData )
    {
        BDBG_ERR(("Output is still active.  Please call NEXUS_AudioOutput_Shutdown first."));
        BDBG_ASSERT(NULL == handle->connector.pMixerData);
    }

    BAPE_SpdifOutput_Close(handle->handle);
    BKNI_Memset(handle, 0, sizeof(NEXUS_SpdifOutput));
}

/***************************************************************************
Summary:
    Get Settings for a SPDIF Output
See Also:
    NEXUS_SpdifOutput_SetSettings
 ***************************************************************************/
void NEXUS_SpdifOutput_GetSettings(
    NEXUS_SpdifOutputHandle handle,
    NEXUS_SpdifOutputSettings *pSettings    /* [out] Settings */
    )
{
    /* Sanity Check */
    NEXUS_OBJECT_ASSERT(NEXUS_SpdifOutput, handle);
    BDBG_ASSERT(NULL != pSettings);

    /* Copy settings */
    *pSettings = handle->settings;
}

/***************************************************************************
Summary:
    Set Settings for a SPDIF Output
See Also:
    NEXUS_SpdifOutput_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_SpdifOutput_SetSettings(
    NEXUS_SpdifOutputHandle handle,
    const NEXUS_SpdifOutputSettings *pSettings
    )
{
    int i;
    BERR_Code errCode;
    NEXUS_SpdifOutputSettings defaults;
    BAPE_SpdifOutputSettings settings;

    NEXUS_OBJECT_ASSERT(NEXUS_SpdifOutput, handle);
    if ( NULL == pSettings )
    {
        NEXUS_SpdifOutput_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

    BAPE_SpdifOutput_GetSettings(handle->handle, &settings);
    
    settings.limitTo16Bits = pSettings->limitTo16Bits;
    settings.ditherEnabled = pSettings->dither;
    settings.useRawChannelStatus = handle->rawStatus;
    if ( handle->rawStatus )
    {
        uint32_t raw;
        raw = handle->rawChannelStatus.leftChannelLow;
        for ( i = 0; i < 4; i++ )
        {
            settings.rawChannelStatus[i] = raw&0xff;
            raw >>= 8;
        }
        raw = handle->rawChannelStatus.leftChannelHigh;
        for ( i = 4; i < 8; i++ )
        {
            settings.rawChannelStatus[i] = raw&0xff;
            raw >>= 8;
        }
    }
    else
    {
        settings.channelStatus.professional = pSettings->channelStatusInfo.professionalMode;
        settings.channelStatus.copyright = pSettings->channelStatusInfo.swCopyRight;
        settings.channelStatus.categoryCode = pSettings->channelStatusInfo.categoryCode;
        settings.channelStatus.clockAccuracy = pSettings->channelStatusInfo.clockAccuracy;
    }

    errCode = BAPE_SpdifOutput_SetSettings(handle->handle, &settings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->settings = *pSettings;

    return NEXUS_SUCCESS;
}

/***************************************************************************
Summary:
    Get the audio connector for a SPDIF output
See Also:

 ***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_SpdifOutput_GetConnector(
    NEXUS_SpdifOutputHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_SpdifOutput, handle);
    return &handle->connector;
}

/***************************************************************************
Summary:
    Program the transmitted channel status in raw format.
Description:
    This API will set the SPDIF channel status bits using raw 64-bit values
    per channel.  Not available on all platforms.  Once set, the values in
    NEXUS_SpdifOutputSettings will be ignored.  To clear these settings, pass
    NULL for pChannelStatus.
See Also:
    NEXUS_AudioDecoder_GetRawChannelStatus
 ***************************************************************************/
NEXUS_Error NEXUS_SpdifOutput_SetRawChannelStatus(
    NEXUS_SpdifOutputHandle handle,
    const NEXUS_AudioRawChannelStatus *pChannelStatus
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_SpdifOutput, handle);
    if ( NULL == pChannelStatus )
    {
        handle->rawStatus = false;
    }
    else
    {
        handle->rawStatus = true;
        handle->rawChannelStatus = *pChannelStatus;
        NEXUS_SpdifOutput_SetSettings(handle, &handle->settings);
    }
    return NEXUS_SUCCESS;
}

