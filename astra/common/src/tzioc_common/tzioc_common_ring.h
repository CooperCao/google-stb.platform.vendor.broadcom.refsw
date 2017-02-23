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

typedef uintptr_t (*tzioc_offset2addr_pfn)(uintptr_t ulOffset);

/* TZIOC ring buf */

struct tzioc_ring_buf {
    uint64_t ulBuffOffset;              /* buffer starting offset */
    uint64_t ulBuffSize;                /* buffer size in bytes */

    uint64_t ulWrOffset;                /* write offset of next byte to write */
    uint64_t ulRdOffset;                /* read offset of next byte to read */

    tzioc_offset2addr_pfn pWrOffset2Addr; /* write offset2addr func */
#ifdef __arm__
    uint32_t pad_1;
#endif
    tzioc_offset2addr_pfn pRdOffset2Addr; /* read offset2addr func */
#ifdef __arm__
    uint32_t pad_2;
#endif
};

int __tzioc_ring_init(
    struct tzioc_ring_buf *pRing,
    uintptr_t ulBuffOffset,
    uintptr_t ulBuffSize,
    uint32_t ulFlags,
    tzioc_offset2addr_pfn pOffset2Addr);

uintptr_t ring_bytes(
    struct tzioc_ring_buf *pRing,
    uintptr_t ulWrOffset,
    uintptr_t ulRdOffset);

uintptr_t ring_space(
    struct tzioc_ring_buf *pRing,
    uintptr_t ulWrOffset,
    uintptr_t ulRdOffset);

uintptr_t ring_wrap(
    struct tzioc_ring_buf *pRing,
    uintptr_t ulOffset,
    uintptr_t ulInc);

int ring_poke(
    struct tzioc_ring_buf *pRing,
    uintptr_t ulWrOffset,
    uint8_t *pData,
    uintptr_t ulSize);

int ring_peek(
    struct tzioc_ring_buf *pRing,
    uintptr_t ulRdOffset,
    uint8_t *pData,
    uintptr_t ulSize);

#ifdef __cplusplus
}
#endif

#endif
