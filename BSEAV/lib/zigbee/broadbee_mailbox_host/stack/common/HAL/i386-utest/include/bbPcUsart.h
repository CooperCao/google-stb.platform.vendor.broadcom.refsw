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
* FILENAME: $Workfile: trunk/stack/common/HAL/i386-utest/include/bbPcUsart.h $
*
* DESCRIPTION:
*   pc usart declaration.
*
* $Revision: 2088 $
* $Date: 2014-04-07 14:32:51Z $
*
****************************************************************************************/
#ifndef _PC_USART_H
#define _PC_USART_H

/************************* INCLUDES ****************************************************/
#include <Windows.h>
#include "bbSysUtils.h"
#include "bbHalUsart.h"

/************************* DEFINITIONS *************************************************/
#define MAX_PC_USART_CHANNEL_AMOUNT 255U

/************************* TYPES *******************************************************/
/**//**
 * \brief usart interrupt handler type.
 */
typedef void(*UsartVector_t)(void *const link);

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief initializes usart
    \param[in] channel - USART channel
    \param[in] txComplete - pointer to the tx interrupt handler
    \param[in] rxComplete - pointer to the rx interrupt handler
    \param[in] link - pointer to the upper descriptor.
****************************************************************************************/
void PC_UsartEnable(UsartChannel_t channel, UsartVector_t txComplete, UsartVector_t rxComplete, void *const link);

/************************************************************************************//**
    \brief disables usart
    \param[in] channel - USART channel
****************************************************************************************/
void PC_UsartDisable(UsartChannel_t channel);


/************************************************************************************//**
    \brief Checks capacity of tx buffer.
    \param[in] channel - USART channel
    \return true if tx buffer can receive one more byte.
****************************************************************************************/
INLINE bool PC_UsartSendCapacity(UsartChannel_t channel)
{
    (void)channel;
    return true;
}

/************************************************************************************//**
    \brief Sends one byte
    \param[in] channel - USART channel
    \param[in] value - byte value
****************************************************************************************/
void PC_UsartSendByte(UsartChannel_t channel, uint8_t value);

/************************************************************************************//**
    \brief Read one byte
    \param[in] channel - USART channel
    \return read value
****************************************************************************************/
uint8_t PC_UsartReceiveByte(UsartChannel_t channel);

/************************************************************************************//**
    \brief Gets number of received bytes
    \param[in] channel - USART channel
    \return bytes number
****************************************************************************************/
uint8_t PC_UsartGetRxDataSize(UsartChannel_t channel);

#endif /* _PC_USART_H */
/* eof bbPcUsart.h */