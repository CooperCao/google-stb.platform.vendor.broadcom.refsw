/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
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

PlatformMediaPlayerContext gPlayerContext =
{
    { 0, 0, 0, 0 },
    0,
    { false, { 0, 0 }, NEXUS_VideoFormat_eUnknown, 0, 0, 0 },
    0
};

const int PLATFORM_TRICK_RATE_1X = NEXUS_NORMAL_DECODE_RATE;

PlatformMediaPlayerHandle platform_media_player_create(PlatformHandle platform, PlatformCallback streamInfoCallback, void * streamInfoContext)
{
    PlatformMediaPlayerHandle player;

    BDBG_ASSERT(platform);
    BDBG_ASSERT(platform->gfx);

    if(gPlayerContext.count >= MAX_MOSAICS) {
        BDBG_ERR(("Failed to create platform media player. Max number (%d) of players already created.", MAX_MOSAICS));
        return NULL;
    }

    player = BKNI_Malloc(sizeof(*player));
    BDBG_ASSERT(player);
    BKNI_Memset(player, 0, sizeof(*player));

    BKNI_CreateMutex(&player->mutex);
    platform->player = player;
    player->platform = platform;
    player->streamInfo.callback = streamInfoCallback;
    player->streamInfo.context = streamInfoContext;

    if(!gPlayerContext.count) {
        unsigned i;
        media_player_get_default_create_settings(&gPlayerContext.nxCreateSettings);
        for (i=0; i<(sizeof(platform->gfx->sc)/sizeof(platform->gfx->sc[0])); i++) {
            if (platform->gfx->sc[i].numWindows <= 1) break;
            gPlayerContext.nxCreateSettings.window.surfaceClientId = platform->gfx->sc[i].id;
            media_player_create_mosaics(gPlayerContext.nxPlayerMosaic, platform->gfx->sc[i].numWindows, &gPlayerContext.nxCreateSettings);
            BDBG_ASSERT(gPlayerContext.nxPlayerMosaic[0]);
        }
        if (platform->gfx->sc[i].numWindows)
            gPlayerContext.nxCreateSettings.window.surfaceClientId = platform->gfx->sc[i].id;
        gPlayerContext.nxCreateSettings.maxWidth = 3840;
        gPlayerContext.nxCreateSettings.maxHeight = 2160;
        gPlayerContext.nxPlayer = media_player_create(&gPlayerContext.nxCreateSettings);
        BDBG_ASSERT(gPlayerContext.nxPlayer);
    }

    player->index = gPlayerContext.count++;
    player->streamInfoGatherer = platform_scheduler_add_listener(platform_get_scheduler(platform), &platform_media_player_p_scheduler_callback, player);
    if (!player->streamInfoGatherer)
    {
        BDBG_WRN(("Unable to monitor stream info"));
    }

    return player;
}

void platform_media_player_destroy(PlatformMediaPlayerHandle player)
{
    if (!player) return;
    if (player->streamInfoGatherer)
    {
        platform_scheduler_remove_listener(platform_get_scheduler(player->platform), player->streamInfoGatherer);
    }
    BKNI_AcquireMutex(player->mutex);
    player->nxPlayer = NULL;
    player->platform->player = NULL;
    BKNI_ReleaseMutex(player->mutex);
    BKNI_DestroyMutex(player->mutex);
    BKNI_Free(player);
    gPlayerContext.count--;
    if (!gPlayerContext.count) {
        media_player_destroy_mosaics(gPlayerContext.nxPlayerMosaic, player->platform->gfx->sc[0].numWindows);
        media_player_destroy(gPlayerContext.nxPlayer);
    }
}

int platform_media_player_start(PlatformMediaPlayerHandle player, const char * url, bool mosaic)
{
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderClientSettings settings;
    NEXUS_VideoDecoderExtendedSettings extendedSettings;
    NEXUS_VideoDecoderSettings decoderSettings;
    bool pip_required = false;
    int rc;

    BDBG_ASSERT(player);

    BKNI_AcquireMutex(player->mutex);

    if (mosaic)
        if (player->index < player->platform->gfx->maxMosaics) {
            player->nxPlayer = gPlayerContext.nxPlayerMosaic[player->index];
         } else {
            player->nxPlayer = gPlayerContext.nxPlayer;
            pip_required = true;
         }
    else
        player->nxPlayer = gPlayerContext.nxPlayer;

    /* TEMP disable CC for mosaics */
    videoDecoder = media_player_get_video_decoder(player->nxPlayer);
    NEXUS_SimpleVideoDecoder_GetClientSettings(videoDecoder, &settings);
    settings.closedCaptionRouting = false;
    NEXUS_SimpleVideoDecoder_SetClientSettings(videoDecoder, &settings);

    NEXUS_SimpleVideoDecoder_GetSettings(videoDecoder, &decoderSettings);
    decoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    NEXUS_SimpleVideoDecoder_SetSettings(videoDecoder, &decoderSettings);

    NEXUS_SimpleVideoDecoder_GetExtendedSettings(videoDecoder, &extendedSettings);
    if (strstr(url, "/hdr/") && strstr(url, ".webm")) {
        extendedSettings.eotf = NEXUS_VideoEotf_eHdr10;
    } else {
        extendedSettings.eotf = NEXUS_VideoEotf_eInvalid;
    }
    NEXUS_SimpleVideoDecoder_SetExtendedSettings(videoDecoder, &extendedSettings);

    media_player_get_default_start_settings(&player->nxStartSettings);
    player->nxStartSettings.stream_url = url;
    player->nxStartSettings.audio.pid = 0;
    player->nxStartSettings.forceStopDisconnect = true;
    if (pip_required) player->nxStartSettings.videoWindowType = NxClient_VideoWindowType_ePip;
    rc = media_player_start(player->nxPlayer, &player->nxStartSettings);
    if (!rc)
    {
        player->trickRate = PLATFORM_TRICK_RATE_1X;
        player->state = PlatformMediaPlayerState_ePlaying;
        platform_scheduler_wake(platform_get_scheduler(player->platform));
    }
    else { BDBG_ERR(("unable to start media player %d for '%s': %d", player->index, player->nxStartSettings.stream_url, rc)); }
    BKNI_ReleaseMutex(player->mutex);
    return rc;
}

void platform_media_player_stop(PlatformMediaPlayerHandle player)
{
    BDBG_ASSERT(player);
    BKNI_AcquireMutex(player->mutex);
    if(!player->nxPlayer)
    {
        BKNI_ReleaseMutex(player->mutex);
        return;
    }
    player->state = PlatformMediaPlayerState_eInit;
    media_player_stop(player->nxPlayer);
    player->nxPlayer = NULL;
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
        platform_scheduler_wake(platform_get_scheduler(player->platform));
    }
    media_player_trick(player->nxPlayer, rate);
    BKNI_ReleaseMutex(player->mutex);
}

void platform_media_player_frame_advance(PlatformMediaPlayerHandle player, bool mosaic)
{
    BDBG_ASSERT(player);
    BKNI_AcquireMutex(player->mutex);
    if (mosaic)
        if (player->index < player->platform->gfx->maxMosaics) {
            player->nxPlayer = gPlayerContext.nxPlayerMosaic[player->index];
         } else {
            player->nxPlayer = gPlayerContext.nxPlayer;
         }
    else
        player->nxPlayer = gPlayerContext.nxPlayer;

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
        pInfo->dynrng = platform_p_dynamic_range_from_nexus(player->streamInfo.nx.eotf, player->streamInfo.nx.dolbyVision);
        pInfo->gamut = platform_p_colorimetry_from_nexus(player->streamInfo.nx.matrixCoefficients);
        pInfo->space = PlatformColorSpace_eYCbCr422;
        pInfo->depth = player->streamInfo.nx.colorDepth;
        pInfo->format.width = player->streamInfo.nx.sourceHorizontalSize;
        pInfo->format.height = player->streamInfo.nx.sourceVerticalSize;
        pInfo->format.interlaced = !player->streamInfo.nx.streamProgressive;
        pInfo->format.rate = platform_p_frame_rate_from_nexus(player->streamInfo.nx.frameRate);
    }
    else
    {
        pInfo->dynrng = PlatformDynamicRange_eUnknown;
        pInfo->gamut = PlatformColorimetry_eUnknown;
        pInfo->space = PlatformColorSpace_eUnknown;
    }
}

void platform_media_player_p_scheduler_callback(void * pContext, int param)
{
    PlatformMediaPlayerHandle player = pContext;

    BDBG_ASSERT(player);

    if (platform_media_player_p_get_stream_info(player))
    {
        if (player->streamInfo.callback)
        {
            player->streamInfo.callback(player->streamInfo.context, player->index);
        }
    }
}
