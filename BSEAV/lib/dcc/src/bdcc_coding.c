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
#include "bdcc_coding.h"
#include "bdcc_coding_c1_handlers.h"
#include "bdcc_int.h"
#include "bdcc_priv.h"
#include "bdcc_gfx.h"
#include "b_dcc_lib.h"

BDBG_MODULE(BDCC_CODING);


#define clNeedSecondByte (0xFFFFFFFF)
#define clNeedThirdByte  (0xFFFFFFFE)
#define BDCC_COD_P_CMD_03_ETX 0x03

static BDCC_COD_P_ProcessDisposition ProcessCmd(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char * pAddlBytes
    );
static BDCC_COD_P_ProcessDisposition ProcessCmd_WindowText(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char * pAddlBytes
    );
static unsigned int ClassifyFirstByte(
    unsigned char cmd
    );
static unsigned int ClassifyBothBytes(
    unsigned char cmd,
    unsigned char next
    );
static unsigned int ClassifyThreeBytes(
    unsigned char cmd,
    unsigned char b2,
    unsigned char b3
    );
static BDCC_COD_P_ProcessDisposition SIBuf_ProcessCmd(
    BDCC_INT_P_Handle hCodObject,
    BDCC_CBUF * pOutBuf,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char * pAddlBytes
    );

#ifdef __cplusplus
extern "C" {
#endif
typedef BDCC_COD_P_ProcessDisposition (* BDCC_COD_P_CmdProcPfunc)(BDCC_INT_P_Handle, unsigned char, unsigned int, unsigned char *);

typedef struct BDCC_COD_P_C1Info
{
    BDCC_COD_P_CmdProcPfunc	pFunc;
    const char *szC1Name;
} BDCC_COD_P_C1Info;

static const BDCC_COD_P_C1Info BDCC_COD_P_C1Handlers[] =
{
    { BDCC_COD_P_ProcessCmd_C1_SetCurrentWindow,    /* cmd = 0x80 */    "SetCurrentWindow 0" },
    { BDCC_COD_P_ProcessCmd_C1_SetCurrentWindow,    /* cmd = 0x81 */    "SetCurrentWindow 1" },
    { BDCC_COD_P_ProcessCmd_C1_SetCurrentWindow,    /* cmd = 0x82 */    "SetCurrentWindow 2" },
    { BDCC_COD_P_ProcessCmd_C1_SetCurrentWindow,    /* cmd = 0x83 */    "SetCurrentWindow 3" },
    { BDCC_COD_P_ProcessCmd_C1_SetCurrentWindow,    /* cmd = 0x84 */    "SetCurrentWindow 4" },
    { BDCC_COD_P_ProcessCmd_C1_SetCurrentWindow,    /* cmd = 0x85 */    "SetCurrentWindow 5" },
    { BDCC_COD_P_ProcessCmd_C1_SetCurrentWindow,    /* cmd = 0x86 */    "SetCurrentWindow 6" },
    { BDCC_COD_P_ProcessCmd_C1_SetCurrentWindow,    /* cmd = 0x87 */    "SetCurrentWindow 7" },

    { BDCC_COD_P_ProcessCmd_C1_ClearWindows,        /* cmd = 0x88 */    "ClearWindows" },
    { BDCC_COD_P_ProcessCmd_C1_DisplayWindows,      /* cmd = 0x89 */    "DisplayWindows" },
    { BDCC_COD_P_ProcessCmd_C1_HideWindows,         /* cmd = 0x8A */    "HideWindows" },
    { BDCC_COD_P_ProcessCmd_C1_ToggleWindows,       /* cmd = 0x8B */    "ToggleWindows" },
    { BDCC_COD_P_ProcessCmd_C1_DeleteWindows,       /* cmd = 0x8C */    "DeleteWindows" },
    { BDCC_COD_P_ProcessCmd_C1_Delay,               /* cmd = 0x8D */    "Delay" },
    { BDCC_COD_P_ProcessCmd_C1_DelayCancel,         /* cmd = 0x8E */    "DelayCancel" },
    { BDCC_COD_P_ProcessCmd_C1_Reset,               /* cmd = 0x8F */    "Reset" },

    { BDCC_COD_P_ProcessCmd_C1_SetPenAttributes,    /* cmd = 0x90 */    "SetPenAttributes" },
    { BDCC_COD_P_ProcessCmd_C1_SetPenColor,         /* cmd = 0x91 */    "SetPenColor" },
    { BDCC_COD_P_ProcessCmd_C1_SetPenLocation,      /* cmd = 0x92 */    "SetPenLocation" },
    { BDCC_COD_P_ProcessCmd_C1_Stub,                /* cmd = 0x93 */    "Stub" },
    { BDCC_COD_P_ProcessCmd_C1_Stub,                /* cmd = 0x94 */    "Stub" },
    { BDCC_COD_P_ProcessCmd_C1_Stub,                /* cmd = 0x95 */    "Stub" },
    { BDCC_COD_P_ProcessCmd_C1_Stub,                /* cmd = 0x96 */    "Stub" },
    { BDCC_COD_P_ProcessCmd_C1_SetWindowAttributes, /* cmd = 0x97 */    "SetWindowAttributes" },

    { BDCC_COD_P_ProcessCmd_C1_DefineWindow,        /* cmd = 0x98 */    "DefineWindow 0" },
    { BDCC_COD_P_ProcessCmd_C1_DefineWindow,        /* cmd = 0x99 */    "DefineWindow 1" },
    { BDCC_COD_P_ProcessCmd_C1_DefineWindow,        /* cmd = 0x9A */    "DefineWindow 2" },
    { BDCC_COD_P_ProcessCmd_C1_DefineWindow,        /* cmd = 0x9B */    "DefineWindow 3" },
    { BDCC_COD_P_ProcessCmd_C1_DefineWindow,        /* cmd = 0x9C */    "DefineWindow 4" },
    { BDCC_COD_P_ProcessCmd_C1_DefineWindow,        /* cmd = 0x9D */    "DefineWindow 5" },
    { BDCC_COD_P_ProcessCmd_C1_DefineWindow,        /* cmd = 0x9E */    "DefineWindow 6" },
    { BDCC_COD_P_ProcessCmd_C1_DefineWindow,        /* cmd = 0x9F */    "DefineWindow 7" },
} ;
#ifdef __cplusplus
}
#endif


static BDCC_Error
BDCC_Coding_P_Init(
    BDCC_INT_P_Handle hCodObject,
    BDCC_WINLIB_Handle hWinLibHandle,
    BCCGFX_P_GfxHandle hCCGfxHandle,
    unsigned int SILimit,
    unsigned int uiColumns,
    B_Dcc_Type Type
    )
{
    int wnd ;

    BDBG_ASSERT((hCodObject));

    BKNI_Memset(hCodObject, 0, sizeof(BDCC_INT_P_Object)) ;

    hCodObject->hWinLibHandle = hWinLibHandle ;
    hCodObject->hCCGfxHandle = hCCGfxHandle ;
    for ( wnd=0 ; wnd < BDCC_INT_P_NUM_WINDOWS ; wnd++ )
    {
        hCodObject->WindowInfo[wnd].WndId = wnd ;
    }
    hCodObject->SILimit = SILimit ;
    hCodObject->uiColumns = uiColumns ;
    hCodObject->Type = Type;
#ifdef FCC_MIN_DECODER
    hCodObject->AgeList = 0x76543210;
#endif
    return(BDCC_Error_eSuccess) ;
}


/**************************************************************************     
 *
 * Function:        BDCC_Coding_P_Open
 *
 * Inputs:          hCodObject	        - Handle to object
 *                  Type                - 608 or 708 caption type
 *                  SILimit         - size for overflow, norm 128
 *
 * Outputs:
 *
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
    )
{
    BDCC_INT_P_Handle hCodObject;

    BDBG_ASSERT((phCodObject));
    BDBG_ASSERT((hWinLibHandle));
    BDBG_ASSERT((hCCGfxHandle));
    
    /* allocate the Object */
    *phCodObject = hCodObject = (BDCC_INT_P_Handle)BKNI_Malloc( sizeof(BDCC_INT_P_Object));
    if(hCodObject == NULL)
    {
        return BDCC_Error_eNoMemory;
    }

    return(BDCC_Coding_P_Init(hCodObject, hWinLibHandle, hCCGfxHandle, SILimit, uiColumns, Type) );
}


/*
** Clear all the CC windows and then refresh the screen.
*/
BDCC_Error BDCC_Coding_P_ScreenClear(
    BDCC_INT_P_Handle hCodObject
    )
{
    BDCC_INT_P_Window *pWI;
    unsigned char WndMask = 0x00 ;
	int i;
    
	BDBG_ASSERT((hCodObject));

    /* EIA-608 and CEB-10 recommend only clearing and hiding the visible windows.
    ** Non-visible windows may contain captions yet to be displayed.
    */
    for( i = 0 ; i < 8 ; i++)
    {
        WndMask >>= 1;
        pWI = &hCodObject->WindowInfo[i] ;
        if(pWI->WindowDef.Visible) WndMask |= 0x80;
	}
    
    BDCC_COD_P_ProcessCmd_C1_ClearWindows(hCodObject, 0x88, 1, &WndMask);
	BDCC_COD_P_ProcessCmd_C1_HideWindows(hCodObject, 0x8a, 1, &WndMask) ;
    BCCGFX_P_RedrawScreen( hCodObject->hCCGfxHandle );

    return(BDCC_Error_eSuccess);
}


/**************************************************************************
 *
 * Function:        BDCC_Coding_P_Reset
 *
 * Inputs:          hCodObject          - Handle to object
 *                  Type                - 608 or 708 caption type
 *
 * Outputs:
 *
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
    )
{
    unsigned char WndMask = 0xFF;
    unsigned int OverrideMask;
    B_Dcc_OverRides Overrides;
    
    BDBG_ASSERT((hCodObject));
    BDCC_COD_P_ProcessCmd_C1_ClearWindows(hCodObject, 0x88, 1, &WndMask);
    BDCC_COD_P_ProcessCmd_C1_DeleteWindows(hCodObject, 0x8C, 1, &WndMask);
    BCCGFX_P_RedrawScreen( hCodObject->hCCGfxHandle );

    /* 
    ** Save the overrides.
    */
    OverrideMask = hCodObject->OverrideMask;
    Overrides = hCodObject->Overrides;

    BDCC_Coding_P_Init(hCodObject, hCodObject->hWinLibHandle,hCodObject->hCCGfxHandle,
                            hCodObject->SILimit, hCodObject->uiColumns, Type);

    /* 
    ** re-apply the overrides
    */
    BDCC_Coding_P_Override(hCodObject, OverrideMask, &Overrides);
    return(BDCC_Error_eSuccess);
}


void RenderWnd(BDCC_INT_P_Handle hCodObject, BDCC_INT_P_Window * pw);
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
    BDCC_INT_P_Handle	hCodObject,
    unsigned int OverrideMask,
    const B_Dcc_OverRides *pOverrides
    )
{
    int i;

    BDBG_ASSERT((hCodObject));
    hCodObject->OverrideMask = OverrideMask;
    hCodObject->Overrides = *pOverrides;

    for( i = 0 ; i < 8 ; i++)
    {
            BDCC_INT_P_Window * pWI = &hCodObject->WindowInfo[i];
            if(pWI->fDefined)   RenderWnd(hCodObject, pWI);
        }
    return(BDCC_Error_eSuccess);
}



/**************************************************************************
 *
 * Function:        BDCC_Coding_P_Close
 *
 * Inputs:
 *
 * Outputs:
 *                  hCodObject  - Handle to object
 *
 * Returns:         dccSuccess or a standard BDCC_Error error code
 *
 * Description:
 *
 * This function closes the module 
 *
 **************************************************************************/
BDCC_Error      
BDCC_Coding_P_Close(
    BDCC_INT_P_Handle hCodObject
    )
{
    BDBG_ASSERT((hCodObject));
    
    BKNI_Free(hCodObject);
    return(BDCC_Error_eSuccess);
}


/**************************************************************************
 *
 * Function:        BDCC_SIBuf_P_Process
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
    )
{
    unsigned int NumBytes, AddlBytes;
    unsigned char curByte, Byte2, Byte3, cmd;
    unsigned int curIndex;
    unsigned int cl;
    unsigned char * pAddlBytes;
    BDCC_COD_P_ProcessDisposition pd;

    BDBG_ASSERT((hCodObject));

    BDCC_CBUF_ResetPeek(pInBuf);
    NumBytes = pInBuf->NumBytes;
    BDBG_MSG(("%s,NumBytes %d",__FUNCTION__,NumBytes));
    if ( hCodObject->fDelay )
    {
        if ( BCCGFX_P_TimerQuery(hCodObject->hCCGfxHandle) )
        {
            /* expired */
            hCodObject->fDelay = 0;
            BDCC_CBUF_UpdatePost(pOutBuf);

            hCodObject->SIBuf_StreamBytesWritten += hCodObject->SIBuf_StreamBytesPosted;
            hCodObject->SIBuf_StreamBytesPosted = 0;
            hCodObject->SIBuf_NumCmdsWritten += hCodObject->SIBuf_NumCmdsPosted;
            hCodObject->SIBuf_NumCmdsPosted = 0;
        }
    }

    for ( curIndex=0 ; curIndex < NumBytes ; curIndex++ )
    {
        curByte = BDCC_CBUF_PeekByte(pInBuf);
        cl = ClassifyFirstByte(curByte);
        if ( cl == clNeedSecondByte )
        {
            if ( (curIndex + 2) > NumBytes )
            {
                /* 2nd byte not in input */
                return(BDCC_Error_eSuccess);
            }
            Byte2 = BDCC_CBUF_PeekByte(pInBuf);
            cl = ClassifyBothBytes(curByte, Byte2);
            if ( cl == clNeedThirdByte )
            {
                if ( (curIndex + 3) > NumBytes )
                {
                    /* 3rd byte not in input */
                    return(BDCC_Error_eSuccess);
                }
                Byte3 = BDCC_CBUF_PeekByte(pInBuf);
                cl = ClassifyThreeBytes(curByte, Byte2, Byte3);
            }
        }

        BDCC_CBUF_ResetPeek(pInBuf);
        cmd = BDCC_CBUF_PeekByte(pInBuf);
        AddlBytes = cl ; /* cl is, in fact, AddlBytes */
        BDBG_MSG(( "curbyte 0x%02x   addlbytes 0x%02x  fDelay %d\n", curByte, AddlBytes, hCodObject->fDelay));
        if ( (curIndex + 1 + AddlBytes) > NumBytes )
        {
            /* not all addl bytes in input buf */
            return(BDCC_Error_eSuccess);
        }
        pAddlBytes = BDCC_CBUF_PeekPtr(pInBuf, AddlBytes);
        if ( NULL == pAddlBytes ) /* redundant */
        {
            /* not all addl bytes in input buf */
            return(BDCC_Error_eSuccess);
        }

#if 0
        if ( BDCC_P_DUMP_STREAM_FLAG & BDCC_P_CODING )
        {
            int ab;
            static int seq=0;
            seq++;
            BDCC_DBG_MSG(( "%03d Dump Coding (%02X)", seq % 1000,cmd));

            if ( AddlBytes )
            {
                for ( ab=0 ; ab < AddlBytes ; ab++ )
                {
                    BDCC_DBG_MSG(( "  %02X", pAddlBytes[ab]));
                }
            }
            else if ( cmd >= 0x20   &&   cmd < 0x80 )
            {
                BDCC_DBG_MSG(( "  '%c'", cmd));
            }
            BDCC_DBG_MSG(( "\n"));
        }
#endif

        pd = SIBuf_ProcessCmd(hCodObject, pOutBuf, cmd, AddlBytes, pAddlBytes);

        if ( pd == BDCC_COD_P_ProcessDisposition_eConsumeContinue )
        {
            BDCC_CBUF_UpdatePeek(pInBuf);
        }
        else if ( pd == BDCC_COD_P_ProcessDisposition_eConsumeDone )
        {
            BDCC_CBUF_UpdatePeek(pInBuf);
            break ;
        }
        else if ( pd == BDCC_COD_P_ProcessDisposition_eConsumePause )
        {
            BDCC_CBUF_UpdatePeek(pInBuf);
            BDBG_MSG(( "SIBuf_Proc: Pausing\n"));
            return(BDCC_Error_eWrnPause);
        }
        else /* if ( pd == BDCC_COD_P_ProcessDisposition_eDone ) */
        {
            break;
        }

        /* intentionally modify the 'for' loop index */
        curIndex += AddlBytes;

    } /* for (each input byte) */

    return(BDCC_Error_eSuccess);
} /* DccSIBuf_Process */

/**************************************************************************  
 *
 * Function:        BDCC_Coding_P_Process
 *
 * Inputs:
 *                  hCodObject      - Handle to object
 *                  pInBuf          - input circular buffer
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
    BDCC_CBUF * pInBuf
    )
{
    unsigned int NumCmds;
    unsigned char cmd, AddlBytes;
    unsigned int curIndex;
    unsigned char * pAddlBytes;
    BDCC_COD_P_ProcessDisposition pd;
    bool needETX = false;

    BDBG_ASSERT((hCodObject));
    BDBG_MSG(("Entered %s",__FUNCTION__));
    BDCC_CBUF_ResetPeek(pInBuf) ;
    NumCmds = hCodObject->SIBuf_NumCmdsWritten;

    BDBG_MSG(("%s, NumCmds %d",__FUNCTION__, NumCmds));

    for ( curIndex=0 ; curIndex < NumCmds ; curIndex++ )
    {
        cmd = BDCC_CBUF_PeekByte(pInBuf);
        AddlBytes = BDCC_CBUF_PeekByte(pInBuf);

        pAddlBytes = BDCC_CBUF_PeekPtr(pInBuf, AddlBytes);
        if ( NULL == pAddlBytes ) /* redundant */
        {
            /* not all addl bytes in input buf */
            BDBG_WRN(( "DccCoding_Process: not all addl bytes in input buf\n")) ;
            return(BDCC_Error_eSuccess) ;
        }

#if 1
        if ( BDCC_P_DUMP_STREAM_FLAG & BDCC_P_CODING )
        {
            int ab ;
            BDBG_MSG(( "Dump Coding (%02X)", cmd)) ;

            if ( AddlBytes )
            {
                for ( ab=0 ; ab < AddlBytes ; ab++ )
                {
                    BDBG_MSG(( "  %02X", pAddlBytes[ab])) ;
                }
            }
            else if ( cmd >= 0x20   &&   cmd < 0x80 )
            {
                BDBG_MSG(( "  '%c'", cmd)) ;
            }
            BDBG_MSG(( "\n")) ;
        }
#endif

        if(((cmd >= 0x80) && (cmd <= 0x9f)) && (needETX == true))
        {
           /* EIA708 says that for left justified text, must display each character as it
            * is received. We inject EXT here to accomplish this. If not left justified then
            * it will be filtered out downstream.  Inject here since command may move cursor
            * Y coord, caption row may never be displayed!
            */
            ProcessCmd(hCodObject, BDCC_COD_P_CMD_03_ETX, 0, NULL) ;
            needETX = false;
        }
        else if(cmd >= 32)
        {
            needETX = true;
        }
        else if( cmd == 0x03 || cmd == 0x08 || cmd == 0x0c || cmd == 0x0d || cmd == 0x0e)
        {
            needETX = false;
        }
        
        pd = ProcessCmd(hCodObject, cmd, AddlBytes, pAddlBytes) ;

        if ( pd == BDCC_COD_P_ProcessDisposition_eConsumeContinue )
        {
            BDCC_CBUF_UpdatePeek(pInBuf) ;
            hCodObject->SIBuf_NumCmdsWritten-- ;
            hCodObject->SIBuf_StreamBytesWritten -= 1 + AddlBytes ;
        }
        else if ( pd == BDCC_COD_P_ProcessDisposition_eConsumeDone )
        {
            BDCC_CBUF_UpdatePeek(pInBuf) ;
            hCodObject->SIBuf_NumCmdsWritten-- ;
            hCodObject->SIBuf_StreamBytesWritten -= 1 + AddlBytes ;
            break ;
        }
        else /* if ( pd == BDCC_COD_P_ProcessDisposition_eDone ) */
        {
            break ;
        }
    } /* for (each input byte) */

    /* EIA708 says that for left justified text, must display each character as it
     * is received. We inject EXT here to accomplish this. If not left justified then
     * it will be filtered out downstream
     */
    if ( pd == BDCC_COD_P_ProcessDisposition_eConsumeContinue && needETX)
    {
        ProcessCmd(hCodObject, BDCC_COD_P_CMD_03_ETX, 0, NULL) ;
    }

    return(BDCC_Error_eSuccess) ;
} /* DccCoding_Process */


static BDCC_COD_P_ProcessDisposition 
ProcessCmd(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char * pAddlBytes
    )
{
    BDCC_COD_P_ProcessDisposition pd;
    int C1HandlerIndex;

    BDBG_ASSERT((hCodObject));

    BDBG_MSG(( "ProcessCmd:  processing cmd 0x%02X\n", cmd));
    switch ( cmd & 0xF0 )
    {
        case 0x80:
        case 0x90:
            /*
             * C1 Code Space
             *
             * First, push thru an ETX code which flushes any
             * pending Text Segment.  Second, call the C1 
             * handler.
             */

            /*
            ** The "end-of-text" (BDCC_COD_P_CMD_03_ETX) character has 
            ** implications for when to refresh the screen, inserting these at "random" 
            ** is killing performance.
            ** We need to use other triggers to initiate redrawing. 7/7/06
            */
#if 0
            if ( cmd != 0x90   &&   cmd != 0x91 )
                ProcessCmd_WindowText(hCodObject, BDCC_COD_P_CMD_03_ETX, 0, NULL);
#endif
            C1HandlerIndex = cmd & 0x1F ;
            BDBG_MSG(( BDCC_COD_P_C1Handlers[C1HandlerIndex].szC1Name));
            BDBG_MSG(( "\n"));
            pd = BDCC_COD_P_C1Handlers[C1HandlerIndex].pFunc(hCodObject, cmd, AddlBytes, pAddlBytes);
            break;

        default:
            pd = ProcessCmd_WindowText(hCodObject, cmd, AddlBytes, pAddlBytes);
            break;

    } /* switch ( cmd ) */

    return(pd);
} /* ProcessCmd */


static BDCC_COD_P_ProcessDisposition 
ProcessCmd_WindowText(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    BDCC_INT_P_Window *pw;
    BDCC_INT_P_Pen *pp;
    int wnd;

    BDBG_ASSERT((hCodObject));

    wnd = hCodObject->CurrentWindow;
    pw = &hCodObject->WindowInfo[wnd];
    pp = &pw->Pen;

    BDBG_MSG(( "ProcessCmd_WindowText 0x%02x  (%c)\n", cmd, cmd));
    BDCC_INT_P_AccumulateChar(hCodObject, pw, cmd, AddlBytes, pAddlBytes);
    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);

} /* ProcessCmd_Other */


unsigned int ClassifyFirstByte(
    unsigned char cmd
    )
{
    unsigned int AddlBytes;

    switch ( cmd )
    {
        case 0x10:
            AddlBytes = clNeedSecondByte;
            break;

        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
            AddlBytes = 1;
            break;

        case 0x18:
        case 0x19:
        case 0x1a:
        case 0x1b:
        case 0x1c:
        case 0x1d:
        case 0x1e:
        case 0x1f:
            AddlBytes = 2;
            break;

        case 0x80:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
            AddlBytes = 0;
            break;

        case 0x88:
        case 0x89:
        case 0x8a:
        case 0x8b:
        case 0x8c:
            AddlBytes = 1;
            break;

        case 0x8d:
            AddlBytes = 1;
            break;

        case 0x8e:
        case 0x8f:
            AddlBytes = 0;
            break;


        case 0x90:
            AddlBytes = 2;
            break ;

        case 0x91:
            AddlBytes = 3;
            break;

        case 0x92:
            AddlBytes = 2;
            break;


        case 0x97:
            AddlBytes = 4;
            break;

        case 0x98:
        case 0x99:
        case 0x9a:
        case 0x9b:
        case 0x9c:
        case 0x9d:
        case 0x9e:
        case 0x9f:
            AddlBytes = 6;
            break;


        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        default:
            AddlBytes = 0;
            break;
    }

    return(AddlBytes);
} /* ClassifyFirstByte */


unsigned int ClassifyBothBytes(
    unsigned char cmd,
    unsigned char next
    )
{
    unsigned int AddlBytes = 0;

    if ( cmd == 0x10 )
    {
    /* upper 5 bits matter */
        switch ( next & 0xF8 )
        {
            case 0x00 : AddlBytes = 1; break;
            case 0x08 : AddlBytes = 2; break;
            case 0x10 : AddlBytes = 3; break;
            case 0x18 : AddlBytes = 4; break;

            case 0x20:
            case 0x28:
            case 0x30:
            case 0x38:
            case 0x40:
            case 0x48:
            case 0x50:
            case 0x58:
            case 0x60:
            case 0x68:
            case 0x70:
            case 0x78: AddlBytes = 1; break;

            case 0x80: AddlBytes = 5; break;
            case 0x88: AddlBytes = 6; break;


            case 0x90:
            case 0x98:
                /* variable length */
                AddlBytes = clNeedThirdByte; break;
                break;

            case 0xa0:
            case 0xa8:
            case 0xb0:
            case 0xb8:
            case 0xc0:
            case 0xc8:
            case 0xd0:
            case 0xd8:
            case 0xe0:
            case 0xe8:
            case 0xf0:
            case 0xf8: AddlBytes = 1; break;
        }
    }

    return(AddlBytes);
} /* ClassifyBothBytes */


unsigned int ClassifyThreeBytes(
    unsigned char cmd,
    unsigned char b2,
    unsigned char b3
    )
{
    unsigned int AddlBytes = 0;

    if ( cmd == 0x10 )
    {
        if ( (b2 & 0xF0) == 0x90 )
        {
            AddlBytes = (b3 & 0x3F) + 2;
        }
    }

    return(AddlBytes);
} /* ClassifyThreeBytes */


BDCC_COD_P_ProcessDisposition 
SIBuf_ProcessCmd(
    BDCC_INT_P_Handle hCodObject,
    BDCC_CBUF * pOutBuf,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char * pAddlBytes
    )
{
    BDBG_ASSERT((hCodObject));

    if ( cmd == 0x8D ) /* DELAY cmd */
    {
        unsigned int msDelayVal;
        if ( hCodObject->fDelay )
        {
            /* already delayed */
            return(BDCC_COD_P_ProcessDisposition_eDone);
        }

        hCodObject->fDelay = 1;

        /*
        ** The CC data is in 10th's fo seconds, the timer wants milliseconds.
        */ 
        msDelayVal = ( *pAddlBytes * 100 );
        BCCGFX_P_TimerStart( hCodObject->hCCGfxHandle, msDelayVal );

        /* force an ETX */
        /* TODO: is this a good thing to do? */
        cmd = 0x03;
        AddlBytes = 0;
    }
    else if ( cmd == 0x8E ) /* CANCEL cmd */
    {
        BCCGFX_P_TimerCancel(hCodObject->hCCGfxHandle);
        if ( hCodObject->fDelay )
            BDCC_CBUF_UpdatePost(pOutBuf);
        hCodObject->fDelay = 0;
        /* force an ETX */
        /* TODO: is this a good thing to do? */
        cmd = 0x03;
        AddlBytes = 0;
    }

    if ( cmd == 0x8F ) /* RESET cmd */
    {
        BDBG_MSG(( "SIBuf_ProcessCmd:  RESET cmd\n"));
        BDCC_CBUF_Reset(pOutBuf);
        BDCC_Coding_P_Reset(hCodObject, hCodObject->Type);
        return(BDCC_COD_P_ProcessDisposition_eConsumeDone);
    }
    else
    {
        /*
         * This is a command that goes into the
         * Service Input Buffer.  The fDelay flag
         * which indicates a current delay command
         * tells us if we should post the cmd or
         * write the command.
         */
        BDCC_CBUF_PostByte(pOutBuf, cmd);
        BDCC_CBUF_PostByte(pOutBuf, (unsigned char)AddlBytes);
        if ( AddlBytes )
        {
            BDCC_CBUF_PostPtr(pOutBuf, pAddlBytes, AddlBytes);
        }
        hCodObject->SIBuf_StreamBytesPosted += 1 + AddlBytes;
        hCodObject->SIBuf_NumCmdsPosted += 1;

        /*
        ** Per the EIA guideline, a DELAY operation should be canceled when the 
        ** Service Input Buffer is full.  In this situation, the buffer is considered
        ** full when it contains 128 bytes of data (even if the buffer is larger).  
        ** By defining "full" as 128 bytes, the DELAY operation will have consistent
        ** behavior on all platforms.
        */
        if ( hCodObject->fDelay && ((hCodObject->SIBuf_StreamBytesWritten + hCodObject->SIBuf_StreamBytesPosted) >= 128 ))
        {
            hCodObject->fDelay = false;
        }

        /*
         * If we're not currently delaying, then update the
         * previous posts (making them visible to the next
         * layer).
         */
        if ( ! hCodObject->fDelay )
        {
            BDBG_MSG(( "not delaying, so UpdatePost\n"));
            BDCC_CBUF_UpdatePost(pOutBuf);
            hCodObject->SIBuf_StreamBytesWritten += hCodObject->SIBuf_StreamBytesPosted;
            hCodObject->SIBuf_StreamBytesPosted = 0;
            hCodObject->SIBuf_NumCmdsWritten += hCodObject->SIBuf_NumCmdsPosted;
            hCodObject->SIBuf_NumCmdsPosted = 0;

        }
        else
        {
            BDBG_MSG(( "delaying so dont updatepost\n"));
        }

        /*
         * Reset the Service Input Buffer if it's full.
         */
        if ( (hCodObject->SIBuf_StreamBytesWritten + hCodObject->SIBuf_StreamBytesPosted) >= hCodObject->SILimit )
        {
            /* Service Input Buffer is said to be Full 
            * eventhough the actual buffer is not */
            BDBG_MSG(( "SIBuf_ProcessCmd:  Service Input Buffer full\n"));
            BDCC_CBUF_Reset(pOutBuf) ;
            BDCC_Coding_P_Reset(hCodObject, hCodObject->Type);
            return(BDCC_COD_P_ProcessDisposition_eConsumeDone);
        }

        /*
         * Otherwise, check if we should let the lower layers catch up.
         */
        else if ( (hCodObject->SIBuf_StreamBytesWritten + hCodObject->SIBuf_StreamBytesPosted) >= (hCodObject->SILimit / 2) )
        {
            BDBG_MSG(( "SIBuf_ProcessCmd:  pausing\n"));
            return(BDCC_COD_P_ProcessDisposition_eConsumePause);
        }
    }

    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);

} /* SIBuf_ProcessCmd */


/**************************************************************************
 *  
 * Function:        BDCC_Update_Delay
 * 
 * Inputs:
 * 					hCodObject  - Handle to object
 *
 * Outputs:
 * 
 * Returns:         dccSuccess
 * 
 * Description:
 * 
 * 	This function checks the fDelay and it is set, then timeout
 *  
 **************************************************************************/
BDCC_Error
BDCC_Update_Delay(
    BDCC_INT_P_Handle   hCodObject,
    BDCC_CBUF           *pOutBuf
)
{
    BDBG_ASSERT(hCodObject);

    if ( hCodObject->fDelay )
    {
        if ( BCCGFX_P_TimerQuery(hCodObject->hCCGfxHandle) )
        {
            /* expired */
            hCodObject->fDelay = 0 ;
            BDCC_CBUF_UpdatePost(pOutBuf);

            hCodObject->SIBuf_StreamBytesWritten += hCodObject->SIBuf_StreamBytesPosted ;
            hCodObject->SIBuf_StreamBytesPosted = 0 ;
            hCodObject->SIBuf_NumCmdsWritten += hCodObject->SIBuf_NumCmdsPosted ;
            hCodObject->SIBuf_NumCmdsPosted = 0 ;
        }
    }
    return BDCC_Error_eSuccess;
}

/* Send the test string found in BSEAV/app/eia708/708_test_string.h to the CC engine */
#include <bkni.h>

BDCC_Error BDCC_Coding_P_SendTestString(
    BDCC_INT_P_Handle hCodObject,
    const unsigned char *pTestSt,
    const unsigned int TestStLen
    )
{

    unsigned char cmd, AddlBytes;
    unsigned char * pAddlBytes;
    unsigned char * pCmd = (unsigned char *) pTestSt;
    BDCC_COD_P_ProcessDisposition pd;

    BDBG_ASSERT((hCodObject));

    while(pCmd < (pTestSt + TestStLen))
    {
        cmd = *(pCmd++);
        AddlBytes = ClassifyFirstByte(cmd);
        pAddlBytes = pCmd;
        pd = ProcessCmd(hCodObject, cmd, AddlBytes, pAddlBytes);

        pCmd += AddlBytes;
        BKNI_Sleep(10);
    } /* for (each input byte) */

    return(BDCC_Error_eSuccess);
}/* BDCC_Coding_P_SendTestString*/
