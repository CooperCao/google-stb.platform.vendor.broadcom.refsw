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

	static int ioctl(uint32_t fd, unsigned long cmd, uintptr_t arg);

	inline static uintptr_t offset2addr(uintptr_t offset) {
		if (offset < smemSize)
			return offset + (uintptr_t)psmem;
		else
			return (uintptr_t)-1;
	}

	inline static uintptr_t addr2offset(uintptr_t addr) {
		if (addr >= (uintptr_t)psmem &&
			addr <  (uintptr_t)psmem + smemSize)
			return addr - (uintptr_t)psmem;
        else
            return (uintptr_t)-1;
    }

    /* in case physical address, not offset, is used */
    inline static uintptr_t vaddr2paddr(uintptr_t vaddr) {
        uintptr_t offset = addr2offset(vaddr);
        if (offset != -1)
            return smemStart + offset;
		else
			return (uintptr_t)-1;
	}

	inline static uintptr_t paddr2vaddr(uintptr_t paddr) {
		uintptr_t offset = paddr - smemStart;
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
	static uintptr_t smemStart;
	static uintptr_t smemSize;
	static uint32_t sysIrq;

    /* shared memory */
    static struct tzioc_shared_mem *psmem;

    /* system client */
    static struct tzioc_client *psysClient;

    /* spinlock for data access */
	static SpinLock lock;

    /* peer state */
    static bool peerUp;

    /* cpu start time (in ticks) */
    static uint64_t tzStartTicks;
    static uint64_t nwStartTicks;
};

#endif /* TZIOC_H */
