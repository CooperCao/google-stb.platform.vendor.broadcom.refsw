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
 
#include "b_api_shim.h"
#include "bdcc_priv.h"
#include "bdcc_int.h"
#include "bdcc_intgfx.h"

BDBG_MODULE(BDCCINT);

static void BDCC_INT_P_BuildWndAttributesFromStyleID(BDCC_INT_P_Window * pw);
static void BDCC_INT_P_BuildPenFromStyleID(BDCC_INT_P_Window * pw);


void BDCC_INT_P_UpdateWindow(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw)
{
	unsigned int * pm = &pw->UpdateMask ;

	/*
	 * validate the window
	 */
	if ( ! pw->fDefined   &&   !(*pm & BDCC_ATTR_MASK_DEFINED_CLR) )
		return ;
		
	/*
	 * If we're creating a window, we do a
	 * little extra processing:
	 *   - set window attr per WndStyleID
	 *   - set pen attr per PenStyleID
	 *   - set pen location to (0,0)
	 *   - clear the window
	 */
	if ( pw->UpdateMask & BDCC_ATTR_MASK_DEFINED_SET )
	{
		BDCC_INT_P_BuildWndAttributesFromStyleID(pw) ;
		BDCC_INT_P_BuildPenFromStyleID(pw) ;

		

		pw->Pen.Row = 0 ;
		pw->Pen.Col = 0 ;
		/* oh, let's face it, we want everything */
		*pm |= 0xffffffff ;
		*pm &= ~BDCC_ATTR_MASK_DEFINED_CLR ; /* except this */
	}

	if ( pw->UpdateMask & BDCC_ATTR_MASK_PRINTDIR 
	 ||  pw->UpdateMask & BDCC_ATTR_MASK_SCROLLDIR
	 ||  pw->UpdateMask & BDCC_ATTR_MASK_JUSTIFY
	)
	{
		pw->UpdateMask |= BDCC_ATTR_MASK_CLEARWINDOW ;
	}

	if ( pw->UpdateMask & BDCC_ATTR_MASK_CLEARWINDOW )
	{
		pw->Pen.Row = 0 ;
		pw->Pen.Col = 0 ;
		pw->UpdateMask |= BDCC_ATTR_MASK_PENLOC ;
	}

	/*
	 * Call next layer.
	 */
	BCCGFX_INT_P_UpdateWindow(hCodObject, pw) ;
	
}

void BDCC_INT_P_Reset(BDCC_INT_P_Handle hCodObject)
{
	BCCGFX_INT_P_Reset(hCodObject) ;
}

void BDCC_INT_P_AccumulateChar(
		BDCC_INT_P_Handle hCodObject,
		BDCC_INT_P_Window * pw,
		unsigned char ch,
		unsigned int AddlBytes,
		unsigned char * pAddlBytes
		) 
{
	BDBG_MSG(("DccIntAccumulateChar, wndid = %d, ch = 0x%02x fDefined = 0x%08x\n", pw->WndId, ch, pw->fDefined)) ;
	if ( pw->fDefined )
	{
		/* RLQ, inject ETX can generate too many ETXs, (e.g. 0x8c, 0x89, 0x90, 0x91, 0x90 sequence), so if it is back-to-back ETX, drop it */
		static unsigned char pre_ch = 0;

		if ((0x03 == ch) && (pre_ch == ch)) {
			return;
		}
		pre_ch = ch;
		BCCGFX_INT_P_AccumulateChar(hCodObject, pw, ch, AddlBytes, pAddlBytes) ;
	}
	
} /* rifcAccumulateChar */




#define BDCC_INT_P_NUM_WND_STYLES			7
#define BDCC_INT_P_DccColor(r2,g2,b2)				((((r2)&3)<<4) | (((g2)&3)<<2) | ((b2)&3))
static const BDCC_INT_P_WindowAttr BDCC_INT_P_PreDefinedWndStyles[BDCC_INT_P_NUM_WND_STYLES] =
{
	/*just                                      print dir       							scroll dir      				wwrap   		effect 			n/a n/a  				fill color      			opacity     						border color */
	{BCCGFX_P_Justify_eLeft, 	BCCGFX_P_Direction_eLeftToRight, 	BCCGFX_P_Direction_eBottomToTop,	false	, BCCGFX_P_Effect_eSnap,	0,	0,	BDCC_INT_P_DccColor(0,0,0),	BDCC_Opacity_Solid,			BDCC_Edge_Style_None, 	BDCC_INT_P_DccColor(0,0,0)},
	{BCCGFX_P_Justify_eLeft, 	BCCGFX_P_Direction_eLeftToRight, 	BCCGFX_P_Direction_eBottomToTop,	false	, BCCGFX_P_Effect_eSnap,	0,	0,	BDCC_INT_P_DccColor(0,0,0),	BDCC_Opacity_Transparent,	BDCC_Edge_Style_None, 	BDCC_INT_P_DccColor(0,0,0)},
	{BCCGFX_P_Justify_eCenter, BCCGFX_P_Direction_eLeftToRight, 	BCCGFX_P_Direction_eBottomToTop,	false, BCCGFX_P_Effect_eSnap,	0,	0,	BDCC_INT_P_DccColor(0,0,0),	BDCC_Opacity_Solid,			BDCC_Edge_Style_None, 	BDCC_INT_P_DccColor(0,0,0)},
	{BCCGFX_P_Justify_eLeft,	BCCGFX_P_Direction_eLeftToRight, 	BCCGFX_P_Direction_eBottomToTop,	true,  BCCGFX_P_Effect_eSnap,	0,	0,	BDCC_INT_P_DccColor(0,0,0),	BDCC_Opacity_Solid,			BDCC_Edge_Style_None, 	BDCC_INT_P_DccColor(0,0,0)},
	{BCCGFX_P_Justify_eLeft,	BCCGFX_P_Direction_eLeftToRight, 	BCCGFX_P_Direction_eBottomToTop,	true, BCCGFX_P_Effect_eSnap,	0,	0,	BDCC_INT_P_DccColor(0,0,0),	BDCC_Opacity_Transparent,	BDCC_Edge_Style_None, 	BDCC_INT_P_DccColor(0,0,0)},
	{BCCGFX_P_Justify_eCenter,BCCGFX_P_Direction_eLeftToRight, 	BCCGFX_P_Direction_eBottomToTop,	true,  BCCGFX_P_Effect_eSnap,	0,	0,	BDCC_INT_P_DccColor(0,0,0),	BDCC_Opacity_Solid,			BDCC_Edge_Style_None, 	BDCC_INT_P_DccColor(0,0,0)},
	{BCCGFX_P_Justify_eLeft,	BCCGFX_P_Direction_eTopToBottom,	BCCGFX_P_Direction_eRightToLeft,	false	, BCCGFX_P_Effect_eSnap,	0,	0,	BDCC_INT_P_DccColor(0,0,0),	BDCC_Opacity_Solid,			BDCC_Edge_Style_None, 	BDCC_INT_P_DccColor(0,0,0)},
} ;

void BDCC_INT_P_BuildWndAttributesFromStyleID(BDCC_INT_P_Window * pw)
{
	int StyleIndex ;
	
	if ( pw->WindowDef.WndStyleID <= BDCC_INT_P_NUM_WND_STYLES )
	{
		StyleIndex = pw->WindowDef.WndStyleID ;
		if ( StyleIndex == 0 )
			StyleIndex = 1 ;
		pw->WindowAttr = BDCC_INT_P_PreDefinedWndStyles[StyleIndex-1] ;
	}
}

#define BDCC_INT_P_NUM_PEN_STYLES			7
BDCC_INT_P_Pen BDCC_INT_P_PreDefinedPenStyles[BDCC_INT_P_NUM_PEN_STYLES] =
{		/* size       							font          		    tt  		offset  				  I      UL 		edge                     	    cmd_args 				fg color       				opacity 						bg color      				opacity       						edge color 		  cmd_args[3] */
	{{BDCC_PenSize_Standard,BDCC_FontStyle_Default,	    0, BCCGFX_P_VertOffset_eNormal, false, false, BDCC_Edge_Style_None,	  {0x05,0x00}},{BDCC_INT_P_DccColor(2,2,2),BDCC_Opacity_Solid, BDCC_INT_P_DccColor(0,0,0),BDCC_Opacity_Solid, 		 BDCC_INT_P_DccColor(0,0,0), {0x2A,0x00,0x00}} ,0,0},
	{{BDCC_PenSize_Standard,BDCC_FontStyle_MonoSerifs, 0, BCCGFX_P_VertOffset_eNormal, false, false, BDCC_Edge_Style_None, 	  {0x05,0x01}},{BDCC_INT_P_DccColor(2,2,2),BDCC_Opacity_Solid, BDCC_INT_P_DccColor(0,0,0),BDCC_Opacity_Solid, 		 BDCC_INT_P_DccColor(0,0,0), {0x2A,0x00,0x00}} ,0,0},
	{{BDCC_PenSize_Standard,BDCC_FontStyle_PropSerifs,  0, BCCGFX_P_VertOffset_eNormal, false, false, BDCC_Edge_Style_None, 	  {0x05,0x02}},{BDCC_INT_P_DccColor(2,2,2),BDCC_Opacity_Solid, BDCC_INT_P_DccColor(0,0,0),BDCC_Opacity_Solid, 		 BDCC_INT_P_DccColor(0,0,0), {0x2A,0x00,0x00}} ,0,0},
	{{BDCC_PenSize_Standard,BDCC_FontStyle_Mono,	    0, BCCGFX_P_VertOffset_eNormal, false, false, BDCC_Edge_Style_None, 	  {0x05,0x03}},{BDCC_INT_P_DccColor(2,2,2),BDCC_Opacity_Solid, BDCC_INT_P_DccColor(0,0,0),BDCC_Opacity_Solid, 		 BDCC_INT_P_DccColor(0,0,0), {0x2A,0x00,0x00}} ,0,0},
	{{BDCC_PenSize_Standard,BDCC_FontStyle_Prop	,	    0, BCCGFX_P_VertOffset_eNormal, false, false, BDCC_Edge_Style_None, 	  {0x05,0x04}},{BDCC_INT_P_DccColor(2,2,2),BDCC_Opacity_Solid, BDCC_INT_P_DccColor(0,0,0),BDCC_Opacity_Solid, 		 BDCC_INT_P_DccColor(0,0,0), {0x2A,0x00,0x00}} ,0,0},
	{{BDCC_PenSize_Standard,BDCC_FontStyle_Mono,	    0, BCCGFX_P_VertOffset_eNormal, false, false, BDCC_Edge_Style_Uniform, {0x05,0x1b}},{BDCC_INT_P_DccColor(2,2,2),BDCC_Opacity_Solid, BDCC_INT_P_DccColor(0,0,0),BDCC_Opacity_Transparent, BDCC_INT_P_DccColor(0,0,0), {0x2A,0xC0,0x00}} ,0,0},
	{{BDCC_PenSize_Standard,BDCC_FontStyle_Prop,	    0, BCCGFX_P_VertOffset_eNormal, false, false, BDCC_Edge_Style_Uniform, {0x05,0x1c}},{BDCC_INT_P_DccColor(2,2,2),BDCC_Opacity_Solid, BDCC_INT_P_DccColor(0,0,0),BDCC_Opacity_Transparent, BDCC_INT_P_DccColor(0,0,0), {0x2A,0xC0,0x00}} ,0,0},
} ;

void BDCC_INT_P_BuildPenFromStyleID(BDCC_INT_P_Window * pw)
{
	int StyleIndex ;
	
	if ( pw->WindowDef.PenStyleID <= BDCC_INT_P_NUM_PEN_STYLES )
	{
		StyleIndex = pw->WindowDef.PenStyleID ;
		if ( StyleIndex == 0 )
			StyleIndex = 1 ;
		pw->Pen = BDCC_INT_P_PreDefinedPenStyles[StyleIndex-1] ;
	}
	
} /* BDCC_INT_P_BuildPenFromStyleID */

