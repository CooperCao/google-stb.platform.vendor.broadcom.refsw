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


/*
    The NEXUS Keyladder API enables the derivation of a key from a box secret (root key) and
    one or more ladder keys (ladderKeys). The derived key can be routed to a keyslot entry. The derived
    or intermediate ladder keys are never accessible to the user.

    The root key can be configured by 1 of 3 mechanisms. These are:
      1. OTP Direct. This uses a key programmed into the on chip's OTP (One Time Programmable) memory as the root key.
      2. OTP via ASKM (Advanced Symmetric Key Management). This mechanism diffuses an OTP with CA vendor specific
         configuration parameters to derive the root key. It allows an OTP key to be used independently by multiple
         CA Vendors.
      3. Global Key. This mechanism derives the root key from a set of "Global Keys" prgrammed on the OTP.

    Control if the keyladder is as follows:
    - Ladder configuration:
        - specify how the root key if to be generated.
        - configure ladder behaviour.
            - keyladder mode
            - algroithm
            - number of levels.
            - destination of key
        - configure contribution key, if required.
    - Load ladderKeys (input key data.)
*/

#ifndef NEXUS_KEYLADDER__H_
#define NEXUS_KEYLADDER__H_

#include "nexus_types.h"
#include "nexus_keyslot.h"
#include "nexus_security_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define NEXUS_KEYLADDER_LADDER_KEY_SIZE (16)
#define NEXUS_KEYLADDER_CHALLENGE_NONCE_MAX_LEN  (16)
#define NEXUS_KEYLADDER_CHALLENGE_RESPONSE_MAX_LEN  (16)

#define NEXUS_HWKL_ID (0xFEDCCDEF)

typedef struct NEXUS_KeyLadder* NEXUS_KeyLadderHandle;

typedef struct NEXUS_KeyLadderAllocateSettings
{
    NEXUS_SecurityCpuContext owner; /* used to specify whether the keyladder is intended for HOST or SAGE usage. */
} NEXUS_KeyLadderAllocateSettings;


/*
    Describes how the keyladder is to behave.
*/
typedef enum NEXUS_KeyLadderMode
{
    NEXUS_KeyLadderMode_eCa_64_4,      /* Generates 64 bit key that must be routed to a CP block from level 4. */
    NEXUS_KeyLadderMode_eCp_64_4,      /* Generates 64 bit key that must be routed to a CA block from level 4. */
    NEXUS_KeyLadderMode_eCa_64_5,
    NEXUS_KeyLadderMode_eCp_64_5,
    NEXUS_KeyLadderMode_eCa_128_4,
    NEXUS_KeyLadderMode_eCp_128_4,
    NEXUS_KeyLadderMode_eCa_128_5,
    NEXUS_KeyLadderMode_eCp_128_5,
    NEXUS_KeyLadderMode_eCa_64_7,
    NEXUS_KeyLadderMode_eCa_128_7,
    NEXUS_KeyLadderMode_eGeneralPurpose1,
    NEXUS_KeyLadderMode_eGeneralPurpose2,
    NEXUS_KeyLadderMode_eCa64_45,
    NEXUS_KeyLadderMode_eCp64_45,
    NEXUS_KeyLadderMode_eHwlk,
    NEXUS_KeyLadderMode_eEtsi_5,
    NEXUS_KeyLadderMode_eScte52Ca_5,
    NEXUS_KeyLadderMode_eSageBlDecrypt,
    NEXUS_KeyLadderMode_eSage128_5,
    NEXUS_KeyLadderMode_eSage128_4,
    /* more to come */
    NEXUS_KeyLadderMode_eMax
} NEXUS_KeyLadderMode;


/* the different menchanisms for gererating a KeyLadder root key. */
typedef enum NEXUS_KeyLadderRootType
{
    NEXUS_KeyLadderRootType_eOtpDirect,         /* Use OTP as root key direct.  */
    NEXUS_KeyLadderRootType_eOtpAskm,           /* derive root key from OTP via ASKM diffusion. */
    NEXUS_KeyLadderRootType_eGlobalKey,         /* derive root key from OTP Global keys. Uses ASKM. */
    NEXUS_KeyLadderRootType_eCustomerKey,       /* derive rook from customer keys. [note: maybe deprecated for zeus5.] */
    NEXUS_KeyLadderRootType_eMax
} NEXUS_KeyLadderRootType;


/* the scope of the ASKM CA Vendor ID */
typedef enum NEXUS_KeyladderCaVendorIdScope
{
    NEXUS_KeyladderCaVendorIdScope_eChipFamily,     /* CA Vendor ID generates root key differently per chip family */
    NEXUS_KeyladderCaVendorIdScope_eFixed,          /* CA Vendor ID generates root key independent of chip family. */
    NEXUS_KeyladderCaVendorIdScope_eMax
}NEXUS_KeyladderCaVendorIdScope;

/* how the ASKM STB owner ID is to be determined */
typedef enum NEXUS_KeyLadderStbOwnerIdSelect
{
    NEXUS_KeyLadderStbOwnerIdSelect_eOtp,           /* Read StbOwnerId from OTP */
    NEXUS_KeyLadderStbOwnerIdSelect_eOne,           /* Hardcode StbOwnerId to One */
    NEXUS_KeyLadderStbOwnerIdSelect_eZero,          /* Hardcode StbOwnerId to Zero */
    NEXUS_KeyLadderStbOwnerIdSelect_eMax
}NEXUS_KeyLadderStbOwnerIdSelect;

/* how the Global Key Owner Id is to be determined. */
typedef enum NEXUS_KeyLadderGlobalKeyOwnerIdSelect
{
    NEXUS_KeyLadderGlobalKeyOwnerIdSelect_eMsp0,            /* read owner from MSP OTP loaction 0 */
    NEXUS_KeyLadderGlobalKeyOwnerIdSelect_eMsp1,            /* read owner from MSP OTP loaction 1 */
    NEXUS_KeyLadderGlobalKeyOwnerIdSelect_eOne,             /* hardcode to 1 */
    NEXUS_KeyLadderGlobalKeyOwnerIdSelect_eMax
} NEXUS_KeyLadderGlobalKeyOwnerIdSelect;


/* describes where the generated key is to be routed. */
typedef enum NEXUS_KeyLadderDestination
{
    NEXUS_KeyLadderDestination_eNone,         /* not routed, key remains on ladder level. */
    NEXUS_KeyLadderDestination_eKeyslotKey,   /* a keyslot key location */
    NEXUS_KeyLadderDestination_eKeyslotIv,    /* a keyslot IV location */
    NEXUS_KeyLadderDestination_eKeyslotIv2,   /* a keyslot 2nd IV location */
    NEXUS_KeyLadderDestination_eHdcp,         /* HDCP */
    NEXUS_KeyLadderDestination_eMax
}NEXUS_KeyLadderDestination;

typedef enum NEXUS_KeyLadderSwizzelType
{
    NEXUS_KeyLadderSwizzelType_eNone = 0,
    NEXUS_KeyLadderSwizzelType_e1    = 1,
    NEXUS_KeyLadderSwizzelType_e0    = 2,
    NEXUS_KeyLadderSwizzelType_eMax
}NEXUS_KeyLadderSwizzelType;


typedef struct NEXUS_KeyLadderSettings
{
    NEXUS_CryptographicAlgorithm algorithm;
    NEXUS_CryptographicOperation operation;
    NEXUS_KeyLadderMode mode;   /* Describes the allowed operations on the keyladder. */

    struct
    {
        NEXUS_KeyLadderRootType type;
        bool swapRootKey;       /* if true, the key's 8 top bytes will be swapped with lower 8.  */

        /* valid if root.type is NEXUS_KeyLadderRootType_eOtpDirect or NEXUS_KeyLadderRootType_eOtpAskm */
        unsigned otpKeyIndex;         /* Specifies which OTP key is to be used. */

        /* valid if root.type is NEXUS_KeyLadderRootType_eOtpAskm or NEXUS_KeyLadderRootType_eGlobalKey */
        struct{
            unsigned                        caVendorId;      /* id unique to CA vendor. */
            NEXUS_KeyladderCaVendorIdScope  caVendorIdScope;
            NEXUS_KeyLadderStbOwnerIdSelect stbOwnerSelect;  /* allow owner/boradcaster to add diffusion to key */
            bool swapKey;                                    /* swap upper and lower 64 bits of Askm key2 (3DesAba keyladder only). */
        }askm;

        /* valid if root.type is NEXUS_KeyLadderRootType_eGlobalKey */
        struct{
            NEXUS_KeyLadderGlobalKeyOwnerIdSelect owner;
            unsigned                              index;     /* identify what global key to select. */
        }globalKey;

        /* valid if root.type is NEXUS_KeyLadderRootType_eCustomerKey. */
        struct {
            NEXUS_KeyLadderSwizzelType type;
            unsigned swizzle1IndexSel;
            bool enableSwizzle0a;
            struct {
                unsigned keyVar;
                unsigned keyIndex;
                bool decrypt;    /* 0 encrypt, 1 decrypt */
            }low, high;
        }customerKey;

    }root;

    /* valid if mode is NEXUS_KeyLadderMode_eHwlk*/
    struct{
        unsigned  numlevels;     /* length of the hwkl*/
        NEXUS_CryptographicOperation  algorithm;  /* algorithm of the hwkl*/
        uint8_t    moduleId;     /* moduleId to use */
    }hwkl;

} NEXUS_KeyLadderSettings;


/**
    Structure to specify the level key. If the key is to be routed, the key's destination can be configured.
**/
typedef struct NEXUS_KeyLadderLevelKey
{
    unsigned level;            /* an index value between 3 and max allowed by the keyladder mode. */
    uint8_t ladderKey[NEXUS_KEYLADDER_LADDER_KEY_SIZE]; /* know as "procIn" in secV1. */
    unsigned ladderKeySize;           /* ladderKey size on bits. Must be less than BHSM_KEYLADDER_LADDER_KEY_SIZE*8 */

    /* describes where the generated key is to be routed. */
    struct
    {
        NEXUS_KeyLadderDestination destination;

        /* valid if destination is keyslot key or IV */
        struct
        {
            NEXUS_KeySlotHandle handle;     /* destination keyslot */
            NEXUS_KeySlotBlockEntry  entry; /* destination block and polarity within keyslot.*/
        }keySlot;
    }route;

} NEXUS_KeyLadderLevelKey;


typedef struct NEXUS_KeyLadderChallenge
{
    unsigned nonceSize;
    uint8_t nonce[NEXUS_KEYLADDER_CHALLENGE_NONCE_MAX_LEN];
}NEXUS_KeyLadderChallenge;

typedef struct NEXUS_KeyLadderChallengeResponse
{
    unsigned responseSize;
    uint8_t response[NEXUS_KEYLADDER_CHALLENGE_RESPONSE_MAX_LEN];
}NEXUS_KeyLadderChallengeResponse;

typedef struct NEXUS_KeyLadderInfo
{
    unsigned index;     /* The index of the keyladder. The same index specified on NEXUS_KeyLadder_Allocate */
}NEXUS_KeyLadderInfo;

void NEXUS_KeyLadder_GetDefaultAllocateSettings(
    NEXUS_KeyLadderAllocateSettings *pSettings
    );

/*
    Allocate a KeyLadder Resource
*/
NEXUS_KeyLadderHandle NEXUS_KeyLadder_Allocate(        /* attr{destructor=NEXUS_KeyLadder_Free}  */
    unsigned index,                                    /* supports NEXUS_ANY_ID to allow NEXUS manage KeyLadder instances. */
    const NEXUS_KeyLadderAllocateSettings *pSettings
    );

/*
    Free a KeyLadder Resource
*/
void NEXUS_KeyLadder_Free(
    NEXUS_KeyLadderHandle handle
    );

/*
    returns the current or default setting for the keyladder.
*/
void NEXUS_KeyLadder_GetSettings(
    NEXUS_KeyLadderHandle handle,
    NEXUS_KeyLadderSettings *pSettings                  /* [out] */
    );

/*
    Configure the keyladder.
*/
NEXUS_Error NEXUS_KeyLadder_SetSettings(
    NEXUS_KeyLadderHandle handle,
    const NEXUS_KeyLadderSettings *pSettings
    );




void  NEXUS_KeyLadder_GetLevelKeyDefault( NEXUS_KeyLadderLevelKey *pKey );

/*
    Progress the key down the keyladder.
*/
NEXUS_Error NEXUS_KeyLadder_GenerateLevelKey(
    NEXUS_KeyLadderHandle handle,
    const NEXUS_KeyLadderLevelKey  *pKey
    );

/*
    Returns information on the keyladder instance.
*/
NEXUS_Error NEXUS_KeyLadder_GetInfo(
    NEXUS_KeyLadderHandle handle,
    NEXUS_KeyLadderInfo  *pInfo
    );

void NEXUS_KeyLadder_GetDefaultChallenge(
    NEXUS_KeyLadderChallenge  *pChallenge         /* [out] */
    );

/*
    Challenge the keyladder. Can be used to authenticate the STB.
*/
NEXUS_Error NEXUS_KeyLadder_Challenge(
    NEXUS_KeyLadderHandle handle,
    const NEXUS_KeyLadderChallenge  *pChallenge,
    NEXUS_KeyLadderChallengeResponse *pResponse    /* [out] */
    );

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
