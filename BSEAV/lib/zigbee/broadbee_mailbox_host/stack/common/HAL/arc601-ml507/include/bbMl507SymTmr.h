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
*
* FILENAME: $Workfile: trunk/stack/common/HAL/arc601-ml507/include/bbMl507SymTmr.h $
*
* DESCRIPTION:
*   ML507 Symbol Timer Driver interface addons.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
*****************************************************************************************/

#ifndef _BB_ML507_SYM_TMR_H
#define _BB_ML507_SYM_TMR_H

/************************* INCLUDES ***********************************************************************************/
#include "bbSysTypes.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   The LOG2-factor of the Symbol Clock frequency to the Symbol Rate.
 * \details The Symbol Clock frequency is produced by the Symbol Timer Prescaler (from the Symbol Timer unit main clock)
 *  and drives the Symbol Counter, that in turn counts symbol fractions.
 * \details The Symbol Clock frequency is calculated as <em>f_clk = f_sym * 2 ^ k</em>, where:
 *  - \e k is the value defined with this constant (the LOG2-factor),
 *  - \e f_sym is the desired Symbol Rate, in symbol/s,
 *  - \e f_clk is the Symbol Clock (Symbol Counter) frequency, in symbol_fraction/s.
 *
 * \details The <em>2 ^ k</em> is the reciprocal of the selected fraction of Symbol Clock.
 * \details The maximum allowed value of this factor in order not to exceed the 32-bit unsigned integer rank
 *  (0xFFFFFFFF) and support signed durations up to 0x03FFFFFF (the maximum lifetime of indirect transaction) is 5 -
 *  i.e., 32 counts per one symbol. The minimum allowed value is 0 (i.e., one count per symbol).
 * \details In fact, the 1/16 is chosen as fraction (the corresponding factor value is 4) - just in order to have Symbol
 *  Clock equal to 1 us for the 2.45 GHz band with O-QPSK modulation.
 */
#define HAL_SYMBOL__LOG2_FREQ_FACTOR        (4)

/**//**
 * \brief   ML507 Symbol Timer Counter register.
 * \details The SYMBOL_COUNTER value contains the following fields:
 *  - bits 31..0 - COUNTER[31..0] - Current value of the Symbol Counter register. Software can overwrite this register.
 *
 * \details Assign the COUNTER[31..0] with zero to properly restart the Symbol Timer.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclause SYMBOL TIMER.
 */
#define ML507_REG__SYMBOL_COUNTER           (0x00C00038)

/**//**
 * \brief   Returns reference to the current timestamp according to the Symbol Counter.
 * \return  Reference to the current timestamp according to the Symbol Counter, in symbol fractions.
 */
#define HAL_Symbol__Tstamp()                (*SYS_REG32(ML507_REG__SYMBOL_COUNTER))

/**//**
 * \brief   Assignment of particular unused channels of the Symbol Timer for needs of ML507 platform.
 */
enum {
    ML507_SYMBOL_MATCH__PHY_TIMED_TX    = 7,    /*!< Timed ACK transmission timer for PHY. */
};

/************************* PROTOTYPES *********************************************************************************/
/**//**
 * \name    Set handler-functions for Symbol Timer Match events for needs of ML507 platform.
 */
/**@{*/
extern void ML507_Symbol_Match__PHY_TimedTx(void);      /*!< Channel #7: Timed ACK transmission timer for PHY. */
/**@}*/

#endif /* _BB_ML507_SYM_TMR_H */
