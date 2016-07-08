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
#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO
#include "nexus_platform_features.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nexus_pid_channel.h"
#include "nexus_stc_channel.h"
#include "nexus_parser_band.h"
#include "nexus_i2s_input.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_capture.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_core_utils.h"
#include <string.h>
#include <stdlib.h>

#define CAPTURE_I2S_INPUT       0

typedef struct captureCallbackParameters
{
    NEXUS_AudioCaptureHandle capture;
    FILE *pFile;
} captureCallbackParameters;

static void capture_callback(void *pParam, int param)
{
    NEXUS_AudioCaptureHandle capture;
    FILE *pFile;
    NEXUS_Error errCode;
    captureCallbackParameters *captureCBParams;

    captureCBParams = (captureCallbackParameters *) pParam ;
    capture = captureCBParams->capture ;
    pFile = captureCBParams->pFile ;

    for ( ;; )
    {
        void *pBuffer;
        size_t bufferSize;

        /* Check available buffer space */
        errCode = NEXUS_AudioCapture_GetBuffer(capture, (void **)&pBuffer, &bufferSize);
        if ( errCode )
        {
            fprintf(stderr, "Error getting capture buffer\n");
            NEXUS_AudioCapture_Stop(capture);
            return;
        }

        if ( bufferSize > 0 )
        {
            /* Write samples to disk */
            if ( 1 != fwrite(pBuffer, bufferSize, 1, pFile) )
            {
                fprintf(stderr, "Error writing to disk\n");
                NEXUS_AudioCapture_Stop(capture);
                return;
            }

            /*fprintf(stderr, "Data callback - Wrote %d bytes\n", (int)bufferSize);*/
            errCode = NEXUS_AudioCapture_ReadComplete(capture, bufferSize);
            if ( errCode )
            {
                fprintf(stderr, "Error committing capture buffer\n");
                NEXUS_AudioCapture_Stop(capture);
                return;
            }
        }
        else
        {
            break;
        }
    }
}

int main(int argc, char **argv)
{
    FILE *pFile;
    const char *pFileName = "audio_capture.dat";
    NEXUS_PlatformConfiguration config;
    bool wavFile=false;
    int numSeconds=30;
    #if CAPTURE_I2S_INPUT
    NEXUS_Error errCode;
    NEXUS_I2sInputHandle i2sInput;
    NEXUS_AudioOutputSettings outputSettings;
    #else
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_AudioDecoderHandle decoder;
    NEXUS_AudioDecoderStartSettings decoderSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_ParserBandSettings parserBandSettings;
    #endif
    NEXUS_AudioCaptureHandle capture;
    NEXUS_AudioCaptureStartSettings captureStartSettings;
    NEXUS_AudioCaptureOpenSettings captureOpenSettings;
    captureCallbackParameters captureCBParams;

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&config);

    if ( argc > 1 )
    {
        pFileName = argv[1];
        if ( strlen(pFileName) > 4 )
        {
            if ( strstr(pFileName, ".wav") == pFileName + (strlen(pFileName) - 4) )
            {
                /* Filename ends in .wav, write as .wav file */
                wavFile = true;
            }
        }
    }
    if ( argc > 2 )
    {
        numSeconds = atoi(argv[2]);
        if ( numSeconds <= 0 )
        {
            fprintf(stderr, "usage: %s <filename> <duration in seconds>\n", argv[0]);
            return -1;
        }
    }

    pFile = fopen(pFileName, "wb+");
    if ( NULL == pFile )
    {
        fprintf(stderr, "Unable to open file '%s' for writing\n", pFileName);
        return 0;
    }

    #if CAPTURE_I2S_INPUT
    /* Set timebase 0 to I2S input */
    {
        NEXUS_TimebaseSettings timebaseSettings;
        NEXUS_Timebase_GetSettings(NEXUS_Timebase_e0, &timebaseSettings);

        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eI2sIn;
        timebaseSettings.sourceSettings.i2s.index = 0;
        timebaseSettings.sourceSettings.i2s.sampleRate = 48000;
        NEXUS_Timebase_SetSettings(NEXUS_Timebase_e0, &timebaseSettings);
    }

    i2sInput = NEXUS_I2sInput_Open(0, NULL);
    if ( NULL == i2sInput )
    {
        fprintf(stderr, "Unable to open i2s input 0\n");
        return -1;
    }
    #else
    decoder = NEXUS_AudioDecoder_Open(0, NULL);
    if ( NULL == decoder )
    {
        fprintf(stderr, "Unable to open decoder 0\n");
        return 0;
    }
    #endif

    NEXUS_AudioCapture_GetDefaultOpenSettings(&captureOpenSettings);
    capture = NEXUS_AudioCapture_Open(0, &captureOpenSettings);
    if ( NULL == capture )
    {
        fprintf(stderr, "Unable to open capture channel\n");
        return 0;
    }

    #if CAPTURE_I2S_INPUT
    NEXUS_AudioOutput_GetSettings(NEXUS_AudioCapture_GetConnector(capture), &outputSettings);
    outputSettings.timebase = NEXUS_Timebase_e0;
    outputSettings.nco = NEXUS_AudioOutputNco_e0;
    NEXUS_AudioOutput_SetSettings(NEXUS_AudioCapture_GetConnector(capture), &outputSettings);
    #endif

    /* Setup WAV file if desired (*always little-endian) */
    if ( wavFile )
    {
        fwrite("RIFF", 4, 1, pFile);    /* Byte 0..3 RIFF */
        fputc(0, pFile);                /* Byte 4..7 file size - 4*/
        fputc(0, pFile);                /* Byte 5 */
        fputc(0, pFile);                /* Byte 6 */
        fputc(0, pFile);                /* Byte 7 */
        fwrite("WAVE", 4, 1, pFile);    /* Byte 8..11 WAVE */
        fwrite("fmt ", 4, 1, pFile);    /* Byte 12..15 fmt */
        fputc(16, pFile);               /* Byte 16..19 format chunk length (16 bytes) */
        fputc(0, pFile);                /* Byte 17 */
        fputc(0, pFile);                /* Byte 18 */
        fputc(0, pFile);                /* Byte 19 */
        fputc(1, pFile);                /* Byte 20..21 compression code (1=PCM) */
        fputc(0, pFile);                /* Byte 21 */
        fputc(2, pFile);                /* Byte 22..23 Number of channels (2) */
        fputc(0, pFile);                /* Byte 23 */
        fputc(0, pFile);                /* Byte 24..27 Sample Rate (actual value later from decoder) */
        fputc(0, pFile);                /* Byte 25 */
        fputc(0, pFile);                /* Byte 26 */
        fputc(0, pFile);                /* Byte 27 */
        fputc(0, pFile);                /* Byte 28..31 Average Bytes/Second (actual value later from decder) */
        fputc(0, pFile);                /* Byte 29 */
        fputc(0, pFile);                /* Byte 30 */
        fputc(0, pFile);                /* Byte 31 */
        fputc(2, pFile);                /* Byte 32..33 Block Align (4 -- 2 bytes/channel * 2 channels) */
        fputc(0, pFile);
        fputc(16, pFile);               /* Byte 34..35 Bits Per Sample (16) */
        fputc(0, pFile);
        fwrite("data", 4, 1, pFile);    /* Byte 36..39 data */
        fputc(0, pFile);                /* Byte 40..43 data size - 4*/
        fputc(0, pFile);                /* Byte 41 */
        fputc(0, pFile);                /* Byte 42 */
        fputc(0, pFile);                /* Byte 43 */
    }

    #if CAPTURE_I2S_INPUT
    /* Connect outputs to i2s input */
    NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(capture),
                               NEXUS_I2sInput_GetConnector(i2sInput));
    #else
    /* Connect outputs to decoder */
    #if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]),
                               NEXUS_AudioDecoder_GetConnector(decoder, NEXUS_AudioDecoderConnectorType_eStereo));
    #endif
    #if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]),
                               NEXUS_AudioDecoder_GetConnector(decoder, NEXUS_AudioDecoderConnectorType_eStereo));
    #endif
    /* Connect capture to decoder */
    NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(capture),
                               NEXUS_AudioDecoder_GetConnector(decoder, NEXUS_AudioDecoderConnectorType_eStereo));
    #endif

    /* Start the capture -- no data will be received until the decoder starts */
    NEXUS_AudioCapture_GetDefaultStartSettings(&captureStartSettings);
    captureStartSettings.dataCallback.callback = capture_callback;
    captureCBParams.capture = capture;
    captureCBParams.pFile = pFile;
    captureStartSettings.dataCallback.context = &captureCBParams;
    NEXUS_AudioCapture_Start(capture, &captureStartSettings);

    #if CAPTURE_I2S_INPUT
    errCode = NEXUS_I2sInput_Start(i2sInput);
    if ( errCode )
    {
        fprintf(stderr, "Unable to start I2S Capture\n");
        return -1;
    }
    #else
    /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    /* Map a parser band to the streamer input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the StcChannel to do TSM with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = NEXUS_PidChannel_Open(parserBand, 0x11, NULL);
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Start the audio decoder */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&decoderSettings);
    decoderSettings.codec = NEXUS_AudioCodec_eAc3;
    decoderSettings.pidChannel = NEXUS_PidChannel_Open(parserBand, 0x14, NULL);
    decoderSettings.stcChannel = stcChannel;
    NEXUS_AudioDecoder_Start(decoder, &decoderSettings);
    #endif

    fprintf(stderr, "Started capture for %d seconds.\n", numSeconds);
    BKNI_Sleep(numSeconds * 1000);

    fprintf(stderr, "Stopping capture\n");
    NEXUS_StopCallbacks(capture);
    NEXUS_AudioCapture_Stop(capture);

    /* After StopCallbacks, we are guaranteed no more callbacks will arrive.  If we're writing raw data, we're done.
       If we're writing a .wav file, seek back to the beginning and finish up the header */
    if ( wavFile )
    {
        unsigned long fileLength;
        NEXUS_AudioCaptureStatus captureStatus;

        fileLength = ftell(pFile);

        printf("%lu bytes written to file\n", fileLength);
        if ( fileLength == 44 )
        {
            printf("Warning, empty file detected.  Double-check data source\n");
        }

        NEXUS_AudioCapture_GetStatus(capture, &captureStatus);

        fseek(pFile, 4, SEEK_SET);  /* Need to write file size - 4 to this offset */
        fileLength -= 4;
        fputc(fileLength & 0xff, pFile);
        fputc((fileLength >> 8) & 0xff, pFile);
        fputc((fileLength >> 16) & 0xff, pFile);
        fputc((fileLength >> 24) & 0xff, pFile);
        fseek(pFile, 24, SEEK_SET); /* Need to write sample rate here */
        fputc(captureStatus.sampleRate & 0xff, pFile);
        fputc((captureStatus.sampleRate>>8) & 0xff, pFile);
        fputc((captureStatus.sampleRate>>16) & 0xff, pFile);
        fputc((captureStatus.sampleRate>>24) & 0xff, pFile);
        /* Need to write sampleRate * 4 here */
        captureStatus.sampleRate *= 4;
        fputc(captureStatus.sampleRate & 0xff, pFile);
        fputc((captureStatus.sampleRate>>8) & 0xff, pFile);
        fputc((captureStatus.sampleRate>>16) & 0xff, pFile);
        fputc((captureStatus.sampleRate>>24) & 0xff, pFile);
        fseek(pFile, 40, SEEK_SET);  /* Need to write data size (file size - 44) to this offset */
        fileLength -= 40;
        fputc(fileLength & 0xff, pFile);
        fputc((fileLength >> 8) & 0xff, pFile);
        fputc((fileLength >> 16) & 0xff, pFile);
        fputc((fileLength >> 24) & 0xff, pFile);
    }

    printf("Stopping decoder\n");
    #if CAPTURE_I2S_INPUT
    NEXUS_I2sInput_Stop(i2sInput);
    NEXUS_I2sInput_Close(i2sInput);
    #else
    NEXUS_AudioDecoder_Stop(decoder);
    NEXUS_AudioDecoder_Close(decoder);
    #endif
    NEXUS_AudioCapture_Close(capture);

    fflush(pFile);
    fclose(pFile);

    printf("Done\n");
    NEXUS_Platform_Uninit();
    return 0;
}
#else /* NEXUS_HAS_AUDIO */
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
