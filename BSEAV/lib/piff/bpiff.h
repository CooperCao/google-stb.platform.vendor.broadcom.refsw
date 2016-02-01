/******************************************************************************
* (c) 2012-2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#ifndef BPIFF_H
#define BPIFF_H

#include "bstd.h"
#include "bioatom.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NB_OF_TRACKS (5)

#define BPIFF_UUID_LENGTH (16)  /* Lenght of UUID in bytes */
typedef struct bpiff_uuid {
    uint8_t data[BPIFF_UUID_LENGTH];
} bpiff_uuid;

typedef struct bpiff_psshBoxInfo {
    bpiff_uuid systemId;        /* UUID of the Content Protection system. */
} bpiff_psshBoxInfo;

typedef struct bpiff_schemeTypeBox {
    uint8_t version;
    uint32_t flags;
    uint32_t schemeType;
    uint32_t schemeVersion;
} bpiff_schemeTypeBox;

typedef struct bpiff_originalFormatBox {
    uint32_t codingName;
} bpiff_originalFormatBox;

typedef enum bpiff_encryptionType {
    bpiff_encryptionType_eNone,    /* no encryption */
    bpiff_encryptionType_eWmdrm,   /* wmdrm encrypted asf */
    bpiff_encryptionType_eAesCtr,  /* AES CTR encrypted stream */
    bpiff_encryptionType_eAesCbc,  /* AES CBC encrypted stream */
    bpiff_encryptionType_eMax
} bpiff_encryptionType;

#define BPIFF_KEY_ID_LENGTH (16)    /* Key Id length in bytes */
typedef struct bpiff_SchemeInfo {
    bpiff_encryptionType algorithm; /* Default algorithm used to encrypt the track */
    unsigned ivSize;                /* Default Initialization Vector (IV) size in bytes
                                       8 (64-bit) - Supported for AES-CTR
                                       16 (128-bit) - Supported for both AES-CTR and AES-CBC */
    uint8_t keyId[BPIFF_KEY_ID_LENGTH];   /* Default Key Id for this track */
} bpiff_SchemeInfo;

typedef struct bpiff_trackEncryptionBox {
    uint8_t version;
    uint32_t flags;
    bpiff_SchemeInfo info;
} bpiff_trackEncryptionBox;

typedef struct bpiff_protectionSchemeInfo {
    bpiff_schemeTypeBox schemeType;
    bpiff_originalFormatBox originalFormat;
    bpiff_trackEncryptionBox trackEncryption;
    uint32_t trackId;
} bpiff_protectionSchemeInfo;

typedef struct bpiff_mp4_headers {
    bpiff_psshBoxInfo psshSystemId; /* Protection System Specific Header Box */
    uint8_t *pPsshData;             /* Set to NULL if no DRM has been found in the moov  */
    uint32_t psshDataLength;
    bpiff_protectionSchemeInfo scheme[MAX_NB_OF_TRACKS]; /* Pool of protection scheme info for tracks */
    uint32_t nbOfSchemes;           /* Number of encrypted tracks discovered from the moov */
    bool piff_1_3;                  /* Set when PIFF 1.3 (CENC) is detected */
} bpiff_mp4_headers;

/**
Summary:
WMDRMPD get default mp4 header

Description:
Initialize a mp4 header structure to its default state
**/
void bpiff_GetDefaultMp4Header(
    bpiff_mp4_headers *pMp4
    );

/**
Summary:
WMDRMPD free mp4 header

Description:
Helper function dynamically allocation information created during MP4 parsing.
**/
void bpiff_FreeMp4Header(
    bpiff_mp4_headers *pMp4
    );

/**
Summary:
Parse Moov box

Description:
This function is used to parse a moov box and extract all the DRM information
**/
int bpiff_parse_moov(
    bpiff_mp4_headers *header,
    batom_t atom
    );


#ifdef __cplusplus
}
#endif

#endif /*BPIFF_H*/
