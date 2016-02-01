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
/* Nexus example app:
Playback and decode of raw LPCM file using playpump and audio dcodecr with a simple algorithm to detect the end of audio decode. */

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_AUDIO

#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

/* 
Summary:
Returns bits 'e'..'b' from word 'w', 

Example:
	B_GET_BITS(0xDE,7,4)==0x0D
	B_GET_BITS(0xDE,3,0)=0x0E 
*/
#define B_GET_BITS(w,e,b)  (((w)>>(b))&(((unsigned)(-1))>>((sizeof(unsigned))*8-(e+1-b))))

int
make_wave_header(uint8_t *buf, size_t buf_size, unsigned bits_per_sample, unsigned sample_rate, unsigned nchannels)
{
    static const uint8_t wav_template[] = {
        'R','I','F','F',
        0xFF,0xFF,0xFF,0xFF,     /* unbounded size */
        'W','A','V','E',
        'f','m','t',' ',
        0x12, 0x00, 0x00, 0x00, /* length of fmt block */
        0x01, 0x00,             /* FormatTag -> LPCM */
        0x00, 0x00,             /* Channels */
        0x00, 0x00, 0x00, 0x00, /* sample rate */
        0x00, 0x00, 0x00, 0x00, /* average byte rate */
        0x00, 0x00,             /* block align */
        0x00, 0x00,             /* bits per sample */
        0x00, 0x00,             /* size of extra block */
        'd','a','t','a',
        0x00,0xFF,0xFF,0xFF     /* size of payload of 0xFFFFFF00 bytes ~ 4GB, after sending that much data, application would need to resend header */
    };
    unsigned block_align = (bits_per_sample/8)*nchannels;
    unsigned byte_rate = block_align*sample_rate;

    if(buf_size<sizeof(wav_template)) {
        /* not enough size for rhe header */
        return -1;
    }
    BKNI_Memcpy(buf, wav_template, sizeof(wav_template));
    /* populate variable fields */
    buf[22] = B_GET_BITS(nchannels, 7, 0);
    buf[23] = B_GET_BITS(nchannels, 15, 8);

    buf[24] = B_GET_BITS(sample_rate, 7, 0);
    buf[25] = B_GET_BITS(sample_rate, 15, 8);
    buf[26] = B_GET_BITS(sample_rate, 23, 16);
    buf[27] = B_GET_BITS(sample_rate, 31, 24);

    buf[28] = B_GET_BITS(byte_rate, 7, 0);
    buf[29] = B_GET_BITS(byte_rate, 15, 8);
    buf[30] = B_GET_BITS(byte_rate, 23, 16);
    buf[31] = B_GET_BITS(byte_rate, 31, 24);

    buf[32] = B_GET_BITS(block_align, 7, 0);
    buf[33] = B_GET_BITS(block_align, 15, 8);

    buf[34] = B_GET_BITS(bits_per_sample, 7, 0);
    buf[35] = B_GET_BITS(bits_per_sample, 15, 8);

    return sizeof(wav_template);
}

int main(int argc, const char *argv[]) {
    FILE *file;
    const char *filename = "videos/B-LPCM-4.pcm";
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump;
    BKNI_EventHandle event;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformSettings platformSettings;
    bool header;
    unsigned last_depth, depth_count;

    if(argc>1) {
        filename = argv[1];
    }
    /* quick and dirty read from file into memory, then do playpump from memory */
    file = fopen(filename, "rb");
    if(!file) {
        goto error;
    }
    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);

    /**
    playpump
    **/
    playpump = NEXUS_Playpump_Open(0, NULL);
    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.transportType = NEXUS_TransportType_eWav; /* we convert raw LPCM to WAV */
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);
    audioPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, 0x1, NULL); 

    /**
    audio decode
    **/
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = NEXUS_AudioCodec_ePcmWav;
    audioProgram.pidChannel = audioPidChannel;

    /* start */
    NEXUS_Playpump_Start(playpump);
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

    for(header=true;;) {
        void *buffer;
        size_t buffer_size;
        NEXUS_Error rc;

        if (NEXUS_Playpump_GetBuffer(playpump, &buffer, &buffer_size)) {
            fprintf(stderr,"playpump error\n");
            break;
        }
        buffer_size -= buffer_size%2; /* make buffer 2 bytes alligned */
        if (buffer_size == 0) {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
        }

        /* The first call to get_buffer will return the entire XPT playback buffer.
        If we use it, we're going to have to wait until the descriptor is complete,
        and then we're going to underflow. So use a max size. */
#define MAX_READ (32*1024)
        if (buffer_size > MAX_READ)
            buffer_size = MAX_READ;

        if(header) { /* to play LPCM data we first need to proovide metadata (sampling rate, number of channels, etc) by constructint the wave header */
            int header_len;

            header_len = make_wave_header(buffer, buffer_size, 16 /* bits per sample */, 48000 /* sampling rate */, 2 /* number of channels, 2 is stereo */);
            if(header_len<0) {
                fprintf(stderr,"not enough space for the header\n");
                goto error;
            }
            rc = NEXUS_Playpump_WriteComplete(playpump, 0, header_len);
            BDBG_ASSERT(!rc);
            header = false;
        } else {
            int read_len;

            read_len=fread(buffer, 1, buffer_size, file);
            if(read_len<0) {
                fprintf(stderr,"read error\n");
                goto error;
            } else if(read_len>0) {
                int i;
                uint8_t *b = buffer;
                for(i=0;i<read_len;i+=2) { /* bytes swap payload to convert from BE to LE data */
                   uint8_t t = b[0];
                   b[0] = b[1];
                   b[1] = t;
                   b+=2;
                }
                rc = NEXUS_Playpump_WriteComplete(playpump, 0, read_len);
                BDBG_ASSERT(!rc);
            } else /* read_lan==0 */ {
                printf("end of file\n");
                break;
            }
        }
    }

    /* wait for the end of decode */

    /* first , wait for the playback fifo to stay static for a period of time. */
    for(last_depth=0,depth_count=0;depth_count<4;) {
        NEXUS_PlaypumpStatus status;
        BKNI_Sleep(200);
        NEXUS_Playpump_GetStatus(playpump, &status);
        printf("playpump fifo %d\n", status.fifoDepth);
        if (status.fifoDepth == last_depth && status.fifoDepth<MAX_READ ) {
            depth_count++;
        } else {
            depth_count = 0;
            last_depth = status.fifoDepth;
        }
    }


    /* then wait for the audio fifo to stay static for a period of time. */
    for(last_depth=0,depth_count=0;depth_count<4;) {
        NEXUS_AudioDecoderStatus status;
        BKNI_Sleep(50);
        NEXUS_AudioDecoder_GetStatus(audioDecoder, &status);
        printf("audio fifo %d\n", status.fifoDepth);
        if(status.fifoDepth == last_depth) {
            depth_count++;
        } else {
            depth_count = 0;
            last_depth = status.fifoDepth;
        }
    }
    printf("audio decode is done\n");
    return 0;
error:
    return 1;
}
#else /* NEXUS_HAS_AUDIO */
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
