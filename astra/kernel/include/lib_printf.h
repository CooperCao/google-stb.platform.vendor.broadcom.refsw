/***************************************************************************
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
 ***************************************************************************/

#ifndef __LIB_PRINTF_H__
#define __LIB_PRINTF_H__

#ifndef __STDC_VERSION__
#define __STDC_VERSION__ 199900L
#endif

#ifdef __cplusplus
#include <cstdarg>
#else
#include <stdarg.h>
#endif

#define __printf(a, b)          __attribute__((format(printf, a, b)))

#define ANSI_RED    "\e[31;1m"
#define ANSI_GREEN  "\e[32;1m"
#define ANSI_YELLOW "\e[33;1m"
#define ANSI_BLUE   "\e[34;1m"
#define ANSI_BLINK  "\e[5m"
#define ANSI_NOBLINK    "\e[25m"
#define ANSI_RESET  "\e[0m"

#ifdef __cplusplus
extern "C" {
#endif

__printf(1, 0)
int xvsprintf(char *outbuf, const char *templat, va_list marker);

__printf(2, 3)
int xsprintf(char *buf, const char *templat, ...);

__printf(1, 2)
int xprintf(const char *templat, ...);

int xvprintf(const char *templat, va_list marker);

#ifdef __cplusplus
}
#endif

#define err_msg(fmt, ...) xprintf(ANSI_RED fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define warn_msg(fmt, ...) xprintf(ANSI_YELLOW fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define info_msg(fmt, ...) xprintf(ANSI_BLUE fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define success_msg(fmt, ...) xprintf(ANSI_GREEN fmt ANSI_RESET "\n", ##__VA_ARGS__)


#define ATA_LogErr(fmt, ...) xprintf(ANSI_RED "\nNIGHTCRAWLER ERROR : " fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define ATA_LogMsg(fmt, ...) xprintf("\nNIGHTCRAWLER MSG : " fmt "\n", ##__VA_ARGS__)
#define ATA_LogWar(fmt, ...) xprintf(ANSI_YELLOW "\nNIGHTCRAWLER WARNING : " fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define ATA_LogSuccess(fmt, ...) xprintf(ANSI_GREEN "\nNIGHTCRAWLER SUCCESS : " fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define ATA_LogFail(fmt, ...) xprintf(ANSI_RED "\nNIGHTCRAWLER FAIL : " fmt ANSI_RESET "\n", ##__VA_ARGS__)


/*
 * compatibility macros
 */

#define printf xprintf
#define sprintf xsprintf
#define vsprintf xvsprintf
#define vprintf xvprintf

#ifdef __cplusplus
extern "C" void print_init();
#endif

#endif /* __LIB_PRINTF_H__ */
