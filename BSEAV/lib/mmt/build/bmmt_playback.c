/***************************************************************************
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
 **************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bfifo.h"
#include "bfile_stdio.h"
#include "biobits.h"
#include "bmedia_util.h"

#include "bioatom.h"
#include "bpool.h"
#include "barena.h"
#include "bioatom.h"
#include "bpool.h"
#include "btlv_parser.h"
#include "bmmt_parser.h"
#include "bmmt_demux.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>


#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <linux/if.h>
#include <linux/if_tun.h>
#include "nexus_platform.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_composite_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include "nexus_frontend.h"
#include "nexus_parser_band.h"

struct nexus_state {
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_PlaypumpStatus playpumpStatus;
    bool stcSet;
    const struct mmt_cfg *cfg;
    struct {
        void *data;
        size_t size;
        unsigned offset;
    } buffer;
};

#define BTLV_PKT_READ_SIZE 16*1024
#define MAX_TLV_BUFFERS 8
#define MAX_TS_BUFFERS 360
BDBG_MODULE(bmmt_test);
BDBG_FILE_MODULE(bmmt_test_mmt);

static void
usage(const char *name, const char *opt)
{
    if (opt) {
        printf("Unknown option %s\n", opt);
    }
    printf(
           "%s: MMT tool\n"
           "Usage: %s [options] <input> <pes_out>\n"
           "where options:\n",
           name, name);
    printf("-help - this help\n"
           "-input_format - MPEG2TS-1 TLV-2 \n"
           " tlv_pid - valid if input_format=1 \n"
           "-msg <module> - set message level debug output for the module\n"
           "-video_ip <ADDR> - destination IP address and port for video stream\n"
           "-video_id <ID> - packet ID for video stream\n");
    printf(
           "-video_pes <ID> - PES ID for video stream\n"
           "-audio_ip <ADDR> - destination IP address and port for audio stream\n"
           "-audio_id <ID> - packet ID for audio stream\n"
           "-audio_pes <ID> - PES ID for audio stream\n"
           "-signalling_id <ID> - packet ID for the signalling data\n"
           );
    printf(
           "-display_format format - display format (%u - 720p, %u - 1080p, %u - 4KP60\n", NEXUS_VideoFormat_e720p, NEXUS_VideoFormat_e1080p60hz, NEXUS_VideoFormat_e3840x2160p60hz
           );
    return;
}
typedef enum mmt_input_format{
    emmt_input_format_mpeg2ts,
    emmt_input_format_tlv,
    emmt_input_format_max
}mmt_input_format;

typedef struct mmt_cfg {
    struct {
        bool set;
        btlv_ip_address ip;
        unsigned id;
        unsigned pes_id;
    } video, audio;
    unsigned signalling_id;
    mmt_input_format input_format;
    uint32_t tlv_pid;
    NEXUS_VideoFormat displayFormat;
} mmt_cfg;

struct mmt_state;

typedef struct mmt_stream_state {
    bmmt_timestamp_queue timestamp_queue;
    bmmt_demux_stream_t stream;
    struct {
        void *data;
        unsigned offset;
        unsigned length;
    } buffer;
    struct mmt_state *parent;
} mmt_stream_state;

typedef struct mmt_state {
    bool initial_timestamp_valid;
    batom_factory_t factory;
    btlv_ntp_time initial_timestamp;
    mmt_stream_state video;
    mmt_stream_state audio;
    bmmt_demux_t demux;
    FILE *fout;
    struct nexus_state nexus;
    struct {
        void *data;
        unsigned offset;
        unsigned length;
    } messageBuffer;
} mmt_state;

static int b_mmt_test_process_signalling_message(const mmt_cfg *cfg, mmt_state *state, batom_cursor *payload, const btlv_signalling_header *signalling_header)
{
    int rc;
    BSTD_UNUSED(cfg);
    rc = bmmt_demux_process_signaling_message(state->demux, payload, signalling_header);
    return rc;
}

static int b_mmt_test_process_mpu(const mmt_cfg *cfg, mmt_stream_state *state, const btlv_mmt_packet_header *header, batom_cursor *payload)
{
    int rc;
    BSTD_UNUSED(cfg);
    rc = bmmt_demux_stream_process_payload(state->parent->demux, state->stream, header, payload);
    return rc;
}

static void b_test_write_stream_data_begin(mmt_state *state)
{
    if (!state->fout) {
        state->nexus.buffer.offset = 0;
        state->nexus.buffer.size = 0;
    }
    return;
}

static void b_test_write_stream_data_chunk(mmt_state *state, const void *buf, unsigned len)
{
    NEXUS_Error rc;
    unsigned offset;
    unsigned left;

    if(state->fout) {
        fwrite(buf, len, 1, state->fout);
        return;
    }

    for(offset=0,left=len;left>0;) {
        unsigned to_copy;
        if(state->nexus.buffer.size > state->nexus.buffer.offset + left) {
            to_copy = left;
        } else {
            to_copy  = (state->nexus.buffer.size - state->nexus.buffer.offset);
        }
        if(to_copy>0) {
            BKNI_Memcpy((uint8_t *)state->nexus.buffer.data + state->nexus.buffer.offset, (uint8_t *)buf+offset, to_copy);
            left -= to_copy;
            offset += to_copy;
            state->nexus.buffer.offset += to_copy;
        } else { /* to_copy == 0 */
            unsigned dropThreshold = 16384;
            if(state->nexus.buffer.offset>0) {
                BDBG_MSG(("Complete %u", state->nexus.buffer.offset));
                NEXUS_Playpump_WriteComplete(state->nexus.playpump, 0, state->nexus.buffer.offset);
            }
            state->nexus.buffer.size = 0;
            state->nexus.buffer.offset = 0;
            rc = NEXUS_Playpump_GetBuffer( state->nexus.playpump, &state->nexus.buffer.data, &state->nexus.buffer.size);
            if(rc!=NEXUS_SUCCESS) {
                (void)BERR_TRACE(rc);
                return;
            }
            BDBG_MSG(("GetBuffer:%u %u", (unsigned)((uint8_t *)state->nexus.buffer.data- (uint8_t *)state->nexus.playpumpStatus.bufferBase), (unsigned)state->nexus.buffer.size));
            if(state->nexus.buffer.size<dropThreshold) {
                if( (uint8_t *)state->nexus.playpumpStatus.bufferBase + (state->nexus.playpumpStatus.fifoSize - dropThreshold) <= (uint8_t *)state->nexus.buffer.data) {
                    BDBG_MSG(("Drop %u", (unsigned)state->nexus.buffer.size));
                    NEXUS_Playpump_WriteComplete(state->nexus.playpump, state->nexus.buffer.size , 0);
                    {
                        NEXUS_VideoDecoderStatus vstatus;
                        NEXUS_AudioDecoderStatus astatus;
                        uint32_t stc;


                        NEXUS_VideoDecoder_GetStatus(state->nexus.videoDecoder, &vstatus);
                        NEXUS_AudioDecoder_GetStatus(state->nexus.audioDecoder, &astatus);
                        NEXUS_StcChannel_GetStc(state->nexus.stcChannel, &stc);
                        BDBG_LOG(("video %u/%u (%u%%) pts=%#x, stc=%#x (diff %d) queueDepth=%d", vstatus.fifoDepth, vstatus.fifoSize,
                        vstatus.fifoSize ? vstatus.fifoDepth * 100 / vstatus.fifoSize : 0,
                        vstatus.pts, stc, vstatus.ptsStcDifference, vstatus.queueDepth));
                        BDBG_LOG(("audio %u/%u (%u%%) pts=%#x, stc=%#x (diff %d), queuedFrames=%d", astatus.fifoDepth, astatus.fifoSize,
                            astatus.fifoSize ? astatus.fifoDepth * 100 / astatus.fifoSize : 0,
                            astatus.pts, stc, astatus.ptsStcDifference, astatus.queuedFrames));

                    }
                } else {
                    BDBG_MSG(("Wait"));
                    BKNI_Sleep(10); /* wait 10 milliseconds */
                }
                state->nexus.buffer.size = 0;
            }
        }
    }
    return;
}

static void b_test_write_stream_data_end(mmt_state *state)
{

    if(!state->fout && state->nexus.buffer.offset>0) {
        BDBG_MSG(("Done %u", state->nexus.buffer.offset));
        NEXUS_Playpump_WriteComplete(state->nexus.playpump, 0, state->nexus.buffer.offset);
    }
    return;
}

static void b_test_stream_data(void *stream_context, batom_accum_t data, const bmmt_demux_time_info *time_info)
{
    mmt_stream_state *state = stream_context;
    batom_cursor cursor;
    unsigned i;
    uint8_t pes_header[BMEDIA_PES_HEADER_MAX_SIZE];
    size_t pes_header_length;
    size_t length = 0;

    if(state == &state->parent->video) {
        length = 0; /* unbounded PES for video*/
    } else {
        length = batom_accum_len(data); /* bounded otherwise */
    }
    if (!state->parent->fout) {
        if(time_info->mpu_time_valid && time_info->pes_info.pts_valid)
        {
            uint32_t stc;
            if(!state->parent->nexus.stcSet)
            {
                    NEXUS_Error rc;
                    stc = time_info->mpu_time;
                    BDBG_MSG(("MPU:%x PTS:%x", (unsigned)(time_info->mpu_time), (unsigned)time_info->pes_info.pts));
                    rc = NEXUS_StcChannel_SetStc(state->parent->nexus.stcChannel, stc);
                    state->parent->nexus.stcSet = rc==NEXUS_SUCCESS;
            }
        }
    }

    BDBG_MSG(("stream_data:%s %u bytes (PTS:%u)", state == &state->parent->video?"video":"audio", (unsigned)batom_accum_len(data), time_info->pes_info.pts_valid?time_info->pes_info.pts:0));
    pes_header_length = bmedia_pes_header_init(pes_header, length , &time_info->pes_info);
    b_test_write_stream_data_begin(state->parent);
    b_test_write_stream_data_chunk(state->parent, pes_header, pes_header_length);
    batom_cursor_from_accum(&cursor, data);
    for(i=0;i<cursor.count;i++) {
        b_test_write_stream_data_chunk(state->parent, cursor.vec[i].base, cursor.vec[i].len);
    }
    b_test_write_stream_data_end(state->parent);
    state->buffer.offset = 0;
    return;
}

static void
b_atom_free_mmt(batom_t atom, void *user)
{
    const batom_vec *vec = batom_get_vec(atom, 0);
    BDBG_ASSERT(user);
    BDBG_MSG(("free_mmt %p", vec->base));
    BSTD_UNUSED(atom);
    BKNI_Free(vec->base);
    return;
}

static const batom_user b_atom_mmt = {
    b_atom_free_mmt,
    0
};

int b_test_copy_payload(void *context, batom_accum_t accum, batom_cursor *cursor, unsigned bytes)
{
    void *data;
    batom_t atom;
    struct mmt_state *state = context;
    size_t copied_bytes;

#if 1
    data = BKNI_Malloc(bytes);
#else
    {
        static char buf[16384];
        data = buf;
    }
#endif
    if(data==NULL) {
        goto err_alloc;
    }
    copied_bytes = batom_cursor_copy(cursor, data, bytes);
    if(copied_bytes != bytes) {
        BDBG_LOG(("%u %u", (unsigned)copied_bytes, bytes));
        goto err_copy;
    }
    atom = batom_from_range(state->factory, data, bytes, &b_atom_mmt, NULL);
    if(atom==NULL) {
        goto err_atom;
    }
    batom_accum_add_atom(accum, atom);
    batom_release(atom);
    return 0;

err_atom:
err_copy:
    BKNI_Free(data);
err_alloc:
    return -1;
}

int b_test_stream_copy_payload(void *stream_context, batom_accum_t accum, batom_cursor *cursor, unsigned bytes)
{
    mmt_stream_state *state = stream_context;
    if(state->buffer.offset + bytes < state->buffer.length) {
        void *data = (uint8_t *)state->buffer.data + state->buffer.offset;
        size_t copied_bytes;
        copied_bytes = batom_cursor_copy(cursor, data, bytes);
        if(copied_bytes != bytes) {
            return -1;
        }
        batom_accum_add_range(accum, (uint8_t *)state->buffer.data + state->buffer.offset, bytes);
        state->buffer.offset += bytes;
        return 0;
    }
    return b_test_copy_payload(state->parent, accum, cursor, bytes);
}

static int b_mmt_test_process_single_packet(const mmt_cfg *cfg, mmt_state *state, btlv_ip_parser *ip_parser, batom_cursor *cursor)
{
    int rc=0;
    btlv_ip_parser_result ip_result;


    if(cfg->video.set || cfg->audio.set) {
        rc = btlv_ip_parser_process(ip_parser, cursor, &ip_result);
        if(rc==0) {
            if(cfg->video.set || cfg->audio.set) {
                batom_cursor payload;
                if(btlv_ip_demux(&ip_result, &cfg->video.ip, &payload) || btlv_ip_demux(&ip_result, &cfg->audio.ip, &payload)) {
                    btlv_mmt_packet_header mmt;
                    if(btlv_parse_mmt_header(&payload,&mmt)==0) {
                        if(mmt.packet_id==cfg->video.id && mmt.type==BTLV_MMT_TYPE_MPU) {
                            b_mmt_test_process_mpu(cfg, &state->video, &mmt, &payload);
                        } else if(mmt.packet_id==cfg->audio.id && mmt.type==BTLV_MMT_TYPE_MPU) {
                            b_mmt_test_process_mpu(cfg, &state->audio, &mmt, &payload);
                        } else if(mmt.packet_id==cfg->signalling_id && mmt.type == BTLV_MMT_TYPE_SIGNALLING_MESSAGE) {
                            btlv_signalling_header signalling_header;
                            if(btlv_parse_signalling_header(&payload, &signalling_header)==0) {
                                if(signalling_header.f_i==0) {
                                    b_mmt_test_process_signalling_message(cfg, state, &payload, &signalling_header);
                                }
                                else
                                {
                                    if (signalling_header.f_i >= 1 && signalling_header.f_i <=3) {
                                        size_t len = batom_cursor_size(&payload);
                                        batom_cursor_copy(&payload,state->messageBuffer.data + state->messageBuffer.offset,len);
                                        state->messageBuffer.offset += (unsigned)len;
                                    }
                                    if (signalling_header.f_i == 3 && state->messageBuffer.offset)
                                    {
                                        batom_vec signal_vec;
                                        batom_cursor signal_payload;
                                        BATOM_VEC_INIT(&signal_vec,state->messageBuffer.data,state->messageBuffer.offset);
                                        batom_cursor_from_vec(&signal_payload,&signal_vec,1);
                                        b_mmt_test_process_signalling_message(cfg, state, &signal_payload, &signalling_header);
                                        state->messageBuffer.offset = 0;
                                    }

                                }

                            }
                        } else {
                            BDBG_MSG(("unknown packed_id:%#x type:%#x", (unsigned)mmt.packet_id, (unsigned)mmt.type));
                        }
                    }
                } else {
                    BDBG_MSG(("unknown IP"));
                }
            }
        }
    }
    return rc;
}

static int platform_init(bool mmt_to_pes)
{
    if (mmt_to_pes) {
         BKNI_Init();
         BDBG_Init();
         BDBG_SetLevel(BDBG_eWrn);
    }
    else {
        NEXUS_PlatformSettings platformSettings;
        NEXUS_Error rc;
        NEXUS_Platform_GetDefaultSettings(&platformSettings);
        platformSettings.openFrontend = false;
        platformSettings.transportModuleSettings.maxDataRate.parserBand[NEXUS_ParserBand_e0] = 50 * 1000 * 1000;
        rc = NEXUS_Platform_Init(&platformSettings);
        if (rc) return -1;

    }
    return 0;
}

static int platform_setup(mmt_cfg *cfg, mmt_state *state)
{

    if (state->fout) {
        BSTD_UNUSED(cfg);
    }
    else {
        NEXUS_PlatformConfiguration platformConfig;
        NEXUS_StcChannelSettings stcSettings;
        NEXUS_VideoDecoderSettings videoDecoderSettings;
        NEXUS_VideoDecoderStartSettings videoProgram;
        NEXUS_AudioDecoderStartSettings audioProgram;
        NEXUS_PlaypumpSettings playpumpSettings;
    #if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_HdmiOutputStatus hdmiStatus;
    #endif
        NEXUS_DisplaySettings displaySettings;
        NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
        NEXUS_Error rc;

        NEXUS_Platform_GetConfiguration(&platformConfig);

        state->nexus.playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
        BDBG_ASSERT(state->nexus.playpump);
        NEXUS_Playpump_GetSettings(state->nexus.playpump, &playpumpSettings);
        playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
        NEXUS_Playpump_SetSettings(state->nexus.playpump, &playpumpSettings);

        NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0;
        stcSettings.mode = NEXUS_StcChannelMode_eHost;
        state->nexus.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

        /* Bring up audio decoders and outputs */
        NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
        audioDecoderOpenSettings.fifoSize = 512*1024;
        state->nexus.audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
    #if NEXUS_NUM_AUDIO_DACS
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
            NEXUS_AudioDecoder_GetConnector(state->nexus.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    #endif
    #if NEXUS_NUM_SPDIF_OUTPUTS
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(state->nexus.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    #endif
    #if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_AudioOutput_AddInput(
            NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
            NEXUS_AudioDecoder_GetConnector(state->nexus.audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    #endif

        /* Bring up video display and outputs */
        NEXUS_Display_GetDefaultSettings(&displaySettings);
        if(cfg->displayFormat) {
            displaySettings.format = cfg->displayFormat;
        }
        state->nexus.display = NEXUS_Display_Open(0, &displaySettings);
        state->nexus.window = NEXUS_VideoWindow_Open(state->nexus.display, 0);

    #if NEXUS_NUM_COMPONENT_OUTPUTS
        NEXUS_Display_AddOutput(state->nexus.display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    #endif
    #if NEXUS_NUM_COMPOSITE_OUTPUTS
        NEXUS_Display_AddOutput(state->nexus.display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    #endif
    #if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_Display_AddOutput(state->nexus.display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
        rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
        if ( !rc && hdmiStatus.connected )
        {
            /* If current display format is not supported by monitor, switch to monitor's preferred format.
               If other connected outputs do not support the preferred format, a harmless error will occur. */
            NEXUS_Display_GetSettings(state->nexus.display, &displaySettings);
            if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
                displaySettings.format = hdmiStatus.preferredVideoFormat;
                NEXUS_Display_SetSettings(state->nexus.display, &displaySettings);
            }
        }
    #endif

        /* bring up decoder and connect to display */
        state->nexus.videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
        NEXUS_VideoDecoder_GetSettings(state->nexus.videoDecoder, &videoDecoderSettings);
        videoDecoderSettings.maxWidth = 3840;
        videoDecoderSettings.maxHeight = 2160;
        NEXUS_VideoDecoder_SetSettings(state->nexus.videoDecoder, &videoDecoderSettings);
        NEXUS_VideoWindow_AddInput(state->nexus.window, NEXUS_VideoDecoder_GetConnector(state->nexus.videoDecoder));

        state->nexus.videoPidChannel = NEXUS_Playpump_OpenPidChannel(state->nexus.playpump, cfg->video.pes_id, NULL);
        state->nexus.audioPidChannel = NEXUS_Playpump_OpenPidChannel(state->nexus.playpump, cfg->audio.pes_id, NULL);

        /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = NEXUS_VideoCodec_eH265;
        videoProgram.pidChannel = state->nexus.videoPidChannel;
        videoProgram.stcChannel = state->nexus.stcChannel;
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.codec = NEXUS_AudioCodec_eAacLoas;
        audioProgram.pidChannel = state->nexus.audioPidChannel;
        audioProgram.stcChannel = state->nexus.stcChannel;

        /* Start decoders */
        NEXUS_VideoDecoder_Start(state->nexus.videoDecoder, &videoProgram);
        if (cfg->audio.set) {
            NEXUS_AudioDecoder_Start(state->nexus.audioDecoder, &audioProgram);
        }

        NEXUS_Playpump_Start(state->nexus.playpump);
        NEXUS_Playpump_GetStatus(state->nexus.playpump, &state->nexus.playpumpStatus);

        state->nexus.cfg = cfg;
    }

    return 0;
}

static void platform_shutdown(mmt_state *state)
{
    /* Bring down system */

    if(!state->fout) {
        NEXUS_VideoDecoder_Stop(state->nexus.videoDecoder);
        NEXUS_AudioDecoder_Stop(state->nexus.audioDecoder);
        NEXUS_Playpump_Stop(state->nexus.playpump);
        NEXUS_Playpump_Close(state->nexus.playpump);
        NEXUS_VideoDecoder_Close(state->nexus.videoDecoder);
        NEXUS_AudioDecoder_Close(state->nexus.audioDecoder);
        NEXUS_VideoWindow_Close(state->nexus.window);
        NEXUS_Display_Close(state->nexus.display);
        NEXUS_StcChannel_Close(state->nexus.stcChannel);
    }
    return;
}

static void platform_uninit(bool mmt_to_pes)
{
    if (mmt_to_pes) {
         BDBG_Uninit();
         BKNI_Uninit();
    }else {
        NEXUS_Platform_Uninit();
    }
    return;
}

int main(int argc, const char *argv[])
{
    FILE *fin=NULL;
    const char *file=NULL;
    const char *pes=NULL;
    int arg;
    int file_arg=0;
    batom_factory_stats factory_stats;
    mmt_cfg cfg;
    mmt_state state;
    bmmt_demux_config demux_config;
    bmmt_demux_stream_config demux_stream_config;


    BKNI_Memset(&state,0,sizeof(state));
    BKNI_Memset(&cfg,0,sizeof(cfg));
    cfg.video.ip.type = btlv_ip_address_invalid;
    cfg.video.pes_id = 0xE0;
    cfg.audio.ip.type = btlv_ip_address_invalid;
    cfg.audio.pes_id = 0xC0;
    cfg.input_format = emmt_input_format_max;
    cfg.tlv_pid = 0;

    arg=1;
    while (argc>arg) {
        if (!strcmp("-help",argv[arg])) {
            usage(argv[0], NULL);
            goto done;
        } else if (!strcmp("-msg",argv[arg]) && argc>arg+1) {
            arg++;
            BDBG_SetModuleLevel(argv[arg], BDBG_eMsg);
        }  else if (!strcmp("-video_ip",argv[arg]) && argc>arg+1) {
            arg++;
            if(bmmt_parse_ipv6_address(argv[arg], &cfg.video.ip.address.ipv6)!=0) {
                BDBG_ERR(("Invald IP address '%s'", argv[arg]));
            }
            cfg.video.ip.type = btlv_ip_address_ipv6;
            cfg.video.set = true;
        } else if (!strcmp("-video_id",argv[arg]) && argc>arg+1) {
            arg++;
            cfg.video.id = strtol(argv[arg],NULL,0);
            cfg.video.set = true;
        } else if (!strcmp("-video_pes",argv[arg]) && argc>arg+1) {
            arg++;
            cfg.video.pes_id = strtol(argv[arg],NULL,0);
        } else if (!strcmp("-audio_ip",argv[arg]) && argc>arg+1) {
            arg++;
            if(bmmt_parse_ipv6_address(argv[arg], &cfg.audio.ip.address.ipv6)!=0) {
                BDBG_ERR(("Invald IP address '%s'", argv[arg]));
            }
            cfg.audio.ip.type = btlv_ip_address_ipv6;
            cfg.audio.set = true;
        } else if (!strcmp("-audio_id",argv[arg]) && argc>arg+1) {
            arg++;
            cfg.audio.id = strtol(argv[arg],NULL,0);
            cfg.audio.set = true;
        } else if (!strcmp("-audio_pes",argv[arg]) && argc>arg+1) {
            arg++;
            cfg.audio.pes_id = strtol(argv[arg],NULL,0);
        } else if (!strcmp("-signalling_id",argv[arg]) && argc>arg+1) {
            arg++;
            cfg.signalling_id = strtol(argv[arg],NULL,0);
        }
        else if (!strcmp("-input_format",argv[arg]) && argc>arg+1) {
            arg++;
            cfg.input_format = strtol(argv[arg],NULL,0)-1;
        }
        else if (!strcmp("-tlv_pid",argv[arg]) && argc>arg+1) {
            arg++;
            cfg.tlv_pid = strtol(argv[arg],NULL,0);
        }
        else if (!strcmp("-display_format",argv[arg]) && argc>arg+1) {
            arg++;
            cfg.displayFormat = strtol(argv[arg],NULL,0);
        }
        else if (*argv[arg]!='\0' && (*argv[arg]!='-' || argv[arg][1]=='\0'))  {
            switch(file_arg) {
            case 0:
                 file = argv[arg];
                break;
            case 1: pes = argv[arg]; break;
            default:
                usage(argv[0], argv[arg]);
                goto done;
            }
            file_arg++;
        } else {
            usage(argv[0], argv[arg]);
            goto done;
        }

        arg++;
    }
    BDBG_ERR(("cfg.input_format %d cfg.tlv_pid %x",cfg.input_format, cfg.tlv_pid));
    if (!file || cfg.input_format == emmt_input_format_max) {
        usage(argv[0], NULL);
        goto done;
    }

    if(cfg.input_format == emmt_input_format_mpeg2ts && cfg.tlv_pid==0) {
        usage(argv[0], NULL);
        goto done;
    }


    if(file) {
        if(strcmp(file,"-")!=0) {
            fin = fopen(file,"rb");
            if (!fin) {
                perror(file);return 1;
            }
        } else {
            fin = stdin;
        }
    }

    if(pes==NULL) {
        state.fout = NULL;
    } else if(strcmp(pes,"-")!=0) {
        state.fout = fopen(pes,"wb");
        if (!state.fout) {
            perror(pes);return 1;
        }
    } else {
        state.fout = stdout;
    }

    platform_init((state.fout==NULL)?false:true);

    if(platform_setup(&cfg, &state)!=0) {
        goto done;
    }

    state.video.timestamp_queue=bmmt_timestamp_queue_create();
    state.audio.timestamp_queue=bmmt_timestamp_queue_create();
    state.initial_timestamp_valid = false;
    state.video.stream = NULL;
    state.audio.stream = NULL;

    state.factory = batom_factory_create(bkni_alloc, 256);

    bmmt_demux_config_init(&demux_config);
    demux_config.context = &state;
    demux_config.copy_payload = b_test_copy_payload;
    state.demux = bmmt_demux_create(state.factory, &demux_config);
    if(cfg.video.set) {
        state.video.buffer.length = 2*1024*1024;
        state.video.buffer.offset = 0;
        state.video.buffer.data = BKNI_Malloc(state.video.buffer.length);
        state.video.parent = &state;
        BDBG_ASSERT(state.video.buffer.data);
        bmmt_demux_stream_config_init(&demux_stream_config);
        demux_stream_config.stream_type = bmmt_stream_type_h265;
        demux_stream_config.packet_id = cfg.video.id;
        demux_stream_config.pes_stream_id = cfg.video.pes_id;
        demux_stream_config.stream_context = &state.video;
        demux_stream_config.stream_data = b_test_stream_data;
        demux_stream_config.copy_payload = b_test_stream_copy_payload;
        state.video.stream = bmmt_demux_stream_create(state.demux, &demux_stream_config);
        BDBG_ASSERT(state.video.stream);
    }
    if(cfg.audio.set) {
        state.audio.buffer.length = 2*1024*1024;
        state.audio.buffer.offset = 0;
        state.audio.buffer.data = BKNI_Malloc(state.audio.buffer.length);
        state.audio.parent = &state;
        BDBG_ASSERT(state.audio.buffer.data);
        bmmt_demux_stream_config_init(&demux_stream_config);
        demux_stream_config.stream_type = bmmt_stream_type_aac;
        demux_stream_config.packet_id = cfg.audio.id;
        demux_stream_config.pes_stream_id = cfg.audio.pes_id;
        demux_stream_config.stream_context = &state.audio;
        demux_stream_config.stream_data = b_test_stream_data;
        demux_stream_config.copy_payload = b_test_stream_copy_payload;
        state.audio.stream = bmmt_demux_stream_create(state.demux, &demux_stream_config);
        BDBG_ASSERT(state.audio.stream);
    }
     if (cfg.signalling_id) {
        state.messageBuffer.length = 16*1024*5;
        state.messageBuffer.offset = 0;
        state.messageBuffer.data = BKNI_Malloc(state.messageBuffer.length);
    }

    if(cfg.video.set || cfg.audio.set)
    {
        btlv_parser parser;
        btlv_ip_parser ip_parser;
        struct {
            void *ts_buf;
        } io_data[cfg.input_format == emmt_input_format_mpeg2ts?MAX_TS_BUFFERS:MAX_TLV_BUFFERS];
        unsigned last_io_data;
        BKNI_Memset(io_data, 0, sizeof(io_data));
        if(cfg.input_format == emmt_input_format_mpeg2ts) {
            btlv_parser_init(&parser, cfg.tlv_pid);
        }
        else{
            btlv_parser_init(&parser, BTLV_DEFAULT_PID);
        }
        btlv_ip_parser_init(&ip_parser);
        for(last_io_data=0;;) {
            int rc;
            void *buf;
            btlv_parser_packets packets;
            if(last_io_data>=sizeof(io_data)/sizeof(io_data[0])) {
                BDBG_ERR(("last_io_data %x",last_io_data));
                    BDBG_ASSERT(0);
            }
            buf  = io_data[last_io_data].ts_buf;
            if(buf==NULL) {
                buf = BKNI_Malloc((cfg.input_format == emmt_input_format_mpeg2ts)?BMPEG2TS_PKT_LEN:BTLV_PKT_READ_SIZE);
                BDBG_ASSERT(buf);
                io_data[last_io_data].ts_buf = buf;
            }
            rc = fread(buf,1,(cfg.input_format == emmt_input_format_mpeg2ts)?BMPEG2TS_PKT_LEN:BTLV_PKT_READ_SIZE,fin);
            if(rc<=0) {
                break;
            }
            if(cfg.input_format == emmt_input_format_tlv) {
                rc = btlv_parser_process_tlv(&parser, buf,rc, &packets);
            }
            else
            {
                rc = btlv_parser_process_mpeg2ts(&parser, buf, &packets);
            }
            if(rc==0) {
                unsigned i;
                if(packets.packet_valid) {
                    rc = b_mmt_test_process_single_packet(&cfg, &state, &ip_parser, &packets.packet);
                }
                for(i=0;i<packets.count;i++) {
                    batom_cursor cursor;
                    batom_cursor_from_vec(&cursor, packets.packets+i, 1);
                    rc = b_mmt_test_process_single_packet(&cfg, &state, &ip_parser, &cursor);
                }
                if(packets.keep_previous_packets ) {
                    last_io_data++;
                } else {
                    if(packets.keep_current_packet) { /* current packet to the first */
                        io_data[last_io_data].ts_buf = io_data[0].ts_buf;
                        io_data[0].ts_buf = buf;
                        last_io_data = 1;
                    } else {
                        last_io_data = 0; /* reset buffer */
                    }
                }
            } else if(rc==BTLV_RESULT_UNKNOWN_PID) {
                continue;
            } else {
                btlv_parser_reset(&parser);
                last_io_data = 0;
            }
            fflush(state.fout);
        }
        btlv_ip_parser_shutdown(&ip_parser);
        for(last_io_data=0;last_io_data<sizeof(io_data)/sizeof(io_data[0]);last_io_data++) {
            void *buf  = io_data[last_io_data].ts_buf;
            if(buf) {
                BKNI_Free(buf);
                io_data[last_io_data].ts_buf = NULL;
            }
        }
    }

    if(state.video.stream) {
        bmmt_demux_stream_destroy(state.demux, state.video.stream);
        BKNI_Free(state.video.buffer.data);
    }
    if(state.audio.stream) {
        bmmt_demux_stream_destroy(state.demux, state.audio.stream);
        BKNI_Free(state.audio.buffer.data);
    }

    if(state.messageBuffer.data) {
        BKNI_Free(state.messageBuffer.data);
    }

    bmmt_demux_destroy(state.demux);
    bmmt_timestamp_queue_destroy(state.video.timestamp_queue);
    bmmt_timestamp_queue_destroy(state.audio.timestamp_queue);

    if(state.fout && state.fout!=stdout) {
        fclose(state.fout);
    }
    if(fin!=NULL && fin!=stdin) {
        fclose(fin);
    }
    batom_factory_get_stats(state.factory, &factory_stats);
    BDBG_WRN(("status: atoms[live:%u allocated:%u freed:%u] alloc[pool:%u/%u arena:%u/%u alloc:%u/%u]", factory_stats.atom_live, factory_stats.atom_allocated, factory_stats.atom_freed, factory_stats.alloc_pool, factory_stats.free_pool, factory_stats.alloc_arena, factory_stats.free_arena, factory_stats.alloc_alloc, factory_stats.free_alloc));

    batom_factory_dump(state.factory);
    batom_factory_destroy(state.factory);

    platform_shutdown(&state);
done:
    platform_uninit((pes==NULL)?false:true);

    return 0;
}
