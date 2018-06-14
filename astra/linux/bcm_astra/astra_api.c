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

#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/jiffies.h>

#include "astra_version.h"
#include "astra_api.h"
#include "astra_drv.h"

int astra_version_get(astra_version *pVersion)
{
    if (!pVersion) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    /* get astra version */
    return _astra_version_get(pVersion);
}
EXPORT_SYMBOL(astra_version_get);

int astra_config_get(astra_config *pConfig)
{
    if (!pConfig) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    /* get astra config */
    return _astra_config_get(pConfig);
}
EXPORT_SYMBOL(astra_config_get);

int astra_status_get(astra_status *pStatus)
{
    if (!pStatus) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    /* get astra status */
    return _astra_status_get(pStatus);
}
EXPORT_SYMBOL(astra_status_get);

astra_client_handle astra_client_open(
    const char *pName,
    astra_event_callback pCallback,
    void *pPrivData)
{
    struct astra_client *pClient;

    if (!pName || pName[0] == '\0' ||
        !pCallback) {
        LOGE("Invalid args");
        return NULL;
    }

    /* open astra kernel client */
    pClient = _astra_kernel_client_open(
        pName,
        pCallback,
        pPrivData);

    if (!pClient) {
        LOGE("Failed to open astra client");
        return NULL;
    }

    LOGI("Open astra client %s, handle %p", pClient->name, pClient);

    return (astra_client_handle)pClient;
}
EXPORT_SYMBOL(astra_client_open);

void astra_client_close(
    astra_client_handle hClient)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient) {
        LOGE("Invalid args");
        return;
    }

    LOGI("Close astra client %s, handle %p", pClient->name, pClient);

    /* close astra kernel client */
    _astra_kernel_client_close(pClient);
}
EXPORT_SYMBOL(astra_client_close);

astra_uapp_handle astra_uapp_open(
    astra_client_handle hClient,
    const char *pName,
    const char *pPath)
{
    struct astra_client *pClient = (struct astra_client *)hClient;
    struct astra_uapp *pUapp;

    if (!pClient ||
        !pName || pName[0] == '\0' ||
        !pPath || pPath[0] == '\0') {
        LOGE("Invalid args");
        return NULL;
    }

    /* open astra userapp */
    pUapp = _astra_uapp_open(
        pClient,
        pName,
        pPath);

    if (!pUapp) {
        LOGE("Failed to open astra userapp");
        return NULL;
    }

    LOGI("Open astra client %s userapp %s, handle %p",
         pClient->name, pUapp->name, pUapp);

    return pUapp;
}
EXPORT_SYMBOL(astra_uapp_open);

void astra_uapp_close(
    astra_uapp_handle hUapp)
{
    struct astra_uapp *pUapp = (struct astra_uapp *)hUapp;
    struct astra_client *pClient;

    if (!pUapp) {
        LOGE("Invalid args");
        return;
    }

    pClient = pUapp->pClient;

    LOGI("Close astra client %s userapp %s, handle %p",
         pClient->name, pUapp->name, pUapp);

    /* close astra userapp */
    _astra_uapp_close(pUapp);
}
EXPORT_SYMBOL(astra_uapp_close);

astra_peer_handle astra_peer_open(
    astra_uapp_handle hUapp,
    const char *pName)
{
    struct astra_uapp *pUapp = (struct astra_uapp *)hUapp;
    struct astra_peer *pPeer;
    struct astra_client *pClient;

    if (!pUapp ||
        !pName || pName[0] == '\0') {
        LOGE("Invalid args");
        return NULL;
    }

    pClient = pUapp->pClient;

    /* open astra peer */
    pPeer = _astra_peer_open(
        pUapp,
        pName);

    if (!pPeer) {
        LOGE("Failed to open astra peer");
        return NULL;
    }

    LOGI("Open astra client %s userapp %s peer %s, handle %p",
         pClient->name, pUapp->name, pPeer->name, pPeer);

    return (astra_peer_handle)pPeer;
}
EXPORT_SYMBOL(astra_peer_open);

void astra_peer_close(
    astra_peer_handle hPeer)
{
    struct astra_peer *pPeer = (struct astra_peer *)hPeer;
    struct astra_uapp *pUapp;
    struct astra_client *pClient;

    if (!pPeer) {
        LOGE("Invalid args");
        return;
    }

    pUapp = pPeer->pUapp;
    pClient = pUapp->pClient;

    LOGI("Close astra client %s userapp %s peer %s, handle %p",
         pClient->name, pUapp->name, pPeer->name, pPeer);

    /* close astra peer */
    _astra_peer_close(pPeer);
}
EXPORT_SYMBOL(astra_peer_close);

int astra_msg_send(
    astra_peer_handle hPeer,
    const void *pMsg,
    uint32_t msgLen)
{
    struct astra_peer *pPeer = (struct astra_peer *)hPeer;

    if (!pPeer || !pMsg ||
        msgLen == 0 || msgLen > ASTRA_MSG_LEN_MAX) {
        LOGE("Invalid args");
        return -EINVAL;
    }
    /* send astra msg */
    return _astra_msg_send(
        pPeer,
        pMsg,
        msgLen);
}
EXPORT_SYMBOL(astra_msg_send);

int astra_msg_receive(
    astra_client_handle hClient,
    astra_peer_handle *phPeer,
    void *pMsg,
    uint32_t *pMsgLen,
    int timeout)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient || !pMsg || !phPeer || !pMsgLen ||
        *pMsgLen == 0) {
        LOGE("Invalid args");
        return -EINVAL;
    }
    /* receive astra msg */
    return _astra_msg_receive(
        pClient,
        phPeer,
        pMsg,
        pMsgLen,
        timeout);
}
EXPORT_SYMBOL(astra_msg_receive);

void *astra_mem_alloc(
    astra_client_handle hClient,
    uint32_t size)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient || size == 0) {
        LOGE("Invalid args");
        return NULL;
    }

    /* alloc astra shared mem */
    return _astra_mem_alloc(
        pClient,
        size);
}
EXPORT_SYMBOL(astra_mem_alloc);

void astra_mem_free(
    astra_client_handle hClient,
    void *pBuff)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient || !pBuff) {
        LOGE("Invalid args");
        return;
    }

    /* free astra shared mem */
    _astra_mem_free(
        pClient,
        pBuff);
}
EXPORT_SYMBOL(astra_mem_free);

astra_paddr_t astra_pmem_alloc(
    astra_client_handle hClient,
    uint32_t size)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient || size == 0) {
        LOGE("Invalid args");
        return 0;
    }

    /* To Be Done */
    return 0;
}
EXPORT_SYMBOL(astra_pmem_alloc);

void astra_pmem_free(
    astra_client_handle hClient,
    astra_paddr_t paddr)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient || paddr == 0) {
        LOGE("Invalid args");
        return;
    }

    /* To Be Done */
}
EXPORT_SYMBOL(astra_pmem_free);

void *astra_offset2vaddr(
    astra_client_handle hClient,
    uint64_t offset)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient) {
        LOGE("Invalid args");
        return NULL;
    }

    return _astra_offset2vaddr(
        pClient,
        offset);
}
EXPORT_SYMBOL(astra_offset2vaddr);

uint64_t astra_vaddr2offset(
    astra_client_handle hClient,
    void *pBuff)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient) {
        LOGE("Invalid args");
        return 0;
    }

    return _astra_vaddr2offset(
        pClient,
        pBuff);
}
EXPORT_SYMBOL(astra_vaddr2offset);

astra_file_handle astra_file_open(
    astra_client_handle hClient,
    const char *pPath,
    int flags)
{
    struct astra_client *pClient = (struct astra_client *)hClient;
    struct astra_file *pFile;

    if (!pClient ||
        !pPath || pPath[0] == '\0') {
        LOGE("Invalid args");
        return NULL;
    }

    /* open astra file */
    pFile = _astra_file_open(
        pClient,
        pPath,
        flags);

    if (!pFile) {
        LOGE("Failed to open astra file");
        return NULL;
    }

    LOGI("Open astra client %s file %s, handle %p",
         pClient->name, pFile->path, pFile);

    return (astra_file_handle)pFile;
}
EXPORT_SYMBOL(astra_file_open);

void astra_file_close(
    astra_file_handle hFile)
{
    struct astra_file *pFile = (struct astra_file *)hFile;
    struct astra_client *pClient;

    if (!pFile) {
        LOGE("Invalid args");
        return;
    }

    pClient = pFile->pClient;

    LOGI("Close astra client %s file %s, handle %p",
         pClient->name, pFile->path, pFile);

    /* close astra file */
    _astra_file_close(pFile);
}
EXPORT_SYMBOL(astra_file_close);

int astra_file_write(
    astra_file_handle hFile,
    astra_paddr_t paddr,
    uint32_t bytes)
{
    struct astra_file *pFile = (struct astra_file *)hFile;
    struct astra_client *pClient;

    if (!pFile || paddr == 0 || bytes == 0) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    pClient = pFile->pClient;

    LOGD("Write client %s file %s", pClient->name, pFile->path);

    /* write to astra file */
    return _astra_file_write(
        pFile,
        paddr,
        bytes);
}
EXPORT_SYMBOL(astra_file_write);

int astra_file_read(
    astra_file_handle hFile,
    astra_paddr_t paddr,
    uint32_t bytes)
{
    struct astra_file *pFile = (struct astra_file *)hFile;
    struct astra_client *pClient;

    if (!pFile || paddr == 0 || bytes == 0) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    pClient = pFile->pClient;

    LOGD("Read client %s file %s", pClient->name, pFile->path);

    /* read from astra file */
    return _astra_file_read(
        pFile,
        paddr,
        bytes);
}
EXPORT_SYMBOL(astra_file_read);

int astra_call_smc(
    astra_client_handle hClient,
    astra_smc_code code)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient) {
        LOGE("Invalid args");
        return -EINVAL;
    }

    /* call astra smc */
    return _astra_call_smc(
        pClient,
        code);
}
EXPORT_SYMBOL(astra_call_smc);

void astra_uapp_coredump(
    astra_uapp_handle hUapp)
{
    struct astra_uapp *pUapp = (struct astra_uapp *)hUapp;
    struct astra_client *pClient;

    if (!pUapp) {
        LOGE("Invalid args");
        return;
    }

    pClient = pUapp->pClient;

    LOGI("Coredump astra client %s userapp %s, handle %p",
        pClient->name, pUapp->name, pUapp);

    /* close astra userapp */
    _astra_uapp_coredump(pUapp);
}
EXPORT_SYMBOL(astra_uapp_coredump);
