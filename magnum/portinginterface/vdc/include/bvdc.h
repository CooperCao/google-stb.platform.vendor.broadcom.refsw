/******************************************************************************
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
 ******************************************************************************/
#ifndef BVDC_H__
#define BVDC_H__

#include "bchp.h"          /* Chip information */
#include "breg_mem.h"      /* Chip register access (memory mapped). */
#include "bmma.h"          /* Chip memory access. */
#include "brdc.h"          /* Register DMA */
#include "bfmt.h"          /* Video timing format */
#include "bint.h"          /* L2 Interrupt Manager */
#include "bvdc_errors.h"   /* VDC error codes */
#include "bavc.h"          /* bavc */
#include "bavc_vee.h"      /* bavc soft encoder */
#include "btmr.h"          /* Timestamps */
#include "bvdc_tnt.h"
#include "bavc_hdmi.h"
#include "bpxl_plane.h"
#include "bbox.h"
#include "bbox_vdc.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************* Module Overview ********************************
The Video Display Control API is a library that allows control of video
compositing and display backend of Broadcom's video cores.  Its fairly
general API can be used for a wide range of hardware implementations.

This API is targeted for Hydra core based chip, and future chips.  This API
exploits all possible display modes provided by chip capabilities, and
other modes that haven't been discovered.  It also attempts to hide all the
gritty hardware details from the application.

The API operates on a context handle.  There is main VDC handle which
represents a certain state of the video display control hardware.  In
addition there are also subset of API that operate on subcontext handle
that represents single compositor, display window (graphics or video),
source (graphics or video), or display.  It defines various inputs, outputs,
display location, output destinations, process modes, and so on.

Most of the API functions are used in user mode context.  There are also a
few special API functions for using in interrupt handle context. They are
more likely used to switch video or graphics frame buffer, and are named
with an "_isr" suffix.

In the user mode context, after one has completed the intended setup to the
display context, one "applies" it to the hardware.  It is only at this time
that the actual configurations (i.e. register writes or queues) of the
hardware occurs.  Modifying a display control contexts does not affect the
hardware at all until it is applied.  Please notice that the setup calls
between two "applyings" is order independent, and that if there is conflict
found as "applying" all new settings would not be applied.

As a contrast, a setting in _isr mode is always applied to the hardware
immediately. The calling order is significant.  Particularly, if a graphics or
video picture is updated in _isr mode and some  configurings (such as source
clip, destination rectangles, or some format) are supposed to be updated
accordingly, they must be done in the same _isr session, otherwise partial
change might be used in hardware and failure or unexpected effect could
happen.  Since there is no error check as in user mode "applying", user needs
to be more careful as calling _isr API functions.

A VDC module could be instanced with several compositor sub-module, each
compositor sub-module could have several window sub-modules, the windows in
a compositor are blended together, and the blended result is then displayed
by a display module.  Each window has a unique source, either video or
graphics, and each picture from the source is configured to experience a
series of processing, such as scaling, before the blending.  For more
details, please refer to the description of BVDC_Heap_Handle,
BVDC_Compositor_Handle, BVDC_Window_Handle, BVDC_Source_Handle, and
BVDC_Display_Handle.
***************************************************************************/


/***************************************************************************
 * VDC handles
 **************************************************************************/

/***************************************************************************
Summary:
    Main Video Display Control (VDC) context handle.

Description:
    This is an opaque handle that application created with BVDC_Open.
    BVDC_Handle holds the context of the VDC.  There should only one
    BVDC_Handle per chip.  The main VDC handle is use to create additional
    VDC sub handles.  Currently these are the available handles that can
    be created from BVDC_Handle, BVDC_Heap_Handle, BVDC_Compositor_Handle,
    BVDC_Display_Handle, and BVDC_Source_Handle.

See Also:
    BVDC_Open, BVDC_Close.
***************************************************************************/
typedef struct BVDC_P_Context              *BVDC_Handle;

/***************************************************************************
Summary:
    The heap handle created from a secondary (or shared) memory controller.

Description:
    This is an opaque handle that the application created using
    BVDC_Heap_Create. This heap is separate from the internal heap created
    by the VDC.  This heap handle allows application to specify the
    Which memory controller use with which Window.  In addition if memory
    are to be pooled and shared between PIs application can created
    a BVDC_Heap_Handle from the dedicated shared memory.

See Also:
    BVDC_Heap_Create, BVDC_Heap_Destroy, BVDC_Window_Create.
***************************************************************************/
typedef struct BVDC_P_BufferHeapContext    *BVDC_Heap_Handle;

/***************************************************************************
Summary:
    Compositor context handle.

Description:
    This is an opaque handle that application created with
    BVDC_Compositor_Create.  BVDC_Compositor_Handle holds the context for a
    compositor, and it is created from a BVDC_Handle.  There are multiple
    compositors for a given BVDC_Handle.  The maximum support number of
    compositor can be queried with BVDC_GetMaxCompositorCount.

    A compositor could have several video and graphics windows, which are
    blended together. The blended result is then displayed using a display
    module. The compositor handle is needed when a window handle or display
    handle is created. The maximum number of video windows and graphics
    windows supported by a specific compositor can be queried with
    BVDC_Compositor_GetMaxWindowCount.

    When a compositor handle is no longer needed, it should be destroyed by
    BVDC_Compositor_Destroy.

See Also:
    BVDC_Compositor_Create, BVDC_Compositor_Destroy.
    BVDC_Compositor_SetBackgroundColor, BVDC_Compositor_GetBackgroundColor
    BVDC_Compositor_GetMaxWindowCount
***************************************************************************/
typedef struct BVDC_P_CompositorContext    *BVDC_Compositor_Handle;

/***************************************************************************
Summary:
    Window context handle.

Description:
    This is an opaque handle that application created with
    BVDC_Window_Create.  BVDC_Window_Handle holds the context for a graphics
    or video window, and is created from a BVDC_Compositor_Handle.  There
    could be multiple windows for a given compositor.  The maximum supported
    number of video windows and graphics windows can be queried with
    BVDC_Compositor_GetMaxWindowCount.

    Each window has a unique source handle. Whether a window is a video window
    or graphics window, is determined when the window handle is created,
    basing on the source handle passed to BVDC_Window_Create  After a window
    is created, its source could NOT be changed.

    The video and graphics windows in a compositor are blended together, and
    the blended result is then displayed using a display module.  For example
    on a system that support "Main" video and "Pip" video, the compositor
    would have two (2) video windows.  One window could display a Mpeg video
    source in fullscreen or "Main", and the other could display a Analog line
    -in source in quarter size or "Pip".  There could also be a graphics
    window to display the menu, which is blended together with the video
    windows.

    For each window, the user could configure the blending and clip / scaling
    using window sub-module API functions. For example, user could set the
    source clip rectangle, scaler output, and destination rectangle.  The
    three together define how to pick a rectangle sub-area from the source for
    further process, how to scale the picked source rectangle, how to pick a
    rectangle sub-area from the scaled picture, and where, on the screen, to
    display the picked sub-area of the scaled picture. The three rectangles
    mentioned above could be set in user mode context for one time setting, or
    in interrupt handler for real time updating, and could be configured to be
    automatically adjusted by BVDC for best display effect. The adjustments
    include box (black patch) auto-cut, pan scan, aspect ratio correction, and
    scale factor rounding. They are performed in the above order. The user set
    rectangles are applied after pan scan and before aspect ratio correction.

    Most of the window API functions applies to both graphics and video in the
    same way. There are a few exceptions. The following API functions only
    apply to video;

    BVDC_Window_SetSurfaceBuffer, BVDC_Window_GetSurfaceBuffer,
    BVDC_Window_SetDeinterlaceConfiguration,
    BVDC_Window_GetDeinterlaceConfiguration,
    BVDC_Window_SetPanScanType,   BVDC_Window_GetPanScanType,
    BVDC_Window_SetUserPanScan,   BVDC_Window_GetUserPanScan
    BVDC_Window_EnableBoxDetect,  BVDC_Window_DisableBoxDetect

    When a window handle is no longer needed, it should be destroyed by
    BVDC_Window_Destroy, and apply the changes by calling BVDC_ApplyChanges.

See Also:
    BVDC_Window_Create, BVDC_Window_Destroy.
***************************************************************************/
typedef struct BVDC_P_WindowContext        *BVDC_Window_Handle;

/***************************************************************************
Summary:
    Source context handle.

Description:
    This is an opaque handle that holds the context for a particular source
    device to VDC module. It is needed when the source device is configured
    or used.

    Source handle is created with BVDC_Source_Create and from a BVDC_Handle.
    A BAVC_SourceId enum number is passed to BVDC_Source_Create to identify
    the source device (hardware module) that the source handle is to be
    created to represent.

    There are two major type of sources, the video source and graphics
    source. For example, Mpeg feeder, and ITU-R-656 input are video
    sources, and graphics feeder is graphics source.

    After created, a source handle could be assigned to a window (of the same
    type, i.e. video or graphics, with a 7038 chip) as its unique source with
    BVDC_Window_Create. A source could be connected to multiple windows.

    Several video and graphics windows (two video windows and one graphics
    window in 7038 chip) could be blended together in a compositor. Before
    the pixel color is passed to the window for blending, color space
    conversion would automatically be done to each source input to match the
    main window source.

    And before the color space conversion, video inputs could involve
    dimension scaling / filtering and luma keying operation in order, and
    graphics input could experience a series of operations in the order of
    color key, horizontal scaling, and gamma correction. And before the
    operations mentioned above, the graphics pixel fetched from memory is
    always expanded to an AYCrCb or ARGB format with 8 bits per component.
    If the pixel format does not have alpha, 0xff is used.

    The fully scaling of video input and the horizontal scaling of
    graphics input, are configured by VDC module automatically, basing on
    the window source clip rectangle and scaler output rectangle, the other
    operations are configured by the higher level software using the source
    sub-module functions, which are prefixed with BVDC_Source_.

    There are two Mpeg feeders, one vedc, and one itu-r-656 input in the
    7038 chip. Once the video pixel is feeded into VDC module, it is
    automatically expanded to 4:4:4 YCrCb format before blended with other
    windows.

    TODO: add more details for mpeg feeder and itu-r-656.

    There are two graphics feeders in the 7038 chip. Each of them fetches
    pixel data from the video memory and outputs to the video compositors. A
    variety of YCrCb, ARGB and palette pixel formats are supported for both SD
    and HD applications. Please refer to BPXL module for the complete list.

    The attributes and configurations of mpeg decoder, and itu-r-656
    are configured using the appropriate libraries. For example, the
    Titan API is used to set up which stream to decode. The VDM only
    knows about MPEG as a particular source of bits.  For
    eItur656_x sources it will get capture and playback with appropriate
    scale ratio.

    When a source handle is no longer needed, it should be destroyed by
    BVDC_Source_Destroy.

    Some source sub-module API finctions are applicable to both video and
    graphics similarly. But quite amount of them are currently only used to
    some specific type sources. Calling a source sub-module function with a
    source handle that represents a wrong type of source device would return
    an error.

Example:
    The following API functions are currently only used for VDEC type of
    source;

       BVDC_Source_SetVideoFormat, BVDC_Source_GetVideoFormat

    The following API functions are currently only used for MPEG type of
    source;

       BVDC_Source_SetVideoMuteColor, BVDC_Source_GetVideoMuteColor
       BVDC_Source_MpegDataReady_isr

    The following API functions are currently only used for graphics type of
    source;

       BVDC_Source_SetSurface,
       BVDC_Source_SetSurface_isr,
       BVDC_Source_GetSurface,
       BVDC_Source_GetSurface_isr,
       BVDC_Source_SetChromaExpansion,
       BVDC_Source_EnableColorKey, BVDC_Source_DisableColorKey,
       BVDC_Source_SetHorizontalScaleCoeffs,
       BVDC_Source_EnableGammaCorrection, BVDC_Source_DisableGammaCorrection

See Also:
    BVDC_Source_Create, BVDC_Source_Destroy,
    BVDC_Window_Create,
***************************************************************************/
typedef struct BVDC_P_SourceContext        *BVDC_Source_Handle;

/***************************************************************************
Summary:
    Display context handle.

Description:
    This is an opaque handle that applications create with
    BVDC_Display_Create.  BVDC_Display_Handle holds the context for a
    display, and it is created from a BVDC_Compositor_Handle.  Therefore,
    the number of display handles supported is based on the maximum number
    of compositors.

See Also:
    BVDC_Display_Create, BVDC_Display_Destroy.
***************************************************************************/
typedef struct BVDC_P_DisplayContext       *BVDC_Display_Handle;


/***************************************************************************
 * VDC defines
 **************************************************************************/

/***************************************************************************
Summary:
    This definition defines the ITU-R-656 video outputs

See Also:
    BVDC_Display_Set656Configuration, BVDC_Display_Get656Configuration
***************************************************************************/
#define BVDC_Itur656Output_0                UINT32_C(0x0001)

/***************************************************************************
Summary:
    This definition defines the HD_DVI video input port

    BVDC_HdDviInput_Ext - This is for external chip connected to HD_DVI
        input port.  For example in the ref board 97398 the 7411 is connected
        to this BVDC_HdDviInput_Ext.

    BVDC_HdDviInput_Hdr - This is for internalHDMI Reveicver (HDR).

See Also:
    BVDC_Source_SetInputPort, BVDC_Source_GetInputPort
***************************************************************************/
#define BVDC_HdDviInput_Ext                 UINT32_C(0x0)
#define BVDC_HdDviInput_Hdr                 UINT32_C(0x1)

/***************************************************************************
Summary:
    This definition defines the DAC (Digital to Analog Converter) outputs.

Notes:
    This is how it gets mapped on "97038 Broadcom Reference Board" (Other
    system board might be different).  It's recommended that application
    defines DAC to approriate name to improve code readability.  For example
    #define BVDC_Dac_Cvbs_0              BVDC_Dac_x (tailor after board)
    #define BVDC_Dac_Chroma_0            BVDC_Dac_x
    #define BVDC_Dac_Luma_0              BVDC_Dac_x
    etc.

    Software           Schematic         PIN
    BVDC_Dac_0         VDAC0_COMPOSITE   E17
    BVDC_Dac_1         VDAC0_CHROMA      E16
    BVDC_Dac_2         VDAC0_LUMA        D16
    BVDC_Dac_3         VDAC1_RED/Pr      E18
    BVDC_Dac_4         VDAC1_GREEN/Y     D18
    BVDC_Dac_5         VDAC1_BLUE/Pb     C19

See Also:
    BVDC_Display_SetDacConfiguration, BVDC_Display_GetDacConfiguration
***************************************************************************/
#define BVDC_Dac_0                          UINT32_C(0x001)
#define BVDC_Dac_1                          UINT32_C(0x002)
#define BVDC_Dac_2                          UINT32_C(0x004)
#define BVDC_Dac_3                          UINT32_C(0x008)
#define BVDC_Dac_4                          UINT32_C(0x010)
#define BVDC_Dac_5                          UINT32_C(0x020)
#define BVDC_Dac_6                          UINT32_C(0x040)
#define BVDC_MAX_DACS                       7

/***************************************************************************
Summary:
    This definition defines the Hdmi outputs supported

See Also:
    BVDC_Display_SetHdmiConfiguration, BVDC_Display_GetHdmiConfiguration
***************************************************************************/
#define BVDC_Hdmi_0                         UINT32_C(0x00000001)
#define BVDC_Hdmi_1                         UINT32_C(0x00000002)

/***************************************************************************
Summary:
    This definition defines the Rfm outputs supported

See Also:
    BVDC_Display_SetRfmConfiguration, BVDC_Display_GetRfmConfiguration
***************************************************************************/
#define BVDC_Rfm_0                          UINT32_C(0x001)

/***************************************************************************
Summary:
    This definition defines the analog component outputs supported

See Also:
    BVDC_Display_SetDacConfiguration,
    BVDC_Display_SetMpaaDecimation, BVDC_Display_GetMpaaDecimation
***************************************************************************/
#define BVDC_Cmpnt_0                        UINT32_C(0x001)

/***************************************************************************
Summary:
    This definition defines the maximum z-order for window.

Description:
    The Z-Order specifies the video and graphics ordering when compositing.
    Z-Order of 0 is bottom most, and BVDC_Z_ORDER_MAX is front most.  Each
    window of the same compositor must have different z-order value.
***************************************************************************/
#define BVDC_Z_ORDER_MAX                    (4)

/***************************************************************************
Summary:
    This definition defines the maximum alpha for window.

Description:
    The alpha of the window use for blending.  This is for video window
    only.  Graphics surface each will has it alpha per-pixel base.

See Also:
    BVDC_Window_SetBlendFactor, BVDC_Window_GetBlendFactor,
    BVDC_Window_SetAlpha, BVDC_Window_GetAlpha.
***************************************************************************/
#define BVDC_ALPHA_MAX                      (0xff)

/***************************************************************************
Description:
    This macro represents the 3 color channels in VDEC.  Associated with
    YPrPb/YCrCb/RGB.

***************************************************************************/
#define BVDC_COLOR_CHANNELS            (3)

/***************************************************************************
Description:
    This macro represents the number of phase lock bin count.

***************************************************************************/
#define BVDC_PC_PHASE_LOCK_BIN         (15)

/***************************************************************************
Description:
    This macro represents the number of LUMA histogram data count.

***************************************************************************/
#define BVDC_LUMA_HISTOGRAM_COUNT      (64)

/***************************************************************************
Description:
    This macro represents the number of threshold level used in histogram.

***************************************************************************/
#define BVDC_LUMA_HISTOGRAM_LEVELS      (4)

/***************************************************************************
Description:
    This macro represents the number of DC table rows and colums.

***************************************************************************/
#define BVDC_DC_TABLE_ROWS              (15)
#define BVDC_DC_TABLE_COLS              (17)

/***************************************************************************
Description:
    This macro represents the number of Hue/Sat histogram data count.

***************************************************************************/
#define BVDC_HUE_SAT_HISTOGRAM_COUNT   (23)

/***************************************************************************
Description:
    This macro represents the number of Cr/Cb histogram data count.

***************************************************************************/
#define BVDC_CR_CB_HISTOGRAM_COUNT     (32)

/***************************************************************************
Description:
    This macro represents the number of element in the Color Space Converstion
    matrix.  It's a 3x5 matrix.

***************************************************************************/
#define BVDC_CSC_COEFF_COUNT           (15)

/***************************************************************************
Description:
    This macro represent the number of 2D/3D comb parameters.

***************************************************************************/
#define BVDC_COMB_PARAM_COUNT          (8)

/***************************************************************************
Description:
    This macro represents the QP scaling steps.

See Also:
    BVDC_Dnr_Settings
***************************************************************************/
#ifndef BVDC_QP_ADJUST_STEPS
#define BVDC_QP_ADJUST_STEPS           (100)
#endif

/***************************************************************************
Description:
    Color Matrix type for the Color Space conversion use in window, source,
    and display.

***************************************************************************/
typedef int32_t BVDC_ColorMatrix[BVDC_CSC_COEFF_COUNT];


/***************************************************************************
 * VDC enums
 **************************************************************************/

/* BVDC_HeapId: Not needed */

/***************************************************************************
Summary:
    This enumeration represents the different control mode.

Description:
    BVDC_Mode is an enumeration which represents different ways to control
    VDC internal states or options.

    BVDC_Mode_eAuto: internal VDC will decide which option to use base on
    best offort

    BVDC_Mode_eOff : forcefully turn off the control
    BVDC_Mode_eOn  : forcefully turn on the control

See Also:
***************************************************************************/
typedef enum
{
    BVDC_Mode_eAuto = 0,
    BVDC_Mode_eOff,
    BVDC_Mode_eOn
} BVDC_Mode;

/***************************************************************************
Summary:
    This enumeration represents the different compositor.

Description:
    BVDC_CompositorId is an enumeration which represents the many compositors
    that is repsonsible for blending video windows, graphics windows, and
    background into one video signal (BVB) that is sent to VEC for formating
    (the number of compositor is chip dependence).  The video signal (input
    to VEC) which then get formatted and send to variety of output destinations
    like DACs, DVI, ITU-R-656, and RF-Modulator.

    Output signals from compositor to VEC are NOT created equal.  Some signals
    can go to any output destinations, but some are restricted.  For example,
    BVDC_CompositorId_eCompositor2 (7038/3560) singals can only be sent to
    ITU-R-656 destination.

See Also:
    BVDC_Compositor_Create, BVDC_Compositor_Destroy.
***************************************************************************/
typedef enum
{
    BVDC_CompositorId_eCompositor0 = 0,
    BVDC_CompositorId_eCompositor1,
    BVDC_CompositorId_eCompositor2,
    BVDC_CompositorId_eCompositor3,
    BVDC_CompositorId_eCompositor4,
    BVDC_CompositorId_eCompositor5,
    BVDC_CompositorId_eCompositor6,
    BVDC_CompositorId_eCompositorMax = BVDC_CompositorId_eCompositor6
} BVDC_CompositorId;

/***************************************************************************
Summary:
    This enumeration represents the different window id.

Description:
    BVDC_WindowId is an enumeration which represents the many windows.
    (the number of window is chip dependence).  Note that window id is in
    compositor scope.

    Window are NOT created equal.  Some window has different capability, eg
    some will have MAD (Motion Adaptive Deinterlacer),
    PEP (Picture Enhancement Processing), and different
    RTS (Realtime Scheduling System) Resettings for memory access bandwidth.
    Application needs to specify what window to create by passing the window id
    to BVDC_Window_Create.

    BVDC_WindowId_eVideo0 -
        This represent the first Video Window on a given compositor.  It
        normally equip with more capability, and commonly use as Main or
        fullscreen window.

    BVDC_WindowId_eVideo1 -
        This represent the second Video Window on a given compositor.  It
        normally equip with less capability, and commonly use as PIP.  Usually
        more restriced of features.  In some chipset it also restricted to
        access certains memory controller, hence when creating window with
        this ID application must pass the correct BVDC_Heap_Handle.

    BVDC_WindowId_eGfx0 -
        This represent the first Graphics Window on a given compositor.

    BVDC_WindowId_eGfx1 -
        This represent the second Graphics Window on a given compositor.

    BVDC_WindowId_eGfx2 -
        This represent the third Graphics Window on a given compositor.

    BVDC_WindowId_eAuto -
        This represent that the window ID will be internally compute by VDC;
        For multiple heap usage, and shared memory, and BVDC_WindowId_eGfx1,
        BVDC_WindowId_eGfx2, user must explicity specify a window ID.

    Note that BVDC_WindowId_eGfxN does necessary match up with
    BAVC_SourceId_eGfxN.

See Also:
    BVDC_Window_Create, BVDC_window_Destroy, BAVC_SourceId.
***************************************************************************/
typedef enum
{
    BVDC_WindowId_eVideo0 = 0,
    BVDC_WindowId_eVideo1,
    BVDC_WindowId_eGfx0,
    BVDC_WindowId_eGfx1,
    BVDC_WindowId_eGfx2,
    BVDC_WindowId_eAuto

} BVDC_WindowId;

/* BVDC_SourceId: See aslo BAVC_SourceId */

/***************************************************************************
Summary:
    This enumeration represents the different display.

Description:
    BVDC_DisplayId is an enumeration which represents the many displays
    that is repsonsible for taken the output of compositore and formatted
    to the output port HDMI, DACs (CVBS, SVideo, YPrPb), ITU-R656, etc.

    Each display ID corresponds to a display object, which associates
    with a HW vec path, for analog or digital output port, or encoder
    consumption.

    BVDC_DisplayId_eAuto - Specify that BVDC_Display_Create uses
    compositor ID as a VEC's ID in conjuntion with the bVecSwap flag in
    BVDC_Settings.  For example if application uses BVDC_BVDC_DisplayId_eAuto
    to create a Display with a CompositoreId_Compositor0, a corresponding
    BVDC_BVDC_DisplayId_eDisplay0.  See also BVDC_Settings for bVecSwap
    definitions.

See Also:
    BVDC_Settings, BVDC_Compositor_Create, BVDC_Display_Create,
    BVDC_Display_Destroy.
***************************************************************************/
typedef enum
{
    BVDC_DisplayId_eDisplay0 = 0,
    BVDC_DisplayId_eDisplay1,
    BVDC_DisplayId_eDisplay2,
    BVDC_DisplayId_eDisplay3,
    BVDC_DisplayId_eDisplay4,
    BVDC_DisplayId_eDisplay5,
    BVDC_DisplayId_eDisplay6,
    BVDC_DisplayId_eAuto

} BVDC_DisplayId;

/***************************************************************************
Summary:
    This enumeration represents the different display timing generators.

Description:
    BVDC_DisplayTg is an enumeration which represents the display timing
    generator that is repsonsible to drive the display timing. Each display
    needs a master Tg to drive its output timing and to generate the triggers;

    Note, when STG is the master TG of a disaply, that display cannot have
    any other slave TG.

    BVDC_DisplayTg_ePrimIt - Specify Primary Input Timing generator.

    BVDC_DisplayTg_eSecIt  - Specify Secondary Input Timing generator.

    BVDC_DisplayTg_eTertIt - Specify Tertiery Input Timing generator.

    BVDC_DisplayTg_e656Dtg - Specify 656 out Digital Timing Generator.

    BVDC_DisplayTg_eDviDtg - Specify DVI out Digital Timing Generator.

    BVDC_DisplayTg_eStg0   - Specify ViCE channel 0 out Simple Timing Generator.

    BVDC_DisplayTg_eStg1   - Specify ViCE channel 1 out Simple Timing Generator.

    BVDC_DisplayTg_eStg2   - Specify ViCE channel 2 out Simple Timing Generator.

    BVDC_DisplayTg_eStg3   - Specify ViCE channel 3 out Simple Timing Generator.

    BVDC_DisplayTg_eStg4   - Specify ViCE channel 4 out Simple Timing Generator.

    BVDC_DisplayTg_eStg5   - Specify ViCE channel 5 out Simple Timing Generator.

See Also:
    BVDC_Display_Settings, BVDC_Display_Create
***************************************************************************/
typedef enum
{
    BVDC_DisplayTg_ePrimIt = 0,
    BVDC_DisplayTg_eSecIt,
    BVDC_DisplayTg_eTertIt,
    BVDC_DisplayTg_eDviDtg,
    BVDC_DisplayTg_e656Dtg,
    BVDC_DisplayTg_eStg0,
    BVDC_DisplayTg_eStg1,
    BVDC_DisplayTg_eStg2,
    BVDC_DisplayTg_eStg3,
    BVDC_DisplayTg_eStg4,
    BVDC_DisplayTg_eStg5,
    BVDC_DisplayTg_eUnknown
} BVDC_DisplayTg;

#define BVDC_DisplayTg_eStg         BVDC_DisplayTg_eStg0

typedef enum
{
    BVDC_DisplayOutput_eComponent = 0,
    BVDC_DisplayOutput_eComposite,
    BVDC_DisplayOutput_eSVideo,
    BVDC_DisplayOutput_eDvo,
    BVDC_DisplayOutput_e656
} BVDC_DisplayOutput;

/***************************************************************************
Summary:
    This enumeration represents sync settings for the DVO output.

Description:
    BVDC_SyncPolarity_eSame - Does not change the polarity of DVO output
        Vertical Sync.

    BVDC_SyncPolarity_eInvert - Inverses the polarity of DVI Vertical Sync.

See Also:
    BVDC_Source_SetChromaExpansion
****************************************************************************/
typedef enum
{
    BVDC_SyncPolarity_eSame = 0,
    BVDC_SyncPolarity_eInvert

} BVDC_SyncPolarity;

/***************************************************************************
Summary:
    This enumeration represents the methods of expanding YCrCb 4:2:2 format
    to 4:4:4.

Description:

See Also:
    BVDC_Source_SetChromaExpansion
****************************************************************************/
typedef enum
{
    BVDC_ChromaExpansion_eLinearInterpolate  /* linearly interpolated */

} BVDC_ChromaExpansion;

/***************************************************************************
Summary:
    This enumeration represents the filter coefficient sets for horizontal
    scaling.

Description:
    Determine actual values for scale coefficients.

See Also:
    BVDC_Source_SetHorizontalScaleCoeffs
****************************************************************************/
typedef enum
{
    BVDC_FilterCoeffs_eAuto,            /* Select by internal algorithm */
    BVDC_FilterCoeffs_ePointSample,     /* Point sampled filtering */
    BVDC_FilterCoeffs_eBilinear,        /* Bilinear filtering */
    BVDC_FilterCoeffs_eAnisotropic,     /* Anisotropic filtering */
    BVDC_FilterCoeffs_eSharp            /* Tabled sin(x)/x filtering */

} BVDC_FilterCoeffs;

/***************************************************************************
Summary:
    This enumeration describes the Macrovision types

Description:
    BVDC_MacrovisionType contains all Macrovision standard types defined by
    Macrovision plus the certification test required types.

See Also:
    BVDC_Display_SetMacrovisionType, BVDC_Display_GetMacrovisionType
***************************************************************************/
typedef enum
{
    BVDC_MacrovisionType_eNoProtection = 0,   /* No copy protection */
    BVDC_MacrovisionType_eType1,
    BVDC_MacrovisionType_eType2,
    BVDC_MacrovisionType_eType3,
    BVDC_MacrovisionType_eCustomized,         /* User will provide table */
    BVDC_MacrovisionType_eType1_Rgb,          /* MV goes on RGB as well as
                                                 other formats. */
    BVDC_MacrovisionType_eType2_Rgb,          /* MV goes on RGB as well as
                                                 other formats. */
    BVDC_MacrovisionType_eType3_Rgb,          /* Mv goes on RGB as well as
                                                 other formats. */
    BVDC_MacrovisionType_eTest01,             /* MV certification test 01 */
    BVDC_MacrovisionType_eTest02              /* MV certification test 02 */
} BVDC_MacrovisionType;

/* Backwards compatibiliby */
#define BVDC_MacrovisionType_eAgcOnly       BVDC_MacrovisionType_eType1
#define BVDC_MacrovisionType_eAgc2Lines     BVDC_MacrovisionType_eType2
#define BVDC_MacrovisionType_eAgc4Lines     BVDC_MacrovisionType_eType3
#define BVDC_MacrovisionType_eAgcOnly_Rgb   BVDC_MacrovisionType_eType1_Rgb
#define BVDC_MacrovisionType_eAgc2Lines_Rgb BVDC_MacrovisionType_eType2_Rgb
#define BVDC_MacrovisionType_eAgc4Lines_Rgb BVDC_MacrovisionType_eType3_Rgb

/***************************************************************************
Summary:
    This enumeration describes the output color space

Description:
    The enum contains both HD and SD output color spaces, with comments which
    indicate the display format types that must be selected. Certain outputs
    are for HD or SD display formats only.

    7038 shares the Dacs between the display handles. When a user selects certain
    Dacs, it is reserved for a particular display handle.
    BVDC_DacOutput_eUnused means a Dac is not used, and available for use by
    other display handles. If all valid displays mark a Dac as unused, that Dac
    will be disabled by VDC.

See Also:
    BVDC_Display_SetDacConfiguration, BVDC_Display_GetDacConfiguration
***************************************************************************/
typedef enum
{
    /* For SD video formats ONLY! */
    BVDC_DacOutput_eSVideo_Luma = 0,
    BVDC_DacOutput_eSVideo_Chroma,
    BVDC_DacOutput_eComposite,

    /* SMPTE 274, SMPTE 276 */
    BVDC_DacOutput_eRed,
    BVDC_DacOutput_eGreen,
    BVDC_DacOutput_eBlue,

    /* BT.601 (480i,Pal,480p) */
    /* BT.709 (1080i,720p) */
    BVDC_DacOutput_eY,
    BVDC_DacOutput_ePr,
    BVDC_DacOutput_ePb,

    /* Hsync on the DAC output for DTV application which requires separate Hsync
       and Vsync along with RGB outputs; Hsync DAC output can only be set on
       display 0; when Hsync DAC output is selected, no Vsync signal can be
       carried on the DAC outputs, and the Vsync would automatically be exported
       to the EXT_SYNC0 pinout (please refer to RDB MISC_OUT_MUX register); when
       Hsync is selected, no CVBS or SVideo can be set as outputs; */
    BVDC_DacOutput_eHsync,

    /* SCART: G channel might carry no sync; */
    BVDC_DacOutput_eGreen_NoSync,

    /* DAC is configured to take IFD signals directly.  The input is configured
     * through the IFD specified. Valid source format is BVDC_P_FormatInfo->bSD
     * (Standard Definition NTSC/PAL/etc interlaced). */
    BVDC_DacOutput_eIfdm0,
    BVDC_DacOutput_eIfdm1,

    /* DAC is configured to take output that go thru the Group Delay and Audio
     * Trap" filters aka "GRPD_x".
     * this DAC can then be use with external RFM module. */
    BVDC_DacOutput_eFilteredCvbs,

    BVDC_DacOutput_eUnused              /* Not used */

} BVDC_DacOutput;

/***************************************************************************
Summary:
    This enumeration describes the RFM output

Description:
    7038 supports 1 RF Modulator port. Users can select CVBS output from the
    Vec to the RF port, or a constant value. BVDC_RfmOutput_eUnused will
    disable Rfm output.

See Also:
    BVDC_Display_SetRfmConfiguration, BVDC_Display_GetRfmConfiguration
***************************************************************************/
typedef enum
{
    BVDC_RfmOutput_eCVBS = 0,   /* CVBS output to RF port */
    BVDC_RfmOutput_eConstant,   /* Constant value to RF port */
    BVDC_RfmOutput_eUnused      /* Not used */

} BVDC_RfmOutput;

/***************************************************************************
Summary:
    This enumeration describes the video mute mode.

Description:
    This function can be used for smooth channel change to display a user
    defined picture instead of a garbage picture.

    There are 3 posssible modes:
        BVDC_MuteMode_eDisable:
            Display the normal input video. This can be used to un-mute.
        BVDC_MuteMode_eConst:
            Display a const color defined by user.
        BVDC_MuteMode_eRepeat:
            Repeat the last valid picture

See Also:
    BVDC_Source_SetMuteMode
    BVDC_Source_GetMuteMode
    BVDC_Source_SetVideoMuteColor
    BVDC_Source_GetVideoMuteColor
***************************************************************************/
typedef enum
{
    BVDC_MuteMode_eDisable = 0,
    BVDC_MuteMode_eConst,
    BVDC_MuteMode_eRepeat

} BVDC_MuteMode;

/***************************************************************************
Summary:
    This enumeration represents the resume mode.

Description:
    This enumeration represents how a module is resumed after it is auto-
    muted by BVDC.

See Also:
    BVDC_Source_SetResumeMode
****************************************************************************/
typedef enum
{
    BVDC_ResumeMode_eAuto = 0,        /* resume automatically by VDC */
    BVDC_ResumeMode_eManual       /* resume as user instructed */

} BVDC_ResumeMode;

/***************************************************************************
Summary:
    This enumeration represents the interface types that support MPAA
    decimation.

Description:
    BVDC_MpaaDeciIf is an enumeration which represents the output interface
    types that support MPAA decimation. When MPAA decimation is enabled,
    significant amount of pixels in the output video signal are replaced
    with values interpolated from neighborhood pixels, and therefore the
    video quality is lowered.

See Also:
    BVDC_Display_SetMpaaDecimation, BVDC_Display_GetMpaaDecimation
***************************************************************************/
typedef enum
{
    BVDC_MpaaDeciIf_eHdmi = 0,  /* digital HD/SD output interface */
    BVDC_MpaaDeciIf_eComponent, /* analog component HD/SD output interface */
    BVDC_MpaaDeciIf_eUnused     /* Not used */

} BVDC_MpaaDeciIf;

/***************************************************************************
Summary:
    This enumeration represents blend factor for each window.

Description:
    BVDC_BlendFactor is an enumeration which represents many blend factor
    that are being used for blending a video window or a graphics window
    onto the destination picture.  The following equation is used for the
    blending.

       DstComponent = (SrcComponent * SrcBlendFactor) +
                      (DstComponent * DstBlendFactor)

See Also:
    BVDC_Window_SetBlendFactor, BVDC_Window_GetBlendFactor
***************************************************************************/
typedef enum
{
    BVDC_BlendFactor_eZero = 0,                /* 0 */
    BVDC_BlendFactor_eOne,                     /* 1 */
    BVDC_BlendFactor_eSrcAlpha,                /* Alpha Src */
    BVDC_BlendFactor_eOneMinusSrcAlpha,        /* 1 - (Alpha Src) */
    BVDC_BlendFactor_eConstantAlpha,           /* Constant Alpha */
    BVDC_BlendFactor_eOneMinusConstantAlpha    /* 1 - (Constant Alpha) */

} BVDC_BlendFactor;

/***************************************************************************
Summary:
    Contains pan scan type

Description:
    Defines how the size and position of the scan out rectangle of the feeder
    is calculated.

    When pan scan is set to BVDC_PanScanType_eDisable, pan scan will be
    disabled and the source rectangle will be unmodified. For MPEG sources,
    this means the source rectangle will be defined by the
    ulSourceHorizontalSize and ulSourceVerticalSize parameters from the
    BAVC_MVD_Field structure.

    When pan scan is not set to BVDC_PanScanType_eDisable, the source
    rectangle will be determined by the ulDisplayHorizontalSize and
    ulDisplayHorizontalSize parameters from the BAVC_MVD_Field structure.
    This new rectangle, referred to as the pan scan rectangle, will be
    initally centered within the original source rectangle and may be adjusted
    by additional pan scan vectors defined by BVDC_PanScanType.

    Whether pan scan is enabled or not, BVDC_Window_SetSrcClip() will
    continue to operate on the resulting rectangle. So when pan-scan is
    enabled, the source clipping will occur on the pan scan rectangle and
    not the original source rectangle.

    When pan scan is set to BVDC_PanScanType_eStream, the stream pan scan
    vector specified by i16_HorizontalPanScan and i16_VerticalPanScan in the
    BAVC_MVD_Field structure will be used to adjust the position of the source
    pan scan rectangle.

    When pan scan is set to BVDC_PanScanType_eUser, the user pan scan vector
    will be used to adjust the source pan scan rectangle.

    When pan scan is set to BVDC_PanScanType_eStreamAndUser, both stream
    and user pan scan vectors will be used to adjust the source pan scan
    rectangle.

***************************************************************************/
typedef enum
{
    BVDC_PanScanType_eDisable = 0,      /* Disable all pan scan.
                                         * Source rectangle is unmodified */
    BVDC_PanScanType_eStream,           /* Use stream pan scan vector to adjust
                                         * source pan scan rectangle */
    BVDC_PanScanType_eUser,             /* Use user pan scan vector to adjust
                                         * source pan scan rectangle */
    BVDC_PanScanType_eStreamAndUser     /* Use both stream and user pan scan vector
                                         * to adjust source pan scan rectangle */
} BVDC_PanScanType;


/***************************************************************************
Summary:
    This enum describe the desire aspect ratio correction modes.

Description:
    - BVDC_AspectRatioMode_eBypass
        Aspect ratio correction is not performed. Use settings of source clip
        scale and destination rectangles as set with BVDC_Window_SetSrcClip,
        BVDC_Window_SetScalerOutput, and BVDC_Window_SetDestRect. Rectangles
        set with the _isr versions of these functions will be ignored.

    - BVDC_AspectRatioMode_eBypass_isr
        Deprecated.

    - BVDC_AspectRatioMode_eUseAllSource (aka Box)
        Aspect ratio correction is performed on top of the user set rectangles.
        The aspect ratio of the user clipped source area is kept intact, and
        the area (not the pixel numbers) is squarely scaled to fill the
        destination rectangle area as much as possible and the empty areas are
        patched with black bars. That means the scaler-out rectangle and
        destination rectangle will be shrunken in one direction. Full source of
        4x3 on 16x9 will be shown as pillar box with black bars on the sides.
        Full source of 16x9 on 4x3 will be shown as letterbox with black bars
        on top and bottom. Full source of 4x3 on 4x3 and full source of 16x9 on
        16x9 will both be shown fullscreen with no black bars. Rectangles set
        with the _isr versions of functions will be ignored.

    - BVDC_AspectRatioMode_eUseAllDestination (aka Zoom)
        Aspect ratio correction is performed on top of the user set rectangles.
        The user defined clipped source area is further clipped in one direction
        so that it has the same aspect ratio as the destination window area. The
        clipped source area (not the pixel numbers) is then squarely scaled to
        fit the whole destination window area. Full source of 4x3 on 16x9 will
        result in source clipping in top and bottom. Full source of 16x9 on 4x3
        will result in left and right source clipping. Full source of 4x3 on 4x3
        and full source of 16x9 on 16x9 will be displayed without adjustment.
        Rectangles set with the _isr versions of functions will be ignored.

See Also:
    BVDC_Window_SetAspectRatioMode
***************************************************************************/
typedef enum
{
    BVDC_AspectRatioMode_eBypass = 0,
    BVDC_AspectRatioMode_eBypass_isr,
    BVDC_AspectRatioMode_eUseAllSource,
    BVDC_AspectRatioMode_eUseAllDestination

} BVDC_AspectRatioMode;

/***************************************************************************
Summary:
    Specifies various modes for handling AFD (Active Format Descriptor).

Description:
    This enumeration specifies different modes for handling AFD (Active
    Format Descriptor).  AFD is value transmitted in the stream's user data
    that can be use to clip away unwanted content such as black vertical or
    horizontal bars; or even non-important contents.  The specification of
    clipping is base on AFD specs.

    BVDC_AfdMode_eDisabled - This mode indicates that NO clipping will be
    performed even if the stream contains the AFD value.

    BVDC_AfdMode_eStream - This mode indicates that clipping will be
    performed if the stream contains the AFD value.

    BVDC_AfdMode_eUser - This mode indicates that clipping will be
    performed with the value user specifies, and ignore the value from stream.

See Also:
    BVDC_Window_SetAfdSettings, BVDC_Window_GetAfdSettings
***************************************************************************/
typedef enum
{
    BVDC_AfdMode_eDisabled = 0,
    BVDC_AfdMode_eStream,
    BVDC_AfdMode_eUser

} BVDC_AfdMode;

/***************************************************************************
Summary:
    Specifies various clipping modes to be use with AFD.

Description:
    This enumeration specifies different amount of clipping when AFD is enabled.

    BVDC_AfdClip_eNominal - This mode clips nominal.  This will clip away the
    black content in the stream base on AFD value.

    BVDC_AfdClip_eOptionalLevel1 - This mode clips away the BVDC_AfdClip_eNominal
    content + optional contents at level1.

    BVDC_AfdClip_eOptionalLevel2 - This mode clips away the BVDC_AfdClip_eNominal
    content + optional contents at level2.

See Also:
    BVDC_Window_SetAfdSettings, BVDC_Window_GetAfdSettings
***************************************************************************/
typedef enum
{
    BVDC_AfdClip_eNominal = 0,
    BVDC_AfdClip_eOptionalLevel1,
    BVDC_AfdClip_eOptionalLevel2

} BVDC_AfdClip;

/***************************************************************************
Summary:
    Contains the split screen demo modes

Description:
    These enumerations are used into the demo mode to disable or enable
    one side of the split screen.

See Also:
    BVDC_SplitScreenSettings
    BVDC_Window_SetSplitScreen
***************************************************************************/
typedef enum
{
    BVDC_SplitScreenMode_eDisable = 0,     /* disable split screen */
    BVDC_SplitScreenMode_eLeft,            /* enable the left split screen */
    BVDC_SplitScreenMode_eRight            /* enable the right split screen */

} BVDC_SplitScreenMode;


/***************************************************************************
Summary:
    Contains the color clip modes

Description:
    Modes set for color clip.  Mode none is used to disable color clip.
    and the other 3 modes are used to enable color clip for B0.  In C0, there
    will be distinct operation per mode.
***************************************************************************/
typedef enum
{
    BVDC_ColorClipMode_None = 0,     /* Disable color clip          */
    BVDC_ColorClipMode_White,        /* Enable color clip for White */
    BVDC_ColorClipMode_Black,        /* Enable color clip for Black */
    BVDC_ColorClipMode_Both          /* Enable color clip for both  */

} BVDC_ColorClipMode;

/***************************************************************************
Summary:
    Contains the ANR/DNR filter mode

Description:
    ANR modes.

    BVDC_FilterMode_eDisable - Noise filter module (ANR/DNR/etc) is completely
    disabled and is taken off from BVN VNET;

    BVDC_FilterMode_eBypass - Noise filter module (ANR/DNR/etc) is connected
    into BVN VNET, but the video pixels are bypassed from noise filter module
    without any filtering;

    BVDC_FilterMode_eEnable, Noise filter module (ANR/DNR/etc) is connected
    into BVN VNET and the video pixels are filtered.

See Also:
    BVDC_Anr_Settings, BVDC_Window_SetAnrConfiguration, BVDC_Window_GetAnrConfiguration.
    BVDC_Dnr_Settings, BVDC_Source_GetDnrConfiguration, BVDC_Source_SetDnrConfiguration
***************************************************************************/
typedef enum
{
    BVDC_FilterMode_eDisable = 0,
    BVDC_FilterMode_eBypass,
    BVDC_FilterMode_eEnable

} BVDC_FilterMode;

/***************************************************************************
Summary:
    This enumeration represents the different vnet modules of a video window.

Description:
    After inputted from its source (such as MFD or HDMI input) and before
    ouputted to the compositor, a video window's pictures go through a series
    of video processing modules. Those modules (including the source and
    compositor) are connected by VNET, and are called the vnet modules of this
    window.

    A vnet module might be before, at, or after the CAP. The vnet part before
    and including CAP is called writer part, and rest is called reader part.

    Which specific hardware module of the same type (such as SCL_0, or SCL_1
    of BVDC_VnetModule_eScl type) are used for this window, are decided by
    VDC internally.

    BVDC_VnetModule_eSrc -
        This represents the input source of the video window, such as MPEG
        feeder or HDMI input.

    BVDC_VnetModule_eVfd -
        This represents the video feeder of the video window.

    BVDC_VnetModule_eDnr -
        This represents the DNR of the video window.

    BVDC_VnetModule_eMad -
        This represents the MVP or MAD of the video window. MVP typically contains
        sub-modules HSCL, ANR, MCDI or MADR inside. MVD as a whole is considered
        as one vnet module. Its sub-modules are not.

    BVDC_VnetModule_eScl -
        This represents the SCL of the video window.

    BVDC_VnetModule_eCap -
        This represents the video capturer of the video window.

See Also:
    BVDC_Window_CallbackSettings
    BVDC_Window_CallbackData
    BVDC_Window_InstallCallback
    BVDC_Window_SetCallbackSettings
    BVDC_Window_GetCallbackSettings
***************************************************************************/
typedef enum
{
    BVDC_VnetModule_eSrc = 0,
    BVDC_VnetModule_eVfd,
    BVDC_VnetModule_eDnr,
    BVDC_VnetModule_eMad,
    BVDC_VnetModule_eScl,
    BVDC_VnetModule_eCap
} BVDC_VnetModule;

/***************************************************************************
Summary:
    This enumeration describes MAD Game Mode options.

Description:
    MAD game modes try to provide less video processing delay, or display latency to suit
    the game display requirement. It could also be used to reduce memory bandwidth
    and/or memory footprint requirements for different profiles of DTV products.

    BVDC_MadGameMode_eOff is the normal mode with 3-field delay; best quality so far;
    This mode supports all MAD features, including 3:2 and 2:2 reverse pulldown; currently
    it requires 5-field buffer allocations.

    MAD game modes are divided into three groups according to the required field buffer
    allocations.

    The group of MAD game modes with 5-field buffer allocations are:

        e5Fields_2Delay
             Not supported now;
        e5Fields_1Delay
             1-field delay; better quality;
             This mode could support inverse telecine(3:2/2:2) but not now;
        e5Fields_0Delay
             0-field delay; good quality;
             Thise mode uses motion detection but can't support reverse pulldown;
        e5Fields_ForceSpatial
             The continuous HardStart mode with 0-field delay and spatial-only filtering;
             might see vertical jitter with static sharp content due to no motion detection
             but more responsive to fast motion;

    The group of MAD game modes with 4-field buffer allocations is:
        e4Fields_1Delay
             1-field delay; good quality;
             1-field delay; moderate quality with limited motion detection;
             supports inverse telecine(3:2/2:2);
        e4Fields_0Delay
             0-field delay; good quality;
        e4Fields_ForceSpatial
             0-field delay and spatial-only filtering; may see vertical jitter with static
             sharp content due to no motion detection but more responsive to fast motion;

    The group of MAD game modes with 3-field buffer allocations are:

        e3Fields_2Delay
             Not supported now;
        e3Fields_1Delay
             1-field delay; moderate quality with limited motion detection;
             supports inverse telecine(3:2/2:2);
        e3Fields_0Delay
             0-field delay; reduced quality with limited motion detection;
             can't support inverse telecine;
        e3Fields_ForceSpatial
             0-field delay and spatial-only filtering; may see vertical jitter with static
             sharp content due to no motion detection but more responsive to fast motion;

    The group of MAD game modes with 0/1-field buffer allocations are:

        eMinField_ForceSpatial
             0/1 -field delay and spatial-only filtering. 0 OR 1 buffer needed,
             depending on MCVP (1 buffer) or MADR (0 buffer)

    Note:
    1) when user switches MAD game mode between different buffer allocation groups, VDC will reconfig
    BVN, so blank of screen; but if game mode switch occurs within the same MAD buffer allocation
    group, VDC won't reconfig BVN, therefore no blank screen;

    2) The 3-field group of modes are intended to be used on the low memory bandwidth small
    memory footprint system;

    3) BVDC_MadGameMode_e4Fields_1Delay and BVDC_MadGameMode_e4Fields_ForceSpatial modes are NOT
    supported at the moment.

    4) For chipsets with MCDI core, normal mode and all game modes currently take 4-field buffers.
    BVDC_MadGameMode_eOff introduces 2-field delay. BVDC_MadGameMode_e5Fields_1Delay,
    BVDC_MadGameMode_e4Fields_1Delay and BVDC_MadGameMode_e3Fields_1Delay introduce 1-field delay.
    The rest modes have no delay.

See Also:
    BVDC_Deinterlace_Settings, BVDC_Window_SetDeinterlaceConfiguration,
    BVDC_Window_GetDeinterlaceConfiguration.
***************************************************************************/
typedef enum
{
    BVDC_MadGameMode_eOff = 0,             /* Disable MAD Game Mode, i.e. the normal mode */
    BVDC_MadGameMode_e5Fields_2Delay,      /* Mad Game Mode 5 Active Pixel Fields, 2 Field Delay */
    BVDC_MadGameMode_e5Fields_1Delay,      /* Mad Game Mode 5 Active Pixel Fields, 1 Field Delay */
    BVDC_MadGameMode_e5Fields_0Delay,      /* Mad Game Mode 5 Active Pixel Fields, 0 Field Delay */
    BVDC_MadGameMode_e5Fields_ForceSpatial,/* MAD Game Mode spatial-only with 5-field buffer allocation */
    BVDC_MadGameMode_e4Fields_2Delay,      /* Mad Game Mode 4 Active Pixel Fields, 2 Field Delay */
    BVDC_MadGameMode_e4Fields_1Delay,      /* Mad Game Mode 4 Active Pixel Fields, 1 Field Delay */
    BVDC_MadGameMode_e4Fields_0Delay,      /* Mad Game Mode 4 Active Pixel Fields, 0 Field Delay */
    BVDC_MadGameMode_e4Fields_ForceSpatial,/* Mad Game Mode spatial-only with 4-field buffer allocation */
    BVDC_MadGameMode_e3Fields_2Delay,      /* Mad Game Mode 3 Active Pixel Fields, 2 Field Delay */
    BVDC_MadGameMode_e3Fields_1Delay,      /* Mad Game Mode 3 Active Pixel Fields, 1 Field Delay */
    BVDC_MadGameMode_e3Fields_0Delay,      /* Mad Game Mode 3 Active Pixel Fields, 0 Field Delay */
    BVDC_MadGameMode_e3Fields_ForceSpatial,/* Mad Game Mode spatial-only with 3-field buffer allocation */
    BVDC_MadGameMode_eMinField_ForceSpatial,/* Mad Game Mode spatial-only with 0/1-field buffer allocation */

    BVDC_MadGameMode_eMaxCount
} BVDC_MadGameMode;

/* To be backwards compatible */
#define BVDC_MadGameMode_eHardStart            BVDC_MadGameMode_e5Fields_ForceSpatial
#define BVDC_MadGameMode_e5Fields              BVDC_MadGameMode_e5Fields_1Delay
#define BVDC_MadGameMode_e3Fields              BVDC_MadGameMode_e3Fields_1Delay
#define BVDC_MadGameMode_e3Fields_LowBandwidth BVDC_MadGameMode_e3Fields_1Delay

/***************************************************************************
Summary:
    This enumeration describes MAD chroma motion method.

Description:
***************************************************************************/
typedef enum
{
    /* Chroma 422 or 420 motion mode: cross chroma aware typical for analog */
    BVDC_Deinterlace_ChromaMotionMode_eXchromaAware  = 0,

    /* Chroma 422 or 420 motion mode: digital chroma */
    BVDC_Deinterlace_ChromaMotionMode_eDigitalChroma

} BVDC_Deinterlace_ChromaMotionMode;


/***************************************************************************
Summary:
    This enumeration describes MAD chroma field 420 interpolation method.

Description:
***************************************************************************/
typedef enum
{
    /* Chroma 420 simultaneous-phase mode interpolation */
    BVDC_Deinterlace_Chroma420InvMethod_eSim  = 0,

    /* Chroma 420 poly-phase mode interpolation */
    BVDC_Deinterlace_Chroma420InvMethod_ePoly

} BVDC_Deinterlace_Chroma420InvMethod;


/***************************************************************************
Summary:
    This enumeration describes MAD chroma 422 motion adaptive processing mode.

Description:
***************************************************************************/
typedef enum
{
    /* Chroma 422 motion adaptive processing mode */
    BVDC_Deinterlace_Chroma422MaMode_eMotionAdaptive  = 0,

    /* Interpolation Chroma 420 processing mode */
    BVDC_Deinterlace_Chroma422MaMode_eInt420

} BVDC_Deinterlace_Chroma422MaMode;


/***************************************************************************
Summary:
    This enumeration describes MAD chroma 422 inverse telecine processing mode.

Description:
***************************************************************************/
typedef enum
{
    /* Chroma 422 motion adaptive processing mode */
    BVDC_Deinterlace_Chroma422ItMode_eMotionAdaptive = 0,

    /* Interpolation Chroma 420 processing mode */
    BVDC_Deinterlace_Chroma422ItMode_eInt420,

    /* Inverse telecine Chroma 420 processing mode */
    BVDC_Deinterlace_Chroma422ItMode_eItXchroma,

    /* Inverse Telecine direct weave chroma processing mode */
    BVDC_Deinterlace_Chroma422ItMode_eItDirect

} BVDC_Deinterlace_Chroma422ItMode;


/***************************************************************************
Summary:
    This enumeration describes MAD temporal motion modes.

Description:
***************************************************************************/
typedef enum
{
    BVDC_Deinterlace_TmMode_eNormal = 0,
    BVDC_Deinterlace_TmMode_eBoost1,
    BVDC_Deinterlace_TmMode_eBoost2

} BVDC_Deinterlace_TmMode;


/***************************************************************************
Summary:
    This enumeration describes MAD spatial motion modes.

Description:
***************************************************************************/
typedef enum
{
    BVDC_Deinterlace_SmMode_eNormal = 0,
    BVDC_Deinterlace_smMode_eBoost1

} BVDC_Deinterlace_SmMode;

/***************************************************************************
Summary:
    This enumeration describes input data bus mode for HD_DVI.

Description:
***************************************************************************/
typedef enum
{
    BVDC_HdDvi_InputDataMode_e36Bit = 0,
    BVDC_HdDvi_InputDataMode_e24Bit,
    BVDC_HdDvi_InputDataMode_e30Bit

} BVDC_HdDvi_InputDataMode;

/***************************************************************************
Summary:
    This enumeration describes the ringing suprresion modes during the
    422 <-> 444 conversion.

    BVDC_RingSuppressionMode_eDisable - Ring suppression mode is disable
    during the converions.

    BVDC_RingSuppressionMode_eNormal - Ring suppression mode is enable
    during the converions.

    BVDC_RingSuppressionMode_eDouble - Ring suppression mode is enable
    during the converions, and double means further suppressed.

Description:
***************************************************************************/
typedef enum
{
    BVDC_RingSuppressionMode_eDisable = 0,
    BVDC_RingSuppressionMode_eNormal,
    BVDC_RingSuppressionMode_eDouble

} BVDC_RingSuppressionMode;

/***************************************************************************
Summary:
    This enumeration describes 422 -> 444 conversion filter being use.

    BVDC_422To444Filter_eLinear - Linear interpolation filter is being used.

    BVDC_422To444Filter_eSixTaps - 6-Taps filter is being used.

    BVDC_422To444Filter_eChromaDuplication - Chroma duplication is being used.

    BVDC_422To444Filter_eTenTaps - 10-Taps filter is being used.

Description:
***************************************************************************/
typedef enum
{
    BVDC_422To444Filter_eLinear = 0,
    BVDC_422To444Filter_eSixTaps,
    BVDC_422To444Filter_eChromaDuplication,
    BVDC_422To444Filter_eTenTaps

} BVDC_422To444Filter;

/***************************************************************************
Summary:
    This enumeration describes 444 -> 422 conversion filter being use.

    BVDC_444To422Filter_e12221 - Down Sampler applies 12221 filter for
    converting to 422 Chroma samples.

    BVDC_444To422Filter_e14641 - Down Sampler applies 14641 filter for
    converting to 422 Chroma samples.

    BVDC_444To422Filter_eDecimate - Down Sampler drops alternate 444
    Chroma samples.

    BVDC_444To422Filter_eStandard - Down Sampler applies standard filter
    for converting to 422 Chroma samples.

Description:
***************************************************************************/
typedef enum
{
    BVDC_444To422Filter_e12221 = 0,
    BVDC_444To422Filter_e14641,
    BVDC_444To422Filter_eDecimate,
    BVDC_444To422Filter_eStandard

} BVDC_444To422Filter;

/***************************************************************************
Summary:
    This enumeration describes the number of bins used for histogram.

Description:
***************************************************************************/
typedef enum
{
    BVDC_HistBinSelect_e16_Bins = 0,
    BVDC_HistBinSelect_e32_Bins,
    BVDC_HistBinSelect_e64_Bins,
    BVDC_HistBinSelect_eInvalid

} BVDC_HistBinSelect;

/***************************************************************************
Summary:
    This enumeration describes the chroma histogram types

Description:
***************************************************************************/
typedef enum
{
    BVDC_ChromaHistType_eHueSat = 0,
    BVDC_ChromaHistType_eCrCb,
    BVDC_ChromaHistType_eInvalid
} BVDC_ChromaHistType;

/***************************************************************************
Summary:
    This structure describes 422 -> 444 up sampler controls.

    bUnbiasedRound - Perform conditional round up or down in 422to444
    conversion result.  Applies when output's fractional bits equal to the
    mid-point (0.5) of LSB bit.

    eFilterType - Filter to use for the conversion.

    eRingRemoval - Ring suppression mdoe for the conversion.

Description:
***************************************************************************/
typedef struct
{
    bool                               bUnbiasedRound;
    BVDC_422To444Filter                eFilterType;
    BVDC_RingSuppressionMode           eRingRemoval;

} BVDC_422To444UpSampler;

/***************************************************************************
Summary:
    This structure describes 444 -> 422 down sampler controls.

    eFilterType - Filter to use for the conversion.

    eRingRemoval - Ring suppression mdoe for the conversion.

Description:
***************************************************************************/
typedef struct
{
    BVDC_444To422Filter                eFilterType;
    BVDC_RingSuppressionMode           eRingRemoval;

} BVDC_444To422DnSampler;

/***************************************************************************
Description:
    The following describes maximums for the input mux count in HDDVI
***************************************************************************/
#define BVDC_MAX_HDDVI_MUX_COUNT                 (36)

/***************************************************************************
Description:
    The following describes maximums for the display count in VDC
***************************************************************************/
#define BVDC_MAX_DISPLAYS                         (7)

/***************************************************************************
Description:
    The following describes maximums for the video window count per display
***************************************************************************/
#define BVDC_MAX_VIDEO_WINDOWS                    (2)

/***************************************************************************
Description:
    The following describes maximums for the memory controller count
***************************************************************************/
#define BVDC_MAX_MEMC                             (3)

/***************************************************************************
Summary:
    This enumerates display Vsync alignment direction.

Description:
    This enum indicates which way to adjust the display clock. eFaster means adjust display's
    clock to run faster; eSlower means to adjust display's clock to run slower; eAuto means
    VDC will decide the alignment direction. The purpose is to achieve display alignment with
    another display.

    Note in eAuto mode, if the two displays are both interlaced formats, VDC will align them
    such that their top/bottom cadences match, i.e., top field of one display is aligned with
    the top field of the other display; if either display is in progressive format, VDC will align
    them in the quicker convergence way, i.e. if the delta between the two displays Vsyncs
    is 0.2 frame looking forward and 0.8 frame looking backward, then VDC will speed up the
    display clock to align forward.

    In practice, it's strongly recommended that eAuto mode is used when two displays are
    both in interlaced formats to have favorable lipsync result.

See Also:
    BVDC_Display_AlignmentSettings
***************************************************************************/
typedef enum
{
    BVDC_AlignmentDirection_eAuto = 0,
    BVDC_AlignmentDirection_eFaster,
    BVDC_AlignmentDirection_eSlower
} BVDC_AlignmentDirection;

/***************************************************************************
Summary:
    This enumerates the user preferred scaler and capture position when the
    result of bandwidth equation is in the decision oscillation region.

Description:
    When the result of bandwidth equation for determining scaler and
    capture position falls in the decision oscillation range, VDC will place
    scaler and capture according to the specified preference.

See Also:
    BVDC_Window_SetBandwidthEquationParams
    BVDC_Window_GetBandwidthEquationParams

***************************************************************************/
typedef enum
{
    BVDC_SclCapBias_eAuto = 0,
    BVDC_SclCapBias_eSclBeforeCap,
    BVDC_SclCapBias_eSclAfterCap
} BVDC_SclCapBias;


/***************************************************************************
Summary:
    This enumerate specifies the SCART function select type

Description:
    In SCART mode the VDEC will return the function select encoded in the video
    signal.  This the type return by PI.

See Also:
***************************************************************************/
typedef enum
{
    BVDC_ScartFuncSelect_eNoSignal = 0,
    BVDC_ScartFuncSelect_eAspectRatio_16_9,
    BVDC_ScartFuncSelect_eAspectRatio_4_3,
    BVDC_ScartFuncSelect_eAspectRatio_Invalid

} BVDC_ScartFuncSelect;

/***************************************************************************
Summary:
    This enumerate specifies the source buffer selection in 3D.

Description:
    BVDC_3dSourceBufferSelect_eNormal:
        This is the default normal mode for 3d: Left buffer from source goes
        to left buffer in display, right buffer from source goes to right buffer
        in display.
    BVDC_3dSourceBufferSelect_eLeftBuffer
        This is a special case in 3d: Left buffer from source goes to both
        left and right buffer in display.
    BVDC_3dSourceBufferSelect_eRightBuffer
        This is a special case in 3d: right buffer from source goes to both
        left and right buffer in display.

See Also:
***************************************************************************/
typedef enum
{
    BVDC_3dSourceBufferSelect_eNormal = 0,
    BVDC_3dSourceBufferSelect_eLeftBuffer,
    BVDC_3dSourceBufferSelect_eRightBuffer

} BVDC_3dSourceBufferSelect;

/***************************************************************************
Summary:
    This enumerate specifies the states of Dacs connection

Description:
    BVDC_DacConnectionState_eUnknown:
        This is the state when no detection available.  Could be because
        Dac is not connected or cable detection is disabled.
    BVDC_DacConnectionState_eConnected:
        This is the state when cable is plugged in to the Dac.
    BVDC_DacConnectionState_eDisconnected:
        This is the state when cable is not plugged in to the Dac.

See Also:
***************************************************************************/
typedef enum
{
    BVDC_DacConnectionState_eUnknown,
    BVDC_DacConnectionState_eConnected,
    BVDC_DacConnectionState_eDisconnected,
    BVDC_DacConnectionState_eMax
} BVDC_DacConnectionState;

/***************************************************************************
Summary:
    This enumerate specifies the CRC Type for MPEG source

Description:
    BVDC_Source_CrcType_eCrc32:
        This is the default mode for non HEVC inputs. Supports both HEVC and
        non HEVC inputs. Get one 32-bit CRC value for Luma, one 32-bit CRC
        value for Chroma in this type.
    BVDC_Source_CrcType_eChecksum:
        Not supported for non HEVC inputs. Supports both 8-bit and 10-bit HEVC
        inputs. Get one 32-bit CRC value for Luma, one 32-bit CRC value for
        Chroma Cb and one 32-bit CRC value for Chroma Cr in this type.
    BVDC_Source_CrcType_eCrc16:
        Not supported for non HEVC inputs. Supports only 8-bit HEVC inputs.
        Get one 16-bit CRC value for Luma, one 16-bit CRC value for Chroma Cb
        and one 16-bit CRC value for Chroma Cr in this type.

See Also:
***************************************************************************/
typedef enum
{
    BVDC_Source_CrcType_eCrc32,
    BVDC_Source_CrcType_eChecksum,
    BVDC_Source_CrcType_eCrc16,
    BVDC_Source_CrcType_eMax

} BVDC_Source_CrcType;


/***************************************************************************
Summary:
    This enumerate specifies the deinterlacer mode, used in
    BVDC_WinMemConfigSettings.

Description:
    BVDC_DeinterlacerMode_eNone:
        This is equivalent to disable deinterlacer. No deinterlacing, no memory.
    BVDC_DeinterlacerMode_eBestQuality:
        Deinterlacer is enabled in this mode. This mode requires more
        memory, has more latency.
    BVDC_DeinterlacerMode_eLowestLatency:
        Deinterlacer is enabled in this mode. This mode requires minimal
        latency and memory.

See Also:
    BVDC_WinMemConfigSettings
***************************************************************************/
typedef enum BVDC_DeinterlacerMode
{
    BVDC_DeinterlacerMode_eNone = 0,
    BVDC_DeinterlacerMode_eBestQuality = 1,
    BVDC_DeinterlacerMode_eLowestLatency,
    BVDC_DeinterlacerMode_eMax

} BVDC_DeinterlacerMode;

/***************************************************************************
 * VDC data structures
 **************************************************************************/

/***************************************************************************
Summary:
    This structure describes the heap usage of a memory controller.

Description:
    BVDC_Heap_Settings is a structure that use to describe the public
    settings of a memory controller. The settings are in chip scope.

    ulBufferCnt_4HD -
        default number of quad HD buffers available for VDC.

    ePixelFormat_4HD -
        default pixel format for quad HD buffer. The format defines quad
        HD buffers size in VDC. Default pixel format for quad HD buffer is
        BPXL_eY08_Cb8_Y18_Cr8.

    eBufferFormat_4HD  -
        default quad HD buffer format. This format defines the quad HD
        buffers size in VDC. Default quad HD buffer format is
        BFMT_VideoFmt_e4096x2160p_24Hz.

    ulBufferCnt_4HD_Pip -
        default numbfer 4HD PIP buffers available for VDC.  This max PIP size
        is 1/4 of 4HD buffers. PIP windows smaller or equal to 1/4 4HD
        buffer size can use these buffers. PIP windows bigger than 1/4 of 4HD
        buffer size needs to use 4HD buffers.

        Note these buffers can also be used by MAD/ANR (or any other BVN HW
        that require memory).

    ulBufferCnt_2HD -
        default number of double HD buffers available for VDC.

    ePixelFormat_2HD -
        default pixel format for double HD buffer. The format defines double
        HD buffers size in VDC. Default pixel format for double HD buffer is
        BPXL_eY08_Cb8_Y18_Cr8.

    eBufferFormat_2HD  -
        default double HD buffer format. This format defines the double HD
        buffers size in VDC. Default double HD buffer format is
        BFMT_VideoFmt_e1080p_30Hz.

    ulBufferCnt_2HD_Pip -
        default numbfer 2HD PIP buffers available for VDC.  This max PIP size
        is 1/4 of 2HD buffers. PIP windows smaller or equal to 1/4 2HD
        buffer size can use these buffers. PIP windows bigger than 1/4 of 2HD
        buffer size needs to use 2HD buffers.

        Note these buffers can also be used by MAD/ANR (or any other BVN HW
        that require memory).

    ulBufferCnt_HD -
        default number of HD buffers available for VDC.

    ePixelFormat_HD -
        default pixel format for single HD buffer. The format defines single HD
        buffers size in VDC. Default pixel format for single HD buffer is
        BPXL_eY08_Cb8_Y18_Cr8.

    eBufferFormat_HD -
        default HD buffer format. This format defines the HD buffers size in
        VDC. Default HD buffer format is BFMT_VideoFmt_e1080i.

    ulBufferCnt_HD_Pip -
        default numbfer HD PIP buffers available for VDC.  This max PIP size
        is 1/4 of HD buffers. PIP windows smaller or equal to 1/4 HD
        buffer size can use these buffers. PIP windows bigger than 1/4 of HD
        buffer size needs to use HD buffers.

        Note these buffers can also be used by MAD/ANR (or any other BVN HW
        that require memory).

    ulBufferCnt_SD -
        default number of SD buffers available for VDC.

    ePixelFormat_SD -
        default pixel format for SD buffer. The format defines single HD
        buffers size in VDC. Default pixel format for SD buffer is
        BPXL_eY08_Cb8_Y18_Cr8.

    eBufferFormat_SD -
        default SD buffer format. This format defines the SD buffers size in
        VDC. Default SD buffer format is BFMT_VideoFmt_ePAL_G.

See Also:
    BVDC_Heap_GetDefaultSettings, BVDC_Window_Create
***************************************************************************/
typedef struct
{
    /* Quad HD Buffer settings */
    uint32_t                 ulBufferCnt_4HD;
    BPXL_Format              ePixelFormat_4HD;
    BFMT_VideoFmt            eBufferFormat_4HD;
    uint32_t                 ulBufferCnt_4HD_Pip;

    /* Double HD Buffer settings */
    uint32_t                 ulBufferCnt_2HD;
    BPXL_Format              ePixelFormat_2HD;
    BFMT_VideoFmt            eBufferFormat_2HD;
    uint32_t                 ulBufferCnt_2HD_Pip;

    /* HD Buffer settings */
    uint32_t                 ulBufferCnt_HD;
    BPXL_Format              ePixelFormat_HD;
    BFMT_VideoFmt            eBufferFormat_HD;
    uint32_t                 ulBufferCnt_HD_Pip;

    /* SD Buffer settings */
    uint32_t                 ulBufferCnt_SD;
    BPXL_Format              ePixelFormat_SD;
    BFMT_VideoFmt            eBufferFormat_SD;
    uint32_t                 ulBufferCnt_SD_Pip;

}BVDC_Heap_Settings;

/***************************************************************************
Summary:
    This structure describes the settings for the video display control.

Description:
    BVDC_Settings is a structure that use to describe the public
    settings of a video display control that includes all the miscellaneous
    configurations.  The settings are in chip scope.

    eVideoFormat -
        default video format (for both source and display);

    eDisplayFrameRate -
        use this display framerate, otherwise VDC will default to
        BAVC_FrameRateCode_e60

    eColorMatrix_HD -
        default color format setting for the input pictures with unknown color
        matrix, and with vertical size greater than 576;

    eColorMatrix_SD -
        default color format setting for the input pictures with unknown color
        matrix, and vertical size less equal to 480( with 480p );

    bEnableFgt -
        Enable FGT support. There will be no FGT support by default.

    bVecSwap -
        Specify if VDC should swap vec/cmp routing.  For chips that has
        multiple compositor/vec we can hook it up
        as CMP_0 -> VEC_0 or CMP_0 -> VEC_1.  This done so that an HD/SD path
        can drive different combinations of DACs, HDMI, RFM, and/or 656Output.
        In the case of chip with only one compositor and one vec, this flag
        is ignored.  Typically on a single decode system it's desired for
        CMP_0 to be able to drive all outputs, but on system with dual display
        it desired to limit CMP_0, to only 3 DACs, and CMP_1 to 6 DACs.  This
        is because when in HD format only 3 DACs are used.

    stHeapSettings - Main memory heap controller settings.  This structure
        contains the parameters to allocate memrory from the "main" hMem
        from BVDC_Open().  All subsequences heap creation are done by using
        BVDC_Heap_Create().

    aulDacBandGapAdjust - Adjustment to the video TDAC and QDAC bandgap
        (DAC Reference Voltage Adjust) setting.   The default value is correct
        for most chipsets. However, there are some production runs that require
        an adjustment for correct amplitude, depends on the particular fab line
        that manufactured the chip.
        It directly adjusting the value below.  Please consult corresponding RDB
        for additional information.  Each unit represent an increase/decrease of
        0.5% of the reference voltage.  The values program into the
        following registers depend on chipset.
            - MISC_DAC_BG_CTRL_0.IREF_ADJ
            - MISC_QDAC_BG_CTRL_REG.IREF_ADJ
            - MISC_TDAC_BG_CTRL_REG.IREF_ADJ
            - MISC_DAC_BIAS_CTRL_0.GAIN_ADJ
            - MISC.DAC_INST_BIAS_CTRL_0.GAIN_ADJ

    eDacDetection - Option to turn on/off DAC auto detection logic.
        This feature will not work if buffer chip is present on the board.

See Also:
    BVDC_GetDefaultSettings, BAVC_MatrixCoefficients, BFMT_VideoInfo,
    BVDC_Heap_Settings, BVDC_Mode
***************************************************************************/
typedef struct
{
    BFMT_VideoFmt                      eVideoFormat;
    BAVC_FrameRateCode                 eDisplayFrameRate;
    BAVC_MatrixCoefficients            eColorMatrix_HD;
    BAVC_MatrixCoefficients            eColorMatrix_SD;

    bool                               bVecSwap;
    bool                               bEnableFgt;

    /* Main memory controller settings. */
    BVDC_Heap_Settings                 stHeapSettings;

    /* Video DAC (video DAC) bandgap adjustment */
    uint32_t                           aulDacBandGapAdjust[BVDC_MAX_DACS];
    BVDC_Mode                          eDacDetection;

    /* For Memory saving.  Not allocating RUL memory (RDC Slot/List). */
    bool                               bDisableMosaic;
    bool                               bDisable656Input;
    bool                               bDisableHddviInput;

    /* box modes */
    BBOX_Handle                        hBox;

} BVDC_Settings;

/***************************************************************************
Summary:
    A structure representing the rectangle.
    Note:  Even though the offset is signed integer, we don't support
    negative offset to avoid moving rectangle out of canvas boundary. If user
    want the effect of moving rectangle out of the canvas, app needs to clip
    the rectangle explicitly before passing it to VDC.

Description:
    lLeft    - horizontal offset of the left edge of the rectangle from the
               left boundary of a canvas.

    lTop     - vertical offset of the top edge of the rectangle from the
               top boundary of a canvas.

    ulWidth  - width of the rectangle.

    ulHeight - height of the rectangle.

See Also:
    BVDC_Window_SetMosaic, BVDC_Window_SetMosaicDstRects
****************************************************************************/
typedef struct
{
    int32_t      lLeft;
    int32_t      lTop;
    uint32_t     ulWidth;
    uint32_t     ulHeight;

} BVDC_Rect;

/***************************************************************************
Summary:
    This structure describes the default settings a compositor

Description:
    BVDC_Compositor_Settings is a structure that use to describe the public
    settings of a compositor aka CMP.

    reserved for future use.

See Also:
    BVDC_Compositor_GetDefaultSettings, BVDC_Compositor_Create.
***************************************************************************/
typedef void *BVDC_Compositor_Settings;

/***************************************************************************
Summary:
    This structure describes the default settings a window

Description:
    BVDC_Window_Settings is a structure that use to describe the public
    settings of a window.

    hHeap - Specify what hHeap to be used for this window.  If hHeap is NULL
        the internal VDC created heap will be used.  Application is responsible
        for passing the correct heap on system with multiple heaps.  Passing
        incorrect will result in undefined system behaviors.

    hHeap - Specify what hHeap to be used for this window. Capture block on this
        window always use this heap. If hHeap is NULL the internal VDC created
        heap will be used.  Application is responsible for passing the correct
        heap on system with multiple heaps.  Passing incorrect will result in
        undefined system behaviors.

    hDeinterlacerHeap - Specify what hHeap to be used for deinterlacer on this
        window. If hDeinterlacerHeap is specified and it is not the same as
        hHeap, hDeinterlacerHeap will be used for deinterlacer allocation. If
        hDeinterlacerHeap is NULL or it is the same as hHeap, then hHeap will be
        used for deinterlacer allocations. Application is responsible
        for passing the correct heap on system with multiple heaps.  Passing
        incorrect will result in undefined system behaviors.

    ulMaxMosaicRect - Specifies the maximum number of mosaic rects.  This is
        setting is only provided as information to the user and not used to
        configure the window.

    bForceSyncLock - flags the double-buffering implementation instead of
       sync-slipped multi-buffering; this has to combine with VEC locking
       usage to reduce memory allocation;

    bAllocFullScreen - This flag describes option that user can specify trade
    offs between optimizing memory or flashes during transitions. Indicate to
    use full display/compositor size  when allocating memory.
    Default is bAllocFullScreen=false.  Which mean that it only allocate what
    needed depend on the size set by BVDC_Window_SetDstRect.  On resize it may
    cause artifacts.  When it's bAllocFullScreen=true it will allocate as if
    it's display fullscreen.

    pMinSrcFmt - This flag indicate what the minimal source format's size
    (pMinSrcFmt->ulWidth * pMinSrcFmt->ulHeight) to allocate the memory.  For
    example the amount of pixels will be
    ulPixels = MAX( (pSrcFmt->ulWidth * pSrcFmt->ulHeight),
                    (pMinSrcFmt->ulWidth * pMinSrcFmt->ulHeight) );
    Default value is pMinSrcFmt = NULL which means use the pSrcFmt size.  This
    allow user to specify pMinSrcFmt to say 720p format.  Which when source
    format changes less than 720p will not need to realloc.  This is to avoid
    the re-allocation and flush of buffers that introduces black frames when
    source dynamically changes resolution.

    pMinDspFmt - This flag indicate what the minimal display format's size
    (pMinDspFmt->ulWidth * pMinDspFmt->ulHeight) to allocate the memory.  For
    example the amount of pixels will be:
    ulPixels = MAX( (pDspFmt->ulWidth * pDspFmt->ulHeight),
                    (pMinDspFmt->ulWidth * pMinDspFmt->ulHeight) );
    Default value is pMinDspFmt = NULL which means use the pDstFmt size.  This
    allow user to specify pMinDspFmt to say 720p format.  Which when display
    format changes less than 720p will not need to realloc.  This is to avoid
    the re-allocation and flush of buffers that introduces black frames on
    encoder path window.

    bDeinterlacerAllocFull - This flag indicates if user wants full buffers
    allocation for deinterlacer, i.e. no black screen transition
    when various deinterlacer game modes are changed; this means VDC will
    always allocate deinterlacer buffers for normal mode - the worst case;
    default to false;

    bBypassVideoProcessings - This flag indicates if user wants bypass video
    processing like dering and color space conv, which are turn on by default
    as long as HW supports. This is a debug feature likely used by ViCE FW
    team only.
See Also:
    BVDC_Window_GetDefaultSettings
    BVDC_Window_Create
    BVDC_Display_SetAlignment
***************************************************************************/
typedef struct
{
    BVDC_Heap_Handle                   hHeap;
    BVDC_Heap_Handle                   hDeinterlacerHeap;
    uint32_t                           ulMaxMosaicRect;

    bool                               bForceSyncLock;
    bool                               bAllocFullScreen;
    const BFMT_VideoInfo              *pMinSrcFmt;
    const BFMT_VideoInfo              *pMinDspFmt;

    bool                               bDeinterlacerAllocFull;
    bool                               bBypassVideoProcessings;

}BVDC_Window_Settings;

/***************************************************************************
Summary:
    This structure describes the default settings a source

Description:
    BVDC_Source_Settings is a structure that use to describe the public
    settings of a source.

    hHeap - Specify what hHeap to be used for this source.  If hHeap is NULL
        the internal VDC created heap will be used.  Application is responsible
        for passing the correct heap on system with multiple heaps.  Passing
        incorrect will result in undefined system behaviors.

    bGfxSrc - Specify whether this source device is used to feed graphics
        surface.

    eMtgMode - How to decide whether this mpeg feeder will be a Mpeg Time Generator
        (RDC slot trigger)


See Also:
    BVDC_Source_GetDefaultSettings, BVDC_Source_Create.
***************************************************************************/
typedef struct
{
    BVDC_Heap_Handle                   hHeap;
    BVDC_CompositorId                  eDsSrcCompId;
    BVDC_Mode                          eMtgMode;
    bool                               bGfxSrc;

}BVDC_Source_Settings;

/***************************************************************************
Summary:
    This structure describes the default settings a display

Description:
    BVDC_Display_Settings is a structure that use to describe the public
    settings of a display.

    eMasterTg - Specify which Tg drives this display and triggers the display
    RULs. Normally, display 0 uses prim_tg as the master tg, display 1 uses
    sec_tg as master tg, display 2 uses tert_tg if 7400 else 656_tg as master
    tg; however, DTV chips like 3560/3563 could set dvo_tg as master for
    display 2;

    hHeap - Specify what hHeap to be used for this handle.  If hHeap is NULL
    the internal VDC created heap will be used.  Application is responsible
    for passing the correct heap on system with multiple heaps.  Passing
    incorrect will result in undefined system behaviors.

    bArib480p - 480I and 480P video output will follow ARIB convention.
    Mainly, this entails a different start line for active video. This
    convention is used in Japan. Default value is false. Note that the
    name of this item is misleading: the item affects both 480I and 480P
    video.

    bModifiedSync - This option modifies the generation of tri-level
    sync. for 720P, 1080I, and 1080P video, when the color space is
    YPrPb. The default value is false, and this results in sync on
    the Y channel only. If the user changes the value to true, then
    sync will appear on all three channels. The default value (false)
    is consistent with CEA-770_3, clause 8.1.2. The alternate value
    (true) is consistent with SMPTE 296M-1997, clause 12.7

See Also:
    BVDC_Display_GetDefaultSettings, BVDC_Display_Create.
***************************************************************************/
typedef struct
{
    BVDC_Heap_Handle                   hHeap;
    BVDC_DisplayTg                     eMasterTg;
    bool                               bArib480p;
    bool                               bModifiedSync;

}BVDC_Display_Settings;

/***************************************************************************
Summary:
    This structure describes the mask settings for the window callback
    function.

Description:
    BVDC_Window_CallbackMask is a structure that used to describe the public
    mask settings of a window callback function. The masks are used by
    application to specify which event it interested in.

    bVsyncDelay     - User wants to be notified about vsync delay changes. Note the Vsync
                              rate associated with the delay is also reported in the callback to
                              help user with lipsync computation. So when Vsync delay value
                              is not changed, but Vsync rate is changed, window callback will
                              fire as well.
    bDriftDelay     - User wants to be notified about drift delay changes beyond
                              ulLipSyncTolerance in BVDC_Window_CallbackSettings. The
                              drift delay change is calculated as the delta between window's
                              current drift delay and the last time callback's drift delay.
    bGameModeDelay  - User wants to be notified if the game mode delay control threshold
                              has been exceeded. This callback is used to help user control of src/display
                              clock/timebase to achieve shorter multi-buffer delay for game play.
                              VDC also furnishes a different API - BVDC_Window_SetGameModeDelay
                              to allow VDC internal control of display clock to achieve similar goal.
                              It's up to user to choose which game mode control method to use.
                              The pro of VDC internal clock control is faster convergence (seconds) and
                              user hands-free. The con is analog video output may cause TV to roll.
                              The pro/con of user clock/timebase control is potentially smoother control
                              but much slower (minutes) to converge to set target delay.
    bRectAdjust     - User wants to be notified about the change of window's VDC-adjusted destination
                              rectangle.
    bSyncLock       - User wants to be notified about the change of window's sync lock status.

    bCrc            - User wants to be notified about the VNET_B CRC values. In this case, eCrcModule
                              other than BVDC_VnetModule_eCap must also be specified.

See Also:
    BVDC_Window_CallbackSettings
    BVDC_Window_CallbackData
    BVDC_Window_InstallCallback
    BVDC_Window_SetCallbackSettings
    BVDC_Window_GetCallbackSettings
***************************************************************************/
typedef struct
{
    uint32_t                       bVsyncDelay       : 1;
    uint32_t                       bDriftDelay       : 1;
    uint32_t                       bGameModeDelay    : 1;
    uint32_t                       bRectAdjust       : 1;
    uint32_t                       bSyncLock         : 1;
    uint32_t                       bCrc              : 1;

} BVDC_Window_CallbackMask;

/***************************************************************************
Summary:
    This structure describes the settings for the window callback function.

Description:
    BVDC_Window_CallbackSettings is a structure used to control the
    window callback function.

    When stMask.bVsyncDelay is set, VDC will callback when the window's
    current BVN Vsync delay or Vsync rate is different from the last callback.

    When stMask.bDriftDelay is set, VDC will callback when the delta between window's
    current drift delay and the last time callback's drift delay is greater than the
    ulLipSyncTolerance specified in the BVDC_Window_CallbackSettings.

    When stMask.bGameModeDelay is set, VDC will callback when the window's
    current capture buffer delay is outside of the specified range of
    [ulGameModeReadWritePhaseDiff - ulGameModeTolerance,
     ulGameModeReadWritePhaseDiff + ulGameModeTolerance]. This callback gives
    multi-buffer delay feedback to the user clock/timebase control algorithm for
    game play (shorter video processing latency) purpose.

    When stMask.bRectAdjust is set, VDC will callback when the VDC-adjusted window's
    destination rectangle changes.

    When stMask.bCrc is set, VDC will use eCrcModule as the source of VNET_B_CRC,
    and it will callback at every display vsync to pass back the CRC value.

    stMask              - mask settings
    ulLipSyncTolerance  - lip sync tolerance in microseconds.
    ulGameModePhaseDiff - this is the desired capture buffer delay (in usecs) between
                          when a picture is captured and when it's playbacked.
    ulGameModeTolerance - this is the tolerance from the center as defined by
                          ulGameModePhaseDiff. This is in usecs.
    eCrcModule         - check VNET_B_CRC against which vnet module of this window if
                          stMask.bCrc is on. Please notice that eCrcModule can not be
                          BVDC_VnetModule_eCap.

See Also:
    BVDC_Window_CallbackMask
    BVDC_VnetModule
    BVDC_Window_CallbackData
    BVDC_Window_InstallCallback
    BVDC_Window_SetCallbackSettings
    BVDC_Window_GetCallbackSettings
    BVDC_Window_SetGameModeDelay
    BVDC_Window_GetGameModeDelay
***************************************************************************/
typedef struct
{
    BVDC_Window_CallbackMask       stMask;
    uint32_t                       ulLipSyncTolerance;
    uint32_t                       ulGameModeReadWritePhaseDiff;
    uint32_t                       ulGameModeTolerance;
    BVDC_VnetModule                eCrcModule;

} BVDC_Window_CallbackSettings;

/***************************************************************************
Summary:
    This structure describes the settings for the window game mode delay
    control function.

Description:
    BVDC_Window_GameModeSettings is a structure used to describe the
    public settings of a window game mode buffer delay control feature.

    The game mode delay control can place the sync-slipped window's multi-
    buffer playback pointer to a target offset relative to its capture
    pointer to achieve smaller multi-buffer delay and faster game response.

    Note, to make sure the game mode delay control converges and stablize,
    before enabling game mode delay control function, user should make sure
    the followings:

    1) make the window's display track a DPCR timebase;
    2) program the DPCR timebase to lock the window's source rate;

    Also note,

    - This function is currently meant for the DTV application with
      DVO panel output; analog output might cause TV rolling or loss of sync
      during adjustment; analog and digital outputs of the same display cannot
      simultaneously turn on if a window enables game mode;
    - A display can only have one window with game mode delay
      control enabled.
    - Only sync-slipped window can have this feature.

    bEnable                - Enable the feature; default OFF; if there is significant
                             rate gap between source and display, like 50->60, VDC will
                             internally disable it;
    bForceCoarseTrack      - Force coarse tracking of non-standard source refresh
                             rate for game mode display;
                             when this flag is turned on, display can track the source
                             rate within +-0.3% variance of the nominal value.
                             When this flag is turned off, VDC may internally switch
                             between fine track and coarse track: if source
                             format is PC format, or window master frame rate tracking
                             is off, VDC will still use coarse adjustment anyhow. So this
                             flag is mainly an override for TV input rate way off from
                             its nominal value; default OFF;
    ulBufferDelayTarget    - This is the desired difference (in usecs) between
                             source VSYNC and the display VSYNC at the capture
                             buffer.
    ulBufferDelayTolerance - This is the tolerance from the center as defined by
                             ulBufferDelayTarget. This is in usecs.

See Also:
    BVDC_Window_SetGameModeDelay
    BVDC_Window_GetGameModeDelay
***************************************************************************/
typedef struct
{
    bool                           bEnable;
    bool                           bForceCoarseTrack;

    uint32_t                       ulBufferDelayTarget;
    uint32_t                       ulBufferDelayTolerance;

} BVDC_Window_GameModeSettings;

/***************************************************************************
Summary:
    A structure contains information about the current window state, such
    as current vsync delay, lip sync drift delay, and game mode delay
    This structure provides the feedback data to lipsync algorithm.

Description:
    This structure returns asynchronous information about a window
    to application that needs to know about the window.   For example lipsync module
    needs to know the vsync or drift delay for lip sync control purpose.

    The data in the struct might be invalid after the callback fuction retruns.
    The upper level software should make copy of those data if they are needed
    later.

    stMask          - indicates which information is changed against the callback criteria,
                           where the BVDC_Window_CallbackSettings defines the callback criteria.

    ulVsyncDelay    - number of Vsyncs to take a window's input picture to flow from source
                           to display; this value includes the BVN processing latency in unit of
                           fields or frames based on the vertical refresh rate defined by ulVsyncRate;
                           Note this value excludes the delay offset added by user; this value only
                           reflects the ideal vsync delay; the real BVN delay may drift over time
                           when display timing slips with source timing; normally the drift range
                           is within [-1, 1] Vsync of this value. This value will change only when
                           BVN configuration is changed. When one video source feeds multiple displays
                           sharing a single audio path, lipsync algorithm may need to match all display
                           paths ulVsyncDelays by inserting delay offset buffers appropriately.
    ulDriftDelay    - total BVN delay in microseconds, excluding the delay offset added
                           by user, including the sub-field phase difference between the source and
                           display Vsyncs. When stMask.bDriftDelay is set, VDC will callback when
                           the delta between window's current drift delay and the last time callback's
                           drift delay is greater than the ulLipSyncTolerance specified in the
                           BVDC_Window_CallbackSettings.
    ulGameModeDelay - This is the capture buffer delay from when a picture is captured to
                           when it's fed to display; When stMask.bGameModeDelay is set, VDC will
                           callback when the window's current capture buffer delay is outside of the
                           specified range of
                           [ulGameModeReadWritePhaseDiff - ulGameModeTolerance,
                            ulGameModeReadWritePhaseDiff + ulGameModeTolerance].
    ulVsyncRate     - vertical refresh rate in 1/BFMT_FREQ_FACTOR Hz, that the Vsync delay
                           value is based on. This could help upper layer software to convert the
                           ulVsyncDelay into time unit and to control lipsync. Note this Vsync rate may
                           be the source or display Vsync rate, the slower one of the two depending the
                           BVN configuration and usage mode.
    ulVsyncRate     - vertical refresh rate in 1/BFMT_FREQ_FACTOR Hz, that the Vsync delay
                           value is based on. This is the display Vsync rate. Upper layer software
                           can use the slower one of two (ulVsyncRate and ulSrcVsyncRate) depending the
                           BVN configuration and usage mode, to convert the ulVsyncDelay into time unit
                           and to control lipsync.
    ulSrcVsyncRate     - vertical refresh rate in 1/BFMT_FREQ_FACTOR Hz. This is the source Vsync rate.
                           Upper layer software can use the slower one of two (ulVsyncRate and ulSrcVsyncRate)
                           depending the BVN configuration and usage mode, to convert the ulVsyncDelay
                           into time unit and to control lipsync.
    stOutputRect    - VDC-adjusted window's destination rectangle.
    bSynclock       - Window's sync lock status

    ulCrcLuma       -
    ulCrcChroma     - VNET_B_CRC luma/C value when stMask.bCrc is on.  When eCrcModule is not used in
                           the vnet (for eaxmple, if eCrcModule is BVDC_VnetModule_eMad, but MAD /
                           MVP is not used in the current vnet), or during VNET reconfigure, or the
                           picture is muted by user or source, the CRC value returned is 0.

See Also:
    BVDC_Window_CallbackSettings
    BVDC_Window_InstallCallback
    BVDC_Window_SetCallbackSettings
    BVDC_Window_GetCallbackSettings
***************************************************************************/
typedef struct
{
    BVDC_Window_CallbackMask       stMask;
    uint32_t                       ulVsyncDelay;
    uint32_t                       ulDriftDelay;
    uint32_t                       ulGameModeDelay;
    uint32_t                       ulVsyncRate;
    uint32_t                       ulSrcVsyncRate;
    bool                           bSyncLock;
    BVDC_Rect                      stOutputRect;
    uint32_t                       ulCrcLuma;
    uint32_t                       ulCrcChroma;

} BVDC_Window_CallbackData;

/***************************************************************************
Summary:
    This structure contains information about the current backlight state

Description:
    This structure returns information about backlight scaling factor
    calculated from the dynamic contrast algorithm to application.

    ulShift                  - the fixed point format precision for backlight
                               scaling factor
    iScalingFactor           - backlight scaling factor from dynamic contrast
                               in fixed point format S.U where U is dictated
                               by the ulShift above

See Also:
***************************************************************************/
typedef struct
{
    uint32_t                           ulShift;
    int                                iScalingFactor;
} BVDC_Backlight_CallbackData;

/***************************************************************************
Summary:
    This structure describes the mask settings for the display
    callback function.

Description:
    BVDC_Display_CallbackMask is a structure that used to describe the public
    mask settings of a display callback function. The masks are used by
    application to specify which event it is interested in.

    bRateChange  - User wants to be notified when Rate changes

    bCrc         - User wants to be notified when CRC data is available

    bPerVsync    - User wants to be notified at every display vsync for display
        cadence.

    bStgPictureId - User wants to be notified when
        BVDC_Display_CallbackData.ulStgPictureId changes.

    bCableDetect - User wants to be notified when DAC detect status changes

See Also:
***************************************************************************/
typedef struct
{
    uint32_t                       bRateChange       : 1;
    uint32_t                       bCrc              : 1;
    uint32_t                       bPerVsync         : 1;
    uint32_t                       bStgPictureId     : 1;
    uint32_t                       bCableDetect      : 1;

} BVDC_Display_CallbackMask;

/***************************************************************************
Summary:
    A structure contains information about the current display state,
    such as the CRC YCrCb values

Description:
    This structure is used to pass back information about a display to apps.

    stMask       - indicates which information is changed against the callback
                   criteria, that is defined by BVDC_Display_CallbackSettings.

    eId          - display Id
    ePolarity    - display polarity
    sDisplayInfo - display info for rate changes when stMask.bRateChange is on
    ulCrcLuma    - CRC(CMP) Luma value when stMask.bCrc is on
    ulCrcCr      - CRC(CMP) Cr value when stMask.bCrc is on
    ulCrcCb      - CRC(CMP) Cb value when stMask.bCrc is on

    ulStgPictureId - This is a single incrementing value which changes for
        every buffer provided by the STG core.

    ulDecodePictureID - Decoded picture id propagated from decoder.  The decode
        ID will be taken from the master window.

    aeDacConnectionState - state of cable connection per dac

See Also:
***************************************************************************/
typedef struct
{
    BVDC_Display_CallbackMask      stMask;
    BVDC_DisplayId                 eId;
    BAVC_Polarity                  ePolarity;
    BAVC_VdcDisplay_Info           sDisplayInfo;
    uint32_t                       ulCrcLuma;
    uint32_t                       ulCrcCr;
    uint32_t                       ulCrcCb;
    uint32_t                       ulStgPictureId;
    uint32_t                       ulDecodePictureId;
    BVDC_DacConnectionState        aeDacConnectionState[BVDC_MAX_DACS];

} BVDC_Display_CallbackData;

/***************************************************************************
Summary:
    This structure describes the settings for the display callback function.

Description:
    BVDC_Display_CallbackSettings is a structure that used to describe the
    public settings of a display callback function.

See Also:
***************************************************************************/
typedef struct
{
    BVDC_Display_CallbackMask      stMask;

} BVDC_Display_CallbackSettings;


/***************************************************************************
Summary:
    This structure describes the settings for the DVO display settings

Description:
    There various control that are output panel specific.  Some panel would
    like to control H/V-Sync polarity so that the panel internal video
    processor could detect correct H/V-Sync polarity to avoid active video
    displayed on blank time.

    Some panel doesn't like others which have the ability to detect Hs,Vs
    polarity or have extern IC to finish timing singal detection and adaptive
    for it. That's why we need such API to control H/V-Sync polarity .

    10 bit panels include Samsung panel, most new panels and customers.
    8 bit panels include Chimei. Default is 10 bit panel.

    eDePolarity - Specify the "Data Enable Polarity" default is (same)
    eHsPolarity - Specify the "Horizontal Sync Polarity" default is (same)
    eVsPolarity - Specify the "Vertical Sync Polarity" default is (same)

    b8BitPanel - Specify that the panel is in 8-bit mode otherwise 10-bit mode.
    default is (false).

    stSpreadSpectrum.bEnable - enable spread spectrum; default is off.
    stSpreadSpectrum.ulFrequency - spread spectrum frequency
    stSpreadSpectrum.ulDelta - spread spectrum frequency shift range. This
        parameter is 1000 based. For example, when setting ulDelta to 10, the
        spread spectrum frequency will be in the range of [99%, 101%] of
        stSpreadSpectrum.ulFrequency.

    The formulas below describes how "ulFrequency" and "ulDelta" translate
    to the previous "ulScale", "ulHold", "ulRange" and "ulStep" parameters.

    ulHold = 3
    ulScale = 7
    ulRange = 0.001 * FCW * (2**(-10-ulScale)) * ulDelta
    FCW = DVPO_RM_0_OFFSET
    ulStep = 1 / ( (1/ulFrequency) * 108000000 / ulRange / 512 / 4)

    Or "ulFrequency" and "ulDelta" can be mapped from the previous "ulScale",
    "ulHold", "ulRange" and "ulStep" parameters with the following formulas:

    ulHold = 3
    ulScale = 7
    FCW = DVPO_RM_0_OFFSET
    ulDelta = ulRange / (0.001 * FCW * (2**(-10-ulScale)))
    ulFrequency = (ulStep * 52734) / ulRange

See Also:
    BVDC_Display_SetDvoConfiguration
***************************************************************************/
typedef struct
{
    BVDC_SyncPolarity              eDePolarity;
    BVDC_SyncPolarity              eHsPolarity;
    BVDC_SyncPolarity              eVsPolarity;

    bool                           b8BitPanel;

    /* Spread Spectrum configurations */
    struct
    {
        bool                       bEnable;
        uint32_t                   ulFrequency;
        uint32_t                   ulDelta;
    } stSpreadSpectrum;
} BVDC_Display_DvoSettings;

/***************************************************************************
Summary:
    This structure describes the display alignment settings

Description:
    User may want to align one display's Vsync timing with another display for various
    purpose, e.g. lipsync. This setting structure specifies which direction the alignment
    will be performed, where eFaster means to speedup current display's clock to align
    its Vsync timing to the target display; eSlower means to slow down the current
    display's clock to align its Vsync timing to the target display.

    when VDC finds out the current display and target display's Vsync timing difference
    is within a tight rang, it will automatically stop the process until the next time
    it finds out the delta is out of that control range.

    While VDC adjust a display's alignment, it will mute its DAC and HDMI/656 outputs
    to prevent TV rolling;

    bKeepBvnConnected: this flag allows users to keep BVN path of the aligning
    VEC alive or not, so that the target VEC path won't get effected if it's
    on a syncslip path.  Please note that if selecting this option, make sure
    system have enough RTS to support it, or you will get unpredictable behavior.

See Also:
    BVDC_AlignmentDirection
    BVDC_Display_SetAlignment
    BVDC_Display_GetAlignment
***************************************************************************/
typedef struct
{
    BVDC_AlignmentDirection               eDirection;
    bool                                  bKeepBvnConnected;

    /* TODO: add other controls, like speed, aligned phase, etc... */

} BVDC_Display_AlignmentSettings;

/***************************************************************************
Summary:
    This structure describes memory configuration settings for VIP.

Description:
    max w/h/interlaced affects memory allocation size.
See Also:
    BVDC_DispMemConfigSettings
***************************************************************************/
typedef struct BVDC_VipMemConfigSettings
{
    unsigned               ulMemcId;
    uint32_t               ulMaxWidth;
    uint32_t               ulMaxHeight;
    bool                   bSupportInterlaced;
} BVDC_VipMemConfigSettings;

/***************************************************************************
Summary:
    This structure describes the STG settings

Description:
    This struct describes the STG trigger mode as real-time(RT) vs
    non-realtime(NRT).

    In RT mode, STG generates timer triggers based on fixed display frame rate,
    which is for real-time video encode/transcode usage;

    In NRT mode, STG generates end-of-picture(EOP) triggers based on the BVB
    EOP event, which is for file-to-file non-realtime transcode usage.

    ulStcSnapshotLoAddr - STC snapshot register address for lower 32-bit of 42-bit STC.
    ulStcSnapshotHiAddr - STC snapshot register address for higher 10-bit of 42-bit STC.

    hHeap - upper layer sw needs to create VIP picture heap according to the memConfig's ulVipSize.
    stMemSettings - max format supported by VIP, which affects memory allocation.

    NOTE: This API allows user to dynamically adjust VIP memory allocation so user is responsible
    to make sure runtime allocation not exceeding worst case and not to fragment memory.
    It's recommended not to dynamically change the VIP heap allocation.

    Also NOTE: user MUST stop and return all extracted encode pictures back to display before disabling
    VIP or change VIP heap memory settings!!

See Also:
    BVDC_Display_SetStgConfiguration
    BVDC_Display_GetStgConfiguration
    BVDC_Display_ReturnBuffer_isr
***************************************************************************/
typedef struct
{
    bool               bNonRealTime;
    uint32_t           ulStcSnapshotLoAddr; /* for new soft transcoder STC snapshot */
    uint32_t           ulStcSnapshotHiAddr;
    struct {
        BMMA_Heap_Handle   hHeap; /* for new soft transcoder VIP picture heap. If NULL, disabled VIP */
        BVDC_VipMemConfigSettings stMemSettings;
    } vip;
} BVDC_Display_StgSettings;


/***************************************************************************
Summary:
    Prototype for external modules to install callback with VDC.

Description:
    Upper level application install callback for a specific update event from
    from VDC.  For example HDMI modules needed to be notified whenever VDC
    display changes it rate manager or format, so that it (HDMI) could also
    config its rate manager to run at the same rate.  This callback prototype
    should be use for all the callback that register with VDC.

Inpute:
    pvParam1 - User defined data structure casted to void.
    iParam2 - Additional user defined value.
    pvVdcData - Data pass from VDC to application thru callback.  The
    data structure must defined in bavc.h if data is being pass back and forth
    between porting interface modules, or the data structure can be a VDC
    public structure. The data in the struct might be invalid to application
    after the callback fuction retruns. The application should make copy of
    those data if they are needed later. If it's not a data structure, but
    rather a primitive type then VDC will translate that accordingly.  This
    data is provided by VDC.  The internal data of pvVdcData may contains
    pointers (or place holder) that application can write to. In a sense passing
    data back to VDC.

Output:
    None

See Also:
    BVDC_Display_InstallCallback,
    BVDC_Display_UnInstallCallback,
    BVDC_Window_InstallCallback
**************************************************************************/
typedef void (*BVDC_CallbackFunc_isr)
    ( void                            *pvParam1,
      int                              iParam2,
      void                            *pvVdcData );


/***************************************************************************
Summary:
    Prototype of source picture callback function.

Description:
    This is the prototype of the source picture callback function. Once
    installed by the application, it is called at ISR context by BVDC with
    every field / frame, to get the info about the next picture (a video /
    graphics field/frame) for display, and to pass to user the current state
    of the used BVDC source sub-module for synchronization.

    The call back function is defined by user and is installed to BVDC with
    BVDC_Source_InstallPictureCallback. The next picture info is represented
    by a BAVC picture struct, and is returned by the call back function with
    pointer ppvPicture. The source sub-module state is passed to user by
    parameter eSourceState. The BAVC picture struct type is BAVC_Gfx_Picture
    for graphics and BAVC_VDC_HdDvi_Picture for BXVD video.

    When the graphics produced by BGRC or BP3D is displayed by BVDC, the
    call back function could be used if triple/multi-buffering is needed
    for smooth graphics animation. With this usage, the graphics (pixel)
    buffers are managed by user, and one graphics buffer is pulled by BVDC
    from BGRC / BP3D for every frame / field of video display, using the
    call back function. The pulled graphics buffer has been newly produced
    by BGRC or BP3D and is represented by the returned BAVC_Gfx_Picture
    struct.

    After a graphics buffer is passed to BVDC, roughly one field/frame video
    display time later, it is fetched by a VDC graphics feeder, and the
    fetching would last for one field / frame video display time. That means
    the graphics pixel buffer passed by the first calling of the picture
    call back function would be freed by BVDC when the call back function is
    called at the third time, and so on, ....

    If the upstream has not produced a new graphics buffer yet the call back
    function should return NULL for (*ppvPicture), then the previous graphics
    buffer would be used one more time by BVDC for display.

    In the case that 7411 is used to provide the AVC video, the video signal
    is sent to BVDC as a HD DVI source, and 7411 is controlled by BXVD. The
    picture call back function is used to get the property of the next HD
    DVI field/frame sent to BVDC from 7411, and to pass the source state
    info to user for synchronization. It is called for every field / frame.

Input:
    pvParm1 - Pointer to a user defined struct that user wish to pass to this
    call back function. It might include info about the upstream module such
    as BGRC, BP3D or BXVD, and the BVDC source sub-module that is used to
    accept the upstream produced picture. It could also be NULL.
    iParm2 - An int number that user wish to pass to this call back function.
    ePolarity - Polarity expected by BVDC for next picture.
    eSourceState - Current state of BVDC source sub-module that is used to
    accept the picture buffer from upstream module.

Output:
    ppvPicture  - A pointer to a source specific picture structure:
    GFX:     BAVC_Gfx_Picture (describes graphics to be used NEXT)
    HD_DVI:  BAVC_HdDvi_Picture (describes the CURRENT captured buffer)

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_InstallPictureCallback
    BVDC_Source_SetSurface,
    BVDC_Source_SetSurface_isr,
***************************************************************************/
typedef void (*BVDC_Source_PictureCallback_isr)
    ( void                            *pvParm1,
      int                              iParm2,
      BAVC_Polarity                    ePolarity,
      BAVC_SourceState                 eSourceState,
      void                           **ppvPicture );

/***************************************************************************
Summary:
    A structure representing the information detected from box-detect.

Description:
    This structure is passed from BVDC to user as a parameter of the user
    defined box-detect call-back function and is used by user to determine
    whether there is letter box or pillar box in the source video signal.

    For letter box, it is expected to have two black patch regions on the
    top and bottom of the screen, and the active video box (called white box)
    is located between them. Because of noise, filtering or other reasons,
    there are two thin gray strips separating the top black patch and bottom
    black patch and the white box. Letter box is illustrated in the following:

       +-------------------------------------------------+
       |                                                 |
       |                 Top black patch                 |
       |                                                 |
       |-------------------------------------------------|
       |                Top gray strip                   |
       |-------------------------------------------------|<- WhiteBoxTop
       |                                                 |
       |                                                 |
       |                                                 |
       |                                                 |
       |             Active video rectangle              |
       |                 (White box)                     |
       |                                                 |
       |                                                 |
       |                                                 |
       |                                                 |
       |-------------------------------------------------|<- WhiteBoxBottom
       |               Bottom gray strip                 |
       |-------------------------------------------------|
       |                                                 |
       |                 Bottom black patch              |
       |                                                 |
       +-------------------------------------------------+

    In a typical letter box case, experiments show that the white box is
    about 50% to 85% of the original source height, and it is located roughly
    in the center of the screen. The top and bottom gray strip width are
    expected to be less than 3. BVDC also returns a strength number for each
    of the four white box edges, in order to help the box detect decision. The
    strength indicates the brightness of the inner edge of the white box. The
    top and bottom edge strength are expected to be bigger than 50 for letter
    box.

    Similarly, pillar box involves two black patch regions on the left and
    right side of the screen and the white box is located between them. And
    there are two thin gray strip separating the left black patch and the
    right black patch and the white box.

    In the typical pillar box case, the white box is expected to be about 50%
    to 85% of the original source width, and it is located roughly in the
    center of the screen too. The left and right gray strip width are expected
    to be less than 3. And the left and right edge strength are expected to be
    bigger than 20.

    The numbers listed here are only for reference. User might use any
    criteria that is suitable to their application. The basic idea is that
    small gray strip width and high edge strength indicate high possibility of
    box existence.

See Also:
    BVDC_Window_BoxDetectCallback_isr
****************************************************************************/
typedef struct
{
    /* active video (whuite bix) rectangle */
    uint32_t  ulWhiteBoxLeft;
    uint32_t  ulWhiteBoxTop;
    uint32_t  ulWhiteBoxWidth;
    uint32_t  ulWhiteBoxHeight;

    /* gray strip width: relative thin strip from white to black */
    uint32_t  ulGrayWidthTop;
    uint32_t  ulGrayWidthLeft;
    uint32_t  ulGrayWidthRight;
    uint32_t  ulGrayWidthBottom;

    /* edge strength: brightness of white box inner edge */
    uint32_t  ulEdgeStrengthTop;
    uint32_t  ulEdgeStrengthLeft;
    uint32_t  ulEdgeStrengthRight;
    uint32_t  ulEdgeStrengthBottom;

} BVDC_BoxDetectInfo;

/***************************************************************************
Summary:
    Specifies AFD (Active Format Descriptor) settings.

Description:
    This structure specifies the AFD settings.

    eMode - AFD handling mode. Default value is (BVDC_AfdMode_eDisabled)

    eClip - AFD clipping mode. Default value is (BVDC_AfdClip_eNominal)

    ulAfd - User's specifies AFD value only to be use for the case
    (eMode == BVDC_AfdMode_eUser).  Default value is (0).

See Also:
    BVDC_Window_SetAfdSettings, BVDC_Window_GetAfdSettings.
***************************************************************************/
typedef struct
{
    BVDC_AfdMode                       eMode;
    BVDC_AfdClip                       eClip;
    uint32_t                           ulAfd;

} BVDC_AfdSettings;

/***************************************************************************
Summary:
    Prototype of the box detect callback.

Description:
    This is the prototype of the box detect callback function. It is provided
    by user as a parameter to BVDC_Window_EnableBoxDetect.

    It is called by BVDC right after box detect is enabled for a window, and
    when the detected data changes. The calling is at interrupt handler
    context. As parameters to this call back function, BVDC passes back to
    user a pointer to the struct BVDC_BoxDetectInfo.

    Based on the info in BVDC_BoxDetectInfo, it might be able to decide
    whether there is letter box or pillar box in the source video.

    In the call back function, user might make some reasonable judgment on
    the box detection, and set source clip rectangle to clip the black patch
    regions, or only save the passed-in info and process it later, or even
    simply do nothing. One thing to notice is that the struct pointed by
    pBoxDetectInfo is no longer available after this call back function is
    returned.

Input:
    pvParm1 - Pointer to a user defined struct that user wish to pass to this
    call back function. It might include info about the window this box detect
    belongs to. It could be NULL.
    iParm2 - An int number that user wish to pass to this call back function.
    pBoxDetectInfo - Pointer to the struct passed from BVDC to user for box
    existence judgment.

Output:

Returns:

See Also:
    BVDC_Window_EnableBoxDetect
***************************************************************************/
typedef void (*BVDC_Window_BoxDetectCallback_isr)
    ( void                            *pvParm1,
      int                              iParm2,
      const BVDC_BoxDetectInfo        *pBoxDetectInfo );


/***************************************************************************
Summary:
    A structure representing the captured field/frame that is provided to the
    user application.

Description:
    captureBuffer  - a left surface derived from a field or frame
    rCaptureBuffer - a right surface derived from a field or frame
    eOutOrientation - display 3D orientation of the left and right surfaces
    eCapturePolarity - the polarity of the captured image.  It's undefined if
        hCaptureBuffer is NULL.

See Also:
    BVDC_Window_GetBuffer
****************************************************************************/
typedef struct
{
    BPXL_Plane                         captureBuffer;
    BPXL_Plane                         rCaptureBuffer;
    BFMT_Orientation                   eOutOrientation;
    BAVC_Polarity                      eCapturePolarity;
} BVDC_Window_CapturedImage;


/***************************************************************************
Summary:
    This structure describes the mask settings for the source
    callback function.

Description:
    BVDC_Source_CallbackMask is a structure that used to describe the public
    mask settings of a source callback function. The masks are used by
    application to specify which event it is interested in.

    bFmtInfo     - User wants to be notified when input video format changes
    bActive      - User wants to be notified when input status changes
    bCrcValue    - User wants to be notified when CRC data is available
    bFrameRate   - User wants to be notified when framerate changes
    bSrcPending  - User wants to be notified when need to manualy resume

See Also:
    BVDC_Source_InstallCallback
****************************************************************************/
typedef struct
{
    uint32_t                           bFmtInfo                     : 1;
    uint32_t                           bActive                      : 1;
    uint32_t                           bCrcValue                    : 1;
    uint32_t                           bFrameRate                   : 1;
    uint32_t                           bSrcPending                  : 1;

} BVDC_Source_CallbackMask;

/* For backward compatibility */
typedef BVDC_Source_CallbackMask BVDC_Source_DirtyBits;

/***************************************************************************
Summary:
    A structure representing the captured field/frame that is provided to the
    user application.

Description:
    bActive - It specifies if the source active or detecting video.

    pFmtInfo - It specifies the format informaiton of the source.  Inside the
        structure ulWidth, ulHeight, ulDigitalWidth, ulDigitalHeight, bInterlaced,
        and ulVertFreq are always valid. The other elements are valid only if
        its eVideoFmt < BFMT_VideoFmt_eMaxCount. This is always true for VDEC /
        HD_DVI source, but might not be true for some digital source from MVD/XVD.

    ulCrcValue - It specifies the CRC values of decoded buffers.  Only valid
        for MVD/XVD source that the source is feeding out the decoded buffers
        via feeder.
    eSourcePolarity - the CRC key picture's field polarity;
    ulIdrPicId      - the CRC key picture's IDR picture ID;
    lPicOrderCnt    - the CRC key picture's order count;

    ulLumaCrc - It specifies the Luma CRC values of decoded buffers.  Only
        valid for MVD/XVD source that the source is feeding out the decoded
        buffers via feeder. This is a 32-bit CRC Luma value for
        BVDC_Source_CrcType_eCrc32 type, a 32-bit CRC Luma checksum for
        BVDC_Source_CrcType_eChecksum type, and a 16-bit CRC Luma value for
        BVDC_Source_CrcType_eCrc16 type.

    ulChromaCrc - It specifies the Chroma CRC values of decoded buffers.  Only
        valid for MVD/XVD source that the source is feeding out the decoded
        buffers via feeder. This is a 32-bit CRC Chroma value for
        BVDC_Source_CrcType_eCrc32 type, 32-bit CRC Chroma Cb checksum for
        BVDC_Source_CrcType_eChecksum type, and 16-bit CRC Chroma Cb value for
        BVDC_Source_CrcType_eCrc16 type.

    ulChroma1Crc - It specifies the Chroma CRC values of decoded buffers.  Only
        valid for MVD/XVD source that the source is feeding out the decoded
        buffers via feeder. This is not used for BVDC_Source_CrcType_eCrc32 type,
        a 32-bit CRC Chroma Cr checksum for BVDC_Source_CrcType_eChecksum type,
        and a 16-bit CRC Chroma Cr value for BVDC_Source_CrcType_eCrc16 type.

    ulLumaCrcR - It specifies the Luma CRC values of decoded buffers for right
        eye.Only valid for MVD/XVD source that the source is feeding out the
        decoded buffers via feeder. This is a 32-bit CRC Luma value for
        BVDC_Source_CrcType_eCrc32 type, a 32-bit CRC Luma checksum for
        BVDC_Source_CrcType_eChecksum type, and a 16-bit CRC Luma value for
        BVDC_Source_CrcType_eCrc16 type.

    ulChromaCrcR - It specifies the Chroma CRC values of decoded buffers for
        right eye. Only valid for MVD/XVD source that the source is feeding out
        the decoded buffers via feeder. This is a 32-bit CRC Chroma value for
        BVDC_Source_CrcType_eCrc32 type, 32-bit CRC Chroma Cb checksum for
        BVDC_Source_CrcType_eChecksum type, and 16-bit CRC Chroma Cb value for
        BVDC_Source_CrcType_eCrc16 type.

    ulChroma1CrcR - It specifies the Chroma CRC values of decoded buffers for right eye.
        Only valid for MVD/XVD source that the source is feeding out the decoded
        buffers via feeder. This is not used for BVDC_Source_CrcType_eCrc32 type,
        a 32-bit CRC Chroma Cr checksum for BVDC_Source_CrcType_eChecksum type,
        and a 16-bit CRC Chroma Cr value for BVDC_Source_CrcType_eCrc16 type.

    eFrameRateCode - It specifies the frame rate code of the source. For MPEG,
        HD_DVI sources, frame rate code is passed from the picture callback;
        for VDEC source, it's detected from VDEC core.

    ulVertRefreshRate - It is in units of 1/BFMT_FREQ_FACTOR Mhz. It is a translation of
        eFrameRateCode.

See Also:
    BVDC_Source_InstallCallback, BVDC_Source_SetResumeMode
****************************************************************************/
typedef struct
{
    /* These bits tell which of the following field changes. */
    BVDC_Source_CallbackMask           stMask;

    /* 656/HD_DVI only */
    const BFMT_VideoInfo              *pFmtInfo;
    bool                               bActive;

    /* MPEG feeder XVD/MVD */
    uint32_t                           ulLumaCrc;
    uint32_t                           ulChromaCrc;
    uint32_t                           ulChroma1Crc;
    uint32_t                           ulLumaCrcR;
    uint32_t                           ulChromaCrcR;
    uint32_t                           ulChroma1CrcR;
    bool                               bMtgSrc;           /* dirty maked by bFrameRate */

    uint32_t                           ulIdrPicId;
    int32_t                            lPicOrderCnt;
    BAVC_Polarity                      eSourcePolarity;
    BAVC_FrameRateCode                 eFrameRateCode;    /* dirty maked by bFrameRate */
    uint32_t                           ulVertRefreshRate; /* dirty maked by bFrameRate */

} BVDC_Source_CallbackData;


/***************************************************************************
Summary:
    This structure describes the settings for the source callback function.

Description:
    BVDC_Source_CallbackSettings is a structure that used to describe the
    public settings of a source callback function.

    stMask       - mask settings used by application to specify which event
                   it is interested in.

See Also:
    BVDC_Source_CallbackMask
    BVDC_Source_GetCallbackSettings
    BVDC_Source_SetCallbackSettings
***************************************************************************/
typedef struct
{
    BVDC_Source_CallbackMask           stMask;

} BVDC_Source_CallbackSettings;

/***************************************************************************
Summary:
    Get the current settings of the source callback.

Description:
    This function gets the current settings for the source callback function.

Input:
    hSource - Source handle created earlier.
    pSettings - Points to the current callback function settings.

See Also:
    BVDC_Source_CallbackData
    BVDC_Source_CallbackSettings
    BVDC_Source_InstallCallback
    BVDC_Source_SetCallbackSettings
***************************************************************************/
BERR_Code BVDC_Source_GetCallbackSettings
    ( BVDC_Source_Handle                   hSource,
      BVDC_Source_CallbackSettings        *pSettings );

/***************************************************************************
Summary:
    Set the settings of the source callback.

Description:
    This function sets the settings for the source callback function.

    Apps must call this function before BVDC_Source_InstallCallback.

Input:
    hSource - Source handle created earlier.
    pSettings - Points to the callback function settings defined by user.

See Also:
    BVDC_Source_CallbackData
    BVDC_Source_CallbackSettings
    BVDC_Source_InstallCallback
    BVDC_Source_GetCallbackSettings
***************************************************************************/
BERR_Code BVDC_Source_SetCallbackSettings
    ( BVDC_Source_Handle                   hSource,
      const BVDC_Source_CallbackSettings  *pSettings );


/***************************************************************************
Summary:
    Array to hold a pair of FMT format enums.

Description:
    This is an array to hold two FMT enum for mapping or other purposes.

    eFmtA - A format A (from).

    eFmtB - B format B (to).

See Also:
    BVDC_Source_SetReMapFormats
****************************************************************************/
typedef struct
{
    BFMT_VideoFmt                      eFmtA;
    BFMT_VideoFmt                      eFmtB;

} BVDC_VideoFmtPair;

/***************************************************************************
Summary:
    This structure describes the settings used the contrast stretch algorithm.

Description:
    BVDC_ContrastStretch is a structure that used to describe the
    public settings that used in the contrast stretch calculation.

    ulShift                  - the fixed point format precision for gains
    iGain                    - the amount of stretch towards min and max
                               (recommended value of 1 in fixed point format)
    iFilterNum               - the numerator of the min, mid, max low
                               pass filter
    iFilterDenom             - the denomerator of the min, mid, max low
                               pass filter
    iDynContrastBlackGain    - gain for black stretch side in Dynamic Contrast
                               (in fixed point format)
    iDynContrastWhiteGain    - gain for white stretch side in Dynamic Contrast
                               (in fixed point format)
    uiDynContrastBlackLimit  - limit for black stretch in Dynamic Contrast
    uiDynContrastWhiteLimit  - limit for white stretch in Dynamic Contrast
    uiDynContrastEstCurMaxPt - point to estimate current max luma in dynamic
                               contrast
    uiDynContrastEstCurMinPt - point to estimate current min luma in dynamic
                               contrast
    pvCustomParams           - point to custom params used by customer dynamic
                               contrast algorithm

    ulPwmMaxApl              - Indicate the Maximum APL for backlight dimming
    ulPwmMinApl              - Indicate the Minimum APL for backlight dimming
    ulPwmMinPercent          - Indicate the Minimum percent for backlight
                               dimming.  Maximum percent is fixed at 100.
    ulDcLoThresh             - Number of threshold used for Low level
    ulDcHiThresh             - Number of threshold used for Hi level
    ulHiThreshBlendMin       - Level of Hi threshold MIN blending
    ulHiThreshBlendRng       - Range of Hi threshold blending
    ulLoThreshBlendMin       - Level of Lo threshold MIN blending
    ulLoThreshBlendRng       - Range of Lo threshold blending
    ulHiThreshRatio          - Hi threshold ratio.  This number is defined as
                               fractional (value * 1000) so 1.000 = (1000) and
                               0.250 = (250) etc.
    ulLoThreshRatio          - Lo threshold ratio.  Same convention as
                               ulHiThresRatio.  Sum of these 2 ratios must be
                               1.000
    bInterpolateTables       - A flag to indicate if interpolation between
                               aulDCTable1 and aulDCTable2 is enabled
    alIreTable               - Array containing a family of IRE values
    aulDcTable1              - Array containing a family of curves that define
                               the input-luma to output-luma function, for
                               defined APL levels.
    aulDcTable2              - Array containing a family of curves that define
                               the input-luma to output-luma function, for
                               defined APL levels.
    bBypassSat               - A flag to select bypassing saturation adjustment

    pfCallback               - A function pointer to a callback that will
                               update back light scaling factor.
    pvParm1                  - User defined structure.casted to void.
    iParm2                   - User defined value.

    Restriction for data in alIreTable, aulDcTable1 and aulDcTable2 (if
    bInterpolateTables flag is set)tables:
    - All values in the table should be 0-1023.
    - The first column should start at 64 or less, and finish at 940 or more,
      and should always be increasing (to avoid the divide by zero problem).
    - The values along each row (excluding the first column) should always be
      increasing.
    - The values in the IRE table should always be increasing, with increments
      of at least 16 between each entry.

    The fields are for the dynamic contrast stretch algorithm. These,
    however, do not allow the user direct control of the
    LAB (Luma Adjustment Block) table. The algorithm just uses these fields as
    basis for calculating the contrast stretch.   Warning: These can change
    in future chip revisions.

See Also:
    BVDC_Window_SetContrastStretch
    BVDC_Window_GetContrastStretch
***************************************************************************/
typedef struct
{
    uint32_t                           ulShift;
    int                                iGain;
    int                                iFilterNum;                  /* UNUSED */
    int                                iFilterDenom;                /* UNUSED */
    int                                iDynContrastBlackGain;       /* UNUSED */
    int                                iDynContrastWhiteGain;       /* UNUSED */
    uint16_t                           uiDynContrastBlackLimit;     /* UNUSED */
    uint16_t                           uiDynContrastWhiteLimit;     /* UNUSED */
    uint16_t                           uiDynContrastEstCurMaxPt;    /* UNUSED */
    uint16_t                           uiDynContrastEstCurMinPt;    /* UNUSED */
    void                              *pvCustomParams;

    uint32_t                           ulPwmMaxApl;
    uint32_t                           ulPwmMinApl;
    uint32_t                           ulPwmMinPercent;
    uint32_t                           ulDcLoThresh;
    uint32_t                           ulDcHiThresh;
    uint32_t                           ulHiThreshBlendMin;
    uint32_t                           ulHiThreshBlendRng;
    uint32_t                           ulLoThreshBlendMin;
    uint32_t                           ulLoThreshBlendRng;
    uint32_t                           ulHiThreshRatio;
    uint32_t                           ulLoThreshRatio;
    bool                               bInterpolateTables;
    int32_t                            alIreTable[BVDC_DC_TABLE_COLS - 1];
    uint32_t                           aulDcTable1[BVDC_DC_TABLE_ROWS * BVDC_DC_TABLE_COLS];
    uint32_t                           aulDcTable2[BVDC_DC_TABLE_ROWS * BVDC_DC_TABLE_COLS];
    bool                               bBypassSat;

    /* Generic Callback */
    BVDC_CallbackFunc_isr              pfCallback;
    void                              *pvParm1;
    int                                iParm2;
} BVDC_ContrastStretch;


/***************************************************************************
Summary:
    This structure describes the setting for blue stretch

Description:
    BVDC_BlueStretch is a structure that used to described the public
    settings for blue stretch

    ulBlueStretchOffset - Offset used to calculate the scaling coefficient.
                          Range from 0 to 0x7FFFF.

    ulBlueStretchSlope  - Slope used to calculate the scaling coefficient
                          Range from 0 to 7.

See Also:
    BVDC_Window_SetBlueStretch
    BVDC_Window_GetBlueStretch
***************************************************************************/
typedef struct
{
    uint32_t                           ulBlueStretchOffset;
    uint32_t                           ulBlueStretchSlope;
} BVDC_BlueStretch;

/***************************************************************************
Summary:
    This structure describes the color bar order used the CMS algorithm.

Description:
    BVDC_ColorBar is a structure that used to describe the
    setting order of colors that used in the CMS calculation.

    lGreen               - the setting for Green
    lYellow              - the setting for Yellow
    lRed;                - the setting for Red
    lMagenta             - the setting for Magenta
    lBlue                - the setting for Blue
    lCyan                - the setting for Cyan

See Also:
    BVDC_Window_SetCmsControl
    BVDC_Window_GetCmsControl
***************************************************************************/
typedef struct
{
    int32_t     lGreen;
    int32_t     lYellow;
    int32_t     lRed;
    int32_t     lMagenta;
    int32_t     lBlue;
    int32_t     lCyan;

} BVDC_ColorBar;


/***************************************************************************
Summary:
    This structure describes cusomtimze index to select filter coefficients

Description:
    The BVDC_CoefficientIndex is to allow user to select the customize
    fitler coefficients in for scaler (possible mad/hscaler), in the built-in
    or user replacable tables in bvdc_coeffs_priv.c.  The default are 0.

    ulSclVertLuma    - Selecting the scaler vertical luma filter index.
    ulSclHorzLuma    - Selecting the scaler horizontal luma filter index.
    ulSclVertChroma  - Selecting the scaler vertical chroma filter index.
    ulSclHorzChroma  - Selecting the scaler horizontal chroma filter index.
    ulHsclHorzLuma   - Selecting the hscaler horizontal luma filter index.
    ulHsclHorzChroma - Selecting the hscaler horizontal chroma filter index.
    ulMadHorzLuma    - Selecting the mad horizontal luma filter index.
    ulMadHorzChroma  - Selecting the mad horizontal chroma filter index.

    Graphics windows only use ulSclVertLuma and ulSclHorizLuma.
    The rest of the filter indices are ignored.

    There is a set of built-ins coeffs that is in the range of
    101 -> 126 that have effects of softness -> sharpness trade-offs.  User can
    use these indices to override the default.

See Also:
    BVDC_Window_SetCoefficientIndex
    BVDC_Window_GetCoefficientIndex
***************************************************************************/
typedef struct
{
    /* SCL */
    uint32_t                           ulSclVertLuma;
    uint32_t                           ulSclHorzLuma;

    uint32_t                           ulSclVertChroma;
    uint32_t                           ulSclHorzChroma;

    /* HSCL */
    uint32_t                           ulHsclHorzLuma;
    uint32_t                           ulHsclHorzChroma;

    /* MAD */
    uint32_t                           ulMadHorzLuma;
    uint32_t                           ulMadHorzChroma;

} BVDC_CoefficientIndex;

/***************************************************************************
Summary:
    This structure describes current window status.

Description:
    BVDC_Window_Status is a structure that contains the current status
    of the window.

    bSyncLock - This parameter indicates if the current window is sync
    locked or not. It's always false for window connects to source
    other than MPEG source. Note this flag may be invalid after calling
    BVDC_Window_Destroy or BVDC_ApplyChanges.

See Also:
    BVDC_Window_GetStatus
***************************************************************************/
typedef struct
{
    bool                               bSyncLock;

} BVDC_Window_Status;

/***************************************************************************
Summary:
    This structure describes the window settings for split screen demo mode.

Description:
    BVDC_SplitScreenSettings is a structure that contains the window setting of
    each feature that has the split screen demo mode capability.

    eHue             - split screen mode for hue and saturation
    eContrast        - split screen mode for contrast
    eAutoFlesh       - split screen mode for auto flesh
    eBrightness      - split screen mode for brightness
    eColorTemp       - split screen mode for color temperature
    eSharpness       - split screen mode for sharpness
    eBlueBoost       - split screen mode for blue boost
    eGreenBoost      - split screen mode for green boost
    eCms             - split screen mode for CMS
    eContrastStretch - split screen mode for contrast stretch
    eBlueStretch     - split screen mode for blue stretch
    eFgt             - split screen mode for fgt
    eAnr             - split screen mode for Anr
    eDeJagging       - split screen mode for Scaler's dejagging
    eDeRinging       - split screen mode for Scaler's deringing

See Also:
    BVDC_SplitScreenMode
    BVDC_Window_SetSplitScreen
    BVDC_Source_SetSplitScreen
***************************************************************************/
typedef struct
{
    BVDC_SplitScreenMode               eHue;
    BVDC_SplitScreenMode               eContrast;
    BVDC_SplitScreenMode               eBrightness;
    BVDC_SplitScreenMode               eColorTemp;
    BVDC_SplitScreenMode               eSharpness;
    BVDC_SplitScreenMode               eContrastStretch;
    BVDC_SplitScreenMode               eFgt;
    BVDC_SplitScreenMode               eAnr;
    BVDC_SplitScreenMode               eDeJagging;
    BVDC_SplitScreenMode               eDeRinging;
    BVDC_SplitScreenMode               eCms;
    BVDC_SplitScreenMode               eAutoFlesh;
    BVDC_SplitScreenMode               eBlueBoost;
    BVDC_SplitScreenMode               eGreenBoost;
    BVDC_SplitScreenMode               eBlueStretch;

} BVDC_Window_SplitScreenSettings;

/***************************************************************************
Summary:
    This structure describes the source settings for split screen demo mode.

Description:
    BVDC_Source_SplitScreenSettings is a structure that contains the source
    setting of each feature that has the split screen demo mode capability.

    eDnr             - split screen mode for DNR

See Also:
    BVDC_SplitScreenMode
    BVDC_Window_SetSplitScreen
    BVDC_Source_SetSplitScreen
***************************************************************************/
typedef struct
{
    BVDC_SplitScreenMode               eDnr;

} BVDC_Source_SplitScreenSettings;


/***************************************************************************
Summary:
    This structure describes the source format status.

Description:
    BVDC_Source_InputStatus is a structure that contains the
    input format status reading from HDDVI hw.  These are values
    from hardware.

    bVsyncDetected - This parameter indicates whether there is vsync signal
    in the input video data or not.

    bNoSignal - This parameter indicates whether there is signal or not.

    bInterlaced - This parameter indicates the input format's raster type.
    It's either interlaced / progressive.

    ulHPolarity - This parameter indicates the input format's horizontal
    polarity.  0 it's positive polarity.  1 it's a negative polarity.

    ulVPolarity - This parameter indicates the input format's vertical
    polarity.  0 it's positive polarity.  1 it's a negative polarity.

    ulAvWidth - This parameter indicates the input format's horizontal
    active pixels.

    ulAvHeight - This parameter indicates the input format's vertical
    active pixels.

    ulHBlank - This parameter indicates the input format's horizontal
    blanking pixels.

    ulVBlank - This parameter indicates the input format's vertical
    blanking pixels.

    ulVertFreq - This parameter indicates the input format's vertical
    frequencty.  In 1/BFMT_FREQ_FACTOR Hz.  60Hz would be 6000.  59.94Hz would be
    5994, and etc.

    pFmtInfo - This parameter indicates what format we're detecting and setting
    the HW to.  In the case of there is a signal but unknown format this will be
    NULL, or bNoSignal == true this will also be NULL.  Note that if remap is
    enabled it's the remapped format.

    pOriFmtInfo - Same as pFmtInfo, but in the case of remapped this the format
    information prior to remapped.

    Note that user can get information about the source via the callback.
    Which returns the BFMT_VideoInfo of a given formats.

See Also:
    BVDC_Source_GetInputStatus, BVDC_Source_SetReMapFormats
***************************************************************************/
typedef struct
{
    bool                               bVsyncDetected;
    bool                               bNoSignal;
    bool                               bInterlaced;
    uint32_t                           ulHPolarity;
    uint32_t                           ulVPolarity;
    uint32_t                           ulAvWidth;
    uint32_t                           ulAvHeight;
    uint32_t                           ulVBlank;
    uint32_t                           ulHBlank;
    uint32_t                           ulVertFreq;
    const BFMT_VideoInfo              *pFmtInfo;
    const BFMT_VideoInfo              *pOriFmtInfo;

} BVDC_Source_InputStatus;

/***************************************************************************
Summary:
    This structure describes the capabilities of a vdc

Description:
    BVDC_Capabilities is a structure that contains the capabilities of vdc.

    ulNumBox - Indicates the number of letter box detection (detection black
    bars content).  This is is only available in older chips.  Most later chips
    does not support box detection.

    ulNumCmp - Indicates the number of video graphics compositor.  This is
    enumerated with BVDC_CompositorId.

    ulNumMad - Indicates the number of MAD (Motion Adapative Deinterlacer)
    digital processing hardware.  Shared resource among windows.

    ulNumDnr - Indicates the number of DNR (Digital Noise Reduction) digital
    processing hardware.  Shared resource among windows.

    ulNumPep - Indicates the number of PEP (Picture Enhancement Processing).
    Not shared resource but rather tied or hardwired to
    BVDC_CompositorId_eCompositor0::BVDC_WindowId_eVideo0 window.  These are
    functionality available when a window has PEP:
        * BVDC_Window_SetContrastStretch
        * BVDC_Window_SetCmsControl
        * BVDC_Window_SetAutoFlesh
        * BVDC_Window_SetBlueBoost
        * BVDC_Window_SetGreenBoost
        * BVDC_Window_SetBlueStretch

    ulNumTab - Indicates the number of TAB (Transient Adjustment Block)
    aka TNT (The Next TAB).  (Not shared resource but rather hardwired to
    BVDC_CompositorId_eCompositor0::BVDC_WindowId_eVideo0 window.  These are
    functionality available when a window has TAB:
        * BVDC_Window_SetSharpness

    ulNumDac - Indicates the number of DACs available for output.  DACs can be
    combine to create output port (CVBS, SVideo, YPrPb, RGB, SCART).  Each
    port will require different number of DAC.

    ulNumRfm - Indicates the number of RFM output.

    ulNumStg - Indicates the number of STG (Simple Trigger Generation) output
    to ViCE encoder (a transcode channel).

    ulNumAtg - Indicates the number of Analog Timing Generator

    ulNum656Input - Indicates the number of ITU-R-656 input.

    ulNum656Output - Indicates the number of ITU-R-656 output.

    ulNumHdmiInput - Indicates the number of HDMI (aka HD_DVI for VDC) input.

    ulNumHdmiOutput - Indicates the number of HDMI output.

    ulNumAlgPaths - Indicates the number of analog paths supported

    bAlgStandAlond - Indicates if the display can be stand-alone i.e no need
                     to occupy an analog path even if no DACs or RFM connected

    b3DSupport - Indicates the 3D stereo Orientation Fmt support

See Also:
    BVDC_GetCapabilities, BVDC_Display_SetDacConfiguration,
    BVDC_Display_SetRfmConfiguration, BVDC_Display_SetStgConfiguration,
    BVDC_Display_Set656Configuration, BVDC_Display_SetHdmiConfiguration,
    BVDC_Source_SetDnrConfiguration.
***************************************************************************/
typedef struct BVDC_Capabilities
{
    uint32_t                           ulNumBox;
    uint32_t                           ulNumCmp;
    uint32_t                           ulNumMad;
    uint32_t                           ulNumDnr;
    uint32_t                           ulNumPep;
    uint32_t                           ulNumTab;
    uint32_t                           ulNumDac;
    uint32_t                           ulNumRfm;
    uint32_t                           ulNumStg;
    uint32_t                           ulNumAtg;
    uint32_t                           ulNumCap;
    uint32_t                           ulNumVfd;
    uint32_t                           ulNumMfd;

    uint32_t                           ulNum656Input;
    uint32_t                           ulNum656Output;

    uint32_t                           ulNumHdmiInput;
    uint32_t                           ulNumHdmiOutput;

    uint32_t                           ulNumAlgPaths;
    bool                               bAlgStandAlone;

    bool                               b3DSupport;
} BVDC_Capabilities;

/***************************************************************************
Summary:
    This structure describes the capabilities of a display

Description:
    BVDC_Display_Capabilities is a structure that contains the
    capabilities of display.

    pfIsVidfmtSupported - This function checks if a given video format is
    supported or not.  Note if a given format is not supported
    BVDC_Display_SetVideoFormat would also return error.

See Also:
    BVDC_Display_GetCapabilities, BVDC_Display_SetVideoFormat.
***************************************************************************/
typedef struct
{
    bool                (*pfIsVidfmtSupported)(BFMT_VideoFmt eVideoFmt);

    /* might add other parameters */

} BVDC_Display_Capabilities;

/***************************************************************************
Summary:
    This structure describes the capabilities of a compositor

Description:
    BVDC_Compositor_Capabilities is a structure that contains the
    capabilities of compositor.

    reserved for future use.

See Also:
    BVDC_Compositor_GetCapabilities
***************************************************************************/
typedef void *BVDC_Compositor_Capabilities;

/***************************************************************************
Summary:
    This structure describes the capabilities of a source.

Description:
    BVDC_Source_Capabilities is a structure that contains the
    capabilities of source.

    pfIsPxlfmtSupported - This function checks if a given pixel format is
    supported or not.

See Also:
BVDC_Source_GetCapabilities
***************************************************************************/
typedef struct
{
    bool                (*pfIsPxlfmtSupported)(BPXL_Format ePxlFmt);

    /* might add other parameters */

} BVDC_Source_Capabilities;

/***************************************************************************
Summary:
    This structure describes the capabilities of a window

Description:
    BVDC_Window_Capabilities is a structure that contains the
    capabilities of window.

    reserved for future use.

See Also:
    BVDC_Window_GetCapabilities
***************************************************************************/
typedef void *BVDC_Window_Capabilities;

/***************************************************************************
Summary:
    This structure describes MAD reverse 3:2 pulldown configurations.

Description:

    ulPhaseMatchThreshold - Reverse 3:2 phase correlation match threshold;
    When a particular phase gives a correlation that is greater than the
    threshold, the corresponding counter is incremented. If it is less than
    the threshold, the counter is decremented.

    ulEnterLockLevel - Reverse 3:2 phase correlation match counter "enter lock"
    value. When the corresponding counter value increases beyond an
    "increasing" threshold, that phase is considered locked.

    ulRepfVetoLevel - Reverse 3:2 repeat field motion veto level.   When
    repf_motion value is greater than this threshold then it is implied that
    this really can not be a repeat field. When this occurs, a veto bit is
    set and the counter for that phase is set to zero and not allowed to
    increment.  If it was locked before this point in time, once this
    threshold is exceeded, the lock is removed.

    ulRepfPxlCorrectLevel - Reverse 3:2 repeat field motion pixel correct level.
    The repf_motion is compared against this threshold.  If repf_motion is
    less than this threshold, the ppufm (per pixel un-expected field motion)
    is disabled. If repf_motion is larger this threshold, but lower
    than REPF_VETO_LEVEL, the ppufm is enabled.

    ulLockSatLevel - Reverse 3:2 phase correlation match counter saturation level;

    ulBadEditLevel - Reverse 3:2 bad edit level.  If frame_unexpected_motion
    value is greater than this threshold then it is assumed that either a
    bad-edit has occurred or there has been a change to interlace-sourced
    material. Either way, all the phase match counters are set back to zero.

    ulBwvLuma32Threshold  - Difference in PCC's between the direction we plan
    to weave and the other direction is compared to this threshold to
    declare 3:2 lock invalid. Larger value mean that a larger bad weave would
    be required to declare lock invalid.

    maybe more config ...

See Also:
    BVDC_Deinterlace_Settings
    BVDC_Window_SetDeinterlaceConfiguration
***************************************************************************/
typedef struct
{
    uint32_t                           ulPhaseMatchThreshold;
    uint32_t                           ulEnterLockLevel;
    uint32_t                           ulRepfVetoLevel;
    uint32_t                           ulRepfPxlCorrectLevel;
    uint32_t                           ulLockSatLevel;
    uint32_t                           ulBadEditLevel;
    uint32_t                           ulBwvLuma32Threshold;

    /* might add other modes, like force spatial etc */
} BVDC_Deinterlace_Reverse32Settings;


/***************************************************************************
Summary:
    This structure describes MAD reverse 2:2 pulldown configurations.

Description:

    ulNonMatchMatchRatio  - Reverse 2:2 NonMatch_UM and Match_UM ratio. The
    reverse 2:2 phase detector only increments this phase counter if, not
    only are the measured UM values bounded with the programmable thresholds,
    but also the ratio between the NonMatch_UM and Match_UM is sufficiently
    large.

    ulEnterLockLevel - Reverse 2:2 phase correlation match counter "enter lock"
    value. When the corresponding counter value increases beyond an
    "increasing" threshold, that phase is considered locked.

    ulLockSatLevel - Reverse 2:2 phase correlation match counter saturation
    level;

    ulBwvLuma22Threshold - Difference in PCC's between the direction we plan
    to weave and the other direction is compared to this threshold to declare
    2:2 lock invalid. Larger value mean that a larger bad weave would be
    required to declare lock invalid.

    maybe more config ...

    Note, current VDC implements software algorithm for reverse 2:2 pulldown;
    therefore these settings should make their way into the software algorithm
    if the software alg is used (REV22_AUTO is off).

See Also:
    BVDC_Deinterlace_Settings
    BVDC_Window_SetDeinterlaceConfiguration
***************************************************************************/
typedef struct
{
    uint32_t                           ulNonMatchMatchRatio;
    uint32_t                           ulEnterLockLevel;
    uint32_t                           ulLockSatLevel;
    uint32_t                           ulBwvLuma22Threshold;

    /* might add other modes, like force spatial etc */
} BVDC_Deinterlace_Reverse22Settings;

/***************************************************************************
Summary:
    This structure customizes MAD chroma processing configurations.

Description:

    bChromaField420EdgeDetMode - Chroma Field 420 interpolation edge detection
    mode.  When this bit is set to 1, use the new method.

    bChromaField420InitPhase - This is to indicate the processed picture is
    started from even or odd line.  default is 0, which means even line.

    eChromaField420InvMethod - This is to indicate how we want to get
    the original 420 chroma pixel;

    eChroma422MotionAdaptiveMode - Chroma 422 motion adaptive mode.

    eChroma422InverseTelecineMode - Chroma 422 inverse telecine mode.

    ulMaxXChroma - Maximum amount to let the resulting chroma deviate from
    the spatial average.  Range from 0 to 255.

    maybe more config ...

See Also:
    BVDC_Deinterlace_Settings
    BVDC_Window_SetDeinterlaceConfiguration
***************************************************************************/
typedef struct
{
    /* chroma 420 processing mode */
    bool                                bChromaField420EdgeDetMode;
    bool                                bChromaField420InitPhase;
    BVDC_Deinterlace_Chroma420InvMethod eChromaField420InvMethod;

    /* cross-chroma processing mode */
    BVDC_Deinterlace_Chroma422MaMode    eChroma422MotionAdaptiveMode;
    BVDC_Deinterlace_Chroma422ItMode    eChroma422InverseTelecineMode;

    uint32_t                            ulMaxXChroma;

    /* chroma motion mode */
    BVDC_Deinterlace_ChromaMotionMode   eChroma422MotionMode;
    BVDC_Deinterlace_ChromaMotionMode   eChroma420MotionMode;

    /* chroma calculation mode: if true, use the newer algorithm; */
    bool                                bMS_3548;/* motion spatial */
    bool                                bMT_3548;/* motion temporal */

    /* cross-chroma Field Chroma 420 motion adaptive mode.*/
    BVDC_Deinterlace_Chroma422MaMode    eChroma420MotionAdaptiveMode;

    /* might add other modes, like force spatial etc */
} BVDC_Deinterlace_ChromaSettings;

/***************************************************************************
Summary:
    This structure customizes MAD motion mode.

Description:

    eTmMode    - Temporal motion mode selection;

    eSmMode    - Spatial motion mode selection;

    bEnableQmK - field K quantized motion load enable;

    bEnableQmL - field L quantized motion load enable;

    bEnableQmM - field M quantized motion load enable;

    maybe more config ...

    Note, since QM stores could be manipulated by VDC internally when supporting
    low delay or low bandwidth mode (MAD Game Mode), the user control of
    bEnableQmK/L/M will be ignored for MAD game mode;

See Also:
    BVDC_Deinterlace_Settings
    BVDC_Window_SetDeinterlaceConfiguration
***************************************************************************/
typedef struct
{
    /* temporal motion mode */
    BVDC_Deinterlace_TmMode            eTmMode;

    /* spatial motion mode */
    BVDC_Deinterlace_SmMode            eSmMode;

    /* fields quantized motion load enable */
    bool                               bEnableQmK;
    bool                               bEnableQmL;
    bool                               bEnableQmM;

    /* might add other modes, like force spatial etc */
} BVDC_Deinterlace_MotionSettings;

/***************************************************************************
Summary:
    This structure customizes MAD low angle settings.

Description:

    ulLaControlDirRatio - This field determines how clearly identifiable an
    edge needs to be. Increasing the value will make the MAD low-angle
    processing more conservative meaning that it will only filter along the
    direction of more prominent edges.

    ulLaControlRangeLimitScale - This field is reduced, it will also make the
    low angle processing more conservative meaning that only more pronounced
    edges will be filtered in the direction of the edge.

    ulLaMinNorthStrength - This field is increased the low-angle processing
    will be more conservative and only edges that are more pronounced will be
    filtered in the direction of the edge.

See Also:
    BVDC_Deinterlace_Settings
    BVDC_Window_SetDeinterlaceConfiguration
***************************************************************************/
typedef struct
{
    uint32_t                           ulLaControlDirRatio;
    uint32_t                           ulLaControlRangeLimitScale;
    uint32_t                           ulLaMinNorthStrength;

} BVDC_Deinterlace_LowAngleSettings;

/***************************************************************************
Summary:
    This structure describes MAD configurations.

Description:
    MAD always receives 444 data and output 444 data, but internally
    process data as 422.  Hence will perform down sample 444 to 422 to process
    and then up sample data from 422 to 444.  This parameter allow user to
    customize conversion process.

    eGameMode            - The MAD game mode selection;
    bReverse32Pulldown   - Turn on/off 3:2 reverse pulldown;
    bReverse22Pulldown   - Turn on/off 2:2 reverse pulldown;
    pReverse32Settings   - Reverse 3:2 pulldown settings; not used if
                           bReverse32Pulldown is false;
    pReverse22Settings   - Reverse 2:2 pulldown settings; not used if
                           bReverse22Pulldown is false;
    pChromaSettings      - Chroma processing settings;
    pMotionSettings      - Motion processing settings;
    pUpSampler           - Up sampling conversion settings.
    pDnSampler           - Down sampling conversion settings.
    pLowAngles           - Low angles settings
    ePxlFormat           - field storage pixel format: 8-bit or 10-bit 4:2:2.

    bShrinkWidth - This flag indicates if VDC should scale down
    the width of the input source so that the deinterlacer could be used in the
    case that the source width is bigger than the deinterlace HW limitation.
    Please notice that this would sacrifice horizontal resolution, and that
    this will lead deinterlacer to require more memory buf and bandwidth.

    ePqEnhancement - Enhancement processing policies.  eAuto will have internal
    decision logic to apply best picture possible.  eOn will force those policies
    on.  eOff will keep those policies out.  Let user control.
    Default is (eAuto).  eOn will automatically bypass deinterlacer in
    480i -> 480i, 576i -> 576i, and 1080i -> 1080i cases (similarly to cases
    where the source is progressive no deinterlacer is needed).  Toggle
    deinterlacer bypass will changes the BVN video propagation delay and
    also causes BVN reconfigurations that yield blackframes.

    maybe more config ...

    Note, the structure pointers (pReverse32Settings, pstReverse22Settings,
    pChromaSettings, pMotionSettings) are used here instead of real structure data
    store; for "Set" function call, if the pointers are NULL, VDC would use
    the default settings; otherwise power users can pass in the settings to
    override VDC's internal default; for "Get" function call, the pointers
    output will always be NULLs;

    ePxlFormat should be one of the 8-bit 4:2:2 pixel formats for settop box
    chipsets that don't support 10-bit video data; it could be one of the
    8-bit or 10-bit 4:2:2 pixel formats for DTV chips that support 10-bit
    video data; ePxlFormat may impact bandwidth and memory allocation.
    For chipsets with ANR/Deinterlacer memory sharing mode, like 3548, the
    ANR pixel format should be the same as Deinterlacer pixel format.
    Default is 8-bit 422.

    For chipsets with ANR/Deinterlacer memory sharing mode, like 3548, the ANR
    ulBitsPerPixel filed should be the same as Deinterlacer ulBitsPerPixel filed.

See Also:
    BVDC_Window_SetDeinterlaceConfiguration
    BVDC_Window_GetDeinterlaceConfiguration
    BVDC_Window_GetDeinterlaceDefaultConfiguration
***************************************************************************/

typedef struct
{
    BVDC_MadGameMode                   eGameMode;

    bool                               bReverse32Pulldown;
    BVDC_Deinterlace_Reverse32Settings *pReverse32Settings;

    bool                               bReverse22Pulldown;
    BVDC_Deinterlace_Reverse22Settings *pReverse22Settings;

    BVDC_Deinterlace_ChromaSettings   *pChromaSettings;

    BVDC_Deinterlace_MotionSettings   *pMotionSettings;

    /* Up/down sampling filters */
    BVDC_422To444UpSampler            *pUpSampler;
    BVDC_444To422DnSampler            *pDnSampler;

    /* Low angles */
    BVDC_Deinterlace_LowAngleSettings *pLowAngles;

    /* field storage pixel format: 8-bit or 10-bit 4:2:2; */
    BPXL_Format                        ePxlFormat;

    bool                               bShrinkWidth;
    BVDC_Mode                          ePqEnhancement;

    /* might add other parameters */

} BVDC_Deinterlace_Settings;


/***************************************************************************
Summary:
    This structure describes ANR configurations.

Description:

    eMode - disable, bypass, or enable ANR.

    iSnDbAdjust - user adjustment to S/N db number.

    If S/N is around 60 db, the video signal is very clean and ANR will be
    configured to perform very little filtering. When S/N db number become
    smaller, the video signal is more noisy and ANR will be configured to
    perform stronger filtering. As it reaches to about 25 db, ANR filtering
    reaches the strongest.

    iSnDbAdjust is subtracted from the S/N db number before it is used to
    decide the strength of ANR filtering. If user wishes to increase ANR
    filtering effect, iSnDbAdjust should be set to a positive number, and the
    bigger iSnDbAdjust is, the stronger ANR filtering is. And if it is set to
    a negative number, ANR filtering effect is adjusted weaker. 0 means no
    user adjustment to the ANR filtering strength.

    ePxlFormat - field storage pixel format: 8-bit or 10-bit 4:2:2. User must set
    it to one of the 8-bit YCrCb 4:2:2 pixel formats for 3563; ePxlFormat may
    impact bandwidth and memory allocation. For chipsets with ANR/Deinterlacer
    memory sharing mode, like 3548, the ANR pixel format should be the same as
    Deinterlacer pixel format. Default is 8-bit 422.

    pvUserInfo - a pointer to a user defined struct. This struct will be passed
    to BVDC_P_Anr_GetKValue_isr, that could be rewritten by customer to achieve
    special ANR filtering strength. A customer written BVDC_P_Anr_GetKValue_isr
    might customize the ANR K values (again, that control the strength of ANR
    filtering) according to the information passed by pvUserInfo.

See Also:
    BVDC_FilterMode, BVDC_Window_SetAnrConfiguration, BVDC_Window_GetAnrConfiguration.
***************************************************************************/

typedef struct
{
    BVDC_FilterMode                    eMode;
    int                                iSnDbAdjust;

    /* field storage pixel format: 8-bit or 10-bit 4:2:2; */
    BPXL_Format                        ePxlFormat;

    void                              *pvUserInfo;

} BVDC_Anr_Settings;


/***************************************************************************
Summary:
    This structure describes scaler configurations.

    bSclVertDejagging - Toggle the vertical scaler dejagging.

    bSclHorzLumaDeringing - Toggle the horizontal luma scaler deringing.

    bSclVertLumaDeringing - Toggle the vertical luma scaler deringing.

    bSclHorzChromaDeringing - Toggle the horizontal chroma scaler deringing.

    bSclVertChromaDeringing - Toggle the vertical chroma scaler deringing.

    bSclVertPhaseIgnore - ignore the vertical scaler init phase adjustment.

    ulSclDejaggingCore - Dejagging core value ranging from 0 to 255 (U8.4)
    for 8-bit system, or 0 to 1023 (U10.2) for 10-bit system.

    ulSclDejaggingGain - Dejagging weight factor (U2.3) for the second score.
    It is used in Score delta calcuation.
    Its values can be  0, 1/8, 1/4, 3/8, 1/2, 3/4, 1, 3/2,  or 2.

    ulSclDejaggingHorz - Dejagging horizontal weight factor (U2.3) for
    the second score.  It is used in Score delta calcuation.
    Its values can be  0, 1/8, 1/4, 3/8, 1/2, 3/4, 1, 3/2,  or 2.

See Also:
    BVDC_Window_SetScalerConfiguration.
***************************************************************************/
typedef struct
{
    bool                               bSclVertDejagging;
    bool                               bSclHorzLumaDeringing;
    bool                               bSclVertLumaDeringing;
    bool                               bSclHorzChromaDeringing;
    bool                               bSclVertChromaDeringing;
    bool                               bSclVertPhaseIgnore;
    uint32_t                           ulSclDejaggingCore;
    uint32_t                           ulSclDejaggingGain;
    uint32_t                           ulSclDejaggingHorz;

} BVDC_Scaler_Settings;


/***************************************************************************
Summary:
    This structure describes Color Clip slopes/joints configurations.

Description:
    The job of the color clip is to gracefully remap out of range RGB values in a
    graceful way before hardware does the clipping to 0 or 1023.  What gets clipped
    is purely a function of the matrix which converts display-colorspace YCbCr to
    display-colorspace RGB.  This matrix is completely determined by the display's
    primaries and white point.

    It only depends on the primaries and white point of the display, which is fixed.
    Therefore, the slopes and joints can be computed offline and stored for each
    TV model or individual unit.

    ulCrYSlopeA     - Chroma versus (219-Luma) Slope; in U16.16 fixed point format.
    ulCrYSlopeB     - Chroma versus (219-Luma) Slope; in U16.16 fixed point format.
    ulCbYSlopeA     - Chroma versus (219-Luma) Slope; in U16.16 fixed point format.
    ulCbYSlopeB     - Chroma versus (219-Luma) Slope; in U16.16 fixed point format.
    ulCrJoint       - Joint of Cr Slope_A and Slope_B.
    ulCbJoint       - Joint of Cb Slope_A and Slope_B.
    eColorClipMode  - modes for color clipping.

    Note: slopes are unsigned and the derivation should follow the provided
    programming guide.

    TODO: how to describe A vs B? shall we compute those inside VDC and only
    expose more understandable RGBW as input?

See Also:
    BVDC_Compositor_SetColorClip
    BVDC_Compositor_GetColorClip
***************************************************************************/
typedef struct
{
    uint32_t                           ulCrYSlopeA;
    uint32_t                           ulCrYSlopeB;
    uint32_t                           ulCbYSlopeA;
    uint32_t                           ulCbYSlopeB;
    uint32_t                           ulCrJoint;
    uint32_t                           ulCbJoint;
    bool                               bExtendedWhite;
    bool                               bExtendedBlack;
    BVDC_ColorClipMode                 eColorClipMode;

} BVDC_ColorClipSettings;


/***************************************************************************
Summary:
    Contains the mosaic rectangles top configuration settings

Description:
    bVideoInMosaics       - This boolean controls whether to clear in/outside
                            region of Mosaic Rectangles;
                            true: to clear outside rects, intended for channels
                            list app that multiple channels video contents are
                            decoded/captured into a single video window's mosaic
                            rects where outside region of those rects is cleared;
                            false: normally used for HD-DVD application that will
                            surface up the lower layer video or gfx as mosaic
                            effect; It can also be used to display bars instead of
                            transparent video for other windows in box mode on
                            the same compositor.
    bClearRectByMaskColor - clear the region with the mask color or source video;
                            true: mask color; false: source video;
    ulClearRectAlpha      - alpha blend the region's content with the below layer;
                            range is 0 ~ 255, where 0: transparant; 255: opaque;
    ulMaskColorRed        - red component of mask color; 0 ~ 255 8-bit for now;
    ulMaskColorGreen      - green component of mask color; 0 ~ 255 8-bit for now;
    ulMaskColorBlue       - blue component of mask color; 0 ~ 255 8-bit for now;

See Also:
    BVDC_Window_SetMosaicConfiguration
***************************************************************************/
typedef struct
{
    bool                               bVideoInMosaics;
    bool                               bClearRectByMaskColor;
    uint32_t                           ulClearRectAlpha;
    uint32_t                           ulMaskColorRed;
    uint32_t                           ulMaskColorGreen;
    uint32_t                           ulMaskColorBlue;

} BVDC_MosaicConfiguration;


/***************************************************************************
Summary:
    A structure representing the clipping paramters.

Description:
    Note that the unit of clipping is specified by respective API that
    uses the BVDC_ClipRect structure.

    ulLeft - The amount being clipping from left of the incomming image.
    ulRight - The amount being clipping from left of the incomming image.
    ulTop - The amount being clipping from left of the incomming image.
    ulBottom - The amount being clipping from left of the incomming image.

See Also:
    BVDC_LumaSettings.
****************************************************************************/
typedef struct
{
    uint32_t     ulLeft;
    uint32_t     ulRight;
    uint32_t     ulTop;
    uint32_t     ulBottom;

} BVDC_ClipRect;


/***************************************************************************
Summary:
    This structure describes the luma setting.

Description:
    BVDC_LumaSettings is a structure that contains the
    settings for getting the luma min/max/sum, and histogram data
    out of the compositor/window.

    stRegion - This parameter indicates the region to be computed
    the Histogram data, or the region to collect luma sum for the compositor.
    It's specifies in the term of clipping of the relative to the input of
    the Histogram Block.  By default there is no rectangle clipping.  The
    unit of clipping is in 100th of percent.

    No clipping example:
        stHistoRegion.ulLeft   = 0
        stHistoRegion.ulRight  = 0
        stHistoRegion.ulTop    = 0
        stHistoRegion.ulBottom = 0

    5% clipping on each side example:
        stHistoRegion.ulLeft   = 500
        stHistoRegion.ulRight  = 500
        stHistoRegion.ulTop    = 500
        stHistoRegion.ulBottom = 500

    aulLevelThres - This parameter indicates the threshold level in percentage.
    Right now, these thresholds are read only, users cannot program yet.

    eNumBins - This parameter indicates the number of histogram bins used

See Also:
    BVDC_Compositor_SetLumaStatsConfiguration,
    BVDC_Compositor_GetLumaStatsConfiguration,
    BVDC_Window_SetLumaStatsConfiguration,
    BVDC_Window_GetLumaStatsConfiguration
***************************************************************************/
typedef struct
{
    BVDC_ClipRect                      stRegion;
    uint32_t                           aulLevelThres[BVDC_LUMA_HISTOGRAM_LEVELS];
    BVDC_HistBinSelect                 eNumBins;

} BVDC_LumaSettings;



/***************************************************************************
Summary:
    This structure describes the luma status.

Description:
    BVDC_LumaStatus is a structure that contains the
    luma average and histogram data of the compositor/window.

    ulSum - This parameter indicates the sum of all luma in the specified
    region.

    ulMin - This parameter indicates the min of all luma in the specified
    region.

    ulMax - This parameter indicates the min of all luma in the specified
    region.

    aulHistogram - This is luma histogram data count.

    aulLevelStats - This is the histogram level statistic.  This report the
    luma value at which the associated level threshold was reached.

    ulPixelCnt - This is the total pixel count.

See Also:
    BVDC_Compositor_GetLumaStats,
    BVDC_Window_GetLumaStats
***************************************************************************/
typedef struct
{
    uint32_t                           ulSum;
    uint32_t                           ulMin;
    uint32_t                           ulMax;
    uint32_t                           aulHistogram[BVDC_LUMA_HISTOGRAM_COUNT];
    uint32_t                           aulLevelStats[BVDC_LUMA_HISTOGRAM_LEVELS];
    uint32_t                           ulPixelCnt;
} BVDC_LumaStatus;

/***************************************************************************
Summary:
    This structure describes the chroma setting.

Description:
    BVDC_ChromaSettings is a structure that contains the settings of the
    chroma rectangle, sat min and hue min/max values, and the chroma
    histogram type for a window.

    stRegion - This parameter indicates the region to obtain the
    the Chroma Histogram data for. It's specified in terms of clipping
    amount relative to the input to the Chroma Histogram Block.  By default
    there is no clipping.  The unit of clipping is in 100th of a percent.

        No clipping example:
            stRegion.ulLeft   = 0
            stRegion.ulRight  = 0
            stRegion.ulTop    = 0
            stRegion.ulBottom = 0

        5% clipping on each side example:
            stRegion.ulLeft   = 500
            stRegion.ulRight  = 500
            stRegion.ulTop    = 500
            stRegion.ulBottom = 500

    eType    - indicates the chroma histogram type

    ulSatMin - This parameter indicates the min of all saturation in the
               specified region.

    ulHueMax - Maximum Hue Value. Enter as unsigned hue angle (U9.1 format)
               in the range 0 to 359.5 degrees..

    ulHueMin - Minimum Hue Value. Enter as unsigned hue angle (U9.1 format)
               in the range 0 to 359.5 degrees.

See Also:
    BVDC_Window_SetChromaStatsConfiguration,
    BVDC_Window_GetChromaStatsConfiguration
***************************************************************************/
typedef struct
{
    BVDC_ClipRect                      stRegion;
    BVDC_ChromaHistType                eType;
    uint32_t                           ulSatMin;
    uint32_t                           ulHueMin;
    uint32_t                           ulHueMax;
} BVDC_ChromaSettings;

/***************************************************************************
Summary:
    This structure describes the chroma status.

Description:
    BVDC_ChromaStatus is a structure that contains the chroma histogram data
    of a window. Note that interpretation of the contents of the arrays
    is highly dependent on the chroma histogram type specified in the
    BVDC_ChromaSettings structure.

    stCrCbHist.aulCrHistogram - this will be populated only for CR/CB type
                                histogram
    stCrCbHist.aulCbHistogram - this will be populated only for CR/CB type
                                histogram

    stHueSatHist.aulHueHistogram - this will be populated only for HUE/SAT type
                                histogram
    stHueSatHist.aulSatHistogram - this will be populated only for HUE/SAT type
                                histogram

    ulCount                    - indicates the number of elements in the
                                 above histogram arrays, ie., 32 for CR/CB type
                                 or 24 for HUE/SAT type histogram

See Also:
    BVDC_ChromaSettings
    BVDC_Window_GetChromaStats
***************************************************************************/
typedef union {
    struct
    {
        uint32_t aulCrHistogram[BVDC_CR_CB_HISTOGRAM_COUNT];
        uint32_t aulCbHistogram[BVDC_CR_CB_HISTOGRAM_COUNT];
        uint32_t ulCount;
    } stCrCbHist;

    struct
    {
        uint32_t aulHueHistogram[BVDC_HUE_SAT_HISTOGRAM_COUNT];
        uint32_t aulSatHistogram[BVDC_HUE_SAT_HISTOGRAM_COUNT];
        uint32_t ulCount;
    } stHueSatHist;
} BVDC_ChromaStatus;

/***************************************************************************
Summary:
    This structure describes the compositor OSD settings.

Description:

    ucOsdSelectThreshold - This parameter specifies the OSD Select threshold
    value. OSD Select = 0 if the threshold value is 255.

See Also:
    BVDC_Compositor_SetOsdConfiguration,
    BVDC_Compositor_GetOsdConfiguration
***************************************************************************/
typedef struct
{
    uint8_t                            ucOsdSelectThreshold;

} BVDC_OsdSettings;


/***************************************************************************
Summary:
    This structure describes dithering of the window.

Description:
    This structure allow application to control the dithering process apply
    on window.

    bReduceSmooth - Reduces smoothing for bright pictures.  Default value is
    false.

    bSmoothEnable - Enable smoothing filter.  Defaul value is false.

    ulSmoothLimit - The range 0 to 0xf.  Default value is 0.

See Also:
    BVDC_Window_SetDitherConfiguration;
***************************************************************************/
typedef struct
{
    bool                               bReduceSmooth;
    bool                               bSmoothEnable;
    uint32_t                           ulSmoothLimit;

} BVDC_DitherSettings;


/***************************************************************************
Summary:
    This structure describes color key settings of the window.

Description:
    This structure allow application to control the color key apply
    on window.

    bLumeKey       - Enable luma key support.
    ucLumaKeyMask  - Window's surface luma comparison mask. Default value is 0.
    ucLumaKeyHigh  - Window's surface maximum for luma comparison.
                     Default value is 0xff.
    ucLumaKeyLow   - Window's surface mimimum for luma comparison.
                     Default value is 0.

    bChromaRedKey       - Enable chroma red key support.
    ucChromaRedKeyMask  - Window's surface chroma red comparison mask. Default
                     value is 0.
    ucChromaRedKeyHigh  - Window's surface maximum for chroma red comparison.
                     Default value is 0xff.
    ucChromaRedKeyLow   - Window's surface mimimum for chroma red comparison.
                     Default value is 0.

    bChromaBlueKey       - Enable chroma blue key support.
    ucChromaBlueKeyMask  - Window's surface chroma blue comparison mask. Default
                     value is 0.
    ucChromaBlueKeyHigh  - Window's surface maximum for chroma blue comparison.
                     Default value is 0xff.
    ucChromaBlueKeyLow   - Window's surface mimimum for chroma blue comparison.
                     Default value is 0.

See Also:
    BVDC_Window_GetColorKeyConfiguration
    BVDC_Window_SetColorKeyConfiguration
***************************************************************************/
typedef struct
{
    /* Luma key settings */
    bool                               bLumaKey;
    uint8_t                            ucLumaKeyMask;
    uint8_t                            ucLumaKeyHigh;
    uint8_t                            ucLumaKeyLow;

    /* Chroma Red key settings */
    bool                               bChromaRedKey;
    uint8_t                            ucChromaRedKeyMask;
    uint8_t                            ucChromaRedKeyHigh;
    uint8_t                            ucChromaRedKeyLow;

    /* Chroma Blue key settings */
    bool                               bChromaBlueKey;
    uint8_t                            ucChromaBlueKeyMask;
    uint8_t                            ucChromaBlueKeyHigh;
    uint8_t                            ucChromaBlueKeyLow;

} BVDC_ColorKey_Settings;

/***************************************************************************
Summary:
    This structure describes customized HD_DVI tolerance settings used in
    format detection.

Description:
    This will allow application to fine tune the HD_DVI block.

    ulWidth - This parameter specifies the width tolerance in format detection.
    ulHeight - This parameter specifies the height tolerance in format detection.

See Also:
    BVDC_Source_SetHdDviConfiguration;
***************************************************************************/
typedef struct
{
    /* Width and height tolerance */
    uint32_t                           ulWidth;
    uint32_t                           ulHeight;

} BVDC_HdDvi_FormatTolerance;

/***************************************************************************
Summary:
    This structure describes various HD_DVI customized settings.

Description:
    This will allow application to fine tune the HD_DVI block.

    bEnableDe - This parameter specifies whether to enable De or not
    eInputDataMode - This parameter specifies whether the input data bus is in
    30 bit mode or 24 bit mode.
    bOverrideMux - This parameter specifies whether or not to override internal
    input mux settings for HD_DVI. aucMux will be used to program input mux
    settings if bOverrideMux is true.
    aucMux - This array specifies input mux settings for HD_DVI. This is used to
    program HD_DVI_0_INPUT_MUX_SELECT_0.MUX_DATA_0 to
    HD_DVI_0_INPUT_MUX_SELECT_0.MUX_DATA_35.

See Also:
    BVDC_Source_SetHdDviConfiguration;
***************************************************************************/
typedef struct
{
    bool                               bEnableDe;
    BVDC_HdDvi_InputDataMode           eInputDataMode;

    /* tolerance for format detection */
    BVDC_HdDvi_FormatTolerance         stFmtTolerence;

    bool                               bOverrideMux;
    uint8_t                            aucMux[BVDC_MAX_HDDVI_MUX_COUNT];

} BVDC_HdDvi_Settings;

/***************************************************************************
Summary:
    This structure describes various DNR customized settings.

Description:
    This will allow application to fine tune the DNR block.

    eMnrMode - This parameter indicates if MNR (Mosquito Noise Reduction) should
    be apply or not.

    iMnrLevel - This parameter indicates the level of MNR to apply to the ulQp
    value.  See Qp note below.

    eBnrMode - This parameter indicates if BNR (Block Noise Reduction) should
    be apply or not.

    iBnrLevel - This parameter indicates the level of BNR to apply to the ulQp
    value.  See Qp note below.

    eDcrMode - This parameter indicates if DCR (Digital Contour Reduction) should
    be apply or not.

    iDcrLevel - This parameter indicates the level of DCR to apply to the ulQp
    value.  See Qp note below.

    bUserOffset - This parameter indicates if the offset to be computed by user
    or by VDC internally.  In most cases the VDC internal computed should be good
    especially for digital sources.  For analog source, since the VDC does not
    know if the source has been overscan or adjusted, this allow the application
    to fine tuned the offset to get better filtering.

    ulVertOffset - This parameter is the user's vertical offset use noise
    filtering.  Ignored if bUserOffset=false.

    ulHorzOffset - This parameter is the user's horizontal offset use noise
    filtering.  Ignored if bUserOffset=false.

    ulQp - This parameter specifies the user non-zero ulQp level to be override
    with the value provided by MVD/XVD decoder.  Default value is 0 (hence
    use the ulQp from decoder).

    pvUserInfo - This parameter specifies user customized info to be use for
    filter lookup table.  This is for advanced user only.  Default value is
    NULL.

    Notes: The ulQp from decoder/user specify is further adjusted with iXnrLevel.
    iXnrLevel is used to reduce/amplify the ulQp.  The resulted ulAdjQp is

    ulAdjQp = ulQp * ((iMnrLevel + BVDC_QP_ADJUST_STEPS) / BVDC_QP_ADJUST_STEPS);

    For example, with BVDC_QP_ADJUST_STEPS = 100,
    for DNR, the value of iXnrLevel is in the range of (-100, MAX_INT).
    This give the affective scaling range of ~= (1/100, ((MAX_INT + 100)/100)).
    And for DNR_H, the value of iXnrLevel is in the range of (-100, 200).

See Also:
    BVDC_Source_SetDnrConfiguration;
***************************************************************************/
typedef struct
{
    BVDC_FilterMode                    eMnrMode;
    int32_t                            iMnrLevel;

    BVDC_FilterMode                    eBnrMode;
    int32_t                            iBnrLevel;

    BVDC_FilterMode                    eDcrMode;
    int32_t                            iDcrLevel;

    bool                               bUserOffset;
    uint32_t                           ulVertOffset;
    uint32_t                           ulHorzOffset;

    uint32_t                           ulQp;
    void                              *pvUserInfo;

} BVDC_Dnr_Settings;


/*****************************************************************************
Summary:
    Structure for VDC power standby settings

    pvStub - Stub holder for future expansion.

See Also:
    BVDC_Standby, BVDC_Resume
 *****************************************************************************/
typedef struct BVDC_StandbySettings
{
    void                              *pvStub; /* Stub holder */

} BVDC_StandbySettings;


/***************************************************************************
Summary:
    This structure describes memory configuration settings for VDC window.

Description:
    bUsed - Indicate if this window is used or not
    memcIndex - Defines MEMC index
    eMaxSourceFormat - Defines maximum source format
    ePixelFormat - Defines pixel format: 8-bit 422 or 10-bit 422

    Enable any one of the following flags will increase capture buffer
    count to 2:
    bMosaicMode - Defines mosaic mode, can be enabled by calling
        BVDC_Window_SetMosaicConfiguration
    b3DMode - Defines 3D mode, can be enabled by calling
        BVDC_Display_SetOrientation or BVDC_Source_SetOrientation
    bPsfMode - Defines Psf mode, can be enabled by calling
        BVDC_Source_SetPsfMode. Enable Psf mode will also enable frame
        capture
    bSideBySide - Defines side by side, can be enabled by calling
        BVDC_Window_SetDstRect
    bPip - Defines picture in graphics or picture in picture, can be
        enabled by calling BVDC_Window_SetDstRect. Enable this flag
        will make buffer size 1/4 of full size.
    bSmoothScaling - Defines smooth scaling, can be enabled by calling
        BVDC_Window_SetBandwidthEquationParams
    bBoxDetect - Defines Letter box detection, can be enabled by calling
        BVDC_Window_EnableBoxDetect
    bArbitraryCropping - Defines arbitrary cropping mode, can be enabled
        by calling BVDC_Window_SetPanScanType, BVDC_Window_SetAspectRatioMode,
        BVDC_Window_SetSrcClip
    bIndependentCropping - Defines independent cropping on HD/SD mode,
        can be enabled by BVDC_Window_SetSrcClip.

    Enable any one of the following flags will increase capture buffer
    count to 4:

    bNonMfdSource - Defines non-mfd/vfd source, can be enabled by selecting
        HD_DVI, 656, or mfd/vfd (with MTG enabled,
        BVDC_Source_Settings.eMtgMode) source.

    bSyncSlip - Defines sync slip mode, can be enabled by selecting non
        MFD/VFD source or slaved window for MFD/VFD source

    bLipsync - Defines lip sync, can be enabled by calling
        BVDC_Window_SetDelayOffset. Enable this flag will increase
        capture buffer count by 1

    b5060Convert - Defines 50 <-> 60 conversion, can be enabled by
        selecting different frame rate for source and display. Enable
        this flag will increase capture buffer count by 1

    bSlave_24_25_30_Display - Defines special frame capture on SD display,
        can be enabled by slave to 24/25/30 HD display.  Enable this
        flag or bPsfMode will enable frame capture

    eDeinterlacerMode - Defines deinterlacer, can be enabled by calling
        BVDC_Window_SetDeinterlaceConfiguration. Set this value will
        add deinterlacer buffer counts to final buffer counts

    ulAdditionalBufCnt - Defines additional capture buffer count, can be used
        for delay offset or transcoding buffers

***************************************************************************/
typedef struct
{
    bool                   bUsed;

    /* MEMC settings for capture */
    uint32_t               ulMemcIndex;
    /* MEMC settings for deinterlacer */
    uint32_t               ulMadMemcIndex;

    /* Source settings */
    BFMT_VideoFmt          eMaxSourceFormat;
    bool                   bNonMfdSource;

    /* Window settings */
    bool                   bMosaicMode;
    bool                   b3DMode;
    bool                   bPsfMode;
    bool                   bSideBySide;
    bool                   bPip;
    /* DME: not needed. if stWindow[1] is used, we have PIP. or is this more like "max size"?
    same with bSideBySide. */
    bool                   bSmoothScaling;
    bool                   bBoxDetect;
    /* TODO: bool          bMfdTriggering  */
    bool                   bArbitraryCropping;
    bool                   bIndependentCropping;

    bool                   bSyncSlip;

    bool                   bLipsync;
    bool                   b5060Convert;
    bool                   bSlave_24_25_30_Display;

    BVDC_DeinterlacerMode  eDeinterlacerMode;

    uint32_t               ulAdditionalBufCnt;

} BVDC_WinMemConfigSettings;

/***************************************************************************
Summary:
    This structure describes memory configuration settings for VDC display.

Description:
    bUsed - Indicate if this display is used or not
    eMaxDisplayFormat - Maximum display format

    stWindow - Defines memory configuration settings per video
        window. Default setting: MFD/VFD source, sync locked full screen only,
        no deinterlacer: 0 capture buffers, 0 deinterlacer buffers

See Also:
    BVDC_Heap_Settings
    BVDC_WinMemConfigSettings
    BVDC_GetDefaultMemConfigSettings
    BVDC_GetMemoryConfiguration
***************************************************************************/
typedef struct
{
    /* Display settings */
    bool                       bUsed;
    BFMT_VideoFmt              eMaxDisplayFormat;

    /* Memory config settings for new soft transcoder VIP heap */
    struct {
        bool                      bUsed;
        BCHP_MemoryInfo          *pstMemoryInfo; /* memory stripe parameters dependent */
        BVDC_VipMemConfigSettings stCfgSettings;
    } vip;

    /* Memory config settings for all windows on the display */
    BVDC_WinMemConfigSettings  stWindow[BVDC_MAX_VIDEO_WINDOWS];

} BVDC_DispMemConfigSettings;

/***************************************************************************
Summary:
    This structure describes memory configuration settings for RDC.

Description:
    memcIndex - Defines MEMC index

See Also:
    BVDC_Heap_Settings
    BVDC_WinMemConfigSettings
    BVDC_DispMemConfigSettings
    BVDC_GetDefaultMemConfigSettings
    BVDC_GetMemoryConfiguration
***************************************************************************/
typedef struct
{
    /* MEMC settings */
    uint32_t            ulMemcIndex;

} BVDC_RdcMemConfigSettings;


/***************************************************************************
Summary:
    This structure describes VDC memory configuration settings.

Description:
    stHeapSettings - Defines size of VDC buffers in each category. Only need
        to set pixel format and buffer format: ePixelFormat_4HD, eBufferFormat_4HD,
        ePixelFormat_2HD, eBufferFormat_2HD, ePixelFormat_HD, eBufferFormat_HD,
        ePixelFormat_SD and eBufferFormat_SD

    stDisplay - Defines memory configuration settings per display
    stRdc - Defines memory configuration settings for RDC

See Also:
    BVDC_Heap_Settings
    BVDC_DispMemConfigSettings
    BVDC_GetDefaultMemConfigSettings
    BVDC_GetMemoryConfiguration
***************************************************************************/
typedef struct
{
    BVDC_Heap_Settings          stHeapSettings;

    /* Memory config settings for all displays */
    BVDC_DispMemConfigSettings  stDisplay[BVDC_MAX_DISPLAYS];

    /* Memory config settings for RDC */
    BVDC_RdcMemConfigSettings   stRdc;

} BVDC_MemConfigSettings;


/***************************************************************************
Summary:
    This structure describes VDC memory configuration.

Description:
    stMemc - Defines VDC memory configuration per memc.

        ulSize - Defines total size (byte) of VDC buffers on specific memc.
        stHeapSettings - Defines size and count of VDC buffers in each category
            on specific memc
        ulRulSize - RDC (Regisger DMA Controller) RUL (Regisgter Update List)
            memory.
        ulVipSize - VIP (Video Input Processor) picture memory for new soft transcoder.

***************************************************************************/
typedef struct BVDC_MemConfig
{
    struct {
        uint32_t           ulSize;
        uint32_t           ulRulSize;
        uint32_t           ulVipSize; /* for VIP heap allocation */
        BVDC_Heap_Settings stHeapSettings;

    } stMemc[BVDC_MAX_MEMC];

} BVDC_MemConfig;


/***************************************************************************
Summary:
    This structure describes display Hdmi settings.

Description:
    This will allow application to fine tune the vec hdmi settings.

    eColorComponent - This parameter specifies RGB/YCbCr type (4:2:0/4:2:2/4:4:4).
    eColorRange - This parameter specifies whether limited range or full range.
    eEotf - This parameter specifies whether sdr, gama-hdr, or pq-hdr, ...

See Also:
    BVDC_Display_SetHdmiSettings;
***************************************************************************/
typedef struct
{
    BAVC_Colorspace              eColorComponent;
    BAVC_ColorRange              eColorRange;
    BAVC_HDMI_DRM_EOTF           eEotf;
} BVDC_Display_HdmiSettings;

/***************************************************************************
Summary:
    This structure describes the adjustment of HDR display of SDR graphics

Description:
    This will allow application to adjust the linear approximation of converting
    SDR graphics to HDR for HDMI HDR display

    sYSlider  - Y adjust slider, from -32768 to 32767. The bigger the Y slider
        is, the brighter the display is. Default value is 0.
    sCbSlider - Cb adjust slider, from -32768 to 32767. The bigger the Cb slider
        is, the more blue saturated the display is. Default value is 0.
    sCrSlider - Cr adjust slider, from -32768 to 32767. The bigger the Cr slider
        is, the more red saturated the display is. Default value is 0.

See Also:
    BVDC_Source_SetSdrGfxToHdrApproximationAdjust;
***************************************************************************/
typedef struct
{
    int16_t                      sYSlider;
    int16_t                      sCbSlider;
    int16_t                      sCrSlider;
} BVDC_Source_SdrGfxToHdrApproximationAdjust;

/***************************************************************************
 * Public API
 **************************************************************************/

/***************************************************************************
 * Function prototype
 **************************************************************************/

/***************************************************************************
Summary:
    This function gets the default VDC memory configuration settings.

Description:
    User can use this API to get default VDC memory configuration settings
    on specific chip, then modify the settings based on specific system settings,
    and call BVDC_GetMemoryConfiguration to get VDC buffer counts and buffer
    sizes.

Input:
Output:
    pMemConfigSettings  - pointer to a BVDC_MemConfigSettings struct defines
        default VDC memory configuration settings.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Success.

See Also:
    BVDC_GetMemoryConfiguration
**************************************************************************/
void BVDC_GetDefaultMemConfigSettings
    ( BVDC_MemConfigSettings             *pMemConfigSettings );

/***************************************************************************
Summary:
    This function gets the VDC memory configuration, which includes VDC
    buffer counts and buffer sizes.

Description:
    User can call BVDC_GetDefaultMemConfigSettings to get default VDC memory
    configuration settings on specific chip, then modify the settings based on
    specific system settings, and call this API to get VDC buffer counts and
    buffer sizes.

Input:
    pMemConfigSettings  - pointer to a BVDC_MemConfigSettings struct
        defines VDC memory configuration settings.
Output:
    pMemConfig  - pointer to a BVDC_MemConfig struct defines VDC memory
        configuration, which inlcudes buffer counts and buffer sizes.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Success.

See Also:
    BVDC_GetDefaultMemConfigSettings
**************************************************************************/
BERR_Code BVDC_GetMemoryConfiguration
    ( const BVDC_MemConfigSettings       *pMemConfigSettings,
      BVDC_MemConfig                     *pMemConfig );

/***************************************************************************
Summary:
    Get the vdc's capabilities.

Description:
    This function gets the vdc's capabilities.

Input:
    hVdc - A valid vdc handle created earlier.

Output:
    pCapabilities - point to the capabilities structure.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_Capabilities
    BVDC_Compositor_Capabilities
**************************************************************************/
BERR_Code BVDC_GetCapabilities
    ( BVDC_Handle                      hVdc,
      BVDC_Capabilities               *pCapabilities );


/***************************************************************************
Summary:
    This function gets BVDC's inherent default setting structure.

Description:
    BDVC's inherent default setting structure could be queried by this API
    function prior to BVDC_Open, modified and then passed to BVDC_Open.
    This save application tedious work of filling in the configuration
    structure.

Input:
    hBox         - Handle to the box mode

Output:
    pDefSettings - A reference to default settings structure.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get VDC default settings.

See Also:
    BVDC_Open.
**************************************************************************/
BERR_Code BVDC_GetDefaultSettings
    ( const BBOX_Handle                hBox,
      BVDC_Settings                   *pDefSettings );

/***************************************************************************
Summary:
    This function opens and initializes the video display control block.

Description:
    The first thing one need to do is open a VDC (also create a VDC
    main context handle, BVDC_Handle).  This will hold the states
    of VDC hardware blocks.  From this handle user create number
    of compositors (number is chip dependence); where display window
    will be created from.  This handle is required when we call
    BVDC_ApplyChanges to update the hardware.  This handle eventually be
    closed by calling BVDC_Close.

Input:
    hChp - The chip handle that application created earlier during chip
    initialization sequence.  This handle is use for get chip
    information, chip revision, and miscellaneous chip configurations.

    hReg - The register handle that application created earlier during
    chip initialization sequence.  This handle is use to access chip
    registers (VDC registers).

    hMem - The local memory heap handle.  VDC uses local memory for
    various needs like video capture/playback.

    hInt - The level2 interrupt handle that application created
    earlier chip initialization sequence.  This handle is use to install
    level 2 interrupt callback.

    hRdc - The register DMA control handle that application created earlier
    during chip initialization sequence.  This handle is use to setup up
    Register Update List (RUL) and to be execute at a specific trigger in
    short amount of times.

    hTmr - The timer handle that application created earlier during chip
    initialization sequence.  This handle is used by VDC to handle interrupt
    service order, and/or uses for timestamp managements.

    pDefSettings - The default setting that user want the BVDC to be
    in. This parameter can be NULL. In this case BVDC's inherent default
    structure will be used. This inherent structure could be queried prior
    to BVDC_Open with BVDC_GetDefaultSettings, modified and passed to
    BVDC_Open.

Output:
    phVdc - a reference to a VDC handle.  Upon successful open this will
    reference to a fresh new VDC handle (context).  If error occur during
    BVDC_Open *phVdc be NULL.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully opened VDC.

See Also:
    BVDC_Close, BVDC_GetDefaultSettings.
**************************************************************************/
BERR_Code BVDC_Open
    ( BVDC_Handle                     *phVdc,
      BCHP_Handle                      hChp,
      BREG_Handle                      hReg,
      BMMA_Heap_Handle                 hMma,
      BINT_Handle                      hInt,
      BRDC_Handle                      hRdc,
      BTMR_Handle                      hTmr,
      const BVDC_Settings             *pDefSettings );

/***************************************************************************
Summary:
    This function closes the video display control block.

Description:
    Upon terminating video usage application needs to call BVDC_CLose to
    close video display control to free internal allocated resources.
    After closed VDC is an invalid handle, and should not be use with any
    other VDC functions.  Failure to close will result in undefine
    consequences.
    Application is responsible for close/destroy/free other allocated
    resources; compositor handles, video and graphics window handles before
    calling BVDC_Close.  BVDC_Close returns success if hVdc is NULL.

Input:
    hVdc - Video display control handle that was previously opened by
    BVDC_Open.

Output:
    hVdc - Video display control handle becomes invalid.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully closed VDC.

See Also:
    BVDC_Open.
**************************************************************************/
BERR_Code BVDC_Close
    ( BVDC_Handle                      hVdc );

/***************************************************************************
Summary:
    This function "Apply Changes" and update hardware in the video display
    control block.

Description:
    After all settings in the user mode context have been made which
    include configuring the sources, destinations, and various parameters,
    BVDC_ApplyChanges will "apply" the "changes" to hardware.  It is
    possible the the context handle is not setup correctly.  In that case an
    error will return, and no hardware will be updated.  The hardware it
    update will depend on what resources (capture, playback, scaler, and so
    on) are being used.  This function is optimized to only update required
    registers.

    BVDC_ApplyChanges actually uses register DMA to queue up register
    lists that executed when appropriate trigger fires.

    Please notice that the setting in interrupt handle context, using the
    API functions named with an "_isr" suffix, applies to the hardware
    immediately, regardless of the calling of BVDC_ApplyChanges.

Input:
    hVdc - Video display control handle that was previously opened by
    BVDC_Open, and setup with BVDC calls.

Output:
    <None>

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully updated hardware.

See Also:
    BREG_Handle.
**************************************************************************/
BERR_Code BVDC_ApplyChanges
    ( BVDC_Handle                      hVdc );

/***************************************************************************
Summary:
    This function "Abort Changes" that recently set, but not yet applied or
    has been applied but failed.

Description:
    After all settings in the user mode context have been made which
    include configuring the sources, destinations, and various parameters,
    BVDC_ApplyChanges will "apply" the "changes" to hardware.  In the case
    BVDC_ApplyChanges fails, and user wanted to abort all the new changes.
    This is meant to recover from the last good applied.  This function
    take effect immediately.

Input:
    hVdc - Video display control handle that was previously opened by
    BVDC_Open, and setup with BVDC calls.

Output:
    <None>

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully updated hardware.

See Also:
    BVDC_ApplyChanges, and all the set functins.
**************************************************************************/
BERR_Code BVDC_AbortChanges
    ( BVDC_Handle                      hVdc );

/***************************************************************************
Summary:
    This function gets the maximum number of compositor the video display
    control has.

Description:
    For informational only.  The number of compositor to be created from
    VDC handle can not exceed this max value.

Input:
    hVdc - Video display control handle that was previously opened by
    BVDC_Open, and setup with BVDC calls.

Output:
    pulCompositorCount - A reference to store the value.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully queried the number of compositor.

See Also:
    BVDC_Compositor_GetMaxWindowCount.
**************************************************************************/
BERR_Code BVDC_GetMaxCompositorCount
    ( const BVDC_Handle                hVdc,
      uint32_t                        *pulCompositorCount );

/*****************************************************************************
Summary:
    Get default standby settings.

See Also:
    BVDC_Standby
*****************************************************************************/
void BVDC_GetDefaultStandbySettings
    ( BVDC_StandbySettings            *pStandbypSettings );

/*****************************************************************************
Summary:
    Enter standby mode

Description:
    This function enters standby mode with the VDC module, if supported.
    The VDC modules (displays/compositors/sources/window must not be in use in
    order to successfully enter standby mode.
    If standby mode is not supported, calling this function has no effect.

    When in standby mode, the device clocks are turned off, resulting in a
    minimal power state.

    No BVDC_* calls should be made until standby mode is exitted by calling
    BVDC_Resume().  Calling BVDC_* api at while in standy result in
    undefined results.

Returns:
    BERR_SUCCESS - If standby is successful, otherwise error

See Also:
    BVDC_Resume
*****************************************************************************/
BERR_Code BVDC_Standby
    ( BVDC_Handle                      hVdc,
      const BVDC_StandbySettings      *pStandbypSettings );

/*****************************************************************************
Summary:
    Exit standby mode

Description:
    This function exits from standby mode with the VDC module, if supported.
    After exitting standby mode, upper-level SW is free to call
    BVDC_* functions.

Returns:
    BERR_SUCCESS - If standby is successful, otherwise error

See Also:
    BVDC_Standby
*****************************************************************************/
BERR_Code BVDC_Resume
    ( BVDC_Handle                      hVdc );

/***************************************************************************
Summary:
    This function gets the maximum area coverage for all the mosaic
    rectangles on a given display in mosaic mode.

Description:

Input:
    hVdc - A valid VDC handle created earlier.
    eDisplayId - display Id
    ulRectsCount - Number of Mosaic rectangles; must <= BAVC_MOSAIC_MAX

Output:
    pulCoverage - maximum area coverage in percentage for all mosaic rectangles.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
**************************************************************************/
BERR_Code BVDC_GetMaxMosaicCoverage
    ( BVDC_Handle                      hVdc,
      BVDC_DisplayId                   eDisplayId,
      uint32_t                         ulRectsCount,
      uint32_t                        *pulCoverage );

/***************************************************************************
 * Heap
 **************************************************************************/

/***************************************************************************
Summary:
    This function creates a heap from a given memory.

Description:
    A heap handle is used for allocating/deallocating buffers needed by a
    window. The application must know the use of this heap as this is
    separate from the heap created internally by the VDC. This heap is then
    passed in to BVDC_Window_Create via the BVDC_Window_Settings structure.

Input:
    hVdc - Handle to the VDC module.

    hMem - The memory to create the heap from.

    *pDefSettings - Specifies the type and number of buffers that are to be
    created for the heap. This must be populated by the user, hence, must
    not be NULL.

Output:
    *phHeap - The heap handle to be created

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully created a window handle.

See Also:
    BVDC_Heap_Destroy, BVDC_Window_Create, BVDC_Heap_Settings.
**************************************************************************/
BERR_Code BVDC_Heap_Create
    ( BVDC_Handle                      hVdc,
      BVDC_Heap_Handle                *phHeap,
      BMMA_Heap_Handle                 hMma,
      const BVDC_Heap_Settings        *pDefSettings );

/***************************************************************************
Summary:
    This function destroys a heap created from a second memory.

Description:
    The application must know which heap is to be destroyed. The VDC cannot
    distinguish the heap to be destroyed unless it is the VDC created
    internal heap.

Input:
    hHeap - The heap handle to be destroyed

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully created a heap handle.

See Also:
    BVDC_Heap_Create
**************************************************************************/
BERR_Code BVDC_Heap_Destroy
    ( BVDC_Heap_Handle                 hHeap );


/***************************************************************************
 * Video & Graphics Compositor
 **************************************************************************/

/***************************************************************************
Summary:
    This function gets BVDC_Compositor default setting structure.

Description:
    BDVC_Compositor_Settings inherent default setting structure could be
    queried by this API function prior to BVDC_Compositor_Create, modified and
    then passed to BVDC_Compositor_Create.  This save application tedious
    work of filling in the configuration structure.

Input:
    eCompositorId - Default settings for specific compositor.

Output:
    pDefSettings - A reference to default settings structure.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get BVDC_Compositor_Settings default settings.

See Also:
    BVDC_Compositor_Create, BVDC_Compositor_Settings.
**************************************************************************/
BERR_Code BVDC_Compositor_GetDefaultSettings
    ( BVDC_CompositorId                eCompositorId,
      BVDC_Compositor_Settings        *pDefSettings );

/***************************************************************************
Summary:
    This function creates and initializes a compositor.

Description:
    A compositor contains a number of graphics windows, video windows.
    Before application can create a video window
    it will need a compositor handle.  From this compositor handle
    application can create many video and grahpics windows, and hardware
    cursors.  These will get blended together into a single video stream
    or BVB which get sent to VEC.

    Does not take immediate effect.  Requires an ApplyChanges() call.

Input:
    hVdc - The VDC handle that application created earlier.

    eCompositorId - A compositor enumeration that has special attributes
    associated with.

    pDefSettings - Should be NULL for standard usage modes.

Output:
    phCompositor - a reference to a compositor handle.  Upon successful
    open this will reference to a fresh new compositor handle (context).
    If error occur during BVDC_Compositor_Create *phCompositor be NULL.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully created a compositor handle.

See Also:
    BVDC_Compositor_Destroy
**************************************************************************/
BERR_Code BVDC_Compositor_Create
    ( BVDC_Handle                      hVdc,
      BVDC_Compositor_Handle          *phCompositor,
      BVDC_CompositorId                eCompositorId,
      const BVDC_Compositor_Settings  *pDefSettings );

/***************************************************************************
Summary:
    This function destroys a compositor.

Description:
    Destroy the compositor handle, and releasing internal resources.
    Prior to calling BVDC_Compositor_Destroy application must destory
    all windows created from this compositor.  Failure to destory
    will result in undefine consequences.

    Application is responsible for destroy/free other resources;
    window handles before calling BVDC_Compositor_Destroy.

    Does not take immediate effect.  Requires an ApplyChanges() call.

Input:
    hCompositor - A compositor handle that was previously created
    by BVDC_Compositor_Create.

Output:
    hCompositor - A compositor handle becomes invalid.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully destroyed of a compositor.

See Also:
    BVDC_Compositor_Create.
**************************************************************************/
BERR_Code BVDC_Compositor_Destroy
    ( BVDC_Compositor_Handle           hCompositor );

/***************************************************************************
Summary:
    This function gets the maximum number of video or graphics window
    this compositor has.

Description:
    For informational only.  The number of window to be created from this
    compositor handle can not exceed this max value.

Input:
    hCompositor - A compositor handle that was previously created
    by BVDC_Compositor_Create.

Output:
    pulVideoWindowCount - A reference to store the maximum video window count.
    pulGfxWindowCount - A reference to store the maximum graphics window count.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully queried the number of video or graphics
    windows.

See Also:
**************************************************************************/
BERR_Code BVDC_Compositor_GetMaxWindowCount
    ( const BVDC_Compositor_Handle     hCompositor,
      uint32_t                        *pulVideoWindowCount,
      uint32_t                        *pulGfxWindowCount );


/***************************************************************************
Summary:
    This function sets the background color for this compositor.

Description:
    Set the background for this compositor.  This background color is also
    alpha blended with other video windows and graphics windows.
    Background color is only available for BVDC_CompositorId_eCompositor0
    and BVDC_CompositorId_eCompositor1.  Since BVDC_CompositorId_eCompositor2
    (7038/3560) is only capable of composing a single video window there will
    be no background for these compositors.  Valid values for uchRed, uchGreen,
    and uchBlue are from 0-255.

    Default value is ucRed=0x00, ucGreen=0x00, ucBlue=0x00.  Black background.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hCompositor - A compositor handle that was previously created
    by BVDC_Compositor_Create.
    ucRed - The RED component of the background color.
    ucGreen - The GREEN component of the background color.
    ucBlue -  The BLUE component of the background color.

Output:
    <None>

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the background color.

See Also:
    BVDC_Window_SetAlpha, BVDC_Window_SetBlendFactor
**************************************************************************/
BERR_Code BVDC_Compositor_SetBackgroundColor
    ( BVDC_Compositor_Handle           hCompositor,
      uint8_t                          ucRed,
      uint8_t                          ucGreen,
      uint8_t                          ucBlue );

/***************************************************************************
Summary:
    This function gets the background color of this compositor.

Description:
    Get the background of this compositor.  This background color is also
    alpha blended with other video windows and graphics windows.
    Background color is only available for BVDC_CompositorId_eCompositor0
    and BVDC_CompositorId_eCompositor1.  Since BVDC_CompositorId_eCompositor2
    (7038/3560) is only capable of composing a single video window there will
    be no background for these compositors.  In this case puchRed, puchGreen,
    and puchBlue will remain unchanged.  The outputs can be a NULL pointer.

Input:
    hCompositor - A valid compositor handle created earlier.

Output:
    pucRed - The RED component of the background color.
    pucGreen - The GREEN component of the background color.
    pucBlue -  The BLUE component of the background color.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the background color.

See Also:
    BVDC_Window_SetAlpha.
**************************************************************************/
BERR_Code BVDC_Compositor_GetBackgroundColor
    ( const BVDC_Compositor_Handle     hCompositor,
      uint8_t                         *pucRed,
      uint8_t                         *pucGreen,
      uint8_t                         *pucBlue );


/***************************************************************************
Summary:
    This function set the luma sum configuration for a compositor.

Description:
    Specify what is the region of the compositor to compute the luma sum.
    The region must be bounded by the compositor's canvas.  For example
    if the compositor/display output is 1080p, then the region is
    1920x1080.

Input:
    hCompositor - A compositor handle that was previously created
    by BVDC_Compositor_Create.

    pLumaSettings - A pointer contains the settings.  If pLumaSettings is NULL
    The region will be size of compositor's format active video.  For
    example if the format 720p the size will be 1280x720p, or if 1080i the
    the region will be 1920x1080i.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.  If the compositor
       does not support luma sum computation.

    BERR_SUCCESS - Successfully queried the luma sum.

See Also:
    BVDC_LumaSettings,
    BVDC_Compositor_GetLumaStatus,
    BVDC_Compositor_GetLumaStatsConfiguration
**************************************************************************/
BERR_Code BVDC_Compositor_SetLumaStatsConfiguration
    ( BVDC_Compositor_Handle           hCompositor,
      const BVDC_LumaSettings         *pLumaSettings );


/***************************************************************************
Summary:
    This function get the luma sum configuration for a compositor.

Description:
    Get the specified luma sum settings.

Input:
    hCompositor - A compositor handle that was previously created
    by BVDC_Compositor_Create.

Output:
    pLumaSettings - A reference to store the settings.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.  If the compositor
       does not support luma sum computation.

    BERR_SUCCESS - Successfully queried the luma sum.

See Also:
    BVDC_LumaSettings,
    BVDC_Compositor_GetLumaStatus,
    BVDC_Compositor_SetLumaStatsConfiguration
**************************************************************************/
BERR_Code BVDC_Compositor_GetLumaStatsConfiguration
    ( const BVDC_Compositor_Handle     hCompositor,
      BVDC_LumaSettings               *pLumaSettings );


/***************************************************************************
Summary:
    This function gets the luma sum of current field/frame of this
    compositor.

Description:
    The luma sum value can be use to dynamically adjust backlit of the output
    depend on content.  It's envision that application will period read this
    luma sum value, and adjust backlit, brightness, and etc to their desired
    output.  The value of pLumaStatus is the value of the last field/frame.
    In the case of chipset of that does not support luma sum pLumaStatus
    will not be updated by this function.

Input:
    hCompositor - A compositor handle that was previously created
    by BVDC_Compositor_Create.

Output:
    pLumaStatus - A reference to store the luma sum status.  In the case that
    the chipset does not support luma min/max the compositor will return
    pLumaStatus->ulMin pLumaStatus->ulMax = 0.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully queried the luma sum.

See Also:
    BVDC_LumaStatus,
    BVDC_Window_SetLumaStatsConfiguration.
**************************************************************************/
BERR_Code BVDC_Compositor_GetLumaStatus
    ( const BVDC_Compositor_Handle     hCompositor,
      BVDC_LumaStatus                 *pLumaStatus );


/***************************************************************************
Summary:
    This function sets the OSD configuration for a compositor.

Description:
    The purpose of this function is to allow application to marked
    composited output of compositor as "having graphics" or "not".  The reason
    for this is to allow post-processing video chipsets to use the output
    of compositor but ignore the graphics pixels.

    The LVDS output has some extra bits which are being re-used for this
    purpose.  These bits will be set based on whether or not the alpha of the
    graphic is above or below a certain threshold
    BVDC_OsdSettings.ucOsdSelectThreshold
    (corresponding to CMP_0_OSD_SELECT_THRESHOLD)

Input:
    hCompositor - A compositor handle that was previously created
    by BVDC_Compositor_Create.

    pOsdSettings - A pointer contains the settings.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.  If the compositor
       does not support OSD configuration.

    BERR_SUCCESS - Successfully set the OSD configuration.

See Also:
    BVDC_OsdSettings,
    BVDC_Compositor_GetOsdConfiguration
**************************************************************************/
BERR_Code BVDC_Compositor_SetOsdConfiguration
    ( BVDC_Compositor_Handle           hCompositor,
      const BVDC_OsdSettings          *pOsdSettings );

/***************************************************************************
Summary:
    This function gets the OSD configuration settings for a compositor.

Description:
    This function returns the OSD setting of a compositor.

Input:
    hCompositor - A compositor handle that was previously created
    by BVDC_Compositor_Create.

Output:
    pOsdSettings - A reference to store the OSD settings.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters. If the compositor
    does not support OSD configuration.
    BERR_SUCCESS - Successfully queried the OSD configuration.

See Also:
    BVDC_OsdSettings,
    BVDC_Compositor_SetOsdConfiguration
**************************************************************************/
BERR_Code BVDC_Compositor_GetOsdConfiguration
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_OsdSettings                *pOsdSettings );

/***************************************************************************
Summary:
    This function configures a compositor's Color Clip slopes/joints.

Description:
    The job of the color clip is to gracefully remap out of range RGB values in a
    graceful way before hardware does the clipping to 0 or 1023.  What gets clipped
    is purely a function of the matrix which converts display-colorspace YCbCr to
    display-colorspace RGB.  This matrix is completely determined by the display's
    primaries and white point.

    hCompositor          - a compositor handle.
    pstColorClipSettings - settings with all joints/slopes.

See Also:
    BVDC_ColorClipSettings
    BVDC_Compositor_GetColorClip
***************************************************************************/
BERR_Code BVDC_Compositor_SetColorClip
    ( BVDC_Compositor_Handle           hCompositor,
      const BVDC_ColorClipSettings    *pstColorClipSettings );

/***************************************************************************
Summary:
    This function gets a compositor's Color Clip slopes/joints settings.

Description:
    The job of the color clip is to gracefully remap out of range RGB values in a
    graceful way before hardware does the clipping to 0 or 1023.  What gets clipped
    is purely a function of the matrix which converts display-colorspace YCbCr to
    display-colorspace RGB.  This matrix is completely determined by the display's
    primaries and white point.

    hWindow              - a window handle.
    pstColorClipSettings - settings with all joints/slopes.

See Also:
    BVDC_ColorClipSettings
    BVDC_Compositor_GetColorClip
***************************************************************************/
BERR_Code BVDC_Compositor_GetColorClip
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_ColorClipSettings          *pstColorClipSettings );


/***************************************************************************
Summary:
    Get the compositor's capabilities.

Description:
    This function gets the compositor's capabilities.

Input:
    hCompositor - A valid compositor handle created earlier.

Output:
    pCapabilities - point to the compositor capabilities structure.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Compositor_Capabilities
**************************************************************************/
BERR_Code BVDC_Compositor_GetCapabilities
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_Compositor_Capabilities    *pCapabilities );


/***************************************************************************
 * Video/Gfx Window
 **************************************************************************/

/***************************************************************************
Summary:
    This function gets window default setting structure.

Description:
    BVDC_Window_Settings inherent default setting structure could be
    queried by this API function prior to BVDC_Window_Create, modified and
    then passed to BVDC_Window_Create.  This save application tedious
    work of filling in the configuration structure.

Input:
    eWindowId - Default settings for specific window.

Output:
    pDefSettings - A reference to default settings structure.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get BVDC_Window_Settings default settings.

See Also:
    BVDC_Window_Create.
**************************************************************************/
BERR_Code BVDC_Window_GetDefaultSettings
    ( BVDC_WindowId                    eWindowId,
      BVDC_Window_Settings            *pDefSettings );

/***************************************************************************
Summary:
    This function creates a window handle.

Description:
    A window handle is to control a video or graphics to be shown on a
    compositor.  Application can control various configurations of a window
    like it size and positions.  Scaling and clipping is achieved by
    control the source, scaler output, and destination rectangles.

    Special note for MPEG/AVC source (source created with
    BAVC_SourceId_eMpeg0 and BAVC_SourceId_eMpeg1).  These sources are from
    internal digital decoder which which source is slaved (or synclock) to a
    display/compositor (BVDC_Display_Handle).

    Does not take immediate effect.  Requires an ApplyChanges() call.

Input:
    hCompositor - A compositor handle that was previously created
    by BVDC_Window_Create.

    hSource - The initial source handle connected to this window. It
    determines the type of the window, i.e. video or graphics window.

    eId - Specify what window Id should VDC tries to open. eId must be a
    valid window id for this chip set.  The id must also match with the source.
    That is video window goes with video source, and graphics window goes with
    graphics source.  Error will be return if source and window does not match.

    pDefSettings - Should be NULL for standard usage modes.

Output:
    phWindow - With a successful BVDC_Window_Create this BVDC_WindowHandle
    will be ready to for use.  If failure occurred phWindow will be NULL.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully created a window handle.
    BERR_NO_HARDWARE - The number of window has been exhausted.

See Also:
    BVDC_Compositor_GetMaxWindowCount, BVDC_Window_Destroy,
    BVDC_Heap_Create, BVDC_Window_Settings.
**************************************************************************/
BERR_Code BVDC_Window_Create
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_Window_Handle              *phWindow,
      BVDC_WindowId                    eId,
      BVDC_Source_Handle               hSource,
      const BVDC_Window_Settings      *pDefSettings );

/***************************************************************************
Summary:
    This function destroys a window handle.

Description:
    This function destroy window.  It will free the internal resources,
    and the window will be disappear from the outputs.  Application is
    still responsible for closing/destroy the source; for example if source
    is MPEG it would need to stop decoding.

    Does not take immediate effect.  Requires an ApplyChanges() call.

Input:
    <None>

Output:
    hWindow - This window handle become invalid after BVDC_Window_Destroy.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully destroyed the window handle.

See Also:
    BVDC_Window_Create.
**************************************************************************/
BERR_Code BVDC_Window_Destroy
    ( BVDC_Window_Handle               hWindow );

/***************************************************************************
Summary:
    This function sets the source clip rectangle of a window.

Description:
    This function selects portion of source rectangle to be used for further
    process, such as scale and display. One can think of zooming into a
    particular region of the source.

    The source clip rectangle should be completely within the original source
    rectangle. The original source width, height might be from decoded stream,
    or from a graphics surface.  If specify values happen to be overclipped
    no clipping will be performed.

    If BVDC_Window_SetSrcClip is never called for a window, its whole
    source rectangle is used for further process.

    When pan scan, or box detect and auto-black-cut, is enabled, source
    clip is performed after them. In another word, the "source rectangle"
    mentioned above is the "pan scan rectangle" or the "active video box",
    and not the "original source rectangle".

    Due to internal hw storage of pixel in YUV 4:2:2 format, which require
    size and starting pixel to be even number.  In this case software will
    round down to require hardware size.

    This setting will not be applied until a call to BVDC_ApplyChanges is
    made.

Input:
    hWindow - A valid window handle created earlier.
    ulLeft - Amount of pixel to clip from the left edge (default 0).
    ulRight - Amount of pixel to clip from the right edge (default 0).
    ulTop - Amount of pixel to clip from the top edge (default 0).
    ulBottom - Amount of pixel to clip from the bottom edge (default 0).

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the source clip rectangle of a window.

See Also:
    BVDC_Window_GetSrcClip,
    BVDC_Window_SetScalerOutput, BVDC_Window_SetDstRect,
    BVDC_Window_EnableBoxDetect, BVDC_Window_SetPanScanType,
    BVDC_Window_SetAspectRatioMode, BVDC_Window_SetScaleFactorRounding
**************************************************************************/
BERR_Code BVDC_Window_SetSrcClip
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulLeft,
      uint32_t                         ulRight,
      uint32_t                         ulTop,
      uint32_t                         ulBottom );


/***************************************************************************
Summary:
    This function sets the source clip offset of the right window.

Description:
    This function allows the user to set the source clip X offset delta for
    the right window in 3D mode.

Input:
    hWindow - A window handle that was previously created by BVDC_Window_Create.
    lLeftDelta - Right window source clip X offset delta is relative to left
              window source clip X offset.
              source clip(Right window) = source clip (Left window) + lLeftDelta

Output:
    None.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the source clip X-offset

See Also:
    BVDC_Window_SetSrcClip
    BVDC_Window_GetSrcRightClip
**************************************************************************/
BERR_Code BVDC_Window_SetSrcRightClip
    ( BVDC_Window_Handle               hWindow,
      int32_t                          lLeftDelta );

/***************************************************************************
Summary:
    This function gets the source clip detla of the right window.


Input:
    hWindow - A compositor handle that was previously created by BVDC_Window_Create.

Output:
    plLeftDelta - Right window source clip delta is relative to left window
              source clip.
              source clip(Right window) = source clip(Left window) + lLeftDelta

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the plRWinXOffsetDelta.

See Also:
    BVDC_Window_SetSrcRightClip
**************************************************************************/
BERR_Code BVDC_Window_GetSrcRightClip
    ( const BVDC_Window_Handle         hWindow,
      int32_t                         *plLeftDelta );


/***************************************************************************
Summary:
    This function gets the source clip rectangle of a window.

Description:
    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - A valid window handle created earlier.

Output:
    pulLeft - Amount of pixel to clip from the left edge (default 0).
    pulRight - Amount of pixel to clip from the right edge (default 0).
    pulTop - Amount of pixel to clip from the top edge (default 0).
    pulBottom - Amount of pixel to clip from the bottom edge (default 0).

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the source clip rectangle of a window.

See Also:
    BVDC_Window_SetSrcClip.
**************************************************************************/
BERR_Code BVDC_Window_GetSrcClip
    ( const BVDC_Window_Handle         hWindow,
      uint32_t                        *pulLeft,
      uint32_t                        *pulRight,
      uint32_t                        *pulTop,
      uint32_t                        *pulBottom );

/***************************************************************************
Summary:
    This function sets the destination rectangle of a window.

Description:
    This function specifies the destination rectangle, inside the canvas'
    active video region, to show the window.  The destination is bounded by
    the current display video format.  This function is typically called
    together with the display video format re-setting. And in this case
    BVDC_Window_SetScalerOutput should also be called to specify how to
    scale the (clipped) source and how to clip the scaled picture for final
    display.

    (lLeft, lTop) = (0,0) means the first active pixels on screen.  The
    width and height is bounded by the display format's active video region
    size.

    The default destination rectangle is the whole active video region of
    the canvas at the time when the window is created, that is determined
    by the display video format.

    Please notice that video of the same format could have different active
    video height when it is from different type of source or output interface.
    As setting the destination rectangle, user need to take account of the
    source height, BVDC internal height (refer to BFMT), and the output
    height, to map the source video to the display canvas correctly.

    For example, when a MPEG video of NTSC format is displayed on an 480i
    NTSC TV, it might be proper to map a 720x480 MPEG video source to the
    lower 480 lines of the 720x482 canvas, in order to match the ARIB spec.
    To do so, the destination rectangle's lTop and ulHeight needs to be set
    as 2 and 480 respectively.

    Due to internal hw storage of pixel in YUV 4:2:2 format, which require
    size and starting pixel to be even number.  In this case software will
    round down to require hardware size.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - A valid window handle created earlier.
    lLeft - relative to the active video rectangle of the display canvas.
    lTop  - relative to the active video rectangle of the display canvas.
    ulWidth  - default to the active video width of the display canvas.
    ulHeight - default to the active video heigh of the display canvas.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set destination rectangle of a window.

See Also:
    BVDC_Display_SetVideoFormat,
    BVDC_Window_GetDstRect,
    BVDC_Window_SetSrcClip, BVDC_Window_SetScalerOutput,
    BVDC_Window_EnableBoxDetect, BVDC_Window_SetPanScanType,
    BVDC_Window_SetAspectRatioMode, BVDC_Window_SetScaleFactorRounding
**************************************************************************/
BERR_Code BVDC_Window_SetDstRect
    ( BVDC_Window_Handle               hWindow,
      int32_t                          lLeft,
      int32_t                          lTop,
      uint32_t                         ulWidth,
      uint32_t                         ulHeight );


/***************************************************************************
Summary:
    This function gets the destination rectangle of a window.

Description:
    This function gets the window's destination rectangle.  The
    destination is bouned by the current output format of the compositor.

Input:
    hWindow - A valid window handle created earlier.

Output:
    plLeft - relative to the active video rectangle of the display canvas.
    plTop  - relative to the active video rectangle of the display canvas.
    pulWidth  - default to the active video width of the display canvas.
    pulHeight - default to the active video heigh of the display canvas.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the destination rectangle of a window.

See Also:
    BVDC_Window_SetDstRect.
**************************************************************************/
BERR_Code BVDC_Window_GetDstRect
    ( const BVDC_Window_Handle         hWindow,
      int32_t                         *plLeft,
      int32_t                         *plTop,
      uint32_t                        *pulWidth,
      uint32_t                        *pulHeight );

/***************************************************************************
Summary:
    This function sets the scaler output.

Description:
    The clipped source rectangle is typically scaled (up or down), and then
    the whole scaled picture, or a sub-rectangle of it, is displayed on the
    screen area defined by the destination rectangle.

    The finally displayed (whole or sub) rectangle area in the scaled picture,
    is called destination clip rectangle. Its width and height are defined to
    be the same as the destination rectangle.

    This functions specifies how much we want to scale up/down the clipped
    source rectangle, and the relative offset of the destination clip
    rectangle inside the scaled picture.

    In some applications, the clipped source rectangle might be scaled to
    a size a little bit bigger than the size of the destination rectangle,
    and the destination clipping is used to get rid of the unclean edge of
    the scaled picture.

    Due to hardware limitation, graphics window can not perform vertical
    scaling. In this case, ulHeight specified by BVDC_Window_SetScalerOutput
    must be the same as the clipped graphics surface height.

    The default scaler-out rectangle has the size of the whole active video
    region of the canvas at the time when the window is created, that is
    determined by the display video format. The default offset of the
    destination clip rectangle inside the scaler-out is (0, 0).

    Due to internal hw storage of pixel in YUV 4:2:2 format, which require
    size and starting pixel to be even number.  In this case software will
    round down to require hardware size.

    Does not take immediate effect. Requires a BVDC_ApplyChanges() call.

Input:
    hWindow - A valid window handle created earlier.
    ulLeft - Horizontal offset of destination clip rect within the "scaling
        coordinate space"  - used to define what is being fed from scaler's
        output rectangle.
    ulTop - Vertical offset of destination clip rect within the "scaling
        coordinate space" - used to define what is being fed from scaler's
        output rectangle.
    ulWidth  - Width of the scaled picture - used to define the horizontal
        scaling factor.
    ulHeight -  Height of the scaled picture - used to define the vertical
        scaling factor.
Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the scaler output.

See Also:
    BVDC_Window_GetScalerOutput,
    BVDC_Window_SetSrcClip, BVDC_Window_SetDstRect,
    BVDC_Window_EnableBoxDetect, BVDC_Window_SetPanScanType,
    BVDC_Window_SetAspectRatioMode, BVDC_Window_SetScaleFactorRounding
**************************************************************************/
BERR_Code BVDC_Window_SetScalerOutput
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulLeft,
      uint32_t                         ulTop,
      uint32_t                         ulWidth,
      uint32_t                         ulHeight );


/***************************************************************************
Summary:
    This function gets the scaler output rectangle.

Description:
    This functions gets the scaler output rectangle.

Input:
    hWindow - A valid window handle created earlier.
    When detect "Destination Rectangle" change this output rectangle will
    reset to the new "Destination Rectangle".

Output:
    pulLeft - Horizontal offset of destination clip rect within the "scaling
        coordinate space"  - used to define what is being fed from scaler's
        output rectangle.
    pulTop - Vertical offset of destination clip rect within the "scaling
        coordinate space" - used to define what is being fed from scaler's
        output rectangle.
    pulWidth  - Width of the scaled picture - used to define the horizontal
        scaling factor.
    pulHeight -  Height of the scaled picture - used to define the vertical
        scaling factor.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the destination clip rectangle of a
    window.

See Also:
    BVDC_Window_SetScalerOutput
**************************************************************************/
BERR_Code BVDC_Window_GetScalerOutput
    ( const BVDC_Window_Handle         hWindow,
      uint32_t                        *pulLeft,
      uint32_t                        *pulTop,
      uint32_t                        *pulWidth,
      uint32_t                        *pulHeight );

/***************************************************************************
Summary:
    This function sets the window alpha value of a window.

Description:
    This function specifies the window alpha value of a window.

    For a video window, this alpha value is used as "source alpha" during
    blending it onto the destination picture.

    If the window is a graphics window, this alpha is typically multiplied
    with the pixel alpha, and the per-pixel product is then used as "source
    alpha" during blending the graphics window onto destination picture,
    similarly as the window alpha is used to blend video window.

    If color key is enabled in the graphics feeder, the window alpha might
    be replaced by a "keyed alpha" for some pixels that pass the key test.
    Please refer to BVDC_Source_EnableColorKey for detail.

    If a window's SrcBlendFactor and DstBlendFactor are all constants
    (either in the case that one is BVDC_BlendFactor_eConstantAlpha and the
    other is BVDC_BlendFactor_eOneMinusConstantAlpha, or in the case that
    one is BVDC_BlendFactor_eOne and the other is BVDC_BlendFactor_eZero),
    the window alpha, and the per-pixel alpha of graphics, are totally
    ignored, hence have no contribution to the finally display.

    If this function is never called for a window, the default value of 255
    is used.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - A valid window handle created earlier.
    ucAlpha - Window alpha value.  A valid range for alpha
    value is from (0-255).  Alpha value of zero (0) mean a window is total
    transparent, and a value of 255 is total opaque.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the alpha value of a window.

See Also:
    BVDC_Window_GetAlpha,
    BVDC_Window_SetBlendFactor, BVDC_Source_EnableColorKey
**************************************************************************/
BERR_Code BVDC_Window_SetAlpha
    ( BVDC_Window_Handle               hWindow,
      uint8_t                          ucAlpha );

/***************************************************************************
Summary:
    This function gets the window alpha value of a window.

Description:
    This function gets the window alpha value of a window. This alpha
    value is used as blending with other windows.

Input:
    hWindow - A valid window handle created earlier.

Output:
    ucAlpha - A window alpha value.  A valid range for alpha
    value is from (0-255).  Alpha value of zero (0) mean a window is total
    transparent, and a value of 255 is total opaque.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the alpha value of a window.

See Also:
    BVDC_Window_SetAlpha, BVDC_Window_SetBlendFactor.
**************************************************************************/
BERR_Code BVDC_Window_GetAlpha
    ( const BVDC_Window_Handle         hWindow,
      uint8_t                         *pucAlpha );

/***************************************************************************
Summary:
    This function sets the blending factors and the constant alpha value for
    a window.

Description:
    This function sets the blending factors and the constant alpha value for
    a window. They are used for blending with other windows.

    There could be multiple visible windows for a given compositor. The
    visible windows are blended onto the destination picture in order, from
    smaller Z order value to bigger Z order value, to generate the final
    destination picture for display.

    The blending is processed in stages. Starting from the constant
    background color set by BVDC_Compositor_SetBackgroundColor, the
    destination picture is blended with one source window at each stage, to
    accumulate the contribution of the source window. The blending is done
    for each color component separately. The blending equation is

        DstComponent = (SrcComponent * SrcBlendFactor) +
                       (DstComponent * DstBlendFactor)

    The SrcBlendFactor and DstBlendFactor in the above equation are set by
    this function. The higher level software must set them correctly so
    that the final blended color is valid, i.e. in the valid range (such as
    in the range of 16 to 235 for Y). Otherwise, unexpected color might show
    up.

    Assume the destination color and the source color are in the valid range
    in a stage, its blended result will also be in the valid range if the
    blending equation is balanced, i.e. if

        DstBlendFactor + SrcBlendFactor = 1.0.

    For video window, it is recommended to set SrcBlendFactor as
    BVDC_BlendFactor_eSrcAlpha and DstBlendFactor as
    BVDC_BlendFactor_eOneMinusSrcAlpha. This is the default setting
    for video window. Other factor combinations are allowed, as long as
    the blending equation is balanced.

    The blending factor setting for a graphics window is similar, but extra
    attention is needed.

    If the pixel color in its source graphics surface has been pre-
    multiplied with the pixel alpha, that is quite likely if the surface was
    created by BGRC with scaling, then SrcBlendFactor, DstBlendFactor, and
    ConstantAlpha for the graphics window have to be set as
    BVDC_BlendFactor_eConstantAlpha, BVDC_BlendFactor_eOneMinusSrcAlpha,
    and 0xff respectively, in order to balance the blending equation. The
    situation is demonstrated in the following diagram.

                               Blend Stage
                              +------------+
             / D_C1 \         |    D_Ci    |
        Dest | D_C2 |      -->|     *      |
             \ D_C3 /         |(1.0 - S_A) |
                              |     +      |
             / S_C1 * S_A \   |(S_Ci * S_A)|   / D_C1*(1.0-S_A) + S_C1*S_A \
        Gfx  | S_C2 * S_A |-->|     *      |-->| D_C2*(1.0-S_A) + S_C2*S_A |
        Src  | S_C3 * S_A |   |    1.0     |   \ D_C3*(1.0-S_A) + S_C3*S_A /
             \ S_A        /   +------------+

    Otherwise, SrcBlendFactor and DstBlendFactor are recommended to be set
    to BVDC_BlendFactor_eSrcAlpha and BVDC_BlendFactor_eOneMinusSrcAlpha
    respectively. This is also the default setting for graphics windows.

                         Blend Stage
                        +------------+
             / D_C1 \   |    D_Ci    |
        Dest | D_C2 |-->|     *      |
             \ D_C3 /   |(1.0 - S_A) |
                        |     +      |
             / S_C1 \   |    S_Ci    |   / D_C1*(1.0-S_A) + S_C1*S_A \
        Gfx  | S_C2 |-->|     *      |-->| D_C2*(1.0-S_A) + S_C2*S_A |
        Src  | S_C3 |   |    S_A     |   \ D_C3*(1.0-S_A) + S_C3*S_A /
             \ S_A  /   +------------+

    In this case, it is also allowed to use constant blend factors, as long
    as the blending equation is balanced. If so, the graphics pixel alpha
    input to the blending stage is ignored. That means the pixel alpha in
    the graphics source surface, the graphics window alpha, and the graphics
    color keying operation, have no effect on the final display.

                          Blend Stage
                        +------------+
             / D_C1 \   |    D_Ci    |
        Dest | D_C2 |-->|     *      |
             \ D_C3 /   |(1.0 - Cnst)|
                        |     +      |
             / S_C1 \   |    S_Ci    |   / D_C1*(1.0-Cnst) + S_C1*Cnst \
        Gfx  | S_C2 |-->|     *      |-->| D_C2*(1.0-Cnst) + S_C2*Cnst |
        Src  | S_C3 |   |   Cnst     |   \ D_C3*(1.0-Cnst) + S_C3*Cnst /
             \ S_A  /   +------------+

    Please notice that the "source alpha" used during blending of a video
    window is the constant window alpha set by BVDC_Window_SetAlpha. And
    the "source alpha" of a graphics window used during blending is the
    (per-pixel) product of the pixel alpha with either window alpha set
    by BVDC_Window_SetAlpha, or the "keyed alpha" set by
    BVDC_Source_EnableColorKey, depending on whether color key is enabled
    and the pixel passes the color key test.

    ucConstantAlpha is initialized as 255 by default for all windows.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - A valid window handle created earlier.
    eSrcBlendFactor - Source blending factor.
    eDstBlendFactor - Destination blending factor.
    ucConstantAlpha - Constant alpha used when a blending factor is
    BVDC_BlendFactor_eConstantAlpha or BVDC_BlendFactor_eOneMinusConstantAlpha


Example:
    The blending happening in stages depending on the number of active
    windows.  For example if we have two windows video & gfx and gfx
    is has higher z-order the blending process would be like following

                     +----------+
      BG-Color --D-->| blender  |
                     |    0     |      +----------+
         video --S-->|          |--D-->| blender  |
                     +----------+      |    1     |       +----------+
                             gfx --S-->|          |---D-->| blender  |
                                       +----------+       |    2     |
                                       (video/gfx)  --S-->|          |--> result
                                                          +----------+

    The BVDC_Window_SetZOrder select the blender the window going to
    use.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the alpha value of a window.

See Also:
    BVDC_Window_GetBlendFactor,
    BVDC_Window_SetAlpha, BVDC_Source_EnableColorKey,
    BVDC_Window_SetZOrder, BVDC_Window_SetVisibility.
**************************************************************************/
BERR_Code BVDC_Window_SetBlendFactor
    ( BVDC_Window_Handle               hWindow,
      BVDC_BlendFactor                 eSrcBlendFactor,
      BVDC_BlendFactor                 eDstBlendFactor,
      uint8_t                          ucConstantAlpha );

/***************************************************************************
Summary:
    This function gets the current blending factors and constant alpha of
    a window.

Description:
    The current blending factors and constant alpha value of the window are
    used for blending it onto the destination picture.

Input:
    hWindow - A valid window handle created earlier.

Output:
    peSrcBlendFactor - Source blending factor.
    peDstBlendFactor - Destination blending factor.
    pucConstantAlpha - Constant alpha to be use for constant alpha
    blending factor.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the blending factors and constant alpha.

See Also:
    BVDC_Window_SetBlendFactor, BVDC_Window_GetZOrder.
**************************************************************************/
BERR_Code BVDC_Window_GetBlendFactor
    ( const BVDC_Window_Handle         hWindow,
      uint8_t                         *pucConstantAlpha,
      BVDC_BlendFactor                *peSrcBlendFactor,
      BVDC_BlendFactor                *peDstBlendFactor );

/***************************************************************************
Summary:
    This function sets the z-order value of a window.

Description:
    This function specifies the z-order value of a window. The window Z
    order values of a compositor determine the order in that the visiable
    windows are blended onto the destination picture to accumulate their
    contribution.

    A window with z-order value of zero (0) would be blended first, and a
    window with z-order value of BVDC_Z_ORDER_MAX is blended at last. And
    each window must have different z-order value.  User must call this
    function to set the z order.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - A valid window handle created earlier.
    ucZOrder - A window z-order value.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the z-order value of a window.

See Also:
    BVDC_Window_GetZOrder, BVDC_Window_SetBlendFactor.
**************************************************************************/
BERR_Code BVDC_Window_SetZOrder
    ( BVDC_Window_Handle               hWindow,
      uint8_t                          ucZOrder );

/***************************************************************************
Summary:
    This function gets the z-order value of a window.

Description:
    This function gets the z-order value of a window.

Input:
    hWindow - A valid window handle created earlier.

Output:
    pucZOrder - A reference to store a window z-order value.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the z-order value of a window.

See Also:
    BVDC_Window_SetZOrder.
**************************************************************************/
BERR_Code BVDC_Window_GetZOrder
    ( const BVDC_Window_Handle         hWindow,
      uint8_t                         *pucZOrder );

/***************************************************************************
Summary:
    This function shows/hides the window.

Description:
    This function specifies if a window is to be shown or hidden.  When it's
    hidden all the video / graphics processing still going on, but it just
    not get shown that all.  For example it is possible that decoding process
    is still going, but video is not show.  One can still capture video for
    later use.

    If this function is never called for a window, it is shown by default.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - A valid window handle created earlier.
    bVisible - A flag to control visibility (show/hide).

Output:
Each window must have different z-order value.
Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the flag to shows/hides the window.

See Also:
    BVDC_Window_GetVisibility.
**************************************************************************/
BERR_Code BVDC_Window_SetVisibility
    ( BVDC_Window_Handle               hWindow,
      bool                             bVisible );

/***************************************************************************
Summary:
    This function gets that status of window visibility.

Description:
    This function get the status if a window is shown or hidden.  When it's
    hidden all the video / graphics processing still going on, but it just
    not get shown that all.  For example it is possible that decoding process
    is still going, but video is not show.  One can still capture video for
    later use.

Input:
    hWindow - A valid window handle created earlier.

Output:
    pbVisible - A reference to store a flag to control visibility.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the status of window visibility.

See Also:
    BVDC_Window_SetVisibility.
**************************************************************************/
BERR_Code BVDC_Window_GetVisibility
    ( const BVDC_Window_Handle         hWindow,
      bool                            *pbVisible );

/***************************************************************************
Summary:
    This function configures deinterlace modes

Description:
    Enables or disables the deinterlace feature. De-interlace is supported
    on the Main display, any aspect ratio for Full-screen images only. It is
    not supported on PIP, PIG or secondary displays. Converts 480i source to
    480p, 720p or 1080i outputs, and applies to up-scaling only.

    Deinterlace is disabled by default.

    If Null config pointer is passed, use default settings, i.e. enable both
    3:2 and 2:2 reverse pulldown;

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - A valid window handle created earlier.
    bDeinterlace - enables/disables deinterlace (true/false)
    pMadSettings - structure to configure Mad.
Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the flag to shows/hides the window.

See Also:
    BVDC_Deinterlace_Settings
    BVDC_Window_GetDeinterlaceConfiguration
**************************************************************************/
BERR_Code BVDC_Window_SetDeinterlaceConfiguration
    ( BVDC_Window_Handle               hWindow,
      bool                             bDeinterlace,
      const BVDC_Deinterlace_Settings *pMadSettings );

/***************************************************************************
Summary:
    This function gets the deinterlace setting

Description:

Input:
    hWindow - A valid window handle created earlier.
Output:
    pbDeinterlace - Deinterlace setting applied
    pMadSettings  - structure to configure Mad.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the flag to shows/hides the window.

See Also:
    BVDC_Deinterlace_Settings
    BVDC_Window_GetDeinterlaceConfiguration
**************************************************************************/
BERR_Code BVDC_Window_GetDeinterlaceConfiguration
    ( const BVDC_Window_Handle         hWindow,
      bool                            *pbDeinterlace,
      BVDC_Deinterlace_Settings       *pMadSettings );

/***************************************************************************
Summary:
    This function gets the default deinterlace setting

Description:

Input:
    hWindow - A valid window handle created earlier.
Output:
    pMadSettings  - structure to configure Mad.
    Note: For pMadSettings' internal pointer type fields: "pReverse32Settings",
          "pReverse22Settings", "pChromaSettings", "pMotionSettings",
          "pUpSampler", "pDnSampler", and "pLowAngles", caller must
          either guarantee that space is allocated for the pointer(s) if
          it wants to retrieve settings of that field(s) or explicitly
          set the pointer(s) of that field(s) to NULL.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Success.
See Also:
    BVDC_Deinterlace_Settings
    BVDC_Window_GetDeinterlaceConfiguration

**************************************************************************/
BERR_Code BVDC_Window_GetDeinterlaceDefaultConfiguration
    ( const BVDC_Window_Handle         hWindow,
      BVDC_Deinterlace_Settings       *pMadSettings );

/***************************************************************************
Summary:
    Speficify AFD clipping mode and settings.

Description:
    This function enable the enabling/disabling the use of AFD clipping mode.
    When enabled the clipping from Letter Box detection and PanScan
    will be ignored.

Input:
    hWindow - A valid window handle created earlier.
    pAfdSettings - AFD settings to set

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set AFD settings for the window.

See Also:
    BVDC_Window_GetAfdSettings.
***************************************************************************/
BERR_Code BVDC_Window_SetAfdSettings
    ( BVDC_Window_Handle               hWindow,
      const BVDC_AfdSettings          *pAfdSettings );

/***************************************************************************
Summary:
    This function gets AFD setting applied to the window

Description:
    Return the  AFD setting applied to the window.

Input:
    hWindow - A valid window handle created earlier.

Output:
    pAfdSettings - AFD setting applied to the window.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_SetAfdSettings
**************************************************************************/
BERR_Code BVDC_Window_GetAfdSettings
    ( const BVDC_Window_Handle         hWindow,
      BVDC_AfdSettings                *pAfdSettings );

/***************************************************************************
Summary:
    This function sets pan scan type for the window

Description:
    Sets pan scan type to control how the size and position of the scan out
    rectangle of the feeder is calculated.

    When pan scan is set to BVDC_PanScanType_eDisable, pan scan will be
    disabled and the source rectangle will be unmodified. For MPEG sources,
    this means the source rectangle will be defined by the
    ulSourceHorizontalSize and ulSourceVerticalSize parameters from the
    BAVC_MVD_Field structure.

    When pan scan is not set to BVDC_PanScanType_eDisable, the source rectangle
    will be determined by the ulDisplayHorizontalSize and
    ulDisplayHorizontalSize parameters from the BAVC_MVD_Field structure. This
    new rectangle, referred to as the pan scan rectangle, will be initally
    centered within the original source rectangle and may be adjusted by
    additional pan scan vectors defined by BVDC_PanScanType.

    Whether pan scan is enabled or not, BVDC_Window_SetSrcClip() will continue
    to operate on the resulting rectangle. So when pan-scan is enabled, the
    source clipping will occur on top of the pan scan rectangle and not the
    original source rectangle.

    When pan scan is set to BVDC_PanScanType_eStream, the stream pan scan
    vector specified by i16_HorizontalPanScan and i16_VerticalPanScan in the
    BAVC_MVD_Field structure will be used to adjust the position of the source
    pan scan rectangle.

    When pan scan is set to BVDC_PanScanType_eUser, the user pan scan vector
    will be used to adjust the source pan scan rectangle.

    When pan scan is set to BVDC_PanScanType_eStreamAndUser, both stream and
    user pan scan vectors will be used to adjust the source pan scan rectangle.

    In all cases, the pan scan rectangle cannot be adjusted outside of the
    original source.

    When pan scan is not set to BVDC_PanScanType_eDisable, automatic box
    black-cut is not performed regardless of the setting by
    BVDC_Window_EnableBoxDetect.

    BVDC_PanScanType_eDisable is initially set by default. Pan scan can only
    be enabled for MPEG sources.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - A valid window handle created earlier.
    ePanScanType - Pan scan type to set.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_GetPanScanType,
    BVDC_Window_SetUserPanScan,
    BVDC_Window_SetSrcClip, BVDC_Window_SetScalerOutput, BVDC_Window_SetDstRect,
    BVDC_Window_EnableBoxDetect,
    BVDC_Window_SetAspectRatioMode, BVDC_Window_SetScaleFactorRounding
**************************************************************************/
BERR_Code BVDC_Window_SetPanScanType
    ( BVDC_Window_Handle               hWindow,
      BVDC_PanScanType                 ePanScanType );

/***************************************************************************
Summary:
    This function gets pan scan type applied to the window

Description:
    Return the pan scan type applied to the window.

Input:
    hWindow - A valid window handle created earlier.

Output:
    pePanScanType - Pan scan type applied to the window.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_SetPanScanType
**************************************************************************/
BERR_Code BVDC_Window_GetPanScanType
    ( const BVDC_Window_Handle         hWindow,
      BVDC_PanScanType                *pePanScanType );

/***************************************************************************
Summary:
    This function sets user pan scan vector for the window

Description:
    Refer to BVDC_Window_SetPanScanType. When pan scan is set to
    BVDC_PanScanType_eUser, the user pan scan vector will be used to adjust
    the source pan scan rectangle. And when pan scan is set to
    BVDC_PanScanType_eStreamAndUser, both stream and user pan scan vectors
    will be used to adjust the source pan scan rectangle.

    When the source is type of Mpeg / HdDvi and the pan scan rectangle is
    provided by the stream (i.e. the display horizontal size and display
    vertical size provided from the video stream are not 0, and one of them
    is smaller than the original source size), it is used during the pan
    scan process. In all other cases (particularly, in the 656 video
    source cases), the pan scan rectangle size will be calculated by BVDC.
    Either source width or height will be cut so that the pan scan rectangle
    matches the display aspect ratio.

    Notice that setting window's aspect ratio mode to "zoom" performs
    similar functionality. However, it can only zoom in the center of the
    source. User pan scan allows to shift the zoom area. Another difference
    is that aspect ratio correction is performed after the application of
    the user set rectangles, but pan scan is performed before that.

    As an example of application, consider the case that a source video
    signal, with aspect ratio of 4:3, carries a Cinema movie content with
    letter box patched, and is displayed on a wide screen TV. Using user pan
    scan, then further user clipping either the width or height, together
    with setting aspect ratio mode to "zoom", the application program could
    conveniently support user to quickly adjust the zoom area to active
    content area manually.

    This function sets user pan scan vector for the window.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - A valid window handle created earlier.
    sHorzPanScan - Horizontal pan scan, in unit of pixel.
    sVertPanScan - Vertical pan scan, in unit of pixel.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_SetPanScanType, BVDC_Window_SetAspectRatioMode,
    BVDC_Window_GetUserPanScan
**************************************************************************/
BERR_Code BVDC_Window_SetUserPanScan
    ( BVDC_Window_Handle               hWindow,
      int32_t                          lHorzPanScan,
      int32_t                          lVertPanScan );

/***************************************************************************
Summary:
    This function sets user pan scan vector applied to the window

Description:
    Return the user pan scan vercor applied to the window.

Input:
    hWindow - A valid window handle created earlier.

Output:
    psHorzPanScan - Horizontal pan scan applied to the window, in unit of pixel.
    psVertPanScan - Vertical pan scan applied to the window, in unit of pixel.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_SetUserPanScan
**************************************************************************/
BERR_Code BVDC_Window_GetUserPanScan
    ( const BVDC_Window_Handle         hWindow,
      int32_t                         *plHorzPanScan,
      int32_t                         *plVertPanScan );

/***************************************************************************
Summary:
    This function sets the aspect ratio mode for this window.

Description:
    Aspect ratio of a rectangle area is the ratio of its horizontal measure
    to its vertical measure. It might NOT be the same as the ratio of the
    pixel numbers, indeed it is the ratio of the pixel numbers multiplied
    with the aspect ratio of a pixel area.

    When the clipped source rectangle area is scaled to a rectangle area
    with a different aspect ratio, the display will not look correct, it
    will shrink in one direction and expand in another direction.

    This function sets the aspect ratio correction mode for this window, it
    specifies the method how the source clip rectangle, scaler-out rectangle,
    and destination rectangle are set to BVDC, and whether and how to adjust
    them to maintain the aspect-ratio match. Refer to BVDC_AspectRatioMode
    for the detail behavior of each aspect ratio mode.

    Aspect mode correction is performed on top of the user set rectangles,
    and before scale factor rounding.

    When source pixel aspect ratio is computed, the the full source aspect
    ratio is typically considered to be contributed by the full source canvas.
    But it is possible to specify a sub-rectangle of the source canvas as the
    area that make up the the full source aspect ratio. This is also true for
    display pixel aspect ratio. Refer to BVDC_Source_SetAspectRatioCanvasClip
    and BVDC_Display_SetAspectRatioCanvasClip for more detail.

Input:
    hWindow - A valid window handle created earlier.
    eAspectRatioMode - A desired aspect ratio correction mode.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_GetAspectRatioMode,
    BVDC_Source_OverrideAspectRatio, BVDC_Source_SetAspectRatioCanvasClip,
    BVDC_Display_SetAspectRatio, BVDC_Display_SetAspectRatioCanvasClip,
    BVDC_Window_SetSrcClip,
    BVDC_Window_SetScalerOutput
    BVDC_Window_SetDstRect
    BVDC_Window_EnableBoxDetect, BVDC_Window_SetPanScanType,
    BVDC_Window_SetScaleFactorRounding.
**************************************************************************/
BERR_Code BVDC_Window_SetAspectRatioMode
    ( BVDC_Window_Handle               hWindow,
      BVDC_AspectRatioMode             eAspectRatioMode );

/***************************************************************************
Summary:
    This function gets the aspect ratio correct mode for this window.

Description:
    Get the mode that perform aspect ratio correction such as 4x3 to 16x9,
    and 16x9 to 4x3.

Input:
    hWindow - A valid window handle created earlier.

Output:
    peAspectRatioMode - A currect aspect ratio correction mode.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_SetAspectRatioMode
**************************************************************************/
BERR_Code BVDC_Window_GetAspectRatioMode
    ( const BVDC_Window_Handle         hWindow,
      BVDC_AspectRatioMode            *peAspectRatioMode );

/***************************************************************************
Summary:
    This function enable box detect for this window.

Description:
    After box detect is enabled for a video window, its video source stream
    is examined to detect potential letter box (such as conveying video
    content with aspect ratio 16:9 on a video stream signal with aspect
    ratio 4:3) and pillar box (such as 4:3 content on 16:9 signal).

    Right after box detect is enabled for a window, and when the detected
    data changes, BVDC will call the user provided call back function
    BoxDetectCallBack if it is not NULL. The calling is at interrupt handler
    context. As a paramter, a pointer to a BVDC_BoxDetectInfo struct is
    passed to user. Based on the infomation stored on this struct, it might
    be able to decide whether there is letter box or pillar box in the source
    video. Refer to the description of BVDC_Window_BoxDetectCallback_isr and
    BVDC_BoxDetectInfo for more detail.

    If bAutoCutBlack is true and if pan scan is not enabled, BVDC will
    automatically crop the black patch region from the source video before
    source clip rectangle is applied. In an other word, if source clip
    rectangle is set, it will further clip from the white box, not the
    original video source rectangle.

    When pan scan is enabled, bAutoCutBlack is ignored and treated as false.

    In the case that user want to clip the black regions by setting source
    clip rectangle, it could be done either in the call back function
    BoxDetectCallBack with or later with the info
    saved in BoxDetectCallBack.

    Enabling box detect to a graphics window generates an error.

    There are limited number of box detect hardware modules in the chip (2 in
    7038 B0). When all of them are already used, an error code
    BERR_BOX_DETECT_HW_BUSY is returned by future BVDC_Window_EnableBoxDetect.
    Box detect should be disabled when it is not needed, to free the hardware
    resource. It is valid to call BVDC_Window_EnableBoxDetect again when box
    detect is already enabled for the window. In this case the same hardware
    module is used and the state is reset.

    Due to hardware limitation of 7038 B0, box detect should not be enabled
    with a 7038 B0 chip when vbi pass-through is used.

    This setting will not be applied until a call to BVDC_ApplyChanges is
    made.

Input:
    hWindow - A valid window handle created earlier.
    BoxDetectCallBack - Call back function when BVDC detect new box or box
    boundary changes.
    pvParm1 - Pointer to a user defined struct that user wish to pass to this
    call back function. It might include info about the window this box detect
    belogs to.
    iParm2 - An int number that user wish to pass to this call back function.
    bAutoCutBlack - Whether automatically cut the detected black patch regions.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_BOX_DETECT_HW_BUSY - All box detect HW modules already used.
    BERR_SUCCESS - Successfully enabled box detect for the window.

See Also:
    BVDC_Window_DisableBoxDetect, BVDC_Window_SetSrcClip,
    BVDC_Window_SetScalerOutput, BVDC_Window_SetDstRect,
    BVDC_Window_SetPanScanType, BVDC_Window_SetAspectRatioMode,
    BVDC_Window_SetScaleFactorRounding,
**************************************************************************/
BERR_Code BVDC_Window_EnableBoxDetect
    ( BVDC_Window_Handle                hWindow,
      BVDC_Window_BoxDetectCallback_isr pfBoxDetectCallBack,
      void                             *pvParm1,
      int                               iParm2,
      bool                              bAutoCutBlack );

/***************************************************************************
Summary:
    This function disable box detect for this window.

Description:
    After box detect is disabled, the hardware module is freed for future
    usage.

    This setting will not be applied until a call to BVDC_ApplyChanges is
    made.

Input:
    hWindow - A valid window handle created earlier.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully enabled box detect for the window.

See Also:
    BVDC_Window_EnableBoxDetect,
**************************************************************************/
BERR_Code BVDC_Window_DisableBoxDetect
    ( BVDC_Window_Handle               hWindow );

/***************************************************************************
Summary:
    This function sets the nonlinear horizontal scaling for video window.

Description:
    When a video source is displayed on a monitor with wider aspect ratio,
    such as a video source of 4:3 displayed on a 16:9 monitor, user could
    choose to linearly scale (in both horizontal and vertical direction) the
    video source onto a pillar box in the center of the screen (with source
    aspect ratio correctly maintained every where), or to non-linearly scale
    the horizontal direction of the video to cover the whole screen. When
    the horizontal direction is non-linearly scaled, a central source region
    is linearly scaled (likely with source aspect ratio maintained), and
    the equally sized left and right edge regions are more stretched
    horizontally to cover the whole monitor.

    The parameter ulNonLinearSrcWidth and ulNonLinearSclOutWidth specify the
    edge region width (of one side). If one of them is 0 and another is not 0,
    the 0 is replaced automatically by BVDC based on the non-zero one and
    aspect ratio maintaining in the central region. Non-linear feature is
    turned off when both of them are set to 0, and this is the default. It
    is also disabled if AspectRatioMode of the window is set to
    BVDC_AspectRatioMode_eUseAllSource / eUseAllDestination.

    Due to the hardware small ulNonLinearSrcWidth and ulNonLinearSclOutWidth
    can cause some calculation underflow. It is suggested to set both of them
    not less than one quarter of the total width.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - A valid window handle created earlier.
    ulNonLinearSrcWidth - Source width of one nonlinearly scaled edge region.
    ulNonLinearSclOutWidth - Scale output width of one nonlinearly scaled
    source edge region.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_GetNonLinearScl,
    BVDC_Window_SetAspectRatioMode, BVDC_Window_SetScaleFactorRounding,
    BVDC_Display_SetAspectRatio
**************************************************************************/
BERR_Code BVDC_Window_SetNonLinearScl(
      BVDC_Window_Handle               hWindow,
      uint32_t                         ulNonLinearSrcWidth,
      uint32_t                         ulNonLinearSclOutWidth);

/***************************************************************************
Summary:
    This function gets the nonlinear horizontal scale setting for video window.

Description:
    This function gets the nonlinear horizontal scale setting for video window.

    Refer to BVDC_Window_SetNonLinearScl for the meaning of the output
    parameters.

Input:
    hWindow - A valid window handle created earlier.

Output:
    pulNonLinearSrcWidth - Currently applied source width of one nonlinearly
    scaled edge region.
    pulNonLinearSclOutWidth - Currently applied Scale output width of one
    nonlinearly scaled source edge region.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_SetNonLinearScl
**************************************************************************/
BERR_Code BVDC_Window_GetNonLinearScl
    ( const BVDC_Window_Handle         hWindow,
      uint32_t                        *pulNonLinearSrcWidth,
      uint32_t                        *pulNonLinearSclOutWidth);

/***************************************************************************
Summary:
    This function sets the scale factor rounding tolerance.

Description:
    When the clipped source is scaled to a size specified by
    BVDC_Window_SetScalerOutput, the picture quality is typically better if
    it is scaled up by an integer factor such as 1, 2, .... Therefore we
    might want to round scale factor to the closest integer value when they
    are close enough. Notice that a down scaling factor might be rounded to
    1 when it is close to.

    One good example is when a NTSC video without over scan is displayed on
    a 720x480 window. The non-over-scanned video has a size of 704x480. The
    video picture will be more clear if the 704x480 is displayed in the
    center of the 720x480 window without involving scaling.

    This function sets the scale factor rounding tolerance ulHrzTolerance
    and ulVrtTolerance for horizontal and vertical direction respectively.
    The tolerance defines how close to the closest integer when the scale
    factor should be rounded to the integer.

    The closeness is measured by the percentage, of the absolute difference
    between the scale factor and the closest integer. Notice that we will
    never round to integer 0, since the percentage of the absolute difference
    to 0 is always infinite.

    The default value for both ulHrzTolerance and ulVrtTolerance are 0. That
    means scale factor rounding is disabled by default. A value of 50 makes
    up-scale-factors always be rounded to the nearest integer.

    With a value of 3 for ulHrzTolerance, the horizontal scale factor in the
    above example will be rounded to 1, and the 704x480 will be displayed in
    the center of the 720x480 window without scaling. More examples include
    that source size of 352x240, 88x60, 44x30, ... will be scaled up by integer
    factors and be displayed in the center of 720x480 window, and that a
    704x480 window will display a source size of 720x480 by cutting 8 pixels
    from both sides and without scaling.

    Scale factor rounding is performed on top of letter-box clipping, pan-scan,
    the user set rectangles and aspect ratio correction performed in in VDC.
    The rounding is done separately for horizontal and vertical direction.

    If scale factor is rounded down in one direction, VDC would adjust to cut
    less from source and/or to further cut in output rectangles. Please
    notice that user set source clipping is always honored.

    In the case of rounding up, VDC would adjust to cut less from output
    rectangles and/or to further cut source.

    Notice that scale factor rounding will affect the accuracy of the aspect
    ratio, therefore the tolerance should NOT be set to be too big, although
    it is allowed.

    This setting will not be applied until a call to BVDC_ApplyChanges is
    made.

Input:
    hWindow - A valid window handle created earlier.
    ulHrzTolerance - horizontal scale factor rounding tolerance.  The unit
    is in percentage.

    ulVrtTolerance - vertical scale factor rounding tolerance.  The unit
    is in percentage.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_GetScaleFactorRounding,
    BVDC_Window_SetSrcClip,
    BVDC_Window_SetScalerOutput,
    BVDC_Window_SetDstRect,
    BVDC_Window_EnableBoxDetect, BVDC_Window_SetPanScanType,
    BVDC_Window_SetAspectRatioMode, BVDC_Window_SetNonLinearScl
**************************************************************************/
BERR_Code BVDC_Window_SetScaleFactorRounding
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulHrzTolerance,
      uint32_t                         ulVrtTolerance  );

/***************************************************************************
Summary:
    This function gets the scale factor rounding tolerance.

Description:
    Scale factor rounding tolerance specifies how close to an integer or the
    inverse of an integer when the scale factor is, it should be rounded to
    the integer or the inverse of the integer. One tolerance value for
    horizontal scale factor, and one value for vertical scale factor, are set
    by BVDC_Window_SetScaleFactorRounding. Refer to its description for detail.

Input:
    hWindow - A valid window handle created earlier.

Output:
    pulHrzTolerance - horizontal scale factor rounding tolerance.
    pulVrtTolerance - vertical scale factor rounding tolerance.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_SetScaleFactorRounding
**************************************************************************/
BERR_Code BVDC_Window_GetScaleFactorRounding
    ( BVDC_Window_Handle               hWindow,
      uint32_t                        *pulHrzTolerance,
      uint32_t                        *pulVrtTolerance  );


/***************************************************************************
Summary:
    This function makes the window's display track its source frame rate.

Description:
    When bUseSrcFrameRate, this window's source will be used as the master
    framerate for the display.  It will match the display framerate with
    the source.  By default it is disabled.  The default display framerate
    will be from the default BVDC_Settings.  In the case of compositor/display
    has two or more video windows, only one window should be set as master.
    The display framerate selection or tracking is based on source only
    when given display format capable of outputing multiple framerates.
    For example 720p is capable of 59.94 Hz or 60.00Hz, so 720p display
    can track 60 or 59.95 source; but NTSC is always 29.97 f/s, or vertical
    refresh rate is 59.94 Hz, so NTSC display cannot track 60Hz source.

    When window framerate tracking is toggled from enabled to disabled, the
    display framerate will remain the same as what current framerate is. User
    needs to install a display rate change callback to know what the current
    display frame rate is.

    The frame rate tracking will be ignored if user calls
    BVDC_Display_SetDropFrame(true).

Input:
    hWindow - A valid window handle created earlier.
    bUseSrcFrameRate - Enable window's source to be display framerate.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_GetMasterFrameRate,
    BVDC_Display_CallbackData, BVDC_Display_InstallCallback
**************************************************************************/
BERR_Code BVDC_Window_SetMasterFrameRate
    ( BVDC_Window_Handle               hWindow,
      bool                             bUseSrcFrameRate );

/***************************************************************************
Summary:
    This function get the window's framerate tracking setting.

Description:

Input:
    hWindow - A valid window handle created earlier.

Output:
    pbUseSrcFrameRate - framerate tracking or not.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_SetMasterFrameRate
**************************************************************************/
BERR_Code BVDC_Window_GetMasterFrameRate
    ( const BVDC_Window_Handle         hWindow,
      bool                            *pbUseSrcFrameRate );

/***************************************************************************
Summary:
    This function enables or disables force capture.

Description:
    Enables or disables force capture. The selected video window is always
    captured when force capture is enabled. Disable force capture doesn't
    mean window is not captured. When force capture is disabled, VDC determines
    internally whether the window needs to be captured or not, depends on
    display configuration.

    Force capture is disabled by default.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - A valid window handle created earlier.
    bForceCapture - enables/disables force capture (true/false)

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set force capture.

See Also:
    BVDC_Window_GetForceCapture
**************************************************************************/
BERR_Code BVDC_Window_SetForceCapture
    ( BVDC_Window_Handle               hWindow,
      bool                             bForceCapture );

/***************************************************************************
Summary:
    This function gets the force capture setting.

Description:
    Returns the force capture state applied to the window handle specified.

    Force capture on means the window is always captured. Force capture
    off means the window may or may not be captured, depends on display
    configuration.

Input:
    hWindow - A valid window handle created earlier.

Output:
    pbForceCapture - Force capture setting applied

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get force capture flag.

See Also:
    BVDC_Window_SetForceCapture
**************************************************************************/
BERR_Code BVDC_Window_GetForceCapture
    ( const BVDC_Window_Handle          hWindow,
      bool                             *pbForceCapture );


/***************************************************************************
Summary:
    Get the number of vsync delay thru BVN network.

Description:
    Returns the maximum number of vysnc delay for a given window. The returned
    delay from the callback will never exceed this value.  The delay is due
    to additional video processing in BVN.

Input:
    hWindow - A valid window handle created earlier.

Output:
    puiMaxVsyncDelay - The maximum number of vsync delay for this particular
    window.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
See Also:
    BVDC_Window_GetDelayOffset
    BVDC_Window_SetDelayOffset
**************************************************************************/
BERR_Code BVDC_Window_GetMaxDelay
    ( const BVDC_Window_Handle         hWindow,
      unsigned int                    *puiMaxVsyncDelay );

/***************************************************************************
Summary:
    Set the vsync delay offset thru BVN network.

Description:
    Set the vysnc delay offset for the given window. This additioanl delay
    is added to capture and playback thru BVN network. The application needs
    to make sure there is enough memory available for capture. If capture was
    enabled for the given window path, uiVsyncDelayOffset new buffers are
    needed. If capture was not enabled for the given window path,
    uiVsyncDelayOffset + 1 new buffers are needed.

    The offset needs to be reset to proper value if new delay is detected by
    VDC, and the callback functions is called by VDC.

Input:
    hWindow - A valid window handle created earlier.

Output:
    uiVsyncDelayOffset - The vsync delay offset for this particular window.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
See Also:
    BVDC_Window_InstallCallback.
    BVDC_Window_GetMaxDelay
    BVDC_Window_GetDelayOffset
**************************************************************************/
BERR_Code BVDC_Window_SetDelayOffset
    ( BVDC_Window_Handle               hWindow,
      unsigned int                     uiVsyncDelayOffset );

/***************************************************************************
Summary:
    Get the vsync delay offset thru BVN network.

Description:
    Get the vysnc delay offset for the given window.

Input:
    hWindow - A valid window handle created earlier.

Output:
    puiVsyncDelayOffset - The vsync delay offset for this particular
    window.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
See Also:
    BVDC_Window_InstallCallback.
    BVDC_Window_GetMaxDelay
    BVDC_Window_SetDelayOffset
**************************************************************************/
BERR_Code BVDC_Window_GetDelayOffset
    ( const BVDC_Window_Handle         hWindow,
      unsigned int                    *puiVsyncDelayOffset );

/***************************************************************************
Summary:
    Install or uninstall a callback with VDC for window related update.

Description:
    This function install a callback with VDC to be called at every vsync to
    read VNET_V_CRC, or whenever VDC changes its BVN delay such as vsync delay,
    BVN drift delay or other info due to additional video processing in BVN
    modules; deinterlacer, 3dComb, capture & playback. Since Audio module need
    to know BVN delay for lip sync, this callback serve as event update for
    Audio.

    The pvVdcData pointer passes back by this callback is defined in
    BVDC_Window_CallbackData, which contains all necessary information for Audio
    to config itself. The ulVsyncDelay and ulDriftDelay in BVDC_Window_CallbackData
    do not include delay offset set by BVDC_Window_SetDelayOffset.

    Uninstalls a callback for a given window if pfCallback is NULL.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - Window handle created earlier with BVDC_Window_Create.
    pfCallback - A function pointer to a callback that will update the
                 lip sync for Audio.
    pvParm1 - User defined structure.casted to void.
    iParm2 - User defined value.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Window_CallbackMask
    BVDC_Window_CallbackData
    BVDC_Window_CallbackSettings
    BVDC_Window_SetCallbackSettings
    BVDC_Window_GetCallbackSettings
**************************************************************************/
BERR_Code BVDC_Window_InstallCallback
    ( BVDC_Window_Handle               hWindow,
      const BVDC_CallbackFunc_isr      pfCallback,
      void                            *pvParm1,
      int                              iParm2 );

/***************************************************************************
Summary:
    Set the default settings of the window callback.

Description:
    This function sets the default setings for the window callback function.
    User can specify the lip sync tolerance in microseconds, or the mask for
    the vsync delay change, drift delay change, or other changes defined in
    BVDC_Window_CallbackSettings.

    This functions needs to be called before BVDC_Window_InstallCallback.

Input:
    hWindow - Window handle created earlier with BVDC_Window_Create.
    pSettings - Points to the callback function settings defined by user.

See Also:
    BVDC_Window_CallbackMask
    BVDC_Window_CallbackData
    BVDC_Window_CallbackSettings
    BVDC_Window_InstallCallback
    BVDC_Window_GetCallbackSettings
***************************************************************************/
BERR_Code BVDC_Window_SetCallbackSettings
        ( BVDC_Window_Handle                  hWindow,
          const BVDC_Window_CallbackSettings *pSettings );

/***************************************************************************
Summary:
    Get the current settings of the window callback.

Description:
    This function gets the current setings for the window callback function.

Input:
    hWindow - Window handle created earlier with BVDC_Window_Create.
    pSettings - Points to the current callback function settings.

See Also:
    BVDC_Window_CallbackMask
    BVDC_Window_CallbackData
    BVDC_Window_CallbackSettings
    BVDC_Window_InstallCallback
    BVDC_Window_GetCallbackSettings
***************************************************************************/
BERR_Code BVDC_Window_GetCallbackSettings
        ( BVDC_Window_Handle                  hWindow,
          BVDC_Window_CallbackSettings       *pSettings );


/***************************************************************************
Summary:
    This function sets the window's color space conversion process with
    RGB matching or not.

Description:
    A correct linear color space conversion process in the digital domain
    takes into consideration that the RGB space for HD and SD are different.
    In other words, the color space conversion does the RGB matching, i.e.
    HD (YCbCr) -> HD (RGB) -> SD (RGB) -> SD (YCbCr), or vice versa.
    However, many streams at practice might not be encoded
    with it in mind, i.e. HD (RGB) assumed not different from SD (RGB). In
    this case, the color space conversion doesn't need to do RGB matching,
    i.e., HD (YCbCr) -> RGB -> SD (YCbCr).
    So the application might want to toggle the color space conversion process
    with RGB matching or without to display various streams correctly.

    By default, a video window's color space conversion process DOES NOT do
    RGB matching.

Input:
    hWindow        - A valid window handle created earlier.
    bRgbMatching   - Enable window color space conversion process' RGB matching.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_GetRgbMatching
**************************************************************************/
BERR_Code BVDC_Window_SetRgbMatching
    ( BVDC_Window_Handle               hWindow,
      bool                             bRgbMatching );

/***************************************************************************
Summary:
    This function gets the window's color space conversion process attribute.

Description:
    Get the status of this window's RGB matching process.

Input:
    hWindow         - A valid window handle created earlier.

Output:
    pbRgbMatching - A currect RGB matching status.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_SetRgbMatching
**************************************************************************/
BERR_Code BVDC_Window_GetRgbMatching
    ( const BVDC_Window_Handle         hWindow,
      bool                            *pbRgbMatching );

/***************************************************************************
Summary:
    This function sets a window's contrast level.

Description:
    Sets the contrast level of a window by adjusting color space conversion
    matrices.

Input:
    hWindow        - A valid window handle created earlier.
    sContrast      - Window's contrast level, from -32768 to 32767.
                     Default value is 0.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set contrast level for the window.

See Also:
    BVDC_Window_GetContrast
**************************************************************************/
BERR_Code BVDC_Window_SetContrast
    ( BVDC_Window_Handle               hWindow,
      int16_t                          sContrast );

/***************************************************************************
Summary:
    This function gets a window's contrast level.

Description:
    Gets the contrast level of a window.

Input:
    hWindow        - A valid window handle created earlier.

Output:
    psContrast     - Window's current contrast level, from -32768 to 32767.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned contrast value for the window.

See Also:
    BVDC_Window_SetContrast
**************************************************************************/
BERR_Code BVDC_Window_GetContrast
    ( BVDC_Window_Handle               hWindow,
      int16_t                         *psContrast );

/***************************************************************************
Summary:
    This function sets a window's saturation level.

Description:
    Sets the saturation level of a window by adjusting color space conversion
    matrices.

Input:
    hWindow        - A valid window handle created earlier.
    sSaturation    - Window's saturation level, from -32768 to 32767.
                     Default value is 0.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set saturation level for the window.

See Also:
    BVDC_Window_GetSaturation
**************************************************************************/
BERR_Code BVDC_Window_SetSaturation
    ( BVDC_Window_Handle               hWindow,
      int16_t                          sSaturation );

/***************************************************************************
Summary:
    This function gets a window's saturation level.

Description:
    Gets the saturation level of a window.

Input:
    hWindow        - A valid window handle created earlier.

Output:
    psSaturation   - Window's current saturation level, from -32768 to
                     32767.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned saturation value for the window.

See Also:
    BVDC_Window_SetSaturation
**************************************************************************/
BERR_Code BVDC_Window_GetSaturation
    ( BVDC_Window_Handle               hWindow,
      int16_t                         *psSaturation );

/***************************************************************************
Summary:
    This function sets a window's hue level.

Description:
    Sets the hue level of a window by adjusting color space conversion
    matrices.

Input:
    hWindow        - A valid window handle created earlier.
    sHue           - Window's hue level, from -32768 to 32767.
                     Default value is 0.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_GetHue
**************************************************************************/
BERR_Code BVDC_Window_SetHue
    ( BVDC_Window_Handle               hWindow,
      int16_t                          sHue );

/***************************************************************************
Summary:
    This function gets a window's hue level.

Description:
    Gets the hue level of a window.

Input:
    hWindow        - A valid window handle created earlier.

Output:
    psHue          - Window's current hue level, from -32768 to 32767.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned hue value for the window.

See Also:
    BVDC_Window_SetHue
**************************************************************************/
BERR_Code BVDC_Window_GetHue
    ( BVDC_Window_Handle               hWindow,
      int16_t                         *psHue );

/***************************************************************************
Summary:
    This function sets a window's brightness level.

Description:
    Sets the brightness level of a window by adjusting color space conversion
    matrices.

Input:
    hWindow        - A valid window handle created earlier.
    sBrightness    - Window's brightness level, from -32768 to 32767.
                     Default value is 0.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_GetBrightness
**************************************************************************/
BERR_Code BVDC_Window_SetBrightness
    ( BVDC_Window_Handle               hWindow,
      int16_t                          sBrightness );

/***************************************************************************
Summary:
    This function gets a window's brightness level.

Description:
    Gets the brightness level of a window.

Input:
    hWindow        - A valid window handle created earlier.

Output:
    psBrightness   - Window's current brightness level, from -32768 to
                     32767.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned brightness value for the window.

See Also:
    BVDC_Window_SetBrightness
**************************************************************************/
BERR_Code BVDC_Window_GetBrightness
    ( BVDC_Window_Handle               hWindow,
      int16_t                         *psBrightness );

/***************************************************************************
Summary:
    This function sets a window's sharpness level.

Description:
    Sets the sharpness level of a window by using the
    PEP (Picture Enhancement Processing) block.  When this is called without a
    prior call to BVDC_Window_SetSharpnessConfig, the default sharpness
    settings are used.  The sharpness are process/control internally by one of
    the following hardware block TAB (Transient Adjustment Block) or the
    TNT (The Next TAB).

Input:
    hWindow          - A valid window handle created earlier.
    bSharpnessEnable - Enable sharpness control
    sSharpness       - Window's sharpness level, from -32768 to 32767.
                       Default value is 0.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set sharpness for the window.

See Also:
    BVDC_Window_GetSharpness
    BVDC_Window_SetSharpnessConfig
    BVDC_Window_GetSharpnessConfig
**************************************************************************/
BERR_Code BVDC_Window_SetSharpness
    ( BVDC_Window_Handle               hWindow,
      bool                             bSharpnessEnable,
      int16_t                          sSharpness );

/***************************************************************************
Summary:
    This function gets a window's sharpness level.

Description:
    Gets the sharpness level of a window.

Input:
    hWindow           - A valid window handle created earlier.

Output:
    pbSharpnessEnable - The correct sharpness control status
    psSharpness       - Window's current sharpness level, from -32768 to
                        32767.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned sharpness value for the window.

See Also:
    BVDC_Window_SetSharpness
**************************************************************************/
BERR_Code BVDC_Window_GetSharpness
    ( BVDC_Window_Handle               hWindow,
      bool                            *pbSharpnessEnable,
      int16_t                         *psSharpness );

/***************************************************************************
Summary:
    This function sets a window's custom sharpness settings.

Description:
    Sets the sharpness settings of a window. This works in conjunction
    with BVDC_Window_SetSharpness. BVDC_Window_SetSharpness must be called
    with bSharpnessEnable to true for this to take effect.

    Once the users call this function to set sharpness custom config, sSharpness
    value set in BVDC_Window_SetSharpness will be ignored, and any calls to
    function BVDC_Window_SetSharpness with different sSharpness value won't
    take effect until users call this function again with a NULL pointer to
    disable custom config first.

Input:
    hWindow                 - A valid window handle created earlier.
    pSharpnessSettings      - A pointer to a structure that contains
                              the  sharpness settings.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set custom sharpness settings for the window.

See Also:
    BVDC_SharpnessSettings
    BVDC_Window_SetSharpness
    BVDC_Window_GetSharpnessConfig
**************************************************************************/
BERR_Code BVDC_Window_SetSharpnessConfig
    ( BVDC_Window_Handle               hWindow,
      const BVDC_SharpnessSettings    *pSharpnessSettings );

/***************************************************************************
Summary:
    This function gets a window's sharpness level.

Description:
    Gets the current sharpness setting of a window.

Input:
    hWindow - A valid window handle created earlier.

Output:
    pSharpnessSettings - Window's current sharpness settings.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned the custom sharpness settngs for the
    window.

See Also:
    BVDC_SharpnessSettings
    BVDC_Window_SetSharpness
    BVDC_Window_SetSharpnessConfig
**************************************************************************/
BERR_Code BVDC_Window_GetSharpnessConfig
    ( BVDC_Window_Handle               hWindow,
      BVDC_SharpnessSettings          *pSharpnessSettings );

/***************************************************************************
Summary:
    This function sets a window's Color temperature level.

Description:
    Sets the Color temperature level of a window by adjusting color space
    conversion  matrices.

Input:
    hWindow - A valid window handle created earlier.
    sColorTemp - Window's color temperature level, from -32768 to 32767.
    Default value is 0.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set color temperature for the window.

See Also:
    BVDC_Window_GetColorTemp
**************************************************************************/
BERR_Code BVDC_Window_SetColorTemp
    ( BVDC_Window_Handle               hWindow,
      int16_t                          sColorTemp );

/***************************************************************************
Summary:
    This function gets a window's color temperature level.

Description:
    Gets the color temperature level of a window.

    Note:  The color temp value returned by this function becomes invalid
    after BVDC_Window_SetAttenuationRGB is used.

    BVDC_Window_SetColorTemp and BVDC_Window_SetAttenuationRGB perform
    similar functions using the same hardware but different conventions.
    Because BVDC_Window_SetAttenuationRGB parameters do not map one-to-one
    onto BVDC_Window_SetColorTemp values, internal color temperature values
    are not updated after BVDC_Window_SetAttenuationRGB is called and do not
    reflect the actual color adjustments in use.

    Thus, it is not recommended to mix BVDC_Window_Set/GetAttenuationRGB and
    BVDC_Window_Set/GetColorTemp calls, and BVDC_Window_GetColorTemp should
    not be used after a call to BVDC_Window_SetAttenuationRGB until
    BVDC_Window_SetColorTemp is called again.

Input:
    hWindow        - A valid window handle created earlier.

Output:
    psColorTemp   - Window's current color temperature level, from -32768 to
                     32767.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned color temperature value for the window.

See Also:
    BVDC_Window_SetColorTemp
    BVDC_Window_SetAttenuationRGB
    BVDC_Window_GetAttenuationRGB
**************************************************************************/
BERR_Code BVDC_Window_GetColorTemp
    ( BVDC_Window_Handle               hWindow,
      int16_t                         *psColorTemp );

/***************************************************************************

Summary:
   This function sets the conversion matrix at the current RGB attenuation.

Description:
    Sets the conversion matrix at the current RGB attenuation.

Input:
     hWindow        - A valid window handle created earlier.

Output:
     nAttenuationR/G/B, nOffsetR/G/B    - Conversion matrix in RGB domain
                                          in 20.11 signed fixed point.
                                          1 is represented by 0x800 and
                                          -1 is 0xffffffff.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned color temperature value for the window.

See Also: BVDC_Window_GetAttenuationRGB
**************************************************************************/
BERR_Code BVDC_Window_SetAttenuationRGB
    ( BVDC_Window_Handle               hWindow,
      int32_t                          nAttenuationR,
      int32_t                          nAttenuationG,
      int32_t                          nAttenuationB,
      int32_t                          nOffsetR,
      int32_t                          nOffsetG,
      int32_t                          nOffsetB);

/***************************************************************************

Summary:
   This function gets the conversion matrix at the current RGB attenuation.

Description:
    Gets the conversion matrix at the current RGB attenuation.

Input:
     hWindow        - A valid window handle created earlier.

Output:
     plAttenuationR/G/B, plOffsetR/G/B - Conversion matrix in RGB domain
                                         in 20.11 signed fixed point.
                                         1 is represented by 0x800 and
                                         -1 is 0xffffffff.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned color temperature value for the window.

See Also: BVDC_Window_SetAttenuationRGB
**************************************************************************/
BERR_Code BVDC_Window_GetAttenuationRGB
    ( BVDC_Window_Handle               hWindow,
      int32_t                         *plAttenuationR,
      int32_t                         *plAttenuationG,
      int32_t                         *plAttenuationB,
      int32_t                         *plOffsetR,
      int32_t                         *plOffsetG,
      int32_t                         *plOffsetB);

/***************************************************************************
Summary:
    This function sets a window's flesh tone level.

Description:
    Sets the flesh tone level of a window by adjusting
    CAB (Color Adustment Block) table in the
    PEP (Picture Enhancement Processing) block.  This feature provides the
    capability to improve the human skin color when it is distorted, i.e redish
    or blueish.  The higher the level, the stronger the effect it has.

    This function will be ignored while user is loading customized table.

Input:
    hWindow          - A valid window handle created earlier.
    ulFleshtone      - Window's flesh tone level, from 0 to 4.
                       Default value is 0.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set flesh tone for the window.

See Also:
    BVDC_Window_GetAutoFlesh
    BVDC_Window_LoadCabTable
**************************************************************************/
BERR_Code BVDC_Window_SetAutoFlesh
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulFleshtone );

/***************************************************************************
Summary:
    This function sets the coefficient index.

Description:

Input:
    hWindow     - A valid window handle created earlier.
    ulIndex     - the coefficient index.

Output:

Returns:
    BERR_SUCCESS - Coefficient index was set.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
**************************************************************************/
BERR_Code BVDC_Window_SetCoefficientIndex
    ( BVDC_Window_Handle               hWindow,
      const BVDC_CoefficientIndex     *pCtIndex );

/***************************************************************************
Summary:
    This function gets a window's flesh tone level.

Description:
    Gets the flesh tone level of a window.

Input:
    hWindow           - A valid window handle created earlier.

Output:
    pulFleshtone      - Window's current flesh tone level, from 0 to 4

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned flesh tone value for the window.

See Also:
    BVDC_Window_SetAutoFlesh
**************************************************************************/
BERR_Code BVDC_Window_GetAutoFlesh
    ( const BVDC_Window_Handle          hWindow,
      uint32_t                         *pulFleshtone );

/***************************************************************************
Summary:
    This function sets a window's green boost level.

Description:
    Sets the green boost level of a window by adjusting CAB table in the PEP
    block.  This feature provides the capability to improve green color while
    leaving other colors untouched.  The higher the level, the brighter green
    color we get.  This will give very pleasant view for the scenes with green
    grass and leaves.

    This function will be ignored while user is loading customized table.

Input:
    hWindow          - A valid window handle created earlier.
    ulGreenBoost     - Window's green boost level, from 0 to 4.
                       Default value is 0.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set green boost for the window.

See Also:
    BVDC_Window_GetGreenBoost
    BVDC_Window_LoadCabTable
**************************************************************************/
BERR_Code BVDC_Window_SetGreenBoost
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulGreenBoost );

/***************************************************************************
Summary:
    This function gets a window's green boost level.

Description:
    Gets the green boost level of a window.

Input:
    hWindow           - A valid window handle created earlier.

Output:
    pulGreenBoost     - Window's current green boost level, from 0 to 4

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned green boost value for the window.

See Also:
    BVDC_Window_SetGreenBoost
**************************************************************************/
BERR_Code BVDC_Window_GetGreenBoost
    ( const BVDC_Window_Handle          hWindow,
      uint32_t                         *pulGreenBoost );

/***************************************************************************
Summary:
    This function sets a window's blue boost level.

Description:
    Sets the blue boost level of a window by adjusting CAB table in the PEP
    block.  The feature provides the capability to improve the blue color while
    leaving other color untouched.  The higher the level, the brighter blue
    color we get.  This will give very pleasant view for the scenes with blue
    occean and sky.

    This function will be ignored while user is loading customized table.

Input:
    hWindow          - A valid window handle created earlier.
    ulBlueBoost      - Window's blue boost level, from 0 to 4.
                       Default value is 0.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set blue boost for the window.

See Also:
    BVDC_Window_GetBlueBoost
    BVDC_Window_LoadCabTable
**************************************************************************/
BERR_Code BVDC_Window_SetBlueBoost
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulBlueBoost );

/***************************************************************************
Summary:
    This function gets a window's blue boost level.

Description:
    Gets the blue boost level of a window.

Input:
    hWindow           - A valid window handle created earlier.

Output:
    pulBlueBoost      - Window's current blue boost level, from 0 to 4

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned blue boost value for the window.

See Also:
    BVDC_Window_SetBlueBoost
**************************************************************************/
BERR_Code BVDC_Window_GetBlueBoost
    ( const BVDC_Window_Handle          hWindow,
      uint32_t                         *pulBlueBoost );

/***************************************************************************
Summary:
    This function sets a window's CMS (Color Management System) controls.

Description:
    Sets the saturation gain and hue gain for each color.  These settings
    are used to adjust the CAB table in the PEP block.  The feature provides
    the capability to manage each color in the color bar independently.

    If used concurrently with other features in the CAB table (ie, auto
    flesh, green boost or blue boost), error will be reported.
    This function will be ignored while user is loading customized table.

Input:
    hWindow            - A valid window handle created earlier.
    pSaturationGain    - Saturation gain setting for each color,
                         Range from -140 to 140, 0 is disabled.
    pHueGain           - Hue gain setting for each color,
                         Range from -50 (-pi/6) to 50 (pi/6),
                         0 is disabled.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set blue boost for the window.

See Also:
    BVDC_ColorBar
    BVDC_Window_GetCmsControl
**************************************************************************/
BERR_Code BVDC_Window_SetCmsControl
    ( const BVDC_Window_Handle          hWindow,
      const BVDC_ColorBar              *pSaturationGain,
      const BVDC_ColorBar              *pHueGain );

/***************************************************************************
Summary:
    This function gets a window's CMS controls.

Description:
    Gets the CMS controls of a window.

Input:
    hWindow            - A valid window handle created earlier.

Output:
    pSaturationGain    - Window's current saturation gain setting for
                         each color, range from -140 to 140, 0 is disabled.
    pHueGain           - Window's current hue gain setting for each color,
                         Range from -50 (-pi/6) to 50 (pi/6),
                         0 is disabled. to 4

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned CMS controls for the window.

See Also:
    BVDC_ColorBar
    BVDC_Window_SetCmsControl
**************************************************************************/
BERR_Code BVDC_Window_GetCmsControl
    ( const BVDC_Window_Handle          hWindow,
      BVDC_ColorBar                    *pSaturationGain,
      BVDC_ColorBar                    *pHueGain );

/***************************************************************************
Summary:
    This function enables/disables a window's contrast stretch.

Description:
    Enables or disables contrast stretch for a window. When this
    is called without a prior call to BVDC_Window_SetContrastStretch,
    the default contrast stretch settings are used.

    In the case of chipset with new histogram hardware, this function
    will return error if histogram is enabled for any other window other
    than this window.  If histogram is not enabled before hand by user,
    this function will forced histogram to enable with default settings.

Input:
    hWindow - A valid window handle created earlier.
    bEnable - Indicates whether to enable or disable contrast stretch.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully enabled contrast stretch for the window.

See Also:
    BVDC_ContrastStretch
    BVDC_Window_SetContrastStretch
    BVDC_Window_GetContrastStretch
**************************************************************************/
BERR_Code BVDC_Window_EnableContrastStretch
    ( BVDC_Window_Handle               hWindow,
      bool                             bEnable );

/***************************************************************************
Summary:
    This function sets a window's contrast stretch settings.

Description:
    Sets the contrast stretch settings of a window.  These settings are used
    to calculate the LAB (Luma Adjustment Block) table for the PEP. Works in
    conjunction with BVDC_Window_EnableContrastStretch.
    BVDC_Window_EnableContrastStretch must be called for this to take effect.

Input:
    hWindow          - A valid window handle created earlier.
    pContrastStretch - A pointer to the constrast stretch setting structure

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set contrast stretch settings for the window.

See Also:
    BVDC_ContrastStretch
    BVDC_Window_EnableContrastStretch
    BVDC_Window_GetContrastStretch
**************************************************************************/
BERR_Code BVDC_Window_SetContrastStretch
    ( BVDC_Window_Handle               hWindow,
      const BVDC_ContrastStretch      *pContrastStretch );

/***************************************************************************
Summary:
    This function gets a window's contrast stretch settings.

Description:
    Gets the current contrast stretch settings of a window.

Input:
    hWindow           - A valid window handle created earlier.

Output:
    pContrastStretch  - Window's current contrast stretch setting

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned contrast stretch settings for the window.

See Also:
    BVDC_ContrastStretch
    BVDC_Window_EnableContrastStretch
    BVDC_Window_SetContrastStretch
**************************************************************************/
BERR_Code BVDC_Window_GetContrastStretch
    ( const BVDC_Window_Handle          hWindow,
      BVDC_ContrastStretch             *pContrastStretch );

/***************************************************************************
Summary:
    This function sets a window's blue stretch setting.

Description:
    This feature provides the capability to add blue component to very bright
    white content.  For this feature to work properly, users have to fill
    up the BVDC_BlueStretch structure as well as using the function
    BVDC_Window_LoadLabTableCustomized to load the Cb and Cr offset table.
    If users not provide the Cb and Cr offset table, the internal Cb, Cr
    tables will be used instead.  Calling this function with a NULL pointer
    for blue stretch structure to disable the feature.

Input:
    hWindow          - A valid window handle created earlier.
    pBlueStretch     - A pointer to the blue stretch setting structure

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set blue stretch for the window.

See Also:
    BVDC_BlueStretch
    BVDC_Window_GetBlueStretch
    BVDC_Window_LoadLabTableCustomized
**************************************************************************/
BERR_Code BVDC_Window_SetBlueStretch
    ( BVDC_Window_Handle               hWindow,
      const BVDC_BlueStretch          *pBlueStretch );

/***************************************************************************
Summary:
    This function gets a window's blue stretch setting.

Description:
    Gets the blue stretch setting of a window.

Input:
    hWindow           - A valid window handle created earlier.

Output:
    pBlueStretch      - Window's current blue stretch setting

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned blue stretch setting for the window.

See Also:
    BVDC_Window_SetBlueStretch
**************************************************************************/
BERR_Code BVDC_Window_GetBlueStretch
    ( const BVDC_Window_Handle          hWindow,
      BVDC_BlueStretch                 *pBlueStretch );

/***************************************************************************
Summary:
    This function sets a window's split screen settings.

Description:
    Sets the split screen settings of a window.  These settings are used
    to disable or enable features on the specified split screen.

Input:
    hWindow          - A valid window handle created earlier.
    pSplitScreen     - A pointer to the split screen setting structure

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set split screen settings for the window.

See Also:
    BVDC_SplitScreenMode
    BVDC_Window_SplitScreenSettings
    BVDC_Window_GetSplitScreenMode
    BVDC_Source_SetSplitScreenMode
    BVDC_Source_GetSplitScreenMode
**************************************************************************/
BERR_Code BVDC_Window_SetSplitScreenMode
    ( BVDC_Window_Handle                      hWindow,
      const BVDC_Window_SplitScreenSettings  *pSplitScreen );

/***************************************************************************
Summary:
    This function gets a window's split screen settings.

Description:
    Gets the current split screen settings of a window.

Input:
    hWindow           - A valid window handle created earlier.

Output:
    pSplitScreen      - Window's current split screen setting

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned split screen settings for the window.

See Also:
    BVDC_SplitScreenMode
    BVDC_SplitScreenSettings
    BVDC_Window_SetSplitScreenMode
**************************************************************************/
BERR_Code BVDC_Window_GetSplitScreenMode
    ( const BVDC_Window_Handle          hWindow,
      BVDC_Window_SplitScreenSettings  *pSplitScreen );

/***************************************************************************
Summary:
    This function sets a window's MosaicMode or ClearRect top configuration.

Description:
    Mosaic mode allows several decode channels to be displayed in a single BVN
    video path, that in one field time, we'll display multiple channels in a
    "mosaic" -- typically as an option for showing possible channels selections
    to the end user. In this mode, the multiple decodes appear as if mosaics
    on top of lower layer video/gfx, while the outside region of those rectangles
    is cleared;

    ClearRect feature allows multiple rectangles inside a video window to
    have see-through capability, such that the lower layer of video or gfx
    shows up to display as if "mosaics".

    Note:  MosaicMode is only available for MVD/XVD window.
    This API will return error if the chipset doesn't support this
    feature;

    Note:  If pstSettings->bVideoInMosaics is true, the widtch and heigh set by
    BVDC_Window_SetScalerOutput and BVDC_Window_SetDstRect must match. That is,
    no "destination cut" is allowed in this case.

Input:
    hWindow           - A valid window handle created earlier;
    bEnable           - enable/disable mosaic or ClearRect feature;
    pstMosaicSettings - settings of mosaic/ClearRect configuration; ignored
                        if bEnable is false;

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set Mosaic mode for the window.

See Also:
    BVDC_MosaicConfiguration, BVDC_Window_SetMosaicRectsVisibility,
    BVDC_Window_SetMosaicDstRects, BVDC_Window_SetMosaicTrackChannel
**************************************************************************/
BERR_Code BVDC_Window_SetMosaicConfiguration
    ( BVDC_Window_Handle               hWindow,
      bool                             bEnable,
      const BVDC_MosaicConfiguration  *pstSettings );

/***************************************************************************
Summary:
    This function gets a window's MosaicMode or ClearRect top configuration.

Description:

Input:
    hWindow           - A valid window handle created earlier;
    pbEnable          - enable status;
    pstMosaicSettings - settings of mosaic/ClearRect configuration;

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set Mosaic mode for the window.

See Also:
    BVDC_MosaicConfiguration, BVDC_Window_SetMosaicConfiguration,
    BVDC_Window_SetMosaicRectsVisibility,
    BVDC_Window_SetMosaicDstRects, BVDC_Window_SetMosaicTrackChannel
**************************************************************************/
BERR_Code BVDC_Window_GetMosaicConfiguration
    ( const BVDC_Window_Handle         hWindow,
      bool                            *pbEnable,
      BVDC_MosaicConfiguration        *pstSettings );

/***************************************************************************
Summary:
    This function specifies number of mosaic rectangles and their visibilities
    within a video window.

Description:
    Mosaic mode allows several decode channels to be displayed in a single BVN
    video path, that in one field time, we'll display multiple channels in a
    "mosaic" -- typically as an option for showing possible channels selections
    to the end user.

    ClearRect feature allows multiple rectangles inside a video window to
    have see-through capability, such that the lower layer of video or gfx
    surface up to display as if "mosaics".

    Note:  Mosaic mode is only available for MVD/XVD window.
    This API will return error if the chipset doesn't support CMP ClearRect feature
    or the window doesn't support ClearRect;

Input:
    hWindow          - A valid window handle created earlier;
    ulRectsCount     - number of mosaic rectangles in the window; 0 turns off
                       mosaic mode automatically;
    abRectsVisible[] - array of visibilities for each rectangle;

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set Mosaic mode for the window.

See Also:
    BAVC_MOSAIC_MAX, BVDC_Window_SetMosaicConfiguration,
    BVDC_Window_SetMosaicDstRects, BVDC_Window_SetMosaicTrackChannel
**************************************************************************/
BERR_Code BVDC_Window_SetMosaicRectsVisibility
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulRectsCount,
      const bool                       abRectsVisible[] );

/***************************************************************************
Summary:
    This function sets a window's Mosaics layout characteristics.

Description:
    Mosaic mode allows several decodes to be displayed in a single BVN video
    path, that in one field time, we'll display multiple channels in a "mosaic"
    -- typically as an option for showing possible channel selections to the
    end user.

    ClearRect feature allows multiple rectangles inside a video window to
    have see-through capability, such that the lower layer of video or gfx
    surface up to display as if "mosaics".

    This function sets a window's Mosaic/ClearRect  characteristics, i.e. number
    of mosaic rectangles and their placement/sizes. The size of the array given
    must match the value provided to BVDC_Window_SetMosaicRectsVisibility().

    If a window enables mosaic mode, decoder needs to feed a list of pictures
    to be displayed in the mosaic rectangles. If decoder feeds less than the
    number of rectangles, VDC would only update the given decodes and rest of
    the rectangles would be ignored; if decoder feeds more pictures than the
    rectangles, VDC would only update the mosaics and discard the rest of
    pictures list.

    Note:  Mosaic mode is only available for MVD/XVD window; this function only
    makes sense if Mosaic Config is enabled;

    Mosaic mode rectangles can be placed anywhere within the window's ScalerOutput
    rectangle, i.e. the lLeft/lTop offset is relative to the left top corner
    of the scaler output rectangle; each mosaic can not exceed the boundary
    of ScalerOutput rectangle;

    This API will return error if the chipset doesn't support CMP ClearRect feature
    or the window doesn't support ClearRect;
Input:
    hWindow          - A valid window handle created earlier;
    ulRectsCount     - Number of Mosaic rectangles; must <= BAVC_MOSAIC_MAX;
                       it MUST match with the count in BVDC_Window_SetMosaic();
    astRects[]       - array of Mosaic rectangles;

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set Mosaic mode for the window.

See Also:
    BVDC_Window_SetMosaicConfiguration, BVDC_Window_SetMosaicRectsVisibility,
    BVDC_Window_SetMosaicTrackChannel, BVDC_Rect, BAVC_MOSAIC_MAX
**************************************************************************/
BERR_Code BVDC_Window_SetMosaicDstRects
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulRectsCount,
      const BVDC_Rect                  astRects[] );

/***************************************************************************
Summary:
    This function specifies number of mosaic rectangles and their z-order
    values within a video window.

Description:
    Mosaic mode allows several decode channels to be displayed in a single BVN
    video path, that in one field time, we'll display multiple channels in a
    "mosaic" -- typically as an option for showing possible channels selections
    to the end user.

    ClearRect feature allows multiple rectangles inside a video window to
    have see-through capability, such that the lower layer of video or gfx
    surface up to display as if "mosaics".

    The window Z order values determine the order in that the visiable windows
    are blended onto the destination picture to accumulate their contribution.
    A window with z-order value of zero (0) would be blended first, and a
    window with z-order value of BVDC_Z_ORDER_MAX is blended at last.
    User must call this function to set the z order, otherwise the default order
    will be used.

    Does not take immediate effect. Requires an ApplyChanges() call.

    Note:  Mosaic mode is only available for MVD/XVD window. If same MVD/XVD
    source connects to multiple windows simultaneously, user must set same
    z-order value on each mosaic window for the same channel.

    This API will return error if the chipset doesn't support CMP ClearRect feature
    or the window doesn't support ClearRect;

Input:
    hWindow          - A valid window handle created earlier;
    ulRectsCount     - Number of Mosaic rectangles; must <= BAVC_MOSAIC_MAX;
                       it MUST match with the count in BVDC_Window_SetMosaic();
    aucZOrder[]      - array of z-order values;

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the z-order values.

See Also:
    BAVC_MOSAIC_MAX, BVDC_Window_SetMosaicConfiguration,
    BVDC_Window_SetMosaicDstRects, BVDC_Window_SetMosaicTrackChannel
**************************************************************************/
BERR_Code BVDC_Window_SetMosaicRectsZOrder
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulRectsCount,
      const uint8_t                    aucZOrder[] );


/***************************************************************************
Summary:
    This function specifies which of the mosaic's source channel should be used
    for source tracking.

Description:
    Mosaic mode allows several decode channels to be displayed in a single BVN
    video path, that in one field time, we'll display multiple channels in a
    "mosaic" -- typically as an option for showing possible channels selections
    to the end user.

    While all the mosaic buffers are presented at the same time to the VDC,
    we still need to know which mosaic to track so that the display rate can
    be adjusted and therefore adjust the decoder as well. This function only
    takes effect if this window is the master frame rate tracking window.
    If not set, VDC tracks the first channel of the mosaic list.

    Note, mosaic mode is only available for MVD/XVD window.
    This API will return error if the chipset doesn't support CMP ClearRect
    feature or the window doesn't support ClearRect;

Input:
    hWindow     - A valid window handle created earlier;
    ulChannelId - To be tracked channel number;
                  0 <= ulChannelId < BAVC_MOSAIC_MAX.
                  Note if the tracked channel does not exist, then source
                  tracking is effectively disabled.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set Mosaic mode for the window.

See Also:
    BVDC_Window_SetMosaicConfiguration, BVDC_Window_SetMosaicRectsVisibility,
    BVDC_Window_SetMosaicDstRects, BVDC_Window_SetMasterFrameRate,
    BVDC_Window_SetMasterFrameRate
**************************************************************************/
BERR_Code BVDC_Window_SetMosaicTrack
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulChannelId );

/***************************************************************************
Summary:
    This function get the window's mosaic framerate tracking channel.

Description:

Input:
    hWindow - A valid window handle created earlier.

Output:
    pulChannelId - mosaic framerate tracking channel.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set pan scan for the window.

See Also:
    BVDC_Window_SetMosaicTrack
**************************************************************************/
BERR_Code BVDC_Window_GetMosaicTrack
    ( BVDC_Window_Handle               hWindow,
      uint32_t                        *pulChannelId );

/***************************************************************************
Summary:
    Sets video window's capturing pixel format.

Description:
    A video window's video fields/frames can be captured into graphics
    surfaces for user. This function sets the pixel format for the capture
    graphics surfaces.

    Capturing pixel format might be any of the 8-bit 4:2:2, 10-bit 4:2:2,
    and 10-bit 4:4:4 YCbCr formats that are specified in BPXL. However,
    it is important that it matches the source  pixel format configured in
    the module that is going to use this graphics surface. Otherwise, the
    pixel color will be messed-up. The typical modules that may use graphics
    surface as source are BGRC, BP2D, VDC graphics feeder and mpeg feeder.
    Please notice that some of them might only accept a subset of pixel
    formats.

    If the video window being displayed is being captured, the capture pixel
    format must be: a) For MPEG Feeder cores with a minor version earlier
    than 0x60 (see MFD_0_REVISION_ID) and Video Feeder cores with a minor
    version earlier than 0x50, BPXL_eY18_Cr8_Y08_Cb8, BPXL_eCr8_Y18_Cb8_Y08,
    BPXL_eY18_Cb8_Y08_Cr8, or BPXL_eCb8_Y18_Cr8_Y08; otherwise, pink and
    green video will result. b) Cores with minor version 0x60 and 0x50
    (for MFD and VFD respectively) and later can support all formats.

    In case the that ePixelFormat is a higher bandwidth, and the system is
    doing dynamics RTS it will automatically lower the capure pixel format
    (byte per pixel) so that RTS is not violated.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - the window handle created earlier with BVDC_Window_Create. The
    captured buffer pertains to this window.
    ePixelFormat - the pixel format

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Open, BVDC_Window_SetUserCaptureBufferCount,
    BVDC_Window_GetBuffer, BVDC_Window_ReturnBuffer,
**************************************************************************/
BERR_Code BVDC_Window_SetPixelFormat
    ( BVDC_Window_Handle               hWindow,
      BPXL_Format                      ePixelFormat);

/***************************************************************************
Summary:
    Enables video to graphics surface capture and sets the max number of
    capture buffers allowed for the user to access at same time.

Description:
    A video window's video fields/frames can be captured into graphics
    surfaces (aka capture buffers). User can then acquire those surfaces with
    BVDC_Window_GetBuffer. This function enables this user capturing feature
    and sets the max number of capture buffers allowed for the user to hold
    in hands at same time.

    It must not exceed 2 with current design. If exceeded, an error is
    returned and no capture will take place. If 1 or 2 is passed, the user
    capture feature is enabled, and user can then acquire the last captured
    surface. If 0 is passed, no capture will take place, and it effectively
    turns off the user capture feature.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hWindow - the window handle created earlier with BVDC_Window_Create. The
    captured buffer pertains to this window.
    uiCaptureBufferCount - the max number of capture buffers the user app
    intends to use.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Open, BVDC_Window_SetPixelFormat,
    BVDC_Window_GetBuffer, BVDC_Window_ReturnBuffer,
**************************************************************************/
BERR_Code BVDC_Window_SetUserCaptureBufferCount
    ( BVDC_Window_Handle               hWindow,
      unsigned int                     uiCaptureBufferCount );

/***************************************************************************
Summary:
    Obtains the last captured buffer from the associated video window.

Description:
    After BVDC_Window_SetUserCaptureBufferCount is called to turn on the
    video to graphics surface (aka capture buffers) capturing feature, this
    function is used by user to acquire the last captured field/frame buffer.

    The returned pCapturedPic is a pointer to a BVDC_Window_CapturedImage
    struct. It includes the captured graphics surface handle and a few extra
    information such as video polarity. Whether a field or a frame is
    captured, is decided by BVDC automatically according to the video source
    polarity and whether deinterlace is involved before capturing. The
    surface pixel format is specified with BVDC_Window_SetPixelFormat. The
    width and height of the surface depends on the video source size and
    BVDC configurations such as source clipping and scaling. It is obvious
    that the height also depends on whether a field or a frame is captured.
    Although the size can not be directly specified by user, all features of
    the graphics surface can be queried from the surface handle.

    The acquired graphics surface could be used as a source to BGRC, BP2D,
    VDC graphics feeder and mpeg feeder. The surface is supposed to be
    returned to hWindow with BVDC_Window_ReturnBuffer as soon as it is no-
    longer needed by the user, as the max number of capture buffers allowed
    to be held by user at same time is limited.

    Takes immediate effect.

Input:
    hWindow - the window handle created earlier with BVDC_Window_Create.

Output:
    pCapturedImage - the last captured buffer associated with hWindow.

Returns:
    BVDC_ERR_CAPTURED_BUFFER_NOT_FOUND - No valid capture buffer to return
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

Note:
    A capture buffer can be successfully returned only if a valid picture
    has been captured on it and the buffer node is at certain position
    of the VDC multi-buffering picture nodes chain. Otherwise,
    BVDC_ERR_CAPTURED_BUFFER_NOT_FOUND will be returned. When this occurs,
    user application can wait for a small amount of time (e.g. 10 ms) and
    try calling BVDC_Window_GetBuffer() again. A capture buffer should become
    available in no more than 2 vSyncs.

See Also:
    BVDC_Window_ReturnBuffer, BVDC_Window_CapturedImage,
    BVDC_Open, BVDC_Window_SetPixelFormat,
**************************************************************************/
BERR_Code BVDC_Window_GetBuffer
    ( BVDC_Window_Handle               hWindow,
      BVDC_Window_CapturedImage       *pCapturedImage);

/***************************************************************************
Summary:
    Returns a captured buffer to the associated window.

Description:
    This function returns the capture buffer to the associated window
    immediately so that it can be used for next capture or get ready for
    being freed by calling BVDC_Window_SetUserCaptureBufferCount() next.

    If the intention is to return the buffer for next capture, an ApplyChanges()
    call is not required. If the intention is to get ready for being freed,
    an ApplyChanges() call however is required.

    The hCaptureBuffer must be a graphics surface handle that was aquired
    with BVDC_Window_GetBuffer from hWindow. Otherwise, unpredictable
    results may occur.

Input:
    hWindow - the window handle created earlier with BVDC_Window_Create.
    pCapturedImage - the captured buffer associated with hWindow.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeeds

Note:
    An outstanding capture buffer must be returned and freed if the
    associated window is to be shut down or reconfigured.

See Also:
    BVDC_Window_GetBuffer, BVDC_Window_CapturedImage,
    BVDC_Open, BVDC_Window_SetPixelFormat,
**************************************************************************/
BERR_Code BVDC_Window_ReturnBuffer
    ( BVDC_Window_Handle               hWindow,
      BVDC_Window_CapturedImage       *pCapturedImage );

/***************************************************************************
Summary:
    This function sets a window's color key settings.

Description:
    Sets the color key settings of a window.

Input:
    hWindow             - A valid window handle created earlier.
    pColorKeySettings   - settings of color key configuration.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set color key for the window.

See Also:
    BVDC_ColorKey_Settings
    BVDC_Window_GetColorKeyConfiguration
**************************************************************************/
BERR_Code BVDC_Window_SetColorKeyConfiguration
    ( BVDC_Window_Handle               hWindow,
      const BVDC_ColorKey_Settings    *pColorKeySettings );

/***************************************************************************
Summary:
    This function gets a window's color key settings.

Description:
    Gets the color key settings of a window.

Input:
    hWindow        - A valid window handle created earlier.

Output:
    pColorKeySettings - settings of color key configuration.

Returns:
    BERR_SUCCESS - Successfully returned hue value for the window.

See Also:
    BVDC_ColorKey_Settings
    BVDC_Window_SetColorKeyConfiguration
**************************************************************************/
BERR_Code BVDC_Window_GetColorKeyConfiguration
    ( BVDC_Window_Handle               hWindow,
      BVDC_ColorKey_Settings          *pColorKeySettings );


/***************************************************************************
Summary:
    This function set the luma sum configuration for a window.

Description:
    Specify what is the region of the window to compute the luma sum.
    The region must be bounded by the window's size.  For example
    if the window output is 1080p, then the region is 1920x1080.

Input:
    hWindow - A window handle that was previously created
    by BVDC_Window_Create.

    pLumaSettings - A pointer contains the settings.  If pLumaSettings is NULL
    The region will be size of compositor's format active video.  For
    example if the format 720p the size will be 1280x720p, or if 1080i the
    the region will be 1920x1080i.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.  If the window
       does not support luma sum computation.

    BERR_SUCCESS - Successfully queried the luma sum.

See Also:
    BVDC_LumaSettings,
    BVDC_Window_GetLumaStatus,
    BVDC_Window_GetLumaStatsConfiguration
**************************************************************************/
BERR_Code BVDC_Window_SetLumaStatsConfiguration
    ( BVDC_Window_Handle               hWindow,
      const BVDC_LumaSettings         *pLumaSettings );


/***************************************************************************
Summary:
    This function get the luma sum configuration for a window.

Description:
    Get the specified luma sum settings.

Input:
    hWindow - A window handle that was previously created
    by BVDC_Window_Create.

Output:
    pLumaSettings - A reference to store the settings.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.  If the window
       does not support luma sum computation.

    BERR_SUCCESS - Successfully queried the luma sum.

See Also:
    BVDC_LumaSettings,
    BVDC_Window_GetLumaStatus,
    BVDC_Window_SetLumaStatsConfiguration
**************************************************************************/
BERR_Code BVDC_Window_GetLumaStatsConfiguration
    ( BVDC_Window_Handle               hWindow,
      BVDC_LumaSettings               *pLumaSettings );


/***************************************************************************
Summary:
    This function gets the luma sum of current field/frame of this
    window.

Description:
    The luma sum value can be use to dynamically adjust backlit of the output
    depend on content.  It's envision that application will period read this
    luma sum value, and adjust backlit, brightness, and etc to their desired
    output.  The value of pLumaStatus is the value of the last field/frame.
    In the case of chipset of that does not support luma sum pLumaStatus will
    not be updated by this function.  The min/max is updated every two
    fields/frame.  In the case of chipset with new histogram hardware, this
    function will not return valid data unless users enable dynamic contrast
    or set Luma Stats Configuration for the corresponding window.

Input:
    hWindow - A compositor handle that was previously created
    by BVDC_Window_Create.

Output:
    pLumaStatus - A reference to store the luma sum status.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully queried the luma sum.

See Also:
    BVDC_LumaStatus,
    BVDC_Window_SetLumaStatsConfiguration.
**************************************************************************/
BERR_Code BVDC_Window_GetLumaStatus
    ( const BVDC_Window_Handle         hWindow,
      BVDC_LumaStatus                 *pLumaStatus );

/***************************************************************************
Summary:
    This function set the chroma status configuration for a window.

Description:
    Specifies the region of the window to obtain the chroma histogram from.
    The region's maximum size is bounded by the window's size.  For example
    if the window output is 1080p, then the region is 1920x1080.

    This requires a call to BVDC_ApplyChanges

Input:
    hWindow - A window handle that was previously created
    by BVDC_Window_Create.

    pChromaSettings - A pointer containing the settings.  If pChromaSettings
    is NULL, chroma histogram collection is disabled

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.  If the window
       does not support chroma histogram collection.

    BERR_SUCCESS - Successfully queried the chroma.

See Also:
    BVDC_ChromaSettings,
    BVDC_Window_GetChromaStatus,
    BVDC_Window_GetChromaStatsConfiguration
**************************************************************************/
BERR_Code BVDC_Window_SetChromaStatsConfiguration
    ( BVDC_Window_Handle               hWindow,
      const BVDC_ChromaSettings       *pSettings );


/***************************************************************************
Summary:
    This function get the chroma status configuration for a window.

Description:
    Get the specified chroma settings.

Input:
    hWindow - A window handle that was previously created
    by BVDC_Window_Create.

Output:
    pChromaSettings - A reference to store the settings.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.  If the window
       does not support luma sum computation.

    BERR_SUCCESS - Successfully queried the luma sum.

See Also:
    BVDC_ChromaSettings,
    BVDC_Window_GetChromaStatus,
    BVDC_Window_SetChromaStatsConfiguration
**************************************************************************/
BERR_Code BVDC_Window_GetChromaStatsConfiguration
    ( BVDC_Window_Handle               hWindow,
      BVDC_ChromaSettings             *pSettings );


/***************************************************************************
Summary:
    This function gets the chroma histogram of the given window.

Description:
    The chroma histogram can be used to dynamically adjust the backlight of
    the output.  It's envisioned that the application will periodically read
    the chroma histogram. The value of pChromaStatus is not field/frame accurate
    because of hardware limitation. At the maximum, chroma histogram is
    available every other frame.

    In the case of a chipset that does not support chroma histogram,
    pChromaStatus will not be updated by this function.

    To obtain the chroma histogram, BVDC_Window_SetChromaStatsConfiguration
    must be called; otherwise, a NULL is returned for pChromaStatus.

Input:
    hWindow - A compositor handle that was previously created
    by BVDC_Window_Create.

Output:
    pChromaStatus - A reference to store the chroma histogram.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully queried the chroma histogram.

See Also:
    BVDC_ChromaStatus,
    BVDC_Window_SetChromaStatsConfiguration.
**************************************************************************/
BERR_Code BVDC_Window_GetChromaStatus
    ( const BVDC_Window_Handle         hWindow,
      BVDC_ChromaStatus               *pStatus );

/***************************************************************************
Summary:
    Sets the 3x5 color matrix in CMP for the video window.

Description:
    This matrix is used to convert the incoming source color space into
    output color space.
    Note, setting this matrix will override the VDC's internal one, i.e.
    the VDC brightness, contrast, hue and saturation adjustment to the
    internal matrix will not take effect any more. User is responsible
    of adjusting it at upper layer.

    The values used for the color matrix are equal to the values stored
    in the pl32_Matrix array right shifted by the ulShift value. In
    addition, negative values are converted to positive values before
    shifting and then turned back into negative values. For
    example, if a value in the matrix was 4 and the shift value was 2, then
    the final value is 1 (4>>2 = 1). If a value in the matrix was 7
    and the shift value was 3, then the final value would be 0.875.
    If a value in the matrix was -7 and the shift value was 3 then the
    final values is -0.875.

    [Y_out ]   [M[0]  M[1]  M[2]  M[3] [M[4] ]   [Y_in ]
    [Cb_out]   [M[5]  M[6]  M[7]  M[8] [M[9] ]   [Cb_in]
    [Cr_out] = [M[10] M[11] M[12] M[13][M[14]] * [Cr_in]
    [A_out ]   [0     0     0     1     0    ]   [A_in ]
    [  1   ]   [0     0     0     0     1    ]   [  1  ]

    {Y_in, Cb_in, Cr_in, A_in}     = Input pixel before apply M matrix.
    {Y_out, Cb_Out, Cr_out, A_out} = Output pixel after apply M matrix.

    Note: The last two rows of the above matrix are used to facilitate
    computation only. It is not implemented physically in hardware.
    Columns 0 to 2 are multiplicative coefficients. Columns 4 and 5 are
    additive coefficients.  Note that for video csc the current hw does not
    support alpha multiply.

    With source auto format detection, the source input color
    space could dynamically change between SD and HD; if user override
    of the window color matrix is not updated in a prompt way, the
    color transition artifact could appear with dynamic source format
    change.

Input:
    hWindow     - A valid window handle created earlier.
    bOverride   - Whether to override VDC internal matrix with the supplied
                  one; if false, pl32_Matrix and ulShift will be ignored,
                  and the default matrices inside VDC will take place;
    pl32_Matrix - An int32_t array that holds the 3x4 (or 5x3) matrix coeffs.
    ulShift     - Matrix bit shift value; this parameter specifies the
                  number of fractional bits in the custom matrix coeffs;
                  its intention is to make API portable and hardware
                  independent; user should choose a much higher precision
                  than current hw precision to be future prove; VDC
                  will convert the user matrxi coeffs into final hw
                  settings and hide chip specific precision difference;

Output:
    None

Returns:
    BERR_SUCCESS - Color matrix was set.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
    BVDC_Window_SetContrast
    BVDC_Window_SetSaturation
    BVDC_Window_SetHue
    BVDC_Window_SetBrightness
****************************************************************************/
BERR_Code BVDC_Window_SetColorMatrix
    ( BVDC_Window_Handle               hWindow,
      bool                             bOverride,
      const int32_t                    pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                         ulShift );

/***************************************************************************
Summary:
    Gets the 3x5 color matrix for the window.

Description:
    This matrix is used to convert the incoming source color space into
    CbYCr color space.  This fuction will get the current matrix in use.
    Either internal by VDC's or by user specified with bOverided.

    The values used for the color matrix are equal to the values stored
    in the pl32_Matrix array right shifted by the ulShift value. In
    addition, negative values are converted to positive values before
    shifting and then turned back into negative values. For
    example, if a value in the matrix was 4 and the shift value was 2, then
    the final value is 1 (4>>2 = 1). If a value in the matrix was 7
    and the shift value was 3, then the final value would be 0.875.
    If a value in the matrix was -7 and the shift value was 3 then the
    final values is -0.875.

    The values within the color matrix are in the following order:

    [Y_out ]   [M[0]  M[1]  M[2]  M[3] [M[4] ]   [Y_in ]
    [Cb_out]   [M[5]  M[6]  M[7]  M[8] [M[9] ]   [Cb_in]
    [Cr_out] = [M[10] M[11] M[12] M[13][M[14]] * [Cr_in]
    [A_out ]   [0     0     0     1     0    ]   [A_in ]
    [  1   ]   [0     0     0     0     1    ]   [  1  ]

    {Y_in, Cb_in, Cr_in, A_in}     = Input pixel before applying M matrix.
    {Y_out, Cb_Out, Cr_out, A_out} = Output pixel after applying M matrix.

    Note: The last two rows of the above matrix are used to facilitate
    computation only. It is not implemented physically in hardware.
    Columns 0 to 2 are multiplicative coefficients. Columns 4 and 5 are
    additive coefficients.  Note that for video csc the current hw does not
    support alpha multiply.

Input:
    hSource     - A valid source handle created earlier.

Output:
    pbOverride  - Whether the matrix was  override VDC internal matrix with
    the supplied one; if false, pl32_Matrix and ulShift will be ignored,
    and the default matrices inside VDC will take place;

    pl32_Matrix - An int32_t array that holds the 3x5 matrix coeffs.

    pulShift    - Matrix bit shift value; this parameter specifies the
    number of fractional bits in the custom matrix coeffs; its intention is to
    make API portable and hardware independent; user should choose a much
    higher precision than current hw precision to be future prove; VDC
    will convert the user matrxi coeffs into final hw settings and hide chip
    specific precision difference;

    if *pbOverride is true it will return the shift value previous set
    by user otherwise it will return shift value of 16; or another word the
    matrix element is in the form of S15.16 2's complement.

Returns:
    BERR_SUCCESS - Color matrix was set.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
****************************************************************************/
BERR_Code BVDC_Window_GetColorMatrix
    ( BVDC_Window_Handle               hWindow,
      bool                            *pbOverride,
      int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                        *pulShift );

/***************************************************************************
Summary:
    This function loads the CAB (Color Adjustment Block) table.

Description:
    Disable or enable CAB block by providing the Cab table pointer.  A NULL
    pointer disables CAB. A partial table can be loaded by using the
    ulOffset and ulSize parameters.

    A few caveats follow.

    1. If partial tables are to be loaded, it is required that the entire table
    be loaded first because the uninitialized portions of the table cannot be
    filled in with initial values that are guaranteed to work correctly with
    the partially loaded table.

    2. When loading partial tables, the VDC cannot guarantee that it would be
    error free. It is the responsiblilty of the end user to assure its
    validity and operationability with the rest of the PEP block.

    3. The CAB table is persistent. If disabled, the table will not be erased.
    The window would have to be closed to reset the table.

    4. Once user table is loaded, user cannot select auto flesh, green strecth,
    blue boost, and CMS algorithm until user disable customized table by
    passing in the NULL pointer table.

Input:
    hWindow     - A valid window handle created earlier.
    pulCabTable - pointer to the Cab table.
    ulOffset    - the offset into the table
    ulSize      - the size of the table

Output:

Returns:
    BVDC_ERR_PEP_WINDOW_NOT_SUPPORT - Window not support PEP
    BVDC_ERR_PEP_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully load the CAB table.

See Also:
    BVDC_Window_SetGreenBoost
    BVDC_Window_SetBlueBoost
    BVDC_Window_SetAutoFlesh
**************************************************************************/
BERR_Code BVDC_Window_LoadCabTable
    ( BVDC_Window_Handle               hWindow,
      const uint32_t                  *pulCabTable,
      uint32_t                         ulOffset,
      uint32_t                         ulSize );

/***************************************************************************
Summary:
    This function loads the customized LAB (Luma Adjustment Block) table.

Description:
    This function provides the capability to load the user defined luma table
    or Cb and Cr tables or both.  NULL pointer for all three tables will
    disable user LAB (Luma Adjustment Block), otherwise enable.  A partial
    table can be loaded by using the ulOffset and ulSize parameters.

    A few caveats follow.

    1. If partial tables are to be loaded, it is required that the entire table
    be loaded first because the uninitialized portions of the table cannot be
    filled in with initial values that are guaranteed to work correctly with
    the partially loaded table.

    2. When loading partial tables, the VDC cannot guarantee that it would be
    error free. It is the responsiblilty of the end user to assure its
    validity and operationability with the rest of the PEP block.

    3. The LAB (Luma Adjustment Block) table is persistent. If disabled, the
    table will not be erased.  The window would have to be closed to reset the
    table.

    4. Once the luma table is loaded, user cannot use the dynamic contrast
    feature until disabling user luma table by calling the function again with
    a NULL pointer for luma table.

Input:
    hWindow      - A valid window handle created earlier.
    pulLumaTable - pointer to the luma table
    pulCbTable   - pointer to the Cb table
    pulCrTable   - pointer to the Cr table
    ulOffset     - the offset into the LAB (Luma Adjustment Block) table
    ulSize       - the size of the LAB (Luma Adjustment Block) table

Output:

Returns:
    BVDC_ERR_PEP_WINDOW_NOT_SUPPORT - Window not support PEP
    BVDC_ERR_PEP_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully load the LAB (Luma Adjustment Block) table.

See Also:
    BVDC_Window_EnableContrastStretch
    BVDC_Window_SetContrastStretch
    BVDC_Window_GetContrastStretch
**************************************************************************/
BERR_Code BVDC_Window_LoadLabTableCustomized
    ( BVDC_Window_Handle               hWindow,
      const uint32_t                  *pulLumaTable,
      const uint32_t                  *pulCbTable,
      const uint32_t                  *pulCrTable,
      uint32_t                         ulOffset,
      uint32_t                         ulSize );

/***************************************************************************
Summary:
    This function loads the LAB (Luma Adjustment Block) table.

Description:
    Disable or enable LAB (Luma Adjustment Block) block by providing the Lab
    table pointer.  A NULL pointer disables user LAB. A partial table can be
    loaded by using the ulOffset and ulSize parameters.  This function should
    be replaced by the newer function BVDC_Window_LoadLabTableCustomized.

    A few caveats follow.

    1. If partial tables are to be loaded, it is required that the entire table
    be loaded first because the uninitialized portions of the table cannot be
    filled in with initial values that are guaranteed to work correctly with
    the partially loaded table.

    2. When loading partial tables, the VDC cannot guarantee that it would be
    error free. It is the responsiblilty of the end user to assure its
    validity and operationability with the rest of the PEP block.

    3. The LAB table is persistent. If disabled, the table will not be erased.
    The window would have to be closed to reset the table.

Input:
    hWindow     - A valid window handle created earlier.
    pulLabTable - pointer to the Lab table.
    ulOffset    - the offset into the table
    ulSize      - the size of the table

Output:

Returns:
    BVDC_ERR_PEP_WINDOW_NOT_SUPPORT - Window not support PEP
    BVDC_ERR_PEP_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully load the LAB table.

See Also:
    BVDC_Window_LoadLabTableCustomized
    BVDC_Window_EnableContrastStretch
    BVDC_Window_SetContrastStretch
    BVDC_Window_GetContrastStretch
**************************************************************************/
BERR_Code BVDC_Window_LoadLabTable
    ( BVDC_Window_Handle               hWindow,
      const uint32_t                  *pulLabTable,
      uint32_t                         ulOffset,
      uint32_t                         ulSize );

/***************************************************************************
Summary:
    This function sets the window game mode buffer delay.

Description:
    The game mode delay control can place the sync-slipped window's multi-
    buffer playback pointer to a target offset relative to its capture
    pointer to achieve smaller multi-buffer delay and faster game response.

    Note, to make sure the game mode delay control converges and stablize,
    before enabling game mode delay control function, user should make sure
    the followings:

    1) make the window's display track a DPCR timebase;
    2) program the DPCR timebase to lock the window's source rate;

    Also note,

    - This function is currently meant for the DTV application with
      DVO panel output; analog output might cause TV rolling or loss of sync
      during adjustment; analog and digital outputs of the same display cannot
      simultaneously turn on if a window enables game mode;
    - A display can only have one window with game mode delay
      control enabled.
    - Only sync-slipped window can have this feature.

Input:
    hWindow          - A valid window handle created earlier.
    pGameModeDelay - pointer to the game mode delay setting.

Output:

Returns:
    BVDC_ERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully load the set the game mode.

See Also:
    BVDC_Window_GameModeSettings
**************************************************************************/
BERR_Code BVDC_Window_SetGameModeDelay
    ( BVDC_Window_Handle                     hWindow,
      const BVDC_Window_GameModeSettings    *pGameModeDelay );

/***************************************************************************
Summary:
    This function gets the window game mode buffer delay settings.

Description:

Input:
    hWindow          - A valid window handle created earlier.
    pGameModeDelay - pointer to the game mode delay setting.

Output:

Returns:
    BVDC_ERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the game mode

See Also:
    BVDC_Window_GameModeSettings
**************************************************************************/
BERR_Code BVDC_Window_GetGameModeDelay
    ( BVDC_Window_Handle               hWindow,
      BVDC_Window_GameModeSettings    *pGameModeDelay );


/***************************************************************************
Summary:
    This function set the window dither controls.

Description:
    The dither controls the video surface to be smooth.

Input:
    hWindow - A valid window handle created earlier.
    pDither - pointer to the dithering control settings.

Output:

Returns:
    BVDC_ERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get configurations.

See Also:
    BVDC_Window_GetDitherConfiguration
**************************************************************************/
BERR_Code BVDC_Window_SetDitherConfiguration
    ( BVDC_Window_Handle               hWindow,
      const BVDC_DitherSettings       *pDither );


/***************************************************************************
Summary:
    This function get the window dither controls.

Description:
    The dither controls the video surface to be smooth.

Input:
    hWindow - A valid window handle created earlier.
    pDither - pointer to the dithering control settings.

Output:

Returns:
    BVDC_ERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set configurations.

See Also:
    BVDC_Window_SetDitherConfiguration
**************************************************************************/
BERR_Code BVDC_Window_GetDitherConfiguration
    ( const BVDC_Window_Handle         hWindow,
      BVDC_DitherSettings             *pDither );

/***************************************************************************
Summary:
    This function disables, enables anr for a video window.

Description:
    Anr stands for analog noise reduction.

    There are several ANR modes. To totally disable ANR, BVDC_FilterMode_eDisable
    should be used. To temporarily turn-off the ANR filtering without causing
    the BVN HW to reconfigure, eBypass is recommended. Please notice that with
    eBypass the ANR HW and the associated memory are not released for other
    source to use.

    In order to set ANR configuration with this API function, ANR's current
    configuration should be queried first by BVDC_Source_GetAnrConfiguration,
    and be modified, and then be passed to this API function. This ensures that
    old application software works correctly after more anr controls are added
    to BVDC_Anr_Settings.

    Anr is disabled by default.

    Note, in chips after 3548 B0 ANR shares capturer/feader operation with
    Deinterlacer to save memory and bandwidth.

    Note, 3548 ANR will support both 8-bit and 10-bit 4:2:2 pixel format for
    the reference picture storage, while older 3563 ANR only supports 8-bit 4:2:2.

    Does not take immediate effect.  Requires an ApplyChanges() call.

Input:
    hWindow    - A valid window handle created earlier.
    pAnrSettings - Video window's ANR setting.
Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set anr's configuration.

See Also:
    BVDC_Window_GetAnrConfiguration
**************************************************************************/
BERR_Code BVDC_Window_SetAnrConfiguration
    ( BVDC_Window_Handle               hWindow,
      const BVDC_Anr_Settings         *pAnrSettings );

/***************************************************************************
Summary:
    This function gets the current anr on/off setting for a window.

Description:

Input:
    hWindow - A valid window handle created earlier.

Output:
    pAnrSettings  - current window ANR setting; only eMode is effective for now;

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the current anr configuration for the source.

See Also:
    BVDC_Window_SetAnrConfiguration
**************************************************************************/
BERR_Code BVDC_Window_GetAnrConfiguration
    ( BVDC_Window_Handle               hWindow,
      BVDC_Anr_Settings               *pAnrSettings );

/***************************************************************************
Summary:
    This function sets the configuration for scaler.

Description:
    This function allow user to configure various parameters in scaler to
    improve scaler picture quality performance.  By default
    pScalerSettings->bSclVertDejagging       (ON)
    pScalerSettings->bSclHorzLumaDeringing   (ON)
    pScalerSettings->bSclVertLumaDeringing   (ON)
    pScalerSettings->bSclHorzChromaDeringing (ON)
    pScalerSettings->bSclVertChromaDeringing (ON)
    pScalerSettings->ulSclDejaggingCore      (0)
    pScalerSettings->ulSclDejaggingGain      (4)
    pScalerSettings->ulSclDejaggingHorz      (4)

Input:
    hWindow - A valid window handle created earlier.
    pScalerSettings  - current window scaler setting;

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the current scaler configuration.

See Also:
    BVDC_Window_GetScalerConfiguration
**************************************************************************/
BERR_Code BVDC_Window_SetScalerConfiguration
    ( BVDC_Window_Handle               hWindow,
      const BVDC_Scaler_Settings      *pScalerSettings );


/***************************************************************************
Summary:
    This function gets the configuration for scaler.

Description:
    This function allow user to get the configuration of various parameters
    in scaler that improve scaler picture quality performance.

Input:
    hWindow - A valid window handle created earlier.

Output:
    pScalerSettings  - current window scaler setting;

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the current scaler configuration.

See Also:
    BVDC_Window_SetScalerConfiguration
**************************************************************************/
BERR_Code BVDC_Window_GetScalerConfiguration
    ( BVDC_Window_Handle               hWindow,
      BVDC_Scaler_Settings            *pScalerSettings );

/***************************************************************************
Summary:
    This function gets the current status of the window.

Description:
    This function gets the current status of the window. The information can
    be used for lip sync etc.

Input:
    hWindow - A compositor handle that was previously created
    by BVDC_Window_Create.

Output:
    pWindowStatus - A reference to store the status.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully queried the luma sum.

See Also:
**************************************************************************/
BERR_Code BVDC_Window_GetStatus
    ( const BVDC_Window_Handle         hWindow,
      BVDC_Window_Status              *pWindowStatus );

/***************************************************************************
Summary:
    This function sets the offset of the right window.

Description:
    This function allows the user to set the X offset delta for the right window
    in 3D mode.

Input:
    hWindow - A compositor handle that was previously created
              by BVDC_Window_Create.
    lRWinXOffsetDelta - Right window X offset delta is relative to left window X offset.
              offset(Right window) = offset(Left window) + lRWinXOffsetDelta

Output:
    None.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the X-offset.

See Also:
    BVDC_Window_GetDstRightRect
**************************************************************************/
BERR_Code BVDC_Window_SetDstRightRect
    ( const BVDC_Window_Handle         hWindow,
      int32_t                          lRWinXOffsetDelta);

/***************************************************************************
Summary:
    This function gets the offset detla of the right window.

Description:
    This function allows the user to set the X-offset delta for the right window
    in 3D mode.

Input:
    hWindow - A compositor handle that was previously created
              by BVDC_Window_Create.

Output:
    plRWinXOffsetDelta - Right window X offset delta is relative to left window X offset.
              offset(Right window) = offset(Left window) + lRWinXOffsetDelta

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the plRWinXOffsetDelta.

See Also:
    BVDC_Window_SetDstRightRect
**************************************************************************/
BERR_Code BVDC_Window_GetDstRightRect
    ( const BVDC_Window_Handle         hWindow,
      int32_t                         *plRWinXOffsetDelta );

/***************************************************************************
Summary:
    Set the BASE and DELTA parameters for a window's bandwidth equation.
    This equation is used to determine whether we should scale before
    capture or after.

Description:
    ulDelta is 1000 based. For example, having ulDelta as 20 means tolerance
    (delta) is 2%. That is, the decision oscillation region is
    [0.98, 1.02].

    eSclCapBias specifies the preferred scaler position if the result
    of bandwidth equation falls in the decision oscillation region.

    This requires a call to BVDC_ApplyChanges to activate the settings.

Inputs:
    hWindow      - the window handle
    ulDelta      - tolerance value
    eSclCapBias  - prefered scaler/captuer bias

Returns:

See Also:
    BVDC_Window_GetBandwidthEquationParams
****************************************************************************/
BERR_Code BVDC_Window_SetBandwidthEquationParams
    ( BVDC_Window_Handle               hWindow,
      const uint32_t                   ulDelta,
      const BVDC_SclCapBias            eSclCapBias);


/***************************************************************************
Summary:
    Get the current settings for a window's bandwidth equation.
    This equation is used to determine whether we should scale before
    capture or after.

Description:
    ulDelta is 1000 based.

Inputs:
    hWindow       - the window handle
    pulDelta      - tolerance value for bandwidth equation
    eSclCapBias   - prefered scaler/captuer bias

Returns:

See Also:
    BVDC_Window_SetBandwidthEquationParams
****************************************************************************/
BERR_Code BVDC_Window_GetBandwidthEquationParams
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         *pulDelta,
      BVDC_SclCapBias                  *peSclCapBias);


/***************************************************************************
Summary:
    Get the window's capabilities.

Description:
    This function gets the window's capabilities.

Input:
    hWindow - A valid window handle created earlier.

Output:
    pCapabilities - point to the window capabilities structure.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Window_Capabilities
**************************************************************************/
BERR_Code BVDC_Window_GetCapabilities
    ( BVDC_Window_Handle               hWindow,
      BVDC_Window_Capabilities        *pCapabilities );

/***************************************************************************
 * Source configuration.
 **************************************************************************/

/***************************************************************************
Summary:
    This function gets source default setting structure.

Description:
    BVDC_Source_Settings inherent default setting structure could be
    queried by this API function prior to BVDC_Source_Create, modified and
    then passed to BVDC_Source_Create.  This save application tedious
    work of filling in the configuration structure.

Input:
    eSourceId - Default settings for specific source.

Output:
    pDefSettings - A reference to default settings structure.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get BVDC_Source_Settings default settings.

See Also:
    BVDC_Source_Create.
**************************************************************************/
BERR_Code BVDC_Source_GetDefaultSettings
    ( BAVC_SourceId                    eSourceId,
      BVDC_Source_Settings            *pDefSettings );

/***************************************************************************
Summary:
    This function querys the right VFD to feed graphics surface.

Description:
    VFD can be used to feed graphics surface. But for a given video window,
    not every VFD are proper to be connected to. This function returns the
    source ID of a proper VFD to be used to feed graphics surface for a
    video window, before the window is created.

Input:
    hCompositor - The compositor that the video window belongs to.
    eWindowId - The video window ID that is going to display the graphics
    surface.

Output:
    peSourceId - The source ID of a VFD good to feed graphics surface.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get vfd source ID.

See Also:
    BVDC_Source_Create.
**************************************************************************/
BERR_Code BVDC_Source_QueryVfd
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_WindowId                    eWindowId,
      BAVC_SourceId                   *peSourceId );

/***************************************************************************
Summary:
    Creates a source handle to a specific source device.

Description:
    Once created, the handle to a source device is required by source sub-
    module functions in order to configure the source device and is needed
    by BVDC_Window_Create that connects the source to a window of
    the same type (video type or graphics type).

    The process to configure and use a source device is as follows:

    o A handle to the specific source device is created by
      BVDC_Source_Create with its BAVC_SourceId enum and its
      given heap. A heap is given in case the source requires memory, eg.,
      3D-Comb and ANR.
    o Create a new window with this source handle, or connect this source
      to a window of the same type using BVDC_Window_Create.
    o Changes to the source are made through the various Set, Enable, and
      Disable source sub-module functions. If a specific state has never
      be set explicitly, the default value specified in each of the
      functions is used.
    o When BVDC_ApplyChanges is called, all new setting to the source are
      applied to hardware, together with the other new settings to VDC.
    o When no longer needed, the source handle should be destroyed with a
      call to BVDC_Source_Destroy.

    When eSourceId is one of the Mpeg or Vfd type, the source device is a
    video source, but it can also be used to feed graphics surface. In
    this case, the bGfxSrc bool in the BVDC_Source_Settings struct must
    be set to true, and this source must be connected to video windows.

    For a given video window ID, BVDC_Source_QueryVfd should be used to
    query the proper vfd ID that could be used to feed graphics surface
    for the window (before the window handle is created).

    Does not take immediate effect.  Requires an ApplyChanges() call.

Input:
    hVdc - Handle to the VDC module.

    eSourceId - Which specific source device the handle is created for.

    pDefSettings - Should be NULL for standard usage modes.

    Note if pDefSettings is non-NULL, pDefSettings->hHeap must be a valid heap
    for this chipset otherwise system behavior is undefined.

Output:
    phSource - The created source handle.  If failure occurred phSource
    will be NULL.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully created a handle to the specified source
    device.

See Also:
    BVDC_Source_Destroy
    BVDC_Source_GetDefaultSettings, BVDC_Source_QueryVfd
**************************************************************************/
BERR_Code BVDC_Source_Create
    ( BVDC_Handle                      hVdc,
      BVDC_Source_Handle              *phSource,
      BAVC_SourceId                    eSourceId,
      const BVDC_Source_Settings      *pDefSettings );

/***************************************************************************
Summary:
    Destroys a handle to a specific source device.

Description:
    Once this function is called the source handle can no longer be used to
    control the specific source device.

    All source handles should be destroyed before closing the VDC module
    handle by BVDC_Close. And before a source handle is destroyed, it should
    be disconnected from all windows, by destroying the window handle, or
    resetting the window's source handle to some other source.

    Does not take immediate effect.  Requires an ApplyChanges() call.

Input:
    hSource - The source handle to destroy.

Output:

Returns:
    BERR_INVALID_PARAMETER - hSource is not a valid source device handle.
    BERR_SUCCESS - Successfully destroyed

See Also:
    BVDC_Source_Create
**************************************************************************/
BERR_Code BVDC_Source_Destroy
    ( BVDC_Source_Handle               hSource );


/***************************************************************************
Summary:
    This function gets the source size of a window.

Description:
    The width and height of the feeder's output, or source to the window.

Input:
    hSource - A valid source handle created earlier.

Output:
    pulWidth  - The source width of the feeder.
    pulHeight - The source height of the feeder.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the source size of a window.

See Also:
    BVDC_Window_SetSrcClip.
**************************************************************************/
BERR_Code BVDC_Source_GetSize
    ( const BVDC_Source_Handle         hSource,
      uint32_t                        *pulWidth,
      uint32_t                        *pulHeight );

/***************************************************************************
Summary:
    This function sets the video format of a video source.

Description:
    This function currently only applies to the source type of 656 or
    HdDvi. It would return BERR_INVALID_PARAMETER if hSource represents other
    type of source.

    This functions specifies the source video format when auto format
    detection is NOT enabled. If auto format detection IS enabled, it is
    ignored and the source video format is dynamically auto-detected by BVDC.
    Refer to BVDC_Source_SetAutoFormat for detail.

    As source video format changes, the source aspect ratio is updated
    automatically by BVDC to the typical value according to the video format.
    However, it could be overridden by user. For detail refer to
    BVDC_Source_OverrideAspectRatio.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hSource - A valid source handle created earlier.
    eVideoFmt - A video source or input format.  Video source come in many
    different timing formats.  For analog the source format now can include
    many HD formats (i.e. 1080i) in addition to the traditional NTSC/PAL.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the video source format of a window.

See Also:
    BVDC_Source_GetVideoFormat, BVDC_Source_SetAutoFormat,
    BVDC_Source_OverrideAspectRatio.
**************************************************************************/
BERR_Code BVDC_Source_SetVideoFormat
    ( BVDC_Source_Handle               hSourceVdec,
      BFMT_VideoFmt                    eVideoFmt );

/***************************************************************************
Summary:
    Enable and setup source auto format-detection.

Description:
    This function currently only applies to the source type of
    HdDvi. It would return BERR_INVALID_PARAMETER if hSource represents other
    type of source.

    The source video format of the input video signal from HD_DVI/HDMI
    could dynamically change, this API function is used to enable and
    setup source auto format detection in BVDC.  In additional to the enable
    flag user can provide a list of video formats that it wanted BVDC to
    search for. This is especially useful for reducing the search time.

    When auto format-detection is enabled BVDC_Source_SetVideoFormat is
    ignored.

    As source video format change is auto-detected, the source aspect ratio
    is updated automatically by BVDC to the typical value according to the
    video format. However, it could be overridden by user. For detail refer
    to BVDC_Source_OverrideAspectRatio.

Input:
    hSource - A valid source handle created earlier.
    bAutoDetect - A flag specifying if VDC should do auto format detection.
    aeFormats - A list of supported formats to seach for.
    ulNumFormats - The number of the of the above list.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_GetVideoFormat, BVDC_Source_SetVideoFormat,
    BVDC_Source_OverrideAspectRatio.
**************************************************************************/
BERR_Code BVDC_Source_SetAutoFormat
    ( BVDC_Source_Handle               hSource,
      bool                             bAutoDetect,
      const BFMT_VideoFmt              aeFormats[],
      uint32_t                         ulNumFormats );

/***************************************************************************
Summary:
    Override the detected format with new remap format.

Description:
    This function overrides the detected format with the user specified format
    list.  In the case of PC input (or HDMI) input format are closely related
    that application would like the end-user to override the detected.  This
    work while auto-format detection is enabled (auto format enabled with
    BVDC_Source_SetAutoFormat).

    NOTE: This remap currently only work for PC input source, other sources
    are undefined.  In addition not all format can be remapped.  Only those
    formats of very similar can be done.  For example 1024x768p@60Hz,
    1280x768p@60Hz, and 1360x768p@60Hz.  This function assumpt user is fully
    aware of format remapping capabilities.

Input:
    hSource - A valid source handle created earlier.

    bReMap - This flag will enable/disable the remapping.  Default is false.

    aReMapFormats - This parameter contains the list of formats to be remapped.
    It contains format pairs that contains "from" and "to".  If enabled when PI
    first detects aReMapFormats[i].eFmtA it will set aReMapFormats[i].eFmtB
    instead.

    ulReMapFormats - This paramter indicates the number of remap pairs.  This
    number must be less than BFMT_VideoFmt_eMaxCount.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_SetAutoFormat, BVDC_Source_SetVideoFormat.
**************************************************************************/
BERR_Code BVDC_Source_SetReMapFormats
    ( BVDC_Source_Handle               hSource,
      bool                             bReMap,
      const BVDC_VideoFmtPair          aReMapFormats[],
      uint32_t                         ulReMapFormats );

/***************************************************************************
Summary:
    This function gets the current video format of a source.

Description:
    This function currently only applies to the source type of 656 or
    HdDvi. It would return BERR_INVALID_PARAMETER if hSource represents other
    type of source.

    This functions gets the current video format of a source.

    Source video format is either set with BVDC_Source_SetVideoFormat or
    auto-detected by BVDC, depending on whether auto format detection is
    enabled. Refer to BVDC_Source_SetAutoFormat for detail.

Input:
    hSourceVdec - A valid source handle created earlier.

Output:
    peVideoFmt - Current source video format.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the video source format of a window.

See Also:
    BVDC_Source_SetVideoFormat, BVDC_Source_SetAutoFormat.
**************************************************************************/
BERR_Code BVDC_Source_GetVideoFormat
    ( const BVDC_Source_Handle         hSourceVdec,
      BFMT_VideoFmt                   *peVideoFmt );

/***************************************************************************
Summary:
    Override the source aspect ratio.

Description:
    This function currently only applies to the source type of VDEC, 656, or
    HdDvi. It would return BERR_INVALID_PARAMETER if hSource represents other
    type of source.

    Source aspect ratio is used when aspect ratio correction is performed by
    BVDC.

    A source typically has aspect ratio 4:3 or 16:9 if its video format is
    SD type or HD type respectively. The typical aspect ratio corresponding
    to a video format could be found from the BFMT_VideoInfo structure
    queried with BFMT_GetVideoFormatInfoPtr.

    As source video format changes, the source aspect ratio is updated
    automatically by BVDC to the typical value according to the video format.

    However, there are some SD video contents that are supposed to be watched
    on wide screen. In this case the source aspect ratio is 16:9 although the
    source video format is of SD type.

    This API function allows user to override the source aspect ratio enum.

    For example, as the source video format dynamically changes and it is
    auto-detected by BVDC, the end user might see the picture is stretched
    either horizontally or vertically, and then he/she could use some
    application provided user interface to re-configure and that result in
    the calling to this API function and/or BVDC_Window_SetAspectRatioMode,
    till the aspect ratio shown out is satisfied.

    It is important to remember that the source aspect ratio set with
    BVDC_Source_OverrideAspectRatio would be updated automatically by BVDC
    as the source video format changes.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hSource - Source handle created earlier with BVDC_Source_Create.
    eAspectRatio - Desired aspect ratio.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
See Also:
    BVDC_Source_GetAspectRatio, BVDC_Source_OverrideAspectRatio_isr,
    BVDC_Source_SetVideoFormat, BVDC_Source_SetAutoFormat,
    BVDC_Source_SetAspectRatioCanvasClip,
    BVDC_Window_SetAspectRatioMode, BVDC_Window_SetNonLinearScl
**************************************************************************/
BERR_Code BVDC_Source_OverrideAspectRatio
    ( BVDC_Source_Handle               hSource,
      BFMT_AspectRatio                 eAspectRatio );

/***************************************************************************
Summary:
    Override the source aspect ratio in "_isr" context.

Description:
    This is the "_isr" version of BVDC_Source_OverrideAspectRatio. It is
    used in interrupt handler or critical section.

    The change will take effect immediately, without the need of calling to
    BVDC_ApplyChanges. It overrides the setting previously auto-updated by
    BVDC according to the source video format or activated by previous
    ApplyChanges. And also it could be overridden by future source video
    format changes or by future pair of calling to
    BVDC_Source_OverrideAspectRatio and BVDC_ApplyChanges.

Input:
    hSource - Source handle created earlier with BVDC_Source_Create.
    eAspectRatio - Desired aspect ratio.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
See Also:
    BVDC_Source_GetAspectRatio, BVDC_Source_OverrideAspectRatio
    BVDC_Source_SetVideoFormat, BVDC_Source_SetAutoFormat,
    BVDC_Source_SetAspectRatioCanvasClip,
    BVDC_Window_SetAspectRatioMode, BVDC_Window_SetNonLinearScl
**************************************************************************/
BERR_Code BVDC_Source_OverrideAspectRatio_isr
    ( BVDC_Source_Handle               hSource,
      BFMT_AspectRatio                 eAspectRatio );

/***************************************************************************
Summary:
    This function gets the current aspect ratio enum of a source.

Description:
    This function currently only applies to the source type of 656 or
    HdDvi. It would return BERR_INVALID_PARAMETER if hSource represents other
    type of source.

    This functions gets the current aspect ratio enum of a video source.

    Source aspect ratio is used when aspect raio correction is performed by
    BVDC. It is automatically updated by BVDC according to the source video
    format, and might be overriden with BVDC_Source_OverrideAspectRatio by
    user.

Input:
    hSourceVdec - A valid source handle created earlier.

Output:
    eAspectRatio - Current source aspect ratio enum.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the video source format of a window.

See Also:
    BVDC_Source_SetVideoFormat, BVDC_Source_SetAutoFormat,
    BVDC_Source_OverrideAspectRatio.
**************************************************************************/
BERR_Code BVDC_Source_GetAspectRatio
    ( const BVDC_Source_Handle         hSourceVdec,
      BFMT_AspectRatio                *peAspectRatio );

/***************************************************************************
Summary:
    This function sets the video source aspect ratio canvas clip.

Description:
    This function currently only applies to video source. It would return
    BERR_INVALID_PARAMETER if hSource represents a graphics source.

    By default, the full source aspect ratio is considered to be contributed
    by the full source rectangle (i.e. the source canvas), and based on that
    the source pixel aspect ratio is calculated. However, in some video
    content it is a sub-rectangle area (inside the source canvas) that make
    up the full source aspect ratio, and the remaining surrounding edge does
    not convey real picture pixel. This API function allows user to specify
    the sub-rectangle by describing the numbers of pixels to clip from the
    left, right, top, and bottom of the full source canvas.

    Does not take immediate effect. Requires an ApplyChanges() call.

    If the the specified clipping row or column number is bigger than half of
    the full source size, it will causes the following ApplyChanges to fail
    and to return an error.

Input:
    hSource - A valid source handle created earlier.
    ulLeft - The number of columns to clip from the left of source canvas.
    ulRight - The number of columns to clip from the right of source canvas.
    ulTop - The number of rows to clip from the top of source canvas.
    ulBottom - The number of rows to clip from the bottom of source canvas.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the video source aspect ratio canvas clip.

See Also:
    BVDC_Window_SetAspectRatioMode, BVDC_Source_OverrideAspectRatio,
    BVDC_Source_SetAspectRatioCanvasClip_isr,
    BVDC_Display_SetAspectRatioCanvasClip.
**************************************************************************/
BERR_Code BVDC_Source_SetAspectRatioCanvasClip
    ( BVDC_Source_Handle               hSource,
      uint32_t                         ulLeft,
      uint32_t                         ulRight,
      uint32_t                         ulTop,
      uint32_t                         ulBottom );

/***************************************************************************
Summary:
    This function sets the video source aspect ratio canvas clip in _isr mode.

Description:
    This is the "_isr" version of BVDC_Source_SetAspectRatioCanvasClip. It
    is used in interrupt handler or critical section.

    The change will take effect immediately, without the need of calling to
    BVDC_ApplyChanges. It overrides the setting activated by previous
    ApplyChanges and also it could be overridden by future pair of calling
    to BVDC_Source_SetAspectRatioCanvasClip and BVDC_ApplyChanges.

    If the the specified clipping row or column number is bigger than half of
    the full source size, the _isr setting is simply ignored.

Input:
    hSource - A valid source handle created earlier.
    ulLeft - The number of columns to clip from the left of source canvas.
    ulRight - The number of columns to clip from the right of source canvas.
    ulTop - The number of rows to clip from the top of source canvas.
    ulBottom - The number of rows to clip from the bottom of source canvas.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the video source aspect ratio canvas clip.

See Also:
    BVDC_Window_SetAspectRatioMode, BVDC_Source_OverrideAspectRatio,
    BVDC_Source_SetAspectRatioCanvasClip,
    BVDC_Display_SetAspectRatioCanvasClip.
**************************************************************************/
BERR_Code BVDC_Source_SetAspectRatioCanvasClip_isr
    ( BVDC_Source_Handle               hSource,
      uint32_t                         ulLeft,
      uint32_t                         ulRight,
      uint32_t                         ulTop,
      uint32_t                         ulBottom );

/***************************************************************************
Summary:
    This function select the input of the HD_DVI source

Description:

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hSource - A valid source handle created earlier.
    ulInputPort - input port setting

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully

See Also:
**************************************************************************/
BERR_Code BVDC_Source_SetInputPort
    ( BVDC_Source_Handle               hSourceHddvi,
      uint32_t                         ulInputPort );

/***************************************************************************
Summary:
    This function get the input of the HD_DVI source

Description:
    Returns the input port state applied to the source handle specified

Input:
    hSource - A valid source handle created earlier.

Output:
    pulInputPort - input port setting applied

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully
**************************************************************************/
BERR_Code BVDC_Source_GetInputPort
    ( BVDC_Source_Handle               hSourceHddvi,
      uint32_t                        *pulInputPort );

/***************************************************************************
Summary:
    Sets Mpeg source information.

Description:
    This function is typically called by application for every frame / field
    of display at ISR to set MPEG source information.

    The function passes a BAVC_MFD_Field struct by pointer pvFieldData. It
    represents the next MPEG frame /field to pass from the MVD/XVD module
    to the VDC display pipeline.

    The MPEG information includes format change and vsync information, and
    it's used to program MPEG/Video feeder, capture and scaler block.

    Application needs to get MPEG information from MVD/XVD before calling
    this function.  Application can also overwrite data from MVD/XVD to pass
    user settings, such as pan scan etc, or to correct errors.

    After BVDC_Source_MpegDataReady_isr is called, roughly one field / frame
    later the pixel buffer represented by pvFieldData would be fetched by a
    VDC mpeg feeder, and the fetching would last for one field / frame display
    time. That means the pixel buffer passed by the first calling of
    BVDC_Source_MpegDataReady_isr would be freed by the VDC module when
    BVDC_Source_MpegDataReady_isr is called at the third time, and so on, ....

    This function can also be called by application for every frame / field
    of display at ISR to set a graphics surface as a video source to the MFD.
    In this case the BAVC_MVD_Field struct contains a valid hSurface field.

    Different usages require different values to the fields of the passed
    BAVC_MFD_Picture struct. Refer to BAVC_MFD_Picture for detail.

Input:
    pvSourceHandlePtr - Specifies which source device to set
    pvFieldData - Contains MPEG information to pass from MVD to VDC for MPEG
    source. This should only be pass with an Mpeg source.
    iParm2 - Second optional parameter given to the
    BMVD_InstallInterruptCallback function.

Output:

Returns:
    BERR_INVALID_PARAMETER - hSource is not a valid source handle.
    BERR_SUCCESS - Successfully enabled the source device.

See Also:
    BVDC_Source_Create
    BVDC_Source_Destroy
****************************************************************************/
void BVDC_Source_MpegDataReady_isr
    ( void                            *pvSourceHandle,
      int                              iParm2,
      void                            *pvFieldData );

/***************************************************************************
Summary:
    Get the interrupt name for MPEG data ready.

Description:
    This function return the interrupt name when RUL of a MPEG source
    completed execution.  The interrupt could driven by VEC Vsync,
    or combination of CAPTURE DONE.

    Example;
        (1) VEC Vsync -> RDC Slot

        (2) (Capture0 Done && Capture1 Done) -> RDC Slot.

Input:
    hSource - Specifies which MPEG source device to set
    pFieldData - Contains MPEG information to pass from MVD to VDC for MPEG
    source. This should only be pass with an Mpeg source.

    eFieldPolarity - Field polarity of the field associates with the
    interrupt.

Output:
    pInterruptName - The L2 interrupt name that can be use with BINT
    to create callback.

Returns:
    BERR_INVALID_PARAMETER - Some parameter is invalid.
    BERR_SUCCESS - Successfully returned interrupt id

See Also:
    BVDC_Source_MpegDataReady_isr, BINT_CreateCallback.

****************************************************************************/
BERR_Code BVDC_Source_GetInterruptName
    ( BVDC_Source_Handle               hSource,
      const BAVC_Polarity              eFieldPolarity,
      BINT_Id                         *pInterruptName );

/***************************************************************************
Summary:
    Installs a source picture callback function.

Description:
    This function can be used by application to install source picture
    callback function. Once installed, the callback function will
    be called by VDC at _isr time for every field / frame of display, to
    get the info about the next picture (a video/graphics field/frame) for
    display, and to pass to user the current state of the used BVDC source
    sub-module for synchronization.

    Only one callback can be installed. Passing NULL to this function
    disables the callback.

    When the source is a type of HD_DVI, if the callback is disabled there
    are no AVC parameters associated with the captured buffer. VDC will use
    the default settings based on source format.

Input:
    hSource   - HD_DVI or graphics source handle created earlier with
    BVDC_Source_Create.
    BVDC_Source_PictureCallback_isr - Callback function to be called at _isr.
    pvParm1 - Pointer to a user defined struct that user wish to pass to the
    call back function. It might include info about the upstream module such
    as BGRC, BP3D or BXVD, and the BVDC source sub-module that is used to
    accept the upstream produced picture. It could also be NULL.
    iParm2 - An int number that user wish to pass to this call back function.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_SetVideoFormat
****************************************************************************/
BERR_Code BVDC_Source_InstallPictureCallback
    ( BVDC_Source_Handle               hSource,
      BVDC_Source_PictureCallback_isr  srcCallback,
      void                            *pvParm1,
      int                              iParm2 );

/***************************************************************************
Summary:
    Install a callback with VDC for SOURCE related update.

Description:
    This function install a callback with VDC to be called whenever VDC's
    changes or detected it new source property (59.94Hz, 60.00Hz, format,
    size, interlaced and etc) or other info.

    The pvVdcData pointer passes back by this callback is defines in bvdc.h
    called BVDC_Source_CallbackData it contains all necessary information.
    BVDC_Source_CallbackData also contains other information
    that is update by the source.  Information about the MPEG source can also
    be obtain with the callback from decoder via callback.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hSource - Source handle created earlier with BVDC_Source_Create.
    pfCallback - A user defined function pointer adapt to user update.
    pvParm1 - User defined structure.casted to void.
    iParm2 - User defined value.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_CallbackData
**************************************************************************/
BERR_Code BVDC_Source_InstallCallback
    ( BVDC_Source_Handle               hSource,
      const BVDC_CallbackFunc_isr      pfCallback,
      void                            *pvParm1,
      int                              iParm2 );


/***************************************************************************
Summary:
    Sets the video mute color for MPEG source.

Description:
    The colors set in this function are used by feeder to program a fixed
    color feed when bMute flag in BAVC_MVD_Field is set.

    This function can be used for smooth channel change to scan out a user
    defined fixed color instead of a garbage picture from MPEG buffer.

    MVD decides when to mute the picture by setting bMute flag in
    BAVC_MVD_Field to notify VDC to feed out a fixed color provided by user
    instead of using the supplied buffers from MVD.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hSource - The source device to modify.
    ucRed - The RED component of the mute color.
    ucGreen - The GREEN component of the mute color.
    ucBlue -  The BLUE component of the mute color.

Output:

Returns:
    BERR_INVALID_PARAMETER - hSource is not a valid source handle.
    BERR_SUCCESS - Successfully set video mute color.

See Also:
    BVDC_ApplyChanges
    BVDC_Source_GetVideoMuteColor
****************************************************************************/
BERR_Code BVDC_Source_SetVideoMuteColor
    ( BVDC_Source_Handle               hSource,
      const uint8_t                    ucRed,
      const uint8_t                    ucGreen,
      const uint8_t                    ucBlue );

/***************************************************************************
Summary:
    Gets the video mute color applied for the MPEG source.

Description:
    This fixed color can be used for smooth channel change to scan out a user
    defined fixed color instead of a garbage picture from MPEG buffer.

    MVD decides when to mute the picture by setting bMute flag in
    BAVC_MVD_Field to notify VDC to feed out a fixed color provided by user
    instead of using the supplied buffers from MVD.

Input:
    hSource - The source device to modify.

Output:
    pucRed - The RED component of the mute color.
    pucGreen - The GREEN component of the mute color.
    pucBlue -  The BLUE component of the mute color.

Returns:
    BERR_INVALID_PARAMETER - hSource is not a valid source handle.
    BERR_SUCCESS - Successfully disabled gamma correction

See Also:
    BVDC_ApplyChanges
    BVDC_Source_SetVideoMuteColor
****************************************************************************/
BERR_Code BVDC_Source_GetVideoMuteColor
    ( const BVDC_Source_Handle          hSource,
      uint8_t                          *pucRed,
      uint8_t                          *pucGreen,
      uint8_t                          *pucBlue );


/***************************************************************************
Summary:
    Set the source's mute mode.

Description:
    This function can be used for smooth channel change to display a user
    defined picture instead of a garbage picture.

    There are 3 posssible modes:
        BVDC_MuteMode_eDisable:
            Display the normal input video. This can be used to un-mute.
        BVDC_MuteMode_eConst:
            Display a const color defined by user.
        BVDC_MuteMode_eRepeat:
            Repeat the last valid picture

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hSource - A valid source handle created earlier.
    eMuteMode - The new mute mode to set.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_GetMuteMode
    BVDC_Source_SetVideoMuteColor
    BVDC_Source_GetVideoMuteColor
**************************************************************************/
BERR_Code BVDC_Source_SetMuteMode
    ( BVDC_Source_Handle               hSource,
      BVDC_MuteMode                    eMuteMode );

/***************************************************************************
Summary:
    Get the source's mute mode.

Description:
    This function gets the window's mute mode.

Input:
    hSource - A valid source handle created earlier.

Output:
    peMuteMode - point to the current mute mode.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_SetMuteMode
    BVDC_Source_SetVideoMuteColor
    BVDC_Source_GetVideoMuteColor
**************************************************************************/
BERR_Code BVDC_Source_GetMuteMode
    ( const BVDC_Source_Handle         hSource,
      BVDC_MuteMode                   *peMuteMode  );

/***************************************************************************
Summary:
    Specifies the graphics surface (set) to use in the graphics source device
    represented by the source handle.

Description:
    This function only applies to the source type of gfx feeder, mpeg feeder,
    and video feeder (vfd). It would return BERR_INVALID_PARAMETER if hSource
    represent other type of source handle.

    Graphics feeder can support all pixel formats defined in BPXL, mpeg feeder
    and vfd feeder can only support those 4:2:2 YCrCb formats. Both graphics
    feeder, mpeg feeder, and vfd in most new chips such as 7422, 7425 7344
    7346 and 7231 can support 3D display. However, the 3D display capability
    might be limited by the display window. Graphics feeder in some old chips
    such as 7400, 7405, 7325, and 7335 support a separate alpha surface.

    The struct member eInOrientation in BAVC_Gfx_Picture specifies the
    orientation of the graphics source surfaces. hSurface and hRSurface
    specify the main / left and right source surfaces. The rest of the
    BAVC_Gfx_Picture members are for alpha surfaces and are to be obsolete.

    If a BPXL_Plane member in BAVC_Gfx_Picture is not needed, it
    must be set as NULL.

    After a pixel is fetched from surfaces, it is automatically expanded
    into four 8-bits components by all means. For example, a palette format
    pixel is expanded into either ARGB or AYCrCb by color look-up, depending
    on the pixel format of the palette entries. Any missing chroma is
    computed from neighbor pixels. If alpha component does not exist for a
    pixel format, 0xff is used.

    If the surface's pixel buffer was provided by user when the surface was
    created, user has to ensure that the pixel buffer was allocated by
    BMMA_Alloc. Similarly, if the palette entry buffer was provided by the
    user when the surface's palette was created, user should also ensure
    that the palette entry buffer was allocated by  BMMA_Alloc. If any of
    the buffers was not allocated by BMMA_Alloc, the behavior is not
    specified.

    If the pixel format of this source surface is a palette format and the
    the surface's palette handle is not NULL, normally the palette table
    defined by the palette handle attached to the source surface would be
    loaded to the graphics feeder hardware when next BVDC_ApplyChanges is
    called. One exception is in the case that gamma correction is also
    enabled, then BVDC_ApplyChanges would fail and return an error.

    After a graphics surface (set) is set to BVDC for display, it will be
    fetched by hardware as soon as possible. When a surface is fetched by
    hardware, user should not change the content of its pixel buffer and
    palette buffer. Otherwise mixed partial new content and partial old
    content might be used for display. BVDC_Source_GetSurface can be used to
    query which surface (set) is currently on display. The surfaces set to
    BVDC before it are released from BVDC and are free for application to
    create new content.

    Setting with BVDC_Source_SetSurface will be displayed by BVDC right after
    the coming vsync if the surface width, height, pitch, and pixel format
    does not change from previous setting. Otherwise it is not to be applied
    until a call to BVDC_ApplyChanges is made.

    Besides BVDC_Source_SetSurface, the graphics surfaces could also be set
    by BVDC_Source_SetSurface_isr and BVDC_Source_PictureCallback_isr. Both
    of them are involved at ISR context and their setting are applied without
    the need of BVDC_ApplyChanges.

    However, It is recommended to use BVDC_Source_SetSurface, because it
    provides the best performance for smooth animation.

    If BVDC_Source_SetSurface is never called for this hSource and a source
    call back function is never installed for this hSource, after it is used
    by a window BVDC_ApplyChanges will return an error.

    Note: GFD 1 in 7550 chip can not perform horizontal, nor vertical scaling.
    nor color matrix. And it only supports P4 or YCbCr source surface format.

Input:
    hSource - The source device to be modified.
    pGfxPic - The data structure to contain main/left surface handle,
    right surface handle as well as its orientation.
    Null left surface handle results in an error return.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the main surface.

See Also:
    BVDC_Source_GetSurface
    BVDC_Window_SetSrcClip
    BVDC_Window_SetAlpha
    BVDC_ApplyChanges
    BVDC_Source_SetSurface_isr, BVDC_Source_PictureCallback_isr
****************************************************************************/
BERR_Code BVDC_Source_SetSurface
    ( BVDC_Source_Handle               hSource,
      const BAVC_Gfx_Picture          *pGfxPic);


/***************************************************************************
Summary:
    Query the source surface (set) which is currently fetched by hardware.

Description:
    This function currently only applies to the source type of graphics
    feeder and video feeder. It would return BERR_INVALID_PARAMETER if
    hSource represents other type of source handle.

    Refer to BVDC_Source_setSurface for usage.

    This function should only be used in user mode. Serious problem could
    happen if it is used in _isr context or in critical session.

Input:
    hSource - The source device to be modified.

Output:
    pGfxPic - The graphics surface (set) that is currently displayed by VDC.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the currently displayed main surface.

See Also:
    BVDC_Source_SetSurface, BVDC_Source_GetSurface_isr
****************************************************************************/
BERR_Code BVDC_Source_GetSurface
    ( BVDC_Source_Handle               hSource,
      BAVC_Gfx_Picture                *pGfxPic);


/***************************************************************************
Summary:
    Specifies the graphics surface (set) to use in the graphics source device
    represented by the source handle.

Description:
    This is the "_isr" version of BVDC_Source_SetSurface. It is used in
    interrupt handler or critical session.

    The setting with BVDC_Source_SetSurface_isr WILL be applied immediately,
    regardless of the calling to BVDC_ApplyChanges.

    For an example of applications, consider the case that a BGRC task is
    issued to produce a new graphics surface and a call back function is
    called at ISR context when the task is completed. Then inside the call
    back function BVDC_Source_SetSurface_isr could be called to set the
    newly produced graphics surface (set) on display. This surface (set)
    would be displayed by BVDC until it is changed by user.

Input:
    hSource - The source device to be modified.
    pGfxPic - The data structure to contain left/main surface handle, right
    surface handle as well as its orientation. Null left surface handle
    results in an error return.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the main surface.

See Also:
    BVDC_Source_GetSurface_isr,
    BVDC_Window_SetSrcClip, BVDC_Window_SetAlpha
    BVDC_Source_SetSurface, BVDC_Source_PictureCallback_isr
****************************************************************************/
BERR_Code BVDC_Source_SetSurface_isr
    ( BVDC_Source_Handle               hSource,
      const BAVC_Gfx_Picture          *pGfxPic);

/***************************************************************************
Summary:
    Query the source surface (set) which is currently fetched by hardware.

Description:
    This is the "_isr" version of BVDC_Source_GetSurface. It is used in
    interrupt handler or critical session.

    Refer to BVDC_Source_setSurface for usage.

Input:
    hSource - The source device to be modified.

Output:
    pGfxPic - The graphics surface (set) that is currently displayed by VDC.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get the currently displayed main surface.

See Also:
    BVDC_Source_GetSurface, BVDC_Source_SetSurface_isr
****************************************************************************/
BERR_Code BVDC_Source_GetSurface_isr
    ( BVDC_Source_Handle               hSource,
      BAVC_Gfx_Picture                *pGfxPic);

/***************************************************************************
Summary:
    Specify how the YCrCb 4:2:2 format is expanded to 4:4:4 format in the
    in the graphics source device specified by the source handle.

Description:
    This function currently only applies to the source type of graphics
    feeder. It would return BERR_INVALID_PARAMETER if hSource does not
    represent a graphics feeder.

    When a YCrCb 4:2:2 format is used as an input of graphics feeder, say
    BPXL_eY08_Cb8_Y18_Cr8, the pixels must first be expanded to 4:4:4 format
    before they are further processed. This function specifies how the format
    are expanded.

    Currently BVDC_ChromaExpansion_eLinearInterpolate is the only valid value
    of eChromaExpansion. It is also the default value.

    This setting will not be applied until a call to BVDC_ApplyChanges is
    made.

Input:
    hSource - The source device to modify.
    eChromaExpansion - Chroma expansion enum

Returns:
    BERR_SUCCESS - chroma expansion method is set successfully.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
    BVDC_Source_SetSurface
    BVDC_ApplyChanges
****************************************************************************/
BERR_Code BVDC_Source_SetChromaExpansion
    ( BVDC_Source_Handle               hSource,
      BVDC_ChromaExpansion             eChromaExpansion );

/***************************************************************************
Summary:
    Sets and enables color keying in the graphics source device specified by
    the source handle.

Description:
    This function currently only applies to the source type of graphics
    feeder. It would return BERR_INVALID_PARAMETER if hSource does not
    represent a graphics feeder.

    It is used to enable color keying and to specify some basic color keying
    parameters.

    When color key is not enabled, window alpha set by BVDC_Window_SetAlpha
    is multiplied onto pixel alpha, to generate a new pixel alpha for further
    processing.

    If color keying is enabled and a pixel passes the key test, the given
    ucKeyedAlpha, rather than the window alpha, will be multiplied onto the
    pixel alpha. If a pixel fails to pass the key test, the window alpha is
    still used, to be multiplied onto the pixel alpha. In both case, the
    per pixel product is used as the new pixel alpha in further processing.

    A pixel is considered passing the color key test if ALL of its COLOR
    components are in the range defined by the given min/max. The range
    includes the low and high edges. Before the key test, the given mask is
    applied to each color component.

    If a window's SrcBlendFactor and DstBlendFactor are all constants,
    either in the case that one is BVDC_BlendFactor_eConstantAlpha and the
    other is BVDC_BlendFactor_eOneMinusConstantAlpha, or in the case that
    one is BVDC_BlendFactor_eOne and the other is BVDC_BlendFactor_eZero,
    the pixel alpha input to the blending stage are totally ignored, hence
    color key operation has no contribution to the finally display.

    The default is to have color keying disabled.

    The notation AC2C1C0 in the parameter names represents a four components
    value with each component taking 8-bits. C2C1C0 could be RGB or YCrCb, and
    A is not used.

    This setting will not be applied until a call to BVDC_ApplyChanges is
    made.

Input:
    hSource - The source device to modify.
    ulMinAC2C1C0 - Minimum AC2C1C0 test value.
    ulMaxAC2C1C0 - Maximum AC2C1C0 test value.
    ulMasAC2C1C0 - Mask for AC2C1C0.
    ucKeyedAlpha - Alpha to use when test passes.

    TODO: ulMinAC2C1C0 is a 4 components type? define a global FourCC type?

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set and enabled color key

See Also:
    BVDC_Window_SetAlpha, BVDC_Window_SetBlendFactor,
    BVDC_Source_DisableColorKey, BVDC_ApplyChanges
****************************************************************************/
BERR_Code BVDC_Source_EnableColorKey
    ( BVDC_Source_Handle               hSource,
      uint32_t                         ulMinAC2C1C0,
      uint32_t                         ulMaxAC2C1C0,
      uint32_t                         ulMaskAC2C1C0,
      uint8_t                          ucKeyedAlpha );

/***************************************************************************
Summary:
    Disables color keying in the graphics source device specified by the
    source handle.

Description:
    This function currently only applies to the source type of graphics
    feeder. It would return BERR_INVALID_PARAMETER if hSource does not
    represent a graphics feeder.

    When color key is not enabled, window alpha set by BVDC_Window_SetAlpha
    is multiplied onto pixel alpha, to generate a new pixel alpha for
    further processing.

    The default state is to have color keying disabled.

    This setting will not be applied until a call to BVDC_ApplyChanges is
    made.

Input:
    hSource - The source device to modify.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully disabled color key.

See Also:
    BVDC_Window_SetAlpha, BVDC_Source_EnableColorKey, BVDC_ApplyChanges
****************************************************************************/
BERR_Code BVDC_Source_DisableColorKey
    ( BVDC_Source_Handle               hSource );

/***************************************************************************
Summary:
    Specifies what coefficient set to use for the horizontal scaler in
    the graphics source device specified by the source handle.

Description:
    This function currently only applies to the source type of graphics
    feeder. It would return BERR_INVALID_PARAMETER if hSource does not
    represent a graphics feeder.

    If the output scale rectangle width set by BVDC_Window_SetScalerOutput
    is not equal to the source clip rectangle width set by
    BVDC_Window_SetSrcClip the graphics feeder will horizontally scale
    the source clip rectangle to the width of the output scale rectangle.
    The filter coefficients set with this function is used during the
    scaling.

    The goal of the horizontal scaling is to perform aspect-ratio-
    conversion as well as the SD up-scaling to HD format. As up-scaling SD
    format to HD format, higher level software could use BGRC module to
    perform the vertical up-scaling part, and perform horizontal up-scaling
    in graphics feeder, in order to save memory and bandwidth.

    The scaling is not performed if the output scale rectangle width is
    equal to the source clip rectangle width. Other than programming the
    source clip width and output scale width, user can not manually enable
    nor disable the scaling filter.

    There are some filter coefficients sets pre-defined by BVDC, one of them
    could be choosen by this API finction with a BVDC_FilterCoeffs enum
    parameter. The default value is BVDC_FilterCoeffs_eBilinear.

    If BVDC_FilterCoeffs_eSharp is used, a specific sharp coefficient set can
    also be selected by calling BVDC_Window_SetCoefficientIndex.

    This setting will not be applied until a call to BVDC_ApplyChanges is
    made.

Input:
    hSource - The source device to modify.
    eCoeffs - Identify the coefficient set to use.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set horizontal scale coefficients.

See Also:
    BVDC_Window_SetSrcClip
    BVDC_Window_SetScalerOutput
    BVDC_Window_SetCoefficientIndex
    BVDC_ApplyChanges
****************************************************************************/
BERR_Code BVDC_Source_SetScaleCoeffs
    ( BVDC_Source_Handle               hSource,
      BVDC_FilterCoeffs                eHorzCoeffs,
      BVDC_FilterCoeffs                eVertCoeffs );


/***************************************************************************
Summary:
    Sets and enables gamma correction in the graphics source device specified
    by the source handle.

Description:
    This function currently only applies to the source type of graphics
    feeder. It would return BERR_INVALID_PARAMETER if hSource does not
    represent a graphics feeder.

    Gamma correction is to compensate the non-linear relation of the
    brightness seen on a CRT and the color components output from Hydra to
    the CRT. As enabled, graphics feeder loads the gamma (correction) table
    specified by pvGammaTable and uiNumEntries into the graphics feeder from
    memory, and uses the current pixel color component as index to look up
    its gamma corrected value from the table, right before the color matrix
    operation. Due to hardware limitation, if the graphics surface has a
    palette format, gamma correction can not be enabled. Otherwise,
    BVDC_ApplyChanges will return an error. If gamma correction is needed with
    a palette surface, the higher level software should combine the gamma
    correction into the entries of the palette table when it is built.

    Please notice that each color component is used independently to look up its
    gamma corrected value, and that the looking up is done after the pixel color
    is expanded into four 8-bits components. The looking up result is also 8-bits.

    The user provided gamma table buffer must have 256 uint32_t entries, and be
    allocated with BMMA_Alloc, otherwise the enabling will fail and error will be
    returned.

    If the graphics surface is type of RGB, the 256 entries should be arranged as
    the following:
             |  R3|  R2|  R1|  R0|
             |  R7|  R6|  R5|  R4|
             |  G3|  G2|  G1|  G0|
             |  G7|  G6|  G5|  G4|
             |  B3|  B2|  B1|  B0|
             |  B7|  B6|  B5|  B4|
             |  A3|  A2|  A1|  A0|
             |  A7|  A6|  A5|  A4|

             | R11| R10|  R9|  R8|
             | R15| R14| R13| R12|
             | G11| G10|  G9|  G8|
             | G15| G14| G13| G12|
             | B11| B10|  B9|  B8|
             | B15| B14| B13| B12|
             | A11| A10|  A9|  A8|
             | A15| A14| A13| A12|

             ... ...

             |R251|R250|R249|R248|
             |R255|R254|R253|R252|
             |G251|G250|G249|G248|
             |G255|G254|G253|G252|
             |B251|B250|B249|B248|
             |B255|B254|B253|B252|
             |A251|A250|A249|A248|
             |A255|A254|A253|A252|

    In the above table Ri is the gamma corrected R value according to original R
    value i. Gi and Bi are similar for G and B component.

    If the surface is type of YCrCb, the R, G, B in the above table are replaced
    with Y, Cb, Cr respectively.

    A is not gamma corrected, therefore Ai in the above table is not used. It is
    there only for place holder.

    By default gamma correction is disabled.

    This setting will not be applied until a call to BVDC_ApplyChanges is
    made.

Input:
    hSource - The source device to modify.
    uiNumEntries - Number of entries for the table. It must be 256.
    hGammaTable - Memory block that contains the entries. This is allocated
                  thru BMMA_Alloc and must be passed to
                  BVDC_Source_DisableGammaCorrection.

Output:

Returns:
    BERR_INVALID_PARAMETER - hSource is not a valid source handle.
    BERR_PALETTE_FORMAT - the source surface pixel format is a palette type.
    BERR_SUCCESS - Successfully enabled gamma correction.

See Also:
    BVDC_Source_SetSrc
    BVDC_Source_DisableGammaCorrection
    BVDC_ApplyChanges
****************************************************************************/
BERR_Code BVDC_Source_EnableGammaCorrection
    ( BVDC_Source_Handle               hSource,
      uint32_t                         ulNumEntries,
      const BMMA_Block_Handle          hGammaTable );

/***************************************************************************
Summary:
    Disables gamma correction in the graphics source device specified by the
    source handle.

Description:
    This function currently only applies to the source type of graphics
    feeder. It would return BERR_INVALID_PARAMETER if hSource does not
    represent a graphics feeder.

    By default gamma correction is disabled.

    This setting will not be applied until a call to BVDC_ApplyChanges is
    made.

Input:
    hSource - The source device to modify.
    hGammaTable - Memory block that contains the entries. Must be the
                  same block that was passed into
                  BVDC_Source_EnableGammaCorrection.

Output:

Returns:
    BERR_INVALID_PARAMETER - hSource is not a valid source handle.
    BERR_SUCCESS - Successfully disabled gamma correction

See Also:
    BVDC_Source_EnableGammaCorrection
    BVDC_ApplyChanges
****************************************************************************/
BERR_Code BVDC_Source_DisableGammaCorrection
    ( BVDC_Source_Handle               hSource,
      const BMMA_Block_Handle          hGammaTable );

/***************************************************************************
Summary:
    Sets the adjustment for SDR graphics to HDR conversion in the graphics
    source device specified by the source handle.

Description:
    This function currently only applies to the source type of graphics
    feeder. It would return BERR_INVALID_PARAMETER if hSource does not
    represent a graphics feeder.

    When a SDR graphics surface is displayed on a HDMI device with HDR EOTF
    the pixel values need to be converted from SDR EOTF to HDR. Depending
    on the room brightness and the video displayed at the time, the graphics
    might look too dark or too bright to user, or too less or too much bluish
    /reddish. This function allows user to adjust them to his/her preference.

    This setting will be in effect only if the graphics surface has SDR EOTF,
    and the HDMI display has HDR EOTF.

    Refer to BVDC_Source_SdrGfxToHdrApproximationAdjust for default settings.

    This setting will not be applied until a call to BVDC_ApplyChanges is
    made.

Input:
    hSource - The source device to modify.
    pSdrGfxToHdrApproxAdj - A reference to approximation adjust settings structure.

Output:

Returns:
    BERR_INVALID_PARAMETER - hSource is not a valid source handle.
    BERR_SUCCESS - Successfully enabled gamma correction.

See Also:
    BVDC_Source_SdrGfxToHdrApproximationAdjust
    BVDC_Display_HdmiSettings, BVDC_Display_SetHdmiSettings
****************************************************************************/
BERR_Code BVDC_Source_SetSdrGfxToHdrApproximationAdjust
    ( BVDC_Source_Handle               hSource,
      const BVDC_Source_SdrGfxToHdrApproximationAdjust *pSdrGfxToHdrApproxAdj);

/***************************************************************************
Summary:
    Sets constant color for surfaces with alpha-only pixel format in the
    graphics source device specified by the source handle.

Description:
    This function currently only applies to the source type of graphics
    feeder. It would return BERR_INVALID_PARAMETER if hSource does not
    represent a graphics feeder.

    The constant color set by this API function is used as the pixel color
    when the graphics source surface has an alpha-only format. The default
    constant color is black (i.e., R, G, B are all zero).

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hSource - The source device to modify.
    ucRed - The RED component of the constant color for alpha-only surface.
    ucGreen - The GREEN component of the constant color for alpha-only surface.
    ucBlue -  The BLUE component of the constant color for alpha-only surface.

Output:

Returns:
    BERR_INVALID_PARAMETER - hSource is not a valid source handle.
    BERR_SUCCESS - Successfully set constant color for the source.

See Also:
    BVDC_ApplyChanges
****************************************************************************/
BERR_Code BVDC_Source_SetConstantColor
    ( BVDC_Source_Handle               hSource,
      uint8_t                          ucRed,
      uint8_t                          ucGreen,
      uint8_t                          ucBlue );

/***************************************************************************
Summary:
    Set various customized DNR settings.

Description:
    This function allow application/driver to overwrite the interal default
    DNR settings.  Once it's specified PI will use these settings for all
    input/output settings.  Application/driver is responsible for the settings
    once it's overwrite.

    DNR only applies to mvd/xvd source.
    User can effectively bypass individual DNR filter for the source by setting
    eMnrMode, eBnrMode or eDcrMode to BVDC_FilterMode_eBypass.
    User can also completely disable DNR by setting all eMnrMode AND
    eBnrMode AND eDcrMode to BVDC_FilterMode_eDisable.

    Number of sources that can support DNR at same time are limited by hardware
    availability. DNR block may be shared between different sources when
    there is not enough DNR blocks available.

    Note:  A source has to be created AND applied before setting its DNR
    filters. All windows that use the source will be set in the same way.

    DNR is disabled by default.

    Does not take immediate effect. Requires an ApplyChanges() call.

    pvUserInfo in pDnrSettings is ignored for internal DNR.

Input:
    hSource      - A valid source handle created earlier.

    pDnrSettings - A structure that describes what settings to be
    overwrite by application/driver.  If pDnrSettings is NULL all default
    internal settings will be used.  Or selectively pDnrSettings->XXX
    is 0 to pick and chose which to use internal defaults.
    Default value for pDnrSettings NULL.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_GetDnrConfiguration
**************************************************************************/
BERR_Code BVDC_Source_SetDnrConfiguration
    ( BVDC_Source_Handle               hSource,
      const BVDC_Dnr_Settings         *pDnrSettings );

/***************************************************************************
Summary:
    Get the DNR configuration.

Description:
    This function get DNR configurations.

Input:
    hSource      - A valid source handle created earlier.

Output:
    pDnrSettings - A structure that describe the DNR settings.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_SetDnrConfiguration
**************************************************************************/
BERR_Code BVDC_Source_GetDnrConfiguration
    ( const BVDC_Source_Handle          hSource,
      BVDC_Dnr_Settings                *pDnrSettings );

/***************************************************************************
Summary:
    This function sets a source's split screen settings.

Description:
    Sets the split screen settings of a source.  These settings are used
    to disable or enable features on the specified split screen.

Input:
    hSource       - A valid source handle created earlier
    pSplitScreen  - A pointer to the split screen setting structure

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set split screen settings for the window.

See Also:
    BVDC_SplitScreenMode
    BVDC_SplitScreenSettings
    BVDC_Source_GetSplitScreenMode
**************************************************************************/
BERR_Code BVDC_Source_SetSplitScreenMode
    ( BVDC_Source_Handle                      hSource,
      const BVDC_Source_SplitScreenSettings  *pSplitScreen );

/***************************************************************************
Summary:
    This function gets a source's split screen settings.

Description:
    Gets the current split screen settings of a window.

Input:
    hSource   - A valid source handle created earlier

Output:
    pSplitScreen      - Source's current split screen setting

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned split screen settings for the window.

See Also:
    BVDC_SplitScreenMode
    BVDC_SplitScreenSettings
    BVDC_Window_SetSplitScreenMode
**************************************************************************/
BERR_Code BVDC_Source_GetSplitScreenMode
    ( const BVDC_Source_Handle          hSource,
      BVDC_Source_SplitScreenSettings  *pSplitScreen );


/***************************************************************************
Summary:
    Set various customized HD_DVI settings.

Description:
    This function allow application/driver to overwrite the interal default
    HD_DVi settings.  Once it's specified PI will use these settings for all
    input/output settings.  Application/driver is responsible for the settings
    once it's overwrite.

    bEnableDe in pHdDviSettings is ignored for internal HD_DVI.

Input:
    hSourceHdDvi - A valid source handle created earlier.

    pHdDviSettings - A structure that describes what settings to be
    overwrite by application/driver.  If pHdDviSettings is NULL all default
    internal settings will be be used.  Or selectively pHdDviSettings->XXX
    is 0 to pick and chose which to use internal defaults.
    Default value for pHdDviSettings NULL.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
BVDC_Source_GetHdDviConfiguration
**************************************************************************/
BERR_Code BVDC_Source_SetHdDviConfiguration
    ( BVDC_Source_Handle               hSourceHdDvi,
      const BVDC_HdDvi_Settings       *pHdDviSettings );



/***************************************************************************
Summary:
    Get the HD_DVI configuration.

Description:
    This function get HD_DVI configurations.

Input:
    hSourceHdDvi - A valid source handle created earlier.

Output:
    pHdDviSettings - A structure that describe the HD_DVI settings.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_SetHdDviConfiguration
**************************************************************************/
BERR_Code BVDC_Source_GetHdDviConfiguration
    ( const BVDC_Source_Handle         hSourceHdDvi,
      BVDC_HdDvi_Settings             *pHdDviSettings );

/***************************************************************************
Summary:
    Enable/disable the value of VDEC's H/V start position set by VDC.

Description:
    This function attempts to override the value of H/V start of VDEC current
    value.  It's meant for doing pc intput auto sync.  Note that this H/V
    START value override with this function will potential not yield the
    correct video output.  Especially on auto format detection where VDEC
    have detected a new format.  Hence, on a newly format detected it will
    reset to the new format's h/v start.  In addition it will notify
    application thru a callback so that new override h/v start can be set.

    The purpose of this function is for SW Work-around for auto position
    and auto size using VDEC's HV-Start & LBOX!  It moves the coordination of
    H/V start in VDEC and use LBOX to detect first H/V start of first non-black
    pixel as starting postion, and h/v size.

    Note: ulHStart=0, ulVStart=0 represent the first raster point.  Active
    video.

Input:
    hSourceVdec - A valid source handle created earlier.

    bOverrideVdc - This flag will indicate to override the interal value of
       H/V start of VDEC with ulHStart/ulVStart.

    ulHStart - Number of horizontal sample value for VDEC.

    ulVStart - Number of vertical line value for VDEC.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_SetHVStart, BVDC_Window_EnableBoxDetect.
**************************************************************************/
BERR_Code BVDC_Source_SetHVStart
    ( BVDC_Source_Handle               hSource,
      bool                             bOverrideVdc,
      uint32_t                         ulHStart,
      uint32_t                         ulVStart );


/***************************************************************************
Summary:
    Get the current values of H/V start for the current format.

Description:
    This function return the current H/V start of the vdec of the current
    detected format.  Note that in the case of auto format detection is
    turned on; applications need to re-get to make sure the value reflects
    the new detected format.

Input:
    hSource - A valid source handle created earlier.

    pbOverrideVdc - This flag will indicate to override the interal value of
       H/V start of VDEC with ulHStart/ulVStart.

    pulHStart - Number of horizontal sample value for VDEC.

    pulVStart - Number of vertical line value for VDEC.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_SetHVStart, BVDC_Window_EnableBoxDetect.
    BVDC_Source_GetDefaultHVStart
**************************************************************************/
BERR_Code BVDC_Source_GetHVStart
    ( BVDC_Source_Handle               hSource,
      bool                            *pbOverrideVdc,
      uint32_t                        *pulHStart,
      uint32_t                        *pulVStart );


/***************************************************************************
Summary:
    Get the default values of H/V start for the current format.

Description:
    This function return the default H/V start of the vdec of the current
    detected format.  Note that in the case of auto format detection is
    turned on; applications need to re-get to make sure the value reflects
    the new detected format.

Input:
    hSource - A valid source handle created earlier.

    pulHStart - Number of horizontal sample value for VDEC/HDDVI.

    pulVStart - Number of vertical line value for VDEC/HDDVI.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_SetHVStart, BVDC_Window_EnableBoxDetect.
    BVDC_Source_GetHVStart
**************************************************************************/
BERR_Code BVDC_Source_GetDefaultHVStart
    ( BVDC_Source_Handle               hSource,
      uint32_t                        *pulHStart,
      uint32_t                        *pulVStart );



/***************************************************************************
Summary:
    Enable/disable user to override the default orientation provided from
    XVD or FMT.

Description:
    This function attempts to override the default orientation from XVD for
    MPEG source, or default orientation from FMT for HDDVI source.

    The purpose of this function is to get 3D half resolution cases for
    normal 2D formats. The 2D format will contain both left and right eyes
    packed in an over/under or left/right orientation.

Input:
    hSource - A valid source handle created earlier. Only applies to MPEG
       or HDDVI source.

    bOverride - This flag will indicate to override the default orientation
       or not.

    eOrientation - Orientation provided by user. Note this has to match the
       content of the source.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_GetOrientation
**************************************************************************/
BERR_Code BVDC_Source_SetOrientation
    ( BVDC_Source_Handle               hSource,
      bool                             bOverride,
      BFMT_Orientation                 eOrientation );

/***************************************************************************
Summary:
    Get the current orientation for the source.

Description:
    This function returns the current orientation provided by user.

    User can override the default orientation from XVD for MPEG source,
    or default orientation from FMT for HDDVI source.

Input:
    hSource - A valid source handle created earlier. Only applies to MPEG
       or HDDVI source.

    pbOverride - Current state to override the default orientation
       or not.

    peOrientation - Current orientation provided by user.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_SetOrientation
**************************************************************************/
BERR_Code BVDC_Source_GetOrientation
    ( BVDC_Source_Handle               hSource,
      bool                            *pbOverride,
      BFMT_Orientation                *peOrientation );


/***************************************************************************
Summary:
    Get the HDDVI input format status.

Description:
    This function get input format status.  It returns the raw value detected
    by hardware.  Values such as hpolarity, vpolarity, av_height, av_width,
    and etc.

Input:
    hSource - A valid source handle created earlier.

Output:
    pInputStatus - A structure that describe the input format status.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_InstallCallback
**************************************************************************/
BERR_Code BVDC_Source_GetInputStatus
    ( const BVDC_Source_Handle         hSource,
      BVDC_Source_InputStatus         *pInputStatus );

/***************************************************************************
Summary:
    Sets the 3x5 color matrix for the source.

Description:
    This matrix is used to convert the incoming source color space into
    CbYCr color space.
    Note, setting this matrix will override the VDC's default one.

    The values used for the color matrix are equal to the values stored
    in the pl32_Matrix array right shifted by the ulShift value. In
    addition, negative values are converted to positive values before
    shifting and then turned back into negative values. For
    example, if a value in the matrix was 4 and the shift value was 2, then
    the final value is 1 (4>>2 = 1). If a value in the matrix was 7
    and the shift value was 3, then the final value would be 0.875.
    If a value in the matrix was -7 and the shift value was 3 then the
    final values is -0.875.

    The values within the color matrix are in the following order:

    [Y_out ]   [M[0]  M[1]  M[2]  M[3] [M[4] ]   [Y_in ]
    [Cb_out]   [M[5]  M[6]  M[7]  M[8] [M[9] ]   [Cb_in]
    [Cr_out] = [M[10] M[11] M[12] M[13][M[14]] * [Cr_in]
    [A_out ]   [0     0     0     1     0    ]   [A_in ]
    [  1   ]   [0     0     0     0     1    ]   [  1  ]

    {Y_in, Cb_in, Cr_in, A_in}     = Input pixel before apply M matrix.
    {Y_out, Cb_Out, Cr_out, A_out} = Output pixel after apply M matrix.

    Note: The last two rows of the above matrix are used to facilitate
    computation only. It is not implemented physically in hardware.
    Columns 0 to 2 are multiplicative coefficients. Columns 4 and 5 are
    additive coefficients.  Note that for video csc the current hw does not
    support alpha multiply.

    Note, with source auto format detection, the source input color
    space could dynamically change between SD and HD; if user override
    of the source color matrix is not updated in a prompt way, the
    color transition artifact could appear with dynamic source format
    change.

Input:
    hSource     - A valid source handle created earlier.
    bOverride   - Whether to override VDC internal matrix with the supplied
                  one; if false, pl32_Matrix and ulShift will be ignored,
                  and the default matrices inside VDC will take place;
    pl32_Matrix - An int32_t array that holds the 3x5 matrix coeffs.
    ulShift     - Matrix bit shift value; this parameter specifies the
                  number of fractional bits in the custom matrix coeffs;
                  its intention is to make API portable and hardware
                  independent; user should choose a much higher precision
                  than current hw precision to be future prove; VDC
                  will convert the user matrxi coeffs into final hw
                  settings and hide chip specific precision difference;

Output:
    None

Returns:
    BERR_SUCCESS - Color matrix was set.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
****************************************************************************/
BERR_Code BVDC_Source_SetColorMatrix
    ( BVDC_Source_Handle               hSource,
      bool                             bOverride,
      const int32_t                    pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                         ulShift );

/***************************************************************************
Summary:
    Gets the 3x5 color matrix for the source.

Description:
    This matrix is used to convert the incoming source color space into
    CbYCr color space.  This fuction will get the current matrix in used.
    Either internal by VDC's or by user specified with bOverided.

    The values used for the color matrix are equal to the values stored
    in the pl32_Matrix array right shifted by the ulShift value. In
    addition, negative values are converted to positive values before
    shifting and then turned back into negative values. For
    example, if a value in the matrix was 4 and the shift value was 2, then
    the final value is 1 (4>>2 = 1). If a value in the matrix was 7
    and the shift value was 3, then the final value would be 0.875.
    If a value in the matrix was -7 and the shift value was 3 then the
    final values is -0.875.

    The values within the color matrix are in the following order:

    [Y_out ]   [M[0]  M[1]  M[2]  M[3] [M[4] ]   [Y_in ]
    [Cb_out]   [M[5]  M[6]  M[7]  M[8] [M[9] ]   [Cb_in]
    [Cr_out] = [M[10] M[11] M[12] M[13][M[14]] * [Cr_in]
    [A_out ]   [0     0     0     1     0    ]   [A_in ]
    [  1   ]   [0     0     0     0     1    ]   [  1  ]

    {Y_in, Cb_in, Cr_in, A_in}     = Input pixel before apply M matrix.
    {Y_out, Cb_Out, Cr_out, A_out} = Output pixel after apply M matrix.

    Note: The last two rows of the above matrix are used to facilitate
    computation only. It is not implemented physically in hardware.
    Columns 0 to 2 are multiplicative coefficients. Columns 4 and 5 are
    additive coefficients.  Note that for video csc the current hw does not
    support alpha multiply.

Input:
    hSource     - A valid source handle created earlier.

Output:
    pbOverride  - Whether the matrix was  override VDC internal matrix with
    the supplied one; if false, pl32_Matrix and ulShift will be ignored,
    and the default matrices inside VDC will take place;

    pl32_Matrix - An int32_t array that holds the 3x5 matrix coeffs.

    pulShift    - Matrix bit shift value; this parameter specifies the
    number of fractional bits in the custom matrix coeffs; its intention is to
    make API portable and hardware independent; user should choose a much
    higher precision than current hw precision to be future prove; VDC
    will convert the user matrxi coeffs into final hw settings and hide chip
    specific precision difference;

    if *pbOverride is true it will return the shift value previous set
    by user otherwise it will return shift value of 16; or another word the
    matrix element is in the form of S15.16 2's complement.

Returns:
    BERR_SUCCESS - Color matrix was set.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
****************************************************************************/
BERR_Code BVDC_Source_GetColorMatrix
    ( BVDC_Source_Handle               hSource,
      bool                            *pbOverride,
      int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                        *pulShift );

/***************************************************************************
Summary:
    Gets the 3x5 color matrix for the vdec source frontend.

Description:
    This matrix is used to convert the incoming source color space into
    CbYCr color space.  This fuction will get the current matrix in used.
    Either internal by VDC's or by user specified with bOverided.

    The values used for the color matrix are equal to the values stored
    in the pl32_Matrix array right shifted by the ulShift value. In
    addition, negative values are converted to positive values before
    shifting and then turned back into negative values. For
    example, if a value in the matrix was 4 and the shift value was 2, then
    the final value is 1 (4>>2 = 1). If a value in the matrix was 7
    and the shift value was 3, then the final value would be 0.875.
    If a value in the matrix was -7 and the shift value was 3 then the
    final values is -0.875.

    The values within the color matrix are in the following order:

    [Y_out ]   [M[0]  M[1]  M[2]  M[3] [M[4] ]   [Y_in ]
    [Cb_out]   [M[5]  M[6]  M[7]  M[8] [M[9] ]   [Cb_in]
    [Cr_out] = [M[10] M[11] M[12] M[13][M[14]] * [Cr_in]
    [A_out ]   [0     0     0     1     0    ]   [A_in ]
    [  1   ]   [0     0     0     0     1    ]   [  1  ]

    {Y_in, Cb_in, Cr_in, A_in}     = Input pixel before apply M matrix.
    {Y_out, Cb_Out, Cr_out, A_out} = Output pixel after apply M matrix.

    Note: The last two rows of the above matrix are used to facilitate
    computation only. It is not implemented physically in hardware.
    Columns 0 to 2 are multiplicative coefficients. Columns 4 and 5 are
    additive coefficients.  Note that for video csc the current hw does not
    support alpha multiply.

Input:
    hSource     - A valid source handle created earlier.

Output:
    pbOverride  - Whether the matrix was  override VDC internal matrix with
    the supplied one; if false, pl32_Matrix and ulShift will be ignored,
    and the default matrices inside VDC will take place;

    pl32_Matrix - An int32_t array that holds the 3x5 matrix coeffs.

    pulShift    - Matrix bit shift value; this parameter specifies the
    number of fractional bits in the custom matrix coeffs; its intention is to
    make API portable and hardware independent; user should choose a much
    higher precision than current hw precision to be future prove; VDC
    will convert the user matrxi coeffs into final hw settings and hide chip
    specific precision difference;

    if *pbOverride is true it will return the shift value previous set
    by user otherwise it will return shift value of 16; or another word the
    matrix element is in the form of S15.16 2's complement.

Returns:
    BERR_SUCCESS - Color matrix was set.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
****************************************************************************/
BERR_Code BVDC_Source_GetFrontendColorMatrix
    ( BVDC_Source_Handle               hSource,
      bool                            *pbOverride,
      int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                        *pulShift );

/***************************************************************************
Summary:
    Enable/disable the Progressive Segmented Frame (PsF) mode.

Description:
    By "Progressive Frames" we mean that the content is progressive in nature,
    specifically 1080p @ 24/25/30Hz. By "Segmented" we mean that the frames are
    actually encoded as interlaced content.

    The purpose of this function is to support 1080p24/25/30 sync-locked source
    with frame scanout and output to 50/60 Hz display.

    Note:
    To make it work, app needs to config decoder side properly to override
    the Correct Display to allow 1080p frames passed to VDC in the picture data
    ready callback. PsF will force capture the MPEG video. If the sync-locked
    display is interlaced, PsF may allocate 1080p buffer to support 1080i output
    and 480p buffer for 480i output. So user should be aware of the larger buffer
    allocation implied by this feature.

    This API only applies to MFD source.

Input:
    hSourceMfd - A valid source handle created earlier.

    bEnable    - This flag will enable/disable the PsF mode.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_GetPsfMode.
**************************************************************************/
BERR_Code BVDC_Source_SetPsfMode
    ( BVDC_Source_Handle               hSourceMfd,
      bool                             bEnable );


/***************************************************************************
Summary:
    Get the current PsF mode.

Description:
    This function return the current PsF mode for this source.

Input:
    hSource  - A valid source handle created earlier.

    pbEnable - This flag will indicate the PsF mode on/off setting.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_SetPsfMode.
**************************************************************************/
BERR_Code BVDC_Source_GetPsfMode
    ( BVDC_Source_Handle               hSource,
      bool                            *pbEnable );

/***************************************************************************
Summary:
    Enable/disable force frame capture mode.

Description:
    The purpose of this function is to force VDC to capture frame for raaga
    encoder when the BVN is driven by an interlaced display such as 480i@60.

    The repeated frames are only captured once in this mode.

    Note:
    To make it work, app needs to config decoder side properly to override
    the Correct Display to allow frames passed to VDC in the picture data
    ready callback. ForceFrameCapture will first force capture the MPEG video. If
    the sync-locked display is interlaced, it may allocate 1080p buffer to support
    1080i output and 480p buffer for 480i output. So user should be aware of the
    larger buffer allocation implied by this feature.

    This API only applies to MFD source (including gfx surface to MFD).

Input:
    hSourceMfd - A valid source handle created earlier.

    bForce    - This flag will enable/disable the ForceFrameCapture mode.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_GetPsfMode.
**************************************************************************/
BERR_Code BVDC_Source_ForceFrameCapture
    ( BVDC_Source_Handle               hSourceMfd,
      bool                             bForce );


/***************************************************************************
Summary:
    This function sets source resume mode.

Description:
    This function sets how the source's down stream processing and display
    are started/resumed after the source is first time used by a window, or
    after a dynamic video format change is detected.

    When the source is first connected to a window, or when the source is
    dynamically switched to a new format (such as from 480i to 1080i), the
    current BVDC configuration, such as setting to down stream modules and
    RTS (real time schedule), might not match the new source format. The
    mismatch could potentially cause memory allocation and bandwidth failure,
    and/or hang the display system.

    Even if the new source format is successfully displayed with the current
    BVDC configuration, user might want to change some setting according to
    the new source format. That would cause an annoying display glitch.

    Therefore it is recommended to mute the down stream processing and display
    until the user's modification to the out-dated configuration in BVDC are
    completed.

    To do so, this function should be called with the value of eResumeMode
    as BVDC_ResumeMode_eManual. And a user provided callback function should
    be registered to BVDC with BVDC_Source_InstallCallback in order to get
    informed when a dynamic source format change is detected by BVDC.

    With BVDC_ResumeMode_eManual mode, BVDC will automatically mute all
    windows that input from this source when a new source format is detected.
    In this situation, BVDC will not immediately allocate resource according
    to the new source format change, to avoid resource shortage caused by
    potential configuration mismatch.

    As the callback function is called, the dirty bit bSrcPending in the
    passed BVDC_Source_CallbackData indicates that the calling-back is due
    to a new source format detecting. In this case, the info should be passed
    to the user mode app thread. Then BVDC configuration should be modified
    accordingly, and finally BVDC_Source_Resume should be called to resume the
    source's down stream processing and display, in the app thread.

    If the source is a type of MVD/XVD, the callback might not pass enough
    information that is needed to re-configure BVDC. Those info could be
    found in the BAVC_MVD/XVD_Picture struct passed from MVD/XVD.

    The default value of eResumeMode is BVDC_ResumeMode_eAuto, in order
    to maintain back compatibility.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hSource - A valid source handle created earlier.
    eResumeMode - How the display of the source is started.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the video source format of a window.

See Also:
    BVDC_Source_InstallCallback, BVDC_Source_Resume,
    BVDC_Source_SetResumeMode
**************************************************************************/
BERR_Code BVDC_Source_SetResumeMode
    ( BVDC_Source_Handle               hSource,
      BVDC_ResumeMode                  eResumeMode );

/***************************************************************************
Summary:
    Get the source's resume mode.

Description:
    This function gets the source's resume mode.

Input:
    hSource - A valid source handle created earlier.

Output:
    peResumeMode - point to the current resume mode.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_SetResumeMode
**************************************************************************/
BERR_Code BVDC_Source_GetResumeMode
    ( const BVDC_Source_Handle         hSource,
      BVDC_ResumeMode                 *peResumeMode );

/***************************************************************************
Summary:
    This function instructs BVDC to resume the source.

Description:
    This function is needed only if BVDC_ResumeMode_eManual is set to the
    source.

    In this case, each window that inputs from this source is automatically
    muted after the source is first time connected to the window, or after
    a dynamic source format is detected by VDC. This function is then needed
    to instruct BVDC to resume processing and display for all windows that
    input from this source after the corresponding modification to BVDC
    configuration has been completed.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hSource - A valid source handle created earlier.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully informed BVDC to start display.

See Also:
    BVDC_Source_SetResumeMode, BVDC_Source_InstallCallback
**************************************************************************/
BERR_Code BVDC_Source_Resume
    ( BVDC_Source_Handle               hSource );


/***************************************************************************
Summary:
    This function instructs BVDC to suspend the source.

Description:
    This function is needed only if BVDC_ResumeMode_eManual is set to the
    source.

    In this case, each window that inputs from this source is automatically
    muted after the source is first time connected to the window, or after
    a dynamic source format is detected by VDC. This function is then needed
    to instruct BVDC to force source pending of current processing and display.

    This effectively forces an artificial source change, so that all
    connected windows will go into pending, and required user to call
    BVDC_Source_Resume() after new configurations is set.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hSource - A valid source handle created earlier.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully informed BVDC to start display.

See Also:
    BVDC_Source_SetResumeMode, BVDC_Source_InstallCallback,
    BVDC_Source_Resume
**************************************************************************/
BERR_Code BVDC_Source_ForcePending
    ( BVDC_Source_Handle               hSource );



/***************************************************************************
Summary:
    Get the source's capabilities.

Description:
    This function gets the source's capabilities.

Input:
    hSource - A valid source handle created earlier.

Output:
    pCapabilities - point to the source capabilities structure.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_Capabilities
**************************************************************************/
BERR_Code BVDC_Source_GetCapabilities
    ( const BVDC_Source_Handle         hSource,
      BVDC_Source_Capabilities        *pCapabilities );

/***************************************************************************
Summary:
    Set the MPEG source's CRC type.

Description:
    This function sets the MPEG source's src type. Not supported for
    other type of source.

Input:
    hSource - A valid source handle created earlier.
    eCrcType - Crc type to set.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_GetCrcType
**************************************************************************/
BERR_Code BVDC_Source_SetCrcType
    ( const BVDC_Source_Handle         hSource,
      BVDC_Source_CrcType              eCrcType );

/***************************************************************************
Summary:
    Get the MPEG source's CRC Type.

Description:
    This function gets the MPEG source's CRC type.

Input:
    hSource - A valid source handle created earlier.

Output:
    peCrcType - point to the current crc type.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Source_SetCrcType
**************************************************************************/
BERR_Code BVDC_Source_GetCrcType
    ( const BVDC_Source_Handle         hSource,
      BVDC_Source_CrcType             *peCrcType );

/***************************************************************************
 * Display BackEnd
 **************************************************************************/

/***************************************************************************
Summary:
    This function gets display default setting structure.

Description:
    BVDC_Display_Settings inherent default setting structure could be
    queried by this API function prior to BVDC_Display_Create, modified and
    then passed to BVDC_Display_Create.  This save application tedious
    work of filling in the configuration structure.

Input:
    eDisplayId - Default settings for specific display.

Output:
    pDefSettings - A reference to default settings structure.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully get BVDC_Display_Settings default settings.

See Also:
    BVDC_Display_Create, BVDC_Display_Settings.
**************************************************************************/
BERR_Code BVDC_Display_GetDefaultSettings
    ( BVDC_DisplayId                   eDisplayId,
      BVDC_Display_Settings           *pDefSettings );

/***************************************************************************
Summary:
    This function creates a display handle

Description:
    Creates a display handle associated with a valid Compositor
    handle. BVDC_Open must be called prior to this routine. Returns an error
    if the selection or parmaters are invalid.

    Does not take immediate effect.  Requires an ApplyChanges() call.

Input:
    hCompositor - Compositor handle

    eDisplayId - Display to create; note that the display Id has implied
                 meaning of vbi capability.

Output:
    phDisplay - pointer to display handle

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_Destroy
**************************************************************************/
BERR_Code BVDC_Display_Create
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_Display_Handle             *phDisplay,
      BVDC_DisplayId                   eDisplayId,
      const BVDC_Display_Settings     *pDefSettings );

/***************************************************************************
Summary:
    This function destroys a display handle

Description:
    This function destroy the display handle.

    Before calling this function, upper layer needs to ensure that MPAA
    decimation is not currently enabled with this display.

    Does not take immediate effect.  Requires an ApplyChanges() call.

Input:
    hDisplay - Display handle created earlier.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_Create
**************************************************************************/
BERR_Code BVDC_Display_Destroy
    ( BVDC_Display_Handle              hDisplay );

/***************************************************************************
Summary:
    This function gets the display Id associated with the display handle

Description:
    Returns the display Id for the display handle.

    BVDC_DisplayId_eDisplay0 - Specify that the specific display uses
    VEC_0 (aka Primary Vec) hardware.

    BVDC_DisplayId_eDisplay1 - Specify that the specific display uses
    VEC_1 (aka Secondary Vec) hardware.

    BVDC_DisplayId_eDisplay2 - Specify that the specific display uses
    VEC_2 (aka Teteriary Vec) hardware or bypass display.

Input:
    hDisplay - Display handle created earlier.

Output:
    peDisplayId - pointer to the display Id

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
**************************************************************************/
BERR_Code BVDC_Display_GetDisplayId
    ( const BVDC_Display_Handle        hDisplay,
      BVDC_DisplayId                  *peDisplayId );

/***************************************************************************
Summary:
    This function gets the VBI path associated with the display handle

Description:
    Returns the VBI path for the display handle. Users must call this
    function, to get the known VBI path, and pass that information to the
    appropriate VBI PI API.  It will return the VBI encode path,
    on a display that has a bypass only with 656 output it will return
    BAVC_VbiPath_eBypassX.

Input:
    hDisplay - Display handle created earlier.

Output:
    peVbiPath - pointer to VBI path

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
**************************************************************************/
BERR_Code BVDC_Display_GetVbiPath
    ( const BVDC_Display_Handle        hDisplay,
      BAVC_VbiPath                    *peVbiPath );

/***************************************************************************
Summary:
    This function gets the Analog VBI path associated with the display
    handle.

Description:
    Returns the VBI path for the display handle. Users must call this
    function, to get the known VBI path, and pass that information to the
    appropriate VBI PI API.  It will return the (analog) VBI encode path,
    on a display that has a bypass only with 656 output it will return
    BAVC_VbiPath_eUnknown.

    This similiar to BVDC_Display_GetVbiPath, except it does not return
    vbi encoder path for bypass.

Input:
    hDisplay - Display handle created earlier with BVDC_Display_Create.

Output:
    peVbiPath - pointer to VBI path

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
**************************************************************************/
BERR_Code BVDC_Display_GetAnalogVbiPath
    ( const BVDC_Display_Handle        hDisplay,
      BAVC_VbiPath                    *peVbiPath );

/***************************************************************************
Summary:
    This function sets the video format for a specific display.

Description:
    Sets the output video format associated with a display handle. Returns an
    error if the video format is invalid.

    Corresponding to the display video format change, the window destination
    rectangle and display aspect ratio should typically be reset together by
    the application program, unless the value doesn't change.

    The display canvas size and typical aspect ratio corresponding to a video
    format could be found from the BFMT_VideoInfo structure queried with
    BFMT_GetVideoFormatInfoPtr.

    Unlike source aspect ratio, display aspect ratio is NOT automatically
    updated according to display format change.

    7038 supports 6 Dacs, which are shared between the two Input Timing
    blocks. Setting a display's video format to BFMT_VideoFmt_eDisableCRT
    will turn off the CRT and disable all Dacs associated with this display.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hDisplay - Display handle created earlier.
    eVideoFormat - Video format

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetVideoFormat,
    BVDC_Window_SetDstRect, BVDC_Display_SetAspectRatio
**************************************************************************/
BERR_Code BVDC_Display_SetVideoFormat
    ( BVDC_Display_Handle              hDisplay,
      BFMT_VideoFmt                    eVideoFormat );

/***************************************************************************
Summary:
    This function returns the video format.

Description:
    Returns the current video format (HW-applied)
    associated with the display handle.

Input:
    hDisplay - Display handle created earlier.

Output:
    peDispFormat - display format

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetDisplayConfiguration
**************************************************************************/
BERR_Code BVDC_Display_GetVideoFormat
    ( const BVDC_Display_Handle        hDisplay,
      BFMT_VideoFmt                   *peDispFormat );


/***************************************************************************
Summary:
    This function set the custom format timing that is not yet define in
    BFMT_VideoFmt.

Description:
    This function allows application to load different format timing information
    into the BFMT_VideoFmt_eCustom0/1/2.  This is so that
    with newly format timing become available one does not need to recompile
    to support this new format timing.  This is to allow application such as
    Kylin DTV App to support new panel that haven't been support before at
    runtime.

    This API can also set STG output custom format for video encoder usage.

    To support new format timing also require system integrator to be aware
    of RTS (real time schedule) for memory clients requirement must also be
    met.  Failure to do so will result in undefined system behavior.

Input:
    hDisplay - Video format information handle that was previously opened by
    BFMT_Open.

    pCustomFmt - Contain the video information about the custom format.  Which
    include the standard active video width/height/etc, and in addition it
    also include the custom microcodes and rate manager settings for DTV usage.
    For STG custom format, the following BFMT_VideoInfo data needs to be set properly:
        eVideoFmt:                     eCustom0/1/2
        ulWidth  :                     hsize of STG output
        ulHeight :                     vsize of STG output
        ulDigitalWidth :               same as ulWidth
        ulDigitalHeight:               same as ulHeight
        ulScanWidth    :               same as ulWidth
        ulScanHeight   :               same as ulHeight
        ulVertFreq     :               vertical refresh rate in 1/100th Hz
        bInterlaced    :               interlaced or not
        eAspectRatio   :               display aspect ratio (DAR)
        ulPxlFreq      :               average pixel rate = ulWidth * ulHeight *
                                           ulVertFreq / 1000000
        *pchFormatStr  :               format name
        *pCustomInfo   :               NULL
    All others are don't care for STG output.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set new format timing information.

See Also:
    BVDC_Display_SetVideoFormat.
**************************************************************************/
BERR_Code BVDC_Display_SetCustomVideoFormat
    ( BVDC_Display_Handle              hDisplay,
      const BFMT_VideoInfo            *pCustomFmt );

/***************************************************************************
Summary:
    This function set the custom format timing that is not yet define in
    BFMT_VideoFmt.

Description:
    This function allows application to load different format timing information
    into the BFMT_VideoFmt_eCustom0/BFMT_VideoFmt_eCustom1.  This is so that
    with newly format timing become available one does not need to recompile
    to support this new format timing.  This is to allow application such as
    Kylin DTV App to support new panel that haven't been support before at
    runtime.

    To support new format timing also require system integrator to be aware
    of RTS (real time schedule) for memory clients requirement must also be
    met.  Failure to do so will result in undefined system behavior.

Input:
    hDisplay - Video format information handle that was previously opened by
    BFMT_Open.

Returns:
    (const BFMT_VideoInfo*) the format information timing.

See Also:
    BVDC_Display_SetCustomVideoFormat.
**************************************************************************/
const BFMT_VideoInfo* BVDC_Display_GetCustomVideoFormat
    ( BVDC_Display_Handle              hDisplay );

/***************************************************************************
Summary:
    Specify the display aspect ratio.

Description:
    This API function sets the aspect ratio enum for the display monitor. It
    is used when aspect ratio correction is performed by BVDC. It is called
    typically when the display video format is re-set.

    A display typically has aspect ratio 4:3 or 16:9 if its video format is
    SD type or HD type respectively. The typical aspect ratio corresponding
    to a video format could be found from the BFMT_VideoInfo structure
    queried with BFMT_GetVideoFormatInfoPtr.

    There are some TV monitor that could internally scale SD video signal
    up horizontally to display as wide screen mode. In this case its display
    aspect ratio is 16:9 even if the display video format is SD type. There
    exist also some TV monitor that could display HD signal as aspect ratio
    4:3.

    The default value is the "typical aspect ratio" corresponding to the
    default display video format when BVDC was openned.

    Unlike source aspect ratio, display aspect ratio is NOT automatically
    updated according to display format change.

    If BFMT_AspectRatio_eSAR is meant to be passed for eAspectRatio, please
    use BVDC_Display_SetSampleAspectRatio. It is an extenstion of this function.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hDisplay - Display handle created earlier.
    eAspectRatio - Desired aspect ratio.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
See Also:
    BVDC_Display_GetAspectRatio
    BVDC_Display_SetSampleAspectRatio
    BVDC_Window_SetAspectRatioMode, BVDC_Display_SetAspectRatioCanvasClip
    BVDC_Window_SetNonLinearScl
**************************************************************************/
BERR_Code BVDC_Display_SetAspectRatio
    ( BVDC_Display_Handle              hDisplay,
      BFMT_AspectRatio                 eAspectRatio );

/***************************************************************************
Summary:
    Get the display output aspect ratio.

Description:
    This function gets the current display aspect ratio enum.

Input:
    hDisplay - Display handle created earlier.

Output:
    peAspectRatio - The current display aspect ratio.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
See Also:
    BVDC_Display_SetAspectRatio
    BVDC_Window_GtAspectRatioMode
    BVDC_Window_GetHorizScale
**************************************************************************/
BERR_Code BVDC_Display_GetAspectRatio
    ( const BVDC_Display_Handle        hDisplay,
      BFMT_AspectRatio                *peAspectRatio );

/***************************************************************************
Summary:
    Specify the display aspect ratio as BFMT_AspectRatio_eSAR.

Description:
    This API function is a extension to BVDC_Display_SetAspectRatio.

    It sets display aspect ratio to BFMT_AspectRatio_eSAR and sets the
    corresponding iSampleAspectRatioX and iSampleAspectRatioY. It is used
    when aspect ratio correction is performed by BVDC. It is called
    typically when the display video format is re-set.

    Please refer to BVDC_Display_SetAspectRatio for detail.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hDisplay - Display handle created earlier.
    uiSampleAspectRatioX - display pixel sample width.
    uiSampleAspectRatioY - display pixel sample height.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
See Also:
    BVDC_Display_GetSampleAspectRatio
    BVDC_Display_SetAspectRatio
    BVDC_Window_SetAspectRatioMode, BVDC_Display_SetAspectRatioCanvasClip
    BVDC_Window_SetNonLinearScl
**************************************************************************/
BERR_Code BVDC_Display_SetSampleAspectRatio
    ( BVDC_Display_Handle              hDisplay,
      uint16_t                         uiSampleAspectRatioX,
      uint16_t                         uiSampleAspectRatioY );

/***************************************************************************
Summary:
    Get the display pixel sample width and height

Description:
    This function gets the current display pixel sample width and height. The
    returned value is valid only if the current display aspect ratio is set
    by BVDC_Display_SetSampleAspectRatio. Otherwise the returned value is un-
    defined.

Input:
    hDisplay - Display handle created earlier.

Output:
    puiSampleAspectRatioX - The current display pixel sample width
    puiSampleAspectRatioY - The current display pixel sample height

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
See Also:
    BVDC_Display_SetSampleAspectRatio
    BVDC_Display_GetAspectRatio
    BVDC_Window_GtAspectRatioMode
    BVDC_Window_GetHorizScale
**************************************************************************/
BERR_Code BVDC_Display_GetSampleAspectRatio
    ( const BVDC_Display_Handle        hDisplay,
      uint16_t                        *puiSampleAspectRatioX,
      uint16_t                        *puiSampleAspectRatioY );

/***************************************************************************
Summary:
    This function sets the display aspect ratio canvas clip.

Description:
    By default, the full display aspect ratio is considered to be contributed
    by the full display canvas, and based on that the display pixel aspect
    ratio is calculated. However, with some display device it is a sub-
    rectangle area (inside the display canvas) that make up the full display
    aspect ratio, and the remaining surrounding edge is not visible. This API
    function allows user to specify the sub-rectangle by describing the
    numbers of pixels to clip from the left, right, top, and bottom of the
    full display canvas.

    Does not take immediate effect. Requires an ApplyChanges() call.

    If the the specified clipping row or column number is bigger than half
    of the full display size, it will causes the following ApplyChanges to
    fail and to return an error.

Input:
    hDisplay - A valid display handle created earlier.
    ulLeft - The number of columns to clip from the left of display canvas.
    ulRight - The number of columns to clip from the right of display canvas.
    ulTop - The number of rows to clip from the top of display canvas.
    ulBottom - The number of rows to clip from the bottom of display canvas.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the display aspect ratio canvas clip.

See Also:
    BVDC_Display_SetAspectRatio, BVDC_Window_SetAspectRatioMode,
    BVDC_Source_SetAspectRatioCanvasClip.
**************************************************************************/
BERR_Code BVDC_Display_SetAspectRatioCanvasClip
    ( BVDC_Display_Handle              hDisplay,
      uint32_t                         ulLeft,
      uint32_t                         ulRight,
      uint32_t                         ulTop,
      uint32_t                         ulBottom );

/***************************************************************************
Summary:
    This function configures the Dac settings

Description:
    Enables or disables the Dacs for the HD or SD output, associated
    with a Display handle. Returns an error if the selection is invalid.

    Does not take immediate effect. Requires an ApplyChanges() call.  To save
    power application/driver can set all the DACs to BVDC_DacOutput_eUnused.
    If the DAC does is not used it will get shutoff.

Input:
    hDisplay   - Display handle created earlier.
    ulDacs     - Dac(s) selected.  This is a bit position define with
                 BVDC_Dac_0, BVDC_Dac_1, ..., DVDC_Dac_5.
    eDacOutput - Dac output

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetDacConfiguration
**************************************************************************/
BERR_Code  BVDC_Display_SetDacConfiguration
    ( BVDC_Display_Handle              hDisplay,
      uint32_t                         ulDacs,
      BVDC_DacOutput                   eDacOutput );

/***************************************************************************
Summary:
    This function queries the Dac output setting

Description:
    Returns the Dac output setting for a specific Dac. Users will need to
    call this function for each Dac output setting.

Input:
    hDisplay - Display handle created earlier.
    ulDac    - Dac selected.  This is a bit position define with
               BVDC_Dac_0, BVDC_Dac_1, ..., DVDC_Dac_5.  Can only query
               one dac at a time.

Output:
    peDacOutput - pointer to Dac output

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetDacConfiguration
**************************************************************************/
BERR_Code BVDC_Display_GetDacConfiguration
    ( const BVDC_Display_Handle        hDisplay,
      uint32_t                         ulDac,
      BVDC_DacOutput                  *peDacOutput );

/***************************************************************************
Summary:
    This function setups the Hdmi output

Description:
    Enables or disables Hdmi output associated with a Display
    handle. Returns an error if the selection is invalid.  This function
    requires that all other display(s) must already shut off their HDMI output
    (e.g. BAVC_MatrixCoefficients_eUnknown) and applied.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hDisplay    - Display handle created earlier.
    ulHdmi      - Which HDMI (For 7038, BVDC_Hdmi_0 is the only valid choice)
    eHdmiOutput - HDMI output color space standards
                  a) BAVC_MatrixCoefficients_eItu_R_BT_709
                  b) BAVC_MatrixCoefficients_eFCC
                  c) BAVC_MatrixCoefficients_eSmpte_170M
                  d) BAVC_MatrixCoefficients_eSmpte_240M
                  e) BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG
                  f) BAVC_MatrixCoefficients_eXvYCC_709
                  g) BAVC_MatrixCoefficients_eXvYCC_601
                  h) BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL,
                  i) BAVC_MatrixCoefficients_eItu_R_BT_2020_CL,

                  j) BAVC_MatrixCoefficients_eUnknown      - HDMI output off;

    Note that the last type is used to disable HDMI output. Other enums values
    should match the display format set to this hDisplay. For eaxmple, NTSC
    should be combined with BAVC_MatrixCoefficients_eSmpte_170M or
    BAVC_MatrixCoefficients_eXvYCC_601. BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG
    and BAVC_MatrixCoefficients_eXvYCC_601 should be set with PAL. HD should
    be set together with BAVC_MatrixCoefficients_eItu_R_BT_709 or
    BAVC_MatrixCoefficients_eXvYCC_709. UHD should be together with
    BAVC_MatrixCoefficients_eItu_R_BT_709, BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL,
    or BAVC_MatrixCoefficients_eItu_R_BT_2020_CL.

    BAVC_MatrixCoefficients_eHdmi_RGB, BAVC_MatrixCoefficients_eDvi_Full_Range_RGB,
    and BAVC_MatrixCoefficients_eHdmi_Full_Range_YCbCr are to be obsolete. Use
    BAVC_Colorspace and BAVC_ColorRange with BVDC_Display_SetHdmiSettings instead.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetHdmiConfiguration, BVDC_Display_SetVideoFormat.
**************************************************************************/
BERR_Code BVDC_Display_SetHdmiConfiguration
    ( BVDC_Display_Handle              hDisplay,
      uint32_t                         ulHdmi,
      BAVC_MatrixCoefficients          eHdmiOutput );

/***************************************************************************
Summary:
    This function queries the Hdmi output settings

Description:
    Returns Hdmi output color space interpretation.
    Note:  If the output is BAVC_MatrixCoefficients_eUnknown, it means hdmi
    output is disabled.

Input:
    hDisplay - Display handle created earlier.
    ulHdmi   - Which HDMI (For 7038, BVDC_Hdmi_0 is the only valid selection)

Output:
    peHdmiOutput - pointer to Hdmi output color space

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetHdmiConfiguration
**************************************************************************/
BERR_Code BVDC_Display_GetHdmiConfiguration
    ( const BVDC_Display_Handle        hDisplay,
      uint32_t                         ulHdmi,
      BAVC_MatrixCoefficients         *peHdmiOutput );

/***************************************************************************
Summary:
    This function sets the display's HDMI settings

Description:
    This function will set the display's HDMI settings.

Input:
    hDisplay        - Display handle created earlier.
    pHdmiSettings   - A reference to hdmi settings structure.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetHdmiSettings, BVDC_Display_HdmiSettings.
**************************************************************************/
BERR_Code BVDC_Display_SetHdmiSettings
    ( const BVDC_Display_Handle        hDisplay,
      const BVDC_Display_HdmiSettings *pHdmiSettings );

/***************************************************************************
Summary:
    This function gets the display's HDMI settings

Description:
    This function will get the display's HDMI settings

Input:
    hDisplay        - Display handle created earlier.

Output:
    pHdmiSettings   - A reference to hdmi settings structure.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetHdmiSettings, BVDC_Display_HdmiSettings.
**************************************************************************/
BERR_Code BVDC_Display_GetHdmiSettings
    ( const BVDC_Display_Handle        hDisplay,
      BVDC_Display_HdmiSettings       *pHdmiSettings );

/***************************************************************************
Summary:
    This function sets the number of compositor lines that Hdmi drops
    for a given format

Description:
    Sets the number of compositor lines that Hdmi will drop for a specific
    format.  Returns an error if the format does not support this function,
    or if the number of lines is invalid.

Input:
    hDisplay        - Display handle created earlier.
    ulHdmi          - Which HDMI (For 7038, BVDC_Hdmi_0 is the only valid selection)
       eVideoFormat    - Which format to configure the number of lines dropped for.
       ulHdmiDropLines - The number of compositor lines to drop.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetHdmiDropLines
**************************************************************************/
BERR_Code BVDC_Display_SetHdmiDropLines
    ( const BVDC_Display_Handle        hDisplay,
      uint32_t                         ulHdmi,
      BFMT_VideoFmt                    eVideoFormat,
      uint32_t                         ulHdmiDropLines);

/***************************************************************************
Summary:
    This function returns the number of compositor lines that Hdmi drops
    for a given format

Description:
    Gets the number of compositor lines that Hdmi will drop for a specific
    format.

Input:
    hDisplay     - Display handle created earlier.
    ulHdmi       - Which HDMI (For 7038, BVDC_Hdmi_0 is the only valid selection)
       eVideoFormat - Which format to configure the number of lines dropped for.

Output:
    pulHdmiDropLines - pointer to the number of lines dropped.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetHdmiDropLines
**************************************************************************/
BERR_Code BVDC_Display_GetHdmiDropLines
    ( const BVDC_Display_Handle        hDisplay,
      uint32_t                         ulHdmi,
      BFMT_VideoFmt                    eVideoFormat,
      uint32_t                         *pulHdmiDropLines);

/***************************************************************************
Summary:
    This function sets the display's HDMI to output sync signal to HDMI
    core.

Description:
    This function will set the display's HDMI to output sync only.  No video
    will be output.  Only the sync (raster) signal is output with a fixed
    color (that is the same as the associated compositor).  Note this flag
    is ignore if the hDisplay does not drive the HDMI output.

Input:
    hDisplay        - Display handle created earlier.
    bSyncOnly       - Indicate to output sync only.   If false it out sync
                      and data.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetHdmiSyncOnly, BVDC_Display_SetHdmiConfiguration.
**************************************************************************/
BERR_Code BVDC_Display_SetHdmiSyncOnly
    ( const BVDC_Display_Handle        hDisplay,
      bool                             bSyncOnly );

/***************************************************************************
Summary:
    This function gets the display's HDMI to output sync signal to HDMI
    core.

Description:
    This function will get the display's HDMI to output sync only.  No video
    will be output.  Only the sync (raster) signal is output with a fixed
    color (that is the same as the associated compositor).  Note this flag
    is ignore if the hDisplay does not drive the HDMI output.

Input:
    hDisplay        - Display handle created earlier.
    pbSyncOnly      - Indicate to output sync only.   If false it out sync
                      and data.  State last set to vdc.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetHdmiSyncOnly, BVDC_Display_GetHdmiConfiguration.
**************************************************************************/
BERR_Code BVDC_Display_GetHdmiSyncOnly
    ( const BVDC_Display_Handle        hDisplay,
      bool                            *pbSyncOnly );

/***************************************************************************
Summary:
    This function sets the display's HDMI to output in xvYCC colorspace.

Description:
    This function will set the display's HDMI to output in xvYCC colorspace.
    This should be used to output xvYCC content to a xvYCC capable HDMI
    receiver. Note this flag is ignored if the hDisplay does not drive
    the HDMI output.

Input:
    hDisplay        - Display handle created earlier.
    bXvYCC          - Indicates xvYCC output.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
**************************************************************************/
BERR_Code BVDC_Display_SetHdmiXvYcc
    ( const BVDC_Display_Handle        hDisplay,
      bool                             bXvYcc );

/***************************************************************************
Summary:
    This function gets the display's HDMI xvYCC colorspace setting.

Description:
    This function will get the display's HDMI xvYCC colorspace setting.
    Note this flag is ignored if the hDisplay does not drive the HDMI
    output.

Input:
    hDisplay        - Display handle created earlier.
    pbXvYcc         - Indicate to output sync only.   If false it out sync
                      and data.  State last set to vdc.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
**************************************************************************/
BERR_Code BVDC_Display_GetHdmiXvYcc
    ( const BVDC_Display_Handle        hDisplay,
      bool                            *pbXvYcc );

/***************************************************************************
Summary:
    This function sets the display's HDMI to the desired color depth.

Description:
    This function will set the display's HDMI to output 24-bit
    standard color, or 30-, 36-, or 48-bit deep color.

    This should be used to output deep color to a capable HDMI
    receiver. This setting is ignored if the hDisplay does not drive
    the HDMI output.

Input:
    hDisplay        - Display handle created earlier.
    eColorDepth     - Indicates the desired color depth

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
**************************************************************************/
BERR_Code BVDC_Display_SetHdmiColorDepth
    ( const BVDC_Display_Handle        hDisplay,
      BAVC_HDMI_BitsPerPixel           eColorDepth );

/***************************************************************************
Summary:
    This function gets the display's HDMI Color Depth setting.

Description:
    This function will get the display's HDMI Color Depth setting.

Input:
    hDisplay        - Display handle created earlier.
    pColorDepth     - Pointer for receiving the color depth.


Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
**************************************************************************/
BERR_Code BVDC_Display_GetHdmiColorDepth
    ( const BVDC_Display_Handle        hDisplay,
      BAVC_HDMI_BitsPerPixel          *pColorDepth );

/***************************************************************************
Summary:
    This function sets the display's HDMI to the desired Pixel Repetition.

Description:
    This function will set the display's HDMI for pixel
        repetition (only pertinent for lower resolutions).

Input:
    hDisplay        - Display handle created earlier.
    ePixelRepetition     - Indicates the desired repetition factor

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
**************************************************************************/
BERR_Code BVDC_Display_SetHdmiPixelRepetition
    ( const BVDC_Display_Handle        hDisplay,
      BAVC_HDMI_PixelRepetition        ePixelRepetition );

/***************************************************************************
Summary:
    This function gets the display's HDMI Pixel Repetition setting.

Description:
    This function will get the display's HDMI Pixel Repetition setting.

Input:
    hDisplay        - Display handle created earlier.
    pPixelRepetition     - Pointer for receiving the pixel repetition.


Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
**************************************************************************/
BERR_Code BVDC_Display_GetHdmiPixelRepetition
    ( const BVDC_Display_Handle        hDisplay,
      BAVC_HDMI_PixelRepetition       *pPixelRepetition );

/***************************************************************************
Summary:
    This function sets the display's HDMI output format.

Description:
    This function will set the display's HDMI for output format.
    It's mainly used to output 4kx2k format on HDMI path only while the rest
    of the BVN cannot support these formats yet.

Input:
    hDisplay        - Display handle created earlier.
    eHDMIFormat     - Indicates the format used for HDMI path
    eMatchingFormat - Indicates when the HDMI format can be used

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
**************************************************************************/
BERR_Code BVDC_Display_SetHdmiFormat
    ( const BVDC_Display_Handle        hDisplay,
      BFMT_VideoFmt                    eHDMIFormat,
      BFMT_VideoFmt                    eMatchingFormat );

/***************************************************************************
Summary:
    This function gets the display's HDMI format setting.

Description:
    This function will get the display's HDMI format setting.

Input:
    hDisplay         - Display handle created earlier.
    peHDMIFormat     - Pointer for receiving the format used for HDMI path.
    peMatchingFormat - Pointer for receiving the format matched used

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
**************************************************************************/
BERR_Code BVDC_Display_GetHdmiFormat
    ( const BVDC_Display_Handle        hDisplay,
      BFMT_VideoFmt                   *peHDMIFormat,
      BFMT_VideoFmt                   *peMatchingFormat );

/***************************************************************************
Summary:
    This function configures the 656 output

Description:
    Enables or disables 656 output associated with a Display
    handle. Returns an error if the selection is invalid.  This function
    requires that all other display(s) must already shut off their 656 output
    and applied.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hDisplay    - Display handle created earlier.
    ul656Output - which Itur656 (For 7038, BVDC_Itur656Output_0 is the only valid selection)
    bEnable     - Enable or disable output

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_Get656Configuration
**************************************************************************/
BERR_Code BVDC_Display_Set656Configuration
    ( BVDC_Display_Handle              hDisplay,
      uint32_t                         ul656Output,
      bool                             bEnable );

/***************************************************************************
Summary:
    This function queries the 656 output settings

Description:
    Returns the Itur656 output selection.

Input:
    hDisplay    - Display handle created earlier.
    ul656Output - which Itur656 (For 7038, BVDC_Itur656Output_0 is the only valid selection)

Output:
    pbEnable656Output - 655 enabled or disabled

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_Set656Configuration
**************************************************************************/
BERR_Code BVDC_Display_Get656Configuration
    ( const BVDC_Display_Handle        hDisplay,
      uint32_t                         ul656Output,
      bool                            *pbEnable );

/***************************************************************************
Summary:
    This function configures the Rfm output

Description:
    Enables or disables the Rfm output associated with the display. Users can
    select either CVBS(composite) output from the Vec, or set a constant value
    to the RF port.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hDisplay         - Display handle created earlier.
    ulRfmOutput      - Which Rfm output (For 7038, BVDC_Rfm_0 is the only valid selection)
    eRfmOutput       - Rfm output
    ulConstantValue  - constant value for BVDC_RfmOutput_eConstant

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetRfmConfiguration
**************************************************************************/
BERR_Code BVDC_Display_SetRfmConfiguration
    ( BVDC_Display_Handle              hDisplay,
      uint32_t                         ulRfmOutput,
      BVDC_RfmOutput                   eRfmOutput,
      uint32_t                         ulConstantValue);

/***************************************************************************
Summary:
    This function returns a specific Rfm output setting

Description:
    Returns the Rfm output setting

Input:
    hDisplay    - Display handle created earlier.
    ulRfmOutput - Which Rfm output (For 7038, BVDC_Rfm_0 is the only valid selection)

Output:
    peRfmOutput       - Rfm output setting
    pulConstantValue  - Rfm constant value

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetRfmConfiguration
**************************************************************************/
BERR_Code BVDC_Display_GetRfmConfiguration
    ( const BVDC_Display_Handle        hDisplay,
      uint32_t                         ulRfmOutput,
      BVDC_RfmOutput                  *peRfmOutput,
      uint32_t                        *pulConstantValue);

/***************************************************************************
Summary:
    Enables or disables MPAA decimation on an output port associated with
    the display. MPAA decimation lowers the output video quality. It is
    typically used to protect HD content.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hDisplay - Display handle created earlier.
    eMpaaDeciIf - An output interface type that support MPAA decimation.
    ulOutPorts - Or-ed bits (e.g. BVDC_Hdmi_0 | BVDC_Hdmi_1) representing
    some output ports of eMpaaDeciIf type, configured with
    BVDC_Display_SetDacConfiguration, BVDC_Display_SetHdmiConfiguration, or
    BVDC_Display_Set656Configuration.
    bEnable  - Enable/Disable MPAA decimation

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

Note:
    Although this interface supports multiple ports for each eMpaaDeciIf
    type, the implementation currently supports only one port.

See Also:
    BVDC_Display_GetMpaaDecimation,
    BVDC_Display_SetDacConfiguration, BVDC_Display_SetHdmiConfiguration,
    BVDC_Display_Set656Configuration
**************************************************************************/
BERR_Code BVDC_Display_SetMpaaDecimation
    ( BVDC_Display_Handle              hDisplay,
      BVDC_MpaaDeciIf                  eMpaaDeciIf,
      uint32_t                         ulOutPorts,
      bool                             bEnable );

/***************************************************************************
Summary:
    Returns the MPAA decimation settings applied to the output port
    associated with the display.

Input:
    hDisplay - Display handle created earlier.
    eMpaaDeciIf - An output interface type that support MPAA decimation.
    ulOutPort - Bit field representing a specific output port of eMpaaDeciIf
    type, configured with BVDC_Display_SetDacConfiguration,
    BVDC_Display_SetHdmiConfiguration, or BVDC_Display_Set656Configuration.

Output:
    pbEnable  - Enable/Disable MPAA decimation

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetMpaaDecimation
**************************************************************************/
BERR_Code BVDC_Display_GetMpaaDecimation
    ( const BVDC_Display_Handle        hDisplay,
      BVDC_MpaaDeciIf                  eMpaaDeciIf,
      uint32_t                         ulOutPort,
      bool                            *pbEnable );

/***************************************************************************
Summary:
    This function enable/disable drop frame

Description:
    There some of the display format timing that capable of doing frame drop
    such as rate=x, rate=x/1.001.  For example 1080i/720p both can run at
    60.00Hz or frame drop at 59.94Hz.  If enable this function will do frame
    drop when applicable.  If enabled it will run at 59.94Hz, 29.97Hz, 23.97Hz,
    etc.  If disabled (default) it will run default rate specified in
    BVDC_Settings.eDisplayFrameRate.

Input:
    hDisplay - Display handle created earlier.
    eDropFrame -
        - eAuto use whatever last used.  Initial rate is from
            BVDC_Settings.eDisplayFrameRate
        - eOn will enable drop frame (e.g. 59.94hz)
        - eOff will disable drop frame (e.g. 60.00hz)

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetDropFrame, BVDC_Window_SetMasterFrameRate.
**************************************************************************/
BERR_Code BVDC_Display_SetDropFrame
    ( BVDC_Display_Handle              hDisplay,
      BVDC_Mode                        eDropFrame );

/***************************************************************************
Summary:
    This function get force frame drop status.

Description:
    There some of the display format timing that capable of doing frame drop
    such as rate=x, rate=x/1.001.  For example 1080i/720p both can run at
    60.00Hz or frame drop at 59.94Hz.  If enable this function will do frame
    drop when applicable.  If enabled it will run at 59.94Hz, 29.97Hz, 23.97Hz,
    etc.  If disabled (default) it will run default rate specified in
    BVDC_Settings.eDisplayFrameRate.

Input:
    hDisplay - Display handle created earlier.
    peDropFrame - Enabled/Disabled framedrop.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetDropFrame, BVDC_Window_SetMasterFrameRate.
**************************************************************************/
BERR_Code BVDC_Display_GetDropFrame
    ( const BVDC_Display_Handle        hDisplay,
      BVDC_Mode                       *peDropFrame );

/***************************************************************************
Summary:
    Set the display's timebase.

Description:
    A display can lock on any timebase.  Default VDC config would be
    display 0 (HD display) locks on timebase 0, display 1 (SD display)
    locks on timebase 1 and bypass display locks on timebase 2.
    The bypass video path is intended for analog PVR, which takes analog
    video source, output 656 to external mpeg encoder. The 656 output locks
    on the VDEC, which is locked by timebase 2 according to the baseline
    configuration.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hDisplay - Display handle created earlier.
    eTimeBase - The new display timebase.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
See Also:
**************************************************************************/
BERR_Code BVDC_Display_SetTimebase
    ( BVDC_Display_Handle              hDisplay,
      BAVC_Timebase                    eTimeBase );

/***************************************************************************
Summary:
    Get the display's timebase.

Description:
    A display can lock on any timebase.  Default VDC config would be
    display 0 (HD display) locks on timebase 0, display 1 (SD display)
    locks on timebase 1 and bypass display locks on timebase 2.
    The bypass video path is intended for analog PVR, which takes analog
    video source, output 656 to external mpeg encoder. The 656 output locks
    on the VDEC, which is locked by timebase 2 according to the baseline
    configuration.

Input:
    hDisplay - Display handle created earlier.

Output:
    peTimeBase - point to the current display timebase.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed
See Also:
**************************************************************************/
BERR_Code BVDC_Display_GetTimebase
    ( const BVDC_Display_Handle        hDisplay,
      BAVC_Timebase                   *peTimeBase );

/***************************************************************************
Summary:
    Get the current settings of the display callback.

Description:
    This function gets the current setings for the display callback function.

Input:
    hDisplay - Display handle created earlier.
    pSettings - Points to the current callback function settings.

See Also:
    BVDC_Display_CallbackMask
    BVDC_Display_CallbackData
    BVDC_Display_CallbackSettings
    BVDC_Display_InstallCallback
    BVDC_Display_GetCallbackSettings
***************************************************************************/
BERR_Code BVDC_Display_GetCallbackSettings
    ( BVDC_Display_Handle                  hDisplay,
      BVDC_Display_CallbackSettings       *pSettings );

/***************************************************************************
Summary:
    Set the settings of the display callback.

Description:
    This function sets the setings for the display callback function.

    Apps must call this function before BVDC_Display_InstallCallback.

Input:
    hDisplay - Display handle created earlier.
    pSettings - Points to the callback function settings defined by user.

See Also:
    BVDC_Display_CallbackMask
    BVDC_Display_CallbackData
    BVDC_Display_CallbackSettings
    BVDC_Display_InstallCallback
    BVDC_Display_GetCallbackSettings
***************************************************************************/
BERR_Code BVDC_Display_SetCallbackSettings
    ( BVDC_Display_Handle                  hDisplay,
      const BVDC_Display_CallbackSettings *pSettings );

/***************************************************************************
Summary:
    Install a callback with VDC for DISPLAY related update.

Description:
    This function install a callback with VDC to be called whenever VDC changes
    its display rate (59.94Hz, 60.00Hz, and etc) or other info, or to be called
    at every vsync for display cadence.

    Since HDMI module must run the same rate as display this callback serve as
    event update for HDMI. The rate can change on the fly according decoded
    source.

    The pvVdcData pointer passes back by this callback (BVDC_CallbackFunc_isr)
    is BVDC_Display_CallbackData.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hDisplay - Display handle created earlier.
    pfCallback - A function pointer to a callback that will update the HDMI's
    rate manager to match with the display's rate manager, and timebase.
    pvParm1 - User defined structure.casted to void.
    iParm2 - User defined value.

Output:
    None

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Window_SetMasterFrameRate,
    BVDC_Display_UnInstallCallback
**************************************************************************/
BERR_Code BVDC_Display_InstallCallback
    ( BVDC_Display_Handle              hDisplay,
      const BVDC_CallbackFunc_isr      pfCallback,
      void                            *pvParm1,
      int                              iParm2 );

/***************************************************************************
Summary:
    This function enables/disables a display's color correction.

Description:
    Enables/disables color correction functionality. This is used with
    BVDC_Display_LoadColorCorrectionTable or BVDC_DisplaySetColorCorrectionTable.

Input:
    hDisplay  - A valid display handle created earlier.
    bEnable   - Enable/disable color correcion.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully enabled/disabled color correction for
        the given display.

See Also:
    BVDC_Display_SetColorCorrectionTable, BVDC_Display_LoadColorCorrectionTable
**************************************************************************/
BERR_Code BVDC_Display_EnableColorCorrection
    ( BVDC_Display_Handle   hDisplay,
      bool                  bEnable );

/***************************************************************************
Summary:
    This function loads a color correction table.

Description:
    This loads an externally generated color correction table for the given
    display.

    Must call BVDC_Display_EnableColorCorrection for this to take effect. If
    BVDC_Display_SetColorCorrectionTable was called previously, those settings
    will be overridden by this table and vice-versa.

Input:
    hDisplay     - A valid display handle created earlier.
    pulCcbTable  - pointer to the color correction table.

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully loaded color correction table for the given
        display.

See Also:
    BVDC_Display_EnableColorCorrection, BVDC_Display_SetColorCorrectionTable
**************************************************************************/
BERR_Code BVDC_Display_LoadColorCorrectionTable
    ( BVDC_Display_Handle   hDisplay,
      const uint32_t      *pulCcbTable );

/***************************************************************************
Summary:
    This function sets a display's color correction via an index to
    an internal color correction table.

Description:
    The gamma table index and color temperature index are used to select the
    color correction table generated by the white balance algorithm.  If the
    gamma table index or the color temperature index is larger than the number
    of tables supported, invalid parameter error is return.

    Must call BVDC_Display_EnableColorCorrection for this to take effect. If
    BVDC_Display_LoadColorCorrectionTable was called previously, those settings
    will be overridden by the table used with this function call and vice-versa.

Input:
    hDisplay  - A valid display handle created earlier.

    ulGammaTableId - Gamma table ID to apply in the white balance.
    0 corresponds to the first table, N-1 the last table supported.  It is
    also the actual order of the gamma values specified when these tables
    are generated.  The actual gamma value corresponds to each table can be
    also found in the comment of each table.

    ulColorTempId - Color temperature ID to apply in the white balance.
    0 corresponds to the first color temperature value specified in the color
    temp input text file used to generate these Color Correction tables.  The
    actual color temp value corresponds to each table can also be found in the
    comment of each table.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set the color correction settings for the
        given display.

See Also:
    BVDC_Display_EnableColorCorrection, BVDC_Display_GetColorCorrectionTable,
    BVDC_Display_LoadColorCorrectionTable
**************************************************************************/
BERR_Code BVDC_Display_SetColorCorrectionTable
    ( BVDC_Display_Handle   hDisplay,
      uint32_t              ulGammaTableId,
      uint32_t              ulColorTempId );
/***************************************************************************
Summary:
    This function gets a display's index to an internal color correction Table.

Description:
    Gets the index associated with the display's current color correction table.
    The table is an internal table. As such, this can only be used
    with BVDC_Display_SetColorCorrectionTable. If used with
    BVDC_Display_LoadColorCorrectionTable, an error will be returned.

Input:
    hWindow  - A valid display handle created earlier.

Output:
    pbEnable        - Indicates if color correction is enabled

    pulGammaTableId - Display's current gamma table used for color correction

    pulColorTempId - Display's current color temp used for color correction

Returns:
    BERR_NOT_SUPPORTED - User is loading customized CCB table, cannot return
        internal indexes.
    BERR_SUCCESS - Successfully returned the index to the current color
        correction table for the given display.

See Also:
    BVDC_Display_EnableColorCorrection, BVDC_Window_SetColorCorrectionTable
**************************************************************************/
BERR_Code BVDC_Display_GetColorCorrectionTable
    ( const BVDC_Display_Handle   hDisplay,
      bool                       *pbEnable,
      uint32_t                   *pulGammaTableId,
      uint32_t                   *pulColorTempId );

/***************************************************************************
Summary:
    Sets the 3x5 color matrix in DVO path for the display.

Description:
    This matrix is used to convert the incoming BVN color space into
    the DVO output color space.
    Note, setting this matrix will override the VDC's internal one.

    The values used for the color matrix are equal to the values stored
    in the pl32_Matrix array right shifted by the ulShift value. In
    addition, negative values are converted to positive values before
    shifting and then turned back into negative values. For
    example, if a value in the matrix was 4 and the shift value was 2, then
    the final value is 1 (4>>2 = 1). If a value in the matrix was 7
    and the shift value was 3, then the final value would be 0.875.
    If a value in the matrix was -7 and the shift value was 3 then the
    final values is -0.875.

    Legal offset values in column 5 range from 0.0-255.0.

    The values within the color matrix are in the following order when output
    is configured as YCbCr:

    [Y_out ]   [M[0]  M[1]  M[2]  M[3] [M[4] ]   [Y_in ]
    [Cb_out]   [M[5]  M[6]  M[7]  M[8] [M[9] ]   [Cb_in]
    [Cr_out] = [M[10] M[11] M[12] M[13][M[14]] * [Cr_in]
    [A_out ]   [0     0     0     1     0    ]   [A_in ]
    [  1   ]   [0     0     0     0     1    ]   [  1  ]

    {Y_in, Cb_in, Cr_in, A_in}     = Input pixel before apply M matrix.
    {Y_out, Cb_Out, Cr_out, A_out} = Output pixel after apply M matrix.

    When the output is configured as RGB, the values within the color matrix
    are in the following order:

    [R_out ]   [M[0]  M[1]  M[2]  M[3] [M[4] ]   [Y_in ]
    [G_out ]   [M[5]  M[6]  M[7]  M[8] [M[9] ]   [Cb_in]
    [B_out ] = [M[10] M[11] M[12] M[13][M[14]] * [Cr_in]
    [A_out ]   [0     0     0     1     0    ]   [A_in ]
    [  1   ]   [0     0     0     0     1    ]   [  1  ]

    {Y_in, Cb_in, Cr_in, A_in}     = Input pixel before apply M matrix.
    {R_out, G_Out, B_out, A_out} = Output pixel after apply M matrix.

    How the values in the table are interpreted and read in depend on whether
    the output is configured as RGB or YCbCr by BVDC_Display_SetHdmiConfiguration().

    Note: The last two rows of the above matrix are used to facilitate
    computation only. It is not implemented physically in hardware.
    Columns 0 to 2 are multiplicative coefficients. Columns 4 and 5 are
    additive coefficients.  Note that for video csc the current hw does not
    support alpha multiply.

Input:
    hDisplay    - A valid display handle created earlier.
    bOverride   - Whether to override VDC internal matrix with the supplied
                  one; if false, pl32_Matrix and ulShift will be ignored,
                  and the default matrices inside VDC will take place;
    pl32_Matrix - An int32_t array that holds the 3x5 matrix coeffs.
    ulShift     - Matrix bit shift value; this parameter specifies the
                  number of fractional bits in the custom matrix coeffs;
                  its intention is to make API portable and hardware
                  independent; user should choose a much higher precision
                  than current hw precision to be future prove; VDC
                  will convert the user matrxi coeffs into final hw
                  settings and hide chip specific precision difference;

Output:
    None

Returns:
    BERR_SUCCESS - Color matrix was set.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also: BVDC_Display_SetHdmiConfiguration
****************************************************************************/
BERR_Code BVDC_Display_SetDvoColorMatrix
    ( BVDC_Display_Handle              hDisplay,
      bool                             bOverride,
      const int32_t                    pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                         ulShift );

/***************************************************************************
Summary:
    Gets the 3x5 color matrix in DVO path for the display.

Description:
    This matrix is used to convert the incoming source color space into
    CbYCr color space.  This fuction will get the current matrix in used.
    Either internal by VDC's or by user specified with bOverided.

    The values used for the color matrix are equal to the values stored
    in the pl32_Matrix array right shifted by the ulShift value. In
    addition, negative values are converted to positive values before
    shifting and then turned back into negative values. For
    example, if a value in the matrix was 4 and the shift value was 2, then
    the final value is 1 (4>>2 = 1). If a value in the matrix was 7
    and the shift value was 3, then the final value would be 0.875.
    If a value in the matrix was -7 and the shift value was 3 then the
    final values is -0.875.

    Legal offset values in column 5 range from 0.0-255.0.

    The values within the color matrix are in the following order when output
    is configured as YCbCr:

    [Y_out ]   [M[0]  M[1]  M[2]  M[3] [M[4] ]   [Y_in ]
    [Cb_out]   [M[5]  M[6]  M[7]  M[8] [M[9] ]   [Cb_in]
    [Cr_out] = [M[10] M[11] M[12] M[13][M[14]] * [Cr_in]
    [A_out ]   [0     0     0     1     0    ]   [A_in ]
    [  1   ]   [0     0     0     0     1    ]   [  1  ]

    {Y_in, Cb_in, Cr_in, A_in}     = Input pixel before apply M matrix.
    {Y_out, Cb_Out, Cr_out, A_out} = Output pixel after apply M matrix.

    When the output is configured as RGB, the values within the color matrix
    are in the following order:

    [R_out ]   [M[0]  M[1]  M[2]  M[3] [M[4] ]   [Y_in ]
    [G_out ]   [M[5]  M[6]  M[7]  M[8] [M[9] ]   [Cb_in]
    [B_out ] = [M[10] M[11] M[12] M[13][M[14]] * [Cr_in]
    [A_out ]   [0     0     0     1     0    ]   [A_in ]
    [  1   ]   [0     0     0     0     1    ]   [  1  ]

    {Y_in, Cb_in, Cr_in, A_in}     = Input pixel before apply M matrix.
    {R_out, G_Out, B_out, A_out} = Output pixel after apply M matrix.

    How the values in the table are read and returned depend on whether
    the output is configured as RGB or YCbCr by BVDC_Display_SetHdmiConfiguration().

    Note: The last two rows of the above matrix are used to facilitate
    computation only. It is not implemented physically in hardware.
    Columns 0 to 2 are multiplicative coefficients. Columns 4 and 5 are
    additive coefficients.  Note that for video csc the current hw does not
    support alpha multiply.

Input:
    hSource     - A valid source handle created earlier.

Output:
    pbOverride  - Whether the csc was using a user supplied override matrix
    or an internal VDC matrix.

    pl32_Matrix - An int32_t array that holds the 3x5 matrix coeffs.

    pulShift    - Matrix bit shift value; this parameter specifies the
    number of fractional bits in the custom matrix coeffs; its intention is to
    make API portable and hardware independent; user should choose a much
    higher precision than current hw precision to be future prove; VDC
    will convert the user matrxi coeffs into final hw settings and hide chip
    specific precision difference;

    if *pbOverride is true it will return the shift value previous set
    by user otherwise it will return shift value of 16; or another word the
    matrix element is in the form of S15.16 2's complement.

Returns:
    BERR_SUCCESS - Color matrix was set.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also: BVDC_Display_SetHdmiConfiguration
****************************************************************************/
BERR_Code BVDC_Display_GetDvoColorMatrix
    ( BVDC_Display_Handle              hDisplay,
      bool                            *pbOverride,
      int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                        *pulShift );

/***************************************************************************

Summary:
   This function sets the dvo conversion matrix at the current RGB attenuation.

Description:
    Sets the conversion matrix at the current RGB attenuation.

Input:
     hWindow        - A valid window handle created earlier.

Output:
     nAttenuationR/G/B, nOffsetR/G/B    - Conversion matrix in RGB domain
                                          in 20.11 signed fixed point.
                                          1 is represented by 0x800 and
                                          -1 is 0xffffffff.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned RGB attenuation values for the window.

See Also: BVDC_Display_GetDvoAttenuationRGB
**************************************************************************/
BERR_Code BVDC_Display_SetDvoAttenuationRGB
    ( BVDC_Display_Handle              hDisplay,
      int32_t                          nAttenuationR,
      int32_t                          nAttenuationG,
      int32_t                          nAttenuationB,
      int32_t                          nOffsetR,
      int32_t                          nOffsetG,
      int32_t                          nOffsetB);

/***************************************************************************

Summary:
   This function gets the conversion matrix at the current RGB attenuation.

Description:
    Gets the conversion matrix at the current RGB attenuation.

Input:
     hWindow        - A valid window handle created earlier.

Output:
     plAttenuationR/G/B, plOffsetR/G/B - Conversion matrix in RGB domain
                                         in 20.11 signed fixed point.
                                         1 is represented by 0x800 and
                                         -1 is 0xffffffff.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully returned RGB attenuation values for the window.

See Also: BVDC_Display_SetDvoAttenuationRGB
**************************************************************************/
BERR_Code BVDC_Display_GetDvoAttenuationRGB
    ( BVDC_Display_Handle              hDisplay,
      int32_t                         *plAttenuationR,
      int32_t                         *plAttenuationG,
      int32_t                         *plAttenuationB,
      int32_t                         *plOffsetR,
      int32_t                         *plOffsetG,
      int32_t                         *plOffsetB);

/***************************************************************************
Summary:
    This function set the H/V DE sync polarity for DVO output.

Description:
    This function should only be used when the output video format is one of
        BFMT_VideoFmt_eCustom0
        BFMT_VideoFmt_eCustom1
        BFMT_VideoFmt_eCustom2

Input:
    hDisplay  - A valid display handle created earlier.
    pDvoSettings - DVO sync settings.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetDvoConfiguration
**************************************************************************/
BERR_Code BVDC_Display_SetDvoConfiguration
    ( BVDC_Display_Handle              hDisplay,
      const BVDC_Display_DvoSettings  *pDvoSettings );

/***************************************************************************
Summary:
    This function get the H/V DE sync polarity for DVO output.

Description:

Input:
    hDisplay  - A valid display handle created earlier.

Output:
    pDvoSettings - DVO sync settings storage

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetDvoConfiguration
**************************************************************************/
BERR_Code BVDC_Display_GetDvoConfiguration
    ( const BVDC_Display_Handle        hDisplay,
      BVDC_Display_DvoSettings        *pDvoSettings );

/***************************************************************************
Summary:
    This function sets the display alignment control.

Description:
    User may want to align one display's Vsync timing with target display for various
    purpose, e.g. lipsync. The setting structure specifies which direction the alignment
    may be performed, where eFaster means to speedup current display's clock to align
    its Vsync timing to the target display; eSlower means to slow down the current
    display's clock to align its Vsync timing to the target display. However, for lipsync
    purpose, it's desired to automatically align display in certain direction if the target and
    this display are both in interlaced formats to have cadence match; user needs to set
    the alignment direction to eAuto.

    when VDC finds out the current display and target display's Vsync timing difference
    is within a tight rang, it will automatically stop the process until the next time
    it finds out the delta is out of that control range.

    Note this function only makes sense when the target display and the current display
    have the lockable vertical refresh rate pair (25+50, 50+50, 59+59, 23.97+59.94 or 29.97+59.94).

    Also note that VEC alignment combines with window bForceSyncLocked setting can reduce
    the video window memory allocation from sync-slipped multi-buffers to double-buffers.

    For stability, we don't support alignment chain; when window game mode is on, we
    don't support alignment.

Input:
    hDisplay          - A valid display handle created earlier.
    hTargetDisplay - The target display handle; NULL effectively disable the feature.
    pAlignSettings  - alignment settings; NULL effectively disable the feature.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_AlignmentSettings
    BVDC_Display_GetAlignment
    BVDC_Window_SetGameModeDelay
**************************************************************************/
BERR_Code BVDC_Display_SetAlignment
    ( BVDC_Display_Handle                   hDisplay,
      BVDC_Display_Handle                   hTargetDisplay,
      const BVDC_Display_AlignmentSettings *pAlignSettings );

/***************************************************************************
Summary:
    This function gets the display alignment setting.

Description:

Input:
    hDisplay  - A valid display handle created earlier.

Output:
    phTargetDisplay - The target display handle
    pAlignSettings    - alignment settings.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_AlignmentSettings
    BVDC_Display_SetAlignment
**************************************************************************/
BERR_Code BVDC_Display_GetAlignment
    ( const BVDC_Display_Handle        hDisplay,
      BVDC_Display_Handle             *phTargetDisplay,
      BVDC_Display_AlignmentSettings  *pAlignSettings );

/***************************************************************************
Summary:
    This function sets the display orientation.

Description:
    This function allows users to overwrite the default orientation defined
    in FMT with the 3D orientation (L/R or O/U)

Input:
    hDisplay    - A valid display handle created earlier.
    eOrienation - 2D/3D orienation to be displayed

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetOrientation
**************************************************************************/
BERR_Code BVDC_Display_SetOrientation
    ( const BVDC_Display_Handle        hDisplay,
      BFMT_Orientation                 eOrientation );


/***************************************************************************
Summary:
    This function gets the display 3D Orientation setting.

Description:

Input:
    hDisplay  - A valid display handle created earlier.

Output:
    peOrientation - Orientation setting

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetOrientation
**************************************************************************/
BERR_Code BVDC_Display_GetOrientation
    ( const BVDC_Display_Handle        hDisplay,
      BFMT_Orientation                *peOrientation );

/***************************************************************************
Summary:
    This function sets the source buffer selection in 3d display.

Description:

Input:
    hDisplay     - A valid display handle created earlier.
    e3dSrcBufSel - 3D source buffer selection used for display

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_Get3dSourceBufferSelect
**************************************************************************/
BERR_Code BVDC_Display_Set3dSourceBufferSelect
    ( const BVDC_Display_Handle        hDisplay,
      BVDC_3dSourceBufferSelect        e3dSrcBufSel );


/***************************************************************************
Summary:
    This function gets the source buffer selection in 3d display.

Description:

Input:
    hDisplay      - A valid display handle created earlier.
    pe3dSrcBufSel - 3D source buffer selection setting

Output:

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_Set3dSourceBufferSelect
**************************************************************************/
BERR_Code BVDC_Display_Get3dSourceBufferSelect
    ( const BVDC_Display_Handle        hDisplay,
      BVDC_3dSourceBufferSelect       *pe3dSrcBufSel );

/***************************************************************************
Summary:
    Get the display's capabilities.

Description:
    This function gets the display's capabilities.

Input:
    hDisplay - A valid display handle created earlier.  If NULL it will return
    the general capabilties of display system.

Output:
    pCapabilities - point to the display capabilities structure.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_Capabilities
**************************************************************************/
BERR_Code BVDC_Display_GetCapabilities
    ( BVDC_Display_Handle              hDisplay,
      BVDC_Display_Capabilities       *pCapabilities );

/*****************************************************************************
Summary:
    Structure for information about VEC IT microcode

See Also:
    BVDC_Display_GetItUcodeInfo
 *****************************************************************************/
typedef struct BVDC_Display_ItUcodeInfo
{
    uint32_t ulAnalogTimestamp; /* Time stamp for analog microcode */
    uint32_t ulAnalogChecksum;  /* Checksum for analog microcode */
    uint32_t ulI656Timestamp;   /* Time stamp for ITU-R 656 microcode */
    uint32_t ulI656Checksum;    /* Checksum for ITU-R 656 microcode */
    uint32_t ulDviTimestamp;    /* Time stamp for DVI/HDMI microcode */
    uint32_t ulDviChecksum;     /* Checksum for DVI/HDMI microcode */

} BVDC_Display_ItUcodeInfo;

/*****************************************************************************
Summary:
    Returns information about VEC IT microcde currently loaded in hardware.

Description:
    This function returns time stamps and checksums for VEC IT
    microcode currently loaded into hardware. Information is returned
    for analog microcode, ITU-R656 microcode, and DVI/HDMI microcode.

Input:
    hDisplay - A valid display handle.

Output:
    pInfo - The microcode information.

Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - A supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVDC_Display_GetItUcodeInfo
    ( BVDC_Display_Handle hDisplay,
      BVDC_Display_ItUcodeInfo* pInfo );

/*****************************************************************************
Summary:
    Ramp STG display resolution.

Description:
    This function forces STG display to ramp resolution from 1/4 size to full size in specified
    number of pictures. This is a one shot call.

Input:
    hDisplay - A valid display handle.
    ulResolutionRampCount - number of pictures

Output:

Returns:
    BERR_SUCCESS              - The function call succeeded.
    BERR_INVALID_PARAMETER    - A supplied parameter was invalid,
                                possibly NULL.
 *****************************************************************************/
BERR_Code BVDC_Display_RampStgResolution
    ( BVDC_Display_Handle hDisplay,
      uint32_t            ulResolutionRampCount );

/***************************************************************************
Summary:
    This function set the STG configuration for ViCE output.

Description:
    This function enables/disables STG output and configures the STG trigger
    mode as real-time(RT) vs non-realtime(NRT) if enabled.

    This API is required to enable STG display output in master mode.
    If hDisplay has a non-STG master timing generator, this API attaches or
    detaches STG as a slave output to the master display.

    In RT mode, STG generates timer triggers based on fixed display frame rate,
    which is for real-time video encode/transcode usage;

    In NRT mode, STG generates end-of-picture(EOP) triggers based on the BVB
    EOP event, which is for file-to-file non-realtime transcode usage.

    Does not take immediate effect. Requires an ApplyChanges() call.

Input:
    hDisplay  - A valid display handle created earlier.
    bEnable   - explicit enable/disable flag.
    pStgSettings - STG settings.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_StgSettings
    BVDC_Display_GetStgConfiguration
**************************************************************************/
BERR_Code BVDC_Display_SetStgConfiguration
    ( BVDC_Display_Handle              hDisplay,
      bool                             bEnable,
      const BVDC_Display_StgSettings  *pStgSettings );

/***************************************************************************
Summary:
    This function get the STG settings for ViCE output.

Description:

Input:
    hDisplay  - A valid display handle created earlier.

Output:
    pbEnable     - enable flag storage
    pStgSettings - STG settings storage

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_StgSettings
    BVDC_Display_SetStgConfiguration
**************************************************************************/
BERR_Code BVDC_Display_GetStgConfiguration
    ( const BVDC_Display_Handle        hDisplay,
      bool                            *pbEnable,
      BVDC_Display_StgSettings        *pStgSettings );

/***************************************************************************
Summary:
    Get the Display interrupt name for the new state

Description:
    This function returns the interrupt name associated with the Display
    RUL when execution is completed. Applications can use the interrupt
    name to register a callback with BINT.

Input:
    hDisplay       - Display handle created earlier with BVDC_Display_Create.
    eFieldPolarity - Field polarity for that interrupt (top or bottom)

Output:
    pInterruptName - The L2 interrupt name that can be use with BINT
    to create the callback.

Returns:
    BERR_INVALID_PARAMETER - hDisplay is not a valid display handle.
    BERR_SUCCESS - Function succeed

See Also:
    BINT_CreateCallback.
****************************************************************************/
BERR_Code BVDC_Display_GetInterruptName
    ( BVDC_Display_Handle              hDisplay,
      const BAVC_Polarity              eFieldId,
      BINT_Id                         *pInterruptName );

/***************************************************************************
Summary:
    Set user specified custom VF filter

Description:
    This function allows the user to specify a custom VF filter and
    override hardcoded defaults, or disable custom VF filters and
    restore defaults.

Input:
    hDisplay          - Display handle created earlier with BVDC_Display_Create.
    eDisplayOutput    - The display output to apply filter settings to.
    eDacOutput        - The output channel to apply filter settings to.
    bOverride         - Whether to override VDC internal VF filters with the supplied
                        one; if false, paulFilterTaps and ulSize will be ignored,
                        and the default VF filters inside VDC will be used.
    paulFilterRegs    - The VF filter tap registers in an array.
    ulNumFilterRegs   - Number of elements in VF filter tap register array.  Should be
                        5 or greater for chips with 8-bit filters and 10 or greater
                        for chips with 10-bit filters.
    ulSumOfTaps       - "Sum of taps" bitfield consistent with the above array.
                        Like the array coefficients, this is a hardware-specific
                        number with typical values 0, 1, 2, 3.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetVfFilter.
****************************************************************************/
BERR_Code BVDC_Display_SetVfFilter
    ( BVDC_Display_Handle              hDisplay,
      BVDC_DisplayOutput               eDisplayOutput,
      BVDC_DacOutput                   eDacOutput,
      bool                             bOverride,
      const uint32_t                  *paulFilterRegs,
      uint32_t                         ulNumFilterRegs,
      uint32_t                         ulSumOfTaps );

/***************************************************************************
Summary:
    Get VF filter currently being used

Description:
    This function retrieves the current VF filter used by VDC for a
    the selected output and channel.

Input:
    hDisplay          - Display handle created earlier with BVDC_Display_Create.
    eDisplayOutput    - The display output to get filter settings of.
    eDacOutput        - The output channel to get filter settings of.

Output:
    pbOverride        - Whether VDC is currently using a hardcoded default
                        internal VF filter or a custom one supplied by the user.
    paulFilterRegs    - The currently used VF filter tap registers copied to this array.
    ulNumFilterRegs   - Number of elements in VF filter tap register array.  Should be
                        5 or greater for chips with 8-bit filters and 10 or greater
                        for chips with 10-bit filters.
    pulSumOfTaps      - "Sum of taps" bitfield consistent with the above array.
                        Like the array coefficients, this is a hardware-specific
                        number with typical values 0, 1, 2, 3.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetVfFilter.
****************************************************************************/
BERR_Code BVDC_Display_GetVfFilter
    ( BVDC_Display_Handle              hDisplay,
      BVDC_DisplayOutput               eDisplayOutput,
      BVDC_DacOutput                   eDacOutput,
      bool                            *pbOverride,
      uint32_t                        *paulFilterRegs,
      uint32_t                         ulNumFilterRegs,
      uint32_t                        *pulSumOfTaps );

/***************************************************************************
Summary:
    Mute a selected output

Description:
    This function allows the selected output to be muted while preserving
    sync.  Please note that Svideo and composite share muting, so
    muting/unmuting one will mute/unmute the other.

Input:
    hDisplay          - Display handle created earlier with BVDC_Display_Create.
    eDisplayOutput    - The display output to apply mute settings to.
    eMuteMode         - Mute mode for the selected display output.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetMuteMode.
****************************************************************************/
BERR_Code BVDC_Display_SetMuteMode
    ( BVDC_Display_Handle              hDisplay,
      BVDC_DisplayOutput               eDisplayOutput,
      BVDC_MuteMode                    eMuteMode );

/***************************************************************************
Summary:
    Get the mute settings of a selected output

Description:
    This function retrieves the mute status of the selected output.

Input:
    hDisplay          - Display handle created earlier with BVDC_Display_Create.
    eDisplayOutput    - The display output to apply mute settings to.

Output:
    peMuteMode        - Returned mute mode of selected output.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetMuteMute.
****************************************************************************/
BERR_Code BVDC_Display_GetMuteMode
    ( BVDC_Display_Handle              hDisplay,
      BVDC_DisplayOutput               eDisplayOutput,
      BVDC_MuteMode                   *peMuteMode );

/***************************************************************************
Summary:
    Write an artificial vysnc interrupt bit.

Description:
    This function set a "go" bit reg address for encoder.  This is to
    generate an artificial display vsync.

Input:
    hDisplay
        - Display handle created earlier with BVDC_Display_Create.

    bEnable
        - To enable the artificial vsync pulse.  Default is false.

    ulVsyncRegAddr
        - The address for VEC to write vsync bit.  It's expected that
        VDC will have complete access (e.g. Power is enabled, not in reset, no
        GISB timeout, etc) when VDC access to this register when enable.

    ulVsyncMask
        - The artificial vsync bit mask.  The value is expected to be write-thru
        to ulVsyncRegAddr with no bits to be preserved.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_GetArtificialVsync.
****************************************************************************/
BERR_Code BVDC_Display_SetArtificialVsync
    ( BVDC_Display_Handle              hDisplay,
      bool                             bEnable,
      uint32_t                         ulVsyncRegAddr,
      uint32_t                         ulVsyncMask );
/* DEPRECATED: */
#define BVDC_Display_SetArtificticalVsync BVDC_Display_SetArtificialVsync

/***************************************************************************
Summary:
    Get the current artificial vysnc settings.

Description:
    This function get the current artificial vysnc settings.

Input:
    hDisplay
        - Display handle created earlier with BVDC_Display_Create.

    pbEnable
        - To enable the artificial vsync pulse.  Default is false.

    pulVsyncRegAddr
        - The address for VEC to write vsync bit.  It's expected that
        VDC will have complete access (e.g. Power is enabled, not in reset, no
        GISB timeout, etc) when VDC access to this register when enable.

    pulVsyncMask
        - The artificial vsync bit mask.  The value is expected to be write-thru
        to ulVsyncRegAddr with no bits to be preserved.

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Function succeed

See Also:
    BVDC_Display_SetArtificialVsync.
****************************************************************************/
BERR_Code BVDC_Display_GetArtificialVsync
    ( const BVDC_Display_Handle        hDisplay,
      bool                            *pbEnable,
      uint32_t                        *pulVsyncRegAddr,
      uint32_t                        *pulVsyncMask );
/* DEPRECATED: */
#define BVDC_Display_GetArtificticalVsync BVDC_Display_GetArtificialVsync

/***************************************************************************
Summary:
    Obtains the displayable point picture buffer from the associated video display in
    _isr context

Description:
    The display picture buffers are allocated by VDC STG display.
    An example application of this is for upper level software
    to get the current VDC display buffer and to pass to Raaga encoder,
    inside the callback function called from VDC at every display vsync. The
    callback function is installed by upper level software to VDC display.

    Upper level software should call BVDC_Display_GetBuffer_isr inside
    or after the display's per vsync callback.

    Note, this function is destructive that the output buffer will be removed from display
    encode picture queue, so upper layer is responsible to return later to prevent leak.

Input:
    hDisplay - the display handle created earlier with BVDC_Display_Create.

Output:
    pEncodePicture - the picture buffer captured at the displayable point.

Returns:

See Also:
    BVDC_Display_ReturnBuffer_isr, BAVC_EncodePictureBuffer,
    BVDC_Display_InstallCallback
**************************************************************************/
void BVDC_Display_GetBuffer_isr
(   BVDC_Display_Handle        hDisplay,
    BAVC_EncodePictureBuffer  *pEncodePicture);

/***************************************************************************
Summary:
    Returns a buffer to the associated display in _isr context.

Description:
    This is the counterpart to the BVDC_Display_GetBuffer_isr.

Input:
    hDisplay          - the display handle created earlier with BVDC_Display_Create.
    pEncodePicture - the picture buffer captured at the displayable point.

Output:
    None

Returns:

Note:
    An outstanding display buffer must be returned and freed if the
    associated display is to be destroyed to prevent memory leak.

    Also NOTE: user MUST return all extracted encode pictures back to display before disabling
    VIP or change VIP heap memory settings!!

See Also:
    BVDC_Display_GetBuffer_isr, BAVC_EncodePictureBuffer,
    BVDC_Display_InstallCallback
    BVDC_Display_StgSettings
**************************************************************************/
void BVDC_Display_ReturnBuffer_isr
(   BVDC_Display_Handle        hDisplay,
    const BAVC_EncodePictureBuffer  *pEncodePicture);

/***************************************************************************
Summary:
    This function sets a display's bar data handling mode. If eMode is eAuto, VDC internally
    would compute the bar data type and value; else if eOn, the bar data type and value
    will be set from this API and ignore from source; else bar data is dropped, i.e., no bar
    data is output from the display, i.e., eInvalid bar data type is used.

Description:
    Enables/disables/auto mode bar data functionality for encoder.

Input:
    hDisplay  - A valid display handle created earlier.
    eMode   - Enable/disable/auto mode bar data. If eOff, bar data will be disabled.
    eType     - Bar data type if mode is eOn.
    ulTopLeftBarData - Bar data value top/left setting if mode is eOn.
    ulBotRightBarData - Bar data value bot/right setting if mode is eOn.

Output:
    none

Returns:
    BERR_INVALID_PARAMETER - Invalid function parameters.
    BERR_SUCCESS - Successfully set bar data for the given display.

See Also:
**************************************************************************/
BERR_Code BVDC_Display_SetBarData
    ( BVDC_Display_Handle   hDisplay,
      BVDC_Mode             eMode,
      BAVC_BarDataType      eType,
      uint32_t              ulTopLeftBarData,
      uint32_t              ulBotRightBarData);

/***************************************************************************
Summary:
   Nexus will call this function to discover all the (memory access) cores
   use by this window.
Description:
Returns:
See Also:
****************************************************************************/
void BVDC_Window_GetCores
    ( BVDC_WindowId                    eWindowId,     /* [in] window id */
      BBOX_Handle                      hBox,          /* [in] boxmode configuratuion */
      BVDC_Source_Handle               hSource,       /* [in] source for this window */
      BVDC_Compositor_Handle           hCompositor,   /* [in] compositor the window belong to */
      BAVC_CoreList                   *pCoreList      /* [out] all potential cores use by this window */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_H__ */

/* End of File */
