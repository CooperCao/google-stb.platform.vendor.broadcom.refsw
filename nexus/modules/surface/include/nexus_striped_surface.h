/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
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
 **************************************************************************/
#ifndef NEXUS_STRIPED_SURFACE_H__
#define NEXUS_STRIPED_SURFACE_H__

#include "nexus_types.h"
#include "nexus_surface_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
A striped surface is a video plane which is obtained from the VideoDecoder.
Use NEXUS_Graphics2D_Destripe to convert this to a NEXUS_SurfaceHandle, which can be used in the graphics subsystem.
**/
typedef struct NEXUS_StripedSurface *NEXUS_StripedSurfaceHandle;

/**
Summary:
**/
void NEXUS_StripedSurface_GetDefaultCreateSettings(
    NEXUS_StripedSurfaceCreateSettings *pSettings /* [out] */
    );

/**
Summary:
**/
NEXUS_StripedSurfaceHandle NEXUS_StripedSurface_Create( /* attr{destructor=NEXUS_StripedSurface_Destroy}  */
    const NEXUS_StripedSurfaceCreateSettings *pSettings
    );

/**
Summary:
**/
void NEXUS_StripedSurface_Destroy(
    NEXUS_StripedSurfaceHandle stripedSurface
    );

/**
Summary:
Get settings for this striped surface. These are set at create time only.
**/
void NEXUS_StripedSurface_GetCreateSettings(
    NEXUS_StripedSurfaceHandle stripedSurface,
    NEXUS_StripedSurfaceCreateSettings *pSettings /* [out] */
    );

/**
Summary:
Information about a striped surface obtained by NEXUS_StripedSurface_GetStatus
**/
typedef struct NEXUS_StripedSurfaceStatus
{
    unsigned width;     /* same as NEXUS_StripedSurfaceCreateSettings.imageWidth */
    unsigned height;    /* same as NEXUS_StripedSurfaceCreateSettings.imageHeight */
} NEXUS_StripedSurfaceStatus;

/**
Summary:
DEPRECATED: Get information about the striped surface

Description:
Use NEXUS_StripedSurface_GetCreateSettings to get equivalent information.

A striped surface is the output of NEXUS_StillDecoder_GetStripedSurface.
Also see NEXUS_Graphics2D_Destripe.
This function is used if you want to know the dimensions of a striped surface before destriping it.
**/
NEXUS_Error NEXUS_StripedSurface_GetStatus(
    NEXUS_StripedSurfaceHandle stripedSurface,
    NEXUS_StripedSurfaceStatus *pStatus /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif
