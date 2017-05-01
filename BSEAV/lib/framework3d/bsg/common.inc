#############################################################################
# Copyright (C) 2013 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#
#############################################################################

# Get the platform locations sorted
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

ifndef BCHP_VER
$(error BCHP_VER is not defined)
endif

# Set up paths (use shell to generate canonical paths)
BSEAV_DIR        := $(shell cd $(BSG_DIR)/../../..            ; pwd)
OPENSOURCE       := $(BSEAV_DIR)/lib
NEXUS_TOP        ?= $(shell cd $(BSEAV_DIR)/../nexus         ; pwd)

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

V3D_DIR          ?= $(shell cd $(BSEAV_DIR)/lib/gpu/$(V3D_PREFIX)/driver; pwd)
V3D_PLATFORM_DIR ?= $(shell cd $(V3D_DIR)/../platform                   ; pwd)

BCHP_VER_LOWER := $(shell echo $(BCHP_VER) | tr A-Z a-z)

CXXFLAGS += -O2 -DNDEBUG

ifeq ($(V3D_PREFIX),vc5)
  CXXFLAGS += -DBSG_USE_ES3 -DBSG_VC5
endif

PLATFORM_FULL = $(NEXUS_PLATFORM)$(BCHP_VER_LOWER)

GCCGTEQ_40800 := $(shell expr `$(CC) -dumpversion | awk 'BEGIN { FS = "." }; { printf("%d%02d%02d", $$1, $$2, $$3) }'` \>= 40800)

# Default architecture to little-endian
ifndef B_REFSW_ARCH
B_REFSW_ARCH = mipsel-linux
endif

# Use architecture to determine endian-ness flags
ifeq ($(filter ${B_REFSW_ARCH}, mips-linux mips-uclibc mips-linux-uclibc), ${B_REFSW_ARCH})
  CXXFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_BIG
else
  CXXFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE
endif

ifeq ($(NXCLIENT_SUPPORT),y)
# add nxclient
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CXXFLAGS += $(NXCLIENT_CFLAGS)
LDFLAGS  += $(NXCLIENT_LDFLAGS)
else
CXXFLAGS += -DSINGLE_PROCESS
endif

# C++ compiler flags
CXXFLAGS += -std=c++0x
CXXFLAGS += $(addprefix -I,$(INCLUDES))

