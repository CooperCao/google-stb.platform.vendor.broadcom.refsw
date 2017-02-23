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
#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_video_output.h"
#include "nexus_video_window.h"
#include "nexus_video_image_input.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_output.h"
#include "bkni.h"
#include "bfile_stdio.h"
#include "bflv_decoder.h"
#include "bogg_decoder.h"
#include "nexus_video_adj.h"
#include "nexus_picture_ctrl.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_core_utils.h"
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(ffmpeg);


#undef perror
int
main(int argc, const char *argv[])
{
    NEXUS_Error errCode;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowSettings windowCfg;
    NEXUS_DisplayHandle display;
    NEXUS_VideoImageInputHandle imageInput;
    NEXUS_VideoInput  videoInput;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_AudioDecoderHandle pcmDecoder;
    const char *fname;
    FILE *fin;
    bfile_io_read_t fd;
    bflv_decoder_t flv;
    bflv_decoder_start_settings flvSettings;
    bflv_decoder_start_information flvInformation;

    NEXUS_AudioDecoderStartSettings audioProgram;
    bogg_decoder_t ogg;
    bogg_decoder_start_settings oggSettings;
    bogg_decoder_start_information oggInformation;
    NEXUS_VideoFormatInfo formatInfo;
    bool flv_file = false;
    bool split_screen = false;
    bool pip = argc>7;
    unsigned swdecoder_index = 1;

    NEXUS_VideoWindowSplitScreenSettings windowSplitCfg;
    NEXUS_VideoWindowDnrSettings dnrCfg;
    NEXUS_PictureCtrlCommonSettings pictureCfg;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    errCode = NEXUS_Platform_Init(&platformSettings);

    NEXUS_Platform_GetConfiguration(&platformConfig);


    fname = argc>1?argv[1]:"stream.flv";
    flv_file = strstr(fname, "flv")!=NULL;
    if(flv_file) {
        swdecoder_index = 0;
    }

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
#if 1
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
#else
    displaySettings.format = NEXUS_VideoFormat_e720p;
#endif
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_SCART_INPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ScartInput_GetVideoOutputConnector(platformConfig.inputs.scart[0]));
#if NEXUS_NUM_SCART_INPUTS>1
    NEXUS_Display_AddOutput(display, NEXUS_ScartInput_GetVideoOutputConnector(platformConfig.inputs.scart[1]));
#endif
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    if(displaySettings.format == NEXUS_VideoFormat_eNtsc) {
        NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    }
#endif
    window = NEXUS_VideoWindow_Open(display, swdecoder_index);
    NEXUS_VideoWindow_GetSettings(window, &windowCfg);

    windowCfg.visible = true;
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &formatInfo);
    if(pip) {
        windowCfg.position.x = formatInfo.width/2;
        windowCfg.position.y = 0;
        windowCfg.position.width = formatInfo.width/2;
        windowCfg.position.height = formatInfo.height/2;
    } else {
        windowCfg.position.x = 0;
        windowCfg.position.y = 0;
        windowCfg.position.width = formatInfo.width;
        windowCfg.position.height = formatInfo.height;
    }
    NEXUS_VideoWindow_SetSettings(window, &windowCfg);

    if(split_screen) {
        NEXUS_VideoWindow_GetSplitScreenSettings(window, &windowSplitCfg);
        windowSplitCfg.dnr = NEXUS_VideoWindowSplitScreenMode_eLeft;
        windowSplitCfg.brightness = NEXUS_VideoWindowSplitScreenMode_eRight;
        NEXUS_VideoWindow_SetSplitScreenSettings(window, &windowSplitCfg);

        NEXUS_PictureCtrl_GetCommonSettings(window, &pictureCfg);
        pictureCfg.brightness = -1*1024;
        NEXUS_PictureCtrl_SetCommonSettings(window, &pictureCfg);


        NEXUS_VideoWindow_GetDnrSettings(window, &dnrCfg);
        dnrCfg.mnr.mode = NEXUS_VideoWindowFilterMode_eEnable;
        dnrCfg.mnr.level = 0*1024;
        dnrCfg.bnr.mode = NEXUS_VideoWindowFilterMode_eEnable;
        dnrCfg.bnr.level = 0*1024;
        dnrCfg.dcr.mode = NEXUS_VideoWindowFilterMode_eEnable;
        dnrCfg.dcr.level = 0*1024;
        dnrCfg.qp = 0;
        NEXUS_VideoWindow_SetDnrSettings(window, &dnrCfg);
    }

    imageInput = NEXUS_VideoImageInput_Open(swdecoder_index, NULL);
    videoInput = NEXUS_VideoImageInput_GetConnector(imageInput);
    NEXUS_VideoWindow_AddInput(window, videoInput);

    stcChannel = NEXUS_StcChannel_Open(1, NULL);

    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

    playpump = NEXUS_Playpump_Open(0, NULL);

    if(flv_file) {
        fin = fopen(fname, "rb");
        if(!fin) { perror(fname);return -1; }
        fd = bfile_stdio_read_attach(fin);
        flv = bflv_decoder_create(NULL);

        bflv_decoder_get_default_start_settings(&flvSettings);
        flvSettings.fd = fd;
        flvSettings.imageInput = imageInput;
        flvSettings.stcChannel = stcChannel;
        flvSettings.playpump = playpump;
        flvSettings.audioDecoder = pcmDecoder;

        bflv_decoder_start(flv, &flvSettings, &flvInformation);

        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.codec = NEXUS_AudioCodec_eMp3;
        audioProgram.pidChannel = flvInformation.audioPid;
        audioProgram.stcChannel = stcChannel;
        NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
    } else {
        ogg = bogg_decoder_create(NULL);

        bogg_decoder_get_default_start_settings(&oggSettings);
        oggSettings.imageInput = imageInput;
        oggSettings.stcChannel = stcChannel;
        oggSettings.fname = fname;
        bogg_decoder_start(ogg, &oggSettings, &oggInformation);
        if(pip) {
            NEXUS_PlaybackHandle playback;
            NEXUS_FilePlayHandle file;
            unsigned pid;
            NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
            NEXUS_StcChannelHandle stcChannelPlay;
            NEXUS_VideoWindowHandle windowPlay;
            NEXUS_VideoDecoderHandle videoDecoder;
            NEXUS_PlaybackSettings playbackSettings;
            NEXUS_VideoDecoderStartSettings videoProgram;
            NEXUS_AudioDecoderStartSettings audioProgram;
            NEXUS_PlaybackPidChannelSettings playbackPidSettings;

            playback = NEXUS_Playback_Create();
            stcChannelPlay = NEXUS_StcChannel_Open(0, NULL);

            windowPlay = NEXUS_VideoWindow_Open(display, 1);
            NEXUS_VideoWindow_GetSettings(windowPlay, &windowCfg);

            windowCfg.visible = true;
            NEXUS_VideoWindow_SetSettings(windowPlay, &windowCfg);

            videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
            NEXUS_VideoWindow_AddInput(windowPlay, NEXUS_VideoDecoder_GetConnector(videoDecoder));

            NEXUS_Playback_GetSettings(playback, &playbackSettings);
            playbackSettings.playpump = playpump;
            playbackSettings.playpumpSettings.transportType = strtol(argv[2], NULL, 0);
            playbackSettings.stcChannel = stcChannelPlay;
            NEXUS_Playback_SetSettings(playback, &playbackSettings);


            pid = strtol(argv[3], NULL, 0);
            NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
            videoProgram.codec = strtol(argv[4], NULL, 0);
            videoProgram.stcChannel = stcChannelPlay;

            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            playbackPidSettings.pidTypeSettings.video.codec = videoProgram.codec;
            playbackPidSettings.pidTypeSettings.video.index = true;
            playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
            videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, pid, &playbackPidSettings);
            videoProgram.pidChannel = videoPidChannel;

            pid = strtol(argv[5], NULL, 0);
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            playbackPidSettings.pidTypeSettings.audio.primary = pcmDecoder;
            audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, pid, &playbackPidSettings);

            NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
            audioProgram.codec = strtol(argv[6], NULL, 0);
            audioProgram.pidChannel = audioPidChannel;
            audioProgram.stcChannel = stcChannelPlay;

            /* Start decoders */
            NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
            NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);


            file = NEXUS_FilePlay_OpenPosix(argv[7], argv[8]);
            if (!file) {
                fprintf(stderr, "can't open file:%s\n", fname);
                return -1;
            }
            NEXUS_Playback_Start(playback, file, NULL);
        }
     }
    getchar();

    BDBG_WRN(("shutdown[1]"));

    NEXUS_VideoImageInput_SetSurface(imageInput, NULL);
    BDBG_WRN(("shutdown[2]"));
    NEXUS_VideoWindow_RemoveAllInputs(window);
    NEXUS_VideoInput_Shutdown(videoInput);

#if 0
    bflv_decoder_destroy(flv);
    bfile_stdio_read_detach(fd);
    fclose(fin);
#endif

    NEXUS_VideoImageInput_Close(imageInput);

    BDBG_Uninit();
    BKNI_Uninit();
    return 0;
}
