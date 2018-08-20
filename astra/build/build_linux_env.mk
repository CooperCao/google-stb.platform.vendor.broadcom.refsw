#############################################################################
# Copyright (C) 2018 Broadcom.
# The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to
# the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied),
# right to use, or waiver of any kind with respect to the Software, and
# Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
# THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
# IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#############################################################################

# TZ platform
PLATFORM ?= brcmstb

# convert NEXUS_PLATFORM to astra defines
ifneq ($(NEXUS_PLATFORM),)
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc

TARGET ?= $(BCHP_CHIP)
TARGET_REV ?= $(BCHP_VER)
NEXUS_BUILD := y
else
TARGET ?= 7445
TARGET_REV ?= d0
NEXUS_BUILD := n
endif

# TZ verbosity is derived from refsw verbosity if defined
ifeq ($(B_REFSW_VERBOSE),y)
VERBOSE := y
endif

# TZ verbosity specific defines
ifeq ($(VERBOSE),y)
Q_ :=
else
Q_ := @
MAKEFLAGS += --no-print-directory
endif

# TZ arch is derived from refsw arch if defined
ifeq ($(B_REFSW_ARCH),arm-linux)
ARCH ?= arm
else ifeq ($(B_REFSW_ARCH),aarch64-linux)
ARCH ?= arm64
endif

# TZ arch defaults to 64-bit ARM
ARCH ?= arm64

# TZ arch specific defines
ifeq ($(ARCH),arm)
# 32-bit ARM defines
LINUX_TOOLCHAIN ?= /projects/stbopt_p/toolchains_303/stbgcc-4.8-1.2
LINUX_CROSS_COMPILE ?= arm-linux-gnueabihf
TZ_ARMGNU ?= $(LINUX_TOOLCHAIN)/bin/$(LINUX_CROSS_COMPILE)
LINUX_USER_SYSROOT ?= $(LINUX_TOOLCHAIN)/$(LINUX_CROSS_COMPILE)/sys-root
ARCH_DIR    := arm
ARCHFLAGS   += -mcpu=cortex-a15 -mfpu=neon
OBJ_TARGET  := elf32-littlearm
else
# 64-bit ARM defines
LINUX_TOOLCHAIN ?= /opt/toolchains/stbgcc-6.3-1.2
LINUX_CROSS_COMPILE ?= aarch64-unknown-linux-gnueabi
TZ_ARMGNU ?= $(LINUX_TOOLCHAIN)/bin/$(LINUX_CROSS_COMPILE)
LINUX_USER_SYSROOT ?= $(LINUX_TOOLCHAIN)/$(LINUX_CROSS_COMPILE)/sys-root
ARCH_DIR    := aarch64
ARCHFLAGS   += -mcpu=cortex-a53+crypto
OBJ_TARGET  := elf64-littleaarch64
endif

# TZ obj top is derived from refsw obj root if defined
ifneq (,$(B_REFSW_OBJ_ROOT))
TZ_OBJ_TOP ?= $(B_REFSW_OBJ_ROOT)/astra
else ifneq (,$(B_REFSW_OBJ_DIR))
TZ_OBJ_TOP ?= $(TZ_TOP)/../$(B_REFSW_OBJ_DIR)/astra
endif

# TZ obj top default
TZ_OBJ_TOP ?= $(TZ_TOP)/../obj.$(ARCH_DIR)/astra

# TZ (module) src dir and obj dir
TZ_DIR ?= $(TZ_TOP)/$(TZ_MOD)
TZ_OBJ_DIR ?= $(TZ_OBJ_TOP)/$(TZ_MOD)
