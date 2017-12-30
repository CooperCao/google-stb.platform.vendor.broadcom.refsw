/***************************************************************************
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
        LOGE("Invalid args");
        return 0;
    }

    pClient = _tzioc_client_open(pName);

    if (!pClient) {
        LOGE("Failed to open user client");
        return 0;
    }

    LOGD("client %s, handle %p, msgQ %d, id %d",
         pClient->name, pClient, pClient->msgQ, pClient->id);

    *pMsgQ = (int)pClient->msgQ;
    *pId = pClient->id;
    return (tzioc_client_handle)pClient;
}

void tzioc_client_close(
    tzioc_client_handle hClient)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient) {
        LOGE("Invalid args");
        return;
    }

    LOGD("client %s, handle %p, msgQ %d, id %d",
         pClient->name, pClient, pClient->msgQ, pClient->id);

    _tzioc_client_close(pClient);
}

int tzioc_client_getid(
    tzioc_client_handle hClient,
    const char *pName,
    uint32_t ulPid,
    uint8_t *pId)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient || !pName || !pId) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    return _tzioc_client_getid(
        pClient,
        pName,
        ulPid,
        pId);
}

int tzioc_msg_send(
    tzioc_client_handle hClient,
    tzioc_msg_hdr *pHdr)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient || !pHdr) {
        LOGE("Invalid args");
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
        LOGE("Invalid args");
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
    UNUSED(hClient);
    UNUSED(ulSize);
    return NULL;
}

void tzioc_mem_free(
    tzioc_client_handle hClient,
    void *pBuff)
{
    UNUSED(hClient);
    UNUSED(pBuff);
}

uintptr_t tzioc_offset2vaddr(
    tzioc_client_handle hClient,
    uint32_t ulOffset)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    return _tzioc_offset2vaddr(
        pClient,
        ulOffset);
}

uint32_t tzioc_vaddr2offset(
    tzioc_client_handle hClient,
    uintptr_t ulVaddr)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient || !ulVaddr) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    return _tzioc_vaddr2offset(
        pClient,
        ulVaddr);
}

void *tzioc_map_paddr(
    tzioc_client_handle hClient,
    uintptr_t ulPaddr,
    uint32_t ulSize,
    uint32_t ulFlags)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;
    struct tzioc_mem_region region;

    if (!pClient || ulPaddr == 0 || ulSize == 0) {
        LOGE("Invalid args");
        return NULL;
    }

    /* user-space mapping always no-exec */
    ulFlags |= TZIOC_MEM_NO_EXEC;

    /* use plural version of ioctl cmd */
    region.ulPaddr = ulPaddr;
    region.ulVaddr = 0;
    region.ulSize  = ulSize;
    region.ulFlags = ulFlags;

    if (_tzioc_map_paddrs(pClient, 1, &region))
        /* return NULL in case of error */
        return NULL;

    return (void *)region.ulVaddr;
}

void tzioc_unmap_paddr(
    tzioc_client_handle hClient,
    uintptr_t ulPaddr,
    uint32_t ulSize)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;
    struct tzioc_mem_region region;

    if (!pClient || ulPaddr == 0 || ulSize == 0) {
        LOGE("Invalid args");
        return;
    }

    /* use plural version of ioctl cmd */
    region.ulPaddr = ulPaddr;
    region.ulVaddr = 0;
    region.ulSize  = ulSize;
    region.ulFlags = 0;

    _tzioc_unmap_paddrs(pClient, 1, &region);
}

void tzioc_unmap_vaddr(
    tzioc_client_handle hClient,
    uintptr_t ulVaddr,
    uint32_t ulSize)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;
    struct tzioc_mem_region region;

    if (!pClient || ulVaddr == 0 || ulSize == 0) {
        LOGE("Invalid args");
        return;
    }

    /* use plural version of ioctl cmd */
    region.ulPaddr = 0;
    region.ulVaddr = ulVaddr;
    region.ulSize  = ulSize;
    region.ulFlags = 0;

    _tzioc_unmap_paddrs(pClient, 1, &region);
}

int tzioc_map_paddrs(
    tzioc_client_handle hClient,
    uint8_t ucCount,
    tzioc_mem_region *pRegions)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;
    size_t i;

    if (!pClient || !pRegions ||
        ucCount == 0 || ucCount > TZIOC_MEM_REGION_MAX) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    for (i = 0; i < ucCount; i++) {
        if (pRegions[i].ulPaddr == 0 || pRegions[i].ulSize == 0) {
            LOGE("Invalid args");
            return -EINVAL;
        }

        /* user-space mapping always no-exec */
        pRegions[i].ulFlags |= TZIOC_MEM_NO_EXEC;
    }

    return _tzioc_map_paddrs(
        pClient,
        ucCount,
        pRegions);
}

int tzioc_unmap_paddrs(
    tzioc_client_handle hClient,
    uint8_t ucCount,
    tzioc_mem_region *pRegions)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;
    int idx;

    if (!pClient || !pRegions ||
        ucCount == 0 || ucCount > TZIOC_MEM_REGION_MAX) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    /* use paddrs, zero out vaddrs for backward compatibility */
    for (idx = 0; idx < ucCount; idx++)
        pRegions[idx].ulVaddr = 0;

    return _tzioc_unmap_paddrs(
        pClient,
        ucCount,
        pRegions);
}

int tzioc_unmap_vaddrs(
    tzioc_client_handle hClient,
    uint8_t ucCount,
    tzioc_mem_region *pRegions)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;
    int idx;

    if (!pClient || !pRegions ||
        ucCount == 0 || ucCount > TZIOC_MEM_REGION_MAX) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    /* use vaddrs, zero out paddrs to be safe */
    for (idx = 0; idx < ucCount; idx++)
        pRegions[idx].ulPaddr = 0;

    return _tzioc_unmap_paddrs(
        pClient,
        ucCount,
        pRegions);
}

uintptr_t tzioc_paddr2vaddr(
    tzioc_client_handle hClient,
    uintptr_t ulPaddr)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    return _tzioc_paddr2vaddr(
        pClient,
        ulPaddr);
}

uintptr_t tzioc_vaddr2paddr(
    tzioc_client_handle hClient,
    uintptr_t ulVaddr)
{
    struct tzioc_client *pClient = (struct tzioc_client *)hClient;

    if (!pClient || !ulVaddr) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    return _tzioc_vaddr2paddr(
        pClient,
        ulVaddr);
}

int tzioc_gen_irq(
    tzioc_client_handle hClient,
    uint32_t ulIrq)
{
    UNUSED(hClient);
    UNUSED(ulIrq);
    return 0;
}
