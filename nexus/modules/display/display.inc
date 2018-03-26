#############################################################
#  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
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
############################################################

#
# Conditional Options for this Makefile
#

######################
# Define this module #
######################
NEXUS_MODULES += DISPLAY

##################
# Module Options #
##################

###################
# Module Includes #
###################
NEXUS_DISPLAY_PRIVATE_INCLUDES = $(NEXUS_TOP)/modules/display/src
NEXUS_DISPLAY_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/display/include


NEXUS_DISPLAY_PRECOMPILED_HEADER = $(NEXUS_TOP)/modules/display/src/nexus_display_module.h

##################
# Module Sources #
##################
ifeq ($(PLATFORM_CYGNUS),y)
NEXUS_DISPLAY_SRC_DIR = $(NEXUS_TOP)/../rockford/cygnus/nexus/modules/display/src
else
NEXUS_DISPLAY_SRC_DIR = $(NEXUS_TOP)/modules/display/src
endif
NEXUS_DISPLAY_SOURCES := \
    $(addprefix $(NEXUS_DISPLAY_SRC_DIR)/, \
        nexus_ccir656_input.c \
        nexus_hddvi_input.c \
        nexus_display.c \
        nexus_display_graphics.c \
        nexus_display_module.c \
        nexus_display_vbi.c \
        nexus_display_mosaic.c \
        nexus_hdmi_input_connection.c \
        nexus_picture_ctrl.c \
        nexus_video_adj.c \
        nexus_video_image_input.c \
        nexus_video_input.c \
        nexus_video_input_vbi.c \
        nexus_video_output.c \
        nexus_video_window.c \
    )

NEXUS_DISPLAY_LOCAL_SOURCES := $(NEXUS_TOP)/modules/display/src/nexus_display_local.c

# check the VDC's env variable
ifeq ($(BVDC_MACROVISION),y)
NEXUS_DISPLAY_DEFINES += MACROVISION_SUPPORT=1
else
# DCS and MACROVISION cannot be set at same time
ifeq ($(BVDC_DCS),y)
NEXUS_DISPLAY_DEFINES += DCS_SUPPORT=1
endif
endif

# capture VDC RUL's to disk for debug
# on the reference board, define capture_ruls to the filename
ifeq ($(BRDC_USE_CAPTURE_BUFFER),y)
NEXUS_DISPLAY_DEFINES += BRDC_USE_CAPTURE_BUFFER=1
endif

# output VDC multibuffer log
ifeq ($(BVDC_BUF_LOG),y)
NEXUS_DISPLAY_DEFINES += BVDC_BUF_LOG=1
endif


# turn on BVN error debug messages
BVDC_SUPPORT_BVN_DEBUG ?= y
ifeq ($(BVDC_SUPPORT_BVN_DEBUG),y)
NEXUS_DISPLAY_DEFINES += BVDC_SUPPORT_BVN_DEBUG=1
endif

ifeq ($(NEXUS_MTG_DISABLED),y)
NEXUS_DISPLAY_DEFINES += NEXUS_MTG_DISABLED=1
endif

#######################
# Module Dependencies #
#######################
NEXUS_DISPLAY_DEPENDENCIES := SURFACE VIDEO_DECODER HDMI_OUTPUT RFM HDMI_INPUT HDMI_DVO TRANSPORT SAGE GRAPHICS2D

# thunk layer
NEXUS_DISPLAY_PRIVATE_INCLUDES += $(NEXUS_SYNCTHUNK_DIR)
NEXUS_DISPLAY_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_display_thunks.c

##################
# Magnum Modules #
##################
# These modules will be excluded from app-only builds
NEXUS_DISPLAY_MAGNUM_MODULES =  \
    $(MAGNUM)/portinginterface/vdc/bvdc.inc \
    $(MAGNUM)/commonutils/udp/budp.inc \
    $(MAGNUM)/commonutils/rdc/brdc.inc \
    $(MAGNUM)/commonutils/cfc/bcfc.inc

ifneq ($(NEXUS_VBI_SUPPORT),n)
NEXUS_DISPLAY_MAGNUM_MODULES +=  \
    $(MAGNUM)/syslib/vbilib/bvbilib.inc \
    $(MAGNUM)/portinginterface/vbi/bvbi.inc

NEXUS_DISPLAY_DEFINES += NEXUS_VBI_SUPPORT=1
endif

ifneq ($(NEXUS_VIDEO_ENCODER_SUPPORT),n)
NEXUS_DISPLAY_MAGNUM_MODULES += $(MAGNUM)/syslib/xudlib/bxudlib.inc
endif

ifneq ($(NEXUS_XDM_SUPPORT),n)
ifeq ($(NEXUS_VIDEO_DECODER_SUPPORT),n)
NEXUS_DISPLAY_MAGNUM_MODULES += $(MAGNUM)/commonutils/xdm/bxdm.inc
endif
endif

BCHP_DBV_HW_DEF=$(NEXUS_TOP)/../magnum/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)/bchp_hdr_cmp_0_dbv.h
NEXUS_DISPLAY_DBV_IMPL=$(NEXUS_TOP)/modules/display/src/nexus_display_dbv.c
ifneq ("$(wildcard $(BCHP_DBV_HW_DEF))","")
ifneq ("$(wildcard $(NEXUS_DISPLAY_DBV_IMPL))","")
NEXUS_DBV_SUPPORT?=y
endif
endif
ifeq ($(NEXUS_DBV_SUPPORT),y)
BVDC_DBV_SUPPORT:=y
NEXUS_DISPLAY_DEFINES += NEXUS_DBV_SUPPORT=1
NEXUS_DISPLAY_SOURCES += $(NEXUS_TOP)/modules/display/src/nexus_display_dbv.c
else
BVDC_DBV_SUPPORT:=n
endif

ifeq ($(NEXUS_DISPLAY_DYNRNG_EXTENSION_SUPPORT),y)
include ${NEXUS_TOP}/extensions/display/dynrng/dynrng.inc
endif

ifneq ($(NEXUS_DISPLAY_EXTENSION_INC),)
include $(NEXUS_DISPLAY_EXTENSION_INC)
endif
