/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *******************************************************************************/

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

#if WITH_NEXUS
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
        NEXUS_PlaypumpHandle playpump;
        NEXUS_PlaypumpStatus playpumpStatus;
        NEXUS_RecpumpHandle recpump;
        NEXUS_PidChannelHandle allPassPidChannel;

        NEXUS_FrontendHandle frontend;
        NEXUS_ParserBand parserBand;

        NEXUS_RemuxHandle remux;
    } feed;
    struct {
        void *data;
        size_t size;
        unsigned offset;
    } buffer;
};
#else /* #if WITH_NEXUS  */
struct nexus_state {
    unsigned unused;
};
#endif


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
           "-msg <module> - set message level debug output for the module\n"
           "-pcap - save PCAP type of dump\n"
           "-tlv - demux TS to TLV\n"
           "-tun - inject PCAP data to the TUN\n"
           "-video_ip <ADDR> - destination IP address and port for video stream\n"
           "-video_id <ID> - packet ID for video stream\n");
    printf(
           "-video_pes <ID> - PES ID for video stream\n"
           "-audio_ip <ADDR> - destination IP address and port for audio stream\n"
           "-audio_id <ID> - packet ID for audio stream\n"
           "-audio_pes <ID> - PES ID for audio stream\n"
           "-signalling_id <ID> - packet ID for the signalling data\n"
           );
#if WITH_NEXUS
    printf(
           "-live - feed data through the transport HW\n"
           "-capture - capture MPEG-2 TS stream\n"
           "-remux - route data through remux\n"
           "-transport_bitrate - bitrate of the transport stream, bps\n"
           "-live_buffer - buffer for live data, milliseconds\n"
          );
    printf(
           "-display_format format - display format (%u - 720p, %u - 1080p, %u - 4KP60\n", NEXUS_VideoFormat_e720p, NEXUS_VideoFormat_e1080p60hz, NEXUS_VideoFormat_e3840x2160p60hz
           );
#endif
    return;
}

typedef struct mmt_cfg {
    bool tlv;
    bool pcap;
    bool tun;
    bool tap;
    struct {
        bool set;
        btlv_ip_address ip;
        unsigned id;
        unsigned pes_id;
    } video, audio;
    unsigned signalling_id;
    unsigned transport_bitrate;
    unsigned live_buffer;
    bool live;
#if WITH_NEXUS
    NEXUS_VideoFormat displayFormat;
    struct {
        bool live;
        NEXUS_FrontendQamMode mode;
        unsigned frequency;
        bool capture;
        bool remux;
    } feed;
#endif
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
} mmt_state;



static int pcap_write32(FILE *f, uint32_t data)
{
    return fwrite(&data, sizeof(data), 1, f);
}

static int pcap_write16(FILE *f, uint16_t data)
{
    return fwrite(&data, sizeof(data), 1, f);
}

static int pcap_read32(FILE *f, uint32_t *data)
{
    if(fread(data, sizeof(*data), 1, f)!=1) {
        return -1;
    }
    return 0;
}

static int pcap_read16(FILE *f, uint16_t *data)
{
    if(fread(data, sizeof(*data), 1, f)!=1) {
        return -1;
    }
    return 0;
}

int pcap_write_header(FILE *f, unsigned max_length, unsigned data_link)
{
    /* https://wiki.wireshark.org/Development/LibpcapFileFmtrmat
     * Global Header
     */
    pcap_write32(f, 0xa1b2c3d4); /* magic_number */
    pcap_write16(f, 2); /* version_major */
    pcap_write16(f, 4); /* version_minor */
    pcap_write32(f, 0); /* thiszone */
    pcap_write32(f, 0); /* sigfigs */
    pcap_write32(f, max_length); /* snaplen */
    pcap_write32(f, data_link); /* network */
    return 0;
}

int pcap_write_packet(FILE *f, unsigned type, const batom_cursor *cursor)
{
    size_t payload = batom_cursor_size(cursor);
    unsigned i;

    /* https://wiki.wireshark.org/Development/LibpcapFileFormat
     * Record (Packet) Header
     */
    pcap_write32(f, 0); /* ts_sec */
    pcap_write32(f, 0); /* ts_usec */
#if 0
    payload += 6 + 6 + 2; /* Ethernet header */

    /* https://www.ietf.org/rfc/rfc2464.txt
     *   Transmission of IPv6 Packets over Ethernet Networks
     * 3.  Frame Format
     */
    pcap_write16(f, 0); pcap_write32(f, 0); /* destination */
    pcap_write16(f, 0); pcap_write32(f, 0); /* source */
    fputc(0x86, f); fputc(f, 0xDD); /* type */
#endif
    payload += 4;
    pcap_write32(f, payload); /* incl_len */
    pcap_write32(f, payload); /* orig_len */

    pcap_write32(f, type);
    if(cursor->left>0) {
        i = cursor->pos - 1;
        fwrite((const uint8_t *)cursor->vec[i].base + (cursor->vec[i].len - cursor->left), cursor->left, 1, f);
    }
    for(i=cursor->pos;i<cursor->count;i++) {
        fwrite(cursor->vec[i].base, cursor->vec[i].len, 1, f);
    }
    return 0;
}

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

#if WITH_NEXUS
static void b_test_write_stream_data_begin(mmt_state *state)
{
    state->nexus.buffer.offset = 0;
    state->nexus.buffer.size = 0;
    return;
}

static void b_test_write_stream_data_chunk(mmt_state *state, const void *buf, unsigned len)
{
    NEXUS_Error rc;
    unsigned offset;
    unsigned left;

    if(state->fout) {
#if 1
        fwrite(buf, len, 1, state->fout);
#endif
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
    if(state->nexus.buffer.offset>0) {
        BDBG_MSG(("Done %u", state->nexus.buffer.offset));
        NEXUS_Playpump_WriteComplete(state->nexus.playpump, 0, state->nexus.buffer.offset);
    }
    return;
}
#else /* WITH_NEXUS */
#define b_test_write_stream_data_begin(state)
#define b_test_write_stream_data_chunk(state, buf, len) fwrite(buf, len, 1, state->fout)
#define b_test_write_stream_data_end(state)
#endif

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
    BDBG_MSG(("stream_data:%s %u bytes (PTS:%u)", state == &state->parent->video?"video":"audio", (unsigned)batom_accum_len(data), time_info->pes_info.pts_valid?time_info->pes_info.pts:0));
#if WITH_NEXUS
    if(state->parent->nexus.cfg->transport_bitrate || state->parent->nexus.cfg->feed.live) {
        if(time_info->mpu_time_valid && time_info->pes_info.pts_valid && state->parent->nexus.cfg->live_buffer>0 ) {
            uint32_t stc;
            unsigned delay = state->parent->nexus.cfg->live_buffer * (45000/1000);
            if(0 && state->parent->nexus.stcSet && delay) {
                unsigned delta;
                stc = 0;
                NEXUS_StcChannel_GetStc(state->parent->nexus.stcChannel, &stc);
                delta = time_info->mpu_time - stc;
                if(delta>100*delay) {
                    state->parent->nexus.stcSet = false;
                }
            }
            if(!state->parent->nexus.stcSet) {
                NEXUS_Error rc;
                stc = time_info->mpu_time - delay;
                BDBG_LOG(("MPU:%ums PTS:%ums delay %ums STC:%#x(%ums)", (unsigned)(time_info->mpu_time/45), (unsigned)(time_info->pes_info.pts/45), (unsigned)(delay/45), (unsigned)stc, (unsigned)(stc/45)));
                rc = NEXUS_StcChannel_SetStc(state->parent->nexus.stcChannel, stc);
                state->parent->nexus.stcSet = rc==NEXUS_SUCCESS;
            }
       }
    }
#endif
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
#if 1
    BKNI_Free(vec->base);
#endif
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

    if(cfg->tlv) {
        unsigned i;
        bool debug=false;
        if(debug) batom_cursor_dump(cursor,"packet");
        for(i=0;i<cursor->count;i++) {
            fwrite(cursor->vec[i].base, cursor->vec[i].len, 1, state->fout);
        }
    }

    if(cfg->pcap || cfg->video.set || cfg->audio.set) {
        rc = btlv_ip_parser_process(ip_parser, cursor, &ip_result);
        if(rc==0) {
            if(cfg->pcap) {
                switch(ip_result.type) {
                case btlv_ip_parser_result_ipv6:
                    BDBG_MSG(("IPV6 packet %u bytes", (unsigned)batom_cursor_size(&ip_result.data)));
                    pcap_write_packet(state->fout, 24  /* BSD_AFNUM_INET6_BSD */, &ip_result.data);
                    break;
                case btlv_ip_parser_result_ipv4:
                    BDBG_MSG(("IPV4 packet %u bytes", (unsigned)batom_cursor_size(&ip_result.data)));
                    pcap_write_packet(state->fout, 2 /* BSD_AFNUM_INET */, &ip_result.data);
                    break;
                default:
                    break;
                }
            }
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

static int inject_tun(const mmt_cfg *cfg, FILE *fin)
{
    struct ifreq ifr;
    int fd;
    int rc;
    const char *dev = "/dev/net/tun";
    int ifd;
    char tun[64];
    uint32_t data32;
    uint16_t data16;

    fd = open(dev, O_RDWR);
    if(fd<0) {
        perror(dev);
        return -1;
    }
    BKNI_Memset(&ifr, 0, sizeof(ifr));
    BKNI_Snprintf(ifr.ifr_name,sizeof(ifr.ifr_name),"%s", cfg->tap?"tap%d":"tun%d");
    if(cfg->tap) {
        ifr.ifr_flags = IFF_TAP;
    } else {
        ifr.ifr_flags = IFF_TUN;
    }
    rc = ioctl(fd, TUNSETIFF, &ifr);
    BDBG_MSG(("0 %s", ifr.ifr_name));
    if(rc!=0) {
        perror(dev);
        close(fd);
        return -1;
    }
    BKNI_Snprintf(tun,sizeof(tun),"%s", ifr.ifr_name);
    BDBG_MSG(("Using %s",tun));

    ifd = socket(AF_INET, SOCK_DGRAM, 0);
    if(ifd<0) {
        perror("AF_INET");
        return -1;
    }
    BKNI_Memset(&ifr, 0, sizeof(ifr));
    BKNI_Snprintf(ifr.ifr_name,sizeof(ifr.ifr_name),"%s", tun);
    ifd = socket(AF_INET, SOCK_DGRAM, 0);
    rc = ioctl(ifd, SIOCGIFFLAGS, &ifr);
    if(rc<0) {
        perror("SIOCGIFFLAGS");
        return -1;
    }
    ifr.ifr_flags |= IFF_UP | IFF_RUNNING | IFF_PROMISC;
    rc = ioctl(ifd, SIOCSIFFLAGS, &ifr);
    if(rc<0) {
        perror("SIOCSIFFLAGS");
        return -1;
    }
    close(ifd);
    pcap_read32(fin, &data32);
    if(data32!=0xa1b2c3d4) { /* magic_number */
        return -1;
    }
    pcap_read16(fin, &data16);/* version_major */
    pcap_read16(fin, &data16);/* version_minor */
    pcap_read32(fin, &data32);/* thiszone */
    pcap_read32(fin, &data32);/* sigfigs */
    pcap_read32(fin, &data32);/* snaplen */
    pcap_read32(fin, &data32);/* network */
    for(;;) {
        uint32_t ts_sec,ts_usec;
        unsigned len;
        uint8_t data[4096];
        int rc;
        unsigned offset;
        unsigned protocol=0;

        BSTD_UNUSED(ts_sec);
        BSTD_UNUSED(ts_usec);

        pcap_read32(fin, &data32); ts_sec = data32;
        pcap_read32(fin, &data32); ts_usec = data32;

        pcap_read32(fin, &data32); len = data32; /* incl_len */
        pcap_read32(fin, &data32); /* orig_len */
        if(len<4) {
            BDBG_ERR(("Invalud packet len %u", len));
            break;
        }
        len -=4;

        rc = pcap_read32(fin, &data32); /* type */
        if(rc<0) {
            break;
        }
        B_MEDIA_SAVE_UINT16_BE(data+0,0); /* tun flags */
        offset = 2;
        if( data32==24 ) { /* BSD_AFNUM_INET6_BSD */
            protocol = 0x86DD; /* ETH_P_IPV6 */; /* tun protocol */
        } else if( data32==2 ) { /* BSD_AFNUM_INET */
            protocol = 0x0800; /* ETH_P_IP */  /* tun protocol */
        } else {
            BDBG_ERR(("Invalud pcap protocol %u", data32));
            break;
        }
        B_MEDIA_SAVE_UINT16_BE(data+offset,protocol);
        offset += 2;
        data[offset+0] = 0x01; data[offset+1] = 0x02; data[offset+2] = 0x03; data[offset+3] = 0x04; data[offset+4] = 0x05; /* DST MAC */
        offset += 6;
        data[offset+0] = 0x11; data[offset+1] = 0x12; data[offset+2] = 0x13; data[offset+3] = 0x14; data[offset+4] = 0x15; /* SRC MAC */
        offset += 6;
        B_MEDIA_SAVE_UINT16_BE(data+offset,protocol);
        offset += 2;
        rc = fread(data+offset, len, 1, fin);
        if(rc!=1) {
            break;
        }
        len += offset;
        rc = write(fd, data, len);
        BDBG_MSG(("write %u %d", rc, len));
        if(rc<0 || rc!=(int)len) {
            perror("write");
            break;
        }
        usleep(100*1000);
    }
    return 0;
}

#if WITH_NEXUS

static void dataready_callback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    return;
}

static void overflow_callback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    BDBG_ERR(("Recpump overflow!"));
    return;
}

static int platform_init(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_Error rc;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = true;
    platformSettings.transportModuleSettings.maxDataRate.parserBand[NEXUS_ParserBand_e0] = 50 * 1000 * 1000;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;

    return 0;
}

static int platform_setup(mmt_cfg *cfg, mmt_state *state)
{
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PlaypumpSettings playpumpSettings;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_PlaypumpOpenPidChannelSettings  playpumpPidCfg;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
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
    if((cfg->feed.live || cfg->transport_bitrate!=0) )  {
        stcSettings.mode = NEXUS_StcChannelMode_eHost;
    } else {
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    }
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
    NEXUS_AudioDecoder_Start(state->nexus.audioDecoder, &audioProgram);

    NEXUS_Playpump_Start(state->nexus.playpump);
    NEXUS_Playpump_GetStatus(state->nexus.playpump, &state->nexus.playpumpStatus);

    if(cfg->feed.live) {
        NEXUS_FrontendQamSettings qamSettings;
        NEXUS_FrontendAcquireSettings settings;
        NEXUS_ParserBandSettings parserBandSettings;
        NEXUS_FrontendUserParameters userParams;
        NEXUS_PidChannelSettings pidCfg;
        unsigned recordPid = 0;
        unsigned i;

        NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
        settings.capabilities.qam = true;
        state->nexus.feed.frontend = NEXUS_Frontend_Acquire(&settings);
        if (!state->nexus.feed.frontend) {
            BDBG_ERR(("Unable to find QAM-capable frontend"));
            return -1;
        }

        state->nexus.feed.parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
        BDBG_ASSERT(state->nexus.feed.parserBand);
        NEXUS_ParserBand_GetSettings(state->nexus.feed.parserBand, &parserBandSettings);
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        if(recordPid==0) {
            parserBandSettings.allPass=true;
            parserBandSettings.teiIgnoreEnabled = true;
            parserBandSettings.acceptAdapt00 = true;
            parserBandSettings.continuityCountEnabled = false;
        }
        NEXUS_Frontend_GetUserParameters(state->nexus.feed.frontend, &userParams);
        if (userParams.isMtsif) {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
            parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(state->nexus.feed.frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
        } else {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
            parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
        }
        NEXUS_ParserBand_SetSettings(state->nexus.feed.parserBand, &parserBandSettings);

        NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
        qamSettings.frequency = cfg->feed.frequency;
        qamSettings.mode = cfg->feed.mode;
        qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
        rc = NEXUS_Frontend_TuneQam(state->nexus.feed.frontend, &qamSettings);
        BDBG_WRN(("tuning %d MHz... mode = %d", qamSettings.frequency/1000000, qamSettings.mode));
        for(i=0;i<100;i++) {
            NEXUS_FrontendFastStatus status;
            rc = NEXUS_Frontend_GetFastStatus(state->nexus.feed.frontend, &status);
            if (rc == NEXUS_SUCCESS) {
                if(status.lockStatus == NEXUS_FrontendLockStatus_eLocked) {
                    break;
                } else if(status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal) {
                    BDBG_ERR(("No signal at the tuned frequency."));
                }
            } else if (rc == NEXUS_NOT_SUPPORTED) {
                NEXUS_FrontendQamStatus qamStatus;
                rc = NEXUS_Frontend_GetQamStatus(state->nexus.feed.frontend, &qamStatus);
                if (qamStatus.receiverLock) {
                    break;
                }
            } else {
                return -1;
            }
            BKNI_Sleep(100);
        }
        BDBG_WRN(("tuned %d MHz... mode = %d", qamSettings.frequency/1000000, qamSettings.mode));

        if(recordPid==0) {
            NEXUS_PidChannel_GetDefaultSettings(&pidCfg);
            rc = NEXUS_ParserBand_GetAllPassPidChannelIndex(state->nexus.feed.parserBand, &pidCfg.pidChannelIndex);
            BDBG_ASSERT(rc==NEXUS_SUCCESS);
            state->nexus.feed.allPassPidChannel = NEXUS_PidChannel_Open(state->nexus.feed.parserBand, 0 /* don't care */, &pidCfg) ;
        } else {
            state->nexus.feed.allPassPidChannel = NEXUS_PidChannel_Open(state->nexus.feed.parserBand, recordPid, NULL);
        }
    } else { /*  if(cfg->feed.live)  */
        /* start playpump -> recpump feed */

        state->nexus.feed.playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
        BDBG_ASSERT(state->nexus.feed.playpump);
        NEXUS_Playpump_GetSettings(state->nexus.feed.playpump, &playpumpSettings);
        playpumpSettings.transportType = NEXUS_TransportType_eTs;
        playpumpSettings.allPass = true;
        playpumpSettings.maxDataRate = 81 * 1000 * 1000;
        playpumpSettings.continuityCountEnabled = false;
        if(cfg->transport_bitrate==0) {
            playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_eNone;
        } else {
            playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_eBinary;
            playpumpSettings.timestamp.pacing = true;
            playpumpSettings.timestamp.resetPacing = true;
            playpumpSettings.timestamp.pacingMaxError = 52734;  /* 1 second */
        }
        NEXUS_Playpump_SetSettings(state->nexus.feed.playpump, &playpumpSettings);
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&playpumpPidCfg);
        NEXUS_Playpump_GetAllPassPidChannelIndex(state->nexus.feed.playpump, (unsigned *)&playpumpPidCfg.pidSettings.pidChannelIndex );
        state->nexus.feed.allPassPidChannel = NEXUS_Playpump_OpenPidChannel(state->nexus.feed.playpump,  0 /* don't care */, &playpumpPidCfg);
        NEXUS_Playpump_Start(state->nexus.feed.playpump);
        NEXUS_Playpump_GetStatus(state->nexus.feed.playpump, &state->nexus.feed.playpumpStatus);
        if(cfg->feed.remux) {
            NEXUS_ParserBandSettings parserBandSettings;
            NEXUS_PidChannelSettings pidCfg;
            NEXUS_RemuxSettings remuxSettings;

            NEXUS_Remux_GetDefaultSettings(&remuxSettings);
            remuxSettings.bypass = true;
            remuxSettings.paused = false;
            remuxSettings.pcrCorrectionEnabled = false;
            remuxSettings.enablePcrJitterAdjust = false;
            remuxSettings.allPass = true;
            remuxSettings.outputClock = NEXUS_RemuxClock_e40_5Mhz;
            remuxSettings.insertNullPackets = false;
            state->nexus.feed.remux = NEXUS_Remux_Open(0, &remuxSettings);
            NEXUS_Remux_AddPidChannel(state->nexus.feed.remux, state->nexus.feed.allPassPidChannel);
            NEXUS_Remux_Start(state->nexus.feed.remux);

            state->nexus.feed.parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
            NEXUS_ParserBand_GetSettings(state->nexus.feed.parserBand, &parserBandSettings);
            parserBandSettings.teiIgnoreEnabled = true;
            parserBandSettings.acceptAdapt00 = true;
            parserBandSettings.continuityCountEnabled = false;
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eRemux;
            parserBandSettings.sourceTypeSettings.remux = state->nexus.feed.remux;
            parserBandSettings.transportType = NEXUS_TransportType_eTs;
            parserBandSettings.allPass=true;
            NEXUS_ParserBand_SetSettings(state->nexus.feed.parserBand, &parserBandSettings);
            NEXUS_PidChannel_GetDefaultSettings(&pidCfg);
            NEXUS_ParserBand_GetAllPassPidChannelIndex(state->nexus.feed.parserBand, &pidCfg.pidChannelIndex);
            state->nexus.feed.allPassPidChannel = NEXUS_PidChannel_Open(state->nexus.feed.parserBand, 0 /* don't care */, &pidCfg) ; /* overwrite state->nexus.feed.allPassPidChannel */
        }
    }

    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    recpumpOpenSettings.data.dataReadyThreshold  = (recpumpOpenSettings.data.bufferSize * 3)/4;
    recpumpOpenSettings.indexType = NEXUS_RecpumpIndexType_eNone;
    state->nexus.feed.recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
    NEXUS_Recpump_AddPidChannel(state->nexus.feed.recpump, state->nexus.feed.allPassPidChannel, NULL);

    NEXUS_Recpump_GetSettings(state->nexus.feed.recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = dataready_callback;
    recpumpSettings.data.overflow.callback = overflow_callback;
    if(cfg->transport_bitrate==0 && !cfg->feed.live) {
        recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
    } else {
        recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eDisable;
    }
    recpumpSettings.outputTransportType = NEXUS_TransportType_eBulk;
    rc = NEXUS_Recpump_SetSettings(state->nexus.feed.recpump, &recpumpSettings);
    BDBG_ASSERT(rc==NEXUS_SUCCESS);

    NEXUS_Recpump_Start(state->nexus.feed.recpump);

    state->nexus.cfg = cfg;
    return 0;
}

static void platform_shutdown(mmt_state *state)
{
    /* Bring down system */

    NEXUS_Recpump_Stop(state->nexus.feed.recpump);
    if(!state->nexus.cfg->feed.live) {
        NEXUS_Playpump_Stop(state->nexus.feed.playpump);
    }
    NEXUS_Recpump_Close(state->nexus.feed.recpump);
    if(state->nexus.cfg->feed.live) {
        NEXUS_ParserBand_Close(state->nexus.feed.parserBand);
        NEXUS_Frontend_Release(state->nexus.feed.frontend);
    } else {
        if(state->nexus.cfg->feed.remux) {
            NEXUS_Remux_Stop(state->nexus.feed.remux);
            NEXUS_ParserBand_Close(state->nexus.feed.parserBand);
            NEXUS_Remux_Close(state->nexus.feed.remux);
        }
        NEXUS_Playpump_Close(state->nexus.feed.playpump);
    }

    NEXUS_VideoDecoder_Stop(state->nexus.videoDecoder);
    NEXUS_AudioDecoder_Stop(state->nexus.audioDecoder);
    NEXUS_Playpump_Stop(state->nexus.playpump);
    NEXUS_Playpump_Close(state->nexus.playpump);
    NEXUS_VideoDecoder_Close(state->nexus.videoDecoder);
    NEXUS_AudioDecoder_Close(state->nexus.audioDecoder);
    NEXUS_VideoWindow_Close(state->nexus.window);
    NEXUS_Display_Close(state->nexus.display);
    NEXUS_StcChannel_Close(state->nexus.stcChannel);
    return;
}

static void platform_uninit(void)
{
    NEXUS_Platform_Uninit();
    return;
}
#else /* #if WITH_NEXUS */
static int platform_init(void)
{
    BKNI_Init();
    BDBG_Init();
    BDBG_SetLevel(BDBG_eWrn);
    return 0;
}
static int platform_setup(mmt_cfg *cfg, mmt_state *state)
{
    BSTD_UNUSED(cfg);
    BSTD_UNUSED(state);
    return 0;
}

static void platform_shutdown(mmt_state *state)
{
    BSTD_UNUSED(state);
    return ;
}

static void platform_uninit(void)
{
    BDBG_Uninit();
    BKNI_Uninit();
    return;
}
#endif
#if WITH_NEXUS
struct thread_state {
    const mmt_cfg *cfg;
    const mmt_state *state;
    FILE *fin;
    bool done;
};

static void *thread_feed(void *arg)
{
    struct thread_state *t = arg;
    FILE *fin = t->fin;
    const mmt_state *state = t->state;
    const mmt_cfg *cfg = t->cfg;
    uint64_t offset = 0;


    if(cfg->transport_bitrate) {
        BDBG_LOG(("Sending data at %uMbps", cfg->transport_bitrate / (1000*1000)));
    }
    for(;;) {
        unsigned dropThreshold = 16384;
        void *data;
        size_t size;
        NEXUS_Error rc;

        rc = NEXUS_Playpump_GetBuffer( state->nexus.feed.playpump, &data, &size);
        if(rc!=NEXUS_SUCCESS) {
            (void)BERR_TRACE(rc);
            break;
        }
        BDBG_MSG(("feed GetBuffer:%u %u", (unsigned)((uint8_t *)data- (uint8_t *)state->nexus.feed.playpumpStatus.bufferBase), (unsigned)size));
        if(size<dropThreshold) {
            if( (uint8_t *)state->nexus.feed.playpumpStatus.bufferBase + (state->nexus.feed.playpumpStatus.fifoSize - dropThreshold) <= (uint8_t *)data) {
                BDBG_MSG(("feed Drop"));
                NEXUS_Playpump_WriteComplete(state->nexus.feed.playpump, size , 0);
            } else {
                BDBG_MSG(("feed Wait"));
                BKNI_Sleep(10); /* wait 10 milliseconds */
            }
        } else {
            unsigned max_size = (cfg->transport_bitrate==0 ? BMPEG2TS_PKT_LEN  : (BMPEG2TS_PKT_LEN+4)) * 512;
            int bytes;
            if(size>max_size) {
                size = max_size;
            }

            if(cfg->transport_bitrate==0) {
                bytes = fread(data,1,size, fin);
            } else {
                for(bytes=0;;) {
                    uint32_t timestamp = ( (27*1000*1000)*8*offset)/ cfg->transport_bitrate;
                    uint8_t *buf = (uint8_t *)data + bytes;
                    int result;
                    if(bytes + BMPEG2TS_PKT_LEN+4 > (int)size) {
                        BDBG_MSG(("feed timestamp:%u offset:%u bytes:%u", (unsigned)timestamp, (unsigned)offset, (unsigned)bytes));
                        break;
                    }
                    B_MEDIA_SAVE_UINT32_BE(buf,timestamp);
                    result = fread(buf+4, BMPEG2TS_PKT_LEN, 1, fin);
                    if(result!=1) {
                        break;
                    }
                    offset += BMPEG2TS_PKT_LEN;
                    bytes += BMPEG2TS_PKT_LEN+4;
                }
            }
            if(bytes<=0) {
                break;
            }
            NEXUS_Playpump_WriteComplete(state->nexus.feed.playpump, 0, bytes);
        }
    }
    t->done = true;
    return NULL;
}

static int process_mmt_buffer(const mmt_cfg *cfg, mmt_state *state, btlv_parser *parser, btlv_ip_parser *ip_parser, void *buf, unsigned size, unsigned *precycled, unsigned *plast_io_data)
{
    int rc = 0;
    btlv_parser_packets packets;
    unsigned offset;
    unsigned recycled = *precycled;
    unsigned last_io_data = *plast_io_data;

    for(offset=0;offset+BMPEG2TS_PKT_LEN<=size;offset+=BMPEG2TS_PKT_LEN) {
        rc = btlv_parser_process(parser, (uint8_t *)buf+offset, &packets);
        if(rc==0) {
            unsigned i;
            if(packets.packet_valid) {
                rc = b_mmt_test_process_single_packet(cfg, state, ip_parser, &packets.packet);
            }
            for(i=0;i<packets.count;i++) {
                batom_cursor cursor;
                batom_cursor_from_vec(&cursor, packets.packets+i, 1);
                rc = b_mmt_test_process_single_packet(cfg, state, ip_parser, &cursor);
            }
            if(packets.keep_previous_packets ) {
                last_io_data++;
            } else {
                if(packets.keep_current_packet) { /* current packet to the first */
                    recycled += BMPEG2TS_PKT_LEN*last_io_data;
                    last_io_data = 1;
                } else {
                    recycled += BMPEG2TS_PKT_LEN*(last_io_data+1);
                    last_io_data = 0; /* reset buffer */
                }
            }
        } else if(rc==BTLV_RESULT_UNKNOWN_PID) {
            if(last_io_data==0) {
                recycled += BMPEG2TS_PKT_LEN;
            } else {
                last_io_data++;
            }
            continue;
        } else {
            btlv_parser_reset(parser);
            recycled += BMPEG2TS_PKT_LEN*(last_io_data+1);
            last_io_data = 0;
        }
    }
    *plast_io_data = last_io_data;
    *precycled = recycled;
    return rc;
}


static void live_feed(const mmt_cfg *cfg, mmt_state *state, FILE *fin)
{
    pthread_t tid;
    struct thread_state t;
    btlv_parser parser;
    btlv_ip_parser ip_parser;
    unsigned last_io_data=0;
    unsigned skip = 0;
    unsigned processed = 0;

    if(!cfg->feed.live) {
        t.fin = fin;
        t.state = state;
        t.cfg = cfg;
        t.done = false;
        pthread_create(&tid, NULL, thread_feed, &t);
    }

    btlv_parser_init(&parser, BTLV_DEFAULT_PID);
    btlv_ip_parser_init(&ip_parser);

    for(;;) {
        const void *data[2];
        size_t size[2];
        NEXUS_Error rc;

        rc = NEXUS_Recpump_GetDataBufferWithWrap(state->nexus.feed.recpump, &data[0], &size[0], &data[1],&size[1]);
        if(rc!=NEXUS_SUCCESS) {
            (void)BERR_TRACE(rc);
            break;
        }
        BDBG_MODULE_MSG(bmmt_test_mmt,("Recpump: %p,%u %p,%u %u", data[0], (unsigned)size[0], data[1], (unsigned)size[1], skip));
        if(size[0]+size[1]>skip) {
            unsigned i;
            unsigned recycled =0;
            unsigned sleepThreshold = 16384;
            bool sleep = size[0]+size[1]<(skip + sleepThreshold);
            for(i=0;i<2;i++) {
                if(size[i]==0) {
                    continue;
                }
                if( size[i] > skip ) {
                    void *buf = (uint8_t *)(data[i]) + skip;
                    unsigned length = size[i] - skip;
                    BDBG_MODULE_MSG(bmmt_test_mmt,("+[%u] %p:%u %u %u %u %u", i, buf, length, (unsigned)size[i], skip, recycled, last_io_data));
                    if(cfg->feed.capture) {
                        /*
                         * to find binary pattern
                         * grep -obUaP "\x47\x00\x2d\x6a\x54\x9f"
                         *
                         * to cut file from offset
                         * tail -c +26977296 | hexdump -C | head
                         *
                         * to print first 8 bytes of each TS packet
                         * perl -e 'my $d;while(read (STDIN,$d,188)) {print join(" ", map {sprintf("%02x", ord($_))} split(//,substr($d,0,8)));print "\n";}'
                         */
                        BDBG_ASSERT(state->fout);
                        fwrite(buf,length,1,state->fout);
                        fflush(state->fout);
                        recycled += length;
                    } else {
                        process_mmt_buffer(cfg, state, &parser, &ip_parser, buf, length, &recycled, &last_io_data);
                    }
                    BDBG_MODULE_MSG(bmmt_test_mmt,("-[%u] %p %u %u", i, buf+length, recycled, last_io_data));
                    skip = 0;
                } else {
                    skip -= size[i];
                }
            }
            BDBG_ASSERT(size[0] + size[1] >= recycled);
            skip = (size[0] + size[1]) - recycled;
            NEXUS_Recpump_DataReadComplete(state->nexus.feed.recpump, recycled);
            processed += recycled;
            if( (processed-recycled)/(16 * 1024 * 1024) != processed/(16 * 1024*1024)) {
                BDBG_LOG(("Processed %uMBytes",processed/(1024*1024)));
            }
            if(sleep) {
                BKNI_Sleep(10); /* wait 10 milliseconds */
            }
        } else {
            if(t.done) {
                for(;;) {
                    NEXUS_VideoDecoderStatus prev,new;
                    NEXUS_VideoDecoder_GetStatus(state->nexus.videoDecoder, &prev);
                    BKNI_Sleep(100);
                    NEXUS_VideoDecoder_GetStatus(state->nexus.videoDecoder, &new);
                    if(new.pts == prev.pts) {
                        break;
                    }
                }
                break;
            }
            BKNI_Sleep(10); /* wait 10 milliseconds */
        }
    }
    btlv_ip_parser_shutdown(&ip_parser);
    if(!cfg->feed.live) {
        pthread_join(tid, NULL);
    }
    return;
}
#else /* #if WITH_NEXUS */
static void live_feed(const mmt_cfg *cfg, mmt_state *state, FILE *fin)
{
    BSTD_UNUSED(cfg);
    BSTD_UNUSED(state);
    BSTD_UNUSED(fin);
    fprintf(stderr,"Not supported\n");
    return;
}
#endif

int
main(int argc, const char *argv[])
{
    FILE *fin=NULL;
    const char *file=NULL;
    const char *pes=NULL;
    int arg;
    int file_arg=0;
    batom_factory_stats factory_stats;
    char pes_name[1024];
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
    cfg.live_buffer = 1000/2; /* half a second */

    platform_init();

    if(0) {
        btlv_ipv6_address ipv6;

        bmmt_parse_ipv6_address("ff0e::1.3001", &ipv6);
        batom_range_dump(ipv6.addr, sizeof(ipv6.addr),"ff0e::1.3001");
        bmmt_parse_ipv6_address("ff0e:0:0:0:0:0:0:1.3001", &ipv6);
        batom_range_dump(ipv6.addr, sizeof(ipv6.addr),"ff0e:0:0:0:0:0:0:1.3001");
        bmmt_parse_ipv6_address("2001::34", &ipv6);
        batom_range_dump(ipv6.addr, sizeof(ipv6.addr),"2001::34");
        if(argc>1) {
            bmmt_parse_ipv6_address(argv[1], &ipv6);
            batom_range_dump(ipv6.addr, sizeof(ipv6.addr),argv[1]);
        }
        exit(0);
    }

    arg=1;
    while (argc>arg) {
        if (!strcmp("-help",argv[arg])) {
            usage(argv[0], NULL);
            goto done;
        } else if (!strcmp("-msg",argv[arg]) && argc>arg+1) {
            arg++;
            BDBG_SetModuleLevel(argv[arg], BDBG_eMsg);
        } else if (!strcmp("-pcap",argv[arg])) {
            cfg.pcap = true;
        } else if (!strcmp("-tlv",argv[arg])) {
            cfg.tlv = true;
        } else if (!strcmp("-tun",argv[arg])) {
            cfg.tun = true;
        } else if (!strcmp("-tap",argv[arg])) {
            cfg.tap = true;
        } else if (!strcmp("-video_ip",argv[arg]) && argc>arg+1) {
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
#if WITH_NEXUS
        } else if (!strcmp("-live",argv[arg])) {
            cfg.live = true;
        } else if (!strcmp("-live_buffer",argv[arg]) && argc>arg+1) {
            arg++;
            cfg.live_buffer = strtol(argv[arg],NULL,0);
        } else if (!strcmp("-transport_bitrate",argv[arg]) && argc>arg+1) {
            arg++;
            cfg.transport_bitrate = strtol(argv[arg],NULL,0);
        } else if (!strcmp("-capture",argv[arg])) {
            cfg.feed.capture = true;
        } else if (!strcmp("-display_format",argv[arg]) && argc>arg+1) {
            arg++;
            cfg.displayFormat = strtol(argv[arg],NULL,0);
        } else if (!strcmp("-remux",argv[arg])) {
            cfg.feed.remux = true;
#endif
        } else if (*argv[arg]!='\0' && (*argv[arg]!='-' || argv[arg][1]=='\0'))  {
            switch(file_arg) {
            case 0:
                 file = argv[arg];
#if WITH_NEXUS
                 BSTD_UNUSED(pes_name);
#else
                {
                    char *dot;
                    strcpy(pes_name, file);
                    dot = strrchr(pes_name, '.');
                    if (dot) {
                        strcpy(dot+1,"pes");
                        pes = pes_name;
                    }
                }
#endif
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
    if (!file
#if !WITH_NEXUS
        || !pes
#endif
        ) {
        usage(argv[0], NULL);
        goto done;
    }
#if WITH_NEXUS
    {
        unsigned freq;
        unsigned mode;
        int rc = sscanf(file,"qam%u:%u", &mode, &freq);
        if(rc==2) {
            cfg.feed.live = true;
            file = NULL;
            cfg.feed.frequency = freq * 1000000;
            switch (mode) {
            default:
                BDBG_ERR(("Incorrect mode %d specified. Defaulting to 64(NEXUS_FrontendQamMode_e64)", mode));
            case 64:
                cfg.feed.mode = NEXUS_FrontendQamMode_e64;
                break;
            case 256:
                cfg.feed.mode = NEXUS_FrontendQamMode_e256;
                break;
            }
        }
    }
#endif
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
    if(cfg.tun || cfg.tap) {
        inject_tun(&cfg, fin);
        goto close_fin;
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
    if(0 && cfg.tlv) {
        for(;;) {
            int rc;
            uint8_t buf[BMPEG2TS_PKT_LEN];
            btlv_segments segments;
            rc = fread(buf,sizeof(buf),1,fin);
            if(rc!=1) {
                break;
            }
            rc = btlv_parser_split(BTLV_DEFAULT_PID, buf, &segments);
            if(rc==0) {
                bool debug=true;
                unsigned i;
                if(segments.head.size) {
                    if(debug) batom_range_dump(buf+segments.head.offset,segments.head.size,"head");
                    fwrite(buf+segments.head.offset,segments.head.size,1,state.fout);
                }
                for(i=0;i<segments.count;i++) {
                    if(debug) batom_range_dump(buf+segments.packets[i].offset,segments.packets[i].size,"packet");
                    fwrite(buf+segments.packets[i].offset,segments.packets[i].size,1,state.fout);
                }
                if(segments.tail.size) {
                    if(debug) batom_range_dump(buf+segments.tail.offset,segments.tail.size,"tail");
                    fwrite(buf+segments.tail.offset,segments.tail.size,1,state.fout);
                }
            }
        }
    } else if(cfg.live) {
        live_feed(&cfg,&state, fin);
    } else if(cfg.tlv || cfg.pcap || cfg.video.set || cfg.audio.set) {
        btlv_parser parser;
        btlv_ip_parser ip_parser;
        struct {
            void *ts_buf;
        } io_data[360];
        unsigned last_io_data;
        BKNI_Memset(io_data, 0, sizeof(io_data));
        btlv_parser_init(&parser, BTLV_DEFAULT_PID);
        btlv_ip_parser_init(&ip_parser);
        if(cfg.pcap) {
            pcap_write_header(state.fout, 4096, 0 /* http://www.tcpdump.org/linktypes.html   LINKTYPE_NULL  */);
        }
        for(last_io_data=0;;) {
            int rc;
            void *buf;
            btlv_parser_packets packets;
            if(last_io_data>=sizeof(io_data)/sizeof(io_data[0])) {
                BDBG_ASSERT(0);
            }
            buf  = io_data[last_io_data].ts_buf;
            if(buf==NULL) {
                buf = BKNI_Malloc(BMPEG2TS_PKT_LEN);
                BDBG_ASSERT(buf);
                io_data[last_io_data].ts_buf = buf;
            }
            rc = fread(buf,BMPEG2TS_PKT_LEN,1,fin);
            if(rc!=1) {
                break;
            }
            rc = btlv_parser_process(&parser, buf, &packets);
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

    bmmt_demux_destroy(state.demux);
    bmmt_timestamp_queue_destroy(state.video.timestamp_queue);
    bmmt_timestamp_queue_destroy(state.audio.timestamp_queue);

    if(state.fout && state.fout!=stdout) {
        fclose(state.fout);
    }
close_fin:
    if(fin!=NULL && fin!=stdin) {
        fclose(fin);
    }
    batom_factory_get_stats(state.factory, &factory_stats);
    BDBG_WRN(("status: atoms[live:%u allocated:%u freed:%u] alloc[pool:%u/%u arena:%u/%u alloc:%u/%u]", factory_stats.atom_live, factory_stats.atom_allocated, factory_stats.atom_freed, factory_stats.alloc_pool, factory_stats.free_pool, factory_stats.alloc_arena, factory_stats.free_arena, factory_stats.alloc_alloc, factory_stats.free_alloc));

    batom_factory_dump(state.factory);
    batom_factory_destroy(state.factory);

    platform_shutdown(&state);
done:
    platform_uninit();

    return 0;
}
