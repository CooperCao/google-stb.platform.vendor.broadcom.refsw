/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifndef ATLAS_H__
#define ATLAS_H__

#include "bstd.h"

#ifdef STACK_TRACE
#include <sys/resource.h>
#include "brcm_sig_hdl.h"
#endif

#include "nexus_platform.h"
#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#undef MIN
#define MIN(x, y)  (((x) > (y)) ? (y) : (x))
#undef MAX
#define MAX(x, y)  (((x) < (y)) ? (y) : (x))
#undef ABS
#define ABS(x)     (((x) < 0) ? -(x) : (x))

#define COLOR_STEP              0x000a0a0a
#define COLOR_BLACK             0xFF000000
#define COLOR_EGGSHELL          0xFFe1e1e1
#define COLOR_YELLOW            0xFFAAAA00
#define COLOR_GREEN             0xFF80c42f
#define COLOR_GREY              0xFFa0a0a0
#define COLOR_GREY_DARK         0xFF0d0d0d
#define COLOR_BLUE              0xFF0662C6
#define COLOR_BLUE_LIGHT        0xFF7EAFE2
#define COLOR_BLUE_DARK         0xFF022040
#define COLOR_BLUE_SLATE        0xFF22527A

#define COLOR_ORANGE            0xFFFCA400
#define COLOR_ORANGE_LIGHT      0xFFFCC100
#define COLOR_ORANGE_DARK       0xFFFC8200
#define COLOR_BLUE_DARK2        0xFF034199
#define COLOR_BLUE_DARK2_LIGHT  0xFF0456CC
#define COLOR_BLUE_DARK2_DARK   0xFF022B66
#define COLOR_LIME              0xFFA4D52A
#define COLOR_LIME_LIGHT        0xFFC1F931
#define COLOR_LIME_DARK         0xFF83AA22
#define COLOR_MAGENTA           0xFFA1228D
#define COLOR_MAGENTA_LIGHT     0xFFD42DB9
#define COLOR_MAGENTA_DARK      0xFF6E1760
#define COLOR_RED               0xFFCC2C2E
#define COLOR_RED_LIGHT         0xFFFF3739
#define COLOR_RED_DARK          0xFF992122

/* Atlas error codes */
typedef enum eRet
{
    eRet_Ok               = 0,
    eRet_OutOfMemory      = 1,
    eRet_InvalidParameter = 2,
    eRet_NotSupported     = 3,
    eRet_NotAvailable     = 4,
    eRet_Busy             = 5,
    eRet_ExternalError    = 6,
    eRet_InvalidState     = 7,
    eRet_Timeout          = 8
} eRet;

#define NERROR_TO_ERROR(nerr_code, err_code)                                                \
    do                                                                                      \
    {                                                                                       \
        if ((nerr_code) == NEXUS_SUCCESS) { (err_code) = eRet_Ok; }                         \
        else                                                                                \
        if ((nerr_code) == NEXUS_NOT_INITIALIZED) { (err_code) = eRet_InvalidState; }       \
        else                                                                                \
        if ((nerr_code) == NEXUS_INVALID_PARAMETER) { (err_code) = eRet_InvalidParameter; } \
        else                                                                                \
        if ((nerr_code) == NEXUS_OUT_OF_SYSTEM_MEMORY) { (err_code) = eRet_OutOfMemory; }   \
        else                                                                                \
        if ((nerr_code) == NEXUS_OUT_OF_DEVICE_MEMORY) { (err_code) = eRet_OutOfMemory; }   \
        else                                                                                \
        if ((nerr_code) == NEXUS_TIMEOUT) { (err_code) = eRet_Timeout; }                    \
        else                                                                                \
        if ((nerr_code) == NEXUS_OS_ERROR) { (err_code) = eRet_ExternalError; }             \
        else                                                                                \
        if ((nerr_code) == NEXUS_LEAKED_RESOURCE) { (err_code) = eRet_ExternalError; }      \
        else                                                                                \
        if ((nerr_code) == NEXUS_NOT_SUPPORTED) { (err_code) = eRet_NotSupported; }         \
        else                                                                                \
        if ((nerr_code) == NEXUS_UNKNOWN) { (err_code) = eRet_ExternalError; }              \
        else                                                                                \
        if ((nerr_code) == NEXUS_NOT_AVAILABLE) { (err_code) = eRet_NotAvailable; }         \
        else                                                                                \
        { (err_code) = eRet_ExternalError; }                                                \
    } while (0);

/*TODO: Right now we don't take any action for error case so all BIP_Errors are marked as external error.*/
#define BERROR_TO_ERROR(nerr_code, err_code)                                 \
    do                                                                       \
    {                                                                        \
        if ((nerr_code) != BIP_SUCCESS) { (err_code) = eRet_ExternalError; } \
    } while (0);

#define NEXUS_SUCCESS               0  /* success (always zero) */
#define NEXUS_NOT_INITIALIZED       1  /* parameter not initialized */
#define NEXUS_INVALID_PARAMETER     2  /* parameter is invalid */
#define NEXUS_OUT_OF_SYSTEM_MEMORY  3  /* out of KNI module memory (aka OS memory) */
#define NEXUS_OUT_OF_DEVICE_MEMORY  4  /* out of MEM module memory (ala heap memory) */
#define NEXUS_TIMEOUT               5  /* reached timeout limit */
#define NEXUS_OS_ERROR              6  /* generic OS error */
#define NEXUS_LEAKED_RESOURCE       7  /* resource being freed has attached resources that haven't been freed */
#define NEXUS_NOT_SUPPORTED         8  /* requested feature is not supported */
#define NEXUS_UNKNOWN               9  /* unknown */
#define NEXUS_NOT_AVAILABLE         10 /* no resource available */

/*
 * the following atlas error codes and support macros are used thoughout the
 * application to handle atlas api errors, nexus api errors, b_os api errors,
 * and memory allocation errors.
 */
#define DEL(var)             \
    do {                     \
        if (NULL != (var)) { \
            delete (var);    \
            (var) = NULL;    \
        }                    \
    } while (0)

#define FRE(var)             \
    do {                     \
        if (NULL != (var)) { \
            free(var);       \
            (var) = NULL;    \
        }                    \
    } while (0)

/* print helpful trace only if ATLAS_MEMORY_LEAK_DETECT is exported */
#define ATLAS_MEMLEAK_TRACE(str)                                        \
    do {                                                                \
        if (getenv("ATLAS_MEMORY_LEAK_DETECT")) {                       \
            BDBG_WRN(("--> %s:%s %s", __FILE__, __FUNCTION__, (str)));  \
        }                                                               \
    } while (0)

/* print atlas error and return given error code */
#define ATLAS_ERROR(str, err)      (BDBG_ERR(("ATLAS ERROR: %s - code:%d at %s: %d", (str), (err), __FILE__, __LINE__)), err)

/* print atlas warning and return given error code */
#define ATLAS_WARN(str, err)       (BDBG_WRN(("ATLAS WARNING: %s - code:%d at %s: %d", (str), (err), __FILE__, __LINE__)), err)

/* print atlas lua error and return given error code */
#define ATLAS_LUA_ERROR(str, err)  (BDBG_ERR(("ATLAS LUA ERROR: %s - code:%d at %s: %d", (str), (err), __FILE__, __LINE__)), err)

/* print nexus error and return eRet_ExternalError code */
#define NEXUS_ERROR(str, err)      (BDBG_ERR(("NEXUS ERROR: %s - code:%d at %s: %d", (str), (err), __FILE__, __LINE__)), eRet_ExternalError)

/* if nexus error, print error and return eRet_ExternalError.  otherwise return eRet_Ok */
#define CHECK_NEXUS_ERROR(str, nerr) \
    ((NEXUS_SUCCESS != (nerr)) ?  NEXUS_ERROR(str, nerr) : eRet_Ok)

/* if nexus error, print warning and return eRet_ExternalError.  otherwise return eRet_Ok */
#define CHECK_NEXUS_WARN(str, nerr) \
    ((NEXUS_SUCCESS != (nerr)) ?  NEXUS_ERROR(str, nerr) : eRet_Ok)

/* if nexus error, print error then assert */
#define CHECK_NEXUS_ERROR_ASSERT(str, nerr)   \
    do {                                      \
        CHECK_NEXUS_ERROR((str), (nerr));     \
        BDBG_ASSERT(NEXUS_SUCCESS == (nerr)); \
    } while (0)

/* if nexus error, print error, set given err_var with atlas error code, and goto given label */
#define CHECK_NEXUS_ERROR_GOTO(err_str, err_var, nerr_code, label)                                         \
    do {                                                                                                   \
        if (NEXUS_SUCCESS != (nerr_code)) {                                                                \
            BDBG_ERR(("NEXUS ERROR: %s - code:%d at %s: %d", (err_str), (nerr_code), __FILE__, __LINE__)); \
            NERROR_TO_ERROR((nerr_code), (err_var));                                                       \
            goto label;                                                                                    \
        }                                                                                                  \
        else {                                                                                             \
            (err_var) = eRet_Ok;                                                                           \
        }                                                                                                  \
    } while (0)

/* if nexus error, print warning, set given err_var with atlas error code, and goto given label */
#define CHECK_NEXUS_WARN_GOTO(err_str, err_var, nerr_code, label)                                         \
    do {                                                                                                  \
        if (NEXUS_SUCCESS != (nerr_code)) {                                                               \
            BDBG_WRN(("NEXUS WARN: %s - code:%d at %s: %d", (err_str), (nerr_code), __FILE__, __LINE__)); \
            NERROR_TO_ERROR((nerr_code), (err_var));                                                      \
            goto label;                                                                                   \
        }                                                                                                 \
        else {                                                                                            \
            (err_var) = eRet_Ok;                                                                          \
        }                                                                                                 \
    } while (0)

/* if bip error, print error, set given err_var with atlas error code, and goto given label */
#define CHECK_BIP_ERROR_GOTO(err_str, err_var, nerr_code, label)                                         \
    do {                                                                                                 \
        if (BIP_SUCCESS != (nerr_code)) {                                                                \
            BDBG_ERR(("BIP ERROR: %s - code:%d at %s: %d", (err_str), (nerr_code), __FILE__, __LINE__)); \
            BERROR_TO_ERROR((nerr_code), (err_var));                                                     \
            goto label;                                                                                  \
        }                                                                                                \
        else {                                                                                           \
            (err_var) = eRet_Ok;                                                                         \
        }                                                                                                \
    } while (0)

/* if bip error, print error, set given err_var with atlas error code, and goto given label */
#define CHECK_BIP_ERROR(err_str, bip_err_code)                                                              \
    do {                                                                                                    \
        if (0 != (bip_err_code)) {                                                                          \
            BDBG_ERR(("BIP ERROR: %s - code:%d at %s: %d", (err_str), (bip_err_code), __FILE__, __LINE__)); \
        }                                                                                                   \
    } while (0)

/* if nexus error, print warning, set given err_var with atlas error code, and goto given label */
#define CHECK_BIP_WARN_GOTO(err_str, err_var, nerr_code, label)                                           \
    do {                                                                                                  \
        if (NEXUS_SUCCESS != (nerr_code)) {                                                               \
            BDBG_WRN(("NEXUS WARN: %s - code:%d at %s: %d", (err_str), (nerr_code), __FILE__, __LINE__)); \
            BERROR_TO_ERROR((nerr_code), (err_var)); /* Here we will have to write a BERROR_TO_ERROR */   \
            goto label;                                                                                   \
        }                                                                                                 \
        else {                                                                                            \
            (err_var) = eRet_Ok;                                                                          \
        }                                                                                                 \
    } while (0)

/* if pmlib error, print error, set given err_var with atlas error code, and goto given label */
#define CHECK_PMLIB_ERROR_GOTO(err_str, err_var, pm_err_code, label)                                         \
    do {                                                                                                     \
        if (0 != (pm_err_code)) {                                                                            \
            BDBG_ERR(("pmlib ERROR: %s - code:%d at %s: %d", (err_str), (pm_err_code), __FILE__, __LINE__)); \
            (err_var) = eRet_ExternalError;                                                                  \
            goto label;                                                                                      \
        }                                                                                                    \
        else {                                                                                               \
            (err_var) = eRet_Ok;                                                                             \
        }                                                                                                    \
    } while (0)

/* if pmlib error, print error, set given err_var with atlas error code, and goto given label */
#define CHECK_PMLIB_ERROR(err_str, pm_err_code)                                                              \
    do {                                                                                                     \
        if (0 != (pm_err_code)) {                                                                            \
            BDBG_ERR(("pmlib ERROR: %s - code:%d at %s: %d", (err_str), (pm_err_code), __FILE__, __LINE__)); \
        }                                                                                                    \
    } while (0)

/* if b_os error, print error, set given err_var with atlas error code, and goto given label */
#define CHECK_BOS_ERROR_GOTO(err_str, err_var, berr_code, label)                                              \
    do {                                                                                                      \
        if (B_ERROR_SUCCESS != (berr_code)) {                                                                 \
            BDBG_ERR(("B_OS_LIB ERROR: %s - code:%d at %s: %d", (err_str), (berr_code), __FILE__, __LINE__)); \
            (err_var) = eRet_ExternalError;                                                                   \
            goto label;                                                                                       \
        }                                                                                                     \
        else {                                                                                                \
            (err_var) = eRet_Ok;                                                                              \
        }                                                                                                     \
    } while (0)

/* if b_os error, print warning, set given err_var with atlas error code, and goto given label */
#define CHECK_BOS_WARN_GOTO(err_str, err_var, berr_code, label)                                              \
    do {                                                                                                     \
        if (B_ERROR_SUCCESS != (berr_code)) {                                                                \
            BDBG_WRN(("B_OS_LIB WARN: %s - code:%d at %s: %d", (err_str), (berr_code), __FILE__, __LINE__)); \
            (err_var) = eRet_ExternalError;                                                                  \
            goto label;                                                                                      \
        }                                                                                                    \
        else {                                                                                               \
            (err_var) = eRet_Ok;                                                                             \
        }                                                                                                    \
    } while (0)

/* if b_os error, print warning, set given err_var with atlas error code */
#define CHECK_BOS_WARN(err_str, err_var, berr_code)                                                          \
    do {                                                                                                     \
        if (B_ERROR_SUCCESS != (berr_code)) {                                                                \
            BDBG_WRN(("B_OS_LIB WARN: %s - code:%d at %s: %d", (err_str), (berr_code), __FILE__, __LINE__)); \
            (err_var) = eRet_ExternalError;                                                                  \
        }                                                                                                    \
        else {                                                                                               \
            (err_var) = eRet_Ok;                                                                             \
        }                                                                                                    \
    } while (0)

/* if b_os error, print message, set given err_var with atlas error code, and goto given label */
#define CHECK_BOS_MSG_GOTO(err_str, err_var, berr_code, label)                                               \
    do {                                                                                                     \
        if (B_ERROR_SUCCESS != (berr_code)) {                                                                \
            BDBG_MSG(("B_OS_LIB WARN: %s - code:%d at %s: %d", (err_str), (berr_code), __FILE__, __LINE__)); \
            (err_var) = eRet_ExternalError;                                                                  \
            goto label;                                                                                      \
        }                                                                                                    \
        else {                                                                                               \
            (err_var) = eRet_Ok;                                                                             \
        }                                                                                                    \
    } while (0)

#ifdef NETAPP_SUPPORT
/* if netapp error, print error, set given err_var with atlas error code, and goto given label */
#define CHECK_NETAPP_ERROR_GOTO(err_str, err_var, netapp_err_code, label)                                         \
    do {                                                                                                          \
        if (0 != (netapp_err_code)) {                                                                             \
            BDBG_ERR(("netapp ERROR: %s - code:%d at %s: %d", (err_str), (netapp_err_code), __FILE__, __LINE__)); \
            (err_var) = eRet_ExternalError;                                                                       \
            goto label;                                                                                           \
        }                                                                                                         \
        else {                                                                                                    \
            (err_var) = eRet_Ok;                                                                                  \
        }                                                                                                         \
    } while (0)

/* if netapp error, print error, set given err_var with atlas error code, and goto given label */
#define CHECK_NETAPP_ERR_GOTO(err_str, netapp_err_code, label)                                                    \
    do {                                                                                                          \
        if (0 != (netapp_err_code)) {                                                                             \
            BDBG_ERR(("netapp ERROR: %s - code:%d at %s: %d", (err_str), (netapp_err_code), __FILE__, __LINE__)); \
            goto label;                                                                                           \
        }                                                                                                         \
    } while (0)

/* if netapp error, print error, set given err_var with atlas error code, and goto given label */
#define CHECK_NETAPP_ERROR(err_str, netapp_err_code)                                                              \
    do {                                                                                                          \
        if (0 != (netapp_err_code)) {                                                                             \
            BDBG_ERR(("netapp ERROR: %s - code:%d at %s: %d", (err_str), (netapp_err_code), __FILE__, __LINE__)); \
        }                                                                                                         \
    } while (0)
#endif /* ifdef NETAPP_SUPPORT */

/* if atlas error, print error and continue */
#define CHECK_ERROR(err_str, err_code)                                                                    \
    do {                                                                                                  \
        if (eRet_Ok != (err_code)) {                                                                      \
            BDBG_ERR(("ATLAS ERROR: %s - code:%d at %s: %d", (err_str), (err_code), __FILE__, __LINE__)); \
        }                                                                                                 \
    } while (0)

/* if atlas error, print error and goto given label */
#define CHECK_ERROR_GOTO(err_str, err_code, label)                                                        \
    do {                                                                                                  \
        if (eRet_Ok != (err_code)) {                                                                      \
            BDBG_ERR(("ATLAS ERROR: %s - code:%d at %s: %d", (err_str), (err_code), __FILE__, __LINE__)); \
            goto label;                                                                                   \
        }                                                                                                 \
    } while (0)

/* if atlas error, print warning and goto given label */
#define CHECK_WARN(err_str, err_code)                                                                    \
    do {                                                                                                 \
        if (eRet_Ok != (err_code)) {                                                                     \
            BDBG_WRN(("ATLAS WARN: %s - code:%d at %s: %d", (err_str), (err_code), __FILE__, __LINE__)); \
        }                                                                                                \
    } while (0)

/* if atlas error, print warning and goto given label */
#define CHECK_WARN_GOTO(err_str, err_code, label)                                                        \
    do {                                                                                                 \
        if (eRet_Ok != (err_code)) {                                                                     \
            BDBG_WRN(("ATLAS WARN: %s - code:%d at %s: %d", (err_str), (err_code), __FILE__, __LINE__)); \
            goto label;                                                                                  \
        }                                                                                                \
    } while (0)

/* if ptr equals NULL, print error, set given err_code to err_var */
#define CHECK_PTR_ERROR(err_str, ptr, err_var, err_code)                                           \
    do {                                                                                           \
        if (NULL == (ptr)) {                                                                       \
            BDBG_ERR(("ATLAS ERROR: %s - NULL pointer at %s: %d", (err_str), __FILE__, __LINE__)); \
            (err_var) = (err_code);                                                                \
        }                                                                                          \
    } while (0)

/* if ptr equals NULL, print warning, set given err_code to err_var */
#define CHECK_PTR_WARN(err_str, ptr, err_var, err_code)                                           \
    do {                                                                                          \
        if (NULL == (ptr)) {                                                                      \
            BDBG_WRN(("ATLAS WARN: %s - NULL pointer at %s: %d", (err_str), __FILE__, __LINE__)); \
            (err_var) = (err_code);                                                               \
        }                                                                                         \
    } while (0)

/* if ptr equals NULL, print error, set given err_code to err_var and goto given label */
#define CHECK_PTR_ERROR_GOTO(err_str, ptr, err_var, err_code, label)                               \
    do {                                                                                           \
        if (NULL == (ptr)) {                                                                       \
            BDBG_ERR(("ATLAS ERROR: %s - NULL pointer at %s: %d", (err_str), __FILE__, __LINE__)); \
            (err_var) = (err_code);                                                                \
            goto label;                                                                            \
        }                                                                                          \
    } while (0)

/* if ptr equals NULL, print warn, set given err_code to err_var and goto given label */
#define CHECK_PTR_WARN_GOTO(err_str, ptr, err_var, err_code, label)                               \
    do {                                                                                          \
        if (NULL == (ptr)) {                                                                      \
            BDBG_WRN(("ATLAS WARN: %s - NULL pointer at %s: %d", (err_str), __FILE__, __LINE__)); \
            (err_var) = (err_code);                                                               \
            goto label;                                                                           \
        }                                                                                         \
    } while (0)

/* if ptr equals NULL, print message, set given err_code to err_var and goto given label */
#define CHECK_PTR_MSG_GOTO(err_str, ptr, err_var, err_code, label)                               \
    do {                                                                                         \
        if (NULL == (ptr)) {                                                                     \
            BDBG_MSG(("ATLAS MSG: %s - NULL pointer at %s: %d", (err_str), __FILE__, __LINE__)); \
            (err_var) = (err_code);                                                              \
            goto label;                                                                          \
        }                                                                                        \
    } while (0)

/* conversion from an Enum to MString */
#define ENUM_TO_MSTRING_DECLARE(methodname, ntype) \
    MString methodname(ntype var);                 \

#define ENUM_TO_MSTRING_INIT_CPP(classname, methodname, ntype) \
    MString classname::methodname(ntype var)

#define ENUM_TO_MSTRING_INIT_C(methodname, ntype) \
    MString methodname(ntype var)

#define ENUM_TO_MSTRING_START()                        \
    {                                                  \
        static MString     unknownValue = "Undefined"; \
        static MStringHash hash;                       \
        static bool        firstRun = true;            \
        if (true == firstRun)                          \
        {                                              \
            firstRun = false;

#define ENUM_TO_MSTRING_ENTRY(nvalue, strvalue)  hash.add(MString(nvalue).s(), strvalue);

#define ENUM_TO_MSTRING_END()                                                                     \
    }                                                                                             \
    MString strLookup = hash.get(MString(var).s());                                               \
    if (strLookup.isNull())                                                                       \
    {                                                                                             \
        BDBG_WRN(("%s enum:%d is missing corresponding valid string value.", __FUNCTION__, var)); \
        strLookup = unknownValue;                                                                 \
    }                                                                                             \
    return strLookup;                                                                             \
    }

/* conversion from MString to enum */
#define STRING_TO_ENUM_DECLARE(methodname, ntype)  ntype methodname(const char * str);

#define STRING_TO_ENUM_INIT_CPP(classname, methodname, ntype) \
    ntype classname::methodname(const char * str)

#define STRING_TO_ENUM_INIT_C(methodname, ntype) \
    ntype methodname(const char * str)

#define STRING_TO_ENUM_START()              \
    {                                       \
        static MStringHash hash;            \
        static bool        firstRun = true; \
                                            \
        if (true == firstRun)               \
        {                                   \
            firstRun = false;

#define STRING_TO_ENUM_ENTRY(strvalue, nvalue)  hash.add(strvalue, MString(nvalue).s());

#define STRING_TO_ENUM_END(ntype)                                                                \
    }                                                                                            \
    MString strLookup = hash.get(str);                                                           \
    if (strLookup.isNull())                                                                      \
    {                                                                                            \
        BDBG_WRN(("%s string:%s is missing corresponding valid enum value", __FUNCTION__, str)); \
    }                                                                                            \
    return (ntype)(strLookup.toInt());                                                           \
    }

#ifdef __cplusplus
}
#endif

#endif /* ATLAS_H__ */