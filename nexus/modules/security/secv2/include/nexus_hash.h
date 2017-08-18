/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#define NEXUS_HASH_MAX_LENGTH (16)
#define NEXUS_HASH_LAST_DATA  (1)

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

        uint8_t softKey[NEXUS_HASH_MAX_LENGTH];  /* key length will be equal to Sha Alorithm block size.
                                                    softKey be used if appendKey==true and key.keyladder.handle==NULL */
    }key;

   NEXUS_CallbackDesc hashComplete;      /* callback to notify client when NEXUS has completed the RSA request. Client to
                                            call NEXUS_Hash_Poll to retrieve result. The client can specify a NULL callback
                                            and use NEXUS_Hash_GetResult to check for completion. */
}NEXUS_HashSettings;

typedef struct NEXUS_HashResult
{
    uint8_t data[NEXUS_HASH_MAX_LENGTH];
    unsigned dataLength;

}NEXUS_HashResult;

/*
Description:
    Create a HASH context. There is no limit on the number of hash contexts that can be created.
*/
NEXUS_HashHandle NEXUS_Hash_Create(void); /* attr{destructor=NEXUS_Hash_Destroy} */

/*
Description:
    Destroy HASH context.
*/
void NEXUS_Hash_Destroy(
    NEXUS_HashHandle handle
    );

/*
Description:
    Retrieve current or default settings.
*/
void NEXUS_Hash_GetSettings(
    NEXUS_HashHandle handle,
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
    Add data to hash. NEXUS_Hash_SubmitData can be called multiple times on an instance, the data will be
    queued internally and processed in sequence.

    Return values:
        - NEXUS_SUCCESS.        The request is queued
        - NEXUS_UNKNOWN.
*/
NEXUS_Error NEXUS_Hash_SubmitData(
    NEXUS_HashHandle handle,
    NEXUS_Addr dataOffset,       /* data to be hashed. */
    unsigned dataSize,           /* size of data. */
    bool last                    /* set to true (NEXUS_HASH_LAST_DATA) if it's that last chunk of data to be hashed. */
    );


/*
    Poll NEXUS to check if a request has completed and return the calculated HASH.
    Return values:
        - NEXUS_SECURITY_PENDING      Context busy processing data.
        - NEXUS_SECURITY_WAITING      Context waiting for more data (see "last" flag above).
        - NEXUS_SUCCESS               Data is available and returned within pResult.
        - NEXUS_UNKNOWN
*/
NEXUS_Error NEXUS_Hash_GetResult(
    NEXUS_HashHandle handle,
    NEXUS_HashResult *pResult /* [out] */
    );


#ifdef __cplusplus
}
#endif

#endif /* NEXUS_HASH_H__ */
