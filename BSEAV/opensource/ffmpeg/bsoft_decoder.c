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
#include "bsoft_decoder.h"
#include "bkni.h"

BDBG_MODULE(bsoft_decoder);

#define BDBG_MSG_TRACE(x)


#define B_SOFTDECODER_FRAME_QUEUE   16
BLST_SQ_HEAD(bsoft_decoder_frame_queue, bsoft_decoder_frame);
BDBG_OBJECT_ID(bsoft_decoder);
struct bsoft_decoder {
    /* maintain to FIFO's in order to account for delay in the display pipeline, where frame is still used by the display even after callback */
    struct bsoft_decoder_frame_queue decoder_queue; /* frames avaliable for the decoder */
    struct bsoft_decoder_frame_queue display_queue; /* frames avaliable for the display */
    struct bsoft_decoder_frame_queue delay_queue; /* frames used by the display */
    unsigned decoder_frames;
    unsigned display_frames;
    unsigned delay_frames;
    const struct bsoft_decoder_frame *ready_frame;

    bsoft_decoder_config config;
    BDBG_OBJECT(bsoft_decoder)
    struct bsoft_decoder_frame frames[B_SOFTDECODER_FRAME_QUEUE];
};

static void bsoft_decoder_frame_init(struct bsoft_decoder_frame *frame);
static void bsoft_decoder_frame_shutdown(struct bsoft_decoder_frame *frame);

void bsoft_decoder_default_config(bsoft_decoder_config *config)
{
    BKNI_Memset(config, 0, sizeof(*config));
    return;
}

bsoft_decoder_t
bsoft_decoder_create(const bsoft_decoder_config *config)
{
    bsoft_decoder_t decoder;
    BERR_Code rc;
    unsigned i;

    BDBG_ASSERT(config);
#if 0
    if(config->imageInput == NULL || config->stcChannel == NULL) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);goto err_config;
    }
#endif
    decoder = BKNI_Malloc(sizeof(*decoder));
    if(!decoder) { rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc; }

    BDBG_OBJECT_INIT(decoder, bsoft_decoder);

    BLST_SQ_INIT(&decoder->decoder_queue);
    BLST_SQ_INIT(&decoder->display_queue);
    BLST_SQ_INIT(&decoder->delay_queue);
    for(i=0;i<sizeof(decoder->frames)/sizeof(decoder->frames[0]);i++) {
        bsoft_decoder_frame_init(&decoder->frames[i]);
        BLST_SQ_INSERT_TAIL(&decoder->decoder_queue, &decoder->frames[i], link);
    }
    decoder->ready_frame = NULL;
    decoder->display_frames = 0;
    decoder->delay_frames = 0;
    decoder->decoder_frames = B_SOFTDECODER_FRAME_QUEUE;
    decoder->config = *config;
    return decoder;

err_alloc:
    return NULL;
}

void
bsoft_decoder_destroy(bsoft_decoder_t decoder)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(decoder, bsoft_decoder);

    for(i=0;i<sizeof(decoder->frames)/sizeof(decoder->frames[0]);i++) {
        bsoft_decoder_frame_shutdown(&decoder->frames[i]);
    }
    BDBG_OBJECT_DESTROY(decoder, bsoft_decoder);
    BKNI_Free(decoder);
    return;
}

void
bsoft_decoder_frame_init(struct bsoft_decoder_frame *frame)
{
    frame->width = 0;
    frame->height = 0;
    frame->surface = NULL;
    frame->qscale = 8;
    return;
}

void
bsoft_decoder_frame_shutdown(struct bsoft_decoder_frame *frame)
{
    if(frame->surface) {
        NEXUS_Surface_Destroy(frame->surface);
    }
    frame->width = 0;
    frame->height = 0;
    frame->timestamp = 0;
    frame->surface = NULL;
    frame->qscale = 8;
    return;
}


NEXUS_Error
bsoft_decoder_frame_alloc(struct bsoft_decoder_frame *frame, unsigned width, unsigned height)
{
    NEXUS_SurfaceCreateSettings surfaceCfg;

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCfg);
    frame->width = width;
    frame->height = height;
    surfaceCfg.width = width;
    surfaceCfg.height = height;
    surfaceCfg.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    if(frame->surface) {
        NEXUS_Surface_Destroy(frame->surface);
    }
    frame->surface = NEXUS_Surface_Create(&surfaceCfg);
    if(!frame->surface) {
        frame->width = 0;
        frame->height = 0;
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    return NEXUS_SUCCESS;
}

static int32_t
b_timestamp_diff(uint32_t stc, uint32_t pts)
{
    return (int32_t)stc-(int32_t)pts;
}

const struct bsoft_decoder_frame *
bsoft_decoder_get_mature_frame(bsoft_decoder_t sd, uint32_t stc, NEXUS_VideoImageInputSettings *imageInputSettings)
{
    struct bsoft_decoder_frame *frame;

    BDBG_OBJECT_ASSERT(sd, bsoft_decoder);
    BDBG_ASSERT(imageInputSettings);

#if 0
    {
        NEXUS_AudioDecoderStatus audioStatus;

        NEXUS_AudioDecoder_GetStatus(sd->pcmDecoder, &audioStatus);
        BDBG_WRN(("audio0            pts %#x, fifo=%d%%", audioStatus.pts, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0));
    }
#endif

    if(sd->delay_frames>3) {
        frame = BLST_SQ_FIRST(&sd->delay_queue);
        BDBG_ASSERT(frame);
        BLST_SQ_REMOVE_HEAD(&sd->delay_queue, link);
        sd->delay_frames--;
        BLST_SQ_INSERT_TAIL(&sd->decoder_queue, frame, link);
        sd->decoder_frames++;
    }
    frame = BLST_SQ_FIRST(&sd->display_queue);
    if(frame) {
        int32_t diff = b_timestamp_diff(stc, frame->timestamp);
        BDBG_MSG(("stc=%u pts=%u %d %#lx %u qs=%u", stc/45, frame->timestamp/45, (int)diff, (unsigned long)frame, sd->display_frames, frame->qscale));
        if(diff>=-15*45) {
            struct bsoft_decoder_frame *next_frame;
            while(NULL!=(next_frame=BLST_SQ_NEXT(frame, link))) {
                diff = b_timestamp_diff(stc, next_frame->timestamp);
                BDBG_MSG_TRACE(("stc=%u pts_next=%u %d %#lx", stc/45, next_frame->timestamp/45, (int)diff, (unsigned long)next_frame));
                if(diff<15*45) {
                    break;
                }
                BDBG_MSG(("tsm drop %#lx", (unsigned long)frame));
                BLST_SQ_REMOVE_HEAD(&sd->display_queue, link);
                sd->display_frames--;
                BLST_SQ_INSERT_TAIL(&sd->decoder_queue, frame, link);
                sd->decoder_frames++;
                frame = next_frame;
            }
            BLST_SQ_REMOVE_HEAD(&sd->display_queue, link);
            sd->display_frames--;
            BLST_SQ_INSERT_TAIL(&sd->delay_queue, frame, link);
            sd->delay_frames++;
            imageInputSettings->duplicate = false;
            imageInputSettings->qScale = frame->qscale;
            BDBG_MSG(("display %#lx:%u:%u:%u", (unsigned long)frame, sd->decoder_frames, sd->display_frames, sd->delay_frames));
        } else {
            frame = BLST_SQ_LAST(&sd->delay_queue);
            imageInputSettings->duplicate = true;
            BDBG_MSG(("tsm repeat %#lx", (unsigned long)frame));
        }
    } else {
        BDBG_MSG(("display underflow %#lx", (unsigned long)frame));
        frame = BLST_SQ_LAST(&sd->delay_queue);
        BDBG_ASSERT(frame);
    }
    return frame;
}

const struct bsoft_decoder_frame *
bsoft_decoder_get_display_frame(bsoft_decoder_t sd, NEXUS_VideoImageInputSettings *imageInputSettings)
{
    struct bsoft_decoder_frame *frame;

    BDBG_OBJECT_ASSERT(sd, bsoft_decoder);
    BDBG_ASSERT(imageInputSettings);
    frame = BLST_SQ_FIRST(&sd->display_queue);

    if(frame!=NULL) {
        BLST_SQ_REMOVE_HEAD(&sd->display_queue, link);
        sd->display_frames--;
        BLST_SQ_INSERT_TAIL(&sd->delay_queue, frame, link);
        sd->delay_frames++;
        imageInputSettings->duplicate = false;
        imageInputSettings->qScale = frame->qscale;
    }
    return frame;
}

#if 0
NEXUS_Error
bsoft_decoder_show_frame(bsoft_decoder_t sd)
{
    const struct bsoft_decoder_frame *frame;
    BDBG_OBJECT_ASSERT(sd, bsoft_decoder);

    frame = sd->ready_frame;
    if(frame) {
        sd->ready_frame = NULL;
        NEXUS_Surface_Flush(frame->surface);
        NEXUS_VideoImageInput_SetSettings(sd->config.imageInput, &sd->imageInputSettings);
        NEXUS_VideoImageInput_SetSurface(sd->config.imageInput, frame->surface);
    }
    return NEXUS_SUCCESS;
}
#endif


struct bsoft_decoder_frame *
bsoft_decoder_pop_free_frame(bsoft_decoder_t sd)
{
    struct bsoft_decoder_frame *frame;
    BDBG_OBJECT_ASSERT(sd, bsoft_decoder);
    frame = BLST_SQ_FIRST(&sd->decoder_queue);
    if(frame) {
        BLST_SQ_REMOVE_HEAD(&sd->decoder_queue, link);
        sd->decoder_frames--;
    }
    return frame;
}

void
bsoft_decoder_push_decoded_frame(bsoft_decoder_t sd, struct bsoft_decoder_frame *frame)
{
    BDBG_OBJECT_ASSERT(sd, bsoft_decoder);
    BDBG_ASSERT(frame);

    BLST_SQ_INSERT_TAIL(&sd->display_queue, frame, link);
    sd->display_frames++;
    return;
}
