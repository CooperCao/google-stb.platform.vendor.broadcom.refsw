/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2017 Broadcom. All rights reserved.
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
 **************************************************************************/
#include "bstd.h"
#include "nexus_video_decoder_soft.h"
#include "nexus_video_decoder_soft_vpx.h"
#include "vpx/vp8dx.h"
#include "vpx/vpx_decoder.h"
#include "blst_list.h"
#include "bkni.h"

BDBG_MODULE(bsoft_vpx_decode);

struct b_libvpx_buffer {
    BLST_D_ENTRY(b_libvpx_buffer) link;
    b_video_softdecode_memory_token memory_token;
    size_t size;
    bool used;
};
BLST_D_HEAD(b_libvpx_buffer_list, b_libvpx_buffer);

typedef struct b_video_vpx_softdecode {
    b_video_softdecode parent; /* must be first member */
    vpx_codec_ctx_t codec;
    vpx_codec_iter_t iter;
    const b_video_sofdecode_memory_methods *memory_methods;
    void *decode_parent;
    unsigned picture_count;
    struct {
        struct b_libvpx_buffer_list cache;
        struct b_libvpx_buffer_list used;
        unsigned allocated;
    } allocator;
} b_video_vpx_softdecode;

static void b_vpx_frame_buffer_destroy(b_video_vpx_softdecode *decode, struct b_libvpx_buffer *buffer)
{
    decode->memory_methods->free(decode->decode_parent, &buffer->memory_token);
    BKNI_Free(buffer);
    return;
}

static void b_vpx_frame_buffer_free(b_video_vpx_softdecode *decode, struct b_libvpx_buffer *buffer)
{
    BLST_D_REMOVE(&decode->allocator.cache, buffer, link);
    b_vpx_frame_buffer_destroy(decode, buffer);
    return;
}

static struct b_libvpx_buffer *b_vpx_frame_buffer_new(b_video_vpx_softdecode *decode, size_t min_size)
{
    struct b_libvpx_buffer *buffer;
    NEXUS_Error rc;

    buffer = BKNI_Malloc(sizeof(*buffer));
    if(buffer==NULL) {
        (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_buffer_alloc;
    }
    BKNI_Memset(buffer, 0, sizeof(*buffer));
    buffer->size = min_size;
    rc = decode->memory_methods->alloc(decode->decode_parent, min_size, &buffer->memory_token);
    if(rc!=NEXUS_SUCCESS) {
        (void)BERR_TRACE(rc);
        goto err_data_alloc;
    }
    BDBG_MSG(("allocate %p(%u)", (void *)buffer->memory_token.ptr, (unsigned)buffer->size));
    return buffer;

err_data_alloc:
    BKNI_Free(buffer);
err_buffer_alloc:
    return NULL;
}

static int b_vpx_frame_buffer_get(void *cb_priv, size_t min_size, vpx_codec_frame_buffer_t *fb)
{
    struct b_libvpx_buffer *buffer;
    b_video_vpx_softdecode *decoder = cb_priv;

    buffer = BLST_D_FIRST(&decoder->allocator.cache);
    if(buffer) {
        if(buffer->size >= min_size) {
            BDBG_MSG(("reuse %p(%u)", buffer->memory_token.ptr, (unsigned)buffer->size));
            BLST_D_REMOVE_HEAD(&decoder->allocator.cache, link);
        } else {
            BDBG_LOG(("resize %p(%u:%u)", buffer->memory_token.ptr, (unsigned)buffer->size, (unsigned)min_size));
            b_vpx_frame_buffer_free(decoder, buffer);
            buffer = NULL;
        }
    }
    if(buffer==NULL) {
        buffer = b_vpx_frame_buffer_new(decoder, min_size);
        if(buffer==NULL) {
            BDBG_ERR(("Can't allocate new buffer (%u)", min_size));
            return -1;
        }
    }
    buffer->used = true;
    BLST_D_INSERT_HEAD(&decoder->allocator.used, buffer, link);
    decoder->allocator.allocated++;
    fb->data = buffer->memory_token.ptr;
    fb->size = buffer->size;
    fb->priv = buffer;
    return 0;
}

static int b_vpx_frame_buffer_release(void *cb_priv, vpx_codec_frame_buffer_t *fb)
{
    b_video_vpx_softdecode *decoder = cb_priv;
    struct b_libvpx_buffer *buffer = fb->priv;

    if(buffer->used) {
        buffer->used = false;
        decoder->allocator.allocated--;
        BLST_D_REMOVE(&decoder->allocator.used, buffer, link);
        BLST_D_INSERT_HEAD(&decoder->allocator.cache, buffer, link);
    } else {
        BDBG_WRN(("%p: releasing unused buffer %p(%u)", (void *)decoder, (void *)buffer, (unsigned)buffer->size));
    }
    return 0;
}

static void b_vpx_frame_allocator_destroy(b_video_vpx_softdecode *decode)
{
    struct b_libvpx_buffer *buffer;

    while( NULL != (buffer=BLST_D_FIRST(&decode->allocator.cache))) {
        BLST_D_REMOVE_HEAD(&decode->allocator.cache, link);
        b_vpx_frame_buffer_destroy(decode, buffer);
    }
    return;
}



static NEXUS_Error
b_vpx_video_start(b_video_softdecode_t d, NEXUS_VideoCodec codec)
{
    b_video_vpx_softdecode *decode = (void*)d;
    const vpx_codec_iface_t *decoder = vpx_codec_vp9_dx();
    BDBG_MSG(("start"));
    BSTD_UNUSED(codec);
    decode->iter = NULL;
    decode->picture_count = 0;
    if (vpx_codec_dec_init(&decode->codec, decoder, NULL, 0)) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (vpx_codec_set_frame_buffer_functions(&decode->codec, b_vpx_frame_buffer_get, b_vpx_frame_buffer_release,decode)) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    return NEXUS_SUCCESS;
}

static void
b_vpx_video_stop(b_video_softdecode_t d)
{
    b_video_vpx_softdecode *decode = (void*)d;
    BDBG_MSG(("stop"));
    vpx_codec_destroy(&decode->codec);
    b_vpx_frame_allocator_destroy(decode);
    return;
}

static void
b_vpx_video_destroy(b_video_softdecode_t d)
{
    b_video_vpx_softdecode *decode = (void*)d;
    BDBG_MSG(("destroy"));
    BKNI_Free(decode);
}


static NEXUS_Error
b_vpx_video_decode(b_video_softdecode_t d, const b_avcodec_compressed_frame *compressed, b_avcodec_frame *frame)
{
    b_video_vpx_softdecode *decode = (void*)d;
    vpx_image_t *image = NULL;
#if 1
    /* do real decode */
    if(compressed->buf) {
        if (vpx_codec_decode(&decode->codec, compressed->buf, compressed->size, (void *)compressed, sizeof(*compressed))) {
            BDBG_ERR(("vpx_error:'%s'", vpx_codec_error_detail(&decode->codec)));
            return BERR_TRACE(NEXUS_UNKNOWN);
        }
        decode->iter = NULL;
    }
    image = vpx_codec_get_frame(&decode->codec, &decode->iter);
    if(image==NULL) {
        decode->iter = NULL;
        return /* BERR_TRACE */ (NEXUS_NOT_AVAILABLE);
    }
#else
    /* simulate decoding */
    {
        static struct b_libvpx_buffer * buffer = NULL;
        static vpx_image_t _image;

        image = &_image;
        BKNI_Memset(image, 0, sizeof(*image));
        image->d_w = 1280;
        image->d_h = 720;

        if(buffer==NULL) {
            buffer = b_vpx_frame_buffer_new(decode, 2 * image->d_w * image->d_h);
        }
        image->planes[0] = buffer->memory_token.ptr;
        image->stride[0] = image->d_w;
        image->planes[1] = buffer->memory_token.ptr;
        image->stride[1] = image->d_w/2;
        image->planes[2] = buffer->memory_token.ptr;
        image->stride[2] = image->d_w/2;
        image->user_priv = frame;
    }
#endif
    BKNI_Memset(frame, 0, sizeof(*frame));
    frame->width = image->d_w;
    frame->height = image->d_h;
    frame->y.buf = image->planes[0];
    frame->y.stride = image->stride[0];
    frame->u.buf = image->planes[1];
    frame->u.stride = image->stride[1];
    frame->v.buf = image->planes[2];
    frame->v.stride = image->stride[2];
    frame->picture_count = decode->picture_count;
    decode->picture_count ++;
    if(image->user_priv) {
        const b_avcodec_compressed_frame *frame_compressed = image->user_priv;
        frame->pts_valid = frame_compressed->pts_valid;
        frame->pts = frame_compressed->pts;
    }
    BDBG_MSG(("decode: [%u] %uX%u", frame->picture_count, frame->width, frame->height));
    return NEXUS_SUCCESS;
}

static const b_video_softdecode_methods b_vpx_video_methods = {
    b_vpx_video_start,
    b_vpx_video_stop,
    b_vpx_video_destroy,
    b_vpx_video_decode
};

b_video_softdecode_t
bvpx_video_decode_create(const b_video_sofdecode_memory_methods *memory_methods, void *parent)
{
    b_video_vpx_softdecode *decode;

    BDBG_MSG(("create"));

    decode = BKNI_Malloc(sizeof(*decode));
    if(!decode) {
        return NULL;
    }
    BKNI_Memset(decode, 0, sizeof(*decode));
    BLST_D_INIT(&decode->allocator.cache);
    BLST_D_INIT(&decode->allocator.used);
    decode->allocator.allocated = 0;
    decode->decode_parent = parent;
    decode->memory_methods = memory_methods;
    decode->parent.methods = &b_vpx_video_methods;
    return &decode->parent;
}
