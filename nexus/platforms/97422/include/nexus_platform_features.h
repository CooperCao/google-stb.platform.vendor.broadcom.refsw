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
#define NEXUS_NUM_PARSER_BANDS 16
#if (BCHP_VER >= BCHP_VER_B0)
#define NEXUS_NUM_PID_CHANNELS 512
#else
#define NEXUS_NUM_PID_CHANNELS 256
#endif
#define NEXUS_NUM_VCXOS 3
#define NEXUS_NUM_MESSAGE_FILTERS 128
#define NEXUS_NUM_PLAYPUMPS 16
#define NEXUS_NUM_PACKET_SUB 8

#define NEXUS_NUM_VIDEO_DECODERS 2


/* XVD Heaps
   TODO: Add correct XVD memory sizes when SVC 3D numbers are available
See 7422_Memory_Worksheet.xls to calculate custom numbers */
/* 32(MEMC0)+32(MEMC1) bit non UMA - 2 HD Decodes, 1 HD Still, 1 SD Still 11 CIF , 14 QCIF
   SVD1: Includes support for SVC 2D/3D and MVC
   AVD0: Includes support for AVC Level 4.2 */
#define NEXUS_VIDEO_DECODER_SVD0_32MEMC0_GENERAL_HEAP_SIZE (21*1024*1024)
#define NEXUS_VIDEO_DECODER_SVD0_32MEMC1_SECURE_HEAP_SIZE  (0)
#define NEXUS_VIDEO_DECODER_SVD0_32MEMC1_PICTURE_HEAP_SIZE (55*1024*1024)

#define NEXUS_VIDEO_DECODER_AVD1_32MEMC0_GENERAL_HEAP_SIZE (16*1024*1024)
#define NEXUS_VIDEO_DECODER_AVD1_32MEMC0_SECURE_HEAP_SIZE  (0)
#define NEXUS_VIDEO_DECODER_AVD1_32MEMC0_PICTURE_HEAP_SIZE (43*1024*1024)

/* Audio Features */
#define NEXUS_NUM_AUDIO_DECODERS 6
#define NEXUS_NUM_AUDIO_DACS 2
#define NEXUS_NUM_SPDIF_INPUTS 1
#define NEXUS_NUM_SPDIF_OUTPUTS 1
#define NEXUS_NUM_AUDIO_DUMMY_OUTPUTS 4
#define NEXUS_NUM_AUDIO_MIXERS 8
#define NEXUS_NUM_AUDIO_INPUT_CAPTURES 3

#define NEXUS_NUM_I2S_INPUTS 1
#define NEXUS_NUM_I2S_OUTPUTS 2
#define NEXUS_NUM_AUDIO_CAPTURE_CHANNELS 1
#define NEXUS_NUM_AUDIO_CAPTURES 1
#define NEXUS_NUM_AUDIO_PLAYBACKS 3

/* Display Features */
#define NEXUS_NUM_656_OUTPUTS 2
#define NEXUS_NUM_COMPONENT_OUTPUTS 1
#define NEXUS_NUM_COMPOSITE_OUTPUTS 1
#define NEXUS_NUM_SVIDEO_OUTPUTS 0

/* Max number of displays and windows supported
by this platform. Actual numbers may vary depeding
upon the chip usage. See below */
#define NEXUS_NUM_DISPLAYS 3
#define NEXUS_NUM_VIDEO_WINDOWS   2


/*  - Display buffers required for main memory heap on MEMC1
 *  - The main path, main PIP, secondary path, and secondary PIP path
 *    are allocated from the main VDC heap on MEMC1 */
#define NEXUS_32MEMC0_32MEMC1_STR  "32+32 bit NonUMA,Display HD+HD,1080p"
#define NEXUS_DISPLAY_NUM_SD_BUFFERS_MEMC1              0
#define NEXUS_DISPLAY_NUM_SD_PIP_BUFFERS_MEMC1          5
#define NEXUS_DISPLAY_NUM_FULL_HD_BUFFERS_MEMC1         8
#define NEXUS_DISPLAY_NUM_FULL_HD_PIP_BUFFERS_MEMC1     13
#define NEXUS_DISPLAY_NUM_HD_BUFFERS_MEMC1              12
#define NEXUS_DISPLAY_NUM_HD_PIP_BUFFERS_MEMC1          0

#define NEXUS_DISPLAY_NUM_SD_BUFFERS_MEMC0              0
#define NEXUS_DISPLAY_NUM_SD_PIP_BUFFERS_MEMC0          0
#define NEXUS_DISPLAY_NUM_FULL_HD_BUFFERS_MEMC0         0
#define NEXUS_DISPLAY_NUM_FULL_HD_PIP_BUFFERS_MEMC0     0
#define NEXUS_DISPLAY_NUM_HD_BUFFERS_MEMC0              0
#define NEXUS_DISPLAY_NUM_HD_PIP_BUFFERS_MEMC0          0


#if defined(NEXUS_PLATFORM_7422_DBS)
/* DBS Frontend */
#define NEXUS_MAX_FRONTENDS 8
#else
/* Cable Frontend */
#if defined(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT)
#define NEXUS_MAX_FRONTENDS 17
#else
#define NEXUS_MAX_FRONTENDS 9
#endif

#if NEXUS_USE_FRONTEND_DAUGHTER_CARD
#define NEXUS_NUM_FRONTEND_CARD_SLOTS 1
#endif
#endif

#define NEXUS_MAX_3255_ADSCHN 0
#define NEXUS_3255_OOB_TUNER_IFFREQ (1250000)/* 1.25 MHz */
#define NEXUS_SHARED_FRONTEND_INTERRUPT 1

/* SPI */
#define NEXUS_NUM_SPI_CHANNELS 2



/* I2C Channels */
#define NEXUS_NUM_I2C_CHANNELS 5

/* I2C channel usage assignments. Refer to BSC table in the board schematics. */
#if (BCHP_VER >= BCHP_VER_B0)
#define NEXUS_I2C_CHANNEL_HDMI_TX         0
#define NEXUS_I2C_CHANNEL_LNA             3  /* BCM3405 */
#define NEXUS_I2C_CHANNEL_EXT_RFM         3  /* External RFM */
#define NEXUS_I2C_CHANNEL_TUNERS_4_5      3  /* BCM3112_4/_5(for SV board) */
#define NEXUS_I2C_CHANNEL_TUNERS_0_1_2_3  4  /* BCM3112_0/_1/_2/_3(for SV board) */
#define NEXUS_I2C_CHANNEL_DSTRM_TUNER     4  /* BCM3128(for VMS board) */
#else
#define NEXUS_I2C_CHANNEL_HDMI_TX         3
#define NEXUS_I2C_CHANNEL_LNA             1  /* BCM3405 */
#define NEXUS_I2C_CHANNEL_EXT_RFM         1  /* External RFM */
#define NEXUS_I2C_CHANNEL_DSTRM_TUNER     2  /* BCM3128(for VMS board) */
#define NEXUS_I2C_CHANNEL_TUNERS_4_5      1  /* BCM3112_4/_5(for SV board) */
#define NEXUS_I2C_CHANNEL_TUNERS_0_1_2_3  2  /* BCM3112_0/_1/_2/_3(for SV board) */
#endif





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
#define NEXUS_MEMC0_GRAPHICS_HEAP       2
#define NEXUS_MEMC0_PICTURE_BUFFER_HEAP 3
#define NEXUS_MEMC1_PICTURE_BUFFER_HEAP 4

#define NEXUS_AVS_MONITOR           1


#endif /* #ifndef NEXUS_PLATFORM_FEATURES_H__ */

