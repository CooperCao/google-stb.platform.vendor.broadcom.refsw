/***************************************************************************
 *     (c)2011-2012 Broadcom Corporation
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
#ifndef NEXUS_SURFACE_CURSOR_H__
#define NEXUS_SURFACE_CURSOR_H__

#include "nexus_surface_compositor.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
surface compositor cursor;
**/
typedef struct NEXUS_SurfaceCursor *NEXUS_SurfaceCursorHandle;

/**
Summary:
Static configuration of NEXUS_SurfaceCursor
**/
typedef struct NEXUS_SurfaceCursorCreateSettings {
    struct {
        NEXUS_SurfaceRegion maxSize; /* maximum cursor size, in framebuffer pixels */
    } display[NEXUS_MAX_DISPLAYS]; 
    NEXUS_SurfaceHandle surface; 
} NEXUS_SurfaceCursorCreateSettings;

/**
Summary:
Dynamic configuration of NEXUS_SurfaceCursor
**/
typedef struct NEXUS_SurfaceCursorSettings {
    NEXUS_SurfaceComposition composition;
} NEXUS_SurfaceCursorSettings;

/**
Summary:
**/
void NEXUS_SurfaceCursor_GetDefaultCreateSettings(
    NEXUS_SurfaceCursorCreateSettings *createSettings
   );

/**
Summary:
**/
NEXUS_SurfaceCursorHandle NEXUS_SurfaceCursor_Create( /* attr{destructor=NEXUS_SurfaceCursor_Destroy} */
    NEXUS_SurfaceCompositorHandle surfaceCompositor,
    const NEXUS_SurfaceCursorCreateSettings *createSettings 
   );

/**
Summary:
**/
void NEXUS_SurfaceCursor_Destroy(
    NEXUS_SurfaceCursorHandle  cursor
    );

/**
Summary:
**/
void NEXUS_SurfaceCursor_GetSettings (
    NEXUS_SurfaceCursorHandle  cursor,
    NEXUS_SurfaceCursorSettings *settings
    );

/**
Summary:
**/
NEXUS_Error NEXUS_SurfaceCursor_SetSettings (
    NEXUS_SurfaceCursorHandle  cursor,
    const NEXUS_SurfaceCursorSettings *settings
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_SURFACE_CURSOR_H__ */

