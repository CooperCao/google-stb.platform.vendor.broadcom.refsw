/***************************************************************************
 *     Copyright (c) 2007-2012, Broadcom Corporation
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
 * BMedia library, stream probe module
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#ifndef _BMKV_PROBE_H__
#define _BMKV_PROBE_H__

#include "bmedia_probe_impl.h"


#ifdef __cplusplus
extern "C"
{
#endif


typedef struct bmkv_Attachment {
    BLST_SQ_ENTRY(bmkv_Attachment) link; /* this field is used to link attachments together */
    bool FileData_valid;
    bool FileUID_valid;
    uint64_t FileData_size; 
    uint64_t FileData_offset;
    uint64_t FileUID;
    const char *FileName;
    const char *FileDescription; 
    const char *FileMimeType;
} bmkv_Attachment;

typedef BLST_SQ_HEAD(bmkv_AttachmentList, bmkv_Attachment) bmkv_AttachmentList;


typedef struct bmkv_Chapters {
    BLST_SQ_ENTRY(bmkv_Chapters) link; /* this field is used to link chapters together */
    bool ChapterUID_valid;
    uint64_t ChapterUID;
    uint32_t ChapterTimeStart;
    uint32_t ChapterTimeEnd;
    uint32_t ChapterFlagHidden;
    uint32_t ChapterFlagEnabled;
    struct {
        const char *ChapString;
        const char *ChapLanguage;
        const char *ChapCountry;
    } ChapterDisplay;
} bmkv_Chapters;

typedef BLST_SQ_HEAD(bmkv_ChaptersList, bmkv_Chapters) bmkv_ChaptersList;

typedef struct bmkv_Editions {
      BLST_SQ_ENTRY(bmkv_Editions) link; /* this field is used to link editions together */
      uint64_t EditionUID;
      uint32_t EditionFlagHidden;
      uint32_t EditionFlagDefault;
      uint32_t EditionFlagOrdered;
      bmkv_ChaptersList chapters;
} bmkv_Editions;

typedef struct bmkv_probe_next_volume {
    bool next_volume_offset_valid;
    uint64_t next_volume_offset;
} bmkv_probe_next_volume;

typedef BLST_SQ_HEAD(bmkv_EditionsList, bmkv_Editions) bmkv_EditionsList;

typedef struct bmkv_probe_stream {
    bmedia_probe_stream media;
    bmkv_AttachmentList attachments;
    bmkv_EditionsList editions;
    bmkv_probe_next_volume next_volume;
    uint32_t TracksDataVersion;
    uint32_t TracksDataSize;
    const void *TracksDataPayload;
    char docType[16]; /* null terminated doc type */
} bmkv_probe_stream;

#define BMKV_PROBE_MAX_ATTACHMENT_LINKS 4

typedef struct bmkv_probe_track {
    bmedia_probe_track media;
    bool unsupported; /* set to true is detected some MKV syntat that can't be supported during playback */
    char language[16]; 
    char codec_id[32];
    bool TrickTrackFlag;
    bool ContentCompAlgo_valid;
    bool AttachmentLink_valid;
    unsigned ContentCompAlgo;
    uint64_t AttachmentLink[BMKV_PROBE_MAX_ATTACHMENT_LINKS];
    size_t CodecPrivate_len; /* size of the CodecPrivate data, 0 if unavailable */
    const void *CodecPrivate_data;
    size_t CodecName_len;
    uint8_t *CodecName_data;
    unsigned DisplayWidth;
    unsigned DisplayHeight;
    uint64_t DefaultDuration; /* default duration of single sample, in ns */
    uint64_t TrackUID;
    uint64_t TrickTrackUID;
    uint64_t MasterTrackUID;
} bmkv_probe_track;

extern const bmedia_probe_format_desc bmkv_probe;

#ifdef __cplusplus
}
#endif


#endif /* _BMKV_PROBE_H__ */

