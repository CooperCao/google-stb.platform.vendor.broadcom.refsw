/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
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
 * MPEG-2 TS Parser/Demux library
 *
 *******************************************************************************/
#include "bstd.h"
#include "bmpeg2ts_psi_probe.h"
#include "bmpeg2ts_parser.h"
#include "bmpeg2psi_parser.h"
#include "bmpeg2ts_probe.h"
#include "bkni.h"
#include "ts_psi.h"
#include "ts_pat.h"
#include "ts_pmt.h"
#include "bmedia_util.h"
#include "biobits.h"

#define BMPEG2TS_PSI_PAT_PID    0x00

BDBG_MODULE(bmpeg2ts_psi_probe);
#define BDBG_MSG_TRACE(x) /* BDBG_MSG(x) */

BDBG_OBJECT_ID(bmpeg2ts_psi_probe_t);

typedef struct bmpeg2ts_psi_probe *bmpeg2ts_psi_probe_t;

typedef struct b_mpeg2ts_psi_pid {
    bmpeg2ts_parser_pid ts; /* must be first */
    bool fired; /* PSI payload is  used only once */
    uint8_t count;
    uint16_t program_number;
    BLST_S_HEAD(b_mpeg2ts_psi_pid_group, b_mpeg2ts_psi_pid) group;
    BLST_S_ENTRY(b_mpeg2ts_psi_pid) link;
    bmpeg2ts_psi_probe_t probe;
    bmpeg2psi_parser psi;
} b_mpeg2ts_psi_pid;

typedef struct b_mpeg2ts_pcr_pid {
    bmpeg2ts_parser_pid ts; /* must be first */
    bmpeg2ts_psi_probe_t probe;
    struct {
        uint32_t pcr;
        off_t off;
    } first,last;
    BLST_S_ENTRY(b_mpeg2ts_pcr_pid) link;
} b_mpeg2ts_pcr_pid;

struct bmpeg2ts_psi_probe {
    BDBG_OBJECT(bmpeg2ts_psi_probe_t)
    bmpeg2ts_probe_stream *stream;
    batom_factory_t factory;
    bmpeg2ts_parser_t parser;
    b_mpeg2ts_psi_pid pat_pid;
    bool done;
    uint16_t n_pat_payloads;
    b_mpeg2ts_psi_pid *pmt_pids;
    unsigned n_pmt_pids;
    unsigned done_pmt_pids;
    BLST_S_HEAD(b_mpeg2ts_pcr_pids, b_mpeg2ts_pcr_pid) pcr_pids;
    uint8_t section_buf[1021]; /* section_length to not exceed 1021 bytes */
    unsigned pkt_len;
};


static bmpeg2ts_parser_action
b_mpeg2ts_psi_probe_ts_payload(bmpeg2ts_parser_pid *pid, unsigned flags, batom_accum_t src, batom_cursor *cursor, size_t len)
{
    b_mpeg2ts_psi_pid *psi_pid= (b_mpeg2ts_psi_pid *)pid;
    bmpeg2ts_parser_action action = bmpeg2ts_parser_action_consume;
    BDBG_MSG(("b_mpeg2ts_psi_probe_ts_payload:%p pid:%#x %u bytes", (void *)psi_pid->probe, psi_pid->ts.pid, (unsigned)len));
    for(psi_pid=BLST_S_FIRST(&psi_pid->group);psi_pid;psi_pid=BLST_S_NEXT(psi_pid, link)) {
        batom_cursor data;
        BATOM_CLONE(&data, cursor);
        action = bmpeg2psi_parser_feed(&psi_pid->psi, flags, src, &data, len);
    }
    batom_cursor_skip(cursor, len);
    return action;
}

static bmpeg2ts_parser_action
b_mpeg2ts_psi_probe_pcr_payload(bmpeg2ts_parser_pid *pid, unsigned flags, batom_accum_t src, batom_cursor *cursor, size_t len)
{
    b_mpeg2ts_pcr_pid *pcr_pid= (b_mpeg2ts_pcr_pid *)pid;
    BDBG_MSG(("b_mpeg2ts_psi_probe_ts_pcr_payload:%p pid:%#x %u bytes", (void *)pcr_pid->probe, pcr_pid->ts.pid, (unsigned)len));
    BSTD_UNUSED(src);
    BSTD_UNUSED(cursor);
    BSTD_UNUSED(len);
    if(flags&BMPEG2TS_PCR_FLAG) {
        bmpeg2ts_parser_status status;
        bmpeg2ts_parser_get_status(pcr_pid->probe->parser, &status);
        pcr_pid->last.pcr = pid->pcr_base;
        pcr_pid->last.off = status.offset;
        if((pcr_pid->first.pcr==0 && pcr_pid->first.off==0) || pcr_pid->first.pcr > pid->pcr_base) {
            pcr_pid->first.pcr = pid->pcr_base;
            pcr_pid->first.off = status.offset;
        }
    }
    return bmpeg2ts_parser_action_skip;
}

static int
b_mpeg2ts_psi_probe_init_pid(bmpeg2ts_psi_probe_t probe, b_mpeg2ts_psi_pid *psi_pid, uint16_t pid, void (*section)(void *section_cnxt, batom_t atom))
{
    int rc;

    bmpeg2ts_parser_pid_init(&psi_pid->ts, pid);
    rc = bmpeg2psi_parser_init(probe->factory, &psi_pid->psi);
    if(rc<0) {
        goto done;
    }
    psi_pid->probe = probe;
    psi_pid->ts.payload = b_mpeg2ts_psi_probe_ts_payload;
    psi_pid->psi.section_cnxt = psi_pid;
    psi_pid->psi.section = section;
    psi_pid->fired = false;
    psi_pid->count = 0;
    BLST_S_INIT(&psi_pid->group);
    BLST_S_INSERT_HEAD(&psi_pid->group, psi_pid, link);
    BDBG_MSG_TRACE(("b_mpeg2ts_psi_probe_init_pid: %#lx %u", (unsigned long)probe, (unsigned)pid));
    bmpeg2ts_parser_add_pid(probe->parser, &psi_pid->ts);
done:
    return rc;
}

static int
b_mpeg2ts_psi_probe_add_psi(bmpeg2ts_psi_probe_t probe, b_mpeg2ts_psi_pid *psi_pid, b_mpeg2ts_psi_pid *master_psi_pid, void (*section)(void *section_cnxt, batom_t atom))
{
    int rc;

    bmpeg2ts_parser_pid_init(&psi_pid->ts, master_psi_pid->ts.pid);
    rc = bmpeg2psi_parser_init(probe->factory, &psi_pid->psi);
    if(rc<0) {
        goto done;
    }
    psi_pid->probe = probe;
    psi_pid->ts.payload = b_mpeg2ts_psi_probe_ts_payload;
    psi_pid->psi.section_cnxt = psi_pid;
    psi_pid->psi.section = section;
    psi_pid->fired = false;
    psi_pid->count = 0;
    BLST_S_INIT(&psi_pid->group);
    BLST_S_INSERT_HEAD(&master_psi_pid->group, psi_pid, link);
done:
    return rc;
}

static size_t
b_mpeg2ts_psi_copy_section(bmpeg2ts_psi_probe_t probe, batom_t atom)
{
    batom_cursor cursor;
    batom_cursor_from_atom(&cursor, atom);
    return batom_cursor_copy(&cursor,probe->section_buf, sizeof(probe->section_buf));
}

#define B_FORMAT_IDENTIFIER_HDMV (0x48444d56 /* HDMV */)

static bool
b_mpeg2ts_psi_check_format_identifier(bmpeg2ts_psi_probe_t probe, size_t section_length, uint32_t format_identifier)
{
    unsigned reg_desc_idx=0;
    TS_PSI_descriptor reg_desc;
    for (reg_desc_idx=0; NULL!=(reg_desc = TS_PMT_getDescriptor(probe->section_buf, section_length, reg_desc_idx));reg_desc_idx++) {
        switch(reg_desc[0]) {
        case TS_PSI_DT_Registration:
            /* calculate and check format_identifier */
            {
                uint32_t identifier = (reg_desc[2] << 24) + (reg_desc[3] << 16) + (reg_desc[4] << 8) + reg_desc[5];
                if (identifier == format_identifier) {
                    return true;
                }
            }
            break;
        default:
            break;
        }
    }
    return false;
}

static void
b_mpeg2ts_psi_parse_pmt(bmpeg2ts_psi_probe_t probe, size_t section_length, unsigned pmt_pid)
{
    int i;
    TS_PSI_header header;
    int nstreams;
    unsigned pcr_pid_no;
    b_mpeg2ts_pcr_pid *pcr_pid;

    BDBG_ASSERT(probe->stream);

    TS_PSI_getSectionHeader(probe->section_buf, &header );
    {
        /* save PCR pid */
        bmpeg2ts_psi_probe_track *pcr_track = BKNI_Malloc(sizeof(*pcr_track));
        if(!pcr_track) {
            BDBG_ERR(("b_mpeg2ts_psi_parse_pmt: %p can't allocate %u bytes", (void *)probe, (unsigned)sizeof(*pcr_track)));
            goto error;
        }
        BKNI_Memset(pcr_track, 0, sizeof(*pcr_track));
        bmedia_probe_track_init(&pcr_track->media);
        pcr_track->parsed_payload = 0;
        pcr_track->transport_type = bmpeg2ts_psi_transport_pcr;
        pcr_track->media.program = header.table_id_extension;
        pcr_pid_no =  TS_PMT_getPcrPid(probe->section_buf, section_length);
        pcr_track->media.number = pcr_pid_no;
        pcr_track->media.type = bmedia_track_type_pcr;
        pcr_track->descriptor_size = 0;
        pcr_track->descriptor = NULL;
        /* coverity[address_free] */
        bmedia_probe_add_track(&probe->stream->media, &pcr_track->media);
    }
    {
        bmpeg2ts_psi_probe_track *track = BKNI_Malloc(sizeof(*track));
        if(!track) {
            BDBG_ERR(("b_mpeg2ts_psi_p, unsiged pmt_pidarse_pmt: %p can't allocate %u bytes", (void *)probe, (unsigned)sizeof(*track)));
            goto error;
        }
        BKNI_Memset(track,0,sizeof(*track));
        bmedia_probe_track_init(&track->media);
        track->parsed_payload = 0;
        track->media.type = bmedia_track_type_other;
        track->media.number = pmt_pid /* pmt_stream.elementary_PID*/;
        track->media.program = header.table_id_extension;
        track->transport_type = bmpeg2ts_psi_transport_pmt;
        bmedia_probe_add_track(&probe->stream->media, &track->media);
    }

    for(pcr_pid=BLST_S_FIRST(&probe->pcr_pids);pcr_pid!=NULL;pcr_pid=BLST_S_NEXT(pcr_pid, link)) {
        if(pcr_pid->ts.pid == pcr_pid_no) {
            break;
        }
    }
    if(pcr_pid==NULL) {
        pcr_pid = BKNI_Malloc(sizeof(*pcr_pid));
        if(!pcr_pid) {
            goto error;
        }
        pcr_pid->first.pcr = 0;
        pcr_pid->first.off = 0;
        pcr_pid->last.pcr = 0;
        pcr_pid->last.off = 0;
        pcr_pid->probe = probe;
        bmpeg2ts_parser_pid_init(&pcr_pid->ts, pcr_pid_no);
        pcr_pid->ts.payload = b_mpeg2ts_psi_probe_pcr_payload;
        BLST_S_INSERT_HEAD(&probe->pcr_pids, pcr_pid, link);
        BDBG_MSG_TRACE(("b_mpeg2ts_psi_parse_pmt: %#lx add pcr_pid %u", (unsigned long)probe, (unsigned)track->number));
        bmpeg2ts_parser_add_pid(probe->parser, &pcr_pid->ts);
    }


    for(nstreams = TS_PMT_getNumStreams(probe->section_buf, section_length), i = 0; i < nstreams; i++ )
    {
        TS_PMT_stream pmt_stream;
        TS_PSI_descriptor desc = NULL;
        TS_PSI_descriptor transport_desc = NULL;
        baudio_format audio_codec = baudio_format_unknown;
        bvideo_codec video_codec = bvideo_codec_unknown;
        bmpeg2ts_psi_transport_type transport_type = bmpeg2ts_psi_transport_unknown;
        bmpeg2ts_psi_dvb_subtitle dvb_subtitle;
        bmpeg2ts_ac3_bsmod ac3_bsmod;
        bmpeg2ts_psi_caption_service  caption;
        int desc_idx;
        bmpeg2ts_psi_language language;
        unsigned audio_substreams_count = 0;
        struct {
            uint8_t id;
            baudio_format codec;
        } audio_substreams[2];

        BKNI_Memset(&audio_substreams, 0, sizeof(audio_substreams));
        BKNI_Memset(&dvb_subtitle, 0, sizeof(dvb_subtitle));
        BKNI_Memset(&language, 0, sizeof(language));
        BKNI_Memset(&ac3_bsmod, 0, sizeof(ac3_bsmod));
        BKNI_Memset(&caption, 0, sizeof(caption));
        dvb_subtitle.valid = false;
        language.valid = false;
        if (TS_PMT_getStream(probe->section_buf, section_length, i, &pmt_stream )!=BERR_SUCCESS) {
            BDBG_WRN(("b_mpeg2ts_psi_parse_pmt: %#lx invalid pmt_data program:%u stream:%u", (unsigned long)probe, header.table_id_extension, i));
            continue;
        }

        BDBG_MSG(("PMT: pid:%#x stream_type:%#x", pmt_stream.elementary_PID,pmt_stream.stream_type));
        switch( pmt_stream.stream_type )
        {
        /* video formats */
        case TS_PSI_ST_11172_2_Video:  /* MPEG-1 */
            video_codec = bvideo_codec_mpeg1;
            break;
        case TS_PSI_ST_13818_2_Video: /* MPEG-2 */
            video_codec = bvideo_codec_mpeg2;
            break;
        case TS_PSI_ST_14496_2_Video: /* MPEG-4 Part 2 */
            video_codec = bvideo_codec_mpeg4_part2;
            break;
        case TS_PSI_ST_14496_10_Video: /* H.264/AVC */
            video_codec = bvideo_codec_h264;
            break;
        case TS_PSI_ST_14496_10_AnnexG_Video: /* H.264/AVC */
            video_codec = bvideo_codec_h264_svc;
            break;
        case TS_PSI_ST_14496_10_AnnexH_Video: /* H.264/AVC */
            video_codec = bvideo_codec_h264_mvc;
            break;
        case TS_PSI_ST_AVS_Video: /* AVS */
            video_codec = bvideo_codec_avs;
            break;
        case TS_PSI_ST_23008_2_Video_brcm:
        case TS_PSI_ST_23008_2_Video:
            video_codec = bvideo_codec_h265;
            break;
        case TS_PSI_ST_SMPTE_VC1:      /* VC-1 */
            /* need to parse descriptor and then subdescriptor to determine profile */
            for (desc_idx=0; NULL!=(desc = TS_PMT_getStreamDescriptor(probe->section_buf, section_length, i, desc_idx));desc_idx++) {
                switch(desc[0]) {
                case TS_PSI_DT_Registration:
                    /* calculate and check format_identifier */
                    {
                        uint32_t format_identifier = (desc[2] << 24) + (desc[3] << 16) + (desc[4] << 8) + desc[5];
                        if (format_identifier == 0x56432D31) {
                            /* check that proper sub-descriptor exists */
                            int subdescriptor_tag = desc[6];
                            if (subdescriptor_tag == 0x01) {
                                int profile_level = desc[7];
                                if (profile_level >= 0x90) { /* Advanced Profile ES */
                                    video_codec = bvideo_codec_vc1;
                                } else { /* Simple/Main Profile ES */
                                    video_codec = bvideo_codec_vc1_sm;
                                }
                            }
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            break;

        /* audio formats */
        case TS_PSI_ST_11172_3_Audio: /* MPEG-1 */
            audio_codec = baudio_format_mpeg;
            break;
        case TS_PSI_ST_13818_3_Audio: /* MPEG-2 */
            audio_codec = baudio_format_mpeg;
            break;
        case TS_PSI_ST_13818_7_AAC:  /* MPEG-2 AAC */
            audio_codec = baudio_format_aac;
            break;
        case TS_PSI_ST_14496_3_Audio: /* MPEG-4 AAC */
            audio_codec = baudio_format_aac_plus;
            break;
        case TS_PSI_ST_ATSC_AC3:      /* ATSC or BD AC-3 */
            audio_codec = baudio_format_ac3;
            break;
        case TS_PSI_ST_BD_DTS:
            audio_codec = baudio_format_dts;
            break;
        case TS_PSI_ST_BD_EAC3:       /* BD E-AC3 */
            audio_codec = baudio_format_ac3_plus;
            audio_substreams_count = 2;
            audio_substreams[0].codec = baudio_format_ac3;
            audio_substreams[0].id = 0x71;
            audio_substreams[1].codec = baudio_format_ac3_plus;
            audio_substreams[1].id = 0x72;
            break;

        case TS_PSI_ST_BD_DTS_HD_MA:     /* This can be DTS-HD MA (BD). */
            if ( b_mpeg2ts_psi_check_format_identifier(probe,section_length,B_FORMAT_IDENTIFIER_HDMV) )
            {
                audio_codec = baudio_format_dts_hd;
            }
            break;
        case TS_PSI_ST_ATSC_DTS_HD:
            audio_codec = baudio_format_dts_hd;
            break;
        case TS_PSI_ST_IEEE1394_LPCM:   /* 0x83 is 1394-LPCM or AC3 in BD */
        /* case TS_PSI_ST_BD_TRUEHD: */
            if ( b_mpeg2ts_psi_check_format_identifier(probe,section_length,B_FORMAT_IDENTIFIER_HDMV) ) {
                audio_codec = baudio_format_mlp;
                audio_substreams_count = 2;
                audio_substreams[0].codec = baudio_format_ac3;
                audio_substreams[0].id = 0x76;
                audio_substreams[1].codec = baudio_format_mlp;
                audio_substreams[1].id = 0x72;
            } else {
                audio_codec = baudio_format_lpcm_1394;
            }
            break;
        case TS_PSI_ST_ATSC_DTS_HiRes:          /* Technically DTS-HD HRA in BD */
            audio_codec = baudio_format_dts_hd;
            break;
        case TS_PSI_ST_ATSC_EAC3:     /* ATSC Enhanced AC-3 */
            audio_codec = baudio_format_ac3_plus;
            break;
        case TS_PSI_ST_AVS_Audio:     /* AVS */
            audio_codec = baudio_format_avs;
            break;
        case TS_PSI_ST_DRA_Audio:
            audio_codec = baudio_format_dra;
            break;

        /* video or audio */
        case TS_PSI_ST_ATSC_Video:   /* 0x80 (user-defined) is ATSC MPEG-2, Digicypher II video or BD-LPCM audio */
            if ( b_mpeg2ts_psi_check_format_identifier(probe,section_length,B_FORMAT_IDENTIFIER_HDMV) )
            {
                audio_codec = baudio_format_lpcm_bluray;
            }
            else
            {
                video_codec = bvideo_codec_mpeg2;
            }
            break;
        case TS_PSI_ST_13818_1_PrivatePES:  /* examine descriptors to handle private data */
            for (desc_idx=0; NULL!=(desc = TS_PMT_getStreamDescriptor(probe->section_buf, section_length, i, desc_idx));desc_idx++) {
                BDBG_MSG(("PMT: pid:%#x private_pes desc[0]:%#x", pmt_stream.elementary_PID,desc[0]));
                switch(desc[0]) {
                /* video formats */
                case TS_PSI_DT_VideoStream:
                    /* MPEG_1_only_flag is bit 2 of desc[2], this determines MPEG-1/2 */
                    if (desc[2] & 0x04) {
                        video_codec = bvideo_codec_mpeg1;
                    } else {
                        video_codec = bvideo_codec_mpeg2;
                    }
                    break;
                case TS_PSI_DT_MPEG4_Video:
                    video_codec = bvideo_codec_mpeg4_part2;
                    break;
                case TS_PSI_DT_AVC:
                    video_codec = bvideo_codec_h264;
                    break;
                case TS_PSI_DT_AVS_Video:
                    video_codec = bvideo_codec_avs;
                    break;
                case TS_PSI_DT_SVC:
                    video_codec = bvideo_codec_h264_svc;
                    break;
                case TS_PSI_DT_MVC:
                    video_codec = bvideo_codec_h264_mvc;
                    break;
                /* audio formats */
                case TS_PSI_DT_AudioStream:/* Same baudio_format for MPEG-1 or MPEG-2 audio */
                    audio_codec = baudio_format_mpeg;
                    break;
                case TS_PSI_DT_MPEG2_AAC:/* Note baudio_format_aac = MPEG-2 AAC */
                    audio_codec = baudio_format_aac;
                    break;
                case TS_PSI_DT_MPEG4_Audio:/* Note baudio_format_aac_plus = MPEG-4 AAC */
                    audio_codec = baudio_format_aac_plus;
                    break;
                case TS_PSI_DT_DVB_AAC:/* Note baudio_format_aac_plus = MPEG-4 AAC */
                    audio_codec = baudio_format_aac_plus;
                    break;
                case TS_PSI_DT_DVB_AC3:
                    audio_codec = baudio_format_ac3;
                    break;
                case TS_PSI_DT_DVB_EnhancedAC3:
                    audio_codec = baudio_format_ac3_plus;
                    break;
                case TS_PSI_DT_DVB_DTS:
                    audio_codec = baudio_format_dts;
                    break;
                case TS_PSI_DT_DVB_DRA:
                    audio_codec = baudio_format_dra;
                    break;
                case TS_PSI_DT_DVB_Teletext:
                    transport_type = bmpeg2ts_psi_transport_teletext;
                    transport_desc = desc;
                    break;
                case TS_PSI_DT_DVB_Subtitling:
                    transport_type = bmpeg2ts_psi_transport_dvb_subtitles;
                    transport_desc = desc;
                    BKNI_Memcpy(dvb_subtitle.languageCode, desc+2, sizeof(dvb_subtitle.languageCode)-1);
                    dvb_subtitle.valid = true;
                    dvb_subtitle.type =  desc[5];
                    dvb_subtitle.compositionPageId = B_MEDIA_LOAD_UINT16_BE(desc,6);
                    dvb_subtitle.ancillaryPageId = B_MEDIA_LOAD_UINT16_BE(desc,8);
                    BDBG_MSG(("subtitle language code: %s type:%d composition page:%u ancillary page:%u", dvb_subtitle.languageCode, dvb_subtitle.type, dvb_subtitle.compositionPageId, dvb_subtitle.ancillaryPageId));
                    break;
                case TS_PSI_DT_DVB_Extension:
                    /* DVB Document A038
                     * 6.2.16 Extension descriptor */
                    BDBG_MSG(("PMT: pid:%#x private_pes desc[0]:%#x extension:%#x", pmt_stream.elementary_PID,desc[0], desc[2]));
                    switch(desc[2]) {
                    case TS_PSI_DT_DVB_Extension_AC4:
                        /* DVB Document A038
                         * D.7 AC-4 descriptor syntax and semantics */
                        audio_codec = baudio_format_ac4;
                        break;
                    }
                    break;
                default:
                    BDBG_MSG(("private descriptor %x",desc[0]));
                    break;
                }
            }
            break;
        default:
            break;
        }

        /* Add check for Language descriptor */
        for (desc_idx=0; NULL!=(desc = TS_PMT_getStreamDescriptor(probe->section_buf, section_length, i, desc_idx));desc_idx++) {
            switch(desc[0]) {
            case TS_PSI_DT_ISO_639_Language:
                transport_desc = desc;
                language.type = desc[5]; /* table 2.53, iso13818-1 */
                BKNI_Memcpy(language.code, desc+2, sizeof(language.code)-1);
                language.code[sizeof(language.code)-1]='\0';
                language.valid = true;
                BDBG_MSG(("audio language code:%s, type:%u", language.code, language.type));
                break;
            case TS_PSI_DT_ATSC_AC3_Audio:
                ac3_bsmod.valid = true;
                ac3_bsmod.bsmod = B_GET_BITS(desc[4], 7, 5); /* ATSC A/52:2012 Table A4.1 AC-3 Audio Descriptor Syntax */
                break;
            case TS_PSI_DT_ATSC_CaptionService: /* A-65 Program and System Information Protocol for Terrestrial Broadcast and Cable 6.9.2 Caption Service Descriptor */
                {
                    unsigned i;
                    transport_type = bmpeg2ts_psi_transport_caption_service;

                    caption.valid = true;
                    caption.number_of_services = B_GET_BITS(desc[2],4,0);

                    if(caption.number_of_services > sizeof(caption.services)/sizeof(caption.services[0])) {
                        caption.number_of_services = sizeof(caption.services)/sizeof(caption.services[0]);
                    }

                    for (i= 0; i< caption.number_of_services; i++) {
                        BKNI_Memcpy(caption.services[i].language, &desc[3+i*6], 3);
                        caption.services[i].language[3] = '\0';
                        caption.services[i].digital_cc = B_GET_BIT(desc[3+i*6+3],7);
                        if(caption.services[i].digital_cc) {
                            caption.services[i].cc.line21_field = B_GET_BIT(desc[3+i*6+3],0);
                        } else {
                            caption.services[i].cc.caption_service_number = B_GET_BITS(desc[3+i*6+3],5,0);
                        }
                        caption.services[i].easy_reader = B_GET_BIT(desc[3+i*6+4],7);
                        caption.services[i].wide_aspect_ratio = B_GET_BIT(desc[3+i*6+4],6);
                    }
                }
                break;
            default:
                break;
            }
        }

        if (video_codec!=bvideo_codec_unknown) {
            for (desc_idx=0; NULL!=(desc = TS_PMT_getDescriptor(probe->section_buf, section_length, desc_idx));desc_idx++) {
                switch(desc[0]) {
                    case TS_PSI_DT_ATSC_CA:
                        transport_desc = desc;
                        break;
                    default:
                        break;
                }
            }
        }
        {
            /* save other pids */
            bmpeg2ts_psi_probe_track *track = BKNI_Malloc(sizeof(*track) + (transport_desc ? transport_desc[1] : 0));
            if(!track) {
                BDBG_ERR(("b_mpeg2ts_psi_parse_pmt: %p can't allocate %u bytes", (void *)probe, (unsigned)sizeof(*track)));
                goto error;
            }
            bmedia_probe_track_init(&track->media);
            track->parsed_payload = 0;
            track->dvb_subtitle = dvb_subtitle;
            track->ac3_bsmod = ac3_bsmod;
            track->caption_service = caption;
            track->language = language;
            track->media.program = header.table_id_extension;
            track->media.number = pmt_stream.elementary_PID;
            track->transport_type = transport_type;

            if (transport_desc) {
                uint8_t *extra = (uint8_t *)track + sizeof(*track);
                track->descriptor_size = transport_desc[1];
                track->descriptor = extra;
                BKNI_Memcpy(extra, &transport_desc[2], track->descriptor_size);
            }
            else {
                track->descriptor_size = 0;
                track->descriptor = NULL;
            }

            if(video_codec!=bvideo_codec_unknown) {
                track->media.type = bmedia_track_type_video;
                track->media.info.video.codec = video_codec;
            } else if (audio_codec!=baudio_format_unknown) {
                track->media.type = bmedia_track_type_audio;
                track->media.info.audio.codec = audio_codec;
                if(audio_substreams_count==2) {
                    bmpeg2ts_psi_probe_track *track2 = BKNI_Malloc(sizeof(*track2) + (transport_desc ? transport_desc[1] : 0));
                    if(track2) {
                        bmedia_probe_track_init(&track2->media);
                        *track2 = *track;
                        track2->media.number = B_MEDIA_MPEG2TS_TRACKNO_MAKE(pmt_stream.elementary_PID, audio_substreams[1].id);
                        track2->media.info.audio.codec = audio_substreams[1].codec;
                        bmedia_probe_add_track(&probe->stream->media, &track2->media);
                    } else {
                        BDBG_ERR(("b_mpeg2ts_psi_parse_pmt: %p can't allocate %u bytes", (void *)probe, (unsigned)sizeof(*track)));
                    }
                    track->media.info.audio.codec = audio_substreams[0].codec;
                    track->media.number = B_MEDIA_MPEG2TS_TRACKNO_MAKE(pmt_stream.elementary_PID, audio_substreams[0].id);
                }
            } else {
                track->media.type = bmedia_track_type_other;
            }
            BDBG_MSG(("b_mpeg2ts_psi_parse_pmt: %#lx adding track %u:%u %u(%u)", (unsigned long)track, track->media.program, track->media.number, track->media.type, pmt_stream.stream_type ));
            /* coverity[address_free] */
            bmedia_probe_add_track(&probe->stream->media, &track->media);
        }
    }

    return;
error:
    probe->done = true;
    return;
}


static void
b_mpeg2ts_psi_probe_pmt(void *section_cnxt, batom_t atom)
{
    b_mpeg2ts_psi_pid *psi_pid = section_cnxt;
    bmpeg2ts_psi_probe_t probe = psi_pid->probe;
    BDBG_OBJECT_ASSERT(probe, bmpeg2ts_psi_probe_t);
    psi_pid->count = (psi_pid->count+1)%256;
    if(!psi_pid->fired) {
        size_t section_size = b_mpeg2ts_psi_copy_section(probe, atom);
        BDBG_MSG(("b_mpeg2ts_psi_probe_pmt: %p  section_size:%u pid:%#x program_number:%u (%u:%u)", (void *)probe, (unsigned)section_size, psi_pid->ts.pid, psi_pid->program_number, probe->n_pmt_pids, probe->done_pmt_pids ));
        if(TS_PMT_validate(probe->section_buf, section_size)) {
            psi_pid->fired = true;
            BDBG_ASSERT(probe->n_pmt_pids>probe->done_pmt_pids);
            b_mpeg2ts_psi_parse_pmt(probe, section_size, psi_pid->ts.pid);
            probe->done_pmt_pids++;
            if(probe->done_pmt_pids==probe->n_pmt_pids) {
                BDBG_MSG(("b_mpeg2ts_psi_parse_pmt: %#lx captured all %u PMT", (unsigned long)probe, probe->n_pmt_pids));
                probe->done = true;
            }
        } else {
            batom_dump(atom, "invalid PMT");
        }
    }
    batom_release(atom);
    return;
}

static void
b_mpeg2ts_psi_probe_pat(void *section_cnxt, batom_t atom)
{
    b_mpeg2ts_psi_pid *psi_pid = section_cnxt;
    bmpeg2ts_psi_probe_t probe = psi_pid->probe;
    BDBG_OBJECT_ASSERT(probe, bmpeg2ts_psi_probe_t);
    probe->n_pat_payloads++;
    psi_pid->count = (psi_pid->count+1)%256;
    if(!psi_pid->fired) {
        size_t section_size = b_mpeg2ts_psi_copy_section(probe, atom);
        BDBG_MSG(("b_mpeg2ts_psi_probe_pat: %p  section_size:%u", (void *)probe, (unsigned)section_size));
        if(TS_PAT_validate(probe->section_buf, section_size)) {
            unsigned i;
            unsigned n_pmt_pids;
            unsigned nprograms;
            b_mpeg2ts_psi_pid *pmt_pids;
            psi_pid->fired = true;
            nprograms = TS_PAT_getNumPrograms(probe->section_buf);
            BDBG_MSG(("b_mpeg2ts_psi_probe_pat: %p  nprograms:%u", (void *)probe, nprograms));
            pmt_pids = BKNI_Malloc(sizeof(*pmt_pids)*nprograms);
            if(!pmt_pids) {
                BDBG_ERR(("b_mpeg2ts_psi_probe_pat: %p can't allocate %u bytes", (void *)probe, nprograms*(unsigned)sizeof(*pmt_pids)));
                goto error;
            }
            probe->stream = BKNI_Malloc(sizeof(*probe->stream));
            if(!probe->stream) {
                BDBG_ERR(("b_mpeg2ts_psi_probe_pat: %p can't allocate %u bytes", (void *)probe, (unsigned)sizeof(*probe->stream)));
                BKNI_Free(pmt_pids);
                goto error;
            }
            bmedia_probe_stream_init(&probe->stream->media, bstream_mpeg_type_ts);
            for(n_pmt_pids=i=0;i<nprograms;i++) {
                TS_PAT_program  program;
                BERR_Code rc;
                unsigned j;
                b_mpeg2ts_psi_pid *master_psi_pid;

                rc = TS_PAT_getProgram( probe->section_buf, section_size, i, &program);
                if(rc!=BERR_SUCCESS) {
                    break;
                }
                if(program.PID==0) {
                    BDBG_WRN(("b_mpeg2ts_psi_probe_pat: %#lx pid %u can't be PMT pid", (unsigned long)probe, program.PID));
                    continue;
                }
                for(master_psi_pid=NULL,j=0;j<n_pmt_pids;j++) {
                    if(program.PID==pmt_pids[j].ts.pid) {
                        master_psi_pid = &pmt_pids[j];
                        break;
                    }
                }
                if(master_psi_pid==NULL) {
                    if(b_mpeg2ts_psi_probe_init_pid(probe, &pmt_pids[n_pmt_pids], program.PID, b_mpeg2ts_psi_probe_pmt)<0) {
                        break;
                    }
                } else {
                    BDBG_MSG(("b_mpeg2ts_psi_probe_pmt: %#lx attach new PMT to the pid %u", (unsigned long)probe, master_psi_pid->ts.pid));
                    if(b_mpeg2ts_psi_probe_add_psi(probe, &pmt_pids[n_pmt_pids], master_psi_pid, b_mpeg2ts_psi_probe_pmt)<0) {
                        break;
                    }
                }
                pmt_pids[n_pmt_pids].psi.filter.mask[0] = 0x00; pmt_pids[n_pmt_pids].psi.filter.coef[0] = 0x02;
                pmt_pids[n_pmt_pids].psi.filter.mask[3] = 0x00; pmt_pids[n_pmt_pids].psi.filter.coef[3] = program.program_number >> 8;
                pmt_pids[n_pmt_pids].psi.filter.mask[4] = 0x00; pmt_pids[n_pmt_pids].psi.filter.coef[4] = (program.program_number)&0xFF;
                pmt_pids[n_pmt_pids].program_number = program.program_number;
                BDBG_MSG(("b_mpeg2ts_psi_probe_pat: %#lx %u program:%u pid:%#x ", (unsigned long)probe, n_pmt_pids, program.program_number, program.PID));
                n_pmt_pids++;
            }
            if(n_pmt_pids>0) {
                probe->n_pmt_pids = n_pmt_pids;
                probe->pmt_pids = pmt_pids;
            } else {
                BKNI_Free(pmt_pids);
                goto error;
            }
        } else {
            batom_dump(atom, "invalid PAT");
        }
    }
done:
    batom_release(atom);
    return;
error:
    probe->done = true;
    goto done;
}

static bmedia_probe_base_t
b_mpeg2ts_psi_probe_create_len(batom_factory_t factory, unsigned header_size)
{
    bmpeg2ts_psi_probe_t probe;
    bmpeg2ts_parser_cfg parser_cfg;

    probe = BKNI_Malloc(sizeof(*probe));
    if(!probe) {
        BDBG_ERR(("b_mpeg2ts_psi_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(probe, bmpeg2ts_psi_probe_t);
    BLST_S_INIT(&probe->pcr_pids);
    probe->stream = NULL;
    probe->pmt_pids = NULL;
    probe->factory = factory;
    probe->pkt_len = BMPEG2TS_PKT_LEN + header_size;
    bmpeg2ts_parser_default_cfg(&parser_cfg);

    parser_cfg.header_size = header_size;
    probe->parser = bmpeg2ts_parser_create(factory, &parser_cfg);
    if(!probe->parser) {
        goto err_parser;
    }
    if(b_mpeg2ts_psi_probe_init_pid(probe, &probe->pat_pid, BMPEG2TS_PSI_PAT_PID, b_mpeg2ts_psi_probe_pat)<0) {
        goto err_pat;
    }
    return (bmedia_probe_base_t)probe;

err_pat:
    bmpeg2ts_parser_destroy(probe->parser);
err_parser:
    BKNI_Free(probe);
err_alloc:
    return NULL;
}


static bmedia_probe_base_t
b_mpeg2ts_psi_probe_create(batom_factory_t factory)
{
    return b_mpeg2ts_psi_probe_create_len(factory, 0);
}

static bmedia_probe_base_t
b_mpeg2ts192_psi_probe_create(batom_factory_t factory)
{
    return b_mpeg2ts_psi_probe_create_len(factory, 4);
}

static void
b_mpeg2ts_psi_probe_destroy(bmedia_probe_base_t probe_)
{
    bmpeg2ts_psi_probe_t probe = (bmpeg2ts_psi_probe_t)probe_;

    BDBG_OBJECT_ASSERT(probe, bmpeg2ts_psi_probe_t);
    BDBG_ASSERT(probe->stream==NULL); /* can't destroy probe in middle of parsing */
    bmpeg2psi_parser_shutdown(&probe->pat_pid.psi);

    bmpeg2ts_parser_remove_pid(probe->parser, probe->pat_pid.ts.pid);
    bmpeg2ts_parser_destroy(probe->parser);
    BDBG_OBJECT_DESTROY(probe, bmpeg2ts_psi_probe_t);
    BKNI_Free(probe);
    return;
}

static const bmedia_probe_stream *
b_mpeg2ts_psi_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
    bmpeg2ts_psi_probe_t probe = (bmpeg2ts_psi_probe_t)probe_;
    bmpeg2ts_probe_stream *stream;
    unsigned i;
    off_t off=0;
    bmpeg2ts_parser_t parser;
    b_mpeg2ts_pcr_pid *pcr_pid;
    unsigned max_bitrate = 0;

    BDBG_OBJECT_ASSERT(probe, bmpeg2ts_psi_probe_t);

    BDBG_ASSERT(probe->stream==NULL);

    parser = probe->parser;

    probe->pat_pid.fired = false;
    probe->done = false;
    probe->done_pmt_pids = 0;
    probe->n_pmt_pids = 0;
    probe->n_pat_payloads = 0;
    do {
        batom_t atom;
        bfile_buffer_result result;
        size_t feed_len;
        size_t atom_len;
        bmpeg2ts_parser_status status;
        const size_t read_len = BMEDIA_PROBE_FEED_SIZE;

        BDBG_MSG(("b_mpeg2ts_psi_probe_parse: %p reading %u:%u", (void *)probe, (unsigned)(config->parse_offset+off), (unsigned)read_len));
        atom = bfile_buffer_read(buf, config->parse_offset+off, read_len, &result);
        if(!atom) {
            break;
        }
        atom_len = batom_len(atom);
        BDBG_MSG(("b_mpeg2ts_psi_probe_parse: %p read %u:%u -> %p", (void *)probe, (unsigned)(config->parse_offset+off), (unsigned)atom_len, (void *)atom));
        off += atom_len;
        batom_pipe_push(pipe, atom);
        feed_len = bmpeg2ts_parser_feed(parser, pipe);
        if(feed_len!=atom_len) {
            break;
        }
        if((!probe->pat_pid.fired && off>6*1024*1024) || off>8*1024*1024) {
            BDBG_MSG(("b_mpeg2ts_psi_probe_parse: %p read too much data",  (void *)probe));
            break; /* if we parsed 8Mbytes worth of data without PAT pid then we are done */
        }
        bmpeg2ts_parser_get_status(parser, &status);
        if(status.nresyncs>8 || status.resync_bytes > 8*BMPEG2TS_PKT_LEN) {
            BDBG_MSG(("b_mpeg2ts_psi_probe_parse: %#lx too much(%u:%u) resyncs ",  (unsigned long)probe, status.nresyncs, status.resync_bytes));
            break;
        }
    } while( !probe->done );
    BDBG_MSG(("b_mpeg2ts_psi_probe_parse: %#lx parsed %u bytes", (unsigned long)probe, (unsigned)off));
    if(probe->done) {
        for(pcr_pid = BLST_S_FIRST(&probe->pcr_pids);pcr_pid;pcr_pid=BLST_S_NEXT(pcr_pid, link)) {
            if(pcr_pid->last.pcr!=pcr_pid->first.pcr) {
                unsigned bitrate;
                bitrate = (8*45000*(pcr_pid->last.off - pcr_pid->first.off))/(pcr_pid->last.pcr - pcr_pid->first.pcr);
                if(bitrate>max_bitrate) {
                    max_bitrate = bitrate;
                }
            }
        }
    }

    for(i=0;i<probe->n_pmt_pids;i++) {
        BDBG_ASSERT(probe->pmt_pids);
        bmpeg2psi_parser_shutdown(&probe->pmt_pids[i].psi);
        if(BLST_S_FIRST(&probe->pmt_pids[i].group)) {
            bmpeg2ts_parser_remove_pid(parser, probe->pmt_pids[i].ts.pid);
        }
    }
    if(probe->pmt_pids) {
        BKNI_Free(probe->pmt_pids);
        probe->pmt_pids = NULL;
    }
    bmpeg2psi_parser_flush(&probe->pat_pid.psi);
    while( NULL!=(pcr_pid= BLST_S_FIRST(&probe->pcr_pids))) {
        BLST_S_REMOVE_HEAD(&probe->pcr_pids, link);
        BKNI_Free(pcr_pid);
    }
    bmpeg2ts_parser_reset(parser);
    bmpeg2ts_parser_add_pid(parser, &probe->pat_pid.ts);

    if (probe->stream) {
        stream = probe->stream;
        stream->media.max_bitrate = max_bitrate;
        stream->pkt_len = probe->pkt_len;
        probe->stream = NULL;
    } else {
        stream = NULL;
    }
    return &stream->media;
}

const bmedia_probe_format_desc bmpeg2ts_psi_probe = {
    bstream_mpeg_type_ts,
    bmpeg2ts_probe_ext, /* ext_list */
    BMPEG2TS_PKT_LEN*BMPEG2TS_PROBE_PACKETS, /* read several transport packets */
    bmpeg2ts_probe_header_match, /* header_match */
    b_mpeg2ts_psi_probe_create, /* create */
    b_mpeg2ts_psi_probe_destroy, /* destroy */
    b_mpeg2ts_psi_probe_parse, /* parse */
    bmedia_probe_basic_stream_free /* stream free */
};

const bmedia_probe_format_desc bmpeg2ts192_psi_probe = {
    bstream_mpeg_type_ts,
    bmpeg2ts192_probe_ext, /* ext_list */
    (BMPEG2TS_PKT_LEN+4)*BMPEG2TS_PROBE_PACKETS, /* read several transport packets */
    bmpeg2ts192_probe_header_match, /* header_match */
    b_mpeg2ts192_psi_probe_create, /* create */
    b_mpeg2ts_psi_probe_destroy, /* destroy */
    b_mpeg2ts_psi_probe_parse, /* parse */
    bmedia_probe_basic_stream_free /* stream free */
};
