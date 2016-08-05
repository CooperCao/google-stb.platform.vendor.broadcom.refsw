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
* FILENAME: $Workfile: trunk/stack/common/HAL/include/bbHalSymTmr.h $
*
* DESCRIPTION:
*   Symbol Timer Driver interface.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
*****************************************************************************************/

#ifndef _BB_HAL_SYM_TMR_H
#define _BB_HAL_SYM_TMR_H

/************************* INCLUDES ***********************************************************************************/
#include "bbSysBasics.h"

#ifdef __SoC__
# include "bbSocSymTmr.h"
#elif defined(__ML507__)
# include "bbMl507SymTmr.h"
#else /* __i386__ */
#define HAL_SYMBOL__LOG2_FREQ_FACTOR 5
#endif

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   The default symbol rate to be assigned to the Symbol Timer at startup.
 * \details Symbol rate 62.5 ksymbol/s correspond to 2.45 GHz band with O-QPSK modulation.
 */
#define HAL_SYMBOL_RATE__DEFAULT_HZ         (62500)

#ifndef HAL_SYMBOL__LOG2_FREQ_FACTOR
# error The LOG2-factor of the Symbol Clock frequency is not defined. Define its value in the range from 0 to 5.
#elif (HAL_SYMBOL__LOG2_FREQ_FACTOR < 0 || HAL_SYMBOL__LOG2_FREQ_FACTOR > 5)
# error The LOG2-factor of the Symbol Clock frequency is outside the allowed range from 0 to 5.
#endif

/**//**
 * \brief   Data type for the Symbol Timer timestamp, in symbol fractions.
 * \note    When necessary, this type may also be used for timestamps in whole symbols.
 */
typedef uint32_t  HAL_Symbol__Tstamp_t;

/**//**
 * \brief   Data type for the Symbol Timer timeshift, signed integer, in symbol fractions.
 * \details Use this data type for representation of difference between two timestamps.
 * \note    The minuend A timestamp is considered to be in the future with respect to the subtrahend B timestamp if the
 *  difference (A - B), expressed as a timeshift, receives positive value. Otherwise, if the difference is negative, the
 *  timestamp A is considered in the past with respect to the timestamp B.
 * \note    When necessary, this type may also be used for timeshifts in whole symbols.
 */
typedef int32_t  HAL_Symbol__Tshift_t;

/**//**
 * \brief   The minimum safe value of timeshift according to the Symbol Clock (Symbol Counter) frequency factor, symbol
 *  rate and CPU main clock.
 * \details The smallest safe value of the timeshift for the Symbol Counter in order not to run over the appointed
 *  timestamp without interrupt triggering when the compare-match channel is being appointed is 2 symbol clock periods,
 *  where the first unit is provided for quantization of the current timestamp and the second unit guarantees the front
 *  edge transition on the compare-match channel to generate interrupt. This limit is theoretically achievable in the
 *  easiest case: frequency factor 0, symbol rate 12.5 ksymbol/s, CPU main clock 54 MHz. For different cases this limit
 *  must be increased because CPU may not be in time with all the necessary calculations while performing appointment.
 *  The worst case: frequency factor 5, symbol rate 62.5 ksymbol/s, CPU main clock 27 MHz.
 * \details The following results were observed on SoC at symbol rate 62.5 ksymbol/s, CPU main clock 54 MHz:
 *  - frequency factor 0..3 - minimal safe timeshift 2
 *  - frequency factor 4 - minimal safe timeshift 3
 *  - frequency factor 5 - minimal safe timeshift 6
 *
 * \details The following results were observed on SoC at symbol rate 62.5 ksymbol/s, CPU main clock 27 MHz:
 *  - frequency factor 0..2 - minimal safe timeshift 2
 *  - frequency factor 3 - minimal safe timeshift 3
 *  - frequency factor 4 - minimal safe timeshift 5
 *  - frequency factor 5 - minimal safe timeshift 8
 *
 * \details According to collected results the following set of safe timeshifts is accepted for all possible cases:
 *  - frequency factor 0..1 - minimal safe timeshift 2
 *  - frequency factor 2 - minimal safe timeshift 3
 *  - frequency factor 3 - minimal safe timeshift 4
 *  - frequency factor 4 - minimal safe timeshift 6
 *  - frequency factor 5 - minimal safe timeshift 9 or 10
 *
 * \details The set introduced above, for frequency factors in the range from 2 to 5, may be expressed with the
 *  formula:<\br>
 *      safe_timeshift = 2 ^ (factor - 2) + 2
 */
#if (HAL_SYMBOL__LOG2_FREQ_FACTOR <= 1)
# define HAL_SYMBOL__SAFE_TSHIFT            ((HAL_Symbol__Tshift_t)(2))
#else
# define HAL_SYMBOL__SAFE_TSHIFT            ((HAL_Symbol__Tshift_t)((1 << (HAL_SYMBOL__LOG2_FREQ_FACTOR - 2)) + 2))
#endif

/**//**
 * \brief   Returns the safe value for the specified timeshift according to the Symbol Clock frequency factor.
 * \param[in]   t       Arbitrary value of the timeshift, signed integer, in symbol fractions.
 * \return  Safe value of the timeshift, in symbol fractions.
 * \details Use this macro function to avoid the situation of the Symbol Counter to run over the calculated timestamp
 *  for the given (unsafe) timeshift. For example, if the Match event is being appointed for the timestamp that is
 *  already less than the current timestamp, or that equals to the current timestamp, the Match event will not be
 *  triggered. Then, even if the appointed timestamp is greater than the current timestamp, but the difference is too
 *  low (just one or two Symbol Counter units), the Symbol Counter may advance its value directly in the middle of the
 *  COMPARE register assignment and due to this reason the Match event will not be triggered also. This problem becomes
 *  actual for high symbol rates, small symbol fractions and slow CPU clocking.
 * \note    This macro is based on the MAX() macro and due to this reason may produce side effects on \p t if it's a
 *  complex expression or a function call. In such cases the caller shall introduce a local variable to hold the value
 *  of \p t passed as the argument to this macro.
 */
#define HAL_Symbol__Safe(t)                 (MAX(((HAL_Symbol__Tshift_t)(t)), HAL_SYMBOL__SAFE_TSHIFT))

/**//**
 * \brief   Converts a time period expressed in whole symbols to symbol fractions.
 * \param[in]   t       The time period, in whole symbols.
 * \return  The time period, in symbol fractions.
 * \note    This macro does not perform validation of \p t and shall be used carefully - i.e., only when it is known
 *  that \p t will not overrun the result.
 */
#define HAL_Symbol__SymbToFrac(t)           (((HAL_Symbol__Tshift_t)(t)) << HAL_SYMBOL__LOG2_FREQ_FACTOR)

/**//**
 * \brief   Converts a time period expressed in symbol fractions to whole symbols, truncating the uneven result
 *  downwards (as signed integers - i.e., to zero) to the nearest integer value.
 * \param[in]   t       The time period, in symbol fractions.
 * \return  The time period, in whole symbols.
 * \details Use this macro function to calculate the remaining time to the existent appointment in whole symbols, when
 *  shall decide if the new appointment (in whole symbols) is closer to the current moment than the existent one.
 */
#define HAL_Symbol__FracToSymb(t)           (((HAL_Symbol__Tshift_t)(t)) >> HAL_SYMBOL__LOG2_FREQ_FACTOR)

/**//**
 * \brief   Converts a time period expressed in microseconds to symbol fractions at the symbol rate 62.5 ksymbol/s.
 * \param[in]   t       The time period, in microseconds.
 * \return  The time period, in symbol fractions, at the symbol rate 62.5 ksymbol/s.
 * \details This function rounds the returned value down.
 * \note    This implementation is specific to the symbol rate 62.5 ksymbol/s, where single symbol equals 16 us exactly.
 */
#if (HAL_SYMBOL__LOG2_FREQ_FACTOR < 4)
# define HAL_Symbol__UsecToFrac_62k5(t)     (((HAL_Symbol__Tshift_t)(t)) >> (4 - HAL_SYMBOL__LOG2_FREQ_FACTOR))
#else
# define HAL_Symbol__UsecToFrac_62k5(t)     (((HAL_Symbol__Tshift_t)(t)) << (HAL_SYMBOL__LOG2_FREQ_FACTOR - 4))
#endif

/**//**
 * \brief   Converts a time period expressed in symbol fractions to microseconds at the symbol rate 62.5 ksymbol/s.
 * \param[in]   t       The time period, in symbol fractions.
 * \return  The time period, in microseconds, at the symbol rate 62.5 ksymbol/s.
 * \details This function rounds the returned value down.
 * \note    This implementation is specific to the symbol rate 62.5 ksymbol/s, where single symbol equals 16 us exactly.
 */
#if (HAL_SYMBOL__LOG2_FREQ_FACTOR < 4)
# define HAL_Symbol__FracToUsec_62k5(t)     (((HAL_Symbol__Tshift_t)(t)) << (4 - HAL_SYMBOL__LOG2_FREQ_FACTOR))
#else
# define HAL_Symbol__FracToUsec_62k5(t)     (((HAL_Symbol__Tshift_t)(t)) >> (HAL_SYMBOL__LOG2_FREQ_FACTOR - 4))
#endif

/**//**
 * \brief   Converts a time period expressed in octets to 1/10 symbol fractions.
 * \param[in]   t       The time period, in octets.
 * \return  The time period, in 1/10 fractions of whole symbol, for the modulation scheme consuming 2 symbols per octet.
 * \note    This implementation is specific to the O-QPSK modulation scheme at 2.45 GHz and 868/915 MHz bands that
 *  consumes 2 symbols per octet.
 */
#define HAL_Symbol__OctetToSymbX10_OQPSK(t)         (((HAL_Symbol__Tshift_t)(t)) * 20)

/**//**
 * \brief   Enumeration of Symbol Timer compare-match channels and their assignment.
 * \details Unused channels may be enabled on particular platforms.
 * \details Channels are numbered from 0 to N-1, where N is the number of channels.
 * \details Channels #0~5 are the low-priority channels; channels #6 and #7 are the high-priority channels.
 */
enum HAL_Symbol__Match_Channel_t {
    HAL_SYMBOL_MATCH__MFE_EXPIRED   = 0,    /*!< Channel #0: Timeout timer for pending transactions. */
    HAL_SYMBOL_MATCH__MFE_TIMEOUT   = 1,    /*!< Channel #1: Timeout/delay timer for MAC-FE FSMs. */
    HAL_SYMBOL_MATCH__MLE_TRX_OFF   = 2,    /*!< Channel #2: Timed transceiver switching off timer. */
    HAL_SYMBOL_MATCH__CHANNEL_3     = 3,    /*!< Channel #3: Unused. May be assigned on particular platform. */
    HAL_SYMBOL_MATCH__CHANNEL_4     = 4,    /*!< Channel #4: Unused. May be assigned on particular platform. */
    HAL_SYMBOL_MATCH__CHANNEL_5     = 5,    /*!< Channel #5: Unused. May be assigned on particular platform. */
    HAL_SYMBOL_MATCH__MRTS_TIMEOUT  = 6,    /*!< Channel #6: Timeout/delay timer for MAC RTS Flowcharts. */
    HAL_SYMBOL_MATCH__PHY_CONFIRM   = 7,    /*!< Channel #7: Trigger for issuing postponed confirmation from PHY. */
};

/**//**
 * \brief   Total number of channels.
 * \details Channels are numbered from 0 to N-1, where N is the number of channels.
 */
#define HAL_SYMBOL__CHANNELS_NUM            (8)

/**//**
 * \brief   Number of the low-priority channels.
 * \details Channels are numbered from 0 to N-1, where N is the number of channels.
 * \details Channels #0~5 are the low-priority channels; channels #6 and #7 are the high-priority channels.
 */
#define HAL_SYMBOL__CHANNELS_NUM_LP         (6)

/**//**
 * \brief   Type for the Symbol Timer compare-match event handler.
 */
typedef void HAL_Symbol__Match_Handler_t(void);

/************************* PROTOTYPES *********************************************************************************/
/**//**
 * \brief   Initializes the Symbol Timer Driver.
 * \details This function performs the following:
 *  - Configure ARC interrupts used by the Symbol Timer Driver hardware and software.
 *  - Disengage all the Symbol Timer channels and disable interrupts from them.
 *  - Clear spurious interrupt requests from the Symbol Timer.
 *  - Set the Symbol Timer prescaler default Symbol Rate.
 *  - Reset Prescaler and Symbol Counters.
 *
 * \details This function must be called once on the application startup. All interrupts of Level 1 and Level 2 must be
 *  disabled during this function execution. The hardware must be reset/disabled/stopped prior to call this function.
 * \note    This function does not enable ARC interrupts. Interrupts must be enabled by the application startup routine
 *  after all the necessary software and hardware are configured.
 */
void HAL_Symbol__Init(void);

/**//**
 * \brief   Configures the Symbol Timer symbol rate.
 * \param[in]   symbolRate      Value of the desired symbol rate, in symbol/s.
 * \details This function assigns the Symbol Prescaler register to perform generation of the Symbol Clock at the
 *  specified symbol rate multiplied by the predefined frequency factor.
 * \details The Symbol Counter register is left unchanged by this function. By these means the timestamping remains
 *  progressive after the Symbol Timer reconfiguration. All compare-match statuses and channel assignments are also left
 *  unchanged in order not to disturb other software units functioning.
 * \details The Prescaler Counter is reset to zero by this function. It is performed in order to arrange correct
 *  switching of symbol frequency in the case when the Prescaler register is assigned with a value that is smaller than
 *  the previous one.
 * \note    It is recommended to call this function right after reset of the MAC and PHY. Hence, it's not necessary.
*/
void HAL_Symbol__Config(const uint32_t symbolRate);

/**//**
 * \brief   Appoints the specified Symbol Timer channel to trigger at the given timeshift from the current moment.
 *  Applicable to all channels from 0 to 7.
 * \param[in]   tshift      Timeshift to appointment from the current timestamp, in symbol fractions. Must be greater or
 *  equal to zero.
 * \param[in]   channel     The identifier of a Symbol Timer channel to be used for the appointment.
 * \details This function makes timed appointment to the Symbol Timer channel to call its statically linked handler
 *  function in \p tshift symbol fractions from the current moment. The Symbol Timer channel specified with the
 *  \p channel must be free from appointments at the moment when this function is called. The appointment is triggered
 *  only once, and after that the Symbol Timer channel is disengaged.
 * \note    Negative values are prohibited for the \p tshift. Zero timeshift, timeshift equal to a single symbol
 *  fraction (i.e., single Symbol Counter unit), and extremely low timeshifts (in the case of high Symbol Counter
 *  frequency factor) are increased to the minimum safe timeshift value.
 */
void HAL_Symbol__Appoint(const HAL_Symbol__Tshift_t tshift, const enum HAL_Symbol__Match_Channel_t channel);

/**//**
 * \brief   Reappoints the specified Symbol Timer channel to trigger at the given timeshift from the current
 *  moment if the new timestamp is closer to the current moment than the previously appointed, or simply appoints the
 *  channel if there is no appointment on it at the moment. Applicable only to low-priority channels from 0 to 5.
 * \param[in]   tshift      Timeshift to new appointment from the current timestamp, in symbol fractions. Must be
 *  greater or equal to zero.
 * \param[in]   channel         The identifier of a Symbol Timer channel to be used for the new appointment.
 * \note    This function is applicable only to low-priority channels - channels from 0 to 5.
 * \details This function reappoints originally appointed event to the new \p tshift symbol fractions from the current
 *  moment, if the new timestamp is closer to the current moment then the previously (re-)appointed one. In the
 *  case when the Symbol Timer channel \p channel is currently free from an appointment (the original appointment had
 *  already triggered and has been handled, or there was no appointment at all), the new appointment is performed
 *  unconditionally.
 * \note    Negative values are prohibited for the \p tshift. Zero timeshift, timeshift equal to a single symbol
 *  fraction (i.e., single Symbol Counter unit), and extremely low timeshifts (in the case of high Symbol Counter
 *  frequency factor) are increased to the minimum safe timeshift value.
 */
void HAL_Symbol__Reappoint_LP(const HAL_Symbol__Tshift_t tshift, const enum HAL_Symbol__Match_Channel_t channel);

/**//**
 * \brief   Recalls previously appointed (and probably just triggered but not handled) Match event from the specified
 *  Symbol Timer channel. Applicable to all channels from 0 to 7.
 * \param[in]   channel     The identifier of a Symbol Timer channel to be freed from its appointment.
 * \details This function disengages the corresponding Symbol Timer channel. If the specified channel is free at the
 *  moment, this function does nothing.
 */
void HAL_Symbol__Recall(const enum HAL_Symbol__Match_Channel_t channel);

/**//**
 * \name    Set of handler-functions for Symbol Timer Match events.
 * \details This set may be augmented on particular platforms.
 */
/**@{*/
extern void HAL_Symbol_Match__MFE_Expired(void);        /*!< Channel #0: Timeout timer for pending transactions. */
extern void HAL_Symbol_Match__MFE_Timeout(void);        /*!< Channel #1: Timeout/delay timer for MAC-FE FSMs. */
extern void HAL_Symbol_Match__MLE_TrxOff(void);         /*!< Channel #2: Timed transceiver switching off timer. */
extern void HAL_Symbol_Match__MRTS_Timeout(void);       /*!< Channel #6: Timeout/delay timer for MAC RTS Flowcharts. */
extern void HAL_Symbol_Match__PHY_Confirm(void);        /*!< Channel #7: Trigger for issuing confirmation from PHY. */
/**@}*/

#endif /* _BB_HAL_SYM_TMR_H */
