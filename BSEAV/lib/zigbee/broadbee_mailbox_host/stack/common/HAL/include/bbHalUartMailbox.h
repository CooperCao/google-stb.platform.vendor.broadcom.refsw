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
 *      Contains the definitions for UART Mailbox.
 *
*******************************************************************************/

#ifndef _HAL_UART_MAILBOX_H
#define _HAL_UART_MAILBOX_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysFifo.h"
#include "bbHalMailbox.h"
#include "bbHalUsart.h"

/************************* DEFINITIONS *************************************************/

/**//**
 * \brief The maximum possible length of UART Mailbox message payload.
 */
#define HAL_UART_MAILBOX_MAX_DATA_LENGTH    HAL_MAILBOX_TXRX_FIFO_CAPACITY // 128 bytes

/**//**
 * \brief The UART channel to be used.
 */
#define HAL_UART_MAILBOX_CHANNEL            HAL_USART_CHANNEL_0

/**//**
 * \brief The capacity of the TX FIFO.
 */
#define HAL_UART_MAILBOX_TX_FIFO_LENGTH     256

/**//**
 * \brief The capacity of the RX FIFO.
 */
#define HAL_UART_MAILBOX_RX_FIFO_LENGTH     256

/************************* TYPES *******************************************************/

/**//**
 * \brief Internal UART Mailbox states declaration.
 */
typedef enum
{
    HAL_UART_MAILBOX_IDLE = 0x00,

    /* States for TX buffer */
    HAL_UART_MAILBOX_COMPILE_MESSAGE,
    HAL_UART_MAILBOX_TX_STARTED,
    HAL_UART_MAILBOX_TX_FINISHED,

    /* States for RX buffer */
    HAL_UART_MAILBOX_PREAMBLE,
    HAL_UART_MAILBOX_LENGTH,
    HAL_UART_MAILBOX_MESSAGE,
    HAL_UART_MAILBOX_CRC,
    HAL_UART_MAILBOX_RX_FINISHED,
    HAL_UART_MAILBOX_WAIT_FOR_RX_END,
} HAL_UartMailboxState_t;

/**//**
 * \brief Type declaration of UART FIFO descriptor.
 */
typedef struct _HAL_UartMailboxDescriptor_t HAL_UartMailboxDescriptor_t;

/**//**
 * \brief Type declaration of a offline callback primitive.
 * \note Should be called when the Host system becomes not available.
 * \param[in] descr - the pointer to the UART FIFO descriptor structure.
 */
typedef void (*HAL_UartMailboxOfflineCallback_t)(HAL_UartMailboxDescriptor_t *const descr);

/**//**
 * \brief Type declaration of ready-to-send callback primitive.
 * \note Should be called when the TX FIFO is ready to send new message.
 *       It is not necessary to have TX FIFO completely free, since it is
 *       possible to write new message to the TX FIFO when the previous one is
 *       still there.
 * \param[in] descr - the pointer to the UART FIFO descriptor structure.
 */
typedef void (*HAL_UartMailboxReadyToSendCallback_t)(HAL_UartMailboxDescriptor_t *const descr);

/**//**
 * \brief Type declaration of data-received callback primitive.
 * \note Should be called from the H2Z "full" interrupt routine.
 * \param[in] descr - the pointer to the UART FIFO descriptor structure.
 */
typedef void (*HAL_UartMailboxDataReceivedCallback_t)(HAL_UartMailboxDescriptor_t *const descr);


/**//**
 * \brief UART Mailbox message structure.
 */
typedef struct _HAL_UartMailboxMessage_t
{
    /* Message structure. */
    uint32_t    preamble;
    uint8_t     messageLength;
    uint8_t     message[HAL_UART_MAILBOX_MAX_DATA_LENGTH];
    uint16_t    crc;

    /* Service parameters. */
    uint8_t                 offset;
    HAL_UartMailboxState_t  state;
} HAL_UartMailboxMessage_t;

/**//**
 * \brief Definition of the UART FIFO descriptor.
 */
typedef struct _HAL_UartMailboxDescriptor_t
{
    /* Internal RX/TX buffers */
    HAL_UartMailboxMessage_t txBuffer;
    HAL_UartMailboxMessage_t rxBuffer;

    /* Memory needed for UART module.  */
    HAL_UsartDescriptor_t usart;
    uint8_t usartTxBuffer[HAL_UART_MAILBOX_TX_FIFO_LENGTH];
    uint8_t usartRxBuffer[HAL_UART_MAILBOX_RX_FIFO_LENGTH];

    /* Offline callback. */
    HAL_UartMailboxOfflineCallback_t         offlineCallback;
    /* Ready-to-send callback. */
    HAL_UartMailboxReadyToSendCallback_t     rtsCallback;
    /* Data received callback. */
    HAL_UartMailboxDataReceivedCallback_t    rxCallback;
} HAL_UartMailboxDescriptor_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief Initialize FIFO driver.
    \param[in] descr - UART FIFO descriptor.
****************************************************************************************/
void HAL_UartMailboxInit(HAL_UartMailboxDescriptor_t *const descr);

/************************************************************************************//**
    \brief Close given FIFO descriptor.
    \param[in] descr - UART FIFO descriptor.
****************************************************************************************/
void HAL_UartMailboxClose(HAL_UartMailboxDescriptor_t *const descr);

/************************************************************************************//**
    \brief Puts a piece of data to the TX FIFO.
    \note Generates assert if conveyed chunk of data doesn't fit to the free part
          of TX FIFO.
          Generates assert if TX FIFO is not held for transmission (unexpected call).
    \param[in] descr - UART FIFO descriptor.
    \param[in] data - pointer to the chunk of data.
    \param[in] dataSize - data chunk length.
****************************************************************************************/
void HAL_UartMailboxTx(HAL_UartMailboxDescriptor_t *const descr, const uint8_t *data, uint8_t dataSize);

/************************************************************************************//**
    \brief Returns a size of the remaining free place in TX FIFO.
    \param[in] descr - UART FIFO descriptor.
    \return Size of the remaining free place in TX FIFO.
****************************************************************************************/
uint8_t HAL_UartMailboxTxFifoAvailableSize(HAL_UartMailboxDescriptor_t *const descr);

/************************************************************************************//**
    \brief Finalizes the transmission. Sends the "full" interrupt to the destination
           subsystem.
    \param[in] descr - UART FIFO descriptor.
****************************************************************************************/
void HAL_UartMailboxTxEnd(HAL_UartMailboxDescriptor_t *const descr, HAL_HostId_t destSubsystem);

/************************************************************************************//**
    \brief Reads a piece of data from RX FIFO.
    \note Generates assert if RX FIFO is empty (unexpected call).
          Generates assert if size of data in RX FIFO is less than requested.
    \param[in] descr - UART FIFO descriptor.
    \param[in, out] buffer - a pointer to the given buffer.
    \param[in] length - number of bytes to be read.
****************************************************************************************/
void HAL_UartMailboxRx(HAL_UartMailboxDescriptor_t *const descr, uint8_t *buffer, uint8_t length);

/************************************************************************************//**
    \brief Finalizes the reception procedure. Indicates that the message has been
           successfully read by higher layer.
    \note Generates assert if message was not read completely.
    \param[in] descr - UART FIFO descriptor.
****************************************************************************************/
void HAL_UartMailboxRxEnd(HAL_UartMailboxDescriptor_t *const descr);

#endif /* _HAL_UART_MAILBOX_H */

/* eof bbHalUartMailbox.h */
