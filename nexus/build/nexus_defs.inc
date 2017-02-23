############################################################
#  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#
# Module Description:
#   This file contains a set of definitions for building or using nexus
#
############################################################

############################################################
# Overview:
#
# This file is used by both nexus and application Makefiles
# It is included by the platform Makefile (for internal nexus builds)
# It is included by the platform_app.inc (for application builds)
#
############################################################

#
# Sanity check that NEXUS_TOP is defined
#
ifndef NEXUS_TOP
$(error NEXUS_TOP is not defined.)
endif

# backward-compatibility
ifdef PLATFORM
NEXUS_PLATFORM ?= $(PLATFORM)
endif
ifdef MODE
NEXUS_MODE ?= $(MODE)
endif
ifdef OS
B_REFSW_OS ?= $(OS)
endif
ifdef DEBUG
B_REFSW_DEBUG ?= $(DEBUG)
endif
ifdef VERBOSE
B_REFSW_VERBOSE ?= $(VERBOSE)
endif
ifdef ARCH
B_REFSW_ARCH ?= $(ARCH)
endif
ifdef SHAREABLE
B_REFSW_SHAREABLE ?= $(SHAREABLE)
endif
ifdef NEXUS_EXTRALDFLAGS
NEXUS_EXTRA_LDFLAGS += $(NEXUS_EXTRALDFLAGS)
endif

ifdef NEXUS_ANDROID_SUPPORT
ifndef ANDROID
$(error ANDROID must point to the top of a previously built Android source directory.)
endif

B_REFSW_ANDROID ?= $(NEXUS_ANDROID_SUPPORT)
endif

#
# Builds default to linuxuser
#
ifeq ($(B_REFSW_OS),)
B_REFSW_OS = linuxuser
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${NEXUS_TOP}/../${B_REFSW_OBJ_DIR}

#
# Define the bin directories
#
ifeq ($(NEXUS_BIN_DIR),)
# location of final binaries
NEXUS_BIN_DIR = ${B_REFSW_OBJ_ROOT}/nexus/bin${NEXUS_BIN_DIR_SUFFIX}
endif
ifeq ($(NEXUS_BUILD_DIR),)
# location of intermediate binaries and build artifacts
NEXUS_BUILD_DIR := ${B_REFSW_OBJ_ROOT}/nexus/core
$(echo ${NEXUS_BUILD_DIR})
endif
ifeq ($(NEXUS_SYNCTHUNK_DIR),)
# subdir for auto-generated thunk files
NEXUS_SYNCTHUNK_DIR := $(NEXUS_BUILD_DIR)/syncthunk
endif

#
# Set OS-independent defaults
#
ifeq ($(B_REFSW_DEBUG),)
B_REFSW_DEBUG = y
endif

#
# Define root of other trees
#
ifeq ($(NEXUS_TOP),nexus)
MAGNUM := magnum
ROCKFORD := rockford
BSEAV := BSEAV
else
MAGNUM := $(NEXUS_TOP)/../magnum
ROCKFORD := $(MAGNUM)/../rockford
BSEAV := $(MAGNUM)/../BSEAV
endif
#
# Define options for quiet builds
#
ifneq ($(B_REFSW_VERBOSE),)
Q_:=
else
Q_:=@
MAKEFLAGS += --no-print-directory
endif

#
# Define debug options
# Always enable DBG, but allow B_REFSW_DEBUG=n to reduce size
# But allow B_REFSW_DEBUG=no_error_messages to fully turn off all DBG
#
ifeq ($(B_REFSW_DEBUG),no_error_messages)
# leave BDBG_DEBUG_BUILD=0
else
ifeq ($(B_REFSW_DEBUG),minimal)
# leave BDBG_DEBUG_BUILD=0
B_REFSW_DEBUG_LEVEL=err
B_REFSW_DEBUG_COMPACT_ERR=y
else
NEXUS_CFLAGS += -DBDBG_DEBUG_BUILD=1
ifeq ($(B_REFSW_DEBUG),n)
B_REFSW_DEBUG_LEVEL?=log
B_REFSW_DEBUG_COMPACT_ERR?=y
B_REFSW_DEBUG_ASSERT_FAIL?=n
else
# everything
endif
endif
endif

# multiprocess support
# enables class verification code
NEXUS_SERVER_SUPPORT ?= y
ifeq ($(NEXUS_SERVER_SUPPORT),y)
NEXUS_CFLAGS += -DNEXUS_SERVER_SUPPORT=1
endif


#
# Include os tools and flags
#
ifeq ($(filter $(B_REFSW_OS), ucos ucos_ii ucos_iii), $(B_REFSW_OS))
include $(NEXUS_TOP)/build/os/ucos/os_tools.inc
include $(NEXUS_TOP)/build/os/ucos/os_flags.inc
else
include $(NEXUS_TOP)/build/os/$(B_REFSW_OS)/os_tools.inc
include $(NEXUS_TOP)/build/os/$(B_REFSW_OS)/os_flags.inc
endif

#
# Install binaries to $(NEXUS_BUILD_DIR)/$(B_REFSW_OS).$(B_REFSW_ARCH)
#
ifeq ($(NEXUS_OBJ_DIR),)
ifeq ($(NEXUS_WEBCPU),core1_server)
NEXUS_OBJ_DIR := $(NEXUS_BUILD_DIR)/$(B_REFSW_OS).$(B_REFSW_ARCH).$(NEXUS_WEBCPU)
else
ifdef NEXUS_MODE
NEXUS_OBJ_DIR := $(NEXUS_BUILD_DIR)/$(B_REFSW_OS).$(B_REFSW_ARCH).$(NEXUS_MODE)
else
NEXUS_OBJ_DIR := $(NEXUS_BUILD_DIR)/$(B_REFSW_OS).$(B_REFSW_ARCH)
endif
endif
endif

#
# Define a flag to indicate this file has been included
#
NEXUS_DEFS_INCLUDED := yes

# default media flags for transport.inc, playback.inc and others
ifeq ($(MEDIA_ASF_SUPPORT),)
MEDIA_ASF_SUPPORT := $(shell test -e ${BSEAV}/lib/asf/basf.inc && echo y)
endif
ifeq ($(MEDIA_AVI_SUPPORT),)
MEDIA_AVI_SUPPORT := $(shell test -e ${BSEAV}/lib/avi/bavi.inc && echo y)
endif
ifeq ($(MEDIA_FLV_SUPPORT),)
MEDIA_FLV_SUPPORT := $(shell test -e ${BSEAV}/lib/media/flv/bflv.inc && echo y)
endif
ifeq ($(MEDIA_RMFF_SUPPORT),)
MEDIA_RMFF_SUPPORT := $(shell test -e ${BSEAV}/lib/media/rmff/brmff.inc && echo y)
endif
