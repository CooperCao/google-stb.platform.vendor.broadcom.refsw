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

#ifndef TZIOC_COMMON_MEM_H
#define TZIOC_COMMON_MEM_H

#define TZIOC_MEM_HEAP_MAX              8
#define TZIOC_MEM_BUFF_MAX              256

#define TZIOC_MEM_PAGE_SIZE             0x1000
#define TZIOC_MEM_PAGE_MASK             0xFFF
#define TZIOC_MEM_PAGE_SHIFT            12

#define TZIOC_MEM_MAGIC_WORD            0x4D454D49 /* "IMEM" in ASCII */

/* mem heap layout */
struct tzioc_mem_heap_layout {
    uint32_t ulBuffPages;
    uint32_t ulBuffCount;
};

/* mem heap */
struct tzioc_mem_heap {
    uint32_t ulTableIdx;                /* heap table idx */
    uint32_t ulHeapOffset;              /* heap offset */
    uint32_t ulBuffSize;                /* buffer size */
    uint32_t ulBuffCount;               /* buffer count */
};

/* mem control block */
struct tzioc_mem_cb {
    struct tzioc_mem_heap heaps[TZIOC_MEM_HEAP_MAX]; /* heaps */
    uint8_t tables[TZIOC_MEM_BUFF_MAX]; /* tables (concatenated) */
};

#ifdef __cplusplus
extern "C" {
#endif

int __tzioc_heaps_init(
    struct tzioc_mem_heap *pHeaps,
    uint8_t *pTables,
    uint32_t ulHeapsOffset,
    uint32_t ulHeapsSize,
    struct tzioc_mem_heap_layout *pHeapsLayout);

int __tzioc_mem_alloc(
    uint8_t ucClientId,
    uint32_t ulSize,
    void **ppBuff);

int __tzioc_mem_free(
    uint8_t ucClientId,
    void *pBuff);

#ifdef __cplusplus
}
#endif

/* To be initialized by the specific OS */
extern struct tzioc_mem_cb *pTziocMemCB;

#endif /* TZIOC_COMMON_MEM_H */
