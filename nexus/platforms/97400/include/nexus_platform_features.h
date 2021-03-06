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
* API Description:
*   API name: Platform Features
*    This file defines a set of controllable fields for a 7400 platform.
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

#define NEXUS_NUM_PARSER_BANDS 6
#define NEXUS_NUM_PID_CHANNELS 256
#define NEXUS_NUM_VCXOS 2
#define NEXUS_NUM_MESSAGE_FILTERS 64
#define NEXUS_NUM_PLAYPUMPS 5

/* Audio Features */
#define NEXUS_NUM_AUDIO_DECODERS 3
#define NEXUS_NUM_AUDIO_DACS 3      /* This is a lie, there are 2, but it allows one DAC per display, 
                                       dac0 for display[0], dac1 for display[1] and display[2], as
                                       per native Settop API.
                                       Settop won't use DAC on both display[1] and display[2], you 
                                       shouldn't either. */
#define NEXUS_NUM_SPDIF_OUTPUTS 1
#define NEXUS_NUM_AUDIO_MIXERS 9
#define NEXUS_NUM_I2S_OUTPUTS 3

#define NEXUS_NUM_AUDIO_CAPTURE_CHANNELS 0
#define NEXUS_NUM_AUDIO_CAPTURES 1
#define NEXUS_NUM_AUDIO_PLAYBACKS 2
#define NEXUS_HAS_AUDIO_TSM_LOG_SUPPORT 1

/* Video Features */
#define NEXUS_NUM_VIDEO_DECODERS 2
#define NEXUS_VIDEO_DECODER_GENERAL_HEAP_SIZE 17406362
#define NEXUS_VIDEO_DECODER_SECURE_HEAP_SIZE 0
#define NEXUS_VIDEO_DECODER_PICTURE_HEAP_SIZE 31813796


/* Display Features */

#define NEXUS_NUM_656_OUTPUTS 1
#define NEXUS_NUM_DISPLAYS 3
#define NEXUS_NUM_VIDEO_WINDOWS   2

#define NEXUS_DISPLAY_NUM_SD_BUFFERS         10
#define NEXUS_DISPLAY_NUM_HD_BUFFERS         0
#define NEXUS_DISPLAY_NUM_FULL_HD_BUFFERS    6      /* allocate full HD bufffers instead of HD buffers, required for 1080p o/p */  

/* Video outputs */
#define NEXUS_NUM_COMPOSITE_OUTPUTS 2
#define NEXUS_NUM_COMPONENT_OUTPUTS 1
#define NEXUS_NUM_SVIDEO_OUTPUTS 1


/* I2C Channels */
#define NEXUS_NUM_I2C_CHANNELS 5




/* The maximum number of frontends is usually equal to the number of slots on the reference board. */
#define NEXUS_MAX_FRONTENDS 7
#define NEXUS_NUM_FRONTEND_CARD_SLOTS 3



/* SMARTCARD CHANNELS */
#define NEXUS_NUM_SMARTCARD_CHANNELS 3

#define NEXUS_NUM_MEMC 3
#define NEXUS_PLATFORM_P_READ_MEMC_CONFIG 1

#endif /* #ifndef NEXUS_PLATFORM_FEATURES_H__ */

