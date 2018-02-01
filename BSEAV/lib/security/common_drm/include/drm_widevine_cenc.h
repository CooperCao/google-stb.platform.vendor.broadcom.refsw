/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
**   DRM_WidevineCenc_GetDefaultParamSettings
**
** DESCRIPTION:
**   Retrieve the default settings
**
** PARAMETERS:
**   pWidevineParamSettings - pointer to settings structure
**
** RETURNS:
**   void
**
******************************************************************************/
void DRM_WidevineCenc_GetDefaultParamSettings(
    DRM_WidevineCenc_Settings *pWidevineCencSettings);

/******************************************************************************
 ** FUNCTION:
 **   DRM_WidevineCenc_Initialize
 **
 ** DESCRIPTION:
 **   Reads the bin file specified and loads the credential info
 **
 ** PARAMETERS:
 **   pWidevineCencSettings - pointer to settings structure
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful
 **
 ******************************************************************************/
DrmRC DRM_WidevineCenc_Initialize(
    DRM_WidevineCenc_Settings *pWidevineCencSettings);

/******************************************************************************
 ** FUNCTION:
 **   DRM_WidevineCenc_Finalize
 **
 ** DESCRIPTION:
 **   Close the WidevineCenc DRM module
 **
 ** PARAMETERS:
 **   void
 **
 ** RETURNS:
 **   void
 **
 ******************************************************************************/
void DRM_WidevineCenc_Finalize(void);

/******************************************************************************
 ** FUNCTION
 **   DRM_WidevineCenc_AesCMAC
 **
 ** DESCRIPTION:
 **   Do an AES CBC/CMAC crypto operation on a buffer (VOD mode only)
 **
 ** PARAMETERS:
 **   pWrappedKey - Wrapped key
 **   wrappedKeySize - Size of Wrapped key
 **   pInputBuf   - Input
 **   inputSize   - Length of input
 **   pOutBuf     - output
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_WidevineCenc_AesCMAC(
    uint8_t *pWrappedKey,
    uint32_t wrappedKeySize,
    uint8_t *pInputBuf,
    uint32_t inputSize,
    uint8_t  *pOutBuf);

/******************************************************************************
 ** FUNCTION
 **   DRM_WidevineCenc_GetKeyBox
 **
 ** DESCRIPTION:
 **   Retrieves whole keybox. Otherwise, it returns null.
 **
 ** PARAMETERS:
 **   pKeybox[out] - Pointer to keybox
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_WidevineCenc_GetKeyBox(uint8_t *pKeyBox);

/******************************************************************************
 ** FUNCTION
 **   DRM_WidevineCenc_GetDeviceId
 **
 ** DESCRIPTION:
 **   Extract 32-byte Device ID from WidevineCenc credential
 **
 ** PARAMETERS:
 **   pDevID [in/out] - Pointer to the buffer where the DevID Data will be copied into
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_WidevineCenc_GetDeviceId(uint8_t* pDevID);

/******************************************************************************
 ** FUNCTION
 **   DRM_Widevine_GetDeviceId
 **
 ** DESCRIPTION:
 **   Extract 32-byte Device ID from Widevine L3 credential
 **
 ** PARAMETERS:
 **   pDevID [in/out] - Pointer to the buffer where the DevID Data will be copied into
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_Widevine_GetDeviceId(uint8_t* pDevID);

/******************************************************************************
 ** FUNCTION
 **   DRM_WidevineCenc_GetKey
 **
 ** DESCRIPTION:
 **   Extract 16-byte key from WidevineCenc credential
 **
 ** PARAMETERS:
 **   pKey [in/out] - Pointer to the buffer where key data will be copied into
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_WidevineCenc_GetKey(uint8_t* pKey);

/******************************************************************************
 ** FUNCTION
 **   DRM_WidevineCenc_GetRpk
 **
 ** DESCRIPTION:
 **   Extract rpk field of the keybox from WidevineCenc credential
 **
 ** PARAMETERS:
 **   pKey [in/out] - Pointer to the buffer where rpk data will be copied into
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_WidevineCenc_GetRpk(uint8_t* pKey);

/******************************************************************************
 ** FUNCTION
 **   DRM_WidevineCenc_GetKeyData
 **
 ** DESCRIPTION:
 **   Extract the 72-byte ID field of the keybox from WidevineCenc credential
 **
 ** PARAMETERS:
 **   pId [in/out] - Pointer to the buffer where keydata data will be copied into
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_WidevineCenc_GetKeyData(uint8_t* pId);

/******************************************************************************
 ** FUNCTION
 **   DRM_WidevineCenc_GetMagicData
 **
 ** DESCRIPTION:
 **   Extract the 4-byte magic data of the keybox from WidevineCenc credential
 **
 ** PARAMETERS:
 **   pMagicData [in/out] - Pointer to the buffer where magic data will be copied into
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_WidevineCenc_GetMagicData(uint8_t* pMagicData);

/******************************************************************************
 ** FUNCTION
 **   DRM_WidevineCenc_GetCrcData
 **
 ** DESCRIPTION:
 **   Extract the 4-byte CRC data of the keybox from WidevineCenc credential
 **
 ** PARAMETERS:
 **   pCrcData [in/out] - Pointer to the buffer where CRC data will be copied into
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_WidevineCenc_GetCrcData(uint8_t* pCrcData);

/******************************************************************************
 ** FUNCTION
 **   DRM_WidevineCenc_Encrypt
 **
 ** DESCRIPTION:
 **   Encrypts buffer with the key and IV passed.
 **
 ** PARAMETERS:
 **   pSrc[in] - Pointer to source buffer.  Must be allocated with a call to
 **              NEXUS_Memory_Allocate AND it's length should be 16-byte aligned.
 **   src_length[in] - Length of the source buffer to encrypt.  Must be 16-byte aligned.
 **   pDst[out] - Pointer to destination buffer and must be allocated with
 **               a call to NEXUS_Memory_Allocate
 **   pKey[in]  - Pointer to a buffer containing the key to be used
 **   pIv[in]   - Optional, should be all 0x00's if not used. Pointer to the
 **               IV used for AES-CBC operations.
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_WidevineCenc_Encrypt(
    uint8_t *pSrc,
    uint32_t src_length,
    uint8_t *pDst,
    uint8_t *pKey,
    uint8_t *pIv);

/******************************************************************************
 ** FUNCTION
 **   DRM_WidevineCenc_Decrypt
 **
 ** DESCRIPTION:
 **   Decrypts buffer with the key and IV passed.
 **
 ** PARAMETERS:
 **   pSrc[in] - Pointer to source buffer.  Must be allocated with a call to
 **              NEXUS_Memory_Allocate AND it's length should be 16-byte aligned.
 **   src_length[in] - Length of the source buffer to decrypt.  Must be 16-byte aligned.
 **   pDst[out] - Pointer to destination buffer and must be allocated with
 **               a call to NEXUS_Memory_Allocate
 **   pKey[in]  - Pointer to a buffer containing the key to be used
 **   pIv[in]   - Optional, should be all 0x00's if not used. Pointer to the
 **               IV used for AES-CBC operations.
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error
 **
 ******************************************************************************/
DrmRC DRM_WidevineCenc_Decrypt(
    uint8_t *pSrc,
    uint32_t src_length,
    uint8_t *pDst,
    uint8_t *pKey,
    uint8_t *pIv);

#ifdef __cplusplus
}
#endif

#endif /*DRM_WIDEVINE_CENC_H_*/
