############################################################
#  Copyright (C) 2018 Broadcom.
#  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to
#  the terms and conditions of a separate, written license agreement executed
#  between you and Broadcom (an "Authorized License").  Except as set forth in
#  an Authorized License, Broadcom grants no license (express or implied),
#  right to use, or waiver of any kind with respect to the Software, and
#  Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
#  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
#  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization,
#  constitutes the valuable trade secrets of Broadcom, and you shall use all
#  reasonable efforts to protect the confidentiality thereof, and to use this
#  information only in connection with your use of Broadcom integrated circuit
#  products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
############################################################

#
# Conditional Options for this Makefile
#

######################
# Define this module #
######################
NEXUS_MODULES += HDMI_OUTPUT

###################
# Module Includes #
###################
NEXUS_HDMI_OUTPUT_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/hdmi_output/include

NEXUS_HDMI_OUTPUT_PRECOMPILED_HEADER = $(NEXUS_TOP)/modules/hdmi_output/src/nexus_hdmi_output_module.h

##################
# Module Sources #
##################
NEXUS_HDMI_OUTPUT_SOURCES :=  \
                             $(NEXUS_TOP)/modules/hdmi_output/src/nexus_hdmi_output.c \
                             $(NEXUS_TOP)/modules/hdmi_output/src/nexus_hdmi_output_module.c \
                             $(NEXUS_TOP)/modules/hdmi_output/src/nexus_hdmi_output_hdcp.c \
                             $(NEXUS_TOP)/modules/hdmi_output/src/nexus_hdmi_output_hotplug.c \
                             $(NEXUS_TOP)/modules/hdmi_output/src/nexus_hdmi_output_format_validation.c \
                             $(NEXUS_TOP)/modules/hdmi_output/src/nexus_hdmi_output_control.c \
                             $(NEXUS_TOP)/modules/hdmi_output/src/nexus_hdmi_output_extra.c \
                             $(NEXUS_TOP)/modules/hdmi_output/src/nexus_hdmi_output_drm.c \
                             $(NEXUS_TOP)/modules/hdmi_output/src/nexus_hdmi_output_debug.c

# thunk layer
NEXUS_HDMI_OUTPUT_PRIVATE_INCLUDES += $(NEXUS_TOP)/modules/hdmi_output/src $(NEXUS_SYNCTHUNK_DIR)
NEXUS_HDMI_OUTPUT_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_hdmi_output_thunks.c

#######################
# Module Dependencies #
#######################
NEXUS_HDMI_OUTPUT_DEPENDENCIES := SECURITY I2C HDMI_INPUT SAGE

##################
# Magnum Modules #
##################
NEXUS_HDMI_OUTPUT_MAGNUM_MODULES =  \
    $(MAGNUM)/portinginterface/hdm/bhdm.inc

#
# HDCP (same logic exists in hdmi_input.inc)
#
# NEXUS_HDCP_SUPPORT=y  ==> enable 1.x and 2.x if HW supports and SAGE available (default)
# NEXUS_HDCP_SUPPORT=1x ==> enable 1.x only
# NEXUS_HDCP_SUPPORT=n  ==> no HDCP
#
ifeq ($(filter y 1x n,$(NEXUS_HDCP_SUPPORT)),)
NEXUS_HDCP_SUPPORT := y
endif

ifeq ($(findstring SECURITY, $(NEXUS_MODULES)),SECURITY)
	NEXUS_HDMI_OUTPUT_MAGNUM_MODULES += \
	    $(MAGNUM)/syslib/hdcplib/bhdcplib.inc

	ifeq ($(findstring SAGE, $(NEXUS_MODULES)),SAGE)
		NEXUS_HDMI_OUTPUT_MAGNUM_MODULES += \
		    $(MAGNUM)/syslib/sagelib/bsagelib.inc

		##########################################################
		##### SAGE PLATFORMS WITHOUT HDCP2.x SUPPORT ####
		##########################################################
		ifeq ($(filter $(BCHP_CHIP), 7435), $(BCHP_CHIP))
			### No DEFINES ####

		##################################################
		##### else, SAGE PLATFORMS SUPPORT HDCP2.x    ####
		##################################################
		else
			ifeq ($(NEXUS_HDCP_SUPPORT),y)
			NEXUS_HDMI_OUTPUT_DEFINES += NEXUS_HAS_HDCP_2X_SUPPORT
			BHDCPLIB_HDCP_SUPPORT=y
			endif
		endif
	endif

	# HDCP 1.x
	ifeq ($(filter y 1x,$(NEXUS_HDCP_SUPPORT)),$(NEXUS_HDCP_SUPPORT))
	BHSM_KEYLADDER=ON
	BHDCPLIB_HDCP_SUPPORT=y
	endif
endif

BCHP_DBV_HW_DEF=$(NEXUS_TOP)/../magnum/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)/bchp_hdr_cmp_0_dbv.h
NEXUS_HDMI_DBV_IMPL=$(NEXUS_TOP)/modules/hdmi_output/src/nexus_hdmi_output_dbv.c
ifneq ("$(wildcard $(BCHP_DBV_HW_DEF))","")
ifneq ("$(wildcard $(NEXUS_HDMI_DBV_IMPL))","")
NEXUS_DBV_SUPPORT?=y
endif
endif
ifeq ($(NEXUS_DBV_SUPPORT),y)
NEXUS_HDMI_OUTPUT_DEFINES += NEXUS_DBV_SUPPORT=1
NEXUS_HDMI_OUTPUT_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/hdmi_output/include/dbv
NEXUS_HDMI_OUTPUT_SOURCES += $(NEXUS_TOP)/modules/hdmi_output/src/nexus_hdmi_output_dbv.c
endif
