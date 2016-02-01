/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
 ***************************************************************************/

#ifndef BGRC_H__
#define BGRC_H__

#include "bchp.h"
#include "bmem.h"
#include "bsur.h"
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

Test Plan
---------

	(I) - Test implemented.
	(F) - Test failed.
	(P) - Test passed.
	(?) - Questions about implementation.

	* Feeders: Responsible for fetching graphics data from a specified format.
		* Format Definition: Describes input and output pixel formats.
			* (P) Blit RGB formats to RGB formats.
			* (P) Blit YCrCb formats to YCrCb formats.
			* (F) Blit Alpha formats to Alpha formats.
			* (F) Blit Alpha formats to RGB formats.
			* (F) Blit Alpha formats to YCrCb formats.
			* (F) Blit RGB formats to Alpha formats.
			* (F) Blit YCrCb formats to Alpha formats.
		* Palette: Use fetched pixel as an index into color lookup table.
			* (P) Blit Palette formats to same Palette formats bypassing table lookup.
			* (P) Blit Palette formats to RGB formats with table lookup.
			* (P) Blit Palette formats to YCrCb formats with table lookup.
		* YCrCb Filtering: Expands chroma values using filtering.
			* (F) Blit YCrCb422 to AYCrCb444 with filtering.
		* Blit Direction: Feeders can fetch data in any direction.
			* (P) Blit RGB format from top-to-bottom and left-to-right.
			* (P) Blit RGB format from bottom-to-top and left-to-right.
			* (P) Blit RGB format from top-to-bottom and right-to-left.
			* (P) Blit RGB format from bottom-to-top and right-to-left.
		* Constant Pixel: Replaces fetched pixel from surface.
			* (P) Blit constant pixel to RGB and YCrCb formats.

	* Color Key: Allows replacement of the pixels fetched by the feeder.
		* Single or Range
			* (P) Blit RGB and YCrCb formats using single color key.
			* (P) Blit RGB and YCrCb formats using ranged color key.

	* Color Matrix: Multiplies pixels fetched by the feeder with a matrix.
		* Color Conversion: Matrix can be used to convert to a new color space.
			* (P) Blit RGB formats to YCrCb formats.
			* (P) Blit YCrCb formats to RGB formats.

	* Blend: Blends pixels using formula with specified inputs.
		* Standard Blends: Several blends commonly supported by APIs.
			* (P) Blit RGB and YCrCb formats using One/Zero blend.
			* (P) Blit RGB and YCrCb formats using Zero/One blend.
			* (P) Blit RGB and YCrCb formats using One/One blend.
			* (P) Blit RGB and YCrCb formats using SrcAlpha/InvSrcAlpha blend.
			* (P) Blit RGB and YCrCb formats using One/InvSrcAlpha blend.
			* (P) Blit RGB and YCrCb formats using DstColor/One blend.

	* ROP: Logically combines input pixels.
		* Standard ROPs: Simple ROPs using source and destination.
			* (P) Blit RGB and YCrCb formats using 'copy' ROP.
			* (P) Blit RGB and YCrCb formats using 'and' ROP.
			* (P) Blit RGB and YCrCb formats using 'or' ROP.
			* (P) Blit RGB and YCrCb formats using 'xor' ROP.
		* Pattern ROPs: ROPs using pattern as a source.
			* (P) Blit RGB and YCrCb formats using several pattern ROPs.

	* Scaler: Source surface can be scaled to output size.
		* Rounding Mode: Least significant bit handling.
			* Blit RGB format using truncate rounding.
			* Blit RGB format using nearest rounding.
		* Edge Condition: Filling of missing taps on edges.
			* Blit RGB format using using replicate edge condition.
			* Blit RGB format using using mirror edge condition.
		* Scaler Order: Determines order of horizontal and vertical scales.
			* Blit RGB format using horizontal then vertical scale.
			* Blit RGB format using vertical then horizontal scale.
		* Averagers and Filters: Produce filtered pixels during scale.
			* Blit RGB format using horizontal averagers and filters.
			* Blit RGB format using vertical averager.
			* Blit RGB format using horizontal and vertical averagers and filters.

See Also:
	BGRC_Open,
	BSUR_Surface_Create
****************************************************************************/

/***************************************************************************
Summary:
	The handle for the graphics compositor module.

Description:
	This is the main handle required to do any operations within the 
	graphics compositor module.

See Also:
	BGRC_Open
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
	Used to specify how values are rounded for the final result.

Description:

See Also:
	BGRC_Source_SetChromaExpansion,
	BGRC_Destination_SetChromaExpansion
****************************************************************************/
typedef enum BGRC_ChromaExpansion
{
	BGRC_ChromaExpansion_eReplicate,  /* Cr and Cb values are replicated */
	BGRC_ChromaExpansion_eFilter      /* Cr and Cb values are filtered */
} BGRC_ChromaExpansion;

/***************************************************************************
Summary:
	Used to specify how values are rounded for the final result.

Description:

See Also:
	BGRC_Source_SetColorMatrixRounding,
	BGRC_Destination_SetColorMatrixRounding,
	BGRC_Output_SetColorMatrixRounding,
	BGRC_Source_SetScaleRounding
****************************************************************************/
typedef enum BGRC_Rounding
{
	BGRC_Rounding_eTruncate,  /* Values are truncated */
	BGRC_Rounding_eNearest    /* Values are rounded to nearest */
} BGRC_Rounding;

/***************************************************************************
Summary:
	Used to determine whether color keying comes before or after the color
	matrix operation.

Description:

See Also:
	BGRC_Source_SetKeyMatrixOrder,
	BGRC_Destination_SetKeyMatrixOrder
****************************************************************************/
typedef enum BGRC_KeyMatrixOrder
{
	BGRC_KeyMatrixOrder_eKeyThenMatrix, /* Color keying operation
	                                       comes before color matrix. */
	BGRC_KeyMatrixOrder_eMatrixThenKey  /* Color matrix operation
	                                       comes before color keying. */
} BGRC_KeyMatrixOrder;

/***************************************************************************
Summary:
	Used to determine the order of color keying, color matrix, and scaling
	when processing pixels.

Description:

See Also:
	BGRC_Source_SetKeyMatrixScaleOrder,
****************************************************************************/
typedef enum BGRC_KeyMatrixScaleOrder
{
	BGRC_KeyMatrixScaleOrder_eScaleThenKeyThenMatrix, /* Scale-Key-Matrix */
	BGRC_KeyMatrixScaleOrder_eScaleThenMatrixThenKey, /* Scale-Matrix-Key */
	BGRC_KeyMatrixScaleOrder_eKeyThenMatrixThenScale, /* Key-Matrix-Scale */
	BGRC_KeyMatrixScaleOrder_eKeyThenScaleThenMatrix, /* Key-Scale-Matrix */
	BGRC_KeyMatrixScaleOrder_eMatrixThenScaleThenKey, /* Matrix-Scale-Key */
	BGRC_KeyMatrixScaleOrder_eMatrixThenKeyThenScale, /* Matrix-Key-Scale */
	BGRC_KeyMatrixScaleOrder_eBypassAll               /* None */
} BGRC_KeyMatrixScaleOrder;

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
	BGRC_FilterCoeffs_eAntiFlutterSharp  /* Sharp Anti-Flutter filtering */
} BGRC_FilterCoeffs;

/***************************************************************************
Summary:
	Specifies what to do when an edge condition is reached.

Description:

See Also:
	BGRC_Source_SetScaleEdgeCondition
****************************************************************************/
typedef enum BGRC_EdgeCondition
{
	BGRC_EdgeCondition_eReplicateLast,  /* Last pixel is replicated */
	BGRC_EdgeCondition_eMirror          /* Mirrored along edge */
} BGRC_EdgeCondition;

/***************************************************************************
Summary:
	Specifies what source to use for each individual piece of the 
	blending equation.

Description:

See Also:
	BGRC_Blend_SetColorBlend,
	BGRC_Blend_SetAlphaBlend
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

Description:

See Also:
	BGRC_Output_SetColorKeySelection
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
	Used to determine range type when using a YCbCr 420 source surface.

See Also:
	BGRC_Output_SetColorKeySelection
****************************************************************************/
typedef enum BGRC_MacroBlockRange
{
	BGRC_MacroBlockRange_None,       /* Disable */
	BGRC_MacroBlockRange_Expansion,  /* MacroBlock range expansion */
	BGRC_MacroBlockRange_Remapping   /* MacroBlock range remapping */
} BGRC_MacroBlockRange;

/***************************************************************************
Summary:
	This structure describes the capabilities of a grc

Description:
	BGRC_Capabilities is a structure that contains the capabilities of grc.

	ulMaxHrzDownSclRatio - max of horizontal down scale ratio

	ulMaxVerDownSclRatio - max of vertical down scale ratio

See Also:
	BGRC_GetCapabilities
***************************************************************************/
typedef struct BGRC_Capabilities
{
	uint32_t ulMaxHrzDownSclRatio;
	uint32_t ulMaxVerDownSclRatio;
} BGRC_Capabilities;

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

Description:
	This function returns that default settings which can be modified
	when calling BGRC_Open. These settings are defined above.

Returns:
	BERR_SUCCESS - Default settings returned.

See Also:
	BGRC_Open
****************************************************************************/
BERR_Code BGRC_GetDefaultSettings(
	BGRC_Settings *pDefSettings 
);

/***************************************************************************
Summary:
	Get the grc's capabilities.

Description:
	This function gets the grc's capabilities.

input:
	hGrc - A valid grc handle created earlier.

Output:
	pCapabilities - point to the capabilities structure.

See Also:
	BGRC_Capabilities
**************************************************************************/
void BGRC_GetCapabilities
	( BGRC_Handle                      hGrc,
	  BGRC_Capabilities               *pCapabilities );

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
	BMEM_Handle hMemory,               /* [in] Memory allocation handle. */
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

/***************************************************************************
Summary:
	Resets all state back to default values.

Description:
	This call does not make any changes in hardware, nor does it
	effect any outstanding operations.

	This is implemented by calling reset functions for source,
	destination, blend, pattern, and output.

Returns:
	BERR_SUCCESS - Module state was successfully reset.

See Also:
	BGRC_Open,
	BGRC_Source_ResetState,
	BGRC_Destination_ResetState,
	BGRC_Blend_ResetState,
	BGRC_Pattern_ResetState,
	BGRC_Output_ResetState
****************************************************************************/
BERR_Code BGRC_ResetState(
	BGRC_Handle hGrc                   /* [in] Handle to GRC module. */
);

/***************************************************************************
Summary:
	Issues state to hardware.

Description:
	When this call is made, all the state specified in the source,
	destination, blend, pattern, and output blocks will be 
	checked to make sure it is valid. If it is valid, it will be
	combined together to form one task which will then be issued
	to hardware.

    Hardware will work on this task asyncronously. The user has
	the option of waiting for the task to complete by using the function
	BGRC_IssueStateAndWait, having the module call a callback function
	when complete using BGRC_IssueState, or not be notified when
	this task is complete.

	Tasks issued to hardware will be completed in the order given.
	For example, you could issue five tasks without task completion
	notification and then wait on the sixth. When the sixth
	completes then all six tasks are guaranteed to have completed.

	Callbacks will be called from an interrupt handler and must
	follow the calling restrictions of interrupt handlers.

Returns:
	BERR_SUCCESS - Task was issued.

See Also:
	BGRC_IssueStateAndWait
****************************************************************************/
BERR_Code BGRC_IssueState(
	BGRC_Handle   hGrc,           /* [in] Handle to GRC module. */
	BGRC_Callback pCallback,      /* [in] Pointer to completion callback. */
	void         *pData           /* [in] Pointer to callback user data. */
);

/***************************************************************************
Summary:
	Issues state to hardware and wait for completion.

Description:
	When this call is made, all the state specified in the source,
	destination, blend, pattern, and output blocks will be 
	checked to make sure it is valid. If it is valid, it will be
	combined together to form one task which will then be issued
	to hardware.

    Hardware will work on this task asyncronously. The user has
	the option of waiting for the task to complete by using the function
	BGRC_IssueStateAndWait, having the module call a callback function
	when complete using BGRC_IssueState, or not be notified when
	this task is complete.

	Tasks issued to hardware will be completed in the order given.
	For example, you could issue five tasks without task completion
	notification and then wait on the sixth. When the sixth
	completes then all six tasks are guaranteed to have completed.

	This specific function does not use a callback but waits for
	the given task to complete.

Returns:
	BERR_SUCCESS - Task was issued and finished.

See Also:
	BGRC_IssueState
****************************************************************************/
BERR_Code BGRC_IssueStateAndWait(
	BGRC_Handle    hGrc                /* [in] Handle to GRC module. */
);

/***************************************************************************
Summary:
	Checks if device operations have completed.

Description:
	This function will check the hardware registers to determine if all
	previous device operations issued through BGRC_IssueState() have 
	completed. If pending operations are complete, then true will be 
	returned in the pbComplete parameter, otherwise false will be returned.

Returns:
	BERR_SUCCESS - Task was issued and finished.

See Also:
	BGRC_IssueState
****************************************************************************/
BERR_Code BGRC_OperationsComplete(
	BGRC_Handle hGrc,                  /* [in] Handle to GRC module. */
	bool *pbComplete                   /* [out] Pointer to return value. */
);

/***************************************************************************
Summary:
	Wait until operations are ready.

Description:
	This function will wait until there are memory resources available to
	perform another blit. If a callback function is provided then this 
	function will return immediately, and call the callback function when
	memory resources are available.

Returns:
	BERR_SUCCESS - Task was issued and finished.

See Also:
	BGRC_WaitForOperationsComplete
****************************************************************************/
BERR_Code BGRC_WaitForOperationReady(
	BGRC_Handle   hGrc,
	BGRC_Callback pCallback_isr,
	void         *pData
);

/***************************************************************************
Summary:
	Wait until operations are complete.

Description:
	This function will wait until all operations are complete. If a callback 
	function is provided then this function will return immediately, and 
	call the callback function when	all operations are complete.

Returns:
	BERR_SUCCESS - Task was issued and finished.

See Also:
	BGRC_WaitForOperationReady
****************************************************************************/
BERR_Code BGRC_WaitForOperationsComplete(
	BGRC_Handle   hGrc,
	BGRC_Callback pCallback_isr,
	void         *pData
);

/***************************************************************************
Summary:
	Sets the main surface for the source.

Description:
	This is used in conjunction with the alpha surface specified
	with BGRC_Source_SetAlphaSurface to describe the source pixels.

	This surface is not required. As a default, this surface is not
	specified.

Returns:
	BERR_SUCCESS - Main surface was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Source_SetAlphaSurface,
****************************************************************************/
BERR_Code BGRC_Source_SetSurface(
	BGRC_Handle hGrc,                  /* [in] Handle to GRC module. */
	BSUR_Surface_Handle hSurface       /* [in] Main surface handle. */
);

/***************************************************************************
Summary:
	Sets the alpha surface for the source

Description:
	This is used in conjunction with the main surface specified with
	BGRC_Source_SetSurface to describe the source pixels.

	Currently, this can only be used with a main surface having a pixel
	format of BPXL_eR5_G6_B5 and an alpha surface having a pixel format
	of BPXL_eA1 or BPXL_eW1.

	This surface is not required. As a default, this surface is not
	specified.

Returns:
	BERR_SUCCESS - Alpha surface was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_SetSurface
****************************************************************************/
BERR_Code BGRC_Source_SetAlphaSurface(
	BGRC_Handle hGrc,                     /* [in] Handle to GRC module. */
	BSUR_Surface_Handle hSurface          /* [in] Alpha surface handle. */
);

/***************************************************************************
Summary:
	Enables or disables the palette index bypass.

Description:
    Typically when a source contains a palette, the palette index
	is used to get color channel information from an actual palette.
	However, the user may decide to override this behavior and
	pass through the index as is. This function is used to 
	enable and disable this state.

	If this state is enabled, C2 and C1 will be set to zero and C0 will
	contain the index value. 

    The default is for the palette index to be used as a reference
	to a palette and not to use the palette index.

Returns:
	BERR_SUCCESS - Palette bypass was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_SetSurface
****************************************************************************/
BERR_Code BGRC_Source_TogglePaletteBypass(
	BGRC_Handle hGrc,               /* [in] Handle to GRC module. */
	bool bEnableBypass              /* [in] If true, index will be used
									   directly. If false, the palette will
									   be referenced. */
);

/***************************************************************************
Summary:
	Controls the chroma expansion for compressed YCrCb formats.

Description:
    When a compressed YCrCb format is used as an input, say
	BPXL_eY08_Cb8_Y18_Cr8, the pixels must first be uncompressed
	before they are converted. This function controls how the 
	pixels are uncompressed.

	As a default, eChromaExpansion will have a value of
	BGRC_ChromaExpansion_eReplicate.

Returns:
	BERR_SUCCESS - Alpha surface was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_SetSurface
****************************************************************************/
BERR_Code BGRC_Source_SetChromaExpansion(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	BGRC_ChromaExpansion eChromaExpansion  /* [in] Chroma expansion type. */
);

/***************************************************************************
Summary:
	Enables zero padding or bit replication.

Description:
	When the source feeder reads pixels, if the pixel components are less 
	than 8 bits in size, it can either zero pad the low order bits, or 
	replicate the high order bits into the low order bits.

    The default is false (replication).

Returns:
	BERR_SUCCESS - Palette bypass was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_SetSurface
****************************************************************************/
BERR_Code BGRC_Source_SetZeroPad(
	BGRC_Handle hGrc,                /* [in] Handle to GRC module. */
	bool bEnableZeroPad              /* [in] If true, zero padding will be
									    used. If false, bit replication
										will be used. */
);

/***************************************************************************
Summary:
	Sets color key values for the source.

Description:
	When called, the source color key state is set to use the given values.
	When color keying is enabled with BGRC_Source_ToggleColorKey,
	the values are used as follows:

	o Input color channels are masked with ulMask. The minimum
	  (ulMin) and maximum (ulMax) range values are also masked.
	o If each of the components is less than or equal to the masked
	  maximum and greater than or equal to the masked minimum, then the
	  value is considered to be color keyed. Otherwise, it is not color
	  keyed.
	o If the value is color keyed, Any bits of the result that have
	  a corresponding bit set in ulReplacementMask are replaced
	  with the bit value in ulReplacement.

	When setting bExclusive to true, the above compares become less than
	instead of less equal, and greater than instead of greater equal.

	When using ARGB, the color format of the parameters are 
	BPXL_eA8_R8_G8_B8, and when using AYCbCr, the formats are 
	BPXL_eA8_Y8_Cb8_Cr8.

	When a pixel is or isn't chroma keyed it effects the final output
	selection specified with the function BGRC_Output_SetColorKeySelection.

	The default is for all values to be set to zero, and bExclusive set 
	to false.

Returns:
	BERR_SUCCESS - Color key was enabled and set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Source_ToggleColorKey,
	BGRC_Source_SetKeyMatrixOrder,
	BGRC_Output_SetColorKeySelection,
****************************************************************************/
BERR_Code BGRC_Source_SetColorKey(
	BGRC_Handle hGrc,                     /* [in] Handle to GRC module. */
	uint32_t ulMin,                       /* [in] Minimum color key value. */
	uint32_t ulMax,                       /* [in] Maximum color key value. */
	uint32_t ulMask,                      /* [in] Color key mask. */
	uint32_t ulReplacement,               /* [in] Replacement value. */
	uint32_t ulReplacementMask,           /* [in] Replacement mask. */
	bool bExclusive                       /* [in] Inclusive/exclusive
	                                         compare. */
);

/***************************************************************************
Summary:
	Toggles the state of color keying.

Description:
	This function is used to enable and disable the state of color keying.
	When enabled, the values and process described in 
	BGRC_Source_SetColorKey will be used. When disabled, all pixels
	are marked as not being color keyed. This effects the final output
	selection, specified with the function
	BGRC_Output_SetColorKeySelection.

	The default is for color keying to be disabled.
	
Returns:
	BERR_SUCCESS - Color key was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Source_SetColorKey,
	BGRC_Source_SetKeyMatrixOrder,
	BGRC_Output_SetColorKeySelection,
****************************************************************************/
BERR_Code BGRC_Source_ToggleColorKey(
	BGRC_Handle hGrc,                /* [in] Handle to GRC module. */
	bool bEnable                     /* [in] If true, color keying is
										enabled. If false, color keying
										is disabled. */
);

/***************************************************************************
Summary:
	Sets the 5x4 color matrix for the source. 

Description:
	This matrix is used to modify the incoming source values.
	Typically this is done to convert from one color space to another.

	The values used for the color matrix are equal to the values stored 
	in the ai32_Matrix array right shifted by the ulShift value. In
	addition, negative values are converted to positive values before
	shifting and then turned back into negative values. For
	example, if a value in the matrix was 4 and the shift value was 2, then
	the final value is 1 (4>>2 = 1). If a value in the matrix was 7
	and the shift value was 3, then the final value would be 0.875.
	If a value in the matrix was -7 and the shift value was 3 then the
	final values is -0.875.

	The values within the color matrix are in the following order:

	      |  M[0]   M[1]   M[2]   M[3]   M[4] |
	      |  M[5]   M[6]   M[7]   M[8]   M[9] |
	      | M[10]  M[11]  M[12]  M[13]  M[14] |
	      | M[15]  M[16]  M[17]  M[18]  M[19] |

	This is multiplied with a column vector of 
	< C2in, C1in, C0in, Ain, 1 > which yields the following equations:

	C2out =  M[0]*C2in +  M[1]*C1in +  M[2]*C0in +  M[3]*Ain +  M[4]
	C1out =  M[5]*C2in +  M[6]*C1in +  M[7]*C0in +  M[8]*Ain +  M[9]
	C0out = M[10]*C2in + M[11]*C1in + M[12]*C0in + M[13]*Ain + M[14]
	Aout  = M[15]*C2in + M[16]*C1in + M[17]*C0in + M[18]*Ain + M[19]

	The default is to for the matrix to represent the identity.
	M[0] = M[6] = M[12] = M[18] = 1. All other values are zero.

Returns:
	BERR_SUCCESS - Color matrix was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_ToggleColorMatrix,
	BGRC_Source_SetColorMatrixRounding,
	BGRC_Source_SetKeyMatrixOrder,
****************************************************************************/
BERR_Code BGRC_Source_SetColorMatrix5x4(
	BGRC_Handle hGrc,                 /* [in] Handle to GRC module. */
	const int32_t ai32_Matrix[20],    /* [in] Array of unshifted matrix
	                                     values. */
	uint32_t ulShift                  /* [in] Matrix shift value. */
);

/***************************************************************************
Summary:
	Toggles the state of the color matrix.

Description:
	This function is used to enable and disable the color matrix.
	When enabled, the values and process described in 
	BGRC_Source_SetColorMatrix5x4 will be used. When disabled, the matrix
	values have no effect.

	The default is for the color matrix to be disabled.
	
Returns:
	BERR_SUCCESS - Color matrix was enabled and set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_SetColorMatrix5x4,
	BGRC_Source_SetKeyMatrixOrder,
****************************************************************************/
BERR_Code BGRC_Source_ToggleColorMatrix(
	BGRC_Handle hGrc,                /* [in] Handle to GRC module. */
	bool bEnable                     /* [in] If true, the color matrix is
										enabled. If false, the color matrix
										is disabled. */
);

/***************************************************************************
Summary:
	Specifies how to round the output values from the source color matrix.

Description:
	The default rounding state is BGRC_Rounding_eNearest.

	In our current hardware implementation, this setting cannot differ from
	the destination color matrix rounding setting or output color matrix
	rounding setting. The user must insure that these settings are the
	same or an error will result.

Returns:
	BERR_SUCCESS - Color matrix rounding was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Source_SetColorMatrix5x4,
	BGRC_Source_ToggleColorMatrix,
	BGRC_Destination_SetColorMatrixRounding,
	BGRC_Output_SetColorMatrixRounding
****************************************************************************/
BERR_Code BGRC_Source_SetColorMatrixRounding(
	BGRC_Handle hGrc,                   /* [in] Handle to GRC module. */
	BGRC_Rounding eMatrixRounding       /* [in] Matrix rounding selection. */
);

/***************************************************************************
Summary:
	Determines the order of the color key and matrix operation.

Description:
	Gives the user the option of doing color keying first and color 
	matrix operations second or the option of doing color matrix
	operations first and color keying second.

	The default is to use an order of 
	BGRC_KeyMatrixOrder_eKeyThenMatrix.

Returns:
	BERR_SUCCESS - Order was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_ToggleColorKey,
	BGRC_Source_ToggleColorMatrix
****************************************************************************/
BERR_Code BGRC_Source_SetKeyMatrixOrder(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	BGRC_KeyMatrixOrder eKeyMatrixOrder    /* [in] Color key/matrix order
	                                          selection. */
);

/***************************************************************************
Summary:
	Determines the order of the color key, color matrix, and scaling.

Description:
	Gives the user the option of selecting the order of color key,
	color matrix, and scaling operations. The user also has the option
	of bypassing all these operations.

	The default is to use an order of 
	BGRC_KeyMatrixScaleOrder_eKeyThenScaleThenMatrix.

Returns:
	BERR_SUCCESS - Order was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_ToggleColorKey,
	BGRC_Source_ToggleColorMatrix
****************************************************************************/
BERR_Code BGRC_Source_SetKeyMatrixScaleOrder(
	BGRC_Handle hGrc,                             /* [in] Handle to GRC. */
	BGRC_KeyMatrixScaleOrder eKeyMatrixScaleOrder /* [in] Key/matrix/scale
	                                                 order selection. */
);

/***************************************************************************
Summary:
	Sets the constant color for the source.

Description:
	This value is used when the source surfaces are missing the alpha
	or color channels. When the color is ARGB, the color format is 
	BPXL_eA8_R8_G8_B8, and when it is AYCbCr, the format is 
	BPXL_eA8_Y8_Cb8_Cr8. When the surface format is less than a byte in 
	size, the high-bits of the low-byte contains the constant color.


	The default is 0xFF000000 (Alpha is 1, color channels are 0).
  
Returns:
	BERR_SUCCESS - Constant alpha and color was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_SetSurface,
	BGRC_Source_SetAlphaSurface
****************************************************************************/
BERR_Code BGRC_Source_SetColor(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	uint32_t ulColor                       /* [in] Constant values to use. */
);

/***************************************************************************
Summary:
	Specifies the sub-rectangle of the source surface.

Description:
	Only the values within this rectangle will be used in the
	compositor calculations. The values given here must be restricted
	to the size of the surface itself.

	The source will be scaled to the size of the output
	rectangle, specified by BGRC_Output_SetRectangle.

	The default behavior is to use a rectangle the size of the entire
	specified source.

Returns:
	BERR_SUCCESS - Rectangle was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Output_SetRectangle
****************************************************************************/
BERR_Code BGRC_Source_SetRectangle(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	uint32_t ulLeft,                       /* [in] Rectangle left. */
	uint32_t ulTop,                        /* [in] Rectangle top. */
	uint32_t ulWidth,                      /* [in] Rectangle width. */
	uint32_t ulHeight                      /* [in] Rectangle height. */
);

/***************************************************************************
Summary:
	Sets the coefficients for the source filter.

Description:
	Specifies the coefficients to use when filtering is enabled on the
	source. If scaling is enabled, filtering will be enabled except
	for severe downscaling. If scaling isn't enabled, filtering
	can be enabled through BGRC_Source_ToggleFilter.

	The default for both filter coefficients is BGRC_FilterCoeffs_eSharp.

Returns:
	BERR_SUCCESS - Filter coefficients were set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_ToggleFilter
****************************************************************************/
BERR_Code BGRC_Source_SetFilterCoeffs(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	BGRC_FilterCoeffs eHorizontalCoeffs,   /* [in] Horizontal coefficients 
	                                          selection. */
	BGRC_FilterCoeffs eVerticalCoeffs      /* [in] Vertical coefficients
	                                          selection. */
);

/***************************************************************************
Summary:
	Used to toggle filtering on the source.

Description:
	This function is used to enable filtering on the horizontal
	or vertical axis of the source. This state is only used when scaling
	is disabled for a specific axis.

Returns:
	BERR_SUCCESS - Filtering state was updated.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_SetFilterCoeffs
****************************************************************************/
BERR_Code BGRC_Source_ToggleFilter(
	BGRC_Handle hGrc,                 /* [in] Handle to GRC module. */
	bool bEnableHorizontalFilter,     /* [in] If true, the horizontal
	                                     filter is enabled. When false,
									     the horizontal filter is disabled. */       
	bool bEnableVerticalFilter        /* [in] If true, the vertical
	                                     filter is enabled. When false,
									     the vertical filter is disabled. */
);

/***************************************************************************
Summary:
	Sets the initial phase adjustment for filtering.

Description:
	Specifies how many source pixels to initial shift when filtering.
	The phase will be shifted by the shift value and added to the default
	initial phase value. A positive value will shift the image left or 
	up, and a negative value will shift it right or down.

	The resulting initial phase shift formula when scaling is:
		(SrcSize / OutSize - 1) / 2 + PhaseAdj

	When not scaling the formula is:
		1/2 + PhaseAdj

	The final adjusted phase shift must be greater than or equal to -4.0 
	and less than 4.0.

	The default values are all 0.

Returns:
	BERR_SUCCESS - Filter inital phase was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_SetFilterCoeffs
****************************************************************************/
BERR_Code BGRC_Source_SetFilterPhaseAdjustment(
	BGRC_Handle hGrc,                  /* [in] Handle to GRC module. */
	int32_t iHorizontalPhase,          /* [in] Horizontal initial phase. */
	int32_t iVerticalPhase,            /* [in] Vertical initial phase. */
	uint32_t ulShift                   /* [in] Initial phase shift value. */
);

/***************************************************************************
Summary:
	Sets the scale factors when scaling or filtering.

Description:
	Overrides the horizontal and vertical scale factor when scaling or when 
	filtering is enabled. When either a numerator or a denominator is set 
	to 0, the corresponding scale factor will be calculated	using the source 
	and output rectangles, where the source size is used as the numerator.

	The default values are all 0.

Returns:
	BERR_SUCCESS - Scale factors were set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_SetFilterPhaseAdjustment
****************************************************************************/
BERR_Code BGRC_Source_SetFixedScaleFactor(
	BGRC_Handle hGrc,                  /* [in] Handle to GRC module. */
	uint32_t ulHorizontalNumerator,    /* [in] Horizontal/Source numerator */
	uint32_t ulHorizontalDenominator,  /* [in] Horizontal/Source denominator */
	uint32_t ulVerticalNumerator,      /* [in] Vertical/Output numerator */
	uint32_t ulVerticalDenominator     /* [in] Vertical/Output denominator */
);

/***************************************************************************
Summary:
	Specifies whether to adjust alpha for scaling.

Description:
	This bit will have effect only when "ScaleAlphaAdjust" is enabled.
	When enabled, alpha/2 value is subtracted from input sample components. 
	When disabled, a constant value of 128 is subtracted from the input 
	sample components. These subtractions are done before data goes into 
	the filter and are added back once the sample comes out, however the 
	alpha value added is the modified (scaled) alpha value.

	The default is false.

Returns:
	BERR_SUCCESS - Alpha adjust was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
  
See Also:
	BGRC_Source_SetFilterCoeffs
****************************************************************************/
BERR_Code BGRC_Source_SetScaleAlphaAdjust(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	bool bAdjustAlpha                      /* [in] Adjust alpha enable. */
);

/***************************************************************************
Summary:
	Specifies whether to alpha premultiply for scaling.

Description:
	Indicates if the input samples are required to be treated as alpha 
	premultiplied by the scaler. This will have effect only when 
	BGRC_Source_SetScaleAlphaAdjust is enabled. When enabled, alpha/2 value 
	is subtracted from input sample. When disabled, a constant value of 0x80 
	is subtracted from the input sample. These subtractions are done before 
	data goes into the FIR. They are added back in the same way as above once 
	the sample comes out of FIR. However, the alpha value added is the 
	modified (scaled) alpha value.

	The default is false.

Returns:
	BERR_SUCCESS - Scale premultiply was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
  
See Also:
	BGRC_Source_SetFilterCoeffs
****************************************************************************/
BERR_Code BGRC_Source_SetScaleAlphaPreMultiply(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	bool bAlphaPreMultiply                 /* [in] Premultiply enable. */
);

/***************************************************************************
Summary:
	Specifies how to round values after scaling.

Description:
	The default rounding state is BGRC_Rounding_eNearest.

Returns:
	BERR_SUCCESS - Scale rounding was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_SetFilterCoeffs
****************************************************************************/
BERR_Code BGRC_Source_SetScaleRounding(
	BGRC_Handle hGrc,                    /* [in] Handle to GRC module. */
	BGRC_Rounding eScaleRounding         /* [in] Scale rounding selection. */
);
	
/***************************************************************************
Summary:
	Specifies what to do on scaler edge conditions.

Description:
	The default is to use an edge condition of BGRC_EdgeCondition_eMirror.

Returns:
	BERR_SUCCESS - Scale edge condition was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
  
See Also:
	BGRC_Source_SetFilterCoeffs
****************************************************************************/
BERR_Code BGRC_Source_SetScaleEdgeCondition(
	BGRC_Handle hGrc,                    /* [in] Handle to GRC module. */
	BGRC_EdgeCondition eEdgeCondition    /* [in] Edge condition selection. */
);

/***************************************************************************
Summary:
	Enables alpha premultiply.

Description:
	When this bit is enabled the components of the pixel are multiplied by 
	the correspnding alpha value before scaling. 

Returns:
	BERR_SUCCESS - Alpha premultiply was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
  
See Also:
	BGRC_Source_SetClutScaleMode
****************************************************************************/
BERR_Code BGRC_Source_SetAlphaPreMultiply(
	BGRC_Handle hGrc,                    /* [in] Handle to GRC module. */
	bool bAlphaPreMultiply               /* [in] Alpha premultiply enable. */
);

/***************************************************************************
Summary:
	Enables alpha premultiply offset.

Description:
	When this bit is enabled a constant value of 128 is subtracted from 
	the components before alpha-premultiply and is added back afterwords. 
	This has no effect if "AlphaPreMultiply" is not enabled.

Returns:
	BERR_SUCCESS - Alpha premultiply offset was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
  
See Also:
	BGRC_Source_SetClutScaleMode
****************************************************************************/
BERR_Code BGRC_Source_SetAlphaPreMultiplyOffset(
	BGRC_Handle hGrc,                    /* [in] Handle to GRC module. */
	bool bOffset                         /* [in] Offset adjustment enable. */
);

/***************************************************************************
Summary:
	Sets MacroBlock range type for YCbCr 420 source.

Description:
	When using a YCbCr 420 source, the MacroBlock range can be set to
	none, expansion, or remapping for both Y and CbCr. With remapping, 
	a scale factor between 0 and 7 is required.

	The formula for range remapping using a scale factor is:
	Y = CLIP(((Y - 128) * (ucScaleFactorY + 9) + 4) >> 3) + 128)
	C = CLIP(((C - 128) * (ucScaleFactorC + 9) + 4) >> 3) + 128)

	Default is none.

Returns:
	BERR_SUCCESS - MacroBlock was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
  
See Also:
	BGRC_Source_SetSurface
****************************************************************************/
BERR_Code BGRC_Source_SetMacroBlock(
	BGRC_Handle hGrc,                    /* [in] Handle to GRC module. */
	BGRC_MacroBlockRange eRangeY,        /* [in] Range type for Y */
	BGRC_MacroBlockRange eRangeC,        /* [in] Range type for CbCr */
	uint8_t ucScaleFactorY,              /* [in] Scale factor for Y */
	uint8_t ucScaleFactorC               /* [in] Scale factor for CbCr */
);

/***************************************************************************
Summary:
	Sets MacroBlock strip width.

Description:
	When using a YCbCr 420 source, the MacroBlock strip width defaults
	to 64 but can also be set to 128 for newer M2MCs.

	Default is 64.

Returns:
	BERR_SUCCESS - MacroBlock strip width was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
  
See Also:
	BGRC_Source_SetMacroBlock
****************************************************************************/
BERR_Code BGRC_Source_SetMacroBlock_StripWidth(
	BGRC_Handle hGrc,                    /* [in] Handle to GRC module. */
	uint32_t ulStripWidth                /* [in] Macroblock strip width */
);

/***************************************************************************
Summary:
	Sets MacroBlock storage format.

Description:
	When using a YCbCr 420 source, the MacroBlock source can either be 
	stored in the display or linear format.

	Default is display format (false).

Returns:
	BERR_SUCCESS - MacroBlock linear format was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
  
See Also:
	BGRC_Source_SetMacroBlock
****************************************************************************/
BERR_Code BGRC_Source_SetMacroBlock_LinearFormat(
	BGRC_Handle hGrc,                    /* [in] Handle to GRC module. */
	bool bLinearFormat                   /* [in] Enable linear format */
);

/***************************************************************************
Summary:
	Sets MacroBlock endian.

Description:
	When using a YCbCr 420 source, the MacroBlock source can either be 
	stored as little or big endian.

	Default is little endian (false).

Returns:
	BERR_SUCCESS - MacroBlock endian was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
  
See Also:
	BGRC_Source_SetMacroBlock
****************************************************************************/
BERR_Code BGRC_Source_SetMacroBlock_Endian(
	BGRC_Handle hGrc,                    /* [in] Handle to GRC module. */
	bool bBigEndian                      /* [in] Enable big endian format */
);

/***************************************************************************
Summary:
	Sets source blit direction.

Description:
	This function inverts the pixels of the surface in the specified 
	direction(s), from right to left, and bottom to top.

	Defaults are false, left to right, and top to bottom.

Returns:
	BERR_SUCCESS - Direction was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
****************************************************************************/
BERR_Code BGRC_Source_SetDirection(
	BGRC_Handle hGrc,
	bool bRightToLeft,
	bool bBottomToTop 
);

/***************************************************************************
Summary:
	Resets the state of the source.

Description:
	All state set by source functions is now set to be the default value.

Returns:
	BERR_SUCCESS - Default state now set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Source_SetDefault
****************************************************************************/
BERR_Code BGRC_Source_ResetState(
	BGRC_Handle hGrc                     /* [in] Handle to GRC module. */
);

/***************************************************************************
Summary:
	Sets the default parameters for the source.

Description:
	Each source function has a default state associated with it, described
	in each of the functions used to modify that state. However, the user
	may wish to modify the default state so that future calls to 
	BGRC_Source_ResetState will use user defined values.
	
	This function allows you to change the default state. First the user
	calls functions to set the needed source state. It then calls
	this function which will then use the current state as the default.
	Subsequent calls to BGRC_Source_ResetState will use the new
	values.

Returns:
	BERR_SUCCESS - Default state now set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Source_ResetState
****************************************************************************/
BERR_Code BGRC_Source_SetDefault(
	BGRC_Handle hGrc                     /* [in] Handle to GRC module. */
);

/***************************************************************************
Summary:
	Sets the main surface for the destination.

Description:
	This is used in conjunction with the alpha surface specified
	with BGRC_Destination_SetAlphaSurface to describe the destination
	pixels.

	This surface is not required. As a default, this surface is not
	specified.

Returns:
	BERR_SUCCESS - Main surface was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Destination_SetAlphaSurface,
****************************************************************************/
BERR_Code BGRC_Destination_SetSurface(
	BGRC_Handle hGrc,                  /* [in] Handle to GRC module. */
	BSUR_Surface_Handle hSurface       /* [in] Main surface handle. */
);

/***************************************************************************
Summary:
	Sets the alpha surface for the destination

Description:
	This is used in conjunction with the main surface specified with
	BGRC_Destination_SetSurface to describe the destination pixels.

	Currently, this can only be used with a main surface having a pixel
	format of BPXL_eR5_G6_B5 and an alpha surface having a pixel format
	of BPXL_eA1 or BPXL_eW1.

	This surface is not required. As a default, this surface is not
	specified.

Returns:
	BERR_SUCCESS - Alpha surface was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Destination_SetSurface,
****************************************************************************/
BERR_Code BGRC_Destination_SetAlphaSurface(
	BGRC_Handle hGrc,                     /* [in] Handle to GRC module. */
	BSUR_Surface_Handle hSurface          /* [in] Alpha surface handle. */
);

/***************************************************************************
Summary:
	Enables or disables the palette index bypass.

Description:
    Typically when a source contains a palette, the palette index
	is used to get color channel information from an actual palette.
	However, the user may decide to override this behavior and
	pass through the index as is. This function is used to 
	enable and disable this state.

	If this state is enabled, C2 and C1 will be set to zero and C0 will
	contain the index value. 

    The default is for the palette index to be used as a reference
	to a palette and not to use the palette index.

Returns:
	BERR_SUCCESS - Palette bypass was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.

See Also:
	BGRC_Destination_SetSurface
****************************************************************************/
BERR_Code BGRC_Destination_TogglePaletteBypass(
	BGRC_Handle hGrc,               /* [in] Handle to GRC module. */
	bool bEnableBypass              /* [in] If true, index will be used
									   directly. If false, the palette will
									   be referenced. */
);

/***************************************************************************
Summary:
	Controls the chroma expansion for compressed YCrCb formats.

Description:
    When a compressed YCrCb format is used as an input, say
	BPXL_eY08_Cb8_Y18_Cr8, the pixels must first be uncompressed
	before they are converted into the device's internal color format, 
	BPXL_eA8_Y8_Cb8_Cr8. This function controls how the pixels 
	are uncompressed.

	As a default, eChromaExpansion will have a value of
	BGRC_ChromaExpansion_eReplicate.

Returns:
	BERR_SUCCESS - Alpha surface was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Destination_SetSurface,
****************************************************************************/
BERR_Code BGRC_Destination_SetChromaExpansion(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	BGRC_ChromaExpansion eChromaExpansion  /* [in] Chroma expansion type. */
);

/***************************************************************************
Summary:
	Enables zero padding or bit replication.

Description:
	When the destination feeder reads pixels, it the pixel components are 
	less than 8 bits in size, it can either zero pad the low order bits, or 
	replicate the high order bits into the low order bits.

    The default is false (replication).

Returns:
	BERR_SUCCESS - Palette bypass was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Destination_SetSurface
****************************************************************************/
BERR_Code BGRC_Destination_SetZeroPad(
	BGRC_Handle hGrc,                /* [in] Handle to GRC module. */
	bool bEnableZeroPad              /* [in] If true, zero padding will be
									    used. If false, bit replication
										will be used. */
);

/***************************************************************************
Summary:
	Sets color key values for the destination.

Description:
	When called, the destination color key state is set to use the given
	values. When color keying is enabled with
	BGRC_Destination_ToggleColorKey, the values are used as follows:

	o Input color channels are masked with ulMask. The minimum (ulMin) 
	  and maximum (ulMax) range values are also masked.
	o If each of the components is less than or equal to the masked
	  maximum and greater than or equal to the masked minimum, then the
	  value is considered to be color keyed. Otherwise, it is not color
	  keyed.
	o If the value is color keyed, Any bits of the result that have
	  a corresponding bit set in ulReplacementMask are replaced with 
	  the bit value in ulReplacement.

	When setting bExclusive to true, the above compares become less than
	instead of less equal, and greater than instead of greater equal.

	When using ARGB, the color format of the parameters are 
	BPXL_eA8_R8_G8_B8, and when using AYCbCr, the formats are 
	BPXL_eA8_Y8_Cb8_Cr8.

	When a pixel is or isn't chroma keyed it effects the final output
	selection specified with the function BGRC_Output_SetColorKeySelection.

	The default is for all values to be set to zero, and bExclusive set to 
	false.

Returns:
	BERR_SUCCESS - Color key was enabled and set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Destination_ToggleColorKey,
	BGRC_Destination_SetKeyMatrixOrder,
	BGRC_Output_SetColorKeySelection,
****************************************************************************/
BERR_Code BGRC_Destination_SetColorKey(
	BGRC_Handle hGrc,                     /* [in] Handle to GRC module. */
	uint32_t ulMin,                       /* [in] Minimum color key value. */
	uint32_t ulMax,                       /* [in] Maximum color key value. */
	uint32_t ulMask,                      /* [in] Color key mask. */
	uint32_t ulReplacement,               /* [in] Replacement value. */
	uint32_t ulReplacementMask,           /* [in] Replacement mask. */
	bool bExclusive                       /* [in] Inclusive/exclusive
	                                         compare. */
);

/***************************************************************************
Summary:
	Toggles the state of color keying.

Description:
	This function is used to enable and disable the state of color keying.
	When enabled, the values and process described in 
	BGRC_Destination_SetColorKey will be used. When disabled, all pixels
	are marked as not being color keyed. This effects the final output
	selection, specified with the function
	BGRC_Output_SetColorKeySelection.

	The default is for color keying to be disabled.
	
Returns:
	BERR_SUCCESS - Color key was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Destination_SetColorKey,
	BGRC_Destination_SetKeyMatrixOrder,
	BGRC_Output_SetColorKeySelection,
****************************************************************************/
BERR_Code BGRC_Destination_ToggleColorKey(
	BGRC_Handle hGrc,                /* [in] Handle to GRC module. */
	bool bEnable                     /* [in] If true, color keying is
										enabled. If false, color keying
										is disabled. */
);

/***************************************************************************
Summary:
	Sets the 5x4 color matrix for the destination. 

Description:
	This matrix is used to modify the incoming destination values.
	Typically this is done to convert from one color space to another.

	The values used for the color matrix are equal to the values stored 
	in the ai32_Matrix array right shifted by the ulShift value. In
	addition, negative values are converted to positive values before
	shifting and then turned back into negative values. For
	example, if a value in the matrix was 4 and the shift value was 2, then
	the final value is 1 (4>>2 = 1). If a value in the matrix was 7
	and the shift value was 3, then the final value would be 0.875.
	If a value in the matrix was -7 and the shift value was 3 then the
	final values is -0.875.

	The values within the color matrix are in the following order:

	      |  M[0]   M[1]   M[2]   M[3]   M[4] |
	      |  M[5]   M[6]   M[7]   M[8]   M[9] |
	      | M[10]  M[11]  M[12]  M[13]  M[14] |
	      | M[15]  M[16]  M[17]  M[18]  M[19] |

	This is multiplied with a column vector of 
	< C2in, C1in, C0in, Ain, 1 > which yields the following equations:

	C2out =  M[0]*C2in +  M[1]*C1in +  M[2]*C0in +  M[3]*Ain +  M[4]
	C1out =  M[5]*C2in +  M[6]*C1in +  M[7]*C0in +  M[8]*Ain +  M[9]
	C0out = M[10]*C2in + M[11]*C1in + M[12]*C0in + M[13]*Ain + M[14]
	Aout  = M[15]*C2in + M[16]*C1in + M[17]*C0in + M[18]*Ain + M[19]

	The default is to for the matrix to represent the identity.
	M[0] = M[6] = M[12] = M[18] = 1. All other values are zero.

Returns:
	BERR_SUCCESS - Color matrix was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.

See Also:
	BGRC_Destination_ToggleColorMatrix,
	BGRC_Destination_SetColorMatrixRounding,
	BGRC_Destination_SetKeyMatrixOrder,
****************************************************************************/
BERR_Code BGRC_Destination_SetColorMatrix5x4(
	BGRC_Handle hGrc,                 /* [in] Handle to GRC module. */
	const int32_t ai32_Matrix[20],    /* [in] Array of unshifted matrix
	                                     values. */
	uint32_t ulShift                  /* [in] Matrix shift value. */
);

/***************************************************************************
Summary:
	Toggles the state of the color matrix.

Description:
	This function is used to enable and disable the color matrix.
	When enabled, the values and process described in 
	BGRC_Destination_SetColorMatrix5x4 will be used. When disabled,
	the matrix values have no effect.

	The default is for the color matrix to be disabled.
	
Returns:
	BERR_SUCCESS - Color matrix was enabled and set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.

See Also:
	BGRC_Destination_SetColorMatrix5x4,
	BGRC_Destination_SetKeyMatrixOrder,
****************************************************************************/
BERR_Code BGRC_Destination_ToggleColorMatrix(
	BGRC_Handle hGrc,                /* [in] Handle to GRC module. */
	bool bEnable                     /* [in] If true, the color matrix is
										enabled. If false, the color matrix
										is disabled. */
);

/***************************************************************************
Summary:
	Specifies how to round the output values from the destination color
	matrix.

Description:
	The default rounding state is BGRC_Rounding_eNearest.

	In our current hardware implementation, this setting cannot differ from
	the source color matrix rounding setting or output color matrix
	rounding setting. The user must insure that these settings are the
	same or an error will result.

Returns:
	BERR_SUCCESS - Color matrix rounding was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
  
See Also:
	BGRC_Destination_SetColorMatrix5x4,
	BGRC_Destination_ToggleColorMatrix,
	BGRC_Source_SetColorMatrixRounding,
	BGRC_Output_SetColorMatrixRounding
****************************************************************************/
BERR_Code BGRC_Destination_SetColorMatrixRounding(
	BGRC_Handle hGrc,                   /* [in] Handle to GRC module. */
	BGRC_Rounding eMatrixRounding       /* [in] Matrix rounding selection. */
);

/***************************************************************************
Summary:
	Determines the order of the color key and matrix operation.

Description:
	Gives the user the option of doing color keying first and color 
	matrix operations second or the option of doing color matrix
	operations first and color keying second.

	The default is to use an order of 
	BGRC_KeyMatrixOrder_eKeyThenMatrix.

Returns:
	BERR_SUCCESS - Color keying was enabled.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.

See Also:
	BGRC_Destination_ToggleColorKey,
	BGRC_Destination_ToggleColorMatrix
****************************************************************************/
BERR_Code BGRC_Destination_SetKeyMatrixOrder(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	BGRC_KeyMatrixOrder eKeyMatrixOrder    /* [in] Color key/matrix order 
	                                          selection. */
);

/***************************************************************************
Summary:
	Sets the constant color for the destination.

Description:
	This value is used when the destination surfaces are missing the alpha
	or color channels. When the color is ARGB, the color format is 
	BPXL_eA8_R8_G8_B8, and when it is AYCbCr, the format is 
	BPXL_eA8_Y8_Cb8_Cr8. When the surface format is less than a byte in 
	size, the high-bits of the low-byte contains the constant color.

	The default is 0xFF000000 (Alpha is 1, color channels are 0).
  
Returns:
	BERR_SUCCESS - Constant alpha and color was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Destination_SetSurface,
	BGRC_Destination_SetAlphaSurface
****************************************************************************/
BERR_Code BGRC_Destination_SetColor(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	uint32_t ulColor                       /* [in] Constant values to use. */
);

/***************************************************************************
Summary:
	Specifies the sub-rectangle of the destination surface.

Description:
	Only the values within this rectangle will be used in the
	compositor calculations. The values given here must be restricted
	to the size of the surface itself.

	The source will be scaled to the size of the output
	rectangle, specified by BGRC_Output_SetRectangle. Since our
	current hardware does not support scaling of the destination, 
	the destination rectangle size must match the size of the 
	output rectangle.

	The default behavior is to use a rectangle the size of the entire
	specified destination.

Returns:
	BERR_SUCCESS - Rectangle was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Output_SetRectangle
****************************************************************************/
BERR_Code BGRC_Destination_SetRectangle(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	uint32_t ulLeft,                       /* [in] Rectangle left. */
	uint32_t ulTop,                        /* [in] Rectangle top. */
	uint32_t ulWidth,                      /* [in] Rectangle width. */
	uint32_t ulHeight                      /* [in] Rectangle height. */
);

/***************************************************************************
Summary:
	Sets destination blit direction.

Description:
	This function inverts the pixels of the surface in the specified 
	direction(s), from right to left, and bottom to top.

	Defaults are false, left to right, and top to bottom.

Returns:
	BERR_SUCCESS - Direction was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
****************************************************************************/
BERR_Code BGRC_Destination_SetDirection(
	BGRC_Handle hGrc,
	bool bRightToLeft,
	bool bBottomToTop 
);

/***************************************************************************
Summary:
	Resets the state of the destination.

Description:
	All state set by destination functions is now set to be the default
	value.

Returns:
	BERR_SUCCESS - Default state now set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Destination_SetDefault
****************************************************************************/
BERR_Code BGRC_Destination_ResetState(
	BGRC_Handle hGrc                     /* [in] Handle to GRC module. */
);

/***************************************************************************
Summary:
	Sets the default parameters for the destination.

Description:
	Each destination function has a default state associated with it,
	described in each of the functions used to modify that state. However,
	the user may wish to modify the default state so that future calls to 
	BGRC_Destination_ResetState will use user defined values.
	
	This function allows you to change the default state. First the user
	calls functions to set the needed destination state. It then calls
	this function which will then use the current state as the default.
	Subsequent calls to BGRC_Destination_ResetState will use the new
	values.

Returns:
	BERR_SUCCESS - Default state now set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Destination_ResetState
****************************************************************************/
BERR_Code BGRC_Destination_SetDefault(
	BGRC_Handle hGrc                     /* [in] Handle to GRC module. */
);

/***************************************************************************
Summary:
	Sets the parameters for the pattern operation.

Description:
	Mixes source, destination, and/or pattern using standard 
	ROP3 operations.

	The pattern will be oriented to the output top/left position.

Returns:
	BERR_SUCCESS - Pattern was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Output_SetColorKeySelection
****************************************************************************/
BERR_Code BGRC_Pattern_Set(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	uint8_t ucVector,                      /* [in] 8 bit ROP vector. */
	const uint8_t aucPattern[8],           /* [in] 8x8 bit ROP pattern. */
	uint32_t ulColor0,                     /* [in] ROP background color. */
	uint32_t ulColor1                      /* [in] ROP foreground color. */
);

/***************************************************************************
Summary:
	Resets the state of the pattern block.

Description:
	All state set by pattern functions is now set to be the default value.

Returns:
	BERR_SUCCESS - Default state now set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Pattern_SetDefault
****************************************************************************/
BERR_Code BGRC_Pattern_ResetState(
	BGRC_Handle hGrc                     /* [in] Handle to GRC module. */
);

/***************************************************************************
Summary:
	Sets the default parameters for the pattern block.

Description:
	Each pattern function has a default state associated with it, described
	in each of the functions used to modify that state. However, the user
	may wish to modify the default state so that future calls to 
	BGRC_Pattern_ResetState will use user defined values.
	
	This function allows you to change the default state. First the user
	calls functions to set the needed source state. It then calls
	this function which will then use the current state as the default.
	Subsequent calls to BGRC_Pattern_ResetState will use the new
	values.

Returns:
	BERR_SUCCESS - Default state now set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Pattern_ResetState
****************************************************************************/
BERR_Code BGRC_Pattern_SetDefault(
	BGRC_Handle hGrc                     /* [in] Handle to GRC module. */
);

/***************************************************************************
Summary:
	Sets the constant color for blending operations.

Description:
	This value is used during the blend operations when blend source values
	are BGRC_Blend_Source_eConstantColor, 
	BGRC_Blend_Source_eInverseConstantColor, 
	BGRC_Blend_Source_eConstantAlpha, or 
	BGRC_Blend_Source_eInverseConstantAlpha. When the color is ARGB, the 
	color format is BPXL_eA8_R8_G8_B8, and when it is AYCbCr, the format 
	is BPXL_eA8_Y8_Cb8_Cr8. When the surface format is less than a byte in 
	size, the high-bits of the low-byte contains the constant color.

	The default is 0xFF000000 (Alpha is 1, color channels are 0).

Returns:
	BERR_SUCCESS - Constant color was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Blend_SetColorBlend,
	BGRC_Blend_SetAlphaBlend
****************************************************************************/
BERR_Code BGRC_Blend_SetColor(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	uint32_t ulColor                       /* [in] Constant values to use. */
);

/***************************************************************************
Summary:
	Sets the blending operations for the color components.

Description:
	The basic function used for each color component is determined
	by the following function:

	(A*B) +/- (C*D) +/- E

	By picking and choosing the values for A, B, C, D, and E, you can
	achieve many different kinds of blending effects. In addition,
	you can make the +/- choice in the equation.

	The default is for eSourceA to be BGRC_Blend_Source_eSourceColor,
	eSourceB to be BGRC_Blend_Source_eOne, bSubtractCD to be false,
	eSourceC to be BGRC_Blend_Source_eZero, eSourceD to be
	BGRC_Blend_Source_eZero, bSubtractE to be false, and eSourceE to be
	BGRC_Blend_Source_eZero. This results in a pass through of the color
	information.

Returns:
	BERR_SUCCESS - Color blend operations were set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Blend_SetAlphaBlend
****************************************************************************/
BERR_Code BGRC_Blend_SetColorBlend(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	BGRC_Blend_Source eSourceA,            /* [in] Source for A. */
	BGRC_Blend_Source eSourceB,            /* [in] Source for B. */
	bool bSubtractCD,                      /* [in] Subtract C*D enable. */
	BGRC_Blend_Source eSourceC,            /* [in] Source for C. */
	BGRC_Blend_Source eSourceD,            /* [in] Source for D. */
	bool bSubtractE,                       /* [in] Subtract for E enable. */
	BGRC_Blend_Source eSourceE             /* [in] Source for E. */
);

/***************************************************************************
Summary:
	Sets the blending operations for the alpha component.

Description:
	The basic function used for the alpha component is determined
	by the following function:

	(A*B) +/- (C*D) +/- E

	By picking and choosing the values for A, B, C, D, and E, you can
	achieve many different kinds of blending effects. In addition,
	you can make the +/- choice in the equation.

	The default is for eSourceA to be BGRC_Blend_Source_eSourceAlpha,
	eSourceB to be BGRC_Blend_Source_eOne, bSubtractCD to be false,
	eSourceC to be BGRC_Blend_Source_eZero, eSourceD to be
	BGRC_Blend_Source_eZero, bSubtractE to be false, and eSourceE to be
	BGRC_Blend_Source_eZero. This results in a pass through of the alpha
	component.

    Note that the following sources cannot be used by the alpha blender:
  	BGRC_Blend_Source_eSourceColor, BGRC_Blend_Source_eInverseSourceColor,
	BGRC_Blend_Source_eDestinationColor, or 
	BGRC_Blend_Source_eInverseDestinationColor.

Returns:
	BERR_SUCCESS - Alpha blend operations were set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Blend_SetColorBlend
****************************************************************************/
BERR_Code BGRC_Blend_SetAlphaBlend(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	BGRC_Blend_Source eSourceA,            /* [in] Source for A. */
	BGRC_Blend_Source eSourceB,            /* [in] Source for B. */
	bool bSubtractCD,                      /* [in] Subtract C*D enable. */
	BGRC_Blend_Source eSourceC,            /* [in] Source for C. */
	BGRC_Blend_Source eSourceD,            /* [in] Source for D. */
	bool bSubtractE,                       /* [in] Subtract for E enable. */
	BGRC_Blend_Source eSourceE             /* [in] Source for E. */
);

/***************************************************************************
Summary:
	Resets the state of the blend block.

Description:
	All state set by blend functions is now set to be the default value.

Returns:
	BERR_SUCCESS - Default state now set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Blend_SetDefault
****************************************************************************/
BERR_Code BGRC_Blend_ResetState(
	BGRC_Handle hGrc                     /* [in] Handle to GRC module. */
);

/***************************************************************************
Summary:
	Sets the default parameters for the blend block.

Description:
	Each blend function has a default state associated with it, described
	in each of the functions used to modify that state. However, the user
	may wish to modify the default state so that future calls to 
	BGRC_Blend_ResetState will use user defined values.
	
	This function allows you to change the default state. First the user
	calls functions to set the needed blend state. It then calls
	this function which will then use the current state as the default.
	Subsequent calls to BGRC_Blend_ResetState will use the new
	values.

Returns:
	BERR_SUCCESS - Default state now set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Blend_ResetState
****************************************************************************/
BERR_Code BGRC_Blend_SetDefault(
	BGRC_Handle hGrc                     /* [in] Handle to GRC module. */
);

/***************************************************************************
Summary:
	Sets the main surface for the output.

Description:
	This is used in conjunction with the alpha surface specified
	with BGRC_Output_SetAlphaSurface to describe the location where the
	final pixels are stored.

	This surface must be supplied. As a default, this surface is not
	specified.

Returns:
	BERR_SUCCESS - Main surface was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Output_SetAlphaSurface,
****************************************************************************/
BERR_Code BGRC_Output_SetSurface(
	BGRC_Handle hGrc,                  /* [in] Handle to GRC module. */
	BSUR_Surface_Handle hSurface       /* [in] Main surface handle. */
);

/***************************************************************************
Summary:
	Sets the alpha surface for the output.

Description:
	This is used in conjunction with the main surface specified with
	BGRC_Output_SetSurface to describe the location where the
	final pixels are stored.

	Currently, this can only be used with a main surface having a pixel
	format of BPXL_eR5_G6_B5 and an alpha surface having a pixel format
	of BPXL_eA1 or BPXL_eW1.

	This surface is not required. As a default, this surface is not
	specified.

Returns:
	BERR_SUCCESS - Alpha surface was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Output_SetSurface
****************************************************************************/
BERR_Code BGRC_Output_SetAlphaSurface(
	BGRC_Handle hGrc,                     /* [in] Handle to GRC module. */
	BSUR_Surface_Handle hSurface          /* [in] Alpha surface handle. */
);

/***************************************************************************
Summary:
	Sets the 5x4 color matrix for the output. 

Description:
	This matrix is used to modify the outgoing output values.
	Typically this is done to convert from one color space to another.

	The values used for the color matrix are equal to the values stored 
	in the ai32_Matrix array right shifted by the ulShift value. In
	addition, negative values are converted to positive values before
	shifting and then turned back into negative values. For
	example, if a value in the matrix was 4 and the shift value was 2, then
	the final value is 1 (4>>2 = 1). If a value in the matrix was 7
	and the shift value was 3, then the final value would be 0.875.
	If a value in the matrix was -7 and the shift value was 3 then the
	final values is -0.875.

	The values within the color matrix are in the following order:

	      |  M[0]   M[1]   M[2]   M[3]   M[4] |
	      |  M[5]   M[6]   M[7]   M[8]   M[9] |
	      | M[10]  M[11]  M[12]  M[13]  M[14] |
	      | M[15]  M[16]  M[17]  M[18]  M[19] |

	This is multiplied with a column vector of 
	< C2in, C1in, C0in, Ain, 1 > which yields the following equations:

	C2out =  M[0]*C2in +  M[1]*C1in +  M[2]*C0in +  M[3]*Ain +  M[4]
	C1out =  M[5]*C2in +  M[6]*C1in +  M[7]*C0in +  M[8]*Ain +  M[9]
	C0out = M[10]*C2in + M[11]*C1in + M[12]*C0in + M[13]*Ain + M[14]
	Aout  = M[15]*C2in + M[16]*C1in + M[17]*C0in + M[18]*Ain + M[19]

	The default is to for the matrix to represent the identity.
	M[0] = M[6] = M[12] = M[18] = 1. All other values are zero.

Returns:
	BERR_SUCCESS - Color matrix was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.

See Also:
	BGRC_Output_ToggleColorMatrix,
	BGRC_Output_SetColorMatrixRounding,
	BGRC_Output_SetKeyMatrixOrder,
****************************************************************************/
BERR_Code BGRC_Output_SetColorMatrix5x4(
	BGRC_Handle hGrc,                 /* [in] Handle to GRC module. */
	const int32_t ai32_Matrix[20],    /* [in] Array of unshifted matrix
	                                     values. */
	uint32_t ulShift                  /* [in] Matrix shift value. */
);

/***************************************************************************
Summary:
	Toggles the state of the color matrix.

Description:
	This function is used to enable and disable the color matrix.
	When enabled, the values and process described in 
	BGRC_Output_SetColorMatrix5x4 will be used. When disabled, the matrix
	values have no effect.

	The default is for the color matrix to be disabled.
	
Returns:
	BERR_SUCCESS - Color matrix was enabled and set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.

See Also:
	BGRC_Output_SetColorMatrix5x4,
	BGRC_Output_SetKeyMatrixOrder,
****************************************************************************/
BERR_Code BGRC_Output_ToggleColorMatrix(
	BGRC_Handle hGrc,                /* [in] Handle to GRC module. */
	bool bEnable                     /* [in] If true, the color matrix is
										enabled. If false, the color matrix
										is disabled. */
);

/***************************************************************************
Summary:
	Specifies how to round the output values from the output color matrix.

Description:
	The default rounding state is BGRC_Rounding_eNearest.

	In our current hardware implementation, this setting cannot differ from
	the source color matrix rounding setting or destination color matrix
	rounding setting. The user must insure that these settings are the
	same or an error will result.

Returns:
	BERR_SUCCESS - Color matrix rounding was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
  
See Also:
	BGRC_Output_SetColorMatrix5x4,
	BGRC_Output_ToggleColorMatrix,
	BGRC_Source_SetColorMatrixRounding,
	BGRC_Destination_SetColorMatrixRounding
****************************************************************************/
BERR_Code BGRC_Output_SetColorMatrixRounding(
	BGRC_Handle hGrc,                   /* [in] Handle to GRC module. */
	BGRC_Rounding eMatrixRounding       /* [in] Matrix rounding selection. */
);

/***************************************************************************
Summary:
	Specifies the sub-rectangle of the output surface.

Description:
	Only the values within this rectangle will be used to store the
	compositor calculations. The values given here must be restricted
	to the size of the surface itself.

	The source rectangle, specified with BGRC_Source_SetRectangle will
	be scaled to the size of this output rectangle. Since the destination
	cannot be scaled, its rectangle specified with
	BGRC_Destination_SetRectangle must have the same with and height as
	the output rectangle.

	The default behavior is to use a rectangle the size of the entire
	specified output.

Returns:
	BERR_SUCCESS - Rectangle was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Source_SetRectangle,
	BGRC_Destination_SetRectangle
****************************************************************************/
BERR_Code BGRC_Output_SetRectangle(
	BGRC_Handle hGrc,                      /* [in] Handle to GRC module. */
	uint32_t ulLeft,                       /* [in] Rectangle left. */
	uint32_t ulTop,                        /* [in] Rectangle top. */
	uint32_t ulWidth,                      /* [in] Rectangle width. */
	uint32_t ulHeight                      /* [in] Rectangle height. */
);

/***************************************************************************
Summary:
	Determines the selection based on color key results to be customized.

Description:
	This function determines the final output of the blender by checking
	whether a specific pixel was color keyed by source or destination
	and using the result specified in this table.

	The following defaults are used:

	eSrcNotKeyedDstNotKeyed - BGRC_Output_ColorKeySelection_eTakeSource
	eSrcNotKeyedDstKeyed - BGRC_Output_ColorKeySelection_eTakeSource
	eSrcKeyedDstNotKeyed - BGRC_Output_ColorKeySelection_eTakeDestination
	eSrcKeyedDstKeyed - BGRC_Output_ColorKeySelection_eTakeDestination

Returns:
	BERR_SUCCESS - Selection based on color key was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Source_ToggleColorKey,
	BGRC_Destination_ToggleColorKey
****************************************************************************/
BERR_Code BGRC_Output_SetColorKeySelection(
	BGRC_Handle hGrc,         /* [in] Handle to GRC module. */             
	BGRC_Output_ColorKeySelection eSrcNotKeyedDstNotKeyed, /* [in] 
		Selection when neither source or destination is color keyed. */
	BGRC_Output_ColorKeySelection eSrcNotKeyedDstKeyed,    /* [in]
		Selection when source isn't color keyed but the destination is. */
	BGRC_Output_ColorKeySelection eSrcKeyedDstNotKeyed,    /* [in]
		Selection when source is color keyed but the destination isn't. */
	BGRC_Output_ColorKeySelection eSrcKeyedDstKeyed        /* [in]
		Selection when both source and destination are color keyed. */

);

/***************************************************************************
Summary:
	Enables dithering.

Description:
	Enables dithering of output pixels. 

	If dithering is enabled, a 2-dimensional dithering will be applied
	to adjust the channels in the output block based on the low 2 bits of
	the current X and Y position being written. 

	This is typically done to increase the perceived range of colors for
	a given resulting image.

	The default state is false.
  
Returns:
	BERR_SUCCESS - Constant alpha and color was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.

See Also:
	BGRC_Output_SetSurface,
****************************************************************************/
BERR_Code BGRC_Output_SetDither(
	BGRC_Handle hGrc,                     /* [in] Handle to GRC module.*/
	bool bEnableDither                    /* [in] If true, dither will be used.
	                                         If false, dither will not be
											 used. */
);

/***************************************************************************
Summary:
	Sets output blit direction.

Description:
	This function inverts the pixels of the surface in the specified
	direction(s), from right to left, and bottom to top.

	Defaults are false, left to right, and top to bottom.

Returns:
	BERR_SUCCESS - Direction was set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
	BERR_NOT_SUPPORTED - Not supported.
****************************************************************************/
BERR_Code BGRC_Output_SetDirection(
	BGRC_Handle hGrc,
	bool bRightToLeft,
	bool bBottomToTop
);

/***************************************************************************
Summary:
	Resets the state of the output.

Description:
	All state set by output functions is now set to be the default value.

Returns:
	BERR_SUCCESS - Default state now set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Output_SetDefault
****************************************************************************/
BERR_Code BGRC_Output_ResetState(
	BGRC_Handle hGrc                     /* [in] Handle to GRC module. */
);

/***************************************************************************
Summary:
	Sets the default parameters for the output.

Description:
	Each output function has a default state associated with it, described
	in each of the functions used to modify that state. However, the user
	may wish to modify the default state so that future calls to 
	BGRC_Output_ResetState will use user defined values.
	
	This function allows you to change the default state. First the user
	calls functions to set the needed output state. It then calls
	this function which will then use the current state as the default.
	Subsequent calls to BGRC_Output_ResetState will use the new
	values.

Returns:
	BERR_SUCCESS - Default state now set.
	BERR_INVALID_PARAMETER - One of the input parameters was invalid.
  
See Also:
	BGRC_Output_ResetState
****************************************************************************/
BERR_Code BGRC_Output_SetDefault(
	BGRC_Handle hGrc                     /* [in] Handle to GRC module. */
);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BGRC_H__ */

/* end of file */
