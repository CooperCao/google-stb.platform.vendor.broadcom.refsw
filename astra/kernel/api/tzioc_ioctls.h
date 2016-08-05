/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifndef TZIOC_IOCTLS_H
#define TZIOC_IOCTLS_H

#include <stdint.h>

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

#ifndef TZIOC_MEM_REGIONS
#define TZIOC_MEM_REGIONS

typedef struct tzioc_mem_region {
    uint32_t ulPaddr;
    uint32_t ulVaddr;
    uint32_t ulSize;
    uint32_t ulFlags;
} tzioc_mem_region;

#define TZIOC_MEM_DEVICE                (0x1 << 0)
#define TZIOC_MEM_RD_ONLY               (0x1 << 1)
#define TZIOC_MEM_NO_EXEC               (0x1 << 2)

#define TZIOC_MEM_REGION_MAX            32

#endif /* TZIOC_MEM_REGION */

enum {
    TZIOC_IOCTL_FIRST = 0,

    TZIOC_IOCTL_CLIENT_OPEN = TZIOC_IOCTL_FIRST,
    TZIOC_IOCTL_CLIENT_CLOSE,
    TZIOC_IOCTL_CLIENT_GETID,
    TZIOC_IOCTL_MSG_SEND,
    TZIOC_IOCTL_MEM_ALLOC,
    TZIOC_IOCTL_MEM_FREE,
    TZIOC_IOCTL_CALL_SMC,
    TZIOC_IOCTL_MAP_PADDR,
    TZIOC_IOCTL_UNMAP_PADDR,
    TZIOC_IOCTL_MAP_PADDRS,
    TZIOC_IOCTL_UNMAP_PADDRS,

    TZIOC_IOCTL_LAST
};

struct tzioc_ioctl_client_open_data {
    int retVal;

    char name[TZIOC_CLIENT_NAME_LEN_MAX];
    uint32_t msgQ;

    uint32_t hClient;
    uint32_t id;
    uint32_t smemStart;
    uint32_t smemSize;
};

struct tzioc_ioctl_client_close_data {
    int retVal;

    uint32_t hClient;
};

struct tzioc_ioctl_client_getid_data {
    int retVal;

    uint32_t hClient;
    char name[TZIOC_CLIENT_NAME_LEN_MAX];
    uint32_t pid;

    uint32_t id;
};

struct tzioc_ioctl_msg_send_data {
    int retVal;

    uint32_t hClient;
    struct tzioc_msg_hdr hdr;
    uint32_t payloadAddr;
};

struct tzioc_ioctl_map_paddr_data {
    int retVal;

    uint32_t hClient;
    uint32_t paddr;
    uint32_t size;
    uint32_t flags;

    uint32_t vaddr;
};

struct tzioc_ioctl_unmap_paddr_data {
    int retVal;

    uint32_t hClient;
    uint32_t paddr;
    uint32_t size;
};

struct tzioc_ioctl_map_paddrs_data {
    int retVal;

    uint32_t hClient;
    uint32_t count;
    uint32_t regionsAddr;
};

struct tzioc_ioctl_unmap_paddrs_data {
    int retVal;

    uint32_t hClient;
    uint32_t count;
    uint32_t regionsAddr;
};

#endif /* TZIOC_IOCTLS_H */
