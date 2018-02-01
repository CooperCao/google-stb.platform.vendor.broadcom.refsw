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

#ifndef SSCE_TL_H__
#define SSCE_TL_H__


#include "bstd.h"
#include "bkni.h"

#include "bsagelib_types.h"
#include "bsagelib_crypto_types.h"


#ifdef __cplusplus
extern "C" {
#endif


#define RSA_PUBLIC_EXPONENT_DEFAULT  0x10001


/* This handle is used to store the context of an SsceTl instance */
typedef struct SsceTl_P_Instance *SsceTl_Handle;


typedef enum SsceTl_KeyType_e
{
    SsceTl_KeyType_eRsa = 0,
    SsceTl_KeyType_eEcc,
    SsceTl_KeyType_eMax
} SsceTl_KeyType_e;


typedef enum SsceTl_RsaSize_e
{
    SsceTl_RsaSize_e1024 = 0,
    SsceTl_RsaSize_e2048,
    SsceTl_RsaSize_e3072,
    SsceTl_RsaSize_e4096,
    SsceTl_RsaSize_eMax
} SsceTl_RsaSize_e;


typedef enum SsceTl_RsaSignAlgorithm_e
{
    SsceTl_RsaSignAlgorithm_ePkcs = 0,
    SsceTl_RsaSignAlgorithm_ePss,
    SsceTl_RsaSignAlgorithm_eMax
} SsceTl_RsaSignAlgorithm_e;


typedef enum SsceTl_EccCurve_e
{
    SsceTl_EccCurve_eprime256v1 = 0,
    SsceTl_EccCurve_eMax
}SsceTl_EccCurve_e;


typedef enum SsceTl_EccSignAlgorithm_e
{
    SsceTl_EccSignAlgorithm_eEcdsa = 0,
    SsceTl_EccSignAlgorithm_eMax
} SsceTl_EccSignAlgorithm_e;


typedef enum SsceTl_RetrieveType_e
{
    SsceTl_RetrieveType_ePublicKey = 0,
    SsceTl_RetrieveType_eCertificate,
    SsceTl_RetrieveType_eMax
} SsceTl_RetrieveType_e;


/***************************************************************************
Summary:
CreateKey Settings structure

See Also:
SsceTl_GetDefaultCreateKeySettings()
SsceTl_CreateKey()
***************************************************************************/
typedef struct SsceTl_CreateKeySettings
{
    SsceTl_KeyType_e keyType;    /* key type: RSA or ECC                     */
    SsceTl_RsaSize_e rsaSize;    /* only used for RSA keys                   */
    uint32_t rsaPublicExponent;  /* only used for RSA keys                   */
    SsceTl_EccCurve_e eccCurve;  /* only used for ECC keys                   */
    char *label;                 /* input label string                       */
    uint8_t *keyContainer;       /* output key container buffer              */
                                 /* if NULL, no output will be written       */
    uint32_t *keyContainerLen;   /* input:  size of the key container buffer */
                                 /* output: actual size used                 */
} SsceTl_CreateKeySettings;


/***************************************************************************
Summary:
LoadKey Settings structure

See Also:
SsceTl_GetDefaultLoadKeySettings()
SsceTl_LoadKey()
***************************************************************************/
typedef struct SsceTl_LoadKeySettings
{
    char *label;               /* input label string               */
    uint8_t *keyContainer;     /* input key container buffer       */
    uint32_t keyContainerLen;  /* size of the key container buffer */
} SsceTl_LoadKeySettings;


/***************************************************************************
Summary:
UpdateCertificate Settings structure

See Also:
SsceTl_GetDefaultUpdateCertificateSettings()
SsceTl_UpdateCertificate()
***************************************************************************/
typedef struct SsceTl_UpdateCertificateSettings
{
    char *label;                /* input label string                       */
    uint8_t *certificate;       /* input certificate in DER format          */
    uint32_t certificateLen;    /* size of the certificate buffer           */
    uint8_t *keyContainer;      /* output key container buffer              */
                                /* if NULL, no output will be written       */
    uint32_t *keyContainerLen;  /* input:  size of the key container buffer */
                                /* output: actual size used                 */
} SsceTl_UpdateCertificateSettings;


/***************************************************************************
Summary:
Retrieve Settings structure

See Also:
SsceTl_GetDefaultRetrieveSettings()
SsceTl_Retrieve()
***************************************************************************/
typedef struct SsceTl_RetrieveSettings
{
    SsceTl_RetrieveType_e retrieveType;  /* public key or certificate          */
    char *label;                         /* input label string                 */
    uint8_t *output;                     /* output buffer in DER format        */
                                         /* if NULL, no output will be written */
    uint32_t *outputLen;                 /* input:  size of the output buffer  */
                                         /* output: actual size used           */
} SsceTl_RetrieveSettings;


/***************************************************************************
Summary:
Sign Settings structure

See Also:
SsceTl_GetDefaultSignSettings()
SsceTl_Sign()
***************************************************************************/
typedef struct SsceTl_SignSettings
{
    BSAGElib_Crypto_ShaVariant_e shaVariant;     /* SHA variant            */
    SsceTl_RsaSignAlgorithm_e rsaSignAlgorithm;  /* only used for RSA keys */
    uint32_t rsaPssSaltLen;                      /* only used for RSA PSS  */
    SsceTl_EccSignAlgorithm_e eccSignAlgorithm;  /* only used for ECC keys */
    char *label;             /* input label string                         */
    uint8_t *digest;         /* input digest buffer                        */
    uint32_t digestLen;      /* must match the length of the shaVariant    */
    uint8_t *signature;      /* output signature buffer                    */
                             /* if NULL, no output will be written         */
    uint32_t *signatureLen;  /* input:  size of the signature buffer       */
                             /* output: actual size used                   */
} SsceTl_SignSettings;


/***************************************************************************
Summary:
Initialize an instance of the SSCE module on SAGE

See Also:
SsceTl_Uninit()
***************************************************************************/
BERR_Code SsceTl_Init(
    SsceTl_Handle *pSsceHandle);


/***************************************************************************
Summary:
Uninitialize the given instance of the SSCE module on SAGE

See Also
SsceTl_Init()
***************************************************************************/
void SsceTl_Uninit(
    SsceTl_Handle hSsceTl);


/***************************************************************************
Summary:
Get default CreateKey settings

Description:
This function is used to initialize an SsceTl_CreateKeySettings
structure with default settings

See Also
SsceTl_CreateKey()
***************************************************************************/
void SsceTl_GetDefaultCreateKeySettings(
    SsceTl_CreateKeySettings *pCreateKeySettings);


/***************************************************************************
Summary:
Creates a new asymmmetric key pair with the specified label.
SAGE will return an encrypted key container containing the asymmetric key
pair to the host.
SAGE will retain a copy of the key container in SAGE RAM until the context
is unitialized.

See Also
SsceTl_GetDefaultCreateKeySettings()
***************************************************************************/
BERR_Code SsceTl_CreateKey(
    SsceTl_Handle hSsceTl,
    SsceTl_CreateKeySettings *pCreateKeySettings);


/***************************************************************************
Summary:
Get default LoadKey settings

Description:
This function is used to initialize an SsceTl_LoadKeySettings structure with
default settings

See Also
SsceTl_LoadKey()
***************************************************************************/
void SsceTl_GetDefaultLoadKeySettings(
    SsceTl_LoadKeySettings *pLoadKeySettings);


/***************************************************************************
Summary:
Loads an encrypted key container into SAGE.
SAGE will retain a copy of the key container in SAGE RAM until the context
is unitialized.

See Also
SsceTl_GetDefaultLoadKeySettings()
***************************************************************************/
BERR_Code SsceTl_LoadKey(
    SsceTl_Handle hSsceTl,
    SsceTl_LoadKeySettings *pLoadKeySettings);


/***************************************************************************
Summary:
Get default UpdateCertificate settings

Description:
This function is used to initialize an SsceTl_UpdateCertificateSettings
structure with default settings

See Also
SsceTl_UpdateCertificate()
***************************************************************************/
void SsceTl_GetDefaultUpdateCertificateSettings(
    SsceTl_UpdateCertificateSettings *pUpdateCertificateSettings);


/***************************************************************************
Summary:
Adds an X.509 certificate into an encrypted key container containing a key
pair with the specified label.
If no key pair exists with that label, an error will be returned.
If a certificate is already associated with the key pair, an error will be
returned.
If the public key inside the X.509 certificate doesn't match the public key
with the specified label, an error will be returned.
SAGE will return an encrypted key container containing the asymmetric key
pair and certificate to the host.
SAGE will retain a copy of the key container in SAGE RAM until the context
is unitialized.

See Also
SsceTl_GetDefaultUpdateCertificateSettings()
***************************************************************************/
BERR_Code SsceTl_UpdateCertificate(
    SsceTl_Handle hSsceTl,
    SsceTl_UpdateCertificateSettings *pUpdateCertificateSettings);


/***************************************************************************
Summary:
Get default Retrieve settings

Description:
This function is used to initialize an SsceTl_RetrieveSettings
structure with default settings

See Also
SsceTl_Retrieve()
***************************************************************************/
void SsceTl_GetDefaultRetrieveSettings(
    SsceTl_RetrieveSettings *pRetrieveSettings);


/***************************************************************************
Summary:
Retrieves the public key or the certificate with the specified label.

See Also
SsceTl_GetDefaultRetrieveSettings()
***************************************************************************/
BERR_Code SsceTl_Retrieve(
    SsceTl_Handle hSsceTl,
    SsceTl_RetrieveSettings *pRetrieveSettings);


/***************************************************************************
Summary:
Get default Sign settings

Description:
This function is used to initialize an SsceTl_SignSettings structure with
default settings

See Also
SsceTl_Sign()
***************************************************************************/
void SsceTl_GetDefaultSignSettings(
    SsceTl_SignSettings *pSignSettings);


/***************************************************************************
Summary:
Signs the input using the private key with the specified label.
Signing is allowed whether or not a certificate is associated with the key
pair.

See Also
SsceTl_GetDefaultSignSettings()
***************************************************************************/
BERR_Code SsceTl_Sign(
    SsceTl_Handle hSsceTl,
    SsceTl_SignSettings *pSignSettings);


#ifdef __cplusplus
}
#endif


#endif /*SSCE_TL_H__*/
