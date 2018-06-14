/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#include "libastra.h"
#include "libastra_ioctl.h"
#include "libastra_coredump.h"

struct astra_struct astra = {
    .fd = -1,
    .core_fd = -1,
    .psmem = NULL,
};

struct astra_struct *pAstra = &astra;

static void *astra_client_event_thread(void *arg)
{
    struct astra_client *pClient = (struct astra_client *)arg;
    astra_event event;
    char eventData[16] __attribute__ ((aligned (8)));
    uint32_t eventDataLen;
    int err = 0;

    while (1) {
        /* poll for event */
        err = _astra_ioctl_event_poll(
            pClient->hKClient,
            &event,
            &eventData,
            &eventDataLen);

        /* exit if client is closed */
        if (err == -ENODEV)
            break;

        /* ignore all other errors */
        if (err)
            continue;

        /* callback with received event */
        if (pClient->pCallback) {
            pClient->pCallback(
                event,
                (eventDataLen) ? &eventData : NULL,
                pClient->pPrivData);
        }
    }

    return NULL;
}

int _astra_init(void)
{
    int err = 0;

    /* open astra device */
    pAstra->fd = open("/dev/astra", O_RDWR | O_SYNC);

    if (pAstra->fd == -1) {
        LOGE("failed to open astra device");
        return errno;
    }

    /* get astra version */
    err = _astra_ioctl_version_get(&pAstra->version);

    if (err) {
        LOGE("failed to get astra version");
        goto ERR_EXIT;
    }

    /* get astra config */
    err = _astra_ioctl_config_get(&pAstra->config);

    if (err) {
        LOGE("failed to get astra config");
        goto ERR_EXIT;
    }

    /* get astra status */
    err = _astra_ioctl_status_get(&pAstra->status);

    if (err) {
        LOGE("failed to get astra status");
        goto ERR_EXIT;
    }

    /* map astra shared memory to user space */
    pAstra->smemSize = pAstra->config.smemSize;
    pAstra->psmem = mmap(
        0,
        pAstra->smemSize,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        pAstra->fd,
        0);

    if (pAstra->psmem == (void *)-1) {
        LOGE("failed to map astra shared memory to user space");
        pAstra->psmem = NULL;
        err = errno;
        goto ERR_EXIT;
    }

    return 0;

 ERR_EXIT:
    _astra_deinit();
    return err;
}

int _astra_deinit(void)
{
    /* unmap astra shared memory from user space */
    if (pAstra->psmem) {
        munmap(
            pAstra->psmem,
            pAstra->smemSize);
        pAstra->psmem = NULL;
    }

    /* close astra device */
    if (pAstra->fd != -1) {
        close(pAstra->fd);
        pAstra->fd = -1;
    }

    return 0;
}

int _astra_version_get(
    struct astra_version *pVersion)
{
    int err = 0;

    if (pAstra->fd == -1) {
        err = _astra_init();
        if (err) return err;
    }

    *pVersion = pAstra->version;
    return 0;
}

int _astra_config_get(
    struct astra_config *pConfig)
{
    int err = 0;

    if (pAstra->fd == -1) {
        err = _astra_init();
        if (err) return err;
    }

    *pConfig = pAstra->config;
    return 0;
}

int _astra_status_get(
    struct astra_status *pStatus)
{
    int err = 0;

    if (pAstra->fd == -1) {
        err = _astra_init();
        if (err) return err;
    }

    *pStatus = pAstra->status;
    return 0;
}

struct astra_client *_astra_client_open(
    const char *pName,
    astra_event_callback pCallback,
    void *pPrivData)
{
    struct astra_client *pClient;
    int err = 0;

    if (pAstra->fd == -1) {
        err = _astra_init();
        if (err) return 0;
    }

    /* alloc client */
    pClient = malloc(sizeof(struct astra_client));
    if (!pClient) {
        LOGE("failed to alloc astra client");
        return 0;
    }

    /* init client */
    memset(pClient, 0, sizeof(struct astra_client));
    pClient->pCallback = pCallback;
    pClient->pPrivData = pPrivData;

    /* open kernel client */
    err = _astra_ioctl_client_open(
        pName,
        pClient,
        &pClient->hKClient);

    if (err) {
        LOGE("failed to open kernel client");
        goto ERR_EXIT;
    }

    /* create event thread */
    err = pthread_create(
        &pClient->eventThread,
        NULL,
        &astra_client_event_thread,
        pClient);

    if (err) {
        LOGE("failed to create event thread");
        /* reset pthread id on err */
        pClient->eventThread = (pthread_t)0;
        goto ERR_EXIT;
    }

    return pClient;

 ERR_EXIT:
    _astra_client_close(pClient);
    return 0;
}

void _astra_client_close(
    struct astra_client *pClient)
{
    int err = 0;

    /* terminate event thread */
    if (pClient->eventThread) {
        /* send exit to event thread */
        _astra_ioctl_event_exit(pClient->hKClient);

        /* wait for event thread to terminate */
        pthread_join(pClient->eventThread, NULL);
    }

    /* close kernel client */
    if (pClient->hKClient) {
        err = _astra_ioctl_client_close(pClient->hKClient);

        if (err) {
            LOGE("failed to close kernel client");
        }
    }

    /* free client */
    free(pClient);
}

astra_uapp_handle _astra_uapp_open(
    struct astra_client *pClient,
    const char *pName,
    const char *pPath)
{
    astra_uapp_handle pUapp;
    int err = 0;

    /* open kernel uapp */
    err = _astra_ioctl_uapp_open(
        pClient->hKClient,
        pName,
        pPath,
        (astra_kuapp_handle *)&pUapp);

    if (err) {
        LOGE("failed to open kernel userapp");
        return 0;
    }

    return pUapp;
}

void _astra_uapp_close(
    astra_uapp_handle pUapp)
{
    int err = 0;

    /* close kernel uapp */
    err = _astra_ioctl_uapp_close((astra_kuapp_handle)pUapp);

    if (err) {
        LOGE("failed to close kernel userapp");
    }
}

astra_peer_handle _astra_peer_open(
    astra_uapp_handle pUapp,
    const char *pName)
{
    astra_peer_handle pPeer;
    int err = 0;

    /* open kernel peer */
    err = _astra_ioctl_peer_open(
        (astra_kuapp_handle)pUapp,
        pName,
        (astra_kpeer_handle *)&pPeer);

    if (err) {
        LOGE("failed to open kernel peer");
        return 0;
    }

    return pPeer;
}

void _astra_peer_close(
    astra_peer_handle pPeer)
{
    int err = 0;

    /* close kernel peer */
    err = _astra_ioctl_peer_close((astra_kpeer_handle)pPeer);

    if (err) {
        LOGE("failed to close kernel peer");
    }
}

int _astra_msg_send(
    astra_peer_handle pPeer,
    const void *pMsg,
    uint32_t msgLen)
{
    int err = 0;

    /* send msg to kernel */
    err = _astra_ioctl_msg_send(
        (astra_kpeer_handle)pPeer,
        pMsg,
        msgLen);

    if (err) {
        LOGE("failed to send msg to kernel");
        return err;
    }

    return 0;
}

int _astra_msg_receive(
    struct astra_client *pClient,
    astra_peer_handle *ppPeer,
    void *pMsg,
    uint32_t *pMsgLen,
    int timeout)
{
    int err = 0;

    /* receive msg from kernel */
    err = _astra_ioctl_msg_receive(
        (astra_kclient_handle)pClient->hKClient,
        (astra_kpeer_handle *)ppPeer,
        pMsg,
        pMsgLen,
        timeout);

    if (err) {
        if (err != -ENOMSG)
            LOGE("failed to receive msg from kernel");
        return err;
    }

    return 0;
}

void *_astra_mem_alloc(
    struct astra_client *pClient,
    uint32_t size)
{
    uint32_t buffOffset;
    void *pBuff;
    int err = 0;

    err = _astra_ioctl_mem_alloc(
        pClient->hKClient,
        size,
        &buffOffset);

    if (err) {
        LOGE("failed to alloc shared mem");
        return NULL;
    }

    pBuff = _astra_offset2vaddr(
        pClient,
        buffOffset);

    if (!pBuff) {
        LOGE("failed to convert offset to vaddr");
        return NULL;
    }

    return pBuff;
}

void _astra_mem_free(
    struct astra_client *pClient,
    void *pBuff)
{
    uint32_t buffOffset;
    int err = 0;

    buffOffset = _astra_vaddr2offset(
        pClient,
        pBuff);

    if (buffOffset == 0) {
        LOGE("failed to convert vaddr to offset");
        return;
    }

    err = _astra_ioctl_mem_free(
        pClient->hKClient,
        buffOffset);

    if (err) {
        LOGE("failed to free shared mem");
        return;
    }
}

void *_astra_offset2vaddr(
    struct astra_client *pClient,
    uint64_t offset)
{
    UNUSED(pClient);

    if (offset < pAstra->smemSize)
        return (uint8_t *)pAstra->psmem + offset;
    else
        return NULL;
}

uint64_t _astra_vaddr2offset(
    struct astra_client *pClient,
    void *pBuff)
{
    UNUSED(pClient);

    if ((uint8_t *)pBuff >= (uint8_t *)pAstra->psmem &&
        (uint8_t *)pBuff <  (uint8_t *)pAstra->psmem + pAstra->smemSize)
        return (uint8_t *)pBuff - (uint8_t *)pAstra->psmem;
    else
        return 0;
}

astra_file_handle _astra_file_open(
    struct astra_client *pClient,
    const char *pPath,
    int flags)
{
    astra_file_handle pFile;
    int err = 0;

    /* open kernel file */
    err = _astra_ioctl_file_open(
        pClient->hKClient,
        pPath,
        flags,
        (astra_kfile_handle *)&pFile);

    if (err) {
        LOGE("failed to open kernel file");
        return 0;
    }

    return pFile;
}

void _astra_file_close(
    astra_file_handle pFile)
{
    int err = 0;

    /* close kernel file */
    err = _astra_ioctl_file_close((astra_kfile_handle)pFile);

    if (err) {
        LOGE("failed to close kernel file");
    }
}

int _astra_file_write(
    astra_file_handle pFile,
    astra_paddr_t paddr,
    uint32_t bytes)
{
    int err = 0;

    /* write to kernel file */
    err = _astra_ioctl_file_write(
        (astra_kfile_handle)pFile,
        paddr,
        &bytes);

    if (err) {
        LOGE("failed to write to kernel file");
        return err;
    }

    return bytes;
}

int _astra_file_read(
    astra_file_handle pFile,
    astra_paddr_t paddr,
    uint32_t bytes)
{
    int err = 0;

    /* read from kernel file */
    err = _astra_ioctl_file_read(
        (astra_kfile_handle)pFile,
        paddr,
        &bytes);

    if (err) {
        LOGE("failed to read from kernel file");
        return err;
    }

    return bytes;
}

int _astra_call_smc(
    struct astra_client *pClient,
    astra_smc_code code)
{
    int err = 0;

    /* make kernel call SMC */
    err = _astra_ioctl_call_smc(
        pClient->hKClient,
        code);

    if (err) {
        LOGE("failed to call SMC");
        return err;
    }

    return 0;
}

#define COREDUMP_FILE_SIZE        4*1024*1024

void _astra_uapp_coredump(
    astra_uapp_handle pUapp)
{
    int err = 0;
    void * core_buf=NULL;
    unsigned int *buf=NULL;

    /* open Coredump device */
    pAstra->core_fd = open("/dev/astra_coredump", O_RDWR | O_SYNC);

    if (pAstra->core_fd == -1) {
        LOGE("failed to open coredump device");
        return ;
    }

    core_buf = mmap(
        0,
        COREDUMP_FILE_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        pAstra->core_fd,
        0);

    if (core_buf== (void *)-1) {
        LOGE("failed to map coredump memory to user space");
        return;
    }
    /* uapp core dump */
    err = _astra_ioctl_uapp_coredump((astra_kuapp_handle)pUapp);

    if (err) {
        LOGE("failed to core dump userapp");
    }

    buf = (unsigned int *)core_buf;
    _astra_uapp_coredump_proc(buf);

    munmap(
        core_buf,
        COREDUMP_FILE_SIZE);
}
