/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

#ifndef BGRC_H__
#define BGRC_H__

#include "bchp.h"
#include "bmma.h"
#include "bint.h"
#include "breg_mem.h"
#include "bgrc_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=Module Overview: ********************************************************
The purpose of this module is to take surfaces allocated by the surface
module, composite the pixels together, and create new pixels for another
surface. The resulting surface (frame buffer) can then be displayed by
providing it to the Video Display Control module, or it can be used in
another graphics compositor operation to create another result.

Non Real-Time Client
--------------------

Each operation of the compositor may take several fields to compute
and the same operation may take more or less time each time it is
computed. The compositor is referred to as a "non real-time client" since
its operations are not guaranteed to finish within a certain amount of
time.

The reasons for using this approach are:

o The entire graphics operation can be computed once and stored in a frame
  buffer. Other real-time graphics engines that do not use frame buffers
  have to re-compute the results of graphics and contually consume bandwidth
  in order to compute the same result.

o Since we are allowed to take several frames to compute a frame buffer, the
  complexity of the operation is no longer constrained by the available
  bandwidth of one field. We can now implement programmable blenders,
  high-resolution scaling in both X and Y, and anti-flutter filters for HD
  resolution graphics without reducing the real-time capabilities of
  the chipset.

o The number of source surfaces used to create the frame buffer is not
  limited. Typical real-time graphics engines have limits on how many
  surfaces they support per line and how many may overlap on a given pixel.

o Complex graphics operations do not effect the video display. Whatever
  bandwidth is remaining after the other chipset operations will be used to
  compute the graphics. In real-time graphics, if you pushed the capabilities
  of the graphics engine, you could affect the actual display and other
  real-time clients by reducing their available bandwidth.

The disadvantages to this approach are:

o User must allocate a frame buffer to store the computed frame buffer.
  In order to prevent tearing and similar issues, the user
  will need to implement double-buffering or a similar mechanism.

o When the user changes an original source surface, the user must
  re-compute the frame buffer which used that source surface. Graphics
  engines that don't use frame buffers don't have this problem since
  they are continually updating the display allowing them to automatically
  update when a change to a source surface is made.

Color Channels
--------------

The graphics compositor does not work in one specific color space, such as
YCbCr or RGB space, but operates in a generic color space that the user
defines. This arbitrary color space has one alpha channel, always referred
to as A, and three color channels, referred to as C2, C1, and C0.

Once the user understands the rules for converting input pixels into
(A,C2,C1,C0) space, and understands the steps taken from input to output,
this generic color space will allow the user to define the color space at
many of the stages in the pipeline. Literally the user can have a color
space for the input data, a second color space for blending, and a third
color space for the output. They could also just use one color space for
the entire pipeline.

The rules for translation are as follows. When constant values are provided,
such as when supplying color key ranges, matrix coefficients, and constant
color values, they are always provided as (A,C2,C1,C0) values. These values
are passed directly to hardare for use in the pipeline. When pixels
are translated from input surfaces, alpha and color channels use the
following rules that operate according to the surface's format:

o If the pixels contain an alpha value, the alpha channel (A) will contain
  the pixel value. If it doesn't, the constant alpha value will be used.
o If the pixels contain color information, the color channels (C2,C1,C0)
  will be assigned colors from that pixel. If using YCrCb colors then
  C2=Y, C1=Cb, C0=Cr. If using RGB colors then C2=R, C1=G, C0=B. If the
  pixels do not contain color information the constant color channel values
  will be used.
o If the surface references a palette, the above rules are applied on the
  palette entries according to the format of the palette. If the pixels
  also contain other values, such as an alpha in addition to a palette index,
  then those values will replace any entries within the palette before
  conversion. The user may override derefrencing of the palette and use
  the index value directly. In that case, C0 will contain the palette index
  while the other two color channels will contain zero.
o If the surface uses a compressed format, such as BPXL_eY08_Cb8_Y18_Cr8,
  the values will be uncompressed using user specified chroma expansion
  rules before using the above rules t convert into (A,C2,C1,C0) values.
o After conversion, all color channels will be internally stored as 8-bit
  values. When used in equations, such as the blending equation, those
  8-bit values are treated as if that range yielded values from 0 to 1
  (inclusive).

The same rules are used in reverse to store color channels in the output.

In all of the above cases, when an input pixel value contains less than
8 bits, the value will be converted to an 8-bit value by replicating the
high order bits. For example, if you have a 5-bit binary value of 01101,
the resulting 8-bit expanded value will be 01101011. This rule applies
for the alpha (A) channel as well as the color channels (C2,C1,C0).

Color conversion example
------------------------

BPXL_eR5_G6_B5        - Alpha (A) from constant. C2=R, C1=G, and C0=B.
BPXL_eB5_G5_R5_A1     - Alpha (A) from pixel. C2=R, C1=G, and C0=B.
BPXL_eA8              - Alpha (A) from pixel. C2, C1, and C0 from constant.
BPXL_eY08_Cb8_Y18_Cr8 - Alpha (A) from constant for both pixels. C2=Y0 for
first pixel. C2=Y1 for second pixel. C1=converted Cr and C0=converted Cb
for both pixels.

Data Flow
---------

The process and the routing of the alpha and color channels can be seen in
the following diagram. The diagram shows the main process as well as showing
what state each of the blocks control. The five blocks of the graphics
compositor process are named source, destination, blend, pattern, and output.

{image:graphicscompositor overview}

Main Surface and Alpha Surface
------------------------------

You will notice that for both the Source and Destination blocks, two surfaces
can be provided for each block. One of these surfaces is the main surface,
simply referred to as "Surface". The other surface is the "Alpha Surface".
In general, most users will only be using main surface. This surface supports
any of the standard pixel formats. The only time where the alpha surface is
being used is in one very specific case. The main surface must have a format
of BPXL_R5_G6_B5 and the alpha surface must have a format of BPXL_A1 or BPXL_W1.
These two surfaces combined will describe a 17-bit format where the color
channel information will come from the main surface and the alpha information
will come from the alpha surface.

Color Keying
------------

In addition to the four color channel values stored for each pixel, there is
one additional bit of information. This is the color key bit which is set in the
color keying state in the source and destination blocks. The operation to set
this bit is enabled with BGRC_Source_ToggleColorKey and
BGRC_Destination_ToggleColorKey. The parameters for the color key are set
with calls to BGRC_Source_SetColorKey and BGRC_Source_SetColorKey.

The parameters that you can set allow you to define a min/max range of the color
key, and a mask which can be used to mask out the value of the input pixel
before the comparison is made. If the pixel passes the color key test, the
color key bit for that pixel is marked and if any bit in the replacement mask
is set, the corresponding bit in the replacement color is used to replace the
input color.

The additional color key bit is used later on in the output block in the
procedure defined by the BGRC_Output_SetColorKeySelection. This section determines
which pixel will be the final result by looking at the color key bit for
the source and destination. Depending on those four possible combinations,
the final pixel can come from the source block, the destination block, the
blend block, or the pattern block.

Color keying is typically used when a specific color value in the source needs
to be replaced by the corresponding pixel in the destination. To get this
effect, set the source color key min, max, and mask to appropriate values.
Remember that if your mask contains zeros, you must adjust your min and max
ranges to allow a match when those input bits are masked. In addition to the
source color key parameters you must also tell the output
color key selection to choose the pixel from the destination when the source
color key is on. With this final setting you will be able to get source color
keying working.

In this typical case, there is no real need to specify a replacement color
and replacement mask since the purpose here is to replace the pixel with one
from the destination and not replace it with a constant color.

One real tricky case with color keying is when you're combining it with
scaling. The issue is centered around the additional color key bit. This bit
along with the corresponding four color channels is fed into the scaler.
The scaler must then produce four new color channel values as well as a new
color key bit. The color key bit will be set only if all the contributors to
that pixel have the color key bit set. The issue this causes can be seen by
noticing that if half the pixels entering the scaler are color keyed and
the rest are not, the result will be marked as not color keyed even though
the color key color was used by the scaler. This will mean that the color
used for the color key can be spread outside the color key range and those
pixels will not be marked as color keyed.

There are several ways to lessen the impact of this problem. You can use
the replacement mask and replacement color to replace the input color key with
a color like black that doesn't stand out when filtered with neighboring
pixels. If you need to re-introduce this color after the scaler, like if
you're planning to use it in a color key operation in the display, you can do
so by providing the color key value as the destination constant color and using
the output selection to choose the destination color when source color keying
is enabled.

Another way to lessen the impact is to choose different filter coefficients
for the scaler with a call to BGRC_Source_SetFilterCoeffs. This can be used
to lessen the spread of the color key value outside of the color key area.

Source Block
------------

The source block is responsible for defining the one of the main inputs to
the compositor. In a typical situation where a graphic surface is being
composited into a frame buffer surface, the source block represents the
graphic surface and not the frame buffer.

The functions used to define the source block are the following:

o Surface: BGRC_Source_SetSurface,
  BGRC_Source_SetRectangle
o Alpha Surface: BGRC_Source_SetRectangle,
  BGRC_Source_SetAlphaSurface
o Constant Color: BGRC_Source_SetColor
o Chroma Expansion: BGRC_Source_SetChromaExpansion
o Palette Dereference: BGRC_Source_TogglePaletteBypass
o Per-pixel (A,C2,C1,C0) value: BGRC_Source_SetZeroPad
o Color Keying and Color Matrix operation: BGRC_Source_SetColorKey,
  BGRC_Source_ToggleColorMatrix, BGRC_Source_ToggleColorKey,
  BGRC_Source_SetColorMatrix5x4,
  BGRC_Source_SetColorMatrixRounding,
  BGRC_Source_SetKeyMatrixOrder
o Scaling: BGRC_Source_SetScaleAlphaAdjust,
  BGRC_Source_SetFilterCoeffs, BGRC_Source_SetScaleRounding,
  BGRC_Source_SetScaleEdgeCondition
o General: BGRC_Source_SetDefault, BGRC_Source_Reset

Destination Block
-----------------

The destination block is responsible for defining the one of the main
inputs to the compositor. In a typical situation where a graphic surface
is being composited into a frame buffer surface, the destination block
represents the frame buffer and not the graphic surface.

In general, all the capabilities of the source block are found in the
destination block. The only exception to this is that you cannot
scale an input specified as a destination.

The functions used to define the destination block are the following:

o Surface: BGRC_Destination_SetSurface,
  BGRC_Destination_SetRectangle
o Alpha Surface: BGRC_Destination_SetRectangle,
  BGRC_Destination_SetAlphaSurface
o Constant Color: BGRC_Destination_SetColor
o Chroma Expansion: BGRC_Destination_SetChromaExpansion
o Palette Dereference: BGRC_Destination_TogglePaletteBypass
o Per-pixel (A,C2,C1,C0) value: BGRC_Destination_SetZeroPad
o Color Keying and Color Matrix operation: BGRC_Destination_SetColorKey,
  BGRC_Destination_ToggleColorMatrix, BGRC_Destination_ToggleColorKey,
  BGRC_Destination_SetColorMatrix5x4,
  BGRC_Destination_SetColorMatrixRounding,
  BGRC_Destination_SetKeyMatrixOrder
o General: BGRC_Destination_SetDefault, BGRC_Destination_Reset

Blend Block
-----------

The purpose of the blend block is to blend the results from the source
block with the results from the destination block and provide that value
as an input the output block.

The functions used to define the blend block are the following:

o Constant Color: BGRC_Blend_SetColor
o Color and Alpha Blending Operations: BGRC_Blend_SetAlphaBlend,
  BGRC_Blend_SetColorBlend
o General: BGRC_Blend_SetDefault, BGRC_Blend_Reset

Pattern Block
-------------

The purpose of the pattern block is to provide pattern operations using
the results from the source block and the destination block and provide
that value as an input the output block.

The functions used to define the pattern block are the following:

o Pattern Operation: BGRC_Pattern_Set
o General: BGRC_Pattern_SetDefault, BGRC_Pattern_Reset

Output Block
------------

Results from the other four blocks are provided as inputs to this block.
The purpose of this block is to choose which inputs to use as the final
result, to convert the (A,C2,C1,C0) value into the final color space,
and to specify where the output should be stored.

The functions used to define the output block are the following:

o Output Selection: BGRC_Output_SetColorKeySelection
o Color Matrix: BGRC_Output_SetColorMatrixRounding
o Dither: BGRC_Output_SetDither
o Surface: BGRC_Output_SetRectangle,
  BGRC_Output_SetSurface
o Alpha Surface: BGRC_Output_SetAlphaSurface,
  BGRC_Output_SetRectangle
o General: BGRC_Output_SetDefault, BGRC_Output_Reset

Usage
-----

o User creates handle using BGRC_Open.
o User sets state using the source, destination, blend, pattern,
  and output blocks.
o User issues the state using BGRC_IssueState or
  BGRC_IssueStateAndWait.

It is important to note that until the operation is completed, the user should
not modify any input pixels or input palettes. Doing so will result in an
undefined result.
****************************************************************************/

/***************************************************************************
Summary:
    The handle for the graphics compositor module.
****************************************************************************/
typedef struct BGRC_P_Handle *BGRC_Handle;

/***************************************************************************
Summary:
    Callback used for task completion.

Description:
    When a task is issued, this callback can be used to get notification
    when the task is completed.

See Also:
    BGRC_IssueState
****************************************************************************/
typedef BERR_Code (*BGRC_Callback)(BGRC_Handle hGrc, void *pData);

/***************************************************************************
Summary:
    Specifies what coefficients to use for scaling.

    The following is a description of the filter types:

    Point Sample: Single closest pixel (1 tap) is used, resulting in
    no filtering. Generates blocky up-scales, and down-scales with missing
    pixel information.

    Bilinear: Closest pixel is filtered with its neighbor (2 taps) using
    coefficients dervied from the pixel position. Generates slightly blurry
    up-scales, and down-scales of less than a half will have missing pixel
    information.

    Anisotropic: Closest pixel is area filtered with its neighbors
    (up to 8 taps). This filter is the same as bilinear when up-scaling.
    When down-scaling, the amount of taps used depends on the scale factor.

    Sharp: Closet pixel is area filtered with its neighbors using filter
    coefficents generated with a sin wave formula (all 8 taps). Each scale
    factor has its own table of filter coefficients up to a scale of 8 to 9.
    Up-scales larger than 8 to 9 use a 1 to N coefficent table. (Default)

    Sharper: Same as sharp, except when scaling larger than 8 to 9, a
    sharper filter coefficent table is used.

    Blurry: Closest pixel is filtered with two closet neighbors (3 tap).
    Similar to bilinear except blurryer.

See Also:
    BGRC_Source_SetFilterCoeffs
****************************************************************************/
typedef enum BGRC_FilterCoeffs
{
    BGRC_FilterCoeffs_ePointSample,      /* Point sampled filtering */
    BGRC_FilterCoeffs_eBilinear,         /* Bilinear filtering */
    BGRC_FilterCoeffs_eAnisotropic,      /* Anisotropic filtering */
    BGRC_FilterCoeffs_eSharp,            /* Tabled sin(x)/x filtering */
    BGRC_FilterCoeffs_eSharper,          /* Sharper upscale */
    BGRC_FilterCoeffs_eBlurry,           /* 3-point filtering */
    BGRC_FilterCoeffs_eAntiFlutter,      /* Anti-Flutter filtering */
    BGRC_FilterCoeffs_eAntiFlutterBlurry,/* Blurry Anti-Flutter filtering */
    BGRC_FilterCoeffs_eAntiFlutterSharp, /* Sharp Anti-Flutter filtering */
    BGRC_FilterCoeffs_eMax
} BGRC_FilterCoeffs;

/***************************************************************************
Summary:
    Specifies what source to use for each individual piece of the
    blending equation.
****************************************************************************/
typedef enum BGRC_Blend_Source
{
    BGRC_Blend_Source_eZero,                    /* Zero */
    BGRC_Blend_Source_eHalf,                    /* 1/2 */
    BGRC_Blend_Source_eOne,                     /* One */
    BGRC_Blend_Source_eSourceColor,             /* Color from source */
    BGRC_Blend_Source_eInverseSourceColor,      /* 1-color from source */
    BGRC_Blend_Source_eSourceAlpha,             /* Alpha from source */
    BGRC_Blend_Source_eInverseSourceAlpha,      /* 1-alpha from source */
    BGRC_Blend_Source_eDestinationColor,        /* Color from destination */
    BGRC_Blend_Source_eInverseDestinationColor, /* 1-color from destination */
    BGRC_Blend_Source_eDestinationAlpha,        /* Alpha from destination */
    BGRC_Blend_Source_eInverseDestinationAlpha, /* 1-alpha from destination */
    BGRC_Blend_Source_eConstantColor,           /* Color from blend color */
    BGRC_Blend_Source_eInverseConstantColor,    /* 1-color from blend color */
    BGRC_Blend_Source_eConstantAlpha,           /* Alpha from blend color */
    BGRC_Blend_Source_eInverseConstantAlpha     /* 1-alpha from blend color */

} BGRC_Blend_Source;

/***************************************************************************
Summary:
    Used to determine what value to take depending on the color keying
    results.
****************************************************************************/
typedef enum BGRC_Output_ColorKeySelection
{
    BGRC_Output_ColorKeySelection_eTakeSource,       /* Take source pixel */
    BGRC_Output_ColorKeySelection_eTakeBlend,        /* Take blended pixel */
    BGRC_Output_ColorKeySelection_eTakeDestination,  /* Take destination pixel */
    BGRC_Output_ColorKeySelection_eTakePattern       /* Take pattern pixel */
} BGRC_Output_ColorKeySelection;

/***************************************************************************
Summary:
    This structure describes the capabilities of a grc
***************************************************************************/
typedef struct BGRC_Capabilities
{
    uint32_t ulMaxHrzDownSclRatio;
    uint32_t ulMaxVerDownSclRatio;
} BGRC_Capabilities;

/***************************************************************************
Summary:
    This structure describes the grc is a m2mc or mipmap
***************************************************************************/
typedef enum BGRC_Mode
{
    BGRC_eBlitter,
    BGRC_eMipmap,
    BGRC_eMax
} BGRC_Mode;

/***************************************************************************
Summary:
    This structure describes the standby settings

Description:
    BGRC_StandbySettings describes the standby settings

    bS3Standby - whether S3 standby

See Also:
    BGRC_Standby, BGRC_Resume
***************************************************************************/
typedef struct BGRC_StandbySettings
{
    bool bS3Standby;
} BGRC_StandbySettings;

/***************************************************************************
Summary:
    This structure describes some initial settings when opening the
    graphics compositor.

Description:
    This structure is used to initialize some private settings within the
    driver. If this structure is not used when opening the driver, the
    driver will use its own default settings.

    ulPacketMemoryMax - Maximum memory threshold for register set allocations
                       that are placed in the M2MC operation queue. The
                       driver will allocate past this threshold but will
                       deallocte to the threshold when the memory becomes
                       idle. Setting this to 0 will allow for unlimited
                       packet memory allocations. The default value is
                       BGRC_PACKET_MEMORY_MAX defined below.
    ulOperationMax    - Maximum amount of operation structures allowed to be
                       stored before deallocations begin. These structures
                       are used by the driver's interrupt handler to track
                       M2MC operations. It will allocate past this threshold
                       but will deallocate to the threshold when the
                       structures become idle. Setting this to 0 will allow
                       for unlimited operation structure allocations. The
                       default value is BGRC_OPERATION_MAX defined below.
    ulDeviceNum      - M2MC Device number. 0 is default, and only 7400 and
                       7438 can set this value to 0 or 1.
    ulWaitTimeout    - How many seconds to wait before the device times out,
                       and the driver assumes the device is hung. The default
                       is 10 seconds.
    bPreAllocMemory  - Allocate all memory specified by ulPacketMemoryMax
                       when opening module. An error will be returned
                       when memory is exceeded. Defaults to true.

See Also:
    BGRC_Open
****************************************************************************/
typedef struct BGRC_Settings
{
    uint32_t ulPacketMemoryMax;      /* max packet memory */
    uint32_t ulOperationMax;         /* max operations */
    uint32_t ulDeviceNum;            /* M2MC device number */
    uint32_t ulWaitTimeout;          /* seconds to wait before timing out */
    bool bPreAllocMemory;            /* allocate memory when opening module */
} BGRC_Settings;

#define BGRC_PACKET_MEMORY_MAX    131072
#define BGRC_OPERATION_MAX          2048
#define BGRC_WAIT_TIMEOUT             10

/***************************************************************************
Summary:
    Gets default settings.
****************************************************************************/
void BGRC_GetDefaultSettings(
    BGRC_Settings *pDefSettings
);

/***************************************************************************
Summary:
Get the grc's capabilities.
**************************************************************************/
void BGRC_GetCapabilities
    ( BGRC_Handle                      hGrc,
      BGRC_Capabilities               *pCapabilities );

/***************************************************************************
Summary:
Get the grc's mipmap support character.
**************************************************************************/
BGRC_Mode BGRC_GetMode_isrsafe
    ( uint32_t                         ulDeviceIdx);
/***************************************************************************
Summary:
    Opens the graphics compositor module.

Description:
    The module is opened and a graphics compositor module handle is
    created and returned. This handle will be necessary to perform any
    tasks in the graphics compositor module.

    Once opened, all state for each of the sub-modules will be reset as
    if a call to BGRC_ResetState was made.

Returns:
    BERR_SUCCESS - Module was opened and valid handle was returned.
    BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
    BGRC_Close
****************************************************************************/
BERR_Code BGRC_Open(
    BGRC_Handle *phGrc,                /* [out] Pointer to returned GRC
                                          handle. */
    BCHP_Handle hChip,                 /* [in] Chip handle. */
    BREG_Handle hRegister,             /* [in] Register access handle. */
    BMMA_Heap_Handle hMemory,          /* [in] Memory allocation handle. */
    BINT_Handle hInterrupt,            /* [in] Interrupt handle. */
    const BGRC_Settings *pDefSettings  /* [in] Pointer to initial settings. */
);

/***************************************************************************
Summary:
    Closes the graphics compositor module.

Description:
    Once this function is called no more graphics compositor module
    functions can be used.

    Outstanding tasks may be interrupted without callbacks being
    called.

See Also:
    BGRC_Open
****************************************************************************/
void BGRC_Close(
    BGRC_Handle hGrc                   /* [in] GRC module handle to close. */
);

/*****************************************************************************
Summary:
    Get default standby settings.

See Also:
    BGRC_Standby
*****************************************************************************/
void BGRC_GetDefaultStandbySettings
    ( BGRC_StandbySettings            *pStandbypSettings );

/*****************************************************************************
Summary:
    Enter standby mode

Description:
    This function enters standby mode with the GRC module, if supported.
    All packets in all conexts created with this GRC must have been processed
    and completed by HW in order to successfully enter standby mode.
    If standby mode is not supported, calling this function has no effect.

    When in standby mode, the device is pwred off.

    No BGRC_* calls should be made until standby mode is exitted by calling
    BGRC_Resume().  Calling BGRC_* api at while in standy result in
    undefined results.

Returns:
    BERR_SUCCESS - If standby is successful, otherwise error

See Also:
    BGRC_Resume
*****************************************************************************/
BERR_Code BGRC_Standby
    ( BGRC_Handle                      hGrc,
      const BGRC_StandbySettings      *pStandbySettings );

/*****************************************************************************
Summary:
    Exit standby mode

Description:
    This function exits from standby mode with the GRC module, if supported.
    After exitting standby mode, upper-level SW is free to call
    BGRC_* functions.

Returns:
    BERR_SUCCESS - If standby is successful, otherwise error

See Also:
    BGRC_Standby
*****************************************************************************/
BERR_Code BGRC_Resume
    ( BGRC_Handle                      hGrc );

BERR_Code BGRC_SetSecureMode(
    BGRC_Handle hGrc,
    bool secure
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGRC_H__ */

/* end of file */
