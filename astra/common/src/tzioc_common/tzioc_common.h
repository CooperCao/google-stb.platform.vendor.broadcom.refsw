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

#ifndef TZIOC_COMMON_H
#define TZIOC_COMMON_H

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#if defined(__linux__)

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/printk.h>

#ifndef LOGI
#define LOGD(format, ...) pr_info("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGV(format, ...) pr_info("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGW(format, ...) pr_warn ("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGE(format, ...) pr_err  ("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGI(format, ...) pr_info ("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#endif

#elif defined(__TZOS__)

#include <stdint.h>
#include <stdbool.h>
#include <bits/errno.h>
#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"

#ifndef LOGI
#define LOGD(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGV(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGW(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGE(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGI(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#endif

#else
#error "Unknown operating system!"
#endif

#include "tzioc_common_msg.h"
#include "tzioc_common_mem.h"
#include "tzioc_common_client.h"

/* shared memory */
struct tzioc_shared_mem {
    uint32_t ulMagic;

    /* msg rings */
    struct tzioc_ring_buf t2nRing;  /* TZOS to NWOS msg ring */
    struct tzioc_ring_buf n2tRing;  /* NWOS to TZOS msg ring */

    /* data to be shared */
};

#ifdef __cplusplus
extern "C" {
#endif

/* To be implemented by the specific OS */
extern uintptr_t _tzioc_offset2addr(uintptr_t ulOffset);
extern uintptr_t _tzioc_addr2offset(uintptr_t ulAddr);

#ifdef __cplusplus
}
#endif

#endif /* TZIOC_H */
