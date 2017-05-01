/******************************************************************************
 * Broadcom Proprietary and Confidential. (c) 2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/
#ifndef PLATFORM_TYPES_H__
#define PLATFORM_TYPES_H__ 1

#include <stdbool.h>

#define MAX_MOSAICS 4

typedef struct Platform * PlatformHandle;
typedef struct PlatformPicture * PlatformPictureHandle;
typedef struct PlatformGraphics * PlatformGraphicsHandle;
typedef struct PlatformMediaPlayer * PlatformMediaPlayerHandle;
typedef struct PlatformDisplay * PlatformDisplayHandle;
typedef struct PlatformReceiver * PlatformReceiverHandle;
typedef struct PlatformInput * PlatformInputHandle;
typedef struct PlatformScheduler * PlatformSchedulerHandle;
typedef struct PlatformListener * PlatformListenerHandle;
typedef void (*PlatformCallback)(void * context, int param);

typedef enum PlatformInputMethod
{
    PlatformInputMethod_eRemote,
    PlatformInputMethod_eConsole,
    PlatformInputMethod_eMax
} PlatformInputMethod;

typedef enum PlatformDynamicRange
{
    PlatformDynamicRange_eAuto,
    PlatformDynamicRange_eSdr,
    PlatformDynamicRange_eHlg,
    PlatformDynamicRange_eHdr10,
    PlatformDynamicRange_eDolbyVision,
    PlatformDynamicRange_eInvalid,
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
    PlatformColorSpace_eYCbCr422,
    PlatformColorSpace_eYCbCr444,
    PlatformColorSpace_eYCbCr420,
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
    PlatformInputEvent_eQuit,
    PlatformInputEvent_eToggleOsd,
    PlatformInputEvent_eToggleGuide,
    PlatformInputEvent_eToggleOutputDynamicRangeLock,
    PlatformInputEvent_eCycleColorimetry,
    PlatformInputEvent_eCycleOutputDynamicRange,
    PlatformInputEvent_eNextThumbnail,
    PlatformInputEvent_ePrevThumbnail,
    PlatformInputEvent_eCycleBackground,
    PlatformInputEvent_eNextVideoSetting,
    PlatformInputEvent_ePrevVideoSetting,
    PlatformInputEvent_eNextGraphicsSetting,
    PlatformInputEvent_ePrevGraphicsSetting,
    PlatformInputEvent_eNextStream,
    PlatformInputEvent_ePrevStream,
    PlatformInputEvent_eTogglePause,
    PlatformInputEvent_eTogglePig,
    PlatformInputEvent_eToggleMosaicLayout,
    PlatformInputEvent_eToggleDetails,
    PlatformInputEvent_eScenario,
    PlatformInputEvent_eStartCommandShell,
    PlatformInputEvent_eMax
} PlatformInputEvent;

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

typedef struct PlatformPictureFormat
{
    unsigned width;
    unsigned height;
    unsigned rate;
    bool interlaced;
} PlatformPictureFormat;

typedef struct PlatformPictureInfo
{
    PlatformPictureFormat format;
    PlatformDynamicRange dynrng;
    PlatformColorimetry gamut;
    PlatformColorSpace space;
    unsigned depth;
} PlatformPictureInfo;

typedef struct PlatformPictureModel
{
    PlatformPictureInfo info;
    PlatformTriState plm;
} PlatformPictureModel;

typedef struct PlatformSelectorModel
{
    bool format;
    bool dynrng;
    bool gamut;
    bool space;
    bool depth;
} PlatformSelectorModel;

typedef struct PlatformReceiverModel
{
    PlatformCapability format;
    PlatformCapability dynrng;
    PlatformCapability gamut;
    PlatformCapability space;
    PlatformCapability depth;
} PlatformReceiverModel;

typedef struct PlatformModel
{
    PlatformPictureModel vid[MAX_MOSAICS];
    PlatformPictureModel gfx;
    PlatformPictureModel out;
    PlatformSelectorModel sel;
    PlatformReceiverModel rcv;
    PlatformPictureHandle thumbnail;
    PlatformPictureHandle background;
} PlatformModel;

#endif /* PLATFORM_TYPES_H__ */
