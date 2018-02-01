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
#ifndef DRM_COMMON_PRIV_H_
#define DRM_COMMON_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common_crypto.h"
#include "drm_types.h"

typedef struct DRM_Common_Settings_t
{
    CommonCryptoHandle cryptHnd;
    pid_t pid;
    pid_t tid;
    bool multiThread;
    DrmRC drmRc;
} DRM_Common_Settings, *DRM_Common_Handle;

/******************************************************************************
 ** FUNCTION
 **   DRM_Common_Initialize
 **
 ** DESCRIPTION:
 **   Calls 'DRM_Common_BasicInitialize' internally but also requires the path
 **   to a valid DRM bin file.
 **
 ** PARAMETERS:
 ** commonDrmInit - DrmCommonInit_t structure to pass on to DRM_Common_BasicInitialize
 ** key_file - path to valid DRM bin file
 **
 ** RETURNS:
 **   Drm_Success or other.
 **
 ******************************************************************************/
DrmRC DRM_Common_Initialize(
    DrmCommonInit_t *pCommonDrmSettings,
    char *key_file);

DrmRC DRM_Common_AcquireHandle(DRM_Common_Handle* drmHnd);
DrmRC DRM_Common_GetHandle(DRM_Common_Handle* drmHnd);
DrmRC DRM_Common_CloseHandle(void);

#ifdef __cplusplus
}
#endif
#endif /*DRM_COMMON_PRIV_H_*/
