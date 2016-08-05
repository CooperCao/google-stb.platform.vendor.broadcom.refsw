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
***************************************************************************/
#ifndef NEXUS_PLATFORM_FEATURES_H__
#define NEXUS_PLATFORM_FEATURES_H__

#include "nexus_platform_generic_features_priv.h"

/*  NEXUS_PLATFORM_MEMC0_ONLY */

/* 7439 Single memory controller configuration starts here
   This is useful to do emulation for other single memory controller chips */

/* Transport Features */

#define NEXUS_NUM_PARSER_BANDS 24
#define NEXUS_NUM_PID_CHANNELS 768
#define NEXUS_NUM_VCXOS 2
#define NEXUS_NUM_MESSAGE_FILTERS 128
#define NEXUS_NUM_PLAYPUMPS 32


#define NEXUS_NUM_VIDEO_DECODERS 4         /* 2 each for HEVD */

/* Audio Features */
#define NEXUS_NUM_AUDIO_DECODERS 6
#define NEXUS_NUM_AUDIO_INPUT_CAPTURES 1 /* Number of external inputs active at a time */
#define NEXUS_NUM_AUDIO_DACS 1
#define NEXUS_NUM_SPDIF_INPUTS 0
#define NEXUS_NUM_SPDIF_OUTPUTS 1
#define NEXUS_NUM_AUDIO_MIXERS 8
#define NEXUS_NUM_AUDIO_DUMMY_OUTPUTS 2

#define NEXUS_NUM_I2S_INPUTS 1
#define NEXUS_NUM_I2S_OUTPUTS 1
#define NEXUS_NUM_AUDIO_PLAYBACKS 3

#define NEXUS_HAS_AUDIO_MUX_OUTPUT 1

/* Display Features */
#define NEXUS_NUM_656_OUTPUTS 0

/* DAC features  */
#define NEXUS_NUM_COMPONENT_OUTPUTS 1
#define NEXUS_NUM_COMPOSITE_OUTPUTS 1
#define NEXUS_NUM_SVIDEO_OUTPUTS 0

/* Transcode Proprerties */
#define NEXUS_NUM_VCE_DEVICES    1
#define NEXUS_NUM_VCE_CHANNELS   2 /* per device */
#define NEXUS_NUM_VIDEO_ENCODERS   (NEXUS_NUM_VCE_CHANNELS * NEXUS_NUM_VCE_DEVICES)

/* Max number of displays and windows supported
by this platform. Actual numbers may vary depeding
upon the chip usage. See below */
#define NEXUS_NUM_DISPLAYS   4 /* C0/C1 for local HD/SD simul,C2,C3 for dual transcodes on >= B0*/
#define NEXUS_NUM_VIDEO_WINDOWS   2 /* per display */

/* Cable Frontend */
/* The maximum number of input bands on 7425 are 9. Also 3128 has 8 DS + 1 OB */
#ifdef NEXUS_FRONTEND_3128
#define NEXUS_3128_MAX_DOWNSTREAM_CHANNELS 8
#endif

#ifdef NEXUS_USE_FRONTEND_DAUGHTER_CARD
#define NEXUS_NUM_FRONTEND_CARD_SLOTS 1
#endif

#ifdef NEXUS_USE_7252S_GW
#define NEXUS_MAX_FRONTENDS 9
#endif

#define NEXUS_MAX_3255_ADSCHN 8
#define NEXUS_3255_OOB_TUNER_IFFREQ (1250000)/* 1.25 MHz */
#define NEXUS_SHARED_FRONTEND_INTERRUPT 1




/* I2C Channels */
#define NEXUS_NUM_I2C_CHANNELS 5

/* I2C channel usage assignments. Refer to BSC table in the board schematics. */
#define NEXUS_I2C_CHANNEL_HDMI_TX          0
#define NEXUS_I2C_CHANNEL_LNA             5  /*  */

#if defined NEXUS_USE_7449_SV || defined NEXUS_USE_7439_SV
#define NEXUS_I2C_CHANNEL_DSTRM_TUNER     3
#else
#define NEXUS_I2C_CHANNEL_DSTRM_TUNER     4  /* xytodo: BCM3128(for VMS board) */
#endif

/* SMARTCARD CHANNELS */
#define NEXUS_NUM_SMARTCARD_CHANNELS 2

/* DVB-CI Details */
#define NEXUS_DVB_CI_MEMORY_BASE (0xEFF00000)
#define NEXUS_DVB_CI_MEMORY_LENGTH (1024*1024)

/* Memory features */
/* Max Memc's on this chip! */
#define NEXUS_NUM_MEMC 2

/* default heap indices, refer to memory map document  */
#define NEXUS_MEMC0_MAIN_HEAP           0 /* Main Heap */
#define NEXUS_MEMC0_GRAPHICS_HEAP       1 /* Single large graphics heap*/
#define NEXUS_VIDEO_SECURE_HEAP         3 /* CABAC, CDB, RS and XC buffers for encoder */
#define NEXUS_MEMC0_PICTURE_BUFFER_HEAP 4 /* XVD/VDC buffers for 2 encode*/
#define NEXUS_MEMC1_PICTURE_BUFFER_HEAP 5 /* XVD/VDC buffers for 2 decode/encode*/
#define NEXUS_MEMC1_GRAPHICS_HEAP       6 /* Single large graphics heap*/
#define NEXUS_SAGE_SECURE_HEAP          8 /* SAGE HEAP - 32 MB */
#define NEXUS_MEMC1_HIGH_MEMORY_HEAP    9
#define NEXUS_MEMC0_SECURE_PICTURE_BUFFER_HEAP 10
#define NEXUS_MEMC1_SECURE_PICTURE_BUFFER_EXT_HEAP 11
#define NEXUS_MEMC1_SECURE_PICTURE_BUFFER_HEAP 12
#define NEXUS_MEMC1_SECURE_GRAPHICS_HEAP 13
#define NEXUS_EXPORT_HEAP 14


#define NEXUS_AVS_MONITOR           0

#define NEXUS_NUM_SPI_CHANNELS 4

#endif /* #ifndef NEXUS_PLATFORM_FEATURES_H__ */
