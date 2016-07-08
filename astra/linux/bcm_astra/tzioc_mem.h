/***************************************************************************
 * Copyright (c)2016 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 (GPLv2) for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 (GPLv2) along with this source code.
 ***************************************************************************/

#ifndef TZIOC_MEM_H
#define TZIOC_MEM_H

#include <linux/types.h>
#include <linux/spinlock.h>

#include "tzioc_common.h"

/* mem module */
struct tzioc_mem_module {
    /* spinlock */
    spinlock_t lock;

    /* mem control block */
    struct tzioc_mem_cb memCB;
};

int __init _tzioc_mem_module_init(void);
int _tzioc_mem_module_deinit(void);

void *_tzioc_mem_alloc(
    struct tzioc_client *pClient,
    uint32_t ulSize);

void _tzioc_mem_free(
    struct tzioc_client *pClient,
    void *pBuff);

#endif /* TZIOC_MEM_H */
