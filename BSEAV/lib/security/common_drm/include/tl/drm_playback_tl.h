/***************************************************************************
 *     (c)2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 *
 * Module Description: Thin layer Playback implementation
 *
 * Revision History:
 *
 **************************************************************************/
#ifndef _DRM_PLAYBACK_TL_H_
#define _DRM_PLAYBACK_TL_H_

#include "drm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DrmPlaybackContext_t *DrmPlaybackHandle_t;


/*******************************************************************************
// FUNCTION:
//  DRM_Playback_Initialize
//
// DESCRIPTION:
//   This function initialize the playback monitoring module
//   MUST BE CALLED PRIOR TO USING ANY OTHER API IN THE MODULE.
//
// @param[out] playbackHandle  Returned module handle
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Playback_Initialize(DrmPlaybackHandle_t  *playbackHandle);

/*******************************************************************************
// FUNCTION:
//  DRM_Playback_Finalize
//
// DESCRIPTION:
//   Clean up the Playback monitoring module
//
// @param[in] pHandle Module handle
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Playback_Finalize(DrmPlaybackHandle_t pHandle);

/*******************************************************************************
// FUNCTION:
//  DRM_Playback_Stop
//
// DESCRIPTION:
//   Issue a stop command to the monitoring module
//
// @param[in] pHandle Module handle
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Playback_Stop(DrmPlaybackHandle_t pHandle);

#ifdef __cplusplus
}
#endif

#endif /*_DRM_PLAYBACK_TL_H_*/
