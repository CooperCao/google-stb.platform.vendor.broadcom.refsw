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
 *      Debug asserts toolset implementation.
 *
*******************************************************************************/

/************************* INCLUDES *****************************************************/
#include "bbSysDbg.h"               /* Debug asserts toolset interface. */

#if defined(_DEBUG_)
# include "bbHalDbg.h"              /* Debugging Hardware interface. */
# include "bbHalUsart.h"            /* HAL UART interface */
# include "bbHalIrqCtrl.h"          /* Interrupt Controller Hardware interface. */
#endif

#if defined(_DEBUG_LOG_) || (defined(_DEBUG_) && (_DEBUG_CONSOLELOG_ >= 1))
# include <stdarg.h>                /* Macros and type for functions that require variable numbers of arguments. */
# include <stdio.h>                 /* Standard input/output facilities. */                                                 /* TODO: Eliminate when the vsnprintf() is substituted with a custom function. */
#endif


/************************* VALIDATIONS **************************************************/
#if !defined(_DEBUG_)
# error This unit shall be compiled only for the Debug build.
#endif


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Size of the null-terminating string buffer (including one byte for the
 *  trailing null symbol) for logged messages.
 */
#define SYS_DBG_LOG_BUFFER_SIZE     256


/************************* STATIC FUNCTIONS PROTOTYPES **********************************/
#if !defined(_DEBUG_LOG_) && defined(_DEBUG_) && (_DEBUG_CONSOLELOG_ >= 1)
/*************************************************************************************//**
 * \brief   Logs the custom formatted debugging string with auxiliary parameters.
 * \param[in]   format      Pointer to the constant string specifying the format of the
 *  logged message.
 * \param[in]   va_args     The variable length comma-separated list of parameters forming
 *  the custom warning message according to the printf arguments list specification.
*****************************************************************************************/
SYS_STATIC void sysDbgLogStr(const char *format, ...);
#endif /* !(_DEBUG_LOG_) && (_DEBUG_) && (_DEBUG_CONSOLELOG_ >= 1) */


/************************* IMPLEMENTATION ***********************************************/
#if defined(_DEBUG_)
/*
 * Logs error and halts program execution.
 */
void sysDbgHalt(const uint32_t errorUid /* , const char *const fileName, const uint32_t fileLine ) */
# if defined(_DEBUG_FILELINE_)
                                           , const char *const fileName, const uint32_t fileLine
# endif
                                                                                                 )
{
    HAL_IRQ_DISABLE();
#if defined(_USE_ASYNC_UART_) && (_MAILBOX_INTERFACE_ == 1)
    while(1);
#endif

# if defined(_DEBUG_FILELINE_)
#  if (_DEBUG_CONSOLELOG_ >= 1)
    sysDbgLogStr("HALT: %s(%d) - 0x%08X", fileName, fileLine, errorUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 1)
    HAL_DbgHalt(errorUid, fileName, fileLine);
#  endif

# else /* ! _DEBUG_FILELINE_ */
#  if (_DEBUG_CONSOLELOG_ >= 1)
    sysDbgLogStr("HALT: 0x%08X", errorUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 1)
    HAL_DbgHalt(errorUid);
#  endif

# endif /* ! _DEBUG_FILELINE_ */

    while(1);
}
#endif /* _DEBUG_ */


#if defined(_DEBUG_LOG_)
/*
 * Logs warning and proceeds with program execution.
 */
void sysDbgLogId(const uint32_t warningUid /* , const char *const fileName, const uint32_t fileLine ) */
# if defined(_DEBUG_FILELINE_)
                                              , const char *const fileName, const uint32_t fileLine
# endif
                                                                                                    )
{
# if defined(_DEBUG_FILELINE_)
#  if (_DEBUG_CONSOLELOG_ >= 2)
    sysDbgLogStr("WARN: %s(%d) - 0x%08X", fileName, fileLine, warningUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 2)
    HAL_DbgLogId(warningUid, fileName, fileLine);
#  endif

# else /* ! _DEBUG_FILELINE_ */
#  if (_DEBUG_CONSOLELOG_ >= 2)
    sysDbgLogStr("WARN: 0x%08X", warningUid);
#  endif
#  if (_DEBUG_HARNESSLOG_ >= 2)
    HAL_DbgLogId(warningUid);
#  endif

# endif /* ! _DEBUG_FILELINE_ */
}
#endif /* _DEBUG_LOG_ */


#if (defined(_DEBUG_LOG_) && (defined(_DEBUG_STDOUTLOG_) || defined(_DEBUG_STDERRLOG_))) || (defined(_DEBUG_) && (_DEBUG_CONSOLELOG_ >= 1))
/*
 * Logs custom formatted debugging string with auxiliary parameters.
 */
void sysDbgLogStr(const char *const format, ...)
{
    char     message[SYS_DBG_LOG_BUFFER_SIZE];      /* String buffer for the message to be logged. */
    va_list  args;                                  /* Pointer to the variable arguments list of this function. */

    va_start(args, format);
    vsnprintf(message, SYS_DBG_LOG_BUFFER_SIZE, format, args);                                                              /* TODO: Implement custom tiny formatted print. */
    va_end(args);

#ifdef _USE_ASYNC_UART_
    HAL_UsartDescriptor_t *ConsoleGetUartDesc();
    HAL_UsartWrite(ConsoleGetUartDesc(), (const uint8_t *)message, strlen(message));
#else
    HAL_DbgLogStr(message);
#endif
}
#endif /* (_DEBUG_LOG_) || ((_DEBUG_) && (_DEBUG_CONSOLELOG_ >= 1)) */


/* eof bbSysDbg.c */