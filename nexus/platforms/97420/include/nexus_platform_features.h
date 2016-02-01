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

#include "bstd.h"
#include "nexus_platform_generic_features_priv.h"

/* Transport Features */
#define NEXUS_NUM_PARSER_BANDS 6
#define NEXUS_NUM_PID_CHANNELS 256
#define NEXUS_NUM_VCXOS 2
#if (BCHP_VER >= BCHP_VER_B0)
#else
#endif
#define NEXUS_NUM_MESSAGE_FILTERS 128
#define NEXUS_NUM_PLAYPUMPS 8

#define NEXUS_NUM_VIDEO_DECODERS 2


/* XVD Heaps
See 7420_Memory_Worksheet.xls to calculate custom numbers */
/* 32(MEMC0)+32(MEMC1) bit non UMA - 2 HD Decodes, 1 HD Still, 1 SD Still 6 CIF , 12 QCIF*/
#define NEXUS_VIDEO_DECODER_32MEMC0_32MEMC1_GENERAL_HEAP_SIZE (10*1024*1024)
#define NEXUS_VIDEO_DECODER_32MEMC0_32MEMC1_SECURE_HEAP_SIZE  (0)
/* We're allocating the same Picture heap size on both controllers for now,
   however, we'll have to modify this eventually to support separate allocations
   on MEMC0 and MEMC1 */
#define NEXUS_VIDEO_DECODER_32MEMC0_32MEMC1_PICTURE_HEAP_SIZE (26*1024*1024)
#define NEXUS_NUM_VIDEO_DECODERS_32MEMC0_32_MEMC1 NEXUS_NUM_VIDEO_DECODERS


/* Audio Features */
#define NEXUS_NUM_AUDIO_DECODERS 3
#define NEXUS_NUM_AUDIO_DACS 2
#define NEXUS_NUM_SPDIF_OUTPUTS 1
#define NEXUS_NUM_AUDIO_MIXERS 8

#define NEXUS_NUM_I2S_INPUTS 1
#define NEXUS_NUM_I2S_OUTPUTS 2
#define NEXUS_NUM_AUDIO_CAPTURE_CHANNELS 1
#define NEXUS_NUM_AUDIO_CAPTURES 1
#define NEXUS_NUM_AUDIO_PLAYBACKS 3

/* Display Features */
#define NEXUS_NUM_656_OUTPUTS 1
#define NEXUS_NUM_COMPONENT_OUTPUTS 1
#define NEXUS_NUM_COMPOSITE_OUTPUTS 2
#define NEXUS_NUM_SVIDEO_OUTPUTS 1
#define NEXUS_NUM_HDDVI_INPUTS 1

#if NEXUS_DVO_DVI_LOOPBACK_SUPPORT
#define NEXUS_NUM_HDMI_DVO 1
#else
#define NEXUS_NUM_HDMI_DVO 0
#endif
/* Max number of displays and windows supported
by this platform. Actual numbers may vary depeding
upon the chip usage. See below */
#define NEXUS_NUM_DISPLAYS 3
#define NEXUS_NUM_VIDEO_WINDOWS   2


/*
 * MEMC1 memory heap is the main heap handle passed to BVDC_Open.
 */
#define NEXUS_32MEMC0_32MEMC1_STR  "32+32 bit NonUMA,Display HD+SD,1080p"

#define NEXUS_DISPLAY_NUM_FULLHD_BUFFERS_ON_MEMC1         5
#define NEXUS_DISPLAY_NUM_HD_BUFFERS_ON_MEMC1             0
#define NEXUS_DISPLAY_NUM_SD_BUFFERS_ON_MEMC1             0
#define NEXUS_DISPLAY_NUM_FULLHD_PIP_BUFFERS_ON_MEMC1     3
#define NEXUS_DISPLAY_NUM_HD_PIP_BUFFERS_ON_MEMC1         0
#define NEXUS_DISPLAY_NUM_SD_PIP_BUFFERS_ON_MEMC1         0
#define NEXUS_NUM_VIDEO_WINDOWS_ON_MEMC1                  NEXUS_NUM_VIDEO_WINDOWS


/*
 * VDC PIP windows heap would be created on MEMC0.
 */

/*
 * For Z order swap the VDC buffers must be same on both the 
 * memory controllers.If no Z order, then VDC windows 
 * are destroyed and recreated and attached to swapped input
 * sources.
 */
#if NEXUS_ZORDER_PIP_SWAP
#define NEXUS_DISPLAY_NUM_FULLHD_BUFFERS_ON_MEMC0         4
#define NEXUS_DISPLAY_NUM_HD_BUFFERS_ON_MEMC0             0
#define NEXUS_DISPLAY_NUM_SD_BUFFERS_ON_MEMC0             0
#define NEXUS_DISPLAY_NUM_FULLHD_PIP_BUFFERS_ON_MEMC0     3
#define NEXUS_DISPLAY_NUM_HD_PIP_BUFFERS_ON_MEMC0         0
#define NEXUS_DISPLAY_NUM_SD_PIP_BUFFERS_ON_MEMC0         0
#else
#define NEXUS_DISPLAY_NUM_FULLHD_BUFFERS_ON_MEMC0         0
#define NEXUS_DISPLAY_NUM_HD_BUFFERS_ON_MEMC0             0
#define NEXUS_DISPLAY_NUM_SD_BUFFERS_ON_MEMC0             3
#define NEXUS_DISPLAY_NUM_FULLHD_PIP_BUFFERS_ON_MEMC0     3
#define NEXUS_DISPLAY_NUM_HD_PIP_BUFFERS_ON_MEMC0         0
#define NEXUS_DISPLAY_NUM_SD_PIP_BUFFERS_ON_MEMC0         0
#endif
#define NEXUS_NUM_VIDEO_WINDOWS_ON_MEMC0                  NEXUS_NUM_VIDEO_WINDOWS

#define NEXUS_NUM_DISPLAYS_32MEMC0_32MEMC1_CONFIG0        NEXUS_NUM_DISPLAYS


/* Cable Frontend */
#define NEXUS_NUM_FRONTEND_CARD_SLOTS 0
#if VMS93380_SUPPORT
#define NEXUS_SHARED_FRONTEND_INTERRUPT
#define NEXUS_MAX_FRONTENDS 5
#define NEXUS_MAX_3255_ADSCHN 0
#define NEXUS_3255_OOB_TUNER_IFFREQ (1250000)/* 1.25 MHz */
#else
#define NEXUS_MAX_FRONTENDS 4
#define NEXUS_MAX_3255_ADSCHN 3
#endif


/* I2C Channels */
#define NEXUS_NUM_I2C_CHANNELS 5





/* SMARTCARD CHANNELS */
#define NEXUS_NUM_SMARTCARD_CHANNELS 2

/* DVB-CI Details */
#define NEXUS_DVB_CI_MEMORY_BASE (0x19800000)
#define NEXUS_DVB_CI_MEMORY_LENGTH (1024*1024)

/* Memory features */
#define NEXUS_NUM_MEMC 2

/* default heap indices */
#define NEXUS_MEMC0_MAIN_HEAP           0
#define NEXUS_MEMC1_MAIN_HEAP           1
#define NEXUS_MEMC1_GRAPHICS_HEAP       2
#define NEXUS_MEMC0_GRAPHICS_HEAP       3

#endif /* #ifndef NEXUS_PLATFORM_FEATURES_H__ */

