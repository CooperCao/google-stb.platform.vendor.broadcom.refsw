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
#include "stream_player.h"
#include "stream_player_priv.h"
#include "util_priv.h"
#include "platform.h"
#include "blst_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

bool stream_player_p_file_filter(const char * path)
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

StreamPlayerHandle stream_player_create(PlatformHandle platform, PlatformCallback streamInfoCallback, void * streamInfoContext)
{
    StreamPlayerHandle player;

    player = malloc(sizeof(*player));
    assert(player);
    memset(player, 0, sizeof(*player));
    player->platformPlayer = platform_media_player_create(platform, streamInfoCallback, streamInfoContext);
    assert(player->platformPlayer);
    BLST_Q_INIT(&player->sources);
    return player;
}

void stream_player_destroy(StreamPlayerHandle player)
{
    StreamSource * pSource;

    if (!player) return;

    for (pSource = BLST_Q_FIRST(&player->sources); pSource; pSource = BLST_Q_FIRST(&player->sources))
    {
        BLST_Q_REMOVE(&player->sources, pSource, link);
        if (pSource->switcher)
        {
            file_switcher_destroy(pSource->switcher);
        }
        free(pSource);
    }
    platform_media_player_destroy(player->platformPlayer);
    free(player);
}

void stream_player_add_stream_source(StreamPlayerHandle player, const char * name, const char * path)
{
    StreamSource * pSource;

    assert(player);

    pSource = malloc(sizeof(*pSource));
    assert(pSource);
    memset(pSource, 0, sizeof(*pSource));
    pSource->switcher = file_switcher_create(name, path, &stream_player_p_file_filter, false);
    assert(pSource->switcher);
    pSource->eotf = stream_player_p_get_eotf_from_source_name(name);
    BLST_Q_INSERT_TAIL(&player->sources, pSource, link);
}

static const char * eotfStrings[] =
{
    "AUTO",
    "SDR",
    "HLG",
    "HDR10",
    "INVALID",
    "UNKNOWN",
    NULL
};

void stream_player_print(StreamPlayerHandle player)
{
    assert(player);

    if (!player->pCurrentSource) { printf("No current stream source\n"); return; }
    printf("Currently playing '%s' (%s)\n", file_switcher_get_path(player->pCurrentSource->switcher), eotfStrings[player->pCurrentSource->eotf]);
}

void stream_player_next(StreamPlayerHandle player, bool mosaic)
{
    assert(player);

    if (!player->pCurrentSource) { printf("next: no current stream source\n"); return; }

    stream_player_stop(player);
    if (file_switcher_get_position(player->pCurrentSource->switcher) + 1 < (int)file_switcher_get_count(player->pCurrentSource->switcher))
    {
        file_switcher_next(player->pCurrentSource->switcher);
    }
    else
    {
        do
        {
            player->pCurrentSource = BLST_Q_NEXT(player->pCurrentSource, link);
            if (!player->pCurrentSource) { player->pCurrentSource = BLST_Q_FIRST(&player->sources); }
        } while (!file_switcher_get_count(player->pCurrentSource->switcher));
        file_switcher_first(player->pCurrentSource->switcher);
    }
    stream_player_start(player, mosaic);
}

void stream_player_prev(StreamPlayerHandle player, bool mosaic)
{
    assert(player);

    if (!player->pCurrentSource) { printf("prev: no current stream source\n"); return; }

    stream_player_stop(player);
    file_switcher_prev(player->pCurrentSource->switcher);
    while (!file_switcher_get_path(player->pCurrentSource->switcher))
    {
        player->pCurrentSource = BLST_Q_PREV(player->pCurrentSource, link);
        if (!player->pCurrentSource) { player->pCurrentSource = BLST_Q_LAST(&player->sources); }
        if (player->pCurrentSource)
        {
            file_switcher_last(player->pCurrentSource->switcher);
        }
    }
    stream_player_start(player, mosaic);
}

void stream_player_first(StreamPlayerHandle player, bool mosaic)
{
    assert(player);

    stream_player_stop(player);
    player->pCurrentSource = BLST_Q_FIRST(&player->sources);
    if (player->pCurrentSource)
    {
        file_switcher_first(player->pCurrentSource->switcher);
    }
    stream_player_start(player, mosaic);
}

unsigned stream_player_get_count(StreamPlayerHandle player)
{
    unsigned count = 0;
    StreamSource * pSource;
    for (pSource = BLST_Q_FIRST(&player->sources); pSource; pSource = BLST_Q_NEXT(pSource, link))
    {
        count += file_switcher_get_count(pSource->switcher);
    }
    return count;
}

static void stream_player_p_play_stream(StreamPlayerHandle player, StreamSource * pSource, int streamIndex, bool mosaic, bool forceRestart)
{
    StreamSource * pOldSource = NULL;
    int oldPosition = -1;

    pOldSource = player->pCurrentSource;
    if (pOldSource)
    {
        oldPosition = file_switcher_get_position(pOldSource->switcher);
    }

    if (!forceRestart && ((pOldSource != pSource) || (oldPosition != streamIndex)))
    {
        stream_player_stop(player);
        player->pCurrentSource = pSource;
        if (pSource)
        {
            file_switcher_set_position(pSource->switcher, streamIndex);
            player->pCurrentUrl = set_string(player->pCurrentUrl, file_switcher_get_path(pSource->switcher));
            printf("stream_player: playing stream '%s'\n", player->pCurrentUrl);
        }
        stream_player_start(player, mosaic);
    }
}

void stream_player_play_stream_by_index(StreamPlayerHandle player, int streamIndex, bool mosaic, bool forceRestart)
{
    StreamSource * pSource = NULL;
    unsigned count = 0;

    if (streamIndex >= (int)stream_player_get_count(player)) { printf("stream_player: stream index %d out of bounds (%u)\n", streamIndex, count); return; }
    for (pSource = BLST_Q_FIRST(&player->sources); pSource; pSource = BLST_Q_NEXT(pSource, link))
    {
        count = file_switcher_get_count(pSource->switcher);
        if (streamIndex < (int)count)
        {
            break;
        }
        streamIndex -= count;
    }

    stream_player_p_play_stream(player, pSource, streamIndex, mosaic, forceRestart);
}

void stream_player_p_play_stream_by_path(StreamPlayerHandle player, const char * streamPath, bool mosaic, bool forceRestart)
{
    StreamSource * pSource = NULL;
    int streamIndex = -1;

    for (pSource = BLST_Q_FIRST(&player->sources); pSource; pSource = BLST_Q_NEXT(pSource, link))
    {
        streamIndex = file_switcher_find(pSource->switcher, streamPath);
        if (streamIndex != -1)
        {
            break;
        }
    }

    if (streamIndex == -1) { printf("stream_player: stream path '%s' not found\n", streamPath); return; }

    stream_player_p_play_stream(player, pSource, streamIndex, mosaic, forceRestart);
}

void stream_player_play_stream_by_url(StreamPlayerHandle player, const char * streamUrl, bool mosaic, bool forceRestart)
{
    if (!streamUrl) { printf("stream_player: NULL stream url\n"); return; }

    if (strchr(streamUrl, ':') || streamUrl[0] == '/')
    {
        if (!forceRestart && player->pCurrentUrl && !strcmp(player->pCurrentUrl, streamUrl)) return;
        if (player->started)
        {
            stream_player_stop(player);
        }
        player->pCurrentUrl = set_string(player->pCurrentUrl, streamUrl);
        printf("stream_player: playing stream '%s'\n", player->pCurrentUrl);
        stream_player_start(player, mosaic);
    }
    else
    {
        stream_player_p_play_stream_by_path(player, streamUrl, mosaic, forceRestart);
    }
}

void stream_player_stop(StreamPlayerHandle player)
{
    assert(player);
    if (!player->pCurrentUrl) { printf("stop: no current stream\n"); return; }

#ifdef DEBUG
    printf("Stopping '%s'\n", player->pCurrentUrl);
#endif
    platform_media_player_stop(player->platformPlayer);
    player->started = false;
}

void stream_player_start(StreamPlayerHandle player, bool mosaic)
{
    int rc;
    assert(player);
    if (!player->pCurrentUrl) { printf("start: no current stream\n"); return; }
    rc = platform_media_player_start(player->platformPlayer, player->pCurrentUrl, mosaic);
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

PlatformDynamicRange stream_player_p_get_eotf_from_source_name(const char * sourceName)
{
    assert(sourceName);

    if
    (
        strstr(sourceName, "sdr")
        ||
        strstr(sourceName, "SDR")
        ||
        strstr(sourceName, "Sdr")
    )
    {
        return PlatformDynamicRange_eSdr;
    }
    else if
    (
        strstr(sourceName, "hdr")
        ||
        strstr(sourceName, "HDR")
        ||
        strstr(sourceName, "Hdr")
    )
    {
        return PlatformDynamicRange_eHdr10;
    }
    else if
    (
        strstr(sourceName, "hlg")
        ||
        strstr(sourceName, "HLG")
        ||
        strstr(sourceName, "Hlg")
    )
    {
        return PlatformDynamicRange_eHlg;
    }
    else if
    (
        strstr(sourceName, "mix")
        ||
        strstr(sourceName, "Mix")
        ||
        strstr(sourceName, "MIX")
    )
    {
        return PlatformDynamicRange_eHdr10;
    }
    else
    {
        return PlatformDynamicRange_eInvalid;
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

void stream_player_frame_advance(StreamPlayerHandle player, bool mosaic)
{
    assert(player);
    platform_media_player_frame_advance(player->platformPlayer, mosaic);
    printf("stream_player: frame advanced\n");
}
