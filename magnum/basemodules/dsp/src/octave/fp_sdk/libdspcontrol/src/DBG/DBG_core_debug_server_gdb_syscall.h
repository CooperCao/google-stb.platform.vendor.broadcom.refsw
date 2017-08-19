/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/

#ifndef _DBG_CORE_DEBUG_SERVER_GDB_SYSCALL_H_
#define _DBG_CORE_DEBUG_SERVER_GDB_SYSCALL_H_

#include <stdint.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DBG.h"

#include "DBG_core_debug_server.h"

typedef enum fargs_types_t
{
    fargs_type_hex_int = 0,
    fargs_type_string
}fargs_types_t;

typedef struct fargs
{
    fargs_types_t type;
    uint32_t u32_value;
    uint32_t u32_opt_len;   /*Optional length parameter for string arguments*/
}fargs_t;

#if defined (__FPM1015_ONWARDS__)
typedef uint32_t fp_reg_t;
#define SYSCALL_NUM_REG     FP_REG_R3
#define SYSCALL_ARG0_REG    FP_REG_R0
#define SYSCALL_ARG1_REG    FP_REG_R1
#define SYSCALL_ARG2_REG    FP_REG_R2
#else
typedef uint64_t fp_reg_t;
#define SYSCALL_NUM_REG     FP_REG_R0
#define SYSCALL_ARG0_REG    FP_REG_R1
#define SYSCALL_ARG1_REG    FP_REG_R2
#define SYSCALL_ARG2_REG    FP_REG_R3
#endif

void
DBG_core_gdb_syscall_handle(DBG *dbg);

void
DBG_core_gdb_syscall_handle_reply(DBG *dbg);

#endif
