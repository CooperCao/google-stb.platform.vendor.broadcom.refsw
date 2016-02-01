/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef RSA_TL_H__
#define RSA_TL_H__

#include "bstd.h"
#include "bkni.h"
#include "bsagelib_crypto_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This handle is used to store the context of a RsaTl instance */
typedef struct RsaTl_P_Instance *RsaTl_Handle;

typedef struct {
    char drm_binfile_path[256]; /* which is "./drm.bin" by default */
}RsaSettings;

/***************************************************************************
Summary:
Get default settings for loading the RSA module on SAGE

Description:
Retrieve the set of default values used to load the RSA module on SAGE

See Also:
RsaTl_Init()
***************************************************************************/
void
RsaTl_GetDefaultSettings(RsaSettings *pRsaModuleSettings);

/***************************************************************************
Summary:
Initialize an instance of the RSA module on SAGE

Description:
Use the settings structure to pass any initialization values to the RSA
module on SAGE

See Also:
RsaTl_Uninit()
***************************************************************************/
BERR_Code
RsaTl_Init(RsaTl_Handle *pRsaTlHandle,
           RsaSettings *pRsaModuleSettings);

/***************************************************************************
Summary:
Uninitialize the given instance of the RSA module on SAGE

See Also:

***************************************************************************/
void
RsaTl_Uninit(RsaTl_Handle hRsaTl);

/***************************************************************************
Summary:
Perform an RSA sign or verify opertion on SAGE

Description:


***************************************************************************/
BERR_Code
RsaTl_SignVerify(RsaTl_Handle hRsaTl,
                 uint32_t commandId,
                 uint8_t *pSrcData,
                 uint32_t srcLength,
                 uint32_t rsaKeyIndex,
                 BSAGElib_Crypto_ShaVariant_e shaVariant,
                 uint8_t *pSignature,
                 uint32_t signatureLength);

/***************************************************************************
Summary:
Perform an RSA encrypt or decrypt operation on SAGE

Description:


***************************************************************************/
BERR_Code
RsaTl_EncryptDecrypt(RsaTl_Handle hRsaTl,
                     uint32_t commandId,
                     uint8_t *pSrcData,
                     uint32_t srcLength,
                     uint8_t *pDstData,
                     uint32_t *pDstLength,
                     uint32_t padding,
                     uint32_t rsaKeyIndex);


/***************************************************************************
Summary:
Fetch the public key parameters for a given index

See Also:

***************************************************************************/
BERR_Code
RsaTl_GetPublicKey(RsaTl_Handle hRsaTl,
                   uint32_t rsaKeyIndex,
                               uint8_t *pModulus,
                               uint32_t *pModulusLength,
                               uint8_t *pPublicExponent);

#ifdef __cplusplus
}
#endif

#endif /*RSA_TL_H__*/
