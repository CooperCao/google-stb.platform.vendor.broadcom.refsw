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

#include "tzioc.h"
#include "tzioc_msg.h"
#include "tzioc_mem.h"
#include "tzioc_client.h"
#include "tzioc_ioctl.h"

#include "kernel.h"
#include "tztask.h"
#include "svcutils.h"
#include "lib_printf.h"
#include "tzioc_ioctls.h"

// Static non-const data from TzIocIoctl class
TzIoc::TzIocIoctl::IoctlHandler TzIoc::TzIocIoctl::handlers[TZIOC_IOCTL_LAST];

// Short-hand to get ioctl offset
#define IOCTL_OFFSET(name)      (TZIOC_IOCTL_##name - TZIOC_IOCTL_FIRST)

void TzIoc::TzIocIoctl::init(void *devTree) {
    UNUSED(devTree);

    memset(handlers, 0, sizeof(handlers));

    handlers[IOCTL_OFFSET(CLIENT_OPEN   )] = clientOpen;
    handlers[IOCTL_OFFSET(CLIENT_CLOSE  )] = clientClose;
    handlers[IOCTL_OFFSET(CLIENT_GETID  )] = clientGetId;
    handlers[IOCTL_OFFSET(MSG_SEND      )] = msgSend;
    handlers[IOCTL_OFFSET(MAP_PADDRS    )] = mapPaddrs;
    handlers[IOCTL_OFFSET(UNMAP_PADDRS  )] = unmapPaddrs;
    handlers[IOCTL_OFFSET(PADDR_TO_VADDR)] = paddr2vaddr;
    handlers[IOCTL_OFFSET(VADDR_TO_PADDR)] = vaddr2paddr;
}

int TzIoc::TzIocIoctl::doIoctl(uint32_t cmd, void *arg) {
    if (handlers[cmd - TZIOC_IOCTL_FIRST]) {
        return handlers[cmd - TZIOC_IOCTL_FIRST](arg);
    }
    else {
        printf("Unsupported TZIOC ioctl cmd 0x%x\n", (unsigned int)cmd);
        return -ENOTTY;
    }
}

int TzIoc::TzIocIoctl::clientOpen(void *arg)
{
    struct tzioc_ioctl_client_open_data clientOpenData;

    SysCalls::copyFromUser(
        (struct tzioc_ioctl_client_open_data *)arg,
        &clientOpenData);

    char *pName = clientOpenData.name;
    int msgQ = (int)clientOpenData.msgQ;

    if (pName[0] == '\0' ||
        msgQ < 0) {
        printf("Invalid args in TZIOC ioctl client open cmd\n");
        return -EINVAL;
    }

    TzTask *pTask = TzTask::current();

    struct tzioc_client *pClient =
        TzIocClient::userClientOpen(
            pName,
            pTask,
            msgQ);

    if (pClient == NULL) {
        LOGE("Failed to open user client");
        clientOpenData.retVal = -EFAULT;
        goto EXIT;
    }

    clientOpenData.hClient   = (uintptr_t)pClient;
    clientOpenData.id        = (uint32_t)pClient->id;
    clientOpenData.smemStart = (uintptr_t) psmem;
    clientOpenData.smemSize  = smemSize;
    clientOpenData.retVal    = 0;

 EXIT:
    SysCalls::copyToUser(
        (struct tzioc_ioctl_client_open_data *)arg,
        &clientOpenData);

    return 0;
}

int TzIoc::TzIocIoctl::clientClose(void *arg)
{
    struct tzioc_ioctl_client_close_data clientCloseData;

    SysCalls::copyFromUser(
        (struct tzioc_ioctl_client_close_data *)arg,
        &clientCloseData);

    struct tzioc_client *pClient =
        (struct tzioc_client *)clientCloseData.hClient;

    if (!pClient) {
        printf("Invalid args in TZIOC ioctl client close cmd\n");
        return -EINVAL;
    }

    TzIocClient::userClientClose(pClient);

    clientCloseData.retVal = 0;

    SysCalls::copyToUser(
        (struct tzioc_ioctl_client_close_data *)arg,
        &clientCloseData);

    return 0;
}

int TzIoc::TzIocIoctl::clientGetId(void *arg)
{
    struct tzioc_ioctl_client_getid_data clientGetIdData;

    SysCalls::copyFromUser(
        (struct tzioc_ioctl_client_getid_data *)arg,
        &clientGetIdData);

    struct tzioc_client *pClient =
        (struct tzioc_client *)clientGetIdData.hClient;

    if (pClient->id != TZIOC_CLIENT_ID_UAPPD) {
        printf("Only user app daemon can issue get id cmd\n");
        return -EPERM;
    }

    char *pName = clientGetIdData.name;
    uint32_t pid = clientGetIdData.pid;

    if (pName[0] == '\0') {
        printf("Invalid args in TZIOC ioctl client get id cmd\n");
        return -EINVAL;
    }

    if (pid == 0) {
        pClient = TzIocClient::clientFindByName(pName);
    }
    else {
        const TzTask *pTask = TzTask::taskFromId(pid);
        if (pTask == NULL) {
            LOGE("Failed to find task id %d", (int)pid);
            clientGetIdData.retVal = -ENOENT;
            goto EXIT;
        }

        pClient = TzIocClient::clientFindByNameAndTask(pName, pTask);
    }

    if (pClient == NULL) {
        LOGE("Failed to find client");
        clientGetIdData.retVal = -ENOENT;
        goto EXIT;
    }

    clientGetIdData.id = (uint32_t)pClient->id;
    clientGetIdData.retVal = 0;

 EXIT:
    SysCalls::copyToUser(
        (struct tzioc_ioctl_client_getid_data *)arg,
        &clientGetIdData);

    return 0;
}

int TzIoc::TzIocIoctl::msgSend(void *arg)
{
    struct tzioc_ioctl_msg_send_data msgSendData;

    SysCalls::copyFromUser(
        (struct tzioc_ioctl_msg_send_data *)arg,
        &msgSendData);

    struct tzioc_client *pClient =
        (struct tzioc_client *)msgSendData.hClient;
    struct tzioc_msg_hdr *pHdr = &msgSendData.hdr;

    if (!pClient ||
        pHdr->ucOrig != pClient->id ||
        pHdr->ulLen > TZIOC_MSG_PAYLOAD_MAX) {
        printf("Invalid args in TZIOC ioctl msg send cmd\n");
        return -EINVAL;
    }

    static uint8_t aucPayload[TZIOC_MSG_PAYLOAD_MAX];
    if (pHdr->ulLen) {
        SysCalls::fromUser(
            (void *)(uintptr_t)msgSendData.payloadAddr,
            aucPayload,
            pHdr->ulLen);
    }

    msgSendData.retVal =
        TzIocMsg::send(
            pClient,
            pHdr,
            aucPayload);

    SysCalls::copyToUser(
        (struct tzioc_ioctl_msg_send_data *)arg,
        &msgSendData);

    return 0;
}

int TzIoc::TzIocIoctl::mapPaddrs(void *arg)
{
    struct tzioc_ioctl_map_paddrs_data mapPaddrsData;

    SysCalls::copyFromUser(
        (struct tzioc_ioctl_map_paddrs_data *)arg,
        &mapPaddrsData);

    struct tzioc_client *pClient =
        (struct tzioc_client *)mapPaddrsData.hClient;
    uint8_t count = mapPaddrsData.count;

    if (!pClient ||
        count == 0 || count > TZIOC_MSG_PAYLOAD_MAX) {
        printf("Invalid args in TZIOC ioctl map paddr cmd\n");
        return -EINVAL;
    }

    static struct tzioc_mem_region regions[TZIOC_MSG_PAYLOAD_MAX];
    SysCalls::fromUser(
        (void *)mapPaddrsData.regionsAddr,
        regions,
        sizeof(struct tzioc_mem_region) * count);

    mapPaddrsData.retVal =
        TzIocMem::mapPaddrs(
            pClient,
            count,
            regions);

    if (!mapPaddrsData.retVal) {
        SysCalls::toUser(
            (void *)mapPaddrsData.regionsAddr,
            regions,
            sizeof(struct tzioc_mem_region) * count);
    }

    SysCalls::copyToUser(
        (struct tzioc_ioctl_map_paddrs_data *)arg,
        &mapPaddrsData);

    return 0;
}

int TzIoc::TzIocIoctl::unmapPaddrs(void *arg)
{
    struct tzioc_ioctl_unmap_paddrs_data unmapPaddrsData;

    SysCalls::copyFromUser(
        (struct tzioc_ioctl_unmap_paddrs_data *)arg,
        &unmapPaddrsData);

    struct tzioc_client *pClient =
        (struct tzioc_client *)unmapPaddrsData.hClient;
    uint8_t count = unmapPaddrsData.count;

    if (!pClient ||
        count == 0 || count > TZIOC_MSG_PAYLOAD_MAX) {
        printf("Invalid args in TZIOC ioctl unmap paddr cmd\n");
        return -EINVAL;
    }

    static struct tzioc_mem_region regions[TZIOC_MSG_PAYLOAD_MAX];
    SysCalls::fromUser(
        (void *)unmapPaddrsData.regionsAddr,
        regions,
        sizeof(struct tzioc_mem_region) * count);

    unmapPaddrsData.retVal =
        TzIocMem::unmapPaddrs(
            pClient,
            count,
            regions);

    SysCalls::copyToUser(
        (struct tzioc_ioctl_unmap_paddrs_data *)arg,
        &unmapPaddrsData);

    return 0;
}

int TzIoc::TzIocIoctl::paddr2vaddr(void *arg)
{
    struct tzioc_ioctl_paddr_to_vaddr_data paddrToVaddrData;

    SysCalls::copyFromUser(
        (struct tzioc_ioctl_paddr_to_vaddr_data *)arg,
        &paddrToVaddrData);

    struct tzioc_client *pClient =
        (struct tzioc_client *)paddrToVaddrData.hClient;

    if (!pClient ||
        paddrToVaddrData.paddr == 0) {
        printf("Invalid args in TZIOC ioctl paddr to vaddr cmd\n");
        return -EINVAL;
    }

    paddrToVaddrData.retVal =
        TzIocMem::paddr2vaddr(
            pClient,
            paddrToVaddrData.paddr,
            &paddrToVaddrData.vaddr);

    SysCalls::copyToUser(
        (struct tzioc_ioctl_paddr_to_vaddr_data *)arg,
        &paddrToVaddrData);

    return 0;
}

int TzIoc::TzIocIoctl::vaddr2paddr(void *arg)
{
    struct tzioc_ioctl_vaddr_to_paddr_data vaddrToPaddrData;

    SysCalls::copyFromUser(
        (struct tzioc_ioctl_vaddr_to_paddr_data *)arg,
        &vaddrToPaddrData);

    struct tzioc_client *pClient =
        (struct tzioc_client *)vaddrToPaddrData.hClient;

    if (!pClient ||
        vaddrToPaddrData.vaddr == 0) {
        printf("Invalid args in TZIOC ioctl vaddr to paddr cmd\n");
        return -EINVAL;
    }

    vaddrToPaddrData.retVal =
        TzIocMem::vaddr2paddr(
            pClient,
            vaddrToPaddrData.vaddr,
            &vaddrToPaddrData.paddr);

    SysCalls::copyToUser(
        (struct tzioc_ioctl_vaddr_to_paddr_data *)arg,
        &vaddrToPaddrData);

    return 0;
}
