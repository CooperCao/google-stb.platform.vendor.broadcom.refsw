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
#ifndef DRM_NETFLIX_H_
#define DRM_NETFLIX_H_

#ifdef NETFLIX_SAGE_IMPL
#include "tl/drm_netflix_tl.h"
#else

#include "drm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Netflix parameter settings structure */
typedef struct DrmNetflixParamSettings_t
{
    DrmCommonInit_t drmCommonInit;
}DrmNetflixParamSettings_t;

/******************************************************************************
 ** FUNCTION:
 **   DRM_Netflix_GetDefaultParamSettings
 **
 ** DESCRIPTION:
 **   Retrieve the default settings
 **
 ** PARAMETERS:
 **   pNetflixParamSettings - pointer to settings structure
 **
 ** RETURNS:
 **   void
 **
 ******************************************************************************/
void DRM_Netflix_GetDefaultParamSettings(
    DrmNetflixParamSettings_t *pNetflixParamSettings);

/******************************************************************************
 ** FUNCTION:
 **   DRM_Netflix_SetParamSettings
 **
 ** DESCRIPTION:
 **   Set the parameter settings
 **
 ** PARAMETERS:
 **   netflixParamSettings - settings structure
 **
 ** RETURNS:
 **   void
 **
 ******************************************************************************/
void DRM_Netflix_SetParamSettings(
    DrmNetflixParamSettings_t netflixParamSettings);

/******************************************************************************
 ** FUNCTION:
 **   DRM_Netflix_Initialize
 **
 ** DESCRIPTION:
 **   Reads the bin file specified and pre-loads the confidential info
 **   Must be called only once prior to any other module API call.
 **
 ** PARAMETERS:
 **   key_file - filepath of the netflix credential file
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful
 **
 ******************************************************************************/
DrmRC DRM_Netflix_Initialize(char *key_file);

/******************************************************************************
 ** FUNCTION:
 **   DRM_Netflix_Finalize
 **
 ** DESCRIPTION:
 **   Close the Netflix DRM module
 **
 ** PARAMETERS:
 **   N/A
 **
 ** RETURNS:
 **   N/A
 **
 ******************************************************************************/
DrmRC DRM_Netflix_Finalize(void);

/******************************************************************************
 ** FUNCTION:
 **   DRM_Netflix_GetEsn
 **
 ** DESCRIPTION:
 **   Extract ESN from Netflix credential
 **
 ** PARAMETERS:
 **   pEsn [in/out] - Pointer to the buffer where the ESN will be copied into.
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_Netflix_GetEsn(uint8_t* pEsn);

/******************************************************************************
 ** FUNCTION:
 **   DRM_Netflix_GetKpe
 **
 ** DESCRIPTION:
 **   Extract KPE from Netflix credential
 **
 ** PARAMETERS:
 **   pKpe [in/out] - Pointer to the buffer where the Kpe will be copied into.
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_Netflix_GetKpe(uint8_t* pKpe);

/******************************************************************************
 ** FUNCTION:
 **   DRM_Netflix_GetKph
 **
 ** DESCRIPTION:
 **   Extract KPH from Netflix credential
 **
 ** PARAMETERS:
 **   pKph [in/out] - Pointer to the buffer where the Kph will be copied into.
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_Netflix_GetKph(uint8_t* pKph);

/******************************************************************************
 ** FUNCTION:
 **   DRM_Netflix_Decrypt
 **
 ** DESCRIPTION:
 **   Decrypts the stream received in parameter
 **
 ** PARAMETERS:
 **   pBuf [in/out] - Pointer to the buffer of encrypted data.  Used to return decrypted data.
 **   uiSize [in]   - Size of the data to decrypt
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_Netflix_Decrypt(uint8_t* pBuf,
                          uint32_t uiSize);

/******************************************************************************
 ** FUNCTION:
 **   DRM_Netflix_Encrypt
 **
 ** DESCRIPTION:
 **   Encrypts the stream received in parameter
 **
 ** PARAMETERS:
 **   pBuf [in/out] - Pointer to the buffer of data.  Used to return encrypted data.
 **   uiSize [in]   - Size of the data to encrypt
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_Netflix_Encrypt(uint8_t* pBuf,
                          uint32_t uiSize);

#ifdef __cplusplus
}
#endif

#endif

#endif /*DRM_NETFLIX_H_*/
