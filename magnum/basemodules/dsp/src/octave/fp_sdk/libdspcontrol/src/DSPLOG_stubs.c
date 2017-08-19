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

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/COMMON.h"
#include "libdspcontrol/DSPLOG.h"

#include "libfp/c_utils.h"



#if FEATURE_IS(LIBC, FREESTANDING) && !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)

void DSPLOG_setExitFunction(DSPLOG_EXIT_FUNC_T new_func)
{
    DSPLOG_EXIT_FUNC = new_func;
}

#endif


#if !B_REFSW_MINIMAL

void DSPLOG_setLevel(int new_level __unused)
{
}

#endif /* !B_REFSW_MINIMAL */


int DSPLOG_getLevel(void)
{
    return DSPLOG_NOTHING_LEVEL;
}


#if FEATURE_IS(DSPLOG_SINK, STDIO)

void DSPLOG_setFileSink(FILE *new_sink __unused)
{
}

#elif FEATURE_IS(DSPLOG_SINK, MEMORY)

void DSPLOG_setMemorySink(uint8_t *new_buffer __unused, size_t buffer_size __unused)
{
}

#endif  /* FEATURE_IS(DSPLOG_SINK, STDIO) */


#if !B_REFSW_MINIMAL

void DSPLOG_setFormat(bool new_show_target   __unused,
                      bool new_show_time     __unused,
                      bool new_show_filename __unused)
{
}

#endif /* !B_REFSW_MINIMAL */


#if !FEATURE_IS(DSPLOG_SINK, NULL)  /* Already sorted out in headers, if this is true */

void DSPLOG_impl(const char *fileName __unused,
                 const int line       __unused,
                 const int logLevel   __unused,
                 const char *fmt      __unused,
                 ...)
{
}


void DSPLOG_impl_va(const char *fileName __unused,
                    const int line       __unused,
                    const int logLevel   __unused,
                    const char *fmt      __unused,
                    va_list va_args      __unused)
{
}

#endif


#if !B_REFSW_MINIMAL

void DSPLOG_memdump(int log_level        __unused,
                    const char *prefix   __unused,
                    const uint8_t *bytes __unused,
                    unsigned len         __unused)
{
}


void DSPLOG_memdump_wrap(int log_level           __unused,
                         const char *prefix      __unused,
                         const uint8_t *bytes    __unused,
                         unsigned len            __unused,
                         unsigned bytes_per_line __unused)
{
}

#endif /* !B_REFSW_MINIMAL */
