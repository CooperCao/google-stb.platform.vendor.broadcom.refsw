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


#ifndef COMMON_CRYPTO_SECV2_H__
#define COMMON_CRYPTO_SECV2_H__

#include "nexus_dma.h"
#include "nexus_security_datatypes.h"
#include "nexus_keyladder.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Common crypto Handle
***************************************************************************/
typedef struct CommonCrypto *CommonCryptoHandle;

/* Maximum size of the key in bytes.*/
#define COMMON_CRYPTO_KEY_SIZE 32
/* Procin size in bytes.*/
#define COMMON_CRYPTO_PROC_SIZE 16
#define XPT_TS_PACKET_SIZE (188)

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

/**
Summary:
This enum defines the origin of the key
**/


typedef enum CommonCryptoKeySrc
{

    CommonCrypto_eClearKey,  /* Clear text key */
    CommonCrypto_eCustKey, /*not supported in Z5/Secv2, but
                                                needs to be defined for usage to be flagged
                                                and to allow Z4/Secv2 to use*/
    CommonCrypto_eOtpDirect,
    CommonCrypto_eOtpAskm,
    CommonCrypto_eGlobalKey,
    CommonCrypto_eMax

}CommonCryptoKeySrc;




typedef struct CommonCryptoKeyLadderOperationStruct
{
    NEXUS_CryptographicOperation SessionKeyOperation;
    NEXUS_CryptographicOperation SessionKeyOperationKey2;
    NEXUS_CryptographicOperation ControlWordKeyOperation;
}CommonCryptoKeyLadderOperationStruct;

typedef enum CommonCryptoKeyType
{
    CommonCryptoKeyType_eKey,
    CommonCryptoKeyType_eIv,
    CommonCryptoKeyType_eMax
}CommonCryptoKeyType;



/***************************************************************************
Summary:
Clear Key/IV structure

See Also: CommonCryptoClearKeySettings
CommonCrypto_GetDefaulClearKeySettings()
CommonCrypto_LoadClearKeyIv()
***************************************************************************/
typedef struct CommonCryptoKeyIvSettings
{
    uint8_t key[COMMON_CRYPTO_KEY_SIZE]; /* Key */
    uint32_t keySize; /* Key size in bytes, set to 0 if no key is present */
    uint8_t iv[COMMON_CRYPTO_KEY_SIZE]; /* IV */
    uint32_t ivSize;  /* IV size in bytes, set to 0 if no IV is present. */
}CommonCryptoKeyIvSettings;

/***************************************************************************
Summary:
Clear Key Settings structure

See Also: CommonCryptoKeyIvSettings
CommonCrypto_GetDefaulClearKeySettings()
CommonCrypto_LoadClearKeyIv()
***************************************************************************/
typedef struct CommonCryptoClearKeySettings
{
    NEXUS_KeySlotHandle keySlot;          /* Key slot where the key will be loaded. */

    NEXUS_KeySlotBlockEntry keySlotEntryType;           /* Key destination entry type */

    CommonCryptoKeyIvSettings settings; /* Key/IV information to load in the key slot. */
}CommonCryptoClearKeySettings;

/***************************************************************************
Summary:
Key Ladder structure

See Also: CommonCryptoCipheredKeySettings
CommonCrypto_GetDefaulCipheredKeySettings()
CommonCrypto_LoadCipheredKey()
***************************************************************************/
typedef struct CommonCryptoKeyLadderSettings
{
    uint8_t custKeySelect; /* Ignored when key src = CommonCrypto_eOtpKey */
    uint8_t keyVarHigh;    /* Ignored when key src = CommonCrypto_eOtpKey */
    uint8_t keyVarLow;     /* Ignored when key src = CommonCrypto_eOtpKey */
    uint8_t procInForKey3[COMMON_CRYPTO_PROC_SIZE];
    uint32_t key3Size;
    uint8_t procInForKey4[COMMON_CRYPTO_PROC_SIZE];
    uint32_t key4Size;

    bool overwriteKeyLadderOperation;
    CommonCryptoKeyLadderOperationStruct KeyLadderOpStruct;

    bool overwriteVKLSettings;  /* Not used */

    NEXUS_CryptographicAlgorithm keyladderAlgType; /* this is equivalent to keyladdertype in secv1*/
    NEXUS_KeyLadderInfo keyLadderInfo;
    NEXUS_KeyLadderMode keyladderMode;
    NEXUS_KeyLadderGlobalKeyOwnerIdSelect globalKeyOwnerId;
    unsigned globalKeyIndex;

    bool askmSupport;
    bool aesKeySwap;
}CommonCryptoKeyLadderSettings;

/***************************************************************************
Summary:
Ciphered Key Settings structure

See Also: CommonCryptoKeyLadderSettings
CommonCrypto_GetDefaulCipheredKeySettings()
CommonCrypto_LoadCipheredKey()
***************************************************************************/
typedef struct CommonCryptoCipheredKeySettings
{
    NEXUS_KeySlotHandle keySlot;          /* Key slot where the key will be loaded. */
    CommonCryptoKeySrc keySrc;      /* Origin of the key, either CommonCrypto_eCustKey or CommonCrypto_eOtpKey. */

    NEXUS_KeySlotBlockEntry keySlotEntryType;
    NEXUS_KeyLadderDestination keyDestination;           /* Key destination entry type */

    CommonCryptoKeyLadderSettings settings; /* Key Ladder information used to load the key. */
}CommonCryptoCipheredKeySettings;


/***************************************************************************
Summary:
Algorithm structure

See Also: CommonCryptoKeyConfigSettings
CommonCrypto_GetDefaulKeyConfigSettings()
CommonCrypto_LoadKeyConfig()
***************************************************************************/
typedef struct CommonCryptoAlgorithmSettings
{

    NEXUS_CryptographicOperation opType;         /* Operation to perfrom */
    NEXUS_CryptographicAlgorithm algType;        /* Crypto algorithm */
    NEXUS_CryptographicAlgorithmMode algVariant; /* Cipher chain mode for selected cipher */
    NEXUS_KeySlotPolarity keySlotType;           /* Key destination entry type */
    NEXUS_KeySlotTerminationMode termMode;       /* Termination Type for residual block to be ciphered */
    NEXUS_KeySlotBlockEntry keySlotEntryType;  /*different for decrypt and encrypt operations*/

    bool enableExtKey;                           /* Flag used to enable external key loading during dma transfer on the key slot.
                                                    true = key will prepend data in the dma descriptors. */
    bool enableExtIv;                            /* Flag used to enable external IV loading during dma transfer on the key slot.
                                                    true = iv will prepend data in the dma descriptors. */

    unsigned aesCounterSize;                                /* For algorithm modes predicated on a counter, this parameter spcifies
                                                        the size of the counter in bits. Supported values are 32, 64, 96 and 128 bits.*/
    NEXUS_CounterMode aesCounterMode;
    NEXUS_KeySlotTerminationSolitaryMode solitaryMode;

    uint32_t caVendorID;                         /* Conditional Access Vendor ID - assigned by Broadcom for cipher
                                                    key selection/computation */
    NEXUS_KeyLadderStbOwnerIdSelect stbOwnerID;  /* Source for  STB owner ID - used in ASKM mode for key2 generation */

}CommonCryptoAlgorithmSettings;


/***************************************************************************
Summary:
Key configuration  structure

See Also: CommonCryptoAlgorithmSettings
CommonCrypto_GetDefaulCipheredKeySettings()
CommonCrypto_LoadCipheredKey()
***************************************************************************/
typedef struct CommonCryptoKeyConfigSettings
{
    NEXUS_KeySlotHandle keySlot; /* Key slot where the key will be loaded. */
    CommonCryptoAlgorithmSettings settings; /* Algorithm informaiton used to setup the key slot. */
}CommonCryptoKeyConfigSettings;

/***************************************************************************
Summary:
Key Settings structure

See Also: CommonCryptoAlgorithmSettings
CommonCryptoKeyIvSettings
CommonCryptoKeyLadderSettings
CommonCrypto_GetDefaulKeySettings()
CommonCrypto_SetupKey()
***************************************************************************/
typedef struct CommonCryptoKeySettings
{
    CommonCryptoKeySrc keySrc; /* Key slot where the key will be loaded. */
    NEXUS_KeySlotHandle keySlot; /* Key slot where the key will be loaded. */

    union {
        CommonCryptoKeyIvSettings keyIvInfo;   /* Key/IV information to load in the key slot. */
        CommonCryptoKeyLadderSettings keyLadderInfo; /* Key Ladder information used to load the key. */
    }src;

    CommonCryptoAlgorithmSettings alg;  /* algorithm information used to setup the key slot */
} CommonCryptoKeySettings;


/***************************************************************************
Summary:
Common Crypto settings.

Description:
Settings used during common crypto open
***************************************************************************/
typedef struct CommonCryptoSettings
{
    uint32_t videoSecureHeapIndex; /* deprecated */

    NEXUS_DmaSettings   dmaSettings;
} CommonCryptoSettings;

/***************************************************************************
Summary:
Common Crypto External Key and IV data

Description:
Settings for external key and IV
***************************************************************************/
typedef struct {
    unsigned      slotIndex;                               /* Index into external key-slot table.  */
    struct {
        bool          valid;                               /* Indicates that the associated offset is valid.  */
        unsigned      offset;                              /* offset into external key slot. */
        unsigned      size;
        uint8_t      *pData;
    } key, iv;
} CommonCryptoExternalKeyData;


/***************************************************************************
Summary:
Get Common Crypto module default settings.

Description:
This function is used to initialize a CommonCryptoSettings structure with default settings.
This is required in order to make application code resilient to the addition of new strucutre members in the future.
***************************************************************************/
void CommonCrypto_GetDefaultSettings(
    CommonCryptoSettings *pSettings    /* [out] default settings */
    );

/***************************************************************************
Summary:
Open the common crypto module.

Description:
This function opens a common crypto module.
***************************************************************************/
CommonCryptoHandle CommonCrypto_Open(
    const CommonCryptoSettings *pSettings
    );


/***************************************************************************
Summary:
Close a common crypto module

Description:
This functions closes an instance of the common crypto module. And release its handle.
***************************************************************************/
void CommonCrypto_Close(
    CommonCryptoHandle handle
    );

/***************************************************************************
Summary:
Get default key ladder operation settings.

Description:
This function is used to initialize a CommonCryptoKeyLadderOperationStruct structure
with default settings used during the key3 and key4 generation.

***************************************************************************/
void CommonCrypto_GetDefaultKeyLadderSettings(
        CommonCryptoKeyLadderSettings *pSettings    /* [out] default settings */
    );

/***************************************************************************
Summary:
Get current module settings.

Description:
This functions is used to retrive the current module settings
***************************************************************************/
void CommonCrypto_GetSettings(
    CommonCryptoHandle handle,
    CommonCryptoSettings *pSettings /* [out] */
    );


/***************************************************************************
Summary:
Set the current settings of the module instance.

Description:
This function is used to nodify the module current settings.
***************************************************************************/
NEXUS_Error CommonCrypto_SetSettings(
    CommonCryptoHandle handle,
    const CommonCryptoSettings *pSettings
    );


/***************************************************************************
Summary:
Get default key configuration settings.

Description:
This function is used to initialize a CommonCryptoKeyConfigSettings structure with default settings.
This is required in order to make application code resilient to the addition of new strucutre members in the future.
***************************************************************************/
void CommonCrypto_GetDefaultKeyConfigSettings(
    CommonCryptoKeyConfigSettings *pSettings    /* [out] default settings */
    );

/***************************************************************************
Summary:
Loads the key configuration into the secure processor.

Description:
This function loads the key configuration into a key slot of the secure processor.
It must be called before a key can be loaded in the key slot using either CommonCrypto_LoadClearKeyIv()
or CommonCrypto_LoadCipheredKey().
***************************************************************************/
NEXUS_Error CommonCrypto_LoadKeyConfig(
    CommonCryptoHandle handle,
    const CommonCryptoKeyConfigSettings *pSettings
    );



/***************************************************************************
Summary:
Get default clear key settings .

Description:
This function is used to initialize a CommonCryptoClearKeySettings structure with default settings.
This is required in order to make application code resilient to the addition of new structure members in the future.
***************************************************************************/
void CommonCrypto_GetDefaultClearKeySettings(
    CommonCryptoClearKeySettings *pSettings    /* [out] default settings */
    );
/***************************************************************************
Summary:
Loads a cleartext key into a key slot.

Description:
This function loads a clear text key  into a key slot of the secure processor.
Note that before a key can be loaded in a key slot,the key slot must first be configured by
calling CommonCrypto_LoadKeyConfig().
***************************************************************************/
NEXUS_Error CommonCrypto_LoadClearKeyIv(
    CommonCryptoHandle handle,
    const CommonCryptoClearKeySettings *pSettings
    );

/***************************************************************************
Summary:
Get default ciphered key settings .

Description:
This function is used to initialize a CommonCryptoCipheredKeySettings structure with default settings.
This is required in order to make application code resilient to the addition of new strucutre members in the future.
***************************************************************************/
void CommonCrypto_GetDefaultCipheredKeySettings(
    CommonCryptoCipheredKeySettings *pSettings    /* [out] default settings */
    );
/***************************************************************************
Summary:
Uses the keyladder to load a key into a key slot.

Description:
This function loads a key into a key slot of the secure processor using the keyladder.
Note that before a key can be loaded in a key slot,the key slot must first be configured by
calling CommonCrypto_LoadKeyConfig().
***************************************************************************/
NEXUS_Error CommonCrypto_LoadCipheredKey(
    CommonCryptoHandle handle,
    const CommonCryptoCipheredKeySettings *pSettings
    );


/***************************************************************************
Summary:
Get default key settings .

Description:
This function is used to initialize a CommonCryptoKeySettings structure with default settings.
This is required in order to make application code resilient to the addition of new strucutre members in the future.
***************************************************************************/
void CommonCrypto_GetDefaultKeySettings(
    CommonCryptoKeySettings *pSettings,    /* [out] default settings */
    CommonCryptoKeySrc keySrc
    );

/***************************************************************************
Summary:
Loads a key into the secure processor.

Description:
This function configures a key slot and load a key in it. The key can either be cleartext
or it can comes from the keyladder.
***************************************************************************/
NEXUS_Error CommonCrypto_SetupKey(
    CommonCryptoHandle handle,
    const CommonCryptoKeySettings *pSettings
    );



/***************************************************************************
Summary:
Common crypto DMA Job Settings

Description:
See Also:
CommonCrypto_GetDefaultJobSettings
CommonCrypto_DmaXfer
***************************************************************************/
typedef struct CommonCryptoJobSettings
{
    NEXUS_DmaDataFormat        dataFormat;   /* Data format used when encrypting/decryption data. */
    NEXUS_KeySlotHandle        keySlot;      /* Key slot handle to use during the DMA transfer.  NULL(default) if not encrypting or decrypting data*/
} CommonCryptoJobSettings;

/***************************************************************************
Summary:
Get default dma job settings

Description:
This function is used to initialize a CommonCryptoJobSettings structure with default settings
This is required in order to make application code resilient to the addition of new strucutre members in the future..
***************************************************************************/
void CommonCrypto_GetDefaultJobSettings(
    CommonCryptoJobSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
This functions do a DMA transfer.

Description:
This function creates and start a blocking dma transfer. Them it will use a pooling loop to wait for the transfer to complete.
Special care must be taken by the caller to avoid calling this function in time critical code.
***************************************************************************/
NEXUS_Error CommonCrypto_DmaXfer(
    CommonCryptoHandle handle,
    const CommonCryptoJobSettings *pJobSettings,
    const NEXUS_DmaJobBlockSettings *pBlkSettings,
    unsigned nBlocks
    );

void CommonCrypto_CompileBtp(
    uint8_t * pBtp,
    CommonCryptoExternalKeyData * pBtpData );




#ifdef __cplusplus
}

#endif

#endif /* #ifndef COMMON_CRYPTO_H__ */
