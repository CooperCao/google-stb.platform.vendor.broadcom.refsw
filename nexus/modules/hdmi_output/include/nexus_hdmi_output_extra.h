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
#ifndef NEXUS_HDMI_OUTPUT_EXTRA_H__
#define NEXUS_HDMI_OUTPUT_EXTRA_H__

#include "nexus_hdmi_output.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Dolby Vision Output Mode
**/
typedef enum NEXUS_HdmiOutputDolbyVisionMode
{
    NEXUS_HdmiOutputDolbyVisionMode_eAuto, /* automatically select the best quality output mode.
        If the TV supports Dolby Vision, and all TV requirements are met, enable Dolby Vision,
        otherwise disable Dolby Vision */
    NEXUS_HdmiOutputDolbyVisionMode_eEnabled, /* enable Dolby Vision, even if it means changing formats */
    NEXUS_HdmiOutputDolbyVisionMode_eDisabled, /* disable Dolby Vision */
    NEXUS_HdmiOutputDolbyVisionMode_eMax
} NEXUS_HdmiOutputDolbyVisionMode;

/**
Summary:
HDMI output DolbyVision priority modes
**/
typedef enum NEXUS_HdmiOutputDolbyVisionPriorityMode
{
    NEXUS_HdmiOutputDolbyVisionPriorityMode_eAuto,
    NEXUS_HdmiOutputDolbyVisionPriorityMode_eVideo,
    NEXUS_HdmiOutputDolbyVisionPriorityMode_eGraphics,
    NEXUS_HdmiOutputDolbyVisionPriorityMode_eMax
} NEXUS_HdmiOutputDolbyVisionPriorityMode;

/**
Summary:
Lightly-used settings to configure the HDMI output interface
**/
typedef struct NEXUS_HdmiOutputExtraSettings
{
    bool overrideDynamicRangeMasteringInfoFrame; /* If true, contents of dynamicRangeMasteringInfoFrame are used to
        build the DRM InfoFrame, rather than the default value based on receiver preferences and capabilities, and
        stream information */
    NEXUS_HdmiDynamicRangeMasteringInfoFrame dynamicRangeMasteringInfoFrame;  /* Dynamic Range And Mastering InfoFrame */
    struct
    {
        NEXUS_HdmiOutputDolbyVisionMode outputMode; /* whether to enable Dolby Vision output or not */
        bool blendInIpt; /* If true, gfx will be blended in IPT color space; Default true to conform with Dolby Vision */
        NEXUS_HdmiOutputDolbyVisionPriorityMode priorityMode;
    } dolbyVision; /* API is provisional, subject to change */
} NEXUS_HdmiOutputExtraSettings;

/**
Summary:
Lightly-used status for the HDMI output interface
**/
typedef struct NEXUS_HdmiOutputExtraStatus
{
    struct
    {
        bool supported; /* is Dolby Vision output supported by attached receiver */
        bool enabled; /* is Dolby Vision output enabled */
    } dolbyVision; /* API is provisional, subject to change */

    unsigned phyChangeRequestCounter; /* for internal use, provisional, and subject to change*/
} NEXUS_HdmiOutputExtraStatus;

/**
Summary:
Get current lightly-used extra settings
**/
void NEXUS_HdmiOutput_GetExtraSettings(
    NEXUS_HdmiOutputHandle output,
    NEXUS_HdmiOutputExtraSettings *pSettings    /* [out] Settings */
    );

/**
Summary:
Apply new lightly-used extra settings
**/
NEXUS_Error NEXUS_HdmiOutput_SetExtraSettings(
    NEXUS_HdmiOutputHandle output,
    const NEXUS_HdmiOutputExtraSettings *pSettings
    );

/**
Summary:
Get current lightly-used extra status
**/
void NEXUS_HdmiOutput_GetExtraStatus(
    NEXUS_HdmiOutputHandle output,
    NEXUS_HdmiOutputExtraStatus *pStatus    /* [out] Status */
    );

/**
Summary:
Display attached receiver's EDID information
**/
void NEXUS_HdmiOutput_DisplayRxEdid(
    NEXUS_HdmiOutputHandle handle
    );

/**
Summary:
Display attached receiver's EDID information
**/
void NEXUS_HdmiOutput_DisplayRxInfo(
    NEXUS_HdmiOutputHandle handle
    );

/**
Summary:
Get BCM_VideoFmt from the nth Detail Timing block
**/
void NEXUS_HdmiOutput_GetVideoFormatFromDetailTiming(
    NEXUS_HdmiOutputHandle handle,
    uint8_t detailTimingNumber
    );

/**
Summary:
Modify the colorimetry indicate in the AVI Infoframe
**/
void NEXUS_HdmiOutput_SetAviInfoFrameColorimetry(
    NEXUS_HdmiOutputHandle handle,
    uint8_t colorimetry
    );

/**
Summary:
Modify the picture aspect ratio indicated in the AVI Infoframe
**/
void NEXUS_HdmiOutput_SetAviInfoFrameAspectRatio(
    NEXUS_HdmiOutputHandle handle,
    uint8_t aspectRatio
    );

/**
Summary:
Enable/Disable Pj Checking
**/
void NEXUS_HdmiOutput_EnablePjChecking(
    NEXUS_HdmiOutputHandle handle,
    bool enable
    );

/**
Summary:
Force video pixel to all 0s or 1s
**/
void NEXUS_HdmiOutput_ForceVideoPixel(
    NEXUS_HdmiOutputHandle handle,
    uint8_t pixelValue
    );


/**
Summary:
Enable packet transmission
**/
void NEXUS_HdmiOutput_EnablePacketTransmission(
    NEXUS_HdmiOutputHandle handle,
    uint8_t packetChoice
    );

/**
Summary:
Disable packet transmission
**/
void NEXUS_HdmiOutput_DisablePacketTransmission(
    NEXUS_HdmiOutputHandle handle,
    uint8_t packetChoice
    );

/**
Summary:
Reset the Scrambling configuration

Description:
Some Rx may report status that indicates the Rx is not in the expected
mode.  The most likely scenario is th Tx is scrambling and the Rx is not.
The following API will force the reset of the scrambling configuration to
get both Tx and Rx to match scrambling configurations
**/
NEXUS_Error NEXUS_HdmiOutput_ResetScrambling(
    NEXUS_HdmiOutputHandle handle
    );



#ifdef __cplusplus
}
#endif

#endif
