/******************************************************************************
 *    (c)2010-2012 Broadcom Corporation
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
#ifndef DRM_NETFLIX_H_
#define DRM_NETFLIX_H_

#ifdef NETFLIX_SAGE_IMPL
#include "tl/drm_netflix_tl.h"
#else

#include "drm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Netflix Parameter Settings structure */
typedef struct DrmNetflixParamSettings_t
{
	DrmCommonInit_t drmCommonInit;
}DrmNetflixParamSettings_t;

/******************************************************************************
** FUNCTION:
**  DRM_Netflix_GetDefaultParamSettings
**
** DESCRIPTION:
**   Retrieve the default settings
**
** PARAMETERS:
** pNetflixParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_Netflix_GetDefaultParamSettings(
		DrmNetflixParamSettings_t *pNetflixParamSettings);


/******************************************************************************
** FUNCTION:
**  DRM_Netflix_SetParamSettings
**
** DESCRIPTION:
**   Set the parameter settings
**
** PARAMETERS:
** netflixParamSettings - settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_Netflix_SetParamSettings(
		DrmNetflixParamSettings_t netflixParamSettings);

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
********************************************************************************/
DrmRC DRM_Netflix_Initialize(char *key_file);

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Finalize
//
// DESCRIPTION:
//   Clean up the DRM_Netflix module
//
********************************************************************************/
DrmRC DRM_Netflix_Finalize(void);

/*
/// Retrieves the ESN.
/// @param[in/out]  pEsn  Pointer to the buffer where the ESN will be copied into.
/// @return Drm_Success if the operation is successful or an error.
*/
DrmRC DRM_Netflix_GetEsn(uint8_t* pEsn);

/*
/// Retrieves the KPE.
/// @param[in/out]  pKpe  Pointer to the buffer where the KPE will be copied into.
/// @return Drm_Success if the operation is successful or an error.
*/
DrmRC DRM_Netflix_GetKpe(uint8_t* pKpe);

/*
/// Retrieves the KPH.
/// @param[in/out]  pKph      Pointer to the buffer where the KPH will be copied into.
/// @return Drm_Success if the operation is successful or an error.
*/
DrmRC DRM_Netflix_GetKph(uint8_t* pKph);

/*
/// Decrypts the stream received in parameter.
/// @param[in/out]      pEnc        Pointer to the buffer of encrypted data
/// @param[in]          uiSize      Size of the data to decrypt.
/// @return Drm_Success if the operation is successful or an error.
*/
DrmRC DRM_Netflix_Decrypt(uint8_t* pBuf, 
                    uint32_t uiSize);

/*
/// Decrypts the stream received in parameter.
/// @param[in/out]      pEnc        Pointer to the buffer of encrypted data
/// @param[in]          uiSize      Size of the data to decrypt.
/// @return Drm_Success if the operation is successful or an error.
*/
DrmRC DRM_Netflix_Encrypt(uint8_t* pBuf, 
                    uint32_t uiSize);

#ifdef __cplusplus
}
#endif

#endif

#endif /*DRM_NETFLIX_H_*/
