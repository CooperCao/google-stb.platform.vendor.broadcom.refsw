/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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

#ifndef _BXVD_PLATFORM_7400_H_
#define _BXVD_PLATFORM_7400_H_

#include "bchp_7400.h"
#include "bchp_avd_intr2_0.h"
#include "bchp_avd_intr2_1.h"
#include "bchp_decode_cpuregs2_0.h"
#include "bchp_decode_cpuregs2_1.h"
#include "bchp_decode_cpuregs_0.h"
#include "bchp_decode_cpuregs_1.h"
#include "bchp_xpt_pcroffset.h"
#include "bchp_int_id_avd_intr2_0.h"
#include "bchp_int_id_avd_intr2_1.h"
#include "bchp_decode_ind_sdram_regs_0.h"
#include "bchp_decode_ind_sdram_regs_1.h"
#include "bchp_decode_cpuaux_0.h"
#include "bchp_decode_cpuaux_1.h"
#include "bchp_decode_cpuaux2_0.h"
#include "bchp_decode_cpuaux2_1.h"
#include "bchp_decode_ind_sdram_regs2_0.h"
#include "bchp_decode_ind_sdram_regs2_1.h"
#include "bchp_decode_ip_shim_0.h"
#include "bchp_decode_ip_shim_1.h"
#include "bchp_sun_gisb_arb.h"
#include "bchp_sun_l2.h"
#include "bchp_sun_top_ctrl.h"

#include "bxvd_priv.h"

#define BXVD_MAX_INSTANCE_COUNT     2

#define BXVD_P_VALIDATE_PDEFSETTINGS(pDefSettings)               \
{                                                                \
   if (pDefSettings->hPictureHeap == NULL)                       \
   {                                                             \
      BDBG_ERR(("Picture Heap device Settings not supplied"));   \
      return BERR_TRACE(BERR_INVALID_PARAMETER);                 \
   }                                                             \
}

#define BXVD_P_GET_MEMORY_CONFIG(hXvd, eMemCfgMode)  \
      eMemCfgMode = BXVD_P_MemCfgMode_eNONUMA;

/* Define supported decode protocols mask */
/* AVD Rev H */
#define BXVD_P_PLATFORM_SUPPORTED_PROTOCOLS  BXVD_P_REVH_DECODE_PROTOCOLS_MASK
#define BXVD_P_CORE_REVISION 'H'

/* Get picture buffer atom size */
#define BXVD_P_USE_GET_BUFFER_ATOM_SIZE_REVE0 1
#define BXVD_P_GET_BUFFER_ATOM_SIZE BXVD_P_GetBufferAtomSize_RevE0
BXVD_P_SETUP_GET_BUFFER_ATOM_SIZE_PROTOTYPE(RevE0);

/* Platform Stripe Info */
#define BXVD_P_USE_DETERMINE_STRIPE_INFO_REVE0 1
#define BXVD_P_DETERMINE_STRIPE_INFO BXVD_P_DetermineStripeInfo_RevE0
BXVD_P_DETERMINE_STRIPE_INFO_PROTOTYPE(RevE0);

/* Firmware loading */
#define BXVD_P_USE_FW_LOAD_CORE_REVE0 1
#define BXVD_P_FW_LOAD BXVD_P_FWLoad_RevE0
BXVD_P_FW_LOAD_PROTOTYPE(RevE0);

/* Chip enable */
#define BXVD_P_USE_CORE_CHIP_ENABLE_REVE0 1
#define BXVD_P_CHIP_ENABLE BXVD_P_ChipEnable_RevE0
BXVD_P_CHIP_ENABLE_PROTOTYPE(RevE0);

/* Chip Reset */
#define BXVD_P_USE_CORE_RESET_CHIP_REVE0 1
#define BXVD_P_RESET_CHIP BXVD_P_ChipReset_RevE0
BXVD_P_CHIP_RESET_PROTOTYPE(RevE0);

/* Init Register Pointers/Masks */
#define BXVD_P_USE_INIT_REG_PTRS_7400B0 1
#define BXVD_P_INIT_REG_PTRS BXVD_P_InitRegPtrs_7400B0
BXVD_P_INIT_REG_PTRS_PROTOTYPE(7400B0);

/* Setup FW Memory  */
#define BXVD_P_USE_SETUP_FW_MEMORY_REVE0 1
#define BXVD_P_SETUP_FW_MEMORY BXVD_P_SetupFWMemory_RevE0
BXVD_P_SETUP_FW_MEMORY_PROTOTYPE(RevE0);

/* Setup FW Memory */
#define BXVD_P_USE_TEAR_DOWN_FW_MEMORY_REVE0 1
#define BXVD_P_TEAR_DOWN_FW_MEMORY BXVD_P_TearDownFWMemory_RevE0
BXVD_P_TEAR_DOWN_FW_MEMORY_PROTOTYPE(RevE0);

/* Verify Watchdog Fired */
#define BXVD_P_USE_VERIFY_WATCHDOG_FIRED_REVE0 1
#define BXVD_P_VERIFY_WATCHDOG_FIRED BXVD_P_VerifyWatchdogFired_RevE0_isr
BXVD_P_VERIFY_WATCHDOG_FIRED_PROTOTYPE(RevE0);

#if BXVD_P_POWER_MANAGEMENT

#define BXVD_P_USE_SET_POWER_STATE_REVE0 1
#define BXVD_P_SET_POWER_STATE BXVD_P_SetPowerState_RevE0

void BXVD_P_SetPowerState_RevE0(BXVD_Handle hXvd,
                               BXVD_P_PowerState PowerStateNew);
#endif

#endif
