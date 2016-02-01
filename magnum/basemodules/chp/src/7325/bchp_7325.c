/*************************************************************************** *     Copyright (c) 2006-2012, Broadcom Corporation
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
#include "bchp_7325.h"
#include "bchp_clkgen.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_decode_sd_0.h"
#include "bchp_decode_ip_shim_0.h"
#include "bchp_memc_0_ddr.h"

BDBG_MODULE(BCHP);

/* Miscellaneous macros. */
#define BCHP_P_MAJOR_REV_SHIFT          (4)

/* Chip info and features */
typedef struct BCHP_P_7325_Info
{
    uint32_t      ulChipIdReg; /* index into the table. */

    /* Chip Id */
    uint16_t      usChipId;

    /* Major revision */
    uint16_t      usMajor;

    /* Minor revision */
    uint16_t      usMinor;

    /* TODO: Other features or infos if needed */
} BCHP_P_7325_Info;


/* Lookup table for chip features and etc.
 * The are many times when the chip device id register
 * not conforming to the standard numbering convention. We do 
 * it this way to work-around those problems. 
 * 
 * TODO: Update this table to support new revisions.
 */
static const BCHP_P_7325_Info s_aChipInfoTable[] =
{
#if BCHP_VER == BCHP_VER_B0
    /* B0  */
    {0x73250010, BCHP_BCM7325, BCHP_MAJOR_B, BCHP_MINOR_0},
    {0x73240010, BCHP_BCM7325, BCHP_MAJOR_B, BCHP_MINOR_0},
#else
    #error "Port required"
#endif
};


/* Chip context */
typedef struct BCHP_P_7325_Context
{
    uint32_t                           ulBlackMagic;
    BREG_Handle                        hRegister;
    const BCHP_P_7325_Info            *pChipInfo;
} BCHP_P_7325_Context;

/* Max entry of lookup table */
#define BCHP_P_CHIP_INFO_MAX_ENTRY \
    (sizeof(s_aChipInfoTable) / sizeof(BCHP_P_7325_Info))

/* This macro checks for a validity of a handle, and
 * cast to context pointer. */
#define BCHP_P_GET_CONTEXT(handle, context) \
{ \
    if(!(handle) || \
       !((handle)->chipHandle) || \
       (((BCHP_P_7325_Context*)((handle)->chipHandle))->ulBlackMagic != \
       sizeof(BCHP_P_7325_Context))) \
    { \
        BDBG_ERR(("Corrupted context handle\n")); \
        (context) = NULL; \
    } \
    else \
    { \
        (context) = (BCHP_P_7325_Context*)((handle)->chipHandle); \
    } \
    BDBG_ASSERT(context); \
}

/* Static function prototypes */
static BERR_Code BCHP_P_Close7325
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
 * Open BCM7325 Chip.
 *
 */
BERR_Code BCHP_Open7325
    ( BCHP_Handle                     *phChip,
      BREG_Handle                      hRegister )
{
    BCHP_P_Context *pChip;
    BCHP_P_7325_Context *p7325Chip;
    uint32_t ulChipIdReg;
    uint32_t ulIdx;
    uint32_t ulReg,ulMemc;
    uint32_t bankHeight=0, stripeWidth=0;
    uint32_t ddr0Device=0, ddr1Device =0;
    
    BDBG_ENTER(BCHP_Open7325);

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

    p7325Chip = (BCHP_P_7325_Context*)
        (BKNI_Malloc(sizeof(BCHP_P_7325_Context)));
    if(!p7325Chip)
    {
        BKNI_Free(pChip);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)p7325Chip, 0x0, sizeof(BCHP_P_7325_Context));

    /* Fill up the base chip context. */
    pChip->chipHandle       = (void*)p7325Chip;
    pChip->regHandle        = hRegister;
    pChip->pCloseFunc       = BCHP_P_Close7325;
    pChip->pGetChipInfoFunc = BCHP_P_GetChipInfoComformWithBaseClass;
    pChip->pGetFeatureFunc  = BCHP_P_GetFeature;

    /* Fill up the chip context. */
    p7325Chip->ulBlackMagic = sizeof(BCHP_P_7325_Context);
    p7325Chip->hRegister    = hRegister;

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
        const BCHP_P_7325_Info *compareChipInfo = &s_aChipInfoTable[ulIdx];

        if(s_aChipInfoTable[ulIdx].ulChipIdReg == ulChipIdReg)
        {
            /* Chip Information. */
            p7325Chip->pChipInfo = &(s_aChipInfoTable[ulIdx]);
            break;
        }
        else if (ulIdx == BCHP_P_CHIP_INFO_MAX_ENTRY - 1 && compareChipInfo->usMajor == (ulChipIdReg&0xF0)>>4)
        {
            /* This is a future minor revision. We will allow it with a WRN. */
            BDBG_WRN(("An unknown minor revision %x%c%d has been detected. Certain operations may result in erratic behavior. Please confirm this chip revision is supported with this software.",
                PRINT_CHIP(ulChipIdReg)));
            p7325Chip->pChipInfo = compareChipInfo;
            break;
        }
    }

    if(!p7325Chip->pChipInfo)
    {
        BKNI_Free(p7325Chip);
        BKNI_Free(pChip);
        BDBG_ERR(("unsupported revision: %x%c%d", PRINT_CHIP(ulChipIdReg)));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("found %x%c%d", PRINT_CHIP(p7325Chip->pChipInfo->ulChipIdReg)));

    /* All done. now return the new fresh context to user. */
    *phChip = (BCHP_Handle)pChip;

/* BCHP_NO_INIT allows CHP to be used in a powered down system where many registers should not be touched. */
#ifndef BCHP_NO_INIT
     /* Store values of registers */

    /* Reset some cores. This is needed to avoid L1 interrupts before BXXX_Open can be called per core. */
    ulReg = BREG_Read32(hRegister,BCHP_SUN_TOP_CTRL_STRAP_VALUE_0);
    {
    uint32_t mask =
          BCHP_MASK( SUN_TOP_CTRL_SW_RESET, xpt_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, avd0_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, vec_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, bvnm_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, aio_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, rptd_sw_reset);

    BREG_AtomicUpdate32(hRegister, BCHP_SUN_TOP_CTRL_SW_RESET, mask,
	 BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, xpt_sw_reset, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, avd0_sw_reset, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, vec_sw_reset, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, bvnm_sw_reset, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, aio_sw_reset, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, rptd_sw_reset, 1 ));

    /* Now clear the reset. */
    BREG_AtomicUpdate32(hRegister, BCHP_SUN_TOP_CTRL_SW_RESET, mask, 0);
    }
    

    /* read the strap registers and program system stripe width and pfri bank height */
    
    ulMemc = BCHP_GET_FIELD_DATA(ulReg,SUN_TOP_CTRL_STRAP_VALUE_0,strap_ddr_configuration);
    ddr0Device = BCHP_GET_FIELD_DATA(ulReg, SUN_TOP_CTRL_STRAP_VALUE_0, strap_ddr0_device_config);
    ddr1Device = BCHP_GET_FIELD_DATA(ulReg, SUN_TOP_CTRL_STRAP_VALUE_0, strap_ddr1_device_config);

    BDBG_WRN(("Strap ddr config =%x, ddr0 Device Tech %x, ddr1 Device Tech %x",ulMemc, ddr0Device, ddr1Device));

    /* Table to Determine Stripe Width and Bank Height based on Device Tech and yyy*/
    switch( ulMemc) 
    {
    case 0: /* 32 bit UMA */
        {
            BDBG_WRN(("Strapped for 32 bit UMA"));
            switch(ddr0Device)
            {
                case 0: /* 256 Mbits Device Tech*/
                    stripeWidth =0; /* 64 bytes */
                    bankHeight = 1; /* 2 Mblks */
                    break;
                case 1: /* 512 Mbits Device Tech*/
                    stripeWidth =1; /* 128 bytes */
                    bankHeight = 1; /* 2 Mblks */
                    break;
                case 2: /* 1024 Mbits Device Tech*/
                    stripeWidth =1; /* 128 bytes */
                    bankHeight = 1; /* 2 Mblks */
                    break;
                case 3: /* 2048 Mbits Device Tech*/
                    stripeWidth =1; /* 128 bytes */
                    bankHeight = 1; /* 2 Mblks */
                    break;
            default:
                    BDBG_ERR(("Unknown Value in DDR Device Config Register"));
                    break;
            }

        }
        break;
    case 1: /* 16 bit UMA */
        {
            BDBG_WRN(("Strapped for 16 bit UMA"));
             switch(ddr0Device)
             {
                 case 0: /* 256 Mbits Device Tech*/
                     stripeWidth =0; /* 64 bytes */
                     bankHeight = 0; /* 1 Mblks */
                     break;
                 case 1: /* 512 Mbits Device Tech*/
                     stripeWidth =0; /* 64 bytes */
                     bankHeight = 1; /* 2 Mblks */
                     break;
                 case 2: /* 1024 Mbits Device Tech*/
                     stripeWidth =0; /* 64 bytes */
                     bankHeight = 1; /* 2 Mblks */
                     break;
                 case 3: /* 2048 Mbits Device Tech*/
                     stripeWidth =0; /* 64 bytes */
                     bankHeight = 1; /* 2 Mblks */
                     break;
                default:
                    BDBG_ERR(("Unknown Value in DDR Device Config Register"));
                    break;
               }
        }
        break;
    case 2: /* 16/16 non UMA mode */
            BDBG_WRN(("Strapped for 16/16 bit nonUMA"));
            switch(ddr1Device)
             {
                 case 0: /* 256 Mbits Device Tech*/
                     stripeWidth =0; /* 64 bytes */
                     bankHeight = 0; /* 1 Mblks */
                     break;
                 case 1: /* 512 Mbits Device Tech*/
                     stripeWidth =0; /* 64 bytes */
                     bankHeight = 1; /* 2 Mblks */
                     break;
                 case 2: /* 1024 Mbits Device Tech*/ 
                     stripeWidth =0; /* 64 bytes */
                     bankHeight = 1; /* 2 Mblks */
                     break;
                 case 3: /* 2048 Mbits Device Tech*/
                     stripeWidth =0; /* 64 bytes */
                     bankHeight = 1; /* 2 Mblks */
                     break;
                default:
                    BDBG_ERR(("Unknown Value in DDR Device Config Register"));
                    break;
            }
        break;    
    default:
        BDBG_ERR(("Invalid DDR Strap =%x",ulMemc));
    }


    BREG_AtomicUpdate32(hRegister, 
            BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH, 
            BCHP_MASK( DECODE_SD_0_REG_SD_STRIPE_WIDTH, Strip_Width),
            BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_STRIPE_WIDTH, Strip_Width, stripeWidth )
    );

    BREG_AtomicUpdate32(hRegister, 
        BCHP_DECODE_IP_SHIM_0_PFRI_REG,
        BCHP_MASK( DECODE_IP_SHIM_0_PFRI_REG, pfri_bank_height),
        BCHP_FIELD_DATA( DECODE_IP_SHIM_0_PFRI_REG, pfri_bank_height, bankHeight )
    );

     /*
     *  Refer to SWCFE-294 for explanation on why the below 
     *  power down and power up is required.
     */

    BDBG_MSG((" Power Down "));
     /*  Power Down AVD */
    ulReg = BCHP_CLKGEN_PWRDN_CTRL_0_PWRDN_CLOCK_216_CG_AVD_MASK;
    BREG_AtomicUpdate32(hRegister, BCHP_CLKGEN_PWRDN_CTRL_0, ulReg,ulReg);
    
    ulReg = BCHP_CLKGEN_PWRDN_CTRL_1_PWRDN_CLOCK_108_CG_AVD_MASK;
    BREG_AtomicUpdate32(hRegister, BCHP_CLKGEN_PWRDN_CTRL_1, ulReg,ulReg);

    /* Power Down RAP */
    ulReg = BCHP_CLKGEN_PWRDN_CTRL_0_PWRDN_CLOCK_216_CG_RPTD_MASK |
                    BCHP_CLKGEN_PWRDN_CTRL_0_PWRDN_CLOCK_216_CG_AIO_MASK;
    BREG_AtomicUpdate32(hRegister, BCHP_CLKGEN_PWRDN_CTRL_0, ulReg,ulReg);

    ulReg = BCHP_CLKGEN_PWRDN_CTRL_1_PWRDN_CLOCK_108_CG_RPTD_MASK |
        BCHP_CLKGEN_PWRDN_CTRL_1_PWRDN_CLOCK_108_CG_AIO_MASK;
    BREG_AtomicUpdate32(hRegister, BCHP_CLKGEN_PWRDN_CTRL_1, ulReg,ulReg);

    BDBG_MSG((" Power Up "));
    /* Power Up AVD */
    ulReg = BCHP_CLKGEN_PWRDN_CTRL_0_PWRDN_CLOCK_216_CG_AVD_MASK;
    BREG_AtomicUpdate32(hRegister, BCHP_CLKGEN_PWRDN_CTRL_0, ulReg,0);

    ulReg = (BCHP_CLKGEN_PWRDN_CTRL_1_PWRDN_CLOCK_108_CG_AVD_MASK);
    BREG_AtomicUpdate32(hRegister, BCHP_CLKGEN_PWRDN_CTRL_1, ulReg,0);

    /* Power Up RAP */
    ulReg = (BCHP_CLKGEN_PWRDN_CTRL_0_PWRDN_CLOCK_216_CG_RPTD_MASK |
                 BCHP_CLKGEN_PWRDN_CTRL_0_PWRDN_CLOCK_216_CG_AIO_MASK);
    BREG_AtomicUpdate32(hRegister, BCHP_CLKGEN_PWRDN_CTRL_0, ulReg,0);

    ulReg = (BCHP_CLKGEN_PWRDN_CTRL_1_PWRDN_CLOCK_108_CG_RPTD_MASK |
                 BCHP_CLKGEN_PWRDN_CTRL_1_PWRDN_CLOCK_108_CG_AIO_MASK);
    BREG_AtomicUpdate32(hRegister, BCHP_CLKGEN_PWRDN_CTRL_1, ulReg,0);

    BDBG_MSG((" Power UP COMPLETED"));    
#endif /* BCHP_NO_INIT */

    BDBG_LEAVE(BCHP_Open7325);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_Close7325
    ( BCHP_Handle                      hChip )
{
    BCHP_P_7325_Context *p7325Chip;

    BDBG_ENTER(BCHP_P_Close7325);
    
    BCHP_P_GET_CONTEXT(hChip, p7325Chip);

    if(!p7325Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Invalidate the magic number. */
    p7325Chip->ulBlackMagic = 0;

    BKNI_Free((void*)p7325Chip);
    BKNI_Free((void*)hChip);

    BDBG_LEAVE(BCHP_P_Close7325);
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
    const BCHP_P_7325_Context *p7325Chip;

    BDBG_ENTER(BCHP_P_GetChipInfo);

    BCHP_P_GET_CONTEXT(hChip, p7325Chip);

    if(!p7325Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(pusChipId)
    {
        *pusChipId = p7325Chip->pChipInfo->usChipId;
    }

    if(pusChipMajorRev)
    {
        *pusChipMajorRev = p7325Chip->pChipInfo->usMajor;
    }

    if(pusChipMinorRev)
    {
        *pusChipMinorRev = p7325Chip->pChipInfo->usMinor;
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
    BCHP_P_7325_Context *p7325Chip;
    uint32_t             ulBondStatus;

    BDBG_ENTER(BCHP_P_GetFeature);
    
    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7325Chip);

    /* read bond-out status common for many features */
    ulBondStatus = BREG_Read32(p7325Chip->hRegister,
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
        *(int *)pFeatureValue = 1;
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
    {
        uint32_t ulStrapVal0, ulDdrConfig;

        /* Get the strap value */
        ulStrapVal0 = BREG_Read32(p7325Chip->hRegister, BCHP_SUN_TOP_CTRL_STRAP_VALUE_0);

        ulDdrConfig = BCHP_GET_FIELD_DATA(ulStrapVal0,
            SUN_TOP_CTRL_STRAP_VALUE_0, strap_ddr_configuration);

        if((ulDdrConfig == 4) || (ulDdrConfig == 5))
        {
            *(bool *)pFeatureValue = true;
        }
        else
        {
            *(bool *)pFeatureValue = false;
        }
        rc = BERR_SUCCESS;
        break ;
    }
        
	case BCHP_Feature_eMemCtrl0DDRDeviceTechCount:
	{
	 	uint32_t ulReg, ddrDevice;
	   /* Size of memory part in MBits ie: 256, 512, 1024 */
	   /* Device Tech: 0 (256Mbits), 1 (512MBbits), 2 (1Gbit), 3 (2Gbit), 4 (4Gbit) */
	   ulReg = BREG_Read32(p7325Chip->hRegister, BCHP_MEMC_0_DDR_CNTRLR_CONFIG);
	   ddrDevice = BCHP_GET_FIELD_DATA(ulReg, MEMC_0_DDR_CNTRLR_CONFIG, DEVICE_TECH);

	   switch(ddrDevice)
	   {
	      case 0:

	         *(int *)pFeatureValue = 256;
	         rc = BERR_SUCCESS;
	         break;

	      case 1:

	         *(int *)pFeatureValue = 512;
	         rc = BERR_SUCCESS;
	         break;

	      case 2:

	         *(int *)pFeatureValue = 1024;
	         rc = BERR_SUCCESS;
	         break;

	      case 3:

	         *(int *)pFeatureValue = 2048;
	         rc = BERR_SUCCESS;
	         break;

	      case 4:

	         *(int *)pFeatureValue = 4096;
	         rc = BERR_SUCCESS;
	         break;
	   }
	
		break;
	}

    case BCHP_Feature_eMemCtrl0DramWidthCount:
    {
       uint32_t ulReg, memc_config;
       /* DRAM Width: 0 (32 bit), 1 (16 bit) */
       ulReg = BREG_Read32(p7325Chip->hRegister, BCHP_SUN_TOP_CTRL_STRAP_VALUE_0);
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
    default:
        rc = BERR_TRACE(BERR_UNKNOWN);
    }

    /* return result */
    BDBG_LEAVE(BCHP_P_GetFeature);
    return rc;
}

/* End of File */
