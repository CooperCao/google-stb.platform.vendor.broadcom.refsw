/***************************************************************************
 *     (c)2007-2014 Broadcom Corporation
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
#ifndef NEXUS_DISPLAY_H__
#define NEXUS_DISPLAY_H__

#include "nexus_types.h"
#include "nexus_surface.h"
#include "nexus_display_types.h"
#include "nexus_display_custom.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=*
A Display provides control of a video compositor.

Each display has one or more video windows.
For all displays except NEXUS_DisplayType_eBypass types, there is one graphics framebuffer.
The compositor combines these video sources into one display.

For NEXUS_DisplayType_eAuto types, there is a VEC associated with the compositor for analog output.

NEXUS_VideoInput tokens are connected to NEXUS_VideoWindowHandle's to route from various video sources.
See NEXUS_VideoWindow_AddInput.

NEXUS_VideoOutput tokens are added to NEXUS_Display to route to various video outputs.
See NEXUS_Display_AddOutput.
**/

/**
Summary:
NEXUS_DisplayType is used when opening a Display
**/
typedef enum NEXUS_DisplayType
{
    NEXUS_DisplayType_eAuto = 0, /* Configure display for VEC output. The VEC output is used to drive analog outputs like
                                    component, composite, svideo as well as HDMI output.
                                    This is called eAuto because the index of the VEC (e.g. primary or secondary VEC) is automatically chosen based on internal configuration. */
    NEXUS_DisplayType_eLvds,     /* Configure display for LVDS output by means of the DVO port. This is used to drive an LCD/PDP panel directly.
                                    This is only valid for some chips and Display indexes. */
    NEXUS_DisplayType_eDvo,      /* Configure display for DVO output. This is used to drive an LCD/PDP panel with an external LVDS daughter card.
                                    This is only valid for some chips and Display indexes. */
    NEXUS_DisplayType_eBypass,   /* Configure display for bypass output to CCIR656 or composite output. No graphics. */
    NEXUS_DisplayType_eMax
} NEXUS_DisplayType;

/**
Summary:
Display timing generator selection
**/
typedef enum NEXUS_DisplayTimingGenerator
{
    NEXUS_DisplayTimingGenerator_ePrimaryInput,
    NEXUS_DisplayTimingGenerator_eSecondaryInput,
    NEXUS_DisplayTimingGenerator_eTertiaryInput,
    NEXUS_DisplayTimingGenerator_eHdmiDvo, /* used for HDMI output master mode */
    NEXUS_DisplayTimingGenerator_e656Output,
    NEXUS_DisplayTimingGenerator_eEncoder, /* optional. maps to STG (simple timing generator). also see NEXUS_DisplayStgSettings.
                                              if you open with eAuto and use a display index dedicated to the encoder (like NEXUS_ENCODER_DISPLAY_IDX),
                                              it will select the appropriate STG for the encoder. */
    NEXUS_DisplayTimingGenerator_eAuto, /* default */
    NEXUS_DisplayTimingGenerator_eMax
} NEXUS_DisplayTimingGenerator;

/**
Summary:
3D source buffer selection.

Description:
In 3DTV, the default configuration dictates that the left buffer from source is routed
to the left buffer of the display and the right buffer from source is routed to the
right buffer of the display.
This default can be overriden such that either the left or right source buffer is
routed to both display buffers.

This enum is only effective when both the source and display are configured in 3D.
**/
typedef enum NEXUS_Display3DSourceBuffer {
    NEXUS_Display3DSourceBuffer_eDefault = 0, /* left source buffer for left display buffer, right source buffer for right display buffer */
    NEXUS_Display3DSourceBuffer_eLeft,        /* left source buffer for both display buffers */
    NEXUS_Display3DSourceBuffer_eRight,       /* right source buffer for both display buffers */
    NEXUS_Display3DSourceBuffer_eMax
} NEXUS_Display3DSourceBuffer;

/**
Summary:
Display settings
**/
typedef struct NEXUS_DisplaySettings
{
    NEXUS_DisplayType displayType;         /* The type of display to open. Cannot be changed after Open. */
    NEXUS_DisplayTimingGenerator timingGenerator;
    int vecIndex;                          /* If displayType is NEXUS_DisplayType_eAuto, this selects the VEC for this display.
                                              The default value is -1, which allows Nexus to select the mapping.
                                              Nexus will use the NEXUS_DisplayModuleSettings.vecSwap setting only if vecIndex is -1. */

    NEXUS_VideoFormat format;              /* Output format of video display. If an output is connected which can't support this format the SetSettings call will fail.
                                              Full-screen windows will be automatically resized.
                                              Graphics will be disabled. You must re-enable graphics, usually after setting the newly sized framebuffer. */
    NEXUS_DisplayAspectRatio aspectRatio;  /* Aspect ratio of display, if applicable */
    struct {
        unsigned x, y;
    } sampleAspectRatio;                   /* Valid if aspectRatio is NEXUS_DisplayAspectRatio_eSar */
    NEXUS_Timebase timebase;               /* Timebase that will drive the outputs */
    NEXUS_VideoInputHandle frameRateMaster; /* Deprecated. See NEXUS_VideoWindowSettings.autoMaster. */
    NEXUS_TristateEnable dropFrame;        /* eNotSet is default and means NEXUS_VideoWindowSettings.autoMaster will determine drop frame or non-drop frame rates.
                                              eDisable means 60.00, 30.00, etc., regardless of autoMaster and source.
                                              eEnable means 59.94, 29.97, etc., regardless of autoMaster and source. */
    NEXUS_Pixel background;                /* Background color in NEXUS_PixelFormat_eA8_R8_G8_B8 colorspace */
    bool xvYccEnabled;                     /* If true, Nexus will check the HdmiInput for xvYCC colorspace and, if detected, will output xvYcc to an HdmiOutput or PanelOutput. */
    NEXUS_DisplayHandle alignmentTarget;   /* If non-NULL, Nexus will ask VDC to align this display's timing to the display
                                              specified as the target.  This only works when both displays are the same frame
                                              rate and frameRateMaster is NULL and NEXUS_VideoWindowSettings.autoMaster is false.
                                              If NULL, disables display alignment for this display. */
    NEXUS_CallbackDesc vsyncCallback;      /* Callback fires on every output vsync. Nexus does not deliver this callback with any minimum latency or during
                                              the blanking interval. It should be used for general timing only. */
    struct {
        bool  overrideOrientation;           /* if true, then 'orientation' allows 2D display format to be used as half-res 3D format.
                                                for full-res, set 'format' with full-res 3D format value. */
        NEXUS_VideoOrientation  orientation; /* orientation of the 3D display. only applies if 'overrideOrientation' is true. */
        NEXUS_Display3DSourceBuffer sourceBuffer;
    } display3DSettings;

    unsigned crcQueueSize;              /* if non-zero, CRC capture is enabled. use NEXUS_Display_GetCrcData to retrieve data. */
} NEXUS_DisplaySettings;

/**
Summary:
This type describes possible update modes for the display module

Description:
When updating the display, it's often necessary to call multiple functions to get the
desired setting. Instead of having each function applied immediately to hardware, the user can instruct Nexus
to accumulate changes, then apply them all at once.

By default Nexus operates in NEXUS_DisplayUpdateMode_eAuto mode. In this mode, all changes
are applied immediately and do not accumulate.

If you set Nexus into NEXUS_DisplayUpdateMode_eManual mode, changes will accumulate.
Call NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eNow) to apply the
acculumated changes. Nexus will remain in NEXUS_DisplayUpdateMode_eManual mode.

Be aware that some changes cannot be accumulated, even in NEXUS_DisplayUpdateMode_eManual mode.
These will be applied immediately, as if it was in NEXUS_DisplayUpdateMode_eAuto mode.
This mainly applies to creation and destruction of resources.
**/
typedef enum NEXUS_DisplayUpdateMode
{
    NEXUS_DisplayUpdateMode_eAuto,   /* Update the display settings immediately. */
    NEXUS_DisplayUpdateMode_eManual, /* Accumulate changes until user calls NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eNow) */
    NEXUS_DisplayUpdateMode_eNow     /* Apply all changes accumulated in NEXUS_DisplayUpdateMode_eManual mode. */
} NEXUS_DisplayUpdateMode;

/**
Summary:
This function is used to control when changes in display settings take effect.

Description:
Most display module functions take effect immediately (e.g. resizing of a window).
NEXUS_DisplayModule_SetUpdateMode allows the user to accumulate changes and commit them as a single transaction.
This can reduce execution time and can eliminate transition effects.

For example to change size of two windows simultaneously user would need use this code:

     NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eManual);
     NEXUS_VideoWindow_GetSettings(window0, &windowSettings);
     windowSettings.position.height /= 2;
     windowSettings.position.width /= 2;
     NEXUS_VideoWindow_SetSettings(window0, &windowSettings);
     NEXUS_VideoWindow_GetSettings(window1, &windowSettings);
     windowSettings.position.height *= 2;
     windowSettings.position.width *= 2;
     NEXUS_VideoWindow_SetSettings(window1, &windowSettings);
     NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eAuto);
**/
NEXUS_Error NEXUS_DisplayModule_SetUpdateMode(
    NEXUS_DisplayUpdateMode updateMode
    );

/**
Summary:
Get default NEXUS_DisplaySettings before calling NEXUS_Display_Open.
**/
void NEXUS_Display_GetDefaultSettings(
    NEXUS_DisplaySettings *pSettings    /* [out] */
    );

/**
Summary:
Open a new Display.
**/
NEXUS_DisplayHandle NEXUS_Display_Open( /* attr{destructor=NEXUS_Display_Close}  */
    unsigned displayIndex,
    const NEXUS_DisplaySettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
Close a Display
**/
void NEXUS_Display_Close(
    NEXUS_DisplayHandle display
    );

/**
Summary:
Get the current NEXUS_DisplaySettings from the Display.
**/
void NEXUS_Display_GetSettings(
    NEXUS_DisplayHandle display,
    NEXUS_DisplaySettings *pSettings      /* [out] */
    );

/**
Summary:
Set new NEXUS_DisplaySettings to the Display.
**/
NEXUS_Error NEXUS_Display_SetSettings(
    NEXUS_DisplayHandle display,
    const NEXUS_DisplaySettings *pSettings
    );

#define NEXUS_DISPLAY_REMOVE_OUTPUT_WRONG_CONNECTION    NEXUS_MAKE_ERR_CODE(0x102, 0)
#define NEXUS_DISPLAY_REMOVE_OUTPUT_NOT_CONNECTED       NEXUS_MAKE_ERR_CODE(0x102, 1)
#define NEXUS_DISPLAY_ADD_OUTPUT_ALREADY_CONNECTED      NEXUS_MAKE_ERR_CODE(0x102, 2)

/**
Summary:
Adds unique VideoOutput to the display.

Description:
This causes the content for this display to be routed to this output.
The output must support the Display's current NEXUS_VideoFormat, otherwise the function fails.
**/
NEXUS_Error NEXUS_Display_AddOutput(
    NEXUS_DisplayHandle display,
    NEXUS_VideoOutput output
    );

/**
Summary:
Removes connected VideoOutput from the display
**/
NEXUS_Error NEXUS_Display_RemoveOutput(
    NEXUS_DisplayHandle display,
    NEXUS_VideoOutput output
    );

/**
Summary:
Removes all VideoOutputs connected to the display
**/
void NEXUS_Display_RemoveAllOutputs(
    NEXUS_DisplayHandle display
    );

/**
Summary:
Status information returned by NEXUS_Display_GetStatus
**/
typedef struct NEXUS_DisplayStatus
{
    unsigned refreshRate; /* actual frame rate in 1/1000 Hz (e.g. 29.97 = 29970).
                             This is determined by the framerate of NEXUS_DisplaySettings.format and
                             the drop-frame/non-drop-frame characteristics of the sync-locked source. */
    unsigned numWindows; /* number of video windows supported by this display. may be less than NEXUS_NUM_VIDEO_WINDOWS. */
    bool graphicsSupported; /* true if graphics is supported */
    NEXUS_DisplayTimingGenerator timingGenerator;
} NEXUS_DisplayStatus;

/**
Summary:
Get status information about the display
**/
NEXUS_Error NEXUS_Display_GetStatus(
    NEXUS_DisplayHandle display,
    NEXUS_DisplayStatus *pStatus /* [out] */
    );

/*********************************
* Graphics
********/

/*
Summary:
Graphics compositor settings which instruct the Display how to composite a graphics framebuffer with VideoWindows.

Description:
Graphics scaling is calculated by comparing the source width/height
with the destination width/height.

Instead of assuming the destination width/height is the width/height of the
display format, the application must specify it independently. This is because
the real issue is not the width/height of the display forward, but the x/y/width/height
of the viewable area of the TV. This can vary depending on the application and
target platform.

The source width/height are used to define the scale factor used by graphic
surfaces.
*/
typedef struct NEXUS_GraphicsSettings
{
    bool enabled; /* Enable graphics on the display.
                     When disabled, nexus will forget about any surface set with NEXUS_Display_SetGraphicsFramebuffer and
                     will destroy any internal state related to graphics.
                     After enabling, you must call NEXUS_Display_SetGraphicsFramebuffer to set the framebuffer. */
    bool visible; /* If false, nexus will hide graphics but will remember the framebuffer and will do minimal
                     reconfiguration of the display.
                     Both 'enabled' and 'visible' must be true to see graphics. 'visible' defaults to true. */
    bool antiflutterFilter; /* deprecated. use NEXUS_Graphics2DSettings.verticalFilter instead. */
    uint8_t alpha; /* alpha blending of framebuffer with video where 0xFF is opaque and 0x00 is transparent.  */
    unsigned zorder; /* Z-order of the graphics plane relative to the video windows' zorder. 0 is on bottom. */

    bool chromakeyEnabled; /* should the chromakey values be applied? */
    NEXUS_Pixel lowerChromakey; /* lower bound of chromakey range in NEXUS_PixelFormat_eA8_R8_G8_B8 colorspace */
    NEXUS_Pixel upperChromakey; /* upper bound of chromakey range in NEXUS_PixelFormat_eA8_R8_G8_B8 colorspace */

    NEXUS_Rect position; /* The area within the display where the surface is displayed. */
    NEXUS_Rect clip; /* The area within the surface that is displayed. The clipped area will be scaled to position.width and .height.
                        Some silicon does not support every combination of vertical & horizontal upscale & downscale.  */

    NEXUS_CallbackDesc frameBufferCallback;  /* Callback called when framebuffer has been set. After the callback fires,
        it's possible to use the previous framebuffer without any visible artifacting. */

    NEXUS_CompositorBlendFactor sourceBlendFactor; /* Source refers to the graphics. */
    NEXUS_CompositorBlendFactor destBlendFactor;   /* Dest referes to whatever the graphics is being blended with. */
    uint8_t constantAlpha;                         /* constantAlpha is used if either sourceBlendFactor or destBlendFactor specify it */

    NEXUS_GraphicsFilterCoeffs horizontalFilter;   /* GFD horizontal upscaler coefficients */
    NEXUS_GraphicsFilterCoeffs verticalFilter;     /* GFD vertical  upscaler coefficients */
    unsigned horizontalCoeffIndex;                 /* if horizontalFilter == eSharp, then this index is used for table-driven coefficients for horizontal upscale. */
    unsigned verticalCoeffIndex;                   /* if verticalFilter == eSharp, then this index is used for table-driven coefficients for vertical upscale. */

    struct {
        int rightViewOffset; /* offset of the right view */
    } graphics3DSettings;
} NEXUS_GraphicsSettings;

/**
Summary:
Get VEC vsync callback. This overrides any NEXUS_DisplaySettings.vsyncCallback setting.
**/
NEXUS_Error NEXUS_Display_SetVsyncCallback(
    NEXUS_DisplayHandle display,
    const NEXUS_CallbackDesc *pDesc /* attr{null_allowed=y} You can unset the callback by passing NULL */
    );

/**
Summary:
Get current graphics compositing settings.
**/
void NEXUS_Display_GetGraphicsSettings(
    NEXUS_DisplayHandle display,
    NEXUS_GraphicsSettings *pSettings /* [out] */
    );

/**
Summary:
Set graphics compositing settings.
**/
NEXUS_Error NEXUS_Display_SetGraphicsSettings(
    NEXUS_DisplayHandle display,
    const NEXUS_GraphicsSettings *pSettings
    );

/*
Summary:
Set which surface should be used as the framebuffer.

Description:
There is no implicit double-buffering. The user is responsible to provide any double or triple buffering logic by cycling the framebuffer between two or three surfaces.

If you are doing asynchronous blits into the new framebuffer, to avoid tearing your app must wait for those blits to be completed before calling
NEXUS_Display_SetGraphicsFramebuffer. See NEXUS_Graphics2D_Checkpoint.

The framebuffer will be switched on the next vsync. NEXUS_GraphicsSettings.frameBufferCallback will be fired after that switch happens.
If you are double buffering, to avoid tearing you must not write into the outgoing framebuffer until after frameBufferCallback is fired.
If you want to start updating the next framebuffer before waiting for frameBufferCallback, use triple buffering. You can achieve a sustained 60 fps
graphics update (or 50 fps for 50Hz systems), if your application can prepare the next framebuffer in less than the 16 msec vsync time minus a small amount
of time (e.g. 1-2 msec) to process the frameBufferCallback.

If you call NEXUS_Display_SetGraphicsFramebuffer a second time, before the first call was able to be applied (i.e. before the next vsync), your first call
will be overwritten. NEXUS_Display_SetGraphicsFramebuffer calls are not queued.

Framebuffers are always placed at coordinate 0,0.

The difference between this surface's width and height and the NEXUS_GraphicsSettings destinationWidth and destinationHeight will
result in graphics feeder scaling, if the scaling feature exists.

When you set NEXUS_GraphicsSettings.enabled = false, nexus will forget about any framebuffer that was set. The application can then
safely delete the surface. To re-enable, you must set NEXUS_GraphicsSettings.enabled = true and call NEXUS_Display_SetGraphicsFramebuffer with a new surface.
*/
NEXUS_Error NEXUS_Display_SetGraphicsFramebuffer(
    NEXUS_DisplayHandle display,
    NEXUS_SurfaceHandle frameBuffer
    );

/**
Summary:
Get current color space convertor matrix for the graphics feeder (GFD)
**/
void NEXUS_Display_GetGraphicsColorMatrix(
    NEXUS_DisplayHandle display,
    NEXUS_ColorMatrix *pColorMatrix /* [out] */
    );

/**
Summary:
Set new color space convertor matrix for the graphics feeder (GFD)

Description:
This will override anything set by NEXUS_Display_SetGraphicsColorSettings.
**/
NEXUS_Error NEXUS_Display_SetGraphicsColorMatrix(
    NEXUS_DisplayHandle display,
    const NEXUS_ColorMatrix *pColorMatrix /* attr{null_allowed=y} */
    );

/**
Summary:
**/
void NEXUS_Display_GetGraphicsColorSettings(
    NEXUS_DisplayHandle display,
    NEXUS_GraphicsColorSettings *pSettings /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_Display_SetGraphicsColorSettings(
    NEXUS_DisplayHandle display,
    const NEXUS_GraphicsColorSettings *pSettings
    );

/**
Summary:
This display's interrupts will drive a VideoDecoder without any VideoWindow connection.
Used for video-as-graphics applications.
If the display is closed, interrupts will stop and the VideoDecoder will become idle.
**/
NEXUS_Error NEXUS_Display_DriveVideoDecoder(
    NEXUS_DisplayHandle display
    );

/* backward compatibility */
#define NEXUS_Display_ConnectVideoInput(DISPLAY, VIDEOINPUT) NEXUS_Display_DriveVideoDecoder(DISPLAY)
#define NEXUS_Display_DisconnectVideoInput(DISPLAY, VIDEOINPUT) /* no-op */

/**
Summary:
Enable a default set of picture quality features including MAD, ANR, and DNR.

Description:
By default, Nexus starts with picture quality features disabled. Users must enable each feature and can control each feature's settings.

NEXUS_DisplayModule_SetAutomaticPictureQuality performs a one-time automatic enabling of picture quality features for all displays and windows in the system.
After this function finishes, the user can call PQ functions and learn or modify these settings.

This is a one-time setting. No on-going automatic picture quality changes will be made.
**/
NEXUS_Error NEXUS_DisplayModule_SetAutomaticPictureQuality(void);

/**
Deprecated
**/
NEXUS_Error NEXUS_DisplayModule_SetConfigurationId(
    unsigned configurationId
    );

/**
Summary:
Get compositor CRC data

Description:
You must set NEXUS_DisplaySettings.crcQueueSize to a non-zero value (for example, 30) to capture data.
**/
NEXUS_Error NEXUS_Display_GetCrcData(
    NEXUS_DisplayHandle display,
    NEXUS_DisplayCrcData *pData, /* attr{nelem=numEntries;nelem_out=pNumEntriesReturned} [out] array of crc data structures */
    unsigned numEntries,
    unsigned *pNumEntriesReturned /* [out] */
    );

/*
Summary:
Description of the stereo (Left/Right) framebuffer

**/

typedef struct NEXUS_GraphicsFramebuffer3D {
    NEXUS_VideoOrientation  orientation; /* orientation of the graphics framebuffer */
    NEXUS_SurfaceHandle main; /* mandatory, main surface */
    NEXUS_SurfaceHandle alpha; /* deprecated */
    NEXUS_SurfaceHandle right; /* optional, right surface */
    NEXUS_SurfaceHandle rightAlpha; /* deprecated */
    uint32_t alphaW0; /* deprecated */
    uint32_t alphaW1; /* deprecated */
} NEXUS_GraphicsFramebuffer3D;

/**
Summary:
Get default values for NEXUS_DisplayCustomFormatSettings
**/
void NEXUS_Graphics_GetDefaultFramebuffer3D(
    NEXUS_GraphicsFramebuffer3D *pFrameBuffer3D /* [out] */
    );

/*
Summary:
Sets description of surfaces used as 3d(Left/Right)  framebuffer

Description:
This function is  the same as NEXUS_Display_SetGraphicsFramebuffer with sole exception
that it could provides information about composition of 3D framebuffer

See Also:
    NEXUS_Display_SetGraphicsFramebuffer
    NEXUS_Graphics_GetDefaultFramebuffer3D
*/
NEXUS_Error NEXUS_Display_SetGraphicsFramebuffer3D(
    NEXUS_DisplayHandle display,
    const NEXUS_GraphicsFramebuffer3D *pFrameBuffer3D
    );

/**
Summary:
STG (simple timing generator) settings.

Description:
See BVDC_Display_SetStgConfiguration for more detailed documentation.
**/
typedef struct NEXUS_DisplayStgSettings
{
    bool enabled; /* If NEXUS_DisplayTimingGenerator_eEncoder is set (the STG master), this must be true; Nexus will reject a false setting.
                     When master TG of the display is not STG, if enabled = true then the STG output is slaved on that display;
                     else STG output would be detached from that display. */
    bool nonRealTime; /* configure STG trigger as real-time or non-real-time */
} NEXUS_DisplayStgSettings;

/**
Summary:
Get STG settings
**/
void NEXUS_Display_GetStgSettings(
    NEXUS_DisplayHandle display,
    NEXUS_DisplayStgSettings *pSettings  /* [out] */
    );

/**
Summary:
Set STG settings
**/
NEXUS_Error NEXUS_Display_SetStgSettings(
    NEXUS_DisplayHandle display,
    const NEXUS_DisplayStgSettings *pSettings
    );

/**
Summary:
display module capabilities
**/
typedef struct NEXUS_DisplayCapabilities
{
    unsigned numDisplays; /* Deprecated. The display list is not packed, so you can't assume 0..numDisplays-1 can be opened.
                             Instead, check display[].numVideoWindows > 0. */
    struct {
        unsigned numVideoWindows; /* if 0, display is not usable */
        struct {
            unsigned width, height; /* if 0, graphics is not usable */
        } graphics; /* max capability */
    } display[NEXUS_MAX_DISPLAYS];
    bool displayFormatSupported[NEXUS_VideoFormat_eMax]; /* is NEXUS_DisplaySettings.format supported by any display in the system? */
    unsigned numLetterBoxDetect; /* see NEXUS_VideoWindowSettings.letterBoxDetect */
} NEXUS_DisplayCapabilities;

/**
Summary:
get display module capabilities
**/
void NEXUS_GetDisplayCapabilities(
    NEXUS_DisplayCapabilities *pCapabilities /* [out] */
    );

/**
Summary:
Module status returned in NEXUS_PlatformStatus
**/
typedef struct NEXUS_DisplayModuleStatus
{
    struct {
        NEXUS_MemoryBlockHandle memory; /* NULL if RUL capture is not enabled */
        unsigned offset; /* offset into memory block */
        size_t size; /* size of memory for BDBG_Fifo */
        size_t elementSize; /* BDBG_Fifo elementSize */
    } rulCapture;
} NEXUS_DisplayModuleStatus;

typedef struct NEXUS_DisplayMaxMosaicCoverage
{
    unsigned maxCoverage; /* percentage of screen that can be covered with mosaic windows */
} NEXUS_DisplayMaxMosaicCoverage;

NEXUS_Error NEXUS_Display_GetMaxMosaicCoverage(
    unsigned displayIndex, /* for now, only displayIndex 0 is supported */
    unsigned numMosaics,
    NEXUS_DisplayMaxMosaicCoverage *pCoverage
    );

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_DISPLAY_H__ */
