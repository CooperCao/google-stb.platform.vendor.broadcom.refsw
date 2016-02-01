/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef _BXVD_PLATFORM_7405_H_
#define _BXVD_PLATFORM_7405_H_

#include "bchp_avd_intr2_0.h"
#include "bchp_bvnf_intr2_3.h"
#include "bchp_int_id_avd_intr2_0.h"
#include "bchp_decode_cpuregs2_0.h"
#include "bchp_decode_cpuregs_0.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_sun_gisb_arb.h"
#include "bchp_xpt_pcroffset.h"

#include "bchp_decode_ind_sdram_regs_0.h"
#include "bchp_decode_ind_sdram_regs2_0.h"
#include "bchp_decode_main_0.h"
#include "bchp_decode_sint_0.h"
#include "bchp_decode_sint_oloop_0.h"
#include "bchp_decode_cpuaux2_0.h"
#include "bchp_decode_cpuaux_0.h"
#include "bchp_decode_sd_0.h"
#include "bchp_decode_ip_shim_0.h"

#include "bxvd_priv.h"

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif


/* Macro to determine memory configuration, UMA, NON-UMA or UNKNOWN */

/* 7405 A0/A1, 7335 */
#if (((BCHP_CHIP == 7405) && ((BCHP_VER == BCHP_VER_A0) || (BCHP_VER == BCHP_VER_A1))) || \
       (BCHP_CHIP == 7335))
#define BXVD_P_GET_MEMORY_CONFIG(hXvd, eMemCfgMode)  \
      eMemCfgMode = BXVD_P_MemCfgMode_eUNKNOWN;

/* 7325 */
#elif (BCHP_CHIP == 7325)
#define BXVD_P_GET_MEMORY_CONFIG(hXvd, eMemCfgMode)  \
      eMemCfgMode = BXVD_P_MemCfgMode_eUMA;

/* 7405 B0+,3548, 3556 */
#elif ((BCHP_CHIP == 7405) && (BCHP_VER >= BCHP_VER_B0)) || \
      (BCHP_CHIP == 3548) || \
      (BCHP_CHIP == 3556) || \
      (BCHP_CHIP == 7336) || \
      (BCHP_CHIP == 7340) || \
      (BCHP_CHIP == 7342) || \
      (BCHP_CHIP == 7125) || \
      (BCHP_CHIP == 7408) || \
      (BCHP_CHIP == 7468)

#define BXVD_P_GET_MEMORY_CONFIG(hXvd, eMemCfgMode)                            \
{                                                                              \
   uint32_t uiPfriSource;                                                      \
   uiPfriSource = (hXvd->uiDecode_PFRIDataRegVal  &                            \
                   hXvd->stPlatformInfo.stReg.uiDecode_PFRIDataSourceMask) >>  \
                   hXvd->stPlatformInfo.stReg.uiDecode_PFRIDataSourceShift;    \
   if (uiPfriSource == 0)                                                      \
   {                                                                           \
      eMemCfgMode = BXVD_P_MemCfgMode_eUMA;                                    \
   }                                                                           \
   else                                                                        \
   {                                                                           \
      eMemCfgMode = BXVD_P_MemCfgMode_eNONUMA;                                 \
   }                                                                           \
}
#endif

/* Define clock register and mask values */
#if (BCHP_CHIP == 7405)
#define BXVD_P_AVD0_CTRL                       BCHP_VCXO_CTL_MISC_AVD_CTRL
#define BXVD_P_AVD0_CTRL_PWRDN_MASK            BCHP_VCXO_CTL_MISC_AVD_CTRL_POWERDOWN_MASK
#define BXVD_P_AVD0_CORE_CLK_CTRL              BCHP_CLK_PM_CTRL_1
#define BXVD_P_AVD0_CORE_CLK_CTRL_PWRDN_MASK   BCHP_CLK_PM_CTRL_1_DIS_AVD0_PROG_CLK_MASK
#define BXVD_P_AVD0_SCB_CLK_CTRL               BCHP_CLK_PM_CTRL_2
#define BXVD_P_AVD0_SCB_CLK_CTRL_PWRDWN_MASK   BCHP_CLK_PM_CTRL_2_DIS_AVD_216M_CLK_MASK
#define BXVD_P_AVD0_GISB_CLK_CTRL              BCHP_CLK_PM_CTRL
#define BXVD_P_AVD0_GISB_CLK_CTRL_PWRDWN_MASK  BCHP_CLK_PM_CTRL_DIS_AVD0_108M_CLK_MASK

#elif (BCHP_CHIP == 7325)
#define BXVD_P_AVD0_CTRL                          BCHP_CLKGEN_AVD_CTRL
#define BXVD_P_AVD0_CTRL_PWRDN_MASK               BCHP_CLKGEN_AVD_CTRL_POWERDOWN_MASK
#define BXVD_P_AVD0_CORE_CLK_CTRL                 BCHP_CLKGEN_PWRDN_CTRL_3
#define BXVD_P_AVD0_CORE_CLK_CTRL_PWRDN_MASK      BCHP_CLKGEN_PWRDN_CTRL_3_PWRDN_CLOCK_AVD0_PROG_MASK
#define BXVD_P_AVD0_SCB_CLK_CTRL                  BCHP_CLKGEN_PWRDN_CTRL_0
#define BXVD_P_AVD0_SCB_CLK_CTRL_PWRDWN_MASK      BCHP_CLKGEN_PWRDN_CTRL_0_PWRDN_CLOCK_216_CG_AVD_MASK
#define BXVD_P_AVD0_GISB_CLK_CTRL                 BCHP_CLKGEN_PWRDN_CTRL_1
#define BXVD_P_AVD0_GISB_CLK_CTRL_PWRDWN_MASK     BCHP_CLKGEN_PWRDN_CTRL_1_PWRDN_CLOCK_108_CG_AVD_MASK

#elif (BCHP_CHIP == 7335)
#define BXVD_P_AVD0_CTRL                          BCHP_VCXO_CTL_MISC_AVD_CTRL
#define BXVD_P_AVD0_CTRL_PWRDN_MASK               BCHP_VCXO_CTL_MISC_AVD_CTRL_POWERDOWN_MASK
#define BXVD_P_AVD0_CORE_CLK_CTRL                 BCHP_CLK_PM_CTRL_1
#define BXVD_P_AVD0_CORE_CLK_CTRL_PWRDN_MASK      BCHP_CLK_PM_CTRL_1_DIS_AVD0_PROG_CLK_MASK
#define BXVD_P_AVD0_SCB_CLK_CTRL                  BCHP_CLK_PM_CTRL_2
#define BXVD_P_AVD0_SCB_CLK_CTRL_PWRDWN_MASK      BCHP_CLK_PM_CTRL_2_DIS_AVD_216M_CLK_MASK
#define BXVD_P_AVD0_GISB_CLK_CTRL                 BCHP_CLK_PM_CTRL
#define BXVD_P_AVD0_GISB_CLK_CTRL_PWRDWN_MASK     BCHP_CLK_PM_CTRL_DIS_AVD0_108M_CLK_MASK

#elif (BCHP_CHIP == 3548) || (BCHP_CHIP == 3556)
#define BXVD_P_AVD0_CTRL                        BCHP_VCXO_CTL_MISC_AVD_CTRL
#define BXVD_P_AVD0_CTRL_PWRDN_MASK             BCHP_VCXO_CTL_MISC_AVD_CTRL_POWERDOWN_MASK
#define BXVD_P_AVD0_CORE_CLK_CTRL               BCHP_CLKGEN_PWRDN_CTRL_3
#define BXVD_P_AVD0_CORE_CLK_CTRL_PWRDN_MASK    BCHP_CLKGEN_PWRDN_CTRL_3_PWRDN_CLOCK_250_CG_AVD_MASK
#define BXVD_P_AVD0_SCB_CLK_CTRL                BCHP_CLKGEN_PWRDN_CTRL_0
#define BXVD_P_AVD0_SCB_CLK_CTRL_PWRDWN_MASK    BCHP_CLKGEN_PWRDN_CTRL_0_PWRDN_CLOCK_216_CG_AVD_MASK
#define BXVD_P_AVD0_GISB_CLK_CTRL               BCHP_CLKGEN_PWRDN_CTRL_1
#define BXVD_P_AVD0_GISB_CLK_CTRL_PWRDWN_MASK   BCHP_CLKGEN_PWRDN_CTRL_1_PWRDN_CLOCK_108_CG_AVD_MASK

#elif (BCHP_CHIP == 7125) && (BCHP_VER < BCHP_VER_C0)
#define BXVD_P_AVD0_CTRL                        BCHP_VCXO_CTL_MISC_RAP_AVD_PLL_CHL_6
#define BXVD_P_AVD0_CTRL_PWRDN_MASK             BCHP_VCXO_CTL_MISC_RAP_AVD_PLL_CHL_6_DIS_CH_MASK
#define BXVD_P_AVD0_CORE_CLK_CTRL               BCHP_CLKGEN_PWRDN_CTRL_0
#define BXVD_P_AVD0_CORE_CLK_CTRL_PWRDN_MASK    BCHP_CLKGEN_PWRDN_CTRL_0_PWRDN_CLOCK_250_AVD_MASK
#define BXVD_P_AVD0_SCB_CLK_CTRL                BCHP_CLKGEN_PWRDN_CTRL_1
#define BXVD_P_AVD0_SCB_CLK_CTRL_PWRDWN_MASK    BCHP_CLKGEN_PWRDN_CTRL_1_PWRDN_CLOCK_216_AVD0_MASK
#define BXVD_P_AVD0_GISB_CLK_CTRL               BCHP_CLKGEN_PWRDN_CTRL_2
#define BXVD_P_AVD0_GISB_CLK_CTRL_PWRDWN_MASK   BCHP_CLKGEN_PWRDN_CTRL_2_PWRDN_CLOCK_108_AVD0_MASK

#elif (BCHP_CHIP == 7125) && (BCHP_VER >= BCHP_VER_C0)
#define BXVD_P_AVD0_CTRL                        BCHP_VCXO_CTL_MISC_RAP_AVD_PLL_CHL_6
#define BXVD_P_AVD0_CTRL_PWRDN_MASK             BCHP_VCXO_CTL_MISC_RAP_AVD_PLL_CHL_6_DIS_CH_MASK
#define BXVD_P_AVD0_CORE_CLK_CTRL               BCHP_CLKGEN_AVD_CLK_PM_CTRL
#define BXVD_P_AVD0_CORE_CLK_CTRL_PWRDN_MASK    BCHP_CLKGEN_AVD_CLK_PM_CTRL_DIS_CLK_250_MASK
#define BXVD_P_AVD0_SCB_CLK_CTRL                BCHP_CLKGEN_AVD_CLK_PM_CTRL
#define BXVD_P_AVD0_SCB_CLK_CTRL_PWRDWN_MASK    BCHP_CLKGEN_AVD_CLK_PM_CTRL_DIS_CLK_216_MASK
#define BXVD_P_AVD0_GISB_CLK_CTRL               BCHP_CLKGEN_AVD_CLK_PM_CTRL
#define BXVD_P_AVD0_GISB_CLK_CTRL_PWRDWN_MASK   BCHP_CLKGEN_AVD_CLK_PM_CTRL_DIS_CLK_108_MASK


#elif (BCHP_CHIP == 7340) || (BCHP_CHIP == 7342) || (BCHP_CHIP == 7408) || (BCHP_CHIP == 7468)
#define BXVD_P_AVD0_CTRL                        0x00
#define BXVD_P_AVD0_CTRL_PWRDN_MASK             0x00
#define BXVD_P_AVD0_CORE_CLK_CTRL               0x00
#define BXVD_P_AVD0_CORE_CLK_CTRL_PWRDN_MASK    0x00
#define BXVD_P_AVD0_SCB_CLK_CTRL                0x00
#define BXVD_P_AVD0_SCB_CLK_CTRL_PWRDWN_MASK    0x00
#define BXVD_P_AVD0_GISB_CLK_CTRL               0x00
#define BXVD_P_AVD0_GISB_CLK_CTRL_PWRDWN_MASK   0x00
#endif

#define BXVD_MAX_INSTANCE_COUNT     1

/* UART clock frequency */
#define BXVD_P_AVD_CORE_UART_FREQ (250*1000000)

/* Define supported decode protocols mask */
#if ((BCHP_CHIP == 7335) || (BCHP_CHIP == 7325))
/* AVD Rev H */
#define BXVD_P_PLATFORM_SUPPORTED_PROTOCOLS  BXVD_P_REVH_DECODE_PROTOCOLS_MASK
#define BXVD_P_CORE_REVISION 'H'

#else
/* AVD Rev I */
#define BXVD_P_PLATFORM_SUPPORTED_PROTOCOLS  BXVD_P_REVI_DECODE_PROTOCOLS_MASK
#define BXVD_P_CORE_REVISION 'I'
#endif

/* Get picture buffer atom size */
#define BXVD_P_USE_GET_BUFFER_ATOM_SIZE_REVE0 1
#define BXVD_P_GET_BUFFER_ATOM_SIZE BXVD_P_GetBufferAtomSize_RevE0
BXVD_P_SETUP_GET_BUFFER_ATOM_SIZE_PROTOTYPE(RevE0);

/* Platform Stripe Info */
#define BXVD_P_USE_DETERMINE_STRIPE_INFO_7405 1
#define BXVD_P_DETERMINE_STRIPE_INFO BXVD_P_DetermineStripeInfo_7405
BXVD_P_DETERMINE_STRIPE_INFO_PROTOTYPE(7405);

/* Firmware loading */
#define BXVD_P_USE_FW_LOAD_CORE_REVE0 1
#define BXVD_P_FW_LOAD BXVD_P_FWLoad_RevE0
BXVD_P_FW_LOAD_PROTOTYPE(RevE0);

/* Chip enable */
#define BXVD_P_USE_CORE_CHIP_ENABLE_REVE0 1
#define BXVD_P_CHIP_ENABLE BXVD_P_ChipEnable_RevE0
BXVD_P_CHIP_ENABLE_PROTOTYPE(RevE0);

/* Chip Reset */
#define BXVD_P_USE_CORE_RESET_CHIP_7405 1
#define BXVD_P_RESET_CHIP BXVD_P_ChipReset_7405
BXVD_P_CHIP_RESET_PROTOTYPE(7405);

/* Init Register Pointers/Masks */
#define BXVD_P_USE_INIT_REG_PTRS_7405 1
#define BXVD_P_INIT_REG_PTRS BXVD_P_InitRegPtrs_7405
BXVD_P_INIT_REG_PTRS_PROTOTYPE(7405);

/* Setup FW Memory */
#define BXVD_P_USE_SETUP_FW_MEMORY_REVE0 1
#define BXVD_P_SETUP_FW_MEMORY BXVD_P_SetupFWMemory_RevE0
BXVD_P_SETUP_FW_MEMORY_PROTOTYPE(RevE0);

/* Teardown FW Memory */
#define BXVD_P_USE_TEAR_DOWN_FW_MEMORY_REVE0 1
#define BXVD_P_TEAR_DOWN_FW_MEMORY BXVD_P_TearDownFWMemory_RevE0
BXVD_P_TEAR_DOWN_FW_MEMORY_PROTOTYPE(RevE0);

/* Verify Watchdog Fired */
#define BXVD_P_USE_VERIFY_WATCHDOG_FIRED_REVE0 1
#define BXVD_P_VERIFY_WATCHDOG_FIRED BXVD_P_VerifyWatchdogFired_RevE0_isr
BXVD_P_VERIFY_WATCHDOG_FIRED_PROTOTYPE(RevE0);

#if BXVD_P_POWER_MANAGEMENT

#ifdef BCHP_PWR_RESOURCE_AVD0

/* Use BCHP_PWR PM2 method */
#define BXVD_P_USE_SET_POWER_STATE_REVE0_PM2 1
#define BXVD_P_SET_POWER_STATE BXVD_P_SetPowerState_RevE0_PM2

void BXVD_P_SetPowerState_RevE0_PM2(BXVD_Handle hXvd,
                                    BXVD_P_PowerState PowerStateNew);

#else /* Use direct CLK register access PM method, PM 1 */
#define BXVD_P_USE_XVD_PM1               1
#define BXVD_P_USE_SET_POWER_STATE_REVE0 1
#define BXVD_P_SET_POWER_STATE BXVD_P_SetPowerState_RevE0

void BXVD_P_SetPowerState_RevE0(BXVD_Handle hXvd,
                               BXVD_P_PowerState PowerStateNew);
#endif
#endif

#endif /* _BXVD_PLATFORM_7405_H_ */
