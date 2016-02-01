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
#include "bchp_7340.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_decode_sd_0.h"
#include "bchp_decode_ip_shim_0.h"
#include "bchp_sun_gisb_arb.h"
#include "bchp_memc_ddr23_aphy_ac_0.h"
#include "bchp_memc_ddr_0.h"

BDBG_MODULE(BCHP);

/* Miscellaneous macros. */
#define BCHP_P_MAJOR_REV_SHIFT          (4)

/* Chip info and features */
typedef struct BCHP_P_7340_Info
{
    uint32_t      ulChipIdReg; /* index into the table. */

    /* Chip Id */
    uint16_t      usChipId;

    /* Major revision */
    uint16_t      usMajor;

    /* Minor revision */
    uint16_t      usMinor;

    /* TODO: Other features or infos if needed */
} BCHP_P_7340_Info;


/* Lookup table for chip features and etc.
 * The are many times when the chip device id register
 * not conforming to the standard numbering convention. We do 
 * it this way to work-around those problems. 
 * 
 * TODO: Update this table to support new revisions.
 */
static const BCHP_P_7340_Info s_aChipInfoTable[] =
{
#if BCHP_VER == BCHP_VER_A0 || BCHP_VER == BCHP_VER_A1
    /* A0 code will run on A0 and A1 */
    {0x73400000, BCHP_BCM7340, BCHP_MAJOR_A, BCHP_MINOR_0},
    {0x73400001, BCHP_BCM7340, BCHP_MAJOR_A, BCHP_MINOR_1},
#elif BCHP_VER == BCHP_VER_B0 || BCHP_VER == BCHP_VER_B1
    /* B0 code will run on B1 */
    {0x73400010, BCHP_BCM7340, BCHP_MAJOR_B, BCHP_MINOR_0},
    {0x73500010, BCHP_BCM7340, BCHP_MAJOR_B, BCHP_MINOR_0},
#else
    #error "Port required"
#endif
};


/* Chip context */
typedef struct BCHP_P_7340_Context
{
    uint32_t                           ulBlackMagic;
    BREG_Handle                        hRegister;
    const BCHP_P_7340_Info            *pChipInfo;
} BCHP_P_7340_Context;

/* Max entry of lookup table */
#define BCHP_P_CHIP_INFO_MAX_ENTRY \
    (sizeof(s_aChipInfoTable) / sizeof(BCHP_P_7340_Info))

/* This macro checks for a validity of a handle, and
 * cast to context pointer. */
#define BCHP_P_GET_CONTEXT(handle, context) \
{ \
    if(!(handle) || \
       !((handle)->chipHandle) || \
       (((BCHP_P_7340_Context*)((handle)->chipHandle))->ulBlackMagic != \
       sizeof(BCHP_P_7340_Context))) \
    { \
        BDBG_ERR(("Corrupted context handle\n")); \
        (context) = NULL; \
    } \
    else \
    { \
        (context) = (BCHP_P_7340_Context*)((handle)->chipHandle); \
    } \
    BDBG_ASSERT(context); \
}

/* Static function prototypes */
static BERR_Code BCHP_P_Close7340
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
 * Open BCM7340 Chip.
 *
 */
BERR_Code BCHP_Open7340
    ( BCHP_Handle                     *phChip,
      BREG_Handle                      hRegister )
{
    BCHP_P_Context *pChip;
    BCHP_P_7340_Context *p7340Chip;
    uint32_t ulChipIdReg;
    uint32_t ulIdx;
    uint32_t ulReg;
    uint32_t bankHeight=0, stripeWidth=0, dataWidth=0;
    uint32_t ddr0Device=0;
	bool ddr3=false;

    BDBG_ENTER(BCHP_Open7340);

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

    p7340Chip = (BCHP_P_7340_Context*)
        (BKNI_Malloc(sizeof(BCHP_P_7340_Context)));
    if(!p7340Chip)
    {
        BKNI_Free(pChip);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)p7340Chip, 0x0, sizeof(BCHP_P_7340_Context));

    /* Fill up the base chip context. */
    pChip->chipHandle       = (void*)p7340Chip;
    pChip->regHandle        = hRegister;
    pChip->pCloseFunc       = BCHP_P_Close7340;
    pChip->pGetChipInfoFunc = BCHP_P_GetChipInfoComformWithBaseClass;
    pChip->pGetFeatureFunc  = BCHP_P_GetFeature;

    /* Fill up the chip context. */
    p7340Chip->ulBlackMagic = sizeof(BCHP_P_7340_Context);
    p7340Chip->hRegister    = hRegister;

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
        const BCHP_P_7340_Info *compareChipInfo = &s_aChipInfoTable[ulIdx];

        if(compareChipInfo->ulChipIdReg == ulChipIdReg)
        {
            /* Chip Information. */
            p7340Chip->pChipInfo = compareChipInfo;
            break;
        }
        else if (ulIdx == BCHP_P_CHIP_INFO_MAX_ENTRY - 1 && compareChipInfo->usMajor == (ulChipIdReg&0xF0)>>4)
        {
            /* This is a future minor revision. We will allow it with a WRN. */
            BDBG_WRN(("An unknown minor revision %x%c%d has been detected. Certain operations may result in erratic behavior. Please confirm this chip revision is supported with this software.",
                PRINT_CHIP(ulChipIdReg)));
            p7340Chip->pChipInfo = compareChipInfo;
            break;
        }
    }

    if(!p7340Chip->pChipInfo)
    {
        BKNI_Free(p7340Chip);
        BKNI_Free(pChip);
        BDBG_ERR(("unsupported revision: %x%c%d", PRINT_CHIP(ulChipIdReg)));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("found %x%c%d", PRINT_CHIP(p7340Chip->pChipInfo->ulChipIdReg)));

    /* All done. now return the new fresh context to user. */
    *phChip = (BCHP_Handle)pChip;

    {
    /* Reset some cores. This is needed to avoid L1 interrupts before BXXX_Open can be called per core. */
    const uint32_t mask =
          BCHP_MASK( SUN_TOP_CTRL_SW_RESET, xpt_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, avd0_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, aio_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, rptd_sw_reset);

    BREG_AtomicUpdate32(hRegister, BCHP_SUN_TOP_CTRL_SW_RESET, mask,
          BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, xpt_sw_reset, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, avd0_sw_reset, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, aio_sw_reset, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, rptd_sw_reset, 1 ));

    /* Now clear the reset. */
    BREG_AtomicUpdate32(hRegister, BCHP_SUN_TOP_CTRL_SW_RESET, mask, 0);
    }


    /* read the strap registers and program system stripe width and pfri bank height */
    ulReg = BREG_Read32(hRegister, BCHP_MEMC_DDR_0_CNTRLR_CONFIG);
    ddr0Device = BCHP_GET_FIELD_DATA(ulReg, MEMC_DDR_0_CNTRLR_CONFIG, DEVICE_TECH);

    BDBG_MSG(("Strap ddr0 Device Tech %x, ",ddr0Device));

	/* read the strap registers and program system stripe width and pfri bank height */
	ulReg = BREG_Read32(hRegister, BCHP_MEMC_DDR_0_DRAM_INIT_CNTRL);
	ddr3 = BCHP_GET_FIELD_DATA(ulReg, MEMC_DDR_0_DRAM_INIT_CNTRL, DDR_TECH);
	BDBG_MSG(("ddr%1d detected",ddr3 ? 3:2));


    /* Table to Determine Stripe Width and Bank Height based on Device Tech */
	/* Only 32-bit UMA is supported. */
	dataWidth =1; /* 128-b */
	{
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
			case 4: /* 4096 Mbits Device Tech*/
				stripeWidth =1; /* 128 bytes */
				bankHeight = 1; /* 2 Mblks */
				break;
		default:
				BDBG_ERR(("Unknown Value in DDR Device Config Register"));
				break;
		}

	}
    BREG_AtomicUpdate32(hRegister, 
            BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, 
            BCHP_MASK(DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, pfri_data_width) |
			BCHP_MASK(DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, ddr3_mode),
            BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, pfri_data_width, dataWidth ) |
			BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, ddr3_mode, ddr3 )
    );

    BREG_AtomicUpdate32(hRegister, 
        BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH,
        BCHP_MASK( DECODE_SD_0_REG_SD_STRIPE_WIDTH, pfri_bank_height) |
		BCHP_MASK( DECODE_SD_0_REG_SD_STRIPE_WIDTH, Stripe_Width),
        BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_STRIPE_WIDTH, pfri_bank_height, bankHeight ) |
		BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_STRIPE_WIDTH, Stripe_Width, stripeWidth )
    );

    BDBG_MSG(("data width=%#x, stripe width=%#x,pfri bank height=%#x",dataWidth,stripeWidth,bankHeight));
    BDBG_LEAVE(BCHP_Open7340);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_Close7340
    ( BCHP_Handle                      hChip )
{
    BCHP_P_7340_Context *p7340Chip;

    BDBG_ENTER(BCHP_P_Close7340);
    
    BCHP_P_GET_CONTEXT(hChip, p7340Chip);

    if(!p7340Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Invalidate the magic number. */
    p7340Chip->ulBlackMagic = 0;

    BKNI_Free((void*)p7340Chip);
    BKNI_Free((void*)hChip);

    BDBG_LEAVE(BCHP_P_Close7340);
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
    const BCHP_P_7340_Context *p7340Chip;

    BDBG_ENTER(BCHP_P_GetChipInfo);

    BCHP_P_GET_CONTEXT(hChip, p7340Chip);

    if(!p7340Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(pusChipId)
    {
        *pusChipId = p7340Chip->pChipInfo->usChipId;
    }

    if(pusChipMajorRev)
    {
        *pusChipMajorRev = p7340Chip->pChipInfo->usMajor;
    }

    if(pusChipMinorRev)
    {
        *pusChipMinorRev = p7340Chip->pChipInfo->usMinor;
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
    BCHP_P_7340_Context *p7340Chip;
    uint32_t             ulBondStatus;

    BDBG_ENTER(BCHP_P_GetFeature);
    
    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7340Chip);

    /* read bond-out status common for many features */
    ulBondStatus = BREG_Read32(p7340Chip->hRegister,
        BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_0);
        
    /* which feature? */
    switch (eFeature)
    {
    case BCHP_Feature_e3DGraphicsCapable:
        /* 3D capable? (bool) */
        *(bool *)pFeatureValue = BCHP_GET_FIELD_DATA(ulBondStatus, 
            SUN_TOP_CTRL_OTP_OPTION_STATUS_0, otp_option_3d_disable ) ? false : true;
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
            SUN_TOP_CTRL_OTP_OPTION_STATUS_0, otp_option_macrovision_enable ) ? true : false;
        rc = BERR_SUCCESS;
        break;
        
    case BCHP_Feature_eMpegDecoderCount:
        /* number of MPEG decoders (int) */
        *(int *)pFeatureValue = true;
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
        *(bool *)pFeatureValue = false;
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

/* End of File */
