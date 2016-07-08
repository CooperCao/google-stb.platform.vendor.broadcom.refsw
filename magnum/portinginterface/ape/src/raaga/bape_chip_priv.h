/******************************************************************************
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
 *****************************************************************************/

#ifndef BAPE_CHIP_PRIV_H_
#define BAPE_CHIP_PRIV_H_

#include "bchp_common.h"
#include "bchp_aud_fmm_bf_ctrl.h"
#include "bchp_aud_fmm_dp_ctrl0.h"
#include "bchp_aud_fmm_src_ctrl0.h"
#if defined BCHP_AUD_FMM_OP_CTRL_REG_START
/* Older-style RDB only */
#include "bchp_aud_fmm_op_ctrl.h"
#include "bchp_aud_fmm_iop_ctrl.h"
#endif
#if defined BCHP_AUD_FMM_OP_MCLKGEN_REG_START
    #include "bchp_aud_fmm_op_mclkgen.h"
#endif /* defined BCHP_AUD_FMM_OP_MCLKGEN_REG_START */

#ifdef BCHP_AIO_MISC_REG_START
#include "bchp_aio_misc.h"
#include "bchp_aud_fmm_misc.h"
#else
#include "bchp_aud_misc.h"
#endif

#ifdef BCHP_AUD_FMM_IOP_MISC_REG_START
#include "bchp_aud_fmm_iop_misc.h"
#endif

#if defined BCHP_CLKGEN_REG_START
#include "bchp_clkgen.h"
#endif

#define BAPE_CHIP_MAX_PLAYBACKS (BAPE_CHIP_MAX_SFIFOS)
#define BAPE_CHIP_MAX_INPUT_CAPTURES (BAPE_CHIP_MAX_DFIFOS)

#if BAPE_DSP_SUPPORT
#define BAPE_CHIP_MAX_DECODERS (BAPE_CHIP_MAX_SFIFOS)
#define BAPE_CHIP_MAX_DSP_MIXERS (1)                            /* Allow 1 DSP mixer */
#define BAPE_CHIP_MAX_DSP_TASKS (3+BAPE_CHIP_MAX_DSP_MIXERS)    /* 3 decoders + 1 FW Mixer */
#if BDSP_ARM_AUDIO_SUPPORT
#define BAPE_CHIP_MAX_ARM_TASKS (1)    /* 1 encoder */
#else
#define BAPE_CHIP_MAX_ARM_TASKS (0)
#endif
#else
#define BAPE_CHIP_MAX_DSP_TASKS (0)
#define BAPE_CHIP_MAX_ARM_TASKS (0)
#endif


#define BAPE_CHIP_DSP_FIRST     (1)
#define BAPE_CHIP_DSP_LAST      (5)
#define BAPE_CHIP_ARM_FIRST     (6)
#define BAPE_CHIP_ARM_LAST      (10)


/* Max STCs */
#ifdef BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_END
/* Legacy RDB */
#define BAPE_CHIP_MAX_STCS (BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_END+1)
#ifdef BCHP_AUD_FMM_MISC_STC_LOWERi_ARRAY_BASE
#define BAPE_CHIP_HAS_42BIT_STC (1)
#define BAPE_CHIP_GET_STC_ADDRESS(idx) (BCHP_AUD_FMM_MISC_STC_LOWERi_ARRAY_BASE + ((BCHP_AUD_FMM_MISC_STC_LOWERi_ARRAY_ELEMENT_SIZE/8)*(idx)))
#define BAPE_CHIP_GET_STC_UPPER_ADDRESS(idx) (BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_BASE + ((BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_ELEMENT_SIZE/8)*(idx)))
#else
#define BAPE_CHIP_GET_STC_ADDRESS(idx) (BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_BASE + ((BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_ELEMENT_SIZE/8)*(idx)))
#endif
#else
/* 7429 style RDB */
#define BAPE_CHIP_MAX_STCS (BCHP_AUD_MISC_STC_UPPERi_ARRAY_END+1)

#ifdef BCHP_AUD_MISC_STC_LOWERi_ARRAY_BASE
#define BAPE_CHIP_HAS_42BIT_STC (1)
#define BAPE_CHIP_GET_STC_ADDRESS(idx) (BCHP_AUD_MISC_STC_LOWERi_ARRAY_BASE + ((BCHP_AUD_MISC_STC_LOWERi_ARRAY_ELEMENT_SIZE/8)*(idx)))
#define BAPE_CHIP_GET_STC_UPPER_ADDRESS(idx) (BCHP_AUD_MISC_STC_UPPERi_ARRAY_BASE + ((BCHP_AUD_MISC_STC_UPPERi_ARRAY_ELEMENT_SIZE/8)*(idx)))
#else
#define BAPE_CHIP_GET_STC_ADDRESS(idx) (BCHP_AUD_MISC_STC_UPPERi_ARRAY_BASE + ((BCHP_AUD_MISC_STC_UPPERi_ARRAY_ELEMENT_SIZE/8)*(idx)))
#endif
#endif

/* Check for DACs */
#if defined BCHP_HIFIDAC_CTRL2_REG_START || defined BCHP_HIFIDAC_CTRL_2_REG_START
#define BAPE_CHIP_MAX_DACS (3)
#elif defined BCHP_HIFIDAC_CTRL1_REG_START || defined BCHP_HIFIDAC_CTRL_1_REG_START
#define BAPE_CHIP_MAX_DACS (2)
#elif defined BCHP_HIFIDAC_CTRL0_REG_START || defined BCHP_HIFIDAC_CTRL_0_REG_START
#define BAPE_CHIP_MAX_DACS (1)
#endif

/* Check for the number of I2S outputs... */
#if defined BCHP_AUD_FMM_OP_CTRL_I2SS1_CFG || defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_1_REG_START || defined BCHP_AUD_FMM_IOP_OUT_I2S_1_REG_START
#define BAPE_CHIP_MAX_I2S_OUTPUTS (2)
#elif defined BCHP_AUD_FMM_OP_CTRL_I2SS0_CFG || defined BCHP_AUD_FMM_IOP_OUT_I2S_STEREO_0_REG_START || defined BCHP_AUD_FMM_IOP_OUT_I2S_0_REG_START
#define BAPE_CHIP_MAX_I2S_OUTPUTS (1)
/* There must not be any I2S outputs */
#else
#define BAPE_CHIP_MAX_I2S_OUTPUTS (0)
#endif

/* Check for I2S Multi Outputs */
#define BAPE_CHIP_MAX_I2S_MULTI_OUTPUTS (0)

/* Check for I2S Inputs */
#if defined BCHP_AUD_FMM_IOP_CTRL_I2SIN_CFG1 || defined BCHP_AUD_FMM_IOP_IN_I2S_STEREO_1_REG_START || defined BCHP_AUD_FMM_IOP_IN_I2S_1_REG_START
#define BAPE_CHIP_MAX_I2S_INPUTS (2)
#elif defined BCHP_AUD_FMM_IOP_CTRL_I2SIN_CFG0 || defined BCHP_AUD_FMM_IOP_IN_I2S_STEREO_0_REG_START || defined BCHP_AUD_FMM_IOP_IN_I2S_0_REG_START
#define BAPE_CHIP_MAX_I2S_INPUTS (1)
#else
#define BAPE_CHIP_MAX_I2S_INPUTS (0)
#endif

/* MAI Inputs */
#if defined BCHP_HDMI_RCVR_CTRL_REG_START
#include "bchp_hdmi_rcvr_ctrl.h"
#include "bchp_spdif_rcvr_ctrl.h"
    #define BAPE_CHIP_MAX_MAI_INPUTS (1)
    #define BAPE_CHIP_MAI_INPUT_TYPE_IS_LEGACY 1     /* 1=>true: This is a Legacy (7422/7425) type of MAI/HDMI input */
    #if defined BCHP_SPDIF_RCVR_CTRL_CONFIG_HBR_MODE_MASK
        #define BAPE_CHIP_MAI_INPUT_HBR_SUPPORT 1    /* 7435 added support for HBR input */
    #endif
#elif defined BCHP_AUD_FMM_IOP_IN_HDMI_0_REG_START
    #define BAPE_CHIP_MAX_MAI_INPUTS (1)
    #define BAPE_CHIP_MAI_INPUT_TYPE_IS_IOPIN 1      /* 1=>true: This is a newer (IOP IN) (7429) type of MAI/HDMI input */
    #define BAPE_CHIP_MAI_INPUT_HBR_SUPPORT 1        /* New chips with HDMI input support HBR */
#endif

/* SPDIF Inputs */
#if defined BCHP_SPDIF_RCVR_CTRL_REG_START
    #define BAPE_CHIP_MAX_SPDIF_INPUTS (1)
    #define BAPE_CHIP_SPDIF_INPUT_TYPE_IS_LEGACY 1     /* 1=>true: This is a Legacy (7422/7425) type of SPDIF input */
#elif defined BCHP_AUD_FMM_IOP_IN_SPDIF_0_REG_START
    #define BAPE_CHIP_MAX_SPDIF_INPUTS (1)
    #define BAPE_CHIP_SPDIF_INPUT_TYPE_IS_IOPIN 1      /* 1=>true: This is a newer (IOP IN) (7429) type of SPDIF input */
#endif

#if defined BCHP_RFM_SYSCLK_REG_START
#define BAPE_CHIP_MAX_RFMODS (1)
#else
#define BAPE_CHIP_MAX_RFMODS (0)
#endif

#if defined BCHP_AUD_MISC_CRC_CFGi_ARRAY_BASE
#define BAPE_CHIP_MAX_CRCS (BCHP_AUD_MISC_CRC_CFGi_ARRAY_END - BCHP_AUD_MISC_CRC_CFGi_ARRAY_START + 1)
#else
#define BAPE_CHIP_MAX_CRCS (0)
#endif

#if defined BCHP_AUD_FMM_IOP_OUT_SPDIF_1_REG_START
#define BAPE_CHIP_MAX_SPDIF_OUTPUTS (2)
#elif defined BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 || defined BCHP_AUD_FMM_IOP_OUT_SPDIF_0_REG_START
#define BAPE_CHIP_MAX_SPDIF_OUTPUTS (1)
#endif

#if defined BCHP_AUD_FMM_IOP_OUT_MAI_1_REG_START
#define BAPE_CHIP_MAX_MAI_OUTPUTS (2)
#elif defined BCHP_AUD_FMM_OP_CTRL_MAI_FORMAT || defined BCHP_AUD_FMM_IOP_OUT_MAI_0_REG_START
#define BAPE_CHIP_MAX_MAI_OUTPUTS (1)
#endif

#if defined  BCHP_AUD_FMM_MISC_SEROUT_SEL_SPDIF_OUT_1_ENABLE_MASK || defined BCHP_AUD_MISC_SEROUT_SEL_HDMI_RX_ARC_ENABLE_MASK
#define BAPE_CHIP_MAX_AUDIO_RETURN_CHANNELS (1)
#endif

#define BAPE_CHIP_MAX_OUTPUT_CAPTURES (BAPE_CHIP_MAX_LOOPBACKS)

/* PLL Details */
#if   defined BCHP_AUD_FMM_PLL2_REG_START || defined BCHP_AUD_FMM_IOP_PLL_2_REG_START
#define BAPE_CHIP_MAX_PLLS (3)
#elif defined BCHP_AUD_FMM_PLL1_REG_START || defined BCHP_AUD_FMM_IOP_PLL_1_REG_START
#define BAPE_CHIP_MAX_PLLS (2)
#elif defined BCHP_AUD_FMM_PLL0_REG_START || defined BCHP_AUD_FMM_IOP_PLL_0_REG_START
#define BAPE_CHIP_MAX_PLLS (1)
#else
/* No PLLs */
#define BAPE_CHIP_MAX_PLLS (0)
#endif

/* NCO Details */
#if   defined BCHP_AUD_FMM_OP_MCLKGEN_MCLK_GEN_6_CONTROL || defined BCHP_AUD_FMM_IOP_NCO_6_REG_START
#define BAPE_CHIP_MAX_NCOS (7)
#elif defined BCHP_AUD_FMM_OP_MCLKGEN_MCLK_GEN_5_CONTROL || defined BCHP_AUD_FMM_IOP_NCO_5_REG_START
#define BAPE_CHIP_MAX_NCOS (6)
#elif defined BCHP_AUD_FMM_OP_MCLKGEN_MCLK_GEN_4_CONTROL || defined BCHP_AUD_FMM_IOP_NCO_4_REG_START
#define BAPE_CHIP_MAX_NCOS (5)
#elif defined BCHP_AUD_FMM_OP_MCLKGEN_MCLK_GEN_3_CONTROL || defined BCHP_AUD_FMM_IOP_NCO_3_REG_START
#define BAPE_CHIP_MAX_NCOS (4)
#elif defined BCHP_AUD_FMM_OP_MCLKGEN_MCLK_GEN_2_CONTROL || defined BCHP_AUD_FMM_IOP_NCO_2_REG_START
#define BAPE_CHIP_MAX_NCOS (3)
#elif defined BCHP_AUD_FMM_OP_MCLKGEN_MCLK_GEN_1_CONTROL || defined BCHP_AUD_FMM_IOP_NCO_1_REG_START
#define BAPE_CHIP_MAX_NCOS (2)
#elif defined BCHP_AUD_FMM_OP_MCLKGEN_MCLK_GEN_0_CONTROL || defined BCHP_AUD_FMM_IOP_NCO_0_REG_START
#define BAPE_CHIP_MAX_NCOS (1)
#else
/* No NCOs */
#endif

#define BAPE_BASE_PLL_TO_FS_RATIO   128     /* PLL channel 0 runs at 128 * "base" Fs */

/* SFIFO Details */
#define BAPE_CHIP_MAX_SFIFOS (BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_END+1)

/* DFIFO Details */
#ifdef BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0
#define BAPE_CHIP_MAX_DFIFOS (1)  /* Required because the RDB defs change with only one channel */
#else
#define BAPE_CHIP_MAX_DFIFOS (BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_END+1)
#endif
#if defined(BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0_CAPTURE_MODE_SHIFT) || defined(BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_CAPTURE_MODE_SHIFT)
#define BAPE_CHIP_DFIFO_SUPPORTS_16BIT_CAPTURE 1
#endif

/* SRC Details */
#define BAPE_CHIP_MAX_SRCS (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_END+1)

#if defined BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_BASE
    #define BAPE_CHIP_SRC_TYPE_IS_IIR 1     /* 1=>true: This is an IIR-type of SRC */
    #if BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_END == 487
        /* SRC Coefficient Memory */
        #define BAPE_CHIP_P_SRC_IIR_CHUNK_BASE          0
        #define BAPE_CHIP_P_MAX_SRC_IIR_CHUNKS          11
        #define BAPE_CHIP_P_SRC_IIR_CHUNK_END           (BAPE_CHIP_P_SRC_IIR_CHUNK_BASE + BAPE_CHIP_P_MAX_SRC_IIR_CHUNKS - 1)
        #define BAPE_CHIP_P_MAX_IIR_FILTERS_PER_SRC     8
        #define BAPE_CHIP_P_MAX_IIR_COEFF_PER_SRC       (5 * BAPE_CHIP_P_MAX_IIR_FILTERS_PER_SRC)
        #define BAPE_CHIP_P_MAX_SRC_IIR_COEFF_PER_CHUNK BAPE_CHIP_P_MAX_IIR_COEFF_PER_SRC
        #define BAPE_CHIP_P_TOTAL_SRC_IIR_COEFF         (BAPE_CHIP_P_MAX_SRC_IIR_CHUNKS * BAPE_CHIP_P_MAX_SRC_IIR_COEFF_PER_CHUNK)

        #define BAPE_CHIP_P_SRC_LIN_CHUNK_BASE          (BAPE_CHIP_P_SRC_IIR_CHUNK_END + 1)
        #define BAPE_CHIP_P_MAX_SRC_LIN_CHUNKS          16
        #define BAPE_CHIP_P_SRC_LIN_CHUNK_END           (BAPE_CHIP_P_SRC_LIN_CHUNK_BASE + BAPE_CHIP_P_MAX_SRC_LIN_CHUNKS - 1)
        #define BAPE_CHIP_P_NUM_LIN_COEFF_PER_SRC       3
        #define BAPE_CHIP_P_NUM_LIN_COEFF_PER_CHUNK     BAPE_CHIP_P_NUM_LIN_COEFF_PER_SRC
        #define BAPE_CHIP_P_TOTAL_LIN_COEFF             (BAPE_CHIP_P_MAX_SRC_LIN_CHUNKS * BAPE_CHIP_P_NUM_LIN_COEFF_PER_CHUNK)
        #define BAPE_CHIP_MAX_SRC_COEFF_CHUNKS          (BAPE_CHIP_P_MAX_SRC_IIR_CHUNKS + BAPE_CHIP_P_MAX_SRC_LIN_CHUNKS)

    #elif BCHP_AUD_FMM_SRC_CTRL0_COEFFi_ARRAY_END == 231
        #define BAPE_CHIP_P_SRC_IIR_CHUNK_BASE          0
        #define BAPE_CHIP_P_MAX_SRC_IIR_CHUNKS          4
        #define BAPE_CHIP_P_SRC_IIR_CHUNK_END           (BAPE_CHIP_P_SRC_IIR_CHUNK_BASE + BAPE_CHIP_P_MAX_SRC_IIR_CHUNKS - 1)
        #define BAPE_CHIP_P_MAX_IIR_FILTERS_PER_SRC     8
        #define BAPE_CHIP_P_MAX_IIR_COEFF_PER_SRC       (5 * BAPE_CHIP_P_MAX_IIR_FILTERS_PER_SRC)
        #define BAPE_CHIP_P_MAX_SRC_IIR_COEFF_PER_CHUNK BAPE_CHIP_P_MAX_IIR_COEFF_PER_SRC
        #define BAPE_CHIP_P_TOTAL_SRC_IIR_COEFF         (BAPE_CHIP_P_MAX_SRC_IIR_CHUNKS * BAPE_CHIP_P_MAX_SRC_IIR_COEFF_PER_CHUNK)

        #define BAPE_CHIP_P_SRC_LIN_CHUNK_BASE          (BAPE_CHIP_P_SRC_IIR_CHUNK_END + 1)
        #define BAPE_CHIP_P_MAX_SRC_LIN_CHUNKS          16
        #define BAPE_CHIP_P_SRC_LIN_CHUNK_END           (BAPE_CHIP_P_SRC_LIN_CHUNK_BASE + BAPE_CHIP_P_MAX_SRC_LIN_CHUNKS - 1)
        #define BAPE_CHIP_P_NUM_LIN_COEFF_PER_SRC       3
        #define BAPE_CHIP_P_NUM_LIN_COEFF_PER_CHUNK     BAPE_CHIP_P_NUM_LIN_COEFF_PER_SRC
        #define BAPE_CHIP_P_TOTAL_LIN_COEFF             (BAPE_CHIP_P_MAX_SRC_LIN_CHUNKS * BAPE_CHIP_P_NUM_LIN_COEFF_PER_CHUNK)
        #define BAPE_CHIP_MAX_SRC_COEFF_CHUNKS          (BAPE_CHIP_P_MAX_SRC_IIR_CHUNKS + BAPE_CHIP_P_MAX_SRC_LIN_CHUNKS)
    #else
        #error unsupported number of SRC coefficients
    #endif
#elif defined BCHP_AUD_FMM_SRC_CTRL0_COEFF2X_i_ARRAY_BASE
    #define BAPE_CHIP_SRC_TYPE_IS_LEGACY 1     /* 1=>true: This is an legacy type of SRC */
#else
    #error "Unknown Sample Rate Converter type"
#endif

/* Mixer Details */
#if BCHP_CHIP == 7408
#define BAPE_CHIP_MAX_MIXERS (6)                /* On the 7408, this is 6, others have more -- This must match the HW for coefficient loading. */
#define BAPE_CHIP_MAX_MIXER_INPUTS (4)          /* On the 7408, only four inputs are supported per mixer.  Most other chips is 8. */
#else
#if defined BCHP_AUD_FMM_DP_CTRL0_MIXER_CONFIGi_ARRAY_END
/* 7429-style chips define both of these using RDB arrays */
#define BAPE_CHIP_MAX_MIXERS (BCHP_AUD_FMM_DP_CTRL0_MIXER_CONFIGi_ARRAY_END+1)
#define BAPE_CHIP_MAX_MIXER_INPUTS (BCHP_AUD_FMM_DP_CTRL0_MIXER0_INPUT_CONFIGi_ARRAY_END+1)
#elif defined BCHP_AUD_FMM_DP_CTRL0_MIXER11_CONFIG
/* 7425-style mixer block */
#define BAPE_CHIP_MAX_MIXERS (12)
#define BAPE_CHIP_MAX_MIXER_INPUTS (8)
#else
/* Legacy 8-mixer block */
#define BAPE_CHIP_MAX_MIXERS (8)
#define BAPE_CHIP_MAX_MIXER_INPUTS (8)
#endif
#endif
#define BAPE_CHIP_MAX_MIXER_OUTPUTS (2)
#define BAPE_CHIP_MAX_MIXER_PLAYBACKS (1+BCHP_AUD_FMM_DP_CTRL0_PB_FCI_IDi_ARRAY_END-BCHP_AUD_FMM_DP_CTRL0_PB_FCI_IDi_ARRAY_START)

/* Dummysinks */
#ifdef BCHP_AUD_FMM_IOP_DUMMYSINK_0_REG_START
#include "bchp_aud_fmm_iop_dummysink_0.h"
#define BAPE_CHIP_MAX_DUMMYSINKS (BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_ARRAY_END+1)
#else
#define BAPE_CHIP_MAX_DUMMYSINKS (4)
#endif

/* Loopbacks */
#if BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG0
#define BAPE_CHIP_MAX_LOOPBACKS (1)             /* On the 7408, this is 1, others have more */
#elif defined BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFG
#define BAPE_CHIP_MAX_LOOPBACKS (4)
#elif defined BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFGi_ARRAY_END
#define BAPE_CHIP_MAX_LOOPBACKS (BCHP_AUD_FMM_IOP_CTRL_LOOPBACK_CFGi_ARRAY_END+1)
#elif defined BCHP_AUD_FMM_IOP_LOOPBACK_0_REG_START
#include "bchp_aud_fmm_iop_loopback_0.h"
#define BAPE_CHIP_MAX_LOOPBACKS (BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_ARRAY_END+1)
#endif

#ifdef BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_ARRAY_END
/* FS Timing Sources */
#define BAPE_CHIP_MAX_FS (BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_ARRAY_END+1)
#endif

/* External MCLK Outputs */
#if defined BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_EXTi_ARRAY_END
#define BAPE_CHIP_MAX_EXT_MCLKS (BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_EXTi_ARRAY_END+1)
#elif defined BCHP_AUD_FMM_IOP_MISC_MCLK_CFG_i_ARRAY_END
#define BAPE_CHIP_MAX_EXT_MCLKS (BCHP_AUD_FMM_IOP_MISC_MCLK_CFG_i_ARRAY_END+1)
#endif

#define BAPE_CHIP_MAX_OUTPUT_PORTS (BCHP_SHIFT(AUD_FMM_OP_CTRL_ENABLE_STATUS, reserved0))    /* Total output ports  */

#if defined BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_CFGi_ARRAY_END
#define BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS (BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_CFGi_ARRAY_END+1)
#elif defined BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_7_CFG
#define BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS (8)                  /* New chips provide 8 controllers */
#else
#define BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS (4)                  /* Legacy chips only provided 4 controllers */
#endif

#ifdef BCHP_AUD_FMM_IOP_CTRL_REG_START
#define BAPE_CHIP_MAX_IOP_STREAMS (BAPE_CHIP_MAX_OUTPUT_PORTS + BAPE_CHIP_MAX_LOOPBACKS + BAPE_CHIP_MAX_DUMMYSINKS)
#else
/* The "stream" concept went away on newer chips like 7429 */
#define BAPE_CHIP_MAX_IOP_STREAMS 0
#endif

#if 0/*def BCHP_AUD_MISC_FCI_SPLTR0_CTRLi_ARRAY_START*/
    #define BAPE_CHIP_MAX_FCI_SPLITTERS (BCHP_AUD_MISC_FCI_SPLTR0_CTRLi_ARRAY_END - BCHP_AUD_MISC_FCI_SPLTR0_CTRLi_ARRAY_START + 1)
    #if defined BCHP_AUD_MISC_FCI_SPLTR0_OUT_CFG_OUT_SEL_7_MASK
        #define BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS (8)
    #else
        #warning Fix FCI Splitter Count for this chip!
    #endif
    #define BAPE_INPUT_CAPTURE_REQUIRES_SFIFO   (0)
#else
    #define BAPE_CHIP_MAX_FCI_SPLITTERS         (0)
    #define BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS  (0)
    #define BAPE_INPUT_CAPTURE_REQUIRES_SFIFO   (1)
#endif


#if BAPE_DSP_SUPPORT
    #if BAPE_CHIP_MAX_MAI_OUTPUTS > 0
        #if defined BDSP_MLP_PASSTHROUGH_SUPPORT || defined BDSP_DTSHD_PASSTHRU_SUPPORT || BAPE_CHIP_MAI_INPUT_HBR_SUPPORT
            #define BAPE_CHIP_DEFAULT_NUM_COMPRESSED_16X_BUFFERS (1)    /* One HBR buffer */
            #define BAPE_CHIP_DEFAULT_NUM_COMPRESSED_4X_BUFFERS (0)
        #elif defined BDSP_MS12_SUPPORT || BDSP_MS11_SUPPORT || BDSP_MS10_SUPPORT || defined BDSP_DDP_PASSTHRU_SUPPORT
            #define BAPE_CHIP_DEFAULT_NUM_COMPRESSED_16X_BUFFERS (0)
            #define BAPE_CHIP_DEFAULT_NUM_COMPRESSED_4X_BUFFERS (1)     /* One 4x buffer for AC3+ */
        #else
            #define BAPE_CHIP_DEFAULT_NUM_COMPRESSED_16X_BUFFERS (0)
            #define BAPE_CHIP_DEFAULT_NUM_COMPRESSED_4X_BUFFERS (0)
        #endif
    #else
        #define BAPE_CHIP_DEFAULT_NUM_COMPRESSED_16X_BUFFERS (0)
        #define BAPE_CHIP_DEFAULT_NUM_COMPRESSED_4X_BUFFERS (0)
    #endif

    #define BAPE_CHIP_DEFAULT_NUM_COMPRESSED_BUFFERS (1)    /* By default we have one compressed buffer */
    #define BAPE_CHIP_DEFAULT_NUM_PCM_BUFFERS (4)           /* 5.1 + one stereo */
#else
    #define BAPE_CHIP_DEFAULT_NUM_COMPRESSED_16X_BUFFERS (0)
    #define BAPE_CHIP_DEFAULT_NUM_COMPRESSED_4X_BUFFERS (0)
    #define BAPE_CHIP_DEFAULT_NUM_COMPRESSED_BUFFERS (0)
    #define BAPE_CHIP_DEFAULT_NUM_PCM_BUFFERS (0)
#endif

#define BAPE_CHIP_MAX_PATH_DELAY (128)
#define BAPE_CHIP_BYTES_PER_PCM_SAMPLE (4)
#define BAPE_CHIP_BYTES_PER_PCM_SAMPLE_PAIR (2*BAPE_CHIP_BYTES_PER_PCM_SAMPLE)
#define BAPE_CHIP_BYTES_PER_COMPRESSED_SAMPLE (2)
#define BAPE_CHIP_INTERLEAVE_DSP_SAMPLES (0)

#define BAPE_CHIP_MAX_SFIFO_GROUPS (BAPE_CHIP_MAX_SFIFOS)
#define BAPE_CHIP_MAX_DFIFO_GROUPS (BAPE_CHIP_MAX_DFIFOS)
#define BAPE_CHIP_MAX_SRC_GROUPS (BAPE_CHIP_MAX_SRCS)
#define BAPE_CHIP_MAX_MIXER_GROUPS (BAPE_CHIP_MAX_MIXERS)
#define BAPE_CHIP_MAX_LOOPBACK_GROUPS (BAPE_CHIP_MAX_LOOPBACKS)
#define BAPE_CHIP_MAX_DUMMYSINK_GROUPS (BAPE_CHIP_MAX_DUMMYSINKS)
#define BAPE_CHIP_MAX_FCISPLITTER_GROUPS (BAPE_CHIP_MAX_FCI_SPLITTERS)
#define BAPE_CHIP_MAX_FCISPLITTER_OUTPUT_GROUPS (BAPE_CHIP_MAX_FCI_SPLITTER_OUTPUTS)

/* And finally, here are some things that aren't in the RDB */

/* Define values for the PLL_AUDIO<n>_REFERENCE_CLOCK fields of the
 * CLKGEN_INTERNAL_MUX_SELECT register
 */
#if defined BCHP_CLKGEN_INTERNAL_MUX_SELECT
#if (BCHP_CHIP==7231 && BCHP_VER == BCHP_VER_A0)
    #define BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Fixed  1
    #define BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo0  0
#elif (BCHP_CHIP==7552 || BCHP_CHIP==7543)
    #define BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Fixed  0
    #define BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo0  1
#elif defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_AUDIO0_OSCREF_CMOS_CLOCK_SHIFT || defined BCHP_CLKGEN_PLL_AUDIO0 || defined BCHP_CLKGEN_PLL_AUDIO0_AUDIO0 || \
      ((defined BCHP_AUD_MISC_REVISION_MAJOR_DEFAULT) && (BCHP_AUD_MISC_REVISION_MAJOR_DEFAULT >= 0x00000021)) /* newer 28nm */
    #define BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo0  0
    #define BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo1  1
    #define BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo2  2
#else /* for others  (7422, 7425, 7344, 7346, 7358)  */
    #define BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Fixed  0
    #define BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo0  1
    #define BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo1  2
    #define BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo2  3
#endif

#if   defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo3
#define BAPE_CHIP_MAX_VCXOS (4)
#elif defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo2
#define BAPE_CHIP_MAX_VCXOS (3)
#elif defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo1
#define BAPE_CHIP_MAX_VCXOS (2)
#elif defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo0
#define BAPE_CHIP_MAX_VCXOS (1)
#endif
#endif

/* maximum nodes in a single path */
#define BAPE_MAX_NODES          50

/* FCI bases */
#define BAPE_FCI_BASE_SFIFO     (0x0<<6) /* 0x000 */
#define BAPE_FCI_BASE_SRC       (0x2<<6) /* 0x080 */
#define BAPE_FCI_BASE_MIXER     (0x4<<6) /* 0x100 */
#define BAPE_FCI_BASE_INPUT     (0x6<<6) /* 0x180 */
#define BAPE_FCI_BASE_FCISP     (0x7<<6) /* 0x1c0 */

#endif /* #ifndef BAPE_CHIP_PRIV_H_ */
