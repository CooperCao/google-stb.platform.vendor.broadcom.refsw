/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
/* Nexus example app: playback and decode */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_VIDEO_ENCODER && NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_video_adj.h"
#include "nexus_video_encoder_output.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"


BDBG_MODULE(encode_video);
int main(void)  {
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel, stcChannelEncoder;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

    NEXUS_DisplayHandle displayTranscode;
    NEXUS_VideoWindowHandle windowTranscode;
    NEXUS_VideoEncoderCapabilities videoEncoderCap;
    NEXUS_VideoEncoderHandle videoEncoder;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderDelayRange videoDelay;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderStatus videoEncoderStatus;
    void *pDataBuffer;

    FILE *fout;
    size_t bytes;


    NEXUS_DisplaySettings displaySettings;
    const char *fname = "videos/avatar_AVC_15M.ts";
    FILE *fdesc;

    fdesc = fopen("videos/video_desc.txt","w");
    fprintf(fdesc, "flags(h) origPts(h) pts(Lh)     stc(Lh)     escr(h) ticksPerBit(u) shr(d) offset(h) length(h) videoFlags(h) nalType dts(Lh)\n");

#if defined(NEXUS_MODE_client)
    NEXUS_Platform_Join();
#else
    {
        NEXUS_PlatformSettings platformSettings;
        NEXUS_Platform_GetDefaultSettings(&platformSettings);
        platformSettings.openFrontend = false;
        NEXUS_Platform_Init(&platformSettings);
    }
#endif
    NEXUS_Platform_GetConfiguration(&platformConfig);

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);


    /* bring up decoder and connect to local display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */

    /* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
     * which might cause encoder display GISB error since encoder display would
     * trigger RDC to program mailbox registers in ViCE2;
     */
    videoEncoder = NEXUS_VideoEncoder_Open(0, NULL);
    assert(videoEncoder);
    NEXUS_GetVideoEncoderCapabilities(&videoEncoderCap);

    /* Bring up video encoder display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
#if !NEXUS_NUM_DSP_VIDEO_ENCODERS || NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.format = NEXUS_VideoFormat_e720p24hz;/* bring up 480p first */
#endif
    displayTranscode = NEXUS_Display_Open(videoEncoderCap.videoEncoder[0].displayIndex, &displaySettings);
    assert(displayTranscode);

    windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
    assert(windowTranscode);

#if NEXUS_NUM_DSP_VIDEO_ENCODERS && !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    {
    NEXUS_VideoWindowScalerSettings sclSettings;
    NEXUS_VideoWindowSettings windowSettings;

    NEXUS_VideoWindow_GetSettings(windowTranscode, &windowSettings);
    windowSettings.position.width = 416;
    windowSettings.position.height = 224;
    windowSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
    windowSettings.visible = false;
    NEXUS_VideoWindow_SetSettings(windowTranscode, &windowSettings);

    NEXUS_VideoWindow_GetScalerSettings(windowTranscode, &sclSettings);
    sclSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerBeforeCapture;
    sclSettings.bandwidthEquationParams.delta = 1000000;
    NEXUS_VideoWindow_SetScalerSettings(windowTranscode, &sclSettings);
    }
#endif

    /* connect same decoder to encoder display
     * This simul mode is for video encoder bringup only; audio path may have limitation
     * for simul display+transcode mode;
     */
    NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* Open the video pid channel */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eH264; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x101, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = NEXUS_VideoCodec_eH264;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e29_97;
    #if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    videoEncoderConfig.bitrateMax = 2000*1000;
    #else
    videoEncoderConfig.bitrateMax = 400*1000;
    #endif
#else
    videoEncoderConfig.variableFrameRate = true;
    videoEncoderConfig.frameRate = NEXUS_VideoFrameRate_e24;
    videoEncoderConfig.bitrateMax = 6*1000*1000;
    videoEncoderConfig.streamStructure.framesP = 29; /* IPP GOP size = 30 */
    videoEncoderConfig.streamStructure.framesB = 0;
#endif
    /* Start decoders */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    BKNI_Sleep(1000);

    /* encoder requires different STC broadcast mode from decoder */
    NEXUS_StcChannel_GetDefaultSettings(1, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    stcChannelEncoder = NEXUS_StcChannel_Open(1, &stcSettings);

    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.codec = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e30;
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    videoEncoderStartConfig.bounds.inputDimension.max.width = 416;
    videoEncoderStartConfig.bounds.inputDimension.max.height = 224;
#else
    videoEncoderStartConfig.bounds.inputDimension.max.width = 720;
    videoEncoderStartConfig.bounds.inputDimension.max.height = 480;
#endif
    videoEncoderStartConfig.stcChannel = stcChannelEncoder;
#else
    videoEncoderStartConfig.codec = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e31;
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.stcChannel = stcChannelEncoder;
#endif

    /* NOTE: video encoder delay is in 27MHz ticks */
    NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
    BDBG_WRN(("\n\tVideo encoder end-to-end delay = [%u ~ %u] ms", videoDelay.min/27000, videoDelay.max/27000));
    videoEncoderConfig.encoderDelay = videoDelay.min;

    /* note the Dee is set by SetSettings */
    NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);
    NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);
    NEXUS_VideoEncoder_GetStatus(videoEncoder, &videoEncoderStatus);
    NEXUS_MemoryBlock_Lock(videoEncoderStatus.bufferBlock, &pDataBuffer);

    fout = fopen("videos/encoder.mpg","wb");
    assert(fout);

    for(bytes=0;;) {
        size_t size[2];
        const NEXUS_VideoEncoderDescriptor *desc[2];
        unsigned i,j;
        unsigned descs;


        NEXUS_VideoEncoder_GetBuffer(videoEncoder, &desc[0], &size[0], &desc[1], &size[1]);
        if(size[0]==0 && size[1]==0) {
            NEXUS_VideoDecoderStatus vstatus;

            NEXUS_VideoDecoder_GetStatus(videoDecoder, &vstatus);
            fflush(fout);
            fprintf(stderr, "written %u bytes.... decode:%u\t\r", bytes, vstatus.pts);
            BKNI_Sleep(30);
            continue;
        }
        for(descs=0,j=0;j<2;j++) {
            descs+=size[j];
            for(i=0;i<size[j];i++) {
                if(desc[j][i].length > 0)
                {
                if((desc[j][i].flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_METADATA) ==0) {/* ignore metadata descriptor in es capture */
                    fwrite((const uint8_t *)pDataBuffer + desc[j][i].offset, desc[j][i].length, 1, fout);
                }
                fprintf(fdesc, "%8x %8x   %x%08x   %x%08x %08x     %5u   %5d   %8x %8x %#x %u %x%08x\n", desc[j][i].flags, desc[j][i].originalPts,
                    (uint32_t)(desc[j][i].pts>>32), (uint32_t)(desc[j][i].pts & 0xffffffff),
                    (uint32_t)(desc[j][i].stcSnapshot>>32), (uint32_t)(desc[j][i].stcSnapshot & 0xffffffff), desc[j][i].escr,
                    desc[j][i].ticksPerBit, desc[j][i].shr, desc[j][i].offset, desc[j][i].length, desc[j][i].videoFlags, desc[j][i].dataUnitType,
                    (uint32_t)(desc[j][i].dts>>32), (uint32_t)(desc[j][i].dts & 0xffffffff));
                bytes+= desc[j][i].length;
                if(desc[j][i].length > 0x100000)
                {
                    BDBG_ERR(("++++ desc[%d][%d] length = 0x%x, offset=0x%x", j,i, desc[j][i].length, desc[j][i].offset));
                }

                }
            }
        }
        NEXUS_VideoEncoder_ReadComplete(videoEncoder, descs);
    }
    fclose(fout);

    /* Bring down system */
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_StcChannel_Close(stcChannel);

    NEXUS_Display_Close(displayTranscode);
    NEXUS_VideoEncoder_Close(videoEncoder);
    NEXUS_StcChannel_Close(stcChannelEncoder);

    NEXUS_Platform_Uninit();

    return 0;
}
#else /* NEXUS_HAS_PLAYBACK && NEXUS_HAS_VIDEO_ENCODER && NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("This application is not supported on this platform (needs playback and video decoder/encoder)!\n");
    return 0;
}
#endif
