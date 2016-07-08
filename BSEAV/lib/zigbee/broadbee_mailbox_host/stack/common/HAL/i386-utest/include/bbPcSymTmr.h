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
* FILENAME: $Workfile: trunk/stack/common/HAL/i386-utest/include/bbPcSymTmr.h $
*
* DESCRIPTION:
*   i386 Symbol Timer Simulator interface.
*
* $Revision: 3368 $
* $Date: 2014-08-21 16:02:35Z $
*
*****************************************************************************************/


#ifndef _BB_PC_SYM_TMR_H
#define _BB_PC_SYM_TMR_H


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Simulates for the i386 initialization of the Symbol Timer Hardware.
 */
#define PC_SymbolInit()                                 while(0)


/**//**
 * \brief   Simulates for the i386 configuration of the Symbol Timer Hardware.
 */
#define PC_SymbolConfig(symbolRateId)                   do { ((void)(symbolRateId)); } while(0)


/**//**
 * \brief   Simulates for the i386 returning of the current timestamp.
 */
#define PC_SymbolTimestamp()                            (0)


/**//**
 * \brief   Simulates for the i386 appointing of the specified channel.
 */
#define PC_SymbolAppoint(timeshift, channelId)          do { ((void)(timeshift)); ((void)(channelId)); } while(0)


/**//**
 * \brief   Simulates for the i386 reappointing of the specified channel.
 */
#define PC_SymbolReappoint(newTimeshift, channelId)     do { ((void)(newTimeshift)); ((void)(channelId)); } while(0)


/**//**
 * \brief   Simulates for the i386 recalling of appointment from the specified channel.
 */
#define PC_SymbolRecall(channelId)                      do { ((void)(channelId)); } while(0)


#endif /* _BB_PC_SYM_TMR_H */