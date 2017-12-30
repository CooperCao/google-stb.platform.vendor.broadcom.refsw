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

void BDBG_P_PrintString_isrsafe(const char *fmt, ...) BDBG_P_PRINTF_FORMAT(1,2);
#define BDBG_P_PrintString BDBG_P_PrintString_isrsafe
void BDBG_P_Vprintf_Log_isrsafe(BDBG_ModulePrintKind kind, const char *fmt, va_list ap);
BERR_Code BDBG_P_PrintError_isrsafe(const char *file, unsigned lineno, const char *error, BERR_Code error_no);
BERR_Code BDBG_P_PrintError_small_isrsafe(const char *file, unsigned lineno, BERR_Code error_no);
void BDBG_P_PrintErrorString_small_isrsafe(const char *file, unsigned lineno);
void BDBG_P_Assert_isrsafe(bool expr, const char *file, unsigned line);
#define BDBG_P_AssertFailed  BDBG_P_AssertFailed_isrsafe
void BDBG_P_AssertFailed_isrsafe(const char *expr, const char *file, unsigned line)
#if !defined B_REFSW_DEBUG_ASSERT_NO_FAIL
BDBG_P_NORETURN
#endif
    ;

#ifdef __cplusplus
}
#endif

#endif /* BDBG_PRIV_H */

