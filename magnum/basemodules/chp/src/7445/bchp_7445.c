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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "breg_mem.h"
#include "bchp.h"
#include "bchp_priv.h"
#include "bchp_7445.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_sun_gisb_arb.h"
#include "bchp_memc_ddr_0.h"
#include "bchp_memc_ddr_1.h"
#include "bchp_memc_ddr_2.h"
#include "bchp_memc_arb_0.h"
#include "bchp_pwr.h"
#include "bchp_vice2_l2_1.h"
#include "bchp_avs_ro_registers_0.h"
#include "bchp_avs.h"
#include "bchp_v3d_gca.h"
#include "bchp_v3d_dbg.h"
#include "bchp_v3d_ctl.h"
#include "bchp_vice2_misc_0.h"
#include "bchp_vice2_misc_1.h"

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
#if BCHP_VER == BCHP_VER_D0
    /* D0 code will run on D0 */
    {0x74450030},
    /* D0 code will run on D1 */
    {0x74450031},
#elif BCHP_VER == BCHP_VER_E0
    /* E0 code will run on E0 */
    {0x74450040},
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

BERR_Code BCHP_Open7445
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
    uint32_t ulBondStatus;
    BERR_Code rc;

    BDBG_ENTER(BCHP_Open7445);

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

#if BCHP_PWR_RESOURCE_VICE
    BCHP_PWR_AcquireResource(pChip, BCHP_PWR_RESOURCE_VICE);
#endif

    ulBondStatus = BREG_Read32(pChip->regHandle,BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_0);

    if(BCHP_GET_FIELD_DATA(ulBondStatus,SUN_TOP_CTRL_OTP_OPTION_STATUS_0, otp_option_vice2_0_disable) ==  false)
    {
        BREG_Write32(pChip->regHandle, BCHP_VICE2_MISC_0_SW_INIT_SET, ~BCHP_VICE2_MISC_0_SW_INIT_SET_reserved0_MASK);
        BREG_Write32(pChip->regHandle, BCHP_VICE2_MISC_0_SW_INIT_CLR, ~BCHP_VICE2_MISC_0_SW_INIT_SET_reserved0_MASK);
    }

    if(BCHP_GET_FIELD_DATA(ulBondStatus,SUN_TOP_CTRL_OTP_OPTION_STATUS_0, otp_option_vice2_1_disable) ==  false)
    {
        BREG_Write32(pChip->regHandle, BCHP_VICE2_MISC_1_SW_INIT_SET, ~BCHP_VICE2_MISC_1_SW_INIT_SET_reserved0_MASK);
        BREG_Write32(pChip->regHandle, BCHP_VICE2_MISC_1_SW_INIT_CLR, ~BCHP_VICE2_MISC_1_SW_INIT_SET_reserved0_MASK);
    }

#if BCHP_PWR_RESOURCE_VICE
    BCHP_PWR_ReleaseResource(pChip, BCHP_PWR_RESOURCE_VICE);
#endif

    BDBG_LEAVE(BCHP_Open7445);
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
            SUN_TOP_CTRL_OTP_OPTION_STATUS_0, otp_option_macrovision_disable) ? false : true;
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
            SUN_TOP_CTRL_OTP_OPTION_STATUS_0, otp_option_hdcp_disable ) ? false : true;
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

    case BCHP_Feature_eDisabledL2Registers:
        BKNI_Memset(pFeatureValue, 0, sizeof(BCHP_FeatureData));
        if (hChip->info.productId == 0x7252 || hChip->info.productId == 0x7449 || hChip->info.productId == 0x7448 ||
            hChip->info.productId == 0x72520 || hChip->info.productId == 0x74490 || hChip->info.productId == 0x74480 ) {
            ((BCHP_FeatureData*)pFeatureValue)->data.disabledL2Registers.address[0] = BCHP_VICE2_L2_1_CPU_STATUS;
        }
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


#if !defined(EMULATION)
#ifdef BCHP_PWR_HAS_RESOURCES
#include "bchp_pwr_resources_priv.h"
#endif
#include "bchp_raaga_dsp_misc.h"
#include "bchp_raaga_dsp_misc_1.h"

/* SW workaround to ensure we can hit the Raaga SW_INIT safely */
static void BCHP_P_ResetRaagaCore(const BCHP_Handle hChip, const BREG_Handle hReg)
{
    uint32_t val;

    BSTD_UNUSED(hChip);

    /* unconditionally turn on everything that's needed to do the register write below.
       we don't know what power state we were left in. BCHP_PWR_Open() will later turn stuff off as needed */
#if BCHP_PWR_HW_PLL_RAAGA
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_RAAGA, true);
#endif
#if BCHP_PWR_HW_PLL_RAAGA_PLL_CH0
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_RAAGA_PLL_CH0, true);
#endif
#if BCHP_PWR_HW_PLL_RAAGA_PLL_CH1
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_RAAGA_PLL_CH1, true);
#endif
#if BCHP_PWR_HW_RAAGA0_CLK
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_CLK, true);
#endif
#if BCHP_PWR_HW_RAAGA0_DSP
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_DSP, true);
#endif
#if BCHP_PWR_HW_RAAGA1_CLK
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA1_CLK, true);
#endif
#if BCHP_PWR_HW_RAAGA1_DSP
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA1_DSP, true);
#endif
#if BCHP_PWR_HW_RAAGA0_SRAM
    /* BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_SRAM, true); */
#endif
#if BCHP_PWR_HW_RAAGA1_SRAM
    /* BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA1_SRAM, true); */
#endif

    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT) ;
    val = (val & ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT)))|
     (BCHP_FIELD_DATA(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT,1));
    BREG_Write32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT, val);

    val = BREG_Read32(hReg, BCHP_RAAGA_DSP_MISC_SOFT_INIT);
    val &= ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, INIT_PROC_B));
    BREG_Write32(hReg, BCHP_RAAGA_DSP_MISC_SOFT_INIT, val);

    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_1_SOFT_INIT) ;
    val = (val & ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT)))|
     (BCHP_FIELD_DATA(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT,1));
    BREG_Write32(hReg,BCHP_RAAGA_DSP_MISC_1_SOFT_INIT, val);

    val = BREG_Read32(hReg, BCHP_RAAGA_DSP_MISC_1_SOFT_INIT);
    val &= ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, INIT_PROC_B));
    BREG_Write32(hReg, BCHP_RAAGA_DSP_MISC_1_SOFT_INIT, val);

#if BCHP_PWR_HW_RAAGA0_SRAM
    /* BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_SRAM, false); */
#endif
#if BCHP_PWR_HW_RAAGA1_SRAM
    /* BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA1_SRAM, false); */
#endif
#if BCHP_PWR_HW_RAAGA0_CLK
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_CLK, false);
#endif
#if BCHP_PWR_HW_RAAGA0_DSP
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_DSP, false);
#endif
#if BCHP_PWR_HW_RAAGA1_CLK
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA1_CLK, false);
#endif
#if BCHP_PWR_HW_RAAGA1_DSP
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA1_DSP, false);
#endif
#if BCHP_PWR_HW_PLL_RAAGA_PLL_CH0
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_RAAGA_PLL_CH0, false);
#endif
#if BCHP_PWR_HW_PLL_RAAGA_PLL_CH1
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_RAAGA_PLL_CH1, false);
#endif
#if BCHP_PWR_HW_PLL_RAAGA
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_RAAGA, false);
#endif

    return;

}

static void BCHP_P_ResetV3dCore( const BCHP_Handle hChip, const BREG_Handle hReg )
{
    BREG_Handle  hRegister = hChip->regHandle;
    BSTD_UNUSED(hReg);

#if BCHP_PWR_HW_PLL_MOCA_CH3
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_MOCA_CH3, true);
#endif
#if BCHP_PWR_HW_V3D
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D, true);
#endif
#if BCHP_PWR_HW_V3D_SRAM
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D_SRAM, true);
#endif

    BREG_Write32( hRegister, BCHP_V3D_GCA_SAFE_SHUTDOWN,
        BCHP_FIELD_DATA( V3D_GCA_SAFE_SHUTDOWN, SAFE_SHUTDOWN_EN,   1 ));

    /* poll loop to shutdown the GCA so SCB/MCP traffic is correctly handled prior to
       sw_init */
    /* both SAFE_SHUTDOWN_ACK1 & SAFE_SHUTDOWN_ACK (bits 0 & 1) */
    while( BREG_Read32( hRegister, BCHP_V3D_GCA_SAFE_SHUTDOWN_ACK) != 0x3 )
    {};

    /* clear any pending interrupts */
    BREG_Write32( hRegister, BCHP_V3D_CTL_INTCTL, ~0);
    BREG_Write32( hRegister, BCHP_V3D_DBG_DBQITC, ~0);

#if BCHP_PWR_HW_V3D
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D, false);
#endif
#if BCHP_PWR_HW_V3D_SRAM
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D_SRAM, false);
#endif
#if BCHP_PWR_HW_PLL_MOCA_CH3
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_MOCA_CH3, false);
#endif
}
#endif

/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_ResetMagnumCores
    ( const BCHP_Handle                hChip )
{

    BREG_Handle  hRegister = hChip->regHandle;
#if !defined(EMULATION)
    BCHP_P_ResetRaagaCore(hChip, hRegister); /* must be done first before all other cores. */
    BCHP_P_ResetV3dCore(hChip, hRegister);
#endif
    /* Reset some cores. This is needed to avoid L1 interrupts before BXXX_Open can be called per core. */
    /* Note, SW_INIT set/clear registers don't need read-modify-write. */
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, xpt_sw_init,    1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, hvd0_sw_init,   1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, hvd1_sw_init,   1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, vec_sw_init,    1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, aio_sw_init,    1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, bvn_sw_init,    1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, raaga0_sw_init, 1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, raaga1_sw_init, 1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, gfx_sw_init,    1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, dvp_ht_sw_init, 1));

    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET,
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, vice20_sw_init, 1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, vice21_sw_init, 1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, hvd2_sw_init,   1 ) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, v3d_top_sw_init,1 ));

    BREG_Write32(hChip->regHandle, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET,
          BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, sid_sw_init, 1 )
         );

    /* Now clear the reset. */
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, xpt_sw_init,    1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, hvd0_sw_init,   1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, hvd1_sw_init,   1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, vec_sw_init,    1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, aio_sw_init,    1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, bvn_sw_init,    1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, raaga0_sw_init, 1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, raaga1_sw_init, 1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, gfx_sw_init,    1) |
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, dvp_ht_sw_init, 1));

        BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR,
        BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_CLEAR, vice20_sw_init, 1 ) |
        BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_CLEAR, vice21_sw_init, 1 ) |
        BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_CLEAR, hvd2_sw_init,   1 ) |
        BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_CLEAR, v3d_top_sw_init,1 ));

        BREG_Write32(hChip->regHandle, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR,
           BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_CLEAR, sid_sw_init, 1)
         );

    return BERR_SUCCESS;
}

/* End of File */

