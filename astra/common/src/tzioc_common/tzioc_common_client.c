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

struct tzioc_client *__tzioc_client_find_by_id(uint8_t id)
{
    uint8_t idx;

    for (idx = 0; idx < TZIOC_CLIENT_NUM_MAX; idx++) {
        if (pTziocClientCB->pClients[idx]) {
            struct tzioc_client *pClient
                = pTziocClientCB->pClients[idx];

            if (pClient->id == id)
                return pClient;
        }
    }
    return NULL;
}

struct tzioc_client *__tzioc_client_find_by_name(const char *pName)
{
    uint8_t idx;

    for (idx = 0; idx < TZIOC_CLIENT_NUM_MAX; idx++) {
        if (pTziocClientCB->pClients[idx]) {
            struct tzioc_client *pClient
                = pTziocClientCB->pClients[idx];

            if (!strncmp(pClient->name, pName, TZIOC_CLIENT_NAME_LEN_MAX))
                return pClient;
        }
    }
    return NULL;
}

struct tzioc_client *__tzioc_client_find_by_task(uint32_t task)
{
    uint8_t idx;

    for (idx = 0; idx < TZIOC_CLIENT_NUM_MAX; idx++) {
        if (pTziocClientCB->pClients[idx]) {
            struct tzioc_client *pClient
                = pTziocClientCB->pClients[idx];

            if (pClient->task == task)
                return pClient;
        }
    }
    return NULL;
}

static inline uint8_t find_next_idx(void)
{
    uint8_t idx;

    /* find an available client idx */
    for (idx = 0; idx < TZIOC_CLIENT_NUM_MAX; idx++) {
        if (!pTziocClientCB->pClients[idx])
            return idx;
    }
    return TZIOC_CLIENT_NUM_MAX;
}

static inline uint8_t find_next_id(void)
{
    uint8_t id;

    for (id  = pTziocClientCB->lastId + 1;
         id != pTziocClientCB->lastId;
         id++) {
        if (id == TZIOC_CLIENT_ID_MAX)
            id = 1; /* id 0 is reserved */
        if (!__tzioc_client_find_by_id(id))
            return id;
    }
    return pTziocClientCB->lastId;
}

int __tzioc_kernel_client_open(
    struct tzioc_client *pClient,
    const char *pName,
    tzioc_msg_proc_pfn pMsgProc,
    uint32_t ulPrivData)
{
    uint8_t idx, id;

    /* find next available idx */
    if (pClient->idx) {
        /* use provided idx */
        idx = pClient->idx;
    }
    else {
        idx = find_next_idx();
        if (idx == TZIOC_CLIENT_NUM_MAX) {
            LOGE("max num of clients reached");
            return -ENOENT;
        }
    }

    /* find next available id */
    id = find_next_id();
    if (id == pTziocClientCB->lastId) {
        LOGE("no available client id");
        return -EFAULT;
    }

    /* remember this client id */
    pTziocClientCB->lastId = id;

    /* init client */
    memset(pClient, 0, sizeof(*pClient));

    strncpy(pClient->name, pName, TZIOC_CLIENT_NAME_LEN_MAX);
    pClient->idx = idx;
    pClient->id = id;

    /* kernel client only */
    pClient->kernel = true;
    pClient->msgProc = pMsgProc;
    pClient->privData = ulPrivData;

    pTziocClientCB->pClients[idx] = pClient;
    return 0;
}

int __tzioc_kernel_client_close(
    struct tzioc_client *pClient)
{
    uint8_t idx = pClient->idx;

    if (pTziocClientCB->pClients[idx] != pClient) {
        LOGE("client not found");
        return -ENOENT;
    }

    /* kernel client only */
    /* TBD */

    pTziocClientCB->pClients[idx] = NULL;
    return 0;
}

int __tzioc_user_client_open(
    struct tzioc_client *pClient,
    const char *pName,
    uint32_t task,
    int msgQ)
{
    uint8_t idx, id;

    /* find next available idx */
    if (pClient->idx) {
        /* use provided idx */
        idx = pClient->idx;
    }
    else {
        idx = find_next_idx();
        if (idx == TZIOC_CLIENT_NUM_MAX) {
            LOGE("max num of clients reached");
            return -ENOENT;
        }
    }

    /* find next available id */
    id = find_next_id();
    if (id == pTziocClientCB->lastId) {
        LOGE("no available client id");
        return -EFAULT;
    }

    /* remember this client id */
    pTziocClientCB->lastId = id;

    /* init client */
    memset(pClient, 0, sizeof(*pClient));

    strncpy(pClient->name, pName, TZIOC_CLIENT_NAME_LEN_MAX);
    pClient->task = task;
    pClient->msgQ = msgQ;
    pClient->idx = idx;
    pClient->id = id;

    /* user client only */
    pClient->kernel = false;

    pTziocClientCB->pClients[idx] = pClient;
    return 0;
}

int __tzioc_user_client_close(
    struct tzioc_client *pClient)
{
    uint8_t idx = pClient->idx;

    if (pTziocClientCB->pClients[idx] != pClient) {
        LOGE("client not found");
        return -ENOENT;
    }

    /* user client only */
    /* TBD */

    pTziocClientCB->pClients[idx] = NULL;
    return 0;
}
