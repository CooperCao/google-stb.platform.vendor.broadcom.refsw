/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
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

#define NEXUS_SECURITY_KEYLADDER_KEY_SIZE 32
#define NEXUS_SECURITY_KL_ACTCODE_SIZE    16
#define NEXUS_SECURITY_MAX_PROCIN_LEN     16
#define NEXUS_SECURITY_MAX_PROCOUT_LEN    32
#define NEXUS_SECURITY_MAX_NONCE_LEN      16
#define NEXUS_SECURITY_MAX_RESPONSE_LEN   16

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
	NEXUS_SecurityRootKeySrc_eOtpKeyD,
	NEXUS_SecurityRootKeySrc_eOtpKeyE,
	NEXUS_SecurityRootKeySrc_eOtpKeyF,
    NEXUS_SecurityRootKeySrc_eOtpKeyG,
    NEXUS_SecurityRootKeySrc_eOtpKeyH,
    NEXUS_SecurityRootKeySrc_eReserved0,
    NEXUS_SecurityRootKeySrc_eReserved1,
    NEXUS_SecurityRootKeySrc_eReserved2,
    NEXUS_SecurityRootKeySrc_eGlobalKey,
    NEXUS_SecurityRootKeySrc_eMax
}   NEXUS_SecurityRootKeySrc;

/**
Summary:
This enum defines the supported key generation subcommands.
**/
typedef enum NEXUS_SecurityKeyGenCmdID
{
    NEXUS_SecurityKeyGenCmdID_eKeyGen,
    NEXUS_SecurityKeyGenCmdID_eVKLAssocQuery,
    NEXUS_SecurityKeyGenCmdID_eMax
}   NEXUS_SecurityKeyGenCmdID;


/**
Summary:
This enum defines the supported KeyGen Key3 operations.
**/
typedef enum NEXUS_SecuritySessionKeyOp
{
    NEXUS_SecuritySessionKeyOp_eNoProcess,
    NEXUS_SecuritySessionKeyOp_eExport,
    NEXUS_SecuritySessionKeyOp_eMax
}   NEXUS_SecuritySessionKeyOp;



/**
Summary:
This enum defines the supported keyladder types.
**/
typedef enum NEXUS_SecurityKeyladderType
{
    NEXUS_SecurityKeyladderType_e1Des,
    NEXUS_SecurityKeyladderType_e3Des,
    NEXUS_SecurityKeyladderType_eAes128,
	NEXUS_SecurityKeyladderType_ePKSM,     /* BHSM_SUPPORT_HDDTA */
    NEXUS_SecurityKeyladderType_eMax
}   NEXUS_SecurityKeyladderType;

/**
Summary:
This enum defines the supported Key Ladder selection.
**/
typedef enum NEXUS_SecurityKeyLadderSelect
{
    NEXUS_SecurityKeyLadderSelect_eFWKL,
	NEXUS_SecurityKeyLadderSelect_eHWKL,
	NEXUS_SecurityKeyLadderSelect_ePKL,
    NEXUS_SecurityKeyLadderSelect_eMax
}   NEXUS_SecurityKeyLadderSelect;


/**
Summary:
This enum defines the supported keyladder IDs.
**/
typedef enum NEXUS_SecurityKeyladderID
{
    NEXUS_SecurityKeyladderID_eA,
    NEXUS_SecurityKeyladderID_eB,
    NEXUS_SecurityKeyladderID_eC,
    NEXUS_SecurityKeyladderID_eMax
}   NEXUS_SecurityKeyladderID;


/**
Summary:
This enum defines the supported Customer SubMode.
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
    NEXUS_SecurityCustomerSubMode_eGeneric_CA_64_7  = 0x8,
	NEXUS_SecurityCustomerSubMode_eGeneric_CA_128_7	= 0x9,
    NEXUS_SecurityCustomerSubMode_eReserved10       = 0xA,
    NEXUS_SecurityCustomerSubMode_eReserved11       = 0xB,
    NEXUS_SecurityCustomerSubMode_eSage_128_5       = 0xC,
    NEXUS_SecurityCustomerSubMode_eReserved13       = 0xD,
    NEXUS_SecurityCustomerSubMode_eGeneralPurpose1  = 0xE,  /* Key Ladder for HDMI, IV and Signed Commands */
    NEXUS_SecurityCustomerSubMode_eGeneralPurpose2  = 0xF,  /* Key Ladder for User Hmac */
    NEXUS_SecurityCustomerSubMode_eReserved16       = 0x10,
    NEXUS_SecurityCustomerSubMode_eReserved17       = 0x11,
    NEXUS_SecurityCustomerSubMode_eGeneric_CA_64_45 = 0x12, /* Key Ladder for Generic CA with 64 bit keys using Key4 and Key5 */
    NEXUS_SecurityCustomerSubMode_eGeneric_CP_64_45 = 0x13, /* Key Ladder for Generic CP with 64 bit keys using Key4 and Key5 */
	NEXUS_SecurityCustomerSubMode_eReserved20		= 0x14,
	NEXUS_SecurityCustomerSubMode_eReserved21		= 0x15,
	NEXUS_SecurityCustomerSubMode_eSecureRSA2       = 0x16,
	NEXUS_SecurityCustomerSubMode_eETSI_5           = 0x17,
	NEXUS_SecurityCustomerSubMode_eDTA_M_CA         = 0x18,
	NEXUS_SecurityCustomerSubMode_eDTA_M_CP         = 0x19,
	NEXUS_SecurityCustomerSubMode_eDTA_C_CA         = 0x1A,
	NEXUS_SecurityCustomerSubMode_eDTA_C_CP         = 0x1B,
	NEXUS_SecurityCustomerSubMode_eScte52CA5		= 0x1C,
	NEXUS_SecurityCustomerSubMode_eSageBlDecrypt	= 0x1D,  /* Key Ladder for SAGE FSBL decryption and SAGE EJTAG CR*/
	NEXUS_SecurityCustomerSubMode_eVistaKeyLadder	= 0x1E,
	NEXUS_SecurityCustomerSubMode_eVistaCwc 		= 0x1F,
    NEXUS_SecurityCustomerSubMode_eReserved32       = 0x20,
    NEXUS_SecurityCustomerSubMode_eReserved33       = 0x21,
    NEXUS_SecurityCustomerSubMode_eReserved34       = 0x22,
    NEXUS_SecurityCustomerSubMode_eDupleSource      = 0x23,
    NEXUS_SecurityCustomerSubMode_eDupleDestination = 0x24,
    NEXUS_SecurityCustomerSubMode_eOTPKeyFieldProgramDataDecrypt = 0x25,
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
    NEXUS_SecuritySwizzleType_eMax
}   NEXUS_SecuritySwizzleType;

/**
Summary:
This enum defines the supported swizzle types.
**/
typedef enum NEXUS_SecuritySwizzle0aType
{
    NEXUS_SecuritySwizzle0aType_eNoMsp,
    NEXUS_SecuritySwizzle0aType_eMsp0,
    NEXUS_SecuritySwizzle0aType_eMsp1,
    NEXUS_SecuritySwizzle0aType_eDisabled,
    NEXUS_SecuritySwizzle0aType_eMax
}   NEXUS_SecuritySwizzle0aType;



/**
Summary:
This enum defines the supported Hardware Key Ladder lengths.
**/
typedef enum NEXUS_SecurityHWKLLenSelect
{
	NEXUS_SecurityHWKLLenSelect_eLen0,
	NEXUS_SecurityHWKLLenSelect_eLen1,
	NEXUS_SecurityHWKLLenSelect_eLen2,
	NEXUS_SecurityHWKLLenSelect_eLen3,
    NEXUS_SecurityHWKLLenSelect_eMax
}   NEXUS_SecurityHWKLLenSelect;


/**
Summary:
This enum defines the supported SC01 global mapping in Mode Word.
**/
typedef enum NEXUS_SecuritySC01ModeWordMapping
{
	NEXUS_SecuritySC01ModeWordMapping_eClear,
	NEXUS_SecuritySC01ModeWordMapping_eOdd,
	NEXUS_SecuritySC01ModeWordMapping_eEven,
    NEXUS_SecuritySC01ModeWordMapping_eMax
}   NEXUS_SecuritySC01ModeWordMapping;


/**
Summary:
This enum defines the PKSM Init key size.
**/
typedef enum NEXUS_SecurityPKSMInitState
{
	NEXUS_SecurityPKSMInitState_eNone,
	NEXUS_SecurityPKSMInitState_e64Bits,
	NEXUS_SecurityPKSMInitState_e128Bits,
	NEXUS_SecurityPKSMInitState_e192Bits,
    NEXUS_SecurityPKSMInitState_eMax
}   NEXUS_SecurityPKSMInitState;


/**
Summary:
This enum defines the supported key layer.
**/
typedef enum NEXUS_SecurityKeyLayer
{
    NEXUS_SecurityKeyLayer_eKey3,
    NEXUS_SecurityKeyLayer_eKey4,
    NEXUS_SecurityKeyLayer_eKey5,
    NEXUS_SecurityKeyLayer_eKey6,
    NEXUS_SecurityKeyLayer_eKey7,
    NEXUS_SecurityKeyLayer_eMax
}   NEXUS_SecurityKeyLayer;

/**
Summary:
This enum defines the supported Global Key owner IDs.
**/

typedef enum NEXUS_SecurityGlobalKeyOwnerID
{
    NEXUS_SecurityGlobalKeyOwnerID_eMSP0,
    NEXUS_SecurityGlobalKeyOwnerID_eMSP1,
    NEXUS_SecurityGlobalKeyOwnerID_eReserved,
    NEXUS_SecurityGlobalKeyOwnerID_eUse1,

	NEXUS_SecurityGlobalKeyOwnerID_eMax
} NEXUS_SecurityGlobalKeyOwnerID;

/**
Summary:
This enum defines the supported key tweaking operations.
**/

typedef enum NEXUS_SecurityKeyTweak
{
    NEXUS_SecurityKeyTweak_eNoTweak,
    NEXUS_SecurityKeyTweak_ePKSM,
	NEXUS_SecurityKeyTweak_eTransform,
    NEXUS_SecurityKeyTweak_eDSK,
    NEXUS_SecurityKeyTweak_eDupleConnect,
    NEXUS_SecurityKeyTweak_eMax
} NEXUS_SecurityKeyTweak;


/**
Summary:
ASKM Key2 generation parameter.
**/
typedef enum NEXUS_SecurityMaskKey
{
    NEXUS_SecurityMaskKey_eReal       = 0,
    NEXUS_SecurityMaskKey_eReserved1  = 1,
    NEXUS_SecurityMaskKey_eFixed      = 2
} NEXUS_SecurityMaskKey;

/**
Summary:
This is the Virtual Key Ladder handle declaration.

Description:
This Virtual Key Ladder handle is to be used in all VKL related API functions.

See Also:
NEXUS_Security_AllocateVKL
**/

typedef struct NEXUS_VirtualKeyLadder *NEXUS_VirtualKeyLadderHandle;


/**
Summary:
This struct defines the encrypted session structure.

Description:
This structure contains the information necessary to generate the
Session Key which is Key3 in BRCM terminology.
**/
typedef struct NEXUS_SecurityEncryptedSessionKey
{
	NEXUS_SecurityClientType            client;
    NEXUS_SecurityKeyladderID           keyladderID;    /* not used for 40-nm;  for compatibility */
	NEXUS_SecurityKeyGenCmdID           keyGenCmdID;    /* Key Generation or VKL Association query */
    NEXUS_SecurityKeyladderType         keyladderType;
	NEXUS_SecurityKeyLadderSelect		keyLadderSelect; /*FW key ladder | HW key ladder | PKL  key ladder */
	bool								rootKeySwap;
    NEXUS_SecurityRootKeySrc            rootKeySrc;
    NEXUS_SecuritySwizzleType           swizzleType;
    NEXUS_SecurityKeyType               keyEntryType;
    NEXUS_SecurityKeyIVType             keyDestIVType;
    NEXUS_SecurityAlgorithmConfigDestination dest;/* This member is required only for CACP keyslot configuration */
    NEXUS_SecurityOperation             operation;      /* key ladder operation, encrypt or decrypt */
    NEXUS_SecurityOperation             operationKey2;  /* cusKeyL and cusKeyH encrypt or decrypt */
    bool                                bASKMMode;
    NEXUS_SecurityGlobalKeyOwnerID      globalKeyOwnerId;
	unsigned char                       askmGlobalKeyIndex;
    NEXUS_SecurityKeyTweak              keyTweakOp;
	int                                 globalKeyVersion;  /* Zeus 4.1+ : 0  Current;*/
	bool								bkeyGenBlocked; /* set to true to instruct FW not to generate key */
    NEXUS_SecurityCustomerSubMode       custSubMode;
    NEXUS_SecurityVirtualKeyladderID    virtualKeyLadderID;
    NEXUS_SecurityKeyMode               keyMode;
    NEXUS_SecuritySessionKeyOp          sessionKeyOp;
    unsigned char                       cusKeyL;        /* also used as SwizzleIndex for Swizzle1 */
    unsigned char                       cusKeyH;
    unsigned char                       cusKeyVarL;
    unsigned char                       cusKeyVarH;
    struct
    {
        bool                            askmConfigurationEnable; /* When true, indicates that the following sage ASKM configuration paramters are valid. */
        NEXUS_SecurityOtpId             otpId;          /* valid when askmConfigurationEnable is true. */
        unsigned int                    caVendorId;     /* valid when askmConfigurationEnable is true. */
        unsigned int                    moduleID;       /* valid when askmConfigurationEnable is true. */
        NEXUS_SecurityMaskKey           maskKeySelect;  /* valid when askmConfigurationEnable is true. */
    } sage;
    bool                                cusKeySwizzle0aEnable;
    NEXUS_SecuritySwizzle0aType         cusKeySwizzle0aType;
    unsigned int                        cusKeySwizzle0aVer;
    unsigned char                       keyData[NEXUS_SECURITY_KEYLADDER_KEY_SIZE];
    unsigned int                        keySize;

    bool                                bSwapAESKey;
    bool                                bRouteKey;
	NEXUS_SecurityHWKLLenSelect			hwklLen;
	NEXUS_SecurityAlgorithm				hwklDestAlg;
	bool                                hwklVistaKeyGen;
    unsigned char                       actCode[NEXUS_SECURITY_KL_ACTCODE_SIZE];
    struct
    {
        bool                            reset;
        NEXUS_SecurityPKSMInitState     initState;
        unsigned int                    cycle;		 /* From 0 to 1023 */
    } pksm;
    bool                                applyKeyContribution; /* Zeus 4.2+ : true to apply a key contribution from SAGE. */
    NEXUS_SecurityVirtualKeyladderID    sourceDupleKeyLadderId; /* Zeus 4.2+; the source keyladder in a duple customer submode configuration */

} NEXUS_SecurityEncryptedSessionKey;

/**
Summary:
This struct defines the encrypted control word (CW) structure.

Description:
This structure contains the information necessary to generate the
control word which is Key4 in BRCM terminology.
**/
typedef struct NEXUS_SecurityEncryptedControlWord
{
    NEXUS_SecurityKeyTweak              keyTweakOp;
    bool                                protectionKeyIvSource;  /* Relevant to NEXUS_SecurityKeyTweak_eDSK.  0:use DS, 1:use internal */
	NEXUS_SecurityClientType            client;
    NEXUS_SecurityKeyladderID           keyladderID;    /* not used for 40-nm;  for compatibility */
	NEXUS_SecurityKeyGenCmdID           keyGenCmdID;    /* Key Generation or VKL Association query */
    NEXUS_SecurityKeyladderType         keyladderType;
	NEXUS_SecurityKeyLadderSelect		keyLadderSelect; /*FW key ladder | HW key ladder | Poway key ladder */
	bool								rootKeySwap;
    NEXUS_SecurityRootKeySrc            rootKeySrc;
    NEXUS_SecurityKeyType               keyEntryType;
    NEXUS_SecurityKeyIVType             keyDestIVType;
    NEXUS_SecurityAlgorithmConfigDestination dest;/* This member is required only for CACP keyslot configuration */
    NEXUS_SecuritySwizzleType           swizzleType;
    bool                                bASKMMode;
	bool								bkeyGenBlocked; /* set to true to instruct FW not to generate key */
    NEXUS_SecurityCustomerSubMode       custSubMode;
    NEXUS_SecurityVirtualKeyladderID    virtualKeyLadderID;
    NEXUS_SecurityKeyMode               keyMode;
    NEXUS_SecurityAlgorithmScPolarity   sc01Polarity[NEXUS_SecurityPacketType_eMax]; /* Packets with SC bits 0x01 will be treated as this polarity */
	NEXUS_SecurityAlgorithmScPolarity   sc01GlobalMapping;
    unsigned int                        keySize;
    unsigned char                       keyData[NEXUS_SECURITY_KEYLADDER_KEY_SIZE];
    NEXUS_SecurityOperation             operation;          /* operation mode. This member is NOT required for CA */
    bool                                bSwapAESKey;
    bool                                bRouteKey;
	NEXUS_SecurityHWKLLenSelect			hwklLen;
	NEXUS_SecurityAlgorithm				hwklDestAlg;
    unsigned char                       actCode[NEXUS_SECURITY_KL_ACTCODE_SIZE];
	NEXUS_SecurityKeyLayer				keylayer;
    bool                                applyKeyContribution; /* Zeus 4.2+ : true to apply a key contribution from SAGE. */
    NEXUS_SecurityVirtualKeyladderID    sourceDupleKeyLadderId; /* Zeus 4.2+; the source keyladder in a duple customer submode configuration */

} NEXUS_SecurityEncryptedControlWord;
typedef NEXUS_SecurityEncryptedControlWord NEXUS_SecurityEncryptedKey;

/**
Summary:
This struct defines the structure parameter for ProcOut generation command.
**/


typedef struct NEXUS_SecurityKLProcOutParm
{
    NEXUS_SecurityVirtualKeyladderID    virtualKeyLadderID;
    unsigned int                        procInLen;
	unsigned char                       procIn[NEXUS_SECURITY_MAX_PROCIN_LEN];

} NEXUS_SecurityKLProcOutParm;


/**
Summary:
This struct defines the output for ProcOut generation command.
**/


typedef struct NEXUS_SecurityKLProcOutOutput
{
    unsigned int                        procOutLen;
	unsigned char                       procOut[NEXUS_SECURITY_MAX_PROCOUT_LEN];

} NEXUS_SecurityKLProcOutOutput;


/**
Summary:
This function retrieves default settings for encrypted session key generation.

Description:
This function shall retrieve default settings for encrypted session key generation.

See Also:
NEXUS_Security_GenerateSessionKey

**/
void NEXUS_Security_GetDefaultSessionKeySettings(
    NEXUS_SecurityEncryptedSessionKey  *pSettings    /* [out] */
    );


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
    NEXUS_KeySlotHandle               keyHandle,
    const NEXUS_SecurityEncryptedSessionKey *pSessionKey
    );



/**
Summary:
This function retrieves default settings for control word generation.

Description:
This function shall retrieve default settings for control word generation.

See Also:
NEXUS_Security_GenerateControlWord


**/
void NEXUS_Security_GetDefaultControlWordSettings(
    NEXUS_SecurityEncryptedControlWord *pSettings    /* [out] */
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
    NEXUS_KeySlotHandle                      keyHandle,
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
    NEXUS_KeySlotHandle                      keyHandle,
    const NEXUS_SecurityEncryptedControlWord *pCW
    );

/**
Summary:
This structure provides information for a VKL request to be submitted to BSP.
If virtualKeyLadderID is set to NEXUS_SecurityVirtualKeyLadderID_eMax,
the first available VKL ID will be returned to the application.
**/
typedef struct NEXUS_SecurityVKLSettings
{
    NEXUS_SecurityCustomerSubMode       custSubMode;
	NEXUS_SecurityClientType            client;
	bool                                newVKLCustSubModeAssoc;
} NEXUS_SecurityVKLSettings;



/**
Summary:
This function initializes the VKL Setting struture with default values.
**/
void NEXUS_Security_GetDefaultVKLSettings(
    NEXUS_SecurityVKLSettings *pVKLSettings           /*  [out] */
    );



/**
Summary:
This function allocates a Virtual Key Ladder (VKL) to be used for key generation.
**/
NEXUS_VirtualKeyLadderHandle NEXUS_Security_AllocateVKL(  /* attr{destructor=NEXUS_Security_FreeVKL} */
    const NEXUS_SecurityVKLSettings *pVKLReqSettings
    );

/**
Summary:
This function returns the previously allocated VKL back to the pool of available VKLs.
**/

void NEXUS_Security_FreeVKL(
	NEXUS_VirtualKeyLadderHandle vklHandle
	);


/**
Summary:
This structure provides information for a VKL request to be submitted to BSP.
If virtualKeyLadderID is set to NEXUS_SecurityVirtualKeyLadderID_eMax,
the first available VKL ID will be returned to the application.
**/

typedef struct NEXUS_VirtualKeyLadderInfo
{
	NEXUS_SecurityVirtualKeyladderID      vkl;
    NEXUS_SecurityCustomerSubMode         custSubMode;
	NEXUS_SecurityClientType              client;
} NEXUS_VirtualKeyLadderInfo;


/**
Summary:
This function returns the VKL information belonged to the specified vklHandle.
**/
void NEXUS_Security_GetVKLInfo(
	NEXUS_VirtualKeyLadderHandle        vklHandle,
	NEXUS_VirtualKeyLadderInfo                *pVKLInfo    /* [out] */
	);

/**
Summary:
This function generates the next layer key for a keyslot.

Description:
This function shall generate the next key for a keyslot.  This function
must be called after the NEXUS_Security_GenerateSessionKey.

Note that this function takes the  structure of NEXUS_SecurityEncryptedKey,
same as NEXUS_Security_GenerateControlWord.

See Also:
NEXUS_Security_GenerateSessionKey, NEXUS_Security_GenerateControlWord
**/
NEXUS_Error NEXUS_Security_GenerateNextLayerKey(
    NEXUS_KeySlotHandle keyHandle,
    const NEXUS_SecurityEncryptedControlWord * pCW
    );


NEXUS_Error NEXUS_Security_GenerateProcOut(
    NEXUS_KeySlotHandle                      keyHandle,
    const NEXUS_SecurityKLProcOutParm        *inProcOutParm,
    NEXUS_SecurityKLProcOutOutput            *outProcOut      /* [out] */
    );

typedef enum
{
    NEXUS_SecurityKeyLadderAlgorithm_3Des,
    NEXUS_SecurityKeyLadderAlgorithm_Aes,
    NEXUS_SecurityKeyLadderAlgorithm_eMax
}NEXUS_SecurityKeyLadderAlgorithm;


typedef struct NEXUS_SecurityChallengeResponseParm
{
	NEXUS_SecurityRootKeySrc			otpKeyId;
	NEXUS_SecurityVirtualKeyladderID      vkl;
	NEXUS_SecurityKeyLadderAlgorithm    kladMode;
    unsigned challengeSize;
    unsigned char data[NEXUS_SECURITY_MAX_NONCE_LEN];
}NEXUS_SecurityChallengeResponseParm;

typedef struct NEXUS_SecurityChallengeResponseOutput
{
    unsigned responseSize;
    unsigned char data[NEXUS_SECURITY_MAX_RESPONSE_LEN];
}NEXUS_SecurityChallengeResponseOutput;

NEXUS_Error NEXUS_SecurityChallengeResponse(
    const NEXUS_SecurityChallengeResponseParm *crParm,
    NEXUS_SecurityChallengeResponseOutput *crOutput  /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif
