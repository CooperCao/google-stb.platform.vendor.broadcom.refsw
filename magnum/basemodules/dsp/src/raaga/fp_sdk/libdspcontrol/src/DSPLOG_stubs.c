/****************************************************************************
 *                Copyright (c) 2014 Broadcom Corporation                   *
 *                                                                          *
 *      This material is the confidential trade secret and proprietary      *
 *      information of Broadcom Corporation. It may not be reproduced,      *
 *      used, sold or transferred to any third party without the prior      *
 *      written consent of Broadcom Corporation. All rights reserved.       *
 *                                                                          *
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



#if FEATURE_IS(LIBC, FREESTANDING) && !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)

void DSPLOG_setExitFunction(DSPLOG_EXIT_FUNC_T new_func)
{
    DSPLOG_EXIT_FUNC = new_func;
}

#endif


void DSPLOG_setLevel(int new_level __unused)
{
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



void DSPLOG_setFormat(bool new_show_target   __unused,
                      bool new_show_time     __unused,
                      bool new_show_filename __unused)
{
}


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
