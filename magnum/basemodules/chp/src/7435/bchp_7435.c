/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 * Module Description:
 *
 ***************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "breg_mem.h"
#include "bchp.h"
#include "bchp_priv.h"
#include "bchp_7435.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_decode_sd_0.h"
#include "bchp_decode_sd_1.h"
#include "bchp_decode_ip_shim_0.h"
#include "bchp_decode_ip_shim_1.h"
#include "bchp_sun_gisb_arb.h"

#include "bchp_memc_ddr23_shim_addr_cntl_0.h"
#include "bchp_memc_ddr23_shim_addr_cntl_1.h"
#include "bchp_memc_ddr_0.h"
#include "bchp_memc_ddr_1.h"
#include "bchp_memc_arb_0.h"
#include "bchp_pwr.h"

#include "bchp_avs_ro_registers_0.h"
#include "bchp_avs.h"

#include "bchp_v3d_top_gr_bridge.h"
#include "bchp_v3d_dbg.h"
#include "bchp_v3d_ctl.h"
#include "bchp_vice2_misc_0.h"
#include "bchp_vice2_misc_1.h"
#include "bchp_vice2_l2_0.h"
#include "bchp_vice2_l2_1.h"

BDBG_MODULE(BCHP);

/* Miscellaneous macros. */
#define BCHP_P_MAJOR_REV_SHIFT          (4)

/* Chip info and features */
typedef struct BCHP_P_7435_Info
{
    uint32_t      ulChipFamilyIdReg; /* index into the table. */

    /* Chip Id */
    uint16_t      usChipId;

    /* Major revision */
    uint16_t      usMajor;

    /* Minor revision */
    uint16_t      usMinor;

    /* TODO: Other features or infos if needed */
} BCHP_P_7435_Info;


/* Lookup table for chip features and etc.
 * The are many times when the chip device id register
 * not conforming to the standard numbering convention. We do
 * it this way to work-around those problems.
 *
 * TODO: Update this table to support new revisions.
 */
static const BCHP_P_7435_Info s_aChipInfoTable[] =
{
    /* Chip Family contains the major and minor revs */
#if BCHP_VER == BCHP_VER_A0
    /* A0 code will run on A0 */
    {0x74350000, BCHP_BCM7435, BCHP_MAJOR_A, BCHP_MINOR_0},
#elif BCHP_VER == BCHP_VER_B0
    /* B0 code will run on B0 */
    {0x74350010, BCHP_BCM7435, BCHP_MAJOR_B, BCHP_MINOR_0},
#else
    #error "Port required"
#endif
};


/* Chip context */
typedef struct BCHP_P_7435_Context
{
    uint32_t                           ulBlackMagic;
    BREG_Handle                        hRegister;
    const BCHP_P_7435_Info            *pChipInfo;
    BCHP_P_AvsHandle                  hAvsHandle;
} BCHP_P_7435_Context;

/* Max entry of lookup table */
#define BCHP_P_CHIP_INFO_MAX_ENTRY \
    (sizeof(s_aChipInfoTable) / sizeof(BCHP_P_7435_Info))

/* This macro checks for a validity of a handle, and
 * cast to context pointer. */
#define BCHP_P_GET_CONTEXT(handle, context) \
{ \
    if(!(handle) || \
       !((handle)->chipHandle) || \
       (((BCHP_P_7435_Context*)((handle)->chipHandle))->ulBlackMagic != \
       sizeof(BCHP_P_7435_Context))) \
    { \
        BDBG_ERR(("Corrupted context handle\n")); \
        (context) = NULL; \
    } \
    else \
    { \
        (context) = (BCHP_P_7435_Context*)((handle)->chipHandle); \
    } \
    BDBG_ASSERT(context); \
}

/* Static function prototypes */
static BERR_Code BCHP_P_Close7435
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

static void BCHP_P_ResetRaagaCore
    ( const BCHP_Handle                hChip,
      const BREG_Handle                hReg );

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

/***************************************************************************
 * Open BCM7435 Chip.
 *
 */
BERR_Code BCHP_Open7435
    ( BCHP_Handle                     *phChip,
      BREG_Handle                      hRegister )
{
    BCHP_P_Context *pChip;
    BCHP_P_7435_Context *p7435Chip;
    uint32_t ulChipFamilyIdReg;
    uint32_t ulChipIdReg;
    uint32_t ulIdx;
    uint32_t             ulBondStatus;
    BERR_Code rc;

    BDBG_ENTER(BCHP_Open7435);

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

    p7435Chip = (BCHP_P_7435_Context*)
        (BKNI_Malloc(sizeof(BCHP_P_7435_Context)));
    if(!p7435Chip)
    {
        BKNI_Free(pChip);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)p7435Chip, 0x0, sizeof(BCHP_P_7435_Context));

    /* Fill up the base chip context. */
    pChip->chipHandle       = (void*)p7435Chip;
    pChip->regHandle        = hRegister;
    pChip->pCloseFunc       = BCHP_P_Close7435;
    pChip->pGetChipInfoFunc = BCHP_P_GetChipInfoComformWithBaseClass;
    pChip->pGetFeatureFunc  = BCHP_P_GetFeature;
    pChip->pMonitorPvtFunc  = BCHP_P_MonitorPvt;
    pChip->pGetAvsDataFunc  = BCHP_P_GetAvsData;
    pChip->pStandbyModeFunc = BCHP_P_StandbyMode;

    /* Fill up the chip context. */
    p7435Chip->ulBlackMagic = sizeof(BCHP_P_7435_Context);
    p7435Chip->hRegister    = hRegister;

    /* Chip Family Register id is use for indexing into the table. */
        ulChipFamilyIdReg = BREG_Read32(hRegister, BCHP_SUN_TOP_CTRL_CHIP_FAMILY_ID);
        ulChipIdReg = BREG_Read32(hRegister, BCHP_SUN_TOP_CTRL_PRODUCT_ID);
        ulChipIdReg >>= 16;

/* decompose 32 bit chip family id for use with printf format string %x%c%d
Example: 0x74350000 becomes "7435A0" */
#define PRINT_CHIP(CHIPID) \
    ((CHIPID)>>16), ((((CHIPID)&0xF0)>>4)+'A'), ((CHIPID)&0x0F)

    for(ulIdx = 0; ulIdx < BCHP_P_CHIP_INFO_MAX_ENTRY; ulIdx++)
    {
        BDBG_MSG(("Supported Chip Family and revision: %x%c%d", PRINT_CHIP(s_aChipInfoTable[ulIdx].ulChipFamilyIdReg)));
        BDBG_MSG(("Supported Chip ID: %x", s_aChipInfoTable[ulIdx].usChipId));
        BDBG_MSG(("\n"));
    }

    /* Lookup corresponding chip id. */
    for(ulIdx = 0; ulIdx < BCHP_P_CHIP_INFO_MAX_ENTRY; ulIdx++)
    {
        const BCHP_P_7435_Info *compareChipInfo = &s_aChipInfoTable[ulIdx];

        if(compareChipInfo->ulChipFamilyIdReg == ulChipFamilyIdReg)
        {
            /* Chip Information. */
            p7435Chip->pChipInfo = compareChipInfo;
            break;
        }
        else if (ulIdx == BCHP_P_CHIP_INFO_MAX_ENTRY - 1 && compareChipInfo->usMajor == (ulChipFamilyIdReg&0xF0)>>4)
        {
            /* This is a future minor revision. We will allow it with a WRN. */
            BDBG_WRN(("An unknown minor revision %x%c%d has been detected. Certain operations may result in erratic behavior. Please confirm this chip revision is supported with this software.",
                PRINT_CHIP(ulChipFamilyIdReg)));
            p7435Chip->pChipInfo = compareChipInfo;
            break;
        }
    }

    if(!p7435Chip->pChipInfo)
    {
        BKNI_Free(p7435Chip);
        BKNI_Free(pChip);
        BDBG_ERR(("*****************************************************************\n"));
        BDBG_ERR(("ERROR ERROR ERROR ERROR \n"));
        BDBG_ERR(("Unsupported Revision: %x%c%d", PRINT_CHIP(ulChipFamilyIdReg)));
        BDBG_ERR(("*****************************************************************\n"));
        phChip = NULL;
        BDBG_ASSERT(phChip);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("found %x%c%d", PRINT_CHIP(p7435Chip->pChipInfo->ulChipFamilyIdReg)));

        BCHP_P_ResetMagnumCores( pChip );

    /* Open BCHP_PWR */
    rc = BCHP_PWR_Open(&pChip->pwrManager, pChip);
    if (rc) {
        BKNI_Free(pChip);
        BKNI_Free(p7435Chip);
        return BERR_TRACE(rc);
    }

        /* Open AVS module */
        BCHP_P_AvsOpen(&p7435Chip->hAvsHandle, pChip);
        if(!p7435Chip->hAvsHandle)
        {
            BCHP_PWR_Close(pChip->pwrManager);
            BKNI_Free(pChip);
            BKNI_Free(p7435Chip);
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

#if BCHP_PWR_RESOURCE_AVD1
    BCHP_PWR_AcquireResource(pChip, BCHP_PWR_RESOURCE_AVD1);
#endif
    BREG_Write32(hRegister, BCHP_DECODE_IP_SHIM_1_SOFTSHUTDOWN_CTRL_REG, 0x0);
#if BCHP_PWR_RESOURCE_AVD1
    BCHP_PWR_ReleaseResource(pChip, BCHP_PWR_RESOURCE_AVD1);
#endif

#if BCHP_PWR_RESOURCE_VICE
    BCHP_PWR_AcquireResource(pChip, BCHP_PWR_RESOURCE_VICE);
#endif

    ulBondStatus = BREG_Read32(p7435Chip->hRegister,BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_0);

    if(BCHP_GET_FIELD_DATA(ulBondStatus,SUN_TOP_CTRL_OTP_OPTION_STATUS_0, otp_option_transcode_disable) ==  false)
    {
        BREG_Write32(hRegister, BCHP_VICE2_MISC_0_SW_INIT_SET, ~BCHP_VICE2_MISC_0_SW_INIT_SET_reserved0_MASK);
        BREG_Write32(hRegister, BCHP_VICE2_MISC_0_SW_INIT_CLR, ~BCHP_VICE2_MISC_0_SW_INIT_SET_reserved0_MASK);
        BREG_Write32(hRegister, BCHP_VICE2_MISC_1_SW_INIT_SET, ~BCHP_VICE2_MISC_1_SW_INIT_SET_reserved0_MASK);
        BREG_Write32(hRegister, BCHP_VICE2_MISC_1_SW_INIT_CLR, ~BCHP_VICE2_MISC_1_SW_INIT_SET_reserved0_MASK);
    }

#if BCHP_PWR_RESOURCE_VICE
    BCHP_PWR_ReleaseResource(pChip, BCHP_PWR_RESOURCE_VICE);
#endif

    BDBG_LEAVE(BCHP_Open7435);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_Close7435
    ( BCHP_Handle                      hChip )
{
    BCHP_P_7435_Context *p7435Chip;

    BDBG_ENTER(BCHP_P_Close7435);

    BCHP_P_GET_CONTEXT(hChip, p7435Chip);

    if(!p7435Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (p7435Chip->hAvsHandle) {
        BCHP_P_AvsClose(p7435Chip->hAvsHandle);
        p7435Chip->hAvsHandle = NULL;
    }

    BCHP_PWR_Close(hChip->pwrManager);

    /* Invalidate the magic number. */
    p7435Chip->ulBlackMagic = 0;

    BKNI_Free((void*)p7435Chip);
    BKNI_Free((void*)hChip);

    BDBG_LEAVE(BCHP_P_Close7435);
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
    const BCHP_P_7435_Context *p7435Chip;

    BDBG_ENTER(BCHP_P_GetChipInfo);

    BCHP_P_GET_CONTEXT(hChip, p7435Chip);

    if(!p7435Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(pusChipId)
    {
        *pusChipId = p7435Chip->pChipInfo->usChipId;
    }

    if(pusChipMajorRev)
    {
        *pusChipMajorRev = p7435Chip->pChipInfo->usMajor;
    }

    if(pusChipMinorRev)
    {
        *pusChipMinorRev = p7435Chip->pChipInfo->usMinor;
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
    BCHP_P_7435_Context *p7435Chip;
    uint32_t             ulBondStatus;
    uint32_t             uiReg;

    BDBG_ENTER(BCHP_P_GetFeature);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7435Chip);

    /* read bond-out status common for many features */
    ulBondStatus = BREG_Read32(p7435Chip->hRegister,
        BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_0);

    /* Read RTS settings to find the number of hardware (ViCE) video encode channels */
    uiReg = BREG_Read32(p7435Chip->hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_18);
    if (0x7fff == BCHP_GET_FIELD_DATA(uiReg, MEMC_ARB_0_CLIENT_INFO_18, BO_VAL))
    {
        uiReg = BREG_Read32(p7435Chip->hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_18);
        uiReg |= (BCHP_MASK(MEMC_ARB_0_CLIENT_INFO_18, RR_EN));
        BREG_Write32(p7435Chip->hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_18, uiReg);
        uiReg = BREG_Read32(p7435Chip->hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_19);
        uiReg |= (BCHP_MASK(MEMC_ARB_0_CLIENT_INFO_19, RR_EN));
        BREG_Write32(p7435Chip->hRegister, BCHP_MEMC_ARB_0_CLIENT_INFO_19, uiReg);
    }

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
        /* Read RTS settings to find the number of hardware (ViCE) video encode channels */
        uiReg = BREG_Read32(p7435Chip->hRegister, BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_0);
        if (BCHP_GET_FIELD_DATA(uiReg, SUN_TOP_CTRL_OTP_OPTION_STATUS_0, otp_option_transcode_disable)) {
            ((BCHP_FeatureData*)pFeatureValue)->data.disabledL2Registers.address[0] = BCHP_VICE2_L2_0_CPU_STATUS;
            ((BCHP_FeatureData*)pFeatureValue)->data.disabledL2Registers.address[1] = BCHP_VICE2_L2_1_CPU_STATUS;
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
 * {private}
 *
 */
static BERR_Code BCHP_P_ResetMagnumCores
    ( const BCHP_Handle                hChip )
{

    BREG_Handle  hRegister = hChip->regHandle;
    uint32_t ulChipIdReg = BREG_Read32(hRegister, BCHP_SUN_TOP_CTRL_PRODUCT_ID);
    uint32_t ulChipId    = ulChipIdReg >> 16;
    uint32_t masks;

    BCHP_P_ResetRaagaCore(hChip, hRegister); /* must be done first before all other cores. */
    BCHP_P_ResetV3dCore(hChip, hRegister);

    /* Reset some cores. This is needed to avoid L1 interrupts before BXXX_Open can be called per core. */
    /* Note, SW_INIT set/clear registers don't need read-modify-write. */
    masks = BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, avd0_sw_init, 1 )   |
            BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, svd0_sw_init, 1 )   |
            BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, xpt_sw_init, 1 )    |
            BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, aio_sw_init, 1 )    |
            BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, raaga0_sw_init, 1 ) |
            BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, raaga1_sw_init, 1 );
#if !BINT_DISPLAY_DISABLED
    masks |= BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, vec_sw_init, 1 ) |
             BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, bvn_sw_init, 1 ) |
             BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_SET, dvp_ht_sw_init, 1);
#endif
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_0_SET, masks);
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET,
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, sid_sw_init, 1));
    if (ulChipId == 0x7435)
    {
        BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET,
                     BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, vice20_sw_init, 1 )
                     | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_SET, vice21_sw_init, 1 ));
    }

    /* Now clear the reset. */
    masks = BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, avd0_sw_init, 1)   |
            BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, svd0_sw_init, 1)   |
            BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, xpt_sw_init, 1)    |
            BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, aio_sw_init, 1)    |
            BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, raaga0_sw_init, 1) |
            BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, raaga1_sw_init, 1);
#if !BINT_DISPLAY_DISABLED
    masks |= BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, vec_sw_init, 1) |
             BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, bvn_sw_init, 1) |
             BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_0_CLEAR, dvp_ht_sw_init, 1);
#endif
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR, masks);
    BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR,
         BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_CLEAR, sid_sw_init, 1));
    if (ulChipId == 0x7435 )
    {
        BREG_Write32(hRegister, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR,
                     BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_CLEAR, vice20_sw_init, 1 )
                     | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_INIT_1_CLEAR, vice21_sw_init, 1 ));
    }

    return BERR_SUCCESS;
}

#include "bchp_pwr_resources_priv.h"
#include "bchp_raaga_dsp_misc.h"
#include "bchp_raaga_dsp_misc_1.h"


/* SW workaround to ensure we can hit the Raaga SW_INIT safely */
static void BCHP_P_ResetRaagaCore(const BCHP_Handle hChip, const BREG_Handle hReg)
{
    uint32_t val;

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

    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_REVISION) ;

    BDBG_MSG(("REV ID VAL = 0x%x", val));

    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT) ;
    val &=  ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT,INIT_PROC_B));
    BREG_Write32(hReg,BCHP_RAAGA_DSP_MISC_SOFT_INIT, val);

    /*RDB says no need of Read modify write.*/
    val = 0;
    val = (BCHP_FIELD_DATA(SUN_TOP_CTRL_SW_INIT_0_SET,   raaga0_sw_init,1));
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_SW_INIT_0_SET, val);

    BKNI_Delay(2);

    /*RDB says no need of Read modify write.*/
    val = 0;
    val = (BCHP_FIELD_DATA(SUN_TOP_CTRL_SW_INIT_0_CLEAR, raaga0_sw_init,1));
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR, val);


    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_1_SOFT_INIT) ;
    val = (val & ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT)))|
     (BCHP_FIELD_DATA(RAAGA_DSP_MISC_SOFT_INIT, DO_SW_INIT,1));
    BREG_Write32(hReg,BCHP_RAAGA_DSP_MISC_1_SOFT_INIT, val);

    val = BREG_Read32(hReg, BCHP_RAAGA_DSP_MISC_1_REVISION);

    BDBG_MSG(("REV ID VAL = 0x%x", val));

    val = BREG_Read32(hReg,BCHP_RAAGA_DSP_MISC_1_SOFT_INIT) ;
    val &=  ~(BCHP_MASK(RAAGA_DSP_MISC_SOFT_INIT,INIT_PROC_B));
    BREG_Write32(hReg,BCHP_RAAGA_DSP_MISC_1_SOFT_INIT, val);

    /*RDB says no need of Read modify write.*/
    val = 0;
    val = (BCHP_FIELD_DATA(SUN_TOP_CTRL_SW_INIT_0_SET,   raaga1_sw_init,1));
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_SW_INIT_0_SET, val);

    BKNI_Delay(2);

    /*RDB says no need of Read modify write.*/
    val = 0;
    val = (BCHP_FIELD_DATA(SUN_TOP_CTRL_SW_INIT_0_CLEAR, raaga1_sw_init,1));
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR, val);

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
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_MOCA_CH3, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D, true);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D_SRAM, true);

    BREG_Write32(hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, V3D_CLK_108_SW_INIT, ASSERT));
    BREG_Write32(hReg, BCHP_V3D_TOP_GR_BRIDGE_SW_INIT_0, BCHP_FIELD_ENUM(V3D_TOP_GR_BRIDGE_SW_INIT_0, V3D_CLK_108_SW_INIT, DEASSERT));

    BREG_Write32(hReg, BCHP_V3D_CTL_INTCTL, ~0);
    BREG_Write32(hReg, BCHP_V3D_DBG_DBQITC, ~0);

    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D, false);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_V3D_SRAM, false);
    BCHP_PWR_P_HW_ControlId(hChip, BCHP_PWR_HW_PLL_MOCA_CH3, false);
}

/* This gets called regularly to handle the AVS processing */
static void BCHP_P_MonitorPvt( BCHP_Handle hChip, BCHP_AvsSettings *pSettings )
{
    BCHP_P_7435_Context *p7435Chip;

    BDBG_ENTER(BCHP_P_MonitorPvt);
    BSTD_UNUSED(pSettings);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7435Chip);

    if (p7435Chip->hAvsHandle)
        BCHP_P_AvsMonitorPvt(p7435Chip->hAvsHandle);

    BDBG_LEAVE(BCHP_P_MonitorPvt);
}

static BERR_Code BCHP_P_GetAvsData( BCHP_Handle hChip, BCHP_AvsData *pData )
{
    BCHP_P_7435_Context *p7435Chip;

    BDBG_ASSERT(pData);

    BDBG_ENTER(BCHP_GetAVdata);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7435Chip);

    if (p7435Chip->hAvsHandle)
        BCHP_P_AvsGetData(p7435Chip->hAvsHandle, pData);

    BDBG_LEAVE(BCHP_GetAVdata);
    return BERR_SUCCESS;
}

static BERR_Code BCHP_P_StandbyMode( BCHP_Handle hChip, bool activate )
{
    BCHP_P_7435_Context *p7435Chip;

    BDBG_ENTER(BCHP_P_StandbyMode);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7435Chip);

    /* Do anything required for CHP Standby changes */

    if (p7435Chip->hAvsHandle)
        BCHP_P_AvsStandbyMode(p7435Chip->hAvsHandle, activate);

    BDBG_LEAVE(BCHP_P_StandbyMode);
    return BERR_SUCCESS;
}

/* End of File */
