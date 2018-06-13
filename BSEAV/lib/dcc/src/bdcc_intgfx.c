/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

/****************************************************************************
 *                    T h e o r y  O f  O p e r a t i o n
 *
 * Relationship to bdccgfx.c
 *
 * Most calls to the Closed Caption Graphics layer (bdccgfx.c) go through
 * this module.
 *
 * Window Buffers and Text Segments
 *
 * At this layer in the stack of 708 DTVCC libraries, we maintain the
 * Window Buffers.  A Window Buffer is an array of Text Segments.  A
 * Text Segment is a row of characters for one row of a window (or it's
 * a column of characters if the print direction is vertical).  By saving or
 * buffering the characters, we gain some advantages (as opposed to merely
 * rendering characters to a surface and disposing of the character).  These
 * advantages relate to the ability to re-render:
 *
 * 1.  Flashing -- flashing can be implemented by re-rendering.
 * 2.  BackSpace and SetPenLocation -- these are best handled by re-rendering
 * 3.  Create Window on Show -- the option of delaying the creation of a window
 *     and hence its surface(s) (a limited resource) until the window is shown
 *     is impossible without buffering
 *
 * Flash By Re-Rendering
 *
 * Flashing pen foregrounds and backgrounds are supported; flashing window fills
 * are not supported.  Flashing is implemented in two different ways that are
 * controlled by two compile-time build defines:  FLASH_BY_RERENDER and
 * FLASH_BY_2SURFACES.
 *
 * If FLASH_BY_RERENDER is 1, then the flashing is accomplished by re-rendering
 * the rows in which there are flashing foregrounds or backgrounds.  Rows that
 * don't have any flashing are not re-rendered.  This is done through a joint
 * effort of the bcmDccGfx.c file and this file.   This module keeps
 * track of which rows have flashing characters and keeps track of the timing.
 * The RenderRow function calls ccgfxRenderBegin in bcmDccGfx.c, passing in a
 * boolean which defines which 'phase' of the flash is to be rendered.
 *
 * If FLASH_BY_2SURFACES is 1, then flashing is accomplished by using 2 row
 * surfaces for every window row instead of 1, and then switching between the
 * two.  To not unnecessarily consume row surfaces, only those rows that actually
 * have flashing characters will be assigned two surfaces.  This is handled solely
 * by bcmDccGfx.c.
 *
 * Create Window on Show
 *
 * This implementation attempts to reduce the surface memory used by the lower
 * bcmDccGfx.c layer (the Closed Captioning Graphics layer) by delaying the
 * window creation until the window is actually visible.  This is done by buffering
 * the characters in the Window Buffers and keeping track of the windows' visibility
 * states.  When a window becomes visible, then a call is made to the graphics layer
 * to actually create the window (and hence to assign the surface memory).  TODO: add
 * support for the alternate method of rendering on the fly, controlled by a compile-
 * time build define.
 *
 * gifxUpdateWindow and gifxAccumulateChar
 *
 * The two entry points of the module are gifxUpdateWindow and gifxAccumulateChar.
 * gifxUpdateWindow dispatches the appropriate handlers for the window attributes
 * and gifxAccumulateChar buffers the incoming characters to the window buffers.
 *
 ****************************************************************************/


                        /********************
                         *
                         * Includes
                         *
                         ********************/

#include "b_api_shim.h"
#include "bdcc_priv.h"
#include "bdcc_coding.h"
#include "bdcc_int.h"
#include "bdcc_intgfx.h"
#include "bdcc_gfx.h"
#include "bdcc_bits.h"
#include "assert.h"


BDBG_MODULE(BDCCINTGFX);

                        /********************
                         *
                         * Defines
                         *
                         ********************/

/* GT - Need to define this to 1 otherwise we often miss the bottom line of a scrolling window because some
** broadcasters don't liberally add ETX's until the end of the line when it is about to scroll up
*/
#define BCCGFX_INTGFX_P_HONOR_LEFTJUST_SPECIAL_CASES            1


                        /********************
                         *
                         * Types
                         *
                         ********************/

typedef void (* BDCC_INT_P_UpdateHandler)(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window *) ;
typedef struct
{
    int                         fContinue ;
    BDCC_INT_P_UpdateHandler        pfnHandler ;
    unsigned int                ConditionMask ;
} BDCC_INT_P_UpdateHandlerPair ;



                        /********************
                         *
                         * Local Prototypes
                         *
                         ********************/

static void BDCC_INTGFX_P_WindowDefine(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void BDCC_INTGFX_P_WindowDelete(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void BDCC_INTGFX_P_WindowPosition(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void BDCC_INTGFX_P_WindowFill(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void BDCC_INTGFX_P_WindowClear(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void BDCC_INTGFX_P_WindowSize(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void BDCC_INTGFX_P_PenFgColor(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void BDCC_INTGFX_P_PenBgColor(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void BDCC_INTGFX_P_PenEdgeColor(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void BDCC_INTGFX_P_PenEdgeType(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void BDCC_INTGFX_P_PrintDirection(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void BDCC_INTGFX_P_Justify(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void BDCC_INTGFX_P_Visible(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void BDCC_INTGFX_P_PenLocation(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void CalculateWindowPos(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
void RenderWnd(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
static void RenderRow(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw, int Row, int ts, int fForce);
static void ClearAllTS(BDCC_INT_P_Window * pw) ;
static void ClearTS(BDCC_INT_P_Window * pw, int ts);
static void ResizeWndBuf(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw, int OldRowCount, int NewRowCount);
static void RotateWndBuf(BDCC_INT_P_Window * pw, int RotateAmt, int OldRowCount);



                        /********************
                         *
                         * Globals
                         *
                         ********************/


static const BDCC_INT_P_UpdateHandlerPair UpdateHandlers[] =
{
    {1, BDCC_INTGFX_P_WindowDefine,     (BDCC_ATTR_MASK_DEFINED_SET)},
    {1, BDCC_INTGFX_P_WindowPosition,       (BDCC_ATTR_MASK_ANCHOR)},
    {1, BDCC_INTGFX_P_WindowSize,           (BDCC_ATTR_MASK_PENSIZE | BDCC_ATTR_MASK_RCCOUNT | BDCC_ATTR_MASK_FONTSTYLE)},
    {1, BDCC_INTGFX_P_WindowFill,           (BDCC_ATTR_MASK_FILL | BDCC_ATTR_MASK_FILLOP)},
    {1, BDCC_INTGFX_P_WindowClear,          (BDCC_ATTR_MASK_CLEARWINDOW)},
    {1, BDCC_INTGFX_P_PenFgColor,           (BDCC_ATTR_MASK_PENFG)},
    {1, BDCC_INTGFX_P_PenBgColor,           (BDCC_ATTR_MASK_PENBG)},
    {1, BDCC_INTGFX_P_PenEdgeColor,     (BDCC_ATTR_MASK_EDGECOLOR)},
    {1, BDCC_INTGFX_P_PenEdgeType,      (BDCC_ATTR_MASK_EDGETYPE)},
    {1, BDCC_INTGFX_P_PrintDirection,       (BDCC_ATTR_MASK_PRINTDIR)},
    {1, BDCC_INTGFX_P_Justify,              (BDCC_ATTR_MASK_JUSTIFY)},
    {1, BDCC_INTGFX_P_PenLocation,          (BDCC_ATTR_MASK_PENLOC)},
    {1, BDCC_INTGFX_P_Visible,              (BDCC_ATTR_MASK_VISIBLE)},
    {1, BDCC_INTGFX_P_WindowDelete,     (BDCC_ATTR_MASK_DEFINED_CLR)},
} ;
#define BDCC_INT_P_NUM_UPDATE_HANDLER_PAIRS     (sizeof(UpdateHandlers)/sizeof(UpdateHandlers[0]))


                        /********************
                         *
                         * Functions
                         *
                         ********************/


void BCCGFX_INT_P_Reset(BDCC_INT_P_Handle hCodObject)
{
    BSTD_UNUSED(hCodObject);
}

#define CHECK_FLAG(flg)         if ( pw->UpdateMask & (flg)) {BDBG_ERR((#flg "\n"));}
void BCCGFX_INT_P_UpdateWindow(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    unsigned int h ;

    for ( h=0 ; h < BDCC_INT_P_NUM_UPDATE_HANDLER_PAIRS ; h++ )
    {
        if ( pw->UpdateMask & UpdateHandlers[h].ConditionMask )
        {
            UpdateHandlers[h].pfnHandler(hCodObject, pw) ;
            if ( ! UpdateHandlers[h].fContinue )
                break ;
        }
    }
}



#if CREATE_ONLY_VISIBLE_WINDOWS
void BDCC_INTGFX_P_WindowDefine(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BDBG_MSG(("BDCC_INTGFX_P_WindowDefine %d\n", pw->WndId)) ;
            ClearAllTS(pw) ;
            pw->WndBuf.TsRowScrollAdj = 0 ;
//  wbClear(&pw->WndBuf) ;
}


void BDCC_INTGFX_P_WindowDelete(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BDBG_MSG(( "BDCC_INTGFX_P_WindowDelete %d\n", pw->WndId)) ;
    pw->WindowDef.Visible = 0 ;
    BDCC_INTGFX_P_Visible(hCodObject, pw) ;
}

void BDCC_INTGFX_P_Visible(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BDBG_MSG(("BDCC_INTGFX_P_Visible %d\n", pw->WndId)) ;
    if ( pw->WindowDef.Visible )
    {
        BCCGFX_P_WndCreate(hCodObject->hCCGfxHandle, pw->WndId, pw->WindowDef.RowCount, pw->WindowDef.ColCount) ;
        pw->ccgfxCreated = 1 ;
        BCCGFX_P_SetPenSize(hCodObject->hCCGfxHandle, pw->WndId, pw->Pen.Attr.PenSize) ;
        BDCC_INTGFX_P_WindowPosition(hCodObject, pw) ;
        BCCGFX_P_SetPriority(hCodObject->hCCGfxHandle, pw->WndId, pw->WindowDef.Priority) ;
        BDCC_INTGFX_P_WindowFill(hCodObject, pw) ;
        BCCGFX_P_WndClear(hCodObject->hCCGfxHandle, pw->WndId) ;
        // BDCC_INTGFX_P_PenFgColor(hCodObject, pw) ;
        // BDCC_INTGFX_P_PenBgColor(hCodObject, pw) ;
        //BDCC_INTGFX_P_WindowSize(hCodObject, pw) ;
        BDCC_INTGFX_P_PrintDirection(hCodObject, pw) ;
        BDCC_INTGFX_P_Justify(hCodObject, pw) ;

        RenderWnd(pw) ;
        BCCGFX_P_WndShow(hCodObject->hCCGfxHandle, pw->WndId, 1) ;
    }
    else
    {
        if ( pw->ccgfxCreated )
        {
//          BCCGFX_P_ShowWindow(hCodObject->hCCGfxHandle, pw->WndId, 0) ;
            BCCGFX_P_WndDestroy(hCodObject->hCCGfxHandle, pw->WndId) ;
            pw->ccgfxCreated = 0 ;
        }
    }
}

#else /* not CREATE_ONLY_VISIBLE_WINDOWS */
void BDCC_INTGFX_P_WindowDefine(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BDBG_MSG(("BDCC_INTGFX_P_WindowDefine %d\n", pw->WndId)) ;
    ClearAllTS(pw) ;
    pw->WndBuf.TsRowScrollAdj = 0 ;
    BCCGFX_P_WndCreate(hCodObject->hCCGfxHandle, pw->WndId, pw->WindowDef.RowCount, pw->WindowDef.ColCount) ;
    pw->ccgfxCreated = 1 ;
}


void BDCC_INTGFX_P_WindowDelete(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BDBG_MSG(( "BDCC_INTGFX_P_WindowDelete %d\n", pw->WndId)) ;
    BCCGFX_P_WndShow(hCodObject->hCCGfxHandle, pw->WndId, 0) ;
    BCCGFX_P_WndDestroy(hCodObject->hCCGfxHandle, pw->WndId) ;
    pw->ccgfxCreated = 0 ;
}

void BDCC_INTGFX_P_Visible(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BDBG_MSG(("BDCC_INTGFX_P_Visible %d\n", pw->WndId)) ;
    BCCGFX_P_SetPriority(hCodObject->hCCGfxHandle, pw->WndId, pw->WindowDef.Priority) ;
    BCCGFX_P_WndShow(hCodObject->hCCGfxHandle, pw->WndId, pw->WindowDef.Visible) ;
}

#endif /* not CREATE_ONLY_VISIBLE_WINDOWS */



void BDCC_INTGFX_P_WindowPosition(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BDBG_MSG(("BDCC_INTGFX_P_WindowPosition %d\n", pw->WndId)) ;
    CalculateWindowPos(hCodObject, pw) ;
    if ( pw->ccgfxCreated )
    {
        BCCGFX_P_WndResize(hCodObject->hCCGfxHandle, pw->WndId, pw->WindowDef.RowCount, pw->WindowDef.ColCount) ;
        BCCGFX_P_WndPosition(hCodObject->hCCGfxHandle, pw->WndId, pw->rWindowAnchor, pw->cWindowAnchor, pw->WindowDef.AnchorPoint, hCodObject->uiColumns) ;
    }
}


void BDCC_INTGFX_P_WindowFill(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BDBG_MSG(("BDCC_INTGFX_P_WindowFill %d\n", pw->WndId)) ;
    if ( pw->ccgfxCreated )
    {
        bool winFillColorOvr = (hCodObject->OverrideMask & BDCC_ATTR_MASK_FILL);
        bool winFillOpacityOvr = (hCodObject->OverrideMask & BDCC_ATTR_MASK_FILLOP);

        BCCGFX_P_SetWindowFill(hCodObject->hCCGfxHandle, pw->WndId,
            winFillOpacityOvr ? hCodObject->Overrides.WinOpacity : pw->WindowAttr.FillOpacity,
            winFillColorOvr ? hCodObject->Overrides.WinColor : pw->WindowAttr.FillColor) ;


        BCCGFX_P_WndClear(hCodObject->hCCGfxHandle, pw->WndId) ;
        RenderWnd(hCodObject, pw) ;
    }
}

void BDCC_INTGFX_P_WindowClear(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BDBG_MSG(("BDCC_INTGFX_P_WindowClear %d\n", pw->WndId)) ;
    if ( pw->ccgfxCreated )
    {
        BCCGFX_P_WndClear(hCodObject->hCCGfxHandle, pw->WndId) ;
    }
    BDBG_MSG(( "BDCC_INTGFX_P_WindowClear: clearing PendingScroll\n")) ;
    pw->fPendingScroll = 0 ;
    ClearAllTS(pw) ;
    BDBG_MSG(("BDCC_INTGFX_P_WindowClear %d DONE\n", pw->WndId)) ;
}

void BDCC_INTGFX_P_PenFgColor(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BSTD_UNUSED(hCodObject);
    BDBG_MSG(("BDCC_INTGFX_P_PenFgColor %d\n", pw->WndId)) ;
}

void BDCC_INTGFX_P_PenEdgeColor(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BSTD_UNUSED(hCodObject);
    BDBG_MSG(("BDCC_INTGFX_P_PenEdgeColor %d\n", pw->WndId)) ;
}

void BDCC_INTGFX_P_PenEdgeType(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BSTD_UNUSED(hCodObject);
    BDBG_MSG(("BDCC_INTGFX_P_PenEdgeType %d\n", pw->WndId)) ;
}

void BDCC_INTGFX_P_WindowSize(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BDBG_MSG(("BDCC_INTGFX_P_WindowSize %d\n", pw->WndId)) ;

    ResizeWndBuf(hCodObject, pw, pw->WndBuf.SavedRowCount, pw->WindowDef.RowCount) ;

    CalculateWindowPos(hCodObject, pw) ;
    if ( pw->ccgfxCreated )
    {
        /*BCCGFX_P_SetPenSize(hCodObject->hCCGfxHandle, pw->WndId, pw->Pen.Attr.PenSize) ; */
        BCCGFX_P_WndResize(hCodObject->hCCGfxHandle, pw->WndId, pw->WindowDef.RowCount, pw->WindowDef.ColCount) ;
        BCCGFX_P_WndPosition(hCodObject->hCCGfxHandle, pw->WndId, pw->rWindowAnchor, pw->cWindowAnchor, pw->WindowDef.AnchorPoint, hCodObject->uiColumns) ;
    }
    pw->WndBuf.SavedRowCount = pw->WindowDef.RowCount ;
}

void BDCC_INTGFX_P_PenBgColor(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BSTD_UNUSED(hCodObject);
    BDBG_MSG(("BDCC_INTGFX_P_PenBgColor %d\n", pw->WndId)) ;
}

void BDCC_INTGFX_P_PrintDirection(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BDBG_MSG(("BDCC_INTGFX_P_PrintDirection %d\n", pw->WndId)) ;
    if ( pw->ccgfxCreated )
        BCCGFX_P_SetPrintDirection(hCodObject->hCCGfxHandle, pw->WndId, pw->WindowAttr.PrintDirection) ;
}

void BDCC_INTGFX_P_Justify(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BDBG_MSG(("BDCC_INTGFX_P_Justify %d\n", pw->WndId)) ;
    if ( pw->ccgfxCreated )
        BCCGFX_P_SetJustification(hCodObject->hCCGfxHandle, pw->WndId, pw->WindowAttr.Justify) ;
}


void BDCC_INTGFX_P_PenLocation(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    BSTD_UNUSED(hCodObject);
    BDBG_MSG(("BDCC_INTGFX_P_PenLocation %d, rc = (%d,%d)\n", pw->WndId, pw->Pen.Row, pw->Pen.Col)) ;
}


void CalculateWindowPos(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    int cAnchor, rAnchor;

    /*
     * determine the anchor point in row,col
     * where row is 0 to 14 and col is 0 to 31 or 41
     */
    if ( pw->WindowDef.RelativePos )
    {
        cAnchor = pw->WindowDef.AnchorHorz * hCodObject->uiColumns / 100 ;
        rAnchor = pw->WindowDef.AnchorVert * BDCC_P_CY15 / 100 ;
    }
    else
    {
        cAnchor = pw->WindowDef.AnchorHorz * hCodObject->uiColumns / (hCodObject->uiColumns* BDCC_P_GRID160210) ;
        rAnchor = pw->WindowDef.AnchorVert * BDCC_P_CY15 / BDCC_P_CYGRID75 ;
    }

    pw->rWindowAnchor = rAnchor ;
    pw->cWindowAnchor = cAnchor ;

} /* CalculateWindowRect */



static int IsPrintable(unsigned char ch)
{
    return(ch >= 0x20) ;
}

void CalcDirectionalValues(
    BDCC_INT_P_Window * pw,
    int ** ppPrimaryRC,
    int ** ppSecondaryRC,
    int * pPrimaryRCInc,
    int * pSecondaryRCInc,
    int * pPrimaryRCReset,
    int * pSecondaryRCLimit)
{
#if 1
    /* we're only supporting this right now so we hard code for Coverity checker */
    *ppPrimaryRC = &pw->Pen.Col ;
    *ppSecondaryRC = &pw->Pen.Row ;
    *pPrimaryRCInc = 1;
    *pSecondaryRCInc = 1;
    *pPrimaryRCReset = 0;
    *pSecondaryRCLimit = pw->WindowDef.RowCount ;
#else
    BCCGFX_P_Direction PrtDir = pw->WindowAttr.PrintDirection ;
    BCCGFX_P_Direction ScrDir = pw->WindowAttr.ScrollDirection ;

    /* we're only supporting this right now */
    PrtDir = BCCGFX_P_Direction_eLeftToRight ;
    ScrDir = BCCGFX_P_Direction_eBottomToTop ;

    if ( PrtDir == BCCGFX_P_Direction_eLeftToRight
     ||  PrtDir == BCCGFX_P_Direction_eRightToLeft )
    {
        /* horizontal */
        *ppPrimaryRC = &pw->Pen.Col ;
        *ppSecondaryRC = &pw->Pen.Row ;
        *pPrimaryRCInc = ((PrtDir == BCCGFX_P_Direction_eLeftToRight) ? 1 : -1) ;
        *pSecondaryRCInc = ((ScrDir == BCCGFX_P_Direction_eBottomToTop) ? 1 : -1) ;
        *pPrimaryRCReset = ((PrtDir == BCCGFX_P_Direction_eLeftToRight) ? 0 : pw->WindowDef.ColCount-1) ;
        *pSecondaryRCLimit = pw->WindowDef.RowCount ;
    }
    else
    {
        /* vertical */
        *ppPrimaryRC = &pw->Pen.Row ;
        *ppSecondaryRC = &pw->Pen.Col ;
        *pPrimaryRCInc = ((PrtDir == BCCGFX_P_Direction_eTopToBottom) ? 1 : -1) ;
        *pSecondaryRCInc = ((ScrDir == BCCGFX_P_Direction_eRightToLeft) ? 1 : -1) ;
        *pPrimaryRCReset = ((PrtDir == BCCGFX_P_Direction_eTopToBottom) ? 0 : pw->WindowDef.RowCount-1) ;
        *pSecondaryRCLimit = pw->WindowDef.ColCount ;
    }
#endif
}


void ClearAllTS(BDCC_INT_P_Window * pw)
{
    int ts ;

    for ( ts=0 ; ts < BDCC_COD_P_NUM_TEXT_SEGMENTS ; ts++ )
    {
        ClearTS(pw, ts) ;
    }
}

void ClearTS(BDCC_INT_P_Window * pw, int ts)
{
    int i ;

    for ( i=0 ; i < BDCC_COD_P_TEXT_SEGMENT_LEN ; i++ )
    {
        pw->WndBuf.aTextSeg[ts].aCharInfo[i].ch = 0 ;
    }
    pw->WndBuf.aTextSeg[ts].fRenderNeeded = 0 ;
    pw->WndBuf.aTextSeg[ts].fClrRowNeeded = 0 ;
    pw->WndBuf.aTextSeg[ts].iBeyondLastChar = 0 ;
    pw->WndBuf.aTextSeg[ts].iLastBeyondLastChar = 0 ;
    pw->WndBuf.aTextSeg[ts].fContainsFlash = 0 ;
}


unsigned char MapMultiByteTextToC1(
    BDCC_INT_P_Handle hCodObject,
    unsigned char ch,
    unsigned int AddlBytes,
    unsigned char * pAddlBytes)
{
    assert(hCodObject);

    if ( ch != 0x10   ||   AddlBytes != 1 )
        return(0) ;

    switch ( *pAddlBytes )
    {
        case 0x20 : ch = 0x80 ; break ;
        case 0x21 : ch = 0x81 ; break ;
        case 0x25 : ch = 0x85 ; break ;
        case 0x2A : ch = 0x8A ; break ;
        case 0x2C : ch = 0x8C ; break ;
        case 0x30 : ch = 0x90 ; break ;
        case 0x31 : ch = 0x91 ; break ;
        case 0x32 : ch = 0x92 ; break ;
        case 0x33 : ch = 0x93 ; break ;
        case 0x34 : ch = 0x94 ; break ;
        case 0x35 : ch = 0x95 ; break ;
        case 0x39 : ch = 0x99 ; break ;
        case 0x3A : ch = 0x9A ; break ;
        case 0x3C : ch = 0x9C ; break ;
        case 0x3D : ch = 0x9D ; break ;
        case 0x3F : ch = 0x9F ; break ;
        case 0x76 : ch = 0x82 ; break ;
        case 0x77 : ch = 0x83 ; break ;
        case 0x78 : ch = 0x84 ; break ;
        case 0x79 : ch = 0x86 ; break ;
        case 0x7A : ch = 0x87 ; break ;
        case 0x7B : ch = 0x88 ; break ;
        case 0x7C : ch = 0x89 ; break ;
        case 0x7D : ch = 0x8B ; break ;
        case 0x7E : ch = 0x8F ; break ;
        case 0x7F : ch = 0x8D ; break ;
        case 0xA0 : ch = 0x8E ; break ;
        default   : ch = 0    ; break ;
    }

    return(ch) ;

} /* RemapMultiByteTextToC1 */

void BCCGFX_INT_P_AccumulateChar(
        BDCC_INT_P_Handle hCodObject,
        BDCC_INT_P_Window * pw,
        unsigned char ch,
        unsigned int AddlBytes,
        unsigned char * pAddlBytes
        )
{
    int fPrintable ;
    int * pPrimaryRC ;
    int * pSecondaryRC ;
    int PrimaryRCInc, SecondaryRCInc, PrimaryRCReset, SecondaryRCLimit ;
    int ts  ;
    int chChanged ;
    /* inject ETX can generate too many ETXs, (e.g. 0x8c, 0x89, 0x90, 0x91, 0x90 sequence), so if it is back-to-back ETX, drop it */
    static unsigned char pre_ch = 0;

    BDCC_INT_P_TsElement * pTextElement;

    /* Indicates that the window has been modified and that the screen
    ** should be refreshed at the appropriate time.
    */
    bool bWindowDirty = false ;

    /*
    ** Indicates if the screen should be refreshed before exiting this routine.
    */
    bool bRedrawNeeded = false;

    /*
    ** Will be set to true if "flashing" is enabled in the middle of a line.
    */
    bool bForceReRendering = false;

    /*
     * If we're dealing with a multi-byte
     * character, indicated by a non-zero
     * AddlBytes, then remap it into one
     * of the C1 slots.  Note that the C1
     * processing was done before this point.
     */
    if ( AddlBytes )
    {
        ch = MapMultiByteTextToC1(hCodObject, ch, AddlBytes, pAddlBytes) ;
    }
    /* need to check some back-to-back charaters after multi bytes mapping */
    if (pre_ch == ch) {
        if (0x03 == ch) {
                return;
        }
        /* SWSTB-9657. It is the stream issue in a way to encode misc control and PAC. IF customer want to the fix, pleasse uncomment out following */
#if 0
        if (0x80 == ch) {
            BDBG_MSG(("%s: TSP 0x80\n", __func__));
            return;
        }
#endif
    }
    pre_ch = ch;

    CalcDirectionalValues(pw,
        &pPrimaryRC, &pSecondaryRC,
        &PrimaryRCInc, &SecondaryRCInc,
        &PrimaryRCReset, &SecondaryRCLimit) ;

    if ( 0 )
    {
        BDBG_WRN(("gifxAccumulateChar pw = %p\n", (void*)pw)) ;
        BDBG_WRN(("gifxAccumulateChar SecondaryRC = %d\n", *pSecondaryRC)) ;
        BDBG_WRN(("gifxAccumulateChar TsRowScrollAdj = %d\n", pw->WndBuf.TsRowScrollAdj)) ;
        BDBG_WRN(("gifxAccumulateChar fPendingScroll = %d\n", pw->fPendingScroll)) ;
        BDBG_WRN(("gifxAccumulateChar RowCount = %d\n", pw->WindowDef.RowCount)) ;
    }
    if ( pw->WindowDef.RowCount == 0 )
        return ;

    ts = (*pSecondaryRC + pw->WndBuf.TsRowScrollAdj + pw->fPendingScroll) % pw->WindowDef.RowCount ;

    fPrintable = IsPrintable(ch) ;
    if ( fPrintable )
    {
        /*
         * If we have a pending scroll, do it now.
         */
        if ( pw->fPendingScroll )
        {
            BDBG_MSG(( "PendingScroll, do it now\n")) ;
            BCCGFX_P_Scroll(hCodObject->hCCGfxHandle, pw->WndId) ;
            pw->fPendingScroll = 0 ;
            pw->WndBuf.TsRowScrollAdj = (pw->WndBuf.TsRowScrollAdj + 1) % pw->WindowDef.RowCount ;
            ClearTS(pw, ts) ;
        }
	/* SWSTB-9115 */
        if ( *pPrimaryRC > pw->WindowDef.ColCount )
        {
            /*
             * beyond the width of the window
             * CEB-10 says decoders may ignore
             */
            fPrintable = 0 ;
        }
        else
        {
            BDBG_MSG(( "*pPrimaryRC is %d   ts is %d\n", *pPrimaryRC, ts)) ;

            /*
            ** Save the character in the window buffer and all the relevant pen attributes.
            ** Keep track if anything has truly changed.
            */

            pTextElement = &(pw->WndBuf.aTextSeg[ts].aCharInfo[*pPrimaryRC]);

            chChanged = ( pTextElement->ch != ch ) ;
            pTextElement->ch = ch ;

            chChanged += ( pTextElement->PenAttrArgs[0] != pw->Pen.Attr.CmdArgs[0] ) ;
            pTextElement->PenAttrArgs[0] = pw->Pen.Attr.CmdArgs[0] ;

            chChanged += ( pTextElement->PenAttrArgs[1] != pw->Pen.Attr.CmdArgs[1] ) ;
            pTextElement->PenAttrArgs[1] = pw->Pen.Attr.CmdArgs[1] ;

            chChanged += ( pTextElement->PenColorArgs[0] != pw->Pen.Color.CmdArgs[0] ) ;
            pTextElement->PenColorArgs[0] = pw->Pen.Color.CmdArgs[0] ;

            chChanged += ( pTextElement->PenColorArgs[1] != pw->Pen.Color.CmdArgs[1] ) ;
            pTextElement->PenColorArgs[1] = pw->Pen.Color.CmdArgs[1] ;

            chChanged += ( pTextElement->PenColorArgs[2] != pw->Pen.Color.CmdArgs[2] ) ;
            pTextElement->PenColorArgs[2] = pw->Pen.Color.CmdArgs[2] ;

            /*
             * if fg opacity is flashing  OR   bg opacity is flashing, set flag
             */
            if ( (pw->Pen.Color.CmdArgs[0] & 0xC0) == 0x40   ||   (pw->Pen.Color.CmdArgs[1] & 0xC0) == 0x40 )
            {
#if FLASH_BY_2SURFACES
                            /*
                            ** If flashing is enable in the middle of a row, we have a "do over" or a "mulligan", the
                            ** entire line of text needs to be rerendered from the beginning to this character.
                            ** This is true for both the "standard" and the "flash" surface.
                            */
                if ( false == pw->WndBuf.aTextSeg[ts].fContainsFlash )
                {
                    bForceReRendering = true;
                }
#endif
                pw->WndBuf.aTextSeg[ts].fContainsFlash = 1 ;
            }




            if ( *pPrimaryRC >= pw->WndBuf.aTextSeg[ts].iBeyondLastChar )
            {
                pw->WndBuf.aTextSeg[ts].iBeyondLastChar = 1 + *pPrimaryRC ;
                pw->WndBuf.aTextSeg[ts].fRenderNeeded = 1 ;
            }
            else
            {
                /*
                 * leave iBeyondLastChar alone, because we're
                 * changing some char in the middle
                 * meaning we want to clear the row because we may
                 * be decreasing the entire extent and don't want
                 * leftovers
                 */
                if ( chChanged )
                {
                    pw->WndBuf.aTextSeg[ts].fRenderNeeded = 1 ;
                    pw->WndBuf.aTextSeg[ts].fClrRowNeeded = 1 ;

                    /*
                     * also we have to ensure that we don't do
                     * incremental rendering -- we need full
                     */
                    pw->WndBuf.aTextSeg[ts].iLastBeyondLastChar = 0 ;
                }
            }

        }

        /*
         * Update the Pen position
         */
        *pPrimaryRC += PrimaryRCInc ;
    }

    /*
     * Detect normal end of text segment.
     */
    if ( ch == 0x0d   ||   ch == 0x03   || ch == 0x08)
    {
        /*
        ** This will probably take some tweaking.
        ** Don't mark the window as dirty if we've only gotten "end-of-text" characters.
        ** The streams seem to send these just for fun (or is it logic higher up in the stack?)
        ** Exception for left justified text. We inject ETX upstream to ensure that each "burst"
        ** of characters are displayed for left justification as per the spec.
        */
        if (ch != 0x03 ||  pw->WindowAttr.Justify == BCCGFX_P_Justify_eLeft)
        {
            bWindowDirty = true;
        }

        if ( ch == 0x08 )
        {
            /* backspace */
            if ( pw->WndBuf.aTextSeg[ts].iBeyondLastChar == *pPrimaryRC )
                pw->WndBuf.aTextSeg[ts].iBeyondLastChar-- ;
            /* do we need to scoot left? */
            /* SWSTB-9115 */
            *pPrimaryRC -= PrimaryRCInc ;
            if (*pPrimaryRC < 0)	/* validation */
                *pPrimaryRC = 0;
            pw->WndBuf.aTextSeg[ts].aCharInfo[*pPrimaryRC].ch = 0 ;
            /* don't clear row in BS case */
            pw->WndBuf.aTextSeg[ts].fClrRowNeeded = false ;
            pw->WndBuf.aTextSeg[ts].iLastBeyondLastChar = 0 ;
            pw->WndBuf.aTextSeg[ts].fRenderNeeded = 1 ;
        }

#if CREATE_ONLY_VISIBLE_WINDOWS
        if ( pw->WindowDef.Visible )
#endif
        {
            /* SWSTB-9991. Make sure pFlashSurface is created for corresponding row it no alwasy */
            if (((hCodObject->OverrideMask & BDCC_ATTR_MASK_FILLOP) && (hCodObject->Overrides.WinOpacity == BDCC_Opacity_Flash))) {
                int i;
                for (i = 0; i < pw->WindowDef.RowCount + 1; i++) {
                        if (i != *pSecondaryRC) {
                                BCCGFX_P_FlashEmptyLine(hCodObject->hCCGfxHandle, pw->WndId, i) ;
                        }
                }
            }
            RenderRow(hCodObject, pw, *pSecondaryRC, ts, bForceReRendering) ;
        }


        /*
         *
         */
        if ( ch == 0x0d )
        {
            *pPrimaryRC = PrimaryRCReset ;
            *pSecondaryRC += SecondaryRCInc ;
            /* check for scroll */
            if ( *pSecondaryRC >= SecondaryRCLimit   ||   *pSecondaryRC < 0 )
            {
                pw->fPendingScroll = 1 ;
                *pSecondaryRC -= SecondaryRCInc ;
                BDBG_MSG(("PendingScroll, queue it up, wnd %d pw %p row remains at %d, inc %d, limit %d\n",
                          pw->WndId, (void*)pw,
                        *pSecondaryRC,SecondaryRCInc,SecondaryRCLimit)) ;
            }
            else
            {
                BDBG_MSG(("got CR but no need to scroll\n")) ;
            }
        }
    }
    else if ( ch == 0x0C ) /* FF */
    {
        bWindowDirty = true; /* the window will be modified in some way */

        BDCC_INTGFX_P_WindowClear(hCodObject, pw) ;
        pw->Pen.Row = 0 ;
        pw->Pen.Col = 0 ;
        BDCC_INTGFX_P_PenLocation(hCodObject, pw) ;
    }
    else if ( ch == 0x0E ) /* HCR */
    {
        bWindowDirty = true; /* the window will be modified in some way */

        BCCGFX_P_FillRow(hCodObject->hCCGfxHandle, pw->WndId, pw->Pen.Row) ;
        ClearTS(pw,ts) ;
        pw->Pen.Col = 0 ;
    }

    /*
    ** Redrawing the screen too often is killing performance.
    ** The logic below attempts to only redraw the screen when it is necessary.
    */

    /*
    ** If the window has been modified in some way, set the "dirty" bit to indicate that the
    ** screen should be refeshed at later time.
    */
    if ( bWindowDirty )
    {
        BCCGFX_P_SetDirtyState( hCodObject->hCCGfxHandle, hCodObject->CurrentWindow,  true );
    }

    /*
    ** If the window is "dirty" && "visible" && "not in the middle of a scroll operation",
    ** refresh the screen if we've just received one of the following control codes;
    ** FF, CR, HCR, Backspace, ETX (end-of-text).
    ** Note: if we are scrolling, the refresh will be handled in "BCCGFX_P_Periodic()".
    */
    bRedrawNeeded = BCCGFX_P_GetShowState( hCodObject->hCCGfxHandle, hCodObject->CurrentWindow );
    bRedrawNeeded &= BCCGFX_P_GetDirtyState( hCodObject->hCCGfxHandle, hCodObject->CurrentWindow ) ;
    bRedrawNeeded &= !( pw->fPendingScroll );
    bRedrawNeeded &= !( BCCGFX_P_GetScrollPhase( hCodObject->hCCGfxHandle, pw->WndId ) );

    if ( bRedrawNeeded )
    {
        if ( ch == 0x0d   ||  ch == 0x03   ||  ch == 0x08 || ch == 0x0C || ch == 0x0E )
        {
            BCCGFX_P_RedrawScreen( hCodObject->hCCGfxHandle );
            BCCGFX_P_SetDirtyState( hCodObject->hCCGfxHandle, hCodObject->CurrentWindow,  false );
        }
    }

} /* gifxAccumulateChar */

void RenderWnd(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
    int row ;
    int ts ;
    for ( row=0 ; row < pw->WindowDef.RowCount ; row++ )
    {
        ts = (row + pw->WndBuf.TsRowScrollAdj) % pw->WindowDef.RowCount ;
        pw->WndBuf.aTextSeg[ts].fClrRowNeeded = 1;
        RenderRow(hCodObject, pw, row, ts, 1);
    }
}

#if FLASH_BY_RERENDER
int FlashCycleState = 1 ;
void BCCGFX_INT_P_Periodic(BDCC_INT_P_Handle hCodObject)
{
    static int times = 0 ;
    int wnd ;
    int ts ;
    int row ;
    int needRedraw = 0 ;

    times++ ;

    /*
     * every so often ...
     */
    if ( (times % 20) == 0 )
    {
        BDBG_MSG(( "gifxPeriodic qualified\n")) ;
        FlashCycleState ^= 1 ;

        /*
         * for each window...
         */
        for ( wnd=0 ; wnd < BDCC_INT_P_NUM_WINDOWS ; wnd++ )
        {
            BDCC_INT_P_Window * pw = &hCodObject->WindowInfo[wnd] ;

            /*
             * ...visible windows, that is ...
             */
            if ( pw->WindowDef.Visible
             && pw->fDefined )  /* visible AND defined */
            {

                /*
                 * ...for each row ...
                 */
                for ( row=0 ; row < pw->WindowDef.RowCount ; row++ )
                {

                    /*
                     * re-render the row if it contains any flashing attributes or there are any flash overrides
                     */
                    ts = (row + pw->WndBuf.TsRowScrollAdj + pw->fPendingScroll) % pw->WindowDef.RowCount ;

                    if (( pw->WndBuf.aTextSeg[ts].fContainsFlash ) ||
                        ((hCodObject->OverrideMask & BDCC_ATTR_MASK_PENFGOP)
                                        && (hCodObject->Overrides.FgOpacity  == BDCC_Opacity_Flash)) ||
                        ((hCodObject->OverrideMask & BDCC_ATTR_MASK_PENBGOP)
                                        && (hCodObject->Overrides.BgOpacity  == BDCC_Opacity_Flash)) ||
                        ((hCodObject->OverrideMask & BDCC_ATTR_MASK_FILLOP)
                                        && (hCodObject->Overrides.WinOpacity == BDCC_Opacity_Flash)))
                    {
                        BDBG_MSG(( "ReRendering wnd %d row %d\n", wnd, row)) ;
                        pw->WndBuf.aTextSeg[ts].fClrRowNeeded = 1;
                        RenderRow(hCodObject, pw, row, ts, 1) ;
                        needRedraw = 1;
                    }
                }
                if(needRedraw) BCCGFX_P_RedrawScreen( hCodObject->hCCGfxHandle );

            }
        }
    }
}
#endif /* FLASH_BY_RERENDER */

/*
Summary:
    Construct a UNI string in the buffer provided from the cstring.
*/
static unsigned int MapC1toUTF32(const unsigned char ch)
{

    static const BDCC_UTF32 C1_To_UTF32_Map[] = {
        0x266A, 0x0000, 0x0000, 0x215B, 0x215C, 0x215D, 0x2026, 0x215E,
        0x007C, 0x2510, 0x2514, 0x0160, 0x2500, 0x0152, 0x250C, 0xF101,
        0x2518, 0x007F, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x0000,
        0x0000, 0x0000, 0x2122, 0x0161, 0x0000, 0x0153, 0x2120, 0x0000,
        0x0178};

        if((ch > 0x7E) && (ch < 0xA0))
            return C1_To_UTF32_Map[ch - 0x7f] ;
        else
            return ch;
}


void RenderRow(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw, int Row, int ts, int fForce)
{
    int i, Incremental ;
    unsigned char ch ;
    int CharStart, CharEnd ;
    unsigned int FgOpacity = 0xFFFFFFFF ;
    unsigned int FgColor = 0xFFFFFFFF ;
    unsigned int BgOpacity = 0xFFFFFFFF ;
    unsigned int BgColor = 0xFFFFFFFF ;
    unsigned int PenSize = 0xFFFFFFFF ;
    unsigned int PenItalics = 0xFFFFFFFF ;
    unsigned int PenUnderline = 0xFFFFFFFF ;
    unsigned int PenEdgeType = 0xFFFFFFFF ;
    unsigned int PenEdgeColor = 0xFFFFFFFF ;
    unsigned int PenFontStyle = 0xFFFFFFFF ;
#if FLASH_BY_2SURFACES
    unsigned int ContainsFlash ;
#endif
    BDCC_UTF32 UTF32_ch;

    /* if rendering is not needed, bail out */
    if (! (fForce   ||    pw->WndBuf.aTextSeg[ts].fRenderNeeded) )
    {
        return ;
    }

    /* if nothing in the text buffer, bail out */
    if ( 0 == pw->WndBuf.aTextSeg[ts].iBeyondLastChar )
    {
        return ;
    }

    /* if nothing NEW in the text buffer, bail out */
    if ( pw->WndBuf.aTextSeg[ts].iBeyondLastChar <= pw->WndBuf.aTextSeg[ts].iLastBeyondLastChar
     &&  ! fForce )
    {
        return ;
    }


    if ( fForce )
        Incremental = 0 ;
    else
        Incremental = pw->WndBuf.aTextSeg[ts].iLastBeyondLastChar ;

#if FLASH_BY_2SURFACES
        /* check user override for flashing */
    if(
        ((hCodObject->OverrideMask & BDCC_ATTR_MASK_PENFGOP) && (hCodObject->Overrides.FgOpacity  == BDCC_Opacity_Flash)) ||
        ((hCodObject->OverrideMask & BDCC_ATTR_MASK_PENBGOP) && (hCodObject->Overrides.BgOpacity  == BDCC_Opacity_Flash)) ||
        ((hCodObject->OverrideMask & BDCC_ATTR_MASK_FILLOP)  && (hCodObject->Overrides.WinOpacity == BDCC_Opacity_Flash))
    )
        {
            ContainsFlash = true;
        }
        else
        {
            ContainsFlash = pw->WndBuf.aTextSeg[ts].fContainsFlash;
        }
#endif


#if FLASH_BY_RERENDER
    BCCGFX_P_RenderBegin(hCodObject->hCCGfxHandle, pw->WndId, Row, Incremental, FlashCycleState) ;
#elif FLASH_BY_2SURFACES
    BCCGFX_P_RenderBegin(hCodObject->hCCGfxHandle, pw->WndId, Row, Incremental, ContainsFlash) ;
#else
    BCCGFX_P_RenderBegin(hCodObject->hCCGfxHandle, pw->WndId, Row, Incremental) ;
#endif

    if ( fForce )
    {
        CharStart = 0 ;
        CharEnd = pw->WndBuf.aTextSeg[ts].iBeyondLastChar ;
    }
    else
    {
        CharStart = pw->WndBuf.aTextSeg[ts].iLastBeyondLastChar ;
        CharEnd = pw->WndBuf.aTextSeg[ts].iBeyondLastChar ;
    }
    BDBG_MSG(("RenderRow:  CharStart %d CharEnd %d Inc %d\n", CharStart, CharEnd, Incremental)) ;


    if (pw->WndBuf.aTextSeg[ts].fClrRowNeeded )
    {
        bool winFillColorOvr = (hCodObject->OverrideMask & BDCC_ATTR_MASK_FILL);
        bool winFillOpacityOvr = (hCodObject->OverrideMask & BDCC_ATTR_MASK_FILLOP);

        BDBG_MSG(( "fClrRowNeeded\n")) ;

        BCCGFX_P_SetWindowFill(hCodObject->hCCGfxHandle, pw->WndId,
            winFillOpacityOvr ? hCodObject->Overrides.WinOpacity : pw->WindowAttr.FillOpacity,
            winFillColorOvr ? hCodObject->Overrides.WinColor : pw->WindowAttr.FillColor) ;

        BCCGFX_P_FillRow(hCodObject->hCCGfxHandle, pw->WndId, /*pw->Pen.*/Row) ;
        pw->WndBuf.aTextSeg[ts].fClrRowNeeded = 0 ;
    }

    for ( i=CharStart ; i < CharEnd ; i++ )
    {
        int ByteOff = 0 ;
        int BitOff = 0 ;
        unsigned int arg, arg2, arg3, arg4, edgecolor ;

        ch = pw->WndBuf.aTextSeg[ts].aCharInfo[i].ch ;

        UTF32_ch = MapC1toUTF32(ch); /* unpack and convert to unicode since we must support extended characters */

        if ( UTF32_ch )
        {

            arg = BDCC_BITS_P_GetNextBits(pw->WndBuf.aTextSeg[ts].aCharInfo[i].PenColorArgs, &ByteOff, &BitOff, 2) ;
            arg = (hCodObject->OverrideMask & BDCC_ATTR_MASK_PENFGOP) ?  hCodObject->Overrides.FgOpacity : arg ;
            arg2 = BDCC_BITS_P_GetNextBits(pw->WndBuf.aTextSeg[ts].aCharInfo[i].PenColorArgs, &ByteOff, &BitOff, 6) ;
            arg2 = (hCodObject->OverrideMask & BDCC_ATTR_MASK_PENFG) ?  hCodObject->Overrides.FgColor : arg2 ;

            arg3 = BDCC_BITS_P_GetNextBits(pw->WndBuf.aTextSeg[ts].aCharInfo[i].PenColorArgs, &ByteOff, &BitOff, 2) ;
            arg3 = (hCodObject->OverrideMask & BDCC_ATTR_MASK_PENBGOP) ?  hCodObject->Overrides.BgOpacity : arg3 ;
            arg4 = BDCC_BITS_P_GetNextBits(pw->WndBuf.aTextSeg[ts].aCharInfo[i].PenColorArgs, &ByteOff, &BitOff, 6) ;
            arg4 = (hCodObject->OverrideMask & BDCC_ATTR_MASK_PENBG) ?  hCodObject->Overrides.BgColor : arg4 ;

            if ( arg != FgOpacity   ||   arg2 != FgColor || arg3 != BgOpacity   ||   arg4 != BgColor )
            {
                BCCGFX_P_SetPenBgColor(hCodObject->hCCGfxHandle, pw->WndId, BgOpacity=arg3, BgColor=arg4) ;
                BCCGFX_P_SetPenFgColor(hCodObject->hCCGfxHandle, pw->WndId, FgOpacity=arg, FgColor=arg2) ;
            }

            arg = BDCC_BITS_P_GetNextBits(pw->WndBuf.aTextSeg[ts].aCharInfo[i].PenColorArgs, &ByteOff, &BitOff, 2) ;
            edgecolor = BDCC_BITS_P_GetNextBits(pw->WndBuf.aTextSeg[ts].aCharInfo[i].PenColorArgs, &ByteOff, &BitOff, 6) ;
            edgecolor = (hCodObject->OverrideMask & BDCC_ATTR_MASK_EDGECOLOR) ?  hCodObject->Overrides.EdgeColor : edgecolor ;

            if ( edgecolor != PenEdgeColor )
            {
                BCCGFX_P_SetPenEdgeColor(hCodObject->hCCGfxHandle, pw->WndId, PenEdgeColor=edgecolor) ;
            }

            ByteOff = 0 ;
            BitOff = 6 ;

            arg = BDCC_BITS_P_GetNextBits(pw->WndBuf.aTextSeg[ts].aCharInfo[i].PenAttrArgs, &ByteOff, &BitOff, 2) ;
            arg = (hCodObject->OverrideMask & BDCC_ATTR_MASK_PENSIZE) ?  hCodObject->Overrides.PenSize : arg ;
            if ( arg != PenSize )
            {
                BCCGFX_P_SetPenSize(hCodObject->hCCGfxHandle, pw->WndId, PenSize=arg) ;
            }

            arg = BDCC_BITS_P_GetNextBits(pw->WndBuf.aTextSeg[ts].aCharInfo[i].PenAttrArgs, &ByteOff, &BitOff, 1) ;
            arg = (hCodObject->OverrideMask & BDCC_ATTR_MASK_ITALICS) ?  hCodObject->Overrides.PenStyle : arg ;
            if ( arg != PenItalics )
            {
                BCCGFX_P_SetPenItalics(hCodObject->hCCGfxHandle, pw->WndId, PenItalics=arg) ;
            }

            arg = BDCC_BITS_P_GetNextBits(pw->WndBuf.aTextSeg[ts].aCharInfo[i].PenAttrArgs, &ByteOff, &BitOff, 1) ;
            arg = (hCodObject->OverrideMask & BDCC_ATTR_MASK_UNDERLINE) ?  hCodObject->Overrides.PenStyle : arg ;
            if ( arg != PenUnderline )
            {
                BCCGFX_P_SetPenUnderline(hCodObject->hCCGfxHandle, pw->WndId, PenUnderline=arg) ;
            }

            arg = BDCC_BITS_P_GetNextBits(pw->WndBuf.aTextSeg[ts].aCharInfo[i].PenAttrArgs, &ByteOff, &BitOff, 3) ;
            arg = (hCodObject->OverrideMask & BDCC_ATTR_MASK_EDGETYPE) ?  hCodObject->Overrides.EdgeType : arg ;
            if ( arg != PenEdgeType )
            {
                BCCGFX_P_SetPenEdgeType(hCodObject->hCCGfxHandle, pw->WndId, PenEdgeType=arg ) ;
            }

            arg = BDCC_BITS_P_GetNextBits(pw->WndBuf.aTextSeg[ts].aCharInfo[i].PenAttrArgs, &ByteOff, &BitOff, 3) ;
            arg = (hCodObject->OverrideMask & BDCC_ATTR_MASK_FONTSTYLE) ?  hCodObject->Overrides.FontStyle : arg ;

            if ( arg != PenFontStyle )
            {
                BCCGFX_P_SetPenFontStyle(hCodObject->hCCGfxHandle, pw->WndId, PenFontStyle=arg) ;
            }

        }

        BCCGFX_P_RenderChar(hCodObject->hCCGfxHandle, UTF32_ch) ;

    }

    BCCGFX_P_RenderEnd(hCodObject->hCCGfxHandle, pw->rWindowAnchor, pw->cWindowAnchor, pw->WindowDef.AnchorPoint, hCodObject->uiColumns) ;

    pw->WndBuf.aTextSeg[ts].fRenderNeeded = 0 ;

    pw->WndBuf.aTextSeg[ts].iLastBeyondLastChar = pw->WndBuf.aTextSeg[ts].iBeyondLastChar ;

}



void RotateWndBuf(BDCC_INT_P_Window * pw, int RotateAmt, int OldRowCount)
{
    /* this can be optimized, but it is called very infrequently */
    int i, ts ;
    BDCC_INT_P_TextSegment SaveTS ;

    for ( i=0 ; i < RotateAmt ; i++ )
    {
        SaveTS = pw->WndBuf.aTextSeg[0] ;
        for ( ts=0 ; ts < OldRowCount-1 ; ts++ )
        {
            pw->WndBuf.aTextSeg[ts] = pw->WndBuf.aTextSeg[ts+1] ;
        }
        pw->WndBuf.aTextSeg[ts] = SaveTS ;
    }
}


void ResizeWndBuf(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw, int OldRowCount, int NewRowCount)
{
    int AddlRows = NewRowCount - OldRowCount ;
    int ts,row ;
    int InsertAfterTS ;
    int RotateAmt ;

    BSTD_UNUSED(hCodObject);

    if ( OldRowCount == 0 )
        return ;

    if ( AddlRows > 0 )
    {
        /* GROWING */
        BDBG_MSG(( "ResizeWndBuf GROWING wnd=%d rowinc=%d ScrollAdj %d\n", pw->WndId, AddlRows,pw->WndBuf.TsRowScrollAdj)) ;

        InsertAfterTS = (pw->WndBuf.TsRowScrollAdj + OldRowCount -1) % OldRowCount ;

        /* scoot down */
        for ( ts=(OldRowCount-1) ; ts > InsertAfterTS ; ts-- )
        {
            pw->WndBuf.aTextSeg[ts+AddlRows] = pw->WndBuf.aTextSeg[ts] ;
        }

        /* init new TSes */
        for ( ts = (InsertAfterTS+1) ; ts < (InsertAfterTS+1+AddlRows) ; ts++ )
        {
            ClearTS(pw, ts) ;
        }

        /* finish up */
        if ( pw->WndBuf.TsRowScrollAdj )
        {
            pw->WndBuf.TsRowScrollAdj += AddlRows ;
            pw->WndBuf.TsRowScrollAdj = pw->WndBuf.TsRowScrollAdj % NewRowCount ;
        }
    }
    else if ( AddlRows < 0 )
    {
        /* shrinking */
        BDBG_MSG(("ResizeWndBuf SHRINKING orc %d nrc %d  wnd=%d pw %p rowinc=%d ScrollAdj %d\n",
            OldRowCount, NewRowCount,
                  pw->WndId, (void*)pw,AddlRows,pw->WndBuf.TsRowScrollAdj)) ;

        /*
         * We have a choice here whether to remove rows from the top
         * or from the bottom.  Since neither the 708 spec nor the 708
         * implementation guidance doc (EIA/CEA-CEB-10) give any
         * guidance, we'll adopt the 608 method of erasing the
         * top rows.
         */

        /* rotate rows such that rows to drop are at end of array */
        RotateAmt = (pw->WndBuf.TsRowScrollAdj - AddlRows) % OldRowCount ;
        RotateWndBuf(pw, RotateAmt, OldRowCount) ;

        /* now drop rows at end of array by adjusting appropriate counters */
        pw->WndBuf.TsRowScrollAdj = 0 ;

        /* recycle the TS's */
        for ( row=0 ; row < -AddlRows ; row++ )
        {
            ts = row + NewRowCount ;
            ClearTS(pw, ts) ;
        }

        /* adjust pen.row if necessary */
        if ( pw->Pen.Row >= NewRowCount )
        {
            BDBG_MSG(( "ResizeWndBuf adjusting Pen.Row, old %d, new %d\n", pw->Pen.Row, NewRowCount-1)) ;
            pw->Pen.Row = NewRowCount - 1 ;
        }

    }
}
