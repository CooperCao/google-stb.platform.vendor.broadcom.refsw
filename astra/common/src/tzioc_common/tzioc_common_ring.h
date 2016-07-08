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

#ifndef TZIOC_COMMON_RING_H
#define TZIOC_COMMON_RING_H
#if defined(__linux__)

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/printk.h>

#elif defined(__TZOS__)

#include <stdint.h>
#include <stdbool.h>
#include <bits/errno.h>
#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TZIOC_RING_CREATE               (1 << 0)
#define TZIOC_RING_WRITE                (1 << 1)
#define TZIOC_RING_READ                 (1 << 2)

typedef uint32_t (*tzioc_offset2addr_pfn)(uint32_t ulOffset);

/* TZIOC ring buf */
struct tzioc_ring_buf {
    uint32_t ulBuffOffset;              /* buffer starting offset */
    uint32_t ulBuffSize;                /* buffer size in bytes */

    uint32_t ulWrOffset;                /* write offset of next byte to write */
    uint32_t ulRdOffset;                /* read offset of next byte to read */

    tzioc_offset2addr_pfn pWrOffset2Addr; /* write offset2addr func */
    tzioc_offset2addr_pfn pRdOffset2Addr; /* read offset2addr func */
};

int __tzioc_ring_init(
    struct tzioc_ring_buf *pRing,
    uint32_t ulBuffOffset,
    uint32_t ulBuffSize,
    uint32_t ulFlags,
    tzioc_offset2addr_pfn pOffset2Addr);

uint32_t ring_bytes(
    struct tzioc_ring_buf *pRing,
    uint32_t ulWrOffset,
    uint32_t ulRdOffset);

uint32_t ring_space(
    struct tzioc_ring_buf *pRing,
    uint32_t ulWrOffset,
    uint32_t ulRdOffset);

uint32_t ring_wrap(
    struct tzioc_ring_buf *pRing,
    uint32_t ulOffset,
    uint32_t ulInc);

int ring_poke(
    struct tzioc_ring_buf *pRing,
    uint32_t ulWrOffset,
    uint8_t *pData,
    uint32_t ulSize);

int ring_peek(
    struct tzioc_ring_buf *pRing,
    uint32_t ulRdOffset,
    uint8_t *pData,
    uint32_t ulSize);

#ifdef __cplusplus
}
#endif

#endif
