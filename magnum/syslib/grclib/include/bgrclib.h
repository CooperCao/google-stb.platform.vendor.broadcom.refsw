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

/*= Module Overview *********************************************************
<verbatim>

Overview

This GRClib is a higher level interface to the GRC module and provides
various fill and blit routines. The syslib is intended to offer a more
convenient API to the M2MC hardware (that is, the blitter engine).


API Summary

o Initialization
  - BGRClib_Open............Opens the syslib
  - BGRClib_Close...........Closes the syslib

o Callback specification
  - BGRClib_SetCallback.....Sets the callback and param to be used when
                            callback is requested

o Basic operations
  - BGRClib_Fill............Simple fill
  - BGRClib_Copy............Simple copy of one surface to another

o Porter-Duff operations
  - BGRClib_PorterDuffBlit..Performs blit according to Porter-Duff operator
  - BGRClib_PorterDuffFill..Performs fill according to Porter-Duff operator

o Extended Fill
  - BGRClib_BlendedFill.....Allows selection of fill channel (alpha or color) and option
                            of blending fill color with surface and of modifying alpha

o Fully flexible blit
  - BGRClib_Blit............Blit provides various blending options, including color
                            conversion


Definition: STANDARD COLOR FORMAT
o The term "standard color format" refers to a 32bit color value such that when
  the color format of the related surface is ARGB, then the "standard color
  format" refers to the BPXL_eA8_R8_G8_B8 format; when color format of the
  related surface is AYCbCr, then the "standard color format" refers to the
  format is BPXL_eA8_Y8_Cb8_Cr8. When the color format of the related surface
  is less than a byte in size, then the high-bits of the low-byte contains the
  constant color.

Design

This GRClib does not attempt to change the execution or threading model
of the GRC module. This syslib supports both blocking (synchronous) and
non-blocking (asychronous) usage, in the same manner that the GRC module
provides both the BGRC_IssueStateAndWait and the BGRC_IssueState routines.
The BGRClib_SetCallback() routine is used to set the callback is used for
all callbacks from this syslib.

All routines use Magnum types as parameters; no encapsulated types are introduced.

If multiple threads are calling GRClib, then the application must serialize
these calls.

Although the GRClib attempts to provide all fill and blit operations that might
be needed by an application, the GRClib does not "own" the GRC_Handle (which
was created by the GRC module). The application is permitted to call the GRC
module directly, but must complete any operations (that is, call BGRC_IssueState
or BGRC_IssueStateAndWait) before calling any routines in the this syslib. Also,
calls into GRC module must be serialize with the calls into GRClib.

Notes

If multiple M2MC blocks are used by the application, then it is the responsibility
of the application to synchronize operations between the M2MC blocks, if
necessary.

The GRClib does not validate all of the parameters against the requested operation,
especially in the BGRClib_Blit() routine, due to the large number of possible
combinations supported. Accordingly, some blit requests may have unexpected results
if passed inconsistent parameters, while others might return errors from the GRC module.

Symbols

  Ca: constant alpha
  Cc: constant color
  Sa: source pixel alpha
  Sc: source pixel color
  Da: destination pixel alpha
  Dc: destination pixel color
  Pa: output pixel alpha
  Pc: output pixel color

</verbatim>
***************************************************************************/

#ifndef BGRCLIB_H__
#define BGRCLIB_H__

#include "bstd.h"
#include "berr.h"
#include "bgrc.h"
#include "bpxl.h"
#if GRCPACKET_SUPPORT
#include "bgrc_packet.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * Enums
 *****************************************************************************/

/*****************************************************************************
  Summary:
    Color Format Type

  Description:
    This enum is used to identify the color format of any constant color
    parameters. The color format could be the standard color format (see above)
    or it could be the same color as an associated surface (for a fill, for
    example).

  See Also:
    BGRClib_Fill, BGRClib_BlendedFill
 *****************************************************************************/
typedef enum
{
    BGRClib_ColorFmtType_eSameAsSurf = 0,   /* The associated color parameter has the same format as
                                               a related surface. */
    BGRClib_ColorFmtType_eStdFormat         /* The associated color parameter has the standard color
                                               format. (See definition, above) */
} BGRClib_ColorFmtType;


/*****************************************************************************
  Summary:
    Pending Specification

  Description:
    This specifies whether the routine should block (by using BGRC_IssueStateAndWait)
    or not block (by using BGRC_IssueState).

  See Also:
    BGRClib_SetCallback, BGRClib_Fill, BGRClib_BlendedFill, etc
 *****************************************************************************/
typedef enum
{
    BGRClib_PendType_eBlock = 0,       /* Block until the call completes by using BGRC_IssueStateAndWait*/
    BGRClib_PendType_eNoBlockNoCB,     /* Do not block and do not use callback registered with GRClib */
    BGRClib_PendType_eNoBlockUseCB     /* Do not block. Use the callback registered with GRClib */
} BGRClib_PendType;


/*****************************************************************************
  Summary:
    Fill operations available to BGRClib_Blended_Fill

  Description:
    Options available for manipulating the color or alpha channel with
    BGRClib_Blended_Fill

  See Also:
    BGRClib_Blended_Fill
 *****************************************************************************/
typedef enum
{
    BGRCLib_FillOp_eIgnore = 0,         /* Do not change the values for the channel (either color or alpha)
                                           Pc = Sc; Pa = Sa */
    BGRCLib_FillOp_eCopy,               /* Copy the value of the channel (color or alpha) from the constant
                                           color to the surface: Pc = Cc; Pa = Ca */
    BGRCLib_FillOp_eBlend               /* For color channel, blend surface color with constant color using
                                           constant alpha. For alpha channel, combine constant alpha with source alpha.
                                           Pc = Ca*Cc + (1-Ca)*Sc; Pa = Ca*Ca + (1-Ca)*Sa */
} BGRCLib_FillOp;

/*****************************************************************************
  Summary:
    PorterDuff Operations

  Description:
    This enum lists the 12 Porter-Duff operations that can be performed
    between two surfaces.


  See Also:
    BGRClib_PorterDuffBlit and BGRClib_PorterDuffFill
 *****************************************************************************/
typedef enum
{
    BGRCLib_PorterDuffOp_eClear = 0,   /* Erase output  */
    BGRCLib_PorterDuffOp_eSrc,         /* Copy source to output */
    BGRCLib_PorterDuffOp_eDst,         /* Copy dest to output */
    BGRCLib_PorterDuffOp_eSrcOver,     /* Source over dest */
    BGRCLib_PorterDuffOp_eDstOver,     /* Dest over source */
    BGRCLib_PorterDuffOp_eSrcIn,       /* Part of source within dest */
    BGRCLib_PorterDuffOp_eDstIn,       /* Part of dest within source */
    BGRCLib_PorterDuffOp_eSrcOut,      /* Part of source not in dest */
    BGRCLib_PorterDuffOp_eDstOut,      /* Part of dest not in source */
    BGRCLib_PorterDuffOp_eSrcAtop,     /* Part of source in dest and remainder is dest */
    BGRCLib_PorterDuffOp_eDstAtop,     /* Part of dest in source and remainder is source */
    BGRCLib_PorterDuffOp_eXor,         /* Part of src not in dest and part of dest not in source */

    BGRCLib_PorterDuffOp_Count
} BGRCLib_PorterDuffOp;

/*****************************************************************************
  Summary:
    Color operations available in blit

  Description:

    Note that alpha values are always relative to the source and should be
    between 0x00 and 0xFF. Therefore alpha of 0xFF means take all of the source
    and of the destination. Likewise alpha of 0x00 means take none of source color,
    all of the destination color.

  See Also:
    BGRClib_Blit
 *****************************************************************************/
typedef enum
{
    BGRCLib_BlitColorOp_eCopySource = 0,            /* Select the source color. Pc = Sc */
    BGRCLib_BlitColorOp_eUseConstantAlpha,          /* Blend the source and dest colors using the alpha from the constant param
                                                       Pc = Ca*Sc + (1-Ca)*Dc */
    BGRCLib_BlitColorOp_eUseSourceAlpha,            /* Blend the source and dest colors using the alpha from the source pixels
                                                       Pc = Sa*Sc + (1-Sa)*Dc */
    BGRCLib_BlitColorOp_eUseDestAlpha,              /* Blend the source and dest colors using the alpha from the dest pixels
                                                       Pc = Da*Sc + (1-Da)*Dc */
    BGRCLib_BlitColorOp_eSelectPaletteWithColorkey, /* Combine two palettized surfaces using colorkey to select per pixel.
                                                       Use the colorKeyLower and colorKeyUpper fields of BGRClib_BlitParams
                                                       Pc = Sc (Note: this is the same as _eCopySource) */
    BGRCLib_BlitColorOp_eModulate,                  /* [DEPRECATED] use _eUseSourceAlpha instead  */
    BGRCLib_BlitColorOp_eAdd,                       /* Adds source and destination color (i.e., Pc = Sc + Dc). This can be used for
                                                       brightening and over-saturation effects */
    BGRCLib_BlitColorOp_eUseBlendFactors,           /* Use blend factors specified in BGRClib_BlitParams.colorBlend */

    BGRCLib_BlitColorOp_Count
} BGRCLib_BlitColorOp;


/*****************************************************************************
  Summary:
    Alpha operations available in blit

  Description:
    This enum lists the operations that are available on the alpha channel.

  See Also:
    BGRClib_Blit
 *****************************************************************************/
typedef enum
{
    BGRCLib_BlitAlphaOp_eCopySource = 0,
    BGRCLib_BlitAlphaOp_eUseConstAlpha,             /* Copy the constant parameter for the output alpha (Pa = Ca) */
    BGRCLib_BlitAlphaOp_eUseSourceAlpha,            /* Use the source alpha for the output alpha (Pa = Sa) */
    BGRCLib_BlitAlphaOp_eUseDestAlpha,              /* Use the dest alpha for the output alpha (Pa = Da) */
    BGRCLib_BlitAlphaOp_eUseCombinedAlpha,          /* Combine source and dest alpha. (Pa = Sa*Sa + Da*(1-Sa) */
    BGRCLib_BlitAlphaOp_eEmulateTransparentVideo,   /* Calculate alpha values so that video window, when placed behind the
                                                       graphics window, appears to be in front of the graphics, with an
                                                       alpha of Ca. (Pa = 1 - (1-Sa)*Ca) */
    BGRCLib_BlitAlphaOp_eModulate,                  /* [DEPRECATED] use _eUseSourceAlpha instead */
    BGRCLib_BlitAlphaOp_eAdd,                       /* Adds source and destionation alpha (Pa = Sa + Da) */
    BGRCLib_BlitAlphaOp_eUseBlendFactors,           /* Use blend factors specified in BGRClib_BlitParams.alphaBlend */

    BGRCLib_BlitAlphaOp_Count
} BGRCLib_BlitAlphaOp;

/*****************************************************************************
 * Structures
 *****************************************************************************/

/*****************************************************************************
  Summary:
    Rectangle type

  Description:
    Rectangle.

  See Also:
    BGRClib_Open
 *****************************************************************************/
typedef struct
{
    int32_t top;
    int32_t left;
    int32_t bottom;
    int32_t right;

} BRect;

/*****************************************************************************
  Summary:
    GRC Blending Equation

  Description:
    This structure exposes the blending equation used by the GRC during blits.

  See Also:
    BGRClib_Blit
 *****************************************************************************/
typedef struct
{
    /* Blend: a*b +/- b*c +/- e*/
    BGRC_Blend_Source a;
    BGRC_Blend_Source b;
    bool              subcd;
    BGRC_Blend_Source c;
    BGRC_Blend_Source d;
    bool              sube;
    BGRC_Blend_Source e;
} BGRClib_BlendEquation;

/*****************************************************************************
  Summary:
    Param block for blit routine

  Description:
    The param block provides all of the parameters for the generic blit function.

  See Also:
    BGRClib_Blit
 *****************************************************************************/
typedef struct
{
    BSUR_Surface_Handle*    srcSurfaceList;  /*  [in] A surface handle list. May be NULL */
    uint32_t                srcSurfaceCnt;
    const BRect*            srcRectList;     /*  [in] Source area of blit. NULL indicates entire surface. */
    uint32_t                srcRectCnt;

    BSUR_Surface_Handle*    dstSurfaceList;  /*  [in] A surface handle. May be NULL */
    uint32_t                dstSurfaceCnt;
    const BRect*            dstRectList;     /*  [in] Destination area of blit. NULL indicates entire surface. */
    uint32_t                dstRectCnt;

    BSUR_Surface_Handle*    outSurfaceList;  /*  [in] A surface handle. Required. */
    uint32_t                outSurfaceCnt;
    const BRect*            outRectList;     /*  [in] Output area of blit. NULL indicates entire surface. */
    uint32_t                outRectCnt;

    BGRCLib_BlitColorOp     colorOp;         /*  [in] Selection of color blending equation */
    BGRCLib_BlitAlphaOp     alphaOp;         /*  [in] Selection of alpha blending equation */
    BGRClib_BlendEquation   colorBlend;      /*  [in] Color blending equation (see bgrc.h:BGRC_Blend_SetColorBlend). Used when colorOp==_eUseBlendFactor.
                                                 [out] Otherwise, this data is popoluted with color equation used according to the colorOp */
    BGRClib_BlendEquation   alphaBlend;      /*  [in] Alpha blending equation (see bgrc.h:BGRC_Blend_SetAlphaBlend). Used when alphaOp==_eUseBlendFactor.
                                                 [out] Otherwise, this data is popoluted with alpha equation used according to the alphaOp */

    uint32_t                constantColor;   /*  [in] Constant alpha and color. Only standard color format supported. Used by some operations */
    uint32_t                destContColor;   /*  [in] Destination constant alpha and color */

    BGRC_Output_ColorKeySelection colorKeySelect; /* [in] Select which block provides pixels to output.
                                                      For operations which compute new pixel values, the blend block should be used.
                                                      For operations with palette surfaces, the source is typically the block to be used;
                                                      unless destination colorkeys is enabled, then the destination block should be used. */

    /* Filter to use for scaling */
    BGRC_FilterCoeffs       horzFilter;      /* [in] See BGRC_FilterCoeffs and BGRC_Source_SetFilterCoeffs in bgrc.h */
    BGRC_FilterCoeffs       vertFilter;      /* [in] See BGRC_FilterCoeffs and BGRC_Source_SetFilterCoeffs in bgrc.h */

    /* Blit Reversal Flags */
    bool                    mirrorSrcVertically;
    bool                    mirrorSrcHorizontally;
    bool                    mirrorDstVertically;
    bool                    mirrorDstHorizontally;
    bool                    mirrorOutVertically;
    bool                    mirrorOutHorizontally;

    bool                    srcAlphaPremult;    /* Enable/Disable Source Alpha Premultiplication */

} BGRClib_BlitParams;


/*****************************************************************************
  Summary:
    Color Conversion Matrix Param block for blit routine

  Description:
    The param block provides optional color conversion matrix parameters for
    the generic blit function.

  See Also:
    BGRClib_Blit
 *****************************************************************************/
typedef struct
{
    /* Source Color Conversion Matrix */
    int32_t*                conversionMatrix;/* Color conversion matrix. NULL indicates no matrix provided. */
    uint32_t                matrixShift;     /* Matrix shift value. See BGRC_Source_SetColorMatrix5x4() for more info */

} BGRClib_BlitMatrixParams;

/*****************************************************************************
  Summary:
    Color key Param block for blit routine

  Description:
    The param block provides optional color key parameters for the generic
    blit function.

  See Also:
    BGRClib_Blit
 *****************************************************************************/
typedef struct
{

    bool                    enableColorKey;  /*  [in] Enable colorkeying on the source surface */
    uint32_t                colorKeyLower;   /*  [in] Lower colorkey value. Only standard color format supported. */
    uint32_t                colorKeyUpper;   /*  [in] Upper colorkey value. Only standard color format supported. */
    uint32_t                colorKeyMask;    /*  [in] Mask to use when comparing color key to source color */
    uint32_t                colorKeyReplace; /*  [in] Replacement color */
    uint32_t                colorKeyRplMask; /*  [in] Mask for replacement color */

    bool                    enableDstColorKey;  /*  [in] Enable colorkeying on the source surface */
    uint32_t                dstColorKeyLower;   /*  [in] Lower colorkey value. Only standard color format supported. */
    uint32_t                dstColorKeyUpper;   /*  [in] Upper colorkey value. Only standard color format supported. */
    uint32_t                dstColorKeyMask;    /*  [in] Mask to use when comparing color key to source color */
    uint32_t                dstColorKeyReplace; /*  [in] Replacement color */
    uint32_t                dstColorKeyRplMask; /*  [in] Mask for replacement color */

    BGRC_Output_ColorKeySelection   cksOnlySrcColorKeyed;
    BGRC_Output_ColorKeySelection   cksOnlyDstColorKeyed;
    BGRC_Output_ColorKeySelection   cksBothSrcDstColorKeyed;

} BGRClib_BlitColorKeyParams;

/*****************************************************************************
  Summary:
    Pattern (raster operations) Param block for blit routine

  Description:
    The param block provides all of the parameters for the generic blit function.

  See Also:
    BGRClib_Blit
 *****************************************************************************/
typedef struct
{
	uint8_t  ropVector;       /* [in] 8 bit ROP vector. */
	uint8_t* pattern;         /* [in] 8x8 bit ROP pattern. */
	uint32_t backColor;       /* [in] ROP background color. */
	uint32_t foreColor;       /* [in] ROP foreground color. */

} BGRClib_BlitPatternParams;

/*****************************************************************************
  Summary:
    Additional Scaling Control Param block for blit routine

  Description:
    The param block provides additional scaling control parameters for the
    generic blit function.

  See Also:
    BGRClib_Blit
    BGRC_Source_SetFilterPhaseAdjustment
    BGRC_Source_SetFixedScaleFactor
 *****************************************************************************/
typedef struct
{
	/* See BGRC_Source_SetFilterPhaseAdjustment */
    bool     setFilterPhaseAdj;         /* [in] true: Set the filter phase adjustment */
	int32_t  iHorizontalPhase;          /* [in] Horizontal initial phase. */
	int32_t  iVerticalPhase;            /* [in] Vertical initial phase. */
	uint32_t ulPhaseFixedPointShift;    /* [in] Fixed-point shift, in bits, to allow for fractional values. */

	/* BGRC_Source_SetFixedScaleFactor */
    bool     setFixedScaleFactor;       /* [in] true: Set the fixed scale factor */
	uint32_t ulHorizontalNumerator;     /* [in] Horizontal/Source numerator */
	uint32_t ulHorizontalDenominator;   /* [in] Horizontal/Source denominator */
	uint32_t ulVerticalNumerator;       /* [in] Vertical/Output numerator */
	uint32_t ulVerticalDenominator;     /* [in] Vertical/Output denominator */

} BGRClib_BlitScalingControlParams;

/*****************************************************************************
  Summary:
    Handle for BGRClib module

  Description:
    The BGRClib_Handle, once opened, represents the state of the BGRClib
    module.

  See Also:
    BGRClib_Open
 *****************************************************************************/
typedef struct BGRClib_P_Data_tag* BGRClib_Handle;


/*****************************************************************************
 * Public API
 *****************************************************************************/


/*****************************************************************************
  Summary:
    Initialize the BGRClib module

  Description:
    This module intializes the BGRClib module.

  Returns:
    BERR_SUCCESS              - The handle was successfully created.
    BERR_INVALID_PARAMETER    - One of the supplied parameters was invalid,
                                possibly NULL.
    BERR_OUT_OF_SYSTEM_MEMORY - Memory allocation failed.

  See Also:
    BGRClib_Close
 *****************************************************************************/
BERR_Code BGRClib_Open(
    BGRClib_Handle*  pgrclibHandle, /* [out] A pointer to an allocated BGRClib_Handle */
    BGRC_Handle      grcHandle      /*  [in] An initialized BGRC module handle. */
#if GRCPACKET_SUPPORT
    ,
    BGRC_PacketContext_Handle packetContext
#endif
);


/*****************************************************************************
  Summary:
    Close a BGRClib_Handle.

  Description:
    This function frees the BGRClib_Handle structure and all associated data.

 *****************************************************************************/
void BGRClib_Close(
    BGRClib_Handle grclibHandle /* [in] A valid BGRClib_Handle object */
);


/*****************************************************************************
  Summary:
    Reset the state.

  Description:
    This function resets the state of the GRC module and resets all
    state info being tracked by GRClib.

    This MUST be called any time GRC is used outside of GRClib

 *****************************************************************************/
BERR_Code BGRClib_ResetState(
    BGRClib_Handle grclibHandle /* [in] A valid BGRClib_Handle object */
);

#if !GRCPACKET_SUPPORT
/*****************************************************************************
  Summary:
    Specify callback params.

  Description:
    This function sets the callback params for all callbacks.

 *****************************************************************************/
void BGRClib_SetCallback(
    BGRClib_Handle grclibHandle,    /* [in] A valid BGRClib_Handle object */
    BGRC_Callback  callback_isr,    /* [in] Pointer to completion callback. */
    void*          callback_data
);
#endif

#if GRCPACKET_SUPPORT
BERR_Code BGRClib_Sync(
	BGRClib_Handle grclib
);

BERR_Code BGRClib_Idle(
	BGRClib_Handle grclib,
	bool *pIdle
);

BERR_Code BGRClib_Memset32(
	BGRClib_Handle grclib,
	uint32_t offset,
	uint32_t data,
	uint32_t count,
	BGRClib_PendType pendtype
);

BERR_Code BGRClib_Destripe_Blit(
	BGRClib_Handle grclib,
	BSUR_Surface_Handle outSurface,
	const BRect *pOutRect,
	uint32_t lumaOffset,
	uint32_t chromaOffset,
	uint32_t pitch,
	uint32_t width,
	uint32_t height,
	uint32_t stripeWidth,
	uint32_t lumaStripeHeight,
	uint32_t chromaStripeHeight,
	const int32_t colorMatrix[],
	uint32_t matrixShift,
	BGRClib_PendType pendtype
);

BERR_Code BGRClib_Batch_Blit(
	BGRClib_Handle grclib,
	BSUR_Surface_Handle outSurface,
	const uint32_t *pOutOffsets,
	const uint16_t *pOutPoints,
	const uint16_t *pOutSizes,
	BSUR_Surface_Handle srcSurface,
	const uint32_t *pSrcOffsets,
	const uint16_t *pSrcPoints,
	const uint16_t *pSrcSizes,
	BSUR_Surface_Handle dstSurface,
	const uint32_t *pDstOffsets,
	const uint16_t *pDstPoints,
	const uint32_t *pColors,
	uint32_t count,
	uint32_t *index,
	BGRClib_PendType pendtype
);
#endif

/*****************************************************************************
  Summary:
    Fill a rectangular area of a surface

  Description:
    This routine fills an area of a surface with the specified color. The
    color may be in the standard color format (see definitions, above) or
    may match the format of the surface.

  Returns:
    BERR_SUCCESS                 - The surface fill completed.
    BERR_INVALID_PARAMETER       - One of the supplied parameters was invalid,
                                   possibly NULL.
    BGRC_ERR_M2MC_DEVICE_IS_HUNG - M2MC device timed-out

  See Also:
    BGRClib_ColorFmtType
 *****************************************************************************/
BERR_Code BGRClib_Fill(
    BGRClib_Handle          grclibHandle,    /*  [in] A valid BGRClib_Handle object */
    BSUR_Surface_Handle     surface,         /*  [in] A surface handle */
    uint32_t                constantcolor,   /*  [in] Color, either in color format of the surface */
                                             /*       or in the standard color format */
    BGRClib_ColorFmtType    fmtType,         /*  [in] Helps identify the format of the color param.  */
                                             /*       standard color format (see definitions, above) */
    const BRect*            pRect,           /*  [in] Pointer to a rect specifying the part of a */
                                             /*       surface to fill, or NULL to fill the entire surface */
    BGRClib_PendType        pendtype         /*  [in] Specifies if operation should pend (i.e., use */
                                             /*       BGRC_IssueStateAndWait), or should return immediately */
                                             /*       (i.e., use BGRC_IssueState) */

);


/*****************************************************************************
  Summary:
    Copy one surface to another

  Description:
    This routine copies an area of the source surface an area of the output
    surface. No color space conversion or color keying is performed. If the
    source surface is palette-based and the output surface is not
    paletted-based, then the palette entries in the source pixels will be
    converted to colors in the output pixels

  Returns:
    BERR_SUCCESS                 - The surface fill completed.
    BERR_INVALID_PARAMETER       - One of the supplied parameters was invalid,
                                   possibly NULL.
    BGRC_ERR_M2MC_DEVICE_IS_HUNG - M2MC device timed-out

  See Also:

 *****************************************************************************/
BERR_Code BGRClib_Copy(
    BGRClib_Handle          grclibHandle,    /*  [in] A valid BGRClib_Handle object */
    BSUR_Surface_Handle     srcSurface,      /*  [in] A surface handle */
    const BRect*            pSrcRect,        /*  [in] Pointer to a rect specifying the part of a */
                                             /*       surface to fill, or NULL to fill the entire surface */
    BSUR_Surface_Handle     outSurface,      /*  [in] A surface handle */
    const BRect*            pOutRect,        /*  [in] Pointer to a rect specifying the part of a */
                                             /*       surface to fill, or NULL to fill the entire surface */
    BGRClib_PendType        pendtype         /*  [in] Specifies if operation should pend or not */

);


/*****************************************************************************
  Summary:
    Fill, tint or otherwise modify the pixels of an
    area of a surface using a constant value

  Description:
    This routine modifies the color channels and/or the alpha channel of the
    pixels of a surface using a constant value

  Returns:
    BERR_SUCCESS                 - The surface fill completed.
    BERR_INVALID_PARAMETER       - One of the supplied parameters was invalid,
                                   possibly NULL.
    BGRC_ERR_M2MC_DEVICE_IS_HUNG - M2MC device timed-out

  See Also:

 *****************************************************************************/
BERR_Code BGRClib_Blended_Fill(
    BGRClib_Handle          grclibHandle,    /*  [in] A valid BGRClib_Handle object */
    BSUR_Surface_Handle     surface,         /*  [in] A surface handle */
    uint32_t                constantColor,   /*  [in] Color, either in C3C2C3C1 format */
                                             /*       or the specified format */
    BGRClib_ColorFmtType    fmtType,         /*  [in] Helps identify the format of the color param.  */
                                             /*       standard color format (see definitions, above) */
    const BRect*            pRect,           /*  [in] Pointer to a rect specifying the part of a */
                                             /*       surface to fill, or NULL to fill the entire surface */
    BGRCLib_FillOp          colorOp,         /*  [in] Color operation */
    BGRCLib_FillOp          alphaOp,         /*  [in] Alpha operation */
    BGRClib_PendType        pendtype         /*  [in] Specifies if operation should pend or not */
);


/*****************************************************************************
  Summary:
    Fill, tint or otherwise modify the pixels of an
    area of a surface using a constant value and Porter-Duff composition rules.

  Description:
    This routine modifies the color channels and/or the alpha channel of the
    pixels of a surface using a constant value and Porter-Duff composition rules.

  Returns:
    BERR_SUCCESS                 - The surface fill completed.
    BERR_INVALID_PARAMETER       - One of the supplied parameters was invalid,
                                   possibly NULL.
    BGRC_ERR_M2MC_DEVICE_IS_HUNG - M2MC device timed-out

  See Also:

 *****************************************************************************/
BERR_Code BGRClib_PorterDuffFill(
    BGRClib_Handle          grclibHandle,    /*  [in] A valid BGRClib_Handle object */
    BGRCLib_PorterDuffOp    pdOp,            /*  [in] A Porter-Duff operation */
    BSUR_Surface_Handle     surface,         /*  [in] A surface handle */
    uint32_t                constantColor,   /*  [in] Color, either in C3C2C3C1 format */
                                             /*       or the specified format */
    BGRClib_ColorFmtType    fmtType,         /*  [in] Helps identify the format of the color param.  */
                                             /*       standard color format (see definitions, above) */
    const BRect*            pRect,           /*  [in] Pointer to a rect specifying the part of a */
                                             /*       surface to fill, or NULL to fill the entire surface */
    BGRClib_PendType        pendtype         /*  [in] Specifies if operation should pend or not */
);


/*****************************************************************************
  Summary:
    Combine two surfaces into a third using Porter-Duff composition rules

  Description:
    This routine combines an area of the source surface and an area of the
    destination surface into an area of the output surface using Porter-Duff
    composition rules. No color space conversion or color keying is performed.

    If the source surface is palette-based and the output surface is not
    paletted-based, then the palette entries in the source pixels will be
    converted to colors in the output pixels

  Returns:
    BERR_SUCCESS                 - The surface fill completed.
    BERR_INVALID_PARAMETER       - One of the supplied parameters was invalid,
                                   possibly NULL.
    BGRC_ERR_M2MC_DEVICE_IS_HUNG - M2MC device timed-out

  See Also:

 *****************************************************************************/
BERR_Code BGRClib_PorterDuffBlit(
    BGRClib_Handle          grclibHandle,    /*  [in] A valid BGRClib_Handle object */
    BGRCLib_PorterDuffOp    pdOp,            /*  [in] A Porter-Duff operation */

    BSUR_Surface_Handle     srcSurface,      /*  [in] A surface handle */
    const BRect*            pSrcRect,        /*  [in] Pointer to a rect specifying the part of a */
                                             /*       surface to fill, or NULL to fill the entire surface */
    BSUR_Surface_Handle     dstSurface,      /*  [in] A surface handle */
    const BRect*            pDstRect,        /*  [in] Pointer to a rect specifying the part of a */
                                             /*       surface to fill, or NULL to fill the entire surface */
    BSUR_Surface_Handle     outSurface,      /*  [in] A surface handle */
    const BRect*            pOutRect,        /*  [in] Pointer to a rect specifying the part of a */
                                             /*       surface to fill, or NULL to fill the entire surface */
    BGRClib_PendType        pendtype         /*  [in] Specifies if operation should pend or not */

);


/*****************************************************************************
  Summary:
    Populate param block with default values

  Description:
    Set-up the blit param block for typical values for performing a blit
    on non-palette surfaces

  See Also:
    BGRClib_Blit
    BGRClib_GetDefaultPaletteBlitParams
 *****************************************************************************/
void BGRClib_GetDefaultBlitParams(
    BGRClib_BlitParams *params
);


/*****************************************************************************
  Summary:
    Populate param block with default values for Palette blits

  Description:
    Set-up the blit param block for typical values for performaing a blit on
    palette surfaces. The important differences are that a point sample filter
    is used and the source block is selected instead of the blend block.

  See Also:
    BGRClib_Blit
    BGRClib_GetDefaultBlitParams
 *****************************************************************************/
void BGRClib_GetDefaultPaletteBlitParams(
    BGRClib_BlitParams *params
);


/*****************************************************************************
  Summary:
    Populate colorkey param block with default values

  Description:
    Set-up the colorkey blit param block for typical values for performing a
    colorkey operation. The main purpose of this is to set-up the
    colorkey selection parameters.

  See Also:
    BGRClib_Blit
 *****************************************************************************/
void BGRClib_GetDefaultColorKeyParams(
    BGRClib_BlitColorKeyParams *colorkeyparams
);


/*****************************************************************************
  Summary:
    Populate scaling control param block with default values

  Description:
    Set-up the scaling control param block for initial values.

  See Also:
    BGRClib_Blit
 *****************************************************************************/
void BGRClib_GetDefaultScalingControlParams(
    BGRClib_BlitScalingControlParams *scalingparams
);

/*****************************************************************************
  Summary:
    Get color blend factors

  Description:
    Gets blend factors according to BGRCLib_BlitColorOp

  See Also:
    BGRClib_Blit
 *****************************************************************************/
BERR_Code BGRClib_GetColorBlendFactors(
    BGRCLib_BlitColorOp colorOp,
    BGRClib_BlendEquation* factors);


/*****************************************************************************
  Summary:
    Get alpha blend factors

  Description:
    Gets blends factors according to BGRCLib_BlitAlphaOp

  See Also:
    BGRClib_Blit
 *****************************************************************************/
BERR_Code BGRClib_GetAlphaBlendFactors(
    BGRCLib_BlitAlphaOp alphaOp,
    BGRClib_BlendEquation* factors);


/*****************************************************************************
  Summary:
    Get Porter-Duff blend factors

  Description:
    Gets blend factors used for Porter-Duff blits

  See Also:
    BGRClib_Blit
 *****************************************************************************/
BERR_Code BGRClib_GetPorterDuffBlendFactors(
    BGRCLib_PorterDuffOp pdOp,
    BGRClib_BlendEquation* colorFactors,
    BGRClib_BlendEquation* alphaFactors);


/*****************************************************************************
  Summary:
    Blit operation

  Description:


  See Also:
    BGRClib_Blit
 *****************************************************************************/
BERR_Code BGRClib_Blit(
    BGRClib_Handle                          grclibHandle,    /*  [in] A valid BGRClib_Handle object */
    const BGRClib_BlitParams*               params,          /*  [in] General blit parameters */
    const BGRClib_BlitColorKeyParams*       colorkeyParams,  /*  [in] Optional colorkey parameters. Use NULL if not using colorkeying */
    const BGRClib_BlitMatrixParams*         matrixParams,    /*  [in] Optional color conversion matrix parameters. Use NULL if not using color matrix */
    const BGRClib_BlitPatternParams*        patternParams,   /*  [in] Optional (ROP) pattern parameters. Use NULL if not using pattern block */
    const BGRClib_BlitScalingControlParams* scalingParams,   /*  [in] Optional scaling control parameters. Use NULL for default scaling behavior. */
    BGRClib_PendType                        pendtype         /*  [in] Specifies if operation should pend or not */
);



#ifdef __cplusplus
}
#endif

#endif /* BGRCLIB_H__ */
