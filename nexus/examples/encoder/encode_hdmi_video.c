/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
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
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_VIDEO_ENCODER && NEXUS_HAS_HDMI_INPUT
#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_video_encoder_output.h"
#include "nexus_file.h"
#include "nexus_video_encoder.h"
#include "nexus_hdmi_input.h"
#include "nexus_timebase.h"

#include <stdio.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"

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

static void avInfoFrameChangedCallback(void *context, int param)
{
	NEXUS_HdmiInputStatus status;
	NEXUS_HdmiInputHandle hdmiInput = (NEXUS_HdmiInputHandle)context;

	BSTD_UNUSED(param);
	NEXUS_HdmiInput_GetStatus(hdmiInput, &status);
	fprintf(stderr, "HDMI Source AV InfoFrame Change callback: video format %ux%u@%.3f%c; color space: %d\n",
		status.avWidth,
		status.avHeight,
		(double)status.vertFreq/100,
		status.interlaced? 'i' : 'p',
		status.colorSpace);
}

int main(void)  {
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;

    NEXUS_DisplayHandle displayTranscode;
    NEXUS_VideoWindowHandle windowTranscode;
    NEXUS_VideoEncoderCapabilities videoEncoderCap;
    NEXUS_VideoEncoderHandle videoEncoder;
    NEXUS_VideoEncoderSettings videoEncoderConfig;
    NEXUS_VideoEncoderStartSettings videoEncoderStartConfig;
    NEXUS_VideoEncoderDelayRange delayRange;
    NEXUS_VideoEncoderStatus videoEncoderStatus;
    FILE *fout;
    size_t bytes;
    NEXUS_HdmiInputHandle hdmiInput;
    NEXUS_HdmiInputSettings hdmiInputSettings;
    NEXUS_TimebaseSettings timebaseSettings;
    void *pDataBuffer;
    NEXUS_DisplaySettings displaySettings;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.autoConfigTimebase = false;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    NEXUS_Timebase_GetSettings(NEXUS_Timebase_e0, &timebaseSettings);
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eHdDviIn;
    NEXUS_Timebase_SetSettings(NEXUS_Timebase_e0, &timebaseSettings);

    NEXUS_HdmiInput_GetDefaultSettings(&hdmiInputSettings);
    hdmiInputSettings.timebase = NEXUS_Timebase_e0;
    /* use NEXUS_HdmiInput_OpenWithEdid ()
        if EDID PROM (U1304 and U1305) is NOT installed;
        reference boards usually have the PROMs installed.
        this example assumes Port1 EDID has been removed
    */

    /* all HDMI Tx/Rx combo chips have EDID RAM */
    hdmiInputSettings.useInternalEdid = true ;
    hdmiInput = NEXUS_HdmiInput_OpenWithEdid(0, &hdmiInputSettings, &SampleEDID[0], (uint16_t) sizeof(SampleEDID));
    if(!hdmiInput) {
        fprintf(stderr, "Can't get hdmi input\n");
        return -1;
    }
    NEXUS_HdmiInput_GetSettings(hdmiInput, &hdmiInputSettings);
    hdmiInputSettings.aviInfoFrameChanged.callback = avInfoFrameChangedCallback;
    hdmiInputSettings.aviInfoFrameChanged.context  = hdmiInput;
    NEXUS_HdmiInput_SetSettings(hdmiInput, &hdmiInputSettings);

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
    displaySettings.format = NEXUS_VideoFormat_e720p;
    displayTranscode = NEXUS_Display_Open(videoEncoderCap.videoEncoder[0].displayIndex, &displaySettings);
    assert(displayTranscode);

    windowTranscode = NEXUS_VideoWindow_Open(displayTranscode, 0);
    assert(windowTranscode);

    /* connect same decoder to encoder display
     * This simul mode is for video encoder bringup only; audio path may have limitation
     * for simul display+transcode mode;
     */
    NEXUS_VideoWindow_AddInput(windowTranscode, NEXUS_HdmiInput_GetVideoConnector(hdmiInput));


    NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
    videoEncoderConfig.variableFrameRate = true;
    videoEncoderConfig.bitrateMax = 3*1000*1000;
    videoEncoderConfig.streamStructure.framesP = 29;
    videoEncoderConfig.streamStructure.framesB = 0;

    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);
    videoEncoderStartConfig.codec = NEXUS_VideoCodec_eH264;
    videoEncoderStartConfig.profile = NEXUS_VideoCodecProfile_eBaseline;
    videoEncoderStartConfig.level = NEXUS_VideoCodecLevel_e31;
    videoEncoderStartConfig.input = displayTranscode;
    videoEncoderStartConfig.stcChannel = stcChannel;

	NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &delayRange);
	printf("\t encoder A2P delay = %u ms\n", delayRange.min/27000);
    videoEncoderConfig.encoderDelay = delayRange.min;
    NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);
    NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);
    NEXUS_VideoEncoder_GetStatus(videoEncoder, &videoEncoderStatus);
    NEXUS_MemoryBlock_Lock(videoEncoderStatus.bufferBlock, &pDataBuffer);

    fout = fopen("videos/hdmi.mpg","wb");
    assert(fout);

    for(bytes=0;;) {
        size_t size[2];
        const NEXUS_VideoEncoderDescriptor *desc[2];
        unsigned i,j;
        unsigned descs;


        NEXUS_VideoEncoder_GetBuffer(videoEncoder, &desc[0], &size[0], &desc[1], &size[1]);
        if(size[0]==0 && size[1]==0) {
            fflush(fout);
            fprintf(stderr, "written %u bytes....\t\r", bytes);
            BKNI_Sleep(30);
            continue;
        }
        for(descs=0,j=0;j<2;j++) {
            descs+=size[j];
            for(i=0;i<size[j];i++) {
                if((desc[j][i].flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_METADATA) ==0) {/* ignore metadata descriptor in es capture */
                    fwrite((const uint8_t *)pDataBuffer + desc[j][i].offset, desc[j][i].length, 1, fout);
                    bytes+= desc[j][i].length;
                }
            }
        }
        NEXUS_VideoEncoder_ReadComplete(videoEncoder, descs);
    }
    fclose(fout);

    /* Bring down system */
    NEXUS_VideoInput_Shutdown(NEXUS_HdmiInput_GetVideoConnector(hdmiInput));
    NEXUS_StcChannel_Close(stcChannel);

    NEXUS_Display_Close(displayTranscode);
    NEXUS_VideoEncoder_Close(videoEncoder);

    NEXUS_Platform_Uninit();

    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
