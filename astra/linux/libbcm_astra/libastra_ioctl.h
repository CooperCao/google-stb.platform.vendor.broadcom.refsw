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

#ifndef LIBASTRA_IOCTL_H
#define LIBASTRA_IOCTL_H

#include "libastra.h"

/* astra ioctl calls */
int _astra_ioctl_event_poll(
    astra_kclient_handle hKClient,
    astra_event *pEvent,
    void *pEventData,
    uint32_t *pEventDataLen);

int _astra_ioctl_event_exit(
    astra_kclient_handle hKClient);

int _astra_ioctl_version_get(
    struct astra_version *pVersion);

int _astra_ioctl_config_get(
    struct astra_config *pConfig);

int _astra_ioctl_status_get(
    struct astra_status *pStatus);

int _astra_ioctl_call_smc(
    astra_kclient_handle hKClient,
    astra_smc_code code);

int _astra_ioctl_client_open(
    const char *pName,
    void *pPrivData,
    astra_kclient_handle *phKClient);

int _astra_ioctl_client_close(
    astra_kclient_handle hKClient);

int _astra_ioctl_uapp_open(
    astra_kclient_handle hKClient,
    const char *pName,
    const char *pPath,
    astra_kuapp_handle *phKUapp);

int _astra_ioctl_uapp_close(
    astra_kuapp_handle hKUapp);

int _astra_ioctl_peer_open(
    astra_kuapp_handle hKUapp,
    const char *pName,
    astra_kpeer_handle *phKPeer);

int _astra_ioctl_peer_close(
    astra_kpeer_handle hKPeer);

int _astra_ioctl_msg_send(
    astra_kpeer_handle hKPeer,
    const void *pMsg,
    uint32_t msgLen);

int _astra_ioctl_msg_receive(
    astra_kclient_handle hKClient,
    astra_kpeer_handle *phKPeer,
    void *pMsg,
    uint32_t *pMsgLen,
    int timeout);

int _astra_ioctl_mem_alloc(
    astra_kclient_handle hKClient,
    uint32_t size,
    uint32_t *pBuffOffset);

int _astra_ioctl_mem_free(
    astra_kclient_handle hKClient,
    uint32_t buffOffset);

int _astra_ioctl_file_open(
    astra_kclient_handle hKClient,
    const char *pPath,
    int flags,
    astra_kfile_handle *phKFile);

int _astra_ioctl_file_close(
    astra_kfile_handle hKFile);

int _astra_ioctl_file_write(
    astra_kfile_handle hKFile,
    astra_paddr_t paddr,
    uint32_t *pBytes);

int _astra_ioctl_file_read(
    astra_kfile_handle hKFile,
    astra_paddr_t paddr,
    uint32_t *pBytes);

int _astra_ioctl_uapp_coredump(
    astra_kuapp_handle hKUapp);


#endif /* LIBASTRA_H */
