/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#ifndef PLATFORM_TYPES_H__
#define PLATFORM_TYPES_H__ 1

#include <stdbool.h>

#define MAX_STREAMS 4

typedef struct Platform * PlatformHandle;
typedef struct PlatformPicture * PlatformPictureHandle;
typedef struct PlatformGraphics * PlatformGraphicsHandle;
typedef struct PlatformMediaPlayer * PlatformMediaPlayerHandle;
typedef struct PlatformDisplay * PlatformDisplayHandle;
typedef struct PlatformHdmiReceiver * PlatformHdmiReceiverHandle;
typedef struct PlatformInput * PlatformInputHandle;
typedef struct PlatformScheduler * PlatformSchedulerHandle;
typedef struct PlatformListener * PlatformListenerHandle;
typedef void (*PlatformCallback)(void * context, int param);

typedef enum PlatformUsageMode
{
    PlatformUsageMode_eFullScreenVideo,
    PlatformUsageMode_ePictureInGraphics,
    PlatformUsageMode_eMosaic,
    PlatformUsageMode_eMainPip,
    PlatformUsageMode_eMax
} PlatformUsageMode;

typedef enum PlatformDynamicRangeProcessingMode
{
    PlatformDynamicRangeProcessingMode_eAuto,
    PlatformDynamicRangeProcessingMode_eOff,
    PlatformDynamicRangeProcessingMode_eMax
} PlatformDynamicRangeProcessingMode;

typedef enum PlatformDynamicRangeProcessingType
{
    PlatformDynamicRangeProcessingType_ePlm,
    PlatformDynamicRangeProcessingType_eDolbyVision,
    PlatformDynamicRangeProcessingType_eTechnicolorPrime,
    PlatformDynamicRangeProcessingType_eMax
} PlatformDynamicRangeProcessingType;

typedef enum PlatformDynamicRange
{
    PlatformDynamicRange_eAuto,
    PlatformDynamicRange_eLegacy,
    PlatformDynamicRange_eSdr,
    PlatformDynamicRange_eHlg,
    PlatformDynamicRange_eHdr10,
    PlatformDynamicRange_eDolbyVision,
    PlatformDynamicRange_eHdr10Plus,
    PlatformDynamicRange_eTechnicolorPrime,
    PlatformDynamicRange_eUnknown,
    PlatformDynamicRange_eMax
} PlatformDynamicRange;

typedef enum PlatformColorimetry
{
    PlatformColorimetry_eAuto,
    PlatformColorimetry_e601,
    PlatformColorimetry_e709,
    PlatformColorimetry_e2020,
    PlatformColorimetry_eInvalid,
    PlatformColorimetry_eUnknown,
    PlatformColorimetry_eMax
} PlatformColorimetry;

typedef enum PlatformColorSpace
{
    PlatformColorSpace_eAuto,
    PlatformColorSpace_eRgb,
    PlatformColorSpace_eYCbCr,
    PlatformColorSpace_eInvalid,
    PlatformColorSpace_eUnknown,
    PlatformColorSpace_eMax
} PlatformColorSpace;

typedef enum PlatformCapability
{
    PlatformCapability_eUnsupported,
    PlatformCapability_eSupported,
    PlatformCapability_eUnknown,
    PlatformCapability_eMax
} PlatformCapability;

typedef enum PlatformInputEvent
{
    PlatformInputEvent_eUnknown,
    PlatformInputEvent_ePause,
    PlatformInputEvent_eExit,
    PlatformInputEvent_eUp,
    PlatformInputEvent_eDown,
    PlatformInputEvent_eRight,
    PlatformInputEvent_eLeft,
    PlatformInputEvent_eSelect,
    PlatformInputEvent_ePower,
    PlatformInputEvent_eChannelUp,
    PlatformInputEvent_eChannelDown,
    PlatformInputEvent_eNumber,
    PlatformInputEvent_eInfo,
    PlatformInputEvent_eMenu,
    PlatformInputEvent_eMax
} PlatformInputEvent;

typedef void (*PlatformInputEventCallback)(void * context, PlatformInputEvent event, int param);

typedef enum PlatformTriState
{
    PlatformTriState_eOff,
    PlatformTriState_eOn,
    PlatformTriState_eInactive,
    PlatformTriState_eMax
} PlatformTriState;

typedef enum PlatformHorizontalAlignment
{
    PlatformHorizontalAlignment_eLeft,
    PlatformHorizontalAlignment_eDefault = PlatformHorizontalAlignment_eLeft,
    PlatformHorizontalAlignment_eCenter,
    PlatformHorizontalAlignment_eRight,
    PlatformHorizontalAlignment_eMax
} PlatformHorizontalAlignment;

typedef enum PlatformVerticalAlignment
{
    PlatformVerticalAlignment_eTop,
    PlatformVerticalAlignment_eDefault = PlatformVerticalAlignment_eTop,
    PlatformVerticalAlignment_eCenter,
    PlatformVerticalAlignment_eBottom,
    PlatformVerticalAlignment_eMax
} PlatformVerticalAlignment;

typedef enum PlatformPlayMode
{
    PlatformPlayMode_eDefault,
    PlatformPlayMode_eLoop,
    PlatformPlayMode_eOnce,
    PlatformPlayMode_eMax
} PlatformPlayMode;

typedef enum PlatformRenderingPriority
{
    PlatformRenderingPriority_eAuto,
    PlatformRenderingPriority_eVideo,
    PlatformRenderingPriority_eGraphics,
    PlatformRenderingPriority_eMax
} PlatformRenderingPriority;

typedef struct PlatformRect
{
    int x;
    int y;
    unsigned width;
    unsigned height;
} PlatformRect;

typedef struct PlatformTextRenderingSettings
{
    PlatformRect rect;
    unsigned color;
    PlatformHorizontalAlignment halign;
    PlatformVerticalAlignment valign;
} PlatformTextRenderingSettings;

typedef enum PlatformAspectRatioType
{
    PlatformAspectRatioType_eAuto,
    PlatformAspectRatioType_eDisplay,
    PlatformAspectRatioType_ePixel,
    PlatformAspectRatioType_eMax
} PlatformAspectRatioType;

typedef struct PlatformAspectRatio
{
    PlatformAspectRatioType type;
    unsigned x;
    unsigned y;
} PlatformAspectRatio;

typedef struct PlatformPictureFormat
{
    unsigned width;
    unsigned height;
    unsigned rate;
    bool dropFrame;
    bool interlaced;
} PlatformPictureFormat;

typedef struct PlatformPictureCtrlSettings
{
    int contrast;
    int saturation;
    int hue;
    int brightness;
} PlatformPictureCtrlSettings;

typedef struct PlatformPictureInfo
{
    PlatformPictureFormat format;
    PlatformDynamicRange dynrng;
    PlatformColorimetry gamut;
    PlatformColorSpace space;
    PlatformAspectRatio ar;
    int sampling;
    int depth;
} PlatformPictureInfo;

typedef struct PlatformPictureModel
{
    PlatformPictureInfo info;
    PlatformTriState processing;
} PlatformPictureModel;

typedef struct PlatformSelectorModel
{
    bool format;
    bool dynrng;
    bool gamut;
    bool space;
    bool depth;
} PlatformSelectorModel;

typedef struct PlatformHdmiReceiverModel
{
    PlatformCapability format;
    PlatformCapability dynrng;
    PlatformCapability gamut;
    PlatformCapability space;
    PlatformCapability depth;
} PlatformHdmiReceiverModel;

typedef struct PlatformModel
{
    PlatformPictureModel vid[MAX_STREAMS];
    PlatformPictureModel gfx;
    PlatformRenderingPriority renderingPriority;
    PlatformPictureModel out;
    PlatformSelectorModel sel;
    PlatformHdmiReceiverModel rcv;
    PlatformPictureHandle thumbnail;
    PlatformPictureHandle background;
} PlatformModel;

typedef struct PlatformDynamicRangeProcessingCapabilities
{
    bool typesSupported[PlatformDynamicRangeProcessingType_eMax];
} PlatformDynamicRangeProcessingCapabilities;

typedef struct PlatformDynamicRangeProcessingSettings
{
    PlatformDynamicRangeProcessingMode modes[PlatformDynamicRangeProcessingType_eMax];
} PlatformDynamicRangeProcessingSettings;

typedef struct PlatformMediaPlayerStartSettings
{
    const char * url;
    PlatformPlayMode playMode;
    bool startPaused;
    bool stcTrick;
} PlatformMediaPlayerStartSettings;

typedef struct PlatformPqSettings
{
    struct
    {
        bool enabled;
    } sharpness;
    struct
    {
        bool enabled;
    } anr;
    struct
    {
        bool enabled;
    } dnr;
    struct
    {
        bool enabled;
    } deinterlacing;
    struct
    {
        bool enabled;
    } deringing;
    struct
    {
        bool enabled;
    } dejagging;

    PlatformPictureCtrlSettings pictureCtrlSettings;
} PlatformPqSettings;

typedef struct PlatformMediaPlayerSettings
{
    PlatformUsageMode usageMode;
    PlatformPqSettings pqSettings;
} PlatformMediaPlayerSettings;

#define PLATFORM_SCHEDULER_MAIN 0
#define PLATFORM_SCHEDULER_GFX 1
#define PLATFORM_SCHEDULER_USAGE 2
#define PLATFORM_SCHEDULER_COUNT 3

#endif /* PLATFORM_TYPES_H__ */
