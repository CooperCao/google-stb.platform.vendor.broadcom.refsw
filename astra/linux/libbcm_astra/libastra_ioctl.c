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

#include "astra_ioctls.h"
#include "libastra_ioctl.h"

int _astra_ioctl_event_poll(
    astra_kclient_handle hKClient,
    astra_event *pEvent,
    void *pEventData,
    size_t *pEventDataLen)
{
    struct astra_ioctl_event_poll_data eventPollData;
    int err = 0;

    eventPollData.hClient = hKClient;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_EVENT_POLL,
        (void *)&eventPollData);

    if (err)
        return -EIO;

    if (eventPollData.retVal)
        return eventPollData.retVal;

    *pEvent = eventPollData.event;
    *pEventDataLen = eventPollData.eventDataLen;
    if (*pEventDataLen) {
        memcpy(pEventData, eventPollData.eventData, *pEventDataLen);
    }

    return 0;
}

int _astra_ioctl_event_exit(
    astra_kclient_handle hKClient)
{
    struct astra_ioctl_event_exit_data eventExitData;
    int err = 0;

    eventExitData.hClient = hKClient;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_EVENT_EXIT,
        (void *)&eventExitData);

    if (err)
        return -EIO;

    if (eventExitData.retVal)
        return eventExitData.retVal;

    return 0;
}

int _astra_ioctl_version_get(
    struct astra_version *pVersion)
{
    struct astra_ioctl_version_get_data versionGetData;
    int err = 0;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_VERSION_GET,
        (void *)&versionGetData);

    if (err)
        return -EIO;

    if (versionGetData.retVal)
        return versionGetData.retVal;

    memcpy(pVersion, &versionGetData.version, sizeof(*pVersion));
    return 0;
}

int _astra_ioctl_config_get(
    struct astra_config *pConfig)
{
    struct astra_ioctl_config_get_data configGetData;
    int err = 0;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_CONFIG_GET,
        (void *)&configGetData);

    if (err)
        return -EIO;

    if (configGetData.retVal)
        return configGetData.retVal;

    memcpy(pConfig, &configGetData.config, sizeof(*pConfig));
    return 0;
}

int _astra_ioctl_status_get(
    struct astra_status *pStatus)
{
    struct astra_ioctl_status_get_data statusGetData;
    int err = 0;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_STATUS_GET,
        (void *)&statusGetData);

    if (err)
        return -EIO;

    if (statusGetData.retVal)
        return statusGetData.retVal;

    memcpy(pStatus, &statusGetData.status, sizeof(*pStatus));
    return 0;
}

int _astra_ioctl_call_smc(
    astra_kclient_handle hKClient,
    astra_smc_code code)
{
    struct astra_ioctl_call_smc_data callSmcData;
    int err = 0;

    callSmcData.hClient = hKClient;
    callSmcData.code = code;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_CALL_SMC,
        (void *)&callSmcData);

    if (err)
        return -EIO;

    if (callSmcData.retVal)
        return callSmcData.retVal;

    return 0;
}

int _astra_ioctl_client_open(
    const char *pName,
    void *pPrivData,
    astra_kclient_handle *phKClient)
{
    struct astra_ioctl_client_open_data clientOpenData;
    int err = 0;

    /* Avoid unused param warning */
    pPrivData = pPrivData;

    strncpy(clientOpenData.name, pName, ASTRA_NAME_LEN_MAX-1);
    /* Coverity:Explicitly null terminate string */
    clientOpenData.name[ASTRA_NAME_LEN_MAX-1] = '\0';

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_CLIENT_OPEN,
        (void *)&clientOpenData);

    if (err)
        return -EIO;

    if (clientOpenData.retVal)
        return clientOpenData.retVal;

    *phKClient = clientOpenData.hClient;
    return 0;
}

int _astra_ioctl_client_close(
    astra_kclient_handle hKClient)
{
    struct astra_ioctl_client_close_data clientCloseData;
    int err = 0;

    clientCloseData.hClient = hKClient;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_CLIENT_CLOSE,
        (void *)&clientCloseData);

    if (err)
        return -EIO;

    if (clientCloseData.retVal)
        return clientCloseData.retVal;

    return 0;
}

int _astra_ioctl_uapp_open(
    astra_kclient_handle hKClient,
    const char *pName,
    const char *pPath,
    astra_kuapp_handle *phKUapp)
{
    struct astra_ioctl_uapp_open_data uappOpenData;
    int err = 0;

    uappOpenData.hClient = hKClient;
    strncpy(uappOpenData.name, pName, ASTRA_NAME_LEN_MAX-1);
    /* Coverity:Explicitly null terminate string */
    uappOpenData.name[ASTRA_NAME_LEN_MAX-1] = '\0';
    strncpy(uappOpenData.path, pPath, ASTRA_PATH_LEN_MAX-1);
    /* Coverity:Explicitly null terminate string */
    uappOpenData.path[ASTRA_PATH_LEN_MAX-1] = '\0';

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_UAPP_OPEN,
        (void *)&uappOpenData);

    if (err)
        return -EIO;

    if (uappOpenData.retVal)
        return uappOpenData.retVal;

    *phKUapp = uappOpenData.hUapp;
    return 0;
}

int _astra_ioctl_uapp_close(
    astra_kuapp_handle hKUapp)
{
    struct astra_ioctl_uapp_close_data uappCloseData;
    int err = 0;

    uappCloseData.hUapp = hKUapp;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_UAPP_CLOSE,
        (void *)&uappCloseData);

    if (err)
        return -EIO;

    if (uappCloseData.retVal)
        return uappCloseData.retVal;

    return 0;
}

int _astra_ioctl_peer_open(
    astra_kuapp_handle hKUapp,
    const char *pName,
    astra_kpeer_handle *phKPeer)
{
    struct astra_ioctl_peer_open_data peerOpenData;
    int err = 0;

    peerOpenData.hUapp = hKUapp;
    strncpy(peerOpenData.name, pName, ASTRA_NAME_LEN_MAX-1);
    /* Coverity:Explicitly null terminate string */
    peerOpenData.name[ASTRA_NAME_LEN_MAX-1] = '\0';

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_PEER_OPEN,
        (void *)&peerOpenData);

    if (err)
        return -EIO;

    if (peerOpenData.retVal)
        return peerOpenData.retVal;

    *phKPeer = peerOpenData.hPeer;
    return 0;
}

int _astra_ioctl_peer_close(
    astra_kpeer_handle hKPeer)
{
    struct astra_ioctl_peer_close_data peerCloseData;
    int err = 0;

    peerCloseData.hPeer = hKPeer;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_PEER_CLOSE,
        (void *)&peerCloseData);

    if (err)
        return -EIO;

    if (peerCloseData.retVal)
        return peerCloseData.retVal;

    return 0;
}

int _astra_ioctl_msg_send(
    astra_kpeer_handle hKPeer,
    const void *pMsg,
    size_t msgLen)
{
    struct astra_ioctl_msg_send_data msgSendData;
    int err = 0;

    msgSendData.hPeer = hKPeer;
    msgSendData.pMsg = pMsg;
    msgSendData.msgLen = msgLen;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_MSG_SEND,
        (void *)&msgSendData);

    if (err)
        return -EIO;

    if (msgSendData.retVal)
        return msgSendData.retVal;

    return 0;
}

int _astra_ioctl_msg_receive(
    astra_kclient_handle hKClient,
    astra_kpeer_handle *phKPeer,
    void *pMsg,
    size_t *pMsgLen,
    int timeout)
{
    struct astra_ioctl_msg_receive_data msgReceiveData;
    int err = 0;

    msgReceiveData.hClient = hKClient;
    msgReceiveData.pMsg = pMsg;
    msgReceiveData.msgLen = *pMsgLen;
    msgReceiveData.timeout = timeout;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_MSG_RECEIVE,
        (void *)&msgReceiveData);

    if (err)
        return -EIO;

    if (msgReceiveData.retVal)
        return msgReceiveData.retVal;

    *phKPeer = msgReceiveData.hPeer;
    *pMsgLen = msgReceiveData.msgLen;
    return 0;
}

int _astra_ioctl_mem_alloc(
    astra_kclient_handle hKClient,
    size_t size,
    uint32_t *pBuffOffset)
{
    struct astra_ioctl_mem_alloc_data memAllocData;
    int err = 0;

    memAllocData.hClient = hKClient;
    memAllocData.size = size;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_MEM_ALLOC,
        (void *)&memAllocData);

    if (err)
        return -EIO;

    if (memAllocData.retVal)
        return memAllocData.retVal;

    *pBuffOffset = memAllocData.buffOffset;
    return 0;
}

int _astra_ioctl_mem_free(
    astra_kclient_handle hKClient,
    uint32_t buffOffset)
{
    struct astra_ioctl_mem_free_data memFreeData;
    int err = 0;

    memFreeData.hClient = hKClient;
    memFreeData.buffOffset = buffOffset;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_MEM_FREE,
        (void *)&memFreeData);

    if (err)
        return -EIO;

    if (memFreeData.retVal)
        return memFreeData.retVal;

    return 0;
}

int _astra_ioctl_file_open(
    astra_kclient_handle hKClient,
    const char *pPath,
    int flags,
    astra_kfile_handle *phKFile)
{
    struct astra_ioctl_file_open_data fileOpenData;
    int err = 0;

    fileOpenData.hClient = hKClient;
    strncpy(fileOpenData.path, pPath, ASTRA_PATH_LEN_MAX-1);
    /* Coverity:Explicitly null terminate string */
    fileOpenData.path[ASTRA_PATH_LEN_MAX-1] = '\0';

    fileOpenData.flags = flags;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_FILE_OPEN,
        (void *)&fileOpenData);

    if (err)
        return -EIO;

    if (fileOpenData.retVal)
        return fileOpenData.retVal;

    *phKFile = fileOpenData.hFile;
    return 0;
}

int _astra_ioctl_file_close(
    astra_kfile_handle hKFile)
{
    struct astra_ioctl_file_close_data fileCloseData;
    int err = 0;

    fileCloseData.hFile = hKFile;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_FILE_CLOSE,
        (void *)&fileCloseData);

    if (err)
        return -EIO;

    if (fileCloseData.retVal)
        return fileCloseData.retVal;

    return 0;
}

int _astra_ioctl_file_write(
    astra_kfile_handle hKFile,
    astra_paddr_t paddr,
    size_t *pBytes)
{
    struct astra_ioctl_file_write_data fileWriteData;
    int err = 0;

    fileWriteData.hFile = hKFile;
    fileWriteData.paddr = paddr;
    fileWriteData.bytes = *pBytes;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_FILE_WRITE,
        (void *)&fileWriteData);

    if (err)
        return -EIO;

    if (fileWriteData.retVal)
        return fileWriteData.retVal;

    *pBytes = fileWriteData.bytes;
    return 0;
}

int _astra_ioctl_file_read(
    astra_kfile_handle hKFile,
    astra_paddr_t paddr,
    size_t *pBytes)
{
    struct astra_ioctl_file_read_data fileReadData;
    int err = 0;

    fileReadData.hFile = hKFile;
    fileReadData.paddr = paddr;
    fileReadData.bytes = *pBytes;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_FILE_READ,
        (void *)&fileReadData);

    if (err)
        return -EIO;

    if (fileReadData.retVal)
        return fileReadData.retVal;

    *pBytes = fileReadData.bytes;
    return 0;
}

int _astra_ioctl_uapp_coredump(
    astra_kuapp_handle hKUapp)
{
    struct astra_ioctl_uapp_coredump_data uappCoredumpData;
    int err = 0;

    uappCoredumpData.hUapp = hKUapp;

    err = ioctl(
        pAstra->fd,
        ASTRA_IOCTL_UAPP_COREDUMP,
        (void *)&uappCoredumpData);

    if (err)
        return -EIO;

    if (uappCoredumpData.retVal)
        return uappCoredumpData.retVal;

    return 0;
}
