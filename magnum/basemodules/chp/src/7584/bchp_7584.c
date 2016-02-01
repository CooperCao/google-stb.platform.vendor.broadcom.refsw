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
#include "bchp_7584.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_decode_sd_0.h"
#include "bchp_decode_ip_shim_0.h"
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

/* Miscellaneous macros. */
#define BCHP_P_MAJOR_REV_SHIFT          (4)

/* Chip info and features */
typedef struct BCHP_P_7584_Info
{
    uint32_t      ulChipIdReg; /* index into the table. */

    /* Chip Id */
    uint16_t      usChipId; /* PRODUCT_ID */

    /* Major revision */
    uint16_t      usMajor;

    /* Minor revision */
    uint16_t      usMinor;

    /* TODO: Other features or infos if needed */
} BCHP_P_7584_Info;


/* Lookup table for chip features and etc.
 * The are many times when the chip device id register
 * not conforming to the standard numbering convention. We do
 * it this way to work-around those problems.
 *
 * TODO: Update this table to support new revisions.
 */
static const BCHP_P_7584_Info s_aChipInfoTable[] =
{
#if ((BCHP_VER == BCHP_VER_A0) || (BCHP_VER == BCHP_VER_A1))
    /* A0 code will run on A0, A1 */
   {0x75840000, BCHP_BCM7584, BCHP_MAJOR_A, BCHP_MINOR_0},
   {0x75830000, BCHP_BCM7583, BCHP_MAJOR_A, BCHP_MINOR_0},
   {0x75760000, BCHP_BCM7576, BCHP_MAJOR_A, BCHP_MINOR_0},
   {0x75840001, BCHP_BCM7584, BCHP_MAJOR_A, BCHP_MINOR_1},
   {0x75830001, BCHP_BCM7583, BCHP_MAJOR_A, BCHP_MINOR_1},
   {0x75760001, BCHP_BCM7576, BCHP_MAJOR_A, BCHP_MINOR_1},
#else
    #error "Port required"
#endif
};

/* Chip context */
typedef struct BCHP_P_7584_Context
{
    uint32_t                           ulBlackMagic;
    BREG_Handle                        hRegister;
    const BCHP_P_7584_Info            *pChipInfo;
    BCHP_P_AvsHandle                  hAvsHandle;
} BCHP_P_7584_Context;

/* Max entry of lookup table */
#define BCHP_P_CHIP_INFO_MAX_ENTRY \
    (sizeof(s_aChipInfoTable) / sizeof(BCHP_P_7584_Info))

/* This macro checks for a validity of a handle, and
 * cast to context pointer. */
#define BCHP_P_GET_CONTEXT(handle, context) \
{ \
    if(!(handle) || \
       !((handle)->chipHandle) || \
       (((BCHP_P_7584_Context*)((handle)->chipHandle))->ulBlackMagic != \
       sizeof(BCHP_P_7584_Context))) \
    { \
        BDBG_ERR(("Corrupted context handle\n")); \
        (context) = NULL; \
    } \
    else \
    { \
        (context) = (BCHP_P_7584_Context*)((handle)->chipHandle); \
    } \
    BDBG_ASSERT(context); \
}

/* Static function prototypes */
static BERR_Code BCHP_P_Close7584
    ( BCHP_Handle                      hChip );

static BERR_Code BCHP_P_GetChipInfoComformWithBaseClass
    ( const BCHP_Handle                hChip,
      uint16_t                        *pusChipId,
      uint16_t                        *pusChipRev );

static BERR_Code BCHP_P_GetChipInfo
    ( const BCHP_Handle                hChip,
      uint16_t                        *pusChipId,
      uint16_t                        *pusChipMajorRev,
      uint16_t                        *pusChipMinorRev );

static BERR_Code BCHP_P_GetFeature
    ( const BCHP_Handle                hChip,
      const BCHP_Feature               eFeature,
      void                            *pFeatureValue );

static BERR_Code BCHP_P_ResetMagnumCores
    ( const BCHP_Handle                hChip );

static void BCHP_P_MonitorPvt
    ( BCHP_Handle                      hChip,
      BCHP_AvsSettings                *pSettings );

static BERR_Code BCHP_P_GetAvsData
    ( BCHP_Handle                      hChip,
      BCHP_AvsData                    *pData );

static BERR_Code BCHP_P_StandbyMode
    ( BCHP_Handle                      hChip,
      bool                             activate );

static void BCHP_P_ResetRaagaCore
    (const BCHP_Handle                 hChip,
     const BREG_Handle                 hReg);

static void BCHP_P_ResetGfxCore
    ( const BCHP_Handle                hChip,
      const BREG_Handle                hReg );

/***************************************************************************
 * Open BCM7584 Chip.
 *
 */
BERR_Code BCHP_Open7584
    ( BCHP_Handle                     *phChip,
      BREG_Handle                      hRegister )
{
    BCHP_P_Context *pChip;
    BCHP_P_7584_Context *p7584Chip;
    uint32_t ulChipIdReg;
    uint32_t ulIdx;
    BERR_Code rc;

    BDBG_ENTER(BCHP_Open7584);

    if((!phChip) ||
       (!hRegister))
    {
        BDBG_ERR(("Invalid parameter\n"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* If error ocurr user get a NULL *phChip */
    *phChip = NULL;

    /* Alloc the base chip context. */
    pChip = (BCHP_P_Context*)(BKNI_Malloc(sizeof(BCHP_P_Context)));
    if(!pChip)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pChip, 0x0, sizeof(BCHP_P_Context));

    p7584Chip = (BCHP_P_7584_Context*)
        (BKNI_Malloc(sizeof(BCHP_P_7584_Context)));
    if(!p7584Chip)
    {
        BKNI_Free(pChip);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)p7584Chip, 0x0, sizeof(BCHP_P_7584_Context));

    /* Fill up the base chip context. */
    pChip->chipHandle       = (void*)p7584Chip;
    pChip->regHandle        = hRegister;
    pChip->pCloseFunc       = BCHP_P_Close7584;
    pChip->pGetChipInfoFunc = BCHP_P_GetChipInfoComformWithBaseClass;
    pChip->pGetFeatureFunc  = BCHP_P_GetFeature;
    pChip->pMonitorPvtFunc  = BCHP_P_MonitorPvt;
    pChip->pGetAvsDataFunc  = BCHP_P_GetAvsData;
    pChip->pStandbyModeFunc = BCHP_P_StandbyMode;

    /* Fill up the chip context. */
    p7584Chip->ulBlackMagic = sizeof(BCHP_P_7584_Context);
    p7584Chip->hRegister    = hRegister;

    /* Chip Family Register id is use for indexing into the table. */
    ulChipIdReg = BREG_Read32(hRegister, BCHP_SUN_TOP_CTRL_PRODUCT_ID);

/* decompose 32 bit chip id for use with printf format string %x%c%d
Example: 0x75840000 becomes "7584A0" */
#define PRINT_CHIP(CHIPID) \
    ((CHIPID)>>16), ((((CHIPID)&0xF0)>>4)+'A'), ((CHIPID)&0x0F)

    for(ulIdx = 0; ulIdx < BCHP_P_CHIP_INFO_MAX_ENTRY; ulIdx++)
    {
        BDBG_MSG(("Supported Chip Family and revision: %x%c%d", PRINT_CHIP(s_aChipInfoTable[ulIdx].ulChipIdReg)));
        BDBG_MSG(("Supported Chip ID: %x", s_aChipInfoTable[ulIdx].usChipId));
        BDBG_MSG(("\n"));
    }

    /* Lookup corresponding chip id. */
    for(ulIdx = 0; ulIdx < BCHP_P_CHIP_INFO_MAX_ENTRY; ulIdx++)
    {
        const BCHP_P_7584_Info *compareChipInfo = &s_aChipInfoTable[ulIdx];

        if(compareChipInfo->ulChipIdReg == ulChipIdReg)
        {
            /* Chip Information. */
            p7584Chip->pChipInfo = compareChipInfo;
            break;
        }
        else if (ulIdx == BCHP_P_CHIP_INFO_MAX_ENTRY - 1 && compareChipInfo->usMajor == (ulChipIdReg&0xF0)>>4)
        {
            /* This is a future minor revision. We will allow it with a WRN. */
            BDBG_WRN(("An unknown minor revision %x%c%d has been detected. Certain operations may result in erratic behavior. Please confirm this chip revision is supported with this software.",
                PRINT_CHIP(ulChipIdReg)));
            p7584Chip->pChipInfo = compareChipInfo;
            break;
        }
    }

    if(!p7584Chip->pChipInfo)
    {
        BKNI_Free(p7584Chip);
        BKNI_Free(pChip);
        BDBG_ERR(("*****************************************************************\n"));
        BDBG_ERR(("ERROR ERROR ERROR ERROR \n"));
        BDBG_ERR(("Unsupported Revision: %x%c%d", PRINT_CHIP(ulChipIdReg)));
        BDBG_ERR(("*****************************************************************\n"));
        phChip = NULL;
        BDBG_ASSERT(phChip);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("found %x%c%d", PRINT_CHIP(p7584Chip->pChipInfo->ulChipIdReg)));

    /* Open BCHP_PWR - but first, do a reset on some of the Magnum controlled cores so
     * that they don't interfere with BCHP_PWR_Open's powering up/down.
     */
    rc = BCHP_P_ResetMagnumCores( pChip );

#if BCHP_PWR_SUPPORT
    /* Open BCHP_PWR */
    rc = BCHP_PWR_Open(&pChip->pwrManager, pChip);
    if (rc) {
        BKNI_Free(pChip);
        BKNI_Free(p7584Chip);
        return BERR_TRACE(rc);
    }
#endif

    /* Open AVS module */
    BCHP_P_AvsOpen(&p7584Chip->hAvsHandle, pChip);
    if(!p7584Chip->hAvsHandle)
    {
#if BCHP_PWR_SUPPORT
        BCHP_PWR_Close(pChip->pwrManager);
#endif
        BKNI_Free(pChip);
        BKNI_Free(p7584Chip);
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
    BREG_Write32(hRegister, BCHP_DECODE_IP_SHIM_0_SOFTSHUTDOWN_CTRL_REG, 0x0);

#ifdef BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED
        BCHP_PWR_ReleaseResource(pChip, BCHP_PWR_RESOURCE_MAGNUM_CONTROLLED);
#endif

    BDBG_LEAVE(BCHP_Open7584);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_Close7584
    ( BCHP_Handle                      hChip )
{
    BCHP_P_7584_Context *p7584Chip;

    BDBG_ENTER(BCHP_P_Close7584);

    BCHP_P_GET_CONTEXT(hChip, p7584Chip);

    if(!p7584Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (p7584Chip->hAvsHandle) {
        BCHP_P_AvsClose(p7584Chip->hAvsHandle);
        p7584Chip->hAvsHandle = NULL;
    }
#if BCHP_PWR_SUPPORT
    /* Note: PWR_Close goes here (after AvsClose) */
    BCHP_PWR_Close(hChip->pwrManager);
#endif
    /* Invalidate the magic number. */
    p7584Chip->ulBlackMagic = 0;

    BKNI_Free((void*)p7584Chip);
    BKNI_Free((void*)hChip);

    BDBG_LEAVE(BCHP_P_Close7584);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_GetChipInfoComformWithBaseClass
    ( const BCHP_Handle                hChip,
      uint16_t                        *pusChipId,
      uint16_t                        *pusChipRev )

{
    BERR_Code eStatus;
    uint16_t usMajor=0;
    uint16_t usMinor=0;

    eStatus = BERR_TRACE(BCHP_P_GetChipInfo(hChip, pusChipId,
        &usMajor, &usMinor));
    if(BERR_SUCCESS != eStatus)
    {
        return eStatus;
    }

    if(pusChipRev)
    {
        *pusChipRev = ((usMajor << BCHP_P_MAJOR_REV_SHIFT) + usMinor);
    }

    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_GetChipInfo
    ( const BCHP_Handle                hChip,
      uint16_t                        *pusChipId,
      uint16_t                        *pusChipMajorRev,
      uint16_t                        *pusChipMinorRev )
{
    const BCHP_P_7584_Context *p7584Chip;

    BDBG_ENTER(BCHP_P_GetChipInfo);

    BCHP_P_GET_CONTEXT(hChip, p7584Chip);

    if(!p7584Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(pusChipId)
    {
        *pusChipId = p7584Chip->pChipInfo->usChipId;
    }

    if(pusChipMajorRev)
    {
        *pusChipMajorRev = p7584Chip->pChipInfo->usMajor;
    }

    if(pusChipMinorRev)
    {
        *pusChipMinorRev = p7584Chip->pChipInfo->usMinor;
    }

    BDBG_LEAVE(BCHP_P_GetChipInfo);
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
    BCHP_P_7584_Context *p7584Chip;
    uint32_t             ulBondStatus;
    uint32_t             uiReg;
    uint32_t             avd_freq;

    BDBG_ENTER(BCHP_P_GetFeature);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7584Chip);

    /* read bond-out status common for many features */
    ulBondStatus = BREG_Read32(p7584Chip->hRegister,
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

       uiReg = BREG_Read32(p7584Chip->hRegister, BCHP_CLKGEN_PLL_AVD_PLL_CHANNEL_CTRL_CH_2);
       avd_freq = BCHP_GET_FIELD_DATA(uiReg, CLKGEN_PLL_AVD_PLL_CHANNEL_CTRL_CH_2, MDIV_CH2);
       if(avd_freq == 0)
           *(int *)pFeatureValue = 0;
       else
           *(int *)pFeatureValue = 3006/avd_freq;

       rc = BERR_SUCCESS;
       break;

    case BCHP_Feature_eRfmCapable:
        /* RFM capable? (bool) */
        if(p7584Chip->pChipInfo->usChipId == BCHP_BCM7583)
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
 *  Be called in bint_7584.c since bchp handle can not pass to bint module
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

/* This gets called regularly to handle the AVS processing */
static void BCHP_P_MonitorPvt( BCHP_Handle hChip, BCHP_AvsSettings *pSettings )
{
    BCHP_P_7584_Context *p7584Chip;

    BDBG_ENTER(BCHP_P_MonitorPvt);
    BSTD_UNUSED(pSettings);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7584Chip);

    if (p7584Chip->hAvsHandle)
        BCHP_P_AvsMonitorPvt(p7584Chip->hAvsHandle);

    BDBG_LEAVE(BCHP_P_MonitorPvt);
}

/* This provides the current AVS data */
static BERR_Code BCHP_P_GetAvsData( BCHP_Handle hChip, BCHP_AvsData *pData )
{
    BCHP_P_7584_Context *p7584Chip;

    BDBG_ASSERT(pData);

    BDBG_ENTER(BCHP_GetAVdata);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7584Chip);

    if (p7584Chip->hAvsHandle)
        BCHP_P_AvsGetData(p7584Chip->hAvsHandle, pData);

    BDBG_LEAVE(BCHP_GetAVdata);
    return BERR_SUCCESS;
}

static BERR_Code BCHP_P_StandbyMode( BCHP_Handle hChip, bool activate )
{
    BCHP_P_7584_Context *p7584Chip;

    BDBG_ENTER(BCHP_P_StandbyMode);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7584Chip);

    /* Do anything required for CHP Standby changes */

    if (p7584Chip->hAvsHandle)
        BCHP_P_AvsStandbyMode(p7584Chip->hAvsHandle, activate);

    BDBG_LEAVE(BCHP_P_StandbyMode);
    return BERR_SUCCESS;
}

/* End of File */
