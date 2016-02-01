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
* FILENAME: $Workfile: trunk/stack/common/HAL/include/bbHalRadio.h $
*
* DESCRIPTION:
*   Radio Hardware interface.
*
* $Revision: 4249 $
* $Date: 2014-10-27 22:19:16Z $
*
*****************************************************************************************/


#ifndef _BB_HAL_RADIO_H
#define _BB_HAL_RADIO_H


/************************* INCLUDES *****************************************************/
#include "bbHalSymTmr.h"            /* Symbol Timer Hardware interface. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of transceiver states for the Radio Hardware.
 * \details The UNDEFINED state stands for the case when the transceiver is in the middle
 *  of its state switching; it may also be used as the initial value for the software
 *  cashed value of the real hardware state after the software initialization. Other
 *  states are conventional according to the Standard.
 */
typedef enum _HAL_RadioStateId_t
{
    HAL_RADIO_STATE_UNDEFINED = 0x00,       /*!< Transceiver state is UNDEFINED (i.e., during the state switching or
                                                just after the hardware and software reset and/or initialization). */

    HAL_RADIO_STATE_TRX_OFF   = 0x08,       /*!< Transceiver is in the TRX_OFF state. */

    HAL_RADIO_STATE_RX_ON     = 0x06,       /*!< Transceiver is in the RX_ON state. */

    HAL_RADIO_STATE_BUSY_RX   = 0x01,       /*!< Transceiver is in the BUSY_RX state. */

    HAL_RADIO_STATE_TX_ON     = 0x09,       /*!< Transceiver is in the TX_ON state. */

    HAL_RADIO_STATE_BUSY_TX   = 0x02,       /*!< Transceiver is in the BUSY_TX state. */

} HAL_RadioStateId_t;


/**//**
 * \brief   Enumeration of commands to switch the Radio transceiver state.
 * \note    The deferrable TRX_OFF command is implemented on the PHY layer by means of the
 *  software via issuing the immediate FORCE_TRX_OFF command just after completion of
 *  BUSY_RX or BUSY_TX state; and the original immediate FORCE_TRX_OFF command is
 *  implemented directly by the Radio hardware. The deferrable TRX_OFF command, even if it
 *  is supported by the Radio hardware, is not used for calls to the Radio HAL.
 * \note    The deferrable RX_ON command has no immediate variant (i.e., there is no
 *  FORCE_RX_ON command indeed) according to the Standard; and also its immediate variant
 *  is not implemented by the Radio hardware. But in fact, the deferrable RX_ON command
 *  acts just like immediate FORCE_RX_ON, if only it is not issued for the BUSY_TX state
 *  of the transceiver. The Radio HAL functions shall not be called with the RX_ON (namely
 *  the FORCE_RX_ON) command if the Radio hardware transceiver is in the BUSY_TX state.
 *  The PHY implements deferring of the RX_ON command by means of the software via issuing
 *  the quasi-immediate FORCE_RX_ON command just after completion of the BUSY_TX state.
 * \note    The TX_ON command is indeed the FORCE_TX_ON command according to the Standard,
 *  and is performed as immediate command in all cases. The truly deferrable TX_ON command
 *  is not used, even if it is implemented by the hardware (according to the older
 *  IEEE Std. 804.15.4-2003).
 */
typedef enum _HAL_RadioSetTrxStateCmd_t
{
    HAL_RADIO_CMD_FORCE_TRX_OFF = 0x03,     /*!< Switch the transceiver state to TRX_OFF immediately. */

    HAL_RADIO_CMD_FORCE_RX_ON   = 0x06,     /*!< Switch the transceiver state to RX_ON quasi-immediately. */

    HAL_RADIO_CMD_FORCE_TX_ON   = 0x09,     /*!< Switch the transceiver state to TX_ON immediately. */

} HAL_RadioSetTrxStateCmd_t;


/**//**
 * \brief   Enumeration of results of received frame CRC validation.
 */
typedef enum _HAL_RadioCrcValid_t
{
    HAL_RADIO_CRC_BROKEN = 0,       /*!< The received frame has broken CRC. */

    HAL_RADIO_CRC_VALID  = 1,       /*!< The received frame has valid CRC. */

} HAL_RadioCrcValid_t;


/**//**
 * \brief   Enumeration of modes for designating either immediate or timed transmission.
 */
typedef enum _HAL_RadioTxTimed_t
{
    HAL_RADIO_TX_IMMEDIATE = 0,     /*!< Start transmission immediately. */

    HAL_RADIO_TX_TIMED     = 1,     /*!< Perform timed transmission. */

} HAL_RadioTxTimed_t;


/**//**
 * \brief   Enumeration of CCA statuses.
 */
typedef enum _HAL_RadioCcaStatusId_t
{
    HAL_RADIO_CCA_BUSY = 0x00,      /*!< The CCA attempt has detected a busy channel. */

    HAL_RADIO_CCA_IDLE = 0x04,      /*!< The CCA attempt has detected an idle channel. */

} HAL_RadioCcaStatusId_t;


/**//**
 * \brief   Data type for the Energy Level representation.
 */
typedef uint32_t  HAL_RadioEd_t;


/**//**
 * \brief   Data type for the LQI value representation.
 */
typedef uint32_t  HAL_RadioLqi_t;


/**//**
 * \brief   Data type for the Radio Transceiver Channel identifier.
 * \details According to the IEEE Std. 804.15.4-2006 the Radio shall implement up to 27
 *  different channels on each channel page with identifiers from 0 to 26.
 */
typedef uint8_t  HAL_RadioChannelId_t;


/**//**
 * \brief   Data type for the plain value of the Channel-on-Page structured object.
 * \details Value of this type has 16-bit width. The MSB contains the channel page
 *  identifier; and the LSB contains the logical channel identifier.
 * \note    This data type is not binary compatible with the 32-bit data type representing
 *  the channel page identifier in bits #27-31 and the implemented channels mask in bits
 *  #0-26.
 */
typedef uint16_t  HAL_RadioChannelOnPagePlain_t;


/**//**
 * \brief   Macro to extract the logical channel identifier from a Channel-on-Page object.
 * \param[in]   channelOnPagePlain      Plain value of a Channel-on-Page object.
 * \return  Logical channel identifier.
 */
#define HAL_RADIO_EXTRACT_CHANNEL_ID(channelOnPagePlain)        ((uint8_t)(channelOnPagePlain))


/**//**
 * \brief   Macro to extract the channel page identifier from a Channel-on-Page object.
 * \param[in]   channelOnPagePlain      Plain value of a Channel-on-Page object.
 * \return  Channel page identifier.
 */
#define HAL_RADIO_EXTRACT_PAGE_ID(channelOnPagePlain)           ((uint8_t)((channelOnPagePlain) >> 8))


/**//**
 * \brief   Number of channel pages implemented.
 */
#if defined(__SoC__)
# define HAL_RADIO_CHANNEL_PAGES_NUMBER     SOC_RADIO_CHANNEL_PAGES_NUMBER
#
#elif defined(__ML507__)
# define HAL_RADIO_CHANNEL_PAGES_NUMBER     AT86RF231_CHANNEL_PAGES_NUMBER
#
#else /* __i386__ */
# define HAL_RADIO_CHANNEL_PAGES_NUMBER     PC_RADIO_CHANNEL_PAGES_NUMBER
#
#endif


/*
 * Identifiers and IEEE-indexes of all implemented channel pages.
 * NOTE: C-indexes of all channel pages must start from zero and follow sequentially,
 *  while IEEE-indexes may be listed in arbitrary order and starting not necessarily from
 *  zero (at least because the page #0 may not be implemented in particular case).
 */
#if defined(__SoC__)
# define HAL_RADIO_CHANNEL_PAGE_0   SOC_RADIO_CHANNEL_PAGE_0        /*!< IEEE-index of the channel page[0]. */
# define HAL_RADIO_CHANNEL_PAGE_1   SOC_RADIO_CHANNEL_PAGE_1        /*!< IEEE-index of the channel page[1]. */
# define HAL_RADIO_CHANNEL_PAGE_2   SOC_RADIO_CHANNEL_PAGE_2        /*!< IEEE-index of the channel page[2]. */
#
#elif defined(__ML507__)
# define HAL_RADIO_CHANNEL_PAGE_0   AT86RF231_CHANNEL_PAGE_0        /*!< IEEE-index of the channel page[0]. */
#
#else /* __i386__ */
# define HAL_RADIO_CHANNEL_PAGE_0   PC_CHANNEL_PAGE_0               /*!< Simulated IEEE-index of the channel page[0]. */
#
#endif


/*
 * Ranges of implemented channels on all implemented channel pages.
 * NOTE: The identifier of a page (from 0 to N) is the relative index in the page
 *  descriptors array (i.e. the C-index), but not the IEEE-index of the page.
 * NOTE: Identifiers (names) of all implemented channel sets must have sequential numeric
 *   suffixes, for example: BCM7366_CHANNELS_ON_PAGE_0, ..._PAGE_1, ..._PAGE_2, etc.
 *   Their suffixes must comply with corresponding suffixes of described channel pages.
 */
#if defined(__SoC__)
# define HAL_RADIO_CHANNEL_MIN_ON_PAGE_0    SOC_RADIO_CHANNEL_MIN_ON_PAGE_0     /*!< First channel on the page[0]. */
# define HAL_RADIO_CHANNEL_MAX_ON_PAGE_0    SOC_RADIO_CHANNEL_MAX_ON_PAGE_0     /*!< Last channel on the page[0]. */
# define HAL_RADIO_CHANNEL_MIN_ON_PAGE_1    SOC_RADIO_CHANNEL_MIN_ON_PAGE_1     /*!< First channel on the page[1]. */
# define HAL_RADIO_CHANNEL_MAX_ON_PAGE_1    SOC_RADIO_CHANNEL_MAX_ON_PAGE_1     /*!< Last channel on the page[1]. */
# define HAL_RADIO_CHANNEL_MIN_ON_PAGE_2    SOC_RADIO_CHANNEL_MIN_ON_PAGE_2     /*!< First channel on the page[2]. */
# define HAL_RADIO_CHANNEL_MAX_ON_PAGE_2    SOC_RADIO_CHANNEL_MAX_ON_PAGE_2     /*!< Last channel on the page[2]. */
#
#elif defined(__ML507__)
# define HAL_RADIO_CHANNEL_MIN_ON_PAGE_0    AT86RF231_CHANNEL_MIN_ON_PAGE_0     /*!< First channel on the page[0]. */
# define HAL_RADIO_CHANNEL_MAX_ON_PAGE_0    AT86RF231_CHANNEL_MAX_ON_PAGE_0     /*!< Last channel on the page[0]. */
#
#else /* __i386__ */
# define HAL_RADIO_CHANNEL_MIN_ON_PAGE_0    PC_CHANNEL_MIN_ON_PAGE_0    /*!< First channel simulated on the page[0]. */
# define HAL_RADIO_CHANNEL_MAX_ON_PAGE_0    PC_CHANNEL_MAX_ON_PAGE_0    /*!< Last channel simulated on the page[0]. */
#
#endif


/*
 * Sets of implemented channels on all implemented channel pages.
 * NOTE: The identifier of a page (from 0 to N) is the relative index in the page
 *  descriptors array (i.e. the C-index), but not the IEEE-index of the page.
 * NOTE: Identifiers (names) of all implemented channel sets must have sequential numeric
 *   suffixes, for example: SOC_RADIO_CHANNELS_ON_PAGE_0, ..._PAGE_1, ..._PAGE_2, etc.
 *   Their suffixes must comply with corresponding suffixes of described channel pages.
 */
#if defined(__SoC__)
# define HAL_RADIO_CHANNELS_ON_PAGE_0   SOC_RADIO_CHANNELS_ON_PAGE_0    /*!< Channels implemented on the page[0]. */
# define HAL_RADIO_CHANNELS_ON_PAGE_1   SOC_RADIO_CHANNELS_ON_PAGE_1    /*!< Channels implemented on the page[1]. */
# define HAL_RADIO_CHANNELS_ON_PAGE_2   SOC_RADIO_CHANNELS_ON_PAGE_2    /*!< Channels implemented on the page[2]. */
#
#elif defined(__ML507__)
# define HAL_RADIO_CHANNELS_ON_PAGE_0   AT86RF231_CHANNELS_ON_PAGE_0    /*!< Channels implemented on the page[0]. */
#
#else /* __i386__ */
# define HAL_RADIO_CHANNELS_ON_PAGE_0   PC_CHANNELS_ON_PAGE_0           /*!< Channels simulated on the page[0]. */
#
#endif


/*
 * Default channel and channel page.
 */
#if defined(__SoC__)
# define HAL_RADIO_PAGE_DEFAULT         SOC_RADIO_PAGE_DEFAULT          /*!< Default channel page. */
# define HAL_RADIO_CHANNEL_DEFAULT      SOC_RADIO_CHANNEL_DEFAULT       /*!< Default channel. */
#
#elif defined(__ML507__)
# define HAL_RADIO_PAGE_DEFAULT         AT86RF231_PAGE_DEFAULT          /*!< Default channel page. */
# define HAL_RADIO_CHANNEL_DEFAULT      AT86RF231_CHANNEL_DEFAULT       /*!< Default channel. */
#
#else /* __i386__ */
# define HAL_RADIO_PAGE_DEFAULT         PC_PAGE_DEFAULT                 /*!< Simulated default channel page. */
# define HAL_RADIO_CHANNEL_DEFAULT      PC_CHANNEL_DEFAULT              /*!< Simulated default channel. */
#
#endif


/**//**
 * \brief   Data type for the Radio Transceiver nominal transmit power value.
 * \details Nominal transmit power is measured in decibels relative to 1 mW, i.e. in dBm.
 * \details The valid range is from -32 dBm to +31 dBm.
 */
typedef int8_t  HAL_RadioTxPowerValue_t;


/*
 * Limits, default and tolerance for the Radio Transceiver nominal transmit power value.
 */
#if defined(__SoC__)
# define HAL_RADIO_TX_POWER_MIN  SOC_RADIO_TX_POWER_MIN     /*!< Minimum implemented Radio Transmitter power value. */
# define HAL_RADIO_TX_POWER_MAX  SOC_RADIO_TX_POWER_MAX     /*!< Maximum implemented Radio Transmitter power value. */
# define HAL_RADIO_TX_POWER_DEF  SOC_RADIO_TX_POWER_DEF     /*!< Default value of the Radio Transmitter power. */
# define HAL_RADIO_TX_POWER_TOL  SOC_RADIO_TX_POWER_TOL     /*!< Tolerance of the Radio Transmitter power value,
                                                                absolute value, measured in dBm. */
#
#elif defined(__ML507__)
# define HAL_RADIO_TX_POWER_MIN  AT86RF231_TX_POWER_MIN     /*!< Minimum implemented Radio Transmitter power value. */
# define HAL_RADIO_TX_POWER_MAX  AT86RF231_TX_POWER_MAX     /*!< Maximum implemented Radio Transmitter power value. */
# define HAL_RADIO_TX_POWER_DEF  AT86RF231_TX_POWER_DEF     /*!< Default value of the Radio Transmitter power. */
# define HAL_RADIO_TX_POWER_TOL  AT86RF231_TX_POWER_TOL     /*!< Tolerance of the Radio Transmitter power value,
                                                                absolute value, measured in dBm. */
#
#else /* __i386__ */
# define HAL_RADIO_TX_POWER_MIN  PC_TX_POWER_MIN        /*!< Simulated minimum Radio Transmitter power value. */
# define HAL_RADIO_TX_POWER_MAX  PC_TX_POWER_MAX        /*!< Simulated maximum Radio Transmitter power value. */
# define HAL_RADIO_TX_POWER_DEF  PC_TX_POWER_DEF        /*!< Simulated default value of the Radio Transmitter power. */
# define HAL_RADIO_TX_POWER_TOL  PC_TX_POWER_TOL        /*!< Simulated tolerance of the Radio Transmitter power value,
                                                            absolute value, measured in dBm. */
#
#endif


/**//**
 * \brief   Enumeration of CCA Modes.
 * \note    Not necessarily all modes are supported by particular Radio Hardware.
 */
typedef enum _HAL_RadioCcaModeId_t
{
    HAL_RADIO_CCA_MODE_3OR  = 0,      /*!< Mode 3-OR, carrier sense OR energy above threshold. */

    HAL_RADIO_CCA_MODE_1    = 1,      /*!< Mode 1, energy above threshold. */

    HAL_RADIO_CCA_MODE_2    = 2,      /*!< Mode 2, carrier sense only. */

    HAL_RADIO_CCA_MODE_3AND = 3,      /*!< Mode 3-AND, carrier sense AND energy above threshold. */

} HAL_RadioCcaModeId_t;


/*
 * Limits and default value for the Radio CCA mode.
 */
#if defined(__SoC__)
# define HAL_RADIO_CCA_MODE_MIN  SOC_RADIO_CCA_MODE_MIN     /*!< Minimum code value of implemented Radio CCA mode. */
# define HAL_RADIO_CCA_MODE_MAX  SOC_RADIO_CCA_MODE_MAX     /*!< Maximum code value of implemented Radio CCA mode. */
# define HAL_RADIO_CCA_MODE_DEF  SOC_RADIO_CCA_MODE_DEF     /*!< Default code value of Radio CCA mode. */
#
#elif defined(__ML507__)
# define HAL_RADIO_CCA_MODE_MIN  AT86RF231_CCA_MODE_MIN     /*!< Minimum code value of implemented Radio CCA mode. */
# define HAL_RADIO_CCA_MODE_MAX  AT86RF231_CCA_MODE_MAX     /*!< Maximum code value of implemented Radio CCA mode. */
# define HAL_RADIO_CCA_MODE_DEF  AT86RF231_CCA_MODE_DEF     /*!< Default code value of Radio CCA mode. */
#
#else /* __i386__ */
# define HAL_RADIO_CCA_MODE_MIN  PC_CCA_MODE_MIN            /*!< Simulated minimum code value of Radio CCA mode. */
# define HAL_RADIO_CCA_MODE_MAX  PC_CCA_MODE_MAX            /*!< Simulated maximum code value of Radio CCA mode. */
# define HAL_RADIO_CCA_MODE_DEF  PC_CCA_MODE_DEF            /*!< Simulated default code value of Radio CCA mode. */
#
#endif


/**//**
 * \brief   Converts number of bytes to number of 4-byte words rounding it up.
 * \param[in]   bytes   Number of bytes to convert. Must not be zero.
 * \return  Number of whole or incomplete 4-byte words covering \p bytes.
 * \details This macro function converts specified number of bytes to the number of whole
 *  or incomplete 4-byte words. If the specified number of bytes is not a multiple of 4,
 *  then the number of words is rounded up.
 * \note    Do not use this macro for the case when \p bytes equals to or less then zero.
 */
#define HAL_RADIO_CONVERT_BYTES_TO_WORDS(bytes)     ((bytes) >> 2) + (0 != ((bytes) & 0x3))
/*
 * Validate the size of word on this platform; validate the result of implicit conversion
 * of boolean expression that evaluates to TRUE to integer data type.
 */
SYS_DbgAssertStatic(4 == sizeof(uint32_t));
SYS_DbgAssertStatic(1 == (0 != 1));


/**//**
 * \brief   Converts number of 4-byte words to number of bytes.
 * \param[in]   words   Number of 4-byte words to convert.
 * \return  Number of bytes contained in \p words.
 */
#define HAL_RADIO_CONVERT_WORDS_TO_BYTES(words)     ((words) * sizeof(uint32_t))


/*
 * Sizes of different fields of PPDU, in octets.
 */
#define HAL_FIELD_SIZE_PPDU_PHR          1  /*!< Size of the field PPDU.PHR equals 1 octet. */
#define HAL_FIELD_MAX_SIZE_PPDU_PSDU   127  /*!< Maximum size of the field PPDU.PSDU equals 127 octets. */
#define HAL_FIELD_SIZE_MPDU_MHR_FCF      2  /*!< Size of the field MPDU.MHR.FrameControl equals 2 octets. */
#define HAL_FIELD_SIZE_MPDU_MHR_DSN      1  /*!< Size of the field MPDU.MHR.SequenceNumber (DSN) equals 1 octet. */
#define HAL_FIELD_SIZE_MPDU_MHR_BSN      1  /*!< Size of the field MPDU.MHR.SequenceNumber (BSN) equals 1 octet. */
#define HAL_FIELD_SIZE_MPDU_MHR_PANID    2  /*!< Size of the field MPDU.MHR.Dst(Src)PANId equals 2 octets. */
#define HAL_FIELD_SIZE_MPDU_MHR_SADDR    2  /*!< Size of the field MPDU.MHR.Dst(Src)ShortAddress equals 2 octets. */
#define HAL_FIELD_SIZE_MPDU_MHR_EADDR    8  /*!< Size of the field MPDU.MHR.Dst(Src)ExtendedAddress equals 8 octets. */
#define HAL_FIELD_SIZE_MPDU_MSDU_CMD     1  /*!< Size of the field MPDU.MSDU.MacCommandFrameId equals 1 octet. */
#define HAL_FIELD_SIZE_MPDU_MSDU_SUPERFRAMESPEC     2   /*!< Superframe Specification field has 2 octets. */
#define HAL_FIELD_SIZE_MPDU_MSDU_GTSSPEC            1   /*!< GTS Specification field has 1 octet. */
#define HAL_FIELD_SIZE_MPDU_MSDU_PENDINGADDRSPEC    1   /*!< Pending Address Specification field has 1 octet. */
#define HAL_FIELD_SIZE_MPDU_MFR_FCS      2  /*!< Size of the field MPDU.MFR.FrameCheckSequence equals 2 octets. */
#define HAL_FIELD_SIZE_AUXILIARY_LQI     1  /*!< Size of the auxiliary field LQI equals 1 octet. */


/**//**
 * \brief   Minimum allowed value for the PPDU.PHR field, in octets.
 * \details This value correspond to the total length of the minimum allowed MHR + MSDU
 *  field (at least 1 octet) and the obligatory MPDU.MFR.FCS field (2 octets).
 */
#define HAL_FIELD_MIN_ALLOWED_PPDU_PHR      (1 + HAL_FIELD_SIZE_MPDU_MFR_FCS)


/**//**
 * \brief   Maximum allowed value for the PPDU.PHR field, in octets.
 */
#define HAL_FIELD_MAX_ALLOWED_PPDU_PHR      (HAL_FIELD_MAX_SIZE_PPDU_PSDU)


/**//**
 * \brief   Bitmask to apply to a PPDU.PHR value that was just read from the Radio
 *  hardware Frame RX-Buffer.
 * \details This bitmask guarantees the filtered value of PPDU.PHR to be not greater than
 *  127 in order not to overflow software buffers for PPDU.
 */
#define HAL_FIELD_BITMASK_PPDU_PHR  0x7F


/**//**
 * \brief   Offset of the 1-byte PPDU.PHR field in all hardware and software PPDU buffers.
 */
#define HAL_FIELD_OFFSET_PPDU_PHR  0


/**//**
 * \brief   Maximum size of PPDU, in octets.
 * \details This value includes the size of obligatory PPDU.PHR field (1 octet) and the
 *  maximum allowed size of the PPDU.PSDU field (up to 127 octets).
 */
#define HAL_FIELD_MAX_SIZE_PPDU     (HAL_FIELD_SIZE_PPDU_PHR + HAL_FIELD_MAX_SIZE_PPDU_PSDU)


/**//**
 * \brief   Maximum number of octets to be pre-read from the received PPDU.
 * \details The following fields are pre-read at maximum in the most general case:
 *  - PPDU.PHR (PHR),
 *  - MPDU.MHR.FrameControl (FCF),
 *  - MPDU.MHR.SequenceNumber (DSN),
 *  - MPDU.MHR.DstPANId (Dst. PANId),
 *  - MPDU.MHR.DstExtendedAddress (Dst. Extended Address),
 *  - MPDU.MHR.SrcPANId (Src. PANId),
 *  - MPDU.MHR.SrcExtendedAddress (Src. Extended Address),
 *  - MPDU.MSDU.MacCommandFrameId (CMD).
 * \details The following fields are pre-read in the case of beacon without GTS
 *  Specification, Pending Address Specification and Beacon Payload:
 *  - PPDU.PHR (PHR),
 *  - MPDU.MHR.FrameControl (FCF),
 *  - MPDU.MHR.SequenceNumber (BSN),
 *  - MPDU.MHR.SrcPANId (Src. PANId),
 *  - MPDU.MHR.SrcExtendedAddress (Src. Extended Address),
 *  - MPDU.MSDU.GTSSpecification,
 *  - MPDU.MSDU.PendingAddressSpecification.
 */
#define HAL_RADIO_RX_BUFFER_PREREAD_OCTETS_MAX\
        MAX(\
                (HAL_FIELD_SIZE_PPDU_PHR +\
                        HAL_FIELD_SIZE_MPDU_MHR_FCF +\
                        HAL_FIELD_SIZE_MPDU_MHR_DSN +\
                        HAL_FIELD_SIZE_MPDU_MHR_PANID * 2 +\
                        HAL_FIELD_SIZE_MPDU_MHR_EADDR * 2 +\
                        HAL_FIELD_SIZE_MPDU_MSDU_CMD),\
                (HAL_FIELD_SIZE_PPDU_PHR +\
                        HAL_FIELD_SIZE_MPDU_MHR_FCF +\
                        HAL_FIELD_SIZE_MPDU_MHR_BSN +\
                        HAL_FIELD_SIZE_MPDU_MHR_PANID +\
                        HAL_FIELD_SIZE_MPDU_MHR_EADDR +\
                        HAL_FIELD_SIZE_MPDU_MSDU_SUPERFRAMESPEC +\
                        HAL_FIELD_SIZE_MPDU_MSDU_GTSSPEC +\
                        HAL_FIELD_SIZE_MPDU_MSDU_PENDINGADDRSPEC))

/*
 * Validate the maximum number of bytes to be pre-read.
 */
SYS_DbgAssertStatic(25 == HAL_RADIO_RX_BUFFER_PREREAD_OCTETS_MAX);


/**//**
 * \brief   Maximum size of 4-byte words software buffer to store the pre-read part of the
 *  received PPDU, in 4-byte words.
 */
#define HAL_RADIO_RX_BUFFER_PREREAD_WORDS_MAX\
        HAL_RADIO_CONVERT_BYTES_TO_WORDS(HAL_RADIO_RX_BUFFER_PREREAD_OCTETS_MAX)

/*
 * Validate the size of pre-read buffer, in 4-byte words.
 */
SYS_DbgAssertStatic(7 == HAL_RADIO_RX_BUFFER_PREREAD_WORDS_MAX);


/**//**
 * \brief   Minimum number of octets shall be pre-read from the received PPDU for valid
 *  frame other than Acknowledgment (ACK) frame.
 * \details The following fields must be pre-read in any case for Beacon, Data or MAC
 *  Command frame:
 *  - PPDU.PHR (PHR),
 *  - MPDU.MHR.FrameControl (FCF),
 *  - MPDU.MHR.SequenceNumber (DSN or BSN),
 *  - MPDU.MHR.Dst(Src)PANId (Dst. or Src. PANId),
 *  - MPDU.MHR.Dst(Src)ShortAddress (Dst. or Src. Short Address).
 *
 * \note    Both Dst. and Src. addresses may not be omitted in the MPDU.MHR for a valid
 *  frame other then Acknowledgment frame.
 */
#define HAL_RADIO_RX_BUFFER_PREREAD_OCTETS_MIN\
        (HAL_FIELD_SIZE_PPDU_PHR +\
         HAL_FIELD_SIZE_MPDU_MHR_FCF +\
         HAL_FIELD_SIZE_MPDU_MHR_DSN +\
         HAL_FIELD_SIZE_MPDU_MHR_PANID +\
         HAL_FIELD_SIZE_MPDU_MHR_SADDR)

/*
 * Validate the minimum number of bytes to be pre-read.
 */
SYS_DbgAssertStatic(8 == HAL_RADIO_RX_BUFFER_PREREAD_OCTETS_MIN);


/**//**
 * \brief   Minimum number of 4-byte words in the pre-read part of the received PPDU of
 *  the frame type other than Acknowledgment (ACK) frame, in 4-byte words.
 */
#define HAL_RADIO_RX_BUFFER_PREREAD_WORDS_MIN\
        HAL_RADIO_CONVERT_BYTES_TO_WORDS(HAL_RADIO_RX_BUFFER_PREREAD_OCTETS_MIN)

/*
 * Validate the minimum number of 4-byte words to be preread.
 */
SYS_DbgAssertStatic(2 == HAL_RADIO_RX_BUFFER_PREREAD_WORDS_MIN);


/**//**
 * \brief   Standard size for the destination buffer used for complete PPDU reading,
 *  in octets.
 * \details The received frame buffer finally contains the PPDU.PHR field (1 octet) and
 *  the PPDU.PSDU (i.e., the MPDU) field (up to 127 octets) except the MPDU.MFR.FCS field
 *  (2 octets).
 */
#define HAL_RADIO_RX_BUFFER_OCTETS_MAX\
        (HAL_FIELD_MAX_SIZE_PPDU - HAL_FIELD_SIZE_MPDU_MFR_FCS)

/*
 * Validate the maximum number of bytes to be finally read.
 */
SYS_DbgAssertStatic(126 == HAL_RADIO_RX_BUFFER_OCTETS_MAX);


/**//**
 * \brief   Standard size for the destination buffer used for complete PPDU reading,
 *  in 4-byte words.
 */
#define HAL_RADIO_RX_BUFFER_WORDS_MAX\
        HAL_RADIO_CONVERT_BYTES_TO_WORDS(HAL_RADIO_RX_BUFFER_OCTETS_MAX)

/*
 * Validate the size of final read buffer, in 4-byte words.
 */
SYS_DbgAssertStatic(32 == HAL_RADIO_RX_BUFFER_WORDS_MAX);


/**//**
 * \brief   Standard size for the source buffer used for complete PPDU writing, in octets.
 * \details The transmitted frame buffer contains the PPDU.PSDU (i.e., the MPDU) field (up
 *  to 127 octets) except the MPDU.MFR.FCS field (2 octets).
 * \note    Unlike the received frame buffer, the transmitted frame buffer does not
 *  contain the PPDU.PHR field (1 octet). It is implemented just for tight compatibility
 *  with the SoC Radio Hardware.
 */
#define HAL_RADIO_TX_BUFFER_OCTETS_MAX\
        (HAL_FIELD_MAX_SIZE_PPDU_PSDU - HAL_FIELD_SIZE_MPDU_MFR_FCS)

/*
 * Validate the maximum number of bytes to be written.
 */
SYS_DbgAssertStatic(126 == HAL_RADIO_RX_BUFFER_OCTETS_MAX);


/**//**
 * \brief   Standard size for the source buffer used for complete PPDU writing,
 *  in 4-byte words.
 */
#define HAL_RADIO_TX_BUFFER_WORDS_MAX\
        HAL_RADIO_CONVERT_BYTES_TO_WORDS(HAL_RADIO_TX_BUFFER_OCTETS_MAX)

/*
 * Validate the size of write buffer, in 4-byte words.
 */
SYS_DbgAssertStatic(32 == HAL_RADIO_TX_BUFFER_WORDS_MAX);


/**//**
 * \brief   Performs the RF-INIT.request to initialize the Radio Hardware.
 * \details This function cancels all processes on the Radio Hardware, resets its state to
 *  the default configuration, then performs the Radio Hardware initialization, and
 *  finally switches the transceiver state to TRX_OFF.
 */
#if defined(__SoC__)
# define HAL_RadioInit()        SOC_RadioInit()
#
#elif defined(__ML507__)
# define HAL_RadioInit()        ML507_RadioInit()
#
#else /* __i386__ */
# define HAL_RadioInit()        PC_RadioInit()
#
#endif


/**//**
 * \brief   Performs the RF-SET-TRX-STATE.request to switch the Radio Hardware transceiver
 *  state.
 * \param[in]   cmdId   Identifier of a command to switch the Radio transceiver state. The
 *  following commands are supported: FORCE_TRX_OFF, FORCE_RX_ON, FORCE_TX_ON.
 * \details This function performs the Radio Hardware transceiver state switching between
 *  the following three states: TRX_OFF, RX_ON(BUSY_RX), TX_ON(BUSY_TX). These states
 *  stand for:
 *  - TRX_OFF - transceiver is switched off,
 *  - RX_ON   - transceiver is ready to receive,
 *  - BUSY_RX - transceiver is currently receiving,
 *  - TX_ON   - transceiver is ready to transmit,
 *  - BUSY_TX - transceiver is currently transmitting.
 *
 * \details State transitions between RX_ON and BUSY_RX, and between TX_ON and BUSY_TX are
 *  not covered by this function. The first one occurs automatically when a new packet
 *  reception starts and finishes if the transceiver is in the RX_ON state; and the second
 *  occurs when transmission is started by issuing the RF-TRANSMIT.request and then
 *  finishes if the transceiver is in the TX_ON state.
 * \details The state transition is commenced by this function immediately - i.e.,
 *  deferred commands are not supported and must be implemented by the PHY software.
 * \details The Radio Hardware is not able to perform a requested state transition in
 *  negligibly short period. When the state switching is completed by the hardware, the
 *  PHY will be notified with the RF-SET-TRX-STATE.confirm. Until such a confirmation is
 *  issued, calls to the RF-GET-TRX-STATE.request may return UNDEFINED state.
 * \note    On ML507 target platform the confirmation is issued in result of the deferred
 *  polling of the Radio Hardware state. This is performed by the Radio HAL automatically.
 * \details If this function is called again prior to the Radio Hardware confirms the
 *  previous state switching, the previous request to switch the state is canceled by the
 *  hardware without confirmation and the new state switching is started. Only the last
 *  request will be confirmed in this case to the PHY.
 * \note    On ML507 platform with the Atmel AT86RF231 radio the previous request will be
 *  completed instead of canceling but also without confirmation. It may take a prolonged
 *  time to wait for the Atmel radio to complete the previous request. During this time
 *  this function will perform continuous polling of the Radio state until it becomes
 *  defined.
 * \details Other requests to the Radio HAL from the state-switching group (to start
 *  transmission, CCA, ED or switch the channel) are prohibited until the
 *  RF-SET-TRX-STATE.confirm is returned to the PHY.
 * \details Commands to switch the state to TRX_OFF and TX_ON are performed as
 *  FORCE-commands (i.e., they terminate BUSY_TX and BUSY_RX if one is being currently
 *  performed). If need to perform the deferrable variant of TRX_OFF, the PHY shall wait
 *  for completion of continuing BUSY_TX or BUSY_RX itself and only then call this
 *  function.
 * \note    Deferrable TX_ON command was defined only in the IEEE Std. 804.15.4-2003 and
 *  was fully substituted (with the same command code) with the FORCE_TX_ON command (but
 *  preserving the original name TX_ON) in the IEEE Std. 804.15.4-2006. If still in need
 *  to perform the deferrable variant of the TX_ON command, it must be performed by the
 *  PHY software.
 * \details Command to switch the state to RX_ON is performed by the hardware as a
 *  deferrable command (i.e., for the case of BUSY_TX it will wait until transmission is
 *  completed). The PHY shall not call this function with RX_ON command if it is known
 *  that transmission was started and has not ended; the PHY may issue RX_ON command only
 *  if it is known that it will be performed just as immediate FORCE_RX_ON command. If
 *  need to perform the deferrable variant of RX_ON, the PHY shall wait for completion of
 *  continuing BUSY_TX itself and only then call this function.
 */
#if defined(__SoC__)
# define HAL_RadioSetTrxStateReq(cmdId)         SOC_RadioSetTrxStateReq(cmdId)
#
#elif defined(__ML507__)
# define HAL_RadioSetTrxStateReq(cmdId)         ML507_RadioSetTrxStateReq(cmdId)
#
#else /* __i386__ */
# define HAL_RadioSetTrxStateReq(cmdId)         PC_RadioSetTrxStateReq(cmdId)
#
#endif


/**//**
 * \brief   Performs the RF-GET-TRX-STATE.request to obtain the Radio Hardware transceiver
 *  state.
 * \return  Identifier of the actual state of the Radio transceiver.
 * \details Call this function prior to perform particular requests to the Radio Hardware
 *  to discover if it is in the appropriate state.
 * \note    If the transceiver is currently in the middle of state switching, this
 *  function will return the UNDEFINED state.
 */
#if defined(__SoC__)
# define HAL_RadioGetTrxState()     SOC_RadioGetTrxState()
#
#elif defined(__ML507__)
# define HAL_RadioGetTrxState()     ML507_RadioGetTrxState()
#
#else /* __i386__ */
# define HAL_RadioGetTrxState()     PC_RadioGetTrxState()
#
#endif


/**//**
 * \brief   Locks permanently the content of the Radio Hardware Frame RX-Buffer.
 * \details This function disables access to the Frame RX-Buffer for the Radio Hardware.
 *  If a next packet is received from the media, it will be ignored, but the content of
 *  the Frame RX-Buffer will stay unchanged. The following values provided by the Radio
 *  Hardware are also locked simultaneously with the Frame RX-Buffer:
 *  - the result of CRC value validation for the received PPDU,
 *  - the Link Quality Indication (LQI) value for the received PPDU,
 *  - the Start Timestamp value for the received PPDU,
 *  - the PPDU.PHR and PPDU.PSDU are locked as the content of the Frame RX-Buffer.
 *
 * \details This function also prevents new packets reception by the Radio Hardware even
 *  if the Frame RX-Buffer is currently ready to receive (i.e., is not locked by the
 *  hardware on previously received packet).
 * \note    On particular platforms the Frame RX-Buffer may be locked automatically by the
 *  Radio Hardware just after new packet reception (only in the case when the received
 *  packet passes the enabled hardware filters). Nevertheless, the PHY shall call this
 *  function in order to assure that the Frame RX-Buffer is locked and/or will stay locked
 *  until it is completely read by the MAC-LE software.
 * \note    On particular platforms the software may not be provided with facilities to
 *  lock the Frame RX-Buffer if it was not locked by the Radio Hardware. In such a case
 *  the specific Radio HAL Software shall drop all received packets instead of indicating
 *  them to the PHY (i.e., to disable issuing the RF-RECEIVE.indication) until the
 *  software unlocks the Frame RX-Buffer.
 */
#if defined(__SoC__)
# define HAL_RadioRxBufferLock()        SOC_RadioRxBufferLock()
#
#elif defined(__ML507__)
# define HAL_RadioRxBufferLock()        ML507_RadioRxBufferLock()
#
#else /* __i386__ */
# define HAL_RadioRxBufferLock()        PC_RadioRxBufferLock()
#
#endif


/**//**
 * \brief   Unlocks the Radio Hardware Frame RX-Buffer for new packet reception.
 * \details This function restores access to the Frame RX-Buffer for the Radio Hardware.
 *  The Frame RX-Buffer may have been locked on a new packet reception by the Radio
 *  Hardware automatically or for some reason by the PHY software to prevent new packets
 *  reception. In both cases this function (re-)enables the Frame RX-Buffer.
 * \note    On particular platforms the Frame RX-Buffer, if it was locked by the Radio
 *  Hardware on a new frame reception, is not unlocked automatically and thus must be
 *  unlocked by the software.
 */
#if defined(__SoC__)
# define HAL_RadioRxBufferUnlock()      SOC_RadioRxBufferUnlock()
#
#elif defined(__ML507__)
# define HAL_RadioRxBufferUnlock()      ML507_RadioRxBufferUnlock()
#
#else /* __i386__ */
# define HAL_RadioRxBufferUnlock()      PC_RadioRxBufferUnlock()
#
#endif


/**//**
 * \brief   Starts the pre-read phase of the received PPDU content for the Radio Hardware
 *  Frame RX-Buffer.
 * \return  TRUE if the received PPDU has valid CRC value; FALSE if it is broken.
 * \details This function prepares the Radio Hardware for reading of the necessary part of
 *  the received packet in order to provide the MAC software with data to perform
 *  filtering and acknowledgment. This function also returns the result of the CRC value
 *  validation that was performed automatically by the Radio Hardware for the just
 *  received frame.
 * \note    In general a received PPDU is signaled by the Radio hardware to the PHY, and
 *  by the PHY to the MAC-LE irrespectively to the result of the CRC validation; the
 *  MAC-LE is responsible for the CRC validation and received frame filtering and
 *  rejection. But on particular platforms there may be a MAC accelerator implemented; and
 *  received packets with broken CRC may not be signaled to the PHY at all. For particular
 *  implementations of the HAL, it may be in need to process all PPDU, even with broken
 *  CRC, at least to switch the software cash value of the Radio Hardware transceiver
 *  state back from BUSY_RX to RX_ON.
 */
#if defined(__SoC__)
# define HAL_RadioRxBufferPreReadStart()        SOC_RadioRxBufferPreReadStart()
#
#elif defined(__ML507__)
# define HAL_RadioRxBufferPreReadStart()        ML507_RadioRxBufferPreReadStart()
#
#else /* __i386__ */
# define HAL_RadioRxBufferPreReadStart()        PC_RadioRxBufferPreReadStart()
#
#endif


/**//**
 * \brief   Performs pre-reading of the received PPDU content for the Radio Hardware Frame
 *  RX-Buffer.
 * \param[out]  dst     Pointer to the current position in the destination 4-byte words
 *  buffer for the received PPDU.
 * \param[in]   count   Number of 4-byte words to read.
 * \details This function is used to read content of the Frame RX-Buffer in 4-byte chunks
 *  at the pre-read phase. It may be called arbitrary number of times. This function reads
 *  \p count of 4-byte words starting at the current position inside the Frame RX-Buffer
 *  and stores them serially in the memory pointed with the \p dst. The first octet is
 *  placed into the least significant byte of 4-byte word. The Frame RX-Buffer Access
 *  Window is shifted automatically by the Radio hardware; the \p dst must be shifted by
 *  the caller after this function returns for the \p count number of requested words.
 */
#if defined(__SoC__)
# define HAL_RadioRxBufferPreRead(dst, count)       SOC_RadioRxBufferPreRead(dst, count)
#
#elif defined(__ML507__)
# define HAL_RadioRxBufferPreRead(dst, count)       ML507_RadioRxBufferPreRead(dst, count)
#
#else /* __i386__ */
# define HAL_RadioRxBufferPreRead(dst, count)       PC_RadioRxBufferPreRead(dst, count)
#
#endif


/**//**
 * \brief   Starts the final-read phase of the received PPDU content for the Radio
 *  Hardware Frame RX-Buffer.
 * \details This function prepares the Radio Hardware for reading of the rest part of the
 *  received packet (after the pre-reading phase) in order to provide the MAC software
 *  with data to perform further processing of the received frame.
 */
#if defined(__SoC__)
# define HAL_RadioRxBufferFinalReadStart()      SOC_RadioRxBufferFinalReadStart()
#
#elif defined(__ML507__)
# define HAL_RadioRxBufferFinalReadStart()      ML507_RadioRxBufferFinalReadStart()
#
#else /* __i386__ */
# define HAL_RadioRxBufferFinalReadStart()      PC_RadioRxBufferFinalReadStart()
#
#endif


/**//**
 * \brief   Performs final-reading of the received PPDU content and its LQI value for the
 *  Radio Hardware Frame RX-Buffer.
 * \param[out]  ppdu    Pointer to the origin of the 4-byte words array for the received
 *  PPDU.
 * \return  Value of the Link Quality Indicator (LQI) for the received packet.
 * \details This function is used to read content of the Frame RX-Buffer in 4-byte chunks
 *  at the final-read phase. It is called once by the MAC-LE. This function reads the
 *  whole PPDU except the trailing MPDU.MFR.FCS 2-byte field (it contains the CRC value,
 *  and is not needed to the MAC for the final processing of the received frame) and
 *  stores the PPDU in the memory pointed with \p ppdu. The first octet is placed into the
 *  least significant byte of 4-byte word. The number of 4-byte words to read is
 *  calculated by this function according to the PPDU.PHR value read out from the Frame
 *  RX-Buffer.
 * \note    For safety the value of PPDU.PHR field (the first octet of PPDU) is limited by
 *  127 (i.e., the PPDU.PSDU total length is up to 127 octets, and the PPDU length is up
 *  to 128 octets) in order not to overrun the array pointed with the \p ppdu.
 *  Nevertheless, the value of the PPDU.PHR is stored into the \p ppdu as-is without any
 *  limitations.
 * \note    Depending on the Radio Hardware the LQI value may be calculated by different
 *  methods. It may be evaluated by the Radio Hardware automatically and returned by the
 *  Radio HAL software as-is, or it may be evaluated by the Radio HAL on the base of more
 *  simple properties of the received packet (energy level, sound-to-noise ratio, etc.).
 */
#if defined(__SoC__)
# define HAL_RadioRxBufferFinalRead(ppdu)       SOC_RadioRxBufferFinalRead(ppdu)
#
#elif defined(__ML507__)
# define HAL_RadioRxBufferFinalRead(ppdu)       ML507_RadioRxBufferFinalRead(ppdu)
#
#else /* __i386__ */
# define HAL_RadioRxBufferFinalRead(ppdu)       PC_RadioRxBufferFinalRead(ppdu)
#
#endif


/**//**
 * \brief   Finishes the continuous pre- or final-read session on the communication
 *  channel with the Radio Hardware.
 * \details This function closes the communication session with the Radio unit.
 * \note    This function is provided to support platforms which uses an external Radio
 *  Hardware and communicates with the Radio unit via a special communication channel
 *  instead of direct access via the CPU address-data bus. For the case of platform with
 *  the internal Radio unit this function does nothing.
 */
#if defined(__SoC__)
# define HAL_RadioRxBufferReadFinish()      SOC_RadioRxBufferReadFinish()
#
#elif defined(__ML507__)
# define HAL_RadioRxBufferReadFinish()      ML507_RadioRxBufferReadFinish()
#
#else /* __i386__ */
# define HAL_RadioRxBufferReadFinish()      PC_RadioRxBufferReadFinish()
#
#endif


/**//**
 * \brief   Writes specified PHR and PSDU fields of the PPDU to be transmitted into the
 *  Radio Hardware Frame TX-Buffer.
 * \param[in]   phr     Value of the PPDU.PHR field. Must be from 3 to 127.
 * \param[in]   psdu    Pointer to the origin of the 4-byte words buffer containing the
 *  PPDU.PSDU field of the packet to be transmitted.
 * \details This function assigns the Radio Hardware Frame TX-Buffer with the PPDU to be
 *  transmitted. The PPDU.PHR value (1-byte length) specified with the \p phr is written
 *  the first; then the array of <em>(PHR - 2)<\em> bytes specified with the \p psdu is
 *  written. The PPDU.PSDU field is assigned except the trailing 2-bytes length
 *  MPDU.MFR.FCS field containing the CRC value of consecutive MPDU.MHR and MPDU.MSDU
 *  fields. The Radio Hardware must automatically calculate and assign the value of the
 *  MPDU.MFR.FCS field in order to free the MAC software of responsibility to evaluate and
 *  provide the calculated CRC with the PPDU.PSDU field (i.e., the MPDU.MFR.FCS field)
 *  specified with the \p psdu.
 * \note
 *  This function does not start the new frame transmission, it just prepares the frame to
 *  be transmitted in the Frame TX-Buffer.
 */
#if defined(__SoC__)
# define HAL_RadioTxBufferWrite(phr, psdu)      SOC_RadioTxBufferWrite(phr, psdu)
#
#elif defined(__ML507__)
# define HAL_RadioTxBufferWrite(phr, psdu)      ML507_RadioTxBufferWrite(phr, psdu)
#
#else /* __i386__ */
# define HAL_RadioTxBufferWrite(phr, psdu)      PC_RadioTxBufferWrite(phr, psdu)
#
#endif


/**//**
 * \brief   Performs the RF-TRANSMIT.request for transmission of a PPDU prepared in the
 *  Radio Hardware Frame TX-Buffer.
 * \param[in]   timed       TRUE to appoint the timed transmission at the timestamp
 *  specified with the \p startTime; FALSE to start the transmission immediately.
 * \param[in]   startTime   The timestamp according to the Symbol Counter, in symbol
 *  quotients, at which the timed PPDU transmission shall start. This argument is ignored
 *  for the case of immediate transmission (i.e., if the \p timed equals to FALSE).
 * \details This function starts transmission of a PPDU prepared in the Radio Hardware
 *  Frame TX-Buffer; the packet must be prepared beforehand. This function does not
 *  perform the Radio Hardware transceiver switching from different states to the TX_ON
 *  state; the transceiver must already be in the TX_ON state. This function also does not
 *  perform CSMA-CA algorithm, it just starts transmission.
 * \details In the case of request for immediate transmission \p timed argument must be
 *  FALSE, and \p startTime argument is ignored; and in the case of request for timed
 *  transmission \p timed must be TRUE, and \p startTime specifies the time when to start
 *  transmission. Before arranging the timed transmission, it will be validated if the
 *  timestamp specified is not too close to the current timestamp; and if it is too close
 *  then transmission will be started immediately.
 * \note    For the case of timed transmission, according to particular Radio Hardware
 *  capabilities, HAL will either assign the Radio Hardware with the task to start timed
 *  transmission, or it will arrange the timed transmission with the help of hardware
 *  Symbol Timer and software timed signal handler.
 * \details The moment to start transmission given with \p startTime for the case o timed
 *  transmission defines the timestamp of onset of the first symbol of PPDU preamble
 *  (i.e., the SHR field). The particular implementation of the Radio HAL shall take into
 *  account the actual RX-to-TX time of the Radio transceiver.
 */
#if defined(__SoC__)
# define HAL_RadioTransmitReq(timed, startTime)     SOC_RadioTransmitReq(timed, startTime)
#
#elif defined(__ML507__)
# define HAL_RadioTransmitReq(timed, startTime)     ML507_RadioTransmitReq(timed, startTime)
#
#else /* __i386__ */
# define HAL_RadioTransmitReq(timed, startTime)     PC_RadioTransmitReq(timed, startTime)
#
#endif


/**//**
 * \brief   Performs the RF-CCA.request to the Radio Hardware.
 * \details This function starts the Clear Channel Assessment (CCA) cycle for 8 symbols on
 *  the Radio Hardware receiver. The transmitter must be switched to the RX_ON state
 *  beforehand. In order to prevent eventual reception during the CCA the receiver must be
 *  locked. The CCA will be performed according one of implemented methods according to
 *  the current value of the pyCCAMode attribute. The result of CCA is returned with the
 *  RF-CCA.confirm.
 * \note    On particular platforms CCA may be finished prior to 8-symbol period elapses
 *  if a valid packet is sensed in the media and the phyCCAMode specifies the mode based
 *  on carrier detection. In this case the BUSY status is returned.
 */
#if defined(__SoC__)
# define HAL_RadioCcaReq()      SOC_RadioCcaReq()
#
#elif defined(__ML507__)
# define HAL_RadioCcaReq()      ML507_RadioCcaReq()
#
#else /* __i386__ */
# define HAL_RadioCcaReq()      PC_RadioCcaReq()
#
#endif


#if defined(_ZBPRO_) || defined(RF4CE_TARGET)
/**//**
 * \brief   Performs the RF-ED.request to the Radio Hardware.
 * \details This function starts the Energy Detection (ED) cycle for 8 symbols on the
 *  Radio Hardware receiver. The transmitter must be switched to the RX_ON state
 *  beforehand. In order to prevent eventual reception during the ED the receiver must be
 *  locked. The result of ED is returned with the RF-ED.confirm.
 */
# if defined(__SoC__)
#  define HAL_RadioEdReq()      SOC_RadioEdReq()
#
# elif defined(__ML507__)
#  define HAL_RadioEdReq()      ML507_RadioEdReq()
#
# else /* __i386__ */
#  define HAL_RadioEdReq()      PC_RadioEdReq()
#
# endif
#endif /* _ZBPRO_ || RF4CE_TARGET */


/**//**
 * \brief   Performs the RF-SET-CHANNEL.request to the Radio Hardware.
 * \param[in]   channelOnPage   The 16-bit plain value including the identifier of a
 *  channel page (in the MSB) and the identifier of a logical channel (in the LSB).
 * \details This function switches both the current channel page and the current channel.
 *  Finally it issues the RF-SET-CHANNEL.confirm.
 * \note    The PHY shall validate that the specified Channel-on-Page combination is
 *  implemented by the Radio Hardware. This function does not validate the specified
 *  Channel-on-Page combination; the confirmation issued contains no status information.
 */
#if defined(__SoC__)
# define HAL_RadioSetChannelReq(channelOnPage)      SOC_RadioSetChannelReq(channelOnPage)
#
#elif defined(__ML507__)
# define HAL_RadioSetChannelReq(channelOnPage)      ML507_RadioSetChannelReq(channelOnPage)
#
#else /* __i386__ */
# define HAL_RadioSetChannelReq(channelOnPage)      PC_RadioSetChannelReq(channelOnPage)
#
#endif


/**//**
 * \brief   Switches the Radio Hardware transceiver nominal transmit power value.
 * \param[in]   txPower     Value of transmit power to be assigned, in dBm.
 * \details This function switches the nominal transmit power. Transmit power is measured
 *  in decibels relative to 1 mW, i.e. in dBm. The valid range is from -32 dBm to +31 dBm.
 *  Switching is performed synchronously; no confirmation is issued.
 * \note    The PHY shall validate that the specified transmit power value is supported by
 *  the Radio Hardware. This function does not validate the specified transmit power.
 */
#if defined(__SoC__)
# define HAL_RadioSetTransmitPower(txPower)     SOC_RadioSetTransmitPower(txPower)
#
#elif defined(__ML507__)
# define HAL_RadioSetTransmitPower(txPower)     ML507_RadioSetTransmitPower(txPower)
#
#else /* __i386__ */
# define HAL_RadioSetTransmitPower(txPower)     PC_RadioSetTransmitPower(txPower)
#
#endif


/**//**
 * \brief   Switches the Radio Hardware transceiver CCA mode.
 * \param[in]   ccaMode     Identifier of the CCA Mode to be assigned.
 * \details
 *  This function switches the CCA mode. Switching is performed synchronously; no
 *  confirmation is issued.
 * \note
 *  The PHY shall validate that the specified CCA mode is supported by the Radio Hardware.
 *  This function does not validate the specified CCA mode.
 */
#if defined(__SoC__)
# define HAL_RadioSetCcaMode(ccaMode)       SOC_RadioSetCcaMode(ccaMode)
#
#elif defined(__ML507__)
# define HAL_RadioSetCcaMode(ccaMode)       ML507_RadioSetCcaMode(ccaMode)
#
#else /* __i386__ */
# define HAL_RadioSetCcaMode(ccaMode)       PC_RadioSetCcaMode(ccaMode)
#
#endif


#if defined(RF4CE_TARGET)
/**//**
 * \brief   Returns the current RSSI value measured by the Radio Hardware, in dBm.
 * \return  Value of the RSSI, in dBm.
 */
# if defined(__SoC__)
#  define HAL_RadioGetRssi()        SOC_RadioGetRssi()
#
# elif defined(__ML507__)
#  define HAL_RadioGetRssi()        ML507_RadioGetRssi()
#
# else /* __i386__ */
#  define HAL_RadioGetRssi()        PC_RadioGetRssi()
#
# endif
#endif /* RF4CE_TARGET */


#if defined(_ZBPRO_) && defined(_RF4CE_)

/**//**
 * \brief   Assigns the Radio Hardware Frame Filter with the PAN Coordinator flag.
 * \param[in]   context     Context index, either 0 or 1.
 * \param[in]   panCoord    Value of the PAN Coordinator to be assigned.
 * \note    For the case when the second context of a dual-context application is left
 *  disabled, the corresponding channel of the Hardware Frame Filter is also left
 *  unassigned. It may produce spurious address matches that shall be rejected by the MAC
 *  software frame filter.
 */
# if defined(__SoC__)
#  define HAL_RadioFrameFilterSetPanCoord(context, panCoord)    SOC_RadioFrameFilterSetPanCoord(context, panCoord)
#
# elif defined(__ML507__)
#  define HAL_RadioFrameFilterSetPanCoord(context, panCoord)    ML507_RadioFrameFilterSetPanCoord(context, panCoord)
#
# else /* __i386__ */
#  define HAL_RadioFrameFilterSetPanCoord(context, panCoord)    PC_RadioFrameFilterSetPanCoord(context, panCoord)
#
# endif


/**//**
 * \brief   Assigns the Radio Hardware Frame Filter with the PAN ID value.
 * \param[in]   context     Context index, either 0 or 1.
 * \param[in]   panId       Value of the PAN ID to be assigned.
 * \note    For the case when the second context of a dual-context application is left
 *  disabled, the corresponding channel of the Hardware Frame Filter is also left
 *  unassigned. It may produce spurious address matches that shall be rejected by the MAC
 *  software frame filter.
 */
# if defined(__SoC__)
#  define HAL_RadioFrameFilterSetPanId(context, panId)      SOC_RadioFrameFilterSetPanId(context, panId)
#
# elif defined(__ML507__)
#  define HAL_RadioFrameFilterSetPanId(context, panId)      ML507_RadioFrameFilterSetPanId(context, panId)
#
# else /* __i386__ */
#  define HAL_RadioFrameFilterSetPanId(context, panId)      PC_RadioFrameFilterSetPanId(context, panId)
#
# endif


/**//**
 * \brief   Assigns the Radio Hardware Frame Filter with the Short Address value.
 * \param[in]   context     Context index, either 0 or 1.
 * \param[in]   shortAddr   Value of the Short Address to be assigned.
 * \note    For the case when the second context of a dual-context application is left
 *  disabled, the corresponding channel of the Hardware Frame Filter is also left
 *  unassigned. It may produce spurious address matches that shall be rejected by the MAC
 *  software frame filter.
 */
# if defined(__SoC__)
#  define HAL_RadioFrameFilterSetShortAddr(context, shortAddr)    SOC_RadioFrameFilterSetShortAddr(context, shortAddr)
#
# elif defined(__ML507__)
#  define HAL_RadioFrameFilterSetShortAddr(context, shortAddr)    ML507_RadioFrameFilterSetShortAddr(context, shortAddr)
#
# else /* __i386__ */
#  define HAL_RadioFrameFilterSetShortAddr(context, shortAddr)    PC_RadioFrameFilterSetShortAddr(context, shortAddr)
#
# endif


/**//**
 * \brief   Assigns the Radio Hardware Frame Filter with the Extended Address value.
 * \param[in]   context     Context index, either 0 or 1.
 * \param[in]   extAddr     Value of the Extended Address to be assigned.
 * \note    For the case when the second context of a dual-context application is left
 *  disabled, the corresponding channel of the Hardware Frame Filter is also left
 *  unassigned. It may produce spurious address matches that shall be rejected by the MAC
 *  software frame filter.
 */
# if defined(__SoC__)
#  define HAL_RadioFrameFilterSetExtAddr(context, extAddr)      SOC_RadioFrameFilterSetExtAddr(context, extAddr)
#
# elif defined(__ML507__)
#  define HAL_RadioFrameFilterSetExtAddr(context, extAddr)      ML507_RadioFrameFilterSetExtAddr(context, extAddr)
#
# else /* __i386__ */
#  define HAL_RadioFrameFilterSetExtAddr(context, extAddr)      PC_RadioFrameFilterSetExtAddr(context, extAddr)
#
# endif


#else /* ! ( _ZBPRO_ && _RF4CE_ ) */

/**//**
 * \brief   Assigns the Radio Hardware Frame Filter with the PAN Coordinator flag.
 * \param[in]   panCoord    Value of the PAN Coordinator to be assigned.
 * \note    The PAN Coordinator flag is assigned for both Hardware Frame Filter channels
 *  in spite of single-context application in order to reduce the number of spurious
 *  address matches in the second channel if it would be left unassigned.
 */
# if defined(__SoC__)
#  define HAL_RadioFrameFilterSetPanCoord(panCoord)         SOC_RadioFrameFilterSetPanCoord(panCoord)
#
# elif defined(__ML507__)
#  define HAL_RadioFrameFilterSetPanCoord(panCoord)         ML507_RadioFrameFilterSetPanCoord(panCoord)
#
# else /* __i386__ */
#  define HAL_RadioFrameFilterSetPanCoord(panCoord)         PC_RadioFrameFilterSetPanCoord(panCoord)
#
# endif


/**//**
 * \brief   Assigns the Radio Hardware Frame Filter with the PAN ID value.
 * \param[in]   panId       Value of the PAN ID to be assigned.
 * \note    The PAN ID value is assigned for both Hardware Frame Filter channels in spite
 *  of single-context application in order to reduce the number of spurious address
 *  matches in the second channel if it would be left unassigned.
 */
# if defined(__SoC__)
#  define HAL_RadioFrameFilterSetPanId(panId)       SOC_RadioFrameFilterSetPanId(panId)
#
# elif defined(__ML507__)
#  define HAL_RadioFrameFilterSetPanId(panId)       ML507_RadioFrameFilterSetPanId(panId)
#
# else /* __i386__ */
#  define HAL_RadioFrameFilterSetPanId(panId)       PC_RadioFrameFilterSetPanId(panId)
#
# endif


/**//**
 * \brief   Assigns the Radio Hardware Frame Filter with the Short Address value.
 * \param[in]   shortAddr   Value of the Short Address to be assigned.
 * \note    The Short Address value is assigned for both Hardware Frame Filter channels in
 *  spite of single-context application in order to reduce the number of spurious address
 *  matches in the second channel if it would be left unassigned.
 */
# if defined(__SoC__)
#  define HAL_RadioFrameFilterSetShortAddr(shortAddr)       SOC_RadioFrameFilterSetShortAddr(shortAddr)
#
# elif defined(__ML507__)
#  define HAL_RadioFrameFilterSetShortAddr(shortAddr)       ML507_RadioFrameFilterSetShortAddr(shortAddr)
#
# else /* __i386__ */
#  define HAL_RadioFrameFilterSetShortAddr(shortAddr)       PC_RadioFrameFilterSetShortAddr(shortAddr)
#
# endif


/**//**
 * \brief   Assigns the Radio Hardware Frame Filter with the Extended Address value.
 * \param[in]   extAddr     Value of the Extended Address to be assigned.
 * \note    The Extended Address value is assigned for both Hardware Frame Filter channels
 *  in spite of single-context application in order to reduce the number of spurious
 *  address matches in the second channel if it would be left unassigned.
 */
# if defined(__SoC__)
#  define HAL_RadioFrameFilterSetExtAddr(extAddr)       SOC_RadioFrameFilterSetExtAddr(extAddr)
#
# elif defined(__ML507__)
#  define HAL_RadioFrameFilterSetExtAddr(extAddr)       ML507_RadioFrameFilterSetExtAddr(extAddr)
#
# else /* __i386__ */
#  define HAL_RadioFrameFilterSetExtAddr(extAddr)       PC_RadioFrameFilterSetExtAddr(extAddr)
#
# endif

#endif /* ! ( _ZBPRO_ && _RF4CE_ ) */


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Issues the RF-SET-TRX-STATE.confirm to the PHY from the Radio HAL for a just
 *  finished transceiver state switching.
 * \param[in]   trxState    Current state of the Radio transceiver. The following states
 *  may be confirmed: TRX_OFF, RX_ON, TX_ON. This parameter is used only for the Debug
 *  build.
 * \details
 *  Call this function from the specific Radio hardware real-time interrupt request
 *  handler in the case of TRX_STATE_SWITCHING_COMPLETE signal.
 * \details
 *  The PHY shall provide implementation for this callback handler-function. The HAL calls
 *  this function to signal the end of the last request to switch the transceiver state.
*****************************************************************************************/
#if defined(_DEBUG_COMPLEX_)
HAL_PUBLIC void HAL_RadioSetTrxStateConf(const HAL_RadioStateId_t trxState);
#else
HAL_PUBLIC void HAL_RadioSetTrxStateConf(void);
#endif


/*************************************************************************************//**
 * \brief   Issues the RF-RECEIVE.indication to the PHY from the Radio HAL for a just
 *  received PPDU.
 * \param[in]   startTime   The timestamp according to the Symbol Counter, in symbol
 *  quotients, at which the reception being indicated has actually started.
 * \param[in]   endTime     The timestamp according to the Symbol Counter, in symbol
 *  quotients, at which the reception being indicated has actually ended.
 * \details
 *  The moment of reception start, contained in \p startTime, is bound to the onset of the
 *  PPDU.PHR field (i.e., to the fallout of the PPDU.SHR.SFD field) of the received
 *  packet.
 * \details
 *  The moment of reception end, contained in \p endTime, is bound to the fallout of the
 *  last symbol of the PPDU of the received packet.
 * \details
 *  Call this function from the specific Radio hardware real-time interrupt request
 *  handler in the case of RX_COMPLETE signal.
 * \details
 *  The PHY shall provide implementation for this callback handler-function. The HAL calls
 *  this function to signal the end of reception of a PHY packet.
*****************************************************************************************/
HAL_PUBLIC void HAL_RadioReceiveInd(const HAL_SymbolTimestamp_t startTime, const HAL_SymbolTimestamp_t endTime);


/*************************************************************************************//**
 * \brief   Issues the RF-TRANSMIT.confirm to the PHY from the Radio HAL for a just
 *  transmitted PPDU.
 * \param[in]   startTime   The timestamp according to the Symbol Counter, in symbol
 *  quotients, at which the transmission being confirmed has actually started.
 * \param[in]   endTime     The timestamp according to the Symbol Counter, in symbol
 *  quotients, at which the transmission being confirmed has actually ended.
 * \details
 *  The moment of transmission start, contained in \p startTime, is bound to the onset of
 *  the PPDU.PHR field (i.e., to the fallout of the PPDU.SHR.SFD field) of the transmitted
 *  packet.
 * \details
 *  The moment of transmission end, contained in \p endTime, is bound to the fallout of
 *  the last symbol of the PPDU of the transmitted packet.
 * \details
 *  Call this function from the Radio hardware real-time interrupt request handler in the
 *  case of TX_COMPLETE signal.
 * \details
 *  The PHY shall provide implementation for this callback handler-function. The HAL calls
 *  this function to signal the end of transmission of a packet.
*****************************************************************************************/
HAL_PUBLIC void HAL_RadioTransmitConf(const HAL_SymbolTimestamp_t startTime, const HAL_SymbolTimestamp_t endTime);


/*************************************************************************************//**
 * \brief   Issues the RF-CCA.confirm to the PHY from the Radio HAL for a just finished
 *  CCA cycle.
 * \param[in]   status      The result of the request to perform a CCA.
 * \details
 *  Call this function from the Radio hardware real-time interrupt request handler in the
 *  case of CCA_COMPLETE signal.
 * \details
 *  The PHY shall provide implementation for this callback handler-function. The HAL calls
 *  this function to signal the end of a single CCA cycle.
*****************************************************************************************/
HAL_PUBLIC void HAL_RadioCcaConf(const HAL_RadioCcaStatusId_t status);


#if defined(_ZBPRO_) || defined(RF4CE_TARGET)
/*************************************************************************************//**
 * \brief   Issues the RF-ED.confirm to the PHY from the Radio HAL for a just finished ED
 *  measurement.
 * \param[in]   energyLevel     The ED level for the current channel.
 * \details
 *  Call this function from the Radio hardware real-time interrupt request handler in the
 *  case of ED_COMPLETE signal.
 * \details
 *  The PHY shall provide implementation for this callback handler-function. The HAL calls
 *  this function to signal the end of a single ED measurement.
*****************************************************************************************/
HAL_PUBLIC void HAL_RadioEdConf(const HAL_RadioEd_t energyLevel);
#endif /* _ZBPRO_ || RF4CE_TARGET */


/*************************************************************************************//**
 * \brief   Issues the RF-SET-CHANNEL.confirm to the PHY from the Radio HAL for a just
 *  finished channel switching.
 * \details
 *  Call this function from the Radio hardware real-time interrupt request handler in the
 *  case of SET_CHANNEL_COMPLETE signal.
 * \details
 *  The PHY shall provide implementation for this callback handler-function. The HAL calls
 *  this function to signal the end of channel switching.
*****************************************************************************************/
HAL_PUBLIC void HAL_RadioSetChannelConf(void);


/************************* INCLUDES *****************************************************/
#if defined(__SoC__)
# include "bbSocRadio.h"            /* SoC Radio Hardware interface. */
#elif defined(__ML507__)
# include "bbMl507Radio.h"          /* ML507 Radio Hardware interface. */
#else /* __i386__ */
# include "bbPcRadio.h"             /* i386 Radio Simulator interface. */
#endif


#endif /* _BB_HAL_RADIO_H */
