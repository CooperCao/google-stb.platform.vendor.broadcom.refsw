/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#include "bstd.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "nexus_base_types.h"
#include "blst_list.h"
#include "ffmpeg_buffers.h"
#include "bkni.h"

BDBG_MODULE(ffmpeg_buffers);

struct b_avcodec_buffer {
    BLST_D_ENTRY(b_avcodec_buffer) link;
    struct b_avcodec_buffer_queue *parent;
    b_video_softdecode_memory_token memory_token;
    AVPicture picture;
    enum AVPixelFormat format;
    int width, height;
    int total_size;
};

static int b_avcodec_align(int v, unsigned alignment)
{
    unsigned r;
    r = v + (alignment - 1);
    r -= r%alignment;
    return r;
}

static void b_avcodec_release_buffer(void *opaque, uint8_t *data)
{
    struct b_avcodec_buffer *buffer = opaque;
    struct b_avcodec_buffer_queue *state = buffer->parent;
    BSTD_UNUSED(data);

    BDBG_MSG(("release %p(%d)", buffer->memory_token.ptr, buffer->total_size));

    BLST_D_REMOVE(&state->used, buffer, link);
    BLST_D_INSERT_HEAD(&state->cache, buffer, link);
    return;
}

static void b_avcodec_destroy_buffer(struct b_avcodec_buffer *buffer)
{
    b_avcodec_buffer_queue *state = buffer->parent;

    state->memory_methods->free(state->parent, &buffer->memory_token);
    BKNI_Free(buffer);
    return;
}

static void b_avcodec_free_buffer_cache( b_avcodec_buffer_queue *state)
{
    struct b_avcodec_buffer *buffer;

    while( NULL != (buffer=BLST_D_FIRST(&state->cache))) {
        BLST_D_REMOVE_HEAD(&state->cache, link);
        b_avcodec_destroy_buffer(buffer);
    }
    return;
}

static struct b_avcodec_buffer *b_avcodec_allocate_new_buffer(b_avcodec_buffer_queue *state, const AVFrame *frame)
{
    struct b_avcodec_buffer *buffer;
    int width, height;
    int stride_align[AV_NUM_DATA_POINTERS];
    int total_size;
    NEXUS_Error rc;

    buffer = BKNI_Malloc(sizeof(*buffer));
    if(buffer==NULL) {
        (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_buffer_alloc;
    }
    BKNI_Memset(buffer, 0, sizeof(*buffer));
    buffer->parent = state;
    buffer->width = frame->width;
    buffer->height = frame->height;
    buffer->format = frame->format;

    width = b_avcodec_align(frame->width, 64);
    height = b_avcodec_align(frame->height, 16);

    avcodec_align_dimensions2(state->context, &width, &height, stride_align);
    av_image_fill_linesizes(buffer->picture.linesize, buffer->format, width);
    total_size = av_image_fill_pointers(buffer->picture.data, buffer->format, height, NULL, buffer->picture.linesize);

    rc = state->memory_methods->alloc(state->parent, total_size, &buffer->memory_token);
    if(rc!=NEXUS_SUCCESS) {
        (void)BERR_TRACE(rc);
        goto err_data_alloc;
    }
    av_image_fill_pointers(buffer->picture.data, buffer->format, height, buffer->memory_token.ptr, buffer->picture.linesize);
    buffer->total_size = total_size;
    BDBG_MSG(("allocate %p(%d)", buffer->memory_token.ptr, buffer->total_size));
    return buffer;

err_data_alloc:
    BKNI_Free(buffer);
err_buffer_alloc:
    return NULL;
}

static int b_avcodec_get_buffer2(AVCodecContext *avctx, AVFrame *frame, int flags)
{
    struct b_avcodec_buffer *buffer;
    b_avcodec_buffer_queue *state = avctx->opaque;

    BSTD_UNUSED(flags);

    buffer = BLST_D_FIRST(&state->cache);
    if(buffer) {
        if(buffer->width == frame->width && buffer->height == frame->height && buffer->format == frame->format) {
            BDBG_MSG(("reuse %p(%d)", buffer->memory_token.ptr, buffer->total_size));
            BLST_D_REMOVE_HEAD(&state->cache, link);
        } else {
            b_avcodec_free_buffer_cache(state);
            buffer = NULL;
        }
    }
    if(buffer==NULL) {
        buffer = b_avcodec_allocate_new_buffer(state, frame);
        if(buffer==NULL) {
            return -1;
        }
    }

    frame->buf[0] = av_buffer_create(buffer->memory_token.ptr, buffer->total_size, b_avcodec_release_buffer, buffer, 0);
    if(frame->buf[0]==NULL) {
        b_avcodec_destroy_buffer(buffer);
        return -1;
    }
    BKNI_Memcpy(frame->data, buffer->picture.data, sizeof(frame->data));
    BKNI_Memcpy(frame->linesize, buffer->picture.linesize,  sizeof(frame->linesize));
    frame->extended_data = frame->data;
    BLST_D_INSERT_HEAD(&state->used, buffer, link);

    BDBG_MSG(("get_buffer %dx%d -> %d %p %p %p %u %u %u", frame->width, frame->height, buffer->total_size, frame->data[0], frame->data[1], frame->data[2], frame->linesize[0], frame->linesize[1], frame->linesize[2]));

    return 0;
}

void b_avcodec_buffer_queue_init(b_avcodec_buffer_queue *state, const b_video_sofdecode_memory_methods *memory_methods, void *parent)
{
    BKNI_Memset(state, 0, sizeof(*state));
    state->memory_methods = memory_methods;
    state->parent = parent;
    return;
}


void b_avcodec_buffer_queue_connect(b_avcodec_buffer_queue *state, AVCodecContext *context)
{
    BLST_D_INIT(&state->cache);
    BLST_D_INIT(&state->used);
    state->context = context;
    state->context->opaque = state;
    state->context->get_buffer2 = b_avcodec_get_buffer2;
    return;
}

void b_avcodec_buffer_queue_disconnect(b_avcodec_buffer_queue *state)
{
    b_avcodec_free_buffer_cache(state);
    return;
}

const b_video_softdecode_memory_token *
b_avcodec_buffer_queue_find(b_avcodec_buffer_queue *state, const void *buf)
{
    struct b_avcodec_buffer *buffer;

    for(buffer=BLST_D_FIRST(&state->used);buffer;buffer=BLST_D_NEXT(buffer, link)) {
        if( (const uint8_t *)buf >= (const uint8_t *)buffer->memory_token.ptr && (const uint8_t *)buf  < (const uint8_t *)buffer->memory_token.ptr + buffer->total_size) {
            break;
        }
    }
    return &buffer->memory_token;
}
