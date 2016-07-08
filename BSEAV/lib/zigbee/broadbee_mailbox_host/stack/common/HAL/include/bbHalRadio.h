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
* FILENAME: $Workfile: trunk/stack/common/HAL/include/bbHalRadio.h $
*
* DESCRIPTION:
*   IEEE Std 802.15.4-2006 compatible Radio Driver interface.
*
* $Revision: 10327 $
* $Date: 2016-03-08 02:34:51Z $
*
*****************************************************************************************/

#ifndef _BB_HAL_RADIO_H
#define _BB_HAL_RADIO_H

/************************* INCLUDES ***********************************************************************************/
#include "bbHalSymTmr.h"

#ifdef __SoC__
# include "bbSocRadio.h"
#else
# include "bbMl507Radio.h"
#endif

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \name    Enumerations for use with PHY Services Access Points (SAP).
 * \details Generally there is one main PHY enumeration introduced according to the Standard and a number of derivative
 *  enumerations actually used with particular PHY primitives.
 * \details The Radio Driver implements PHY primitives in the way almost according to the Standard but with the
 *  following changes that influence the use of PHY enumerations:
 *  - Radio Driver primitives must not be called in inappropriate Driver or Radio hardware state, or with invalid
 *      parameters. Due to this restriction, primitives never return (confirm with) failure statuses.
 *  - The PLME-SET-TRX-STATE.request accepts reduced set of state switching commands.
 *  - The PLME-SET-TRX-STATE.confirm returns (implies) the SUCCESS status instead of RX_ON, TX_ON, TRX_OFF if requested
 *      to switch to the same state as the currently set.
 *  - The PLME-CCA.confirm returns IDLE/BUSY status as a value of Boolean type instead of Enumeration.
 *  - The PLME-GET/SET primitives are implemented as the set of Getter and Setter synchronous functions (except the
 *      current channel and channel page switching), published variables and public constants, and due to this reason
 *      may not return failure statuses specific to PHY PAN Information Base (PIB) attributes accessing rules violation.
 */
/**@{*/
/**//**
 * \brief   PHY enumeration values.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.3, table 18.
 */
enum PHY_Enum_t {
    PHY__BUSY                   = 0x00,     /*!< The CCA attempt has detected a busy channel. */
    PHY__BUSY_RX                = 0x01,     /*!< The transceiver is asked to change its state while receiving. */
    PHY__BUSY_TX                = 0x02,     /*!< The transceiver is asked to change its state while transmitting. */
    PHY__FORCE_TRX_OFF          = 0x03,     /*!< The transceiver is to be switched off immediately. */
    PHY__IDLE                   = 0x04,     /*!< The CCA attempt has detected an idle channel. */
    PHY__INVALID_PARAMETER      = 0x05,     /*!< A SET/GET request was issued with a parameter in the primitive that is
                                                out of the valid range. */
    PHY__RX_ON                  = 0x06,     /*!< The transceiver is in or is to be configured into the receiver enabled
                                                state. */
    PHY__SUCCESS                = 0x07,     /*!< A SET/GET, an ED operation, or a transceiver state change was
                                                successful. */
    PHY__TRX_OFF                = 0x08,     /*!< The transceiver is in or is to be configured into the transceiver
                                                disabled state. */
    PHY__TX_ON                  = 0x09,     /*!< The transceiver is in or is to be configured into the transmitter
                                                enabled state. */
    PHY__UNSUPPORTED_ATTRIBUTE  = 0x0A,     /*!< A SET/GET request was issued with the identifier of an attribute that
                                                is not supported. */
    PHY__READ_ONLY              = 0x0B,     /*!< A SET/GET request was issued with the identifier of an attribute that
                                                is read-only. */
};
SYS_DbgAssertStatic(sizeof(enum PHY_Enum_t) == 1);

/**//**
 * \brief   Enumeration of PHY state codes.
 * \details This Radio Driver implements custom PLME-GET-TRX-STATE primitive that returns the current state of the
 *  Radio. The Radio state code returned may be one of conventional codes or the UNDEFINED (0x00) state.
 * \note    This enumeration indicates the state of the Radio Driver which in general correspond to the actual Radio
 *  hardware state. Hence, the state of the Radio hardware may differ during short periods from the Driver state until
 *  the Driver state is synchronized with the hardware state after corresponding Radio hardware interrupt servicing.
 * \note    Implementations on particular platforms may extend this enumeration with private codes - for example, for
 *  different phases of BUSY states (preparation and continuing), or different types of UNDEFINED state (initially
 *  undefined and in the middle of state switching).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.3, table 18.
 */
enum PHY_State_t {
    PHY_STATE__UNDEFINED    = 0x00,             /*!< The transceiver state is undefined. */
    PHY_STATE__BUSY_RX      = PHY__BUSY_RX,     /*!< The transceiver is during reception. */
    PHY_STATE__BUSY_TX      = PHY__BUSY_TX,     /*!< The transceiver is during transmission. */
    PHY_STATE__RX_ON        = PHY__RX_ON,       /*!< The transceiver is in the receiver enabled state. */
    PHY_STATE__TRX_OFF      = PHY__TRX_OFF,     /*!< The transceiver is in the transceiver disabled state. */
    PHY_STATE__TX_ON        = PHY__TX_ON,       /*!< The transceiver is in the transmitter enabled state. */
};
SYS_DbgAssertStatic(sizeof(enum PHY_State_t) == 1);

/**//**
 * \brief   Enumeration of PHY state switching commands.
 * \details The PLME-SET-TRX-STATE.request, as it is implemented in this Radio Driver, accepts reduced set of commands.
 *  Indeed, all three implemented commands are performed as FORCE commands. Due to this reason there are no separate
 *  commands FORCE_TRX_OFF (0x03) and TRX_OFF (0x08) - the TRX_OFF (0x08) shall be used in both cases.
 * \note    There are additional restrictions which commands may be issued to the Radio Driver when it persists in
 *  particular states.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.7, table 14.
 */
enum PHY_Cmd_t {
    PHY_CMD__TRX_OFF    = PHY__TRX_OFF,     /*!< Switch/force to TRX_OFF state. */
    PHY_CMD__RX_ON      = PHY__RX_ON,       /*!< Switch/force to RX_ON state. */
    PHY_CMD__TX_ON      = PHY__TX_ON,       /*!< Switch/force to TX_ON state. */
};
SYS_DbgAssertStatic(sizeof(enum PHY_Cmd_t) == 1);
SYS_DbgAssertStatic(PHY_CMD__TRX_OFF == PHY_STATE__TRX_OFF);
SYS_DbgAssertStatic(PHY_CMD__RX_ON == PHY_STATE__RX_ON);
SYS_DbgAssertStatic(PHY_CMD__TX_ON == PHY_STATE__TX_ON);
/**@}*/

/**//**
 * \name    Enumeration of Radio Driver codes of asynchronous task on processing.
 * \details When the Driver is free of task processing it's in the IDLE state. When a new request is received by the
 *  Driver, it starts processing it within the corresponding XXXX_REQ task. As soon as request processing is finished,
 *  the Driver either issues confirmation immediately (on SoC) or schedules the corresponding XXXX_CONF task for issuing
 *  the confirmation in the context of the corresponding Level 1 IRQ (on ML507). When issuing confirmation the Driver
 *  switches to the IDLE state and calls the confirmation handler function provided by the higher-level layer.
 * \note    A new request processing must not be requested until the Driver confirmed the previous request. Driver is
 *  able to receive a request only in the IDLE state. The higher-level layer is responsible for tracking the Driver
 *  state (at least whether its BUSY or IDLE) and must not issue two or more concurrent requests. In debug build attempt
 *  to issue more than one request will halt the software with error.
 * \note    On ML507 platform all requests are processed in the context of one of Level 2 IRQs (either Radio, Timer #1,
 *  or Symbol Timer channel #7). The Driver uses software-triggered IRQs to issue confirmations on accomplished requests
 *  in the context of Level 1 IRQ, because the Level 2 IRQs are used only by this Driver exclusively for its internal
 *  needs. Between scheduling the confirmation and its actual issuing the Driver persists in one of the XXXX_CONF
 *  states. During this period Driver is considered BUSY with respect to ability to start processing of a new request
 *  from the higher-level layer (just because the previous request has not been confirmed yet), but the internal
 *  services of the Driver may perform different behavior while the Driver is in particular XXXX_CONF states if compared
 *  to corresponding XXXX_REQ states just because the Driver is actually free of request processing when it's in a
 *  XXXX_REQ state (for example, a new packet reception may not start in STATE_REQ state, but it may be started in
 *  STATE_CONF state).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.3, table 18.
 */
enum HAL_Radio__TASK_Code_t {
    HAL_RADIO_TASK__IDLE            = 0x00,     /*!< Driver is currently free of request processing. */
    HAL_RADIO_TASK__DATA_REQ        = 0x01,     /*!< Data transmission request is being processed. */
    HAL_RADIO_TASK__STATE_REQ       = 0x02,     /*!< State switching request is being processed. */
    HAL_RADIO_TASK__CCA_REQ         = 0x03,     /*!< CCA detection request is being processed. */
    HAL_RADIO_TASK__ED_REQ          = 0x04,     /*!< Energy detection request is being processed. */
    HAL_RADIO_TASK__CHANNEL_REQ     = 0x05,     /*!< Channel switching request is being processed. */
    HAL_RADIO_TASK__CONF            = 0x80,     /*!< Flag specifying that request has been processed but not confirmed
                                                    yet. This code is used only in conjunction with request codes. */
    HAL_RADIO_TASK__DATA_CONF       = 0x81,     /*!< Data transmission request is to be confirmed. */
    HAL_RADIO_TASK__STATE_CONF      = 0x82,     /*!< State switching request is to be confirmed. */
    HAL_RADIO_TASK__CCA_CONF        = 0x83,     /*!< CCA detection request is to be confirmed. */
    HAL_RADIO_TASK__ED_CONF         = 0x84,     /*!< Energy detection request is to be confirmed. */
    HAL_RADIO_TASK__CHANNEL_CONF    = 0x85,     /*!< Channel switching request is to be confirmed. */
};
SYS_DbgAssertStatic(HAL_RADIO_TASK__DATA_CONF    == (HAL_RADIO_TASK__CONF | HAL_RADIO_TASK__DATA_REQ));
SYS_DbgAssertStatic(HAL_RADIO_TASK__STATE_CONF   == (HAL_RADIO_TASK__CONF | HAL_RADIO_TASK__STATE_REQ));
SYS_DbgAssertStatic(HAL_RADIO_TASK__CCA_CONF     == (HAL_RADIO_TASK__CONF | HAL_RADIO_TASK__CCA_REQ));
SYS_DbgAssertStatic(HAL_RADIO_TASK__ED_CONF      == (HAL_RADIO_TASK__CONF | HAL_RADIO_TASK__ED_REQ));
SYS_DbgAssertStatic(HAL_RADIO_TASK__CHANNEL_CONF == (HAL_RADIO_TASK__CONF | HAL_RADIO_TASK__CHANNEL_REQ));

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    Data types and macro-functions used for Radio channel and channel page representation.
 * \details A total of 32 channel pages are available with channel pages 3 to 31 being reserved for future use. A total
 *  of 27 channels numbered 0 to 26 are available per channel page.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.1.2.1, 6.1.2.2, table 2.
 */
/**@{*/
/**//**
 * \brief   Data type used for packed representation of the couple of channel page and channel identifiers.
 * \details This data type represents particular channel on particular channel page in the following format:
 *  - bits 4..0 - Channel[4..0] - indicate the channel in the range from 0 to 26
 *  - bits 7..5 - Page[2..0] - indicate the channel page in the range from 0 to 7
 */
typedef uint8_t  PHY_PageChannel_t;
SYS_DbgAssertStatic(sizeof(PHY_PageChannel_t) == 1);

/**//**
 * \brief   Data type for the Radio channel page identifier.
 */
typedef uint8_t  PHY_Page_t;
SYS_DbgAssertStatic(sizeof(PHY_Page_t) == 1);

/**//**
 * \brief   Data type for the Radio channel identifier.
 */
typedef uint8_t  PHY_Channel_t;
SYS_DbgAssertStatic(sizeof(PHY_Channel_t) == 1);

/**//**
 * \brief   Data type for Radio channel mask (set, bitmap).
 * \details This data type represents the bitmap of channels available on particular channel page in the following
 *  format:
 *  - bits 26..0 - ChannelMask[26..0] - indicate the status: 1 - channel is available, 0 - channel is unavailable - for
 *      each of up to 27 valid channels (bit #k indicates the status of channel k) supported by a channel page
 *  - bits 31..27 - Page[4..0] - indicate the channel page in the range from 0 to 2
 *
 * \note    The channel mask may be given also without specifying the page in bits 31..27.
 * \note    In the case of a single bit mask, the channel mask may be used for representing the positional code of a
 *  channel. The bit #k (that is the only bit set to one in the bitmap) will indicate the channel k.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.1.2.1, 6.1.2.2, table 2, 23.
 */
typedef uint32_t  PHY_ChannelMask_t;
SYS_DbgAssertStatic(sizeof(PHY_ChannelMask_t) == 4);

/**//**
 * \brief   The maximum actually possible identifier of a Radio channel page.
 * \details A total of 32 channel pages are available with channel pages 3 to 31 being reserved for future use.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.1.2.2, table 2.
 */
#define PHY_PAGE_MAX                (2)

/**//**
 * \brief   The total number of actually defined Radio channel pages.
 * \details A total of 32 channel pages are available with channel pages 3 to 31 being reserved for future use.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.1.2.2, table 2.
 */
#define PHY_PAGES_NUM               (PHY_PAGE_MAX + 1)

/**//**
 * \brief   The maximum possible identifier of a Radio channel.
 * \details A total of 27 channels numbered 0 to 26 are available per channel page.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.1.2.1, table 2.
 */
#define PHY_CHANNEL_MAX             (26)

/**//**
 * \brief   The maximum possible number of channels on a single channel page.
 * \details A total of 27 channels numbered 0 to 26 are available per channel page.
 * \note    This constant may also be used as the invalid channel identifier.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.1.2.1, table 2.
 */
#define PHY_CHANNELS_NUM            (PHY_CHANNEL_MAX + 1)

/**//**
 * \brief   The complete mask of channels on a single channel page.
 * \details A total of 27 channels numbered 0 to 26 are available per channel page.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.1.2.1, table 2.
 */
#define PHY_LEGAL_CHANNELS          BIT_MASK(PHY_CHANNELS_NUM)

/**//**
 * \brief   Composes packed page-channel identifier from distinct channel page and channel identifiers.
 * \param[in]   page        The page identifier, in the range from 0 to 7.
 * \param[in]   channel     The channel identifier, in the range from 0 to 26.
 * \return  The packed page-channel identifier.
 * \details The packed page-channel identifier bits 4..0 indicate the channel, bits 7..5 indicate the page.
 */
#define PHY__Make_PageChannel(page, channel)\
        ((((PHY_PageChannel_t)(page)) << 5) | (((PHY_PageChannel_t)(channel)) << 0))

/**//**
 * \brief   Extracts the channel page identifier from the packed page-channel identifier.
 * \param[in]   pgch    The packed page-channel identifier.
 * \return  The extracted page identifier.
 * \details The packed page-channel identifier bits 4..0 indicate the channel, bits 7..5 indicate the page.
 */
#define PHY__Take_Page(pgch)        ((PHY_Page_t)(((PHY_PageChannel_t)(pgch)) >> 5))

/**//**
 * \brief   Extracts the Radio channel identifier from the packed page-channel identifier.
 * \param[in]   pgch    The packed page-channel identifier.
 * \return  The extracted channel identifier.
 * \details The packed page-channel identifier bits 4..0 indicate the channel, bits 7..5 indicate the page.
 */
#define PHY__Take_Channel(pgch)     ((PHY_Page_t)(((PHY_PageChannel_t)(pgch)) & 0x1F))

/**//**
 * \brief   Substitutes the page identifier with the new one in the given packed page-channel identifier.
 * \param[in/out]   pgch        Reference to the packed page-channel identifier to be updated.
 * \param[in]       page        The new page identifier, in the range from 0 to 7.
 * \details The channel identifier is preserved, only the channel page identifier is updated.
 */
#define PHY__Update_Page(pgch, page)\
        do { (pgch) = ((((PHY_PageChannel_t)(page)) << 5) | (((PHY_PageChannel_t)(pgch)) & 0x1F)); } while(0)

/**//**
 * \brief   Substitutes the channel identifier with the new one in the given packed page-channel identifier.
 * \param[in/out]   pgch        Reference to the packed page-channel identifier to be updated.
 * \param[in]       channel     The new channel identifier, in the range from 0 to 26.
 * \details The channel page identifier is preserved, only the channel identifier is updated.
 */
#define PHY__Update_Channel(pgch, channel)\
        do { (pgch) = ((((PHY_PageChannel_t)(pgch)) & 0xE0) | (((PHY_PageChannel_t)(channel)) << 0)); } while(0)
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    Data types and macro-functions used for estimation of the Radio link quality and media conditions.
 */
/**@{*/
/**//**
 * \brief   Data type for Energy Detection (ED) measurement.
 * \details The ED result is an integer ranging from 0x00 to 0xFF. The minimum ED value (0x00) indicates received power
 *  less than 10 dB above the specified receiver sensitivity, and the range of received power spanned by the ED values
 *  is at least 40 dB. Within this range, the mapping from the received power in decibels to ED value is linear with an
 *  accuracy of ± 6 dB.
 * \details Under the specific test conditions, a compliant Radio hardware is capable of achieving a sensitivity of:
 *  - minus 85 dBm or better, for the case of 2450 MHz band with O-QPSK modulation (channel page 0, channels 11~26)
 *  - minus 92 dBm or better, for the case of 868/915 MHz band with BPSK modulation (channel page 0, channels 0~10)
 *  - minus 85 dBm or better, for the case of 868/915 MHz band with ASK modulation (channel page 1, channels 0~10)
 *  - minus 85 dBm or better, for the case of 868/915 MHz band with O-QPSK modulation (channel page 2, channels 0~10)
 *
 * \details Expression used for conversion of raw ED values to scale expressed in dBm is established with public
 *  constants specific to particular Radio hardware.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.2.4, 6.5.3.3, 6.6.3.4, 6.7.3.4, 6.8.3.4, 6.9.7, table 11.
 */
typedef uint8_t  PHY_ED_t;
SYS_DbgAssertStatic(sizeof(PHY_ED_t) == 1);

/**//**
 * \brief   Data type for Received Signal Strength Indicator (RSSI).
 * \details The RSSI result is an integer ranging from 0x00 to 0xFF. The minimum RSSI value is 0x00.
 * \details The RSSI is not covered by the Standard.
 * \details Expression used for conversion of raw RSSI values to scale expressed in dBm is established with public
 *  constants specific to particular Radio hardware.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.9.7.
 */
typedef uint8_t  PHY_RSSI_t;
SYS_DbgAssertStatic(sizeof(PHY_RSSI_t) == 1);

/**//**
 * \brief   Data type for Link Quality Indicator (LQI).
 * \details The LQI is an integer ranging from 0x00 to 0xFF. The minimum and maximum LQI values (0x00 and 0xFF) are
 *  associated with the lowest and highest quality compliant signals detectable by the receiver, and LQI values in
 *  between are uniformly distributed between these two limits. At least eight unique values of LQI are used.
 * \details The SoC implements LQI using receiver Energy Detection (ED). The ML507 with AT86RF Radio implements LQI
 *  using signal-to-noise ratio estimation. For the case of ML507 with AT86RF Radio the Radio Driver reports also the
 *  Received Signal Strength Indicator (RSSI) that is measured using receiver ED when issuing the PD-DATA.indication to
 *  the higher-level layer.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.1.3, 6.9.8, table 8.
 */
typedef uint8_t  PHY_LQI_t;
SYS_DbgAssertStatic(sizeof(PHY_LQI_t) == 1);

/**//**
 * \brief   The maximum possible value of Energy Detection (ED) expressed in the raw format.
 * \details This value is intended to be used as the invalid or unknown value of ED.
 * \details The ED result is an integer ranging from 0x00 to 0xFF.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.4, 6.9.7, table 11.
 */
#define PHY_ED_MAX      (0xFFu)

/**//**
 * \brief   Converts the 8-bit raw Energy Detection (ED) value into the dBm scale.
 * \param[in]   ed      The raw ED value to be converted to dBm scale.
 * \return  The ED value, 8-bit signed integer, in dBm.
 * \details Coefficients used in the expression are defined in the platform specific Radio Driver header file.
 */
#define PHY__ED_to_dBm(ed)\
        (((int8_t)(((uint16_t)(ed)) * ((int16_t)(PHY_SCALE__ED_1_to_dBm)))) + ((int8_t)(PHY_SCALE__ED_0_to_dBm)))

/**//**
 * \brief   Converts the 8-bit raw Received Signal Strength Indicator (RSSI) value into the dBm scale.
 * \param[in]   rssi    The raw RSSI value to be converted to dBm scale.
 * \return  The RSSI value, 8-bit signed integer, in dBm.
 * \details Coefficients used in the expression are defined in the platform specific Radio Driver header file.
 */
#define PHY__RSSI_to_dBm(rssi)\
        (((int8_t)(((uint16_t)(rssi)) * ((int16_t)(PHY_SCALE__RSSI_1_to_dBm)))) + ((int8_t)(PHY_SCALE__RSSI_0_to_dBm)))
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    Data types used for the transceiver configuration.
 */
/**@{*/
/**//**
 * \brief   Enumeration of CCA Modes.
 * \note    Mode 3 is defined as 3-AND in compliance with IEEE 802.15.4-2003 standard where it is defined as "Carrier
 *  sense with energy above threshold". Mode 0 is defined for 3-OR introduced in IEEE 802.15.4-2006.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.4.2, 6.9.9, table 23.
 */
enum PHY_CCAMode_t {
    PHY_CCA__MODE_3_OR      = 0,    /*!< Mode 3-OR, carrier sense OR energy above threshold. */
    PHY_CCA__MODE_1         = 1,    /*!< Mode 1, energy above threshold. */
    PHY_CCA__MODE_2         = 2,    /*!< Mode 2, carrier sense only. */
    PHY_CCA__MODE_3_AND     = 3,    /*!< Mode 3-AND, carrier sense AND energy above threshold. */
};
SYS_DbgAssertStatic(sizeof(enum PHY_CCAMode_t) == 1);

/**//**
 * \brief   Data type for the PHY nominal transmit power.
 * \details This type represents a signed 8-bit integer in twos-complement format, corresponding to the nominal transmit
 *  power of the device in decibels relative to 1 mW. Valid range is from -32 dBm to +31 dBm. Values outside this range
 *  are limited at the corresponding boundary.
 * \note    When a value of this data type is used separately the whole 8-bit width must be treated as a signed 8-bit
 *  integer - i.e., for negative values bits 6 and 7 must be set to one (in general case they must extend the binary
 *  value of the bit 5). Hence, this data type is also used in reduced signed 6-bit integer form when included into the
 *  phyTransmitPower attribute. In this case, bits 7..6 are delegated for different purposes and must not be treated as
 *  a part of the signed 8-bit integer value representation - consequently, the binary value of the bit 5 must be
 *  extended to bits 6 and 7 when converting from the 6-bit form to the 8-bit form.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.4.2, table 23.
 */
typedef int8_t  PHY_TXPower_t;
SYS_DbgAssertStatic(sizeof(PHY_TXPower_t) == 1);
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    PHY constants.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.3, 6.4.1, 6.9.1, 6.9.2, 7.2.1, 7.2.2.3, tables 21, 22.
 */
/**@{*/
#define PHY_aMaxPHYPacketSize   (127)       /*!< The maximum PSDU size (in octets) the PHY shall be able to receive. */
#define PHY_aTurnaroundTime     (12)        /*!< RX-to-TX or TX-to-RX maximum turnaround time (in symbol periods). */
#define PHY_aAckMPDUOverhead    (5)         /*!< The number of octets added by the MAC to the PSDU for the ACK frame. */
#define PHY_aFCSSize            (2)         /*!< The number of octets added by the MAC to the PSDU for the FCS field. */
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    Data types for the PHY packet format.
 */
/**@{*/
/**//**
 * \brief   Data type for single Octet (8-bit byte).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.3, table 16.
 */
typedef uint8_t  Octet_t;
SYS_DbgAssertStatic(sizeof(Octet_t) == 1);

/**//**
 * \brief   Data type for PHY packet (PHY protocol data unit, PPDU).
 * \note    Indeed, PPDU includes not only PHY header (PHR) and PHY payload (PSDU), but the synchronization header (SHR)
 *  also. This data type represents only PHR plus PSDU, excluding the SHR. The SHR is generated and treated by the Radio
 *  hardware automatically.
 * \note    Generally PPDU has variable length. This data type represents PPDU having the maximum length: 1 octet of PHR
 *  plus up to 127 octets of PSDU (again excluding the SHR).
 * \details The PPDU array has 4-bytes alignment in order to be compatible with 32-bit access necessary for SoC Radio.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.3, figure 16.
 */
typedef Octet_t  PPDU_t[1 /*PHR*/ + PHY_aMaxPHYPacketSize] __attribute__((aligned(4)));
SYS_DbgAssertStatic(sizeof(PPDU_t) == 128);
SYS_DbgAssertStatic(__alignof__(PPDU_t) % 4 == 0);

/**//**
 * \brief   Data type for PHY header (PHR) of a packet.
 * \details The PHR includes the following fields:
 *  - bits 6..0 - FrameLength[6..0] - specifies the total number of octets contained in the PSDU.
 *  - bit 7 - Reserved[0] - must be set to zero on transmission and ignored on reception.
 *
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.3, figure 16.
 */
typedef Octet_t  PHR_t;
SYS_DbgAssertStatic(sizeof(PHR_t) == 1);

/**//**
 * \brief   Data type for the Frame Length field of the PHY header (PHR).
 * \details The Frame Length field is 7 bits in length and specifies the total number of octets contained in the PSDU
 *  (i.e., PHY payload). It is a value between 0 and aMaxPHYPacketSize (127).
 * \details For the MAC mode of operation of PHY the Frame Length value must belong to one of allowed ranges:
 *  - 0 to 4 - reserved. Must not be used for transmission. Rejected on reception.
 *  - 5 - PSDU is expected to contain MSDU of an Acknowledgment frame.
 *  - 6 to 8 - reserved. Must not be used for transmission. Rejected on reception.
 *  - 9 to aMaxPHYPacketSize (127) - PSDU is expected to contain MSDU of a Beacon, Data, or Command frame (i.e., except
 *      the Acknowledgment frame).
 *
 * \details For the pure PHY mode of operation of PHY the Frame Length value is allowed to have arbitrary value in the
 *  range from 0 to aMaxPHYPacketSize (127).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.3, 6.3.3, table 21.
 */
typedef uint8_t  PHY_FrameLen_t;
SYS_DbgAssertStatic(sizeof(PHY_FrameLen_t) == 1);

/**//**
 * \brief   Data type for PHY payload (PHY service data unit, PSDU) of a packet.
 * \note    Generally PSDU has variable length. This data type represents PSDU having the maximum length.
 * \details The PSDU array has 4-bytes alignment in order to be compatible with 32-bit access necessary for SoC Radio.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.3, 6.3.4, figure 16.
 */
typedef Octet_t  PSDU_t[PHY_aMaxPHYPacketSize] __attribute__((aligned(4)));
SYS_DbgAssertStatic(sizeof(PSDU_t) == 127);
SYS_DbgAssertStatic(__alignof__(PSDU_t) % 4 == 0);
/**@}*/

/************************* PROTOTYPES *********************************************************************************/
/**//**
 * \brief   The code of the asynchronous request on processing by the Radio Driver.
 * \details This variable holds the code of the request being currently processed. It serves for canceling of CCA and ED
 *  requests and synchronously finishing State and Channel switching requests started during the RX_ON state on incoming
 *  RX_START event from the Radio (i.e., when the Radio entered the BUSY_RX state directly in the middle of request).
 * \details This variable is provided also for debug purposes. It restricts the Radio Driver of be engaged by two (or
 *  more) requests simultaneously.
 * \details Initially this variable is set with IDLE code which means that the Driver is free of request processing.
 */
extern enum HAL_Radio__TASK_Code_t  HAL_Radio__TASK;

/**//**
 * \name    Radio Driver TX and RX frame buffers.
 * \par     Documentation
 *  See Broadcom ZIGBEE MAC & HIF HARDWARE ON SOC 8/30/2013 (UPDATED ON 7/24/2014 FOR ACTUAL SOC IMPLEMENTATION),
 *  subclauses TX-BUFFER (1), RX-BUFFER (1).
 */
/**@{*/
/**//**
 * \brief   TX frame buffer for beacon, data, or command frame - PHR.
 * \details PSDU is stored in a separate array.
 * \par     Documentation
 *  See Broadcom ZIGBEE MAC & HIF HARDWARE ON SOC 8/30/2013 (UPDATED ON 7/24/2014 FOR ACTUAL SOC IMPLEMENTATION),
 *  subclause TX-BUFFER (1).
 */
extern PHR_t  PHY_FrmBuf__TX_BDC_PHR;
SYS_DbgAssertStatic(sizeof(PHY_FrmBuf__TX_BDC_PHR) == 1);

/**//**
 * \brief   TX frame buffer for beacon, data, or command frame - PSDU.
 * \note    Generally PSDU has variable length. This variable is able to involve a PSDU having the maximum length.
 * \details The PSDU array has 4-bytes alignment in order to be compatible with 32-bit access necessary for SoC Radio.
 * \note    PHR is stored in a separate variable.
 * \par     Documentation
 *  See Broadcom ZIGBEE MAC & HIF HARDWARE ON SOC 8/30/2013 (UPDATED ON 7/24/2014 FOR ACTUAL SOC IMPLEMENTATION),
 *  subclause TX-BUFFER (1).
 */
extern PSDU_t  PHY_FrmBuf__TX_BDC_PSDU;                                                                                     // IDEA: [MAC Security] Use dynamic memory and stack to transfer the frame. The caller is responsible for freeing memory allocated for the PSDU.
SYS_DbgAssertStatic(sizeof(PHY_FrmBuf__TX_BDC_PSDU) == 127);                                                                //  ... It's allowed to dismiss the PSDU as soon as this function [PHY__DATA_req] returned - i.e., prior to the TX_END event occurred.
SYS_DbgAssertStatic(__alignof__(PHY_FrmBuf__TX_BDC_PSDU) % 4 == 0);                                                         //  ... The PSDU content is pushed into the TX frame buffer by this function completely prior to return.

/**//**
 * \brief   TX frame buffer for ACK frame PSDU.
 * \details The ACK frame PSDU involves exactly 5 octets.
 * \details The PSDU array has 4-bytes alignment in order to be compatible with 32-bit access necessary for SoC.
 * \note    It's not necessary to keep the ACK frame PHR, because the FrameLength field constantly equals 5.
 * \note    The timed ACK frame transmission is a part of the MAC operation mode. Due to this reason the MFR.FCS field
 *  is not saved - its value is calculated automatically during transmission.
 * \par     Documentation
 *  See Broadcom ZIGBEE MAC & HIF HARDWARE ON SOC 8/30/2013 (UPDATED ON 7/24/2014 FOR ACTUAL SOC IMPLEMENTATION),
 *  subclause TX-BUFFER (1).
 */
extern Octet_t  PHY_FrmBuf__TX_ACK_PSDU[PHY_aAckMPDUOverhead - PHY_aFCSSize]  __attribute__((aligned(4)));                  // IDEA: Compose ACK frame dynamically. It's enough to keep only DSN and FP fields. FCS may be saved into the LUT.
SYS_DbgAssertStatic(sizeof(PHY_FrmBuf__TX_ACK_PSDU) == 3);
SYS_DbgAssertStatic(__alignof__(PHY_FrmBuf__TX_ACK_PSDU) % 4 == 0);

/**//**
 * \brief   RX frame buffer.
 * \note    Indeed, PPDU includes not only PHY header (PHR) and PHY payload (PSDU), but the synchronization header (SHR)
 *  also. This data type represents only PHR plus PSDU, excluding the SHR. The SHR is generated and treated by the Radio
 *  hardware automatically.
 * \note    Generally PPDU has variable length. This variable is able to involve a PPDU having the maximum length:
 *  1 octet of PHR plus up to 127 octets of PSDU (again excluding the SHR).
 * \details The PPDU array has 4-bytes alignment in order to be compatible with 32-bit access necessary for SoC Radio.
 * \par     Documentation
 *  See Broadcom ZIGBEE MAC & HIF HARDWARE ON SOC 8/30/2013 (UPDATED ON 7/24/2014 FOR ACTUAL SOC IMPLEMENTATION),
 *  subclauses RX-BUFFER (1).
 */
extern union PHY_FrmBuf__RX_PPDU_t {
    PPDU_t          ppdu;                           /*!< PPDU of the received packet. */
    struct {
        Octet_t     phr;                            /*!< PHR of the received packet. */
        Octet_t     psdu[PHY_aMaxPHYPacketSize];    /*!< PSDU of the received packet. */
    };
} PHY_FrmBuf__RX_PPDU;
SYS_DbgAssertStatic(sizeof(PHY_FrmBuf__RX_PPDU) == 128);
SYS_DbgAssertStatic(__alignof__(PHY_FrmBuf__RX_PPDU) % 4 == 0);
SYS_DbgAssertStatic(offsetof(union PHY_FrmBuf__RX_PPDU_t, psdu) == 1);
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    Radio Driver auxiliary data after packets transmission/reception and requests execution.
 */
/**@{*/
/**//**
 * \brief   Combined variable for storing different parameters of the last received packet.
 * \details Data is stored in the following compact form:
 *  - LQI[7..0] or CGT[31..0] - LQI or CGT value of the last received packet
 *  - RSSI[7..0] or ED[31..0] - RSSI or ED value of the last received packet
 *  - PageChannel[7..0] - the page-channel identifier indicating page and channel on which the last packet was received
 *  - FCSValid[0] - the FCS validation status of the last received MAC frame
 *
 * \note    LQI/CGT and RSSI/ED values are published in their hardware-specific format. Conversion functions defined in
 *  this interface must be used to express results in the hardware-independent format. Channel page and channel are
 *  published in the packed format of page-channel identifier. The FCS field is validated only in the MAC mode of
 *  operation.
 */
extern struct PHY_FrmBuf__RX_Stuff_t {
    union {
        PHY_LQI_t           lqi;            /*!< LQI value, hardware-specific units. */
        uint32_t            cgt;            /*!< CGT value, hardware-specific units. */
    };
    union {
        PHY_RSSI_t          rssi;           /*!< RSSI value, hardware-specific units. */
        uint32_t            ed;             /*!< ED value, hardware-specific units. */
    };
    PHY_PageChannel_t       pgch;           /*!< Page-channel packed identifier. */
    Bool8_t                 fcsValid;       /*!< TRUE if FCS of the received MAC frame is valid. */
    uint16_t                reserved;       /*!< Reserved. */
} PHY_FrmBuf__RX_Stuff;
SYS_DbgAssertStatic(sizeof(PHY_FrmBuf__RX_Stuff) == 12);

/**//**
 * \brief   Timestamps of the last transmitted and the last received packets.
 * \details All timestamps reported by this Driver are expressed in symbol fractions according to configuration of the
 *  Symbol Timer unit.
 * \note    Timestamps are unsigned 32-bit integers. If necessary to publish timestamps expressed in symbols (not symbol
 *  fractions) and in 24-bit format, one shall use timestamp conversion functions defined by the Symbol Timer unit and
 *  ignore the remaining higher-order bits in binary representation of a timestamp value.
 * \details Start and end timestamps reported by the Driver for transmitted and received frames are bound to PHY packet
 *  in the hardware-independent way, no additional normalization needed. The packet start synchronization point is the
 *  onset of the first symbol past the SFD, namely, the first symbol of the FrameLength field of PHR. The packet end
 *  synchronization point is the cutoff of the last symbol of PSDU.
 * \note    The macSyncSymbolOffset attribute is assumed to be equal to zero.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.4.2, table 86.
 */
extern struct PHY_FrmBuf__Tstamps_t {
    HAL_Symbol__Tstamp_t    start;      /*!< Start timestamp, in symbol fractions. */
    HAL_Symbol__Tstamp_t    end;        /*!< End timestamp, in symbol fractions. */
} PHY_FrmBuf__TX_Tstamps, PHY_FrmBuf__RX_Tstamps;
SYS_DbgAssertStatic(sizeof(PHY_FrmBuf__TX_Tstamps) == 8);
SYS_DbgAssertStatic(sizeof(PHY_FrmBuf__RX_Tstamps) == 8);

/**//**
 * \brief   Variable for saving status/results of the last confirmed request to the Driver.
 * \details The following data are saved depending on the type of request:
 *  - status of the Clear Channel Assessment (CCA) detection request: IDLE or BUSY
 *  - result of the Energy Detection (ED) measurement request: energy level
 *
 * \details The CCA status is published in boolean format: TRUE stays for IDLE, FALSE for BUSY.
 * \details The ED value is published in its hardware-specific format. Conversion function defined in this interface
 *  must be used to express the result in the hardware-independent format (in dBm).
 * \note    Driver prohibits activation of two or more concurrent requests. Due to this reason results published after
 *  different requests may be stored in the shared memory space (in a union).
 */
extern union PHY_FrmBuf__Status_t {
    Bool8_t     ccaIdle;    /*!< The saved CCA status for returning in the postponed confirmation. This variable stores
                                status of the last performed CCA detection. It is assigned with TRUE for the IDLE status
                                or FALSE for the BUSY status. */
    PHY_ED_t    edLevel;    /*!< The saved Energy level value for returning in the postponed confirmation, expressed in
                                hardware-specific units, from 0x00 to 0xFF. This variable stores result of the last
                                performed energy detection measurement. */
} PHY_FrmBuf__Status;
SYS_DbgAssertStatic(sizeof(PHY_FrmBuf__Status) == 1);
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Initializes the Radio Driver.
 * \details This function cancels all processes on the Radio hardware, resets its state to the default configuration,
 *  then performs the Radio hardware initialization, and finally switches the transceiver state to the TRX_OFF state.
 * \details This function must be called once on the application startup.
 * \note    On the SoC BCM7366 platform this function must be called with all interrupts enabled. On the ML507 with
 *  external AT86RF platform this function must be called with all interrupts disabled.
 * \note    This function does not enable ARC interrupts. Interrupts must be enabled by the application startup routine
 *  after all the necessary software and hardware are configured either prior or after this function is called depending
 *  on the platform (see above).
 */
void PHY__Init(void);

/**//**
 * \brief   Initiates immediate packet transmission.
 * \details This function has implicit parameters which must be assigned by the caller prior to call it:
 *  - PHY_FrmBuf__TX_BDC_PHR    - value of the PHR of the transmitted packet. Allowed values for the FrameLength[6..0]
 *      field (bits 6..0 of the PHR) are from 0 to 127 in the pure PHY mode of operation, and either 5 or from 9 to 127
 *      in the MAC mode. The Reserved[0] field (bit 7 of the PHR) must be set to zero.
 *  - PHY_FrmBuf__TX_BDC_PSDU   - array of bytes with PSDU of the transmitted packet. Must contain FrameLength bytes in
 *      the pure PHY mode of operation, or at least (FrameLength - 2) bytes in the MAC mode (the trailing two bytes
 *      holding the MFR.FCS field may be omitted in this mode).
 *
 * \details The Driver must persist in the TX_ON state at the moment when this function is called. This function
 *  switches the Driver into the BUSY_TX state (or one of the BUSY_TX substates), pushes the given PHR and PSDU into the
 *  Radio hardware TX frame buffer and instigates the Radio hardware to start transmitting the packet immediately. After
 *  that Driver starts waiting for the TX_DONE event from the Radio hardware. As soon as this event occurred (i.e., when
 *  the packet transmission over the air is actually finished), the Driver is switched back to the TX_ON state and the
 *  confirmation callback is called by the Driver.
 * \details The PSDU to be transmitted must be assembled by the caller prior to call this function. The packet payload
 *  length (the FrameLength field of the PHR) is specified with the \p PHY_FrmBuf__TX_BDC_PHR bits 6..0. The content of
 *  the \p PHY_FrmBuf__TX_BDC_PSDU containing the PSDU is left unchanged after the packet is transmitted and may be used
 *  again to retry the packet if necessary.
 * \details In the MAC mode of operation the FCS field of the MFR is calculated automatically over the specified PSDU.
 *  The last two bytes of the PSDU are ignored, even if they are assigned by the caller, and substituted with the
 *  automatically calculated FCS value for transmission. Otherwise the whole PSDU is transmitted as-is. In any case, the
 *  FrameLength field of the PHR must specify the PSDU total length including the FCS field. The (re-)calculated FCS
 *  value is not saved in the \p PHY_FrmBuf__TX_BDC_PSDU.
 * \details It is not allowed to issue new request to the Radio Driver, either of the same or different type, until the
 *  previous request is confirmed.
 * \details This function disables interrupts of Level 1 and 2 for the period of its execution and restores the status
 *  on return. This function must normally be called by the higher-level layer from the Level 1 interrupt execution
 *  context just because all confirmation and indication callback handlers provided by the higher-level layer are called
 *  by the Driver also in the context of a Level 1 interrupt. Hence this function may be called from the main thread as
 *  well if necessary. It is allowed to issue a new request to the Driver directly in the context of any confirmation or
 *  indication handler called by the Driver.
 * \details Timed transmission is not supported by this function.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.1.1, table 6.
 */
void PHY__DATA_req(void);

/**//**
 * \brief   Initiates timed transmission of the MAC Acknowledgment (ACK) frame.
 * \details This function has implicit parameter which must be assigned by the caller prior to call it:
 *  - PHY_FrmBuf__TX_ACK_PSDU   - array of bytes with PSDU of the transmitted ACK frame.
 *
 * \details This function is a version of the Data request processor dedicated for ACK frame transmission. Mostly it
 *  behaves as the general version of the Data transmission request, but this one has the following differences:
 *  - This function commences transmission of the ACK frame by the Symbol Timer exactly in 12.375 symbols starting from
 *      the RX end timestamp of the last received MAC frame.
 *  - The PSDU of the ACK frame is provided in the dedicated Radio Driver ACK frame buffer. It allows to acknowledge
 *      incoming frames (i.e., to transmit ACK frames) received between attempts to transmit the same outgoing frame
 *      that is stored in separate beacon-data-command (BDC) frame buffer. There is no need to recover the BDC frame
 *      buffer after the ACK frame transmission when resuming the interrupted transmission of a frame.
 *  - The PHR and its FrameLength field are not specified with parameters, because ACK frames have constant size equal
 *      to 5 octets.
 *
 * \note    The ACK frame is transmitted namely in 12.375 symbols instead of 12.0 in order to comply with the RX-to-TX
 *  turnaround certification test which states that the ACK frame transmission must be started in (12.0..12.75) symbols
 *  after the acknowledged frame reception. Here 12.375 is the middle of the 12.0..12.75 range.
 * \note    This function belongs to the MAC mode operation. There is no need to evaluate the MFR.FCS field of the PSDU
 *  and provide it in the ACK frame buffer. It will be evaluated automatically by this function.
 * \details The Driver must persist in the TX_ON state at the moment when this function is called. Exact behavior of
 *  this function depends on implementation. In general it switches the Driver into the BUSY_TX state (or one of the
 *  BUSY_TX substates), pushes the given ACK frame PSDU into the Radio hardware TX frame buffer, assigns the FrameLength
 *  with 5 (the ACK frame PSDU size) and engages the Radio Driver software or hardware to commence transmission at the
 *  evaluated timestamp. Particular implementations of this function may differ in the order of these steps. All the
 *  remaining activities are performed in the same way as in the general case of Data request.
 * \note    If for some reason the remaining time to the specified timestamp is too short to perform postponed operation
 *  with the Symbol Timer (or if the specified timestamp has already passed), transmission is started immediately.
 * \details The ACK frame PSDU to be transmitted must be assembled by the caller prior to call this function. The packet
 *  payload length (the FrameLength field of the PHR) is considered to be equal to 5 octets. The FCS field of the MFR is
 *  calculated automatically over the specified PSDU. There is no need for the caller to provide the FCS value in the
 *  ACK frame PSDU.
 * \details It is not allowed to issue new request to the Radio Driver, either of the same or different type, until the
 *  previous request is confirmed.
 * \details This function disables interrupts of Level 1 and 2 for the period of its execution and restores the status
 *  on return. This function must normally be called by the higher-level layer from the Level 1 interrupt execution
 *  context just because all confirmation and indication callback handlers provided by the higher-level layer are called
 *  by the Driver also in the context of a Level 1 interrupt. Hence this function may be called from the main thread as
 *  well if necessary. It is allowed to issue a new request to the Driver directly in the context of any confirmation or
 *  indication handler called by the Driver.
 * \details The ACK frame transmission request is confirmed by the same callback function with the general case Data
 *  request.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.1.1, table 6.
 */
void PHY__DATA_ACK_req();

/**//**
 * \brief   Handles confirmation of a packet transmission.
 * \details This function has implicit parameter which is assigned by the Radio Driver prior to call it:
 *  - PHY_FrmBuf__TX_Tstamps    - start and end timestamps synchronized respectively on the onset of the PHR and on the
 *      cutoff of the PPDU (PSDU) of the transmitted packet, in symbol fractions.
 *
 * \details This function must be provided by the higher-level layer. It will be called by the Driver when Radio reports
 *  TX_DONE event after transmitting a packet over the air. This function is called by the Driver in the context of a
 *  Level 1 interrupt.
 * \details It is allowed to issue a new request to the Driver directly in the context of this function execution. The
 *  Driver persists in the TX_ON state at the moment of this function call and will keep staying in this state until a
 *  new request is issued by the higher-level layer.
 * \details The content of the Driver TX frame buffer is kept unchanged after the packet is transmitted and may be used
 *  again to retry the packet.
 * \details There is no status of the performed transmission attempt. If this function is called by the Driver, it means
 *  that the confirmed transmission was performed successfully. There is no a common case failure that may be confirmed
 *  here.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.1.2, table 7.<\br>
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 7.1.3, figure 7-2.<\br>
 *  See Broadcom ZIGBEE MAC & HIF HARDWARE ON SOC 8/30/2013 (UPDATED ON 7/24/2014 FOR ACTUAL SOC IMPLEMENTATION),
 *  subclause INTERRUPTS INTO ZIGBEE ARC CPU, TX-BUFFER (4).
 */
extern void PHY__DATA_conf(void);

/**//**
 * \brief   Handles indication of a packet reception.
 * \details This function has implicit parameters which are assigned by the Radio Driver prior to call it:
 *  - PHY_FrmBuf__RX_PPDU       - array of bytes with PPDU of the received packet. Contains PHR at PPDU[0] and from 0 to
 *      127 octets of PSDU starting from PPDU[1]. Value of the FrameLength field of PHR is from 0 to 127.
 *  - PHY_FrmBuf__RX_Stuff      - structure containing LQI/CGT and RSSI/ED values of the received packet (given in the
 *      hardware-specific format), current channel page and channel, FCS validation status.
 *  - PHY_FrmBuf__RX_Tstamps    - start and end timestamps synchronized respectively on the onset of the PHR and on the
 *      cutoff of the PPDU (PSDU) of the received packet, in symbol fractions.
 *
 * \details This function must be provided by the higher-level layer. It will be called by the Driver when it reports
 *  the RX_DONE event after receiving a packet over the air. This function is called by the Driver in the context of a
 *  Level 1 interrupt.
 * \details The received PPDU is put by the Driver into the statically allocated RX frame buffer and published to the
 *  higher-level layer with this function. The RX frame buffer is switched into the locked state by the Driver
 *  automatically on assignment with a new packet, and it stays locked until this function returned. The Driver unlocks
 *  the buffer for new receptions automatically as soon as this function returned. During this period if a new packet
 *  is sensed, it is rejected. After this function returned, the RX frame buffer may be rewritten by the Driver with a
 *  newly received packet - so, the higher-level layer has to copy the packet published with this function prior to
 *  return from this function if the packet must be processed later in a postponed task.
 * \details It is allowed to issue a new request to the Driver directly in the context of this function execution. The
 *  Driver persists in the RX_ON state at the moment of this function call and will keep staying in this state until a
 *  new request is issued by the higher-level layer.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.1.3, 6.9.7, 6.9.8, table 8.<\br>
 *  See Atmel 8111C-MCU Wireless-09/09, subclauses 7.1.3, 8.3, 8.6, figure 7-2.<\br>
 *  See Broadcom ZIGBEE MAC & HIF HARDWARE ON SOC 8/30/2013 (UPDATED ON 7/24/2014 FOR ACTUAL SOC IMPLEMENTATION),
 *  subclause INTERRUPTS INTO ZIGBEE ARC CPU, RX-BUFFER (3), LINK QUALITY INDICATOR (LQI).
 */
extern void PHY__DATA_ind(void);

/**//**
 * \brief   Initiates the Radio state switching.
 * \param[in]   cmd     Command to switch the state. One of the following values: TRX_OFF, RX_ON, TX_ON.
 * \details This function performs the radio state switching almost in conformity with the IEEE Std. 802.15.4-2006/2003.
 *  The mentioned standards have significant differences in how the state switching is performed from states BUSY_RX and
 *  BUSY_TX. Due to this reason, this function prohibits particular combinations of input and output states,
 *  nevertheless preserving conventional behavior for other combinations. Also the command FORCE_TRX_OFF is not fully
 *  implemented, meaning that it does not reset the PHY (particularly, when it's in the TX_ON or BUSY_TX state). All
 *  changes to standards are described below.
 * \details Table of implemented state switchings. Input states are listed on the left, commands are listed on the top,
 *  output states are given in cells:<\br>
 *  |           | TRX_OFF               | RX_ON                 | TX_ON             |
 *  | TRX_OFF   | TRX_OFF               | RX_ON                 | not allowed       |
 *  | RX_ON     | TRX_OFF               | RX_ON                 | TX_ON             |
 *  | BUSY_RX   | TRX_OFF, abort RX     | BUSY_RX, continue RX  | TX_ON, abort RX   |
 *  | TX_ON     | not allowed           | RX_ON                 | not allowed       |
 *  | BUSY_TX   | not allowed           | not allowed           | not allowed       |
 *
 * \details Switching between TRX_OFF and RX_ON states, and switching between RX_ON and TX_ON states may be performed
 *  directly. Switching between TRX_OFF and TX_ON states must be performed through the RX_ON state only. The BUSY_RX
 *  state is treated as the RX_ON state when a state switching is requested, and the packet reception is canceled
 *  immediately both by TRX_OFF and TX_ON commands (they may be considered as the FORCE_XXXX commands with respect to
 *  the BUSY_RX state, but not the BUSY_TX state). When the Driver is in the BUSY_TX state it must not be requested to
 *  change its state (postponed state switching is not supported), one shall wait for the Driver to exit from BUSY_TX to
 *  TX_ON and then request the state switching. When the Driver is in the TX_ON or BUSY_TX state it must not be
 *  requested to reenter the TX_ON state; hence it's allowed to issue the TRX_OFF command when the Driver is already in
 *  the TRX_OFF state, and to issue the RX_ON when driver is either in the RX_ON or BUSY_RX state (in the latter case
 *  packet reception is continued) - in both cases the Driver preserves its current state.
 * \details It is not allowed to issue new request to the radio, either of the same or different type, until the
 *  previous request is confirmed.
 * \details This function disables interrupts of Level 1 and 2 for the period of its execution and restores the status
 *  on return. This function must normally be called by the higher-level layer from the Level 1 interrupt execution
 *  context just because all confirmation and indication callback handlers provided by the higher-level layer are called
 *  by the Driver also in the context of a Level 1 interrupt. Hence this function may be called from the main thread as
 *  well if necessary. It is allowed to issue a new request to the Driver directly in the context of any confirmation or
 *  indication handler called by the Driver.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.2.7, 6.2.3, tables 14, 18.
 */
void PHY__STATE_req(const enum PHY_Cmd_t cmd);

/**//**
 * \brief   Handles confirmation of the Radio state switching.
 * \details This function must be provided by the higher-level layer. It will be called by the Driver when Radio reports
 *  STATE_DONE event after switching its state. This function is called by the Driver in the context of a Level 1
 *  interrupt.
 * \details It is allowed to issue a new request to the Driver directly in the context of this function execution. The
 *  Driver has already entered the output state requested by the corresponding state switching request being currently
 *  confirmed.
 * \details There is no status of the performed state switching attempt. If this function is called by the Driver, it
 *  means that the confirmed state switching was performed successfully. There is no a common case failure that may be
 *  confirmed here.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.8, table 15.<\br>
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 7.1, figure 7-1.
 */
extern void PHY__STATE_conf(void);

/**//**
 * \brief   Initiates a Clear Channel Assessment (CCA) detection.
 * \details This function performs a CCA cycle almost in conformity with the IEEE Std. 802.15.4-2006/2003. The mentioned
 *  standards have tiny difference in how the requested CCA is confirmed when the radio current state is TX_ON. Due to
 *  this reason, and also taking into account that there is no practical reason to request CCA during TX_ON and TRX_OFF
 *  states, this function prohibits requesting CCA for radio states TX_ON (BUSY_TX) and TRX_OFF, nevertheless preserving
 *  conventional behavior for RX_ON and BUSY_RX states.
 * \details Strictly according to the IEEE Std. 802.15.4-2006/2003, CCA takes up to 8 symbols when the radio is in the
 *  RX_ON and not BUSY_RX state and then returns with either IDLE or BUSY status, and returns (immediately) with the
 *  BUSY status if the radio is currently in the BUSY_RX state (i.e., the PPDU reception is in progress) irrespectively
 *  of the CCA operation mode. The radio is considered to be in the BUSY_RX state (i.e., continue receiving the PPDU)
 *  following detection of the SFD until the number of octets specified by the decoded PHR has been received. The
 *  requested CCA detection does not cancel PPDU reception.
 * \details It is not allowed to issue new request to the radio, either of the same or different type, until the
 *  previous request is confirmed.
 * \details This function disables interrupts of Level 1 and 2 for the period of its execution and restores the status
 *  on return. This function must normally be called by the higher-level layer from the Level 1 interrupt execution
 *  context just because all confirmation and indication callback handlers provided by the higher-level layer are called
 *  by the Driver also in the context of a Level 1 interrupt. Hence this function may be called from the main thread as
 *  well if necessary. It is allowed to issue a new request to the Driver directly in the context of any confirmation or
 *  indication handler called by the Driver.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.2.1, 6.9.9.
 */
void PHY__CCA_req(void);

/**//**
 * \brief   Handles confirmation of the Clear Channel Assessment (CCA) detection.
 * \details This function has implicit parameter which is assigned by the Radio Driver prior to call it:
 *  - PHY_FrmBuf__Status.ccaIdle    - TRUE if the confirmed status is IDLE (0x04); FALSE if the confirmed status is
 *      BUSY (0x00).
 *
 * \note    Take into account that the returned status has the boolean type, but not the PHY enumeration type.
 * \details This function must be provided by the higher-level layer. It will be called by the Driver when Radio reports
 *  CCA_DONE event after executing a CCA cycle. This function is called by the Driver in the context of a Level 1
 *  interrupt.
 * \details It is allowed to issue a new request to the Driver directly in the context of this function execution. The
 *  Driver has already finished the CCA cycle requested by the corresponding CCA request being currently confirmed.
 * \details This function may return either IDLE or BUSY status. If this function is called by the Driver, it means that
 *  the confirmed CCA cycle was performed successfully and its result is either IDLE or BUSY. There is no a common case
 *  failure that may be confirmed here unlike the IEEE Std. 802.15.4-2006/2003 case - i.e., the failure statuses TRX_OFF
 *  and TX_ON may not be returned.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.2.2, 6.2.3, tables 10, 18.
 */
extern void PHY__CCA_conf(void);

/**//**
 * \brief   Initiates an Energy Detection (ED) measurement.
 * \details This function performs an ED cycle almost in conformity with the IEEE Std. 802.15.4-2006/2003. The mentioned
 *  standards define error statuses for cases when ED is requested while the radio current state is TX_ON or TRX_OFF.
 *  Taking into account that there is no practical reason to request ED during other states except RX_ON (or BUSY_RX),
 *  this function prohibits requesting CCA for radio states TX_ON (BUSY_TX) and TRX_OFF, nevertheless preserving
 *  conventional behavior for RX_ON state and almost conventional for BUSY_RX state. For the case of BUSY_RX state this
 *  function simulates ED measurement in order not to disturb the packet reception - the energy level value 0 dBm is
 *  confirmed.
 * \details It is not allowed to issue new request to the radio, either of the same or different type, until the
 *  previous request is confirmed.
 * \details This function disables interrupts of Level 1 and 2 for the period of its execution and restores the status
 *  on return. This function must normally be called by the higher-level layer from the Level 1 interrupt execution
 *  context just because all confirmation and indication callback handlers provided by the higher-level layer are called
 *  by the Driver also in the context of a Level 1 interrupt. Hence this function may be called from the main thread as
 *  well if necessary. It is allowed to issue a new request to the Driver directly in the context of any confirmation or
 *  indication handler called by the Driver.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.2.3, 6.9.7.
 */
void PHY__ED_req(void);

/**//**
 * \brief   Handles confirmation of the Energy Detection (ED) measurement.
 * \details This function has implicit parameter which is assigned by the Radio Driver prior to call it:
 *  - PHY_FrmBuf__Status.edLevel    - Energy level detected, unsigned 8-bit integer, in hardware-specific units. Use
 *      conversion macro-function provided by the Radio Driver to obtain the ED value in dBm.
 *
 * \details This function must be provided by the higher-level layer. It will be called by the Driver when Radio reports
 *  CCA_ED_DONE event after executing an ED cycle. This function is called by the Driver in the context of a Level 1
 *  interrupt.
 * \details It is allowed to issue a new request to the Driver directly in the context of this function execution. The
 *  Driver has already finished the ED cycle requested by the corresponding ED request being currently confirmed.
 * \details There is no status of the performed ED measurement attempt. If this function is called by the Driver, it
 *  means that the confirmed ED measurement was performed successfully. There is no a common case failure that may be
 *  confirmed here.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.4, table 11.
 */
extern void PHY__ED_conf(void);

/**//**
 * \brief   Initiates the Radio channel and page switching.
 * \param[in]   pgch    The new channel page and channel to be switched to. Valid range is hardware-dependent.
 * \note    The channel and page switching is established by the dedicated request-confirm pair instead of conventional
 *  interface provided by the PLME-SET primitive used for the PHY PIB attribute phyCurrentChannel.
 * \details It is not allowed to issue new request to the radio, either of the same or different type, until the
 *  previous request is confirmed.
 * \details This function disables interrupts of Level 1 and 2 for the period of its execution and restores the status
 *  on return. This function must normally be called by the higher-level layer from the Level 1 interrupt execution
 *  context just because all confirmation and indication callback handlers provided by the higher-level layer are called
 *  by the Driver also in the context of a Level 1 interrupt. Hence this function may be called from the main thread as
 *  well if necessary. It is allowed to issue a new request to the Driver directly in the context of any confirmation or
 *  indication handler called by the Driver.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.1.2, 6.2.9.2, tables 2, 16, 23.
 */
void PHY__CHANNEL_req(const PHY_PageChannel_t pgch);

/**//**
 * \brief   Handles confirmation of the channel and page switching.
 * \note    The channel and page switching is established by the dedicated request-confirm pair instead of conventional
 *  interface provided by the PLME-SET primitive used for the PHY PIB attribute phyCurrentChannel.
 * \details This function must be provided by the higher-level layer. It will be called by the Driver when Radio reports
 *  CHANNEL_DONE event after executing a channel switching. This function is called by the Driver in the context of a
 *  Level 1 interrupt.
 * \details It is allowed to issue a new request to the Driver directly in the context of this function execution. The
 *  Driver has already finished the channel switching requested by the corresponding channel request being currently
 *  confirmed.
 * \details There is no status of the performed channel switching attempt. If this function is called by the Driver, it
 *  means that the confirmed channel switching was performed successfully. There is no a common case failure that may be
 *  confirmed here.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.10, table 17.
 */
extern void PHY__CHANNEL_conf(void);

/**//**
 * \brief   Switches the Radio transceiver Clear Channel Assessment (CCA) Mode.
 * \param[in]   ccaMode     CCA Mode to be set.
 * \details This function switches the CCA Mode. Switching is performed synchronously; no confirmation is issued. Newly
 *  selected CCA Mode will take effect next time CCA detection is requested.
 */
void PHY__CCAMode_set(const enum PHY_CCAMode_t ccaMode);

/**//**
 * \brief   Tunes the Radio transceiver nominal transmit power.
 * \param[in]   txPower     Value of transmit power to be assigned, in dBm.
 * \details This function switches the nominal transmit power. Transmit power is measured in decibels relative to 1 mW,
 *  i.e. in dBm. The valid range is from -32 dBm to +31 dBm. Switching is performed synchronously; no confirmation is
 *  issued.
 * \note    The PHY shall validate that the specified transmit power value is supported by the Radio Hardware. This
 *  function does not validate the specified value. If the requested transmit power is outside the actually implemented
 *  range, it is set to the corresponding (top or bottom) boundary.
 */
void HAL_RadioSetTransmitPower(const PHY_TXPower_t txPower);

/**//**
 * \brief   Returns the current Received Signal Strength Indicator (RSSI) reported by the Radio hardware.
 * \return  Value of the RSSI, hardware-specific units.
 * \details The RSSI result is an integer ranging from 0x00 to 0xFF. The minimum RSSI value is 0x00. Use conversion
 *  function provided by this interface to express the RSSI in dBm.
 * \details The function returns synchronously. If the Radio hardware is not busy at the moment, the new RSSI
 *  measurement may be performed (depending on the platform). If the Radio is currently busy, the last measured RSSI
 *  value is returned. For the second case, the Radio Driver renews the stored RSSI value periodically on its own
 *  discretion.
 */
PHY_RSSI_t PHY__RSSI_get(void);

/**//**
 * \brief   Returns the current Radio Driver state.
 * \return  The current Radio Driver state. One of TRX_OFF, RX_ON, TX_ON, BUSY_RX, BUSY_TX, UNDEFINED.
 * \details On particular platforms there are additional substates. This function performs conversion of the substate
 *  code to the main state code.
 * \note    The Radio Driver state may differ from the Radio hardware state during short periods of state switching when
 *  either the Radio Driver state is being updated after the Radio hardware state, or vice-versa.
 */
enum PHY_State_t PHY__STATE_get(void);

#ifdef _PHY_TEST_HOST_INTERFACE_
/******************************* Functions to switch return path to PHY_TEST module ***********************************/
extern void PHY_Test_DataConf(void);
extern void PHY_Test_DataInd(void);
extern void PHY_Test_EdConf(void);
extern void PHY_Test_SetTrxStateConf(void);
#endif

#endif /* _BB_HAL_RADIO_H */
