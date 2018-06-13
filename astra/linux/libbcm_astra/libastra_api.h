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

#ifndef LIBASTRA_API_H
#define LIBASTRA_API_H

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ASTRA_WAIT_DEFINES
#define ASTRA_WAIT_DEFINES
#define ASTRA_WAIT_NONE         0
#define ASTRA_WAIT_FOREVER     -1
#endif

#ifndef ASTRA_NAME_DEFINES
#define ASTRA_NAME_DEFINES
#define ASTRA_NAME_LEN_MAX      32
#define ASTRA_PATH_LEN_MAX      128
#endif

#ifndef ASTRA_MSG_DEFINES
#define ASTRA_MSG_DEFINES
#define ASTRA_MSG_LEN_MAX       (1024-8)
#endif

/*****************************************************************************
Summary:
    Astra physical address.

Description:
    Only physical address can be passed between diffferent worlds. The size
    of physical address depends on the implementaion of the system (e.g.
    32-bit, 64-bit).
******************************************************************************/

typedef uint64_t astra_paddr_t;

/*****************************************************************************
Summary:
    Astra local client handle.

Description:
    This is an opaque handle of astra local client that is created with
    astra_client_open().

    It is freed with astra_client_close().
******************************************************************************/

typedef struct astra_client *astra_client_handle;

/*****************************************************************************
Summary:
    Astra user app handle.

Description:
    This is an opaque handle of astra user app in the other world that is
    created with astra_uapp_open().

    It is freed with astra_uapp_close().
******************************************************************************/

typedef struct astra_uapp *astra_uapp_handle;

/*****************************************************************************
Summary:
    Astra peer client handle.

Description:
    This is an opaque handle of astra peer client that is created with
    astra_peer_open().

    It is freed with astra_peer_close().
******************************************************************************/

typedef struct astra_peer *astra_peer_handle;

/*****************************************************************************
Summary:
    Astra file handle.

Description:
    This is an opaque handle of astra file that is created with
    astra_file_open().

    It is freed with astra_file_close().
******************************************************************************/

typedef struct astra_file *astra_file_handle;

/*****************************************************************************
Summary:
    Astra version.

Description:
    This struct contains astra version information.

******************************************************************************/

typedef struct astra_version {
    uint8_t major;
    uint8_t minor;
    uint16_t build;
} astra_version;

/*****************************************************************************
Summary:
    Astra configurations.

Description:
    This struct describes astra configurations. It is filled by calling
    astra_config_get().

    smemSize - size of shared memory between two worlds
    pmemSize - size of astra private memory

******************************************************************************/

typedef struct astra_config {
    uint32_t smemSize;
    uint32_t pmemSize;
} astra_config;

/*****************************************************************************
Summary:
    Astra status.

Description:
    This struct describes astra operational status. It is filled by calling
    astra_status_get().

    up - astra is up and running

******************************************************************************/

typedef struct astra_status {
    bool up;
} astra_status;

/*****************************************************************************
Summary:
    Astra event types.

Description:
    These are event types indicated when the client event callback function
    is called.
******************************************************************************/

typedef enum astra_event {
    ASTRA_EVENT_MSG_RECEIVED,
    ASTRA_EVENT_UAPP_EXIT,
    ASTRA_EVENT_MAX
} astra_event;

/*****************************************************************************
Summary:
    Astra SMC calling code.

Description:
    These are Astra defined SMC calling codes. They are NOT the actual SMC
    calling codes conforming to ARM SMC calling conventions.
******************************************************************************/

typedef enum astra_smc_code {
    ASTRA_SMC_CODE_SWITCH,
    ASTRA_SMC_CODE_MAX
} astra_smc_code;

/*****************************************************************************
Summary:
    Prototype of astra client event callback function.

Description:
    This is the callback function that a client optionally registers with
    astra at the time of astra_client_open(). This function is called from
    the context of astra and should NOT block.

    pEventData points to the data associated with the particular callback
    event. In case that event data is NOT used for the paricular event,
    pEventData is set to NULL. The following event data are used:

        ASTRA_EVENT_UAPP_EXIT   - astra_uapp_handle

    pPrivData is private data of the client. It is passed in by the client
    st the time of astra_client_open(). It is returned when the client event
    callback function is called.
******************************************************************************/

typedef void (*astra_event_callback)(
    astra_event event,
    void *pEventData,
    void *pPrivData);

/*****************************************************************************
Summary:
    Initialize astra in a user space process.

Description:
    This function initializes the astra library in a user space process.
    If it is NOT called explicitly, any of the following astra API calls
    would also trigger astra initialization:

    - astra_version_get()
    - astra_config_get()
    - astra_status_get()
    - astra_client_open()

Input:

Output:

Returns:
    0 - success
    errno - failure
******************************************************************************/

int astra_init(void);

/*****************************************************************************
Summary:
    Deinitialize astra in a user space process.

Description:
    This function deinitializes the astra library in a user space process.

Input:

Output:

Returns:
    0 - success
    errno - failure
******************************************************************************/

int astra_deinit(void);

/*****************************************************************************
Summary:
    Get astra version.

Description:

Input:
    pVersion - pointer to astra version

Output:
    *pVersion - astra version

Returns:
    0 - success
    errno - failure
******************************************************************************/

int astra_version_get(astra_version *pVersion);

/*****************************************************************************
Summary:
    Get astra configurations.

Description:

Input:
    pConfig - pointer to astra configurations

Output:
    *pConfig - astra configurations

Returns:
    0 - success
    errno - failure
******************************************************************************/

int astra_config_get(astra_config *pConfig);

/*****************************************************************************
Summary:
    Get astra operational status.

Description:

Input:
    pStatus - pointer to astra operational status

Output:
    *pStatus - astra operational status

Returns:
    0 - success
    errno - failure
******************************************************************************/

int astra_status_get(astra_status *pStatus);

/*****************************************************************************
Summary:
    Open and register a astra local client.

Description:
    Each local client must have a unique name in its own world. If multiple
    clients try to register with the same client name, only the first one can
    register with astra successfully.

    A client can register a event callback function along with a pointer to
    its private data. The callback function is called from the context of
    astra.

Input:
    pName - pointer to astra local client name string
    pCallback - pointer to client event callback function
    pPrivData - pointer to client private data

Output:

Returns:
    handle to astra local client
    NULL - failure
******************************************************************************/

astra_client_handle astra_client_open(
    const char *pName,
    astra_event_callback pCallback,
    void *pPrivData);

/*****************************************************************************
Summary:
    Unregister and close a astra local client.

Description:

Input:
    hClient - astra local client handle

Output:

Returns:

******************************************************************************/

void astra_client_close(
    astra_client_handle hClient);

/*****************************************************************************
Summary:
    Open a astra user app in the other world.

Description:
    If the named user app is already running in the other world, this function
    returns success. Otherwise, the named user app is spawn with given the
    executable name.

Input:
    hClient - astra local client handle
    pName - pointer to user app name string
    pPath - pointer to user app executable file path

Output:

Returns:
    handle to astra user app.
******************************************************************************/

astra_uapp_handle astra_uapp_open(
    astra_client_handle hClient,
    const char *pName,
    const char *pPath);

/*****************************************************************************
Summary:
    Close a astra user app in the other world.

Description:
    This func only has effect if the user app in the other world has been
    previously opened by the same calling local client.

    The reference count on the user app is decremented. The user app would
    be terminated if the reference count reaches zero.

Input:
    hUapp - astra user app handle

Output:

Returns:

******************************************************************************/

void astra_uapp_close(
    astra_uapp_handle hUapp);

/*****************************************************************************
Summary:
    Open connection to a astra peer client in the other world.

Description:
    A local client must call this function to open and connect to a peer
    client in the other world before communicating with it.

Input:
    hUapp - astra user app handle
    pName - pointer to astra peer client name string

Output:

Returns:
    handle to astra peer client
    NULL - failure
******************************************************************************/

astra_peer_handle astra_peer_open(
    astra_uapp_handle hUapp,
    const char *pName);

/*****************************************************************************
Summary:
    Close connection to a astra peer client in the other world.

Description:

Input:
    hPeer - astra peer client handle

Output:

Returns:

******************************************************************************/

void astra_peer_close(
    astra_peer_handle hPeer);

/*****************************************************************************
Summary:
    Send a astra msg.

Description:
    This call is non-blocking.

Input:
    hPeer - astra peer client handle
    pMsg - pointer to msg data
    msgLen - msg length in bytes

Output:

Returns:
    0 - success
    errno - failure
******************************************************************************/

int astra_msg_send(
    astra_peer_handle hPeer,
    const void *pMsg,
    uint32_t msgLen);

/*****************************************************************************
Summary:
    Receive a astra msg.

Description:
    If the timeout value is ASTRA_WAIT_NONE, this call return error -ENOMSG if
    there is no received msg.

    If the timeout value is non-zero, this call may block if there is no
    received msg.

Input:
    hClient - astra local client handle
    phPeer - pointer to astra peer client handle
    pMsg - pointer to msg buffer
    pMsgLen - pointer to msg buffer length in bytes (as input)
    timeout - timeout value in ms

Output:
    *phPeer - astra peer client handle
    *pMsg - msg data
    *pMsgLen - msg data length in bytes (as output overriding input)

Returns:
    0 - success
    errno - failure
******************************************************************************/

int astra_msg_receive(
    astra_client_handle hClient,
    astra_peer_handle *phPeer,
    void *pMsg,
    uint32_t *pMsgLen,
    int timeout);

/*****************************************************************************
Summary:
    Allocate a buffer from astra shared memory.

Description:

Input:
    hClient - astra local client handle
    size - size of the allocation in bytes

Output:

Returns:
    pointer to allocated buffer - success;
    NULL - failure
******************************************************************************/

void *astra_mem_alloc(
    astra_client_handle hClient,
    uint32_t size);

/*****************************************************************************
Summary:
    Free a buffer previously allocated from astra shared memory.

Description:

Input:
    hClient - astra local client handle
    pBuffer - ponter to the buffer to be freed

Output:

Returns:

******************************************************************************/

void astra_mem_free(
    astra_client_handle hClient,
    void *pBuff);

/*****************************************************************************
Summary:
    Allocate a buffer from astra private memory.

Description:
    This function is used to allocate a buffer from astra private memory. The
    buffer is protected by memory range checker and is considered secure.

    The allocated buffer is guranteed to be contiguous in physical space.

Input:
    hClient - astra local client handle
    size - size of the allocation in bytes

Output:

Returns:
    physical address of allocated buffer - success;
    0 - failure
******************************************************************************/

astra_paddr_t astra_pmem_alloc(
    astra_client_handle hClient,
    uint32_t size);

/*****************************************************************************
Summary:
    Free a buffer previously allocated from astra private memory.

Description:

Input:
    hClient - astra local client handle
    paddr - physical address of the buffer to be freed

Output:

Returns:

******************************************************************************/

void astra_pmem_free(
    astra_client_handle hClient,
    astra_paddr_t paddr);

/*****************************************************************************
Summary:
    Convert astra shared memory offset to virtual address.

Description:
    The address must lie within the astra shared memory.

Input:
    hClient - astra local client handle
    offset - offset in astra shared memory

Output:

Returns:
    virtual address - success
    NULL - failure
******************************************************************************/

void *astra_offset2vaddr(
    astra_client_handle hClient,
    uint64_t offset);

/*****************************************************************************
Summary:
    Convert virtual address of a buffer to astra shared memory offset.

Description:
    The address must lie within the astra shared memory.

Input:
    hClient - astra local client handle
    pBuff - pointer to buffer

Output:

Returns:
    astra shared memory offset - success
    0 - failure
******************************************************************************/

uint64_t astra_vaddr2offset(
    astra_client_handle hClient,
    void *pBuff);

/*****************************************************************************
Summary:
    Open a astra file in the other world.

Description:
    Similar to the standard file open(), a local client calls this function
    to open or create a file in the other world.

Input:
    hClient - astra local client handle
    pName - pointer to astra peer client name string
    flags - open flags (fcntl.h defines with augments)

Output:

Returns:
    handle to astra file
    NULL - failure
******************************************************************************/

astra_file_handle astra_file_open(
    astra_client_handle hClient,
    const char *pPath,
    int flags);

/*****************************************************************************
Summary:
    Close a astra file in the other world.

Description:
    Similar to the standard file close(), a local client calls this function
    to close a file opened previously in the other world.

Input:
    hFile - astra file handle

Output:

Returns:

******************************************************************************/

void astra_file_close(
    astra_file_handle hFile);

/*****************************************************************************
Summary:
    Write to a astra file in the other world.

Description:
    Similar to the standard file write(), a local client calls this function
    to write to a file opened previously in the other world.

    Note that physical address of the data buffer is used. The buffer must
    be contiguous in physical space.

Input:
    hFile - astra file handle
    paddr - physical address of data to be write
    bytes - number of bytes to be written

Output:

Returns:
    number of bytes written
    errno - failure
******************************************************************************/

int astra_file_write(
    astra_file_handle hFile,
    astra_paddr_t paddr,
    uint32_t bytes);

/*****************************************************************************
Summary:
    Read from a astra file in the other world.

Description:
    Similar to the standard file read(), a local client calls this function
    to read from a file opened previously in the other world.

    Note that physical address of the data buffer is used. The buffer must
    be contiguous in physical space.

Input:
    hFile - astra file handle
    paddr - physical address of data to be filled
    bytes - number of bytes to be read

Output:

Returns:
    number of bytes read
    errno - failure
******************************************************************************/

int astra_file_read(
    astra_file_handle hFile,
    astra_paddr_t paddr,
    uint32_t bytes);

/*****************************************************************************
Summary:
    Call SMC to switch to the other world.

Description:

Input:
    hClient - astra local client handle
    code - SMC calling code

Output:

Returns:
    0 - success
    errno - failure
******************************************************************************/

int astra_call_smc(
    astra_client_handle hClient,
    astra_smc_code code);

void astra_uapp_coredump(
    astra_uapp_handle hUapp);


#ifdef __cplusplus
}
#endif

#endif /* LIBASTRA_API_H */
