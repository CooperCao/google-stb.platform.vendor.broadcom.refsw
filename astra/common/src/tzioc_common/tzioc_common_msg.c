/***************************************************************************
 * Copyright (c)2016 Broadcom. All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2 (GPLv2).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 (GPLv2) for more details.
 ***************************************************************************/

#include "tzioc_common.h"
#include "tzioc_common_ring.h"

#ifdef DUMP_MSG_HDR
static inline void tzioc_msg_dump(
    struct tzioc_msg_hdr *pHdr)
{
    LOGV("\ttype: %d", pHdr->ucType);
    LOGV("\torig: %d", pHdr->ucOrig);
    LOGV("\tdest: %d", pHdr->ucDest);
    LOGV("\tseq:  %d", pHdr->ucSeq);
    LOGV("\tlen:  %d", pHdr->ulLen);
}
#endif

int __tzioc_msg_send(
    struct tzioc_msg_hdr *pHdr,
    uint8_t *pPayload)
{
    struct tzioc_ring_buf *pRing = pTziocMsgCB->pSndRing;
    uint32_t ulWrOffset;
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
    uint32_t ulRdOffset;
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
