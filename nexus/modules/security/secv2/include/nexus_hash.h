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

#ifndef NEXUS_HASH_H__
#define NEXUS_HASH_H__

#include "nexus_types.h"
#include "nexus_base_types.h"
#include "nexus_keyladder.h"
#include "nexus_security_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_Hash* NEXUS_HashHandle;

#define NEXUS_HASH_MAX_LENGTH (32)

/*
    Structure used to configure a HASH context.
*/
typedef struct NEXUS_HashSettings
{
    NEXUS_HashType hashType;       /* sha160, sha256, etc. */

    bool appendKey;                /* append key to data to be hashed */

    struct  /* if appendKey==true, specify key; either keyladder or software key. */
    {
        struct
        {
            NEXUS_KeyLadderHandle handle;   /* Handle of keyladder */
            unsigned level;                 /* the keyladder layer to collect the key from. */
        }keyladder;

        uint8_t softKey[NEXUS_HASH_MAX_LENGTH];  /* SoftKey be used if appendKey==true and key.keyladder.handle==NULL */
        unsigned softKeySize;                    /* The size of the sowtware key to be added to data. */
    }key;

}NEXUS_HashSettings;

typedef struct NEXUS_HashData
{
    NEXUS_Addr dataOffset;       /* data to be hashed. */
    unsigned dataSize;           /* size of data. */
    bool last;                   /* set to true if it's the last chunk of the data to be hashed. */
}NEXUS_HashData;

typedef struct NEXUS_HashResult
{
    uint8_t hash[NEXUS_HASH_MAX_LENGTH];
    unsigned hashLength;

}NEXUS_HashResult;

/*
Description:
    Create a HASH context. There is no limit on the number of hash contexts that can be created.
*/
NEXUS_HashHandle NEXUS_Hash_Create( /* attr{destructor=NEXUS_Hash_Destroy} */
    void
    );

/*
Description:
    Destroy HASH context.
*/
void NEXUS_Hash_Destroy(
    NEXUS_HashHandle handle
    );

/*
Description:
    Retrieve default settings.
*/
void NEXUS_Hash_GetDefaultSettings(
    NEXUS_HashSettings *pSettings /* [out] */
    );

/*
Description:
    Configure the HASH context.
*/
NEXUS_Error NEXUS_Hash_SetSettings(
    NEXUS_HashHandle handle,
    const NEXUS_HashSettings *pSettings
    );


/*
Description:
    Retrieve default data.
*/
void NEXUS_Hash_GetDefaultData(
    NEXUS_HashData *pData /* [out] */
    );


/*
Description:
    Add data to hash. NEXUS_Hash_SubmitData is synchronous, it will block until the request is complete.

    Return values:
        - NEXUS_SUCCESS.  The request has succeeded.
        - NEXUS_UNKNOWN.
*/
NEXUS_Error NEXUS_Hash_SubmitData(
    NEXUS_HashHandle handle,
    const NEXUS_HashData *pData,  /* The data to be hashed. */
    NEXUS_HashResult *pResult       /* [out] attr{null_allowed=y} The resulting HASH. Only referensed if pData->last is true. */
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_HASH_H__ */
