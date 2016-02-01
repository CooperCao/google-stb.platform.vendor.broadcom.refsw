/***************************************************************************
 *     Copyright (c) 2007-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * BMedia library, MKV stream probe module
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bmkv_probe.h"
#include "bmkv_file_parser.h"
#include "bkni.h"
#include "bmedia_util.h"
#include "bavc_video_probe.h"
#include "biobits.h"
#include "bmpeg4_util.h"
#include "baac_probe.h"

BDBG_MODULE(bmkv_probe);


typedef struct bmkv_probe  *bmkv_probe_t;

typedef struct bmkv_AttachedFile {
    bmkv_utf8 FileDescription;
    bmkv_utf8 FileName;
    bmkv_utf8 FileMimeType;
    uint64_t FileUID;
    bmkv_size FileData;
    struct {
	bool FileDescription;
	bool FileName;
	bool FileMimeType;
	bool FileUID;
	bool FileData;
    } validate;
} bmkv_AttachedFile;

typedef struct bmkv_ChapterTrack {
    uint32_t ChapterTrackNumber;
    struct {
	bool ChapterTrackNumber;
    } validate;
} bmkv_ChapterTrack;

typedef struct bmkv_ChapterDisplay {
    bmkv_utf8 ChapString;
    char ChapLanguage[16];
    char ChapCountry[16];
    struct {
	bool ChapString;
	bool ChapLanguage;
	bool ChapCountry;
    } validate;
} bmkv_ChapterDisplay;

typedef struct bmkv_ChapterAtom {
    uint64_t ChapterUID;
    uint64_t ChapterTimeStart;
    uint64_t ChapterTimeEnd;
    uint32_t ChapterFlagHidden;
    uint32_t ChapterFlagEnabled;
    bmkv_data ChapterSegmentUID;
    bmkv_data ChapterSegmentEditionUID;
    uint32_t ChapterPhysicalEquiv;
    bmkv_table ChapterTrack;
    bmkv_table ChapterDisplay;
    struct {
	bool ChapterUID;
	bool ChapterTimeStart;
	bool ChapterTimeEnd;
	bool ChapterFlagHidden;
	bool ChapterFlagEnabled;
	bool ChapterSegmentUID;
	bool ChapterSegmentEditionUID;
	bool ChapterPhysicalEquiv;
	bool ChapterTrack;
	bool ChapterDisplay;
    } validate;
} bmkv_ChapterAtom;

typedef struct bmkv_EditionEntry {
    uint64_t EditionUID;
    uint32_t EditionFlagHidden;
    uint32_t EditionFlagDefault;
    uint32_t EditionFlagOrdered;
    bmkv_table ChapterAtom;
    struct {
	bool EditionUID;
	bool EditionFlagHidden;
	bool EditionFlagDefault;
	bool EditionFlagOrdered;
	bool ChapterAtom;
    } validate;
} bmkv_EditionEntry;

struct bmkv_probe {
    BDBG_OBJECT(bmkv_probe_t)
    bmkv_probe_stream *stream;
    bmkv_probe_track *track;
    batom_factory_t factory;
    struct {
	bfile_cached_segment cache;
    } attachment;
    struct {
	bfile_cached_segment cache;
    } edition;
    struct bmkv_file_parser mkv_file;
    bmkv_AttachedFile AttachedFile;
    bmkv_EditionEntry EditionEntry;
};

BDBG_OBJECT_ID(bmkv_probe_t);

static bool
b_mkv_probe_header_match(batom_cursor *header)
{
	bmkv_header mkv_header;
    bmkv_EBML ebml;

	if(!bmkv_parse_header(header, &mkv_header)) {
        return false;
	}
    if(mkv_header.id != BMKV_EBML_ID) {
        return false;
    }
    if(!bmkv_EBML_parse(header, mkv_header.size, &ebml)) {
        return false;
    }
    return bmkv_EBML_validate(&ebml);
}


static const bmedia_probe_file_ext b_mkv_ext[] =  {
	{"mkv"},
	{"webm"},
	{""}
};


static bmedia_probe_base_t
b_mkv_probe_create(batom_factory_t factory)
{
	bmkv_probe_t	probe;
    int rc;

	probe = BKNI_Malloc(sizeof(*probe));
	if(!probe) {
		BDBG_ERR(("b_mkv_probe_create: can't allocate %u bytes", (unsigned)sizeof(*probe)));
		goto err_alloc;
	}
	BDBG_OBJECT_INIT(probe, bmkv_probe_t);
	probe->stream = NULL;
	probe->track = NULL;
    probe->factory = factory;

    rc = bmkv_file_parser_init(&probe->mkv_file, factory);
    if(rc<0) {
        goto err_parser;
    }

	return (bmedia_probe_base_t)probe;
err_parser:
	BKNI_Free(probe);
err_alloc:
	return NULL;
}

static void
b_mkv_probe_destroy(bmedia_probe_base_t probe_)
{
	bmkv_probe_t probe = (bmkv_probe_t)probe_;

	BDBG_OBJECT_ASSERT(probe, bmkv_probe_t);
	bmkv_file_parser_shutdown(&probe->mkv_file);
	BDBG_OBJECT_DESTROY(probe, bmkv_probe_t);
	BKNI_Free(probe);
	return;
}

static void
b_mkv_probe_add_tracks(bmkv_probe_t probe, bmkv_probe_stream *stream)
{
    unsigned i;

    if(!probe->mkv_file.validate.tracks || !probe->mkv_file.trackData.validate.TrackEntry) {
        return ;
    }
    stream->media.nprograms = 1;
    for(i=0;i<probe->mkv_file.trackData.TrackEntry.nelems;i++) {
        const bmkv_TrackEntry *mkv_track = &BMKV_TABLE_ELEM(probe->mkv_file.trackData.TrackEntry,bmkv_TrackEntry,i);
        bmkv_probe_track *track;
        size_t extra_size=0;
        uint8_t *extra;
        bmkv_file_encoding_info encoding_info;

        if( !mkv_track->validate.TrackNumber) {
            continue;
        }
        if( mkv_track->validate.CodecPrivate) {
            extra_size += mkv_track->CodecPrivate.data_len;
        }
        if( mkv_track->validate.CodecName) {
            extra_size += mkv_track->CodecName.utf8_len;
        }
        track = BKNI_Malloc(sizeof(*track)+extra_size);
        if(!track) {
            BDBG_ERR(("%s: %p can't allocate %u bytes", "b_mkv_probe_add_tracks", (void *)track, (unsigned)sizeof(*track)));
            break;
        }
        extra = (uint8_t *)track + sizeof(*track);
        bmedia_probe_track_init(&track->media);
        stream->media.ntracks ++;
        track->language[0]='\0';
        track->codec_id[0]='\0';
        track->CodecName_len = 0;
        track->CodecName_data = NULL;
        track->AttachmentLink_valid = false ;
        BKNI_Memset(track->AttachmentLink, 0, sizeof(track->AttachmentLink));
        track->media.number = mkv_track->TrackNumber;
        track->CodecPrivate_len = 0;
        track->TrickTrackUID = 0;
        track->MasterTrackUID = 0;
        track->TrickTrackFlag = false;
        track->unsupported = false;
        BLST_SQ_INSERT_TAIL(&stream->media.tracks, &track->media, link);
        if(mkv_track->validate.Language) {
            BDBG_CASSERT(sizeof(mkv_track->Language)==sizeof(track->language));
            BKNI_Memcpy(track->language, mkv_track->Language, sizeof(track->language));
        }
        if(mkv_track->validate.CodecPrivate) {
            track->CodecPrivate_data = extra;
            track->CodecPrivate_len = mkv_track->CodecPrivate.data_len;
            BKNI_Memcpy(extra, mkv_track->CodecPrivate.data, mkv_track->CodecPrivate.data_len);
            extra += mkv_track->CodecPrivate.data_len;
        }
        if(mkv_track->validate.CodecID) {
            BDBG_CASSERT(sizeof(mkv_track->CodecID)==sizeof(track->codec_id));
            BKNI_Memcpy(track->codec_id, mkv_track->CodecID, sizeof(track->codec_id));
        }
        if(mkv_track->validate.CodecName) {
            track->CodecName_data = extra;
            track->CodecName_len = mkv_track->CodecName.utf8_len;
            BKNI_Memcpy(extra, mkv_track->CodecName.utf8_data, mkv_track->CodecName.utf8_len);
            extra += mkv_track->CodecName.utf8_len;
        }
        if(mkv_track->validate.AttachmentLink) {
            unsigned j;
            for(j=0;j<mkv_track->AttachmentLink.nelems;j++) {
                const bmkv_AttachmentLink *bmkv_attachment;
                if(j>=BMKV_PROBE_MAX_ATTACHMENT_LINKS)
                    break;
                bmkv_attachment = &BMKV_TABLE_ELEM(mkv_track->AttachmentLink, bmkv_AttachmentLink, j);
                track->AttachmentLink[j] = bmkv_attachment->AttachmentLink;
            }
            track->AttachmentLink_valid = true;
        }
        if(mkv_track->validate.TrackUID) {
            track->TrackUID = mkv_track->TrackUID;
        }
        if(mkv_track->validate.TrickTrackUID) {
            track->TrickTrackUID = mkv_track->TrickTrackUID;
        }
        if(mkv_track->validate.MasterTrackUID) {
            track->MasterTrackUID = mkv_track->MasterTrackUID;
        }
        if(mkv_track->validate.TrickTrackFlag) {
            track->TrickTrackFlag = mkv_track->TrickTrackFlag;
        }
        bmkv_file_parser_process_encoding_info(mkv_track, &encoding_info);
        track->unsupported = !encoding_info.supported;
        track->ContentCompAlgo_valid = encoding_info.ContentCompAlgo_valid;
        track->ContentCompAlgo = encoding_info.ContentCompAlgo;
        if(mkv_track->validate.Video) {
            track->media.type = bmedia_track_type_video;
            track->media.info.video.codec = bvideo_codec_unknown;
            if(mkv_track->Video.nelems>0) {
                const bmkv_TrackEntryVideo *video = &BMKV_TABLE_ELEM(mkv_track->Video,bmkv_TrackEntryVideo, 0);
                if(video->validate.PixelWidth) {
                    track->media.info.video.width = video->PixelWidth;
                }
                if(video->validate.PixelHeight) {
                    track->media.info.video.height = video->PixelHeight;
                }
                if(video->validate.DisplayWidth) {
                    track->DisplayWidth = video->DisplayWidth;
                }
                if(video->validate.DisplayHeight) {
                    track->DisplayHeight = video->DisplayHeight;
                }
                track->DefaultDuration = mkv_track->DefaultDuration;
                if(bmkv_IsTrackVideoAvc(mkv_track)) {
                    track->media.info.video.codec = bvideo_codec_h264;
                    /* V_MPEG4/ISO/AVC ID http://haali.cs.msu.ru/mkv/codecs.pdf */
                    if(mkv_track->validate.CodecPrivate && mkv_track->CodecPrivate.data && mkv_track->CodecPrivate.data_len>=8) {
                        int word;
                        batom_vec vec;
                        batom_cursor cursor;
                        unsigned j;

                        batom_vec_init(&vec, mkv_track->CodecPrivate.data,  mkv_track->CodecPrivate.data_len);
                        batom_cursor_from_vec(&cursor, &vec, 1);
                        batom_cursor_skip(&cursor, 1/*Reserved*/+1/*Profile*/+1/*Reserved*/+1/*Level*/+1/*nalu_len*/);
                        word = batom_cursor_next(&cursor);
                        if(word!=BATOM_EOF) {
                            bmedia_probe_h264_video *h264_info = (bmedia_probe_h264_video *)&track->media.info.video.codec_specific;
                            for(j=0;j<B_GET_BITS(word,4,0);j++) {
                                unsigned size=batom_cursor_uint16_be(&cursor);
                                if(BATOM_IS_EOF(&cursor)||size==0) {
                                    break;
                                }
                                b_h264_video_sps_parse(&h264_info->sps, cursor.cursor+1, size-1);
                                if(h264_info->sps.valid) {
                                    break;
                                }
                            }
                        }
                    }
                } else if(bmkv_IsTrackVideoHevc(mkv_track)) {
                    track->media.info.video.codec = bvideo_codec_h265;
                } else if(bmkv_IsTrackVideoMpeg1(mkv_track)) {
                    track->media.info.video.codec = bvideo_codec_mpeg1;
                } else if(bmkv_IsTrackVideoMpeg2(mkv_track)) {
                    track->media.info.video.codec = bvideo_codec_mpeg2;
                } else if(bmkv_IsTrackVideoMpeg4Asp(mkv_track)) {
                    track->media.info.video.codec = bvideo_codec_mpeg4_part2;
                } else if(bmkv_IsTrackVideoVp8(mkv_track)) {
                    track->media.info.video.codec = bvideo_codec_vp8;
                } else if(bmkv_IsTrackVideoVp9(mkv_track)) {
                    track->media.info.video.codec = bvideo_codec_vp9;
                } else if (bmkv_IsTrackVideoVFW(mkv_track)) {
                    batom_vec vec;
                    batom_cursor cursor;
                    bmedia_bitmapinfo bitmap_info;

                    track->media.info.video.timestamp_order = bmedia_timestamp_order_decode;
                    batom_vec_init(&vec, mkv_track->CodecPrivate.data,  mkv_track->CodecPrivate.data_len);
                    batom_cursor_from_vec(&cursor, &vec, 1);
                    if (bmedia_read_bitmapinfo(&bitmap_info, &cursor)) {
                        if(BMEDIA_FOURCC_DIVX5_CODEC(bitmap_info.biCompression) || BMEDIA_FOURCC_XVID_CODEC(bitmap_info.biCompression) || BMEDIA_FOURCC_3IVX_CODEC(bitmap_info.biCompression) ) {
                            track->media.info.video.codec = bvideo_codec_mpeg4_part2;
                        } else if(BMEDIA_FOURCC_VC1AP_CODEC(bitmap_info.biCompression)) {
                            track->media.info.video.codec = bvideo_codec_vc1;
                        } else if(BMEDIA_FOURCC_H264_CODEC(bitmap_info.biCompression)){
                            track->media.info.video.codec = bvideo_codec_h264;
                        } else if(BMEDIA_FOURCC_DIVX3_CODEC(bitmap_info.biCompression)) {
                            track->media.info.video.codec = bvideo_codec_divx_311;
                        }
                    }
                }
            }
        } else if(mkv_track->validate.Audio) {
            const bmkv_TrackEntryAudio *audio;
            bmedia_info_aac info_aac;

            track->media.type = bmedia_track_type_audio;
            track->media.info.audio.codec = bvideo_codec_unknown;
            if(mkv_track->Audio.nelems>0) {
                audio = &BMKV_TABLE_ELEM(mkv_track->Audio,bmkv_TrackEntryAudio, 0);
                if(audio->validate.SamplingFrequency) {
                    track->media.info.audio.sample_rate = audio->SamplingFrequency;
                }
                if(audio->validate.Channels) {
                    track->media.info.audio.channel_count = audio->Channels;
                }
                if(audio->validate.BitDepth) {
                    track->media.info.audio.sample_size = audio->BitDepth;
                }
                if(bmkv_IsTrackAudioMkvAac(mkv_track) || bmkv_IsTrackAudioAac(mkv_track,&info_aac)) {
                    bmedia_info_aac parsed_aac;
                    batom_vec vec;
                    batom_cursor cursor;

                    batom_vec_init(&vec, mkv_track->CodecPrivate.data,  mkv_track->CodecPrivate.data_len);
                    batom_cursor_from_vec(&cursor, &vec, 1);

                    if (bmedia_info_probe_aac_info(&cursor,&parsed_aac)) {
                        /*  We need to use the probed sample rate in case of AAC-HE */
                        track->media.info.audio.sample_rate = bmedia_info_aac_sampling_frequency_from_index(parsed_aac.sampling_frequency_index);
                        track->media.info.audio.codec = parsed_aac.profile == 5 ? baudio_format_aac_plus_adts: baudio_format_aac;
                        if (parsed_aac.profile <= 5)
                        {
                            ((bmedia_probe_aac_audio*)&track->media.info.audio.codec_specific)->profile =
                                (bmedia_probe_aac_profile)parsed_aac.profile;
                        }
                        else
                        {
                            ((bmedia_probe_aac_audio*)&track->media.info.audio.codec_specific)->profile =
                                bmedia_probe_aac_profile_unknown;
                        }
                    } else {
                        track->media.info.audio.codec = baudio_format_aac;
                        ((bmedia_probe_aac_audio*)&track->media.info.audio.codec_specific)->profile =
                            bmedia_probe_aac_profile_lc;
                    }
                } else if(bmkv_IsTrackAudioMpeg1Layer3(mkv_track)) {
                    track->media.info.audio.codec = baudio_format_mp3;
                } else if(bmkv_IsTrackAudioMpeg1Layer1_2(mkv_track)) {
                    track->media.info.audio.codec = baudio_format_mpeg;
                } else if(bmkv_IsTrackAudioAc3(mkv_track)) {
                    track->media.info.audio.codec = baudio_format_ac3;
                } else if(bmkv_IsTrackAudioAc3Plus(mkv_track)) {
                    track->media.info.audio.codec = baudio_format_ac3_plus;
                } else if(bmkv_IsTrackAudioDts(mkv_track)) {
                    track->media.info.audio.codec = baudio_format_dts;
                } else if(bmkv_IsTrackAudioVorbis(mkv_track)) {
                    track->media.info.audio.codec = baudio_format_vorbis;
                } else if (bmkv_IsTrackAudioACM(mkv_track)) {
                    batom_vec vec;
                    batom_cursor cursor;
                    bmedia_waveformatex wf;

                    BATOM_VEC_INIT(&vec, mkv_track->CodecPrivate.data,  mkv_track->CodecPrivate.data_len);
                    batom_cursor_from_vec(&cursor, &vec, 1);
                    if (bmedia_read_waveformatex(&wf, &cursor)) {
                        if(BMEDIA_WAVFMTEX_AUDIO_MPEG(&wf)) {
                            track->media.info.audio.codec = baudio_format_mpeg;
                        } else if(BMEDIA_WAVFMTEX_AUDIO_MP3(&wf)) {
                            track->media.info.audio.codec = baudio_format_mp3;
                        } else if(BMEDIA_WAVFMTEX_AUDIO_DTS(&wf)) {
                            track->media.info.audio.codec = baudio_format_dts;
                        }
                    }
                }
            }
        } else {
            track->media.type = bmedia_track_type_other;
        }
    }

    for(i=0;i<probe->mkv_file.trackData.TracksData.nelems;i++) {
        const bmkv_TracksData *mkv_tracksdata = &BMKV_TABLE_ELEM(probe->mkv_file.trackData.TracksData,bmkv_TracksData,i);
        if(mkv_tracksdata->validate.TracksDataVersion) {
            stream->TracksDataVersion = mkv_tracksdata->TracksDataVersion;
        }
        if(mkv_tracksdata->validate.TracksDataSize) {
            stream->TracksDataSize = mkv_tracksdata->TracksDataSize;
        }
        if(mkv_tracksdata->validate.TracksDataPayload) {
            stream->TracksDataPayload = BKNI_Malloc(mkv_tracksdata->TracksDataPayload.data_len);
            BKNI_Memcpy((void*)stream->TracksDataPayload, mkv_tracksdata->TracksDataPayload.data, mkv_tracksdata->TracksDataPayload.data_len);
        }
    }
    return ;
}


static const BMKV_PARSER_BEGIN(bmkv_AttachedFile_parser)
    BMKV_PARSER_FIELD_UTF8(bmkv_AttachedFile, 0x467E, FileDescription)
    BMKV_PARSER_FIELD_UTF8(bmkv_AttachedFile, 0x466E, FileName)
    BMKV_PARSER_FIELD_UTF8(bmkv_AttachedFile, 0x4660, FileMimeType)
    BMKV_PARSER_FIELD_STOP(bmkv_AttachedFile, 0x465C, FileData)
    BMKV_PARSER_FIELD_UNSIGNED64(bmkv_AttachedFile, 0x46AE, FileUID)
BMKV_PARSER_END(bmkv_AttachedFile_parser);

static void
bmkv_AttachedFile_init(bmkv_AttachedFile *file)
{
	BKNI_Memset(file, 0, sizeof(*file));
	return;
}

static const BMKV_PARSER_BEGIN(bmkv_ChapterTrack_parser)
     BMKV_PARSER_FIELD_UNSIGNED(bmkv_ChapterTrack, 0x89, ChapterTrackNumber)
BMKV_PARSER_END(bmkv_ChapterTrack_parser);

static void
bmkv_ChapterTrack_init(void *data)
{
    bmkv_ChapterTrack *chapter_track = data;
    BKNI_Memset(chapter_track, 0, sizeof(*chapter_track));
	return;
}

static const bmkv_parser_desc bmkv_ChapterTrack_desc = {
    "ChapterTrack",
    bmkv_ChapterTrack_parser,
    sizeof(bmkv_ChapterTrack),
    bmkv_ChapterTrack_init
};

static const BMKV_PARSER_BEGIN(bmkv_ChapterDisplay_parser)
     BMKV_PARSER_FIELD_UTF8(bmkv_ChapterDisplay, 0x85, ChapString)
     BMKV_PARSER_FIELD_STRING(bmkv_ChapterDisplay, 0x437C, ChapLanguage)
     BMKV_PARSER_FIELD_STRING(bmkv_ChapterDisplay, 0x437E, ChapCountry)
BMKV_PARSER_END(bmkv_ChapterDisplay_parser);

static void
bmkv_ChapterDisplay_init(void *data)
{
    bmkv_ChapterDisplay *chapter_display = data;
    BKNI_Memset(chapter_display, 0, sizeof(*chapter_display));
	return;
}

static const bmkv_parser_desc bmkv_ChapterDisplay_desc = {
    "ChapterDisplay",
    bmkv_ChapterDisplay_parser,
    sizeof(bmkv_ChapterDisplay),
    bmkv_ChapterDisplay_init
};

static const BMKV_PARSER_BEGIN(bmkv_ChapterAtom_parser)
     BMKV_PARSER_FIELD_UNSIGNED64(bmkv_ChapterAtom, 0x73C4, ChapterUID)
     BMKV_PARSER_FIELD_UNSIGNED64(bmkv_ChapterAtom, 0x91, ChapterTimeStart)
     BMKV_PARSER_FIELD_UNSIGNED64(bmkv_ChapterAtom, 0x92, ChapterTimeEnd)
     BMKV_PARSER_FIELD_UNSIGNED(bmkv_ChapterAtom, 0x98, ChapterFlagHidden)
     BMKV_PARSER_FIELD_UNSIGNED(bmkv_ChapterAtom, 0x4598, ChapterFlagEnabled)
     BMKV_PARSER_FIELD_DATA(bmkv_ChapterAtom, 0x6E67, ChapterSegmentUID)
     BMKV_PARSER_FIELD_DATA(bmkv_ChapterAtom, 0x6EBC, ChapterSegmentEditionUID)
     BMKV_PARSER_FIELD_UNSIGNED(bmkv_ChapterAtom, 0x63C3, ChapterPhysicalEquiv)
     BMKV_PARSER_FIELD_TABLE(bmkv_ChapterAtom, 0x8F, ChapterTrack, bmkv_ChapterTrack_desc)
     BMKV_PARSER_FIELD_TABLE(bmkv_ChapterAtom, 0x80, ChapterDisplay, bmkv_ChapterDisplay_desc)
BMKV_PARSER_END(bmkv_ChapterAtom_parser);

static void
bmkv_ChapterAtom_init(void *data)
{
    bmkv_ChapterAtom *chapter_atom = data;
    BKNI_Memset(chapter_atom, 0, sizeof(*chapter_atom));
	return;
}

static const bmkv_parser_desc bmkv_ChapterAtom_desc = {
    "ChapterAtom",
    bmkv_ChapterAtom_parser,
    sizeof(bmkv_ChapterAtom),
    bmkv_ChapterAtom_init
};

static const BMKV_PARSER_BEGIN(bmkv_EditionEntry_parser)
     BMKV_PARSER_FIELD_UNSIGNED64(bmkv_EditionEntry, 0x45BC, EditionUID)
     BMKV_PARSER_FIELD_UNSIGNED(bmkv_EditionEntry, 0x45BD, EditionFlagHidden)
     BMKV_PARSER_FIELD_UNSIGNED(bmkv_EditionEntry, 0x45DB, EditionFlagDefault)
     BMKV_PARSER_FIELD_UNSIGNED(bmkv_EditionEntry, 0x45DD, EditionFlagOrdered)
     BMKV_PARSER_FIELD_TABLE(bmkv_EditionEntry,0xB6, ChapterAtom, bmkv_ChapterAtom_desc)
BMKV_PARSER_END(bmkv_EditionEntry_parser);

static void
bmkv_EditionEntry_init(bmkv_EditionEntry *edition_entry)
{
    BKNI_Memset(edition_entry, 0, sizeof(*edition_entry));
	return;
}

/* maximum size of the element head (id + size) */
#define B_MKV_PROBE_MAX_ELEMENT_HEAD B_MKV_MAX_ELEMENT_HEAD
/* size of read transaction for the data stream */
#define B_MKV_PROBE_DATA_BLOCK_SIZE (60*1024)

uint8_t *
b_mkv_probe_copy_string(uint8_t *buf, const bmkv_utf8 *utf8)
{
    BKNI_Memcpy(buf, utf8->utf8_data, utf8->utf8_len);
    buf += utf8->utf8_len;
    return buf;
}

static void
b_mkv_probe_parse_chapters(bmkv_probe_t probe, bmkv_probe_stream *stream, const bfile_segment *segment)
{
    bfile_cached_segment_set(&probe->edition.cache, segment->start, segment->len);
    for(;;) {
	    if(bfile_cached_segment_reserve(&probe->edition.cache, B_MKV_PROBE_MAX_ELEMENT_HEAD)) {
		    bmkv_header header;
		    size_t elem_offset;
		    size_t parse_offset;
		    bfile_segment editionentry_data;
		    bmkv_Editions *edition;
		    bmkv_Chapters *chapter;
		    size_t extra_size=0;
		    uint8_t *extra_data;
		    bmkv_ChapterAtom *chapter_atom;

		    unsigned i;
		    if(!bmkv_parse_header(&probe->edition.cache.cursor, &header)) {
			    break;
		    }
		    if(!bfile_cached_segment_reserve(&probe->edition.cache, header.size)) {
			    break;
		    }
		    if(header.id != 0x45B9  ) {
			    batom_cursor_skip(&probe->edition.cache.cursor, header.size);
			    continue;
		    }
		    elem_offset = batom_cursor_pos(&probe->edition.cache.cursor);
		    parse_offset = elem_offset;
		    bmkv_EditionEntry_init(&probe->EditionEntry);
		    bfile_segment_clear(&editionentry_data);
		    if(!bmkv_element_parse(&probe->edition.cache.cursor, header.size-(parse_offset-elem_offset), bmkv_EditionEntry_parser, "EditionEntry", &probe->EditionEntry)) {
			    bmkv_element_shutdown(bmkv_EditionEntry_parser, &probe->EditionEntry);
			    goto err_parser_chapters;
		    }
		    bmkv_element_print(bmkv_EditionEntry_parser, BDBG_eMsg, 0, "EditionEntry", &probe->EditionEntry);

		    edition = BKNI_Malloc(sizeof(*edition));
		    if(!edition) { break; }
		    BKNI_Memset(edition, 0, sizeof(*edition));

		    if(probe->EditionEntry.validate.EditionUID) {
			    edition->EditionUID = probe->EditionEntry.EditionUID;
		    }
		    if(probe->EditionEntry.validate.EditionFlagHidden) {
			    edition->EditionFlagHidden = probe->EditionEntry.EditionFlagHidden;
		    }
		    if(probe->EditionEntry.validate.EditionFlagDefault) {
			    edition->EditionFlagDefault = probe->EditionEntry.EditionFlagDefault;
		    }
		    if(probe->EditionEntry.validate.EditionFlagOrdered) {
			    edition->EditionFlagOrdered = probe->EditionEntry.EditionFlagOrdered;
		    }
            BDBG_MSG(("edition : UID " BDBG_UINT64_FMT " Flags %u:%u:%u", BDBG_UINT64_ARG(edition->EditionUID), edition->EditionFlagHidden, edition->EditionFlagDefault, edition->EditionFlagOrdered));

		    BLST_SQ_INIT(&edition->chapters);

		    chapter_atom = (bmkv_ChapterAtom *)probe->EditionEntry.ChapterAtom.data;

		    for(i=0;i<probe->EditionEntry.ChapterAtom.nelems;i++){
			    bmkv_ChapterDisplay *chapter_display = NULL;
			    if(chapter_atom[i].validate.ChapterDisplay) {
				    chapter_display = (bmkv_ChapterDisplay *)chapter_atom[i].ChapterDisplay.data;
				    extra_size = chapter_display->ChapString.utf8_len + sizeof(chapter_display->ChapLanguage) + sizeof(chapter_display->ChapCountry);
			    }
			    chapter = BKNI_Malloc(sizeof(*chapter)+extra_size);
			    if(!chapter) { break; }
			    BKNI_Memset(chapter, 0, sizeof(*chapter));

			    if(chapter_atom[i].validate.ChapterUID) {
				    chapter->ChapterUID_valid = true;
				    chapter->ChapterUID = chapter_atom[i].ChapterUID;
			    }
			    if(chapter_atom[i].validate.ChapterTimeStart) {
				    chapter->ChapterTimeStart = chapter_atom[i].ChapterTimeStart / 1000000;
			    }
			    if(chapter_atom[i].validate.ChapterTimeEnd) {
				    chapter->ChapterTimeEnd = chapter_atom[i].ChapterTimeEnd / 1000000;
			    }
			    if(chapter_atom[i].validate.ChapterFlagHidden) {
				    chapter->ChapterFlagHidden = chapter_atom[i].ChapterFlagHidden;
			    }
			    if(chapter_atom[i].validate.ChapterFlagEnabled) {
				    chapter->ChapterFlagEnabled = chapter_atom[i].ChapterFlagEnabled;
			    }
			    if(chapter_display) {
					    extra_data =  (uint8_t *)chapter + sizeof(*chapter);
					    if(chapter_display->validate.ChapString){
						    chapter->ChapterDisplay.ChapString = (char*)extra_data;
						    extra_data = b_mkv_probe_copy_string(extra_data, &chapter_display->ChapString);
					    }
					    if(chapter_display->validate.ChapLanguage){
						    chapter->ChapterDisplay.ChapLanguage = (char*)extra_data;
						    BKNI_Memcpy(extra_data, chapter_display->ChapLanguage, sizeof(chapter_display->ChapLanguage));
						    extra_data += sizeof(chapter_display->ChapLanguage);
					    }
					    if(chapter_display->validate.ChapCountry){
						    chapter->ChapterDisplay.ChapCountry = (char*)extra_data;
						    BKNI_Memcpy(extra_data, chapter_display->ChapCountry, sizeof(chapter_display->ChapCountry));
						    extra_data += sizeof(chapter_display->ChapCountry);
					    }
			    }
                BDBG_MSG(("\tchapter: UID " BDBG_UINT64_FMT " Time [%d->%d] Flags %u:%u %s %s %s", BDBG_UINT64_ARG(chapter->ChapterUID), chapter->ChapterTimeStart, chapter->ChapterTimeEnd, chapter->ChapterFlagHidden, chapter->ChapterFlagEnabled, chapter->ChapterDisplay.ChapString, chapter->ChapterDisplay.ChapLanguage, chapter->ChapterDisplay.ChapCountry));
			    BLST_SQ_INSERT_TAIL(&edition->chapters, chapter, link);
		    }
		    BLST_SQ_INSERT_TAIL(&stream->editions, edition, link);
		    bmkv_element_shutdown(bmkv_EditionEntry_parser, &probe->EditionEntry);
		    bfile_cached_segment_seek(&probe->edition.cache, bfile_cached_segment_tell(&probe->edition.cache));
	    } else {
		    break;
	    }
    }
err_parser_chapters:
    return;
}

static void
b_mkv_probe_parse_attachments(bmkv_probe_t probe, bmkv_probe_stream *stream, const bfile_segment *segment)
{
    uint64_t offset;
    bfile_cached_segment_set(&probe->attachment.cache, segment->start, segment->len);
    for(offset=0;;) {
        bfile_cached_segment_seek(&probe->attachment.cache, offset);
	    if(bfile_cached_segment_reserve(&probe->attachment.cache, B_MKV_PROBE_MAX_ELEMENT_HEAD)) {
		    bmkv_header header;
		    size_t elem_offset;
		    size_t parse_offset;
		    bfile_segment fileAttachment_data;
		    bmkv_Attachment *attachment;
		    size_t extra_size;
		    void *extra_data;

		    if(!bmkv_parse_header(&probe->attachment.cache.cursor, &header)) {
			    break;
		    }
		    elem_offset = batom_cursor_pos(&probe->attachment.cache.cursor);
		    if(header.id != 0x61A7  /* AttachedFile */) {
                offset += elem_offset + header.size;
			    continue;
		    }
		    parse_offset = elem_offset;
		    bmkv_AttachedFile_init(&probe->AttachedFile);
		    bfile_segment_clear(&fileAttachment_data);
		    do {
                size_t cursor_pos;

                bfile_cached_segment_reserve(&probe->attachment.cache, 16*1024);
			    probe->AttachedFile.validate.FileData = false;
			    if(!bmkv_element_parse(&probe->attachment.cache.cursor, header.size-(parse_offset-elem_offset), bmkv_AttachedFile_parser, "AttachedFile", &probe->AttachedFile)) {
				    bmkv_element_shutdown(bmkv_AttachedFile_parser, &probe->AttachedFile);
				    goto err_parser_attachedFile;
			    }
			    bmkv_element_print(bmkv_AttachedFile_parser, BDBG_eMsg, 0, "AttachedFile", &probe->AttachedFile);
                cursor_pos = batom_cursor_pos(&probe->attachment.cache.cursor);
			    if(probe->AttachedFile.validate.FileData) {
				    bfile_segment_set(&fileAttachment_data, bfile_cached_segment_tell(&probe->attachment.cache), probe->AttachedFile.FileData);
				    cursor_pos += probe->AttachedFile.FileData;
			    }
                offset += cursor_pos;
                parse_offset += cursor_pos;
                bfile_cached_segment_seek(&probe->attachment.cache, offset);
		    } while((parse_offset-elem_offset)<header.size);
		    extra_size = probe->AttachedFile.FileDescription.utf8_len + probe->AttachedFile.FileMimeType.utf8_len + probe->AttachedFile.FileName.utf8_len;
		    attachment = BKNI_Malloc(sizeof(*attachment)+extra_size);
		    if(!attachment) { break; }
		    BKNI_Memset(attachment, 0, sizeof(*attachment));
		    extra_data =  (uint8_t *)attachment + sizeof(*attachment);
		    if(probe->AttachedFile.validate.FileName) {
			    attachment->FileName = extra_data;
			    extra_data = b_mkv_probe_copy_string(extra_data, &probe->AttachedFile.FileName);
		    }
		    if(probe->AttachedFile.validate.FileDescription) {
			    attachment->FileDescription = extra_data;
			    extra_data = b_mkv_probe_copy_string(extra_data, &probe->AttachedFile.FileDescription);
		    }
		    if(probe->AttachedFile.validate.FileMimeType) {
			    attachment->FileMimeType = extra_data;
			    extra_data = b_mkv_probe_copy_string(extra_data, &probe->AttachedFile.FileMimeType);
			    BSTD_UNUSED(extra_data); /* suppress coverity warning */
		    }
		    if(fileAttachment_data.len>0) {
			    attachment->FileData_valid = true;
			    attachment->FileData_size = fileAttachment_data.len;
			    attachment->FileData_offset = fileAttachment_data.start + segment->start;
		    }
		    if(probe->AttachedFile.validate.FileUID) {
			    attachment->FileUID_valid = true;
			    attachment->FileUID = probe->AttachedFile.FileUID;
		    }
		    BDBG_MSG(("attachment: %#x %s['%s':%s] at %u:%u", (unsigned)attachment->FileUID, attachment->FileName, attachment->FileDescription, attachment->FileMimeType, (unsigned)attachment->FileData_offset, (unsigned)attachment->FileData_size));
		    BLST_SQ_INSERT_TAIL(&stream->attachments, attachment, link);
		    bmkv_element_shutdown(bmkv_AttachedFile_parser, &probe->AttachedFile);
		    bfile_cached_segment_seek(&probe->attachment.cache, bfile_cached_segment_tell(&probe->attachment.cache)); /* reset old data */
	    } else {
		    break;
	    }
    }
err_parser_attachedFile:
    return;
}

static void
b_mkv_probe_next_volume(bmkv_probe_t probe, bmkv_probe_stream *stream, bfile_buffer_t file_buf)
{
    batom_t atom;
    bfile_buffer_result result;
    batom_cursor cursor;
    bmkv_header header;
    uint64_t offset;

    offset = probe->mkv_file.segment.start + probe->mkv_file.segment.len;
   	atom = bfile_buffer_read(file_buf, offset, B_MKV_PROBE_MAX_ELEMENT_HEAD, &result);
    if(atom==NULL) {
        return;
    }
    batom_dump(atom, "data");
    batom_cursor_from_atom(&cursor, atom);
    if(bmkv_parse_header(&cursor, &header) && header.id==BMKV_EBML_ID) {
        BDBG_MSG(("b_mkv_probe_next_volume:%#lx found next volume at %lu", (unsigned long)probe, (unsigned long)offset));
        stream->next_volume.next_volume_offset_valid = true;
        stream->next_volume.next_volume_offset = offset;
    }
    batom_release(atom);
    return;
}

static const bmedia_probe_stream *
b_mkv_probe_parse(bmedia_probe_base_t probe_, bfile_buffer_t buf, batom_pipe_t pipe, const bmedia_probe_parser_config *config)
{
	bmkv_probe_t probe = (bmkv_probe_t)probe_;
    int rc;
	bmkv_probe_stream *stream = NULL;

	BDBG_OBJECT_ASSERT(probe, bmkv_probe_t);
    BSTD_UNUSED(config);


    probe->mkv_file.config.attachment = config->stream_specific.mkv.probe_attachments;
    rc = bmkv_file_parser_parse(&probe->mkv_file, buf, pipe);

    if(rc!=0 || !probe->mkv_file.validate.segment_info) {
        goto err_stream;
    }

    stream = BKNI_Malloc(sizeof(*stream));
    if(!stream) {
        BDBG_ERR(("b_mkv_probe_parse: %p can't allocate %u bytes", (void *)probe, (unsigned)sizeof(*stream)));
        goto err_alloc;
    }
    BKNI_Memset(stream, 0, sizeof(*stream));
    BDBG_CASSERT(sizeof(stream->docType)==sizeof(probe->mkv_file.docType));
    BKNI_Memcpy(stream->docType, probe->mkv_file.docType, sizeof(stream->docType));

    rc = bfile_cached_segment_init(&probe->attachment.cache, buf, probe->factory, B_MKV_PROBE_DATA_BLOCK_SIZE);
    if(rc<0) {
        goto err_cached_segment;
    }
    rc = bfile_cached_segment_init(&probe->edition.cache, buf, probe->factory, B_MKV_PROBE_DATA_BLOCK_SIZE);
    if(rc<0) {
        goto err_cached_segment;
    }

    bmedia_probe_stream_init(&stream->media, bstream_mpeg_type_mkv);
    stream->media.index = bmedia_probe_index_required;
    stream->next_volume.next_volume_offset_valid = false;
    BLST_SQ_INIT(&stream->attachments);
    BLST_SQ_INIT(&stream->editions);
    if(probe->mkv_file.segment_info.validate.Duration && probe->mkv_file.segment_info.validate.TimecodeScale)  {
        stream->media.duration = (probe->mkv_file.segment_info.Duration * probe->mkv_file.segment_info.TimecodeScale) / 1000000 ;
    }
    b_mkv_probe_add_tracks(probe, stream);
    if(probe->mkv_file.attachment.len>0) {
        b_mkv_probe_parse_attachments(probe, stream, &probe->mkv_file.attachment);
    }
    if(probe->mkv_file.chapters.len>0) {
        b_mkv_probe_parse_chapters(probe, stream, &probe->mkv_file.chapters);
    }
    if(config->stream_specific.mkv.probe_next_volume) {
        b_mkv_probe_next_volume(probe, stream, buf);
    }

    bfile_cached_segment_shutdown(&probe->attachment.cache);
    bfile_cached_segment_shutdown(&probe->edition.cache);
    bmkv_file_parser_release(&probe->mkv_file);
    return &stream->media;

err_cached_segment:
    BKNI_Free(stream);
err_alloc:
err_stream:
    bmkv_file_parser_release(&probe->mkv_file);
    return NULL;
}

static void
b_mkv_probe_stream_free(bmedia_probe_base_t probe_, bmedia_probe_stream *probe_stream)
{
	bmkv_probe_t probe = (bmkv_probe_t)probe_;
	bmkv_probe_track *track;
	bmkv_probe_stream *stream = (bmkv_probe_stream *)probe_stream;
	bmkv_Attachment *attachment;
	bmkv_Editions *edition;
	bmkv_Chapters *chapter;

	BDBG_OBJECT_ASSERT(probe, bmkv_probe_t);
	BSTD_UNUSED(probe);

	BDBG_ASSERT(stream);

	while(NULL!=(track=(bmkv_probe_track*)BLST_SQ_FIRST(&stream->media.tracks))) {
		BLST_SQ_REMOVE_HEAD(&stream->media.tracks, link);
		BKNI_Free(track);
	}
	while(NULL!=(attachment=BLST_SQ_FIRST(&stream->attachments))) {
		BLST_SQ_REMOVE_HEAD(&stream->attachments, link);
		BKNI_Free(attachment);
	}
	while(NULL!=(edition=BLST_SQ_FIRST(&stream->editions))) {
		BLST_SQ_REMOVE_HEAD(&stream->editions, link);
		while(NULL!=(chapter=BLST_SQ_FIRST(&edition->chapters))) {
			BLST_SQ_REMOVE_HEAD(&edition->chapters, link);
			BKNI_Free(chapter);
		}
		BKNI_Free(edition);
	}
	if(stream->TracksDataPayload){
		BKNI_Free((void*)stream->TracksDataPayload);
	}
	BKNI_Free(stream);
	return;
}


const bmedia_probe_format_desc bmkv_probe = {
	bstream_mpeg_type_mkv,
	b_mkv_ext, /* ext_list */
	256, /* EBML element shall be smaller then that */
	b_mkv_probe_header_match, /* header_match */
	b_mkv_probe_create, /* create */
	b_mkv_probe_destroy, /* destroy */
	b_mkv_probe_parse, /* parse */
	b_mkv_probe_stream_free /* stream free */
};

