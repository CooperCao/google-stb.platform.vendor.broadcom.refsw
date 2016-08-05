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
 ******************************************************************************/

#ifndef NEXUS_PLATFORM_FEATURES_H_
#define NEXUS_PLATFORM_FEATURES_H_

#include "bstd.h"

/* Video Features,do not change these numbers
   this covers the chip max capabilities  */
#define NEXUS_NUM_VIDEO_DECODERS  0         /* 2 each for HEVD and each HVD */
#define NEXUS_NUM_XVD_DEVICES   0
#define NEXUS_NUM_MOSAIC_DECODES 0
#define NEXUS_NUM_STILL_DECODES 0

#define NEXUS_NUM_656_OUTPUTS 0
#define NEXUS_NUM_HDMI_INPUTS 0
#define NEXUS_NUM_HDMI_OUTPUTS 0
#define NEXUS_NUM_CEC 0
#define NEXUS_NUM_COMPONENT_OUTPUTS 0
#define NEXUS_NUM_COMPOSITE_OUTPUTS 0
#define NEXUS_NUM_SVIDEO_OUTPUTS 0
#define NEXUS_NUM_RFM_OUTPUTS  0
#define NEXUS_NUM_PARSER_BANDS 1

#define NEXUS_NUM_AUDIO_PLAYBACKS 0
#define NEXUS_NUM_AUDIO_DSP 0
#define NEXUS_NUM_AUDIO_DUMMY_OUTPUTS 0
/* Avoid audio logging (Not supported for this platform) */
#define NEXUS_AUDIO_MODULE_FAMILY BCHP_CHIP

#undef NEXUS_NUM_RFM_OUTPUTS
#define NEXUS_NUM_RFM_OUTPUTS 0

/* Video Encoder Features,do not change these numbers
   this covers the chip max capabilities  */
#define NEXUS_NUM_VCE_DEVICES    0
#define NEXUS_NUM_VCE_CHANNELS   0 /* per device */
#define NEXUS_NUM_VIDEO_ENCODERS   (NEXUS_NUM_VCE_CHANNELS * NEXUS_NUM_VCE_DEVICES)

/* Max number of displays and windows supported
by this platform. Actual numbers may vary depeding
upon the chip usage. See below */
#define NEXUS_NUM_DISPLAYS 1 /* C0/C1 for local HD/SD simul,C2 for display 3 & c3,c4,c5,c6  for quad transcodes */
#define NEXUS_NUM_VIDEO_WINDOWS   2 /* per display */
#if 0
#define NEXUS_NUM_DNR 5 /* for 6xMFDs */
#define NEXUS_NUM_DCR 4
#define NEXUS_NUM_VIDEO_DACS 4
#define NEXUS_NUM_LAB 1
#define NEXUS_NUM_CAB 1
#define NEXUS_NUM_PEP 1
#define NEXUS_HAS_TNT 1
#endif

#define NEXUS_NUM_2D_ENGINES 1
#define NEXUS_NUM_3D_ENGINES 1
#define NEXUS_HAS_GFD_VERTICAL_UPSCALE 0

#define NEXUS_MAX_FRONTENDS NEXUS_NUM_PARSER_BANDS

#define NEXUS_NUM_I2C_CHANNELS 0
#define NEXUS_MAX_I2C_CHANNELS 0

/* Memory features */
#define NEXUS_NUM_MEMC 1

/* default heap indices, refer to memory map document  */
#define NEXUS_MEMC0_MAIN_HEAP           0

#define NEXUS_PLATFORM_P_GET_FRAMEBUFFER_HEAP_INDEX 1

#define NEXUS_AVS_MONITOR           0
#endif /* #ifndef NEXUS_PLATFORM_FEATURES_H__ */
