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
 *****************************************************************************/

#include "tzioc_common.h"
#include "tzioc_common_ring.h"

//#define DUMP_MSG_HDR

#ifdef DUMP_MSG_HDR
static inline void tzioc_msg_dump(
    struct tzioc_msg_hdr *pHdr)
{
    LOGV("\ttype: %d", pHdr->ucType);
    LOGV("\torig: %d", pHdr->ucOrig);
    LOGV("\tdest: %d", pHdr->ucDest);
    LOGV("\tseq:  %d", pHdr->ucSeq);
    LOGV("\tlen:  %d", (int)pHdr->ulLen);
}
#endif

int __tzioc_msg_send(
    struct tzioc_msg_hdr *pHdr,
    uint8_t *pPayload)
{
    struct tzioc_ring_buf *pRing = pTziocMsgCB->pSndRing;
    uintptr_t ulWrOffset;
    uint32_t ulStCode;
    int err = 0;

    if (pHdr == NULL ||
        pHdr->ulLen > TZIOC_MSG_PAYLOAD_MAX ||
        (pHdr->ulLen != 0 && pPayload == NULL)) {
        return -EINVAL;
    }

    ulWrOffset = pRing->ulWrOffset;

    ulStCode = TZIOC_MSG_START_CODE;
    err = ring_poke(pRing, ulWrOffset, (uint8_t *)&ulStCode, sizeof(ulStCode));
    if (err) return err;
    ulWrOffset = ring_wrap(pRing, ulWrOffset, sizeof(ulStCode));

    err = ring_poke(pRing, ulWrOffset, (uint8_t *)pHdr, sizeof(*pHdr));
    if (err) return err;
    ulWrOffset = ring_wrap(pRing, ulWrOffset, sizeof(*pHdr));

    if (pHdr->ulLen != 0) {
        err = ring_poke(pRing, ulWrOffset, pPayload, pHdr->ulLen);
        if (err) return err;
        ulWrOffset = ring_wrap(pRing, ulWrOffset, pHdr->ulLen);
    }

    pRing->ulWrOffset = ulWrOffset;

#ifdef DUMP_MSG_HDR
    LOGV("TZIOC msg sent:");
    tzioc_msg_dump(pHdr);
#endif
    return 0;
}

int __tzioc_msg_receive(
    struct tzioc_msg_hdr *pHdr,
    uint8_t *pPayload,
    uint32_t ulSize)
{
    struct tzioc_ring_buf *pRing = pTziocMsgCB->pRcvRing;
    uintptr_t ulRdOffset;
    uint32_t ulStCode;
    int err = 0;

    if (pHdr == NULL) {
        return -EINVAL;
    }

    ulRdOffset = pRing->ulRdOffset;

    while (1) {
        err = ring_peek(pRing, ulRdOffset, (uint8_t *)&ulStCode, sizeof(ulStCode));
        if (err) return err;
        if (ulStCode == TZIOC_MSG_START_CODE) break;
        ulRdOffset = ring_wrap(pRing, ulRdOffset, 1);
    }
    ulRdOffset = ring_wrap(pRing, ulRdOffset, sizeof(ulStCode));

    err = ring_peek(pRing, ulRdOffset, (uint8_t *)pHdr, sizeof(*pHdr));
    if (err) return err;
    ulRdOffset = ring_wrap(pRing, ulRdOffset, sizeof(*pHdr));

    if (pHdr->ulLen > TZIOC_MSG_PAYLOAD_MAX) {
        /* update offset to skip msg */
        pRing->ulRdOffset = ulRdOffset;
        return -EINVAL;
    }

    if (pHdr->ulLen != 0) {
        if (pPayload == NULL) {
            if (ulSize != (uint32_t)-1) {
                return -ENOSPC;
            }
            /* continue to skip payload */
        }
        else {
            if (ulSize < pHdr->ulLen) {
                return -ENOSPC;
            }
            /* continue to read payload */
            err = ring_peek(pRing, ulRdOffset, pPayload, pHdr->ulLen);
            if (err) return err;
        }
        ulRdOffset = ring_wrap(pRing, ulRdOffset, pHdr->ulLen);
    }

    pRing->ulRdOffset = ulRdOffset;

#ifdef DUMP_MSG_HDR
    LOGV("TZIOC msg received:");
    tzioc_msg_dump(pHdr);
#endif
    return 0;
}
