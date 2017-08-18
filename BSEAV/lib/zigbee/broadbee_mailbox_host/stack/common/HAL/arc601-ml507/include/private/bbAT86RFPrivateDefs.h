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
 *      Atmel AT86RF Radio Hardware private definitions.
 *
*******************************************************************************/

#ifndef _BB_AT86RF_PRIVATE_DEFS_H
#define _BB_AT86RF_PRIVATE_DEFS_H

/************************* INCLUDES ***********************************************************************************/
#include "bbSysTypes.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \name    Enumeration of the AT86RF Radio SPI protocol commands.
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 6.2, table 6-2.
 */
enum AT86RF_SPI__CMD_Code_t {
    AT86RF_SPI_CMD__FRM_READ    = 0x20,     /*!< Read the Frame Buffer. */
    AT86RF_SPI_CMD__FRM_WRITE   = 0x60,     /*!< Write the Frame Buffer. */
    AT86RF_SPI_CMD__REG_READ    = 0x80,     /*!< Read the specified Register. */
    AT86RF_SPI_CMD__REG_WRITE   = 0xC0,     /*!< Write the specified Register. */
};

/**//**
 * \name    AT86RF Radio Registers.
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 14.
 */
/**@{*/
/**//**
 * \brief   The TRX_STATUS register signals the current AT86RF Radio transceiver state and the status of the CCA
 *  measurement.
 * \details The TRX_STATUS value contains the following fields:
 *  - bits 4..0 - TRX_STATUS[4..0] - Radio Transceiver Status (read only).
 *  - bit 5 - Reserved[0].
 *  - bit 6 - CCA_STATUS[0] - CCA Status Result (read only): 0 - busy, 1 - idle.
 *  - bit 7 - CCA_DONE[0] - CCA Algorithm Status (read only): 0 - in progress, 1 - finished.
 *
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclauses 7.1.5, 8.5.6, figure 7-1, tables 7-3, 8-10, 8-11.
 */
#define AT86RF_REG__TRX_STATUS      (0x01)

/**//**
 * \brief   The TRX_STATE register controls the AT86RF Radio transceiver states by receiving the state transition
 *  commands and reports the status of the RX_AACK and TX_ARET procedures in the Extended Operating Mode.
 * \details The TRX_STATE value contains the following fields:
 *  - bits 4..0 - TRX_CMD[4..0] - State Control Command.
 *  - bits 7..5 - TRAC_STATUS[2..0] - The status of the RX_AACK and TX_ARET procedures (read only).
 *
 * \note    It's not necessary to apply the AND-mask when assigning the TRX_CMD field, because the TRAC_STATUS field is
 *  read-only.
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 7.1.5, figure 7-1, tables 7-4, 7-16.
 */
#define AT86RF_REG__TRX_STATE       (0x02)

/**//**
 * \brief   The TRX_CTRL_0 register controls the AT86RF Radio drive current of the digital output pads and the CLKM
 *  clock rate.
 * \details The TRX_CTRL_0 value contains the following fields:
 *  - bits 2..0 - CLKM_CTRL[2..0] - Clock Rate Setting at pin CLKM.
 *  - bit 3 - CLKM_SHA_SEL[0] - CLKM Clock Rate Update Scheme.
 *  - bits 5..4 - PAD_IO_CLKM[1..0] - CLKM Driver Strength.
 *  - bits 7..6 - PAD_IO[1..0] - Digital Output Driver Strength.
 *
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclauses 1.3, 9.6.4, 9.6.5, tables 1-3...1-6, 9-12...9-14.
 */
#define AT86RF_REG__TRX_CTRL_0      (0x03)

/**//**
 * \brief   The TRX_CTRL_1 register of the AT86RF Radio is a multi-purpose register to control various operating modes
 *  and settings of the radio transceiver.
 * \details The TRX_CTRL_1 value contains the following fields:
 *  - bit 0 - IRQ_POLARITY[0] - Configuration of Pin 24 (IRQ).
 *  - bit 1 - IRQ_MASK_MODE[0] - Interrupt Polling Configuration.
 *  - bits 3..2 - SPI_CMD_MODE[1..0] - Radio Transceiver Status Information - PHY_STATUS.
 *  - bit 4 - RX_BL_CTRL[0] - Frame Buffer Empty Indicator.
 *  - bit 5 - TX_AUTO_CRC_ON[0] - Controls the automatic FCS generation for TX operations.
 *  - bit 6 - IRQ_2_EXT_EN[0] - RX Frame Time Stamping Mode for pin 10 (DIG2).
 *  - bit 7 - PA_EXT_EN[0] - RF Front-End Control Pins (DIG3, DIG4).
 *
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclauses 6.3, 6.6, 8.2.3, 8.2.5, 11.5, 11.6, 11.7, figures 11-11...11-13,
 *  tables 6-3, 6-9...6-11, 11-15.
 */
#define AT86RF_REG__TRX_CTRL_1      (0x04)

/**//**
 * \brief   The PHY_TX_PWR register controls the AT86RF Radio output power and the ramping of the transmitter.
 * \details The PHY_TX_PWR value contains the following fields:
 *  - bits 3..0 - TX_PWR[3..0] - TX Output Power Setting.
 *  - bits 5..4 - PA_LT[1..0] - PA Enable Time Relative to the Start of the Frame (SHR).
 *  - bits 7..6 - PA_BUF_LT[1..0] - PA Buffer Enable Time Relative to the PA.
 *
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclauses 9.2.4, 9.2.5, figures 9-3, tables 9-2...9-4.
 */
#define AT86RF_REG__PHY_TX_PWR      (0x05)

/**//**
 * \brief   The PHY_RSSI register is a multi purpose register that indicates FCS validity, provides random numbers and
 *  shows the actual RSSI value.
 * \details The PHY_RSSI value contains the following fields:
 *  - bits 4..0 - RSSI[4..0] - The result of the automated RSSI measurement.
 *  - bits 6..5 - RND_VALUE[1..0] - The 2-bit random value.
 *  - bit 7 - RX_CRC_VALID[0] - Indicates whether the last received frame has a valid FCS or not.
 *
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclauses 8.2, 8.3, 11.2, figure 8-4.
 */
#define AT86RF_REG__PHY_RSSI        (0x06)

/**//**
 * \brief   The PHY_ED_LEVEL register contains the result of an ED measurement.
 * \details The PHY_ED_LEVEL value contains the following fields:
 *  - bits 7..0 - ED_LEVEL[7] - Energy Detection level.
 *
 * \details Write arbitrary value into this register to start ED measurement.
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 8.4, figure 8-5, table 8-7.
 */
#define AT86RF_REG__PHY_ED_LEVEL    (0x07)

/**//**
 * \brief   The PHY_CC_CCA register of the AT86RF Radio is provided to initiate and control a CCA measurement and set
 *  the IEEE Std. 802.15.4-2006/2003 - 2.4 GHz channel number.
 * \details The PHY_CC_CCA value contains the following fields:
 *  - bits 4..0 - CHANNEL[4..0] - Defines the RX/TX channel.
 *  - bits 6..5 - CCA_MODE[1..0] - CCA Mode.
 *  - bit 7 - CCA_REQUEST[0] - Manual request for CCA measurement.
 *
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclauses 8.5, 9.7, tables 8-8, 8-12, 9-17.
 */
#define AT86RF_REG__PHY_CC_CCA      (0x08)

/**//**
 * \brief   The TRX_CTRL_2 register of the AT86RF Radio is a multi-purpose register to control various settings of the
 *  radio transceiver.
 * \details The TRX_CTRL_2 value contains the following fields:
 *  - bits 1..0 - OQPSK_DATA_RATE[1..0] - OQPSK Data Rate.
 *  - bits 6..2 - Reserved[4..0].
 *  - bit 7 - RX_SAFE_MODE[0] - Dynamic Frame Buffer Protection Mode.
 *
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclauses 11.3, 11.8, tables 11-8, 11-9, 11-17.
 */
#define AT86RF_REG__TRX_CTRL_2      (0x0C)

/**//**
 * \brief   The IRQ_MASK register of the AT86RF Radio is used to enable or disable individual interrupts.
 * \details An interrupt is enabled if the corresponding bit is set to 1.
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 6.6, table 6-9.
 */
#define AT86RF_REG__IRQ_MASK        (0x0E)

/**//**
 * \brief   The IRQ_STATUS register of the AT86RF Radio contains the status of the pending interrupt requests.
 * \details By reading the register after an interrupt is signaled at pin 24 (IRQ) the source of the issued interrupt
 *  can be identified. A read access to this register resets all interrupt bits, and so clears the IRQ_STATUS register.
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 6.6, table 6-9.
 */
#define AT86RF_REG__IRQ_STATUS      (0x0F)
/**@}*/

/**//**
 * \name    Enumeration of the AT86RF Radio transceiver statuses in the Basic Operating Mode.
 * \details The following states originate form conventional IEEE Std. 802.15.4-2006(2003) states:
 *  - BUSY_RX (0x01) - radio has received SHR of a packet and is receiving the remainder of the packet.
 *  - BUSY_TX (0x02) - radio is transmitting a packet.
 *  - RX_ON (0x06) - radio is ready to receive but is not receiving currently.
 *  - TRX_OFF (0x08) - radio is switched off.
 *  - TX_ON (0x09) - radio is ready to transmit but is not transmitting currently.
 *
 * \details The following codes were added by the Radio vendor:
 *  - P_ON (0x00) - radio was just switched on and is waiting for a command to enter the TRX_OFF.
 *  - IN_PROGRESS (0x1F) - state changing is currently in progress.
 *
 * \note    Do not try to initiate a further state change while the radio transceiver is in IN_PROGRESS (0x1F) state.
 * \note    Only the actually used codes are listed.
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 7.1.5, figure 7-1, table 7-3.<\br>
 *  See IEEE 802.15.4-2006, subclause 6.2.3, table 18.
 */
enum AT86RF_TRX__STATUS_Code_t {
    AT86RF_TRX_STATUS__P_ON             = 0x00,     /*!< Power-On state after VDD is switched on. */
    AT86RF_TRX_STATUS__BUSY_RX          = 0x01,     /*!< BUSY_RX state. */
    AT86RF_TRX_STATUS__BUSY_TX          = 0x02,     /*!< BUSY_TX state. */
    AT86RF_TRX_STATUS__RX_ON            = 0x06,     /*!< RX_ON state. */
    AT86RF_TRX_STATUS__TRX_OFF          = 0x08,     /*!< TRX_OFF state. */
    AT86RF_TRX_STATUS__TX_ON            = 0x09,     /*!< TX_ON state. */
    AT86RF_TRX_STATUS__IN_PROGRESS      = 0x1F,     /*!< Transceiver is in the middle of a state transition. */
};

/**//**
 * \name    Enumeration of the AT86RF Radio state control commands in the Basic Operating Mode.
 * \details The following commands originate form conventional IEEE Std. 802.15.4-2006(2003) commands:
 *  - FORCE_TRX_OFF (0x03) - switch (force) the radio to TRX_OFF immediately.
 *  - FORCE_TX_ON (0x04) - switch (force) the radio to TX_ON immediately (comply with TX_ON definition having the code
 *      0x09 given in IEEE Std. 802.15-4-2006).
 *  - TRX_OFF (0x08) - switch the radio to TRX_OFF if it's idle, postpone if it'is in BUSY_RX or BUSY_TX.
 *  - RX_ON (0x06) - switch the radio to RX_ON if it's idle, postpone if it'is in BUSY_TX.
 *  - TX_ON (0x09) - switch the radio to TX_ON if it's idle, postpone if it'is in BUSY_RX (comply with TX_ON definition
 *      given in IEEE Std. 802.15-4-2003, but not the 2006).
 *
 * \details The following codes were added by the Radio vendor:
 *  - NOP (0x00) - no operation. Keep the current state.
 *
 * \note    The command FORCE_TX_ON (0x04) must not be used when the radio is in P_ON, SLEEP, RESET, TRX_OFF, and all
 *  XXX_NOCLK states, as well as IN_PROGRESS towards these states.
 * \note    Only the actually used codes are listed.
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 7.1.5, figure 7-1, table 7-4.<\br>
 *  See IEEE 802.15.4-2006, subclause 6.2.3, table 18.
 */
enum AT86RF_TRX__CMD_Code_t {
    AT86RF_TRX_CMD__NOP             = 0x00,     /*!< No operation. */
    AT86RF_TRX_CMD__FORCE_TRX_OFF   = 0x03,     /*!< Force TRX_OFF state. */
    AT86RF_TRX_CMD__FORCE_TX_ON     = 0x04,     /*!< Force TX_ON state. */
    AT86RF_TRX_CMD__RX_ON           = 0x06,     /*!< Switch to RX_ON state. */
    AT86RF_TRX_CMD__TRX_OFF         = 0x08,     /*!< Switch to TRX_OFF state. */
    AT86RF_TRX_CMD__TX_ON           = 0x09,     /*!< Switch to TX_ON state. */
};

/**//**
 * \name    Enumeration of the AT86RF Radio interrupt sources in Basic Operating Mode.
 * \note    The IRQ_4 is the multi-functional interrupt: AWAKE_END and CCA_ED_DONE - the AWAKE_END signal is triggered
 *  on the entering TRX_OFF (0x08) from the P_ON (0x00) state.
 * \par     Documentation
 *  See Atmel 8111C-MCU Wireless-09/09, subclause 6.6, table 6-9.
 */
enum AT86RF_IRQ__Source_t {
    AT86RF_IRQ__PLL_LOCK        = 0,    /*!< IRQ_0. Indicates PLL lock. */
    AT86RF_IRQ__PLL_UNLOCK      = 1,    /*!< IRQ_1. Indicates PLL unlock. */
    AT86RF_IRQ__RX_START        = 2,    /*!< IRQ_2. Indicates the start of a PSDU reception. The TRX_STATE changes to
                                            BUSY_RX, the PHR is valid to read from Frame Buffer. */
    AT86RF_IRQ__TRX_END         = 3,    /*!< IRQ_3. Indicates the completion of a frame reception/transmission. */
    AT86RF_IRQ__AWAKE_END       = 4,    /*!< IRQ_4. Indicates radio transceiver reached TRX_OFF state after P_ON, RESET,
                                            or SLEEP states. */
    AT86RF_IRQ__CCA_ED_DONE     = 4,    /*!< IRQ_4. Indicates the end of a CCA or ED measurement. */
    AT86RF_IRQ__AMI             = 5,    /*!< IRQ_5. Indicates address matching. */
    AT86RF_IRQ__TRX_UR          = 6,    /*!< IRQ_6. Indicates a Frame Buffer access violation. */
    AT86RF_IRQ__BAT_LOW         = 7,    /*!< IRQ_7. Indicates a supply voltage below the programmed threshold. */
};

#endif /* _BB_AT86RF_PRIVATE_DEFS_H */

/* eof bbAT86RFPrivateDefs.h */