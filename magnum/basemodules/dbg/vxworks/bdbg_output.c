/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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
#if	BINT_NO_TASK_ISR
#include "logLib.h"
#endif

void 
BDBG_P_AssertFailed(const char *expr, const char *file, unsigned line)
{
    BDBG_P_PrintString("!!! Assert '%s' Failed at %s:%d\n", expr, file, line);
    BKNI_Fail();
}

BERR_Code 
BDBG_P_PrintError(const char *file, unsigned lineno, const char *error, BERR_Code error_no)
{
    if (error_no != BERR_SUCCESS) {
        BDBG_P_PrintString("!!!Error %s(%#x) at %s:%d\n", error, error_no, file, lineno);
    }
    return error_no;
}
 
int
BDBG_P_Vprintf(const char *fmt, va_list ap)
{
	return BKNI_Vprintf(fmt, ap);
}

void 
BDBG_P_PrintString(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

#if	BINT_NO_TASK_ISR
{
	static char g_logStr[250];

	/* This assumes that Message Logging loaded and enabled */
	BDBG_LOCK(); /* To prevent others from using the shared 'gp_logStr' */
	vsprintf( g_logStr, fmt, ap );
	logMsg((char *) "%s", (int) g_logStr, 0, 0, 0, 0, 0);
	BDBG_UNLOCK();
}
#else
    BDBG_P_Vprintf(fmt, ap);
#endif
    
    va_end( ap );

    return;
}

/* End of file */

