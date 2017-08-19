/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#include "b_api_shim.h"
#include "nexus_surface.h"
#include "bcc_winlib.h"
#include "bfont.h"

BDBG_MODULE(bdcc_winlib);

#define BDCC_WINLIB_CHARS_PER_ROW   32
#define BDCC_WINLIB_CHAR_EDGE_WIDTH 1
#define BDCC_WINLIB_ULINE_HEIGHT    1  /* the thickness of the underline */
#define DIRTY_RECTANGLE_METHOD /* recomposite the minimum screen area using this algorithm */


/* This structure represents a caption row
*/
struct BDCC_WINLIB_Row
{
    BDCC_WINLIB_Handle      hWinLibHandle;  /* handle of parent WinLib */
    NEXUS_SurfaceHandle     surface;        /* handle to Settop surface instance */
    struct bfont_surface_desc surface_desc; /* info about surface needed for bfont */
    BDCC_WINLIB_hRow        next;           /* next element in ordered linked list */
    BDCC_Justify            justification;  /* Left Right or Center justification */
    uint32_t                width;          /* width of the row surface */
    uint32_t                height;         /* height of the row surface */
    uint32_t                penPositionx;   /* coordinates for the next char render */
    uint32_t                penPositiony;
    uint32_t                fillColorARGB32;/* window fill color */
    BDCC_Opacity            fillOpacity;    /* window fill opacity */
    BDCC_WINLIB_Rect        dispRect;       /* target screen display rectangle */
    BDCC_WINLIB_Rect        clipRect;       /* source surface clip rectangle */
    BDCC_WINLIB_Rect        textRect;       /* rectangle describing the text area within the row surface */
    bool                    toBeDisplayed;  /*  the CC command stream has made this window (surface) visible */
    bool                    clippedOut;     /* the coordinates of the surface are completely outside the frame buffer */
    uint32_t                zorder;


} BDCC_WINLIB_Row;

#if (B_HAS_SOFT_GFX==1)
#include "b_softgfx_lib.h"
#define NEXUS_Graphics2DHandle B_SoftGfxHandle
#define NEXUS_Graphics2DFillSettings B_SoftGfx_FillSettings
#define NEXUS_Graphics2DOpenSettings B_SoftGfx_OpenSettings
#define NEXUS_Graphics2DBlitSettings B_SoftGfx_BlitSettings
#define NEXUS_Graphics2D_GetDefaultBlitSettings B_SoftGfx_GetDefaultBlitSettings
#define NEXUS_Graphics2D_GetDefaultFillSettings B_SoftGfx_GetDefaultFillSettings
#define NEXUS_Graphics2D_GetDefaultOpenSettings B_SoftGfx_GetDefaultOpenSettings
#define NEXUS_Graphics2D_Open(y, x) B_SoftGfx_Open(x)
#define NEXUS_Graphics2D_Close B_SoftGfx_Close
#define NEXUS_Graphics2D_Fill B_SoftGfx_Fill
#define NEXUS_Graphics2D_Blit B_SoftGfx_Blit
#define NEXUS_BlitColorOp_eMax B_SoftGfx_BlitColorOp_eMax
#define NEXUS_BlitColorOp_eCopySource B_SoftGfx_BlitColorOp_eCopySource
#define NEXUS_BlitColorOp_eUseSourceAlpha B_SoftGfx_BlitColorOp_eUseSourceAlpha
#define NEXUS_BlitAlphaOp_eMax B_SoftGfx_BlitAlphaOp_eMax
#define NEXUS_BlitAlphaOp_eCopySource B_SoftGfx_BlitAlphaOp_eCopySource
#else
#include "nexus_graphics2d.h"
#endif /* B_HAS_SOFT_GFX */

struct BDCC_WINLIB_Object
{
    BDCC_WINLIB_OpenSettings settings;
    NEXUS_Graphics2DHandle  graphics;       /* handle to Settop graphics instance */
#ifdef DIRTY_RECTANGLE_METHOD
    BDCC_WINLIB_Rect        diffRect; /* difference between framebuffer1 and framebuffer2 */
    BDCC_WINLIB_Rect        dirtyRect; /* rect representing the extent of change since last framebuffer switch */
#endif
    NEXUS_SurfaceHandle     currentFrameBuffer; /* the one being rendered */
    NEXUS_VideoFormat       videoFormat;
    uint32_t                framebufferWidth; /* horizontal size of the caption region on screen */
    uint32_t                framebufferHeight; /* vertical size of the caption region on screen */
    BDCC_WINLIB_hRow        pCurrentRow;
    BDCC_WINLIB_hRow        pTempRowSurface;
    BDCC_WINLIB_hRow        pTempFlashRowSurface;
    uint32_t                tempSurfFillExtent;
    uint32_t                tempSurfIncRenderExtent;
    BDCC_WINLIB_hRow        head; /* head of z-ordered linked list of caption rows */
    uint32_t                maxGlyphWidth[BDCC_PenSize_Max_Size];
    uint32_t                maxGlyphHeight[BDCC_PenSize_Max_Size];
    bfont_t                 fontFaces[ BDCC_FontStyle_Max_Value ]
                                        [ BDCC_PenSize_Max_Size ]
                                        [ 2 ]; /* normal / italic */
    bfont_t                 font;
    BDCC_PenSize            penSize;
    uint32_t                foreGroundColorARGB32;
    BDCC_Opacity            foreGroundOpacity;
    uint32_t                backGroundColorARGB32;
    BDCC_Opacity            backGroundOpacity;
    uint32_t                edgeColorARGB32;
    uint32_t                fillColorARGB32;
    BDCC_Edge               edgeType;
    uint32_t                edgeWidth;
    bool                    underline;
    bool                    hideDisplay;
    BKNI_EventHandle        gfxEvent;       /* grpahics checkpoint sync event*/
} BDCC_WINLIB_Object;


/* IMPORTANT - You have the following pixel format options
**
** the canvas onto which all of the caption windows are composited can be ARGB8888 or ARGB2222
** the rendered text segments that make up the caption windows can be ARGB8888 or ARGB2222
**
** Since the inherent EIA708 colorspace is ARGB2222 it makes sense for the caption windows to be ARGB2222
** although the antialiasing performance of the text rendering is poor on the smallest of font sizes
**
** If the canvas is shared with the customer UI it is unlikely that the customer would want to use an ARGB2222
** canvas.
**
** Other considerations. If the canvas is ARGB2222 then any scaled blits to that canvas must use the point sample filter.
** This is a limitation of our M2MC.  If point sample is not used then color specs can be observed on the rendered font.
**
** To use an ARGB2222 canvas define ARGB2222_CANVAS.
** To use ARGB2222 for the caption windows just define ARGB2222
*/



#ifdef ARGB2222_CANVAS
#define USE_ARGB2222 /* ARGB2222 Canvas needs ARGB2222 caption windows */
#endif


#ifdef USE_ARGB2222
static const uint8_t map_2_to_8_rgb[4]   = {0, 85, 170, 255};
static const uint8_t map_2_to_8_alpha[4] = {0, 70, 140, 255};


#define ARGB_2222_TO_ARGB_8888(x) ( \
(map_2_to_8_alpha[((x) & 0xC0) >> 6] << 24)   | \
(map_2_to_8_rgb[((x) & 0x30) >> 4] << 16)   | \
(map_2_to_8_rgb[((x) & 0xC) >> 2] << 8)     | \
  map_2_to_8_rgb[(x) & 0x3])


#define ARGB_8888_TO_ARGB_2222(x) ( \
(((x) & 0xC0000000) >> 24) | \
(((x) & 0x00C00000) >> 18) | \
(((x) & 0x0000C000) >> 12) | \
(((x) & 0x000000C0) >> 6))
#endif


#define COPY_RECT_2_RECT(srcRect,destRect)          \
    do                                              \
    {                                               \
        (destRect)->x       = (srcRect)->x;         \
        (destRect)->y       = (srcRect)->y;         \
        (destRect)->width   = (srcRect)->width;     \
        (destRect)->height  = (srcRect)->height;    \
    }                                               \
    while(0)


static BDCC_WINLIB_ErrCode fillRectangle(
    BDCC_WINLIB_hRow row,
    const BDCC_WINLIB_Rect *pRect,
    uint32_t colorARGB32
    );



#ifdef DIRTY_RECTANGLE_METHOD
static void extendDirtyRectangle(
    BDCC_WINLIB_Handle hWinLibHandle,
    const BDCC_WINLIB_Rect *rect
    );
#endif



static void CompositeRectangle(
    BDCC_WINLIB_hRow row,
    BDCC_WINLIB_Rect *fillRect,
    int penX,
    int penY,
    int flashFG,
    int flashBG,
    uint32_t rowFillColorARGB32,
    BDCC_Opacity rowFillOpacity,
    BDCC_UTF32 ch
    );



static void set_event(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void checkpoint(NEXUS_Graphics2DHandle gfx, BKNI_EventHandle event)
{
    NEXUS_Error rc;

    rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
    if (rc == NEXUS_GRAPHICS2D_QUEUED) {
        rc = BKNI_WaitForEvent(event, 5000);
    }
}

void BDCC_WINLIB_GetDefaultOpenSettings( BDCC_WINLIB_OpenSettings *pSettings )
{
    memset(pSettings, 0, sizeof(*pSettings));
}

BDCC_WINLIB_ErrCode BDCC_WINLIB_Open( BDCC_WINLIB_Handle *phWinLibHandle, const BDCC_WINLIB_OpenSettings *pSettings )
{
    BDCC_WINLIB_Handle hWinLibHandle;
    NEXUS_Graphics2DSettings Settings;
    int rc;

    BDBG_MSG(("%s", BSTD_FUNCTION));

    *phWinLibHandle = hWinLibHandle = (BDCC_WINLIB_Handle) BKNI_Malloc(sizeof(BDCC_WINLIB_Object));
    if(hWinLibHandle == NULL) {
        return BERR_TRACE(BDCC_WINLIB_ERROR_NO_MEMORY);
    }

    BKNI_Memset( hWinLibHandle,0,sizeof(*hWinLibHandle));

    rc = BKNI_CreateEvent(&hWinLibHandle->gfxEvent);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto error;
    }
    hWinLibHandle->edgeWidth = BDCC_WINLIB_CHAR_EDGE_WIDTH;
    hWinLibHandle->settings = *pSettings;

    hWinLibHandle->graphics = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    if (!hWinLibHandle->graphics) {
        rc = BERR_TRACE(-1);
        goto error;
    }
    NEXUS_Graphics2D_GetSettings(hWinLibHandle->graphics, &Settings);
    Settings.checkpointCallback.callback = set_event;
    Settings.checkpointCallback.context = hWinLibHandle->gfxEvent;
    NEXUS_Graphics2D_SetSettings(hWinLibHandle->graphics, &Settings);

    /*
    ** The following size accomodates all display formats - where the horiz
    ** size is larger (e.g.1080i) the horizontal filter is automatically invoked
    */
    hWinLibHandle->framebufferWidth = pSettings->framebufferWidth;
    hWinLibHandle->framebufferHeight = pSettings->framebufferHeight;
    hWinLibHandle->currentFrameBuffer = (hWinLibHandle->settings.flip)(hWinLibHandle->settings.context);

    return BDCC_WINLIB_SUCCESS;

error:
    if (hWinLibHandle->gfxEvent) {
        BKNI_DestroyEvent(hWinLibHandle->gfxEvent);
    }
    BKNI_Free(hWinLibHandle);
    return rc;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_Close(
    BDCC_WINLIB_Handle hWinLibHandle
    )
{
    uint32_t i = (BDCC_FontStyle_Max_Value * BDCC_PenSize_Max_Size * 2);
    bfont_t *pFontObj = (bfont_t *)&hWinLibHandle->fontFaces;

    BDBG_ASSERT( hWinLibHandle );
    BDBG_ASSERT( hWinLibHandle->pTempRowSurface );
    BDBG_ASSERT( hWinLibHandle->pTempFlashRowSurface );

    BDBG_MSG(("%s", BSTD_FUNCTION));

    if( hWinLibHandle->head )
    {
        BDBG_ERR(("ERROR! - Caption Rows still allocated"));
        return BDCC_WINLIB_FAILURE;
    }

    while(i--)
        if( pFontObj[i] )
        {
            BDBG_ERR(("ERROR!- Fonts Still Open"));
            return BDCC_WINLIB_FAILURE;
        }

    /* free the implicitly allocated surfaces */
    BDCC_WINLIB_DestroyCaptionRow( hWinLibHandle->pTempRowSurface );
    BDCC_WINLIB_DestroyCaptionRow( hWinLibHandle->pTempFlashRowSurface );
    if (hWinLibHandle->graphics) {
        NEXUS_Graphics2D_Close(hWinLibHandle->graphics);
    }
    BKNI_DestroyEvent(hWinLibHandle->gfxEvent);
    BKNI_Free( hWinLibHandle );

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_CreateCaptionRow(
    BDCC_WINLIB_Handle hWinLibHandle,
    BDCC_WINLIB_hRow *pWin)
{
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    NEXUS_SurfaceMemory surfmem;
    uint32_t width, height;
#ifdef USE_ARGB2222
    int i;
#endif
    BDCC_WINLIB_hRow *pWinTemp = pWin;


    width  = ( hWinLibHandle->maxGlyphWidth [BDCC_PenSize_Large] + hWinLibHandle->edgeWidth  ) * BDCC_WINLIB_CHARS_PER_ROW;
    height =   hWinLibHandle->maxGlyphHeight[BDCC_PenSize_Large] + hWinLibHandle->edgeWidth;

    BDBG_MSG(("%s Width %d, Height %d", BSTD_FUNCTION,width,height));

    /*
    **  This code takes a settop api graphics handle and gets
    **  a surface ready to use
    **/
    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
    surfaceCreateSettings.width  = width;
    surfaceCreateSettings.height = height;
    surfaceCreateSettings.pMemory = NULL; /* not pre-allocated */
#ifdef USE_ARGB2222
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_ePalette8;
#else
    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
#endif

    /* Let's implicitly allocate the temp row surfaces first time this is called */
    do
    {
        *pWinTemp = (BDCC_WINLIB_hRow)BKNI_Malloc(sizeof(BDCC_WINLIB_Row));
        if(*pWinTemp == NULL)
        {
            return BERR_TRACE(BDCC_WINLIB_ERROR_NO_MEMORY);
        }
        BKNI_Memset(*pWinTemp,0,sizeof( BDCC_WINLIB_Row ));

        (*pWinTemp)->hWinLibHandle = hWinLibHandle ;

        (*pWinTemp)->width = width;
        (*pWinTemp)->height = height;

        (*pWinTemp)->surface = NEXUS_Surface_Create( &surfaceCreateSettings );
        bfont_get_surface_desc((*pWinTemp)->surface, &(*pWinTemp)->surface_desc);
        BDBG_ASSERT((*pWinTemp)->surface);
        NEXUS_Surface_GetMemory( (*pWinTemp)->surface, &surfmem );

#ifdef USE_ARGB2222
        BDBG_ASSERT(surfmem.numPaletteEntries == 256);

        for( i=0 ; i< 256; i++)
        {
            surfmem.palette[i] = ARGB_2222_TO_ARGB_8888(i);
        }
#endif

        BDBG_MSG(("framebuffer %p, %dx%d, pitch %d\n", surfmem.buffer,
            surfaceCreateSettings.width, surfaceCreateSettings.height, surfmem.pitch));

        if(hWinLibHandle->pTempRowSurface)
            pWinTemp = &hWinLibHandle->pTempFlashRowSurface;
        else
            pWinTemp = &hWinLibHandle->pTempRowSurface;
    }
    while((!hWinLibHandle->pTempRowSurface) || (!hWinLibHandle->pTempFlashRowSurface));

    BDBG_MSG(("%s %p", BSTD_FUNCTION, (void*)*pWin));

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_DestroyCaptionRow(
    BDCC_WINLIB_hRow row
    )
{
    BDCC_WINLIB_hRow *phRow = &(row->hWinLibHandle->head);

    BDBG_MSG(("%s %p", BSTD_FUNCTION, (void*)row));
    BDBG_ASSERT(row);

    /* remove row from linked list */
    phRow = &(row->hWinLibHandle->head);
    while( NULL != *phRow )
    {
        if( *phRow == row )
        {
            *phRow = row->next;
            break;
        }
        phRow = &((*phRow)->next);
    }

#ifdef DIRTY_RECTANGLE_METHOD
    /* We want to make sure we clear the rect that it occupied on screen */
    extendDirtyRectangle(row->hWinLibHandle, &row->dispRect);
#endif

    NEXUS_Surface_Destroy(row->surface);
    BKNI_Free(row);

    return BDCC_WINLIB_SUCCESS;
}



#define ARGB(a,r,g,b)   ((((a)&0xFF) << 24) | (((r)&0xFF) << 16) | (((g)&0xFF) << 8) | (((b)&0xFF) << 0))
static uint32_t ConvertColor(
    BDCC_Opacity DTVCC_Opacity,
    uint8_t  DTVCC_Color
    )
{
    static uint32_t ColorLookup[] = {0,127,191,255};
    uint32_t alpha, red, grn, blu;

    switch(DTVCC_Opacity)
    {
        case BDCC_Opacity_Transparent:
            alpha = 0x0;
            break;
        case BDCC_Opacity_Translucent:
            alpha = 0x80;
            break;
        case BDCC_Opacity_Solid:
        case BDCC_Opacity_Flash:
        default:
        alpha = 0xff;
        break;
    }

    red = ColorLookup[((DTVCC_Color) >> 4) & 3];
    grn = ColorLookup[((DTVCC_Color) >> 2) & 3];
    blu = ColorLookup[((DTVCC_Color) >> 0) & 3];

    return(ARGB(alpha,red,grn,blu));
}



#ifdef DIRTY_RECTANGLE_METHOD
static void extendDirtyRectangle(
    BDCC_WINLIB_Handle hWinLibHandle,
    const BDCC_WINLIB_Rect *pRect
    )
{

    /* only extend if the new rectangle has some area */
    if( pRect->width && pRect->height )
    {
        if( hWinLibHandle->dirtyRect.width && hWinLibHandle->dirtyRect.height )
        {
            /* absorb this rect into the dirty rect */

            /* coords of bottom right of dirty rect */
            uint32_t xPrime = hWinLibHandle->dirtyRect.x + hWinLibHandle->dirtyRect.width;
            uint32_t yPrime = hWinLibHandle->dirtyRect.y + hWinLibHandle->dirtyRect.height;

            /* does the new rectangle extend the old? */
            if (( pRect->x + pRect->width ) > xPrime )
            {
                xPrime = pRect->x + pRect->width;
            }

            if (( pRect->y + pRect->height ) > yPrime )
            {
                yPrime = pRect->y + pRect->height;
            }

            if ( pRect->x < hWinLibHandle->dirtyRect.x )
                hWinLibHandle->dirtyRect.x = pRect->x;

            if ( pRect->y < hWinLibHandle->dirtyRect.y )
                hWinLibHandle->dirtyRect.y = pRect->y;

            hWinLibHandle->dirtyRect.width = xPrime - hWinLibHandle->dirtyRect.x;
            hWinLibHandle->dirtyRect.height = yPrime - hWinLibHandle->dirtyRect.y;

            BDBG_MSG(("Subseq Rect = %d %d %d %d",
                hWinLibHandle->dirtyRect.x, hWinLibHandle->dirtyRect.y, hWinLibHandle->dirtyRect.width, hWinLibHandle->dirtyRect.height));

        }
        else
        {
            /* First dirty region defines the dirty rect */
            hWinLibHandle->dirtyRect = *pRect;
            BDBG_MSG(("Initial Rect = %d %d %d %d",
                hWinLibHandle->dirtyRect.x, hWinLibHandle->dirtyRect.y, hWinLibHandle->dirtyRect.width, hWinLibHandle->dirtyRect.height));
        }
    }
}
#endif



BDCC_WINLIB_ErrCode BDCC_WINLIB_ClearCaptionRow(
    BDCC_WINLIB_hRow row,
    BDCC_Opacity opacity,
    uint8_t color
    )
{
    BDCC_WINLIB_Rect rc;

    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);

    row->fillColorARGB32 = ConvertColor( opacity, color );
    row->fillOpacity = opacity;

    /* fill the caption row with the associated windows 'window fill color' */
    rc.x = 0;
    rc.y = 0;
    rc.height = row->height;
    rc.width = row->width;

    fillRectangle(row, &rc, row->fillColorARGB32);

    /* the text has been cleared so reset the text bounding rectangle */
    row->textRect.height = row->hWinLibHandle->maxGlyphHeight[BDCC_PenSize_Small];
    row->textRect.width = 0;
    row->textRect.x= 0;
    row->textRect.y= row->hWinLibHandle->maxGlyphHeight[BDCC_PenSize_Large] - row->hWinLibHandle->maxGlyphHeight[BDCC_PenSize_Small];

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_GetFrameBufferSize(
    BDCC_WINLIB_Handle hWinLibHandle,
    uint32_t *width,
    uint32_t *height
    )
{
    NEXUS_VideoFormat format;
    (hWinLibHandle->settings.get_framebuffer_dimensions)(hWinLibHandle->settings.context, &format, width, height);
#ifdef DIRTY_RECTANGLE_METHOD
    if( format != hWinLibHandle->videoFormat )
    {
        hWinLibHandle->videoFormat = format;
        hWinLibHandle->diffRect.x = hWinLibHandle->dirtyRect.x = 0;
        hWinLibHandle->diffRect.y = hWinLibHandle->dirtyRect.y = 0;
        hWinLibHandle->diffRect.width = hWinLibHandle->dirtyRect.width = *width;
        hWinLibHandle->diffRect.height = hWinLibHandle->dirtyRect.height = *height;
    }
#endif
    return BDCC_WINLIB_SUCCESS;
}

/* If user uses smaller CC surface (height or width is samller than output frame buffer's),
   they need to set whenver output resolution is changed*/
BDCC_WINLIB_ErrCode BDCC_WINLIB_SetFrameBufferSize(
    BDCC_WINLIB_Handle hWinLibHandle,
    uint32_t width,
    uint32_t height
    )
{
    hWinLibHandle->framebufferWidth = width;
    hWinLibHandle->framebufferHeight = height;

    return BDCC_WINLIB_SUCCESS;
}
BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCaptionRowZorder(
    BDCC_WINLIB_hRow row,
    uint32_t zorder
    )
{
    BDCC_WINLIB_hRow *phRow = &(row->hWinLibHandle->head);
    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);

    row->zorder = zorder;

    /* re-order the linked list of rows based on z-order */

    /* first remove our entry if it exists */
    phRow = &(row->hWinLibHandle->head);
    while( NULL != *phRow )
    {
       if( *phRow == row )
            {
            *phRow = row->next;
            break;
        }
        phRow = &((*phRow)->next);
    }

    /* ... then reinsert based upon z-order */
    phRow = &(row->hWinLibHandle->head);
    while( NULL != *phRow )
    {
        if((*phRow)->zorder > zorder)
        {
            break;
        }
        phRow = &((*phRow)->next);
    }

    row->next = *phRow;
    *phRow = row;

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCaptionRowVisibility(
    BDCC_WINLIB_hRow row,
    bool visible
    )
{
    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);

#ifdef DIRTY_RECTANGLE_METHOD
    if( row->toBeDisplayed != visible )
        extendDirtyRectangle(row->hWinLibHandle, &row->dispRect);
#endif

    row->toBeDisplayed = visible;

    return BDCC_WINLIB_SUCCESS;
}



int BDCC_WINLIB_IsCaptionRowVisible(
    BDCC_WINLIB_hRow row
    )
{
    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);

    return( row->toBeDisplayed ? (( row->clippedOut ) ? false : true ) : false);
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_SetClipState(
    BDCC_WINLIB_hRow row,
    bool ClipState
    )
{
    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);

    row->clippedOut = ClipState;

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_GetPenPosition(
    BDCC_WINLIB_hRow row,
    uint32_t *pX,
    uint32_t *pY
    )
{
    BDBG_MSG(("%s x=%d,y=%d", BSTD_FUNCTION,row->penPositionx,row->penPositiony));
    BDBG_ASSERT(row);
    BDBG_ASSERT(pX);
    BDBG_ASSERT(pY);

    *pX= row->penPositionx;
    *pY = row->penPositiony;

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_SetPenPosition(
    BDCC_WINLIB_hRow row,
    uint32_t x,
    uint32_t y
    )
{
    BDBG_MSG(("%s x=%d,y=%d", BSTD_FUNCTION,x,y));
    BDBG_ASSERT(row);

    row->penPositionx = x;
    row->penPositiony = y;

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCaptionRowDispRect(
    BDCC_WINLIB_hRow row,
    const BDCC_WINLIB_Rect *pRect
    )
{
    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);
    BDBG_ASSERT(pRect);

#ifdef DIRTY_RECTANGLE_METHOD
    /* if the position has changed or it's the current row, extend the dirty rectangle */
    if( row->toBeDisplayed ) /* only if visible */
    {
    if( (row->dispRect.x != pRect->x) ||
        (row->dispRect.y != pRect->y) ||
        (row->dispRect.width != pRect->width) ||
        (row->dispRect.height != pRect->height) ||
        (row->hWinLibHandle->pCurrentRow == row))
            extendDirtyRectangle(row->hWinLibHandle, pRect);

    extendDirtyRectangle(row->hWinLibHandle, &row->dispRect);
    }
#endif

    row->dispRect = *pRect;

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_GetCaptionRowTextRect(
    BDCC_WINLIB_hRow row,
    BDCC_WINLIB_Rect *pRect
    )
{
    uint32_t minCaptionRowHeight = row->hWinLibHandle->maxGlyphHeight[BDCC_PenSize_Small];

    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);
    BDBG_ASSERT(pRect);

    *pRect = row->textRect;

    /* even if the caption row contains no text we must still display it
    ** therefore give it the minimum height
    **/
    if(pRect->height < minCaptionRowHeight)
        pRect->height = minCaptionRowHeight;

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCaptionRowClipRect(
    BDCC_WINLIB_hRow row,
    const BDCC_WINLIB_Rect *pRect
    )
{

    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);
    BDBG_ASSERT(pRect);

    row->clipRect = *pRect;

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_GetMaxBoundingRect(
    BDCC_WINLIB_hRow row,
    uint32_t numColumns,
    BDCC_PenSize penSize,
    BDCC_WINLIB_Rect *largestBoundingRect
    )
{
    #define BDCC_WINLIB_ROW_PADDING 6

    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);
    BDBG_ASSERT(largestBoundingRect);

    largestBoundingRect->x = largestBoundingRect->y = 0;
    largestBoundingRect->width = (numColumns * row->hWinLibHandle->maxGlyphWidth[penSize]) + BDCC_WINLIB_ROW_PADDING;
    largestBoundingRect->height = row->hWinLibHandle->maxGlyphHeight[penSize];

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_GetSurfaceRect(
    BDCC_WINLIB_hRow row,
    uint32_t *width,
    uint32_t *height
    )
{
    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);
    BDBG_ASSERT(width);
    BDBG_ASSERT(height);

    *width = row->width;
    *height = row->height;

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_GetDisplayRect(
    BDCC_WINLIB_hRow row,
    BDCC_WINLIB_Rect *pRect
    )
{
    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);
    BDBG_ASSERT(pRect);

    *pRect = row->dispRect;

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCharFGColor(
    BDCC_WINLIB_hRow row,
    uint8_t foreGroundColor,
    BDCC_Opacity Opacity
    )
{
    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);
    BDBG_ASSERT(Opacity <= BDCC_Opacity_Transparent);

    row->hWinLibHandle->foreGroundOpacity = Opacity;
    row->hWinLibHandle->foreGroundColorARGB32 = ConvertColor(Opacity, foreGroundColor);

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCharBGColor(
    BDCC_WINLIB_hRow row,
    uint8_t backGroundColor,
    BDCC_Opacity Opacity
    )
{
    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);
    BDBG_ASSERT(Opacity <= BDCC_Opacity_Transparent);

    row->hWinLibHandle->backGroundOpacity = Opacity;
    row->hWinLibHandle->backGroundColorARGB32 = ConvertColor(Opacity, backGroundColor);

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCharEdgeColor(
    BDCC_WINLIB_hRow row,
    uint8_t edgeColor,
    BDCC_Opacity Opacity
    )
{
    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);
    BDBG_ASSERT(Opacity <= BDCC_Opacity_Transparent);

    row->hWinLibHandle->edgeColorARGB32 = ConvertColor(Opacity, edgeColor);

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCharEdgeType(
    BDCC_WINLIB_hRow row,
    BDCC_Edge edgeType
    )
{
    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);
    BDBG_ASSERT(edgeType < BDCC_Edge_Style_Max_Value);

    row->hWinLibHandle->edgeType = edgeType;

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_SetEdgeWidth(
    BDCC_WINLIB_hRow row,
    uint32_t edgeWidth
    )
{
    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);

    row->hWinLibHandle->edgeWidth = edgeWidth;

    return BDCC_WINLIB_SUCCESS;
}


static BDCC_WINLIB_ErrCode fillRectangle(
    BDCC_WINLIB_hRow row,
    const BDCC_WINLIB_Rect *pRect,
    uint32_t colorARGB32
    )
{
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_Error error;

    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(row);
    BDBG_ASSERT(pRect);

    NEXUS_Graphics2D_GetDefaultFillSettings( &fillSettings );
    fillSettings.alphaOp = NEXUS_FillOp_eCopy;
    fillSettings.colorOp = NEXUS_FillOp_eCopy;

#ifdef USE_ARGB2222
    fillSettings.color = ARGB_8888_TO_ARGB_2222( colorARGB32 );
#else
    fillSettings.color = colorARGB32;
#endif

    COPY_RECT_2_RECT( pRect, &fillSettings.rect );
    fillSettings.surface = row->surface;

    /* flush cached memory */
    NEXUS_Surface_Flush(row->surface);

    error = NEXUS_Graphics2D_Fill( row->hWinLibHandle->graphics, &fillSettings );
    if(error)
    {
        BDBG_ERR(("NEXUS_Graphics2D_Fill() returned %d", error));
        return BDCC_WINLIB_FAILURE;
    }
    /*sync gfx operation*/
    checkpoint(row->hWinLibHandle->graphics, row->hWinLibHandle->gfxEvent);

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_LoadFont(
    BDCC_WINLIB_Handle hWinLibHandle,
    const char *pFontFile,
    uint32_t fontSize,
    BDCC_FontStyle fontStyle,
    BDCC_PenSize penSize,
    BDCC_PenStyle penStyle
    )
{
    BDCC_WINLIB_ErrCode error = BDCC_WINLIB_SUCCESS ;
    bfont_t font;
    int32_t numToMeasure, advance, width, height ;
    char charsToMeasure [] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;


    BDBG_MSG(("%s", BSTD_FUNCTION));

    BSTD_UNUSED(fontSize);
    BDBG_ASSERT( hWinLibHandle );
    BDBG_ASSERT( pFontFile );
    BDBG_ASSERT( BDCC_FontStyle_Max_Value > fontStyle );
    BDBG_ASSERT( BDCC_PenSize_Max_Size > penSize );
    BDBG_ASSERT( BDCC_PenStyle_Underline > penStyle ); /* no underline font supported, draw the underline instead */

#ifdef FREETYPE_SUPPORT
    {
        struct bfont_open_freetype_settings settings;
        bfont_get_default_open_freetype_settings(&settings);
        settings.filename = pFontFile;
        settings.size = fontSize;
        settings.italics = (penStyle == BDCC_PenStyle_Italics) ? 1 : 0;
        font = bfont_open_freetype(&settings);
    }
#else
     font = bfont_open(pFontFile);
#endif
    if (!font)
        return BDCC_WINLIB_FAILURE;

    hWinLibHandle->fontFaces[ fontStyle ][ penSize ][ penStyle ] = font;

    BDBG_MSG(("BDCC_WINLIB_LoadFont %p %s", (void*)font, (penStyle == BDCC_PenStyle_Standard) ? "standard" : "italic"));


    /* To calculate window sizes, we need to know the height of the tallest glyph and the width of the
    ** widest glyph.  Walk through the list of characters to find the max width and height.
    */
    numToMeasure = sizeof( charsToMeasure ) - 1; /* skip the NULL termination */

    while( numToMeasure-- )
    {
        if( bfont_get_glyph_metrics(font, charsToMeasure[ numToMeasure ], &width, &height, &advance) )
        {
            BDBG_MSG(("Failed function bfont_get_glyph_metrics()"));
            error = BDCC_WINLIB_FAILURE;
        }

        if ( (uint32_t)advance > hWinLibHandle->maxGlyphWidth[penSize] )
        {
            hWinLibHandle->maxGlyphWidth[penSize] = advance;
        }

        if ( (uint32_t)height > hWinLibHandle->maxGlyphHeight[penSize] )
        {
            hWinLibHandle->maxGlyphHeight[penSize] = height;
        }
    }

    return error;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_UnloadFonts(
    BDCC_WINLIB_Handle hWinLibHandle
    )
{
    int i, j;
    bfont_t *pFontObj;
    bfont_t font;

    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(hWinLibHandle);

    pFontObj = (bfont_t *)&hWinLibHandle->fontFaces;

    i = (BDCC_FontStyle_Max_Value * BDCC_PenSize_Max_Size * 2);

    /* beware, we could use the same base font for different 708 fonts.
    ** Typically we make the 'default' font a duplicate of one of the others
    ** so we have to check for multiple references
    **/
    while( i-- )
    {
        font = pFontObj[ i ];

        if(font)
        {
            BDBG_MSG(("BDCC_WINLIB_UnloadFont %p", (void*)font));
            bfont_close(font);

            /* look for other references to this font */
            j = i;
            while(j--)
            {
                if(pFontObj[ j ] == font)
                {
                    pFontObj[ j ] = NULL;
                }
            }
            pFontObj[ i ] = NULL;
        }
    }

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_SetFont(
    BDCC_WINLIB_hRow row,
    BDCC_FontStyle fontStyle,
    BDCC_PenSize penSize,
    BDCC_PenStyle penStyle
    )
{
    bfont_t pSelectedFont;
    BDCC_WINLIB_Handle hWinLibHandle = row->hWinLibHandle;

    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT( row ) ;
    BDBG_ASSERT( BDCC_FontStyle_Max_Value > fontStyle );
    BDBG_ASSERT( BDCC_PenSize_Max_Size > penSize );
    BDBG_ASSERT( BDCC_PenStyle_Max_Pen_Style > penStyle );

    switch(penStyle)
    {
        case BDCC_PenStyle_Italics:
            pSelectedFont = hWinLibHandle->fontFaces[ fontStyle ][ penSize ][ BDCC_PenStyle_Italics ];
            hWinLibHandle->underline = false;
            break;
        case BDCC_PenStyle_Underline:
            pSelectedFont = hWinLibHandle->fontFaces[ fontStyle ][ penSize ][ BDCC_PenStyle_Standard ];
            hWinLibHandle->underline = true;
            break;
        case BDCC_PenStyle_Italics_Underline:
            pSelectedFont = hWinLibHandle->fontFaces[ fontStyle ][ penSize ][ BDCC_PenStyle_Italics ];
            hWinLibHandle->underline = true;
            break;
        case BDCC_PenStyle_Standard:
        default:
            pSelectedFont = hWinLibHandle->fontFaces[ fontStyle ][ penSize ][ BDCC_PenStyle_Standard ];
            hWinLibHandle->underline = false;
            break;
    }

    if ( !pSelectedFont )
    {
        BDBG_ERR(( "BDCC_WINLIB_SetFont: Font not loaded!!!!"));
        BDBG_ASSERT(false);
    }

    BDBG_MSG(("BDCC_WINLIB_SetFont %p %s", (void*)pSelectedFont,
                    ((penStyle == BDCC_PenStyle_Standard) ||
                    (penStyle == BDCC_PenStyle_Underline)) ? "standard" : "italic"));

    hWinLibHandle->font = pSelectedFont;
    hWinLibHandle->penSize = penSize;

    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode
    BDCC_WINLIB_DrawString(
    BDCC_WINLIB_hRow row,
    const BDCC_UTF32 *str,
    uint32_t len
    )
{
    int rc = 0, rc2 = 0;
    bfont_draw_text_settings draw_text_settings;
    NEXUS_Rect rect = {0,0,0,0};

    static const int edgeOffsets[6][4] =
        { /* Fgnd X  Fgnd Y  Edge X   Edge Y */
            {   0,      0,      0,      0   },  /* BDCC_Edge_Style_None */
            {   1,      0,      0,      -1  },  /* BDCC_Edge_Style_Raised */
            {   0,      0,      1,      1   },  /* BDCC_Edge_Style_Depressed */
            {   0,      0,      0,      0   },  /* BDCC_Edge_Style_Uniform */
            {   2,      0,      0,      2   },  /* BDCC_Edge_Style_LeftDropShadow */
            {   0,      0,      2,      2   }   /* BDCC_Edge_Style_RightDropShadow */
        };


    BDBG_ASSERT(row);
    BDBG_ASSERT(str);

    BDBG_MSG(("%s String = %s>", BSTD_FUNCTION,(char*)str));
    BDBG_MSG(("%s Window Posx = %d, Posy=%d",BSTD_FUNCTION, row->penPositionx, row->penPositiony));

    /*
    ** Edges are implemented by rendering the character twice.
    ** First with the edge color and offset by the width of the edge.
    ** Then a second time with the foreground color.
    ** This doesn't look very good, I think the real solution is to use a fonts designed
    ** for TV's which include edge support.
    ** This implementation is useful to verify that all the infrastructure is in place for controlling font edges.
    */

    bfont_get_default_draw_text_settings(&draw_text_settings);
    draw_text_settings.text_type = bfont_text_type_utf32;
    switch(row->hWinLibHandle->edgeType)
    {
        case BDCC_Edge_Style_Depressed:
        case BDCC_Edge_Style_LeftDropShadow:
        case BDCC_Edge_Style_RightDropShadow:
        case BDCC_Edge_Style_Raised:
            rect.x = row->penPositionx + (edgeOffsets[row->hWinLibHandle->edgeType][2] * row->hWinLibHandle->edgeWidth);
            rect.y = row->penPositiony + (edgeOffsets[row->hWinLibHandle->edgeType][3] * row->hWinLibHandle->edgeWidth);
            rc =  bfont_draw_text_ex(
                    &row->surface_desc,
                    row->hWinLibHandle->font,
                    &rect,
                    (const char*)str,
                    len,
                    row->hWinLibHandle->edgeColorARGB32,
                    &draw_text_settings);
            break;

        case BDCC_Edge_Style_Uniform:
        case BDCC_Edge_Style_None:
        default:
            break;
    }

    rect.x = row->penPositionx + (edgeOffsets[row->hWinLibHandle->edgeType][0] * row->hWinLibHandle->edgeWidth);
    rect.y = row->penPositiony + (edgeOffsets[row->hWinLibHandle->edgeType][1] * row->hWinLibHandle->edgeWidth);
    rc2 =  bfont_draw_text_ex(
            &row->surface_desc,
            row->hWinLibHandle->font,
            &rect,
            (const char *)str,
            len,
            row->hWinLibHandle->foreGroundColorARGB32,
            &draw_text_settings);

    if(rc || rc2)
        BDBG_MSG(("bfont_draw_text failed ! rc = %d\n", rc));

    return BDCC_WINLIB_SUCCESS;

}



BDCC_WINLIB_ErrCode BDCC_WINLIB_DrawHLine(
    BDCC_WINLIB_hRow row,
    uint32_t x,
    uint32_t y,
    uint32_t wide,
    uint32_t length,
    uint32_t FgColorARGB32
    )
{
    unsigned i;
    uint32_t *rowptr = (uint32_t *)((unsigned long)row->surface_desc.buffer + row->surface_desc.pitch * y);
    BDBG_ASSERT(row);
    BSTD_UNUSED(wide);
    for (i=0;i<length && i+x<row->surface_desc.width;i++) {
        rowptr[x+i] = FgColorARGB32;
    }
    return BDCC_WINLIB_SUCCESS;
}



BDCC_WINLIB_ErrCode BDCC_WINLIB_Copy(
    BDCC_WINLIB_hRow src,
    BDCC_WINLIB_hRow dst,
    const BDCC_WINLIB_Rect *pSrcRect,
    const BDCC_WINLIB_Rect *pDstRect
    )
{
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_Error error;

    BDBG_ASSERT( src );
    BDBG_ASSERT( dst );
    BDBG_ASSERT( pSrcRect );
    BDBG_ASSERT( pDstRect );



    /* flush cached memory */
    NEXUS_Surface_Flush(src->surface);

    NEXUS_Graphics2D_GetDefaultBlitSettings( &blitSettings );

    blitSettings.source.surface = src->surface;
    COPY_RECT_2_RECT( pSrcRect, &blitSettings.source.rect );

    blitSettings.output.surface = dst->surface;
    COPY_RECT_2_RECT( pDstRect, &blitSettings.output.rect );

    blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

    error = NEXUS_Graphics2D_Blit( src->hWinLibHandle->graphics, &blitSettings );
    if(error)
    {
        BDBG_ERR(("NEXUS_Graphics2D_Blit() returned %d", error));
        return BDCC_WINLIB_FAILURE;
    }
    /*sync gfx operation*/
    checkpoint(src->hWinLibHandle->graphics, src->hWinLibHandle->gfxEvent);

    return BDCC_WINLIB_SUCCESS;
}



#ifdef DIRTY_RECTANGLE_METHOD
void RectIntersection(
    BDCC_WINLIB_Rect rect1,
    BDCC_WINLIB_Rect rect2,
    BDCC_WINLIB_Rect *pRectIntersect
    )
{
    uint32_t rect1_x_prime = (rect1.x + rect1.width);
    uint32_t rect1_y_prime = (rect1.y + rect1.height);
    uint32_t rect2_x_prime = (rect2.x + rect2.width);
    uint32_t rect2_y_prime = (rect2.y + rect2.height);
    uint32_t rectIntersect_x_prime, rectIntersect_y_prime;

    pRectIntersect->x = 0;
    pRectIntersect->y = 0;
    pRectIntersect->width = 0;
    pRectIntersect->height = 0;

    /* check for intersection */
    if ((rect1.x >= rect2_x_prime) || (rect2.x >= rect1_x_prime) ||
        (rect1.y >= rect2_y_prime) || (rect2.y >= rect1_y_prime))
        return;

    rectIntersect_x_prime = (rect1_x_prime < rect2_x_prime) ? rect1_x_prime : rect2_x_prime;
    rectIntersect_y_prime = (rect1_y_prime < rect2_y_prime) ? rect1_y_prime : rect2_y_prime;

    pRectIntersect->x = (rect1.x < rect2.x) ? rect2.x : rect1.x;
    pRectIntersect->y = (rect1.y < rect2.y) ? rect2.y : rect1.y;
    pRectIntersect->width = rectIntersect_x_prime - pRectIntersect->x;
    pRectIntersect->height = rectIntersect_y_prime - pRectIntersect->y;
}
#endif

BDCC_WINLIB_ErrCode BDCC_WINLIB_HideDisp(
    BDCC_WINLIB_Handle hWinLibHandle,
    bool hide
    )
{
#ifdef DIRTY_RECTANGLE_METHOD
    uint32_t width, height;

    BDCC_WINLIB_GetFrameBufferSize(hWinLibHandle, &width, &height);

    hWinLibHandle->dirtyRect.x = 0;
    hWinLibHandle->dirtyRect.y = 0;
    hWinLibHandle->dirtyRect.width = width;
    hWinLibHandle->dirtyRect.height = height;
    hWinLibHandle->diffRect.x = 0;
    hWinLibHandle->diffRect.y = 0;
    hWinLibHandle->diffRect.width = width;
    hWinLibHandle->diffRect.height = height;
#endif

    hWinLibHandle->hideDisplay = hide;

    return BDCC_WINLIB_SUCCESS;
}


BDCC_WINLIB_ErrCode BDCC_WINLIB_UpdateScreen(
    BDCC_WINLIB_Handle hWinLibHandle
    )
{
    BDCC_WINLIB_hRow hRow;
    NEXUS_SurfaceHandle nextFrameBuffer;
    NEXUS_Graphics2DFillSettings fillSettings;
        NEXUS_Error error;
    uint32_t framebufferDisplayWidth, framebufferDisplayHeight;
#ifdef DIRTY_RECTANGLE_METHOD
    BDCC_WINLIB_Rect dirtyRect, intersectRect;
#endif

    BDBG_MSG(("%s", BSTD_FUNCTION));
    BDBG_ASSERT(hWinLibHandle);

    if((!hWinLibHandle->hideDisplay)
#ifdef DIRTY_RECTANGLE_METHOD
        /* bail straight out (for performance) if nothing has changed */
        && (hWinLibHandle->dirtyRect.width || hWinLibHandle->dirtyRect.height)
#endif
      )
    {

    bool rectUpdated;
    BDCC_WINLIB_GetFrameBufferSize( hWinLibHandle, &framebufferDisplayWidth, &framebufferDisplayHeight );


    nextFrameBuffer = hWinLibHandle->currentFrameBuffer;
    if (!nextFrameBuffer) {
        return -1;
    }

#ifdef DIRTY_RECTANGLE_METHOD

    /* NASTY: it's possible that a caption row will straddle the dirty rectangle. This happens in some SCTE test streams
    ** This is not good because it means that we will just have to update part of a caption row
    ** The division used in the calculation will result in rounding errors if the display width != framebuffer width.
    ** The rounding error will be evident in the display Instead, if a caption row straddles
    ** the dirty rectangle, lets absorb the entire caption row into the dirty rectangle */

    do
    {
        hRow = hWinLibHandle->head;
        rectUpdated = false;
        while( NULL != hRow )
        {
            if(BDCC_WINLIB_IsCaptionRowVisible( hRow ))
            {
                RectIntersection(hWinLibHandle->dirtyRect, hRow->dispRect, &intersectRect);
                if ( intersectRect.width && intersectRect.height ) /* caption row intersects dirty rect */
                {
                    if((intersectRect.width != hRow->dispRect.width) || (intersectRect.height != hRow->dispRect.height))
                    {
                        /* if only a partial intersection then absorb the entire caption row */
                        extendDirtyRectangle(hWinLibHandle, &hRow->dispRect);
                        rectUpdated = true;
                        BDBG_MSG(("Dirty rect straddles caption row, extending dirty rect to encompass caption row"));
                    }
                }
            }
            hRow = hRow->next;
        }
    }
    while(rectUpdated); /* if we updated the dirty rect then we must go round again */

    /* temp copy */
    dirtyRect = hWinLibHandle->dirtyRect;

    /* diff rect may extend dirty rect */
    extendDirtyRectangle(hWinLibHandle, &hWinLibHandle->diffRect);

    /* calc new diff rect for next framebuffer */
    hWinLibHandle->diffRect = dirtyRect;

    /*
    ** Cleanout the next frame buffer
    */

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = nextFrameBuffer;
    /* sometimes we use the horizontal scaler in the graphics feeder */
    fillSettings.rect.x = (hWinLibHandle->dirtyRect.x * hWinLibHandle->framebufferWidth) / framebufferDisplayWidth;
    fillSettings.rect.width = (hWinLibHandle->dirtyRect.width* hWinLibHandle->framebufferWidth) / framebufferDisplayWidth;
    if (hWinLibHandle->framebufferHeight >= framebufferDisplayHeight)
    {
        fillSettings.rect.y = hWinLibHandle->dirtyRect.y;
        fillSettings.rect.height = hWinLibHandle->dirtyRect.height;
    }
    else
    {
        fillSettings.rect.y = (hWinLibHandle->dirtyRect.y * hWinLibHandle->framebufferHeight) / framebufferDisplayHeight;
        fillSettings.rect.height = ( hWinLibHandle->dirtyRect.height * hWinLibHandle->framebufferHeight) / framebufferDisplayHeight;
    }
    fillSettings.color = 0x00000000;
#else

    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = nextFrameBuffer;
    fillSettings.rect.width = hWinLibHandle->framebufferWidth;
    fillSettings.rect.height = hWinLibHandle->framebufferHeight;
    fillSettings.color = 0x00000000;
#endif

#ifdef DIRTY_RECTANGLE_METHOD
    if( fillSettings.rect.width && fillSettings.rect.height )
    {
#endif
    BDBG_MSG(("Clear %d %d %d %d", fillSettings.rect.x, fillSettings.rect.y, fillSettings.rect.width, fillSettings.rect.height));

    error = NEXUS_Graphics2D_Fill(hWinLibHandle->graphics, &fillSettings);
    if(error)
    {
        BDBG_ERR(("NEXUS_Graphics2D_Fill() returned %d", error));
        return BDCC_WINLIB_FAILURE;
    }

    /*
    ** Composite all of the visible rows on to the framebuffer
    */
    hRow = hWinLibHandle->head;

    while( NULL != hRow )
    {
       NEXUS_Graphics2DBlitSettings blitSettings;

        if(BDCC_WINLIB_IsCaptionRowVisible( hRow ))
        {
            /* visible and not clipped out so let's composite it */
            BDBG_MSG(("Compositing Row %p zorder %d", (void*)hRow, hRow->zorder));

            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);

            /* flush cached memory */
            NEXUS_Surface_Flush(hRow->surface);

            BDBG_MSG(("Source x=%d y=%d w=%d h=%d",
                hRow->clipRect.x, hRow->clipRect.y,
                hRow->clipRect.width, hRow->clipRect.height));
            BDBG_MSG(("Dest   x=%d y=%d w=%d h=%d",
                hRow->dispRect.x, hRow->dispRect.y,
                hRow->dispRect.width, hRow->dispRect.height));

#ifdef DIRTY_RECTANGLE_METHOD
            RectIntersection(hWinLibHandle->dirtyRect, hRow->dispRect, &intersectRect);

            BDBG_MSG(("Dirty = %d %d %d %d, Disp = %d %d %d %d, Intersect = %d %d %d %d",
                hWinLibHandle->dirtyRect.x, hWinLibHandle->dirtyRect.y, hWinLibHandle->dirtyRect.width, hWinLibHandle->dirtyRect.height,
                hRow->dispRect.x, hRow->dispRect.y, hRow->dispRect.width, hRow->dispRect.height,
                intersectRect.x, intersectRect.y, intersectRect.width, intersectRect.height));

            /* only render if the caption row is in the dirty rectangle */
            if ( intersectRect.width && intersectRect.height )
            {
                /* previously in this function we have insured that the caption row disp rectangle will be entirely inside the dirty rect */

#endif

                blitSettings.source.surface = hRow->surface;
                blitSettings.source.rect.x = hRow->clipRect.x;
                blitSettings.source.rect.y = hRow->clipRect.y;
                blitSettings.source.rect.width = hRow->clipRect.width;
                blitSettings.source.rect.height = hRow->clipRect.height;


                blitSettings.output.surface = nextFrameBuffer;
                blitSettings.output.rect.x = (hRow->dispRect.x * hWinLibHandle->framebufferWidth) / framebufferDisplayWidth;
                blitSettings.output.rect.width = (hRow->dispRect.width * hWinLibHandle->framebufferWidth) / framebufferDisplayWidth;
                if (hWinLibHandle->framebufferHeight >= framebufferDisplayHeight)
                {
                    blitSettings.output.rect.y = hRow->dispRect.y;
                    blitSettings.output.rect.height = hRow->dispRect.height;
                }
                else
                {
                    blitSettings.output.rect.y = (hRow->dispRect.y * hWinLibHandle->framebufferHeight) / framebufferDisplayHeight;;
                    blitSettings.output.rect.height = (hRow->dispRect.height * hWinLibHandle->framebufferHeight) / framebufferDisplayHeight;
                }
                blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
                blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

                BDBG_MSG(("Source %d %d %d %d, Dest %d %d %d %d",
                    blitSettings.source.rect.x, blitSettings.source.rect.y, blitSettings.source.rect.width, blitSettings.source.rect.height,
                    blitSettings.output.rect.x, blitSettings.output.rect.y, blitSettings.output.rect.width, blitSettings.output.rect.height));

                NEXUS_Graphics2D_Blit(hWinLibHandle->graphics, &blitSettings);

#ifdef DIRTY_RECTANGLE_METHOD
            }
#endif
        }

        hRow = hRow->next;
    }

#ifdef DIRTY_RECTANGLE_METHOD
    }
#endif

    /*sync gfx operation*/
    checkpoint(hWinLibHandle->graphics, hWinLibHandle->gfxEvent);
    hWinLibHandle->currentFrameBuffer = (hWinLibHandle->settings.flip)(hWinLibHandle->settings.context);

#ifdef DIRTY_RECTANGLE_METHOD
    hWinLibHandle->dirtyRect.x = 0;
    hWinLibHandle->dirtyRect.y = 0;
    hWinLibHandle->dirtyRect.width = 0;
    hWinLibHandle->dirtyRect.height = 0;
#endif
    }
    return BDCC_WINLIB_SUCCESS;
}



#if FLASH_BY_2SURFACES
void BDCC_WINLIB_RenderStart(
        BDCC_WINLIB_hRow row,
        BDCC_WINLIB_hRow flashrow,
        BDCC_Justify justification
        )
{
    BSTD_UNUSED(flashrow);
#else
void BDCC_WINLIB_RenderStart(
        BDCC_WINLIB_hRow row,
        BDCC_Justify justification
        )
{
#endif
    /* Caption row is clear and we will start to render the first character */
    row->hWinLibHandle->pCurrentRow = row;
    row->hWinLibHandle->tempSurfFillExtent = 0;
    row->hWinLibHandle->tempSurfIncRenderExtent = 0;
    row->penPositionx= 0;
    row->penPositiony= 0;

    row->justification = justification;

} /* BDCC_WINLIB_RenderStart */



static uint32_t ColorAvg(
    uint32_t fgColorARGB32,
    uint32_t bgColorARGB32,
    uint8_t destAlpha
    )
{
    uint32_t fgRedARGB32 = fgColorARGB32 & 0x00ff0000;
    uint32_t fgBluARGB32 = fgColorARGB32 & 0x0000ff00;
    uint32_t fgGrnARGB32 = fgColorARGB32 & 0x000000ff;

    fgRedARGB32 += (bgColorARGB32 & 0x00ff0000) >> 1;
    fgBluARGB32 += (bgColorARGB32 & 0x0000ff00) >> 1;
    fgGrnARGB32 += (bgColorARGB32 & 0x000000ff) >> 1;

    return ((destAlpha << 24) |
            (fgRedARGB32 & 0x00ff0000) |
            (fgBluARGB32 & 0x0000ff00) |
            (fgGrnARGB32 & 0x000000ff));
}



static void CompositeRectangle(
    BDCC_WINLIB_hRow row,
    BDCC_WINLIB_Rect *fillRect,
    int penX,
    int penY,
    int flashFg,
    int flashBg,
    uint32_t rowFillColorARGB32,
    BDCC_Opacity rowFillOpacity,
    BDCC_UTF32 ch
    )
{
    /* We need to fill the rectangle with the window fill color followed by the character background color
    ** In order to avoid performing two fills (thus wasting bandwidth) we combine them into one fill
    ** according to the matrix below
    **/

    /*                                         Win Fill Opacity
    **
    ** B                       Solid         Flash      Translucent   Transparent
    ** g                       -----         -----      -----------   -----------
    **        Solid           bg color      bg color      bg color     bg color
    ** O      -----
    ** p      Flash           bg color      bg color      bg color     bg color
    ** a      -----
    ** c    Translucent      blend a=100   blend a=100   blend a=50    bg color
    ** i    -----------
    ** t    Transparent       win color     win color     win color    bg color
    ** y    -----------
    **/



    uint32_t fillColorARGB32 = rowFillColorARGB32; /* default the fill color to the window fill color */

    if(rowFillOpacity == BDCC_Opacity_Transparent)
    {
        /* make Alpha 0 */
        fillColorARGB32 &= ~0xff000000;
    }
    else if(rowFillOpacity == BDCC_Opacity_Translucent)
    {
        /* make Alpha 50% */
        fillColorARGB32 &= ~0xff000000;
        fillColorARGB32 |=  0x80000000;
    }

    if(ch && !flashBg)
    {
        if( (row->hWinLibHandle->backGroundOpacity == BDCC_Opacity_Flash) ||
            (row->hWinLibHandle->backGroundOpacity == BDCC_Opacity_Solid) ||
            (rowFillOpacity == BDCC_Opacity_Transparent))
        {
            fillColorARGB32 = row->hWinLibHandle->backGroundColorARGB32;
        }
        else if(row->hWinLibHandle->backGroundOpacity == BDCC_Opacity_Translucent)
        {
            if(rowFillOpacity == BDCC_Opacity_Translucent)
            {
                fillColorARGB32 = ColorAvg(rowFillColorARGB32, row->hWinLibHandle->backGroundColorARGB32, 0x80);
            }
            else
            {
                fillColorARGB32 = ColorAvg(rowFillColorARGB32, row->hWinLibHandle->backGroundColorARGB32, 0xff);
            }
        }
    }

    fillRectangle(row, fillRect, fillColorARGB32);

    if(ch)
    {
        if(!flashFg)
        {
            BDCC_WINLIB_SetPenPosition(row, penX, penY);
            BDCC_WINLIB_DrawString(row, &ch, 1);

            if ( row->hWinLibHandle->underline )
            {
                BDCC_WINLIB_DrawHLine(
                    row,
                    fillRect->x,
                    (row->hWinLibHandle->maxGlyphHeight[BDCC_PenSize_Large] -BDCC_WINLIB_ULINE_HEIGHT),
                    BDCC_WINLIB_ULINE_HEIGHT,
                    fillRect->width,
                    row->hWinLibHandle->foreGroundColorARGB32 );
            }
        }
    }
}



#if FLASH_BY_2SURFACES
int BDCC_WINLIB_RenderChar(
    BDCC_WINLIB_hRow row,
    BDCC_WINLIB_hRow flashrow,
    BDCC_UTF32 ch
    )
#elif FLASH_BY_RERENDER
int BDCC_WINLIB_RenderChar(
    BDCC_WINLIB_hRow row,
    int flashCycleState,
    BDCC_UTF32 ch
    )
#else
int BDCC_WINLIB_RenderChar(
    BDCC_WINLIB_hRow row,
    BDCC_UTF32 ch
    )
#endif
{
    int32_t glyphAdvance, glyphWidth, glyphHeight;
    BDCC_WINLIB_Rect rc;
    uint32_t xPos, yPos;
    int32_t error;
    uint32_t ExtentSpacePre = 0;
    bfont_t hFont = row->hWinLibHandle->font;
    BDCC_WINLIB_hRow temprow;
#if FLASH_BY_2SURFACES
    BDCC_WINLIB_hRow tempFlashrow = NULL;
#endif

    BDBG_MSG(( "DoRenderChar: enter 0x%02x\n", ch));

    /* we only render into the temprow surface for Full, Center or Right justification.
    ** For left justification we render directly into the visible row surface.
    */
    temprow = ( BDCC_Justify_eLeft == row->justification ) ?
        row : row->hWinLibHandle->pTempRowSurface;

#if FLASH_BY_2SURFACES
    if(flashrow)
    {
        tempFlashrow = ( BDCC_Justify_eLeft == row->justification ) ?
        flashrow : row->hWinLibHandle->pTempFlashRowSurface;
    }
#endif

    /*
    ** first determine various dimensions:
    **   iGlyphAdvance - distance between this glyph origin and next glyph origin (used to advance the pen)
    **   iGlyphWidth        - width of the glyph (differs from the advance for italic fonts)
    **   rc.width       - width of char background
    */
    if ( ch == 0 )
    {
        /*
        ** This is some form of transparent whitespace, use the width of a space character.
        */
        BDCC_UTF32 tempChar = 0x20;

        error = bfont_get_glyph_metrics( hFont, tempChar, &glyphWidth, &glyphHeight, &glyphAdvance);

        /*
        ** If we failed, you have to wonder if a font has been loaded,
        ** pick a number just to keep things going.
        */
        if ( error || glyphAdvance == 0  )
        {
            BDBG_MSG(("Failed function bfont_get_glyph_metrics()"));
            glyphAdvance = row->hWinLibHandle->maxGlyphWidth[row->hWinLibHandle->penSize];
        }

    }
    else
    {
        error = bfont_get_glyph_metrics( hFont, ch, &glyphWidth, &glyphHeight, &glyphAdvance);

        /*("--- %c \th=%d \tw=%d\n", ch, iGlyphHeight, iGlyphAdvance );*/

        /*
        ** Will "error" always be non-zero if "iGlyphAdvance == 0"?
        ** In any event, this character is not supported by the selected font.
        ** For most (all?) situations the standard says to use "underline"
        ** for unsupport characters.
        */
        if ( error || glyphAdvance == 0 )
        {
            ch = 0x5F;  /* i.e. the underline character */

            error = bfont_get_glyph_metrics( hFont, ch, &glyphWidth, &glyphHeight, &glyphAdvance);

            /*
            ** If we failed again, try the space character.
            */
            if ( error || glyphAdvance == 0  )
            {
                ch = 0x20;
                error = bfont_get_glyph_metrics( hFont, ch, &glyphWidth, &glyphHeight, &glyphAdvance);

                /*
                ** If we failed yet again, you have to wonder if a font has been loaded,
                ** pick a number just to keep things going.
                */
                if ( error || glyphAdvance == 0  )
                {
                    BDBG_MSG(("Failed function bfont_get_glyph_metrics()"));
                    glyphAdvance = row->hWinLibHandle->maxGlyphWidth[row->hWinLibHandle->penSize];
                }
            }
        }
    }
    rc.width = glyphAdvance ;


    /*
    ** next, clip to the edge of the surface
    */
    BDCC_WINLIB_GetPenPosition(row, &xPos, &yPos);

    /* calculate the window fill extent */

    /* For each character we need to perform a caption window fill color fill
    ** followed by a character background color fill (we could combine them)
    ** We perform this for each character in turn but we have to be careful
    ** that we choose between the glyph advance and glyph width. for italics
    ** we will need to choose the glyph width since it has a greater extent than
    ** the glyph advance
    **/

    rc.x = row->hWinLibHandle->tempSurfFillExtent;
    rc.width = xPos + ((glyphWidth > glyphAdvance) ? glyphWidth : glyphAdvance) - rc.x;

    /*rc.width += 3;*//* TODO FIXME!!! */

    rc.width = (rc.width > 0) ? rc.width : 1;
    row->hWinLibHandle->tempSurfFillExtent += rc.width;

    if ( row->hWinLibHandle->tempSurfFillExtent > row->width)
    {
        /* beyond the edge */
        BDBG_MSG(( "DoRChar(%02x):  beyond the edge\n", ch));
        /* ensure we don't render any more chars now or later */
        BDCC_WINLIB_SetPenPosition(row, row->width-1, yPos);

        /* bail out */
        return(0);
    }

    rc.y = yPos;
        yPos = row->hWinLibHandle->maxGlyphHeight[BDCC_PenSize_Large] - row->hWinLibHandle->maxGlyphHeight[row->hWinLibHandle->penSize];

    /* height of the row may change as more characters are recieved therefore assume worst case fill */
    rc.height = row->hWinLibHandle->maxGlyphHeight[BDCC_PenSize_Large];

#if FLASH_BY_RERENDER
    CompositeRectangle( temprow, &rc, xPos, yPos,
        ((row->hWinLibHandle->foreGroundOpacity == BDCC_Opacity_Flash) && (flashCycleState == 0)) ? 1 : 0 ,
        ((row->hWinLibHandle->backGroundOpacity == BDCC_Opacity_Flash) && (flashCycleState == 0)) ? 1 : 0 ,
        row->fillColorARGB32, row->fillOpacity, ch );
#else
    CompositeRectangle( temprow, &rc, xPos, yPos, 0, 0, row->fillColorARGB32, row->fillOpacity, ch );
#endif

#if FLASH_BY_2SURFACES
    if(flashrow)
    {
        CompositeRectangle( tempFlashrow, &rc, xPos, yPos,
            (row->hWinLibHandle->foreGroundOpacity == BDCC_Opacity_Flash) ? 1 : 0 ,
            (row->hWinLibHandle->backGroundOpacity == BDCC_Opacity_Flash) ? 1 : 0 ,
            row->fillColorARGB32,
            (row->fillOpacity == BDCC_Opacity_Flash) ? BDCC_Opacity_Transparent : row->fillOpacity,
            ch
            );
    }
#endif

    if(ch)
    {
        /* now use the real height for drawing the character */
        rc.height = row->hWinLibHandle->maxGlyphHeight[row->hWinLibHandle->penSize];

        if(rc.height > row->textRect.height)
        {
            row->textRect.y = ( row->hWinLibHandle->maxGlyphHeight[BDCC_PenSize_Large] - rc.height );
            row->textRect.height = rc.height;
        }
    }

    BDCC_WINLIB_SetPenPosition(row, xPos + glyphAdvance, rc.y);

    BDBG_MSG(( "DoRenderChar: exit\n")) ;

    row->hWinLibHandle->tempSurfIncRenderExtent += (glyphAdvance + ExtentSpacePre);

    return glyphAdvance + ExtentSpacePre;

} /* end of BDCC_WINLIB_RenderChar() */




/* Copy temp surface to the row surface and justify */
#if FLASH_BY_2SURFACES
void BDCC_WINLIB_RenderEnd(
    BDCC_WINLIB_hRow row,
    BDCC_WINLIB_hRow flashrow
    )
#else
void BDCC_WINLIB_RenderEnd(
    BDCC_WINLIB_hRow row
    )
#endif
{
    int berr;
    BDCC_WINLIB_Rect srcRect; /* source rectangle for the copy */
    BDCC_WINLIB_Rect destRect; /* destination rectangle for the copy */
    BDCC_WINLIB_hRow temprow = row->hWinLibHandle->pTempRowSurface;
#if FLASH_BY_2SURFACES
    BDCC_WINLIB_hRow tempFlashrow = row->hWinLibHandle->pTempFlashRowSurface;
#endif

    /* For Center and Right justification, copy from the temp surface with justification */
    if ( BDCC_Justify_eLeft != row->justification )
    {
        BDBG_MSG(( "source surface %p   dest surface %p\n", (void*)temprow, (void*)row));


        /* for Right, Center and Full justifications we perform the copy when the
        ** ETX (End of Text) command has been received and we copy the entire line
        ** justifying accoringly
        **/
        destRect.height = srcRect.height = row->textRect.height;
        destRect.y = srcRect.y = row->textRect.y;
        destRect.width = srcRect.width = row->hWinLibHandle->tempSurfFillExtent;

        if ( BDCC_Justify_eRight  == row->justification )
        {
            destRect.x = row->width - row->hWinLibHandle->tempSurfFillExtent;
        }
        else if ( BDCC_Justify_eCenter == row->justification )
        {
            destRect.x = (row->width - row->hWinLibHandle->tempSurfFillExtent) / 2;
        }
        else /* Don't support FULL justification, instead, LEFT justify it */
        {
            destRect.x = 0;
        }

        srcRect.x = 0;

        /* check and clip of course */
        if ( (destRect.x + destRect.width ) > row->width )
        {
            BDBG_MSG(( "DoREnd: clipping\n"));
            return;
        }

        row->textRect = destRect; /* caption library needs this to clip the row later */

        /* this is slow */
        berr = BDCC_WINLIB_Copy(temprow, row, &srcRect, &destRect);
        if(berr)
        {
            BDBG_MSG(( "DoREnd:  BDCC_WINLIB_Copy returned %d\n", berr));
        }

#if FLASH_BY_2SURFACES
        if(flashrow) /* flashrow can legitimately be NULL if there is no flash in this row */
        {
            flashrow->textRect = destRect;

            /* this is slow */
            berr = BDCC_WINLIB_Copy(tempFlashrow, flashrow, &srcRect, &destRect);
            if(berr)
            {
                BDBG_MSG(( "DoREnd:  BDCC_WINLIB_Copy returned %d\n", berr));
            }
        }
#endif
    }
} /* BDCC_WINLIB_RenderEnd */



static const BDCC_WINLIB_Interface gWinlibIf =
{
    BDCC_WINLIB_CreateCaptionRow,
    BDCC_WINLIB_DestroyCaptionRow,
    BDCC_WINLIB_ClearCaptionRow,
    BDCC_WINLIB_SetCharFGColor,
    BDCC_WINLIB_SetCharBGColor,
    BDCC_WINLIB_SetCharEdgeColor,
    BDCC_WINLIB_SetCharEdgeType,
    BDCC_WINLIB_SetFont,
    BDCC_WINLIB_SetCaptionRowZorder,
    BDCC_WINLIB_SetCaptionRowClipRect,
    BDCC_WINLIB_SetCaptionRowDispRect,
    BDCC_WINLIB_GetCaptionRowTextRect,
    BDCC_WINLIB_GetDisplayRect,
    BDCC_WINLIB_GetSurfaceRect,
    BDCC_WINLIB_GetMaxBoundingRect,
    BDCC_WINLIB_GetFrameBufferSize,
    BDCC_WINLIB_SetFrameBufferSize,
    BDCC_WINLIB_HideDisp,
    BDCC_WINLIB_UpdateScreen,
    BDCC_WINLIB_SetClipState,
    BDCC_WINLIB_SetCaptionRowVisibility,
    BDCC_WINLIB_IsCaptionRowVisible,
    BDCC_WINLIB_RenderStart,
    BDCC_WINLIB_RenderChar,
    BDCC_WINLIB_RenderEnd
};



void BDCC_WINLIB_GetInterface(
    BDCC_WINLIB_Interface *winlibIf
    )
{
    *winlibIf = gWinlibIf;
}

