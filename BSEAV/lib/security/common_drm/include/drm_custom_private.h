/******************************************************************************
 *    (c)2012 Broadcom Corporation
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
 * $brcm_Log: $
 * 
 *****************************************************************************/
#ifndef DRM_CUSTOM_PRIVATE_H_
#define DRM_CUSTOM_PRIVATE_H_

#include "drm_types.h"
#include "drm_metadata.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DrmCustomPrivateDataType_e
{
	DrmCustomPrivateDataType_eSharedCert_Id1 = 1,
	DrmCustomPrivateDataType_eSharedCert_Id2 = 2,
	DrmCustomPrivateDataType_eSharedCert_Id3 = 3,
	DrmCustomPrivateDataType_eSharedKey_Id1 = 4,
	DrmCustomPrivateDataType_eSharedKey_Id2 = 5,
	DrmCustomPrivateDataType_eSharedKey_Id3 = 6,
	DrmCustomPrivateDataType_eUniqueCert_Id1 = 7,
	DrmCustomPrivateDataType_eUniqueCert_Id2 = 8,
	DrmCustomPrivateDataType_eUniqueCert_Id3 = 9,
	DrmCustomPrivateDataType_eUniqueKey_Id1 = 10,
	DrmCustomPrivateDataType_eUniqueKey_Id2 = 11,
	DrmCustomPrivateDataType_eUniqueKey_Id3 = 12,
	DrmCustomPrivateDataType_eMax
}DrmCustomPrivateDataType_e;

/* CustomPrivate Parameter Settings structure */
typedef struct DrmCustomPrivateParamSettings_t
{
	DrmCommonInit_t drmCommonInit;
}DrmCustomPrivateParamSettings_t;

/******************************************************************************
** FUNCTION:
**  DRM_CustomPrivate_GetDefaultParamSettings
**
** DESCRIPTION:
**   Retrieve the default settings
**
** PARAMETERS:
** pCustomPrivateParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_CustomPrivate_GetDefaultParamSettings(
		DrmCustomPrivateParamSettings_t *pCustomPrivateParamSettings);

/******************************************************************************
** FUNCTION:
**  DRM_CustomPrivate_SetParamSettings
**
** DESCRIPTION:
**   Set param settings
**
** PARAMETERS:
** customPrivateParamSettings - settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_CustomPrivate_SetParamSettings(
		DrmCustomPrivateParamSettings_t customPrivateParamSettings);


/******************************************************************************
** FUNCTION
**   DRM_CustomPrivate_Init
**
** DESCRIPTION:
**   Initializes the Custom Private module
**
** PARAMETERS:
**   drm_bin_filepath[in] - Pointer to the DRM bin file path in the root filesystem
**
** RETURNS:
**   Drm_Success when the operation is successful or an error.
**
******************************************************************************/
DrmRC DRM_CustomPrivate_Init(char *drm_bin_filepath);


/******************************************************************************
** FUNCTION
**   DRM_CustomPrivate_UnInit
**
** DESCRIPTION:
**   Cleans up the module
**
** PARAMETERS:
**
** RETURNS:
**      N/A
**
******************************************************************************/
void DRM_CustomPrivate_UnInit(void);


/******************************************************************************
** FUNCTION
**   DRM_CustomPrivate_GetData
**
** DESCRIPTION:
**   Return the appropriate data to the caller.  The call to this function is
**   made twice.   Frist call the funcion passing NULL as the pointer argument:
**   For example,
**   	DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eXXXXXXXXX, NULL, &size);
**
**   This will return the data size in which the user can allocate the
**   appropriate sized buffer to call the API a 2nd time.
** PARAMETERS:
**   dataType - Type of data to fetch
**   pBuf - Pointer to the buffer where consumer key will be copied into.
**   *dataSize - pointer to uint32_t to return data size
**
** RETURNS:
**   Drm_Success when the operation is successful or an error.
**
******************************************************************************/
DrmRC 
DRM_CustomPrivate_GetData(
		DrmCustomPrivateDataType_e dataType,
		uint8_t *pBuffer,
		uint32_t *dataSize
		);

#ifdef __cplusplus
}
#endif

#endif /* DRM_CUSTOM_PRIVATE_H_ */
