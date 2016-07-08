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
 *     Example application showing decode mixed with PCM data.
 *
 *****************************************************************************/
#include "nexus_platform.h"
#include "nexus_platform_features.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO && NEXUS_NUM_AUDIO_PLAYBACKS
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
#include "nexus_audio_capture.h"
#include "nexus_display.h"
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

BDBG_MODULE(audio_playback_suspend);

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

bool suspended = false;
FILE *pFile = NULL;
BKNI_EventHandle event;
bool done = false;

int main(int argc, char **argv)
{

    NEXUS_PlatformConfiguration config;
    NEXUS_AudioPlaybackHandle playback;
    NEXUS_PlatformSettings platformSettings;
    pthread_t playbackThread;
#if NEXUS_HAS_HDMI_OUTPUT
    NEXUS_DisplayHandle display=NULL;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_HdmiOutputStatus hdmiStatus;
    unsigned timeout=100;
#endif

    if (argc > 1)
    {
        pFile = fopen(argv[1], "rb");
        if ( NULL == pFile )
        {
            fprintf(stderr, "Unable to open '%s' for reading\n", argv[1]);
            return -1;
        }
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&config);

    BKNI_CreateEvent(&event);

    playback = NEXUS_AudioPlayback_Open(0, NULL);
    if ( NULL == playback )
    {
        fprintf(stderr, "Unable to open playback channel\n");
        return 0;
    }

#if NEXUS_HAS_HDMI_OUTPUT
    /* Look for HDMI */
    for ( timeout = 100; timeout > 0; timeout-- )
    {
        NEXUS_HdmiOutput_GetStatus(config.outputs.hdmi[0], &hdmiStatus);
        if ( hdmiStatus.connected )
        {
            NEXUS_Display_GetDefaultSettings(&displaySettings);
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            display = NEXUS_Display_Open(0, &displaySettings);
            if ( display )
            {
                NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(config.outputs.hdmi[0]));
            }
            break;
        }
        BKNI_Sleep(10);
    }

    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(config.outputs.hdmi[0]),
                               NEXUS_AudioPlayback_GetConnector(playback));
#endif

#if NEXUS_NUM_AUDIO_DACS
    /* Connect DAC/HDMI to playback */
    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]),
                               NEXUS_AudioPlayback_GetConnector(playback));

#endif

    /* Start threads for decoder and capture */
    pthread_create(&playbackThread, NULL, playback_thread, playback);

    while (!done)
    {
        int tmp;

        /* Display Menu */
        printf("Main Menu\n");
        printf(" 0) Exit\n");
        printf(" 1) %s the playback\n", suspended?"Resumes":"Suspended");
        printf("Enter Selection: \n");
        scanf("%d", &tmp);

        switch ( tmp )
        {
            case 0:
                done = true;
                break;
            case 1:
                if (!suspended)
                {
                    NEXUS_AudioPlayback_Suspend(playback);
                }
                else
                {
                    NEXUS_AudioPlayback_Resume(playback);
                }
                suspended = !suspended;
                break;
        }
    }

    pthread_join(playbackThread, NULL);
    NEXUS_AudioPlayback_Stop(playback);
    NEXUS_AudioPlayback_Close(playback);
#if NEXUS_HAS_HDMI_OUTPUT
    NEXUS_Display_Close(display);
#endif
    BKNI_DestroyEvent(event);
    NEXUS_Platform_Uninit();
    printf("Done\n");
    return 0;
}

static void *playback_thread(void *pParam)
{
    NEXUS_AudioPlaybackHandle playback = pParam;
    NEXUS_AudioPlaybackStartSettings playbackSettings;
    NEXUS_Error errCode;
    int16_t *pBuffer;
    NEXUS_PlatformConfiguration config;
    NEXUS_AudioOutputSettings outputSettings;
    dataCallbackParameters dataCBParams;

    size_t bytesToPlay = 48000*4*120;    /* 48 kHz, 4 bytes/sample, 120 seconds */
    size_t bytesPlayed=0;
    size_t offset=0;
    size_t bufferSize;

    NEXUS_Platform_GetConfiguration(&config);

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

    switch ( playbackSettings.sampleRate )
    {
    case 32000:
    case 44100:
#if NEXUS_NUM_AUDIO_DACS
        NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]), &outputSettings);
        outputSettings.defaultSampleRate = playbackSettings.sampleRate;
        NEXUS_AudioOutput_SetSettings(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]), &outputSettings);
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(config.outputs.hdmi[0]), &outputSettings);
        outputSettings.defaultSampleRate = playbackSettings.sampleRate;
        NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(config.outputs.hdmi[0]), &outputSettings);
#endif
        break;
    default:
        break;
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
            /* If we are suspended then we won't be any events to wait for */
            if (!suspended)
            {
                /* Wait for data callback */
                errCode = BKNI_WaitForEvent(event, 1000);
                if ( errCode )
                {
                    printf("error waiting for event\n");
                }
            }

        }
    } while ( (BERR_SUCCESS == errCode || suspended) && bytesPlayed < bytesToPlay && !done);

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
