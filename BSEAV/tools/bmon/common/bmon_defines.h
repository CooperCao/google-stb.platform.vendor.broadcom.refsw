/******************************************************************************
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
 *****************************************************************************/
#ifndef __BMON_DEFINES_H__
#define __BMON_DEFINES_H__

#include <stdio.h>

/* TTTTTTTTTTTTTTT another min inside bmon_utils.h */
#undef MIN
#define MIN(x, y)  (((x) > (y)) ? (y) : (x))
#undef MAX
#define MAX(x, y)  (((x) < (y)) ? (y) : (x))
#undef ABS
#define ABS(x)     (((x) < 0) ? -(x) : (x))
#undef ROUND
#define ROUND(x)   ((int)((x) + 0.5))
#undef SECS_TO_MSECS
#define SECS_TO_MSECS(x) ((int)((x) * 1000.0))
#undef UNUSED
#define UNUSED(x)  ((void)x)
#define __FUNCTION__ __extension__ __FUNCTION__

#define MAX_OBJECTS  16

#define FRE(var)             \
    do {                     \
        if (NULL != (var)) { \
            free(var);       \
            (var) = NULL;    \
        }                    \
    } while (0)

#define BFRE(var)            \
    do {                     \
        if (NULL != (var)) { \
            B_Os_Free(var);  \
            (var) = NULL;    \
        }                    \
    } while (0)

#pragma GCC diagnostic ignored "-Wvariadic-macros"
#define NOPRINTF(...)
#define NOFPRINTF(...)

/*#define BMON_TRACE_ENABLE*/
#ifdef BMON_TRACE_ENABLE
#define BMON_TRACE(str) fprintf(stderr, "TRACE %s(): %s\n", __FUNCTION__, str);
#else
#define BMON_TRACE(str)
#endif

/* TO_STRING() is used to convert the given #define to a string literal */
#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#define TRIM_TRAILING_NEWLINE(line)     \
    do {                                \
        unsigned int len = 0;           \
        if (line && strlen( line ))     \
        {                               \
            len = strlen( line );       \
            if (line[len-1] == '\n')    \
            {                           \
                line[len-1] = 0;        \
            }                           \
        }                               \
    } while(0)

#define TRIM_LEADING_CHARS(line, num)                                   \
    do {                                                                \
        unsigned int index = 0;                                         \
        while (('\0' != line[index]) && ('\0' != line[index + (num)]))  \
        {                                                               \
            line[index] = line[index + (num)];                          \
            index++;                                                    \
        }                                                               \
        line[index] = '\0';                                             \
    } while(0)

#define TRIM_LEADING_SLASH(line)                                        \
    do {                                                                \
        if ('/' == line[0])                                             \
        {                                                               \
            TRIM_LEADING_CHARS(line, 1);                                \
        }                                                               \
    } while(0)

/* print a BAS error string/code */
#define PRINT_BAS_ERROR(err_str, err_code)                                                                 \
    do {                                                                                                   \
        fprintf(stderr, "BAS ERROR: %s - code:%d at %s: %d\n", (err_str), (err_code), __FILE__, __LINE__); \
    } while (0)

/* print a Nexus error string/code */
#define PRINT_NEXUS_ERROR(err_str, nerr_code)                                                                 \
    do {                                                                                                      \
        fprintf(stderr, "NEXUS ERROR: %s - code:%d at %s: %d\n", (err_str), (nerr_code), __FILE__, __LINE__); \
    } while (0)

/* print a B_Error string/code */
#define PRINT_BOS_ERROR(err_str, berr_code)                                                                      \
    do {                                                                                                         \
        fprintf(stderr, "B_OS_LIB ERROR: %s - code:%d at %s: %d\n", (err_str), (berr_code), __FILE__, __LINE__); \
    } while (0)

/* if err_code != 0, print err_str */
#define CHECK_ERROR(err_str, err_code)          \
    do {                                        \
        if (0 > (err_code)) {                   \
            PRINT_BAS_ERROR(err_str, err_code); \
        }                                       \
    } while (0)

/* if err_code != 0, print err_str and goto given label */
#define CHECK_ERROR_GOTO(err_str, err_code, label) \
    do {                                           \
        if (0 > (err_code)) {                      \
            PRINT_BAS_ERROR(err_str, err_code);    \
            goto label;                            \
        }                                          \
    } while (0)

/* if ptr equals NULL, print error, set given err_code to err_var and goto given label */
#define CHECK_PTR_ERROR_GOTO(err_str, ptr, err_var, err_code, label) \
    do {                                                             \
        if (NULL == (ptr)) {                                         \
            PRINT_BAS_ERROR(err_str, err_code);                      \
            (err_var) = (err_code);                                  \
            goto label;                                              \
        }                                                            \
    } while (0)

/* if ptr equals NULL, goto given label */
#define CHECK_PTR_GOTO(ptr, label) \
    do {                           \
        if (NULL == (ptr)) {       \
            goto label;            \
        }                          \
    } while (0)

/* if ptr equals NULL, print error, and continue */
#define CHECK_PTR_ERROR_CONTINUE(err_str, ptr) \
    do {                                       \
        if (NULL == (ptr)) {                   \
            PRINT_BAS_ERROR(err_str, -1);      \
            continue;                          \
        }                                      \
    } while (0)

#define NERROR_TO_ERROR(nerr_code, err_code)                  \
    do                                                        \
    {                                                         \
        if ((nerr_code) == NEXUS_SUCCESS) { (err_code) = 0; } \
        else                                                  \
        { (err_code) = -1; }                                  \
    } while (0);

/* if nexus error, print error, set given err_var with atlas error code, and goto given label */
#define CHECK_NEXUS_ERROR_GOTO(err_str, err_var, nerr_code, label) \
    do {                                                           \
        if (NEXUS_SUCCESS != (nerr_code)) {                        \
            PRINT_NEXUS_ERROR(err_str, nerr_code);                 \
            NERROR_TO_ERROR((nerr_code), (err_var));               \
            goto label;                                            \
        }                                                          \
        else {                                                     \
            (err_var) = 0;                                         \
        }                                                          \
    } while (0)

#define BERROR_TO_ERROR(berr_code, err_code)                          \
    do                                                                \
    {                                                                 \
        if ((berr_code) == 0 /*B_ERROR_SUCCESS*/) { (err_code) = 0; } \
        else                                                          \
        { (err_code) = -1; }                                          \
    } while (0);

/* if b_os error, print error, set given err_var with atlas error code, and goto given label */
#define CHECK_BOS_ERROR_GOTO(err_str, err_var, berr_code, label) \
    do {                                                         \
        if (B_ERROR_SUCCESS != (berr_code)) {                    \
            PRINT_BOS_ERROR(err_str, berr_code);                 \
            BERROR_TO_ERROR((berr_code), (err_var));             \
            goto label;                                          \
        }                                                        \
        else {                                                   \
            (err_var) = 0;                                       \
        }                                                        \
    } while (0)

#endif /* __BMON_DEFINES_H__ */