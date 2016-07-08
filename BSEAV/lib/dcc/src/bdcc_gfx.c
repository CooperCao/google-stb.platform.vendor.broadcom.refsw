/***************************************************************************
 *     (c)2002-2013 Broadcom Corporation
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
 ***************************************************************************/



/****************************************************************************
 *                    T h e o r y  O f  O p e r a t i o n
 *
 * This section describes how this 7030 MSTV implementation of the
 * CCGFX Interface was implemented.
 *
 * This implementation is:
 *    DTVCC 708 Aware (necessarily from the CCGFX interface)
 *    Depends on bwin
 *
 * Surface Row Pool
 *
 * This implementation manages a pool of 1 row surfaces.  When a window
 * is created, surfaces are gotten from the pool for each of the window's
 * rows.  Likewise, when the window is destroyed, the surface rows are 
 * returned to the pool.  The calling layer has the choice of creating windows
 * in response to 708 Define Window commands (ie., whether or not they're
 * visible) or creating windows only when they are to be made visible.
 * TODO:  Delay assigning surfaces to window rows until they're rendered
 * into.
 * 
 * Fonts and Font Rendering
 * 
 * The intention is that the fonts and the font rendering algorithms are
 * customizable for different customers/systems.  Many of the 708 features
 * can be implemented in either of two ways:  as a font or as extra rendering.
 * For example, italics can be handled as a separate font or as additional
 * processing when the glyph is rendered into the surface.
 * 
 * Justification
 * 
 * This implementation supports left, right and center justification, but not
 * full justification.  This is accomplished by rendering characters one at a
 * time into a temporary row surface, keeping track of the accumulated width.
 * At the end of a text segment rendering, the text portion of the temporary
 * row surface is copied (blt'ed) to the appropriate, on-screen row surface
 * according to the justification mode.
 * 
 * Scrolling
 * 
 * Scrolling in the bottom-to-top direction is supported in this implementation.
 * This is made relatively easy by the choice of using row surfaces for the 
 * windows.  When a window is created, row surfaces are allocated from the pool
 * and pointers to these surfaces are stored in an array for the given window.
 * Scrolling is accomplished by 'slipping' the relationship between the array
 * index and its row position within the window.  This is done with the window
 * variable IndxOfTopRow.
 * 
 * Smooth Scrolling
 * 
 * The scrolling described above produces a jerky scroll.  That is, at the time
 * of a 1-row scroll, the window will scroll an entire text row at once, which
 * is normally 25 lines.  Smooth scrolling is the effect where the scroll appears
 * smooth.  This is done by sequencing the scroll over several vertical synch
 * periods, scrolling by 2 lines at a time.
 * 
 * This implementation uses the 7030's ability to position a surface anywhere on
 * the screen and the porting interface's notion of View Rectangles.
 * 
 * Here's how it works.  When an 'n' row window is scrolling, you actually have
 * 'n-1' complete rows and 2 partial rows.  For the complete rows, it's the 
 * surface position that gets changed throughout the scroll.  For the partial
 * rows, we manipulate the height and position of the view rectangle within
 * the row surface.  The treatment of the partial rows is done in two different
 * ways, controlled by a compile-time build define:  EXTRA_ROW_SCROLL.
 * 
 * If EXTRA_ROW_SCROLL is 1 at compile time, then the n-row window is actually
 * assigned 'n+1' surface rows.  The extra row allows each of the two partial
 * rows to have their own surface.  This produces the nicest looking scroll.
 * 
 * If EXTRA_ROW_SCROLL is 0, then only the bottom (incoming) row gets a surface.
 * The top row (outgoing) appears to be erased immediately upon the start of
 * scrolling.
 * 
 * Flashing
 * 
 * Flashing pen foregrounds and backgrounds are supported; flashing window fills
 * are not supported.  Flashing is implemented in two different ways that are
 * controlled by two compile-time build defines:  FLASH_BY_RERENDER and
 * FLASH_BY_2SURFACES.
 * 
 * If FLASH_BY_RERENDER is 1, then the flashing is accomplished by re-rendering
 * the rows in which there are flashing foregrounds or backgrounds.  Rows that
 * don't have any flashing are not re-rendered.  This is done through a joint
 * effort of the bcmDccIntGfx.c file and this file.   bcmDccIntGfx.c keeps
 * track of which rows have flashing characters and keeps track of the timing.
 * Its RenderRow function calls this module's ccgfxRenderBegin, passing in a
 * boolean which defines which 'phase' of the flash is to be rendered.
 * 
 * If FLASH_BY_2SURFACES is 1, then flashing is accomplished by using 2 row
 * surfaces for every window row instead of 1, and then switching between the
 * two.  To not unnecessarily consume row surfaces, only those rows that actually
 * have flashing characters will be assigned two surfaces.
 * 
 ****************************************************************************/


#include "b_api_shim.h"
#include "bdcc_gfx.h"
#include "bdcc_priv.h"
#include "bcc_winlib.h"

BDBG_MODULE(BDCCGFX);

#define msgEntry					0
#define msgInfo						0
#define msgError					1
#define EXTRA_ROW_SCROLL			1

/*
** The number of lines each surface is moved per phase of the scroll 
** operation is equivalent to the largest character size in the top 
** row divided by BCCGFX_P_MAX_SCROLL_PHASE. I.E. if the top row is 
** 40 pixels high and the BCCGFX_P_MAX_SCROLL_PHASE is 4, the scroll
** operation is divided into 4 phases each requiring the surface to move up by 10 pixels
*/
#define BCCGFX_P_MAX_SCROLL_PHASE		8

/*
** The minimum number of milliseconds to wait before redrawing
** the screen, at least for scrolling.  This number was generated empirically
** from the "rollup" tests.  It may need to change dynamically based on the 
** display resolution and other factors.
*/
#define BCCGFX_P_MIN_MSECS_BETWEEN_SCROLLS 30

/*
** The EIA standards specify that the flash rate should be
** - not slower than 1 flash per second 
** - not faster than 2 flashes per second
** We'll pick a number somewhere in between.
*/
#define BCCGFX_MIN_MSECS_BETWEEN_FLASHES  500


/*
** The number of surfaces to allocate when the library is initialized.
** Since new surfaces are allocated on demand, this number probaby isn't
** all the important.
** We'll pick "1" (an arbitrary choice) since we know that we'll
** want a scratch rendering surface.
** "0" works fine as well.
*/

#ifdef FCC_MIN_DECODER
/* 4 visible + 4 visible flash + 4 hidden + 4 hidden flash + 8 scroll */
#define BCCGFX_P_INITIAL_SURFACE_COUNT    24
#else
#define BCCGFX_P_INITIAL_SURFACE_COUNT    1
#endif


#define CCGFX_DBG_SURFACES_PER_POOL	0
#define CCGFX_P_CHARS_PER_ROW			32
#define CCGFX_MAX_ROWS_PER_WND			15
#define CCGFX_NUM_WNDS					8
#define CCGFX_DEFAULT_ALPHA_LEVEL		0x50

#define CCGFX_ULINE_HEIGHT 1  /* the thickness of the line used to "underline" characters */

typedef struct _BCCGFX_P_SurfaceInfo_ BCCGFX_P_SurfaceInfo ;

struct _BCCGFX_P_SurfaceInfo_
{
	int					        fUsed ;
	BDCC_WINLIB_hRow	 hSurface ;
	BCCGFX_P_SurfaceInfo *      pNextSurfInfo;
};


typedef struct
{
	int         NumSurfaces ;       /* number of surfaces that have been allocated  */
	int         FreeCount ;            /* number allocated that are not being used               */

	BCCGFX_P_SurfaceInfo * pSurfInfoList;        /* pointer to a list of surface descriptors */

} BCCGFX_P_SurfaceRowPool ;


typedef struct
{
	BDCC_WINLIB_hRow	 win ;
#if FLASH_BY_2SURFACES
	BDCC_WINLIB_hRow		pFlashSurface ; /* NULL means no flashing on this row */
#endif
	int				xSurface ;
	int				ySurface ;
	BDCC_PenSize    LargestPenSize ; /* we dynamically size the row according to the largest pen size in the row */
	int				EmptyRow; /* instruction to ignore LargestPenSize because the row is empty */
} BCCGFX_P_RowInfo ;

typedef struct
{
	int					fCreated ;
	int					ShowState ;
	int					FirstRowRow ;
	int					Column ;
	int					RowCount ;
	int					ActualRowCount ; /* considering 1 extra for scrolling, if EXTRA_ROW_SCROLL */
	int					ColCount ;
	int					IndxOfTopRow ;
	uint32_t			ScrollPhase ;
	BCCGFX_P_RowInfo		RowInfo[CCGFX_MAX_ROWS_PER_WND+1] ;

	BDCC_Opacity		FillOpacity ;
	unsigned int		FillColor ;

	BDCC_Opacity		PenFgOpacity ;
	unsigned int		PenFgColor ;
	
	unsigned int		PenEdgeColor;
	BDCC_Edge           EdgeType;

	BDCC_Opacity		PenBgOpacity ;
	unsigned int		PenBgColor ;

	int					Italics ;
	int					Underline ;
	BCCGFX_P_Justify	Justification ;
	BCCGFX_P_Direction	PrintDirection ;

	int					ZOrder ;
	BDCC_PenSize	PenSize ;
	BDCC_FontStyle	FontStyle ;

	int					iWinWidth ;
	bool                            bNeedToRedraw;
} BCCGFX_P_WND_INFO ;


typedef struct BCCGFX_P_GfxObject
{
	BDCC_WINLIB_Handle	hWinLibHandle;
	unsigned int			AlphaLevel ;
	uint32_t				Columns ;
	uint32_t				ScaleFactor; /* in hundredths */
	int						iSafeTitleX; /* how much the CC window needs to be indented in the X direction */
	int						iSafeTitleY; /* how much the CC window needs to be indented in the Y direction */

	int						iFrameBufferWidth; /* width of the frame buffer, used to clip windows */
	int						iFrameBufferHeight; /* height of the frame buffer, used to clip windows */
	
	BCCGFX_P_SurfaceRowPool	SurfacePool ;
	int						Render_WndId ;
	int						Render_Row ;
	int						RenderedExtent ;
	BCCGFX_P_WND_INFO		WndInfo[CCGFX_NUM_WNDS] ;

	BDCC_WINLIB_hRow	pCurWin ;
#if FLASH_BY_2SURFACES
	BDCC_WINLIB_hRow	pCurFlashWin ;
	int						RenderWithFlash ;
#endif

	int						FlashCycleState ;

	/* Time related information */
	/*unsigned int        msAccumulated ;*/
	bool					bTimerStarted ;
	unsigned int			msTimerStart ;
	unsigned int			msTimerDelay ;
	unsigned int			msCurrentTime; /* running time in milliseconds */
	unsigned int			msLastScreenRefresh; /* time stamp of last screen refresh */
	unsigned int			msTimeLastScroll;
	unsigned int			msTimeLastFlash;
	
	BDCC_WINLIB_Interface WinLibInterface;
	
} BCCGFX_P_GfxObject;


static int BCCGFX_P_SurfaceInit(
	BCCGFX_P_GfxHandle hCCGfxHandle
	);
static void BCCGFX_P_SurfaceClose(
	BCCGFX_P_GfxHandle hCCGfxHandle
	);
static int BCCGFX_P_SurfaceAlloc(
	BCCGFX_P_GfxHandle hCCGfxHandle,
	BCCGFX_P_SurfaceInfo ** pSurfInfo
	);
static BDCC_WINLIB_hRow BCCGFX_P_SurfaceReserve(
	BCCGFX_P_GfxHandle hCCGfxHandle
	);
static void BCCGFX_P_SurfaceRelease(
	BCCGFX_P_GfxHandle hCCGfxHandle,
	BDCC_WINLIB_hRow hSurface
	);

static void BCCGFX_P_SurfaceReleaseAll(
    BCCGFX_P_GfxHandle hCCGfxHandle
    );

static void PickFont(
    BCCGFX_P_GfxHandle	hCCGfxHandle,
	int WndId
	);
static void FillSurface(
	BCCGFX_P_GfxHandle hCCGfxHandle,
	BDCC_WINLIB_hRow pSurface,
	BDCC_Opacity opacity,
	uint8_t FillColor
	);

static void WndPos(
	BCCGFX_P_GfxHandle hCCGfxHandle,
	int WndId,
	int FirstRow,
	int Col,
	int LineAdj
	);
static void FillIRow(
	BCCGFX_P_GfxHandle hCCGfxHandle,
	BCCGFX_P_WND_INFO * pWI,
	int irow
	);
static void RotateRows(
	BCCGFX_P_WND_INFO * pWI,
	int RotateAmt
	) ;
static int TranslateAnchorRCtoUpperLeftXY(
	BCCGFX_P_GfxHandle hCCGfxHandle,
	BCCGFX_P_WND_INFO * pWI,
	int rAnchor, int cAnchor,
	int * pxWindow,
	int * pyWindow,
	BCCGFX_P_AnchorPoint AnchorPt,
	int nColumns
	);

#if CCGFX_DBG_SURFACES_PER_POOL
static void dbg_Init(
	BCCGFX_P_GfxHandle hCCGfxHandle
	);
static void dbg_Fini(
	BCCGFX_P_GfxHandle hCCGfxHandle
	);
static void dbg_Update(
	BCCGFX_P_GfxHandle hCCGfxHandle
	);
static void dbg_Str(
	BCCGFX_P_GfxHandle hCCGfxHandle,
	char * str
	);
#endif


/****************************************************************
 *
 * Function:		BCCGFX_P_Open
 *
 * Input:		
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function initializes all structures and resources
 * needed by this CCGFX library.
 *
 *****************************************************************/
int BCCGFX_P_Open(
    BCCGFX_P_GfxHandle *phCCGfxHandle, 
    BDCC_WINLIB_Handle hWinLibHandle,
    BDCC_WINLIB_Interface *pWinLibInterface
    )
{
	BCCGFX_P_GfxHandle hCCGfxHandle;
    int iError = BDCC_Error_eNoMemory;

    BDBG_ASSERT((hWinLibHandle));
    BDBG_ASSERT((pWinLibInterface->CreateCaptionRow));
    BDBG_ASSERT((pWinLibInterface->DestroyCaptionRow));
    BDBG_ASSERT((pWinLibInterface->ClearCaptionRow));
    BDBG_ASSERT((pWinLibInterface->SetCharFGColor));
    BDBG_ASSERT((pWinLibInterface->SetCharBGColor));
    BDBG_ASSERT((pWinLibInterface->SetCharEdgeColor));
    BDBG_ASSERT((pWinLibInterface->SetCharEdgeType));
    BDBG_ASSERT((pWinLibInterface->SetFont));
    BDBG_ASSERT((pWinLibInterface->SetCaptionRowZorder));
    BDBG_ASSERT((pWinLibInterface->SetCaptionRowClipRect));
    BDBG_ASSERT((pWinLibInterface->SetCaptionRowDispRect));
    BDBG_ASSERT((pWinLibInterface->GetCaptionRowTextRect));
    BDBG_ASSERT((pWinLibInterface->GetDisplayRect));
    BDBG_ASSERT((pWinLibInterface->GetSurfaceRect));
    BDBG_ASSERT((pWinLibInterface->GetMaxBoundingRect));
    BDBG_ASSERT((pWinLibInterface->GetFrameBufferSize));
    BDBG_ASSERT((pWinLibInterface->UpdateScreen));
    BDBG_ASSERT((pWinLibInterface->SetClipState));
    BDBG_ASSERT((pWinLibInterface->SetCaptionRowVisibility));
    BDBG_ASSERT((pWinLibInterface->IsCaptionRowVisible));
    BDBG_ASSERT((pWinLibInterface->RenderStart));
    BDBG_ASSERT((pWinLibInterface->RenderChar));
    BDBG_ASSERT((pWinLibInterface->RenderEnd));

	hCCGfxHandle = *phCCGfxHandle = (BCCGFX_P_GfxHandle)BKNI_Malloc(sizeof(BCCGFX_P_GfxObject));

	if ( hCCGfxHandle )
	{
        BKNI_Memset(hCCGfxHandle, 0, sizeof(BCCGFX_P_GfxObject)); /* zero out structure */
	    hCCGfxHandle->hWinLibHandle = hWinLibHandle ;
		hCCGfxHandle->WinLibInterface = *pWinLibInterface ;
        iError = BDCC_Error_eSuccess;
	}

    return iError;
	}


int BCCGFX_P_Init(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
    B_Dcc_Settings * pEngineSettings 
    )
{
    BDBG_ASSERT((hCCGfxHandle));

	hCCGfxHandle->AlphaLevel = CCGFX_DEFAULT_ALPHA_LEVEL ;
	hCCGfxHandle->iSafeTitleX = pEngineSettings->iSafeTitleX;
	hCCGfxHandle->iSafeTitleY = pEngineSettings->iSafeTitleY;
	hCCGfxHandle->ScaleFactor = pEngineSettings->ScaleFactor;

	/* 
    ** We need to ensure that the surfaces we are about to allocate are not wide (or taller)
    ** than the frame buffer.
    ** We'll also use this information for clipping windows to the edge of the frame buffer.
    */
    hCCGfxHandle->WinLibInterface.GetFrameBufferSize
        ( hCCGfxHandle->hWinLibHandle,
        (uint32_t *) &hCCGfxHandle->iFrameBufferWidth,
        (uint32_t *) &hCCGfxHandle->iFrameBufferHeight);

    /* 
	 * init the Windowing engine
	 */
	BCCGFX_P_SurfaceInit( hCCGfxHandle );

	/*
	 * sometimes its useful to debug with debug surface
	 */
#if CCGFX_DBG_SURFACES_PER_POOL
	dbg_Init() ;
#endif

	return BDCC_Error_eSuccess ;

}

/****************************************************************
 *
 * Function:		BCCGFX_P_Reset
 *
 * Input:		
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function reset the BCCGFX module previously opened in
 * BCCGFX_P_Open().
 *
 *****************************************************************/
void BCCGFX_P_Reset(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    B_Dcc_Settings *pEngineSettings
    )
{
#ifdef FCC_MIN_DECODER
    /* for FCC avoid fragmentation */
    BCCGFX_P_SurfaceReleaseAll(hCCGfxHandle);
#else
    /* First free up the surfaces. */
    BCCGFX_P_SurfaceClose( hCCGfxHandle );
#endif

    /* Then call the init routine. */
    BCCGFX_P_Init( hCCGfxHandle, pEngineSettings );
}

/****************************************************************
 *
 * Function:		BCCGFX_P_Close
 *
 * Input:		
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function frees all resources previously allocated in
 * ccgfxInit().
 *
 *****************************************************************/
void BCCGFX_P_Close(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{
    BDBG_ASSERT((hCCGfxHandle));

#if CCGFX_DBG_SURFACES_PER_POOL
	/*
	 * debug surface
	 */
	dbg_Fini() ;
#endif

	/*
	 * free up the surfaces, 
	 */
	BCCGFX_P_SurfaceClose(hCCGfxHandle) ;

	BKNI_Free(hCCGfxHandle) ;
}


/****************************************************************
 *
 * Function:		BCCGFX_P_WndCreate
 *
 * Input:			WndId			- window identifier
 *					RowCount		- number of rows
 *					ColCount		- number of columns
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function creates a window with the given row and
 * column counts.
 *
 *****************************************************************/
void BCCGFX_P_WndCreate(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId,
    int RowCount,
    int ColCount
    )
{
    int row;
    BCCGFX_P_WND_INFO * pWI ;

    BDBG_MSG(( "ccgfxWndCreate wnd=%d rc=%d cc=%d\n", WndId, RowCount, ColCount)) ;

    BDBG_ASSERT((hCCGfxHandle));

    if ( WndId >= CCGFX_NUM_WNDS  ||   RowCount > CCGFX_MAX_ROWS_PER_WND )
    {
    BDBG_ERR(( "ccgfxWndCreate:  error WndId or RowCount too big\n")) ;
    return ;
    }

    pWI = &hCCGfxHandle->WndInfo[WndId] ;

    if ( pWI->fCreated )
    {
        BDBG_WRN(( "ccgfxWndCreate: ERROR wnd %d already created\n", WndId)) ;
    }
    else
    {
#if EXTRA_ROW_SCROLL
        pWI->ActualRowCount = RowCount + 1 ;
#else
        pWI->ActualRowCount = RowCount ;
#endif

        pWI->ColCount = ColCount ;

        for ( row=0 ; row < pWI->ActualRowCount ; row++ )
        {
            pWI->RowInfo[row].win = BCCGFX_P_SurfaceReserve(hCCGfxHandle) ;
			pWI->RowInfo[row].LargestPenSize = BDCC_PenSize_Small;
			pWI->RowInfo[row].EmptyRow = true;
            BDBG_MSG(( "ccgfxWndCreate, just alloced surface %p\n", (void*)pWI->RowInfo[row].win)) ;
            hCCGfxHandle->WinLibInterface.SetCaptionRowZorder(pWI->RowInfo[row].win, 14) ; /* set later by Priority */
        }

        pWI->fCreated = 1 ;
        pWI->FirstRowRow = 0 ; /* later set by Position */
        pWI->RowCount = RowCount ;

    }

#if CCGFX_DBG_SURFACES_PER_POOL
    dbg_Update(hCCGfxHandle) ;
#endif

}


/****************************************************************
 *
 * Function:		BCCGFX_P_WndDestroy
 *
 * Input:			WndId			- window identifier
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function destroys a window.  If its not already hidden,
 * it hides the window first.
 *
 *****************************************************************/
void BCCGFX_P_WndDestroy(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId
    )
{
	int irow ;
	BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;

	BDBG_MSG(( "ccgfxWndDestroy wnd=%d\n", WndId)) ;
	
	if ( ! pWI->fCreated )
		return ;

	for ( irow=0 ; irow < pWI->ActualRowCount ; irow++ )
	{
		if ( pWI->RowInfo[irow].win )
		{
			hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pWI->RowInfo[irow].win, false) ;
			BCCGFX_P_SurfaceRelease(hCCGfxHandle, pWI->RowInfo[irow].win) ;
		}
#if FLASH_BY_2SURFACES
		if ( pWI->RowInfo[irow].pFlashSurface )
		{
			hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pWI->RowInfo[irow].pFlashSurface, false) ;
			BCCGFX_P_SurfaceRelease(hCCGfxHandle, pWI->RowInfo[irow].pFlashSurface) ;
		}
#endif
	}

	BCCGFX_P_RedrawScreen( hCCGfxHandle );

	BKNI_Memset(pWI, 0, sizeof(*pWI)) ;
    
#if CCGFX_DBG_SURFACES_PER_POOL
	dbg_Update(hCCGfxHandle) ;
#endif	
}


/****************************************************************
 *
 * Function:		BCCGFX_P_WndResize
 *
 * Input:			WndId			- window identifier
 *					RowCount		- number of rows
 *					ColCount		- number of columns
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function resizes a window with the given row and
 * column counts.
 *
 *****************************************************************/
void BCCGFX_P_WndResize(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId,
    int RowCount,
    int ColCount
    )
{
	int row, irow;
	int InsertAfterIRow ;
	int RotateAmt ;
	BCCGFX_P_WND_INFO *pWI ;
	bool bRefreshScreen = false;
	
    BDBG_ASSERT((hCCGfxHandle));

	BDBG_MSG(( "ccgfxWndResize wnd=%d rc=%d cc=%d\n", WndId, RowCount, ColCount)) ;

	if ( WndId >= CCGFX_NUM_WNDS  ||   RowCount > CCGFX_MAX_ROWS_PER_WND )
	{
		BDBG_ERR(( "ccgfxWndResize:  error WndId or RowCount too big\n")) ;
		return ;
	}

	pWI = &hCCGfxHandle->WndInfo[WndId] ;
	if ( !pWI->fCreated )
	{
		BDBG_ERR(( "ccgfxWndResize: ERROR wnd %d not created\n", WndId)) ;
	}
	else
	{
		/* are we growing rowcount or shrinking? */
		int AddlRows = RowCount - pWI->RowCount ;

		if ( AddlRows > 0 )
		{
			/* GROWING */
			BDBG_MSG(( "ccgfxWndResize GROWING wnd=%d rowinc=%d IndxOfTopRow %d\n", WndId, AddlRows,pWI->IndxOfTopRow)) ;

			InsertAfterIRow = (pWI->IndxOfTopRow + pWI->ActualRowCount -1) % pWI->ActualRowCount ;

			/* scoot down */
			for ( irow=(pWI->ActualRowCount-1) ; irow > InsertAfterIRow ; irow-- )
			{
				pWI->RowInfo[irow+AddlRows] = pWI->RowInfo[irow] ; 
			}

			/* add new surfaces */
			for ( irow = (InsertAfterIRow+1) ; irow < (InsertAfterIRow+1+AddlRows) ; irow++ )
			{
				pWI->RowInfo[irow].win = BCCGFX_P_SurfaceReserve(hCCGfxHandle) ;
				BDBG_MSG(( "ccgfxWndCreate, just alloced surface %p, irow is %d\n", (void*)pWI->RowInfo[irow].win,irow)) ;
				hCCGfxHandle->WinLibInterface.SetCaptionRowZorder(pWI->RowInfo[irow].win, pWI->ZOrder) ;
				FillIRow(hCCGfxHandle, pWI, irow) ;
				if ( pWI->ShowState )
					hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pWI->RowInfo[irow].win, true) ;
				else
					hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pWI->RowInfo[irow].win, false) ;
			}

			/* finish up */
			pWI->ActualRowCount += AddlRows ;
			pWI->RowCount += AddlRows ;
			if ( pWI->IndxOfTopRow )
			{
				pWI->IndxOfTopRow += AddlRows ;
				pWI->IndxOfTopRow = pWI->IndxOfTopRow % pWI->ActualRowCount ; 
			}

			bRefreshScreen = true;
			
		}
		else if ( AddlRows < 0 )
		{
			/* shrinking */

			/*
			 * We have a choice here whether to remove rows from the top
			 * or from the bottom.  Since neither the 708 spec nor the 708 
			 * implementation guidance doc (EIA/CEA-CEB-10) give any 
			 * guidance, we'll adopt the 608 method of erasing the
			 * top rows.
			 */

			/* rotate rows such that rows to drop are at end of array */
			RotateAmt = (pWI->IndxOfTopRow - AddlRows) % pWI->ActualRowCount ;
			RotateRows(pWI, RotateAmt) ;

			/* now drop rows at end of array by adjusting appropriate counters */
			pWI->IndxOfTopRow = 0 ;
			pWI->ActualRowCount += AddlRows ; /* remember this is negative */
			pWI->RowCount += AddlRows ; /* remember this is negative */

			/* recycle the row surfaces back into the pool */
			for ( row=0 ; row < -AddlRows ; row++ )
			{
				irow = row + pWI->ActualRowCount ;
				if ( pWI->RowInfo[irow].win )
				{
					hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pWI->RowInfo[irow].win, false) ;
					BCCGFX_P_SurfaceRelease(hCCGfxHandle, pWI->RowInfo[irow].win) ;
				}
#if FLASH_BY_2SURFACES
				if ( pWI->RowInfo[irow].pFlashSurface )
				{
					hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pWI->RowInfo[irow].pFlashSurface, false) ;
					BCCGFX_P_SurfaceRelease(hCCGfxHandle, pWI->RowInfo[irow].pFlashSurface) ;
				}
#endif
			}
			
			bRefreshScreen = true;
			
		}
		else
		{
			BDBG_MSG(( "Resize, RowCount not changing\n")) ;
		}

		pWI->ColCount = ColCount ;

	}

	/*
	** TODO: should the screen be redrawn at this point?
	*/

	/*if ( bRefreshScreen )*/
	if ( 0 )
	{
	    BCCGFX_P_SetDirtyState( hCCGfxHandle, WndId,  false );
	    BCCGFX_P_RedrawScreen( hCCGfxHandle );
	}

}

/****************************************************************
 *
 * Function:		BCCGFX_P_WndPosition
 *
 * Input:			WndId			- window identifier
 *					rAnchor			- screen row of the anchor
 *					cAnchor			- screen column of the anchor
 *					AnchorPt		- anchor point
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function moves the given window according to
 * the given screen coordinates.  The display/hide state is not
 * affected by this call.
 *
 *****************************************************************/
void BCCGFX_P_WndPosition(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId,
    int rAnchor,
    int cAnchor,
    BCCGFX_P_AnchorPoint AnchorPt,
    int nColumns
    )
{
	BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;
	int xWindow, yWindow ;

	BDBG_MSG(("ccgfxWndPosition wnd=%d row=%d col=%d\n", WndId, rAnchor, cAnchor)) ;

	/* width is based on the largest character size in the caption window) */
	pWI->iWinWidth = TranslateAnchorRCtoUpperLeftXY(hCCGfxHandle, pWI, rAnchor, cAnchor, &xWindow, &yWindow, AnchorPt, nColumns) ;

	WndPos(hCCGfxHandle, WndId, yWindow, xWindow, 0) ;
} /* BCCGFX_P_WndPosition */

					 
/****************************************************************
 *
 * Function:		BCCGFX_P_WndShow
 *
 * Input:			WndId			- window identifier
 *					ShowState		- 0 for hide; 1 for display
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * ccgfxWndShow() changes the given window's display/hide 
 * state according to the given ShowState.
 *
 *****************************************************************/
void BCCGFX_P_WndShow(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId,
    int ShowState
    )
{
	BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;
	BDCC_WINLIB_hRow pSurface ;
	int row ;
	int rcerr ;
	bool bRefreshScreen = false;
	
	BDBG_MSG(("ccgfxWndShow wnd=%d state=%d\n", WndId, ShowState)) ;

	/*
	 * we may need to do some adjustments of the
	 * show states of various windows based on
	 * the z orders, depending on how gracefully
	 * the hardware handles this, experiment first
	 * before implementing this
	 */

	for ( row=0 ; row < pWI->ActualRowCount ; row++ )
	{
		pSurface = pWI->RowInfo[row].win ;
		if ( pSurface )
		{
			if ( ShowState )
			{
				/* SHOW */
				BDBG_MSG(( "ccgfxWndShow, showing wnd %d row %d %p\n", WndId, row, (void*)pSurface)) ;
#if FLASH_BY_2SURFACES
				if ( pWI->RowInfo[row].pFlashSurface )
				{
					/* 2 surfaces, so now we chose based on FlashCycleState */
					if ( hCCGfxHandle->FlashCycleState )
					{
						hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pWI->RowInfo[row].pFlashSurface, false) ;
						rcerr = hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pSurface, true) ;
					}
					else
					{
						hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pSurface, false) ;
						rcerr = hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pWI->RowInfo[row].pFlashSurface, true) ;
					}
				}
				else
				{
					rcerr = hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pSurface, true) ;
				}
#else
				rcerr = hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pSurface, true) ;
#endif
			}
			else
			{
				/* HIDE */
				rcerr = hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pSurface, false) ;
#if FLASH_BY_2SURFACES
				if ( pWI->RowInfo[row].pFlashSurface )
				{
					hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pWI->RowInfo[row].pFlashSurface, false) ;
				}
#endif
			}
			if ( rcerr )
			{
				BDBG_ERR(( "ccgfxWndShow failed, err %d\n", rcerr)) ;
			}
		}
	}


	if ( pWI->ShowState != ShowState )
	{
	    bRefreshScreen = true;
	}

	pWI->ShowState = ShowState ;
	
	/*
	** If the state of the window has changed, redraw the screen.
	** TODO: are more "smarts" needed here to determine when to refresh
	** the screen?
	*/

	if ( bRefreshScreen )
	{
	    BCCGFX_P_SetDirtyState( hCCGfxHandle, WndId,  false );
	    BCCGFX_P_RedrawScreen( hCCGfxHandle );
	}
} /* BCCGFX_P_WndShow */

					 
/****************************************************************
 *
 * Function:		BCCGFX_P_WndClear
 *
 * Input:			WndId			- window identifier
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function clears the given window by filling it with 
 * the current fill color/opacity.
 *
 *****************************************************************/
void BCCGFX_P_WndClear(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId
    )
{
	BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;
	int row ;
	BCCGFX_P_SurfaceRowPool *pSurfPool = &hCCGfxHandle->SurfacePool ;

	BDBG_MSG(("ccgfxWndClear wnd=%d pSurfPool = %p, hCCGfxHandle = %p\n", WndId, (void*)pSurfPool, (void*)hCCGfxHandle)) ;
	BDBG_MSG(("ccgfxWndClear pWI->fCreated is %d\n", pWI->fCreated)) ;

	if ( ! pWI->fCreated )
		return ;

	for ( row=0 ; row < pWI->ActualRowCount ; row++ )
	{
		BCCGFX_P_FillRow(hCCGfxHandle, WndId, row) ;
	}
} /* BCCGFX_P_WndClear */


void BCCGFX_P_TimeUpdate(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    unsigned int *pCurrentTime
    )
{
    b_time_t currentTime;
    unsigned int    uiCurrentMilliSecs;

    b_time_get( &currentTime );

    uiCurrentMilliSecs = currentTime.tv_sec * 1000 ;
    uiCurrentMilliSecs += currentTime.tv_usec / 1000 ;

    hCCGfxHandle->msCurrentTime = uiCurrentMilliSecs;

    if ( pCurrentTime )
    {
        *pCurrentTime = uiCurrentMilliSecs;
    }

    return;
}

/*
** Simply sample the current time.
*/
void BCCGFX_P_TimeReset(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{

    BCCGFX_P_TimeUpdate( hCCGfxHandle, NULL  );

    return;
}

/*
** Return the "local" copy of the current time.
** The "copy" is set by calling "BCCGFX_P_TimeUpdate()".
*/
unsigned int BCCGFX_P_TimeGet(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{
    return hCCGfxHandle->msCurrentTime ;
}


void BCCGFX_P_TimerStart(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    unsigned int msDelayVal
    )
{
	BCCGFX_P_TimeUpdate( hCCGfxHandle, &(hCCGfxHandle->msTimerStart ) );
	
	hCCGfxHandle->msTimerDelay = msDelayVal;

	BDBG_MSG(("ccgfxTimer_Start:  start = 0x%08x  delay = 0x%08x\n", 
		hCCGfxHandle->msTimerStart, hCCGfxHandle->msTimerDelay)) ;
		
	hCCGfxHandle->bTimerStarted = 1 ;
}

void BCCGFX_P_TimerCancel(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{
	BDBG_MSG(( "ccgfxTimer_Cancel\n")) ;
	hCCGfxHandle->bTimerStarted = 0 ;
}

unsigned int BCCGFX_P_TimerQuery(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{
	unsigned int msDelaySoFar ;
	
	if ( hCCGfxHandle->bTimerStarted == 0 )
		return(1) ;

	BCCGFX_P_TimeUpdate( hCCGfxHandle, NULL );
	
	msDelaySoFar = hCCGfxHandle->msCurrentTime - hCCGfxHandle->msTimerStart ;

	if ( msDelaySoFar >= hCCGfxHandle->msTimerDelay )
	{
		BDBG_MSG(( "ccgfxTimer_Query: EXPIRED\n")) ;
		return(1) ;
	}
	
	return(0) ;
}

					 
/****************************************************************
 *
 * Function:		ccgfxScroll
 *
 * Input:			WndId			- window identifier
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function performs an n-row scroll of the given window
 * where n depends on the height of the row).
 * This includes clearing the exposed row.
 *
 *****************************************************************/
void BCCGFX_P_Scroll(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId
    )
{
	BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;
	
	BDBG_MSG(("ccgfxScroll wnd=%d\n", WndId)) ;

	if ( pWI->ScrollPhase )
	{
		/* 
		** If already scrolling, finish it up in a hurry
		** TODO: this is not a visually pleasing approach, it causes the text to jump by 
		** ( "height of a CC line of text" - NumLinesScrolled ).
		** We'll have to come up with a better solution if we get complaints.
		**
		** Note: the UserData ISR queue overflows it we just spin here waiting for the scroll to finish.
		*/
		pWI->ScrollPhase = 0 ;
		pWI->IndxOfTopRow++ ; 
		pWI->IndxOfTopRow = pWI->IndxOfTopRow % pWI->ActualRowCount ; 
#if EXTRA_ROW_SCROLL
		BCCGFX_P_FillRow(hCCGfxHandle, WndId, pWI->RowCount) ;
#endif
	}
	
#if EXTRA_ROW_SCROLL == 0
	BCCGFX_P_FillRow(hCCGfxHandle, WndId, 0) ;
#endif

	pWI->ScrollPhase = 1;   /* non-zero means scroll in progress */

	WndPos(hCCGfxHandle, WndId, pWI->FirstRowRow, pWI->Column, pWI->ScrollPhase) ;
}


int BCCGFX_P_GetScrollPhase(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId
    )
{
	BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;

	return pWI->ScrollPhase;
}

/****************************************************************
 *
 * Function:		ccgfxPeriodic
 *
 * Input:			<void>
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * BCCGFX_P_Periodic() allows this module to sequence scrolling
 * to achieve a smooth scrolling effect.  This function must
 * be called on a periodic basis, usually based on the vertical
 * sync
 *
 *****************************************************************/
void BCCGFX_P_Periodic(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{
	int wnd ;
	BCCGFX_P_WND_INFO * pWI ;
	unsigned int uiCurTime;

	/*
	** Redraw the screen for a flash operation.
	*/
	bool bFlashRedrawNeeded = false;

	/*
	** Redraw the screen for a scroll  operation.
	*/
	bool bScrollRedrawNeeded = false;

	/*
	** Sufficient time has elapsed, scroll the screen if appropriate. 
	*/
	bool bScrollTimeElapsed;

	uiCurTime = BCCGFX_P_TimeGet( hCCGfxHandle );

	bScrollTimeElapsed = ( BCCGFX_P_MIN_MSECS_BETWEEN_SCROLLS < ( uiCurTime - hCCGfxHandle->msTimeLastScroll ));

	/*
	 * handle the smooth scrolling
	 */
	for ( wnd=0 ; wnd < CCGFX_NUM_WNDS ; wnd++ )
	{
		pWI = &hCCGfxHandle->WndInfo[wnd] ;

		if ( pWI->ScrollPhase && bScrollTimeElapsed )
		{
			pWI->ScrollPhase ++;
			if ( pWI->ScrollPhase >= BCCGFX_P_MAX_SCROLL_PHASE )
			{
				/* done */
				pWI->ScrollPhase= 0 ;
				pWI->IndxOfTopRow++ ; 
				pWI->IndxOfTopRow = pWI->IndxOfTopRow % pWI->ActualRowCount ; 
#if EXTRA_ROW_SCROLL
				BCCGFX_P_FillRow(hCCGfxHandle, wnd, pWI->RowCount) ;
#endif
			}
			BDBG_MSG(( "BCCGFX_P_Periodic, scrolling wnd %d\n", wnd)) ;

			WndPos(hCCGfxHandle, wnd, pWI->FirstRowRow, pWI->Column, pWI->ScrollPhase) ;

			/*
			** Since a window has been scrolled, refresh the screen before exiting this routine.
			*/
			bScrollRedrawNeeded = true;

		}
	}

#if FLASH_BY_2SURFACES
	/*
	** Handle the flashing by toggling between the flash and non-flash surfaces.
	** (For any row that contains flashing characters).
	**
	** Per the EIA standards;
	** - not slower than 1 flash per second 
	** - not faster than 2 flashes per second 
	*/

	if ( BCCGFX_MIN_MSECS_BETWEEN_FLASHES < uiCurTime - hCCGfxHandle->msTimeLastFlash )
	{
		BDBG_MSG(( "ccgfxPeriodic qualified for flash\n")) ;

		hCCGfxHandle->FlashCycleState ^= 1 ;
		for ( wnd=0 ; wnd < CCGFX_NUM_WNDS ; wnd++ )
		{
			int row ;
			pWI = &hCCGfxHandle->WndInfo[wnd] ;

			/*
			** If the window is visible, flash rows as appropriate.
			*/

			if (  pWI->ShowState )
			{
			    for ( row=0 ; row < pWI->ActualRowCount ; row++ )
			    {
			        if ( pWI->RowInfo[row].pFlashSurface != NULL )
			        {
			            BDCC_WINLIB_hRow pOldSurface ;
			            BDCC_WINLIB_hRow pNewSurface ;

			            /*
			            ** okay, now we have a flashing row switch it
			            */
			            if ( hCCGfxHandle->FlashCycleState )
			            {
			                pNewSurface = pWI->RowInfo[row].win ;
			                pOldSurface = pWI->RowInfo[row].pFlashSurface ;
			            }
			            else
			            {
			                pNewSurface = pWI->RowInfo[row].pFlashSurface ;
			                pOldSurface = pWI->RowInfo[row].win ;
			            }
			            
			            hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pOldSurface, false) ;
			            hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pNewSurface, true) ;
			            BDBG_MSG(( "just flipped surfaces for wnd %d row %d\n", wnd, row)) ;

			        }

			        bFlashRedrawNeeded = true;

			    }
				 
			}   /* end of if (  pWI->ShowState ) */
		}
	}
	 
#endif

        /*
        ** An event has occured that requires that the screen be redrawn.
        */
	if ( bFlashRedrawNeeded || bScrollRedrawNeeded )
	{
	    BCCGFX_P_RedrawScreen( hCCGfxHandle );
	    
	    /*
	    ** Since the screen was refreshed, we don't need to remember that
	    ** this window has been modified.
	    */

	    /*
	    ** PR #39630: Window ID exceeding maximum, overwriting character address space.
	    ** TODO:  Find better way to implement.  Should this be pulled into above loop?    
	    */

	    if ( bScrollRedrawNeeded  )
	    {
	        hCCGfxHandle->msTimeLastScroll = uiCurTime;
	    }

	    if ( bFlashRedrawNeeded )
	    {
	        hCCGfxHandle->msTimeLastFlash = uiCurTime;
	    }
	    
	}
}

/****************************************************************
 *
 * Function:		BCCGFX_P_FillRow
 *
 * Input:			WndId			- window identifier
 *					row				- zero-based, wnd-based row
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function fills the given wnd row with the window fill color.
 *
 *****************************************************************/
void BCCGFX_P_FillRow(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId,
    int row
    )
{
	BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;
	int irow ;
	
	irow = (row + pWI->IndxOfTopRow + ((pWI->ScrollPhase)?1:0)) % pWI->ActualRowCount ;
	FillIRow(hCCGfxHandle, pWI, irow) ;
}

static void FillIRow(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    BCCGFX_P_WND_INFO * pWI,
    int irow
    )
{
#if FLASH_BY_RERENDER
    /* if the fill opacity is flash then it must be solid when FlashCycleState is 1 and transparent when 0 */
	BDCC_Opacity fillOpacity = ((pWI->FillOpacity == BDCC_Opacity_Flash) && 
	                                        (hCCGfxHandle->FlashCycleState == 0)) ?
                                    BDCC_Opacity_Transparent : pWI->FillOpacity;
#else
	BDCC_Opacity fillOpacity =  pWI->FillOpacity;
#endif
	
	BDCC_WINLIB_hRow pSurface = pWI->RowInfo[irow].win ;

	/* reset the largest rendered character size in the row to 'small' */
	pWI->RowInfo[irow].LargestPenSize = BDCC_PenSize_Small;
	pWI->RowInfo[irow].EmptyRow = true;


	FillSurface(hCCGfxHandle, pSurface, fillOpacity, pWI->FillColor) ;
#if FLASH_BY_2SURFACES
	if ( pWI->RowInfo[irow].pFlashSurface )
	{
	    /* if the fill opacity is 'flash' then pSurface is 'solid' and pFlashSurface is 'transparent' */
	    BDCC_Opacity flashOpacity = (pWI->FillOpacity == BDCC_Opacity_Flash) ? 
                                    BDCC_Opacity_Transparent : pWI->FillOpacity;
        
		FillSurface(hCCGfxHandle, pWI->RowInfo[irow].pFlashSurface, flashOpacity, pWI->FillColor) ;
	}
#endif
}


/****************************************************************
 *
 * Function:		BCCGFX_P_RenderBegin
 *
 * Input:			WndId			- window identifier
 *					Row				- zero-based, wnd-based row
 *					Incremental		- 1 for incremental, 0 for full
 *					RenderWithFlash - non-zero if at least one char in subsequent
 *									  ccgfxRenderChar calls has flash opacity
 *					FlashCycleState	- non-zero for 'flash' phase
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function allows the interface to prepare for subsequent
 * calls to ccgfxRenderChar.
 *
 *****************************************************************/
#if FLASH_BY_RERENDER
void BCCGFX_P_RenderBegin(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		int Row, 
		int Incremental, 
		int FlashCycleState)
#elif FLASH_BY_2SURFACES
void BCCGFX_P_RenderBegin(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		int Row, 
		int Incremental, 
		int RenderWithFlash)
#else
void BCCGFX_P_RenderBegin(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		int Row, 
		int Incremental)
#endif
{
	int irow ;
	BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;

	BDBG_MSG(("ccgfxRenderBegin wnd=%d row=%d inc=%d\n", 
		WndId, Row,Incremental)) ;
#if FLASH_BY_2SURFACES
	BDBG_MSG(("ccgfxRenderBegin RWF=%d\n", RenderWithFlash)) ;
#endif

	if ( ! pWI->fCreated )
	{
		BDBG_ERR(( "ccgfxRenderBegin:  Error wnd %d not created\n", WndId)) ;
		return ;
	}

	hCCGfxHandle->Render_WndId = WndId ;
	hCCGfxHandle->Render_Row = Row ;

	if (!Incremental)
	{ 
		/* Rendering a line from the beginning */
		irow = (hCCGfxHandle->Render_Row + pWI->IndxOfTopRow + ((pWI->ScrollPhase)?1:0)) % pWI->ActualRowCount ;
		hCCGfxHandle->pCurWin = pWI->RowInfo[irow].win;

#if FLASH_BY_2SURFACES
		hCCGfxHandle->pCurFlashWin = pWI->RowInfo[irow].pFlashSurface;

		if ( RenderWithFlash )
		{
			if ( hCCGfxHandle->pCurFlashWin )
			{
				BDBG_MSG(( "ccgfxRenderEnd:  already have flash surface???\n")) ;
			}
			else
			{
			    hCCGfxHandle->pCurFlashWin = pWI->RowInfo[irow].pFlashSurface = BCCGFX_P_SurfaceReserve(hCCGfxHandle) ;

			    if ( hCCGfxHandle->pCurFlashWin )
			    {
			    	/* if the fill opacity is 'flash' then pCurWin is 'solid' and pCurFlashWin is 'transparent' */
	                BDCC_Opacity opacity = (pWI->FillOpacity == BDCC_Opacity_Flash) ? 
                                    BDCC_Opacity_Transparent : pWI->FillOpacity;

				    hCCGfxHandle->WinLibInterface.SetCaptionRowZorder(hCCGfxHandle->pCurFlashWin, pWI->ZOrder) ;
			        FillSurface(hCCGfxHandle, hCCGfxHandle->pCurFlashWin, opacity, pWI->FillColor) ;			
			    }
			}
		}
		else
		{
			/*
			 * if we have a flash surface for this row
			 * put it back into the pool and hide it
			 */
			if ( hCCGfxHandle->pCurFlashWin )
			{

				hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(hCCGfxHandle->pCurFlashWin, false) ; 
				BCCGFX_P_SurfaceRelease(hCCGfxHandle, hCCGfxHandle->pCurFlashWin) ;
				hCCGfxHandle->pCurFlashWin = pWI->RowInfo[irow].pFlashSurface = NULL ;

				/* hidden the flash surface so make sure the non-flash is showing */
				hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(hCCGfxHandle->pCurWin, true) ;
			}
		}
#endif
#if FLASH_BY_RERENDER
		hCCGfxHandle->FlashCycleState = FlashCycleState ;
#elif FLASH_BY_2SURFACES
		hCCGfxHandle->RenderWithFlash = RenderWithFlash ;
#endif
		/* reset the temp row fill tracking variables */

		
		hCCGfxHandle->WinLibInterface.SetCharFGColor(hCCGfxHandle->pCurWin, pWI->PenFgColor, pWI->PenFgOpacity) ;
		hCCGfxHandle->WinLibInterface.SetCharBGColor(hCCGfxHandle->pCurWin, pWI->PenBgColor, pWI->PenBgOpacity) ;
		hCCGfxHandle->WinLibInterface.SetCharEdgeColor(hCCGfxHandle->pCurWin, pWI->PenEdgeColor, pWI->PenFgOpacity) ; /* foreground and edge opacities are identical */
	
		/* inform winlib that we are rendering from the beginning */
#if FLASH_BY_2SURFACES
		hCCGfxHandle->WinLibInterface.RenderStart( hCCGfxHandle->pCurWin, hCCGfxHandle->pCurFlashWin, pWI->Justification );
#else
		hCCGfxHandle->WinLibInterface.RenderStart( hCCGfxHandle->pCurWin, pWI->Justification );
#endif
		
#if FLASH_BY_2SURFACES
		if ( hCCGfxHandle->RenderWithFlash )
		{
			if ( hCCGfxHandle->pCurFlashWin )
			{
				hCCGfxHandle->WinLibInterface.SetCharFGColor(hCCGfxHandle->pCurFlashWin, pWI->PenFgColor, pWI->PenFgOpacity) ; 
				hCCGfxHandle->WinLibInterface.SetCharBGColor(hCCGfxHandle->pCurFlashWin, pWI->PenBgColor, pWI->PenBgOpacity) ;
				hCCGfxHandle->WinLibInterface.SetCharEdgeColor(hCCGfxHandle->pCurFlashWin, pWI->PenEdgeColor, pWI->PenFgOpacity) ;
			}
		}
#endif
	}

	PickFont(hCCGfxHandle, WndId) ;

	hCCGfxHandle->RenderedExtent = 0 ;	
} /* ccgfxRenderBegin */


/****************************************************************
 *
 * Function:		BCCGFX_P_RenderChar
 *
 * Input:			ch				- window identifier
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function renders the given character.  It may be rendered
 * directly to the screen or to a temporary surface which is later
 * copied to an on-screen surface during ccgfxRenderEnd().
 *
 *****************************************************************/
void BCCGFX_P_RenderChar(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    BDCC_UTF32 ch
    )
{
	int Extent ;
	int irow ;
	BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[hCCGfxHandle->Render_WndId] ;

	if ( ! pWI->fCreated )
		return ;

	if ( ch )
	{
		BDBG_MSG(("ccgfxRenderChar ch=0x%02x (%c)\n", ch, ch)) ;
	}

	if ( (ch == 0x80) || (ch == 0x81) )
	{
		/* TSP or NBTSP, ie Transparent Space */
		BDBG_MSG(( "ccgfxRenderChar Transparent Space\n")) ;
		ch = 0 ;
	}

	irow = (hCCGfxHandle->Render_Row + pWI->IndxOfTopRow + ((pWI->ScrollPhase)?1:0)) % pWI->ActualRowCount ;

	BDBG_MSG(( "ccgfxRenderChar:  calling DoRenderChar pCurWin %p\n", (void*)hCCGfxHandle->pCurWin)) ;

#if FLASH_BY_2SURFACES
	Extent = hCCGfxHandle->WinLibInterface.RenderChar( hCCGfxHandle->pCurWin, hCCGfxHandle->pCurFlashWin, ch );
#elif FLASH_BY_RERENDER
	Extent = hCCGfxHandle->WinLibInterface.RenderChar( hCCGfxHandle->pCurWin, hCCGfxHandle->FlashCycleState , ch );
#else
	Extent = hCCGfxHandle->WinLibInterface.RenderChar( hCCGfxHandle->pCurWin, ch );
#endif

	/* Keep track of the largest rendered character size in this row - it defines the row height*/
	if(pWI->PenSize > pWI->RowInfo[irow].LargestPenSize)
		pWI->RowInfo[irow].LargestPenSize = pWI->PenSize;
	
	pWI->RowInfo[irow].EmptyRow = false;
	hCCGfxHandle->RenderedExtent += Extent ;
	BDBG_MSG(( "ccgfxRenderChar: exit\n")) ;

} /* BCCGFX_P_RenderChar */


/****************************************************************
 *
 * Function:		BCCGFX_P_RenderEnd
 *
 * Input:		
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function announces the end of character rendering.  It 
 * allows the interface the opportunity to copy the rendering
 * to the appropriate location on screen.
 *
 *****************************************************************/
void BCCGFX_P_RenderEnd(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int rAnchor,
    int cAnchor,
    BCCGFX_P_AnchorPoint AnchorPt,
    int nColumns
    )
{
	BCCGFX_P_WND_INFO * pWI;
	int irow ;
	int xWindow, yWindow ;

	BDBG_MSG(( "ccgfxRenderEnd:  window id %d\n", hCCGfxHandle->Render_WndId)) ;

	if ( hCCGfxHandle->RenderedExtent == 0 ) /* PAXEL can't handle this degenerate case */
		return ;

	if ( hCCGfxHandle->Render_WndId == -1 )
	{
		BDBG_ERR(( "ccgfxRenderEnd:  Error bad window id %d\n", hCCGfxHandle->Render_WndId)) ;
		return ;
	}

	pWI = &hCCGfxHandle->WndInfo[hCCGfxHandle->Render_WndId] ;
	if ( ! pWI->fCreated )
	{
		BDBG_ERR(( "ccgfxRenderEnd:  Error window %d not created\n", hCCGfxHandle->Render_WndId)) ;
		return ;
	}

	BDBG_MSG(( "ccgfxRenderEnd:  window id %d row %d extent %d\n", hCCGfxHandle->Render_WndId, hCCGfxHandle->Render_Row,hCCGfxHandle->RenderedExtent)) ;

	irow = (hCCGfxHandle->Render_Row + pWI->IndxOfTopRow + ((pWI->ScrollPhase)?1:0)) % pWI->ActualRowCount ;
	BDBG_MSG(( "RenderEnd using DestSurface %p, irow %d, IndxOfTopRow %d\n", (void*)hCCGfxHandle->pCurWin,irow,pWI->IndxOfTopRow)) ;


#if FLASH_BY_2SURFACES
	hCCGfxHandle->WinLibInterface.RenderEnd(hCCGfxHandle->pCurWin, hCCGfxHandle->pCurFlashWin);

	if ( hCCGfxHandle->RenderWithFlash )
	{
		if ( pWI->RowInfo[irow].pFlashSurface )
		{
			if ( pWI->ShowState && !hCCGfxHandle->WinLibInterface.IsCaptionRowVisible(pWI->RowInfo[irow].win) )
			{
				hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pWI->RowInfo[irow].pFlashSurface, true) ;
			}
			else
			{
				hCCGfxHandle->WinLibInterface.SetCaptionRowVisibility(pWI->RowInfo[irow].pFlashSurface, false) ;
			}
		}
		else
		{
			BDBG_ERR(( "ccgfxRenderEnd:  ERROR pFlashSurface is NULL\n")) ;
		}
	}
#else
	hCCGfxHandle->WinLibInterface.RenderEnd(hCCGfxHandle->pCurWin);
#endif

	pWI->iWinWidth = TranslateAnchorRCtoUpperLeftXY(hCCGfxHandle, pWI, rAnchor, cAnchor, &xWindow, &yWindow, AnchorPt, nColumns) ;
	WndPos(hCCGfxHandle, hCCGfxHandle->Render_WndId, yWindow, xWindow, pWI->ScrollPhase) ;

	hCCGfxHandle->Render_WndId = -1 ;
} /* BCCGFX_P_RenderEnd */


/****************************************************************
 *
 * Function:		BCCGFX_P_SetWindowFill
 *
 * Input:			WndId			- window identifier
 *					FillOpacity		- window fill opacity
 *					FillColor		- window fill color RGB(222)
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function sets the given window's fill attributes.
 *
 *****************************************************************/
void BCCGFX_P_SetWindowFill(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		BDCC_Opacity FillOpacity, 
		unsigned int FillColor)
{
	BDBG_MSG(("ccgfxSetWindowFill wnd=%d\n", WndId)) ;

	hCCGfxHandle->WndInfo[WndId].FillOpacity = FillOpacity ;
	hCCGfxHandle->WndInfo[WndId].FillColor = FillColor ;
}


/****************************************************************
 *
 * Function:		BCCGFX_P_SetJustification
 *
 * Input:			WndId				- window identifier
 *					Justification		- justification enum
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function sets the given window's justification.
 *
 *****************************************************************/
void BCCGFX_P_SetJustification(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
    BCCGFX_P_Justify Justification
    )
{
	hCCGfxHandle->WndInfo[WndId].Justification = Justification ;
}


/****************************************************************
 *
 * Function:		BCCGFX_P_SetPrintDirection
 *
 * Input:			WndId			- window identifier
 *					PrintDirectoin	- window print direction
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function sets the given window's print direction.
 *
 *****************************************************************/
void BCCGFX_P_SetPrintDirection(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
    BCCGFX_P_Direction PrintDirection
    )
{
	BDBG_MSG(("ccgfxSetPrintDirection wnd=%d\n", WndId)) ;

	hCCGfxHandle->WndInfo[WndId].PrintDirection = PrintDirection ;
}

					 
/****************************************************************
 *
 * Function:		BCCGFX_P_SetPriority
 *
 * Input:			WndId			- window identifier
 *					Priority		- window display order
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function sets the given window's display order.
 *
 *****************************************************************/
void BCCGFX_P_SetPriority(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
    unsigned int Priority
    )
{
	BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;
	int ZOrder ;
	int row ;

	BDBG_MSG(("ccgfxSetPriority wnd=%d\n", WndId)) ;

	/*
	 * convert priority to venom2's notion of zorder
	 * priority is 0 to 7, 0 on top
	 * zorder   is 0 to 15, 15 on top
	 */
	ZOrder = (int)(12 - Priority) ;
	pWI->ZOrder = ZOrder ;

	for ( row = 0 ; row < pWI->ActualRowCount ; row++ )
	{
		hCCGfxHandle->WinLibInterface.SetCaptionRowZorder(pWI->RowInfo[row].win, ZOrder) ;
	}

}

					 
/****************************************************************
 *
 * Function:		BCCGFX_P_SetScrollDirection
 *
 * Input:			WndId			- window identifier
 *					ScrollDirection	- window scroll direction
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function sets the given window's scroll direction.
 *
 *****************************************************************/
void BCCGFX_P_SetScrollDirection(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
    BCCGFX_P_Direction ScrollDirection
    )
{
	BSTD_UNUSED(hCCGfxHandle);
	BSTD_UNUSED(ScrollDirection);
	BDBG_MSG(("ccgfxSetScrollDirection wnd=%d\n", WndId)) ;
}

					 
/****************************************************************
 *
 * Function:		BCCGFX_P_SetEffect
 *
 * Input:			WndId			- window identifier
 *					EffectType		- window effect type
 *					EffectDirection	- window effect direction
 *					EffectSpeed		- speed (in 0.5 second units)
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function sets the given window's effect type and
 * characteristics.
 *
 *****************************************************************/
void BCCGFX_P_SetEffect(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		BCCGFX_P_Effect EffectType, 
		BCCGFX_P_Direction EffectDirection, 
    unsigned int EffectSpeed
    )
{
	BSTD_UNUSED(hCCGfxHandle);
	BSTD_UNUSED(WndId);
	BSTD_UNUSED(EffectType);
	BSTD_UNUSED(EffectDirection);
	BSTD_UNUSED(EffectSpeed);
	BDBG_MSG(("ccgfxSetEffect wnd=%d\n", WndId)) ;
}

					 
/****************************************************************
 *
 * Function:		ccgfxSetBorder
 *
 * Input:			WndId			- window identifier
 *					BorderType		- window border type
 *					BorderColor		- window border color RGB(222)
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function sets the given window's scroll direction.
 *
 *****************************************************************/
void BCCGFX_P_SetBorder(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		BDCC_Edge BorderType, 
    unsigned int BorderColor
    )
{
	BSTD_UNUSED(hCCGfxHandle);
	BSTD_UNUSED(WndId);
	BSTD_UNUSED(BorderType);
	BSTD_UNUSED(BorderColor);
	BDBG_MSG(("ccgfxSetBorder wnd=%d\n", WndId)) ;
}



/****************************************************************
 *
 * Function:		ccgfxSetTranslucentLevel
 *
 * Input:			
 *					TranslucentLevel	- alpha blending value for
 *										  oTranslucent opacity
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function sets the given window's scroll direction.
 *
 *****************************************************************/
void BCCGFX_P_SetTranslucentLevel(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
    unsigned int TranslucentLevel
    )
{
	BDBG_MSG(("ccgfxSetTranslucentLevel level=%d\n", TranslucentLevel)) ;

	hCCGfxHandle->AlphaLevel = TranslucentLevel ;
}


/****************************************************************
 *
 * Function:		ccgfxSetPenFgColor
 *
 * Input:			WndId			- window identifier
 *					Opacity			- pen opacity
 *					Color			- pen color RGB(222)
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function sets the given window's pen's foreground
 * opacity and color.
 *
 *****************************************************************/
void BCCGFX_P_SetPenFgColor(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		BDCC_Opacity Opacity, 
    unsigned int Color
    )
{
	BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;
	
	BDBG_MSG(("ccgfxSetPenFgColor wnd=%d\n", WndId)) ;

	pWI->PenFgOpacity = Opacity ;
	pWI->PenFgColor = Color ;

	if ( hCCGfxHandle->Render_WndId == WndId )
	{
		hCCGfxHandle->WinLibInterface.SetCharFGColor(hCCGfxHandle->pCurWin, pWI->PenFgColor, pWI->PenFgOpacity) ;
#if FLASH_BY_2SURFACES
		if ( hCCGfxHandle->pCurFlashWin )
		{
			hCCGfxHandle->WinLibInterface.SetCharFGColor(hCCGfxHandle->pCurFlashWin, pWI->PenFgColor, pWI->PenFgOpacity) ;
		}
#endif
	}
}

void BCCGFX_P_SetPenEdgeColor(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
    int WndId, 
    unsigned int Color
    )
{
	BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;
	
	BDBG_MSG(("ccgfxSetPenFgColor wnd=%d\n", WndId)) ;

	pWI->PenEdgeColor = Color ;

	/*
	** Note: use the foreground opacity with the edge color.
	*/
	if ( hCCGfxHandle->Render_WndId == WndId )
	{
		hCCGfxHandle->WinLibInterface.SetCharEdgeColor(hCCGfxHandle->pCurWin, pWI->PenEdgeColor, pWI->PenFgOpacity) ;
#if FLASH_BY_2SURFACES
		if ( hCCGfxHandle->pCurFlashWin )
		{
			hCCGfxHandle->WinLibInterface.SetCharEdgeColor(hCCGfxHandle->pCurFlashWin ,pWI->PenEdgeColor, pWI->PenFgOpacity) ;
		}
#endif
	}
}
					 
void BCCGFX_P_SetPenEdgeType(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
    int WndId, 
    BDCC_Edge edgeType
    )
{
    BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;

    BDBG_MSG(("BCCGFX_P_SetPenEdgeType wnd=%d\n", WndId)) ;

    pWI->EdgeType = edgeType ;

    hCCGfxHandle->WinLibInterface.SetCharEdgeType( hCCGfxHandle->pCurWin, edgeType );

#if FLASH_BY_2SURFACES
	if ( hCCGfxHandle->pCurFlashWin )
	{
    	hCCGfxHandle->WinLibInterface.SetCharEdgeType( hCCGfxHandle->pCurFlashWin, edgeType );
	}
#endif

    return;
}

/****************************************************************
 *
 * Function:		BCCGFX_P_SetPenBgColor
 *
 * Input:			WndId			- window identifier
 *					Opacity			- pen opacity
 *					Color			- pen color RGB(222)
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function sets the given window's pen's background
 * opacity and color.
 *
 *****************************************************************/
void BCCGFX_P_SetPenBgColor(
    BCCGFX_P_GfxHandle hCCGfxHandle,
		int WndId, 
		BDCC_Opacity Opacity, 
    unsigned int Color
    )
{
	BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;
	
	BDBG_MSG(("ccgfxSetPenBgColor wnd=%d\n", WndId)) ;

	pWI->PenBgOpacity = Opacity ;
	pWI->PenBgColor = Color ;
	
	if ( hCCGfxHandle->Render_WndId == WndId )
	{
		hCCGfxHandle->WinLibInterface.SetCharBGColor(hCCGfxHandle->pCurWin, pWI->PenBgColor, pWI->PenBgOpacity) ;
#if FLASH_BY_2SURFACES
		if ( hCCGfxHandle->pCurFlashWin )
		{
			hCCGfxHandle->WinLibInterface.SetCharBGColor(hCCGfxHandle->pCurFlashWin, pWI->PenBgColor, pWI->PenBgOpacity) ;
		}
#endif
	}
}


/****************************************************************
 *
 * Function:		BCCGFX_P_SetPenSize
 *
 * Input:			WndId			- window identifier
 *					PenSize			- pen size
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function sets the given window's pen's size.
 *
 *****************************************************************/
void BCCGFX_P_SetPenSize(
    BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
    BDCC_PenSize PenSize
    )
{

	BDBG_MSG(("ccgfxSetPenSize wnd=%d\n", WndId)) ;

	if ( hCCGfxHandle->WndInfo[WndId].PenSize != PenSize )
	{
		hCCGfxHandle->WndInfo[WndId].PenSize = PenSize ;
		PickFont(hCCGfxHandle, WndId) ;
	}
}

					 
/****************************************************************
 *
 * Function:		BCCGFX_P_SetPenFontStyle
 *
 * Input:			WndId			- window identifier
 *					FontStyle		- font style
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function sets the given window's pen's font style.
 *
 *****************************************************************/
void BCCGFX_P_SetPenFontStyle(
    BCCGFX_P_GfxHandle hCCGfxHandle,
		int WndId, 
    BDCC_FontStyle FontStyle
    )
{
	BDBG_MSG(("ccgfxSetPenFontStyle wnd=%d\n", WndId)) ;

	if ( hCCGfxHandle->WndInfo[WndId].FontStyle != FontStyle )
	{
		hCCGfxHandle->WndInfo[WndId].FontStyle = FontStyle ;
		PickFont(hCCGfxHandle, WndId) ;
	}
}

					 
/****************************************************************
 *
 * Function:		BCCGFX_P_SetPenItalics
 *
 * Input:			WndId			- window identifier
 *					Italics			- 1 for on; 0 for off
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function turns on or off italics for the given window's pen.
 *
 *****************************************************************/
void BCCGFX_P_SetPenItalics(
    BCCGFX_P_GfxHandle hCCGfxHandle,
		int WndId, 
    int Italics
    )
{
	BDBG_MSG(("ccgfxSetPenItalics wnd=%d italics=%d\n", WndId, Italics)) ;

	if ( hCCGfxHandle->WndInfo[WndId].Italics != Italics )
	{
		hCCGfxHandle->WndInfo[WndId].Italics = Italics ;
		PickFont(hCCGfxHandle, WndId) ;
	}
}

					 
/****************************************************************
 *
 * Function:		BCCGFX_P_SetPenUnderline
 *
 * Input:			WndId			- window identifier
 *					Underline		- 1 for on; 0 for off
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function turns on or off underline for the given window's pen.
 *
 *****************************************************************/
void BCCGFX_P_SetPenUnderline(
    BCCGFX_P_GfxHandle hCCGfxHandle,
		int WndId, 
    int Underline
    )
{
	BDBG_MSG(("ccgfxSetPenUnderline wnd=%d\n", WndId)) ;
	
	if ( hCCGfxHandle->WndInfo[WndId].Underline != Underline )
	{
		hCCGfxHandle->WndInfo[WndId].Underline = Underline ;
		PickFont(hCCGfxHandle, WndId) ;
	}
}

/****************************************************************
 *
 * Function:		BCCGFX_P_RedrawScreen
 *
 * Input:			WndId			- window identifier
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function causes the screen to be redrawn with the most recent CC data.
 *
 *****************************************************************/
void BCCGFX_P_RedrawScreen(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{
	BDBG_MSG(("BCCGFX_P_RedrawScreen \n")) ;

	hCCGfxHandle->WinLibInterface.UpdateScreen( hCCGfxHandle->hWinLibHandle );

	/*
	** For the time dependent actions (like scrolling), keep track of when
	** the screen was last redrawn.
	*/
	BCCGFX_P_TimeUpdate( hCCGfxHandle, &(hCCGfxHandle->msLastScreenRefresh) );
	
	return;
}

/****************************************************************
 *
 * Function:		BCCGFX_P_GetShowState
 *
 * Input:			WndId			- window identifier
 *
 * Output:
 *
 * Returns:             0 for hidden, 1 for visible
 *
 * Description:
 *
 *
 *****************************************************************/
int BCCGFX_P_GetShowState(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId
    )
{
	BCCGFX_P_WND_INFO * pWinInfo = &hCCGfxHandle->WndInfo[WndId] ;
	int                                 iShowState;
	
	BDBG_MSG(("BCCGFX_P_GetShowState enter\n")) ;

	iShowState = pWinInfo->ShowState;

	BDBG_MSG(("BCCGFX_P_GetShowState enter\n")) ;

	return iShowState;
} /* BCCGFX_P_WndShow */

/****************************************************************
 *
 * Function:		BCCGFX_P_SetFlushState
 *
 * Input:			WndId			- window identifier
 *
 * Output:
 *
 * Returns:             0 for hidden, 1 for visible
 *
 * Description:
 *
 *
 *****************************************************************/
void BCCGFX_P_SetDirtyState(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId,
    bool bState
    )
{
	BCCGFX_P_WND_INFO * pWinInfo = &hCCGfxHandle->WndInfo[WndId] ;
	
	BDBG_MSG(("BCCGFX_P_SetDirtyState enter\n")) ;

	pWinInfo->bNeedToRedraw = bState;

	BDBG_MSG(("BCCGFX_P_SetDirtyState enter\n")) ;

	return;
} /* BCCGFX_P_WndShow */

/****************************************************************
 *
 * Function:		BCCGFX_P_GetFlushState
 *
 * Input:			WndId			- window identifier
 *
 * Output:
 *
 * Returns:             0 for hidden, 1 for visible
 *
 * Description:
 *
 *
 *****************************************************************/
bool BCCGFX_P_GetDirtyState(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId
    )
{
	BCCGFX_P_WND_INFO * pWinInfo = &hCCGfxHandle->WndInfo[WndId] ;
	bool                                bState;
	
	BDBG_MSG(("BCCGFX_P_GetDirtyState enter\n")) ;

	 bState = pWinInfo->bNeedToRedraw;

	BDBG_MSG(("BCCGFX_P_GetDirtyState enter\n")) ;

	return bState;
} /* BCCGFX_P_WndShow */


static int TranslateAnchorRCtoUpperLeftXY(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    BCCGFX_P_WND_INFO *pWI,
    int rAnchor,
    int cAnchor,
    int *pxWindow,
    int *pyWindow,
    BCCGFX_P_AnchorPoint AnchorPt,
    int nColumns
    )
{
    int xAnchor, yAnchor, srcWindowWidth=0, srcWindowHeight=0, dstWindowWidth, dstWindowHeight, xWindow=0, yWindow=0 ;
	int row, irow = 0 ;
	BDCC_PenSize largestPenSize = BDCC_PenSize_Small, RowPenSize;
	BDCC_WINLIB_Rect clipRect, largestBoundingRect;
    /*
    ** compute anchor in (x,y)
    */    
    xAnchor = hCCGfxHandle->iSafeTitleX + (cAnchor * ((hCCGfxHandle->iFrameBufferWidth  - (2 * hCCGfxHandle->iSafeTitleX)) / nColumns)) ;
    yAnchor = hCCGfxHandle->iSafeTitleY + (rAnchor * ((hCCGfxHandle->iFrameBufferHeight - (2 * hCCGfxHandle->iSafeTitleY)) / 15)) ;
	/*printf("Y: safe title = %d, rA = %d, FrameBufferHeight = %d, res = %d, AP=%d\n", hCCGfxHandle->iSafeTitleY, rAnchor, hCCGfxHandle->iFrameBufferHeight, yAnchor, AnchorPt); */ 
	/*printf("X: safe title = %d, rA = %d, FrameBufferWidth = %d, res = %d, AP=%d\n", hCCGfxHandle->iSafeTitleX, cAnchor, hCCGfxHandle->iFrameBufferWidth, xAnchor, AnchorPt); */
	/* Compute the height and width of the window based on the individual character pen sizes */
	for ( row=0 ; row < (pWI->RowCount + 1) ; row++ )
	{
	    irow = (row + pWI->IndxOfTopRow) % (pWI->RowCount + 1) ;
		
		if ( row != pWI->RowCount ) /* don't include the extra row for scrolling when calculating the window height */
		{
			hCCGfxHandle->WinLibInterface.GetCaptionRowTextRect(pWI->RowInfo[irow].win, &clipRect);
			srcWindowHeight += clipRect.height;
		}

		RowPenSize = pWI->RowInfo[irow].EmptyRow ? pWI->PenSize: pWI->RowInfo[irow].LargestPenSize; 

		if ( RowPenSize > largestPenSize )
			largestPenSize = RowPenSize;
	}
	
	hCCGfxHandle->WinLibInterface.GetMaxBoundingRect(pWI->RowInfo[irow].win, pWI->ColCount, largestPenSize, &largestBoundingRect);

	srcWindowWidth = largestBoundingRect.width;

	/* need to use the destination window sizes to calculate the top left co-ordinates from the anchor co-ordinates */
	dstWindowWidth  = (srcWindowWidth * hCCGfxHandle->ScaleFactor) / 100;
	dstWindowHeight = (srcWindowHeight * hCCGfxHandle->ScaleFactor) / 100;

	/*printf("SrcWndHeight=%d SrcWndWidth=%d DstWndHeight=%d, DstWndWidth=%d\n", srcWindowHeight, srcWindowWidth, dstWindowHeight, dstWindowWidth);*/
	
    /*
    ** compute x of upper left corner of window
    */
    switch ( AnchorPt )
    {
    case BCCGFX_P_AnchorPoint_eUpperLeft :
    case BCCGFX_P_AnchorPoint_eMidLeft :
    case BCCGFX_P_AnchorPoint_eLowerLeft :
        xWindow = xAnchor ;
        break ;

    case BCCGFX_P_AnchorPoint_eUpperCenter :
    case BCCGFX_P_AnchorPoint_eMidCenter :
    case BCCGFX_P_AnchorPoint_eLowerCenter :
        xWindow = xAnchor - dstWindowWidth/2 ;
        break ;

    case BCCGFX_P_AnchorPoint_eUpperRight :
    case BCCGFX_P_AnchorPoint_eMidRight :
    case BCCGFX_P_AnchorPoint_eLowerRight :
        xWindow = xAnchor - dstWindowWidth ;
        break ;
    }

    /*
    ** compute y of upper left corner of window
    */
    switch ( AnchorPt )
    {
    case BCCGFX_P_AnchorPoint_eUpperLeft :
    case BCCGFX_P_AnchorPoint_eUpperCenter :
    case BCCGFX_P_AnchorPoint_eUpperRight :
        yWindow = yAnchor ;
        break ;

    case BCCGFX_P_AnchorPoint_eMidLeft :
    case BCCGFX_P_AnchorPoint_eMidCenter :
    case BCCGFX_P_AnchorPoint_eMidRight :
        yWindow = yAnchor - dstWindowHeight/2 ;
        break ;

    case BCCGFX_P_AnchorPoint_eLowerLeft :
    case BCCGFX_P_AnchorPoint_eLowerCenter :
    case BCCGFX_P_AnchorPoint_eLowerRight :
        yWindow = yAnchor - dstWindowHeight ;
        break ;
    }

	/* ensure at least iSafeTitleX, iSafeTitleY */
	if(xWindow < hCCGfxHandle->iSafeTitleX) xWindow = hCCGfxHandle->iSafeTitleX;
	if(yWindow < hCCGfxHandle->iSafeTitleY) yWindow = hCCGfxHandle->iSafeTitleY;

    BDBG_MSG(("Translate:  cc %d   width = %d\n", pWI->ColCount, srcWindowWidth)) ;

    if ( pxWindow )
        *pxWindow = xWindow ;

    if ( pyWindow )
        *pyWindow = yWindow ;
	/*printf(" %d %d\n", xWindow, yWindow);*/
    /*
    ** TODO: if we add space at the beginning and end of the line,
    ** will we need to do something similiar to the following?
    **
    ** wWindow += SPACE_PRE + SPACE_POST ;
    */

    return( srcWindowWidth ) ;
}

static void RotateRows(
    BCCGFX_P_WND_INFO * pWI,
    int RotateAmt
    )
{
	/* this can be optimized, but it is called very infrequently */
	int i, irow ;
	BCCGFX_P_RowInfo SaveRowInfo ;
	
	for ( i=0 ; i < RotateAmt ; i++ )
	{
		SaveRowInfo = pWI->RowInfo[0] ;
		for ( irow=0 ; irow < pWI->ActualRowCount-1 ; irow++ )
		{
			pWI->RowInfo[irow] = pWI->RowInfo[irow+1] ;
		}
		pWI->RowInfo[irow] = SaveRowInfo ;
	}
}

static void WndPos(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId,
    int FirstRow,
    int Col,
    int ScrollPhase
    )
{
    BCCGFX_P_WND_INFO * pWI = &hCCGfxHandle->WndInfo[WndId] ;
    BDCC_WINLIB_hRow pSurface ;
#if FLASH_BY_2SURFACES
    BDCC_WINLIB_hRow pFlashSurface ;
#endif
    int row, xSurface, ySurface, irow ;
    BDCC_WINLIB_Rect srcRect ;
    BDCC_WINLIB_Rect dstRect ;
	BDCC_WINLIB_Rect textRect ;
	uint32_t surfaceWidth;
	uint32_t surfaceHeight ;
    int RowCountAdj = 0 ;
	uint32_t LineAdj = 0 ;
    bool bClippedOut = 0 ;
    if ( ! pWI->fCreated )
        return ;

    RowCountAdj = 1 ;

    /* now FirstRow and Col args are really  yWindow and xWindow */
    xSurface = Col ;
    ySurface =  FirstRow;

    for ( row=0 ; row < (pWI->RowCount + 1) ; row++ )
    {		
        irow = (row + pWI->IndxOfTopRow) % (pWI->RowCount + RowCountAdj) ;
        pSurface = pWI->RowInfo[irow].win ;

        if ( pSurface )
        {
            hCCGfxHandle->WinLibInterface.GetSurfaceRect( pSurface, &surfaceWidth, &surfaceHeight);
            BDBG_ASSERT((surfaceWidth >= (uint32_t) pWI->iWinWidth));
            hCCGfxHandle->WinLibInterface.GetCaptionRowTextRect( pSurface, &textRect );		

            if (( xSurface >= hCCGfxHandle->iFrameBufferWidth - hCCGfxHandle->iSafeTitleX) || 
				(ySurface >= hCCGfxHandle->iFrameBufferHeight - hCCGfxHandle->iSafeTitleY))
            {			    
                srcRect.x = dstRect.x = 0;
                srcRect.y = dstRect.y = 0;
                srcRect.width = dstRect.width = 0;
                srcRect.height = dstRect.height = 0;	

                bClippedOut = true;     /* The surface lies entirely outside the framebuffer. */
            }
            else
            {
                dstRect.x = xSurface;
                dstRect.y = ySurface;

				/* align different character sizes to the bottom of the row */
                srcRect.y = textRect.y;
                if ( row == 0 )
                {
                    /* show a partial top row */
					LineAdj = (textRect.height * ScrollPhase) / BCCGFX_P_MAX_SCROLL_PHASE;
                    srcRect.y += LineAdj ;
                    srcRect.height = textRect.height - LineAdj;
                }
                else if ( row == pWI->RowCount )
                {
                    /* last row possibly partial */
                	if(LineAdj < textRect.height)
                    	srcRect.height = LineAdj ;
					else
						srcRect.height = textRect.height;
                }
                else
                {
                   srcRect.height = textRect.height;
                }

				srcRect.width = pWI->iWinWidth;

				switch(pWI->Justification)
				{
					case BDCC_Justify_eLeft:
					case BDCC_Justify_eFull:
						srcRect.x = 0;
						break;
					case BDCC_Justify_eCenter:
						if(textRect.width)
						{
                            BDBG_ASSERT((textRect.x >= ((pWI->iWinWidth - textRect.width) / 2)));
							srcRect.x = textRect.x - ((pWI->iWinWidth - textRect.width) / 2);
						}
						else
						{
							srcRect.x = 0;
						}
						break;
					case BDCC_Justify_eRight:
						if(textRect.width)
						{
                            BDBG_ASSERT(((textRect.x + textRect.width) >= (uint32_t) pWI->iWinWidth));
							srcRect.x = (textRect.x + textRect.width) - pWI->iWinWidth ;
						}
						else
						{
							srcRect.x = 0;
						}
						break;
				}

				/* apply scale factor if one has been specified */
				dstRect.height = (srcRect.height * hCCGfxHandle->ScaleFactor) / 100;
				dstRect.width = (srcRect.width * hCCGfxHandle->ScaleFactor) / 100;

                /*
                ** Clip to the width of the frame buffer.
                ** taking into account "iSafeTitleX"
                */
                if ( dstRect.x + dstRect.width > (uint32_t) (hCCGfxHandle->iFrameBufferWidth - hCCGfxHandle->iSafeTitleX) )
                {
                    dstRect.width = (( hCCGfxHandle->iFrameBufferWidth - hCCGfxHandle->iSafeTitleX ) - dstRect.x );
					srcRect.width = (dstRect.width * 100) / hCCGfxHandle->ScaleFactor;
                }

                /*
                ** Clip to the height of the frame buffer.
                ** taking into account "iSafeTitleY"?
                */
                if ( dstRect.y + dstRect.height > (uint32_t) (hCCGfxHandle->iFrameBufferHeight - hCCGfxHandle->iSafeTitleY) )
                {
                    dstRect.height = (( hCCGfxHandle->iFrameBufferHeight - hCCGfxHandle->iSafeTitleY ) - dstRect.y );
					srcRect.height = (dstRect.height * 100) / hCCGfxHandle->ScaleFactor;
                }

                /*
                ** After intersecting with the framebuffer, check if the surface has been clipped out.
                */
                if ( dstRect.height <= 0 || dstRect.width <= 0 )
                {
                    srcRect.x = dstRect.x = 0;
                    srcRect.y = dstRect.y = 0;
                    srcRect.width = dstRect.width = 0;
                    srcRect.height = dstRect.height = 0;
                    bClippedOut = true;
                }
                else
                {
                    bClippedOut = false;
                }
                
                BDBG_MSG(( "position (%d,%d)\n", xSurface, ySurface)) ;
                
            }

            /*
            ** Defines which portion of the surface will be displayed.
            */
            hCCGfxHandle->WinLibInterface.SetCaptionRowClipRect( pSurface, &srcRect );

            /*
            ** Defines where the surface will be displayed.
            */
            hCCGfxHandle->WinLibInterface.SetCaptionRowDispRect( pSurface, &dstRect );

            /*
            ** Set the clip state for use by the rendering system.
            */
            hCCGfxHandle->WinLibInterface.SetClipState( pSurface, bClippedOut );


#if FLASH_BY_2SURFACES
            pFlashSurface = pWI->RowInfo[irow].pFlashSurface ;

            if ( pFlashSurface )
            {
                hCCGfxHandle->WinLibInterface.SetCaptionRowClipRect( pFlashSurface, &srcRect );
                hCCGfxHandle->WinLibInterface.SetCaptionRowDispRect( pFlashSurface, &dstRect );
                hCCGfxHandle->WinLibInterface.SetClipState( pFlashSurface, bClippedOut );
            }
#endif
            pWI->RowInfo[irow].xSurface = xSurface ;
            pWI->RowInfo[irow].ySurface = ySurface ;

			/* next row starts beneath the current one */
			ySurface += dstRect.height;
        }/* end of if ( pSurface */
        
    }   /* end of for ( "each row" ) */

    pWI->FirstRowRow = FirstRow ;
    pWI->Column = Col ;

} /* end of WndPos() */


static void PickFont(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    int WndId
    )
{
    int iError;
	BDCC_PenStyle penStyle;
	BDCC_FontStyle fontStyle;
	BDCC_PenSize penSize;

    if(hCCGfxHandle->WndInfo[WndId].Underline)
    {
		penStyle = hCCGfxHandle->WndInfo[WndId].Italics ? BDCC_PenStyle_Italics_Underline : BDCC_PenStyle_Underline;
    }
	else
	{
		penStyle = hCCGfxHandle->WndInfo[WndId].Italics ? BDCC_PenStyle_Italics : BDCC_PenStyle_Standard;
	}
		
    fontStyle = hCCGfxHandle->WndInfo[WndId].FontStyle;
	penSize = hCCGfxHandle->WndInfo[WndId].PenSize;

    iError = hCCGfxHandle->WinLibInterface.SetFont(
		hCCGfxHandle->pCurWin,
		fontStyle,
		penSize,
		penStyle
		);
#if FLASH_BY_2SURFACES
	if ( hCCGfxHandle->pCurFlashWin )
	{

	    iError = hCCGfxHandle->WinLibInterface.SetFont(
			hCCGfxHandle->pCurFlashWin,
	    	fontStyle,
	    	penSize,
			penStyle
		) ;
	}
#endif

    if ( iError )
    {
        BDBG_ERR(( "PickFont:  error setting font: %d\n", iError)) ;
    }
}



static void FillSurface(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    BDCC_WINLIB_hRow pSurface,
    BDCC_Opacity Opacity,
    uint8_t FillColor
    )
{
    BSTD_UNUSED(hCCGfxHandle);

    if ( pSurface )
    {
        BDBG_MSG(("FillSurface, filling pSurface %p with color 0x%08x\n", (void*)pSurface,FillColor)) ;

        hCCGfxHandle->WinLibInterface.ClearCaptionRow(pSurface, Opacity, FillColor) ;
    }
}


/************************
 *  Initializing the win engine and 7 CCgfx Windows
 ************************/

static int BCCGFX_P_SurfaceInit(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{
    int i, iNumSurfacesToAllocate;
    BCCGFX_P_SurfaceRowPool *pSurfPool = &(hCCGfxHandle->SurfacePool);
    BCCGFX_P_SurfaceInfo **ppNextSurfInfo = &(pSurfPool->pSurfInfoList);
  
    BDBG_MSG(("BCCGFX_P_SurfaceInit")) ;

    /*
    ** Now that we know what size to make the surfaces, allocate some number of them.
    ** This number could be "0" since surfaces are allocate on demand in BCCGFX_P_SurfaceReserve.
    */

    if(pSurfPool->NumSurfaces < BCCGFX_P_INITIAL_SURFACE_COUNT)
    {
        iNumSurfacesToAllocate = BCCGFX_P_INITIAL_SURFACE_COUNT - pSurfPool->NumSurfaces;

    for ( i=0 ; i < iNumSurfacesToAllocate; i++ )
    {
        BCCGFX_P_SurfaceAlloc( hCCGfxHandle, ppNextSurfInfo );
        pSurfPool->FreeCount++ ;

        ppNextSurfInfo = &((*ppNextSurfInfo)->pNextSurfInfo);
    }   
    }

    BDBG_MSG(("%s %d %d", __FUNCTION__, pSurfPool->NumSurfaces, pSurfPool->FreeCount));
    return BDCC_Error_eSuccess ;
}


static void BCCGFX_P_SurfaceClose(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{
    BCCGFX_P_SurfaceInfo * pNextSurfInfo ;
    BCCGFX_P_SurfaceRowPool *pSurfPool = &hCCGfxHandle->SurfacePool ;
    BCCGFX_P_SurfaceInfo *pSurfInfo = pSurfPool->pSurfInfoList;

    /*
    ** Walk through the list of surface descriptors;
    ** first freeing the associated surface, then the descriptor.
    */
    while( pSurfInfo )
    {
        pNextSurfInfo = pSurfInfo->pNextSurfInfo;

        if ( pSurfInfo->hSurface )
        {
            hCCGfxHandle->WinLibInterface.DestroyCaptionRow( pSurfInfo->hSurface ) ;
        }

        BKNI_Free( pSurfInfo );
        pSurfInfo = pNextSurfInfo;

    }

    pSurfPool->FreeCount = 0;
    pSurfPool->NumSurfaces = 0;
    pSurfPool->pSurfInfoList = NULL;
    BDBG_MSG(("%s %d %d", __FUNCTION__, pSurfPool->NumSurfaces, pSurfPool->FreeCount));

    return;
}


static int BCCGFX_P_SurfaceAlloc(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    BCCGFX_P_SurfaceInfo **ppNextSurfInfo
    )
{
    BCCGFX_P_SurfaceInfo *          pSurfInfo ;
    BCCGFX_P_SurfaceRowPool *   pSurfPool = &hCCGfxHandle->SurfacePool ;

    /*
    ** First allocate and initialize the descriptor for the surface.
    */
    pSurfInfo = *ppNextSurfInfo = (BCCGFX_P_SurfaceInfo *)BKNI_Malloc( sizeof(BCCGFX_P_SurfaceInfo) );
    BDBG_ASSERT((pSurfInfo));

    BKNI_Memset( pSurfInfo, 0, sizeof(BCCGFX_P_SurfaceInfo) );

    /*
    ** Then allocate the surface.
    */
    hCCGfxHandle->WinLibInterface.CreateCaptionRow( 
                hCCGfxHandle->hWinLibHandle, 
                &(pSurfInfo->hSurface)
                );

    BDBG_ASSERT((pSurfInfo->hSurface));

    pSurfPool->NumSurfaces++ ;

    return 0;
}
	

static BDCC_WINLIB_hRow BCCGFX_P_SurfaceReserve(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{

    BCCGFX_P_SurfaceInfo * pSurfInfo ;
    BCCGFX_P_SurfaceInfo ** ppNextSurfInfo ;
    BDCC_WINLIB_hRow pSurface = NULL ;
    BCCGFX_P_SurfaceRowPool *pSurfPool = &hCCGfxHandle->SurfacePool ;

    if ( pSurfPool->FreeCount )
    {
        /*
        ** Walk through the list of surfaces until we find 
        ** one that is not being used.
        */

        pSurfInfo = pSurfPool->pSurfInfoList;

        while( pSurfInfo )
        {

            if ( !pSurfInfo->fUsed  )
            {
                pSurface = pSurfInfo->hSurface ;
                pSurfInfo->fUsed = 1 ;
                pSurfPool->FreeCount-- ;
                break ;
            }
            pSurfInfo = pSurfInfo->pNextSurfInfo;
        }

        BDBG_ASSERT((pSurface));
    }
    else
    {
        /* Need to allocate another surface. Add it to the head */
        ppNextSurfInfo = &( pSurfPool->pSurfInfoList );
        pSurfInfo = pSurfPool->pSurfInfoList;
        BCCGFX_P_SurfaceAlloc( hCCGfxHandle, ppNextSurfInfo );
        (*ppNextSurfInfo)->pNextSurfInfo = pSurfInfo;

        pSurfInfo = pSurfPool->pSurfInfoList;
        BDBG_ASSERT((pSurfInfo));
        pSurface = pSurfInfo->hSurface ;

        pSurfInfo->fUsed = 1 ;
    }

    if ( NULL == pSurface )
    {
        BDBG_ERR(( "BCCGFX_P_SurfaceReserve: unable to alloc surface\n")) ;
    }

    return( pSurface ) ;
}


static void BCCGFX_P_SurfaceReleaseAll(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{
    BCCGFX_P_SurfaceRowPool *pSurfPool = &hCCGfxHandle->SurfacePool;
    BCCGFX_P_SurfaceInfo *pSurfInfo = pSurfPool->pSurfInfoList;
        
    while(pSurfInfo)
    {
        pSurfInfo->fUsed = 0 ;
        pSurfInfo = pSurfInfo->pNextSurfInfo;
    }
	
    pSurfPool->FreeCount = pSurfPool->NumSurfaces;
    BDBG_MSG(("%s %d %d", __FUNCTION__, pSurfPool->NumSurfaces, pSurfPool->FreeCount));
}


static void BCCGFX_P_SurfaceRelease(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    BDCC_WINLIB_hRow pSurface
    )
{

    BCCGFX_P_SurfaceInfo *          pSurfInfo ;
    BCCGFX_P_SurfaceRowPool *   pSurfPool = &hCCGfxHandle->SurfacePool ;

    if ( pSurface )
    {
        /*
        ** Walk through the list of surfaces until we find the correct one
        ** to make as "unused".
        */
        
        pSurfInfo = pSurfPool->pSurfInfoList;

        while( pSurfInfo )
        {
            if ( pSurface == pSurfInfo->hSurface )
            {
                if(pSurfInfo->fUsed)
                {
                BDBG_MSG(( "BCCGFX_P_SurfaceRelease %p\n", (void*)pSurface)) ;
                pSurfInfo->fUsed = 0 ;
                pSurfPool->FreeCount++ ;
                }
                break ;
            }

            pSurfInfo = pSurfInfo->pNextSurfInfo;
        }
    }
    }

    

#if CCGFX_DBG_SURFACES_PER_POOL
static void dbg_Fini(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{
	if ( hCCGfxHandle && hCCGfxHandle->pDebugFont )
	{
		BCCGFX_WINLIB_P_UnloadFont(hCCGfxHandle->pDebugFont) ;
		hCCGfxHandle->pDebugFont = NULL ;
	}
}

static void dbg_Init(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{
	int xSurface, ySurface ;
	int berr ;
	
	hCCGfxHandle->pDebugSurface = BCCGFX_P_SurfaceReserve() ;
	xSurface = hCCGfxHandle->iSafeTitleX 10 ;
	ySurface = hCCGfxHandle->iSafeTitleY 0 ;
	BCCGFX_WINLIB_P_SetPosition(hCCGfxHandle->pDebugSurface, xSurface, ySurface) ;

	berr = BCCGFX_WINLIB_P_LoadFont(hCCGfxHandle->hWinLibHandle, 
				"fonts/arial_18_aa.bwin_font", 
				&hCCGfxHandle->pDebugFont) ;
	if ( berr || (NULL == hCCGfxHandle->pDebugFont) )
	{
		BDBG_ERR(( "BCCGFX_WINLIB_P_LoadFont returned err %d\n", berr)) ;
	}
	berr = BCCGFX_WINLIB_P_SetFont(hCCGfxHandle->pDebugSurface, hCCGfxHandle->pDebugFont) ;
	if ( berr )
	{
		BDBG_ERR(( "BCCGFX_WINLIB_P_SetFont returned err %d\n", berr)) ;
	}

	BCCGFX_WINLIB_P_SetForegroundColor(hCCGfxHandle->pDebugSurface, 0xffff0000) ;
	BCCGFX_WINLIB_P_SetBackgroundColor(hCCGfxHandle->pDebugSurface, 0xff000000) ;

	dbg_Str("debug") ;
	
	BCCGFX_WINLIB_P_Show(hCCGfxHandle->pDebugSurface) ;
}

static void dbg_Str(
    BCCGFX_P_GfxHandle hCCGfxHandle,
    char *str
    )
{
/*	unsigned int DebugFillARGB = 0x22000044 ; // translucent dk blue */
	unsigned int DebugFillARGB = 0xff000044 ; /* translucent dk blue*/
	FillSurface(hCCGfxHandle->pDebugSurface, DebugFillARGB) ;
	BCCGFX_WINLIB_P_SetPenPosition(hCCGfxHandle->pDebugSurface, 0, 0 ) ;
	BCCGFX_WINLIB_P_DrawString(hCCGfxHandle->pDebugSurface, (unsigned char *)str) ;
	BCCGFX_WINLIB_P_Show(hCCGfxHandle->pDebugSurface) ;
}

static void dbg_Update(
    BCCGFX_P_GfxHandle hCCGfxHandle
    )
{
#if 1
	char str[64] ;
	sprintf(str, "BldCfg: ") ;
#if FLASH_BY_2SURFACES
	strcat(str, " F2S") ;
#elif FLASH_BY_RERENDER
	strcat(str, " FRE") ;
#else
	strcat(str, " nof") ;
#endif
#if EXTRA_ROW_SCROLL
	strcat(str, " XRS") ;
#else
	strcat(str, " xrs") ;
#endif
#if CREATE_ONLY_VISIBLE_WINDOWS
	strcat(str, " COVW") ;
#else
	strcat(str, " covw") ;
#endif
	strcat(str, " pxl") ;

	dbg_Str(str) ;
#else
	char str[12] ;
	static char num[] = "0123456789" ;
	int wnd ;
	
	for ( wnd=0 ; wnd < CCGFX_NUM_WNDS ; wnd++ )
	{
		if ( hCCGfxHandle->WndInfo[wnd].fCreated )
			str[wnd] = num[wnd] ;
		else
			str[wnd] = ' ' ;
	}
	str[wnd] = 0 ;
	dbg_Str(str) ;
#endif
}

#endif

