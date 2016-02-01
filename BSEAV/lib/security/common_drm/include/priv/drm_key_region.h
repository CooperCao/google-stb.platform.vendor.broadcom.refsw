/******************************************************************************
 *    (c)2010-2011 Broadcom Corporation
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
 * $brcm_Workfile: drm_key_region.h $
 * $brcm_Revision: 2 $
 * $brcm_Date: 4/24/12 4:21p $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *
 *****************************************************************************/
#ifndef DRM_KEY_REGION_H_
#define DRM_KEY_REGION_H_

#include "drm_types.h"
#include "common_crypto.h"
#include "drm_metadata.h"

typedef enum DrmTypeFlash{
    DrmTypeFlash_eCustKeySelect = 0,
    DrmTypeFlash_eKeyVarLow,
    DrmTypeFlash_eKeyVarHigh,
    DrmTypeFlash_eProcIn1Data,
    DrmTypeFlash_eProcIn2Data,
    DrmTypeFlash_eProcIn3Data,
    DrmTypeFlash_eMax
}DrmTypeFlash;


DrmRC
DRM_KeyRegion_Init(void);


DrmRC
DRM_KeyRegion_UnInit(void);


DrmRC
DRM_KeyRegion_Read(
    uint8_t*    pdata,
    uint32_t    offset,
    uint32_t    nbytes);

DrmRC
DRM_KeyRegion_SetCustDrmFilePath(
    char* custom_drm_file_path);

/******************************************************************************
** FUNCTION
**   DRM_KeyRegion_GetKeyData
**
** DESCRIPTION:
**    Fetch a DRM module's specific DRM data.
**
** PARAMETERS:
**   drm_type - Enum from the 'drm_types_e' (i.e. DRM_NETFLIX)
**   pBuf	  - the caller should cast the DRM specific data structure
**   			(found in drm_metadata.h) as a (uint8_t *)&struct and pass it
**
** RETURNS:
**   Drm_Success when the operation is successful or an error.
**
******************************************************************************/
DrmRC
DRM_KeyRegion_GetKeyData(drm_types_e drm_type, uint8_t *pBuf);

/******************************************************************************
** FUNCTION
**   DRM_KeyRegion_ReadKey2Info
**
** DESCRIPTION:
**    Read flash key2 information
**
** PARAMETERS:
**   pKi - Pointer to sturcture containing the key data.
**
** RETURNS:
**   Drm_Success when the operation is successful or an error.
**
******************************************************************************/
DrmRC DRM_KeyRegion_ReadKey2Info(
		CommonCryptoKeyLadderSettings *pKi);


/**********************************************************************************************
DRM_KeyRegion_SetKeyProvisionType

Set the provisioning type for way in which the keys will be fetched.

Input Parameters:
    provisioningType
        Type: DrmKeyProvisioningType
        Purpose: Set to either (DrmKeyProvisioningType_eUtv or DrmKeyProvisioningType_eBrcm)

Returns:
    SUCCESS: Drm_Success
    FAILURE: other
**********************************************************************************************/
DrmRC DRM_KeyRegion_SetKeyProvisionType(DrmKeyProvisioningType provisioningType);



/**********************************************************************************************
DRM_KeyRegion_GetKeyProvisionType

Return the provisioning type

Input Parameters:
    *provisioningType
        Type: DrmKeyProvisioningType
        Purpose: Will be set to either (DrmKeyProvisioningType_eUtv or DrmKeyProvisioningType_eBrcm)

Returns:
    SUCCESS: Drm_Success
    FAILURE: other
**********************************************************************************************/
DrmRC DRM_KeyRegion_GetKeyProvisionType(DrmKeyProvisioningType *provisioningType);

#endif /*DRM_KEY_REGION_H_*/
