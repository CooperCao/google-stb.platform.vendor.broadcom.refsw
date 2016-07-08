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

#include "libtzioc_api.h"
#include "libtzioc.h"

tzioc_client_handle tzioc_client_open(
    const char *pName,
    int *pMsgQ,
    uint8_t *pId)
{
    struct tzioc_client *pClient;

    if (!pName || !pMsgQ || !pId) {
        LOGE("invalid args");
        return 0;
    }

    pClient = _tzioc_client_open(pName);

    if (!pClient) {
        LOGE("failed to open user client");
        return 0;
    }

    LOGD("client %s, handle %p, msgQ %d, id %d",
         pClient->name, (void *)pClient, pClient->msgQ, pClient->id);

    *pMsgQ = (int)pClient->msgQ;
    *pId = pClient->id;
    return (tzioc_client_handle)pClient;
}

void tzioc_client_close(
    tzioc_client_handle hClient)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient) {
        LOGE("invalid args");
        return;
    }

    LOGD("client %s, handle %p, msgQ %d, id %d",
         pClient->name, (void *)pClient, pClient->msgQ, pClient->id);

    _tzioc_client_close(pClient);
}

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

int tzioc_msg_send(
    tzioc_client_handle hClient,
    tzioc_msg_hdr *pHdr)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient || !pHdr) {
        LOGE("invalid args");
        return -EINVAL;
    }

    if (pHdr->ucOrig != pClient->id ||
        pHdr->ulLen > TZIOC_MSG_PAYLOAD_MAX) {
        return -EBADMSG;
    }

    return _tzioc_msg_send(
        pClient,
        pHdr);
}

int tzioc_msg_receive(
    tzioc_client_handle hClient,
    tzioc_msg_hdr *pHdr,
    uint32_t ulSize,
    uint32_t ulTimeout)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient || !pHdr) {
        LOGE("invalid args");
        return -EINVAL;
    }

    return _tzioc_msg_receive(
        pClient,
        pHdr,
        ulSize,
        ulTimeout);
}

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

uint32_t tzioc_offset2vaddr(
    tzioc_client_handle hClient,
    uint32_t ulOffset)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient) {
        LOGE("invalid args");
        return -EINVAL;
    }

    return _tzioc_offset2vaddr(
        pClient,
        ulOffset);
}

uint32_t tzioc_vaddr2offset(
    tzioc_client_handle hClient,
    uint32_t ulVaddr)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient || !ulVaddr) {
        LOGE("invalid args");
        return -EINVAL;
    }

    return _tzioc_vaddr2offset(
        pClient,
        ulVaddr);
}

int tzioc_call_smc(
    tzioc_client_handle hClient,
    uint8_t ucMode)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient) {
        LOGE("invalid args");
        return -EINVAL;
    }

    /* assuming ucMode == SMC callnum */
    return _tzioc_call_smc(
        pClient,
        ucMode);
}
