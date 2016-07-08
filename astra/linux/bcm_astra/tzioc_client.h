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

#ifndef TZIOC_CLIENT_H
#define TZIOC_CLIENT_H

#include <linux/types.h>
#include <linux/spinlock.h>

#include "tzioc_common.h"

/* client module */
struct tzioc_client_module {
    /* spinlock */
    spinlock_t lock;

    /* client control block */
    struct tzioc_client_cb clientCB;
};

int __init _tzioc_client_module_init(void);
int _tzioc_client_module_deinit(void);

struct tzioc_client *_tzioc_client_find_by_id(uint8_t id);
struct tzioc_client *_tzioc_client_find_by_name(const char *pName);
struct tzioc_client *_tzioc_client_find_by_task(const struct task_struct *pTask);

struct tzioc_client *_tzioc_kernel_client_open(
    const char *pName,
    tzioc_msg_proc_pfn pMsgProc,
    uint32_t ulPrivData);

void _tzioc_kernel_client_close(
    struct tzioc_client *pClient);

struct tzioc_client *_tzioc_user_client_open(
    const char *pName,
    struct task_struct *pTask,
    int msgQ);

void _tzioc_user_client_close(
    struct tzioc_client *pClient);

#endif /* TZIOC_CLIENT_H */
