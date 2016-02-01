/***************************************************************************
 *     (c)2003-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
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
 **************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "breg_mem.h"
#include "bchp.h"
#include "bchp_priv.h"
#include "bchp_75845.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_hevd_ol_ctl_0.h"
#include "bchp_sun_gisb_arb.h"
#include "bchp_memc_ddr23_shim_addr_cntl_0.h"
#include "bchp_memc_ddr_0.h"
#include "bchp_clkgen.h"
#include "bchp_pwr.h"
#include "bchp_pwr_resources_priv.h"
#include "bchp_avs_ro_registers_0.h"
#include "bchp_avs.h"
#include "bchp_pwr_resources_priv.h"
#include "bchp_raaga_dsp_misc.h"
#include "bchp_sid_gr.h"
#include "bchp_dvp_ht.h"

BDBG_MODULE(BCHP);


static const struct BCHP_P_Info s_aChipInfoTable[] =
{
#if (BCHP_VER == BCHP_VER_A0)
   {0x07584500},
#else
    #error "Port required"
#endif
    {0}
};


/* Static function prototypes */
static BERR_Code BCHP_P_GetFeature
    ( const BCHP_Handle                hChip,
      const BCHP_Feature               eFeature,
      void                            *pFeatureValue );

static BERR_Code BCHP_P_ResetMagnumCores
    ( const BCHP_Handle                hChip );


static void BCHP_P_ResetRaagaCore
    (const BCHP_Handle                 hChip,
     const BREG_Handle                 hReg);

static void BCHP_P_ResetGfxCore
    ( const BCHP_Handle                hChip,
      const BREG_Handle                hReg );

BERR_Code BCHP_Open75845
    ( BCHP_Handle                     *phChip,
      BREG_Handle                      hRegister )
{
    BCHP_OpenSettings openSettings;

    BCHP_GetDefaultOpenSettings(&openSettings);
    openSettings.reg = hRegister;

    return BCHP_Open(phChip, &openSettings);
}


/***************************************************************************
 * Open BCM75845 Chip.
 *
 */
BERR_Code BCHP_Open (BCHP_Handle * phChip, const BCHP_OpenSettings * pSettings)
{
    BCHP_P_Context *pChip;
#if BCHP_PWR_SUPPORT
    BERR_Code rc;
#endif

    BDBG_ENTER(BCHP_Open75845);

    if (!phChip)
    {
        BDBG_ERR(("Invalid parameter\n"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* If error ocurr user get a NULL *phChip */
    *phChip = NULL;

    pChip = BCHP_P_Open(pSettings, s_aChipInfoTable);
    if (!pChip) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Fill up the base chip context. */
    pChip->pGetFeatureFunc  = BCHP_P_GetFeature;

    BCHP_P_ResetMagnumCores(pChip);

#if BCHP_PWR_SUPPORT
    /* Open BCHP_PWR */
    rc = BCHP_PWR_Open(&pChip->pwrManager, pChip);
    if (rc) {
        BKNI_Free(pChip);
        return BERR_TRACE(rc);
    }
#endif

    /* Open AVS module */
    BCHP_P_AvsOpen(&pChip->hAvsHandle, pChip);
    if(!pChip->hAvsHandle)
    {
#if BCHP_PWR_SUPPORT
        BCHP_PWR_Close(pChip->pwrManager);
#endif
        BKNI_Free(pChip);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* All done. now return the new fresh context to user. */
    *phChip = (BCHP_Handle)pChip;

    /* Now, if we have power management, do a second reset of the Magnum cores. This second reset is
     * necessary to reset the cores that might have been powered down during the first reset above.
     * Make sure the Magnum hardware is powered up before the reset, then power it down afterwards.
     */
#ifdef BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED
    BCHP_PWR_AcquireResource(pChip, BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED);
#endif

#if BCHP_PWR_SUPPORT
    BCHP_P_ResetMagnumCores( pChip );
#endif

    /* Clear AVD/SVD shutdown enable bit */
#if 0 /* to check: whether it's still needed for new video decoder */
    BREG_Write32(pChip->regHandle, BCHP_HEVD_OL_CTL_0_SOFTSHUTDOWN_CTRL, 0x0);
#endif

#ifdef BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED
    BCHP_PWR_ReleaseResource(pChip, BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED);
#endif

    BDBG_LEAVE(BCHP_Open75845);
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
    uint32_t             uiReg;
    uint32_t             avd_freq;

    BDBG_ENTER(BCHP_P_GetFeature);

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
        *(int *)pFeatureValue = 1;
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

    case BCHP_Feature_eAVDCoreFreq:

       uiReg = BREG_Read32(hChip->regHandle, BCHP_CLKGEN_PLL_AVD_PLL_CHANNEL_CTRL_CH_2);
       avd_freq = BCHP_GET_FIELD_DATA(uiReg, CLKGEN_PLL_AVD_PLL_CHANNEL_CTRL_CH_2, MDIV_CH2);
       if(avd_freq == 0)
           *(int *)pFeatureValue = 0;
       else
           *(int *)pFeatureValue = 3006/avd_freq;

       rc = BERR_SUCCESS;
       break;

    case BCHP_Feature_eRfmCapable:
        /* RFM capable? (bool) */
        if(hChip->info.productId == BCHP_BCM75835)
        {
            *(bool *)pFeatureValue = false;
        }
        else
        {
            *(bool *)pFeatureValue = true;
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

/***************************************************************************
 * Public function:
 *  Be called in bint_75845.c since bchp handle can not pass to bint module
 */
static void BCHP_P_ResetRaagaCore(const BCHP_Handle hChip, const BREG_Handle hReg)
{
    uint32_t val = 0;

    /* unconditionally turn on everything that's needed to do the register write below.
       we don't know what power state we were left in. BCHP_PWR_Open() will later turn stuff off as needed */
#if BCHP_PWR_SUPPORT
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_AVD, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_AVD_CH2, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_108_CLK, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_DSP_CLK, true);
    /* BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_SRAM, true); */
#else
    BSTD_UNUSED(hChip);
#endif

    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT) ;
    val = (val & ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT)))|
     (BCHP_FIELD_DATA(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT,1));
    BREG_Write32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT, val);

    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_REVISION) ;
    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_REVISION) ;

    BDBG_MSG(("REV ID VAL = 0x%x", val));

    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT) ;
    val &=  ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT,INIT_PROC_B));
    BREG_Write32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT, val);

    /*RDB says no need of Read modify write.*/
    val = 0;
    val = (BCHP_FIELD_DATA(SUN_TOP_CTRL_SW_INIT_0_SET, raaga_sw_init,1));
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_SW_INIT_0_SET, val);

    BKNI_Delay(2);

    /*RDB says no need of Read modify write.*/
    val = 0;
    val = (BCHP_FIELD_DATA(SUN_TOP_CTRL_SW_INIT_0_CLEAR, raaga_sw_init,1));
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR, val);

    return;
}

/* Needed for resetting SID block correctly */
static void BCHP_P_ResetGfxCore
    ( const BCHP_Handle                hChip,
      const BREG_Handle                hReg )
{
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_SID, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_AVD0_108_CLK, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_AVD0_SCB_CLK, true);

    BREG_Write32(hReg, BCHP_SID_GR_SW_INIT_0, BCHP_FIELD_DATA(SID_GR_SW_INIT_0, SID_SW_INIT, 1));
    BREG_Write32(hReg, BCHP_SID_GR_SW_INIT_0, BCHP_FIELD_DATA(SID_GR_SW_INIT_0, SID_SW_INIT, 0));
}

static BERR_Code BCHP_P_ResetMagnumCores
    ( const BCHP_Handle                hChip )

{
    BREG_Handle  hRegister = hChip->regHandle;

    /* Workaround with 33-2.5 kernel. */
#if !BCHP_PWR_SUPPORT
    uint32_t mask;

    mask = BCHP_CLKGEN_RAAGA_DSP_TOP_CLOCK_ENABLE_RAAGA_108_CLOCK_ENABLE_MASK;
    BREG_AtomicUpdate32(hRegister, BCHP_CLKGEN_RAAGA_DSP_TOP_CLOCK_ENABLE, mask, mask);

    mask = (BCHP_CLKGEN_SYS_CTRL_CLOCK_DISABLE_DISABLE_SC0_CLOCK_MASK |
        BCHP_CLKGEN_SYS_CTRL_CLOCK_DISABLE_DISABLE_SC1_CLOCK_MASK);
    BREG_AtomicUpdate32(hRegister, BCHP_CLKGEN_SYS_CTRL_CLOCK_DISABLE, mask, 0);
#endif

    BCHP_P_ResetRaagaCore(hChip, hChip->regHandle);
    BCHP_P_ResetGfxCore(hChip, hRegister);

    /* Reset some cores. This is needed to avoid L1 interrupts before BXXX_Open can be called per core. */
    /* Note, SW_INIT set/clear registers don't need read-modify-write. */
    BREG_Write32(hChip->regHandle, BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,
           BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, xpt_sw_init, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, avd0_sw_init, 1 )    /* avd0_sw_init */
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, vec_sw_init, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, aio_sw_init, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, bvn_sw_init, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, raaga_sw_init, 1)
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, dvp_ht_sw_init, 1));

    /* Now clear the reset. */
    BREG_Write32(hChip->regHandle, BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,
           BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, xpt_sw_init, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, avd0_sw_init, 1 )    /* avd0_sw_init */
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, vec_sw_init, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, aio_sw_init, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, bvn_sw_init, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, raaga_sw_init, 1)
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, dvp_ht_sw_init, 1));

    /* Note, SW_INIT set/clear registers don't need read-modify-write. */
    BREG_Write32(hChip->regHandle, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET,
        BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, sid_sw_init, 1 ));

    /* Now clear the reset. */
    BREG_Write32(hChip->regHandle, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR,
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_CLEAR, sid_sw_init, 1));

    return BERR_SUCCESS;
}

/* End of File */
