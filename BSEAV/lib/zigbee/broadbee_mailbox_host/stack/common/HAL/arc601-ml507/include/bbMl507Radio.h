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
* FILENAME: $Workfile: trunk/stack/common/HAL/arc601-ml507/include/bbMl507Radio.h $
*
* DESCRIPTION:
*   ML507 with external AT86RF231 Radio Driver interface addons.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
*****************************************************************************************/

#ifndef _BB_ML507_RADIO_H
#define _BB_ML507_RADIO_H

/************************* INCLUDES ***********************************************************************************/
#include "bbSysBasics.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   Number of channel pages implemented.
 */
#define HAL_RADIO__PAGES_NUM        (1)

/**//**
 * \name    IEEE 5-bit identifiers of implemented channel pages.
 * \note    Pages are enumerated sequentially form 0. Page index is not the same with the IEEE identifier of the page.
 */
/**@{*/
#define HAL_RADIO__PAGE_0           (0)         /*!< The page[0] is the page '0'. */
#define HAL_RADIO__PAGE_1                       /*!< The page[1] is not implemented. */
#define HAL_RADIO__PAGE_2                       /*!< The page[2] is not implemented. */
/**@}*/

/**//**
 * \brief   IEEE 5-bit identifier of the default channel page.
 * \note    The default page must be one of the implemented pages.
 */
#define HAL_RADIO__PAGE_DEF         (0)

/**//**
 * \name    Ranges of implemented channels on different pages.
 * \note    Pages are enumerated sequentially form 0. Page index is not the same with the IEEE identifier of the page.
 */
/**@{*/
#define HAL_RADIO__CHANNEL_MIN_ON_PAGE_0        (11)        /*!< First channel on the page[0]. */
#define HAL_RADIO__CHANNEL_MAX_ON_PAGE_0        (26)        /*!< Last channel on the page[0]. */
#define HAL_RADIO__CHANNEL_MIN_ON_PAGE_1                    /*!< The page[1] is not implemented. */
#define HAL_RADIO__CHANNEL_MAX_ON_PAGE_1                    /*!< The page[1] is not implemented. */
#define HAL_RADIO__CHANNEL_MIN_ON_PAGE_2                    /*!< The page[2] is not implemented. */
#define HAL_RADIO__CHANNEL_MAX_ON_PAGE_2                    /*!< The page[2] is not implemented. */
/**@}*/

/**//**
 * \name    Sets of implemented channels on different pages.
 * \note    Pages are enumerated sequentially form 0. Page index is not the same with the IEEE identifier of the page.
 */
/**@{*/
#define HAL_RADIO__CHANNELS_ON_PAGE_0       (0x07FFF800)    /*!< Channels implemented on the page[0]. */
#define HAL_RADIO__CHANNELS_ON_PAGE_1                       /*!< Channels implemented on the page[1]. */
#define HAL_RADIO__CHANNELS_ON_PAGE_2                       /*!< Channels implemented on the page[2]. */
/**@}*/

/**//**
 * \brief   Default channel.
 * \note    The default channel must be one of the implemented channels on the default page.
 */
#define HAL_RADIO__CHANNEL_DEF      (11)

/**//**
 * \name    Limits and default value for the Radio CCA Mode.
 * \details Allowed codes of CCA mode are the following:
 *  - 0 - Mode 3-OR, Carrier sense OR energy above threshold
 *  - 1 - Mode 1, Energy above threshold
 *  - 2 - Mode 2, Carrier sense only
 *  - 3 - Mode 3-AND, Carrier sense AND energy above threshold
 */
/**@{*/
#define HAL_RADIO__CCA_MODE_MIN     (PHY_CCA__MODE_3_OR)    /*!< Minimum code value of implemented Radio CCA mode. */
#define HAL_RADIO__CCA_MODE_MAX     (PHY_CCA__MODE_3_AND)   /*!< Maximum code value of implemented Radio CCA mode. */
#define HAL_RADIO__CCA_MODE_DEF     (PHY_CCA__MODE_1)       /*!< Default code value of Radio CCA mode. */
/**@}*/

/**//**
 * \name    Limits, tolerance and default value for the Radio transceiver nominal Transmit Power, in dBm.
 * \details Allowed values for transmit power are (-32..+31) dbm.
 * \note    The DEFAULT value corresponds to the power-on configuration of the AT86RF radio and must not be changed.
 */
/**@{*/
#define HAL_RADIO__TX_POWER_MIN     (-17)       /*!< Minimum implemented Radio Transmitter power value. */
#define HAL_RADIO__TX_POWER_MAX     (+3)        /*!< Maximum implemented Radio Transmitter power value. */
#define HAL_RADIO__TX_POWER_DEF     (+3)        /*!< Default value of the Radio Transmitter power. */
#define HAL_RADIO__TX_POWER_TOL     (1)         /*!< Tolerance of the Radio Transmitter power value. */
/**@}*/

/**//**
 * \name    Configuration constants used for conversions of Energy Detection (ED) values reported by the Driver in their
 *  hardware-specific format into the hardware-independent format.
 * \details The ED' value expressed in the native AT86RF231 format receives values in the range from 0x00 to 0x54. It
 *  may be converted to ED expressed in decibels as follows:<\br>
 *  ED [dBm] = -91 + 1 * ED'
 * \note    Use conversion function provided by the hardware-independent Radio Driver interface.
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 8.4, figure 8-5.
 */
/**@{*/
#define PHY_SCALE__ED_1_to_dBm          (+1)        /*!< The scale factor. */
#define PHY_SCALE__ED_0_to_dBm          (-91)       /*!< The constant shift term. */
/**@}*/

/**//**
 * \name    Configuration constants used for conversions of Received Signal Strength Indicator (RSSI) values reported by
 *  the Driver in their hardware-specific format into the hardware-independent format.
 * \details The RSSI' value expressed in the native AT86RF231 format receives values in the range from 0x00 to 0x1C. It
 *  may be converted to RSSI expressed in decibels as follows:<\br>
 *  RSSI [dBm] = -91 + 3 * (RSSI' - 1) = -94 + 3 * RSSI'
 * \note    Use conversion function provided by the hardware-independent Radio Driver interface.
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 8.2, figure 8-5.
 */
/**@{*/
#define PHY_SCALE__RSSI_1_to_dBm        (+3)        /*!< The scale factor. */
#define PHY_SCALE__RSSI_0_to_dBm        (-94)       /*!< The constant shift term. */
/**@}*/

/**//**
 * \name    Set of macro-functions used for configuring the Radio hardware frame filter.
 * \note    The ML507 with external AT86RF231 Radio do not use hardware frame filter. These functions are just stubs for
 *  compatibility with the SoC platform.
 */
/**@{*/
# define HAL_RadioFrameFilterSetPanCoord(...)       while(0)
# define HAL_RadioFrameFilterSetPanId(...)          while(0)
# define HAL_RadioFrameFilterSetShortAddr(...)      while(0)
# define HAL_RadioFrameFilterSetExtAddr(...)        while(0)
/**@}*/

#endif /* _BB_ML507_RADIO_H */
