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
#include "os.h"
#include "bchp_common.h"
#include "common_arm.h"
#include "interrupts.h"
#include "int1.h"
#include "bchp_hif_cpu_intr1.h"
#include "bchp_timer.h"
#ifndef BSU_USE_UPG_TIMER
#include "bchp_webhif_timer.h"
#endif

#ifdef BSU_USE_UPG_TIMER
    #define REG_TIMER_CTRL                      BCHP_TIMER_TIMER3_CTRL
    #define TIMER_CTRL_MODE_SHIFT               BCHP_TIMER_TIMER3_CTRL_MODE_SHIFT
    #define REG_TIMER_IE0                       BCHP_TIMER_TIMER_IE0
    #define TIMER_IE0_TMRxT0_SHIFT              BCHP_TIMER_TIMER_IE0_TMR3TO_SHIFT
    #define TIMERx_CTRL_ENA_SHIFT               BCHP_TIMER_TIMER3_CTRL_ENA_SHIFT
    #define REG_INTR1_STATUS                    BCHP_HIF_CPU_INTR1_INTR_W2_STATUS
    #define INTR1_TIMER_MASK                    BCHP_HIF_CPU_INTR1_INTR_W2_STATUS_UPG_TMR_CPU_INTR_MASK
    #define REG_TIMER_IS                        BCHP_TIMER_TIMER_IS
    #define TIMER_MASK                          BCHP_TIMER_TIMER_IS_TMR3TO_MASK
    #define OTHER_TIMER_MASK                    (BCHP_TIMER_TIMER_IS_WDINT_MASK | BCHP_TIMER_TIMER_IS_TMR1TO_MASK | BCHP_TIMER_TIMER_IS_TMR0TO_MASK)
    #define REG_INTR1_MASK_CLEAR                BCHP_HIF_CPU_INTR1_INTR_W2_MASK_CLEAR
    #define REG_INTR1_MASK_SET                  BCHP_HIF_CPU_INTR1_INTR_W2_MASK_SET
    #define INTR1_MASK_CLEAR_TIMER_INTR_MASK    BCHP_HIF_CPU_INTR1_INTR_W2_MASK_CLEAR_UPG_TMR_CPU_INTR_MASK
    #define INTR1_MASK_SET_TIMER_INTR_MASK      BCHP_HIF_CPU_INTR1_INTR_W2_MASK_SET_UPG_TMR_CPU_INTR_MASK
    #define REG_INTR1_MASK_STATUS               BCHP_HIF_CPU_INTR1_INTR_W2_MASK_STATUS
#else
    #define REG_TIMER_CTRL                      BCHP_WEBHIF_TIMER_TIMER0_CTRL
    #define TIMER_CTRL_MODE_SHIFT               BCHP_TIMER_TIMER0_CTRL_MODE_SHIFT
    #define REG_TIMER_IE0                       BCHP_WEBHIF_TIMER_TIMER_IE0
    #define TIMER_IE0_TMRxT0_SHIFT              BCHP_TIMER_TIMER_IE0_TMR0TO_SHIFT
    #define TIMERx_CTRL_ENA_SHIFT               BCHP_TIMER_TIMER0_CTRL_ENA_SHIFT
    #define REG_INTR1_STATUS                    BCHP_HIF_CPU_INTR1_INTR_W1_STATUS
    #define INTR1_TIMER_MASK                    BCHP_HIF_CPU_INTR1_INTR_W1_STATUS_WEBHIF_TIMER_CPU_INTR_MASK
    #define REG_TIMER_IS                        BCHP_WEBHIF_TIMER_TIMER_IS
    #define TIMER_MASK                          BCHP_TIMER_TIMER_IS_TMR0TO_MASK
    #define REG_INTR1_MASK_CLEAR                BCHP_HIF_CPU_INTR1_INTR_W1_MASK_CLEAR
    #define REG_INTR1_MASK_SET                  BCHP_HIF_CPU_INTR1_INTR_W1_MASK_SET
    #define INTR1_MASK_CLEAR_TIMER_INTR_MASK    BCHP_HIF_CPU_INTR1_INTR_W1_MASK_CLEAR_WEBHIF_TIMER_CPU_INTR_MASK
    #define INTR1_MASK_SET_TIMER_INTR_MASK      BCHP_HIF_CPU_INTR1_INTR_W1_MASK_SET_WEBHIF_TIMER_CPU_INTR_MASK
    #define REG_INTR1_MASK_STATUS               BCHP_HIF_CPU_INTR1_INTR_W1_MASK_STATUS
#endif

void timer_init(void)
{
    unsigned int reg;

    /* Setup 10ms interrupts */
    reg = BCHP_PHYSICAL_OFFSET | REG_TIMER_CTRL;
    DEV_WR(reg, (1 << TIMER_CTRL_MODE_SHIFT) | 270000);

    reg = BCHP_PHYSICAL_OFFSET | REG_TIMER_IE0;
    DEV_WR(reg, DEV_RD(reg) | 1 << TIMER_IE0_TMRxT0_SHIFT);     /* enable timer 0 interrupt */

    reg = BCHP_PHYSICAL_OFFSET | REG_TIMER_CTRL;
    DEV_WR(reg, DEV_RD(reg) | (1 << TIMERx_CTRL_ENA_SHIFT));    /* enable the timer */
}

void timer_uninit(void)
{
    unsigned int reg;

    reg = BCHP_PHYSICAL_OFFSET | REG_TIMER_CTRL;
    DEV_WR(reg, DEV_RD(reg) &= ~(1 << TIMERx_CTRL_ENA_SHIFT));    /* disable the timer */
}

void interrupt_handler(void)
{
    unsigned int L1_status;
    unsigned int L2_status;

    OSIntEnter();

    L1_status = DEV_RD(BCHP_PHYSICAL_OFFSET | REG_INTR1_STATUS);
    if (L1_status & INTR1_TIMER_MASK) {
        L2_status = DEV_RD(BCHP_PHYSICAL_OFFSET | REG_TIMER_IS);
        if (L2_status & TIMER_MASK) {
            DEV_WR(BCHP_PHYSICAL_OFFSET | REG_TIMER_IS, TIMER_MASK);
            OSTimeTick();
        }
#ifdef BSU_USE_UPG_TIMER
        if (L2_status & OTHER_TIMER_MASK) {
            CPUINT1_Isr();
        }
#endif
    }
    else {
        CPUINT1_Isr();
    }

    OSIntExit();
}

void interrupts_init(void)
{
    timer_init();
    DEV_WR(BCHP_PHYSICAL_OFFSET | REG_INTR1_MASK_CLEAR, INTR1_MASK_CLEAR_TIMER_INTR_MASK);
}

void interrupts_uninit(void)
{
    DEV_WR(BCHP_PHYSICAL_OFFSET | REG_INTR1_MASK_SET, INTR1_MASK_SET_TIMER_INTR_MASK);
    timer_uninit();
}
