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
 ***************************************************************************/

/*= Module Overview *********************************************************
<verbatim>

Overview

This version of GRClib is intended to convert nexus graphics2d non-packet function calls
into GRC packets.

It is only for internal use and is not backward compatible with previous non-packet based GRClib
API's.

The API does not use magnum's BSUR or BPXL types. Instead, conversion is done from nexus handles and types
directly to BM2MC packet types.

BRect is asserted to be the same as NEXUS_Rect.

Remove all functions that nexus does not use.

Pixel format conversion for fills must be done in nexus.

All calls are non-blocking
***************************************************************************/

#ifndef BGRCLIB_H__
#define BGRCLIB_H__

#include "bstd.h"
#include "berr.h"
#include "bgrc.h"
#include "bgrc_packet.h"

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************
 * Enums
 *****************************************************************************/

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
    BGRCLib_BlitAlphaOp_eCopySource = 0,            /* Use the source alpha for the output alpha (Pa = Sa) */
    BGRCLib_BlitAlphaOp_eUseConstAlpha,             /* Copy the constant parameter for the output alpha (Pa = Ca) */
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
    Rectangle type which matches NEXUS_Rect
 *****************************************************************************/
typedef struct
{
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
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
    BGRClib_GetDefaultBlitParams
 *****************************************************************************/
typedef struct
{
    BM2MC_PACKET_Plane*     srcSurface;  /*  A surface handle list. May be NULL */
    const BRect*            srcRect;     /*  Source area of blit. NULL indicates entire surface. */
    unsigned                srcPaletteOffset;
    BM2MC_PACKET_Plane*     dstSurface;  /*  A surface handle. May be NULL */
    const BRect*            dstRect;     /*  Destination area of blit. NULL indicates entire surface. */
    BM2MC_PACKET_Plane*     outSurface;  /*  A surface handle. Required. */
    const BRect*            outRect;     /*  Output area of blit. NULL indicates entire surface. */

    BGRCLib_BlitColorOp     colorOp;         /*  Selection of color blending equation */
    BGRCLib_BlitAlphaOp     alphaOp;         /*  Selection of alpha blending equation */
    BGRClib_BlendEquation   colorBlend;      /*  Color blending equation (see bgrc.h:BGRC_Blend_SetColorBlend). Used when colorOp==_eUseBlendFactor.
                                                 Otherwise, this data is populated with color equation used according to the colorOp */
    BGRClib_BlendEquation   alphaBlend;      /*  Alpha blending equation (see bgrc.h:BGRC_Blend_SetAlphaBlend). Used when alphaOp==_eUseBlendFactor.
                                                 Otherwise, this data is populated with alpha equation used according to the alphaOp */

    uint32_t                constantColor;       /*  BLEND Constant alpha and color. Only standard color format supported. Used by some operations */
    uint32_t                destConstantColor;   /*  DEST Constant alpha and color. Only standard color format supported. Used by some operations */
    uint32_t                sourceConstantColor; /*  SOURCE Constant alpha and color. Only standard color format supported. Used by some operations */

    BGRC_Output_ColorKeySelection colorKeySelect; /* Select which block provides pixels to output.
                                                      For operations which compute new pixel values, the blend block should be used.
                                                      For operations with palette surfaces, the source is typically the block to be used;
                                                      unless destination colorkeys is enabled, then the destination block should be used. */

    /* Filter to use for scaling */
    BGRC_FilterCoeffs       horzFilter;      /* See BGRC_FilterCoeffs and BGRC_Source_SetFilterCoeffs in bgrc.h */
    BGRC_FilterCoeffs       vertFilter;      /* See BGRC_FilterCoeffs and BGRC_Source_SetFilterCoeffs in bgrc.h */

    bool                    chromaFilter;    /* Applied to both source, destination and output surfaces. See BM2MC_PACKET_PacketSourceControl, BM2MC_PACKET_PacketDestinationControl, and BM2MC_PACKET_PacketOutputControl in bm2mc_packet.h */

    /* Blit Reversal Flags */
    bool                    mirrorSrcVertically;
    bool                    mirrorSrcHorizontally;
    bool                    mirrorDstVertically;
    bool                    mirrorDstHorizontally;
    bool                    mirrorOutVertically;
    bool                    mirrorOutHorizontally;

    bool                    srcAlphaPremult;    /* Enable/Disable Source Alpha Premultiplication */
    BM2MC_PACKET_eFilterOrder sourceFilterOrder; /* order of scale, color key and color matrix applied to source */

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
    const int32_t*          conversionMatrix;/* Color conversion matrix. NULL indicates no matrix provided. */
    uint32_t                matrixShift;     /* Matrix shift value. See BGRC_Source_SetColorMatrix5x4() for more info */

} BGRClib_BlitMatrixParams;

/*****************************************************************************
  Summary:
    Param block for DestripeBlit routine

  Description:
    The param block provides all of the parameters for DestripeBlit function.

  See Also:
    BGRClib_DestripeBlit
    BGRClib_GetDefaultDestripeBlitParams
 *****************************************************************************/
typedef struct
{
    const BM2MC_PACKET_StripedPlane*  srcSurface;   /* top/bottom luma/chroma surfaces */
    const BRect*                      srcRect;      /* Source area of blit. NULL indicates entire surface. */
    const BM2MC_PACKET_Plane*         outSurface;   /* output surface */
    const BRect*                      outRect;      /* Output area of blit. NULL indicates entire surface. */

    /* Filter to use for scaling */
    BGRC_FilterCoeffs       horzFilter;      /* See BGRC_FilterCoeffs and BGRC_Source_SetFilterCoeffs in bgrc.h */
    BGRC_FilterCoeffs       vertFilter;      /* See BGRC_FilterCoeffs and BGRC_Source_SetFilterCoeffs in bgrc.h */

    bool                    chromaFilter;    /* Applied to both source and output surfaces. See BM2MC_PACKET_PacketSourceControl and BM2MC_PACKET_PacketOutputControl in bm2mc_packet.h */

    BGRClib_BlitMatrixParams matrixParams;
} BGRClib_DestripeBlitParams;

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

    bool                    enableColorKey;  /*  Enable colorkeying on the source surface */
    uint32_t                colorKeyLower;   /*  Lower colorkey value. Only standard color format supported. */
    uint32_t                colorKeyUpper;   /*  Upper colorkey value. Only standard color format supported. */
    uint32_t                colorKeyMask;    /*  Mask to use when comparing color key to source color */
    uint32_t                colorKeyReplace; /*  Replacement color */
    uint32_t                colorKeyRplMask; /*  Mask for replacement color */

    bool                    enableDstColorKey;  /*  Enable colorkeying on the source surface */
    uint32_t                dstColorKeyLower;   /*  Lower colorkey value. Only standard color format supported. */
    uint32_t                dstColorKeyUpper;   /*  Upper colorkey value. Only standard color format supported. */
    uint32_t                dstColorKeyMask;    /*  Mask to use when comparing color key to source color */
    uint32_t                dstColorKeyReplace; /*  Replacement color */
    uint32_t                dstColorKeyRplMask; /*  Mask for replacement color */

    BGRC_Output_ColorKeySelection   cksOnlySrcColorKeyed;
    BGRC_Output_ColorKeySelection   cksOnlyDstColorKeyed;
    BGRC_Output_ColorKeySelection   cksBothSrcDstColorKeyed;
    BGRC_Output_ColorKeySelection   cksNeitherSrcDstColorKeyed;

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
    uint8_t  ropVector;       /* 8 bit ROP vector. */
    uint8_t* pattern;         /* 8x8 bit ROP pattern. */
    uint32_t backColor;       /* ROP background color. */
    uint32_t foreColor;       /* ROP foreground color. */

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
    bool     setFilterPhaseAdj;         /* true: Set the filter phase adjustment */
    int32_t  iHorizontalPhase;          /* Horizontal initial phase. */
    int32_t  iVerticalPhase;            /* Vertical initial phase. */
    uint32_t ulPhaseFixedPointShift;    /* Fixed-point shift, in bits, to allow for fractional values. */

    /* BGRC_Source_SetFixedScaleFactor */
    bool     setFixedScaleFactor;       /* true: Set the fixed scale factor */
    uint32_t ulHorizontalNumerator;     /* Horizontal/Source numerator */
    uint32_t ulHorizontalDenominator;   /* Horizontal/Source denominator */
    uint32_t ulVerticalNumerator;       /* Vertical/Output numerator */
    uint32_t ulVerticalDenominator;     /* Vertical/Output denominator */

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
    BGRC_Handle      grcHandle,     /*  An initialized BGRC module handle. */
    BGRC_PacketContext_Handle packetContext
);


/*****************************************************************************
  Summary:
    Close a BGRClib_Handle.

  Description:
    This function frees the BGRClib_Handle structure and all associated data.

 *****************************************************************************/
void BGRClib_Close(
    BGRClib_Handle grclibHandle /* A valid BGRClib_Handle object */
);


BERR_Code BGRClib_Memset32(
    BGRClib_Handle grclib,
    uint32_t offset,
    uint32_t data,
    uint32_t count
);

/*****************************************************************************
  Summary:
    Populate param block with default values

  Description:
    Set-up the destripe blit param block for typical values for performing a
    destripe blit

  See Also:
    BGRClib_DestripeBlit
 *****************************************************************************/
void BGRClib_GetDefaultDestripeBlitParams(
    BGRClib_DestripeBlitParams *params
);

/*****************************************************************************
  Summary:
    destripe a striped 4:2:0 video picture into a single surface.

  Description:
    Video picture decoded by video decoder might be stored in top/bottom
    Luma/Chroma surfaces, with 4:2:0 format and in stripe mode. This routine
    trasfer the picture into a single surface format.

  Returns:
    BERR_SUCCESS                 - The surface fill completed.
    BERR_INVALID_PARAMETER       - One of the supplied parameters was invalid,
                                   possibly NULL.
    BGRC_ERR_M2MC_DEVICE_IS_HUNG - M2MC device timed-out

  See Also:

 *****************************************************************************/
BERR_Code BGRClib_Destripe_Blit(
    BGRClib_Handle grclib,
    const BGRClib_DestripeBlitParams*       params          /*  Destripe blit parameters */
);

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
 *****************************************************************************/
BERR_Code BGRClib_Fill(
    BGRClib_Handle            grclibHandle,    /*  A valid BGRClib_Handle object */
    const BM2MC_PACKET_Plane *pSurface,
    uint32_t                  constantcolor,   /*  Color, either in color format of the surface */
                                               /*       or in the standard color format */
    const BRect*              pRect            /*  Pointer to a rect specifying the part of a */
                                               /*       surface to fill, or NULL to fill the entire surface */
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
    BGRClib_Handle          grclibHandle,    /*  A valid BGRClib_Handle object */
    const BM2MC_PACKET_Plane     *pSurface,
    uint32_t                constantColor,   /*  Color, either in C3C2C3C1 format */
                                             /*       or the specified format */
    const BRect*            pRect,           /*  Pointer to a rect specifying the part of a */
                                             /*       surface to fill, or NULL to fill the entire surface */
    BGRCLib_FillOp          colorOp,         /*  Color operation */
    BGRCLib_FillOp          alphaOp          /*  Alpha operation */
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
    BGRClib_Handle            grclibHandle,    /*  A valid BGRClib_Handle object */
    BGRCLib_PorterDuffOp      pdOp,            /*  A Porter-Duff operation */
    const BM2MC_PACKET_Plane *pSurface,
    uint32_t                  constantColor,   /*  Color, either in C3C2C3C1 format */
                                               /*       or the specified format */
    const BRect*              pRect            /*  Pointer to a rect specifying the part of a */
                                               /*       surface to fill, or NULL to fill the entire surface */
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
    BGRClib_Handle            grclibHandle,  /*  A valid BGRClib_Handle object */
    BGRCLib_PorterDuffOp      pdOp,          /*  A Porter-Duff operation */

    const BM2MC_PACKET_Plane *pSrcSurface,
    const BRect*              pSrcRect,      /*  Pointer to a rect specifying the part of a */
                                             /*       surface to fill, or NULL to fill the entire surface */
    unsigned paletteOffset,
    const BM2MC_PACKET_Plane *pDstSurface,
    const BRect*              pDstRect,      /*  Pointer to a rect specifying the part of a */
                                             /*       surface to fill, or NULL to fill the entire surface */
    const BM2MC_PACKET_Plane *pOutSurface,
    const BRect*              pOutRect       /*  Pointer to a rect specifying the part of a */
                                             /*       surface to fill, or NULL to fill the entire surface */
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
    Blit operation

  Description:


  See Also:
    BGRClib_Blit
 *****************************************************************************/
BERR_Code BGRClib_Blit(
    BGRClib_Handle                          grclibHandle,    /*  A valid BGRClib_Handle object */
    const BGRClib_BlitParams*               params,          /*  General blit parameters */
    const BGRClib_BlitColorKeyParams*       colorkeyParams,  /*  Optional colorkey parameters. Use NULL if not using colorkeying */
    const BGRClib_BlitMatrixParams*         matrixParams,    /*  Optional color conversion matrix parameters. Use NULL if not using color matrix */
    const BGRClib_BlitPatternParams*        patternParams,   /*  Optional (ROP) pattern parameters. Use NULL if not using pattern block */
    const BGRClib_BlitScalingControlParams* scalingParams    /*  Optional scaling control parameters. Use NULL for default scaling behavior. */
);

#ifdef __cplusplus
}
#endif

#endif /* BGRCLIB_H__ */
