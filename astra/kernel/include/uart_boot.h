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

#ifndef _UART_BOOT_H_
#define _UART_BOOT_H_
/*
 * NS16550A UART
 */
#if 0
#if (TARGET_SOC == 7251)
#define UART0_BASE 0xf0406b00
#endif

#if (TARGET_SOC == 7250)
#define UART0_BASE 0xf040b400
#endif

#if (TARGET_SOC == 7268)
#define UART0_BASE 0xf040c000
#endif

#if (TARGET_SOC == 7445) && (TARGET_SOC_REV == 0xC0)
#define UART0_BASE 0xf0406b00
#endif
#if (TARGET_SOC == 7445) && (TARGET_SOC_REV == 0xD0)
#define UART0_BASE 0xf040ab00
#endif

#if (TARGET_SOC == 7145) && (TARGET_SOC_REV == 0xA0)
#define UART0_BASE 0xf0406c00
#endif

#if (TARGET_SOC == 7145) && (TARGET_SOC_REV == 0xB0)
#define UART0_BASE 0xf040a900
#endif

#ifndef UART0_BASE
#error Please verify UART base address
#define UART0_BASE  0x00000000
#endif
#endif
#define UART0_BASE 0xf040c000
/* Rx status register */
#define RXRDA          0x01
#define RXOVFERR       0x02
#define RXPARERR       0x04
#define RXFRAMERR      0x08

#define THRE 				0x20
#define UART_SDW_RBR     	0x00
#define UART_SDW_THR     	0x00
#define UART_SDW_DLL     	0x00
#define UART_SDW_DLH     	0x04
#define UART_SDW_IER     	0x04
#define UART_SDW_IIR     	0x08
#define UART_SDW_FCR     	0x08
#define UART_SDW_LCR     	0x0c
#define UART_SDW_MCR     	0x10
#define UART_SDW_LSR     	0x14
#define UART_SDW_MSR     	0x18
#define UART_SDW_SCR     	0x1c

#endif /* _UART_BOOT_H_ */
