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
#ifndef NEXUS_HDDVI_INPUT_H__
#define NEXUS_HDDVI_INPUT_H__

#include "nexus_types.h"
#include "nexus_display_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Settings for HdDvi input
**/
typedef struct NEXUS_HdDviInputSettings {
    bool autoFormat;           /* If true, auto-detection is performed. 'format' is ignored. */
    NEXUS_VideoFormat format;  /* If autoFormat is false, 'format' will set the HD-DVI video format. */
    bool enableDe;
    unsigned inputDataMode; /* 24, 30 or 36 bit */
    struct {
        unsigned width;
        unsigned height;
    } formatDetectionTolerance;

    bool external;   /* Internal or external hddvi input */

    struct {
        bool enabled;
        uint8_t data[36];   /* chip-specific custom configuration */
    } custom;    

    struct {
        bool enabled;
        unsigned horizontal, vertical;
    } startPosition;    

    NEXUS_ColorSpace colorSpace;  /* RGB, YCrcb 422 or YCrcb 444 */
    NEXUS_ColorPrimaries colorPrimaries;  /* Color Primaries */
    NEXUS_MatrixCoefficients matrixCoef; /* Matrix coefficients. */
    NEXUS_TransferCharacteristics transferCharacteristics; /* Transfer characteristics */

} NEXUS_HdDviInputSettings;

/**
Summary:
Handle for the HD-DVI input interface
**/
typedef struct NEXUS_HdDviInput *NEXUS_HdDviInputHandle;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
**/
void NEXUS_HdDviInput_GetDefaultSettings(
    NEXUS_HdDviInputSettings *pSettings /* [out] */
    );

/**
Summary:
Open a new HD-DVI Input.

**/
NEXUS_HdDviInputHandle NEXUS_HdDviInput_Open( /* attr{destructor=NEXUS_HdDviInput_Close}  */
    unsigned index,
    const NEXUS_HdDviInputSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
Close an HD-DVI Input.
**/
void NEXUS_HdDviInput_Close(
    NEXUS_HdDviInputHandle hdDviInput
    );

/**
Summary:
This function is used to get current configuration
**/
void NEXUS_HdDviInput_GetSettings(
    NEXUS_HdDviInputHandle hdDviInput,
    NEXUS_HdDviInputSettings *pSettings /* [out] */
    );

/**
Summary:
This function is used to set new configuration
**/
NEXUS_Error NEXUS_HdDviInput_SetSettings(
    NEXUS_HdDviInputHandle hdDviInput,
    const NEXUS_HdDviInputSettings *pSettings
    );

/**
Summary:
Returns the abstract NEXUS_VideoInput connector for the video input.

Description:
Connect to a VideoWindow using NEXUS_VideoWindow_AddInput.
**/
NEXUS_VideoInputHandle NEXUS_HdDviInput_GetConnector(
    NEXUS_HdDviInputHandle hdDviInput
    );

/**
Summary:
Status returned by NEXUS_HdDviInput_GetStatus
**/
typedef struct NEXUS_HdDviInputStatus
{
    NEXUS_VideoFormat videoFormat;
    NEXUS_AspectRatio aspectRatio;
} NEXUS_HdDviInputStatus;

/**
Summary:
Get status
**/
NEXUS_Error NEXUS_HdDviInput_GetStatus(
    NEXUS_HdDviInputHandle hdDviInput,
    NEXUS_HdDviInputStatus *pStatus /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_HDDVI_INPUT_H__ */

