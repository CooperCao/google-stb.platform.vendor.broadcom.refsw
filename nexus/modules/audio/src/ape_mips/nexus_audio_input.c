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
*   API name: AudioInput
*   Generic API for audio filter graph management
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"
#include "blst_queue.h"
#include "blst_slist.h"
#include "priv/nexus_audio_decoder_priv.h"
#include "nexus_vcxo.h"

BDBG_MODULE(nexus_audio_input);

typedef enum NEXUS_AudioOutputTiming
{
    NEXUS_AudioOutputTiming_eFlexible,
    NEXUS_AudioOutputTiming_eDac,
    NEXUS_AudioOutputTiming_ePll,
    NEXUS_AudioOutputTiming_eMax
} NEXUS_AudioOutputTiming;

typedef struct NEXUS_AudioInputMixerNode
{
    BAPE_MixerHandle mixer;
    BAPE_MixerSettings settings;
    unsigned usageCount;
    unsigned mixerIndex;
    NEXUS_AudioOutputTiming timing;
    unsigned dacTimingCount;
    unsigned pllTimingCount;
    unsigned flexTimingCount;
    BLST_Q_ENTRY(NEXUS_AudioInputMixerNode) mixerNode;
} NEXUS_AudioInputMixerNode;

typedef struct NEXUS_AudioOutputNode
{
    NEXUS_AudioOutputObject *pOutputConnector;
    NEXUS_AudioInputMixerNode *pMixerNode;          /* Copy of mixer this was added to */
    BLST_Q_ENTRY(NEXUS_AudioOutputNode) outputNode;
} NEXUS_AudioOutputNode;

typedef struct NEXUS_AudioDownstreamNode
{
    NEXUS_AudioInputObject *pDownstreamObject;
    BLST_Q_ENTRY(NEXUS_AudioDownstreamNode) downstreamNode;
} NEXUS_AudioDownstreamNode;

typedef struct NEXUS_AudioUpstreamNode
{
    void *pConnectionData;
    size_t connectionDataSize;
    NEXUS_AudioInputObject *pUpstreamObject;
    BLST_Q_ENTRY(NEXUS_AudioUpstreamNode) upstreamNode;
} NEXUS_AudioUpstreamNode;

BDBG_OBJECT_ID(NEXUS_AudioInputData);

typedef struct NEXUS_AudioInputData
{
    BDBG_OBJECT(NEXUS_AudioInputData)
    NEXUS_AudioInputObject *pConnector;
    BLST_Q_HEAD(UpstreamList, NEXUS_AudioUpstreamNode) upstreamList;       /* List of inputs to this node.  NEXUS_AudioUpstreamNodes will be in this list */
    BLST_Q_HEAD(DownstreamList, NEXUS_AudioDownstreamNode) downstreamList; /* List of downstream objects connected to this node.
                                                                              AudioDownstreamNodes will be in this list */
    BLST_Q_HEAD(OutputList, NEXUS_AudioOutputNode) outputList;             /* One input can contain one or more outputs */
    BLST_Q_HEAD(MixerList, NEXUS_AudioInputMixerNode) mixerList;

    BAPE_MixerInputVolume inputVolume;
} NEXUS_AudioInputData;

#define MAX_MIXERS 6
static bool g_mixerIdAllocated[MAX_MIXERS];

static bool NEXUS_AudioInput_P_IsRunningUpstream(NEXUS_AudioInput input);
static bool NEXUS_AudioInput_P_IsRunningDownstream(NEXUS_AudioInput input);
static void NEXUS_AudioInput_P_UnlinkMixerOutput(NEXUS_AudioInput input, NEXUS_AudioOutputNode *pOutputNode);
static NEXUS_Error NEXUS_AudioInput_P_CheckOutputMixer(NEXUS_AudioInput input, NEXUS_AudioOutputNode *pOutputNode);
static NEXUS_Error NEXUS_AudioInput_P_CheckOutputMixers(NEXUS_AudioInput input);
static NEXUS_AudioOutputTiming NEXUS_AudioInput_P_GetOutputTiming(NEXUS_AudioOutputHandle output);

#if BDBG_DEBUG_BUILD
static const char *g_pTypeNames[NEXUS_AudioInputType_eMax] =
{
    "Decoder",
    "Playback",
    "I2s",
    "Hdmi",
    "Spdif",
    "Rf",
    "Rf Decoder",
    "Mixer",
    "Encoder",
    "Bbe",
    "Xen",
    "Ddbm",
    "DtsNeo",
    "AutoVolume",
    "ProLogic2",
    "SrsXt",
    "CustomSurround",
    "CustomBass",
    "CustomVoice",
    "AnalogDecoder",
    "AnalogInput"
};
#endif

static NEXUS_AudioOutputTiming NEXUS_AudioInput_P_GetOutputTiming(NEXUS_AudioOutputHandle output)
{
    BDBG_ASSERT(NULL != output);
    switch ( output->objectType )
    {
    case NEXUS_AudioOutputType_eSpdif:
    case NEXUS_AudioOutputType_eI2s:
        return NEXUS_AudioOutputTiming_ePll;
    case NEXUS_AudioOutputType_eHdmi:
    case NEXUS_AudioOutputType_eCapture:
        return NEXUS_AudioOutputTiming_eFlexible;
    case NEXUS_AudioOutputType_eDac:
    case NEXUS_AudioOutputType_eRfm:
        return NEXUS_AudioOutputTiming_eDac;
    default:
        BDBG_ERR(("Output timing not known for output type %u", output->objectType));
        BDBG_ASSERT(0);
        return NEXUS_AudioOutputTiming_eMax;
    }
}

bool NEXUS_AudioInput_P_IsRunning(NEXUS_AudioInput input)
{
    BDBG_ASSERT(NULL != input);
    if ( NEXUS_AudioInput_P_IsRunningUpstream(input) ||
         NEXUS_AudioInput_P_IsRunningDownstream(input) )
    {
        return true;
    }
    return false;
}

static bool NEXUS_AudioInput_P_IsRunningUpstream(NEXUS_AudioInput input)
{
    NEXUS_AudioUpstreamNode *pNode;
    NEXUS_AudioInputData *pData;
    bool running = false;

    BDBG_ASSERT(NULL != input);

    BDBG_MSG(("NEXUS_AudioInput_P_IsRunning(%p)", input));

    pData = input->pMixerData;

    /* Can't be running if not connected */
    if ( NULL == pData )
    {
        BDBG_MSG(("No data - not running"));
        return false;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    switch ( input->objectType )
    {
#if NEXUS_NUM_AUDIO_DECODERS
    case NEXUS_AudioInputType_eDecoder:
        BDBG_MSG(("Decoder type - checking run status"));
        running = NEXUS_AudioDecoder_P_IsRunning(input->pObjectHandle);
        break;
#endif
#if NEXUS_NUM_AUDIO_PLAYBACKS
    case NEXUS_AudioInputType_ePlayback:
        BDBG_MSG(("Playback type - checking run status"));
        running = NEXUS_AudioPlayback_P_IsRunning(input->pObjectHandle);
        break;
#endif
#if NEXUS_NUM_I2S_INPUTS && 0
    case NEXUS_AudioInputType_eI2s:
        BDBG_MSG(("I2sInput type - checking run status"));
        running = NEXUS_I2sInput_P_IsRunning(input->pObjectHandle);
        break;
#endif
#if NEXUS_NUM_RF_AUDIO_DECODERS
    case NEXUS_AudioInputType_eRfDecoder:
        BDBG_MSG(("RF Audio Decoder type - checking run status"));
        running = NEXUS_RfAudioDecoder_P_IsRunning(input->pObjectHandle);
        break;
#endif
    default:
        /* Recurse if not a decoder type, stop if any input is running - that means we are also */
        BDBG_MSG(("other type - recursively checking run status"));
        pNode = BLST_Q_FIRST(&pData->upstreamList);
        while ( NULL != pNode && false == running )
        {
            running = running || NEXUS_AudioInput_P_IsRunningUpstream(pNode->pUpstreamObject);
            pNode = BLST_Q_NEXT(pNode, upstreamNode);
        }
        break;
    }

    BDBG_MSG(("Returning running status %d", running));
    return running;
}

static bool NEXUS_AudioInput_P_IsRunningDownstream(NEXUS_AudioInput input)
{
    NEXUS_AudioDownstreamNode *pNode;
    NEXUS_AudioInputData *pData;
    bool running = false;

    BDBG_ASSERT(NULL != input);

    BDBG_MSG(("NEXUS_AudioInput_P_IsRunning(%p)", input));

    pData = input->pMixerData;

    /* Can't be running if not connected */
    if ( NULL == pData )
    {
        BDBG_MSG(("No data - not running"));
        return false;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    switch ( input->objectType )
    {
#if NEXUS_NUM_AUDIO_DECODERS
    case NEXUS_AudioInputType_eDecoder:
        BDBG_MSG(("Decoder type - checking run status"));
        running = NEXUS_AudioDecoder_P_IsRunning(input->pObjectHandle);
        break;
#endif
#if NEXUS_NUM_AUDIO_PLAYBACKS
    case NEXUS_AudioInputType_ePlayback:
        BDBG_MSG(("Playback type - checking run status"));
        running = NEXUS_AudioPlayback_P_IsRunning(input->pObjectHandle);
        break;
#endif
#if NEXUS_NUM_I2S_INPUTS && 0
    case NEXUS_AudioInputType_eI2s:
        BDBG_MSG(("I2sInput type - checking run status"));
        running = NEXUS_I2sInput_P_IsRunning(input->pObjectHandle);
        break;
#endif
#if NEXUS_NUM_RF_AUDIO_DECODERS
    case NEXUS_AudioInputType_eRfDecoder:
        BDBG_MSG(("RF Audio Decoder type - checking run status"));
        running = NEXUS_RfAudioDecoder_P_IsRunning(input->pObjectHandle);
        break;
#endif
    default:
        break;
    }

    if ( !running )
    {
        /* Recurse  */
        BDBG_MSG(("recursively checking run status"));
        pNode = BLST_Q_FIRST(&pData->downstreamList);
        while ( NULL != pNode && false == running )
        {
            running = running || NEXUS_AudioInput_P_IsRunningDownstream(pNode->pDownstreamObject);
            pNode = BLST_Q_NEXT(pNode, downstreamNode);
        }
    }

    BDBG_MSG(("Returning running status %d", running));
    return running;
}

NEXUS_AudioInputFormat NEXUS_AudioInput_P_GetFormat(NEXUS_AudioInput input)
{
    NEXUS_AudioUpstreamNode *pNode;
    NEXUS_AudioInputFormat format;
    NEXUS_AudioInputData *pData;

    BDBG_MSG(("NEXUS_AudioInput_P_GetFormat(%p)", input));

    /* If input format is set at this node, return it.  Otherwise, recurse up, taking the maximum number of channels. */
    if ( input->format == NEXUS_AudioInputFormat_eNone )
    {
        pData = input->pMixerData;

        /* Can't do anything if not connected */
        if ( NULL == pData )
        {
            return NEXUS_AudioInputFormat_eNone;
        }
        BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

        format = NEXUS_AudioInputFormat_eNone;

        pNode = BLST_Q_FIRST(&pData->upstreamList);
        while ( NULL != pNode )
        {
            NEXUS_AudioInputFormat newFormat;

            newFormat = NEXUS_AudioInput_P_GetFormat(pNode->pUpstreamObject);

            if ( newFormat > format )
            {
                format = newFormat;
            }

            pNode = BLST_Q_NEXT(pNode, upstreamNode);
        }
    }
    else
    {
        format = input->format;
    }

    return format;
}

static NEXUS_AudioInputData *NEXUS_AudioInput_P_CreateData(NEXUS_AudioInput input)
{
    NEXUS_AudioInputData *pData;

    BDBG_ASSERT(NULL != input);

    BDBG_MSG(("Creating connection data for input %p", input));

    pData = BKNI_Malloc(sizeof(NEXUS_AudioInputData));
    if ( NULL != pData )
    {
        int i, j;
        BDBG_OBJECT_SET(pData, NEXUS_AudioInputData);
        input->pMixerData = pData;
        pData->pConnector = input;
        BLST_Q_INIT(&pData->upstreamList);
        BLST_Q_INIT(&pData->downstreamList);
        BLST_Q_INIT(&pData->outputList);
        BLST_Q_INIT(&pData->mixerList);

        for ( i = 0; i < BAPE_Channel_eMax; i++ )
        {
            for ( j = 0; j < BAPE_Channel_eMax; j++ )
            {
                pData->inputVolume.coefficients[i][j] = (i==j)?BAPE_VOLUME_NORMAL:BAPE_VOLUME_MIN;
            }
        }
        pData->inputVolume.muted = false;
    }

    return pData;
}

NEXUS_Error NEXUS_AudioInput_P_AddInput(NEXUS_AudioInput destination, NEXUS_AudioInput source)
{
    NEXUS_AudioInputData *pDestinationData, *pSourceData;
    NEXUS_AudioDownstreamNode *pDownstream;
    NEXUS_AudioUpstreamNode *pUpstream;
    NEXUS_Error errCode;

    BDBG_ASSERT(NULL != destination);
    BDBG_ASSERT(NULL != source);

    BDBG_MSG(("NEXUS_AudioInput_P_AddInput(%p,%p) [%s->%s]", destination, source,
              g_pTypeNames[source->objectType], g_pTypeNames[destination->objectType]));

    /* On 7440/3563/7405+ architectures, inputs must be stopped for this to proceed. */
    if ( NEXUS_AudioInput_P_IsRunning(destination) )
    {
        BDBG_ERR(("Can not add inputs to a node while it's running."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if ( NEXUS_AudioInput_P_IsRunning(source) )
    {
        BDBG_ERR(("Can not add running inputs to a node."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if ( (source->objectType == NEXUS_AudioInputType_eHdmi ||
          source->objectType == NEXUS_AudioInputType_eSpdif) &&
         destination->objectType != NEXUS_AudioInputType_eDecoder )
    {
        BDBG_ERR(("Can only connect SPDIF or HDMI Input to a decoder"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Other sanity checks are done by caller, they differ per-type */
    pDestinationData = destination->pMixerData;
    if ( NULL == pDestinationData )
    {
        /* Must allocate data on the first connection */
        pDestinationData = NEXUS_AudioInput_P_CreateData(destination);
    }
    BDBG_OBJECT_ASSERT(pDestinationData, NEXUS_AudioInputData);

    pSourceData = source->pMixerData;
    if ( NULL == pSourceData )
    {
        /* Must allocate data on the first connection */
        pSourceData = NEXUS_AudioInput_P_CreateData(source);
    }
    BDBG_OBJECT_ASSERT(pSourceData, NEXUS_AudioInputData);

    pDownstream = BKNI_Malloc(sizeof(NEXUS_AudioDownstreamNode));
    if ( NULL == pDownstream )
    {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }

    pUpstream = BKNI_Malloc(sizeof(NEXUS_AudioUpstreamNode));
    if ( NULL == pUpstream )
    {
        BKNI_Free(pDownstream);
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }

    pDownstream->pDownstreamObject = destination;
    pUpstream->pUpstreamObject = source;
    pUpstream->pConnectionData = NULL;
    pUpstream->connectionDataSize = 0;

    /* TODO: Link post-processing */

    /* Link source -> APE mixer if required */
    if ( destination->objectType == NEXUS_AudioInputType_eMixer )
    {
        NEXUS_AudioInputMixerNode *pMixerNode;
        for ( pMixerNode = BLST_Q_FIRST(&pDestinationData->mixerList);
              pMixerNode != NULL;
              pMixerNode = BLST_Q_NEXT(pMixerNode, mixerNode) )
        {
            errCode = BAPE_Mixer_AddInput(pMixerNode->mixer, (BAPE_MixerInput)source->port, (source->objectType == NEXUS_AudioInputType_eDecoder)?true:false);
            if ( errCode )
            {
                BKNI_Free(pDownstream);
                BKNI_Free(pUpstream);
                return BERR_TRACE(errCode);
            }
        }
    }

    /* Create connection from source -> destination */
    BLST_Q_INSERT_TAIL(&pSourceData->downstreamList, pDownstream, downstreamNode);
    /* Create reverse-linkage for destination -> source */
    BLST_Q_INSERT_TAIL(&pDestinationData->upstreamList, pUpstream, upstreamNode);

    /* Done */
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioInput_P_RemoveAllInputs(NEXUS_AudioInput destination)
{
    NEXUS_Error errCode;
    NEXUS_AudioUpstreamNode *pNode;
    NEXUS_AudioInputData *pDestinationData;

    BDBG_MSG(("NEXUS_AudioInput_P_RemoveAllInputs(%p)", destination));

    pDestinationData = destination->pMixerData;

    if ( NULL != pDestinationData )
    {
        BDBG_OBJECT_ASSERT(pDestinationData, NEXUS_AudioInputData);
        while ( NULL != (pNode=BLST_Q_FIRST(&pDestinationData->upstreamList)) )
        {
            errCode = NEXUS_AudioInput_P_RemoveInput(destination, pNode->pUpstreamObject);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
    }
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioInput_P_RemoveInput(NEXUS_AudioInput destination, NEXUS_AudioInput source)
{
    NEXUS_AudioUpstreamNode *pUpstreamNode;
    NEXUS_AudioDownstreamNode *pDownstreamNode;
    NEXUS_AudioInputData *pDestinationData;
    NEXUS_AudioInputData *pSourceData;

    BDBG_MSG(("NEXUS_AudioInput_P_RemoveInput(%p,%p)", destination, source));

    pDestinationData = destination->pMixerData;
    BDBG_OBJECT_ASSERT(pDestinationData, NEXUS_AudioInputData);    /* It's impossible for this to be NULL in a valid config. */

    /* On 7440/3563/7405+ architectures, inputs must be stopped for this to proceed. */
    if ( NEXUS_AudioInput_P_IsRunning(destination) )
    {
        BDBG_ERR(("Can not remove inputs from a node while it's running."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("Unlink upstream link"));

    if ( destination->objectType == NEXUS_AudioInputType_eMixer )
    {
        NEXUS_AudioInputMixerNode *pMixerNode;
        for ( pMixerNode = BLST_Q_FIRST(&pDestinationData->mixerList);
              pMixerNode != NULL;
              pMixerNode = BLST_Q_NEXT(pMixerNode, mixerNode) )
        {
            BAPE_Mixer_RemoveInput(pMixerNode->mixer, (BAPE_MixerInput)source->port);
        }
    }

    /* TODO: Unlink post-processing */

    /* Unlink upstream link */
    pUpstreamNode = BLST_Q_FIRST(&pDestinationData->upstreamList);
    while ( NULL != pUpstreamNode && pUpstreamNode->pUpstreamObject != source )
    {
        pUpstreamNode = BLST_Q_NEXT(pUpstreamNode, upstreamNode);
    }
    /* We had better find a node or the list is broken */
    if ( NULL == pUpstreamNode)
    {
        BDBG_ASSERT(NULL != pUpstreamNode);
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    BDBG_ASSERT(pUpstreamNode->pUpstreamObject == source);
    BLST_Q_REMOVE(&pDestinationData->upstreamList, pUpstreamNode, upstreamNode);
    BDBG_MSG(("Free connection data"));
    /* Free up connection-specific data if set */
    if ( NULL != pUpstreamNode->pConnectionData )
    {
        BDBG_MSG(("Connection data exists -- freeing"));
        BKNI_Free(pUpstreamNode->pConnectionData);
        pUpstreamNode->pConnectionData = NULL;
        pUpstreamNode->connectionDataSize = 0;
    }
    BDBG_MSG(("Freeing upstream node"));
    BKNI_Free(pUpstreamNode);

    pSourceData = source->pMixerData;
    BDBG_OBJECT_ASSERT(pSourceData, NEXUS_AudioInputData);

    BDBG_MSG(("Free downstream data"));
    pDownstreamNode = BLST_Q_FIRST(&pSourceData->downstreamList);
    while ( NULL != pDownstreamNode && pDownstreamNode->pDownstreamObject != destination )
    {
        pDownstreamNode = BLST_Q_NEXT(pDownstreamNode, downstreamNode);
    }
    /* We had better find a node or the list is broken */
    if ( NULL == pDownstreamNode )
    {
        BDBG_ASSERT(NULL != pDownstreamNode);
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    BDBG_ASSERT(pDownstreamNode->pDownstreamObject == destination);
    BLST_Q_REMOVE(&pSourceData->downstreamList, pDownstreamNode, downstreamNode);
    BKNI_Free(pDownstreamNode);

    return BERR_SUCCESS;
}

bool NEXUS_AudioInput_P_IsConnected(NEXUS_AudioInput destination, NEXUS_AudioInput source)
{
    NEXUS_AudioUpstreamNode *pUpstreamNode;
    NEXUS_AudioInputData *pDestinationData;
    NEXUS_AudioInputData *pSourceData;

    BDBG_ASSERT(NULL != destination);
    BDBG_ASSERT(NULL != source);

    BDBG_MSG(("NEXUS_AudioInput_P_IsConnected(%p,%p)", destination, source));

    pDestinationData = destination->pMixerData;
    pSourceData = source->pMixerData;

    /* No possible connection if data is absent for either object */
    if ( NULL == destination->pMixerData || NULL == source->pMixerData )
    {
        return false;
    }
    BDBG_OBJECT_ASSERT(pDestinationData, NEXUS_AudioInputData);
    BDBG_OBJECT_ASSERT(pSourceData, NEXUS_AudioInputData);

    /* Check for destination->source link */
    pUpstreamNode = BLST_Q_FIRST(&pDestinationData->upstreamList);
    while ( NULL != pUpstreamNode && pUpstreamNode->pUpstreamObject != source )
    {
        pUpstreamNode = BLST_Q_NEXT(pUpstreamNode, upstreamNode);
    }

    return (NULL == pUpstreamNode) ? false : true;
}

static NEXUS_Error NEXUS_AudioInput_P_IterateOutputs(NEXUS_AudioInput input, NEXUS_AudioOutputList *pOutputList, int *pIndex, bool scanMixers)
{
    int i;
    NEXUS_AudioOutputNode *pOutputNode;
    NEXUS_AudioDownstreamNode *pDownstreamNode;
    NEXUS_AudioInputData *pData;
    NEXUS_Error errCode = BERR_SUCCESS;

    BDBG_ASSERT(NULL != input);
    BDBG_ASSERT(NULL != pOutputList);
    BDBG_ASSERT(NULL != pIndex);

    BDBG_MSG(("NEXUS_AudioInput_P_IterateOutputs(%p)", input));

    /* First, iterate through any outputs directly attached to this object */
    i = *pIndex;
    pData = input->pMixerData;
    BDBG_MSG(("Scanning node %p for outputs", input));
    if ( NULL == pData || i >= NEXUS_AUDIO_MAX_OUTPUTS )
    {
        if ( NULL == pData )
        {
            BDBG_MSG(("No connection data at node %p", input));
        }
        else
        {
            BDBG_MSG(("Output list full at node %p", input));
        }
        goto done;
    }
    pOutputNode = BLST_Q_FIRST(&pData->outputList);
    while ( NULL != pOutputNode && i < NEXUS_AUDIO_MAX_OUTPUTS )
    {
        BDBG_ASSERT(NULL != pOutputNode->pOutputConnector);
        pOutputList->outputs[i++] = pOutputNode->pOutputConnector;
        BDBG_MSG(("Found output %p at node %p", pOutputNode->pOutputConnector, input));
        pOutputNode = BLST_Q_NEXT(pOutputNode, outputNode);
    }

    /* Now recurse through all downstream connections */
    pDownstreamNode = BLST_Q_FIRST(&pData->downstreamList);
    while ( NULL != pDownstreamNode && i < NEXUS_AUDIO_MAX_OUTPUTS )
    {
        if ( (pDownstreamNode->pDownstreamObject->objectType != NEXUS_AudioInputType_eMixer) ||
             true == scanMixers )
        {
            BDBG_MSG(("Recursing into downstream node %p", pDownstreamNode->pDownstreamObject));
            errCode = NEXUS_AudioInput_P_IterateOutputs(pDownstreamNode->pDownstreamObject, pOutputList, &i, scanMixers);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto done;
            }
        }
        else
        {
            BDBG_MSG(("Skipping downstream mixer %p", pDownstreamNode->pDownstreamObject));
        }
        pDownstreamNode = BLST_Q_NEXT(pDownstreamNode, downstreamNode);
    }

done:
    *pIndex = i;
    return errCode;
}

NEXUS_Error NEXUS_AudioInput_P_GetOutputs(NEXUS_AudioInput input, NEXUS_AudioOutputList *pOutputList, bool directOnly)
{
    int i=0;

    BDBG_ASSERT(NULL != input);
    BDBG_ASSERT(NULL != pOutputList);

    BDBG_MSG(("NEXUS_AudioInput_P_GetOutputs(%p)", input));

    /* Invalidate list on first call */
    BKNI_Memset(pOutputList, 0, sizeof(NEXUS_AudioOutputList));

    return NEXUS_AudioInput_P_IterateOutputs(input, pOutputList, &i, !directOnly);
}

NEXUS_Error NEXUS_AudioInput_P_SetConnectionData(NEXUS_AudioInput destination, NEXUS_AudioInput source, const void *pData, size_t dataSize)
{
    NEXUS_AudioInputData *pInputData;
    NEXUS_AudioUpstreamNode *pUpstreamNode;

    BDBG_ASSERT(NULL != destination);
    BDBG_ASSERT(NULL != source);
    BDBG_ASSERT(NULL != pData);
    BDBG_ASSERT(dataSize > 0);

    pInputData = destination->pMixerData;
    BDBG_OBJECT_ASSERT(pInputData, NEXUS_AudioInputData);

    pUpstreamNode = BLST_Q_FIRST(&pInputData->upstreamList);
    while ( NULL != pUpstreamNode && pUpstreamNode->pUpstreamObject != source )
    {
        pUpstreamNode = BLST_Q_NEXT(pUpstreamNode, upstreamNode);
    }
    if ( NULL == pUpstreamNode )
    {
        BDBG_ASSERT(NULL != pUpstreamNode);
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    if ( NULL != pUpstreamNode->pConnectionData && dataSize != pUpstreamNode->connectionDataSize )
    {
        /* Odd request.  Size has changed?  Handle it anyway */
        BKNI_Free(pUpstreamNode->pConnectionData);
        pUpstreamNode->connectionDataSize = 0;
        pUpstreamNode->pConnectionData = NULL;
    }

    if ( NULL == pUpstreamNode->pConnectionData )
    {
        /* Malloc the data */
        pUpstreamNode->pConnectionData = BKNI_Malloc(dataSize);
        if ( NULL == pUpstreamNode->pConnectionData )
        {
            return BERR_OUT_OF_SYSTEM_MEMORY;
        }
        pUpstreamNode->connectionDataSize = dataSize;
    }

    BDBG_ASSERT(NULL != pUpstreamNode->pConnectionData);

    /* Copy data */
    BKNI_Memcpy(pUpstreamNode->pConnectionData, pData, dataSize);

    return BERR_SUCCESS;
}

const void *NEXUS_AudioInput_P_GetConnectionData(NEXUS_AudioInput destination, NEXUS_AudioInput source)
{
    NEXUS_AudioInputData *pInputData;
    NEXUS_AudioUpstreamNode *pUpstreamNode;
    NEXUS_Error rc=0;

    BDBG_ASSERT(NULL != destination);
    BDBG_ASSERT(NULL != source);

    pInputData = destination->pMixerData;
    BDBG_OBJECT_ASSERT(pInputData, NEXUS_AudioInputData);

    pUpstreamNode = BLST_Q_FIRST(&pInputData->upstreamList);
    while ( NULL != pUpstreamNode && pUpstreamNode->pUpstreamObject != source )
    {
        pUpstreamNode = BLST_Q_NEXT(pUpstreamNode, upstreamNode);
    }
    if ( NULL == pUpstreamNode )
    {
        BDBG_ASSERT(NULL != pUpstreamNode);
        rc=BERR_TRACE(NEXUS_UNKNOWN);
        return NULL;
    }

    return pUpstreamNode->pConnectionData;  /* Return connection data pointer - may be NULL if never set */
}

NEXUS_Error NEXUS_AudioInput_P_ConnectOutput(NEXUS_AudioInput input, NEXUS_AudioOutputHandle output)
{
    NEXUS_AudioInputData *pInputData;
    NEXUS_AudioOutputNode *pNode;

    BDBG_ASSERT(NULL != input);
    BDBG_ASSERT(NULL != output);

    pInputData = input->pMixerData;

    /* SPDIF and HDMI are special cases.  HDMI accepts no outputs, SPDIF
       can be connected either to a decoder or a SPDIF output as bypass */
    if ( input->objectType == NEXUS_AudioInputType_eHdmi || input->objectType == NEXUS_AudioInputType_eSpdif )
    {
        BDBG_ERR(("Can not directly connect outputs to SPDIF or HDMI Input.  Please connect to a decoder for use."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("Connecting output %p to input %p", output, input));

    if ( NULL == pInputData )
    {
        pInputData = NEXUS_AudioInput_P_CreateData(input);
        if ( NULL == pInputData )
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }
    BDBG_OBJECT_ASSERT(pInputData, NEXUS_AudioInputData);

    pNode = BKNI_Malloc(sizeof(NEXUS_AudioOutputNode));
    if ( NULL == pNode )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    pNode->pOutputConnector = output;
    pNode->pMixerNode = NULL;
    BLST_Q_INSERT_TAIL(&pInputData->outputList, pNode, outputNode);

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioInput_P_DisconnectOutput(NEXUS_AudioInput input, NEXUS_AudioOutputHandle output)
{
    NEXUS_AudioInputData *pInputData;
    NEXUS_AudioOutputNode *pNode;

    BDBG_ASSERT(NULL != input);
    BDBG_ASSERT(NULL != output);

    pInputData = input->pMixerData;
    BDBG_OBJECT_ASSERT(pInputData, NEXUS_AudioInputData);    /* Illegal for this to be NULL with connections */

    pNode = BLST_Q_FIRST(&pInputData->outputList);
    while ( NULL != pNode && pNode->pOutputConnector != output )
    {
        pNode = BLST_Q_NEXT(pNode, outputNode);
    }
    if ( NULL == pNode)
    {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    /* Clear DAC slave selection if required. */
    if ( NEXUS_AudioOutput_P_IsDacSlave(pNode->pOutputConnector) )
    {
        NEXUS_AudioOutput_P_SetSlaveSource(pNode->pOutputConnector, NULL);
    }

    /*
       If a DAC is removed, the slave source will be re-evaluated at next start call
       (During AddDestinations).  No need to check that here.
    */

    NEXUS_AudioInput_P_UnlinkMixerOutput(input, pNode);
    BLST_Q_REMOVE(&pInputData->outputList, pNode, outputNode);

    BKNI_Memset(pNode, 0, sizeof(*pNode));
    BKNI_Free(pNode);

    return BERR_SUCCESS;
}

void NEXUS_AudioInput_Shutdown(
    NEXUS_AudioInput input
    )
{
    NEXUS_AudioInputData *pInputData;
    NEXUS_AudioOutputNode *pNode;
    NEXUS_AudioDownstreamNode *pDownstreamNode;
    NEXUS_Error rc;

    BDBG_ASSERT(NULL != input);

    BDBG_MSG(("Shutting down connector %p", input));

    pInputData = input->pMixerData;

    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_ERR(("Input %p is still running.  Stop it first.", input));
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }

    if ( NULL != pInputData )
    {
        BDBG_OBJECT_ASSERT(pInputData, NEXUS_AudioInputData);
        /* Remove all inputs */
        rc = NEXUS_AudioInput_P_RemoveAllInputs(input);
        if (rc) {rc = BERR_TRACE(rc);}
        /* Break all downstream connections */
        while ( (pDownstreamNode=BLST_Q_FIRST(&pInputData->downstreamList)) )
        {
            BDBG_WRN(("Forcibly breaking connection between input %p and %p on shutdown", pDownstreamNode->pDownstreamObject, input));
            rc = NEXUS_AudioInput_P_RemoveInput(pDownstreamNode->pDownstreamObject, input);
            if (rc) {
                rc = BERR_TRACE(rc);
                break;
            }
        }
        /* Remove all outputs */
        while ( NULL != (pNode = BLST_Q_FIRST(&pInputData->outputList)) )
        {
            BDBG_WRN(("Forcibly removing output %p on shutdown (port=%d)", pNode->pOutputConnector, pNode->pOutputConnector->port));
            rc = NEXUS_AudioOutput_RemoveInput(pNode->pOutputConnector, input);
            if (rc) {
                rc = BERR_TRACE(rc);
                break;
            }
        }

        /* Free connection data */
        BDBG_OBJECT_DESTROY(pInputData, NEXUS_AudioInputData);
        BKNI_Free(pInputData);
        input->pMixerData = NULL;
    }

    return ;
}

/***************************************************************************
Summary:
    Prepare the input chain to start.  May build and/or validate connections.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_PrepareToStart(
    NEXUS_AudioInput input
    )
{
    NEXUS_Error errCode;
    NEXUS_AudioInputData *pData;

    BDBG_ASSERT(NULL != input);

    BDBG_MSG(("NEXUS_AudioInput_P_PrepareToStart(%p)", input));

    pData = input->pMixerData;
    if ( NULL == pData )
    {
        /* If no connections exist, there is nothing to do */
        return BERR_SUCCESS;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    BDBG_ASSERT(input->objectType != NEXUS_AudioInputType_eHdmi);
    BDBG_ASSERT(input->objectType != NEXUS_AudioInputType_eSpdif);
    BDBG_ASSERT(input->objectType != NEXUS_AudioInputType_eMixer);

    BDBG_MSG(("Checking output mixers"));
    errCode = NEXUS_AudioInput_P_CheckOutputMixers(input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

void NEXUS_AudioInput_GetSyncSettings_priv( NEXUS_AudioInput audioInput, NEXUS_AudioInputSyncSettings *pSyncSettings )
{
    NEXUS_ASSERT_MODULE();
    switch (audioInput->objectType)
    {
    case NEXUS_AudioInputType_eDecoder:
        NEXUS_AudioDecoder_GetSyncSettings_priv((NEXUS_AudioDecoderHandle)audioInput->pObjectHandle, pSyncSettings);
        break;
    default:
        BDBG_WRN(("This audio input does not support SyncChannel"));
        break;
    }
}

NEXUS_Error NEXUS_AudioInput_SetSyncSettings_priv( NEXUS_AudioInput audioInput, const NEXUS_AudioInputSyncSettings *pSyncSettings )
{
    NEXUS_Error rc = 0;
    NEXUS_ASSERT_MODULE();
    switch (audioInput->objectType)
    {
    case NEXUS_AudioInputType_eDecoder:
        rc = NEXUS_AudioDecoder_SetSyncSettings_priv((NEXUS_AudioDecoderHandle)audioInput->pObjectHandle, pSyncSettings);
        break;
    default:
        BDBG_WRN(("This audio input does not support SyncChannel"));
        break;
    }
    return rc;
}

NEXUS_Error NEXUS_AudioInput_GetSyncStatus_isr( NEXUS_AudioInput audioInput, NEXUS_AudioInputSyncStatus *pSyncStatus )
{
    NEXUS_Error rc = 0;
    switch (audioInput->objectType)
    {
    case NEXUS_AudioInputType_eDecoder:
        rc = NEXUS_AudioDecoder_GetSyncStatus_isr((NEXUS_AudioDecoderHandle)audioInput->pObjectHandle, pSyncStatus);
        break;
    default:
        BDBG_WRN(("This audio input does not support SyncChannel"));
        break;
    }
    return rc;
}

void * NEXUS_AudioInput_GetDecoderHandle_priv(NEXUS_AudioInput audioInput)
{
    if (audioInput->objectType == NEXUS_AudioInputType_eDecoder)
    {
        return audioInput->pObjectHandle;
    }
    else
    {
        return NULL;
    }
}


/***************************************************************************
Summary:
    Returns the first object downstream from the current object that matches
    the specified type.  This is a depth-first search, not breadth-first.
 ***************************************************************************/
NEXUS_AudioInput NEXUS_AudioInput_P_FindByType(
    NEXUS_AudioInput input,
    NEXUS_AudioInputType type
    )
{
    NEXUS_AudioInput obj=NULL;
    NEXUS_AudioInputData *pData;
    NEXUS_AudioDownstreamNode *pNode;

    BDBG_ASSERT(NULL != input);

    if ( NULL == input->pMixerData )
    {
        return NULL;
    }

    pData = input->pMixerData;
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    /* Recurse through all children */
    for ( pNode = BLST_Q_FIRST(&pData->downstreamList);
        NULL != pNode;
        pNode = BLST_Q_NEXT(pNode, downstreamNode) )
    {
        if ( pNode->pDownstreamObject != NULL )
        {
            if ( pNode->pDownstreamObject->objectType == type )
            {
                return pNode->pDownstreamObject;
            }
            else
            {
                obj = NEXUS_AudioInput_P_FindByType(pNode->pDownstreamObject, type);
                if ( obj )
                {
                    return obj;
                }
            }
        }
    }

    /* Not found if we get here */
    return NULL;
}

static void NEXUS_AudioInput_P_UnlinkMixerOutput(NEXUS_AudioInput input, NEXUS_AudioOutputNode *pOutputNode)
{
    NEXUS_AudioInputData *pData;
    NEXUS_AudioInputMixerNode *pMixerNode;
    BDBG_ASSERT(NULL != input);
    BDBG_ASSERT(NULL != pOutputNode);
    pData = input->pMixerData;
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);
    

    pMixerNode = pOutputNode->pMixerNode;
    if ( pMixerNode )
    {
        BDBG_ASSERT(pMixerNode->usageCount > 0);
        pMixerNode->usageCount--;
        BAPE_Mixer_RemoveOutput(pMixerNode->mixer, (BAPE_MixerOutput)pOutputNode->pOutputConnector->port);
        if ( pMixerNode->usageCount == 0 )
        {
            /* Destroy the mixer */
            BAPE_Mixer_Close(pMixerNode->mixer);
            BDBG_ASSERT(pMixerNode->mixerIndex < MAX_MIXERS);
            g_mixerIdAllocated[pMixerNode->mixerIndex] = false;
            BLST_Q_REMOVE(&pData->mixerList, pMixerNode, mixerNode);
            BKNI_Free(pMixerNode);
        }
        else
        {
            switch ( NEXUS_AudioInput_P_GetOutputTiming(pOutputNode->pOutputConnector) )
            {
            case NEXUS_AudioOutputTiming_eDac:
                BDBG_ASSERT(pMixerNode->dacTimingCount > 0);
                pMixerNode->dacTimingCount--;
                break;
            case NEXUS_AudioOutputTiming_ePll:
                BDBG_ASSERT(pMixerNode->pllTimingCount > 0);
                pMixerNode->pllTimingCount--;
                break;
            case NEXUS_AudioOutputTiming_eFlexible:
                BDBG_ASSERT(pMixerNode->flexTimingCount > 0);
                pMixerNode->flexTimingCount--;
                break;
            default:
                /* Should never get here */
                BDBG_ASSERT(0);
                break;
            }

            if ( (pMixerNode->dacTimingCount == 0 && pMixerNode->timing == NEXUS_AudioOutputTiming_eDac) ||
                 (pMixerNode->pllTimingCount == 0 && pMixerNode->timing == NEXUS_AudioOutputTiming_ePll)  )
            {
                pMixerNode->timing = NEXUS_AudioOutputTiming_eFlexible;
            }
        }
        pOutputNode->pMixerNode = NULL;
    }
}

NEXUS_Error NEXUS_AudioInput_P_OutputSettingsChanged(
    NEXUS_AudioInput input, 
    NEXUS_AudioOutputHandle output
    )
{
    NEXUS_AudioInputData *pData;
    NEXUS_AudioOutputNode *pOutputNode;

    BDBG_ASSERT(NULL != input);
    pData = input->pMixerData;
    if ( NULL == pData )
    {
        return NEXUS_SUCCESS;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    /* Check my local outputs first */
    for ( pOutputNode = BLST_Q_FIRST(&pData->outputList);
          pOutputNode != NULL;
          pOutputNode = BLST_Q_NEXT(pOutputNode, outputNode) )
    {
        if ( pOutputNode->pOutputConnector == output )
        {
            return NEXUS_AudioInput_P_CheckOutputMixer(input, pOutputNode);
        }
    }

    BDBG_ERR(("output %p is not connected to input %p", output, input));
    BDBG_ASSERT(0);
    return BERR_TRACE(BERR_INVALID_PARAMETER);
}

static NEXUS_Error NEXUS_AudioInput_P_CheckOutputMixer(
    NEXUS_AudioInput input, 
    NEXUS_AudioOutputNode *pOutputNode
    )
{
    NEXUS_AudioInputMixerNode *pMixerNode;
    BAPE_MixerSettings mixerSettings;
    NEXUS_AudioInputData *pData;
    NEXUS_Error errCode=0;
    unsigned i=0;
    NEXUS_AudioOutputTiming timing;

    BDBG_ASSERT(NULL != input);
    BDBG_ASSERT(NULL != pOutputNode);
    pData = input->pMixerData;
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    timing = NEXUS_AudioInput_P_GetOutputTiming(pOutputNode->pOutputConnector);

    if ( NEXUS_AudioOutput_P_IsDacSlave(pOutputNode->pOutputConnector) )
    {
        return NEXUS_SUCCESS;
    }

    BDBG_MSG(("Getting output mixer settings"));
    NEXUS_AudioOutput_P_GetMixerSettings(pOutputNode->pOutputConnector, &mixerSettings);

    if ( pOutputNode->pMixerNode )
    {
        /* See if our settings have changed relative to the current node */
        if ( BKNI_Memcmp(&mixerSettings, &pOutputNode->pMixerNode->settings, sizeof(BAPE_MixerSettings)) )
        {
            BDBG_MSG(("Unlinking mixer output"));
            NEXUS_AudioInput_P_UnlinkMixerOutput(input, pOutputNode);
        }
    }

    if ( NULL == pOutputNode->pMixerNode )
    {
        /* Scan through all mixers to see if we match existing settings */
        for ( pMixerNode = BLST_Q_FIRST(&pData->mixerList);
              pMixerNode != NULL;
              pMixerNode = BLST_Q_NEXT(pMixerNode, mixerNode) )
        {
            BDBG_MSG(("Comparing against mixer node %p", pMixerNode));
            if ( !BKNI_Memcmp(&mixerSettings, &pMixerNode->settings, sizeof(BAPE_MixerSettings)) )
            {
                if ( (pMixerNode->timing == NEXUS_AudioOutputTiming_eDac && timing == NEXUS_AudioOutputTiming_ePll) ||
                     (pMixerNode->timing == NEXUS_AudioOutputTiming_ePll && timing == NEXUS_AudioOutputTiming_eDac))
                {
                    BDBG_MSG(("Can not add PLL and DAC outputs to the same mixer."));
                    continue;
                }
                BDBG_MSG(("Comparing against mixer node %p - MATCH", pMixerNode));
                errCode = BAPE_Mixer_AddOutput(pMixerNode->mixer, (BAPE_MixerOutput)pOutputNode->pOutputConnector->port);
                if ( errCode )
                {
                    return BERR_TRACE(errCode);
                }
                NEXUS_AudioOutput_P_SetOutputFormat(pOutputNode->pOutputConnector, NEXUS_AudioInput_P_GetFormat(input));
                pOutputNode->pMixerNode = pMixerNode;
                pMixerNode->usageCount++;
                switch ( timing )
                {
                case NEXUS_AudioOutputTiming_eFlexible:
                    pMixerNode->flexTimingCount++;
                    break;
                case NEXUS_AudioOutputTiming_ePll:
                    pMixerNode->pllTimingCount++;
                    if ( pMixerNode->timing == NEXUS_AudioOutputTiming_eFlexible )
                    {
                        BDBG_MSG(("Added PLL output to flex mixer.  Switching mixer timing to PLL."));
                        pMixerNode->timing = NEXUS_AudioOutputTiming_ePll;
                    }
                    break;
                case NEXUS_AudioOutputTiming_eDac:
                    pMixerNode->dacTimingCount++;
                    if ( pMixerNode->timing == NEXUS_AudioOutputTiming_eFlexible )
                    {
                        BDBG_MSG(("Added DAC output to flex mixer.  Switching mixer timing to DAC."));
                        pMixerNode->timing = NEXUS_AudioOutputTiming_eDac;
                    }
                default:
                    break;
                }
                goto check_vcxo_source;
            }
        }

        /* If we get here, we need to add a new mixer */
        for ( i = 0; i < MAX_MIXERS; i++ )
        {
            if ( !g_mixerIdAllocated[i] )
            {
                g_mixerIdAllocated[i] = true;
                break;
            }
        }
        if ( i >= MAX_MIXERS )
        {
            BDBG_ERR(("Insufficient mixers available for output configuration.  Please reduce the number of branches or the number of discrete independent delay values."));
            errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto err_mixer_id;
        }
        pMixerNode = BKNI_Malloc(sizeof(NEXUS_AudioInputMixerNode));
        if ( NULL == pMixerNode )
        {
            g_mixerIdAllocated[i] = false;
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto err_malloc;
        }
        BKNI_Memset(pMixerNode, 0, sizeof(NEXUS_AudioInputMixerNode));
        pMixerNode->mixerIndex = i;
        pMixerNode->usageCount = 1;
        switch ( timing )
        {
        case NEXUS_AudioOutputTiming_eFlexible:
            pMixerNode->flexTimingCount=1;
            break;
        case NEXUS_AudioOutputTiming_ePll:
            pMixerNode->pllTimingCount=1;
            break;
        case NEXUS_AudioOutputTiming_eDac:
            pMixerNode->dacTimingCount=1;
        default:
            break;
        }
        pMixerNode->timing = timing;
        BDBG_MSG(("Allocating mixer %u", i));
        errCode = BAPE_Mixer_Open(NEXUS_AUDIO_DEVICE_HANDLE, i, &mixerSettings, &pMixerNode->mixer);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_mixer_open;
        }
        pMixerNode->settings = mixerSettings;
        if ( input->objectType == NEXUS_AudioInputType_eMixer )
        {
            NEXUS_AudioUpstreamNode *pUpstreamNode;
            /* Mixers add all upstream objects as inputs */
            for ( pUpstreamNode = BLST_Q_FIRST(&pData->upstreamList);
                  pUpstreamNode != NULL;
                  pUpstreamNode = BLST_Q_NEXT(pUpstreamNode, upstreamNode) )
            {
                errCode = BAPE_Mixer_AddInput(pMixerNode->mixer, (BAPE_MixerInput)pUpstreamNode->pUpstreamObject->port,
                                              (pUpstreamNode->pUpstreamObject->objectType == NEXUS_AudioInputType_eDecoder)?true:false);
                if ( errCode )
                {
                    errCode = BERR_TRACE(errCode);
                    goto err_add_input;
                }
            }
        }
        else
        {
            /* Non-mixers add themselves as inputs */
            errCode = BAPE_Mixer_AddInput(pMixerNode->mixer, (BAPE_MixerInput)input->port,
                                          (input->objectType == NEXUS_AudioInputType_eDecoder)?true:false);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_add_input;
            }
        }
        errCode = BAPE_Mixer_AddOutput(pMixerNode->mixer, (BAPE_MixerOutput)pOutputNode->pOutputConnector->port);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_add_output;
        }
        NEXUS_AudioOutput_P_SetOutputFormat(pOutputNode->pOutputConnector, NEXUS_AudioInput_P_GetFormat(input));

        /* Add to the list */
        pOutputNode->pMixerNode = pMixerNode;
        BLST_Q_INSERT_TAIL(&pData->mixerList, pMixerNode, mixerNode);
    }

check_vcxo_source:
    pMixerNode = pOutputNode->pMixerNode;
    if ( pMixerNode->dacTimingCount == 0 )
    {
        NEXUS_AudioOutputSettings audioOutputSettings;
        NEXUS_AudioOutput_GetSettings(pOutputNode->pOutputConnector, &audioOutputSettings);
        if ( audioOutputSettings.autoConfigureVcxo )
        {
            /* Determine PLL linkage */
            NEXUS_AudioOutputPllSettings pllSettings;
            NEXUS_AudioOutputPll_GetSettings(audioOutputSettings.pll, &pllSettings);
            if ( pllSettings.mode == NEXUS_AudioOutputPllMode_eVcxo )
            {
                NEXUS_VcxoSettings vcxoSettings;
                NEXUS_Vcxo_GetSettings(pllSettings.modeSettings.vcxo.vcxo, &vcxoSettings);
                if ( vcxoSettings.timebase != audioOutputSettings.timebase )
                {
                    BDBG_WRN(("Changing VCXO-PLL %u timebase from %u to %u", pllSettings.modeSettings.vcxo.vcxo, vcxoSettings.timebase, audioOutputSettings.timebase));
                    vcxoSettings.timebase = audioOutputSettings.timebase;
                    errCode = NEXUS_Vcxo_SetSettings(pllSettings.modeSettings.vcxo.vcxo, &vcxoSettings);
                    if ( errCode )
                    {
                        /* Report and continue */
                        (void)BERR_TRACE(errCode);
                    }
                }
            }
        }
    }
    return BERR_SUCCESS;

err_add_output:
    BAPE_Mixer_RemoveAllInputs(pMixerNode->mixer);
err_add_input:
    BAPE_Mixer_Close(pMixerNode->mixer);
err_mixer_open:
    BKNI_Free(pMixerNode);
err_malloc:
    g_mixerIdAllocated[i] = false;
err_mixer_id:
    return errCode;
}

static NEXUS_Error NEXUS_AudioInput_P_CheckOutputMixers(NEXUS_AudioInput input)
{
    NEXUS_Error errCode;
    NEXUS_AudioInputData *pData;
    NEXUS_AudioDownstreamNode *pDownstreamNode;
    NEXUS_AudioOutputNode *pOutputNode;

    BDBG_ASSERT(NULL != input);
    pData = input->pMixerData;
    if ( NULL == pData )
    {
        return NEXUS_SUCCESS;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    /* Check my local outputs first */
    for ( pOutputNode = BLST_Q_FIRST(&pData->outputList);
          pOutputNode != NULL;
          pOutputNode = BLST_Q_NEXT(pOutputNode, outputNode) )
    {
        BDBG_MSG(("Checking Output Mixer for output %p (output type %d)", pOutputNode->pOutputConnector, pOutputNode->pOutputConnector->objectType));
        NEXUS_AudioInput_P_CheckOutputMixer(input, pOutputNode);
    }

    for ( pDownstreamNode = BLST_Q_FIRST(&pData->downstreamList);
          pDownstreamNode != NULL;
          pDownstreamNode = BLST_Q_NEXT(pDownstreamNode, downstreamNode) )
    {
        /* Recurse through children */
        BDBG_MSG(("Recursively Checking Output Mixers for input %p (input type %d)", pDownstreamNode->pDownstreamObject, pDownstreamNode->pDownstreamObject->objectType));
        errCode = NEXUS_AudioInput_P_CheckOutputMixers(pDownstreamNode->pDownstreamObject);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

void NEXUS_AudioInput_P_GetVolume(
    NEXUS_AudioInput input, 
    BAPE_MixerInputVolume *pInputVolume    /* [out] */
    )
{
    NEXUS_AudioInputData *pData;

    BDBG_ASSERT(NULL != input);
    BDBG_ASSERT(NULL != pInputVolume);
    pData = input->pMixerData;

    if ( NULL == pData )
    {
        pData = NEXUS_AudioInput_P_CreateData(input);
        if ( NULL == pData )
        {
            (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            BKNI_Memset(pInputVolume, 0, sizeof(*pInputVolume));
            return;
        }
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);
    *pInputVolume = pData->inputVolume;
}

static NEXUS_Error NEXUS_AudioInput_P_SetVolumeDownstream(
    NEXUS_AudioInput previous, 
    NEXUS_AudioInput input,
    const BAPE_MixerInputVolume *pInputVolume
    )
{
    NEXUS_AudioDownstreamNode *pDownstreamNode;
    NEXUS_AudioInputMixerNode *pMixerNode;
    NEXUS_AudioInputData *pData;
    BERR_Code errCode;

    BDBG_ASSERT(NULL != input);
    BDBG_ASSERT(NULL != pInputVolume);

    pData = input->pMixerData;
    if ( NULL == pData )
    {
        pData = NEXUS_AudioInput_P_CreateData(input);
        if ( NULL == pData )
        {
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
    }
    
    /* Store Volume */
    pData->inputVolume = *pInputVolume;

    /* Iterate through my mixers and apply the volume setting */
    for ( pMixerNode = BLST_Q_FIRST(&pData->mixerList);
          pMixerNode != NULL;
          pMixerNode = BLST_Q_NEXT(pMixerNode, mixerNode) )
    {
        if ( input->objectType == NEXUS_AudioInputType_eMixer )
        {
            errCode = BAPE_Mixer_SetInputVolume(pMixerNode->mixer, (BAPE_MixerInput)previous->port, pInputVolume);
        }
        else
        {
            errCode = BAPE_Mixer_SetInputVolume(pMixerNode->mixer, (BAPE_MixerInput)input->port, pInputVolume);
        }
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Stop recursing after a mixer is found */
    if ( input->objectType != NEXUS_AudioInputType_eMixer )
    {
        /* Iterate through downstream nodes and apply mixer volume */
        for ( pDownstreamNode = BLST_Q_FIRST(&pData->downstreamList);
              pDownstreamNode != NULL;
              pDownstreamNode = BLST_Q_NEXT(pDownstreamNode, downstreamNode) )
        {
            errCode = NEXUS_AudioInput_P_SetVolumeDownstream(input, pDownstreamNode->pDownstreamObject, pInputVolume);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
    }

    /* Done */
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioInput_P_SetVolume(
    NEXUS_AudioInput input, 
    const BAPE_MixerInputVolume *pInputVolume
    )
{
    return NEXUS_AudioInput_P_SetVolumeDownstream(NULL, input, pInputVolume);
}

static bool NEXUS_AudioInput_P_IsConnectedDownstream(NEXUS_AudioInput source, NEXUS_AudioInput sink)
{
    NEXUS_AudioDownstreamNode *pNode;
    NEXUS_AudioInputData *pData;
    bool connected = false;

    BDBG_OBJECT_ASSERT(source, NEXUS_AudioInput);
    BDBG_OBJECT_ASSERT(sink, NEXUS_AudioInput);

    BDBG_MSG(("NEXUS_AudioInput_P_IsConnectedDownstream(%p,%p)", source,sink));

    pData = source->pMixerData;

    /* Can't be running if not connected */
    if ( NULL == pData )
    {
        return false;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    /* This call has reversed args, sink<-source */
    connected = NEXUS_AudioInput_P_IsConnected(sink, source);

    /* Recurse  */
    BDBG_MSG(("recursively checking connected status"));
    pNode = BLST_Q_FIRST(&pData->downstreamList);
    while ( NULL != pNode && false == connected )
    {
        connected = connected || NEXUS_AudioInput_P_IsConnectedDownstream(pNode->pDownstreamObject, sink);
        pNode = BLST_Q_NEXT(pNode, downstreamNode);
    }

    BDBG_MSG(("Returning connected status %d", connected));

    return connected;
}

void NEXUS_AudioInput_IsConnectedToInput(
    NEXUS_AudioInput input1,
    NEXUS_AudioInput input2,
    bool *pConnected            /* [out] True if the nodes are connected in a filter graph */
    )
{
    bool connected;

    BDBG_OBJECT_ASSERT(input1, NEXUS_AudioInput);
    BDBG_OBJECT_ASSERT(input2, NEXUS_AudioInput);
    
    connected = NEXUS_AudioInput_P_IsConnectedDownstream(input1, input2);
    if ( false == connected )
    {
        connected = NEXUS_AudioInput_P_IsConnectedDownstream(input2, input1);
    }
    *pConnected = connected;
}

void NEXUS_AudioInput_IsConnectedToOutput(
    NEXUS_AudioInput input,
    NEXUS_AudioOutputHandle output,
    bool *pConnected            /* [out] True if the nodes are connected in a filter graph */
    )
{
    NEXUS_AudioOutputList outputList;
    NEXUS_Error errCode;
    unsigned i;

    BDBG_OBJECT_ASSERT(input, NEXUS_AudioInput);
    BDBG_OBJECT_ASSERT(output, NEXUS_AudioOutput);
    BDBG_ASSERT(NULL != pConnected);

    *pConnected = false;

    errCode = NEXUS_AudioInput_P_GetOutputs(input, &outputList, false);
    if ( BERR_SUCCESS == errCode )
    {
        for ( i = 0; i < NEXUS_AUDIO_MAX_OUTPUTS; i++ )
        {
            if ( outputList.outputs[i] == output )
            {
                *pConnected = true;
                return;
            }
        }
    }
}

void NEXUS_AudioInput_HasConnectedOutputs(
    NEXUS_AudioInput input,
    bool *pConnected            /* [out] True if one or more output is connected to this node */
    )
{
    NEXUS_AudioOutputList outputList;
    NEXUS_Error errCode;
    BDBG_ASSERT(NULL != pConnected);

    *pConnected = false;
    errCode = NEXUS_AudioInput_P_GetOutputs(input, &outputList, false);
    if ( BERR_SUCCESS == errCode )
    {
        if ( outputList.outputs[0] != NULL )
        {
            *pConnected = true;
        }
    }
}
