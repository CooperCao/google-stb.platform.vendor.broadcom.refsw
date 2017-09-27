/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/
#ifndef NEXUS_PLATFORM_GENERIC_FEATURES_H__
#define NEXUS_PLATFORM_GENERIC_FEATURES_H__

#include "bstd.h"

/* NEXUS_NUM_INPUT_BANDS is actually max input bands.
See NEXUS_TransportCapabilities.numInputBands for actual number. */
#define NEXUS_NUM_INPUT_BANDS 32

/* actual is one per AVD. only for backward compat. */
#define NEXUS_NUM_STILL_DECODES 1
/* See NEXUS_TransportCapabilities.numRecpumps for actual number. */
#define NEXUS_NUM_RECPUMPS 48
#define NEXUS_NUM_RAVE_CONTEXTS NEXUS_NUM_RECPUMPS
#define NEXUS_NUM_RAVE_CHANNELS 1
/* See NEXUS_TransportCapabilities.numStcChannels for actual number. */
#define NEXUS_NUM_STC_CHANNELS 16
/* See NEXUS_TransportCapabilities.numTimebases for actual number. */
#define NEXUS_NUM_TIMEBASES 14
/* use NEXUS_FrontendUserParameters.isMtsif, not this macro */
#define NEXUS_NUM_MTSIF 4
/* See NEXUS_TransportCapabilities.numRemux for actual number. */
#define NEXUS_NUM_REMUX 2

#define NEXUS_NUM_UARTS 3

/* undefine NEXUS_HAS_GRAPHICSV3D by compiling with V3D_SUPPORT=n */
#ifdef NEXUS_HAS_GRAPHICSV3D
#define NEXUS_NUM_3D_ENGINES 1
#else
#define NEXUS_NUM_3D_ENGINES 0
#endif

#ifdef NEXUS_HAS_UHF_INPUT
#define NEXUS_NUM_UHF_INPUTS 1
#else
#define NEXUS_NUM_UHF_INPUTS 0
#endif

#ifdef NEXUS_HAS_XPT_DMA
#define NEXUS_NUM_DMA_CHANNELS 31
#elif defined NEXUS_HAS_DMA
#if BCHP_CHIP == 7435
#define NEXUS_NUM_DMA_CHANNELS 2
#else
#define NEXUS_NUM_DMA_CHANNELS 1
#endif
#endif

#ifdef NEXUS_HAS_HDMI_INPUT
#define NEXUS_NUM_HDMI_INPUTS 1
#else
#define NEXUS_NUM_HDMI_INPUTS 0
#endif

#ifdef NEXUS_HAS_HDMI_OUTPUT
#if BCHP_CHIP == 7439 && BCHP_VER == BCHP_VER_A0
#define NEXUS_NUM_HDMI_OUTPUTS 2
#else
#define NEXUS_NUM_HDMI_OUTPUTS 1
#endif
#else
#define NEXUS_NUM_HDMI_OUTPUTS 0
#endif

#ifdef NEXUS_HAS_CEC
#define NEXUS_NUM_CEC 1
#else
#define NEXUS_NUM_CEC 0
#endif

#ifdef NEXUS_HAS_RFM
#define NEXUS_NUM_RFM_OUTPUTS  1
#else
#define NEXUS_NUM_RFM_OUTPUTS  0
#endif

#ifdef NEXUS_HAS_GRAPHICS2D
#if (BCHP_CHIP == 7278 ) && !defined NEXUS_WEBCPU
#define NEXUS_NUM_2D_ENGINES 3
#elif (BCHP_CHIP == 7366 || BCHP_CHIP == 7435 || BCHP_CHIP == 74371 || BCHP_CHIP == 7439 || BCHP_CHIP == 7445 || BCHP_CHIP == 7278 || BCHP_CHIP == 11360 ) && !defined NEXUS_WEBCPU
#define NEXUS_NUM_2D_ENGINES 2
#else
#define NEXUS_NUM_2D_ENGINES 1
#endif
#else
#define NEXUS_NUM_2D_ENGINES 0
#endif

#ifdef NEXUS_HAS_VIDEO_DECODER
/* unused. only for backward compat. */
#define NEXUS_NUM_XVD_DEVICES 1
#define NEXUS_NUM_MOSAIC_DECODES 14
#else
#define NEXUS_NUM_XVD_DEVICES 0
#define NEXUS_NUM_MOSAIC_DECODES 0
#endif

/* NEXUS_NUM_656_INPUTS is unused in Nexus except in some older pinmux code,
but is retained here for app backward compat */
#define NEXUS_NUM_656_INPUTS 0

#define NEXUS_HAS_GFD_VERTICAL_UPSCALE 1

#define NEXUS_PLATFORM_DEFAULT_HEAP (0)

/* unused. only for backward compat. */
#define NEXUS_NUM_GPIO_PINS      256
#define NEXUS_NUM_SGPIO_PINS     16
#define NEXUS_NUM_AON_GPIO_PINS  32
#define NEXUS_NUM_AON_SGPIO_PINS 16

/* do not use. extend NEXUS_DisplayCapabilities if needed. */
#define NEXUS_NUM_DNR 5
#define NEXUS_NUM_DCR 4
#define NEXUS_NUM_VIDEO_DACS 7
#define NEXUS_NUM_LAB 2
#define NEXUS_NUM_CAB 2
#define NEXUS_NUM_PEP 1
#define NEXUS_HAS_TNT 1

#ifdef NEXUS_HAS_AUDIO
/* NEXUS_NUM_AUDIO_DSP is deprecated and may be higher than actual number of DSP's.
Use NEXUS_AudioCapabilities.numDsps at run-time instead.

For a compile-time test for RAAGA, use:
#include "nexus_audio_init.h"
#if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
*/
#define NEXUS_NUM_AUDIO_DSP 2

#ifndef NEXUS_NUM_AUDIO_DECODERS
#define NEXUS_NUM_AUDIO_DECODERS NEXUS_MAX_AUDIO_DECODERS
#endif

#ifndef NEXUS_NUM_AUDIO_PLAYBACKS
#define NEXUS_NUM_AUDIO_PLAYBACKS NEXUS_MAX_AUDIO_PLAYBACKS
#endif

#ifndef NEXUS_NUM_AUDIO_MIXERS
#define NEXUS_NUM_AUDIO_MIXERS NEXUS_MAX_AUDIO_MIXERS
#endif

#ifndef NEXUS_HAS_AUDIO_MUX_OUTPUT
#define NEXUS_HAS_AUDIO_MUX_OUTPUT 1
#endif

#ifndef NEXUS_NUM_AUDIO_DACS
#define NEXUS_NUM_AUDIO_DACS NEXUS_MAX_AUDIO_DAC_OUTPUTS
#endif

#ifndef NEXUS_NUM_I2S_OUTPUTS
#define NEXUS_NUM_I2S_OUTPUTS NEXUS_MAX_AUDIO_I2S_OUTPUTS
#endif

#ifndef NEXUS_NUM_SPDIF_OUTPUTS
#define NEXUS_NUM_SPDIF_OUTPUTS NEXUS_MAX_AUDIO_SPDIF_OUTPUTS
#endif

#ifndef NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
#define NEXUS_NUM_AUDIO_DUMMY_OUTPUTS NEXUS_MAX_AUDIO_DUMMY_OUTPUTS
#endif

#ifndef NEXUS_NUM_AUDIO_CAPTURES
#define NEXUS_NUM_AUDIO_CAPTURES NEXUS_MAX_AUDIO_CAPTURE_OUTPUTS
#endif

#ifndef NEXUS_NUM_AUDIO_CRCS
#define NEXUS_NUM_AUDIO_CRCS NEXUS_MAX_AUDIO_CRC_OUTPUTS
#endif

#ifndef NEXUS_NUM_I2S_INPUTS
#define NEXUS_NUM_I2S_INPUTS NEXUS_MAX_AUDIO_I2S_INPUTS
#endif

#ifndef NEXUS_NUM_SPDIF_INPUTS
#define NEXUS_NUM_SPDIF_INPUTS NEXUS_MAX_AUDIO_SPDIF_INPUTS
#endif

#ifndef NEXUS_NUM_AUDIO_INPUT_CAPTURES
#define NEXUS_NUM_AUDIO_INPUT_CAPTURES NEXUS_MAX_AUDIO_INPUT_CAPTURES
#endif
#else /* NEXUS_HAS_AUDIO */
#ifndef NEXUS_NUM_AUDIO_DECODERS
#define NEXUS_NUM_AUDIO_DECODERS 0
#endif

#ifndef NEXUS_NUM_AUDIO_PLAYBACKS
#define NEXUS_NUM_AUDIO_PLAYBACKS 0
#endif
#ifndef NEXUS_NUM_AUDIO_MIXERS
#define NEXUS_NUM_AUDIO_MIXERS 0
#endif

#ifndef NEXUS_HAS_AUDIO_MUX_OUTPUT
#define NEXUS_HAS_AUDIO_MUX_OUTPUT 0
#endif

#ifndef NEXUS_NUM_AUDIO_DACS
#define NEXUS_NUM_AUDIO_DACS 0
#endif

#ifndef NEXUS_NUM_I2S_OUTPUTS
#define NEXUS_NUM_I2S_OUTPUTS 0
#endif

#ifndef NEXUS_NUM_SPDIF_OUTPUTS
#define NEXUS_NUM_SPDIF_OUTPUTS 0
#endif

#ifndef NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
#define NEXUS_NUM_AUDIO_DUMMY_OUTPUTS 0
#endif

#ifndef NEXUS_NUM_AUDIO_CAPTURES
#define NEXUS_NUM_AUDIO_CAPTURES 0
#endif

#ifndef NEXUS_NUM_AUDIO_CRCS
#define NEXUS_NUM_AUDIO_CRCS 0
#endif

#ifndef NEXUS_NUM_I2S_INPUTS
#define NEXUS_NUM_I2S_INPUTS 0
#endif

#ifndef NEXUS_NUM_SPDIF_INPUTS
#define NEXUS_NUM_SPDIF_INPUTS 0
#endif

#ifndef NEXUS_NUM_AUDIO_INPUT_CAPTURES
#define NEXUS_NUM_AUDIO_INPUT_CAPTURES 0
#endif
#endif

#define NEXUS_MEMC0_MAIN_HEAP                   0
#define NEXUS_VIDEO_SECURE_HEAP                 1
#define NEXUS_SAGE_SECURE_HEAP                  2
#define NEXUS_MEMC0_PICTURE_BUFFER_HEAP         3
#define NEXUS_MEMC0_GRAPHICS_HEAP               4
#define NEXUS_MEMC0_DRIVER_HEAP                 5
#define NEXUS_MEMC1_PICTURE_BUFFER_HEAP         6
#define NEXUS_MEMC1_GRAPHICS_HEAP               7
#define NEXUS_MEMC1_DRIVER_HEAP                 8
#define NEXUS_MEMC2_PICTURE_BUFFER_HEAP         9
#define NEXUS_MEMC2_GRAPHICS_HEAP               10
#define NEXUS_MEMC2_DRIVER_HEAP                 11
#define NEXUS_MEMC0_SECURE_PICTURE_BUFFER_HEAP  12
#define NEXUS_MEMC1_SECURE_PICTURE_BUFFER_HEAP  13
#define NEXUS_MEMC2_SECURE_PICTURE_BUFFER_HEAP  14
#define NEXUS_MEMC0_SECURE_GRAPHICS_HEAP        15
#define NEXUS_MEMC1_SECURE_GRAPHICS_HEAP        16
#define NEXUS_MEMC2_SECURE_GRAPHICS_HEAP        17
#define NEXUS_EXPORT_HEAP                       18

#endif
