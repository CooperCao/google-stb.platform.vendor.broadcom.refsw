/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      Debugging Hardware interface.
 *
*******************************************************************************/

#ifndef _BB_HAL_DBG_H
#define _BB_HAL_DBG_H

/************************* INCLUDES ***********************************************************************************/
#include "bbSysDbg.h"

#if defined (__i386__)
# include "bbPcDbg.h"
#endif

/************************* DEFINITIONS ********************************************************************************/
#if defined(_DEBUG_) && (_DEBUG_HARNESSLOG_ >= 1)
/*************************************************************************************//**
 * \brief   Logs error via the Mailbox interface and halts program execution until reset
 *  signal is received via the Mailbox, then restarts the application.
 * \param[in]   errorUid    Error identifier to be logged.
*****************************************************************************************/
extern void HAL__DbgHalt(const uint32_t errorUid);
# if defined(_DEBUG_FILELINE_)
/**//**
 * \brief   Logs error into the numeric debugging interface and halts program execution or waits for the external reset
 *  signal.
 * \param[in]   errorUid    Error identifier to be logged.
 * \param[in]   fileName    Pointer to the constant string with the name of the file containing triggered asserted
 *  expression.
 * \param[in]   fileLine    Number of the line in the file at which the asserted expression was validated.
 * \details On particular platforms with different options this macro may have different behavior on how and if the
 *  application is halted: put into the infinite loop or restarted, etc.
 * \details Numeric debugging on embedded platforms is performed via the Mailbox unit.
 */
#  if defined(__SoC__) || defined(__ML507__)
#   define HAL_DbgHalt(errorUid, fileName, fileLine)            HAL__DbgHalt(errorUid)
#  else /* __i386__ */
#   define HAL_DbgHalt(errorUid, fileName, fileLine)            PC_DbgHalt(errorUid, fileName, fileLine)
#  endif
#
# else /* ! _DEBUG_FILELINE_ */
/**//**
 * \brief   Logs error into the numeric debugging interface and halts program execution or waits for the external reset
 *  signal.
 * \param[in]   errorUid    Error identifier to be logged.
 * \details On particular platforms with different options this macro may have different behavior on how and if the
 *  application is halted: put into the infinite loop or restarted, etc.
 * \details Numeric debugging on embedded platforms is performed via the Mailbox unit.
 */
#  if defined(__SoC__) || defined(__ML507__)
#   define HAL_DbgHalt(errorUid)                                HAL__DbgHalt(errorUid)
#  else /* __i386__ */
#   error Numeric logging on i386 platform shall include file-line information (file name and line number).
#  endif
#
# endif /* ! _DEBUG_FILELINE_ */
#endif /* (_DEBUG_) && (_DEBUG_HARNESSLOG_ >= 1) */

#if defined(_DEBUG_LOG_) && (_DEBUG_HARNESSLOG_ >= 2)
/*************************************************************************************//**
 * \brief   Logs warning via the Mailbox interface and proceeds with program execution.
 * \param[in]   warningUid      Warning identifier to be logged.
*****************************************************************************************/
extern void HAL__DbgLogId(const uint32_t warningUid);
# if defined(_DEBUG_FILELINE_)
/**//**
 * \brief   Logs warning into the numeric debugging interface and proceeds with program execution.
 * \param[in]   warningUid      Warning identifier to be logged.
 * \param[in]   fileName        Pointer to the constant string with the name of the file containing triggered asserted
 *  expression.
 * \param[in]   fileLine        Number of the line in the file at which the asserted expression was validated.
 * \details On particular platforms with different options this macro may have different behavior on how and if the
 *  application execution is proceeded: proceed without pause, pause for logging output, or even halt, etc.
 * \details Numeric debugging on embedded platforms is performed via the Mailbox unit.
 */
#  if defined(__SoC__) || defined(__ML507__)
#   define HAL_DbgLogId(warningUid, fileName, fileLine)         HAL__DbgLogId(warningUid)
#  else /* __i386__ */
#   define HAL_DbgLogId(warningUid, fileName, fileLine)         PC_DbgLogId(warningUid, fileName, fileLine)
#  endif
#
# else /* ! _DEBUG_FILELINE_ */
/**//**
 * \brief   Logs warning into the numeric debugging interface and proceeds with program execution.
 * \param[in]   warningUid      Warning identifier to be logged.
 * \details On particular platforms with different options this macro may have different behavior on how and if the
 *  application execution is proceeded: proceed without pause, pause for logging output, or even halt, etc.
 * \details Numeric debugging on embedded platforms is performed via the Mailbox unit.
 */
#  if defined(__SoC__) || defined(__ML507__)
#   define HAL_DbgLogId(warningUid)                             HAL__DbgLogId(warningUid)
#  else /* __i386__ */
#   error Numeric logging on i386 platform shall include file-line information (file name and line number).
#  endif
#
# endif /* ! _DEBUG_FILELINE_ */
#endif /* (_DEBUG_LOG_) && (_DEBUG_HARNESSLOG_ >= 2) */

#if defined(_DEBUG_LOG_) || (defined(_DEBUG_) && (_DEBUG_CONSOLELOG_ >= 1))
/*************************************************************************************//**
 * \brief   Logs custom debugging message into the text console and proceeds with program
 *  execution.
 * \param[in]   message     Pointer to the constant string specifying message to be
 *  logged.
 * \details
 *  The text output of this function may be directed to stdout, stderr, or UART.
 * \details
 *  The trailing End-of-Line symbol is appended automatically.
*****************************************************************************************/
extern void HAL__DbgLogStr(const char *const message);
/**//**
 * \brief   Logs custom debugging message into the text console and proceeds with program execution.
 * \param[in]   message     Pointer to the constant string specifying message to be logged.
 * \details On particular platforms with different options this macro may have different behavior on how and if the
 *  warning message is logged: printed into the stdout or stderr, or transmitted via the UART, etc.
 * \details The trailing End-of-Line symbol is appended automatically.
 */
# if defined(__SoC__) || defined(__ML507__)
#  define HAL_DbgLogStr(message)                                HAL__DbgLogStr(message)
# else /* __i386__ */
#  define HAL_DbgLogStr(message)                                PC_DbgLogStr(message)
# endif
#
#endif /* (_DEBUG_LOG_) || ((_DEBUG_) && (_DEBUG_CONSOLELOG_ >= 1)) */

#endif /* _BB_HAL_DBG_H */

/* eof bbHalDbg.h */
