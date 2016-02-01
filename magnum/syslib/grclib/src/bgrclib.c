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
#include "bgrclib.h"
#include "bstd.h"
#include "berr.h"
#include "berr_ids.h"
#include "bdbg.h"
#include "bkni.h"

BDBG_MODULE(BGRClib);

/***************************************************************************
* Forward declarations of static (private) functions
***************************************************************************/

#define GRCLIB_CHECK(grclibh) BDBG_ASSERT(grclibh != NULL && grclibh->hGRC != NULL)

/* This macro allows a concise representation of the BGRC_Blend_Source enum */
#define B_BBS(x) (BGRC_Blend_Source_e ## x)

static BERR_Code BGRClib_p_Go(
    BGRClib_Handle          grclibHandle,
    BGRClib_PendType        pendtype
);

static BERR_Code BGRClib_p_ConvertColorToStdColor(
    BSUR_Surface_Handle     surface,
    uint32_t                constantcolor,
    BGRClib_ColorFmtType    fmtType,
    uint32_t*               pColor
);

static BERR_Code BGRClib_p_GetValidPositionInfo(
    const BRect*            pRect,
    BSUR_Surface_Handle     surface,
    uint32_t*               pX,
    uint32_t*               pY,
    uint32_t*               pWidth,
    uint32_t*               pHeight);

#define PBRECT_WIDTH(r)  (r->right - r->left)
#define PBRECT_HEIGHT(r) (r->bottom - r->top)

/***************************************************************************
* Implementation of "BGRClib_" API functions
***************************************************************************/

typedef struct BGRClib_P_Data_tag
{
    BGRC_Handle     hGRC;
    BGRC_Callback   callback_isr;
    void*           callback_data;

    /* Data to optimize calls in to GRC */
    uint32_t srcColor;
    uint32_t dstColor;
    uint32_t bldColor;

    uint32_t srcX, srcY, srcWidth, srcHeight;
    uint32_t dstX, dstY, dstWidth, dstHeight;
    uint32_t outX, outY, outWidth, outHeight;

    BGRC_Output_ColorKeySelection selA;
    BGRC_Output_ColorKeySelection selB;
    BGRC_Output_ColorKeySelection selC;
    BGRC_Output_ColorKeySelection selD;


} BGRClib_P_Data;

/***************************************************************************/

static void BGRClib_p_ClearOptimized(BGRClib_Handle  grclibH)
{
    /* Reset all values for optimizing calls into GRC */
    grclibH->selA = -1;
    grclibH->selB = -1;
    grclibH->selC = -1;
    grclibH->selD = -1;
    grclibH->srcColor = 0xdeadbeef;
    grclibH->dstColor = 0xdeadbeef;
    grclibH->bldColor = 0xdeadbeef;
    grclibH->outX = grclibH->outY = grclibH->outWidth = grclibH->outHeight = 0xdeadbeef;
    grclibH->srcX = grclibH->srcY = grclibH->srcWidth = grclibH->srcHeight = 0xdeadbeef;
    grclibH->dstX = grclibH->dstY = grclibH->dstWidth = grclibH->dstHeight = 0xdeadbeef;

}

/***************************************************************************/

BERR_Code BGRClib_Open( 
    BGRClib_Handle*  pgrclibHandle,
    BGRC_Handle      grcHandle
)
{
    BGRClib_Handle grclibH;

    BDBG_ENTER(BGRClib_Open);

    if (!pgrclibHandle || !grcHandle)
    {
        BDBG_ERR(("Invalid parameter\n"));
        BDBG_LEAVE(BGRClib_Open);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Alloc the main GRC context. */
    grclibH = (BGRClib_Handle)(BKNI_Malloc(sizeof(BGRClib_P_Data)));

    if (!grclibH)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset(grclibH, 0x0, sizeof(BGRClib_P_Data));
    BGRClib_p_ClearOptimized(grclibH);

    /* Store the GRC handle */
    grclibH->hGRC = grcHandle;

    /* All done. now return the new fresh context to user. */
    *pgrclibHandle = grclibH;

    BDBG_LEAVE(BGRClib_Open);
    return BERR_SUCCESS;
}


/***************************************************************************/
void BGRClib_Close( 
    BGRClib_Handle grclibHandle
)
{
    BDBG_ENTER(BGRClib_Close);
    GRCLIB_CHECK(grclibHandle);

    /* Release context in system memory */
    BKNI_Free((void*)grclibHandle);

    BDBG_LEAVE(BGRClib_Close);
}

/***************************************************************************/
BERR_Code BGRClib_ResetState( 
    BGRClib_Handle grclibHandle /* [in] A valid BGRClib_Handle object */
)
{
    BERR_Code err;

    BDBG_ENTER(BGRClib_ResetGRCState);
    GRCLIB_CHECK(grclibHandle);

    /* Reset the GRC */
    err = BGRC_ResetState(grclibHandle->hGRC);

    /* Clear all internal state info */
    BGRClib_p_ClearOptimized(grclibHandle);

    BDBG_LEAVE(BGRClib_ResetGRCState);

    return BERR_TRACE(err);
}

/***************************************************************************/
void BGRClib_SetCallback( 
    BGRClib_Handle grclibHandle,
    BGRC_Callback  callback_isr,
    void*          callback_data
)
{
    BDBG_ENTER(BGRClib_SetCallback);
    GRCLIB_CHECK(grclibHandle);

    grclibHandle->callback_isr  = callback_isr;
    grclibHandle->callback_data = callback_data;

    BDBG_LEAVE(BGRClib_SetCallback);
}


/*****************************************************************************/
BERR_Code BGRClib_Fill(
    BGRClib_Handle          grclibHandle,
    BSUR_Surface_Handle     surface,
    uint32_t                constantColor,
    BGRClib_ColorFmtType    fmtType,
    const BRect*            pRect,
    BGRClib_PendType        pendtype
)
{
    BERR_Code   err = BERR_SUCCESS;
    BGRC_Handle grc;
    uint32_t    color;
    uint32_t    x, y, width, height;

    BDBG_ENTER(BGRClib_Fill);
    GRCLIB_CHECK(grclibHandle);
    BDBG_ASSERT(surface);

    grc = grclibHandle->hGRC;

    /* Get the rectangle (x, y, width, height) for the surface. */
    /* This routine handles the case where the rect is NULL */
    if (err == BERR_SUCCESS)
        err = BGRClib_p_GetValidPositionInfo(pRect, surface, &x, &y, &width, &height);

    /* Convert the color (if necessary) from the user format (which matches the */
    /* surface format) to the standard color format (A, C2, C2, C1) which is */
    /* required by the GRC module and M2MC core */
    if (err == BERR_SUCCESS)
        err = BGRClib_p_ConvertColorToStdColor(surface, constantColor, fmtType, &color);

    /* Clear the input (source and destination) surfaces */
    if (err == BERR_SUCCESS)
    {
        err = BGRC_Destination_SetSurface(grc, NULL);
    }

    if (err == BERR_SUCCESS)
	{
        err = BGRC_Source_SetSurface(grc, NULL);
    }

    /* Set the color in the source. Since the surface is NULl (above) the */
    /* M2MC will pull the color and alpha data from the value instead */
    if (err == BERR_SUCCESS && grclibHandle->srcColor != color)
    {
        err = BGRC_Source_SetColor(grc, color);
        grclibHandle->srcColor = color;
    }

    /* Set the output rectangle and surface */
    if (err == BERR_SUCCESS &&
        (grclibHandle->outX != x || grclibHandle->outY != y ||
         grclibHandle->outWidth != width || grclibHandle->outHeight != height))
    {
        err = BGRC_Output_SetRectangle(grc, x, y, width, height);
        grclibHandle->outX = x;
        grclibHandle->outY = y;
        grclibHandle->outWidth  = width;
        grclibHandle->outHeight = height;
    }

    if (err == BERR_SUCCESS)
    {
        err = BGRC_Output_SetSurface(grc, surface);
    }

    /* Specify that only the source is desired (the second param) */
    if (err == BERR_SUCCESS &&
        (grclibHandle->selA != BGRC_Output_ColorKeySelection_eTakeSource      || 
         grclibHandle->selB != BGRC_Output_ColorKeySelection_eTakeSource      ||
         grclibHandle->selC != BGRC_Output_ColorKeySelection_eTakeDestination || 
         grclibHandle->selD != BGRC_Output_ColorKeySelection_eTakeDestination))
    {
        err = BGRC_Output_SetColorKeySelection(grc,
            BGRC_Output_ColorKeySelection_eTakeSource,
            BGRC_Output_ColorKeySelection_eTakeSource,
            BGRC_Output_ColorKeySelection_eTakeDestination,
            BGRC_Output_ColorKeySelection_eTakeDestination);

        grclibHandle->selA = BGRC_Output_ColorKeySelection_eTakeSource;
        grclibHandle->selB = BGRC_Output_ColorKeySelection_eTakeSource;
        grclibHandle->selC = BGRC_Output_ColorKeySelection_eTakeDestination;
        grclibHandle->selD = BGRC_Output_ColorKeySelection_eTakeDestination;
    }

    /* Get it going */
    if (err == BERR_SUCCESS)
        err = BGRClib_p_Go(grclibHandle, pendtype);

    BDBG_LEAVE(BGRClib_Fill);
    return BERR_TRACE(err);
}


/*****************************************************************************/
BERR_Code BGRClib_Copy(
    BGRClib_Handle          grclibHandle,
    BSUR_Surface_Handle     srcSurface,
    const BRect*            pSrcRect,
    BSUR_Surface_Handle     outSurface,
    const BRect*            pOutRect,
    BGRClib_PendType        pendtype

)
{
    BERR_Code   err = BERR_SUCCESS;
    BGRC_Handle grc;
    uint32_t    xSrc, ySrc, widthSrc, heightSrc;
    uint32_t    xOut, yOut, widthOut, heightOut;
    BPXL_Format outFormat;
    uint32_t    paletteToggled = false;

    BDBG_ENTER(BGRClib_Copy);
    GRCLIB_CHECK(grclibHandle);
    BDBG_ASSERT(srcSurface && outSurface);

    grc = grclibHandle->hGRC;

    /* Get the rectangle (x, y, width, height) for the source and output surfaces. */
    /* This routine handles the case where the rect is NULL */
    if (err == BERR_SUCCESS)
        err = BGRClib_p_GetValidPositionInfo(pSrcRect, srcSurface, &xSrc, &ySrc, &widthSrc, &heightSrc);

    if (err == BERR_SUCCESS)
        err = BGRClib_p_GetValidPositionInfo(pOutRect, outSurface, &xOut, &yOut, &widthOut, &heightOut);

    /* Set the source surface and rectangle */
    if (err == BERR_SUCCESS)
    {
        err = BGRC_Source_SetSurface(grc, srcSurface);
    }

    if (err == BERR_SUCCESS &&
       ((grclibHandle->srcX != xSrc || grclibHandle->srcY != ySrc || 
         grclibHandle->srcWidth != widthSrc || grclibHandle->srcHeight != heightSrc)))
    {
        err = BGRC_Source_SetRectangle(grc, xSrc, ySrc, widthSrc, heightSrc);
        grclibHandle->srcX = xSrc;
        grclibHandle->srcY = ySrc;
        grclibHandle->srcWidth  = widthSrc;
        grclibHandle->srcHeight = heightSrc;
    }

    /* Clear the destination surface */
    if (err == BERR_SUCCESS)
    {
        err = BGRC_Destination_SetSurface(grc, NULL);
    }

    /* Set the output surface and rectangle */
    if (err == BERR_SUCCESS)
    {
        err = BGRC_Output_SetSurface(grc, outSurface);
    }

    if (err == BERR_SUCCESS &&
        (grclibHandle->outX != xOut || grclibHandle->outY != yOut ||
         grclibHandle->outWidth != widthOut || grclibHandle->outHeight != heightOut))
    {
        err = BGRC_Output_SetRectangle(grc, xOut, yOut, widthOut, heightOut);
        grclibHandle->outX = xOut;
        grclibHandle->outY = yOut;
        grclibHandle->outWidth  = widthOut;
        grclibHandle->outHeight = heightOut;
    }

    /* Specify that this is a copy by using _eTakeSource for the second param */
    if (err == BERR_SUCCESS &&
        (grclibHandle->selA != BGRC_Output_ColorKeySelection_eTakeSource      || 
         grclibHandle->selB != BGRC_Output_ColorKeySelection_eTakeSource      ||
         grclibHandle->selC != BGRC_Output_ColorKeySelection_eTakeDestination || 
         grclibHandle->selD != BGRC_Output_ColorKeySelection_eTakeDestination))
    {
        err = BGRC_Output_SetColorKeySelection(grc,
            BGRC_Output_ColorKeySelection_eTakeSource,
            BGRC_Output_ColorKeySelection_eTakeSource,
            BGRC_Output_ColorKeySelection_eTakeDestination,
            BGRC_Output_ColorKeySelection_eTakeDestination);

        grclibHandle->selA = BGRC_Output_ColorKeySelection_eTakeSource; 
        grclibHandle->selB = BGRC_Output_ColorKeySelection_eTakeSource;
        grclibHandle->selC = BGRC_Output_ColorKeySelection_eTakeDestination;
        grclibHandle->selD = BGRC_Output_ColorKeySelection_eTakeDestination;
    }

    /* If the destination a palette surface, then bypass the look-up so we can */
    /* get the source palette values into the destination. */
    if (err == BERR_SUCCESS)
        err = BSUR_Surface_GetFormat(outSurface, &outFormat);

    if (err == BERR_SUCCESS && BPXL_IS_PALETTE_FORMAT(outFormat))
    {
        paletteToggled = true;
        err = BGRC_Source_TogglePaletteBypass(grc, true);
    }

    if (err == BERR_SUCCESS && BPXL_IS_PALETTE_FORMAT(outFormat))
        err = BGRC_Destination_TogglePaletteBypass(grc, true);

    /* Start the operation */
    if (err == BERR_SUCCESS)
        err = BGRClib_p_Go(grclibHandle, pendtype);

    if (err == BERR_SUCCESS && paletteToggled)
        err = BGRC_Source_TogglePaletteBypass(grc, false);

    if (err == BERR_SUCCESS && paletteToggled)
        err = BGRC_Destination_TogglePaletteBypass(grc, false);

    BDBG_LEAVE(BGRClib_Copy);
    return BERR_TRACE(err);
} 


/*****************************************************************************/


static const struct
{
    BGRClib_BlendEquation c;
    BGRClib_BlendEquation a;
} PDFills[] =
{
    {   /* Porter-Duff: Clear           */
        /* Pc = 0                       */
        /* Pa = 0                       */
        { B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: Src             */
        /* Pc = Sc                      */
        /* Pa = Sa                      */
        { B_BBS(ConstantColor), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(ConstantAlpha), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: Dst             */
        /* Pc = Dc                      */
        /* Pa = Da                      */
        { B_BBS(SourceColor), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(SourceAlpha), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: SrcOver         */
        /* Pc = Sc + Dc * (1 - Sa)      */
        /* Pa = Sa + Da * (1 - Sa)      */
        { B_BBS(ConstantColor), B_BBS(One), false, B_BBS(SourceColor), B_BBS(InverseConstantAlpha), false, B_BBS(Zero) },
        { B_BBS(ConstantAlpha), B_BBS(One), false, B_BBS(SourceAlpha), B_BBS(InverseConstantAlpha), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: DstOver         */
        /* Pc = Sc * (1 - Da) + Dc      */
        /* Pa = Sa * (1 - Da) + Da      */
        { B_BBS(ConstantColor), B_BBS(InverseSourceAlpha), false, B_BBS(SourceColor), B_BBS(One), false, B_BBS(Zero) },
        { B_BBS(ConstantAlpha), B_BBS(InverseSourceAlpha), false, B_BBS(SourceAlpha), B_BBS(One), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: SrcIn           */
        /* Pc = Sc * Da                 */
        /* Pa = Sa * Da                 */
        { B_BBS(ConstantColor), B_BBS(SourceAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(ConstantAlpha), B_BBS(SourceAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: DstIn            */
        /* Pc = Dc * Sa                  */
        /* Pa = Da * Sa                  */
        { B_BBS(SourceColor), B_BBS(ConstantAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(SourceAlpha), B_BBS(ConstantAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: SrcOut           */
        /* Pc = Sc * (1 - Da)            */
        /* Pa = Sa * (1 - Da)            */
        { B_BBS(ConstantColor), B_BBS(InverseSourceAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(ConstantAlpha), B_BBS(InverseSourceAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: DstOut            */
        /* Pc = Dc * (1 - Sa)             */
        /* Pa = Da * (1 - Sa)             */
        { B_BBS(SourceColor), B_BBS(InverseConstantAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(SourceAlpha), B_BBS(InverseConstantAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: SrcAtop           */
        /* Pc = Sc * Da + Dc * (1 - Sa)   */
        /* Pa = Da                        */
        { B_BBS(ConstantColor), B_BBS(SourceAlpha), false, B_BBS(SourceColor), B_BBS(InverseConstantAlpha), false, B_BBS(Zero) },
        { B_BBS(SourceAlpha),   B_BBS(One),         false, B_BBS(Zero),        B_BBS(Zero),                 false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: DstAtop           */
        /* Pc = Sc * (1 - Da) + Dc * Sa   */
        /* Pa = Sa                        */
        { B_BBS(ConstantColor), B_BBS(InverseSourceAlpha), false, B_BBS(SourceColor), B_BBS(ConstantAlpha), false, B_BBS(Zero) },
        { B_BBS(ConstantAlpha), B_BBS(One),                false, B_BBS(Zero),        B_BBS(Zero),          false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: Xor                   */
        /* Pc = Sc * (1 - Da) + Dc * (1 - Sa) */
        /* Pa = Sa * (1 - Da) + Da * (1 - Sa) */
        { B_BBS(ConstantColor), B_BBS(InverseSourceAlpha), false, B_BBS(SourceColor), B_BBS(InverseConstantAlpha), false, B_BBS(Zero) },
        { B_BBS(ConstantAlpha), B_BBS(InverseSourceAlpha), false, B_BBS(SourceAlpha), B_BBS(InverseConstantAlpha), false, B_BBS(Zero) }
    }
};


/*****************************************************************************/
static BERR_Code BGRClib_p_Blended_Fill(
    BGRClib_Handle          grclibHandle,
    BSUR_Surface_Handle     surface,
    uint32_t                constantColor,
    BGRClib_ColorFmtType    fmtType,
    const BRect*            pRect
)
{
    BERR_Code   err = BERR_SUCCESS;
    BGRC_Handle grc;
    uint32_t    color;
    uint32_t    x, y, width, height;

    grc = grclibHandle->hGRC;

    /* Get the rectangle (x, y, width, height) for the surface. */
    /* This routine handles the case where the rect is NULL */
    if (err == BERR_SUCCESS)
        err = BGRClib_p_GetValidPositionInfo(pRect, surface, &x, &y, &width, &height);

    /* Convert the color (if necessary) from the user format (which matches the */
    /* surface format) to the standard color format (A, C2, C2, C1) which is */
    /* required by the GRC module and M2MC core */
    if (err == BERR_SUCCESS)
        err = BGRClib_p_ConvertColorToStdColor(surface, constantColor, fmtType, &color);

    /* Clear the destination surface */
    if (err == BERR_SUCCESS)
    {
        err = BGRC_Destination_SetSurface(grc, NULL);
    }

    /* Set the Source info. Note that we are setting the surface */
    if (err == BERR_SUCCESS)
    {
        err = BGRC_Source_SetSurface(grc, surface);
    }

    if (err == BERR_SUCCESS && grclibHandle->srcColor != color) 
    {
        err = BGRC_Source_SetColor(grc, color);
        grclibHandle->srcColor = color;
    }

    if (err == BERR_SUCCESS &&
        (grclibHandle->srcX != x || grclibHandle->srcY != y || 
         grclibHandle->srcWidth != width || grclibHandle->srcHeight != height))
    {
        err = BGRC_Source_SetRectangle(grc, x, y, width, height);
        grclibHandle->srcX = x;
        grclibHandle->srcY = y;
        grclibHandle->srcWidth  = width;
        grclibHandle->srcHeight = height;
    }
        
    /* Set the Blend color */
    if (err == BERR_SUCCESS && grclibHandle->bldColor != color)
    {
        err = BGRC_Blend_SetColor(grc, color);
        grclibHandle->bldColor = color;
    }

    /* Set the output */
    if (err == BERR_SUCCESS &&
        (grclibHandle->outX != x || grclibHandle->outY != y ||
         grclibHandle->outWidth != width || grclibHandle->outHeight != height))
    {
        err = BGRC_Output_SetRectangle(grc, x, y, width, height);
        grclibHandle->outX = x;
        grclibHandle->outY = y;
        grclibHandle->outWidth  = width;
        grclibHandle->outHeight = height;
    }

    if (err == BERR_SUCCESS)
    {
        err = BGRC_Output_SetSurface(grc, surface);
    }

    /* Specify that only the source is desired (the second param) */
    if (err == BERR_SUCCESS &&
        (grclibHandle->selA != BGRC_Output_ColorKeySelection_eTakeBlend       ||
         grclibHandle->selB != BGRC_Output_ColorKeySelection_eTakeSource      ||
         grclibHandle->selC != BGRC_Output_ColorKeySelection_eTakeDestination || 
         grclibHandle->selD != BGRC_Output_ColorKeySelection_eTakeDestination))
    {
        err = BGRC_Output_SetColorKeySelection(grc,
            BGRC_Output_ColorKeySelection_eTakeBlend, /* <--- Takes color from blend block */
            BGRC_Output_ColorKeySelection_eTakeSource,
            BGRC_Output_ColorKeySelection_eTakeDestination,
            BGRC_Output_ColorKeySelection_eTakeDestination);

        grclibHandle->selA = BGRC_Output_ColorKeySelection_eTakeBlend;
        grclibHandle->selB = BGRC_Output_ColorKeySelection_eTakeSource;
        grclibHandle->selC = BGRC_Output_ColorKeySelection_eTakeDestination;
        grclibHandle->selD = BGRC_Output_ColorKeySelection_eTakeDestination;
    }

    return BERR_TRACE(err);
}

/*****************************************************************************/
BERR_Code BGRClib_Blended_Fill(
    BGRClib_Handle          grclibHandle,
    BSUR_Surface_Handle     surface,
    uint32_t                constantColor,
    BGRClib_ColorFmtType    fmtType,
    const BRect*            pRect,
    BGRCLib_FillOp          colorOp,
    BGRCLib_FillOp          alphaOp,
    BGRClib_PendType        pendtype
)
{
    BERR_Code   err;
    BGRC_Handle grc;

    BDBG_ENTER(BGRClib_Blended_Fill);
    GRCLIB_CHECK(grclibHandle);
    BDBG_ASSERT(surface);

    err = BGRClib_p_Blended_Fill(grclibHandle, surface, constantColor, fmtType, pRect);
    grc = grclibHandle->hGRC;

    /* Set the COLOR blend equation depending on the operation requested */
    if (err != BERR_SUCCESS)
        ;
    else if (colorOp == BGRCLib_FillOp_eIgnore)
    {
        err = BGRC_Blend_SetColorBlend(grc,
                   BGRC_Blend_Source_eSourceColor, BGRC_Blend_Source_eOne,
                   false, BGRC_Blend_Source_eZero, BGRC_Blend_Source_eZero,
                   false, BGRC_Blend_Source_eZero);
    }
    else if (colorOp == BGRCLib_FillOp_eCopy)
    {
        err = BGRC_Blend_SetColorBlend(grc,
                   BGRC_Blend_Source_eConstantColor, BGRC_Blend_Source_eOne,
                   false, BGRC_Blend_Source_eZero, BGRC_Blend_Source_eZero,
                   false, BGRC_Blend_Source_eZero);
    }
    else if (colorOp == BGRCLib_FillOp_eBlend)
    {
        err = BGRC_Blend_SetColorBlend(grc,
                   BGRC_Blend_Source_eConstantColor, BGRC_Blend_Source_eConstantAlpha,
                   false, BGRC_Blend_Source_eSourceColor, BGRC_Blend_Source_eInverseConstantAlpha,
                   false, BGRC_Blend_Source_eZero);
    }
    else
    {
        BDBG_ASSERT(false);
        err = BERR_INVALID_PARAMETER;
    }

    /* Set the ALPHA blend equation depending on the operation requested */
    if (err != BERR_SUCCESS)
        ;
    else if (alphaOp == BGRCLib_FillOp_eIgnore)
    {
        err = BGRC_Blend_SetAlphaBlend(grc,
                   BGRC_Blend_Source_eSourceAlpha, BGRC_Blend_Source_eOne,
                   false, BGRC_Blend_Source_eZero, BGRC_Blend_Source_eZero,
                   false, BGRC_Blend_Source_eZero);
    }
    else if (alphaOp == BGRCLib_FillOp_eCopy)
    {
        err = BGRC_Blend_SetAlphaBlend(grc,
                   BGRC_Blend_Source_eConstantAlpha, BGRC_Blend_Source_eOne,
                   false,  BGRC_Blend_Source_eZero,  BGRC_Blend_Source_eZero,
                   false,  BGRC_Blend_Source_eZero);
    }
    else if (alphaOp == BGRCLib_FillOp_eBlend)
    {
        err = BGRC_Blend_SetAlphaBlend(grc,
                   BGRC_Blend_Source_eSourceAlpha, BGRC_Blend_Source_eInverseConstantAlpha,
                   false, BGRC_Blend_Source_eConstantAlpha, BGRC_Blend_Source_eOne,
                   false, BGRC_Blend_Source_eZero);
    }
    else
    {
        BDBG_ASSERT(false);
        err = BERR_INVALID_PARAMETER;
    }

    /* Start the operation */
    if (err == BERR_SUCCESS)
        err = BGRClib_p_Go(grclibHandle, pendtype);

    BDBG_LEAVE(BGRClib_Blended_Fill);
    return BERR_TRACE(err);
}

/*****************************************************************************/
BERR_Code BGRClib_PorterDuffFill(
    BGRClib_Handle          grclibHandle,
    BGRCLib_PorterDuffOp    pdOp,
    BSUR_Surface_Handle     surface,
    uint32_t                constantColor,
    BGRClib_ColorFmtType    fmtType,
    const BRect*            pRect,
    BGRClib_PendType        pendtype
)
{
    BERR_Code   err;
    BGRC_Handle grc;

    BDBG_ENTER(BGRClib_PorterDuffFill);
    GRCLIB_CHECK(grclibHandle);
    BDBG_ASSERT(surface);

    err = BGRClib_p_Blended_Fill(grclibHandle, surface, constantColor, fmtType, pRect);
    grc = grclibHandle->hGRC;

    /* Set the COLOR blend equation depending on the operation requested */
    /* See the table above with the equations */
    if (err == BERR_SUCCESS)
        err = BGRC_Blend_SetColorBlend(grc,
           PDFills[pdOp].c.a, 
           PDFills[pdOp].c.b, 
           PDFills[pdOp].c.subcd, 
           PDFills[pdOp].c.c, 
           PDFills[pdOp].c.d, 
           PDFills[pdOp].c.sube, 
           PDFills[pdOp].c.e);

    if (err == BERR_SUCCESS)
        err = BGRC_Blend_SetAlphaBlend(grc,
           PDFills[pdOp].a.a, 
           PDFills[pdOp].a.b, 
           PDFills[pdOp].a.subcd, 
           PDFills[pdOp].a.c, 
           PDFills[pdOp].a.d, 
           PDFills[pdOp].a.sube, 
           PDFills[pdOp].a.e);

    /* Start the operation */
    if (err == BERR_SUCCESS)
        err = BGRClib_p_Go(grclibHandle, pendtype);

    BDBG_LEAVE(BGRClib_PorterDuffFill);
    return BERR_TRACE(err);
}


/*****************************************************************************/


static const struct
{
    BGRClib_BlendEquation c;
    BGRClib_BlendEquation a;
} PDBlends[] =
{
    {   /* Porter-Duff: Clear           */
        /* Pc = 0                       */
        /* Pa = 0                       */
        { B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: Src             */
        /* Pc = Sc                      */
        /* Pa = Sa                      */
        { B_BBS(SourceColor), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(SourceAlpha), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: Dst             */
        /* Pc = Dc                      */
        /* Pa = Da                      */
        { B_BBS(DestinationColor), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(DestinationAlpha), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: SrcOver         */
        /* Pc = Sc + Dc * (1 - Sa)      */
        /* Pa = Sa + Da * (1 - Sa)      */
        { B_BBS(SourceColor), B_BBS(One), false, B_BBS(DestinationColor), B_BBS(InverseSourceAlpha), false, B_BBS(Zero) },
        { B_BBS(SourceAlpha), B_BBS(One), false, B_BBS(DestinationAlpha), B_BBS(InverseSourceAlpha), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: DstOver         */
        /* Pc = Sc * (1 - Da) + Dc      */
        /* Pa = Sa * (1 - Da) + Da      */
        { B_BBS(SourceColor), B_BBS(InverseDestinationAlpha), false, B_BBS(DestinationColor), B_BBS(One), false, B_BBS(Zero) },
        { B_BBS(SourceAlpha), B_BBS(InverseDestinationAlpha), false, B_BBS(DestinationAlpha), B_BBS(One), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: SrcIn           */
        /* Pc = Sc * Da                 */
        /* Pa = Sa * Da                 */
        { B_BBS(SourceColor), B_BBS(DestinationAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(SourceAlpha), B_BBS(DestinationAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: DstIn            */
        /* Pc = Dc * Sa                  */
        /* Pa = Da * Sa                  */
        { B_BBS(DestinationColor), B_BBS(SourceAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(DestinationAlpha), B_BBS(SourceAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: SrcOut           */
        /* Pc = Sc * (1 - Da)            */
        /* Pa = Sa * (1 - Da)            */
        { B_BBS(SourceColor), B_BBS(InverseDestinationAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(SourceAlpha), B_BBS(InverseDestinationAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: DstOut            */
        /* Pc = Dc * (1 - Sa)             */
        /* Pa = Da * (1 - Sa)             */
        { B_BBS(DestinationColor), B_BBS(InverseSourceAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) },
        { B_BBS(DestinationAlpha), B_BBS(InverseSourceAlpha), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: SrcAtop           */
        /* Pc = Sc * Da + Dc * (1 - Sa)   */
        /* Pa = Da                        */
        { B_BBS(SourceColor), B_BBS(DestinationAlpha), false, B_BBS(DestinationColor), B_BBS(InverseSourceAlpha), false, B_BBS(Zero) },
        { B_BBS(DestinationAlpha), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: DstAtop           */
        /* Pc = Sc * (1 - Da) + Dc * Sa   */
        /* Pa = Sa                        */
        { B_BBS(SourceColor), B_BBS(InverseDestinationAlpha), false, B_BBS(DestinationColor), B_BBS(SourceAlpha), false, B_BBS(Zero) },
        { B_BBS(SourceAlpha), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero) }
    },
    {   /* Porter-Duff: Xor                   */
        /* Pc = Sc * (1 - Da) + Dc * (1 - Sa) */
        /* Pa = Sa * (1 - Da) + Da * (1 - Sa) */
        { B_BBS(SourceColor), B_BBS(InverseDestinationAlpha), false, B_BBS(DestinationColor), B_BBS(InverseSourceAlpha), false, B_BBS(Zero) },
        { B_BBS(SourceAlpha), B_BBS(InverseDestinationAlpha), false, B_BBS(DestinationAlpha), B_BBS(InverseSourceAlpha), false, B_BBS(Zero) }
    }
};



/***************************************************************************/
BERR_Code BGRClib_PorterDuffBlit(
    BGRClib_Handle          grclibHandle,
    BGRCLib_PorterDuffOp    pdOp,
    BSUR_Surface_Handle     srcSurface,
    const BRect*            pSrcRect,
    BSUR_Surface_Handle     dstSurface,
    const BRect*            pDstRect,
    BSUR_Surface_Handle     outSurface,
    const BRect*            pOutRect,
    BGRClib_PendType        pendtype

)
{
    BERR_Code   err = BERR_SUCCESS;
    BGRC_Handle grc;
    uint32_t    xSrc, ySrc, widthSrc, heightSrc;
    uint32_t    xDst, yDst, widthDst, heightDst;
    uint32_t    xOut, yOut, widthOut, heightOut;
    BPXL_Format outFormat;
    uint32_t    paletteToggled = false;

    BDBG_ENTER(BGRClib_PorterDuffBlit);
    GRCLIB_CHECK(grclibHandle);
    BDBG_ASSERT(pdOp < BGRCLib_PorterDuffOp_Count);
    grc = grclibHandle->hGRC;

    /* This is caught in debug build by assert, so catch error in release build */
    if (pdOp >= BGRCLib_PorterDuffOp_Count)
        err = BERR_INVALID_PARAMETER;

    /* Get the rectangle (x, y, width, height) for the source and output surfaces. */
    /* This routine handles the case where the rect is NULL */
    if (err == BERR_SUCCESS)
        err = BGRClib_p_GetValidPositionInfo(pSrcRect, srcSurface, &xSrc, &ySrc, &widthSrc, &heightSrc);

    if (err == BERR_SUCCESS)
        err = BGRClib_p_GetValidPositionInfo(pDstRect, dstSurface, &xDst, &yDst, &widthDst, &heightDst);

    if (err == BERR_SUCCESS)
        err = BGRClib_p_GetValidPositionInfo(pOutRect, outSurface, &xOut, &yOut, &widthOut, &heightOut);

    /* Set the source surface and rectangle */
    if (err == BERR_SUCCESS)
    {
        err = BGRC_Source_SetSurface(grc, srcSurface);
    }

    if (err == BERR_SUCCESS &&
        (grclibHandle->srcX != xSrc || grclibHandle->srcY != ySrc || 
         grclibHandle->srcWidth != widthSrc || grclibHandle->srcHeight != heightSrc))
    {
        err = BGRC_Source_SetRectangle(grc, xSrc, ySrc, widthSrc, heightSrc);
        grclibHandle->srcX = xSrc;
        grclibHandle->srcY = ySrc;
        grclibHandle->srcWidth  = widthSrc;
        grclibHandle->srcHeight = heightSrc;
    }

    /* Set the destination surface and rectangle */
    if (err == BERR_SUCCESS)
    {
        err = BGRC_Destination_SetSurface(grc, dstSurface);
    }

    if (err == BERR_SUCCESS &&
        (grclibHandle->dstX != xDst || grclibHandle->dstY != yDst || 
         grclibHandle->dstWidth != widthDst || grclibHandle->dstHeight != heightDst))
    {
        err = BGRC_Destination_SetRectangle(grc, xDst, yDst, widthDst, heightDst);
        grclibHandle->dstX = xDst;
        grclibHandle->dstY = yDst;
        grclibHandle->dstWidth  = widthDst;
        grclibHandle->dstHeight = heightDst;
    }

    /* Set the output surface and rectangle */
    if (err == BERR_SUCCESS)
    {
        err = BGRC_Output_SetSurface(grc, outSurface);
    }

    if (err == BERR_SUCCESS &&
        (grclibHandle->outX != xOut || grclibHandle->outY != yOut || 
         grclibHandle->outWidth != widthOut || grclibHandle->outHeight != heightOut))
    {
        err = BGRC_Output_SetRectangle(grc, xOut, yOut, widthOut, heightOut);
        grclibHandle->outX = xOut;
        grclibHandle->outY = yOut;
        grclibHandle->outWidth  = widthOut;
        grclibHandle->outHeight = heightOut;
    }

    /* If the output a palette surface, then bypass the look-up so we can */
    /* get the source palette values into the output. */
    if (err == BERR_SUCCESS)
        err = BSUR_Surface_GetFormat(outSurface, &outFormat);

    if (err == BERR_SUCCESS && BPXL_IS_PALETTE_FORMAT(outFormat))
    {
        err = BGRC_Source_TogglePaletteBypass(grc, true);
        paletteToggled = true;
    }

    if (err == BERR_SUCCESS && BPXL_IS_PALETTE_FORMAT(outFormat))
        err = BGRC_Destination_TogglePaletteBypass(grc, true);

    /* Specify that this is a blend by using _eTakeBlend for the second param */
    if (err == BERR_SUCCESS &&
        (grclibHandle->selA != BGRC_Output_ColorKeySelection_eTakeBlend       || 
         grclibHandle->selB != BGRC_Output_ColorKeySelection_eTakeSource      ||
         grclibHandle->selC != BGRC_Output_ColorKeySelection_eTakeDestination || 
         grclibHandle->selD != BGRC_Output_ColorKeySelection_eTakeDestination))
    {
        err = BGRC_Output_SetColorKeySelection(grc,
            BGRC_Output_ColorKeySelection_eTakeBlend,
            BGRC_Output_ColorKeySelection_eTakeSource,
            BGRC_Output_ColorKeySelection_eTakeDestination,
            BGRC_Output_ColorKeySelection_eTakeDestination);

        grclibHandle->selA = BGRC_Output_ColorKeySelection_eTakeBlend;
        grclibHandle->selB = BGRC_Output_ColorKeySelection_eTakeSource;
        grclibHandle->selC = BGRC_Output_ColorKeySelection_eTakeDestination;
        grclibHandle->selD = BGRC_Output_ColorKeySelection_eTakeDestination;
    }

    /* Set the COLOR blend equation depending on the operation requested */
    /* See the table above with the equations */
    if (err == BERR_SUCCESS)
        err = BGRC_Blend_SetColorBlend(grc,
           PDBlends[pdOp].c.a, 
           PDBlends[pdOp].c.b, 
           PDBlends[pdOp].c.subcd, 
           PDBlends[pdOp].c.c, 
           PDBlends[pdOp].c.d, 
           PDBlends[pdOp].c.sube, 
           PDBlends[pdOp].c.e);

    if (err == BERR_SUCCESS)
        err = BGRC_Blend_SetAlphaBlend(grc,
           PDBlends[pdOp].a.a, 
           PDBlends[pdOp].a.b, 
           PDBlends[pdOp].a.subcd, 
           PDBlends[pdOp].a.c, 
           PDBlends[pdOp].a.d, 
           PDBlends[pdOp].a.sube, 
           PDBlends[pdOp].a.e);

    /* Start the operation */
    if (err == BERR_SUCCESS)
        err = BGRClib_p_Go(grclibHandle, pendtype);

    if (err == BERR_SUCCESS && paletteToggled)
        err = BGRC_Source_TogglePaletteBypass(grc, false);

    if (err == BERR_SUCCESS && paletteToggled)
        err = BGRC_Destination_TogglePaletteBypass(grc, false);

    BDBG_LEAVE(BGRClib_PorterDuffBlit);
    return BERR_TRACE(err);;
} 


/*****************************************************************************/
void BGRClib_GetDefaultBlitParams(
    BGRClib_BlitParams *params
)
{
    BDBG_ENTER(BGRClib_GetDefaultBlitParams);
    BDBG_ASSERT(params);

    BKNI_Memset(params, 0, sizeof(BGRClib_BlitParams));

    params->colorOp = BGRCLib_BlitColorOp_eCopySource;
    params->alphaOp = BGRCLib_BlitAlphaOp_eCopySource;


    params->horzFilter = BGRC_FilterCoeffs_eAnisotropic;
    params->vertFilter = BGRC_FilterCoeffs_eAnisotropic;

    params->colorKeySelect = BGRC_Output_ColorKeySelection_eTakeBlend;

    /* Use the same defaults at the GRC module */
    params->constantColor = 0xFF000000;
    params->destContColor = 0xFF000000;

    BDBG_LEAVE(BGRClib_GetDefaultBlitParams);
}

/*****************************************************************************/
void BGRClib_GetDefaultPaletteBlitParams(
    BGRClib_BlitParams *params
)
{
    BDBG_ENTER(BGRClib_GetDefaultPaletteBlitParams);

    /* First, use the overall defaults */
    BGRClib_GetDefaultBlitParams(params);

    /* Use the Point Sample filter */
    params->horzFilter = BGRC_FilterCoeffs_ePointSample;
    params->vertFilter = BGRC_FilterCoeffs_ePointSample;

    /* Use the source surface */
    params->colorKeySelect = BGRC_Output_ColorKeySelection_eTakeSource;

    BDBG_LEAVE(BGRClib_GetDefaultBlitParams);
}

/*****************************************************************************/
void BGRClib_GetDefaultColorKeyParams(
    BGRClib_BlitColorKeyParams *colorkeyparams
)
{
    BDBG_ENTER(BGRClib_GetDefaultColorKeyParams);
    BDBG_ASSERT(colorkeyparams);

    BKNI_Memset(colorkeyparams, 0, sizeof(BGRClib_BlitColorKeyParams));

    colorkeyparams->cksOnlySrcColorKeyed    = BGRC_Output_ColorKeySelection_eTakeDestination;
    colorkeyparams->cksOnlyDstColorKeyed    = BGRC_Output_ColorKeySelection_eTakeSource;
    colorkeyparams->cksBothSrcDstColorKeyed = BGRC_Output_ColorKeySelection_eTakeDestination;

    BDBG_LEAVE(BGRClib_GetDefaultColorKeyParams);
}


/*****************************************************************************/
void BGRClib_GetDefaultScalingControlParams(
    BGRClib_BlitScalingControlParams *scalingparams
)
{
    BDBG_ENTER(BGRClib_GetDefaultScalingControlParams);
    BDBG_ASSERT(scalingparams);

    BKNI_Memset(scalingparams, 0, sizeof(BGRClib_BlitScalingControlParams));

    scalingparams->setFilterPhaseAdj = false;

    scalingparams->setFixedScaleFactor = false;
    scalingparams->ulHorizontalNumerator = 1;
    scalingparams->ulHorizontalDenominator = 1;
    scalingparams->ulVerticalNumerator = 1;
    scalingparams->ulVerticalDenominator = 1;

    BDBG_LEAVE(BGRClib_GetDefaultScalingControlParams);
}

/*****************************************************************************/


static const BGRClib_BlendEquation BlitColorBlends[] =
{
    {   /* _eCopySource: Pc = Sc */
        B_BBS(SourceColor), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero)
    },
    {   /* _eUseConstantAlpha: Pc = Sc * Ca + Dc * (1 - Ca) */
        B_BBS(SourceColor), B_BBS(ConstantAlpha), false, B_BBS(DestinationColor), B_BBS(InverseConstantAlpha), false, B_BBS(Zero)
    },
    {   /* _eUseSourceAlpha: Pc = Sc * Sa + Dc * (1 - Sa) */
        B_BBS(SourceColor), B_BBS(SourceAlpha), false, B_BBS(DestinationColor), B_BBS(InverseSourceAlpha), false, B_BBS(Zero)
    },
    {   /* _eUseDestAlpha: Pc = Sc * Da + Dc * (1 - Da) */
        B_BBS(SourceColor), B_BBS(DestinationAlpha), false, B_BBS(DestinationColor), B_BBS(InverseDestinationAlpha), false, B_BBS(Zero)
    },
    {   /* _eSelectPaletteWithColorkey: Pc = Sc */
        B_BBS(SourceColor), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero)
    },
    {   /* _eModulate:  (Deprecated) */
        B_BBS(SourceColor), B_BBS(SourceAlpha), false, B_BBS(DestinationColor), B_BBS(InverseSourceAlpha), false, B_BBS(Zero)
    },
    {   /* _eAdd:  Pc = Sc + Dc */
        B_BBS(SourceColor), B_BBS(One), false, B_BBS(DestinationColor), B_BBS(One), false, B_BBS(Zero)
    },
    {   /* _eUseBlendFactors: Never used. This entry exists so that the enum list matches the size of this array */
        B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero)
    }
};



static const BGRClib_BlendEquation BlitAlphaBlends[] =
{
    {   /* _eCopySource: Pa = Sa */
        B_BBS(SourceAlpha), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero)
    },
    {   /* _eUseConstAlpha: Pa = Ca */
        B_BBS(ConstantAlpha), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero)
    },
    {   /* _eUseSourceAlpha: Pa = Sa */
        B_BBS(SourceAlpha), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero)
    },
    {   /* _eUseDestAlpha: Pa = Da */
        B_BBS(DestinationAlpha), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero)
    },
    {   /* _eUseCombinedAlpha: Pa = Sa + Da*(1-Sa) */
        B_BBS(SourceAlpha), B_BBS(One), false, B_BBS(InverseSourceAlpha), B_BBS(DestinationAlpha), false, B_BBS(Zero)
    },
    {   /* _eEmulateTransparentVideo: */
        B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero)
    },
    {   /* _eModulate: (Deprecated) */
        B_BBS(SourceAlpha), B_BBS(One), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero)
    },
    {   /* _eAdd: Pa = Sa + Da */
        B_BBS(SourceAlpha), B_BBS(One), false, B_BBS(DestinationAlpha), B_BBS(One), false, B_BBS(Zero)
    },
    {   /* _eUseBlendFactors: Never used. This entry exists so that the enum list matches the size of this array */
        B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero), B_BBS(Zero), false, B_BBS(Zero)
    }
};

/*****************************************************************************/
BERR_Code BGRClib_GetColorBlendFactors(
    BGRCLib_BlitColorOp colorOp, 
    BGRClib_BlendEquation* factors)
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ASSERT(factors != NULL);
    BDBG_ASSERT(colorOp < BGRCLib_BlitColorOp_Count);

    if (colorOp < BGRCLib_BlitColorOp_Count && colorOp != BGRCLib_BlitColorOp_eUseBlendFactors && factors != NULL)
        *factors = BlitColorBlends[colorOp];
    else
        err = BERR_INVALID_PARAMETER;

    return err;
}


/*****************************************************************************/
BERR_Code BGRClib_GetAlphaBlendFactors(
    BGRCLib_BlitAlphaOp alphaOp, 
    BGRClib_BlendEquation* factors)
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ASSERT(factors != NULL);
    BDBG_ASSERT(alphaOp < BGRCLib_BlitAlphaOp_Count);

    if (alphaOp < BGRCLib_BlitAlphaOp_Count && alphaOp != BGRCLib_BlitAlphaOp_eUseBlendFactors && factors != NULL)
        *factors = BlitAlphaBlends[alphaOp];
    else
        err = BERR_INVALID_PARAMETER;

    return err;
}

/*****************************************************************************/
BERR_Code BGRClib_GetPorterDuffBlendFactors(
    BGRCLib_PorterDuffOp pdOp, 
    BGRClib_BlendEquation* colorFactors, 
    BGRClib_BlendEquation* alphaFactors)
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ASSERT(colorFactors != NULL && alphaFactors != NULL);
    BDBG_ASSERT(pdOp < BGRCLib_PorterDuffOp_Count);

    if (pdOp < BGRCLib_PorterDuffOp_Count && colorFactors != NULL && alphaFactors != NULL)
    {
        *colorFactors = PDBlends[pdOp].c;
        *alphaFactors = PDBlends[pdOp].a;
    }
    else
        err = BERR_INVALID_PARAMETER;

    return err;
}

/***************************************************************************/
#define BMAX(a, b) ((a<b)?b:a)
#define BGRCLIB_IS_BLURRY_FILTER(_f_)               \
    (_f_ == BGRC_FilterCoeffs_eBlurry            || \
     _f_ == BGRC_FilterCoeffs_eAntiFlutterBlurry || \
     _f_ == BGRC_FilterCoeffs_eAntiFlutter       || \
     _f_ == BGRC_FilterCoeffs_eAntiFlutterSharp)
     
/***************************************************************************/
BERR_Code BGRClib_Blit(
    BGRClib_Handle                    grclibHandle,
    const BGRClib_BlitParams*         params,
    const BGRClib_BlitColorKeyParams* colorkeyParams,
    const BGRClib_BlitMatrixParams*   matrixParams,
    const BGRClib_BlitPatternParams*  patternParams,
    const BGRClib_BlitScalingControlParams* scalingParams,
    BGRClib_PendType                  pendtype
)
{
    BERR_Code   err = BERR_SUCCESS;
    BGRC_Handle grc;
    uint32_t    xSrc, ySrc, widthSrc, heightSrc;
    uint32_t    xDst, yDst, widthDst, heightDst;
    uint32_t    xOut, yOut, widthOut, heightOut;
    BPXL_Format outFormat;
    uint32_t    maxCnt, index;
    uint32_t    x, y, w, h;
    uint32_t    count;
    uint32_t    matrixToggled   = false;
    uint32_t    colorkeyToggled = false;
    uint32_t    paletteToggled  = false;
    uint32_t    filterToggled   = false;
    uint32_t    scalingToggled  = false;
    uint32_t    patternToggled  = false;

    const BRect*        pRect;
    BSUR_Surface_Handle surf;
    BSUR_Surface_Handle srcSurf, dstSurf,  outSurf;
    BGRClib_BlendEquation       colorBlend;
    BGRClib_BlendEquation       alphaBlend;

    BGRC_Output_ColorKeySelection cksNotSrcNotDst;
    BGRC_Output_ColorKeySelection cksDstNotSrc;
	BGRC_Output_ColorKeySelection cksSrcNotDst;
	BGRC_Output_ColorKeySelection cksSrcDst;

    BDBG_ENTER(BGRClib_Blit);
    GRCLIB_CHECK(grclibHandle);
    BDBG_ASSERT(sizeof(BlitColorBlends)/sizeof(*BlitColorBlends) == BGRCLib_BlitColorOp_Count);
    BDBG_ASSERT(sizeof(BlitAlphaBlends)/sizeof(*BlitAlphaBlends) == BGRCLib_BlitAlphaOp_Count);
    BDBG_ASSERT(params->colorOp < BGRCLib_BlitColorOp_Count && params->alphaOp < BGRCLib_BlitAlphaOp_Count);
    BDBG_ASSERT(params->outSurfaceList && params->outSurfaceCnt);


    /* ---------------------------------------- */
    /* Set-up local variables                   */
    /* ---------------------------------------- */
    grc = grclibHandle->hGRC;

    /* Clamp input values (for coverity and release builds) */
    /* Add support for user-specified blend equation */
    if (params->colorOp == BGRCLib_BlitColorOp_eUseBlendFactors)
        colorBlend = params->colorBlend;
    else if (params->colorOp < BGRCLib_BlitColorOp_Count)
        colorBlend = BlitColorBlends[params->colorOp];
    else
        colorBlend = BlitColorBlends[BGRCLib_BlitColorOp_eCopySource];

    if (params->alphaOp == BGRCLib_BlitAlphaOp_eUseBlendFactors)
        alphaBlend = params->alphaBlend;
    else if (params->colorOp < BGRCLib_BlitAlphaOp_Count)
        alphaBlend = BlitAlphaBlends[params->alphaOp];
    else
        alphaBlend = BlitAlphaBlends[BGRCLib_BlitAlphaOp_eCopySource];

    /* Determine the maximum list length */
    maxCnt = BMAX(params->srcRectCnt, params->dstRectCnt);
    maxCnt = BMAX(maxCnt, params->outRectCnt);
    maxCnt = BMAX(maxCnt, params->dstSurfaceCnt);
    maxCnt = BMAX(maxCnt, params->srcSurfaceCnt);
    maxCnt = BMAX(maxCnt, params->outSurfaceCnt);

    /* Keep track of changes to minimize calls to GRC */
    xSrc = ySrc = widthSrc = heightSrc = 0xffffff;
    xDst = yDst = widthDst = heightDst = 0xffffff;
    xOut = yOut = widthOut = heightOut = 0xffffff;
    srcSurf = dstSurf = outSurf = (BSUR_Surface_Handle)0xffffff;

    /* Initialization to resolve coverity */
    x = 0;
    y = 0;
    w = 0;
    h = 0;
 
    /* Must have at least one output surface */
    if ((params->outSurfaceCnt == 0 || params->outSurfaceList == NULL || *params->outSurfaceList == NULL) && err == BERR_SUCCESS)
        err = BERR_TRACE(BERR_INVALID_PARAMETER);


    /* ---------------------------------------- */
    /* Set-up the common parts of the operation */
    /* ---------------------------------------- */

    /* If the output a palette surface, then bypass the look-up so we can */
    /* get the source palette values into the destination. */
    /* Note that it is expected that the source surface is also a palette. */
    if (err == BERR_SUCCESS)
    {
        surf = *params->outSurfaceList;

        if (err == BERR_SUCCESS)
            err = BSUR_Surface_GetFormat(surf, &outFormat);
    
        if (err == BERR_SUCCESS && BPXL_IS_PALETTE_FORMAT(outFormat))
        {
            paletteToggled = true;
            err = BGRC_Source_TogglePaletteBypass(grc, true);
        }
    
        if (err == BERR_SUCCESS && BPXL_IS_PALETTE_FORMAT(outFormat))
            err = BGRC_Destination_TogglePaletteBypass(grc, true);
    }

    /* Specify which pixels are used: source, destination or blend block */
    /* The blend block should be used for any operation that computes new pixels */
    /* The source block should be used when the output is a palette surface */
    /* If color key selection is activated for the Destination surface, then the */
    /* destination block should be used.  */
    /* Note: If colorkeying is enabled with the params parameter and if the individual */
    /*        source pixel is colorkeyed that the destination color is used.  */
    if (err == BERR_SUCCESS)
    {
        /* Set-up the color key selection values */
        cksNotSrcNotDst = params->colorKeySelect;

        /* Selection when source is color keyed, but destination is not  */
        if (colorkeyParams != NULL && colorkeyParams->enableColorKey)
            cksSrcNotDst = colorkeyParams->cksOnlySrcColorKeyed;
        else
            cksSrcNotDst = BGRC_Output_ColorKeySelection_eTakeDestination;

        /* Selection when destination is color keyed, but source is not */
        if (colorkeyParams != NULL && colorkeyParams->enableDstColorKey)
            cksDstNotSrc = colorkeyParams->cksOnlyDstColorKeyed;
        else
            cksDstNotSrc = BGRC_Output_ColorKeySelection_eTakeSource;

        /* Selection when both source and destination surfaces are colorkeyed */
        if (colorkeyParams != NULL && colorkeyParams->enableColorKey && colorkeyParams->enableDstColorKey)
            cksSrcDst = colorkeyParams->cksBothSrcDstColorKeyed;
        else
            cksSrcDst = BGRC_Output_ColorKeySelection_eTakeDestination;

        /* Set-up the GRC */
        if (grclibHandle->selA != cksNotSrcNotDst || 
            grclibHandle->selB != cksDstNotSrc    ||
            grclibHandle->selC != cksSrcNotDst    || 
            grclibHandle->selD != cksSrcDst)
        {
            err = BGRC_Output_SetColorKeySelection(grc,
                cksNotSrcNotDst, /* No color keying */
                cksDstNotSrc,    /* Src is color keyed, dest is not */
                cksSrcNotDst,    /* Dst is color keyed, src is not */
                cksSrcDst);      /* Both src and dst color keyed */

            grclibHandle->selA = cksNotSrcNotDst;
            grclibHandle->selB = cksDstNotSrc;
            grclibHandle->selC = cksSrcNotDst;
            grclibHandle->selD = cksSrcDst;
        }
    }

    /* Set the COLOR blend equation depending on the operation requested */
    if (err == BERR_SUCCESS)
        err = BGRC_Blend_SetColorBlend(grc, colorBlend.a, colorBlend.b, colorBlend.subcd, colorBlend.c,colorBlend.d, colorBlend.sube, colorBlend.e);

    /* Set the ALPHA blend equation depending on the operation requested */
    if (err == BERR_SUCCESS)
        err = BGRC_Blend_SetAlphaBlend(grc, alphaBlend.a, alphaBlend.b, alphaBlend.subcd, alphaBlend.c, alphaBlend.d, alphaBlend.sube, alphaBlend.e);


    /* Filter Coefficients */
    /* 2008.09.09: Updated logic to enable filtering only for a blurry filter. For scaled blits, the GRC */
    /* will always enable filtering. For unscaled blits, enabling the filter can affect performance, so it */
    /* is only enabled when the filter specified is blurry. */
    /* 2009.04.22: Set the horizontal and vertical flags as necessary. Also, use a macro for readability and consistency. */
    if ((BGRCLIB_IS_BLURRY_FILTER(params->horzFilter) || BGRCLIB_IS_BLURRY_FILTER(params->vertFilter)) 
        && err == BERR_SUCCESS)
    {
        err = BGRC_Source_ToggleFilter(grc, BGRCLIB_IS_BLURRY_FILTER(params->horzFilter), BGRCLIB_IS_BLURRY_FILTER(params->vertFilter));
        filterToggled = true;
    }

    if (err == BERR_SUCCESS)
        err = BGRC_Source_SetFilterCoeffs(grc, params->horzFilter, params->vertFilter);

    /* Set-up the Constant color in the Blend block */
    /* This is the value used any "Constant Alpha" or "Constant Color" operations. */
    if (err == BERR_SUCCESS && grclibHandle->bldColor != params->constantColor)
    {
        err = BGRC_Blend_SetColor(grc, params->constantColor);
        grclibHandle->bldColor = params->constantColor;
    }

    /* Populate the 'default' colors in the Source and Destination */
    /* These values will be used if the corresponding channel is missing from from the format or if the */
    /* source or destination surface is NULL. */
    if (err == BERR_SUCCESS && grclibHandle->srcColor != params->constantColor)
    {
        err = BGRC_Source_SetColor(grc, params->constantColor);
        grclibHandle->srcColor = params->constantColor;
    }

    if (err == BERR_SUCCESS && grclibHandle->dstColor != params->destContColor)
    {
        err = BGRC_Destination_SetColor(grc, params->destContColor);
        grclibHandle->dstColor = params->destContColor;
    }


    /* If a color conversion matrix was provided, then use it */
    if (matrixParams != NULL && matrixParams->conversionMatrix != NULL)
    {
        if (err == BERR_SUCCESS)
            err = BGRC_Source_ToggleColorMatrix(grc, true);

        if (err == BERR_SUCCESS)
            err = BGRC_Source_SetColorMatrix5x4(grc, matrixParams->conversionMatrix, matrixParams->matrixShift);

        matrixToggled = true;
    }

    /* Set-up colorkey parameters, if param block passed */
    if (colorkeyParams)
    {
        /* Set-up colorkey for source, if requested */
        if (colorkeyParams->enableColorKey)
        {
            /* Note that if the pixel is colorkeyed, then, according to the blend equation above, */
            /* the color selected will be the pixel from destination surface/block. */
            if (err == BERR_SUCCESS)
                err = BGRC_Source_SetColorKey(grc,
                    colorkeyParams->colorKeyLower,
                    colorkeyParams->colorKeyUpper,
                    colorkeyParams->colorKeyMask,
                    colorkeyParams->colorKeyReplace,
                    colorkeyParams->colorKeyRplMask,
                    false);
        
            if (err == BERR_SUCCESS)
                err = BGRC_Source_ToggleColorKey(grc, true);
        
            if (err == BERR_SUCCESS)
                err = BGRC_Source_SetKeyMatrixScaleOrder(grc, BGRC_KeyMatrixScaleOrder_eKeyThenScaleThenMatrix);
        }
        
        /* Set-up colorkey for destination, if requested */
        if (colorkeyParams->enableDstColorKey)
        {
            /* Note that if the pixel is colorkeyed, then, according to the blend equation above, */
            /* the color selected will be the pixel from source surface/block. */
            /* Also note that the params->colorKeySelect should be set to BGRC_Output_ColorKeySelection_eTakeDestination */
            if (err == BERR_SUCCESS)
                err = BGRC_Destination_SetColorKey(grc,
                    colorkeyParams->dstColorKeyLower,
                    colorkeyParams->dstColorKeyUpper,
                    colorkeyParams->dstColorKeyMask,
                    colorkeyParams->dstColorKeyReplace,
                    colorkeyParams->dstColorKeyRplMask,
                    false);
        
            if (err == BERR_SUCCESS)
                err = BGRC_Destination_ToggleColorKey(grc, true);
        
        }

        colorkeyToggled = true;
    }

    /* Set-up patterns, if param block passed */
    if (patternParams != NULL)
    {
        if (err == BERR_SUCCESS)
            err = BGRC_Pattern_Set(grc, patternParams->ropVector, patternParams->pattern, 
                                   patternParams->backColor, patternParams->foreColor);

        patternToggled = true;
    }

    /* Set-up scaling control, if param block passed */
    if (scalingParams != NULL)
    {
        if (scalingParams->setFilterPhaseAdj && err == BERR_SUCCESS)
            err = BGRC_Source_SetFilterPhaseAdjustment(grc, scalingParams->iHorizontalPhase, scalingParams->iVerticalPhase, 
                                                       scalingParams->ulPhaseFixedPointShift);

        if (scalingParams->setFixedScaleFactor && err == BERR_SUCCESS)
            err = BGRC_Source_SetFixedScaleFactor(grc, scalingParams->ulHorizontalNumerator, scalingParams->ulHorizontalDenominator, 
                                                  scalingParams->ulVerticalNumerator, scalingParams->ulVerticalDenominator);

        scalingToggled = true;
    }

    /* Configure mirroring, as specified */
    if (err == BERR_SUCCESS)
        err = BGRC_Source_SetDirection(grc, params->mirrorSrcHorizontally, params->mirrorSrcVertically);

    if (err == BERR_SUCCESS)
        err = BGRC_Destination_SetDirection(grc, params->mirrorDstHorizontally, params->mirrorDstVertically);

    if (err == BERR_SUCCESS)
        err = BGRC_Output_SetDirection(grc, params->mirrorOutHorizontally, params->mirrorOutVertically);

    /* Configure Source Alpha Premultiplication */
    if (err == BERR_SUCCESS)
        err = BGRC_Source_SetAlphaPreMultiply(grc, params->srcAlphaPremult);

    /* Set-up vars for optimizing GRC calls */
    xSrc      = grclibHandle->srcX;
    ySrc      = grclibHandle->srcY;
    widthSrc  = grclibHandle->srcWidth;
    heightSrc = grclibHandle->srcHeight;

    xDst      = grclibHandle->dstX;
    yDst      = grclibHandle->dstY;
    widthDst  = grclibHandle->dstWidth;
    heightDst = grclibHandle->dstHeight;

    xOut      = grclibHandle->outX;
    yOut      = grclibHandle->outY;
    widthOut  = grclibHandle->outWidth;
    heightOut = grclibHandle->outHeight;

    /* ---------------------------------------- */
    /* Loop over the lists                      */
    /* ---------------------------------------- */
    for (index = 0; index < maxCnt && err == BERR_SUCCESS; index++)
    {
        /* --------------------------- */
        /* Source                      */
        /* --------------------------- */
        /* Get the Source Surface and Rectangle according to the index */
        count = params->srcSurfaceCnt;
        surf = NULL;
        if (count > 0)
            surf = params->srcSurfaceList[index % count];

        count = params->srcRectCnt;
        pRect = NULL;
        if (count > 0)
            pRect = &params->srcRectList[index % count];

        /* Update the Source Surface, if it changed */
        if (srcSurf != surf)
        {
            err = BGRC_Source_SetSurface(grc, surf);
            srcSurf = surf;
        }

        /* Update the Source Rectangle, if it changed */
        if (surf != NULL && err == BERR_SUCCESS)
            err = BGRClib_p_GetValidPositionInfo(pRect, surf, &x, &y, &w, &h);

        if ((x != xSrc || y != ySrc || w != widthSrc || h != heightSrc)  && err == BERR_SUCCESS)
        {
            err = BGRC_Source_SetRectangle(grc, x, y, w, h);
            xSrc = x; ySrc = y; widthSrc = w; heightSrc = h;
        }
    
        /* --------------------------- */
        /* Destination                 */
        /* --------------------------- */
        /* Get the Destination Surface and Rectangle according to the index */
        count = params->dstSurfaceCnt;
        surf = NULL;
        if (count > 0)
            surf = params->dstSurfaceList[index % count];

        count = params->dstRectCnt;
        pRect = NULL;
        if (count > 0)
            pRect = &params->dstRectList[index % count];

        /* Update the Destination Surface, if it changed */
        if (dstSurf != surf && err == BERR_SUCCESS)
        {
            err = BGRC_Destination_SetSurface(grc, surf);
            dstSurf = surf;
        }

        /* Update the Destination Rectangle, if it changed */
        if (surf != NULL && err == BERR_SUCCESS)
            err = BGRClib_p_GetValidPositionInfo(pRect, surf, &x, &y, &w, &h);

        if ((x != xDst || y != yDst || w != widthDst || h != heightDst) && err == BERR_SUCCESS)
        {
            err = BGRC_Destination_SetRectangle(grc, x, y, w, h);
            xDst = x; yDst = y; widthDst = w; heightDst = h;
        }
    
        /* --------------------------- */
        /* Output                      */
        /* --------------------------- */
        /* Get the Output Surface and Rectangle according to the index */
        count = params->outSurfaceCnt;
        surf = NULL;
        if (count > 0)
            surf = params->outSurfaceList[index % count];

        count = params->outRectCnt;
        pRect = NULL;
        if (count > 0)
            pRect = &params->outRectList[index % count];

        /* Update the Output Surface, if it changed */
        if (outSurf != surf && err == BERR_SUCCESS)
        {
            err = BGRC_Output_SetSurface(grc, surf);
            outSurf = surf;
        }

        /* Update the Output Rectangle, if it changed */
        if (surf != NULL && err == BERR_SUCCESS)
            err = BGRClib_p_GetValidPositionInfo(pRect, surf, &x, &y, &w, &h);

        if ((x != xOut || y != yOut || w != widthOut || h != heightOut) && err == BERR_SUCCESS)
        {
            err = BGRC_Output_SetRectangle(grc, x, y, w, h);
            xOut = x; yOut = y; widthOut = w; heightOut = h;
        }
    
   
        /* --------------------------- */
        /* Go!                         */
        /* --------------------------- */
        /* Start the operations, all non-block no callback, except the last, which honors the pendtype */
        if (err == BERR_SUCCESS)
        {
            if (index < maxCnt - 1)
                err = BGRClib_p_Go(grclibHandle, BGRClib_PendType_eNoBlockNoCB);
            else
                err = BGRClib_p_Go(grclibHandle, pendtype);
        }
    }

    grclibHandle->srcX      = xSrc;
    grclibHandle->srcY      = ySrc;
    grclibHandle->srcWidth  = widthSrc;
    grclibHandle->srcHeight = heightSrc;

    grclibHandle->dstX      = xDst;
    grclibHandle->dstY      = yDst;
    grclibHandle->dstWidth  = widthDst;
    grclibHandle->dstHeight = heightDst;

    grclibHandle->outX      = xOut;
    grclibHandle->outY      = yOut;
    grclibHandle->outWidth  = widthOut;
    grclibHandle->outHeight = heightOut;

    /* Undo any 'toggled' changes */
    /* Colorkey */
    if (err == BERR_SUCCESS && colorkeyToggled)
        err = BGRC_Source_ToggleColorKey(grc, false);

    if (err == BERR_SUCCESS && colorkeyToggled)
        err = BGRC_Destination_ToggleColorKey(grc, false);

    /* Color Matrix */
    if (err == BERR_SUCCESS && matrixToggled)
        err = BGRC_Source_ToggleColorMatrix(grc, false);

    /* Filter */
    if (err == BERR_SUCCESS && filterToggled)
        err = BGRC_Source_ToggleFilter(grc, false, false);

    /* Palette */
    if (err == BERR_SUCCESS && paletteToggled)
        err = BGRC_Source_TogglePaletteBypass(grc, false);

    if (err == BERR_SUCCESS && paletteToggled)
        err = BGRC_Destination_TogglePaletteBypass(grc, false);

    /* Scaling */
    if (err == BERR_SUCCESS && scalingToggled)
        err = BGRC_Source_SetFilterPhaseAdjustment(grc, 0, 0, 0);

    if (err == BERR_SUCCESS && scalingToggled)
        err = BGRC_Source_SetFixedScaleFactor(grc, 0, 0, 0, 0);

    /* Patterns */
    if (err == BERR_SUCCESS && patternToggled)
        err = BGRC_Pattern_ResetState(grc);

    /* Clear anything unconditionally set but not tracked */
    if (err == BERR_SUCCESS)
        err = BGRC_Source_SetDirection(grc, false, false);

    if (err == BERR_SUCCESS)
        err = BGRC_Destination_SetDirection(grc, false, false);

    if (err == BERR_SUCCESS)
        err = BGRC_Output_SetDirection(grc, false, false);

    if (err == BERR_SUCCESS)
        err = BGRC_Source_SetAlphaPreMultiply(grc, false);

    if (err == BERR_SUCCESS)
        err = BGRC_Source_SetFilterCoeffs(grc, BGRC_FilterCoeffs_eSharp, BGRC_FilterCoeffs_eSharp);

    if (err == BERR_SUCCESS)
        err = BGRC_Source_SetKeyMatrixScaleOrder(grc, BGRC_KeyMatrixScaleOrder_eScaleThenKeyThenMatrix);

    BDBG_LEAVE(BGRClib_Blit);
    return BERR_TRACE(err);;
}


/*****************************************************************************
  Summary:
    Start the GRC operation according to the pendtype param

  Description:
    This routine calls either BGRC_IssueState or BGRC_IssueStateAndWait 
    according to pendtype parameter.    

 *****************************************************************************/
BERR_Code BGRClib_p_Go(
    BGRClib_Handle          grclibHandle,
    BGRClib_PendType        pendtype
)
{
    BERR_Code err;

    if (pendtype == BGRClib_PendType_eBlock)
        err = BGRC_IssueStateAndWait(grclibHandle->hGRC);

    else if (pendtype == BGRClib_PendType_eNoBlockNoCB)
        err = BGRC_IssueState(grclibHandle->hGRC, NULL, NULL);

    else if (pendtype == BGRClib_PendType_eNoBlockUseCB)
        err = BGRC_IssueState(grclibHandle->hGRC, grclibHandle->callback_isr, grclibHandle->callback_data);

    else
    {
        BDBG_ASSERT(false);
        err = BERR_INVALID_PARAMETER;
    }

    return BERR_TRACE(err);
}


/*****************************************************************************
  Summary:
    Converts color according to the ColorFmtType param

  Description:

    .    
    switch (surface->create_settings.pixel_format) {
    case bgraphics_pixel_format_palette2:
        pixel = (pixel & 0x3) << 6;
        break;
    case bgraphics_pixel_format_palette4:
        pixel = (pixel & 0xF) << 4;
        break;
    case bgraphics_pixel_format_a8_palette8:
        pixel = ((pixel & 0xFF00) << 16) | (pixel & 0xFF);
        break;
    case bgraphics_pixel_format_a8:
        pixel = (pixel & 0xFF) << 24;
        break;
    case bgraphics_pixel_format_y08_cb8_y18_cr8:
        {
        unsigned int aycrcb_pixel;
        BPXL_ConvertPixel_RGBtoYCbCr(BPXL_eA8_Y8_Cb8_Cr8, BPXL_eA8_R8_G8_B8, pixel, &aycrcb_pixel);
        BDBG_MSG(("convert pixel on fill %08x => %08x", pixel, aycrcb_pixel));
        pixel = aycrcb_pixel;
        }
        break;
    default:
        break;

 *****************************************************************************/
BERR_Code BGRClib_p_ConvertColorToStdColor(
    BSUR_Surface_Handle     surface,
    uint32_t                constantColor,
    BGRClib_ColorFmtType    fmtType,
    uint32_t*               pColor
)
{
    BERR_Code   err = BERR_SUCCESS;
    BPXL_Format srcformat;
    BPXL_Format dstformat;

    if (fmtType == BGRClib_ColorFmtType_eStdFormat)
    {
        /* The color provided was in the format needed by the GRC/M2MC */
        /* so no conversion needed */
        *pColor = constantColor;
    }
    else if (fmtType == BGRClib_ColorFmtType_eSameAsSurf)
    {
        if (err == BERR_SUCCESS)
            err = BSUR_Surface_GetFormat(surface, &srcformat);

        if (err == BERR_SUCCESS)
        {
            dstformat = (BPXL_IS_RGB_FORMAT(srcformat)) ? BPXL_eA8_R8_G8_B8 : BPXL_eA8_Y8_Cb8_Cr8;
            err = BPXL_ConvertPixel(dstformat, srcformat, constantColor, (unsigned int*)pColor);
        }
    }
    else
    {
        BDBG_ASSERT(false);
        err = BERR_INVALID_PARAMETER;
    }

    return BERR_TRACE(err);
}


/*****************************************************************************
  Summary:
    Get position (x, y, width, height) from either a rect or from the surface
    if the rect is NULL

  Description:
    This routine handles the work of testing whether a rect pointer is NULL
    or not. If the rect is NULL, then set the x and y to zero and use the
    width and height from the surface.    

 *****************************************************************************/
BERR_Code BGRClib_p_GetValidPositionInfo(
    const BRect*            pRect,
    BSUR_Surface_Handle     surface,
    uint32_t*               pX,
    uint32_t*               pY,
    uint32_t*               pWidth,
    uint32_t*               pHeight)
{
    BERR_Code err = BERR_SUCCESS;

    if (pRect != NULL)
    {
        *pX = pRect->left;
        *pY = pRect->top;
        *pWidth = PBRECT_WIDTH(pRect);
        *pHeight = PBRECT_HEIGHT(pRect);
    }
    else
    {
        *pX = 0;
        *pY = 0;

        err = BSUR_Surface_GetDimensions(surface, pWidth, pHeight);
    }

    return err;
}
/* End of File */
