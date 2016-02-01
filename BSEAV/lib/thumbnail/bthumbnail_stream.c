/******************************************************************************
 * (c) 2002-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bthumbnail.h"
#include "bthumbnail_priv.h"

BDBG_MODULE(bthumbnail_stream);

void bthumbnail_stream_get_default_create_settings( bthumbnail_stream_create_settings *create_settings )
{
    BKNI_Memset(create_settings, 0, sizeof(*create_settings));
}

bthumbnail_stream_t bthumbnail_stream_create( bthumbnail_manager_t manager, const bthumbnail_stream_create_settings *create_settings )
{
    bthumbnail_stream_t stream;

    stream = BKNI_Malloc(sizeof(*stream));
    if (stream == 0) {
        return NULL;
    }
    BKNI_Memset(stream, 0, sizeof(*stream));
    BDBG_OBJECT_SET(stream, bthumbnail_stream);

    stream->create_settings = *create_settings;
    BLST_D_INIT(&stream->list);
    BLST_D_INSERT_HEAD(&manager->list, stream, link);

    stream->manager = manager;
    stream->settings.view_window.spacing = 30; /* 30 seconds */
    stream->settings.view_window.first_time = 0;
    stream->settings.view_window.last_time = 5 * 60; /* 5 minutes */
    stream->settings.full_view.number = 5;
    stream->settings.full_view.min_spacing = 15; /* seconds */
    manager->wait_for_change = false;

    return stream;
}

void bthumbnail_stream_destroy( bthumbnail_stream_t stream )
{
    bthumbnail_t thumbnail;

    BDBG_OBJECT_ASSERT(stream, bthumbnail_stream);

    /* coverity[alias] */
    /* coverity[use_after_free] */
    while ((thumbnail = BLST_D_FIRST(&stream->list))) {
        bthumbnail_p_destroy(thumbnail);
    }

    BLST_D_REMOVE(&stream->manager->list, stream, link);
    BDBG_OBJECT_UNSET(stream, bthumbnail_stream);
    BKNI_Free(stream);
}

void bthumbnail_stream_get_settings( bthumbnail_stream_t stream, bthumbnail_stream_settings *settings )
{
    BDBG_OBJECT_ASSERT(stream, bthumbnail_stream);
    *settings = stream->settings;
}

int bthumbnail_stream_set_settings( bthumbnail_stream_t stream, const bthumbnail_stream_settings *settings )
{
    BDBG_OBJECT_ASSERT(stream, bthumbnail_stream);
    stream->settings = *settings;
    stream->manager->wait_for_change = false;
    /* TODO: flush thumbnails that don't line up with new values */
    return 0;
}

#define ALIGN(num,spacing) \
    do { \
        if (spacing && num%spacing) { \
            if (num < spacing) { \
                num = spacing; \
            } \
            else { \
                num -= num%spacing; \
            } \
       } \
    } while (0)

int bthumbnail_p_stream_get_thumbnails( bthumbnail_stream_t stream, unsigned first_time, unsigned last_time,
    bthumbnail_t *thumbnail_array, unsigned total, unsigned *total_read, unsigned spacing )
{
    unsigned target_time;
    bthumbnail_t thumbnail;

    BSTD_UNUSED(last_time);
    BDBG_OBJECT_ASSERT(stream, bthumbnail_stream);

    *total_read = 0;
    target_time = first_time;

    ALIGN(target_time, spacing);

    BDBG_MSG(("get_thumbnails starting at %d", target_time));
#if 0
/* TODO: this code has accurate spacing, but shows holes if a thumbnail is missing. */
    for (thumbnail = BLST_D_FIRST(&stream->list); thumbnail; ) {
        if (*total_read == total) {
            break;
        }
        if (spacing && thumbnail->data.time % spacing) {
            BDBG_MSG(("  skipping %d", thumbnail->data.time));
            thumbnail = BLST_D_NEXT(thumbnail, link);
            continue;
        }

        if (thumbnail->data.time > target_time) {
            BDBG_MSG(("  no thumbnail at %d", target_time));
            thumbnail_array[*total_read] = NULL;
            (*total_read)++;
            target_time += spacing;
        }
        else if (thumbnail->data.time == target_time) {
            if (thumbnail->data.surface && !thumbnail->manager) {
                BDBG_MSG(("  thumbnail at %d", target_time));
                thumbnail_array[*total_read] = thumbnail;
            }
            else {
                BDBG_MSG(("  NULL thumbnail at %d", target_time));
                thumbnail_array[*total_read] = NULL;
            }
            (*total_read)++;
            target_time += spacing;
            thumbnail = BLST_D_NEXT(thumbnail, link);
        }
        else {
            BDBG_MSG(("  skipping %d", thumbnail->data.time));
            thumbnail = BLST_D_NEXT(thumbnail, link);
        }
    }
#else
    for (thumbnail = BLST_D_FIRST(&stream->list); thumbnail; ) {
        if (*total_read == total) {
            break;
        }
        if (spacing && thumbnail->data.time % spacing) {
            BDBG_MSG(("  skipping %d", thumbnail->data.time));
            thumbnail = BLST_D_NEXT(thumbnail, link);
            continue;
        }

        if (thumbnail->data.time > target_time) {
            target_time += spacing;
        }
        else if (thumbnail->data.time == target_time && thumbnail->data.surface && !thumbnail->manager) {
            BDBG_MSG(("  thumbnail at %d", target_time));
            thumbnail_array[*total_read] = thumbnail;
            (*total_read)++;
            target_time += spacing;
            thumbnail = BLST_D_NEXT(thumbnail, link);
        }
        else {
            BDBG_MSG(("  skipping %d", thumbnail->data.time));
            thumbnail = BLST_D_NEXT(thumbnail, link);
        }
    }
#endif

    stream->manager->amount_short = total - *total_read;

    return 0;
}

int bthumbnail_stream_get_thumbnails( bthumbnail_stream_t stream, unsigned first_time, unsigned last_time,
    bthumbnail_t *thumbnail_array, unsigned total, unsigned *total_read )
{
    return bthumbnail_p_stream_get_thumbnails(stream, first_time, last_time,
        thumbnail_array, total, total_read, stream->settings.view_window.spacing);
}

int bthumbnail_stream_get_full_view_thumbnails( bthumbnail_stream_t stream,
    bthumbnail_t *thumbnail_array, unsigned total, unsigned *total_read )
{
    return bthumbnail_p_stream_get_thumbnails(stream, stream->settings.first_time, 0xFFFFFFFF,
        thumbnail_array, total, total_read, stream->actual_full_view_spacing);
}

#define BDBG_MSG_SEARCH(X) /* BDBG_MSG */

bthumbnail_t bthumbnail_stream_p_create_next_thumbnail(bthumbnail_stream_t stream)
{
    bthumbnail_t thumbnail;
    unsigned spacing;
    unsigned target_first_time, target_last_time;

    BDBG_OBJECT_ASSERT(stream, bthumbnail_stream);

    if (stream->settings.view_window.visible) {
        spacing = stream->settings.view_window.spacing;
        target_first_time = stream->settings.view_window.first_time;
        target_last_time = stream->settings.view_window.last_time;

        if (target_first_time > stream->settings.last_time || target_last_time < stream->settings.first_time) {
            BDBG_MSG_SEARCH(("search stream %p: out of view_window", stream));
            return NULL;
        }

        if (target_first_time < stream->settings.first_time)
            target_first_time = stream->settings.first_time;
        if (target_last_time > stream->settings.last_time)
            target_last_time = stream->settings.last_time;
    }
    else if (stream->settings.full_view.visible) {
        spacing = stream->settings.full_view.number ? (stream->settings.last_time - stream->settings.first_time) / stream->settings.full_view.number : stream->settings.full_view.min_spacing;
        if (spacing < stream->settings.full_view.min_spacing) {
            spacing = stream->settings.full_view.min_spacing;
        }
        target_first_time = stream->settings.first_time;
        target_last_time = stream->settings.last_time;
        stream->actual_full_view_spacing = spacing; /* remember this for our get */
    }
    else {
        BDBG_MSG_SEARCH(("search stream %p: invisible", stream));
        return NULL;
    }

    ALIGN(target_first_time, spacing);

    BDBG_MSG_SEARCH(("search stream %p: range %d..%d, %d spacing", stream, target_first_time, target_last_time, spacing));
    /* create view_window thumbnail */
    for (thumbnail = BLST_D_FIRST(&stream->list); thumbnail && target_first_time <= target_last_time; thumbnail = BLST_D_NEXT(thumbnail, link)) {
        if (thumbnail->data.time < target_first_time) {
            continue;
        }
        else if (thumbnail->data.time == target_first_time) {
            /* we already have it. try the next. */
            target_first_time += spacing;
        }
        else {
            /* we don't have it, so create one. */
            return bthumbnail_p_create_thumbnail(stream, target_first_time);
        }
    }
    if (!thumbnail && target_first_time <= target_last_time) {
        return bthumbnail_p_create_thumbnail(stream, target_first_time);
    }

    /* this stream has all the thumbnails it wants */
    return NULL;
}

void bthumbnail_stream_get_data( bthumbnail_stream_t stream, bthumbnail_stream_data *data )
{
    BDBG_OBJECT_ASSERT(stream, bthumbnail_stream);
    data->total_thumbnails = 0; /* TODO */
}

void bthumbnail_stream_get_create_settings( bthumbnail_stream_t stream, bthumbnail_stream_create_settings *create_settings )
{
    BDBG_OBJECT_ASSERT(stream, bthumbnail_stream);
    *create_settings = stream->create_settings;
}

bool bthumbnail_stream_p_delete_thumbnails(bthumbnail_stream_t stream, bool aggressive)
{
    bthumbnail_t thumbnail;
    bthumbnail_t furthest = NULL;
    unsigned max_distance = 0;

    BDBG_OBJECT_ASSERT(stream, bthumbnail_stream);
    /* if neither view_window or full_view is visible, delete thumbnails. also, if only full view is visible,
    delete non-full view thumbnails */
    if (!stream->settings.view_window.visible) {
        for (thumbnail = BLST_D_FIRST(&stream->list); thumbnail; thumbnail = BLST_D_NEXT(thumbnail, link)) {
            if (!stream->settings.full_view.visible || !stream->actual_full_view_spacing || thumbnail->data.time % stream->actual_full_view_spacing) {
                BDBG_MSG(("delete from %d %d %d", stream->settings.full_view.visible, thumbnail->data.time, stream->actual_full_view_spacing));
                bthumbnail_p_destroy(thumbnail);
                return true;
            }
        }
    }
    if (!aggressive) return false;

    /* we have to delete thumbnails from this stream. delete the thumbnail which is furthest from that view window. */
    for (thumbnail = BLST_D_FIRST(&stream->list); thumbnail; thumbnail = BLST_D_NEXT(thumbnail, link)) {
        unsigned d;
        if (thumbnail->data.time < stream->settings.view_window.first_time) {
            d = stream->settings.view_window.first_time - thumbnail->data.time;
        }
        else {
            d = thumbnail->data.time - stream->settings.view_window.first_time;
        }
        if (d >= max_distance) {
            max_distance = d;
            furthest = thumbnail;
        }
    }
    if (furthest) {
        BDBG_MSG(("delete furthest %d %d", max_distance));
        bthumbnail_p_destroy(furthest);
        return true;
    }

    return false;
}
