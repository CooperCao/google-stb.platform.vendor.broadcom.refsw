/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
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
#include "bstd.h"
#include "bkni.h"
#include "bmem.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_vnet_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_common_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_capture_priv.h"
#include "bvdc_feeder_priv.h"


BDBG_MODULE(BVDC_BUF);
BDBG_FILE_MODULE(BVDC_WIN_BUF);
BDBG_OBJECT_ID(BVDC_BUF);

#if (BVDC_BUF_LOG == 1)
/***************************************************************************
 */

/* Logging Legend

   Depends on the event, each logging entry has the following format:
   * uuuuuuuu:bbbb:mx:y[...W.R]z:iiiiiiii:vvvvvvvv
     + uuuuuuuu: Reference timestamp (microsecond)
     + bbbbb:    Event number index in the circular buffer
     + mx:       Event m of window x
       = Ax:y    Window x writer skips 2 fields, y:pCurWriterBuf->ulBufferId
       = Bx:y    Window x writer skips 1 field/frame, y:pCurWriterBuf->ulBufferId
       = Cx:y    Window x reader repeat 1 field/frame, y:pCurReaderBuf->ulBufferId
       = Dx:y    Window x reader repeat for polarity, y:pCurReaderBuf->ulBufferId
       = Ex:y    Window x new writer IRQ at position y
       = Fx:y    Window x new reader IRQ at position y
     + [...W.R]: Position of reader and writer pointers
     + z:        Number of active buffers
     + iiiiiiii: other info, i.e., number of captured fields, source polarity, vec polarity, etc
     + vvvvvvvv: VDC timestamp

   * uuuuuuuu:bbbb:mx:yyyyyyyy:zzzzz
     + uuuuuuuu: Reference timestamp (microsecond)
     + bbbbb:    Event number index in the circular buffer
     + mx:
       = Hx:     Reader latency on window x
       = Ix:     Writer latency on window x
     + yyyyyyyy: Time delta
     + zzzzz:    Number of captured fields so far

   * uuuuuuuu:bbbb:Jx:R=y W=z
     + uuuuuuuu: Reference timestamp (microsecond)
     + bbbbb:    Event number index in the circular buffer
     + Jx:       Invalidate on window x
     + R=y:      Set reader to position y
     + W=z       Set writer to position z

   * uuuuuuuu:bbbb:mx:yyyyy
     + uuuuuuuu: Reference timestamp (microsecond)
     + bbbbb:    Event number index in the circular buffer
     + mx:
       = Kx:     Reader of window x moved by writer due to misordered ISR (type 1)
       = Lx:     Writer of window x moved by reader due to misordered ISR (type 1)
     + yyyyy:    Number of captured fields

   * uuuuuuuu:bbbb:mx:cnt=y
     + uuuuuuuu:  Reference timestamp (microsecond)
     + bbbbb:     Event number index in the circular buffer
     + mx:
       = Mx:cnt=y Added buffer entry on window x to count=y
       = Nx:cnt=y Removed buffer entry on window x to count=y

   * uuuuuuuu:bbbb:Qx:cccccccc:pppppppp
     + uuuuuuuu:  Reference timestamp (microsecond)
     + bbbbb:     Event number index in the circular buffer
     + Qx:        Timestamp update on window x
     + cccccccc:  VDC capture timestamp (microsecond)
     + pppppppp:  VDC playback timestamp (microsecond)
 */

#define BVDC_P_BUF_LOG_ENTRIES         2048                             /* Number of log entries - must be of powers of 2 */
#define BVDC_P_BUF_LOG_ENTRIES_MASK    (2048-1)                         /* Mask of the number of log entries */
#define BVDC_P_BUF_LOG_ENTRIES_PAD     64                               /* Number of events to dump before and after the last skip/repeat event */
#define BVDC_P_BUF_LOG_MAX_OUTPUT      (BVDC_P_BUF_LOG_ENTRIES_PAD<<1)  /* Maximum number of log entries at each round */
#define BVDC_P_BUF_LOG_BUFFERS_NUM_MAX 16                               /* Maximum number buffer entries of eacn VDC window */
#define BVDC_P_BUF_LOG_TRACE_WINDOW_0  0                                /* The first VDC window to trace */
#define BVDC_P_BUF_LOG_TRACE_WINDOW_1  1                                /* The second VDC window to trace */
#define BVDC_P_BUF_LOG_TRIGGER_DUMP    32                               /* The difference between the last skip/repeat and last written
                                                                           entry exceeds 32 entries. Exceeding this triggers a dump of the
                                                                           log. */

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

typedef union  {
    uint32_t ulIdx;
    struct  {
        uint16_t usIdxEvent;
        uint16_t usIdxRound;
    } stIdxPart;
} BVDC_P_BufLogIdx;

typedef struct {
    uint32_t    ulWindowId;
    uint32_t    ulBufferId;
    char        cRW[BVDC_P_BUF_LOG_BUFFERS_NUM_MAX];
    uint32_t    ulBufferNum;
    uint32_t    ulBufferTS;
    uint32_t    ulInfo;
} BVDC_P_BufLogSkipRepeat;

typedef struct  {
    uint32_t    ulTimeDiff;
    uint32_t    ulWindowId;
    uint32_t    ulNumCapField;
} BVDC_P_BufLogTimeStamp;

typedef struct {
    uint32_t    ulU0;
    uint32_t    ulU1;
    uint32_t    ulU2;
    uint32_t    ulU3;
} BVDC_P_BufLogGeneric;

typedef struct BVDC_P_BufLog {
    char        cLogType;                                      /* See the logging legend above... */
    uint32_t    ulTime;                                        /* Timestamp from the free-running timer (microseconds) */
    union {
        BVDC_P_BufLogSkipRepeat   stSkipRepeat;                /* Skip/Repeat event - events {A, B, C, D} */
        BVDC_P_BufLogTimeStamp    stTimeStamp;                 /* Latency event */
        BVDC_P_BufLogGeneric      stGeneric;                   /* Generic event */
    } BufLogData;
} BVDC_P_BufLog;

static BVDC_P_BufLog          astBufLog[BVDC_P_BUF_LOG_ENTRIES];      /* The circular buffer */
static BVDC_P_BufLogIdx       stBufLog_IdxW = {0};                    /* Writer pointer */
static BVDC_P_BufLogIdx       stBufLog_IdxLastSkipRepeat={0};         /* The position of the last Skip/Repeat event */
static BVDC_BufLogState       eBufLogState = BVDC_BufLogState_eReset; /* State of the logging */
static BVDC_CallbackFunc_isr  pfBufLogCallback = NULL;                /* User registered callback function */
static void                   *pvBufLogCallbackParm1 = NULL;          /* First user callback parameter */
static int                    iBufLogParm2 = 0;                       /* Second user callback parameter */
static BTMR_TimerRegisters    stTmrRegs={0,0};                        /* VDC timer registers */
static bool                   bstBufLogEnable[BVDC_P_WindowId_eComp0_G0]
	= {true, true, true, true, true, true};

void BVDC_P_Buffer_SetLogStateAndDumpTrigger
	( BVDC_BufLogState                    eLogState,
	  const BVDC_CallbackFunc_isr         pfCallback,
	  void                               *pvParm1,
	  int                                 iParm2 )
{
	/* Store user settings */
	eBufLogState = eLogState;
	pfBufLogCallback = pfCallback;
	pvBufLogCallbackParm1 = pvParm1;
	iBufLogParm2 = iParm2;
	return;
}

/*
 * Outputs the log, from BVDC_P_BUF_LOG_ENTRIES_PAD events prior to the first skip/repeat event through BVDC_P_BUF_LOG_ENTRIES_PAD events
 * after the last skip/repeat event.
 * It tries to catch up with the stBufLog_IdxLastSkipRepeat index, that moves whenever another skip/repeat event occurs.
 */
void BVDC_P_Buffer_DumpLog(void)
{
    int              iTmp;
    char             szStringTmp[128];
    uint32_t         ulBufCtr = 0, ulOutputCtr = 0;
    BVDC_P_BufLog    *pstSrc = (BVDC_P_BufLog*)&(astBufLog[0]);
    BVDC_P_BufLogIdx stBufLog_IdxRLast, stBufLog_IdxR;
    uint16_t         usBufLogR;

    /* compute the starting point */
    stBufLog_IdxR = stBufLog_IdxLastSkipRepeat;
    BVDC_P_BUF_LOG_IDX_SUB(stBufLog_IdxR, BVDC_P_BUF_LOG_ENTRIES_PAD);

    /* header marker */
    BDBG_ERR(("BRCM===%08x:%08x:%08x===>", stBufLog_IdxR.ulIdx, stBufLog_IdxLastSkipRepeat.ulIdx, stBufLog_IdxW.ulIdx));
    while(1)
    {
        usBufLogR = stBufLog_IdxR.stIdxPart.usIdxEvent;
        iTmp = BKNI_Snprintf(&(szStringTmp[0]), 128,
                       "%010u:%04u:%c",
                       (pstSrc + usBufLogR)->ulTime,
                       usBufLogR,
                       (pstSrc + usBufLogR)->cLogType);

        if ((pstSrc + usBufLogR)->cLogType <= ((eBufLogState == BVDC_BufLogState_eAutomaticReduced) ? 'D' : 'G'))
        {
            iTmp += BKNI_Snprintf(&(szStringTmp[iTmp]), 128-iTmp,
                            "%u:%u[",
                            (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulWindowId,
                            (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulBufferId);
            for(ulBufCtr=0; ulBufCtr<((pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulBufferNum); ulBufCtr++)
            {
                iTmp += BKNI_Snprintf(&(szStringTmp[iTmp]), 128-iTmp,
                                "%c",
                                (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.cRW[ulBufCtr]);
            }
            iTmp += BKNI_Snprintf(&(szStringTmp[iTmp]), 128-iTmp,
                            "]%u:%08x:%012u",
                            (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulBufferNum,
                            (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulInfo,
                            (pstSrc + usBufLogR)->BufLogData.stSkipRepeat.ulBufferTS);
            if ((pstSrc + usBufLogR)->cLogType <= 'D')
            {
                iTmp += BKNI_Snprintf(&(szStringTmp[iTmp]), 128-iTmp, "<===");  /* mark the skip/repeat event to ease reading of the logs */
            }
            BDBG_ERR(("%s", szStringTmp));

        }
        else if(eBufLogState != BVDC_BufLogState_eAutomaticReduced)
        {
            if((pstSrc + usBufLogR)->cLogType == 'J')
            {
                BDBG_ERR(("%s%u:R=%u W=%u",
                          szStringTmp,
                          (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU0,
                          (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU1,
                          (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU2));

            }
            else if( ((pstSrc + usBufLogR)->cLogType == 'K') || ((pstSrc + usBufLogR)->cLogType == 'L') ||
					 ((pstSrc + usBufLogR)->cLogType == 'U') || ((pstSrc + usBufLogR)->cLogType == 'V') ||
					 ((pstSrc + usBufLogR)->cLogType == 'S') || ((pstSrc + usBufLogR)->cLogType == 'T'))
            {
                BDBG_ERR(("%s%u:%u:%012u:%012u",
                          szStringTmp,
                          (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU0,
                          (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU1,
                          (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU2,
                          (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU3));

            }
            else if( ((pstSrc + usBufLogR)->cLogType == 'M') || ((pstSrc + usBufLogR)->cLogType == 'N') )
            {
                BDBG_ERR(("%s%u:cnt=%u",
                          szStringTmp,
                          (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU0,
                          (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU1));

            }
            else if( ((pstSrc + usBufLogR)->cLogType == 'Q') )
            {
                BDBG_ERR(("%s%u:%012u:%012u",
                          szStringTmp,
                          (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU0,
                          (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU1,
                          (pstSrc + usBufLogR)->BufLogData.stGeneric.ulU2));

            }
            else
            {
                BDBG_ERR(("%s%u:%u:%u",
                          szStringTmp,
                          (pstSrc + usBufLogR)->BufLogData.stTimeStamp.ulWindowId,
                          (pstSrc + usBufLogR)->BufLogData.stTimeStamp.ulTimeDiff,
                          (pstSrc + usBufLogR)->BufLogData.stTimeStamp.ulNumCapField));
            }
        }

        ulOutputCtr++;
        if(ulOutputCtr >= BVDC_P_BUF_LOG_MAX_OUTPUT)
        {
            BDBG_ERR(("Reached output limit"));
            break;
        }
        else
        {
            /* compute the last entry - match to recent buffer entries only */
            stBufLog_IdxRLast = stBufLog_IdxLastSkipRepeat;
            BVDC_P_BUF_LOG_IDX_ADD(stBufLog_IdxRLast, BVDC_P_BUF_LOG_ENTRIES_PAD);

            /* adjust to the latest buffer entries only if necessary */
            if( stBufLog_IdxRLast.ulIdx > stBufLog_IdxW.ulIdx )
            {
                stBufLog_IdxRLast = stBufLog_IdxW;
            }

            /* break when stBufLog_IdxLastSkipRepeat doesn't move anymore */
            if(stBufLog_IdxR.ulIdx == stBufLog_IdxRLast.ulIdx)
            {
                break;
            }
            BVDC_P_BUF_LOG_IDX_INCREMENT(stBufLog_IdxR);
        }
    }
    BDBG_ERR(("<===BRCM"));                                   /* footer marker */
}


static void BVDC_P_BufLogAddEvent_isr( int type,
	                                   uint32_t v1,
                                       uint32_t v2,
                                       uint32_t v3,
                                       uint32_t v4,
                                       const BVDC_P_Buffer_Handle hBuffer
                                     )
{
    int      i, j;
	bool bSkipRepeatEvent = false;
    uint16_t usBufLogW;
	uint32_t ulDiff;

    if(eBufLogState == BVDC_BufLogState_eReset)
        return;

	if(!bstBufLogEnable[v1])
	{
		return;
	}

    BVDC_P_BUF_LOG_IDX_INCREMENT(stBufLog_IdxW);
    usBufLogW = stBufLog_IdxW.stIdxPart.usIdxEvent;

	if (0 == stTmrRegs.status)
		BTMR_GetTimerRegisters(hBuffer->hWindow->hCompositor->hVdc->hTimer, &stTmrRegs);

    astBufLog[usBufLogW].cLogType = type;
    BTMR_ReadTimer_isr(hBuffer->hWindow->hCompositor->hVdc->hTimer, &(astBufLog[usBufLogW].ulTime));

    if (type <= 'G')                                            /* skip / repeat */
    {
        BVDC_P_PictureNode *pPicture;

        astBufLog[usBufLogW].BufLogData.stSkipRepeat.ulWindowId = v1;
        astBufLog[usBufLogW].BufLogData.stSkipRepeat.ulBufferId = v2;
        astBufLog[usBufLogW].BufLogData.stSkipRepeat.ulBufferNum = hBuffer->ulActiveBufCnt;
        astBufLog[usBufLogW].BufLogData.stSkipRepeat.ulBufferTS = v4;
        astBufLog[usBufLogW].BufLogData.stSkipRepeat.ulInfo = v3;

        pPicture = BLST_CQ_FIRST(hBuffer->pBufList);

        BKNI_Memset((void*)(astBufLog[usBufLogW].BufLogData.stSkipRepeat.cRW), ' ', sizeof(char) * BVDC_P_BUF_LOG_BUFFERS_NUM_MAX);

		/* ulBufCnt is the max number of buffer that could be allocated.  Usually it is 4.
            We go through the list and find the active ones.  cRW[index] cannot use i as index. */
        for(i = 0, j=0; i < (int)hBuffer->ulBufCnt; i++)
        {
            if(pPicture->stFlags.bActiveNode)
            {
                astBufLog[usBufLogW].BufLogData.stSkipRepeat.cRW[j++] = (pPicture == hBuffer->pCurReaderBuf) ? 'R'
                                                              : ((pPicture == hBuffer->pCurWriterBuf) ? 'W' : 'x');

                if (j==BVDC_P_BUF_LOG_BUFFERS_NUM_MAX)
                    break;
            }
            pPicture = BVDC_P_Buffer_GetNextNode(pPicture);
        }

		if (stBufLog_IdxW.ulIdx >= stBufLog_IdxLastSkipRepeat.ulIdx)
		{
			ulDiff = stBufLog_IdxW.ulIdx - stBufLog_IdxLastSkipRepeat.ulIdx;
		}
		else
		{
			ulDiff = stBufLog_IdxLastSkipRepeat.ulIdx - stBufLog_IdxW.ulIdx;
		}

		/* Only trigger the dumps on skip/repeat events or when the difference between
		   the last skip/repeat and last written entry exceeds 32 entries. The latter
		   condition guaranties that no entries will be overwritten before being dumped
		   to the console. */
        if ((type <= 'D') || (ulDiff > BVDC_P_BUF_LOG_TRIGGER_DUMP))
            bSkipRepeatEvent = true;
    }
    else if (type <= 'I')
    {
        astBufLog[usBufLogW].BufLogData.stTimeStamp.ulWindowId = v1;
        astBufLog[usBufLogW].BufLogData.stTimeStamp.ulTimeDiff = v2;
        astBufLog[usBufLogW].BufLogData.stTimeStamp.ulNumCapField = v3;
    }
    else
    {
        astBufLog[usBufLogW].BufLogData.stGeneric.ulU0 = v1;
        astBufLog[usBufLogW].BufLogData.stGeneric.ulU1 = v2;
        astBufLog[usBufLogW].BufLogData.stGeneric.ulU2 = v3;
        astBufLog[usBufLogW].BufLogData.stGeneric.ulU3 = v4;
    }

    if(bSkipRepeatEvent)
    {
        stBufLog_IdxLastSkipRepeat = stBufLog_IdxW;
        if(eBufLogState >= BVDC_BufLogState_eAutomatic)
        {
            if(pfBufLogCallback)
                pfBufLogCallback(pvBufLogCallbackParm1, iBufLogParm2, NULL);
            else
                BDBG_ERR(("BufLog-AutoTrigger: User callback function unavailable"));
        }
    }
}


void BVDC_P_Buffer_SetManualTrigger(void)
{
    if(pfBufLogCallback)
    {
        /* Compute the dump-starting point for the manual trigger */
        stBufLog_IdxLastSkipRepeat = stBufLog_IdxW;
        BVDC_P_BUF_LOG_IDX_SUB(stBufLog_IdxLastSkipRepeat, BVDC_P_BUF_LOG_ENTRIES_PAD);

        /* Execute the callback */
        pfBufLogCallback(pvBufLogCallbackParm1, iBufLogParm2, NULL);
    }
    else
    {
        BDBG_ERR(("BufLog-ManualTrigger: User callback function unavailable"));
    }
}

void BVDC_P_Buffer_EnableBufLog
	( BVDC_P_WindowId         eId,
	  bool                    bEnable)
{
	bstBufLogEnable[eId] = bEnable;
	BDBG_MSG(("BVDC_P_Buffer_EnableBufLog: %d %d", eId, bEnable));
}

#endif

/* VEC alignment may have 200 usecs error, RUL sampling of timestamp may have
   another +/-100 usecs error; worst case double-buffer reader/writer pointers
   timestamps relative error may be up to ~400usecs; */
#define BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE   (\
	2 * BVDC_P_MULTIBUFFER_RW_TOLERANCE + BVDC_P_USEC_ALIGNMENT_THRESHOLD)

static void BVDC_P_Buffer_UpdateTimestamps_isr
	( BVDC_Window_Handle               hWindow,
	  const BAVC_Polarity              eSrcPolarity,
	  const BAVC_Polarity              eDispPolarity );

static void BVDC_P_Buffer_CheckWriterIsrOrder_isr
	( BVDC_Window_Handle               hWindow,
	  const BAVC_Polarity              eSrcPolarity,
	  const BAVC_Polarity              eDispPolarity );

static void BVDC_P_Buffer_MoveSyncSlipWriterNode_isr
	( BVDC_Window_Handle               hWindow,
	  const BAVC_Polarity              eSrcPolarity);

static void BVDC_P_Buffer_CheckReaderIsrOrder_isr
	( BVDC_Window_Handle               hWindow,
	  const BAVC_Polarity              eSrcPolarity,
	  const BAVC_Polarity              eDispPolarity );

static void BVDC_P_Buffer_MoveSyncSlipReaderNode_isr
	( BVDC_Window_Handle               hWindow,
	  const BAVC_Polarity              eVecPolarity);


/***************************************************************************
 *
 */
static BVDC_P_PictureNode *BVDC_P_GetWriterByDelay_isr
	( BVDC_P_PictureNode     *pCurReader,
	  uint32_t                ulVsyncDelay )
{
	uint32_t             i = 0;
	BVDC_P_PictureNode  *pTempNode = pCurReader;

	while( i++ < ulVsyncDelay )
	{
		BVDC_P_Buffer_GetNextActiveNode(pTempNode, pTempNode);
	}

	return pTempNode;
}


/***************************************************************************
 *
 */
static BVDC_P_PictureNode *BVDC_P_GetReaderByDelay_isr
	 ( BVDC_P_PictureNode             *pCurWriter,
	   uint32_t                        ulVsyncDelay )
{
	uint32_t i = 0;
	BVDC_P_PictureNode  *pTempNode = pCurWriter;

	while( i++ < ulVsyncDelay )
	{
		BVDC_P_Buffer_GetPrevActiveNode(pTempNode, pTempNode);
	}

	return pTempNode;
}

/***************************************************************************
  * {private}
  * To get the current reader->writer buffer delay;
  */
static uint32_t BVDC_P_Buffer_GetCurrentDelay_isr
	( BVDC_Window_Handle               hWindow )
{
	BVDC_P_PictureNode		*pTempNode;

	uint32_t delay_count;

	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
	BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

	/* So far there are 3 cases that we allow reader and writer point
	 * to the same buffer node:
	 *
	 * 1) game mode: buffer delay is 0;
	 * 2) progressive pull down: buffer delay is the count between reader and writer;
	 * 3) flag bRepeatCurrReader is set when adding buffer nodes: buffer delay is 0.
	 */
	if (hWindow->hBuffer->pCurReaderBuf == hWindow->hBuffer->pCurWriterBuf)
	{
		bool bProgressivePullDown;

		/* buffer delay is always 0 if bRepeatCurrReader is set */
		if (hWindow->bRepeatCurrReader)
			return 0;
		/* Check whether it is caused by progress pull down */
		BVDC_P_Buffer_CalculateRateGap_isr(hWindow->stCurInfo.hSource->ulVertFreq,
			hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
			&hWindow->hBuffer->eWriterVsReaderRateCode, &hWindow->hBuffer->eReaderVsWriterRateCode);

		/* forced synclocked double-buffer might have interlaced pulldown reader overlapped with writer pointer */
		bProgressivePullDown =	VIDEO_FORMAT_IS_PROGRESSIVE(hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->eVideoFmt) &&
								(hWindow->hBuffer->eWriterVsReaderRateCode == BVDC_P_WrRate_NotFaster) &&
								(hWindow->hBuffer->eReaderVsWriterRateCode == BVDC_P_WrRate_2TimesFaster);

		if (!bProgressivePullDown)
			return 0;
	}

	delay_count = 1;
	BVDC_P_Buffer_GetNextActiveNode(pTempNode, hWindow->hBuffer->pCurReaderBuf);

	while (pTempNode != hWindow->hBuffer->pCurWriterBuf)
	{
		BVDC_P_Buffer_GetNextActiveNode(pTempNode, pTempNode);
		delay_count++;
	}

	return delay_count;
}


/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Buffer_InitPictureNode
	( BVDC_P_PictureNode              *pPicture )
{
	/* Fill-in the default info for each node. */
	pPicture->pHeapNode            = NULL;
	pPicture->pHeapNode_R          = NULL;
	BKNI_Memset((void*)&pPicture->stFlags, 0x0, sizeof(BVDC_P_PicNodeFlags));
	pPicture->stFlags.bMute           = true;
	pPicture->stFlags.bCadMatching    = false;

	BVDC_P_CLEAN_ALL_DIRTY(&(pPicture->stVnetMode));
	pPicture->stVnetMode.stBits.bInvalid  = BVDC_P_ON;
	pPicture->eFrameRateCode       = BAVC_FrameRateCode_e29_97;
	pPicture->eMatrixCoefficients  = BAVC_MatrixCoefficients_eSmpte_170M;
	pPicture->eDisplayPolarity     = BAVC_Polarity_eTopField;
	pPicture->eSrcPolarity         = BAVC_Polarity_eTopField;
	pPicture->eSrcOrientation      = BFMT_Orientation_e2D;
	pPicture->eDispOrientation     = BFMT_Orientation_e2D;
	pPicture->PicComRulInfo.eSrcOrigPolarity = BAVC_Polarity_eTopField;
	pPicture->eDstPolarity         = BAVC_Polarity_eTopField;
	pPicture->pSurface             = NULL;
	pPicture->pSurface_R           = NULL;

	pPicture->bValidTimeStampDelay = false;

	pPicture->stSrcOut.lLeft       = 0;
	pPicture->stSrcOut.lLeft_R     = 0;
	pPicture->stSrcOut.lTop        = 0;
	pPicture->stSrcOut.ulWidth     = BFMT_NTSC_WIDTH;
	pPicture->stSrcOut.ulHeight    = BFMT_NTSC_HEIGHT;

	pPicture->stSclOut             = pPicture->stSrcOut;
	pPicture->stWinOut             = pPicture->stSrcOut;
	pPicture->stSclCut             = pPicture->stSrcOut;
	pPicture->stCapOut             = pPicture->stSrcOut;
	pPicture->stVfdOut             = pPicture->stSrcOut;
	pPicture->stMadOut             = pPicture->stSrcOut;

	pPicture->pSrcOut              = &pPicture->stSrcOut;
	pPicture->pSclOut              = &pPicture->stSclOut;
	pPicture->pWinOut              = &pPicture->stWinOut;
	pPicture->pVfdOut              = &pPicture->stVfdOut;
	pPicture->pVfdIn               = &pPicture->stVfdOut;

	pPicture->pDnrOut              = NULL;
	pPicture->pAnrOut              = NULL;
	pPicture->pMadOut              = NULL;
	pPicture->pCapOut              = NULL;
	pPicture->pDnrIn               = NULL;
	pPicture->pAnrIn               = NULL;
	pPicture->pMadIn               = NULL;
	pPicture->pSclIn               = NULL;
	pPicture->pCapIn               = NULL;

	pPicture->pWinIn               = NULL;

	pPicture->ulCaptureTimestamp   = 0;
	pPicture->ulPlaybackTimestamp  = 0;
	pPicture->ulIdrPicID           = 0;
	pPicture->ulPicOrderCnt        = 0;

	/* used for NRT mode transcode: default true to freeze STC and not get encoded */
	pPicture->bIgnorePicture       = true;
	pPicture->bStallStc            = true;
	pPicture->bEnable10Bit         = false;
	pPicture->bEnableDcxm          = false;

	/* used for inbound STG Fmt switch */
	pPicture->pStgFmtInfo          = (BFMT_VideoInfo *)BFMT_GetVideoFormatInfoPtr(BFMT_VideoFmt_eNTSC);

	pPicture->stCustomFormatInfo.eVideoFmt = BFMT_VideoFmt_eCustom2;
	pPicture->stCustomFormatInfo.ulDigitalWidth  = pPicture->stCustomFormatInfo.ulScanWidth  = pPicture->stCustomFormatInfo.ulWidth = 352;
	pPicture->stCustomFormatInfo.ulDigitalHeight = pPicture->stCustomFormatInfo.ulScanHeight = pPicture->stCustomFormatInfo.ulWidth = 288;
	pPicture->stCustomFormatInfo.ulTopActive = pPicture->stCustomFormatInfo.ulActiveSpace
		= pPicture->stCustomFormatInfo.ulTopMaxVbiPassThru = pPicture->stCustomFormatInfo.ulBotMaxVbiPassThru = 0;
	pPicture->stCustomFormatInfo.ulVertFreq     = 5000;
	pPicture->stCustomFormatInfo.ulPxlFreqMask  = BFMT_PXL_27MHz;
	pPicture->stCustomFormatInfo.bInterlaced    = false,
	pPicture->stCustomFormatInfo.eAspectRatio   = BFMT_AspectRatio_e4_3,
	pPicture->stCustomFormatInfo.eOrientation   = BFMT_Orientation_e2D,
	pPicture->stCustomFormatInfo.ulPxlFreq      = 2700,
	pPicture->stCustomFormatInfo.pchFormatStr   =
										BDBG_STRING("BFMT_VideoFmt_eCustom2");
	pPicture->stCustomFormatInfo.pCustomInfo    = NULL;

	/* mosaic data init*/
	{

		pPicture->eMadPixelHeapId = BVDC_P_BufferHeapId_eUnknown;
		pPicture->eMadQmHeapId    = BVDC_P_BufferHeapId_eUnknown;
	}
	BKNI_Memset((void*)&pPicture->stCapCompression,
		0x0, sizeof(BVDC_P_Compression_Settings));

	return;
}


/***************************************************************************
 * {private}
 *
 * This function creates a multi-buffer list with zero (0) node.
 */
BERR_Code BVDC_P_Buffer_Create
	( const BVDC_Window_Handle         hWindow,
	  BVDC_P_Buffer_Handle            *phBuffer )
{
	uint32_t i;
	BVDC_P_BufferContext *pBuffer;

	BDBG_ENTER(BVDC_P_Buffer_Create);
	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

	/* BDBG_SetModuleLevel("BVDC_BUF", BDBG_eMsg);  */

	/* (1) Create buffer context */
	pBuffer = (BVDC_P_BufferContext*)BKNI_Malloc(sizeof(BVDC_P_BufferContext));
	if(!pBuffer)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	/* Clear out context */
	BKNI_Memset((void*)pBuffer, 0x0, sizeof(BVDC_P_BufferContext));
	BDBG_OBJECT_SET(pBuffer, BVDC_BUF);

	/* These fields do not change during runtime. */
	pBuffer->hWindow      = hWindow;
	pBuffer->ulBufCnt     = BVDC_P_MAX_MULTI_BUFFER_COUNT;

	/* (2) Create buffer head */
	pBuffer->pBufList =
		(BVDC_P_Buffer_Head*)BKNI_Malloc(sizeof(BVDC_P_Buffer_Head));
	if(!pBuffer->pBufList)
	{
		BDBG_OBJECT_DESTROY(pBuffer, BVDC_BUF);
		BKNI_Free(pBuffer);
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}
	BLST_CQ_INIT(pBuffer->pBufList);

	/* (3) Create picture nodes */
	for(i = 0; i < pBuffer->ulBufCnt; i++)
	{
		BVDC_P_PictureNode *pPicture =
			(BVDC_P_PictureNode*)BKNI_Malloc(sizeof(BVDC_P_PictureNode));
		if(!pPicture)
		{
			while(i--)
			{
				pPicture = BLST_CQ_FIRST(pBuffer->pBufList);
				BLST_CQ_REMOVE_HEAD(pBuffer->pBufList, link);
				BKNI_Free(pPicture);
			}
			BDBG_OBJECT_DESTROY(pBuffer, BVDC_BUF);
			BKNI_Free(pBuffer->pBufList);
			BKNI_Free(pBuffer);
			return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		}

		/* Clear out, insert it into the list. */
		BKNI_Memset((void*)pPicture, 0x0, sizeof(BVDC_P_PictureNode));

		/* Initialize non-changing fields. */
		pPicture->hBuffer    = (BVDC_P_Buffer_Handle)pBuffer;
		pPicture->ulBufferId = i;

		BLST_CQ_INSERT_TAIL(pBuffer->pBufList, pPicture, link);
	}
	/* All done. now return the new fresh context to user. */
	*phBuffer = (BVDC_P_Buffer_Handle)pBuffer;

	BDBG_LEAVE(BVDC_P_Buffer_Create);
	return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Buffer_Destroy
	( BVDC_P_Buffer_Handle             hBuffer )
{
	BVDC_P_PictureNode         *pPicture;

	BDBG_ENTER(BVDC_P_Buffer_Destroy);
	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

	/* [3] Free memory for individual buffer node */
	while(hBuffer->ulBufCnt--)
	{
		pPicture = BLST_CQ_FIRST(hBuffer->pBufList);
		BLST_CQ_REMOVE_HEAD(hBuffer->pBufList, link);
		BKNI_Free(pPicture);
	}

	/* [2] Free memory for buffer head. */
	BKNI_Free((void*)hBuffer->pBufList);

	BDBG_OBJECT_DESTROY(hBuffer, BVDC_BUF);
	/* [1] Free memory for main context. */
	BKNI_Free((void*)hBuffer);

	BDBG_LEAVE(BVDC_P_Buffer_Destroy);
	return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Buffer_Init
	( BVDC_P_Buffer_Handle             hBuffer )
{
	uint32_t i;
	BVDC_P_PictureNode   *pPicture;

	BDBG_ENTER(BVDC_P_Buffer_Init);
	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

	/* Re-Initialize fields that may changes during previous run. */
	hBuffer->bSyncLock        = false;
	hBuffer->ulSkipCnt        = 0;
	hBuffer->ulNumCapField    = 0;
	hBuffer->ulActiveBufCnt   = 0;
	hBuffer->ulVsyncDelay     = 0;
	hBuffer->eWriterVsReaderRateCode  = BVDC_P_WrRate_NotFaster;
	hBuffer->eReaderVsWriterRateCode  = BVDC_P_WrRate_NotFaster;
	hBuffer->eLastBuffAction  = BVDC_P_Last_Buffer_Action_Reader_Moved;

	/* For reader and writer ISR ordering */
	hBuffer->bWriterNodeMovedByReader = false;
	hBuffer->bReaderNodeMovedByWriter = false;
	hBuffer->ulPrevWriterTimestamp = 0;
	hBuffer->ulCurrWriterTimestamp = 0;
	hBuffer->ulPrevReaderTimestamp = 0;
	hBuffer->ulCurrReaderTimestamp = 0;
	hBuffer->bReaderWrapAround = false;
	hBuffer->bWriterWrapAround = false;

#if (BVDC_P_USE_RDC_TIMESTAMP)
	hBuffer->ulMaxTimestamp = BRDC_GetTimerMaxValue(hBuffer->hWindow->hCompositor->hVdc->hRdc);
#else
	hBuffer->ulMaxTimestamp = BTMR_ReadTimerMax();
#endif

	hBuffer->ulGameDelaySamplePeriod = 1;
	hBuffer->ulGameDelaySampleCnt    = 0;

	/* Keep track of skip/repeat statistics */
	hBuffer->ulSkipStat       = 0;
	hBuffer->ulRepeatStat     = 0;

	/* Default for reader and writer. */
	hBuffer->pCurReaderBuf    = BLST_CQ_FIRST(hBuffer->pBufList);
	hBuffer->pCurWriterBuf    = hBuffer->pCurReaderBuf;

	hBuffer->bMtgMadDisplay1To1RateRelationship = false;
	hBuffer->bMtgRepeatMode = false;
	hBuffer->ulMtgSrcRepeatCount = 0;
	hBuffer->ulMtgUniquePicCount = 0;
	hBuffer->ulMtgDisplayRepeatCount = 0;

	/* Initialize all the picture nodes. */
	pPicture = hBuffer->pCurReaderBuf;
	for(i = 0; i < hBuffer->ulBufCnt; i++)
	{
		BVDC_P_Buffer_InitPictureNode(pPicture);
		pPicture = BVDC_P_Buffer_GetNextNode(pPicture);
	}

	BKNI_Memset((void*)hBuffer->aBufAdded, 0x0, sizeof(hBuffer->aBufAdded));
	hBuffer->iLastAddedBufIndex = 0;

#if BVDC_P_REPEAT_ALGORITHM_ONE
	hBuffer->bRepeatForGap = false;
#endif

	BDBG_LEAVE(BVDC_P_Buffer_Init);
}


/***************************************************************************
 * {private}
 *
 * Add additioanl picture node to the buffer context.
 *
 * ahSurface is the array of all the surface allocated for the buffer/window.
 * Totally ulSurfaceCount surfaces will be added. The index of surfaces added
 * are:
 * hBuffer->ulActiveBufCnt ... hBuffer->ulActiveBufCnt + ulSurfaceCount - 1
 */
BERR_Code BVDC_P_Buffer_AddPictureNodes_isr
	( BVDC_P_Buffer_Handle             hBuffer,
	  BVDC_P_HeapNodePtr               apHeapNode[],
	  BVDC_P_HeapNodePtr               apHeapNode_R[],
	  uint32_t                         ulSurfaceCount,
	  uint32_t                         ulBufDelay,
	  bool                             bSyncLock,
	  bool                             bInvalidate)
{
	uint32_t                    i;
	BVDC_P_PictureNode         *pPicture;
	BERR_Code                   err = BERR_SUCCESS;

#if (!BVDC_P_SUPPORT_3D_VIDEO)
	BSTD_UNUSED(apHeapNode_R);
#endif

	BDBG_ENTER(BVDC_P_Buffer_AddPictureNodes_isr);
	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

	if(ulSurfaceCount + hBuffer->ulActiveBufCnt > BVDC_P_MAX_MULTI_BUFFER_COUNT)
	{
		BDBG_ERR(("More than MAX!"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Update delay */
	hBuffer->ulVsyncDelay = ulBufDelay;

	/* 1) always add the new buffer node right after the current writer*/
	for(i = 0; i < ulSurfaceCount; i++)
	{
		BDBG_ASSERT(apHeapNode);
		BDBG_ASSERT(apHeapNode[i]);

		pPicture = BVDC_P_Buffer_GetNextNode(hBuffer->pCurWriterBuf);

		while(pPicture->stFlags.bActiveNode || pPicture->stFlags.bUsedByUser)
		{
			pPicture = BVDC_P_Buffer_GetNextNode(pPicture);
		}

		/* Get a non active node */
		pPicture->pHeapNode        = apHeapNode[i];
#if (BVDC_P_SUPPORT_3D_VIDEO)
		if(apHeapNode_R)
			pPicture->pHeapNode_R      = apHeapNode_R[i];
		else
			pPicture->pHeapNode_R      = NULL;
#endif
		pPicture->stFlags.bActiveNode      = true;
		pPicture->stFlags.bMute            = true;
		pPicture->eDisplayPolarity = BAVC_Polarity_eTopField;
		pPicture->eSrcOrientation  = BFMT_Orientation_e2D;
		pPicture->eDispOrientation = BFMT_Orientation_e2D;

		/* We will have to reposition this newly allocated picture node.
		 */
		/* Take the node out from the buffer chain.
		 */
		BLST_CQ_REMOVE(hBuffer->pBufList, pPicture, link);

		/* Add this node back to the chain. Place it to be
		 * the one right after the current writer.
		 */
		BLST_CQ_INSERT_AFTER(hBuffer->pBufList, hBuffer->pCurWriterBuf, pPicture, link);

		hBuffer->ulActiveBufCnt++;
		hBuffer->aBufAdded[hBuffer->iLastAddedBufIndex] = pPicture;
		hBuffer->iLastAddedBufIndex++;

		/* Buffer initialization, point current writer to a active buffer */
		if (!hBuffer->pCurWriterBuf->stFlags.bActiveNode)
			hBuffer->pCurWriterBuf = pPicture;

#if (BVDC_BUF_LOG == 1)
		BVDC_P_BufLogAddEvent_isr('M',
							hBuffer->hWindow->eId,
							hBuffer->ulActiveBufCnt,
							0,
							0,
							hBuffer);
#else

		BDBG_MSG(("Add buffer heap node %p (%s %2d) to B%d",
			(void *)apHeapNode[i],
			BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(apHeapNode[i]->pHeapInfo->eBufHeapId),
			apHeapNode[i]->ulBufIndex, pPicture->ulBufferId));
#if (BVDC_P_SUPPORT_3D_VIDEO)
		if(apHeapNode_R)
		{
			BDBG_MSG(("Add Right buffer heap node %p (%s %2d) to B%d",
				(void *)apHeapNode_R[i],
				BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(apHeapNode_R[i]->pHeapInfo->eBufHeapId),
				apHeapNode_R[i]->ulBufIndex, pPicture->ulBufferId));
		}
#endif
#endif
	}

	hBuffer->bSyncLock = bSyncLock;

	if (bInvalidate)
	{
		hBuffer->pCurReaderBuf = BVDC_P_GetReaderByDelay_isr(
			hBuffer->pCurWriterBuf, ulBufDelay);
	}
	else
	{
		/* 2) Set repeat current reader flag until buffer delay is reached;
		   force reader repeat until the reader/writer buffer delay catches up
		   with hBuffer->ulVsyncDelay; resume normal movement of buffer pointers
		   afterwards. */
		hBuffer->hWindow->bRepeatCurrReader = true;
	}

	BDBG_LEAVE(BVDC_P_Buffer_AddPictureNodes_isr);
	return err;
}


/***************************************************************************
 *
 */
static BVDC_P_PictureNode *BVDC_P_Buffer_LastAddedNonUserCaptureNode_isr
	( BVDC_P_Buffer_Handle             hBuffer )
{
	int tempIdx, i;
	BVDC_P_PictureNode *pPicture;

	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

	/* Locate the last added but not used for user capture buffer
	 * node.
	 */
	tempIdx = hBuffer->iLastAddedBufIndex - 1;

	while(hBuffer->aBufAdded[tempIdx]->stFlags.bUsedByUser)
	{
		BDBG_ASSERT(tempIdx);
		tempIdx--;
	}

	pPicture = hBuffer->aBufAdded[tempIdx];

	/* Shift all the nodes before this one down so that
	 * there is no hole in aBufAdded[] array.
	 */
	for (i = tempIdx; i < (hBuffer->iLastAddedBufIndex - 1); i++)
		hBuffer->aBufAdded[i] = hBuffer->aBufAdded[i+1];

	return pPicture;

}



/***************************************************************************
 * {private}
 *
 * Release picture nodes from the buffer context.
 *
 * ahSurface is the array of all the surface allocated for the buffer/window.
 * Totally ulSurfaceCount surfaces will be released. The index of surfaces
 * released are:
 * pBuffer->ulActiveBufCnt - ulSurfaceCount... pBuffer->ulActiveBufCnt - 1
 *
 */
BERR_Code BVDC_P_Buffer_ReleasePictureNodes_isr
	( BVDC_P_Buffer_Handle             hBuffer,
	  BVDC_P_HeapNodePtr               apHeapNode[],
	  BVDC_P_HeapNodePtr               apHeapNode_R[],
	  uint32_t                         ulSurfaceCount,
	  uint32_t                         ulBufDelay)
{
	uint32_t                    i;
	BVDC_P_PictureNode         *pBufferToRemove;
	BERR_Code                   err = BERR_SUCCESS;

#if (!BVDC_P_SUPPORT_3D_VIDEO)
	BSTD_UNUSED(apHeapNode_R);
#endif

	BDBG_ENTER(BVDC_P_Buffer_ReleasePictureNodes_isr);
	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

	if(hBuffer->ulActiveBufCnt < ulSurfaceCount)
	{
		BDBG_ERR(("Less than MIN!"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	hBuffer->ulVsyncDelay = ulBufDelay;

	/* Always remove the last added buffer node, but it
	 * must not be marked as user capture buffer
	 */
	for(i = 0; i < ulSurfaceCount; i++)
	{
		BDBG_ASSERT(hBuffer->iLastAddedBufIndex);

		pBufferToRemove = BVDC_P_Buffer_LastAddedNonUserCaptureNode_isr(hBuffer);
		pBufferToRemove->stFlags.bActiveNode = false;
		apHeapNode[i] = pBufferToRemove->pHeapNode;
#if (BVDC_P_SUPPORT_3D_VIDEO)
		if(apHeapNode_R)
			apHeapNode_R[i] = pBufferToRemove->pHeapNode_R;
#endif

#if (BVDC_BUF_LOG == 1)
		hBuffer->iLastAddedBufIndex--;
		hBuffer->ulActiveBufCnt--;

		BVDC_P_BufLogAddEvent_isr('N',
							hBuffer->hWindow->eId,
							hBuffer->ulActiveBufCnt,
							0,
							0,
							hBuffer);
#else
		BDBG_MSG(("Release buffer heap node %p (%s %2d) to B%d",
			(void *)pBufferToRemove->pHeapNode,
			BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(apHeapNode[i]->pHeapInfo->eBufHeapId),
			pBufferToRemove->pHeapNode->ulBufIndex, pBufferToRemove->ulBufferId));
#if (BVDC_P_SUPPORT_3D_VIDEO)
		if(apHeapNode_R)
		{
			BDBG_MSG(("Release Right buffer heap node %p (%s %2d) to B%d",
				(void *)pBufferToRemove->pHeapNode_R,
				BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(apHeapNode_R[i]->pHeapInfo->eBufHeapId),
				pBufferToRemove->pHeapNode->ulBufIndex, pBufferToRemove->ulBufferId));
		}
#endif

		hBuffer->iLastAddedBufIndex--;
		hBuffer->ulActiveBufCnt--;
#endif
	}

	if(hBuffer->ulActiveBufCnt)
	{
		/* Current reader and writer nodes might have been released
		 * during the above process. So we have to reposition them.
		 */
		while(!hBuffer->pCurWriterBuf->stFlags.bActiveNode)
			BVDC_P_Buffer_GetNextActiveNode(hBuffer->pCurWriterBuf, hBuffer->pCurWriterBuf);

		hBuffer->pCurReaderBuf = BVDC_P_GetReaderByDelay_isr(
			hBuffer->pCurWriterBuf, ulBufDelay);
	}
	else
	{
		hBuffer->pCurReaderBuf = BLST_CQ_FIRST(hBuffer->pBufList);
		hBuffer->pCurWriterBuf = hBuffer->pCurReaderBuf;
	}

	BDBG_LEAVE(BVDC_P_Buffer_ReleasePictureNodes_isr);
	return err;
}



#if (BVDC_P_SUPPORT_3D_VIDEO)
/***************************************************************************
 * {private}
 * Add or free picture node for right capture buffer.
 *
 */
BERR_Code BVDC_P_Buffer_SetRightBufferPictureNodes_isr
	( BVDC_P_Buffer_Handle             hBuffer,
	  BVDC_P_HeapNodePtr               apHeapNode_R[],
	  uint32_t                         ulCount,
	  bool                             bAdd)
{
	uint32_t                    i;
	BVDC_P_PictureNode         *pPicture;
	BERR_Code                   err = BERR_SUCCESS;

	BDBG_ENTER(BVDC_P_Buffer_SetRightBufferPictureNodes_isr);
	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

	BDBG_ASSERT(ulCount == hBuffer->ulActiveBufCnt);

	/* Start with current writer*/
	pPicture = hBuffer->pCurWriterBuf;

	if(bAdd)
	{
		for(i = 0; i < ulCount; i++)
		{
			/* Get next active node */
			while(!pPicture->stFlags.bActiveNode || pPicture->pHeapNode_R)
			{
				BVDC_P_Buffer_GetNextActiveNode(pPicture, pPicture);
			}
			pPicture->pHeapNode_R = apHeapNode_R[i];
			BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Add Right buffer heap node %p (%s %2d) to B%d (%d)",
				(void *)apHeapNode_R[i],
				BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(apHeapNode_R[i]->pHeapInfo->eBufHeapId),
				apHeapNode_R[i]->ulBufIndex, pPicture->ulBufferId,
				pPicture->stFlags.bMute));
		}
	}
	else
	{
		for(i = 0; i < ulCount; i++)
		{
			/* Get next active node */
			while(!pPicture->stFlags.bActiveNode || (pPicture->pHeapNode_R == NULL))
			{
				BVDC_P_Buffer_GetNextActiveNode(pPicture, pPicture);
			}
			BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Free Right buffer heap node %p (%s %2d) from B%d",
				(void *)pPicture->pHeapNode_R,
				BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(pPicture->pHeapNode_R->pHeapInfo->eBufHeapId),
				pPicture->pHeapNode_R->ulBufIndex, pPicture->ulBufferId));
			apHeapNode_R[i] = pPicture->pHeapNode_R;
			pPicture->pHeapNode_R = NULL;
		}
	}


	BDBG_LEAVE(BVDC_P_Buffer_SetRightBufferPictureNodes_isr);
	return err;
}

#endif

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Buffer_Invalidate_isr
	( BVDC_P_Buffer_Handle             hBuffer )
{
	uint32_t                    ulCount;
	BVDC_P_PictureNode         *pTempNode;

	BDBG_ENTER(BVDC_P_Buffer_Invalidate_isr);
	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

	BDBG_MSG(("Invalidate buffer nodes"));

	/* Invalidate all capture data */
	pTempNode = hBuffer->pCurReaderBuf;
	for( ulCount = 0; ulCount < hBuffer->ulBufCnt; ulCount++ )
	{
		pTempNode->eDstPolarity               = BAVC_Polarity_eTopField;
		pTempNode->eDisplayPolarity           = BAVC_Polarity_eTopField;
		pTempNode->eOrigSrcOrientation        = BFMT_Orientation_e2D;
		pTempNode->eSrcOrientation            = BFMT_Orientation_e2D;
		pTempNode->eDispOrientation           = BFMT_Orientation_e2D;
#if (BVDC_P_DCX_3D_WORKAROUND)
		pTempNode->bEnableDcxm = hBuffer->hWindow->bSupportDcxm &&
			!BFMT_IS_3D_MODE(hBuffer->hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt);
		pTempNode->bEnable10Bit = hBuffer->hWindow->bIs10BitCore && pTempNode->bEnableDcxm;
#else
		pTempNode->bEnable10Bit               = hBuffer->hWindow->bIs10BitCore;
		pTempNode->bEnableDcxm                = hBuffer->hWindow->bSupportDcxm;
#endif
		pTempNode->stFlags.bMute              = true;
		pTempNode->stFlags.bMuteMad           = false;
		pTempNode->stFlags.bPictureRepeatFlag = false;
		pTempNode->stFlags.bRepeatField       = false;
		pTempNode->stFlags.bCadMatching       = false;
		pTempNode->ulAdjQp                    = 0;
		pTempNode->ulCaptureTimestamp         = 0;
		pTempNode->ulPlaybackTimestamp        = 0;
		pTempNode = BVDC_P_Buffer_GetNextNode(pTempNode);
	}

	/* reset capture number */
	hBuffer->ulNumCapField  = 0;
	hBuffer->ulSkipCnt      = 0;
	hBuffer->pCurWriterBuf = BVDC_P_GetWriterByDelay_isr(
		hBuffer->pCurReaderBuf, hBuffer->ulVsyncDelay);
	hBuffer->eLastBuffAction = BVDC_P_Last_Buffer_Action_Reader_Moved;
	hBuffer->bWriterNodeMovedByReader = false;
	hBuffer->bReaderNodeMovedByWriter = false;
	hBuffer->ulPrevWriterTimestamp = 0;
	hBuffer->ulCurrWriterTimestamp = 0;
	hBuffer->ulPrevReaderTimestamp = 0;
	hBuffer->ulCurrReaderTimestamp = 0;

	hBuffer->bMtgMadDisplay1To1RateRelationship = false;
	hBuffer->bMtgRepeatMode = false;
	hBuffer->ulMtgSrcRepeatCount = 0;
	hBuffer->ulMtgUniquePicCount = 0;
	hBuffer->ulMtgDisplayRepeatCount = 0;

#if BVDC_P_REPEAT_ALGORITHM_ONE
	hBuffer->bRepeatForGap = false;
#endif

#if (BVDC_BUF_LOG == 1)
	BVDC_P_BufLogAddEvent_isr('J',
						hBuffer->hWindow->eId,
						hBuffer->pCurReaderBuf->ulBufferId,
						hBuffer->pCurWriterBuf->ulBufferId,
						0,
						hBuffer);

#else
	BDBG_MSG(("Set reader buffer node to B%d", hBuffer->pCurReaderBuf->ulBufferId));
	BDBG_MSG(("Set writer buffer node to B%d", hBuffer->pCurWriterBuf->ulBufferId));
#endif

	BDBG_LEAVE(BVDC_P_Buffer_Invalidate_isr);
	return BERR_SUCCESS;
}

#if 0
/***************************************************************************
 *
 */
BVDC_P_PictureNode* BVDC_P_Buffer_GetPrevWriterNode_isr
	( const BVDC_P_Buffer_Handle       hBuffer )
{
	BVDC_P_PictureNode     *pPrevNode;

	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

	if(!hBuffer->ulActiveBufCnt)
		return hBuffer->pCurWriterBuf;

	BVDC_P_Buffer_GetPrevActiveNode(pPrevNode, hBuffer->pCurWriterBuf);
	return pPrevNode;
}
#endif

#if 0
/***************************************************************************
 *
 */
void BVDC_P_Buffer_SetCurWriterNode_isr
	( const BVDC_P_Buffer_Handle       hBuffer,
	  BVDC_P_PictureNode              *pPicture )
{
	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

	hBuffer->pCurWriterBuf = pPicture;
}
#endif

#if 0
/***************************************************************************
 *
 */
void BVDC_P_Buffer_SetCurReaderNode_isr
	( const BVDC_P_Buffer_Handle       hBuffer,
	  BVDC_P_PictureNode              *pPicture )
{
	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

	hBuffer->pCurReaderBuf = pPicture;
}
#endif

/***************************************************************************
 *
 */
void  BVDC_P_Buffer_ReturnBuffer_isr
	( BVDC_P_Buffer_Handle            hBuffer,
	  BVDC_P_PictureNode             *pPicture )
{
	BVDC_P_PictureNode *pTempNode, *pNextTempNode;

	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

	pPicture->stFlags.bUsedByUser = false;
	pPicture->pSurface = NULL;
	pPicture->stFlags.bMute = true;

	/* TODO: if returned buf heapId is not the same, then free it and re-alloc
	 * a buf with right heapId */

	/* If the chain has no active node, it means we are not doing
	 * capture any more. Then mark the picture node as not used by user
	 * and not active.
	 */
	if (hBuffer->ulActiveBufCnt == 0)
	{
		pPicture->stFlags.bActiveNode = false;
	}
	else
	{
		/* We will have to reposition this returned picture node.
		 * It can not be inserted between reader and writer since
		 * that will affect vsync delay. So we insert it right
		 * after the current writer.
		 */

		/* Take the node out from the buffer chain.
		 */
		BLST_CQ_REMOVE(hBuffer->pBufList, pPicture, link);

		/* Add this node back to the chain. Place it to be
		 * the one right after the current writer.
		 */
		pNextTempNode = BVDC_P_Buffer_GetNextNode(hBuffer->pCurWriterBuf);
		pPicture->eDstPolarity = pNextTempNode->eDstPolarity;
		BLST_CQ_INSERT_AFTER(hBuffer->pBufList, hBuffer->pCurWriterBuf, pPicture, link);

		/* Increment active buffer count */
		pPicture->stFlags.bActiveNode = true;
		hBuffer->ulActiveBufCnt++;

		/* Toggle the picture node destination polarity pointed
		 * to by the nodes after newly adde node but before the reader. This
		 * is necessary to keep the field prediction correct.
		 */
		BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pTempNode, pPicture);

		while (pTempNode != hBuffer->pCurReaderBuf)
		{
			pTempNode->eDstPolarity = BVDC_P_NEXT_POLARITY(pTempNode->eDstPolarity);
			BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextTempNode, pTempNode);
			pTempNode = pNextTempNode;
		} ;
	}

	return;
}


/***************************************************************************
 *
 */
BERR_Code  BVDC_P_Buffer_ExtractBuffer_isr
	( BVDC_P_Buffer_Handle            hBuffer,
	  BVDC_P_PictureNode            **ppPicture )
{
	BVDC_P_PictureNode *pTempNode = NULL, *pNextTempNode = NULL, *pPrevWriterNode = NULL;

	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);

	/* Criterion for such a node:
	 * 1) Active
	 * 2) Not used by user
	 * 3) Not current writer buffer
	 * 4) Not the previous writer buffer which could be in use
	 * 5) Not between reader and writer because taking one of those out will affect
	 *	  lipsync delay.
	 * 6) The buffer is not muted.
	 * 7) The buffer's pixel format matches with user specified.
	 *
	 * The one after current writer should satisfy the above criterion.
	 */

	BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pTempNode, hBuffer->pCurWriterBuf);
	BVDC_P_Buffer_GetPrevActiveAndNotUsedByUserNode(pPrevWriterNode, hBuffer->pCurWriterBuf);

	if ((pTempNode == hBuffer->pCurWriterBuf) || (pTempNode == pPrevWriterNode)
		|| (pTempNode == hBuffer->pCurReaderBuf) || pTempNode->stFlags.bMute
		|| (pTempNode->ePixelFormat != hBuffer->hWindow->stCurInfo.ePixelFormat)
		|| (hBuffer->ulActiveBufCnt ==0)
	)
	{
		BDBG_MSG(("No user capture buffer available! Window %d ", hBuffer->hWindow->eId));
#if 0
		BDBG_ERR(( "current writer ID %d, previous writer ID %d, current reader ID %d, next writer ID %d ",
					hBuffer->pCurWriterBuf->ulBufferId, pPrevWriterNode->ulBufferId,
					hBuffer->pCurReaderBuf->ulBufferId, pTempNode->ulBufferId));

		BDBG_ERR(("   Dump out the whole buffer chain	 "	));
		while(pTempNode != hBuffer->pCurWriterBuf)
		{
			BDBG_ERR(("Buffer Id = %d ", pTempNode->ulBufferId));
			BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextTempNode, pTempNode);
			pTempNode = pNextTempNode;
		}

		BDBG_ASSERT(0);
#endif
		return BVDC_ERR_NO_AVAIL_CAPTURE_BUFFER;
	}


	/* Mark picture node as currently used by user */
	pTempNode->stFlags.bUsedByUser = true;
	pTempNode->stFlags.bActiveNode = false;

	/* need to be set after the gfx surface has been created */
	pTempNode->pSurface = NULL;

	*ppPicture = pTempNode;

	/* Decrement active buffer count. */
	hBuffer->ulActiveBufCnt--;

	/* Toggle the picture node destination polarity pointed
	 * to by the next writer and the nodes after it but before the reader. This
	 * is necessary to keep the field prediction correct.
	 */
	BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pTempNode, hBuffer->pCurWriterBuf);

	while (pTempNode != hBuffer->pCurReaderBuf)
	{
		pTempNode->eDstPolarity = BVDC_P_NEXT_POLARITY(pTempNode->eDstPolarity);
		BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextTempNode, pTempNode);
		pTempNode = pNextTempNode;
	} ;

	return BERR_SUCCESS;
}


/***************************************************************************
 * The main multi-buffering algorithm
 ***************************************************************************/
/***************************************************************************
 * This functions compares ulTimeStamp1 and ulTimeStamp2, and returns the
 * more recent time stamp
 */
#if (BVDC_P_USE_RDC_TIMESTAMP)
static uint32_t BVDC_P_Buffer_GetRecentTimeStamp_isr
	( BVDC_P_Buffer_Handle             hBuffer,
	  uint32_t                         ulPrevTimeStamp,
	  uint32_t                         ulTimeStamp1,
	  uint32_t                         ulTimeStamp2 )
{
	uint32_t    ulRecentTimeStamp;

	/* Check if time stamp warp around */
	if(ulTimeStamp1 < ulPrevTimeStamp)
		ulTimeStamp1 += hBuffer->ulMaxTimestamp;

	if(ulTimeStamp2 < ulPrevTimeStamp)
		ulTimeStamp2 += hBuffer->ulMaxTimestamp;

	if(ulTimeStamp2 > ulTimeStamp1)
		ulRecentTimeStamp = ulTimeStamp2;
	else
		ulRecentTimeStamp = ulTimeStamp1;

	/* Get back real time stamp */
	if(ulRecentTimeStamp > hBuffer->ulMaxTimestamp)
		ulRecentTimeStamp -= hBuffer->ulMaxTimestamp;

	return ulRecentTimeStamp;
}
#endif

/***************************************************************************
 *
 */
static void BVDC_P_Buffer_UpdateWriterTimestamps_isr
	( BVDC_Window_Handle               hWindow,
	  const BAVC_Polarity              eFieldId )
{
	uint32_t ulTimestamp;
#if (BVDC_P_USE_RDC_TIMESTAMP)
	uint32_t           ulOtherSlotTimestamp;
	BRDC_Slot_Handle   hCaptureSlot, hOtherCaptureSlot;
#endif

#if (!BVDC_P_USE_RDC_TIMESTAMP)
	BSTD_UNUSED(eFieldId);
#endif

	BDBG_ENTER(BVDC_P_Buffer_UpdateWriterTimestamps_isr);
	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
	BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

#if (BVDC_P_USE_RDC_TIMESTAMP)
	if(eFieldId == BAVC_Polarity_eFrame)
	{
		if(BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId))
		{
			/* Progressive format use frame slot */
			hCaptureSlot = hWindow->stCurInfo.hSource->ahSlot[BAVC_Polarity_eFrame];
			ulTimestamp = BRDC_Slot_GetTimerSnapshot_isr(hCaptureSlot);
		}
		else
		{
			/* Non-Mpeg sources use 2 slots */
			/* Progressive format use top field slot */
			hCaptureSlot = hWindow->stCurInfo.hSource->ahSlot[BAVC_Polarity_eTopField];
			ulTimestamp = BRDC_Slot_GetTimerSnapshot_isr(hCaptureSlot);
		}
	}
	else
	{
		/* get time stamp from both slots, choose the later one */
		hCaptureSlot = hWindow->stCurInfo.hSource->ahSlot[eFieldId];
		hOtherCaptureSlot = hWindow->stCurInfo.hSource->ahSlot[BVDC_P_NEXT_POLARITY(eFieldId)];

		ulTimestamp = BRDC_Slot_GetTimerSnapshot_isr(hCaptureSlot);
		ulOtherSlotTimestamp = BRDC_Slot_GetTimerSnapshot_isr(hOtherCaptureSlot);

		ulTimestamp = BVDC_P_Buffer_GetRecentTimeStamp_isr(hWindow->hBuffer,
			hWindow->hBuffer->ulPrevWriterTimestamp, ulTimestamp, ulOtherSlotTimestamp);
	}
#else

	ulTimestamp = BREG_Read32(hWindow->stCurResource.hCapture->hRegister, hWindow->stCurResource.hCapture->ulTimestampRegAddr);
	/* convert to microseconds based on a 27MHz clock since direct regsiter reads are tick values */
	ulTimestamp /= BVDC_P_CLOCK_RATE;

#endif

#if (BVDC_BUF_LOG == 1)
	BVDC_P_BufLogAddEvent_isr('Q',
                        hWindow->eId,
                        ulTimestamp,
                        0xAABB, /* Writer time stamp mark */
                        0,
                        hWindow->hBuffer);
#endif

	if (ulTimestamp != hWindow->hBuffer->ulCurrWriterTimestamp)
	{
		uint32_t ulTempCurWriterTs = hWindow->hBuffer->ulCurrWriterTimestamp;

		if (hWindow->hBuffer->bWriterWrapAround)
		{
			ulTempCurWriterTs -= hWindow->hBuffer->ulMaxTimestamp;
			hWindow->hBuffer->bWriterWrapAround = false;
		}

		hWindow->hBuffer->ulPrevWriterTimestamp = ulTempCurWriterTs;
		hWindow->hBuffer->ulCurrWriterTimestamp = ulTimestamp;
	}

	/* Apply correction to wraparound, if necssary. */
	if (hWindow->hBuffer->ulCurrWriterTimestamp < hWindow->hBuffer->ulPrevWriterTimestamp )
	{
		hWindow->hBuffer->ulCurrWriterTimestamp   += hWindow->hBuffer->ulMaxTimestamp;
		hWindow->hBuffer->bWriterWrapAround = true;
	}

	BDBG_LEAVE(BVDC_P_Buffer_UpdateWriterTimestamps_isr);
	return;
}

/***************************************************************************
 *
 */
static void BVDC_P_Buffer_UpdateReaderTimestamps_isr
	( BVDC_Window_Handle               hWindow,
	  const BAVC_Polarity              eFieldId )
{
	uint32_t ulTimestamp;
#if (BVDC_P_USE_RDC_TIMESTAMP)
	uint32_t           ulOtherSlotTimestamp;
	BRDC_Slot_Handle   hPlaybackSlot, hOtherPlaybackSlot;
#endif

#if (!BVDC_P_USE_RDC_TIMESTAMP)
	BSTD_UNUSED(eFieldId);
#endif

	BDBG_ENTER(BVDC_P_Buffer_UpdateReaderTimestamps_isr);
	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
	BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

#if (BVDC_P_USE_RDC_TIMESTAMP)
	/* display uses 2 slots */
	if(eFieldId == BAVC_Polarity_eFrame)
	{
		/* Progressive format use top field slot */
		hPlaybackSlot = hWindow->hCompositor->ahSlot[BAVC_Polarity_eTopField];
		ulTimestamp = BRDC_Slot_GetTimerSnapshot_isr(hPlaybackSlot);
	}
	else
	{
		/* get time stamp from both slots, choose the later one */
		hPlaybackSlot = hWindow->hCompositor->ahSlot[eFieldId];
		hOtherPlaybackSlot = hWindow->hCompositor->ahSlot[BVDC_P_NEXT_POLARITY(eFieldId)];

		ulTimestamp = BRDC_Slot_GetTimerSnapshot_isr(hPlaybackSlot);
		ulOtherSlotTimestamp = BRDC_Slot_GetTimerSnapshot_isr(hOtherPlaybackSlot);

		ulTimestamp = BVDC_P_Buffer_GetRecentTimeStamp_isr(hWindow->hBuffer,
			hWindow->hBuffer->ulPrevReaderTimestamp, ulTimestamp, ulOtherSlotTimestamp);
	}
#else

	ulTimestamp = BREG_Read32(hWindow->stCurResource.hPlayback->hRegister,
		hWindow->stCurResource.hPlayback->ulTimestampRegAddr);

	/* convert to microseconds based on a 27MHz clock since direct regsiter reads are tick values */
	ulTimestamp /= BVDC_P_CLOCK_RATE;
#endif

#if (BVDC_BUF_LOG == 1)
	BVDC_P_BufLogAddEvent_isr('Q',
							hWindow->eId,
							0xBBAA, /* Reader time stamp mark */
							ulTimestamp,
							0,
							hWindow->hBuffer);
#endif

	if (ulTimestamp != hWindow->hBuffer->ulCurrReaderTimestamp)
	{
		uint32_t ulTempCurReaderTs = hWindow->hBuffer->ulCurrReaderTimestamp;

		if (hWindow->hBuffer->bReaderWrapAround)
		{
			ulTempCurReaderTs -= hWindow->hBuffer->ulMaxTimestamp;
			hWindow->hBuffer->bReaderWrapAround = false;
		}

		hWindow->hBuffer->ulPrevReaderTimestamp = ulTempCurReaderTs;
		hWindow->hBuffer->ulCurrReaderTimestamp = ulTimestamp;
	}

	/* Apply correction to wraparound, if necssary. */
	if (hWindow->hBuffer->ulCurrReaderTimestamp < hWindow->hBuffer->ulPrevReaderTimestamp)
	{
		hWindow->hBuffer->ulCurrReaderTimestamp   += hWindow->hBuffer->ulMaxTimestamp;
		hWindow->hBuffer->bReaderWrapAround = true;
	}

	BDBG_LEAVE(BVDC_P_Buffer_UpdateReaderTimestamps_isr);
	return;

}

/***************************************************************************
 * When a pic node is returned by BVDC_P_Buffer_GetNextWriterNode_isr, it is
 * marked as muted for safe. Window_Writer_isr will mark it as un-muted when
 * it is sure that the pic node is valid: the rect ptr is initialized, and
 * the video pixels are really captured.
 */
BVDC_P_PictureNode* BVDC_P_Buffer_GetNextWriterNode_isr
	( BVDC_Window_Handle     hWindow,
	  const BAVC_Polarity    eSrcPolarity,
	  bool                   bMtg,
	  bool                   bMtgRepeat )
{
	BVDC_P_PictureNode      *pNextNode;
	uint32_t                 ulTimeStamp;
	uint32_t                 ulMadOutPhase=1;

	BDBG_ENTER(BVDC_P_Buffer_GetNextWriterNode_isr);
	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
	BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

	if(!hWindow->hBuffer->ulActiveBufCnt)
	{
		hWindow->hBuffer->pCurWriterBuf->stFlags.bMute = true;
		goto done;
	}

	hWindow->hBuffer->ulNumCapField++;

	if (hWindow->stCurInfo.hSource->bMtgSrc)
	{
		if (bMtgRepeat)
		{
			hWindow->hBuffer->ulMtgSrcRepeatCount++;
			hWindow->hBuffer->ulMtgUniquePicCount = 0;
		}
		else
		{
			hWindow->hBuffer->ulMtgUniquePicCount++;
		}

		hWindow->hBuffer->bMtgRepeatMode =
			(hWindow->hBuffer->ulMtgSrcRepeatCount >= 1 && hWindow->hBuffer->ulMtgUniquePicCount <= 1) ? true : false;
	}
	/* ----------------------------------
	 * sync lock case
	 *-----------------------------------*/
	/* Move both reader and writer at same time if sync locked */
	if(hWindow->hBuffer->bSyncLock)
	{
		/* Don't need to check state */
		BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNode, hWindow->hBuffer->pCurWriterBuf);
		pNextNode->stFlags.bMute  = true;
		hWindow->hBuffer->pCurWriterBuf = pNextNode;

		/* Move reader */
		/* if just added lipsync delay, repeat until the delay number is reached */
		if(hWindow->hBuffer->hWindow->bRepeatCurrReader)
		{
			uint32_t ulCurDelay = BVDC_P_Buffer_GetCurrentDelay_isr(hWindow);
			if(ulCurDelay >= hWindow->hBuffer->ulVsyncDelay)
			{
				hWindow->hBuffer->hWindow->bRepeatCurrReader = false;
			}

			/* repeat if current delay less or equal to desired delay */
			if(ulCurDelay <= hWindow->hBuffer->ulVsyncDelay)
			{
				BDBG_MSG(("Win%d current buffer delay = %d, expect %d",
					hWindow->hBuffer->hWindow->eId, ulCurDelay, hWindow->hBuffer->ulVsyncDelay));
				goto done;
			}
		}
		BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNode, hWindow->hBuffer->pCurReaderBuf);
		hWindow->hBuffer->pCurReaderBuf = pNextNode;
		goto done;
	}
	/* forced sync-lock double-buffer works under VEC locking scheme */
	else if(hWindow->stSettings.bForceSyncLock)
	{
		uint32_t ulTimestampDiff;

		/* Note the reduced memory mode (VEC locking) will use timestamp to avoid sticky tearing:
		   1) Every writer/reader isr will update its own ts;
		   2) if writer or reader isr finds that its updated ts is close to the counterpart's, then
		      it means its counterpart was just serviced before itself; in this case, if my next move
		      steps onto its counterpart, then next field/frame will get tearing, so pause my move
		      right here to prevent tearing;
		 */
		/* 1) update writer timestamp */
		BVDC_P_Buffer_UpdateWriterTimestamps_isr(hWindow, eSrcPolarity);

		/* 2) get the delta ts = |w_ts - r_ts|; */
		if(hWindow->hBuffer->ulCurrWriterTimestamp > hWindow->hBuffer->ulCurrReaderTimestamp)
		{
			ulTimestampDiff = hWindow->hBuffer->ulCurrWriterTimestamp - hWindow->hBuffer->ulCurrReaderTimestamp;
		}
		else
		{
			ulTimestampDiff = hWindow->hBuffer->ulCurrReaderTimestamp - hWindow->hBuffer->ulCurrWriterTimestamp;
		}

		BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNode, hWindow->hBuffer->pCurWriterBuf);

		/* If a) delta Ts is small (reader isr was just served for the aligned vsync), and
		      b) writer will step on reader, and
		      c) src/display vsync rates are similar,
		   then don't move writer pointer since it could tear; */
		if( (ulTimestampDiff > BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) ||
			(hWindow->hBuffer->pCurReaderBuf != pNextNode) ||
			!BVDC_P_EQ_DELTA(hWindow->stCurInfo.hSource->ulVertFreq,
			hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq, 50))
		{
			pNextNode->stFlags.bMute  = true;
			hWindow->hBuffer->pCurWriterBuf = pNextNode;
		}
		else
		{
			BDBG_MSG((">>> Pause writer to avoid tearing!"));
		}
#ifdef BVDC_DEBUG_FORCE_SYNC_LOCK
		BDBG_MSG(("Win%d W(%d), R(%d), p(%d), ts_w=%d",
			hWindow->eId, hWindow->hBuffer->pCurWriterBuf->ulBufferId,
			hWindow->hBuffer->pCurReaderBuf->ulBufferId,
			eSrcPolarity, ulTimestampDiff));
#endif
		goto done;
	}

	hWindow->hBuffer->bMtgMadDisplay1To1RateRelationship = (bMtg &&
		((hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq == 24 * BFMT_FREQ_FACTOR)  ||
		 (hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq == 25 * BFMT_FREQ_FACTOR))) ? true : false;

	/* See BVDC_P_Window_Writer_isr after the call to BVDC_P_Buffer_GetNextWriterNode_isr(). This ascertains
	   that the deinterlacer phase set by the Writer ISR is the same phase used by the multibuffer algorithm in
	   determining which MTG-based picture warrants multibuffer processing. */
	if (bMtg)
	{
		ulMadOutPhase = (hWindow->pCurWriterNode->ulMadOutPhase + 1) % 5;
	}

	/* Determine if MTG phase or MTG-based picture repeats warrant multibuffer processing. If not, drop the picture. */
	if ((hWindow->pCurWriterNode->stFlags.bRev32Locked && (ulMadOutPhase != 3 && ulMadOutPhase != 1)) || bMtgRepeat)
	{
		hWindow->hBuffer->pCurWriterBuf = hWindow->pCurWriterNode;
		if (bMtgRepeat)
		{
			BDBG_MSG(("Win[%d]: Drop repeated picture, B[%d]",
					hWindow->eId, hWindow->hBuffer->pCurWriterBuf->ulBufferId));
		}
		else
		{
			BDBG_MSG(("Win[%d]: Use current phase %d, B[%d]",
					hWindow->eId, hWindow->hBuffer->pCurWriterBuf->ulMadOutPhase, hWindow->hBuffer->pCurWriterBuf->ulBufferId));
		}
		goto done;
	}

	/* ----------------------------------
	 * sync slip case
	 *-----------------------------------*/
	/* if src has lost signal, writer vnet will shut down, we should avoid to force
	 * reader's pic node, otherwise reader might start to use a un-initialized pic
	 * node.
	 * if src is set as constant or repeat mode, also can't move reader's pic
	 * node. */
	if((hWindow->stCurInfo.hSource->bStartFeed) &&
	   (BVDC_MuteMode_eDisable == hWindow->stCurInfo.hSource->stCurInfo.eMuteMode))
	{
		BVDC_P_Buffer_CheckWriterIsrOrder_isr(hWindow,
			hWindow->stCurInfo.hSource->eNextFieldId,
			BVDC_P_NEXT_POLARITY(hWindow->hBuffer->pCurReaderBuf->eDisplayPolarity));
	}

	if (!hWindow->hBuffer->bWriterNodeMovedByReader)
		BVDC_P_Buffer_MoveSyncSlipWriterNode_isr(hWindow, eSrcPolarity); /* Update current writer node */
	else
	{
		hWindow->hBuffer->bWriterNodeMovedByReader = false; /* clear the flag */
	}

	/* Update picture timestamp */
	ulTimeStamp = hWindow->hBuffer->ulCurrWriterTimestamp;
	while( ulTimeStamp > hWindow->hBuffer->ulMaxTimestamp )
		ulTimeStamp -= hWindow->hBuffer->ulMaxTimestamp;
	hWindow->hBuffer->pCurWriterBuf->ulCaptureTimestamp = ulTimeStamp;
	/* The delay in this node is now invalid */
	hWindow->hBuffer->pCurWriterBuf->bValidTimeStampDelay = false;

done:

#if (BVDC_BUF_LOG == 1)
	BVDC_P_BufLogAddEvent_isr('E',
		hWindow->eId,
		hWindow->hBuffer->pCurWriterBuf->ulBufferId,
		hWindow->hBuffer->ulNumCapField << 24 | (eSrcPolarity << 16) | (hWindow->hBuffer->pCurWriterBuf->PicComRulInfo.eSrcOrigPolarity << 8) | hWindow->hBuffer->pCurWriterBuf->eDstPolarity,
		hWindow->hBuffer->ulCurrWriterTimestamp,
		hWindow->hBuffer);
#endif

	BDBG_LEAVE(BVDC_P_Buffer_GetNextWriterNode_isr);
	return (hWindow->hBuffer->pCurWriterBuf);
}


/***************************************************************************
 *
 */
static void BVDC_P_Buffer_UpdateTimestamps_isr
	( BVDC_Window_Handle               hWindow,
	  const BAVC_Polarity              eSrcPolarity,
	  const BAVC_Polarity              eDispPolarity )
{
	BDBG_ENTER(BVDC_P_Buffer_UpdateTimestamps_isr);

	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

	BVDC_P_Buffer_UpdateReaderTimestamps_isr(hWindow, eDispPolarity);
	BVDC_P_Buffer_UpdateWriterTimestamps_isr(hWindow, eSrcPolarity);

#if (BVDC_BUF_LOG == 1)
	BVDC_P_BufLogAddEvent_isr('Q',
	    hWindow->eId,
        hWindow->hBuffer->ulCurrWriterTimestamp,
        hWindow->hBuffer->ulCurrReaderTimestamp,
        hWindow->hBuffer->ulNumCapField,
        hWindow->hBuffer);
#endif

	BDBG_LEAVE(BVDC_P_Buffer_UpdateTimestamps_isr);
	return;
}

/***************************************************************************
 *
 */
static void BVDC_P_Buffer_CheckWriterIsrOrder_isr
	( BVDC_Window_Handle               hWindow,
	  const BAVC_Polarity              eSrcPolarity,
	  const BAVC_Polarity              eDispPolarity )
{
	uint32_t ulTimestampDiff;

	BDBG_ENTER(BVDC_P_Buffer_CheckWriterIsrOrder_isr);
	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
	BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

	BVDC_P_Buffer_UpdateTimestamps_isr(hWindow, eSrcPolarity, eDispPolarity);

	/* reader is seemingly ahead of writer so maybe misordered */
#if (BVDC_BUF_LOG == 1)
					BVDC_P_BufLogAddEvent_isr('U',
						hWindow->eId,
						hWindow->stCurResource.hPlayback->ulTimestamp,
						hWindow->hBuffer->ulCurrWriterTimestamp,
						hWindow->hBuffer->ulCurrReaderTimestamp,
						hWindow->hBuffer);
#endif

	if ( hWindow->hBuffer->ulCurrReaderTimestamp <= hWindow->hBuffer->ulCurrWriterTimestamp )
	{
		/* Verify if reader timestamp is equivalent to playback timestamp. If not, then we have a misordered ISR */
		if (hWindow->stCurResource.hPlayback->ulTimestamp != hWindow->hBuffer->ulCurrReaderTimestamp)
		{
			ulTimestampDiff = hWindow->hBuffer->ulCurrWriterTimestamp - hWindow->hBuffer->ulCurrReaderTimestamp;

			/* detemine whether we keep cadence or not */
			if (ulTimestampDiff < BVDC_P_MULTIBUFFER_RW_TOLERANCE) /* keep current cadence */
			{
				if ((hWindow->hBuffer->eLastBuffAction == BVDC_P_Last_Buffer_Action_Writer_Moved) &&
					(hWindow->hBuffer->eWriterVsReaderRateCode == hWindow->hBuffer->eReaderVsWriterRateCode))
				{
					BVDC_P_Buffer_MoveSyncSlipReaderNode_isr(hWindow, eDispPolarity);
				hWindow->hBuffer->bReaderNodeMovedByWriter = true;
#if (BVDC_BUF_LOG == 1)
					BVDC_P_BufLogAddEvent_isr('K',
						hWindow->eId,
						hWindow->hBuffer->ulNumCapField,
						hWindow->hBuffer->ulCurrWriterTimestamp,
						hWindow->hBuffer->ulCurrReaderTimestamp,
						hWindow->hBuffer);
#else
					BDBG_MSG(("(A) Win[%d] W: %d writer ISR moved reader due to misordered ISR",
						hWindow->eId, hWindow->hBuffer->ulNumCapField));
#endif
				}
				else
				{

					BDBG_MSG(("(A) Win[%d] Writer ISR can only move the reader once.", hWindow->eId));
				}
			}
		}
	}



	/* Update Capture ISR timestamp */
	if (hWindow->hBuffer->bWriterWrapAround)
	{
		hWindow->stCurResource.hCapture->ulTimestamp = hWindow->hBuffer->ulCurrWriterTimestamp - hWindow->hBuffer->ulMaxTimestamp;
	}
	else
	{
		hWindow->stCurResource.hCapture->ulTimestamp = hWindow->hBuffer->ulCurrWriterTimestamp;
	}

	BDBG_LEAVE(BVDC_P_Buffer_CheckWriterIsrOrder_isr);
	return;
}

#define BVDC_P_PHASE2_TIME_STAMP_ADJUST  0x208D
static void BVDC_P_Buffer_MoveSyncSlipWriterNode_isr
	( BVDC_Window_Handle               hWindow,
	  const BAVC_Polarity              eSrcPolarity )
{
	BVDC_P_PictureNode         *pNextNode, *pNextNextNode, *pPrevNode;
	uint32_t                    ulSrcVertRate, ulDstVertRate;
	bool                        bSkip;
	uint32_t                    ulGap;
	uint32_t                    ulPrevBufCntNeeded;
	bool                        bProgressivePullDown = false;
	bool                        bBuf50to60Hz;

	BDBG_ENTER(BVDC_P_Buffer_MoveSyncSlipWriterNode_isr);

	BSTD_UNUSED(eSrcPolarity);
	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
	BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

	if(hWindow->hBuffer->ulNumCapField < BVDC_P_BUFFER_NUM_FIELD_CAPTURE_B4_DISPLAY)
		goto done;

	/* keep track of the last buffer buffer action	*/
	hWindow->hBuffer->eLastBuffAction = BVDC_P_Last_Buffer_Action_Writer_Moved;

	/* Get next writer buffer */
	BVDC_P_Buffer_GetPrevActiveAndNotUsedByUserNode(pPrevNode, hWindow->hBuffer->pCurWriterBuf);
	BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNode, hWindow->hBuffer->pCurWriterBuf);

	/* Get writer buffer after next. Needed for progressive mode. */
	BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNextNode, pNextNode);

	/* Determine rate gap */
	BVDC_P_Buffer_CalculateRateGap_isr(hWindow->stCurInfo.hSource->ulVertFreq,
		hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
		&hWindow->hBuffer->eWriterVsReaderRateCode, &hWindow->hBuffer->eReaderVsWriterRateCode);

	/* Set no rate gap for MTG mode without deinterlacer and with picture repeats. */
	if (hWindow->stCurInfo.hSource->bMtgSrc &&
		!BVDC_P_VNET_USED_MAD(hWindow->stVnetMode) &&
		hWindow->hBuffer->bMtgRepeatMode)
	{
		hWindow->hBuffer->eWriterVsReaderRateCode = BVDC_P_WrRate_NotFaster;
		hWindow->hBuffer->eReaderVsWriterRateCode = BVDC_P_WrRate_NotFaster;
	}
	/* determine the timestamp sampling period for game mode */
	if((BVDC_P_ROUND_OFF(hWindow->stCurInfo.hSource->ulVertFreq,
		(BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR) << 1) ==
		BVDC_P_ROUND_OFF(hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
		(BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR))
	{
		/* 1) 1to2 src/disp rate ratio */
		hWindow->hBuffer->ulGameDelaySamplePeriod = 2;
	}
	else if((BVDC_P_ROUND_OFF(hWindow->stCurInfo.hSource->ulVertFreq,
		(BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR) * 5) ==
		BVDC_P_ROUND_OFF(hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
		(BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR) * 2)
	{
		/* 2) 2to5 src/disp rate ratio */
		hWindow->hBuffer->ulGameDelaySamplePeriod = 5;
	}
	else
	{
		/* 3) default */
		hWindow->hBuffer->ulGameDelaySamplePeriod = 1;
	}

	if (hWindow->stCurInfo.uiVsyncDelayOffset)
	{
		/* Calculate gap between reader and writer. This guarantees that the
		 * delay between the writer and the reader will not exceed the
		 * desired delay (vysnc delay + 1).
		 * Note, this decision is before advancing the write pointer! */
		ulGap = BVDC_P_Buffer_GetCurrentDelay_isr(hWindow);
		bSkip = (ulGap > (hWindow->hBuffer->ulVsyncDelay)) ? true : false;
	}
	else
	{
		bSkip = false;
	}

	/* When displaying 1080p24/25/30 source as 1080p48/50/60, we cut the number
	 * of buffer to 3 to save memory. The algorithm allows writer to catch up
	 * reader and both of them point to the same buffer node.
	 *
	 * Note: This may cause video tearing if reader somehow misses interrupts.
	 */
	bProgressivePullDown =  VIDEO_FORMAT_IS_PROGRESSIVE(hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->eVideoFmt) &&
							(hWindow->hBuffer->eWriterVsReaderRateCode == BVDC_P_WrRate_NotFaster) &&
							(hWindow->hBuffer->eReaderVsWriterRateCode == BVDC_P_WrRate_2TimesFaster);

	/* Check if next node is reader OR if we are in full progressive mode. */
	if (((pNextNode == hWindow->hBuffer->pCurReaderBuf) && !bProgressivePullDown) ||
	    (((hWindow->hBuffer->eWriterVsReaderRateCode > BVDC_P_WrRate_NotFaster) ||
	      hWindow->hBuffer->bMtgMadDisplay1To1RateRelationship) &&
	     (pNextNextNode == hWindow->hBuffer->pCurReaderBuf)) ||
	    bSkip)
	{
		hWindow->hBuffer->ulSkipStat++;

		/* Skip one frame if capture as frame, or one field if capture as field but display is progressive and
		 * MAD is not at reader side. Otherwise, skip two fields.
		 * Note: If MAD is at reader side, skipping one field will break the MAD input TBTB cadence. This
		 * will result in MAD falling back to spatial-only mode.
		 */
		if (!((hWindow->bFrameCapture) ||
			  (VIDEO_FORMAT_IS_PROGRESSIVE(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt) &&
			   (!BVDC_P_VNET_USED_MAD_AT_READER(hWindow->stVnetMode)))))
		{
			hWindow->hBuffer->pCurWriterBuf = pPrevNode;

#if (BVDC_BUF_LOG == 1)
            BVDC_P_BufLogAddEvent_isr('A',
                hWindow->eId,
                hWindow->hBuffer->pCurWriterBuf->ulBufferId,
                hWindow->hBuffer->ulNumCapField << 24 | hWindow->hBuffer->ulSkipStat,
                hWindow->hBuffer->ulCurrWriterTimestamp,
                hWindow->hBuffer);
#else
            BDBG_MSG(("Win[%d] W: \tSkip 2 fields, num of cap fields %d, total %d",
                hWindow->eId, hWindow->hBuffer->ulNumCapField, hWindow->hBuffer->ulSkipStat));
#endif
		}
		else
		{

#if (BVDC_BUF_LOG == 1)
            BVDC_P_BufLogAddEvent_isr('B',
                hWindow->eId,
                hWindow->hBuffer->pCurWriterBuf->ulBufferId,
                hWindow->hBuffer->ulNumCapField << 24 | hWindow->hBuffer->ulSkipStat,
                hWindow->hBuffer->ulCurrWriterTimestamp,
                hWindow->hBuffer);
#else
            BDBG_MSG(("Win[%d] W: \tSkip 1 frame/field, num of cap frames/field %d, total %d",
                hWindow->eId, hWindow->hBuffer->ulNumCapField, hWindow->hBuffer->ulSkipStat));
#endif
		}
	}
	else
	{
		hWindow->hBuffer->pCurWriterBuf = pNextNode;
	}

	hWindow->hBuffer->pCurWriterBuf->stFlags.bMute = true;

	ulPrevBufCntNeeded = hWindow->hBuffer->hWindow->ulBufCntNeeded;

	/* Check to see if buffer count was reduced due to progressive display format. If so and
	 * the reader or writer rate gaps is 1, increment the buffer cnt. */
	if ((!hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced || bProgressivePullDown) &&
		!hWindow->hBuffer->bMtgMadDisplay1To1RateRelationship &&
		!(hWindow->stCurInfo.hSource->bMtgSrc && !BVDC_P_VNET_USED_MAD(hWindow->stVnetMode)))
	{
		if ((hWindow->hBuffer->eWriterVsReaderRateCode == hWindow->hBuffer->eReaderVsWriterRateCode) || bProgressivePullDown)
		{
			if (!hWindow->bBufferCntDecremented)
			{
				if (hWindow->bBufferCntIncremented)
				{
					/* From N+1 buffers to the N buffers first */
					hWindow->ulBufCntNeeded --;
					hWindow->ulBufDelay--;
					hWindow->hBuffer->ulVsyncDelay--;

					hWindow->bBufferCntIncremented = false;
				}

				/* From N buffers to N-1 buffers */
				hWindow->ulBufCntNeeded --;
				if (hWindow->hBuffer->eWriterVsReaderRateCode == hWindow->hBuffer->eReaderVsWriterRateCode)
				{
					hWindow->ulBufDelay--;
					hWindow->hBuffer->ulVsyncDelay--;
				}
				else /* Progressive pull down */
				{
					hWindow->bBufferCntDecrementedForPullDown = true;
				}

				hWindow->bBufferCntDecremented = true;
				BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] Decrementing buffer count from %d to %d due progressive display format",
					hWindow->eId, ulPrevBufCntNeeded, hWindow->ulBufCntNeeded));
			}
		}
		else if (hWindow->bBufferCntDecremented)
		{
			hWindow->ulBufCntNeeded ++;

			if (!hWindow->bBufferCntDecrementedForPullDown)
			{
				hWindow->ulBufDelay++;
				hWindow->hBuffer->ulVsyncDelay++;
			}
			else
			{
				hWindow->bBufferCntDecrementedForPullDown = false;
			}
			hWindow->bBufferCntDecremented = false;
			BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] Incrementing buffer count from %d to %d due to rate gap",
				hWindow->eId, ulPrevBufCntNeeded, hWindow->ulBufCntNeeded));
		}
	}

	/* If source/dest relationship requreis a writer gap, capture as interlaced and interlaced display,
	 * increment the number of buffers. */
	ulSrcVertRate = BVDC_P_ROUND_OFF(hWindow->stCurInfo.hSource->ulVertFreq,
		(BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR);
	ulDstVertRate = BVDC_P_ROUND_OFF(hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
		(BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR);

	/* SW7425-4703: roll back version 255 for SW7425-3748 */
	bBuf50to60Hz = (ulSrcVertRate == 50 && ulDstVertRate == 60) ? true : false;
	if ((!hWindow->bFrameCapture) && (!VIDEO_FORMAT_IS_PROGRESSIVE(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt))
		&& ((hWindow->hBuffer->eWriterVsReaderRateCode > BVDC_P_WrRate_NotFaster) ||
			(bBuf50to60Hz && (!BVDC_P_VNET_USED_SCALER_AT_READER(hWindow->stVnetMode)))))
	{
		if (!hWindow->bBufferCntIncremented)
		{
			if (hWindow->bBufferCntDecremented)
			{
				/* From N-1 buffers to N buffers first */
				hWindow->ulBufCntNeeded ++;
				if (!hWindow->bBufferCntDecrementedForPullDown)
				{
					hWindow->ulBufDelay++;
					hWindow->hBuffer->ulVsyncDelay++;
				}
				else
				{
					hWindow->bBufferCntDecrementedForPullDown = false;
				}
				hWindow->bBufferCntDecremented = false;
			}

			/* From N buffers to N+1 buffers */
			hWindow->ulBufCntNeeded++;
			hWindow->ulBufDelay++;
			hWindow->hBuffer->ulVsyncDelay++;
			hWindow->bBufferCntIncremented = true;
			BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] Incrementing buffer count from %d to %d ",
					hWindow->eId, ulPrevBufCntNeeded, hWindow->ulBufCntNeeded));
		}
	}
	else
	{
		if (hWindow->bBufferCntIncremented)
		{
			hWindow->ulBufCntNeeded--;
			hWindow->ulBufDelay--;
			hWindow->hBuffer->ulVsyncDelay--;
			hWindow->bBufferCntIncremented = false;
			BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] Decrementing buffer count from %d to %d ",
				hWindow->eId, ulPrevBufCntNeeded, hWindow->ulBufCntNeeded));
		}
	}

	/* set dirty bit */
	if (hWindow->ulBufCntNeeded != ulPrevBufCntNeeded)
	{
		hWindow->stCurInfo.stDirty.stBits.bReallocBuffers = BVDC_P_DIRTY;
	}

done:

	BDBG_LEAVE(BVDC_P_Buffer_MoveSyncSlipWriterNode_isr);

	return;
}


/***************************************************************************
 * {private}
 *
 */
BVDC_P_PictureNode* BVDC_P_Buffer_GetNextReaderNode_isr
	( BVDC_Window_Handle     hWindow,
	  const BAVC_Polarity    eVecPolarity,
	  bool                   bMtg,
	  bool                   bMtgRepeat )
{
	uint32_t                 ulTimeStamp;

	BDBG_ENTER(BVDC_P_Buffer_GetNextReaderNode_isr);
	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
	BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

	BSTD_UNUSED(bMtg);
	BSTD_UNUSED(bMtgRepeat);

	if(!hWindow->hBuffer->ulActiveBufCnt)
	{
		goto done;
	}

	/* ----------------------------------
	 * sync lock case
	 *-----------------------------------*/
	/* Move both reader and writer at same time if sync locked */
	if(hWindow->hBuffer->bSyncLock)
	{
		goto done;
	}
	/* Forced sync-lock under VEC locking scheme */
	else if(hWindow->stSettings.bForceSyncLock)
	{
		uint32_t ulTimestampDiff;
		BVDC_P_PictureNode         *pNextNode;
		uint32_t ulCurDelay;

		/* lipsync delay is enforced later */
		hWindow->bRepeatCurrReader = false;
		ulCurDelay = BVDC_P_Buffer_GetCurrentDelay_isr(hWindow);

		/* Note the reduced memory mode (VEC locking) will use timestamp to avoid sticky tearing:
		   1) Every writer/reader isr will update its own ts;
		   2) if writer or reader isr finds that its updated ts is close to the counterpart's, then
		      it means its counterpart was just serviced before itself; in this case, if my next move
		      steps onto its counterpart, then next field/frame will get tearing, so pause my move
		      right here to prevent tearing;
		 */
		/* 1) update reader timestamp */
		BVDC_P_Buffer_UpdateReaderTimestamps_isr(hWindow, eVecPolarity);

		/* 2) get the deltaTs between w_ts and r_ts; */
		if(hWindow->hBuffer->ulCurrWriterTimestamp > hWindow->hBuffer->ulCurrReaderTimestamp)
		{
			ulTimestampDiff = hWindow->hBuffer->ulCurrWriterTimestamp - hWindow->hBuffer->ulCurrReaderTimestamp;
		}
		else
		{
			ulTimestampDiff = hWindow->hBuffer->ulCurrReaderTimestamp - hWindow->hBuffer->ulCurrWriterTimestamp;
		}

		/* get next reader */
		BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNode, hWindow->hBuffer->pCurReaderBuf);

		/* If a) delta Ts is small (writer isr was just served for the aligned vsync), and
		      b) reader will step on writer, and
		      c) src/display vsync rates are similar,
		   then don't move reader pointer since it could tear; */
		if( (ulTimestampDiff > BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) ||
			(hWindow->hBuffer->pCurWriterBuf != pNextNode) ||
			!BVDC_P_EQ_DELTA(hWindow->stCurInfo.hSource->ulVertFreq,
			hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq, 50))
		{
			if((/* 1->1 sync-slipped */
				BVDC_P_EQ_DELTA(hWindow->stCurInfo.hSource->ulVertFreq, hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq, 50)
			    ) ||
			   (/* 24->60; 60Hz sync-slipped master */
			   (hWindow->hBuffer->pCurWriterBuf == hWindow->hBuffer->pCurReaderBuf) &&
			   (2397 == hWindow->stCurInfo.hSource->ulVertFreq) &&
			   (5994 == hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) &&
			   (((ulTimestampDiff >= BVDC_P_USEC_ONE_VSYNC_INTERVAL(hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) -
				  BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) &&
				 (ulTimestampDiff <= BVDC_P_USEC_ONE_VSYNC_INTERVAL(hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) +
				  BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE)) ||
			    ((ulTimestampDiff >= 3 * BVDC_P_USEC_ONE_VSYNC_INTERVAL(hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) / 2 -
				  BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) &&
				 (ulTimestampDiff <= 3 * BVDC_P_USEC_ONE_VSYNC_INTERVAL(hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) / 2 +
				  BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE)))) ||
			   (/* 1->2 (25/30->50/60); SD sync-slipped master */
			   (hWindow->hBuffer->pCurWriterBuf == hWindow->hBuffer->pCurReaderBuf) &&
			   (2*hWindow->stCurInfo.hSource->ulVertFreq == hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) &&
			   ((ulTimestampDiff >= BVDC_P_USEC_ONE_VSYNC_INTERVAL(hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) -
				 BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) &&
				(ulTimestampDiff <= BVDC_P_USEC_ONE_VSYNC_INTERVAL(hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq) +
				 BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE))))
			{
				/* make sure lipsync delay enforced in case of missed isr */
				if(((ulTimestampDiff > BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) &&
				    (ulCurDelay >= hWindow->hBuffer->ulVsyncDelay)) ||
				   ((ulTimestampDiff <= BVDC_P_DOUBLE_BUFFER_TS_TOLERANCE) &&
				   (ulCurDelay > hWindow->hBuffer->ulVsyncDelay || ulCurDelay==0)))
				{
					hWindow->hBuffer->pCurReaderBuf = pNextNode;
				}
				else
				{
					BDBG_MSG(("== Pause reader to keep desired lipsync delay! curDly=%d, tgtDly=%d",
						ulCurDelay, hWindow->hBuffer->ulVsyncDelay));
				}
			}
		}
		else
		{
			BDBG_MSG(("|| Pause reader to avoid tearing!"));
		}
#ifdef	BVDC_DEBUG_FORCE_SYNC_LOCK
		BDBG_MSG(("Win%d \tR(%d), W(%d), s(%d)->d(%d)->v(%d), TS [%d, %d] [%d, %d], ts_r=%d, srcFreq=%d, dispFreq=%d",
			hWindow->eId, hWindow->hBuffer->pCurReaderBuf->ulBufferId, hWindow->hBuffer->pCurWriterBuf->ulBufferId,
			hWindow->hBuffer->pCurReaderBuf->eSrcPolarity, hWindow->hBuffer->pCurReaderBuf->eDstPolarity, eVecPolarity,
			hWindow->hCompositor->hDisplay->ulTsSampleCount, hWindow->hCompositor->hDisplay->ulTsSamplePeriod,
			hWindow->hCompositor->hDisplay->ulCurrentTs,
			hWindow->stCurInfo.hSource->hSyncLockCompositor->hDisplay->ulCurrentTs, ulTimestampDiff,
			hWindow->stCurInfo.hSource->ulVertFreq, hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq));
#endif
		goto done;
	}

	/* ----------------------------------
	 * starts sync slip case
	 *-----------------------------------*/
	if(hWindow->stCurInfo.hSource->bStartFeed)
	{
		/* Check if reader ISR is executing out of order */
		BVDC_P_Buffer_CheckReaderIsrOrder_isr(hWindow,
			hWindow->stCurInfo.hSource->eNextFieldId,
			BVDC_P_NEXT_POLARITY(hWindow->hBuffer->pCurReaderBuf->eDisplayPolarity));
	}

	if (!hWindow->hBuffer->bReaderNodeMovedByWriter)
		BVDC_P_Buffer_MoveSyncSlipReaderNode_isr(hWindow, eVecPolarity); /* Update current writer node */
	else
	{
		hWindow->hBuffer->bReaderNodeMovedByWriter = false; /* clear the flag */
	}

	/* Update picture timestamp */
	ulTimeStamp = hWindow->hBuffer->ulCurrReaderTimestamp;
	while( ulTimeStamp > hWindow->hBuffer->ulMaxTimestamp )
		ulTimeStamp -= hWindow->hBuffer->ulMaxTimestamp;
	hWindow->hBuffer->pCurReaderBuf->ulPlaybackTimestamp = ulTimeStamp;
	/* The delay in this node is now valid */
	hWindow->hBuffer->pCurReaderBuf->bValidTimeStampDelay = true;

done:

#if (BVDC_BUF_LOG == 1)
    BVDC_P_BufLogAddEvent_isr('F',
        hWindow->eId,
        hWindow->hBuffer->pCurReaderBuf->ulBufferId,
        hWindow->hBuffer->ulNumCapField << 24 | (eVecPolarity << 16) | (hWindow->hBuffer->pCurReaderBuf->PicComRulInfo.eSrcOrigPolarity << 8) | hWindow->hBuffer->pCurReaderBuf->eDstPolarity,
        hWindow->hBuffer->ulCurrReaderTimestamp,
        hWindow->hBuffer);
#endif

	/* NOTE: The reader and writer should not be pointing to the same node at this point;
	 * otherwise, there will be tearing or unexpected video glitches. The only situation
	 * where R == W is during the execution of 1/2 field delay game mode.
	 */
	BDBG_LEAVE(BVDC_P_Buffer_GetNextReaderNode_isr);
	return (hWindow->hBuffer->pCurReaderBuf);
}

/***************************************************************************
 *
 */
static void BVDC_P_Buffer_CheckReaderIsrOrder_isr
	( BVDC_Window_Handle               hWindow,
	  const BAVC_Polarity              eSrcPolarity,
	  const BAVC_Polarity              eDispPolarity )
{
	uint32_t ulTimestampDiff;

	BDBG_ENTER(BVDC_P_Buffer_CheckReaderIsrOrder_isr);
	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
	BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

	BVDC_P_Buffer_UpdateTimestamps_isr(hWindow, eSrcPolarity, eDispPolarity);

#if (BVDC_BUF_LOG == 1)
					BVDC_P_BufLogAddEvent_isr('V',
						hWindow->eId,
						hWindow->stCurResource.hCapture->ulTimestamp,
						hWindow->hBuffer->ulCurrReaderTimestamp,
						hWindow->hBuffer->ulCurrWriterTimestamp,
						hWindow->hBuffer);
#endif


	/* writer is seemingly ahead of reader so maybe misordered */
	if ( hWindow->hBuffer->ulCurrReaderTimestamp >= hWindow->hBuffer->ulCurrWriterTimestamp )
	{
		/* Verify if writer timestamp is equivalent to capture timestamp. If not, then we have a misordered ISR */
		if (hWindow->stCurResource.hCapture->ulTimestamp != hWindow->hBuffer->ulCurrWriterTimestamp)
		{
			ulTimestampDiff = hWindow->hBuffer->ulCurrReaderTimestamp - hWindow->hBuffer->ulCurrWriterTimestamp;

			/* detemine whether we keep cadence or not */
			if (ulTimestampDiff < BVDC_P_MULTIBUFFER_RW_TOLERANCE) /* keep current cadence */
			{
				if ((hWindow->hBuffer->eLastBuffAction == BVDC_P_Last_Buffer_Action_Reader_Moved) &&
					(hWindow->hBuffer->eWriterVsReaderRateCode == hWindow->hBuffer->eReaderVsWriterRateCode))
				{
					BVDC_P_Buffer_MoveSyncSlipWriterNode_isr(hWindow, eSrcPolarity);
					hWindow->hBuffer->bWriterNodeMovedByReader = true;

#if (BVDC_BUF_LOG == 1)
					BVDC_P_BufLogAddEvent_isr('L',
						hWindow->eId,
						hWindow->hBuffer->ulNumCapField,
						hWindow->hBuffer->ulCurrReaderTimestamp,
						hWindow->hBuffer->ulCurrWriterTimestamp,
						hWindow->hBuffer);
#else
					BDBG_MSG(("(A) Win[%d] R: %d reader ISR moved writer due to ISR misorder", hWindow->eId, hWindow->hBuffer->ulNumCapField));
#endif
				}
				else
				{

					BDBG_MSG(("(A) Win[%d] Reader ISR can only move the writer once.", hWindow->eId));
				}
			}
		}
	}


	/* Update Feeder ISR timestamp */
	if (hWindow->hBuffer->bReaderWrapAround)
	{
		hWindow->stCurResource.hPlayback->ulTimestamp = hWindow->hBuffer->ulCurrReaderTimestamp - hWindow->hBuffer->ulMaxTimestamp;
	}
	else
	{
		hWindow->stCurResource.hPlayback->ulTimestamp = hWindow->hBuffer->ulCurrReaderTimestamp;
	}

	BDBG_LEAVE(BVDC_P_Buffer_CheckReaderIsrOrder_isr);
	return;
}


static void BVDC_P_Buffer_MoveSyncSlipReaderNode_isr
	( BVDC_Window_Handle               hWindow,
	  const BAVC_Polarity              eVecPolarity )
{
	BVDC_P_PictureNode         *pNextNode, *pNextNextNode, *pPrevNode, *pCurNode;
	bool                        bRepeat;
	uint32_t                    ulGap;

	BDBG_ENTER(BVDC_P_Buffer_MoveSyncSlipReaderNode_isr);
	BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
	BDBG_OBJECT_ASSERT(hWindow->hBuffer, BVDC_BUF);

	if(hWindow->hBuffer->ulNumCapField < BVDC_P_BUFFER_NUM_FIELD_CAPTURE_B4_DISPLAY)
		goto done;

	/* keep track of the last buffer buffer action	*/
	hWindow->hBuffer->eLastBuffAction = BVDC_P_Last_Buffer_Action_Reader_Moved;

	/* ----------------------------------
	 * if lipsync delay was adjusted and the expected delay has not been reached;
	 *-----------------------------------*/
	if(hWindow->hBuffer->hWindow->bRepeatCurrReader)
	{
		uint32_t ulCurDelay = BVDC_P_Buffer_GetCurrentDelay_isr(hWindow);
		if(ulCurDelay >= hWindow->hBuffer->ulVsyncDelay)
		{
			hWindow->hBuffer->hWindow->bRepeatCurrReader = false;
		}

		/* repeat if current delay less or equal to desired */
		if(ulCurDelay <= hWindow->hBuffer->ulVsyncDelay)
		{
			BDBG_MSG(("Win%d current buffer delay = %d, expect %d",
				hWindow->hBuffer->hWindow->eId, ulCurDelay, hWindow->hBuffer->ulVsyncDelay));
			goto done;
		}
	}

	/* Get next reader buffer */
	BVDC_P_Buffer_GetPrevActiveAndNotUsedByUserNode(pPrevNode, hWindow->hBuffer->pCurReaderBuf);
	BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNode, hWindow->hBuffer->pCurReaderBuf);

	/* Get reader buffer after next */
	BVDC_P_Buffer_GetNextActiveAndNotUsedByUserNode(pNextNextNode, pNextNode);

	if (hWindow->stCurInfo.uiVsyncDelayOffset)
	{
		/* Calculate gap between reader and writer. This guarantees that the
		 * delay between the writer and the reader will not be less than the
		 * desired delay (vysnc delay - 1).
		 * Note, this decision is before advancing the read pointer! */
		ulGap = BVDC_P_Buffer_GetCurrentDelay_isr(hWindow);
		bRepeat = (ulGap < (hWindow->hBuffer->ulVsyncDelay + (VIDEO_FORMAT_IS_PROGRESSIVE(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt) ? 1: 0))) ?
					true : false;
	}
	else
	{
		bRepeat = false;
	}

	/* Check if we are encroaching the writer */
	if((((pNextNode == hWindow->hBuffer->pCurWriterBuf) &&
		 !hWindow->stCurInfo.stGameDelaySetting.bEnable) ||
	     ((hWindow->hBuffer->eReaderVsWriterRateCode > BVDC_P_WrRate_NotFaster) && (pNextNextNode == hWindow->hBuffer->pCurWriterBuf)) ||
	      bRepeat) && !hWindow->hBuffer->bMtgMadDisplay1To1RateRelationship)
	{
		/* Repeat reader. We may have to bear a field inversion here if both
		 * source and display are interlaced and pictures are not captured as
		 * frame.
		 */
#if BVDC_P_REPEAT_ALGORITHM_ONE
			hWindow->hBuffer->bRepeatForGap = true;
#endif
			hWindow->hBuffer->ulRepeatStat++;
			hWindow->hBuffer->pCurReaderBuf->stFlags.bPictureRepeatFlag = true;

			hWindow->hBuffer->ulMtgDisplayRepeatCount++;

#if (BVDC_BUF_LOG == 1)
            BVDC_P_BufLogAddEvent_isr('C',
                hWindow->eId,
                hWindow->hBuffer->pCurReaderBuf->ulBufferId,
                hWindow->hBuffer->ulNumCapField << 24 | hWindow->hBuffer->ulRepeatStat,
                hWindow->hBuffer->ulCurrReaderTimestamp,
                hWindow->hBuffer);
#else
            BDBG_MSG(("Win[%d], B[%d] R: \t Repeat one field for r-w gap, total %d",
                hWindow->hBuffer->hWindow->eId, hWindow->hBuffer->pCurReaderBuf->ulBufferId,
                hWindow->hBuffer->ulRepeatStat));
#endif
	}
	else
	{
		pCurNode = hWindow->hBuffer->pCurReaderBuf;

		/* Advance the reader anyway first */
		hWindow->hBuffer->pCurReaderBuf = pNextNode;

		/* Now we need to decide if reader's polarity satisfies VEC.
		 */
		if (!hWindow->hBuffer->pCurReaderBuf->stFlags.bMute &&
			hWindow->hBuffer->pCurReaderBuf->stFlags.bCadMatching &&
			(hWindow->hBuffer->pCurReaderBuf->eDstPolarity != eVecPolarity))
		{
#if BVDC_P_REPEAT_ALGORITHM_ONE
			/* If polarity mismatch is caused by a reader repeat to avoid catching up writer, then
			 * move reader to next field anyway to spread 2 repeats over 4 fields.
			 * Bear a field inversion. Reader bvdc_buffer_priv.h for details.
			 */
			if((!hWindow->hBuffer->bRepeatForGap) ||
			   ((pNextNode == hWindow->hBuffer->pCurWriterBuf) && (!hWindow->stCurInfo.stGameDelaySetting.bEnable)) ||
			   ((hWindow->hBuffer->eReaderVsWriterRateCode > BVDC_P_WrRate_NotFaster) && (pNextNextNode == hWindow->hBuffer->pCurWriterBuf)))
#endif
			{
				hWindow->hBuffer->ulRepeatStat++;

#if (BVDC_BUF_LOG == 1)
                BVDC_P_BufLogAddEvent_isr('D',
                    hWindow->eId,
                    hWindow->hBuffer->pCurReaderBuf->ulBufferId,
                    hWindow->hBuffer->ulNumCapField << 24 | (eVecPolarity << 16) | (hWindow->hBuffer->pCurReaderBuf->PicComRulInfo.eSrcOrigPolarity << 8) | hWindow->hBuffer->pCurReaderBuf->eDstPolarity,
                    hWindow->hBuffer->ulCurrReaderTimestamp,
                    hWindow->hBuffer);
#else
                BDBG_MSG(("Win[%d] R: \t Repeat for polarity, src I, VEC %d. orig src p %d, cap %d, total %d",
                    hWindow->hBuffer->hWindow->eId, eVecPolarity, hWindow->hBuffer->pCurReaderBuf->PicComRulInfo.eSrcOrigPolarity,
                    hWindow->hBuffer->pCurReaderBuf->eDstPolarity, hWindow->hBuffer->ulRepeatStat));
#endif

				/* Repeat one field so that next field will match VEC polarity.
				 * We bear a field inversion here.
				 */
				/* restore the current reader buffer */
				hWindow->hBuffer->pCurReaderBuf = pCurNode;
				hWindow->hBuffer->pCurReaderBuf->stFlags.bPictureRepeatFlag = true;
				hWindow->hBuffer->ulMtgDisplayRepeatCount++;
			}
		}

		/* This is a special case for handling MTG 60i-24 sources (3:2 locked by thge deinterlacer) and displayed at 50Hz.  This is needed to prevent
		   a 2:2:2:2:3:1:3 cadence. The expected cadence is 2:2:2:2:2:3:2:2:2:2....
		 */
		if (hWindow->hBuffer->pCurReaderBuf->stFlags.bRev32Locked &&
			hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq == (50 * BFMT_FREQ_FACTOR))
		{
			/* Check if R encroaches W. */
			if (pNextNode == hWindow->hBuffer->pCurWriterBuf)
			{
				/* Restore the current reader buffer. */
				hWindow->hBuffer->pCurReaderBuf = pCurNode;
				hWindow->hBuffer->ulMtgDisplayRepeatCount++;
			}
			else
			{
				/* Check if R has repeated at least once */
				if (hWindow->hBuffer->ulMtgDisplayRepeatCount == 0)
				{
					/* Restore the current reader buffer. This ensures a repeat. */
					hWindow->hBuffer->pCurReaderBuf = pCurNode;
					hWindow->hBuffer->ulMtgDisplayRepeatCount++;
				}
				else
				{
					hWindow->hBuffer->ulMtgDisplayRepeatCount = 0;
				}
			}
		}

#if BVDC_P_REPEAT_ALGORITHM_ONE
		hWindow->hBuffer->bRepeatForGap = false;
#endif
	}

done:
	BDBG_LEAVE(BVDC_P_Buffer_MoveSyncSlipReaderNode_isr);
	return ;
}


/***************************************************************************
 *
 */
void BVDC_P_Buffer_CalculateRateGap_isr
	( const uint32_t         ulSrcVertFreq,
	  const uint32_t         ulDispVertFreq,
	  BVDC_P_WrRateCode     *peWriterVsReaderRateCode,
	  BVDC_P_WrRateCode     *peReaderVsWriterRateCode )
{
	uint32_t	ulDstVertRate, ulSrcVertRate;

	/* 29.97Hz and 30Hz, 59.94Hz and 60Hz are considered
	 * the same rate after round off. */
	ulDstVertRate = BVDC_P_ROUND_OFF(ulDispVertFreq,
		(BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR);
	ulSrcVertRate = BVDC_P_ROUND_OFF(ulSrcVertFreq,
		(BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR);

	/* Preset writer and reader rate gap */
	*peWriterVsReaderRateCode = BVDC_P_WrRate_NotFaster;
	*peReaderVsWriterRateCode = BVDC_P_WrRate_Faster;

	if(ulDstVertRate == ulSrcVertRate)
	{
		*peWriterVsReaderRateCode = BVDC_P_WrRate_NotFaster;
		*peReaderVsWriterRateCode = BVDC_P_WrRate_NotFaster;
	}
	else
	{
		if(ulSrcVertRate > ulDstVertRate)
		{
			*peReaderVsWriterRateCode = BVDC_P_WrRate_NotFaster;
			*peWriterVsReaderRateCode = ((ulSrcVertRate / ulDstVertRate) >= 2)
				? BVDC_P_WrRate_2TimesFaster : BVDC_P_WrRate_Faster;
		}
		else
		{
			*peWriterVsReaderRateCode = BVDC_P_WrRate_NotFaster;
			*peReaderVsWriterRateCode = ((ulDstVertRate / ulSrcVertRate) >= 2)
				? BVDC_P_WrRate_2TimesFaster : BVDC_P_WrRate_Faster;
		}
	}

	return;
}

/***************************************************************************
 *
 */
uint32_t BVDC_P_Buffer_CalculateBufDelay_isr
	( BVDC_P_PictureNode   *pPicture,
	  bool                 *pbValidDelay )
{
	if(pbValidDelay)
		*pbValidDelay = pPicture->bValidTimeStampDelay;

	return ((pPicture->ulPlaybackTimestamp < pPicture->ulCaptureTimestamp) ?
		    (pPicture->ulPlaybackTimestamp + pPicture->hBuffer->ulMaxTimestamp - pPicture->ulCaptureTimestamp)
		    : (pPicture->ulPlaybackTimestamp - pPicture->ulCaptureTimestamp));
}

/***************************************************************************
 *
 */
BVDC_P_PictureNode * BVDC_P_Buffer_GetCurWriterNode_isr
	( const BVDC_P_Buffer_Handle       hBuffer )
{
	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);
	return (hBuffer->pCurWriterBuf);
}

/***************************************************************************
 *
 */
BVDC_P_PictureNode * BVDC_P_Buffer_GetCurReaderNode_isr
	( const BVDC_P_Buffer_Handle       hBuffer )
{
	BDBG_OBJECT_ASSERT(hBuffer, BVDC_BUF);
	return (hBuffer->pCurReaderBuf);
}

/* End of file. */
