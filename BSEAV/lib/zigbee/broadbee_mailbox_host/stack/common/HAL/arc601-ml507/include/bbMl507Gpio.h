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
* FILENAME: $Workfile: trunk/stack/common/HAL/arc601-ml507/include/bbMl507Gpio.h $
*
* DESCRIPTION:
*   ML507 GPIO interface.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
*****************************************************************************************/

#ifndef _BB_ML507_GPIO_H
#define _BB_ML507_GPIO_H

/************************* INCLUDES ***********************************************************************************/
#include "bbSysTypes.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \name    ML507 GPIO unit registers.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclauses GPIO ON FPGA, PORT PINS ON ML507.
 */
/**@{*/
/**//**
 * \brief   ML507 GPIO registers base.
 * \details The BASE address is used in conjunction with all the rest macros (used as indexes) to obtain absolute
 *  addresses of corresponding registers.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclauses GPIO ON FPGA.
 */
#define ML507_REG__GPIO_BASE        (0x00C00014)

/**//**
 * \brief   ML507 GPIO Outputs State register, shift to 8-bit base.
 * \details The GPIO_DOUT value contains the following fields:
 *  - bits 3..0 - DOUT[3..0] - Force the GPIO output pins #3..0.
 *  - bits 7..4 - Reserved[3..0].
 *
 * \details The state of a GPIO pin repeats the value of the corresponding DOUT bit: value 0 of a bit drives its pin to
 *  L, value 1 drives the pin to H.
 * \details For a GPIO pin to effectively work as the output, the corresponding bit in the ENABLE register must be set;
 *  otherwise a GPIO works only for input.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclauses GPIO ON FPGA, PORT PINS ON ML507.
 */
#define ML507_REG__GPIO_DOUT        ((0x00C00014 - ML507_REG__GPIO_BASE) / 1)

/**//**
 * \brief   ML507 GPIO Outputs Enable register, shift to 8-bit base.
 * \details The GPIO_ENABLE value contains the following fields:
 *  - bits 3..0 - ENABLE[3..0] - Configuration of the GPIO input pins #3..0.
 *  - bits 7..4 - Reserved[3..0].
 *
 * \details A GPIO pin becomes the output pin when the corresponding bit in the ENABLE register is set; otherwise a GPIO
 *  pin works only for input.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclauses GPIO ON FPGA, PORT PINS ON ML507.
 */
#define ML507_REG__GPIO_ENABLE      ((0x00C00015 - ML507_REG__GPIO_BASE) / 1)

/**//**
 * \brief   ML507 GPIO Inputs State register, shift to 8-bit base.
 * \details The GPIO_DIN value contains the following fields:
 *  - bits 3..0 - DIN[3..0] - State of the GPIO input pins #3..0.
 *  - bits 7..4 - Reserved[3..0].
 *
 * \details The value of a DIN bit repeats the state of the corresponding GPIO pin: state L of a pin forces its bit to
 *  0, state H forces the bit to 1.
 * \details All the GPIO pins work as inputs even if they are configured as outputs (in the latter case the input state
 *  of a pin repeats its output state).
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclauses GPIO ON FPGA, PORT PINS ON ML507.
 */
#define ML507_REG__GPIO_DIN         ((0x00C00016 - ML507_REG__GPIO_BASE) / 1)
/**@}*/

/**//**
 * \name    Enumeration of the ML507 GPIO, designation for AT86RF Radio pins and other purposes.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclause AT86RF231 TRANSCEIVER CONNECTION, PORT PINS ON
 *  ML507.<\br>
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 1.1, table 1-1.
 */
enum ML507_GPIO__Port_t {
    ML507_GPIO__AUX_0           = 0,    /*!< ML507 GPIO #0 is used as the auxiliary port #0. */
    ML507_GPIO__AUX_1           = 1,    /*!< ML507 GPIO #1 is used as the auxiliary port #1. */
    ML507_GPIO__AT86RF_SLPTR    = 2,    /*!< ML507 GPIO #2 is designated for SLP_TR pin of AT86RF. */
    ML507_GPIO__AT86RF_NRST     = 3,    /*!< ML507 GPIO #3 is designated for /RST pin of AT86RF. */
};

/**//**
 * \name    Set of macro-functions for manipulating auxiliary GPIO.
 */
/**@{*/
/**//**
 * \brief   Enable auxiliary GPIO to work as outputs.
 */
#define ML507_GPIO__Aux_Enable()\
        do { HAL_ATOMIC_START {\
            reg8_t *const gpio = SYS_REG8(ML507_REG__GPIO_BASE);\
            gpio[ML507_REG__GPIO_ENABLE] &= ~((1 << ML507_GPIO__AUX_0) | (1 << ML507_GPIO__AUX_1));\
            gpio[ML507_REG__GPIO_DOUT] &= ~((1 << ML507_GPIO__AUX_0) | (1 << ML507_GPIO__AUX_1));\
            gpio[ML507_REG__GPIO_ENABLE] |= (1 << ML507_GPIO__AUX_0) | (1 << ML507_GPIO__AUX_1);\
        } HAL_ATOMIC_END } while(0)

/**//**
 * \brief   Set the auxiliary GPIO #0 state.
 * \param[in]   state       State to be assigned to GPIO #0.
 */
#define ML507_GPIO__Aux_0(state)\
        do { HAL_ATOMIC_START {\
            reg8_t *const gpio = SYS_REG8(ML507_REG__GPIO_BASE);\
            gpio[ML507_REG__GPIO_DOUT] =\
                    (gpio[ML507_REG__GPIO_DOUT] & ~(1 << ML507_GPIO__AUX_0)) | ((state) << ML507_GPIO__AUX_0);\
        } HAL_ATOMIC_END } while(0)

/**//**
 * \brief   Set the auxiliary GPIO #1 state.
 * \param[in]   state       State to be assigned to GPIO #1.
 */
#define ML507_GPIO__Aux_1(state)\
        do { HAL_ATOMIC_START {\
            reg8_t *const gpio = SYS_REG8(ML507_REG__GPIO_BASE);\
            gpio[ML507_REG__GPIO_DOUT] =\
                    (gpio[ML507_REG__GPIO_DOUT] & ~(1 << ML507_GPIO__AUX_1)) | ((state) << ML507_GPIO__AUX_1);\
        } HAL_ATOMIC_END } while(0)
/**@}*/

#endif /* _BB_ML507_GPIO_H */
