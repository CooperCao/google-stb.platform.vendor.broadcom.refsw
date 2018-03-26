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
         configuration paramters to derive the root key. It allows an OTP key to be used independently by multiple
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
    - Load ladderKeys (input key data.)
*/

#ifndef BHSM_KEYLADDER__H_
#define BHSM_KEYLADDER__H_

#include "bhsm_keyslot.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BHSM_KEYLADDER_LADDER_KEY_SIZE (16)
#define BHSM_KEYLADDER_CHALLENGE_NONCE_MAX_LEN  (16)
#define BHSM_KEYLADDER_CHALLENGE_RESPONSE_MAX_LEN  (16)
#define BHSM_HWKL_ID (0xFEDCCDEF)

typedef struct BHSM_P_KeyLadder* BHSM_KeyLadderHandle;

typedef struct BHSM_KeyLadderAllocateSettings
{
    BHSM_SecurityCpuContext owner; /* used to specify whether the keyladder is intended for HOST or SAGE usage. */
    unsigned index;                /* supports BHSM_ANY_ID to allow NEXUS manage KeyLadder instances.
                                      HWKL_ID is also support to indicate the intention of using the HW keyladder */

} BHSM_KeyLadderAllocateSettings;


/* Describes how the keyladder is to behave. */
typedef enum BHSM_KeyLadderMode
{
    BHSM_KeyLadderMode_eCa_64_4,      /* Generates 64 bit key that must be routed to a CP block from level 4. */
    BHSM_KeyLadderMode_eCp_64_4,      /* Generates 64 bit key that must be routed to a CA block from level 4. */
    BHSM_KeyLadderMode_eCa_64_5,
    BHSM_KeyLadderMode_eCp_64_5,
    BHSM_KeyLadderMode_eCa_128_4,
    BHSM_KeyLadderMode_eCp_128_4,
    BHSM_KeyLadderMode_eCa_128_5,
    BHSM_KeyLadderMode_eCp_128_5,
    BHSM_KeyLadderMode_eCa_64_7,
    BHSM_KeyLadderMode_eCa_128_7,
    BHSM_KeyLadderMode_eCa64_45,
    BHSM_KeyLadderMode_eCp64_45,
    BHSM_KeyLadderMode_eSageBlDecrypt,
    BHSM_KeyLadderMode_eSage128_5,
    BHSM_KeyLadderMode_eSage128_4,
    BHSM_KeyLadderMode_eGeneralPurpose1,
    BHSM_KeyLadderMode_eGeneralPurpose2,
    BHSM_KeyLadderMode_eEtsi_5,
    BHSM_KeyLadderMode_eScte52Ca_5,
    BHSM_KeyLadderMode_eHwlk,
    BHSM_KeyLadderMode_eRpmb,
    /* more to come */
    BHSM_KeyLadderMode_eMax
} BHSM_KeyLadderMode;


/* the different menchanisms for gererating a KeyLadder root key. */
typedef enum BHSM_KeyLadderRootType
{
    BHSM_KeyLadderRootType_eOtpDirect,         /* Use OTP as root key direct.  */
    BHSM_KeyLadderRootType_eOtpAskm,              /* derive root key from OTP via ASKM diffusion. */
    BHSM_KeyLadderRootType_eGlobalKey,         /* derive root key from OTP Global keys. */
    BHSM_KeyLadderRootType_eCustomerKey,       /* derive rook from customer keys. [note: maybe deprecated for zeus5.] */
    BHSM_KeyLadderRootType_eMax
} BHSM_KeyLadderRootType;


/* the scope of the ASKM CA Vendor ID */
typedef enum BHSM_KeyladderCaVendorIdScope
{
    BHSM_KeyladderCaVendorIdScope_eChipFamily,     /* CA Vendor ID generates root key differently per chip family */
    BHSM_KeyladderCaVendorIdScope_eFixed,          /* CA Vendor ID generates root key independent of chip family. */
    BHSM_KeyladderCaVendorIdScope_eMax
}BHSM_KeyladderCaVendorIdScope;

/* how the ASKM STB owner ID is to be determined */
typedef enum BHSM_KeyLadderStbOwnerIdSelect
{
    BHSM_KeyLadderStbOwnerIdSelect_eOtp,           /* Read StbOwnerId from OTP */
    BHSM_KeyLadderStbOwnerIdSelect_eOne,           /* Hardcode StbOwnerId to One */
    BHSM_KeyLadderStbOwnerIdSelect_eZero,          /* Hardcode StbOwnerId to Zero */
    BHSM_KeyLadderStbOwnerIdSelect_eMax
}BHSM_KeyLadderStbOwnerIdSelect;

/* how the Global Key Owner Id is to be determined. */
typedef enum BHSM_KeyLadderGlobalKeyOwnerIdSelect
{
    BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp0,            /* read owner from MSP OTP loaction 0 */
    BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMsp1,            /* read owner from MSP OTP loaction 1 */
    BHSM_KeyLadderGlobalKeyOwnerIdSelect_eOne,             /* hardcode to 1 */
    BHSM_KeyLadderGlobalKeyOwnerIdSelect_eMax
} BHSM_KeyLadderGlobalKeyOwnerIdSelect;


/* describes where the generated key is to be routed. */
typedef enum
{
    BHSM_KeyLadderDestination_eNone,         /* not routed, key remains on ladder level. */
    BHSM_KeyLadderDestination_eKeyslotKey,   /* a keyslot key location */
    BHSM_KeyLadderDestination_eKeyslotIv,    /* a keyslot IV location */
    BHSM_KeyLadderDestination_eKeyslotIv2,   /* a keyslot 2nd IV location */
    BHSM_KeyLadderDestination_eHdcp,         /* Zeus 4 only. */
    BHSM_KeyLadderDestination_eMax
}BHSM_KeyLadderDestination;


typedef enum
{
    BHSM_SwizzelType_eNone = 0,
    BHSM_SwizzelType_e1    = 1,
    BHSM_SwizzelType_e0    = 2,
    BHSM_SwizzelType_eMax
}BHSM_SwizzelType;


typedef struct
{
    unsigned index;
}BHSM_KeyLadderInfo;


typedef struct
{
    BHSM_CryptographicAlgorithm algorithm; /* the algorithm to use on the keyladder */
    BHSM_CryptographicOperation operation; /* encrypt or decrypt  */
    BHSM_KeyLadderMode mode;               /* Describes the allowed operations on the keyladder. */
    unsigned numLevels;                    /* NOT USED ***DEPRECATED**. Will be removed. */

    struct
    {
        BHSM_KeyLadderRootType type;

        /* valid if root.type is BHSM_KeyLadderRootType_eOtpDirect or BHSM_KeyLadderRootType_eOtpAskm */
        unsigned otpKeyIndex;         /* Specifies which OTP key is to be used. */

        /* valid if root.type is BHSM_KeyLadderRootType_eOtpAskm or BHSM_KeyLadderRootType_eGlobalKey */
        struct{
            /* SAGE_ONLY_BEGIN */
            unsigned                        sageModuleId;    /* requried only on SAGE */
            unsigned                        caVendorIdExtension;      /* id unique to CA vendor. */
            /* SAGE_ONLY_END */
            unsigned                        caVendorId;      /* id unique to CA vendor. */
            BHSM_KeyladderCaVendorIdScope  caVendorIdScope;
            BHSM_KeyLadderStbOwnerIdSelect stbOwnerSelect;  /* allow owner/boradcaster to add diffusion to key */
            bool swapKey;                                   /* swap key1's 8 top bytes with lower 8 (for 3DesAba ladder). */
        }askm;

        /* valid if root.type is BHSM_KeyLadderRootType_eGlobalKey */
        struct{
            BHSM_KeyLadderGlobalKeyOwnerIdSelect owner;
            unsigned                              index;     /* identify what global key to select. */
        }globalKey;

        /* valid if root.type is BHSM_KeyLadderRootType_eCustomerKey. */
        struct {
            BHSM_SwizzelType type;
            unsigned swizzle1IndexSel;
            bool enableSwizzle0a;
            unsigned cusKeySwizzle0aVariant;
            unsigned cusKeySwizzle0aVersion;
            struct {
                unsigned keyVar;
                unsigned keyIndex;
                bool decrypt;    /* 0 encrypt, 1 decrypt */
            }low, high;
        }customerKey;
    }root;

    struct{
        unsigned  numlevels;     /* length of the hwkl*/
        BHSM_CryptographicAlgorithm  algorithm;     /* algorithm of the hwkl*/
        uint8_t   moduleId;     /* moduleId to use */
    }hwkl;

} BHSM_KeyLadderSettings;


/**
    Structure to specify the level key. If the key is to be routed, the key's destination can be configured.
**/
typedef struct
{
    unsigned level;                                     /* an index value between 3 and max levels allowed by the keyladder mode. */
    uint8_t  ladderKey[BHSM_KEYLADDER_LADDER_KEY_SIZE]; /* know as "procIn" in secV1. */
    unsigned ladderKeySize;                             /* ladderKey size on bits. Must be less than BHSM_KEYLADDER_LADDER_KEY_SIZE*8 */

    /* describes where the generated key is to be routed. */
    struct
    {
        BHSM_KeyLadderDestination destination;

        /* valid if destination is keyslot key or IV */
        struct
        {
            BHSM_KeyslotHandle handle;     /* destination keyslot */
            BHSM_KeyslotBlockEntry entry;  /* destination block and polarity within keyslot.*/
        }keySlot;
    }route;

} BHSM_KeyLadderLevelKey;


typedef struct BHSM_KeyLadderChallenge
{
    unsigned nonceSize;
    uint8_t nonce[BHSM_KEYLADDER_CHALLENGE_NONCE_MAX_LEN];
}BHSM_KeyLadderChallenge;

typedef struct BHSM_KeyLadderChallengeResponse
{
    unsigned responseSize;
    uint8_t response[BHSM_KEYLADDER_CHALLENGE_RESPONSE_MAX_LEN];
}BHSM_KeyLadderChallengeResponse;


typedef struct
{
    bool clearOwnership;  /* release the keyadder ownership also. */
}BHSM_KeyLadderInvalidate;



/*
    Allocate/Free a KeyLadder Resource
*/
BHSM_KeyLadderHandle BHSM_KeyLadder_Allocate( BHSM_Handle hHsm, const BHSM_KeyLadderAllocateSettings *pSettings );
void BHSM_KeyLadder_Free( BHSM_KeyLadderHandle handle );

/*
    returns the current or default setting for the keyladder.
*/
void BHSM_KeyLadder_GetSettings( BHSM_KeyLadderHandle handle, BHSM_KeyLadderSettings *pSettings );

/*
    Configure the keyladder.
*/
BERR_Code BHSM_KeyLadder_SetSettings( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderSettings *pSettings );


/*
    Progress the key down the keyladder.
*/
BERR_Code BHSM_KeyLadder_GenerateLevelKey( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderLevelKey  *pKey );


/*
    Invalidate the keyladder.
*/
BERR_Code BHSM_KeyLadder_Invalidate( BHSM_KeyLadderHandle handle, const BHSM_KeyLadderInvalidate  *pInvalidate );


/*
Description:
    Return KeyLadder information.
*/
BERR_Code BHSM_KeyLadder_GetInfo( BHSM_KeyLadderHandle handle, BHSM_KeyLadderInfo *pInfo );


/* DEPRECATED . USE BHSM_KeyLadder_GetInfo */
BERR_Code BHSM_GetKeyLadderInfo( BHSM_KeyLadderHandle handle, BHSM_KeyLadderInfo *pInfo );



void BHSM_KeyLadder_GetDefaultChallenge( BHSM_KeyLadderChallenge  *pChallenge );

/* Challange the keyladder. Can be used to authenticate the STB.  */
BERR_Code BHSM_KeyLadder_Challenge( BHSM_KeyLadderHandle handle,
                                    const BHSM_KeyLadderChallenge  *pChallenge,
                                    BHSM_KeyLadderChallengeResponse *pResponse );




/*************************************************************************/
/******************************** PRIVATE ********************************/
/*************************************************************************/

typedef struct{
    unsigned dummy;
}BHSM_KeyLadderModuleSettings;

BERR_Code BHSM_KeyLadder_Init( BHSM_Handle hHsm, BHSM_KeyLadderModuleSettings *pSettings );

void BHSM_KeyLadder_Uninit( BHSM_Handle hHsm );

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
