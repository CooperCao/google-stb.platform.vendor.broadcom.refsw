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

#include "tzioc_common_ring.h"
#include "tzioc_common.h"


int __tzioc_ring_init(
    struct tzioc_ring_buf *pRing,
    uintptr_t ulBuffOffset,
    uintptr_t ulBuffSize,
    uint32_t ulFlags,
    tzioc_offset2addr_pfn pOffset2Addr)
{
    if (ulFlags & TZIOC_RING_CREATE) {
        pRing->ulBuffOffset = ulBuffOffset;
        pRing->ulBuffSize   = ulBuffSize;
        pRing->ulRdOffset   = pRing->ulBuffOffset;
        pRing->ulWrOffset   = pRing->ulBuffOffset;
    }

    if (ulFlags & TZIOC_RING_WRITE) {
        pRing->pWrOffset2Addr = pOffset2Addr;
    }
    if (ulFlags & TZIOC_RING_READ) {
        pRing->pRdOffset2Addr = pOffset2Addr;
    }
    return 0;
}

uintptr_t ring_bytes(
    struct tzioc_ring_buf *pRing,
    uintptr_t ulWrOffset,
    uintptr_t ulRdOffset)
{
    if (ulWrOffset >= ulRdOffset) {
        return ulWrOffset - ulRdOffset;
    }
    else {
        return pRing->ulBuffSize - (ulRdOffset - ulWrOffset);
    }
}

uintptr_t ring_space(
    struct tzioc_ring_buf *pRing,
    uintptr_t ulWrOffset,
    uintptr_t ulRdOffset)
{
    /* minimum 1 byte of unwritten space has to be reserved */
    if (ulWrOffset >= ulRdOffset) {
        return pRing->ulBuffSize - (ulWrOffset - ulRdOffset) - 1;
    }
    else {
        return (ulRdOffset - ulWrOffset) - 1;
    }
}

uintptr_t ring_wrap(
    struct tzioc_ring_buf *pRing,
    uintptr_t ulOffset,
    uintptr_t ulInc)
{
    ulOffset += ulInc;
    if (ulOffset >= pRing->ulBuffOffset + pRing->ulBuffSize) {
        ulOffset -= pRing->ulBuffSize;
    }
    return ulOffset;
}

int ring_poke(
    struct tzioc_ring_buf *pRing,
    uintptr_t ulWrOffset,
    uint8_t *pData,
    uintptr_t ulSize)
{
    uintptr_t ulNxSpace;
    if (ring_space(
            pRing,
            ulWrOffset,
            pRing->ulRdOffset) < ulSize) {
        return -ENOSPC;
    }
    ulNxSpace = pRing->ulBuffOffset + pRing->ulBuffSize - ulWrOffset;

    if (ulNxSpace > ulSize) {
        memcpy(
            (void *)pRing->pWrOffset2Addr(ulWrOffset),
            (void *)pData,
            ulSize);
    }
    else {
        memcpy(
            (void *)pRing->pWrOffset2Addr(ulWrOffset),
            (void *)pData,
            ulNxSpace);

        memcpy(
            (void *)pRing->pWrOffset2Addr(pRing->ulBuffOffset),
            (void *)(pData + ulNxSpace),
            ulSize - ulNxSpace);
    }
    return 0;
}

int ring_peek(
    struct tzioc_ring_buf *pRing,
    uintptr_t ulRdOffset,
    uint8_t *pData,
    uintptr_t ulSize)
{
    uint32_t ulNxBytes;

    if (ring_bytes(
            pRing,
            ulRdOffset,
            pRing->ulWrOffset) < ulSize) {
        return -ENOMSG;
    }

    ulNxBytes = pRing->ulBuffOffset + pRing->ulBuffSize - ulRdOffset;
    if (ulNxBytes > ulSize) {
        memcpy(
            (void *)pData,
            (void *)pRing->pRdOffset2Addr(ulRdOffset),
            ulSize);
    }
    else {
        memcpy(
            (void *)pData,
            (void *)pRing->pRdOffset2Addr(ulRdOffset),
            ulNxBytes);

        memcpy(
            (void *)(pData + ulNxBytes),
            (void *)pRing->pRdOffset2Addr(pRing->ulBuffOffset),
            ulSize - ulNxBytes);
    }
    return 0;
}
