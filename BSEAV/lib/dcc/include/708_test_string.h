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
#ifndef BDCC_TEST
#define BDCC_TEST



#define BDCC_MAX_TESTS 20 /* must be adjusted for each test we add/remove */

#define sh(v,s)    ((v) << (s))
#define DefineWindow(id,p,ap,rp,av,ah,rc,cc,rl,cl,v,ws,ps)	\
	(0x98 | sh(id,0)), \
	(sh(v,5) | sh(rl,4) | sh(cl,3) | sh(p,0)), \
	(sh(rp,7) | sh(av,0)), \
	(sh(ah,0)), \
	(sh(ap,4) | sh((rc)-1,0)), \
	(sh((cc)-1,0)), \
	(sh(ws,3) | sh(ps,0))
#define SetPenColor(fo,fr,fg,fb,bo,br,bg,bb,er,eg,eb) \
	0x91, \
	(sh(fo,6) | sh(fr,4) | sh(fg,2) | sh(fb,0)), \
	(sh(bo,6) | sh(br,4) | sh(bg,2) | sh(bb,0)), \
	(           sh(er,4) | sh(eg,2) | sh(eb,0))
#define Delay(dv) \
	0x8D, (dv)
#define ToggleWindows(wmask) \
	0x8B, (wmask)
#define DeleteWindows(wmask) \
	0x8C, (wmask)
#define ClearWindows(wmask) \
	0x88, (wmask)
#define DisplayWindows(wmask) \
	0x89, (wmask)
#define HideWindows(wmask) \
	0x8A, (wmask)
#define DelayCancel \
	0x8E
#define FF \
	0x0C
#define HCR \
	0x0E
#define SetWindowAttr(j,pd,sd,ww,de,ed,es,fr,fg,fb,fo,bt,br,bg,bb) \
	0x97, \
	(sh(fo,6) | sh(fr,4) | sh(fg,2) | sh(fb,0)), \
	(sh(bt,6) | sh(br,4) | sh(bg,2) | sh(bb,0)), \
	(sh(ww,6) | sh(pd,4) | sh(sd,2) | sh(j,0)), \
	(sh(es,4) | sh(ed,2) | sh(de,0))
#define SetPenAttr(psz,fs,i,u,ed) \
	0x90, \
	psz, \
	(sh(i,7) | sh(u,6) | sh(ed,3) | sh(fs,0))
#define CurrentWindow(wnd)		(0x80 | wnd)
#define SetPenLocation(r,c)		\
	0x92, (r), (c)




/***************************************************
 *             Test Stream Strategy
 *             --------------------
 *
 * Legend Window
 * 
 * Window 7 (zero based, mask 0x80) will always be 
 * visible and will announce the current test and
 * what is expected to be displayed in the other
 * windows.  It will not move and be located in the
 * first row.
 *
 ***************************************************/


/* Test 1 - 4:3
** ------------
** 8 caption window anchored to the 4 corners and 4 mid-points
** of the safe title area. Should be equally spaced out around
** the edge of the safe title area
*/
unsigned char testStream1[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,0,0,0,0,4,8,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '1', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(1,0,1,0,0,80,4,8,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '2', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(2,0,2,0,0,159,4,8,1,1,1,1,1),
	CurrentWindow(2),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '3', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(3,0,5,0,37,159,4,8,1,1,1,1,1),
	CurrentWindow(3),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '4', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(4,0,8,0,74,159,4,8,1,1,1,1,1),
	CurrentWindow(4),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '5', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(5,0,7,0,74,80,4,8,1,1,1,1,1),
	CurrentWindow(5),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '6', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(6,0,6,0,74,0,4,8,1,1,1,1,1),
	CurrentWindow(6),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '7', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(7,0,3,0,37,0,4,8,1,1,1,1,1),
	CurrentWindow(7),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '8', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,
} ;

/* Test 2 - 16:9
** -------------
** 8 caption window anchored to the 4 corners and 4 mid-points
** of the safe title area. Should be equally spaced out around
** the edge of the safe title area
*/
unsigned char testStream2[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,0,0,0,0,4,8,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '1', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(1,0,1,0,0,105,4,8,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '2', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(2,0,2,0,0,209,4,8,1,1,1,1,1),
	CurrentWindow(2),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '3', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(3,0,5,0,37,209,4,8,1,1,1,1,1),
	CurrentWindow(3),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '4', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(4,0,8,0,74,209,4,8,1,1,1,1,1),
	CurrentWindow(4),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '5', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(5,0,7,0,74,105,4,8,1,1,1,1,1),
	CurrentWindow(5),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '6', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(6,0,6,0,74,0,4,8,1,1,1,1,1),
	CurrentWindow(6),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '7', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,

	DefineWindow(7,0,3,0,37,0,4,8,1,1,1,1,1),
	CurrentWindow(7),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'W', 'i', 'n', 'd', 'o', 'w', ' ', '8', 0x03,
	SetPenLocation(1,0),	
	'L', 'i', 'n', 'e', ' ', '2', 0x03,
	SetPenLocation(2,0),	
	'L', 'i', 'n', 'e', ' ', '3', 0x03,
	SetPenLocation(3,0),	
	'L', 'i', 'n', 'e', ' ', '4', 0x03,
} ;

/* Test 3 - 4:3
** ------------
** Display the 8 Window Fill colors
*/
unsigned char testStream3[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,0,0,0,0,4,8,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(0,0,3,0,0,0,0,0,0,0,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'B', 'l', 'a', 'c', 'k', ' ', ' ', 0x03,

	DefineWindow(1,0,1,0,0,80,4,8,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(0,0,3,0,0,0,0,0,0,3,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'B', 'l', 'u', 'e', ' ', ' ', 0x03,

	DefineWindow(2,0,2,0,0,159,4,8,1,1,1,1,1),
	CurrentWindow(2),
	SetWindowAttr(0,0,3,0,0,0,0,0,3,0,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'G', 'r', 'e', 'e', 'n', ' ', ' ', 0x03,

	DefineWindow(3,0,5,0,37,159,4,8,1,1,1,1,1),
	CurrentWindow(3),
	SetWindowAttr(0,0,3,0,0,0,0,0,3,3,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'C', 'y', 'a', 'n', ' ', ' ', 0x03,

	DefineWindow(4,0,8,0,74,159,4,8,1,1,1,1,1),
	CurrentWindow(4),
	SetWindowAttr(0,0,3,0,0,0,0,3,0,0,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'R', 'e', 'd', ' ', ' ', ' ', 0x03,

	DefineWindow(5,0,7,0,74,80,4,8,1,1,1,1,1),
	CurrentWindow(5),
	SetWindowAttr(0,0,3,0,0,0,0,3,0,3,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'M', 'a', 'g', 'e', 'n', 't', 'a', ' ', 0x03,

	DefineWindow(6,0,6,0,74,0,4,8,1,1,1,1,1),
	CurrentWindow(6),
	SetWindowAttr(0,0,3,0,0,0,0,3,3,0,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'Y', 'e', 'l', 'l', 'o', 'w', ' ', 0x03,

	DefineWindow(7,0,3,0,37,0,4,8,1,1,1,1,1),
	CurrentWindow(7),
	SetWindowAttr(0,0,3,0,0,0,0,3,3,3,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'W', 'h', 'i', 't', 'e', ' ', ' ', 0x03,
} ;

/* Test 4 - 16:9
** -------------
** Display the 8 Window Fill colors
*/
unsigned char testStream4[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,0,0,0,0,4,8,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(0,0,3,0,0,0,0,0,0,0,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'B', 'l', 'a', 'c', 'k', ' ', ' ', 0x03,

	DefineWindow(1,0,1,0,0,105,4,8,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(0,0,3,0,0,0,0,0,0,3,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'B', 'l', 'u', 'e', ' ', ' ', 0x03,

	DefineWindow(2,0,2,0,0,209,4,8,1,1,1,1,1),
	CurrentWindow(2),
	SetWindowAttr(0,0,3,0,0,0,0,0,3,0,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'G', 'r', 'e', 'e', 'n', ' ', ' ', 0x03,

	DefineWindow(3,0,5,0,37,209,4,8,1,1,1,1,1),
	CurrentWindow(3),
	SetWindowAttr(0,0,3,0,0,0,0,0,3,3,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'C', 'y', 'a', 'n', ' ', ' ', 0x03,

	DefineWindow(4,0,8,0,74,209,4,8,1,1,1,1,1),
	CurrentWindow(4),
	SetWindowAttr(0,0,3,0,0,0,0,3,0,0,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'R', 'e', 'd', ' ', ' ', ' ', 0x03,

	DefineWindow(5,0,7,0,74,105,4,8,1,1,1,1,1),
	CurrentWindow(5),
	SetWindowAttr(0,0,3,0,0,0,0,3,0,3,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'M', 'a', 'g', 'e', 'n', 't', 'a', ' ', 0x03,

	DefineWindow(6,0,6,0,74,0,4,8,1,1,1,1,1),
	CurrentWindow(6),
	SetWindowAttr(0,0,3,0,0,0,0,3,3,0,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'Y', 'e', 'l', 'l', 'o', 'w', ' ', 0x03,

	DefineWindow(7,0,3,0,37,0,4,8,1,1,1,1,1),
	CurrentWindow(7),
	SetWindowAttr(0,0,3,0,0,0,0,3,3,3,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'W', 'h', 'i', 't', 'e', ' ', ' ', 0x03,
} ;



/* Test 5 - 4:3
** ------------
** Test the transparent window fill
** None of the fill colors should be visible
*/
unsigned char testStream5[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,0,0,0,0,4,8,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(0,0,3,0,0,0,0,0,0,0,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'B', 'l', 'a', 'c', 'k', ' ', ' ', 0x03,

	DefineWindow(1,0,1,0,0,80,4,8,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(0,0,3,0,0,0,0,0,0,3,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'B', 'l', 'u', 'e', ' ', ' ', 0x03,

	DefineWindow(2,0,2,0,0,159,4,8,1,1,1,1,1),
	CurrentWindow(2),
	SetWindowAttr(0,0,3,0,0,0,0,0,3,0,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'G', 'r', 'e', 'e', 'n', ' ', ' ', 0x03,

	DefineWindow(3,0,5,0,37,159,4,8,1,1,1,1,1),
	CurrentWindow(3),
	SetWindowAttr(0,0,3,0,0,0,0,0,3,3,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'C', 'y', 'a', 'n', ' ', ' ', 0x03,

	DefineWindow(4,0,8,0,74,159,4,8,1,1,1,1,1),
	CurrentWindow(4),
	SetWindowAttr(0,0,3,0,0,0,0,3,0,0,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'R', 'e', 'd', ' ', ' ', ' ', 0x03,

	DefineWindow(5,0,7,0,74,80,4,8,1,1,1,1,1),
	CurrentWindow(5),
	SetWindowAttr(0,0,3,0,0,0,0,3,0,3,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'M', 'a', 'g', 'e', 'n', 't', 'a', ' ', 0x03,

	DefineWindow(6,0,6,0,74,0,4,8,1,1,1,1,1),
	CurrentWindow(6),
	SetWindowAttr(0,0,3,0,0,0,0,3,3,0,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'Y', 'e', 'l', 'l', 'o', 'w', ' ', 0x03,

	DefineWindow(7,0,3,0,37,0,4,8,1,1,1,1,1),
	CurrentWindow(7),
	SetWindowAttr(0,0,3,0,0,0,0,3,3,3,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'W', 'h', 'i', 't', 'e', ' ', ' ', 0x03,
} ;

/* Test 6 - 16:9
** -------------
** Test the transparent window fill
** None of the fill colors should be visible
*/
unsigned char testStream6[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,0,0,0,0,4,8,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(0,0,3,0,0,0,0,0,0,0,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'B', 'l', 'a', 'c', 'k', ' ', ' ', 0x03,

	DefineWindow(1,0,1,0,0,105,4,8,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(0,0,3,0,0,0,0,0,0,3,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'B', 'l', 'u', 'e', ' ', ' ', 0x03,

	DefineWindow(2,0,2,0,0,209,4,8,1,1,1,1,1),
	CurrentWindow(2),
	SetWindowAttr(0,0,3,0,0,0,0,0,3,0,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'G', 'r', 'e', 'e', 'n', ' ', ' ', 0x03,

	DefineWindow(3,0,5,0,37,209,4,8,1,1,1,1,1),
	CurrentWindow(3),
	SetWindowAttr(0,0,3,0,0,0,0,0,3,3,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'C', 'y', 'a', 'n', ' ', ' ', 0x03,

	DefineWindow(4,0,8,0,74,209,4,8,1,1,1,1,1),
	CurrentWindow(4),
	SetWindowAttr(0,0,3,0,0,0,0,3,0,0,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'R', 'e', 'd', ' ', ' ', ' ', 0x03,

	DefineWindow(5,0,7,0,74,105,4,8,1,1,1,1,1),
	CurrentWindow(5),
	SetWindowAttr(0,0,3,0,0,0,0,3,0,3,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	'M', 'a', 'g', 'e', 'n', 't', 'a', ' ', 0x03,

	DefineWindow(6,0,6,0,74,0,4,8,1,1,1,1,1),
	CurrentWindow(6),
	SetWindowAttr(0,0,3,0,0,0,0,3,3,0,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'Y', 'e', 'l', 'l', 'o', 'w', ' ', 0x03,

	DefineWindow(7,0,3,0,37,0,4,8,1,1,1,1,1),
	CurrentWindow(7),
	SetWindowAttr(0,0,3,0,0,0,0,3,3,3,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'W', 'h', 'i', 't', 'e', ' ', ' ', 0x03,
} ;

/* Test 7 - 4:3
** ------------
** Single max size window in middle of the screen
** using the center anchor point
*/
unsigned char testStream7[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,4,0,37,80,15,32,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(7,0),	
	' ', 'C', 'e', 'n', 't', 'e', 'r', ' ', 0x03,

} ;

/* Test 8 - 16:9
** -------------
** Single max size window in middle of the screen
** using the center anchor point
*/
unsigned char testStream8[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,4,0,37,105,15,32,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(7,0),	
	' ', 'C', 'e', 'n', 't', 'e', 'r', ' ', 0x03,

} ;


/* Test 9 - 4:3
** ------------
** Test text justification
*/
unsigned char testStream9[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,4,0,15,80,1,20,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'L', 'e', 'f', 't', ' ', ' ', 0x03,

	DefineWindow(1,0,4,0,30,80,1,20,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(1,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'R', 'i', 'g', 'h', 't', ' ', ' ', 0x03,

	DefineWindow(2,0,4,0,45,80,1,20,1,1,1,1,1),
	CurrentWindow(2),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'C', 'e', 'n', 't', 'e', 'r', ' ', 0x03,

	DefineWindow(3,0,4,0,60,80,1,20,1,1,1,1,1),
	CurrentWindow(3),
	SetWindowAttr(3,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'L', 'e', 'f', 't', ' ', ' ', 0x03,
} ;

/* Test 10 - 16:9
** --------------
** Test text justification
*/
unsigned char testStream10[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,4,0,15,105,1,20,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(0,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'L', 'e', 'f', 't', ' ', ' ', 0x03,

	DefineWindow(1,0,4,0,30,105,1,20,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(1,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'R', 'i', 'g', 'h', 't', ' ', ' ', 0x03,

	DefineWindow(2,0,4,0,45,105,1,20,1,1,1,1,1),
	CurrentWindow(2),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', 'C', 'e', 'n', 't', 'e', 'r', ' ', 0x03,

	DefineWindow(3,0,4,0,60,105,1,20,1,1,1,1,1),
	CurrentWindow(3),
	SetWindowAttr(3,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(0,0),	
	' ', ' ', 'L', 'e', 'f', 't', ' ', ' ', 0x03,
} ;

/* Test 11 - 4:3
** -------------
** Font Styles at different sizes
*/
unsigned char testStream11[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,0,0,10,0,10,10,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(0,0,0,0,0),
	SetPenLocation(0,0),	
	'S', 'M', 'A', 'L', 'L', 0x03,
	SetPenAttr(0,0,0,0,0),
	SetPenLocation(2,0),	
	'D', 'e', 'f', 'a', 'u', 'l', 't', 0x03,
	SetPenAttr(0,1,0,0,0),
	SetPenLocation(3,0),	
	'M', 'o', 'n', 'o', '-', 'S', 'e', 'r', 'i', 'f', 0x03,
	SetPenAttr(0,2,0,0,0),
	SetPenLocation(4,0),	
	'P', 'r', 'o', 'p', '-', 'S', 'e', 'r', 'i', 'f', 0x03,
	SetPenAttr(0,3,0,0,0),
	SetPenLocation(5,0),	
	'M', 'o', 'n', 'o', '-', 'S', 'a', 'n', 's', 0x03,
	SetPenAttr(0,4,0,0,0),
	SetPenLocation(6,0),	
	'P', 'r', 'o', 'p', '-', 'S', 'a', 'n', 's', 0x03,
	SetPenAttr(0,5,0,0,0),
	SetPenLocation(7,0),	
	'C', 'a', 's', 'u', 'a', 'l', 0x03,
	SetPenAttr(0,6,0,0,0),
	SetPenLocation(8,0),	
	'C', 'u', 'r', 's', 'i', 'v', 'e', 0x03,
	SetPenAttr(0,7,0,0,0),
	SetPenLocation(9,0),	
	'S', 'm', 'a', 'l', 'l', ' ', 'C', 'a', 'p', 's', 0x03,

	DefineWindow(1,0,0,0,10,50,10,10,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenLocation(0,0),	
	'M', 'E', 'D', 0x03,
	SetPenAttr(1,0,0,0,0),
	SetPenLocation(2,0),	
	'D', 'e', 'f', 'a', 'u', 'l', 't', 0x03,
	SetPenAttr(1,1,0,0,0),
	SetPenLocation(3,0),	
	'M', 'o', 'n', 'o', '-', 'S', 'e', 'r', 'i', 'f', 0x03,
	SetPenAttr(1,2,0,0,0),
	SetPenLocation(4,0),	
	'P', 'r', 'o', 'p', '-', 'S', 'e', 'r', 'i', 'f', 0x03,
	SetPenAttr(1,3,0,0,0),
	SetPenLocation(5,0),	
	'M', 'o', 'n', 'o', '-', 'S', 'a', 'n', 's', 0x03,
	SetPenAttr(1,4,0,0,0),
	SetPenLocation(6,0),	
	'P', 'r', 'o', 'p', '-', 'S', 'a', 'n', 's', 0x03,
	SetPenAttr(1,5,0,0,0),
	SetPenLocation(7,0),	
	'C', 'a', 's', 'u', 'a', 'l', 0x03,
	SetPenAttr(1,6,0,0,0),
	SetPenLocation(8,0),	
	'C', 'u', 'r', 's', 'i', 'v', 'e', 0x03,
	SetPenAttr(1,7,0,0,0),
	SetPenLocation(9,0),	
	'S', 'm', 'a', 'l', 'l', ' ', 'C', 'a', 'p', 's', 0x03,

	DefineWindow(2,0,0,0,10,100,10,10,1,1,1,1,1),
	CurrentWindow(2),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(0,0),	
	'L', 'A', 'R', 'G', 'E', 0x03,
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(2,0),	
	'D', 'e', 'f', 'a', 'u', 'l', 't', 0x03,
	SetPenAttr(2,1,0,0,0),
	SetPenLocation(3,0),	
	'M', 'o', 'n', 'o', '-', 'S', 'e', 'r', 'i', 'f', 0x03,
	SetPenAttr(2,2,0,0,0),
	SetPenLocation(4,0),	
	'P', 'r', 'o', 'p', '-', 'S', 'e', 'r', 'i', 'f', 0x03,
	SetPenAttr(2,3,0,0,0),
	SetPenLocation(5,0),	
	'M', 'o', 'n', 'o', '-', 'S', 'a', 'n', 's', 0x03,
	SetPenAttr(2,4,0,0,0),
	SetPenLocation(6,0),	
	'P', 'r', 'o', 'p', '-', 'S', 'a', 'n', 's', 0x03,
	SetPenAttr(2,5,0,0,0),
	SetPenLocation(7,0),	
	'C', 'a', 's', 'u', 'a', 'l', 0x03,
	SetPenAttr(2,6,0,0,0),
	SetPenLocation(8,0),	
	'C', 'u', 'r', 's', 'i', 'v', 'e', 0x03,
	SetPenAttr(2,7,0,0,0),
	SetPenLocation(9,0),	
	'S', 'm', 'a', 'l', 'l', ' ', 'C', 'a', 'p', 's', 0x03,

} ;

/* Test 12 - 16:9
** --------------
** Font Styles at different sizes
*/
unsigned char testStream12[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,0,0,10,0,10,10,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(0,0,0,0,0),
	SetPenLocation(0,0),	
	'S', 'M', 'A', 'L', 'L', 0x03,
	SetPenAttr(0,0,0,0,0),
	SetPenLocation(2,0),	
	'D', 'e', 'f', 'a', 'u', 'l', 't', 0x03,
	SetPenAttr(0,1,0,0,0),
	SetPenLocation(3,0),	
	'M', 'o', 'n', 'o', '-', 'S', 'e', 'r', 'i', 'f', 0x03,
	SetPenAttr(0,2,0,0,0),
	SetPenLocation(4,0),	
	'P', 'r', 'o', 'p', '-', 'S', 'e', 'r', 'i', 'f', 0x03,
	SetPenAttr(0,3,0,0,0),
	SetPenLocation(5,0),	
	'M', 'o', 'n', 'o', '-', 'S', 'a', 'n', 's', 0x03,
	SetPenAttr(0,4,0,0,0),
	SetPenLocation(6,0),	
	'P', 'r', 'o', 'p', '-', 'S', 'a', 'n', 's', 0x03,
	SetPenAttr(0,5,0,0,0),
	SetPenLocation(7,0),	
	'C', 'a', 's', 'u', 'a', 'l', 0x03,
	SetPenAttr(0,6,0,0,0),
	SetPenLocation(8,0),	
	'C', 'u', 'r', 's', 'i', 'v', 'e', 0x03,
	SetPenAttr(0,7,0,0,0),
	SetPenLocation(9,0),	
	'S', 'm', 'a', 'l', 'l', ' ', 'C', 'a', 'p', 's', 0x03,

	DefineWindow(1,0,0,0,10,70,10,10,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	SetPenLocation(0,0),	
	'M', 'E', 'D', 0x03,
	SetPenAttr(1,0,0,0,0),
	SetPenLocation(2,0),	
	'D', 'e', 'f', 'a', 'u', 'l', 't', 0x03,
	SetPenAttr(1,1,0,0,0),
	SetPenLocation(3,0),	
	'M', 'o', 'n', 'o', '-', 'S', 'e', 'r', 'i', 'f', 0x03,
	SetPenAttr(1,2,0,0,0),
	SetPenLocation(4,0),	
	'P', 'r', 'o', 'p', '-', 'S', 'e', 'r', 'i', 'f', 0x03,
	SetPenAttr(1,3,0,0,0),
	SetPenLocation(5,0),	
	'M', 'o', 'n', 'o', '-', 'S', 'a', 'n', 's', 0x03,
	SetPenAttr(1,4,0,0,0),
	SetPenLocation(6,0),	
	'P', 'r', 'o', 'p', '-', 'S', 'a', 'n', 's', 0x03,
	SetPenAttr(1,5,0,0,0),
	SetPenLocation(7,0),	
	'C', 'a', 's', 'u', 'a', 'l', 0x03,
	SetPenAttr(1,6,0,0,0),
	SetPenLocation(8,0),	
	'C', 'u', 'r', 's', 'i', 'v', 'e', 0x03,
	SetPenAttr(1,7,0,0,0),
	SetPenLocation(9,0),	
	'S', 'm', 'a', 'l', 'l', ' ', 'C', 'a', 'p', 's', 0x03,

	DefineWindow(2,0,0,0,10,140,10,10,1,1,1,1,1),
	CurrentWindow(2),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(0,0),	
	'L', 'A', 'R', 'G', 'E', 0x03,
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(2,0),	
	'D', 'e', 'f', 'a', 'u', 'l', 't', 0x03,
	SetPenAttr(2,1,0,0,0),
	SetPenLocation(3,0),	
	'M', 'o', 'n', 'o', '-', 'S', 'e', 'r', 'i', 'f', 0x03,
	SetPenAttr(2,2,0,0,0),
	SetPenLocation(4,0),	
	'P', 'r', 'o', 'p', '-', 'S', 'e', 'r', 'i', 'f', 0x03,
	SetPenAttr(2,3,0,0,0),
	SetPenLocation(5,0),	
	'M', 'o', 'n', 'o', '-', 'S', 'a', 'n', 's', 0x03,
	SetPenAttr(2,4,0,0,0),
	SetPenLocation(6,0),	
	'P', 'r', 'o', 'p', '-', 'S', 'a', 'n', 's', 0x03,
	SetPenAttr(2,5,0,0,0),
	SetPenLocation(7,0),	
	'C', 'a', 's', 'u', 'a', 'l', 0x03,
	SetPenAttr(2,6,0,0,0),
	SetPenLocation(8,0),	
	'C', 'u', 'r', 's', 'i', 'v', 'e', 0x03,
	SetPenAttr(2,7,0,0,0),
	SetPenLocation(9,0),	
	'S', 'm', 'a', 'l', 'l', ' ', 'C', 'a', 'p', 's', 0x03,
} ;

/* Test 13 - 4:3 or 16:9
** ---------------------
** Single max size window in middle of the screen
** using the center anchor point
*/
unsigned char testStream13[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,4,0,37,80,15,10,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(0,0,0,0,0),
	SetPenLocation(0,0),	
	'S', 'M', 'A', 'L', 'L', 0x03,
	SetPenAttr(1,0,0,0,0),
	SetPenLocation(1,0),	
	'M', 'E', 'D', 'I', 'U', 'M', 0x03,
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(2,0),	
	'L', 'A', 'R', 'G', 'E', 0x03,
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(3,0),	
	'M',
	SetPenAttr(0,0,0,0,0),
	'I',
	SetPenAttr(1,0,0,0,0),
	'X',
	SetPenAttr(0,0,0,0,0),
	'T',
	SetPenAttr(2,0,0,0,0),
	'U',
	SetPenAttr(1,0,0,0,0),
	'R',
	SetPenAttr(2,0,0,0,0),
	'E', 0x03,
	
	SetPenAttr(1,0,0,0,0),
	SetPenLocation(5,0),	
	'N', 'O', 'R', 'M', 'A', 'L', 0x03,
	SetPenAttr(1,0,1,0,0),
	SetPenLocation(6,0),	
	'I', 'T', 'A', 'L', 'I', 'C', 0x03,
	SetPenAttr(1,0,0,0,0),
	SetPenLocation(7,0),	
	'M',
	SetPenAttr(1,0,1,0,0),
	'I',
	SetPenAttr(1,0,1,0,0),
	'X',
	SetPenAttr(1,0,0,0,0),
	'T',
	SetPenAttr(1,0,1,0,0),
	'U',
	SetPenAttr(1,0,0,0,0),
	'R',
	SetPenAttr(1,0,1,0,0),
	'E', 0x03,

	SetPenAttr(1,0,0,0,0),
	SetPenLocation(9,0),	
	'M',
	SetPenAttr(0,0,1,0,0),
	'I',
	SetPenAttr(1,0,1,0,0),
	'X',
	SetPenAttr(0,0,0,0,0),
	'T',
	SetPenAttr(2,0,1,0,0),
	'U',
	SetPenAttr(1,0,0,0,0),
	'R',
	SetPenAttr(2,0,1,0,0),
	'E',
	SetPenAttr(0,0,0,0,0),
	' ',
	SetPenAttr(2,0,1,0,0),
	'O',
	SetPenAttr(1,0,1,0,0),
	'F', 0x03,
	
	SetPenAttr(1,0,0,0,0),
	SetPenLocation(10,0),	
	'S',
	SetPenAttr(2,0,1,0,0),
	'I',
	SetPenAttr(0,0,1,0,0),
	'Z',
	SetPenAttr(2,0,0,0,0),
	'E',
	SetPenAttr(2,0,1,0,0),
	'S',
	SetPenAttr(1,0,0,0,0),
	' ',
	SetPenAttr(1,0,1,0,0),
	'A',
	SetPenAttr(1,0,0,0,0),
	'N',
	SetPenAttr(0,0,1,0,0),
	'D', 0x03,
	
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(11,0),	
	'S',
	SetPenAttr(0,0,1,0,0),
	'T',
	SetPenAttr(0,0,1,0,0),
	'Y',
	SetPenAttr(0,0,0,0,0),
	'L',
	SetPenAttr(1,0,1,0,0),
	'E',
	SetPenAttr(1,0,0,0,0),
	'S', 0x03,
} ;

/* Test 14 - 4:3 or 16:9
** ---------------------
** Single max size window in middle of the screen
** using the center anchor point
*/
unsigned char testStream14[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,4,0,37,80,15,10,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(0,0,0,0,0),
	SetPenLocation(0,0),	
	'S', 'M', 'A', 'L', 'L', 0x03,
	SetPenAttr(1,0,0,0,0),
	SetPenLocation(1,0),	
	'M', 'E', 'D', 'I', 'U', 'M', 0x03,
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(2,0),	
	'L', 'A', 'R', 'G', 'E', 0x03,
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(3,0),	
	'M',
	SetPenAttr(0,0,0,0,0),
	'I',
	SetPenAttr(1,0,0,0,0),
	'X',
	SetPenAttr(0,0,0,0,0),
	'T',
	SetPenAttr(2,0,0,0,0),
	'U',
	SetPenAttr(1,0,0,0,0),
	'R',
	SetPenAttr(2,0,0,0,0),
	'E', 0x03,
	
	SetPenAttr(1,0,0,0,0),
	SetPenLocation(5,0),	
	'N', 'O', 'R', 'M', 'A', 'L', 0x03,
	SetPenAttr(1,0,1,0,0),
	SetPenLocation(6,0),	
	'I', 'T', 'A', 'L', 'I', 'C', 0x03,
	SetPenAttr(1,0,0,0,0),
	SetPenLocation(7,0),	
	'M',
	SetPenAttr(1,0,1,0,0),
	'I',
	SetPenAttr(1,0,1,0,0),
	'X',
	SetPenAttr(1,0,0,0,0),
	'T',
	SetPenAttr(1,0,1,0,0),
	'U',
	SetPenAttr(1,0,0,0,0),
	'R',
	SetPenAttr(1,0,1,0,0),
	'E', 0x03,

	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,3,0,0,0,3,3,0,0,0),
	SetPenLocation(9,0),	
	'M',
	SetPenColor(0,0,3,0,0,0,3,0,0,0,0),
	SetPenAttr(0,0,1,0,0),
	'I',
	SetPenColor(0,0,0,0,0,0,0,3,0,0,0),
	SetPenAttr(1,0,1,0,0),
	'X',
	SetPenColor(0,3,0,3,0,0,3,0,3,0,0),
	SetPenAttr(0,0,0,0,0),
	'T',
	SetPenColor(0,3,3,0,0,0,0,0,0,0,0),
	SetPenAttr(2,0,1,0,0),
	'U',
	SetPenColor(0,3,0,0,0,0,0,0,3,0,0),
	SetPenAttr(1,0,0,0,0),
	'R',
	SetPenColor(0,0,3,0,0,0,3,3,0,0,0),
	SetPenAttr(2,0,1,0,0),
	'E',
	SetPenColor(0,3,3,0,0,0,3,0,3,0,0),
	SetPenAttr(0,0,0,0,0),
	' ',
	SetPenColor(0,0,0,3,0,0,0,0,0,0,0),
	SetPenAttr(2,0,1,0,0),
	'O',
	SetPenColor(0,3,0,0,0,0,3,3,0,0,0),
	SetPenAttr(1,0,1,0,0),
	'F', 0x03,
	
	SetPenAttr(1,0,0,0,0),
	SetPenColor(0,3,3,3,0,0,3,0,3,0,0),
	SetPenLocation(10,0),	
	'S',
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(2,0,1,0,0),
	'I',
	SetPenColor(0,0,0,0,0,0,0,0,0,0,0),
	SetPenAttr(0,0,1,0,0),
	'Z',
	SetPenColor(0,0,3,0,0,0,0,3,3,0,0),
	SetPenAttr(2,0,0,0,0),
	'E',
	SetPenColor(0,0,3,3,0,0,3,0,0,0,0),
	SetPenAttr(2,0,1,0,0),
	'S',
	SetPenColor(0,3,0,0,0,0,0,0,3,0,0),
	SetPenAttr(1,0,0,0,0),
	' ',
	SetPenColor(0,3,0,0,0,0,3,3,0,0,0),
	SetPenAttr(1,0,1,0,0),
	'A',
	SetPenColor(0,3,3,3,0,0,3,3,0,0,0),
	SetPenAttr(1,0,0,0,0),
	'N',
	SetPenColor(0,3,0,0,0,0,3,0,3,0,0),
	SetPenAttr(0,0,1,0,0),
	'D', 0x03,
	
	SetPenAttr(2,0,0,0,0),
	SetPenColor(0,3,3,0,0,0,0,3,3,0,0),
	SetPenLocation(11,0),	
	'S',
	SetPenColor(0,3,3,3,0,0,3,3,0,0,0),
	SetPenAttr(0,0,1,0,0),
	'T',
	SetPenColor(0,0,0,3,0,0,3,0,0,0,0),
	SetPenAttr(0,0,1,0,0),
	'Y',
	SetPenColor(0,3,3,3,0,0,0,3,0,0,0),
	SetPenAttr(0,0,0,0,0),
	'L',
	SetPenColor(0,3,0,0,0,0,0,0,3,0,0),
	SetPenAttr(1,0,1,0,0),
	'E',
	SetPenColor(0,3,0,3,0,0,3,0,0,0,0),
	SetPenAttr(1,0,0,0,0),
	'S', 0x03,
} ;



/* Test 15 - 4:3 or 16:9
** ---------------------
** Single max size window in middle of the screen
** using the center anchor point
*/
unsigned char testStream15[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,4,0,37,80,15,10,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(3,0),	
	'N', 'O', 'R', 'M', 'A', 'L', 0x03,
	SetPenAttr(2,0,1,0,0),
	SetPenLocation(5,0),	
	'I', 'T', 'A', 'L', 'I', 'C', 0x03,
	SetPenAttr(2,0,0,1,0),
	SetPenLocation(7,0),	
	'U', 'N', 'D', 'E', 'R', 'L', 'I', 'N', 'E', 0x03,
	SetPenAttr(2,0,1,1,0),
	SetPenLocation(9,0),	
	'I', 'T', 'A', 'L', 'I', 'C', 0x03,
	SetPenAttr(2,0,1,1,0),
	SetPenLocation(10,0),	
	'U', 'N', 'D', 'E', 'R', 'L', 'I', 'N', 'E', 0x03,
} ;

/* Test 16 - 4:3 or 16:9
** ---------------------
** Edge Types
*/
unsigned char testStream16[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,4,0,37,80,15,10,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),
	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(1,0),	
	'E', 'D', 'G', 'E', ' ', 'T', 'Y', 'P', 'E', 'S', 0x03,
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(3,0),	
	'N', 'O', ' ', 'E', 'D', 'G', 'E', 0x03,
	SetPenAttr(2,0,0,0,1),
	SetPenLocation(5,0),	
	'R', 'A', 'I', 'S', 'E', 'D', 0x03,
	SetPenAttr(2,0,0,0,2),
	SetPenLocation(7,0),	
	'D', 'E', 'P', 'R', 'E', 'S', 'S', 'E', 'D', 0x03,
	SetPenAttr(2,0,0,0,3),
	SetPenLocation(9,0),	
	'U', 'N', 'I', 'F', 'O', 'R', 'M', 0x03,
	SetPenAttr(2,0,0,0,4),
	SetPenLocation(11,0),	
	'L', 'E', 'F', 'T', ' ', 'D', 'R', 'O', 'P',  0x03,
	SetPenAttr(2,0,0,0,5),
	SetPenLocation(13,0),	
	'R', 'I', 'G', 'H', 'T', ' ', 'D', 'R', 'O', 'P', 0x03,
} ;




/* Test 17 - 4:3 or 16:9
** ---------------------
** Solid & Transparent Foreground Colors
*/
unsigned char testStream17[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,0,0,0,0,15,10,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),

	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(0,0),	
	'S', 'O', 'L', 'I', 'D', 0x03,


	SetPenColor(0,0,0,0,0,0,3,0,0,0,0),
	SetPenLocation(2,0),	
	'B', 'l', 'a', 'c', 'k', 0x03,

	SetPenColor(0,0,0,3,0,0,3,0,0,0,0),
	SetPenLocation(3,0),	
	'B', 'l', 'u', 'e', 0x03,

	SetPenColor(0,0,3,0,0,0,0,0,0,0,0),
	SetPenLocation(4,0),	
	'G', 'r', 'e', 'e', 'n', 0x03,

	SetPenColor(0,0,3,3,0,0,3,0,0,0,0),
	SetPenLocation(5,0),	
	'C', 'y', 'a', 'n', 0x03,

	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(6,0),	
	'R', 'e', 'd', 0x03,

	SetPenColor(0,3,0,3,0,0,3,0,0,0,0),
	SetPenLocation(7,0),	
	'M', 'a', 'g', 'e', 'n', 't', 'a', 0x03,

	SetPenColor(0,3,3,0,0,0,3,0,0,0,0),
	SetPenLocation(8,0),	
	'Y', 'e', 'l', 'l', 'o', 'w', 0x03,

	SetPenColor(0,3,3,3,0,0,3,0,0,0,0),
	SetPenLocation(9,0),	
	'W', 'h', 'i', 't', 'e', 0x03,

	DefineWindow(1,0,2,0,0,159,15,10,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),

	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(0,0),	
	'T', '-', 'P', 'A', 'R', 'E', 'N', 'T', 0x03,


	SetPenColor(3,0,0,0,0,0,3,0,0,0,0),
	SetPenLocation(2,0),	
	'B', 'l', 'a', 'c', 'k', 0x03,

	SetPenColor(3,0,0,3,0,0,3,0,0,0,0),
	SetPenLocation(3,0),	
	'B', 'l', 'u', 'e', 0x03,

	SetPenColor(3,0,3,0,0,0,3,0,0,0,0),
	SetPenLocation(4,0),	
	'G', 'r', 'e', 'e', 'n', 0x03,

	SetPenColor(3,0,3,3,0,0,3,0,0,0,0),
	SetPenLocation(5,0),	
	'C', 'y', 'a', 'n', 0x03,

	SetPenColor(3,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(6,0),	
	'R', 'e', 'd', 0x03,

	SetPenColor(3,3,0,3,0,0,3,0,0,0,0),
	SetPenLocation(7,0),	
	'M', 'a', 'g', 'e', 'n', 't', 'a', 0x03,

	SetPenColor(3,3,3,0,0,0,3,0,0,0,0),
	SetPenLocation(8,0),	
	'Y', 'e', 'l', 'l', 'o', 'w', 0x03,

	SetPenColor(3,3,3,3,0,0,3,0,0,0,0),
	SetPenLocation(9,0),	
	'W', 'h', 'i', 't', 'e', 0x03,
} ;


/* Test 18 - 4:3 or 16:9
** ---------------------
** Flashing & Translucent Foreground Colors
*/
unsigned char testStream18[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,0,0,0,0,15,10,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),

	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(0,0),	
	'F', 'L', 'A', 'S', 'H', 'I', 'N', 'G', 0x03,


	SetPenColor(1,0,0,0,0,0,3,0,0,0,0),
	SetPenLocation(2,0),	
	'B', 'l', 'a', 'c', 'k', 0x03,

	SetPenColor(1,0,0,3,0,0,3,0,0,0,0),
	SetPenLocation(3,0),	
	'B', 'l', 'u', 'e', 0x03,

	SetPenColor(1,0,3,0,0,0,3,0,0,0,0),
	SetPenLocation(4,0),	
	'G', 'r', 'e', 'e', 'n', 0x03,

	SetPenColor(1,0,3,3,0,0,3,0,0,0,0),
	SetPenLocation(5,0),	
	'C', 'y', 'a', 'n', 0x03,

	SetPenColor(1,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(6,0),	
	'R', 'e', 'd', 0x03,

	SetPenColor(1,3,0,3,0,0,3,0,0,0,0),
	SetPenLocation(7,0),	
	'M', 'a', 'g', 'e', 'n', 't', 'a', 0x03,

	SetPenColor(1,3,3,0,0,0,3,0,0,0,0),
	SetPenLocation(8,0),	
	'Y', 'e', 'l', 'l', 'o', 'w', 0x03,

	SetPenColor(1,3,3,3,0,0,3,0,0,0,0),
	SetPenLocation(9,0),	
	'W', 'h', 'i', 't', 'e', 0x03,

	DefineWindow(1,0,2,0,0,159,15,10,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),

	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(0,0),	
	'T', '-', 'L', 'U', 'C', 'E', 'N', 'T', 0x03,


	SetPenColor(2,0,0,0,0,0,3,0,0,0,0),
	SetPenLocation(2,0),	
	'B', 'l', 'a', 'c', 'k', 0x03,

	SetPenColor(2,0,0,3,0,0,3,0,0,0,0),
	SetPenLocation(3,0),	
	'B', 'l', 'u', 'e', 0x03,

	SetPenColor(2,0,3,0,0,0,3,0,0,0,0),
	SetPenLocation(4,0),	
	'G', 'r', 'e', 'e', 'n', 0x03,

	SetPenColor(2,0,3,3,0,0,3,0,0,0,0),
	SetPenLocation(5,0),	
	'C', 'y', 'a', 'n', 0x03,

	SetPenColor(2,3,0,0,0,0,3,0,0,0,0),
	SetPenLocation(6,0),	
	'R', 'e', 'd', 0x03,

	SetPenColor(2,3,0,3,0,0,3,0,0,0,0),
	SetPenLocation(7,0),	
	'M', 'a', 'g', 'e', 'n', 't', 'a', 0x03,

	SetPenColor(2,3,3,0,0,0,3,0,0,0,0),
	SetPenLocation(8,0),	
	'Y', 'e', 'l', 'l', 'o', 'w', 0x03,

	SetPenColor(2,3,3,3,0,0,3,0,0,0,0),
	SetPenLocation(9,0),	
	'W', 'h', 'i', 't', 'e', 0x03,
} ;






/* Test 19 - 4:3 or 16:9
** ---------------------
** Solid & Transparent background Colors
*/
unsigned char testStream19[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,0,0,0,0,15,10,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),

	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(0,0),	
	'S', 'O', 'L', 'I', 'D', 0x03,


	SetPenColor(0,0,3,0,0,0,0,0,0,0,0),
	SetPenLocation(2,0),	
	'B', 'l', 'a', 'c', 'k', 0x03,

	SetPenColor(0,0,3,0,0,0,0,3,0,0,0),
	SetPenLocation(3,0),	
	'B', 'l', 'u', 'e', 0x03,

	SetPenColor(0,0,3,0,0,0,3,0,0,0,0),
	SetPenLocation(4,0),	
	'G', 'r', 'e', 'e', 'n', 0x03,

	SetPenColor(0,0,3,0,0,0,3,3,0,0,0),
	SetPenLocation(5,0),	
	'C', 'y', 'a', 'n', 0x03,

	SetPenColor(0,0,3,0,0,3,0,0,0,0,0),
	SetPenLocation(6,0),	
	'R', 'e', 'd', 0x03,

	SetPenColor(0,0,3,0,0,3,0,3,0,0,0),
	SetPenLocation(7,0),	
	'M', 'a', 'g', 'e', 'n', 't', 'a', 0x03,

	SetPenColor(0,0,3,0,0,3,3,0,0,0,0),
	SetPenLocation(8,0),	
	'Y', 'e', 'l', 'l', 'o', 'w', 0x03,

	SetPenColor(0,0,3,0,0,3,3,3,0,0,0),
	SetPenLocation(9,0),	
	'W', 'h', 'i', 't', 'e', 0x03,

	DefineWindow(1,0,2,0,0,159,15,10,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),

	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(0,0),	
	'T', '-', 'P', 'A', 'R', 'E', 'N', 'T', 0x03,


	SetPenColor(0,0,3,0,3,0,0,0,0,0,0),
	SetPenLocation(2,0),	
	'B', 'l', 'a', 'c', 'k', 0x03,

	SetPenColor(0,0,3,0,3,0,0,3,0,0,0),
	SetPenLocation(3,0),	
	'B', 'l', 'u', 'e', 0x03,

	SetPenColor(0,0,3,0,3,0,3,0,0,0,0),
	SetPenLocation(4,0),	
	'G', 'r', 'e', 'e', 'n', 0x03,

	SetPenColor(0,0,3,0,3,0,3,3,0,0,0),
	SetPenLocation(5,0),	
	'C', 'y', 'a', 'n', 0x03,

	SetPenColor(0,0,3,0,3,3,0,0,0,0,0),
	SetPenLocation(6,0),	
	'R', 'e', 'd', 0x03,

	SetPenColor(0,0,3,0,3,3,0,3,0,0,0),
	SetPenLocation(7,0),	
	'M', 'a', 'g', 'e', 'n', 't', 'a', 0x03,

	SetPenColor(0,0,3,0,3,3,3,0,0,0,0),
	SetPenLocation(8,0),	
	'Y', 'e', 'l', 'l', 'o', 'w', 0x03,

	SetPenColor(0,0,3,0,3,3,3,3,0,0,0),
	SetPenLocation(9,0),	
	'W', 'h', 'i', 't', 'e', 0x03,
} ;


/* Test 20 - 4:3 or 16:9
** ---------------------
** Flashing & Translucent Foreground Colors
*/
unsigned char testStream20[] =
{
	DeleteWindows(0xFF),
		
	DefineWindow(0,0,0,0,0,0,15,10,1,1,1,1,1),
	CurrentWindow(0),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),

	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(0,0),	
	'F', 'L', 'A', 'S', 'H', 'I', 'N', 'G', 0x03,
	
	SetPenColor(0,0,3,0,1,0,0,0,0,0,0),
	SetPenLocation(2,0),	
	'B', 'l', 'a', 'c', 'k', 0x03,

	SetPenColor(0,0,3,0,1,0,0,3,0,0,0),
	SetPenLocation(3,0),	
	'B', 'l', 'u', 'e', 0x03,
	
	SetPenColor(0,0,3,0,1,0,3,0,0,0,0),
	SetPenLocation(4,0),	
	'G', 'r', 'e', 'e', 'n', 0x03,

	SetPenColor(0,0,3,0,1,0,3,3,0,0,0),
	SetPenLocation(5,0),	
	'C', 'y', 'a', 'n', 0x03,

	SetPenColor(0,0,3,0,1,3,0,0,0,0,0),
	SetPenLocation(6,0),	
	'R', 'e', 'd', 0x03,

	SetPenColor(0,0,3,0,1,3,0,3,0,0,0),
	SetPenLocation(7,0),	
	'M', 'a', 'g', 'e', 'n', 't', 'a', 0x03,

	SetPenColor(0,0,3,0,1,3,3,0,0,0,0),
	SetPenLocation(8,0),	
	'Y', 'e', 'l', 'l', 'o', 'w', 0x03,

	SetPenColor(0,0,3,0,1,3,3,3,0,0,0),
	SetPenLocation(9,0),	
	'W', 'h', 'i', 't', 'e', 0x03,

	DefineWindow(1,0,2,0,0,159,15,10,1,1,1,1,1),
	CurrentWindow(1),
	SetWindowAttr(2,0,3,0,0,0,0,1,1,1,0,0,0,0,0),

	SetPenColor(0,3,0,0,0,0,3,0,0,0,0),
	SetPenAttr(2,0,0,0,0),
	SetPenLocation(0,0),	
	'T', '-', 'L', 'U', 'C', 'E', 'N', 'T', 0x03,


	SetPenColor(0,0,3,0,2,0,0,0,0,0,0),
	SetPenLocation(2,0),	
	'B', 'l', 'a', 'c', 'k', 0x03,

	SetPenColor(0,0,3,0,2,0,0,3,0,0,0),
	SetPenLocation(3,0),	
	'B', 'l', 'u', 'e', 0x03,

	SetPenColor(0,0,0,0,2,0,3,0,0,0,0),
	SetPenLocation(4,0),	
	'G', 'r', 'e', 'e', 'n', 0x03,

	SetPenColor(0,0,3,0,2,0,3,3,0,0,0),
	SetPenLocation(5,0),	
	'C', 'y', 'a', 'n', 0x03,

	SetPenColor(0,0,3,0,2,3,0,0,0,0,0),
	SetPenLocation(6,0),	
	'R', 'e', 'd', 0x03,

	SetPenColor(0,0,3,0,2,3,0,3,0,0,0),
	SetPenLocation(7,0),	
	'M', 'a', 'g', 'e', 'n', 't', 'a', 0x03,

	SetPenColor(0,0,3,0,2,3,3,0,0,0,0),
	SetPenLocation(8,0),	
	'Y', 'e', 'l', 'l', 'o', 'w', 0x03,

	SetPenColor(0,0,3,0,2,3,3,3,0,0,0),
	SetPenLocation(9,0),	
	'W', 'h', 'i', 't', 'e', 0x03,

} ;



unsigned char *Dtvcc708Stream[] =
	{
		testStream1,
		testStream2,
		testStream3,
		testStream4,
		testStream5,
		testStream6,
		testStream7,
		testStream8,
		testStream9,
		testStream10,
		testStream11,
		testStream12,
		testStream13,
		testStream14,
		testStream15,
		testStream16,
		testStream17,
		testStream18,
		testStream19,
		testStream20,
	};

int Dtvcc708StreamLength[] =
	{
		sizeof(testStream1),
		sizeof(testStream2),
		sizeof(testStream3),
		sizeof(testStream4),
		sizeof(testStream5),
		sizeof(testStream6),
		sizeof(testStream7),
		sizeof(testStream8),
		sizeof(testStream9),
		sizeof(testStream10),
		sizeof(testStream11),
		sizeof(testStream12),
		sizeof(testStream13),
		sizeof(testStream14),
		sizeof(testStream15),
		sizeof(testStream16),
		sizeof(testStream17),
		sizeof(testStream18),
		sizeof(testStream19),
		sizeof(testStream20),
	};


#endif
