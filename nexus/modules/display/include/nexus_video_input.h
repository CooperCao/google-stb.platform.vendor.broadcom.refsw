/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_VIDEO_INPUT_H__
#define NEXUS_VIDEO_INPUT_H__

#include "nexus_types.h"
#include "nexus_display_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* unused */
typedef enum
{
    NEXUS_VideoInputCrcType_eCrc32,
    NEXUS_VideoInputCrcType_eChecksum,
    NEXUS_VideoInputCrcType_eCrc16
} NEXUS_VideoInputCrcType;

/**
Summary:
Settings which are generic to all video inputs
**/
typedef struct NEXUS_VideoInputSettings
{
    bool mute;                        /* Send the muteColor to the VideoWindow. Actual source data is discarded.
                                         This only applies to analog/hdmi sources. See NEXUS_VideoDecoderSettings.mute for digital. */
    NEXUS_Pixel muteColor;            /* Color of muted video in NEXUS_PixelFormat_eA8_R8_G8_B8 colorspace */
    bool repeat;                      /* Repeat last picture to the VideoWindow. This can be used for a continuous picture during channel change.
                                         This only applies to analog/hdmi sources. See NEXUS_VideoDecoder_ChannelChangeMode for digital.
                                         If mute==true, it will override repeat==true. */
    NEXUS_CallbackDesc sourceChanged; /* Called whenever anything in NEXUS_VideoInputStatus changes. */
    NEXUS_CallbackDesc sourcePending; /* Called whenever NEXUS_VideoInputStatus.sourcePending goes true and a NEXUS_VideoInput_SetResumeMode(videoInput, NEXUS_VideoInputResumeMode_eNow) is required.
                                         See NEXUS_VideoInput_SetResumeMode for more information about source pending and dynamic RTS. */
    struct {
        bool                    enable;
        NEXUS_VideoFormat       videoFormat;
        NEXUS_Video3DStructure  structure;
        NEXUS_Video3DSubSample  subSample;
        bool                    overrideOrientation; /*  force to override source orientation */
        NEXUS_VideoOrientation  orientation; /* orientation of the 3D source */
    } video3DSettings;                /* Settings for 3D sources. */

    NEXUS_VideoInputCrcType crcType;    /* unused */
    unsigned crcQueueSize;              /* if non-zero, CRC capture is enabled. use NEXUS_VideoInput_GetCrcData to retrieve data. */
} NEXUS_VideoInputSettings;

/**
Summary:
Get current settings
**/
void NEXUS_VideoInput_GetSettings(
    NEXUS_VideoInputHandle input,
    NEXUS_VideoInputSettings *pSettings    /* [out] */
    );

/**
Summary:
Apply new settings
**/
NEXUS_Error NEXUS_VideoInput_SetSettings(
    NEXUS_VideoInputHandle input,
    const NEXUS_VideoInputSettings *pSettings
    );

/**
Summary:
Status which is generic to all video inputs
**/
typedef struct NEXUS_VideoInputStatus
{
    bool videoPresent; /* If true, then active video is detected at the source.
                          If false, the other members of NEXUS_VideoInputStatus will be based on the previous active format. */
    unsigned width, height;
    NEXUS_AspectRatio aspectRatio;
    NEXUS_VideoFormat format;           /* You can determine if a source is progressive using NEXUS_VideoFormat_GetInfo(format). */
    NEXUS_VideoFrameRate frameRate;     /* This is an enumerated frame rate code */
    unsigned refreshRate;               /* This is a numeric refresh rate (aka frame rate) value in 1/1000 Hz. e.g. 29.97 = 29970. */
    bool sourcePending; /* Set to true if Nexus is in NEXUS_VideoInputResumeMode_eManual mode and is currently waiting for an eNow trigger.
                           When this is changed to true, you will receive a NEXUS_VideoInputSettings.sourcePending callback.
                           This will not be set to false until NEXUS_VideoInput_SetResumeMode(videoInput, NEXUS_VideoInputResumeMode_eNow) is called. */
} NEXUS_VideoInputStatus;

/**
Summary:
Get current status
**/
NEXUS_Error NEXUS_VideoInput_GetStatus(
    NEXUS_VideoInputHandle input,
    NEXUS_VideoInputStatus *pStatus    /* [out] */
    );

/**
Summary:
The application must call NEXUS_VideoInput_Shutdown after the NEXUS_VideoInput
has been disconnected from all video windows.
This is needed to free internally cached data.
**/
void NEXUS_VideoInput_Shutdown(
    NEXUS_VideoInputHandle input
    );

/**
Summary:
Get current color space convertor matrix
**/
void NEXUS_VideoInput_GetColorMatrix(
    NEXUS_VideoInputHandle input,
    NEXUS_ColorMatrix *pColorMatrix /* [out] */
    );

/**
Summary:
Set new color space convertor matrix

Description:
This only works for analog and HDMI inputs.
It does not work for VideoDecoder inputs.
**/
NEXUS_Error NEXUS_VideoInput_SetColorMatrix(
    NEXUS_VideoInputHandle input,
    const NEXUS_ColorMatrix *pColorMatrix
    );

/**
Deprecated
**/
typedef enum
{
    NEXUS_VideoInputResumeMode_eAuto,
    NEXUS_VideoInputResumeMode_eManual,
    NEXUS_VideoInputResumeMode_eNow,
    NEXUS_VideoInputResumeMode_eFreezeRts
} NEXUS_VideoInputResumeMode;

/* deprecated */
#define NEXUS_VideoInput_SetResumeMode(videoInput,resumeMode) NEXUS_NOT_SUPPORTED
#define NEXUS_VideoInput_ForcePending(videoInput) NEXUS_NOT_SUPPORTED

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_VIDEO_INPUT_H__ */
