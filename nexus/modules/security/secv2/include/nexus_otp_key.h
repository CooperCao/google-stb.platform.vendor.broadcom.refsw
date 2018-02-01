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

#include "nexus_keyladder.h"

/**
These functions read parameters related to specifice OTP keys.
There are ~9 OTP Keys (number dependent on the chip). The "index" paramter
identifies a particular OTP key.
**/

#ifndef NEXUS_OTP_KEY__H_
#define NEXUS_OTP_KEY__H_

#include "nexus_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define NEXUS_OTP_KEY_HASH_LENGTH     (8)
#define NEXUS_OTP_KEY_ID_LENGTH       (8)
#define NEXUS_OTP_KEY_PROGRAM_LENGTH  (16)

/*
    Structure used to read OTP Key info
*/
typedef struct NEXUS_OtpKeyInfo
{
    uint8_t hash[NEXUS_OTP_KEY_HASH_LENGTH];  /* Hash of OTP key */
    uint8_t id[NEXUS_OTP_KEY_ID_LENGTH];      /* OTP KeyId */

    unsigned blackBoxId;
    bool     caKeyLadderAllow;  /* Derived Keys can be routed to CA keyslot block */
    bool     cpKeyLadderAllow;  /* Derived Keys can be routed to CPS/CPD keyslot blocks */
    bool     gp1KeyLadderAllow; /* General Purpose keyladder 1 allowed. */
    bool     gp2KeyLadderAllow; /* General Purpose keyladder 2 allowed. */
    bool     sageKeyLadderAllow; /* OTP key can be used from SAGE. */
    bool     rootKeySwapAllow;   /* OTP root key could be swapped. */
    bool     deobfuscationEnabled; /* OTP key is obfuscated.  */
    unsigned customerMode;

}NEXUS_OtpKeyInfo;


/*
    Structure used to program OTP Key
*/
typedef struct NEXUS_OtpKeyProgram
{
    uint8_t keyData[NEXUS_OTP_KEY_PROGRAM_LENGTH];  /* encrypted key data. */

    struct
    {
        NEXUS_KeyLadderHandle handle;          /* keyLadder used to decrypt keydata */
        unsigned level;                        /* keyLadder level */
    }keyladder;                                /* keyladder used to decrypt KeyData. */

}NEXUS_OtpKeyProgram;



/*
Description:
   Returns information on the specified OTP Key
*/
NEXUS_Error NEXUS_OtpKey_GetInfo(
    unsigned index,                 /* Identify the OTP Key, Index 0 refers to OTP "Key A" */
    NEXUS_OtpKeyInfo *pKeyInfo      /* [out] The returned Key information.  */
    );



/*
    Programs an OTP Key.
*/
NEXUS_Error NEXUS_OtpKey_Program(
    unsigned index,                 /* Identify the OTP Key, Index 0 refers to OTP "Key A" */
    const NEXUS_OtpKeyProgram *pKeyConfig
    );

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
