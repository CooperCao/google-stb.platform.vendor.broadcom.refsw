/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

/*
 * cpupower.cpp
 *
 *  Created on: Mar 6, 2015
 *      Author: gambhire
 */

#include <cstdint>

#include "config.h"

#include "arm/arm.h"
#include "platform.h"
#include "pgtable.h"
#include "lib_printf.h"
#include "brcmstb.h"

extern uint32_t num_cpus;

enum {
    ZONE_MAN_CLKEN_MASK = BIT(0),
    ZONE_MAN_RESET_CNTL_MASK = BIT(1),
    ZONE_MAN_MEM_PWR_MASK = BIT(4),
    ZONE_RESERVED_1_MASK = BIT(5),
    ZONE_MAN_ISO_CNTL_MASK = BIT(6),
    ZONE_MANUAL_CONTROL_MASK = BIT(7),
    ZONE_PWR_DN_REQ_MASK = BIT(9),
    ZONE_PWR_UP_REQ_MASK = BIT(10),
    ZONE_BLK_RST_ASSERT_MASK = BIT(12),
    ZONE_PWR_OFF_STATE_MASK = BIT(25),
    ZONE_PWR_ON_STATE_MASK = BIT(26),
    ZONE_DPG_PWR_STATE_MASK = BIT(28),
    ZONE_MEM_PWR_STATE_MASK = BIT(29),
    ZONE_RESET_STATE_MASK = BIT(31),
};

static void cpuPowerOn(int cpuNum) {
    uint32_t power_zone_base = STB_REG_ADDR(STB_HIF_CPUBIUCTRL_CPU0_PWR_ZONE_CNTRL_REG);
    uint32_t power_zone_reg = power_zone_base + (cpuNum * 4);

#ifdef AUTO_POWER_ON_SEQ
    REG_WR(power_zone_reg, ZONE_PWR_UP_REQ_MASK);

    uint32_t val = REG_RD(power_zone_reg);
    while (!(val & ZONE_PWR_ON_STATE_MASK))
        val = REG_RD(power_zone_reg);
#else
    /* HW7445-1289 - sequence below is manual core bringup which affects all
               28nm chips except for 7445a0 */
    uint32_t reg_val = REG_RD(power_zone_reg);
    reg_val = (reg_val & 0xffffff00) | ZONE_MAN_ISO_CNTL_MASK;
    REG_WR(power_zone_reg, reg_val);

    reg_val = REG_RD(power_zone_reg);
    reg_val = (reg_val & 0xffffffff) | ZONE_MANUAL_CONTROL_MASK;
    REG_WR(power_zone_reg, reg_val);

    reg_val = REG_RD(power_zone_reg);
    reg_val = (reg_val & 0xffffffff) | ZONE_RESERVED_1_MASK;
    REG_WR(power_zone_reg, reg_val);

    reg_val = REG_RD(power_zone_reg);
    reg_val = (reg_val & 0xffffffff) | ZONE_MAN_MEM_PWR_MASK;
    REG_WR(power_zone_reg, reg_val);

    uint32_t val = REG_RD(power_zone_reg);
    while (!(val & ZONE_MEM_PWR_STATE_MASK))
        val = REG_RD(power_zone_reg);

    reg_val = REG_RD(power_zone_reg);
    reg_val = (reg_val & 0xffffffff) | ZONE_MAN_CLKEN_MASK;
    REG_WR(power_zone_reg, reg_val);

    val = REG_RD(power_zone_reg);
    while (!(val & ZONE_DPG_PWR_STATE_MASK))
        val = REG_RD(power_zone_reg);

    reg_val = REG_RD(power_zone_reg);
    reg_val = (reg_val & 0xffffffff) & ~ZONE_MAN_ISO_CNTL_MASK;
    REG_WR(power_zone_reg, reg_val);

    reg_val = REG_RD(power_zone_reg);
    reg_val = (reg_val & 0xffffffff) | ZONE_MAN_RESET_CNTL_MASK;
    REG_WR(power_zone_reg, reg_val);
#endif
}

static void cpuBoot(int cpuNum, void *physBootAddr) {
    const uint32_t offset = cpuNum * 8;

    STB_REG_WR_OFFSET(STB_HIF_CONTINUATION_STB_BOOT_HI_ADDR0, offset, 0);

    STB_REG_WR_OFFSET(STB_HIF_CONTINUATION_STB_BOOT_ADDR0, offset, (unsigned long)physBootAddr);

    asm volatile ("dsb\r\n isb":::"memory");

    STB_REG_FLD_CLR(STB_HIF_CPUBIUCTRL_CPU_RESET_CONFIG_REG, BIT(cpuNum));
}

int Platform::numCpus() {
    return num_cpus;
}

void Platform::powerOn(int cpuNum, void *physBootAddr) {
    cpuPowerOn(cpuNum);
    cpuBoot(cpuNum, physBootAddr);
}

void Platform::powerOff(int cpuNum) {
    UNUSED(cpuNum);
    //TBD
}

void Platform::reboot() {
    /* Reset source enable register */
    STB_REG_FLD_SET(STB_SUN_TOP_CTRL_RESET_SOURCE_ENABLE, 1, 0, 1);

    /* master reset control register */
    STB_REG_FLD_SET(STB_SUN_TOP_CTRL_SW_MASTER_RESET, 1, 0, 1);
}
