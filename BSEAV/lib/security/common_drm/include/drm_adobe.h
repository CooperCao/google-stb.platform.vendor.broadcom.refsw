/***************************************************************************
 *     (c)2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 **************************************************************************/

#ifndef DRM_ADOBE_H__
#define DRM_ADOBE_H__

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"


#include "drm_types.h"
#include "drm_common.h"
#include "drm_common_priv.h"
#include "nexus_keyladder.h"

#if SECURERSA_SUPPORT
#include "common_crypto_securersa.h"
#endif

#include "nexus_random_number.h"



#if ADOBE_SAGE
#include "drm_adobe_tl_priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define USE_RSA_STRUCT
#define DUAL_DRMKEY

#define DRM_ADOBE_RSA1024_KEYSZ 128
#define DRM_ADOBE_RSA2048_KEYSZ 256
#define DRM_ADOBE_RSA_PUBLIC_EXP_SZ 3
#define BRCM_HWID_LEN 32
#define DRM_ADOBE_MAX_RAND_LENGTH NEXUS_MAX_RANDOM_NUMBER_LENGTH
#define AES_BLOCK_SZ 16


typedef struct DrmAdobeParamSettings_t
{
    BDBG_OBJECT(drm_adobe)
    DrmCommonInit_t drmCommonInit;
    char * drm_bin_file_path;
#if ADOBE_SAGE
    DrmAdobe_DeviceCapabilities capabilities;
#endif
}DrmAdobeParamSettings_t;

typedef enum DrmAdobe_binOffset_e{
    DrmAdobe_binOffset_SecureAdobeAESkey     = 0,
    DrmAdobe_binOffset_AdobeDRMDecKey        = 4,
#ifdef DUAL_DRMKEY

    DrmAdobe_binOffset_AdobeDRMSigningKey    = 8,
    DrmAdobe_binOffset_DrmDecCert            = 12,
    DrmAdobe_binOffset_DrmSignCert           = 16,
#else

    DrmAdobe_binOffset_DrmDecCert            = 8,

#endif

    DrmAdobe_SDK_binOffset_Sessionkey3                  = 0,
    DrmAdobe_SDK_binOffset_KeyLadder_IV                 = 4,
    DrmAdobe_SDK_binOffset_RSA_Key0                     = 8,
    DrmAdobe_SDK_binOffset_RSA_Key1                     = 12,
    DrmAdobe_SDK_binOffset_RSA_Key2                     = 16,
    DrmAdobe_SDK_binOffset_Key0_Cert                    = 20,
    DrmAdobe_SDK_binOffset_Key1_Cert                    = 24,
    DrmAdobe_SDK_binOffset_Key2_Cert                    = 28,
    DrmAdobe_SDK_binOffset_Indiv_Transport_Cert         = 32,
    DrmAdobe_SDK_binOffset_Indiv_Transport_Cert_SHA256  = 36,
    DrmAdobe_SDK_binOffset_Root_Cert_Digest             = 40,
    DrmAdobe_binOffset_eMax

}DrmAdobe_binOffset_e;

#define PUBLIC_VERIFY_KEY_INDEX          0
#define HASHLOCKED_PRIVRSA_KEY_SLOT      0
#define ADOBE_DRM_DECRYPTION_RSA_SLOT    4
#define ADOBE_DRM_SIGNKEY_RSA_SLOT       0
#define ADOBE_DRM_MACHINE_RSA_SLOT       4
#define ADOBE_SHAREDDOMAIN_RSA_SLOT      0
#define ADOBE_DOMAIN_RSA_SLOT            0

#define ADOBE_SECURE_AESKEY_KPK_SLOT     1
#define ADOBE_SESSSION_KEY3_KPK_SLOT     2
#define ADOBE_GENERIC_KPK_SLOT           3

#define ADOBE_SECURE_MkrId_eId0          0
#define ADOBE_SECURE_MkrId_eId1          1

#define RSA_SLOT_4

typedef enum DrmAdobe_CertType_e
{
    DrmAdobe_CertType_eDUAL         = 0,    /**<  Single DRM key used for signing and encryption. */
    DrmAdobe_CertType_eSIGN         = 1,    /**<  Signing key from the DRM key pair */
    DrmAdobe_Certype_eEncryption    = 2,    /**<  Encryption key from the DRM key pair */
    DrmAdobe_CertType_eUnknown
}DrmAdobe_CertType_e;

typedef enum DrmAdobe_KeyType_e
{
    DrmAdobe_KeyType_eAES128    = 0, /* 128 bytes of raw hex */
    DrmAdobe_KeyType_eRSA1024   = 1, /* PKCS #8 in PrivateKeyInfo structure if AXS_CAPABILITY_PKCS8_IMPORT_SUPPORT else min RSA structure */
    DrmAdobe_KeyType_eRSA2048   = 2, /* PKCS #8 in PrivateKeyInfo structure if AXS_CAPABILITY_PKCS8_IMPORT_SUPPORT else min RSA structure */
    DrmAdobe_KeyType_eUnknown
}DrmAdobe_KeyType_e;


typedef enum DrmAdobe_PadType_e
{
    DrmAdobe_PadType_ePKCS1V15         = 0,
    DrmAdobe_PadType_ePKCS1_OAEP       = 1,
    DrmAdobe_PadType_eNO_PADDING       = 2,
    DrmAdobe_PadType_ePKCS7            = 3,
    DrmAdobe_PadType_eUnknown
}DrmAdobe_PadType_e;


typedef enum DrmAdobe_AesMode_e
{
    DrmAdobe_AesMode_eNOOP     = 0,	/* For use when the enum is not relevent to the API call */
    DrmAdobe_AesMode_eAESECB   = 1,
    DrmAdobe_AesMode_eAESCBC   = 2,
    DrmAdobe_AesMode_eAESCTR   = 3,
    DrmAdobe_AesMode_eUnknown
}DrmAdobe_AesMode_e;

typedef enum DrmAdobe_KeyLadderType_e
{
        DrmAdobe_KeyLadderType_eSESSION_KEY     = 0,    /* The result of the decrypt is a generic/session key */
        DrmAdobe_KeyLadderType_eCONTENT_KEY     = 1,    /* The result of the decrypt is content */
        DrmAdobe_KeyLadderType_eMACHINE_KEY     = 2,    /* The result of the decrypt is a machine key */
        DrmAdobe_KeyLadderType_eDOMAIN_KEY      = 3,    /* The result of the decrypt is a domain key */
        DrmAdobe_KeyLadderType_eSHARED_DOMAIN   = 4,    /* The result of the decrypt is a shared domain key */
        DrmAdobe_KeyLadderType_eDRM_KEY         = 5,    /* The result of the decrypt is neither a key or content */
        DrmAdobe_KeyLadderType_eTRANSPORT_PUB   = 6,
        DrmAdobe_KeyLadderType_eTRANSPORT_PRIV  = 7,
        DrmAdobe_KeyLadderType_eUnknown
}DrmAdobe_KeyLadderType_e;

typedef enum DrmAdobe_RsaPubkeyOpType_e
{
    DrmAdobe_RsaPubkeyOpType_eEnc     = 0,
    DrmAdobe_RsaPubkeyOpType_eDec     = 1,
    DrmAdobe_RsaPubkeyOpType_eVerify  = 5

}DrmAdobe_RsaPubkeyOpType_e;


typedef enum DrmAdobe_SecureRsaOpType_e
{
    DrmAdobe_SecureRsaOpType_eAes2Rsa = 0,
    DrmAdobe_SecureRsaOpType_eRsa2Aes = 1,
    DrmAdobe_SecureRsaOpType_eAes2Aes = 3,
    DrmAdobe_SecureRsaOpType_eMax

}DrmAdobe_SecureRsaOpType_e;

typedef enum DrmAdobe_AdobeKeyIndex_e
{
#if ADOBE_SAGE
    DrmAdobe_AdobeKeyIndex_eSwKey                           = 0x9000,
#else
        DrmAdobe_AdobeKeyIndex_eRsaDrmPriv                  = 0x0,
        DrmAdobe_AdobeKeyIndex_eSessionKey3                 = 0x1000,
        DrmAdobe_AdobeKeyIndex_eSharedDomain                = 0x2000,
        DrmAdobe_AdobeKeyIndex_eRsaMachine                  = 0x3000,
        DrmAdobe_AdobeKeyIndex_eSessionKey1                 = 0x4000,
        DrmAdobe_AdobeKeyIndex_eRsaDomain                   = 0x5000,
        DrmAdobe_AdobeKeyIndex_eCklKey                      = 0x6000,  /*content key ladder keys*/
        DrmAdobe_AdobeKeyIndex_eSwKey                       = 0x7000,
        DrmAdobe_AdobeKeyIndex_eSecureAdobeAESkey           = 0x99999,
#endif
        DrmAdobe_AdobeKeyIndex_eRsaDrmPrivSigningkey        = 0xFFFFF,
        DrmAdobe_AdobeKeyIndex_eTransportPublicKey          = 0x00D000,
        DrmAdobe_AdobeKeyIndex_eTransportPrivateKey         = 0x00E000,

        DrmAdobe_AdobeKeyIndex_eUnknown
}DrmAdobe_AdobeKeyIndex_e;


typedef struct DrmAdobeMetaDataStructure_T
{
    uint8_t encrypt;    /**< Set to 1 for encrypt, set to 0 for decrypt */
    uint8_t DiskIO;     /**< Set to 1 if reading or writing data on disk */
}DrmAdobeMetaDataStructure;

typedef struct DrmAdobeSessionContext_t{

    DrmAdobe_AdobeKeyIndex_e adobeIndex;
    DrmCommonOperationStruct_t drmCommonOpStruct;
    DrmAdobe_PadType_e padType;
#if ADOBE_SAGE
    uint8_t iv0[16];
    uint8_t iv_curr[16];
#else
    uint32_t iv0[4];
    uint32_t iv_curr[4];
#endif
    uint8_t prevTrailingBytes[16];
    uint32_t prevTrailingBytesSize;
    bool firstChunkDecrypted;
    bool bUseCtxforDiskIO;
}DrmAdobeSessionContext_t;

typedef struct DrmAdobeKeyIndexMap_t
{
        DrmAdobe_AdobeKeyIndex_e        adobeIndex;    /*the index we return to upper layers.*/
        DrmAdobe_AdobeKeyIndex_e        EncKeyIndex;   /* index of the key with which this is wrapped*/
        DrmAdobe_PadType_e              padType;
        DrmAdobe_KeyType_e              keyType;

        uint8_t                         *keybuf;       /*we have to copy this into our local membuf and not use adobe buf*/
        uint32_t                        keybufsz;

        bool                            NeedTransform;

        bool                            swKey;         /* if this key just a sw key like public key*/

        DrmAdobe_AesMode_e              AesMode;
#if ADOBE_SAGE
    uint8_t iv[16];
#else
        uint32_t                        iv[4];         /*explore the possibility of changing to uint8_t else rename the buf as iv_wordbuf*/
        CommomCrypto_SecureRsa_KeyID    BseckKeyIndex;
        CommomCrypto_SecureRsa_KeyType  BseckKeyType;

        DrmAdobe_KeyLadderType_e        keyName;        /*Machinekey, DRM key, aes session key 1, key 2 etc. this may not be needed*/

        bool                            loaded;         /*if the key is overwritten , this flag will be set and the entire keyladder needs to be built when adobe requests an operation with this key*/
        /*needed to route the key to M2m keyslot for AES CBC mode as and CTR mode as we do not have the algo info at this point, two keyslots will be allocated
        and the appropriate one will be used*/
        NEXUS_KeySlotHandle             keySlotHndlCbc;
        NEXUS_KeySlotHandle             keySlotHndlCtr;
        uint32_t                        reserved; /*RFU*/
#endif
} DrmAdobeKeyIndexMap_t;

/**************************************************************************************
Summary:
 Adobe RSA settings structure

**************************************************************************************/
typedef struct encrypt_operation_t
{
    /* Source address and size are required for ALL RSA operations. */
    uint8_t *pSrcAddr;         /* Pointer to the source data.                                           */
    uint32_t srcDataSize;      /* Source data size. For RSA signing and verification, there is no       */
                               /* restriction on the source data size. For RSA encryption, the size     */
                               /* shall be smaller or equal to (KeySizeInBytes - 11). This is required  */
                               /* due to the PKCS #1 v1.5 padding scheme used for encryption. For RSA   */
                               /* decryption, the size shall be equal to (KeySizeInBytes).              */

    /* Destination address and size are required for RSA encryption and decryption ONLY */
    uint8_t *pDestAddr;        /* Pointer to the destination data. Used to return the result for RSA    */
                               /* encrypt and RSA decrypt.                                              */
    uint32_t *pDestDataSize;   /* [in/out] Pointer to the destination data size. For RSA encryption,    */
                               /* the size of the destination data shall be larger than or equal to     */
                               /* (KeySizeInBytes). For RSA decryption, it shall be larger than or      */
                               /* equal to the size of the decrypted data. When the decrypted data size */
                               /* size is not known, the size of the destination data could be set to   */
                               /* (KeySizeInBytes). For RSA encryption and decryption, the actual data  */
                               /* written to the destination address is also returned in this field.    */
    DrmAdobe_PadType_e padType;
}encrypt_operation_t;


typedef struct verify_operation_t
{
    uint8_t *pSignatureAddr;   /* Pointer to the signature. For RSA verification, it is used as the     */
                               /* reference signature.                                                  */
    uint32_t signatureSize;    /* For RSA verify, it means the signature size. It shall be equal to     */
                               /* (KeySizeInBytes). For RSA sign, it means the signature buffer size.   */
                               /* It shall be equal to or larger than the key size in bytes.            */

    uint8_t *pDigestAddr;
    uint32_t digestSize;

    DrmAdobe_PadType_e padType;
}verify_operation_t;


typedef struct sign_operation_t
{
    uint8_t *pSignatureAddr;   /* Pointer to the signature. For RSA signing, it is used to return the   */
                               /* computed signature.                                                   */
    uint32_t *signatureSize;   /* pointer to varible which will be filled in by API           */

    uint8_t *pSrcAddr;
    uint32_t srcDataSize;
    DrmAdobe_PadType_e padType;

}sign_operation_t;

typedef struct DrmAdobe_RsaSwSettings_t
{
    DrmAdobe_RsaPubkeyOpType_e operation_type;   /* DrmAdobe_RsaPubkeyOpType_eEnc */
                                                /* DrmAdobe_RsaPubkeyOpType_eDec */
                                                /* DrmAdobe_RsaPubkeyOpType_eVerify */

    union {
            encrypt_operation_t encrypt_op;
            verify_operation_t verify_op;
            sign_operation_t sign_op;
          }operation_struct;

}DrmAdobe_RsaSwSettings_t;

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_GetDefaultParamSettings
**
** DESCRIPTION:
**   Retrieve the default settings
**
** PARAMETERS:
** pAdobeParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_Adobe_GetDefaultParamSettings( DrmAdobeParamSettings_t *pAdobeParamSettings);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_SetParamSettings
**
** DESCRIPTION:
**   Set param settings
**
** PARAMETERS:
** adobeParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_Adobe_SetParamSettings(DrmAdobeParamSettings_t *pAdobeParamSettings);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_GetDefaultRsaSettings
**
** DESCRIPTION:
**   Get Default Rsa param settings
**
** PARAMETERS:
** DrmAdobe_RsaSwSettings_t - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/

void DRM_Adobe_GetDefaultRsaSettings(DrmAdobe_RsaSwSettings_t *pAdobeRsaSettings);


/******************************************************************************
** FUNCTION:
**  DRM_Adobe_UnInit
**
** DESCRIPTION:
**   Free all gloabal variable, delete all keys and cleanup
**
** PARAMETERS:
** void
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_UnInit(void);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_Initialize
**
** DESCRIPTION:
**   Initialize common drm and bcrypt. load the hashlock pkg and drm keys into
** secure RSA HW
**
** PARAMETERS:
** void
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_Initialize(DrmAdobeParamSettings_t *pSettings);



/******************************************************************************
** FUNCTION:
**  DRM_Adobe_GetDRMCert
**
** DESCRIPTION:
**   Return the Hw Drm cerificate from adobe_key bin file to the caller
**
** PARAMETERS:
** keyType:Input: If the HW supports multiple DRM keys, then this specifies what type, Dual/decryption for decryption cert and sign fornsigning cert
** cert:Output: Certificate of the HW block. Memory allocated by caller
** size:Input/Output: Size of certificate. If not large enough, then the HW will return the correct size.
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_GetDRMCert( uint32_t keyType,
                            uint8_t *cert,
                            uint32_t *size);


/******************************************************************************
** FUNCTION:
**  DRM_Adobe_GetRootCertDigest
**
** DESCRIPTION:
**   Return the Root Cert Digest from sdk bin file to the caller
**
** PARAMETERS:
** cert:Output: Root Certificate digest. Memory allocated by SW
** size:Input/Output: Size of certificate. If not large enough, then the HW will return the correct size
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_GetRootCertDigest(uint8_t *cert,
                                  uint32_t *size);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_GetHWID
**
** DESCRIPTION:
**   Read the OTP dev ID concatenate it with magic number and hash it and return the hash
**   as the Hw ID.
**
** PARAMETERS:
** HWID :Output, Pointer to a 64 byte buffer that is malloced by caller
** HWIDLength:Input/Output: Size of HWID. If not large enough, then error is returned
** MagicNumber: Pointer to a 16 byte buffer that is malloced by caller.
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_GetHWID(uint8_t *pHWID,
                        uint32_t *pHWIDLength,
                        uint8_t *pMagicNumber);


/******************************************************************************
** FUNCTION:
**  DRM_Adobe_MemoryFree
**
** DESCRIPTION:
**   Free the memory pointed by the input argumnet
**
** PARAMETERS:
** HWMemPTR :Pointer to memory to be freed
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_MemoryFree(uint8_t *pHWMemPTR);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_GetRand
**
** DESCRIPTION:
**   Generate Random number of given size using HW
**
** PARAMETERS:
** random :Output: Buffer to put random. Memory for random number is provided by caller
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_GetRand(uint8_t *random,
                        uint32_t size);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_LoadKey
**
** DESCRIPTION:
**   Load an encrypted key into either HW The only state information that is passed back is the index.
**   When a key is loaded, keyladderType indicates what that key is used for (content or loading another key)
**
** PARAMETERS:
** EncKeyIndex: Input: Index of key used to decrypt key data. If Index = 0, then decrypt with DRM key
** data:Input: Key data. Format determined by key type
** len: Input: Length of key data
** inputKeyType:Input: Keytype
** mode:Input: AESECB || AESCBC || AESCTR. If doing AES
** iv:Input: Pointer to a 16 byte IV buffer. Can be NULL if doing algorithm or mode with no IV
** keyladderType:Input: The index to the key returned is going to be used to decrypt more keys, content, or other
** padtype: Input: Type of padding used on the data
** metadata:Input: is intended to use to indicate HW transform flag
** index: Output:Key index to be returned
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC  DRM_Adobe_LoadKey(uint32_t EncKeyIndex,
                         uint8_t *data,
                         uint32_t len,
                         DrmAdobe_KeyType_e inputKeyType,
                         DrmAdobe_AesMode_e mode,
                         uint8_t *iv,
                         DrmAdobe_KeyLadderType_e keyladderType,
                         DrmAdobe_PadType_e padtype,
                         uint8_t *metadata,
                         uint32_t *index);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_DeleteKey
**
** DESCRIPTION:
**   Delete a key loaded either with setclearkey or Loadkey. incase of HW key, free the keyslot
**
** PARAMETERS:
** Index: Input: Index of key to be deleted
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_DeleteKey(uint32_t index);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_CreateDRMSessionCtx
**
** DESCRIPTION:
**   Create a DRM session for encrypt/decrypt operation
**
** PARAMETERS:
** index:Input: Index to key to be used
** padtype:Input: Type of padding used on the data
** mode:Input: AESECB || AESCBC || AESCTR
** iv:Input: Pointer to a 16 byte IV buffer. Will be NULL if doing AES ECB
** metadata:Input/Output: TBD
** CTX: Output:handle to the session created.
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_CreateDRMSessionCtx(uint32_t index,
                                    DrmAdobe_PadType_e padtype,
                                    DrmAdobe_AesMode_e mode,
                                    uint8_t *iv,
                                    uint8_t *metadata,
                                    DrmAdobeSessionContext_t **CTX);


/******************************************************************************
** FUNCTION:
**  DRM_Adobe_DestroyDRMSessionCtx
**
** DESCRIPTION:
**   Delete a encrypt/decrypt context
**
** PARAMETERS:
** CTX: Input: pointer to the Context to be deleted
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
void DRM_Adobe_DestroyDRMSessionCtx(DrmAdobeSessionContext_t *CTX);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_DecryptInit
**
** DESCRIPTION:
**   Setup a session to decrypt data or content
**
** PARAMETERS:
** CTX: Input: pointer to the Context to be used
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_DecryptInit(DrmAdobeSessionContext_t *CTX);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_DecryptUpdate
**
** DESCRIPTION:
**   Decrypt a chunk of data. Return all decrypted full blocks.
**
** PARAMETERS:
** CTX :in:Pointer to HW DRM context.
** input:in: Data to be decrypted
** length :in:Length of data to be decrypted
** output:in:in/out: Data Length
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_DecryptUpdate(  DrmAdobeSessionContext_t *CTX,
                                uint8_t *input,
                                uint32_t length,
                                uint8_t *output,
                                uint32_t *outlength);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_DecryptFinal
**
** DESCRIPTION:
**   Decrypt a padding bytes if any.
**
** PARAMETERS:
** AppIDHandle:in: ApplicationID Handle created by the HW
** CTX:in: Pointer to HW DRM context.
** output:[in,out]: Decrypted Output Data. Memory to be malloced by caller
** outlength:[in,out]: Data Length
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_DecryptFinal(DrmAdobeSessionContext_t *CTX,
                             uint8_t *input,
                             uint32_t length,
                             uint8_t *output,
                             uint32_t *outlength);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_DecryptFullSampleAndReturn
**
** DESCRIPTION:
**   decrypts the input encrypted data and returns the clear data
**
** PARAMETERS:
** CTX:Input: Pointer to HW DRM context. Can be NULL if not required by HW
** input:Input/Output: Data to be inline decrypted
** output:Output: Decrypted Output Data. Memory to be malloced by caller
** outlength: in/out parameter holding the length of clear data
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_DecryptFullSampleAndReturn( DrmAdobeSessionContext_t *CTX,
                                            uint8_t *input,
                                            uint32_t *length,
                                            uint8_t *output,
                                            uint32_t *outlength);


/******************************************************************************
** FUNCTION:
**  DRM_Adobe_EncryptData
**
** DESCRIPTION:
**   Encrypt a chunk of data with the key and IV nad algo in the context
**
** PARAMETERS:
** CTX: Input: Pointer to HW DRM context. Can be NULL if not required by HW
** input:Input: Data to be encrypted
** length: Input: Length of data to be encrypted
** outlength: Output: Data Length
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_EncryptData(DrmAdobeSessionContext_t *CTX,
                            uint8_t *input,
                            uint32_t length,
                            uint32_t *outlength,
                            uint8_t *output);


/******************************************************************************
** FUNCTION:
**  DRM_Adobe_SetClearKey
**
** DESCRIPTION:
**   Load a clear key that is used by openssl or M2m engine
**
** PARAMETERS:
** key: Input: Pointer to key to be loaded
** length:Input: Length of key to be loaded
** keyType:Input: AES or RSA
** keyladderType: Session key or transport private or transport public key.
** index: output:return index of the key after loading
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_SetClearKey(uint8_t * key,
                            uint32_t length,
                            DrmAdobe_KeyType_e keyType,
                            DrmAdobe_KeyLadderType_e keyladderType,
                            uint32_t *index);

/******************************************************************************
** FUNCTION:
**  DRM_Adobe_PublicKeyOperation
**
** DESCRIPTION:
**   Uses bcrypt APi to do either public key encrypt or verify
**
** PARAMETERS:
** Index: Input: Index of key to be use
** pAdobeRsaSettings:poibter to teh structure containing the input rsa parameters
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_PublicKeyOperation( uint32_t index,
                                    DrmAdobe_RsaSwSettings_t *pAdobeRsaSettings);


/******************************************************************************
** FUNCTION:
**  DRM_Adobe_PrivateKeyEncrypt
**
** DESCRIPTION:
**   signs the incoming data with private key loaded in secure RSA HW
**
** PARAMETERS:
** Index: Input: Index of key to be deleted
** pAdobeRsaSettings:poibter to teh structure containing the input rsa parameters
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_PrivateKeyEncrypt( uint32_t index,
                                   DrmAdobe_RsaSwSettings_t *pAxsRsaSettings);


/******************************************************************************
** FUNCTION:
**  DRM_Adobe_GetSharedDomainKey
**
** DESCRIPTION:
**   Reads the encrypted shared domain key from sdk.bin and return sthe wrapping key and the wrappedn key
**
** PARAMETERS:
** outWrapKey,
** outWrapKeySize,
** outPrivKey,
** outPrivKeySize
** @param[in,out] outWrapKey buffer with an AES-128 session key encrypted with the DRM key.
** @param[in,out] outWrapKeySize size of session key buffer.
** @param[in,out] outPrivKey buffer with an RSA-1024 private key encrypted with the session key in "keyFormat" format.
** @param[in,out] outPrivKeySize size of private key buffer.
**
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_GetSharedDomainKey( uint8_t * outWrapKey,
                                    uint32_t * outWrapKeySize,
                                    uint8_t * outPrivKey,
                                    uint32_t * outPrivKeySize);


/******************************************************************************
** FUNCTION:
**  DRM_Adobe_GetIndivTransportCert
**
** DESCRIPTION:
**  Read the Indivtransport certificatem form sdk.bin and return
**
** PARAMETERS:
** cert:Output: Certificate of the HW block.
** size:Input/Output: Size of certificate. If not large enough, then then return the correct size
** RETURNS:
**   DrmRC.
**
******************************************************************************/
DrmRC DRM_Adobe_GetIndivTransportCert( uint8_t *cert,
                                       uint32_t *size);
#ifdef __cplusplus
}
#endif

#endif /* #ifndef DRM_ADOBE_H__ */


