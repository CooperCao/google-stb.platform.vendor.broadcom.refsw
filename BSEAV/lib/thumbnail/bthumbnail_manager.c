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

BDBG_OBJECT_ID(bthumbnail_manager);
BDBG_OBJECT_ID(bthumbnail_stream);
BDBG_OBJECT_ID(bthumbnail);

BDBG_MODULE(bthumbnail_manager);

void bthumbnail_manager_get_default_settings( bthumbnail_manager_settings *settings )
{
    BKNI_Memset(settings, 0, sizeof(*settings));
    settings->maximum_memory = 16 * 1024 * 1024;
    settings->compressed_buffer_size = 1 * 1024 * 1024;
}

bthumbnail_manager_t bthumbnail_manager_create( const bthumbnail_manager_settings *settings )
{
    bthumbnail_manager_t manager;

    if (settings->maximum_memory == 0) {
        return NULL;
    }

    manager = BKNI_Malloc(sizeof(*manager));
    if (manager == 0) {
        return NULL;
    }
    BKNI_Memset(manager, 0, sizeof(*manager));
    BDBG_OBJECT_SET(manager, bthumbnail_manager);

    BLST_D_INIT(&manager->list);
    BLST_D_INIT(&manager->dangling);
    manager->settings = *settings;
    manager->compressed_buffer = BKNI_Malloc(settings->compressed_buffer_size);

    return manager;
}

void bthumbnail_manager_destroy( bthumbnail_manager_t manager )
{
    bthumbnail_stream_t stream;
    bthumbnail_t thumbnail;

    BDBG_OBJECT_ASSERT(manager, bthumbnail_manager);

    /* coverity[alias] */
    /* coverity[use_after_free] */
    while ((stream = BLST_D_FIRST(&manager->list))) {
        bthumbnail_stream_destroy(stream);
    }
    /* coverity[alias] */
    /* coverity[use_after_free] */
    while ((thumbnail = BLST_D_FIRST(&manager->dangling))) {
        bthumbnail_p_destroy(thumbnail);
    }
    BKNI_Free(manager->compressed_buffer);
    BDBG_OBJECT_UNSET(manager, bthumbnail_manager);
    BKNI_Free(manager);
}

int bthumbnail_manager_get_next_request( bthumbnail_manager_t manager, bthumbnail_t *p_thumbnail )
{
    bthumbnail_stream_t stream;
    bthumbnail_t thumbnail = NULL;

    BDBG_OBJECT_ASSERT(manager, bthumbnail_manager);

    /* this algorithm needs to select the next most important thumbnail that needs to be decoded.
    1) missing thumbnails in a visible view_window
    2) missing thumbnails in a visible full_view
    3) missing thumbnails in an invisible full_view
    */
    for (stream = BLST_D_FIRST(&manager->list); stream; stream = BLST_D_NEXT(stream, link)) {
        if (stream->settings.view_window.visible) {
            thumbnail = bthumbnail_stream_p_create_next_thumbnail(stream);
            if (thumbnail) goto done;
        }
    }
    for (stream = BLST_D_FIRST(&manager->list); stream; stream = BLST_D_NEXT(stream, link)) {
        if (stream->settings.full_view.visible) {
            thumbnail = bthumbnail_stream_p_create_next_thumbnail(stream);
            if (thumbnail) goto done;
        }
    }
    for (stream = BLST_D_FIRST(&manager->list); stream; stream = BLST_D_NEXT(stream, link)) {
        thumbnail = bthumbnail_stream_p_create_next_thumbnail(stream);
        if (thumbnail) goto done;
    }

done:
    if (thumbnail) {
        *p_thumbnail = thumbnail;
        return 0;
    }
    else {
        return -1;
    }
}

int bthumbnail_manager_set_surface( bthumbnail_manager_t manager, bthumbnail_t thumbnail, void *surface, unsigned size )
{
    int result = 0;
    unsigned target_free_space;
    BDBG_OBJECT_ASSERT(manager, bthumbnail_manager);

    thumbnail->data.surface = surface;
    thumbnail->size = size;
    manager->total_size += size;
    manager->total_thumbnails++;

    if (thumbnail->manager) {
        bthumbnail_p_destroy(thumbnail);
    }
    else {
        BDBG_MSG(("setting thumbnail %p, surface %p, time %d, size %d, total_size %d", thumbnail, surface, thumbnail->data.time, size, manager->total_size));
    }

    /* based on last request, how many do we need to free up. also, double it to give some head room. */
    target_free_space = manager->amount_short * size * 2;
    manager->amount_short = 0; /* we've done our best. now clear it. */

    while (manager->total_size+target_free_space > manager->settings.maximum_memory) {
        bool deleted_some = false;
        bthumbnail_stream_t stream;
        BDBG_MSG(("reducing memory from %d to target %d", manager->total_size, manager->settings.maximum_memory));

        /* first, try streams that aren't visible */
        for (stream = BLST_D_FIRST(&manager->list); stream && !deleted_some; stream = BLST_D_NEXT(stream, link)) {
            if (!stream->settings.view_window.visible) {
                deleted_some = bthumbnail_stream_p_delete_thumbnails(stream, false);
            }
        }

        if (!deleted_some) {
            for (stream = BLST_D_FIRST(&manager->list); stream && !deleted_some; stream = BLST_D_NEXT(stream, link)) {
                if (!stream->settings.full_view.visible) {
                    deleted_some = bthumbnail_stream_p_delete_thumbnails(stream, false);
                }
            }
        }

        if (!deleted_some) {
            /* that didn't help. so now try visible streams. */
            for (stream = BLST_D_FIRST(&manager->list); stream && !deleted_some; stream = BLST_D_NEXT(stream, link)) {
                deleted_some = bthumbnail_stream_p_delete_thumbnails(stream, true);
            }

            if (!deleted_some) {
                BDBG_WRN(("bthumbnail_manager unable to reduce memory usage below maximum. please rework algorithm."));
                break;
            }
        }
        BDBG_MSG(("  reduced memory to %d", manager->total_size));
        manager->wait_for_change = true;
    }

    return result;
}

void bthumbnail_manager_clear_visible( bthumbnail_manager_t manager )
{
    bthumbnail_stream_t stream;
    BDBG_OBJECT_ASSERT(manager, bthumbnail_manager);
    for (stream = BLST_D_FIRST(&manager->list); stream; stream = BLST_D_NEXT(stream, link)) {
        stream->settings.view_window.visible = false;
        stream->settings.full_view.visible = false;
    }
    manager->wait_for_change = false;
}

void bthumbnail_manager_p_delete_surface(bthumbnail_manager_t manager, bthumbnail_t thumbnail)
{
    BDBG_OBJECT_ASSERT(manager, bthumbnail_manager);
    BDBG_ASSERT(manager == thumbnail->data.stream->manager);

    BDBG_MSG(("delete thumbnail %p, surface %p, time %d", thumbnail, thumbnail->data.surface, thumbnail->data.time));
    (*manager->settings.delete_surface)( manager->settings.callback_context, thumbnail->data.surface);
    manager->total_size -= thumbnail->size;
    manager->total_thumbnails--;
}
