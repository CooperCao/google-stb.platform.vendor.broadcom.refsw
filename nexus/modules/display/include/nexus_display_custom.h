/***************************************************************************
 *     (c)2007-2011 Broadcom Corporation
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
#ifndef NEXUS_DISPLAY_CUSTOM_H__
#define NEXUS_DISPLAY_CUSTOM_H__

#include "nexus_display.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Custom display format used in NEXUS_Display_SetCustomFormatSettings
**/
typedef struct NEXUS_DisplayCustomFormatSettings
{
    unsigned width;          /* in pixels */
    unsigned height;         /* in pixels */
    unsigned refreshRate;    /* in 1/1000 Hz. for example, 23.976Hz is 23976, 59.94Hz is 59940. */
    bool interlaced;
    NEXUS_DisplayAspectRatio aspectRatio;
    struct {
        unsigned x, y;
    } sampleAspectRatio;     /* Valid if aspectRatio is NEXUS_DisplayAspectRatio_eSar */
    bool dropFrameAllowed;   /* if false, refreshRate will be used exactly. if true, 1001 slip is allowed when tracking to source frame rate. */
} NEXUS_DisplayCustomFormatSettings;

/**
Summary:
Get default values for NEXUS_DisplayCustomFormatSettings
**/
void NEXUS_Display_GetDefaultCustomFormatSettings(
    NEXUS_DisplayCustomFormatSettings *pSettings /* [out] */
    );

/**
Summary:
Set a custom display format along with its settings

Description:
NEXUS_VideoFormat_eCustom0 and NEXUS_VideoFormat_eCustom1 are customizable, but only by replacing magnum/commonuntils/fmt/bfmt_custom.c.
They are not customizable at runtime using this function. Only NEXUS_VideoFormat_eCustom2 is customizable using this function.

On set-top boxes, this is used for transcoder display configuration.

The DTV version of this function is different.
**/
NEXUS_Error NEXUS_Display_SetCustomFormatSettings(
    NEXUS_DisplayHandle display,
    NEXUS_VideoFormat format,
    const NEXUS_DisplayCustomFormatSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_DISPLAY_CUSTOM_H__ */
