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
#include "priv/nexus_audio_decoder_priv.h"

BDBG_MODULE(nexus_audio_input);

typedef struct NEXUS_AudioOutputNode
{
    NEXUS_AudioOutputObject *pOutputConnector;
    NEXUS_AudioAssociationHandle association;           /* Copy of association this was added to */
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
} NEXUS_AudioInputData;

static NEXUS_Error NEXUS_AudioInput_P_Connect(NEXUS_AudioInput input);
static NEXUS_Error NEXUS_AudioInput_P_Disconnect(NEXUS_AudioInput destination, NEXUS_AudioInput source);
static NEXUS_Error NEXUS_AudioInput_P_DisconnectUpstream(NEXUS_AudioInput source);
static NEXUS_Error NEXUS_AudioInput_P_DisconnectDownstream(NEXUS_AudioInput destination, NEXUS_AudioInput source);
static NEXUS_Error NEXUS_AudioInput_P_AddDestinations(NEXUS_AudioInput input, NEXUS_AudioAssociationHandle association, bool downmix);

static void NEXUS_AudioInput_P_ForceStopUpstream(NEXUS_AudioInput input);
static void NEXUS_AudioInput_P_ForceStopDownstream(NEXUS_AudioInput input);
static bool NEXUS_AudioInput_P_IsRunningUpstream(NEXUS_AudioInput input);
static bool NEXUS_AudioInput_P_IsRunningDownstream(NEXUS_AudioInput input);

static NEXUS_Error NEXUS_AudioInput_P_RemoveDestinationsUpstream(NEXUS_AudioInput input);
static NEXUS_Error NEXUS_AudioInput_P_RemoveDestinationsDownstream(NEXUS_AudioInput input);
static NEXUS_Error NEXUS_AudioInput_P_AddPostProcessing(NEXUS_AudioInput input,
                                                        BRAP_ChannelHandle channel, bool slave, bool postMixing,
                                                        BRAP_ProcessingSettings *pPreMixingStages, int numPreMixingStages,
                                                        BRAP_ProcessingSettings *pPostMixingStages, int numPostMixingStages,
                                                        BRAP_ProcessingStageHandle descriptorStage, BRAP_ChannelHandle descriptorPair,
                                                        NEXUS_AudioCodec codec);
#if BDBG_DEBUG_BUILD
static unsigned NEXUS_AudioInput_P_GetNumOutputPaths(NEXUS_AudioInput input);

#if !BDBG_NO_MSG
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
#endif /* BDBG_NO_MSG */
#endif /* BDBG_DEBUG_BUILD */

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

    BDBG_MSG(("NEXUS_AudioInput_P_IsRunning(%p)", (void *)input));

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
#if NEXUS_NUM_I2S_INPUTS
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

    BDBG_MSG(("NEXUS_AudioInput_P_IsRunning(%p)", (void *)input));

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
#if NEXUS_NUM_I2S_INPUTS
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

    BDBG_MSG(("NEXUS_AudioInput_P_GetFormat(%p)", (void *)input));

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

    BDBG_MSG(("Creating connection data for input %p", (void *)input));

    pData = BKNI_Malloc(sizeof(NEXUS_AudioInputData));
    if ( NULL != pData )
    {
        BDBG_OBJECT_SET(pData, NEXUS_AudioInputData);
        input->pMixerData = pData;
        pData->pConnector = input;
        BLST_Q_INIT(&pData->upstreamList);
        BLST_Q_INIT(&pData->downstreamList);
        BLST_Q_INIT(&pData->outputList);
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

    BDBG_MSG(("NEXUS_AudioInput_P_AddInput(%p,%p) [%s->%s]", (void *)destination, (void *)source,
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

    /* Create connection from source -> destination */
    BLST_Q_INSERT_TAIL(&pSourceData->downstreamList, pDownstream, downstreamNode);
    /* Create reverse-linkage for destination -> source */
    BLST_Q_INSERT_TAIL(&pDestinationData->upstreamList, pUpstream, upstreamNode);

    /* Objects connected before decoder do not require this */
    if ( destination->objectType != NEXUS_AudioInputType_eDecoder )
    {
        /* Remove all destinations before potentially destroying associations */
        NEXUS_AudioInput_P_RemoveDestinations(destination);

        /* Trigger Disconnection Callbacks */
        errCode = NEXUS_AudioInput_P_Disconnect(destination, source);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Done */
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_AudioInput_P_RemoveAllInputs(NEXUS_AudioInput destination)
{
    NEXUS_Error errCode;
    NEXUS_AudioUpstreamNode *pNode;
    NEXUS_AudioInputData *pDestinationData;

    BDBG_MSG(("NEXUS_AudioInput_P_RemoveAllInputs(%p)", (void *)destination));

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

static NEXUS_Error NEXUS_AudioInput_P_Disconnect(NEXUS_AudioInput destination, NEXUS_AudioInput source)
{
    NEXUS_Error errCode;

    /* This routine triggers disconnection callbacks both up and down stream */

    BDBG_ASSERT(NULL != destination);
    BDBG_ASSERT(NULL != source);

    BDBG_MSG(("NEXUS_AudioInput_P_Disconnect(%p,%p)", (void *)destination, (void *)source));

    errCode = NEXUS_AudioInput_P_DisconnectDownstream(destination, source);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    errCode = NEXUS_AudioInput_P_DisconnectUpstream(destination);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_AudioInput_P_DisconnectUpstream(NEXUS_AudioInput source)
{
    NEXUS_AudioUpstreamNode *pNode;
    NEXUS_AudioInputData *pData;
    NEXUS_Error errCode;

    /* This routine triggers disconnection callbacks upstream */

    BDBG_ASSERT(NULL != source);

    BDBG_MSG(("NEXUS_AudioInput_P_DisconnectUpstream(%p)", (void *)source));

    pData = source->pMixerData;
    if ( NULL == pData )
    {
        return BERR_SUCCESS;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    /* Fire disconnect callback first to achieve reverse-order (tail->head) */
    if ( source->disconnectCb )
    {
        if ( source->module && source->module != NEXUS_MODULE_SELF )
        {
            NEXUS_Module_Lock(source->module);
        }
        errCode = source->disconnectCb(source->pObjectHandle, source);
        if ( source->module && source->module != NEXUS_MODULE_SELF )
        {
            NEXUS_Module_Unlock(source->module);
        }
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Recurse */
    errCode=BERR_SUCCESS;
    for ( pNode = BLST_Q_FIRST(&pData->upstreamList);
          NULL != pNode;
          pNode = BLST_Q_NEXT(pNode, upstreamNode) )
    {
        errCode = NEXUS_AudioInput_P_DisconnectUpstream(pNode->pUpstreamObject);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            break;
        }
    }

    return errCode;
}

static NEXUS_Error NEXUS_AudioInput_P_DisconnectDownstream(NEXUS_AudioInput destination, NEXUS_AudioInput source)
{
    NEXUS_AudioDownstreamNode *pNode;
    NEXUS_AudioInputData *pData;
    NEXUS_Error errCode;

    /* This routine triggers disconnection callbacks down stream */

    BDBG_ASSERT(NULL != destination);

    BDBG_MSG(("NEXUS_AudioInput_P_DisconnectDownstream(%p)", (void *)destination));

    pData = destination->pMixerData;
    if ( NULL == pData )
    {
        return BERR_SUCCESS;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    /* Recurse first */
    for ( pNode = BLST_Q_FIRST(&pData->downstreamList);
          NULL != pNode;
          pNode = BLST_Q_NEXT(pNode, downstreamNode) )
    {
        errCode = NEXUS_AudioInput_P_DisconnectDownstream(pNode->pDownstreamObject, destination);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Fire disconnect callback last to achieve reverse-order (tail->head) */
    if ( destination->disconnectCb )
    {
        if ( destination->module && destination->module != NEXUS_MODULE_SELF )
        {
            NEXUS_Module_Lock(destination->module);
        }
        errCode = destination->disconnectCb(destination->pObjectHandle, source);
        if ( destination->module && destination->module != NEXUS_MODULE_SELF )
        {
            NEXUS_Module_Unlock(destination->module);
        }
        if ( errCode )
        {
            return BERR_TRACE(errCode);
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
    NEXUS_Error errCode;

    BDBG_MSG(("NEXUS_AudioInput_P_RemoveInput(%p,%p)", (void *)destination, (void *)source));

    pDestinationData = destination->pMixerData;
    BDBG_OBJECT_ASSERT(pDestinationData, NEXUS_AudioInputData);    /* It's impossible for this to be NULL in a valid config. */

    /* On 7440/3563/7405+ architectures, inputs must be stopped for this to proceed. */
    if ( NEXUS_AudioInput_P_IsRunning(destination) )
    {
        BDBG_ERR(("Can not remove inputs from a node while it's running."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Objects connected before decoder do not require this -- channels will not change */
    if ( destination->objectType != NEXUS_AudioInputType_eDecoder )
    {
        BDBG_MSG(("Disconnecting"));

        /* Remove all destinations before potentially destroying associations */
        NEXUS_AudioInput_P_RemoveDestinations(destination);

        /* Trigger Disconnection Callbacks downstream */
        errCode = NEXUS_AudioInput_P_Disconnect(destination, source);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    else
    {
        BDBG_MSG(("Destination is a decoder.  No need to disconnect or invalidate associations"));
    }

    BDBG_MSG(("Unlink upstream link"));

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

    BDBG_MSG(("NEXUS_AudioInput_P_IsConnected(%p,%p)", (void *)destination, (void *)source));

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

BRAP_ChannelHandle NEXUS_AudioInput_P_GetChannel(NEXUS_AudioInput input, NEXUS_AudioInput *pChannelInput)
{
    NEXUS_AudioUpstreamNode *pNode;
    NEXUS_AudioInputData *pData;

    BDBG_ASSERT(NULL != input);

    BDBG_MSG(("NEXUS_AudioInput_P_GetChannel(%p)", (void *)input));

    if ( pChannelInput )
    {
        *pChannelInput = input;
    }

    switch ( input->objectType )
    {
#if NEXUS_NUM_AUDIO_DECODERS
    case NEXUS_AudioInputType_eDecoder:
        return NEXUS_AudioDecoder_P_GetChannel(input->pObjectHandle);
#endif
#if NEXUS_NUM_AUDIO_PLAYBACKS
    case NEXUS_AudioInputType_ePlayback:
        return NEXUS_AudioPlayback_P_GetChannel(input->pObjectHandle);
#endif
#if NEXUS_NUM_I2S_INPUTS
    case NEXUS_AudioInputType_eI2s:
        return NEXUS_I2sInput_P_GetChannel(input->pObjectHandle);
#endif
#if NEXUS_NUM_RF_AUDIO_DECODERS
    case NEXUS_AudioInputType_eRfDecoder:
        return NEXUS_RfAudioDecoder_P_GetChannel(input->pObjectHandle);
#endif
#if NEXUS_NUM_ANALOG_AUDIO_DECODERS
    case NEXUS_AudioInputType_eAnalogDecoder:
        return NEXUS_AnalogAudioDecoder_P_GetChannel(input->pObjectHandle);
#endif
#if NEXUS_NUM_MIXERS
    case NEXUS_AudioInputType_eMixer:
        /* Mixers ALWAYS return NULL for channel handle, as they have multiple inputs. */
        return NULL;
#endif
    default:
        /* Recurse if not a decoder or mixer type, all other types take only a single input */
        pData = input->pMixerData;
        BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);
        pNode = BLST_Q_FIRST(&pData->upstreamList);
        if ( NULL == pNode )
        {
            if ( pChannelInput )
            {
                *pChannelInput = NULL;
            }
            return NULL;
        }
        return NEXUS_AudioInput_P_GetChannel(pNode->pUpstreamObject, pChannelInput);
    }

    /* Can never get here */
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

    BDBG_MSG(("NEXUS_AudioInput_P_IterateOutputs(%p)", (void *)input));

    /* First, iterate through any outputs directly attached to this object */
    i = *pIndex;
    pData = input->pMixerData;
    BDBG_MSG(("Scanning node %p for outputs", (void *)input));
    if ( NULL == pData || i >= NEXUS_AUDIO_MAX_OUTPUTS )
    {
        if ( NULL == pData )
        {
            BDBG_MSG(("No connection data at node %p", (void *)input));
        }
        else
        {
            BDBG_MSG(("Output list full at node %p", (void *)input));
        }
        goto done;
    }
    pOutputNode = BLST_Q_FIRST(&pData->outputList);
    while ( NULL != pOutputNode && i < NEXUS_AUDIO_MAX_OUTPUTS )
    {
        BDBG_ASSERT(NULL != pOutputNode->pOutputConnector);
        pOutputList->outputs[i++] = pOutputNode->pOutputConnector;
        BDBG_MSG(("Found output %p at node %p", (void *)pOutputNode->pOutputConnector, (void *)input));
        pOutputNode = BLST_Q_NEXT(pOutputNode, outputNode);
    }

    /* Now recurse through all downstream connections */
    pDownstreamNode = BLST_Q_FIRST(&pData->downstreamList);
    while ( NULL != pDownstreamNode && i < NEXUS_AUDIO_MAX_OUTPUTS )
    {
        if ( (pDownstreamNode->pDownstreamObject->objectType != NEXUS_AudioInputType_eMixer) ||
             true == scanMixers )
        {
            BDBG_MSG(("Recursing into downstream node %p", (void *)pDownstreamNode->pDownstreamObject));
            errCode = NEXUS_AudioInput_P_IterateOutputs(pDownstreamNode->pDownstreamObject, pOutputList, &i, scanMixers);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto done;
            }
        }
        else
        {
            BDBG_MSG(("Skipping downstream mixer %p", (void *)pDownstreamNode->pDownstreamObject));
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

    BDBG_MSG(("NEXUS_AudioInput_P_GetOutputs(%p)", (void *)input));

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

    BDBG_MSG(("Connecting output %p to input %p", (void *)output, (void *)input));

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
    pNode->association = NULL;
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
    if ( NEXUS_AudioOutput_P_IsSpdifSlave(pNode->pOutputConnector) )
    {
        NEXUS_AudioOutput_P_SetArcSlaveSource(pNode->pOutputConnector, NULL);
    }

    /*
       If a DAC is removed, the slave source will be re-evaluated at next start call
       (During AddDestinations).  No need to check that here.
    */

    BLST_Q_REMOVE(&pInputData->outputList, pNode, outputNode);
    if ( pNode->association )
    {
        NEXUS_AudioAssociation_P_RemoveOutput(pNode->association, pNode->pOutputConnector);
    }
    BKNI_Memset(pNode, 0, sizeof(*pNode));
    BKNI_Free(pNode);

    /* Trigger upstream disconnections if this output was removed.  If it was the last output */
    /* using a channel or association, the resource should be freed.  In effect, this will */
    /* remove all open destinations and detroy associations.  They will be re-constructed on */
    /* the next start call if required */
    NEXUS_AudioInput_P_RemoveDestinations(input);
    NEXUS_AudioInput_P_DisconnectUpstream(input);

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

    BDBG_MSG(("Shutting down connector %p", (void *)input));

    pInputData = input->pMixerData;

    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_WRN(("Forcibly stopping inputs to input %p on shutdown", (void *)input));
        NEXUS_AudioInput_P_ForceStop(input);
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
            BDBG_MSG(("Forcibly breaking connection between input %p and %p on shutdown", (void *)pDownstreamNode->pDownstreamObject, (void *)input));
            rc = NEXUS_AudioInput_P_RemoveInput(pDownstreamNode->pDownstreamObject, input);
            if (rc) {
                rc = BERR_TRACE(rc);
                break;
            }
        }
        /* Remove all outputs */
        while ( NULL != (pNode = BLST_Q_FIRST(&pInputData->outputList)) )
        {
            BDBG_MSG(("Forcibly removing output %p on shutdown (port=%d)", (void *)pNode->pOutputConnector, pNode->pOutputConnector->port));
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

NEXUS_Error NEXUS_AudioInput_P_RemoveDestinations(NEXUS_AudioInput input)
{
    NEXUS_Error errCode;
    /* The goal of this routine is to remove all destinations up and downstream from a specific input */

    BDBG_ASSERT(NULL != input);

    BDBG_MSG(("NEXUS_AudioInput_P_RemoveDestinations(%p)", (void *)input));

    errCode = NEXUS_AudioInput_P_RemoveDestinationsDownstream(input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = NEXUS_AudioInput_P_RemoveDestinationsUpstream(input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_AudioInput_P_RemoveDestinationsDownstream(NEXUS_AudioInput input)
{
    /* The goal of this routine is to remove all destinations downstream from a specific input */
    NEXUS_AudioInputData *pData;
    NEXUS_AudioOutputNode *pOutputNode;
    NEXUS_AudioDownstreamNode *pDownstreamNode;

    BDBG_ASSERT(NULL != input);

    BDBG_MSG(("NEXUS_AudioInput_P_RemoveDestinationsDownstream(%p)", (void *)input));

    pData = input->pMixerData;

    if ( NULL == pData )
    {
        return BERR_SUCCESS;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    /* Remove my destinations first */
    for ( pOutputNode = BLST_Q_FIRST(&pData->outputList);
          NULL != pOutputNode;
          pOutputNode = BLST_Q_NEXT(pOutputNode, outputNode) )
    {
        if ( pOutputNode->association )
        {
            NEXUS_AudioAssociation_P_RemoveOutput(pOutputNode->association, pOutputNode->pOutputConnector);
            pOutputNode->association = NULL;
        }
    }

    /* Recurse through downstream nodes */
    for ( pDownstreamNode = BLST_Q_FIRST(&pData->downstreamList);
          NULL != pDownstreamNode;
          pDownstreamNode = BLST_Q_NEXT(pDownstreamNode, downstreamNode) )
    {
        NEXUS_AudioInput_P_RemoveDestinationsDownstream(pDownstreamNode->pDownstreamObject);
    }

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_AudioInput_P_RemoveDestinationsUpstream(NEXUS_AudioInput input)
{
    /* The goal of this routine is to remove all destinations downstream from a specific input */
    NEXUS_AudioInputData *pData;
    NEXUS_AudioOutputNode *pOutputNode;
    NEXUS_AudioUpstreamNode *pUpstreamNode;

    BDBG_ASSERT(NULL != input);

    BDBG_MSG(("NEXUS_AudioInput_P_RemoveDestinationsUpstream(%p)", (void *)input));

    pData = input->pMixerData;

    if ( NULL == pData )
    {
        return BERR_SUCCESS;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    /* Remove my destinations first */
    for ( pOutputNode = BLST_Q_FIRST(&pData->outputList);
          NULL != pOutputNode;
          pOutputNode = BLST_Q_NEXT(pOutputNode, outputNode) )
    {
        if ( pOutputNode->association )
        {
            NEXUS_AudioAssociation_P_RemoveOutput(pOutputNode->association, pOutputNode->pOutputConnector);
            pOutputNode->association = NULL;
        }
    }

    /* Recurse through upstream nodes */
    for ( pUpstreamNode = BLST_Q_FIRST(&pData->upstreamList);
          NULL != pUpstreamNode;
          pUpstreamNode = BLST_Q_NEXT(pUpstreamNode, upstreamNode) )
    {
        NEXUS_AudioInput_P_RemoveDestinationsUpstream(pUpstreamNode->pUpstreamObject);
    }

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_AudioInput_P_AddDestinations(
    NEXUS_AudioInput input,
    NEXUS_AudioAssociationHandle association,
    bool downmix
    )
{
    NEXUS_AudioDownstreamNode *pDownstreamNode;
    NEXUS_AudioOutputNode *pOutputNode;
    NEXUS_AudioInputData *pData;
    NEXUS_Error errCode;

    BDBG_MSG(("NEXUS_AudioInput_P_AddDestinations(%p)", (void *)input));

    BDBG_ASSERT(NULL != input);

    pData = input->pMixerData;
    if ( NULL == pData )
    {
        return BERR_SUCCESS;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    BDBG_MSG(("Scanning node type %s for outputs", g_pTypeNames[input->objectType]));

    /* DDRE changes format on the fly.  You need to make sure downmix is applied for stereo outputs. */
    if ( input->objectType == NEXUS_AudioInputType_eDolbyDigitalReencode &&
         input->format == NEXUS_AudioInputFormat_ePcmStereo )
    {
        downmix = true;
    }

    /* Add any outputs directly connected to this node */
    for ( pOutputNode = BLST_Q_FIRST(&pData->outputList);
          NULL != pOutputNode;
          pOutputNode = BLST_Q_NEXT(pOutputNode, outputNode) )
    {
        if ( pOutputNode->association && pOutputNode->association != association )
        {
            BDBG_MSG(("Association has changed.  Removing output %p from old association %p", (void *)pOutputNode->pOutputConnector, (void *)pOutputNode->association));
            NEXUS_AudioAssociation_P_RemoveOutput(pOutputNode->association, pOutputNode->pOutputConnector);
            pOutputNode->association = NULL;
        }
        if ( NULL == pOutputNode->association )
        {
            /* For outputs that slave to a DAC (e.g. RFM), find another DAC at this same connection point to bind with */
            /* Or for outputs that slave to a SPDIF output (e.g. ARC), find the SPDIF at this same connection point to bind with */
            if ( NEXUS_AudioOutput_P_IsDacSlave(pOutputNode->pOutputConnector) ||
                 NEXUS_AudioOutput_P_IsSpdifSlave(pOutputNode->pOutputConnector) )
            {
                NEXUS_AudioOutputNode *pMasterOutputNode;
                for ( pMasterOutputNode = BLST_Q_FIRST(&pData->outputList);
                      NULL != pMasterOutputNode;
                      pMasterOutputNode = BLST_Q_NEXT(pMasterOutputNode, outputNode) )
                {
                    if ( pMasterOutputNode->pOutputConnector->objectType == NEXUS_AudioOutputType_eDac &&
                         NEXUS_AudioOutput_P_IsDacSlave(pOutputNode->pOutputConnector))
                    {
                        /* Found DAC to bind with */
                        BDBG_MSG(("Binding dac slave %p to dac %p", (void *)pOutputNode->pOutputConnector, (void *)pMasterOutputNode->pOutputConnector));
                        NEXUS_AudioOutput_P_SetSlaveSource(pOutputNode->pOutputConnector, pMasterOutputNode->pOutputConnector);
                        break;
                    }
                    else if ( pMasterOutputNode->pOutputConnector->objectType == NEXUS_AudioOutputType_eSpdif &&
                              NEXUS_AudioOutput_P_IsSpdifSlave(pOutputNode->pOutputConnector))
                    {
                        /* Found SPDIF to bind with */
                        BDBG_MSG(("Binding ARC slave %p to SPDIF %p", (void *)pOutputNode->pOutputConnector, (void *)pMasterOutputNode->pOutputConnector));
                        NEXUS_AudioOutput_P_SetArcSlaveSource(pOutputNode->pOutputConnector, pMasterOutputNode->pOutputConnector);
                        break;
                    }
                }
                if ( NULL == pMasterOutputNode )
                {
                    if ( NEXUS_AudioOutput_P_IsDacSlave(pOutputNode->pOutputConnector) )
                    {
                        BDBG_ERR(("Audio Output %p requires a DAC to be connected to the same node.  Audio will be disabled for that output until a DAC is attached.",
                                  (void *)pOutputNode->pOutputConnector));
                        NEXUS_AudioOutput_P_SetSlaveSource(pOutputNode->pOutputConnector, NULL);
                    }
                    else
                    {
                        BDBG_ERR(("Audio Output %p requires a SPDIF to be connected to the same node.  Audio will be disabled for that output until a SPDIF is attached.",
                                  (void *)pOutputNode->pOutputConnector));
                        NEXUS_AudioOutput_P_SetArcSlaveSource(pOutputNode->pOutputConnector, NULL);
                    }
                }
            }
            else
            {
                BDBG_MSG(("Adding destination at node type %s (output port=%d)", g_pTypeNames[input->objectType], pOutputNode->pOutputConnector->port));
                BDBG_ASSERT(NULL != association);
                errCode = NEXUS_AudioAssociation_P_AddOutput(association, NEXUS_AudioInput_P_GetFormat(input), pOutputNode->pOutputConnector, downmix);
                if ( errCode )
                {
                    return BERR_TRACE(errCode);
                }
                pOutputNode->association = association;
                errCode = NEXUS_AudioOutput_P_ApplySettings(pOutputNode->pOutputConnector);
                if ( errCode )
                {
                    errCode = BERR_TRACE(errCode);
                }
            }
        }
    }

    /* Recurse */
    for ( pDownstreamNode = BLST_Q_FIRST(&pData->downstreamList);
          NULL != pDownstreamNode;
          pDownstreamNode = BLST_Q_NEXT(pDownstreamNode, downstreamNode) )
    {
        /* Don't recurse through mixers.  They have their own associations */
        if ( pDownstreamNode->pDownstreamObject->objectType == NEXUS_AudioInputType_eMixer )
        {
            /* This will be handled by NEXUS_AudioInput_P_Connect later on. */
        }
        else
        {
            errCode = NEXUS_AudioInput_P_AddDestinations(pDownstreamNode->pDownstreamObject, association, downmix);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
    }

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_AudioInput_P_Connect(NEXUS_AudioInput input)
{
    NEXUS_AudioDownstreamNode *pNode;
    NEXUS_AudioInputData *pData;
    NEXUS_Error errCode;

    BDBG_ASSERT(NULL != input);

    BDBG_MSG(("NEXUS_AudioInput_P_Connect(%p)", (void *)input));

    pData = input->pMixerData;

    if ( NULL == pData )
    {
        return BERR_SUCCESS;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);
    errCode=BERR_SUCCESS;
    for ( pNode = BLST_Q_FIRST(&pData->downstreamList);
          NULL != pNode;
          pNode = BLST_Q_NEXT(pNode, downstreamNode) )
    {
        /* Make link from this node to downstream node */
        if ( pNode->pDownstreamObject->connectCb )
        {
            if ( pNode->pDownstreamObject->module && pNode->pDownstreamObject->module != NEXUS_MODULE_SELF )
            {
                NEXUS_Module_Lock(pNode->pDownstreamObject->module);
            }
            pNode->pDownstreamObject->connectCb(pNode->pDownstreamObject->pObjectHandle, input);
            if ( pNode->pDownstreamObject->module && pNode->pDownstreamObject->module != NEXUS_MODULE_SELF )
            {
                NEXUS_Module_Unlock(pNode->pDownstreamObject->module);
            }
        }
        /* Recurse */
        errCode = NEXUS_AudioInput_P_Connect(pNode->pDownstreamObject);
        if ( errCode )
        {
            errCode= BERR_TRACE(errCode);
            break;
        }
    }
    return errCode;
}


/***************************************************************************
Summary:
    Prepare the input chain to start.  May build and/or validate connections.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_PrepareToStart(
    NEXUS_AudioInput input,
    NEXUS_AudioAssociationHandle association
    )
{
#if BRAP_VER >= 4
    return NEXUS_AudioInput_P_PrepareToStartWithProcessing(input, association, NULL, 0, NULL, NULL, false, NEXUS_AudioCodec_eUnknown);
#else
    return NEXUS_AudioInput_P_PrepareToStartWithProcessing(input, association, NULL, 0, NULL, NULL, true, NEXUS_AudioCodec_eUnknown);
#endif
}

NEXUS_Error NEXUS_AudioInput_P_PrepareToStartWithProcessing(
    NEXUS_AudioInput input,
    NEXUS_AudioAssociationHandle association,
    BRAP_ProcessingStageHandle *pStages,
    int numStages,
    BRAP_ProcessingStageHandle descriptorStage,
    BRAP_ChannelHandle descriptorPair,
    bool downmix,
    NEXUS_AudioCodec codec
    )
{
    NEXUS_Error errCode;
    NEXUS_AudioInputData *pData;
    BRAP_ProcessingSettings preMixingStages, postMixingStages;
    int numPreMixingStages=0, numPostMixingStages=0;
    int branches;

    BDBG_ASSERT(NULL != input);

    BDBG_MSG(("NEXUS_AudioInput_P_PrepareToStart(%p)", (void *)input));

    pData = input->pMixerData;
    if ( NULL == pData )
    {
        /* If no connections exist, there is nothing to do */
        return BERR_SUCCESS;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    #if BDBG_DEBUG_BUILD
    /* Before starting, check if the max number of processing branches has been exceeded. */
    branches = (int)NEXUS_AudioInput_P_GetNumOutputPaths(input)-1;
    if ( branches > g_NEXUS_audioModuleData.moduleSettings.maximumProcessingBranches )
    {
        BDBG_ERR(("Maximum number of post-processing branches exceeded.  %d branches detected, %d allowed.",
                  branches, g_NEXUS_audioModuleData.moduleSettings.maximumProcessingBranches));
        BDBG_ERR(("Please increase the value of NEXUS_AudioModuleSettings.maximumProcessingBranches or use fewer branches."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    #else
    BSTD_UNUSED(branches);
    #endif

    switch ( input->objectType )
    {
    case NEXUS_AudioInputType_eDecoder:
    case NEXUS_AudioInputType_ePlayback:
    case NEXUS_AudioInputType_eI2s:
    case NEXUS_AudioInputType_eRfDecoder:
    case NEXUS_AudioInputType_eMixer:
    case NEXUS_AudioInputType_eAnalogDecoder:
        /* Add destinations that are directly connected to my association */
        errCode = NEXUS_AudioInput_P_AddDestinations(input, association, downmix);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        /* Call connect calls downstream */
        errCode = NEXUS_AudioInput_P_Connect(input);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        if ( input->objectType == NEXUS_AudioInputType_eMixer )
        {
            /* Don't re-add post-processing after mixing.  It was done earlier. */
            break;
        }
        /* After destinations have been added, apply post processing as required */
        BRAP_GetDefaultPostProcessingStages(&preMixingStages);
        BRAP_GetDefaultPostProcessingStages(&postMixingStages);
        if ( NULL != pStages )
        {
            for ( numPreMixingStages = 0; 
                  numPreMixingStages < numStages && numPreMixingStages < BRAP_MAX_PP_PER_BRANCH_SUPPORTED; 
                  numPreMixingStages++ )
            {
                BDBG_MSG(("Adding stage %p from decoder", (void *)pStages[numPreMixingStages]));
                preMixingStages.hAudProcessing[numPreMixingStages] = pStages[numPreMixingStages];
            }
        }
        BDBG_MSG(("Checking for post processing on input %p [channel %p]", (void *)input, (void *)NEXUS_AudioInput_P_GetChannel(input, NULL)));
        errCode = NEXUS_AudioInput_P_AddPostProcessing(input,
                                                       NEXUS_AudioInput_P_GetChannel(input, NULL), false, false,
                                                       &preMixingStages, numPreMixingStages,
                                                       &postMixingStages, numPostMixingStages,
                                                       descriptorStage, descriptorPair, codec);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        break;
    default:
        BDBG_ERR(("Connector type %d can not be started explicitly", input->objectType));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
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
    Validate if an output has a valid destination at the specified input.
 ***************************************************************************/
BRAP_DestinationHandle NEXUS_AudioInput_P_GetOutputDestination(
    NEXUS_AudioInput input,
    NEXUS_AudioOutputHandle output
    )
{
    NEXUS_AudioInputData *pData;

    BDBG_ASSERT(NULL != input);
    BDBG_ASSERT(NULL != output);

    pData = input->pMixerData;

    if ( NULL != pData )
    {
        NEXUS_AudioOutputNode *pNode;

        BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);
        for ( pNode = BLST_Q_FIRST(&pData->outputList);
              NULL != pNode;
              pNode = BLST_Q_NEXT(pNode, outputNode) )
        {
            if ( pNode->pOutputConnector == output )
            {
                if ( pNode->association )
                {
                    return NEXUS_AudioAssociation_P_GetOutputDestination(pNode->association, pNode->pOutputConnector, NULL);
                }
                break;
            }
        }
    }

    BDBG_MSG(("Output %p not connected to input %p", (void *)output, (void *)input));
    return NULL;
}

/***************************************************************************
Summary:
    Invalidate all downstream destinations and any mixer associations
    related to this object.  The channel is being closed or being repurposed.
 ***************************************************************************/
void NEXUS_AudioInput_P_InvalidateConnections(
    NEXUS_AudioInput input
    )
{
    NEXUS_AudioInputData *pData;

    BDBG_ASSERT(NULL != input);

    BDBG_MSG(("Invalidating input %p (type=%s)", (void *)input, g_pTypeNames[input->objectType]));

    /* Remove all downstream destinations */
    NEXUS_AudioInput_P_RemoveDestinations(input);

    pData = input->pMixerData;
    if ( NULL != pData )
    {
        NEXUS_AudioDownstreamNode *pNode;

        BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);
        /* For each destination, trigger disconnections - this will delete any downstream data */
        for ( pNode = BLST_Q_FIRST(&pData->downstreamList);
              NULL != pNode;
              pNode = BLST_Q_NEXT(pNode, downstreamNode) )
        {
            NEXUS_AudioInput_P_Disconnect(pNode->pDownstreamObject, input);
        }
    }
}

/***************************************************************************
Summary:
    Get association settings for an object.  This is used by mixer to
    to determine all upstream raptor channels.
 ***************************************************************************/
NEXUS_Error NEXUS_AudioInput_P_GetAssociationSettings(
    NEXUS_AudioInput input,
    BRAP_AssociatedChanSettings *pSettings
    )
{
    int numChannels=0;
    BRAP_ChannelHandle channel;
    NEXUS_AudioInputData *pData;
    NEXUS_AudioUpstreamNode *pNode;
    NEXUS_AudioInput inputArray[BRAP_MAX_ASSOCIATED_CHANNELS_IN_GRP];

    BDBG_ASSERT(NULL != input);
    BRAP_GetDefaultAssociationSettings(g_NEXUS_audioModuleData.hRap, pSettings);
    BKNI_Memset(inputArray, 0, sizeof(inputArray));

    pData = input->pMixerData;
    if ( NULL == pData )
    {
        BDBG_WRN(("No Channels connected to this object"));
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    for ( pNode = BLST_Q_FIRST(&pData->upstreamList);
          NULL != pNode;
          pNode = BLST_Q_NEXT(pNode, upstreamNode) )
    {
        channel = NEXUS_AudioInput_P_GetChannel(pNode->pUpstreamObject, &inputArray[numChannels]);
        if ( channel )
        {
            BDBG_MSG(("Found channel %d %p, input %p, type %d", numChannels, (void *)channel, (void *)inputArray[numChannels], inputArray[numChannels]->objectType));
            pSettings->sChDetail[numChannels++].hRapCh = channel;
        }
    }

    if ( NEXUS_GetEnv("audio_mixing_disabled") )
    {
        /* Remove any playback channels if there are more than one total channels */
        if ( numChannels > 1 )
        {
            int i;
            for ( i = 0; i < numChannels; )
            {
                if ( inputArray[i]->objectType == NEXUS_AudioInputType_ePlayback )
                {
                    int j;
                    /* remove this channel from the association */
                    BDBG_WRN(("Removing AudioPlayback %p from mixing", inputArray[i]->pObjectHandle));
                    for ( j=i+1; j<numChannels; j++ )
                    {
                        inputArray[j-1] = inputArray[j];
                        pSettings->sChDetail[j-1].hRapCh = pSettings->sChDetail[j].hRapCh;
                    }
                    numChannels--;
                    inputArray[numChannels] = NULL;
                    pSettings->sChDetail[numChannels].hRapCh = NULL;
                    continue;
                }
                i++;
            }
        }
    }

    if ( numChannels == 0 )
    {
        BDBG_WRN(("No valid inputs connected to this object"));
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_AudioInput_P_AddPostProcessing(NEXUS_AudioInput input,
                                                        BRAP_ChannelHandle channel, bool slave, bool postMixing,
                                                        BRAP_ProcessingSettings *pPreMixingStages, int numPreMixingStages,
                                                        BRAP_ProcessingSettings *pPostMixingStages, int numPostMixingStages,
                                                        BRAP_ProcessingStageHandle descriptorStage, BRAP_ChannelHandle descriptorPair,
                                                        NEXUS_AudioCodec codec
                                                        )
{
    NEXUS_AudioDownstreamNode *pNode;
    NEXUS_AudioInputData *pData;
    BRAP_ProcessingStageHandle stage=NULL, stage2=NULL, stage3=NULL, stage4=NULL;
    NEXUS_AudioOutputNode *pOutputNode;
    BRAP_ProcessingSettings adProcessingStages;
    NEXUS_AudioUpstreamNode *pUpstreamNode;
    NEXUS_Error errCode;

    BDBG_MSG(("%s(%d) postMixing %d numPreMixingStages %d numPostMixingStages %d",
        __FUNCTION__, __LINE__, postMixing, numPreMixingStages, numPostMixingStages));

    pData = input->pMixerData;
    if ( NULL == pData )
    {
        return BERR_SUCCESS;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);


    if ( !slave )
    {
        BDBG_MSG(("input->objectType %d",input->objectType));
        switch ( input->objectType )
        {
    #ifdef NEXUS_CVOICE
        case NEXUS_AudioInputType_eCustomVoice:
            stage = NEXUS_CustomVoice_P_GetStageHandle(input->pObjectHandle);
            break;
    #endif
        case NEXUS_AudioInputType_eAutoVolumeLevel:
            stage = NEXUS_AutoVolumeLevel_P_GetStageHandle(input->pObjectHandle);
            break;
        case NEXUS_AudioInputType_e3dSurround:
            stage = NEXUS_3dSurround_P_GetStageHandle(input->pObjectHandle);
            break;
        case NEXUS_AudioInputType_eDtsEncode:
            stage = NEXUS_DtsEncode_P_GetStageHandle(input->pObjectHandle);
            break;
        case NEXUS_AudioInputType_eAc3Encode:
            stage = NEXUS_Ac3Encode_P_GetStageHandle(input->pObjectHandle, codec);
            break;
        case NEXUS_AudioInputType_eTruVolume:
            stage = NEXUS_TruVolume_P_GetStageHandle(input->pObjectHandle);
            break;
        case NEXUS_AudioInputType_eEncoder:
            stage = NEXUS_AudioEncoder_P_GetStageHandle(input->pObjectHandle);
            break;
    #if BCHP_CHIP != 3548 && BCHP_CHIP != 3556 && BRAP_VER < 4
        case NEXUS_AudioInputType_eDolbyVolume:
            stage = NEXUS_DolbyVolume_P_GetStageHandle(input->pObjectHandle);
            break;
        case NEXUS_AudioInputType_eDolbyVolume258:
            stage = NEXUS_DolbyVolume258_P_GetStageHandle(input->pObjectHandle);
            break;
        case NEXUS_AudioInputType_eDolbyDigitalReencode:
            stage = NEXUS_DolbyDigitalReencode_P_GetStageHandle(input->pObjectHandle, input, &stage2);
            break;
        case NEXUS_AudioInputType_eRfEncoder:
            stage = NEXUS_RfAudioEncoder_P_GetStageHandle(input->pObjectHandle);
            break;
    #endif
    #if NEXUS_NUM_AUDIO_MIXERS
        case NEXUS_AudioInputType_eMixer:
            /* Search the mixer for the AD Paired channel */
            for ( pUpstreamNode = BLST_Q_FIRST(&pData->upstreamList);
                  NULL != pUpstreamNode;
                  pUpstreamNode = BLST_Q_NEXT(pUpstreamNode, upstreamNode) )
            {
                NEXUS_AudioInput tmp;            
                if ( descriptorPair == NEXUS_AudioInput_P_GetChannel(pUpstreamNode->pUpstreamObject, &tmp) )
                {
                    int i;
                    BRAP_GetDefaultPostProcessingStages(&adProcessingStages);
                    adProcessingStages.hAudProcessing[0] = descriptorStage;
                    for ( i = 0; i < numPreMixingStages && i < (BRAP_MAX_PP_PER_BRANCH_SUPPORTED-1); i++ )
                    {
                        adProcessingStages.hAudProcessing[i+1] = pPreMixingStages->hAudProcessing[i];
                    }
                    pPreMixingStages = &adProcessingStages;
                    numPreMixingStages = i+1;
                    break;
                }
            }

            stage = NEXUS_AudioMixer_P_GetStageHandle(input->pObjectHandle);
        #if BRAP_VER == 3
            /* In version 3 raptor, FW Mixer and all subsequent stages are added post-mixng since the
               FW Mixer runs as a separate DSP task. */
            postMixing = true;            
        #else
            /* In version 4 raptor, FW Mixer runs as part of the master task only */
            /* Post-processing done after mixing is done as a separate DSP task but the
               mixing is done in the FMM not the DSP.  Need to distinguish which case we've
               encountered. */
            if ( stage )
            {
                /* FW Mixer is being used.  Determine whether the current input is master or slave. */
                if ( NEXUS_AudioMixer_P_IsSlaveChannel(input->pObjectHandle, channel) )
                {
                    slave = true;
                    stage = NULL;   /* Don't apply FW Mixer or any subsequent stages to the slave task. */
                }
                else
                {
                    /* Master Task */
                }
            }
            else
            {
                /* HW Mixer.  Subsequent stages will be post-mixing in the RAP sense. */
                postMixing = true;
                /* TODO: Need to fix BRAP_SetPostProcessingStages() for NULL association. Currently, it causes BRAP_StartChannel() to fail. */
            }
        #endif
            break;
    #endif
        default:
            break;
        }
    }

    BDBG_MSG(("%s(%d) stage %p postMixing %d",__FUNCTION__, __LINE__, (void *)stage, postMixing));

    if ( NULL != stage && !NEXUS_GetEnv("audio_processing_disabled") )
    {
        int numStages = 0;
        BRAP_ProcessingSettings *pStages = NULL;

        if ( postMixing )
        {
            numStages = numPostMixingStages;
            pStages = pPostMixingStages;
        }
        else
        {
            numStages = numPreMixingStages;
            pStages = pPreMixingStages;
        }

        if ( numStages >= BRAP_MAX_PP_PER_BRANCH_SUPPORTED )
        {
            BDBG_ERR(("Max number of post processing stages reached."));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
		if (NULL != pStages) {
	        pStages->hAudProcessing[numStages++] = stage;
	        if ( NULL != stage2 )
	        {
	            if ( numStages >= BRAP_MAX_PP_PER_BRANCH_SUPPORTED )
	            {
	                BDBG_ERR(("Max number of post processing stages reached."));
	                return BERR_TRACE(BERR_INVALID_PARAMETER);
	            }
	            pStages->hAudProcessing[numStages++] = stage2;
	        }
			
			#if NEXUS_HAS_AUDIO_POST_PROCESSING
	        if ( NULL != stage3 )
	        {
	            if ( numStages >= BRAP_MAX_PP_PER_BRANCH_SUPPORTED )
	            {
	                BDBG_ERR(("Max number of post processing stages reached."));
	                return BERR_TRACE(BERR_INVALID_PARAMETER);
	            }
	            pStages->hAudProcessing[numStages++] = stage3;
	        }
	        if ( NULL != stage4 )
	        {
	            if ( numStages >= BRAP_MAX_PP_PER_BRANCH_SUPPORTED )
	            {
	                BDBG_ERR(("Max number of post processing stages reached."));
	                return BERR_TRACE(BERR_INVALID_PARAMETER);
	            }
	            pStages->hAudProcessing[numStages++] = stage4;
	        }
			#else
			BSTD_UNUSED(stage3);
			BSTD_UNUSED(stage4);
			#endif
		}
        if ( postMixing )
        {
            numPostMixingStages = numStages;
        }
        else
        {
            numPreMixingStages = numStages;
        }
    }

    /* Apply current settings to all destinations on this node */
    for ( pOutputNode = BLST_Q_FIRST(&pData->outputList);
        NULL != pOutputNode;
        pOutputNode = BLST_Q_NEXT(pOutputNode, outputNode) )
    {
        if ( pOutputNode->association )
        {
            BRAP_DestinationHandle destination, privDestination=NULL;
            destination = NEXUS_AudioAssociation_P_GetOutputDestination(pOutputNode->association, pOutputNode->pOutputConnector, &privDestination);
            if ( NULL == destination )
            {
                BDBG_ERR(("Unable to get destination handle for output"));
                return BERR_TRACE(BERR_UNKNOWN);
            }
            else
            {
                int i;
                BRAP_ProcessingStageHandle outputProcessingStage;

                /* Certain outputs require a post-processing object.  Add that here. */
                outputProcessingStage = NEXUS_AudioOutput_P_GetProcessingStage(pOutputNode->pOutputConnector);
BDBG_MSG(("%s(%d) outputProcessingStage %p  postMixing %d, numPreMixingStages %d numPostMixingStages%d",__FUNCTION__, __LINE__, (void *)outputProcessingStage, postMixing, numPreMixingStages, numPostMixingStages));

                if ( outputProcessingStage )
                {
                    if ( postMixing )
                    {
                        if ( numPostMixingStages >= BRAP_MAX_PP_PER_BRANCH_SUPPORTED )
                        {
                            BDBG_ERR(("Unable to add required processing stage for output.  Please reduce the number of post-processing stages."));
                            return BERR_TRACE(BERR_NOT_SUPPORTED);
                        }
                        else
                        {
                            pPostMixingStages->hAudProcessing[numPostMixingStages++] = outputProcessingStage;
                        }
                    }
                    else
                    {
                        if ( numPreMixingStages >= BRAP_MAX_PP_PER_BRANCH_SUPPORTED )
                        {
                            BDBG_ERR(("Unable to add required processing stage for output.  Please reduce the number of post-processing stages."));
                            return BERR_TRACE(BERR_NOT_SUPPORTED);
                        }
                        else
                        {
                            pPreMixingStages->hAudProcessing[numPreMixingStages++] = outputProcessingStage;
                        }
                    }
                }
                
                /* Zero out unused slots for stages in cases of branching */
                for ( i = numPreMixingStages; i < BRAP_MAX_PP_PER_BRANCH_SUPPORTED; i++ )
                    pPreMixingStages->hAudProcessing[i] = NULL;
#if BDBG_DEBUG_BUILD
                for ( i = 0; i < numPreMixingStages; i++ )
                {
                    BDBG_MSG(("Post Processing Stage %p for destination %p (port %d)",
                              (void *)pPreMixingStages->hAudProcessing[i], (void *)destination, pOutputNode->pOutputConnector->port));
                }
#endif
                errCode = BRAP_SetPostProcessingStages(destination, channel, pPreMixingStages);
                if ( errCode )
                {
                    return BERR_TRACE(errCode);
                }
                if ( privDestination )
                {
                    errCode = BRAP_SetPostProcessingStages(privDestination, channel, pPreMixingStages);
                    if ( errCode )
                    {
                        return BERR_TRACE(errCode);
                    }
                }
		
                for ( i = numPostMixingStages; i < BRAP_MAX_PP_PER_BRANCH_SUPPORTED; i++ )
                    pPostMixingStages->hAudProcessing[i] = NULL;
                if ( !NEXUS_AudioInput_P_IsRunningUpstream(input) )
                {
#if BDBG_DEBUG_BUILD
                    for ( i = 0; i < numPostMixingStages; i++ )
                    {
                        BDBG_MSG(("Post Processing Stage %p for destination %p (port %d) post-mixing",
                                  (void *)pPostMixingStages->hAudProcessing[i], (void *)destination, pOutputNode->pOutputConnector->port));
                    }
#endif
                    errCode = BRAP_SetPostProcessingStages(destination, NULL, pPostMixingStages);
                    if ( errCode )
                    {
                        return BERR_TRACE(errCode);
                    }
                    if ( privDestination )
                    {
                        errCode = BRAP_SetPostProcessingStages(privDestination, NULL, pPostMixingStages);
                        if ( errCode )
                        {
                            return BERR_TRACE(errCode);
                        }
                    }                    
                }
            }
        }
    }

    /* Recurse through all children */
    errCode=BERR_SUCCESS;
    for ( pNode = BLST_Q_FIRST(&pData->downstreamList);
        NULL != pNode;
        pNode = BLST_Q_NEXT(pNode, downstreamNode) )
    {
        errCode = NEXUS_AudioInput_P_AddPostProcessing(pNode->pDownstreamObject,
                                                       channel, slave, postMixing,
                                                       pPreMixingStages, numPreMixingStages,
                                                       pPostMixingStages, numPostMixingStages,
                                                       descriptorStage, descriptorPair, codec);
        if ( errCode )
        {
            errCode=BERR_TRACE(errCode);
            break;
        }
    }

    return errCode;
}

NEXUS_AudioInput NEXUS_AudioInput_P_FindDirect(
    NEXUS_AudioInput input,
    NEXUS_AudioInputType type
    )
{
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
        }
    }

    /* Not found if we get here */
    return NULL;
}

unsigned NEXUS_AudioInput_P_GetNumDirectConnections(
    NEXUS_AudioInput input
    )
{
    NEXUS_AudioInputData *pData;
    NEXUS_AudioDownstreamNode *pNode;
    unsigned numConnections=0;

    BDBG_ASSERT(NULL != input);

    if ( NULL == input->pMixerData )
    {
        return 0;
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
            numConnections++;
        }
    }

    return numConnections;
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

/***************************************************************************
Summary:
    Returns the first object upstream from the current object that matches
    the specified type.  This is a depth-first search, not breadth-first.
 ***************************************************************************/
NEXUS_AudioInput NEXUS_AudioInput_P_FindUpstream(
    NEXUS_AudioInput input,
    NEXUS_AudioInputType type
    )
{
    NEXUS_AudioInput obj=NULL;
    NEXUS_AudioInputData *pData;
    NEXUS_AudioUpstreamNode *pNode;

    BDBG_ASSERT(NULL != input);

    if ( NULL == input->pMixerData )
    {
        return NULL;
    }

    pData = input->pMixerData;
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    /* Recurse through all children */
    for ( pNode = BLST_Q_FIRST(&pData->upstreamList);
        NULL != pNode;
        pNode = BLST_Q_NEXT(pNode, upstreamNode) )
    {
        if ( pNode->pUpstreamObject != NULL )
        {
            if ( pNode->pUpstreamObject->objectType == type )
            {
                return pNode->pUpstreamObject;
            }
            else
            {
                obj = NEXUS_AudioInput_P_FindUpstream(pNode->pUpstreamObject, type);
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

#if BDBG_DEBUG_BUILD
static unsigned NEXUS_AudioInput_P_GetNumOutputPaths(
    NEXUS_AudioInput input
    )
{
    unsigned paths=0;
    NEXUS_AudioInputData *pData;
    NEXUS_AudioDownstreamNode *pNode;

    BDBG_ASSERT(NULL != input);

    if ( NULL == input->pMixerData )
    {
        return 0;
    }

    pData = input->pMixerData;
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    /* If at least one output is connected at this node, increment the number of paths */
    if ( BLST_Q_FIRST(&pData->outputList) )
    {
        paths++;
    }
        
    /* Recurse through all children */
    for ( pNode = BLST_Q_FIRST(&pData->downstreamList);
        NULL != pNode;
        pNode = BLST_Q_NEXT(pNode, downstreamNode) )
    {
        if ( pNode->pDownstreamObject != NULL )
        {
            paths += NEXUS_AudioInput_P_GetNumOutputPaths(pNode->pDownstreamObject);
        }
    }

    /* Return total number found */
    return paths;
}
#endif

void NEXUS_AudioInput_P_ForceStop(NEXUS_AudioInput input)
{
    NEXUS_AudioDownstreamNode *pNode;
    NEXUS_AudioInputData *pData;

    pData = input->pMixerData;

    /* Can't be running if not connected */
    if ( NULL == pData )
    {
        return;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    NEXUS_AudioInput_P_ForceStopUpstream(input);

    /* Recurse Downstream */
    for ( pNode = BLST_Q_FIRST(&pData->downstreamList);
          NULL != pNode;
          pNode = BLST_Q_NEXT(pNode, downstreamNode) )
    {
        NEXUS_AudioInput_P_ForceStopDownstream(pNode->pDownstreamObject);
    }
}

static void NEXUS_AudioInput_P_ForceStopUpstream(NEXUS_AudioInput input)
{
    /* Stop any upstream components first */
    NEXUS_AudioUpstreamNode *pUpNode;
    NEXUS_AudioInputData *pData;

    BDBG_OBJECT_ASSERT(input, NEXUS_AudioInput);

    BDBG_MSG(("NEXUS_AudioInput_P_ForceStopUpstream(%p)", (void *)input));

    pData = input->pMixerData;

    /* Can't be running if not connected */
    if ( NULL == pData )
    {
        return;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    switch ( input->objectType )
    {
#if NEXUS_NUM_AUDIO_DECODERS
    case NEXUS_AudioInputType_eDecoder:
        if ( NEXUS_AudioDecoder_P_IsRunning(input->pObjectHandle) )
        {
            NEXUS_AudioDecoder_Stop(input->pObjectHandle);
        }
        break;
#endif
#if NEXUS_NUM_AUDIO_PLAYBACKS
    case NEXUS_AudioInputType_ePlayback:
        if ( NEXUS_AudioPlayback_P_IsRunning(input->pObjectHandle) )
        {
            NEXUS_AudioPlayback_Stop(input->pObjectHandle);
        }
        break;
#endif
#if NEXUS_NUM_I2S_INPUTS
    case NEXUS_AudioInputType_eI2s:
        if ( NEXUS_I2sInput_P_IsRunning(input->pObjectHandle) )
        {
            NEXUS_I2sInput_Stop(input->pObjectHandle);
        }
        break;
#endif
#if NEXUS_NUM_RF_AUDIO_DECODERS
    case NEXUS_AudioInputType_eRfDecoder:
        if ( NEXUS_RfAudioDecoder_P_IsRunning(input->pObjectHandle) )
        {
            NEXUS_RfAudioDecoder_Stop(input->pObjectHandle);
        }
        break;
#endif
#if NEXUS_NUM_ANALOG_AUDIO_DECODERS
    case NEXUS_AudioInputType_eAnalogDecoder:
        if ( NEXUS_AnalogAudioDecoder_P_IsRunning(input->pObjectHandle) )
        {
            NEXUS_AnalogAudioDecoder_Stop(input->pObjectHandle);
        }
        break;
#endif
#if NEXUS_NUM_AUDIO_INPUT_CAPTURES > 0
    case NEXUS_AudioInputType_eInputCapture:
        if ( NEXUS_AudioInputCapture_P_IsRunning(input->pObjectHandle) )
        {
            NEXUS_AudioInputCapture_Stop(input->pObjectHandle);
        }
        break;
#endif
    default:
        break;
    }
    /* Recurse Upstream */
    for ( pUpNode = BLST_Q_FIRST(&pData->upstreamList);
          NULL != pUpNode;
          pUpNode = BLST_Q_NEXT(pUpNode, upstreamNode) )
    {
        NEXUS_AudioInput_P_ForceStopUpstream(pUpNode->pUpstreamObject);
    }
}

static void NEXUS_AudioInput_P_ForceStopDownstream(NEXUS_AudioInput input)
{
    /* Stop any upstream components first */
    NEXUS_AudioUpstreamNode *pUpNode;
    NEXUS_AudioDownstreamNode *pDownNode;
    NEXUS_AudioInputData *pData;

    BDBG_OBJECT_ASSERT(input, NEXUS_AudioInput);

    BDBG_MSG(("NEXUS_AudioInput_P_ForceStopUpstream(%p)", (void *)input));

    pData = input->pMixerData;

    /* Can't be running if not connected */
    if ( NULL == pData )
    {
        return;
    }
    BDBG_OBJECT_ASSERT(pData, NEXUS_AudioInputData);

    switch ( input->objectType )
    {
#if NEXUS_NUM_AUDIO_DECODERS
    case NEXUS_AudioInputType_eDecoder:
        if ( NEXUS_AudioDecoder_P_IsRunning(input->pObjectHandle) )
        {
            NEXUS_AudioDecoder_Stop(input->pObjectHandle);
        }
        break;
#endif
#if NEXUS_NUM_AUDIO_PLAYBACKS
    case NEXUS_AudioInputType_ePlayback:
        if ( NEXUS_AudioPlayback_P_IsRunning(input->pObjectHandle) )
        {
            NEXUS_AudioPlayback_Stop(input->pObjectHandle);
        }
        break;
#endif
#if NEXUS_NUM_I2S_INPUTS
    case NEXUS_AudioInputType_eI2s:
        if ( NEXUS_I2sInput_P_IsRunning(input->pObjectHandle) )
        {
            NEXUS_I2sInput_Stop(input->pObjectHandle);
        }
        break;
#endif
#if NEXUS_NUM_RF_AUDIO_DECODERS
    case NEXUS_AudioInputType_eRfDecoder:
        if ( NEXUS_RfAudioDecoder_P_IsRunning(input->pObjectHandle) )
        {
            NEXUS_RfAudioDecoder_Stop(input->pObjectHandle);
        }
        break;
#endif
#if NEXUS_NUM_ANALOG_AUDIO_DECODERS
    case NEXUS_AudioInputType_eAnalogDecoder:
        if ( NEXUS_AnalogAudioDecoder_P_IsRunning(input->pObjectHandle) )
        {
            NEXUS_AnalogAudioDecoder_Stop(input->pObjectHandle);
        }
        break;
#endif
#if NEXUS_NUM_AUDIO_INPUT_CAPTURES > 0
    case NEXUS_AudioInputType_eInputCapture:
        if ( NEXUS_AudioInputCapture_P_IsRunning(input->pObjectHandle) )
        {
            NEXUS_AudioInputCapture_Stop(input->pObjectHandle);
        }
        break;
#endif
    case NEXUS_AudioInputType_eMixer:
    case NEXUS_AudioInputType_eDspMixer:
        /* Mixers are a special case.  They can have multiple inputs, all of which must be stopped also. */
        /* Recurse Upstream */
        for ( pUpNode = BLST_Q_FIRST(&pData->upstreamList);
              NULL != pUpNode;
              pUpNode = BLST_Q_NEXT(pUpNode, upstreamNode) )
        {
            NEXUS_AudioInput_P_ForceStopUpstream(pUpNode->pUpstreamObject);
        }
        break;
        /* Fall Through */
    default:
        break;
    }
    /* Recurse Downstream */
    for ( pDownNode = BLST_Q_FIRST(&pData->downstreamList);
          NULL != pDownNode;
          pDownNode = BLST_Q_NEXT(pDownNode, downstreamNode) )
    {
        NEXUS_AudioInput_P_ForceStopDownstream(pDownNode->pDownstreamObject);
    }
}

static bool NEXUS_AudioInput_P_IsConnectedDownstream(NEXUS_AudioInput source, NEXUS_AudioInput sink)
{
    NEXUS_AudioDownstreamNode *pNode;
    NEXUS_AudioInputData *pData;
    bool connected = false;

    BDBG_OBJECT_ASSERT(source, NEXUS_AudioInput);
    BDBG_OBJECT_ASSERT(sink, NEXUS_AudioInput);

    BDBG_MSG(("NEXUS_AudioInput_P_IsConnectedDownstream(%p,%p)", (void *)source, (void *)sink));

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

    NEXUS_ASSERT_MODULE();

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
