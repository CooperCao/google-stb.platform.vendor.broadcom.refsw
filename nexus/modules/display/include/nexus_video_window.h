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
#ifndef NEXUS_VIDEO_WINDOW_H__
#define NEXUS_VIDEO_WINDOW_H__

#include "nexus_types.h"
#include "nexus_surface.h"
#include "nexus_display_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Used to specify the BVN hardware block whose CRC is to be captured
**/
typedef enum NEXUS_BvnBlock
{
    NEXUS_BvnBlock_eSource,   /* input source, such as MPEG feeder or HDMI input.*/
    NEXUS_BvnBlock_eVfd,      /* video feeder (playback the captured buffer) */
    NEXUS_BvnBlock_eDnr,      /* digital noise reduction */
    NEXUS_BvnBlock_eMad,      /* deinterlace block: MAD or MVP */
    NEXUS_BvnBlock_eScl,      /* scaler */
    NEXUS_BvnBlock_eCap,      /* video capturer */
    NEXUS_BvnBlock_eMax
} NEXUS_BvnBlock;

/**
Summary:
Used to specify color key settings
**/
typedef struct NEXUS_VideoWindowColorKey
{
    bool enabled;     /* Enable colorkey. If disabled, the following values are ignored. */
    uint8_t lower;   /* Lower colorkey value, valid range 0x00 - 0xFF  */
    uint8_t upper;   /* Upper colorkey value, valid range 0x00 - 0xFF  */
    uint8_t mask;    /* Mask to use when comparing color key to source color, valid range 0x00 - 0xFF  */
} NEXUS_VideoWindowColorKey;

/**
Summary:
VideoWindow configuration
**/
typedef struct NEXUS_VideoWindowSettings
{
    NEXUS_Rect position; /* position of the window in the display coordinates */
    NEXUS_Rect clipRect; /* source clipping rectangle in the coordinates of clipBase.
                            if set to all zeros, no source clipping will be performed and clipBase is ignored.
                            negative source clipping (zoom out) is not supported.
                            see sourceClip for an additional method for clipping in source coordinates. */
    NEXUS_Rect clipBase; /* coordinates for the source clipping specified by clipRect.
                            if clipBase is equal to the position rectangle, then clipping done in the window coordinates.
                                this is useful if you want your clipping to be the same across source format changes.
                            if clipBase is equal to the source rectangle, then clipping is calculated for the source coordinates.
                                if you need pixel-precise source clipping, use sourceClip.
                            */

    struct {
        unsigned left, right, top, bottom;
    } sourceClip;        /* sourceClip will clip the source in source coordinates.
                            It can be specified in addition to clipRect/clipBase. The effect is cumulative.
                            If there is a source format change, the current sourceClip will simply be applied to the new source.
                            Use NEXUS_VideoInputResumeMode_eManual is you want to change the clipping before the new source is seen. */

    bool visible;        /* Is window visible? Defaults to true. */
    unsigned zorder;     /* Z-order relative to other VideoWindows and Graphics. A higher number is on top of a lower number. */
    NEXUS_VideoWindowContentMode contentMode; /* automatic aspect ratio correction */
    bool letterBoxDetect; /* enable letter box detection in the source */
    NEXUS_CallbackDesc letterBoxDetectionChange; /* This callback fires if letterBoxDetect is true and there is a change in the letter box
                             state. Call NEXUS_VideoWindow_GetLetterBoxStatus to get current letter box information. */
    bool letterBoxAutoCut; /* If true, Nexus will automatically cut off letterbox sidebars in the source if detected.
                              If false, the application can request a letterBoxDetectionChange callback, call NEXUS_VideoWindow_GetLetterBoxStatus and cut
                              the sidebars itself with source clipping. */
    NEXUS_PixelFormat pixelFormat;  /* pixel format of capture buffer and video feeder */
    bool forceCapture;    /* Always capture video to memory. This consumes more memory bandwidth but is needed to always have flicker-free
                             transitions for high-level operations like SyncChannel. */
    bool autoMaster;      /* If NEXUS_DisplaySettings.dropFrame is eNotSet and if this is true, then use the source
                             attached to this window as the VEC's frame rate master. Defaults to true for window 0. */

    uint8_t alpha;        /* alpha blending of video with other video or graphics where 0xFF is opaque and 0x00 is transparent. */

    NEXUS_CompositorBlendFactor sourceBlendFactor; /* Source refers to this video window. */
    NEXUS_CompositorBlendFactor destBlendFactor;   /* Dest referes to whatever the video window is being blended with. */
    uint8_t constantAlpha;                         /* constantAlpha is used if either sourceBlendFactor or destBlendFactor specify it */

    NEXUS_HeapHandle heap; /* If non-NULL, use a per-VideoWindow heap based on NEXUS_DisplayHeapSettings.
                              If NULL, use the Display Module heap based on NEXUS_DisplayModuleSettings.
                              Must be set before NEXUS_VideoWindow_AddInput is called. */

    NEXUS_VideoFormat minimumSourceFormat; /* Have display pipeline allocate buffers for this minimum source format. This allows for glitchless source format changes.
                                The default is eUnknown, which results in no preallocation.
                                Must be set before NEXUS_VideoWindow_AddInput is called. */
    NEXUS_VideoFormat minimumDisplayFormat; /* Have display pipeline allocate buffers for this minimum display format. This allows for glitchless display format changes.
                                The default is eUnknown, which results in no preallocation.
                                Must be set before NEXUS_VideoWindow_AddInput is called. */

    bool allocateFullScreen; /* Allocate capture buffers for full-screen display, regardless of current source size.
                                This reduces transition effects based on window size changes.
                                Must be set before NEXUS_VideoWindow_AddInput is called. */

    unsigned delay;          /* # of vsyncs of delay to add to this window. This delay will be added to any delay set by NEXUS_SyncChannel. */

    struct {
        bool enabled;                 /* If enabled, nexus will round scale factor according to the following tolerances.
                                         If within the tolerance, no scaling is performed.
                                         for example, avoid scaling 704x480 or 720x482 video to a 720x480 display. */
        unsigned horizontalTolerance; /* units of percentage */
        unsigned verticalTolerance;   /* units of percentage */
    } scaleFactorRounding;

    NEXUS_CallbackDesc outputRectChanged; /* fired whenever a member of NEXUS_VideoWindowStatus.outputRect changes */

    bool preferSyncLock; /* advisory setting. if you have two digital sources connected to the same display,
                            this boolean will determine which of the two paths is sync-locked.
                            or, if you have one digital source connected to two displays, this boolean will
                            determine which of the two paths is sync-locked.
                            if you have only one digital source and one display, or if you have an analog/hdmi source,
                            this setting has no effect. */

    struct {
        bool enabled;
        NEXUS_Rect position; /* the clearRect position is relative to the position of this video window */
        NEXUS_Pixel color; /* 32-bit ARGB color */
    } clearRect; /* VDC will draw the specified color into the video plane */

    struct {
        int rightViewOffset; /* offset of the right view */
    } window3DSettings;

    struct {
        NEXUS_BvnBlock bvnBlock;   /* the BVN hardware block whose CRC is to be captured */
        unsigned crcQueueSize;     /* if non-zero, the above specified BVN hardware block's CRC is captured and queued 
                                      at every vsync. use NEXUS_VideoWindow_GetCrcData to retrieve data. */
    } crc;	

    unsigned userCaptureBufferCount;  /* number of persistent capture buffers used by user for this window. If this is non-zero,
                                         NEXUS_VideoWindow_CaptureVideoBuffer and NEXUS_VideoWindow_ReleaseVideoBuffer will not
                                         call BVDC_Window_SetUserCaptureBufferCount to prevent VDC from reallocating/releasing
                                         buffers. This reduces latency in obtaining and releasing user buffers. */

    struct {
        NEXUS_VideoWindowColorKey luma;   /* luma color key */
        NEXUS_VideoWindowColorKey cb;     /* chroma red */
        NEXUS_VideoWindowColorKey cr;     /* chroma blue */
    } colorKey;
} NEXUS_VideoWindowSettings;

/**
Summary:
Open a VideoWindow from its Display.

Description:
See NEXUS_VideoWindow_AddInput to connect a NEXUS_VideoInput to this window.
**/
NEXUS_VideoWindowHandle NEXUS_VideoWindow_Open( /* attr{destructor=NEXUS_VideoWindow_Close}  */
    NEXUS_DisplayHandle display,
    unsigned windowIndex         /* The windowIndex is relative to its Display, usually 0 for main, 1 for PIP. */
    );

/**
Summary:
Close a VideoWindow
**/
void NEXUS_VideoWindow_Close(
    NEXUS_VideoWindowHandle window
    );

/**
Summary:
Get current settings
**/
void NEXUS_VideoWindow_GetSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowSettings *pSettings /* [out] */
    );

/**
Summary:
Apply new settings
**/
NEXUS_Error NEXUS_VideoWindow_SetSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowSettings *pSettings
    );

/**
Summary:
Status information returned by NEXUS_VideoWindow_GetStatus
**/
typedef struct NEXUS_VideoWindowStatus
{
    unsigned delay; /* Video processing delay on this path, in vsyncs. This includes both capture buffers and video processing delays.
                       Time in milliseconds = delay * 1000*1000/NEXUS_DisplayStatus.refreshRate */
    int phaseDelay; /* Additional sub-field (i.e. sub-vsync) phase delay, in microseconds. */
    NEXUS_Rect outputRect; /* active video after scaling and clipping (includes aspect ratio correction) in display coordinates */
    bool isSyncLocked; /* if true, the interrupts from this window's source are driving VDC processing and tighter lipsync is possible.
                          if false (known as sync-slip), another source's interrupts are driving the display. more delay is required on sync-slip paths. */
} NEXUS_VideoWindowStatus;

/**
Summary:
Get status information about the VideoWindow
**/
NEXUS_Error NEXUS_VideoWindow_GetStatus(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowStatus *pStatus /* [out] */
    );

/**
Summary:
Connect a video input to this window. This causes video to be displayed.
**/
NEXUS_Error NEXUS_VideoWindow_AddInput(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoInputHandle input
    );

/**
Summary:
Disconnect a video input from this window.

Description:
This is called automatically by NEXUS_VideoInput_Shutdown.
**/
NEXUS_Error NEXUS_VideoWindow_RemoveInput(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoInputHandle input
    );

/**
Summary:
Disconnect all video inputs from this window.
**/
void NEXUS_VideoWindow_RemoveAllInputs(
    NEXUS_VideoWindowHandle window
    );

/**
Summary:
Capture a video buffer

Description:
This allows the application to access a video buffer as a graphics surface.

This requires the video pipeline to already be in capture mode. If not, it will return NULL.
Analog video is always captured. Digital video may not be. If capture is not on, you can force
it on with NEXUS_VideoWindowSettings.forceCapture = true.

When the user is done with the video buffer, it must call NEXUS_VideoWindow_ReleaseVideoBuffer to return the buffer.
**/
NEXUS_SurfaceHandle NEXUS_VideoWindow_CaptureVideoBuffer(
    NEXUS_VideoWindowHandle window
    );

/**
Summary:
Release video buffer captured with NEXUS_VideoWindow_CaptureVideoBuffer
**/
NEXUS_Error NEXUS_VideoWindow_ReleaseVideoBuffer(
    NEXUS_VideoWindowHandle window,
    NEXUS_SurfaceHandle surface
    );

/**
Summary:
Used to specify the split screen mode per feature in NEXUS_VideoWindowSplitScreenSettings
**/
typedef enum NEXUS_VideoWindowSplitScreenMode
{
    NEXUS_VideoWindowSplitScreenMode_eDisable = 0,      /* no split screen. normal video. */
    NEXUS_VideoWindowSplitScreenMode_eLeft,             /* the demo feature will appear on the left */
    NEXUS_VideoWindowSplitScreenMode_eRight             /* the demo feature will appear on the right */
} NEXUS_VideoWindowSplitScreenMode;

/**
Summary:
Collection of split-screen options used to demonstrate video processing features.

Description:
Use NEXUS_VideoWindow_SetSplitScreenSettings to set these.
**/
typedef struct NEXUS_VideoWindowSplitScreenSettings
{
    NEXUS_VideoWindowSplitScreenMode    hue;            /* used for both hue and saturation. see NEXUS_PictureCtrlCommonSettings.hue and .saturation. */
    NEXUS_VideoWindowSplitScreenMode    contrast;       /* see NEXUS_PictureCtrlCommonSettings.contrast */
    NEXUS_VideoWindowSplitScreenMode    brightness;     /* see NEXUS_PictureCtrlCommonSettings.brightness */
    NEXUS_VideoWindowSplitScreenMode    colorTemp;      /* see NEXUS_PictureCtrlCommonSettings.colorTemp */
    NEXUS_VideoWindowSplitScreenMode    sharpness;      /* see NEXUS_PictureCtrlCommonSettings.sharpness */
    NEXUS_VideoWindowSplitScreenMode    contrastStretch;/* see NEXUS_PictureCtrlContrastStretch and NEXUS_PictureCtrlAdvColorSettings */
    NEXUS_VideoWindowSplitScreenMode    dnr;            /* see NEXUS_VideoWindowDnrSettings */
    NEXUS_VideoWindowSplitScreenMode    anr;            /* see NEXUS_VideoWindowAnrSettings */
    NEXUS_VideoWindowSplitScreenMode    dejagging;
    NEXUS_VideoWindowSplitScreenMode    deringing;

    NEXUS_VideoWindowSplitScreenMode    cm3d;           /* deprecated */

    NEXUS_VideoWindowSplitScreenMode    cms;
    NEXUS_VideoWindowSplitScreenMode    autoFlesh;
    NEXUS_VideoWindowSplitScreenMode    blueBoost;
    NEXUS_VideoWindowSplitScreenMode    greenBoost;
    NEXUS_VideoWindowSplitScreenMode    blueStretch;
} NEXUS_VideoWindowSplitScreenSettings;

/**
Summary:
Get current split screen settings
**/
void NEXUS_VideoWindow_GetSplitScreenSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowSplitScreenSettings *pSettings /* [out] */
    );

/**
Summary:
Set the video window into a split screen mode for a demo of various features.
**/
NEXUS_Error NEXUS_VideoWindow_SetSplitScreenSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowSplitScreenSettings *pSettings
    );

/***************************************************************************
Summary:
    Get the 4x3 color matrix for this video window.
****************************************************************************/
void NEXUS_VideoWindow_GetColorMatrix(
    NEXUS_VideoWindowHandle window,
    NEXUS_ColorMatrix *pSettings /* [out] */
    );

/***************************************************************************
Summary:
    Sets the 4x3 color matrix for this video window.

Description:
    NEXUS_VideoWindow_SetColorMatrix is used to override the display color
    matrix. Once user call this API, it is supposed that application will
    garrantee the color matrix been programmed is correct.
****************************************************************************/
NEXUS_Error NEXUS_VideoWindow_SetColorMatrix(
    NEXUS_VideoWindowHandle window,
    const NEXUS_ColorMatrix *pSettings /* attr{null_allowed=y} if NULL, internal defaults will be used. */
    );

/***************************************************************************
Summary:
Letter box status available from NEXUS_VideoWindow_GetLetterBoxStatus
****************************************************************************/
typedef struct NEXUS_VideoWindowLetterBoxStatus
{
    /* active video (white box) rectangle */
    unsigned whiteBoxLeft;
    unsigned whiteBoxTop;
    unsigned whiteBoxWidth;
    unsigned whiteBoxHeight;

    /* gray strip width: relative thin strip from white to black */
    unsigned grayWidthTop;
    unsigned grayWidthLeft;
    unsigned grayWidthRight;
    unsigned grayWidthBottom;

    /* edge strength: brightness of white box inner edge */
    unsigned edgeStrengthTop;
    unsigned edgeStrengthLeft;
    unsigned edgeStrengthRight;
    unsigned edgeStrengthBottom;
} NEXUS_VideoWindowLetterBoxStatus;

/***************************************************************************
Summary:
Get letter box status

Description:
See NEXUS_VideoWindowSettings.letterBoxDetectionChange for a callback when
this status changes.
****************************************************************************/
NEXUS_Error NEXUS_VideoWindow_GetLetterBoxStatus(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowLetterBoxStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
Settings used as input to the NEXUS_CalculateVideoWindowPosition helper function.

Description:
horizontalClipping and verticalClipping are specified in terms of total clipping.
For instance, if you want to horizontal clip 3% from each side, you should specify clipping of 600 or 6%.

If you want to perform panscan or nonlinear scaling, you should set manualAspectRatioCorrection to eFull (i.e. no manual aspect ratio correction), then set
NEXUS_VideoWindowSettings.contentMode to eFullNonLinear or ePanScan.
If you want VDC to perform automatic a/r correction, you should set manualAspectRatioCorrection to eFull (i.e. no manual aspect ratio correction), then set
NEXUS_VideoWindowSettings.contentMode to eBox or eZoom.

There is no reference to source pixel dimensions (i.e. width or height). The only source attribute required is the
source aspect ratio if manualAspectRatioCorrection is being used.

Clipping is applied first, then aspect ratio correction is applied to the clipped source.
****************************************************************************/
typedef struct NEXUS_CalculateVideoWindowPositionSettings {
    unsigned horizontalClipping; /* total horizontal clipping in units of 1/100%. */
    unsigned verticalClipping; /* total vertical clipping in units of 1/100%. */

    int horizontalPosition; /* position of source relative to the window. 0 is default. negative values are allowed and will result in clipping. */
    int verticalPosition; /* position of source relative to the window. 0 is default. negative values are allowed and will result in clipping. */

    NEXUS_VideoWindowContentMode manualAspectRatioCorrection; /* manual aspect ratio correction performed by this function.
                                if eFull (default), no a/r correction will be performed.
                                if eBox or eZoom, NEXUS_VideoPosition_CalculateVideoWindowSettings will perform manual a/r correction
                                based on sourceAspectRatio. */
    NEXUS_AspectRatio sourceAspectRatio; /* aspect ratio of unclipped source. this is only used if manualAspectRatioCorrection is eBox or eZoom.
                                The caller is responsible for registering for a NEXUS_VideoInput.sourceChanged callback and recalling this function
                                and applying new results whenever the source's aspect ratio changes. */
    unsigned sourceWidth; /* only required if sourceAspectRatio is NEXUS_AspectRatio_eSar or eSquarePixel */
    unsigned sourceHeight; /* only required if sourceAspectRatio is NEXUS_AspectRatio_eSar or eSquarePixel */
    struct {
        unsigned x, y;
    } sampleAspectRatio; /* only required if sourceAspectRatio is NEXUS_AspectRatio_eSar */

    NEXUS_Rect viewport; /* The desired window dimensions. No content will be displayed outside of this rectangle.
                            The viewport is a virtual window. Unlike the actual hardware VideoWindow, a viewport can have negative x/y coordinates and
                            x+width or y+height that extent past the display. */

    NEXUS_DisplayAspectRatio displayAspectRatio; /* if eAuto (default), calculate from displayWidth/displayHeight. */
    unsigned displayWidth;    /* width of the NEXUS_VideoFormat currently used in NEXUS_DisplaySettings for this window.
                                 normally you will use NEXUS_VideoFormat_GetInfo(NEXUS_DisplaySettings.format, &info) to obtain displayWidth and displayHeight */
    unsigned displayHeight;   /* height of the NEXUS_VideoFormat currently used in NEXUS_DisplaySettings for this window */
} NEXUS_CalculateVideoWindowPositionSettings;

/***************************************************************************
Summary:
Get default settings
****************************************************************************/
void NEXUS_GetDefaultCalculateVideoWindowPositionSettings(
    NEXUS_CalculateVideoWindowPositionSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
A helper function to determine NEXUS_VideoWindowSettings position and clipRect.

Description:
You must populate pWindowSettings using NEXUS_VideoWindow_GetSettings.
pOutputWindowSettings will initialized with pWindowSettings, then the following members will be adjusted:
- position
- clipRect
- clipBase
- contentMode

If you want to perform auto aspect ratio correction, you must set contentMode after calling this function.

The function does not have a NEXUS_VideoWindowHandle; therefore it does not and cannot modify a VideoWindow's state.
You must call NEXUS_VideoWindow_SetSettings afterward to do that.

Example:

    NEXUS_GetDefaultCalculateVideoWindowPositionSettings(&pos);
    pos.viewport = your_rect;
    pos.displayWidth = display_width;
    pos.displayHeight = display_height;
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    NEXUS_CalculateVideoWindowPosition(&pos, &windowSettings, &windowSettings);
    NEXUS_VideoWindow_SetSettings(window, &windowSettings);

****************************************************************************/
NEXUS_Error NEXUS_CalculateVideoWindowPosition(
    const NEXUS_CalculateVideoWindowPositionSettings *pSettings,
    const NEXUS_VideoWindowSettings *pInputWindowSettings, /* input window settings */
    NEXUS_VideoWindowSettings *pOutputWindowSettings /* [out] output window settings with adjustments applied */
    );

/***************************************************************************
Summary:
Get current AFD settings
***************************************************************************/
void NEXUS_VideoWindow_GetAfdSettings(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowAfdSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Set new AFD settings
***************************************************************************/
NEXUS_Error NEXUS_VideoWindow_SetAfdSettings(
    NEXUS_VideoWindowHandle window,
    const NEXUS_VideoWindowAfdSettings *pSettings
    );

/**
Summary:
Video window BVN hardware block's CRC data
**/
typedef struct NEXUS_VideoWindowCrcData
{
    struct {
        uint32_t luma;
        uint32_t chroma;
    } bvn; /* BVN VNET_B CRC */
} NEXUS_VideoWindowCrcData;

/**
Summary:
Get video window CRC data

Description:
You must set NEXUS_VideoWindowSettings.crcQueueSize to a non-zero value (for example, 30) to capture data.
**/
NEXUS_Error NEXUS_VideoWindow_GetCrcData(
    NEXUS_VideoWindowHandle window,
    NEXUS_VideoWindowCrcData *pData, /* attr{nelem=numEntries;nelem_out=pNumEntriesReturned} [out] array of crc data structures */
    unsigned numEntries,
    unsigned *pNumEntriesReturned /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_VIDEO_WINDOW_H__ */

