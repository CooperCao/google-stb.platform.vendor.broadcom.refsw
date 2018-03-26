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

#ifndef _MONITOR_H_
#define _MONITOR_H_

#ifndef __ASSEMBLY__

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <cassert.h>
#include <mon_printf.h>

#include "config.h"
#include "error.h"

#define __bootstrap             __attribute__((__section__(".text.bootstrap")))
#define __boot_params           __attribute__((__section__(".boot_params")))
#define __xlat_tables           __attribute__((__section__(".xlat_tables")))
#define __coherent_ram          __attribute__((__section__(".coherent_ram")))

#define __align4                __attribute__((__aligned__(4)))
#define __align8                __attribute__((__aligned__(8)))
#define __align16               __attribute__((__aligned__(16)))
#define __align32               __attribute__((__aligned__(32)))
#define __align64               __attribute__((__aligned__(64)))
#define __align_page            __attribute__((__aligned__(PAGE_SIZE)))
#define __align_cache           __attribute__((__aligned__(CACHE_LINE_SIZE)))

#define __unused                __attribute__((__unused__))
#define __dead2                 __attribute__((__noreturn__))

#define UNUSED(x)               ((void)(x))
#define MMIO32(addr)            (*(volatile uint32_t *)(addr))
#define MMIO64(addr)            (*(volatile uint64_t *)(addr))

#define SYS_HALT(void) __asm__ volatile("b .")

#define ASSERT(cond) \
if (!(cond)) { \
    ERR_MSG("%s:%d - Assertion failed", __FUNCTION__, __LINE__); \
    SYS_HALT(); \
}

#ifdef DEBUG
#define DBG_ASSERT(cond) ASSERT(cond)
#else
#define DBG_ASSERT(cond)
#endif

#ifdef DHALT
#define DBG_HALT(void)  __asm__ volatile("b .")
#else
#define DBG_HALT(void)
#endif

#else /* __ASSEMBLY__ */

#ifdef DHALT
.macro dbg_halt
	b .
.endm
#else
.macro dbg_halt
.endm
#endif

#endif /* __ASSEMBLY__ */

#ifndef SECURE
#define SECURE			0
#define NON_SECURE		1
#endif

#endif /* _MONITOR_H_ */
