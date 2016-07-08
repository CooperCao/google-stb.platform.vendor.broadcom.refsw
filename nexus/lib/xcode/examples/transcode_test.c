/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
 *****************************************************************************/
/* xcode lib example app */

#include "nexus_platform.h"
#include "nexus_core_utils.h"
#if NEXUS_HAS_VIDEO_ENCODER
#include "bxcode.h"
#include "b_os_lib.h"
#include "namevalue.h"
#include "media_probe.h"
#include "nexus_timebase.h"
#include "transcode_test.h"
#include "bkni.h"
#include "bdbg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

BDBG_MODULE(transcode_test);

#define INPUT_VPID        0x21
#define INPUT_APID        0x22
#define INPUT_VCODEC      NEXUS_VideoCodec_eMpeg2
#define INPUT_ACODEC      NEXUS_AudioCodec_eMpeg

extern NEXUS_PlatformConfiguration g_platformConfig;

static BXCode_Handle open_transcode(BTST_Transcoder_t  *pContext);
static void close_transcode(BTST_Transcoder_t  *pContext);
static NEXUS_Error start_transcode(BTST_Transcoder_t  *pContext);
static void stop_transcode(BTST_Transcoder_t  *pContext);

static const struct {
    double frequency;
    NEXUS_VideoFrameRate nexusFramerate;
} BTST_P_Verticalfrequency[NEXUS_VideoFrameRate_eMax] = {
   {0, NEXUS_VideoFrameRate_eUnknown},
   {7.493, NEXUS_VideoFrameRate_e7_493},
   {7.5, NEXUS_VideoFrameRate_e7_5},
   {9.99, NEXUS_VideoFrameRate_e9_99},
   {10, NEXUS_VideoFrameRate_e10},
   {12, NEXUS_VideoFrameRate_e12},
   {11.988, NEXUS_VideoFrameRate_e11_988},
   {12.5, NEXUS_VideoFrameRate_e12_5},
   {14.985, NEXUS_VideoFrameRate_e14_985},
   {15, NEXUS_VideoFrameRate_e15},
   {19.98, NEXUS_VideoFrameRate_e19_98},
   {20, NEXUS_VideoFrameRate_e20},
   {23.976, NEXUS_VideoFrameRate_e23_976},
   {24, NEXUS_VideoFrameRate_e24},
   {25, NEXUS_VideoFrameRate_e25},
   {29.970, NEXUS_VideoFrameRate_e29_97},
   {30, NEXUS_VideoFrameRate_e30},
   {50, NEXUS_VideoFrameRate_e50},
   {59.940, NEXUS_VideoFrameRate_e59_94},
   {60, NEXUS_VideoFrameRate_e60},
};

/**
Private functions:
**/
static double getRefreshRateFromFrameRate(NEXUS_VideoFrameRate frameRate)
{
    unsigned i;
    for(i=0;i<sizeof(BTST_P_Verticalfrequency)/sizeof(*BTST_P_Verticalfrequency);i++) {
        if (frameRate == BTST_P_Verticalfrequency[i].nexusFramerate) {
            return BTST_P_Verticalfrequency[i].frequency;
        }
    }
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return 0; /* NEXUS_VideoFrameRate_eUnknown */
}

static void input_endOfFileCallback(void *context, int param)
{
    BTST_Transcoder_t *pContext = (BTST_Transcoder_t  *)context;
    BSTD_UNUSED(param);

    BDBG_WRN(("Context%d endOfFile\n", pContext->id));

    if(!pContext->loop)
    {
        /* terminate the NRT context */
        BKNI_SetEvent(pContext->input.eofEvent);
    }
    return;
}

static void input_endOfFileHandler(void *context)
{
    int i;
    BTST_Transcoder_t  *pContext = (BTST_Transcoder_t  *)context;

    while(BKNI_WaitForEvent(pContext->input.eofEvent, BKNI_INFINITE)!=B_ERROR_SUCCESS);
    BDBG_WRN(("Xcode%d input ending...\n", pContext->id));

    /* stop the NRT context at EOS event */
    B_Mutex_Lock(pContext->mutexStarted);
    if(pContext->started) {
        if(g_testContext.loopbackPlayer && g_testContext.loopbackStarted && (g_testContext.loopbackXcodeId == pContext->id)) {
            fprintf(stderr, "Input EOS to stop xcoder%d loopback...\n", g_testContext.loopbackXcodeId);
            xcode_loopback_shutdown(&g_testContext);
        }
        if(pContext->output.file) {
            shutdown_transcode(pContext);
        }
        BDBG_WRN(("Transcode context%d completes.", pContext->id));
    }
    B_Mutex_Unlock(pContext->mutexStarted);
    if(g_testContext.autoQuit) { /* when all contexts are stopped, quit automatically */
        for(i=0; i<NEXUS_NUM_VIDEO_ENCODERS; i++) {
            if(g_testContext.xcodeContext[i].started) return;
        }
        if(g_testContext.doneEvent) {
            g_keyReturn = 'q';
            BKNI_SetEvent(g_testContext.doneEvent);
            BDBG_WRN(("To exit the key handler"));
        }
    }
    return;
}

static void inputDataReadyCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    BDBG_MSG(("Input[%d] ready!", param));
}

static void outputDataReadyCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    BDBG_MSG(("Output[%u] ready!", param));
}

#if NEXUS_HAS_HDMI_INPUT
static uint8_t SampleEDID[] =
{
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x08, 0x6D, 0x74, 0x22, 0x05, 0x01, 0x11, 0x20,
    0x00, 0x14, 0x01, 0x03, 0x80, 0x00, 0x00, 0x78, 0x0A, 0xDA, 0xFF, 0xA3, 0x58, 0x4A, 0xA2, 0x29,
    0x17, 0x49, 0x4B, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
    0x45, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20,
    0x58, 0x2C, 0x25, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x9E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x42,
    0x43, 0x4D, 0x37, 0x34, 0x32, 0x32, 0x2F, 0x37, 0x34, 0x32, 0x35, 0x0A, 0x00, 0x00, 0x00, 0xFD,
    0x00, 0x17, 0x3D, 0x0F, 0x44, 0x0F, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x89,

    0x02, 0x03, 0x3C, 0x71, 0x7F, 0x03, 0x0C, 0x00, 0x40, 0x00, 0xB8, 0x2D, 0x2F, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xE3, 0x05, 0x1F, 0x01, 0x49, 0x90, 0x05, 0x20, 0x04, 0x03, 0x02, 0x07,
    0x06, 0x01, 0x29, 0x09, 0x07, 0x01, 0x11, 0x07, 0x00, 0x15, 0x07, 0x00, 0x01, 0x1D, 0x00, 0x72,
    0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28, 0x55, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x8C, 0x0A,
    0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x18,
    0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0x0B, 0x88, 0x21, 0x00,
    0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9D
};

static void
avInfoFrameChangedCallback(void *context, int param)
{
    NEXUS_HdmiInputStatus status;
    NEXUS_HdmiInputHandle hdmiInput = (NEXUS_HdmiInputHandle)context;

    BSTD_UNUSED(param);
    NEXUS_HdmiInput_GetStatus(hdmiInput, &status);
    BDBG_WRN(("HDMI input AV InfoFrame Change callback: video format %ux%u@%.3f%c",
        status.avWidth,
        status.avHeight,
        (double)status.vertFreq/100,
        status.interlaced? 'i' : 'p'));
}
#endif

#if NEXUS_HAS_FRONTEND
static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendQamStatus qamStatus;

    BSTD_UNUSED(param);

    fprintf(stderr, "Lock callback, frontend 0x%08x\n", (unsigned)frontend);

    NEXUS_Frontend_GetQamAsyncStatus(frontend, &qamStatus);
    fprintf(stderr, "QAM Lock callback, frontend 0x%08x - lock status %d, %d\n", (unsigned)frontend,
        qamStatus.fecLock, qamStatus.receiverLock);
}
#endif

typedef enum BTPCommand
{
   BTPCommand_eChunkId = 0x0D, /* reuse PICTURE_TAG */
   BTPCommand_eLast = 0x82, /* LAST */
   BTPCommand_eEOS = 0x0A /* protocol agnostic EOS or so-called INLINE_FLUSH or TPD */
} BTPCommand;

static void insertBTP(void *buf, uint32_t chunkId, unsigned pid, BTPCommand command)
{
    int i;
    static uint8_t btp[188] = {
            0x47, 0x1f, 0xff, 0x20,
            0xb7, 0x02, 0x2d, 0x00,
            'B',  'R',  'C',  'M',  /* signature */
            0x00, 0x00, 0x00, 0x0d, /* CHUNK_ID command (reuse PICTURE_TAG) */
            0x00, 0x00, 0x00, 0x00, /* dwod 1 */
            0x00, 0x00, 0x00, 0x00, /* dwod 2 */
            0x00, 0x00, 0x00, 0x00, /* dwod 3 */
            0x00, 0x00, 0x00, 0x00, /* dwod 4 */
            0x00, 0x00, 0x00, 0x00, /* dwod 5 */
            0x00, 0x00, 0x00, 0x00, /* dwod 6 */
            0x00, 0x00, 0x00, 0x00, /* dwod 7 */
            0x00, 0x00, 0x00, 0xbc, /* dwod 8 =188 to avoid RAVE dropping following packets */
            0x00, 0x00, 0x00, 0x00, /* dwod 9: chunkID in big endian */
            /* rest of BTP = 0x00 */
        };

    btp[1] = (pid >> 8) & 0x1f;
    btp[2] = pid & 0xff;  /* PID */

    btp[12 + 3] = command;

    switch(command)
    {
       case BTPCommand_eChunkId:
       case BTPCommand_eLast:
       case BTPCommand_eEOS:
        /* big endian dword[9] for CHUNK_ID BTP command's chunkID payload */
        btp[12+36] = (unsigned char) ((chunkId & 0xff000000) >> 24);
        btp[12+36+1] = (unsigned char) ((chunkId & 0x00ff0000) >> 16);
        btp[12+36+2] = (unsigned char) ((chunkId & 0x0000ff00) >> 8);
        btp[12+36+3] = (unsigned char) (chunkId & 0x000000ff);
        break;
    }
    BKNI_Memcpy(buf,(void *)btp,188);
    BDBG_MSG(("BTP:"));
    for(i=0; i<52; i+=4) {
        BDBG_MSG(("%02x %02x %02x %02x", btp[i], btp[i+1], btp[i+2], btp[i+3]));
    }
}

static void inputFeedThread(BTST_Transcoder_t *pContext)
{
    FILE *fp = fopen(pContext->input.data, "rb");
    if(fp == NULL) { BDBG_ERR(("Input feed thread failed to open input file %s", pContext->input.data)); return; }
    while (g_keyReturn != 'q' && pContext->started) {
        void *buffer;
        size_t buffer_size;
        int n;
        NEXUS_Error rc;

        B_Mutex_Lock(pContext->mutexStarted);
        if(!pContext->started) {
            B_Mutex_Unlock(pContext->mutexStarted);
            goto error;
        }
        rc = BXCode_Input_GetBuffer(pContext->hBxcode, &buffer, &buffer_size);
        B_Mutex_Unlock(pContext->mutexStarted);
        if(rc) break;
        if (buffer_size == 0) {
            BKNI_WaitForEvent(pContext->input.dataReadyEvent, 50);
            continue;
        }

        /* The first call to get_buffer will return the entire playback buffer.
        If we use it, we're going to have to wait until the descriptor is complete,
        and then we're going to underflow. So use a max size. */
#define MAX_READ (188*1024)
        if (buffer_size > MAX_READ)
            buffer_size = MAX_READ;

        n = fread(buffer, 1, buffer_size, fp);
        if (n < 0) goto error;
        if (n == 0) {
            if(!pContext->loop) {
                #define EOS_BTP_PACKETS_SIZE   (188*3)
                if(buffer_size >= EOS_BTP_PACKETS_SIZE) {
                    /* if no looping, insert EOS BTP to terminate transcoder */
                    insertBTP(buffer, 0, INPUT_VPID, BTPCommand_eEOS);
                    buffer = (uint8_t*)buffer + 188; buffer_size -= 188;
                    insertBTP(buffer, 0, INPUT_VPID, BTPCommand_eLast);
                    buffer = (uint8_t*)buffer + 188; buffer_size -= 188;
                    insertBTP(buffer, 0, INPUT_VPID, BTPCommand_eEOS);
                    buffer = (uint8_t*)buffer + 188; buffer_size -= 188;
                    B_Mutex_Lock(pContext->mutexStarted);
                    if(!pContext->started) {
                        B_Mutex_Unlock(pContext->mutexStarted);
                        goto error;
                    }
                    rc = BXCode_Input_WriteComplete(pContext->hBxcode, 0, EOS_BTP_PACKETS_SIZE);
                    B_Mutex_Unlock(pContext->mutexStarted);
                    BDBG_ASSERT(!rc);
                } else { BKNI_Sleep(50); continue; } /* if no space for EOS, continue to try later */
            }
            /* wait for the decoder to reach the end of the content before looping */
            while (pContext->started) {
                BXCode_InputStatus inStatus;
                B_Mutex_Lock(pContext->mutexStarted);
                if(!pContext->started) {
                    B_Mutex_Unlock(pContext->mutexStarted);
                    goto error;
                }
                BXCode_GetInputStatus(pContext->hBxcode, &inStatus);
                B_Mutex_Unlock(pContext->mutexStarted);
                if (!inStatus.playpump.fifoDepth) break;
                BKNI_Sleep(50);
            }
            if(pContext->loop) fseek(fp, 0, SEEK_SET);
            else break;
        }
        else {
            B_Mutex_Lock(pContext->mutexStarted);
            if(!pContext->started) {
                B_Mutex_Unlock(pContext->mutexStarted);
                goto error;
            }
            rc = BXCode_Input_WriteComplete(pContext->hBxcode, 0, n);
            B_Mutex_Unlock(pContext->mutexStarted);
            BDBG_ASSERT(!rc);
            BDBG_MSG(("played %d bytes", n));
        }
    }
error:
    BDBG_WRN(("Input stream feed thread completed!"));
    fclose(fp);
    if(pContext->input.eofEvent) BKNI_SetEvent(pContext->input.eofEvent);
}

/* repeatedly feed same input image to xcode until user quit the test */
static void imageInputFeedThread(BTST_Transcoder_t *pContext)
{
    NEXUS_SurfaceCreateSettings surfaceCfg;
    NEXUS_PlatformConfiguration platformConfig;
    unsigned i, bytesRead=0, loopCnt=0;
    unsigned runningPts,ptsIncrement=1500;
    int vSyncs=1;
    unsigned submitIdx=0,releaseIdx=0;
    NEXUS_Error rc;
    FILE *fp = fopen(pContext->input.data, "rb");
    if(fp == NULL) { BDBG_ERR(("Input feed thread failed to open input file %s", pContext->input.data)); return; }

    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Create surfaces that will be used with VideoImageInput */
    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCfg);
    surfaceCfg.width  = pContext->input.maxWidth;
    surfaceCfg.height = pContext->input.maxHeight;
    /* assume YCrYCb YUV422 image format */
    surfaceCfg.pixelFormat = NEXUS_PixelFormat_eY08_Cr8_Y18_Cb8;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_MemoryStatus s;
        if (!platformConfig.heap[i] || NEXUS_Heap_GetStatus(platformConfig.heap[i], &s)) continue;
        BDBG_MSG(("heap[%u].memc=%u, type=%#x, imageInput.memc=%u", i, s.memcIndex, s.memoryType, pContext->imageInputStatus.memcIndex));
        if (s.memcIndex == pContext->imageInputStatus.memcIndex && (s.memoryType & NEXUS_MemoryType_eApplication) && s.largestFreeBlock >= 960*1080*2) {
            surfaceCfg.heap = platformConfig.heap[i];
            break;
        }
    }
    if (!surfaceCfg.heap) {
        BDBG_ERR(("no heap found. RTS failure likely."));
        return;
    }
    for( i=0; i < BTST_NUM_SURFACES ; i++ ) {
        pContext->surface[i].handle = NEXUS_Surface_Create(&surfaceCfg);
        BDBG_ASSERT(pContext->surface[i].handle);
        pContext->surface[i].submitted = false;
    }

    while (g_keyReturn != 'q' && pContext->started) {
        NEXUS_VideoImageInputSurfaceSettings surfaceSettings;
        size_t num_entries;
        NEXUS_SurfaceHandle freeSurface=NULL, pic;
        NEXUS_SurfaceMemory surfaceMemory;

        num_entries=0;
        /* Make sure image surface is not in use by Video Output (VDC) */
        do {
            if ( pContext->surface[submitIdx].submitted ) {
                /* our queue is all used up, need to wait until VideoImageInput returns */
                /* a surface after it has been displayed                                */
                BDBG_MSG(("surface[submitIdx=%d].submitted in use, wait for recycle" , submitIdx ));
                if(BKNI_WaitForEvent( pContext->input.dataReadyEvent, 50) == BERR_TIMEOUT) continue;
            }

            B_Mutex_Lock(pContext->mutexStarted);
            if(!pContext->started) {
                B_Mutex_Unlock(pContext->mutexStarted);
                goto BTST_P_ImageInput_Stopped;
            }
            rc=NEXUS_VideoImageInput_RecycleSurface( pContext->imageInput, &freeSurface , 1, &num_entries );
            B_Mutex_Unlock(pContext->mutexStarted);
            BDBG_ASSERT(!rc);
            if ( num_entries ) {
                /* our surface has been displayed, we can now re-use and re-queue it */
                BDBG_MSG(("surface[releaseIdx=%d].handle=%p  recycSurface=%p" , releaseIdx, pContext->surface[releaseIdx].handle , freeSurface ));
                BDBG_ASSERT( pContext->surface[releaseIdx].handle == freeSurface );
                pContext->surface[releaseIdx].submitted = false;
                if ( ++releaseIdx == BTST_NUM_SURFACES ) releaseIdx=0;
            }
        } while ( num_entries && pContext->started);


        pContext->surface[submitIdx].submitted = true; /* mark as inuse */
        pic = pContext->surface[submitIdx].handle;
        BDBG_MSG(("pic=%p" , pic ));
        if ( ++submitIdx == BTST_NUM_SURFACES ) submitIdx=0;

        NEXUS_Surface_GetMemory( pic, &surfaceMemory);
        BDBG_MSG(("Start read in frame=%d" , loopCnt ));
        bytesRead = fread((uint8_t *)(surfaceMemory.buffer),1,pContext->input.maxWidth*pContext->input.maxHeight*2,fp);
        if ( !bytesRead ) {
            break;
        }
        fseek(fp, 0, SEEK_SET);/* go back to beginning of file: to repeate coding */
        BDBG_MSG(("bytesRead=%d for frame=%d" , bytesRead, loopCnt ));
        loopCnt++;

        NEXUS_Surface_Flush( pic );

        NEXUS_VideoImageInput_GetDefaultSurfaceSettings( &surfaceSettings );
        surfaceSettings.displayVsyncs = vSyncs; /* x * Vysnc(~16mSec) = number of seconds to display */
        surfaceSettings.pts = runningPts;
        runningPts += ptsIncrement;

        /* Submit surface to VideoImageInput, wait if queue to feed VDC is already full */
        do {
            B_Mutex_Lock(pContext->mutexStarted);
            if(!pContext->started) {
                B_Mutex_Unlock(pContext->mutexStarted);
                goto BTST_P_ImageInput_Stopped;
            }
            rc = NEXUS_VideoImageInput_PushSurface(pContext->imageInput, pic , &surfaceSettings );
            B_Mutex_Unlock(pContext->mutexStarted);
            if(rc==NEXUS_IMAGEINPUT_QUEUE_FULL) {
                if(BKNI_WaitForEvent( pContext->input.dataReadyEvent, 50) == BERR_TIMEOUT) continue;
            }
        } while ( rc && pContext->started);
    }

    B_Mutex_Lock(pContext->mutexStarted);
    if(!pContext->started) {
        B_Mutex_Unlock(pContext->mutexStarted);
        goto BTST_P_ImageInput_Stopped;
    }
    NEXUS_VideoImageInput_SetSurface(pContext->imageInput, NULL);  /* clear out any pics being displayed */
    B_Mutex_Unlock(pContext->mutexStarted);

BTST_P_ImageInput_Stopped:
    for( i=0; i < BTST_NUM_SURFACES ; i++ ) {
        NEXUS_Surface_Destroy(pContext->surface[i].handle);
    }
    BDBG_WRN(("Input stream feed thread completed!"));
    fclose(fp);
}

static void outputRecordThread(BTST_Transcoder_t *pContext)
{
    BXCode_OutputStream output;
    BXCode_OutputStatus status;
    char fname[BTST_FILE_NAME_LEN];
    double duration;
    unsigned count = 0;
    size_t total = 0, i=0, j=0, segment=0;
    FILE *fVdesc = NULL, *fVdata = NULL;
    FILE *fAdata[BXCODE_MAX_AUDIO_PIDS] = {NULL,};
    FILE *fAdesc[BXCODE_MAX_AUDIO_PIDS] = {NULL,};

    /* open output ts file or ves file */
    if(pContext->input.enableVideo || pContext->output.type == BXCode_OutputType_eTs) {
        output.type = BXCode_OutputStreamType_eTs;
        fVdata = fopen(pContext->output.data, "wb");
        if(fVdata == NULL) { BDBG_ERR(("Output record thread failed to open output file %s", pContext->output.data)); return; }
    }
    /* open VES descriptor file */
    if(pContext->input.enableVideo && pContext->output.type == BXCode_OutputType_eEs) {
        output.type = BXCode_OutputStreamType_eVes; /* start with ves record */
        fVdesc = fopen("/data/videos/vdesc.csv", "w");
        if(fVdesc) fprintf(fVdesc, "flags,origPts,pts,stcSnapshot,escr,ticksPerBit,shr,offset,length\n");
    }
    /* open AES data and descriptor file for AES output */
    if(pContext->input.enableAudio && pContext->output.type == BXCode_OutputType_eEs)
    {
        if(!pContext->input.enableVideo) output.type = BXCode_OutputStreamType_eAes; /* audio only ES */
        for(i=0; i<pContext->input.numAudios; i++) {
            fAdata[i] = fopen(pContext->output.audioFiles[i], "wb");
            if(fAdata[i] == NULL) { BDBG_ERR(("Output record thread failed to open output AES file %s", pContext->output.audioFiles[i])); return; }
            sprintf(fname, "/data/videos/adesc%u.csv", i);
            fAdesc[i] = fopen(fname, "w");
            if(fAdesc[i]) fprintf(fAdesc[i], "flags,origPts,pts,stcSnapshot,escr,ticksPerBit,shr,offset,length\n");
        }
    }

    /* get segment duration */
    if(pContext->output.type == BXCode_OutputType_eTs) {
        fVdesc = fopen("/data/videos/output.hls", "w");
        if(fVdesc) fprintf(fVdesc, "segment,fileOffset,duration(sec)\n");

        if(pContext->settings.video.encoder.streamStructure.duration) {
            duration = (double)pContext->settings.video.encoder.streamStructure.duration/1000;
        } else {
            double framerate = getRefreshRateFromFrameRate(pContext->settings.video.encoder.frameRate);
            if(pContext->settings.video.encoder.streamStructure.openGop) {
                duration = (pContext->settings.video.encoder.streamStructure.framesP + 1)*
                    (pContext->settings.video.encoder.streamStructure.framesB + 1)/framerate;
            } else {
                duration = (1 + pContext->settings.video.encoder.streamStructure.framesP *
                    (pContext->settings.video.encoder.streamStructure.framesB + 1))/framerate;
            }
            BDBG_MSG(("framerate=%.1f, duration=%.1f", framerate, duration));
        }
    }

    /* get bxcode output status with buffer base address */
    BXCode_GetOutputStatus(pContext->hBxcode, &status);

    /* TODO: add external av mux! */
    output.id   = 0;
    while (count < 20 && g_keyReturn != 'q' && pContext->started) {
        const void *data_buffer[2];
        size_t data_buffer_size[2], size = 0;
        int n = 0;

        if (BXCode_Output_GetDescriptors(pContext->hBxcode, output, &data_buffer[0], &data_buffer_size[0], &data_buffer[1], &data_buffer_size[1]))
            break;
        if (data_buffer_size[0] == 0 && data_buffer_size[1] == 0) {
            BKNI_WaitForEvent(pContext->output.dataReadyEvent, 50);
            if(output.id == 0) {
                count++;
            }
        }
        count = 0;

        for(j = 0; j < 2; j++) {
            for(i = 0; i < data_buffer_size[j]; i++) {
                if(pContext->output.type == BXCode_OutputType_eTs) {
                    BXCode_OutputTsDescriptor *pDesc = (BXCode_OutputTsDescriptor *)data_buffer[j] + i;
                    if(pDesc->flags & BXCODE_OUTPUTDESCRIPTOR_TSFLAG_SEGMENT_START) {
                        BDBG_MSG(("Segment[%u] starts @%#x", segment, total));
                        segment++;
                        if(fVdesc) fprintf(fVdesc, "%u,%u,%.1f\n", segment, total, duration);
                    }
                    if(pDesc->size) {
                       fwrite(pDesc->pData, 1, pDesc->size, fVdata);
                       if (n < 0) goto error;
                       total += pDesc->size;
                       size += pDesc->size;
                    }
                } else if(output.type == BXCode_OutputStreamType_eVes) {
                    NEXUS_VideoEncoderDescriptor *pDesc = (NEXUS_VideoEncoderDescriptor *)data_buffer[j] + i;
                    if(pDesc->length > 0) {
                        if((pDesc->flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_METADATA) ==0) {/* ignore metadata descriptor in es capture */
                            n = fwrite((const uint8_t *)status.video.pBufferBase + pDesc->offset, pDesc->length, 1, fVdata);
                            total+= pDesc->length;
                            size += pDesc->length;
                        }
                    }
                    if(pDesc->flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_FRAME_START) {/* only record frame start desc */
                        if(fVdesc) fprintf(fVdesc, "%#x,%u,%#x%08x,%u,%u,%u,%d,%#x,%#x\n", pDesc->flags, pDesc->originalPts,
                            (uint32_t)(pDesc->pts>>32), (uint32_t)(pDesc->pts & 0xffffffff), (uint32_t)pDesc->stcSnapshot, pDesc->escr,
                            pDesc->ticksPerBit, pDesc->shr, pDesc->offset, pDesc->length);
                    }
                } else if(output.type == BXCode_OutputStreamType_eAes) {
                    NEXUS_AudioMuxOutputFrame *pDesc = (NEXUS_AudioMuxOutputFrame *)data_buffer[j] + i;
                    if(pDesc->length > 0) {
                        if((pDesc->flags & NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_METADATA) ==0) {/* ignore metadata descriptor in es capture */
                            n = fwrite((const uint8_t *)status.audio[output.id].pBufferBase + pDesc->offset, pDesc->length, 1, fAdata[output.id]);
                            total+= pDesc->length;
                            size += pDesc->length;
                        }
                    }
                    if(fAdesc[output.id]) fprintf(fAdesc[output.id], "%#x,%u,%#x%08x,%u,%u,%u,%d,%#x,%#x\n", pDesc->flags, pDesc->originalPts,
                        (uint32_t)(pDesc->pts>>32), (uint32_t)(pDesc->pts & 0xffffffff), (uint32_t)pDesc->stcSnapshot, pDesc->escr,
                        pDesc->ticksPerBit, pDesc->shr, pDesc->offset, pDesc->length);
                }
            }
        }

        if(data_buffer_size[0] + data_buffer_size[1]) {
            BXCode_Output_ReadComplete(pContext->hBxcode, output, data_buffer_size[0]+data_buffer_size[1]);
            BDBG_MSG(("wrote %#x bytes stream[%u][%u] data, %d+%d descriptors", size, output.type, output.id, data_buffer_size[0], data_buffer_size[1]));
        }

        /* iterate through all audio and video ES outputs */
        if(output.type == BXCode_OutputStreamType_eVes && pContext->input.enableAudio) {
            output.type = BXCode_OutputStreamType_eAes;
            output.id   = 0;
        } else if(output.type == BXCode_OutputStreamType_eAes && pContext->input.enableAudio) {
            if(output.id+1 < pContext->input.numAudios) {
                output.id++;
            } else {
                if(pContext->input.enableVideo) {
                    output.type = BXCode_OutputStreamType_eVes;
                }
                output.id = 0;
            }
        }
    }
error:
    if(fVdata) fclose(fVdata);
    if(fVdesc) fclose(fVdesc);
    if(pContext->input.enableAudio && pContext->output.type == BXCode_OutputType_eEs)
    {
        for(i=0; i<pContext->input.numAudios; i++) {
            fclose(fAdata[i]);
            if(fAdesc[i]) fclose(fAdesc[i]);
        }
    }
    BDBG_WRN(("Output stream record thread completed with %#x bytes!", total));
    g_keyReturn = 'q';
}

static BXCode_Handle open_transcode(BTST_Transcoder_t  *pContext)
{
    BXCode_OpenSettings openSettings;
    NEXUS_VideoEncoderCapabilities capabilities;
    unsigned i, vpipes=0;

    BXCode_GetDefaultOpenSettings(&openSettings);
    if(pContext->vpipes > 1) {/* FNRT */
        NEXUS_GetVideoEncoderCapabilities(&capabilities);
        for(i=0; i<NEXUS_MAX_VIDEO_ENCODERS; i++) {
            if(capabilities.videoEncoder[i].supported) vpipes++;
        }
        if(pContext->vpipes > vpipes) {
            pContext->vpipes = vpipes;
        }
        openSettings.vpipes = pContext->vpipes;
        BDBG_MSG(("vpipes = %u", pContext->vpipes));
    }
    if((openSettings.timebase = NEXUS_Timebase_Open(NEXUS_ANY_ID)) == (NEXUS_Timebase)NULL) {
        BDBG_ERR(("Transcoder[%u] failed to open timebase!", pContext->id));
        return NULL;
    }
    pContext->timebase = openSettings.timebase;
    if(g_testContext.decoderZeroUp) {
        openSettings.videoDecoderId = pContext->id;
    }
    pContext->hBxcode = BXCode_Open(pContext->id, &openSettings);
    g_testContext.activeXcodeCount++;
    return pContext->hBxcode;
}

static void close_transcode(BTST_Transcoder_t  *pContext)
{
    g_testContext.activeXcodeCount--;
    close_gfx(pContext);
    BXCode_Close(pContext->hBxcode);
    NEXUS_Timebase_Close(pContext->timebase);
}

static NEXUS_Error start_transcode(BTST_Transcoder_t  *pContext)
{
#if NEXUS_HAS_FRONTEND
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_ParserBandSettings parserBandSettings;
#endif
#if NEXUS_HAS_HDMI_INPUT
    NEXUS_HdmiInputSettings hdmiInputSettings;
    NEXUS_TimebaseSettings timebaseSettings;
#endif
    NEXUS_VideoFormatInfo formatInfo;
    unsigned i;

    if(pContext->started) {
        BDBG_WRN(("Trasncoder[%u] already started!", pContext->id));
        return NEXUS_SUCCESS;
    }

    /* resolve video encoder format */
    if(pContext->output.format != NEXUS_VideoFormat_eCustom2) {
        NEXUS_VideoFormat_GetInfo(pContext->output.format, &formatInfo);
        pContext->output.videoFormat.width       = formatInfo.digitalWidth;
        pContext->output.videoFormat.height      = formatInfo.digitalHeight;
        pContext->output.videoFormat.interlaced  = formatInfo.interlaced;
        pContext->output.videoFormat.refreshRate = formatInfo.verticalFreq * 10;
        pContext->output.videoFormat.aspectRatio = formatInfo.aspectRatio;
    }
    BXCode_GetSettings(pContext->hBxcode, &pContext->settings);
    pContext->settings.video.width  = pContext->output.videoFormat.width;
    pContext->settings.video.height = pContext->output.videoFormat.height;
    pContext->settings.video.refreshRate = pContext->output.videoFormat.refreshRate;
    pContext->settings.video.encoder.bitrateMax = pContext->output.vBitrate;
    pContext->settings.video.encoder.variableFrameRate = pContext->output.variableFramerate;
    pContext->settings.video.encoder.frameRate = pContext->output.framerate;
    pContext->settings.video.encoder.bitrateTarget = pContext->output.targetBitrate;
    pContext->settings.video.encoder.streamStructure.framesP = pContext->output.gopFramesP;
    pContext->settings.video.encoder.streamStructure.framesB = pContext->output.gopFramesB;
    pContext->settings.video.encoder.streamStructure.openGop = pContext->output.openGop;
    pContext->settings.video.encoder.streamStructure.duration = pContext->output.gopDuration;
    pContext->settings.video.encoder.streamStructure.newGopOnSceneChange = pContext->output.newGopOnSceneChange;
    BXCode_SetSettings(pContext->hBxcode, &pContext->settings);
    BXCode_GetDefaultStartSettings(&pContext->startSettings);
    pContext->startSettings.nonRealTime = pContext->nonRealTime;
    pContext->startSettings.input.type = pContext->input.type;
    if(pContext->input.type == BXCode_InputType_eFile) {
        FILE *fp = fopen(pContext->input.index, "r");
        pContext->startSettings.input.data = pContext->input.data;
        if(fp) {
            fclose(fp);
            pContext->startSettings.input.index = pContext->input.index;
            BDBG_MSG(("Input index file: %s", pContext->startSettings.input.index));
        }
        pContext->startSettings.input.loop = pContext->loop;
        pContext->startSettings.input.eofDone.callback = input_endOfFileCallback;
        pContext->startSettings.input.eofDone.context  = pContext;
    } else if(pContext->input.type == BXCode_InputType_eStream || pContext->input.type == BXCode_InputType_eImage) {
        BKNI_CreateEvent(&pContext->input.dataReadyEvent);
        pContext->startSettings.input.dataCallback.callback = inputDataReadyCallback;
        pContext->startSettings.input.dataCallback.context  = pContext->input.dataReadyEvent;
        pContext->startSettings.input.dataCallback.param    = pContext->id;
    }
    pContext->startSettings.input.transportType  = pContext->input.transportType;
    pContext->startSettings.input.timestampType = pContext->input.transportTimestamp;
    pContext->startSettings.input.vPid = pContext->input.videoPid;
    pContext->startSettings.input.pcrPid = pContext->input.pcrPid;
    pContext->startSettings.input.vCodec = pContext->input.vCodec;
    for(i=0; i<pContext->input.numAudios; i++) {
        pContext->startSettings.input.aPid[i] = pContext->input.audioPid[i];
        pContext->startSettings.input.aCodec[i] = pContext->input.aCodec[i];
    }

    /* ts user data settings */
    pContext->startSettings.input.numTsUserDataPids = pContext->input.numUserDataPids;
    if(pContext->input.tsUserDataInput && ((unsigned)(-1)!=pContext->input.numUserDataPids)) {
        for(i=0; i<pContext->input.numUserDataPids; i++) {
            pContext->startSettings.input.userdataPid[i] = pContext->input.userDataPid[i];
            if(pContext->input.remapUserDataPid) {
                pContext->startSettings.output.transport.userdataPid[i] = pContext->input.remappedUserDataPid[i];
            }
        }
    }

#if NEXUS_HAS_HDMI_INPUT
    if(pContext->input.type == BXCode_InputType_eHdmi) {
        unsigned timebaseId;
        NEXUS_Timebase_GetIndex(pContext->timebase, &timebaseId);
        NEXUS_Timebase_GetSettings(pContext->timebase, &timebaseSettings);
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eHdDviIn;
        NEXUS_Timebase_SetSettings(pContext->timebase, &timebaseSettings);
        BDBG_MSG(("Transcoder[%u] set timebase %u.", pContext->id, timebaseId));

        NEXUS_HdmiInput_GetDefaultSettings(&hdmiInputSettings);
        hdmiInputSettings.timebase = pContext->timebase;
        /* use NEXUS_HdmiInput_OpenWithEdid ()
            if EDID PROM (U1304 and U1305) is NOT installed;
            reference boards usually have the PROMs installed.
            this example assumes Port1 EDID has been removed
        */

        /* all HDMI Tx/Rx combo chips have EDID RAM */
        hdmiInputSettings.useInternalEdid = true ;
        pContext->startSettings.input.hdmiInput = NEXUS_HdmiInput_OpenWithEdid(0, &hdmiInputSettings,
            &SampleEDID[0], (uint16_t) sizeof(SampleEDID));
        if(!pContext->startSettings.input.hdmiInput) {
            BDBG_ERR(("Can't get hdmi input!"));
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        NEXUS_HdmiInput_GetSettings(pContext->startSettings.input.hdmiInput, &hdmiInputSettings);
        hdmiInputSettings.aviInfoFrameChanged.callback = avInfoFrameChangedCallback;
        hdmiInputSettings.aviInfoFrameChanged.context  = pContext->startSettings.input.hdmiInput;
        NEXUS_HdmiInput_SetSettings(pContext->startSettings.input.hdmiInput, &hdmiInputSettings);
    }
#endif
#if NEXUS_HAS_FRONTEND
    if(pContext->input.type == BXCode_InputType_eLive) {
            NEXUS_FrontendCapabilities capabilities;
            BDBG_ASSERT(pContext->id < NEXUS_MAX_FRONTENDS);
            pContext->input.frontend = g_platformConfig.frontend[pContext->id];
            if (pContext->input.frontend) {
                NEXUS_Frontend_GetCapabilities(pContext->input.frontend, &capabilities);
                /* Does this frontend support qam? */
                if ( !capabilities.qam )
                {
                    fprintf(stderr, "This platform doesn't support QAM frontend!\n");
                    return -1;
                }
            }

            NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
            qamSettings.frequency = pContext->input.freq* 1000000;
            qamSettings.mode = pContext->input.qamMode;
            switch (pContext->input.qamMode) {
            case NEXUS_FrontendQamMode_e64: qamSettings.symbolRate = 5056900; break;
            case NEXUS_FrontendQamMode_e256: qamSettings.symbolRate = 5360537; break;
            default: BDBG_ERR(("Unsupported QAM mode!")); return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
            qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
            qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
            qamSettings.lockCallback.callback = lock_callback;
            qamSettings.lockCallback.context = pContext->input.frontend;
            NEXUS_Frontend_GetUserParameters(pContext->input.frontend, &userParams);

            /* Map a parser band to the demod's input band. */
            pContext->startSettings.input.parserBand = NEXUS_ParserBand_e0+pContext->id;
            NEXUS_ParserBand_GetSettings(pContext->startSettings.input.parserBand, &parserBandSettings);
            if (userParams.isMtsif) {
                parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
                parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(pContext->input.frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
            }
            else {
                parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
                parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
            }
            parserBandSettings.transportType = NEXUS_TransportType_eTs;
            NEXUS_ParserBand_SetSettings(pContext->startSettings.input.parserBand, &parserBandSettings);
            NEXUS_Frontend_TuneQam(pContext->input.frontend, &qamSettings);
        }
#else
        if(pContext->input.type == BXCode_InputType_eLive) {
            BDBG_ERR(("QAM Inout not supported"));
            BKNI_Fail();
        }
#endif

    pContext->startSettings.output.video.pid = BTST_MUX_VIDEO_PID;
    pContext->startSettings.output.video.encoder.codec   = pContext->output.vCodec;
    pContext->startSettings.output.video.encoder.profile = pContext->output.profile;
    pContext->startSettings.output.video.encoder.level   = pContext->output.level;
    if(pContext->output.customizeDelay) {
        BKNI_Memcpy(&pContext->startSettings.output.video.encoder.bounds, &pContext->output.bounds, sizeof(pContext->output.bounds));
    }
    pContext->startSettings.output.video.encoder.interlaced = pContext->output.videoFormat.interlaced;
    pContext->startSettings.output.video.encoder.nonRealTime = pContext->nonRealTime;
    pContext->startSettings.output.video.encoder.lowDelayPipeline = pContext->output.lowDelay;
    pContext->startSettings.output.video.encoder.encodeUserData = pContext->output.ccUserdata;
    pContext->startSettings.output.video.encoder.adaptiveLowDelayMode = true; /* always enable adaptive low delay */
    for(i=0; i<pContext->input.numAudios; i++) {
        pContext->startSettings.output.audio[i].pid = BTST_MUX_AUDIO_PID+i;
        if(pContext->output.audioEncode[i]) {
            pContext->startSettings.output.audio[i].codec = pContext->output.audioCodec[i];
        }
        pContext->startSettings.output.audio[i].passthrough = !pContext->output.audioEncode[i];
    }
    pContext->startSettings.output.transport.type = pContext->output.type;
    pContext->startSettings.output.transport.progressiveDownload = pContext->output.progressiveDownload;
    pContext->startSettings.output.transport.timestampType = pContext->output.transportTimestamp;
    pContext->startSettings.output.transport.pcrPid = BTST_MUX_PCR_PID;
    pContext->startSettings.output.transport.pmtPid = BTST_MUX_PMT_PID;
    pContext->startSettings.output.transport.segmented = pContext->output.segmented;
    if(pContext->output.file) {
        BDBG_WRN(("BXCode output file %s", pContext->output.data));
        pContext->startSettings.output.transport.file = pContext->output.data;
        pContext->startSettings.output.transport.index = pContext->output.index;
        pContext->startSettings.output.transport.tmpDir = "/data/videos"; /* for mp4 output */
    } else {
        BKNI_CreateEvent(&pContext->output.dataReadyEvent);
        pContext->startSettings.output.transport.dataCallback.callback = outputDataReadyCallback;
        pContext->startSettings.output.transport.dataCallback.context  = pContext->output.dataReadyEvent;
        pContext->startSettings.output.transport.dataCallback.param  = pContext->id;
    }
    BXCode_Start(pContext->hBxcode, &pContext->startSettings);
    pContext->started = true;

    /* stream input dataready event and handler */
    if(BXCode_InputType_eStream==pContext->input.type) {
        /* create input feed thread */
        pContext->input.feedThread = B_Thread_Create("Input feed thread", (B_ThreadFunc)inputFeedThread, (void*)pContext, NULL);
    } else if(pContext->input.type == BXCode_InputType_eImage) {
        BXCode_InputStatus status;
        BXCode_GetInputStatus(pContext->hBxcode, &status);
        pContext->imageInput = status.imageInput;
        pContext->imageInputStatus = status.imageInputStatus;

        /* create image input feed thread */
        pContext->input.feedThread = B_Thread_Create("Image input feed thread", (B_ThreadFunc)imageInputFeedThread, (void*)pContext, NULL);
    }

    /* stream output dataready event and handler */
    if(!pContext->output.file) {
        /* create output record thread */
        pContext->output.recordThread = B_Thread_Create("Output record thread", (B_ThreadFunc)outputRecordThread, (void*)pContext, NULL);
    }
    return NEXUS_SUCCESS;

}

/* stop without close */
static void stop_transcode(
    BTST_Transcoder_t  *pContext )
{
    if(!pContext->started) {
        return;
    }
    printInputStatus(pContext);
    printOutputStatus(pContext);
    BXCode_Stop(pContext->hBxcode);
#if NEXUS_HAS_HDMI_INPUT
    if(pContext->input.type == BXCode_InputType_eHdmi) {
        NEXUS_HdmiInput_Close(pContext->startSettings.input.hdmiInput);
    }
#endif
    pContext->started = false;

    if(BXCode_InputType_eStream==pContext->input.type || pContext->input.type == BXCode_InputType_eImage) {
        B_Mutex_Unlock(pContext->mutexStarted);
        BKNI_Sleep(100);
        B_Mutex_Lock(pContext->mutexStarted);
        B_Thread_Destroy(pContext->input.feedThread);
        BKNI_DestroyEvent(pContext->input.dataReadyEvent);
    }
    if(!pContext->output.file) {
        B_Mutex_Unlock(pContext->mutexStarted);
        BKNI_Sleep(100);
        B_Mutex_Lock(pContext->mutexStarted);
        B_Thread_Destroy(pContext->output.recordThread);
        BKNI_DestroyEvent(pContext->output.dataReadyEvent);
    }
}

NEXUS_Error bringup_transcode(
    BTST_Transcoder_t  *pContext )
{
    NEXUS_Error rc;
    /* open the transcode context */
    if(NULL==open_transcode(pContext)) {BDBG_ERR(("Failed to open bxcode[%u]", pContext->id)); rc = NEXUS_INVALID_PARAMETER; goto err_open;}

    /*******************************
     * START transcoder
     */
    rc = start_transcode(pContext);
    if(NEXUS_SUCCESS!=rc) {BDBG_ERR(("Failed to start bxcode[%u]", pContext->id)); goto err_start;}

    /* input eof event and handler */
    if((BXCode_InputType_eFile==pContext->input.type || BXCode_InputType_eStream==pContext->input.type) && (NULL==pContext->input.eofEvent)) {
        BKNI_CreateEvent(&pContext->input.eofEvent);
        pContext->input.eofHandler = B_Thread_Create("Input EOF handler", (B_ThreadFunc)input_endOfFileHandler, (void*)pContext, NULL);
    }
    return NEXUS_SUCCESS;
err_start:
    close_transcode(pContext);
err_open:
    return BERR_TRACE(rc);
}

void shutdown_transcode(
    BTST_Transcoder_t  *pContext )
{
    /*******************************
     * START transcoder
     */
    stop_transcode(pContext);
    BDBG_MSG(("stopped transcoder %d", pContext->id));

    /* close the transcode context */
    close_transcode(pContext);
    BDBG_MSG(("closed transcoder %d", pContext->id));
}

#ifdef DIAGS
int transcode_main(int argc, char **argv)  {
#else
int main(int argc, char **argv)  {
#endif
    NEXUS_PlatformSettings platformSettings;
    int rc;
    unsigned i;
    unsigned iteration = 0;
    BTST_Transcoder_t *pTranscoder;

    BKNI_Memset(&g_testContext, 0, sizeof(g_testContext));

    /* parse command line arguments */
    rc = cmdline_parse(argc, argv, &g_testContext);
    if(rc == 0) return 0;
    else if(rc < 0) {
        BDBG_ERR(("ERROR in test config!"));
        return rc;
    }
    pTranscoder = &g_testContext.xcodeContext[g_testContext.selectedXcodeContextId];

    /* init */
    B_Os_Init();
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = pTranscoder->input.type == BXCode_InputType_eLive;
    /* audio PI supports 4 by default; we need one extra mixers for each transcoders; */
    platformSettings.audioModuleSettings.dspAlgorithmSettings.typeSettings[NEXUS_AudioDspAlgorithmType_eAudioEncode].count = NEXUS_NUM_VIDEO_ENCODERS;
    platformSettings.audioModuleSettings.maxAudioDspTasks += NEXUS_NUM_VIDEO_ENCODERS + g_testContext.loopbackPlayer;/* to support quad xcodes + loopback decode */
    platformSettings.audioModuleSettings.numCompressedBuffers += NEXUS_NUM_VIDEO_ENCODERS;/* for quad xcodes */
    platformSettings.audioModuleSettings.numPcmBuffers = NEXUS_NUM_VIDEO_ENCODERS + g_testContext.loopbackPlayer;/* to support quad xcodes and loopback decode */
    if((pTranscoder->input.numAudios*2) > platformSettings.audioModuleSettings.maxAudioDspTasks) {
        platformSettings.audioModuleSettings.maxAudioDspTasks = pTranscoder->input.numAudios*2;
    }
    if(pTranscoder->input.numAudios > platformSettings.audioModuleSettings.numCompressedBuffers) {
        platformSettings.audioModuleSettings.numCompressedBuffers = pTranscoder->input.numAudios;
    }
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&g_platformConfig);
    /* detect encoder capabilities */
    {
        unsigned count = 0;
        bool headless = false;
        NEXUS_VideoEncoderCapabilities capabilities;
        NEXUS_DisplayCapabilities displayCapabilities;
        NEXUS_GetVideoEncoderCapabilities(&capabilities);
        for(i=0; i< NEXUS_MAX_VIDEO_ENCODERS; i++) {
            BDBG_MSG(("%u video encoder[%u]/display[%u] supported? %s.", BCHP_CHIP, i, capabilities.videoEncoder[i].displayIndex,
                capabilities.videoEncoder[i].supported? "Yes":"No"));
            count += capabilities.videoEncoder[i].supported;
            /* if encoder masters display 0, assumed headless */
            if(capabilities.videoEncoder[i].supported && capabilities.videoEncoder[i].displayIndex == 0) headless = true;
        }
        if(count == 0) { BDBG_WRN(("This box mode doesn't support video encoding!")); goto error_probe; }
        NEXUS_GetDisplayCapabilities(&displayCapabilities);
        /* if display 0 has no video window, assumed headless */
        if(displayCapabilities.display[0].numVideoWindows == 0) headless = true;
        BDBG_MSG(("Display[0] has %u video windows.", displayCapabilities.display[0].numVideoWindows));
        if(headless) {
            g_testContext.loopbackPlayer = false;
            BDBG_WRN(("This box mode is headless, so disable loopback player!"));
        }
    }
    if(pTranscoder->input.probe) {
        struct probe_request probe_request;
        struct probe_results probe_results;
        int rc;
        char *fname, *ptr;
        char program[20]={'\0',};

        probe_media_get_default_request(&probe_request);
        probe_request.streamname = pTranscoder->input.data;
        probe_request.program = pTranscoder->input.program;
        rc = probe_media_request(&probe_request, &probe_results);
        if (rc) {
            BDBG_ERR(("media probe failed to parse '%s'", probe_request.streamname));
            goto error_probe;
        }
        if(probe_results.num_audio==0 && probe_results.num_video==0) {
            BDBG_ERR(("media probe failed to find any audio or video program in '%s'", probe_request.streamname));
            goto error_probe;
        }
        pTranscoder->input.useStreamAsIndex = probe_results.useStreamAsIndex;
        pTranscoder->input.transportType = probe_results.transportType;
        if(pTranscoder->input.useStreamAsIndex) {
            BKNI_Snprintf(pTranscoder->input.index, BTST_FILE_NAME_LEN, "%s", pTranscoder->input.data);
        } else if(pTranscoder->input.transportType == NEXUS_TransportType_eTs) {
            xcode_index_filename(pTranscoder->input.index, pTranscoder->input.data, false, pTranscoder->input.transportType);
        }
        pTranscoder->input.transportTimestamp = probe_results.timestampType;
        /* TODO: add multiple audio pids support */
        pTranscoder->input.enableVideo = probe_results.num_video;
        pTranscoder->input.vCodec      = probe_results.video[0].codec;
        pTranscoder->input.videoPid    = probe_results.video[0].pid;
        pTranscoder->input.pcrPid      = probe_results.video[0].pid;
        pTranscoder->input.enableAudio = probe_results.num_audio;
        pTranscoder->input.numAudios   = probe_results.num_audio;
        pTranscoder->input.aCodec[0]   = probe_results.audio[0].codec;
        pTranscoder->input.audioPid[0] = probe_results.audio[0].pid;
        print_xcoder_inputSetting(pTranscoder);

        /* strip out directory path */
        fname = ptr = pTranscoder->input.data;
        while(*ptr) {
            if(*ptr == '/') fname = ptr + 1;
            ptr++;
        };
        /* insert program string or not in output file name */
        if(pTranscoder->input.program) BKNI_Snprintf(program, 20, "_program%u", pTranscoder->input.program);
        if(BKNI_Snprintf(pTranscoder->output.data, BTST_FILE_NAME_LEN, "/data/videos/%s%s_%ux%u%c%s_%.0fkps.%s", fname,
            program, pTranscoder->output.videoFormat.width, pTranscoder->output.videoFormat.height,
            pTranscoder->output.videoFormat.interlaced?'i':'p',
            lookup_name(g_videoFrameRateStrs, pTranscoder->output.framerate),
            (float)pTranscoder->output.vBitrate/1000,
            (pTranscoder->output.type == BXCode_OutputType_eTs)? "ts" :
            ((pTranscoder->output.type == BXCode_OutputType_eMp4File)? "mp4" : "ves")) > BTST_FILE_NAME_LEN) {
            BDBG_ERR(("Output file path(%s) is too long > %u characters!", pTranscoder->output.data, BTST_FILE_NAME_LEN));
            goto error_probe;
        }
        if(pTranscoder->output.type == BXCode_OutputType_eTs && pTranscoder->output.file) {
            if(pTranscoder->input.enableVideo)
            {
                xcode_index_filename(pTranscoder->output.index, pTranscoder->output.data, pTranscoder->output.segmented,
                    (BXCode_OutputType_eTs==pTranscoder->output.type)? NEXUS_TransportType_eTs :
                    ((BXCode_OutputType_eMp4File==pTranscoder->output.type)? NEXUS_TransportType_eMp4 : NEXUS_TransportType_eEs));
            }
        } else {
            if(pTranscoder->output.type == BXCode_OutputType_eEs) {
                printf("Output audio file name:\n");
                BKNI_Snprintf(pTranscoder->output.audioFiles[0], BTST_FILE_NAME_LEN, "/data/videos/%s%s.aes", fname, program);
            }
        }
        print_xcoder_outputSetting(pTranscoder);
        /* allow interactive menu control */
        pTranscoder->custom = true;
        /* automatically quit the test at the end of file */
        g_testContext.autoQuit = true;
    }

    BKNI_CreateEvent(&g_testContext.doneEvent);

    for(i=0; i< NEXUS_NUM_VIDEO_ENCODERS; i++) {
        g_testContext.xcodeContext[i].mutexStarted = B_Mutex_Create(NULL);
        g_testContext.xcodeContext[i].id = i;
    }

again:
    /* mutex to protect init bringup of the xcoder and loopback player */
    B_Mutex_Lock(pTranscoder->mutexStarted);

    /* bringup the transcode context */
    bringup_transcode(pTranscoder);

    /****************************************************
     * set up xcoder0 loopback decode/display for debug purpose
     */
    if(g_testContext.loopbackPlayer && 0 != get_display_index(pTranscoder->id)) {
        xcode_loopback_setup(&g_testContext);
        g_testContext.loopbackXcodeId = g_testContext.selectedXcodeContextId;
    } else {/* disable loopback player if loopback display is used by the encoder display */
        g_testContext.loopbackXcodeId = -1;
    }
    B_Mutex_Unlock(pTranscoder->mutexStarted);

    /****************************************************
     *                       key handler                                                 *
     *****************************************************/
    /* wait for 'q' to exit the test */
    if(g_testContext.scriptMode)
    {
        keyHandler(&g_testContext);
    }
    else if (pTranscoder->custom)
    {
        B_ThreadHandle keyHandle;

        /* Turn off buffering for stdin */
        setvbuf(stdin, NULL, _IONBF, 0);

        keyHandle = B_Thread_Create("key handler", (B_ThreadFunc)keyHandler, pTranscoder, NULL);
        while(BKNI_WaitForEvent(g_testContext.doneEvent, BKNI_INFINITE)!=BERR_SUCCESS);
        B_Thread_Destroy(keyHandle);
        BDBG_MSG(("main thread to quit..."));
        g_keyReturn = 'q';
    }
    else
    {
        BXCode_Settings settings;
        BDBG_WRN(("Auto test iteration: %d\n", iteration));
        BKNI_Sleep(30000);/* 30 seconds */
#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
        /* change resolution */
        BXCode_GetSettings(pTranscoder->hBxcode, &settings);
        settings.video.width  = 640;
        settings.video.height = 480;
        settings.video.encoder.frameRate  = NEXUS_VideoFrameRate_e60;
        BXCode_SetSettings(pTranscoder->hBxcode, &settings);
        BDBG_WRN(("format switch to 640x480p60"));
        settings.video.encoder.bitrateMax = 2000000;
        BXCode_SetSettings(pTranscoder->hBxcode, &settings);
        BDBG_WRN(("bitrate switch to 2Mbps"));
        settings.video.encoder.frameRate  = NEXUS_VideoFrameRate_e30;
        settings.video.encoder.bitrateMax = 1000000;
        settings.video.encoder.streamStructure.framesP = 59;
        BXCode_SetSettings(pTranscoder->hBxcode, &settings);
        BDBG_WRN(("frame rate switch to 30fps\n"));
        BKNI_Sleep(30000);/* 30 seconds */
#else
        BSTD_UNUSED(settings);
#endif
        g_keyReturn = iteration > 3 ? 'q' : 'c'; /* continue looping until quit */
    }

    /* bringdown loopback path */
    if(g_testContext.loopbackPlayer && g_testContext.loopbackStarted) {
        xcode_loopback_shutdown(&g_testContext);
    }
    for(i = 0; i < NEXUS_NUM_VIDEO_ENCODERS; i++) {
        B_Mutex_Lock(g_testContext.xcodeContext[i].mutexStarted);
        if(g_testContext.xcodeContext[i].started) {
            shutdown_transcode(&g_testContext.xcodeContext[i]);
        }
        B_Mutex_Unlock(g_testContext.xcodeContext[i].mutexStarted);
        if(g_testContext.xcodeContext[i].input.eofHandler) {
            BKNI_SetEvent(g_testContext.xcodeContext[i].input.eofEvent);
            BKNI_Sleep(1);
            B_Thread_Destroy(g_testContext.xcodeContext[i].input.eofHandler);
            g_testContext.xcodeContext[i].input.eofHandler = NULL;
        }
        if(g_testContext.xcodeContext[i].input.eofEvent) {
            BKNI_DestroyEvent(g_testContext.xcodeContext[i].input.eofEvent);
            g_testContext.xcodeContext[i].input.eofEvent = NULL;
        }
    }

    if(g_keyReturn != 'q')
    {
        iteration++;
        BDBG_WRN(("Start iteration %u.....", iteration));
        pTranscoder = &g_testContext.xcodeContext[0];
        goto again;
    }

    for(i=0; i< NEXUS_NUM_VIDEO_ENCODERS; i++) {
        B_Mutex_Destroy(g_testContext.xcodeContext[i].mutexStarted);
    }

error_probe:
    /* uninit platform */
    NEXUS_Platform_Uninit();

    BKNI_DestroyEvent(g_testContext.doneEvent);
    B_Os_Uninit();

    return 0;
}
#else
#include <stdio.h>

int main(void) {

    printf("\n\nVideo Encoder/Transcode is not supported on this platform\n\n");
    return 0;
}
#endif /* NEXUS_HAS_VIDEO_ENCODER */
