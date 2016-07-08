/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  .  Except as set forth in an Authorized License, Broadcom grants
 *  no license , right to use, or waiver of any kind with respect to the
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
 *  LICENSORS BE LIABLE FOR  CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR  ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 **************************************************************************/
#ifndef NEXUS_VIDEO_DECODER_MODULE_SOFT_H__
#define NEXUS_VIDEO_DECODER_MODULE_SOFT_H__

#include "nexus_video_decoder_soft.h"
#include "blst_squeue.h"
#include "nexus_graphics2d.h"
#define NEXUS_VIDEO_DECODER_SOFT_VERIFY_DATA    0
#if NEXUS_VIDEO_DECODER_SOFT_VERIFY_DATA
#include <stdio.h>
#endif

NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_P_Open_Soft( unsigned index, const NEXUS_VideoDecoderOpenSettings *pOpenSettings);
NEXUS_Error NEXUS_VideoDecoderModule_P_Init_Soft(const NEXUS_VideoDecoderModuleInternalSettings *pSettings);
void NEXUS_VideoDecoderModule_P_Uninit_Soft(void);

BDBG_OBJECT_ID_DECLARE(NEXUS_VideoDecoder_Soft);

typedef struct b_avcodec_frame_buffer {
    void *buf;
    NEXUS_Addr addr;
    size_t size;
} b_avcodec_frame_buffer;

typedef struct NEXUS_VideoDecoder_P_FrameElement {
    BXDM_Picture picture; /* must be the first member */
    BLST_SQ_ENTRY(NEXUS_VideoDecoder_P_FrameElement) link;
    b_avcodec_frame_buffer buffer;
} NEXUS_VideoDecoder_P_FrameElement;

BLST_SQ_HEAD(NEXUS_VideoDecoder_P_FrameQueue_Head, NEXUS_VideoDecoder_P_FrameElement);

typedef struct NEXUS_VideoDecoder_P_FrameQueue {
    struct NEXUS_VideoDecoder_P_FrameQueue_Head list;
    unsigned count;
} NEXUS_VideoDecoder_P_FrameQueue;

#define NEXUS_VIDEODECODER_SOFT_MEMORYBLOCK_MAX 16
struct NEXUS_VideoDecoder_P_Soft_MemoryBlock {
    BMMA_Block_Handle block;
    void *ptr; /* NULL if invalid */
    NEXUS_Addr addr;
    size_t size;
    unsigned ref_cnt;
};

#define NEXUS_VIDEO_DECODER_P_SOFTDECODE_N_FRAMES   8

struct NEXUS_VideoDecoder_Soft_P_FrameMark {
    uint32_t cdbOffset;
    uint32_t pts;
    bool cdbOffsetValid;
    bool ptsValid;
};

typedef struct NEXUS_VideoDecoder_Soft {
    BDBG_OBJECT(NEXUS_VideoDecoder_Soft)
    b_video_softdecode_t soft_decode;
    NEXUS_VideoDecoder_P_FrameQueue ready;
    struct NEXUS_VideoDecoder_P_FrameQueue_Head free; /* frames that could be used by the decoder */
    struct NEXUS_VideoDecoder_P_FrameQueue_Head display; /* frames used in the display */
    struct NEXUS_VideoDecoder_P_FrameQueue_Head recycled; /* frames recycled by the display */
    bool threadExit;
    bool paused;
    bool poweredUp;
    BKNI_MutexHandle lock; /* protects calls to the software video decoder */
    BKNI_MutexHandle memoryLock; /* protects access to the memory allocation state */
    BKNI_EventHandle event;
    BKNI_EventHandle checkpointEvent;
    NEXUS_Graphics2DHandle graphics;
    NEXUS_ThreadHandle thread;
    NEXUS_VideoDecoder_P_FrameElement frames[NEXUS_VIDEO_DECODER_P_SOFTDECODE_N_FRAMES];
    struct {
        bool valid;
        unsigned width;
        unsigned height;
        unsigned stride;
        BMMA_Block_Handle block;
        unsigned max_frame_size;
        void *cached;
        NEXUS_Addr addr;
    } frameMemory;
#if NEXUS_VIDEO_DECODER_SOFT_VERIFY_DATA
    struct {
        unsigned frame_cnt;
        FILE *fin;
    } data;
#endif
    struct {
        struct {
            unsigned itbVisited;
            struct NEXUS_VideoDecoder_Soft_P_FrameMark current,prev;
            struct {
                bool valid;
                bool ptsValid;
                size_t length;
                uint32_t pts;
                size_t data_size;
                uint8_t data[128*1024 - 128]; /* variable length, must be the last field */
            } * compressedFrame;
        } scanFrame;
        struct NEXUS_VideoDecoder_P_Soft_MemoryBlock memoryBlocks[NEXUS_VIDEODECODER_SOFT_MEMORYBLOCK_MAX];
    } state;
} NEXUS_VideoDecoder_Soft;


#endif /* NEXUS_VIDEO_DECODER_MODULE_SOFT_H__ */
