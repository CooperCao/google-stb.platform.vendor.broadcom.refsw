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
* FILENAME: $Workfile: trunk/stack/common/HAL/include/private/bbHalPrivateUsart.h $
*
* DESCRIPTION:
*   Private declaration of USART internal structures.
*
* $Revision: 3250 $
* $Date: 2014-08-13 23:21:53Z $
*
****************************************************************************************/

#ifndef _HAL_PRIVATEUSART_H
#define _HAL_PRIVATEUSART_H

/************************* DEFINITIONS *************************************************/
#if defined(__SoC__)
#   include "bbSocUart.h"
#   define PLATFORM_UsartEnable             SoC_CpuUartEnable
#   define PLATFORM_UsartDisable            SoC_CpuUartDisable
#   define PLATFORM_UsartSendCapasity       SoC_CpuUartSendCapacity
#   define PLATFORM_UsartSendByte           SoC_CpuUartSendByte
#   define PLATFORM_UsartReceiveByte        SoC_CpuUartReceiveByte
#   define PLATFORM_UsartGetRxDataSize      SoC_CpuUartGetRxDataSize
#elif defined(__ML507__)
#   include "bbMl507Usart.h"
#   define PLATFORM_UsartEnable             ML507_UsartEnable
#   define PLATFORM_UsartDisable            ML507_UsartDisable
#   define PLATFORM_UsartSendCapasity       ML507_UsartSendCapacity
#   define PLATFORM_UsartSendByte           ML507_UsartSendByte
#   define PLATFORM_UsartReceiveByte        ML507_UsartReceiveByte
#   define PLATFORM_UsartGetRxDataSize      ML507_UsartGetRxDataSize
#else /* __i386__ */
#   include "bbPcUsart.h"
#   define PLATFORM_UsartEnable             PC_UsartEnable
#   define PLATFORM_UsartDisable            PC_UsartDisable
#   define PLATFORM_UsartSendCapasity       PC_UsartSendCapacity
#   define PLATFORM_UsartSendByte           PC_UsartSendByte
#   define PLATFORM_UsartReceiveByte        PC_UsartReceiveByte
#   define PLATFORM_UsartGetRxDataSize      PC_UsartGetRxDataSize
#endif

/************************* TYPES *******************************************************/

#endif /*_HAL_PRIVATEUSART_H */
/* eof bbHalPrivateUsart.h */