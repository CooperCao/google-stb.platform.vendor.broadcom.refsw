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

#include "libastra_api.h"
#include "libastra.h"

int astra_init(void)
{
    return _astra_init();
}

int astra_deinit(void)
{
    return _astra_deinit();
}

int astra_version_get(astra_version *pVersion)
{
    if (!pVersion) {
        LOGE("invalid args");
        return -EINVAL;
    }

    /* get astra version */
    return _astra_version_get(pVersion);
}

int astra_config_get(astra_config *pConfig)
{
    if (!pConfig) {
        LOGE("invalid args");
        return -EINVAL;
    }

    /* get astra config */
    return _astra_config_get(pConfig);
}

int astra_status_get(astra_status *pStatus)
{
    if (!pStatus) {
        LOGE("invalid args");
        return -EINVAL;
    }

    /* get astra status */
    return _astra_status_get(pStatus);
}

astra_client_handle astra_client_open(
    const char *pName,
    astra_event_callback pCallback,
    void *pPrivData)
{
    struct astra_client *pClient;

    if (!pName || pName[0] == '\0' ||
        !pCallback) {
        LOGE("invalid args");
        return 0;
    }

    /* open astra client */
    pClient = _astra_client_open(
        pName,
        pCallback,
        pPrivData);

    if (!pClient) {
        LOGE("failed to open astra client");
        return 0;
    }

    LOGI("open astra client %s, handle %p", pName, pClient);

    return (astra_client_handle)pClient;
}

void astra_client_close(
    astra_client_handle hClient)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient) {
        LOGE("invalid args");
        return;
    }

    LOGI("close astra client, handle %p", pClient);

    /* close astra client */
    _astra_client_close(pClient);
}

astra_uapp_handle astra_uapp_open(
    astra_client_handle hClient,
    const char *pName,
    const char *pPath)
{
    struct astra_client *pClient = (struct astra_client *)hClient;
    astra_uapp_handle pUapp;

    if (!pClient ||
        !pName || pName[0] == '\0' ||
        !pPath || pPath[0] == '\0') {
        LOGE("invalid args");
        return 0;
    }

    /* open astra userapp */
    pUapp = _astra_uapp_open(
        pClient,
        pName,
        pPath);

    if (!pUapp) {
        LOGE("failed to open astra userapp");
        return 0;
    }

    LOGI("open astra userapp %s", pName);

    return pUapp;
}

void astra_uapp_close(
    astra_uapp_handle hUapp)
{
    if (!hUapp) {
        LOGE("invalid args");
        return;
    }

    LOGI("close astra userapp");

    /* close astra userapp */
    _astra_uapp_close(hUapp);
}

astra_peer_handle astra_peer_open(
    astra_uapp_handle hUapp,
    const char *pName)
{
    astra_peer_handle pPeer;

    if (!hUapp ||
        !pName || pName[0] == '\0') {
        LOGE("invalid args");
        return 0;
    }

    /* open astra peer */
    pPeer = _astra_peer_open(
        hUapp,
        pName);

    if (!pPeer) {
        LOGE("failed to open astra peer");
        return 0;
    }

    LOGI("open astra peer %s", pName);

    return pPeer;
}

void astra_peer_close(
    astra_peer_handle hPeer)
{
    if (!hPeer) {
        LOGE("invalid args");
        return;
    }

    LOGI("close astra peer");

    /* close astra peer */
    _astra_peer_close(hPeer);
}

int astra_msg_send(
    astra_peer_handle hPeer,
    const void *pMsg,
    uint32_t msgLen)
{
    if (!hPeer || !pMsg || msgLen == 0) {
        LOGE("invalid args");
        return -EINVAL;
    }
    /* send astra msg */
    return _astra_msg_send(
        hPeer,
        pMsg,
        msgLen);
}

int astra_msg_receive(
    astra_client_handle hClient,
    astra_peer_handle *phPeer,
    void *pMsg,
    uint32_t *pMsgLen,
    int timeout)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient || !phPeer || !pMsg || !pMsgLen) {
        LOGE("invalid args");
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

void *astra_mem_alloc(
    astra_client_handle hClient,
    uint32_t size)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient || size == 0) {
        LOGE("invalid args");
        return NULL;
    }

    /* alloc astra shared mem */
    return _astra_mem_alloc(
        pClient,
        size);
}

void astra_mem_free(
    astra_client_handle hClient,
    void *pBuff)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient || !pBuff) {
        LOGE("invalid args");
        return;
    }

    /* free astra shared mem */
    _astra_mem_free(
        pClient,
        pBuff);
}

astra_paddr_t astra_pmem_alloc(
    astra_client_handle hClient,
    uint32_t size)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient || size == 0) {
        LOGE("invalid args");
        return 0;
    }

    /* To Be Done */
    return 0;
}

void astra_pmem_free(
    astra_client_handle hClient,
    astra_paddr_t paddr)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient || paddr == 0) {
        LOGE("invalid args");
        return;
    }

    /* To Be Done */
}

void *astra_offset2vaddr(
    astra_client_handle hClient,
    uint64_t offset)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient) {
        LOGE("invalid args");
        return NULL;
    }

    /* astra user space addr conversion */
    return (void *)_astra_offset2vaddr(pClient, offset);
}

uint64_t astra_vaddr2offset(
    astra_client_handle hClient,
    void *pBuff)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient) {
        LOGE("invalid args");
        return 0;
    }

    /* astra user space addr conversion */
    return _astra_vaddr2offset(pClient, pBuff);
}

astra_file_handle astra_file_open(
    astra_client_handle hClient,
    const char *pPath,
    int flags)
{
    struct astra_client *pClient = (struct astra_client *)hClient;
    astra_file_handle pFile;

    if (!pClient ||
        !pPath || pPath[0] == '\0') {
        LOGE("invalid args");
        return 0;
    }

    /* open astra file */
    pFile = _astra_file_open(
        pClient,
        pPath,
        flags);

    if (!pFile) {
        LOGE("failed to open astra file");
        return 0;
    }

    LOGI("open astra file %s", pPath);

    return pFile;
}

void astra_file_close(
    astra_file_handle hFile)
{
    if (!hFile) {
        LOGE("invalid args");
        return;
    }

    LOGI("close astra file");

    /* close astra file */
    _astra_file_close(hFile);
}

int astra_file_write(
    astra_file_handle hFile,
    astra_paddr_t paddr,
    uint32_t bytes)
{
    if (!hFile || paddr == 0 || bytes == 0) {
        LOGE("invalid args");
        return -EINVAL;
    }

    /* write to astra file */
    return _astra_file_write(
        hFile,
        paddr,
        bytes);
}

int astra_file_read(
    astra_file_handle hFile,
    astra_paddr_t paddr,
    uint32_t bytes)
{
    if (!hFile || paddr == 0 || bytes == 0) {
        LOGE("invalid args");
        return -EINVAL;
    }

    /* read from astra file */
    return _astra_file_read(
        hFile,
        paddr,
        bytes);
}

int astra_call_smc(
    astra_client_handle hClient,
    astra_smc_code code)
{
    struct astra_client *pClient = (struct astra_client *)hClient;

    if (!pClient) {
        LOGE("invalid args");
        return -EINVAL;
    }

    /* call astra smc */
    return _astra_call_smc(
        pClient,
        code);
}

void astra_uapp_coredump(
    astra_uapp_handle hUapp)
{
    if (!hUapp) {
        LOGE("invalid args");
        return;
    }

    LOGI("close astra userapp");

    /* close astra userapp */
    _astra_uapp_coredump(hUapp);
}
