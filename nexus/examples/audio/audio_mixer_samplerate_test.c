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
 *
 * Module Description:
 *     Example application showing decode mixed with PCM data.
 *
 *****************************************************************************/
#include <stdio.h>
#include "nexus_platform.h"
#if NEXUS_HAS_AUDIO && NEXUS_NUM_AUDIO_PLAYBACKS && NEXUS_HAS_PLAYBACK
#include "nexus_platform_features.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nexus_audio_playback.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_mixer.h"
#include "nexus_stc_channel.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_playback.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_file.h"
#include <string.h>
#include <pthread.h>
#include <assert.h>

/* 1KHz sine wave at 48 KHz */
static int16_t samples[48] =
{
0,
4276,
8480,
12539,
16383,
19947,
23169,
25995,
28377,
30272,
31650,
32486,
32767,
32486,
31650,
30272,
28377,
25995,
23169,
19947,
16383,
12539,
8480,
4276,
0,
-4277,
-8481,
-12540,
-16384,
-19948,
-23170,
-25996,
-28378,
-30273,
-31651,
-32487,
-32767,
-32487,
-31651,
-30273,
-28378,
-25996,
-23170,
-19948,
-16384,
-12540,
-8481,
-4277
};


/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME44 "videos/silence_44_1k.mp3"
#define FILE_NAME48 "videos/silence_48k.mp3"
#define TRANSPORT_TYPE NEXUS_TransportType_eEs
#define AUDIO_CODEC NEXUS_AudioCodec_eMp3
#define AUDIO_PID 0x1

BDBG_MODULE(audio_playback);

typedef struct dataCallbackParameters
{
    NEXUS_AudioPlaybackHandle playback;
    BKNI_EventHandle event;
} dataCallbackParameters;

static void data_callback(void *pParam1, int param2)
{
    dataCallbackParameters *dataCBParams;

    dataCBParams = (dataCallbackParameters *)pParam1;
    /*
    printf("Data callback - channel 0x%08x\n", (unsigned)dataCBParams->playback);
    */

    BKNI_SetEvent(dataCBParams->event);
}

static void *playback_thread(void *pParam);

FILE *pFile = NULL;
BKNI_EventHandle event;
bool done = false;
bool playbackDone = false;
NEXUS_AudioPlaybackStartSettings playbackSettings;

int main(int argc, char **argv)
{
    NEXUS_PlatformConfiguration config;
    NEXUS_AudioPlaybackHandle audioPlayback;
    NEXUS_AudioMixerHandle mixer;
    NEXUS_AudioMixerSettings mixerSettings;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    pthread_t playbackThread;
    NEXUS_FilePlayHandle file44;
    NEXUS_FilePlayHandle file48;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PlaybackPidChannelSettings pidSettings;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_HdmiOutputStatus hdmiStatus;
    BERR_Code rc;
    const char *fname44 = FILE_NAME44;
    const char *fname48 = FILE_NAME48;
    bool fourtyeight = true;
    bool started = true;


    if (argc > 1)
    {
        pFile = fopen(argv[1], "rb");
        if ( NULL == pFile )
        {
            fprintf(stderr, "Unable to open '%s' for reading\n", argv[1]);
            return -1;
        }
    }

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&config);

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    file44 = NEXUS_FilePlay_OpenPosix(fname44, NULL);
    if (!file44) {
        fprintf(stderr, "can't open file:%s\n", fname44);
        return -1;
    }

    file48 = NEXUS_FilePlay_OpenPosix(fname48, NULL);
    if (!file48) {
        fprintf(stderr, "can't open file:%s\n", fname48);
        return -1;
    }

    BKNI_CreateEvent(&event);

    audioPlayback = NEXUS_AudioPlayback_Open(0, NULL);
    if ( NULL == audioPlayback )
    {
        fprintf(stderr, "Unable to open playback channel\n");
        return 0;
    }

    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    if ( NULL == audioDecoder )
    {
        fprintf(stderr, "Unable to open decoder channel\n");
        return 0;
    }

    NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
    mixerSettings.mixUsingDsp = true;
    mixer = NEXUS_AudioMixer_Open(&mixerSettings);
    if ( NULL == mixer )
    {
        fprintf(stderr, "Unable to open mixer\n");
        return 0;
    }

    /* Setup mixer inputs */
    NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioPlayback_GetConnector(audioPlayback));
    NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

    NEXUS_AudioMixer_GetSettings(mixer, &mixerSettings);
    mixerSettings.outputSampleRate = 48000;
    mixerSettings.master = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    NEXUS_AudioMixer_SetSettings(mixer, &mixerSettings);

#if NEXUS_NUM_HDMI_OUTPUTS
    /* Bring up video display and outputs */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(config.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(config.outputs.hdmi[0], &hdmiStatus);
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
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(config.outputs.hdmi[0]),
        NEXUS_AudioMixer_GetConnector(mixer));

#endif

    /* Connect DAC to mixer */
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]),
                               NEXUS_AudioMixer_GetConnector(mixer));
#endif

#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutputSettings outputSettings;
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]),
            NEXUS_AudioMixer_GetConnector(mixer));
#endif

    /* Setup transport */

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Tell playpump that it's ES. */
    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = TRANSPORT_TYPE;
    playbackSettings.stcChannel = stcChannel;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);


    NEXUS_Playback_GetDefaultPidChannelSettings(&pidSettings);
    pidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    pidSettings.pidSettings.pidTypeSettings.audio.codec = AUDIO_CODEC;
    pidSettings.pidTypeSettings.audio.primary = audioDecoder;
    audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, AUDIO_PID, &pidSettings);


    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = AUDIO_CODEC;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    NEXUS_AudioMixer_Start(mixer);

    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
    if (fourtyeight)
    {
        NEXUS_Playback_Start(playback, file48, NULL);
    }
    else
    {
        NEXUS_Playback_Start(playback, file44, NULL);
    }
    pthread_create(&playbackThread, NULL, playback_thread, audioPlayback);

    while (!done)
    {
       int tmp;

       /* Display Menu */
       printf("Main Menu\n");
       printf(" 0) Exit\n");
       printf(" 1) %s decoder\n",started?"STOP":"START");
       printf(" 2) switch to %s file\n",fourtyeight?"44_1k":"48k");
       printf(" 3) %s playback\n", playbackDone?"START":"STOP");
       scanf("%d", &tmp);
       switch ( tmp )
       {
       case 0:
           done=true;
           break;
       case 1:
           if (started)
           {
               NEXUS_AudioDecoder_Stop(audioDecoder);
               NEXUS_Playback_Stop(playback);
           }
           else
           {
               NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
               if (fourtyeight)
               {
                   NEXUS_Playback_Start(playback, file48, NULL);
               }
               else
               {
                   NEXUS_Playback_Start(playback, file44, NULL);
               }
           }
           started = !started;
           break;
       case 2:
           if (started)
           {
               NEXUS_AudioDecoder_Stop(audioDecoder);
               NEXUS_Playback_Stop(playback);
               if (fourtyeight)
               {

                       NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
                       NEXUS_Playback_Start(playback, file44, NULL);
               }
               else
               {
                   NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);
                   NEXUS_Playback_Start(playback, file48, NULL);
               }
           }
           fourtyeight = !fourtyeight;
           break;
       case 3:
           if (playbackDone)
           {
               playbackDone = !playbackDone;
               pthread_create(&playbackThread, NULL, playback_thread, audioPlayback);
           }
           else
           {
               playbackDone = !playbackDone;
               pthread_join(playbackThread, NULL);
           }
           break;
       }
   }

   NEXUS_Playback_Stop(playback);
   NEXUS_AudioDecoder_Stop(audioDecoder);

   playbackDone = true;
   pthread_join(playbackThread, NULL);
   NEXUS_AudioMixer_RemoveInput(mixer, NEXUS_AudioPlayback_GetConnector(audioPlayback));
   NEXUS_AudioMixer_RemoveInput(mixer, NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
   NEXUS_AudioMixer_Stop(mixer);
   NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eStereo));
   NEXUS_AudioDecoder_Close(audioDecoder);
   NEXUS_AudioPlayback_Close(audioPlayback);
   NEXUS_AudioMixer_Close(mixer);
#if NEXUS_HAS_HDMI_OUTPUT
   NEXUS_VideoWindow_Close(window);
   NEXUS_Display_Close(display);
#endif
   NEXUS_Playback_CloseAllPidChannels(playback);
   NEXUS_FilePlay_Close(file48);
   NEXUS_FilePlay_Close(file44);
   NEXUS_Playback_Destroy(playback);
   NEXUS_Playpump_Close(playpump);

   NEXUS_StcChannel_Close(stcChannel);
   BKNI_DestroyEvent(event);
   NEXUS_Platform_Uninit();
   printf("Done\n");
   return 0;
}

static void *playback_thread(void *pParam)
{
    NEXUS_AudioPlaybackHandle playback = pParam;
    NEXUS_Error errCode;
    int16_t *pBuffer;
    dataCallbackParameters dataCBParams;

    size_t bytesToPlay = 48000*4*120;    /* 48 kHz, 4 bytes/sample, 120 seconds */
    size_t bytesPlayed=0;
    size_t offset=0;
    size_t bufferSize;


    /* Setup main playback */
    NEXUS_AudioPlayback_GetDefaultStartSettings(&playbackSettings);
    playbackSettings.sampleRate = 48000;
    playbackSettings.bitsPerSample = 16;
    playbackSettings.signedData = true;
    playbackSettings.stereo = true;
    playbackSettings.dataCallback.callback = data_callback;
    dataCBParams.event = event;
    dataCBParams.playback = playback;
    playbackSettings.dataCallback.context = &dataCBParams;

    /* If we have a wav file, get the sample rate from it */
    if ( pFile )
    {
        char tmp[4];
        fseek(pFile, 0, SEEK_SET);  /* Skip file length and WAVE */
        fread(tmp, 1, 4, pFile);
        if ( !strncmp("RIFF", tmp, 4) )
        {
            printf(".wav file detected\n");
            fseek(pFile, 8, SEEK_CUR);  /* Skip file length and WAVE */
            /* WAV file.  Look for fmt chunk. */
            for ( ;; )
            {
                if ( 0 == fread(tmp, 1, 4, pFile) )
                {
                    fprintf(stderr, "end of file reached, truncated file?\n");
                    return NULL;
                }
                if ( !strncmp("fmt ", tmp, 4) )
                {
                    unsigned long sampleRate;

                    /* check  chunk length */
                    if ( 16 != fgetc(pFile) )
                    {
                        fprintf(stderr, "Only PCM .wav files are supported\n");
                        return NULL;
                    }
                    fseek(pFile, 3, SEEK_CUR);
                    /* check format */
                    if ( fgetc(pFile) != 0x01 )
                    {
                        fprintf(stderr, "Only PCM .wav files are supported\n");
                        return NULL;
                    }
                    fgetc(pFile);   /* Skip next byte */
                    if ( fgetc(pFile) != 0x02 )
                    {
                        fprintf(stderr, "Only stereo .wav files are supported\n");
                        return NULL;
                    }
                    fgetc(pFile);   /* Skip next byte */
                    sampleRate = fgetc(pFile);
                    sampleRate |= ((unsigned long)fgetc(pFile))<<8;
                    sampleRate |= ((unsigned long)fgetc(pFile))<<16;
                    sampleRate |= ((unsigned long)fgetc(pFile))<<24;
                    playbackSettings.sampleRate = sampleRate;
                    printf(".wav file sample rate %lu Hz\n", sampleRate);
                    /* Skip remainder of fmt chunk */
                    fseek(pFile, 8, SEEK_CUR);
                }
                else if ( !strncmp("data", tmp, 4) )
                {
                    /* Reached data.  Stop looking. */
                    fseek(pFile, 4, SEEK_CUR);
                    playbackSettings.endian = NEXUS_EndianMode_eLittle;
                    break;
                }
                else
                {
                    unsigned int skip;
                    skip = fgetc(pFile);
                    skip |= ((unsigned long)fgetc(pFile))<<8;
                    skip |= ((unsigned long)fgetc(pFile))<<16;
                    skip |= ((unsigned long)fgetc(pFile))<<24;
                    fseek(pFile, skip, SEEK_CUR);
                }
            }
        }
        else
        {
            fprintf(stderr, "RAW data file detected.\n");
            fseek(pFile, 0, SEEK_SET);
        }
    }

    BDBG_WRN(("Starting Playback"));
    errCode = NEXUS_AudioPlayback_Start(playback, &playbackSettings);
    BDBG_ASSERT(!errCode);


    do
    {
        unsigned i;
        /* Check available buffer space */
        errCode = NEXUS_AudioPlayback_GetBuffer(playback, (void **)&pBuffer, &bufferSize);
        if ( errCode )
        {
            printf("Error getting playback buffer\n");
            break;
        }

        if (bufferSize)
        {
            if ( pFile )
            {
                bufferSize = fread(pBuffer, 1, bufferSize, pFile);

                if ( 0 == bufferSize )
                {
                    break;
                }
            }
            else
            {
                /* Copy samples into buffer */
                bufferSize /= 4;
                for ( i=0; i<bufferSize; i++,bytesPlayed+=4 )
                {
                    pBuffer[2*i] = pBuffer[(2*i)+1] = samples[offset];
                    offset++;
                    if ( offset >= 48 )
                    {
                        offset = 0;
                    }
                }
                bufferSize *= 4;
            }

            errCode = NEXUS_AudioPlayback_WriteComplete(playback, bufferSize);
            if ( errCode )
            {
                printf("Error committing playback buffer\n");
                break;
            }
        }
        else
        {
            /* Wait for data callback */
            errCode = BKNI_WaitForEvent(event, 5000);
            if ( errCode )
            {
                printf("error waiting for event\n");
            }
        }
    } while ( (BERR_SUCCESS == errCode) && bytesPlayed < bytesToPlay && !playbackDone && !done && !playbackDone);

    BDBG_WRN(("Stopping Playback"));
    NEXUS_AudioPlayback_Stop(playback);
    return NULL;
}

#else
int main(int argc, char **argv)
{
    fprintf(stderr, "No audio playback support\n");
    argc=argc;
    argv=argv;
    return 0;
}
#endif
