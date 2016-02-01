/******************************************************************************
 *    (c)2014 Broadcom Corporation
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 *****************************************************************************/
#ifndef DRM_WIDEVINE_CENC_H_
#define DRM_WIDEVINE_CENC_H_

#include "drm_types.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct DRM_WidevineCenc_Settings
{
    char * key_file;
    DrmCommonInit_t drmCommonInit;
}DRM_WidevineCenc_Settings;


/******************************************************************************
** FUNCTION:
**  DRM_WidevineCenc_GetDefaultParamSettings
**
** DESCRIPTION:
**   Retrieve the default settings
**
** PARAMETERS:
** pWidevineParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_WidevineCenc_GetDefaultParamSettings(
        DRM_WidevineCenc_Settings *pWidevineCencSettings);


/**********************************************************
 * FUNCTION:
 *  DRM_WidevineCenc_Initialize
 *
 * DESCRIPTION:
 *   Fetches the encrypted keybox information and sets the
 *   decryption mode in either TS mode or regular mode
 *
 * RETURN:
 *     Drm_Success or other
***********************************************************/
DrmRC DRM_WidevineCenc_Initialize(
        DRM_WidevineCenc_Settings *pWidevineCencSettings);

/*
// Frees the key slots for widevine decryption
*/
void DRM_WidevineCenc_Finalize(void);

/*
/// Retrieves the Device ID.
/// @param[in/out]  pDevID  Pointer to the buffer where the DevID Data will be copied into.
/// @return Drm_Success if the operation is successful or an error.
*/
DrmRC DRM_WidevineCenc_GetDeviceId(uint8_t* pDevID);

/*
/// Retrieves the Root Key Data.
/// @param[in/out]  pKeyID      Pointer to the buffer where the Root Key Data will be copied into.
/// @return Drm_Success if the operation is successful or an error.
*/
DrmRC DRM_WidevineCenc_GetKeyData(uint8_t* pKeyID);

/*
 Retrieves the key field of the keybox, namely key data---16 bytes.
 pKey Pointer to the buffer where the key Data will be copied into.
 return Drm_Success if the operation is successful or an error.
*/
DrmRC DRM_WidevineCenc_GetKey(uint8_t* pKey);

/*
 Retrieves the rpk field of the keybox, namely key data---16 bytes.
 pRpk Pointer to the buffer where the rpk Data will be copied into.
 return Drm_Success if the operation is successful or an error.
*/
DrmRC DRM_WidevineCenc_GetRpk(uint8_t* pKey);


/*
 Retrieves magic data -- 4 bytes
 pMagicData Pointer to the buffer where the magic Data will be copied into.
 return Drm_Success if the operation is successful or an error.
*/
DrmRC DRM_WidevineCenc_GetMagicData(uint8_t* pMagicData);

/*
 Retrieves CRC data -- 4 bytes
 pCrcData Pointer to the buffer where the CRC Data will be copied into.
 return Drm_Success if the operation is successful or an error.
*/
DrmRC DRM_WidevineCenc_GetCrcData(uint8_t* pCrcData);

/* Widevine AESCMAC */
DrmRC DRM_WidevineCenc_AesCMAC(
            uint8_t *pWrappedKey,
            uint32_t wrappedKeySize,
            uint8_t *pCtxMac,
            uint32_t ctxMacSize,
            uint8_t  *pOut);

/* retrieves whole keybox*/
DrmRC DRM_WidevineCenc_GetKeyBox(uint8_t *pKeyBox);

/*
/// Decrypts the stream received in parameter.
/// @param[in]      pEnc        Pointer to the buffer of encrypted data
/// @param[in]          uiSize      Size of the data to decrypt.
/// @param[in/out]  pDec        Pointer to the buffer where the decrypted data will be copied into.
/// @param[in]          pIv         IV for CBC decryption
/// @param[in]      dest_type            Decryption type: in place or to destination.
/// @param[in]      keyslot         Use Odd/Even Key
/// @return Drm_Success if the operation is successful or an error.
*/
DrmRC DRM_WidevineCenc_Decrypt(uint8_t* pEnc,
                    uint32_t uiSize,
                    uint8_t* pDec,
                    uint8_t* pIv,
                    DrmDestinationType dest_type,
                    DrmSecurityKeyType keyslot_type);


/******************************************************************************
 FUNCTION:
  DRM_WidevienCenc_Encrypt

 DESCRIPTION:
   Encrypts buffer with the key and IV passed.

 PARAMETERS:
    pSrc[in] - pointer to source buffer.  Must be allocated with a call to
            NEXUS_Memory_Allocate AND it's length should be 16-byte aligned
    src_length[in] - length of the source buffer to encrypt.  Must be 16-byte aligned
    pDst[out] - pointer to destination buffer and must be allocated with a call to
            NEXUS_Memory_Allocate
    pKey[in] - pointer to a buffer containing the key to be used
    pIv[in] - if not used, should be 16-bytes of 0x00's

******************************************************************************/
DrmRC DRM_WidevienCenc_Encrypt(uint8_t *pSrc,
                                uint32_t src_length,
                                uint8_t *pDst,
                                uint8_t *pKey,
                                uint8_t *pIv);


/******************************************************************************
 FUNCTION:
  DRM_WidevienCenc_Decrypt

 DESCRIPTION:
   Decrypts buffer with the key and IV passed.

 PARAMETERS:
    pSrc[in] - pointer to source buffer.  Must be allocated with a call to
            NEXUS_Memory_Allocate AND it's length should be 16-byte aligned
    src_length[in] - length of the source buffer to encrypt.  Must be 16-byte aligned
    pDst[out] - pointer to destination buffer and must be allocated with a call to
            NEXUS_Memory_Allocate
    pKey[in] - pointer to a buffer containing the key to be used
    pIv[in] - if not used, should be 16-bytes of 0x00's

******************************************************************************/
DrmRC DRM_WidevienCenc_Decrypt(uint8_t *pSrc,
                                uint32_t src_length,
                                uint8_t *pDst,
                                uint8_t *pKey,
                                uint8_t *pIv);

#ifdef __cplusplus
}
#endif

#endif /*DRM_WIDEVINE_CENC_H_*/
