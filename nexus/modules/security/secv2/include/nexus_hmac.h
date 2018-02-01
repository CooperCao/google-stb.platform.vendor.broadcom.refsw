/******************************************************************************
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

 ******************************************************************************/

#ifndef NEXUS_HMAC_H__
#define NEXUS_HMAC_H__

#include "nexus_types.h"
#include "nexus_base_types.h"
#include "nexus_keyladder.h"
#include "nexus_security_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_Hmac* NEXUS_HmacHandle;

#define NEXUS_HMAC_MAX_LENGTH (32)

/* used to identify the HMAC key source. */
typedef enum NEXUS_HmacKeySource
{
    NEXUS_HmacKeySource_eKeyLadder,
    NEXUS_HmacKeySource_eSoftwareKey,

    NEXUS_HmacKeySource_eMax
}NEXUS_HmacKeySource;


/*
    Structure used to configure a HMAC context.
*/
typedef struct NEXUS_HmacSettings
{
    NEXUS_HashType hashType;       /* Only SHA hashes supported. */

    struct  /* specify key; either keyladder or software key. */
    {
        NEXUS_HmacKeySource source;         /* specify the hmac key source. */

        struct
        {
            NEXUS_KeyLadderHandle handle;   /* Handle of keyladder */
            unsigned level;                 /* the keyladder layer to collect the key from. */
        }keyladder;                         /* valid if source == NEXUS_HmacKeySource_eKeyLadder */

        uint8_t softKey[NEXUS_HMAC_MAX_LENGTH];  /* key length will be equal to Sha Alorithm block size.
                                                  valid of source == NEXUS_HmacKeySource_eSoftwareKey  */
    }key;

}NEXUS_HmacSettings;


/*
    Structure used to pass HMAC data.
*/
typedef struct NEXUS_HmacData
{
    NEXUS_Addr dataOffset;  /* data to be hashed. */
    unsigned dataSize;      /* size of data in bytes. */
    bool last;              /* set to true if it's the last part of data to contribute to the HMAC. */
}NEXUS_HmacData;


/*
    Structure used to return the HMAC results.
*/
typedef struct NEXUS_HmacResult
{
    uint8_t hmac[NEXUS_HMAC_MAX_LENGTH];
    unsigned hmacLength;
}NEXUS_HmacResult;

/*
    Create a HMAC context.
*/
NEXUS_HmacHandle NEXUS_Hmac_Create( /* attr{destructor=NEXUS_Hmac_Destroy} */
    void
    );
/*
    Destroy HMAC context.
*/
void NEXUS_Hmac_Destroy(
    NEXUS_HmacHandle handle
    );

/*
    Retrieve default settings.
*/
void NEXUS_Hmac_GetDefaultSettings(
    NEXUS_HmacSettings *pSettings  /* [out] */
    );

/*
    Configure the HMAC context.
*/
NEXUS_Error NEXUS_Hmac_SetSettings(
    NEXUS_HmacHandle handle,
    const NEXUS_HmacSettings *pSettings
    );

/*
    Retrieve default data settings.
*/
void NEXUS_Hmac_GetDefaultData(
    NEXUS_HmacData *pData           /* [out] */
    );

/*
Description:
    Add data to HMAC. NEXUS_Hmac_SubmitData is synchronous, it will block until the operation is complete.

    Return values:
        - NEXUS_SUCCESS.        The request has succeeded.
        - NEXUS_UNKNOWN.
*/
NEXUS_Error NEXUS_Hmac_SubmitData(
    NEXUS_HmacHandle handle,
    const NEXUS_HmacData *pData,  /* The data to be part of the HMAC.  */
    NEXUS_HmacResult *pResult      /* [out] attr{null_allowed=y} The generated HMAC. Only referenced if pData->last is true. */
    );


#ifdef __cplusplus
}
#endif

#endif /* NEXUS_SHA_H__ */
