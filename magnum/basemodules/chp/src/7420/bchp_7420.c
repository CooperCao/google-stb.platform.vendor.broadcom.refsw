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
#include "bchp_7420.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_decode_sd_0.h"
#include "bchp_decode_sd_1.h"
#include "bchp_decode_ip_shim_0.h"
#include "bchp_sun_gisb_arb.h"
#include "bchp_clk.h"
#include "bchp_pwr.h"

#if(BCHP_VER >= BCHP_VER_B0)
#include "bchp_memc_ddr23_aphy_ac_0.h"
#include "bchp_memc_ddr23_aphy_ac_1.h"
#include "bchp_memc_ddr_0.h"
#include "bchp_memc_ddr_1.h"
#endif

BDBG_MODULE(BCHP);

/* Miscellaneous macros. */
#define BCHP_P_MAJOR_REV_SHIFT          (4)

/* Chip info and features */
typedef struct BCHP_P_7420_Info
{
    uint32_t      ulChipIdReg; /* index into the table. */

    /* Chip Id */
    uint16_t      usChipId;

    /* Major revision */
    uint16_t      usMajor;

    /* Minor revision */
    uint16_t      usMinor;

    /* TODO: Other features or infos if needed */
} BCHP_P_7420_Info;


/* Lookup table for chip features and etc.
 * The are many times when the chip device id register
 * not conforming to the standard numbering convention. We do
 * it this way to work-around those problems.
 *
 * TODO: Update this table to support new revisions.
 */
static const BCHP_P_7420_Info s_aChipInfoTable[] =
{
#if BCHP_VER == BCHP_VER_A0
    /* A0 code will run on A0 */
    {0x74200000, BCHP_BCM7420, BCHP_MAJOR_A, BCHP_MINOR_0},
#elif BCHP_VER == BCHP_VER_A1
    /* A1 code will run on A1 */
    {0x74200001, BCHP_BCM7420, BCHP_MAJOR_A, BCHP_MINOR_1},
#elif BCHP_VER == BCHP_VER_B0
    /* A1 code will run on A1 */
    {0x74200010, BCHP_BCM7420, BCHP_MAJOR_B, BCHP_MINOR_0},
    {0x74100010, BCHP_BCM7420, BCHP_MAJOR_B, BCHP_MINOR_0},
#elif BCHP_VER == BCHP_VER_C0
    {0x74200020, BCHP_BCM7420, BCHP_MAJOR_C, BCHP_MINOR_0},
    {0x74100020, BCHP_BCM7420, BCHP_MAJOR_C, BCHP_MINOR_0},
    {0x33200020, BCHP_BCM7420, BCHP_MAJOR_C, BCHP_MINOR_0},
#elif BCHP_VER == BCHP_VER_C1
    {0x74200021, BCHP_BCM7420, BCHP_MAJOR_C, BCHP_MINOR_1},
    {0x74100021, BCHP_BCM7420, BCHP_MAJOR_C, BCHP_MINOR_1},
    {0x33200021, BCHP_BCM7420, BCHP_MAJOR_C, BCHP_MINOR_1},
    {0x74090021, BCHP_BCM7420, BCHP_MAJOR_C, BCHP_MINOR_1},
#else
    #error "Port required"
#endif
};


/* Chip context */
typedef struct BCHP_P_7420_Context
{
    uint32_t                           ulBlackMagic;
    BREG_Handle                        hRegister;
    const BCHP_P_7420_Info            *pChipInfo;
} BCHP_P_7420_Context;

/* Max entry of lookup table */
#define BCHP_P_CHIP_INFO_MAX_ENTRY \
    (sizeof(s_aChipInfoTable) / sizeof(BCHP_P_7420_Info))

/* This macro checks for a validity of a handle, and
 * cast to context pointer. */
#define BCHP_P_GET_CONTEXT(handle, context) \
{ \
    if(!(handle) || \
       !((handle)->chipHandle) || \
       (((BCHP_P_7420_Context*)((handle)->chipHandle))->ulBlackMagic != \
       sizeof(BCHP_P_7420_Context))) \
    { \
        BDBG_ERR(("Corrupted context handle\n")); \
        (context) = NULL; \
    } \
    else \
    { \
        (context) = (BCHP_P_7420_Context*)((handle)->chipHandle); \
    } \
    BDBG_ASSERT(context); \
}

/* Static function prototypes */
static BERR_Code BCHP_P_Close7420
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
 * Open BCM7420 Chip.
 *
 */
BERR_Code BCHP_Open7420
    ( BCHP_Handle                     *phChip,
      BREG_Handle                      hRegister )
{
    BCHP_P_Context *pChip;
    BCHP_P_7420_Context *p7420Chip;
    uint32_t ulChipIdReg;
    uint32_t ulIdx;
    uint32_t ulReg,ulMemc,ddr_type, chip_id;
    uint32_t bank0Height=0, stripe0Width=0, data0Width=0, pfri0Source=0;
    uint32_t bank1Height=0, stripe1Width=0, data1Width=0, pfri1Source=0;
    uint32_t ddr0Device=0, ddr1Device =0;
    uint32_t memc0_config=0, memc1_config=0;
    BERR_Code rc;

    BDBG_ENTER(BCHP_Open7420);

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

    p7420Chip = (BCHP_P_7420_Context*)
        (BKNI_Malloc(sizeof(BCHP_P_7420_Context)));
    if(!p7420Chip)
    {
        BKNI_Free(pChip);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)p7420Chip, 0x0, sizeof(BCHP_P_7420_Context));

    /* Fill up the base chip context. */
    pChip->chipHandle       = (void*)p7420Chip;
    pChip->regHandle        = hRegister;
    pChip->pCloseFunc       = BCHP_P_Close7420;
    pChip->pGetChipInfoFunc = BCHP_P_GetChipInfoComformWithBaseClass;
    pChip->pGetFeatureFunc  = BCHP_P_GetFeature;

    /* Fill up the chip context. */
    p7420Chip->ulBlackMagic = sizeof(BCHP_P_7420_Context);
    p7420Chip->hRegister    = hRegister;

    /* Open BCHP_PWR */
    rc = BCHP_PWR_Open(&pChip->pwrManager, pChip); 
    if (rc) {
        BKNI_Free(pChip);
        BKNI_Free(p7420Chip);
        return BERR_TRACE(rc);
    }

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
        const BCHP_P_7420_Info *compareChipInfo = &s_aChipInfoTable[ulIdx];

        if(compareChipInfo->ulChipIdReg == ulChipIdReg)
        {
            /* Chip Information. */
            p7420Chip->pChipInfo = compareChipInfo;
            break;
        }
        else if (ulIdx == BCHP_P_CHIP_INFO_MAX_ENTRY - 1 && compareChipInfo->usMajor == (ulChipIdReg&0xF0)>>4)
        {
            /* This is a future minor revision. We will allow it with a WRN. */
            BDBG_WRN(("An unknown minor revision %x%c%d has been detected. Certain operations may result in erratic behavior. Please confirm this chip revision is supported with this software.",
                PRINT_CHIP(ulChipIdReg)));
            p7420Chip->pChipInfo = compareChipInfo;
            break;
        }
		else if (ulChipIdReg == 0x74200010 && BCHP_VER == BCHP_VER_C0)
		{
    		BDBG_ERR(("*****************************************************************\n"));
            BDBG_ERR(("Note: A 7420C0 build is running on a chip with Chip ID 7420B0 \n"));
    		BDBG_ERR(("*****************************************************************\n"));
            p7420Chip->pChipInfo = &s_aChipInfoTable[0];
            break;
		}
    }

    if(!p7420Chip->pChipInfo)
    {
        BKNI_Free(p7420Chip);
        BKNI_Free(pChip);
    	BDBG_ERR(("*****************************************************************\n"));
    	BDBG_ERR(("ERROR ERROR ERROR ERROR \n"));
        BDBG_ERR(("Unsupported Revision: %x%c%d", PRINT_CHIP(ulChipIdReg)));
    	BDBG_ERR(("*****************************************************************\n"));
	phChip = NULL;
	BDBG_ASSERT(phChip);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("found %x%c%d", PRINT_CHIP(p7420Chip->pChipInfo->ulChipIdReg)));

    /* All done. now return the new fresh context to user. */
    *phChip = (BCHP_Handle)pChip;


    {
    /* Reset some cores. This is needed to avoid L1 interrupts before BXXX_Open can be called per core. */
    const uint32_t mask =
          BCHP_MASK( SUN_TOP_CTRL_SW_RESET, xpt_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, avd0_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, avd1_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, vec_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, aio_sw_reset)
        | BCHP_MASK( SUN_TOP_CTRL_SW_RESET, rptd_sw_reset);

    BREG_AtomicUpdate32(hRegister, BCHP_SUN_TOP_CTRL_SW_RESET, mask,
          BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, xpt_sw_reset, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, avd0_sw_reset, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, avd1_sw_reset, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, vec_sw_reset, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, aio_sw_reset, 1 )
        | BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, rptd_sw_reset, 1 ));

    /* Now clear the reset. */
    BREG_AtomicUpdate32(hRegister, BCHP_SUN_TOP_CTRL_SW_RESET, mask, 0);
    }


#if(BCHP_VER >= BCHP_VER_B0)
	/* Read DDR parameters */
    /* DDR Mode: 0 (DDR3 parts), 1 (DDR2 parts) */
    ulReg = BREG_Read32(hRegister, BCHP_MEMC_DDR23_APHY_AC_0_CONFIG);
	ddr_type = BCHP_GET_FIELD_DATA(ulReg, MEMC_DDR23_APHY_AC_0_CONFIG, DDR_MODE);

    /* DRAM Width: 0 (32 bit), 1 (16 bit) */
    ulReg = BREG_Read32(hRegister, BCHP_MEMC_DDR23_APHY_AC_0_CONFIG);
	memc0_config = BCHP_GET_FIELD_DATA(ulReg, MEMC_DDR23_APHY_AC_0_CONFIG, DRAM_WIDTH);

    ulReg = BREG_Read32(hRegister, BCHP_MEMC_DDR23_APHY_AC_1_CONFIG);
	memc1_config = BCHP_GET_FIELD_DATA(ulReg, MEMC_DDR23_APHY_AC_1_CONFIG, DRAM_WIDTH);

    /* Device Tech: 0 (256Mbits), 1 (512MBbits), 2 (1Gbit), 3 (2Gbit), 4 (4Gbit) */
    ulReg = BREG_Read32(hRegister, BCHP_MEMC_DDR_0_CNTRLR_CONFIG);
	ddr0Device = BCHP_GET_FIELD_DATA(ulReg, MEMC_DDR_0_CNTRLR_CONFIG, DEVICE_TECH);

    ulReg = BREG_Read32(hRegister, BCHP_MEMC_DDR_1_CNTRLR_CONFIG);
	ddr1Device = BCHP_GET_FIELD_DATA(ulReg, MEMC_DDR_1_CNTRLR_CONFIG, DEVICE_TECH);
#else
	memc0_config = 0;
	memc1_config = 0;
	ddr0Device = 2;
	ddr1Device = 2;
	/* 7420 A0/A1: DDR2 */
	ddr_type = 1;
#endif

    /* 7420 Dual decode requires 32b+32b (memc0 + memc1)
	   7420/7410 Single decode can be configured as 32b+16b (memc0 + memc1) */

	/* Dual decode - MEMC1 configured in 32b mode */
    if (memc1_config == 0)
    {
        ulMemc = 1; /* 32b+32b configuration */
     }
	/* Single decode - MEMC1 configured in 16b mode */
    else
    {
        ulMemc = 0; /* 32b+16b configuration */
    }

	/* Table to Determine Stripe Width and Bank Height based on Device Tech and yyy*/
    switch (ulMemc)
    {
        case 0: /* 32b+16b */
        	{
				/* 32b */
                switch(ddr0Device)
                {
                    case 0: /* 256 Mbits Device Tech*/
                        stripe0Width =0; /* 64 bytes */
                        bank0Height = 1; /* 2 Mblks */
                        break;
                    case 1: /* 512 Mbits Device Tech*/
                        stripe0Width =1; /* 128 bytes */
                        bank0Height = 1; /* 2 Mblks */
                        break;
                    case 2: /* 1024 Mbits Device Tech*/
                        stripe0Width =1; /* 128 bytes */
                        bank0Height = 1; /* 2 Mblks */
                        break;
                    case 3: /* 2048 Mbits Device Tech*/
                        stripe0Width =1; /* 128 bytes */
                        bank0Height = 1; /* 2 Mblks */
                        break;
                    case 4: /* 4096 Mbits Device Tech*/
                        stripe0Width =1; /* 128 bytes */
                        bank0Height = 1; /* 2 Mblks */
                        break;
                default:
                        BDBG_ERR(("Unknown Value in DDR0 Device Config Register"));
                        break;
                }

				/* 16b */
                switch(ddr1Device)
                {
                    case 0: /* 256 Mbits Device Tech*/
                        stripe1Width =0; /* 64 bytes */
                        bank1Height = 0; /* 1 Mblks */
                        break;
                    case 1: /* 512 Mbits Device Tech*/
                        stripe1Width =0; /* 64 bytes */
                        bank1Height = 1; /* 2 Mblks */
                        break;
                    case 2: /* 1024 Mbits Device Tech*/
                        stripe1Width =0; /* 64 bytes */
                        bank1Height = 1; /* 2 Mblks */
                        break;
                    case 3: /* 2048 Mbits Device Tech*/
                        stripe1Width =0; /* 64 bytes */
                        bank1Height = 1; /* 2 Mblks */
                        break;
                    case 4: /* 4096 Mbits Device Tech*/
                        stripe1Width =0; /* 64 bytes */
                        bank1Height = 1; /* 2 Mblks */
                        break;
                default:
                        BDBG_ERR(("Unknown Value in DDR1 Device Config Register"));
                        break;
                }
			}

            pfri0Source=0;                           /* PFRI0 attached to SCB0 */
            data0Width = 1;                          /* 128 bit on MEMC0 */

            pfri1Source=1;                           /* PFRI1 attached to SCB1 */
            data1Width = 0;                          /* 64 bit on MEMC1 */

            break;

        case 1: /* 32b+32b */
        	{
                /* 32b */
                switch(ddr0Device)
                {
                    case 0: /* 256 Mbits Device Tech*/
                        stripe0Width =0; /* 64 bytes */
                        bank0Height = 1; /* 2 Mblks */
                        break;
                    case 1: /* 512 Mbits Device Tech*/
                        stripe0Width =1; /* 128 bytes */
                        bank0Height = 1; /* 2 Mblks */
                        break;
                    case 2: /* 1024 Mbits Device Tech*/
                        stripe0Width =1; /* 128 bytes */
                        bank0Height = 1; /* 2 Mblks */
                        break;
                    case 3: /* 2048 Mbits Device Tech*/
                        stripe0Width =1; /* 128 bytes */
                        bank0Height = 1; /* 2 Mblks */
                        break;
                    case 4: /* 4096 Mbits Device Tech*/
                            /* This is the appropriate Device Tech for boards with a configuration
                               of 1GB of memory on MEMC0 consisting of 4 pieces of 256Mx8 = 4x2Gbit
                               - The controller views every two x8 devices as a single x16 device so
                                 the Device Tech is the sum of two x8 devices, i.e. 4Gb */
                        stripe0Width =1; /* 128 bytes */
                        bank0Height = 1; /* 2 Mblks */
                        break;
                default:
                        BDBG_ERR(("Unknown Value in DDR0 Device Config Register"));
                        break;
                }

                /* 32b */
                switch(ddr1Device)
                {
                    case 0: /* 256 Mbits Device Tech*/
                        stripe1Width =0; /* 64 bytes */
                        bank1Height = 1; /* 2 Mblks */
                        break;
                    case 1: /* 512 Mbits Device Tech*/
                        stripe1Width =1; /* 128 bytes */
                        bank1Height = 1; /* 2 Mblks */
                        break;
                    case 2: /* 1024 Mbits Device Tech*/
                        stripe1Width =1; /* 128 bytes */
                        bank1Height = 1; /* 2 Mblks */
                        break;
                    case 3: /* 2048 Mbits Device Tech*/
                        stripe1Width =1; /* 128 bytes */
                        bank1Height = 1; /* 2 Mblks */
                        break;
                    case 4: /* 4096 Mbits Device Tech*/
                        stripe1Width =1; /* 128 bytes */
                        bank1Height = 1; /* 2 Mblks */
                        break;
                default:
                        BDBG_ERR(("Unknown Value in DDR1 Device Config Register"));
                        break;
                }
			}

            pfri0Source=0;                           /* PFRI0 attached to SCB0 */
            data0Width = 1;                          /* 128 bit on MEMC0 */

            pfri1Source=1;                           /* PFRI1 attached to SCB1 */
            data1Width = 1;                          /* 128 bit on MEMC1 */

            break;
    default:
        BDBG_ERR(("Invalid Memory controller configuration = %x", ulMemc));

     }

#if BCHP_PWR_RESOURCE_AVD0
    BCHP_PWR_AcquireResource(pChip, BCHP_PWR_RESOURCE_AVD0);
#endif
    /* Program Stripe Width and PFRI Data width for MEMC0 */
    BREG_AtomicUpdate32(hRegister, BCHP_DECODE_SD_0_REG_SD_STRIPE_WIDTH,
               BCHP_MASK( DECODE_SD_0_REG_SD_STRIPE_WIDTH, Stripe_Width)|
               BCHP_MASK( DECODE_SD_0_REG_SD_STRIPE_WIDTH, pfri_bank_height),
               BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_STRIPE_WIDTH, Stripe_Width, stripe0Width )|
               BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_STRIPE_WIDTH, pfri_bank_height, bank0Height ));

    BREG_AtomicUpdate32(hRegister, BCHP_DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH,
               BCHP_MASK( DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, pfri_data_width)|
               BCHP_MASK( DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, pfri_source),
               BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, ddr3_mode, ~ddr_type)|
               BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, pfri_data_width, data0Width)|
               BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, pfri_source, pfri0Source));
#if BCHP_PWR_RESOURCE_AVD0    
    BCHP_PWR_ReleaseResource(pChip, BCHP_PWR_RESOURCE_AVD0);
#endif

    ulReg = BREG_Read32(hRegister, BCHP_SUN_TOP_CTRL_PROD_REVISION);
    chip_id = BCHP_GET_FIELD_DATA(ulReg, SUN_TOP_CTRL_PROD_REVISION, product_revision);
    chip_id >>= 16;

#if BCHP_PWR_RESOURCE_AVD1
    BCHP_PWR_AcquireResource(pChip, BCHP_PWR_RESOURCE_AVD1);
#endif
    if (chip_id == 0x7420 || chip_id == 0x7409)
    {
        /* Program Stripe Width and PFRI Data width for MEMC1 */
        BREG_AtomicUpdate32(hRegister, BCHP_DECODE_SD_1_REG_SD_STRIPE_WIDTH,
            BCHP_MASK( DECODE_SD_0_REG_SD_STRIPE_WIDTH, Stripe_Width)|
            BCHP_MASK( DECODE_SD_0_REG_SD_STRIPE_WIDTH, pfri_bank_height),
            BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_STRIPE_WIDTH, Stripe_Width, stripe1Width )|
            BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_STRIPE_WIDTH, pfri_bank_height, bank1Height ));

        BREG_AtomicUpdate32(hRegister, BCHP_DECODE_SD_1_REG_SD_PFRI_DATA_WIDTH,
            BCHP_MASK( DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, pfri_data_width)|
            BCHP_MASK( DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, pfri_source),
            BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, ddr3_mode, ~ddr_type)|
            BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, pfri_data_width, data1Width)|
            BCHP_FIELD_DATA( DECODE_SD_0_REG_SD_PFRI_DATA_WIDTH, pfri_source, pfri1Source));
    }
#if BCHP_PWR_RESOURCE_AVD1
    BCHP_PWR_ReleaseResource(pChip, BCHP_PWR_RESOURCE_AVD1);
#endif

    /* BDBG_WRN(("stripe width=%#x,pfri bank height=%#x",BCHP_GET_FIELD_DATA(widthReg,DECODE_SD_0_REG_SD_STRIPE_WIDTH,Strip_Width),
              BCHP_GET_FIELD_DATA(shimReg,DECODE_IP_SHIM_0_PFRI_REG,pfri_bank_height))); */
    BDBG_MSG(("Hack Hack,programming BCHP_SUN_GISB_ARB_REQ_MASK, this should be done in CFE"));
    ulReg = BREG_Read32(hRegister, BCHP_SUN_GISB_ARB_REQ_MASK);
    ulReg &= ~(BCHP_MASK( SUN_GISB_ARB_REQ_MASK, avd0)|
               BCHP_MASK( SUN_GISB_ARB_REQ_MASK, rptd)|
               BCHP_MASK( SUN_GISB_ARB_REQ_MASK, rdc));
    BREG_Write32(hRegister, BCHP_SUN_GISB_ARB_REQ_MASK, ulReg);

    /*
     *  Refer to SWCFE-294 for explanation on why the below 
     *  power down and power up is required.
     */

    /* Power down AVD0 clocks */
    ulReg = (BCHP_CLK_AVD0_CLK_PM_CTRL_DIS_108M_CLK_MASK |
             BCHP_CLK_AVD0_CLK_PM_CTRL_DIS_216M_CLK_MASK |
             BCHP_CLK_AVD0_CLK_PM_CTRL_DIS_250M_CLK_MASK);
    BREG_AtomicUpdate32(hRegister, BCHP_CLK_AVD0_CLK_PM_CTRL,ulReg,ulReg);

    /* Power down AVD1 clocks */
    ulReg = (BCHP_CLK_AVD1_CLK_PM_CTRL_DIS_108M_CLK_MASK |
             BCHP_CLK_AVD1_CLK_PM_CTRL_DIS_216M_CLK_MASK |
             BCHP_CLK_AVD1_CLK_PM_CTRL_DIS_250M_CLK_MASK);
    BREG_AtomicUpdate32(hRegister, BCHP_CLK_AVD1_CLK_PM_CTRL,ulReg,ulReg);

    /* Power down audio core clocks */
    ulReg = (BCHP_CLK_RPT_AIO_CLK_PM_CTRL_DIS_RPT_250M_CLK_MASK |
             BCHP_CLK_RPT_AIO_CLK_PM_CTRL_DIS_108M_CLK_MASK |
             BCHP_CLK_RPT_AIO_CLK_PM_CTRL_DIS_216M_CLK_MASK);
    BREG_AtomicUpdate32(hRegister, BCHP_CLK_RPT_AIO_CLK_PM_CTRL,ulReg,ulReg);

     /* Power up AVD0 108M clock */
    ulReg = (BCHP_CLK_AVD0_CLK_PM_CTRL_DIS_108M_CLK_MASK |
             BCHP_CLK_AVD0_CLK_PM_CTRL_DIS_216M_CLK_MASK |
             BCHP_CLK_AVD0_CLK_PM_CTRL_DIS_250M_CLK_MASK);
    BREG_AtomicUpdate32(hRegister, BCHP_CLK_AVD0_CLK_PM_CTRL,ulReg,0);

    /* Power up AVD1 clocks */
    ulReg = (BCHP_CLK_AVD1_CLK_PM_CTRL_DIS_108M_CLK_MASK |
             BCHP_CLK_AVD1_CLK_PM_CTRL_DIS_216M_CLK_MASK |
             BCHP_CLK_AVD1_CLK_PM_CTRL_DIS_250M_CLK_MASK);
    BREG_AtomicUpdate32(hRegister, BCHP_CLK_AVD1_CLK_PM_CTRL,ulReg,0);

    /* Power up audio clocks */
    ulReg = (BCHP_CLK_RPT_AIO_CLK_PM_CTRL_DIS_RPT_250M_CLK_MASK |
             BCHP_CLK_RPT_AIO_CLK_PM_CTRL_DIS_108M_CLK_MASK |
             BCHP_CLK_RPT_AIO_CLK_PM_CTRL_DIS_216M_CLK_MASK);
    BREG_AtomicUpdate32(hRegister, BCHP_CLK_RPT_AIO_CLK_PM_CTRL,ulReg,0);

    BDBG_LEAVE(BCHP_Open7420);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
static BERR_Code BCHP_P_Close7420
    ( BCHP_Handle                      hChip )
{
    BCHP_P_7420_Context *p7420Chip;

    BDBG_ENTER(BCHP_P_Close7420);

    BCHP_P_GET_CONTEXT(hChip, p7420Chip);

    if(!p7420Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BCHP_PWR_Close(hChip->pwrManager);
    
    /* Invalidate the magic number. */
    p7420Chip->ulBlackMagic = 0;

    BKNI_Free((void*)p7420Chip);
    BKNI_Free((void*)hChip);

    BDBG_LEAVE(BCHP_P_Close7420);
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
    const BCHP_P_7420_Context *p7420Chip;

    BDBG_ENTER(BCHP_P_GetChipInfo);

    BCHP_P_GET_CONTEXT(hChip, p7420Chip);

    if(!p7420Chip)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(pusChipId)
    {
        *pusChipId = p7420Chip->pChipInfo->usChipId;
    }

    if(pusChipMajorRev)
    {
        *pusChipMajorRev = p7420Chip->pChipInfo->usMajor;
    }

    if(pusChipMinorRev)
    {
        *pusChipMinorRev = p7420Chip->pChipInfo->usMinor;
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
    BCHP_P_7420_Context *p7420Chip;
    uint32_t             ulBondStatus;

    BDBG_ENTER(BCHP_P_GetFeature);

    /* get base context */
    BCHP_P_GET_CONTEXT(hChip, p7420Chip);

    /* read bond-out status common for many features */
#if(BCHP_VER >= BCHP_VER_B0)
    ulBondStatus = BREG_Read32(p7420Chip->hRegister,
        BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS_0);
#else
    ulBondStatus = BREG_Read32(p7420Chip->hRegister,
        BCHP_SUN_TOP_CTRL_OTP_OPTION_STATUS);
#endif

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
#if(BCHP_VER >= BCHP_VER_B0)
		*(bool *)pFeatureValue = BCHP_GET_FIELD_DATA(ulBondStatus,
			SUN_TOP_CTRL_OTP_OPTION_STATUS_0, otp_option_macrovision_enable ) ? true : false;
#else
		*(bool *)pFeatureValue = BCHP_GET_FIELD_DATA(ulBondStatus,
			SUN_TOP_CTRL_OTP_OPTION_STATUS, otp_option_macrovision_enable ) ? true : false;
#endif
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_eMpegDecoderCount:
        /* number of MPEG decoders (int) */
        *(int *)pFeatureValue = 2;
        rc = BERR_SUCCESS;
        break;

    case BCHP_Feature_eHdcpCapable:
        /* HDCP capable? (bool) */
#if(BCHP_VER >= BCHP_VER_B0)
        *(bool *)pFeatureValue = BCHP_GET_FIELD_DATA(ulBondStatus,
            SUN_TOP_CTRL_OTP_OPTION_STATUS_0, otp_option_hdcp_disable ) ? false : true;
#else
        *(bool *)pFeatureValue = BCHP_GET_FIELD_DATA(ulBondStatus,
            SUN_TOP_CTRL_OTP_OPTION_STATUS, otp_option_hdcp_disable ) ? false : true;
#endif
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

/* End of File */
