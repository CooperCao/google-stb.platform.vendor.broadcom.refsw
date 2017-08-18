/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
#ifndef NEXUS_DISPLAY_UTILS_H__
#define NEXUS_DISPLAY_UTILS_H__

#include "nexus_types.h"
#include "nexus_display_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NEXUS_GraphicsCompression
{
    NEXUS_GraphicsCompression_eNone,
    NEXUS_GraphicsCompression_eAllowed,
    NEXUS_GraphicsCompression_eRequired,
    NEXUS_GraphicsCompression_eMax
} NEXUS_GraphicsCompression;

/**
Summary:
display module capabilities
**/
typedef struct NEXUS_DisplayCapabilities
{
    unsigned numDisplays; /* Deprecated. The display list is not packed, so you can't assume 0..numDisplays-1 can be opened.
                             Instead, check display[].numVideoWindows > 0. */
    struct {
        unsigned numVideoWindows; /* if 0, display is not usable */
        struct {
            unsigned width, height; /* if 0, graphics is not usable */
            NEXUS_GraphicsCompression compression;
        } graphics; /* max capability */
        struct {
            unsigned maxWidthPercentage, maxHeightPercentage;
        } window[NEXUS_MAX_VIDEO_WINDOWS];
    } display[NEXUS_MAX_DISPLAYS];
    bool displayFormatSupported[NEXUS_VideoFormat_eMax]; /* is NEXUS_DisplaySettings.format supported by any display in the system? */
    unsigned numLetterBoxDetect; /* see NEXUS_VideoWindowSettings.letterBoxDetect */
} NEXUS_DisplayCapabilities;

/**
Summary:
get display module capabilities
**/
void NEXUS_GetDisplayCapabilities(
    NEXUS_DisplayCapabilities *pCapabilities /* [out] */
    );

typedef struct NEXUS_DisplayMaxMosaicCoverage
{
    unsigned maxCoverage; /* percentage of screen that can be covered with mosaic windows */
} NEXUS_DisplayMaxMosaicCoverage;

NEXUS_Error NEXUS_Display_GetMaxMosaicCoverage(
    unsigned displayIndex, /* for now, only displayIndex 0 is supported */
    unsigned numMosaics,
    NEXUS_DisplayMaxMosaicCoverage *pCoverage
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_DISPLAY_UTILS_H__ */
