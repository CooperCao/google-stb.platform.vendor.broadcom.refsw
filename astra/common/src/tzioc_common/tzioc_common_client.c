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

struct tzioc_client *__tzioc_client_find_by_task(uintptr_t task)
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

struct tzioc_client *__tzioc_client_find_by_name_and_task(
    const char *pName,
    uintptr_t task)
{
    uint8_t idx;

    for (idx = 0; idx < TZIOC_CLIENT_NUM_MAX; idx++) {
        if (pTziocClientCB->pClients[idx]) {
            struct tzioc_client *pClient
                = pTziocClientCB->pClients[idx];

            if (!strncmp(pClient->name, pName, TZIOC_CLIENT_NAME_LEN_MAX) &&
                pClient->task == task)
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
    uintptr_t ulPrivData)
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

    strncpy(pClient->name, pName, TZIOC_CLIENT_NAME_LEN_MAX-1);
    pClient->name[TZIOC_CLIENT_NAME_LEN_MAX-1] = '\0';
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
    uintptr_t task,
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

    strncpy(pClient->name, pName, TZIOC_CLIENT_NAME_LEN_MAX-1);
    pClient->name[TZIOC_CLIENT_NAME_LEN_MAX-1] = '\0';
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
