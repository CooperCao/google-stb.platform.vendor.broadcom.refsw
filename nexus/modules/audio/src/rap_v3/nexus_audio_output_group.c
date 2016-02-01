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
*   API name: AudioOutput
*    API to group multiple stereo outputs into a multichannel output
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_audio_output_group);

typedef struct NEXUS_AudioOutputGroup
{
    NEXUS_OBJECT(NEXUS_AudioOutputGroup);
    unsigned numChannelPairs;
    NEXUS_AudioOutputHandle outputs[NEXUS_AudioChannelPair_eMax];
    NEXUS_AudioOutputObject connector;
} NEXUS_AudioOutputGroup;

void NEXUS_AudioOutputGroup_GetDefaultCreateSettings(
    NEXUS_AudioOutputGroupCreateSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_AudioOutputGroupHandle NEXUS_AudioOutputGroup_Create(
    const NEXUS_AudioOutputGroupCreateSettings *pSettings
    )
{
    unsigned i;
    NEXUS_AudioOutputGroupHandle handle;
    bool foundDac=false;
    bool foundOther=false;

    BDBG_ASSERT(NULL != pSettings);

    for ( i = 0; i < NEXUS_AudioChannelPair_eMax; i++ )
    {
        if ( NULL == pSettings->outputs[i] )
        {
            break;
        }
        BDBG_OBJECT_ASSERT(pSettings->outputs[i], NEXUS_AudioOutput);
        if ( NEXUS_AudioOutput_P_IsDacSlave(pSettings->outputs[i]) ||
             NEXUS_AudioOutput_P_IsSpdifSlave(pSettings->outputs[i]) ||
             pSettings->outputs[i]->objectType == NEXUS_AudioOutputType_eGroup ||
             pSettings->outputs[i]->objectType == NEXUS_AudioOutputType_eCapture ||
             pSettings->outputs[i]->objectType == NEXUS_AudioOutputType_eMux )
        {
            BDBG_ERR(("Output %p (type %u) is not supported in output groups.",
                      (void *)pSettings->outputs[i], pSettings->outputs[i]->objectType));
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            return NULL;
        }
        switch ( pSettings->outputs[i]->objectType )
        {
        case NEXUS_AudioOutputType_eDac:
            foundDac = true;
            break;
        default:
            foundOther = true;
            break;
        }
    }

    if ( i == 0 )
    {
        BDBG_ERR(("No outputs specified."));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    else if ( foundDac && foundOther )
    {
        BDBG_ERR(("You can not mix DACs and other types of outputs in the same group."));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    handle = BKNI_Malloc(sizeof(NEXUS_AudioOutputGroup));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    BKNI_Memset(handle, 0, sizeof(NEXUS_AudioOutputGroup));
    BDBG_OBJECT_SET(handle, NEXUS_AudioOutputGroup);
    NEXUS_AUDIO_OUTPUT_INIT(&handle->connector, NEXUS_AudioOutputType_eGroup, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioOutput, &handle->connector, Open);
    handle->connector.port = pSettings->outputs[0]->port;
    handle->numChannelPairs = i;
    for ( i = 0; i < NEXUS_AudioChannelPair_eMax; i++ )
    {
        handle->outputs[i] = pSettings->outputs[i];
    }

    return handle;
}

static void NEXUS_AudioOutputGroup_P_Finalizer(
    NEXUS_AudioOutputGroupHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioOutputGroup);
    NEXUS_AudioOutput_Shutdown(&handle->connector);
    BDBG_OBJECT_DESTROY(handle, NEXUS_AudioOutputGroup);
    BKNI_Free(handle);
}

static void NEXUS_AudioOutputGroup_P_Release( NEXUS_AudioOutputGroupHandle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioOutput, &handle->connector, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_AudioOutputGroup, NEXUS_AudioOutputGroup_Destroy);

NEXUS_AudioOutputHandle NEXUS_AudioOutputGroup_GetConnector(
    NEXUS_AudioOutputGroupHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioOutputGroup);
    return &handle->connector;
}

NEXUS_Error NEXUS_AudioOutputGroup_P_GetDestinationDetails(
    NEXUS_AudioOutputGroupHandle handle,
    NEXUS_AudioInputFormat format,
    BRAP_ChannelHandle primaryChannel,
    BRAP_DstDetails *pDestInfo
    )
{
    BRAP_OutputPortConfig opConfig;
    BERR_Code errCode;
    unsigned i, numOutputs;
    BRAP_OutputMode outputMode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioOutputGroup);

    switch ( format )
    {
    default:
        BDBG_ERR(("Unsupported input format %u", format));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case NEXUS_AudioInputFormat_eCompressed:
    case NEXUS_AudioInputFormat_ePcmStereo:
        numOutputs=1;
        outputMode = BRAP_OutputMode_e2_0;
        break;
    case NEXUS_AudioInputFormat_ePcm5_1:
        numOutputs=3;
        outputMode = BRAP_OutputMode_e3_2;
        break;
    case NEXUS_AudioInputFormat_ePcm7_1:
        numOutputs=4;
        outputMode = BRAP_OutputMode_e3_4;
        break;
    }

#if BRAP_VER >= 4
    pDestInfo->eAudioMode = outputMode;
    pDestInfo->bLfeOn = numOutputs > 2 ? true : false;
#else
    pDestInfo->uDstDetails.sOpDetails.eAudioMode = outputMode;
    pDestInfo->uDstDetails.sOpDetails.bLfeOn = numOutputs > 2 ? true : false;
#endif
    
    if ( numOutputs > handle->numChannelPairs )
    {
        BDBG_ERR(("Output format requires %u channel pairs but only %u are available.", numOutputs, handle->numChannelPairs));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
            
    for ( i = 0; i < numOutputs; i++ )
    {
        pDestInfo->uDstDetails.sOpDetails.eOutput[i] = handle->outputs[i]->port;
        /* If we have HDMI we need to whack MAI Mux and MAI Format and send the proper InfoFrame data */
        if ( handle->outputs[i]->objectType == NEXUS_AudioOutputType_eHdmi )
        {
            BRAP_DstDetails tmpDetails, tmpPrivDetails;
            NEXUS_AudioOutput_P_GetDestinationDetails(handle->outputs[i], 
                                                      format==NEXUS_AudioInputFormat_eCompressed?NEXUS_AudioInputFormat_eCompressed:NEXUS_AudioInputFormat_ePcmStereo,
                                                      primaryChannel, &tmpDetails, &tmpPrivDetails);
        }
        BDBG_MSG(("Output %u port %u", i, pDestInfo->uDstDetails.sOpDetails.eOutput[i]));
        BRAP_GetCurrentOutputConfig(g_NEXUS_audioModuleData.hRap, handle->outputs[i]->port, &opConfig);
        opConfig.bCompressed = (format == NEXUS_AudioInputFormat_eCompressed)?true:false;
        BDBG_MSG(("Setting %s output for port %d", opConfig.bCompressed?"compressed":"stereo",handle->outputs[i]->port));
        errCode = BRAP_SetOutputConfig(g_NEXUS_audioModuleData.hRap, &opConfig);
        if (errCode) return BERR_TRACE(errCode);
    }
    for ( ; i < BRAP_OutputChannelPair_eMax; i++ )
    {
        pDestInfo->uDstDetails.sOpDetails.eOutput[i] = BRAP_OutputPort_eMax;
    }

    return BERR_SUCCESS;
}

void NEXUS_AudioOutputGroup_P_SetDestination(
    NEXUS_AudioOutputGroupHandle handle,
    BRAP_DestinationHandle destination
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, NEXUS_AudioOutputGroup);

    for ( i = 0; i < handle->numChannelPairs; i++ )
    {
        if ( handle->outputs[i]->objectType == NEXUS_AudioOutputType_eHdmi )
        {
            /* Set the destination handle */
            NEXUS_AudioOutput_P_SetDestination(handle->outputs[i], destination);
        }
    }
}

