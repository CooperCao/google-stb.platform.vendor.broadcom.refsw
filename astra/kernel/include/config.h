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

#ifndef _B_TZ_CONFIG_H_
#define _B_TZ_CONFIG_H_

#define __bootstrap         __attribute__((section(".text.bootstrap")))
#define __bootstrap_data    __attribute__((section(".data.bootstrap")))

#define __init              __attribute__((section(".text.init")))
#define __init_data         __attribute__((section(".data")))


#define ALIGN4  __attribute__((__aligned__(4)))
#define ALIGN8  __attribute__((__aligned__(8)))
#define ALIGN16  __attribute__((__aligned__(16)))
#define ALIGN32  __attribute__((__aligned__(32)))
#define ALIGN64  __attribute__((__aligned__(64)))

#define ALIGN_PT  __attribute__((__aligned__(4096)))
#define ALIGN_16K  __attribute__((__aligned__(16384)))

#define NOINLINE __attribute__ ((noinline))

#define UNUSED(x) (void)(x)

#define COMPILER_BARRIER() asm volatile("":::"memory")

#define MAX_DT_SIZE_BYTES  (16 * 1024)

#define INIT_STACK_SIZE  (1024 * 4)
#define MON_STACK_SIZE   (1024 * 4)

#define MAX_NUM_CPUS   4

#define NUM_BOOTSTRAP_BLOCKS        32
#define MAX_NUM_PAGE_TABLE_BLOCKS   1024

// 8MB max kernel image size (code + data).
#define KERNEL_PAGE_TABLE_BLOCK_SIZE  (PAGE_SIZE_4K_BYTES/8)


#define USER_SPACE_STACK_SIZE  (64*1024)

#define NS_WORLD_PRIORITY  50

#define ARCH_PAGE_SIZE          4096

#define ARCH_HALT()  while (1) { asm volatile ("wfi":::); }

#endif
