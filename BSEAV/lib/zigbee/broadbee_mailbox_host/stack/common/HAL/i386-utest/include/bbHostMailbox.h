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
* FILENAME: $Workfile: trunk/stack/common/HAL/i386-utest/include/bbHostMailbox.h $
*
* DESCRIPTION:
*   Definitions for SoC Mailbox hardware abstraction layer.
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
*****************************************************************************************/

#ifndef _BB_HOST_MAILBOX_H
#define _BB_HOST_MAILBOX_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbHalMailbox.h"
#include "bbSysFifo.h"

/************************* TYPES *******************************************************/
#define ZIGBEE_DEVICE_TREE  "/dev/zigbee"
/**//**
 * \brief packed HW mailbox header
 */
typedef struct _SOC_HwFifoPackedHeader_t
{
    union
    {
        struct
        {
            uint32_t messageLength      : 5;
            uint32_t protocolVersion    : 3;
            uint32_t sequenceNumber     : 8;
            uint32_t messageType        : 2;
            uint32_t messageId          : 10;
            uint32_t fragment           : 1;
            uint32_t subSystemId        : 3;
        };
        uint8_t part[sizeof(uint32_t)];
    };
} SOC_HwFifoPackedHeader_t;

/**//**
 * \brief Type declaration of hardware FIFO descriptor.
 */
typedef struct _HOST_HwMailboxDescriptor_t HOST_HwMailboxDescriptor_t;

/**//**
 * \brief Type declaration of a offline callback primitive.
 * \note Should be called when the Host system becomes not available.
 * \param[in] descr - the pointer to the hardware FIFO descriptor structure.
 */
typedef void (*HOST_HwMailboxOfflineCallback_t)(HOST_HwMailboxDescriptor_t *descr);

/**//**
 * \brief Type declaration of ready-to-send callback primitive.
 * \note Should be called when the TX FIFO is ready to send new message.
 *       It is not necessary to have TX FIFO completely free, since it is
 *       possible to write new message to the TX FIFO when the previous one is
 *       still there.
 * \param[in] descr - the pointer to the hardware FIFO descriptor structure.
 */
typedef void (*HOST_HwMailboxReadyToSendCallback_t)(HOST_HwMailboxDescriptor_t *descr);

/**//**
 * \brief Type declaration of data-received callback primitive.
 * \note Should be called from the H2Z "full" interrupt routine.
 * \param[in] descr - the pointer to the hardware FIFO descriptor structure.
 */
typedef void (*HOST_HwMailboxDataReceivedCallback_t)(HOST_HwMailboxDescriptor_t *descr);


/**//**
 * \brief Definition of the hardware fifo descriptor.
 */
typedef struct _HOST_HwMailboxDescriptor_t
{
    /* Because our HW neither byte-accessible nor word-accessible, we need two more
     * software FIFOs to save the data temporarily.
     */
    int                                    zigbeeDeviceFd;
    pthread_t                              interruptThread;
    pthread_mutex_t                        rxFifoMutex;
    SYS_FifoDescriptor_t                   txFifo;
    SYS_FifoDescriptor_t                   rxFifo;
    uint8_t                                txFifoPage[HAL_MAILBOX_TXRX_FIFO_CAPACITY];
    uint8_t                                rxFifoPage[HAL_MAILBOX_TXRX_FIFO_CAPACITY];
    /* Offline callback. */
    HOST_HwMailboxOfflineCallback_t         offlineCallback;
    /* Ready-to-send callback. */
    HOST_HwMailboxReadyToSendCallback_t     rtsCallback;
    /* Data received callback. */
    HOST_HwMailboxDataReceivedCallback_t    rxCallback;
} HOST_HwMailboxDescriptor_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief Initialize FIFO driver.
    \param[in] descr - hardware FIFO descriptor.
****************************************************************************************/
void HOST_HwMailboxInit(HOST_HwMailboxDescriptor_t *const descr);

/************************************************************************************//**
    \brief Close given FIFO descriptor.
    \param[in] descr - hardware FIFO descriptor.
****************************************************************************************/
void HOST_HwMailboxClose(const HOST_HwMailboxDescriptor_t *const descr);

/************************************************************************************//**
    \brief Puts a piece of data to the TX FIFO.
    \note Generates assert if conveyed chunk of data doesn't fit to the free part
          of TX FIFO.
          Generates assert if TX FIFO is not held for transmission (unexpected call).
    \param[in] descr - hardware FIFO descriptor.
    \parem[in] data - pointer to the chunk of data.
    \parem[in] dataSize - data chunk length.
****************************************************************************************/
void HOST_HwMailboxTx(HOST_HwMailboxDescriptor_t *const descr, const uint8_t *data, uint8_t dataSize);

/************************************************************************************//**
    \brief Returns a size of the remaining free place in TX FIFO.
    \param[in] descr - hardware FIFO descriptor.
    \return Size of the remaining free place in TX FIFO.
****************************************************************************************/
uint8_t HOST_HwMailboxTxFifoAvailableSize(HOST_HwMailboxDescriptor_t *const descr);

/************************************************************************************//**
    \brief Finalizes the transmission. Sends the "full" interrupt to the destination
           subsystem.
    \param[in] descr - hardware FIFO descriptor.
****************************************************************************************/
void HOST_HwMailboxTxEnd(HOST_HwMailboxDescriptor_t *const descr, HAL_HostId_t destSubsystem);

/************************************************************************************//**
    \brief Reads a piece of data from RX FIFO.
    \note Generates assert if RX FIFO is empty (unexpected call).
          Generates assert if size of data in RX FIFO is less than requested.
    \param[in] descr - hardware FIFO descriptor.
    \parem[in, out] buffer - a pointer to the given buffer.
    \parem[in] length - number of bytes to be read.
****************************************************************************************/
void HOST_HwMailboxRx(HOST_HwMailboxDescriptor_t *const descr, uint8_t *buffer, uint8_t length);

/************************************************************************************//**
    \brief Finalizes the reception procedure. Indicates that the message has been
           successfully read by higher layer.
    \note Generates assert if message was not read completely.
    \param[in] descr - hardware FIFO descriptor.
****************************************************************************************/
void HOST_HwMailboxRxEnd(HOST_HwMailboxDescriptor_t *const descr);

#endif //_BB_HOST_MAILBOX_H

// eof bbHostMailbox.h