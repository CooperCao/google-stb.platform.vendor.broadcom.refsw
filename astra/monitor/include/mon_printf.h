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

#ifndef _MON_PRINTF_H_
#define _MON_PRINTF_H_

#define ANSI_RED    "\e[31;1m"
#define ANSI_YELLOW "\e[33;1m"
#define ANSI_BLUE   "\e[34;1m"
#define ANSI_GREEN  "\e[32;1m"
#define ANSI_RESET  "\e[0m"

#ifdef DSMP
#include <arch_helpers.h>
#define MPIDR (unsigned int)MPIDR_AFFINITY_VAL(read_mpidr())
#endif

#ifdef RELEASE
#define SYS_MSG(fmt, ...)  mon_printf("MON64: " fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define ERR_MSG(fmt, ...)  mon_printf("MON64: " fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define WARN_MSG(fmt, ...) mon_printf("MON64: " fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define INFO_MSG(fmt, ...)
#else
#ifdef DSMP
#define SYS_MSG(fmt, ...)  mon_printf(ANSI_GREEN  "[%x] SYSTEM: "  fmt ANSI_RESET "\n", MPIDR, ##__VA_ARGS__)
#define ERR_MSG(fmt, ...)  mon_printf(ANSI_RED    "[%x] ERROR: "   fmt ANSI_RESET "\n", MPIDR, ##__VA_ARGS__)
#define WARN_MSG(fmt, ...) mon_printf(ANSI_YELLOW "[%x] WARNING: " fmt ANSI_RESET "\n", MPIDR, ##__VA_ARGS__)
#define INFO_MSG(fmt, ...) mon_printf(ANSI_BLUE   "[%x] INFO: "    fmt ANSI_RESET "\n", MPIDR, ##__VA_ARGS__)
#else
#define SYS_MSG(fmt, ...)  mon_printf(ANSI_GREEN  "SYSTEM: "  fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define ERR_MSG(fmt, ...)  mon_printf(ANSI_RED    "ERROR: "   fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define WARN_MSG(fmt, ...) mon_printf(ANSI_YELLOW "WARNING: " fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define INFO_MSG(fmt, ...) mon_printf(ANSI_BLUE   "INFO: "    fmt ANSI_RESET "\n", ##__VA_ARGS__)
#endif
#endif

#ifdef DEBUG
#ifdef DSMP
#define DBG_MSG(fmt, ...)  mon_printf("[%x] DEBUG: " fmt "\n", MPIDR, ##__VA_ARGS__)
#else
#define DBG_MSG(fmt, ...)  mon_printf("DEBUG: " fmt "\n", ##__VA_ARGS__)
#endif
#else
#define DBG_MSG(fmt, ...)
#endif

#ifdef DEBUG
#define DBG_PRINT(...) mon_printf(__VA_ARGS__)
#else
#define DBG_PRINT(...)
#endif

void mon_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#endif /* _MON_PRINTF_H_ */
