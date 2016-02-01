/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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

#include "bstd.h"
#include "nexus_platform_generic_features_priv.h"

/* Transport Features */
#if (BCHP_CHIP==7344)
#define NEXUS_NUM_PLAYPUMPS 4
#define NEXUS_NUM_PID_CHANNELS 256
#elif ((BCHP_CHIP==7346) || (BCHP_CHIP==73465))
#define NEXUS_NUM_PLAYPUMPS 12
#define NEXUS_NUM_PID_CHANNELS 512
#else
#error "Unknown Chip"
#endif
#define NEXUS_NUM_VCXOS 1
#define NEXUS_NUM_PARSER_BANDS 10
#define NEXUS_NUM_MESSAGE_FILTERS 128

#define NEXUS_NUM_VIDEO_DECODERS 2

/* Audio Features */
#define NEXUS_NUM_AUDIO_DECODERS 3
#define NEXUS_NUM_AUDIO_DACS 1
#define NEXUS_NUM_SPDIF_OUTPUTS 1
#define NEXUS_NUM_AUDIO_MIXERS 4
#ifdef NEXUS_NUM_DSP_VIDEO_ENCODERS
#define NEXUS_NUM_AUDIO_DUMMY_OUTPUTS 2
#define NEXUS_NUM_VIDEO_ENCODERS 1
#endif

#define NEXUS_NUM_I2S_INPUTS 1
#define NEXUS_NUM_I2S_OUTPUTS 1
#define NEXUS_NUM_AUDIO_CAPTURE_CHANNELS 1
#define NEXUS_NUM_AUDIO_CAPTURES 1
#define NEXUS_NUM_AUDIO_PLAYBACKS 2

#define NEXUS_HAS_AUDIO_MUX_OUTPUT 1

/* Display Features */
#define NEXUS_NUM_656_OUTPUTS 1
#define NEXUS_NUM_COMPONENT_OUTPUTS 1
#define NEXUS_NUM_COMPOSITE_OUTPUTS 1
#define NEXUS_NUM_SVIDEO_OUTPUTS 0

/* Max number of displays and windows supported
by this platform. Actual numbers may vary depeding
upon the chip usage. See below */
/* Keep this to non hdsd for bringup */
#define NEXUS_NUM_DISPLAYS 2
#define NEXUS_NUM_VIDEO_WINDOWS   2
#if NEXUS_PLATFORM_97346_I2SFF || NEXUS_PLATFORM_97346_SHR44 || NEXUS_PLATFORM_97346_H43
#endif

/* AVS support */
#define NEXUS_AVS_MONITOR 1


/* Cable Frontend */
#if (BCHP_CHIP==7344)
#if NEXUS_PLATFORM_7418SFF_H
#define NEXUS_MAX_FRONTENDS 3
#define NEXUS_3128_MAX_DOWNSTREAM_CHANNELS 3
#else
#if NEXUS_PLATFORM_7344SV
#define NEXUS_MAX_FRONTENDS 2
#else
#define NEXUS_MAX_FRONTENDS 1
#endif
#endif
/* Internal Frontends define to 1 */
#define NEXUS_7346_MAX_FRONTEND_CHANNELS 1
#elif ((BCHP_CHIP==7346) || (BCHP_CHIP==73465))
#if defined(NEXUS_PLATFORM_97346_SV)
#if NEXUS_USE_FRONTEND_DAUGHTER_CARD
#define NEXUS_MAX_FRONTENDS 9
#define NEXUS_NUM_FRONTEND_CARD_SLOTS 1
#else
#define NEXUS_MAX_FRONTENDS 4
#endif
#elif defined(NEXUS_PLATFORM_97346_HR44)
#define NEXUS_MAX_FRONTENDS 6
#elif defined(NEXUS_PLATFORM_97346_H43)
#define NEXUS_MAX_FRONTENDS 9
#elif defined(NEXUS_PLATFORM_97346_SHR44)
#define NEXUS_MAX_FRONTENDS 12

#if USE_SPI_FRONTEND
/* SPI Channels */
#define NEXUS_NUM_SPI_CHANNELS 2
#endif

#elif NEXUS_PLATFORM_97346_I2SFF
#define NEXUS_MAX_FRONTENDS 4
#else
#define NEXUS_MAX_FRONTENDS 2
#endif
#endif


/* #define NEXUS_MAX_3255_ADSCHN 0
#define NEXUS_3255_OOB_TUNER_IFFREQ (1250000) 1.25 MHz */

#if (BCHP_CHIP==7344)
#define NEXUS_KEYPAD_AON_GPIO 10
#else
#define NEXUS_KEYPAD_AON_GPIO 25
#endif

/* I2C Channels */
#define NEXUS_NUM_I2C_CHANNELS  4

#if (BCHP_CHIP==7344)
#define NEXUS_MOCA_I2C_CHANNEL 2
#if NEXUS_PLATFORM_7418SFF_H
#define NEXUS_I2C_CHANNEL_EXT_RFM         1  /* External RFM */
#endif
#else
#define NEXUS_MOCA_I2C_CHANNEL 1
#endif





/* SMARTCARD CHANNELS */
#define NEXUS_NUM_SMARTCARD_CHANNELS 2

/* DVB-CI Details */
#define NEXUS_DVB_CI_MEMORY_BASE (0x19800000)
#define NEXUS_DVB_CI_MEMORY_LENGTH (1024*1024)
/* Memory features */
#define NEXUS_NUM_MEMC 1

/* default heap indices */
#define NEXUS_MEMC0_MAIN_HEAP           0
#define NEXUS_MEMC0_PICTURE_BUFFER_HEAP 1
#define NEXUS_MEMC0_GRAPHICS_HEAP       2

#endif /* #ifndef NEXUS_PLATFORM_FEATURES_H__ */

