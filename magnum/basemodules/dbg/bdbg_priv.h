/***************************************************************************
 *     Copyright (c) 2010-2013, Broadcom Corporation
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
#ifndef BDBG_PRIV_H
#define BDBG_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif
/* Private declarations starts here */
#define BDBG_NOP() (void)0
#ifndef BDBG_P_PRINTF_FORMAT
/* #error Bad */
#define BDBG_P_PRINTF_FORMAT(fmt,args)
#endif

#ifdef __GNUC__
#define BDBG_P_NORETURN __attribute__ ((noreturn))
#else
#define BDBG_P_NORETURN
#endif

void BDBG_P_PrintString(const char *fmt, ...) BDBG_P_PRINTF_FORMAT(1,2);
void BDBG_P_Vprintf_Log(BDBG_ModulePrintKind kind, const char *fmt, va_list ap);
BERR_Code BDBG_P_PrintError(const char *file, unsigned lineno, const char *error, BERR_Code error_no);
BERR_Code BDBG_P_PrintError_small(const char *file, unsigned lineno, BERR_Code error_no);
void BDBG_P_PrintErrorString_small(const char *file, unsigned lineno);
void BDBG_P_Assert(bool expr, const char *file, unsigned line);
void BDBG_P_AssertFailed(const char *expr, const char *file, unsigned line)
#if !B_REFSW_DEBUG_ASSERT_NO_FAIL
BDBG_P_NORETURN
#endif
    ;

#ifdef __cplusplus
}
#endif

#endif /* BDBG_PRIV_H */

