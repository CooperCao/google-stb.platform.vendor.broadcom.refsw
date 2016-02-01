/***************************************************************************
*     (c)2015 Broadcom Corporation
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
#if NEXUS_HAS_GRAPHICSV3D
#define NEXUS_NUM_3D_ENGINES 1
#else
#define NEXUS_NUM_3D_ENGINES 0
#endif

#if NEXUS_HAS_UHF_INPUT
#if BCHP_CHIP == 7400 || BCHP_CHIP == 7420
#define NEXUS_NUM_UHF_INPUTS 2
#else
#define NEXUS_NUM_UHF_INPUTS 1
#endif
#else
#define NEXUS_NUM_UHF_INPUTS 0
#endif

#if NEXUS_HAS_XPT_DMA
#define NEXUS_NUM_DMA_CHANNELS 31
#elif NEXUS_HAS_DMA
#if BCHP_CHIP == 7400 || BCHP_CHIP == 7435
#define NEXUS_NUM_DMA_CHANNELS 2
#else
#define NEXUS_NUM_DMA_CHANNELS 1
#endif
#endif

#if NEXUS_HAS_HDMI_INPUT
#define NEXUS_NUM_HDMI_INPUTS 1
#else
#define NEXUS_NUM_HDMI_INPUTS 0
#endif

#if NEXUS_HAS_HDMI_OUTPUT
#if BCHP_CHIP == 7439 && BCHP_VER == BCHP_VER_A0
#define NEXUS_NUM_HDMI_OUTPUTS 2
#else
#define NEXUS_NUM_HDMI_OUTPUTS 1
#endif
#else
#define NEXUS_NUM_HDMI_OUTPUTS 0
#endif

#if NEXUS_HAS_CEC
#define NEXUS_NUM_CEC 1
#else
#define NEXUS_NUM_CEC 0
#endif

#if NEXUS_HAS_RFM
#define NEXUS_NUM_RFM_OUTPUTS  1
#else
#define NEXUS_NUM_RFM_OUTPUTS  0
#endif

#if NEXUS_HAS_GRAPHICS2D
#if (BCHP_CHIP == 7145 || BCHP_CHIP == 7366 || BCHP_CHIP == 7435 || BCHP_CHIP == 74371 || BCHP_CHIP == 7439 || BCHP_CHIP == 7445) && !NEXUS_WEBCPU
#define NEXUS_NUM_2D_ENGINES 2
#else
#define NEXUS_NUM_2D_ENGINES 1
#endif
#else
#define NEXUS_NUM_2D_ENGINES 0
#endif

#if NEXUS_HAS_VIDEO_DECODER
/* TODO: convert to generic max when BBOX_XVD specifies this */
#if BCHP_CHIP == 7445
#define NEXUS_NUM_XVD_DEVICES 3
#elif BCHP_CHIP == 7400 || BCHP_CHIP == 7145 || BCHP_CHIP == 7420 || BCHP_CHIP == 7422 || BCHP_CHIP == 7425 || BCHP_CHIP == 7435 || BCHP_CHIP == 7439
#define NEXUS_NUM_XVD_DEVICES 2
#else
#define NEXUS_NUM_XVD_DEVICES 1
#endif
#define NEXUS_NUM_MOSAIC_DECODES 14
#endif

/* NEXUS_NUM_656_INPUTS is unused in Nexus except in some older pinmux code,
but is retained here for app backward compat */
#if BCHP_CHIP == 7125
#define NEXUS_NUM_656_INPUTS 1
#else
#define NEXUS_NUM_656_INPUTS 0
#endif

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

#if NEXUS_HAS_AUDIO
/* NEXUS_NUM_AUDIO_DSP is deprecated and may be higher than actual number of DSP's.
Use NEXUS_AudioCapabilities.numDsps at run-time instead.

For a compile-time test for RAAGA, use:
#include "nexus_audio_init.h"
#if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
*/
#define NEXUS_NUM_AUDIO_DSP 2
#endif

#endif
