/***************************************************************************
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
 ***************************************************************************/

#ifndef BRCMSTB_H_
#define BRCMSTB_H_

enum {
    STB_SUN_TOP_CTRL_RESET_SOURCE_ENABLE,
    STB_SUN_TOP_CTRL_SW_MASTER_RESET,

    STB_HIF_CONTINUATION_STB_BOOT_HI_ADDR0,
    STB_HIF_CONTINUATION_STB_BOOT_ADDR0,

    STB_HIF_CPUBIUCTRL_CPU0_PWR_ZONE_CNTRL_REG,
    STB_HIF_CPUBIUCTRL_CPU_RESET_CONFIG_REG,

    STB_HIF_CPUBIUARCH_ADDRESS_RANGE0_ULIMIT,

    /* only one set of tracelog registers is supported */
    STB_MEMC_SENTINEL_RANGE_START,
    STB_MEMC_SENTINEL_RANGE_END,

    STB_MEMC_TRACELOG_VERSION,
    STB_MEMC_TRACELOG_CONTROL,
    STB_MEMC_TRACELOG_COMMAND,
    STB_MEMC_TRACELOG_BUFFER_PTR,
    STB_MEMC_TRACELOG_BUFFER_PTR_EXT,
    STB_MEMC_TRACELOG_BUFFER_SIZE,
    STB_MEMC_TRACELOG_BUFFER_WR_PTR,
    STB_MEMC_TRACELOG_TRIGGER_MODE,
    STB_MEMC_TRACELOG_FILTER_MODEi_ARRAY_BASE,
    STB_MEMC_TRACELOG_FILTER_ADDR_LOWERi_ARRAY_BASE,
    STB_MEMC_TRACELOG_FILTER_ADDR_UPPERi_ARRAY_BASE,

    STB_REG_LAST
};

uintptr_t stb_reg_addr(uintptr_t reg);

uintptr_t stb_reg_group_size(uintptr_t reg);


#define STB_REG_ADDR(reg) (stb_reg_addr(reg))

#define STB_REG_GROUP_SIZE(reg) (stb_reg_group_size(reg))

#define STB_REG_RD(reg)                                     \
    (*(volatile uint32_t *)STB_REG_ADDR(reg))

#define STB_REG_WR(reg, rval) {                             \
        *(volatile uint32_t *)STB_REG_ADDR(reg) = (rval);   \
    }

#define STB_REG_RD_OFFSET(reg, offset)                      \
    (*(volatile uint32_t *)(STB_REG_ADDR(reg) + offset))

#define STB_REG_WR_OFFSET(reg, offset, rval) {              \
        *(volatile uint32_t *)                              \
            (STB_REG_ADDR(reg) + offset) = (rval);          \
    }

#define STB_REG_FLD_GET(reg, mask, shift)                   \
    ((STB_REG_RD(reg) & (mask)) >> (shift))

#define STB_REG_FLD_CLR(reg, mask) {                        \
        uintptr_t addr = STB_REG_ADDR(reg);                  \
        uint32_t rval = *(volatile uint32_t *)addr;         \
        rval &= ~(mask);                                    \
        *(volatile uint32_t *)addr = rval;                  \
    }

#define STB_REG_FLD_SET(reg, mask, shift, fval) {           \
        uintptr_t addr = STB_REG_ADDR(reg);                  \
        uint32_t rval = *(volatile uintptr_t *)addr;         \
        rval &= ~(mask);                                    \
        rval |= (fval) << (shift);                          \
        *(volatile uint32_t *)addr = rval;                  \
    }

#define REG_RD(addr)											\
	(*(volatile uint32_t *)(addr))

#define REG_WR(addr, rval) {									\
		*(volatile uint32_t *)(addr) = (rval);				\
	}

#define REG_FLD_GET(addr, mask, shift)							\
	((REG_RD(addr) & (mask)) >> (shift))

#define REG_FLD_CLR(addr, mask) {								\
		uint32_t rval = REG_RD(addr);							\
		rval &= ~(mask);										\
		REG_WR(addr);											\
	}

#define REG_FLD_SET(addr, mask, shift, fval) {					\
		uint32_t rval = REG_RD(addr);							\
		rval &= ~(mask);										\
		rval |= (fval) << (shift);								\
		REG_WR(addr);											\
	}


#ifndef BIT
#define BIT(x) ( (uint32_t)1 << x )
#endif

#endif /* BRCMSTB_REGS_H_ */
