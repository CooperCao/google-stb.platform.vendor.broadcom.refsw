/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
#ifndef NEXUS_VIDEO_ADJUST_H__
#define NEXUS_VIDEO_ADJUST_H__

#include "nexus_types.h"
#include "nexus_display_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=*
The Video Adjustment interface allows manipulation of BVN-middle processing units.
**/

/**
Summary:
Get defaults
**/
void NEXUS_VideoWindow_GetDefaultDnrSettings(
    NEXUS_VideoWindowDnrSettings *pSettings
    );

/**
Summary:
Get current DNR settings
**/
void NEXUS_VideoWindow_GetDnrSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowDnrSettings *pSettings /* [out] */
    );

/**
Summary:
Set new DNR settings

Description:
This function maps to BVDC_Source_SetDnrConfiguration.
You can find additional documentation in bvdc.h for this function.
**/
NEXUS_Error NEXUS_VideoWindow_SetDnrSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowDnrSettings *pSettings
    );

/**
Summary:
Set new DNR settings along with custom data for DNR processing

Description:
The format of the additional data is private.
**/
NEXUS_Error NEXUS_VideoWindow_SetCustomDnrSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowDnrSettings *pSettings,
    const uint8_t *pCustomData,   /* attr{null_allowed=y;nelem=numEntries} additional data */
    unsigned numEntries
    );

/**
Summary:
Get defaults
**/
void NEXUS_VideoWindow_GetDefaultAnrSettings(
    NEXUS_VideoWindowAnrSettings *pSettings
    );

/**
Summary:
Get current ANR settings
**/
void NEXUS_VideoWindow_GetAnrSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowAnrSettings *pSettings /* [out] */
    );

/**
Summary:
Set new ANR settings

Description:
This function maps to BVDC_Source_SetAnrConfiguration.
You can find additional documentation in bvdc.h for this function.
**/
NEXUS_Error NEXUS_VideoWindow_SetAnrSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowAnrSettings *pSettings
    );

/**
Summary:
Set new ANR settings along with custom data for ANR processing

Description:
The format of the additional data is private.
**/
NEXUS_Error NEXUS_VideoWindow_SetCustomAnrSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowAnrSettings *pSettings,
    const uint8_t *pCustomData,   /* attr{null_allowed=y;nelem=numEntries} additional data */
    unsigned numEntries /* size of pCustomData in bytes */

    );

/**
Summary:
Get default MAD settings
**/
void NEXUS_VideoWindow_GetDefaultMadSettings(
    NEXUS_VideoWindowMadSettings *pSettings /* [out] */
    );

/**
Summary:
Get current MAD settings
**/
void NEXUS_VideoWindow_GetMadSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowMadSettings *pSettings /* [out] */
    );

/**
Summary:
Set new MAD settings

Description:
This function maps to BVDC_Window_SetDeinterlaceConfiguration.
You can find additional documentation in bvdc.h for this function.
**/
NEXUS_Error NEXUS_VideoWindow_SetMadSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowMadSettings *pSettings
    );

/**
Summary:
Get default scaler settings
**/
void NEXUS_VideoWindow_GetDefaultScalerSettings(
    NEXUS_VideoWindowScalerSettings *pSettings /* [out] */
    );

/**
Summary:
Get current scaler settings
**/
void NEXUS_VideoWindow_GetScalerSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowScalerSettings *pSettings /* [out] */
    );

/**
Summary:
Set new scaler settings

Description:
This function maps to BVDC_Window_SetScalerConfiguration.
You can find additional documentation in bvdc.h for this function.
**/
NEXUS_Error NEXUS_VideoWindow_SetScalerSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowScalerSettings *pSettings
    );

/**
Summary:
Set the window game mode buffer delay.

Description:
See BVDC_Window_GameModeSettings in bvdc.h for details.
**/
NEXUS_Error NEXUS_VideoWindow_SetGameModeDelay(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowGameModeDelay *pSettings
    );

/**
Summary:
Get the window game mode buffer delay settings.
**/
void NEXUS_VideoWindow_GetGameModeDelay(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowGameModeDelay *pSettings /* [out] */
    );

/***************************************************************************
Summary:
    Set indices to select filter coefficients from customizable internal tables

Description:
    This structure allows the user to select customized filter coefficients for scaler, hscaler or mad from the built-in
    or user replacable tables in bvdc_coeffs_priv.c.
    Each value in this structure is used as an index into those tables.
    The default for all indices is 0.

Used in NEXUS_VideoWindow_SetCoefficientIndexSettings.
***************************************************************************/
typedef struct NEXUS_VideoWindowCoefficientIndexSettings
{
    uint32_t sclVertLuma;   /* scaler vertical luma filter index */
    uint32_t sclHorzLuma;   /* scaler horizontal luma filter index */
    uint32_t sclVertChroma; /* scaler vertical chroma filter index */
    uint32_t sclHorzChroma; /* scaler horizontal chroma filter index */

    uint32_t hsclHorzLuma;  /* hscaler horizontal luma filter index */
    uint32_t hsclHorzChroma;/* hscaler horizontal chroma filter index */

    uint32_t madHorzLuma;   /* MAD horizontal luma filter index */
    uint32_t madHorzChroma; /* MAD horizontal chroma filter index */
} NEXUS_VideoWindowCoefficientIndexSettings;

/**
Summary:
Get current coefficient index settings.
**/
void NEXUS_VideoWindow_GetCoefficientIndexSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowCoefficientIndexSettings *pSettings /* [out] */
    );

/**
Summary:
Set new coefficient index settings.

Description:
This function maps to BVDC_Window_SetCoefficientIndex.
You can find additional documentation in bvdc.h for this function.
**/
NEXUS_Error NEXUS_VideoWindow_SetCoefficientIndexSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowCoefficientIndexSettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_VIDEO_ADJUST_H__ */

