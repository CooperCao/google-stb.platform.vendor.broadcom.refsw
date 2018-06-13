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
#ifndef NEXUS_HMAC_SHA_CMD_H__
#define NEXUS_HMAC_SHA_CMD_H__

#include "nexus_security_datatypes.h"
#include "nexus_security.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_HMACSHA_KEY_LEN           32       /* 20 for Sha1, 28 for Sha224, 32 for Sha256 */
#define NEXUS_HMACSHA_DIGEST_LEN	    32       /* in bytes, same as HMACSHA key length */

/**
Summary:

Description:
See Also:
**/
typedef enum NEXUS_HMACSHA_Op
{
    NEXUS_HMACSHA_Op_eSHA,   /* Perform a SHA HASH Operation */
    NEXUS_HMACSHA_Op_eHMAC,  /* Perform a HMAC Operation */
    NEXUS_HMACSHA_Op_eMax

} NEXUS_HMACSHA_Op;

/**
Summary:

Description:
See Also:
**/
typedef enum NEXUS_HMACSHA_KeyInclusion_Op
{
    NEXUS_HMACSHA_KeyInclusion_Op_eNo      = 0,
    NEXUS_HMACSHA_KeyInclusion_Op_eAppend  = 1  /* Include key in data to be hashed */

} NEXUS_HMACSHA_KeyInclusion_Op;

/**
Summary:

Description:
See Also:
**/
typedef enum NEXUS_HMACSHA_SHAType
{
    NEXUS_HMACSHA_SHAType_eSha160 = 0,
    NEXUS_HMACSHA_SHAType_eSha224 = 1,
	NEXUS_HMACSHA_SHAType_eSha256 = 2,
	NEXUS_HMACSHA_SHAType_eMax

} NEXUS_HMACSHA_SHAType;

/**
Summary:

Description:
See Also:
**/
typedef enum NEXUS_HMACSHA_BSPContext
{
    NEXUS_HMACSHA_BSPContext_eContext0,
    NEXUS_HMACSHA_BSPContext_eContext1

} NEXUS_HMACSHA_BSPContext;


/**
Summary:

Description:
See Also:
**/
typedef enum NEXUS_HMACSHA_DataMode
{
	NEXUS_HMACSHA_DataMode_eAllIn = 0x00, /* Single call. All source data included  */
	NEXUS_HMACSHA_DataMode_eMore  = 0x01, /* Multiple calls. More source data to follow in subsequent call. */
	NEXUS_HMACSHA_DataMode_eLast  = 0x10  /* Multiple calls. Last of source data. */

} NEXUS_HMACSHA_DataMode;

/**
**/
typedef enum NEXUS_HMACSHA_DataSource
{
    NEXUS_HMACSHA_DataSource_eCmdBuf = 0,  /* DEPRECATED. Not supported. */
    NEXUS_HMACSHA_DataSource_eDRAM   = 1

} NEXUS_HMACSHA_DataSource;


/**
Summary:

Description:
See Also:
**/
typedef enum NEXUS_HMACSHA_KeySource
{
    NEXUS_HMACSHA_KeySource_eKeyVal    = 0,
    NEXUS_HMACSHA_KeySource_eKeyLadder = 1

} NEXUS_HMACSHA_KeySource;


/**
Summary:
This type defines the data structure for HMAC/SHA operation

Description:
See Also:
NEXUS_HMACSHA_PerformOp
**/
typedef struct NEXUS_HMACSHA_OpSettings
{
	NEXUS_HMACSHA_Op                  opMode;    /* Which operation -- Hmac or Sha */
	NEXUS_HMACSHA_SHAType             shaType;   /* Select SHA type */
	NEXUS_HMACSHA_BSPContext          context;   /* Which context to perform the op. It is the client responsibility to maintains the contexts. */
	NEXUS_HMACSHA_KeySource           keySource; /* Select software or key ladder key.  HMAC only*/
	NEXUS_SecurityVirtualKeyladderID  VKL;       /* If key is from key ladder - Which VKL. HMAC only*/
	NEXUS_SecurityKeySource           keyLayer;  /* If key is from key ladder - Which Key layer. HMAC Only */
	uint32_t                          keyLength; /* Key Length. */
	unsigned char                     keyData[NEXUS_HMACSHA_KEY_LEN]; /* BE byte array */
	NEXUS_HMACSHA_DataMode            dataMode;  /* Data provided in single or multiple calls. */
	NEXUS_HMACSHA_KeyInclusion_Op     keyIncMode;/* Append key to source data before sha operation. Sha Only */
	bool                              byteSwap;  /* Endian Swap the source data. This will add a processing overhead */
    uint32_t                          dataSize;
    uint8_t                           *dataAddress; /* attr{memory=cached;null_allowed=y} */
	NEXUS_HMACSHA_DataSource          dataSrc;   /* DEPRECATED. Parameter ignored. */
	uint32_t                          includeKey;/* DEPRECATED. Not used. See keyIncMode */

} NEXUS_HMACSHA_OpSettings;


/**
Summary:
This type defines the data structure for Digest output from a HMAC/SHA operation

Description:
See Also:
NEXUS_HMACSHA_PerformOp
**/
typedef struct NEXUS_HMACSHA_DigestOutput
{
	unsigned char        digestData[NEXUS_HMACSHA_DIGEST_LEN]; /* BE byte array */
    uint32_t             digestSize;

} NEXUS_HMACSHA_DigestOutput;


/*****************************************************************************
Summary:
Get default settings
*****************************************************************************/
void NEXUS_HMACSHA_GetDefaultOpSettings(
	NEXUS_HMACSHA_OpSettings *pOpSettings		/* [out] */
    );

/*****************************************************************************
Summary:
Perform the requested HMAC/SHA operation
*****************************************************************************/
NEXUS_Error NEXUS_HMACSHA_PerformOp(
    const NEXUS_HMACSHA_OpSettings *pOpSettings,    /* structure holding input parameters */
    NEXUS_HMACSHA_DigestOutput     *pOutput         /* [out] structure holding digest buffer and size */
    );



#ifdef __cplusplus
}
#endif

#endif
