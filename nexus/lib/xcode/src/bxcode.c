/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 **************************************************************************/
#include "bxcode.h"
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "bxcode_priv.h"
#include "nexus_audio.h"

BDBG_MODULE(bxcode);
BDBG_OBJECT_ID(bxcode);

/**
Summary:
**/
void BXCode_GetDefaultOpenSettings(
    BXCode_OpenSettings *pSettings   /* [out] */
)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->audioDspId     = NEXUS_ANY_ID;
    pSettings->videoDecoderId = NEXUS_ANY_ID;
    pSettings->vpipes         = 1;
    pSettings->deferOpen      = true;
    pSettings->timebase       = NEXUS_Timebase_e0;
    NEXUS_VideoEncoder_GetDefaultOpenSettings(&pSettings->videoEncoder);
}

/**
Summary:
By default, open single a/v decoder, encoder/ts mux, display/window, stc channels.
May optionally, defer handles open to start time.
**/
BXCode_Handle BXCode_Open(
    unsigned                   id,
    const BXCode_OpenSettings *pSettings /* Pass NULL for default settings */
)
{
    unsigned i;
    BXCode_P_Context *pContext;
    BXCode_OpenSettings settings;
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_AudioCapabilities audioCap;

    if(!pSettings) {
        BXCode_GetDefaultOpenSettings(&settings);
        pSettings = &settings;
    }
    /* validation */
    if(id > NEXUS_NUM_VIDEO_ENCODERS) {
        BDBG_ERR(("id = %u >= total %u video encoders", id, NEXUS_NUM_VIDEO_ENCODERS));
        goto err_settings;
    }
    NEXUS_GetAudioCapabilities(&audioCap);
    if(pSettings->audioDspId != NEXUS_ANY_ID && pSettings->audioDspId >= audioCap.numDsps) {
        BDBG_ERR(("audioDspId = %u >= total %u DSPs", pSettings->audioDspId, audioCap.numDsps));
        goto err_settings;
    }
    if(pSettings->videoDecoderId != NEXUS_ANY_ID && pSettings->videoDecoderId >= NEXUS_NUM_VIDEO_DECODERS) {
        BDBG_ERR(("videoDecoderId = %u >= total %u video decoders", pSettings->videoDecoderId, NEXUS_NUM_VIDEO_DECODERS));
        goto err_settings;
    }
    if(pSettings->vpipes > NEXUS_NUM_VIDEO_ENCODERS) {
        BDBG_ERR(("vpipes = %u > total %u video encoders", pSettings->vpipes, NEXUS_NUM_VIDEO_ENCODERS));
        goto err_settings;
    }

    pContext = BKNI_Malloc(sizeof(BXCode_P_Context));
    if (!pContext) {
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BKNI_Memset(pContext, 0, sizeof(*pContext));
    BDBG_OBJECT_SET(pContext, bxcode);

    pContext->id = id;
    BKNI_Memcpy(&pContext->openSettings, pSettings, sizeof(*pSettings));
    if(pSettings->vpipes == 0) pContext->openSettings.vpipes = 1;

    /* init the default settings */
    bxcode_init(pContext);

    /* simple load balance: two audio xcodes per dsp core */
    if(pSettings->audioDspId == NEXUS_ANY_ID) pContext->openSettings.audioDspId = id/2;

    /* 1) create sync channel (non-FNRT) */
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
    pContext->syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
    if(NULL==pContext->syncChannel) {BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_sync;}

    /* 2) for file/stream input non-FNRT, open playpump */
    if(!pSettings->deferOpen) {
        if(pSettings->vpipes <= 1) {
            NEXUS_PlaypumpOpenSettings openSettings;
            NEXUS_Playpump_GetDefaultOpenSettings(&openSettings);
            pContext->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &openSettings);
            if(NULL==pContext->playpump) {BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_playpump;}
        }

        /* open video pipes */
        for(i=0; i < pSettings->vpipes; i++)
        {
            /* open the transcode context */
            if(NEXUS_SUCCESS!=bxcode_open_video_transcode(pContext, i)) {
                BDBG_ERR(("Transcoder%u failed to open video pipe%u!", pContext->id, i)); goto err_vpipe;
            }
        }

        /* open audio pipe 0 */
        if(NEXUS_SUCCESS!=bxcode_open_audio_transcode(pContext, 0)) {
            BDBG_ERR(("Transcoder%u failed to open audio pipe 0!", pContext->id)); goto err_apipe;
        }
    }

    g_BXCode_P_State.handles[id] = pContext;
    B_Os_Init();
    if(pSettings->vpipes > 1) {
        pContext->mutexChunk = B_Mutex_Create(NULL);
    }
    pContext->mutexActiveXcoders = B_Mutex_Create(NULL);
    return pContext;
err_apipe:
err_vpipe:
err_playpump:
    NEXUS_SyncChannel_Destroy(pContext->syncChannel);
err_sync:
    BDBG_OBJECT_DESTROY(pContext, bxcode);
err_alloc:
    BKNI_Free(pContext);
err_settings:
    return NULL;
}

/**
Summary:
**/
void BXCode_Close(BXCode_Handle hBxcode)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(hBxcode, bxcode);
    if(!hBxcode->openSettings.deferOpen) {
        bxcode_close_audio_transcode(hBxcode, 0);
        for(i=0; i < hBxcode->openSettings.vpipes; i++) {
            /* close the transcode context */
            bxcode_close_video_transcode(hBxcode, i);
        }
        if(hBxcode->playpump) NEXUS_Playpump_Close(hBxcode->playpump);
    }
    NEXUS_SyncChannel_Destroy(hBxcode->syncChannel);
    if(hBxcode->openSettings.vpipes > 1) {
        B_Mutex_Destroy(hBxcode->mutexChunk);
    }
    B_Mutex_Destroy(hBxcode->mutexActiveXcoders);
    g_BXCode_P_State.handles[hBxcode->id] = NULL;
    BDBG_OBJECT_DESTROY(hBxcode, bxcode);
    BKNI_Free(hBxcode);
    B_Os_Uninit();
}

/**
Summary:
**/
void BXCode_GetDefaultStartSettings(BXCode_StartSettings *pSettings   /* [out] */)
{
    if(!pSettings) {
        BDBG_ERR(("NULL pSettings!"));
        return;
    }
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_VideoEncoder_GetDefaultStartSettings(&pSettings->output.video.encoder);
    pSettings->output.video.encoder.interlaced = false;
    pSettings->output.video.encoder.profile = NEXUS_VideoCodecProfile_eHigh;
    pSettings->output.video.encoder.level = NEXUS_VideoCodecLevel_e31;
    pSettings->output.video.pid = 0x12;
    pSettings->output.audio[0].codec = NEXUS_AudioCodec_eAac;
    pSettings->output.transport.type = BXCode_OutputType_eTs;
    pSettings->output.transport.pcrPid = 0x11;
    pSettings->output.transport.pmtPid = 0x10;
    pSettings->output.transport.intervalPsi = 1000; /* 1 sec */
}

/**
Summary:
Start xcoder
**/
NEXUS_Error BXCode_Start(
    BXCode_Handle               hBxcode,
    const BXCode_StartSettings *pSettings
)
{
    unsigned i;
    NEXUS_StcChannelSettings stcSettings;
    BDBG_OBJECT_ASSERT(hBxcode, bxcode);

    if(!pSettings) {
        BDBG_ERR(("NULL startSettings!"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if((pSettings->input.type != BXCode_InputType_eFile || !pSettings->nonRealTime || pSettings->output.transport.type == BXCode_OutputType_eEs) &&
        hBxcode->openSettings.vpipes > 1) {
        BDBG_ERR(("FNRT mode = %d, inputType = %u, vpipes = %u!",
            pSettings->nonRealTime, pSettings->input.type, hBxcode->openSettings.vpipes));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if(hBxcode->started) {
        BDBG_WRN(("BXCode%u already started!", hBxcode->id));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BKNI_Memcpy(&hBxcode->startSettings, pSettings, sizeof(*pSettings));

    /* each transcoder potentially should only use 2 STCs on the same timebase:
           1) NRT video decode STC; (RT/NRT)
           2) NRT audio decode STC; (NRT)
           3) encode/mux STC;  (RT)
       NOTE: to avoid the 3rd one since NRT mode doesn't really need it,
             encoder/mux STC is only required in RT mode; and in RT mode, a/v decoders share the same STC.
     */
    if(pSettings->nonRealTime) {/* NRT mode */
        NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
        stcSettings.timebase = hBxcode->openSettings.timebase;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        /* allow input underflow to pause NRT pipe by app stream input feed */
        if(pSettings->input.type == BXCode_InputType_eStream) {
            stcSettings.underflowHandling = NEXUS_StcChannelUnderflowHandling_eAllowProducerPause;
        }
        for(i=0; i < hBxcode->openSettings.vpipes && pSettings->output.video.pid; i++)
        {
            hBxcode->video[i].stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
            BDBG_MSG(("BXCode[%u] opened vSTC [%p].", hBxcode->id, (void*)hBxcode->video[i].stcChannel));
            /* assign to encoder side as dummy handle for encoder/mux */
            hBxcode->stcChannelEncoder = hBxcode->video[i].stcChannel;
        }
        for(i=0; i < BXCODE_MAX_AUDIO_PIDS && pSettings->output.audio[i].pid; i++) {
            hBxcode->audio[i].stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
            BDBG_MSG(("BXCode[%u] opened aSTC [%p].", hBxcode->id, (void*)hBxcode->audio[i].stcChannel));
            /* assign to encoder side as dummy handle for encoder/mux */
            hBxcode->stcChannelEncoder = hBxcode->audio[i].stcChannel;
            break; /* audio PIDs share the single aSTC */
        }
    } else {/* RT mode */
        NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
        stcSettings.timebase = hBxcode->openSettings.timebase;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        if(pSettings->input.type == BXCode_InputType_eHdmi) {
            stcSettings.autoConfigTimebase = false; /* hdmi input timebase will be configured by hdmi input module! */
        }
        hBxcode->stcChannelDecoder = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
        BDBG_MSG(("Transcoder%d opened decoder STC [%p].", hBxcode->id, (void*)hBxcode->stcChannelDecoder));

        NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
        stcSettings.timebase = hBxcode->openSettings.timebase;/* should be the same timebase for end-to-end locking */
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;/* for encoder&mux, only timebase matters */
        stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
        stcSettings.autoConfigTimebase = false;/* encoder STC does not auto config timebase */
        hBxcode->stcChannelEncoder = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
        BDBG_MSG(("Transcoder%d opened encoder STC [%p].", hBxcode->id, (void*)hBxcode->stcChannelEncoder));
    }

    if(hBxcode->openSettings.deferOpen) {/* defered open */
        if(hBxcode->openSettings.vpipes <= 1 && /* non-FNRT playpump input */
           (pSettings->input.type == BXCode_InputType_eFile || pSettings->input.type == BXCode_InputType_eStream)) {
            NEXUS_PlaypumpOpenSettings openSettings;
            NEXUS_Playpump_GetDefaultOpenSettings(&openSettings);
            if(pSettings->input.type == BXCode_InputType_eStream && pSettings->input.userBuffer) openSettings.fifoSize = 0; /* external allocation */
            hBxcode->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &openSettings);
            if(NULL==hBxcode->playpump) {BDBG_ERR(("Transcoder%u failed to open input playpump!", hBxcode->id)); return NEXUS_NOT_AVAILABLE;}
        }

        /* open video pipe */
        for(i=0; i < hBxcode->openSettings.vpipes && pSettings->output.video.pid; i++)
        {
            /* open the transcode context */
            if(NEXUS_SUCCESS != bxcode_open_video_transcode(hBxcode, i)) {
                BDBG_ERR(("Transcoder%u failed to open video pipe%u!", hBxcode->id, i)); return NEXUS_NOT_AVAILABLE;
            }
        }

        /* open audio pipe 0 */
        if(NEXUS_SUCCESS != bxcode_open_audio_transcode(hBxcode, 0)) {
            BDBG_ERR(("Transcoder%u failed to open audio pipe!", hBxcode->id)); return NEXUS_NOT_AVAILABLE;
        }
    }

    /* open multi-audio pipes */
    hBxcode->numAudios = 1;
    for(i=1; i < BXCODE_MAX_AUDIO_PIDS && pSettings->output.audio[i].pid; i++) {
        if(NEXUS_SUCCESS != bxcode_open_audio_transcode(hBxcode, i)) {
            BDBG_ERR(("Transcoder%u failed to open audio pipe %u!", hBxcode->id, i)); return NEXUS_NOT_AVAILABLE;
        }
        hBxcode->numAudios++;
    }

    /* for non-FNRT, set up source input */
    if(hBxcode->openSettings.vpipes <= 1) {/* non-FNRT */
        bxcode_p_start_input(hBxcode);
    } else if(pSettings->input.vPid) {
        BNAV_Player_Settings settings;
        BNAV_Player_GetDefaultSettings(&settings);
        settings.videoPid = pSettings->input.vPid;
        settings.readCb = (BP_READ_CB)fread;
        settings.tellCb = (BP_TELL_CB)ftell;
        settings.seekCb = (BP_SEEK_CB)fseek;
        settings.filePointer = hBxcode->fpIndex = fopen(pSettings->input.index, "r");
        if(NEXUS_SUCCESS != BNAV_Player_Open(&hBxcode->bcmplayer, &settings)) {
            BDBG_ERR(("BXCode[%u] failed to open nav player", hBxcode->id));
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    } else {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);;
    }

    /* open mux (TODO: shall we open early as new option?) */
    if(pSettings->output.transport.type == BXCode_OutputType_eTs) {
        if(NEXUS_SUCCESS != bxcode_open_tsmux(hBxcode)) {
            BDBG_ERR(("Transcoder%u failed to open TS mux!", hBxcode->id)); return NEXUS_NOT_AVAILABLE;
        }
    } else if(pSettings->output.transport.type == BXCode_OutputType_eMp4File) {
        if(NEXUS_SUCCESS != bxcode_open_mp4mux(hBxcode)) {
            BDBG_ERR(("Transcoder%u failed to open MP4 file mux!", hBxcode->id)); return NEXUS_NOT_AVAILABLE;
        }
    }

        /************************************************
         * Set up FNRT video xcoder mux and record
         */
    if(hBxcode->openSettings.vpipes > 1 && (pSettings->output.transport.type != BXCode_OutputType_eEs)) {
        if(pSettings->output.transport.type == BXCode_OutputType_eTs) {/* TS mux */
            for(i=1; i < hBxcode->openSettings.vpipes && pSettings->output.video.pid; i++) {/* assign the same PID etc to the rest FNRT video channels, except the encoder is different */
                hBxcode->tsMuxConfig.video[i].pid   = hBxcode->tsMuxConfig.video[0].pid;
                hBxcode->tsMuxConfig.video[i].pesId = hBxcode->tsMuxConfig.video[0].pesId;
                hBxcode->tsMuxConfig.video[i].playpump = hBxcode->tsMuxConfig.video[0].playpump;
                hBxcode->tsMuxConfig.video[i].encoder = hBxcode->video[i].encoder;
            }
        } else if(pSettings->output.transport.type == BXCode_OutputType_eMp4File) {/* MP4 mux */
            for(i=1; i < hBxcode->openSettings.vpipes && pSettings->output.video.pid; i++) {
                hBxcode->mp4MuxConfig.video[i].codec = pSettings->output.video.encoder.codec;
                hBxcode->mp4MuxConfig.video[i].track = hBxcode->mp4MuxConfig.video[0].track;
                hBxcode->mp4MuxConfig.video[i].encoder = hBxcode->video[i].encoder;
            }
        }
    }

    /* start mux */
    if(pSettings->output.transport.type != BXCode_OutputType_eEs) {/* TS/MP4 mux */
        bxcode_start_mux(hBxcode);
    }

    /* set up decoder av sync for non-FNRT */
    if(hBxcode->openSettings.vpipes <= 1) {
        bxcode_p_pre_start(hBxcode);
    }

    /* start video pipe */
    for(i=0; i < hBxcode->openSettings.vpipes; i++)
    {
        if(pSettings->output.video.pid) {/* start video xcoders */
            bxcode_start_video_transcode(hBxcode, i);
        }
    }

    /* start audio pipe */
    for(i=0; i < BXCODE_MAX_AUDIO_PIDS; i++)
    {
        if(pSettings->output.audio[i].pid) {/* start audio xcoders */
            bxcode_start_audio_transcode(hBxcode, i);
        }
    }

    /* set up av sync, encoder start and mux for non-FNRT */
    if(hBxcode->openSettings.vpipes <= 1) {
        bxcode_p_post_start(hBxcode);
    }

    hBxcode->started = true;
    return BERR_SUCCESS;
}

/**
Summary:
**/
void BXCode_Stop(BXCode_Handle hBxcode)
{
    unsigned i;
    BXCode_StartSettings *pSettings = &hBxcode->startSettings;

    BDBG_OBJECT_ASSERT(hBxcode, bxcode);

    if(!hBxcode->started) {
        BDBG_WRN(("BXCode%u was not started!", hBxcode->id));
        return;
    }

    hBxcode->toStop = true;
    /* input stop for non-FNRT */
    if(hBxcode->openSettings.vpipes <= 1) {
        bxcode_p_pre_stop(hBxcode);
    }

    /* stop audio pipes */
    for(i=0; i < BXCODE_MAX_AUDIO_PIDS; i++)
    {
        if(pSettings->output.audio[i].pid) {/* stop audio xcoders */
            bxcode_stop_audio_transcode(hBxcode, i);
        }
    }

    if(hBxcode->openSettings.vpipes > 1) {/* FNRT wait for auto stop EOS */
        /* stop mux */
        BDBG_MSG(("stopping fnrt mux..."));
        bxcode_stop_mux(hBxcode);
    }

    /* stop video pipes */
    for(i=0; i < hBxcode->openSettings.vpipes; i++)
    {
        if(pSettings->output.video.pid) {/* stop video xcoders */
            bxcode_stop_video_transcode(hBxcode, i);
        }
    }

    if(hBxcode->openSettings.vpipes <= 1) {/* FNRT wait for auto stop EOS */
        /* stop mux */
        BDBG_MSG(("stopping mux..."));
        if(pSettings->output.transport.type != BXCode_OutputType_eEs) {/* TS mux */
            bxcode_stop_mux(hBxcode);
        }
    }

    /* for non-FNRT, close source playback */
    if(hBxcode->openSettings.vpipes <= 1) {/* non-FNRT */
        bxcode_p_stop_input(hBxcode);
    } else if(pSettings->input.vPid) {
        BNAV_Player_Close(hBxcode->bcmplayer);
        fclose(hBxcode->fpIndex);
    }

    /* close multi-audio pipes */
    for(i=1; i < BXCODE_MAX_AUDIO_PIDS && pSettings->output.audio[i].pid; i++) {
        bxcode_close_audio_transcode(hBxcode, i);
    }

    /* defered open */
    if(hBxcode->openSettings.deferOpen) {
        if(hBxcode->openSettings.vpipes <= 1 && /* non-FNRT playpump input */
           (pSettings->input.type == BXCode_InputType_eFile || pSettings->input.type == BXCode_InputType_eStream)) {
            NEXUS_Playpump_Close(hBxcode->playpump);
        }

        /* close video pipe */
        for(i=0; i < hBxcode->openSettings.vpipes && pSettings->output.video.pid; i++)
        {
            bxcode_close_video_transcode(hBxcode, i);
        }

        /* open audio pipe 0 */
        bxcode_close_audio_transcode(hBxcode, 0);
    }

    if(pSettings->nonRealTime) {/* NRT mode */
        for(i=0; i < hBxcode->openSettings.vpipes && pSettings->output.video.pid; i++)
        {
            NEXUS_StcChannel_Close(hBxcode->video[i].stcChannel);
            hBxcode->video[i].stcChannel = NULL;
        }
        for(i=0; i < BXCODE_MAX_AUDIO_PIDS && pSettings->output.audio[i].pid; i++) {
            NEXUS_StcChannel_Close(hBxcode->audio[i].stcChannel);
            hBxcode->audio[i].stcChannel = NULL;
            break; /* audio PIDs share the single aSTC */
        }
    } else {/* RT mode */
        NEXUS_StcChannel_Close(hBxcode->stcChannelDecoder);
        NEXUS_StcChannel_Close(hBxcode->stcChannelEncoder);
        hBxcode->stcChannelDecoder = NULL;
        hBxcode->stcChannelEncoder = NULL;
    }

    hBxcode->started = false;
    hBxcode->toStop = false;
}

void BXCode_GetSettings(
    BXCode_Handle          hBxcode,
    BXCode_Settings *pSettings /* [out] */
)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(hBxcode, bxcode);
    BDBG_ASSERT(pSettings);
    if(hBxcode->started) {/* dynamic video settings */
        for(i=0; i < hBxcode->openSettings.vpipes && hBxcode->video[i].display; i++) {
            NEXUS_VideoEncoder_GetSettings(hBxcode->video[i].encoder, &hBxcode->settings.video.encoder);
        }
        NEXUS_Display_GetGraphicsSettings(hBxcode->video[0].display, &pSettings->video.gfxSettings);
    }
    BKNI_Memcpy(pSettings, &hBxcode->settings, sizeof(*pSettings));
}

NEXUS_Error BXCode_SetSettings(
    BXCode_Handle          hBxcode,
    const BXCode_Settings *pSettings
)
{
    NEXUS_Error rc;
    NEXUS_StreamMuxSettings muxSettings;
    unsigned i;
    BDBG_OBJECT_ASSERT(hBxcode, bxcode);
    BDBG_ASSERT(pSettings);

    if(hBxcode->started) {/* dynamic video settings */
        for(i=0; i < hBxcode->openSettings.vpipes && hBxcode->video[i].display; i++) {
            rc = BXCode_P_SetVideoSettings(hBxcode, i, pSettings);
            if(NEXUS_SUCCESS != rc) { BDBG_ERR(("BXCode[%u] failed to set video settings", hBxcode->id)); return BERR_TRACE(rc); }

            rc = NEXUS_VideoEncoder_SetSettings(hBxcode->video[i].encoder, &pSettings->video.encoder);
            if (NEXUS_SUCCESS != rc) return BERR_TRACE(rc);

            /* dynamic toggle video PID output */
            if(hBxcode->startSettings.output.transport.type == BXCode_OutputType_eTs) {
                NEXUS_StreamMux_GetSettings(hBxcode->streamMux, &muxSettings);
                muxSettings.enable.video[0] = pSettings->video.enabled;
                NEXUS_StreamMux_SetSettings(hBxcode->streamMux, &muxSettings);
            }
        }

        /* handle audio codec bitrate and sample rate */
        for(i=0; i < hBxcode->startSettings.output.audio[i].pid && !hBxcode->startSettings.output.audio[i].passthrough; i++) {
            rc = BXCode_P_SetAudioSettings(hBxcode, i, pSettings);
            if(NEXUS_SUCCESS != rc) { BDBG_ERR(("BXCode[%u] failed to set audio codec settings", hBxcode->id)); return BERR_TRACE(rc); }

            /* dynamic toggle audio PID output */
            if(hBxcode->startSettings.output.transport.type == BXCode_OutputType_eTs) {
                NEXUS_StreamMux_GetSettings(hBxcode->streamMux, &muxSettings);
                muxSettings.enable.audio[i] = pSettings->audio[i].enabled;
                rc = NEXUS_StreamMux_SetSettings(hBxcode->streamMux, &muxSettings);
                if(NEXUS_SUCCESS != rc) {
                    BDBG_ERR(("BXCode[%u] failed to %s audio[%u] in stream mux", hBxcode->id, pSettings->audio[i].enabled?"enable":"disable", i));
                    return BERR_TRACE(rc);
                }
            }
        }
    }
    BKNI_Memcpy(&hBxcode->settings, pSettings, sizeof(*pSettings));
    return NEXUS_SUCCESS;
}
#endif /* NEXUS_HAS_VIDEO_ENCODER */
