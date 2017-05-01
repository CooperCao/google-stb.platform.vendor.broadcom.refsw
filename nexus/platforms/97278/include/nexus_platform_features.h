/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


#ifndef NEXUS_PLATFORM_FEATURES_H__
#define NEXUS_PLATFORM_FEATURES_H__

#include "bstd.h"

/* Transport Features */

#define NEXUS_NUM_PARSER_BANDS 24
#define NEXUS_NUM_PID_CHANNELS 768
#define NEXUS_NUM_VCXOS 2
#define NEXUS_NUM_MESSAGE_FILTERS 256
#define NEXUS_NUM_PLAYPUMPS 32


#define NEXUS_NUM_VIDEO_DECODERS 4         /* 2 each for HEVD */

/* Display Features */
#define NEXUS_NUM_656_INPUTS 0
#define NEXUS_NUM_656_OUTPUTS 1

/* DAC features  */
#define NEXUS_NUM_COMPONENT_OUTPUTS 0
#define NEXUS_NUM_COMPOSITE_OUTPUTS 0
#define NEXUS_NUM_SVIDEO_OUTPUTS 0
#define NEXUS_NUM_AUDIO_DACS 0 /* Deprecated.  Apps should read using NEXUS_AudioCapabilities(). For backward compatibility. */

/* Transcode Proprerties */
#define NEXUS_NUM_VCE_DEVICES    1
#define NEXUS_NUM_VCE_CHANNELS   2 /* per device */
#define NEXUS_NUM_VIDEO_ENCODERS   (NEXUS_NUM_VCE_CHANNELS * NEXUS_NUM_VCE_DEVICES)

/* Max number of displays and windows supported
by this platform. Actual numbers may vary depeding
upon the chip usage. See below */
#define NEXUS_NUM_DISPLAYS   4 /* C0/C1 for local HD/SD simul,C2,C3 for dual transcodes*/
#define NEXUS_NUM_VIDEO_WINDOWS   2 /* per display */

#if defined NEXUS_USE_FRONTEND_DAUGHTER_CARD
#define NEXUS_NUM_FRONTEND_CARD_SLOTS 1
#endif

/* I2C Channels */
#define NEXUS_NUM_I2C_CHANNELS 5

/* I2C channel usage assignments. Refer to BSC table in the board schematics. */
#define NEXUS_I2C_CHANNEL_HDMI_TX          0

/* SMARTCARD CHANNELS */
#define NEXUS_NUM_SMARTCARD_CHANNELS 2

/* DVB-CI Details */
#define NEXUS_DVB_CI_MEMORY_BASE (0xEFF00000)
#define NEXUS_DVB_CI_MEMORY_LENGTH (1024*1024)

/* Memory features */
/* Max Memc's on this chip! */
#define NEXUS_NUM_MEMC 2

#define NEXUS_PLATFORM_P_GET_FRAMEBUFFER_HEAP_INDEX 1

#define NEXUS_AVS_MONITOR           0

#define NEXUS_NUM_SPI_CHANNELS 4

#include "nexus_platform_generic_features_priv.h"

#endif /* #ifndef NEXUS_PLATFORM_FEATURES_H__ */
