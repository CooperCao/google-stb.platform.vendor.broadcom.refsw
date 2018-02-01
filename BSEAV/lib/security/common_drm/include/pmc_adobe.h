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
#ifndef PMC_ADOBE_H_
#define PMC_ADOBE_H_

#include "drm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PmcAdobeDataType_e
{
    PmcAdobeDataType_eNone          = 0,
    PmcAdobeDataType_eRootDigest    = 1,
    PmcAdobeDataType_eTransportCert = 2,
    PmcAdobeDataType_eDeviceCred    = 3,
    PmcAdobeDataType_eDeviceCredPwd = 4,
    PmcAdobeDataType_eSharedDom0    = 5,
    PmcAdobeDataType_eSharedDom0Pwd = 6,
    PmcAdobeDataType_eSharedDom1    = 7,
    PmcAdobeDataType_eSharedDom1Pwd = 8,
    PmcAdobeDataType_eSharedDom2    = 9,
    PmcAdobeDataType_eSharedDom2Pwd = 10
}PmcAdobeDataType_e;

/* Adobe Parameter Settings structure */
typedef struct PmcAdobeParamSettings_t
{
    DrmCommonInit_t drmCommonInit;
}PmcAdobeParamSettings_t;

/******************************************************************************
 ** FUNCTION:
 **   DRM_Pmc_AdobeGetDefaultParamSettings
 **
 ** DESCRIPTION:
 **   Retrieve the default settings
 **
 ** PARAMETERS:
 **   pPmcAdobeParamSettings - pointer to settings structure
 **
 ** RETURNS:
 **   void
 **
 ******************************************************************************/
void DRM_Pmc_AdobeGetDefaultParamSettings(
    PmcAdobeParamSettings_t *pPmcAdobeParamSettings);

/******************************************************************************
 ** FUNCTION:
 **   DRM_Pmc_AdobeSetParamSettings
 **
 ** DESCRIPTION:
 **   Set param settings
 **
 ** PARAMETERS:
 **   pmcAdobeParamSettings - settings structure
 **
 ** RETURNS:
 **   void
 **
 ******************************************************************************/
void DRM_Pmc_AdobeSetParamSettings(
    PmcAdobeParamSettings_t pmcAdobeParamSettings);

/******************************************************************************
 ** FUNCTION
 **   DRM_Pmc_AdobeInit
 **
 ** DESCRIPTION:
 **   Initializes the PMC Adobe module with the pmc.bin file and if specified the
 **   DRM bin file as well.  If the 'pmc_bin_filepath' is NULL the pmc_core module
 **   will look for /mnt/pmc.bin at runtime.
 **
 ** PARAMETERS:
 **   pmc_bin_filepath[in] - Pointer to the pmc.bin file path in the root filesystem
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_Pmc_AdobeInit(char *pmc_bin_filepath);

/******************************************************************************
 ** FUNCTION
 **   DRM_Pmc_AdobeUnInit
 **
 ** DESCRIPTION:
 **   Closes Adobe PMC module
 **
 ** PARAMETERS:
 **   N/A
 **
 ** RETURNS:
 **   N/A
 **
 ******************************************************************************/
void DRM_Pmc_AdobeUnInit(void);

/******************************************************************************
 ** FUNCTION
 **   DRM_Pmc_AdobeGetData
 **
 ** DESCRIPTION:
 **   Return the appropriate data to the caller.  The call to this function is
 **   made twice.  First, call the function passing NULL as the pointer argument:
 **   For example,
 **      DRM_Pmc_AdobeGetData(PmcAdobeDataType_eXXXXXXXXX, NULL, &size);
 **
 **   This will return the data size in which the user can allocate the
 **   appropriate sized buffer to call the API a 2nd time.
 **
 **      DRM_Pmc_AdobeGetData(PmcAdobeDataType_eXXXXXXXXX, buffer, &size);
 **
 **   ... which will copy the data.
 **
 ** PARAMETERS:
 **   dataType - Type of data to fetch
 **   pBuffer  - pointer to destination buffer to copy to
 **   dataSize - pointer to uint32_t to return data size
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful or an error.
 **
 ******************************************************************************/
DrmRC DRM_Pmc_AdobeGetData(PmcAdobeDataType_e dataType, uint8_t *pBuffer, uint32_t *dataSize);

#ifdef __cplusplus
}
#endif

#endif /* PMC_ADOBE_H_ */
