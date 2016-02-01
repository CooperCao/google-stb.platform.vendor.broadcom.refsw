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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BDCC_H
#define BDCC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bdcc_cfg.h"
#include "bstd_defs.h"

#define FCC_MIN_DECODER

typedef enum BDCC_608_Service
{
    BDCC_Min_608_Serivce     = 1,
    BDCC_Max_608_Service    = 4
    
} BDCC_608_Service;


typedef enum BDCC_708_Service
{
    BDCC_Min_708_Serivce     = 1,
    BDCC_Max_708_Service    = 63
    
} BDCC_708_Service;

/*
** The EIA standard defines the following pen sizes.
*/
typedef enum BDCC_PenSize
{
	BDCC_PenSize_Small		= 0,
	BDCC_PenSize_Standard	= 1,
	BDCC_PenSize_Large		= 2,
	BDCC_PenSize_Max_Size	= 3
	
} BDCC_PenSize ;

/*
** The EIA standard defines the following pen styles.
*/
typedef enum BDCC_PenStyle
{
	BDCC_PenStyle_Standard			= 0,
	BDCC_PenStyle_Italics			= 1,
	BDCC_PenStyle_Underline			= 2,
	BDCC_PenStyle_Italics_Underline	= 3,
	BDCC_PenStyle_Max_Pen_Style		= 4

} BDCC_PenStyle ;


/*
** The EIA-708 standard specifies 8 font styles. 
*/
typedef enum BDCC_FontStyle
{
	BDCC_FontStyle_Default		= 0,
	BDCC_FontStyle_MonoSerifs	= 1,
	BDCC_FontStyle_PropSerifs	= 2,
	BDCC_FontStyle_Mono		= 3,
	BDCC_FontStyle_Prop		= 4,
	BDCC_FontStyle_Casual		= 5,
	BDCC_FontStyle_Cursive		= 6,
	BDCC_FontStyle_SmallCaps	= 7,
	BDCC_FontStyle_Max_Value    = 8
	
} BDCC_FontStyle ;

/*
** Pen and window colors have an associated opacity, enum'ed here.
** The Translucent opacity is alpha blending with some unspecified alpha value.
*/
typedef enum BDCC_Opacity
{
	BDCC_Opacity_Solid			= 0,
	BDCC_Opacity_Flash			= 1,
	BDCC_Opacity_Translucent	= 2,
	BDCC_Opacity_Transparent	= 3
	
} BDCC_Opacity ;

/*
** The EIA-708 standard defines the following edge (outline) styles. 
*/
typedef enum BDCC_Edge
{
	BDCC_Edge_Style_None				= 0,
	BDCC_Edge_Style_Raised			= 1,
	BDCC_Edge_Style_Depressed			= 2,
	BDCC_Edge_Style_Uniform			= 3,
	BDCC_Edge_Style_LeftDropShadow	= 4,
	BDCC_Edge_Style_RightDropShadow	= 5,
	BDCC_Edge_Style_Max_Value
	
} BDCC_Edge ;

/*
** The EIA-708 standard specifies 4 text justifications. 
*/
typedef enum BDCC_Justify
{
	BDCC_Justify_eLeft	= 0,
	BDCC_Justify_eRight	= 1,
	BDCC_Justify_eCenter= 2, 
	BDCC_Justify_eFull	= 3
} BDCC_Justify ;


/*
** The EIA-708 standard specifies 4 print directions. 
*/
typedef enum BDCC_Direction
{
	BDCC_Direction_eLeftToRight	= 0,
	BDCC_Direction_eRightToLeft	= 1,
	BDCC_Direction_eTopToBottom	= 2,
	BDCC_Direction_eBottomToTop	= 3
} BDCC_Direction ;

/*
** Display mode constants.
*/
typedef enum BDCC_WINLIB_Display_Modes
{
	BDCC_WINLIB_Display_480i,
	BDCC_WINLIB_Display_480p,
	BDCC_WINLIB_Display_720p,
	BDCC_WINLIB_Display_1080i,
	BDCC_WINLIB_Display_1080p,	
	BDCC_WINLIB_Display_Max_Size

}BDCC_WINLIB_Display_Modes;

/*
** The default edge width, this be overridden by the "application".
*/
#define BDCC_Edge_Width 1

/*
** The following attributes can be overriden by the viewer.
*/
typedef struct B_Dcc_OverRides
{
	BDCC_PenSize	PenSize ;
	BDCC_FontStyle	FontStyle ;    
	unsigned int    WinColor ;     
	BDCC_Opacity	WinOpacity ;    
	unsigned int		FgColor ;     
	BDCC_Opacity	FgOpacity ;    
	unsigned int		BgColor ;  
	BDCC_Opacity	BgOpacity ;  
	unsigned int		EdgeColor ;  
	BDCC_Edge	EdgeType ;  
	BDCC_PenStyle	PenStyle;

} B_Dcc_OverRides ;

/*
** EIA specified a color format of RGB222
*/ 
typedef unsigned char BDCC_Color;

/*
** UTF32 unicode format uses 32 bits
*/ 
typedef unsigned int BDCC_UTF32;

/*
** Per the EIA documentation: if CC data is not received for 30 seconds,
** the screen should be cleared.
*/
#define BDCC_Data_Timeout_MSecs ( 1000 * 30 )

/*
** The following masks are used to manage attributes specified both
** in the MPEG stream and by the "viewer".
*/
#define BDCC_ATTR_MASK_NONE				0x00000000

#define BDCC_ATTR_MASK_ANCHOR			0x00000001
#define BDCC_ATTR_MASK_BORDER				0x00000002
#define BDCC_ATTR_MASK_CLEARWINDOW		0x00000004
#define BDCC_ATTR_MASK_DEFINED_CLR		0x00000008
#define BDCC_ATTR_MASK_DEFINED_SET		0x00000010
#define BDCC_ATTR_MASK_EDGECOLOR			0x00000020
#define BDCC_ATTR_MASK_EDGETYPE			0x00000040
#define BDCC_ATTR_MASK_EFFECT				0x00000080
#define BDCC_ATTR_MASK_FILL				0x00000100
#define BDCC_ATTR_MASK_FONTSTYLE			0x00000200
#define BDCC_ATTR_MASK_ITALICS			0x00000400
#define BDCC_ATTR_MASK_JUSTIFY			0x00000800
#define BDCC_ATTR_MASK_LOCK				0x00001000
#define BDCC_ATTR_MASK_PENBG				0x00002000
#define BDCC_ATTR_MASK_PENFG				0x00004000
#define BDCC_ATTR_MASK_PENLOC				0x00008000
#define BDCC_ATTR_MASK_PENSIZE			0x00010000
#define BDCC_ATTR_MASK_PENSTYLEID		0x00020000
#define BDCC_ATTR_MASK_PRINTDIR			0x00040000
#define BDCC_ATTR_MASK_RCCOUNT			0x00080000
#define BDCC_ATTR_MASK_SCROLLDIR			0x00100000
#define BDCC_ATTR_MASK_UNDERLINE			0x00200000
#define BDCC_ATTR_MASK_VISIBLE			0x00400000
#define BDCC_ATTR_MASK_WNDSTYLEID		0x00800000
#define BDCC_ATTR_MASK_WORDWRAP			0x01000000
#define BDCC_ATTR_MASK_PENBGOP			0x02000000
#define BDCC_ATTR_MASK_PENFGOP			0x04000000
#define BDCC_ATTR_MASK_FILLOP			0x08000000





/********************************************
 *
 * Enum:			DCCERR
 *
 * Description:
 *
 *   DCCERR enumerates the error codes returned
 *   by the collection of DCC Libraries.
 *
 ********************************************/
 typedef enum BDCC_Error
 {
	BDCC_Error_eSuccess = 0,
	BDCC_Error_eObjectNotInitialized,
	BDCC_Error_eInvalidParameter,
	BDCC_Error_eOutputBufTooSmall,
	BDCC_Error_eNullPointer,
	BDCC_Error_eBadOutputType,
	BDCC_Error_eBufferOverflow,
	BDCC_Error_eArgOutOfRange,
	BDCC_Error_eSequence,
	BDCC_Error_eWrnPause,
	BDCC_Error_eNoMemory
	
 } BDCC_Error ;

 
/********************************************
 *
 * Structure:		BDCC_OutputInfo
 *
 * Description:
 *
 *   The BDCC_OutputInfo structure holds the
 *   information related to an output buffer.
 *
 ********************************************/
typedef struct BDCC_OutputInfo
{
	/*
	 * pOutputBuf
	 *
	 * This points to a buffer into which
	 * DccXxx_Process() will copy output bytes.
	 */
	unsigned char *			pOutputBuf ;

	/*
	 * OutputBufSize
	 *
	 * The size in bytes of the buffer pointed
	 * to by pOutputBuf.
	 */
	unsigned int			OutputBufSize ;

	/*
	 * OutputBytesProduced
	 *
	 * the DccXxx_Process() function will update this
	 * field, indicating how many bytes were copied
	 * to the output buffer.
	 */
	unsigned int			OutputBytesProduced ;

	/*
	 * DccError
	 *
	 * Recoverable errors related to processing for
	 * a particular output are reported here.
	 */
	BDCC_Error				DccError ;
	
} BDCC_OutputInfo ;


/***************************************************************************
Summary:
	The CCGfx handle to the CCGfx object

Description:
	The CCGfx handle to the CCGfx object

See Also:
	
****************************************************************************/
typedef struct BDCC_WINLIB_Object *BDCC_WINLIB_Handle;



/***************************************************************************
Summary:
	The handle to the winlib row

Description:
	The handle to the winlib row

See Also:
	
****************************************************************************/
typedef struct BDCC_WINLIB_Row *BDCC_WINLIB_hRow;



/***************************************************************************
Summary:
	Winlib errors

Description:
	Winlib errors

See Also:
	
****************************************************************************/
typedef enum BDCC_WINLIB_ErrCode
{
	BDCC_WINLIB_SUCCESS,
	BDCC_WINLIB_FAILURE,
	BDCC_WINLIB_ERROR_NO_MEMORY
} BDCC_WINLIB_ErrCode;



/***************************************************************************
Summary:
	Winlib rectangle

Description:
	Winlib rectangle

See Also:
	
****************************************************************************/
typedef struct BDCC_WINLIB_Rect
{
	unsigned int x; /* can be < 0 */
	unsigned int y; /* can be < 0 */
	unsigned int width;
	unsigned int height;
} BDCC_WINLIB_Rect;


typedef struct BDCC_WINLIB_Interface
{

	/*
	 * Callback functions to customer window library
	 */

	BDCC_WINLIB_ErrCode (*CreateCaptionRow)(
		BDCC_WINLIB_Handle hWinLibHandle, 
		BDCC_WINLIB_hRow *row
		);

	BDCC_WINLIB_ErrCode (*DestroyCaptionRow)(
		BDCC_WINLIB_hRow row
		);

	BDCC_WINLIB_ErrCode (*ClearCaptionRow)(
		BDCC_WINLIB_hRow row,
		BDCC_Opacity opacity,
		unsigned char color
		);

	BDCC_WINLIB_ErrCode (*SetCharFGColor)(
		BDCC_WINLIB_hRow row,
		unsigned char ForeGroundColor,
		BDCC_Opacity Opacity
		);

	BDCC_WINLIB_ErrCode (*SetCharBGColor)(
		BDCC_WINLIB_hRow row,
		unsigned char BackGroundColor,
		BDCC_Opacity Opacity
		);

	BDCC_WINLIB_ErrCode (*SetCharEdgeColor)(
		BDCC_WINLIB_hRow row,
		unsigned char EdgeColor,
		BDCC_Opacity Opacity
		);

	BDCC_WINLIB_ErrCode (*SetCharEdgeType)(
		BDCC_WINLIB_hRow row,
		BDCC_Edge EdgeType
		);

	BDCC_WINLIB_ErrCode (*SetFont)(
		BDCC_WINLIB_hRow row,
		BDCC_FontStyle fontStyle,
		BDCC_PenSize penSize,
		BDCC_PenStyle penStyle
		);

	BDCC_WINLIB_ErrCode (*SetCaptionRowZorder)(
		BDCC_WINLIB_hRow row,
		unsigned int zorder
		);

	BDCC_WINLIB_ErrCode (*SetCaptionRowClipRect)(
		BDCC_WINLIB_hRow row,
		const BDCC_WINLIB_Rect *pRect
		);

	BDCC_WINLIB_ErrCode (*SetCaptionRowDispRect)(
		BDCC_WINLIB_hRow row,
		const BDCC_WINLIB_Rect *pRect
		);

	BDCC_WINLIB_ErrCode (*GetCaptionRowTextRect)(
		BDCC_WINLIB_hRow row,
		BDCC_WINLIB_Rect *pRect
		);


	BDCC_WINLIB_ErrCode (*GetDisplayRect)(
		BDCC_WINLIB_hRow row,
		BDCC_WINLIB_Rect *pRect
		);

	BDCC_WINLIB_ErrCode (*GetSurfaceRect)(
		BDCC_WINLIB_hRow row,
		unsigned int *width,
		unsigned int *height
		);

	BDCC_WINLIB_ErrCode (*GetMaxBoundingRect)(
		BDCC_WINLIB_hRow row,
		unsigned int numColumns,
		BDCC_PenSize penSize,
		BDCC_WINLIB_Rect *pRect
		);

	BDCC_WINLIB_ErrCode (*GetFrameBufferSize)(
		BDCC_WINLIB_Handle hWinLibHandle,
		unsigned int *width,
		unsigned int *height
		);

	BDCC_WINLIB_ErrCode (*SetFrameBufferSize)(
		BDCC_WINLIB_Handle hWinLibHandle,
		unsigned int width,
		unsigned int height
		);
    BDCC_WINLIB_ErrCode (*HideDisp)(
        BDCC_WINLIB_Handle hWinLibHandle,
        bool hide
        );

	BDCC_WINLIB_ErrCode (*UpdateScreen)(
		BDCC_WINLIB_Handle hWinLibHandle
		);

	BDCC_WINLIB_ErrCode (*SetClipState)(
		BDCC_WINLIB_hRow row,
		bool clipState );

	BDCC_WINLIB_ErrCode (*SetCaptionRowVisibility)(
		BDCC_WINLIB_hRow row,
		bool visible
		);

	int (*IsCaptionRowVisible)(
		BDCC_WINLIB_hRow row
		);

#if FLASH_BY_2SURFACES
	void (*RenderStart)(
		BDCC_WINLIB_hRow row,
		BDCC_WINLIB_hRow flashrow,
		BDCC_Justify justification
		);

	int (*RenderChar)(
		BDCC_WINLIB_hRow row,
		BDCC_WINLIB_hRow flashrow,
		BDCC_UTF32 ch
		);

	void (*RenderEnd)(
		BDCC_WINLIB_hRow row,
		BDCC_WINLIB_hRow flashrow
		);

#else
	void (*RenderStart)(
		BDCC_WINLIB_hRow row,
		BDCC_Justify justification
		);

#if FLASH_BY_RERENDER
	int (*RenderChar)(
		BDCC_WINLIB_hRow row,
		int flashCycleState,
		BDCC_UTF32 ch
		);
#else
	int (*RenderChar)(
		BDCC_WINLIB_hRow row,
		BDCC_UTF32 ch
		);
#endif
	void (*RenderEnd)(
		BDCC_WINLIB_hRow row
		);
#endif

}BDCC_WINLIB_Interface;

#ifdef __cplusplus
}
#endif

#endif /* BDCC_H */
