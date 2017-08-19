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
 *
 *
*******************************************************************************/

#ifndef _UNIT_TEST_

/************************* INCLUDES *****************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "bbHostMailbox.h"
#include "zigbee_ioctl.h"
#include "bbSysBasics.h"


/************************* static variables ***************************************/
static HOST_HwMailboxDescriptor_t *hostMailboxDesc = NULL;

/************************* static function **********************************************/

INLINE HOST_HwMailboxDescriptor_t **hostHwMailboxDesc() { return &hostMailboxDesc; }

/************************************************************************************//**
    \brief Should be installed as rx fifo full interrupt handler.
    \ read the data from HW fifo into SW fifo ASAP to release the HW
****************************************************************************************/
static void hostRxInterruptHandler(HOST_HwMailboxDescriptor_t *const descr)
{
    uint32_t dp;
    uint32_t ret;
    uint8_t data[HAL_MAILBOX_TXRX_FIFO_CAPACITY];

    while(1){
        memset(data, 0, sizeof(data));
        ret = Zigbee_Ioctl_ReadFromMbox(descr->zigbeeDeviceFd, data);
        if((int)ret < 0)
            continue;
        /*
         * To support multiple thread, the semaphore is needed since only have one rxfifo.
         */
        pthread_mutex_lock(&descr->rxFifoMutex);
        SYS_FifoWrite(&descr->rxFifo, data, sizeof(data));
        descr->rxCallback(descr);
    }
}

static void hostCreateRxThread(HOST_HwMailboxDescriptor_t *const descr)
{
    pthread_attr_t threadAttr;
    struct sched_param schedParam;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setschedpolicy(&threadAttr, SCHED_FIFO);
    pthread_attr_getschedparam(&threadAttr, &schedParam);
    schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_attr_setschedparam(&threadAttr, &schedParam);
    pthread_attr_setstacksize(&threadAttr, 8*1024);
    pthread_create(&descr->interruptThread,
                        &threadAttr,
           (void* (*)(void*))hostRxInterruptHandler,
                        (void *)descr);
}

/************************* IMPLEMENTATION **********************************************/

/************************************************************************************//**
    \brief Initialize FIFO driver.
    \param[in] descr - hardware FIFO descriptor.
****************************************************************************************/
void HOST_HwMailboxInit(HOST_HwMailboxDescriptor_t *const descr)
{
    SYS_FifoFillDescriptor(&descr->txFifo, descr->txFifoPage, sizeof(descr->txFifoPage));
    SYS_FifoFillDescriptor(&descr->rxFifo, descr->rxFifoPage, sizeof(descr->rxFifoPage));

    *hostHwMailboxDesc() = descr;

    descr->zigbeeDeviceFd = open(ZIGBEE_DEVICE_TREE, O_RDWR);
    SYS_DbgAssert(descr->zigbeeDeviceFd >= 0, HOST_HW_MAILBOX_INIT_1);
    pthread_mutex_init(&descr->rxFifoMutex, NULL);
    hostCreateRxThread(descr);
    descr->rtsCallback(descr);
}

/************************************************************************************//**
    \brief Close given FIFO descriptor.
    \param[in] descr - hardware FIFO descriptor.
****************************************************************************************/
void HOST_HwMailboxClose(const HOST_HwMailboxDescriptor_t *const descr)
{
    close(descr->zigbeeDeviceFd);
}

/************************************************************************************//**
    \brief Puts a piece of data to the TX FIFO.
    \note Generates assert if conveyed chunk of data doesn't fit to the free part
          of TX FIFO.
          Generates assert if TX FIFO is not held for transmission (unexpected call).
    \param[in] descr - hardware FIFO descriptor.
    \parem[in] data - pointer to the chunk of data.
    \parem[in] dataSize - data chunk length.
****************************************************************************************/
void HOST_HwMailboxTx(HOST_HwMailboxDescriptor_t *const descr, const uint8_t *data, uint8_t dataSize)
{
    SYS_DbgAssert((SYS_FifoDataSize(&descr->txFifo) + dataSize) <= sizeof(descr->txFifoPage), HOST_HW_MAILBOX_TX_1);
    SYS_DbgAssert(dataSize == SYS_FifoWrite(&descr->txFifo, data, dataSize), HOST_HW_MAILBOX_TX_2);
}

/************************************************************************************//**
    \brief Returns a size of the remaining free place in TX FIFO.
    \param[in] descr - hardware FIFO descriptor.
    \return Size of the remaining free place in TX FIFO.
****************************************************************************************/
uint8_t HOST_HwMailboxTxFifoAvailableSize(HOST_HwMailboxDescriptor_t *const descr)
{
    return SYS_FifoSpaceAvailable(&descr->txFifo);
}

/************************************************************************************//**
    \brief Finalizes the transmission. Sends the "full" interrupt to the destination
           subsystem.
    \param[in] descr - hardware FIFO descriptor.
    \note  assume the tx fifo have included the fifo header
****************************************************************************************/
void HOST_HwMailboxTxEnd(HOST_HwMailboxDescriptor_t *const descr, HAL_HostId_t destSubsystem)
{
    uint8_t txBuf[HAL_MAILBOX_TXRX_FIFO_CAPACITY] = {0};
    SYS_DbgAssert(SYS_FifoDataSize(&descr->txFifo) <= sizeof(txBuf), HOST_HW_MAILBOX_TX_END_1);
    uint8_t actualLength = SYS_FifoRead(&descr->txFifo, txBuf, sizeof(txBuf));
    // To avoid crash in kernel, the mailbox header should be checked.
    SOC_HwFifoPackedHeader_t *fifoHeader = (SOC_HwFifoPackedHeader_t*)txBuf;
    SYS_DbgAssert((fifoHeader->messageLength + 1)* 4 == actualLength, HOST_HW_MAILBOX_TX_END_2);
    Zigbee_Ioctl_WriteToMbox(descr->zigbeeDeviceFd, txBuf);
    descr->rtsCallback(descr);
}

/************************************************************************************//**
    \brief Reads a piece of data from RX FIFO.
    \note Generates assert if RX FIFO is empty (unexpected call).
          Generates assert if size of data in RX FIFO is less than requested.
          Because the length could be not the 4-bytes alignment, which means we can't directly
          read the data from HW fifo due to the HW fifo restriction.
    \param[in] descr - hardware FIFO descriptor.
    \parem[in, out] buffer - a pointer to the given buffer.
    \parem[in] length - number of bytes to be read.
****************************************************************************************/
void HOST_HwMailboxRx(HOST_HwMailboxDescriptor_t *const descr, uint8_t *buffer, uint8_t length)
{
    SYS_FifoRead(&descr->rxFifo, buffer, length);
}

/************************************************************************************//**
    \brief Finalizes the reception procedure. Indicates that the message has been
           successfully read by higher layer.
    \note Generates assert if message was not read completely.
    \param[in] descr - hardware FIFO descriptor.
****************************************************************************************/
void HOST_HwMailboxRxEnd(HOST_HwMailboxDescriptor_t *const descr)
{
    SYS_FifoReset(&descr->rxFifo);
    pthread_mutex_unlock(&descr->rxFifoMutex);
}
#endif // _UNIT_TEST_

/* eof bbHostMailbox.c */
