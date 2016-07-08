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

#ifndef TZIOC_MEM_H
#define TZIOC_MEM_H

#include <cstdint>

#include "tzioc.h"
#include "tzioc_common.h"
#include "arm/spinlock.h"
#include "utils/vector.h"

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

class TzIoc::TzIocMem
{
public:
    /* defines and types */

    /* physical address mapping record */
    struct PaddrMap {
        struct tzioc_client *pClient;
        uint32_t ulPaddr;
        uint32_t ulVaddr;
        uint32_t ulSize;
    };

public:
    /* static methods */
    static void init(void *devTree);

    static void *alloc(
        struct tzioc_client *pClient,
        uint32_t ulSize);

    static void free(
        struct tzioc_client *pClient,
        void *pBuff);

    static int mapPaddr(
        struct tzioc_client *pClient,
        uint32_t ulPaddr,
        uint32_t ulSize,
        uint32_t ulFlags,
        uint32_t *pulVaddr);

    static int unmapPaddr(
        struct tzioc_client *pClient,
        uint32_t ulPaddr,
        uint32_t ulSize);

    static int mapPaddrs(
        struct tzioc_client *pClient,
        uint8_t ucCount,
        struct tzioc_mem_region *pRegions);

    static int unmapPaddrs(
        struct tzioc_client *pClient,
        uint8_t ucCount,
        struct tzioc_mem_region *pRegions);

public:
    /* public methods */

public:
    /* constructor / destructor / copy op */
    TzIocMem() = delete;
    ~TzIocMem() = delete;
    TzIocMem& operator = (const TzIocMem&) = delete;

private:
    /* private methods */

public:
    /* public data */

private:
    /* private data */

    /* spinlock for data access */
    static spinlock_t lock;

    /* mem control block */
    static struct tzioc_mem_cb memCB;

    /* physical address mappings */
    static tzutils::Vector<PaddrMap> paddrMaps;

}; /* class TzIoc::TzIocMem */

#endif /* TZIOC_MEM_H */
