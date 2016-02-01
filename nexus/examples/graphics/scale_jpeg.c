/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *****************************************************************************/
/* Nexus example app: show jpeg image */
#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_GRAPHICS2D && NEXUS_HAS_DISPLAY && NEXUS_HAS_PICTURE_DECODER
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_core_utils.h"
#include "nexus_picture_decoder.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <unistd.h>
#if 0
#include "btrc.h"
BTRC_MODULE(frame,ENABLE);
BTRC_MODULE(decode,ENABLE);
BTRC_MODULE(scale,ENABLE);
#else
#define BTRC_TRACE(x,y)
#define BTRC_Module_Report(x)
#define BTRC_Module_Reset(x)
#endif

struct app_context {
    NEXUS_Graphics2DHandle gfx;
    NEXUS_PictureDecoderHandle pictureDecoder;
    NEXUS_PictureDecoderStatus pictureStatus;
    BKNI_EventHandle checkpointEvent;
    BKNI_EventHandle decoderStep;
    BKNI_EventHandle bufferFlip;
};

static void buffer_flip_complete(void *context_, int unused)
{
    const struct app_context *context = context_;
    unused = unused;
    BKNI_SetEvent(context->bufferFlip);
    return;
}

static void picture_decoder_step(void *context_, int unused)
{
    const struct app_context *context = context_;
    unused = unused;
    BKNI_SetEvent(context->decoderStep);
    return;
}

static void checkpoint_callback(void *context_, int unused)
{
    const struct app_context *context = context_;
    unused = unused;
    BKNI_SetEvent(context->checkpointEvent);
    return;
}

static void checkpoint(const struct app_context *context)
{
    NEXUS_Error rc;
    rc = NEXUS_Graphics2D_Checkpoint(context->gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(context->checkpointEvent, BKNI_INFINITE);
    }
    BDBG_ASSERT(!rc);
    return;
}

int simple_scaler(const struct app_context *context, NEXUS_SurfaceHandle framebuffer, const NEXUS_Rect *dest)
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceHandle picture;
    NEXUS_PictureDecoderStatus pictureStatus;
    NEXUS_Graphics2DBlitSettings blitSettings;

    /* create picture that could handle complete picture */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = context->pictureStatus.header.format;
    createSettings.width = context->pictureStatus.header.surface.width;
    createSettings.height = context->pictureStatus.header.surface.height;
    picture = NEXUS_Surface_Create(&createSettings);

    /* start decoding */
    BTRC_TRACE(decode, START);
    NEXUS_PictureDecoder_DecodeSegment(context->pictureDecoder, picture, NULL);
    BKNI_WaitForEvent(context->decoderStep, BKNI_INFINITE);
    BTRC_TRACE(decode, STOP);
    NEXUS_PictureDecoder_GetStatus(context->pictureDecoder, &pictureStatus);
    if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
        fprintf(stderr, "decoding failed: line %d\n", __LINE__);
        NEXUS_Surface_Destroy(picture);
        return 1;
    }
    BDBG_ASSERT(pictureStatus.state==NEXUS_PictureDecoderState_eSegmentDone);


    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = picture;
    blitSettings.source.rect.x = 0;
    blitSettings.source.rect.y = 0;
    blitSettings.source.rect.width = pictureStatus.header.width;
    if(NEXUS_PIXEL_FORMAT_IS_YCRCB(pictureStatus.header.format)) {
        blitSettings.source.rect.width += blitSettings.source.rect.width%2; /* YCrCb single pixel has width of 2 */
    }

    blitSettings.source.rect.height = pictureStatus.header.height;
    blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

    blitSettings.output.surface = framebuffer;
    blitSettings.output.rect = *dest;

    BTRC_TRACE(scale, START);
    NEXUS_Graphics2D_Blit(context->gfx, &blitSettings);
    checkpoint(context);
    BTRC_TRACE(scale, STOP);
    NEXUS_Surface_Destroy(picture);
    return 0;
}

int row_scaler(const struct app_context *context, NEXUS_SurfaceHandle framebuffer, const NEXUS_Rect *dest)
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceHandle row;
    NEXUS_PictureDecoderStatus pictureStatus;
    NEXUS_Graphics2DBlitSettings blitSettings;
    const unsigned block = 128;
    unsigned y;
    unsigned dst_y;

    /* create picture that could handle complete picture */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = context->pictureStatus.header.format;
    createSettings.width = context->pictureStatus.header.surface.width;
    createSettings.height = block;
    row = NEXUS_Surface_Create(&createSettings);
    for(dst_y=0,y=0; y<context->pictureStatus.header.height;y+=block)  {
        NEXUS_Rect rect;
        rect.x = 0;
        rect.width = context->pictureStatus.header.surface.width;
        rect.y = y;
        if(rect.y + block <= context->pictureStatus.header.surface.height) {
            rect.height = block;
        } else {
            rect.height = context->pictureStatus.header.surface.height - rect.y;
        }
        /* start decoding */
        BTRC_TRACE(decode, START);
        NEXUS_PictureDecoder_DecodeSegment(context->pictureDecoder, row, &rect);
        BKNI_WaitForEvent(context->decoderStep, BKNI_INFINITE);
        BTRC_TRACE(decode, STOP);
        NEXUS_PictureDecoder_GetStatus(context->pictureDecoder, &pictureStatus);
        if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            fprintf(stderr, "decoding failed: line %d\n", __LINE__);
            NEXUS_Surface_Destroy(row);
            return 1;
        }
        BDBG_ASSERT(pictureStatus.state==NEXUS_PictureDecoderState_eSegmentDone);

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = row;
        blitSettings.source.rect.x = 0;
        blitSettings.source.rect.y = 0;
        blitSettings.source.rect.width = context->pictureStatus.header.width;
        if(NEXUS_PIXEL_FORMAT_IS_YCRCB(context->pictureStatus.header.format)) {
            blitSettings.source.rect.width += blitSettings.source.rect.width%2; /* YCrCb single pixel has width of 2 */
        }

        if(rect.y + block <= context->pictureStatus.header.surface.height) {
            blitSettings.source.rect.height = block;
        } else {
            blitSettings.source.rect.height = context->pictureStatus.header.height - rect.y;
        }
        blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

        blitSettings.output.surface = framebuffer;
        blitSettings.output.rect.x = dest->x;
        blitSettings.output.rect.width = dest->width;
        blitSettings.output.rect.y = dest->y + dst_y;
        blitSettings.output.rect.height = ((rect.y+blitSettings.source.rect.height)*dest->height)/context->pictureStatus.header.height - dst_y;
        /* printf("scale %.3f(%.3f)\n", rect.height/(float)blitSettings.output.rect.height, context->pictureStatus.header.surface.height/(float)dest->height); */

        dst_y += blitSettings.output.rect.height;

        BTRC_TRACE(scale, START);
        NEXUS_Graphics2D_Blit(context->gfx, &blitSettings);
        checkpoint(context);
        BTRC_TRACE(scale, STOP);
    }
    NEXUS_Surface_Destroy(row);
    return 0;
}

int row_scaler_overlay_bottom(const struct app_context *context, NEXUS_SurfaceHandle framebuffer, const NEXUS_Rect *dest)
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceHandle row_next;
    NEXUS_SurfaceHandle row;
    NEXUS_PictureDecoderStatus pictureStatus;
    NEXUS_Graphics2DBlitSettings blitSettings;
    const unsigned block = 128;
    const unsigned overlay = 16;
    unsigned y;
    unsigned dst_y;
    NEXUS_SurfaceMemory mem;

    /* create picture that could handle complete picture */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = context->pictureStatus.header.format;
    createSettings.width = context->pictureStatus.header.surface.width;
    createSettings.height = block;
    row = NEXUS_Surface_Create(&createSettings);
    NEXUS_Surface_GetMemory(row, &mem);

    createSettings.height = block;
    createSettings.pMemory = (uint8_t *)mem.buffer + mem.pitch*overlay;
    row_next = NEXUS_Surface_Create(&createSettings);

    for(dst_y=0,y=0; y<context->pictureStatus.header.height;y+=block-overlay)  {
        NEXUS_Rect rect;
        NEXUS_SurfaceHandle pic_dest;
        bool last = false;
        rect.x = 0;
        rect.width = context->pictureStatus.header.surface.width;
        if(y==0) {
            rect.y = 0;
            rect.height = block;
            pic_dest = row;
        } else {
            rect.y = y+overlay;
            pic_dest = row_next;
            rect.height = block-overlay;
        }
        if(rect.y + rect.height > context->pictureStatus.header.surface.height) {
            if(rect.y >= context->pictureStatus.header.surface.height) {
                break;
            }
            rect.height = context->pictureStatus.header.surface.height - rect.y;
            last = true;
        }
        /* start decoding */
        BTRC_TRACE(decode, START);
        NEXUS_PictureDecoder_DecodeSegment(context->pictureDecoder, pic_dest, &rect);
        BKNI_WaitForEvent(context->decoderStep, BKNI_INFINITE);
        BTRC_TRACE(decode, STOP);
        NEXUS_PictureDecoder_GetStatus(context->pictureDecoder, &pictureStatus);
        if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            fprintf(stderr, "decoding failed: line %d\n", __LINE__);
            NEXUS_Surface_Destroy(row);
            NEXUS_Surface_Destroy(row_next);
            return 1;
        }
        BDBG_ASSERT(pictureStatus.state==NEXUS_PictureDecoderState_eSegmentDone);

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = row;
        blitSettings.source.rect.x = 0;
        blitSettings.source.rect.y = 0;
        blitSettings.source.rect.width = context->pictureStatus.header.width;
        if(NEXUS_PIXEL_FORMAT_IS_YCRCB(context->pictureStatus.header.format)) {
            blitSettings.source.rect.width += blitSettings.source.rect.width%2; /* YCrCb single pixel has width of 2 */
        }
        if(y+block <= context->pictureStatus.header.height) {
            blitSettings.source.rect.height = block;
        } else {
            blitSettings.source.rect.height = context->pictureStatus.header.height - y;
        }
        blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

        blitSettings.output.surface = framebuffer;
        blitSettings.output.rect.x = dest->x;
        blitSettings.output.rect.width = dest->width;
        blitSettings.output.rect.y = dest->y + dst_y;
        blitSettings.output.rect.height = ((y+blitSettings.source.rect.height)*dest->height)/context->pictureStatus.header.height - dst_y;
        /* printf("scale %.3f(%.3f)\n", rect.height/(float)blitSettings.output.rect.height, context->pictureStatus.header.surface.height/(float)dest->height); */

        dst_y = ((y+(block-overlay))*dest->height)/context->pictureStatus.header.height;

        BTRC_TRACE(scale, START);
        NEXUS_Graphics2D_Blit(context->gfx, &blitSettings);
        if(!last) {
            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = row;
            blitSettings.source.rect.x = 0;
            blitSettings.source.rect.width = context->pictureStatus.header.width;
            if(NEXUS_PIXEL_FORMAT_IS_YCRCB(context->pictureStatus.header.format)) {
                blitSettings.source.rect.width += blitSettings.source.rect.width%2; /* YCrCb single pixel has width of 2 */
            }
            blitSettings.source.rect.y = block-overlay;
            blitSettings.source.rect.height = overlay;
            blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
            blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

            blitSettings.output.surface = row;
            blitSettings.output.rect.x = 0;
            blitSettings.output.rect.width = context->pictureStatus.header.width;
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.height = overlay;
            NEXUS_Graphics2D_Blit(context->gfx, &blitSettings);
        }

        checkpoint(context);
        BTRC_TRACE(scale, STOP);
    }
    NEXUS_Surface_Destroy(row);
    NEXUS_Surface_Destroy(row_next);
    return 0;
}

int row_scaler_overlay_full_uniform(const struct app_context *context, NEXUS_SurfaceHandle framebuffer, const NEXUS_Rect *dest)
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceHandle row_next;
    NEXUS_SurfaceHandle row;
    NEXUS_SurfaceHandle screen;
    NEXUS_PictureDecoderStatus pictureStatus;
    NEXUS_Graphics2DBlitSettings blitSettings;
    const unsigned block = 128;
    const unsigned overlay = 16;
    unsigned y;
    unsigned dst_y;
    unsigned dst_step;
    unsigned screen_step;

    NEXUS_SurfaceMemory mem;

    /* create picture that could handle complete picture */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = context->pictureStatus.header.format;
    createSettings.width = context->pictureStatus.header.surface.width;
    createSettings.height = block;
    row = NEXUS_Surface_Create(&createSettings);

    NEXUS_Surface_GetMemory(row, &mem);
    createSettings.height = block;
    createSettings.pMemory = (uint8_t *)mem.buffer + mem.pitch*2*overlay;
    row_next = NEXUS_Surface_Create(&createSettings);

    dst_step = ((block-2*overlay)*dest->height)/context->pictureStatus.header.height;
    screen_step = ((block*dest->height)/context->pictureStatus.header.height);
    NEXUS_Surface_GetCreateSettings(framebuffer, &createSettings);
    createSettings.height = screen_step;
    createSettings.width = dest->width;
    screen = NEXUS_Surface_Create(&createSettings);

    for(dst_y=0,y=0; y<context->pictureStatus.header.height;y+=block-2*overlay)  {
        NEXUS_Rect rect;
        NEXUS_SurfaceHandle pic_dest;
        bool last = false;
        rect.x = 0;
        rect.width = context->pictureStatus.header.surface.width;
        if(y==0) {
            rect.y = 0;
            rect.height = block;
            pic_dest = row;
        } else {
            rect.y = y+2*overlay;
            pic_dest = row_next;
            rect.height = block-2*overlay;
        }
        if(rect.y + rect.height > context->pictureStatus.header.surface.height) {
            if(rect.y >= context->pictureStatus.header.surface.height) {
                break;
            }
            rect.height = context->pictureStatus.header.surface.height - rect.y;
            last = true;
        }
        /* start decoding */
        BTRC_TRACE(decode, START);
        NEXUS_PictureDecoder_DecodeSegment(context->pictureDecoder, pic_dest, &rect);
        BKNI_WaitForEvent(context->decoderStep, BKNI_INFINITE);
        BTRC_TRACE(decode, STOP);
        NEXUS_PictureDecoder_GetStatus(context->pictureDecoder, &pictureStatus);
        if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            fprintf(stderr, "decoding failed: line %d\n", __LINE__);
            NEXUS_Surface_Destroy(row);
            NEXUS_Surface_Destroy(row_next);
            NEXUS_Surface_Destroy(screen);
            return 1;
        }
        BDBG_ASSERT(pictureStatus.state==NEXUS_PictureDecoderState_eSegmentDone);

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = row;
        blitSettings.source.rect.x = 0;
        blitSettings.source.rect.y = 0;
        blitSettings.source.rect.width = context->pictureStatus.header.width;
        if(NEXUS_PIXEL_FORMAT_IS_YCRCB(context->pictureStatus.header.format)) {
            blitSettings.source.rect.width += blitSettings.source.rect.width%2; /* YCrCb single pixel has width of 2 */
        }
        blitSettings.source.rect.height = block;
        blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

        blitSettings.output.surface = screen;
        blitSettings.output.rect.x = 0;
        blitSettings.output.rect.width = dest->width;
        blitSettings.output.rect.y = 0;
        blitSettings.output.rect.height = screen_step;
        BTRC_TRACE(scale, START);
        NEXUS_Graphics2D_Blit(context->gfx, &blitSettings);

        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = screen;
        blitSettings.source.rect.x = 0;
        blitSettings.source.rect.width = dest->width;
        if(y==0) {
            blitSettings.source.rect.y = 0;
            if(y+block <= context->pictureStatus.header.height) {
                blitSettings.source.rect.height = screen_step;
            } else {
                blitSettings.source.rect.height = (context->pictureStatus.header.height*dest->height)/context->pictureStatus.header.height;
            }
        } else {
            blitSettings.source.rect.y = (overlay*dest->height)/context->pictureStatus.header.height;
            if(y+block <= context->pictureStatus.header.height) {
                blitSettings.source.rect.height = dst_step;
            } else {
                blitSettings.source.rect.height = (((context->pictureStatus.header.height-y)-overlay)*dest->height)/context->pictureStatus.header.height;
            }
        }

        blitSettings.output.surface = framebuffer;
        blitSettings.output.rect.x = dest->x;
        blitSettings.output.rect.width = blitSettings.source.rect.width ;
        blitSettings.output.rect.y = dest->y + dst_y;
        blitSettings.output.rect.height = blitSettings.source.rect.height ;

        dst_y += dst_step;
        if(y==0) {
            dst_y += (overlay*dest->height)/context->pictureStatus.header.height;
        }

        NEXUS_Graphics2D_Blit(context->gfx, &blitSettings);
        if(!last) {
            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
            blitSettings.source.surface = row;
            blitSettings.source.rect.x = 0;
            blitSettings.source.rect.width = context->pictureStatus.header.width;
            if(NEXUS_PIXEL_FORMAT_IS_YCRCB(pictureStatus.header.format)) {
                blitSettings.source.rect.width += blitSettings.source.rect.width%2; /* YCrCb single pixel has width of 2 */
            }
            blitSettings.source.rect.y = block-2*overlay;
            blitSettings.source.rect.height = 2*overlay;
            blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
            blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

            blitSettings.output.surface = row;
            blitSettings.output.rect.x = 0;
            blitSettings.output.rect.width = context->pictureStatus.header.width;
            blitSettings.output.rect.y = 0;
            blitSettings.output.rect.height = 2*overlay;
            NEXUS_Graphics2D_Blit(context->gfx, &blitSettings);
        }

        checkpoint(context);
        BTRC_TRACE(scale, STOP);
        /* getchar(); */
    }
    NEXUS_Surface_Destroy(row);
    NEXUS_Surface_Destroy(row_next);
    NEXUS_Surface_Destroy(screen);
    return 0;
}



int row_scaler_pipe(const struct app_context *context, NEXUS_SurfaceHandle framebuffer, const NEXUS_Rect *dest)
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceHandle row[2];
    NEXUS_PictureDecoderStatus pictureStatus;
    NEXUS_Graphics2DBlitSettings blitSettings;
    const unsigned block = 128;
    unsigned y;
    unsigned dst_y;
    int prev_row,cur_row;

    /* create picture that could handle complete picture */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = context->pictureStatus.header.format;
    createSettings.width = context->pictureStatus.header.surface.width;
    createSettings.height = block;
    row[0] = NEXUS_Surface_Create(&createSettings);
    row[1] = NEXUS_Surface_Create(&createSettings);
    for(prev_row=-1,cur_row=0, dst_y=y=0; y<context->pictureStatus.header.height;y+=block)  {
        NEXUS_Rect rect;
        rect.x = 0;
        rect.width = context->pictureStatus.header.surface.width;
        rect.y = y;
        if(rect.y + block <= context->pictureStatus.header.surface.height) {
            rect.height = block;
        } else {
            rect.height = context->pictureStatus.header.surface.height - rect.y;
        }
        /* start decoding */
        BTRC_TRACE(decode, START);
        NEXUS_PictureDecoder_DecodeSegment(context->pictureDecoder, row[cur_row], &rect);
        BKNI_WaitForEvent(context->decoderStep, BKNI_INFINITE);
        BTRC_TRACE(decode, STOP);
        NEXUS_PictureDecoder_GetStatus(context->pictureDecoder, &pictureStatus);
        if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            fprintf(stderr, "decoding failed: line %d\n", __LINE__);
            NEXUS_Surface_Destroy(row[0]);
            NEXUS_Surface_Destroy(row[1]);
            return 1;
        }
        BDBG_ASSERT(pictureStatus.state==NEXUS_PictureDecoderState_eSegmentDone);
        if(prev_row!=-1) {
            BTRC_TRACE(scale, STOP);
        }
        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = row[cur_row];
        blitSettings.source.rect.x = 0;
        blitSettings.source.rect.y = 0;
        blitSettings.source.rect.width = context->pictureStatus.header.width;
        if(NEXUS_PIXEL_FORMAT_IS_YCRCB(context->pictureStatus.header.format)) {
            blitSettings.source.rect.width += blitSettings.source.rect.width%2; /* YCrCb single pixel has width of 2 */
        }
        if(rect.y + block <= context->pictureStatus.header.surface.height) {
            blitSettings.source.rect.height = block;
        } else {
            blitSettings.source.rect.height = context->pictureStatus.header.height - rect.y;
        }
        blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

        blitSettings.output.surface = framebuffer;
        blitSettings.output.rect.x = dest->x;
        blitSettings.output.rect.width = dest->width;
        blitSettings.output.rect.y = dest->y + dst_y;
        blitSettings.output.rect.height = ((rect.y+blitSettings.source.rect.height)*dest->height)/context->pictureStatus.header.height - dst_y;
        /* printf("scale %.3f(%.3f)\n", rect.height/(float)blitSettings.output.rect.height, context->pictureStatus.header.surface.height/(float)dest->height); */

        dst_y += blitSettings.output.rect.height;
        BTRC_TRACE(scale, START);
        NEXUS_Graphics2D_Blit(context->gfx, &blitSettings);
        checkpoint(context);
        prev_row = cur_row;
        cur_row = (cur_row+1)%2;
        /* getchar(); */
    }
    BTRC_TRACE(scale, STOP);
    NEXUS_Surface_Destroy(row[0]);
    NEXUS_Surface_Destroy(row[1]);
    return 0;
}

int row_scaler_uniform(const struct app_context *context, NEXUS_SurfaceHandle framebuffer, const NEXUS_Rect *dest)
{
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_SurfaceHandle row;
    NEXUS_PictureDecoderStatus pictureStatus;
    NEXUS_Graphics2DBlitSettings blitSettings;
    const unsigned block = 128;
    unsigned y;
    unsigned dst_step, dst_y;

    /* create picture that could handle complete picture */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = context->pictureStatus.header.format;
    createSettings.width = context->pictureStatus.header.surface.width;
    createSettings.height = block;
    row = NEXUS_Surface_Create(&createSettings);
    dst_step= (block*dest->height)/context->pictureStatus.header.height;
    /* printf("scale %.3f(%.3f)\n", block/(float)dst_step, context->pictureStatus.header.surface.height/(float)dest->height); */
    for(dst_y=0,y=0; y<context->pictureStatus.header.height;y+=block,dst_y+=dst_step)  {
        NEXUS_Rect rect;
        rect.x = 0;
        rect.width = context->pictureStatus.header.surface.width;
        rect.y = y;
        if(rect.y + block <= context->pictureStatus.header.surface.height) {
            rect.height = block;
        } else {
            rect.height = context->pictureStatus.header.surface.height - rect.y;
        }
        /* start decoding */
        NEXUS_PictureDecoder_DecodeSegment(context->pictureDecoder, row, &rect);
        BKNI_WaitForEvent(context->decoderStep, BKNI_INFINITE);
        NEXUS_PictureDecoder_GetStatus(context->pictureDecoder, &pictureStatus);
        if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            fprintf(stderr, "decoding failed: line %d\n", __LINE__);
            NEXUS_Surface_Destroy(row);
            return 1;
        }
        BDBG_ASSERT(pictureStatus.state==NEXUS_PictureDecoderState_eSegmentDone);


        NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
        blitSettings.source.surface = row;
        blitSettings.source.rect.x = 0;
        blitSettings.source.rect.y = 0;
        blitSettings.source.rect.width = context->pictureStatus.header.surface.width;
        if(NEXUS_PIXEL_FORMAT_IS_YCRCB(context->pictureStatus.header.format)) {
            blitSettings.source.rect.width += blitSettings.source.rect.width%2; /* YCrCb single pixel has width of 2 */
        }
        if(rect.y + block <= context->pictureStatus.header.surface.height) {
            blitSettings.source.rect.height = block;
        } else {
            blitSettings.source.rect.height = context->pictureStatus.header.height - rect.y;
        }
        blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
        blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

        blitSettings.output.surface = framebuffer;
        blitSettings.output.rect.x = dest->x;
        blitSettings.output.rect.width = dest->width;
        blitSettings.output.rect.y = dest->y + dst_y;
        if(rect.y + block <= context->pictureStatus.header.surface.height) {
            blitSettings.output.rect.height =  dst_step;
        } else {
            blitSettings.output.rect.height = (blitSettings.source.rect.height * dest->height)/context->pictureStatus.header.height;
        }

        NEXUS_Graphics2D_Blit(context->gfx, &blitSettings);
        checkpoint(context);
    }
    NEXUS_Surface_Destroy(row);
    return 0;
}

int main(int argc, const char *argv[])
{
    NEXUS_SurfaceHandle framebuffer[2];
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_GraphicsSettings graphicsSettings;
    NEXUS_Graphics2DSettings gfxSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PictureDecoderStartSettings pictureSettings;
    NEXUS_PictureDecoderStatus pictureStatus;
    NEXUS_PictureDecoderOpenSettings pictureDecoderOpenSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_Error rc;
#endif
    struct app_context context;
    const char *jpeg = "videos/picture.jpg";
    FILE *fin ;
    void *buffer;
    size_t size;
    int file_size;
    NEXUS_Rect dest;
    unsigned width,height;
    unsigned buf;

    if(argc>1) {
        jpeg = argv[1];
    }

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format = NEXUS_VideoFormat_e720p;
    display = NEXUS_Display_Open(0, &displaySettings);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
#endif

    fin = fopen(jpeg,"rb");
    if(!fin) {
        perror(jpeg);
        return 1;
    }
    fseek(fin, 0, SEEK_END);

    NEXUS_PictureDecoder_GetDefaultOpenSettings(&pictureDecoderOpenSettings);
    pictureDecoderOpenSettings.bufferSize = ftell(fin);
    context.pictureDecoder = NEXUS_PictureDecoder_Open(0, &pictureDecoderOpenSettings);
    fseek(fin, 0, SEEK_SET);

    NEXUS_PictureDecoder_GetBuffer(context.pictureDecoder, &buffer, &size); /* get location and size of the decoder's buffer */
    file_size = fread(buffer, 1, size, fin); /* read file into the decoder's buffer */
    if(file_size<0) {
        perror(jpeg);
        return 1;
    }

    BKNI_CreateEvent(&context.checkpointEvent);
    BKNI_CreateEvent(&context.decoderStep);
    BKNI_CreateEvent(&context.bufferFlip);

    /* use graphics to fit image to the display framebuffer */
    context.gfx = NEXUS_Graphics2D_Open(0, NULL);
    NEXUS_Graphics2D_GetSettings(context.gfx, &gfxSettings);
    gfxSettings.checkpointCallback.callback = checkpoint_callback;
    gfxSettings.checkpointCallback.context = &context;
    NEXUS_Graphics2D_SetSettings(context.gfx, &gfxSettings);

    /* allocate framebuffer */
    NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
    createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    if(displaySettings.format == NEXUS_VideoFormat_e720p) {
        createSettings.width = 1280;
        createSettings.height = 720;
    } else if(displaySettings.format == NEXUS_VideoFormat_e1080i) {
        createSettings.width = 1920;
        createSettings.height = 1080;
    } else {
        createSettings.width = 720;
        createSettings.height = 480;
    }
    createSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
    framebuffer[0] = NEXUS_Surface_Create(&createSettings);
    framebuffer[1] = NEXUS_Surface_Create(&createSettings);

    NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
    graphicsSettings.enabled = true;
    graphicsSettings.frameBufferCallback.callback = buffer_flip_complete;
    graphicsSettings.frameBufferCallback.context = &context;
    graphicsSettings.position.width = createSettings.width;
    graphicsSettings.position.height = createSettings.height;
    graphicsSettings.clip.width = createSettings.width;
    graphicsSettings.clip.height = createSettings.height;
    NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);

    for(buf=0,width=createSettings.width,height=createSettings.height;;) {
        int rc;

        rc = NEXUS_PictureDecoder_ReadComplete(context.pictureDecoder, 0, file_size); /* commit data to the decoder */
        BDBG_ASSERT(!rc);

        NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
        fillSettings.surface = framebuffer[buf];
        fillSettings.color = 0;
        NEXUS_Graphics2D_Fill(context.gfx, &fillSettings);
        /* checkpoint(&context); */
        /* NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer[buf]); */

        NEXUS_PictureDecoder_GetDefaultStartSettings(&pictureSettings);
        pictureSettings.format = NEXUS_PictureFormat_eJpeg;
        pictureSettings.pictureParsed.callback = picture_decoder_step;
        pictureSettings.pictureParsed.context = &context;
        pictureSettings.segmentDecoded.callback = picture_decoder_step;
        pictureSettings.segmentDecoded.context = &context;
        rc = NEXUS_PictureDecoder_Start(context.pictureDecoder, &pictureSettings);
        BDBG_ASSERT(!rc);
        
        rc = BKNI_WaitForEvent(context.decoderStep, BKNI_INFINITE);
        BDBG_ASSERT(!rc);
        
        rc = NEXUS_PictureDecoder_GetStatus(context.pictureDecoder, &pictureStatus);
        BDBG_ASSERT(!rc);
        if(pictureStatus.state==NEXUS_PictureDecoderState_eError) {
            fprintf(stderr, "decoding failed: line %d\n", __LINE__);
            return 1;
        }
        BDBG_ASSERT(pictureStatus.headerValid);
        context.pictureStatus = pictureStatus;


#if 1
        dest.x = (createSettings.width - width)/2;
        dest.y = (createSettings.height - height)/2;
#else
        dest.x = dest.y = 0;
#endif
        dest.width = width; /* fill to fit entire screen */
        dest.height = height;

        BTRC_TRACE(frame, START);

        rc =
        /* simple_scaler(&context, framebuffer[buf], &dest); */
        /* row_scaler_uniform(&context, framebuffer[buf], &dest); */
        /* row_scaler(&context, framebuffer[buf], &dest); */
        /* row_scaler_pipe(&context, framebuffer[buf], &dest); */
        /* row_scaler_overlay_bottom(&context, framebuffer[buf], &dest); */
        row_scaler_overlay_full_uniform(&context, framebuffer[buf], &dest);
        BTRC_TRACE(frame, STOP);
        NEXUS_PictureDecoder_Stop(context.pictureDecoder);
        if(rc!=0) { break;}
        width = (width*31)/32;
        height = (height*31)/32;
        if(width*15 <= context.pictureStatus.header.width || height*7 <= context.pictureStatus.header.height) {
            width=createSettings.width;
            height=createSettings.height;
        }
        NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer[buf]);
        BKNI_WaitForEvent(context.bufferFlip, BKNI_INFINITE);
        buf = (buf+1)%2;
        BTRC_Module_Report(BTRC_MODULE_HANDLE(frame));
        BTRC_Module_Report(BTRC_MODULE_HANDLE(decode));
        BTRC_Module_Report(BTRC_MODULE_HANDLE(scale));
        BTRC_Module_Reset(BTRC_MODULE_HANDLE(decode));
        BTRC_Module_Reset(BTRC_MODULE_HANDLE(scale));
        /* getchar(); */
    }

    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
