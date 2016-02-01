/*********************************************************************************
*     Copyright (c) 2013, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*   This program is the proprietary software of Broadcom Corporation and/or its licensors, 
*   and may only be used, duplicated, modified or distributed pursuant to the terms and 
*   conditions of a separate, written license agreement executed between you and Broadcom 
*   (an "Authorized License").  Except as set forth in an Authorized License, 
*   Broadcom grants no license (express or implied), right to use, or waiver of any kind 
*   with respect to the Software, and Broadcom expressly reserves all rights in and to the 
*   Software and all intellectual property rights therein.  
* 
*   IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, 
*   AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*   Except as expressly set forth in the Authorized License,
*   1.     This program, including its structure, sequence and organization, constitutes the 
*       valuable trade secrets of Broadcom, and you shall use all reasonable efforts to protect 
*       the confidentiality thereof, and to use this information only in connection with your use 
*       of Broadcom integrated circuit products.
*   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL 
*       FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, 
*       STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND 
*       ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
*       LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO 
*       DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS BE LIABLE 
*       FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT 
*       OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN 
*       ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID 
*       FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING 
*       ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*********************************************************************************/

#ifndef COMMON_CRYPTO_SECURERSA_H__
#define COMMON_CRYPTO_SECURERSA_H__


#include "nexus_security_datatypes.h"
#include "nexus_keyladder.h"
#include "nexus_security_rsa.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum CommomCrypto_SecureRsa_KeyType 
{
    CommomCrypto_SecureRsa_KeyType_Rsa  = 0,
    CommomCrypto_SecureRsa_KeyType_Kpk,
    CommomCrypto_SecureRsa_KeyType_Mkr,
    CommomCrypto_SecureRsa_KeyType_eMax
} CommomCrypto_SecureRsa_KeyType;

typedef enum CommomCrypto_SecureRsa_KeyID
{
    CommomCrypto_SecureRsa_KeyId_0  = 0,
    CommomCrypto_SecureRsa_KeyId_1,
    CommomCrypto_SecureRsa_KeyId_2,
    CommomCrypto_SecureRsa_KeyId_3,
    CommomCrypto_SecureRsa_KeyId_4,
    CommomCrypto_SecureRsa_KeyId_5,
    CommomCrypto_SecureRsa_KeyId_eMax

} CommomCrypto_SecureRsa_KeyID;

typedef struct CommonCrypto_SecureRsa_InitSettings
{
    uint32_t        hashStorageDataSect;    /* the data section index which stores the hash of initial RSA certificates */
    uint32_t        otpKeyDataSect;         /* the data section index which stores the device key */
    uint32_t        rsaDataSize;            /* the size of the initial RSA certificate package file */
    uint8_t*        rsaDataAddress;
}CommonCrypto_SecureRsa_InitSettings;

/***************************************************************************
Summary:
Common Crypto secure rsa parameter settings.

Description:
Settings used for different key loads
***************************************************************************/
typedef struct CommonCrypto_SecureRsa_ParamSettings
{
    NEXUS_SecurityRsaKeySize    	rsaKeySize;    /* RSA private key size of length 1k or 2k bits */
    uint32_t    					rsaKeyIndex;   /* RSA private key slot index (0-4)*/
    NEXUS_SecurityRsa_PaddingType 	paddingType;   /* PKCS or OAEP padding of the encypted data (key) */
	CommomCrypto_SecureRsa_KeyType  encKeyType;
    uint8_t *						encBufAddress;
    uint32_t                        encBufSize; 
    bool                            pubKeyVerify;
    uint8_t *                       publicKeyAddress;
	uint8_t *                       PublicVkeyAddress;
    uint32_t                        PublicVkeySize;
	uint32_t                        PublicVkeyIndex;
    uint8_t *                       sigBufaddress;
    uint32_t                        sigBufSize;
    NEXUS_SecurityRsa_PaddingType   shaType;
    CommomCrypto_SecureRsa_KeyID    srcKeyId;
    CommomCrypto_SecureRsa_KeyID    DestKeyId;
	uint8_t *                       inputDataAddress;
	uint32_t                        inputDataSize;

	NEXUS_SecurityKeyLadderOp        keyLadderOp;

	uint32_t				iv[4];	        /* the IV needed for CBC mode for decryption */ 	
    struct CommonCrypto_SecureRsa_InitSettings InitSettings;

}CommonCrypto_SecureRsa_ParamSettings;

/***************************************************************************
Summary:
Get Common Crypto secure rsa module default settings.

Description:
This function is used to initialize a CommonCrypto secure rsa Settings structure with default settings.
This is required in order to make application code resilient to the addition of new strucutre members in the future.
***************************************************************************/
void CommonCrypto_SecureRsa_GetDefaultSettings(
   CommonCrypto_SecureRsa_ParamSettings *pSettings    /* [out] default settings */
    );


/***************************************************************************
Summary: Load hashLoacked RSA package 


Description:
This function is used to load the RSA Package as per the input settings. 

***************************************************************************/
NEXUS_Error CommonCrypto_SecureRSA_LoadHashLockedRsaPkg(
const CommonCrypto_SecureRsa_ParamSettings *pSettings);




/***************************************************************************
Summary: Load KPK


Description:
This function in unwraps the input key with RSA in teh specified slot and loads to the specified kpk slot. 

***************************************************************************/

NEXUS_Error CommonCrypto_SecureRSA_RsaDecryptLoadKpk(const CommonCrypto_SecureRsa_ParamSettings *pSettings);

/***************************************************************************
Summary: Load renewable RSA key


Description:
This function is unwraps the input key with the kpk in the specified slot and load the RSA key to the specified slot. 

***************************************************************************/
NEXUS_Error CommonCrypto_SecureRSA_LoadRenewableRSAKey(const CommonCrypto_SecureRsa_ParamSettings *pSettings);


/***************************************************************************
Summary: Load MKR after decryption with RSA


Description:
This function is unwraps the input key with RSA in specified slot and loads to the IKR
***************************************************************************/
NEXUS_Error CommonCrypto_SecureRsa_RsaDecryptLoadMkr(const CommonCrypto_SecureRsa_ParamSettings *pSettings);

/***************************************************************************
Summary: Load MKR after decrypting with MKR


Description:
This function unwraps the input key with MKR and reloads the MKR with the new key 

***************************************************************************/

NEXUS_Error CommonCrypto_SecureRsa_MkrDecryptLoadMkr(const CommonCrypto_SecureRsa_ParamSettings* pSettings);



/***************************************************************************
Summary:encrypt with RSA private key. hashing nad padding will be taken care by the application


Description:
This function encrypts the given data with RSA private key. hashing and padding of input data will be taken care by the application
***************************************************************************/

NEXUS_Error CommonCrypto_SecureRsa_RsaPrivKeyEncrypt(const CommonCrypto_SecureRsa_ParamSettings* pSettings, uint8_t*outBuf, uint16_t *outBufSz);


/***************************************************************************
Summary:route the aes key from MKR/IKR to the M2M keyslot


Description:
This function routes the content (aes) key from MKR/IKR to the M2M keyslot
***************************************************************************/
NEXUS_Error CommonCrypto_SecureRsa_RouteKey(NEXUS_SecurityAlgorithm algo, NEXUS_SecurityAlgorithmVariant algoVariant, NEXUS_KeySlotHandle *outKeySlot,CommomCrypto_SecureRsa_KeyID mkrId); 


NEXUS_Error CommonCrypto_SecureRsa_LoadIv( NEXUS_KeySlotHandle *hKeySlot, uint32_t *ivkeys, uint32_t keySz);
#ifdef __cplusplus
}
#endif

#endif /* #ifndef COMMON_CRYPTO_SECURERSA_H__ */

