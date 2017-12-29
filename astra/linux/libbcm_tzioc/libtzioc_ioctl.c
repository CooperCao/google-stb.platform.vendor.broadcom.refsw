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

#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <tzioc_ioctls.h>

#include "libtzioc.h"

int _tzioc_ioctl_client_open(
    struct tzioc_client *pClient)
{
    struct tzioc_ioctl_client_open_data clientOpenData;
    int err = 0;

    strncpy(clientOpenData.name, pClient->name, TZIOC_CLIENT_NAME_LEN_MAX);
    clientOpenData.msgQ = (uint32_t)pClient->msgQ;

    err = ioctl(
        pClient->fd,
        TZIOC_IOCTL_CLIENT_OPEN,
        (void *)&clientOpenData);

    if (err)
        return -EIO;

    if (clientOpenData.retVal)
        return clientOpenData.retVal;

    pClient->hKlient   = clientOpenData.hClient;
    pClient->id        = clientOpenData.id;
    pClient->smemStart = clientOpenData.smemStart;
    pClient->smemSize  = clientOpenData.smemSize;

    return 0;
}

int _tzioc_ioctl_client_close(
    struct tzioc_client *pClient)
{
    struct tzioc_ioctl_client_close_data clientCloseData;
    int err = 0;

    clientCloseData.hClient = pClient->hKlient;

    err = ioctl(
        pClient->fd,
        TZIOC_IOCTL_CLIENT_CLOSE,
        (void *)&clientCloseData);

    if (err)
        return -EIO;

    if (clientCloseData.retVal)
        return clientCloseData.retVal;

    return 0;
}

int _tzioc_ioctl_msg_send(
    struct tzioc_client *pClient,
    struct tzioc_msg_hdr *pHdr)
{
    struct tzioc_ioctl_msg_send_data msgSendData;
    int err = 0;

    msgSendData.hClient = pClient->hKlient;
    memcpy(&msgSendData.hdr, pHdr, sizeof(*pHdr));
    msgSendData.payloadAddr = (uint32_t)TZIOC_MSG_PAYLOAD(pHdr);

    err = ioctl(
        pClient->fd,
        TZIOC_IOCTL_MSG_SEND,
        (void *)&msgSendData);

    if (err)
        return -EIO;

    if (msgSendData.retVal)
        return msgSendData.retVal;

    return 0;
}

int _tzioc_ioctl_mem_alloc(
    struct tzioc_client *pClient,
    uint32_t ulSize,
    uint32_t *pBuffOffset)
{
    struct tzioc_ioctl_mem_alloc_data memAllocData;
    int err = 0;

    memAllocData.hClient = pClient->hKlient;
    memAllocData.size = ulSize;

    err = ioctl(
        pClient->fd,
        TZIOC_IOCTL_MEM_ALLOC,
        (void *)&memAllocData);

    if (err)
        return -EIO;

    if (memAllocData.retVal)
        return memAllocData.retVal;

    *pBuffOffset = memAllocData.buffOffset;
    return 0;
}

int _tzioc_ioctl_mem_free(
    struct tzioc_client *pClient,
    uint32_t ulBuffOffset)
{
    struct tzioc_ioctl_mem_free_data memFreeData;
    int err = 0;

    memFreeData.hClient = pClient->hKlient;
    memFreeData.buffOffset = ulBuffOffset;

    err = ioctl(
        pClient->fd,
        TZIOC_IOCTL_MEM_FREE,
        (void *)&memFreeData);

    if (err)
        return -EIO;

    if (memFreeData.retVal)
        return memFreeData.retVal;

    return 0;
}

int _tzioc_ioctl_call_smc(
    struct tzioc_client *pClient,
    uint32_t ulCallnum)
{
    struct tzioc_ioctl_call_smc_data callSmcData;
    int err = 0;

    callSmcData.hClient = pClient->hKlient;
    callSmcData.callnum = ulCallnum;

    err = ioctl(
        pClient->fd,
        TZIOC_IOCTL_CALL_SMC,
        (void *)&callSmcData);

    if (err)
        return -EIO;

    if (callSmcData.retVal)
        return callSmcData.retVal;

    return 0;
}
