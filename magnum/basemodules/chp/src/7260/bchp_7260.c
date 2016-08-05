/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2006-2015 Broadcom. All rights reserved.
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
 *
 *****************************************************************************/

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "breg_mem.h"
#include "bchp.h"
#include "bchp_priv.h"
#include "bchp_7260.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_clkgen.h"
#include "bchp_sun_gisb_arb.h"

#include "bchp_memc_ddr_0.h"
#include "bchp_memc_arb_0.h"
#include "bchp_pwr.h"

#include "bchp_avs_ro_registers_0.h"
#include "bchp_avs.h"

#include "bchp_v3d_top_gr_bridge.h"
#include "bchp_v3d_ctl_0.h"
#include "bchp_v3d_hub_ctl.h"
#include "bchp_zone0_fs.h"

BDBG_MODULE(BCHP);

#if !BCHP_UNIFIED_IMPL
#error
#endif

/* Lookup table for chip features and etc.
 * The are many times when the chip device id register
 * not conforming to the standard numbering convention. We do
 * it this way to work-around those problems.
 *
 * TODO: Update this table to support new revisions.
 */
static const struct BCHP_P_Info s_aChipInfoTable[] =
{
    /* Chip Family contains the major and minor revs */
#if BCHP_VER == BCHP_VER_A0
    {0x72600000},
#else
    #error "Port required"
#endif
    {0} /* terminate */
};

/* Static function prototypes */
static BERR_Code BCHP_P_GetFeature
    ( const BCHP_Handle                hChip,
      const BCHP_Feature               eFeature,
      void                            *pFeatureValue );

static BERR_Code BCHP_P_ResetMagnumCores
    ( const BCHP_Handle                hChip );

#ifndef EMULATION
static void BCHP_P_ResetRaagaCore
    ( const BCHP_Handle                hChip,
      const BREG_Handle                hReg );

static void BCHP_P_ResetV3dCore
    ( const BCHP_Handle                hChip,
      const BREG_Handle                hReg );
#endif

BERR_Code BCHP_Open7260
    ( BCHP_Handle                     *phChip,
      BREG_Handle                      hRegister )
{
    BCHP_OpenSettings openSettings;
    BCHP_GetDefaultOpenSettings(&openSettings);
    openSettings.reg = hRegister;
    return BCHP_Open(phChip, &openSettings);
}

BERR_Code BCHP_Open( BCHP_Handle *phChip, const BCHP_OpenSettings *pSettings )
{
    BCHP_P_Context *pChip;
    BERR_Code rc;

    BDBG_ENTER(BCHP_Open7260);

    /* If error ocurr user get a NULL *phChip */
    *phChip = NULL;

    pChip = BCHP_P_Open(pSettings, s_aChipInfoTable);
    if (!pChip) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    pChip->pGetFeatureFunc  = BCHP_P_GetFeature;

    BCHP_P_ResetMagnumCores( pChip );

    /* Open BCHP_PWR */
    rc = BCHP_PWR_Open(&pChip->pwrManager, pChip);
    if (rc) {
        BKNI_Free(pChip);
        return BERR_TRACE(rc);
    }

    /* Open AVS module */
    BCHP_P_AvsOpen(&pChip->hAvsHandle, pChip);
    if(!pChip->hAvsHandle) {
        BCHP_PWR_Close(pChip->pwrManager);
        BKNI_Free(pChip);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* All done. now return the new fresh context to user. */
    *phChip = (BCHP_Handle)pChip;

    /* Clear AVD/SVD shutdown enable bit */
#if BCHP_PWR_RESOURCE_AVD0
    BCHP_PWR_AcquireResource(pChip, BCHP_PWR_RESOURCE_AVD0);
#endif
    /* TDB
    BREG_Write32(hRegister, BCHP_DECODE_IP_SHIM_0_SOFTSHUTDOWN_CTRL_REG, 0x0);
    */
#if BCHP_PWR_RESOURCE_AVD0
    BCHP_PWR_ReleaseResource(pChip, BCHP_PWR_RESOURCE_AVD0);
#endif

#if BCHP_PWR_RESOURCE_AVD1
    BCHP_PWR_AcquireResource(pChip, BCHP_PWR_RESOURCE_AVD1);
#endif
    /* TDB
    BREG_Write32(hRegister, BCHP_DECODE_IP_SHIM_1_SOFTSHUTDOWN_CTRL_REG, 0x0);
    */
#if BCHP_PWR_RESOURCE_AVD1
    BCHP_PWR_ReleaseResource(pChip, BCHP_PWR_RESOURCE_AVD1);
#endif

    BDBG_LEAVE(BCHP_Open7260);
    return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_GetFeature
    ( const BCHP_Handle                hChip,
      const BCHP_Feature               eFeature,
      void                            *pFeatureValue )
{
    BERR_Code            rc = BERR_UNKNOWN;
    uint32_t             ulBondStatus;

    BDBG_ENTER(BCHP_P_GetFeature);

    BDBG_OBJECT_ASSERT(hChip, BCHP);

    /* read bond-out status common for many features */
    ulBondStatus = BREG_Read32(hChip->regHandle,
        BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_0);

    /* which feature? */
    switch (eFeature)
    {
    case BCHP_Feature_e3DGraphicsCapable:
        /* 3D capable? (bool) */
        *(bool *)pFeatureValue = true;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_eDvoPortCapable:
        /* dvo port capable? (bool) */
        *(bool *)pFeatureValue = false;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_eMacrovisionCapable:
        /* macrovision capable? (bool) */
        *(bool *)pFeatureValue = BCHP_GET_FIELD_DATA(ulBondStatus,
            SUN_TOP_CTRL_OTP_OPTION_STATUS_1, otp_option_macrovision_disable) ? false : true;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_eMpegDecoderCount:
        /* number of MPEG decoders (int) */
        *(int *)pFeatureValue = 2;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_eHdcpCapable:
        /* HDCP capable? (bool) */
        *(bool *)pFeatureValue = BCHP_GET_FIELD_DATA(ulBondStatus,
            SUN_TOP_CTRL_OTP_OPTION_STATUS_1, otp_option_hdcp_disable ) ? false : true;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_e3desCapable:
        /* 3DES capable? (bool) */
        *(bool *)pFeatureValue = true;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_e1080pCapable:
        /* 1080p Capable? (bool) */
        *(bool *)pFeatureValue = true;
        rc = BERR_SUCCESS;
        break;

    default:
        rc = BCHP_P_GetDefaultFeature(hChip, eFeature, pFeatureValue);
        break;
    }

    /* return result */
    BDBG_LEAVE(BCHP_P_GetFeature);
    return rc;
}


/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_ResetMagnumCores
    ( const BCHP_Handle                hChip )
{

    BREG_Handle  hRegister = hChip->regHandle;
    uint32_t ulRegVal;

    BSTD_UNUSED(ulRegVal);

#ifndef EMULATION
    BCHP_P_ResetRaagaCore(hChip, hRegister); /* must be done first before all other cores. */
    BCHP_P_ResetV3dCore(hChip, hRegister);
#endif

    /* Reset some cores. This is needed to avoid L1 interrupts before BXXX_Open can be called per core. */
    /* Note, SW_INIT set/clear registers don't need read-modify-write. */
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, xpt_sw_init,    1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, hvd0_sw_init,   1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, vec_sw_init,    1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, aio_sw_init,    1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, bvn_sw_init,    1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, raaga0_sw_init, 1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, gfx_sw_init,    1)  |

         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, dvp_ht_sw_init, 1 ));

    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET,

         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, v3d_top_sw_init, 1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, sid_sw_init,  1));

    /* Now clear the reset. */
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, xpt_sw_init,    1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, hvd0_sw_init,   1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, vec_sw_init,    1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, aio_sw_init,    1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, bvn_sw_init,    1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, raaga0_sw_init, 1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, gfx_sw_init,    1) |

         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, dvp_ht_sw_init, 1) );

    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR,

        BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_CLEAR, v3d_top_sw_init, 1) |
        BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_CLEAR, sid_sw_init,   1));

    /* use 324 MHz BVB clock */
    ulRegVal = BREG_Read32(hRegister, BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1);
    ulRegVal &= ~(BCHP_MASK(CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1));
    ulRegVal |= BCHP_FIELD_ENUM(CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1, MDIV_CH1, DEFAULT);
    BREG_Write32(hRegister, BCHP_CLKGEN_PLL_SYS0_PLL_CHANNEL_CTRL_CH_1, ulRegVal);
    BDBG_MSG(("Set BVB clock at 324 MHz."));

    return BERR_SUCCESS;
}

#ifdef BCHP_PWR_HAS_RESOURCES
#include "bchp_pwr_resources_priv.h"
#endif
#include "bchp_raaga_dsp_misc.h"


#ifndef EMULATION
/* SW workaround to ensure we can hit the Raaga SW_INIT safely */
static void BCHP_P_ResetRaagaCore(const BCHP_Handle hChip, const BREG_Handle hReg)
{
    uint32_t val;
    BSTD_UNUSED(hChip);

    /* unconditionally turn on everything that's needed to do the register write below.
       we don't know what power state we were left in. BCHP_PWR_Open() will later turn stuff off as needed */
#if BCHP_PWR_HW_CTRL_RAAGA_PWRDN_REQ
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_CTRL_RAAGA_PWRDN_REQ, true);
#endif
#if BCHP_PWR_HW_RAAGA_CH_CTRL_CH_0_POST_DIV_HOLD_CH0
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA_CH_CTRL_CH_0_POST_DIV_HOLD_CH0, true);
#endif
#if BCHP_PWR_HW_CPU_CH_CTRL_CH_5_POST_DIV_HOLD_CH5
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_CPU_CH_CTRL_CH_5_POST_DIV_HOLD_CH5, true);
#endif
#if BCHP_PWR_HW_STB_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_STB_RAAGA_DSP_0_AIO_RAAGA0_DSP_AIO_RAAGA0, true);
#endif
#if BCHP_PWR_HW_STB_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_STB_RAAGA_DSP_0_RAAGA0_DSP_RAAGA0, true);
#endif
#if BCHP_PWR_HW_RAAGA0_AIO
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_AIO, true);
#endif
#if BCHP_PWR_HW_RAAGA0
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0, true);
#endif

    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT) ;
    val = (val & ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT)))|
     (BCHP_FIELD_DATA(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT,1));
    BREG_Write32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT, val);

    val = BREG_Read32(hReg, BCHP_RAAGA_DSP_MISC_SOFT_INIT);
    val &= ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, INIT_PROC_B));
    BREG_Write32(hReg, BCHP_RAAGA_DSP_MISC_SOFT_INIT, val);

    return;
}

static void BCHP_P_HardwarePowerUpV3D(
   const BREG_Handle hReg
   )
{
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
}

static void BCHP_P_HardwarePowerDownV3D(
   const BREG_Handle hReg
   )
{
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
}

static void BCHP_P_ResetV3dCore( const BCHP_Handle hChip, const BREG_Handle hReg )
{
    BSTD_UNUSED(hChip);

#if BCHP_PWR_HW_CTRL_AVX_PWRDN_REQ
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_CTRL_AVX_PWRDN_REQ, true);
#endif
#if BCHP_PWR_HW_AVX_CH_CTRL_CH_4_POST_DIV_HOLD_CH4
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_AVX_CH_CTRL_CH_4_POST_DIV_HOLD_CH4, true);
#endif
#if BCHP_PWR_HW_CPU_CH_CTRL_CH_4_POST_DIV_HOLD_CH4
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_CPU_CH_CTRL_CH_4_POST_DIV_HOLD_CH4, true);
#endif
#if BCHP_PWR_HW_V3D_CH_CTRL_CH_0_POST_DIV_HOLD_CH0
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D_CH_CTRL_CH_0_POST_DIV_HOLD_CH0, true);
#endif
#if BCHP_PWR_HW_CTRL_V3D_PWRDN_REQ
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_CTRL_V3D_PWRDN_REQ, true);
#endif
#if BCHP_PWR_HW_V3D
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D, true);
#endif
#if BCHP_PWR_HW_STB_V3D_V3D
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_STB_V3D_V3D, true);
#endif
#if BCHP_PWR_HW_V3D_SRAM
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D_SRAM, true);
#endif

    /* We will briefly power up the clocks and the v3d core, reset to clear any interrupts, then power off again */
    BCHP_P_HardwarePowerUpV3D(hReg);

    BREG_Write32(hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, V3D_CLK_108_SW_INIT, ASSERT));
    BREG_Write32(hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, V3D_CLK_108_SW_INIT, DEASSERT));

    BREG_Write32(hReg, BCHP_V3D_CTL_0_INT_CLR, ~0);
    BREG_Write32(hReg, BCHP_V3D_HUB_CTL_INT_CLR, ~0);

    BCHP_P_HardwarePowerDownV3D(hReg);

#if BCHP_PWR_HW_V3D_SRAM
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D_SRAM, false);
#endif
#if BCHP_PWR_HW_STB_V3D_V3D
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_STB_V3D_V3D, false);
#endif
#if BCHP_PWR_HW_V3D
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D, false);
#endif
#if BCHP_PWR_HW_CTRL_V3D_PWRDN_REQ
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_CTRL_V3D_PWRDN_REQ, false);
#endif
#if BCHP_PWR_HW_V3D_CH_CTRL_CH_0_POST_DIV_HOLD_CH0
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D_CH_CTRL_CH_0_POST_DIV_HOLD_CH0, false);
#endif
#if BCHP_PWR_HW_CPU_CH_CTRL_CH_4_POST_DIV_HOLD_CH4
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_CPU_CH_CTRL_CH_4_POST_DIV_HOLD_CH4, false);
#endif
#if BCHP_PWR_HW_AVX_CH_CTRL_CH_4_POST_DIV_HOLD_CH4
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_AVX_CH_CTRL_CH_4_POST_DIV_HOLD_CH4, false);
#endif
#if BCHP_PWR_HW_CTRL_AVX_PWRDN_REQ
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_CTRL_AVX_PWRDN_REQ, false);
#endif
}
#endif
/* End of File */
