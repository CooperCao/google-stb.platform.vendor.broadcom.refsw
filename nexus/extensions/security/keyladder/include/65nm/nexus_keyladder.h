/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 *****************************************************************************/
#ifndef NEXUS_KEYLADDER_H__
#define NEXUS_KEYLADDER_H__

#include "nexus_security_datatypes.h"
#include "nexus_security.h"

/*=**************************
The KeyLadder is an alternate method of deriving cryptographic and conditional access keys.
This extension provides support for the mechanism.
****************************/

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_SHA1_DIGEST_SIZE          20     /* size in byte of SHA1 string */
#define NEXUS_HMACSHA1_LEN              20
#define NEXUS_SECURITY_KEYLADDER_KEY_SIZE 32

/* The code below are for setVich and StartAVD */
#define NEXUS_SECURITY_SETVICH_CMD_IN_BUFFER_SIZE       19
#define NEXUS_SECURITY_SETVICH_CMD_OUT_BUFFER_SIZE      7

/**
Summary:
This enum defines the supported root key sources.
**/
typedef enum NEXUS_SecurityRootKeySrc
{
    NEXUS_SecurityRootKeySrc_eCuskey,
    NEXUS_SecurityRootKeySrc_eOtpKeyA,
    NEXUS_SecurityRootKeySrc_eOtpKeyB,
    NEXUS_SecurityRootKeySrc_eOtpKeyC,
    NEXUS_SecurityRootKeySrc_eReserved0,
    NEXUS_SecurityRootKeySrc_eReserved1,
    NEXUS_SecurityRootKeySrc_eReserved2,
    NEXUS_SecurityRootKeySrc_eAskmSel,

    /* Add new key entry type definition before this line */
    NEXUS_SecurityRootKeySrc_eMax
}   NEXUS_SecurityRootKeySrc;

/**
Summary:
This enum defines the supported keyladder types.
**/
typedef enum NEXUS_SecurityKeyladderType
{
    NEXUS_SecurityKeyladderType_e1Des,
    NEXUS_SecurityKeyladderType_e3Des,
    NEXUS_SecurityKeyladderType_eAes,

    /* Add new key entry type definition before this line */
    NEXUS_SecurityKeyladderType_eMax
}   NEXUS_SecurityKeyladderType;

/**
Summary:
This enum defines the supported keyladder IDs.
**/
typedef enum NEXUS_SecurityKeyladderID
{
    NEXUS_SecurityKeyladderID_eA,
    NEXUS_SecurityKeyladderID_eB,
    NEXUS_SecurityKeyladderID_eC,

    /* Add new key entry type definition before this line */
    NEXUS_SecurityKeyladderID_eMax
}   NEXUS_SecurityKeyladderID;


/**
Summary:
This enum defines the supported Customer SubMode for 7420B0 keyladder.
**/
typedef enum NEXUS_SecurityCustomerSubMode
{
    NEXUS_SecurityCustomerSubMode_eGeneric_CA_64_4  = 0x0,  /* Key Ladder for Generic CA with 64 bit keys using Key4 */
    NEXUS_SecurityCustomerSubMode_eGeneric_CP_64_4  = 0x1,  /* Key Ladder for Generic CP with 64 bit keys using Key4 */
    NEXUS_SecurityCustomerSubMode_eGeneric_CA_64_5  = 0x2,  /* Key Ladder for Generic CA with 64 bit keys using Key5 */
    NEXUS_SecurityCustomerSubMode_eGeneric_CP_64_5  = 0x3,  /* Key Ladder for Generic CP with 64 bit keys using Key5 */
    NEXUS_SecurityCustomerSubMode_eGeneric_CA_128_4 = 0x4,  /* Key Ladder for Generic CA with 128 bit keys using Key4 */
    NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4 = 0x5,  /* Key Ladder for Generic CP with 128 bit keys using Key4 */
    NEXUS_SecurityCustomerSubMode_eGeneric_CA_128_5 = 0x6,  /* Key Ladder for Generic CA with 128 bit keys using Key5 */
    NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_5 = 0x7,  /* Key Ladder for Generic CP with 128 bit keys using Key5 */
    NEXUS_SecurityCustomerSubMode_eReserved8        = 0x8,
    NEXUS_SecurityCustomerSubMode_eReserved9        = 0x9,
    NEXUS_SecurityCustomerSubMode_eReserved10       = 0xA,
    NEXUS_SecurityCustomerSubMode_eReserved11       = 0xB,
    NEXUS_SecurityCustomerSubMode_eReserved12       = 0xC,
    NEXUS_SecurityCustomerSubMode_eReserved13       = 0xD,
    NEXUS_SecurityCustomerSubMode_eGeneralPurpose1  = 0xE,  /* Key Ladder for HDMI, IV and Signed Commands */
    NEXUS_SecurityCustomerSubMode_eGeneralPurpose2  = 0xF,  /* Key Ladder for User Hmac */
    NEXUS_SecurityCustomerSubMode_eReserved16       = 0x10,
    NEXUS_SecurityCustomerSubMode_eReserved17       = 0x11,
    NEXUS_SecurityCustomerSubMode_eReserved18       = 0x12,
    NEXUS_SecurityCustomerSubMode_eReserved19       = 0x13,


    /* Add new Customer SubMode definition before this line */
    NEXUS_SecurityCustomerSubMode_eMax
}   NEXUS_SecurityCustomerSubMode;



/**
Summary:
This enum defines the supported swizzle types.
**/
typedef enum NEXUS_SecuritySwizzleType
{
    NEXUS_SecuritySwizzleType_eNone,
    NEXUS_SecuritySwizzleType_eSwizzle1,
    NEXUS_SecuritySwizzleType_eSwizzle0,

    /* Add new swizzle type definition before this line */
    NEXUS_SecuritySwizzleType_eMax
}   NEXUS_SecuritySwizzleType;

#if 0 
/* Moved to  nexus_security_datatypes.h */

/** Summary: This enum defines the supported swizzle types. **/
typedef enum NEXUS_SecurityKeyMode
{
    NEXUS_SecurityKeyMode_eRegular,
    NEXUS_SecurityKeyMode_eDes56,
    NEXUS_SecurityKeyMode_eReserved2,
    NEXUS_SecurityKeyMode_eReserved3,
    NEXUS_SecurityKeyMode_eDvbConformance,

    /* Add new swizzle type definition before this line */
    NEXUS_SecurityKeyMode_eMax
}   NEXUS_SecurityKeyMode;
#endif

/**
Summary:
**/
typedef struct NEXUS_UserCmdData
{
    unsigned char           hmacSha1Buf[NEXUS_HMACSHA1_LEN];
    unsigned int            hmacSha1Size;
} NEXUS_UserCmdData;

/**
Summary:
**/
typedef struct NEXUS_UserCmdImpl *NEXUS_UserCmdHandle;

/**
Summary:
**/
NEXUS_UserCmdHandle NEXUS_Security_CreateUserCmd(void);

/**
Summary:
**/
void NEXUS_Security_DestroyUserCmd(
    NEXUS_UserCmdHandle userCmd
    );

/**
Summary:
**/
void NEXUS_Security_GetUserCmdSettings(
    NEXUS_UserCmdHandle userCmdhandle,
    NEXUS_UserCmdData *pUserData /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_Security_SetUserCmdSettings(
    NEXUS_UserCmdHandle userCmdHandle,
    const NEXUS_UserCmdData *pData
    );

/**
Summary:
This struct defines the encrypted session structure.

Description:
This structure contains the information necessary to generate the
Session Key which is Key3 in BRCM terminology.
**/
typedef struct NEXUS_SecurityEncryptedKey3
{
    NEXUS_SecurityKeyladderID keyladderID;
    NEXUS_SecurityKeyladderType keyladderType;
    NEXUS_SecurityRootKeySrc rootKeySrc;        /* Root key source */
    NEXUS_SecuritySwizzleType swizzleType;
    NEXUS_SecurityKeyType keyEntryType;
    NEXUS_SecurityOperation operation;
    NEXUS_SecurityOperation operationKey2;
    /* the following 3 members are for 7420B) ASKM support */
    NEXUS_SecurityCustomerSubMode    custSubMode;
    NEXUS_SecurityVirtualKeyladderID virtualKeyLadderID;
    NEXUS_SecurityKeyMode            keyMode;

    unsigned char cusKeyL;                      /* cuskey low */
    unsigned char cusKeyH;                      /* cuskey high */
    unsigned char cusKeyVarL;                   /* cuskey var low */
    unsigned char cusKeyVarH;                   /* cuskey var high */
    unsigned char keyData[NEXUS_SECURITY_KEYLADDER_KEY_SIZE];   /* key data array */
    bool bRouteKey;
} NEXUS_SecurityEncryptedSessionKey;

/**
Summary:
This struct defines the encrypted control word (CW) structure.

Description:
This structure contains the information necessary to generate the
control word which is Key4 in BRCM terminology.
**/
typedef struct NEXUS_SecurityEncryptedKey4
{
    NEXUS_SecurityKeyladderID keyladderID;
    NEXUS_SecurityKeyladderType keyladderType;
    NEXUS_SecurityRootKeySrc rootKeySrc;        /* Root key source */
    NEXUS_SecuritySwizzleType swizzleType;
    /* the following 3 members are for 7420B) ASKM support */
    NEXUS_SecurityCustomerSubMode    custSubMode;
    NEXUS_SecurityVirtualKeyladderID virtualKeyLadderID;
    NEXUS_SecurityKeyMode            keyMode;

    unsigned int keySize;                       /* size of the key */
    NEXUS_SecurityKeyType keyEntryType;
    unsigned char keyData[NEXUS_SECURITY_KEYLADDER_KEY_SIZE];  /* key data array */
    NEXUS_SecurityOperation operation;          /* operation mode. This member is NOT required for CA */
    bool bRouteKey;
} NEXUS_SecurityEncryptedControlWord;

/**
Summary:
This function generates session key for a keyslot.

Description:
This function shall generate session key for a keyslot.  This function
must be called BEFORE the NEXUS_Security_GenerateControlWord
function can be called.

Performance and Timing:
This is a synchronous function that will return when it is done.

See Also:
NEXUS_Security_GenerateControlWord
**/
NEXUS_Error NEXUS_Security_GenerateSessionKey(
    NEXUS_KeySlotHandle keyHandle,
    const NEXUS_SecurityEncryptedSessionKey *pSessionKey
    );


/**
Summary:
This function generates the CW for a keyslot.

Description:
This function shall generate the CW for a keyslot.  This function
must be called after the NEXUS_Security_GenerateSessionKey has been
called to generate the session key.

Performance and Timing:
This is a synchronous function that will return when it is done.

See Also:
NEXUS_Security_GenerateSessionKey
**/
NEXUS_Error NEXUS_Security_GenerateControlWord(
    NEXUS_KeySlotHandle keyHandle,
    const NEXUS_SecurityEncryptedControlWord *pCW
    );


/**
Summary:
This function generates the key5 for a keyslot.

Description:
This function shall generate the key5 for a keyslot.  This function
must be called after the NEXUS_Security_GenerateSessionKey and
NEXUS_Security_GenerateControlWord have been called.

Note that this function takes the same structure as
NEXUS_Security_GenerateControlWord.

Performance and Timing:
This is a synchronous function that will return when it is done.

See Also:
NEXUS_Security_GenerateSessionKey, NEXUS_Security_GenerateControlWord
**/
NEXUS_Error NEXUS_Security_GenerateKey5(
    NEXUS_KeySlotHandle keyHandle,
    const NEXUS_SecurityEncryptedControlWord *pCW
    );

/**
Summary:
**/
NEXUS_Error NEXUS_Security_SetVich(void);

#ifdef __cplusplus
}
#endif

#endif
