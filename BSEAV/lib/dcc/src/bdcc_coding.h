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
 * Header file for the Coding module of ccgfx.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/  


#ifndef BDCC_CODING_H
#define BDCC_CODING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bdcc.h"
#include "bdcc_cbuf.h"
#include "bdcc_gfx.h"


#define BDCC_COD_P_TEXT_SEGMENT_LEN 100
#define BDCC_COD_P_NUM_TEXT_SEGMENTS 42

typedef enum BDCC_COD_P_ProcessDisposition
{
    BDCC_COD_P_ProcessDisposition_eConsumeContinue,
    BDCC_COD_P_ProcessDisposition_eConsumeDone,
    BDCC_COD_P_ProcessDisposition_eDone,
    BDCC_COD_P_ProcessDisposition_eConsumePause
} BDCC_COD_P_ProcessDisposition;

typedef struct  BDCC_INT_P_PenAttibutes
{
    BDCC_PenSize PenSize;
    BDCC_FontStyle FontStyle;
    unsigned int TextTag;
    BCCGFX_P_VertOffset Offset;
    bool Italics;
    bool Underline;
    BDCC_Edge EdgeType;
    unsigned char CmdArgs[2];

} BDCC_INT_P_PenAttibutes;

typedef struct
{
    unsigned int FgColor;
    BDCC_Opacity FgOpacity;
    unsigned int BgColor;
    BDCC_Opacity BgOpacity;
    unsigned int EdgeColor;
    unsigned char CmdArgs[3];

} BDCC_INT_P_PenColor;



typedef struct
{
    /* attributes */
    BDCC_INT_P_PenAttibutes Attr;

    /* color */
    BDCC_INT_P_PenColor Color;
    
    int Row;
    int Col;

} BDCC_INT_P_Pen;

typedef struct
{
    unsigned int RowLock;
    unsigned int ColLock;
    unsigned int Priority;
    unsigned int RelativePos;
    unsigned int AnchorVert;
    unsigned int AnchorHorz;
    BCCGFX_P_AnchorPoint AnchorPoint;
    int RowCount;
    int ColCount;

    /* the following are ignored by ccgfxCreateWindow */
    unsigned int Visible;
    unsigned int WndStyleID;
    unsigned int PenStyleID;
} BDCC_INT_P_WindowDef;

typedef struct
{
    BCCGFX_P_Justify Justify;
    BCCGFX_P_Direction PrintDirection;
    BCCGFX_P_Direction ScrollDirection;
    bool WordWrap;
    BCCGFX_P_Effect DisplayEffect;
    BCCGFX_P_Direction EffectDirection;
    unsigned int EffectSpeed;
    unsigned int FillColor;
    BDCC_Opacity FillOpacity;
    BDCC_Edge BorderType;
    unsigned int BorderColor;
} BDCC_INT_P_WindowAttr;
   
typedef struct
{
    unsigned char ch;
    unsigned char PenAttrArgs[2];
    unsigned char PenColorArgs[3];
} BDCC_INT_P_TsElement;

typedef struct
{
    int fRenderNeeded;
    int fClrRowNeeded;
    int iBeyondLastChar;
    int iLastBeyondLastChar;
    int fContainsFlash;
    BDCC_INT_P_TsElement aCharInfo[BDCC_COD_P_TEXT_SEGMENT_LEN];
} BDCC_INT_P_TextSegment;

typedef struct
{
    int TsRowScrollAdj;
    int SavedRowCount;
    BDCC_INT_P_TextSegment	aTextSeg[BDCC_COD_P_NUM_TEXT_SEGMENTS];
} BDCC_INT_P_WndBuf;

typedef struct BDCC_INT_P_Window
{
    int WndId;
    int fDefined;
    unsigned int UpdateMask;

    /* Define Window */
    BDCC_INT_P_WindowDef WindowDef;

    /* Window Attributes */
    BDCC_INT_P_WindowAttr WindowAttr;

    /* Pen */
    BDCC_INT_P_Pen Pen;

    B_Dcc_OverRides SaveOverridden;

    int fPendingScroll;

    BDCC_INT_P_WndBuf WndBuf;
    int ccgfxCreated;
    int rWindowAnchor;
    int cWindowAnchor;

} BDCC_INT_P_Window;

#define BDCC_INT_P_NUM_WINDOWS 8
#define BDCC_INT_P_SIB_SIZE 128

typedef struct  
{     
    /* The CCGFX Winlib module handle */
    BDCC_WINLIB_Handle hWinLibHandle;
    /* The CCGFX module handle */
    BCCGFX_P_GfxHandle hCCGfxHandle;

    int CurrentWindow;
    unsigned int DefinedWindowsMask;
    BDCC_INT_P_Window	WindowInfo[BDCC_INT_P_NUM_WINDOWS];

    unsigned int            fDelay;
    unsigned int SIBuf_StreamBytesWritten;
    unsigned int SIBuf_StreamBytesPosted;
    unsigned int SIBuf_NumCmdsWritten;
    unsigned int SIBuf_NumCmdsPosted;

    unsigned int OverrideMask;
    B_Dcc_OverRides Overrides;

    unsigned int SILimit;
    unsigned int uiColumns;
    B_Dcc_Type Type;

#ifdef FCC_MIN_DECODER
    uint32_t AgeList;
#endif
} BDCC_INT_P_Object;


typedef BDCC_INT_P_Object *BDCC_INT_P_Handle;

#define BDCC_INT_P_MAKECOLOR222(r,g,b)  (((r)<<4) | ((g)<<2) | (b))


/**************************************************************************
 *
 * Function:        BDCC_Coding_P_Open
 *
 * Inputs:
 *                  SILimit         - size for overflow, norm 128
 *
 * Outputs:
 *                  hCodObject          - Handle to object
 *
 * Returns:         dccSuccess or a standard BDCC_Error error code
 *
 * Description:
 *
 * This function initializes the object structure.
 *
 **************************************************************************/
BDCC_Error
BDCC_Coding_P_Open(
    BDCC_INT_P_Handle *phCodObject,
    BDCC_WINLIB_Handle hWinLibHandle,
    BCCGFX_P_GfxHandle hCCGfxHandle,
    unsigned int SILimit,
    unsigned int uiColumns,
    B_Dcc_Type Type
    );


BDCC_Error BDCC_Coding_P_ScreenClear(
    BDCC_INT_P_Handle hCodObject
    );


/**************************************************************************
 *
 * Function:        BDCC_Coding_P_Reset
 *
 * Inputs:
 *
 * Outputs:
 *                  hCodObject          - Handle to object 
 *
 * Returns:         dccSuccess or a standard BDCC_Error error code
 *
 * Description:
 * 
 * This function resets the Coding/ServiceInputBuffer/Interpretation layers.
 *
 **************************************************************************/
BDCC_Error
BDCC_Coding_P_Reset(
BDCC_INT_P_Handle hCodObject,
B_Dcc_Type Type
);


/**************************************************************************
 *
 * Function:        BDCC_Coding_P_Override
 *
 * Inputs:
 *
 * Outputs:
 *                  hCodObject	- Handle to object 
 *                  OverrideMask	- bitmask of overridden attributes
 *                  pOverrides	- structure of overrides
 *
 * Returns:         dccSuccess or a standard BDCC_Error error code
 *
 * Description:
 * 
 * This function sets the overridden attributes.
 *
 **************************************************************************/
BDCC_Error
BDCC_Coding_P_Override(
    BDCC_INT_P_Handle hCodObject,
    unsigned int OverrideMask,
    const B_Dcc_OverRides *pOverrides
    );


/**************************************************************************
 *
 * Function:        BDCC_Coding_P_Close
 *
 * Inputs:
 *
 * Outputs:
 *                  hCodObject	- Handle to object
 *
 * Returns:         dccSuccess or a standard BDCC_Error error code
 *
 * Description:
 *
 * This function un-initializes the object structure.
 *
 **************************************************************************/
BDCC_Error
BDCC_Coding_P_Close(
    BDCC_INT_P_Handle hCodObject
    );


/**************************************************************************
 *
 * Function:        DccSIBuf_Process
 *
 * Inputs:
 *                  hCodObject  - Handle to object
 *                  pInBuf      - input circular buffer
 *
 * Outputs:
 *
 * Returns:         dccSuccess or a standard BDCC_Error error code
 *
 * Description:
 * 
 * This function reads the input service blocks and outputs to the
 * Service Input Buffer.  It processes the Delay and Delay Cancel commands.
 *
 **************************************************************************/
BDCC_Error
BDCC_SIBuf_P_Process(
    BDCC_INT_P_Handle hCodObject,
    BDCC_CBUF *pInBuf,
    BDCC_CBUF *pOutBuf
    );


/**************************************************************************  
 *
 * Function:        BDCC_Coding_P_Process
 *
 * Inputs:
 *                  hCodObject      - Handle to object
 *                  pInBuf	        - input circular buffer
 *
 * Outputs:
 *
 * Returns:         dccSuccess or a standard BDCC_Error error code
 *
 * Description:
 *
 * BDCC_Coding_P_Process reads the input stream, consisting of codes
 * from a single DTVCC service, and makes the appropriate calls to the
 * CCGfx Closed Captioning Graphics Library.
 *
 * Partial commands will not be consumed from the input stream.  On a
 * subsequent call, when the entire command is in the input buffer, the
 * command will be consumed and processed.
 *
 **************************************************************************/
BDCC_Error
BDCC_Coding_P_Process(
    BDCC_INT_P_Handle hCodObject,
    BDCC_CBUF *pInBuf
    );

BDCC_Error BDCC_Update_Delay(BDCC_INT_P_Handle hCodObject, BDCC_CBUF *pOutBuf);

#ifdef __cplusplus
}
#endif
  
#endif /* BDCC_CODING_H */

