/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#ifndef __BSU_STDIO_H__
#define __BSU_STDIO_H__

#include <stdarg.h>

#define __printf(a, b)          __attribute__((format(printf, a, b)))

#define ANSI_RED	"\e[31;1m"
#define ANSI_YELLOW	"\e[33;1m"
#define ANSI_CYAN	"\e[36;1m"
#define ANSI_BLINK	"\e[5m"
#define ANSI_NOBLINK	"\e[25m"
#define ANSI_RESET	"\e[0m"

__printf(2, 0)
int xvsprintf(char *outbuf, const char *templat, va_list marker);

__printf(2, 3)
int xsprintf(char *buf, const char *templat, ...);

__printf(1, 2)
//int xprintf(const char *templat, ...);
int printf(const char *templat, ...);

extern int (*xprinthook) (const char *);

__printf(1, 0)
int xvprintf(const char *template, va_list marker);

__printf(3, 0)
int xvsnprintf(char *outbuf, int n, const char *templat, va_list marker);

#define err_msg(fmt, ...) xprintf(ANSI_RED fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define warn_msg(fmt, ...) xprintf(ANSI_YELLOW fmt ANSI_RESET "\n", ##__VA_ARGS__)
#define info_msg(fmt, ...) xprintf(ANSI_CYAN fmt ANSI_RESET "\n", ##__VA_ARGS__)

/*
 * compatibility macros
 */

//#define printf xprintf
#define sprintf xsprintf
#define vsprintf xvsprintf
#define vsnprintf xvsnprintf
#define vprintf xvprintf

#endif /* __BSU_STDIO_H__ */
