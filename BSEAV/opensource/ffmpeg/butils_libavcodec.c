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
#include "butils_libavcodec.h"
#include <stdio.h>

BDBG_MODULE(butils_libavcodec);

static void
ffmpeg_copyyuv(AVCodecContext *context, AVFrame *frame, NEXUS_SurfaceHandle surface)
{
    unsigned x,y;
    NEXUS_SurfaceMemory mem;
    unsigned width = context->coded_width;
    unsigned height = context->coded_height;
#define MAKE_YUV(y0,y1,u,v) ((((uint32_t)u)<<24) | (((uint32_t)y1)<<16) | (((uint32_t)v)<<8) | (((uint32_t)y0)))

    NEXUS_Surface_GetMemory(surface, &mem);
    /* BKNI_Memset(mem.buffer, 0, mem.pitch*480*4); */
#if 0
    {
        const uint8_t *u = frame->data[1];
        BDBG_ERR(("00: %02x %02x %02x %02x %02x %02x %02x %02x | %02x %02x %02x %02x %02x %02x %02x %02x", u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7], u[8], u[9], u[10], u[11], u[12], u[13], u[14], u[15]));
        u+=0x10;
        BDBG_ERR(("10: %02x %02x %02x %02x %02x %02x %02x %02x | %02x %02x %02x %02x %02x %02x %02x %02x", u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7], u[8], u[9], u[10], u[11], u[12], u[13], u[14], u[15]));
        u+=0x10;
        BDBG_ERR(("20: %02x %02x %02x %02x %02x %02x %02x %02x | %02x %02x %02x %02x %02x %02x %02x %02x", u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7], u[8], u[9], u[10], u[11], u[12], u[13], u[14], u[15]));
    }
#endif
    for(y=0;y<height;y+=2) {
        const uint8_t *frame_y_0 = (uint8_t*)frame->data[0] + y*frame->linesize[0];
        const uint8_t *frame_y_1 = frame_y_0 + frame->linesize[0];
        const uint8_t *frame_u = (uint8_t*)frame->data[1] + (y/2)*frame->linesize[1];
        const uint8_t *frame_v = (uint8_t*)frame->data[2] + (y/2)*frame->linesize[2];
        uint8_t *dst_0 = (uint8_t *)mem.buffer + mem.pitch*y;
        uint8_t *dst_1 = dst_0 + mem.pitch;
        for(x=0;x<width;x+=2) {
            dst_0[0] = frame_y_0[0];
            dst_1[0] = frame_y_1[0];
            dst_0[1] = dst_1[1] = frame_u[0];
            dst_0[2] = frame_y_0[1];
            dst_1[2] = frame_y_1[1];
            dst_0[3] = dst_1[3] = frame_v[0];

            dst_0+=4;
            dst_1+=4;
            frame_y_0+=2;
            frame_y_1+=2;
            frame_u+=1;
            frame_v+=1;
        }
    }
    /* NEXUS_Surface_Flush(surface); */
    /* getchar(); */

    return;
}



NEXUS_Error
butils_libavcodec_decode(butils_libavcodec *state, const uint8_t *buf, size_t frame_len, struct bsoft_decoder_frame *frame)
{
    int bytes_decoded;
    int frame_completed;

    bytes_decoded = avcodec_decode_video(state->context, state->frame, &frame_completed, buf, frame_len);
    if(bytes_decoded!=(int)frame_len) { return BERR_TRACE(NEXUS_NOT_SUPPORTED); }
    if(state->frame->data[0]==NULL) { return BERR_TRACE(NEXUS_NOT_SUPPORTED); }

    if(state->frame->qscale_table) {
        const int8_t *table = state->frame->qscale_table;
        unsigned qp = 0;
        unsigned mbheight = (state->context->coded_height + 15)/16;
        unsigned mbwidth = (state->context->coded_width + 15)/16;
        unsigned count = mbheight * mbwidth;
        unsigned x,y;
        for(y=0; y<mbwidth; y++) {
            for(x=0;x<mbheight;x++) {
                qp += ((unsigned)table[x])&0x3f;
            }
            table += state->frame->qstride;
        }
        frame->qscale = (qp+count/2)/count;
        BDBG_MSG(("qp %u(%u) %u:%u:%u", qp, frame->qscale, count, mbheight, state->frame->qstride));
    }
#if 0
    {
        static FILE *fout=NULL;

        if(!fout) {
            fout=fopen("soft.dat","wb");
        }
        if(fout) {
            uint32_t size=frame_len;
            fwrite(&size, sizeof(size), 1, fout);
            fwrite(buf, frame_len, 1, fout);
            fflush(fout);
        }
    }
#endif

    if((int)frame->width != state->context->coded_width || (int)frame->height != state->context->coded_height) {
        BDBG_MSG(("resolution %dx%d", state->context->coded_width, state->context->coded_height));
        if(bsoft_decoder_frame_alloc(frame, state->context->coded_width, state->context->coded_height)!=0) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
    }
    ffmpeg_copyyuv(state->context, state->frame, frame->surface);
    return NEXUS_SUCCESS;
}


NEXUS_Error
butils_libavcodec_init(butils_libavcodec *state, AVCodec *codec, AVCodecContext *context)
{
    state->codec = codec;
    if(context==NULL) {
        context = avcodec_alloc_context();
        if(!context) {return BERR_TRACE(NEXUS_NOT_SUPPORTED);}
    }
    state->context = context;
    if(state->codec->capabilities & CODEC_CAP_TRUNCATED) {
        state->context->flags|=CODEC_FLAG_TRUNCATED;
    }
    if(avcodec_open(state->context, state->codec)<0) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    state->frame=avcodec_alloc_frame();
    if(!state->frame) { return BERR_TRACE(NEXUS_NOT_SUPPORTED); }

    return NEXUS_SUCCESS;
}

void
butils_libavcodec_shutdown(butils_libavcodec *state)
{
    avcodec_close(state->context);
    av_free(state->context);
    av_free(state->frame);
    return;
}
