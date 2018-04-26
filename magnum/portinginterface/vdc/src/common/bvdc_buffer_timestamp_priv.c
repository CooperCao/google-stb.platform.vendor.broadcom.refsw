/******************************************************************************
* Copyright (C) 2018 Broadcom.
* The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to
* the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied),
* right to use, or waiver of any kind with respect to the Software, and
* Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
* THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
* IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization,
* constitutes the valuable trade secrets of Broadcom, and you shall use all
* reasonable efforts to protect the confidentiality thereof, and to use this
* information only in connection with your use of Broadcom integrated circuit
* products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
* "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
* OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
* RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
* IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
* A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
* ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
* THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
* OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
* INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
* RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
* HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
* EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
* WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
* FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* Module Description:
*
***************************************************************************/
#include "bvdc_buffer_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_window_priv.h"

BDBG_MODULE(BVDC_BUF_TS);

/***************************************************************************
 * This functions compares ulTimeStamp1 and ulTimeStamp2, and returns the
 * more recent time stamp
 */
#if (BVDC_P_USE_RDC_TIMESTAMP)
uint32_t BVDC_P_Buffer_GetRecentTimeStamp_isr
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
void BVDC_P_Buffer_UpdateWriterTimestamps_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eFieldId )
{
    uint32_t ulTimestamp;
#if (BVDC_P_USE_RDC_TIMESTAMP)
    uint32_t           ulOtherSlotTimestamp;
    BRDC_Slot_Handle   hCaptureSlot, hOtherCaptureSlot;
#else
    BSTD_UNUSED(eFieldId);
#endif

    BDBG_ENTER(BVDC_P_Buffer_UpdateWriterTimestamps_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

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
void BVDC_P_Buffer_UpdateReaderTimestamps_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eFieldId )
{
    uint32_t ulTimestamp;
#if (BVDC_P_USE_RDC_TIMESTAMP)
    uint32_t           ulOtherSlotTimestamp;
    BRDC_Slot_Handle   hPlaybackSlot, hOtherPlaybackSlot;
#else
    BSTD_UNUSED(eFieldId);
#endif

    BDBG_ENTER(BVDC_P_Buffer_UpdateReaderTimestamps_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

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
 *
 */
void BVDC_P_Buffer_UpdateTimestamps_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eSrcPolarity,
      const BAVC_Polarity              eDispPolarity )
{
    BDBG_ENTER(BVDC_P_Buffer_UpdateTimestamps_isr);

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    BVDC_P_Buffer_UpdateReaderTimestamps_isr(hWindow, eDispPolarity);
    BVDC_P_Buffer_UpdateWriterTimestamps_isr(hWindow, eSrcPolarity);

    BDBG_LEAVE(BVDC_P_Buffer_UpdateTimestamps_isr);
    return;
}
