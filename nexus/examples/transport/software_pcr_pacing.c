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
 *
 * Module Description:
 *  Example code on how to pace MPEG-2 TS stream based on PCR's
 *
 *******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "videos/cnnticker.mpg"
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define PCR_PID 0x21

#define B_TS_PKT_LEN 188
#define B_TS_SYNC   0x47

/*
Summary:
Returns bits 'e'..'b' from word 'w',

Example:
	B_GET_BITS(0xDE,7,4)==0x0D
	B_GET_BITS(0xDE,3,0)=0x0E
*/
#define B_GET_BITS(w,e,b)  (((w)>>(b))&(((unsigned)(-1))>>((sizeof(unsigned))*8-(e+1-b))))

/*
Summary:
Returns bit 'b' from word 'w',

Example:
	B_GET_BIT(0xDE,1)==0
	B_GET_BIT(0xDE,7)!=0
*/
#define B_GET_BIT(w,b)  ((w)&(1<<(b)))

#define B_TS_LOAD16(ptr) ((((uint16_t)((ptr)[0]))<<8) | (ptr)[1])
#define B_TS_LOAD32(ptr) ((((uint32_t)((ptr)[0]))<<24) | (((uint32_t)((ptr)[1]))<<16) | (((uint32_t)((ptr)[2]))<<8) | (ptr)[3])

#define BMPEG2TS_PCR_FLAG	(1<<4)

static uint32_t
b_demux_ts_feed_getpcr(const uint8_t *data, uint16_t pcr_pid)
{
    uint32_t word;
    unsigned pid;
    unsigned adaptation_field_control;

	/* ISO/IEC 13818-1 */
    if (data[0]!=B_TS_SYNC) {
        goto err_out_of_sync;
    }
    word = B_TS_LOAD16(data+1);
    if (B_GET_BIT(word, 15) /* transport_error_indicator */) {
        goto ts_error;
    }
    pid = B_GET_BITS(word, 12, 0);
    if(pid!=pcr_pid) {
        goto wrong_pid;
    }
    word = data[3];
    adaptation_field_control = B_GET_BITS(word, 5, 4);
    if(adaptation_field_control==0x02 || adaptation_field_control==0x03) {
        unsigned adaptation_field_length = data[4];
        word = data[5];
        /* fprintf(stderr, "--%u:%u:%u:%u:%u\n", adaptation_field_length, data[5], data[6], data[7],data[8]); */
        if (adaptation_field_length>6 && B_GET_BIT(word, 4)) {
            uint64_t pcr = B_TS_LOAD32(data+6);
            unsigned pcr_ext = B_TS_LOAD16(data+10);
            /* fprintf(stderr, "base:%u ext:%u\n",  (unsigned)pcr, (unsigned)pcr_ext); */
            pcr = (pcr<<1) | B_GET_BITS(pcr_ext,15, 15);
            /* fprintf(stderr, "+base:%u ext:%u\n",  (unsigned)pcr, (unsigned)B_GET_BITS(pcr_ext, 8, 0));  */
            pcr = (pcr * 300) + B_GET_BITS(pcr_ext, 8, 0);
            return (uint32_t)pcr; /* return PCR in units of 27MHz clock - LSB 32 bits only */
        }
    }
wrong_pid:
ts_error:
err_out_of_sync:
    return 0;
}

typedef struct b_tts_converter {
    unsigned npackets;
    uint32_t prev_timestamp;
    unsigned packet_step;
    uint32_t pcr_timestamp;
    uint32_t last_pcr;
    uint16_t pcr_pid;
} b_tts_converter ;

/* assume max 27MBps stream */
#define DEFAULT_STEP    (188*8*1)

/* discontinuity threshold is  0.5 sec */
#define TTS_THRESHOLD   ((27*1000*1000)/2)

void
b_tts_converter_init(b_tts_converter *tts, uint16_t pcr_pid)
{
    tts->npackets = 0;
    tts->packet_step = 0; /* don't increment ts until have found first PCR */
    tts->packet_step = DEFAULT_STEP;
    tts->pcr_timestamp = 0;
    tts->prev_timestamp = tts->pcr_timestamp;
    tts->last_pcr = 0;
    tts->pcr_pid = pcr_pid;
    return;
}

void
b_tts_converter_process(b_tts_converter *tts, const uint8_t *src, uint8_t *dst)
{
    uint32_t pcr;
    uint32_t timestamp;
    pcr = b_demux_ts_feed_getpcr(src, tts->pcr_pid);
    if(pcr==0) {
        timestamp = tts->prev_timestamp + tts->packet_step ;
    } else {
        /* fprintf(stderr, "packet:%u PCR:%u(%u)\n", (unsigned)tts->npackets, (unsigned)pcr, (unsigned)tts->last_pcr); */
        if(tts->last_pcr != 0) {
            uint32_t pcr_diff = pcr - tts->last_pcr;
            if(pcr_diff <= TTS_THRESHOLD) { /* if distance between PCR's less then 1 sec */
                unsigned packet_step = pcr_diff / tts->npackets;
                /* fprintf(stderr, "packet:%u diff:%u npackets:%u step:%u\n", (unsigned)tts->npackets, (unsigned)pcr_diff, (unsigned)tts->npackets, (unsigned)packet_step); */
                if(packet_step < tts->packet_step) {
                    tts->packet_step = packet_step;
                }
            } else {
                fprintf(stderr,"pcr discontinuity\n");
                pcr_diff = TTS_THRESHOLD;
            }
            timestamp = tts->pcr_timestamp + pcr_diff;
            if(timestamp - tts->prev_timestamp >= TTS_THRESHOLD) {
                fprintf(stderr,"timestamp out of order %u:%u\n", timestamp, tts->prev_timestamp);
            }
        }  else {
            timestamp = tts->prev_timestamp + tts->packet_step;
        }
        /* fprintf(stderr, "ts %u:%u:%u\n", tts->pcr_timestamp, tts->prev_timestamp, timestamp); */
        tts->last_pcr = pcr;
        tts->pcr_timestamp = timestamp;
        tts->npackets = 0;
    }
    tts->prev_timestamp = timestamp;
    tts->npackets++;
    dst[0] = (timestamp >> 24)&0xFF;
    dst[1] = (timestamp >> 16)&0xFF;
    dst[2] = (timestamp >> 8)&0xFF;
    dst[3] = timestamp&0xFF;
#if 1
    memcpy(dst+4, src, B_TS_PKT_LEN);
#else
    {
        unsigned i;
        uint32_t *dst_32 = (uint32_t *)(dst+4);
        const uint32_t *src_32 = (uint32_t *)src;
        for(i=0;i<B_TS_PKT_LEN/sizeof(uint32_t);i++) {
            dst_32[i] = src_32[i];
        }
    }
#endif
}

#ifndef BCHP_CHIP

static int
run_timestamp(const char *file, uint16_t pcr_pid)
{
    FILE *fin, *fout;
    char name[256];
    b_tts_converter tts;

    fin = fopen(file, "rb");
    if(!fin) {
        return -1;
    }

    snprintf(name, sizeof(name),"%s.tts", file);
    fout = fopen(name, "wb");
    if(!fout) {
        return -1;
    }

    b_tts_converter_init(&tts, pcr_pid);
    for(;;) {
        uint8_t buf[B_TS_PKT_LEN];
        uint8_t dst[B_TS_PKT_LEN+4];
        int rc  = fread(buf, 1, sizeof(buf), fin);
        if(rc<=0) {
            break;
        }
#if 1
        b_tts_converter_process(&tts, buf, dst);
        fwrite(dst, 1, sizeof(dst), fout);
#else
        {
            uint32_t pcr;
            pcr = b_demux_ts_feed_getpcr(buf, pcr_pid);
            if(pcr) {
                fprintf(stderr, "offset:%u pcr %u\n",  (unsigned)ftell(fin), pcr);
            }
        }
#endif
    }
    fclose(fin);
    fclose(fout);
    return 0;
}

int main(int argc, const char *argv[])
{
    const char *fname = FILE_NAME;
    uint16_t pcr_pid = PCR_PID;

    if(argc>1) {
        fname = argv[1];
    }
    if(argc>2) {
        pcr_pid = strtol(argv[2], NULL, 0);
    }
    return run_timestamp(fname, pcr_pid);
}

#else /* BCHP_CHIP */

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playpump.h"

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void print_status(NEXUS_PlaypumpHandle playpump, NEXUS_StcChannelHandle stcChannel, NEXUS_VideoDecoderHandle vdecode)
{
    NEXUS_PlaypumpStatus status;
    NEXUS_VideoDecoderStatus vstatus;
    uint32_t stc;
    static uint32_t last_stc=0, last_pts=0;

    NEXUS_Playpump_GetStatus(playpump, &status);
    NEXUS_VideoDecoder_GetStatus(vdecode, &vstatus);
    NEXUS_StcChannel_GetStc(stcChannel, &stc);
    printf("\t[%10u] state playpump=%d%%, video=%d%%/pts=%#x/pics=%d, stc=%#x %d:%d\n",
        (unsigned)status.bytesPlayed,
        status.fifoSize?status.fifoDepth*100/status.fifoSize:0,
        vstatus.fifoSize?vstatus.fifoDepth*100/vstatus.fifoSize:0, vstatus.pts, vstatus.queueDepth,
        stc, stc-last_stc, vstatus.pts-last_pts);
    last_stc = stc;
    last_pts = vstatus.pts;
}

int main(int argc, const char *argv[])
{
    FILE *file;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpHandle playpump;
    BKNI_EventHandle event;
    const char *fname = FILE_NAME;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif
    uint16_t pcr_pid = PCR_PID;
    bool pcr_pacing=true; /* to show difference with and without PCR pacing */
    b_tts_converter tts;

    if(argc>1) {
        fname = argv[1];
    }
    if(argc>2) {
        pcr_pid = strtol(argv[2], NULL, 0);
    }
    if(argc>3) {
        pcr_pacing = !pcr_pacing;
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BKNI_CreateEvent(&event);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpump = NEXUS_Playpump_Open(0, &playpumpOpenSettings);

    b_tts_converter_init(&tts, pcr_pid);

    /* use stdio for file I/O to keep the example simple. */
    file = fopen(fname, "rb");
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        goto error;
    }

    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = event;
    if(pcr_pacing) {
        playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_eBinary;
        playpumpSettings.timestamp.pacing = true;
        playpumpSettings.timestamp.pacingMaxError = 52734;  /* 1 second */
    }
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);

    NEXUS_Playpump_Start(playpump);

    videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, pcr_pid, NULL);

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    if(pcr_pacing) {
        stcSettings.mode = NEXUS_StcChannelMode_ePcr;
        stcSettings.modeSettings.pcr.pidChannel = videoPidChannel;
        stcSettings.modeSettings.pcr.disableJitterAdjustment = true;
    } else {
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    }
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);


    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;

    /* Bring up video decoder and display. No audio to keep the example simple. */
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

    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    while (1) {
        for(;;) {
            void *buffer;
            size_t buffer_size;
            int n;
            NEXUS_Error rc;

            if (NEXUS_Playpump_GetBuffer(playpump, &buffer, &buffer_size))
                break;
            if (buffer_size == 0) {
                print_status(playpump, stcChannel, videoDecoder);
                BKNI_WaitForEvent(event, 1000);
                continue;
            }
#define MAX_READ (188*1024)
            /* The first call to get_buffer will return the entire playback buffer.
            If we use it, we're going to have to wait until the descriptor is complete,
            and then we're going to underflow. So use a max size. */
            if (buffer_size > MAX_READ) {
                buffer_size = MAX_READ;
            }
            if(pcr_pacing) {
                uint8_t file_buffer[MAX_READ];
                size_t read_size = B_TS_PKT_LEN * (buffer_size / (B_TS_PKT_LEN+4)); /* don't read more data then we could consume */
                size_t converted_size;
                int off;
                if(read_size==0) {
                    fprintf(stderr, "recycled %u bytes\n",  buffer_size);
                    rc = NEXUS_Playpump_WriteComplete(playpump, buffer_size, 0); /* buffer to small, discard it */
                    BDBG_ASSERT(!rc);
                    continue;
                }
                n = fread(file_buffer, 1, read_size, file);
                if (n < 0) {goto error;}
                if (n == 0) {
                    fprintf(stderr, "restart stream\n");
                    fseek(file, 0, SEEK_SET); /* seek to beginning and keep going */
                    continue;
                }
                for(converted_size=0,off=0;off<n;off+=B_TS_PKT_LEN, converted_size+=B_TS_PKT_LEN+4) {
                    b_tts_converter_process(&tts, file_buffer+off, (uint8_t *)buffer + converted_size);
                }
                if(converted_size>0) {
                    rc = NEXUS_Playpump_WriteComplete(playpump, 0, converted_size);
                    BDBG_ASSERT(!rc);
                } else {
                    fprintf(stderr, "dropped %d bytes\n", n);
                }
            } else {

                n = fread(buffer, 1, buffer_size, file);
                if (n < 0) goto error;
                if (n == 0) {break;}
                rc = NEXUS_Playpump_WriteComplete(playpump, 0, n);
                BDBG_ASSERT(!rc);
            }
        }
        /* wait for the decoder to reach the end of the content before looping */
        while (1) {
            NEXUS_VideoDecoderStatus status;
            print_status(playpump, stcChannel, videoDecoder);
            NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
            if (!status.queueDepth) break;
            BKNI_Sleep(100);
        }
        fseek(file, 0, SEEK_SET);
        NEXUS_Playpump_Flush(playpump);
    }
    return 0;

error:
    return 1;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif

#endif /* BCHP_CHIP */
