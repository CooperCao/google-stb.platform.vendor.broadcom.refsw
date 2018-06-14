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

#ifndef LIBASTRA_H
#define LIBASTRA_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>

#include "libastra_api.h"
#include "astra_ioctls.h"

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#ifndef LOGI
#define LOGD(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGW(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGE(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGI(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#endif

/* astra control block */
struct astra_struct
{
    /* astra device file descriptor */
    int fd;

    /* Core dump device file descriptor */
    int core_fd;

    /* astra version, config and status */
    struct astra_version version;
    struct astra_config config;
    struct astra_status status;

    /* mapped shared memory */
    uint32_t smemSize;
    void *psmem;
};

/* astra user space client */
struct astra_client
{
    /* user space callback */
    astra_event_callback pCallback;
    void *pPrivData;

    /* kernel client */
    astra_kclient_handle hKClient;

    /* event thread */
    pthread_t eventThread;
};

/* exported functions */
int _astra_init(void);
int _astra_deinit(void);

int _astra_version_get(
    struct astra_version *pVersion);

int _astra_config_get(
    struct astra_config *pConfig);

int _astra_status_get(
    struct astra_status *pStatus);

struct astra_client *_astra_client_open(
    const char *pName,
    astra_event_callback pCallback,
    void *pPrivData);

void _astra_client_close(
    struct astra_client *pClient);

astra_uapp_handle _astra_uapp_open(
    struct astra_client *pClient,
    const char *pName,
    const char *pPath);

void _astra_uapp_close(
    astra_uapp_handle pUapp);

astra_peer_handle _astra_peer_open(
    astra_uapp_handle pUapp,
    const char *pName);

void _astra_peer_close(
    astra_peer_handle pPeer);

int _astra_msg_send(
    astra_peer_handle pPeer,
    const void *pMsg,
    uint32_t msgLen);

int _astra_msg_receive(
    struct astra_client *pClient,
    astra_peer_handle *ppPeer,
    void *pMsg,
    uint32_t *pMsgLen,
    int timeout);

void *_astra_mem_alloc(
    struct astra_client *pClient,
    uint32_t size);

void _astra_mem_free(
    struct astra_client *pClient,
    void *pBuff);

astra_paddr_t _astra_pmem_alloc(
    struct astra_client *pClient,
    uint32_t size);

void _astra_pmem_free(
    struct astra_client *pClient,
    astra_paddr_t paddr);

void *_astra_offset2vaddr(
    struct astra_client *pClient,
    uint64_t offset);

uint64_t _astra_vaddr2offset(
    struct astra_client *pClient,
    void *pBuff);

astra_file_handle _astra_file_open(
    struct astra_client *pClient,
    const char *pPath,
    int flags);

void _astra_file_close(
    astra_file_handle pFile);

int _astra_file_write(
    astra_file_handle pFile,
    astra_paddr_t paddr,
    uint32_t bytes);

int _astra_file_read(
    astra_file_handle pFile,
    astra_paddr_t paddr,
    uint32_t bytes);

int _astra_call_smc(
    struct astra_client *pClient,
    astra_smc_code code);

void _astra_uapp_coredump(
    astra_uapp_handle pUapp);

/* global variables */
extern struct astra_struct *pAstra;

#endif /* LIBASTRA_H */
