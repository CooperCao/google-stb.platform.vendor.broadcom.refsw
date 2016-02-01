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
#include <stdio.h>
#include "nexus_platform.h"
#include "nexus_platform_features.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_audio_playback.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include <string.h>
#include <stdlib.h>

#if NEXUS_NUM_AUDIO_PLAYBACKS
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

/* MIN and MAX macros */
#ifndef MIN
#define MIN( a, b )  ( (a) < (b) ? (a) : (b) )
#endif

#ifndef MAX
#define MAX( a, b )  ( (a) > (b) ? (a) : (b) )
#endif

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

/* HDMI Hotplug Callback */
static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    fprintf(stderr, "HDMI hotplug event: %s\n", status.connected?"connected":"not connected");

    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( status.connected )
    {
        NEXUS_DisplaySettings displaySettings;
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !status.videoFormatSupported[displaySettings.format] )
        {
            fprintf(stderr, "\nCurrent format not supported by attached monitor. Switching to preferred format %d\n", status.preferredVideoFormat);
            displaySettings.format = status.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
}

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
    BERR_Code errCode;
    BKNI_EventHandle event;
    NEXUS_PlatformConfiguration config;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;

    NEXUS_AudioPlaybackHandle playbackHandle[NEXUS_NUM_AUDIO_PLAYBACKS];
    NEXUS_AudioPlaybackStartSettings playbackSettings;

    unsigned numPlaybacksInUse = 0;
    unsigned index, count;
    size_t bytesToPlay = 48000*4*2*60;    /* 48 kHz, 4 bytes/sample, 120 seconds (recommended unittest limit) */
    size_t bytesPlayed=0;
    size_t offset[NEXUS_NUM_AUDIO_PLAYBACKS]={0};
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

    /* Init the Nexus platform */
    {
        NEXUS_PlatformSettings platformSettings;
    
        BKNI_Printf("Initializing Nexus\n" );
        NEXUS_Platform_GetDefaultSettings(&platformSettings);
        platformSettings.openFrontend = false;
        errCode = NEXUS_Platform_Init(&platformSettings);
        if (errCode) return -1;
        NEXUS_Platform_GetConfiguration(&config);
    }
    
    /* Bring up the display (for HDMI) */
    {
        NEXUS_DisplaySettings displaySettings;

        BKNI_Printf("Opening Display 0\n" );
        NEXUS_Display_GetDefaultSettings(&displaySettings);
        display = NEXUS_Display_Open(0, &displaySettings);
        window = NEXUS_VideoWindow_Open(display, 0);
    }

    /* Add the HDMI output to the display */
    BKNI_Printf("Adding HDMI Output 0 to Display 0\n" );
    errCode = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(config.outputs.hdmi[0]));
    BDBG_ASSERT(!errCode);

    /* Install hotplug callback -- video only for now */
    {
        NEXUS_HdmiOutputSettings hdmiSettings;

        NEXUS_HdmiOutput_GetSettings(config.outputs.hdmi[0], &hdmiSettings);
        hdmiSettings.hotplugCallback.callback = hotplug_callback;
        hdmiSettings.hotplugCallback.context = config.outputs.hdmi[0];
        hdmiSettings.hotplugCallback.param = (int)display;
        errCode = NEXUS_HdmiOutput_SetSettings(config.outputs.hdmi[0], &hdmiSettings);
        BDBG_ASSERT(!errCode);
    }

    /* Force a hotplug to switch to preferred format */
    hotplug_callback(config.outputs.hdmi[0], (int)display);

    BKNI_CreateEvent(&event);

    for (index=0 ; index<NEXUS_NUM_AUDIO_PLAYBACKS ; index++)
    {
        BKNI_Printf("Opening Playback %u\n", index );
        BDBG_ASSERT(index < NEXUS_NUM_AUDIO_PLAYBACKS);
        playbackHandle[index] = NEXUS_AudioPlayback_Open(index, NULL);
        BDBG_ASSERT(playbackHandle[index]);
    }

    #if NEXUS_NUM_AUDIO_DACS
    /* Connect DACs to plabacks */
    count = MIN( NEXUS_NUM_AUDIO_PLAYBACKS, NEXUS_NUM_AUDIO_DACS );
    numPlaybacksInUse = MAX( numPlaybacksInUse, count );
    for (index=0 ; index<count ; index++)
    {
        NEXUS_AudioOutputSettings outputSettings;

        BKNI_Printf("Adding Playback %u to DAC %u\n", index, index );
        BDBG_ASSERT(index < NEXUS_NUM_AUDIO_PLAYBACKS);
        BDBG_ASSERT(index < NEXUS_NUM_AUDIO_DACS);

        /* Change DAC settings here (if desired) */
        NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[index]), &outputSettings);
        outputSettings.pll = 0;
        NEXUS_AudioOutput_SetSettings(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[index]), &outputSettings);

        /* Connect the Playback to the input of the DAC output */
        errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[index]),
                                             NEXUS_AudioPlayback_GetConnector(playbackHandle[index]));
        BDBG_ASSERT(!errCode);
    }
    #endif

    #if NEXUS_NUM_SPDIF_OUTPUTS
        count = MIN( NEXUS_NUM_AUDIO_PLAYBACKS, NEXUS_NUM_SPDIF_OUTPUTS );
        numPlaybacksInUse = MAX( numPlaybacksInUse, count );
        for (index=0 ; index<count ; index++)
        {
            NEXUS_AudioOutputSettings outputSettings;

            BKNI_Printf("Adding Playback %u to SPDIF %u \n", index, index );
            BDBG_ASSERT(index < NEXUS_NUM_AUDIO_PLAYBACKS);
            BDBG_ASSERT(index < NEXUS_NUM_SPDIF_OUTPUTS);

            /* Change Spdif output settings here (if desired) */
            NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[index]), &outputSettings);
            outputSettings.defaultSampleRate = 48000;
            outputSettings.nco = NEXUS_AudioOutputNco_eMax;
            errCode = NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[index]), &outputSettings);
            BDBG_ASSERT(!errCode);

            /* Connect the Playback to the input of the Spdif output */
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[index]),
                                       NEXUS_AudioPlayback_GetConnector(playbackHandle[index]));
            BDBG_ASSERT(!errCode);
        }
    #endif

    /* Connect I2S outputs to playbacks */
    #if NEXUS_NUM_I2S_OUTPUTS
        count = MIN( NEXUS_NUM_AUDIO_PLAYBACKS, NEXUS_NUM_I2S_OUTPUTS );
        numPlaybacksInUse = MAX( numPlaybacksInUse, count );
        for (index=0 ; index<count ; index++)
        {
            NEXUS_AudioOutputSettings outputSettings;

            BKNI_Printf("Adding Playback %u to I2S %u\n", index, index );
            BDBG_ASSERT(index < NEXUS_NUM_AUDIO_PLAYBACKS);
            BDBG_ASSERT(index < NEXUS_NUM_I2S_OUTPUTS);

            /* Change I2S output settings here (if desired) */
            NEXUS_AudioOutput_GetSettings(NEXUS_I2sOutput_GetConnector(config.outputs.i2s[index]), &outputSettings);
            outputSettings.defaultSampleRate = 48000;
            outputSettings.nco = NEXUS_AudioOutputNco_eMax;
            errCode = NEXUS_AudioOutput_SetSettings(NEXUS_I2sOutput_GetConnector(config.outputs.i2s[index]), &outputSettings);
            BDBG_ASSERT(!errCode);

            /* Connect the Playback to the input of the I2S output */
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_I2sOutput_GetConnector(config.outputs.i2s[index]),
                                       NEXUS_AudioPlayback_GetConnector(playbackHandle[index]));
            BDBG_ASSERT(!errCode);
        }
    #endif

    /* Connect HDMI outputs to playbacks */
    #if NEXUS_NUM_HDMI_OUTPUTS
        count = MIN( NEXUS_NUM_AUDIO_PLAYBACKS, NEXUS_NUM_HDMI_OUTPUTS );
        numPlaybacksInUse = MAX( numPlaybacksInUse, count );
        for (index=0 ; index<count ; index++)
        {
            NEXUS_AudioOutputSettings outputSettings;

            BKNI_Printf("Adding Playback %u to HDMI %u\n", index, index );
            BDBG_ASSERT(index < NEXUS_NUM_AUDIO_PLAYBACKS);
            BDBG_ASSERT(index < NEXUS_NUM_HDMI_OUTPUTS);

            /* Change HDMI output settings here (if desired) */
            NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(config.outputs.hdmi[index]), &outputSettings);
            outputSettings.defaultSampleRate = 48000;
            outputSettings.nco = NEXUS_AudioOutputNco_e0;
            errCode = NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(config.outputs.hdmi[index]), &outputSettings);
            BDBG_ASSERT(!errCode);

            /* Connect the Playback to the input of the HDMI audio output */
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(config.outputs.hdmi[index]),
                                       NEXUS_AudioPlayback_GetConnector(playbackHandle[index]));
            BDBG_ASSERT(!errCode);
        }
    #endif

    NEXUS_AudioPlayback_GetDefaultStartSettings(&playbackSettings);
    if ( argc > 2 )
    {
        playbackSettings.sampleRate = atoi(argv[2]);
    }
    else
    {
        playbackSettings.sampleRate = 48000;
    }

    if ( argc > 3 )
    {
        playbackSettings.bitsPerSample = atoi(argv[3]);
    }
    else
    {
        playbackSettings.bitsPerSample = 16;
    }
    if ( argc > 4 )
    {
        playbackSettings.stereo = atoi(argv[4]);
    }
    else
    {
        playbackSettings.stereo = true;
    }
    playbackSettings.signedData = true;
    playbackSettings.dataCallback.callback = data_callback;
    playbackSettings.dataCallback.context = playbackHandle;
    playbackSettings.dataCallback.param = (int)event;

    /* If we have a wav file, get the sample rate from it */
    if ( pFile )
    {
        wave_header wh;

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
                return 1;
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
                return 1;
            }
            if (wh.channels > 2) {      
                fprintf(stderr, "Invalid number of channels (%u) specified.\n",wh.channels);
                return 1;
            }
            playbackSettings.sampleRate = wh.samplesSec;
            playbackSettings.bitsPerSample = wh.bps;
            playbackSettings.stereo = (wh.channels>1);
            playbackSettings.signedData = (wh.bps != 8);    /* 8-bit .wav files are unsigned */
        }
    }
    printf("Stream sample rate %d, %d bits per sample, %s\n", playbackSettings.sampleRate, playbackSettings.bitsPerSample, playbackSettings.stereo?"stereo":"mono");

    switch ( playbackSettings.sampleRate )
    {
    case 32000:
    case 44100:
        {
            NEXUS_AudioOutputSettings outputSettings;
    
            #if NEXUS_NUM_AUDIO_DACS
            NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]), &outputSettings);
            outputSettings.defaultSampleRate = playbackSettings.sampleRate;
            errCode = NEXUS_AudioOutput_SetSettings(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]), &outputSettings);
            BDBG_ASSERT(!errCode);
            #else
            BSTD_UNUSED(outputSettings);
            #endif

            #if NEXUS_NUM_SPDIF_OUTPUTS
                NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]), &outputSettings);
                outputSettings.defaultSampleRate = playbackSettings.sampleRate;
                errCode = NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]), &outputSettings);
                BDBG_ASSERT(!errCode);
            #endif
            break;
        }
    default:
        break;
    }

    #if NEXUS_NUM_HDMI_OUTPUTS
    {
        NEXUS_AudioOutputSettings outputSettings;
        NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(config.outputs.hdmi[0]), &outputSettings);
    /*  outputSettings.defaultSampleRate = 32000;
        outputSettings.nco = 1;
    */
        errCode = NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(config.outputs.hdmi[0]), &outputSettings);
        BDBG_ASSERT(!errCode);
    }
    #endif

    count = numPlaybacksInUse;
    for (index=0 ; index<count ; index++)
    {
        BKNI_Printf("Starting Playback %u\n", index );
        BDBG_ASSERT(index < NEXUS_NUM_AUDIO_PLAYBACKS);

        playbackSettings.sampleRate = 48000;

        errCode = NEXUS_AudioPlayback_Start(playbackHandle[index], &playbackSettings);
        BDBG_ASSERT(!errCode);
    }

    do
    {
        unsigned i;

        /* Check available buffer space */
        count = numPlaybacksInUse;
        for (index=0 ; index<count ; index++)
        {
            errCode = NEXUS_AudioPlayback_GetBuffer(playbackHandle[index], (void **)&pBuffer, &bufferSize);
            BDBG_ASSERT(!errCode);
            
            if (bufferSize) break;
        }
        
        if (index < count && bufferSize) 
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
                    const int  step = 1;        /* use 1 for 1 KHz, 2 for 2 KHz, 4 for for 4 KHz, etc. */
                    pBuffer[2*i] = pBuffer[(2*i)+1] = samples[offset[index]];
                    offset[index] += step;
                    if ( offset[index] >= 48 )
                    {
                        offset[index] -= 48;
                    }
                }
                bufferSize *= 4;
            }

            errCode = NEXUS_AudioPlayback_WriteComplete(playbackHandle[index], bufferSize);
            BDBG_ASSERT(!errCode);
            
            if ((bytesPlayed/1024) % 100 == 0) {
                BDBG_WRN(("%d of %d KB played", bytesPlayed/1024, bytesToPlay/1024));
            }
        }
        else 
        {
            errCode = BKNI_WaitForEvent(event, 5000);
            BDBG_ASSERT(!errCode);
        }
    } while (bytesPlayed < bytesToPlay);

    count = numPlaybacksInUse;
    for (index=0 ; index<count ; index++)
    {
        printf("Waiting for Playback %u buffer to empty\n", index);

        for (;;)
        {
            NEXUS_AudioPlaybackStatus status;
            NEXUS_AudioPlayback_GetStatus(playbackHandle[index], &status);
            if ( status.queuedBytes > 0 )
            {
                BKNI_Sleep(100);
            }
            else break;
        }
    }
    
    printf("Stopping playback\n");
    count = numPlaybacksInUse;
    for (index=0 ; index<count ; index++)
    {
        BKNI_Printf("Stopping Playback %u\n", index );
        BDBG_ASSERT(index < numPlaybacksInUse);

        NEXUS_AudioPlayback_Stop(playbackHandle[index]);
    }

    #if NEXUS_NUM_AUDIO_DACS
    count = MIN( NEXUS_NUM_AUDIO_PLAYBACKS, NEXUS_NUM_AUDIO_DACS );
    for (index=0 ; index<count ; index++)
    {
        BKNI_Printf("Removing Playback %u from DAC %u\n", index, index );

        BDBG_ASSERT(index < NEXUS_NUM_AUDIO_PLAYBACKS);
        BDBG_ASSERT(index < NEXUS_NUM_AUDIO_DACS);
        NEXUS_AudioOutput_RemoveInput(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[index]),
                                      NEXUS_AudioPlayback_GetConnector(playbackHandle[index]));
    }
    #endif

    #if NEXUS_NUM_SPDIF_OUTPUTS
    count = MIN( NEXUS_NUM_AUDIO_PLAYBACKS, NEXUS_NUM_SPDIF_OUTPUTS );
    for (index=0 ; index<count ; index++)
    {
        BKNI_Printf("Removing Playback %u from SPDIF %u\n", index,  index );
        NEXUS_AudioOutput_RemoveInput(NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[index]),
                                   NEXUS_AudioPlayback_GetConnector(playbackHandle[index]));
    }
    #endif


    #if NEXUS_NUM_I2S_OUTPUTS
        count = MIN( NEXUS_NUM_AUDIO_PLAYBACKS, NEXUS_NUM_I2S_OUTPUTS );
        for (index=0 ; index<count ; index++)
        {
            BKNI_Printf("Removing Playback %u from I2S output %u \n", index, index );
            BDBG_ASSERT(index < NEXUS_NUM_AUDIO_PLAYBACKS);
            BDBG_ASSERT(index < NEXUS_NUM_I2S_OUTPUTS);
            NEXUS_AudioOutput_RemoveInput(NEXUS_I2sOutput_GetConnector(config.outputs.i2s[index]),
                                       NEXUS_AudioPlayback_GetConnector(playbackHandle[index]));
        }
    #endif

    BKNI_Printf("Removing Playback %u from I2S output %u \n", index, index );
    for (index=0 ; index<2 ; index++)
    {
        BKNI_Printf("Closing Playback %u\n", index );
        BDBG_ASSERT(index < NEXUS_NUM_AUDIO_PLAYBACKS);
        NEXUS_AudioPlayback_Close(playbackHandle[index]);
    }

    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_Platform_Uninit();

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

