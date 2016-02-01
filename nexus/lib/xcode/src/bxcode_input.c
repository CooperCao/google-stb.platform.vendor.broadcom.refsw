/***************************************************************************
 *     (c)2010-2014 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#include "bxcode.h"
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "bxcode_priv.h"

BDBG_MODULE(bxcode_input);

static void play_endOfStreamCallback(void *context, int param)
{
    BXCode_P_Context  *pContext = (BXCode_P_Context  *)context;
    BSTD_UNUSED(param);

    BDBG_WRN(("Transcoder%u input file endOfStream\n", pContext->id));

    if(!pContext->startSettings.input.loop && pContext->startSettings.input.eofDone.callback) {
        /* terminate the file input if no loop */
        pContext->startSettings.input.eofDone.callback(pContext->startSettings.input.eofDone.context,
            pContext->startSettings.input.eofDone.param);
    }
    return;
}

static void BXCode_P_InputPlaypumpCallback(void *context, int param)
{
    BXCode_P_Context  *pContext = (BXCode_P_Context  *)context;
    BSTD_UNUSED(param);
    if(pContext->startSettings.input.type == BXCode_InputType_eStream && pContext->startSettings.input.dataCallback.callback) {
        pContext->startSettings.input.dataCallback.callback(pContext->startSettings.input.dataCallback.context,
            pContext->startSettings.input.dataCallback.param);
    }
}

/* private function for non-FNRT decoder input */
void bxcode_p_start_input(
    BXCode_P_Context  *bxcode)
{
    NEXUS_PlaypumpSettings playpumpSettings;
    BXCode_StartSettings *pSettings = &bxcode->startSettings;

    if(pSettings->input.type == BXCode_InputType_eFile) {/* non-FNRT */
        NEXUS_PlaybackSettings playbackSettings;

        /* file playback settings */
        bxcode->playback = NEXUS_Playback_Create();
        BDBG_ASSERT(bxcode->playback);

        bxcode->file = NEXUS_FilePlay_OpenPosix(pSettings->input.data, pSettings->input.index? pSettings->input.index :
            ((pSettings->input.transportType != NEXUS_TransportType_eTs)? pSettings->input.data : NULL));
        if (!bxcode->file) {
            BDBG_ERR(("can't open file:%s\n", pSettings->input.data));
            BDBG_ASSERT(0);
        }
        NEXUS_Playback_GetSettings(bxcode->playback, &playbackSettings);
        playbackSettings.playpump = bxcode->playpump;
        /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
        playbackSettings.playpumpSettings.transportType = pSettings->input.transportType;
        playbackSettings.stcChannel = pSettings->nonRealTime?
            bxcode->video[0].stcChannel : /* NRT mode video stc channel; */
            bxcode->stcChannelDecoder; /* RT mode playback shares the sasme STC channel as a/v decoders */

        /* NRT mode file transcode doesn not need loop */
        playbackSettings.endOfStreamAction = (pSettings->input.loop)? NEXUS_PlaybackLoopMode_eLoop : NEXUS_PlaybackLoopMode_ePause;
        playbackSettings.endOfStreamCallback.callback = play_endOfStreamCallback;
        playbackSettings.endOfStreamCallback.context  = bxcode;
        playbackSettings.playpumpSettings.timestamp.type = pSettings->input.timestampType;
        NEXUS_Playback_SetSettings(bxcode->playback, &playbackSettings);
    }else if(pSettings->input.type == BXCode_InputType_eStream) {/* stream input */
        /* stream playpump settings */
        BKNI_CreateEvent(&bxcode->dataReadyEvent);
        NEXUS_Playpump_GetSettings(bxcode->playpump, &playpumpSettings);
        playpumpSettings.dataCallback.callback = BXCode_P_InputPlaypumpCallback;
        playpumpSettings.dataCallback.context  = bxcode;
        playpumpSettings.dataCallback.param    = bxcode->id;
        NEXUS_Playpump_SetSettings(bxcode->playpump, &playpumpSettings);
    }
}

/* private function for non-FNRT decoder input */
void bxcode_p_stop_input(
    BXCode_P_Context  *bxcode)
{
    BXCode_StartSettings *pSettings = &bxcode->startSettings;

    if(pSettings->input.type == BXCode_InputType_eFile) {/* non-FNRT */
        NEXUS_FilePlay_Close(bxcode->file);
        bxcode->file = NULL;
        NEXUS_Playback_CloseAllPidChannels(bxcode->playback);
        NEXUS_Playback_Destroy(bxcode->playback);
        bxcode->playback = NULL;
    }else if(pSettings->input.type == BXCode_InputType_eStream) {/* stream input */
        NEXUS_Playpump_CloseAllPidChannels(bxcode->playpump);
        /* stream playpump */
        BKNI_DestroyEvent(bxcode->dataReadyEvent);
    }
}

/**
Summary:
**/
NEXUS_Error BXCode_GetInputStatus(
    BXCode_Handle       handle,
    BXCode_InputStatus *pStatus /* [out] */
    )
{
    unsigned i;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, bxcode);
    BDBG_ASSERT(pStatus);
    if(!handle->started) {
        BDBG_WRN(("BXCode%u is not started yet!", handle->id));
        return NEXUS_NOT_AVAILABLE;
    }
    switch(handle->startSettings.input.type) {
    case BXCode_InputType_eFile:
        if(handle->playback) {
            rc = NEXUS_Playback_GetStatus(handle->playback, &pStatus->playback);
            if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_Playback_GetStatus error!")); return BERR_TRACE(rc);}
        }
        rc = NEXUS_VideoDecoder_GetStatus(handle->video[0].decoder, &pStatus->videoDecoder);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_VideoDecoder_GetStatus error!")); return BERR_TRACE(rc);}
        for(i=0; i<BXCODE_MAX_AUDIO_PIDS && handle->startSettings.input.aPid[i]; i++) {
            rc = NEXUS_AudioDecoder_GetStatus(handle->audio[i].decoder, &pStatus->audioDecoder[i]);
            if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_AudioDecoder_GetStatus[%u] error!", i)); return BERR_TRACE(rc);}
        }
        pStatus->numAudios = i;
        break;
    case BXCode_InputType_eStream:
        rc = NEXUS_Playpump_GetStatus(handle->playpump, &pStatus->playpump);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_Playpump_GetStatus error!")); return BERR_TRACE(rc);}
        rc = NEXUS_VideoDecoder_GetStatus(handle->video[0].decoder, &pStatus->videoDecoder);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_VideoDecoder_GetStatus error!")); return BERR_TRACE(rc);}
        for(i=0; i<BXCODE_MAX_AUDIO_PIDS && handle->startSettings.input.aPid[i]; i++) {
            rc = NEXUS_AudioDecoder_GetStatus(handle->audio[i].decoder, &pStatus->audioDecoder[i]);
            if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_AudioDecoder_GetStatus[%u] error!", i)); return BERR_TRACE(rc);}
        }
        pStatus->numAudios = i;
        break;
    case BXCode_InputType_eLive:
        rc = NEXUS_ParserBand_GetStatus(handle->startSettings.input.parserBand, &pStatus->parserBand);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_ParserBand_GetStatus error!")); return BERR_TRACE(rc);}
        rc = NEXUS_VideoDecoder_GetStatus(handle->video[0].decoder, &pStatus->videoDecoder);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_VideoDecoder_GetStatus error!")); return BERR_TRACE(rc);}
        for(i=0; i<BXCODE_MAX_AUDIO_PIDS && handle->startSettings.input.aPid[i]; i++) {
            rc = NEXUS_AudioDecoder_GetStatus(handle->audio[i].decoder, &pStatus->audioDecoder[i]);
            if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_AudioDecoder_GetStatus[%u] error!", i)); return BERR_TRACE(rc);}
        }
        pStatus->numAudios = i;
        break;
#if NEXUS_HAS_HDMI_INPUT
    case BXCode_InputType_eHdmi:
        rc = NEXUS_HdmiInput_GetStatus(handle->startSettings.input.hdmiInput, &pStatus->hdmiInput);
        if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_HdmiInput_GetStatus error!")); return BERR_TRACE(rc);}
        break;
#endif
    case BXCode_InputType_eImage:
        if(handle->video[0].imageInput) {
            pStatus->imageInput = handle->video[0].imageInput;
            rc = NEXUS_VideoImageInput_GetStatus(handle->video[0].imageInput, &pStatus->imageInputStatus);
            if(rc != NEXUS_SUCCESS) {BDBG_ERR(("NEXUS_VideoImageInput_GetStatus error!")); return BERR_TRACE(rc);}
        }
    default:
        break;
    }
    return NEXUS_SUCCESS;
}

/**
Summary:
Submit scatter gather descriptor to BXCode_Input stream interface with app allocated user buffers
App calls NEXUS_Memory_Allocate to allocate N data buffers, fill it then submit via this API to the stream input.
Note, app needs to check input status.stream.playpump.descFifoDepth < N to feed input; else (=N), wait for dataready event from input stream dataCallback.
**/
NEXUS_Error BXCode_Input_SubmitScatterGatherDescriptor(
    BXCode_Handle handle,
    const BXCode_InputDescriptor *pDesc,
    size_t        numDescriptors,
    size_t       *pNumConsumed /* [out] */
    )
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, bxcode);
    BDBG_ASSERT(pDesc);
    if(!handle->started) {
        BDBG_WRN(("BXCode%u is not started yet!", handle->id));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if(handle->startSettings.input.type != BXCode_InputType_eStream) {
        BDBG_WRN(("BXCode%u is not started with stream input!", handle->id));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    /* punch through */
    rc = NEXUS_Playpump_SubmitScatterGatherDescriptor(handle->playpump, pDesc, numDescriptors, pNumConsumed);
    return BERR_TRACE(rc);
}

/*
Summary:
For internally allocated stream feed buffer, app uses GetBuffer to get the pointer for memcpy.
*/
NEXUS_Error BXCode_Input_GetBuffer(
    BXCode_Handle handle,
    void        **pBuffer, /* [out] pointer to memory mapped region that is ready for playback data */
    size_t       *pSize    /* [out] total number of writeable, contiguous bytes which buffer is pointing to */
    )
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, bxcode);
    BDBG_ASSERT(pBuffer);
    if(!handle->started) {
        BDBG_WRN(("BXCode%u is not started yet!", handle->id));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if(handle->startSettings.input.type != BXCode_InputType_eStream) {
        BDBG_WRN(("BXCode%u is not started with stream input!", handle->id));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if(handle->startSettings.input.userBuffer) {
        BDBG_WRN(("Stream input doesn't allocate internal buffer!", handle->id));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    rc = NEXUS_Playpump_GetBuffer(handle->playpump, pBuffer, pSize);
    return BERR_TRACE(rc);
}

/**
Summary:
For internally allocated stream feed buffer, app uses WrieComplete to notify BXCode_Input to consume the data.
**/
NEXUS_Error BXCode_Input_WriteComplete(
    BXCode_Handle handle,
    size_t        skip,      /* skip is the number of bytes at the beginning of the current buffer pointer
                                          which BXCode_Input should skip over. */
    size_t        amountUsed /* amountUsed is the number of bytes, following any skip bytes,
                                          which BXCode_Input should feed into transport. */
    )
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, bxcode);
    if(!handle->started) {
        BDBG_WRN(("BXCode%u is not started yet!", handle->id));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if(handle->startSettings.input.type != BXCode_InputType_eStream) {
        BDBG_WRN(("BXCode%u is not started with stream input!", handle->id));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if(handle->startSettings.input.userBuffer) {
        BDBG_WRN(("Stream input doesn't allocate internal buffer!", handle->id));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    rc = NEXUS_Playpump_ReadComplete(handle->playpump, skip, amountUsed);
    return BERR_TRACE(rc);
}
#endif /* NEXUS_HAS_VIDEO_ENCODER */
