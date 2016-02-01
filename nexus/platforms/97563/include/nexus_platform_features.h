/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
***************************************************************************/
#ifndef NEXUS_PLATFORM_FEATURES_H__
#define NEXUS_PLATFORM_FEATURES_H__

#include "nexus_platform_generic_features_priv.h"

/* Transport Features */
#if defined(NEXUS_PLATFORM_7563_DGL)
#define NEXUS_NUM_PARSER_BANDS 1
#define NEXUS_NUM_PID_CHANNELS 256
#define NEXUS_NUM_VCXOS 1
#define NEXUS_NUM_MESSAGE_FILTERS 128
#define NEXUS_NUM_PLAYPUMPS 1
#else
#define NEXUS_NUM_PARSER_BANDS 10
#define NEXUS_NUM_PID_CHANNELS 256
#define NEXUS_NUM_VCXOS 1
#define NEXUS_NUM_MESSAGE_FILTERS 128
#define NEXUS_NUM_PLAYPUMPS 4
#endif

#define NEXUS_NUM_VIDEO_DECODERS 1

/* TBD: Audio Features */
#define NEXUS_NUM_AUDIO_DECODERS 3
#define NEXUS_NUM_AUDIO_MIXERS 8
#define NEXUS_NUM_I2S_OUTPUTS 1

#ifdef NEXUS_PLATFORM_7563_DGL
#define NEXUS_NUM_AUDIO_DACS 0
#define NEXUS_NUM_SPDIF_OUTPUTS 0
#else
#define NEXUS_NUM_AUDIO_DACS 1
#define NEXUS_NUM_SPDIF_OUTPUTS 1
#endif

#define NEXUS_NUM_I2S_INPUTS 0
#define NEXUS_NUM_AUDIO_CAPTURE_CHANNELS 1
#define NEXUS_NUM_AUDIO_CAPTURES 1
#define NEXUS_NUM_AUDIO_PLAYBACKS 3

/* Display Features */
#define NEXUS_NUM_656_OUTPUTS 0
#define NEXUS_NUM_COMPONENT_OUTPUTS 0
#define NEXUS_NUM_SVIDEO_OUTPUTS 0

#ifdef NEXUS_PLATFORM_7563_DGL
#define NEXUS_NUM_COMPOSITE_OUTPUTS 0
#else
#define NEXUS_NUM_COMPOSITE_OUTPUTS 1
#endif

/* Max number of displays and windows supported
by this platform. Actual numbers may vary depeding
upon the chip usage. See below */
#ifdef NEXUS_PLATFORM_7563_DGL
#define NEXUS_NUM_DISPLAYS 1
#else
#define NEXUS_NUM_DISPLAYS 2
#endif
#define NEXUS_NUM_VIDEO_WINDOWS   1


/* Frontend */
#define NEXUS_NUM_FRONTEND_CARD_SLOTS 0
#define NEXUS_MAX_FRONTENDS 2

#define NEXUS_ENABLE_EXTERNAL_LNA 0



/* I2C Channels */
#define NEXUS_NUM_I2C_CHANNELS 3

/* I2C channel usage assignments. Refer to BSC table in the board schematics. */
#define NEXUS_I2C_CHANNEL_HDMI_TX         0

#if defined(NEXUS_PLATFORM_7563_USFF2L)
#define NEXUS_HAS_EXTERNAL_RFM            1
#define NEXUS_I2C_CHANNEL_EXT_RFM         2
#endif

/* SPI Channels */
#define NEXUS_NUM_SPI_CHANNELS 2





/* SMARTCARD CHANNELS */
#ifdef NEXUS_PLATFORM_7563_DGL
#define NEXUS_NUM_SMARTCARD_CHANNELS 0
#else
#define NEXUS_NUM_SMARTCARD_CHANNELS 2
#endif

/* Memory features */
#define NEXUS_NUM_MEMC 1


/* enable AVS */
#define NEXUS_AVS_MONITOR 1

#define NEXUS_MEMC0_MAIN_HEAP           0
#define NEXUS_MEMC0_PICTURE_BUFFER_HEAP 1
#define NEXUS_MEMC0_GRAPHICS_HEAP       2
#define NEXUS_VIDEO_SECURE_HEAP         3

#define NEXUS_PLATFORM_DEFAULT_HEAP (0)
#define NEXUS_PLATFORM_P_GET_FRAMEBUFFER_HEAP_INDEX 1

#endif /* #ifndef NEXUS_PLATFORM_FEATURES_H__ */

