/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 *   See Module Overview below.
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
#include "bchp_7400.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_clk.h"

BDBG_MODULE(BCHP);

/* Miscellaneous macros. */
#define BCHP_P_MAJOR_REV_SHIFT          (4)

/* Chip info and features */
typedef struct BCHP_P_7400_Info
{
    uint32_t      ulChipIdReg; /* index into the table. */

    /* Chip Id */
    uint16_t      usChipId;

    /* Major revision */
    uint16_t      usMajor;

    /* Minor revision */
    uint16_t      usMinor;

    /* TODO: Other features or infos if needed */
} BCHP_P_7400_Info;


/* Lookup table for chip features and etc.
 * The are many times when the chip device id register
 * not conforming to the standard numbering convention. We do
 * it this way to work-around those problems.
 *
 * TODO: Update this table to support new revisions.
 */
static const BCHP_P_7400_Info s_aChipInfoTable[] =
{
#if BCHP_VER == BCHP_VER_A0
    /* A0 code will run on A0 */
    {0x74000000, BCHP_BCM7400, BCHP_MAJOR_A, BCHP_MINOR_0},
#elif BCHP_VER == BCHP_VER_B0
    /* B0 code will run on B0 and C0 */
    {0x74000010, BCHP_BCM7400, BCHP_MAJOR_B, BCHP_MINOR_0},
    {0x74000020, BCHP_BCM7400, BCHP_MAJOR_C, BCHP_MINOR_0},
#elif BCHP_VER == BCHP_VER_C0
    /* C0 code will run on C0 */
    {0x74000020, BCHP_BCM7400, BCHP_MAJOR_C, BCHP_MINOR_0},
#elif BCHP_VER == BCHP_VER_D0 || BCHP_VER == BCHP_VER_D1 || BCHP_VER == BCHP_VER_D2
    /* DX code will run on DX, E0.  Build for other Dx using D0. */
    {0x74000030, BCHP_BCM7400, BCHP_MAJOR_D, BCHP_MINOR_0},
    {0x74000031, BCHP_BCM7400, BCHP_MAJOR_D, BCHP_MINOR_1},
    {0x74000032, BCHP_BCM7400, BCHP_MAJOR_D, BCHP_MINOR_2},
    {0x74000033, BCHP_BCM7400, BCHP_MAJOR_D, BCHP_MINOR_3},
    {0x74000040, BCHP_BCM7400, BCHP_MAJOR_E, BCHP_MINOR_0},
#elif BCHP_VER == BCHP_VER_E0
    /* E0 code will run on E0 */
    {0x74000040, BCHP_BCM7400, BCHP_MAJOR_E, BCHP_MINOR_0},
#else
    #error "Port required"
#endif
};


/* Chip context */
typedef struct BCHP_P_7400_Context
{
    uint32_t                           ulBlackMagic;
    BREG_Handle                        hRegister;
    const BCHP_P_7400_Info            *pChipInfo;
} BCHP_P_7400_Context;

/* Max entry of lookup table */
#define BCHP_P_CHIP_INFO_MAX_ENTRY \
    (sizeof(s_aChipInfoTable) / sizeof(BCHP_P_7400_Info))

/* This macro checks for a validity of a handle, and
 * cast to context pointer. */
#define BCHP_P_GET_CONTEXT(handle, context) \
{ \
    if(!(handle) || \
       !((handle)->chipHandle) || \
       (((BCHP_P_7400_Context*)((handle)->chipHandle))->ulBlackMagic != \
       sizeof(BCHP_P_7400_Context))) \
    { \
        BDBG_ERR(("Corrupted context handle\n")); \
        (context) = NULL; \
    } \
    else \
    { \
        (context) = (BCHP_P_7400_Context*)((handle)->chipHandle); \
    } \
    BDBG_ASSERT(context); \
}

/* Static function prototypes */
static BERR_Code BCHP_P_Close7400
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

/***************************************************************************
 * Open BCM7400 Chip.
 *
 */
BERR_Code BCHP_Open7400
    ( BCHP_Handle                     *phChip,
      BREG_Handle                      hRegister )
{
    BCHP_P_Context *pChip;
    BCHP_P_7400_Context *p7400Chip;
    uint32_t ulChipIdReg;
    uint32_t ulIdx;
    uint32_t ulRegData;

    BDBG_ENTER(BCHP_Open7400);

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

    p7400Chip = (BCHP_P_7400_Context*)
        (BKNI_Malloc(sizeof(BCHP_P_7400_Context)));
    if(!p7400Chip)
    {
        BKNI_Free(pChip);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)p7400Chip, 0x0, sizeof(BCHP_P_7400_Context));

    /* Fill up the base chip context. */
    pChip->chipHandle       = (void*)p7400Chip;
    pChip->regHandle        = hRegister;
    pChip->pCloseFunc       = BCHP_P_Close7400;
    pChip->pGetChipInfoFunc = BCHP_P_GetChipInfoComformWithBaseClass;
    pChip->pGetFeatureFunc  = BCHP_P_GetFeature;

    /* Fill up the chip context. */
    p7400Chip->ulBlackMagic = sizeof(BCHP_P_7400_Context);
    p7400Chip->hRegister    = hRegister;

    /* Register id is use for indexing into the table. */
    ulChipIdReg = BREG_Read32(hRegister, BCHP_SUN_TOP_CTRL_PROD_REVISION);

/* decompose 32 bit chip id for use with printf format string %x%c%d
Example: 0x74010020 becomes "7401C0" */
#define PRINT_CHIP(CHIPID) \
    ((CHIPID)>>16), ((((CHIPID)&0xF0)>>4)+'A'), ((CHIPID)&0x0F)

    for(ulIdx = 0; ulIdx < BCHP_P_CHIP_INFO_MAX_ENTRY; ulIdx++)
    {
        BDBG_MSG(("supported revision: %x%c%d", PRINT_CHIP(s_aChipInfoTable[ulIdx].ulChipIdReg)));
    }

    /* Lookup corresponding chip id. */
    for(ulIdx = 0; ulIdx < BCHP_P_CHIP_INFO_MAX_ENTRY; ulIdx++)
    {
        const BCHP_P_7400_Info *compareChipInfo = &s_aChipInfoTable[ulIdx];

        if(compareChipInfo->ulChipIdReg == ulChipIdReg)
        {
            /* Chip Information. */
            p7400Chip->pChipInfo = compareChipInfo;
            break;
        }
        else if (ulIdx == BCHP_P_CHIP_INFO_MAX_ENTRY - 1 && compareChipInfo->usMajor == (ulChipIdReg&0xF0)>>4)
        {
            /* This is a future minor revision. We will allow it with a WRN. */
            BDBG_WRN(("An unknown minor revision %x%c%d has been detected. Certain operations may result in erratic behavior. Please confirm this chip revision is supported with this software.",
                PRINT_CHIP(ulChipIdReg)));
            p7400Chip->pChipInfo = compareChipInfo;
            break;
        }
    }

    if(!p7400Chip->pChipInfo)
    {
        BKNI_Free(p7400Chip);
        BKNI_Free(pChip);
        BDBG_ERR(("unsupported revision: %x%c%d", PRINT_CHIP(ulChipIdReg)));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("found %x%c%d", PRINT_CHIP(p7400Chip->pChipInfo->ulChipIdReg)));

    /* All done. now return the new fresh context to user. */
    *phChip = (BCHP_Handle)pChip;

    #if ( BCHP_VER > BCHP_VER_A0 )
    {
    const uint32_t mask =
          BCHP_MASK( SUN_TOP_CTRL_SW_RESET, xpt_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, avd0_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, avd1_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, vec_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, bvnm_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, aio_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, rptd_sw_reset);

    BREG_AtomicUpdate32(hRegister, BCHP_SUN_TOP_CTRL_SW_RESET, mask,
              BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, xpt_sw_reset, 1 )
            | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, avd0_sw_reset, 1 )
            | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, avd1_sw_reset, 1 )
            | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, vec_sw_reset, 1 )
            | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, bvnm_sw_reset, 1 )
            | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, aio_sw_reset, 1 )
            | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, rptd_sw_reset, 1 ));

    /* Now clear the reset. */
    BREG_AtomicUpdate32(hRegister, BCHP_SUN_TOP_CTRL_SW_RESET, mask, 0);
    }
    #endif

    /* Make sure the HDMI CLOCK_xxx_MAX is disabled.  See SW7125-651 for more information. */
    BREG_AtomicUpdate32(hRegister, BCHP_CLK_PM_CTRL,BCHP_CLK_PM_CTRL_DIS_HDMI_108M_CLK_MASK,
                        BCHP_CLK_PM_CTRL_DIS_HDMI_108M_CLK_MASK);
    /* Power down AVD clocks */
    ulRegData = BCHP_CLK_PM_CTRL_1_DIS_AVD1_200_CLK_MASK |
                BCHP_CLK_PM_CTRL_1_DIS_AVD0_200_CLK_MASK |
                BCHP_CLK_PM_CTRL_1_DIS_MEMC_16_0_200_CLK_MASK |
                BCHP_CLK_PM_CTRL_1_DIS_MEMC_16_1_200_CLK_MASK;
    BREG_AtomicUpdate32( hRegister, BCHP_CLK_PM_CTRL_1,ulRegData,ulRegData);
    ulRegData =  BCHP_CLK_PM_CTRL_DIS_AVD1_108M_CLK_MASK |
                  BCHP_CLK_PM_CTRL_DIS_AVD0_108M_CLK_MASK;
    BREG_AtomicUpdate32( hRegister, BCHP_CLK_PM_CTRL,ulRegData,ulRegData);

    /* Power down audio core clocks */
    ulRegData = BCHP_CLK_PM_CTRL_DIS_AIO_108M_CLK_MASK |
                 BCHP_CLK_PM_CTRL_DIS_RPTD_108M_CLK_MASK;
    BREG_AtomicUpdate32( hRegister, BCHP_CLK_PM_CTRL,ulRegData,ulRegData);

    ulRegData = BCHP_CLK_PM_CTRL_2_DIS_AUD_DSP_PROG_CLK_MASK;
    BREG_AtomicUpdate32( hRegister, BCHP_CLK_PM_CTRL_2,ulRegData,ulRegData);

     /* Power up AVD clocks */
    ulRegData = BCHP_CLK_PM_CTRL_1_DIS_AVD1_200_CLK_MASK |
                BCHP_CLK_PM_CTRL_1_DIS_AVD0_200_CLK_MASK |
                BCHP_CLK_PM_CTRL_1_DIS_MEMC_16_0_200_CLK_MASK |
                BCHP_CLK_PM_CTRL_1_DIS_MEMC_16_1_200_CLK_MASK;
    BREG_AtomicUpdate32( hRegister, BCHP_CLK_PM_CTRL_1,ulRegData,0);
    ulRegData =  BCHP_CLK_PM_CTRL_DIS_AVD1_108M_CLK_MASK |
                 BCHP_CLK_PM_CTRL_DIS_AVD0_108M_CLK_MASK;
    BREG_AtomicUpdate32(hRegister, BCHP_CLK_PM_CTRL, ulRegData, 0);


    /* Power up HDMI clock */
    BREG_AtomicUpdate32(hRegister, BCHP_CLK_PM_CTRL,BCHP_CLK_PM_CTRL_DIS_HDMI_108M_CLK_MASK,0);

    /* Power up audio clocks */
    ulRegData = BCHP_CLK_PM_CTRL_2_DIS_AUD_DSP_PROG_CLK_MASK;
    BREG_AtomicUpdate32(hRegister, BCHP_CLK_PM_CTRL_2, ulRegData, 0);

    ulRegData = BCHP_CLK_PM_CTRL_DIS_AIO_108M_CLK_MASK |
                 BCHP_CLK_PM_CTRL_DIS_RPTD_108M_CLK_MASK;
    BREG_AtomicUpdate32(hRegister, BCHP_CLK_PM_CTRL, ulRegData, 0);

    BDBG_LEAVE(BCHP_Open7400);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_Close7400
    ( BCHP_Handle                      hChip )
{
    BCHP_P_7400_Context *p7400Chip;

    BDBG_ENTER(BCHP_P_Close7400);

    BCHP_P_GET_CONTEXT(hChip, p7400Chip);

    if(!p7400Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Invalidate the magic number. */
    p7400Chip->ulBlackMagic = 0;

    BKNI_Free((void*)p7400Chip);
    BKNI_Free((void*)hChip);

    BDBG_LEAVE(BCHP_P_Close7400);
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
    uint16_t usMajor = 0;
    uint16_t usMinor = 0;

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
    const BCHP_P_7400_Context *p7400Chip;

    BDBG_ENTER(BCHP_P_GetChipInfo);

    BCHP_P_GET_CONTEXT(hChip, p7400Chip);

    if(!p7400Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(pusChipId)
    {
        *pusChipId = p7400Chip->pChipInfo->usChipId;
    }

    if(pusChipMajorRev)
    {
        *pusChipMajorRev = p7400Chip->pChipInfo->usMajor;
    }

    if(pusChipMinorRev)
    {
        *pusChipMinorRev = p7400Chip->pChipInfo->usMinor;
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
    BCHP_P_7400_Context *p7400Chip;
    uint32_t             ulBondStatus;

    BDBG_ENTER(BCHP_P_GetFeature);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7400Chip);

    /* read bond-out status common for many features */
    ulBondStatus = BREG_Read32(p7400Chip->hRegister,
        BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS);

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
        *(bool *)pFeatureValue = true;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_eMacrovisionCapable:
        /* macrovision capable? (bool) */
        *(bool *)pFeatureValue = BCHP_GET_FIELD_DATA(ulBondStatus,
            SUN_TOP_CTRL_OTP_OPTION_STATUS, otp_option_macrovision_enable ) ? true : false;
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
            SUN_TOP_CTRL_OTP_OPTION_STATUS, otp_option_hdcp_disable ) ? false : true;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_e3desCapable:
        /* 3DES capable? (bool) */
        *(bool *)pFeatureValue = true;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_e1080pCapable:
        /* 1080p Capable? (bool) */
        *(bool *)pFeatureValue = false;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_eMemCtrl1Capable:
    case BCHP_Feature_eMemCtrl2Capable:
        *(bool *)pFeatureValue = true;
        rc = BERR_SUCCESS;
        break;
     case BCHP_Feature_eMemCtrl0DramWidthCount:
    {
       uint32_t ulReg, memc_config;
       /* DRAM Width: 0 (32 bit), 1 (16 bit) */
       ulReg = BREG_Read32(p7400Chip->hRegister, BCHP_SUN_TOP_CTRL_STRAP_VALUE_0);
       memc_config = BCHP_GET_FIELD_DATA(ulReg, SUN_TOP_CTRL_STRAP_VALUE_0, strap_ddr_configuration);

       if (memc_config == 0)
       {
          *(int *)pFeatureValue = 32;
       }
       else
       {
          *(int *)pFeatureValue = 16;
       }

       rc = BERR_SUCCESS;
       break;
    }

    case BCHP_Feature_eDisabledL2Registers:
        /* by default, none are disabled */
        BKNI_Memset(pFeatureValue, 0, sizeof(BCHP_FeatureData));
        rc = BERR_SUCCESS;
        break;

    default:
        rc = BERR_TRACE(BERR_UNKNOWN);
    }

    /* return result */
    BDBG_LEAVE(BCHP_P_GetFeature);
    return rc;
}

/* End of File */
