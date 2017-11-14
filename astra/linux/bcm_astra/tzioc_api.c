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

#include <linux/slab.h>

#include "tzioc_api.h"
#include "tzioc_drv.h"
#include "tzioc_msg.h"
#include "tzioc_mem.h"
#include "tzioc_client.h"
#include "tzioc_peer.h"

tzioc_client_handle tzioc_client_open(
    const char *pName,
    tzioc_msg_proc_pfn pMsgProc,
    uintptr_t ulPrivData,
    uint8_t *pId)
{
    struct tzioc_client *pClient;

    if (!pName || !pMsgProc || !pId) {
        LOGE("Invalid args");
        return 0;
    }

    pClient = _tzioc_kernel_client_open(
        pName,
        pMsgProc,
        ulPrivData);

    if (!pClient) {
        LOGE("Failed to open kernel client");
        return 0;
    }

    LOGD("client %s, handle %p, id %d",
         pClient->name, pClient, pClient->id);

    *pId = pClient->id;
    return (tzioc_client_handle)pClient;
}
EXPORT_SYMBOL(tzioc_client_open);

void tzioc_client_close(
    tzioc_client_handle hClient)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient) {
        LOGE("Invalid args");
        return;
    }

    LOGD("client %s, handle %p, id %d",
         pClient->name, pClient, pClient->id);

    _tzioc_kernel_client_close(pClient);
}
EXPORT_SYMBOL(tzioc_client_close);

int tzioc_peer_start(
    tzioc_client_handle hClient,
    const char *pPeerName,
    const char *pPeerExec,
    bool bPeerShared)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;
    int err = 0;

    if (!pClient || !pPeerName || !pPeerExec) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    LOGD("client %s, peer %s",
         pClient->name, pPeerName);

    err = _tzioc_peer_start(
        pClient,
        pPeerName,
        pPeerExec,
        bPeerShared);

    if (err) {
        LOGE("Failed to start peer");
        return err;
    }

    return 0;
}
EXPORT_SYMBOL(tzioc_peer_start);

int tzioc_peer_stop(
    tzioc_client_handle hClient,
    const char *pPeerName)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;
    int err = 0;

    if (!pClient || !pPeerName) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    LOGD("client %s, peer %s",
         pClient->name, pPeerName);

    err = _tzioc_peer_stop(
        pClient,
        pPeerName);

    if (err) {
        LOGE("Failed to stop peer");
        return err;
    }

    return 0;
}
EXPORT_SYMBOL(tzioc_peer_stop);

int tzioc_peer_getid(
    tzioc_client_handle hClient,
    const char *pPeerName)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;
    int err = 0;

    if (!pClient || !pPeerName) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    LOGD("client %s, peer %s",
         pClient->name, pPeerName);

    err = _tzioc_peer_getid(
        pClient,
        pPeerName);

    if (err) {
        LOGE("Failed to get peer id");
        return err;
    }

    return 0;
}
EXPORT_SYMBOL(tzioc_peer_getid);

int tzioc_msg_send(
    tzioc_client_handle hClient,
    tzioc_msg_hdr *pHdr)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient || !pHdr ||
        pClient->id != pHdr->ucOrig) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    return _tzioc_msg_send(
        pClient,
        pHdr,
        TZIOC_MSG_PAYLOAD(pHdr));
}
EXPORT_SYMBOL(tzioc_msg_send);

void *tzioc_mem_alloc(
    tzioc_client_handle hClient,
    uint32_t ulSize)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient || !ulSize) {
        LOGE("Invalid args");
        return NULL;
    }

    return _tzioc_mem_alloc(
        pClient,
        ulSize);
}
EXPORT_SYMBOL(tzioc_mem_alloc);

void tzioc_mem_free(
    tzioc_client_handle hClient,
    void *pBuff)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient || !pBuff) {
        LOGE("Invalid args");
        return;
    }

    _tzioc_mem_free(
        pClient,
        pBuff);
}
EXPORT_SYMBOL(tzioc_mem_free);

uintptr_t tzioc_offset2vaddr(
    tzioc_client_handle hClient,
    uint32_t ulOffset)
{
    UNUSED(hClient);

    /* kernel space */
    return _tzioc_offset2addr(ulOffset);
}
EXPORT_SYMBOL(tzioc_offset2vaddr);

uint32_t tzioc_vaddr2offset(
    tzioc_client_handle hClient,
    uintptr_t ulVaddr)
{
    UNUSED(hClient);

    /* kernel space */
    return _tzioc_addr2offset(ulVaddr);
}
EXPORT_SYMBOL(tzioc_vaddr2offset);

int tzioc_call_smc(
    tzioc_client_handle hClient,
    uint32_t ulCode)
{
    UNUSED(hClient);

    /* assuming ucCode == SMC callnum */
    return _tzioc_call_smc(ulCode);
}
EXPORT_SYMBOL(tzioc_call_smc);
