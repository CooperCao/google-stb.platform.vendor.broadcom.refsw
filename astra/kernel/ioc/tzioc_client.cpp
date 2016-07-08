/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ***************************************************************************/

#include "tzioc_client.h"

#include "lib_string.h"
#include "lib_printf.h"

// Static non-const data from TzIocClient class
spinlock_t TzIoc::TzIocClient::lock;
struct tzioc_client_cb TzIoc::TzIocClient::clientCB;
struct tzioc_client TzIoc::TzIocClient::clients[TZIOC_CLIENT_NUM_MAX];

// Exported client control block to common code
struct tzioc_client_cb *pTziocClientCB;

void TzIoc::TzIocClient::init(void *devTree)
{
    UNUSED(devTree);

    // Init spinlock
    spinlock_init("TzIocClient.lock", &lock);

    // Init client control block
    memset(&clientCB, 0, sizeof(clientCB));
    clientCB.lastId = -1;

    // Export client control block to common code
    pTziocClientCB = &clientCB;

    printf("TzIoc client module initialized\n");
}

struct tzioc_client *TzIoc::TzIocClient::clientFindById(uint8_t id)
{
    struct tzioc_client *pClient;

    spin_lock(&lock);
    pClient = __tzioc_client_find_by_id(id);
    spin_unlock(&lock);

    return pClient;
}

struct tzioc_client *TzIoc::TzIocClient::clientFindByName(const char *pName)
{
    struct tzioc_client *pClient;

    spin_lock(&lock);
    pClient = __tzioc_client_find_by_name(pName);
    spin_unlock(&lock);

    return pClient;
}

struct tzioc_client *TzIoc::TzIocClient::clientFindByTask(const TzTask *pTask)
{
    struct tzioc_client *pClient;

    spin_lock(&lock);
    pClient = __tzioc_client_find_by_task((uint32_t)pTask);
    spin_unlock(&lock);

    return pClient;
}

struct tzioc_client *TzIoc::TzIocClient::kernelClientOpen(
    const char *pName,
    tzioc_msg_proc_pfn pMsgProc,
    uint32_t ulPrivData)
{
    struct tzioc_client *pClient;
    uint8_t idx;

    // Find an available client
    // Use clientCB ptr to indicate availability
    for (idx = 0; idx < TZIOC_CLIENT_NUM_MAX; idx++)
        if (!clientCB.pClients[idx]) break;

    if (idx == TZIOC_CLIENT_NUM_MAX) {
        printf("TzIoc max num of clients reached\n");
        return NULL;
    }

    pClient = &clients[idx];
    memset(pClient, 0, sizeof(*pClient));
    pClient->idx = idx;

    spin_lock(&lock);
    int err = __tzioc_kernel_client_open(
        pClient,
        pName,
        pMsgProc,
        ulPrivData);
    spin_unlock(&lock);

    if (err) {
        printf("TzIoc kernel client open failed\n");
        return NULL;
    }

    return pClient;
}

void TzIoc::TzIocClient::kernelClientClose(
    struct tzioc_client *pClient)
{
    spin_lock(&lock);
    __tzioc_kernel_client_close(pClient);
    spin_unlock(&lock);

    // Free client
    // Nothing to do because clientCB ptr is already reset
}

struct tzioc_client *TzIoc::TzIocClient::userClientOpen(
    const char *pName,
    TzTask *pTask,
    int msgQ)
{
    struct tzioc_client *pClient;
    uint8_t idx;

    // Find an available client
    // Use clientCB ptr to indicate availability
    for (idx = 0; idx < TZIOC_CLIENT_NUM_MAX; idx++)
        if (!clientCB.pClients[idx]) break;

    if (idx == TZIOC_CLIENT_NUM_MAX) {
        printf("TzIoc max num of clients reached\n");
        return NULL;
    }

    pClient = &clients[idx];
    memset(pClient, 0, sizeof(*pClient));
    pClient->idx = idx;

    spin_lock(&lock);
    int err = __tzioc_user_client_open(
        pClient,
        pName,
        (uint32_t)pTask,
        msgQ);
    spin_unlock(&lock);

    if (err) {
        printf("TzIoc user client open failed\n");
        return NULL;
    }

    return pClient;
}

void TzIoc::TzIocClient::userClientClose(
    struct tzioc_client *pClient)
{
    spin_lock(&lock);
    __tzioc_user_client_close(pClient);
    spin_unlock(&lock);

    // Free client
    // Nothing to do because clientCB ptr is already reset
}
