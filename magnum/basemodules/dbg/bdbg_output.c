/***************************************************************************
 * Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 ***************************************************************************/


#include "bstd.h"
#include "bkni.h"

#if defined(B_REFSW_WITH_LIBBACKTRACE)
#include "backtrace.h"
#include "backtrace-supported.h"

static void
BKNI_P_backtrace_error_callback(void *vdata, const char *msg, int errnum)
{
    BSTD_UNUSED(vdata);
    BDBG_P_PrintString("!!! BACKTRACE: %s(%d)\n", msg, errnum);
    return;
}

static int
BKNI_P_backtrace_print(void *vdata, uintptr_t pc , const char *filename, int lineno, const char *function)
{
    BSTD_UNUSED(vdata);
    BSTD_UNUSED(pc);
    if(function) {
        BDBG_P_PrintString("!!! BACKTRACE: %s:%u %s\n", filename, lineno, function);
    }
    return 0;
}

static void BKNI_PrintBacktrace_isrsafe(void)
{
    void *state = backtrace_create_state ("", BACKTRACE_SUPPORTS_THREADS, BKNI_P_backtrace_error_callback, NULL);
    backtrace_full (state, 0, BKNI_P_backtrace_print, BKNI_P_backtrace_error_callback, NULL);
    return;
}
#elif defined(__GLIBC__) && !defined(__KERNEL__) && !defined(NEXUS_BASE_OS_linuxkernel) && !defined(NEXUS_BASE_OS_ucos)  /* #if defined(B_REFSW_WITH_LIBBACKTRACE) */
#include <execinfo.h>
#include <stdlib.h>

void BKNI_PrintBacktrace_isrsafe(void)
{
    void *data[64];
    int n = backtrace(data, sizeof(data)/sizeof(*data));
    char **lines;

    lines = backtrace_symbols(data, n);
    if(n>0 && lines) {
        unsigned i;
        BDBG_P_PrintString("!!! BACKTRACE %u to follow\n", (unsigned)n);
        for(i=0;i<(unsigned)n;i++) {
            const char *line = lines[i];
            if(line) {
                BDBG_P_PrintString("!!! BACKTRACE[%u]: %s\n", i, line);
            }
        }
        BDBG_P_PrintString("!!! BACKTRACE %u completed\n", (unsigned)n);
    } else {
        if(n) {
            backtrace_symbols_fd(data, n, STDERR_FILENO);
        } else {
            BDBG_P_PrintString("!!! BACKTRACE[%u] is not available\n", (unsigned)n);
        }
    }
    if(lines) {
        free(lines);
    }
    return;
}
#else /* defined(__GLIBC__) && !defined(__KERNEL__) && !defined(NEXUS_BASE_OS_linuxkernel) */
#define BKNI_PrintBacktrace_isrsafe()
#endif

/* coverity[+kill] */
void
BDBG_P_AssertFailed_isrsafe(const char *expr, const char *file, unsigned line)
{
#if B_REFSW_DEBUG_COMPACT_ERR
    BSTD_UNUSED(expr);
    BDBG_P_PrintString_isrsafe("!!! Assert at %s:%d\n", file, line);
#ifdef BDBG_ASSERT_TO_KNI
    BKNI_Printf("!!! Assert at %s:%d\n", file, line);
#endif
#else
    BDBG_P_PrintString_isrsafe("!!! Assert '%s' Failed at %s:%d\n", expr, file, line);
#ifdef BDBG_ASSERT_TO_KNI
    BKNI_Printf("!!! Assert '%s' Failed at %s:%d\n", expr, file, line);
#endif
#endif
    BKNI_PrintBacktrace_isrsafe();
#if !B_REFSW_DEBUG_ASSERT_NO_FAIL    
    BKNI_Fail();
#endif
}

BERR_Code
BDBG_P_PrintError_isrsafe(const char *file, unsigned lineno, const char *error, BERR_Code error_no)
{
    if (error_no != BERR_SUCCESS) {
        if(error==NULL) {
            error="";
#if !B_REFSW_DEBUG_COMPACT_ERR
            if(error_no <= BERR_NOT_AVAILABLE) {
                static const char * const error_names[BERR_NOT_AVAILABLE+1] = {
                    "BERR_SUCCESS",
                    "BERR_NOT_INITIALIZED",
                    "BERR_INVALID_PARAMETER",
                    "BERR_OUT_OF_SYSTEM_MEMORY",
                    "BERR_OUT_OF_DEVICE_MEMORY",
                    "BERR_TIMEOUT",
                    "BERR_OS_ERROR",
                    "BERR_LEAKED_RESOURCE",
                    "BERR_NOT_SUPPORTED",
                    "BERR_UNKNOWN",
                    "BERR_NOT_AVAILABLE"
                };
                BDBG_CASSERT(BERR_SUCCESS==0);
                BDBG_CASSERT(BERR_NOT_INITIALIZED==1);
                BDBG_CASSERT(BERR_INVALID_PARAMETER==2);
                BDBG_CASSERT(BERR_OUT_OF_SYSTEM_MEMORY==3);
                BDBG_CASSERT(BERR_OUT_OF_DEVICE_MEMORY==4);
                BDBG_CASSERT(BERR_TIMEOUT==5);
                BDBG_CASSERT(BERR_OS_ERROR==6);
                BDBG_CASSERT(BERR_LEAKED_RESOURCE==7);
                BDBG_CASSERT(BERR_NOT_SUPPORTED==8);
                BDBG_CASSERT(BERR_UNKNOWN==9);
                BDBG_CASSERT(BERR_NOT_AVAILABLE==10);
                error = error_names[error_no];
            }
#endif /* B_REFSW_DEBUG_COMPACT_ERR */
        }
        BDBG_P_PrintString_isrsafe("!!!Error %s(%#x) at %s:%d\n", error, error_no, file, lineno);
    }
    return error_no;
}

void BDBG_P_PrintErrorString_small_isrsafe(const char *file, unsigned lineno)
{
    BDBG_P_PrintString_isrsafe("### Error at %s:%d\n", file, lineno);
}

BERR_Code BDBG_P_PrintError_small_isrsafe(const char *file, unsigned lineno, BERR_Code error_no)
{
    return BDBG_P_PrintError_isrsafe(file, lineno, NULL, error_no);
}

#ifdef BDBG_ANDROID_LOG
#include <log/log.h>
#endif

void
BDBG_P_PrintString_isrsafe(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    #ifdef BDBG_ANDROID_LOG
    /* These are always used for errors in bdbg */
    LOG_PRI_VA(ANDROID_LOG_ERROR, "NEXUS", fmt, ap);
    #else
    BDBG_P_Vprintf_Log_isrsafe(BDBG_ModulePrintKind_eString, fmt, ap);
    #endif

    va_end( ap );

    return;
}

#undef BDBG_P_AssertFailed
void
BDBG_P_AssertFailed(const char *expr, const char *file, unsigned line)
{
    BDBG_P_AssertFailed_isrsafe(expr, file, line);
}

#if defined BDBG_DEBUG_BUILD && BDBG_DEBUG_BUILD
void
BDBG_EnterFunction(BDBG_pDebugModuleFile dbg_module, const char *function)
{
    BDBG_EnterFunction_isrsafe(dbg_module, function);
}

void
BDBG_LeaveFunction(BDBG_pDebugModuleFile dbg_module, const char *function)
{
    BDBG_LeaveFunction_isrsafe(dbg_module, function);
}
#endif

/* End of file */
