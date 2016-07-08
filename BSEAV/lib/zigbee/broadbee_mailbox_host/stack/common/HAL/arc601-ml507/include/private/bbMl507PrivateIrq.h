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
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   ML507 IRQ private interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/

#ifndef _BB_ML507_PRIVATE_IRQ_H
#define _BB_ML507_PRIVATE_IRQ_H

/************************* INCLUDES ***********************************************************************************/
#include "bbSysTypes.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   ML507 IRQ4 (compound Radio /IRQ) Clear register.
 * \details The IRQ4 is triggered on an external event on the ML507 J6-2 pin which is connected to the AT86RF /IRQ line
 *  (the compound Radio /IRQ configured to active LO). The ML507 implements logic on the J6-2 input: it triggers the
 *  IRQ4 on the falling edge of the external Radio /IRQ and stays active until reset by the software with the help of
 *  the IRQ4_NCLR register. To clear the IRQ4 one has to assert IRQ4_NCLR = 0. There is no need to set IRQ4_NCLR back to
 *  a nonzero value.
 * \par     Documentation
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclauses INTERRUPT CONNECTIONS TO CPU ON FPGA, EXTERNAL
 *  INTERRUPTS ON FPGA.<\br>
 *  See Atmel 8111C-MCU Wireless-09/09, subclauses 1.1, 6.6, tables 1-1, 6-9.
 */
#define ML507_REG__IRQ4_NCLR        (0x00C00018)

/**//**
 * \name    Enumeration of the ARC native IRQ vectors.
 * \details The following IRQ vectors are used by the ARC native units:
 *  - IRQ3 (level 1) - is used by the ARC Timer #0 for signaling the Compare-Match event. This timer is used by the
 *      System Timer Driver to perform milliseconds timestamping and timed tasks execution.
 *  - IRQ7 (level 2) - is used by the ARC Timer #1 for signaling the Compare-Match event. This timer is used by the
 *      Radio Driver to perform internal timed operations. This interrupt is not used by the BCM SoC.
 *
 * \par     Documentation
 *  See Broadcom ZIGBEE MAC & HIF HARDWARE ON SOC 8/30/2013 (UPDATED ON 7/24/2014 FOR ACTUAL SOC IMPLEMENTATION),
 *  subclause INTERRUPTS INTO ZIGBEE ARC CPU.<\br>
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclauses INTERRUPT CONNECTIONS TO CPU ON FPGA.<\br>
 *  See Synopsys DesignWare ARCompact Instruction-Set Architecture. Programmer's Reference Version 5115-057 June 2012,
 *  subclause 4.3.5, table 4-3.
 */
enum ML507_IRQ__ARC_Vector_t {
    ML507_IRQ__ARC_TIMER0_MATCH     = 3,        /*!< IRQ3: Compare-match in the ARC Timer #0. */
    ML507_IRQ__ARC_TIMER1_MATCH     = 7,        /*!< IRQ7: Compare-match in the ARC Timer #1. */
};

/**//**
 * \name    Enumeration of the ML507 hardware IRQ vectors.
 * \details The following IRQ vectors are used by the ML507 hardware:
 *  - IRQ4 (level 2) - is used by the ML507 external interrupts unit. It is triggered on the falling edge on the J6-2
 *      pin to which the AT86RF /IRQ line is connected (the compound Radio /IRQ configured to active LO). This interrupt
 *      channel has different assignment for the BCM SoC.
 *  - IRQ6 (level 1) - is used by the ML507 UART RX FIFO controller.
 *  - IRQ8 (level 1) - is used by the ML507 UART TX FIFO controller.
 *  - IRQ9 (level 1) - is used by the ML507 Symbol Timer for signaling the compound Compare-Match event from channels
 *      #0~5. These channels are used by different MAC services.
 *  - IRQ10 (level 1) - is used by the ML507 Symbol Timer for signaling the Compare-Match event from channel #6. This
 *      channel is not used both on ML507 and BCM SoC.
 *  - IRQ11 (level 2) - is used by the ML507 Symbol Timer for signaling the Compare-Match event from channel #7. This
 *      channel is used by the Radio Driver to perform Data transmission at specified timestamp. This interrupt is not
 *      used by the BCM SoC.
 *
 * \par     Documentation
 *  See Broadcom ZIGBEE MAC & HIF HARDWARE ON SOC 8/30/2013 (UPDATED ON 7/24/2014 FOR ACTUAL SOC IMPLEMENTATION),
 *  subclause INTERRUPTS INTO ZIGBEE ARC CPU.<\br>
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclauses INTERRUPT CONNECTIONS TO CPU ON FPGA.<\br>
 *  See Synopsys DesignWare ARCompact Instruction-Set Architecture. Programmer's Reference Version 5115-057 June 2012,
 *  subclause 4.3.5, table 4-3.
 */
enum ML507_IRQ__ML507_Vector_t {
    ML507_IRQ__ML507_RADIO_IRQ          = 4,        /*!< IRQ4: Falling edge on ML507 J6-2: AT86RF /IRQ. */
    ML507_IRQ__ML507_UART_RX_IRQ        = 6,        /*!< IRQ6: Service request from the ML507 UART RX FIFO. */
    ML507_IRQ__ML507_UART_TX_IRQ        = 8,        /*!< IRQ8: Service request from the ML507 UART TX FIFO. */
    ML507_IRQ__ML507_SYMBOL05_MATCH     = 9,        /*!< IRQ9: Compare-match in the ML507 SymCntr #0~5. */
    ML507_IRQ__ML507_SYMBOL6_MATCH      = 10,       /*!< IRQ10: Compare-match in the ML507 SymCntr #6. */
    ML507_IRQ__ML507_SYMBOL7_MATCH      = 11,       /*!< IRQ11: Compare-match in the ML507 SymCntr #7. */
};

/**//**
 * \name    Enumeration of the BCM Radio simulator IRQ vectors.
 * \details The ML507 Radio Driver work is performed on the basis of the Level 2 IRQs such as the AT86RF Radio compound
 *  IRQ (the IRQ4 vector) and the ARC Timer #1 IRQ (the IRQ7 vector). This Driver simulates the SoC Radio HAL event
 *  system by issuing Level 1 IRQs over different vectors almost in the same way as the SoC Radio HAL does. The Diver
 *  uses the Auxiliary Register AUX_IRQ_HINT to trigger the Level 1 IRQs on its discretion.
 * \details The following IRQ vectors are used to simulate the SoC Radio HAL:
 *  - IRQ12 (level 1) - is used by the SoC Radio HAL simulator for issuing the PLME-SET-TRX-STATE.confirm. Actually, in
 *      the SoC Radio HAL, this event is signaled through the IRQ4 on one of the Auxiliary Timer channels. The IRQ4
 *      vector on ML507 is occupied by the external Radio IRQ, so the nearest unused IRQ vector - IRQ12 - is used for
 *      this. Notice that there is no special IRQ neither in ML507 nor in AT86RF Radio that confirms the state switching
 *      requested with the PLME-SET-TRX-STATE.request (the PLL_LOCK is not used due to complexity of its processing via
 *      the compound IRQ pin), continuous polling of the AT86RF Radio state is used with postponed IRQ12 processing.
 *  - IRQ13 (level 1) - is used for issuing the PLME-SET.confirm for the PHY PIB attribute phyCurrentChannel on the SoC
 *      Radio simulator on ML507.
 *  - IRQ22 (level 1) - is used for issuing the PLME-CCA.confirm and PLME-ED.confirm both on the SoC Radio HAL and its
 *      simulator on ML507.
 *  - IRQ26 (level 1) - is used for issuing the PD-DATA.confirm both on the SoC Radio HAL and its simulator on ML507.
 *  - IRQ28 (level 1) - is used for issuing the PD-DATA.indication both on the SoC Radio HAL and its simulator on ML507.
 *
 * \par     Documentation
 *  See Broadcom ZIGBEE MAC & HIF HARDWARE ON SOC 8/30/2013 (UPDATED ON 7/24/2014 FOR ACTUAL SOC IMPLEMENTATION),
 *  subclause INTERRUPTS INTO ZIGBEE ARC CPU.<\br>
 *  See BROADBEE INTERIM PLATFORM (UPDATED ON 1/06/2014), subclauses INTERRUPT CONNECTIONS TO CPU ON FPGA.<\br>
 *  See Synopsys DesignWare ARCompact Instruction-Set Architecture. Programmer's Reference Version 5115-057 June 2012,
 *  subclauses 3.3.17, 4.3.5, 4.3.12, figure 3-27, tables 4-3, 4-4.
 */
enum ML507_IRQ__BCM_Vector_t {
    ML507_IRQ__BCM_RADIO_STATE_DONE     = 12,       /*!< IRQ12: BCM Radio simulator: STATE_DONE. */
    ML507_IRQ__BCM_RADIO_CHANNEL_DONE   = 13,       /*!< IRQ13: BCM Radio simulator: CHANNEL_DONE. */
    ML507_IRQ__BCM_RADIO_CCA_ED_DONE    = 22,       /*!< IRQ22: BCM Radio simulator: CCA_ED_DONE. */
    ML507_IRQ__BCM_RADIO_TX_DONE        = 26,       /*!< IRQ26: BCM Radio simulator: TX_DONE. */
    ML507_IRQ__BCM_RADIO_RX_DONE        = 28,       /*!< IRQ28: BCM Radio simulator: RX_DONE. */
};

#endif /* _BB_ML507_PRIVATE_IRQ_H */
