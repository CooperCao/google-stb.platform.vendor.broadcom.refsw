/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to the terms and
* conditions of a separate, written license agreement executed between you and Broadcom
* (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
* no license (express or implied), right to use, or waiver of any kind with respect to the
* Software, and Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
* secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
* and to use this information only in connection with your use of Broadcom integrated circuit products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
* AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
* WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
* THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
* OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
* LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
* OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
* USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
* LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
* EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
* USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
* ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
* LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
* ANY LIMITED REMEDY.
*
* Module Description:
*
***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_common_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_buffer_dbg_priv.h"


BDBG_MODULE(BVDC_BUF_DBG);
BDBG_OBJECT_ID(BVDC_BUF_DBG);

#if (BVDC_BUF_LOG == 1)

/* This index implementation assumes little-endian configuration.
   This allows vdclog_dump to only output the recent events.
   In the 32-bit value ulIdx, the upper 16-bit is the wraparound counter,
   and the lower 16-bit is the actual index to the vdclog event buffer. */
#define BVDC_P_BUF_LOG_IDX_INCREMENT(stIdx) \
                                    stIdx.ulIdx++;                                        \
                                    if(stIdx.stIdxPart.usIdxEvent >= BVDC_P_BUF_LOG_ENTRIES)  \
                                    {                                                   \
                                        stIdx.stIdxPart.usIdxEvent = 0;                   \
                                        stIdx.stIdxPart.usIdxRound++;                     \
                                    }
#define BVDC_P_BUF_LOG_IDX_ADD(stIdx, uValueAdd)                                              \
                                    stIdx.ulIdx += uValueAdd;                                 \
                                    if(stIdx.stIdxPart.usIdxEvent >= BVDC_P_BUF_LOG_ENTRIES)      \
                                    {                                                       \
                                        stIdx.stIdxPart.usIdxEvent &= BVDC_P_BUF_LOG_ENTRIES_MASK;\
                                        stIdx.stIdxPart.usIdxRound++;                         \
                                    }
#define BVDC_P_BUF_LOG_IDX_SUB(stIdx, uValueSub)              \
                                    stIdx.ulIdx -= uValueSub; \
                                    stIdx.stIdxPart.usIdxEvent &= BVDC_P_BUF_LOG_ENTRIES_MASK


void BVDC_P_BufLog_Init
    ( BVDC_P_BufLog *pBufLog )
{
    int i;

    pBufLog->stWriterIdx.ulIdx = 0;
    pBufLog->stLastSkipRepeatIdx.ulIdx = 0;
    pBufLog->eState = BVDC_BufLogState_eReset;
    pBufLog->pfCallback = NULL;
    pBufLog->pvCbParm1 = NULL;
    pBufLog->iCbParm2 = 0;

    for (i=0; i<BVDC_P_WindowId_eComp0_G0; i++)
    {
        pBufLog->bEnable[i] = true;
    }

#if (!BVDC_P_USE_RDC_TIMESTAMP)
    pBufLog->stTmrRegs.status = 0;
    pBufLog->stTmrRegs.control = 0;
#endif

}

void BVDC_P_BufLog_SetStateAndDumpTrigger
    ( BVDC_P_BufLog                *pBufLog,
      BVDC_BufLogState              eLogState,
      const BVDC_CallbackFunc_isr   pfCallback,
      void                         *pvParm1,
      int                           iParm2 )
{
    BDBG_ASSERT(pBufLog);

    /* Store user settings */
    pBufLog->eState = eLogState;
    pBufLog->pfCallback = pfCallback;
    pBufLog->pvCbParm1 = pvParm1;
    pBufLog->iCbParm2 = iParm2;
    return;
}

/*
 * Outputs the log, from BVDC_P_BUF_LOG_ENTRIES_PAD events prior to the first skip/repeat event through BVDC_P_BUF_LOG_ENTRIES_PAD events
 * after the last skip/repeat event.
 * It tries to catch up with the stBufLog_IdxLastSkipRepeat index, that moves whenever another skip/repeat event occurs.
 */
void BVDC_P_BufLog_DumpLog
    ( BVDC_P_BufLog      *pBufLog,
      char               *pLog,
      unsigned int        uiLenToRead,
      unsigned int       *puiReadCount )
{
    uint32_t           ulBufCtr = 0, ulOutputCtr = 0;
    BVDC_P_BufLogInfo *pstSrc = &(pBufLog->astLogInfo[0]);
    BVDC_P_BufLogIdx   stLastReaderIdx, stReaderIdx;
    uint16_t           usBufLogR;
    uint32_t           ulStringOffset = 0;

    /* compute the starting point */
    stReaderIdx = pBufLog->stLastSkipRepeatIdx;
    BVDC_P_BUF_LOG_IDX_SUB(stReaderIdx, BVDC_P_BUF_LOG_ENTRIES_PAD);

    /* header marker */
    ulStringOffset = BKNI_Snprintf(&pLog[0], uiLenToRead,
                            "BRCM===%08x:%08x:%08x===>\n",
                            stReaderIdx.ulIdx, pBufLog->stLastSkipRepeatIdx.ulIdx,  pBufLog->stWriterIdx.ulIdx);

    while(1)
    {
        usBufLogR = stReaderIdx.stIdxPart.usIdxEvent;
        ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead,
                                "%010u:%04u:%c",
                                (pstSrc + usBufLogR)->ulTime,
                                usBufLogR,
                                (pstSrc + usBufLogR)->cLogType);

        if ((pstSrc + usBufLogR)->cLogType <= ((pBufLog->eState == BVDC_BufLogState_eAutomaticReduced) ? 'F' : 'M'))
        {
            ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset,
                                    "%u:%u[",
                                    (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulWindowId,
                                    (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulBufferId);

            for(ulBufCtr=0; ulBufCtr<((pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulBufferNum); ulBufCtr++)
            {
                ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset,
                                        "%c",
                                        (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.cRW[ulBufCtr]);
            }

            ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset,
                                    "]%u:%08x:%012u",
                                    (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulBufferNum,
                                    (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulInfo_1,
                                    (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulBufferTS);

            if ((pstSrc + usBufLogR)->cLogType == 'A' || (pstSrc + usBufLogR)->cLogType == 'B')
            {
                if ((pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulInfo_2 & BVDC_P_BUF_LOG_SKIP_DUE_TO_MTG_ALIGN_SRC)
                {
                    ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== skip due to MTG src align");  /* mark the skip event to ease reading of the logs */
                    ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== pic %x", (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulInfo_3);
                }
                else
                {
                    ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== skip");  /* mark the skip event to ease reading of the logs */
                }
            }

            if ((pstSrc + usBufLogR)->cLogType == 'C' || (pstSrc + usBufLogR)->cLogType == 'D')
            {
                if ((pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulInfo_2 & BVDC_P_BUF_LOG_FORCED_REPEAT_PIC)
                {
                    ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== MTG forced repeat");
                    ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, ", pic %x", (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulInfo_3);
                }
                else if ((pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulInfo_2 & BVDC_P_BUF_LOG_REPEAT_DUE_TO_MTG_ALIGN_SRC)
                {
                    ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== repeat due to MTG src align");  /* mark the repeat event to ease reading of the logs */
                    ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, ", pic %x", (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulInfo_3);
                }
                else
                {
                    ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== repeat");  /* mark the repeat event to ease reading of the logs */
                }
            }

            if ((pstSrc + usBufLogR)->cLogType == 'E')
            {
                ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== possible inversion");  /* mark the inversion event to ease reading of the logs */
            }

            if ((pstSrc + usBufLogR)->cLogType == 'F')
            {
                ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== W overran R");  /* mark the encroachment event to ease reading of the logs */
            }

            if ((pstSrc + usBufLogR)->cLogType == 'G')
            {
                ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== R overran W");  /* mark the encroachment event to ease reading of the logs */
            }

            /* mark skips accordingly */
            if ((pstSrc + usBufLogR)->cLogType == 'K')
            {
                if ((pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulInfo_2 & BVDC_P_BUF_LOG_SKIP_DUE_TO_MTG)
                {
                    ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== skip due to MTG drop");
                }
                else if ((pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulInfo_2 & BVDC_P_BUF_LOG_SKIP_DUE_TO_WRITER_GAP)
                {
                    ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== skip due to writer gap");
                }
                else if ((pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulInfo_2 & BVDC_P_BUF_LOG_W_NORMAL_ADVANCE)
                {
                    ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== normal advance");
                }

                ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, ", pic %x", (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulInfo_3);
            }

            if ((pstSrc + usBufLogR)->cLogType == 'L')
            {
                ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== pic %x", (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulInfo_3);
            }

            /* check for missed/delayed interrupts */
            if ((usBufLogR != 0) && ((pstSrc + usBufLogR)->cLogType == 'K' || (pstSrc + usBufLogR)->cLogType == 'L'))
            {
                uint16_t usLogEntry = usBufLogR;
                uint32_t ulWindowId = (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulWindowId;
                uint32_t ulInterruptLatency = 0;

                /* find previous K or L entry of the same window */
                if ((pstSrc + usBufLogR)->cLogType == 'K')
                {
                    do
                    {
                        usLogEntry--;
                    } while ((usLogEntry != 0) && ((pstSrc + usLogEntry)->cLogType != 'K') &&
                            (ulWindowId == (pstSrc + usLogEntry)->BufLogData.stSkipRepeat.ulWindowId));
                }
                else
                {
                    do
                    {
                        usLogEntry--;
                    } while ((usLogEntry != 0) && ((pstSrc + usLogEntry)->cLogType != 'L') &&
                            (ulWindowId == (pstSrc + usLogEntry)->BufLogData.stSkipRepeat.ulWindowId));
                }

                if (usLogEntry != 0)
                {
                    uint32_t ulCurrentIndexTime = (pstSrc + usBufLogR)->ulTime;

                    /* Account for timer value wrap-around  */
                    if ((pstSrc + usLogEntry)->ulTime > (pstSrc + usBufLogR)->ulTime)
                    {
                        ulCurrentIndexTime += (pstSrc + usLogEntry)->ulMaxTimestamp;
                    }

                    ulInterruptLatency = (ulCurrentIndexTime - (pstSrc + usLogEntry)->ulTime)/1000; /* ms */
                }

                /* pad 10% */
                if (ulInterruptLatency > (110*(pstSrc + usBufLogR)->ulInterruptLatency)/100)
                {
                     ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<=== missed interrupt");
                }
            }

            ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "\n");

        }
        else if(pBufLog->eState != BVDC_BufLogState_eAutomaticReduced)
        {
            if((pstSrc + usBufLogR)->cLogType == 'Q')
            {
                ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset,
                                "%u:R=%u W=%u",
                                (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU0,
                                (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU1,
                                (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU2);
            }
            else if( ((pstSrc + usBufLogR)->cLogType == 'S') || ((pstSrc + usBufLogR)->cLogType == 'T') ||
                     ((pstSrc + usBufLogR)->cLogType == 'Y') || ((pstSrc + usBufLogR)->cLogType == 'Z'))
            {

                ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset,
                                "%u:%u:%012u:%012u",
                                (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU0,
                                (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU1,
                                (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU2,
                                (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU3);
            }
            else if( ((pstSrc + usBufLogR)->cLogType == 'U') || ((pstSrc + usBufLogR)->cLogType == 'V') )
            {
                ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset,
                                "%u:cnt=%u",
                                (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU0,
                                (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU1);
            }
            else if( ((pstSrc + usBufLogR)->cLogType == 'T') )
            {
                ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset,
                                "%u:%012u:%012u",
                                (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU0,
                                (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU1,
                                (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU2);
            }
            else
            {
                ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset,
                                "%u:%u:%u",
                                (pstSrc + usBufLogR)->BufLogData.stTimeStamp.ulWindowId,
                                (pstSrc + usBufLogR)->BufLogData.stTimeStamp.ulTimeDiff,
                                (pstSrc + usBufLogR)->BufLogData.stTimeStamp.ulNumCapField);
            }

            ulStringOffset += BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "\n");
        }

        ulOutputCtr++;
        if(ulOutputCtr >= BVDC_P_BUF_LOG_MAX_OUTPUT)
        {
            BDBG_ERR(("Reached debug message output limit"));
            break;
        }
        else
        {
            /* compute the last entry - match to recent buffer entries only */
            stLastReaderIdx = pBufLog->stLastSkipRepeatIdx;
            BVDC_P_BUF_LOG_IDX_ADD(stLastReaderIdx, BVDC_P_BUF_LOG_ENTRIES_PAD);

            /* adjust to the latest buffer entries only if necessary */
            if( stLastReaderIdx.ulIdx > pBufLog->stWriterIdx.ulIdx )
            {
                stLastReaderIdx =  pBufLog->stWriterIdx;
            }

            /* break when stBufLog_IdxLastSkipRepeat doesn't move anymore */
            if(stReaderIdx.ulIdx == stLastReaderIdx.ulIdx)
            {
                break;
            }
            BVDC_P_BUF_LOG_IDX_INCREMENT(stReaderIdx);
        }
    }


    BKNI_Snprintf(&(pLog[ulStringOffset]), uiLenToRead-ulStringOffset, "<===BRCM\n");

    BDBG_ASSERT(ulStringOffset <= uiLenToRead);

    *puiReadCount = ulStringOffset;
}


void BVDC_P_BufLog_AddEvent_isr
    ( int                        type,
      uint32_t                   v1,
      uint32_t                   v2,
      uint32_t                   v3,
      uint32_t                   v4,
      uint32_t                   v5,
      uint32_t                   v6,
      uint32_t                   v7,
      const BVDC_P_Buffer_Handle hBuffer )

{
    int i, j;
    bool bSkipRepeatEvent = false;
    uint16_t usBufLogW;
    uint32_t ulDiff;
    BVDC_P_BufLog *pBufLog = hBuffer->pBufLog;

    BDBG_ASSERT(hBuffer);
    BDBG_ASSERT(pBufLog);

    if(pBufLog->eState == BVDC_BufLogState_eReset)
        return;

    if(!pBufLog->bEnable[v1])
    {
        return;
    }

    BVDC_P_BUF_LOG_IDX_INCREMENT(pBufLog->stWriterIdx);
    usBufLogW = pBufLog->stWriterIdx.stIdxPart.usIdxEvent;

    pBufLog->astLogInfo[usBufLogW].cLogType = type;

#if (BVDC_P_USE_RDC_TIMESTAMP)
    pBufLog->astLogInfo[usBufLogW].ulTime = BRDC_GetCurrentTimer_isr(hBuffer->hWindow->hCompositor->hVdc->hRdc);
    pBufLog->astLogInfo[usBufLogW].ulMaxTimestamp = BRDC_GetTimerMaxValue(hBuffer->hWindow->hCompositor->hVdc->hRdc);
#else
    if (0 == stTmrRegs.status)
        BTMR_GetTimerRegisters(hBuffer->hWindow->hCompositor->hVdc->hTimer, &stTmrRegs);

    BTMR_ReadTimer_isr(hBuffer->hWindow->hCompositor->hVdc->hTimer, &(pBufLog->astLogInfo[usBufLogW].ulTime));
    pBufLog->astLogInfo[usBufLogW].ulMaxTimestamp = BTMR_ReadTimerMax();
#endif

    if (type <= 'M')                                            /* skip / repeat */
    {
        BVDC_P_PictureNode *pPicture;

        pBufLog->astLogInfo[usBufLogW].BufLogData.stSkipRepeat.ulWindowId = v1;
        pBufLog->astLogInfo[usBufLogW].BufLogData.stSkipRepeat.ulBufferId = v2;
        pBufLog->astLogInfo[usBufLogW].BufLogData.stSkipRepeat.ulBufferNum = hBuffer->ulActiveBufCnt;
        pBufLog->astLogInfo[usBufLogW].BufLogData.stSkipRepeat.ulBufferTS = v4;
        pBufLog->astLogInfo[usBufLogW].BufLogData.stSkipRepeat.ulInfo_1 = v3;
        pBufLog->astLogInfo[usBufLogW].ulInterruptLatency = v5;
        pBufLog->astLogInfo[usBufLogW].BufLogData.stSkipRepeat.ulInfo_2 = v6;
        pBufLog->astLogInfo[usBufLogW].BufLogData.stSkipRepeat.ulInfo_3 = v7;

        pPicture = BLST_CQ_FIRST(hBuffer->pBufList);

        BKNI_Memset((void*)(pBufLog->astLogInfo[usBufLogW].BufLogData.stSkipRepeat.cRW), ' ', sizeof(char) * BVDC_P_BUF_LOG_BUFFERS_NUM_MAX);

        /* ulBufCnt is the max number of buffer that could be allocated.  Usually it is 4.
            We go through the list and find the active ones.  cRW[index] cannot use i as index. */
        for(i = 0, j=0; i < (int)hBuffer->ulBufCnt; i++)
        {
            if(pPicture->stFlags.bActiveNode)
            {
                pBufLog->astLogInfo[usBufLogW].BufLogData.stSkipRepeat.cRW[j++] = (pPicture == hBuffer->pCurReaderBuf) ? 'R'
                                                              : ((pPicture == hBuffer->pCurWriterBuf) ? 'W' : 'x');

                if (j==BVDC_P_BUF_LOG_BUFFERS_NUM_MAX)
                    break;
            }
            pPicture = BVDC_P_Buffer_GetNextNode(pPicture);
        }

        if (pBufLog->stWriterIdx.ulIdx >= pBufLog->stLastSkipRepeatIdx.ulIdx)
        {
            ulDiff = pBufLog->stWriterIdx.ulIdx - pBufLog->stLastSkipRepeatIdx.ulIdx;
        }
        else
        {
            ulDiff = pBufLog->stLastSkipRepeatIdx.ulIdx - pBufLog->stWriterIdx.ulIdx;
        }

        /* Only trigger the dumps on skip/repeat events or when the difference between
           the last skip/repeat and last written entry exceeds 32 entries. The latter
           condition guaranties that no entries will be overwritten before being dumped
           to the console. */
        if ((type <= 'F') || (ulDiff > BVDC_P_BUF_LOG_TRIGGER_DUMP))
            bSkipRepeatEvent = true;
    }
    else if (type <= 'P')
    {
        pBufLog->astLogInfo[usBufLogW].BufLogData.stTimeStamp.ulWindowId = v1;
        pBufLog->astLogInfo[usBufLogW].BufLogData.stTimeStamp.ulTimeDiff = v2;
        pBufLog->astLogInfo[usBufLogW].BufLogData.stTimeStamp.ulNumCapField = v3;
    }
    else
    {
        pBufLog->astLogInfo[usBufLogW].BufLogData.stGeneric.ulU0 = v1;
        pBufLog->astLogInfo[usBufLogW].BufLogData.stGeneric.ulU1 = v2;
        pBufLog->astLogInfo[usBufLogW].BufLogData.stGeneric.ulU2 = v3;
        pBufLog->astLogInfo[usBufLogW].BufLogData.stGeneric.ulU3 = v4;
    }

    if(bSkipRepeatEvent)
    {
        pBufLog->stLastSkipRepeatIdx = pBufLog->stWriterIdx;
        if(pBufLog->eState >= BVDC_BufLogState_eAutomatic)
        {
            if(pBufLog->pfCallback)
                pBufLog->pfCallback(pBufLog->pvCbParm1, pBufLog->iCbParm2, NULL);
            else
                BDBG_ERR(("BufLog-AutoTrigger: User callback function unavailable"));
        }
    }
}


void BVDC_P_BufLog_SetManualTrigger(BVDC_P_BufLog *pBufLog)
{
    if(pBufLog->pfCallback)
    {
        /* Compute the dump-starting point for the manual trigger */
        pBufLog->stLastSkipRepeatIdx = pBufLog->stWriterIdx;
        BVDC_P_BUF_LOG_IDX_SUB(pBufLog->stLastSkipRepeatIdx, BVDC_P_BUF_LOG_ENTRIES_PAD);

        /* Execute the callback */
        pBufLog->pfCallback(pBufLog->pvCbParm1, pBufLog->iCbParm2, NULL);
    }
    else
    {
        BDBG_ERR(("BufLog-ManualTrigger: User callback function unavailable"));
    }
}

void BVDC_P_BufLog_EnableBufLog
    ( BVDC_P_BufLog      *pBufLog,
      BVDC_P_WindowId     eId,
      bool                bEnable)
{
    pBufLog->bEnable[eId] = bEnable;
    BDBG_MSG(("BVDC_P_Buffer_EnableBufLog: %d %d", eId, bEnable));
}

#endif /* BVDC_BUF_LOG == 1 */

/* End of file. */
