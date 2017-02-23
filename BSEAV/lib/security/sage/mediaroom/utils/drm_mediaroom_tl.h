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
 *
 ******************************************************************************/
#ifndef DRM_MEDIAROOM_H_
#define DRM_MEDIAROOM_H_

#include "drm_types.h"
#include "nexus_security_datatypes.h"
#include "nexus_dma.h"
#include "nexus_security.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEDIAROOM_MAGIC_SIZE                                    4
#define MEDIAROOM_VERSION_SIZE                                  4
#define MEDIAROOM_PUBLIC_KEY_SIZE                             256  /*!< size of public key */
#define MEDIAROOM_CERT_SIZE                                  2048


#define BSAGE_PLATFORM_ID_MEDIAROOM 0x1300
#define Mediaroom_ModuleId_eDRM 0x01

/* Chipset type */
typedef enum
{
    ChipType_eZS = 0,
    ChipType_eZB,
    ChipType_eCustomer,
    ChipType_eCustomer1
} ChipType_e;

#define GET_UINT32_FROM_BUF(pBuf) \
    (((uint32_t)(((uint8_t*)(pBuf))[0]) << 24) | \
     ((uint32_t)(((uint8_t*)(pBuf))[1]) << 16) | \
     ((uint32_t)(((uint8_t*)(pBuf))[2]) << 8)  | \
     ((uint8_t *)(pBuf))[3])


/******************************************************************************
* FUNCTION:
*  DRM_Mediaroom_Initialize
*
* DESCRIPTION:
*   Must be called only once prior to any other module API call.
*
* PARAMETERS:
*   char *drm_bin_file
*
******************************************************************************/
DrmRC DRM_Mediaroom_Initialize(char *drm_bin_file);


/******************************************************************************
* FUNCTION:
* DRM_Mediaroom_Uninit
*
* DESCRIPTION:
*   Must be called only once after all other module API calls completed.
*
* PARAMETERS:
*   N/A
*
******************************************************************************/
void DRM_Mediaroom_Uninit();


/******************************************************************************
* FUNCTION:
* DRM_Mediaroom_GetMagic
*
* DESCRIPTION:
*  Get Magic field from drm bin.
*
* PARAMETERS:
*    magic: buffer to copy Magic field. shall be pre-allocated.
*    magicLength: magic buffer size. size shall be enough to hold Magic field.
*
******************************************************************************/
DrmRC DRM_Mediaroom_GetMagic(
    uint8_t *magic,
    uint32_t magicLength
    );


/******************************************************************************
* FUNCTION:
*  DRM_Mediaroom_VerifyMagic
*
* DESCRIPTION:
*  Verify the Magic field in Mediaroom data section.
*
* PARAMETERS:
*    magic: pointer to buffer containing the magic field
*
* RETURNS:
*    1 if good 0 otherwise
*
******************************************************************************/
uint32_t DRM_Mediaroom_VerifyMagic(
    uint8_t *magic
    );


/******************************************************************************
* FUNCTION:
* DRM_Mediaroom_GetVersion
*
* DESCRIPTION:
*  Get Version field from drm bin.
*
* PARAMETERS:
*    version: buffer to copy Version field. shall be pre-allocated.
*    versionLength: version buffer size. size shall be enough to hold Version field.
*
******************************************************************************/
DrmRC DRM_Mediaroom_GetVersion(
    uint8_t *version,
    uint32_t versionLength
    );


/******************************************************************************
* FUNCTION:
*  DRM_Mediaroom_VerifyVersion
*
* DESCRIPTION:
*  Verify the Version field in Mediaroom data section.
*
* PARAMETERS:
*    version: pointer to buffer containing the version field
*
* RETURNS:
*    1 if good 0 otherwise
*
******************************************************************************/
uint32_t DRM_Mediaroom_VerifyVersion(
    uint8_t *version
    );


/******************************************************************************
* FUNCTION:
* DRM_Mediaroom_GetCertificate
*
* DESCRIPTION:
*  Get AV or Non-AV certificate from drm bin.
*
* PARAMETERS:
*    cert: buffer to copy certificate. shall be pre-allocated.
*    certLength: cert buffer size. size shall be enough to hold certificate.
*    fAV: set to true to get AV certificate, set to false to get Non-AV certificate
*
******************************************************************************/
DrmRC DRM_Mediaroom_GetCertificate(
    uint8_t *cert,
    uint32_t certLength,
    bool fAV
    );


/******************************************************************************
* FUNCTION:
*  DRM_Mediaroom_VerifyPb
*
* DESCRIPTION:
*  Verify AV or Non-AV Pb in certificate is the same as AV or Non-AV Pb in Mediaroom data section.
*
* PARAMETERS:
*    result: set to 1 if Pb is valid.
*    fAV: set to true to verify AV Pb, set to false to verify Non-AV Pb
*
******************************************************************************/
DrmRC DRM_Mediaroom_VerifyPb(
    uint32_t *result,
    bool fAV
    );


/******************************************************************************
 * FUNCTION
 *   DRM_Mediaroom_GetChipType
 *
 * DESCRIPTION:
 *    Get the Chip Type
 *
 ******************************************************************************/
ChipType_e DRM_Mediaroom_GetChipType();

#ifdef __cplusplus
}
#endif

#endif /* DRM_MEDIAROOM_H_ */
