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

#ifndef LIBTZIOC_API_H
#define LIBTZIOC_API_H

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TZIOC_CLIENT_DEFINES
#define TZIOC_CLIENT_DEFINES

#define TZIOC_CLIENT_NAME_LEN_MAX       32
#define TZIOC_CLIENT_PATH_LEN_MAX       128

#endif /* TZIOC_CLIENT_DEFINES */

/*****************************************************************************
Summary:
    TZIOC msg.

Description:
    Each TZIOC msg contains a header and a immediately following payload.

    - The msg type defines and the assocaited payload formats are client
      specific. It is expected that the receiving peer client knows how to
      interpret the msgs.

    - The originating client ID is obtained through tzioc_client_open().

    - The destination client ID is obtained through tzioc_client_connect().
******************************************************************************/

#ifndef TZIOC_MSG_DEFINES
#define TZIOC_MSG_DEFINES

typedef struct tzioc_msg_hdr {
    uint8_t  ucType;                    /* msg type */
    uint8_t  ucOrig;                    /* originating client ID */
    uint8_t  ucDest;                    /* destination client ID */
    uint8_t  ucSeq;                     /* msg sequence number */
    uint32_t ulLen;                     /* byte length of msg payload */
} tzioc_msg_hdr;

#define TZIOC_MSG_PAYLOAD(pHdr)         ((uint8_t *)pHdr + sizeof(tzioc_msg_hdr))

#define TZIOC_MSG_SIZE_MAX              1024
#define TZIOC_MSG_PAYLOAD_MAX           (TZIOC_MSG_SIZE_MAX - \
                                         sizeof(struct tzioc_msg_hdr))
#endif /* TZIOC_MSG_DEFINES */

/*****************************************************************************
Summary:
    TZIOC mem region.

Description:
    This struct describes the mapping of a memory region.
******************************************************************************/

#ifndef TZIOC_MEM_REGIONS
#define TZIOC_MEM_REGIONS

typedef struct tzioc_mem_region {
    uintptr_t ulPaddr;
    uintptr_t ulVaddr;
    uint32_t ulSize;
    uint32_t ulFlags;
} tzioc_mem_region;

#define TZIOC_MEM_DEVICE                (0x1 << 0)
#define TZIOC_MEM_RD_ONLY               (0x1 << 1)
#define TZIOC_MEM_NO_EXEC               (0x1 << 2)

#define TZIOC_MEM_REGION_MAX            32

#endif /* TZIOC_MEM_REGION */

/*****************************************************************************
Summary:
    TZIOC client handle.

Description:
    This is an opaque handle of local TZIOC client that is created with
    tzioc_client_open().

    It is freed with tzioc_client_close().
******************************************************************************/

typedef uintptr_t tzioc_client_handle;

/*****************************************************************************
Summary:
    Open and register a local TZIOC client.

Description:
    Each client must have a unique name in its own world. If multiple client
    tasks try to register with the same local client name, only the first one
    can register with TZIOC successfully.

    The Posix msg queue is used to receive incoming TZIOC msgs.

Input:
    pName - pointer to local TZIOC client name string

Output:
    *pMsgQ - msg queue descriptor
    *pId - local TZIOC client ID

Returns:
    Handle to local TZIOC client
    NULL - failure
******************************************************************************/

tzioc_client_handle tzioc_client_open(
    const char *pName,
    int *pMsgQ,
    uint8_t *pId);

/*****************************************************************************
Summary:
    Unregister and close a local TZIOC client.

Description:

Input:
    hClient - local TZIOC client handle

Output:

Returns:

******************************************************************************/

void tzioc_client_close(
    tzioc_client_handle hClient);

/*****************************************************************************
Summary:
    Get ID of a local TZIOC client by name.

Description:

Input:
    hClient - local (calling) TZIOC client handle
    pName - pointer to local (querried) TZIOC client name string
    ulPid - if specified (non-zero), pid of the process which opens the client

Output:
    *pId - local (querried) TZIOC client ID

Returns:
    0 - success
    Errno - failure
******************************************************************************/

int tzioc_client_getid(
    tzioc_client_handle hClient,
    const char *pName,
    uint32_t ulPid,
    uint8_t *pId);

/*****************************************************************************
Summary:
    Send a TZIOC msg.

Description:
    This call is non-blocking.

Input:
    hClient - local TZIOC client handle
    pHdr - pointer to TZIOC msg header

Output:

Returns:
    0 - success
    Errno - failure
******************************************************************************/

int tzioc_msg_send(
    tzioc_client_handle hClient,
    tzioc_msg_hdr *pHdr);

/*****************************************************************************
Summary:
    Receive a TZIOC msg.

Description:
    If the timeout value is zero, this call return error -ENOMSG if there
    is no received msg.

    If the timeout value is non-zero, this call may block if there is no
    received msg.

Input:
    hClient - local TZIOC client handle
    pHdr - pointer to TZIOC msg header
    ulSize - size of TZIOC msg buffer
    ulTimeout - timeout value in ms

Output:

Returns:
    0 - success
    Errno - failure
******************************************************************************/

int tzioc_msg_receive(
    tzioc_client_handle hClient,
    tzioc_msg_hdr *pHdr,
    uint32_t ulSize,
    uint32_t ulTimeout);

/*****************************************************************************
Summary:
    Allocate a buffer from TZIOC shared memory.

Description:

Input:
    hClient - local TZIOC client handle
    ulSize - size of buffer to be allocated

Output:

Returns:
    Pointer to allocated buffer - success;
    NULL - failure
******************************************************************************/

void *tzioc_mem_alloc(
    tzioc_client_handle hClient,
    uint32_t ulSize);

/*****************************************************************************
Summary:
    Free a buffer previously allocated from TZIOC shared memory.

Description:

Input:
    hClient - local TZIOC client handle
    pBuff - pointer to buffer to be freed

Output:

Returns:

******************************************************************************/

void tzioc_mem_free(
    tzioc_client_handle hClient,
    void *pBuff);

/*****************************************************************************
Summary:
    Convert TZIOC shared memory offset to virtual address.

Description:
    The address must lie within the TZIOC shared memory.

Input:
    hClient - local TZIOC client handle
    ulOffset - offset in TZIOC shared memory

Output:

Returns:
    Virtual address - success
    (uint32_t)-1 - failure
******************************************************************************/

uintptr_t tzioc_offset2vaddr(
    tzioc_client_handle hClient,
    uintptr_t ulOffset);

/*****************************************************************************
Summary:
    Convert virtual address to offset in TZIOC shared memory.

Description:
    The address must lie within the TZIOC shared memory.

Input:
    hClient - local TZIOC client handle
    ulVaddr - virtual address

Output:

Returns:
    TZIOC shared memory offset - success
    (uint32_t)-1 - failure
******************************************************************************/

uintptr_t tzioc_vaddr2offset(
    tzioc_client_handle hClient,
    uintptr_t ulVaddr);

/*****************************************************************************
Summary:
    Map a memory region in virtual address space.

Description:

Input:
    hClient - local TZIOC client handle
    ulPaddr - physical address of memory region
    ulSize - size of memory region
    ulFlags - bitmap flags

Output:

Returns:
    Pointer to memory region in virtual address space - success;
    NULL - failure
******************************************************************************/

void *tzioc_map_paddr(
    tzioc_client_handle hClient,
    uintptr_t ulPaddr,
    uint32_t ulSize,
    uint32_t ulFlags);

/*****************************************************************************
Summary:
    Unmap a memory region from virtual address space.

Description:

Input:
    hClient - local TZIOC client handle
    ulPaddr - physical address of memory region
    ulSize - size of physical memory region

Output:

Returns:
******************************************************************************/

void tzioc_unmap_paddr(
    tzioc_client_handle hClient,
    uintptr_t ulPaddr,
    uint32_t ulSize);

/*****************************************************************************
Summary:
    Map multiple physical memory regions to virtual address space.

Description:
    This function can be used to map multiple physical memory regions at once
    with only one flush of cache after the page table has been modified.

    For practical reason, each time this function is called, the number of
    phsyical memory regions is limited to TZIOC_MEM_REGION_MAX.

Input:
    hClient - local TZIOC client handle
    ulCount - number of memory regions
    pRegions - pointer to array of memoery regions (physical addresses, sizes)

Output:
    pRegions - pointer to array of memory regions with virtual addresses

Returns:
    0 - success
    Errno - failure
******************************************************************************/

int tzioc_map_paddrs(
    tzioc_client_handle hClient,
    uint8_t ucCount,
    tzioc_mem_region *pRegions);

/*****************************************************************************
Summary:
    Unmap multiple physical memory regions from virtual address space.

Description:

Input:
    hClient - local TZIOC client handle
    ulCount - number of memory regions
    pRegions - pointer to array of memoery regions

Output:

Returns:
    0 - success
    Errno - failure
******************************************************************************/

int tzioc_unmap_paddrs(
    tzioc_client_handle hClient,
    uint8_t ucCount,
    tzioc_mem_region *pRegions);

/*****************************************************************************
Summary:
    Generate SGI (Software Generated Interrupt) IRQ to the non-secure world.

Description:

Input:
    hClient - local TZIOC client handle
    ulIrq - interrupt request number

Output:

Returns:
    0 - success
    Errno - failure
******************************************************************************/

int tzioc_gen_irq(
    tzioc_client_handle hClient,
    uint32_t ulIrq);

#ifdef __cplusplus
}
#endif

#endif /* LIBTZIOC_API_H */
