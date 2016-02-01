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
#ifndef BCCWINLIB_H
#define BCCWINLIB_H

#include "b_api_shim.h"
#include "bdcc.h"
#include "bdcc_cfg.h"
#include "nexus_surface.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_GetInterface
 *
 * Input:           	
 *
 * Output:          winlibIf             - structure of function pointers
 *                                       representing the winlib interface
 *
 * Returns:         void
 *
 * Description:		Used to retrieve the winlib interface
 *
 ******************************************************************************/
void BDCC_WINLIB_GetInterface(
    BDCC_WINLIB_Interface *winlibInterface
    );
    
typedef struct BDCC_WINLIB_OpenSettings
{
    /* display callbacks allow library to port to variety of environments */
    NEXUS_SurfaceHandle (*flip)(void *context); /* display framebuffer returned by flip (unless first call) and return the next framebuffer for rendering. */
    void (*get_framebuffer_dimensions)(void *context, NEXUS_VideoFormat *format, unsigned *width, unsigned *height); /* get current display format information */
    void *context; /* user context passed back into callbacks */
    
    uint16_t framebufferWidth;
    uint16_t framebufferHeight;
    void *bwinEngine; /* unused */
} BDCC_WINLIB_OpenSettings;

void BDCC_WINLIB_GetDefaultOpenSettings(
    BDCC_WINLIB_OpenSettings *pSettings
    );

/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_Open
 *
 * Input:           hWinLibHandle 	- handle to the newly created window library	
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:		Creates a new caption window library instance.
 *
 *                  On completion of this function the following components
 *                  should be available....
 *
 *                  Frame buffer
 *                  All of the caption fonts should be available.
 *
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_Open(
    BDCC_WINLIB_Handle *phWinLibHandle,
    const BDCC_WINLIB_OpenSettings *pSettings
    );

/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_CreateCaptionRow
 *
 * Input:           hWinLibHandle	- handle to the window library
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:		Closes an instance of a caption window library and releases
 *                  all resources
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_Close(
    BDCC_WINLIB_Handle phWinLibHandle
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_CreateCaptionRow
 *
 * Input:           hWinLibHandle	- handle to the window library
 *
 * Output:          row             - handle to the newly created row surface
 *
 * Returns:         currently unspecified error code
 *
 * Description:     Creates a new caption row surface. The surface is managed
 *                  exclusively by the window library therefore the window
 *                  library is free to pick any surface pixel format and size
 *                  it wishes with the following restrictions.
 *
 *                  1. The surface must support at least 3 levels of alpha
 *                  Since it must support opaque, translucent and transparent
 *                  character foregrounds, character backgrounds and
 *                  character edges.
 *
 *                  2. The surface must be sized to allow a minimum of 32
 *                  columns of text at standard pen size for 4:3 displays 
 *                  and a minimum of 42 columns of text at standard pen size
 *                  for 16:9 displays. The destination rectangle for this 
 *                  caption row on the framebuffer must be less than 1/15th
 *                  of the height of the "Safe Title Area" of the display
 *                  and must be less than or equal to the width of the 
 *                  "Safe Title Area". The "Safe Title Area" is defined to be
 *                  a rectangle of 80% height and 80% width of the display
 *                  centered on the display
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_CreateCaptionRow(
    BDCC_WINLIB_Handle hWinLibHandle,
    BDCC_WINLIB_hRow *row
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_DestroyCaptionRow
 *
 * Input:           row             - handle to the caption row surface
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:     Instructs the window library that the caption row surface
 *                  will no longer be used and the resources can be reclaimed
 *                  if desired. Currently the caption library maintains a pool
 *                  or row surfaces that it cycles through therefore caption
 *                  rows are infrequently destroyed
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_DestroyCaptionRow(
    BDCC_WINLIB_hRow row
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_ClearCaptionRow
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  color           - caption window fill color for this row
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:     Specifies that the caption row surface should cleared with
 *                  the caption window fill color and the cursor position be 
 *                  returned to the start of the row
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_ClearCaptionRow(
    BDCC_WINLIB_hRow row,
    BDCC_Opacity opacity,
    uint8_t color
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_SetCharForegroundColor
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  ForeGroundColor - Character foreground color in R2 G2 B2 
 *
 *                  Opacity	        - Character foreground opacity
 *
 *                                      0 = Solid
 *                                      1 = Translucent
 *                                      2 = Flashing,
 *                                      3 = Transparent
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:     Specifies the forground color and opacity used to render 
 *                  characters.
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCharFGColor(
    BDCC_WINLIB_hRow row,
    uint8_t ForeGroundColor,
    BDCC_Opacity Opacity
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_SetCharBackgroundColor
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  ForeGroundColor - Character background color in R2 G2 B2 
 *
 *                  Opacity         - Character background opacity
 *
 *                                      0 = Solid
 *                                      1 = Translucent
 *                                      2 = Flashing,
 *                                      3 = Transparent
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:     Specifies the background color and opacity used to render 
 *                  characters.
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCharBGColor(
    BDCC_WINLIB_hRow row,
    uint8_t BackGroundColor,
    BDCC_Opacity Opacity
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_SetCharEdgeColor
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  EdgeColor       - Character edge color in format R2 G2 B2
 *
 *                  Opacity         - Character background opacity
 *
 *                                      0 = Solid
 *                                      1 = Translucent
 *                                      2 = Flashing,
 *                                      3 = Transparent
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:     Specifies the edge color used to render characters edges.
 *                  The alpha component of the character foreground color and
 *                  the alpha component of the character edge color will be
 *                  identical
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCharEdgeColor(
    BDCC_WINLIB_hRow row,
    uint8_t EdgeColor,
    BDCC_Opacity Opacity
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_SetCharEdgeType
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  EdgeType        - one of NONE, DEPRESSED, UNIFORM, RAISED,
 *                                  LEFT_DROP_SHADOW, RIGHT_DROP_SHADOW
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:		Specifies an edge type to be used when rendering subsequent
 *                  characters. It is mandatory to support all of the edge types
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCharEdgeType(
    BDCC_WINLIB_hRow row,
    BDCC_Edge EdgeType
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_SetFont
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  fontStyle       - one of DEFAULT, MONO_SERIF, PROP_SERIF
 *                                  MONO_SANS, PROP_SANS, CASUAL, CURSIVE,
 *                                  SMALL_CAPITALS
 *
 *                  penSize         - One of SMALL, STANDARD, LARGE
 *
 *                  penStyle        - One of NORMAL, ITALICS, UNDERLINE, 
 *                                  ITALICS_UNDERLINE
 *
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:		Specifies the Font Style, Pen Style and Pen Size that should
 *                  be used for rendering subsequent characters. Note it is 
 *                  mandatory to support all of the font styles, pen styles
 *                  and pen sizes
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_SetFont(
    BDCC_WINLIB_hRow row,
    BDCC_FontStyle fontStyle,
    BDCC_PenSize penSize,
    BDCC_PenStyle penStyle
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_SetZorder
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  Zorder	        - Z-order of the caption row [0 - 7]
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:		Specifies the Z-order of the caption window [0 - 7] to
 *                  to which this caption row belongs
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCaptionRowZorder(
    BDCC_WINLIB_hRow row,
    uint32_t zorder
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_SetRowClipRect
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  pRect           - Specifies the region of the caption
 *                                  row surface that should displayed.
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:		Specifies  the clip rectangle of the caption row surface
 *
 *                  This is used for example to increase the displayed extent
 *                  of the caption row as it is scrolled onto the bottom of
 *                  the caption window and to reduce the displayed extent of 
 *                  the caption row as it is scrolled off of the top of the 
 *                  caption window.
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCaptionRowClipRect(
    BDCC_WINLIB_hRow row,
    const BDCC_WINLIB_Rect *pRect
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_SetDisplayRect
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  pRect           - Specifies the co-ordinates and size
 *                                  of the region onto which the caption
 *                                  row clip rectangle will be displayed
 *
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:     Specifies the co-ordinates and size of the display rectangle
 *                  that corresponds to the clip rectangle for a row surface
 *                  (see BDCC_WINLIB_P_SetRowClipRect). 
 *                  The size of the display rectangle and clip rectangle may
 *                  differ according to a scale factor specified at init time.
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCaptionRowDispRect(
    BDCC_WINLIB_hRow row,
    const BDCC_WINLIB_Rect *pRect
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_GetDisplayRect
 *
 * Input:           row             - handle to the caption row surface
 *
 * Output:          pRect           - reports the co-ordinates and size
 *                                  of the region onto which the caption
 *                                  row clip rectangle will be displayed
 *
 * Returns:         currently unspecified error code
 *
 * Description:		Reports the co-ordinates and size of the display rectangle
 *                  that corresponds to the clip rectangle for a row surface
 *                  (see BDCC_WINLIB_P_SetRowClipRect). 
 *                  The size of the display rectangle and clip rectangle may
 *                  differ according to a scale factor specified at init time.
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_GetDisplayRect(
    BDCC_WINLIB_hRow row,
    BDCC_WINLIB_Rect *pRect
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_GetCaptionRowTextRect
 *
 * Input:           row             - handle to the caption row surface
 *
 * Output:          pRect           - reports the co-ordinates and size
 *                                  of the region that has been rendered into
 *                                  in this row
 *
 * Returns:         currently unspecified error code
 *
 * Description:     Reports the co-ordinates and size of the rectangle
 *                  that bounds the text in the caption row
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_GetCaptionRowTextRect(
    BDCC_WINLIB_hRow row,
    BDCC_WINLIB_Rect *pRect
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_GetSurfaceRect
 *
 * Input:           row             - handle to the caption row surface
 *
 * Output:          width           - width of the underlying graphics surface
 *
 *                  height          - height of the underlying graphics surface
 *
 * Returns:         currently unspecified error code
 *
 * Description:     reports the width and height of the underlying graphics
 *                  surface corresponding to a caption row
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_GetSurfaceRect(
    BDCC_WINLIB_hRow row,
    uint32_t *width,
    uint32_t *height
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_GetMaxBoundingRect
 *
 * Input:           row             - handle to the caption row surface.
 *
 *                  numColumns      - the number of columns for which the
 *                                  max bounding rectangle has been requested
 *
 *                  penSize	        - the pen size for which the max bounding
 *                                  rectangle has been requested
 *
 * Output:          pRect           - the bounding rectangle that accomodates
 *                                  the the specified number of columns of the
 *                                  widest glyph at the specified pen size
 *
 *
 * Returns:         currently unspecified error code.
 *
 * Description:     reports a rectangle that is guaranteed to accomodate
 *                  'numColumns' of glyphs from any caption font at the
 *                  'penSize' pensize. This is used to calculate the window
 *                  width by the caption library
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_GetMaxBoundingRect(
    BDCC_WINLIB_hRow row,
    uint32_t numColumns,
    BDCC_PenSize penSize,
    BDCC_WINLIB_Rect *pRect
    );

/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_SetFrameBufferSize
 *
 * Input:           hWinLibHandle   - handle to the window library
 *
 *                  width           - width of the safe title area
 *
 *                  height          - height of the safe title area
 *
 * Returns:         currently unspecified error code
 *
 * Description:     Set the width and height of the safe title area.
 *                  This area typically occupies the center 80%W x 80%H of
 *                  the display. All captions must fall within the safe title
 *                  area. Captions nominally falling outside will be clipped
 *                  by the caption library
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_SetFrameBufferSize(
    BDCC_WINLIB_Handle hWinLibHandle,
    uint32_t width,
    uint32_t height
    );

/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_GetFrameBufferSize
 *
 * Input:           hWinLibHandle   - handle to the window library
 *
 * Output:          width           - width of the safe title area
 *
 *                  height          - height of the safe title area
 *
 * Returns:         currently unspecified error code
 *
 * Description:     reports the width and height of the safe title area.
 *                  This area typically occupies the center 80%W x 80%H of 
 *                  the display. All captions must fall within the safe title
 *                  area. Captions nominally falling outside will be clipped
 *                  by the caption library
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_GetFrameBufferSize(
    BDCC_WINLIB_Handle hWinLibHandle,
    uint32_t *width,
    uint32_t *height
    );


/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_HideDisp
 *
 * Input:           hWinLibHandle	- handle to the window library
 *
 *                  hide            - true = hide display, false show display
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:		This API allows sharing of framebuffers with an associated
 *                  application. It can be used to disable/enable rendering of
 *                  closed captions into the shared framebuffers. One example
 *                  of this strategy is to disable closed caption rendering
 *                  whilst the application is using the framebuffer to display
 *                  its UI. When the application UI is cleared, closed caption
 *                  rendering may be resumed
 *
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_HideDisp(
    BDCC_WINLIB_Handle hWinLibHandle,
    bool hide
    );


/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_UpdateScreen
 *
 * Input:           hWinLibHandle	- handle to the window library
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:		Hints to the window library that an event (character render)
 *                  flash state change or scroll position change has occured
 *                  that requires an update of the screen. 
 *
 *                  NOTE: Not all character renders require the display to be
 *                  updated. In general, for left justified text, each character
 *                  should be displayed as soon as possible after it has been
 *                  rendered. For other justifications, rendering need only
 *                  occur on receipt of a FF, CR or a special control char ETX.
 *
 *                  The window library may choose to implement enhanced tracking
 *                  of which of the surface elements needs to be refreshed by
 *                  for example maintaining 'dirty' flags for each surface and
 *                  maintaining internal clip rectangles for dirty areas within
 *                  surfaces.
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_UpdateScreen(
    BDCC_WINLIB_Handle hWinLibHandle
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_Hide
 *
 * Input:           row             - handle to the caption row surface
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:     Specifies whether the caption row falls completely outside
 *                  of the safe title area (in which case it should be marked
 *                  as invisible) or not
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_SetClipState(
    BDCC_WINLIB_hRow row,
    bool clipState );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_Show
 *
 * Input:           row             - handle to the caption row surface
 *                  visible         - show/hide the caption row
 *
 * Output:
 *
 * Returns:         currently unspecified error code
 *
 * Description:     Specifies the visibility of the caption row
 *
 ******************************************************************************/
BDCC_WINLIB_ErrCode BDCC_WINLIB_SetCaptionRowVisibility(
    BDCC_WINLIB_hRow row,
    bool visibility
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_IsShown
 *
 * Input:           row             - handle to the caption row surface
 *
 * Output:
 *
 * Returns:         'true' if the caption row surface is currently visible
 *                  otherwise 'false'
 *
 * Description:     Reports the caption row surface visibility
 *
 ******************************************************************************/
int BDCC_WINLIB_IsCaptionRowVisible(
    BDCC_WINLIB_hRow row
    );




#if FLASH_BY_2SURFACES
/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_RenderStart
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  flashrow        - handle to the caption row flash surface
 *
 *                  justification   - current text justification of the window
 *                                  to which this row belongs
 *
 * Output:
 *
 * Returns:
 *
 * Description:		Indicates that character rendering is about to commence
 *                  from the start of a clean row. Invocation of this function 
 *                  will be preceded by a call to 
 *                  BDCC_WINLIB_P_ClearCaptionRow in order to clear out the
 *                  row. Character rendering should commence from the leftmost
 *                  column (left justification), center (center justification)
 *                  or rightmost column (right justification)
 *
 ******************************************************************************/
void BDCC_WINLIB_RenderStart(
    BDCC_WINLIB_hRow row,
    BDCC_WINLIB_hRow flashrow,
    BDCC_Justify justification
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_RenderChar
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  flashrow        - handle to the caption row flash surface
 *
 *                  ch              - UTF32 (unicode) character
 *
 * Output:
 *
 * Returns:
 *
 * Description:		Initiates the rendering of a single glyph based on the
 *                  character rendering attributes previously specified.
 *                  Characters need not be displayed until 
 *                  BDCC_WINLIB_P_RenderEnd is called.
 *
 *                  Many calls to BDCC_WINLIB_P_RenderChar may be made
 *                  before BDCC_WINLIB_P_RenderEnd is called.
 *
 ******************************************************************************/
int BDCC_WINLIB_RenderChar(
    BDCC_WINLIB_hRow row,
    BDCC_WINLIB_hRow flashrow,
    BDCC_UTF32 ch
    );



/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_RenderEnd
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  flashrow        - handle to the caption row flash surface
 *
 * Output:
 *
 * Returns:
 *
 * Description:		Upon invocation of BDCC_WINLIB_P_RenderEnd,
 *                  all of the glyphs rendered since the previous call to 
 *                  BDCC_WINLIB_P_RenderEnd or BDCC_WINLIB_P_RenderStart
 *                  should be made visible.
 *
 *                  In the case of center justified or right justified text
 *                  it is usual for BDCC_WINLIB_P_RenderEnd to be called
 *                  only when the complete caption row is rendered.
 *                  This reduces the inefficiencies inherent in displaying
 *                  and repositioning center or right justified text on
 *                  a character by character basis.
 *
 *                  for right justified text is is usual for this function
 *                  to be called after each glyph is rendered
 *
 ******************************************************************************/
void BDCC_WINLIB_RenderEnd(
    BDCC_WINLIB_hRow row,
    BDCC_WINLIB_hRow flashrow
    );



#else
/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_RenderStart
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  justification	- current text justification of the window
 *                                  to which this row belongs
 *
 * Output:
 *
 * Returns:
 *
 * Description:		Indicates that character rendering is about to commence
 *                  from the start of a clean row. Invocation of this function 
 *                  will be preceded by a call to 
 *                  BDCC_WINLIB_P_ClearCaptionRow in order to clear out the
 *                  row. Character rendering should commence from the leftmost
 *                  column (left justification), center (center justification)
 *                  or rightmost column (right justification)
 *
 ******************************************************************************/
void BDCC_WINLIB_RenderStart(
    BDCC_WINLIB_hRow row,
    BDCC_Justify justification
    );


#if FLASH_BY_RERENDER
/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_RenderChar
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  flashCycleState - visible/invisible condition of the flash
 *
 *                  ch	            - UTF32 (unicode) character
 *
 * Output:
 *
 * Returns:
 *
 * Description:		Initiates the rendering of a single glyph based on the
 *                  character rendering attributes previously specified.
 *                  Characters need not be displayed until 
 *                  BDCC_WINLIB_P_RenderEnd is called.
 *
 *                  Many calls to BDCC_WINLIB_P_RenderChar may be made
 *                  before BDCC_WINLIB_P_RenderEnd is called.
 *
 ******************************************************************************/
int BDCC_WINLIB_RenderChar(
    BDCC_WINLIB_hRow row,
    int flashCycleState,
    BDCC_UTF32 ch
    );

#else
/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_RenderChar
 *
 * Input:           row             - handle to the caption row surface
 *
 *                  ch	            - UTF32 (unicode) character
 *
 * Output:
 *
 * Returns:
 *
 * Description:		Initiates the rendering of a single glyph based on the
 *                  character rendering attributes previously specified.
 *                  Characters need not be displayed until 
 *                  BDCC_WINLIB_P_RenderEnd is called.
 *
 *                  Many calls to BDCC_WINLIB_P_RenderChar may be made
 *                  before BDCC_WINLIB_P_RenderEnd is called.
 *
 ******************************************************************************/
int BDCC_WINLIB_RenderChar(
    BDCC_WINLIB_hRow row,
    BDCC_UTF32 ch
    );
#endif


/*******************************************************************************
 *
 * Function:        BDCC_WINLIB_P_RenderEnd
 *
 * Input:           row             - handle to the caption row surface
 *
 * Output:
 *
 * Returns:
 *
 * Description:		Upon invocation of BDCC_WINLIB_P_RenderEnd,
 *                  all of the glyphs rendered since the previous call to 
 *                  BDCC_WINLIB_P_RenderEnd or BDCC_WINLIB_P_RenderStart
 *                  should be made visible.
 *
 *                  In the case of center justified or right justified text
 *                  it is usual for BDCC_WINLIB_P_RenderEnd to be called
 *                  only when the complete caption row is rendered.
 *                  This reduces the inefficiencies inherent in displaying
 *                  and repositioning center or right justified text on
 *                  a character by character basis.
 *
 *                  for right justified text is is usual for this function
 *                  to be called after each glyph is rendered
 *
 ******************************************************************************/
void BDCC_WINLIB_RenderEnd(
    BDCC_WINLIB_hRow row
    );



#endif


BDCC_WINLIB_ErrCode BDCC_WINLIB_LoadFont(
    BDCC_WINLIB_Handle hWinLibHandle,
    const char *pFontFile,
    uint32_t fontSize,
    BDCC_FontStyle fontStyle,
    BDCC_PenSize penSize,
    BDCC_PenStyle penStyle
    );



BDCC_WINLIB_ErrCode BDCC_WINLIB_UnloadFonts(
    BDCC_WINLIB_Handle hWinLibHandle
    );


#ifdef __cplusplus
}
#endif

#endif /* BCCWINLIB_H */

