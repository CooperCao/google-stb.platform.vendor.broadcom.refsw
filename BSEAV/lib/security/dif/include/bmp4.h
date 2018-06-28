/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef BMP4_H
#define BMP4_H

#include "bstd.h"
#include "bioatom.h"
#include "bmp4_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AVCC_SPS_OFFSET 6

#define BMP4_MAX_NB_OF_TRACKS (5)
#define BMP4_MAX_PPS_SPS 256

#define BMP4_SAMPLES_POOL_SIZE (500)
#define BMP4_MAX_ENTRIES_PER_SAMPLE (10)
#define BMP4_MAX_SAMPLES (1024)
#define BMP4_UUID_LENGTH (16)  /* Lenght of UUID in bytes */
#define BMP4_KEY_ID_LENGTH (16)    /* Key Id length in bytes */
#define BMP4_MAX_IV_ENTRIES (16)
#define BMP4_MAX_DRM_SCHEMES (4)

static const uint8_t bmp4_nal[] = {0x00, 0x00, 0x00, 0x01};
static const batom_vec bmp4_nal_vec = BATOM_VEC_INITIALIZER((void *)bmp4_nal, sizeof(bmp4_nal));


typedef struct bmp4_uuid {
    uint8_t data[BMP4_UUID_LENGTH];
} bmp4_uuid;

typedef struct bmp4_psshBoxInfo {
    bmp4_uuid systemId;        /* UUID of the Content Protection system. */
} bmp4_psshBoxInfo;

typedef struct bmp4_schemeTypeBox {
    uint8_t version;
    uint32_t flags;
    uint32_t schemeType;
    uint32_t schemeVersion;
} bmp4_schemeTypeBox;

typedef struct bmp4_originalFormatBox {
    uint32_t codingName;
} bmp4_originalFormatBox;

typedef enum bmp4_encryptionType {
    bmp4_encryptionType_eNone,    /* no encryption */
    bmp4_encryptionType_eWmdrm,   /* wmdrm encrypted asf */
    bmp4_encryptionType_eAesCtr,  /* AES CTR encrypted stream */
    bmp4_encryptionType_eAesCbc,  /* AES CBC encrypted stream */
    bmp4_encryptionType_eMax
} bmp4_encryptionType;

typedef struct bmp4_SchemeInfo {
    bmp4_encryptionType algorithm; /* Default algorithm used to encrypt the track */
    unsigned ivSize;                /* Default Initialization Vector (IV) size in bytes
                                       8 (64-bit) - Supported for AES-CTR
                                       16 (128-bit) - Supported for both AES-CTR and AES-CBC */
    uint8_t keyId[BMP4_KEY_ID_LENGTH];   /* Default Key Id for this track */
} bmp4_SchemeInfo;

typedef struct bmp4_trackEncryptionBox {
    uint8_t version;
    uint32_t flags;
    bmp4_SchemeInfo info;
} bmp4_trackEncryptionBox;

typedef struct bmp4_decoderCfg {
    uint32_t size;
    uint8_t data[BMP4_MAX_PPS_SPS];
} bmp4_decoderCfg;

typedef struct bmp4_protectionSchemeInfo {
    bmp4_schemeTypeBox schemeType;
    bmp4_originalFormatBox originalFormat;
    bmp4_trackEncryptionBox trackEncryption;
    bmp4_decoderCfg decConfig;
    uint32_t trackId;
    uint32_t trackType;
    uint32_t trackTimeScale;
} bmp4_protectionSchemeInfo;

typedef struct bmp4_mp4_headers {
    uint8_t numOfDrmSchemes;                              /* Number of DRM systems found in the stream file*/
    uint32_t cpsSpecificDataSize[BMP4_MAX_DRM_SCHEMES];
    uint8_t *cpsSpecificData[BMP4_MAX_DRM_SCHEMES];       /* Content Protection System Specific data*/
    bmp4_psshBoxInfo psshSystemId[BMP4_MAX_DRM_SCHEMES];  /* Protection System Specific Header Box */
    uint8_t *pPsshData[BMP4_MAX_DRM_SCHEMES];             /* Set to NULL if no DRM has been found in the moov  */
    uint32_t psshDataLength[BMP4_MAX_DRM_SCHEMES];
    bmp4_protectionSchemeInfo scheme[BMP4_MAX_NB_OF_TRACKS]; /* Pool of protection scheme info for tracks */
    uint32_t nbOfSchemes;           /* Number of encrypted tracks discovered from the moov */
    bmp4_trackheaderbox trackHeader[BMP4_MAX_NB_OF_TRACKS]; /* Track headers */
    uint32_t nbOfTracks;           /* Number of tracks discovered from the moov */
    bool piff_1_3;                  /* Set when PIFF 1.3 (CENC) is detected */
} bmp4_mp4_headers;

typedef struct bmp4_drm_mp4_box_se_entry {
    uint32_t bytesOfClearData;
    uint32_t bytesOfEncData;

} bmp4_drm_mp4_box_se_entry;

typedef struct SampleInfo {
    uint8_t  iv[BMP4_MAX_IV_ENTRIES];  /* If the IV size is 8, then bytes 0 to 7 of teh 16 byte array contains the IV. */

    /* The following values are only meaningfull if flag & 0x000002 is true */
    uint16_t nbOfEntries;
    uint32_t size;
    uint64_t offset;
    bmp4_drm_mp4_box_se_entry entries[BMP4_MAX_ENTRIES_PER_SAMPLE];

} SampleInfo;

typedef struct bmp4_protection_info {
    uint32_t                    nbOfSchemes;
    bmp4_protectionSchemeInfo   schemeProtectionInfo[BMP4_MAX_NB_OF_TRACKS];
} bmp4_protection_info;

typedef struct bmp4_drm_mp4_se {
    uint32_t flags;
    uint32_t sample_count;
    uint8_t default_sample_info_size;
    SampleInfo samples[BMP4_SAMPLES_POOL_SIZE];

} bmp4_drm_mp4_se;

typedef struct bmp4_mp4_frag_headers {
    uint32_t current_moof_size;
    uint32_t trackId;                    /* Found in TFHD box */
    uint32_t trackType;                  /* Found in TFHD box */
    uint32_t trackTimeScale;             /* Found in MDHD box */
    uint32_t run_sample_count;           /* Found in TRUN box */

    bmp4_track_fragment_run_state state;
    bmp4_track_fragment_run_sample sample_info[BMP4_MAX_SAMPLES];

    bool encrypted; /* Set when SAIO or UUID box is detected */
    bool enc_info_parsed; /* Set when SENC or UUID box is parsed */
    uint32_t aux_info_size;
    bmp4_drm_mp4_se samples_info;

} bmp4_mp4_frag_headers;

/**
Summary:
WMDRMPD get default mp4 header

Description:
Initialize a mp4 header structure to its default state
**/
void bmp4_GetDefaultMp4Header(
    bmp4_mp4_headers *pMp4
    );

/**
Summary:
WMDRMPD free mp4 header

Description:
Helper function dynamically allocation information created during MP4 parsing.
**/
void bmp4_FreeMp4Header(
    bmp4_mp4_headers *pMp4
    );

/**
Summary:
WMDRMPD get default mp4 fragment header

Description:
Initialize a mp4 fragment header structure to its default state
**/
void bmp4_InitMp4FragHeader(
    bmp4_mp4_frag_headers *pMp4Frag
    );

/**
Summary:
Parse file type

Description:
This function is used to parse a file type box and get its file type
**/
bool bmp4_parse_file_type(
    batom_t box,
    bmp4_filetypebox *filetype
    );

/**
Summary:
Parse Moov box

Description:
This function is used to parse a moov box and extract all the DRM information
**/
int bmp4_parse_moov(
    bmp4_mp4_headers *header,
    batom_t atom
    );

/**
Summary:
Parse Moof box

Description:
This function is used to parse a moof box and extract all the fragment track information
**/
int bmp4_parse_moof(
    bmp4_mp4_headers *header,
    bmp4_mp4_frag_headers *frag_header,
    batom_t atom
    );

#ifdef __cplusplus
}
#endif

#endif /*BMP4_H*/
