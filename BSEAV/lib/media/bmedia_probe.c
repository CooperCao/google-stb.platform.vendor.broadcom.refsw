/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * BMedia library, stream probe module
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmedia_probe_impl.h"
#include "bkni.h"
#include "balloc.h"
#if B_HAS_ASF
#include "basf_probe.h"
#endif
#if B_HAS_AVI
#include "bavi_probe.h"
#endif
#if B_HAS_FLV
#include "bflv_probe.h"
#endif
#if B_HAS_RMFF
#include "brmff_probe.h"
#endif
#include "bmkv_probe.h"
#include "bmp4_probe.h"
#include "bmp3_probe.h"
#include "bwav_probe.h"
#include "bogg_probe.h"
#include "bflac_probe.h"
#include "bmedia_cdxa.h"
#include "bmpeg2ts_psi_probe.h"
#include "bmpeg2ts_probe.h"
#include "bmpeg2pes_probe.h"
#include "bmedia_probe_es.h"
#include "bmpeg1_probe.h"
#include "bmpeg_audio_util.h"
#include "bavc_video_probe.h"
#include "bavs_video_probe.h"
#include "bmpeg_video_probe.h"
#include "bac3_probe.h"
#include "bape_probe.h"
#include "baiff_probe.h"
#include "bamr_probe.h"
#include "bmedia_util.h"
#include "bhevc_video_probe.h"

BDBG_MODULE(bmedia_probe);
#define BDBG_MSG_TRACE(x)   /*BDBG_MSG(x)*/

BDBG_OBJECT_ID(bmedia_probe_t);


static const bmedia_probe_format_desc * const b_media_probe_formats[] = {
#if B_HAS_ASF
    &basf_probe,
#endif
#if B_HAS_AVI
    &bavi_probe,
#endif
#if B_HAS_FLV
    &bflv_probe,
#endif
#if B_HAS_RMFF
    &brmff_probe,
#endif
    &baiff_probe,
    &bamr_probe,
    &bape_probe,
    &bmkv_probe,
    &bmp4_probe,
    &bmp3_probe,
    &bwav_probe,
    &bogg_probe,
    &bflac_probe,
    &bcdxa_probe,
    &bmpeg2ts_psi_probe, 
    &bmpeg2ts_probe,
    &bmpeg1_probe,
    &bmpeg2pes_probe,
    &bmpeg2ts192_psi_probe, 
    &bmpeg2ts192_probe
};

#define B_MEDIA_N_PROBES ((sizeof(b_media_probe_formats)/sizeof(*b_media_probe_formats)))

#define B_MEDIA_PROBE_N_SEGS    128
#define B_MEDIA_PROBE_MAX_EXPECTED_BITRATE (5000*1024*8) /* 2000 Kbps */
#define B_MEDIA_PROBE_MIN_EXPECTED_BITRATE (30*1024*8) /* 50 Kbps */
#define B_MEDIA_PROBE_N_DURATION_SEGMENTS (10)
#define B_MEDIA_PROBE_N_BITRATE_RETRIES (6)
#define B_MEDIA_PROBE_MIN_SEGMENT_LENGTH (100000)
#define B_MEDIA_PROBE_MIN_BITRATE_THRESOLD (0.95)
#define B_MEDIA_PROBE_MAX_BITRATE_THRESOLD (1.05)

struct bmedia_probe {
    BDBG_OBJECT(bmedia_probe_t)
    batom_factory_t factory;
    batom_pipe_t pipe;
    bmedia_probe_es_t es_probe;
    bmedia_probe_base_t probes[B_MEDIA_N_PROBES];
};

bmedia_probe_t 
bmedia_probe_create(void)
{
    bmedia_probe_t  probe;
    unsigned i;

    probe = BKNI_Malloc(sizeof(*probe));
    if(!probe) {
        BDBG_ERR(("bmedia_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
        goto err_alloc;
    }
    BDBG_OBJECT_INIT(probe, bmedia_probe_t);
    probe->factory = batom_factory_create(bkni_alloc, 16);
    if(!probe->factory) {
        goto err_factory;
    }
    probe->pipe = batom_pipe_create(probe->factory);
    if(!probe->pipe) {
        goto err_pipe;
    }
    probe->es_probe = NULL;
    /* clear all probes */
    for(i=0;i<B_MEDIA_N_PROBES;i++) {
        probe->probes[i]=NULL;
    }
    BDBG_MSG_TRACE(("bmedia_probe_create<: %#lx", (unsigned long)probe));
    return probe;
err_pipe:
    batom_factory_destroy(probe->factory);
err_factory:
    BKNI_Free(probe);
err_alloc:
    return NULL;
}

void 
bmedia_probe_destroy(bmedia_probe_t probe)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(probe, bmedia_probe_t);
    for(i=0;i<B_MEDIA_N_PROBES;i++) {
        if(probe->probes[i]) {
            b_media_probe_formats[i]->destroy(probe->probes[i]);
        }
    }
    if(probe->es_probe) {
        bmedia_probe_es_destroy(probe->es_probe);
    }
    batom_pipe_destroy(probe->pipe);
    batom_factory_destroy(probe->factory);
    BDBG_OBJECT_DESTROY(probe, bmedia_probe_t);
    BKNI_Free(probe);
    return;
}


static size_t 
b_strlen(const char *s)
{
    size_t off;
    for(off=0;s[off]!='\0';off++) { }
    return off;
}

static const char *
b_strrchr(const char *s, int c)
{
    size_t off;

    for(off=b_strlen(s);off>0;off--) {
        if(s[off]==c) {
            return s+off;
        }
    }
    if(s[0]==c) {
        return s;
    }
    return NULL;
}

static int 
b_strcasecmp(const char *s1, const char *s2)
{
    size_t off;
    for(off=0;;off++) {
        char c1 = s1[off];
        char c2 = s2[off];
        int diff;
        if(c1==c2) {
            if(c1==0) {
                return 0;
            }
            continue;
        }
        if(c1 >= 'a' && c1 <= 'z') {
            c1 &= ~0x20;
        }
        if(c2 >= 'a' && c2 <= 'z') {
            c2 &= ~0x20;
        }
        diff = c1 - c2;
        if(diff!=0) {
            return diff;
        }
    }
}

bool 
bmedia_probe_match_ext(const bmedia_probe_file_ext *ext_list, const char *ext)
{
    unsigned i;
    if(ext_list == NULL || ext == NULL || ext_list[0][0]=='\0' || ext[0]=='\0') {
        return true;
    }
    for(i=0;ext_list[i][0]!='\0';i++) {
        if(b_strcasecmp(ext_list[i], ext)==0) {
            BDBG_MSG(("bmedia_probe_match_ext: ext:%s matched:%s", ext, ext_list[i]));
            return true;
        }
    }
    return false;
}


static unsigned
b_media_probe_filter_ext(bool *valid_probes, const char *ext)
{
    unsigned i,nvalid;
    BDBG_MSG_TRACE(("b_media_probe_filter_ext: ext:%s", ext?ext:"NONE"));
    for(nvalid=0,i=0;i<B_MEDIA_N_PROBES;i++) {
        if(!valid_probes[i]) {
            continue;
        }
        valid_probes[i] = bmedia_probe_match_ext(b_media_probe_formats[i]->ext_list, ext);
        if(valid_probes[i]) {
            nvalid++;
        }
    }
    BDBG_MSG(("b_media_probe_filter_ext: ext:%s nvalid:%u", ext?ext:"NONE", nvalid));
    return nvalid;
}

static unsigned
b_media_probe_filter_header(bmedia_probe_t probe, bool *valid_probes, bfile_buffer_t buf, const bmedia_probe_parser_config *config)
{
    size_t probe_size;
    unsigned i,nvalid=0;
    batom_t atom;
    batom_cursor cursor;
    bfile_buffer_result result;

    BSTD_UNUSED(probe);

    for(i=0,probe_size=4; i<B_MEDIA_N_PROBES; i++) {
        if( valid_probes[i] && probe_size<b_media_probe_formats[i]->header_size ) {
            probe_size=b_media_probe_formats[i]->header_size;
        }
    }
    atom = bfile_buffer_read(buf, config->parse_offset+0, probe_size, &result);
    if(atom==NULL) {
        goto err_probe;
    }
    for(i=0; i<B_MEDIA_N_PROBES; i++) {
        if(valid_probes[i]) {
            batom_cursor_from_atom(&cursor, atom);
            if(b_media_probe_formats[i]->header_match(&cursor)) {
                nvalid++;
                continue;
            }
            valid_probes[i] = false;
        }
    }
    batom_release(atom);
done:
    BDBG_MSG(("b_media_probe_filter_header: nvalid:%u", nvalid));
    return nvalid;

err_probe:
    for(i=0; i<B_MEDIA_N_PROBES; i++) {
        valid_probes[i]=false;
    }
    goto done;
}

static unsigned
b_media_probe_filter_type(bool *valid_probes, bstream_mpeg_type type)
{
    unsigned i;
    unsigned nvalid;
    for(nvalid=0,i=0; i<B_MEDIA_N_PROBES; i++) {
        if(valid_probes[i]) {
            if(b_media_probe_formats[i]->type == type) {
                nvalid++;
                continue;
            }
            valid_probes[i] = false;
        }
    }
    BDBG_MSG(("b_media_probe_filter_type: nvalid:%u", nvalid));
    return nvalid;
}

static int b_media_strlen(const char *s) {
    const char *t=s;
    while (*t) { t++; }
    return t-s;
}

typedef struct b_snprintf_buf {
    size_t left;
    size_t printed;
    char *buf;
} b_snprintf_buf;


static void 
b_snprintf_buf_advance(b_snprintf_buf *b, int rc)
{
    BDBG_ASSERT(b->left>=1);
    if(rc>0) {
        b->printed += rc;
        if((unsigned)rc < b->left) {
            b->left -= rc;
            b->buf += rc;
        } else {
            b->buf += b->left-1;
            b->left = 1;
        }
    }
}

int
bmedia_stream_to_string(const bmedia_probe_stream *stream, char *dest_buf, size_t size)
{
    const char *format;
    const bmedia_probe_track *track;
    unsigned last_program;
    unsigned hours;
    unsigned mins;
    unsigned secs;
    char format_extra[128];
    char fps[16];
    const char *index;
    b_snprintf_buf buf;

    BDBG_ASSERT(dest_buf);
    if(size<1) {
        return -1;
    }

    buf.left = size;
    buf.buf = dest_buf;
    buf.printed = 0;
    if(!stream) {
        return BKNI_Snprintf(dest_buf, size, "unknown");
    }
    fps[0]='\0';
    switch(stream->type) {
    default:
    case bstream_mpeg_type_unknown: format="unknown"; break;
    case bstream_mpeg_type_es: 
        if(stream->probe_id!=BMP3_PROBE_ID) {
            format="ES";
        } else {
            const bmp3_probe_stream *mp3 = (bmp3_probe_stream *)stream;
            const char *id3;
            switch(mp3->id3) {
            default:
            case bmp3_id3_none: id3=""; break;
            case bmp3_id3v1: id3="-ID3V1"; break;
            case bmp3_id3v2_2: id3="-ID3V2_2"; break;
            case bmp3_id3v2_3: id3="-ID3V2_3"; break;
            case bmp3_id3v2_4: id3="-ID3V2_4"; break;
            }
            BKNI_Snprintf(format_extra, sizeof(format_extra), "MP3%s{%s%s%s%s%s}", id3, mp3->title.str, mp3->title.len>0?" ; ":"", mp3->artist.str, mp3->artist.len>0?" ; ":"", mp3->album.str);
            format = format_extra;
        }
        break;
    case bstream_mpeg_type_bes: format="BES";break;
    case bstream_mpeg_type_pes: format="PES";break;
    case bstream_mpeg_type_ts: format="MPEG2-TS";break;
    case bstream_mpeg_type_dss_es: format="DSS-ES";break;
    case bstream_mpeg_type_dss_pes: format="DSS-PES";break;
    case bstream_mpeg_type_vob: format="DVD VOB";break;
#if B_HAS_ASF
    case bstream_mpeg_type_asf:  
        {
            const basf_probe_stream *asf= (basf_probe_stream *)stream;
            char deviceConformanceTemplate[BASF_DEVICE_CONFORMANCE_TEMPLATE_BUFFER+1];
            unsigned i;
            unsigned j;
            *deviceConformanceTemplate = '\0';
            for(j=i=0;i<asf->deviceConformanceTemplateCount;i++) {
                for(;j<BASF_DEVICE_CONFORMANCE_TEMPLATE_BUFFER;j++) {
                    deviceConformanceTemplate[j] = (char )asf->deviceConformanceTemplates[j];
                    if(deviceConformanceTemplate[j]==0) {
                        break;
                    }
                }
                deviceConformanceTemplate[j] = ' ';
                deviceConformanceTemplate[j+1] = '\0';
                j++;
            }
            BKNI_Snprintf(format_extra, sizeof(format_extra), "ASF{%s%s%s%s%s%s}", asf->seekableFlag?"seekable ":"", asf->broadcastFlag?"broadcast ":"", asf->liveFlag?"live":"", asf->stereoscopic_info.stereoscopic?"3D ":"", asf->encryption_type!=basf_encryption_none?"DRM ":"", deviceConformanceTemplate);
            format=format_extra;
            break;
        }
#endif
#if B_HAS_AVI
    case bstream_mpeg_type_avi:
        format="AVI";
        BKNI_Snprintf(fps, sizeof(fps), " %u.%03uFPS ", ((bavi_probe_stream *)stream)->video_framerate/1000, ((bavi_probe_stream *)stream)->video_framerate%1000);
        break;
#endif
    case bstream_mpeg_type_mpeg1: format="MPEG1";break;
    case bstream_mpeg_type_cdxa: format="CDXA";break;
    case bstream_mpeg_type_mp4:
        {
            const bmp4_probe_stream *mp4_stream = (bmp4_probe_stream *)stream;
            BKNI_Snprintf(format_extra, sizeof(format_extra), "MP4 [%s%s%s]", mp4_stream->ftyp, mp4_stream->compatible?"":",not compatible", mp4_stream->fragmented?",fragmented":"");
            format=format_extra;
        }
        break;
    case bstream_mpeg_type_mkv:
        {
            const bmkv_probe_stream *mkv_stream = (bmkv_probe_stream *)stream;
            BKNI_Snprintf(format_extra, sizeof(format_extra), "MKV [%s%s]", mkv_stream->docType, mkv_stream->next_volume.next_volume_offset_valid?":multivolume":"");
            format=format_extra;
        }
        break;
    case bstream_mpeg_type_flv: format="FLV";break;
    case bstream_mpeg_type_wav: format="WAV";break;
    case bstream_mpeg_type_ogg: format="OGG";break;
    case bstream_mpeg_type_flac: format="FLAC";break;
    case bstream_mpeg_type_rmff: format="RMFF";break;
    case bstream_mpeg_type_ape: format="APE";break;
    case bstream_mpeg_type_aiff: format="AIFF";break;
    case bstream_mpeg_type_amr: format="AMR";break;
    }
    switch(stream->index) {
    default:
    case bmedia_probe_index_unknown: index = ""; break;
    case bmedia_probe_index_required:
    case bmedia_probe_index_available: index= "INDEX:"; break;
    case bmedia_probe_index_missing: index= "NO-INDEX:"; break;
    case bmedia_probe_index_self: index="SELF-INDEX:"; break;
    case bmedia_probe_index_unusable: index="UNUSABLE-INDEX:"; break;
    }
    secs = stream->duration/1000;
    mins = secs/60;
    hours = mins/60;
    mins = mins%60;
    secs = secs%60;
    b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "%s[%s%u:%u] %u:%02u.%02u %s%uKbps [ ", format, index, stream->ntracks, stream->nprograms, hours, mins, secs, fps, stream->max_bitrate/1000));
    for(last_program=0,track=BLST_SQ_FIRST(&stream->tracks); track; track=BLST_SQ_NEXT(track,link)) {
        const char *language=NULL, *codec_id=NULL;
#if B_HAS_ASF
        char ascii_language[6];
#endif
        char codec_specific[48];
        size_t codec_specific_offset = 0;

        codec_specific[0]='\0';
        codec_specific[1]='\0';
        if(last_program!=track->program && track!=BLST_SQ_FIRST(&stream->tracks)) {
            b_snprintf_buf_advance(&buf,BKNI_Snprintf(buf.buf, buf.left, "] ["));
        }
        last_program=track->program;
        switch(track->type) {
        case bmedia_track_type_audio:
            switch(track->info.audio.codec) {
            default:
            case baudio_format_unknown: format="unknown";break;
            case baudio_format_mpeg: format="MPEG1/2 Layer1/2";break;
            case baudio_format_mp3: format="MPEG1/2 Layer3";break;
            case baudio_format_aac: format="AAC";break;
            case baudio_format_aac_plus: format="AAC+";break;
            /* case baudio_format_aac_plus_loas: format="AAC+ LOAS";break; */
            case baudio_format_aac_plus_adts: format="AAC+ ADTS";break;
            case baudio_format_ac3: format="AC3"; break;
            case baudio_format_ac3_plus: format="AC3+";break;
            case baudio_format_lpcm_hddvd: format="LPCM HD-DVD";break;
            case baudio_format_lpcm_bluray: format="LPCM BluRay";break;
            case baudio_format_wma_std: format="WMA STD";break;
            case baudio_format_wma_pro: format="WMA PRO";break;
            case baudio_format_lpcm_dvd: format="LPCM DVD";break;
            case baudio_format_pcm: format="PCM";break;
            case baudio_format_amr_nb: format="AMR NB";break;
            case baudio_format_amr_wb: format="AMR WB";break;
            case baudio_format_vorbis: format="VORBIS";break;
            case baudio_format_flac: format="FLAC";break;
            case baudio_format_dra: format="DRA";break;
            case baudio_format_cook: format="COOK";break;
            case baudio_format_g726: format="G726";break;
            case baudio_format_g711: format="G711";break;
            case baudio_format_adpcm: format="ADPCM";break;
            case baudio_format_dvi_adpcm: format="DVI ADPCM";break;
            case baudio_format_dts: format="DTS";break;
            case baudio_format_dts_hd: format="DTS-HD";break;
            case baudio_format_dts_cd: format="DTS-CD";break;
            case baudio_format_dts_lbr: format="DTS-LBR";break;
            case baudio_format_lpcm_1394: format="LPCM-1394";break;
            case baudio_format_ape: format="APE";break;
            case baudio_format_mlp: format="MLP";break;
            case baudio_format_opus: format="OPUS";break;
            case baudio_format_als: format="ALS";break;
            case baudio_format_als_loas: format="ALS LOAS";break;
            case baudio_format_ac4: format="AC4"; break;
            }

            switch(track->info.audio.codec) {
            default:
                break;
            case baudio_format_ac3:
            case baudio_format_ac3_plus:
                if(((bmedia_probe_ac3_audio*)&track->info.audio.codec_specific)->valid) {
                    codec_specific[0]=' ';
                switch(((bmedia_probe_ac3_audio*)&track->info.audio.codec_specific)->codec) {
                    case baudio_format_ac3:
                        BKNI_Snprintf(codec_specific+1, sizeof(codec_specific)-1,"AC3");
                        break;
                    case baudio_format_ac3_plus:
                        BKNI_Snprintf(codec_specific+1, sizeof(codec_specific)-1,"AC3+");
                        break;
                    default:
                        BKNI_Snprintf(codec_specific+1, sizeof(codec_specific)-1,"Unknown");
                        break;
                    }
                }
                break;
            }

            b_snprintf_buf_advance(&buf,BKNI_Snprintf(buf.buf, buf.left,"0x%02x:audio(%s %u:%u:%u:%u%s) ", track->number, format, (unsigned)track->info.audio.sample_rate, (unsigned)track->info.audio.channel_count, (unsigned)track->info.audio.sample_size, (unsigned)track->info.audio.bitrate, codec_specific));
            break;
        case bmedia_track_type_video:
            switch(track->info.video.codec) {
            default:
            case bvideo_codec_unknown: format="unknown";break;
            case bvideo_codec_mpeg1: format="MPEG1";break;
            case bvideo_codec_mpeg2: format="MPEG2";break;
            case bvideo_codec_mpeg4_part2: format="MPEG4 Part2";break;
            case bvideo_codec_h263: format="H.263";break;
            case bvideo_codec_h264: format="H.264"; break;
            case bvideo_codec_h264_svc: format="H.264 SVC"; break;
            case bvideo_codec_h264_mvc: format="H.264 MVC"; break;
            case bvideo_codec_h265: format="H.265"; break;
            case bvideo_codec_vc1: format="VC-1 AP";break;
            case bvideo_codec_vc1_sm: format="VC-1 SP/MP";break;
            case bvideo_codec_divx_311: format="DivX 3.11";break;
            case bvideo_codec_avs: format="AVS";break;
            case bvideo_codec_vp6: format="VP6";break;
            case bvideo_codec_spark: format="Spark/H.263";break;
            case bvideo_codec_rv40: format="RV40";break;
            case bvideo_codec_vp8: format="VP8";break;
            case bvideo_codec_vp9: format="VP9";break;
            case bvideo_codec_mjpeg: format="MJPEG";break;
            }

            switch(track->info.video.codec) {
            default: break;
            case bvideo_codec_h264:
            case bvideo_codec_h264_svc:
            case bvideo_codec_h264_mvc:
                if(((bmedia_probe_h264_video *)&track->info.video.codec_specific)->sps.valid) {
                    codec_specific[0]=' ';
                    b_h264_video_sps_to_string(&((bmedia_probe_h264_video *)&track->info.video.codec_specific)->sps, codec_specific+1, sizeof(codec_specific)-1);
                }
                break;
            case bvideo_codec_mpeg1:
            case bvideo_codec_mpeg2:
                {
                    const bmedia_probe_mpeg_video *mpeg_video = (bmedia_probe_mpeg_video*)&track->info.video.codec_specific;
                    const char *profile;
                    const char *level;
                    switch(mpeg_video->profile) {
                    case 0x5: profile = "Simple";break;
                    case 0x4: profile = "Main";break;
                    case 0x3: profile = "SNR Scalable";break;
                    case 0x2: profile = "Spatially Scalable";break;
                    case 0x1: profile = "High";break;
                    default: profile = "";break;
                    }
                    switch(mpeg_video->level) {
                    case 0x0A: level = "Low";break;
                    case 0x08: level = "Main";break;
                    case 0x06: level = "High 1440";break;
                    case 0x04: level = "High";break;
                    default: level = "";break;
                    }
                    BKNI_Snprintf(codec_specific, sizeof(codec_specific), " %u.%03uFPS(%s,%s%s)", mpeg_video->framerate/1000, mpeg_video->framerate%1000, profile, level, mpeg_video->progressive_sequence?",progressive":"");
                }
                break;
            case bvideo_codec_avs:
                BKNI_Snprintf(codec_specific, sizeof(codec_specific), " %u.%03uFPS", ((bmedia_probe_avs_video*)&track->info.video.codec_specific)->framerate/1000, ((bmedia_probe_avs_video*)&track->info.video.codec_specific)->framerate%1000);
                break;
            case bvideo_codec_h265:
                if(((bmedia_probe_h265_video *)&track->info.video.codec_specific)->sps.valid) {
                    codec_specific[0]=' ';
                    bh265_video_sps_to_string(&((bmedia_probe_h265_video *)&track->info.video.codec_specific)->sps, codec_specific+1, sizeof(codec_specific)-1);
                }
                break;
            }
            codec_specific_offset = b_media_strlen(codec_specific);
            switch(stream->type) {
#if B_HAS_ASF
            case bstream_mpeg_type_asf:
                {
                    const basf_probe_track *asf_track = (basf_probe_track *)track;

                    BKNI_Snprintf(codec_specific+codec_specific_offset, sizeof(codec_specific)-codec_specific_offset, " " BMEDIA_FOURCC_FORMAT "" ,BMEDIA_FOURCC_ARG(asf_track->mediaType));
                    codec_specific_offset = b_media_strlen(codec_specific);
                    if(asf_track->aspectRatioValid && track->info.video.height) {
                        double ar = (asf_track->aspectRatio.x * (float)track->info.video.width) / (asf_track->aspectRatio.y * (float)track->info.video.height);
                        const char *sar;
                        switch((int)(ar*100)) {
                        case 177: sar = "16:9"; break;
                        case 133: sar = "4:3"; break;
                        case 150: sar = "3:2"; break;
                        case 160: sar = "16:10"; break;
                        default: sar = NULL; break;
                        }
                        if(sar) {
                            BKNI_Snprintf(codec_specific+codec_specific_offset, sizeof(codec_specific)-codec_specific_offset, " %s",  sar);
                        } else {
                            BKNI_Snprintf(codec_specific+codec_specific_offset, sizeof(codec_specific)-codec_specific_offset, " (AR %.2f)", ar);
                        }
                        codec_specific_offset = b_media_strlen(codec_specific);
                    }
                    if(asf_track->encryptedContentFlag) {
                        BKNI_Snprintf(codec_specific+codec_specific_offset, sizeof(codec_specific)-codec_specific_offset, " DRM");
                        codec_specific_offset = b_media_strlen(codec_specific);
                    }
                }
                break;
#endif
#if B_HAS_RMFF
            case bstream_mpeg_type_rmff:
                BKNI_Snprintf(codec_specific+codec_specific_offset, sizeof(codec_specific)-codec_specific_offset, " %u.%03uFPS ", ((brmff_probe_track*)track)->info.video.framerate/1000, ((brmff_probe_track*)track)->info.video.framerate%1000);
                break;
#endif
#if B_HAS_AVI
            case bstream_mpeg_type_avi:
                {
                    const bavi_probe_track *avi_track = (bavi_probe_track *)track;
                    BKNI_Snprintf(codec_specific+codec_specific_offset, sizeof(codec_specific)-codec_specific_offset, " " BMEDIA_FOURCC_FORMAT "" ,BMEDIA_FOURCC_ARG(avi_track->media_type));
                    codec_specific_offset = b_media_strlen(codec_specific);
                }
                break;
#endif

            case bstream_mpeg_type_mp4:
                {
                    const bmp4_probe_track *mp4_track = (bmp4_probe_track *)track;
                    if(mp4_track->duration) {
                        unsigned framerate = ((1000*1000)*(uint64_t)mp4_track->sample_count)/mp4_track->duration;
                        BKNI_Snprintf(codec_specific+codec_specific_offset, sizeof(codec_specific)-codec_specific_offset, " %u.%03uFPS %s", framerate/1000, framerate%1000, mp4_track->encrypted?"DRM":"");
                    }
                }
                break;
            case bstream_mpeg_type_ts:
                if(track->probe_id == BMPEG2TS_PSI_PROBE_ID || track->probe_id == BMPEG2TS192_PSI_PROBE_ID) {
                    const bmpeg2ts_psi_probe_track *psi_track =(bmpeg2ts_psi_probe_track *) track;
                    switch(psi_track->transport_type) {
                    case bmpeg2ts_psi_transport_caption_service:
                        if(psi_track->caption_service.number_of_services>0) {
                            BKNI_Snprintf(codec_specific+codec_specific_offset, sizeof(codec_specific)-codec_specific_offset, " caption <%s> ", psi_track->caption_service.services[0].language);
                        }
                        break;
                    default:
                        break;
                    }
                }
                break;
            default:
                break;
            }

            b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "0x%02x:video(%s%s %uKbps %ux%u%s) ", track->number, format, codec_specific, track->info.video.bitrate, track->info.video.width, track->info.video.height, track->info.video.timestamp_order==bmedia_timestamp_order_decode?" DTS":""));
            break;
        case bmedia_track_type_pcr:
            b_snprintf_buf_advance(&buf,BKNI_Snprintf(buf.buf, buf.left,"0x%02x:PCR ", track->number));
            break;
        case bmedia_track_type_other:
            {
                const char *type = "unknown";
                if(stream->type == bstream_mpeg_type_ts && (track->probe_id == BMPEG2TS_PSI_PROBE_ID || track->probe_id == BMPEG2TS192_PSI_PROBE_ID)) {
                    const bmpeg2ts_psi_probe_track *psi_track =(bmpeg2ts_psi_probe_track *) track;
                    switch(psi_track->transport_type) {
                    case bmpeg2ts_psi_transport_dvb_subtitles: type = "DVB subtitles";break;
                    case bmpeg2ts_psi_transport_teletext: type = "Teletext";break;
                    case bmpeg2ts_psi_transport_pmt: type = "PMT";break;
                    case bmpeg2ts_psi_transport_pat: type = "PAT";break;
                    case bmpeg2ts_psi_transport_pcr: type = "PCR";break;
                    case bmpeg2ts_psi_transport_caption_service:
                        type = "caption";
                        if(psi_track->caption_service.number_of_services>0) {
                            b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "<%s> ", psi_track->caption_service.services[0].language));
                        }
                        break;
                    default: break;
                    }
                }
                b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "0x%02x:%s ", track->number, type));
            }
            break;
        }

        switch(stream->type) {
        case bstream_mpeg_type_mkv:
            {
                const bmkv_probe_track *mkv_track = (bmkv_probe_track*)track;
                language = mkv_track->language;
                codec_id = mkv_track->codec_id;
                if(mkv_track->unsupported) {
                    b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "UNSUPPORTED"));
                }
            }
            break;
        case bstream_mpeg_type_mp4:
            language = ((bmp4_probe_track*)track)->language;
            break;
        case bstream_mpeg_type_ts:
            if(track->probe_id == BMPEG2TS_PSI_PROBE_ID || track->probe_id == BMPEG2TS192_PSI_PROBE_ID) {
                const bmpeg2ts_psi_probe_track *psi_track =(bmpeg2ts_psi_probe_track *) track;
                if(psi_track->language.valid) {
                    language = psi_track->language.code;
                }
            }
            break;
#if B_HAS_AVI
        case bstream_mpeg_type_avi:
            language = ((bavi_probe_track*)track)->language;
            break;
#endif
#if B_HAS_ASF
        case bstream_mpeg_type_asf:
            {
                uint16_t *asf_language = ((basf_probe_track*)track)->language;
                if(asf_language) {
                    unsigned i;
                    BKNI_Memset(ascii_language, 0, sizeof(ascii_language));
                    for(i=0;i<sizeof(ascii_language)-1;i++) {
                        ascii_language[i] = (unsigned char)asf_language[i];
                        if(asf_language[i]==0) {
                            break;
                        }
                    }
                    language = ascii_language;
                }
            }
            break;
#endif
        default:
            break;
        }
        if(language && *language!='\0') {
            b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "<%s> ", language));
        }
        if(codec_id && *codec_id!='\0') {
            b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "<%s> ", codec_id));
        }
    }

    b_snprintf_buf_advance(&buf, BKNI_Snprintf(buf.buf, buf.left, "]"));
    return buf.printed;
}

static const bmedia_probe_stream *
b_media_probe_feed_es(bmedia_probe_t probe, const char *ext, bfile_buffer_t buf)
{
    off_t off;
    bmedia_probe_track *track = NULL;
    bmedia_probe_stream *stream = NULL;

    if(!probe->es_probe) {
        probe->es_probe = bmedia_probe_es_create(probe->factory);
        if(!probe->es_probe) {
            return NULL;
        }
    }
    bmedia_probe_es_filter_ext(probe->es_probe, ext);
    for(off=0;;) {
        bfile_buffer_result result;
        unsigned nactive;
        const size_t read_len = BMEDIA_PROBE_FEED_SIZE;
        batom_t atom = bfile_buffer_read(buf, off, read_len, &result);
        BDBG_MSG(("b_media_probe_feed_es: %p off:%u read_len:%u result:%u atom:%p(%u)", (void *)probe, (unsigned)off, (unsigned)read_len, result, (void *)atom, atom?(unsigned)batom_len(atom):0));
        if(atom==NULL) {
            track = bmedia_probe_es_last(probe->es_probe);
            break;
        }
        BDBG_ASSERT(atom);
        off+=batom_len(atom);
        track = bmedia_probe_es_feed(probe->es_probe, atom, &nactive);
        if(track || nactive==0) {
            break;
        }
    }
    bmedia_probe_es_reset(probe->es_probe);
    if(track) {
        stream = BKNI_Malloc(sizeof(*stream));
        if(stream) {
            track->number = 1; /* assign track number */ 
            stream->probe_id = B_MEDIA_N_PROBES;
            bmedia_probe_stream_init(stream, bstream_mpeg_type_es);
            if(track->type == bmedia_track_type_audio) {
                if(stream->max_bitrate==0) {
                    stream->max_bitrate = track->info.audio.bitrate*1000;
                }
            } else if (track->type == bmedia_track_type_video) {
                stream->max_bitrate = track->info.video.bitrate*1000;
            }
            bmedia_probe_add_track(stream, track);
        } else {
            BKNI_Free(track);
        }
    }
    return stream;
}

static void
b_media_probe_parse_aux(bmedia_probe_t probe, bmedia_probe_stream *stream, const bmedia_probe_format_desc *media_probe, bfile_buffer_t buf, const bmedia_probe_parser_config  *parser_config, const bmedia_probe_config *probe_config)
{
    unsigned i;

    for(i=0;i<B_MEDIA_N_PROBES ;i++) {
        if(b_media_probe_formats[i] == media_probe) {
            bmedia_probe_track *track_aux, *track;
            struct bmedia_probe_track_list extra_tracks;
            bmedia_probe_stream *stream_aux;

            BLST_SQ_INIT(&extra_tracks);

            if(!probe->probes[i]) {
                probe->probes[i]= b_media_probe_formats[i]->create(probe->factory);
                if(!probe->probes[i]) {
                    BDBG_WRN(("b_media_probe_parse_aux: %#lx can't create probe for stream type %u", (unsigned long)probe, b_media_probe_formats[i]->type));
                    break;
                }
            }
            stream_aux = (bmedia_probe_stream *)media_probe->parse(probe->probes[i], buf, probe->pipe, parser_config);
            if(stream_aux && stream_aux->type == stream->type) {
                if(stream->duration==0) {
                    stream->duration = stream_aux->duration;
                }
                if(stream->max_bitrate==0) {
                    stream->max_bitrate = stream_aux->max_bitrate;
                }

                while(NULL!=(track_aux=BLST_SQ_FIRST(&stream_aux->tracks))) {
                    bool match_found=false;

                    BLST_SQ_REMOVE_HEAD(&stream_aux->tracks, link);

                    for(track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
                        bool matched = track->number == track_aux->number;
                        bool mpeg2ts_matched = stream->type == bstream_mpeg_type_ts && B_MEDIA_MPEG2TS_TRACKNO_PID(track->number) == B_MEDIA_MPEG2TS_TRACKNO_PID(track_aux->number);
                        if(matched || mpeg2ts_matched) {
                            bmpeg2ts_psi_probe_track *psi_track = (bmpeg2ts_psi_probe_track *)track;
                            bmpeg2ts_probe_track *ts_track = (bmpeg2ts_probe_track *)track_aux;
                            psi_track->parsed_payload = ts_track->parsed_payload;
                            if(track->type == track_aux->type || track->type == bmedia_track_type_other) {
                                bool found_info = false;
                                switch(track->type) {
                                case bmedia_track_type_other:
                                    /* We have seen sometimes that the PMT returns the type as bmedia_track_type_other for some streams.
                                       Since the PIDS match we should be able to use the track_aux info */
                                    track->type = track_aux->type;
                                    if(track_aux->type == bmedia_track_type_video) {
                                        track->info.video = track_aux->info.video;
                                    } else if(track_aux->type == bmedia_track_type_audio) {
                                        track->info.audio = track_aux->info.audio;
                                    }
                                    match_found=true;
                                    found_info=true;
                                    break;
                                case bmedia_track_type_video:
                                    if(track->info.video.codec == track_aux->info.video.codec || track->info.video.codec == bvideo_codec_unknown) {
                                        track->info.video = track_aux->info.video;
                                    } else if(probe_config->probe_config.mpeg2ts_psi.reprobe_codec) {
                                        BDBG_WRN(("b_media_probe_parse_aux: video codecs did not match use track_aux (%d:%d)",track->info.video.codec, track_aux->info.video.codec));
                                        track->info.video = track_aux->info.video;
                                    }
                                    track->info.video = track_aux->info.video;
                                    match_found=true;
                                    found_info=true;
                                    break;
                                case bmedia_track_type_audio:
                                    if(!matched && mpeg2ts_matched) {
                                        if(track->info.audio.codec != track_aux->info.audio.codec) {
                                            break;
                                        }
                                    }
                                    if(matched) {
                                        if(track->info.audio.codec == track_aux->info.audio.codec || track->info.audio.codec == baudio_format_unknown) {
                                            track->info.audio = track_aux->info.audio;
                                        } else if(track_aux->info.audio.codec == baudio_format_dts &&
                                               (track->info.audio.codec == baudio_format_dts_hd)) {
                                            baudio_format old_codec= track->info.audio.codec;
                                            track->info.audio = track_aux->info.audio;
                                            track->info.audio.codec = old_codec;
                                        } else if(probe_config->probe_config.mpeg2ts_psi.reprobe_codec) {
                                            BDBG_WRN(("b_media_probe_parse_aux: audio codecs did not match use track_aux (%d:%d)",track->info.audio.codec, track_aux->info.audio.codec));
                                            track->info.audio = track_aux->info.audio;
                                        }
                                    }
                                    match_found=true;
                                    found_info=true;
                                    break;
                                case bmedia_track_type_pcr:
                                    /* fall through */
                                default:
                                    match_found=true;
                                    break;
                                }
                                if(found_info) {
                                    break;
                                }
                            } else if (track_aux->type == bmedia_track_type_other) { /* other track matches any type */
                                match_found=true;
                            }
                        }
                    }

                    if(!match_found){
                        BLST_SQ_INSERT_HEAD(&extra_tracks, track_aux, link);
                    } else {
                        BKNI_Free(track_aux);
                    }
                }

                /* copy extra tracks */
                while(NULL!=(track_aux=BLST_SQ_FIRST(&extra_tracks))) {
                    BLST_SQ_REMOVE_HEAD(&extra_tracks,link);
                    track_aux->program = BMEDIA_PROBE_INVALID_PROGRAM;
                    track_aux->probe_id = stream->probe_id;
                    BLST_SQ_INSERT_TAIL(&stream->tracks, track_aux, link);
                }

            }
            if(stream_aux) {
                b_media_probe_formats[i]->stream_free(probe->probes[i], (bmedia_probe_stream *)stream_aux);
            }
            break;
        }
    }
    return;
}

#define B_MEDIA_PROBE_TIMESTAMP_STEP    (4*1024*1024)

static int
b_media_probe_get_timestamp(bmedia_probe_t probe, bfile_buffer_t buf, bmedia_timestamp_parser_t timestamp_parser, off_t offset, bmedia_timestamp *timestamp)
{
    bfile_buffer_result result;
    const size_t read_len = BMEDIA_PROBE_FEED_SIZE;
    int rc=-1;
    size_t parsed;
    batom_t atom;

    BSTD_UNUSED(probe);

    timestamp_parser->methods->seek(timestamp_parser, offset);
    timestamp->offset = 0;
    timestamp->timestamp = 0;
    for(parsed=0;parsed<B_MEDIA_PROBE_TIMESTAMP_STEP;) {
        batom_cursor cursor;
        atom = bfile_buffer_read(buf, offset+parsed, read_len, &result);
        BDBG_MSG(("b_media_probe_get_timestamp: %p off:%u(%u) read_len:%u result:%u atom:%p(%u)", (void *)probe, (unsigned)(offset+parsed), (unsigned)parsed, (unsigned)read_len, result, (void *)atom, atom?(unsigned)batom_len(atom):0));
        if(!atom) {
            break;
        }
        parsed += batom_len(atom);
        batom_cursor_from_atom(&cursor, atom);
        rc = timestamp_parser->methods->parse(timestamp_parser, &cursor, timestamp);
        BDBG_MSG(("b_media_probe_get_timestamp: %p rc=%d offset:%u timestamp:%u", (void *)probe, rc, (unsigned)timestamp->offset, (unsigned)timestamp->timestamp));
        batom_release(atom);
        if(rc!=0) {
            if(rc==1) {
                rc=0; /* success */
            }
            break;
        }
        rc=-1;
    }
    return rc;
}
/* Take n samples (B_MEDIA_PROBE_N_DURATION_SEGMENTS) from the segment and average out the average bitrate */
static unsigned
b_media_probe_calculate_segment_bitrate(bmedia_probe_t probe, bfile_buffer_t buf, bmedia_timestamp_parser_t timestamp_parser, const bmedia_probe_config *config, off_t off_first, off_t off_last)
{
    off_t skip;
    off_t off = off_first;
    bmedia_timestamp first, last;
    unsigned bitrate = 0;
    int count = 0;
    int i, rc;
    unsigned sum_bitrate = 0;

    if (off_first >= off_last) {
        return 0;
    }

    skip = (off_last - off_first) / (config->duration_config.max_bitrate_n_segments + 1);

    rc = b_media_probe_get_timestamp(probe, buf, timestamp_parser, off, &first);
    if(rc == 0 ) {

        off += skip;
        for(i = 0; (i < config->duration_config.max_bitrate_n_segments) && (off < off_last); i++)
        {

            rc = b_media_probe_get_timestamp(probe, buf, timestamp_parser, off, &last);
            if(rc!=0) { break;}

            /* If the there is a discontinuity (beginning.pts > end.pts) ignore that value */
            if(off != off_first && first.timestamp < last.timestamp && first.offset < last.offset)
            {
                sum_bitrate += (8*45000*(last.offset - first.offset))/(last.timestamp - first.timestamp);
                count++;
            }
            first = last;

            off += skip;
        }
        if (count != 0) {
            bitrate = sum_bitrate / count;
        }
    }
    return bitrate;
}

/* Check the two halves of a section to see if the bitrate of the halves is similar*/
static unsigned
b_media_probe_calculate_section_bitrate(bmedia_probe_t probe, bfile_buffer_t buf, bmedia_timestamp_parser_t timestamp_parser, const bmedia_probe_config *config, off_t off_first, off_t off_last, int depth)
{
    int rc;
    int bitrate_first_half=0, bitrate_second_half=0, bitrate = 0, bitrate_difference = 0, bitrate_max = 0;
    unsigned duration;
    bmedia_timestamp first, last, middle;

    if (off_first >= off_last) {
        return 0;
    }

    /* Try binary search n times (max_bitrate_probe_depth) or until the section is smaller than B_MEDIA_PROBE_MIN_SEGMENT_LENGTH */
    /* If either of those are true then try to calculate the average bitrate of the entire section based on segments */
    if (depth < config->duration_config.max_bitrate_probe_depth && ((off_last - off_first) > config->duration_config.min_bitrate_segment_size)) {
        rc = b_media_probe_get_timestamp(probe, buf, timestamp_parser, off_first, &first);
        if(rc!=0) {return 0;}

        rc = b_media_probe_get_timestamp(probe, buf, timestamp_parser, (off_first+off_last)/2, &middle);
        if(rc!=0) {return 0;}

        rc = b_media_probe_get_timestamp(probe, buf, timestamp_parser, off_last, &last);
        if(rc!=0) {return 0;}

        /* if the offsets are not sequential then there may have been an error */
        if (first.offset >= middle.offset || middle.offset >= last.offset ||
            first.offset >= last.offset)
        {
            return 0;
        }

        if(middle.timestamp > first.timestamp) {
            duration = (middle.timestamp - first.timestamp);
            bitrate_first_half = (8*45000*(middle.offset - first.offset))/duration;
        }
        if(last.timestamp > middle.timestamp) {
            duration = (last.timestamp - middle.timestamp);
            bitrate_second_half = (8*45000*(last.offset - middle.offset))/duration;
        }

        bitrate_difference = bitrate_first_half > bitrate_second_half ? (bitrate_first_half - bitrate_second_half) : (bitrate_second_half - bitrate_first_half);            
        bitrate_max = bitrate_first_half > bitrate_second_half ? ((bitrate_second_half * config->duration_config.max_bitrate_percentage_difference) /100) : ((bitrate_first_half * config->duration_config.max_bitrate_percentage_difference)/100);
        /* If either half bitrate is 0 or if the bitrates are not within a partiuclar range try again */
        if (bitrate_first_half == 0 || bitrate_second_half == 0 ||
            (bitrate_difference > bitrate_max))
        {
            bitrate_first_half = b_media_probe_calculate_section_bitrate(probe, buf, timestamp_parser, config, off_first, (off_last + off_first) / 2, depth + 1);
            bitrate_second_half = b_media_probe_calculate_section_bitrate(probe, buf, timestamp_parser, config, (off_last + off_first) / 2, off_last, depth + 1);
            
            if (bitrate_first_half == 0 && bitrate_second_half != 0 )
            {
                bitrate_first_half = bitrate_second_half;
            }

            if (bitrate_first_half != 0 && bitrate_second_half == 0)
            {
               bitrate_second_half = bitrate_first_half;
            }
        }
        bitrate = (bitrate_first_half + bitrate_second_half) / 2;
    }
    else
    {
        bitrate =  b_media_probe_calculate_segment_bitrate(probe, buf, timestamp_parser, config, off_first, off_last);
    }
    return bitrate;

}

static unsigned
b_media_duration_from_timestamp(const bmedia_timestamp *timestamp_first, const bmedia_timestamp *timestamp_last, off_t offset_first, off_t offset_last)
{
    uint32_t timestamp_diff;
    unsigned bitrate;
    unsigned duration;

    timestamp_diff = timestamp_last->timestamp - timestamp_first->timestamp;

    if(timestamp_diff==0) {
        return 0;
    }
    if(timestamp_first->offset >= timestamp_last->offset) {
        return 0;
    }
    if(offset_first >= offset_last) {
        return 0;
    }
    bitrate = (8*45000*(timestamp_last->offset - timestamp_first->offset))/timestamp_diff;
    if(bitrate==0) {
        return 0;
    }
    duration = ((1000*8)*(offset_last - offset_first))/bitrate;
    return duration;
}



static unsigned
b_media_probe_acquire_duration(bmedia_probe_t probe, bfile_io_read_t fd, bfile_buffer_t buf, bmedia_timestamp_parser_t timestamp_parser, const bmedia_probe_config *config)
{
    bmedia_timestamp first, last, middle;
    int rc;
    int bitrate_max = 0, bitrate_difference = 0;
    off_t off_first, off_last, read_off_last;
    unsigned duration;
    unsigned duration_pcr, duration_first_half, duration_second_half;
    unsigned step_back;

    BDBG_OBJECT_ASSERT(probe, bmedia_probe_t);
    BDBG_ASSERT(timestamp_parser);
    BDBG_ASSERT(buf);
    BDBG_ASSERT(fd);

    rc = fd->bounds(fd, &off_first, &off_last);
    if(rc!=0) { goto error;}

    rc = b_media_probe_get_timestamp(probe, buf, timestamp_parser, off_first, &first);
    if(rc!=0) { goto error;}

    rc = b_media_probe_get_timestamp(probe, buf, timestamp_parser, (off_first+off_last)/2, &middle);
    if(rc!=0) { goto error;}

    for(read_off_last = off_last, step_back=BMEDIA_PROBE_FEED_SIZE;step_back<B_MEDIA_PROBE_TIMESTAMP_STEP;step_back*=2) {

        if(off_first+step_back<=read_off_last) {
            read_off_last = read_off_last - step_back;
        } else {
            read_off_last = off_first+ 3*((off_last - off_first)/4);
        }
        rc = b_media_probe_get_timestamp(probe, buf, timestamp_parser, read_off_last, &last);
        if(rc==0) { 
            break;
        }
        last = middle;
    }
    BDBG_MSG(("b_media_probe_acquire_duration: first:%u,%u middle:%u,%u last:%u,%u", (unsigned)first.offset, (unsigned)first.timestamp, (unsigned)middle.offset, (unsigned)middle.timestamp, (unsigned)last.offset, (unsigned)last.timestamp));

    if(!(first.offset < last.offset)) {
        goto error;
    }
    duration_pcr = last.timestamp - first.timestamp;
    duration = b_media_duration_from_timestamp(&first, &last, off_first, off_last);
    duration_first_half = b_media_duration_from_timestamp(&first, &middle, off_first, (off_first+off_last)/2);
    duration_second_half = b_media_duration_from_timestamp(&middle, &last,(off_first+off_last)/2, off_last);

    BDBG_MSG(("b_media_probe_acquire_duration: %#lx duration %u(%u) %u+%u", (unsigned long)probe, duration, duration_first_half+duration_second_half, duration_first_half, duration_second_half));
    /* compare that  total  approximately equal to sum of parts */
    if( (9*(duration/16) > (duration_first_half + duration_second_half) || 21*(duration/16) < (duration_first_half + duration_second_half))) {
        /* get lowest bitrate */
        unsigned bitrate_first_half=0, bitrate_second_half=0, bitrate;

        if(middle.timestamp > first.timestamp) {
            duration_pcr = (middle.timestamp - first.timestamp);
            bitrate_first_half = (8*45000*(middle.offset - first.offset))/duration_pcr;
        }
        if(last.timestamp > middle.timestamp) {
            duration_pcr = (last.timestamp - middle.timestamp);
            bitrate_second_half = (8*45000*(last.offset - middle.offset))/duration_pcr;
        }

        /* For discontinous files it may be necessary to break down the file to smaller sections to ensure duration */
        if (config->duration_config.max_bitrate_n_segments != 0 && config->duration_config.max_bitrate_probe_depth != 0)
        {
            bitrate_difference = bitrate_first_half > bitrate_second_half ? (bitrate_first_half - bitrate_second_half) : (bitrate_second_half - bitrate_first_half);            
            bitrate_max = bitrate_first_half > bitrate_second_half ? ((bitrate_second_half * config->duration_config.max_bitrate_percentage_difference) /100) : ((bitrate_first_half * config->duration_config.max_bitrate_percentage_difference)/100);
            /* If either half bitrate is 0 or if the bitrates are not within a partiuclar range try again */
            if (bitrate_first_half == 0 || bitrate_second_half == 0 ||
                (bitrate_difference > bitrate_max))
            {
                bitrate_first_half = b_media_probe_calculate_section_bitrate(probe, buf, timestamp_parser, config, off_first, (off_last + off_first) / 2, 1);
                bitrate_second_half = b_media_probe_calculate_section_bitrate(probe, buf, timestamp_parser, config, (off_last + off_first) / 2, off_last, 1);
            }
            
        }

        if ((bitrate_first_half > B_MEDIA_PROBE_MAX_EXPECTED_BITRATE || bitrate_first_half < B_MEDIA_PROBE_MIN_EXPECTED_BITRATE) &&
            (bitrate_second_half <= B_MEDIA_PROBE_MAX_EXPECTED_BITRATE && bitrate_second_half >= B_MEDIA_PROBE_MIN_EXPECTED_BITRATE)) {
            bitrate = bitrate_second_half;
        } else if ((bitrate_second_half > B_MEDIA_PROBE_MAX_EXPECTED_BITRATE || bitrate_second_half < B_MEDIA_PROBE_MIN_EXPECTED_BITRATE) &&
            (bitrate_first_half <= B_MEDIA_PROBE_MAX_EXPECTED_BITRATE && bitrate_first_half >= B_MEDIA_PROBE_MIN_EXPECTED_BITRATE)) {
            bitrate = bitrate_first_half;
        } else if((bitrate_first_half < bitrate_second_half && bitrate_first_half>0) || bitrate_second_half==0) {
            bitrate = bitrate_first_half;
        } else {
            bitrate = bitrate_second_half;
        }
        if(bitrate>0) {
            duration = ((1000*8)*(off_last - off_first))/bitrate;
        } else {
            duration = 0;
        }
        BDBG_MSG(("b_media_probe_acquire_duration: %#lx detected discontinuous stream, bitrate %u:%u, duration:%u", (unsigned long)probe, bitrate_first_half, bitrate_second_half, duration));
    } else {
        BDBG_MSG(("b_media_probe_acquire_duration: %#lx continuous stream, duration:%u", (unsigned long)probe, duration));
    }
    return duration;

error:
    return 0;
}

static const bmedia_probe_stream *
b_media_probe_parse_step(bmedia_probe_t probe, bfile_io_read_t fd, bfile_buffer_t buf, const bmedia_probe_config *config, const char *ext)
{
    unsigned i;
    bool valid_probes[B_MEDIA_N_PROBES];
    const bmedia_probe_stream *stream=NULL;
    const bmedia_probe_stream *stream_only=NULL;
    bmedia_probe_parser_config parser_config;

    parser_config.parse_offset = config->probe_offset;
    parser_config.probe_index = config->probe_index;
    parser_config.parse_index = config->parse_index;
    parser_config.stream_specific = config->stream_specific;
    parser_config.min_parse_request = config->min_probe_request;
    for(i=0;i<B_MEDIA_N_PROBES;i++) {
        valid_probes[i] = true;
    }

    if(config->type == bstream_mpeg_type_unknown || b_media_probe_filter_type(valid_probes, config->type)>0) {
        if(b_media_probe_filter_ext(valid_probes, ext)) {
            if(b_media_probe_filter_header(probe, valid_probes, buf, &parser_config)>0) {
                for(i=0;i<B_MEDIA_N_PROBES;i++) {
                    if(!valid_probes[i]) {
                        continue;
                    }
                    if(probe->probes[i]==NULL) { /* create probe on demand */
                        probe->probes[i]= b_media_probe_formats[i]->create(probe->factory);
                        if(!probe->probes[i]) {
                            BDBG_WRN(("bmedia_probe_parse: %#lx can't create probe for stream type %u", (unsigned long)probe, b_media_probe_formats[i]->type));
                            continue;
                        }
                    }
                    batom_pipe_flush(probe->pipe);
                    BDBG_MSG(("bmedia_probe_parse: %#lx probing type %u", (unsigned long)probe, b_media_probe_formats[i]->type));
                    stream = b_media_probe_formats[i]->parse(probe->probes[i], buf, probe->pipe, &parser_config);
                    if(stream) {
                        ((bmedia_probe_stream *)stream)->probe_id = i;

                        if(config->probe_duration) {
                            const bmedia_probe_track *track;
                            bmedia_timestamp_parser_t timestamp_parser={0};
                            switch(stream->type) {
                            case bstream_mpeg_type_ts:
                                for(track=BLST_SQ_FIRST(&stream->tracks); track; track=BLST_SQ_NEXT(track, link)) {
                                    if(track->type == bmedia_track_type_pcr) {
                                        timestamp_parser = bmpeg2ts_pcr_parser_create(track->number, ((bmpeg2ts_probe_stream*)stream)->pkt_len);
                                        break;
                                    }
                                }
                                break;
                            case bstream_mpeg_type_pes:
                            case bstream_mpeg_type_vob:
                                for(track=BLST_SQ_FIRST(&stream->tracks); track; track=BLST_SQ_NEXT(track, link)) {
                                    if(track->type == bmedia_track_type_video) {
                                        timestamp_parser = bmpeg2pes_pts_parser_create(track->number, 0);
                                        break;
                                    }
                                }
                                break;
                            case bstream_mpeg_type_mpeg1:
                                for(track=BLST_SQ_FIRST(&stream->tracks); track; track=BLST_SQ_NEXT(track, link)) {
                                    if(track->type == bmedia_track_type_video) {
                                        timestamp_parser = bmpeg1_pts_parser_create(track->number, 0);
                                        break;
                                    }
                                }
                                break;
                            default:
                                timestamp_parser = NULL;
                                break;
                            }
                            if(timestamp_parser) {
                                unsigned duration = b_media_probe_acquire_duration(probe, fd, buf, timestamp_parser, config);
                                if(duration>0) {
                                    ((bmedia_probe_stream *)stream)->duration = duration;
                                    ((bmedia_probe_stream *)stream)->max_bitrate = 0; /* clear max_bitrate so it would be re-evaluated based on the stream size and duration */
                                }
                                timestamp_parser->methods->destroy(timestamp_parser);
                            }
                        }
                        if(config->probe_es || config->probe_payload) {
                            if(b_media_probe_formats[i]==&bmpeg2ts_psi_probe) {
                                b_media_probe_parse_aux(probe, (bmedia_probe_stream *)stream, &bmpeg2ts_probe, buf, &parser_config, config);
                            } else if (b_media_probe_formats[i]==&bmpeg2ts192_psi_probe) {
                                b_media_probe_parse_aux(probe, (bmedia_probe_stream *)stream, &bmpeg2ts192_probe, buf, &parser_config, config);
                            }
                        }
                        /* Check induced MPEG-2 TS index */
                        if(stream->type == bstream_mpeg_type_ts) {
                            const bmedia_probe_track *track;
                            bool has_video = false;
                            bool has_pcr = false;
                            for(track=BLST_SQ_FIRST(&stream->tracks); track; track=BLST_SQ_NEXT(track, link)) {
                                switch(track->type) {
                                case bmedia_track_type_pcr:
                                    has_pcr = true;
                                    break;
                                case bmedia_track_type_video:
                                    has_video = true;
                                    break;
                                default:
                                    break;
                                }
                                if(has_pcr && has_video) {
                                    ((bmedia_probe_stream *)stream)->index = bmedia_probe_index_self;
                                }
                            }
                        }
                        if(stream->type == bstream_mpeg_type_pes || stream->type == bstream_mpeg_type_vob){
                            const bmedia_probe_track *track;
                            for(track=BLST_SQ_FIRST(&stream->tracks); track; track=BLST_SQ_NEXT(track, link)) {
                                if(track->type == bmedia_track_type_video) {
                                    ((bmedia_probe_stream *)stream)->index = bmedia_probe_index_self;
                                    break;
                                }
                            }
                        }
                        if(stream_only) {
                            bmedia_probe_stream_free(probe, stream_only);
                            stream_only = NULL;
                        }
                        if(BLST_SQ_FIRST(&stream->tracks)) {
                            break;
                        }
                        stream_only = stream;
                    }
                }
            }
        }
    }
    if(!stream) {
        stream = stream_only;
    }
    return stream;
}




const bmedia_probe_stream *
bmedia_probe_parse(bmedia_probe_t probe, bfile_io_read_t fd, const bmedia_probe_config *config)
{
    bfile_buffer_cfg buffer_cfg;
    bfile_buffer_t buf;
    const bmedia_probe_stream *stream=NULL;
    const char *ext = NULL;
    void *file_buffer=NULL;


    BDBG_OBJECT_ASSERT(probe, bmedia_probe_t);
    BDBG_ASSERT(fd);
    BDBG_ASSERT(config);
    BDBG_MSG_TRACE(("bmedia_probe_parse>: %#lx fd:%#lx config:%s:%u", (unsigned long)probe, (unsigned long)fd, config->file_name?config->file_name:"", (unsigned)config->type));

    if(!config->buffer) {
        file_buffer = BKNI_Malloc(B_MEDIA_PROBE_N_SEGS * BMEDIA_PROBE_FEED_SIZE);
        if(!file_buffer) {
            goto err_file_buf;
        }
        bfile_buffer_default_cfg(&buffer_cfg);
        buffer_cfg.buf_len = B_MEDIA_PROBE_N_SEGS * BMEDIA_PROBE_FEED_SIZE;
        buffer_cfg.buf = file_buffer;
        buffer_cfg.fd = fd;
        buffer_cfg.nsegs = B_MEDIA_PROBE_N_SEGS;
        buf = bfile_buffer_create(probe->factory, &buffer_cfg);
        if(!buf) {
            BDBG_ERR(("bmedia_probe_parse: %#lx can't create buffer", (unsigned long)probe));
            goto err_buf;
        }
    } else {
        buf = config->buffer;
    }

    if(config->file_name) {
        ext = b_strrchr(config->file_name, '.');
        if(ext) {
            ext++;
        }
        BDBG_MSG(("bmedia_probe_parse: file_name:%s ext:%s", config->file_name, ext?ext:"NONE"));
    }

    stream = b_media_probe_parse_step(probe, fd, buf, config, ext);
    if(!stream && ext && config->probe_all_formats) {
        stream = b_media_probe_parse_step(probe, fd, buf, config, NULL);
    }

    fd->seek(fd, 0, SEEK_SET);

    batom_pipe_flush(probe->pipe);
    if(!stream && config->probe_es) {
        stream = b_media_probe_feed_es(probe, ext, buf);
        if(!stream && ext && config->probe_all_formats) {
            stream = b_media_probe_feed_es(probe, NULL, buf);
        }
    }
    if(stream) {
        if(stream->duration == 0 || stream->max_bitrate==0) {
            off_t first, last;
            int rc = fd->bounds(fd, &first, &last);
            if(rc==0) {
                if(stream->max_bitrate != 0) {
                    ((bmedia_probe_stream *)stream)->duration = ((1000*8)*(last - first))/stream->max_bitrate;
                } else if(stream->duration !=0) {
                    ((bmedia_probe_stream *)stream)->max_bitrate = ((1000*8)*(last - first))/stream->duration;
                }
            }
        }
        if(stream->probe_id < B_MEDIA_N_PROBES) {
            bmedia_probe_track *track;
            if(b_media_probe_formats[stream->probe_id]==&bmp3_probe) {
                ((bmedia_probe_stream *)stream)->probe_id = BMP3_PROBE_ID;
            } else if(b_media_probe_formats[stream->probe_id]==&bmpeg2ts_psi_probe) {
                ((bmedia_probe_stream *)stream)->probe_id = BMPEG2TS_PSI_PROBE_ID;
            } else if(b_media_probe_formats[stream->probe_id]==&bmpeg2ts192_psi_probe) {
                ((bmedia_probe_stream *)stream)->probe_id = BMPEG2TS192_PSI_PROBE_ID;
            } else if(b_media_probe_formats[stream->probe_id]==&bmpeg2pes_probe) {
                ((bmedia_probe_stream *)stream)->probe_id = BMPEG2PES_PROBE_ID;
            }
            for(track=BLST_SQ_FIRST(&stream->tracks); track; track=BLST_SQ_NEXT(track,link)) {
                if(track->probe_id==0) {
                    track->probe_id = stream->probe_id;
                }
            }
        }
        if(stream->type == bstream_mpeg_type_es) {
            const bmedia_probe_track *track =BLST_SQ_FIRST(&stream->tracks);
            if(track && BLST_SQ_NEXT(track, link)==NULL) {
                if(track->type == bmedia_track_type_audio) {
                    switch(track->info.audio.codec) {
                    case baudio_format_aac:
                    case baudio_format_aac_plus_adts:
                    case baudio_format_ac3:
                    case baudio_format_ac3_plus:
                        ((bmedia_probe_stream *)stream)->index = bmedia_probe_index_self;
                    default: break;
                    }
                }
            }
        }
    }


    if(!config->buffer) {
        bfile_buffer_destroy(buf);
    }
    if(file_buffer) {
        BKNI_Free(file_buffer);
    }
#if BDBG_DEBUG_BUILD
    if(stream) {
        char buf[196];

        bmedia_stream_to_string(stream, buf, sizeof(buf));
        BDBG_MSG(("bmedia_probe_stream: %s", buf));
    }
#endif
    return stream;
        
err_buf:
    BKNI_Free(file_buffer);
err_file_buf:
    return NULL;
}

void 
bmedia_probe_default_cfg(bmedia_probe_config *config)
{
    BKNI_Memset(config, 0, sizeof(*config));
    config->file_name = NULL;
    config->type = bstream_mpeg_type_unknown;
    config->probe_es = true;
    config->probe_payload = true;
    config->probe_all_formats = true;
    config->probe_duration = true;
    config->probe_index = true;
    config->parse_index = false;
    config->buffer = NULL;
    config->probe_offset = 0;
    config->min_probe_request = 512*1024;
    config->stream_specific.mkv.probe_next_volume = false;
    config->stream_specific.mkv.probe_attachments = false;

    config->duration_config.max_bitrate_n_segments = 0; 
    config->duration_config.max_bitrate_percentage_difference = 0;
    config->duration_config.max_bitrate_probe_depth = 0;
    config->duration_config.min_bitrate_segment_size = 0;
    config->probe_config.mpeg2ts_psi.reprobe_codec = false;
    return;
}

void 
bmedia_probe_stream_free(bmedia_probe_t probe, const bmedia_probe_stream *stream)
{
    const bmedia_probe_format_desc *probe_desc;
    unsigned  probe_id; 
    BDBG_OBJECT_ASSERT(probe, bmedia_probe_t);
    BDBG_ASSERT(stream);
    switch(stream->probe_id) {
    case BMP3_PROBE_ID:
        probe_desc = &bmp3_probe;
        probe_id = B_MEDIA_N_PROBES;
        break;
    case BMPEG2TS_PSI_PROBE_ID:
        probe_desc = &bmpeg2ts_psi_probe;
        probe_id = B_MEDIA_N_PROBES;
        break;
    case BMPEG2TS192_PSI_PROBE_ID:
        probe_desc = &bmpeg2ts192_psi_probe;
        probe_id = B_MEDIA_N_PROBES;
        break;
    case BMPEG2PES_PROBE_ID:
        probe_desc = &bmpeg2pes_probe;
        probe_id = B_MEDIA_N_PROBES;
        break;
    default:
        BDBG_ASSERT(stream->probe_id <= B_MEDIA_N_PROBES);
        probe_desc = NULL;
        probe_id = stream->probe_id;
        break;
    }
    if(probe_desc) {
        for(probe_id=0;probe_id<B_MEDIA_N_PROBES;probe_id++) {
            if(b_media_probe_formats[probe_id] == probe_desc) {
                break;
            }
        }
    }
    if(probe_id <= B_MEDIA_N_PROBES) {  /* workaround for Coverity not understanding BDBG_ASSERT() */
        if(probe_id!=B_MEDIA_N_PROBES) {
            BDBG_ASSERT(probe->probes[probe_id]);
            b_media_probe_formats[probe_id]->stream_free(probe->probes[probe_id], (bmedia_probe_stream *)stream);
        } else {
            /* ES stream */
            bmedia_probe_basic_stream_free(NULL, (bmedia_probe_stream *)stream);
        }
    }
    return;
}

void 
bmedia_probe_stream_init(bmedia_probe_stream *stream, bstream_mpeg_type type)
{
    BLST_SQ_INIT(&stream->tracks);
    stream->type = type;
    stream->max_bitrate = 0;
    stream->duration = 0;
    stream->nprograms = 0;
    stream->ntracks = 0;
    stream->index = bmedia_probe_index_unknown;
    return;
}


void 
bmedia_probe_basic_stream_free(bmedia_probe_base_t probe, bmedia_probe_stream *stream)
{
    bmedia_probe_track *track;
    BSTD_UNUSED(probe);
    BDBG_ASSERT(stream);

    while(NULL!=(track=BLST_SQ_FIRST(&stream->tracks))) {
        BLST_SQ_REMOVE_HEAD(&stream->tracks, link);
        BKNI_Free(track);
    }
    BKNI_Free(stream);
    return;
}

void 
bmedia_probe_track_init(bmedia_probe_track *track)
{
    BKNI_Memset(track, 0, sizeof(*track));
    track->type = bmedia_track_type_other;
    return;
}

void 
bmedia_probe_add_track(bmedia_probe_stream *stream, bmedia_probe_track *new_track)
{
    bmedia_probe_track *prev, *track;
    bool new_program;

    BDBG_ASSERT(stream);
    BDBG_ASSERT(new_track);

    for(track=BLST_SQ_FIRST(&stream->tracks); track; track=BLST_SQ_NEXT(track, link)) {
        if(track->number == new_track->number && track->type == new_track->type && track->program==new_track->program) {
            BDBG_WRN(("bmedia_probe_add_track: %#lx %#lx duplicate %u:%u:%u", (unsigned long)stream, (unsigned long)new_track, new_track->number, new_track->program, new_track->program));
            BKNI_Free(new_track);
            return;
        }
    }
    for(new_program=true, prev=NULL, track=BLST_SQ_FIRST(&stream->tracks); track; track=BLST_SQ_NEXT(track, link)) {
        if(track->program > new_track->program) {
            break;
        }
        new_program = track->program!=new_track->program;
        prev = track;
    }
    if(prev==NULL) {
        BLST_SQ_INSERT_HEAD(&stream->tracks, new_track, link);
    } else {
        BLST_SQ_INSERT_AFTER(&stream->tracks, prev, new_track, link);
    }
    stream->ntracks++;
    if(new_program) {
        stream->nprograms++;
    }
    return;
}

void 
bmedia_probe_video_init(bmedia_probe_video *video)
{
    BKNI_Memset(video, 0, sizeof(*video));
    video->codec = bvideo_codec_unknown;
    video->timestamp_order = bmedia_timestamp_order_display;
    video->height = 0;
    video->width = 0;
    video->bitrate = 0;
    return;
}



unsigned bmedia_probe_get_video_color_depth(const bmedia_probe_track *track)
{
    unsigned colorDepth;

    if (track->type != bmedia_track_type_video) {
        BDBG_ERR(("bmedia_video_get_color_depth called for non-video track type: %d", track->type));
        colorDepth = 0;
        goto out;
    }
    switch (track->info.video.codec) {
    case bvideo_codec_h265:
        if (((bmedia_probe_h265_video*)&track->info.video.codec_specific)->sps.valid) {
            colorDepth = ((bmedia_probe_h265_video*)&track->info.video.codec_specific)->sps.bit_depth_luma;
        } else {
            colorDepth = 10;
        }
        break;
    case bvideo_codec_vp9:
        colorDepth = 10; /* apparently there is no sensible way to detect whether VP9 uses 8 or 10 bits, so return 10 */
        break;
    default:
        /* we support 8 bits only for all other codecs */
        colorDepth = 8;
        break;
    }

out:
    return colorDepth;
}
