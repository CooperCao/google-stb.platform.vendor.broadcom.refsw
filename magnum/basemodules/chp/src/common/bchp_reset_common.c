/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "breg_mem.h"
#include "bchp.h"
#include "bchp_priv.h"

#include "bchp_common.h"
#if defined BCHP_V3D_TFU_REG_START || BCHP_V3D_TFUV3D_REG_START
/* VC5 */
#define USE_VC5 1

/* Some useful macros to determine the V3D version at compile time */
#define V3D_MAKE_VER(TECH_VERSION, REVISION, SUB_REV, HIDDEN_REV) \
   (((TECH_VERSION) << 24) | ((REVISION) << 16) | ((SUB_REV) << 8) | (HIDDEN_REV))
#define V3D_EXTRACT_TECH_VERSION(VER) ((VER) >> 24)
#define V3D_EXTRACT_REVISION(VER) (((VER) >> 16) & 0xff)
#define V3D_EXTRACT_SUB_REV(VER) (((VER) >> 8) & 0xff)
#define V3D_EXTRACT_HIDDEN_REV(VER) ((VER) & 0xff)

#define V3D_VER (V3D_MAKE_VER(V3D_TECH_VERSION, V3D_REVISION, V3D_SUB_REV, V3D_HIDDEN_REV))
#define V3D_VER_AT_LEAST(TECH_VERSION, REVISION, SUB_REV, HIDDEN_REV) \
   (V3D_VER >= V3D_MAKE_VER(TECH_VERSION, REVISION, SUB_REV, HIDDEN_REV))

/* Determine the core revision */
#ifdef BCHP_V3D_CTL_0_REG_START
#include "bchp_v3d_ctl_0.h"
#define V3D_TECH_VERSION   BCHP_V3D_CTL_0_IDENT0_TVER_DEFAULT
#define V3D_REVISION       BCHP_V3D_CTL_0_IDENT1_REV_DEFAULT
#define V3D_SUB_REV        BCHP_V3D_CTL_0_IDENT3_IPREV_DEFAULT
#define V3D_HIDDEN_REV     0
#elif defined(BCHP_V3D_CTLV3D_REG_START)
#include "bchp_v3d_ctlv3d.h"
#define V3D_TECH_VERSION   BCHP_V3D_CTLV3D_IDENT0_TVER_DEFAULT
#define V3D_REVISION       BCHP_V3D_CTLV3D_IDENT1_REV_DEFAULT
#define V3D_SUB_REV        BCHP_V3D_CTLV3D_IDENT3_IPREV_DEFAULT
#define V3D_HIDDEN_REV     0
#else
#include "bchp_v3d_ctl.h"
#define V3D_TECH_VERSION   BCHP_V3D_CTL_IDENT0_TVER_DEFAULT
#define V3D_REVISION       BCHP_V3D_CTL_IDENT1_REV_DEFAULT
#define V3D_SUB_REV        BCHP_V3D_CTL_IDENT3_IPREV_DEFAULT
#define V3D_HIDDEN_REV     0
#endif

#endif /* BCHP_V3D_TFU_REG_START */

#if ( BCHP_CHIP==7439 || BCHP_CHIP==7250 )
#define USE_RFM 1
#endif

#include "bchp_sun_top_ctrl.h"

#include "bchp_memc_ddr_0.h"
#include "bchp_memc_arb_0.h"
#include "bchp_pwr.h"

#include "bchp_avs_ro_registers_0.h"
#include "bchp_avs.h"
#if ( BCHP_CHIP==7445 )
#include "bchp_v3d_gca.h"
#endif
#ifdef BCHP_V3D_TOP_GR_BRIDGE_REG_START
#include "bchp_v3d_top_gr_bridge.h"
#endif

#ifdef USE_VC5
#include "bchp_clkgen.h"

#ifdef BCHP_V3D_HUB_CTL_REG_START
#include "bchp_v3d_hub_ctl.h"
#else
#include "bchp_v3d_hub_ctlv3d.h"
#endif

#if V3D_VER_AT_LEAST(3,3,0,0)
#else
#include "bchp_v3d_ctl.h"
#endif /* V3D_VER_AT_LEAST */

#else
/* VC4 */
#include "bchp_v3d_dbg.h"
#include "bchp_v3d_ctl.h"
#endif /* USE_VC5 */

#ifdef BCHP_ZONE0_FS_REG_START
#include "bchp_zone0_fs.h"
#endif

#include "bchp_reset_common.h"

BDBG_MODULE(BCHP);

#if !BCHP_UNIFIED_IMPL
#error
#endif

#ifdef BCHP_RAAGA_DSP_MISC_REG_START
#include "bchp_raaga_dsp_misc.h"
#endif

#ifdef BCHP_ASP_ARCSS_CTRL_REG_START
#include "bchp_asp_arcss_ctrl.h"
#endif

#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_xpt_sw_init_MASK
#define XPT_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, xpt_sw_init,    1 )
#else
#define XPT_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_hvd0_sw_init_MASK
#define HVD0_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, hvd0_sw_init,    1 )
#else
#define HVD0_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_hvd1_sw_init_MASK
#define HVD1_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, hvd1_sw_init,    1 )
#else
#define HVD1_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_vec_sw_init_MASK
#define VEC_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, vec_sw_init,    1 )
#else
#define VEC_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_aio_sw_init_MASK
#define AIO_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, aio_sw_init,    1 )
#else
#define AIO_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_bvn_sw_init_MASK
#define BVN_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, bvn_sw_init,    1 )
#else
#define BVN_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_raaga0_sw_init_MASK
#define RAAGA0_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, raaga0_sw_init,    1 )
#else
#define RAAGA0_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_raaga1_sw_init_MASK
#define RAAGA1_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, raaga1_sw_init,    1 )
#else
#define RAAGA1_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_gfx_sw_init_MASK
#define GFX_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, gfx_sw_init,    1 )
#else
#define GFX_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_dvp_ht_sw_init_MASK
#define DVP_HT_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, dvp_ht_sw_init,    1 )
#else
#define DVP_HT_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_dvp_ht0_sw_init_MASK
#define DVP_HT0_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, dvp_ht0_sw_init,    1 )
#else
#define DVP_HT0_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_dvp_hr_sw_init_MASK
#define DVP_HR_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, dvp_hr_sw_init,    1 )
#else
#define DVP_HR_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_rfm_sw_init_MASK
#ifdef USE_RFM
#define RFM_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, rfm_sw_init,    1 )
#else
#define RFM_SW_INIT 0
#endif
#else
#define RFM_SW_INIT 0
#endif

#define BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_DATA ( XPT_SW_INIT | VEC_SW_INIT | HVD0_SW_INIT | HVD1_SW_INIT | RFM_SW_INIT | \
        AIO_SW_INIT | BVN_SW_INIT | RAAGA0_SW_INIT | RAAGA1_SW_INIT | GFX_SW_INIT | DVP_HT_SW_INIT | DVP_HT0_SW_INIT | DVP_HR_SW_INIT )

#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_leap_sw_init_MASK
#define LEAP_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, leap_sw_init,    1 )
#else
#define LEAP_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_vice20_sw_init_MASK
#define VICE20_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, vice20_sw_init,    1 )
#else
#define VICE20_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_vice21_sw_init_MASK
#define VICE21_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, vice21_sw_init,    1 )
#else
#define VICE21_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_v3d_top_sw_init_MASK
#define V3D_TOP_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, v3d_top_sw_init,    1 )
#else
#define V3D_TOP_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sid_sw_init_MASK
#define SID_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, sid_sw_init,    1 )
#else
#define SID_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_hvd2_sw_init_MASK
#define HVD2_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, hvd2_sw_init,    1 )
#else
#define HVD2_SW_INIT 0
#endif
#ifdef BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_asp_top_sw_init_MASK
#define ASP_SW_INIT  BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, asp_top_sw_init,    1 )
#else
#define ASP_SW_INIT 0
#endif


#define BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_DATA ( LEAP_SW_INIT | SID_SW_INIT | VICE20_SW_INIT | VICE21_SW_INIT | V3D_TOP_SW_INIT | HVD2_SW_INIT | ASP_SW_INIT )

#if !defined(EMULATION)

#ifdef BCHP_ASP_ARCSS_CTRL_REG_START
/* SW workaround to ensure we can hit the Raaga SW_INIT safely */
static void BCHP_P_ResetAspCore(const BCHP_Handle hChip, const BREG_Handle hReg)
{
    uint32_t val;
    BSTD_UNUSED(hChip);

#ifdef BCHP_ASP_ARCSS_CTRL_WD_TIMER0_CONTROL
    val = BREG_Read32(hReg,BCHP_ASP_ARCSS_CTRL_WD_TIMER0_CONTROL) ;
    val = (val & ~(BCHP_MASK(ASP_ARCSS_CTRL_WD_TIMER0_CONTROL, STATUS)   |
                   BCHP_MASK(ASP_ARCSS_CTRL_WD_TIMER0_CONTROL, INTRT_EN) |
                   BCHP_MASK(ASP_ARCSS_CTRL_WD_TIMER0_CONTROL, COUNT_EN) ));
    BREG_Write32(hReg,BCHP_ASP_ARCSS_CTRL_WD_TIMER0_CONTROL, val);
#endif /* BCHP_ASP_ARCSS_CTRL_WD_TIMER0_CONTROL */

#ifdef BCHP_ASP_ARCSS_CTRL_WD_TIMER1_CONTROL
    val = BREG_Read32(hReg,BCHP_ASP_ARCSS_CTRL_WD_TIMER1_CONTROL) ;
    val = (val & ~(BCHP_MASK(ASP_ARCSS_CTRL_WD_TIMER1_CONTROL, STATUS)   |
                   BCHP_MASK(ASP_ARCSS_CTRL_WD_TIMER1_CONTROL, INTRT_EN) |
                   BCHP_MASK(ASP_ARCSS_CTRL_WD_TIMER1_CONTROL, COUNT_EN) ));
    BREG_Write32(hReg,BCHP_ASP_ARCSS_CTRL_WD_TIMER1_CONTROL, val);
#endif /* BCHP_ASP_ARCSS_CTRL_WD_TIMER1_CONTROL */

    return;
}
#endif  /* BCHP_ASP_ARCSS_CTRL_REG_START */

#ifdef BCHP_RAAGA_DSP_MISC_REG_START
/* SW workaround to ensure we can hit the Raaga SW_INIT safely */
static void BCHP_P_ResetRaagaCore(const BCHP_Handle hChip, const BREG_Handle hReg)
{
    uint32_t val;
    BSTD_UNUSED(hChip);

    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT) ;
    val = (val & ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT)))|
     (BCHP_FIELD_DATA(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT,1));
    BREG_Write32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT, val);

#ifdef BCHP_RAAGA_DSP_MISC_SOFT_INIT_INIT_PROC_B_SHIFT
    val = BREG_Read32(hReg, BCHP_RAAGA_DSP_MISC_SOFT_INIT);
    val &= ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, INIT_PROC_B));
    BREG_Write32(hReg, BCHP_RAAGA_DSP_MISC_SOFT_INIT, val);
#endif

#if BCHP_RAAGA_DSP_MISC_1_SOFT_INIT
    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_1_SOFT_INIT) ;
    val = (val & ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT)))|
     (BCHP_FIELD_DATA(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT,1));
    BREG_Write32(hReg,BCHP_RAAGA_DSP_MISC_1_SOFT_INIT, val);

    val = BREG_Read32(hReg, BCHP_RAAGA_DSP_MISC_1_SOFT_INIT);
    val &= ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, INIT_PROC_B));
    BREG_Write32(hReg, BCHP_RAAGA_DSP_MISC_1_SOFT_INIT, val);
#endif /* BCHP_RAAGA_DSP_MISC_1_SOFT_INIT */

    return;
}
#endif

#ifdef USE_VC5

static void BCHP_P_HardwarePowerUpV3D(
   const BREG_Handle hReg
   )
{
#ifdef BCHP_ZONE0_FS_PWR_CONTROL
   uint32_t uiSuccess, uiMask;
   uint32_t uiZoneControl;

   uiZoneControl = BCHP_FIELD_DATA(ZONE0_FS_PWR_CONTROL, ZONE_DPG_CNTL_EN, 1) |
      BCHP_FIELD_DATA(ZONE0_FS_PWR_CONTROL, ZONE_PWR_UP_REQ, 1) |
      BCHP_FIELD_DATA(ZONE0_FS_PWR_CONTROL, ZONE_BLK_RST_ASSERT, 1) |
      BCHP_FIELD_DATA(ZONE0_FS_PWR_CONTROL, ZONE_MEM_PWR_CNTL_EN, 1);

   BREG_Write32(hReg, BCHP_ZONE0_FS_PWR_CONTROL, uiZoneControl);

   uiZoneControl = BREG_Read32(hReg, BCHP_ZONE0_FS_PWR_CONTROL);

   uiSuccess = BCHP_MASK(ZONE0_FS_PWR_CONTROL, ZONE_MEM_PWR_STATE) |
      BCHP_MASK(ZONE0_FS_PWR_CONTROL, ZONE_DPG_PWR_STATE) |
      BCHP_MASK(ZONE0_FS_PWR_CONTROL, ZONE_PWR_ON_STATE);

   uiMask = BCHP_MASK(ZONE0_FS_PWR_CONTROL, ZONE_ISO_STATE) |
      BCHP_MASK(ZONE0_FS_PWR_CONTROL, ZONE_MEM_PWR_STATE) |
      BCHP_MASK(ZONE0_FS_PWR_CONTROL, ZONE_DPG_PWR_STATE) |
      BCHP_MASK(ZONE0_FS_PWR_CONTROL, ZONE_PWR_ON_STATE);

   uiZoneControl = BREG_Read32(hReg, BCHP_ZONE0_FS_PWR_CONTROL);

   while ((uiZoneControl & uiMask) != uiSuccess)
      uiZoneControl = BREG_Read32(hReg, BCHP_ZONE0_FS_PWR_CONTROL);
#else
    BSTD_UNUSED(hReg);
#endif /* BCHP_ZONE0_FS_PWR_CONTROL */
}

static void BCHP_P_HardwarePowerDownV3D(
   const BREG_Handle hReg
   )
{
#ifdef BCHP_ZONE0_FS_PWR_CONTROL
   uint32_t uiZoneControl;

   /* Power down V3D */
   uiZoneControl = BREG_Read32(hReg, BCHP_ZONE0_FS_PWR_CONTROL);

   /* Check PWR_ON_STATE bit */
   if (BCHP_GET_FIELD_DATA(uiZoneControl, ZONE0_FS_PWR_CONTROL, ZONE_PWR_ON_STATE))
   {
      uint32_t uiSuccess, uiMask;

      uiZoneControl = BCHP_FIELD_DATA(ZONE0_FS_PWR_CONTROL, ZONE_DPG_CNTL_EN, 1) |
         BCHP_FIELD_DATA(ZONE0_FS_PWR_CONTROL, ZONE_PWR_DN_REQ, 1) |
         BCHP_FIELD_DATA(ZONE0_FS_PWR_CONTROL, ZONE_MEM_PWR_CNTL_EN, 1);

      BREG_Write32(hReg, BCHP_ZONE0_FS_PWR_CONTROL, uiZoneControl);
      uiSuccess = BCHP_MASK(ZONE0_FS_PWR_CONTROL, ZONE_ISO_STATE) |
         BCHP_MASK(ZONE0_FS_PWR_CONTROL, ZONE_PWR_OFF_STATE);

      uiMask = BCHP_MASK(ZONE0_FS_PWR_CONTROL, ZONE_ISO_STATE) |
         BCHP_MASK(ZONE0_FS_PWR_CONTROL, ZONE_MEM_PWR_STATE) |
         BCHP_MASK(ZONE0_FS_PWR_CONTROL, ZONE_DPG_PWR_STATE) |
         BCHP_MASK(ZONE0_FS_PWR_CONTROL, ZONE_PWR_OFF_STATE);

      uiZoneControl = BREG_Read32(hReg, BCHP_ZONE0_FS_PWR_CONTROL);
      while ((uiZoneControl & uiMask) != uiSuccess)
         uiZoneControl = BREG_Read32(hReg, BCHP_ZONE0_FS_PWR_CONTROL);
   }
#else
    BSTD_UNUSED(hReg);
#endif /* BCHP_ZONE0_FS_PWR_CONTROL */
}

#endif /* USE_VC5 */

static void BCHP_P_ResetV3dCore( const BCHP_Handle hChip, const BREG_Handle hReg )
{
    BSTD_UNUSED(hChip);

#ifdef USE_VC5
    /* We will briefly power up the clocks and the v3d core, reset to clear any interrupts, then power off again */
    BCHP_P_HardwarePowerUpV3D(hReg);
#endif /* USE_VC5 */

#if ( BCHP_CHIP == 7445 )
    BREG_Write32( hReg, BCHP_V3D_GCA_SAFE_SHUTDOWN,
          BCHP_FIELD_DATA( V3D_GCA_SAFE_SHUTDOWN, SAFE_SHUTDOWN_EN,   1 ));

    /* poll loop to shutdown the GCA so SCB/MCP traffic is correctly handled prior to
       sw_init */
    /* both SAFE_SHUTDOWN_ACK1 & SAFE_SHUTDOWN_ACK (bits 0 & 1) */
    while( BREG_Read32( hReg, BCHP_V3D_GCA_SAFE_SHUTDOWN_ACK) != 0x3 )
    {};
#endif
#ifdef BCHP_V3D_TOP_GR_BRIDGE_REG_START
#ifdef BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0_SPARE_SW_INIT_SHIFT
    BREG_Write32(hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, SPARE_SW_INIT, ASSERT));
    BREG_Write32(hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, SPARE_SW_INIT, DEASSERT));
#else
    BREG_Write32(hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, V3D_CLK_108_SW_INIT, ASSERT));
    BREG_Write32(hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, V3D_CLK_108_SW_INIT, DEASSERT));
#endif
#endif

#ifdef USE_VC5
#if V3D_VER_AT_LEAST(3, 3, 0, 0)
    #ifdef BCHP_V3D_CTL_0_INT_CLR
    BREG_Write32(hReg, BCHP_V3D_CTL_0_INT_CLR, ~0);
    BREG_Write32(hReg, BCHP_V3D_HUB_CTL_INT_CLR, ~0);
    #else
    BREG_Write32(hReg, BCHP_V3D_CTLV3D_INT_CLR, ~0);
    BREG_Write32(hReg, BCHP_V3D_HUB_CTLV3D_INT_CLR, ~0);
    #endif
#endif /* V3D_VER_AT_LEAST */
#else
    /* VC4 clear any pending interrupts */
    BREG_Write32( hReg, BCHP_V3D_CTL_INTCTL, ~0);
    BREG_Write32( hReg, BCHP_V3D_DBG_DBQITC, ~0);
#endif

#ifdef USE_VC5
    BCHP_P_HardwarePowerDownV3D(hReg);
#endif /* USE_VC5 */
}
#endif

BERR_Code BCHP_Cmn_ResetMagnumCores( const BCHP_Handle hChip )
{

    BREG_Handle  hRegister = hChip->regHandle;

    BDBG_MSG(("BCHP_Cmn_ResetMagnumCores"));

#if !defined(EMULATION)
#ifdef BCHP_RAAGA_DSP_MISC_REG_START
    BCHP_P_ResetRaagaCore(hChip, hRegister); /* must be done first before all other cores. */
#endif
    BCHP_P_ResetV3dCore(hChip, hRegister);
#ifdef BCHP_ASP_ARCSS_CTRL_REG_START
    BCHP_P_ResetAspCore(hChip, hRegister);
#endif
#endif

    /* Reset some cores. This is needed to avoid L1 interrupts before BXXX_Open can be called per core. */
    /* Note, SW_INIT set/clear registers don't need read-modify-write. */
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_0_SET, BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_DATA );

    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_DATA );

    /* Now clear the reset. This assumes the bits for SET and CLEAR always match. */
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR, BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_DATA );

    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_DATA );


#if ( BCHP_CHIP==7439 )
    {
        uint32_t ulRegVal;
#if BCHP_HW7439_439_WORKAROUND
        /* use 216 MHz BVB clock */
        ulRegVal = BREG_Read32(hRegister, BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1);
        ulRegVal &= ~(BCHP_MASK(CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1));
        ulRegVal |= BCHP_FIELD_DATA(CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1, 18);
        BREG_Write32(hRegister, BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1, ulRegVal);
        BDBG_MSG(("Set BVB clock at 216 MHz for video encoder."));
#else

        /* use 324 MHz BVB clock */
        ulRegVal = BREG_Read32(hRegister, BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1);
        ulRegVal &= ~(BCHP_MASK(CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1));
        ulRegVal |= BCHP_FIELD_ENUM(CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1, DEFAULT);
        BREG_Write32(hRegister, BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1, ulRegVal);
        BDBG_MSG(("Set BVB clock at 324 MHz."));
#endif
    }

#endif
    return BERR_SUCCESS;
}

/* End of File */
