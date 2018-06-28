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


#ifndef TZIOC_DRV_H
#define TZIOC_DRV_H

#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/printk.h>

/* compiler switches */
#define KERNEL_IPI_PATCH        1
#define KERNEL_BCM_MQ_PATCH     0
#define IOREMAP_SHARED_MEM      0
#define CPUTIME_ACCOUNTING      0
#define CPUTIME_WINDOW_SIZE     5
#define TZIOC_DEV_SUPPORT       0
#define TZIOC_MSG_ECHO          0

/* utility macros */
#ifndef UNUSED
#define UNUSED(x) (void)x
#endif

#ifndef LOGI
#define LOGD(format, ...) pr_devel("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGW(format, ...) pr_warn ("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGE(format, ...) pr_err  ("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGI(format, ...) pr_info ("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#endif

/* TZIOC config */
struct tzioc_config {
    uint32_t smemSize;
    uint32_t pmemSize;
};

struct tzioc_status {
    bool up;
    bool secure;
};

/* TZIOC device */
struct tzioc_device {
#if TZIOC_DEV_SUPPORT
    /* misc device */
    struct miscdevice *mdev;
#endif /* TZIOC_DEV_SUPPORT */

    /* parameters from device tree */
    uintptr_t smemStart;
    uintptr_t smemSize;
    uint32_t sysIrq;

    /* shared memory */
    struct tzioc_shared_mem *psmem;

    /* system IRQ work */
    struct work_struct sysIrqWork;

    /* spinlock for data access */
    spinlock_t lock;

    /* ioctl module */
    struct tzioc_ioctl_module *pIoctlMod;

    /* msg module */
    struct tzioc_msg_module *pMsgMod;

    /* mem module */
    struct tzioc_mem_module *pMemMod;

    /* client module */
    struct tzioc_client_module *pClientMod;

    /* system client */
    struct tzioc_client *pSysClient;

    /* astra device */
    struct astra_device *adev;

    /* peer state */
    bool peerUp;
    bool secure;

    uint32_t pmemSize;
};

/* exported functions */
int _tzioc_call_smc(uint32_t ulCallnum);

uintptr_t _tzioc_offset2addr(uintptr_t ulOffset);
uintptr_t _tzioc_addr2offset(uintptr_t ulAddr);

/* in case physical address, not offset, is used */
uintptr_t _tzioc_vaddr2paddr(uintptr_t ulVaddr);
uintptr_t _tzioc_paddr2vaddr(uintptr_t ulPaddr);

int _tzioc_config_get(struct tzioc_config *pConfig);
int _tzioc_status_get(struct tzioc_status *pStatus);

/* global variables */
extern struct tzioc_device *tdev;

#endif /* TZIOC_DRV_H */
