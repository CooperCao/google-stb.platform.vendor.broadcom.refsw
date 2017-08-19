/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 ***************************************************************************/

#include "b_api_shim.h"
#include "bdcc_coding.h"
#include "bdcc_priv.h"
#include "bdcc_coding_c1_handlers.h"
#include "bdcc_bits.h"
#include "bdcc_int.h"
#include "bdcc_gfx.h"
#include "bdcc.h"



BDBG_MODULE(BDCC_CODING_HANDLERS);


void UpdateVarDirect(
    void * pValue,
    unsigned int UpdateMaskValue,
    unsigned int * pUpdateMask,
    unsigned int Value
    )
{
    unsigned int *pVal = (unsigned int *)pValue;
    if (Value != *pVal)
    {
        /* new value is different, update it */
        *pVal = Value;
        *pUpdateMask |= UpdateMaskValue;
    }
}


void UpdateVar(
    void *pVal,
    unsigned int UpdateMaskValue,
    int NumBits,
    unsigned int *pUpdateMask,
    unsigned char *pBuf,
    int *pByteOff,
    int *pBitOff
    )
{
    UpdateVarDirect(pVal, UpdateMaskValue, pUpdateMask, BDCC_BITS_P_GetNextBits(pBuf, pByteOff, pBitOff, NumBits));
}


#ifdef FCC_MIN_DECODER

/* FCC defines a minimum decoder requirement to keep a handle
   on memory. 
   Min requirement is for four visible caption rows and four
   hidden caption rows. Caption windows may be discarded as
   needed based onpriority and age (older windows have higher
   priority than younger). */


/* How many active caption rows of particular visibility */
static unsigned int GetTotalRowCount(
    BDCC_INT_P_Handle hCodObject,
    unsigned int wnd,
    unsigned int visible
    )
{
    BDCC_INT_P_Window *pw;
    unsigned int i, count = 0;

    for (i = 0; i < 8; i++)
    {
        if(i == wnd) continue; /* exclude ourself from analysis */
        pw = &hCodObject->WindowInfo[i];
        if(pw->fDefined && (pw->WindowDef.Visible == visible))
        {   
            count += pw->WindowDef.RowCount;
        }
    }

    return count;
}


/* Used to check if one window was originally created before
   another window in order to determine which window to discard
   We maintain a ordered list (by age) of the caption windows in
   a 32 bit register (so that it can shifted easily) */
static unsigned int GetAge(
    BDCC_INT_P_Handle hCodObject,
    unsigned int wnd
    )
{
    uint32_t AgeList = hCodObject->AgeList, i;

    for(i = 0; i < 8; i++)
    {
        if((AgeList & 0xf) == wnd) break;
        AgeList >>= 4;
    }

    return i;
}

/* lower priority younger windows of the same visibility can be destroyed to free up
   some additional rows. Try here */
static bool TryExchange(
    BDCC_INT_P_Handle hCodObject,
    unsigned int wnd, /* window we're trying to create (if any) */
    unsigned int reqdRows, /* how many do we need to free up */
    unsigned int priority, /* priority of window were trying to create */
    unsigned int visible, /* visibility of window we are trying to create */
    unsigned int age, /* age of window we are trying to create */
    bool force /* ignore 'wnd'. We are not trying to create, we just want to cull */
    )
{
    BDCC_INT_P_Window *pw;
    unsigned int i, exchangeCount = 0, rowsAtPriority[8] = {0,0,0,0,0,0,0,0};
    unsigned char targetWindows = 0;

    /* go around once to see if there are enough lower priority rows */
    /* or same priority younger rows to cull so that we can reclaim reqdRows */
    for (i = 0; i < 8; i++)
    {
        if((i == wnd) && !force) continue; /* can't take from ourself */
        pw = &hCodObject->WindowInfo[i];
        if(pw->fDefined && (pw->WindowDef.Visible == visible))
        {
            /* 0 is highest priority, 7 is lowest */
            if((priority < pw->WindowDef.Priority) ||
              ((priority == pw->WindowDef.Priority) && (GetAge(hCodObject, i) < age))
                || force)
            {
                exchangeCount += pw->WindowDef.RowCount;
                rowsAtPriority[pw->WindowDef.Priority] += pw->WindowDef.RowCount;
                targetWindows |= (1 << i); /* make bit mask of windows of interest */
            }
        }
    }

    if(exchangeCount >= reqdRows)
    {
        /* we can do the exchange, start deleting the lowest priority */
        int exchangePriority = 7; /* lowest */
        unsigned int deleted = 0;
        uint32_t AgeList, youngest;

        /* make cumulative */
        rowsAtPriority[6] += rowsAtPriority[7];
        rowsAtPriority[5] += rowsAtPriority[6];
        rowsAtPriority[4] += rowsAtPriority[5];
        rowsAtPriority[3] += rowsAtPriority[4];
        rowsAtPriority[2] += rowsAtPriority[3];
        rowsAtPriority[1] += rowsAtPriority[2];
        rowsAtPriority[0] += rowsAtPriority[1];

        /* start deleting windows to reclaim caption rows */

        /* first phase, consume all or nothing for a particular priority starting with lowest */
        do
        {
            if((rowsAtPriority[exchangePriority]) <= reqdRows)
            {
                if(rowsAtPriority[exchangePriority]) /* at least one */
                {
                    for (i = 0; i < 8; i++)
                    {
                        if(targetWindows & (1 << i)) /* previously marked for attention */
                        {
                            pw = &hCodObject->WindowInfo[i];
                            if(exchangePriority == (int)pw->WindowDef.Priority)
                            {
                                unsigned char destroyWnd = 1 << i;
                                BDCC_COD_P_ProcessCmd_C1_DeleteWindows(hCodObject, 0x8C, 1, &destroyWnd);
                                BDBG_MSG(("Deleted window %d to make space for new rows", i));
                            }
                        }
                    }
                }
                deleted = rowsAtPriority[exchangePriority];
            }
            else
                break;
        }
        while(exchangePriority--); /* increasingly raise priority */

        reqdRows -= deleted;

        if(reqdRows) /* any more needed? */
        {
            /* next phase, delete at least some at a particular priority */
            /* delete in age order, youngest first */
            AgeList = hCodObject->AgeList;

            for (i = 0; i < 8; i++)
            {
                youngest = AgeList & 0xf;
                if(targetWindows & (1 << youngest)) /* previously marked? */
                {
                    pw = &hCodObject->WindowInfo[youngest];
                
                    /* 0 is highest priority, 7 is lowest */
                    if(exchangePriority == (int)pw->WindowDef.Priority)
                    {
                        unsigned char destroyWnd = 1 << youngest;

                        BDBG_MSG(("Deleted window %d to make space for new rows", youngest));
                       
                        if(pw->WindowDef.RowCount < (int)reqdRows)
                        {
                            reqdRows -= pw->WindowDef.RowCount; /* go around again */
                            BDCC_COD_P_ProcessCmd_C1_DeleteWindows(hCodObject, 0x8C, 1, &destroyWnd);
                        }
                        else
                        {
                            BDCC_COD_P_ProcessCmd_C1_DeleteWindows(hCodObject, 0x8C, 1, &destroyWnd);
                            break;
                        }
                    }
                }
                AgeList>>= 4; /* next youngest */
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

#define FCC_MAX_ROW 4
#endif


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_DefineWindow(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    int ByteOff = 0;
    int BitOff = 0;
    int wnd = cmd & 7;
    BDCC_INT_P_Window *pw = &hCodObject->WindowInfo[wnd];
    unsigned int *pm = &pw->UpdateMask;
    unsigned int rcCount;
#ifdef FCC_MIN_DECODER
    unsigned int prevVisible = 0;
#endif

    BDBG_MSG(("%s enter", BSTD_FUNCTION));

    BSTD_UNUSED(AddlBytes);

    pw->WndId = wnd;
    hCodObject->CurrentWindow = wnd;


    /*
     * clear the update mask
     */
    *pm = 0;

    BDCC_BITS_P_GetNextBits(pAddlBytes, &ByteOff, &BitOff, 2);
#ifdef FCC_MIN_DECODER
    /* need this for later */
    if(pw->fDefined) prevVisible = pw->WindowDef.Visible;
#endif
    UpdateVar(&pw->WindowDef.Visible,       BDCC_ATTR_MASK_VISIBLE,     1, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pw->WindowDef.RowLock,       BDCC_ATTR_MASK_LOCK,        1, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pw->WindowDef.ColLock,       BDCC_ATTR_MASK_LOCK,        1, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pw->WindowDef.Priority,      BDCC_ATTR_MASK_VISIBLE,     3, pm, pAddlBytes, &ByteOff, &BitOff);

    UpdateVar(&pw->WindowDef.RelativePos,   BDCC_ATTR_MASK_ANCHOR,      1, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pw->WindowDef.AnchorVert,    BDCC_ATTR_MASK_ANCHOR,      7, pm, pAddlBytes, &ByteOff, &BitOff);

    UpdateVar(&pw->WindowDef.AnchorHorz,    BDCC_ATTR_MASK_ANCHOR,      8, pm, pAddlBytes, &ByteOff, &BitOff);

    UpdateVar(&pw->WindowDef.AnchorPoint,   BDCC_ATTR_MASK_ANCHOR,      4, pm, pAddlBytes, &ByteOff, &BitOff);

    /* adjust for 0-based */
    rcCount = 1+BDCC_BITS_P_GetNextBits(pAddlBytes, &ByteOff, &BitOff,4) ;
    if ( rcCount == 16 )
        rcCount = 0 ;

#ifdef FCC_MIN_DECODER

    /* if redefinition and same size or smaller and same visibility  then were okay */
    if(!(pw->fDefined && ((int)rcCount <= pw->WindowDef.RowCount) && (prevVisible == pw->WindowDef.Visible)) &&
        hCodObject->Type == B_Dcc_Type_e708)
    {
        uint32_t tempAgeList, i;
        uint8_t assertion = 0;
        unsigned int reqdRows = 0, totalRowCount = 0, age=0 /* default to youngest */;
        bool proceed = true;
                    
        if (rcCount > FCC_MAX_ROW)
        {
            proceed = false;
        }
        else
        {
            totalRowCount = GetTotalRowCount(hCodObject, wnd, pw->WindowDef.Visible);
            
            /* check if there are enough rows left to satisfy this request */
            if((totalRowCount + rcCount) > FCC_MAX_ROW)
            {
                /* exceeds max rows, must try to exchange with lower priority window */
                reqdRows = (totalRowCount + rcCount) - FCC_MAX_ROW;
                if(pw->fDefined) age = GetAge(hCodObject, wnd); /* redefinition, give real age */
                proceed = TryExchange(hCodObject, wnd, reqdRows, pw->WindowDef.Priority, pw->WindowDef.Visible, age, false);
            }
        }
        
        if (!proceed)
        {
            if(pw->fDefined)
            {
                /* destroy it if it is already created, cannot accomodate increase in size or change in visibility */
                unsigned char destroyWnd = 1 << wnd;
                BDBG_MSG(("Deleted window, can no longer accomodate window %d", wnd));
                BDCC_COD_P_ProcessCmd_C1_DeleteWindows(hCodObject, 0x8C, 1, &destroyWnd);
            }
            else
            {
                BDBG_MSG(("Define Window command ignored, insufficient rows %d", wnd));
            }
            return BDCC_COD_P_ProcessDisposition_eConsumeContinue;
        }

        /* ... success */
        
        /* update 'Age List' only if new definition (not redefinition) */
        if(!(pw->fDefined))
        {
            /* new definition */
            tempAgeList = hCodObject->AgeList;
            hCodObject->AgeList = 0;
            
            for(i = 0; i < 8; i++)
            {
                uint32_t shiftOutVal = (tempAgeList >> 28) & 0xf;
                tempAgeList <<= 4;
                if(shiftOutVal == (uint32_t)wnd) continue;
                hCodObject->AgeList |= shiftOutVal;
                hCodObject->AgeList <<= 4;
                assertion |= 1 << shiftOutVal;
            } 
            hCodObject->AgeList |= wnd;  /* wnd is newest */
            assertion |= 1 << wnd;
            /* assert that we always have all eight windows */
            BDBG_ASSERT((assertion == 0xff));
        }
    }

#endif

    UpdateVarDirect((unsigned int *)&pw->WindowDef.RowCount,BDCC_ATTR_MASK_RCCOUNT, pm, rcCount);

    BDCC_BITS_P_GetNextBits(pAddlBytes, &ByteOff, &BitOff, 2);

    UpdateVarDirect((unsigned int *)&pw->WindowDef.ColCount,BDCC_ATTR_MASK_RCCOUNT, pm, 1+BDCC_BITS_P_GetNextBits(pAddlBytes, &ByteOff, &BitOff,6));

    BDCC_BITS_P_GetNextBits(pAddlBytes, &ByteOff, &BitOff, 2);
    UpdateVar(&pw->WindowDef.WndStyleID,    BDCC_ATTR_MASK_WNDSTYLEID,  3, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pw->WindowDef.PenStyleID,    BDCC_ATTR_MASK_PENSTYLEID,  3, pm, pAddlBytes, &ByteOff, &BitOff);

    if((pw->WindowDef.AnchorPoint > 8) ||
        (pw->WindowDef.ColCount > (int)hCodObject->uiColumns) ||
        (pw->WindowDef.RelativePos && (pw->WindowDef.AnchorVert > 99)) ||
        (pw->WindowDef.RelativePos && (pw->WindowDef.AnchorHorz > 99)) ||
        ((!pw->WindowDef.RelativePos) && (pw->WindowDef.AnchorVert > 74)) ||
        ((!pw->WindowDef.RelativePos) && (pw->WindowDef.ColCount == 42) /* 16:9 */
                                                                    && (pw->WindowDef.AnchorHorz > 209)) ||
        ((!pw->WindowDef.RelativePos) && (pw->WindowDef.ColCount == 32) /* 4:3 */
                                                                    && (pw->WindowDef.AnchorHorz > 159 ))
        )
    {
        if(pw->fDefined)
        {
            unsigned char destroyWnd = 1 << wnd;
            BDCC_COD_P_ProcessCmd_C1_DeleteWindows(hCodObject, 0x8C, 1, &destroyWnd);
            BDBG_MSG(("Bit error in %s, deleted window", BSTD_FUNCTION));
        }
    }
    else
    {
        hCodObject->DefinedWindowsMask |= (1 << wnd);
        UpdateVarDirect((unsigned int *)&pw->fDefined, BDCC_ATTR_MASK_DEFINED_SET, pm, 1);

        BDCC_INT_P_UpdateWindow(hCodObject, pw);
    }

    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
} /* BDCC_COD_P_ProcessCmd_C1_DefineWindow */


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_SetWindowAttributes(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    int ByteOff = 0;
    int BitOff = 0;
    int wnd = hCodObject->CurrentWindow;
    BDCC_INT_P_Window *pw = &hCodObject->WindowInfo[wnd];
    unsigned int *pm = &pw->UpdateMask;
    unsigned int tempBorderType;

    BDBG_MSG(("%s enter", BSTD_FUNCTION));
    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);

    /*
    * clear the update mask
    */
    *pm = 0;

    UpdateVar(&pw->WindowAttr.FillOpacity,      BDCC_ATTR_MASK_FILLOP,      2, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pw->WindowAttr.FillColor,        BDCC_ATTR_MASK_FILL,        6, pm, pAddlBytes, &ByteOff, &BitOff);

    tempBorderType = BDCC_BITS_P_GetNextBits(pAddlBytes, &ByteOff, &BitOff, 2);
    UpdateVar(&pw->WindowAttr.BorderColor,      BDCC_ATTR_MASK_BORDER,      6, pm, pAddlBytes, &ByteOff, &BitOff);

    tempBorderType |= BDCC_BITS_P_GetNextBits(pAddlBytes, &ByteOff, &BitOff, 1) << 2;
    UpdateVarDirect(&pw->WindowAttr.BorderType, BDCC_ATTR_MASK_BORDER, pm, tempBorderType);
    UpdateVar(&pw->WindowAttr.WordWrap,         BDCC_ATTR_MASK_WORDWRAP,    1, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pw->WindowAttr.PrintDirection,   BDCC_ATTR_MASK_PRINTDIR,    2, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pw->WindowAttr.ScrollDirection,  BDCC_ATTR_MASK_SCROLLDIR,   2, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pw->WindowAttr.Justify,          BDCC_ATTR_MASK_JUSTIFY,     2, pm, pAddlBytes, &ByteOff, &BitOff);

    UpdateVar(&pw->WindowAttr.EffectSpeed,      BDCC_ATTR_MASK_EFFECT,      4, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pw->WindowAttr.EffectDirection,  BDCC_ATTR_MASK_EFFECT,      2, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pw->WindowAttr.DisplayEffect,    BDCC_ATTR_MASK_EFFECT,      2, pm, pAddlBytes, &ByteOff, &BitOff);

    BDCC_INT_P_UpdateWindow(hCodObject, pw) ;

    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
}


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_SetPenAttributes(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    int ByteOff = 0;
    int BitOff = 0;
    int wnd = hCodObject->CurrentWindow;
    BDCC_INT_P_Window *pw = &hCodObject->WindowInfo[wnd];
    BDCC_INT_P_Pen *pp = &pw->Pen;
    unsigned int *pm = &pw->UpdateMask;
    unsigned int i;

    BDBG_MSG(("%s enter", BSTD_FUNCTION));
    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);

    /*
     * clear the update mask
     */
    *pm = 0;

    UpdateVar(&pp->Attr.TextTag,    BDCC_ATTR_MASK_NONE,        4, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pp->Attr.Offset,     BDCC_ATTR_MASK_NONE,        2, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pp->Attr.PenSize,    BDCC_ATTR_MASK_PENSIZE,     2, pm, pAddlBytes, &ByteOff, &BitOff);

    UpdateVar(&pp->Attr.Italics,    BDCC_ATTR_MASK_ITALICS,     1, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pp->Attr.Underline,  BDCC_ATTR_MASK_UNDERLINE,   1, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pp->Attr.EdgeType,   BDCC_ATTR_MASK_EDGETYPE,    3, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pp->Attr.FontStyle,  BDCC_ATTR_MASK_FONTSTYLE,   3, pm, pAddlBytes, &ByteOff, &BitOff);

    if((pw->fDefined) &&
        ((pp->Attr.PenSize > BDCC_PenSize_Large) ||
        (pp->Attr.EdgeType > BDCC_Edge_Style_RightDropShadow))
        )
    {
        unsigned char destroyWnd = 1 << wnd;
        BDCC_COD_P_ProcessCmd_C1_DeleteWindows(hCodObject, 0x8C, 1, &destroyWnd);
        BDBG_MSG(("Bit error in %s, deleted window", BSTD_FUNCTION));
    }
    else
    {
        /* save the raw args for efficient storage in the window buffer */
        for ( i=0 ; i < sizeof(pp->Attr.CmdArgs) ; i++ )
            pp->Attr.CmdArgs[i] = pAddlBytes[i];

        BDCC_INT_P_UpdateWindow(hCodObject, pw);
    }
    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
} /* BDCC_COD_P_ProcessCmd_C1_SetPenAttributes */


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_SetPenColor(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    int ByteOff = 0;
    int BitOff = 0;
    int wnd = hCodObject->CurrentWindow;
    BDCC_INT_P_Window *pw = &hCodObject->WindowInfo[wnd];
    BDCC_INT_P_Pen *pp = &pw->Pen;
    unsigned int *pm = &pw->UpdateMask;
    unsigned int i;

    BDBG_MSG(("%s enter", BSTD_FUNCTION));
    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);

    /*
     * clear the update mask
     */
    *pm = 0;

    UpdateVar(&pp->Color.FgOpacity, BDCC_ATTR_MASK_PENFGOP,     2, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pp->Color.FgColor,   BDCC_ATTR_MASK_PENFG,       6, pm, pAddlBytes, &ByteOff, &BitOff);

    UpdateVar(&pp->Color.BgOpacity, BDCC_ATTR_MASK_PENBGOP,     2, pm, pAddlBytes, &ByteOff, &BitOff);
    UpdateVar(&pp->Color.BgColor,   BDCC_ATTR_MASK_PENBG,       6, pm, pAddlBytes, &ByteOff, &BitOff);

    BDCC_BITS_P_GetNextBits(pAddlBytes, &ByteOff, &BitOff, 2);
    UpdateVar(&pp->Color.EdgeColor, BDCC_ATTR_MASK_EDGECOLOR,   6, pm, pAddlBytes, &ByteOff, &BitOff);

    /* save the raw args for efficient storage in the window buffer */
    for ( i=0 ; i < sizeof(pp->Color.CmdArgs) ; i++ )
        pp->Color.CmdArgs[i] = pAddlBytes[i];

    BDCC_INT_P_UpdateWindow(hCodObject, pw);
    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
}


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_SetPenLocation(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    int ByteOff = 0;
    int BitOff = 0;
    int wnd = hCodObject->CurrentWindow;
    BDCC_INT_P_Window *pw = &hCodObject->WindowInfo[wnd];
    BDCC_INT_P_Pen *pp = &pw->Pen;
    unsigned int *pm = &pw->UpdateMask;

    BDBG_MSG(("%s enter", BSTD_FUNCTION));
    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);

    /*
     * clear the update mask
     */
    *pm = 0;

    BDCC_BITS_P_GetNextBits(pAddlBytes, &ByteOff, &BitOff, 4);
    UpdateVar(&pp->Row,     BDCC_ATTR_MASK_PENLOC,      4, pm, pAddlBytes, &ByteOff, &BitOff);

    BDCC_BITS_P_GetNextBits(pAddlBytes, &ByteOff, &BitOff, 2);
    UpdateVar(&pp->Col,     BDCC_ATTR_MASK_PENLOC,      6, pm, pAddlBytes, &ByteOff, &BitOff);

    if((pw->fDefined) &&
        ((pp->Row < 0) ||
        (pp->Row > 15) || /* max supported rows */
        (pp->Col < 0) ||
        (pp->Col > 42)) /* max supported columns */
        )
    {
        unsigned char destroyWnd = 1 << wnd;
        BDCC_COD_P_ProcessCmd_C1_DeleteWindows(hCodObject, 0x8C, 1, &destroyWnd);
        BDBG_MSG(("Bit error in %s, deleted window", BSTD_FUNCTION));
    }
    else
    {
        BDBG_MSG(("%s wnd=%d row=%d col=%d", BSTD_FUNCTION, wnd, pp->Row, pp->Col));
        BDCC_INT_P_UpdateWindow(hCodObject, pw);
    }
    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
}


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_DeleteWindows(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    unsigned char WindowsMask;
    unsigned char wndmask;
    int wnd;
    BDCC_INT_P_Window *pw;

    BDBG_MSG(("%s enter", BSTD_FUNCTION));
    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);

    WindowsMask = pAddlBytes[0] & hCodObject->DefinedWindowsMask;

    for ( wndmask=1,wnd=0 ; wnd < 8 ; wndmask <<= 1,wnd++ )
    {
        if ( WindowsMask & wndmask )
        {
            unsigned int *pm;
            pw = &hCodObject->WindowInfo[wnd];
            pm = &pw->UpdateMask;
            *pm = 0 ;
            UpdateVarDirect(&pw->fDefined, BDCC_ATTR_MASK_DEFINED_CLR, pm, 0);
            if ( *pm )
                BDCC_INT_P_UpdateWindow(hCodObject, pw);
        }
    }
    hCodObject->DefinedWindowsMask &= ~WindowsMask;

    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
}


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_HideWindows(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    unsigned char WindowsMask;
    unsigned char wndmask;
    int wnd;
    BDCC_INT_P_Window *pw;
    unsigned int *pm;
    /* SW7364-355 */
    unsigned int wndCount = 0;
    int firstWnd = -1; 	/* how many windows defined and firstWnd if it is only one */
    BDBG_MSG(("%s enter", BSTD_FUNCTION));

    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);

    WindowsMask = pAddlBytes[0] & hCodObject->DefinedWindowsMask;

    for ( wndmask=1,wnd=0 ; wnd < 8 ; wndmask <<= 1,wnd++ )
    {
        if ( WindowsMask & wndmask )
        {
            pw = &hCodObject->WindowInfo[wnd];
            pm = &pw->UpdateMask;
            *pm = 0;
            UpdateVarDirect(&pw->WindowDef.Visible, BDCC_ATTR_MASK_VISIBLE, pm, 0);
            if (pw->fDefined) {
		wndCount++;
		if (-1 == firstWnd)
		    firstWnd = wnd;
            }
        }
    }

#ifdef FCC_MIN_DECODER

    if(hCodObject->Type == B_Dcc_Type_e708)
    {
        unsigned int totalHiddenRowCount, reqdRows = 0;

        /* SW7364-355, adjust priority age list only if more than one windows defined */
	if (wndCount < 2)
		totalHiddenRowCount = GetTotalRowCount(hCodObject, firstWnd, false);
	else
		totalHiddenRowCount = GetTotalRowCount(hCodObject, 9 /* dummy */, false);

	/* get total hidden rows, must limit it to 4 */
	if((totalHiddenRowCount) > FCC_MAX_ROW)
        {
            /* exceeds max rows, must cull some windows starting with lowest priority */
            reqdRows = totalHiddenRowCount - FCC_MAX_ROW;
            TryExchange(hCodObject, 0 , reqdRows, 0, false, 0, true); /* guaranteed to cull reqd */
        }
    }

#endif

    /* update mask modified by visible or delete */
    for (wnd=0 ; wnd < 8 ; wnd++)
    {
        pw = &hCodObject->WindowInfo[wnd];
        pm = &pw->UpdateMask;

        if(*pm)
            BDCC_INT_P_UpdateWindow(hCodObject, pw);
    }

    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
}


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_DisplayWindows(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    unsigned char WindowsMask;
    unsigned char wndmask;
    int wnd;
    BDCC_INT_P_Window *pw;
    unsigned int *pm;

    BDBG_MSG(("%s enter", BSTD_FUNCTION));
    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);

    WindowsMask = pAddlBytes[0] & hCodObject->DefinedWindowsMask;


    for ( wndmask=1,wnd=0 ; wnd < 8 ; wndmask <<= 1,wnd++ )
    {
        if ( WindowsMask & wndmask )
        {
            pw = &hCodObject->WindowInfo[wnd] ;
            pm = &pw->UpdateMask ;
            *pm = 0 ;
            UpdateVarDirect(&pw->WindowDef.Visible, BDCC_ATTR_MASK_VISIBLE, pm, 1) ;
        }
    }

#ifdef FCC_MIN_DECODER

    if(hCodObject->Type == B_Dcc_Type_e708)
    {
        unsigned int totalVisibleRowCount, reqdRows = 0;

        /* get total visible rows, must limit it to 4 */
        totalVisibleRowCount = GetTotalRowCount(hCodObject, 9 /* dummy */, true);
            
        /* cull any rows in excess of 4 */
        if((totalVisibleRowCount) > FCC_MAX_ROW)
        {
            /* exceeds max rows, must cull some windows starting with lowest priority */
            reqdRows = totalVisibleRowCount - FCC_MAX_ROW;
            TryExchange(hCodObject, 0, reqdRows, 0, true, 0, true); /* guaranteed to cull reqd */
        }
    }

#endif
    
    /* update mask modified by visible or delete */
    for (wnd=0 ; wnd < 8 ; wnd++)
    {
        pw = &hCodObject->WindowInfo[wnd];
        pm = &pw->UpdateMask;

        if(*pm)
            BDCC_INT_P_UpdateWindow(hCodObject, pw);
    }

    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
}


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_ToggleWindows(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    unsigned char WindowsMask;
    unsigned char wndmask;
    int wnd ;
    BDCC_INT_P_Window *pw;
    unsigned int *pm;

    BDBG_MSG(("%s enter", BSTD_FUNCTION));
    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);

    WindowsMask = pAddlBytes[0] & hCodObject->DefinedWindowsMask;

    for ( wndmask=1,wnd=0 ; wnd < 8 ; wndmask <<= 1,wnd++ )
    {
        if ( WindowsMask & wndmask )
        {
            pw = &hCodObject->WindowInfo[wnd];
            pm = &pw->UpdateMask;
            *pm = 0;

            if ( pw->fDefined )
            {
                UpdateVarDirect(&pw->WindowDef.Visible, BDCC_ATTR_MASK_VISIBLE, pm, 1);
                if ( *pm == 0 )
                {
                    UpdateVarDirect(&pw->WindowDef.Visible, BDCC_ATTR_MASK_VISIBLE, pm, 0);
                }
            }
        }
    }

#ifdef FCC_MIN_DECODER

    if(hCodObject->Type == B_Dcc_Type_e708)
    {
        unsigned int totalVisibleRowCount, totalHiddenRowCount, reqdRows = 0;

        /* get total visible rows, must limit it to 4 */
        totalVisibleRowCount = GetTotalRowCount(hCodObject, 9 /* dummy */, true);

        /* get total hidden rows, must limit it to 4 */
        totalHiddenRowCount = GetTotalRowCount(hCodObject, 9 /* dummy */, false);

        /* cull any rows in excess of 4 */
        if((totalVisibleRowCount) > FCC_MAX_ROW)
        {
            /* exceeds max rows, must cull some windows starting with lowest priority */
            reqdRows = totalVisibleRowCount - FCC_MAX_ROW;
            TryExchange(hCodObject, 0, reqdRows, 0, true, 0, true); /* guaranteed to cull reqd */
        }
    
        if((totalHiddenRowCount) > FCC_MAX_ROW)
        {
            /* exceeds max rows, must cull some windows starting with lowest priority */
            reqdRows = totalHiddenRowCount - FCC_MAX_ROW;
            TryExchange(hCodObject, 0, reqdRows, 0, false, 0, true); /* guaranteed to cull reqd */
        }
    }

#endif

    /* update mask modified by display or delete */
    for (wnd=0 ; wnd < 8 ; wnd++)
    {
        pw = &hCodObject->WindowInfo[wnd] ;
        pm = &pw->UpdateMask ;

        if(*pm)
            BDCC_INT_P_UpdateWindow(hCodObject, pw);
    }

    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue) ;
}


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_ClearWindows(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    unsigned char WindowsMask;
    unsigned char wndmask;
    int wnd;
    BDCC_INT_P_Window *pw;
    unsigned int *pm;

    BDBG_MSG(("%s enter", BSTD_FUNCTION));
    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);

    WindowsMask = pAddlBytes[0] & hCodObject->DefinedWindowsMask;

    for ( wndmask=1,wnd=0 ; wnd < 8 ; wndmask <<= 1,wnd++ )
    {
        if ( WindowsMask & wndmask )
        {
            pw = &hCodObject->WindowInfo[wnd];
            pm = &pw->UpdateMask;
            *pm = BDCC_ATTR_MASK_CLEARWINDOW;
            BDCC_INT_P_UpdateWindow(hCodObject, pw);
        }
    }

    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
}


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_SetCurrentWindow(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    int wnd = cmd & 7;
    BDCC_INT_P_Window * pw = &hCodObject->WindowInfo[wnd];

    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);
    BSTD_UNUSED(pAddlBytes);

    if ( pw->fDefined )
    {
        hCodObject->CurrentWindow = wnd;
    }

    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
}


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_Delay(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    BSTD_UNUSED(hCodObject);
    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);
    BSTD_UNUSED(pAddlBytes);

    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
}


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_DelayCancel(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{

    BSTD_UNUSED(hCodObject);
    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);
    BSTD_UNUSED(pAddlBytes);

    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
}


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_Reset(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    BSTD_UNUSED(hCodObject);
    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);
    BSTD_UNUSED(pAddlBytes);

    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
}


BDCC_COD_P_ProcessDisposition BDCC_COD_P_ProcessCmd_C1_Stub(
    BDCC_INT_P_Handle hCodObject,
    unsigned char cmd,
    unsigned int AddlBytes,
    unsigned char *pAddlBytes
    )
{
    BSTD_UNUSED(hCodObject);
    BSTD_UNUSED(cmd);
    BSTD_UNUSED(AddlBytes);
    BSTD_UNUSED(pAddlBytes);

    return(BDCC_COD_P_ProcessDisposition_eConsumeContinue);
}
