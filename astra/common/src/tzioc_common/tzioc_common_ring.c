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

#include "tzioc_common_ring.h"

int __tzioc_ring_init(
    struct tzioc_ring_buf *pRing,
    uint32_t ulBuffOffset,
    uint32_t ulBuffSize,
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

uint32_t ring_bytes(
    struct tzioc_ring_buf *pRing,
    uint32_t ulWrOffset,
    uint32_t ulRdOffset)
{
    if (ulWrOffset >= ulRdOffset) {
        return ulWrOffset - ulRdOffset;
    }
    else {
        return pRing->ulBuffSize - (ulRdOffset - ulWrOffset);
    }
}

uint32_t ring_space(
    struct tzioc_ring_buf *pRing,
    uint32_t ulWrOffset,
    uint32_t ulRdOffset)
{
    /* minimum 1 byte of unwritten space has to be reserved */
    if (ulWrOffset >= ulRdOffset) {
        return pRing->ulBuffSize - (ulWrOffset - ulRdOffset) - 1;
    }
    else {
        return (ulRdOffset - ulWrOffset) - 1;
    }
}

uint32_t ring_wrap(
    struct tzioc_ring_buf *pRing,
    uint32_t ulOffset,
    uint32_t ulInc)
{
    ulOffset += ulInc;
    if (ulOffset >= pRing->ulBuffOffset + pRing->ulBuffSize) {
        ulOffset -= pRing->ulBuffSize;
    }
    return ulOffset;
}

int ring_poke(
    struct tzioc_ring_buf *pRing,
    uint32_t ulWrOffset,
    uint8_t *pData,
    uint32_t ulSize)
{
    uint32_t ulNxSpace;
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
    uint32_t ulRdOffset,
    uint8_t *pData,
    uint32_t ulSize)
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
