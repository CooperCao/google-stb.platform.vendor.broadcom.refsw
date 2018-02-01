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
 *      PLME-ED and PLME-CCA services data types definition.
 *
*******************************************************************************/

#ifndef _BB_PHY_SAP_TYPES_ENERGY_H
#define _BB_PHY_SAP_TYPES_ENERGY_H

/************************* INCLUDES ***********************************************************************************/
#include "bbPhyBasics.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \name    Data types used for estimation of the radio link quality and media conditions.
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
typedef HAL_Radio__ED_t  PHY_ED_t;
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
typedef HAL_Radio__RSSI_t  PHY_RSSI_t;
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
typedef HAL_Radio__LQI_t  PHY_LQI_t;
SYS_DbgAssertStatic(sizeof(PHY_LQI_t) == 1);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    Macro-constants used for estimation of the radio link quality and media conditions.
 */
/**@{*/
/**//**
 * \brief   The maximum possible value of Energy Detection (ED) expressed in the raw format.
 * \details This value is intended to be used as the invalid or unknown value of ED.
 * \details The ED result is an integer ranging from 0x00 to 0xFF.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.4, 6.9.7, table 11.
 */
#define PHY_ED_MAX      (HAL_RADIO__ED_MAX)
/**@}*/

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \name    Macro-functions used for estimation of the radio link quality and media conditions.
 */
/**@{*/
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
 * \name    Data types used for the PHY configuration.
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

#ifdef _HOST_
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wenum-compare"
#endif
SYS_DbgAssertStatic(PHY_CCA__MODE_3_OR == HAL_RADIO_CCA__MODE_3_OR);
SYS_DbgAssertStatic(PHY_CCA__MODE_1 == HAL_RADIO_CCA__MODE_1);
SYS_DbgAssertStatic(PHY_CCA__MODE_2 == HAL_RADIO_CCA__MODE_2);
SYS_DbgAssertStatic(PHY_CCA__MODE_3_AND == HAL_RADIO_CCA__MODE_3_AND);
#ifdef _HOST_
#pragma GCC diagnostic pop
#endif

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
typedef HAL_Radio__TX_power_t  PHY_TXPower_t;
SYS_DbgAssertStatic(sizeof(PHY_TXPower_t) == 1);
/**@}*/

#endif /* _BB_PHY_SAP_TYPES_ENERGY_H */

/* eof bbPhySapTypesEnergy.h */