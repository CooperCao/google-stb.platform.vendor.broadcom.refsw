/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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
 * Module Description: Application to generate RULs for splash screen.
 *                     This is a slightly modified copy of vdc_dump.c
 *                     ported to Nucleus
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

/* #include "bchp_sec_it.h"
##include "bchp_prim_it.h" */

/* For register exclusions */
#include "bchp_common.h"
/* #include "bchp_dtg.h" */
#include "bchp_irq0.h"
#include "bchp_timer.h"
#include "bchp_scirq0.h"
#include "bchp_ebi.h"
#include "bchp_bsca.h"
#include "bchp_bscd.h"
#include "bchp_gio.h"
#include "bchp_memc_gen_0.h"
#include "bchp_memc_gen_1.h"
#include "bchp_rdc.h"
#include "bchp_memc_arc_0.h"
#include "bchp_memc_arc_1.h"

/* These are the registers that need to be excluded from the register dump either
   because they interrupt the CPU or disturb settings done elsewhere like the CFE
   If you do not want to program certain registers - add them to this macro.
*/
#define BSPLASH_ADDRESS_IN_RANGE(addr, rangeLow, rangeHigh) ((addr>=rangeLow) && (addr <=rangeHigh))

#define BSPLASH_REGDUMP_EXCLUDE(addr)   ( \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_BVNF_INTR2_0_REG_START, BCHP_BVNF_INTR2_5_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_BVNB_INTR2_REG_START, BCHP_BVNB_INTR2_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_HIF_INTR2_REG_START, BCHP_HIF_CPU_INTR1_REG_END)   || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_XPT_BUS_IF_REG_START, BCHP_XPT_XPU_REG_END)   || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_SHVD_INTR2_0_REG_START, BCHP_SHVD_INTR2_0_REG_END)   || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_SHVD_INTR2_1_REG_START, BCHP_SHVD_INTR2_1_REG_END)   || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_RAAGA_DSP_INTH_REG_START, BCHP_RAAGA_DSP_INTH_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_M2MC_L2_REG_START, BCHP_M2MC_L2_REG_END)  || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_M2MC1_L2_REG_START, BCHP_M2MC1_L2_REG_END)  || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_HDMI_TX_INTR2_REG_START, BCHP_HDMI_TX_INTR2_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_VIDEO_ENC_INTR2_REG_START, BCHP_VIDEO_ENC_INTR2_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_BVNB_INTR2_REG_START, BCHP_BVNB_INTR2_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_MEMC_ARC_0_REG_START, BCHP_MEMC_ARC_0_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_MEMC_ARC_1_REG_START, BCHP_MEMC_ARC_1_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_EBI_CS_BASE_0, BCHP_EBI_ECR) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_TIMER_TIMER_IS, BCHP_TIMER_WDCTRL) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_GIO_ODEN_LO, BCHP_GIO_STAT_EXT) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_IRQ0_IRQEN, BCHP_IRQ0_IRQEN) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_SCIRQ0_SCIRQEN, BCHP_SCIRQ0_SCIRQEN) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_TIMER_TIMER_IE0, BCHP_TIMER_TIMER_IE0) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_RAAGA_DSP_SEC0_REG_START, BCHP_RAAGA_DSP_MEM_SUBSYSTEM_1_REG_END)   || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_BSP_CMDBUF_REG_START, BCHP_XPT_SECURITY_NS_REG_END)   || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_RDC_REG_START, BCHP_RDC_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_MEMC_L2_1_0_REG_START, BCHP_MEMC_L2_1_0_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_CPU_PROG_MEM_REG_START, BCHP_AVS_PMB_REGISTERS_REG_END)\
        )

/* Removed  BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_BSCA_CHIP_ADDRESS, BCHP_BSCD_SCL_PARAM) || \ */

/* number of mem heap handles  */
#define SPLASH_NUM_MEM        2

/* heap for RUL bufs */
#define SPLASH_RUL_MEM        1   /* idx to BMEM_Handle array */

/* number of surface */
#define SPLASH_NUM_SURFACE    2

/* surface 0 info */
#define SPLASH_SURF0_MEM      1   /* idx to BMEM_Handle array */
#define SPLASH_SURF0_PXL_FMT  BPXL_eR5_G6_B5
#define SPLASH_SURF0_BMP      "splash.bmp"

/* surface 1 info */
#define SPLASH_SURF1_MEM      1   /* idx to BMEM_Handle array */
#define SPLASH_SURF1_PXL_FMT  BPXL_eR5_G6_B5 
#define SPLASH_SURF1_BMP      "splash.bmp"

/* number of display */
#define SPLASH_NUM_DISPLAY    2

#ifdef  CFG_SPLASH_PAL
/* display 0 info */
#define SPLASH_DISP0_FMT      BFMT_VideoFmt_e576p_50Hz 
#define SPLASH_DISP0_SUR      0   /* idx to splash surface buffer array */

/* display 1 info */
#define SPLASH_DISP1_FMT      BFMT_VideoFmt_ePAL_I 
#define SPLASH_DISP1_SUR      1   /* idx to splash surface buffer array */

#else
/* display 0 info */
#define SPLASH_DISP0_FMT      BFMT_VideoFmt_e480p
#define SPLASH_DISP0_SUR      0   /* idx to splash surface buffer array */

/* display 1 info */
#define SPLASH_DISP1_FMT      BFMT_VideoFmt_eNTSC
#define SPLASH_DISP1_SUR      1   /* idx to splash surface buffer array */
#endif

#define SPLASH_NUM_SVIDEO_OUTPUTS   0
#define SPLASH_NUM_COMPOSITE_OUTPUTS    1
#define SPLASH_NUM_COMPONENT_OUTPUTS    1

#define BRCM_DAC_SVIDEO_LUMA      BVDC_Dac_5
#define BRCM_DAC_SVIDEO_CHROMA    BVDC_Dac_6
#define BRCM_DAC_COMPOSITE_0      BVDC_Dac_3

#define BRCM_DAC_Y                BVDC_Dac_0
#define BRCM_DAC_PR               BVDC_Dac_2
#define BRCM_DAC_PB               BVDC_Dac_1

#define B_I2C_CHANNEL_HDMI    3

/* End of File */



