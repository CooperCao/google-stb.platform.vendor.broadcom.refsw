/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef SECURERSA_TL_H__
#define SECURERSA_TL_H__


#include "bstd.h"
#include "bkni.h"

#include "bsagelib_types.h"
#include "bsagelib_crypto_types.h"


#ifdef __cplusplus
extern "C" {
#endif


/* This handle is used to store the context of a RsaTl instance */
typedef struct SecureRsaTl_P_Instance *SecureRsaTl_Handle;


typedef enum SecureRsaTl_KeyType_e
{
    SecureRsaTl_KeyType_eAll = 0,
    SecureRsaTl_KeyType_eRsa,
    SecureRsaTl_KeyType_eKey3,
    SecureRsaTl_KeyType_eKpk,
    SecureRsaTl_KeyType_eMax
} SecureRsaTl_KeyType_e;

typedef enum SecureRsaTl_RsaKeySlot_e
{
    SecureRsaTl_RsaKeySlot_eSlot0 = 0,
    SecureRsaTl_RsaKeySlot_eSlot1,
    SecureRsaTl_RsaKeySlot_eSlot2,
    SecureRsaTl_RsaKeySlot_eSlot3,
    SecureRsaTl_RsaKeySlot_eSlot4,
    SecureRsaTl_RsaKeySlot_eSlot5,
    SecureRsaTl_RsaKeySlot_eSlot6,
    SecureRsaTl_RsaKeySlot_eSlot7,
    SecureRsaTl_RsaKeySlot_eMax
} SecureRsaTl_RsaKeySlot_e;

typedef enum SecureRsaTl_Key3KeySlot_e
{
    SecureRsaTl_Key3KeySlot_eSlot0 = 0,
    SecureRsaTl_Key3KeySlot_eSlot1,
    SecureRsaTl_Key3KeySlot_eSlot2,
    SecureRsaTl_Key3KeySlot_eSlot3,
    SecureRsaTl_Key3KeySlot_eSlot4,
    SecureRsaTl_Key3KeySlot_eSlot5,
    SecureRsaTl_Key3KeySlot_eSlot6,
    SecureRsaTl_Key3KeySlot_eSlot7,
    SecureRsaTl_Key3KeySlot_eMax
} SecureRsaTl_Key3KeySlot_e;

typedef enum SecureRsaTl_KpkKeySlot_e
{
    SecureRsaTl_KpkKeySlot_eSlot0 = 0,
    SecureRsaTl_KpkKeySlot_eSlot1,
    SecureRsaTl_KpkKeySlot_eSlot2,
    SecureRsaTl_KpkKeySlot_eSlot3,
    SecureRsaTl_KpkKeySlot_eSlot4,
    SecureRsaTl_KpkKeySlot_eSlot5,
    SecureRsaTl_KpkKeySlot_eSlot6,
    SecureRsaTl_KpkKeySlot_eSlot7,
    SecureRsaTl_KpkKeySlot_eMax,
} SecureRsaTl_KpkKeySlot_e;

typedef enum SecureRsaTl_DigestType_e
{
    SecureRsaTl_DigestType_eSha1 = 0,
    SecureRsaTl_DigestType_eSha256,
    SecureRsaTl_DigestType_eMax
} SecureRsaTl_DigestType_e;

typedef enum SecureRsaTl_HmacType_e
{
    SecureRsaTl_HmacType_eSha1 = 0,
    SecureRsaTl_HmacType_eSha256,
    SecureRsaTl_HmacType_eMax
} SecureRsaTl_HmacType_e;

typedef enum SecureRsaTl_RsaKeyType_e
{
    SecureRsaTl_RsaKeyType_ePublic = 0,
    SecureRsaTl_RsaKeyType_ePrivate,
    SecureRsaTl_RsaKeytype_eMax
} SecureRsaTl_RsaKeyType_e;

typedef enum SecureRsaTl_RsaSignaturePadding_e
{
    SecureRsaTl_RsaSignaturePadding_ePkcs = 0,
    SecureRsaTl_RsaSignaturePadding_ePss,
    SecureRsaTl_RsaSignaturePadding_eMax
} SecureRsaTl_RsaSignaturePadding_e;

typedef enum SecureRsaTl_RsaEncryptionPadding_e
{
    SecureRsaTl_RsaEncryptionPadding_ePkcs = 0,
    SecureRsaTl_RsaEncryptionPadding_eOaep,
    SecureRsaTl_RsaEncryptionPadding_eMax
} SecureRsaTl_RsaEncryptionPadding_e;

typedef enum SecureRsaTl_Key3Usage_e
{
    SecureRsaTl_Key3Usage_eIkr = 0,
    SecureRsaTl_Key3Usage_eCa,
    SecureRsaTl_Key3Usage_eCp,
    SecureRsaTl_Key3Usage_eSageKeyladder,  /* not supported */
    SecureRsaTl_Key3Usage_eMax
} SecureRsaTl_Key3Usage_e;


/***************************************************************************
Summary:
Load RSA Package Settings structure

See Also:
SecureRsaTl_GetDefaultLoadRsaPackageSettings()
SecureRsaTl_LoadRsaPackage()
***************************************************************************/
typedef struct
{
    char drm_binfile_path[256];
}SecureRsaTl_LoadRsaPackageSettings;

/***************************************************************************
Summary:
Get Status Settings structure

See Also:
SecureRsaTl_GetDefaultGetStatusSettings()
SecureRsaTl_GetStatus()
***************************************************************************/
typedef struct SecureRsaTl_GetStatusSettings
{
    uint32_t *rsaNumKeys;
    uint32_t *key3NumKeys;
    uint32_t *kpkNumKeys;
    uint32_t *keyInfo;
    uint32_t *keyInfoLen;
} SecureRsaTl_GetStatusSettings;

#define SECURE_RSA_TL_GET_STATUS_NUM_KEY_TYPES  3

/***************************************************************************
Summary:
Remove Key Settings structure

See Also:
SecureRsaTl_GetDefaultRemoveKeySettings()
SecureRsaTl_RemoveKey()
***************************************************************************/
typedef struct SecureRsaTl_RemoveKeySettings
{
    SecureRsaTl_KeyType_e keyType;
    union keyslot
    {
        SecureRsaTl_RsaKeySlot_e rsaKeySlot;
        SecureRsaTl_Key3KeySlot_e key3KeySlot;
        SecureRsaTl_KpkKeySlot_e kpkKeySlot;
    } keyslot;
} SecureRsaTl_RemoveKeySettings;

/***************************************************************************
Summary:
RSA Sign Verify Settings structure

See Also:
SecureRsaTl_GetDefaultRsaSignSettings()
SecureRsaTl_GetDefaultRsaVerifySettings()
SecureRsaTl_RsaSign()
SecureRsaTl_RsaVerify()
***************************************************************************/
typedef struct SecureRsaTl_RsaSignVerifySettings
{
    SecureRsaTl_RsaKeySlot_e rsaKeySlot;
    uint8_t *input;
    uint32_t inputLen;
    SecureRsaTl_RsaSignaturePadding_e sigPadType;
    SecureRsaTl_DigestType_e sigDigestType;
    uint32_t sigPssSaltLen;
    uint8_t *signature;
    uint32_t *signatureLen;
} SecureRsaTl_RsaSignVerifySettings;

/***************************************************************************
Summary:
RSA Host Usage Settings structure

See Also:
SecureRsaTl_GetDefaultRsaHostUsageSettings()
SecureRsaTl_RsaHostUsage()
***************************************************************************/
typedef struct SecureRsaTl_RsaHostUsageSettings
{
    SecureRsaTl_RsaKeySlot_e rsaKeySlot;
    SecureRsaTl_RsaKeyType_e rsaKeyType;
    uint8_t *input;
    uint32_t inputLen;
    uint8_t *output;
    uint32_t *outputLen;
} SecureRsaTl_RsaHostUsageSettings;

/***************************************************************************
Summary:
RSA Decrypt AES Settings structure

See Also:
SecureRsaTl_GetDefaultRsaDecryptKey3Settings()
SecureRsaTl_GetDefaultRsaDecryptKpkSettings()
SecureRsaTl_RsaDecryptKey3()
SecureRsaTl_RsaDecryptKpk()
***************************************************************************/
typedef struct SecureRsaTl_RsaDecryptAesSettings
{
    uint32_t aesKeySlot;
    uint32_t aesKeyLen;
    SecureRsaTl_RsaKeySlot_e encKeySlot;
    SecureRsaTl_RsaEncryptionPadding_e encPadType;
    SecureRsaTl_DigestType_e encDigestType;
    uint8_t *encKeySettings;
    uint32_t encKeySettingsLen;
    uint8_t *encKey;
    uint32_t encKeyLen;  /* must be a multiple of 4 */
    SecureRsaTl_RsaKeySlot_e sigKeySlot;
    SecureRsaTl_RsaSignaturePadding_e sigPadType;
    SecureRsaTl_DigestType_e sigDigestType;
    uint32_t sigPssSaltLen;
    uint8_t *signature;
    uint32_t signatureLen;
} SecureRsaTl_RsaDecryptAesSettings;

#define SECURE_RSA_TL_RSA_DECRYPT_AES_NUM_PARAMETERS  9

/***************************************************************************
Summary:
RSA Load Public Key Settings structure

See Also:
SecureRsaTl_GetDefaultRsaLoadPublicKeySettings()
SecureRsaTl_RsaLoadPublicKey()
***************************************************************************/
typedef struct SecureRsaTl_RsaLoadPublicKeySettings
{
    SecureRsaTl_RsaKeySlot_e rsaKeySlot;
    uint8_t *publicModulus;
    uint32_t publicModulusLen;
    SecureRsaTl_RsaKeySlot_e sigKeySlot;
    SecureRsaTl_RsaSignaturePadding_e sigPadType;
    SecureRsaTl_DigestType_e sigDigestType;
    uint32_t sigPssSaltLen;
    uint8_t *signature;
    uint32_t signatureLen;
} SecureRsaTl_RsaLoadPublicKeySettings;

/***************************************************************************
Summary:
Key3 Import Export Settings structure

See Also:
SecureRsaTl_GetDefaultKey3ImportSettings()
SecureRsaTl_GetDefaultKey3ExportSettings()
SecureRsaTl_Key3Import()
SecureRsaTl_Key3Export()
***************************************************************************/
typedef struct SecureRsaTl_Key3ImportExportSettings
{
    SecureRsaTl_Key3KeySlot_e key3KeySlot;
    uint8_t *encKey3;
    uint32_t *encKey3Len;  /* input for import  */
                           /* output for export */
} SecureRsaTl_Key3ImportExportSettings;

/***************************************************************************
Summary:
Key3 Route Settings structure

See Also:
SecureRsaTl_GetDefaultKey3RouteSettings()
SecureRsaTl_Key3Route()
***************************************************************************/
typedef struct SecureRsaTl_Key3RouteSettings
{
    SecureRsaTl_Key3KeySlot_e key3KeySlot;
    SecureRsaTl_Key3Usage_e keySlotUsage;  /* only used for IKR keys */
    uint32_t keySlotNum;
    BSAGElib_Crypto_Algorithm_e routeAlgorithm;
    BSAGElib_Crypto_AlgorithmVariant_e variant;
    BSAGElib_Crypto_Operation_e operation;
    BSAGElib_Crypto_KeyType_e keyType;
    BSAGElib_Crypto_TerminationMode_e terminationMode;
    BSAGElib_Crypto_AesCounterMode_e counterMode;  /* only used for AES counter mode */
    BSAGElib_Crypto_AesCounterSize_e counterSize;  /* only used for AES counter mode */
    uint8_t *iv;
    uint32_t ivLen;
    BSAGElib_Crypto_Algorithm_e keyLadderAlgorithm;
    uint8_t *procIn;
    uint32_t procInLen;
} SecureRsaTl_Key3RouteSettings;

#define SECURE_RSA_TL_KEY3_ROUTE_NUM_PARAMETERS  11

/***************************************************************************
Summary:
Key3 Unroute Settings structure

See Also:
SecureRsaTl_GetDefaultKey3UnrouteSettings()
SecureRsaTl_Key3Unroute()
***************************************************************************/
typedef struct SecureRsaTl_Key3UnrouteSettings
{
    SecureRsaTl_Key3KeySlot_e key3KeySlot;
} SecureRsaTl_Key3UnrouteSettings;

/***************************************************************************
Summary:
Key3 Calculate HMAC Settings structure

See Also:
SecureRsaTl_GetDefaultKey3CalculateHmacSettings()
SecureRsaTl_Key3CalculateHmac()
***************************************************************************/
typedef struct SecureRsaTl_Key3CalculateHmacSettings
{
    SecureRsaTl_Key3KeySlot_e key3KeySlot;
    uint8_t *inputData;
    uint32_t inputDataLen;
    SecureRsaTl_HmacType_e hmacType;
    uint8_t *hmac;
    uint32_t *hmacLen;
} SecureRsaTl_Key3CalculateHmacSettings;

/***************************************************************************
Summary:
Key3 Append SHA Settings structure

See Also:
SecureRsaTl_GetDefaultKey3AppendShaSettings()
SecureRsaTl_Key3AppendSha()
***************************************************************************/
typedef struct SecureRsaTl_Key3AppendShaSettings
{
    SecureRsaTl_Key3KeySlot_e key3KeySlot;
    uint8_t *inputData;
    uint32_t inputDataLen;
    SecureRsaTl_DigestType_e digestType;
    uint8_t *digest;
    uint32_t *digestLen;
} SecureRsaTl_Key3AppendShaSettings;

/***************************************************************************
Summary:
Key3 Load Clear Ikr Settings structure

See Also:
SecureRsaTl_GetDefaultKey3LoadClearIkrSettings()
SecureRsaTl_Key3LoadClearIkr()
***************************************************************************/
typedef struct SecureRsaTl_Key3LoadClearIkrSettings
{
    SecureRsaTl_Key3KeySlot_e key3KeySlot;
    uint8_t *key;
    uint32_t keyLen;  /* must be 16 or 32 */
} SecureRsaTl_Key3LoadClearIkrSettings;

/***************************************************************************
Summary:
Key3 IKR Decrypt IKR Settings structure

See Also:
SecureRsaTl_GetDefaultKey3IkrDecryptIkrSettings()
SecureRsaTl_Key3IkrDecryptIkr()
***************************************************************************/
typedef struct SecureRsaTl_Key3IkrDecryptIkrSettings
{
    SecureRsaTl_Key3KeySlot_e ikrKeySlot;
    SecureRsaTl_Key3KeySlot_e encKeySlot;
    BSAGElib_Crypto_AlgorithmVariant_e variant;
    uint32_t enableTransform;
    uint8_t *encKey;
    uint32_t encKeyLen;  /* must be 16 or 32 */
    uint8_t *encIv;
    uint32_t encIvLen;
} SecureRsaTl_Key3IkrDecryptIkrSettings;

/***************************************************************************
Summary:
KPK Decrypt RSA Settings structure

See Also:
SecureRsaTl_GetDefaultKpkDecryptRsaSettings()
SecureRsaTl_KpkDecryptRsa()
***************************************************************************/
typedef struct SecureRsaTl_KpkDecryptRsaSettings
{
    SecureRsaTl_RsaKeySlot_e rsaKeySlot;
    SecureRsaTl_KpkKeySlot_e encKeySlot;
    BSAGElib_Crypto_AlgorithmVariant_e variant;
    uint8_t *encKeySettings;
    uint32_t encKeySettingsLen;
    uint8_t *encKey;
    uint32_t encKeyLen;  /* must be a multiple of 4 */
    uint8_t *encIv;
    uint32_t encIvLen;
    SecureRsaTl_RsaKeySlot_e sigKeySlot;
    SecureRsaTl_RsaSignaturePadding_e sigPadType;
    SecureRsaTl_DigestType_e sigDigestType;
    uint32_t sigPssSaltLen;
    uint8_t *signature;
    uint32_t signatureLen;
} SecureRsaTl_KpkDecryptRsaSettings;

#define SECURE_RSA_TL_KPK_DECRYPT_RSA_NUM_PARAMETERS  7

/***************************************************************************
Summary:
KPK Decrypt IKR Settings structure

See Also:
SecureRsaTl_GetDefaultKpkDecryptIkrSettings()
SecureRsaTl_KpkDecryptIkr()
***************************************************************************/
typedef struct SecureRsaTl_KpkDecryptIkrSettings
{
    SecureRsaTl_Key3KeySlot_e ikrKeySlot;
    SecureRsaTl_KpkKeySlot_e encKeySlot;
    BSAGElib_Crypto_AlgorithmVariant_e variant;
    uint8_t *encKey;
    uint32_t encKeyLen;  /* must be 16 or 32 */
    uint8_t *encIv;
    uint32_t encIvLen;
} SecureRsaTl_KpkDecryptIkrSettings;

/***************************************************************************
Summary:
Initialize an instance of the Secure RSA module on SAGE

See Also:
SecureRsaTl_Uninit()
***************************************************************************/
BERR_Code SecureRsaTl_Init(
    SecureRsaTl_Handle *pSecureRsaTlHandle);

/***************************************************************************
Summary:
Uninitialize the given instance of the Secure RSA module on SAGE

See Also
SecureRsaTl_Init()
***************************************************************************/
void SecureRsaTl_Uninit(
    SecureRsaTl_Handle hSecureRsaTl);

/***************************************************************************
Summary:
Get default load RSA package settings

Description:
This function is used to initialize a SecureRsaTl_LoadRsaPackageSettings
structure with default settings

See Also
SecureRsaTl_LoadRsaPackage()
***************************************************************************/
void SecureRsaTl_GetDefaultLoadRsaPackageSettings(
    SecureRsaTl_LoadRsaPackageSettings *pLoadRsaPackageSettings);

/***************************************************************************
Summary:
Load a SAGE Secure RSA drm bin file

See Also
SecureRsaTl_GetDefaultLoadRsaPackageSettings()
***************************************************************************/
BERR_Code SecureRsaTl_LoadRsaPackage(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_LoadRsaPackageSettings *pLoadRsaPackageSettings);

/***************************************************************************
Summary:
Get default get status settings

Description:
This function is used to initialize a SecureRsaTl_GetStatusSettings
structure with default settings

See Also
SecureRsaTl_GetStatus()
***************************************************************************/
void SecureRsaTl_GetDefaultGetStatusSettings(
    SecureRsaTl_GetStatusSettings *pGetStatusSettings);

/***************************************************************************
Summary:
Get the status of the Secure RSA context

Description:
Returns the number of keys and key information

See Also
SecureRsaTl_GetDefaultGetStatusSettings()
***************************************************************************/
BERR_Code SecureRsaTl_GetStatus(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_GetStatusSettings *pGetStatusSettings);

/***************************************************************************
Summary:
Get default remove key settings

Description:
This function is used to initialize a SecureRsaTl_RemoveKeySettings
structure with default settings

See Also
SecureRsaTl_RemoveKey()
***************************************************************************/
void SecureRsaTl_GetDefaultRemoveKeySettings(
    SecureRsaTl_RemoveKeySettings *pRemoveKeySettings);

/***************************************************************************
Summary:
Remove either all keys or a single key

See Also
SecureRsaTl_GetDefaultRemoveKeySettings()
***************************************************************************/
BERR_Code SecureRsaTl_RemoveKey(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RemoveKeySettings *pRemoveKeySettings);

/***************************************************************************
Summary:
Get default RSA sign settings

Description:
This function is used to initialize a SecureRsaTl_RsaSignVerifySettings
structure with default settings

See Also
SecureRsaTl_RsaSign()
***************************************************************************/
void SecureRsaTl_GetDefaultRsaSignSettings(
    SecureRsaTl_RsaSignVerifySettings *pRsaSignVerifySettings);

/***************************************************************************
Summary:
Get default RSA verify settings

Description:
This function is used to initialize a SecureRsaTl_RsaSignVerifySettings
structure with default settings

See Also
SecureRsaTl_RsaVerify()
***************************************************************************/
#define SecureRsaTl_GetDefaultRsaVerifySettings SecureRsaTl_GetDefaultRsaSignSettings

/***************************************************************************
Summary:
Use an RSA private key to sign input data

See Also
SecureRsaTl_GetDefaultRsaSignSettings()
***************************************************************************/
BERR_Code SecureRsaTl_RsaSign(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaSignVerifySettings *pRsaSignSettings);

/***************************************************************************
Summary:
Use RSA to verify the signature of input data

See Also
SecureRsaTl_GetDefaultRsaVerifySettings()
***************************************************************************/
BERR_Code SecureRsaTl_RsaVerify(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaSignVerifySettings *pRsaVerifySettings);

/***************************************************************************
Summary:
Get default RSA host usage settings

Description:
This function is used to initialize a SecureRsaTl_RsaHostUsageSettings
structure with default settings

See Also
SecureRsaTl_RsaHostUsage()
***************************************************************************/
void SecureRsaTl_GetDefaultRsaHostUsageSettings(
    SecureRsaTl_RsaHostUsageSettings *pRsaHostUsageSettings);

/***************************************************************************
Summary:
Perform an RSA private key operation on raw input data

See Also
SecureRsaTl_GetDefaultRsaHostUsageSettings()
***************************************************************************/
BERR_Code SecureRsaTl_RsaHostUsage(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaHostUsageSettings *pRsaHostUsageSettings);

/***************************************************************************
Summary:
Get default RSA decrypt Key3 settings

Description:
This function is used to initialize a SecureRsaTl_RsaDecryptAesSettings
structure with default settings

See Also
SecureRsaTl_RsaDecryptKey3()
***************************************************************************/
void SecureRsaTl_GetDefaultRsaDecryptKey3Settings(
    SecureRsaTl_RsaDecryptAesSettings *pRsaDecryptAesSettings);

/***************************************************************************
Summary:
Get default RSA decrypt KPK settings

Description:
This function is used to initialize a SecureRsaTl_RsaDecryptAesSettings
structure with default settings

See Also
SecureRsaTl_RsaDecryptKpk()
***************************************************************************/
#define SecureRsaTl_GetDefaultRsaDecryptKpkSettings SecureRsaTl_GetDefaultRsaDecryptKey3Settings

/***************************************************************************
Summary:
Decrypt a Key3 key using an RSA private key

See Also
SecureRsaTl_GetDefaultRsaDecryptKey3Settings()
***************************************************************************/
BERR_Code SecureRsaTl_RsaDecryptKey3(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaDecryptAesSettings *pRsaDecryptAesSettings);

/***************************************************************************
Summary:
Decrypt a KPK key using an RSA private key

See Also
SecureRsaTl_GetDefaultRsaDecryptKpkSettings()
***************************************************************************/
BERR_Code SecureRsaTl_RsaDecryptKpk(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaDecryptAesSettings *pRsaDecryptAesSettings);

/***************************************************************************
Summary:
Get default RSA load public key settings

Description:
This function is used to initialize a SecureRsaTl_RsaLoadPublicKeySettings
structure with default settings

See Also
SecureRsaTl_RsaLoadPublicKey()
***************************************************************************/
void SecureRsaTl_GetDefaultRsaLoadPublicKeySettings(
    SecureRsaTl_RsaLoadPublicKeySettings *pRsaLoadPublicKeySettings);

/***************************************************************************
Summary:
Load an RSA public key

See Also
SecureRsaTl_GetDefaultRsaLoadPublicKeySettings()
***************************************************************************/
BERR_Code SecureRsaTl_RsaLoadPublicKey(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaLoadPublicKeySettings *pRsaLoadPublicKeySettings);

/***************************************************************************
Summary:
Get default Key3 import settings

Description:
This function is used to initialize a SecureRsaTl_Key3ImportExportSettings
structure with default settings

See Also
SecureRsaTl_Key3Import()
***************************************************************************/
void SecureRsaTl_GetDefaultKey3ImportSettings(
    SecureRsaTl_Key3ImportExportSettings *pKey3ImportExportSettings);

/***************************************************************************
Summary:
Get default Key3 export settings

Description:
This function is used to initialize a SecureRsaTl_Key3ImportExportSettings
structure with default settings

See Also
SecureRsaTl_Key3Export()
***************************************************************************/
#define SecureRsaTl_GetDefaultKey3ExportSettings SecureRsaTl_GetDefaultKey3ImportSettings

/***************************************************************************
Summary:
Import a Key3 key

See Also
SecureRsaTl_GetDefaultKey3ImportSettings()
***************************************************************************/
BERR_Code SecureRsaTl_Key3Import(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3ImportExportSettings *pKey3ImportSettings);

/***************************************************************************
Summary:
Export a Key3 key

See Also
SecureRsaTl_GetDefaultKey3ExportSettings()
***************************************************************************/
BERR_Code SecureRsaTl_Key3Export(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3ImportExportSettings *pKey3ExportSettings);

/***************************************************************************
Summary:
Get default Key3 route settings

Description:
This function is used to initialize a SecureRsaTl_Key3RouteSettings
structure with default settings

See Also
SecureRsaTl_Key3Route()
***************************************************************************/
void SecureRsaTl_GetDefaultKey3RouteSettings(
    SecureRsaTl_Key3RouteSettings *pKey3RouteSettings);

/***************************************************************************
Summary:
Route a Key3 key to a crypto key slot

See Also
SecureRsaTl_GetDefaultKey3RouteSettings()
***************************************************************************/
BERR_Code SecureRsaTl_Key3Route(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3RouteSettings *pKey3RouteSettings);

/***************************************************************************
Summary:
Get default Key3 unroute settings

Description:
This function is used to initialize a SecureRsaTl_Key3UnrouteSettings
structure with default settings

See Also
SecureRsaTl_Key3Unroute()
***************************************************************************/
void SecureRsaTl_GetDefaultKey3UnrouteSettings(
    SecureRsaTl_Key3UnrouteSettings *pKey3UnrouteSettings);

/***************************************************************************
Summary:
Unroute a Key3 key from a crypto key slot

See Also
SecureRsaTl_GetDefaultKey3UnrouteSettings()
***************************************************************************/
BERR_Code SecureRsaTl_Key3Unroute(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3UnrouteSettings *pKey3UnrouteSettings);

/***************************************************************************
Summary:
Get default Key3 calculate HMAC settings

Description:
This function is used to initialize a SecureRsaTl_Key3CalculateHmacSettings
structure with default settings

See Also
SecureRsaTl_Key3CalculateHmac()
***************************************************************************/
void SecureRsaTl_GetDefaultKey3CalculateHmacSettings(
    SecureRsaTl_Key3CalculateHmacSettings *pKey3CalculateHmacSettings);

/***************************************************************************
Summary:
Calculate HMAC with a Key3 key

See Also
SecureRsaTl_GetDefaultKey3CalculateHmacSettings()
***************************************************************************/
BERR_Code SecureRsaTl_Key3CalculateHmac(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3CalculateHmacSettings *pKey3CalculateHmacSettings);

/***************************************************************************
Summary:
Get default Key3 append SHA settings

Description:
This function is used to initialize a SecureRsaTl_Key3AppendShaSettings
structure with default settings

See Also
SecureRsaTl_Key3AppendSha()
***************************************************************************/
void SecureRsaTl_GetDefaultKey3AppendShaSettings(
    SecureRsaTl_Key3AppendShaSettings *pKey3AppendShaSettings);

/***************************************************************************
Summary:
Calculate a Key3 appended SHA

See Also
SecureRsaTl_GetDefaultKey3AppendShaSettings()
***************************************************************************/
BERR_Code SecureRsaTl_Key3AppendSha(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3AppendShaSettings *pKey3AppendShaSettings);

/***************************************************************************
Summary:
Get default Key3 load clear IKR settings

Description:
This function is used to initialize a SecureRsaTl_Key3LoadClearIkrSettings
structure with default settings

See Also
SecureRsaTl_Key3LoadClearIkr()
***************************************************************************/
void SecureRsaTl_GetDefaultKey3LoadClearIkrSettings(
    SecureRsaTl_Key3LoadClearIkrSettings *pKey3LoadClearIkrSettings);

/***************************************************************************
Summary:
Load a clear key to a Key3 IKR key

See Also
SecureRsaTl_GetDefaultKey3LoadClearIkrSettings()
***************************************************************************/
BERR_Code SecureRsaTl_Key3LoadClearIkr(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3LoadClearIkrSettings *pKey3LoadClearIkrSettings);

/***************************************************************************
Summary:
Get default Key3 IKR decrypt IKR settings

Description:
This function is used to initialize a SecureRsaTl_Key3IkrDecryptIkrSettings
structure with default settings

See Also
SecureRsaTl_Key3IkrDecryptIkr()
***************************************************************************/
void SecureRsaTl_GetDefaultKey3IkrDecryptIkrSettings(
    SecureRsaTl_Key3IkrDecryptIkrSettings *pKey3IkrDecryptIkrSettings);

/***************************************************************************
Summary:
Decrypt an IKR key using an IKR key

See Also
SecureRsaTl_GetDefaultKey3IkrDecryptIkrSettings()
***************************************************************************/
BERR_Code SecureRsaTl_Key3IkrDecryptIkr(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3IkrDecryptIkrSettings *pKey3IkrDecryptIkrSettings);

/***************************************************************************
Summary:
Get default KPK decrypt RSA settings

Description:
This function is used to initialize a SecureRsaTl_KpkDecryptRsaSettings
structure with default settings

See Also
SecureRsaTl_KpkDecryptRsa()
***************************************************************************/
void SecureRsaTl_GetDefaultKpkDecryptRsaSettings(
    SecureRsaTl_KpkDecryptRsaSettings *pKpkDecryptRsaSettings);

/***************************************************************************
Summary:
Decrypt an RSA key using a KPK key

See Also
SecureRsaTl_GetDefaultKpkDecryptRsaSettings()
***************************************************************************/
BERR_Code SecureRsaTl_KpkDecryptRsa(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_KpkDecryptRsaSettings *pKpkDecryptRsaSettings);

/***************************************************************************
Summary:
Get default KPK decrypt IKR settings

Description:
This function is used to initialize a SecureRsaTl_KpkDecryptIkrSettings
structure with default settings

See Also
SecureRsaTl_KpkDecryptIkr()
***************************************************************************/
void SecureRsaTl_GetDefaultKpkDecryptIkrSettings(
    SecureRsaTl_KpkDecryptIkrSettings *pKpkDecryptIkrSettings);

/***************************************************************************
Summary:
Decrypt an IKR key using a KPK key

See Also
SecureRsaTl_GetDefaultKpkDecryptIkrSettings()
***************************************************************************/
BERR_Code SecureRsaTl_KpkDecryptIkr(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_KpkDecryptIkrSettings *pKpkDecryptIkrSettings);


#ifdef __cplusplus
}
#endif


#endif /*SECURERSA_TL_H__*/
