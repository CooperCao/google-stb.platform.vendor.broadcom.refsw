/***************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

/***************************************************************************
 *                            Theory Of Operation
 *                            -------------------
 *
 * 608 modes
 * 
 * EIA-608 Closed Captioning defines 3 captioning modes:  RollUp, PopOn 
 *    and PaintOn.
 * 
 * 
 * 708 windows
 * 
 * EIA-708-B defines 8 windows.  This transcoder design uses window 0 for RollUp;
 * windows 0-3 or 4-7 for PaintOn; and windows 0-7 for PopOn.  For RollUp, window
 * 0 is defined as either 2, 3 or 4 rows and full column width.  For PaintOn, 4
 * 1-row windows are used, either 0-3 or 4-7 depending on the mode when PaintOn
 * was initiated.  For PopOn, all 8 windows are used.  In this case, 4 are displayed
 * while the other 4 are loaded with new characters.
 * 
 * 608 state machine
 * 
 * This design uses a state machine to track the captioning mode.  Each state also
 * defines which windows are visible.
 * 
 * ctrl vs data
 * channel
 * command buffers
 * pen and window styles 2,2
 *
 ***************************************************************************/
/* Includes */
#include "bdcc.h"
#include "b_api_shim.h"
#include "bdcc_608transcoder.h"
#include "bdcc_gfx.h"
#include "bdcc_priv.h"

#include <assert.h>

BDBG_MODULE(BDCC608TRANSCODER);

/* Defines */
#define CheckParity(b)			(!((b) & 0x80))
#define GetDataChan(b1)			(((b1) >> 3) & 1) ;
#define ForceDataChan0(b1)		((b1) & 0xF7)

#define BDCC_P_NUM_608_STATE_INPUTS		16
#define RGB222(r,g,b)			(((r)<<4) | ((g)<<2) | (b))

typedef enum BDCC_608_P_State
{
	BDCC_608_P_State_eIdle,
	BDCC_608_P_State_eRollUp2,
	BDCC_608_P_State_eRollUp3,
	BDCC_608_P_State_eRollUp4,
	BDCC_608_P_State_ePopOn1,
	BDCC_608_P_State_ePopOn2,
	BDCC_608_P_State_ePaintOn1,
	BDCC_608_P_State_ePaintOn2,
	BDCC_608_P_State_ePaintOn02,
	BDCC_608_P_State_eNoChange
} BDCC_608_P_State ;

typedef struct BDCC_608_P_WindowCmds
{
	unsigned char		DefineWindow[7] ;
	unsigned char		SetPenColor[4] ;
	unsigned char		SetPenAttr[3] ;
	unsigned char		SetPenLocation[3] ;
} BDCC_608_P_WindowCmds ;


typedef struct BDCC_608_TranscoderObject
{
	BDCC_608_P_State eState ;
	int				CurrentWindow ;
	int				bLastPairWasCtrl ;
	unsigned char	LastB1 ;
	unsigned char	LastB2 ;
	int				SelectedDataChan ;
	int				CurrentDataChan ;
	int				SelectedField ;
	BDCC_CBUF		*pOutBuf	 ;
	int				fBegLine ;
	int				TextOrXDSMode ;
    bool            newActivity;

	BDCC_608_P_WindowCmds Cmds[8] ;

	struct
	{
		int		fRowAssigned ;
		int		RowAssigned ;
	} WndInfo[8] ;
#if 0
	struct
	{
		int		WndId1 ; // one based
	} RowInfo[15] ;
#endif	
	/* SWSTB-9657 */
	unsigned short		CharCount;		/* char count */
	int			CurWnd;			/* current Wnd */
	BDCC_608_P_State	CurState;		/* current state, current check 4 only to narrow down range of the fix can apply */

} BDCC_608_TranscoderObject;

typedef enum
{
	BDCC_608_P_Color_eWhite		= RGB222(3,3,3),
	BDCC_608_P_Color_eGreen		= RGB222(0,3,0),
	BDCC_608_P_Color_eBlue		= RGB222(0,0,3),
	BDCC_608_P_Color_eCyan		= RGB222(2,2,3),
	BDCC_608_P_Color_eRed		= RGB222(3,0,0),
	BDCC_608_P_Color_eYellow	= RGB222(3,3,0),
	BDCC_608_P_Color_eMagenta	= RGB222(3,0,3)
} BDCC_608_P_Color ;

typedef struct
{
	BDCC_608_P_State		NextState ;
	char *			CmdStr ;
} BDCC_608_P_StateInputDecisions ;

typedef struct BDCC_608_P_StateDecisions
{
	int						WndVisible[8] ;		/* DefWnd set vis & BegLine set vis */
	int						WndRowCount[8] ;	/* DefWnd set rc */
	unsigned char			ClrWndMask ;		/* state trans clear windows */
	unsigned char			DisplayedMask ;		/* erase memory/nondisp memory */
	BDCC_608_P_StateInputDecisions	Decisions[BDCC_P_NUM_608_STATE_INPUTS] ;
} BDCC_608_P_StateDecisions ;

typedef enum BDCC_608_P_CtrlMisc
{
	BDCC_608_P_CtrlMisc_eRCL	= 0x20, /* ResumeCaptionLoading	*/
	BDCC_608_P_CtrlMisc_eBS		= 0x21, /* BackSp */
	BDCC_608_P_CtrlMisc_eAOF	= 0x22, /* Reserved */
	BDCC_608_P_CtrlMisc_eAON	= 0x23, /* Reserved */
	BDCC_608_P_CtrlMisc_eDER	= 0x24, /* DeleteToEndOfRow */
	BDCC_608_P_CtrlMisc_eRU2	= 0x25, /* RollUp2 */
	BDCC_608_P_CtrlMisc_eRU3	= 0x26, /* RollUp3 */
	BDCC_608_P_CtrlMisc_eRU4	= 0x27, /* RollUp4 */
	BDCC_608_P_CtrlMisc_eFON	= 0x28, /* FlashOn */
	BDCC_608_P_CtrlMisc_eRDC	= 0x29, /* ResumeDirectCaptioning */
	BDCC_608_P_CtrlMisc_eTR		= 0x2A, /* TextRestart */
	BDCC_608_P_CtrlMisc_eRTD	= 0x2B, /* ResumeTextDisplay */
	BDCC_608_P_CtrlMisc_eEDM	= 0x2C, /* EraseDispMem */
	BDCC_608_P_CtrlMisc_eCR		= 0x2D, /* CReturn */
	BDCC_608_P_CtrlMisc_eENM	= 0x2E, /* EraseNonDispMem */
	BDCC_608_P_CtrlMisc_eEOC	= 0x2F  /* EndOfCaption */
} BDCC_608_P_CtrlMisc ;

#define BDCC_608_P_MinSingleByteCode    0x20


static void BDCC_608_P_ProcessCtrl(BDCC_608_hTranscoder h608Transcoder, unsigned char b1, unsigned char b2) ;
static void BDCC_608_P_ProcessChar(BDCC_608_hTranscoder h608Transcoder, unsigned char b);
static void BDCC_608_P_ProcessCharSpecial(BDCC_608_hTranscoder h608Transcoder,unsigned char b2);
static void BDCC_608_P_ProcessCtrl2(BDCC_608_hTranscoder h608Transcoder, unsigned char b1, unsigned char b2);
static void BDCC_608_P_ExecuteCmdStr(BDCC_608_hTranscoder h608Transcoder, char * pCmdStr);
static void BDCC_608_P_ProcessCtrlMisc(BDCC_608_hTranscoder h608Transcoder, unsigned char b2);
static void BDCC_608_P_ProcessCtrlPAC(BDCC_608_hTranscoder h608Transcoder, int Row, unsigned char b2PAC);
static void BDCC_608_P_ProcessCtrlMidRow(BDCC_608_hTranscoder h608Transcoder, unsigned char b2);
static void BDCC_608_P_CmdSendDefineAllWindows(BDCC_608_hTranscoder h608Transcoder);
static void BDCC_608_P_CmdSendDefineWindow(BDCC_608_hTranscoder h608Transcoder, int wnd);
#if 0
static void BDCC_608_P_CmdSendDelay(BDCC_608_hTranscoder h608Transcoder, unsigned char tenths);
static void BDCC_608_P_CmdSendShowWindows(BDCC_608_hTranscoder h608Transcoder, unsigned char mask);
#endif
static void BDCC_608_P_CmdSendClearWindows(BDCC_608_hTranscoder h608Transcoder, unsigned char mask);
/* comment out for now to cleanup building warning */
/* static void BDCC_608_P_CmdSendDeleteWindows( BDCC_608_hTranscoder h608Transcoder, 	unsigned char mask);*/
static void BDCC_608_P_CmdSendPenLocation(BDCC_608_hTranscoder h608Transcoder, int wnd);
static void BDCC_608_P_CmdSendPenColor(BDCC_608_hTranscoder h608Transcoder, int wnd);
static void BDCC_608_P_CmdSendSetCurrentWindow(BDCC_608_hTranscoder h608Transcoder, int wnd);
static void BDCC_608_P_CmdSendPenAttr(BDCC_608_hTranscoder h608Transcoder, int wnd);
static void BDCC_608_P_CmdSetVisible(BDCC_608_hTranscoder h608Transcoder, int wnd, int val);
static void BDCC_608_P_CmdSetRowCount(BDCC_608_hTranscoder h608Transcoder, int wnd, int val);
static void BDCC_608_P_CmdSetUnderline(BDCC_608_hTranscoder h608Transcoder, int wnd, int val);
static void BDCC_608_P_CmdSetFlash(BDCC_608_hTranscoder h608Transcoder, int wnd, int val);
static void BDCC_608_P_CmdSetItalics(BDCC_608_hTranscoder h608Transcoder, int wnd, int val);
static void BDCC_608_P_CmdSetRow(BDCC_608_hTranscoder h608Transcoder, int wnd, int val);
static void BDCC_608_P_CmdSetAnchorRow(BDCC_608_hTranscoder h608Transcoder, int wnd, int val);
static void BDCC_608_P_CmdSetCol(BDCC_608_hTranscoder h608Transcoder, int wnd, int val);
static void BDCC_608_P_CmdSetColor(BDCC_608_hTranscoder h608Transcoder, int wnd, int val);
static void BDCC_608_P_ProcessChar2(BDCC_608_hTranscoder h608Transcoder,unsigned char b) ;
static void BDCC_608_P_ProcessCtrlTab(BDCC_608_hTranscoder h608Transcoder, unsigned char b2) ;
static void BDCC_608_P_EraseDisplayedMem(BDCC_608_hTranscoder h608Transcoder) ;
static void BDCC_608_P_EraseNonDisplayedMem(BDCC_608_hTranscoder h608Transcoder) ;
static void BDCC_608_P_CmdSendCarriageReturn(BDCC_608_hTranscoder h608Transcoder);
static void BDCC_608_P_CmdSendSpace(BDCC_608_hTranscoder h608Transcoder);
static void BDCC_608_P_CmdSendBackSpace(BDCC_608_hTranscoder h608Transcoder);
static void BDCC_608_P_CmdSendFlip(BDCC_608_hTranscoder h608Transcoder);
static void BDCC_608_P_ResetRows(BDCC_608_hTranscoder h608Transcoder, unsigned char WndMask);
static void BDCC_608_P_ConsiderTextOrXDS(unsigned char b1,unsigned char b2,int * pTextOrXDSMode);
static void BDCC_608_P_ProcessCharExtended(BDCC_608_hTranscoder h608Transcoder,unsigned char b1, unsigned char b2);
static void BDCC_608_P_ProcessCtrlExtendedAttr(BDCC_608_hTranscoder h608Transcoder,unsigned char b1, unsigned char b2);
static void BDCC_608_P_ProcessCtrlSpecialAssignment(BDCC_608_hTranscoder h608Transcoder,unsigned char b1, unsigned char b2);
static void BDCC_608_P_ProcessCtrlUnknown(BDCC_608_hTranscoder h608Transcoder,unsigned char b1, unsigned char b2);
static unsigned char BDCC_608_P_ConvertParity(unsigned char in);
static int BDCC_608_P_WndFromRow(BDCC_608_hTranscoder h608Transcoder, int Row, BDCC_608_P_State eState) ;

static const BDCC_608_P_WindowCmds DefaultCmds =
{
	/* DefineWindow */		{0x98,0x18,0x00,0x00,0x00,0x1F,0x12}, /* 0x0a for solid fill*/
	/* SetPenColor */		{0x91,0x3F,0x00,0x00},
	/* SetPenAttr */		{0x90,0x05,0x01},
	/* SetPenLocation */	{0x92,0x00,0x00}
} ;

static const BDCC_608_P_StateDecisions StateDecisions[] =
{
	{ /* BDCC_608_P_State_eIdle */ {0,0,0,0,0,0,0,0}, {1,1,1,1,1,1,1,1}, 0xFF, 0x00,
	{
		/* RCL */ {BDCC_608_P_State_ePopOn1,	""},
		/* BS  */ {BDCC_608_P_State_eNoChange,	""},
		/* AOF */ {BDCC_608_P_State_eNoChange,	""},
		/* AON */ {BDCC_608_P_State_eNoChange,	""},
		/* DER */ {BDCC_608_P_State_eNoChange,	""},
		/* RU2 */ {BDCC_608_P_State_eRollUp2,	"en"},
		/* RU3 */ {BDCC_608_P_State_eRollUp3,	"en"},
		/* RU4 */ {BDCC_608_P_State_eRollUp4,	"en"},
		/* FON */ {BDCC_608_P_State_eNoChange,	"s"},
		/* RDC */ {BDCC_608_P_State_ePaintOn1,	""},
		/* TR  */ {BDCC_608_P_State_eNoChange,	""},
		/* RTD */ {BDCC_608_P_State_eNoChange,	""},
		/* EDM */ {BDCC_608_P_State_eNoChange,	"e"},
		/* CR  */ {BDCC_608_P_State_eNoChange,	""},
		/* ENM */ {BDCC_608_P_State_eNoChange,	"n"},
		/* EOC */ {BDCC_608_P_State_ePopOn1,	""},
	}
	},
	{ /* BDCC_608_P_State_eRollUp2 */ {1,0,0,0,0,0,0,0}, {2,1,1,1,1,1,1,1}, 0x00, 0x01,
	{
		/* RCL */ {BDCC_608_P_State_ePopOn1,	""},
		/* BS  */ {BDCC_608_P_State_eNoChange,	"b"},
		/* AOF */ {BDCC_608_P_State_eNoChange,	""},
		/* AON */ {BDCC_608_P_State_eNoChange,	""},
		/* DER */ {BDCC_608_P_State_eNoChange,	""},
		/* RU2 */ {BDCC_608_P_State_eNoChange,	""},
		/* RU3 */ {BDCC_608_P_State_eRollUp3,	""},
		/* RU4 */ {BDCC_608_P_State_eRollUp4,	""},
		/* FON */ {BDCC_608_P_State_eNoChange,	"s"},
		/* RDC */ {BDCC_608_P_State_ePaintOn02,	""},
		/* TR  */ {BDCC_608_P_State_eNoChange,		""},
		/* RTD */ {BDCC_608_P_State_eNoChange,		""},
		/* EDM */ {BDCC_608_P_State_eNoChange,	"e"},
		/* CR  */ {BDCC_608_P_State_eNoChange,	"R"},
		/* ENM */ {BDCC_608_P_State_eNoChange,	"n"},
		/* EOC */ {BDCC_608_P_State_ePopOn2,	""},
	}
	},
	{ /* BDCC_608_P_State_eRollUp3 */ {1,0,0,0,0,0,0,0}, {3,1,1,1,1,1,1,1}, 0x00, 0x01,
	{
		/* RCL */ {BDCC_608_P_State_ePopOn1,	""},
		/* BS  */ {BDCC_608_P_State_eNoChange,	"b"},
		/* AOF */ {BDCC_608_P_State_eNoChange,	""},
		/* AON */ {BDCC_608_P_State_eNoChange,	""},
		/* DER */ {BDCC_608_P_State_eNoChange,	""},
		/* RU2 */ {BDCC_608_P_State_eRollUp2,	""},
		/* RU3 */ {BDCC_608_P_State_eNoChange,	""},
		/* RU4 */ {BDCC_608_P_State_eRollUp4,	""},
		/* FON */ {BDCC_608_P_State_eNoChange,	"s"},
		/* RDC */ {BDCC_608_P_State_ePaintOn02,	""},
		/* TR  */ {BDCC_608_P_State_eNoChange/*BDCC_608_P_State_eIdle note.eg*/,		""},
		/* RTD */ {BDCC_608_P_State_eNoChange/*BDCC_608_P_State_eIdle note.eg*/,		""},
		/* EDM */ {BDCC_608_P_State_eNoChange,	"e"},
		/* CR  */ {BDCC_608_P_State_eNoChange,	"R"},
		/* ENM */ {BDCC_608_P_State_eNoChange,	"n"},
		/* EOC */ {BDCC_608_P_State_ePopOn2,	""},
	}
	},
	{ /* BDCC_608_P_State_eRollUp4 */ {1,0,0,0,0,0,0,0}, {4,1,1,1,1,1,1,1}, 0x00, 0x01,
	{
		/* RCL */ {BDCC_608_P_State_ePopOn1,	""},
		/* BS  */ {BDCC_608_P_State_eNoChange,	"b"},
		/* AOF */ {BDCC_608_P_State_eNoChange,	""},
		/* AON */ {BDCC_608_P_State_eNoChange,	""},
		/* DER */ {BDCC_608_P_State_eNoChange,	""},
		/* RU2 */ {BDCC_608_P_State_eRollUp2,	""},
		/* RU3 */ {BDCC_608_P_State_eRollUp3,	""},
		/* RU4 */ {BDCC_608_P_State_eNoChange,	""},
		/* FON */ {BDCC_608_P_State_eNoChange,	"s"},
		/* RDC */ {BDCC_608_P_State_ePaintOn02,	""},
		/* TR  */ {BDCC_608_P_State_eNoChange,		""},
		/* RTD */ {BDCC_608_P_State_eNoChange,		""},
		/* EDM */ {BDCC_608_P_State_eNoChange,	"e"},
		/* CR  */ {BDCC_608_P_State_eNoChange,	"R"},
		/* ENM */ {BDCC_608_P_State_eNoChange,	"n"},
		/* EOC */ {BDCC_608_P_State_ePopOn2,	""},
	}
	},
	{ /* BDCC_608_P_State_ePopOn1 */ {1,1,1,1,0,0,0,0}, {-1,1,1,1,1,1,1,1}, 0xF0, 0x0F,
	{
		/* RCL */ {BDCC_608_P_State_eNoChange,	""},
		/* BS  */ {BDCC_608_P_State_eNoChange,	"b"},
		/* AOF */ {BDCC_608_P_State_eNoChange,	""},
		/* AON */ {BDCC_608_P_State_eNoChange,	""},
		/* DER */ {BDCC_608_P_State_eNoChange,	""},
		/* RU2 */ {BDCC_608_P_State_eRollUp2,	"en"},
		/* RU3 */ {BDCC_608_P_State_eRollUp3,	"en"},
		/* RU4 */ {BDCC_608_P_State_eRollUp4,	"en"},
		/* FON */ {BDCC_608_P_State_eNoChange,	"s"},
		/* RDC */ {BDCC_608_P_State_ePaintOn1,	""},
		/* TR  */ {BDCC_608_P_State_eNoChange,		""},
		/* RTD */ {BDCC_608_P_State_eNoChange,		""},
		/* EDM */ {BDCC_608_P_State_eNoChange,	"e"},
		/* CR  */ {BDCC_608_P_State_eNoChange,	""},
		/* ENM */ {BDCC_608_P_State_eNoChange,	"n"},
		/* EOC */ {BDCC_608_P_State_ePopOn2,	"f"},
	}
	},
	{ /* BDCC_608_P_State_ePopOn2 */ {0,0,0,0,1,1,1,1}, {1,1,1,1,1,1,1,1}, 0x0F, 0xF0,
	{
		/* RCL */ {BDCC_608_P_State_eNoChange,	""},
		/* BS  */ {BDCC_608_P_State_eNoChange,	"b"},
		/* AOF */ {BDCC_608_P_State_eNoChange,	""},
		/* AON */ {BDCC_608_P_State_eNoChange,	""},
		/* DER */ {BDCC_608_P_State_eNoChange,	""},
		/* RU2 */ {BDCC_608_P_State_eRollUp2,	"en"},
		/* RU3 */ {BDCC_608_P_State_eRollUp3,	"en"},
		/* RU4 */ {BDCC_608_P_State_eRollUp4,	"en"},
		/* FON */ {BDCC_608_P_State_eNoChange,	"s"},
		/* RDC */ {BDCC_608_P_State_ePaintOn2,	""},
		/* TR  */ {BDCC_608_P_State_eNoChange,		""},
		/* RTD */ {BDCC_608_P_State_eNoChange,		""},
		/* EDM */ {BDCC_608_P_State_eNoChange,	"e"},
		/* CR  */ {BDCC_608_P_State_eNoChange,	""},
		/* ENM */ {BDCC_608_P_State_eNoChange,	"n"},
		/* EOC */ {BDCC_608_P_State_ePopOn1,	"f"},
	}
	},
	{ /* BDCC_608_P_State_ePaintOn1 */ {1,1,1,1,0,0,0,0}, {1,1,1,1,1,1,1,1}, 0x00, 0x0F,
	{
		/* RCL */ {BDCC_608_P_State_ePopOn1,	""},
		/* BS  */ {BDCC_608_P_State_eNoChange,	"b"},
		/* AOF */ {BDCC_608_P_State_eNoChange,	""},
		/* AON */ {BDCC_608_P_State_eNoChange,	""},
		/* DER */ {BDCC_608_P_State_eNoChange,	""},
		/* RU2 */ {BDCC_608_P_State_eRollUp2,	"en"},
		/* RU3 */ {BDCC_608_P_State_eRollUp3,	"en"},
		/* RU4 */ {BDCC_608_P_State_eRollUp4,	"en"},
		/* FON */ {BDCC_608_P_State_eNoChange,	"s"},
		/* RDC */ {BDCC_608_P_State_eNoChange,	""},
		/* TR  */ {BDCC_608_P_State_eNoChange,		""},
		/* RTD */ {BDCC_608_P_State_eNoChange,		""},
		/* EDM */ {BDCC_608_P_State_eNoChange,	"e"},
		/* CR  */ {BDCC_608_P_State_eNoChange,	""},
		/* ENM */ {BDCC_608_P_State_eNoChange,	"n"},
		/* EOC */ {BDCC_608_P_State_ePopOn2,	""},
	}
	},
	{ /* BDCC_608_P_State_ePaintOn2 */ {0,0,0,0,1,1,1,1}, {1,1,1,1,1,1,1,1}, 0x00, 0xF0,
	{
		/* RCL */ {BDCC_608_P_State_ePopOn2,	""},
		/* BS  */ {BDCC_608_P_State_eNoChange,	"b"},
		/* AOF */ {BDCC_608_P_State_eNoChange,	""},
		/* AON */ {BDCC_608_P_State_eNoChange,	""},
		/* DER */ {BDCC_608_P_State_eNoChange,	""},
		/* RU2 */ {BDCC_608_P_State_eRollUp2,	"en"},
		/* RU3 */ {BDCC_608_P_State_eRollUp3,	"en"},
		/* RU4 */ {BDCC_608_P_State_eRollUp4,	"en"},
		/* FON */ {BDCC_608_P_State_eNoChange,	"s"},
		/* RDC */ {BDCC_608_P_State_eNoChange,	""},
		/* TR  */ {BDCC_608_P_State_eNoChange,		""},
		/* RTD */ {BDCC_608_P_State_eNoChange,		""},
		/* EDM */ {BDCC_608_P_State_eNoChange,	"e"},
		/* CR  */ {BDCC_608_P_State_eNoChange,	""},
		/* ENM */ {BDCC_608_P_State_eNoChange,	"n"},
		/* EOC */ {BDCC_608_P_State_ePopOn1,	""},
	}
	},
	{ /* BDCC_608_P_State_ePaintOn02 */ {1,0,0,0,1,1,1,1}, {-1,1,1,1,1,1,1,1}, 0x00, 0xF1,
	{
		/* RCL */ {BDCC_608_P_State_ePopOn2,	""},
		/* BS  */ {BDCC_608_P_State_eNoChange,	"b"},
		/* AOF */ {BDCC_608_P_State_eNoChange,	""},
		/* AON */ {BDCC_608_P_State_eNoChange,	""},
		/* DER */ {BDCC_608_P_State_eNoChange,	""},
		/* RU2 */ {BDCC_608_P_State_eRollUp2,	"en"},
		/* RU3 */ {BDCC_608_P_State_eRollUp3,	"en"},
		/* RU4 */ {BDCC_608_P_State_eRollUp4,	"en"},
		/* FON */ {BDCC_608_P_State_eNoChange,	"s"},
		/* RDC */ {BDCC_608_P_State_eNoChange,	""},
		/* TR  */ {BDCC_608_P_State_eNoChange,		""},
		/* RTD */ {BDCC_608_P_State_eNoChange,		""},
		/* EDM */ {BDCC_608_P_State_eNoChange,	"e"},
		/* CR  */ {BDCC_608_P_State_eNoChange,	""},
		/* ENM */ {BDCC_608_P_State_eNoChange,	"n"},
		/* EOC */ {BDCC_608_P_State_ePopOn1,	""},
	}
	},
} ;

/**************************************************************************
 *
 * Function:		DccTranscode_Init
 *
 * Inputs:			
 * 					h608Transcoder				- object to init
 *					Field				- defines 608 field, 1 or 2
 *					Channel				- defines 608 CC channel, 1 or 2
 *
 * Outputs:		
 * 					h608Transcoder				- object state is modified
 *
 * Returns:			dccSuccess or a standard DCCERR error code
 *
 * Description:
 *
 * This function initializes a 608-708 Transcoder session object.
 *
 **************************************************************************/
BDCC_Error BDCC_608_TranscodeOpen(
	BDCC_608_hTranscoder 		*ph608Transcoder,	
	int 						Field, 
	int 						Channel
	)
{
	int wnd;
	BDCC_608_hTranscoder hHandle;
	BDCC_Error	ret = BDCC_Error_eSuccess;

	assert(ph608Transcoder);

	if ( Field != 1 && Field != 2 )
	{
		return(BDCC_Error_eInvalidParameter) ;
	}

	/* allocate the Transcoder handle */
	hHandle = (BDCC_608_hTranscoder)BKNI_Malloc( sizeof(BDCC_608_TranscoderObject));
	if(hHandle == NULL)
	{
		return BDCC_Error_eNoMemory;
	}

	*ph608Transcoder = hHandle;
	
	BKNI_Memset(hHandle, 0, sizeof(BDCC_608_TranscoderObject)) ;
	hHandle->SelectedDataChan = Channel - 1 ;
	hHandle->SelectedField = Field - 1;

	for ( wnd = 0 ; wnd < 8 ; wnd++ )
	{
		hHandle->Cmds[wnd] = DefaultCmds ;
		hHandle->Cmds[wnd].DefineWindow[0] = 0x98 | wnd ;
	}

	return(ret);
	
} /* DccTranscode_Init */

/**************************************************************************
 *
 * Function:		BDCC_608_TranscodeReset
 *
 * Inputs:			
 * 					BDCC_608_hTranscoder				- object to init
 *
 * Outputs:		
 *
 * Returns:			BDCC_Error_eSuccess or a standard BDCC_Error error code
 *
 * Description:
 *
 * This function resets a 608-708 Transcoder session object.
 *
 **************************************************************************/
BDCC_Error BDCC_608_TranscodeReset(
	BDCC_608_hTranscoder 		h608Transcoder,
	int 						Field, 
	int 						Channel
	)
{
	int wnd;

	BKNI_Memset(h608Transcoder, 0, sizeof(BDCC_608_TranscoderObject)) ;
	h608Transcoder->SelectedDataChan = Channel - 1 ;
	h608Transcoder->SelectedField = Field - 1;

	for ( wnd = 0 ; wnd < 8 ; wnd++ )
	{
		h608Transcoder->Cmds[wnd] = DefaultCmds ;
		h608Transcoder->Cmds[wnd].DefineWindow[0] = 0x98 | wnd ;
	}

	return BDCC_Error_eSuccess ;
}

/**************************************************************************
 *
 * Function:		BCCGFX_TranscodeClose
 *
 * Inputs:			
 * 					BDCC_608_hTranscoder	- object to Close
 *
 * Outputs:		
 * 					h608Transcoder		- Handle is freed
 *
 * Returns:			None
 *
 * Description:
 *
 * This function closes a 608-708 Transcoder session object.
 *
 **************************************************************************/
void BDCC_608_TranscodeClose(
	BDCC_608_hTranscoder 	h608Transcoder
	)
{
	assert(h608Transcoder);

	/* Free the memory allocated for Transcoder handle */
	BKNI_Free( h608Transcoder );
	return ;
}



/**************************************************************************
 *
 * Function:		BDCC_608_TranscodeProcess
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 * 					pInBuf				- input buffer
 *
 * Outputs:		
 *					pOutBuf				- output buffer
 *
 * Returns:			dccSuccess or a standard DCCERR error code
 *
 * Description:
 * 
 * This function reads (field,cc_byte1,cc_byte2) triplets from the 
 * input circular buffer and writes DTVCC EIA-708-B codes to the
 * output buffer.
 *
 **************************************************************************/
BDCC_Error BDCC_608_TranscodeProcess(
	BDCC_608_hTranscoder 	h608Transcoder,
	BDCC_CBUF 					*pInBuf,
	BDCC_CBUF 					*pOutBuf
	)
{
	unsigned int			InputBufTriplets ;
	unsigned int			curTriplet ;
	unsigned char *		pCurTriplet ;
	unsigned char			b1, b2 ;
	int					bThisPairWasCtrl ;
	
	/*
	 * Validate Arguments
	 */

	/*
	 * Prep output buffers.
	 */
	h608Transcoder->pOutBuf = pOutBuf ;

	InputBufTriplets = pInBuf->NumBytes / 3 ;
	BDCC_CBUF_ResetPeek(pInBuf) ;
	BDCC_CBUF_ResetPost(pOutBuf) ;

	for ( curTriplet=0 ; curTriplet < InputBufTriplets ; curTriplet++ )
	{
		pCurTriplet = BDCC_CBUF_PeekPtr(pInBuf, 3) ;
		BDCC_CBUF_UpdatePeek(pInBuf) ;

		if ( h608Transcoder->SelectedField == pCurTriplet[0] )
		{
			/*
			 * ConvertParity converts bit 7 from
			 * parity_bit to parity_error
			 */
			b1 = BDCC_608_P_ConvertParity(pCurTriplet[1]) ;
			b2 = BDCC_608_P_ConvertParity(pCurTriplet[2]) ;
			if ( (BDCC_P_DUMP_STREAM_FLAG & BDCC_P_608TRANSCODER)  &&  (b1 || b2) )
			{
				BDBG_WRN(("Dump XCoding (%02x,%02x)\n", b1, b2)) ;
			}

			/*
			 * The following logic is almost verbatim from
			 * the FCC spec for 608.  The goal here is qualify
			 * the 2 bytes of data call ProcessChar() for the
			 * displayable codes and ProcessCtrl() for the
			 * 2-byte control codes.  Note that ProcessChar()
			 * will substitute 0x7F (block) if its input fails
			 * parity.
			 */
			 /*
			 ** We can break the 608 codes into roughly 5 sets
			 **
			 ** - Standard Characters: single bytes that are  ~equivalent to ASCII
			 **   ( processed in "BDCC_608_P_ProcessChar2()" )
			 **
			 ** - Special Characters: two bytes
			 **   ( processed in "BDCC_608_P_ProcessCharSpecial()" )
			 **
			 ** - Mid-Row codes, Miscellaneous Control, PAC (Preamble Address Codes)
			 **   ( processed in "BDCC_608_P_ProcessCtrl2()" )
			 */

			bThisPairWasCtrl = 0 ;
            if (b1 || b2) /* don't waste cycles on NULL's */
            {
    			if ( (b1&0x7F) < BDCC_608_P_MinSingleByteCode )
    			{
    				if ( !CheckParity(b2) )
    				{
    					/* if 2nd fails parity, ignore pair */
    				}
    				else
    				{
    					/* 1st not checked, 2nd good */
    					if ( h608Transcoder->bLastPairWasCtrl   
    					 &&   b2 == h608Transcoder->LastB2   
    					 &&   (!CheckParity(b1) || b1 == h608Transcoder->LastB1))
    					{
    						/* ignore redundant, doesn't matter if b1 fails parity */
    						bThisPairWasCtrl = 1 ;
    					}
    					else
    					{
    						/* not expecting redundant ctrl or this is new ctrl */
    						if ( !CheckParity(b1) )
    						{
    							/* 1st failed parity */
    							BDCC_608_P_ProcessChar(h608Transcoder, 0x7F) ;
    							BDCC_608_P_ProcessChar(h608Transcoder, b2) ;
    						}
    						else
    						{
    							/* 1st good */
    							BDCC_608_P_ProcessCtrl(h608Transcoder, b1, b2) ;
    							bThisPairWasCtrl = 1 ;
    						}
    					}
    				}
    			}
    			else
    			{
    				/* not ctrl */
    				BDCC_608_P_ProcessChar(h608Transcoder, b1) ;
    				BDCC_608_P_ProcessChar(h608Transcoder, b2) ;
    			}
            }

			if ( bThisPairWasCtrl )
			{
				h608Transcoder->LastB1 = b1 ;
				h608Transcoder->LastB2 = b2 ;
			}
			h608Transcoder->bLastPairWasCtrl = bThisPairWasCtrl ;
		}
	} /* for (each triplet) */

	return(BDCC_Error_eSuccess) ;

} /* BDCC_608_TranscodeProcess */


bool BDCC_608_GetNewActivity(
	BDCC_608_hTranscoder h608Transcoder
	)
{
    bool newActivity = h608Transcoder->newActivity;
    
    h608Transcoder->newActivity = false;
    return newActivity;
}

			/****************************
			 *
			 * Internal Functions -- 1st Level
			 *
			 ****************************/

/**************************************************************************
 *
 * Function:		ProcessCtrl
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 * 					b1					- 1st byte of CC pair
 * 					b2					- 2nd byte of CC pair
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This is the first layer of control processing.  Here we merely qualify the
 * channel, 0 or 1.  We also force the channel to chan 0 (encoded in b1)
 * so that the lower control layers can assume this.
 *
 **************************************************************************/
void BDCC_608_P_ProcessCtrl(
	BDCC_608_hTranscoder h608Transcoder, 
	unsigned char b1, 
	unsigned char b2)
{
	h608Transcoder->CurrentDataChan = GetDataChan(b1) ;

	/*
	 * are we enterring or leaving Text mode or XDS?
	 */
	BDCC_608_P_ConsiderTextOrXDS(b1,b2,&h608Transcoder->TextOrXDSMode) ;

	if ((!h608Transcoder->TextOrXDSMode) 
	 &&  (h608Transcoder->CurrentDataChan == h608Transcoder->SelectedDataChan))
	{
		/*
		 * Now that we've filtered to the correct
		 * channel, let's remap chan 1 to chan 0
		 * so that all lower level code can assume
		 * chan 0
		 */
		 
        h608Transcoder->newActivity = true;
		b1 = ForceDataChan0(b1) ;
		
		BDCC_608_P_ProcessCtrl2(h608Transcoder,b1,b2) ;
	}
}


/**************************************************************************
 *
 * Function:		ProcessChar
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 * 					b					- CC character
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This is the first layer of data processing.  Here we merely qualify the
 * channel, 0 or 1.  
 *
 **************************************************************************/
void BDCC_608_P_ProcessChar(
	BDCC_608_hTranscoder h608Transcoder, 
	unsigned char b)
{
	if ( (!h608Transcoder->TextOrXDSMode) 
	 &&  (h608Transcoder->CurrentDataChan == h608Transcoder->SelectedDataChan) )
	{
	    h608Transcoder->newActivity = true;
		BDCC_608_P_ProcessChar2(h608Transcoder,b) ;
	}
} /* ProcessChar */





			/****************************
			 *
			 * Internal Functions -- Lower Levels
			 *
			 ****************************/

/**************************************************************************
 *
 * Function:		ProcessCtrl2
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 * 					b1					- 1st byte of CC pair
 * 					b2					- 2nd byte of CC pair
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function classifies the control pair as one of: Preamble Address
 * Code, Mid-Row Code or Misc Code and calls the appropriate handler.
 *
 **************************************************************************/
void BDCC_608_P_ProcessCtrl2(
	BDCC_608_hTranscoder h608Transcoder, 
	unsigned char b1, 
	unsigned char b2)
{
	unsigned char b2PAC;

	/*
	** If bit '6' of the least significant byte is set, this is a PAC (Preamble Address Code).
	*/
	int isPAC = b2 & 0x40 ;

	int iRowNum=0;

	/*
	** Process PAC (Preamble Address Codes)
	*/
	if ( isPAC )
	{
	    /*
	    ** Now look at bits (10:8,5) to determine which row has been specified.
	    */
	    switch ( b1 )
	    {
	    case 0x10 :
	        /* PAC row 11 */
	        iRowNum = 11;
	        break ;
	        
	    case 0x11 :
	        /* PAC rows 1,2 */
	        iRowNum = ( b2 & 0x20 ) ? 2 : 1 ;
	        break ;
	        
	    case 0x12 :
	        /* PAC rows 3,4 */
	        iRowNum = ( b2 & 0x20 ) ? 4 : 3 ;
	        break ;
	        
	    case 0x13 :
	        /* PAC rows 12,13 */
	        iRowNum = ( b2 & 0x20 ) ? 13 : 12;
	        break ;

	    case 0x14 :
	        /* Misc or PAC rows 14,15 */
	        iRowNum = ( b2 & 0x20 ) ? 15 : 14 ;
	        break ;
	        
	    case 0x15 :
	        /* PAC rows 5,6 */
	        iRowNum = ( b2 & 0x20 ) ? 6 : 5 ;
	        break ;
	        
	    case 0x16 :
	        /* PAC rows 7,8 */
	        iRowNum = ( b2 & 0x20 ) ? 8 : 7 ;
	        break ;
	        
	    case 0x17 :
	        /* Tab or PAC rows 9,10 */
	        iRowNum = ( b2 & 0x20 ) ? 10 : 9 ;
	        break ;
	        
	    default:
	        BDCC_608_P_ProcessCtrlUnknown(h608Transcoder,b1,b2) ;
	        break;
	    }   /* end of switch( b1 ) */    	

	    b2PAC = b2 & 0x1F ;

	    /*
	    ** If "iRowNum" hasn't been set, this isn't a valid PAC code.
	    */
	    if ( iRowNum )
	        BDCC_608_P_ProcessCtrlPAC( h608Transcoder, iRowNum, b2PAC );
	
	}   /* end of if ( PAC ) */

	/*
	** Process the Mid-Row,  Miscellaneous Control and "other" control codes.
	*/
	else
	{

	    switch ( b1 )
	    {
	    case 0x10 :
	        /* extended attributes? Where are these defined? */
	        if ( b2 & 0x20 )
	            BDCC_608_P_ProcessCtrlExtendedAttr(h608Transcoder,b1,b2) ;
	        else
	            BDCC_608_P_ProcessCtrlUnknown(h608Transcoder,b1,b2) ;
	        break ;

	    case 0x11 :
	        /* 	    ** mid-row codes, range ( 0x1120:0x112F )
	        ** special (two byte characters), range ( 0x1130:0x113F )
	        */
	        if ( (b2 & 0xF0) == 0x20 )
	            BDCC_608_P_ProcessCtrlMidRow(h608Transcoder, b2) ;
	        else if ( (b2 & 0xF0) == 0x30 )
	            BDCC_608_P_ProcessCharSpecial(h608Transcoder, b2) ;
	        else
	            BDCC_608_P_ProcessCtrlUnknown(h608Transcoder,b1,b2) ;
	        break ;

	    case 0x12 :
	        /* extended characters? Where are these defined? */
	        if ( (b2 & 0xE0) == 0x20 ) /* 0x20's and 0x30's */
	            BDCC_608_P_ProcessCharExtended(h608Transcoder, b1, b2) ;
	        else
	            BDCC_608_P_ProcessCtrlUnknown(h608Transcoder,b1,b2) ;
	        break ;

	    case 0x13 :
	        /* extended characters? Where are these defined? */
	        if ( (b2 & 0xE0) == 0x20 ) /* 0x20's and 0x30's */
	            BDCC_608_P_ProcessCharExtended(h608Transcoder, b1, b2) ;
	        else
	            BDCC_608_P_ProcessCtrlUnknown(h608Transcoder,b1,b2) ;
	        break ;

	    case 0x14 :
	        /*
	        ** miscellaneous control codes, range ( 0x1420:0x142F )
	        */
	        if ( (b2 & 0xF0) == 0x20 )
	            BDCC_608_P_ProcessCtrlMisc(h608Transcoder, b2) ;
	        else
	            BDCC_608_P_ProcessCtrlUnknown(h608Transcoder,b1,b2) ;
	        break ;

	    case 0x15 :
	        if ( (b2 & 0xF0) == 0x20 )
	        {
	            /* accomodation for field 2, see EIA/CEA-608-B Sect. 8.4 */
	            BDCC_608_P_ProcessCtrlMisc(h608Transcoder, b2) ;
	        }
	        else
	            BDCC_608_P_ProcessCtrlUnknown(h608Transcoder,b1,b2) ;
	        break ;

	    case 0x16 :
	        BDCC_608_P_ProcessCtrlUnknown(h608Transcoder,b1,b2) ;
	        break ;

	    case 0x17 :
	        /* Tab */
	        switch ( b2 )
	        {
	        case 0x21 :
	        case 0x22 :
	        case 0x23 :
	            /*
	            ** Tab codes, range ( 0x1721:0x1723 )
	            */
	            BDCC_608_P_ProcessCtrlTab(h608Transcoder, b2) ;
	            break ;
	        
	        case 0x2D :
	        case 0x2E :
	        case 0x2F :
	            BDCC_608_P_ProcessCtrlExtendedAttr(h608Transcoder,b1,b2) ;
	            break ;
	        
	        case 0x24 :
	        case 0x25 :
	        case 0x26 :
	        case 0x27 :
	        case 0x28 :
	        case 0x29 :
	        case 0x2A :
	            BDCC_608_P_ProcessCtrlSpecialAssignment(h608Transcoder,b1,b2) ;
	            break ;

	        default :
	            BDCC_608_P_ProcessCtrlUnknown(h608Transcoder,b1,b2) ;
	            break ;
	        }

	    break ;
	    
	    }   /* end of switch( b1 ) */
	    
	}   /* end of else ( not PAC ) */

} /* ProcessCtrl2 */


/**************************************************************************
 *
 * Function:		ProcessCtrl_Misc
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 * 					b2					- 2nd byte of CC pair
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function operates on the Misc Control Codes.  These codes are
 * responsible for advancing our state machine, so the state machine is
 * evaluated here.
 *
 **************************************************************************/
void BDCC_608_P_ProcessCtrlMisc(
	BDCC_608_hTranscoder h608Transcoder, 
	unsigned char b2)
{
	BDCC_608_P_State NextState ;
	
	/* 
	 * execute any commands for this transition
	 */
	BDCC_608_P_ExecuteCmdStr(h608Transcoder, StateDecisions[h608Transcoder->eState].Decisions[b2&0x0F].CmdStr) ;

	/*
	 * update the state
	 */
	NextState = StateDecisions[h608Transcoder->eState].Decisions[b2&0x0F].NextState ;
	if ( NextState != BDCC_608_P_State_eNoChange )
	{
		BDBG_MSG(("ProcessCtrl_Misc:  changing state from %d to %d, b2 is 0x%02x\n", h608Transcoder->eState, NextState, b2)) ;
		h608Transcoder->eState = NextState ;
		if ( b2 != BDCC_608_P_CtrlMisc_eEOC ) /* don't clear on EOC (flip) */
			BDCC_608_P_CmdSendClearWindows(h608Transcoder, StateDecisions[h608Transcoder->eState].ClrWndMask) ;
		BDCC_608_P_CmdSendDefineAllWindows(h608Transcoder) ;
		if ( NextState == BDCC_608_P_State_eRollUp2   ||   NextState == BDCC_608_P_State_eRollUp3   ||   NextState == BDCC_608_P_State_eRollUp4 )
			h608Transcoder->CurrentWindow = 0 ;
		BDCC_608_P_CmdSendSetCurrentWindow(h608Transcoder, h608Transcoder->CurrentWindow) ;
	}
/*	Cmd_Send_Delay(h608Transcoder, 10) ; // testing only */
	
} /* ProcessCtrl_Misc */


/**************************************************************************
 *
 * Function:		ProcessCtrl_PAC
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 *					Row					- row decoded from 1st byte
 * 					b2PAC				- 2nd byte of CC pair, cooked
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function operates on the Preamble Address Codes.
 *
 **************************************************************************/
void BDCC_608_P_ProcessCtrlPAC(
	BDCC_608_hTranscoder h608Transcoder, 
	int Row, 
	unsigned char b2PAC)
{
	int wnd ;
	int fPenLocation = 0 ;


	if ( h608Transcoder->eState == BDCC_608_P_State_eRollUp2 
	 ||  h608Transcoder->eState == BDCC_608_P_State_eRollUp3
	 ||  h608Transcoder->eState == BDCC_608_P_State_eRollUp4 ) 
	{
		wnd = 0 ;
	}
	else
	{
		wnd = BDCC_608_P_WndFromRow(h608Transcoder, Row, h608Transcoder->eState) ;
	}

	h608Transcoder->CurrentWindow = wnd ;

	BDCC_608_P_CmdSetUnderline(h608Transcoder, wnd, b2PAC & 1) ;
	b2PAC &= ~1 ;
	BDCC_608_P_CmdSetFlash(h608Transcoder, wnd, 0) ;
	BDCC_608_P_CmdSetItalics(h608Transcoder, wnd, 0) ;
	BDCC_608_P_CmdSetAnchorRow(h608Transcoder, wnd, Row) ; /* column is 0 */
	h608Transcoder->WndInfo[wnd].fRowAssigned = 1 ;
/*RLQ, DCC_608_P_CmdSetAnchorRow set column to 0 already */
#if 0
	BDCC_608_P_CmdSetCol(h608Transcoder, wnd, 0) ;
#endif
	switch ( b2PAC )
	{
		case 0x00 :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eWhite) ;
			break ;
		case 0x02 :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eGreen) ;
			break ;
		case 0x04 :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eBlue) ;
			break ;
		case 0x06 :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eCyan) ;
			break ;
		case 0x08 :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eRed) ;
			break ;
		case 0x0A :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eYellow) ;
			break ;
		case 0x0C :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eMagenta) ;
			break ;
		case 0x0E :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eWhite) ;
			BDCC_608_P_CmdSetItalics(h608Transcoder, wnd, 1) ;
			break ;

		case 0x10 :
		case 0x12 :
		case 0x14 :
		case 0x16 :
		case 0x18 :
		case 0x1A :
		case 0x1C :
		case 0x1E :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eWhite) ;
			BDCC_608_P_CmdSetCol(h608Transcoder, wnd, (b2PAC - 0x10) * 2) ;
			fPenLocation = 1 ;
			break ;
	}

	BDCC_608_P_CmdSendDefineWindow(h608Transcoder, wnd) ;
	BDCC_608_P_CmdSendPenColor(h608Transcoder, wnd) ;
	BDCC_608_P_CmdSendPenAttr(h608Transcoder, wnd) ;

	if ( h608Transcoder->eState == BDCC_608_P_State_eRollUp2 ) 
	{
		BDCC_608_P_CmdSetRow(h608Transcoder, wnd, 1) ;
		fPenLocation = 1 ;
	}
	else if ( h608Transcoder->eState == BDCC_608_P_State_eRollUp3 ) 
	{
		BDCC_608_P_CmdSetRow(h608Transcoder, wnd, 2) ;
		fPenLocation = 1 ;
	}
	else if ( h608Transcoder->eState == BDCC_608_P_State_eRollUp4 ) 
	{
		BDCC_608_P_CmdSetRow(h608Transcoder, wnd, 3) ;
		fPenLocation = 1 ;
	}
	else 
	{
		BDCC_608_P_CmdSetRow(h608Transcoder, wnd, 0) ;
		fPenLocation = 1 ;
	}

	if ( fPenLocation )
	{
		BDCC_608_P_CmdSendPenLocation(h608Transcoder, wnd) ;
	}

	h608Transcoder->fBegLine = 1 ;

} /* ProcessCtrl_PAC */


/**************************************************************************
 *
 * Function:		ProcessCtrl_MidRow
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 * 					b2					- 2nd byte of CC pair
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function operates on the Mid-Row Codes.
 *
 **************************************************************************/
void BDCC_608_P_ProcessCtrlMidRow(
	BDCC_608_hTranscoder h608Transcoder, 
	unsigned char b2)
{
	int wnd = h608Transcoder->CurrentWindow ;

	BDCC_608_P_CmdSetUnderline(h608Transcoder, wnd, b2 & 1) ;
	BDCC_608_P_CmdSetItalics(h608Transcoder, wnd, 0) ;

	switch ( b2 & 0x0E )
	{
		case 0x00 :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eWhite) ;
			break ;
		case 0x02 :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eGreen) ;
			break ;
		case 0x04 :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eBlue) ;
			break ;
		case 0x06 :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eCyan) ;
			break ;
		case 0x08 :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eRed) ;
			break ;
		case 0x0A :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eYellow) ;
			break ;
		case 0x0C :
			BDCC_608_P_CmdSetColor(h608Transcoder, wnd,BDCC_608_P_Color_eMagenta) ;
			break ;
		case 0x0E :
			BDCC_608_P_CmdSetItalics(h608Transcoder, wnd, 1) ;
			break ;
	}
	
	BDCC_608_P_CmdSendPenColor(h608Transcoder, wnd) ;
	BDCC_608_P_CmdSendPenAttr(h608Transcoder, wnd) ;
	BDCC_608_P_CmdSendSpace(h608Transcoder) ;

} /* ProcessCtrl_MidRow */


/**************************************************************************
 *
 * Function:		ExecuteCmdStr
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 * 					pCmdStr				- 2nd byte of CC pair
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function sequentially calls processing functions based on the
 * supplied command string.  Used for doing processing based on state
 * transitions.
 *
 **************************************************************************/
void BDCC_608_P_ExecuteCmdStr(
	BDCC_608_hTranscoder h608Transcoder, 
	char * pCmdStr)
{
	char * p ;
	
	for ( p=pCmdStr ; *p ; p++ )
	{
		switch ( *p )
		{
			case 'e' :
				BDCC_608_P_EraseDisplayedMem(h608Transcoder) ;
				break ;
			case 'n' :
				BDCC_608_P_EraseNonDisplayedMem(h608Transcoder) ;
				break ;
			case 'R' :
				BDCC_608_P_CmdSendCarriageReturn(h608Transcoder) ;
				break ;
			case 's' :
				BDCC_608_P_CmdSendSpace(h608Transcoder) ;
				break ;
			case 'f' :
				BDCC_608_P_CmdSendFlip(h608Transcoder) ;
				break ;
			case 'b' :
				BDCC_608_P_CmdSendBackSpace(h608Transcoder) ;
				break ;
			/* etc */
		}
	}
	
} /* ExecuteCmdStr */



/**************************************************************************
 *
 * Function:		ConvertParity
 *
 * Inputs:			
 * 					in				- input byte
 *
 * Outputs:		
 *
 * Returns:			converted byte
 *
 * Description:
 * 
 * ConvertParity() examines the input byte and determines whether or
 * not it has a parity error, based on ODD parity.  The upper bit is
 * converted from the parity bit to the parity error bit, meaning the
 * high bit of the output byte is 1 iff there was a parity error on the
 * input byte.
 *
 **************************************************************************/
unsigned char BDCC_608_P_ConvertParity(unsigned char in)
{
#if 0
	return(in & 0x7F) ;
#else
	static unsigned char ParityErrorArray[] = {1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1} ;
	unsigned char temp = ((in & 0x0F) ^ (in >> 4)) ;
	return((ParityErrorArray[temp]<<7) | (in&0x7F)) ;
#endif
}


/**************************************************************************
 *
 * Function:		Cmd_Send_DefineAllWindows
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function sends a DefineWindow command to all 8 windows.
 *
 **************************************************************************/
void BDCC_608_P_CmdSendDefineAllWindows(BDCC_608_hTranscoder h608Transcoder)
{
	int i ;

	/*
	 * first hide windows
	 */
	for ( i=0 ; i < 8 ; i++ )
	{
		if ( ! StateDecisions[h608Transcoder->eState].WndVisible[i] )
		{
		   /* BDCC_608_P_CmdSendDeleteWindows(h608Transcoder, i);*/
		   BDCC_608_P_CmdSendDefineWindow(h608Transcoder, i) ;
		}
	}
	/*
	 * then show windows
	 */
	for ( i=0 ; i < 8 ; i++ )
	{
		if ( StateDecisions[h608Transcoder->eState].WndVisible[i] )
			BDCC_608_P_CmdSendDefineWindow(h608Transcoder, i) ;
	}
}

/**************************************************************************
 *
 * Function:		Cmd_Send_DefineWindow
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 *					wnd					- window
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function sends a DefineWindow command to a window.
 *
 **************************************************************************/
void BDCC_608_P_CmdSendDefineWindow(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd)
{
	/*
	 * first, overlay those values that are
	 * defined by the state -- visible and rowcount
	 */
	BDBG_MSG(("transcoder Cmd_Send_DefineWindow wnd %d state %d rc %d vis %d\n", 
		wnd, h608Transcoder->eState, StateDecisions[h608Transcoder->eState].WndRowCount[wnd],
		StateDecisions[h608Transcoder->eState].WndVisible[wnd] && h608Transcoder->WndInfo[wnd].fRowAssigned)) ;
	BDCC_608_P_CmdSetVisible(h608Transcoder, wnd, StateDecisions[h608Transcoder->eState].WndVisible[wnd] && h608Transcoder->WndInfo[wnd].fRowAssigned) ;
	if ( StateDecisions[h608Transcoder->eState].WndRowCount[wnd] >= 0 ) {
		/* SWSTB-9657 */
		if (wnd == h608Transcoder->CurWnd && h608Transcoder->eState == h608Transcoder->CurState ) {
			/* currently limited to check state 4 only */
			if (4 != h608Transcoder->CurState) {
				h608Transcoder->CharCount = 0;
			}
		}
		else {
			h608Transcoder->CharCount = 0;
		}
		BDCC_608_P_CmdSetRowCount(h608Transcoder, wnd, StateDecisions[h608Transcoder->eState].WndRowCount[wnd]) ;
		h608Transcoder->CurWnd = wnd;
		h608Transcoder->CurState = h608Transcoder->eState;
	}

	 /*
	  * then, send it
	  */
	 BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, h608Transcoder->Cmds[wnd].DefineWindow, sizeof(h608Transcoder->Cmds[wnd].DefineWindow)) ;
}


/**************************************************************************
 *
 * Function:		Cmd_Send_SetCurrentWindow
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 *					wnd					- window
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function sends a SetCurrentWindow command.
 *
 **************************************************************************/
void BDCC_608_P_CmdSendSetCurrentWindow(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd)
{
	static unsigned char Cmd[1] = {0x80} ;
	Cmd[0] = 0x80 | wnd ;
	BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, Cmd, sizeof(Cmd)) ;
}


/**************************************************************************
 *
 * Function:		Cmd_Send_PenColor
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 *					wnd					- window
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function sends a SetPenColor command to a window.
 *
 **************************************************************************/
void BDCC_608_P_CmdSendPenColor(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd)
{
	 BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, h608Transcoder->Cmds[wnd].SetPenColor, sizeof(h608Transcoder->Cmds[wnd].SetPenColor)) ;
}


/**************************************************************************
 *
 * Function:		Cmd_Send_PenAttr
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 *					wnd					- window
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function sends a SetPenAttributes command to a window.
 *
 **************************************************************************/
void BDCC_608_P_CmdSendPenAttr(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd)
{
	 BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, h608Transcoder->Cmds[wnd].SetPenAttr, sizeof(h608Transcoder->Cmds[wnd].SetPenAttr)) ;
}


/**************************************************************************
 *
 * Function:		Cmd_Send_PenLocation
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 *					wnd					- window
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function sends a SetPenLocation command to a window.
 *
 **************************************************************************/
void BDCC_608_P_CmdSendPenLocation(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd)
{
	 BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, h608Transcoder->Cmds[wnd].SetPenLocation, sizeof(h608Transcoder->Cmds[wnd].SetPenLocation)) ;
}


/**************************************************************************
 *
 * Function:		Cmd_Send_Delay
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 *					tenths				- delay, in tenths of second
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function sends a Delay command.
 *
 **************************************************************************/
#if 0
void BDCC_608_P_CmdSendDelay(
	BDCC_608_hTranscoder h608Transcoder, 
	unsigned char tenths)
{
	static unsigned char DelayCmd[2] = {0x8d, 0x00} ;
	DelayCmd[1] = tenths ;
	BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, DelayCmd, sizeof(DelayCmd)) ;
}
#endif

/**************************************************************************
 *
 * Function:		Cmd_Send_ClearWindows
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 *					tenths				- delay, in tenths of second
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function sends a ClearWindow command with the supplied mask.
 *
 **************************************************************************/

 /* comment out for now to cleanup building warning */
#if 0
void BDCC_608_P_CmdSendDeleteWindows(
	BDCC_608_hTranscoder h608Transcoder, 
	unsigned char mask)
{
	static unsigned char DelWndCmd[2] = {0x8C, 0x00} ;
	DelWndCmd[1] = mask ;
	BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, DelWndCmd, sizeof(DelWndCmd)) ;
	BDCC_608_P_ResetRows(h608Transcoder, mask) ;
}
#endif
/**************************************************************************
 *
 * Function:		Cmd_Send_ClearWindows
 *
 * Inputs:			
 * 					h608Transcoder				- object, previously init'ed
 *					tenths				- delay, in tenths of second
 *
 * Outputs:		
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function sends a ClearWindow command with the supplied mask.
 *
 **************************************************************************/
void BDCC_608_P_CmdSendClearWindows(
	BDCC_608_hTranscoder h608Transcoder, 
	unsigned char mask)
{
	static unsigned char ClrWndCmd[2] = {0x88, 0x00} ;
	ClrWndCmd[1] = mask ;
	BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, ClrWndCmd, sizeof(ClrWndCmd)) ;
	BDCC_608_P_ResetRows(h608Transcoder, mask) ;
}

/**************************************************************************
 *
 * Function:		BDCC_608_P_SetByteField
 *
 * Inputs:			
 *					mask				- bit mask for byte
 *					shift				- bit shift for byte
 *					val					- bit value for byte
 *
 * Outputs:		
 * 					pByte				- output byte
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function sets a bit field within a byte.
 *
 **************************************************************************/
void BDCC_608_P_SetByteField(unsigned char * pByte, unsigned char mask, int shift, int val)
{
	*pByte &= ~ mask ;
	*pByte |= (mask & (val << shift)) ;
}


/**************************************************************************
 *
 * Function:		Cmd_SetXxx
 *
 * Inputs:			
 *					h608Transcoder				- object, previously init'ed
 *					wnd					- window id
 *					val					- value
 *
 * Outputs:		
 * 					
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * These functions set various fields within the DTVCC command buffers.
 *
 **************************************************************************/
void BDCC_608_P_CmdSetVisible(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd, 
	int val)
{
	BDCC_608_P_SetByteField(&h608Transcoder->Cmds[wnd].DefineWindow[1], 0x20, 5, (unsigned char)val) ;
}

void BDCC_608_P_CmdSetRowCount(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd, 
	int val)
{
	BDBG_MSG(("transcoder BDCC_608_P_CmdSetRowCount: setting row count  for wnd %d to rc-1 = %d\n", wnd, val-1)) ;
	BDCC_608_P_SetByteField(&h608Transcoder->Cmds[wnd].DefineWindow[4], 0x0F, 0, (unsigned char)val-1) ;
}

void BDCC_608_P_CmdSetUnderline(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd, 
	int val)
{
	BDCC_608_P_SetByteField(&h608Transcoder->Cmds[wnd].SetPenAttr[2], 0x40, 6, (unsigned char)val) ;
}

void BDCC_608_P_CmdSetFlash(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd, 
	int val)
{
	BDCC_608_P_SetByteField(&h608Transcoder->Cmds[wnd].SetPenColor[1], 0xC0, 6, (unsigned char)val) ;
}

void BDCC_608_P_CmdSetItalics(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd, 
	int val)
{
	BDCC_608_P_SetByteField(&h608Transcoder->Cmds[wnd].SetPenAttr[2], 0x80, 7, (unsigned char)val) ;
}

void BDCC_608_P_CmdSetRow(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd, 
	int val)
{
	BDBG_MSG(("608 Transcoder: setting row %d for wnd %d\n", val, wnd)) ;
	BDCC_608_P_SetByteField(&h608Transcoder->Cmds[wnd].SetPenLocation[1], 0x0F, 0, (unsigned char)val) ;
}

void BDCC_608_P_CmdSetAnchorRow(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd, 
	int val)
{
    int rowAdjust = StateDecisions[h608Transcoder->eState].WndRowCount[wnd];
#if 0
	BDBG_MSG(("608 Transcoder: setting ANCHOR row %d for wnd %d", (val*BDCC_P_CYGRID75)/BDCC_P_CY15, wnd));
#endif
	/* using real row value instead of Window Positioning Grad value */
	BDBG_MSG(("608 Transcoder: setting ANCHOR row %d for wnd %d", val, wnd));

    /* anchor row is defined as bottom row of caption window in 608 captions but in 708 captions we cannot use the fifteenth row if defined
    ** in this way, so we redefine the window in terms of anchor row being top row of the caption window */
    val -= rowAdjust;
    /* SWSTB-3192 */
    if (val < 0) val = 0;
    BDCC_608_P_SetByteField(&h608Transcoder->Cmds[wnd].DefineWindow[2], 0x7F, 0, (unsigned char)((val*BDCC_P_CYGRID75)/BDCC_P_CY15)) ;
}

void BDCC_608_P_CmdSetCol(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd, 
	int val)
{
	BDBG_MSG(("BDCC_608_P_CmdSetCol: wnd=%d, col=%d, curWnd=%d, CharCount=%d, CurCol=%d", wnd, val, h608Transcoder->CurWnd, h608Transcoder->CharCount, h608Transcoder->Cmds[wnd].SetPenLocation[2] & 0x3F));
	/* SWSTB-9657 */
	if (wnd == h608Transcoder->CurWnd && h608Transcoder->eState == h608Transcoder->CurState) {
		int curCol = (h608Transcoder->Cmds[wnd].SetPenLocation[2] & 0x3F);
		if (val && ((curCol + h608Transcoder->CharCount) >= val)) {
			/* 608 CC encoding issue, that causing missing chars issue due to pen location overlap repeatedly */
			BDBG_WRN(("BDCC_608_P_CmdSetCol: encoding issue can cause CC overrite. col=%d <= accum chars %d + cur col=%d", val, h608Transcoder->CharCount, curCol));
			val = curCol + h608Transcoder->CharCount + 1;
			h608Transcoder->CharCount = 0;
		}
		/* can't exceed 32, it is another issue in xaa stream */
		if (val >= 32)
			val = 0;
	}
	BDCC_608_P_SetByteField(&h608Transcoder->Cmds[wnd].SetPenLocation[2], 0x3F, 0, (unsigned char)val) ;
}

void BDCC_608_P_CmdSetColor(
	BDCC_608_hTranscoder h608Transcoder, 
	int wnd, 
	int val)
{
	BDCC_608_P_SetByteField(&h608Transcoder->Cmds[wnd].SetPenColor[1], 0x3F, 0, (unsigned char)val) ;
}


/**************************************************************************
 *
 * Function:		ProcessChar2
 *
 * Inputs:			
 *					h608Transcoder				- object, previously init'ed
 *					b					- character
 *
 * Outputs:		
 * 					
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function translates a 608 character to a 708 character and
 * outputs it.
 *
 **************************************************************************/
void BDCC_608_P_ProcessChar2(
	BDCC_608_hTranscoder h608Transcoder,
	unsigned char b)
{
	if ( b )
	{
		BDBG_MSG(("ProcessChar2:  0x%02x  '%c'\n", b, b)) ;
	}
#if 0
	if ( h608Transcoder->fBegLine )
	{
		Space(h608Transcoder) ;
		h608Transcoder->fBegLine = 0 ;

		wnd = h608Transcoder->CurrentWindow ;
		if ( StateDecisions[h608Transcoder->eState].WndVisible[wnd] )
			Cmd_Send_ShowWindows(h608Transcoder, (unsigned char)(1<<wnd)) ;
	}
#endif
	/*
	 * check the character for parity
	 * if it fails, change it to the
	 * solid block char
	 */
	if ( b & 0x80 )
	{
		/* parity error, convert to solid block */
		b = 0x7F ;
	}

	/* SWSTB-9657, check how many char accumlated to same wnd, same row */
	h608Transcoder->CharCount++;

	/*
	 * First, translate those chars that don't 
	 * map directly to 708
	 */
	switch ( b )
	{
		case 0x2A : b = 0xE1 ; break ;
		case 0x5C : b = 0xE9 ; break ;
		case 0x5E : b = 0xED ; break ;
		case 0x5F : b = 0xF3 ; break ;
		case 0x60 : b = 0xFA ; break ;
		case 0x7B : b = 0xE7 ; break ;
		case 0x7C : b = 0xF7 ; break ;
		case 0x7D : b = 0xD1 ; break ;
		case 0x7E : b = 0xF1 ; break ;

		case 0x7F :
			/* the solid block is a 2-byte code in 708 */
			b = 0x10 ;
			BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, &b, 1) ;
			b = 0x30 ;
			BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, &b, 1) ;
			return ;
	}

	/*
	 * send it out
	 */
	BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, &b, 1) ;
	
}


/**************************************************************************
 *
 * Function:		ProcessChar_Special
 *
 * Inputs:			
 *					h608Transcoder				- object, previously init'ed
 *					b2					- 2nd byte
 *
 * Outputs:		
 * 					
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function translates a 608 character to a 708 character and
 * outputs it.
 *
 **************************************************************************/
void BDCC_608_P_ProcessCharSpecial(
	BDCC_608_hTranscoder h608Transcoder,
	unsigned char b2)
{
	static unsigned char SpecialChar[] = {0xAE,0xBA,0xBD,0xBF,0x39,0xA2,0xA3,0x7F,0xE0,0x20,0xE8,0xE2,0xEA,0xEE,0xF4,0xFB} ;
#if 0
	if ( h608Transcoder->fBegLine )
	{
		int wnd ;
		
		Space(h608Transcoder) ;
		h608Transcoder->fBegLine = 0 ;

		wnd = h608Transcoder->CurrentWindow ;
		if ( StateDecisions[h608Transcoder->eState].WndVisible[wnd] )
			Cmd_Send_ShowWindows(h608Transcoder, (unsigned char)(1<<wnd)) ;
	}
#endif
	BDBG_MSG(("ProcessChar_Special:  b2=%02X)\n", b2)) ;
	if ( (b2 & 0xF0) == 0x30 )
	{
		b2 = SpecialChar[b2 & 0x0F] ;
		if ( b2 < 0x40 )
		{
			unsigned char ext1 = 0x10 ;
			BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, &ext1, 1) ;
		}
		/* SWSTB-9657, check how many char accumlated to same wnd, same row, including special chars */
		h608Transcoder->CharCount++;

		BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, &b2, 1) ;
	}
}


struct
{
	unsigned char First ;
	unsigned char Second ;
} ExtendedChars[] =
{
	/*
	 * Table 5:  Spanish
	 */
	{0xC1, 0x00},
	{0xC9, 0x00},
	{0xD3, 0x00},
	{0xDA, 0x00},
	{0xDC, 0x00},
	{0xFC, 0x00},
	{0x10, 0x31},
	{0xA1, 0x00},
	
	/*
	 * Table 6:  Misc
	 */
	{0x2A, 0x00},
	{0x27, 0x00},
	{0x10, 0x7D},
	{0xA9, 0x00},
	{0x10, 0x3D},
	{0x10, 0x35},
	{0x10, 0x33},
	{0x10, 0x34},
	
	/*
	 * Table 7:  French
	 */
	{0xC0, 0x00},
	{0xC2, 0x00},
	{0xC7, 0x00},
	{0xC8, 0x00},
	{0xCA, 0x00},
	{0xCB, 0x00},
	{0xEB, 0x00},
	{0xCE, 0x00},
	{0xCF, 0x00},
	{0xEF, 0x00},
	{0xD4, 0x00},
	{0xD9, 0x00},
	{0xF9, 0x00},
	{0xDB, 0x00},
	{0xAB, 0x00},
	{0xBB, 0x00},
	
	/*
	 * Table 8:  Portugese
	 */
	{0xC3, 0x00},
	{0xE3, 0x00},
	{0xCD, 0x00},
	{0xCC, 0x00},
	{0xEC, 0x00},
	{0xD2, 0x00},
	{0xF2, 0x00},
	{0xD5, 0x00},
	{0xF5, 0x00},
	{0x7B, 0x00},
	{0x7D, 0x00},
	{0x5C, 0x00},
	{0x5E, 0x00},
	{0x5F, 0x00},
	{0x7C, 0x00},
	{0x7E, 0x00},
	
	/*
	 * Table 9:  German
	 */
	{0xC4, 0x00},
	{0xE4, 0x00},
	{0xD6, 0x00},
	{0xF6, 0x00},
	{0xDF, 0x00},
	{0xA5, 0x00},
	{0xA4, 0x00},
	{0x10, 0x7A},
	
	/*
	 * Table 10:  Danish
	 */
	{0xC5, 0x00},
	{0xE5, 0x00},
	{0xD8, 0x00},
	{0xF8, 0x00},
	{0x10, 0x7F},
	{0xAC, 0x00},
	{0x10, 0x7C},
	{0x00, 0x00},
} ;

void BDCC_608_P_ProcessCharExtended(
	BDCC_608_hTranscoder h608Transcoder,
	unsigned char b1, 
	unsigned char b2)
{
	int ExtIndex ;
	unsigned char outChar ;

	if ( ((b1 & 0xFE) != 0x12)   ||   ((b2 & 0xE0) != 0x20) )
	{
		BDBG_ERR(("ProcessChar_Extended:  unknown (%02X,%02X)\n", b1,b2)) ;
		return ;
	}

	/*
	 * first, there's an implied backspace
	 */
	BDCC_608_P_CmdSendBackSpace(h608Transcoder) ;
	
	/*
	 * there are 64 extended chars
	 * the b1/b2 bit pattern will be truncated
	 * to produce a 6 bit array index
	 * ---b1----  ---b2----
	 * 0001 001x  001x xxxx
	 */
	ExtIndex = b2 & 0x1F ;
	if ( b1 & 1 )
		ExtIndex |= 0x20 ;

	outChar = ExtendedChars[ExtIndex].First ;
	if ( outChar == 0x10 )
	{
		/* 2 byte code */
		BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, &outChar, 1) ;
		outChar = ExtendedChars[ExtIndex].Second ;
	}
	BDBG_MSG(("ProcessChar_Extended: (%02X,%02X) translated to %02X\n",b1,b2,outChar)) ;
	BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, &outChar, 1) ;
	
} /* ProcessChar_Extended */


void BDCC_608_P_ProcessCtrlExtendedAttr(
	BDCC_608_hTranscoder h608Transcoder,
	unsigned char b1, 
	unsigned char b2)
{
	BSTD_UNUSED(h608Transcoder) ;
	BDBG_MSG(("ProcessCtrl_ExtendedAttr:  (%02X,%02X)\n", b1,b2)) ;
} /* ProcessCtrl_ExtendedAttr */


void BDCC_608_P_ProcessCtrlSpecialAssignment(
	BDCC_608_hTranscoder h608Transcoder,
	unsigned char b1, 
	unsigned char b2)
{
	BSTD_UNUSED(h608Transcoder) ;
	BDBG_MSG(("ProcessCtrl_SpecialAssignment:  (%02X,%02X)\n", b1,b2)) ;
} /* ProcessCtrl_SpecialAssignment */


void BDCC_608_P_ProcessCtrlUnknown(
	BDCC_608_hTranscoder h608Transcoder,
	unsigned char b1, 
	unsigned char b2)
{
	BSTD_UNUSED(h608Transcoder) ;
	BDBG_MSG(("ProcessCtrl_Unknown:  (%02X,%02X)\n", b1,b2)) ;
} /* ProcessCtrl_Unknown */



/**************************************************************************
 *
 * Function:		ProcessCtrl_Tab
 *
 * Inputs:			
 *					h608Transcoder				- object, previously init'ed
 *					b2					- character
 *
 * Outputs:		
 * 					
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * This function performs the handling of the tab misc control codes.
 *
 **************************************************************************/
void BDCC_608_P_ProcessCtrlTab(
	BDCC_608_hTranscoder h608Transcoder, 
	unsigned char b2)
{
	static unsigned char tsp[2] = {0x10,0x20} ;

#if 0
/* SWSTB-9657, xax has TOx which messed up pen location, so bypass TOx if they want, but not recommended  */
return;
#endif

	switch ( b2 )
	{
		case 0x23 :
			BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, tsp, 2) ;
			/* fall thru */
		case 0x22 :
			BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, tsp, 2) ;
			/* fall thru */
		case 0x21 :
			BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, tsp, 2) ;
			break ;
		default :
			BDBG_ERR(("ProcessCtrl_Tab unknown code %02x\n", b2)) ;
			break ;
	}
}


/**************************************************************************
 *
 * Function:		EraseDisplayedMem
 *
 * Inputs:			
 *					h608Transcoder				- object, previously init'ed
 *
 * Outputs:		
 * 					
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * EraseDisplayedMem clears the displayed windows.
 *
 **************************************************************************/
void BDCC_608_P_EraseDisplayedMem(BDCC_608_hTranscoder h608Transcoder)
{
	BDCC_608_P_CmdSendClearWindows(h608Transcoder, StateDecisions[h608Transcoder->eState].DisplayedMask) ;
}


/**************************************************************************
 *
 * Function:		EraseNonDisplayedMem
 *
 * Inputs:			
 *					h608Transcoder				- object, previously init'ed
 *
 * Outputs:		
 * 					
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * EraseNonDisplayedMem clears the non-displayed windows.
 *
 **************************************************************************/
void BDCC_608_P_EraseNonDisplayedMem(BDCC_608_hTranscoder h608Transcoder)
{
	BDCC_608_P_CmdSendClearWindows(h608Transcoder, (unsigned char)(~(StateDecisions[h608Transcoder->eState].DisplayedMask))) ;
}


/**************************************************************************
 *
 * Function:		CarriageReturn
 *
 * Inputs:			
 *					h608Transcoder				- object, previously init'ed
 *
 * Outputs:		
 * 					
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * CR handler.
 *
 **************************************************************************/
void BDCC_608_P_CmdSendCarriageReturn(BDCC_608_hTranscoder h608Transcoder)
{
	unsigned char cr = 0x0d ;
	BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, &cr, 1) ;
	h608Transcoder->fBegLine = 1 ;
}

void BDCC_608_P_CmdSendSpace(BDCC_608_hTranscoder h608Transcoder)
{
	unsigned char sp = 0x20 ;
	BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, &sp, 1) ;
}

void BDCC_608_P_CmdSendBackSpace(BDCC_608_hTranscoder h608Transcoder)
{
	unsigned char sp = 0x08 ;
	BDCC_CBUF_WritePtr(h608Transcoder->pOutBuf, &sp, 1) ;
}

void BDCC_608_P_CmdSendFlip(BDCC_608_hTranscoder h608Transcoder)
{
	BSTD_UNUSED(h608Transcoder) ;
	BDBG_MSG(("Flip\n")) ;
}


/**************************************************************************
 *
 * Function:		WndFromRow
 *
 * Inputs:			
 *					h608Transcoder				- object, previously init'ed
 *					eState				- current 608 state
 *
 * Outputs:		
 *					window according to the row and the state
 * 					
 *
 * Returns:			<void>
 *
 * Description:
 * 
 * WndFromRow returns the window id for the given row and state.
 *
 **************************************************************************/
int BDCC_608_P_WndFromRow(
	BDCC_608_hTranscoder h608Transcoder, 
	int Row, 
	BDCC_608_P_State eState)
{
	int wnd ;
	int offset ;

	if ( eState == BDCC_608_P_State_ePopOn2 || eState == BDCC_608_P_State_ePaintOn1 )
		offset = 0 ;
	else
		offset = 4 ;

	for ( wnd=offset ; wnd < offset+4 ; wnd++ )
	{
		if ( h608Transcoder->WndInfo[wnd].fRowAssigned   &&   h608Transcoder->WndInfo[wnd].RowAssigned == Row )
		{
			/* already have a wnd assigned to this row */
			return(wnd) ;
		}
	}

	for ( wnd=offset ; wnd < offset+4 ; wnd++ )
	{
		if ( ! h608Transcoder->WndInfo[wnd].fRowAssigned )
		{
			h608Transcoder->WndInfo[wnd].fRowAssigned = 1 ;
			h608Transcoder->WndInfo[wnd].RowAssigned = Row ;
			return(wnd) ;
		}
	}

	BDBG_ERR(("WndFromRow:  dccError can't assign wnd for row %d\n", Row)) ;
	return(0) ;
}

void BDCC_608_P_ResetRows(
	BDCC_608_hTranscoder h608Transcoder, 
	unsigned char WndMask)
{
	int wnd ;
	unsigned char mask ;

	for ( wnd=0 , mask=1 ; wnd < 8 ; wnd++ , mask <<= 1 )
	{
		if ( mask & WndMask )
		{
			h608Transcoder->WndInfo[wnd].fRowAssigned = 0 ;
		}
	}
}


void BDCC_608_P_ConsiderTextOrXDS(
	unsigned char b1,
	unsigned char b2,
	int * pTextOrXDSMode)
{
	b1 &= 0x7F ;
	b2 &= 0x7F ;
	switch ( b1 )
	{
		case 0x14 :
		case 0x15 :
		case 0x1c :
		case 0x1d:
			switch ( b2 )
			{
				case BDCC_608_P_CtrlMisc_eEOC :
				case BDCC_608_P_CtrlMisc_eRCL :
				case BDCC_608_P_CtrlMisc_eRDC :
				case BDCC_608_P_CtrlMisc_eRU2 :
				case BDCC_608_P_CtrlMisc_eRU3 :
				case BDCC_608_P_CtrlMisc_eRU4 :
					/* caption mode */
					BDBG_MSG(("ConsiderTextOrXDS:  b2=%02X  CAPTION\n", b2)) ;
					*pTextOrXDSMode = 0 ;
					break ;

				case BDCC_608_P_CtrlMisc_eRTD :
				case BDCC_608_P_CtrlMisc_eTR :
					/* text mode */
					BDBG_MSG(("ConsiderTextOrXDS:  b2=%02X  TEXT\n", b2)) ;
					*pTextOrXDSMode = 1 ;
					break ;
			}
			break ;

		case 0x0f :
			/* end of XDS mode */
			BDBG_MSG(("ConsiderTextOrXDS:  b1=%02X  END XDS -> CAPTION\n", b1)) ;
			*pTextOrXDSMode = 0 ;
			break ;
			
		default :
			if ( (b1 >= 0x01) && (b1 <= 0x0F) )
			{
				/* XDS mode */
				BDBG_MSG(("ConsiderTextOrXDS:  b1=%02X  XDS\n", b1)) ;
				*pTextOrXDSMode = 2 ;
			}
			else
			{
				/* all other control codes don't change the mode */
				BDBG_MSG(("ConsiderTextOrXDS:  b1=%02X  No Change %d\n", b1, *pTextOrXDSMode)) ;
			}
	}

}



