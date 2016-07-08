/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/
/* Nexus example app: transcode a 422 yuv stream into a transport stream using image input interface */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_video_image_input.h"
#include "nexus_display_types.h"

#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder_output.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"
#endif

#include "nexus_core_utils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"

BDBG_MODULE(transcode_playback_yuv_to_ts);

#if NEXUS_HAS_VIDEO_ENCODER
static void imageBufferCallback( void *context, int param )
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    BDBG_MSG(("Got Disp Cback!"));
}
#endif

#define BDBG_API_CALLS(x) /* BDBG_WRN(x)  enable this to see where delays are in the system */

#define NUM_SURFACES 3

struct {
    NEXUS_SurfaceHandle handle;
    bool submitted;
} g_surface[NUM_SURFACES];


/* Set width and height according to the
  width and height of the yuv stream you are using
*/
#define width_encode 1280
#define height_encode 720


int main(int argc, const char *argv[]) {

    NEXUS_StcChannelHandle   stcChannelEncoder;
    NEXUS_StcChannelSettings stcSettings;

    NEXUS_PlatformSettings      platformSettings;
    NEXUS_PlatformConfiguration platformConfig;

    NEXUS_DisplayHandle             displayTranscode;
    NEXUS_VideoWindowHandle         windowTranscode;
    NEXUS_VideoEncoderCapabilities videoEncoderCap;
    NEXUS_VideoEncoderHandle        videoEncoder;
    NEXUS_VideoEncoderStatus        videoEncoderStatus;
    NEXUS_VideoEncoderSettings      videoEncoderConfig;
    NEXUS_VideoEncoderDelayRange    videoDelay;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    FILE   *fout;
    size_t bytes;
    int rc = 0;
    NEXUS_VideoImageInputHandle   imageInput;
    NEXUS_VideoInput              videoInput;
    BKNI_EventHandle              imageEvent;
    NEXUS_VideoImageInputSettings imageInputSetting;
    NEXUS_VideoImageInputStatus   imageInputStatus;
    NEXUS_SurfaceCreateSettings   surfaceCfg;
    NEXUS_SurfaceHandle           pic = NULL;
    NEXUS_SurfaceMemory           surfaceMemory;
    FILE                          *fin;

    unsigned submitIdx,releaseIdx;
    int i,queSize = NUM_SURFACES;
    int loopCnt;

    NEXUS_DisplaySettings displaySettings;
    FILE *fdesc;

    size_t bytesRead=0;
    int vSyncs=1,curarg = 1;
    unsigned runningPts,ptsIncrement=0x1500;

    NEXUS_DisplayHandle       displayHD;
    NEXUS_VideoWindowHandle   windowHD_Main;
    NEXUS_VideoFormatInfo     videoInfo;
    NEXUS_VideoWindowSettings windowSettings;

    void *pDataBuffer;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-que") && curarg+1 < argc) {
            queSize = atoi(argv[++curarg]);
            if ( queSize > NUM_SURFACES ) {
                fprintf( stderr, "App only supports %d surfaces, setting queue to %d surfaces\n" , NUM_SURFACES , NUM_SURFACES );
                queSize = NUM_SURFACES;
            }
        }
        else if (!strcmp(argv[curarg], "-vsync") && curarg+1 < argc ) {
            vSyncs = atoi(argv[++curarg]);
            if ( vSyncs == 0 ) {
                fprintf( stderr, "vSync of ZERO is meaningless, set vSync to 1 \n" );
                vSyncs = 1;
            }
        }
        curarg++;
    }

    fdesc = fopen("videos/video_desc.txt","w");
    fprintf(fdesc, "flags(h) origPts(h) pts(Lh)     escr(h) ticksPerBit(u) shr(d) offset(h) length(h)\n");

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

/*============create a window to display and create syrface to get yuv 4:2:2 raw data to encode============================================*/

    imageInput = NEXUS_VideoImageInput_Open(0, NULL);
    BDBG_ASSERT(imageInput);

    videoInput = NEXUS_VideoImageInput_GetConnector(imageInput);
    BDBG_ASSERT(videoInput);

    NEXUS_VideoImageInput_GetStatus(imageInput, &imageInputStatus);

    BKNI_CreateEvent(&imageEvent);
    NEXUS_VideoImageInput_GetSettings( imageInput, &imageInputSetting );
    imageInputSetting.imageCallback.callback = imageBufferCallback;
    imageInputSetting.imageCallback.context  = imageEvent;
    NEXUS_VideoImageInput_SetSettings( imageInput, &imageInputSetting );

    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCfg);
    surfaceCfg.width  = width_encode;
    surfaceCfg.height = height_encode;
    surfaceCfg.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_MemoryStatus s;
        if (!platformConfig.heap[i] || NEXUS_Heap_GetStatus(platformConfig.heap[i], &s)) continue;
        if (s.memcIndex == imageInputStatus.memcIndex && (s.memoryType & NEXUS_MemoryType_eApplication) && s.largestFreeBlock >= 960*1080*2) {
            surfaceCfg.heap = platformConfig.heap[i];
            break;
        }
    }
    if (!surfaceCfg.heap) {
        BDBG_ERR(("no heap found. RTS failure likely."));
    }
    BDBG_WRN(("(%s,%d): creat three surface.format %d, %dx%d, pitch %d",
        __FUNCTION__, __LINE__,surfaceCfg.pixelFormat, surfaceCfg.width, surfaceCfg.height, surfaceCfg.pitch));

    for( i=0; i < queSize ; i++ ) {
        g_surface[i].handle = NEXUS_Surface_Create(&surfaceCfg);
        if ( !g_surface[i].handle ) {
            fprintf(stderr, "Can't create g_surface[%d].handle \nTerminate program\n",i);
            return -1;
        }
        g_surface[i].submitted = false;
    }

/*============================================================*/

    /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
     * which might cause encoder display GISB error since encoder display would
     * trigger RDC to program mailbox registers in ViCE2;
     */
    videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
    assert(videoEncoder);
    NEXUS_GetVideoEncoderCapabilities(&videoEncoderCap);

    /* Bring up video encoder display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;

    /* Hard code Display output to match fed in YUV stream (720p) */
    displaySettings.format = NEXUS_VideoFormat_e720p;

    displayTranscode = NEXUS_Display_Open(videoEncoderCap.videoEncoder[0].displayIndex, &displaySettings);
    assert(displayTranscode);

    windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
    assert(windowTranscode);

    /* connect same decoder to encoder display
     * This simul mode is for video encoder bringup only; audio path may have limitation
     * for simul display+transcode mode;
     */

    NEXUS_VideoWindow_AddInput(windowTranscode, videoInput);

    /* Add HD display / window to display yuv being fed in onto TV screen */
    NEXUS_Display_GetDefaultSettings(&displaySettings);  /* Get some baseline settings for a display */
    displaySettings.format = NEXUS_VideoFormat_e720p;
    displayHD = NEXUS_Display_Open( 0, &displaySettings);

    /* Associate a window with HD path */
    windowHD_Main = NEXUS_VideoWindow_Open(displayHD, 0);
    assert(windowHD_Main);

#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(displayHD , NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));  /* Attach HDMI output to HD display */
#endif

    /* Scale window to whatever output format is. */
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoInfo);
    NEXUS_VideoWindow_GetSettings(windowHD_Main, &windowSettings);
    windowSettings.position.x      = 0;
    windowSettings.position.y      = 0;
    windowSettings.position.width  = videoInfo.width;
    windowSettings.position.height = videoInfo.height;
    NEXUS_VideoWindow_SetSettings(windowHD_Main, &windowSettings);

    NEXUS_VideoWindow_AddInput(windowHD_Main, videoInput);

    NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
    videoEncoderConfig.frameRate                  = NEXUS_VideoFrameRate_e24;
    videoEncoderConfig.bitrateMax                 = 16*1000*1000;
    videoEncoderConfig.variableFrameRate          = true;
    videoEncoderConfig.streamStructure.framesP    = 29; /* IPP GOP size = 30 */
    videoEncoderConfig.streamStructure.framesB    = 0;

    /* encoder requires different STC broadcast mode from decoder */
    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode     = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits  = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    stcChannelEncoder    = NEXUS_StcChannel_Open(1, &stcSettings);

    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
    videoEncoderStartConfig.codec      = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile    = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level      = NEXUS_VideoCodecLevel_e31;
    videoEncoderStartConfig.input      = displayTranscode;
    videoEncoderStartConfig.stcChannel = stcChannelEncoder;

    /* NOTE: video encoder delay is in 27MHz ticks */
    NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
    BDBG_WRN(("\n\tVideo encoder end-to-end delay = [%u ~ %u] ms", videoDelay.min/27000, videoDelay.max/27000));
    videoEncoderConfig.encoderDelay = videoDelay.min;

    BKNI_Sleep(1000); /* for HDMI mointor output this allows monitor time to sync and display */

    /* note the Dee is set by SetSettings */
    NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);
    NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);
    NEXUS_VideoEncoder_GetStatus(videoEncoder, &videoEncoderStatus);

    NEXUS_MemoryBlock_Lock(videoEncoderStatus.bufferBlock, &pDataBuffer);

    /* ffmpeg converted yuv stream of Brcm 1080p24 SBS 3d stream for test purposes */
    /* any yuv stream with 422 encoding will also work */
    /* If you change stream change width_encode and height_encode to match your test stream */
    fin = fopen("videos/720p60_SBS.yuv","rb");

    if(fin == NULL) {
        printf("\nyuv file can't be read\n");
        assert(fin);
    }

    fout = fopen("videos/yuv_encoded.mpg","wb");
    assert(fout);

    loopCnt=1;
    runningPts = 0;

    submitIdx=0;
    releaseIdx=0;

    for(bytes=0;;) {
        size_t size[2];
        const NEXUS_VideoEncoderDescriptor *desc[2];
        NEXUS_VideoImageInputSurfaceSettings surfaceSettings;
        size_t num_entries;
        NEXUS_SurfaceHandle freeSurface=NULL;
        int j;
        unsigned descs;

/*================================*/
        num_entries=0;
        /* Make sure image surface is not in use by Video Output (VDC) */
            do {
                if ( g_surface[submitIdx].submitted ) {
                    /* our queue is all used up, need to wait until VideoImageInput returns */
                    /* a surface after it has been displayed                                */
                    BDBG_MSG(("g_surface[submitIdx=%d].submitted in use, wait for recycle" , submitIdx ));
                    BKNI_WaitForEvent( imageEvent, BKNI_INFINITE);
                }

                rc=NEXUS_VideoImageInput_RecycleSurface( imageInput, &freeSurface , 1, &num_entries );
                BDBG_ASSERT(!rc);
                if ( num_entries ) {
                    /* our surface has been displayed, we can now re-use and re-queue it */
                    BDBG_MSG(("g_surface[releaseIdx=%d].handle=%p  recycSurface=%p" , releaseIdx, (void*)g_surface[releaseIdx].handle , (void*)freeSurface ));
                    BDBG_ASSERT( g_surface[releaseIdx].handle == freeSurface );
                    g_surface[releaseIdx].submitted = false;
                    if ( ++releaseIdx == NUM_SURFACES ) releaseIdx=0;
                }

            } while ( num_entries );


            g_surface[submitIdx].submitted = true; /* mark as inuse */
            pic = g_surface[submitIdx].handle;
            BDBG_MSG(("pic=%p" , (void*)pic ));
            if ( ++submitIdx == NUM_SURFACES ) submitIdx=0;

        NEXUS_Surface_GetMemory( pic, &surfaceMemory);
        BDBG_MSG(("Start read in frame=%d" , loopCnt ));
        bytesRead = fread((uint8_t *)(surfaceMemory.buffer),1,width_encode*height_encode*2,fin);
        if ( !bytesRead ) {
            break;
        }
        BDBG_MSG(("bytesRead=%d for frame=%d" , bytesRead, loopCnt ));
        loopCnt++;

        NEXUS_Surface_Flush( pic );

        NEXUS_VideoImageInput_GetDefaultSurfaceSettings( &surfaceSettings );
        surfaceSettings.displayVsyncs = vSyncs; /* x * Vysnc(~16mSec) = number of seconds to display */
        surfaceSettings.pts = runningPts;
        runningPts += ptsIncrement;

        /* Submit surface to VideoImageInput, wait if queue to feed VDC is already full */
        do {
            rc = NEXUS_VideoImageInput_PushSurface(imageInput, pic , &surfaceSettings );
            if(rc==NEXUS_IMAGEINPUT_QUEUE_FULL) {
                BKNI_WaitForEvent(imageEvent, BKNI_INFINITE);
            }
        } while ( rc );

/*============================================================*/

        BDBG_API_CALLS(("L%d Bfr NEXUS_VideoEncoder_GetBuffer" , __LINE__ ));
        NEXUS_VideoEncoder_GetBuffer(videoEncoder, &desc[0], &size[0], &desc[1], &size[1]);
        BDBG_API_CALLS(("L%d Aftr NEXUS_VideoEncoder_GetBuffer" , __LINE__ ));

        if(size[0]==0 && size[1]==0) {
        #if 0
            NEXUS_VideoDecoderStatus vstatus;

            NEXUS_VideoDecoder_GetStatus(videoDecoder, &vstatus);
        #endif
            fflush(fout);
            fprintf(stderr, "written %lu bytes.... \t\r", (unsigned long)bytes);
            continue;
        }
        for(descs=0,j=0;j<2;j++) {
            descs+=size[j];
            for(i=0;i<(int)size[j];i++) {
                if(desc[j][i].length > 0)
                {
                fwrite((const uint8_t *)pDataBuffer + desc[j][i].offset, desc[j][i].length, 1, fout);
                fprintf(fdesc, "%8x %8x   %x%08x %08x     %5u   %5d   %8x %lux\n", desc[j][i].flags, desc[j][i].originalPts,
                    (uint32_t)(desc[j][i].pts>>32), (uint32_t)(desc[j][i].pts & 0xffffffff), desc[j][i].escr,
                    desc[j][i].ticksPerBit, desc[j][i].shr, desc[j][i].offset, (unsigned long)desc[j][i].length);
                bytes+= desc[j][i].length;
                if(desc[j][i].length > 0x100000)
                {
                    BDBG_ERR(("++++ desc[%d][%d] length = 0x%x, offset=0x%x", j,i, desc[j][i].length, desc[j][i].offset));
                }

                }
            }
        }
        BDBG_API_CALLS(("L%d Bfr NEXUS_VideoEncoder_ReadComplete" , __LINE__ ));
        NEXUS_VideoEncoder_ReadComplete(videoEncoder, descs);
        BDBG_API_CALLS(("L%d Aftr NEXUS_VideoEncoder_ReadComplete\n\n\n" , __LINE__ ));
    }
    fclose(fout);
    BDBG_WRN(("Read all data, press Enter to continue" ));
    getchar();

    /* Bring down system */
    NEXUS_VideoEncoder_Stop(videoEncoder,NULL);

    NEXUS_VideoImageInput_SetSurface(imageInput, NULL);  /* clear out any pics being displayed */
    NEXUS_VideoInput_Shutdown(videoInput);

    NEXUS_VideoImageInput_Close(imageInput);

    for( i=queSize; i > 0 ; i-- ) {
        NEXUS_Surface_Destroy(g_surface[i-1].handle);
    }
    NEXUS_VideoWindow_RemoveAllInputs( windowHD_Main  );
    NEXUS_VideoWindow_Close(windowHD_Main);

    NEXUS_Display_Close(displayHD);

    NEXUS_VideoEncoder_Close(videoEncoder);
    NEXUS_StcChannel_Close(stcChannelEncoder);

    NEXUS_VideoWindow_Close(windowTranscode);
    NEXUS_Display_Close(displayTranscode);

    NEXUS_Platform_Uninit();

    return 0;
}
#else /* NEXUS_HAS_VIDEO_ENCODER */
int main(void) {
    fprintf(stderr, "VIDEO ENCODER NOT SUPPORTED ON THIS PLATFORM \n");
    return 0;
}
#endif
