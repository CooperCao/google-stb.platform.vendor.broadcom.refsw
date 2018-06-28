/***************************************************************************
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
 ***************************************************************************/


#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include "libtzioc.h"
#include "libtzioc_ioctl.h"

#define TZIOC_DEV_DEFAULT_FD    0xffff

struct tzioc_client *_tzioc_client_open(
    const char *pName)
{
    struct tzioc_client *pClient;
    struct mq_attr msgQAttr;
    char msgQPath[TZIOC_CLIENT_NAME_LEN_MAX + 10];
    int err = 0;

    /* alloc client */
    pClient = malloc(sizeof(struct tzioc_client));
    if (!pClient) {
        LOGE("failed to alloc TZIOC client");
        return NULL;
    }

    /* init client */
    memset(pClient, 0, sizeof(struct tzioc_client));
    pClient->fd = -1;
    pClient->msgQ = -1;
    strncpy(pClient->name, pName, TZIOC_CLIENT_NAME_LEN_MAX-1);
    pClient->name[TZIOC_CLIENT_NAME_LEN_MAX-1] = '\0';

    /* open tzioc device */
    pClient->fd = TZIOC_DEV_DEFAULT_FD;

    /* open msg queue */
    memset(&msgQAttr, 0, sizeof(msgQAttr));
    msgQAttr.mq_maxmsg  = 16;
    msgQAttr.mq_msgsize = 1024;

    sprintf(msgQPath, "/libtzioc.%s", pClient->name);

    pClient->msgQ = mq_open(
        msgQPath,
        O_CREAT | O_RDWR,
        S_IRUSR | S_IXUSR,
        &msgQAttr);

    if (pClient->msgQ == -1) {
        LOGE("failed to open TZIOC msg queue");
        goto ERR_EXIT;
    }

    /* open kernel client */
    err = _tzioc_ioctl_client_open(pClient);
    if (err) {
        LOGE("failed to open TZIOC kernel client");
        goto ERR_EXIT;
    }

    /* map shared memory to user space */
    pClient->psmem = (void *)pClient->smemStart;
    return pClient;

 ERR_EXIT:
    _tzioc_client_close(pClient);
    return NULL;
}

void _tzioc_client_close(
    struct tzioc_client *pClient)
{
    int err = 0;

    /* unmap shared memory from user space */

    /* close kernel client */
    if (pClient->hKlient) {
        err = _tzioc_ioctl_client_close(pClient);
        if (err) {
            LOGE("failed to close TZIOC kernel client");
        }
    }

    /* close msg queue */
    if (pClient->msgQ != -1) {
        char msgQPath[TZIOC_CLIENT_NAME_LEN_MAX + 10];

        err = mq_close(pClient->msgQ);
        if (err == -1) {
            LOGE("failed to close TZIOC msg queue");
        }

        sprintf(msgQPath, "/libtzioc.%s", pClient->name);
        err = mq_unlink(msgQPath);
        if (err == -1) {
            LOGE("failed to unlink TZIOC msg queue");
        }
    }

    /* close tzioc device */

    /* free client */
    free(pClient);
}

int _tzioc_client_getid(
    struct tzioc_client *pClient,
    const char *pName,
    uint32_t ulPid,
    uint8_t *pId)
{
    int err = 0;

    err = _tzioc_ioctl_client_getid(
        pClient,
        pName,
        ulPid,
        pId);

    if (err) {
        LOGE("failed to get client id");
        return err;
    }
    return 0;
}

int _tzioc_msg_send(
    struct tzioc_client *pClient,
    struct tzioc_msg_hdr *pHdr)
{
    int err = 0;

    err = _tzioc_ioctl_msg_send(pClient, pHdr);
    if (err) {
        LOGE("failed to send msg");
        return err;
    }
    return 0;
}

int _tzioc_msg_receive(
    struct tzioc_client *pClient,
    struct tzioc_msg_hdr *pHdr,
    uint32_t ulSize,
    uint32_t ulTimeout)
{
    ssize_t bytes;

    if (ulTimeout == -1) {
        while (1) {
            bytes = mq_receive(
                pClient->msgQ,
                (char *)pHdr,
                ulSize,
                0);

            if (bytes == -1) {
                // Interrupted, return to handle signals
                if (errno == EINTR)
                    return -EINTR;

                LOGE("error receiving msg from msg queue, errno %d", errno);
                return -EIO;
            }
            break;
        }
    }
    else {
        struct timespec timeout;
        uint64_t nsec;
        uint32_t sec;

        clock_gettime(CLOCK_REALTIME, &timeout);
        nsec = timeout.tv_nsec + (uint64_t)ulTimeout * 1000000;
        sec = nsec / 1000000000;
        timeout.tv_sec += sec;
        timeout.tv_nsec = nsec - sec * 1000000000;

        while (1) {
            bytes = mq_timedreceive(
                pClient->msgQ,
                (char *)pHdr,
                ulSize,
                0,
                &timeout);

            if (bytes == -1) {
                // Interrupted, return to handle signals
                if (errno == EINTR)
                    return -EINTR;

                if (errno == ETIMEDOUT)
                    return -ETIMEDOUT;

                LOGE("error receiving msg from msg queue");
                return -EIO;
            }
            break;
        }
    }

    /* check msg size */
    if (bytes != sizeof(*pHdr) + pHdr->ulLen) {
        return -EBADMSG;
    }
    return 0;
}

uintptr_t _tzioc_offset2vaddr(
    struct tzioc_client *pClient,
    uint32_t ulOffset)
{
    if (ulOffset < pClient->smemSize)
        return ulOffset + (uintptr_t)pClient->psmem;
    else
        return (uintptr_t)-1;
}

uint32_t _tzioc_vaddr2offset(
    struct tzioc_client *pClient,
    uintptr_t ulVaddr)
{
    if (ulVaddr >= (uintptr_t)pClient->psmem &&
        ulVaddr <  (uintptr_t)pClient->psmem + pClient->smemSize)
        return ulVaddr - (uintptr_t)pClient->psmem;
    else
        return (uint32_t)-1;
}

int _tzioc_map_paddrs(
    struct tzioc_client *pClient,
    uint8_t ucCount,
    struct tzioc_mem_region *pRegions)
{
    int err = 0;

    err = _tzioc_ioctl_map_paddrs(
        pClient,
        ucCount,
        pRegions);

    if (err) {
        LOGE("failed to unmap physical address ranges");
        return err;
    }
    return 0;
}

int _tzioc_unmap_paddrs(
    struct tzioc_client *pClient,
    uint8_t ucCount,
    struct tzioc_mem_region *pRegions)
{
    int err = 0;

    err = _tzioc_ioctl_unmap_paddrs(
        pClient,
        ucCount,
        pRegions);

    if (err) {
        LOGE("failed to unmap physical address ranges");
        return err;
    }
    return 0;
}

uintptr_t _tzioc_paddr2vaddr(
    struct tzioc_client *pClient,
    uintptr_t ulPaddr)
{
    uintptr_t ulVaddr;
    int err = 0;

    err = _tzioc_ioctl_paddr2vaddr(
        pClient,
        ulPaddr,
        &ulVaddr);

    if (err) {
        LOGE("failed to convert physical address to virtual address");
        return (uintptr_t)-1;
    }
    return ulVaddr;
}

uintptr_t _tzioc_vaddr2paddr(
    struct tzioc_client *pClient,
    uintptr_t ulVaddr)
{
    uintptr_t ulPaddr;
    int err = 0;

    err = _tzioc_ioctl_vaddr2paddr(
        pClient,
        ulVaddr,
        &ulPaddr);

    if (err) {
        LOGE("failed to convert virtual address to physical address");
        return (uintptr_t)-1;
    }
    return ulPaddr;
}
