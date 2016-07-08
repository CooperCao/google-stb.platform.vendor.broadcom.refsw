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

#ifndef TZIOC_H
#define TZIOC_H

#include <cstdint>

#include "arm/spinlock.h"
#include "tztask.h"

/* compiler switches */
#define TZIOC_MSG_ECHO  1

/* TzIoc class */
class TzIoc {
public:
    /* defines and types */
    static const uint32_t TZIOC_DEV_DEFAULT_FD  = 0xffff;

    /* member classes */
    class TzIocMsg;
    class TzIocMem;
    class TzIocClient;
    class TzIocIoctl;

public:
    /* constructor / destructor / copy op */
    TzIoc() = delete;
    ~TzIoc() = delete;
    TzIoc& operator = (const TzIoc&) = delete;

public:
    /* public methods */
    static void init(void *devTree);

    static void proc();
    static void notify();

    static void cleanupTask(TzTask *pTask);

    static int ioctl(uint32_t fd, uint32_t cmd, uint32_t arg);

    inline static uint32_t offset2addr(uint32_t offset) {
        if (offset < smemSize)
            return offset + (uint32_t)psmem;
        else
            return (uint32_t)-1;
    }

    inline static uint32_t addr2offset(uint32_t addr) {
        if (addr >= (uint32_t)psmem &&
            addr <  (uint32_t)psmem + smemSize)
            return addr - (uint32_t)psmem;
        else
            return (uint32_t)-1;
    }

    /* in case physical address, not offset, is used */
    inline static uint32_t vaddr2paddr(uint32_t vaddr) {
        uint32_t offset = addr2offset(vaddr);
        if (offset != -1)
            return smemStart + offset;
        else
            return (uint32_t)-1;
    }

    inline static uint32_t paddr2vaddr(uint32_t paddr) {
        uint32_t offset = paddr - smemStart;
        return offset2addr(offset);
    }

private:
    /* private methods */
    static int sysMsgProc(
        struct tzioc_msg_hdr *pHdr);

#if TZIOC_MSG_ECHO
    static int echoMsgProc(
        struct tzioc_msg_hdr *pHdr);
#endif

public:
    /* public data */

private:
    /* private data */

    /* parameters from device tree */
    static uint32_t smemStart;
    static uint32_t smemSize;
    static uint32_t sysIrq;

    /* shared memory */
    static struct tzioc_shared_mem *psmem;

    /* system client */
    static struct tzioc_client *psysClient;

    /* spinlock for data access */
    static spinlock_t lock;

    /* peer state */
    static bool peerUp;
};

#endif /* TZIOC_H */
