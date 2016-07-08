/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************
/*
* FILENAME: $Workfile: trunk/stack/common/HAL/include/bbHalMailbox.h $
*
* DESCRIPTION:
*   Contains the common definitions for Mailbox hardware.
*
* $Revision: 9364 $
* $Date: 2016-01-04 18:21:51Z $
*
****************************************************************************************/
#ifndef _HAL_MAILBOX_H
#define _HAL_MAILBOX_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Possible Mailbox interfaces.
 */
#define INTERFACE_UART          0
#define INTERFACE_HW_MAILBOX    1

/* Default Mailbox interface definition. */
#ifndef _MAILBOX_INTERFACE_
#   define _MAILBOX_INTERFACE_    INTERFACE_UART
#endif

/**//**
 * \brief Total capacity of the TX/RX FIFO.
 */
#define HAL_MAILBOX_TXRX_FIFO_CAPACITY  128

/************************* TYPES *******************************************************/
/**//**
 * \brief Destination host identifiers.
 */
typedef enum
{
    HAL_HW_FIFO_HOST_ID_STB = 1,
    HAL_HW_FIFO_HOST_ID_CM,
    HAL_HW_FIFO_HOST_ID_RG
} HAL_HostId_t;

/************************* INTERFACE ***************************************************/
/**//**
 * \brief Below is the global definition of HAL Mailbox interface.
 */
#if defined(__SoC__)
#   if INTERFACE_UART == _MAILBOX_INTERFACE_
#       include "bbHalUartMailbox.h"
#       define HAL_MailboxDescriptor_t           HAL_UartMailboxDescriptor_t
#       define HAL_MailboxInit                   HAL_UartMailboxInit
#       define HAL_MailboxClose                  HAL_UartMailboxClose
#       define HAL_MailboxTxFifoAvailableSize    HAL_UartMailboxTxFifoAvailableSize
#       define HAL_MailboxTx                     HAL_UartMailboxTx
#       define HAL_MailboxTxEnd                  HAL_UartMailboxTxEnd
#       define HAL_MailboxRx                     HAL_UartMailboxRx
#       define HAL_MailboxRxEnd                  HAL_UartMailboxRxEnd
#   elif INTERFACE_HW_MAILBOX == _MAILBOX_INTERFACE_
#       include "bbSocMailbox.h"
#       define HAL_MailboxDescriptor_t           SOC_HwMailboxDescriptor_t
#       define HAL_MailboxInit                   SOC_HwMailboxInit
#       define HAL_MailboxClose                  SOC_HwMailboxClose
#       define HAL_MailboxTxFifoAvailableSize    SOC_HwMailboxTxFifoAvailableSize
#       define HAL_MailboxTx                     SOC_HwMailboxTx
#       define HAL_MailboxTxEnd                  SOC_HwMailboxTxEnd
#       define HAL_MailboxRx                     SOC_HwMailboxRx
#       define HAL_MailboxRxEnd                  SOC_HwMailboxRxEnd
#   else
#       error Undefined or incorrect Mailbox interface.
#   endif
#elif defined(__ML507__)
#   if INTERFACE_UART == _MAILBOX_INTERFACE_
#       include "bbHalUartMailbox.h"
#       define HAL_MailboxDescriptor_t           HAL_UartMailboxDescriptor_t
#       define HAL_MailboxInit                   HAL_UartMailboxInit
#       define HAL_MailboxClose                  HAL_UartMailboxClose
#       define HAL_MailboxTxFifoAvailableSize    HAL_UartMailboxTxFifoAvailableSize
#       define HAL_MailboxTx                     HAL_UartMailboxTx
#       define HAL_MailboxTxEnd                  HAL_UartMailboxTxEnd
#       define HAL_MailboxRx                     HAL_UartMailboxRx
#       define HAL_MailboxRxEnd                  HAL_UartMailboxRxEnd
#   elif INTERFACE_HW_MAILBOX == _MAILBOX_INTERFACE_
#       error Incorrect Mailbox interface. ML507 board doesn\t have the Hardware Mailbox.
#   else
#       error Undefined or incorrect Mailbox interface.
#   endif
#else /* __i386__ */
#   ifndef _UNIT_TEST_
#       if INTERFACE_UART == _MAILBOX_INTERFACE_
#           include "bbHalUartMailbox.h"
#           define HAL_MailboxDescriptor_t           HAL_UartMailboxDescriptor_t
#           define HAL_MailboxInit                   HAL_UartMailboxInit
#           define HAL_MailboxClose                  HAL_UartMailboxClose
#           define HAL_MailboxTxFifoAvailableSize    HAL_UartMailboxTxFifoAvailableSize
#           define HAL_MailboxTx                     HAL_UartMailboxTx
#           define HAL_MailboxTxEnd                  HAL_UartMailboxTxEnd
#           define HAL_MailboxRx                     HAL_UartMailboxRx
#           define HAL_MailboxRxEnd                  HAL_UartMailboxRxEnd
#       elif INTERFACE_HW_MAILBOX == _MAILBOX_INTERFACE_
//#           warning Luxoft team need to review this one.
#           include "bbHostMailbox.h"
#           define HAL_MailboxDescriptor_t           HOST_HwMailboxDescriptor_t
#           define HAL_MailboxInit                   HOST_HwMailboxInit
#           define HAL_MailboxClose                  HOST_HwMailboxClose
#           define HAL_MailboxTxFifoAvailableSize    HOST_HwMailboxTxFifoAvailableSize
#           define HAL_MailboxTx                     HOST_HwMailboxTx
#           define HAL_MailboxTxEnd                  HOST_HwMailboxTxEnd
#           define HAL_MailboxRx                     HOST_HwMailboxRx
#           define HAL_MailboxRxEnd                  HOST_HwMailboxRxEnd
#       else
#           error Undefined or incorrect Mailbox interface.
#       endif // INTERFACE_UART
#   else
#       include "bbPcFakeMailbox.h"
#       define HAL_MailboxDescriptor_t           PC_FakeMailboxDescriptor_t
#       define HAL_MailboxInit                   PC_FakeMailboxInit
#       define HAL_MailboxClose                  PC_FakeMailboxClose
#       define HAL_MailboxTxFifoAvailableSize    PC_FakeMailboxTxFifoAvailableSize
#       define HAL_MailboxTx                     PC_FakeMailboxTx
#       define HAL_MailboxTxEnd                  PC_FakeMailboxTxEnd
#       define HAL_MailboxRx                     PC_FakeMailboxRx
#       define HAL_MailboxRxEnd                  PC_FakeMailboxRxEnd
#   endif // _UNIT_TEST_
#endif

#endif /* _HAL_MAILBOX_H */
/* eof bbSysQueue.h */
