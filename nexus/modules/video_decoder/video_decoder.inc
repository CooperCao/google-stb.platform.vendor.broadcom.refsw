############################################################
#     (c)2007-2015 Broadcom Corporation
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

#
# Conditional Options for this Makefile
#

######################
# Define this module #
######################
NEXUS_MODULES += VIDEO_DECODER

##################
# Module Options #
##################

###################
# Module Includes #
###################
NEXUS_VIDEO_DECODER_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/video_decoder/include
NEXUS_VIDEO_DECODER_PRECOMPILED_HEADER := $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder_module.h

##################
# Module Sources #
##################
NEXUS_VIDEO_DECODER_SOURCES := \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder.c \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder_avd_interface.c \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder_common.c \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder_extra.c \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder_userdata.c \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder_priv.c \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder_primer.c \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_still_decoder.c \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_still_decoder_priv.c \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder_trick.c


# Look for DSP video decoder first 
ifneq ($(findstring NEXUS_NUM_DSP_VIDEO_DECODERS, $(NEXUS_PLATFORM_DEFINES)),)
include ${NEXUS_TOP}/extensions/video_decoder/dsp_video_decoder/dsp_video_decoder.inc
NEXUS_VIDEO_DECODER_DYNAMIC_DISPATCH:=y
else
# No DSP video decoder, Look for ZSP video decoder
ifneq ($(findstring NEXUS_NUM_ZSP_VIDEO_DECODERS, $(NEXUS_PLATFORM_DEFINES)),)
include ${NEXUS_TOP}/modules/video_decoder/zspdecode_ext/zspdecode_ext.inc
NEXUS_VIDEO_DECODER_DYNAMIC_DISPATCH:=y
endif
endif

ifneq ($(findstring NEXUS_NUM_SID_VIDEO_DECODERS, $(NEXUS_PLATFORM_DEFINES)),)
ifneq ($(NEXUS_PICTURE_DECODER_SUPPORT),n)
NEXUS_VIDEO_DECODER_DYNAMIC_DISPATCH:=y
include ${NEXUS_TOP}/extensions/video_decoder/sid_video_decoder/sid_video_decoder.inc
endif
endif

ifneq ($(findstring NEXUS_NUM_SOFT_VIDEO_DECODERS, $(NEXUS_PLATFORM_DEFINES)),)
NEXUS_VIDEO_DECODER_DYNAMIC_DISPATCH:=y
include ${NEXUS_TOP}/extensions/video_decoder/soft_video_decoder/soft_video_decoder.inc
endif

ifdef NEXUS_VIDEO_DECODER_DYNAMIC_DISPATCH
NEXUS_VIDEO_DECODER_SOURCES += \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder_dispatch.c \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder_dispatch_init.c \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder_xdm.c 
else
#NO SID/ZSP/DSP video decoder
NEXUS_VIDEO_DECODER_SOURCES += \
    $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder_avd.c 
endif


# thunk layer
NEXUS_VIDEO_DECODER_PRIVATE_INCLUDES += \
    $(NEXUS_TOP)/modules/video_decoder/src $(NEXUS_SYNCTHUNK_DIR)

NEXUS_VIDEO_DECODER_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_video_decoder_thunks.c

ifeq ($(NEXUS_CRC_CAPTURE),y)
NEXUS_VIDEO_DECODER_DEFINES += NEXUS_CRC_CAPTURE=1
endif

ifeq ($(NEXUS_VIDEO_DECODER_GARBAGE_FIFO_WATCHDOG_SUPPORT),y)
NEXUS_VIDEO_DECODER_DEFINES += NEXUS_VIDEO_DECODER_GARBAGE_FIFO_WATCHDOG_SUPPORT=1
endif

#######################
# Module Dependencies #
#######################
NEXUS_VIDEO_DECODER_DEPENDENCIES += TRANSPORT SURFACE

######################
# Module Definitions #
######################

ifeq ($(NEXUS_CONFIG_IMAGE),y)
ifeq ($(NEXUS_MODE),proxy)
BXVD_IMG_ONLY := y
else
BXVD_IMG_EXTERNAL := y
endif
endif

# Allow NEXUS_POWER_MANAGEMENT to be a master switch
ifneq ($(NEXUS_POWER_MANAGEMENT),y)
BXVD_POWER_MANAGEMENT := n
else
# XVD defaults off for settops, so force it on
BXVD_POWER_MANAGEMENT := y
endif

##################
# Magnum Modules #
##################
# These modules will be excluded from app-only builds
NEXUS_VIDEO_DECODER_MAGNUM_MODULES :=  \
    $(MAGNUM)/portinginterface/xvd/bxvd.inc \
    $(MAGNUM)/commonutils/img/bimg.inc \
    $(MAGNUM)/commonutils/xdm/bxdm.inc \
    $(MAGNUM)/commonutils/udp/budp.inc \
    $(MAGNUM)/commonutils/vlc/bvlc.inc 

ifeq ($(filter $(BCHP_CHIP), 7125 7340 7420 7468 7405 7400 7325 7335 7408 7550), $(BCHP_CHIP))
NEXUS_VIDEO_FW_CHECKING_FAMILY := 65nm
else
# Used to load the new ARC600 based decoder
NEXUS_VIDEO_DECODER_MAGNUM_MODULES +=  \
    $(MAGNUM)/commonutils/afl/bafl.inc
NEXUS_VIDEO_FW_CHECKING_FAMILY := 40nm
endif


ifeq ($(OTFPVR_SUPPORT),y)
NEXUS_VIDEO_DECODER_MAGNUM_MODULES += ${BSEAV}/lib/otfpvr/botfpvr.inc
# BSEAV makefiles don't include their module names into MAGNUM_MODULES. must conditionally include into the driver, not proxy.
ifneq ($(findstring $(NEXUS_MODE),proxy client),)
else
NEXUS_EXTRA_MODULES += BOTFPVR
endif
NEXUS_VIDEO_DECODER_DEFINES += NEXUS_OTFPVR=1
NEXUS_VIDEO_DECODER_SOURCES += $(NEXUS_TOP)/modules/video_decoder/src/nexus_video_decoder_otf_trick.c
endif

include ${NEXUS_TOP}/extensions/video_decoder/security/$(NEXUS_VIDEO_FW_CHECKING_FAMILY)/secure_ext.inc

ifeq ($(B_REFSW_DSS_SUPPORT),)
B_REFSW_DSS_SUPPORT=y
endif
ifeq ($(B_REFSW_DSS_SUPPORT),y)
NEXUS_VIDEO_DECODER_DEFINES += B_REFSW_DSS_SUPPORT=1
endif

#####################
# Module Extensions #
#####################

