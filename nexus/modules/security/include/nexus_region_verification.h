/******************************************************************************
* Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
******************************************************************************/

#ifndef NEXUS_REGION_VERIFICATION__
#define NEXUS_REGION_VERIFICATION__

#include "nexus_types.h"
#include "nexus_security_datatypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
    This API allows the NEXUS client to authenticate/verify an area of memory (a region)
    against a box secret key. Once verifcation is enabled on a region, it will be
    background checked against the signature until it's disabled.
******************************************************************************/

#define NEXUS_REGIONVERIFY_SIGNATURE_SIZE (256)
#define NEXUS_REGIONVERIFY_SIGNATURE_PLUS_HEADER_SIZE (NEXUS_REGIONVERIFY_SIGNATURE_SIZE+20)

typedef struct NEXUS_RegionVerify *NEXUS_RegionVerifyHandle;

typedef struct NEXUS_RegionVerifySettings
{
    bool enabled;              /* true to enable region verification on the region, false to disable.
                                  The following configuration parameters are only relevant when enabling
                                  verification.  */
    NEXUS_Addr regionAddress;  /* physical address */
    unsigned regionSize;

    struct{
        unsigned size;                      /* signature size. */
        uint8_t  data[NEXUS_REGIONVERIFY_SIGNATURE_PLUS_HEADER_SIZE];      /* the signature  */

         struct{
            uint32_t marketId;              /* Market Id */
            uint32_t marketIdMask;          /* Market Id Mask */
            uint8_t  systemEpoch;           /* epoch*/
            uint8_t  systemEpochMask;       /* mask of Epoch bits */
            bool     reLocatable;           /* if true (default) the image is not bound it to a particular memory location. */
            uint8_t  systemEpochSelect;     /* select the appropriate Epoch [Zeus4.2+] */
            uint8_t  signatureVersion;      /* the signature layout version. [Zeus4.2+]*/
        }attributes;        /* attributes included within the signature. */

    }signature;             /* Signature of region to be verified. */

    unsigned rsaKeyIndex;   /* The RSA key index used to authenticate the memory region. */

} NEXUS_RegionVerifySettings;


/**
    Allocate a Region Verification resource that will allow the client to authenticate and background check an area of memory.
**/
NEXUS_RegionVerifyHandle NEXUS_RegionVerify_Open(  /* attr{destructor=NEXUS_RegionVerify_Close} */
    unsigned index /* supports NEXUS_ANY_ID */
    );

/**
    Free the Region Verification resource.
**/
void NEXUS_RegionVerify_Close(
    NEXUS_RegionVerifyHandle handle
    );

void NEXUS_RegionVerify_GetSettings(
    NEXUS_RegionVerifyHandle handle,
    NEXUS_RegionVerifySettings *pSettings
    );

/**
    Configure and enable or disable verification on the region.
**/
NEXUS_Error NEXUS_RegionVerify_SetSettings(
    NEXUS_RegionVerifyHandle handle,
    const NEXUS_RegionVerifySettings *pSettings
    );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
