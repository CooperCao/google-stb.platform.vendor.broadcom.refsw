/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/HAL/src/bbHalUsart.c $
*
* DESCRIPTION:
*   usart implementation.
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/
/************************* INCLUDES ****************************************************/
#include "bbHalUsart.h"
#include "private/bbHalPrivateUsart.h"

/************************* IMPLEMENTATION **********************************************/
static void txInterruptHandler(void *const link);
static void rxInterruptHandler(void *const link);
static void raiseRxCallback(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
static void raiseTxCallback(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

typedef enum
{
    RAISE_TX_CALLBACK_STATE,
    RAISE_RX_CALLBACK_STATE,
} UsartHandlers_t;

static const SYS_SchedulerTaskHandler_t usartHandlers[] =
{
    [RAISE_TX_CALLBACK_STATE] = raiseTxCallback,
    [RAISE_RX_CALLBACK_STATE] = raiseRxCallback,
};

static void raiseTxCallback(SYS_SchedulerTaskDescriptor_t *const taskDescriptor)
{
    HAL_UsartDescriptor_t *const usart = GET_PARENT_BY_FIELD(HAL_UsartDescriptor_t, usartTask, taskDescriptor);

    SYS_DbgAssertComplex(usart->txCallback, HALUSART_RAISETXCALLBACK_0);
    usart->txCallback(usart);
}

static void raiseRxCallback(SYS_SchedulerTaskDescriptor_t *const taskDescriptor)
{
    HAL_UsartDescriptor_t *const usart = GET_PARENT_BY_FIELD(HAL_UsartDescriptor_t, usartTask, taskDescriptor);
    /* The python test doesn't repeat frames. In this case, missing bytes are issue. */
    SYS_DbgAssert(!SYS_FifoIsFull(&usart->rxFifo), HALUSART_RX_BUFFER_IS_FULL_MAYBE_SOME_BYTES_ARE_OVERRIDDEN);
    SYS_DbgAssertComplex(usart->rxCallback, HALUSART_RAISERXCALLBACK_0);
    usart->rxCallback(usart);
}

/************************************************************************************//**
    \brief Initializes usart
    \param[in] usart - usart descriptor.
****************************************************************************************/
void HAL_UsartOpen(HAL_UsartDescriptor_t *const usart)
{
    /* TODO: Probably SYS_DbgAssert(usart->txFifo.buffer, HALUSART_OPEN_0); */
    SYS_DbgAssert(&usart->txFifo, HALUSART_OPEN_0);
    /* TODO: Probably SYS_DbgAssert(usart->rxFifo.buffer, HALUSART_OPEN_1); */
    SYS_DbgAssert(&usart->rxFifo, HALUSART_OPEN_1);
    SYS_DbgAssert(usart->txCallback, HALUSART_OPEN_3);
    SYS_DbgAssert(usart->rxCallback, HALUSART_OPEN_4);

    usart->usartTask.priority = SYS_SCHEDULER_HAL_PRIORITY;
    usart->usartTask.handlers = usartHandlers;

    PLATFORM_UsartEnable(usart->channel, txInterruptHandler, rxInterruptHandler, (void *)usart);
    usart->state = OPENED_STATE;
}

/************************************************************************************//**
    \brief Closes usart
    \param[in] usart - usart descriptor.
****************************************************************************************/
void HAL_UsartClose(HAL_UsartDescriptor_t *const usart)
{
    PLATFORM_UsartDisable(usart->channel);
    memset(usart, 0, sizeof(*usart));
    usart->state = CLOSED_STATE;
}

/************************************************************************************//**
    \brief Puts more as possible bytes from software to the hardware FIFO and if
        the software FIFO is empty raises a callback.
    \param[in] usart - usart descriptor.
****************************************************************************************/
INLINE void sendNextChunk(HAL_UsartDescriptor_t *const usart)
{
    while (PLATFORM_UsartSendCapasity(usart->channel) && !SYS_FifoIsEmpty(&usart->txFifo))
        PLATFORM_UsartSendByte(usart->channel, SYS_FifoReadByte(&usart->txFifo));

    if (SYS_FifoIsEmpty(&usart->txFifo))
    {
        usart->state = OPENED_STATE;
        SYS_SchedulerPostTask(&usart->usartTask, RAISE_TX_CALLBACK_STATE);
    }
}

/************************************************************************************//**
    \brief Sends message
    \param[in] usart - usart descriptor.
    \param[in] data - data pointer.
    \param[in] dataLength - data length.

    \return number of sent bytes.
****************************************************************************************/
uint8_t HAL_UsartWrite(HAL_UsartDescriptor_t *const usart, const uint8_t *const data, const uint8_t dataLength)
{
    uint8_t result;

    SYS_DbgAssert(NULL != usart, HALUSART_SEND_00);
    SYS_DbgAssert(CLOSED_STATE != usart->state, HALUSART_SEND_0);

    SYS_DbgAssert(NULL != data, HALUSART_SEND_01);
    SYS_DbgAssert(dataLength, HALUSART_SEND_1);

#if (defined(_SOC_MAC_TEST_) || defined(_SOC_PHY_TEST_))
    result = 0; //Sent bytes
    do
    {
        const uint8_t remain = dataLength - result;
        result += SYS_FifoWrite(&usart->txFifo, (data + result), remain);

        ATOMIC_SECTION_ENTER(UART_WRITE2)
        {
            usart->state = IN_PROGRESS;
            sendNextChunk(usart);
        }
        ATOMIC_SECTION_LEAVE(UART_WRITE2)
    } while (result < dataLength);
#else
    result = SYS_FifoWrite(&usart->txFifo, data, dataLength);

    ATOMIC_SECTION_ENTER(UART_WRITE)
    {
        usart->state = IN_PROGRESS;
        sendNextChunk(usart);
    }
    ATOMIC_SECTION_LEAVE(UART_WRITE)
#endif
    return result;
}

static void txInterruptHandler(void *const link)
{
    HAL_UsartDescriptor_t *const usart = (HAL_UsartDescriptor_t *)link;
    if (IN_PROGRESS == usart->state)
        sendNextChunk(usart);
}

/************************************************************************************//**
    \brief Read message
    \param[in] usart - usart descriptor.
    \param[in] data - data pointer.
    \param[in] dataLength - data length.

    \return number read bytes
****************************************************************************************/
uint8_t HAL_UsartRead(HAL_UsartDescriptor_t *usart, uint8_t *const buffer, const uint8_t bufferLength)
{
    SYS_DbgAssert(CLOSED_STATE != usart->state, HALUSART_READ_0);

    return SYS_FifoRead(&usart->rxFifo, buffer, bufferLength);
}

static void rxInterruptHandler(void *const link)
{
    HAL_UsartDescriptor_t *const usart = (HAL_UsartDescriptor_t *)link;
    if (CLOSED_STATE != usart->state)
    {
        do
            SYS_FifoWriteByte(&usart->rxFifo, PLATFORM_UsartReceiveByte(usart->channel));
        while (PLATFORM_UsartGetRxDataSize(usart->channel));
        SYS_SchedulerPostTask(&usart->usartTask, RAISE_RX_CALLBACK_STATE);
    }
}

/* eof bbHalUsart.c */
