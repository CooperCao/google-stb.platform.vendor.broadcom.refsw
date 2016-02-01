/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/


#include "bstd.h"
#include "bkni.h"

/* coverity[+kill] */
void
BDBG_P_AssertFailed(const char *expr, const char *file, unsigned line)
{
#if B_REFSW_DEBUG_COMPACT_ERR
    BSTD_UNUSED(expr);
    BDBG_P_PrintString("!!! Assert at %s:%d\n", file, line);
#ifdef BDBG_ASSERT_TO_KNI
    BKNI_Printf("!!! Assert at %s:%d\n", file, line);
#endif
#else
    BDBG_P_PrintString("!!! Assert '%s' Failed at %s:%d\n", expr, file, line);
#ifdef BDBG_ASSERT_TO_KNI
    BKNI_Printf("!!! Assert '%s' Failed at %s:%d\n", expr, file, line);
#endif
#endif
#if !B_REFSW_DEBUG_ASSERT_NO_FAIL    
    BKNI_Fail();
#endif
}

BERR_Code
BDBG_P_PrintError(const char *file, unsigned lineno, const char *error, BERR_Code error_no)
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
        BDBG_P_PrintString("!!!Error %s(%#x) at %s:%d\n", error, error_no, file, lineno);
    }
    return error_no;
}

void BDBG_P_PrintErrorString_small(const char *file, unsigned lineno)
{
    BDBG_P_PrintString("### Error at %s:%d\n", file, lineno);
}

BERR_Code BDBG_P_PrintError_small(const char *file, unsigned lineno, BERR_Code error_no)
{
    return BDBG_P_PrintError(file, lineno, NULL, error_no);
}

#ifdef BDBG_ANDROID_LOG
#include <cutils/log.h>
#endif

void
BDBG_P_PrintString(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    #ifdef BDBG_ANDROID_LOG
    /* These are always used for errors in bdbg */
    LOG_PRI_VA(ANDROID_LOG_ERROR, "NEXUS", fmt, ap);
    #else
    BDBG_P_Vprintf_Log(BDBG_ModulePrintKind_eString, fmt, ap);
    #endif

    va_end( ap );

    return;
}

/* End of file */
