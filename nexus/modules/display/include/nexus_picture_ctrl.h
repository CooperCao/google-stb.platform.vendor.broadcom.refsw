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
#ifndef NEXUS_PICTURE_CTRL_H__
#define NEXUS_PICTURE_CTRL_H__

#include "nexus_types.h"
#include "nexus_display_types.h"
#include "nexus_picture_quality_types.h"

#ifdef __cplusplus
extern "C" {
#endif


/*=************************************************************************
The PictureCtrl interface manipulates the backend picture enhancement units.
in future Nexus releases.
**************************************************************************/

/***************************************************************************
Summary:
    Get common color settings
****************************************************************************/
void NEXUS_PictureCtrl_GetCommonSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_PictureCtrlCommonSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
    Set common color settings

Description:
    NEXUS_PictureCtrl_SetCommonSettings is used to do the basic picture
    control of contrast, saturation, hue, brightness, color temperature
    and sharpness.

    It sets the color space convertor (CSC) in the compositor (CMP).
    This function will override anything set by NEXUS_VideoWindow_SetColorMatrix.

See Also:
    NEXUS_VideoWindow_SetColorMatrix - more detailed control of the CSC block
    NEXUS_PictureCtrl_SetSharpnessValue - more detailed control of sharpness
****************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_SetCommonSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_PictureCtrlCommonSettings *pSettings
    );

/***************************************************************************
Summary:
    Get enhanced color settings
****************************************************************************/
void NEXUS_PictureCtrl_GetAdvColorSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_PictureCtrlAdvColorSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
    Set enhanced color settings

Description:
    NEXUS_PictureCtrl_SetAdvColorSettings is used to advanced picture
    control of dynamic contrast, flesh tone, green boost, blue boost
    and monochrome.

See Also:
    NEXUS_PictureCtrl_SetCmsSettings - overrides any fleshTone, greenBoost and blueBoost setting
    NEXUS_PictureCtrl_LoadCabTable - overrides any fleshTone, greenBoost and blueBoost setting
****************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_SetAdvColorSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_PictureCtrlAdvColorSettings *pSettings
    );

/***************************************************************************
Summary:
    Get CMS control settings
****************************************************************************/
void NEXUS_PictureCtrl_GetCmsSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_PictureCtrlCmsSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
    Set CMS control settings

Description:
    NEXUS_PictureCtrl_SetCmsSettings sets the saturation gain and hue gain
    for six basic colors(red/green/blue/cyan/magenta/yellow). It provides
    the capability to manage each color in the color bar independently.

    Note that NEXUS_PictureCtrl_SetCmsSettings is exclusive with the
    parameters of fleshTone, greenBoost and blueBoost in
    NEXUS_PictureCtrl_SetAdvColorSettings because both are based on CAB
    block and they are implemented through different algorithm internally.

    If the user turns on CMS by NEXUS_PictureCtrl_SetCmsSettings, it is not
    allowed to call NEXUS_PictureCtrl_SetAdvColorSettings to turn on
    flesh tone, green boost or blue boost before user turn off CMS.

    If the user turn on fleshTone, greenBoost or blueBoost by calling
    NEXUS_PictureCtrl_SetAdvColorSettings, it is not allowed to turn on
    CMS by NEXUS_PictureCtrl_SetCmsSettings before user turn off fleshTone,
    greenBoost and blueBoost.

See Also:
    NEXUS_PictureCtrl_SetAdvColorSettings
    NEXUS_PictureCtrl_LoadCabTable
****************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_SetCmsSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_PictureCtrlCmsSettings *pSettings
    );

/***************************************************************************
Summary:
    Get current window dither settings
****************************************************************************/
void NEXUS_PictureCtrl_GetDitherSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_PictureCtrlDitherSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
    Set new window dither settings
****************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_SetDitherSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_PictureCtrlDitherSettings *pSettings
    );

/***************************************************************************
Summary:
    Get contrast stretch parameters
****************************************************************************/
void NEXUS_PictureCtrl_GetContrastStretch(
    NEXUS_VideoWindowHandle window,
    NEXUS_PictureCtrlContrastStretch *pContrast /* [out] histogram data */
    );

/***************************************************************************
Summary:
    Set contrast stretch parameters

Description:
    This is an advanced usage of dynamic contrast feature. User can call this
    API to customize dynamic contrast config.

See Also:
    NEXUS_PictureCtrl_SetAdvColorSettings
    NEXUS_PictureCtrl_LoadLabTable
****************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_SetContrastStretch(
    NEXUS_VideoWindowHandle window,
    const NEXUS_PictureCtrlContrastStretch *pContrast /* attr{null_allowed=y} histogram data */
    );

/***************************************************************************
Summary:
    Set contrast stretch parameters along with custom data for LAB processing
****************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_SetCustomContrastStretch(
    NEXUS_VideoWindowHandle window,
    const NEXUS_PictureCtrlContrastStretch *pContrast,
    const int16_t *pTable,    /* attr{nelem=numTableEntries} custom LAB parameters */
    unsigned numTableEntries
    );

/***************************************************************************
Deprecated. Use NEXUS_PictureCtrlCommonSettings.sharpness instead.
****************************************************************************/
void NEXUS_PictureCtrl_GetSharpnessValue(
    NEXUS_VideoWindowHandle window,
    NEXUS_PictureCtrlSharpnessValue *pData /* [out] */
    );

/***************************************************************************
Deprecated. Use NEXUS_PictureCtrlCommonSettings.sharpness instead.
****************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_SetSharpnessValue(
    NEXUS_VideoWindowHandle window,
    const NEXUS_PictureCtrlSharpnessValue *pData
    );

/***************************************************************************
Summary:
    Load the CAB table

Description:
    Disable or enable CAB block by providing the CAB table pointer.
    A NULL pointer disables CAB. A partial table can be loaded by
    using the offset and size parameters.

    A few caveats follow:

    1. If partial tables are to be loaded, it is required that the entire
    table be loaded first because the uninitialized portions of the table
    cannot be filled in with initial values that are guaranteed to work
    correctly with the partially loaded table.
    2. When loading partial tables, the VDC cannot guarantee that it
    would be error free. It is the responsiblilty of the end user to
    assure its validity and operationability with the rest of the PEP block.
    3. The CAB table is persistent. If disabled, the table will not be
    erased. The window would have to be closed to reset the table.
    4. Once user table is loaded, user cannot select auto flesh, green
    boost and blue boost algorithm until user disable customized
    table by passing in the NULL pointer table.

See Also:
    This function overrides values set by NEXUS_PictureCtrl_SetAdvColorSettings or
    NEXUS_PictureCtrl_SetCmsSettings. See them for details of which features are overridden.
**************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_LoadCabTable(
    NEXUS_VideoWindowHandle window,
    const uint32_t *pTable,         /* attr{null_allowed=y;nelem=numTableEntries} table data */
    unsigned numTableEntries,       /* size of the CAB table, size 0 is used to disable custom table. */
    unsigned offset                 /* start offset in the table */
    );

/***************************************************************************
Summary:
    Load the LAB table

Description:
    NEXUS_PictureCtrl_LoadLabTable loads the raw LAB table into LAB block.
    It will override the dynamic contrast settings set through
    NEXUS_PictureCtrl_SetAdvColorSettings or NEXUS_PictureCtrl_SetContrastStretch.

See Also:
    This function overrides values set by NEXUS_PictureCtrl_SetAdvColorSettings or
    NEXUS_PictureCtrl_SetContrastStretch. See them for details of which features are overridden.
**************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_LoadLabTable(
    NEXUS_VideoWindowHandle window,
    const uint32_t *pTable,         /* attr{null_allowed=y;nelem=numTableEntries} table data */
    unsigned numTableEntries,       /* size of the LAB table, size 0 is used to disable custom table. */
    unsigned offset                 /* start offset in the table */
    );

/***************************************************************************
Summary:
    Set luma average calculation region for the display
**************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_ConfigDisplayLumaStatistics(
    NEXUS_DisplayHandle display,
    const NEXUS_ClipRect *pRect /* Luma average calculation region. */
    );

/***************************************************************************
Summary:
    Get luma average value of the display
**************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_GetDisplayLumaStatistics(
    NEXUS_DisplayHandle display,
    NEXUS_LumaStatistics *pLumaStat /* [out] */
    );


/***************************************************************************
Summary:
    Set luma average calculation region for the video window
**************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_ConfigWindowLumaStatistics(
    NEXUS_VideoWindowHandle window,
    const NEXUS_ClipRect *pRect /* Luma average calculation region. */
    );

/***************************************************************************
Summary:
    Get luma average value of video window's surface
**************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_GetWindowLumaStatistics(
    NEXUS_VideoWindowHandle window,
    NEXUS_LumaStatistics *pLumaStat /* [out] */
    );

/***************************************************************************
Summary:
    Load color correction table for white balance
**************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_SetColorCorrectionTable(
    NEXUS_DisplayHandle display,
    const NEXUS_PictureControlColorCorrectionSettings *pSettings
    );

/***************************************************************************
Summary:
    Load custom color correction table for white balance
**************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_SetCustomColorCorrectionTable(
    NEXUS_DisplayHandle display,
    const uint32_t *pTable,           /* attr{null_allowed=y;nelem=numTableEntries} */
    unsigned numTableEntries
    );

/***************************************************************************
Summary:
Get color clip settings
**************************************************************************/
void NEXUS_PictureCtrl_GetColorClipSettings(
    NEXUS_DisplayHandle display,
    NEXUS_PictureCtrlColorClipSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Set color clip settings
**************************************************************************/
NEXUS_Error NEXUS_PictureCtrl_SetColorClipSettings(
    NEXUS_DisplayHandle display,
    const NEXUS_PictureCtrlColorClipSettings *pSettings /* attr{null_allowed=y} */
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_PICTURE_CTRL_H__ */

