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
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/HAL/src/bbHalUartMailbox.c $
*
* DESCRIPTION:
*   usart descr implementation.
*
* $Revision: 3802 $
* $Date: 2014-10-01 12:34:10Z $
*
****************************************************************************************/
#if defined(MAILBOX_UNIT_TEST)
/************************* INCLUDES ****************************************************/
#include "bbPcFakeMailbox.h"

/************************* IMPLEMENTATION **********************************************/
/************************************************************************************//**
    \brief Initialize FIFO driver.
    \param[in] descr - PC Fake Mailbox descriptor.
****************************************************************************************/
void PC_FakeMailboxInit(PC_FakeMailboxDescriptor_t *const descr)
{
    SYS_DbgAssert(descr->offlineCallback, PCFAKEMAILBOX_HALFIFOPROXYINIT_0);
    SYS_DbgAssert(descr->rtsCallback, PCFAKEMAILBOX_HALFIFOPROXYINIT_1);
    SYS_DbgAssert(descr->rxCallback, PCFAKEMAILBOX_HALFIFOPROXYINIT_2);

    /* Initialize TX/RX buffers. */
    memset(&descr->buffer, 0U, sizeof(descr->buffer));

    /* Prepare UART. */
    SYS_FifoFillDescriptor(&descr->fifo, descr->buffer, sizeof(descr->buffer));

    descr->rtsCallback(descr);
}

/************************************************************************************//**
    \brief Close given FIFO descriptor.
    \param[in] descr - PC Fake Mailbox descriptor.
****************************************************************************************/
void PC_FakeMailboxClose(PC_FakeMailboxDescriptor_t *const descr)
{
    (void)descr;
}

/************************************************************************************//**
    \brief Puts a piece of data to the TX FIFO.
    \note Generates assert if conveyed chunk of data doesn't fit to the free part
          of TX FIFO.
          Generates assert if TX FIFO is not held for transmission (unexpected call).
    \param[in] descr - PC Fake Mailbox descriptor.
    \parem[in] data - pointer to the chunk of data.
    \parem[in] dataLength - data chunk length.
****************************************************************************************/
void PC_FakeMailboxTx(PC_FakeMailboxDescriptor_t *const descr, const uint8_t *data, uint8_t dataLength)
{
    SYS_DbgAssert(PC_FAKE_MAILBOX_IDLE == descr->state
                  || PC_FAKE_MAILBOX_COMPILE_MESSAGE == descr->state,
                  PCFAKEMAILBOX_SEND_0);

    if (PC_FAKE_MAILBOX_IDLE == descr->state)
        descr->state = PC_FAKE_MAILBOX_COMPILE_MESSAGE;

    SYS_DbgAssert(PC_FAKE_MAILBOX_MAX_DATA_LENGTH >= (SYS_FifoDataSize(&descr->fifo) + dataLength),
                  PCFAKEMAILBOX_SEND_1);

    SYS_FifoWrite(&descr->fifo, data, dataLength);
}

/************************************************************************************//**
    \brief Returns a size of the remaining free place in TX FIFO.
    \param[in] descr - PC Fake Mailbox descriptor.
    \return Size of the remaining free place in TX FIFO.
****************************************************************************************/
uint8_t PC_FakeMailboxTxFifoAvailableSize(PC_FakeMailboxDescriptor_t *const descr)
{
    return SYS_FifoSpaceAvailable(&descr->fifo);
}

/************************************************************************************//**
    \brief Finalizes the transmission. Sends the "full" interrupt to the destination
           subsystem.
    \param[in] descr - PC Fake Mailbox descriptor.
****************************************************************************************/
void PC_FakeMailboxTxEnd(PC_FakeMailboxDescriptor_t *const descr, HAL_HostId_t destSubsystem)
{
    SYS_DbgAssert(PC_FAKE_MAILBOX_COMPILE_MESSAGE == descr->state, PCFAKEMAILBOX_TXEND_0);

    /* Perform a loopback */
    descr->state = PC_FAKE_MAILBOX_WAIT_FOR_RX_END;
    descr->rxCallback(descr);

    (void) destSubsystem; /* Not used in PC Fake Mailbox implementation. */
}

/************************************************************************************//**
    \brief Reads a piece of data from RX FIFO.
    \note Generates assert if size of data in RX FIFO is less than requested.
    \param[in] descr - PC Fake Mailbox descriptor.
    \parem[in,out] buffer - a pointer to the given buffer.
    \parem[in] length - number of bytes to be read.
****************************************************************************************/
void PC_FakeMailboxRx(PC_FakeMailboxDescriptor_t *const descr, uint8_t *buffer, uint8_t length)
{
    SYS_DbgAssert(PC_FAKE_MAILBOX_WAIT_FOR_RX_END == descr->state, PCFAKEMAILBOX_RX_0);
    SYS_DbgAssert(SYS_FifoDataSize(&descr->fifo) >= length, PCFAKEMAILBOX_RX_1);

    SYS_FifoRead(&descr->fifo, buffer, length);
}

/************************************************************************************//**
    \brief Finalizes the reception procedure. Indicates that the message has been
           successfully read by higher layer.
    \note Generates assert if message was not read completely.
    \param[in] descr - PC Fake Mailbox descriptor.
****************************************************************************************/
void PC_FakeMailboxRxEnd(PC_FakeMailboxDescriptor_t *const descr)
{
    SYS_DbgAssert(PC_FAKE_MAILBOX_WAIT_FOR_RX_END == descr->state, PCFAKEMAILBOX_RXEND_0);
    SYS_DbgAssert(0 == SYS_FifoDataSize(&descr->fifo), PCFAKEMAILBOX_RXEND_1);

    descr->state = PC_FAKE_MAILBOX_IDLE;
    descr->rtsCallback(descr);
}

#endif /* MAILBOX_UNIT_TEST */
/* eof bbHalUartMailbox.c */