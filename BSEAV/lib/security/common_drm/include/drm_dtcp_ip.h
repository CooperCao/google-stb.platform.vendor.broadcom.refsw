/******************************************************************************
 *    (c)2010-2015 Broadcom Corporation
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

#ifndef DRM_DTCP_IP_H_
#define DRM_DTCP_IP_H_

#include "drm_types.h"
#include "drm_metadata.h"
#include "nexus_security_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DTCP-IP Parameter Settings structure */
typedef struct DrmDtcpIpParamSettings_t
{
    DrmCommonInit_t drmCommonInit;
    NEXUS_KeySlotHandle encryptKeyHandle;
    NEXUS_KeySlotHandle decryptKeyHandle;
    bool use_external_keys;
}DrmDtcpIpParamSettings_t;


/******************************************************************************
** FUNCTION:
**  DRM_DtcpIp_GetDefaultParamSettings
**
** DESCRIPTION:
**   Retrieve the default settings
**
** PARAMETERS:
** pDtcpIpParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_DtcpIp_GetDefaultParamSettings(
        DrmDtcpIpParamSettings_t *pDtcpIpParamSettings);

/******************************************************************************
** FUNCTION:
**  DRM_DtcpIp_SetParamSettings
**
** DESCRIPTION:
**   Retrieve the default settings
**
** PARAMETERS:
** dtcpIpParamSettings - settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_DtcpIp_SetParamSettings(
        DrmDtcpIpParamSettings_t dtcpIpParamSettings);

/******************************************************************************
 FUNCTION:
  DRM_DtcpIp_Initialize

 DESCRIPTION:
   Must be called only once prior to any other module API call.

 PARAMETERS:
    N/A

******************************************************************************/
DrmRC DRM_DtcpIp_Initialize(char *key_file);


/******************************************************************************
 FUNCTION:
  DRM_DtcpIp_Finalize

 DESCRIPTION:
   Must be called only once prior to any other module API call.

 PARAMETERS:
    N/A

******************************************************************************/
void DRM_DtcpIp_Finalize(void);


/******************************************************************************
 FUNCTION:
  DRM_DtcpIp_GetData

 DESCRIPTION:
   Retrieves the confidential data from the DRM key region

 PARAMETERS:
 pDtcpIpData[out]  structure containing dtcp-ip data from DRM key region

RETURNS:
    Drm_Success or other (error)

******************************************************************************/
DrmRC DRM_DtcpIp_GetData(drm_dtcp_ip_data_t *pDtcpIpData);


/******************************************************************************
 FUNCTION:
  DRM_DtcpIp_Encrypt

 DESCRIPTION:
   Encrypts buffer with the key and IV (optional) passed.

 PARAMETERS:
    pSrc[in] - pointer to source buffer.  Must be allocated with a call to
            NEXUS_Memory_Allocate AND it's length should be 16-byte aligned
    src_length[in] - length of the source buffer to encrypt.  Must be 16-byte aligned
    pDst[out] - pointer to destination buffer and must be allocated with a call to
            NEXUS_Memory_Allocate
    pKey[in] - pointer to a buffer containing the key to be used
    pIv[in] - optional, should be all 0x00's if not used. Poiter to the IV used
            for AES-CBC operations

******************************************************************************/
DrmRC DRM_DtcpIp_Encrypt(
        uint8_t *pSrc,
        uint32_t src_length,
        uint8_t *pDst,
        uint8_t *pKey,
        uint8_t *pIv
        );


/******************************************************************************
 FUNCTION:
  DRM_DtcpIp_Decrypt

 DESCRIPTION:
   Decrypts buffer with the key and IV (optional) passed.

 PARAMETERS:
    pSrc[in] - pointer to source buffer.  Must be allocated with a call to
            NEXUS_Memory_Allocate AND it's length should be 16-byte aligned
    src_length[in] - length of the source buffer to encrypt.  Must be 16-byte aligned
    pDst[out] - pointer to destination buffer and must be allocated with a call to
            NEXUS_Memory_Allocate
    pKey[in] - pointer to a buffer containing the key to be used
    pIv[in] - optional, should be all 0x00's if not used. Poiter to the IV used
            for AES-CBC operations

******************************************************************************/
DrmRC DRM_DtcpIp_Decrypt(
        uint8_t *pSrc,
        uint32_t src_length,
        uint8_t *pDst,
        uint8_t *pKey,
        uint8_t *pIv
        );


#ifdef __cplusplus
}
#endif

#endif /* DRM_DTCP_IP_H_ */
