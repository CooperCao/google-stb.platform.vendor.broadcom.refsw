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
#include "nexus_video_decoder_module.h"
#include "priv/nexus_core.h"
#include "biobits.h"
#include "bioatom.h"
#include "bxpt_rave.h"
#if NEXUS_VIDEO_DECODER_LIBVPX
#include "nexus_video_decoder_soft_vpx.h"
#else
#include "nexus_video_decoder_soft_ffmpeg.h"
#endif

BDBG_MODULE(nexus_video_decoder_soft);
BDBG_OBJECT_ID(NEXUS_VideoDecoder_Soft);

#define BDBG_MSG_TRACE(x) /* BDBG_LOG(x) */

#define NOT_SUPPORTED(x) NULL
static const NEXUS_VideoDecoder_P_Interface NEXUS_VideoDecoder_P_Interface_Soft;

static NEXUS_Error NEXUS_VideoDecoder_P_Soft_Memory_Allocate(void *self, size_t size, b_video_softdecode_memory_token *token)
{
    NEXUS_VideoDecoderHandle decoder = self;
    const NEXUS_VideoDecoderModuleSettings *pSettings = &g_NEXUS_videoDecoderModuleSettings;
    BMMA_Heap_Handle heap = g_pCoreHandles->heap[pSettings->avdHeapIndex[0]].mma;
    BMMA_Block_Handle block;
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned i;
    struct NEXUS_VideoDecoder_P_Soft_MemoryBlock *memoryBlock=NULL;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BKNI_AcquireMutex(decoder->decoder.soft.memoryLock);

    for(i=0;i<NEXUS_VIDEODECODER_SOFT_MEMORYBLOCK_MAX;i++) {
        if(decoder->decoder.soft.state.memoryBlocks[i].block == NULL) {
            memoryBlock = &decoder->decoder.soft.state.memoryBlocks[i];
            break;
        }
    }
    if(memoryBlock == NULL) {
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE); goto err_memory_block;
    }

    BKNI_Memset(token,0,sizeof(*token));
    memoryBlock->size = size;
    block = BMMA_Alloc(heap, size, 0, NULL);
    if(block==NULL) {
        rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        goto err_alloc;
    }
    memoryBlock->addr = BMMA_LockOffset(block);
    if(memoryBlock->addr == 0) {
        rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        goto err_lock_offset;
    }
    token->ptr = BMMA_Lock(block);
    memoryBlock->ptr = token->ptr;
    if(token->ptr==NULL) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_lock;
    }
    memoryBlock->block = block;
    memoryBlock->ref_cnt = 1;

    token->opaque = memoryBlock;
    BKNI_ReleaseMutex(decoder->decoder.soft.memoryLock);
    return NEXUS_SUCCESS;

err_lock:
    BMMA_UnlockOffset(block, memoryBlock->addr);
err_lock_offset:
    BMMA_Free(block);
err_alloc:
err_memory_block:
    BKNI_ReleaseMutex(decoder->decoder.soft.memoryLock);
    return rc;
}

static void NEXUS_VideoDecoder_P_Soft_Memory_FreeOne(struct NEXUS_VideoDecoder_P_Soft_MemoryBlock *memoryBlock)
{
    BMMA_Unlock(memoryBlock->block, memoryBlock->ptr);
    BMMA_UnlockOffset(memoryBlock->block, memoryBlock->addr);
    BMMA_Free(memoryBlock->block);
    memoryBlock->block = NULL;
    return;
}

static void NEXUS_VideoDecoder_P_Soft_Memory_Free(void *self, const b_video_softdecode_memory_token *token)
{
    NEXUS_VideoDecoderHandle decoder = self;
    struct NEXUS_VideoDecoder_P_Soft_MemoryBlock *memoryBlock=token->opaque;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BKNI_AcquireMutex(decoder->decoder.soft.memoryLock);
    if(memoryBlock->ref_cnt<=0) { (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);goto fail; }
    memoryBlock->ref_cnt --;
    if(memoryBlock->ref_cnt == 0) {
        NEXUS_VideoDecoder_P_Soft_Memory_FreeOne(memoryBlock);
    }
fail:
    BKNI_ReleaseMutex(decoder->decoder.soft.memoryLock);

    return;
}

static const b_video_sofdecode_memory_methods NEXUS_VideoDecoder_P_Memory_Methods = {
    NEXUS_VideoDecoder_P_Soft_Memory_Allocate,
    NEXUS_VideoDecoder_P_Soft_Memory_Free
};

static void
NEXUS_VideoDecoder_P_Soft_Memory_Release( NEXUS_VideoDecoderHandle decoder)
{
    unsigned i;
    BKNI_AcquireMutex(decoder->decoder.soft.memoryLock);
    for(i=0;i<NEXUS_VIDEODECODER_SOFT_MEMORYBLOCK_MAX;i++) {
        struct NEXUS_VideoDecoder_P_Soft_MemoryBlock *memoryBlock = &decoder->decoder.soft.state.memoryBlocks[i];
        if(memoryBlock->block) {
            NEXUS_VideoDecoder_P_Soft_Memory_FreeOne(memoryBlock);
        }
    }
    BKNI_ReleaseMutex(decoder->decoder.soft.memoryLock);
    return;
}

const struct NEXUS_VideoDecoder_P_Soft_MemoryBlock *
NEXUS_VideoDecoder_P_Soft_Memory_Find(NEXUS_VideoDecoderHandle decoder, const void *ptr)
{
    unsigned i;
    for(i=0;i<NEXUS_VIDEODECODER_SOFT_MEMORYBLOCK_MAX;i++) {
        const struct NEXUS_VideoDecoder_P_Soft_MemoryBlock *memoryBlock = &decoder->decoder.soft.state.memoryBlocks[i];
        if(memoryBlock->block) {
            if( (uint8_t *)ptr >= (uint8_t *)memoryBlock->ptr && (uint8_t *)ptr <  memoryBlock->size + (uint8_t *)memoryBlock->ptr) {
                return memoryBlock;
            }
        }
    }
    BDBG_ERR(("Unknown ptr %p", ptr));
    return NULL;
}


static NEXUS_Error NEXUS_VideoDecoder_P_SetSettings_Soft( NEXUS_VideoDecoderHandle decoder, const NEXUS_VideoDecoderSettings *pSettings)
{
    NEXUS_Error errCode;
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);

    errCode = NEXUS_VideoDecoder_P_Xdm_ApplySettings(decoder, pSettings, false);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Store settings */
    decoder->settings = *pSettings;

    return NEXUS_SUCCESS;
}

static void
NEXUS_VideoDecoder_P_Soft_InitFrames(NEXUS_VideoDecoderHandle decoder)
{
    decoder->decoder.soft.frameMemory.block = NULL;
    decoder->decoder.soft.frameMemory.max_frame_size = 0;
    decoder->decoder.soft.frameMemory.valid = false;
    return;
}

static void
NEXUS_VideoDecoder_P_Soft_FreeFrames(NEXUS_VideoDecoderHandle decoder)
{
    if(decoder->decoder.soft.frameMemory.valid) {
        BMMA_Unlock(decoder->decoder.soft.frameMemory.block, decoder->decoder.soft.frameMemory.cached);
        BMMA_Free(decoder->decoder.soft.frameMemory.block);
        decoder->decoder.soft.frameMemory.valid = false;
        decoder->decoder.soft.frameMemory.max_frame_size = 0;
    }
    return;
}


static NEXUS_Error
NEXUS_VideoDecoder_P_Soft_SetFrames(NEXUS_VideoDecoderHandle decoder, unsigned width, unsigned height, unsigned max_width, unsigned max_height)
{
    unsigned aligned_width = 16 * ((width+15)/16);
    unsigned aligned_max_width = 16 * ((max_width+15)/16);
    unsigned frame_size;
    unsigned max_frame_size;
    unsigned i;
    const NEXUS_VideoDecoderModuleSettings *pSettings = &g_NEXUS_videoDecoderModuleSettings;
    BMMA_Heap_Handle heap = g_pCoreHandles->heap[pSettings->avdHeapIndex[0]].mma;

    frame_size = 2 * aligned_width * height;
    max_frame_size = 2 * aligned_max_width * max_height;
    if(decoder->decoder.soft.frameMemory.max_frame_size < max_frame_size) {
        NEXUS_VideoDecoder_P_Soft_FreeFrames(decoder);
        decoder->decoder.soft.frameMemory.block = BMMA_Alloc(heap, max_frame_size*(NEXUS_VIDEO_DECODER_P_SOFTDECODE_N_FRAMES), 0, NULL);
        if(!decoder->decoder.soft.frameMemory.block) { return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);}

        decoder->decoder.soft.frameMemory.addr = BMMA_LockOffset(decoder->decoder.soft.frameMemory.block);

        decoder->decoder.soft.frameMemory.cached = BMMA_Lock(decoder->decoder.soft.frameMemory.block);
        if(decoder->decoder.soft.frameMemory.cached==NULL) {
            BMMA_Free(decoder->decoder.soft.frameMemory.block);
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        decoder->decoder.soft.frameMemory.max_frame_size = max_frame_size;
        decoder->decoder.soft.frameMemory.valid = true;
    }
    max_frame_size = decoder->decoder.soft.frameMemory.max_frame_size;
    decoder->decoder.soft.frameMemory.width = width;
    decoder->decoder.soft.frameMemory.height = height;
    decoder->decoder.soft.frameMemory.stride = aligned_width * 2;

    for(i=0;i<NEXUS_VIDEO_DECODER_P_SOFTDECODE_N_FRAMES;i++) {
        decoder->decoder.soft.frames[i].buffer.buf = (uint8_t *)decoder->decoder.soft.frameMemory.cached + i*max_frame_size;
        decoder->decoder.soft.frames[i].buffer.addr = decoder->decoder.soft.frameMemory.addr + i*max_frame_size;
        decoder->decoder.soft.frames[i].buffer.size = frame_size;
    }
#if 0
    for(i=0;i<0;i++) {
        NEXUS_VideoDecoder_P_Soft_BuildBlackFrame(decoder, &decoder->decoder.soft.frames[i], 0x60+0x20*i, 0x80);
        NEXUS_VideoDecoder_P_FrameQueue_Add_isr(&decoder->decoder.soft.ready, &decoder->decoder.soft.frames[i]);
    }
#endif

    return NEXUS_SUCCESS;

}


#if 0
static NEXUS_Error
NEXUS_VideoDecoder_P_Soft_AllocateFrames(NEXUS_VideoDecoderHandle decoder, unsigned width, unsigned height)
{
    unsigned aligned_width = 16 * ((width+15)/16);
    unsigned frame_size;
    unsigned i;
    const NEXUS_VideoDecoderModuleSettings *pSettings = &g_NEXUS_videoDecoderModuleSettings;
    BMMA_Heap_Handle heap = g_pCoreHandles->heap[pSettings->avdHeapIndex[0]].mma;

    frame_size = 2 * aligned_width * height;
    decoder->decoder.soft.frameMemory.block = BMMA_Alloc(heap, frame_size*(NEXUS_VIDEO_DECODER_P_SOFTDECODE_N_FRAMES), 0, NULL);
    if(!decoder->decoder.soft.frameMemory.block) { return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);}

    decoder->decoder.soft.frameMemory.addr = BMMA_LockOffset(decoder->decoder.soft.frameMemory.block);

    decoder->decoder.soft.frameMemory.cached = BMMA_Lock(decoder->decoder.soft.frameMemory.block);
    if(decoder->decoder.soft.frameMemory.cached==NULL) {
        BMMA_Free(decoder->decoder.soft.frameMemory.block);
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    decoder->decoder.soft.frameMemory.width = width;
    decoder->decoder.soft.frameMemory.height = height;
    decoder->decoder.soft.frameMemory.stride = aligned_width * 2;
    decoder->decoder.soft.frameMemory.valid = true;

    for(i=0;i<NEXUS_VIDEO_DECODER_P_SOFTDECODE_N_FRAMES;i++) {
        decoder->decoder.soft.frames[i].buffer.buf = (uint8_t *)decoder->decoder.soft.frameMemory.cached + i*frame_size;
        decoder->decoder.soft.frames[i].buffer.addr = decoder->decoder.soft.frameMemory.addr + i*frame_size;
        decoder->decoder.soft.frames[i].buffer.size = frame_size;
    }
#if 0
    for(i=0;i<0;i++) {
        NEXUS_VideoDecoder_P_Soft_BuildBlackFrame(decoder, &decoder->decoder.soft.frames[i], 0x60+0x20*i, 0x80);
        NEXUS_VideoDecoder_P_FrameQueue_Add_isr(&decoder->decoder.soft.ready, &decoder->decoder.soft.frames[i]);
    }
#endif

    return NEXUS_SUCCESS;
}
#endif

static void NEXUS_VideoDecoder_P_FrameQueue_Init(NEXUS_VideoDecoder_P_FrameQueue *fq)
{
    BLST_SQ_INIT(&fq->list);
    fq->count = 0;
    return;
}

static void NEXUS_VideoDecoder_P_FrameQueue_Add_isr(NEXUS_VideoDecoder_P_FrameQueue *fq, NEXUS_VideoDecoder_P_FrameElement *f)
{
    BLST_SQ_INSERT_TAIL(&fq->list, f, link);
    fq->count++;
    return;
}

static void NEXUS_VideoDecoder_P_FrameQueue_Remove_isr(NEXUS_VideoDecoder_P_FrameQueue *fq, NEXUS_VideoDecoder_P_FrameElement *f)
{
    BDBG_ASSERT(fq->count>0);
    BLST_SQ_REMOVE(&fq->list, f, NEXUS_VideoDecoder_P_FrameElement, link);
    fq->count--;
    return;
}

static NEXUS_Error NEXUS_VideoDecoder_P_Start_SoftDecoder(NEXUS_VideoDecoderHandle decoder)
{
    BERR_Code rc;

    BKNI_AcquireMutex(decoder->decoder.soft.lock);
    rc = decoder->decoder.soft.soft_decode->methods->start(decoder->decoder.soft.soft_decode, decoder->startSettings.codec);

    decoder->decoder.soft.state.scanFrame.current.pts = 0;
    decoder->decoder.soft.state.scanFrame.current.cdbOffset = 0;
    decoder->decoder.soft.state.scanFrame.current.ptsValid = false;
    decoder->decoder.soft.state.scanFrame.current.cdbOffsetValid = false;
    decoder->decoder.soft.state.scanFrame.prev = decoder->decoder.soft.state.scanFrame.current;
    decoder->decoder.soft.state.scanFrame.compressedFrame->valid = false;
    decoder->decoder.soft.state.scanFrame.compressedFrame->ptsValid = false;
    decoder->decoder.soft.paused = false;
    BKNI_SetEvent(decoder->decoder.soft.event);
    BKNI_ReleaseMutex(decoder->decoder.soft.lock);
    if(rc!=NEXUS_SUCCESS) {
        return BERR_TRACE(rc);
    }
    return NEXUS_SUCCESS;
}

static void NEXUS_VideoDecoder_P_Stop_SoftDecoder(NEXUS_VideoDecoderHandle decoder)
{
    decoder->decoder.soft.paused = true;
    BKNI_AcquireMutex(decoder->decoder.soft.lock);
    decoder->decoder.soft.soft_decode->methods->stop(decoder->decoder.soft.soft_decode);
    BKNI_SetEvent(decoder->decoder.soft.event);
    BKNI_ReleaseMutex(decoder->decoder.soft.lock);

    /* recycle all queued frames */
    for(;;) {
        NEXUS_VideoDecoder_P_FrameElement *frame;
        BKNI_EnterCriticalSection();
        frame=BLST_SQ_FIRST(&decoder->decoder.soft.ready.list);
        if(frame) {
            NEXUS_VideoDecoder_P_FrameQueue_Remove_isr(&decoder->decoder.soft.ready, frame);
            BLST_SQ_INSERT_HEAD(&decoder->decoder.soft.free, frame, link);
        }
        BKNI_LeaveCriticalSection();
        if(frame==NULL) {
            break;
        }
    }

    return;
}

static NEXUS_Error NEXUS_VideoDecoder_P_Start_Soft( NEXUS_VideoDecoderHandle decoder, const NEXUS_VideoDecoderStartSettings *pSettings)
{
    NEXUS_Error  rc;
    NEXUS_StcChannelDecoderConnectionSettings stcChannelConnectionSettings;
    BAVC_VideoCompressionStd videoCmprStd;
    NEXUS_RaveStatus raveStatus;
    bool playback = false;
    unsigned stcChannelIndex;

    BDBG_MSG(("NEXUS_VideoDecoder_P_Start_Soft:%#x", (unsigned)decoder));
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);


    rc = NEXUS_VideoDecoder_P_Start_Generic_Prologue(decoder, pSettings);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}


    LOCK_TRANSPORT();
    NEXUS_StcChannel_GetDefaultDecoderConnectionSettings_priv(&stcChannelConnectionSettings);
    stcChannelConnectionSettings.type = NEXUS_StcChannelDecoderType_eVideo;
    stcChannelConnectionSettings.priority = decoder->stc.priority;
    stcChannelConnectionSettings.getPts_isr = NEXUS_VideoDecoder_P_GetPtsCallback_Xdm_isr;
    stcChannelConnectionSettings.getCdbLevel_isr = NEXUS_VideoDecoder_P_GetCdbLevelCallback_isr;
    stcChannelConnectionSettings.stcValid_isr = NEXUS_VideoDecoder_P_StcValidCallback_Xdm_isr;
    stcChannelConnectionSettings.pCallbackContext = decoder;
    UNLOCK_TRANSPORT();

    rc = NEXUS_VideoDecoder_P_Start_SoftDecoder(decoder);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    rc = NEXUS_VideoDecoder_P_Start_Generic_Body(decoder, pSettings, false, &videoCmprStd, &raveStatus, &stcChannelConnectionSettings, &playback, &stcChannelIndex);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    rc = NEXUS_VideoDecoder_P_Xdm_Start(decoder);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

#if NEXUS_VIDEO_DECODER_SOFT_VERIFY_DATA
    decoder->decoder.soft.data.frame_cnt = 0;
    decoder->decoder.soft.data.fin = fopen("soft.dat", "rb");
    if(!decoder->decoder.soft.data.fin) { return BERR_TRACE(NEXUS_NOT_SUPPORTED);}
#endif


    rc = NEXUS_VideoDecoder_P_Start_Priv_Generic_Epilogue(decoder, pSettings);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    rc = NEXUS_VideoDecoder_P_Start_Generic_Epilogue(decoder, pSettings);
    if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}

    return NEXUS_SUCCESS;
}

static void NEXUS_VideoDecoder_P_Stop_Soft( NEXUS_VideoDecoderHandle decoder)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);

    rc = NEXUS_VideoDecoder_P_Stop_Generic_Prologue(decoder);
    if(rc!=NEXUS_SUCCESS) {return;}


    NEXUS_VideoDecoder_P_Stop_Priv_Generic_Prologue(decoder);

    NEXUS_VideoDecoder_P_Stop_SoftDecoder(decoder);

    NEXUS_VideoDecoder_P_Xdm_Stop(decoder);

    NEXUS_VideoDecoder_P_Stop_Priv_Generic_Epilogue(decoder);
    NEXUS_VideoDecoder_P_Stop_Generic_Epilogue(decoder);

    return;
}

static void NEXUS_VideoDecoder_P_Flush_Soft( NEXUS_VideoDecoderHandle decoder)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);
    NEXUS_VideoDecoder_P_Xdm_Stop(decoder);

    NEXUS_VideoDecoder_P_Stop_SoftDecoder(decoder);

    LOCK_TRANSPORT();
    NEXUS_Rave_Disable_priv(decoder->rave);
    NEXUS_Rave_Flush_priv(decoder->rave);
    UNLOCK_TRANSPORT();

    rc = NEXUS_VideoDecoder_P_Xdm_Start(decoder);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}

    LOCK_TRANSPORT();
    NEXUS_Rave_Enable_priv(decoder->rave);
    UNLOCK_TRANSPORT();

    rc = NEXUS_VideoDecoder_P_Start_SoftDecoder(decoder);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);}

    return;
}

static NEXUS_Error NEXUS_VideoDecoder_P_SoftInitPlane(NEXUS_VideoDecoderHandle decoder, BM2MC_PACKET_Plane *pPlane, const b_avcodec_plane *plane, unsigned width, unsigned height, NEXUS_PixelFormat pixelFormat)
{
    const struct NEXUS_VideoDecoder_P_Soft_MemoryBlock *block;
    block = NEXUS_VideoDecoder_P_Soft_Memory_Find(decoder, plane->buf);
    if(block==NULL) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    pPlane->address = block->addr + ((uint8_t *)plane->buf - (uint8_t *)block->ptr);
    pPlane->pitch = plane->stride;
    pPlane->format = pixelFormat;
    pPlane->width = width;
    pPlane->height = height;
    NEXUS_FlushCache(plane->buf, height * plane->stride);
    return NEXUS_SUCCESS;
}

static void NEXUS_VideoDecoder_P_ConvertFrame(NEXUS_VideoDecoderHandle decoder, NEXUS_VideoDecoder_P_FrameElement *frame, const b_avcodec_frame *soft_frame)
{
    BKNI_Memset(&frame->picture, 0, sizeof(frame->picture));
    frame->picture.stBufferInfo.eBufferFormat = BXDM_Picture_BufferFormat_eSingle;
    frame->picture.stBufferInfo.eBufferHandlingMode = BXDM_Picture_BufferHandlingMode_eNormal;
    frame->picture.stBufferInfo.eLumaBufferType = BXDM_Picture_BufferType_e8Bit;
    frame->picture.stBufferInfo.hLuminanceFrameBufferBlock = decoder->decoder.soft.frameMemory.block;
    frame->picture.stBufferInfo.ulLuminanceFrameBufferBlockOffset = (uint8_t *)frame->buffer.buf - (uint8_t *)decoder->decoder.soft.frameMemory.cached;
    frame->picture.stBufferInfo.hChrominanceFrameBufferBlock = NULL;
    frame->picture.stBufferInfo.ulChrominanceFrameBufferBlockOffset = 0;
    frame->picture.stBufferInfo.stSource.bValid = true;
    frame->picture.stBufferInfo.stSource.uiHeight = soft_frame->height;
    frame->picture.stBufferInfo.stSource.uiWidth = soft_frame->width;
    frame->picture.stPictureType.bRandomAccessPoint = soft_frame->random_access_point;
    frame->picture.stPictureType.eCoding = soft_frame->picture_type;
    frame->picture.stAspectRatio.eAspectRatio = BFMT_AspectRatio_eSAR;
    frame->picture.stAspectRatio.uiSampleAspectRatioX = soft_frame->sample_aspect_ratio.numerator;
    frame->picture.stAspectRatio.uiSampleAspectRatioY = soft_frame->sample_aspect_ratio.denominator;
    frame->picture.stAspectRatio.bValid = soft_frame->sample_aspect_ratio_valid;

#if 1
    /* convert frames from YUV 420 to YUV 422 */
    if(1){
        static const BM2MC_PACKET_Blend combColor = {BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, false,
            BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eOne, false, BM2MC_PACKET_BlendFactor_eZero};
        static const BM2MC_PACKET_Blend copyAlpha = {BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eOne, false,
            BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false, BM2MC_PACKET_BlendFactor_eZero};
        BM2MC_PACKET_Plane planeY, planeCb, planeCr, planeYCbCr;
        void *buffer, *next;
        size_t size;
        NEXUS_Error rc;

        NEXUS_VideoDecoder_P_SoftInitPlane(decoder, &planeY, &soft_frame->y, soft_frame->width, soft_frame->height, NEXUS_PixelFormat_eY8);
        NEXUS_VideoDecoder_P_SoftInitPlane(decoder, &planeCb, &soft_frame->u, soft_frame->width/2, soft_frame->height/2, NEXUS_PixelFormat_eCb8);
        NEXUS_VideoDecoder_P_SoftInitPlane(decoder, &planeCr, &soft_frame->v, soft_frame->width/2, soft_frame->height/2, NEXUS_PixelFormat_eCr8);

        planeYCbCr.address = frame->buffer.addr;
        planeYCbCr.pitch = decoder->decoder.soft.frameMemory.stride;
        planeYCbCr.format = NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8;
        planeYCbCr.width = soft_frame->width;
        planeYCbCr.height = soft_frame->height;

        /* contributed by Shi-Long (Steven) Yang <syang@broadcom.com> */
        rc = NEXUS_Graphics2D_GetPacketBuffer(decoder->decoder.soft.graphics, &buffer, &size, 1024);
        BDBG_ASSERT((!rc) && (size));

        next = buffer;
        {
            BM2MC_PACKET_PacketFilterEnable *pPacket = next;
            BM2MC_PACKET_INIT(pPacket, FilterEnable, false );
            pPacket->enable = 0;
            next = ++pPacket;
        }
        {
            BM2MC_PACKET_PacketSourceFeeders *pPacket = next;
            BM2MC_PACKET_INIT(pPacket, SourceFeeders, false );
            pPacket->plane0 = planeCb;
            pPacket->plane1 = planeCr;
            pPacket->color = 0;
            next = ++pPacket;
        }
        {
            BM2MC_PACKET_PacketDestinationFeeder *pPacket = next;
            BM2MC_PACKET_INIT(pPacket, DestinationFeeder, false );
            pPacket->plane = planeY;
            pPacket->color = 0;
            next = ++pPacket;
        }
        {
            BM2MC_PACKET_PacketOutputFeeder *pPacket = next;
            BM2MC_PACKET_INIT(pPacket, OutputFeeder, false);
            pPacket->plane = planeYCbCr;
            next = ++pPacket;
        }
        {
            BM2MC_PACKET_PacketBlend *pPacket = next;
            BM2MC_PACKET_INIT( pPacket, Blend, false );
            pPacket->color_blend = combColor;
            pPacket->alpha_blend = copyAlpha;
            pPacket->color = 0;
            next = ++pPacket;
        }
        {
            BM2MC_PACKET_PacketScaleBlendBlit *pPacket = next;
            BM2MC_PACKET_INIT(pPacket, ScaleBlendBlit, true);
            pPacket->src_rect.x = 0;
            pPacket->src_rect.y = 0;
            pPacket->src_rect.width = planeCb.width;
            pPacket->src_rect.height = planeCb.height;
            pPacket->out_rect.x = 0;
            pPacket->out_rect.y = 0;
            pPacket->out_rect.width = planeYCbCr.width;
            pPacket->out_rect.height = planeYCbCr.height;
            pPacket->dst_point.x = 0;
            pPacket->dst_point.y = 0;
            next = ++pPacket;
        }

        rc = NEXUS_Graphics2D_PacketWriteComplete(decoder->decoder.soft.graphics, (uint8_t*)next - (uint8_t*)buffer);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Graphics2D_Checkpoint(decoder->decoder.soft.graphics, NULL);
        if (rc == NEXUS_GRAPHICS2D_QUEUED) {
            rc = BKNI_WaitForEvent(decoder->decoder.soft.checkpointEvent, 5000);
            BDBG_ASSERT(!rc);
        }
        else {
            BDBG_ASSERT(!rc);
        }
    }
#endif



    if(0) {
        unsigned y;

        BKNI_Memset(frame->buffer.buf, 0x80, frame->buffer.size);
        for(y=0;y<soft_frame->height;y++) {
            uint8_t *dst = (uint8_t *)frame->buffer.buf + 2*soft_frame->y.stride*y;
            const uint8_t *src_y = (uint8_t *)soft_frame->y.buf + soft_frame->y.stride*y;
            const uint8_t *src_u = (uint8_t *)soft_frame->u.buf + soft_frame->u.stride*(y/2);
            const uint8_t *src_v = (uint8_t *)soft_frame->v.buf + soft_frame->v.stride*(y/2);
            unsigned x;
            for(x=0;x<soft_frame->width;x+=2) {
#if 1
                dst[3] = src_y[0];
                dst[1] = src_y[1];
#endif
#if 1
                dst[0] = src_v[0];
                dst[2] = src_u[0];
#endif
#if 0
                if(x==100) {
                    dst[1] = 0xDF;
                    dst[3] = dst[1];
                }
#endif
                dst+=4;
                src_y+=2;
                src_u+=1;
                src_v+=1;
            }
        }
        BMMA_FlushCache(decoder->decoder.soft.frameMemory.block, frame->buffer.buf, frame->buffer.size);
    }

    frame->picture.stBufferInfo.stDisplay = frame->picture.stBufferInfo.stSource;
    /* Omit display size and striping info for now */
    frame->picture.stBufferInfo.stChromaLocation[BAVC_Polarity_eFrame].bValid = true;
    frame->picture.stBufferInfo.stChromaLocation[BAVC_Polarity_eFrame].eMpegType = BAVC_MpegType_eMpeg2;
    frame->picture.stBufferInfo.stChromaLocation[BAVC_Polarity_eTopField].bValid = true;
    frame->picture.stBufferInfo.stChromaLocation[BAVC_Polarity_eTopField].eMpegType= BAVC_MpegType_eMpeg2;
    frame->picture.stBufferInfo.stChromaLocation[BAVC_Polarity_eBotField].bValid = true;
    frame->picture.stBufferInfo.stChromaLocation[BAVC_Polarity_eBotField].eMpegType= BAVC_MpegType_eMpeg2;
    frame->picture.stBufferInfo.eYCbCrType = BAVC_YCbCrType_e4_2_2;
    frame->picture.stBufferInfo.ePulldown = BXDM_Picture_PullDown_eFrameX1;
    frame->picture.stBufferInfo.eSourceFormat = BXDM_Picture_SourceFormat_eProgressive;
    frame->picture.stBufferInfo.stPixelFormat.ePixelFormat = BPXL_eY08_Cb8_Y18_Cr8;
    frame->picture.stBufferInfo.stPixelFormat.bValid = true;
    frame->picture.stBufferInfo.uiRowStride = decoder->decoder.soft.frameMemory.stride;
    frame->picture.stBufferInfo.bValid = true;
    frame->picture.uiSerialNumber = soft_frame->picture_count;
    frame->picture.stPictureType.eCoding = BXDM_Picture_Coding_eUnknown;
    frame->picture.stPictureType.eSequence = BXDM_Picture_Sequence_eProgressive;
    frame->picture.stPTS.bValid = soft_frame->pts_valid;
    frame->picture.stPTS.uiValue = soft_frame->pts;
    frame->picture.stFrameRate.eType = BXDM_Picture_FrameRateType_eUnknown;
    frame->picture.stFrameRate.bValid = true;
    frame->picture.stAspectRatio.eAspectRatio = BFMT_AspectRatio_eUnknown;
    frame->picture.stAspectRatio.bValid = true;

    return;
}

static bool NEXUS_VideoDecoder_P_Soft_ScanItb(void *context, const NEXUS_Rave_P_ItbEntry *itb)
{
    NEXUS_VideoDecoderHandle decoder = context;
    unsigned type = B_GET_BITS(itb->word[0], 31, 24);
    decoder->decoder.soft.state.scanFrame.itbVisited ++;
    switch(type) {
    case NEXUS_Rave_P_ItbType_ePtsDts:
        decoder->decoder.soft.state.scanFrame.current.pts = itb->word[1];
        decoder->decoder.soft.state.scanFrame.current.ptsValid = true;
        BDBG_MSG(("%u PTS %u %p", decoder->decoder.soft.state.scanFrame.itbVisited, (unsigned)decoder->decoder.soft.state.scanFrame.current.pts, (void *)itb));
        break;
    case NEXUS_Rave_P_ItbType_eBaseAddress:
        decoder->decoder.soft.state.scanFrame.current.cdbOffset = itb->word[1];
        decoder->decoder.soft.state.scanFrame.current.cdbOffsetValid = true;
        BDBG_MSG(("%u BASE %#x %p", decoder->decoder.soft.state.scanFrame.itbVisited, (unsigned)decoder->decoder.soft.state.scanFrame.current.cdbOffset, (void *)itb));
        break;
    }
    if(decoder->decoder.soft.state.scanFrame.current.ptsValid && decoder->decoder.soft.state.scanFrame.current.cdbOffsetValid) {
        return true;
    }
    return false;
}

static uint8_t NEXUS_VideoDecoder_P_CdbReadByte(const uint8_t *ptr)
{
    static const uint8_t translate[] = {3,2,1,0};
    unsigned offset = ((unsigned long)ptr)&3;
    ptr -= offset;
    return ptr[translate[offset]];
}

static void NEXUS_VideoDecoder_P_CdbCopy(void *dst_, const void *src_, size_t bytes)
{
    unsigned i;
    uint8_t *dst = dst_;
    const uint8_t *src = src_;

    for(i=0;i<bytes;i++) {
        dst[i] = NEXUS_VideoDecoder_P_CdbReadByte(src + i);
    }
    return;
}

#define B_BCMV_HEADER_LEN   10

static void NEXUS_VideoDecoder_P_Soft_ScanFrame(NEXUS_VideoDecoderHandle decoder)
{
    NEXUS_Error rc;
    size_t frameSize[2];

    decoder->decoder.soft.state.scanFrame.itbVisited = 0;
    LOCK_TRANSPORT();
    rc = NEXUS_Rave_ScanItb_priv(decoder->rave, NEXUS_VideoDecoder_P_Soft_ScanItb, decoder);
    if(rc!=NEXUS_SUCCESS) {
        goto done;
    }
    frameSize[0] = 0;
    frameSize[1] = 0;
    if(decoder->decoder.soft.state.scanFrame.current.ptsValid && decoder->decoder.soft.state.scanFrame.current.cdbOffsetValid) {

        if(decoder->decoder.soft.state.scanFrame.prev.cdbOffsetValid) {
            void *framePtr[2];

            BDBG_MSG(("frame %#x..%#x (%d) PTS: %u", (unsigned)decoder->decoder.soft.state.scanFrame.prev.cdbOffset, (unsigned)decoder->decoder.soft.state.scanFrame.current.cdbOffset, decoder->decoder.soft.state.scanFrame.current.cdbOffset - decoder->decoder.soft.state.scanFrame.prev.cdbOffset, decoder->decoder.soft.state.scanFrame.prev.pts));
            framePtr[1] = NULL;
            framePtr[0] = NEXUS_OffsetToCachedAddr(decoder->decoder.soft.state.scanFrame.prev.cdbOffset);
            if(framePtr[0]==NULL) {goto err_bad_frame;}
            if(decoder->decoder.soft.state.scanFrame.current.cdbOffset >= decoder->decoder.soft.state.scanFrame.prev.cdbOffset) {
                frameSize[0] = decoder->decoder.soft.state.scanFrame.current.cdbOffset - decoder->decoder.soft.state.scanFrame.prev.cdbOffset;
            } else {
                BXPT_Rave_ContextPtrs ctxPtrs;
                void *end_of_frame = NEXUS_OffsetToCachedAddr(decoder->decoder.soft.state.scanFrame.current.cdbOffset);

                if(end_of_frame==NULL) {goto err_bad_frame;}
                rc = NEXUS_Rave_CheckBuffer_priv(decoder->rave, &ctxPtrs);
                if( !(rc == NEXUS_SUCCESS &&
                      (uint8_t *)ctxPtrs.Cdb.DataPtr <= (uint8_t *)framePtr[0] &&
                      (uint8_t *)ctxPtrs.Cdb.DataPtr + ctxPtrs.Cdb.ByteCount > (uint8_t *)framePtr[0] &&
                      ctxPtrs.Cdb.WrapDataPtr &&
                      (uint8_t *)ctxPtrs.Cdb.WrapDataPtr <= (uint8_t *)end_of_frame &&
                      (uint8_t *)ctxPtrs.Cdb.WrapDataPtr + ctxPtrs.Cdb.WrapByteCount > (uint8_t *)end_of_frame
                      )
                    ) {
                    BDBG_ERR(("bad frame rc=%#x (%p,%p)", rc, (void *)ctxPtrs.Cdb.DataPtr, (void *)framePtr[0]) );
                    goto err_bad_frame;
                }
                frameSize[0] = (ctxPtrs.Cdb.DataPtr + ctxPtrs.Cdb.ByteCount) - (uint8_t *)framePtr[0];
                frameSize[1] = (uint8_t *)end_of_frame - ctxPtrs.Cdb.WrapDataPtr;
                framePtr[1] = ctxPtrs.Cdb.WrapDataPtr;
            }
            BDBG_MSG(("frame %p:%u %p:%u %u", (void *)framePtr[0], (unsigned)frameSize[0], (void *)framePtr[1], (unsigned)frameSize[1], (unsigned)decoder->decoder.soft.state.scanFrame.prev.pts));
            if(frameSize[0]+frameSize[1] >= decoder->decoder.soft.state.scanFrame.compressedFrame->data_size) {
                void *new_comressed_frame;
                size_t new_data_size = frameSize[0] + frameSize[1];

                new_data_size += new_data_size/4;
                BDBG_WRN(("jumbo frame %u (%u)", frameSize[0]+frameSize[1], decoder->decoder.soft.state.scanFrame.compressedFrame->data_size));
                new_comressed_frame = BKNI_Malloc( sizeof(*decoder->decoder.soft.state.scanFrame.compressedFrame) - sizeof(decoder->decoder.soft.state.scanFrame.compressedFrame->data) + new_data_size);
                if(new_comressed_frame==NULL) {
                    (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                    goto err_bad_frame;
                }
                BKNI_Memcpy(new_comressed_frame, decoder->decoder.soft.state.scanFrame.compressedFrame, sizeof(*decoder->decoder.soft.state.scanFrame.compressedFrame) - sizeof(decoder->decoder.soft.state.scanFrame.compressedFrame->data)); /* copy state, but don't copy old frame itself */
                BKNI_Free(decoder->decoder.soft.state.scanFrame.compressedFrame);
                decoder->decoder.soft.state.scanFrame.compressedFrame = new_comressed_frame;
                decoder->decoder.soft.state.scanFrame.compressedFrame->data_size = new_data_size;
            }
            NEXUS_FlushCache(framePtr[0], frameSize[0]);
            NEXUS_VideoDecoder_P_CdbCopy(decoder->decoder.soft.state.scanFrame.compressedFrame->data, framePtr[0], frameSize[0]);
            if(frameSize[1]) {
                NEXUS_FlushCache(framePtr[1], frameSize[1]);
                NEXUS_VideoDecoder_P_CdbCopy(decoder->decoder.soft.state.scanFrame.compressedFrame->data + frameSize[0], framePtr[1], frameSize[1]);
            }
            decoder->decoder.soft.state.scanFrame.compressedFrame->length = frameSize[0] + frameSize[1];
            decoder->decoder.soft.state.scanFrame.compressedFrame->ptsValid = decoder->decoder.soft.state.scanFrame.prev.ptsValid;
            decoder->decoder.soft.state.scanFrame.compressedFrame->pts = decoder->decoder.soft.state.scanFrame.prev.pts;
            decoder->decoder.soft.state.scanFrame.compressedFrame->valid = true;
#if 0
            batom_range_dump(decoder->decoder.soft.state.scanFrame.compressedFrame->data, decoder->decoder.soft.state.scanFrame.compressedFrame->length, "frame");
#endif
        }
err_bad_frame:
        decoder->decoder.soft.state.scanFrame.prev = decoder->decoder.soft.state.scanFrame.current;
        decoder->decoder.soft.state.scanFrame.current.ptsValid = false;
        decoder->decoder.soft.state.scanFrame.current.cdbOffsetValid = false;
    }
    if(decoder->decoder.soft.state.scanFrame.itbVisited) {
        NEXUS_Rave_UpdateReadOffset_priv(decoder->rave, frameSize[0]+frameSize[1], decoder->decoder.soft.state.scanFrame.itbVisited * NEXUS_RAVE_P_ITB_SIZE);
    }
done:
    UNLOCK_TRANSPORT();
    return;
}

#include <pthread.h>
static void NEXUS_VideoDecoder_P_Soft_DecoderThread(void *context)
{
    NEXUS_VideoDecoderHandle decoder = context;
    int policy;
    struct sched_param param;

    if(pthread_getschedparam(pthread_self(), &policy, &param)==0) {
        param.sched_priority = 10;
        pthread_setschedparam(pthread_self(),SCHED_FIFO, &param);
    }
    BDBG_MSG(("NEXUS_VideoDecoder_P_Soft_DecoderThread:%#x", (unsigned)decoder));
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);
    for(;;) {
        bool wait = true;
        NEXUS_VideoDecoder_P_FrameElement *frame;
        BKNI_AcquireMutex(decoder->decoder.soft.lock);
        if(decoder->decoder.soft.paused) {
            BKNI_ReleaseMutex(decoder->decoder.soft.lock);
            if(decoder->decoder.soft.threadExit) {
                break;
            }
            goto wait;
        }
        BKNI_ReleaseMutex(decoder->decoder.soft.lock);

        BKNI_EnterCriticalSection();
        frame = BLST_SQ_FIRST(&decoder->decoder.soft.free);
        if(frame) {
            BLST_SQ_REMOVE_HEAD(&decoder->decoder.soft.free, link);
        }
        BKNI_LeaveCriticalSection();
        if(frame==NULL) {
            goto wait;
        }
        BKNI_AcquireMutex(decoder->decoder.soft.lock);
        if(decoder->decoder.soft.paused) {
            BKNI_ReleaseMutex(decoder->decoder.soft.lock);
            goto wait;
        }
        decoder->decoder.soft.state.scanFrame.compressedFrame->valid = false;
        NEXUS_VideoDecoder_P_Soft_ScanFrame(decoder);
        BKNI_ReleaseMutex(decoder->decoder.soft.lock);
        if( !decoder->decoder.soft.state.scanFrame.compressedFrame->valid || decoder->decoder.soft.state.scanFrame.compressedFrame->length<B_BCMV_HEADER_LEN) {
            goto recycle_and_wait;
        }
        {
            b_avcodec_frame soft_frame;
            unsigned cdb_size = decoder->decoder.soft.state.scanFrame.compressedFrame->length-B_BCMV_HEADER_LEN;
            const void *cdb_data = decoder->decoder.soft.state.scanFrame.compressedFrame->data+B_BCMV_HEADER_LEN;
            NEXUS_Error rc;
            b_avcodec_compressed_frame compressed;
            bool resize;
#if NEXUS_VIDEO_DECODER_SOFT_VERIFY_DATA
            unsigned frame_size;
            static uint8_t frame_data[256 * 1024];
            int read_rc;

            read_rc = fread(&frame_size, sizeof(frame_size), 1, decoder->decoder.soft.data.fin);
            if(read_rc<0) { goto recycle_and_wait;}
            BDBG_MSG(("NEXUS_VideoDecoder_P_Soft_DecoderThread:%#x frame_size:%u cdb_size:%u pts:%u", (unsigned)decoder, frame_size, (unsigned)cdb_size, (unsigned)decoder->decoder.soft.state.scanFrame.compressedFrame->pts));
            BDBG_ASSERT(frame_size<=sizeof(frame_data));
            read_rc = fread(frame_data, 1, frame_size, decoder->decoder.soft.data.fin);
            if(read_rc!=(int)frame_size) {
                goto wait;
            }
            if(frame_size!=cdb_size) {
                BDBG_LOG(("frame_size=%u cdb_size=%u", frame_size, cdb_size));
                /* BDBG_ASSERT(0); */
            }
            if(BKNI_Memcmp(frame_data,cdb_data,cdb_size)!=0) {
                unsigned i;
                for(i=0;i<cdb_size;i++) {
                    if(((uint8_t *)frame_data)[i] != ((uint8_t *)cdb_data)[i]) {
                        BDBG_LOG(("%#x: %#x %#x", i, ((uint8_t *)frame_data)[i] != ((uint8_t *)cdb_data)[i]));
                        break;
                    }
                }
                batom_range_dump(frame_data, frame_size, "frame_data");
                batom_range_dump(cdb_data, cdb_size, "cdb_data");
                BDBG_ASSERT(0);
            }
#endif
            compressed.buf = cdb_data;
            compressed.size = cdb_size;
            compressed.pts_valid = decoder->decoder.soft.state.scanFrame.compressedFrame->ptsValid;
            compressed.pts = decoder->decoder.soft.state.scanFrame.compressedFrame->pts;

            BKNI_AcquireMutex(decoder->decoder.soft.lock);
            if(decoder->decoder.soft.paused) {
                BKNI_ReleaseMutex(decoder->decoder.soft.lock);
                goto wait;
            }
            rc = decoder->decoder.soft.soft_decode->methods->decode(decoder->decoder.soft.soft_decode, &compressed,  &soft_frame);
            BKNI_ReleaseMutex(decoder->decoder.soft.lock);
#if 0
            if(decoder->decoder.soft.state.scanFrame.compressedFrame->pts > 3833325) {
            BDBG_LOG(("NEXUS_VideoDecoder_P_Soft_DecoderThread:%#x cdb_size:%u pts:%u", (unsigned)decoder, cdb_size, (unsigned)decoder->decoder.soft.state.scanFrame.compressedFrame->pts));
            batom_range_dump(cdb_data, cdb_size, "cdb");
            }
#endif
            if(rc==NEXUS_NOT_AVAILABLE) {
                goto recycle;
            }
            if(rc!=NEXUS_SUCCESS) {
                rc=BERR_TRACE(rc);
#if 0
                BDBG_LOG(("NEXUS_VideoDecoder_P_Soft_DecoderThread:%#x cdb_size:%u pts:%u", (unsigned)decoder, cdb_size, (unsigned)decoder->decoder.soft.state.scanFrame.compressedFrame->pts));
                batom_range_dump(cdb_data, cdb_size, "cdb");
                if(cdb_size > 64) {
                    batom_range_dump((uint8_t *)cdb_data+(cdb_size-64), 64, "tail");
                }
#endif
                goto recycle;
            }

            BKNI_AcquireMutex(decoder->decoder.soft.lock);
            if(decoder->decoder.soft.paused) {
                BKNI_ReleaseMutex(decoder->decoder.soft.lock);
                goto wait;
            }
            resize = soft_frame.width != decoder->decoder.soft.frameMemory.width || soft_frame.height != decoder->decoder.soft.frameMemory.height;
            if(decoder->decoder.soft.frameMemory.valid && resize) {
                BDBG_WRN(("reallocating frame buffers %ux%u (%ux%u)", soft_frame.width, soft_frame.height, decoder->decoder.soft.frameMemory.width, decoder->decoder.soft.frameMemory.height));
            }
            if(!decoder->decoder.soft.frameMemory.valid || resize) {
                unsigned max_width = soft_frame.width;
                unsigned max_height = soft_frame.height;
                if(max_width<1280) {
                    max_width = 1280;
                }
                if(max_height<720) {
                    max_height = 720;
                }
                rc = NEXUS_VideoDecoder_P_Soft_SetFrames(decoder, soft_frame.width, soft_frame.height, max_width, max_height);
                if(rc!=BERR_SUCCESS) {
                    rc=BERR_TRACE(rc);
                    BKNI_ReleaseMutex(decoder->decoder.soft.lock);
                    goto recycle_and_wait;
                }
            }
#if 0
            BDBG_MSG(("NEXUS_VideoDecoder_P_Soft_DecoderThread:%#x %u frame:%uX%u %#x", (unsigned)decoder, frame - decoder->decoder.soft.frames, soft_frame.width, soft_frame.height, frame->picture.stBufferInfo.pLumaBufferVirtualAddress));
#endif
            NEXUS_VideoDecoder_P_ConvertFrame(decoder, frame, &soft_frame);
            BKNI_ReleaseMutex(decoder->decoder.soft.lock);
            BKNI_EnterCriticalSection();
            NEXUS_VideoDecoder_P_FrameQueue_Add_isr(&decoder->decoder.soft.ready, frame);
            BKNI_LeaveCriticalSection();
            continue;
        }
recycle:
        wait = false;
recycle_and_wait:
        BKNI_EnterCriticalSection();
        BLST_SQ_INSERT_HEAD(&decoder->decoder.soft.free, frame, link);
        BKNI_LeaveCriticalSection();
        if(!wait && !decoder->decoder.soft.paused) { continue; }
wait:
        BKNI_WaitForEvent(decoder->decoder.soft.event, 100);
        continue;
    }
    return;
}

static BERR_Code NEXUS_VideoDecoder_P_Soft_GetPictureCount_isr(void *context, uint32_t *pictureCount)
{
    NEXUS_VideoDecoderHandle decoder = context;
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Soft_GetPictureCount_isr>:%#x", (unsigned)decoder));
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);
    *pictureCount = decoder->decoder.soft.ready.count;
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Soft_GetPictureCount_isr<:%#x %u", (unsigned)decoder, *pictureCount));
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_VideoDecoder_P_Soft_PeekAtPicture_isr(void *context, uint32_t index, const BXDM_Picture **pUnifiedPicture)
{
    NEXUS_VideoDecoderHandle decoder = context;
    const NEXUS_VideoDecoder_P_FrameElement *frame;
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Soft_PeekAtPicture_isr:%#x %u", (unsigned)decoder, index));
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);
    for(frame=BLST_SQ_FIRST(&decoder->decoder.soft.ready.list);frame && index>0;index--) {
        frame = BLST_SQ_NEXT(frame, link);
    }
    if(frame==NULL) {
        *pUnifiedPicture = NULL;
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    *pUnifiedPicture = &frame->picture;
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_VideoDecoder_P_Soft_GetNextPicture_isr(void *context, const BXDM_Picture **pUnifiedPicture)
{
    NEXUS_VideoDecoderHandle decoder = context;
    NEXUS_VideoDecoder_P_FrameElement *frame;
    bool recycled=false;
    bool free_queue_empty;


    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Soft_GetNextPicture_isr>:%#x", (unsigned)decoder));
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);
    free_queue_empty = BLST_SQ_FIRST(&decoder->decoder.soft.free);
    while(NULL!=(frame=BLST_SQ_FIRST(&decoder->decoder.soft.recycled))) {
        BLST_SQ_REMOVE_HEAD(&decoder->decoder.soft.recycled, link);
        BLST_SQ_INSERT_TAIL(&decoder->decoder.soft.free, frame, link);
        recycled = true;
    }
    if(free_queue_empty && recycled) {
        BKNI_SetEvent_isr(decoder->decoder.soft.event);
    }
    frame=BLST_SQ_FIRST(&decoder->decoder.soft.ready.list);
    if(frame==NULL) {
        *pUnifiedPicture = NULL;
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    NEXUS_VideoDecoder_P_FrameQueue_Remove_isr(&decoder->decoder.soft.ready, frame);
    BLST_SQ_INSERT_TAIL(&decoder->decoder.soft.display, frame, link);
    *pUnifiedPicture = &frame->picture;
#if 0
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Soft_GetNextPicture_isr<:%#x %#x %#x[%u]", (unsigned)decoder, (unsigned)frame, (unsigned)frame->picture.stBufferInfo.pLumaBufferVirtualAddress, frame - decoder->decoder.soft.frames));
#endif
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_VideoDecoder_P_Soft_ReleasePicture_isr(void *context, const BXDM_Picture *pUnifiedPicture, const BXDM_Decoder_ReleasePictureInfo *pReleasePictureInfo)
{
    NEXUS_VideoDecoderHandle decoder = context;
    NEXUS_VideoDecoder_P_FrameElement *frame = (void *)pUnifiedPicture;
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Soft_ReleasePicture_isr>:%#x %#x %#x", (unsigned)decoder, (unsigned)pUnifiedPicture, (unsigned)pReleasePictureInfo));
    BSTD_UNUSED(pReleasePictureInfo);
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);
    BLST_SQ_REMOVE(&decoder->decoder.soft.display, frame, NEXUS_VideoDecoder_P_FrameElement, link);
    BLST_SQ_INSERT_TAIL(&decoder->decoder.soft.recycled, frame, link);
    BDBG_MSG_TRACE(("NEXUS_VideoDecoder_P_Soft_ReleasePicture_isr<:%#x %#x %#x", (unsigned)decoder, (unsigned)pUnifiedPicture, (unsigned)pReleasePictureInfo));
    return BERR_SUCCESS;
}

static const BXDM_Decoder_Interface NEXUS_VideoDecoder_P_Soft_DecoderInterface = {
    NEXUS_VideoDecoder_P_Soft_GetPictureCount_isr,
    NEXUS_VideoDecoder_P_Soft_PeekAtPicture_isr,
    NEXUS_VideoDecoder_P_Soft_GetNextPicture_isr,
    NEXUS_VideoDecoder_P_Soft_ReleasePicture_isr,
    NULL,
    NULL,
    NULL,
    NULL
};

static void NEXUS_VideoDecoder_P_Soft_CheckPointCallback(void *data, int unused)
{
    NEXUS_VideoDecoderHandle decoder = data;
    BSTD_UNUSED(unused);
    BKNI_SetEvent(decoder->decoder.soft.checkpointEvent);
    return;
}

NEXUS_VideoDecoderHandle
NEXUS_VideoDecoder_P_Open_Soft( unsigned index, const NEXUS_VideoDecoderOpenSettings *pOpenSettings)
{
    NEXUS_VideoDecoderHandle decoder;
    NEXUS_VideoDecoderOpenMosaicSettings mosaicSettings;
    NEXUS_Error rc;
    NEXUS_RaveOpenSettings raveSettings;
    unsigned i;
    NEXUS_ThreadSettings threadSettings;
    NEXUS_Graphics2DSettings graphicsSettings;

    BDBG_MSG(("NEXUS_VideoDecoder_P_Open_Soft:%u", index));

    BSTD_UNUSED(index);
    decoder = BKNI_Malloc(sizeof(*decoder));
    if(!decoder) { (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BKNI_Memset(decoder, 0, sizeof(*decoder));
    BDBG_OBJECT_SET(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);
    decoder->intf = &NEXUS_VideoDecoder_P_Interface_Soft;
    decoder->decoder.soft.state.scanFrame.compressedFrame = BKNI_Malloc(sizeof(*decoder->decoder.soft.state.scanFrame.compressedFrame));
    if(decoder->decoder.soft.state.scanFrame.compressedFrame==NULL) { (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error; }
    decoder->decoder.soft.state.scanFrame.compressedFrame->data_size = sizeof(decoder->decoder.soft.state.scanFrame.compressedFrame->data);

    NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(&mosaicSettings);
    if (pOpenSettings) {
        mosaicSettings.openSettings = *pOpenSettings;
    }
    LOCK_TRANSPORT();
    NEXUS_Rave_GetDefaultOpenSettings_priv(&raveSettings);
    UNLOCK_TRANSPORT();
    raveSettings.config.Cdb.Length = (3*1024*1024)/2;
    raveSettings.config.Itb.Length = (512*1024);

    rc = NEXUS_VideoDecoder_P_Init_Generic(decoder, &raveSettings, &mosaicSettings);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    rc = BKNI_CreateMutex(&decoder->decoder.soft.lock);
    if(rc!=BERR_SUCCESS){(void)BERR_TRACE(rc);goto error;}
    rc = BKNI_CreateMutex(&decoder->decoder.soft.memoryLock);
    if(rc!=BERR_SUCCESS){(void)BERR_TRACE(rc);goto error;}
    rc = BKNI_CreateEvent(&decoder->decoder.soft.event);
    if(rc!=BERR_SUCCESS){(void)BERR_TRACE(rc);goto error;}

    rc = BKNI_CreateEvent(&decoder->decoder.soft.checkpointEvent);
    if(rc!=BERR_SUCCESS){(void)BERR_TRACE(rc);goto error;}
    decoder->decoder.soft.graphics = NEXUS_Graphics2D_Open(0, NULL);
    if(decoder->decoder.soft.graphics==NULL) {(void)BERR_TRACE(rc);goto error;}

    NEXUS_Graphics2D_GetSettings(decoder->decoder.soft.graphics, &graphicsSettings);
    graphicsSettings.checkpointCallback.callback = NEXUS_VideoDecoder_P_Soft_CheckPointCallback;
    graphicsSettings.checkpointCallback.context = decoder;
    NEXUS_Graphics2D_SetSettings(decoder->decoder.soft.graphics, &graphicsSettings);

    decoder->decoder.soft.paused = true;
    decoder->decoder.soft.threadExit = false;
    decoder->decoder.soft.poweredUp = false;
    BKNI_Memset(decoder->settings.supportedCodecs, true, sizeof(decoder->settings.supportedCodecs));
#if NEXUS_VIDEO_DECODER_LIBVPX
    decoder->decoder.soft.soft_decode = bvpx_video_decode_create(&NEXUS_VideoDecoder_P_Memory_Methods, decoder);
#else
    decoder->decoder.soft.soft_decode = bffmpeg_video_decode_create(&NEXUS_VideoDecoder_P_Memory_Methods, decoder);
#endif
    if(!decoder->decoder.soft.soft_decode) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;}

    NEXUS_VideoDecoder_P_Soft_InitFrames(decoder);

    rc = NEXUS_VideoDecoder_P_Xdm_Initialize(decoder, &NEXUS_VideoDecoder_P_Soft_DecoderInterface, decoder);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    BLST_SQ_INIT(&decoder->decoder.soft.display);
    BLST_SQ_INIT(&decoder->decoder.soft.recycled);
    BLST_SQ_INIT(&decoder->decoder.soft.free);
    NEXUS_VideoDecoder_P_FrameQueue_Init(&decoder->decoder.soft.ready);

    for(i=0;i<NEXUS_VIDEO_DECODER_P_SOFTDECODE_N_FRAMES;i++) {
        BLST_SQ_INSERT_TAIL(&decoder->decoder.soft.free, &decoder->decoder.soft.frames[i], link);
    }

    for(i=0;i<NEXUS_VIDEODECODER_SOFT_MEMORYBLOCK_MAX;i++) {
        decoder->decoder.soft.state.memoryBlocks[i].block = NULL;
    }

    NEXUS_Thread_GetDefaultSettings(&threadSettings);
    decoder->decoder.soft.thread = NEXUS_Thread_Create("nx_soft_video_decode", NEXUS_VideoDecoder_P_Soft_DecoderThread, decoder, &threadSettings);
    if(!decoder->decoder.soft.thread) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;}

    return decoder;
error:
    NEXUS_VideoDecoder_Close(decoder);
err_alloc:
    return NULL;
}

static void
NEXUS_VideoDecoder_P_Close_Soft( NEXUS_VideoDecoderHandle decoder)
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);
    NEXUS_VideoDecoder_P_Close_Generic(decoder);
    if(decoder->decoder.soft.thread) {
        BKNI_AcquireMutex(decoder->decoder.soft.lock);
        decoder->decoder.soft.paused = true;
        decoder->decoder.soft.threadExit = true;
        BKNI_SetEvent(decoder->decoder.soft.event);
        BKNI_ReleaseMutex(decoder->decoder.soft.lock);
        NEXUS_Thread_Destroy(decoder->decoder.soft.thread);
    }

    NEXUS_VideoDecoder_P_Xdm_Shutdown(decoder);

    if(decoder->decoder.soft.soft_decode) {
        decoder->decoder.soft.soft_decode->methods->destroy(decoder->decoder.soft.soft_decode);
    }

    NEXUS_VideoDecoder_P_Soft_Memory_Release(decoder);

    NEXUS_VideoDecoder_P_Soft_FreeFrames(decoder);
    if(decoder->decoder.soft.lock) {
        BKNI_DestroyMutex(decoder->decoder.soft.lock);
    }
    if(decoder->decoder.soft.memoryLock) {
        BKNI_DestroyMutex(decoder->decoder.soft.memoryLock);
    }
    if(decoder->decoder.soft.event) {
        BKNI_DestroyEvent(decoder->decoder.soft.event);
    }
    if(decoder->decoder.soft.checkpointEvent) {
        BKNI_DestroyEvent(decoder->decoder.soft.checkpointEvent);
    }
    if(decoder->decoder.soft.graphics) {
        NEXUS_Graphics2D_Close(decoder->decoder.soft.graphics);
    }
    if(decoder->decoder.soft.state.scanFrame.compressedFrame) {
        BKNI_Free(decoder->decoder.soft.state.scanFrame.compressedFrame);
    }
    BDBG_OBJECT_DESTROY(decoder, NEXUS_VideoDecoder);
    BKNI_Free(decoder);
    return;
}

NEXUS_Error
NEXUS_VideoDecoderModule_P_Init_Soft(const NEXUS_VideoDecoderModuleSettings *pSettings)
{
    if(pSettings->audio==NULL) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    return BERR_SUCCESS;
}

void
NEXUS_VideoDecoderModule_P_Uninit_Soft(void)
{
    return;
}

static void NEXUS_VideoDecoder_GetSourceId_priv_Soft(NEXUS_VideoDecoderHandle decoder, BAVC_SourceId *pSource)
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);
    NEXUS_ASSERT_MODULE();
    *pSource = BAVC_SourceId_eMpeg0;
    return;
}

static void NEXUS_VideoDecoder_GetDisplayConnection_priv_Soft(NEXUS_VideoDecoderHandle decoder, NEXUS_VideoDecoderDisplayConnection *pConnection)
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);
    NEXUS_ASSERT_MODULE();
    *pConnection = decoder->displayConnection;
}

static NEXUS_Error NEXUS_VideoDecoder_P_GetStatus_Soft( NEXUS_VideoDecoderHandle decoder, NEXUS_VideoDecoderStatus *pStatus)
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);
    NEXUS_VideoDecoder_P_GetStatus_Generic(decoder, pStatus);
    pStatus->queueDepth = decoder->decoder.soft.ready.count;
    return NEXUS_VideoDecoder_P_GetStatus_Generic_Xdm(decoder, pStatus);
}


static NEXUS_Error NEXUS_VideoDecoder_P_SetTrickState_Soft( NEXUS_VideoDecoderHandle decoder, const NEXUS_VideoDecoderTrickState *pState)
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&decoder->decoder.soft, NEXUS_VideoDecoder_Soft);

    decoder->trickState = *pState;
    return BERR_SUCCESS;
}

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

static NEXUS_Error NEXUS_VideoDecoder_P_SetPowerState_Soft(NEXUS_VideoDecoderHandle videoDecoder, bool powerUp)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_OBJECT_ASSERT(&videoDecoder->decoder.soft, NEXUS_VideoDecoder_Soft);

    if (powerUp && !videoDecoder->decoder.soft.poweredUp)
    {
#if defined BCHP_PWR_RESOURCE_AVD0_PWR
        BCHP_PWR_AcquireResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_AVD0_PWR);
#endif
#if defined BCHP_PWR_RESOURCE_AVD0_CLK
        BCHP_PWR_AcquireResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_AVD0_CLK);
#endif
        videoDecoder->decoder.soft.poweredUp = true;
    }
    else if ( !powerUp && videoDecoder->decoder.soft.poweredUp )
    {
        /* if started, we can automatically stop. */
        if (videoDecoder->started) {
            NEXUS_VideoDecoder_Stop(videoDecoder);
        }

        /* if we're connected, we must fail. destroying a connected XVD channel will cause VDC window timeouts.
        the VDC window must be destroyed using NEXUS_VideoWindow_RemoveInput. */
        if (videoDecoder->displayConnection.dataReadyCallback_isr) {
            BDBG_ERR(("You must call NEXUS_VideoWindow_RemoveInput before NEXUS_VideoDecoder_SetPowerState(false)."));
            return BERR_TRACE(NEXUS_UNKNOWN);
        }

#if defined BCHP_PWR_RESOURCE_AVD0_CLK
        BCHP_PWR_ReleaseResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_AVD0_CLK);
#endif
#if defined BCHP_PWR_RESOURCE_AVD0_PWR
        BCHP_PWR_ReleaseResource(g_pCoreHandles->chp, BCHP_PWR_RESOURCE_AVD0_PWR);
#endif
        videoDecoder->decoder.soft.poweredUp = false;
    }
    return NEXUS_SUCCESS;
}



static const NEXUS_VideoDecoder_P_Interface NEXUS_VideoDecoder_P_Interface_Soft = {
    NEXUS_VideoDecoder_P_Close_Soft,
    NEXUS_VideoDecoder_P_GetOpenSettings_Common,
    NEXUS_VideoDecoder_P_GetSettings_Common,
    NEXUS_VideoDecoder_P_SetSettings_Soft,
    NEXUS_VideoDecoder_P_Start_Soft,
    NEXUS_VideoDecoder_P_Stop_Soft,
    NEXUS_VideoDecoder_P_Flush_Soft,
    NEXUS_VideoDecoder_P_GetStatus_Soft,
    NEXUS_VideoDecoder_P_GetConnector_Common,
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_GetStreamInformation_Soft),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_SetStartPts_Soft),
    NEXUS_VideoDecoder_P_IsCodecSupported_Generic,
    NEXUS_VideoDecoder_P_SetPowerState_Soft,
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_Reset_Soft),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_GetExtendedStatus_Soft),
    NEXUS_VideoDecoder_P_GetExtendedSettings_NotImplemented,
    NEXUS_VideoDecoder_P_SetExtendedSettings_NotImplemented,
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_CreateStripedSurface_Soft),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_DestroyStripedSurface_Soft),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_CreateStripedMosaicSurfaces_Soft),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_DestroyStripedMosaicSurfaces_Soft),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_GetMostRecentPts_Soft),
    NEXUS_VideoDecoder_P_GetTrickState_Common,
    NEXUS_VideoDecoder_P_SetTrickState_Soft,
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_FrameAdvance_Soft),
    NOT_SUPPORTED(NEXUS_VideoDecoder_P_GetNextPts_Soft),
    NEXUS_VideoDecoder_P_GetPlaybackSettings_Common,
    NEXUS_VideoDecoder_P_SetPlaybackSettings_Common,
    NOT_SUPPORTED(NEXUS_StillDecoder_P_Open_Soft),

#if NEXUS_HAS_ASTM
    NEXUS_VideoDecoder_GetAstmSettings_priv_Common,
    NEXUS_VideoDecoder_SetAstmSettings_priv_Xdm,
    NEXUS_VideoDecoder_GetAstmStatus_Common_isr,
#endif

    NEXUS_VideoDecoder_GetDisplayConnection_priv_Soft,
    NEXUS_VideoDecoder_SetDisplayConnection_priv_Xdm,
    NEXUS_VideoDecoder_GetSourceId_priv_Soft,
    NEXUS_VideoDecoder_GetHeap_priv_Common,
    NEXUS_VideoDecoder_GetSyncSettings_priv_Common,
    NEXUS_VideoDecoder_SetSyncSettings_priv_Xdm,
    NEXUS_VideoDecoder_GetSyncStatus_Common_isr,
    NEXUS_VideoDecoder_UpdateDisplayInformation_priv_Xdm,
    NOT_SUPPORTED(NEXUS_VideoDecoder_GetDecodedFrames_Soft),
    NOT_SUPPORTED(NEXUS_VideoDecoder_ReturnDecodedFrames_Soft)
};
