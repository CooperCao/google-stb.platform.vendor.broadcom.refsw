/******************************************************************************
 * (c) 2006-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#ifndef BDSP_RAAGA_TYPES_H__
#define BDSP_RAAGA_TYPES_H__
#include "bdsp_types.h"
#include "bkni.h"
#include "bstd.h"



/*------------------------- RDB INCLUDE FILES --------------------------------*/
#include "bchp_common.h"
#include "bchp_int_id_raaga_dsp_inth.h"
#include "bchp_int_id_raaga_dsp_fw_inth.h"
#include "bchp_raaga_dsp_dma.h"
#include "bchp_raaga_dsp_esr_si.h"
#include "bdsp_bchp_raaga_dsp_fw_cfg.h"

#ifdef BCHP_RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO0_BASEADDR
    #define RAAGA_DSP_FW_CFG_PAGE_START BCHP_RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO0_BASEADDR
#elif defined BCHP_RAAGA_DSP_FW_CFG_SW_UNDEFINED_SECTION0_i_ARRAY_BASE
    #define RAAGA_DSP_FW_CFG_PAGE_START BCHP_RAAGA_DSP_FW_CFG_SW_UNDEFINED_SECTION0_i_ARRAY_BASE
#else
    #error "Cannot find first register in the fw_cfg page"
#endif

#include "bchp_raaga_dsp_fw_inth.h"
#include "bchp_raaga_dsp_inth.h"
#include "bchp_raaga_dsp_mem_subsystem.h"
#include "bchp_raaga_dsp_misc.h"
#include "bchp_raaga_dsp_peri_dbg_ctrl.h"
#include "bchp_raaga_dsp_peri_sw.h"
#include "bchp_raaga_dsp_rgr.h"
#include "bchp_raaga_dsp_timers.h"
#include "bchp_common.h"


/****** Defines for DSP1 ***********/
#ifdef BCHP_RAAGA_DSP_RGR_1_REG_START
#include "bchp_raaga_dsp_rgr_1.h"
#endif

#if defined BCHP_RAAGA_DSP_INTH_1_REG_START
#include "bchp_int_id_raaga_dsp_inth_1.h"
#include "bchp_int_id_raaga_dsp_fw_inth_1.h"
#endif

#ifdef BCHP_CLKGEN_REG_START

#include "bchp_clkgen.h"
#endif

#ifdef BCHP_AUD_FMM_PLL0_REG_START
#include "bchp_aud_fmm_pll0.h"
#endif
#ifdef BCHP_AUD_FMM_PLL1_REG_START
#include "bchp_aud_fmm_pll1.h"
#endif
#ifdef BCHP_AUD_FMM_PLL2_REG_START

#include "bchp_aud_fmm_pll2.h"
#endif

#ifdef BCHP_HIFIDAC_CTRL1_REG_START
#include "bchp_hifidac_ctrl1.h"
#include "bchp_hifidac_esr1.h"
#include "bchp_hifidac_rm1.h"
#endif

#ifdef BCHP_HIFIDAC_CTRL2_REG_START
#include "bchp_hifidac_ctrl2.h"
#include "bchp_hifidac_esr2.h"
#include "bchp_hifidac_rm2.h"
#endif

#ifdef BCHP_AUDIO0_PLL_REG_START
#include "bchp_audio0_pll.h"
#endif
#ifdef BCHP_AUDIO1_PLL_REG_START
#include "bchp_audio1_pll.h"
#endif
#ifdef BCHP_PLL_AUDIO0_REG_START
#include "bchp_pll_audio0.h"
#endif
#ifdef BCHP_PLL_AUDIO1_REG_START
#include "bchp_pll_audio1.h"
#endif

/* Max number of DSPs present in SOC */
#if defined BCHP_RAAGA_DSP_RGR_1_REG_START
#define BDSP_RAAGA_MAX_DSP 2
#else
#define BDSP_RAAGA_MAX_DSP 1
#endif

#ifdef BCHP_RAAGA_DSP_DMA_1_REG_START
#include "bchp_raaga_dsp_dma_1.h"
#endif


/* Max Post-processing branches & stages */

#define BDSP_RAAGA_MAX_STAGE_PER_BRANCH (7+3)/*MAX PP stage added externally + MAX internal stage added*/
                                                                               /*(Decode + SRC + DSOLA)*/
#define BDSP_RAAGA_MAX_BRANCH   3
#define BDSP_RAAGA_MAX_BRANCH_PER_TASK   BDSP_RAAGA_MAX_BRANCH


/* Max settings for video encoder on Raaga */
#define BDSP_RAAGA_MAX_BRANCH_VIDEO_ENCODE              1
#define BDSP_RAAGA_MAX_STAGE_PER_BRANCH_VIDEO_ENCODE    2   /* 1 might be needed for genCdbItb */

#define BDSP_RAAGA_MAX_BRANCH_SCM               1
#define BDSP_RAAGA_MAX_STAGE_PER_BRANCH_SCM     1
#endif
