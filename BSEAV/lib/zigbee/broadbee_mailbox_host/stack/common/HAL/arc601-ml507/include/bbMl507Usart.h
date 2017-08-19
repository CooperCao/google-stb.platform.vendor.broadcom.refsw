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
 *      ml507 usart declaration.
 *
*******************************************************************************/

#ifndef _ML507_USART_H
#define _ML507_USART_H

/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"
#include "bbHalUsart.h"

/************************* DEFINITIONS *************************************************/
/* ========================= UART Registers ===================================
0x00c0_001c : reg_H : UART Status
    reg_H[26]   : TX FIFO Interrupt Enable
    reg_H[25]   : TX FIFO Interrupt Status
    reg_H[24]   : TX FIFO overflow indicator; Write 1 to clear this flag.
    reg_H[18]   : RX FIFO Interrupt Enable
    reg_H[17]   : RX FIFO Interrupt Status
    reg_H[16]   : RX FIFO overflow indicator; Write 1 to clear this flag.
    reg_H[12:8] : TX FIFO data count
    reg_H[4:0]  : RX FIFO data count

0x00c0_0020 : reg_I : UART Control
    reg_I[16]   : TX Output Enable
    reg_I[12:8] : TX FIFO interrupt threshold
    reg_I[4:0]  : RX FIFO interrupt threshold

0x00c0_0024 : reg_J : UART Data FIFO access port
    reg_J[7:0]  : Read/Write port to UART data FIFO

0x00c0_0028 : reg_K : UART Baud Rate Control 1
    reg_K[11:0] : Master Divider : 54Mhz/115200 = 1875/4;
                  1875 - 1 = 1874 (default); Set A = 1875; B = 4.
    reg_K[27:16]: TX Slave Divider : (A / B * 4) - 1 = 1874 (default)

0x00c0_002c : reg_L : UART Baud Rate Control 2
    reg_L[8:0]  : RX Slave Divider : (A / B / 2) - 1 = 233 (default)

Notes: for master clock=27MHz:
    27MHz/115200 = 1875/8; A=1875; B=8.
    mst_div = 1875-1 =1874
    tx_div = (A/B*4)-1=937
    rx_div = (A/B/2)-1=116
=============================================================================*/

#define REG_UART_RX_FIFO_COUNT              (*((volatile unsigned char *)0x00C0001C))
#define REG_UART_TX_FIFO_COUNT              (*((volatile unsigned char *)0x00C0001D))
#define REG_UART_RX_INT_CONTROL             (*((volatile unsigned char *)0x00C0001E))
#define REG_UART_TX_INT_CONTROL             (*((volatile unsigned char *)0x00C0001F))

#define REG_UART_RX_FIFO_INT_THRESHOLD      (*((volatile unsigned char *)0x00C00020))
#define REG_UART_TX_FIFO_INT_THRESHOLD      (*((volatile unsigned char *)0x00C00021))
#define REG_UART_TX_SWITCH                  (*((volatile unsigned char *)0x00C00022))

#define REG_UART_FIFO_PORT                  (*((volatile unsigned char *)0x00C00024))
#define REG_UART_BAUD_RATE_MST_DIV          (*((volatile unsigned short *)0x00C00028))
#define REG_UART_TX_SLAVE_DIV               (*((volatile unsigned short *)0x00C0002A))
#define REG_UART_RX_SLAVE_DIV               (*((volatile unsigned short *)0x00C0002C))


#define UART_CLEAR_OVRFLW       0x01
#define UART_CLEAR_INT          0x02
#define UART_ENABLE_INT         0X04

#define MAX_ML507_USART_CHANNEL_AMOUNT  1U
#define ML507_USART_CHANNEL_NUMBER      0U

#define ML507_USART_SHADOW_BUFFER_SIZE  16U

/************************* TYPES *******************************************************/
/**//**
 * \brief usart interrupt handler type.
 */
typedef void(*UsartVector_t)(void *const link);

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief initializes usart on ml570
    \param[in] channel - USART channel
    \param[in] txComplete - pointer to the tx interrupt handler
    \param[in] rxComplete - pointer to the rx interrupt handler
    \param[in] link - pointer to the upper descriptor.
****************************************************************************************/
void ML507_UsartEnable(UsartChannel_t channel, UsartVector_t txComplete, UsartVector_t rxComplete, void *const link);

/************************************************************************************//**
    \brief disables usart on ml570
    \param[in] channel - USART channel
****************************************************************************************/
void ML507_UsartDisable(UsartChannel_t channel);

/************************************************************************************//**
    \brief Checks capacity of tx buffer.
    \param[in] channel - USART channel
    \return true if tx buffer can receive one more byte.
****************************************************************************************/
INLINE bool ML507_UsartSendCapacity(UsartChannel_t channel)
{
    return (REG_UART_TX_FIFO_COUNT != ML507_USART_SHADOW_BUFFER_SIZE);
}

/************************************************************************************//**
    \brief Sends one byte
    \param[in] channel - USART channel
    \param[in] value - byte value
****************************************************************************************/
INLINE void ML507_UsartSendByte(UsartChannel_t channel, uint8_t value)
{
    SYS_DbgAssertComplex(ML507_UsartSendCapacity(channel), ML507USART_SENDBYTE_0);
    REG_UART_FIFO_PORT = value;
}

/************************************************************************************//**
    \brief Read one byte
    \param[in] channel - USART channel
    \return read value
****************************************************************************************/
INLINE uint8_t ML507_UsartReceiveByte(UsartChannel_t channel)
{
    (void)channel;
    return REG_UART_FIFO_PORT;
}

/************************************************************************************//**
    \brief Gets number of received bytes
    \param[in] channel - USART channel
    \return bytes number
****************************************************************************************/
INLINE uint8_t ML507_UsartGetRxDataSize(UsartChannel_t channel)
{
    (void)channel;
    return REG_UART_RX_FIFO_COUNT;
}

/************************************************************************************//**
    \brief Sets tx interrupt control
    \param[in] channel - USART channel
****************************************************************************************/
INLINE void ML507_UsartSetTxIntControl(UsartChannel_t channel)
{
    (void)channel;
    REG_UART_TX_INT_CONTROL = UART_ENABLE_INT | UART_CLEAR_INT | UART_CLEAR_OVRFLW;
}

/************************************************************************************//**
    \brief Sets rx interrupt control
    \param[in] channel - USART channel
****************************************************************************************/
INLINE void ML507_UsartSetRxIntControl(UsartChannel_t channel)
{
    (void)channel;
    REG_UART_RX_INT_CONTROL = UART_ENABLE_INT | UART_CLEAR_INT | UART_CLEAR_OVRFLW;
}

#endif /* _ML507_USART_H */

/* eof bbMl507Usart.h */
