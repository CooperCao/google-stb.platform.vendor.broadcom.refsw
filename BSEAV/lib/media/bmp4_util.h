/***************************************************************************
 *  Copyright (C) 2007-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * MP4 container parser library
 *
 ***************************************************************************/
#ifndef _BMP4_UTIL_H__
#define _BMP4_UTIL_H__

#include "bmpeg4_util.h"

#ifdef __cplusplus
extern "C"
{
#endif



#define BMP4_TYPE(c1,c2,c3,c4) (((uint32_t)c4) | ((uint32_t)c3<<8) | ((uint32_t)c2<<16) | ((uint32_t)c1<<24))
#define B_MP4_TYPE_FORMAT "'%c%c%c%c'"
#define B_MP4_TYPE_ARG(f) (((f)>>24)&0xFF), (((f)>>16)&0xFF), (((f)>>8)&0xFF), ((f)&0xFF)
#define BMP4_TYPE_BEGIN BMP4_TYPE(0,0,0,0)
#define BMP4_TYPE_END BMP4_TYPE(255,255,255,255)


#define BMP4_BOX_MAX_SIZE  (4+4+8)

typedef struct bmp4_box {
	uint64_t size;
	uint32_t type;
} bmp4_box;

#define BMP4_EXTENDED BMP4_TYPE('u','u','i','d')
typedef struct bmp4_box_extended {
	uint8_t usertype[16];
} bmp4_box_extended;

#define BMP4_FULLBOX_SIZE	(4)
typedef struct bmp4_fullbox {
	uint8_t version;
	uint32_t flags;
} bmp4_fullbox;


#define BMP4_FILETYPEBOX	BMP4_TYPE('f','t','y','p')
typedef struct bmp4_filetypebox {
	uint32_t major_brand;
	uint32_t minor_version;
	uint32_t compatible_brands[16];
	size_t ncompatible_brands;
} bmp4_filetypebox;

#define BMP4_MOVIE	BMP4_TYPE('m','o','o','v')
#define BMP4_MOVIEHEADER	BMP4_TYPE('m','v','h','d')
typedef struct bmp4_movieheaderbox {
	bmp4_fullbox fullbox;
	uint64_t creation_time;
	uint64_t modification_time;
	uint32_t timescale;
	uint64_t duration;
	uint32_t rate;
	uint16_t volume;
	uint32_t matrix[9];
	uint32_t next_track_ID;
} bmp4_movieheaderbox;

#define BMP4_TRACK	BMP4_TYPE('t','r','a','k')
#define BMP4_TRACKHEADER BMP4_TYPE('t','k','h','d')
typedef struct bmp4_trackheaderbox {
	bmp4_fullbox fullbox;
	uint64_t creation_time;
	uint64_t modification_time;
	uint32_t track_ID;
	uint64_t duration;
	uint16_t layer;
	uint16_t alternate_group;
	uint16_t volume;
	uint32_t matrix[9];
	uint32_t width;
	uint32_t height;
} bmp4_trackheaderbox;

#define BMP4_MEDIA 	BMP4_TYPE('m','d','i','a')
#define BMP4_MEDIAHEADER BMP4_TYPE('m','d','h','d')
typedef struct bmp4_mediaheaderbox {
	bmp4_fullbox fullbox;
	uint64_t creation_time;
	uint64_t modification_time;
	uint32_t timescale;
	uint64_t duration;
    char    language[4]; /* NULL terminated ISO-639-2/T language code */
} bmp4_mediaheaderbox;

#define BMP4_HANDLER	BMP4_TYPE('h','d','l','r')
typedef struct bmp4_handlerbox {
	bmp4_fullbox fullbox;
	uint32_t handler_type;
	char name[32];
} bmp4_handlerbox;

#define BMP4_SAMPLESIZE 	BMP4_TYPE('s','t','s','z')
#define BMP4_COMPACTSAMPLESIZE	BMP4_TYPE('s','t','z','2')
typedef struct bmp4_sample_size_header {
	bmp4_fullbox fullbox;
  	uint32_t default_sample_size;
    uint32_t sample_count;
} bmp4_sample_size_header;

typedef struct bmp4_compact_sample_size_header {
	bmp4_fullbox fullbox;
    uint8_t field_size;
    uint32_t sample_count;
} bmp4_compact_sample_size_header;

#define BMP4_MOVIE_FRAGMENT	BMP4_TYPE('m','o','o','f')
#define BMP4_MOVIE_DATA     BMP4_TYPE('m','d','a','t')


#define BMP4_HANDLER_AUDIO	BMP4_TYPE('s','o','u','n')
#define BMP4_HANDLER_VISUAL	BMP4_TYPE('v','i','d','e')

#define BMP4_MEDIAINFORMATION	BMP4_TYPE('m','i','n','f')

#define BMP4_SAMPLETABLE	BMP4_TYPE('s','t','b','l')

#define BMP4_SAMPLEDESCRIPTION	BMP4_TYPE('s','t','s','d')


typedef struct bmp4_visualsampleentry {
    uint16_t width;
    uint16_t height;
    uint32_t horizresolution;
    uint32_t vertresolution;
    uint16_t frame_count;
    char    compressorname[32];
    uint16_t depth;
} bmp4_visualsampleentry;

typedef struct bmp4_audiosampleentry {
    uint16_t version;
	uint16_t channelcount;
	uint16_t samplesize;
	uint32_t samplerate;
} bmp4_audiosampleentry;

typedef enum bmp4_sample_type {
    bmp4_sample_type_avc,  /* AVC, H.264 video */
    bmp4_sample_type_mp4a, /* AAC, Mpeg4 audio */
    bmp4_sample_type_mp4v, /* Mpeg4-Part2 video */
    bmp4_sample_type_s263, /* H.263 video */
    bmp4_sample_type_ac3,  /* AC3 audio */
    bmp4_sample_type_eac3, /* E-AC3 audio */
    bmp4_sample_type_ac4,  /* AC4 audio */
    bmp4_sample_type_samr, /* AMR narrow-band speech audio */
    bmp4_sample_type_sawb, /* AMR wide-band speech audio */
    bmp4_sample_type_sawp, /* Extended AMR wide-band speech audio */
    bmp4_sample_type_drmi, /* DRM'ed video stream */
    bmp4_sample_type_drms, /* DRM'ed audio stream */
    bmp4_sample_type_mpg,  /* MPEG audio */
    bmp4_sample_type_qt_ima_adpcm,  /* DVI/Intel IMAADPCM-ACM */
    bmp4_sample_type_qt_ima4_adpcm,  /* IMA 4:1 ADPCM */
    bmp4_sample_type_mjpeg,/* MJPEG */
    bmp4_sample_type_twos, /* PCM 16-bit signed, big or little endian */
    bmp4_sample_type_dts,  /* variations of DTS audio */
    bmp4_sample_type_hevc, /* HEVC, H.265 video */
    bmp4_sample_type_als,  /* MPEG-4 ALS audio */
    bmp4_sample_type_mp3,  /* QuickTime MPEG-1 Layer 3 audio */
    bmp4_sample_type_unknown /* unknown sample */
} bmp4_sample_type;

#define BMP4_SAMPLE_AVC BMP4_TYPE('a','v','c','1')
#define B_MP4_AVC_MAX_SETS  4
typedef struct bmp4_parameter_data {
    size_t len;
    void *data;
} bmp4_parameter_data;

typedef struct bmp4_sample_avc {
    bmp4_visualsampleentry visual;
    bmedia_h264_meta meta;
    uint8_t data[1]; /* pointer to data, variable size */
} bmp4_sample_avc;

#define BMP4_SAMPLE_HVC1     BMP4_TYPE('h','v','c','1')
#define BMP4_SAMPLE_HEV1     BMP4_TYPE('h','e','v','1')
typedef struct bmp4_sample_hevc {
    bmp4_visualsampleentry visual;
    bmedia_h265_meta meta;
} bmp4_sample_hevc;

#define BMP4_SAMPLE_MP4A    BMP4_TYPE('m','p','4','a')
typedef struct bmp4_sample_mp4a {
    bmp4_audiosampleentry audio;
    bmpeg4_es_descriptor mpeg4;
} bmp4_sample_mp4a;

#define BMP4_SAMPLE_MP4V	BMP4_TYPE('m','p','4','v')
typedef struct bmp4_sample_mp4v {
	bmp4_visualsampleentry visual;
	bmpeg4_es_descriptor mpeg4;
} bmp4_sample_mp4v;

#define BMP4_SAMPLE_S263	BMP4_TYPE('s','2','6','3')
#define BMP4_SAMPLE_H263	BMP4_TYPE('h','2','6','3')

typedef struct bmp4_sample_s263 {
    bmp4_visualsampleentry visual;
} bmp4_sample_s263;

#define BMP4_SAMPLE_AC3	BMP4_TYPE('a','c','-','3')
#define BMP4_SAMPLE_EAC3 BMP4_TYPE('e','c','-','3')
typedef struct bmp4_sample_ac3 {
	bmp4_audiosampleentry audio;
} bmp4_sample_ac3;

#define BMP4_SAMPLE_AC4 BMP4_TYPE('a','c','-','4')
typedef struct bmp4_sample_ac4 {
    bmp4_audiosampleentry audio;
} bmp4_sample_ac4;

#define BMP4_SAMPLE_SAMR	BMP4_TYPE('s','a','m','r')
#define BMP4_SAMPLE_SAWB	BMP4_TYPE('s','a','w','b')
#define BMP4_SAMPLE_SAWP	BMP4_TYPE('s','a','w','p')

typedef struct bmp4_sample_amr {
    bmp4_audiosampleentry audio;
    uint32_t vendor;
    uint8_t  decoder_version;
    uint16_t mode_set;
    uint8_t  mode_change_period;
    uint8_t  frames_per_sample;
} bmp4_sample_amr;

#define BMP4_SAMPLE_QT_IMA_ADPCM   BMP4_TYPE('m','s',0x00,0x11)
typedef struct bmp4_sample_ms {
    bmp4_audiosampleentry audio;
    bmedia_waveformatex waveformat;
} bmp4_sample_ms;

typedef struct bmp4_sample_sound_frame_info {
    uint32_t samplesPerPacket;
    uint32_t bytesPerFrame;
} bmp4_sample_sound_frame_info;

#define BMP4_SAMPLE_QT_IMA4_ADPCM    BMP4_TYPE('i','m','a','4')
typedef struct bmp4_sample_ima4 {
    bmp4_audiosampleentry audio;
    bmp4_sample_sound_frame_info frame_info;
} bmp4_sample_ima4;

#define BMP4_SAMPLE_JPEG BMP4_TYPE('j','p','e','g')
#define BMP4_SAMPLE_MJPA  BMP4_TYPE('m','j','p','a')
#define BMP4_SAMPLE_MJPB  BMP4_TYPE('m','j','p','b')
typedef struct bmp4_sample_mjpeg {
	  bmp4_visualsampleentry visual;
      uint32_t type; /* type BMP4_SAMPLE_JPEG, BMP4_SAMPLE_MJPA or BMP4_SAMPLE_MJPB */
} bmp4_sample_mjpeg;

#define BMP4_SAMPLE_TWOS    BMP4_TYPE('t','w','o','s')
#define BMP4_SAMPLE_SOWT    BMP4_TYPE('s','o','w','t')
typedef struct bmp4_sample_twos {
    bmp4_audiosampleentry audio;
    uint32_t type; /* type BMP4_SAMPLE_TWOS or BMP4_SAMPLE_SOWT */
    bmp4_sample_sound_frame_info frame_info;
} bmp4_sample_twos;

#define BMP4_SAMPLE_DTSC BMP4_TYPE('d','t','s','c')
#define BMP4_SAMPLE_DTSH BMP4_TYPE('d','t','s','h')
#define BMP4_SAMPLE_DTSL BMP4_TYPE('d','t','s','l')
#define BMP4_SAMPLE_DTSE BMP4_TYPE('d','t','s','e')
typedef struct bmp4_sample_dts {
    bmp4_audiosampleentry audio;
    uint32_t type;
} bmp4_sample_dts;

#define BMP4_SAMPLE_MP3 BMP4_TYPE('.','m','p','3')
typedef struct bmp4_sample_mp3 {
    bmp4_audiosampleentry audio;
} bmp4_sample_mp3;


typedef struct bmp4_sample_codecprivate {
    unsigned offset;
    bmp4_box box;
} bmp4_sample_codecprivate;

typedef struct bmp4_sampleentry {
    uint16_t data_reference_index;
    bool encrypted;
    uint32_t type;
    bmp4_sample_type sample_type;
    bmp4_sample_codecprivate codecprivate;
    size_t protection_scheme_information_size;
    uint8_t protection_scheme_information[128];
    union {
        bmp4_sample_avc avc;
        bmp4_sample_mp4a mp4a;
        bmp4_sample_mp4v mp4v;
        bmp4_sample_s263 s263;
        bmp4_sample_ac3 ac3;
        bmp4_sample_ac4 ac4;
        bmp4_sample_amr amr;
        bmp4_sample_ms ms;
        bmp4_sample_mjpeg mjpeg;
        bmp4_sample_twos twos;
        bmp4_sample_dts dts;
        bmp4_sample_ima4 ima4;
        bmp4_sample_hevc hevc;
        bmp4_sample_mp3 mp3;
    } codec; /* must be last entry in the structure */
} bmp4_sampleentry;

#define B_MP4_MAX_SAMPLE_ENTRIES    2
typedef struct bmp4_sample_info {
    bmp4_fullbox fullbox;
    uint32_t entry_count;
    bmp4_sampleentry *entries[B_MP4_MAX_SAMPLE_ENTRIES];
} bmp4_sample_info;


#define BMP4_DECODETIMETOSAMPLE    BMP4_TYPE('s','t','t','s')
#define BMP4_COMPOSITIONTIMETOSAMPLE BMP4_TYPE('c','t','t','s')
#define BMP4_SAMPLETOCHINK    BMP4_TYPE('s','t','s','c')
#define BMP4_CHUNKOFFSET    BMP4_TYPE('s','t','c','o')
#define BMP4_CHUNKLARGEOFFSET BMP4_TYPE('c','o','6','4')
#define BMP4_SYNCSAMPLE    BMP4_TYPE('s','t','s','s')
#define BMP4_SAMPLE_DRMS    BMP4_TYPE('d','r','m','s')
#define BMP4_SAMPLE_DRMI    BMP4_TYPE('d','r','m','i')

#define BMP4_EDIT           BMP4_TYPE('e','d','t','s')

#define BMP4_MOVIE_EXTENDS  BMP4_TYPE('m','v','e','x')
#define BMP4_TRACK_EXTENDS  BMP4_TYPE('t','r','e','x')

typedef struct bmp4_trackextendsbox {
    uint32_t track_ID;
    uint32_t default_sample_description_index;
    uint32_t default_sample_duration;
    uint32_t default_sample_size;
    uint32_t default_sample_flags;
} bmp4_trackextendsbox;

#define BMP4_MOVIE_FRAGMENT_HEADER  BMP4_TYPE('m','f','h','d')
typedef struct bmp4_movie_fragment_header {
	bmp4_fullbox fullbox;
	uint32_t sequence_number;
} bmp4_movie_fragment_header;

#define BMP4_TRACK_FRAGMENT BMP4_TYPE('t','r','a','f')
#define BMP4_TRACK_FRAGMENT_HEADER BMP4_TYPE('t','f','h','d')
#define BMP4_TRACK_FRAGMENT_HEADER_MAX_SIZE (BMP4_FULLBOX_SIZE+4+8+4+4+4+4)
typedef struct bmp4_track_fragment_header {
	bmp4_fullbox fullbox;
    uint32_t track_ID;
    struct {
        bool base_data_offset;
        bool sample_description_index;
        bool default_sample_duration;
        bool default_sample_size;
        bool default_sample_flags;
    } validate;
    uint64_t base_data_offset;
    uint32_t sample_description_index;
    uint32_t default_sample_duration;
    uint32_t default_sample_size;
    uint32_t default_sample_flags;
} bmp4_track_fragment_header;

#define BMP4_TRACK_FRAGMENT_RUN BMP4_TYPE('t','r','u','n')

typedef struct bmp4_track_fragment_run_header {
	bmp4_fullbox fullbox;
    uint32_t sample_count;
    struct {
        bool data_offset;
        bool first_sample_flags;
    } validate;
    int32_t data_offset;
    uint32_t first_sample_flags;
} bmp4_track_fragment_run_header;

typedef struct bmp4_track_fragment_run_sample {
    uint64_t offset; /* offset relative to either bmp4_track_fragment_header.base_data_offset or start of the Movie Fragment Box */
    uint64_t time; /* cumulative time */
    uint32_t duration;
    uint32_t size;
    uint32_t flags;
    uint32_t composition_time_offset;
} bmp4_track_fragment_run_sample;

/* #define BMP4_SAMPLE_DEPENDS_ON(flags) B_GET_BITS(flags,26,25)  */
/* #define BMP4_SAMPLE_IS_DEPENDED_ON(flags) B_GET_BITS(flags,24,23) */
#define BMP4_SAMPLE_IS_DIFFERENCE_SAMPLE(flags) B_GET_BIT(flags,16)

typedef struct bmp4_track_fragment_run_state {
    unsigned sample_no;
    uint64_t accumulated_size;
    uint64_t accumulated_time;
} bmp4_track_fragment_run_state;

#define BMP4_SAMPLE_ENCRYPTED_VIDEO BMP4_TYPE('e','n','c','v')
#define BMP4_SAMPLE_ENCRYPTED_AUDIO BMP4_TYPE('e','n','c','a')

size_t bmp4_parse_box(batom_cursor *cursor, bmp4_box *box);
size_t bmp4_scan_box(batom_cursor *cursor, uint32_t type, bmp4_box *box);
bool bmp4_parse_box_extended(batom_cursor *cursor, bmp4_box_extended *box);
bool bmp4_parse_fullbox(batom_cursor *cursor, bmp4_fullbox *box);
bool bmp4_parse_filetype(batom_t box, bmp4_filetypebox *filetype);
bool bmp4_parse_movieheader(batom_t box, bmp4_movieheaderbox *movieheader);
bool bmp4_parse_trackheader(batom_t box, bmp4_trackheaderbox *trackheader);
bool bmp4_parse_mediaheader(batom_t box, bmp4_mediaheaderbox *mediaheader);
bool bmp4_parse_string(batom_cursor *cursor, char *string, size_t strlen);
bool bmp4_parse_handler(batom_t box, bmp4_handlerbox *handler);
bool bmp4_parse_visualsampleentry(batom_cursor *cursor, bmp4_visualsampleentry *entry);
bool bmp4_parse_audiosampleentry(batom_cursor *cursor, bmp4_audiosampleentry *entry);
bool bmp4_parse_sample_info(batom_t box, bmp4_sample_info *sample, uint32_t handler_type);
void bmp4_free_sample_info(bmp4_sample_info *sample);
bool bmp4_parse_sample_avcC(batom_cursor *cursor, bmp4_sample_avc *avc, size_t entry_data_size);
bool bmp4_parse_sample_size_header(batom_cursor *cursor, bmp4_sample_size_header *header);
bool bmp4_parse_compact_sample_size_header(batom_cursor *cursor, bmp4_compact_sample_size_header *header);
bool bmp4_parse_trackextends(batom_t box, bmp4_trackextendsbox *trackextends);
bool bmp4_parse_movie_fragment_header(batom_cursor *cursor, bmp4_movie_fragment_header *header);
bool bmp4_parse_track_fragment_header(batom_cursor *cursor, bmp4_track_fragment_header *header);
bool bmp4_parse_track_fragment_run_header(batom_cursor *cursor, bmp4_track_fragment_run_header  *run_header);
void bmp4_init_track_fragment_run_state(bmp4_track_fragment_run_state *state);
bool bmp4_parse_track_fragment_run_sample(batom_cursor *cursor, const bmp4_track_fragment_header *fragment_header, const bmp4_track_fragment_run_header *run_header, const bmp4_trackextendsbox *track_extends, bmp4_track_fragment_run_state *state, bmp4_track_fragment_run_sample *sample);

#if 0
/* unused functions */
typedef struct bmp4_sample_size {
    uint32_t sample_count;
} bmp4_sample_size;

bool bmp4_parse_sample_size(batom_t box, bmp4_sample_size *sample);
bool bmp4_parse_compact_sample_size(batom_t box, bmp4_sample_size *sample);

#endif


#ifdef __cplusplus
}
#endif


#endif /* _BMP4_UTIL_H__ */

