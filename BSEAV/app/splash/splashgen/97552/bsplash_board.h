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
#ifndef BSPLASH_BOARD_H__
#define BSPLASH_BOARD_H__
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
#include "bchp_rdc.h"
#include "bchp_avs_hw_mntr.h"
#include "bchp_avs_pvt_mntr_config.h"
#include "bchp_avs_asb_registers.h"
#include "bchp_avs_ro_registers_0.h"
#include "bchp_avs_ro_registers_1.h"
#include "bchp_avs_rosc_threshold_1.h"
#include "bchp_avs_rosc_threshold_2.h"



/* new vdc pull the surface adddress from the RDC scratch registers. This information can be found from the VDC team.
   Will request VDC team for an API to determine the scratch adddress location until that happens hardcoding it here */
#define BSPLASH_RDC_SCRATCH_DISP0_REG0  (BCHP_RDC_scratch_i_ARRAY_BASE + 4*42)
#define BSPLASH_RDC_SCRATCH_DISP0_REG1  (BCHP_RDC_scratch_i_ARRAY_BASE + 4*43)
#define BSPLASH_RDC_SCRATCH_DISP1_REG0  (BCHP_RDC_scratch_i_ARRAY_BASE + 4*48)
#define BSPLASH_RDC_SCRATCH_DISP1_REG1  (BCHP_RDC_scratch_i_ARRAY_BASE + 4*49)


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
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_RAAGA_DSP_INTH_REG_START, BCHP_RAAGA_DSP_INTH_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_HDMI_TX_INTR2_REG_START, BCHP_HDMI_TX_INTR2_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_VIDEO_ENC_INTR2_REG_START, BCHP_VIDEO_ENC_INTR2_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_BVNB_INTR2_REG_START, BCHP_BVNB_INTR2_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_MEMC_GEN_0_ARC_0_CNTRL, BCHP_MEMC_GEN_0_ARC_3_VIOLATION_INFO_CMD) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_EBI_CS_BASE_0, BCHP_EBI_ECR) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_TIMER_TIMER_IS, BCHP_TIMER_WDCTRL) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_GIO_ODEN_LO, BCHP_GIO_STAT_EXT) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_IRQ0_IRQEN, BCHP_IRQ0_IRQEN) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_SCIRQ0_SCIRQEN, BCHP_SCIRQ0_SCIRQEN) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_TIMER_TIMER_IE0, BCHP_TIMER_TIMER_IE0) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AIO_MISC_REG_START, BCHP_RAAGA_DSP_SEC0_REG_END)   || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_BSP_CMDBUF_REG_START, BCHP_BSP_INST_PATCHRAM_REG_END)   || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_RDC_REG_START, BCHP_RDC_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_MEMC_L2_1_0_REG_START, BCHP_MEMC_L2_1_0_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_HW_MNTR_SW_CONTROLS, BCHP_AVS_HW_MNTR_IDLE_STATE_0_CEN_ROSC_1) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL, BCHP_AVS_PVT_MNTR_CONFIG_MAX_DAC_CODE) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_ASB_REGISTERS_ASB_COMMAND, BCHP_AVS_ASB_REGISTERS_ASB_BUSY) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS, BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_47) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_RO_REGISTERS_1_POW_WDOG_FAILURE_STATUS, BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_27) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0, BCHP_AVS_ROSC_THRESHOLD_1_INTERRUPT_STATUS_THRESHOLD1_FAULTY_SENSOR) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0, BCHP_AVS_ROSC_THRESHOLD_2_INTERRUPT_STATUS_THRESHOLD2_FAULTY_SENSOR) \
        )

/* Removed  BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_BSCA_CHIP_ADDRESS, BCHP_BSCD_SCL_PARAM) || \ */

#define SPLASH_NUM_SVIDEO_OUTPUTS   0
#define SPLASH_NUM_COMPOSITE_OUTPUTS    1
#define SPLASH_NUM_COMPONENT_OUTPUTS    1

#define BRCM_DAC_SVIDEO_LUMA      BVDC_Dac_5
#define BRCM_DAC_SVIDEO_CHROMA    BVDC_Dac_6
#define BRCM_DAC_COMPOSITE_0      BVDC_Dac_0

#define BRCM_DAC_Y                BVDC_Dac_2
#define BRCM_DAC_PR               BVDC_Dac_3
#define BRCM_DAC_PB               BVDC_Dac_1

#define BRCM_DAC_COMPOSITE_1      BVDC_Dac_3

#define CFG_SPLASH_PAL


/* number of mem heap handles  */
#define SPLASH_NUM_MEM        1

/* heap for RUL bufs */
#define SPLASH_RUL_MEM        0   /* idx to BMEM_Handle array */

/* number of surface */
#define SPLASH_NUM_SURFACE    2

/* surface 0 info */
#define SPLASH_SURF0_MEM      0   /* idx to BMEM_Handle array */
#define SPLASH_SURF0_PXL_FMT  BPXL_eR5_G6_B5
#define SPLASH_SURF0_BMP      "splash.bmp"

/* surface 1 info */
#define SPLASH_SURF1_MEM      0   /* idx to BMEM_Handle array */
#define SPLASH_SURF1_PXL_FMT  BPXL_eR5_G6_B5 
#define SPLASH_SURF1_BMP      "splash.bmp"

#define SPLASH_NUM_DISPLAY    2

#ifdef  CFG_SPLASH_PAL
/* display 0 info */
#define SPLASH_DISP0_FMT      BFMT_VideoFmt_e720p_50Hz
#define SPLASH_DISP0_SUR      0   /* idx to splash surface buffer array */

/* display 1 info */
#define SPLASH_DISP1_FMT      BFMT_VideoFmt_ePAL_I 
#define SPLASH_DISP1_SUR      1   /* idx to splash surface buffer array */

#else
/* display 0 info */
#define SPLASH_DISP0_FMT      BFMT_VideoFmt_e720p
#define SPLASH_DISP0_SUR      0   /* idx to splash surface buffer array */

/* display 1 info */
#define SPLASH_DISP1_FMT      BFMT_VideoFmt_eNTSC
#define SPLASH_DISP1_SUR      1   /* idx to splash surface buffer array */
#endif

#define B_I2C_CHANNEL_HDMI    0
#endif /* #ifndef BSPLASH_BOARD_H__ */
/* End of File */



