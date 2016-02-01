/***************************************************************************
 *     Copyright (c) 2006-2010, Broadcom Corporation
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
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_CHIP_PRIV_H_
#define BAPE_CHIP_PRIV_H_

#include "bchp_common.h"
#include "bchp_aud_fmm_bf_ctrl.h"
#include "bchp_aud_fmm_dp_ctrl0.h"
#include "bchp_aud_fmm_src_ctrl0.h"
#include "bchp_aud_fmm_op_ctrl.h"

/* 7408 resources */
#define BAPE_CHIP_MAX_DECODERS (2)
#define BAPE_CHIP_MAX_PLAYBACKS (2)
#define BAPE_CHIP_MAX_INPUT_CAPTURES (0)    /* 7408 has no external inputs */

#define BAPE_CHIP_MAX_DACS (1)
#define BAPE_CHIP_MAX_I2S_OUTPUTS (0)       /* 7408 has no i2s outputs */
#define BAPE_CHIP_MAX_I2S_INPUTS  (0)       /* 7408 has no i2s inputs */
#define BAPE_CHIP_MAX_SPDIF_OUTPUTS (1)
#define BAPE_CHIP_MAX_MAI_OUTPUTS (1)
#define BAPE_CHIP_MAX_RFMODS (1)
#define BAPE_CHIP_MAX_OUTPUT_CAPTURES (BAPE_CHIP_MAX_LOOPBACKS)

#if   defined BCHP_AUD_FMM_PLL2_REG_START
#define BAPE_CHIP_MAX_PLLS (3)
#elif defined BCHP_AUD_FMM_PLL1_REG_START
#define BAPE_CHIP_MAX_PLLS (2)
#else
#define BAPE_CHIP_MAX_PLLS (1)
#endif

#define BAPE_CHIP_MAX_SOURCE_CHANNELS (BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_END+1)

#ifdef BCHP_AUD_FMM_BF_CTRL_DESTCH_CFG0
#define BAPE_CHIP_MAX_DESTINATION_CHANNELS (1)  /* Required because the RDB defs change with only one channel */
#else
#define BAPE_CHIP_MAX_DESTINATION_CHANNELS (BCHP_AUD_FMM_BF_CTRL_DESTCH_CFGi_ARRAY_END+1)
#endif
#define BAPE_CHIP_MAX_SRCS (BCHP_AUD_FMM_SRC_CTRL0_STRM_CFGi_ARRAY_END+1)
#if BCHP_CHIP == 7408
#define BAPE_CHIP_MAX_MIXERS (6)                /* On the 7408, this is 6, others have more -- This must match the HW for coefficient loading. */
#define BAPE_CHIP_MAX_MIXER_INPUTS (4)          /* On the 7408, only four inputs are supported per mixer.  Most other chips is 8. */
#else
#define BAPE_CHIP_MAX_MIXERS (8)
#define BAPE_CHIP_MAX_MIXER_INPUTS (8)          /* On the 7408, only four inputs are supported per mixer.  Most other chips is 8. */
#endif
#define BAPE_CHIP_MAX_DUMMYSINKS (4)
#if BCHP_CHIP == 7408
#define BAPE_CHIP_MAX_LOOPBACKS (1)             /* On the 7408, this is 1, others have more */
#else
#define BAPE_CHIP_MAX_LOOPBACKS (4)
#endif
#define BAPE_CHIP_MAX_FS (4)
#define BAPE_CHIP_MAX_MIXER_PLAYBACKS (1+BCHP_AUD_FMM_DP_CTRL0_PB_FCI_IDi_ARRAY_END-BCHP_AUD_FMM_DP_CTRL0_PB_FCI_IDi_ARRAY_START)
#define BAPE_CHIP_MAX_OUTPUT_PORTS (BCHP_SHIFT(AUD_FMM_OP_CTRL_ENABLE_STATUS, reserved0))    /* Total output ports  */

#ifdef BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_7_CFG
#define BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS (8)                  /* New chips provide 8 controllers */
#else
#define BAPE_CHIP_MAX_ADAPTRATE_CONTROLLERS (4)                  /* Legacy chips only provided 4 controllers */
#endif

#define BAPE_CHIP_DEFAULT_NUM_COMPRESSED_BUFFERS (2)    /* For AC3+ passthrough we need one for SPDIF out (AC3) and one for HDMI (AC3+) */
#define BAPE_CHIP_DEFAULT_NUM_PCM_BUFFERS (4)           /* 5.1 + one stereo */
#define BAPE_CHIP_MAX_PATH_DELAY (128)
#define BAPE_CHIP_BYTES_PER_PCM_SAMPLE_PAIR (4)
#define BAPE_CHIP_BYTES_PER_COMPRESSED_SAMPLE (2)


#endif /* #ifndef BAPE_CHIP_PRIV_H_ */

