/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <stdio.h>

#if NEXUS_HAS_AUDIO
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
#include <string.h>

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

int main(int argc, char **argv)
{
    BERR_Code errCode;
    BKNI_EventHandle event;
    NEXUS_PlatformConfiguration config;

    NEXUS_AudioPlaybackHandle playback;
    NEXUS_AudioPlaybackStartSettings playbackSettings;

    NEXUS_AudioMixerHandle mixer;
    NEXUS_AudioDecoderHandle decoder;
    NEXUS_AudioDecoderStartSettings decoderSettings;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    dataCallbackParameters dataCBParams;
    NEXUS_AudioCapabilities audioCapabilities;
    NEXUS_AudioOutputHandle audioDacHandle = NULL;

    size_t bytesToPlay = 48000*4*20;    /* 48 kHz, 4 bytes/sample, 20 seconds */
    size_t bytesPlayed=0;
    size_t offset=0;
    int16_t *pBuffer;
    size_t bufferSize;
    FILE *pFile = NULL;

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
    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (audioCapabilities.numDecoders == 0 ||
        audioCapabilities.numPlaybacks == 0 ||
        audioCapabilities.numMixers == 0)
    {
        printf("This application is not supported on this platform (requires decoder, playbacks and mixers).\n");
        return 0;
    }

    if (audioCapabilities.numOutputs.dac > 0)
    {
        audioDacHandle = NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]);
    }

    BKNI_CreateEvent(&event);

    playback = NEXUS_AudioPlayback_Open(0, NULL);
    if ( NULL == playback )
    {
        fprintf(stderr, "Unable to open playback channel\n");
        return 0;
    }

    decoder = NEXUS_AudioDecoder_Open(0, NULL);
    if ( NULL == decoder )
    {
        fprintf(stderr, "Unable to open decoder channel\n");
        return 0;
    }

    mixer = NEXUS_AudioMixer_Open(NULL);
    if ( NULL == mixer )
    {
        fprintf(stderr, "Unable to open mixer\n");
        return 0;
    }

    /* Setup mixer inputs */
    NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioPlayback_GetConnector(playback));
    NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(decoder, NEXUS_AudioDecoderConnectorType_eStereo));

    /* Connect DAC to mixer */
    if (audioDacHandle) {
        NEXUS_AudioOutput_AddInput(audioDacHandle,
                                   NEXUS_AudioMixer_GetConnector(mixer));
    }
    /* Setup transport */
    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);
    /* Map a parser band to the streamer input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Setup StcChannel to sync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = NEXUS_PidChannel_Open(parserBand, 0x11, NULL);    /* Open PCR PID 0x11 */

    /* Start audio decode */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&decoderSettings);
    decoderSettings.codec = NEXUS_AudioCodec_eAc3;
    decoderSettings.pidChannel = NEXUS_PidChannel_Open(parserBand, 0x14, NULL);     /* Open audio PID 0x14 */
    decoderSettings.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);            /* Create STC Channel with above settings */
    NEXUS_AudioDecoder_Start(decoder, &decoderSettings);


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
                    return -1;
                }
                if ( !strncmp("fmt ", tmp, 4) )
                {
                    unsigned long sampleRate;

                    /* check  chunk length */
                    if ( 16 != fgetc(pFile) )
                    {
                        fprintf(stderr, "Only PCM .wav files are supported\n");
                        return -1;
                    }
                    fseek(pFile, 3, SEEK_CUR);
                    /* check format */
                    if ( fgetc(pFile) != 0x01 )
                    {
                        fprintf(stderr, "Only PCM .wav files are supported\n");
                        return -1;
                    }
                    fgetc(pFile);   /* Skip next byte */
                    if ( fgetc(pFile) != 0x02 )
                    {
                        fprintf(stderr, "Only stereo .wav files are supported\n");
                        return -1;
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

    errCode = NEXUS_AudioPlayback_Start(playback, &playbackSettings);
    BDBG_ASSERT(!errCode);

    if ( pFile )
    {
        printf("Starting playback of file '%s'\n", argv[1]);
    }
    else
    {
        printf("Starting playback for 20 seconds.\n");
    }

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
            errCode = BKNI_WaitForEvent(event, 1000);
        }
    } while ( BERR_SUCCESS == errCode && bytesPlayed < bytesToPlay );

    printf("Stopping playback\n");
    NEXUS_AudioPlayback_Stop(playback);

    printf("Done\n");
    return 0;
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
