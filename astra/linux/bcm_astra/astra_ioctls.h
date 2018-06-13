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

#ifndef ASTRA_IOCTLS_H
#define ASTRA_IOCTLS_H

#ifdef __KERNEL__
/* kernel space */
#include <linux/types.h>
#include <asm/ioctl.h>
#include "astra_api.h"
#else
/* user space */
#include <stdint.h>
#include <asm/ioctl.h>
#include "libastra_api.h"
#endif

typedef astra_client_handle astra_kclient_handle;
typedef astra_uapp_handle astra_kuapp_handle;
typedef astra_peer_handle astra_kpeer_handle;
typedef astra_file_handle astra_kfile_handle;

#define ASTRA_IOCTL_MAGIC       0x94 /* 'A'(0x41) + 'S'(0x53) */

enum {
    ASTRA_IOCTL_FIRST = _IO(ASTRA_IOCTL_MAGIC, 0),

    ASTRA_IOCTL_EVENT_POLL = ASTRA_IOCTL_FIRST,
    ASTRA_IOCTL_EVENT_EXIT,

    ASTRA_IOCTL_VERSION_GET,
    ASTRA_IOCTL_CONFIG_GET,
    ASTRA_IOCTL_STATUS_GET,
    ASTRA_IOCTL_CALL_SMC,

    ASTRA_IOCTL_CLIENT_OPEN,
    ASTRA_IOCTL_CLIENT_CLOSE,

    ASTRA_IOCTL_UAPP_OPEN,
    ASTRA_IOCTL_UAPP_CLOSE,

    ASTRA_IOCTL_PEER_OPEN,
    ASTRA_IOCTL_PEER_CLOSE,

    ASTRA_IOCTL_MSG_SEND,
    ASTRA_IOCTL_MSG_RECEIVE,

    ASTRA_IOCTL_MEM_ALLOC,
    ASTRA_IOCTL_MEM_FREE,

    ASTRA_IOCTL_PMEM_ALLOC,
    ASTRA_IOCTL_PMEM_FREE,

    ASTRA_IOCTL_FILE_OPEN,
    ASTRA_IOCTL_FILE_CLOSE,
    ASTRA_IOCTL_FILE_READ,
    ASTRA_IOCTL_FILE_WRITE,

    ASTRA_IOCTL_UAPP_COREDUMP,

    ASTRA_IOCTL_LAST
};

struct astra_ioctl_event_poll_data {
    int retVal;

    uint64_t hClient;

    astra_event event;
    char eventData[16];
    uint32_t eventDataLen;
};

struct astra_ioctl_event_exit_data {
    int retVal;

    uint64_t hClient;
};

struct astra_ioctl_version_get_data {
    int retVal;

    struct astra_version version;
};

struct astra_ioctl_config_get_data {
    int retVal;

    struct astra_config config;
};

struct astra_ioctl_status_get_data {
    int retVal;

    struct astra_status status;
};

struct astra_ioctl_client_open_data {
    int retVal;

    char name[ASTRA_NAME_LEN_MAX];

    uint64_t hClient;
};

struct astra_ioctl_client_close_data {
    int retVal;

    uint64_t hClient;
};

struct astra_ioctl_uapp_open_data {
    int retVal;

    uint64_t hClient;
    char name[ASTRA_NAME_LEN_MAX];
    char path[ASTRA_PATH_LEN_MAX];

    uint64_t hUapp;
};

struct astra_ioctl_uapp_close_data {
    int retVal;

    uint64_t hUapp;
};

struct astra_ioctl_peer_open_data {
    int retVal;

    uint64_t hUapp;
    char name[ASTRA_NAME_LEN_MAX];

    uint64_t hPeer;
};

struct astra_ioctl_peer_close_data {
    int retVal;

    uint64_t hPeer;
};

struct astra_ioctl_msg_send_data {
    int retVal;

    uint64_t hPeer;
    uint64_t pMsg;
    uint32_t msgLen;
};

struct astra_ioctl_msg_receive_data {
    int retVal;

    uint64_t hClient;
    uint64_t pMsg;
    uint32_t msgLen;
    int timeout;

    uint64_t hPeer;
};

struct astra_ioctl_mem_alloc_data {
    int retVal;

    uint64_t hClient;
    uint32_t size;

    uint64_t buffOffset;
};

struct astra_ioctl_mem_free_data {
    int retVal;

    uint64_t hClient;
    uint64_t buffOffset;
};

struct astra_ioctl_call_smc_data {
    int retVal;

    uint64_t hClient;
    uint32_t code;
};

struct astra_ioctl_file_open_data {
    int retVal;

    uint64_t hClient;
    char path[ASTRA_PATH_LEN_MAX];
    int flags;

    uint64_t hFile;
};

struct astra_ioctl_file_close_data {
    int retVal;

    uint64_t hFile;
};

struct astra_ioctl_file_write_data {
    int retVal;

    uint64_t hFile;
    astra_paddr_t paddr;
    uint32_t bytes;
};

struct astra_ioctl_file_read_data {
    int retVal;

    uint64_t hFile;
    astra_paddr_t paddr;
    uint32_t bytes;
};

struct astra_ioctl_uapp_coredump_data {
    int retVal;

    uint64_t hUapp;
    astra_paddr_t paddr;
    uint32_t bytes;
};

#endif /* ASTRA_IOCTLS_H */
