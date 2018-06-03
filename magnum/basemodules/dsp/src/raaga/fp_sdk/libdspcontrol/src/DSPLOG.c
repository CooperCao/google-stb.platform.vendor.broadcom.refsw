/****************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ****************************************************************************/

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/COMMON.h"
#include "libdspcontrol/DSPLOG.h"
#include "libdspcontrol/OS.h"

#include "UTIL.h"


#if FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  error "This module is not suitable for Raaga Magnum"
#endif
#if FEATURE_IS(DSPLOG_SINK, NULL)
#  error "This implementation doesn't support a NULL DSPLOG_SINK feature, use DSPLOG_stubs.c instead"
#endif


/*
 * Module variables
 */
#define MAX_MSG_LEN          512
static int log_verbose_level = DSPLOG_DEFAULT_LEVEL;
static bool show_target      = DSPLOG_DEFAULT_SHOWTARGET;
static bool show_time        = DSPLOG_DEFAULT_SHOWTIME;
static bool show_error_type  = DSPLOG_DEFAULT_SHOWERRORTYPE;
static bool show_filename    = DSPLOG_DEFAULT_SHOWFILENAME;
static char *error_name[]    = { "DUMMY", "FAILURE", "ERROR", "INFO", "DETAIL", "DEBUG", "JUNK" };
#if FEATURE_IS(DSPLOG_SINK, MEMORY)
static uint8_t *mem_sink            = NULL;
static size_t mem_sink_size         = 0;
static size_t mem_sink_write_offset = 0;
#elif FEATURE_IS(DSPLOG_SINK, STDIO)
static FILE *file_sink              = NULL;
#endif
#if FEATURE_IS(LIBC, FREESTANDING)
DSPLOG_EXIT_FUNC_T DSPLOG_EXIT_FUNC = NULL;
#endif


/*
 * Common interface for I/O interface locks
 */
#if defined(MCPHY) && IS_HOST(BM)
/* Workaround for a libc deadlock (yes, it happens!) on FreeRTOS Posix */
#  include "FreeRTOS.h"
#  include "list.h"
#  include "semphr.h"
static xSemaphoreHandle stdio_mutex;
#  define DSPLOG_IO_LOCK_CREATE()     do { stdio_mutex = xSemaphoreCreateRecursiveMutex(); } while(0)
#  define DSPLOG_IO_LOCK_DESTROY()    vQueueDelete(stdio_mutex)
#  define DSPLOG_LOCK_IO()            xSemaphoreTakeRecursive(stdio_mutex, portMAX_DELAY)
#  define DSPLOG_UNLOCK_IO()          xSemaphoreGiveRecursive(stdio_mutex)
#else
#  define DSPLOG_IO_LOCK_CREATE()
#  define DSPLOG_IO_LOCK_DESTROY()
#  define DSPLOG_LOCK_IO()
#  define DSPLOG_UNLOCK_IO()
#endif


/*
 * Target name
 */
#define DSPLOG_TARGETNAME_AND_SPACE     BFPSDK_LIBDSPCONTROL_TARGET_NAME " "


/*
 * Module constructor and destructor.
 */
__attribute__((constructor))
static void DSPLOG_init(void)
{
#if FEATURE_IS(DSPLOG_SINK, STDIO)
    DSPLOG_setFileSink(NULL);
#endif
    DSPLOG_IO_LOCK_CREATE();
}

__attribute__((destructor))
static void DSPLOG_close(void)
{
    DSPLOG_IO_LOCK_DESTROY();
}


#if FEATURE_IS(LIBC, FREESTANDING)

void DSPLOG_setExitFunction(DSPLOG_EXIT_FUNC_T new_func)
{
    DSPLOG_EXIT_FUNC = new_func;
}

#endif


void DSPLOG_setLevel(int new_level)
{
    if((new_level < DSPLOG_NOTHING_LEVEL) || (new_level > DSPLOG_JUNK_LEVEL))
        FATAL_ERROR("Log level must be between %d and %d", DSPLOG_NOTHING_LEVEL, DSPLOG_JUNK_LEVEL);

    log_verbose_level = new_level;
}


int DSPLOG_getLevel(void)
{
    return log_verbose_level;
}


#if FEATURE_IS(DSPLOG_SINK, STDIO)

void DSPLOG_setFileSink(FILE *new_sink)
{
    if(new_sink != NULL)
        file_sink = new_sink;
    else
        file_sink = DSPLOG_DEFAULT_SINK;
}

#elif FEATURE_IS(DSPLOG_SINK, MEMORY)

void DSPLOG_setMemorySink(uint8_t *new_buffer, size_t buffer_size)
{
    mem_sink = new_buffer;
    mem_sink_size = buffer_size;
    memset(mem_sink, 0, mem_sink_size);
}

#endif  /* FEATURE_IS(DSPLOG_SINK, STDIO) */


void DSPLOG_setFormat(bool new_show_target, bool new_show_time, bool new_show_filename)
{
    show_target = new_show_target;
    show_time = new_show_time;
    show_filename = new_show_filename;
}


void DSPLOG_impl(const char *filename, const int line, const int log_level, const char *fmt, ...)
{
    va_list va_args;
    va_start(va_args, fmt);
    DSPLOG_impl_va(filename, line, log_level, fmt, va_args);
    va_end(va_args);
}


void DSPLOG_impl_va(const char *filename, const int line, const int log_level, const char *fmt, va_list va_args)
{
    static char formatted[MAX_MSG_LEN];
    char *cursor = formatted;
    char *lastDirSep;
    int remaining_len;
    int n;
#if FEATURE_IS(DSPLOG_SINK, MEMORY)
    size_t formatted_len;
    size_t avail_space;

    if(mem_sink_size == 0)
        return;
#endif

    if(log_verbose_level >= log_level)
    {
        /*  strip the path from the filename */
        lastDirSep = strrchr(filename, '/');
        if(lastDirSep)
            filename = lastDirSep + 1;

        /*  formatted pieces of the log message */
        if(show_target)
            cursor = stpncpy(cursor, DSPLOG_TARGETNAME_AND_SPACE, MAX_MSG_LEN);

        if(show_time)
        {
            cursor += OS_timeAsString(cursor, MAX_MSG_LEN - (cursor - formatted), "%Y.%m.%d.%H:%M:%S");
            if(cursor - formatted < MAX_MSG_LEN)
            {
                *cursor = ' ';
                cursor++;
            }
        }

        if(show_error_type)
        {
            cursor = stpncpy(cursor, error_name[log_level], MAX_MSG_LEN - (cursor - formatted));
            cursor = stpncpy(cursor, " ", MAX_MSG_LEN - (cursor - formatted));
        }

        if(show_filename)
        {
            /*  The functions snprintf() and vsnprintf() do not write more than size bytes (including the */
            /*  terminating null byte ('\0')). If the output was truncated due to this limit then the return */
            /*  value is the number of characters (excluding the terminating null byte) which would have been */
            /*  written to the final string if enough space had been available. Thus, a return value of size */
            /*  or more means that the output was truncated. */
            remaining_len = MAX_MSG_LEN - (cursor - formatted);
            n = snprintf(cursor, remaining_len, "at %s,%d ", filename, line);
            cursor += n > remaining_len ? remaining_len : n;
        }

        /*  write user formatted data */
        {
            /*  if we wrote something, we left a space at the end */
            if(cursor > formatted)
                cursor = stpncpy(cursor - 1, ": ", MAX_MSG_LEN - (cursor - formatted));

            remaining_len = MAX_MSG_LEN - (cursor - formatted);
            n = vsnprintf(cursor, remaining_len, fmt, va_args);

            cursor += n > remaining_len ? remaining_len : n;
            cursor = stpncpy(cursor, "\r\n", MAX_MSG_LEN - (cursor - formatted));
        }

        /*  always NULL - terminate */
        formatted[MAX_MSG_LEN - 1] = '\0';

        DSPLOG_LOCK_IO();

#if FEATURE_IS(DSPLOG_SINK, STDIO)
        /*  write to sink */
        fputs(formatted, file_sink);
#elif FEATURE_IS(DSPLOG_SINK, MEMORY)
        /*  write to memory */
        formatted_len = strlen(formatted) + 1;
        avail_space = mem_sink_size - mem_sink_write_offset;
        if(formatted_len > avail_space)
        {
            formatted[avail_space - 1] = '\x0';
            formatted_len = avail_space;
        }
        memcpy(&mem_sink[mem_sink_write_offset], formatted, formatted_len);
        mem_sink_write_offset += formatted_len;
#endif

        DSPLOG_UNLOCK_IO();
    }
}


void DSPLOG_memdump(int log_level, const char *prefix, const uint8_t *bytes, unsigned len)
{
    int prefix_length = strlen(prefix);
    char buf[prefix_length + 3 * len + 1];

    char *ptr = buf + prefix_length;
    unsigned i;
    strcpy(buf, prefix);
    for(i = 0; i < len; i++)
        ptr += sprintf(ptr, " %02x", bytes[i]);

    DSPLOG_LOG(log_level, buf);
}


void DSPLOG_memdump_wrap(int log_level, const char *prefix, const uint8_t *bytes, unsigned len, unsigned bytes_per_line)
{
    unsigned i;
    for(i = 0; i < len; i += bytes_per_line)
        DSPLOG_memdump(log_level, prefix, &bytes[i], len - i <= bytes_per_line ? len - i : bytes_per_line);
}
