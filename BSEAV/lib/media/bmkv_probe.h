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
    union {
        struct {
            struct {
                void *data;
                unsigned len;
            } Colour;
        } video;
    } data;
} bmkv_probe_track;

extern const bmedia_probe_format_desc bmkv_probe;

#ifdef __cplusplus
}
#endif


#endif /* _BMKV_PROBE_H__ */

