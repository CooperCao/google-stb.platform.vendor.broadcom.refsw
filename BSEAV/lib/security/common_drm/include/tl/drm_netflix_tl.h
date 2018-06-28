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
#ifndef DRM_NETFLIX__TL_H_
#define DRM_NETFLIX__TL_H_

#include "drm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DrmNetflixSageContext_t *DrmNetFlixSageHandle;

typedef void * DrmNetflixSageAesCtrCtx_t;

typedef void * DrmNetflixSageHmacCtx_t;

typedef void * DrmNetflixSageDHCtx_t;

typedef void * netflixContext_t; /* Netflix context on SAGE */

typedef enum {
        e_AUTH_KCE  = 0,
        e_AUTH_KPE  = 1,
        e_AUTH_KEE  = 2,
        e_HMAC_KCH  = 3,
        e_HMAC_KPH  = 4,
        e_HMAC_KEH  = 5,
        e_KCE_KCH   = 6, /* Client encryption key nad HMAC keys */
        e_KEE_KEH   = 7  /* Client ephemeral key nad HMAC keys */
} NetFlixKeyType_e;

typedef enum {
        e_DIGEST_SHA1   = 0,
        e_DIGEST_SHA224 = 1,
        e_DIGEST_SHA256 = 2,
        e_DIGEST_SHA384 = 3,
        e_DIGEST_SHA512 = 4
} NetFlixDigestType_t;

typedef enum {
        e_AES_128_ECB = 0,
        e_AES_128_CBC = 1
} NetFlixCipherMode_t;

#define HMAC_MAX_SIZE     64      /* longest known is SHA512 */

/* NRD 4.1 */
typedef struct {
    uint32_t key_handle;
    uint32_t key_type;
    uint32_t algorithm;
    uint32_t key_usage_flags;

} NetflixKeyInfo_t;

typedef enum {
        e_NETFLIX_TL_ENCRYPT = 0,
        e_NETFLIX_TL_DECRYPT = 1
} NetFlixSecureStoreOp_t;

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Initialize
//
// DESCRIPTION:
//   This function reads encrypted key image from the user specified file
//   MUST BE CALLED PRIOR TO USING ANY OTHER API IN THE MODULE.
//   key_file is the filepath of the DRM Utility generated key image file
//
// @param[in] key_file  Represents the file path in the root file system where
//                      where the DRM bin file is located
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Initialize(char                     *key_file,
                             DrmNetFlixSageHandle     *netflixSageHandle);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Finalize
//
// DESCRIPTION:
//   Clean up the DRM_Netflix module
//
// @param[in] netflixSageHandle Netflix Sage Module handle
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Finalize(DrmNetFlixSageHandle    netflixSageHandle);


/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Decrypt
//
// DESCRIPTION:
//  Decrypts the data in SAGE using SAGE SecureStore API.
//
// @param[in]      pHandle     Netflix Sage Module handle
// @param[in/out] *pBuf        Pointer to the buffer of encrypted data
// @param[in]      uiSize      Size of the data to decrypt.
// @param[out]    *pOutSize    Size of the decrypted string
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Decrypt(DrmNetFlixSageHandle   pHandle,
                          uint8_t               *pBuf,
                          uint32_t               uiSize,
                          uint32_t              *pOutSize);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Encrypt
//
// DESCRIPTION:
//  Encrypt the data in SAGE using SAGE SecureStore API.
//
// @param[in/out]     *pBuf        Pointer to the buffer of the data
// @param[in]          uiSize      Size of the data to decrypt.
// @param[out]        *pOutSize    Size of the encrypted string
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Encrypt(DrmNetFlixSageHandle   pHandle,
                          uint8_t               *pBuf,
                          uint32_t               uiSize,
                          uint32_t              *pOutSize);


/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_AesCtr_Init
//
// DESCRIPTION:
// Initialize the AesCtr Cipher at the SAGE.
//
// @param[in]      pHandle       Netflix sage handle
// @param[in]      authKeyType   Key type to be used for crypto operations
// @param[in]      cipherMode    Aes Ctr Mode: 128 CBC or ECB
// @param[out]    *pAesCipherCtx The returned AESCtr handle created by SAGE
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_AesCtr_Init(DrmNetFlixSageHandle       pHandle,
                              NetFlixKeyType_e           authKeyType,
                              NetFlixCipherMode_t        cipherMode,
                              DrmNetflixSageAesCtrCtx_t *pAesCipherCtx);


/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_AesCtr_UnInit
//
// DESCRIPTION:
// Uninitialize the AesCtr Cipher at the SAGE.
//
// @param[in]      pHandle       Netflix sage handle
// @param[in]      pAesCipherCtx The AESCtr handle created by SAGE
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_AesCtr_UnInit(DrmNetFlixSageHandle       pHandle,
                                DrmNetflixSageAesCtrCtx_t  pAesCipherCtx);


/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_AesCtr_EncryptInit
//
// DESCRIPTION:
// Initialize the AesCtr Encryption Cipher at the SAGE.
//
// @param[in]         pHandle       Netflix sage handle
// @param[in]         pCipherCtx    The AESCtr handle created by SAGE
// @param[in|option] *pEncKey       Encrypted AES Key if applicable
// @param[in|option]  encKeySize    size of the encrypted AES key if provided
// @param[in|option] *pIV           IV if AES mode is CBC
// @param[in]        *ivSize        size of the IV
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_AesCtr_EncryptInit(DrmNetFlixSageHandle       pHandle,
                                     DrmNetflixSageAesCtrCtx_t  pCipherCtx,
                                     uint8_t                   *pEncKey,
                                     uint32_t                   encKeySize,
                                     uint8_t                   *pIV,
                                     uint32_t                   ivSize);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_AesCtr_DecryptInit
//
// DESCRIPTION:
// Initialize the AesCtr decryption Cipher at the SAGE.
//
// @param[in]         pHandle       Netflix sage handle
// @param[in]         pCipherCtx    The AESCtr handle created by SAGE
// @param[in|option] *pEncKey       Encrypted AES Key if applicable
// @param[in|option]  encKeySize    size of the encrypted AES key if provided
// @param[in|option] *pIV           IV if AES mode is CBC
// @param[in]        *ivSize        size of the IV
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_AesCtr_DecryptInit(DrmNetFlixSageHandle       pHandle,
                                     DrmNetflixSageAesCtrCtx_t  pCipherCtx,
                                     uint8_t                   *pEncKey,
                                     uint32_t                   encKeySize,
                                     uint8_t                   *pIV,
                                     uint32_t                   ivSize);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_AesCtr_Update
//
// DESCRIPTION:
//  Start the Aes operation for the data
//
// @param[in]    pHandle       Netflix sage handle
// @param[in]    pCipherCtx    The AESCtr handle created by SAGE
// @param[in]   *pInBuf        Input data buffer for the AES operation
// @param[in]   *inBufSize     size of the input data buffer
// @param[out]  *pOutBuf       Output data buffer (should be allocated big enough )
// @param[in]    outBufSize    size of the allocated output data buffer
// @param[out]  *outputLen     the actual size of the output data after the AES operaiton
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_AesCtr_Update(DrmNetFlixSageHandle       pHandle,
                                DrmNetflixSageAesCtrCtx_t  pCipherCtx,
                                const uint8_t             *pInBuf,
                                uint32_t                   inBufSize,
                                uint8_t                   *pOutBuf,
                                uint32_t                   outBufSize,
                                int                       *outputLen);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_AesCtr_Final
//
// DESCRIPTION:
//  End the Aes opertaion for the data. This will returned the entire data for
//  the AES operation. Therefore, caller must allocate enough memory for
//  the output buffer.
//
// @param[in]    pHandle       Netflix sage handle
// @param[in]    pCipherCtx    The returned AESCtr handle created by SAGE
// @param[out]  *pOutBuf       Output data buffer (should be allocated big enough )
// @param[in]    outBufSize    size of the allocated output data buffer
// @param[out]  *outputLen     the actual size of the output data after the AES operaiton
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_AesCtr_Final(DrmNetFlixSageHandle       pHandle,
                               DrmNetflixSageAesCtrCtx_t  pCipherCtx,
                               uint8_t                   *pOutBuf,
                               uint32_t                   outBufSize,
                               int                       *outputLen);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Hmac_Init
//
// DESCRIPTION:
//  Initialize the HMAC at the SAGE.
//
// @param[in]        pHandle       Netflix sage handle
// @param[in|out]   *pHmacCtx      The returned Hmac handle created by SAGE
// @param[in]        hmacKeyType   HMAC key type either KPH or KCH
// @param[in]        digestType    Type of digest to use. Currently supports only SHA256
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Hmac_Init(DrmNetFlixSageHandle      pHandle,
                            DrmNetflixSageHmacCtx_t  *pHmacCtx,
                            NetFlixKeyType_e          hmacKeyType,
                            NetFlixDigestType_t       digestType);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Hmac_UnInit
//
// DESCRIPTION:
//  Uninitialize the HMAC at the SAGE.
//
// @param[in]        pHandle       Netflix sage handle
// @param[in]       *pHmacCtx      The returned Hmac handle created by SAGE
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Hmac_UnInit(DrmNetFlixSageHandle      pHandle,
                              DrmNetflixSageHmacCtx_t   pHmacCtx);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Hmac_Compute
//
// DESCRIPTION:
//  Compute the HMAC digest for the data.
//
// @param[in]         pHandle         Netflix sage handle
// @param[in]        *pHmacCtx        The Hmac handle created by SAGE
// @param[in|option] *pEncHmacKey     Encrypted HMAC Key if applicable
// @param[in|option]  encHmacKeySize  size of the encrypted HMAC key if provided
// @param[in]        *pInput          Input data buffer
// @param[in]        *inputSize       size of the input data buffer
// @param[out]       *pOutput         Output data buffer (should be allocated big enough )
// @param[out]       *outputSize      size of the output data
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Hmac_Compute(DrmNetFlixSageHandle      pHandle,
                               DrmNetflixSageHmacCtx_t   pHmacCtx,
                               uint8_t                  *pEncHmacKey,
                               uint32_t                  encHmacKeySize,
                               const uint8_t            *pInput,
                               uint32_t                  inputSize,
                               uint8_t                  *pOutput,
                               uint32_t                 *outputSize);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_DH_Init
//
// DESCRIPTION:
//  Initialize the DiffieHellman cipher at the SAGE.
//
// @param[in]        pHandle       Netflix sage handle
// @param[in|out]   *pDiffHellCtx  The returned DH handle created by SAGE
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_DH_Init(DrmNetFlixSageHandle    pHandle,
                          DrmNetflixSageDHCtx_t  *pDiffHellCtx);


/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_DH_UnInit
//
// DESCRIPTION:
//  Uninitialize the DiffieHellman cipher at the SAGE.
//
// @param[in]    pHandle       Netflix sage handle
// @param[in]   *pDiffHellCtx  The DH handle created by SAGE
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_DH_UnInit(DrmNetFlixSageHandle    pHandle,
                            DrmNetflixSageDHCtx_t   pDiffHellCtx);


/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_DH_Compute_Shared_Secret
//
// DESCRIPTION:
//  Compute the DH shared secret with the given public key.
//
// @param[in]    pHandle           Netflix sage handle
// @param[in]   *pDiffHellCtx      The DH handle created by SAGE
// @param[in]   *pPubKey           The server public key to use
// @param[in]    pubKeySize        The size of the public key.
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_DH_Compute_Shared_Secret(DrmNetFlixSageHandle    pHandle,
                                           DrmNetflixSageDHCtx_t   pDiffHellCtx,
                                           const uint8_t          *pPubKey,
                                           uint32_t                pubKeySize);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_DH_Get_PubKeySize
//
// DESCRIPTION:
//  Get the computed public key size of the DiffieHellman from SAGE
//
// @param[in]    pHandle           Netflix sage handle
// @param[in]   *pDiffHellCtx      The DH handle created by SAGE
// @param[out]  *pPubKeySize       The returned size of the generated public key.
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_DH_Get_PubKeySize(DrmNetFlixSageHandle    pHandle,
                                    DrmNetflixSageDHCtx_t   pDiffHellCtx,
                                    uint32_t               *pPubKeySize); /*output*/

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_DH_Get_PubKey
//
// DESCRIPTION:
//  Get the computed public key of the DiffieHellman from SAGE.
//
// @param[in]      pHandle           Netflix sage handle
// @param[in]     *pDiffHellCtx      The DH handle created by SAGE
// @param[in|out] *pPubKey           The data buffer for storing the public key
// @param[in|out] *pPubKeySize       The actual size of the public key.
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_DH_Get_PubKey(DrmNetFlixSageHandle    pHandle,
                                DrmNetflixSageDHCtx_t   pDiffHellCtx,
                                uint8_t                *pPubKey,
                                uint32_t               *pubKeySize);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_ClientKeys_Create
//
// DESCRIPTION:
//  Create a client key pairs such as Kce/kch. The returned keys are in encrtyped
//  form.
//
// @param[in]         pHandle             Netflix sage handle
// @param[in]        *pDiffHellCtx        The DH handle created by SAGE
// @param[in]         keyTypeToBeCreated  Key type either kce/kch or kee/keh
// @param[in]         dh_pubKey_v2        Public Kye in verion 2 format
// @param[in|out]    *pOutEncKey          The returned key in encrypted form
// @param[in|out]    *pOutEncKeySize      size of the encrypted Key
// @param[in|out]    *pOutEncHmacKey      The returned Hmac key in encrypted form
// @param[in|out]    *pOutEncKeySize      size of the encrypted Hmac Key
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_ClientKeys_Create(DrmNetFlixSageHandle    pHandle,
                                    DrmNetflixSageDHCtx_t   pDiffHellCtx,
                                    NetFlixKeyType_e        keyTypeToBeCreated,
                                    bool                    dh_pubkey_v2,
                                    uint8_t                *pOutEncKey,
                                    uint32_t               *pOutEncKeySize,
                                    uint8_t                *pOutEncHmac,
                                    uint32_t               *pOutEncHmacSize);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Get_EsnSize
//
// DESCRIPTION:
//  Get the the size of the ESN.
//
// @param[in]      pHandle           Netflix sage handle
// @param[in|out] *pEsnSize          The size of the ESN
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Get_EsnSize(DrmNetFlixSageHandle    pHandle,
                              uint32_t               *pEsnSize); /*output*/


/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Get_Esn
//
// DESCRIPTION:
//  Get the the szie of the ESN.
//
// @param[in]      pHandle       Netflix sage handle
// @param[in|out] *pEsn          The data buffer to store the ESN
// @param[in|out] *pEsnSize      The returned size of the ESN
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Get_Esn(DrmNetFlixSageHandle    pHandle,
                          uint8_t                *pEsn,
                          uint32_t               *pEsnSize); /*in|out*/


/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Import_Key
//
// DESCRIPTION:
//  Import a clear key to SAGE
//
// @param[in]      pHandle       Netflix sage handle
// @param[in]      keyDataPtr    Key to be imported
// @param[in]      keySize       The length of the Key
// @param[in]      keyFormat     key format
// @param[in]      algorithm     Algorithm for the key use
// @param[in]      keyUsageFlags Key usages (ENCRYP, DECRYPT, SIGNED, WRAP...)
// @param[out]    *keyHandlePtr  The returned key handle
// @param[out]    *keyTypePtr    The returned key type (SECRET,PUBLIC,PRIVAT...)
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Import_Key(DrmNetFlixSageHandle    pHandle,
                             uint8_t                *keyDataPtr,
                             uint32_t                keySize,
                             uint32_t                keyFormat,
                             uint32_t                algorithm,
                             uint32_t                keyUsageFlags,
                             uint32_t               *keyHandlePtr,  /*out*/
                             uint32_t               *keyTypePtr);   /*out*/

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Import_Sealed_Key
//
// DESCRIPTION:
//  Import a sealed clear key to SAGE
//
// @param[in]      pHandle             Netflix sage handle
// @param[in]     *sealedKeyDataPtr    Sealed Key to be imported
// @param[in]      sealedkeySize       The length of the Sealed Key
// @param[out]    *keyHandlePtr        The returned key handle
// @param[out]    *algorithmPtr        The returned Algorithm for the key
// @param[out]    *keyUsageFlagsPtr    The returned Key usages (ENCRYP, DECRYPT, SIGNED, WRAP...)
// @param[out]    *keyTypePtr          The returned key type (SECRET,PUBLIC,PRIVAT...)
// @param[out]    *keySizePtr          The returned key size
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Import_Sealed_Key(DrmNetFlixSageHandle    pHandle,
                                    uint8_t                *sealedKeyDataPtr,
                                    uint32_t                sealedKeySize,
                                    uint32_t               *keyHandlePtr,     /*out*/
                                    uint32_t               *algorithmPtr,     /*out*/
                                    uint32_t               *keyUsageFlagsPtr, /*out*/
                                    uint32_t               *keyTypePtr,       /*out*/
                                    uint32_t               *keySizePtr);      /*out*/

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Export_Key
//
// DESCRIPTION:
//  Export a clear key from SAGE (the key must be either an extractable or a DH
//  public key).
//
// @param[in]       pHandle       Netflix sage handle
// @param[in]       keyHandle     Key handle for the exported key
// @param[in]       keyFormat     key format
// @param[out]     *keyDataPtr    The returned key data
// @param[in|out]  *keySizePtr    The actual key size returned
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Export_Key(DrmNetFlixSageHandle    pHandle,
                             uint32_t                keyHandle,
                             uint32_t                keyFormat,
                             uint8_t                *keyDataPtr, /*out*/
                             uint32_t               *keySizePtr);/*in|out*/

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Export_Sealed_Key
//
// DESCRIPTION:
//  Export a sealed key from SAGE
//
// @param[in]       pHandle             Netflix sage handle
// @param[in]       sealedKeyHandle     Key handle for the exported key
// @param[out]     *sealedKeyDataPtr    The returned sealed key data
// @param[in|out]  *maxKeySizePtr       The actual key size returned
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Export_Sealed_Key(DrmNetFlixSageHandle    pHandle,
                                    uint32_t                sealedKeyHandle,
                                    uint8_t                *sealedKeyDataPtr, /*out*/
                                    uint32_t               *maxKeySizePtr);   /*in|out*/

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Get_Key_Info
//
// DESCRIPTION:
//  Get a detail get info for the key handle
//
// @param[in]       pHandle             Netflix sage handle
// @param[in]       keyHandle           Requesting Key Handle
// @param[out]     *keyTypePtr          The returned key type
// @param[out]     *extractable         The returned extractable flag
// @param[out]     *algorithmPtr        The returned algorithm
// @param[out]     *keyUsageFlagsPtr    The returned key usage flags
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Get_Key_Info(DrmNetFlixSageHandle    pHandle,
                               uint32_t                keyHandle,
                               uint32_t               *keyTypePtr,        /*out*/
                               bool                   *extractable,       /*out*/
                               uint32_t               *algorithmPtr,      /*out*/
                               uint32_t               *keyUsageFlagsPtr); /*out*/

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Get_Named_Key
//
// DESCRIPTION:
//  Get a key handle for the given name
//
// @param[in]       pHandle             Netflix sage handle
// @param[in]       namedKeyPtr         Requesting Key Name
// @param[in]       namedKeySize        Requesting Key Name size
// @param[out]      keyHandlePtr        The returned key Handle
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Get_Named_Key(DrmNetFlixSageHandle    pHandle,
                                uint8_t                *namedKeyPtr,
                                uint32_t                namedKeySize,
                                uint32_t               *keyHandlePtr); /*out*/

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_AES_CBC
//
// DESCRIPTION:
//  Perform AES CBC using the key handle.
//
// @param[in]       pHandle             Netflix sage handle
// @param[in]       keyHandle           Key handle to be used
// @param[in]       op                  crypto operation, either ENCRYPT or DECRYPT
// @param[in]       IVPtr               IV
// @param[in]       IVDataSize          IV data Size
// @param[in]       InDataPtr           Input Data
// @param[in]       InDataSize          Input Data Size
// @param[out]      OutDataPtr          The returned data
// @param[out]      OutDataSize         The returned data size
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_AES_CBC(DrmNetFlixSageHandle    pHandle,
                          uint32_t                keyHandle,
                          uint32_t                op,
                          uint8_t                *ivPtr,
                          uint32_t                ivDataSize,
                          uint8_t                *inDataPtr,
                          uint32_t                inDataSize,
                          uint8_t                *outDataPtr,   /*out*/
                          uint32_t               *outDataSize); /*out*/

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_HMAC
//
// DESCRIPTION:
//  Compute the HMAC using the given key handle
//
// @param[in]       pHandle             Netflix sage handle
// @param[in]       hmacKeyHandle       HMAC Key handle to be used
// @param[in]       shaType             SHA Digest type
// @param[in]       InDataPtr           Input Data
// @param[in]       InDataSize          Input Data Size
// @param[out]      OutDataPtr          The returned data
// @param[in|out]   OutDataSize         The returned data size
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_HMAC(DrmNetFlixSageHandle    pHandle,
                       uint32_t                hmacKeyHandle,
                       uint32_t                shaType,
                       uint8_t                *inDataPtr,
                       uint32_t                inDataSize,
                       uint8_t                *outDataPtr,   /*out*/
                       uint32_t               *outDataSize); /*in|out*/

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_HMAC_Verify
//
// DESCRIPTION:
//  Verify the HMAC digest with the key handle
//
// @param[in]       pHandle             Netflix sage handle
// @param[in]       hmacKeyHandle       HMAC Key handle to be used
// @param[in]       shaType             SHA Digest type
// @param[in]       InDataPtr           Input Data
// @param[in]       InDataSize          Input Data Size
// @param[in]       hmacDataPtr         The hmac data to be verified
// @param[in]       hmacDataSize        The size of the hamc data
// @param[out]      verified            The returned verfied flag
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_HMAC_Verify(DrmNetFlixSageHandle    pHandle,
                              uint32_t                hmacKeyHandle,
                              uint32_t                shaType,
                              uint8_t                *inDataPtr,
                              uint32_t                inDataSize,
                              uint8_t                *hmacDataPtr,
                              uint32_t                hmacDataSize,
                              bool                   *verified);
/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_DH_Gen_Keys
//
// DESCRIPTION:
//  Generate a DH Private and Public keys in SAG by the given prime and generator
//  values.
//
// @param[in]       pHandle             Netflix sage handle
// @param[in]       generator           Generator to be used
// @param[in]       primePtr            Prime data
// @param[in]       primeSize           Prime data size
// @param[out]      pubKeyHandlePtr     The returned public key handle
// @param[out]      privKeyHandlePtr    The returned private key handle
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_DH_Gen_Keys_NRD5_1(DrmNetFlixSageHandle    pHandle,
                              uint8_t                *generatorPtr,
                              uint32_t                generatorSize,
                              uint8_t                *primePtr,
                              uint32_t                primeSize,
                              uint8_t               *pubKey,
                              uint32_t               *privKeyHandle); /*out*/
/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_DH_Gen_Keys
//
// DESCRIPTION:
//  Generate a DH Private and Public keys in SAG by the given prime and generator
//  values.
//
// @param[in]       pHandle             Netflix sage handle
// @param[in]       generator           Generator to be used
// @param[in]       primePtr            Prime data
// @param[in]       primeSize           Prime data size
// @param[out]      pubKeyHandlePtr     The returned public key handle
// @param[out]      privKeyHandlePtr    The returned private key handle
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_DH_Gen_Keys(DrmNetFlixSageHandle    pHandle,
                              uint8_t                *generatorPtr,
                              uint32_t                generatorSize,
                              uint8_t                *primePtr,
                              uint32_t                primeSize,
                              uint32_t               *pubKeyHandle,   /*out*/
                              uint32_t               *privKeyHandle); /*out*/

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_DH_Derive_Keys
//
// DESCRIPTION:
//  Derive the keys using DH shared secret, which is generated using the peer
//  public key, the DH private key and the given derivation key (probably DKW).
//  values.
//
// @param[in]       pHandle              Netflix sage handle
// @param[in]       dhPrivKeyHandle      DH private key handle
// @param[in]       derivationKeyHandle  Derivation key handle, could be anything valid key handle
// @param[in]       peerPublicKeyPtr     Peer public key
// @param[in]       peerPublicKeySize    Peer public key size
// @param[out]      encKeyHandlePtr      The derived encrytpion key handle  (kce)
// @param[out]      hmacKeyHandlePtr     The derived HMAC key handle        (kch)
// @param[out]      wrapKeyHandlePtr     The derived wrapping key key handle(kcd)
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_DH_Derive_Keys(DrmNetFlixSageHandle    pHandle,
                                 uint32_t                dhPrivKeyHandle,
                                 uint32_t                derivationKeyHandle,
                                 uint8_t                *peerPubKeyPtr,
                                 uint32_t                peerPubKeySize,
                                 uint32_t               *encKeyHandlePtr,   /*out*/
                                 uint32_t               *hmacKeyHandlePtr,  /*out*/
                                 uint32_t               *wrapKeyHandlePtr); /*out*/

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_DH_DELETE_KEY:
//
// DESCRIPTION:
//  Delete the associated raw key from SAGE key store for the key handle.
//
// @param[in]       pHandle              Netflix sage handle
// @param[in]       keyHandle            Key handle
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Delete_Key(DrmNetFlixSageHandle    pHandle,
                             uint32_t                keyHandle);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Secure_Store_Op
//
// DESCRIPTION:
//  Perform SAGE Secure Store Operations, which is either encrypt or decrypt
//  the data in SAGE using SAGE SecureStore API.
//
// @param[in]         *pBuf       Pointer to the buffer of the data
// @param[in]          uiSize     Size of the data for the operations
// @param[out]       *pOutBuf     Pointer to the output buffer
// @param[in|out]    *pOutSize    Size of the encrypted string
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Secure_Store_Op(DrmNetFlixSageHandle   pHandle,
                                  uint8_t               *pBuf,
                                  uint32_t               uiSize,
                                  uint8_t               *pOutBuf,
                                  uint32_t              *pOutSize,
                                  uint32_t               op);
/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_SetNRDVersion
//
// DESCRIPTION:
//  From NRD 5.1 onwards, the netflix DPI will set the Version.
//
//
// @param[in]         version      nrd version


// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_SetNRDVersion(DrmNetFlixSageHandle   pHandle,
    uint32_t               nrdVersion);
#ifdef __cplusplus
}
#endif

#endif /*DRM_NETFLIX_H_*/
