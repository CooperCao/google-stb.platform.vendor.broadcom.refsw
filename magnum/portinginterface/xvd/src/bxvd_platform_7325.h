/***************************************************************************
 *     Copyright (c) 2006-2008, Broadcom Corporation
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

#ifndef _BXVD_PLATFORM_7325_H_
#define _BXVD_PLATFORM_7325_H_

#include "bchp_7325.h"
#include "bchp_avd_intr2_0.h"
#include "bchp_bvnf_intr2_3.h"
#include "bchp_int_id_avd_intr2_0.h"
#include "bchp_decode_cpuregs2_0.h"
#include "bchp_decode_cpuregs_0.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_sun_gisb_arb.h"
#include "bchp_xpt_pcroffset.h"

#include "bchp_decode_ind_sdram_regs2_0.h"
#include "bchp_decode_cpuaux2_0.h"
#include "bchp_decode_cpuaux_0.h"
#include "bchp_decode_sd_0.h"
#include "bchp_decode_ip_shim_0.h"

#define BXVD_MAX_INSTANCE_COUNT     1

#define TEST_PORT_CONTROL_VALUE_ARC_0 0x0000000b

/* UART clock frequency */
#define BXVD_P_AVD_CORE_UART_FREQ (250*1000000)

/* Get picture buffer atom size */
#define BXVD_P_USE_GET_BUFFER_ATOM_SIZE_REVE0 1
#define BXVD_P_GET_BUFFER_ATOM_SIZE BXVD_P_GetBufferAtomSize_RevE0
BXVD_P_SETUP_GET_BUFFER_ATOM_SIZE_PROTOTYPE(RevE0);

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
#define BXVD_P_USE_INIT_REG_PTRS_7405A0 1
#define BXVD_P_INIT_REG_PTRS BXVD_P_InitRegPtrs_7405A0
BXVD_P_INIT_REG_PTRS_PROTOTYPE(7405A0);

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

#endif /* _BXVD_PLATFORM_7325_H_ */
