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
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *******************************************************************************/
#ifndef _BMKV_UTIL_H__
#define _BMKV_UTIL_H__

#include "bioatom.h"
#include "bmedia_util.h"
#include "blst_squeue.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Element id */
typedef uint32_t bmkv_id;

/* Size of the element */
typedef uint64_t bmkv_size;

/* maximum size of the element head (id + size) */
#define B_MKV_MAX_ELEMENT_HEAD 12 /* 4 bytes head and 8 bytes size */
#define B_MKV_MIN_ELEMENT_HEAD 2 /* 1 byte head and 1 byte size */

typedef struct bmkv_header {
    bmkv_id id;
    bmkv_size size;
} bmkv_header;

bool bmkv_parse_header(batom_cursor *cursor, bmkv_header *header);
uint64_t bmkv_parse_unsigned64(batom_cursor *cursor);
int64_t bmkv_parse_signed64(batom_cursor *cursor);
int64_t bmkv_read_signed(batom_cursor *cursor, size_t size);

#define BMKV_INVALID_ID  ((bmkv_id)(-1))
#define BMKV_INVALID_SIZE ((bmkv_size)(-1))
#define BMKV_RESERVED_SIZE ((bmkv_size)(-2))

typedef struct bmkv_unique_id {
    uint8_t data[128/8];
} bmkv_unique_id;

typedef double bmkv_float;
typedef int64_t bmkv_date;

typedef struct bmkv_utf8 {
    size_t utf8_len; /* must be the first field */
    uint8_t *utf8_data;
} bmkv_utf8;

typedef struct bmkv_data {
    size_t data_len; /* must be the first field */
    void *data;
} bmkv_data;

typedef struct bmkv_string {
    size_t string_len; /* must be the first field */
    char *string_data;
} bmkv_string;

typedef struct bmkv_table {
    size_t bmkv_table_alloc_nelems;
    size_t nelems;
    void *data;
} bmkv_table;


#define BMKV_TABLE_ELEM(table,type,n) (((type *)(table).data)[n])

typedef enum bmkv_parser_entry_type {
    bmkv_parser_entry_type_unsigned,
    bmkv_parser_entry_type_signed,
    bmkv_parser_entry_type_unique_id,
    bmkv_parser_entry_type_date,
    bmkv_parser_entry_type_float,
    bmkv_parser_entry_type_string,
    bmkv_parser_entry_type_unsigned64,
    bmkv_parser_entry_type_id,
    bmkv_parser_entry_type_utf8,
    bmkv_parser_entry_type_table,
    bmkv_parser_entry_type_large_string,
    bmkv_parser_entry_type_data,
    bmkv_parser_entry_type_bool,
    bmkv_parser_entry_type_stop,
    bmkv_parser_entry_type_last
} bmkv_parser_entry_type;

typedef struct bmkv_parser_entry {
    bmkv_id  id;
    bmkv_parser_entry_type type;
    uint16_t elem_off;
    uint16_t elem_size;
    uint16_t validate_off;
    uint16_t validate_size;
    const char *name;
} bmkv_parser_entry;

typedef struct bmkv_parser_desc  {
    char bmkv_parser_desc_name[32]; /* must be  the first field and array */
    const bmkv_parser_entry *entries;
    size_t size;
    void (*init)(void *object);
} bmkv_parser_desc;

typedef enum bmkv_element_parse_result {
    bmkv_element_parse_result_success = 0,
    bmkv_element_parse_result_unknown_id,
    bmkv_element_parse_result_error
} bmkv_element_parse_result;

#define BMKV_PARSER_DECLARE(name) bmkv_parser_entry name[];

#define BMKV_PARSER_BEGIN(name) bmkv_parser_entry name[] = {
/* #define B_MKV_PARSER_ENTRY(id, type, object, field) {(id), (type), &((struct object *)NULL->(field)), sizeof((struct object *)NULL->(field)), &((struct object *)NULL->validate.(field))}, */
/* #define B_MKV_PARSER_ENTRY(id, type, object, field) {(id), (type), &(((struct object *)NULL)->(field))   }, */
#define B_MKV_OFFSETOF(type, member) (size_t)&(((type*)0)->member)
#define B_MKV_SIZEOF(type, member) sizeof(((type*)0)->member)
#define B_MKV_PARSER_ENTRY(id, object, type, field) {(id), (type), B_MKV_OFFSETOF(object, field), B_MKV_SIZEOF(object,field),B_MKV_OFFSETOF(object,validate.field), B_MKV_SIZEOF(object,validate.field), #field},
#define BMKV_PARSER_FIELD_UNSIGNED(object,id,field) B_MKV_PARSER_ENTRY((id), object, bmkv_parser_entry_type_unsigned, field )
#define BMKV_PARSER_FIELD_SIGNED(object,id,field) B_MKV_PARSER_ENTRY((id), object, bmkv_parser_entry_type_signed, field )
#define BMKV_PARSER_FIELD_ID(object,id,field) B_MKV_PARSER_ENTRY((id), object, bmkv_parser_entry_type_id, field )
#define BMKV_PARSER_FIELD_STRING(object,id,field) B_MKV_PARSER_ENTRY((id), object, bmkv_parser_entry_type_string, field)
#define BMKV_PARSER_FIELD_UID(object,id,field) B_MKV_PARSER_ENTRY((id), object, bmkv_parser_entry_type_unique_id, field )
#define BMKV_PARSER_FIELD_UNSIGNED64(object,id,field) B_MKV_PARSER_ENTRY((id), object, bmkv_parser_entry_type_unsigned64, field )
#define BMKV_PARSER_FIELD_FLOAT(object,id,field) B_MKV_PARSER_ENTRY((id), object, bmkv_parser_entry_type_float, field )
#define BMKV_PARSER_FIELD_DATE(object,id,field) B_MKV_PARSER_ENTRY((id), object, bmkv_parser_entry_type_date, field )
#define BMKV_PARSER_FIELD_BOOL(object,id,field) B_MKV_PARSER_ENTRY((id), object, bmkv_parser_entry_type_bool, field )
#define BMKV_PARSER_FIELD_STOP(object,id,field) B_MKV_PARSER_ENTRY((id), object, bmkv_parser_entry_type_stop, field )

/* B_MKV_OFFSETOF(object, field.utf8_len) shall be equal to B_MKV_OFFSETOF(object, field), however it also verifies that we pass data of correct type */
#define BMKV_PARSER_FIELD_UTF8(object,id,field) {(id), bmkv_parser_entry_type_utf8, B_MKV_OFFSETOF(object, field.utf8_len), B_MKV_SIZEOF(object,field),B_MKV_OFFSETOF(object,validate.field), B_MKV_SIZEOF(object,validate.field), #field},

/* B_MKV_OFFSETOF(object, field.bmkv_table_alloc_nelems) shall be equal to B_MKV_OFFSETOF(object, field), however it also verifies that we pass data of correct type */
#define BMKV_PARSER_FIELD_TABLE(object,id,field,table_meta) {(id), bmkv_parser_entry_type_table, B_MKV_OFFSETOF(object, field.bmkv_table_alloc_nelems), B_MKV_SIZEOF(object,field),B_MKV_OFFSETOF(object,validate.field), B_MKV_SIZEOF(object,validate.field), table_meta.bmkv_parser_desc_name},

#define BMKV_PARSER_FIELD_DATA(object,id,field) {(id), bmkv_parser_entry_type_data, B_MKV_OFFSETOF(object, field.data_len), B_MKV_SIZEOF(object,field),B_MKV_OFFSETOF(object,validate.field), B_MKV_SIZEOF(object,validate.field), #field},

#define BMKV_PARSER_FIELD_VSTRING(object,id,field) {(id), bmkv_parser_entry_type_data, B_MKV_OFFSETOF(object, field.string_len), B_MKV_SIZEOF(object,field), B_MKV_OFFSETOF(object,validate.field), B_MKV_SIZEOF(object,validate.field), #field},


#define BMKV_PARSER_END(name) {0, bmkv_parser_entry_type_last, 0, 0, 0, 0, NULL} }

bool bmkv_element_parse(batom_cursor *cursor, size_t elem_size, const bmkv_parser_entry *entries, const char *elem_name, void *elems);
bool bmkv_element_skip(batom_cursor *cursor);
bmkv_element_parse_result bmkv_element_parse_from_desc(batom_cursor *cursor, const bmkv_parser_desc *desc, bmkv_id id, void *elem);

void bmkv_element_shutdown(const bmkv_parser_entry *entries, void *elems);
void bmkv_element_print(const bmkv_parser_entry *entries, BDBG_Level level, unsigned padding, const char *name, const void *elem);

#define BMKV_EBML_ID    0x1A45DFA3

typedef struct bmkv_EBML {
    unsigned EBMLVersion;
    unsigned EBMLReadVersion;
    unsigned EBMLMaxIDLength;
    unsigned EBMLMaxSizeLength;
    char DocType[16];
    unsigned DocTypeVersion;
    unsigned DocTypeReadVersion;
    struct {
        bool EBMLVersion;
        bool EBMLReadVersion;
        bool EBMLMaxIDLength;
        bool EBMLMaxSizeLength;
        bool DocType;
        bool DocTypeVersion;
        bool DocTypeReadVersion;
    } validate;
} bmkv_EBML;

#define BMKV_SEEK   0x4DBB
typedef struct bmkv_SeekElement{
    bmkv_id SeekID;
    uint64_t SeekPosition;
    struct {
        bool SeekID;
        bool SeekPosition;
    } validate;
} bmkv_SeekElement;
extern const bmkv_parser_desc bmkv_SeekElement_desc;

#define BMKV_SEEKHEAD_ID 0x114D9B74
typedef struct bmkv_SeekHead {
    bmkv_table Seek;
    struct {
        bool    Seek;
    } validate;
} bmkv_SeekHead;

typedef struct bmkv_ChapterTranslate {
    unsigned ChapterTranslateEditionUID;
    unsigned ChapterTranslateCodec;
    struct {
        bool ChapterTranslateEditionUID;
        bool ChapterTranslateCodec;
    } validate;
} ChapterTranslate;


#define BMKV_VOID_ID    0xEC
#define BMKV_SEGMENT_ID 0x18538067

#define BMKV_SEGMENTINFO_ID 0x1549A966
typedef struct bmkv_SegmentInformation {
    bmkv_unique_id SegmentUID;
    bmkv_utf8 SegmentFilename;
    bmkv_unique_id PrevUID;
    bmkv_utf8 PrevFilename;
    bmkv_unique_id NextUID;
    bmkv_utf8 NextFilename;
    bmkv_unique_id SegmentFamily;
    bmkv_table ChapterTranslate;
    unsigned TimecodeScale;
    bmkv_float Duration;
    bmkv_date DateUTC;
    bmkv_utf8 Title;
    bmkv_utf8 MuxingApp;
    bmkv_utf8 WritingApp;
    struct {
        bool SegmentUID;
        bool SegmentFilename;
        bool PrevUID;
        bool PrevFilename;
        bool NextUID;
        bool NextFilename;
        bool SegmentFamily;
        bool ChapterTranslate;
        bool TimecodeScale;
        bool Duration;
        bool DateUTC;
        bool Title;
        bool MuxingApp;
        bool WritingApp;
    } validate;
} bmkv_SegmentInformation;

typedef struct bmkv_TrackEntryVideo {
    bool FlagInterlaced;
    unsigned StereoMode;
    unsigned PixelWidth;
    unsigned PixelHeight;
    unsigned PixelCropBottom;
    unsigned PixelCropTop;
    unsigned PixelCropLeft;
    unsigned PixelCropRight;
    unsigned DisplayWidth;
    unsigned DisplayHeight;
    unsigned DisplayUnit;
    unsigned AspectRatioType;
    unsigned ColourSpace;
    bmkv_float GammaValue;
    bmkv_data Colour;
    struct {
        bool FlagInterlaced;
        bool StereoMode;
        bool PixelWidth;
        bool PixelHeight;
        bool PixelCropBottom;
        bool PixelCropTop;
        bool PixelCropLeft;
        bool PixelCropRight;
        bool DisplayWidth;
        bool DisplayHeight;
        bool DisplayUnit;
        bool AspectRatioType;
        bool ColourSpace;
        bool GammaValue;
        bool Colour;
    } validate;
} bmkv_TrackEntryVideo;

typedef struct bmkv_TrackEntryAudio {
    bmkv_float SamplingFrequency;
    bmkv_float OutputSamplingFrequency;
    unsigned Channels;
    bmkv_data ChannelPositions;
    unsigned BitDepth;
    struct {
        bool SamplingFrequency;
        bool OutputSamplingFrequency;
        bool Channels;
        bool ChannelPositions;
        bool BitDepth;
    } validate;
} bmkv_TrackEntryAudio;

typedef struct bmkv_ContentCompression {
    unsigned ContentCompAlgo;
    bmkv_data ContentCompSettings;
    struct {
        bool ContentCompAlgo;
        bool ContentCompSettings;
    } validate;
} bmkv_ContentCompression;

typedef struct bmkv_ContentEncryption {
    unsigned ContentEncAlgo;
    bmkv_data ContentEncKeyID;
    bmkv_data ContentSignature;
    bmkv_data ContentSigKeyID;
    unsigned ContentSigAlgo;
    unsigned ContentSigHashAlgo;
    struct {
        bool ContentEncAlgo;
        bool ContentEncKeyID;
        bool ContentSignature;
        bool ContentSigKeyID;
        bool ContentSigAlgo;
        bool ContentSigHashAlgo;
    } validate;
} bmkv_ContentEncryption;

typedef struct bmkv_ContentEncoding {
    unsigned ContentEncodingOrder;
    unsigned ContentEncodingScope;
    unsigned ContentEncodingType;
    bmkv_table ContentCompression;
    bmkv_table ContentEncryption;
    struct {
        bool ContentEncodingOrder;
        bool ContentEncodingScope;
        bool ContentEncodingType;
        bool ContentCompression;
        bool ContentEncryption;
    } validate;
} bmkv_ContentEncoding;

typedef struct bmkv_ContentEncodings {
    bmkv_table ContentEncoding;
    struct {
        bool ContentEncoding;
    } validate;
} bmkv_ContentEncodings;

typedef struct bmkv_AttachmentLink {
    uint64_t AttachmentLink;
     struct {
        bool AttachmentLink;
    } validate;
} bmkv_AttachmentLink;

typedef struct bmkv_TrackEntry {
    unsigned TrackNumber;
    uint64_t TrackUID;
    unsigned TrackType;
    bool FlagEnabled;
    bool FlagDefault;
    bool FlagForced;
    bool FlagLacing;
    unsigned MinCache;
    unsigned MaxCache;
    uint64_t DefaultDuration;
    bmkv_float TrackTimecodeScale;
    signed TrackOffset;
    unsigned MaxBlockAdditionID;
    bmkv_utf8 Name;
    char Language[16];
    char CodecID[32];
    bmkv_data CodecPrivate;
    bmkv_utf8 CodecName;
    bmkv_table AttachmentLink;
    bmkv_utf8 CodecSettings;
    bmkv_string CodecInfoURL;
    bmkv_string CodecDownloadURL;
    bool CodecDecodeAll;
    unsigned TrackOverlay;
    uint64_t TrickTrackUID;
    uint64_t MasterTrackUID;
    bool TrickTrackFlag;
    bmkv_table TrackTranslate;
    bmkv_table Video;
    bmkv_table Audio;
    bmkv_table ContentEncodings;
    struct {
        bool TrackNumber;
        bool TrackUID;
        bool TrackType;
        bool FlagEnabled;
        bool FlagDefault;
        bool FlagForced;
        bool FlagLacing;
        bool MinCache;
        bool MaxCache;
        bool DefaultDuration;
        bool TrackTimecodeScale;
        bool TrackOffset;
        bool MaxBlockAdditionID;
        bool Name;
        bool Language;
        bool CodecID;
        bool CodecPrivate;
        bool CodecName;
        bool AttachmentLink;
        bool CodecSettings;
        bool CodecInfoURL;
        bool CodecDownloadURL;
        bool CodecDecodeAll;
        bool TrackOverlay;
        bool TrickTrackUID;
        bool TrickTrackSegUID;
        bool MasterTrackUID;
        bool MasterTrackSegUID;
        bool TrickTrackFlag;
        bool TrackTranslate;
        bool Video;
        bool Audio;
        bool ContentEncodings;
    } validate;
} bmkv_TrackEntry;

typedef struct bmkv_TracksData {
    unsigned TracksDataVersion;
    unsigned TracksDataSize;
    bmkv_data TracksDataPayload;
    struct {
        bool TracksDataVersion;
        bool TracksDataSize;
        bool TracksDataPayload;
    } validate;
} bmkv_TracksData;

#define BMKV_TRACKS_ID  0x1654AE6B
typedef struct bmkv_Tracks {
    bmkv_table TrackEntry;
    bmkv_table TracksData;
    struct {
        bool TrackEntry;
        bool TracksData;
    } validate;
} bmkv_Tracks;

#define BMKV_CLUSTER_ID 0x1F43B675
#define BMKV_CUES_ID    0x1C53BB6B
#define BMKV_ATTACHMENT_ID 0x1941A469
#define BMKV_CHAPTERS_ID 0x1043A770

typedef struct bmkv_TrackEntryVideoColourMasteringMetadata {
    bmkv_float PrimaryRChromaticityX;
    bmkv_float PrimaryRChromaticityY;
    bmkv_float PrimaryGChromaticityX;
    bmkv_float PrimaryGChromaticityY;
    bmkv_float PrimaryBChromaticityX;
    bmkv_float PrimaryBChromaticityY;
    bmkv_float WhitePointChromaticityX;
    bmkv_float WhitePointChromaticityY;
    bmkv_float LuminanceMax;
    bmkv_float LuminanceMin;
    struct {
        bool PrimaryRChromaticityX;
        bool PrimaryRChromaticityY;
        bool PrimaryGChromaticityX;
        bool PrimaryGChromaticityY;
        bool PrimaryBChromaticityX;
        bool PrimaryBChromaticityY;
        bool WhitePointChromaticityX;
        bool WhitePointChromaticityY;
        bool LuminanceMax;
        bool LuminanceMin;
    } validate;
} bmkv_TrackEntryVideoColourMasteringMetadata;

typedef struct bmkv_TrackEntryVideoColour {
    unsigned MatrixCoefficients;
    unsigned BitsPerChannel;
    unsigned ChromaSubsamplingHorz;
    unsigned ChromaSubsamplingVert;
    unsigned CbSubsamplingHorz;
    unsigned CbSubsamplingVert;
    unsigned ChromaSitingHorz;
    unsigned ChromaSitingVert;
    unsigned Range;
    unsigned TransferCharacteristics;
    unsigned Primaries;
    unsigned MaxCLL;
    unsigned MaxFALL;
    bmkv_table MasteringMetadata;
    struct {
        bool MatrixCoefficients;
        bool BitsPerChannel;
        bool ChromaSubsamplingHorz;
        bool ChromaSubsamplingVert;
        bool CbSubsamplingHorz;
        bool CbSubsamplingVert;
        bool ChromaSitingHorz;
        bool ChromaSitingVert;
        bool Range;
        bool TransferCharacteristics;
        bool Primaries;
        bool MaxCLL;
        bool MaxFALL;
        bool MasteringMetadata;
    } validate;
} bmkv_TrackEntryVideoColour;



bool bmkv_EBML_parse(batom_cursor *cursor, size_t elem_size, bmkv_EBML *ebml);
bool bmkv_EBML_validate(const bmkv_EBML *ebml);
bool bmkv_SeekHead_parse(batom_cursor *cursor, size_t elem_size, bmkv_SeekHead *seek);
void bmkv_SeekHead_shutdown(bmkv_SeekHead *seek);
bool bmkv_SegmentInfo_parse(batom_cursor *cursor, size_t elem_size, bmkv_SegmentInformation *segment);
void bmkv_SegmentInfo_shutdown(bmkv_SegmentInformation *segment);
bool bmkv_Tracks_parse(batom_cursor *cursor, size_t elem_size, bmkv_Tracks *track);
void bmkv_Tracks_shutdown(bmkv_Tracks *track);
extern const char bmkv_matroska[9];

bool bmkv_IsTrackVideoAvc(const bmkv_TrackEntry *track);
bool bmkv_IsTrackVideoHevc(const bmkv_TrackEntry *track);
bool bmkv_IsTrackVideoMpeg2(const bmkv_TrackEntry *track);
bool bmkv_IsTrackVideoMpeg1(const bmkv_TrackEntry *track);
bool bmkv_IsTrackVideoVFW(const bmkv_TrackEntry *track);
bool bmkv_IsTrackVideoMpeg4Asp(const bmkv_TrackEntry *track);
bool bmkv_IsTrackAudioMkvAac(const bmkv_TrackEntry *track);
bool bmkv_IsTrackAudioAc3(const bmkv_TrackEntry *track);
bool bmkv_IsTrackAudioAc3Plus(const bmkv_TrackEntry *track);
bool bmkv_IsTrackAudioDts(const bmkv_TrackEntry *track);
bool bmkv_IsTrackAudioAac(const bmkv_TrackEntry *track, bmedia_info_aac *aac);
bool bmkv_IsTrackAudioMpeg1Layer3(const bmkv_TrackEntry *track);
bool bmkv_IsTrackAudioMpeg1Layer1_2(const bmkv_TrackEntry *track);
bool bmkv_IsTrackAuxiliary(const bmkv_TrackEntry *track);
bool bmkv_IsTrackAudioACM(const bmkv_TrackEntry *track);
bool bmkv_IsTrackAudioVorbis(const bmkv_TrackEntry *track);
bool bmkv_IsTrackVideoVp8(const bmkv_TrackEntry *track);
bool bmkv_IsTrackVideoVp9(const bmkv_TrackEntry *track);
bool bmkv_IsTrackAudioOpus(const bmkv_TrackEntry *track);
bool bmkv_IsTrackAudioPcmInt(const bmkv_TrackEntry *track);

extern const bmkv_parser_desc bmkv_TrackEntryVideoColour_desc;
#ifdef __cplusplus
}
#endif


#endif /* _BMKV_UTIL_H__ */

