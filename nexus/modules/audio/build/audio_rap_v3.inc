############################################################
#     (c)2007-2013 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################

##################
# Module Sources #
##################
NEXUS_AUDIO_SOURCES += \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_association.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_capture_channel.c

####################
# Default Settings #
####################
RAP_MPEG_SUPPORT := y

######################
# Module Definitions #
######################
ifeq ($(RAP_AC3_SUPPORT),y)
NEXUS_AUDIO_DEFINES += NEXUS_AC3
endif
ifeq ($(RAP_DDP_SUPPORT),y)
NEXUS_AUDIO_DEFINES += NEXUS_DDP
RAP_DDP_TO_AC3_SUPPORT := y
# This is currently required for AC3 passthru
RAP_AC3_SUPPORT := y
NEXUS_AUDIO_DEFINES += NEXUS_AC3
endif
ifeq ($(RAP_MS10_SUPPORT),y)
NEXUS_AUDIO_DEFINES += NEXUS_AC3 NEXUS_DDP NEXUS_MS10
endif
ifeq ($(RAP_MS11_SUPPORT),y)
NEXUS_AUDIO_DEFINES += NEXUS_AC3 NEXUS_DDP NEXUS_MS10 NEXUS_MS11
endif
ifeq ($(RAP_WMA_SUPPORT),y)
NEXUS_AUDIO_DEFINES += NEXUS_WMA
endif
ifeq ($(RAP_WMAPRO_SUPPORT),y)
NEXUS_AUDIO_DEFINES += NEXUS_WMAPRO
endif

ifeq ($(RAP_SRSTRUVOL_SUPPORT),y)
NEXUS_AUDIO_DEFINES += NEXUS_SRSTRUVOL
endif

ifeq ($(RAP_DOLBYVOL_SUPPORT),y)
NEXUS_AUDIO_DEFINES += NEXUS_DOLBYVOL
endif


ifeq ($(NEXUS_CONFIG_IMAGE),y)
ifeq ($(NEXUS_MODE),proxy)
BRAP_IMG_ONLY := y
else
BRAP_IMG_EXTERNAL := y
endif
endif

ifeq ($(RAP_TSMLOG_SUPPORT),y)
NEXUS_AUDIO_DEFINES += BRAP_SUPPORT_TSM_LOG
endif

##################
# Magnum Modules #
##################
NEXUS_AUDIO_MAGNUM_MODULES =  \
    $(MAGNUM)/portinginterface/rap/brap.inc

#####################
# Module Extensions #
#####################
ifneq ($(findstring NEXUS_NUM_ZSP_VIDEO_DECODERS, $(NEXUS_PLATFORM_DEFINES)),)
include ${NEXUS_TOP}/modules/audio/${BCHP_CHIP}/video_ext/video_ext.inc
endif
ifneq ($(NEXUS_AUDIO_EXTENSION_INC),)
include $(NEXUS_AUDIO_EXTENSION_INC)
endif
ifeq ($(NEXUS_AUDIO_BUFFER_CAPTURE),y)
include $(NEXUS_TOP)/extensions/audio/buffer_capture/$(BCHP_CHIP)/buffer_capture.inc
endif
