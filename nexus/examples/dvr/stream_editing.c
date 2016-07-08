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
/* Nexus example app: playback and decode */
#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_file_pvr.h"
#endif

#include <assert.h>
#include "bstd.h"
#include "bkni.h"

/* the following define the input files and their characteristics -- these will vary by input file */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x21

static void copy_file(off_t start, off_t end, void *buf, size_t buf_size, bfile_io_read_t src, bfile_io_write_t dst)
{
    off_t current;

    current = src->seek(src, start, SEEK_SET);
    BDBG_ASSERT(start==current);
    for(current = start; current < end;) {
        size_t to_read = buf_size;
        ssize_t result;

        if(current+to_read>end) {
            to_read = end-current;
        }
        result = src->read(src, buf, to_read);
        BDBG_ASSERT(result == (ssize_t)to_read);
        dst->write(dst, buf, result);
        current += result;
    }
    fprintf(stderr, "copied %u bytes\n", (unsigned)(current - start));
    return;
}

int main(void)
{
#if NEXUS_HAS_PLAYBACK
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_FilePlayHandle file, file_cut;
    NEXUS_FileRecordHandle file_write;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    const char *source_fname = "videos/cnnticker.mpg";
    const char *source_navname= "videos/cnnticker.nav";
    const char *dest_fname = "videos/cnnticker_cut.mpg";
    const char *dest_navname = "videos/cnnticker_cut.nav";
    const size_t buf_size = BIO_BLOCK_SIZE*16;
    void *buf;
    void *buf_alligned;
    NEXUS_FilePosition first, last, start, end;
    NEXUS_FilePlayOffset_Settings offsetSettings;
    NEXUS_Error rc;
    unsigned long startCut, endCut;
    NEXUS_PlaybackStatus status;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file = NEXUS_FilePlay_OpenPosix(source_fname, source_navname);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", source_fname);
        return -1;
    }

    /* get size of the source file */
    rc = NEXUS_FilePlay_GetBounds(file, &first, &last);
    BDBG_ASSERT(rc==NEXUS_SUCCESS);
    fprintf(stderr, "first:%u,%u,%u last:%u,%u,%u\n", 
		(unsigned)first.timestamp, (unsigned)first.mpegFileOffset, (unsigned)first.indexOffset, 
		(unsigned)last.timestamp, (unsigned)last.mpegFileOffset, (unsigned)last.indexOffset);

    /* extract portion of the file from 1/4 ... 3/4 */
    startCut = first.timestamp + (last.timestamp - first.timestamp)/4;
    endCut = first.timestamp + 3*(last.timestamp - first.timestamp)/4;
    fprintf(stderr, "cut:%u %u\n", (unsigned)startCut, (unsigned)endCut);

    /* get location of cut points */
    rc = NEXUS_FilePlay_GetLocation( file, startCut, &start);
    BDBG_ASSERT(rc==NEXUS_SUCCESS);
    rc = NEXUS_FilePlay_GetLocation( file, endCut, &end);
    BDBG_ASSERT(rc==NEXUS_SUCCESS);
    fprintf(stderr, "cut:%u:[%u,%u,%u] %u:[%u,%u,%u]\n", 
		(unsigned)startCut, (unsigned)start.timestamp, (unsigned)start.mpegFileOffset, (unsigned)start.indexOffset, 
		(unsigned)endCut, (unsigned)end.timestamp, (unsigned)end.mpegFileOffset, (unsigned)end.indexOffset);

    /* align data to the BIO_BLOCK_SIZE */
    end.mpegFileOffset += (BIO_BLOCK_SIZE-1);
    end.mpegFileOffset -= end.mpegFileOffset % BIO_BLOCK_SIZE;
    start.mpegFileOffset -= start.mpegFileOffset % BIO_BLOCK_SIZE;

    /* open destination file */
    file_write = NEXUS_FileRecord_OpenPosix(dest_fname, dest_navname);
    BDBG_ASSERT(file_write);

    /* allocate and align buffer */
    buf = BKNI_Malloc(buf_size+BIO_BLOCK_SIZE);
    BDBG_ASSERT(buf);
    buf_alligned = (uint8_t*)buf + BIO_BLOCK_SIZE-1;
    buf_alligned = (uint8_t*)buf_alligned - ((unsigned long)buf_alligned%BIO_BLOCK_SIZE);

    /* copy index */
    copy_file(start.indexOffset, end.indexOffset, buf_alligned, buf_size, file->file.index, file_write->index);
    /* and data */
    copy_file(start.mpegFileOffset, end.mpegFileOffset, buf_alligned, buf_size, file->file.data, file_write->data);
    /* free resources */
    BKNI_Free(buf);
    NEXUS_FileRecord_Close(file_write);
    NEXUS_FilePlay_Close(file);

    /* open new file */
    file_cut = NEXUS_FilePlay_OpenPosix(dest_fname, dest_navname);
    BDBG_ASSERT(file_cut);

    NEXUS_FilePlayOffset_GetDefaultSettings(&offsetSettings);
    /* fixup data offset */
    offsetSettings.indexOffset = 0;
    offsetSettings.dataOffset = start.mpegFileOffset;
    file = NEXUS_FilePlayOffset_Open(file_cut, &offsetSettings);
    BDBG_ASSERT(file);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Bring up video display and outputs */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif 
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
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

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* Open the audio and video pid channels */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = VIDEO_CODEC; /* must be told codec for correct handling */
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, &playbackPidSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    /* Start decoders */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    BKNI_Sleep(1000);
    NEXUS_Playback_GetStatus(playback, &status);
    fprintf(stderr, "playback position: %u .. %u .. %u\n", (unsigned)status.first, (unsigned)status.position, (unsigned)status.last);
    /* Playback state machine is driven from inside Nexus. */
    printf("Press ENTER to quit\n");
    getchar();

    /* Bring down system */
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_FilePlay_Close(file_cut);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_Display_Close(display);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_Platform_Uninit();

#else
	printf("This application is not supported on this platform!\n");
#endif
    return 0;
}
#else /* NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("This application is not supported on this platform (needs video decoder)!\n");
    return 0;
}
#endif
