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
* FILENAME: $Workfile: trunk/stack/common/HAL/include/bbHalSymTmr.h $
*
* DESCRIPTION:
*   Symbol Timer Hardware interface.
*
* $Revision: 3943 $
* $Date: 2014-10-07 20:55:38Z $
*
*****************************************************************************************/


#ifndef _BB_HAL_SYM_TMR_H
#define _BB_HAL_SYM_TMR_H


/************************* INCLUDES *****************************************************/
#include "bbSysBasics.h"            /* Basic system environment set. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of codes for Symbol Timer Hardware symbol rate.
 */
/* TODO: Think how to reduce enumeration for the incomplete set of channels supported by the radio. */
typedef enum _HAL_SymbolRateId_t
{
    HAL_SYMBOL_RATE_12K5,           /*!< Symbol rate 12500 symbols/s. */

    HAL_SYMBOL_RATE_20K,            /*!< Symbol rate 20000 symbols/s. */

    HAL_SYMBOL_RATE_25K,            /*!< Symbol rate 25000 symbols/s. */

    HAL_SYMBOL_RATE_40K,            /*!< Symbol rate 40000 symbols/s. */

    HAL_SYMBOL_RATE_50K,            /*!< Symbol rate 50000 symbols/s. */

    HAL_SYMBOL_RATE_62K5,           /*!< Symbol rate 62500 symbols/s. */

    HAL_SYMBOL_RATES_NUMBER,        /*!< Total number of different supported symbol rates. */

} HAL_SymbolRateId_t;


/**//**
 * \brief   The LOG2 of the Symbol Counter Hardware frequency factor.
 * \details The Symbol Counter frequency is calculated as <em>f_cnt = f_sym * 2 ^ k</em>
 *  where:
 *  - \e k is the value defined with this constant (the LOG2 of the frequency factor),
 *  - \e f_sym is the desired symbol frequency, in symbol/s (i.e., in whole symbols per
 *      second),
 *  - \e f_cnt is the Symbol Counter frequency, in counts/s (i.e., in symbol quotients per
 *      second).
 *
 * \details The maximum allowed value of this factor in order not to exceed the 32-bit
 *  unsigned integer rank (0xFFFFFFFF) and support signed durations up to 0x03FFFFFF (the
 *  maximum lifetime of indirect transaction) is 5 - i.e., 32 counts per one symbol. The
 *  minimum allowed value is 0 (i.e., one count per symbol).
 */
#define HAL_SYMBOL_LOG2_FREQ_FACTOR         4

/*
 * Validate HAL_SYMBOL_LOG2_FREQ_FACTOR.
 */
SYS_DbgAssertStatic(HAL_SYMBOL_LOG2_FREQ_FACTOR >= 0);
SYS_DbgAssertStatic(HAL_SYMBOL_LOG2_FREQ_FACTOR <= 5);


/**//**
 * \brief   Data type for the Symbol Timer Hardware timestamp, in symbol quotients.
 * \details Use this data type for storing timestamps, the moments in the time when an
 *  appointed signal must be issued, etc.
 * \details This type may also be used for timestamps in whole symbols.
 */
typedef uint32_t  HAL_SymbolTimestamp_t;


/**//**
 * \brief   Data type for the Symbol Timer Hardware timeshift, in symbol quotients.
 * \details Use this data type for representation of difference between two timestamps, in
 *  symbol quotients. In particular this type may be used to discover if the specified
 *  timestamp is in the past or in the future from the current timestamp.
 * \note    This type is signed 32-bit integer. The negative values are intended in
 *  general for representation of timeshifts to timestamps in the past from the current
 *  timestamp.
 */
typedef int32_t  HAL_SymbolTimeshift_t;


/**//**
 * \brief   Data type for the Symbol Timer Hardware period of time, in whole symbols.
 * \details Use this data type for representation of periods of time (durations) or
 *  difference between two moments in the time, in whole symbols.
 * \note    This type is signed 32-bit integer. The negative values are intended in
 *  general for representation of timeshifts to moments in the past from the current
 *  moment.
 */
typedef HAL_SymbolTimeshift_t  HAL_SymbolPeriod_t;


/**//**
 * \brief   The maximum supported value for time period, in whole symbols.
 */
#define HAL_SYMBOL_MAX_PERIOD                   (HAL_SymbolQuotToSymb(INT32_MAX))


/**//**
 * \brief   The minimum safe value of timeshift according to the Symbol Counter frequency
 *  factor.
 * \details The smallest safe value of the timeshift for the Symbol Counter in order not
 *  to run over the appointed timestamp without interrupt triggering when the
 *  compare-match channel is being appointed is 2 units, where one unit is provided for
 *  quantization of the current timestamp and the second unit is to guarantee the front
 *  edge transition on the compare-match channel to generate interrupt. The value 1 is
 *  potentially unsafe. The value 0 is not allowed. For the Symbol Counter Hardware high
 *  frequencies the minimum safe value must be increased.
 * \details For Symbol Counter LOG2 frequency factor from 0 to 2 the minimum safe timeshift
 *  is equal to 2 units. For the worst case of 27 MHz main CPU clock, 62500 symbol/s
 *  desired symbol rate and LOG2 frequency factor equal to 2 (i.e., the absolute frequency
 *  factor equal to 4) a single unit is equal to 4.0 us or 108 CPU clocks.
 * \details For Symbol Counter LOG2 frequency factor \e k greater then 2 the minimum safe
 *  timeshift is calculated with formula: <em>timeshift_min = 2 ^ (k - 2) + 1</em>.
 */
#if (HAL_SYMBOL_LOG2_FREQ_FACTOR <= 2)
# define HAL_SYMBOL_MINIMUM_SAFE_TIMESHIFT      2
#else
# define HAL_SYMBOL_MINIMUM_SAFE_TIMESHIFT      ((1 << (HAL_SYMBOL_LOG2_FREQ_FACTOR - 2)) + 1)
#endif


/**//**
 * \brief   Returns the safe value for the specified timeshift according to the Symbol
 *  Counter frequency factor.
 * \param[in]   timeshift   Arbitrary value of the timeshift (signed integer), in symbol
 *  quotients.
 * \return  Safe value of the timeshift, in symbol quotients.
 * \details Use this macro function to avoid the situation of the Symbol Counter Hardware
 *  to run over the calculated timestamp for the given (unsafe) timeshift. For example, if
 *  the time-signal is being appointed for the timestamp that is equal to the current
 *  timestamp plus one (i.e., plus a single System Counter unit), the System Counter
 *  Hardware may already advance for this single (or even more) units ahead, and
 *  consequently the Symbol Timer Hardware interrupt will not be requested (or the timed
 *  transmission will not be started). This problem is actual for fast symbol rates and
 *  extremely low timeshifts.
 * \note    It is not recommended to use global variables (or references to global
 *  variables) for the \p timeshift, because the MAX() macro used in the formula will
 *  produce doubled code to read and process the value. It is not allowed also to directly
 *  use for the \p timeshift functions or expressions returning the timeshift. In both
 *  cases use local buffer variable to pass the argument to this macro function.
 */
#define HAL_SymbolSafeTimeshift(timeshift)      (MAX((timeshift), HAL_SYMBOL_MINIMUM_SAFE_TIMESHIFT))


/**//**
 * \brief   Converts a timeshift, given in symbol quotients, to the time period, in whole
 *  symbols, truncating downwards to integer value.
 * \param[in]   timeshift   The timeshift, in symbol quotients, to be converted.
 * \return  The time period, in whole symbols. Periods that are not even truncated
 *  downwards (to zero) to whole periods in symbols.
 * \details Use this macro function to calculate the remaining time to the existent
 *  appointment, when shall decide if the new appointment (in whole symbols) is closer to
 *  the current moment then the existent one.
 */
#define HAL_SymbolQuotToSymb(timeshift)         ((timeshift) >> HAL_SYMBOL_LOG2_FREQ_FACTOR)


/*
 * Auxiliary macro for the HAL_SymbolSymbToQuot() function.
 */
#define HAL_SymbolSymbToQuot_(period)           ((period) << HAL_SYMBOL_LOG2_FREQ_FACTOR)

/**//**
 * \brief   Converts a time period, given in whole symbols, to the timeshift, in symbol
 *  quotients.
 * \param[in]   period      The time period, in whole symbols, to be converted.
 * \return  The timeshift, in symbol quotients.
 */
#if defined(_DEBUG_COMPLEX_)
INLINE HAL_SymbolTimeshift_t HAL_SymbolSymbToQuot(HAL_SymbolPeriod_t period)
{
    SYS_DbgAssertComplex(-HAL_SYMBOL_MAX_PERIOD <= period && period <= HAL_SYMBOL_MAX_PERIOD,
            LOG_HAL_SymbolSymbToQuot_InvalidPeriod);

    return HAL_SymbolSymbToQuot_(period);
}
#else
# define HAL_SymbolSymbToQuot(period)           (HAL_SymbolSymbToQuot_(period))
#endif


/**//**
 * \brief   Converts a time interval duration, given in microseconds, to the timeshift, in
 *  symbol quotients.
 * \param[in]   interval    The time interval duration, in microseconds, to be converted.
 * \return  The timeshift, in symbol quotients. Periods that are not even augmented
 *  upwards to the whole periods in symbol quotients.
 */
/* TODO: This formula is valid only for 62.5 ksym/sec and frequency factor = 4. */
#define HAL_SymbolUsecToQuot(interval)          (interval)


/**//**
 * \brief   Converts a time interval duration, given in octets, to the period, in 1/10 of
 *  whole symbol.
 * \param[in]   interval    The time interval duration, in octets, to be converted.
 * \return  The timeshift, in 1/10 of whole symbol.
 */
/* TODO: This formula is valid only for 62.5 ksym/sec and frequency factor = 4. */
#define HAL_SymbolOctetToSymbX10(interval)      ((interval) * 20)


/**//**
 * \brief   Enumeration of identifiers of the Symbol Timer Hardware channels.
 * \note    Symbol Timer Hardware compare-match channels have the following relative
 *  priority:
 *  - the channel #7 has the highest priority; its IRQ has the mid level of hardware
 *      priority and is served by dedicated handler-function; this channel is used for
 *      timed transmissions on ML507 with external Atmel Radio Hardware,
 *  - the channel #6 has the next priority; its IRQ has low (common) level of hardware
 *      priority but is also served by dedicated handler-function,
 *  - channels #0-5 have the lowest priority; their IRQs have low (common) level of
 *      hardware priority and they share common IRQ handler-function. Amongst these
 *      channels the channel #5 has the highest priority and the channel #0 has the lowest
 *      priority.
 *
 * \details Identifiers in the enumeration are listed here in their relative priority
 *  order from lowest to highest. Do not change assignment of the Symbol Timer Hardware
 *  channels.
 */
/* TODO: Channel #7 has the low (common) level of HW priority. */
typedef enum _HAL_SymbolChannelId_t
{
    HAL_SYMBOL_CHANNEL_MFE_EXPIRED    = 0,      /*!< Channel #0 (the lowest priority) is used by the MAC-FE Transactions
                                                    Dispatcher for appointment of the EXPIRED task. */

    HAL_SYMBOL_CHANNEL_MFE_TIMEOUT    = 1,      /*!< Channel #1 is shared by MAC-FE Request Processors for appointment
                                                    of the TIMEOUT task. */

    HAL_SYMBOL_CHANNEL_MLE_TRX_OFF    = 2,      /*!< Channel #2 is used by the MAC-LE Transceiver Mode Dispatcher for
                                                    timed switching from the RX_ON to the TRX_OFF state after
                                                    MLME-RX-ENABLE.request and after Frame Pending subfield in received
                                                    ACK frame. */

    HAL_SYMBOL_CHANNEL_MLE_DISP_FSM   = 3,      /*!< Channel #3 is used by the MAC-LE Real-Time Dispatcher FSM when
                                                    performing different timed processes. */

    HAL_SYMBOL_CHANNEL_PHY_CONF       = 4,      /*!< Channel #4 is used by the PHY for asynchronous issuing of
                                                    confirmations on requests from the MAC-LE. */                           /* TODO: Think to use SW triggered IRQ on any empty vector. */

    HAL_SYMBOL_CHANNEL_PHY_POLL       = 5,      /*!< Channel #5 is used by the PHY for postponed polling of the Radio
                                                    Hardware transceiver state. */

    HAL_SYMBOL_SHARED_CHANNELS_NUMBER = 6,      /*!< Number of Symbol Timer Hardware channels sharing the same hardware
                                                    interrupt request - channels #0-5. */

    HAL_SYMBOL_CHANNEL_HAL_CHANNEL    = 6,      /*!< Channel #6 is used by the Radio HAL for postponed confirmation on
                                                    channel switching. */

    HAL_SYMBOL_CHANNEL_HAL_TIMED_TX   = 7,      /*!< Channel #7 (the highest priority) is used by the ML507 Radio HAL
                                                    for timed data transmission; not used on the SoC. */

    HAL_SYMBOL_CHANNELS_NUMBER        = 8,      /*!< Total number of Symbol Timer Hardware channels. */

} HAL_SymbolChannelId_t;


/**//**
 * \brief   Template for the callback handler-function of the Symbol Timer Hardware
 *  time-event.
 * \details To issue a previously appointed time-event signal to the dedicated destination
 *  the Symbol Timer Software calls the callback handler-function that was appointed for
 *  the desired timestamp and Symbol Counter channel.
 */
typedef void HAL_SymbolMatchHandler_t(void);


/**//**
 * \brief   Initializes and configures the Symbol Timer Hardware.
 * \details This function perform full hardware reset of the Symbol Timer. All
 *  compare-match channels are disabled (masked), prescaler counter and symbol counter
 *  registers are zeroed (i.e., timestamp is set to zero), prescaler register is assigned
 *  with the default value for the Radio Hardware.
 */
#if defined(__SoC__)
# define HAL_SymbolInit()       SOC_SymbolInit()
#
#elif defined(__ML507__)
# define HAL_SymbolInit()       ML507_SymbolInit()
#
#else /* __i386__ */
# define HAL_SymbolInit()       PC_SymbolInit()
#
#endif


/**//**
 * \brief   Configures the Symbol Timer Hardware.
 * \param[in]   symbolRateId    Identifier code of the desired symbol rate.
 * \details This function assigns Symbol Counter hardware Prescaler register to perform
 *  generation of counts at the specified symbol rate multiplied by the frequency factor.
 * \details The value of Symbol Counter is left unchanged by this function. By these means
 *  the timestamping remains progressive. All compare-match statuses and channels
 *  assignments are also left unchanged in order not to disturb other software units
 *  functioning.
 * \details The Prescaler Counter is reset to zero by this function if its current value
 *  is greater then or equal to the new value of the Prescaler register being assigned. It
 *  is performed in order to arrange correct switching of symbol frequency in the case
 *  when the Prescaler register is configured to smaller value the the previous one.
 */
#if defined(__SoC__)
# define HAL_SymbolConfig(symbolRateId)     SOC_SymbolConfig(symbolRateId)
#
#elif defined(__ML507__)
# define HAL_SymbolConfig(symbolRateId)     ML507_SymbolConfig(symbolRateId)
#
#else /* __i386__ */
# define HAL_SymbolConfig(symbolRateId)     PC_SymbolConfig(symbolRateId)
#
#endif


/**//**
 * \brief   Returns the current timestamp according to the Symbol Counter Hardware.
 * \return  The current timestamp according to the Symbol Counter Hardware, in symbol
 *  quotients.
 * \note    The value of timestamp is reset to zero only on the application startup; it is
 *  not reset when the Symbol Timer Hardware is (re-)configured for particular symbol
 *  frequency by the PHY.
 */
#if defined(__SoC__)
# define HAL_SymbolTimestamp()      SOC_SymbolTimestamp()
#
#elif defined(__ML507__)
# define HAL_SymbolTimestamp()      ML507_SymbolTimestamp()
#
#else /* __i386__ */
# define HAL_SymbolTimestamp()      PC_SymbolTimestamp()
#
#endif


/**//**
 * \brief   Appoints the specified Symbol Timer Hardware channel to call its
 *  handler-function at the given timeshift from the current moment.
 * \param[in]   timeshift   Timeshift from the current timestamp to appoint the timer, in
 *  symbol quotients. Must be greater or equal to zero.
 * \param[in]   channelId   Identifier of the Symbol Timer Hardware channel to be used for
 *  the time-event appointment.
 * \details This function makes timed appointment to the Symbol Timer Hardware to call its
 *  statically linked handler-function in the future in \p timeshift symbol quotients from
 *  the current moment. The Symbol Timer channel specified with the \p channelId must be
 *  free from appointments at the moment when this function is called. The appointment is
 *  triggered only once, and after that the Symbol Timer channel is disengaged.
 * \details Negative values are prohibited for the \p timeshift. Zero timeshift, timeshift
 *  equal to a single symbol quotient (i.e., single Symbol Counter unit), and extremely
 *  low timeshifts (in the case of high Symbol Counter frequency factor) are increased to
 *  the minimum safe timeshift value.
 */
#if defined(__SoC__)
# define HAL_SymbolAppoint(timeshift, channelId)        SOC_SymbolAppoint(timeshift, channelId)
#
#elif defined(__ML507__)
# define HAL_SymbolAppoint(timeshift, channelId)        ML507_SymbolAppoint(timeshift, channelId)
#
#else /* __i386__ */
# define HAL_SymbolAppoint(timeshift, channelId)        PC_SymbolAppoint(timeshift, channelId)
#
#endif


/**//**
 * \brief   Reappoints the specified Symbol Timer Hardware channel to call its
 *  handler-function at the newly given timeshift from the current moment if the new
 *  timestamp is closer to the current timestamp than the originally appointed one. Or
 *  simply appoints the channel if there is no appointment on it.
 * \param[in]   newTimeshift    New timeshift from the current timestamp to appoint the
 *  timer, in symbol quotients. Must be greater or equal to zero.
 * \param[in]   channelId       Identifier of the Symbol Timer Hardware channel to be used
 *  for the time-event appointment.
 * \details This function reappoints originally appointed event to the \p newTimeshift if
 *  the new timestamp is closer to the current moment then the previously (re-)appointed
 *  one. If the Symbol Timer channel specified with the \p channelId is currently free
 *  from an appointment (the original appointment was already triggered, or there was no
 *  appointment at all), the new appointment is performed unconditionally.
 * \details Negative values are prohibited for the \p newTimeshift. Zero timeshift,
 *  timeshift equal to a single symbol quotient (i.e., single Symbol Counter unit), and
 *  extremely low timeshifts (in the case of high Symbol Counter frequency factor) are
 *  increased to the minimum safe timeshift value.
 */
#if defined(__SoC__)
# define HAL_SymbolReappoint(newTimeshift, channelId)       SOC_SymbolReappoint05(newTimeshift, channelId)
#
#elif defined(__ML507__)
# define HAL_SymbolReappoint(newTimeshift, channelId)       ML507_SymbolReappoint(newTimeshift, channelId)
#
#else /* __i386__ */
# define HAL_SymbolReappoint(newTimeshift, channelId)       PC_SymbolReappoint05(newTimeshift, channelId)
#
#endif


/**//**
 * \brief   Recalls previously appointed time-event from the specified Symbol Timer
 *  Hardware channel.
 * \param[in]   channelId   Identifier of the Symbol Timer Hardware channel to be freed
 *  from its appointment.
 * \details This function disengages the corresponding Symbol Timer Hardware channel. If
 *  the specified channel is free at the moment, this function does nothing.
 */
#if defined(__SoC__)
# define HAL_SymbolRecall(channelId)        SOC_SymbolRecall05(channelId)
#
#elif defined(__ML507__)
# define HAL_SymbolRecall(channelId)        ML507_SymbolRecall(channelId)
#
#else /* __i386__ */
# define HAL_SymbolRecall(channelId)        PC_SymbolRecall05(channelId)
#
#endif


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Callback handler-function for the MAC-FE EXPIRED timed-event.
 * \details
 *  This function schedules the EXPIRED task for the MAC-FE.
*****************************************************************************************/
HAL_PUBLIC HAL_SymbolMatchHandler_t  HAL_SymbolHandlerMfeExpired;


/*************************************************************************************//**
 * \brief   Callback handler-function for the MAC-FE TIMEOUT timed-event.
 * \details
 *  This function schedules the TIMEOUT task for the MAC-FE.
*****************************************************************************************/
HAL_PUBLIC HAL_SymbolMatchHandler_t  HAL_SymbolHandlerMfeTimeout;


/*************************************************************************************//**
 * \brief   Callback handler-function for the MAC-LE TRX_OFF timed-event.
 * \details
 *  This functions is called on the appointed time when one of the TRX_ON_WHEN_IDLE mode
 *  timeouts elapses. It compares the current timestamp with saved timestamps of different
 *  timeouts and decides which one of timeouts has just elapsed. Finally, when all
 *  timeouts have elapsed and there is no persistent TRX_ON_WHEN_IDLE assignment, then the
 *  transceiver is switched off.
*****************************************************************************************/
HAL_PUBLIC HAL_SymbolMatchHandler_t  HAL_SymbolHandlerMleTrxOff;


/*************************************************************************************//**
 * \brief   Callback handler-function for the MAC-LE TIMEOUT timed-event.
 * \details
 *  The TIMEOUT signal is appointed by the MAC-LE Real-Time Dispatcher FSM to itself in
 *  order to establish timeouts for the following processes: waiting for the IFS period
 *  prior to starting CSMA-CA and transmission, waiting for the CSMA-CA backoff period,
 *  waiting for the acknowledgment frame reception after transmission for not more then
 *  macAckWaitDuration, waiting for the whole period of Energy Detection scanning.
 * \note
 *  The acknowledgment frame timed transmission on a unicast frame reception is performed
 *  without TIMEOUT event, but with help of hardware capabilities to establish timed
 *  transmission and timestamping of all frames received.
*****************************************************************************************/
HAL_PUBLIC HAL_SymbolMatchHandler_t  HAL_SymbolHandlerMleDispFsm;


/*************************************************************************************//**
 * \brief   Callback handler-function for the PHY-SAP Confirm timed-event.
 * \details
 *  This handler-function is called by the Symbol Timer on an appointed signal if there is
 *  an appointment for issuing a PHY-SAP confirmation primitive. This function issues an
 *  appointed PHY-SAP confirmation primitive and resets appointment.
 * \details
 *  If for any reason at the moment of an appointed signal generation there is no
 *  confirmation appointed (i.e., 'No operation' is appointed), then this function does
 *  nothing.
*****************************************************************************************/
HAL_PUBLIC HAL_SymbolMatchHandler_t  HAL_SymbolHandlerPhyConf;


/*************************************************************************************//**
 * \brief   Callback handler-function for the PHY Radio State Poll timed-event.
 * \details
 *  This handler-function is called by the Symbol Timer on an appointed signal when need
 *  to discover (to poll) the current state of the Radio transceiver. This function polls
 *  the actual transceiver state until it will become defined. When the transceiver state
 *  become defined, this function appoints issuing of the PLME-SET-TRX-STATE.confirm, but
 *  only if there is no deferred state switching already appointed.
*****************************************************************************************/
HAL_PUBLIC HAL_SymbolMatchHandler_t  HAL_SymbolHandlerPhyPoll;


/*************************************************************************************//**
 * \brief   Callback handler-function for the RF-SET-CHANNEL.confirm timed-event.
 * \details
 *  Transfers the RF-SET-CHANNEL.confirm to the PHY from the Radio hardware for a just
 *  finished channel switching.
*****************************************************************************************/
HAL_PUBLIC HAL_SymbolMatchHandler_t  HAL_SymbolHandlerHalChannel;


/*************************************************************************************//**
 * \brief   Callback handler-function for immediate transmission of a PPDU prepared in the
 *  Radio Hardware Frame Buffer.
 * \details
 *  This function immediately starts transmission of a PPDU prepared in the Atmel
 *  AT86RF231 Radio Hardware Frame Buffer. This function does not perform Radio hardware
 *  transceiver switching and also it does not perform CSMA-CA algorithm, it just starts
 *  transmission if the Radio hardware is ready to perform it (i.e., the Radio is in the
 *  TX_ON state).
*****************************************************************************************/
HAL_PUBLIC HAL_SymbolMatchHandler_t  HAL_SymbolHandlerHalTimedTx;


/************************* INCLUDES *****************************************************/
#if defined(__SoC__)
# include "bbSocSymTmr.h"           /* SoC Symbol Timer Hardware interface. */
#elif defined(__ML507__)
# include "bbMl507SymTmr.h"         /* ML507 Symbol Timer Hardware interface. */
#else /* __i386__ */
# include "bbPcSymTmr.h"            /* i386 Symbol Timer Simulator interface. */
#endif


#endif /* _BB_HAL_SYM_TMR_H */