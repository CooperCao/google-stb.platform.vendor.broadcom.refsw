/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/


/*
 * Definitions for the "Secure Unified Image Format" (SUIF)
 */


#ifndef SUIF_H__
#define SUIF_H__

/* make use of standard C types for exact width (uint8_t, uint16_t, uint32_t) */
#include <stdint.h>

/* include common SUIF types */
#include "suif_types.h"

/* Secure Unified Image Format endianess:

   The tier key pacakge is saved in Little Endian
   everything else is saved in Big Endian.

   All multiple-bytes fields have a postfix with endianess:
      _LE for Little Endian and  _BE for Big Endian
   In any case (Big or Little Endian), use the same macros:
   - To read a word (32 bits), make use of SUIF_Get32() macro
   - To read a short (16 bits), make use of SUIF_Get16() macro
   The macro requries a field name, and an endianess to be passed (LE or BE)
   Examples:
      for a 32 bits field named xxyy_LE: SUIF_Get32(xxyy, LE);
      for a 16 bits field named abc_BE: SUIF_Get16(abc, BE);
*/


/* which version of the SUIF spec the headers implement */
#define SUIF_VERSION_MAJOR 0
#define SUIF_VERSION_MINOR 8


/* For internal use
 * TODO : define those macros to appreciate an array of N bytes so they are immune to unaligned access */
#define _SUIF_UINT32_SWAP(value) ( ((value) << 24) | (((value) << 8) & 0x00ff0000) | (((value) >> 8) & 0x0000ff00) | ((value) >> 24) )
#define _SUIF_UINT16_SWAP(value) ( (((value) << 8) & 0xff00) | (((value) >> 8) & 0x00ff) )

/* use GCC predefined macro to check endianness */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define _SUIF_Get32_LE(FIELD32) (*((uint32_t *)FIELD32))
#define _SUIF_Get16_LE(FIELD16) (*((uint16_t *)FIELD16))
#define _SUIF_Get32_BE(FIELD32) _SUIF_UINT32_SWAP(*((uint32_t *)FIELD32))
#define _SUIF_Get16_BE(FIELD16) _SUIF_UINT16_SWAP(*((uint16_t *)FIELD16))
#else
#define _SUIF_Get32_LE(FIELD32) _SUIF_UINT32_SWAP(*((uint32_t *)FIELD32))
#define _SUIF_Get16_LE(FIELD16) _SUIF_UINT16_SWAP(*((uint16_t *)FIELD16))
#define _SUIF_Get32_BE(FIELD32) (*((uint32_t *)FIELD32))
#define _SUIF_Get16_BE(FIELD16) (*((uint16_t *)FIELD16))
#endif
/**/


/******************************************************************
 * Macros for accessing multiple-bytes fields
 ******************************************************************/
#define SUIF_Get32(FIELD32, ENDIAN) _SUIF_Get32##_##ENDIAN(FIELD32##_##ENDIAN)
#define SUIF_Get16(FIELD16, ENDIAN) _SUIF_Get16##_##ENDIAN(FIELD16##_##ENDIAN)


/******************************************************************
 * tier key
 * --------
 *   First definition of tier key controlling parameters
 *   then definition of the tier key package
 *   two structures, one for Zeus4.2, one for Zeus5.x
 *   Note: The tier key is presented in Little Endian format
 ******************************************************************/

#define SUIF_TIER_RSAKEY_SIZE_BYTES (256)
#define SUIF_TIER_RSAKEY_SIG_SIZE_BYTES (256)

#define SUIF_AES_PROCIN_SIZE_BYTES (16)

#if ZEUS_MAJOR_VERSION == 4 || BHSM_ZEUS_VER_MAJOR == 4

/* Note: use SUIF_Get32() to read words (32 bits) */
/* sizeof(SUIF_Zeus42_TierKeyControllingParameters) = 44 bytes */
typedef struct
{
/*   0 [offset] */
    uint8_t reserved0;
    uint8_t pubExponent;         /* 0 => 3; 1 => 64K+1 */
    uint8_t sdlId;
    uint8_t signingRights;       /* 0 => MIPS; 2 => AVD, RAPTOR, RAVE; 4 => BSP; 8 => Boot; 10 => SAGE */
    uint8_t marketId_LE[4];
    uint8_t marketIdMask_LE[4];
    uint8_t epoch;
    uint8_t epochMask;
    uint8_t epochSelect;
    uint8_t marketIdSelect;
/*  16 [offset] */
    uint8_t multiTierKeys;
    uint8_t reserved2;
    uint8_t signatureType;
    uint8_t signatureVersion;
/*  20 [offset] */
} SUIF_Zeus42_TierKeyControllingParameters;

typedef SUIF_Zeus42_TierKeyControllingParameters SUIF_TierKeyControllingParameters;

#elif ZEUS_MAJOR_VERSION >= 5  || BHSM_ZEUS_VER_MAJOR >= 5

/* Note: use SUIF_Get32() to read words (32 bits) */
/* sizeof(SUIF_Zeus51_TierKeyControllingParameters) = 44 bytes */
typedef struct {
/*   0 [offset] */
    uint8_t epochSelect;          /* [2:0] */
    uint8_t marketIdSelect;       /* [1:0] */
    uint8_t signatureType;
    uint8_t signatureVersion;
    uint8_t reserved0;
    uint8_t chipBindingSelect;    /* [5:0] */
    uint8_t reserved1[2];
    uint8_t signingRights;
    uint8_t reserved2[3];
    uint8_t marketId_LE[4];
/*  16 [offset] */
    uint8_t marketIdMask_LE[4];    /* when comparing to value read from OTP */
    uint8_t epoch_LE[4];
    uint8_t epochMask_LE[4];
    uint8_t upperChipsetBinding_LE[4];
/*  32 [offset] */
    uint8_t lowerChipsetBinding_LE[4];
    uint8_t sdlId;             /* metadata1[0] */
    uint8_t multiTierKeys;     /* metadata1[1] */
    uint8_t metadata1[2];/* 2 bytes reserved for metadata */
    uint8_t metadata2[4];/* 4 bytes reserved for metadata */
/*  44 [offset] */
} SUIF_Zeus51_TierKeyControllingParameters;

typedef SUIF_Zeus51_TierKeyControllingParameters SUIF_TierKeyControllingParameters;

#else
#error unsupported Zeus version
#endif

typedef struct
{
    uint8_t keyData[SUIF_TIER_RSAKEY_SIZE_BYTES];
    SUIF_TierKeyControllingParameters controllingParams;
    uint8_t signature[SUIF_TIER_RSAKEY_SIG_SIZE_BYTES];
} SUIF_TierKeyPackage;



/******************************************************************
 * Region verification
 * -------------------
 *   Controlling parameters for region verification (image/data/text)
 *   two structures, one for Zeus4.2, one for Zeus5.x
 ******************************************************************/

#if ZEUS_MAJOR_VERSION == 4 || BHSM_ZEUS_VER_MAJOR == 4

/* Note: use SUIF_Get32() to read words (32 bits) */
/* sizeof(SUIF_Zeus42_RegionVerificationControllingParameters) = 20 bytes */
typedef struct
{
/*   0 [offset] */
    uint8_t reserved0[2];
    uint8_t cpuType;
    uint8_t noReloc;
    uint8_t marketId_BE[4];
    uint8_t marketIdMask_BE[4];
    uint8_t marketIdSelect;
    uint8_t epochSelect;
    uint8_t epochMask;
    uint8_t epoch;
/*  16 [offset] */
    uint8_t signatureVersion;
    uint8_t signatureType;
    uint8_t reserved2[2];
/*  20 [offset] */
} SUIF_Zeus42_RegionVerificationControllingParameters;

typedef SUIF_Zeus42_RegionVerificationControllingParameters SUIF_RegionVerificationControllingParameters;

#elif ZEUS_MAJOR_VERSION >= 5 || BHSM_ZEUS_VER_MAJOR >= 5

/* Note: use SUIF_Get32() to read words (32 bits) */
/* sizeof(SUIF_Zeus51_RegionVerificationControllingParameters) = 44 bytes */
typedef struct
{
/*   0 [offset] */
    uint8_t signatureVersion;
    uint8_t signatureType;
    uint8_t marketIdSelect;       /* [1:0] */
    uint8_t epochSelect;          /* [2:0] */
    uint8_t cpuType;
    uint8_t reserved0;
    uint8_t chipBindingSelect;    /* [5:0] */
    uint8_t reserved1;
    uint8_t reserved2[4];
    uint8_t marketId_BE[4];
/*  16 [offset] */
    uint8_t marketIdMask_BE[4];    /* when comparing to value read from OTP */
    uint8_t epoch_BE[4];
    uint8_t epochMask_BE[4];
    uint8_t upperChipsetBinding_BE[4];
/*  32 [offset] */
    uint8_t lowerChipsetBinding_BE[4];
    uint8_t metadata1[4];/* 4 bytes reserved for metadata */
    uint8_t metadata2[4];/* 4 bytes reserved for metadata */
/*  44 [offset]*/
} SUIF_Zeus51_RegionVerificationControllingParameters;

typedef SUIF_Zeus51_RegionVerificationControllingParameters SUIF_RegionVerificationControllingParameters;

#else
#error unsupported Zeus version
#endif


/******************************************************************
 * Common Header
 * --------------
 *   First define all the types and values used
 *   then define the "Secure Unified Image Format" common header
 ******************************************************************/

/*   use SUIF_Get32() to read words (32 bits)
 *   use SUIF_Get16() to read shorts (16 bits) */
/* sizeof(SUIF_CommonHeader) = 64 bytes */
#define SUIF_COMMON_HEADER_SIZE_BYTES (256)
#define SUIF_COMMON_HEADER_USEFUL_SIZE_BYTES (96) /* this is offsetof(SUIF_CommonHeader, pad) */
#define SUIF_COMMON_HEADER_PAD_SIZE_BYTES (SUIF_COMMON_HEADER_SIZE_BYTES-SUIF_COMMON_HEADER_USEFUL_SIZE_BYTES)
typedef struct
{
/*   0 [offset] */
    uint8_t magicNumber_BE[4];                       /* See SUIF_MagicNumber_e */
    uint8_t imageType;                               /* See SUIF_ImageType_e */
    uint8_t headerVersion;                           /* version of the structure used for the header; fixed 0x1 */
    uint8_t headerLength_BE[2];                      /* size of the image header (sizeof(SUIF_DummyImageHeader)); fixed to 1024 (0x0400) bytes */
    uint8_t SEType;                                  /* Secure Element type; see SUIF_SecureElementType_e */
    uint8_t SEVersion;                               /* Secure Element version; see SUIF_SecureElementVersion_e */
    uint8_t epochVersion;                            /* Software Epoch (antirollback) value */
    uint8_t key0Select;                              /* Key0 to use, see SUIF_Key0Select_e*/
    SUIF_Version imageVersion;                       /* Image Version (4 bytes: [major][minor][revision][branch]) */
/*  16 [offset] */
    uint8_t commonSectionSize_BE[2];                 /* 16 bits: sizeof(SUIF_CommonHeader) ; fixed to 256 (0x0100) bytes */
    uint8_t packagePadSize_BE[2];                    /* 16 bits: size of the package padding section (between header and metadata) */
    uint8_t tierKeyPackageSize_BE[2];                /* 16 bits: sizeof(SUIF_TierKeyPackage) */
    uint8_t controllingParamsSize_BE[2];             /* 16 bits: sizeof(SUIF_TierKeyControllingParameters) */
    uint8_t textSize_BE[4];                          /* 32 bits: size of the text-segment if any */
    uint8_t signedTextSize_BE[4];                    /* 32 bits: size of the text-segment that is signed (<= textSize) */
/*  32 [offset] */
    uint8_t dataSize_BE[4];                          /* 32 bits: size of the data-segment if any */
    uint8_t signedDataSize_BE[4];                    /* 32 bits: size of the data-segment that is signed (<= dataSize) */
    uint8_t metadataSize_BE[4];                      /* 32 bits: size of metadata */
    uint8_t fullPackageSize_BE[4];                   /* 32 bits: size of the full binary in bytes */
/*  48 [offset] */
    uint8_t signingScheme;                           /* See SUIF_SigningScheme_e */
    uint8_t textAndDataSignature;                    /* 0x0 text and data segments signed separatlely ; 0x1 text-segment signature also covers data-segment*/
    uint8_t BESigning;                               /* 0x0 text data and package are signed in Little Endian; 0x1 signed in Big endian */
    uint8_t reserved1;                               /* reserved byte */
    uint8_t zeroIvEncrypt;                           /* 0x0 text, data and metadata encrypted with non-0 IV; 0x1 encryped with all 0s IV */
    uint8_t BEEncryption;                            /* 0x0 all encrypted regions, including IV, are in Little Endian; 0x1 Big Endian */
    uint8_t noEncryption;                            /* 0x0 text and data segments are encrypted; 0x1 they are not encrypted */
    uint8_t reserved2[9];                            /* 9 reserved bytes */
/*  64 [offset] */
    SUIF_Version signingToolVersion;                 /* Signing tool version (4 bytes: [major][minor][revision][branch]) */
    uint8_t releaseType;                             /*  */
    uint8_t configVersion;                           /*  */
    uint8_t testProfile;                             /*  */
    uint8_t reserved3;                               /* reserved byte */
    uint8_t chipNumberAndRev_BE[4];                  /* 32bits: Chip number and revision, example: 0x007439B0 */
/*  76 [offset] */
    uint8_t signingProfileName[20];                  /* Name of the signing profile: "ZD", "NC", "AP_TRIPLE", ... */
/*  96 [offset] */
    uint8_t pad[SUIF_COMMON_HEADER_PAD_SIZE_BYTES];  /* padding/reserved bytes to meat the fixed common header size */
/* 256 [offset] */
} SUIF_CommonHeader;

#define SUIF_HEADER_SIZE_BYTES (1024) /* All images header should be SUIF_HEADER_SIZE_BYTES bytes */

/* to accomodate the size required for all header, dummy header needs to compute the size of padding required */
#define SUIF_DUMMY_IMG_PAD_SIZE_BYTES (SUIF_HEADER_SIZE_BYTES - sizeof(SUIF_CommonHeader))

typedef struct {
/*   0 [offset] */
    SUIF_CommonHeader common;
    /* This dummy header does not contain any image specific fields, it is not a valid header.
       The image specific fields should be defined here, and PAD size adjusted to always match 1k total size */
    uint8_t pad[SUIF_DUMMY_IMG_PAD_SIZE_BYTES];
/*  1k [offset] */
} SUIF_DummyImageHeader;

/* All head of binaries will be shaped and of size of SUIF_DummyHead */
#define SUIF_TIER_KEY_PACKAGES_NUM 3
typedef struct {
    SUIF_DummyImageHeader imageHeader;
    /* in case of single signing (signingScheme = _eNormal), only tierKeyPackages[0] is used */
    SUIF_TierKeyPackage tierKeyPackages[SUIF_TIER_KEY_PACKAGES_NUM];
} SUIF_PackageHeader;

/* for each region verified we need
   - a region verification controlling parameters structure
   - 2 signatures */
typedef struct {
    SUIF_RegionVerificationControllingParameters controllingParameters;
    uint8_t signature0[SUIF_TIER_RSAKEY_SIG_SIZE_BYTES]; /* use signature0 for single signing (signingScheme = _eNormal) */
    uint8_t signature1[SUIF_TIER_RSAKEY_SIG_SIZE_BYTES]; /* may use signature1 for triple signing (signingScheme = _eTriple) */
} SUIF_RegionVerification;

/* fixed size of 256 bits i.e. 32 bytes */
#define SUIF_SHA256_SIZE_BYTES (256/8)

/* the footer of the package
   includes the sha256 of the metadata
   all the signatures and controlling parameters for text, data
   and also the whole package (binary) verification */
typedef struct {
    uint8_t metadataSha256[SUIF_SHA256_SIZE_BYTES];
    SUIF_RegionVerification textVerification;
    SUIF_RegionVerification dataVerification;
    SUIF_RegionVerification packageVerification;
} SUIF_PackageFooter;

/* All binaries are structured as follow:
   HEADER | variable size sections | FOOTER :
   {
       SUIF_PackageHeader header;
       uint8_t text[];              // variable size (header.imageHeader.common.textSize_BE bytes)
       uint8_t data[];              // variable size (header.imageHeader.common.dataSize_BE bytes)
       uint8_t metaData[];          // variable size (header.imageHeader.common.metadataSize_BE bytes)
       uint8_t packagePadding[];    // variable size (header.imageHeader.common.packagePadSize bytes)
       SUIF_PackageFooter footer;
   }
 */

#endif /* SUIF_H__ */
