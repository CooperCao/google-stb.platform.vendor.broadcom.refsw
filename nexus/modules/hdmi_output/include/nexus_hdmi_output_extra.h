/******************************************************************************
  *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
  ******************************************************************************/
#ifndef NEXUS_HDMI_OUTPUT_EXTRA_H__
#define NEXUS_HDMI_OUTPUT_EXTRA_H__

#include "nexus_hdmi_output.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Lightly-used settings to configure the HDMI output interface
**/
typedef struct NEXUS_HdmiOutputExtraSettings
{
    /*
     * DEPRECATED. The dynamicRangeMode member in NEXUS_DisplaySettings will take precedence over
     * this setting, unless dynamicRangeMode is set to NEXUS_VideoDynamicRangeMode_eAuto
     * and overrideDynamicRangeMasteringInfoFrame below is set to true.
     *
     * Also, new dynamic range modes are only available via the use of the NEXUS_VideoDynamicRangeMode
     * enum member dynamicRangeMode in NEXUS_DisplaySettings. In addition, the metadata contained in
     * NEXUS_HdmiDynamicRangeMasteringInfoFrame should only be set for debug
     * purposes, as setting the metadata using the NEXUS_HdmiOutput_SetExtraSettings
     * call is not frame-synchronous.
     *
     * The following table explains how the deprecated and new APIs work together.
     *
     * "Forced" means that the output mode will be forced even if the TV doesn't support it.
     * "override" is set via the overrideDynamicRangeMasteringInfoFrame bool below (the DEPRECATED API).
     * NEXUS_VideoEotf is set via the NEXUS_HdmiDynamicRangeMasteringInfoFrame.eotf field below (the DEPRECATED API).
     * NEXUS_VideoDynamicRangeMode is set via the dynamicRangeMode field in NEXUS_DisplaySettings (the new API).
     * DRMIF - Yes means the DRMIF is transmitted; No means it is not.
     * "xxx" means that it doesn't matter what the given column is set to
     * "Auto" will try to select the best mode based on Broadcom usage rules
     *
     * Usage        DRMIF  Deprecated  Forced  override  NEXUS_VideoEotf            NEXUS_VideoDynamicRangeMode
     * ------------ ------ ----------- ------- --------- -------------------------- ---------------------------------------
     * Legacy SDR   No     Yes         Yes     true      NEXUS_VideoEotf_eInvalid   NEXUS_VideoDynamicRangeMode_eAuto
     * Legacy SDR   No     No          No      xxx       xxx                        NEXUS_VideoDynamicRangeMode_eLegacy
     * SDR          Yes    Yes         Yes     true      NEXUS_VideoEotf_eSdr       NEXUS_VideoDynamicRangeMode_eAuto
     * SDR          Yes    No          No      xxx       xxx                        NEXUS_VideoDynamicRangeMode_eSdr
     * HDR10/PQ     Yes    Yes         Yes     true      NEXUS_VideoEotf_eHdr10/Pq  NEXUS_VideoDynamicRangeMode_eAuto
     * HDR10/PQ     Yes    No          No      xxx       xxx                        NEXUS_VideoDynamicRangeMode_eHdr10
     * HLG          Yes    Yes         Yes     true      NEXUS_VideoEotf_eHlg       NEXUS_VideoDynamicRangeMode_eAuto
     * HLG          Yes    No          No      xxx       xxx                        NEXUS_VideoDynamicRangeMode_eHlg
     * Dolby Vision No     No          No      xxx       xxx                        NEXUS_VideoDynamicRangeMode_eDolbyVision
     * HDR10+       Yes    No          No      xxx       xxx                        NEXUS_VideoDynamicRangeMode_eHdr10Plus
     * Track Input  *      No          No      xxx       xxx                        NEXUS_VideoDynamicRangeMode_eTrackInput
     * Auto         *      No          No      false     xxx                        NEXUS_VideoDynamicRangeMode_eAuto
     *
     * See also: NEXUS_DisplaySettings.dynamicRangeMode
     */
    bool overrideDynamicRangeMasteringInfoFrame;
    NEXUS_HdmiDynamicRangeMasteringInfoFrame dynamicRangeMasteringInfoFrame;  /* Dynamic Range And Mastering InfoFrame (DRMIF) */
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

     struct {
        bool tx; /* true if the transmitter supports the given dynamic range mode, false otherwise */
        bool rx; /* true if the attached receiver supports the given dynamic range mode, false if not */
    } dynamicRangeModeSupported[NEXUS_VideoDynamicRangeMode_eMax];

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
