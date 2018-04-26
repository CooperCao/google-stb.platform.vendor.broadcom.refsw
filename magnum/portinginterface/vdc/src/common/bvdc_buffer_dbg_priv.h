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
 ******************************************************************************/
#ifndef BVDC_BUFFER_DEBUG_PRIV_H__
#define BVDC_BUFFER_DEBUG_PRIV_H__

#include "bvdc_buffer_priv.h"

#if (BVDC_BUF_LOG == 1)
#include "bvdc_dbg.h"

#if BVDC_BUF_DUMP_LOG_TO_FILE
#include "stdio.h"
#endif

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
       = Ex:y    Window x reader repeat 1 field/frame, y:pCurReaderBuf->ulBufferId with possible inversion
       = Fx:y    Window x writer overruns reader which may or may not be allowed, y:pCurWriterBuf->ulBufferId
       = Gx:y    Window x reader overruns writer which may or may not be allowed, y:pCurReaderBuf->ulBufferId
       = Kx:y    Window x new writer IRQ at position y
       = Lx:y    Window x new reader IRQ at position y
     + [...W.R]: Position of reader and writer pointers
     + z:        Number of active buffers
     + iiiiiiii: other info, i.e., number of captured fields, source polarity, vec polarity, etc
     + vvvvvvvv: VDC timestamp

   * uuuuuuuu:bbbb:mx:yyyyyyyy:zzzzz
     + uuuuuuuu: Reference timestamp (microsecond)
     + bbbbb:    Event number index in the circular buffer
     + mx:
       = Ox:     Reader latency on window x
       = Px:     Writer latency on window x
     + yyyyyyyy: Time delta
     + zzzzz:    Number of captured fields so far

   * uuuuuuuu:bbbb:Jx:R=y W=z
     + uuuuuuuu: Reference timestamp (microsecond)
     + bbbbb:    Event number index in the circular buffer
     + Qx:       Invalidate on window x
     + R=y:      Set reader to position y
     + W=z       Set writer to position z

   * uuuuuuuu:bbbb:mx:yyyyy
     + uuuuuuuu: Reference timestamp (microsecond)
     + bbbbb:    Event number index in the circular buffer
     + mx:
       = Sx:     Reader of window x moved by writer due to misordered ISR (type 1)
       = Tx:     Writer of window x moved by reader due to misordered ISR (type 1)
     + yyyyy:    Number of captured fields

   * uuuuuuuu:bbbb:mx:cnt=y
     + uuuuuuuu:  Reference timestamp (microsecond)
     + bbbbb:     Event number index in the circular buffer
     + mx:
       = Ux:cnt=y Added buffer entry on window x to count=y
       = Vx:cnt=y Removed buffer entry on window x to count=y

   * uuuuuuuu:bbbb:Tx:cccccccc:pppppppp
     + uuuuuuuu:  Reference timestamp (microsecond)
     + bbbbb:     Event number index in the circular buffer
     + Tx:        Timestamp update on window x
     + cccccccc:  VDC capture timestamp (microsecond)
     + pppppppp:  VDC playback timestamp (microsecond)

    * uuuuuuuu:bbbb:Yx:cccccccc:pppppppp:rrrrrrrr
     + uuuuuuuu:  Reference timestamp (microsecond)
     + bbbbb:     Event number index in the circular buffer
     + Yx:        Timestamp update on window x
     + cccccccc:  VDC playback timestamp (microsecond)
     + pppppppp:  VDC writer ISR timestamp (microsecond)
     + rrrrrrrr:  VDC reader ISR timestamp (microsecond)

    * uuuuuuuu:bbbb:Zx:cccccccc:pppppppp:rrrrrrrr
     + uuuuuuuu:  Reference timestamp (microsecond)
     + bbbbb:     Event number index in the circular buffer
     + Zx:        Timestamp update on window x
     + cccccccc:  VDC capture timestamp (microsecond)
     + pppppppp:  VDC reader ISR timestamp (microsecond)
     + rrrrrrrr:  VDC writer ISR timestamp (microsecond)
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

/* INFO_1 Fields */
#define BVDC_P_BUF_LOG_DST_POLARITY_SHIFT              0
#define BVDC_P_BUF_LOG_PIC_REPEAT_CNT_SHIFT            0
#define BVDC_P_BUF_LOG_PIC_SKIP_CNT_SHIFT              0
#define BVDC_P_BUF_LOG_ORIG_SRC_POLARITY_SHIFT         8
#define BVDC_P_BUF_LOG_SRC_POLARITY_SHIFT              16
#define BVDC_P_BUF_LOG_VEC_POLARITY_SHIFT              BVDC_P_BUF_LOG_SRC_POLARITY_SHIFT
#define BVDC_P_BUF_LOG_NUM_CAP_FIELDS_SHIFT            24

#define BVDC_P_BUF_LOG_DST_POLARITY                    0x000000FF
#define BVDC_P_BUF_LOG_PIC_REPEAT_CNT                  0x000000FF
#define BVDC_P_BUF_LOG_PIC_SKIP_CNT                    0x000000FF
#define BVDC_P_BUF_LOG_ORIG_SRC_POLARITY               0x0000FF00
#define BVDC_P_BUF_LOG_SRC_POLARITY                    0x00FF0000
#define BVDC_P_BUF_LOG_VEC_POLARITY                    BVDC_P_BUF_LOG_SRC_POLARITY
#define BVDC_P_BUF_LOG_NUM_CAP_FIELDS                  0xFF000000

/* INFO_2 Fields */
#define BVDC_P_BUF_LOG_W_NORMAL_ADVANCE_SHIFT             0
#define BVDC_P_BUF_LOG_SKIP_DUE_TO_WRITER_GAP_SHIFT       1
#define BVDC_P_BUF_LOG_SKIP_DUE_TO_MTG_SHIFT              2
#define BVDC_P_BUF_LOG_W_OVERRAN_R_SHIFT                  3
#define BVDC_P_BUF_LOG_R_NORMAL_ADVANCE_SHIFT             4
#define BVDC_P_BUF_LOG_FORCED_REPEAT_PIC_SHIFT            5
#define BVDC_P_BUF_LOG_FORCED_REPEAT_PIC_COUNT_SHIFT      6
#define BVDC_P_BUF_LOG_R_OVERRAN_W_SHIFT                  7
#define BVDC_P_BUF_LOG_REPEAT_DUE_TO_MTG_ALIGN_SRC_SHIFT  8
#define BVDC_P_BUF_LOG_SKIP_DUE_TO_MTG_ALIGN_SRC_SHIFT    9

#define BVDC_P_BUF_LOG_W_NORMAL_ADVANCE                   0x1
#define BVDC_P_BUF_LOG_SKIP_DUE_TO_WRITER_GAP             0x2
#define BVDC_P_BUF_LOG_SKIP_DUE_TO_MTG                    0x4
#define BVDC_P_BUF_LOG_W_OVERRAN_R                        0x8
#define BVDC_P_BUF_LOG_R_NORMAL_ADVANCE                   0x10
#define BVDC_P_BUF_LOG_FORCED_REPEAT_PIC                  0x20
#define BVDC_P_BUF_LOG_FORCED_REPEAT_COUNT                0x40
#define BVDC_P_BUF_LOG_R_OVERRAN_W                        0x80
#define BVDC_P_BUF_LOG_REPEAT_DUE_TO_MTG_ALIGN_SRC        0x100
#define BVDC_P_BUF_LOG_SKIP_DUE_TO_MTG_ALIGN_SRC          0x200

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
    uint32_t    ulInfo_1;
    uint32_t    ulInfo_2;
    uint32_t    ulInfo_3;
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

typedef struct BVDC_P_BufLogInfo {
    char        cLogType;                       /* See the logging legend above... */
    uint32_t    ulTime;                         /* Timestamp from the free-running timer (microseconds) */
    uint32_t    ulMaxTimestamp;

    union {
        BVDC_P_BufLogSkipRepeat   stSkipRepeat; /* Skip/Repeat event - events {A, B, C, D} */
        BVDC_P_BufLogTimeStamp    stTimeStamp;  /* Latency event */
        BVDC_P_BufLogGeneric      stGeneric;    /* Generic event */
    } BufLogData;

    uint32_t    ulInterruptLatency;
} BVDC_P_BufLogInfo;

typedef struct BVDC_P_BufLog
{
    BVDC_P_BufLogInfo astLogInfo[BVDC_P_BUF_LOG_ENTRIES]; /* A circular buffer containing log info of each buffer activity.*/
    BVDC_P_BufLogIdx stWriterIdx;               /* Writer pointer */
    BVDC_P_BufLogIdx stLastSkipRepeatIdx;       /* The position of the last Skip/Repeat event */
    BVDC_BufLogState eState;                   /* State of the logging */
    BVDC_CallbackFunc_isr pfCallback;           /* User registered callback function */
    void *pvCbParm1;                            /* First user callback parameter */
    int iCbParm2;                               /* Second user callback parameter */
    bool bEnable[BVDC_P_WindowId_eComp0_G0];

#if (!BVDC_P_USE_RDC_TIMESTAMP)
    BTMR_TimerRegisters stTmrRegs;              /* VDC timer registers */
#endif
} BVDC_P_BufLog;

void BVDC_P_BufLog_Init
    ( BVDC_P_BufLog *pBufLog );

void BVDC_P_BufLog_AddEvent_isr
    ( int                        type,
      uint32_t                   v1,
      uint32_t                   v2,
      uint32_t                   v3,
      uint32_t                   v4,
      uint32_t                   v5,
      uint32_t                   v6,
      uint32_t                   v7,
      const BVDC_P_Buffer_Handle hBuffer );

void BVDC_P_BufLog_SetStateAndDumpTrigger
    ( BVDC_P_BufLog               *pBufLog,
      BVDC_BufLogState             eLogState,
      const BVDC_CallbackFunc_isr  pfCallback,
      void                        *pvParm1,
      int                          iParm2 );

void BVDC_P_BufLog_SetManualTrigger
    ( BVDC_P_BufLog *pBufLog );

void BVDC_P_BufLog_EnableBufLog
    ( BVDC_P_BufLog      *pBufLog,
      BVDC_P_WindowId     eId,
      bool                bEnable);

void BVDC_P_BufLog_DumpLog
    ( BVDC_P_BufLog      *pBufLog,
      char               *pLog,
      unsigned int        uiLenToRead,
      unsigned int       *puiReadCount );
#endif /* BVDC_BUF_LOG == 1 */

#endif /* #ifndef BVDC_BUFFER_DEBUG_PRIV_H__*/
/* End of file. */
