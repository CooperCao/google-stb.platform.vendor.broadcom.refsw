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
#ifndef NEXUS_DISPLAY_IMPL_H__
#define NEXUS_DISPLAY_IMPL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*=*
Priv including file for nexus_display.c
**/

/* VideoAdj private context */
typedef struct NEXUS_VideoAdjContext
{
    NEXUS_VideoWindowDnrSettings    stDnrSettings;
    NEXUS_VideoWindowMadSettings    stMadSettings;
    NEXUS_VideoWindowScalerSettings stSclSettings;
    NEXUS_VideoWindowAnrSettings    stAnrSettings;
    NEXUS_VideoWindowGameModeDelay  stGameModeDelaySettings;
    NEXUS_VideoWindowCoefficientIndexSettings coefficientIndexSettings;

    /* Flags to identify settings that were set before a VDC window was created */
    bool    bDnrSet;
    bool    bMadSet;
    bool    bSclSet;
    bool    bAnrSet;
    bool    bGameModeDelaySet;
    bool    bCoefficientIndexSettingsSet;

    /* we copy custom DNR and ANR data into BKNI_Malloc'd memory */
    void *customDnrData;
    void *customAnrData;
} NEXUS_VideoAdjContext;


/* PictureCtrl private context */
typedef struct NEXUS_PictureCtrlContext
{
    /* color settings */
    NEXUS_PictureCtrlCommonSettings  stCommonSettings;
    /* advanced color settings */
    NEXUS_PictureCtrlAdvColorSettings   stAdvColorSettings;
    /* Customized settings */
    NEXUS_PictureCtrlContrastStretch stCustomContrast;
    /* Color management settings */
    NEXUS_PictureCtrlCmsSettings stCmsSettings;
    /* Dither settings */
    NEXUS_PictureCtrlDitherSettings stDitherSettings;

    /* Flags to identify settings that were set before a VDC window was created */
    bool    bCommonSet;
    bool    bAdvColorSet;
    bool    bCustomContrastSet;
    bool    bCmsSet;
    bool    bDitherSet;

    /* we copy custom contrast stretch data into BKNI_Malloc'd memory */
    void *customContrastStretchData;
} NEXUS_PictureCtrlContext;

NEXUS_Error NEXUS_PictureCtrl_P_ApplySetSettings( NEXUS_VideoWindowHandle window );
NEXUS_Error NEXUS_VideoAdj_P_ApplySetSettings( NEXUS_VideoWindowHandle window );
NEXUS_Error NEXUS_P_Display_GetMagnumVideoFormatInfo_isr(NEXUS_DisplayHandle display, NEXUS_VideoFormat videoFormat, BFMT_VideoInfo *pInfo);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_DISPLAY_IMPL_H__ */

