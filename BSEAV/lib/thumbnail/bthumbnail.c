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

BDBG_MODULE(bthumbnail);

int bthumbnail_get_data( bthumbnail_t thumbnail, bthumbnail_data *data )
{
    BDBG_OBJECT_ASSERT(thumbnail, bthumbnail);
    *data = thumbnail->data;
    return 0;
}

bthumbnail_t bthumbnail_p_create_thumbnail( bthumbnail_stream_t stream, unsigned time )
{
    bthumbnail_t thumbnail, cur, prev = NULL;

    BDBG_MSG(("create_thumbnail for %s at %d", stream->create_settings.name, time));
    BDBG_OBJECT_ASSERT(stream, bthumbnail_stream);

    thumbnail = BKNI_Malloc(sizeof(*thumbnail));
    if (thumbnail == 0) {
        return NULL;
    }
    BKNI_Memset(thumbnail, 0, sizeof(*thumbnail));
    BDBG_OBJECT_SET(thumbnail, bthumbnail);

    thumbnail->data.stream = stream;
    thumbnail->data.time = time;

    /* must insert in ascending order by time */
    for (cur = BLST_D_FIRST(&stream->list); cur; cur = BLST_D_NEXT(cur, link)) {
        if (cur->data.time > time)
            break;
        prev = cur;
    }
    if (prev) {
        BLST_D_INSERT_AFTER(&stream->list, prev, thumbnail, link);
    }
    else {
        BLST_D_INSERT_HEAD(&stream->list, thumbnail, link);
    }

#if BDBG_DEBUG_BUILD
    /* verify correct order */
    prev = NULL;
    for (cur = BLST_D_FIRST(&stream->list); cur; cur = BLST_D_NEXT(cur, link)) {
        if (prev) BDBG_ASSERT(cur->data.time > prev->data.time);
        prev = cur;
    }
#endif

    return thumbnail;
}

void bthumbnail_p_destroy( bthumbnail_t thumbnail )
{
    BDBG_OBJECT_ASSERT(thumbnail, bthumbnail);

    if (thumbnail->manager) {
        /* if it was deleted while decoding, it was put into the dangling list. */
        BLST_D_REMOVE(&thumbnail->manager->dangling, thumbnail, link);
        if (thumbnail->data.surface) {
            bthumbnail_manager_p_delete_surface(thumbnail->manager, thumbnail);
        }
    }
    else {
        BLST_D_REMOVE(&thumbnail->data.stream->list, thumbnail, link);

        if (thumbnail->data.surface) {
            bthumbnail_manager_p_delete_surface(thumbnail->data.stream->manager, thumbnail);
        }
    }

    BDBG_OBJECT_UNSET(thumbnail, bthumbnail);
    BKNI_Free(thumbnail);
}
