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
/************************* INCLUDES ****************************************************/
#include "bbSysCrc.h"
#include "bbHalUartMailbox.h"

/************************* DEFINITIONS *************************************************/
#define HAL_UART_MAILBOX_MAGIC_WORD          0xA35B3A79U

/************************* IMPLEMENTATION **********************************************/
static void halUartMailboxSendNextChunk(HAL_UsartDescriptor_t *const usart);
static void halUartMailboxReceiveData(HAL_UsartDescriptor_t *const usart);

/************************************************************************************//**
    \brief Initialize FIFO driver.
    \param[in] descr - UART FIFO descriptor.
****************************************************************************************/
void HAL_UartMailboxInit(HAL_UartMailboxDescriptor_t *const descr)
{
    SYS_DbgAssertComplex(descr->offlineCallback, HALUARTMAILBOX_HALFIFOPROXYINIT_0);
    SYS_DbgAssertComplex(descr->rtsCallback, HALUARTMAILBOX_HALFIFOPROXYINIT_1);
    SYS_DbgAssertComplex(descr->rxCallback, HALUARTMAILBOX_HALFIFOPROXYINIT_2);

    /* Initialize TX/RX buffers. */
    memset(&descr->txBuffer, 0U, sizeof(descr->txBuffer));
    memset(&descr->rxBuffer, 0U, sizeof(descr->rxBuffer));

    /* Prepare UART. */
    SYS_FifoFillDescriptor(&descr->usart.txFifo, descr->usartTxBuffer, sizeof(descr->usartTxBuffer));
    SYS_FifoFillDescriptor(&descr->usart.rxFifo, descr->usartRxBuffer, sizeof(descr->usartRxBuffer));
    descr->usart.txCallback = halUartMailboxSendNextChunk;
    descr->usart.rxCallback = halUartMailboxReceiveData;
    descr->usart.channel = HAL_UART_MAILBOX_CHANNEL;
    HAL_UsartOpen(&descr->usart);

    descr->rtsCallback(descr);
}

/************************************************************************************//**
    \brief Close given FIFO descriptor.
    \param[in] descr - UART FIFO descriptor.
****************************************************************************************/
void HAL_UartMailboxClose(HAL_UartMailboxDescriptor_t *const descr)
{
    HAL_UsartClose(&descr->usart);
}

/************************************************************************************//**
    \brief Puts a piece of data to the TX FIFO.
    \note Generates assert if conveyed chunk of data doesn't fit to the free part
          of TX FIFO.
          Generates assert if TX FIFO is not held for transmission (unexpected call).
    \param[in] descr - UART FIFO descriptor.
    \parem[in] data - pointer to the chunk of data.
    \parem[in] dataLength - data chunk length.
****************************************************************************************/
void HAL_UartMailboxTx(HAL_UartMailboxDescriptor_t *const descr, const uint8_t *data, uint8_t dataLength)
{
    SYS_DbgAssertComplex(HAL_UART_MAILBOX_IDLE == descr->txBuffer.state
                  || HAL_UART_MAILBOX_COMPILE_MESSAGE == descr->txBuffer.state,
                  HALUARTMAILBOX_SEND_0);

    if (HAL_UART_MAILBOX_IDLE == descr->txBuffer.state)
    {
        descr->txBuffer.state = HAL_UART_MAILBOX_COMPILE_MESSAGE;
        descr->txBuffer.offset = 0;
        descr->txBuffer.preamble = HAL_UART_MAILBOX_MAGIC_WORD;
        descr->txBuffer.messageLength = 0;
    }

    SYS_DbgAssertComplex(HAL_UART_MAILBOX_MAX_DATA_LENGTH >= (descr->txBuffer.messageLength + dataLength),
                  HALUARTMAILBOX_SEND_1);

    memcpy(descr->txBuffer.message + descr->txBuffer.messageLength, data, dataLength);
    descr->txBuffer.messageLength += dataLength;
}

/************************************************************************************//**
    \brief Returns a size of the remaining free place in TX FIFO.
    \param[in] descr - UART FIFO descriptor.
    \return Size of the remaining free place in TX FIFO.
****************************************************************************************/
uint8_t HAL_UartMailboxTxFifoAvailableSize(HAL_UartMailboxDescriptor_t *const descr)
{
    SYS_DbgAssertComplex(HAL_UART_MAILBOX_MAX_DATA_LENGTH >= descr->txBuffer.messageLength,
                  HALUARTMAILBOX_AVATXFIFOILABLESIZE_0);

    return HAL_UART_MAILBOX_MAX_DATA_LENGTH - descr->txBuffer.messageLength;
}

/************************************************************************************//**
    \brief Finalizes the transmission. Sends the "full" interrupt to the destination
           subsystem.
    \param[in] descr - UART FIFO descriptor.
****************************************************************************************/
void HAL_UartMailboxTxEnd(HAL_UartMailboxDescriptor_t *const descr, HAL_HostId_t destSubsystem)
{
    SYS_DbgAssertComplex(HAL_UART_MAILBOX_COMPILE_MESSAGE == descr->txBuffer.state, HALUARTMAILBOX_TXEND_0);

    /* Calculate CRC. */
    descr->txBuffer.crc = SYS_Crc16(0, &descr->txBuffer.messageLength, sizeof(descr->txBuffer.messageLength));
    descr->txBuffer.crc = SYS_Crc16(descr->txBuffer.crc, descr->txBuffer.message, descr->txBuffer.messageLength);

    descr->txBuffer.state = HAL_UART_MAILBOX_TX_STARTED;
    halUartMailboxSendNextChunk(&descr->usart);

    (void) destSubsystem; /* Not used in UART Mailbox implementation. */
}

/************************************************************************************//**
    \brief Sends to the UART as much data as it can handle.
    \param[in] descr - UART FIFO descriptor.
    \parem[in] data - pointer to the chunk of data.
    \parem[in] dataLength - data chunk length.
    \parem[in,out] threshold - the expected value of TX buffer offset if all conveyed
                               data is written successfully to the UART. If the threshold
                               is already bigger than current offset no writing to the UART
                               will be performed.
    \return True if all conveyed data is written successfully to the UART,
            false otherwise.
****************************************************************************************/
static bool halUartMailboxSendPart(HAL_UartMailboxDescriptor_t *const descr,
                                   uint8_t *const data, uint8_t dataLength, uint32_t *const threshold)
{
    const uint32_t oldThreshold = *threshold;
    *threshold += dataLength;
    if (*threshold > descr->txBuffer.offset)
    {
        const uint8_t remaining = *threshold - descr->txBuffer.offset;
        const uint8_t localoffset = descr->txBuffer.offset - oldThreshold;
        descr->txBuffer.offset += HAL_UsartWrite(&descr->usart,
                                  data + localoffset, remaining);
        return (*threshold == descr->txBuffer.offset);
    }
    return true;
}

/************************************************************************************//**
    \brief Performs an attempt to send the message to UART. Called as a callback
           when UART is ready to send new data.
    \param[in] usart - pointer to the UART descriptor. (Should be a part of UART Mailbox Descriptor)
****************************************************************************************/
static void halUartMailboxSendNextChunk(HAL_UsartDescriptor_t *const usart)
{
    HAL_UartMailboxDescriptor_t *const descr = GET_PARENT_BY_FIELD(HAL_UartMailboxDescriptor_t, usart, usart);
    uint32_t threshold = 0;

    if (HAL_UART_MAILBOX_TX_FINISHED == descr->txBuffer.state)
    {
        descr->txBuffer.state = HAL_UART_MAILBOX_IDLE;
        SYS_DbgAssertComplex(descr->rtsCallback, HALUARTMAILBOX_SENDNEXTCHUNK_1);
        descr->rtsCallback(descr);
    }
    else if (HAL_UART_MAILBOX_TX_STARTED == descr->txBuffer.state
             && halUartMailboxSendPart(descr, (uint8_t *)&descr->txBuffer.preamble, sizeof(descr->txBuffer.preamble), &threshold)
             && halUartMailboxSendPart(descr, (uint8_t *)&descr->txBuffer.messageLength, sizeof(descr->txBuffer.messageLength), &threshold)
             && halUartMailboxSendPart(descr, (uint8_t *)&descr->txBuffer.message, descr->txBuffer.messageLength, &threshold)
             && halUartMailboxSendPart(descr, (uint8_t *)&descr->txBuffer.crc, sizeof(descr->txBuffer.crc), &threshold))
    {
        descr->txBuffer.state = HAL_UART_MAILBOX_TX_FINISHED;
    }
}

/************************************************************************************//**
    \brief Handles the received data. Called as a callback when UART receives a piece
           of data.
    \param[in] usart - pointer to the UART descriptor. (Should be a part of UART Mailbox Descriptor)
****************************************************************************************/
static void halUartMailboxReceiveData(HAL_UsartDescriptor_t *const usart)
{
    HAL_UartMailboxDescriptor_t *const descr = GET_PARENT_BY_FIELD(HAL_UartMailboxDescriptor_t, usart, usart);

    switch (descr->rxBuffer.state)
    {
        case (HAL_UART_MAILBOX_IDLE):
            descr->rxBuffer.preamble = 0;
            descr->rxBuffer.offset = 0;
            descr->rxBuffer.state = HAL_UART_MAILBOX_PREAMBLE;

        case (HAL_UART_MAILBOX_PREAMBLE):
        {
            while (descr->rxBuffer.preamble != HAL_UART_MAILBOX_MAGIC_WORD)
            {
                uint8_t byte;
                uint8_t read = HAL_UsartRead(&descr->usart, &byte, sizeof(uint8_t));
                if (read)
                {
                    descr->rxBuffer.preamble >>= 8;
                    ((uint8_t *)&descr->rxBuffer.preamble)[3] = byte;
                }
                else
                    break;
            }
            if (descr->rxBuffer.preamble != HAL_UART_MAILBOX_MAGIC_WORD)
                break;
            descr->rxBuffer.state = HAL_UART_MAILBOX_LENGTH;
        }
        case (HAL_UART_MAILBOX_LENGTH):
        {
            if (!HAL_UsartRead(&descr->usart, &descr->rxBuffer.messageLength, 1U))
                break;
            descr->rxBuffer.state = HAL_UART_MAILBOX_MESSAGE;
        }
        case (HAL_UART_MAILBOX_MESSAGE):
        {
            uint8_t read;
            do
            {
                SYS_DbgAssertComplex(sizeof(descr->rxBuffer.message) >= descr->rxBuffer.offset, HALUARTMAILBOX_RECEIVEDATA_0);
                SYS_DbgAssertComplex(descr->rxBuffer.messageLength >= descr->rxBuffer.offset, HALUARTMAILBOX_RECEIVEDATA_1);
                uint8_t toRead = descr->rxBuffer.messageLength - descr->rxBuffer.offset;
                read = HAL_UsartRead(&descr->usart,
                                     &(descr->rxBuffer.message[descr->rxBuffer.offset]),
                                     toRead);
                descr->rxBuffer.offset += read;
            }
            while (read && descr->rxBuffer.messageLength != descr->rxBuffer.offset);

            if (descr->rxBuffer.messageLength != descr->rxBuffer.offset)
                break;
            descr->rxBuffer.state = HAL_UART_MAILBOX_CRC;
        }
        case (HAL_UART_MAILBOX_CRC):
        {
            uint8_t read;
            const uint8_t crcThreshold = descr->rxBuffer.messageLength + sizeof(descr->rxBuffer.crc);
            do
            {
                uint8_t toRead = crcThreshold - descr->rxBuffer.offset;
                read = HAL_UsartRead(&descr->usart,
                                     (uint8_t *)&descr->rxBuffer.crc + (descr->rxBuffer.offset - descr->rxBuffer.messageLength),
                                     toRead);
                descr->rxBuffer.offset += read;
            }
            while (read && crcThreshold != descr->rxBuffer.offset);

            if (crcThreshold != descr->rxBuffer.offset)
                break;
            descr->rxBuffer.offset = 0;
            {
                uint16_t crc = SYS_Crc16(0, &descr->rxBuffer.messageLength, sizeof(descr->rxBuffer.messageLength));
                crc = SYS_Crc16(crc, descr->rxBuffer.message, descr->rxBuffer.messageLength);
                if (crc != descr->rxBuffer.crc)
                {
                    descr->rxBuffer.state = HAL_UART_MAILBOX_IDLE;
                    break;
                }
            }

            descr->rxBuffer.state = HAL_UART_MAILBOX_RX_FINISHED;
        }
        case (HAL_UART_MAILBOX_RX_FINISHED):
        {
            descr->rxBuffer.state = HAL_UART_MAILBOX_WAIT_FOR_RX_END;

            SYS_DbgAssertComplex(descr->rxCallback, HALUARTMAILBOX_RECEIVEDATA_3);
            descr->rxCallback(descr);
        }
        case (HAL_UART_MAILBOX_WAIT_FOR_RX_END):
            break;

        case HAL_UART_MAILBOX_COMPILE_MESSAGE:
        case HAL_UART_MAILBOX_TX_STARTED:
        case HAL_UART_MAILBOX_TX_FINISHED:
        default:
            SYS_DbgAssertComplex(false, HALUARTMAILBOX_RECEIVEDATA_4);
    }
}

/************************************************************************************//**
    \brief Reads a piece of data from RX FIFO.
    \note Generates assert if size of data in RX FIFO is less than requested.
    \param[in] descr - UART FIFO descriptor.
    \parem[in,out] buffer - a pointer to the given buffer.
    \parem[in] length - number of bytes to be read.
****************************************************************************************/
void HAL_UartMailboxRx(HAL_UartMailboxDescriptor_t *const descr, uint8_t *buffer, uint8_t length)
{
    SYS_DbgAssertComplex(HAL_UART_MAILBOX_WAIT_FOR_RX_END == descr->rxBuffer.state, HALUARTMAILBOX_RX_0);
    SYS_DbgAssertComplex((descr->rxBuffer.messageLength - descr->rxBuffer.offset) >= length, HALUARTMAILBOX_RX_1);

    memcpy(buffer, descr->rxBuffer.message + descr->rxBuffer.offset, length);
    descr->rxBuffer.offset += length;
}

/************************************************************************************//**
    \brief Finalizes the reception procedure. Indicates that the message has been
           successfully read by higher layer.
    \note Generates assert if message was not read completely.
    \param[in] descr - UART FIFO descriptor.
****************************************************************************************/
void HAL_UartMailboxRxEnd(HAL_UartMailboxDescriptor_t *const descr)
{
    SYS_DbgAssertComplex(HAL_UART_MAILBOX_WAIT_FOR_RX_END == descr->rxBuffer.state, HALUARTMAILBOX_RXEND_0);
    SYS_DbgAssertComplex(descr->rxBuffer.messageLength == descr->rxBuffer.offset, HALUARTMAILBOX_RXEND_1);

    descr->rxBuffer.state = HAL_UART_MAILBOX_IDLE;
#if defined(_DEBUG_COMPLEX_)
    memset(descr->rxBuffer.message, 0, descr->rxBuffer.messageLength);
#endif
    halUartMailboxReceiveData(&descr->usart);
}

/* eof bbHalUartMailbox.c */