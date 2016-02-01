/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
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
#include <string.h>
#include <stdlib.h>
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#include "nexus_display.h"
#endif

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

#define SWAP32( a )  do{a=((a&0xFF)<<24|(a&0xFF00)<<8|(a&0xFF0000)>>8|(a&0xFF000000)>>24);}while(0)
#define SWAP16( a )  do{a=((a&0xFF)<<8|(a&0xFF00)>>8);}while(0)

typedef struct wave_header
{
    unsigned long riff;         /* 'RIFF' */
    unsigned long riffCSize;    /* size in bytes of file - 8 */
    unsigned long wave;         /* 'WAVE' */
    unsigned long fmt;          /* 'fmt ' */
    unsigned long headerLen;    /* header length (should be 16 for PCM) */
    unsigned short format;      /* 1 - pcm */
    unsigned short channels;    /* 1 - mono, 2 - sterio */
    unsigned long samplesSec;   /* samples / second */
    unsigned long bytesSec;     /* bytes / second */
    unsigned short chbits;      /* channels * bits/sample /8 */
    unsigned short bps;         /* bits per sample (8 or 16) */
                                /* Extensible format */
    unsigned short cbSize;      /* 2 Size of the extension (0 or 22)  */
    unsigned short wValidBitsPerSample; /* 2 Number of valid bits  */
    unsigned short dwChannelMask; /* 4 Speaker position mask  */
    unsigned char SubFormat[16];  /* SubFormat */

    unsigned long dataSig;      /* 'data' */
    unsigned long dataLen;      /* length of data */
}wave_header;

BDBG_MODULE(audio_playback);

static void data_callback(void *pParam1, int param2)
{
    /*
    printf("Data callback - channel 0x%08x\n", (unsigned)pParam1);
    */
    pParam1=pParam1;    /*unused*/
    BKNI_SetEvent((BKNI_EventHandle)param2);
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    BERR_Code errCode;
    BKNI_EventHandle event;
    NEXUS_PlatformConfiguration config;
    NEXUS_AudioOutputSettings outputSettings;
    NEXUS_AudioPlaybackHandle handle;
    NEXUS_AudioPlaybackStartSettings settings;
#if NEXUS_HAS_HDMI_OUTPUT
    NEXUS_DisplayHandle display=NULL;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_HdmiOutputStatus hdmiStatus;
    unsigned timeout=100;
#endif
    size_t bytesToPlay = 48000*4*20;    /* 48 kHz, 4 bytes/sample, 20 seconds */
    size_t bytesPlayed=0;
    size_t offset=0;
    int16_t *pBuffer;
    uint8_t *pCopyBuffer;
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

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&config);

    BKNI_CreateEvent(&event);

    handle = NEXUS_AudioPlayback_Open(0, NULL);
    if ( NULL == handle )
    {
        fprintf(stderr, "Unable to open playback channel\n");
        errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto done;
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
#endif

    /* Connect DAC to playback */
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]),
                               NEXUS_AudioPlayback_GetConnector(handle));
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]),
                               NEXUS_AudioPlayback_GetConnector(handle));
#endif
#if NEXUS_HAS_HDMI_OUTPUT
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(config.outputs.hdmi[0]),
        NEXUS_AudioPlayback_GetConnector(handle));
#endif

    NEXUS_AudioPlayback_GetDefaultStartSettings(&settings);
    if ( argc > 2 )
    {
        settings.sampleRate = atoi(argv[2]);
    }
    else
    {
        settings.sampleRate = 48000;
    }

    if ( argc > 3 )
    {
        settings.bitsPerSample = atoi(argv[3]);
    }
    else
    {
        settings.bitsPerSample = 16;
    }
    if ( argc > 4 )
    {
        settings.stereo = atoi(argv[4]);
    }
    else
    {
        settings.stereo = true;
    }
    if ( argc > 5 )
    {
        if (!strcmp(argv[5], "BE") || !strcmp(argv[5], "be"))
        {
            settings.endian = NEXUS_EndianMode_eBig;
        }
        else if (!strcmp(argv[5], "LE") || !strcmp(argv[5], "le"))
        {
            settings.endian = NEXUS_EndianMode_eLittle;
        }
        else
        {
            fprintf(stderr, "Invalid parameter set for endian.\n");
        }
    }

    settings.signedData = true;
    settings.dataCallback.callback = data_callback;
    settings.dataCallback.context = handle;
    settings.dataCallback.param = (int)event;

    /* If we have a wav file, get the sample rate from it */
    if ( pFile )
    {
        unsigned long dword;
        fseek(pFile,0,SEEK_END);
        bytesToPlay = ftell(pFile);
        fseek(pFile,0,SEEK_SET);
        fread(&dword,4,1,pFile);
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
        SWAP32(dword);
#endif
        if (dword == 0x46464952) /* RIFF */
        { /* WAV */
            wave_header wh;

            fseek(pFile,0,SEEK_SET);
            /* read in the wave file header */
            fread(&wh.riff,4,1,pFile);
            fread(&wh.riffCSize,4,1,pFile);
            fread(&wh.wave,4,1,pFile);
            fread(&wh.fmt,4,1,pFile);
            fread(&wh.headerLen,4,1,pFile);
            fread(&wh.format,2,1,pFile);
            fread(&wh.channels,2,1,pFile);
            fread(&wh.samplesSec,4,1,pFile);
            fread(&wh.bytesSec,4,1,pFile);
            fread(&wh.chbits,2,1,pFile);
            fread(&wh.bps,2,1,pFile);

        #if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
            SWAP32(wh.riff);
            SWAP32(wh.riffCSize);
            SWAP32(wh.wave);
            SWAP32(wh.fmt);
            SWAP32(wh.headerLen);
            SWAP16(wh.format);
            SWAP16(wh.channels);
            SWAP32(wh.samplesSec);
            SWAP32(wh.bytesSec);
            SWAP16(wh.chbits);
            SWAP16(wh.bps);
        #endif

            /* Verify that this is a PCM WAV file. */
            if (wh.riff != 0x46464952) {
                fprintf(stderr, "RAW data file detected.\n");
                fseek(pFile, 0, SEEK_SET);
            }
            else
            {
                if (wh.wave != 0x45564157) {
                    fprintf(stderr, "Not a WAV file.");
                    errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                    goto done;
                }

                if (wh.headerLen == 40 && wh.format == 0xfffe) { /* WAVE_FORMAT_EXTENSIBLE */
                    fread(&wh.cbSize,2,1,pFile);                /* 2 Size of the extension (0 or 22)  */
                    fread(&wh.wValidBitsPerSample,2,1,pFile);   /* 2 Number of valid bits  */
                    fread(&wh.dwChannelMask,4,1,pFile);         /* 4 Speaker position mask  */
                    fread(&wh.SubFormat,16,1,pFile);            /* SubFormat GUID */
                    #if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
                    SWAP16(wh.cbSize);
                    SWAP16(wh.wValidBitsPerSample);
                    SWAP32(wh.dwChannelMask);
                    #endif
                }
                else if (wh.headerLen == 18 && wh.format == 1) { /* oddball WAVE format */
                    fread(&wh.cbSize,2,1,pFile);                /* 2 Size of the extension (0 or 22) ?*/
                }
                else if (wh.headerLen != 16 && wh.format != 1) {
                    fprintf(stderr, "Not PCM data in WAV file. headerLen = %lu, Format 0x%x\n",wh.headerLen,wh.format);
                }

                for (;;) {
                    if (fread(&wh.dataSig,4,1,pFile) != 1)
                        break;
                    if (fread(&wh.dataLen,4,1,pFile) != 1)
                        break;
                #if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
                    SWAP32(wh.dataSig);
                    SWAP32(wh.dataLen);
                #endif
                    /* looking for 'data' chunk */
                    if (wh.dataSig == 0x61746164) {
                        break;
                    }
                    else if (fseek(pFile, wh.dataLen, SEEK_CUR))
                    {
                        break;
                    }
                }
                if (wh.dataSig != 0x61746164) {
                    fprintf(stderr, "No 'data' chunk found in WAV file.\n");
                    errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                    goto done;
                }
                if (wh.channels > 2) {
                    fprintf(stderr, "Invalid number of channels (%u) specified.\n",wh.channels);
                    errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                    goto done;
                }
                settings.sampleRate = wh.samplesSec;
                settings.bitsPerSample = wh.bps;
                settings.stereo = (wh.channels>1);
                settings.signedData = (wh.bps != 8);    /* 8-bit .wav files are unsigned */
                settings.endian = NEXUS_EndianMode_eLittle;
                bytesToPlay = wh.dataLen;
            }
        }
        else if (dword == 0x4D524F46) /* FORM */
        { /* AIFF */
            unsigned long format;
            /* We need to determine if this is an AIFF or an AIFF-C */
            fseek(pFile,4,SEEK_CUR); /* ckDataSize */
            fread(&format,4,1,pFile); /* formType */
        #if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
            SWAP32(format);
        #endif
            if (format == 0x41494643 || format == 0x41494646) /* AIFC or AIFF*/
            {
                unsigned dataStart = 0;
                bool soundDataFound = false, commonChunkFound = false, eos = false;
                /* There is no particular order that AIFF Chunks can occur so we need to find the COMM  and SSND CHUNKS */
                while ((!soundDataFound || !commonChunkFound) && !eos)
                {
                    size_t bytesRead = 0;
                    bytesRead = fread(&dword,4,1,pFile);
                    if (bytesRead != 1)
                    {
                        eos = true;
                        continue;
                    }
                #if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
                    SWAP32(dword);
                #endif
                    if (dword == 0x434F4D4D) /* COMM */
                    {
                        unsigned long skip, skipLocation;
                        uint8_t exp[2];
                        uint8_t significand[8];
                        short channels, bitrate;
                        int i,e;
                        uint64_t s;

                        fread(&skip,4,1,pFile);
                        skipLocation = ftell(pFile);
                        fread(&channels,2,1,pFile); /* numChannels */
                        fseek(pFile,4,SEEK_CUR); /* numSampleFrames */
                        fread(&bitrate,2,1,pFile); /* sampleSize; */
                        fread(&exp,2,1,pFile); /* sampleRate */
                        fread(&significand,8,1,pFile); /* sampleRate */
                        for (i = 0; i < 8; i++)
                        {
                            s = (s<<8) + significand[i];
                        }
                        e = (((int)exp[0]&0x7f)<<8) | exp[1];
                        e -= 16383 + 63;
                        if (exp[0]&0x80)
                        {
                            s= -s;
                        }
                        if(e<0) {
                            settings.sampleRate = s>>(-e);
                        } else {
                            settings.sampleRate = s<<e;
                        }

                     #if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
                        SWAP32(skip);
                        SWAP16(channels);
                        SWAP16(bitrate);
                    #endif
                        if (channels > 2)
                        {
                            goto done;
                        }
                        settings.stereo = channels>1?true:false;
                        settings.bitsPerSample = bitrate;

                        if (format == 0x41494643) /* AIFC */
                        {
                            fread(&dword,4,1,pFile); /* formType */
                        #if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
                            SWAP32(dword);
                        #endif
                            if (dword == 0x736F7774) /* sowt */
                            {
                                settings.endian = NEXUS_EndianMode_eLittle;
                            }
                            else if (dword == 0x4E4F4E45 ) /* NONE */
                            {
                                settings.endian = NEXUS_EndianMode_eBig;
                            }
                            else
                            {
                                goto done;
                            }
                        }
                        else
                        {
                            settings.endian = NEXUS_EndianMode_eBig;
                        }
                        fseek(pFile,skipLocation,SEEK_SET);
                        fseek(pFile,skip,SEEK_CUR);
                        commonChunkFound = true;
                    }
                    else if (dword == 0x53534E44) /* SSND */
                    {
                        fread(&dword,4,1,pFile);
                    #if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
                        SWAP32(dword);
                    #endif
                        dataStart = ftell(pFile) + 8; /* Need to skip 8 bytes to get to start of soundData */
                        bytesToPlay = dword-8; /*size of SSND - 8 bytes */
                        fseek(pFile,dword,SEEK_CUR);
                        soundDataFound = true;
                    }
                    else
                    {
                        fread(&dword,4,1,pFile);
                    #if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
                        SWAP32(dword);
                    #endif
                        fseek(pFile,dword,SEEK_CUR);
                    }
                }
                fseek(pFile,dataStart,SEEK_SET); /* Seek to the start of the audio data */
            }
        } /* AIFF */
        else { /* RAW PCM */
            fprintf(stderr, "RAW data file detected.\n");
            fseek(pFile, 0, SEEK_SET);
        }
    }
    printf("Stream sample rate %d, %d bits per sample, %s, %s\n", settings.sampleRate,
        settings.bitsPerSample, settings.stereo?"stereo":"mono",settings.endian==NEXUS_EndianMode_eLittle?"LE":"BE");

    switch ( settings.sampleRate )
    {
    case 32000:
    case 44100:
#if NEXUS_NUM_AUDIO_DACS
        NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]), &outputSettings);
        outputSettings.defaultSampleRate = settings.sampleRate;
        NEXUS_AudioOutput_SetSettings(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]), &outputSettings);
#endif
#if NEXUS_NUM_SPDIF_OUTPUTS
        NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]), &outputSettings);
        outputSettings.defaultSampleRate = settings.sampleRate;
        NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]), &outputSettings);
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(config.outputs.hdmi[0]), &outputSettings);
        outputSettings.defaultSampleRate = settings.sampleRate;
        NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(config.outputs.hdmi[0]), &outputSettings);
#endif
        break;
    default:
        break;
    }

    errCode = NEXUS_AudioPlayback_Start(handle, &settings);
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
        errCode = NEXUS_AudioPlayback_GetBuffer(handle, (void **)&pBuffer, &bufferSize);
        if ( errCode )
        {
            printf("Error getting playback buffer\n");
            break;
        }

        if (bufferSize)
        {
            if (bytesToPlay < bytesPlayed + bufferSize)
            {
                bufferSize = bytesToPlay - bytesPlayed;
            }

            if ( pFile )
            {
                if (settings.bitsPerSample != 24)
                {
                    bufferSize = fread(pBuffer, 1, bufferSize, pFile);
                    if ( 0 == bufferSize )
                    {
                        break;
                    }
                }
                else
                {
                    size_t bytesCopied = 0;
                    size_t calculatedBufferSize = (bufferSize/4)*3;

                    pCopyBuffer=BKNI_Malloc(calculatedBufferSize*sizeof(uint8_t));
                    if (pCopyBuffer == NULL)
                    {
                        bufferSize = 0;
                        break;;
                    }

                    bufferSize = fread(pCopyBuffer, 1, calculatedBufferSize, pFile);
                    if ( 0 == bufferSize )
                    {
                        BKNI_Free(pCopyBuffer);
                        break;
                    }
                    else
                    {
                        for (i = 0; i < bufferSize/3; i++)
                        {
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
                            pBuffer[(i*2)] = pCopyBuffer[(i*3)] << 8 | pCopyBuffer[(i*3)+1];
                            pBuffer[(i*2)+1] = pCopyBuffer[(i*3)+2] << 8 | 0x00;
#else
                            pBuffer[(i*2)] = pCopyBuffer[(i*3)] << 8 | 0x0;
                            pBuffer[(i*2)+1] = pCopyBuffer[(i*3)+2] << 8 | pCopyBuffer[(i*3)+1];
#endif
                            bytesCopied +=4;
                        }
                        bufferSize = bytesCopied;
                        BKNI_Free(pCopyBuffer);
                    }
                }
                bytesPlayed += bufferSize;
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

            errCode = NEXUS_AudioPlayback_WriteComplete(handle, bufferSize);
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
        }
    } while ( BERR_SUCCESS == errCode && bytesPlayed < bytesToPlay );

    printf("Waiting for buffer to empty\n");
    for ( ;; )
    {
        NEXUS_AudioPlaybackStatus status;
        NEXUS_AudioPlayback_GetStatus(handle, &status);
        if ( status.queuedBytes > 0 )
        {
            BKNI_Sleep(100);
        }
        else
        {
            break;
        }
    }

done:
    if (handle) {
        printf("Stopping playback\n");
        NEXUS_AudioPlayback_Stop(handle);
        BKNI_DestroyEvent(event);
        NEXUS_AudioPlayback_Close(handle);
    }
#if NEXUS_HAS_HDMI_OUTPUT
    if ( display )
    {
        NEXUS_Display_Close(display);
    }
#endif
    NEXUS_Platform_Uninit();
    printf("Done\n");

    return errCode;
}
#else
int main(void)
{
    fprintf(stderr, "No audio playback support\n");
    return 0;
}
#endif

