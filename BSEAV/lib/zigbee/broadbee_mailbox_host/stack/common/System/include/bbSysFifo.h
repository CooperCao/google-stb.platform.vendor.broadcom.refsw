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
* FILENAME: $Workfile: trunk/stack/common/System/include/bbSysFifo.h $
*
* DESCRIPTION:
*   fifo declaration.
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/

#ifndef _SYS_FIFO_H
#define _SYS_FIFO_H

/************************* INCLUDES ****************************************************/
#include "bbSysAtomic.h"
#include "bbSysTypes.h"
#include "bbSysUtils.h"
#include "bbSysDbg.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Type of fifo descriptor.
 */
typedef struct
{
    uint8_t *buffer;
    uint8_t *volatile in;
    uint8_t *volatile out;
    uint16_t bufferLength;
    volatile Bool8_t isFull;
    volatile Bool8_t isEmpty;
} SYS_FifoDescriptor_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief Resets FIFO buffer.
  \param[in] fifo - FIFO descriptor pointer.
****************************************************************************************/
INLINE void SYS_FifoReset(SYS_FifoDescriptor_t *const fifo)
{
    SYS_DbgAssert(fifo, SYSFIFO_SYSFIFORESET_0);

    fifo->in = fifo->buffer;
    fifo->out = fifo->buffer;
    fifo->isFull = false;
    fifo->isEmpty = true;
}

/************************************************************************************//**
  \brief Fills fifo descriptor by given parameters.
  \param[in] fifo - fifo descriptor pointer.
  \param[in] buffer - memory area given for FIFO buffer.
  \param[in] bufferLength - size of allocated buffer.
****************************************************************************************/
INLINE void SYS_FifoFillDescriptor(SYS_FifoDescriptor_t *const fifo, uint8_t *const buffer, const uint16_t bufferLength)
{
    SYS_DbgAssert(fifo, SYSFIFO_SYSFIFOFILLDESCRIPTOR_0);
    SYS_DbgAssert(buffer, SYSFIFO_SYSFIFOFILLDESCRIPTOR_1);
    SYS_DbgAssert(bufferLength, SYSFIFO_SYSFIFOFILLDESCRIPTOR_2);

    fifo->buffer = buffer;
    fifo->bufferLength = bufferLength;
    SYS_FifoReset(fifo);
}

/************************************************************************************//**
  \brief Gets next pointer to the FIFO buffer by given one.
  \param[in] fifo - FIFO descriptor pointer.
  \param[in] pointer - pointer to the current position.

  \return next pointer to the FIFO
****************************************************************************************/
INLINE uint8_t *SYS_FifoNextPoint(const SYS_FifoDescriptor_t *const fifo, const uint8_t *const pointer)
{
    SYS_DbgAssert(fifo, SYSFIFO_SYSFIFONEXTPOINT_0);
    SYS_DbgAssert(pointer >= fifo->buffer, SYSFIFO_SYSFIFONEXTPOINT_1);

    return (pointer >= &fifo->buffer[fifo->bufferLength - 1]) ?
           fifo->buffer :
           (uint8_t *)(pointer + 1);
}

/************************************************************************************//**
  \brief Gets size of data stored into FIFO
  \param[in] fifo - FIFO descriptor pointer.

  \return data size
****************************************************************************************/
INLINE uint16_t SYS_FifoDataSize(const SYS_FifoDescriptor_t *const fifo)
{
    SYS_DbgAssert(fifo, SYSFIFO_SYSFIFODATASIZE_0);

    int32_t size = 0;
    size = (fifo->isFull) ? fifo->bufferLength : fifo->in - fifo->out;
    if (0 > size)
        size += fifo->bufferLength;
    return (uint16_t)size;
}

/************************************************************************************//**
  \brief Returns the size of remaining free space.
  \param[in] fifo - FIFO descriptor pointer.
  \return The size available for writing.
****************************************************************************************/
INLINE uint16_t SYS_FifoSpaceAvailable(const SYS_FifoDescriptor_t *const fifo)
{
    SYS_DbgAssert(fifo, SYSFIFO_SYSFIFOSPACEAVAILABLE_0);
    return fifo->bufferLength - SYS_FifoDataSize(fifo);
}

/************************************************************************************//**
  \brief Checks FIFO state.
  \param[in] fifo - FIFO descriptor pointer.

  \return true if FIFO buffer is full and false otherwise.
****************************************************************************************/
INLINE bool SYS_FifoIsFull(const SYS_FifoDescriptor_t *const fifo)
{
    SYS_DbgAssert(fifo, SYSFIFO_SYSFIFOISFULL_0);
    return fifo->isFull;
}

/************************************************************************************//**
  \brief Checks FIFO state.
  \param[in] fifo - FIFO descriptor pointer.

  \return true if FIFO buffer is empty and false otherwise.
****************************************************************************************/
INLINE bool SYS_FifoIsEmpty(const SYS_FifoDescriptor_t *const fifo)
{
    SYS_DbgAssert(fifo, SYSFIFO_SYSFIFOISEMPTY_0);
    return fifo->isEmpty;
}

/************************************************************************************//**
  \brief Writes one byte to the FIFO
  \param[in] fifo - FIFO descriptor pointer.
  \param[in] value - byte value to be stored

  \return True if byte stored successful and false otherwise
****************************************************************************************/
INLINE bool SYS_FifoWriteByte(SYS_FifoDescriptor_t *const fifo, const uint8_t value)
{
    SYS_DbgAssert(fifo, SYSFIFO_SYSFIFOWRITEBYTE_0);

    if (fifo->isFull)
        return false;

    *fifo->in = value;
    fifo->in = SYS_FifoNextPoint(fifo, fifo->in);

    if (fifo->out == fifo->in)
        fifo->isFull = true;
    fifo->isEmpty = false;

    return true;
}

/************************************************************************************//**
  \brief Reads one byte from FIFO
  \param[in] fifo - FIFO descriptor pointer.

  \return stored value
****************************************************************************************/
INLINE uint8_t SYS_FifoReadByte(SYS_FifoDescriptor_t *const fifo)
{
    SYS_DbgAssert(!fifo->isEmpty, SYSFIFO_SYSFIFOREADBYTE_0);

    uint8_t value = *fifo->out;
    fifo->out = SYS_FifoNextPoint(fifo, fifo->out);

    if (fifo->out == fifo->in)
        fifo->isEmpty = true;
    fifo->isFull = false;
    return value;
}

/************************************************************************************//**
  \brief Writes more as possible bytes from given area
  \param[in] fifo - FIFO descriptor pointer.
  \param[in] data - pointer to the data area.
  \param[in] dataLength - data area size.

  \return stored bytes amount.
****************************************************************************************/
INLINE uint8_t SYS_FifoWrite(SYS_FifoDescriptor_t *const fifo, const  uint8_t *const data, const uint8_t dataLength)
{
    SYS_DbgAssert(fifo, SYSFIFO_SYSFIFOWRITE_0);
    SYS_DbgAssert(data || dataLength != 0, SYSFIFO_SYSFIFOWRITE_1);

    uint16_t dataToWrite = fifo->bufferLength - SYS_FifoDataSize(fifo);
    if (dataLength < dataToWrite)
        dataToWrite = dataLength;
    else
        fifo->isFull = true;
    fifo->isEmpty = false;

    if (dataToWrite)
    {
        if (fifo->in < fifo->out)
        {
            memcpy(fifo->in, data, dataToWrite);
            fifo->in += dataToWrite;
        }
        else
        {
            uint8_t firstPartLength = MIN(dataToWrite, 1 + (&fifo->buffer[fifo->bufferLength - 1] - fifo->in));
            uint8_t secondPartLength = dataToWrite - firstPartLength;

            memcpy(fifo->in, data, firstPartLength);
            if (secondPartLength)
            {
                memcpy(fifo->buffer, &data[firstPartLength], secondPartLength);
                fifo->in = fifo->buffer + secondPartLength;
            }
            else
                fifo->in = SYS_FifoNextPoint(fifo, fifo->in + firstPartLength - 1);
        }
    }
    return dataToWrite;
}

/************************************************************************************//**
  \brief Reads more as possible bytes from FIFO
  \param[in] fifo - FIFO descriptor pointer.
  \param[in] buffer - area to be store for readed data
  \param[in] bufferLength - store area size

  \return readed bytes amount.
****************************************************************************************/
INLINE uint8_t SYS_FifoRead(SYS_FifoDescriptor_t *const fifo, uint8_t *const buffer, const uint8_t bufferLength)
{
    SYS_DbgAssert(buffer || bufferLength != 0, SYSFIFO_SYSFIFOREAD_0);
    uint16_t dataToRead = SYS_FifoDataSize(fifo);
    if (bufferLength < dataToRead)
        dataToRead = bufferLength;
    else
        fifo->isEmpty = true;
    fifo->isFull = false;

    if (dataToRead)
    {
        if (fifo->out < fifo->in)
        {
            memcpy(buffer, fifo->out, dataToRead);
            fifo->out += dataToRead;
        }
        else
        {
            uint8_t firstPartLength = MIN(dataToRead, 1 + (&fifo->buffer[fifo->bufferLength - 1] - fifo->out));
            uint8_t secondPartLength = dataToRead - firstPartLength;

            memcpy(buffer, fifo->out, firstPartLength);
            if (secondPartLength)
            {
                memcpy(&buffer[firstPartLength], fifo->buffer, secondPartLength);
                fifo->out = fifo->buffer + secondPartLength;
            }
            else
                fifo->out = SYS_FifoNextPoint(fifo, fifo->out + firstPartLength - 1);

        }
    }
    return dataToRead;
}
#endif /* _SYS_FIFO_H */
/* eof bbSysFifo.h */