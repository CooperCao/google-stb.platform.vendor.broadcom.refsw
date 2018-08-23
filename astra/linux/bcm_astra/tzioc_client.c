/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include <linux/slab.h>

#include "tzioc_drv.h"
#include "tzioc_client.h"

/* static pointer to client module */
static struct tzioc_client_module *pClientMod;
static struct tzioc_client_cb *pClientCB;

/* exported client control block to common code */
struct tzioc_client_cb *pTziocClientCB;

int _tzioc_client_module_init(void)
{
    /* alloc client module */
    pClientMod = kzalloc(sizeof(struct tzioc_client_module), GFP_KERNEL);
    if (!pClientMod) {
        LOGE("Failed to alloc TZIOC client module");
        return -ENOMEM;
    }

    /* remember client module in TZIOC device */
    tdev->pClientMod = pClientMod;

    /* init spinlock */
    spin_lock_init(&pClientMod->lock);

    /* init client control block */
    pClientCB = &pClientMod->clientCB;
    pClientCB->lastId = -1;

    /* export client control block to common code */
    pTziocClientCB = pClientCB;

    LOGI("TZIOC client module initialized");
    return 0;
}

int _tzioc_client_module_deinit(void)
{
    uint8_t idx;

    /* close all clients */
    for (idx = 0; idx < TZIOC_CLIENT_NUM_MAX; idx++) {
        if (pClientCB->pClients[idx]) {
            struct tzioc_client *pClient = pClientCB->pClients[idx];

            if (pClient->kernel)
                _tzioc_kernel_client_close(pClient);
            else
                _tzioc_user_client_close(pClient);
        }
    }

    /* reset exported client control block to common code */
    pTziocClientCB = NULL;

    /* deinit client control block */
    pClientCB = NULL;

    /* reset client module in TZIOC device */
    tdev->pClientMod = NULL;

    /* free client module control block */
    kfree(pClientMod);

    LOGI("TZIOC client module deinitialized");
    return 0;
}

struct tzioc_client *_tzioc_client_find_by_id(uint8_t id)
{
    struct tzioc_client *pClient;

    spin_lock(&pClientMod->lock);
    pClient = __tzioc_client_find_by_id(id);
    spin_unlock(&pClientMod->lock);

    return pClient;
}

struct tzioc_client *_tzioc_client_find_by_name(const char *pName)
{
    struct tzioc_client *pClient;

    spin_lock(&pClientMod->lock);
    pClient = __tzioc_client_find_by_name(pName);
    spin_unlock(&pClientMod->lock);

    return pClient;
}

struct tzioc_client *_tzioc_client_find_by_task(const struct task_struct *pTask)
{
    struct tzioc_client *pClient;

    spin_lock(&pClientMod->lock);
    pClient = __tzioc_client_find_by_task((uintptr_t)pTask);
    spin_unlock(&pClientMod->lock);

    return pClient;
}

struct tzioc_client *_tzioc_kernel_client_open(
    const char *pName,
    tzioc_msg_proc_pfn pMsgProc,
    uintptr_t ulPrivData)
{
    struct tzioc_client *pClient;
    int err = 0;

    /* alloc client */
    pClient = kzalloc(sizeof(struct tzioc_client), GFP_KERNEL);
    if (!pClient) {
        LOGE("Failed to alloc TZIOC client");
        return NULL;
    }

    spin_lock(&pClientMod->lock);
    err = __tzioc_kernel_client_open(
        pClient,
        pName,
        pMsgProc,
        ulPrivData);
    spin_unlock(&pClientMod->lock);

    if (err) {
        LOGE("Failed to open kernel client %s", pName);
        kfree(pClient);
        return NULL;
    }

    return pClient;
}

void _tzioc_kernel_client_close(
    struct tzioc_client *pClient)
{
    spin_lock(&pClientMod->lock);
    __tzioc_kernel_client_close(pClient);
    spin_unlock(&pClientMod->lock);

    /* free client */
    kfree(pClient);
}

struct tzioc_client *_tzioc_user_client_open(
    const char *pName,
    struct task_struct *pTask,
    int msgQ)
{
    struct tzioc_client *pClient;
    int err = 0;

    /* alloc client */
    pClient = kzalloc(sizeof(struct tzioc_client), GFP_KERNEL);
    if (!pClient) {
        LOGE("Failed to alloc TZIOC client");
        return NULL;
    }

    spin_lock(&pClientMod->lock);
    err = __tzioc_user_client_open(
        pClient,
        pName,
        (uintptr_t)pTask,
        msgQ);
    spin_unlock(&pClientMod->lock);

    if (err) {
        LOGE("Failed to open user client %s", pName);
        kfree(pClient);
        return NULL;
    }

    return pClient;
}

void _tzioc_user_client_close(
    struct tzioc_client *pClient)
{
    spin_lock(&pClientMod->lock);
    __tzioc_user_client_close(pClient);
    spin_unlock(&pClientMod->lock);

    /* free client */
    kfree(pClient);
}
