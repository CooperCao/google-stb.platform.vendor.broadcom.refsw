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
#include "bchp_7346.h"
#include "bchp_pwr_resources_priv.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_decode_sd_0.h"
#include "bchp_sun_gisb_arb.h"
#include "bchp_decode_ip_shim_0.h"
#include "bchp_gfx_gr.h"
#include "bchp_v3d_dbg.h"
#include "bchp_v3d_ctl.h"

#include "bchp_memc_ddr23_shim_addr_cntl_0.h"
#include "bchp_memc_ddr_0.h"

#include "bchp_pwr.h"

#include "bchp_avs_ro_registers_0.h"
#include "bchp_avs.h"

#include "bchp_aon_hdmi_tx.h"

BDBG_MODULE(BCHP);

/* Miscellaneous macros. */
#define BCHP_P_MAJOR_REV_SHIFT          (4)

/* Chip info and features */
typedef struct BCHP_P_7346_Info
{
    uint32_t      ulChipIdReg; /* index into the table. */

    /* Chip Id */
    uint16_t      usChipId;

    /* Major revision */
    uint16_t      usMajor;

    /* Minor revision */
    uint16_t      usMinor;

    /* TODO: Other features or infos if needed */
} BCHP_P_7346_Info;


/* Lookup table for chip features and etc.
 * The are many times when the chip device id register
 * not conforming to the standard numbering convention. We do
 * it this way to work-around those problems.
 *
 * TODO: Update this table to support new revisions.
 */
static const BCHP_P_7346_Info s_aChipInfoTable[] =
{
    /* Chip Family contains the major and minor revs */
#if BCHP_VER == BCHP_VER_A0
	/* A0 code will run on A0 */
   {0x73460000, BCHP_BCM7346, BCHP_MAJOR_A, BCHP_MINOR_0},
   {0x73560000, BCHP_BCM7346, BCHP_MAJOR_A, BCHP_MINOR_0},
#elif BCHP_VER == BCHP_VER_B0 || BCHP_VER == BCHP_VER_B1 || BCHP_VER == BCHP_VER_B2
	/* B0 code will run on B0 only */
   {0x73460010, BCHP_BCM7346, BCHP_MAJOR_B, BCHP_MINOR_0},
   {0x73560010, BCHP_BCM7346, BCHP_MAJOR_B, BCHP_MINOR_0},
   {0x73460011, BCHP_BCM7346, BCHP_MAJOR_B, BCHP_MINOR_1},
   {0x73560011, BCHP_BCM7346, BCHP_MAJOR_B, BCHP_MINOR_1},
   {0x73460012, BCHP_BCM7346, BCHP_MAJOR_B, BCHP_MINOR_2},
   {0x73560012, BCHP_BCM7346, BCHP_MAJOR_B, BCHP_MINOR_2},
#else
    #error "Port required"
#endif
};


/* Chip context */
typedef struct BCHP_P_7346_Context
{
    uint32_t                           ulBlackMagic;
    BREG_Handle                        hRegister;
    const BCHP_P_7346_Info            *pChipInfo;
    BCHP_P_AvsHandle                  hAvsHandle;
} BCHP_P_7346_Context;

/* Max entry of lookup table */
#define BCHP_P_CHIP_INFO_MAX_ENTRY \
    (sizeof(s_aChipInfoTable) / sizeof(BCHP_P_7346_Info))

/* This macro checks for a validity of a handle, and
 * cast to context pointer. */
#define BCHP_P_GET_CONTEXT(handle, context) \
{ \
    if(!(handle) || \
       !((handle)->chipHandle) || \
       (((BCHP_P_7346_Context*)((handle)->chipHandle))->ulBlackMagic != \
       sizeof(BCHP_P_7346_Context))) \
    { \
        BDBG_ERR(("Corrupted context handle\n")); \
        (context) = NULL; \
    } \
    else \
    { \
        (context) = (BCHP_P_7346_Context*)((handle)->chipHandle); \
    } \
    BDBG_ASSERT(context); \
}

/* Static function prototypes */
static BERR_Code BCHP_P_Close7346
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
    ( const BCHP_Handle hChip );

static void BCHP_P_ResetRaagaCore
    ( const BCHP_Handle hChip, const BREG_Handle hReg );

static void BCHP_P_ResetV3dCore
    ( const BCHP_Handle                hChip,
      const BREG_Handle                hReg );

static void BCHP_P_MonitorPvt
    ( BCHP_Handle                      hChip,
      BCHP_AvsSettings                *pSettings );

static BERR_Code BCHP_P_GetAvsData
    ( BCHP_Handle                      hChip,
      BCHP_AvsData                    *pData );

static BERR_Code BCHP_P_StandbyMode
    ( BCHP_Handle                      hChip,
      bool                             activate );

static void BCHP_P_ResetGfxCore
    ( const BCHP_Handle                hChip,
      const BREG_Handle                hReg );


/***************************************************************************
 * Open BCM7346 Chip.
 *
 */
BERR_Code BCHP_Open7346
    ( BCHP_Handle                     *phChip,
      BREG_Handle                      hRegister )
{
    BCHP_P_Context *pChip;
    BCHP_P_7346_Context *p7346Chip;
    uint32_t ulChipIdReg;
    uint32_t ulIdx;
    BERR_Code rc;

    BDBG_ENTER(BCHP_Open7346);

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

    p7346Chip = (BCHP_P_7346_Context*)
        (BKNI_Malloc(sizeof(BCHP_P_7346_Context)));
    if(!p7346Chip)
    {
        BKNI_Free(pChip);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)p7346Chip, 0x0, sizeof(BCHP_P_7346_Context));

    /* Fill up the base chip context. */
    pChip->chipHandle       = (void*)p7346Chip;
    pChip->regHandle        = hRegister;
    pChip->pCloseFunc       = BCHP_P_Close7346;
    pChip->pGetChipInfoFunc = BCHP_P_GetChipInfoComformWithBaseClass;
    pChip->pGetFeatureFunc  = BCHP_P_GetFeature;
    pChip->pMonitorPvtFunc  = BCHP_P_MonitorPvt;
    pChip->pGetAvsDataFunc  = BCHP_P_GetAvsData;
	pChip->pStandbyModeFunc = BCHP_P_StandbyMode;

    /* Fill up the chip context. */
    p7346Chip->ulBlackMagic = sizeof(BCHP_P_7346_Context);
    p7346Chip->hRegister    = hRegister;

    /* Chip Family Register id is use for indexing into the table. */
    ulChipIdReg = BREG_Read32(hRegister, BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID);

/* decompose 32 bit chip family id for use with printf format string %x%c%d
Example: 0x74250000 becomes "7425A0" */
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
        const BCHP_P_7346_Info *compareChipInfo = &s_aChipInfoTable[ulIdx];

        if(compareChipInfo->ulChipIdReg == ulChipIdReg)
        {
            /* Chip Information. */
            p7346Chip->pChipInfo = compareChipInfo;
            break;
        }
        else if (ulIdx == BCHP_P_CHIP_INFO_MAX_ENTRY - 1 && compareChipInfo->usMajor == (ulChipIdReg&0xF0)>>4)
        {
            /* This is a future minor revision. We will allow it with a WRN. */
            BDBG_WRN(("An unknown minor revision %x%c%d has been detected. Certain operations may result in erratic behavior. Please confirm this chip revision is supported with this software.",
                PRINT_CHIP(ulChipIdReg)));
            p7346Chip->pChipInfo = compareChipInfo;
            break;
        }
    }

    if(!p7346Chip->pChipInfo)
    {
        BKNI_Free(p7346Chip);
        BKNI_Free(pChip);
    	BDBG_ERR(("*****************************************************************\n"));
    	BDBG_ERR(("ERROR ERROR ERROR ERROR \n"));
        BDBG_ERR(("Unsupported Revision: %x%c%d", PRINT_CHIP(ulChipIdReg)));
    	BDBG_ERR(("*****************************************************************\n"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("found %x%c%d", PRINT_CHIP(p7346Chip->pChipInfo->ulChipIdReg)));

    /* Open BCHP_PWR - but first, do a reset on some of the Magnum controlled cores so
     * that they don't interfere with BCHP_PWR_Open's powering up/down.
     */
	BCHP_P_ResetMagnumCores(pChip);

     /* Open BCHP_PWR */
    rc = BCHP_PWR_Open(&pChip->pwrManager, pChip);
    if (rc) {
        BKNI_Free(pChip);
        BKNI_Free(p7346Chip);
        return BERR_TRACE(rc);
    }

    /* Open AVS module */
    BCHP_P_AvsOpen(&p7346Chip->hAvsHandle, pChip);
    if(!p7346Chip->hAvsHandle)
    {
    	BCHP_PWR_Close(pChip->pwrManager);
        BKNI_Free(pChip);
        BKNI_Free(p7346Chip);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* All done. now return the new fresh context to user. */
    *phChip = (BCHP_Handle)pChip;

    /* Clear AVD/SVD shutdown enable bit */
#if BCHP_PWR_RESOURCE_AVD0
	BCHP_PWR_AcquireResource(pChip, BCHP_PWR_RESOURCE_AVD0);
#endif
	BREG_Write32(hRegister, BCHP_DECODE_IP_SHIM_0_SOFTSHUTDOWN_CTRL_REG, 0x0);
#if BCHP_PWR_RESOURCE_AVD0
	BCHP_PWR_ReleaseResource(pChip, BCHP_PWR_RESOURCE_AVD0);
#endif

	/* Set CEC_ADDR fields to Unregistered to fix the incorrect Reset values of 14 (Specific Use) */
	{
        uint32_t CecReg = BREG_Read32(hRegister, BCHP_AON_HDMI_TX_CEC_CNTRL_1);
        CecReg &= ~BCHP_AON_HDMI_TX_CEC_CNTRL_1_CEC_ADDR_MASK;
        CecReg |= 0xF << BCHP_AON_HDMI_TX_CEC_CNTRL_1_CEC_ADDR_SHIFT;
        BREG_Write32(hRegister, BCHP_AON_HDMI_TX_CEC_CNTRL_1, CecReg);

        CecReg = BREG_Read32(hRegister, BCHP_AON_HDMI_TX_CEC_CNTRL_6);
        CecReg &= ~(BCHP_AON_HDMI_TX_CEC_CNTRL_6_CEC_ADDR_1_MASK | BCHP_AON_HDMI_TX_CEC_CNTRL_6_CEC_ADDR_2_MASK);
        CecReg |= (0xF << BCHP_AON_HDMI_TX_CEC_CNTRL_6_CEC_ADDR_1_SHIFT) | (0xF << BCHP_AON_HDMI_TX_CEC_CNTRL_6_CEC_ADDR_2_SHIFT);
        BREG_Write32(hRegister, BCHP_AON_HDMI_TX_CEC_CNTRL_6, CecReg);
        BDBG_WRN(("%s: Setting CEC_ADDRxx register fields to 0x15 (Unregistered) address", __FUNCTION__));
	}
    BDBG_LEAVE(BCHP_Open7346);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_Close7346
    ( BCHP_Handle                      hChip )
{
    BCHP_P_7346_Context *p7346Chip;

    BDBG_ENTER(BCHP_P_Close7346);

    BCHP_P_GET_CONTEXT(hChip, p7346Chip);

    if(!p7346Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (p7346Chip->hAvsHandle) {
		BCHP_P_AvsClose(p7346Chip->hAvsHandle);
    	p7346Chip->hAvsHandle = NULL;
	}

    BCHP_PWR_Close(hChip->pwrManager);

    /* Invalidate the magic number. */
    p7346Chip->ulBlackMagic = 0;

    BKNI_Free((void*)p7346Chip);
    BKNI_Free((void*)hChip);

    BDBG_LEAVE(BCHP_P_Close7346);
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
    const BCHP_P_7346_Context *p7346Chip;

    BDBG_ENTER(BCHP_P_GetChipInfo);

    BCHP_P_GET_CONTEXT(hChip, p7346Chip);

    if(!p7346Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(pusChipId)
    {
        *pusChipId = p7346Chip->pChipInfo->usChipId;
    }

    if(pusChipMajorRev)
    {
        *pusChipMajorRev = p7346Chip->pChipInfo->usMajor;
    }

    if(pusChipMinorRev)
    {
        *pusChipMinorRev = p7346Chip->pChipInfo->usMinor;
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
    BCHP_P_7346_Context *p7346Chip;
    uint32_t             ulBondStatus;

    BDBG_ENTER(BCHP_P_GetFeature);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7346Chip);

    /* read bond-out status common for many features */
    ulBondStatus = BREG_Read32(p7346Chip->hRegister,
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

    default:
        rc = BCHP_P_GetDefaultFeature(hChip, eFeature, pFeatureValue);
        break;
    }

    /* return result */
    BDBG_LEAVE(BCHP_P_GetFeature);
    return rc;
}

static BERR_Code BCHP_P_ResetMagnumCores(const BCHP_Handle hChip)
{
    BREG_Handle  hRegister = hChip->regHandle;

	BCHP_P_ResetRaagaCore(hChip, hRegister); /* must be done before ResetMagnumCores() */
	BCHP_P_ResetGfxCore(hChip, hRegister);
    BCHP_P_ResetV3dCore(hChip, hRegister);

    /* Reset some cores. This is needed to avoid L1 interrupts before BXXX_Open can be called per core. */
    /* Note, SW_INIT set/clear registers don't need read-modify-write. */
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, xpt_sw_init, 1 )
         | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, svd0_sw_init, 1 )
         | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, vec_sw_init, 1 )
         | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, aio_sw_init, 1 )
         | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, bvn_sw_init, 1 )
         | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, raaga_sw_init, 1)
         | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, dvp_ht_sw_init, 1));


    /* Now clear the reset. */
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, xpt_sw_init, 1)
         | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, svd0_sw_init, 1)
         | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, vec_sw_init, 1)
         | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, aio_sw_init, 1)
         | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, bvn_sw_init, 1)
         | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, raaga_sw_init, 1)
         | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, dvp_ht_sw_init, 1));


    return BERR_SUCCESS;
}


#include "bchp_pwr_resources_priv.h"
#include "bchp_raaga_dsp_misc.h"


/* SW workaround to ensure we can hit the Raaga SW_INIT safely */
static void BCHP_P_ResetRaagaCore(const BCHP_Handle hChip, const BREG_Handle hReg)
{
    uint32_t val;

    /* unconditionally turn on everything that's needed to do the register write below.
       we don't know what power state we were left in. BCHP_PWR_Open() will later turn stuff off as needed */
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_RAAGA, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_RAAGA_PLL_CH0, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_CLK, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_DSP, true);
    /* BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_RAAGA0_SRAM, true); */

    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT) ;
	val = (val & ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT)))|
	 (BCHP_FIELD_DATA(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT,1));
	BREG_Write32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT, val);

	val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_REVISION) ;
	val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_REVISION) ;

	BDBG_MSG(("REV ID VAL = 0x%x", val));

    val = BREG_Read32(hReg, BCHP_RAAGA_DSP_MISC_SOFT_INIT);
    val &= ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, INIT_PROC_B));
    BREG_Write32(hReg, BCHP_RAAGA_DSP_MISC_SOFT_INIT, val);

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
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_GFX_108M, true);

	BREG_Write32(hReg, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_DATA(GFX_GR_SW_INIT_0, SID_CLK_108_SW_INIT, 1));
	BREG_Write32(hReg, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_DATA(GFX_GR_SW_INIT_0, SID_CLK_108_SW_INIT, 0));
}

static void BCHP_P_ResetV3dCore( const BCHP_Handle hChip, const BREG_Handle hReg )
{
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_MOCA_CH4, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_GFX_108M, true);

    BREG_Write32(hReg, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, V3D_TOP_CLK_108_SW_INIT, ASSERT));
    BREG_Write32(hReg, BCHP_GFX_GR_SW_INIT_0, BCHP_FIELD_ENUM(GFX_GR_SW_INIT_0, V3D_TOP_CLK_108_SW_INIT, DEASSERT));

    BREG_Write32(hReg, BCHP_V3D_CTL_INTCTL, ~0);
    BREG_Write32(hReg, BCHP_V3D_DBG_DBQITC, ~0);

    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D, false);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_GFX_108M, false);
}

/* This gets called regularly to handle the AVS processing */
static void BCHP_P_MonitorPvt( BCHP_Handle hChip, BCHP_AvsSettings *pSettings )
{
    BCHP_P_7346_Context *p7346Chip;

    BDBG_ENTER(BCHP_P_MonitorPvt);
    BSTD_UNUSED(pSettings);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7346Chip);

    if (p7346Chip->hAvsHandle)
    	BCHP_P_AvsMonitorPvt(p7346Chip->hAvsHandle);

    BDBG_LEAVE(BCHP_P_MonitorPvt);
}

static BERR_Code BCHP_P_GetAvsData( BCHP_Handle hChip, BCHP_AvsData *pData )
{
    BCHP_P_7346_Context *p7346Chip;

    BDBG_ASSERT(pData);

    BDBG_ENTER(BCHP_GetAVdata);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7346Chip);

    if (p7346Chip->hAvsHandle)
    	BCHP_P_AvsGetData(p7346Chip->hAvsHandle, pData);

    BDBG_LEAVE(BCHP_GetAVdata);
    return BERR_SUCCESS;
}

static BERR_Code BCHP_P_StandbyMode( BCHP_Handle hChip, bool activate )
{
    BCHP_P_7346_Context *p7346Chip;

    BDBG_ENTER(BCHP_P_StandbyMode);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7346Chip);

	/* Do anything required for CHP Standby changes */

    if (p7346Chip->hAvsHandle)
    	BCHP_P_AvsStandbyMode(p7346Chip->hAvsHandle, activate);

    BDBG_LEAVE(BCHP_P_StandbyMode);
    return BERR_SUCCESS;
}

/* End of File */
