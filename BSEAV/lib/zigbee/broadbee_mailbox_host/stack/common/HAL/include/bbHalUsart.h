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
 *      Declaration of common usart descriptor.
 *
*******************************************************************************/

#ifndef _HAL_USART_H
#define _HAL_USART_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysFifo.h"
#include "bbSysTaskScheduler.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief usart channels definitions.
 */
#define HAL_USART_CHANNEL_0         0

/************************* TYPES *******************************************************/
/**//**
 * \brief predefined usart descriptor type.
 */
typedef struct _HAL_UsartDescriptor_t HAL_UsartDescriptor_t;

/**//**
 * \brief usart callback type.
 */
typedef void(*HAL_UsartCallback_t)(HAL_UsartDescriptor_t *const usart);

/**//**
 * \brief usart channel type.
 */
typedef uint8_t UsartChannel_t;

/**//**
 * \brief usart state enumeration.
 */
typedef enum
{
    CLOSED_STATE = 0U,
    OPENED_STATE,
    IN_PROGRESS,
} HAL_UsartState_t;

/**//**
 * \brief usart descriptor type.
 */
typedef struct _HAL_UsartDescriptor_t
{
    struct
    {
        /* usart state */
        HAL_UsartState_t state;
        /* usart task descriptor */
        SYS_SchedulerTaskDescriptor_t usartTask;
    };
    /* usart channel  */
    UsartChannel_t       channel;
    /* usart tx fifo descriptor */
    SYS_FifoDescriptor_t txFifo;
    /* usart rx fifo descriptor */
    SYS_FifoDescriptor_t rxFifo;
    /* usart tx callback */
    HAL_UsartCallback_t txCallback;
    /* usart rx callback */
    HAL_UsartCallback_t rxCallback;
} HAL_UsartDescriptor_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief Initializes usart
    \param[in] usart - usart descriptor.
****************************************************************************************/
void HAL_UsartOpen(HAL_UsartDescriptor_t *const usart);

/************************************************************************************//**
    \brief Closes usart
    \param[in] usart - usart descriptor.
****************************************************************************************/
void HAL_UsartClose(HAL_UsartDescriptor_t *const usart);

/************************************************************************************//**
    \brief Sends message
    \param[in] usart - usart descriptor.
    \param[in] data - data pointer.
    \param[in] dataLength - data length.

    \return number of sent bytes.
****************************************************************************************/
uint8_t HAL_UsartWrite(HAL_UsartDescriptor_t *const usart, const uint8_t *const data, const uint8_t dataLength);

/************************************************************************************//**
    \brief Read message
    \param[in] usart - usart descriptor.
    \param[in] data - data pointer.
    \param[in] dataLength - data length.

    \return number read bytes
****************************************************************************************/
uint8_t HAL_UsartRead(HAL_UsartDescriptor_t *const usart, uint8_t *const buffer, const uint8_t bufferLength);

#endif /* _HAL_USART_H */

/* eof bbHalUsart.h */