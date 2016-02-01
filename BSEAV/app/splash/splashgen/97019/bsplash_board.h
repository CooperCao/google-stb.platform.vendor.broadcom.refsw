/***************************************************************************
 *     Copyright (c) 2005-2010, Broadcom Corporation
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

/* For register exclusions */
#include "bchp_common.h"
#include "bchp_irq0.h"
#include "bchp_timer.h"
#include "bchp_scirq0.h"
#include "bchp_ebi.h"
#include "bchp_bsca.h"
#include "bchp_gio.h"
#include "bchp_memc_gen_0.h"


/* These are the registers that need to be excluded from the register dump either
   because they interrupt the CPU or disturb settings done elsewhere like the CFE 
   If you do not want to program certain registers - add them to this macro. 
*/
#define BSPLASH_ADDRESS_IN_RANGE(addr, rangeLow, rangeHigh) ((addr>=rangeLow) && (addr <=rangeHigh))

#define BSPLASH_REGDUMP_EXCLUDE(addr)	( \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_BVNF_INTR2_0_REG_START, BCHP_BVNF_INTR2_5_REG_END) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_BVNB_INTR2_REG_START, BCHP_BVNB_INTR2_REG_END) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_HIF_INTR2_REG_START, BCHP_HIF_CPU_INTR1_REG_END)   || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_XPT_BUS_IF_REG_START, BCHP_XPT_GR_REG_END)   || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVD_INTR2_0_REG_START, BCHP_AVD_INTR2_0_REG_END)   || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AUD_DSP_INTH0_REG_START, BCHP_AUD_DSP_INTH0_REG_END) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_HDMI_INTR2_REG_START, BCHP_HDMI_INTR2_REG_END) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_VIDEO_ENC_INTR2_REG_START, BCHP_VIDEO_ENC_INTR2_REG_END) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_BVNB_INTR2_REG_START, BCHP_BVNB_INTR2_REG_END) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_MEMC_GEN_0_ARC_0_CNTRL, BCHP_MEMC_GEN_0_ARC_3_VIOLATION_INFO_CMD) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_EBI_CS_BASE_0, BCHP_EBI_ECR) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_TIMER_TIMER_IS, BCHP_TIMER_WDCTRL) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_GIO_ODEN_LO, BCHP_GIO_STAT_EXT) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_IRQ0_IRQEN, BCHP_IRQ0_IRQEN) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_SCIRQ0_SCIRQEN, BCHP_SCIRQ0_SCIRQEN) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_TIMER_TIMER_IE0, BCHP_TIMER_TIMER_IE0) || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AIO_MISC_REG_START, BCHP_AUD_DSP_SEC0_REG_END)   || \
		BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_RDC_REG_START, BCHP_RDC_REG_END)\
		)


#define SPLASH_NUM_SVIDEO_OUTPUTS	1
#define SPLASH_NUM_COMPOSITE_OUTPUTS	1
#define SPLASH_NUM_COMPONENT_OUTPUTS	1

#define BRCM_DAC_COMPOSITE_0      BVDC_Dac_0
#define BRCM_DAC_COMPOSITE_1      BVDC_Dac_6

#define BRCM_DAC_Y                BVDC_Dac_5
#define BRCM_DAC_PR               BVDC_Dac_3
#define BRCM_DAC_PB               BVDC_Dac_4



#define BRCM_DAC_SVIDEO_LUMA      BVDC_Dac_1
#define BRCM_DAC_SVIDEO_CHROMA    BVDC_Dac_2





/* The default display for this board */
#ifdef CFG_SPLASH_PAL
#define SPLASH_CMP_0_PATH_DISPLAY   BFMT_VideoFmt_e576p_50Hz	
#define SPLASH_CMP_1_PATH_DISPLAY	BFMT_VideoFmt_ePAL_I
#else
#define SPLASH_CMP_0_PATH_DISPLAY	BFMT_VideoFmt_e480p 
#define SPLASH_CMP_1_PATH_DISPLAY	BFMT_VideoFmt_eNTSC
#endif

#define SPLASH_SURFACE_PXL_TYPE BPXL_eR5_G6_B5 /* BPXL_eA8_R8_G8_B8 */

#define B_I2C_CHANNEL_HDMI    0

/* End of File */
