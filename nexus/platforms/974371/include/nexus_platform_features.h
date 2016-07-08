/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
*  ANY LIMITED REMEDY.
*
 * Module Description:
 *
 ***************************************************************************/
#ifndef NEXUS_PLATFORM_FEATURES_H__
#define NEXUS_PLATFORM_FEATURES_H__

#include "nexus_platform_generic_features_priv.h"

/*  NEXUS_PLATFORM_MEMC0_ONLY */


/* 74371 Single memory controller configuration starts here
   This is useful to do emulation for other single memory controller chips */
/* Transport Features */
#define NEXUS_NUM_PARSER_BANDS 24
#define NEXUS_NUM_PID_CHANNELS 768
#define NEXUS_NUM_VCXOS 2
#define NEXUS_NUM_MESSAGE_FILTERS 128
#define NEXUS_NUM_PLAYPUMPS 32
#define NEXUS_NUM_VIDEO_DECODERS 2         /* 2 each for HEVD */
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
#define NEXUS_NUM_AUDIO_DUMMY_OUTPUTS 2
#define NEXUS_NUM_VIDEO_ENCODERS 1
#endif

/* Audio Features */
#define NEXUS_NUM_AUDIO_DECODERS 6
#define NEXUS_NUM_AUDIO_INPUT_CAPTURES 1 /* Number of external inputs active at a time */
#define NEXUS_NUM_AUDIO_DACS 1
#define NEXUS_NUM_SPDIF_INPUTS 0
#define NEXUS_NUM_SPDIF_OUTPUTS 1
#define NEXUS_NUM_AUDIO_MIXERS 8

#define NEXUS_NUM_I2S_INPUTS 1
#define NEXUS_NUM_I2S_OUTPUTS 1
#define NEXUS_NUM_AUDIO_PLAYBACKS 3

#define NEXUS_HAS_AUDIO_MUX_OUTPUT 1

/* Display Features */
#define NEXUS_NUM_656_OUTPUTS 0
#define NEXUS_NUM_COMPONENT_OUTPUTS 0
#if defined NEXUS_USE_74371_SV || NEXUS_USE_7437_USFF
#define NEXUS_NUM_COMPOSITE_OUTPUTS 1
#else
#define NEXUS_NUM_COMPOSITE_OUTPUTS 0
#endif
#define NEXUS_NUM_SVIDEO_OUTPUTS 0

/* Max number of displays and windows supported
by this platform. Actual numbers may vary depeding
upon the chip usage. See below */
#define NEXUS_NUM_DISPLAYS   2 /**/
#define NEXUS_NUM_VIDEO_WINDOWS   2 /* per display */

/* Satellite Frontend */
#if defined NEXUS_USE_74371_XID
#define NEXUS_MAX_FRONTENDS 8
#endif

#define NEXUS_MAX_3255_ADSCHN 8
#define NEXUS_3255_OOB_TUNER_IFFREQ (1250000)/* 1.25 MHz */
#define NEXUS_SHARED_FRONTEND_INTERRUPT 1




/* I2C Channels. Although, BSC-B shouldn't be used in 7250. Hence, there are 4 BSC cores, but only 3 are useable. */
#define NEXUS_NUM_I2C_CHANNELS 4

/* I2C channel usage assignments. Refer to BSC table in the board schematics. */
#define NEXUS_I2C_CHANNEL_HDMI_TX    0

/* SMARTCARD CHANNELS */
#define NEXUS_NUM_SMARTCARD_CHANNELS 2

/* DVB-CI Details */
#define NEXUS_DVB_CI_MEMORY_BASE (0x19800000)
#define NEXUS_DVB_CI_MEMORY_LENGTH (1024*1024)

/* Memory features */
#define NEXUS_NUM_MEMC 1

/* default heap indices, refer to memory map document  */
#define NEXUS_MEMC0_MAIN_HEAP           0 /* Main Heap */
#define NEXUS_MEMC0_GRAPHICS_HEAP       1 /* Single large graphics heap*/
#define NEXUS_VIDEO_SECURE_HEAP         3 /* CABAC, CDB, RS and XC buffers for encoder */
#define NEXUS_MEMC0_PICTURE_BUFFER_HEAP 4 /* XVD/VDC buffers for 2 decode/encode*/
#define NEXUS_SAGE_SECURE_HEAP          5 /* SAGE HEAP - 32 MB */
#define NEXUS_EXPORT_HEAP 6

#define NEXUS_PLATFORM_P_GET_FRAMEBUFFER_HEAP_INDEX 1

#define NEXUS_AVS_MONITOR           0


#endif /* #ifndef NEXUS_PLATFORM_FEATURES_H__ */
