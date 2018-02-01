/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "stream_player.h"
#include "stream_player_priv.h"
#include "platform.h"
#include "blst_queue.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>



bool stream_player_file_filter(const char * path)
{
    assert(path);

    if
    (
        strstr(path, ".ts")
        ||
        strstr(path, ".trp")
        ||
        strstr(path, ".mpg")
        ||
        strstr(path, ".m2ts")
        ||
        strstr(path, ".mpeg2ts")
        ||
        strstr(path, ".mp4")
        ||
        strstr(path, ".mkv")
        ||
        strstr(path, ".wmv")
        ||
        strstr(path, ".avi")
        ||
        strstr(path, ".asf")
        ||
        strstr(path, ".webm")
    )
    {
        return true;
    }
    else
    {
        return false;
    }
}

StreamPlayerHandle stream_player_create(PlatformMediaPlayerHandle platformPlayer)
{
    StreamPlayerHandle player;

    assert(platformPlayer);

    player = malloc(sizeof(*player));
    if (!player) goto error;
    memset(player, 0, sizeof(*player));
    player->platformPlayer = platformPlayer;
    assert(player->platformPlayer);
    BLST_Q_INIT(&player->sources);

    return player;

error:
    stream_player_destroy(player);
    return NULL;
}


void stream_player_destroy(StreamPlayerHandle player)
{
    StreamSource * pSource;

    if (!player) return;

    for (pSource = BLST_Q_FIRST(&player->sources); pSource; pSource = BLST_Q_FIRST(&player->sources))
    {
        BLST_Q_REMOVE(&player->sources, pSource, link);
        free(pSource);
    }
    free(player);
}

void stream_player_add_stream_source(StreamPlayerHandle player, FileManagerHandle filer)
{
    StreamSource * pSource;

    assert(player);
    assert(filer);

    pSource = malloc(sizeof(*pSource));
    assert(pSource);
    memset(pSource, 0, sizeof(*pSource));
    pSource->filer = filer;
    BLST_Q_INSERT_TAIL(&player->sources, pSource, link);
}

void stream_player_print(StreamPlayerHandle player)
{
    assert(player);

    if (!player->pCurrentUrl) { printf("No current stream source\n"); return; }
    printf("Currently playing '%s'\n", player->pCurrentUrl);
}

unsigned stream_player_get_count(StreamPlayerHandle player)
{
    unsigned count = 0;
    StreamSource * pSource;
    for (pSource = BLST_Q_FIRST(&player->sources); pSource; pSource = BLST_Q_NEXT(pSource, link))
    {
        count += file_manager_get_count(pSource->filer);
    }
    return count;
}

static void stream_player_p_play_stream(StreamPlayerHandle player, const char * newPath, const StreamPlayerPlaySettings * pSettings)
{
    const char * oldPath = player->pCurrentUrl;

    /* old and new will be different pointers, but have same content */
    if (pSettings->forceRestart || (oldPath && newPath && strcmp(oldPath, newPath)) || (!oldPath && newPath) || (oldPath && !newPath))
    {
        if (player->started)
        {
            stream_player_stop(player);
        }
        player->pCurrentUrl = set_string(player->pCurrentUrl, newPath);
        printf("stream_player: playing stream '%s'\n", player->pCurrentUrl);
        memcpy(&player->playSettings, pSettings, sizeof(*pSettings));
    }
    if (!player->started)
    {
        stream_player_start(player);
    }
}

static void stream_player_p_play_stream_by_path(StreamPlayerHandle player, const StreamPlayerPlaySettings * pSettings)
{
    StreamSource * pSource = NULL;
    const char * fullPath = NULL;

    for (pSource = BLST_Q_FIRST(&player->sources); pSource; pSource = BLST_Q_NEXT(pSource, link))
    {
        fullPath = file_manager_find(pSource->filer, pSettings->streamUrl);
        if (fullPath)
        {
            break;
        }
    }

    if (!fullPath) { printf("stream_player: stream path '%s' not found\n", pSettings->streamUrl); return; }

    stream_player_p_play_stream(player, fullPath, pSettings);
}

void stream_player_get_platform_settings(StreamPlayerHandle player, PlatformMediaPlayerSettings * pSettings)
{
    assert(player);
    platform_media_player_get_settings(player->platformPlayer, pSettings);
}

int stream_player_set_platform_settings(StreamPlayerHandle player, const PlatformMediaPlayerSettings * pSettings)
{
    PlatformMediaPlayerSettings oldSettings;
    assert(player);
    assert(pSettings);
    platform_media_player_get_settings(player->platformPlayer, &oldSettings);
    if (player->started && pSettings->usageMode != oldSettings.usageMode) { printf("stream_player: can't change usage mode while started\n"); return -1; }
    return platform_media_player_set_settings(player->platformPlayer, pSettings);
}

void stream_player_get_default_play_settings(StreamPlayerPlaySettings * pSettings)
{
    if (pSettings)
    {
        memset(pSettings, 0, sizeof(*pSettings));
    }
}

void stream_player_play_stream(StreamPlayerHandle player, const StreamPlayerPlaySettings * pSettings)
{
    if (!pSettings) { printf("stream_player: NULL play settings\n"); return; }
    if (!pSettings->streamUrl) { printf("stream_player: NULL stream url\n"); return; }

    if (strchr(pSettings->streamUrl, ':') || pSettings->streamUrl[0] == '/')
    {
        stream_player_p_play_stream(player, pSettings->streamUrl, pSettings);
    }
    else
    {
        stream_player_p_play_stream_by_path(player, pSettings);
    }
}

void stream_player_stop(StreamPlayerHandle player)
{
    assert(player);
    if (!player->pCurrentUrl) { printf("stop: no current stream\n"); return; }

#if DEBUG
    printf("Stopping '%s'\n", player->pCurrentUrl);
#endif
    platform_media_player_stop(player->platformPlayer);
    player->started = false;
}

void stream_player_start(StreamPlayerHandle player)
{
    int rc;
    PlatformMediaPlayerStartSettings settings;
    assert(player);
    if (!player->pCurrentUrl) { printf("start: no current stream\n"); return; }
    player->paused = player->playSettings.startPaused;
    platform_media_player_get_default_start_settings(&settings);
    settings.url = player->pCurrentUrl;
    settings.playMode = player->playSettings.playMode;
    settings.startPaused = player->playSettings.startPaused;
    settings.stcTrick = player->playSettings.stcTrick;
    rc = platform_media_player_start(player->platformPlayer, &settings);
    if (!rc) { stream_player_print(player); }
    player->started = true;
}

const PlatformPictureInfo * stream_player_get_picture_info(StreamPlayerHandle player)
{
    if (player && player->platformPlayer)
    {
        platform_media_player_get_picture_info(player->platformPlayer, &player->info);
        return &player->info;
    }
    else
    {
        return NULL;
    }
}

void stream_player_toggle_pause(StreamPlayerHandle player)
{
    assert(player);

    /* toggle stream pause/play */
    if (player->paused)
    {
        platform_media_player_trick(player->platformPlayer, PLATFORM_TRICK_RATE_1X);
        player->paused = false;
    }
    else
    {
        platform_media_player_trick(player->platformPlayer, 0);
        player->paused = true;
    }
}

void stream_player_frame_advance(StreamPlayerHandle player)
{
    assert(player);

    if (!player->paused)
    {
        stream_player_toggle_pause(player);
    }

    platform_media_player_frame_advance(player->platformPlayer);
}
