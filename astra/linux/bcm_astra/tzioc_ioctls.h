/******************************************************************************
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
 *****************************************************************************/

#ifndef TZIOC_IOCTLS_H
#define TZIOC_IOCTLS_H

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif
#include <asm/ioctl.h>

#ifndef TZIOC_CLIENT_DEFINES
#define TZIOC_CLIENT_DEFINES

#define TZIOC_CLIENT_NAME_LEN_MAX       32
#define TZIOC_CLIENT_PATH_LEN_MAX       128

#endif /* TZIOC_CLIENT_DEFINES */

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

#define TZIOC_IOCTL_MAGIC               0xae /* 'T'(0x54) + 'Z'(0x5a) */

enum {
    TZIOC_IOCTL_FIRST = _IO(TZIOC_IOCTL_MAGIC, 0),

    TZIOC_IOCTL_CLIENT_OPEN = TZIOC_IOCTL_FIRST,
    TZIOC_IOCTL_CLIENT_CLOSE,
    TZIOC_IOCTL_MSG_SEND,
    TZIOC_IOCTL_MEM_ALLOC,
    TZIOC_IOCTL_MEM_FREE,
    TZIOC_IOCTL_CALL_SMC,

    TZIOC_IOCTL_LAST
};

struct tzioc_ioctl_client_open_data {
    int retVal;

    char name[TZIOC_CLIENT_NAME_LEN_MAX];
    uint32_t msgQ;

    uintptr_t hClient;
    uint32_t id;
    uint32_t smemStart;
    uint32_t smemSize;
};

struct tzioc_ioctl_client_close_data {
    int retVal;

    uintptr_t hClient;
};

struct tzioc_ioctl_client_getid_data {
    int retVal;

    uintptr_t hClient;
    char name[TZIOC_CLIENT_NAME_LEN_MAX];

    uint32_t id;
};

struct tzioc_ioctl_msg_send_data {
    int retVal;

    uintptr_t hClient;
    struct tzioc_msg_hdr hdr;
    uintptr_t payloadAddr;
};

struct tzioc_ioctl_mem_alloc_data {
    int retVal;

    uintptr_t hClient;
    uint32_t size;

    uint32_t buffOffset;
};

struct tzioc_ioctl_mem_free_data {
    int retVal;

    uintptr_t hClient;
    uint32_t buffOffset;
};

struct tzioc_ioctl_call_smc_data {
    int retVal;

    uintptr_t hClient;
    uint32_t callnum;
};

#endif /* TZIOC_IOCTLS_H */
