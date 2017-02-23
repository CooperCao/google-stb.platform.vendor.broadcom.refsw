/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
  *****************************************************************************/
/* Nexus example app: playback, decode, and paced PES message extracting */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_AUDIO
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_message.h"
#include "nexus_playback.h"
#include "nexus_file.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "sync_manager.h"

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "videos/cnnticker.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC NEXUS_AudioCodec_eMpeg
#define VIDEO_PID 0x258
#define AUDIO_PID 0x259

#define B_GET_BIT(w,b)  ((w)&(1<<(b)))
#define B_GET_BITS(w,e,b)  (((w)>>(b))&(((unsigned)(-1))>>((sizeof(unsigned))*8-(e+1-b))))

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel, pesPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_MessageSettings messageSettings;
    NEXUS_MessageStartSettings messageStartSettings;
    NEXUS_MessageHandle msg;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
#endif
    NEXUS_Error rc;
    const char *fname = FILE_NAME;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    if (argc > 1) fname = argv[1];

    file = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }
    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Open the audio and video pid channels */
    videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, VIDEO_PID, NULL);
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, AUDIO_PID, NULL);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Bring up audio decoders and outputs */
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

    /* Bring up video display and outputs */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

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

    /* bring up decoder and connect to display */
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    /* Start decoders */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

    NEXUS_Message_GetDefaultSettings(&messageSettings);
    msg = NEXUS_Message_Open(&messageSettings);

    NEXUS_Message_GetDefaultStartSettings(msg, &messageStartSettings);
    messageStartSettings.format = NEXUS_MessageFormat_ePesSaveAll;

    /* open two channels & message filter */
    pesPidChannel = NEXUS_Playback_OpenPidChannel(playback, 0x25B, NULL);
    messageStartSettings.pidChannel = pesPidChannel;
    rc = NEXUS_Message_Start(msg, &messageStartSettings);
    BDBG_ASSERT(!rc);

    /* Start playback */
    NEXUS_Playback_Start(playback, file, NULL);

    {
        uint8_t pes_buffer[64*1024+32];
        size_t pes_len=0;
        size_t data_len=0;
        FILE *fout=fopen("data.pes","w");
        btsm_queue_config tsm_config;
        btsm_queue_t tsm;

        btsm_queue_get_default_config(&tsm_config);
        tsm = btsm_queue_create(&tsm_config);
        btsm_queue_start(tsm, NULL);


        for(;;) {
            const void *buffer;
            size_t size;
            unsigned offset;
            btsm_queue_entry tsm_entry;
            uint32_t stc;
            btsm_queue_action tsm_action;

            NEXUS_StcChannel_GetStc(stcChannel, &stc);
            tsm_action = btsm_queue_get(tsm, stc, &tsm_entry);
            if(tsm_action!=btsm_queue_action_repeat) {
                fprintf(stderr, "PES OUT %s %#lx %u(%u)\n", tsm_action==btsm_queue_action_drop?"DROP":"", (unsigned long)tsm_entry.element, (unsigned)tsm_entry.timestamp, (unsigned)stc);
                BKNI_Free(tsm_entry.element);
            }

            rc = NEXUS_Message_GetBuffer(msg, &buffer, &size);
            BDBG_ASSERT(!rc);

            if (!size) {
                BKNI_Sleep(30);
                continue;
            }
            /* fprintf(stderr, "data %u\n", size); */
            fwrite(buffer, 1, size, fout);
            fflush(fout);
            for(offset=0;offset<size;) {
                size_t to_copy = size-offset;
                if(pes_len==0) {
                    if(data_len + to_copy > sizeof(pes_buffer)) {
                        to_copy = sizeof(pes_buffer) - data_len;
                    }
                } else {
                    if(data_len + to_copy > pes_len) {
                        if(pes_len >= data_len) {
                            to_copy = pes_len - data_len;
                        } else {
                            to_copy = 0;
                        }
                    }
                }

                BKNI_Memcpy(pes_buffer+data_len,  (uint8_t *)buffer + offset, to_copy);
                offset += to_copy;
                data_len += to_copy;
                for(;;) {
                    if(pes_len==0) {
                        unsigned i;
                        uint32_t scode;
                        bool pes_scode;
                        for(pes_scode=false,i=0,scode=0xFF;i<data_len;i++) {
                            scode = (scode<<8)|pes_buffer[i];
                            if((scode&0xFFFFFF00) == 0x100) {
                                /* fprintf(stderr, "found %#lx at %u\n", (unsigned long)scode, i); */
                                scode = scode&0xFF;
                                switch(scode) {
                                case 0xBC: /* program_stream_map */
                                case 0xBE: /* padding_stream */
                                case 0xBF: /* private_stream_2 */
                                case 0xF0: /* ECM_stream */
                                case 0xF1: /* EMM_stream */
                                case 0xF2: /* ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Annex A or ISO/IEC 13818- 6_DSMCC_stream */
                                case 0xF8: /* ITU-T Rec. H.222.1 type E */
                                case 0xFF: /* program_stream_directory */
                                    pes_scode = true;
                                    break;
                                default:
                                    if(scode>= 0xBD) {
                                        pes_scode = true;
                                    }
                                    break;
                                }
                                if(pes_scode) { break;}
                                scode = 0xFF;
                            }
                        }
                        if(i>3) {
                           i-=3;
                           data_len -= i;
                           BKNI_Memmove(pes_buffer, (pes_buffer+i), data_len);
                        }
                        if(pes_scode && data_len>8) {
                            pes_len = ((unsigned)pes_buffer[4]<<8) | pes_buffer[5];
                            if(pes_len>0) {
                                pes_len += 6;
                            } else {
                                pes_buffer[3] = 0; /* destroy PES header */
                            }
                        }
                    }
                    if(pes_len==0) {
                        break;
                    }
                    if(data_len >= pes_len) {
                        uint8_t *buffer = BKNI_Malloc(pes_len);

                        BTSM_QUEUE_ENTRY_INIT(&tsm_entry);

#if 1
                        /* use PTS from payload */
                        if(B_GET_BITS(pes_buffer[7], 7, 6)&2) {
                            uint32_t pts_32_30 = B_GET_BITS(pes_buffer[9], 3, 1);
                            uint32_t pts_29_22 = pes_buffer[10];
                            uint32_t pts_21_15 = B_GET_BITS(pes_buffer[11], 7, 1);
                            uint32_t pts_14_7 = pes_buffer[12];
                            uint32_t pts_6_0 = B_GET_BITS(pes_buffer[13], 7, 2);
                            uint32_t pts;

                            pts = (pts_32_30<<29) | (pts_29_22<<21) | (pts_21_15<<14) | (pts_14_7<<6)|pts_6_0;
                            BTSM_QUEUE_ENTRY_SET_TIMESTAMP(&tsm_entry, pts);
                        }
#else
                        NEXUS_StcChannel_GetStc(stcChannel, &stc);
                        BTSM_QUEUE_ENTRY_SET_TIMESTAMP(&tsm_entry, stc+200*45); /* use STC and delay data by 200 miliseconds */
#endif

                        fprintf(stderr, "PES IN %#lx:%u %u(%u) %#x\n", (unsigned long)buffer, pes_len, tsm_entry.timestamp, stc, pes_buffer[3]);
                        BKNI_Memcpy(buffer, pes_buffer, pes_len);
                        tsm_entry.element = buffer;
                        btsm_queue_push(tsm, &tsm_entry);
                        data_len -= pes_len;
                        BKNI_Memmove(pes_buffer, (pes_buffer+pes_len), data_len);
                        pes_len = 0;
                    } else {
                        break;
                    }
                }
            }
            rc = NEXUS_Message_ReadComplete(msg, size);
            BDBG_ASSERT(!rc);
        }
    }



    /* Playback state machine is driven from inside Nexus. */
    BKNI_Sleep(10000);

    /* Bring down system */
    NEXUS_VideoDecoder_Stop(videoDecoder);
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(videoDecoder));
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_AudioDecoder_Close(audioDecoder);
    NEXUS_Display_Close(display);
    NEXUS_Platform_Uninit();
    return 1;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
