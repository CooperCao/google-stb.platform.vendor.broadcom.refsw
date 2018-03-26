##############################################################################
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
######################
# Define this module #
######################
NEXUS_MODULES += HDMI_INPUT

##################
# Module Options #
##################
# Allow NEXUS_POWER_MANAGEMENT to be a master switch

###################
# Module Includes #
###################
NEXUS_HDMI_INPUT_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/hdmi_input/include

##################
# Module Sources #
##################
NEXUS_HDMI_INPUT_SOURCES := \
    $(NEXUS_TOP)/modules/hdmi_input/src/nexus_hdmi_input_module.c \
    $(NEXUS_TOP)/modules/hdmi_input/src/nexus_hdmi_input_debug.c \
    $(NEXUS_TOP)/modules/hdmi_input/src/nexus_hdmi_input.c \
    $(NEXUS_TOP)/modules/hdmi_input/src/nexus_hdmi_input_edid_ram.c \
    $(NEXUS_TOP)/modules/hdmi_input/src/nexus_hdmi_input_hdcp.c \
    $(NEXUS_TOP)/modules/hdmi_input/src/nexus_hdmi_input_hdcp_keyloader.c

# thunk layer
NEXUS_HDMI_INPUT_PRIVATE_INCLUDES += $(NEXUS_TOP)/modules/hdmi_input/src $(NEXUS_SYNCTHUNK_DIR)
NEXUS_HDMI_INPUT_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_hdmi_input_thunks.c

#######################
# Module Dependencies #
#######################
NEXUS_HDMI_INPUT_DEPENDENCIES := BASE CORE TRANSPORT SECURITY SAGE

##################
# Magnum Modules #
##################
# These modules will be excluded from app-only builds
NEXUS_HDMI_INPUT_MAGNUM_MODULES =  \
    $(MAGNUM)/portinginterface/hdr/bhdr.inc \
    $(MAGNUM)/commonutils/acm/bacm.inc

#
# HDCP (same logic exists in hdmi_output.inc)
#
# NEXUS_HDCP_SUPPORT=y  ==> enable 1.x and 2.x if HW supports and SAGE available (default)
# NEXUS_HDCP_SUPPORT=1x ==> enable 1.x only
# NEXUS_HDCP_SUPPORT=n  ==> no HDCP
#
ifeq ($(filter y 1x n,$(NEXUS_HDCP_SUPPORT)),)
NEXUS_HDCP_SUPPORT := y
endif

ifeq ($(findstring SAGE, $(NEXUS_MODULES)),SAGE)
ifeq ($(NEXUS_HDCP_SUPPORT),y)
NEXUS_HDMI_INPUT_MAGNUM_MODULES += \
    $(MAGNUM)/syslib/sagelib/bsagelib.inc \
    $(MAGNUM)/syslib/hdcplib/bhdcplib.inc
NEXUS_HDMI_INPUT_DEFINES += NEXUS_HAS_HDCP_2X_SUPPORT
BHDCPLIB_HDR_SUPPORT := y
endif
endif
ifeq ($(NEXUS_HDMI_INPUT_EXTENSION_SUPPORT),y)
include $(NEXUS_TOP)/../rockford/unittests/nexus/hdmi_input/extensions/hdmi_input_extn.inc
endif
