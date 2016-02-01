/***************************************************************************
 *  Copyright (c) 2006-2013, Broadcom Corporation
 *  All Rights Reserved
 *  Confidential Property of Broadcom Corporation
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
#include "bchp_75635.h"

#include "bchp_aon_hdmi_tx.h"
#include "bchp_clkgen.h"
#include "bchp_pwr_resources_priv.h"
#include "bchp_raaga_dsp_misc.h"
#include "bchp_sun_top_ctrl.h"

BDBG_MODULE(BCHP);

/* Lookup table for chip features and etc.
 * The are many times when the chip device id register
 * not conforming to the standard numbering convention. We do
 * it this way to work-around those problems.
 *
 */
static const struct BCHP_P_Info s_aChipInfoTable[] =
{
    /* Chip Family contains the major and minor revs */
#if BCHP_VER == BCHP_VER_A0
    {0x07563500},
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
    ( const BCHP_Handle hChip );

static void BCHP_P_ResetRaagaCore
    ( const BCHP_Handle hChip,
      const BREG_Handle hReg );

BERR_Code BCHP_Open75635
    ( BCHP_Handle                     *phChip,
      BREG_Handle                      hRegister )
{
    BCHP_OpenSettings openSettings;

    BCHP_GetDefaultOpenSettings(&openSettings);
    openSettings.reg = hRegister;

    return BCHP_Open(phChip, &openSettings);
}

/***************************************************************************
 * Open BCM75635 Chip.
 *
 */
BERR_Code BCHP_Open( BCHP_Handle *phChip, const BCHP_OpenSettings *pSettings )
{
    BCHP_P_Context *pChip;
    BERR_Code rc;

    BDBG_ENTER(BCHP_Open75635);

    if (!phChip) {
        BDBG_ERR(("Invalid parameter\n"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* If error ocurr user get a NULL *phChip */
    *phChip = NULL;

    pChip = BCHP_P_Open(pSettings, s_aChipInfoTable);

    if (!pChip) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    pChip->pGetFeatureFunc  = BCHP_P_GetFeature;

    BCHP_P_ResetMagnumCores(pChip);

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

    /* Set CEC_ADDR fields to Unregistered to fix the incorrect Reset values of 14 (Specific Use) */
    {
        uint32_t CecReg = BREG_Read32(pChip->regHandle, BCHP_AON_HDMI_TX_CEC_CNTRL_1);
        CecReg &= ~BCHP_AON_HDMI_TX_CEC_CNTRL_1_CEC_ADDR_MASK;
        CecReg |= 0xF << BCHP_AON_HDMI_TX_CEC_CNTRL_1_CEC_ADDR_SHIFT;
        BREG_Write32(pChip->regHandle, BCHP_AON_HDMI_TX_CEC_CNTRL_1, CecReg);

        CecReg = BREG_Read32(pChip->regHandle, BCHP_AON_HDMI_TX_CEC_CNTRL_6);
        CecReg &= ~(BCHP_AON_HDMI_TX_CEC_CNTRL_6_CEC_ADDR_1_MASK | BCHP_AON_HDMI_TX_CEC_CNTRL_6_CEC_ADDR_2_MASK);
        CecReg |= (0xF << BCHP_AON_HDMI_TX_CEC_CNTRL_6_CEC_ADDR_1_SHIFT) | (0xF << BCHP_AON_HDMI_TX_CEC_CNTRL_6_CEC_ADDR_2_SHIFT);
        BREG_Write32(pChip->regHandle, BCHP_AON_HDMI_TX_CEC_CNTRL_6, CecReg);
        BDBG_MSG(("%s: Setting CEC_ADDRxx register fields to 0x15 (Unregistered) address", __FUNCTION__));
    }

    BDBG_LEAVE(BCHP_Open75635);
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

    /* Read bond-out status common for many features */
    ulBondStatus = BREG_Read32(hChip->regHandle,
        BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_0);

    /* Which feature ? */
    switch (eFeature)
    {
        case BCHP_Feature_e3DGraphicsCapable:
            /* 3D capable? (bool) */
            *(bool *)pFeatureValue = false;
            rc = BERR_SUCCESS;
            break;

        case BCHP_Feature_eDvoPortCapable:
            /* DVO port capable? (bool) */
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
        {
            /* Return frequency of AVD CPU - Used for UART speed calc. */
            uint32_t uiReg, avd_freq;

            uiReg = BREG_Read32(hChip->regHandle, BCHP_CLKGEN_PLL_AVD_MIPS_PLL_CHANNEL_CTRL_CH_2);
            avd_freq = BCHP_GET_FIELD_DATA(uiReg, CLKGEN_PLL_AVD_MIPS_PLL_CHANNEL_CTRL_CH_2, MDIV_CH2);
            if(avd_freq == 0)
                *(int *)pFeatureValue = 0;
            else
                *(int *)pFeatureValue = 2052/avd_freq;

            rc = BERR_SUCCESS;
            break;
        }

        default:
            rc = BCHP_P_GetDefaultFeature(hChip, eFeature, pFeatureValue);
            break;
    }

    /* Return result */
    BDBG_LEAVE(BCHP_P_GetFeature);
    return rc;
}

static BERR_Code BCHP_P_ResetMagnumCores(const BCHP_Handle hChip)
{
    BREG_Handle  hRegister = hChip->regHandle;

    BCHP_P_ResetRaagaCore(hChip, hRegister); /* must be done before ResetMagnumCores() */

    /* Reset some cores. This is needed to avoid L1 interrupts before BXXX_Open can be called per core. */
    /* Note, SW_INIT set/clear registers don't need read-modify-write. */
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,
                 BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, xpt_sw_init, 1 )
                 | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, avd0_sw_init, 1 )
                 | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, vec_sw_init, 1 )
                 | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, aio_sw_init, 1 )
                 | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, bvn_sw_init, 1 )
                 | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, raaga_sw_init, 1)
                 | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, dvp_ht_sw_init, 1));

    /* Now clear the reset. */
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,
                 BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, xpt_sw_init, 1)
                 | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, avd0_sw_init, 1)
                 | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, vec_sw_init, 1)
                 | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, aio_sw_init, 1)
                 | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, bvn_sw_init, 1)
                 | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, raaga_sw_init, 1)
                 | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, dvp_ht_sw_init, 1));

    return BERR_SUCCESS;
}

/* SW workaround to ensure we can hit the Raaga SW_INIT safely */
static void BCHP_P_ResetRaagaCore(const BCHP_Handle hChip, const BREG_Handle hReg)
{
    uint32_t val;

    /* unconditionally turn on everything that's needed to do the register write below.
       we don't know what power state we were left in. BCHP_PWR_Open() will later turn stuff off as needed */
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_AVD, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_AVD_CH3, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_CLK, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_DSP, true);
    /* BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_SRAM, true); */

    val = BREG_Read32(hReg, BCHP_RAAGA_DSP_MISC_SOFT_INIT);
    val &= ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, INIT_PROC_B));
    BREG_Write32(hReg, BCHP_RAAGA_DSP_MISC_SOFT_INIT, val);
    return;
}

/* End of File */
