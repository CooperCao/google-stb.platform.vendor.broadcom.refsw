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
#ifndef DRM_KEY_BINDING_H_
#define DRM_KEY_BINDING_H_

#include "drm_types.h"

typedef struct drm_key_binding_t
{
    uint8_t proc_in1[16];
    uint8_t proc_in2[16];
    uint8_t devIdA[8];
    uint8_t devIdB[8];
}drm_key_binding_t;

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyBinding_Init
 **
 ** DESCRIPTION:
 **   Currently unused since there are no initialization requirements for drm_key_binding
 **
 ** RETURNS:
 **   N/A
 **
 ******************************************************************************/
void DRM_KeyBinding_Init(void);

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyBinding_UnInit
 **
 ** DESCRIPTION:
 **   Currently unused since there are no closure requirements for drm_key_binding
 **
 ** RETURNS:
 **   N/A
 **
 ******************************************************************************/
void DRM_KeyBinding_UnInit(void);

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyBinding_FetchDeviceIds
 **
 ** DESCRIPTION:
 **   Retrieve the OTP IDs
 **
 ** PARAMETERS:
 **   pStruct [out] - Pointer to struct to contain the OTP IDs
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 **
 ******************************************************************************/
DrmRC DRM_KeyBinding_FetchDeviceIds(
    drm_key_binding_t *pStruct);

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyBinding_GetProcsFromOtp
 **
 ** DESCRIPTION:
 **   Read Proc_in1 and Proc_in2 values from OTP region
 **
 ** PARAMETERS:
 **   pStruct [out] - Pointer to struct to contain the proc_in values
 **
 ** RETURNS:
 **   Success -- Drm_Success
 **   Failure -- Other
 **
 ******************************************************************************/
DrmRC DRM_KeyBinding_GetProcsFromOtp(
    drm_key_binding_t *pStruct);

/******************************************************************************
 ** FUNCTION:
 **   DRM_KeyBinding_GenerateProcsFromOtp
 **
 ** DESCRIPTION:
 **   Generate Proc_in1 and Proc_in2 from OTP values
 **
 ** PARAMETERS:
 **   pStruct [in/out] - Pointer to struct containing the OTP ID, etc.
 **   pProcIn1FromBinHeader [in] - Pointer to proc_in1 input data
 **   pProcIn2FromBinHeader [in] - Pointer to proc_in2 input data
 **
 ** RETURNS:
 **   Success -- Drm_Success (Proc values generated)
 **   Failure -- Other
 **
 ******************************************************************************/
DrmRC DRM_KeyBinding_GenerateProcsFromOtp(
    drm_key_binding_t *pStruct,
    uint8_t *pProcIn1FromBinHeader,
    uint8_t *pProcIn2FromBinHeader);

#endif /*DRM_KEY_BINDING_H_*/
