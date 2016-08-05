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

#ifndef TZIOC_COMMON_CLIENT_H
#define TZIOC_COMMON_CLIENT_H

#ifndef TZIOC_CLIENT_DEFINES
#define TZIOC_CLIENT_DEFINES

#define TZIOC_CLIENT_NAME_LEN_MAX       32
#define TZIOC_CLIENT_PATH_LEN_MAX       128

#endif /* TZIOC_CLIENT_DEFINES */

#define TZIOC_CLIENT_NUM_MAX            32
#define TZIOC_CLIENT_ID_MAX             255

/* system uses fixed client id of 0 */
#ifndef TZIOC_CLIENT_ID_SYS
#define TZIOC_CLIENT_ID_SYS             0
#endif

/* uappd uses fixed client id of 1 */
#ifndef TZIOC_CLIENT_ID_UAPPD
#define TZIOC_CLIENT_ID_UAPPD           1
#endif

/* msg processing function pointer */
#ifndef TZIOC_MSG_PROC_PFN
#define TZIOC_MSG_PROC_PFN

#include "tzioc_common_msg.h"

typedef int (*tzioc_msg_proc_pfn)(
    tzioc_msg_hdr *pHdr,
    uint32_t ulPrivData);

#endif

/* TZIOC client */
struct tzioc_client {
    char name[TZIOC_CLIENT_NAME_LEN_MAX];
    uint8_t idx;
    uint8_t id;
    bool kernel;
    void *psmem;

    /* kernel client only */
    tzioc_msg_proc_pfn msgProc;
    uint32_t privData;

    /* user client only */
    uint32_t task;
    int msgQ;
};

/* client control block */
struct tzioc_client_cb {
    /* client pinters */
    struct tzioc_client *pClients[TZIOC_CLIENT_NUM_MAX];

    /* last assigned client id */
    uint8_t lastId;
};

#ifdef __cplusplus
extern "C" {
#endif

struct tzioc_client *__tzioc_client_find_by_id(uint8_t id);
struct tzioc_client *__tzioc_client_find_by_name(const char *pName);
struct tzioc_client *__tzioc_client_find_by_task(uint32_t task);
struct tzioc_client *__tzioc_client_find_by_name_and_task(
    const char *pName,
    uint32_t task);

int __tzioc_kernel_client_open(
    struct tzioc_client *pClient,
    const char *pName,
    tzioc_msg_proc_pfn pMsgProc,
    uint32_t ulPrivData);

int __tzioc_kernel_client_close(
    struct tzioc_client *pClient);

int __tzioc_user_client_open(
    struct tzioc_client *pClient,
    const char *pName,
    uint32_t task,
    int msgQ);

int __tzioc_user_client_close(
    struct tzioc_client *pClient);

#ifdef __cplusplus
}
#endif

/* To be initialized by the specific OS */
extern struct tzioc_client_cb *pTziocClientCB;

#endif /* TZIOC_COMMON_CLIENT_H */
