/******************************************************************************
 *    (c)2015 Broadcom Corporation
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
 *
 *****************************************************************************/

#ifndef DRM_EDRM_TL_H_
#define DRM_EDRM_TL_H_

#include "drm_types.h"
#include "nexus_security_datatypes.h"
#include "nexus_dma.h"
#include "nexus_security.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EDRM_PUBLIC_KEY_SIZE                             256  /*!< size of public key */
#define EDRM_CERT_SIZE                                  2048

typedef struct DRM_DrmEdrmTl_P_Context_t *DRM_EdrmTlHandle;

/******************************************************************************
 FUNCTION:
  DRM_EdrmTl_Initialize

 DESCRIPTION:
   Must be called only once prior to any other module API call.

 PARAMETERS:
    N/A

******************************************************************************/
DrmRC DRM_EdrmTl_Initialize(char *key_file, DRM_EdrmTlHandle *hEdrmTl);


/******************************************************************************
 FUNCTION:
  DRM_EdrmTl_Finalize

 DESCRIPTION:
   Must be called only once after all other module API calls completed.

 PARAMETERS:
    N/A

******************************************************************************/
void DRM_EdrmTl_Finalize(DRM_EdrmTlHandle hEdrmTl);


/******************************************************************************
 FUNCTION:
  DRM_EdrmTl_GetDeviceCertificate

 DESCRIPTION:
   Get device certificate from drm bin.

 PARAMETERS:
    cert: buffer to copy certificate. shall be pre-allocated.
    certLength: cert buffer size. size shall be enough to hold certificate.

******************************************************************************/
DrmRC DRM_EdrmTl_GetDeviceCertificate(
    DRM_EdrmTlHandle hEdrmTl,
    uint8_t *cert,
    uint32_t certLength
    );


/******************************************************************************
 FUNCTION:
  DRM_EdrmTl_GetDeviceCertificate

 DESCRIPTION:
   Verify public key in certificate is same with key in edrm data section.

 PARAMETERS:
    result: set to 1 if public key is valid.

******************************************************************************/
DrmRC DRM_EdrmTl_VerifyPublicKey(
    DRM_EdrmTlHandle hEdrmTl,
    uint32_t *result
    );

#ifdef __cplusplus
}
#endif

#endif /* DRM_EDRM_TL_H_ */
