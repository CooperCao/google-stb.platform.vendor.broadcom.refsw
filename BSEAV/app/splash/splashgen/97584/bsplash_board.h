/***************************************************************************
 * Copyright (C) 2018 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description: Application to generate RULs for splash screen. 
 *                     This is a slightly modified copy of vdc_dump.c
 *                     ported to Nucleus
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
#define BSPLASH_RDC_SCRATCH_DISP0_REG0  (BCHP_RDC_scratch_i_ARRAY_BASE + 4*46)
#define BSPLASH_RDC_SCRATCH_DISP0_REG1  (BCHP_RDC_scratch_i_ARRAY_BASE + 4*47)
#define BSPLASH_RDC_SCRATCH_DISP1_REG0  (BCHP_RDC_scratch_i_ARRAY_BASE + 4*52)
#define BSPLASH_RDC_SCRATCH_DISP1_REG1  (BCHP_RDC_scratch_i_ARRAY_BASE + 4*53)


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
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVD_INTR2_0_REG_START, BCHP_AVD_INTR2_0_REG_END)   || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_RAAGA_DSP_INTH_REG_START, BCHP_RAAGA_DSP_INTH_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_GFX_L2_REG_START, BCHP_GFX_L2_REG_END)  || \
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
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_BSP_CMDBUF_REG_START, BCHP_BSP_CONTROL_INTR2_REG_END)   || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_RDC_REG_START, BCHP_RDC_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_MEMC_L2_1_0_REG_START, BCHP_MEMC_L2_1_0_REG_END) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_HW_MNTR_SW_CONTROLS, BCHP_AVS_HW_MNTR_IDLE_STATE_0_CEN_ROSC_1) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_PVT_MNTR_CONFIG_PVT_MNTR_CTRL, BCHP_AVS_PVT_MNTR_CONFIG_MAX_DAC_CODE) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_ASB_REGISTERS_ASB_COMMAND, BCHP_AVS_ASB_REGISTERS_ASB_BUSY) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_RO_REGISTERS_0_PVT_TEMPERATURE_MNTR_STATUS, BCHP_AVS_RO_REGISTERS_0_CEN_ROSC_STATUS_47) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_RO_REGISTERS_1_POW_WDOG_FAILURE_STATUS, BCHP_AVS_RO_REGISTERS_1_RMT_ROSC_STATUS_27) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_ROSC_THRESHOLD_1_THRESHOLD1_CEN_ROSC_0, BCHP_AVS_ROSC_THRESHOLD_1_INTERRUPT_STATUS_THRESHOLD1_FAULTY_SENSOR) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AVS_ROSC_THRESHOLD_2_THRESHOLD2_CEN_ROSC_0, BCHP_AVS_ROSC_THRESHOLD_2_INTERRUPT_STATUS_THRESHOLD2_FAULTY_SENSOR) || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11)  || \
        BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_AON_CTRL_SYSTEM_DATA_RAMi_ARRAY_BASE , (BCHP_AON_CTRL_SYSTEM_DATA_RAMi_ARRAY_BASE + (15) * 4)) \
        )

/* Removed  BSPLASH_ADDRESS_IN_RANGE(addr, BCHP_BSCA_CHIP_ADDRESS, BCHP_BSCD_SCL_PARAM) || \ */
/* ---------------------------------------------------
 * Notes:
 * 1. max number supported:
 *        mem heap:   2
 *        surface:    2
 *        display:    2
 *    however, they are very easy to further extend.
 *    Just search for key word "PORT" in the directory, you would find all
 *       places where a few similar lines are needed to add,
 *    or ask Steven Yang to do it
 * 2. surface size is defined by the display format automatically
 * 3. surface pxl format supported:
 *       BPXL_eR5_G6_B5
 *       BPXL_eA8_Y8_Cb8_Cr8
 *       BPXL_eA8_R8_G8_B8
 * 4. a surface must use the correct mem heap. It is decided by the display that uses
 *       this surface. e.g. in 7435 1u4t board, display 0 must uses surface created with
 *       mem heap 0, and display 1 must uses surface created with mem heap 1.
 *    a). if a wrong mem heap is used, the display will be likely BLACK SCREEN.
 *    b). Talk to software / hardware chip leader or nexus / VDC team if you need to find 
 *       which mem heap to use for a display.
 * 5. surfaces could use diff bmp files, also share bmp file
 * 6. display can uses any surface, as long as the surface is created in the right mem
 *       heap. displays can share surface.
 * 7. display video format supported: any format, as long as your heap has enough memeory
 * 8. RUL typically uses mem heap 0, but it is not always true, 7420 chip uses mem heap 1
 * 9. see splash_cfg.h for the default value
 * 10. refer to 97435/bsplash_board.h for an example
 */

/* number of mem heap handles  */
#define SPLASH_NUM_MEM        1

/* heap for RUL bufs */
#define SPLASH_RUL_MEM        0   /* idx to BMMA_Handle array */

/* number of surface */
#define SPLASH_NUM_SURFACE    2


/* surface 0 info */
#define SPLASH_SURF0_MEM      0   /* idx to BMMA_Handle array */
#define SPLASH_SURF0_PXL_FMT  BPXL_eR5_G6_B5
#define SPLASH_SURF0_BMP      "splash.bmp"

/* surface 1 info */
#define SPLASH_SURF1_MEM      0   /* idx to BMMA_Handle array */
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
#define BRCM_DAC_COMPOSITE_0      BVDC_Dac_0

#define BRCM_DAC_Y                BVDC_Dac_2
#define BRCM_DAC_PR               BVDC_Dac_3
#define BRCM_DAC_PB               BVDC_Dac_1

#define BRCM_DAC_COMPOSITE_1      BVDC_Dac_3

/* End of File */
