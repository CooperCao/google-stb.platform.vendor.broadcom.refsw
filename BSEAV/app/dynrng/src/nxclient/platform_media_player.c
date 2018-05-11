/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include "nexus_platform_client.h"
#include "nxclient.h"
#include "media_player.h"
#include "platform.h"
#include "platform_priv.h"
#include "platform_media_player_priv.h"
#include "platform_graphics_priv.h"
#include "nexus_simple_video_decoder.h"
#include "blst_queue.h"
#include "bdbg.h"
#include "bkni.h"
#include <string.h>

BDBG_MODULE(platform_media_player);

static bool gPlayerContextInitialized = false;
static PlatformMediaPlayerContext gPlayerContext;
static void platform_media_player_p_init_context(void)
{
    if (!gPlayerContextInitialized)
    {
        BKNI_Memset(&gPlayerContext, 0, sizeof(gPlayerContext));
        media_player_get_default_create_settings(&gPlayerContext.nxCreateSettings);
        gPlayerContextInitialized = true;
    }
}

const int PLATFORM_TRICK_RATE_1X = NEXUS_NORMAL_DECODE_RATE;

static void platform_media_player_p_create_video_surfaces(PlatformHandle platform)
{
    NEXUS_SurfaceCreateSettings createSettings;
    unsigned i;

    /* we only have enough mem and cpu to do video-as-gfx for one stream */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    createSettings.width = 1920;
    createSettings.height = 1080;
    for (i = 0; i < NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++)
    {
        platform->media.streams[platform_get_pip_stream_id(platform)].surfaces[i] = NEXUS_Surface_Create(&createSettings);
    }
}

PlatformMediaPlayerHandle platform_media_player_create(PlatformHandle platform, PlatformCallback streamInfoCallback, void * streamInfoContext)
{
    PlatformMediaPlayerHandle player;
    unsigned maxStreamCount;

    BDBG_ASSERT(platform);
    BDBG_ASSERT(platform->gfx);

    platform_media_player_p_init_context();

    maxStreamCount = platform_get_max_stream_count(platform);

    if (gPlayerContext.count >= maxStreamCount) {
        BDBG_ERR(("Failed to create platform media player. Max number (%d) of players already created.", maxStreamCount));
        return NULL;
    }

    player = BKNI_Malloc(sizeof(*player));
    BDBG_ASSERT(player);
    BKNI_Memset(player, 0, sizeof(*player));

    BKNI_CreateMutex(&player->mutex);
    player->platform = platform;
    player->streamInfo.callback = streamInfoCallback;
    player->streamInfo.context = streamInfoContext;

    if(!gPlayerContext.count) {
        unsigned decoderMaxWindowCount;
        decoderMaxWindowCount = platform_get_decoder_max_stream_count(platform, 0);

        media_player_get_default_create_settings(&gPlayerContext.nxCreateSettings);
        gPlayerContext.nxCreateSettings.window.surfaceClientId = platform->gfx->surfaceClient.id;
        /* window.id not necessary, mosaics are assumed 0 to n - 1 */
        media_player_create_mosaics(gPlayerContext.nxPlayerMosaic, decoderMaxWindowCount, &gPlayerContext.nxCreateSettings);
        BDBG_ASSERT(gPlayerContext.nxPlayerMosaic[0]);
        gPlayerContext.nxCreateSettings.window.id = platform_get_pip_stream_id(platform);
        if (gPlayerContext.nxCreateSettings.window.id > 0)
        {
            gPlayerContext.nxPlayerPip = media_player_create(&gPlayerContext.nxCreateSettings);
            BDBG_ASSERT(gPlayerContext.nxPlayerPip);
        }
        gPlayerContext.nxCreateSettings.maxWidth = 3840;
        gPlayerContext.nxCreateSettings.maxHeight = 2160;
        gPlayerContext.nxCreateSettings.window.id = platform_get_main_stream_id(platform);
        gPlayerContext.nxPlayer = media_player_create(&gPlayerContext.nxCreateSettings);
        BDBG_ASSERT(gPlayerContext.nxPlayer);

        platform_media_player_p_create_video_surfaces(platform);
    }

    player->streamId = -1;
    player->index = gPlayerContext.count++;
    platform->media.players[player->index] = player;
    player->streamInfoGatherer = platform_scheduler_add_listener(platform_get_scheduler(platform, PLATFORM_SCHEDULER_MAIN), &platform_media_player_p_scheduler_callback, player);
    if (!player->streamInfoGatherer)
    {
        BDBG_WRN(("Unable to monitor stream info"));
    }

    return player;
}

void platform_media_player_destroy(PlatformMediaPlayerHandle player)
{
    if (!player) return;
    BKNI_AcquireMutex(player->mutex);
    if (player->streamInfoGatherer)
    {
        platform_scheduler_remove_listener(platform_get_scheduler(player->platform, PLATFORM_SCHEDULER_MAIN), player->streamInfoGatherer);
    }
    player->nxPlayer = NULL;
    player->platform->media.players[player->index] = NULL;
    gPlayerContext.count--;
    if (!gPlayerContext.count) {
        media_player_destroy_mosaics(gPlayerContext.nxPlayerMosaic, platform_get_decoder_max_stream_count(player->platform, 0));
        if (gPlayerContext.nxPlayerPip)
        {
            media_player_destroy(gPlayerContext.nxPlayerPip);
        }
        media_player_destroy(gPlayerContext.nxPlayer);
    }
    BKNI_ReleaseMutex(player->mutex);
    BKNI_DestroyMutex(player->mutex);
    BKNI_Free(player);
}

static void platform_media_player_p_update_player_impl(PlatformMediaPlayerHandle player, PlatformUsageMode usageMode, bool * pipRequired)
{
    switch (usageMode)
    {
        case PlatformUsageMode_eMosaic:
            if (player->index < platform_get_decoder_max_stream_count(player->platform, 0)) {
                BDBG_ERR(("player %d using mosaic", player->index));
                player->nxPlayer = gPlayerContext.nxPlayerMosaic[player->index];
            } else {
                BDBG_ERR(("player %d using pip", player->index));
                player->nxPlayer = gPlayerContext.nxPlayerPip;
                if (pipRequired) *pipRequired = true;
            }
            break;
        case PlatformUsageMode_eMainPip:
            if (player->index == 1) {
                BDBG_ERR(("player %d using pip", player->index));
                player->nxPlayer = gPlayerContext.nxPlayerPip;
                if (pipRequired) *pipRequired = true;
            } else {
                BDBG_ERR(("player %d using main", player->index));
                player->nxPlayer = gPlayerContext.nxPlayer;
            }
            break;
        default:
            BDBG_ERR(("player %d using main", player->index));
            player->nxPlayer = gPlayerContext.nxPlayer;
            break;
    }
}

void platform_media_player_get_default_start_settings(PlatformMediaPlayerStartSettings * pSettings)
{
    if (pSettings)
    {
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
        pSettings->stcTrick = true;
        pSettings->playMode = PlatformPlayMode_eLoop;
    }
}

int platform_media_player_start(PlatformMediaPlayerHandle player, const PlatformMediaPlayerStartSettings * pSettings)
{
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderClientSettings settings;
    NEXUS_SimpleVideoDecoderStartCaptureSettings captureSettings;
    NEXUS_VideoDecoderSettings decoderSettings;
    unsigned pip;
    int rc;

    BDBG_ASSERT(player);

    BKNI_AcquireMutex(player->mutex);

    pip = platform_get_pip_stream_id(player->platform);
    /* TEMP disable CC for mosaics */
    videoDecoder = media_player_get_video_decoder(player->nxPlayer);
    NEXUS_SimpleVideoDecoder_GetClientSettings(videoDecoder, &settings);
    settings.closedCaptionRouting = false;
    NEXUS_SimpleVideoDecoder_SetClientSettings(videoDecoder, &settings);

    NEXUS_SimpleVideoDecoder_GetSettings(videoDecoder, &decoderSettings);
    decoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_SimpleVideoDecoder_SetSettings(videoDecoder, &decoderSettings);

    media_player_get_default_start_settings(&player->nxStartSettings);
    player->nxStartSettings.stream_url = pSettings->url;
    player->nxStartSettings.video.scanMode = NEXUS_VideoDecoderScanMode_e1080p;
    player->nxStartSettings.video.progressiveOverrideMode = NEXUS_VideoDecoderProgressiveOverrideMode_eDisable;
    player->nxStartSettings.forceStopDisconnect = true;
    player->nxStartSettings.loopMode = pSettings->playMode == PlatformPlayMode_eOnce ? NEXUS_PlaybackLoopMode_ePause : NEXUS_PlaybackLoopMode_eLoop;
    player->nxStartSettings.startPaused = pSettings->startPaused;
    player->nxStartSettings.stcTrick = pSettings->stcTrick;
    if (player->pipRequired)
    {
        if (player->platform->display.maxWindows > 1)
        {
            player->nxStartSettings.videoWindowType = NxClient_VideoWindowType_ePip;
        }
        else
        {
            /* pip as graphics */
            player->nxStartSettings.videoWindowType = NxClient_VideoWindowType_eNone;
        }
        player->platform->media.streams[pip].player = player;
        player->streamId = pip;
    }
    else
    {
        player->platform->media.streams[player->index].player = player;
        player->streamId = player->index;
    }
    rc = media_player_start(player->nxPlayer, &player->nxStartSettings);
    if (!rc)
    {
        player->trickRate = PLATFORM_TRICK_RATE_1X;
        player->state = PlatformMediaPlayerState_ePlaying;
        platform_scheduler_wake(platform_get_scheduler(player->platform, PLATFORM_SCHEDULER_MAIN));
    }
    else { BDBG_ERR(("unable to start media player %d for '%s': %d", player->index, player->nxStartSettings.stream_url, rc)); }

    /* pip as gfx support follows */
    if (player->settings.usageMode == PlatformUsageMode_eMainPip && player->platform->display.maxWindows == 1 && player->streamId == (int)pip)
    {
        NEXUS_SimpleVideoDecoder_GetDefaultStartCaptureSettings(&captureSettings);
        BKNI_Memcpy(&captureSettings.surface, &player->platform->media.streams[pip].surfaces, sizeof(player->platform->media.streams[pip].surfaces));
        captureSettings.forceFrameDestripe = true;
        rc = NEXUS_SimpleVideoDecoder_StartCapture(videoDecoder, &captureSettings);
        if ( rc == NEXUS_NOT_SUPPORTED ) {
            BDBG_WRN(("Video as graphics not supported !" ));
        }
        player->platform->media.streams[pip].performance.startTime = platform_p_get_time();
    }

    BKNI_ReleaseMutex(player->mutex);
    return rc;
}

void platform_media_player_stop(PlatformMediaPlayerHandle player)
{
    BDBG_ASSERT(player);
    BKNI_AcquireMutex(player->mutex);
    player->state = PlatformMediaPlayerState_eInit;
    if (player->nxPlayer && player->streamId != -1)
    {
        /* pip as gfx support follows */
        if (player->settings.usageMode == PlatformUsageMode_eMainPip && player->platform->display.maxWindows == 1)
        {
            NEXUS_SimpleVideoDecoder_StopCapture(media_player_get_video_decoder(player->nxPlayer));
        }
        media_player_stop(player->nxPlayer);
        player->streamInfo.nx.valid = false;
        player->platform->media.streams[player->streamId].player = NULL;
        player->streamId = -1;
    }
    BKNI_ReleaseMutex(player->mutex);
}

void platform_media_player_trick(PlatformMediaPlayerHandle player, int rate)
{
    BDBG_ASSERT(player);
    BDBG_ASSERT(player->nxPlayer);
    BKNI_AcquireMutex(player->mutex);
    if ((rate == 0) && player->trickRate)
    {
        player->trickRate = rate;
        player->state = PlatformMediaPlayerState_ePaused;
    }
    else if (rate && (player->trickRate == 0))
    {
        player->trickRate = rate;
        player->state = PlatformMediaPlayerState_ePlaying;
        platform_scheduler_wake(platform_get_scheduler(player->platform, PLATFORM_SCHEDULER_MAIN));
    }
    media_player_trick(player->nxPlayer, rate);
    BKNI_ReleaseMutex(player->mutex);
}

void platform_media_player_frame_advance(PlatformMediaPlayerHandle player)
{
    BDBG_ASSERT(player);
    BKNI_AcquireMutex(player->mutex);
    media_player_frame_advance(player->nxPlayer, true);
    BKNI_ReleaseMutex(player->mutex);
}

bool platform_media_player_p_get_stream_info(PlatformMediaPlayerHandle player)
{
    bool changed = false;
    NEXUS_VideoDecoderStreamInformation streamInfo;
    NEXUS_SimpleVideoDecoderHandle video;

    BDBG_ASSERT(player);
    BKNI_AcquireMutex(player->mutex);
    if(!player->nxPlayer)
    {
        BKNI_ReleaseMutex(player->mutex);
        return false;
    }
    video = media_player_get_video_decoder(player->nxPlayer);
    NEXUS_SimpleVideoDecoder_GetStreamInformation(video, &streamInfo);
    changed = BKNI_Memcmp(&streamInfo, &player->streamInfo.nx, sizeof(streamInfo)) == 0 ? false : true;
    BKNI_Memcpy(&player->streamInfo.nx, &streamInfo, sizeof(streamInfo));
    BKNI_ReleaseMutex(player->mutex);
    return changed;
}

void platform_media_player_get_picture_info(PlatformMediaPlayerHandle player, PlatformPictureInfo * pInfo)
{
    BDBG_ASSERT(player);
    platform_media_player_p_get_stream_info(player);
    BKNI_Memset(pInfo, 0, sizeof(*pInfo));
    if (player->streamInfo.nx.valid)
    {
        pInfo->dynrng = platform_p_input_dynamic_range_from_nexus(player->streamInfo.nx.eotf, player->streamInfo.nx.dynamicMetadataType);
        pInfo->gamut = platform_p_colorimetry_from_nexus(player->streamInfo.nx.matrixCoefficients);
        pInfo->space = PlatformColorSpace_eYCbCr;
        pInfo->sampling = 420;
        pInfo->depth = player->streamInfo.nx.colorDepth;
        pInfo->format.width = player->streamInfo.nx.sourceHorizontalSize;
        pInfo->format.height = player->streamInfo.nx.sourceVerticalSize;
        pInfo->format.interlaced = !player->streamInfo.nx.streamProgressive;
        pInfo->format.rate = platform_p_frame_rate_from_nexus(player->streamInfo.nx.frameRate);
        pInfo->format.dropFrame = platform_p_drop_frame_from_nexus(player->streamInfo.nx.frameRate);
        platform_p_aspect_ratio_from_nexus(&pInfo->ar, player->streamInfo.nx.aspectRatio, player->streamInfo.nx.sampleAspectRatioX, player->streamInfo.nx.sampleAspectRatioY);
    }
    else
    {
        pInfo->ar.type = PlatformAspectRatioType_eMax;
        pInfo->dynrng = PlatformDynamicRange_eUnknown;
        pInfo->gamut = PlatformColorimetry_eUnknown;
        pInfo->space = PlatformColorSpace_eUnknown;
    }
}

void platform_media_player_p_scheduler_callback(void * pContext, int param)
{
    PlatformMediaPlayerHandle player = pContext;

    BDBG_ASSERT(player);
    (void)param;

    if (platform_media_player_p_get_stream_info(player))
    {
        if (player->streamInfo.callback)
        {
            player->streamInfo.callback(player->streamInfo.context, player->index);
        }
    }
}

void platform_media_player_p_get_default_settings(PlatformMediaPlayerSettings * pSettings)
{
    if (pSettings)
    {
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
        pSettings->usageMode = PlatformUsageMode_eFullScreenVideo;
    }
}

void platform_media_player_get_settings(PlatformMediaPlayerHandle player, PlatformMediaPlayerSettings * pSettings)
{
    BDBG_ASSERT(player);

    if (pSettings)
    {
        BKNI_Memcpy(pSettings, &player->settings, sizeof(*pSettings));
    }
}

int platform_media_player_set_settings(PlatformMediaPlayerHandle player, const PlatformMediaPlayerSettings * pSettings)
{
    int rc = 0;
    NEXUS_SimpleVideoDecoderPictureQualitySettings pqSettings;
    NEXUS_SimpleVideoDecoderHandle svd;

    BDBG_ASSERT(player);
    BDBG_ASSERT(pSettings);
    BKNI_AcquireMutex(player->mutex);
    {
        if (pSettings->usageMode != player->settings.usageMode && player->state != PlatformMediaPlayerState_eInit)
        {
            if (player->streamId != (int)platform_get_main_stream_id(player->platform) || pSettings->usageMode == PlatformUsageMode_eMosaic || player->settings.usageMode == PlatformUsageMode_eMosaic)
            {
                BDBG_WRN(("Can't change usage mode to/from mosaic mode while running"));
                BKNI_ReleaseMutex(player->mutex);
                return -1;
            }
        }
        platform_media_player_p_update_player_impl(player, pSettings->usageMode, &player->pipRequired);
        svd = media_player_get_video_decoder(player->nxPlayer);
        NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(svd, &pqSettings);
        pqSettings.common.sharpnessEnable = pSettings->pqSettings.sharpness.enabled;
        pqSettings.anr.anr.mode = pSettings->pqSettings.anr.enabled ? NEXUS_VideoWindowFilterMode_eEnable : NEXUS_VideoWindowFilterMode_eDisable;
        pqSettings.dnr.bnr.mode = pSettings->pqSettings.dnr.enabled ? NEXUS_VideoWindowFilterMode_eEnable : NEXUS_VideoWindowFilterMode_eDisable;
        pqSettings.dnr.dcr.mode = pSettings->pqSettings.dnr.enabled ? NEXUS_VideoWindowFilterMode_eEnable : NEXUS_VideoWindowFilterMode_eDisable;
        pqSettings.dnr.mnr.mode = pSettings->pqSettings.dnr.enabled ? NEXUS_VideoWindowFilterMode_eEnable : NEXUS_VideoWindowFilterMode_eDisable;
        pqSettings.mad.deinterlace = pSettings->pqSettings.deinterlacing.enabled;
        pqSettings.scaler.horizontalChromaDeringing = pSettings->pqSettings.deringing.enabled;
        pqSettings.scaler.horizontalLumaDeringing = pSettings->pqSettings.deringing.enabled;
        pqSettings.scaler.verticalChromaDeringing = pSettings->pqSettings.deringing.enabled;
        pqSettings.scaler.verticalLumaDeringing = pSettings->pqSettings.deringing.enabled;
        pqSettings.scaler.verticalDejagging = pSettings->pqSettings.dejagging.enabled;
        pqSettings.common.brightness = pSettings->pqSettings.pictureCtrlSettings.brightness;
        pqSettings.common.contrast   = pSettings->pqSettings.pictureCtrlSettings.contrast;
        pqSettings.common.hue        = pSettings->pqSettings.pictureCtrlSettings.hue;
        pqSettings.common.saturation = pSettings->pqSettings.pictureCtrlSettings.saturation;
        rc = NEXUS_SimpleVideoDecoder_SetPictureQualitySettings(svd, &pqSettings);
        BKNI_Memcpy(&player->settings, pSettings, sizeof(*pSettings));
    }
    BKNI_ReleaseMutex(player->mutex);
    return rc;
}

void platform_media_player_p_capture_video(PlatformMediaPlayerHandle player)
{
    if (player->nxPlayer)
    {
        if (player->settings.usageMode == PlatformUsageMode_eMainPip && player->platform->display.maxWindows == 1)
        {
            unsigned pip;
            NEXUS_SimpleVideoDecoderCaptureStatus captureStatus[NUM_CAPTURE_SURFACES];
            NEXUS_SimpleVideoDecoderHandle videoDecoder;

            pip = platform_get_pip_stream_id(player->platform);
            videoDecoder = media_player_get_video_decoder(player->nxPlayer);

            NEXUS_SimpleVideoDecoder_GetCapturedSurfaces(videoDecoder,
                player->platform->media.streams[pip].captures,
                captureStatus,
                NUM_CAPTURE_SURFACES,
                &player->platform->media.streams[pip].validCaptures);
            if (player->platform->media.streams[pip].validCaptures > 1) {
                /* if we get more than one, we recycle the oldest immediately */
                NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(videoDecoder, player->platform->media.streams[pip].captures, player->platform->media.streams[pip].validCaptures-1);
            }
        }
    }
}

void platform_media_player_p_recycle_video(PlatformMediaPlayerHandle player)
{
    if (player->nxPlayer)
    {
        unsigned pip = platform_get_pip_stream_id(player->platform);
        if (player->platform->media.streams[pip].validCaptures)
        {
            NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(
                media_player_get_video_decoder(player->nxPlayer),
                &player->platform->media.streams[pip].captures[player->platform->media.streams[pip].validCaptures-1],
                1);
            player->platform->media.streams[pip].validCaptures = 0;
        }
    }
}
