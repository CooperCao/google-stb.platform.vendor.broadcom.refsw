/***************************************************************************
 *     (c)2007-2012 Broadcom Corporation
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
#ifndef NEXUS_VIDEO_HDMI_INPUT_H__
#define NEXUS_VIDEO_HDMI_INPUT_H__

#include "nexus_types.h"
#include "nexus_display_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
The "VideoHdmiInput" controls the HD-DVI input.
This is typically used for VDC-side control for HdmiInput.

NOTICE: this interface may be refactored under the name NEXUS_HdDvi.
**/

/**
Summary:
HD-DVI input data mode used in NEXUS_VideoHdmiInputSettings
**/
typedef enum NEXUS_HdDviDataMode
{
    NEXUS_HdDviDataMode_e24bit,
    NEXUS_HdDviDataMode_e30bit,
    NEXUS_HdDviDataMode_e36bit,
    NEXUS_HdDviDataMode_eMax
} NEXUS_HdDviDataMode;

/***************************************************************************
Summary:
Settings for VideoHdmiInput input
****************************************************************************/
typedef struct NEXUS_VideoHdmiInputSettings {
    bool autoFormat; /* If true, auto-detection is performed. 'format' is ignored. */
    NEXUS_VideoFormat format; /* If autoFormat is false, 'format' will set the HD-DVI video format. */

    bool autoPosition; /* If false, set the position of video using position.x and position.y, If true, position is set automatically. */
    struct {
        unsigned x,y; /* only applies if autoPosition = false. */
    } position;

    NEXUS_HdDviDataMode dataMode;
    bool deEnabled;
    bool externalInput; /* set to false for HdmiInput */
    struct
    {
        unsigned width;
        unsigned height;
    } formatTolerance;
} NEXUS_VideoHdmiInputSettings;


/**
Summary:
This function is used to get current configuration
**/
void NEXUS_VideoHdmiInput_GetSettings(
    NEXUS_VideoInputHandle hdmiInput, /* videoInput returned by the NEXUS_HdmiInput_GetVideoConnector function */
    NEXUS_VideoHdmiInputSettings *pSettings /* [out] */
    );

/**
Summary:
This function is used to set new configuration
**/
NEXUS_Error NEXUS_VideoHdmiInput_SetSettings(
    NEXUS_VideoInputHandle hdmiInput, /* videoInput returned by the NEXUS_HdmiInput_GetVideoConnector function */
    const NEXUS_VideoHdmiInputSettings *pSettings
    );


#ifdef __cplusplus
}
#endif

#endif /* NEXUS_VIDEO_HDMI_INPUT_H__ */

