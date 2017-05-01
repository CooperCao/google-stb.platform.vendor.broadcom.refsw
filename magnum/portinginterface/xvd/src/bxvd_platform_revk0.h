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
 *
 * [File Description:]
 *
 ***************************************************************************/

#ifndef _BXVD_PLATFORM_REVK0_H_
#define _BXVD_PLATFORM_REVK0_H_

#if ((BCHP_CHIP == 7422) || (BCHP_CHIP == 7425) || (BCHP_CHIP == 7435))
#define BXVD_P_SVD_AND_AVD_PRESENT 1
#define BXVD_P_SVD_PRESENT 1

#if ((BCHP_VER == BCHP_VER_A0) | (BCHP_VER == BCHP_VER_A1))
/* SW7425-628: work around for the bus error caused by register reads. */
#define BXVD_P_SVD_GISB_ERR_WORKAROUND 1
#endif

#elif ((BCHP_CHIP == 7231) || (BCHP_CHIP == 7135) || (BCHP_CHIP == 7344) || (BCHP_CHIP == 7346) || (BCHP_CHIP == 7429))
#define BXVD_P_SVD_ONLY_PRESENT 1
#define BXVD_P_SVD_PRESENT 1

#if (((BCHP_VER == BCHP_VER_A0) || (BCHP_VER == BCHP_VER_A1)) && (BCHP_CHIP != 7429))
/* SW7425-628: work around for the bus error caused by register reads. */
#define BXVD_P_SVD_GISB_ERR_WORKAROUND 1
#endif

#elif (BCHP_CHIP == 7640)
#define BXVD_P_DVD_CHIP 1
#define BXVD_P_AVD_INIT_STRIPE_WIDTH 1
#define BXVD_P_AVD_INIT_STRIPE_MULTIPLE 0
#endif

/* SVD Rev L and later support Interlaced SVC */
#if ((((BCHP_CHIP == 7231) || (BCHP_CHIP == 7344) || (BCHP_CHIP == 7346) || (BCHP_CHIP == 7425)) && \
      ((BCHP_VER != BCHP_VER_A0) && (BCHP_VER != BCHP_VER_A1))) || BCHP_CHIP == 7429)
#define BXVD_P_SVC_INTERLACED_SUPPORT 1
#endif

/* SVD Rev M */
#if (BCHP_CHIP == 7435)
#define BXVD_P_SVC_INTERLACED_SUPPORT 1
#define BXVD_P_VP6_SUPPORT 1
#define BXVD_P_ILS_BUFFERS_INTERNAL 1
#endif

/* AVD Rev M */
#if (BCHP_CHIP == 7584)
#define BXVD_P_VP6_SUPPORT 1
#endif

/* Some platforms need to determine AVD core clock speed at runtime. */
#if (BCHP_CHIP == 7552)
#define BXVD_P_DYNAMIC_AVD_CORE_FREQ 1
#endif

#if BXVD_P_SVD_AND_AVD_PRESENT
#include "bchp_svd_intr2_0.h"
#include "bchp_int_id_svd_intr2_0.h"
#include "bchp_avd_intr2_1.h"
#include "bchp_int_id_avd_intr2_1.h"
#include "bchp_reg_cabac2bins2_1.h"
#include "bchp_decode_ip_shim_1.h"
#include "bchp_decode_sint_1.h"
#include "bchp_decode_sint_oloop_1.h"
#include "bchp_decode_main_1.h"
#include "bchp_avd_cache_1.h"
#include "bchp_decode_rvc_1.h"
#include "bchp_decode_ind_sdram_regs_1.h"
#include "bchp_decode_ind_sdram_regs2_1.h"
#include "bchp_decode_cpuregs2_1.h"
#include "bchp_decode_cpuregs_1.h"
#include "bchp_decode_cpuaux2_1.h"
#include "bchp_decode_cpuaux_1.h"
#include "bchp_decode_sd_1.h"
#include "bchp_bld_decode_cpuregs_0.h"
#include "bchp_bld_decode_ind_sdram_regs_0.h"
#include "bchp_bld_decode_cpuaux_0.h"
#include "bchp_bld_decode_sd_0.h"
#include "bchp_bld_decode_ip_shim_0.h"
#include "bchp_bld_decode_main_0.h"
#include "bchp_ils_regs_0.h"
#include "bchp_bld_decode_sint_0.h"

#elif BXVD_P_SVD_ONLY_PRESENT
#include "bchp_svd_intr2_0.h"
#include "bchp_int_id_svd_intr2_0.h"
#include "bchp_bld_decode_cpuregs_0.h"
#include "bchp_bld_decode_ind_sdram_regs_0.h"
#include "bchp_bld_decode_cpuaux_0.h"
#include "bchp_bld_decode_sd_0.h"
#include "bchp_bld_decode_ip_shim_0.h"
#include "bchp_bld_decode_main_0.h"
#include "bchp_ils_regs_0.h"
#include "bchp_bld_decode_sint_0.h"

#else
#include "bchp_avd_intr2_0.h"
#include "bchp_int_id_avd_intr2_0.h"
#endif

#include "bchp_reg_cabac2bins2_0.h"
#include "bchp_decode_rvc_0.h"
#include "bchp_avd_cache_0.h"

#include "bchp_bvnf_intr2_3.h"
#include "bchp_decode_cpuregs2_0.h"
#include "bchp_decode_cpuregs_0.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_sun_gisb_arb.h"
#include "bchp_xpt_pcroffset.h"
#include "bchp_xpt_rave.h"

#include "bchp_decode_ind_sdram_regs_0.h"
#include "bchp_decode_ind_sdram_regs2_0.h"
#include "bchp_decode_cpuaux2_0.h"
#include "bchp_decode_cpuaux_0.h"
#include "bchp_decode_main_0.h"
#include "bchp_decode_sint_0.h"
#include "bchp_decode_sint_oloop_0.h"
#include "bchp_decode_ip_shim_0.h"

#include "bafl.h"

#define BXVD_P_CHIP_PRODUCT_REVISION  BCHP_SUN_TOP_CTRL_PRODUCT_ID

#define  BXVD_P_AVD_ARC600 1

#include "bxvd_core_avd_revk0.h"

#include "bxvd_priv.h"

#if (!BXVD_P_DVD_CHIP)
#include "bchp_decode_sd_0.h"

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
#else
#define BXVD_P_GET_MEMORY_CONFIG(hXvd, eMemCfgMode)                            \
{                                                                              \
      eMemCfgMode = BXVD_P_MemCfgMode_eUMA;                                    \
}
#endif

#if ((BCHP_CHIP == 7422) || (BCHP_CHIP == 7425) || (BCHP_CHIP == 7435))
#define BXVD_MAX_INSTANCE_COUNT     2
#else
#define BXVD_MAX_INSTANCE_COUNT     1
#endif

#if (BCHP_CHIP == 7344) || (BCHP_CHIP == 7346) || (BCHP_CHIP == 35233) || (BCHP_CHIP == 7425) || (BCHP_CHIP == 7429) || (BCHP_CHIP == 7435)
/* UART clock frequency */
#define BXVD_P_AVD_CORE_UART_FREQ (426*1000000)

#elif (BCHP_CHIP == 7228)
/* UART clock frequency */
#define BXVD_P_AVD_CORE_UART_FREQ (375*1000000)

#else /* All other chips */
/* UART clock frequency */
#define BXVD_P_AVD_CORE_UART_FREQ (400*1000000)
#endif

/* Rev K core use 1.50 MB */

#define BXVD_P_FW_IMAGE_SIZE              0x00180000
#define BXVD_P_FW_INNER_IMAGE_OFFSET      0x00090000
#define BXVD_P_FW_BASELAYER_IMAGE_OFFSET  0x0012d000

/* Define supported decode protocols mask and OTP control register constants. */
#if (((BCHP_CHIP == 7231) || (BCHP_CHIP == 7344) || (BCHP_CHIP == 7346) || (BCHP_CHIP == 7425) || (BCHP_CHIP == 7640)) && \
     ((BCHP_VER != BCHP_VER_A0) && (BCHP_VER != BCHP_VER_A1)) || BCHP_CHIP == 7360 || BCHP_CHIP == 7362 || BCHP_CHIP == 7429)
/* SVD/AVD Rev L */
#define BXVD_P_PLATFORM_SUPPORTED_PROTOCOLS  BXVD_P_REVL_DECODE_PROTOCOLS_MASK
#define BXVD_P_CORE_REVISION 'L'

#define BXVD_P_OTP_CTL_REG  BCHP_DECODE_IP_SHIM_0_OTP_CTL_REG
#define BXVD_P_OTP_CTL_REG_disable_RV9_MASK BCHP_DECODE_IP_SHIM_0_OTP_CTL_REG_disable_RV9_MASK

#elif ((BCHP_CHIP == 7228) || (BCHP_CHIP == 7435) || (BCHP_CHIP == 7563) || (BCHP_CHIP == 7584))
/* SVD/AVD Rev M */
#define BXVD_P_PLATFORM_SUPPORTED_PROTOCOLS  BXVD_P_REVM_DECODE_PROTOCOLS_MASK
#define BXVD_P_CORE_REVISION 'M'

#define BXVD_P_OTP_CTL_REG  BCHP_DECODE_IP_SHIM_0_OTP_CTL_REG
#define BXVD_P_OTP_CTL_REG_disable_RV9_MASK BCHP_DECODE_IP_SHIM_0_OTP_CTL_REG_disable_RV9_MASK

#else
/* SVD/AVD Rev K */
#define BXVD_P_PLATFORM_SUPPORTED_PROTOCOLS  BXVD_P_REVK_DECODE_PROTOCOLS_MASK
#define BXVD_P_CORE_REVISION 'K'

#define BXVD_P_OTP_CTL_REG  0
#define BXVD_P_OTP_CTL_REG_disable_RV9_MASK 0
#endif

#if (BXVD_P_PLATFORM_SUPPORTED_PROTOCOLS & (BXVD_P_CREATE_PROTOCOLS_MASK(BAVC_VideoCompressionStd_eRV9)))
/* HW is RV9 capable, but it could still be disabled by OTP */
#define BXVD_P_RV9_CAPABLE 1

#else
/* HW can not decode RV9 protocol streams */
#define BXVD_P_RV9_CAPABLE 0
#endif

/* Get picture buffer atom size */
#define BXVD_P_USE_GET_BUFFER_ATOM_SIZE_REVE0 1
#define BXVD_P_GET_BUFFER_ATOM_SIZE BXVD_P_GetBufferAtomSize_RevE0
BXVD_P_SETUP_GET_BUFFER_ATOM_SIZE_PROTOTYPE(RevE0);

/* Platform Stripe Info */
#define BXVD_P_USE_DETERMINE_STRIPE_INFO_REVK0 1
#define BXVD_P_DETERMINE_STRIPE_INFO BXVD_P_DetermineStripeInfo_RevK0
BXVD_P_DETERMINE_STRIPE_INFO_PROTOTYPE(RevK0);

/* Firmware loading */
#define BXVD_P_USE_FW_LOAD_CORE_REVK0 1
#define BXVD_P_FW_LOAD BXVD_P_FWLoad_RevK0
BXVD_P_FW_LOAD_PROTOTYPE(RevK0);

/* Chip enable */
#define BXVD_P_USE_CORE_CHIP_ENABLE_REVK0 1
#define BXVD_P_CHIP_ENABLE BXVD_P_ChipEnable_RevK0
BXVD_P_CHIP_ENABLE_PROTOTYPE(RevK0);

/* Chip Reset */
#define BXVD_P_USE_CORE_RESET_CHIP_REVK0 1
#define BXVD_P_RESET_CHIP BXVD_P_ChipReset_RevK0
BXVD_P_CHIP_RESET_PROTOTYPE(RevK0);

/* Init Register Pointers/Masks */
#define BXVD_P_USE_INIT_REG_PTRS_REVK0 1
#define BXVD_P_INIT_REG_PTRS BXVD_P_InitRegPtrs_RevK0
BXVD_P_INIT_REG_PTRS_PROTOTYPE(RevK0);

/* Setup FW Memory */
#define BXVD_P_USE_SETUP_FW_MEMORY_REVE0 1
#define BXVD_P_SETUP_FW_MEMORY BXVD_P_SetupFWMemory_RevE0
BXVD_P_SETUP_FW_MEMORY_PROTOTYPE(RevE0);

/* Teardown FW Memory */
#define BXVD_P_USE_TEAR_DOWN_FW_MEMORY_REVE0 1
#define BXVD_P_TEAR_DOWN_FW_MEMORY BXVD_P_TearDownFWMemory_RevE0
BXVD_P_TEAR_DOWN_FW_MEMORY_PROTOTYPE(RevE0);

/* Verify Watchdog Fired */
#define BXVD_P_USE_VERIFY_WATCHDOG_FIRED_REVK0 1
#define BXVD_P_VERIFY_WATCHDOG_FIRED BXVD_P_VerifyWatchdogFired_RevK0_isr
BXVD_P_VERIFY_WATCHDOG_FIRED_PROTOTYPE(RevK0);

/* Rave Context Register info needed by FW */
#define BXVD_P_RAVE_CONTEXT_SIZE        (BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR)
#define BXVD_P_RAVE_CX_HOLD_CLR_STATUS  BCHP_XPT_RAVE_CX_HOLD_CLR_STATUS
#define BXVD_P_RAVE_PACKET_COUNT        BCHP_XPT_RAVE_PACKET_COUNT

/* Use Host Interface Memory routines */
#define BXVD_P_USE_READ_DISPLAY_INFO_HIM_API_REVK0 1

#define BXVD_P_IS_DISPLAY_INFO_EQUAL BXVD_P_IsDisplayInfoEqual_HIM_API_isr

#if BXVD_P_POWER_MANAGEMENT

/* For systems that do not have BCHP PWR AVD0_PWR resource */
#if (BCHP_CHIP == 35233)

#define BXVD_P_USE_SET_POWER_STATE_REVE0_PM2 1
#define BXVD_P_SET_POWER_STATE BXVD_P_SetPowerState_RevE0_PM2

void BXVD_P_SetPowerState_RevE0_PM2(BXVD_Handle hXvd,
                                    BXVD_P_PowerState PowerState);
#else

#define BXVD_P_USE_SET_POWER_STATE_REVK0 1
#define BXVD_P_SET_POWER_STATE BXVD_P_SetPowerState_RevK0

void BXVD_P_SetPowerState_RevK0(BXVD_Handle hXvd,
                                BXVD_P_PowerState PowerState);
#endif
#endif /* BXVD_P_POWER_MANAGEMENT */

#endif /* _BXVD_PLATFORM_REVK0_H_ */
