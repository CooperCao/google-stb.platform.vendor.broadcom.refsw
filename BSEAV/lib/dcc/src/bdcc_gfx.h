/***************************************************************************
 *     (c)2002-2008 Broadcom Corporation
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
 *   This is the header for the DTVCC 708 library
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BDCCGFX_H
#define BDCCGFX_H

#include "bdcc_cfg.h"
#include "b_dcc_lib.h"
#include "bcc_winlib.h"

#ifdef __cplusplus
extern "C" {
#endif


/************************
 *
 *   Types
 *
 ************************/


/***************************************************************************
Summary:
	Justify is a window attribute. 

Description:
	Justify is a window attribute. 

See Also:
	
****************************************************************************/
typedef enum BCCGFX_P_Justify
{
	BCCGFX_P_Justify_eLeft	= 0,
	BCCGFX_P_Justify_eRight	= 1,
	BCCGFX_P_Justify_eCenter	= 2, 
	BCCGFX_P_Justify_eFull	= 3
} BCCGFX_P_Justify ;


/***************************************************************************
Summary:
	Print or Scroll Direction is a window attribute. 

Description:
	This enum specifies the possible directions for PrintDirection and 
	ScrollDirection, both of which are window attributes.

See Also:
	
****************************************************************************/
typedef enum BCCGFX_P_Direction
{
	BCCGFX_P_Direction_eLeftToRight	= 0,
	BCCGFX_P_Direction_eRightToLeft	= 1,
	BCCGFX_P_Direction_eTopToBottom	= 2,
	BCCGFX_P_Direction_eBottomToTop	= 3
} BCCGFX_P_Direction ;


/***************************************************************************
Summary:
	This enum specifies the possible effect types which govern how windows 
	are to be shown/hidden. 

Description:
	This enum specifies the possible effect types which govern how windows 
	are to be shown/hidden.

See Also:
	
****************************************************************************/
typedef enum BCCGFX_P_Effect
{
	BCCGFX_P_Effect_eSnap = 0,
	BCCGFX_P_Effect_eFade = 1,
	BCCGFX_P_Effect_eWipe = 2
} BCCGFX_P_Effect ;


/***************************************************************************
Summary:
	This enum specifies the vertical offset of a character.

Description:
	This enum specifies the vertical offset of a character.

See Also:
	
****************************************************************************/
typedef enum BCCGFX_P_VertOffset
{
	BCCGFX_P_VertOffset_eSubscript	= 0,
	BCCGFX_P_VertOffset_eNormal		= 1,
	BCCGFX_P_VertOffset_eSuperscript	= 2
} BCCGFX_P_VertOffset ;


/***************************************************************************
Summary:
	A window's anchor point is declared to be one of the following 9 
	locations.

Description:
	A window's anchor point is declared to be one of the following 9 
	locations.

See Also:
	
****************************************************************************/
typedef enum BCCGFX_P_AnchorPoint
{
	BCCGFX_P_AnchorPoint_eUpperLeft		,
	BCCGFX_P_AnchorPoint_eUpperCenter	,
	BCCGFX_P_AnchorPoint_eUpperRight		,
	BCCGFX_P_AnchorPoint_eMidLeft		,
	BCCGFX_P_AnchorPoint_eMidCenter		,
	BCCGFX_P_AnchorPoint_eMidRight		,
	BCCGFX_P_AnchorPoint_eLowerLeft		,
	BCCGFX_P_AnchorPoint_eLowerCenter	,
	BCCGFX_P_AnchorPoint_eLowerRight
} BCCGFX_P_AnchorPoint ;



					 
/***************************************************************************
Summary:
	A window's anchor point is declared to be one of the following 9 
	locations.

Description:
	A window's anchor point is declared to be one of the following 9 
	locations.

See Also:
	
****************************************************************************/
typedef struct BCCGFX_P_Rect
{
	int			x ;
	int			y ;
	int			cx ;
	int			cy ;
} BCCGFX_P_Rect ; 


/****************************************************************************
 *                CCGFX (Closed Captioning Graphics) Interface
 * 
 * Scope
 * 
 * This Closed Captioning Graphics Library is used to glue EIA-708 DTVCC
 * aware code to device specific graphics code.  Therefore, this library
 * is aware of both 708 constructs and device specific constructs.
 * 
 *
 * Windows
 * 
 * The primary construct of this graphics library is a window.  The library
 * manages 8 windows.  These windows are not to be confused with the myriad
 * of meanings for 'window' already prevalent; rather, 'window' here refers
 * to the DTVCC EIA-708 notion of a Closed Captioning window.  Moreover,
 * this interface does not impose any device implementation scheme.  That is,
 * when porting this interface to a particular device/system, one is free
 * to choose whatever scheme makes sense for the given architecture.  For
 * example, for a device with overlay capabilities, the implementor may
 * use 1 overlay surface the size of the screen, or 8 smaller ones, one for
 * each window.
 * 
 * Window Identification
 * 
 * Windows are identified by an integer in the range of 0 to 7.  There are
 * always 8 windows and the client of this API manages when to display and
 * hide these windows.  Most functions in the API operate on a particular 
 * window, and therefore, the window ID is passed in as the first argument.
 * 
 * Print Direction/Scroll Direction/Primary Direction
 * 
 * This graphics library is capable of printing and scrolling in all
 * four directions as enumerated by the eDCC_INT_DIRECTION enum.  The term
 * Primary Direction refers to the print direction.
 * 
 * Color and Opacity
 * 
 * The color and opacity arguments to these API functions are in EIA-708 DTVCC
 * terms.  This means that color is a 6-bit quantity; 2 bits each of red, green
 * and blue.  Opacity is defined by the enum eDCC_INT_OPACITY and is one of:
 * oSolid, oFlash, oTransparent and oTranslucent.
 * 
 * Coordinates
 * 
 * All coordinate argments to these functions are in terms of rows and
 * columns.  The safe title area of the screen is divided into 15 rows
 * and either 32 or 42 columns, depending on 4:3 or 16:9 aspect ratios.
 * 
 * Translucent and Transparent Opacities
 * 
 * The oTranslucent opacity is some level of alpha blending that is specifed
 * by the ccgfxSetTranslucentLevel() function.
 * 
 ***************************************************************************/
					/************************
					 *
					 *   Initialization
					 *
					 ************************/

/***************************************************************************
Summary:
	The CCGfx handle to the CCGfx object

Description:
	The CCGfx handle to the CCGfx object

See Also:
	
****************************************************************************/
typedef struct BCCGFX_P_GfxObject *BCCGFX_P_GfxHandle;

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
 
int BCCGFX_P_Open( BCCGFX_P_GfxHandle *phCCGfxHandle,  BDCC_WINLIB_Handle hWinLibHandle,     BDCC_WINLIB_Interface *pWinLibInterface );
int BCCGFX_P_Init( BCCGFX_P_GfxHandle hCCGfxHandle, B_Dcc_Settings * pEngineSettings );

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
void BCCGFX_P_Reset( BCCGFX_P_GfxHandle hCCGfxHandle, B_Dcc_Settings * pEngineSettings );

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
 * BCCGFX_P_Open().
 *
 *****************************************************************/
void BCCGFX_P_Close(BCCGFX_P_GfxHandle hCCGfxHandle);


					/************************
					 *
					 *   Window Commands
					 *
					 ************************/


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
void BCCGFX_P_WndCreate(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		int RowCount, 
		int ColCount);


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
void BCCGFX_P_WndDestroy(BCCGFX_P_GfxHandle hCCGfxHandle, int WndId);


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
void BCCGFX_P_WndResize(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		int RowCount, 
		int ColCount);

					 
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
void BCCGFX_P_WndPosition(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		int rAnchor, 
		int cAnchor, 
		BCCGFX_P_AnchorPoint AnchorPt,
		int nColumns);

					 
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
 * BCCGFX_P_WndShow() changes the given window's display/hide 
 * state according to the given ShowState.
 *
 *****************************************************************/
void BCCGFX_P_WndShow(BCCGFX_P_GfxHandle hCCGfxHandle, int WndId, int ShowState);

					 
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
void BCCGFX_P_WndClear(BCCGFX_P_GfxHandle hCCGfxHandle, int WndId);


					/************************
					 *
					 *   Time
					 *
					 ************************/
void BCCGFX_P_TimeUpdate( BCCGFX_P_GfxHandle hCCGfxHandle, unsigned int * pCurrentTime );
void BCCGFX_P_TimeReset( BCCGFX_P_GfxHandle hCCGfxHandle );
unsigned int BCCGFX_P_TimeGet(BCCGFX_P_GfxHandle hCCGfxHandle);


					/************************
					 *
					 *   Timer
					 *
					 ************************/

/****************************************************************
 *
 * Function:		BCCGFX_P_TimerStart
 *
 * Input:			DelayVal			- delay in 1/10 sec
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function starts the timer with the given
 * interval in tenths of seconds.
 *
 *****************************************************************/
void BCCGFX_P_TimerStart(BCCGFX_P_GfxHandle hCCGfxHandle, unsigned int msDelayVal);


/****************************************************************
 *
 * Function:		BCCGFX_P_TimerTick
 *
 * Input:			
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function cancels the timer.  All subsequent calls to
 * ccgfxTimer_Query will return true.
 *
 *****************************************************************/
void BCCGFX_P_TimerCancel(BCCGFX_P_GfxHandle hCCGfxHandle);

/****************************************************************
 *
 * Function:		BCCGFX_P_TimerQuery
 *
 * Input:			
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function returns non zero (true) if the timer is not
 * started or if the timer has expired.
 *
 *****************************************************************/
unsigned int BCCGFX_P_TimerQuery(BCCGFX_P_GfxHandle hCCGfxHandle);


					/************************
					 *
					 *   Scrolling
					 *
					 ************************/
					 
/****************************************************************
 *
 * Function:		BCCGFX_P_Scroll
 *
 * Input:			WndId			- window identifier
 *
 * Output:
 *
 * Returns:			<void>
 *
 * Description:
 *
 * This function performs a 1-row scroll of the given window.
 * This includes clearing the exposed row.
 *
 *****************************************************************/
void BCCGFX_P_Scroll(BCCGFX_P_GfxHandle hCCGfxHandle, int WndId);

int BCCGFX_P_GetScrollPhase(BCCGFX_P_GfxHandle hCCGfxHandle, int WndId);

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
void BCCGFX_P_FillRow(BCCGFX_P_GfxHandle hCCGfxHandle, int WndId, int row);


/****************************************************************
 *
 * Function:		BCCGFX_P_Periodic
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
 * synch.
 *
 *****************************************************************/
void BCCGFX_P_Periodic(BCCGFX_P_GfxHandle hCCGfxHandle);



					/************************
					 *
					 *   Rendering
					 *
					 ************************/
					 

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
void BCCGFX_P_RenderBegin(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		int Row, 
		int Incremental, 
		int FlashCycleState);
#elif FLASH_BY_2SURFACES
void BCCGFX_P_RenderBegin(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		int Row, 
		int Incremental, 
		int RenderWithFlash);
#else
void BCCGFX_P_RenderBegin(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		int Row, 
		int Incremental);
#endif


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
void BCCGFX_P_RenderChar(BCCGFX_P_GfxHandle hCCGfxHandle, BDCC_UTF32 UTF32_ch);


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
void BCCGFX_P_RenderEnd(BCCGFX_P_GfxHandle hCCGfxHandle, int rAnchor, int cAnchor, BCCGFX_P_AnchorPoint AnchorPt, int nColumns);					 
					 





					/************************
					 *
					 *   Window Attributes   
					 *
					 ************************/

					 
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
void BCCGFX_P_SetWindowFill(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		BDCC_Opacity FillOpacity, 
		unsigned int FillColor) ;

					 
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
void BCCGFX_P_SetPrintDirection(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		BCCGFX_P_Direction PrintDirection);

					 
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
void BCCGFX_P_SetPriority(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		unsigned int Priority) ;

					 
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
void BCCGFX_P_SetScrollDirection(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		BCCGFX_P_Direction ScrollDirection) ;

					 
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
void BCCGFX_P_SetEffect(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		BCCGFX_P_Effect EffectType, 
		BCCGFX_P_Direction EffectDirection, 
		unsigned int EffectSpeed) ;

					 
/****************************************************************
 *
 * Function:		BCCGFX_P_SetBorder
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
void BCCGFX_P_SetBorder(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		BDCC_Edge BorderType, 
		unsigned int BorderColor) ;



/****************************************************************
 *
 * Function:		BCCGFX_P_SetTranslucentLevel
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
void BCCGFX_P_SetTranslucentLevel(BCCGFX_P_GfxHandle hCCGfxHandle, 
		unsigned int TranslucentLevel) ;


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
void BCCGFX_P_SetJustification(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		BCCGFX_P_Justify Justification) ;



					/************************
					 *
					 *   Pen Attributes   
					 *
					 ************************/

					 
/****************************************************************
 *
 * Function:		BCCGFX_P_SetPenFgColor
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
void BCCGFX_P_SetPenFgColor(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		BDCC_Opacity Opacity, 
		unsigned int Color);

/*
** Set the font edge color for the specified window.
*/
void BCCGFX_P_SetPenEdgeColor( BCCGFX_P_GfxHandle hCCGfxHandle,  int WndId, unsigned int Color );


/*
** Set the edge type for the  specified window.
*/
void BCCGFX_P_SetPenEdgeType( BCCGFX_P_GfxHandle hCCGfxHandle, int WndId, BDCC_Edge edgeType );

					 
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
void BCCGFX_P_SetPenBgColor(BCCGFX_P_GfxHandle hCCGfxHandle,
		int WndId, 
		BDCC_Opacity Opacity, 
		unsigned int Color);

					 
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
void BCCGFX_P_SetPenSize(BCCGFX_P_GfxHandle hCCGfxHandle, 
		int WndId, 
		BDCC_PenSize PenSize) ;

					 
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
void BCCGFX_P_SetPenFontStyle(BCCGFX_P_GfxHandle hCCGfxHandle,
		int WndId, 
		BDCC_FontStyle FontStyle) ;

					 
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
void BCCGFX_P_SetPenItalics(BCCGFX_P_GfxHandle hCCGfxHandle,
		int WndId, 
		int Italics) ;

					 
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
void BCCGFX_P_SetPenUnderline(BCCGFX_P_GfxHandle hCCGfxHandle,
		int WndId, 
		int Underline) ;

/****************************************************************
 *
 * Function:		BCCGFX_P_RedrawScreen
 *
 * Description:
 *
 * This function causes the screen to be redrawn with the most recent CC data.
 *
 *****************************************************************/
void BCCGFX_P_RedrawScreen(BCCGFX_P_GfxHandle hCCGfxHandle );

/****************************************************************
 *
 * Function:		BCCGFX_P_GetShowState
 *
 * Description:
 *
 * This function returns the "ShowState" data.
 *
 *****************************************************************/
int BCCGFX_P_GetShowState( BCCGFX_P_GfxHandle hCCGfxHandle, int WndId );

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
void BCCGFX_P_SetDirtyState( BCCGFX_P_GfxHandle hCCGfxHandle, int WndId, bool bFlushState );

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
bool BCCGFX_P_GetDirtyState( BCCGFX_P_GfxHandle hCCGfxHandle, int WndId  );


#ifdef __cplusplus
}
#endif

#endif /* BDCCGFX_H */

