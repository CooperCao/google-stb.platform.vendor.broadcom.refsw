#############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#############################################################################

ifneq ($(MAKE_VERBOSE),)
Q_:=
else
Q_:=@
MAKEFLAGS += --no-print-directory
endif

# utility defines
CP := cp -fp

# arch defines
ARCH ?= arm
CROSS_COMPILE ?= arm-linux-

# path defines
TOP_DIR ?= $(CURDIR)/../..
B_REFSW_OBJ_ROOT ?= $(TOP_DIR)/../obj$(TZ_ARCH)/astra
ASTRA_TDRV_OBJ_DIR ?= $(B_REFSW_OBJ_ROOT)/linux/astra_tdrv
ASTRA_DIR ?= $(B_REFSW_OBJ_ROOT)/linux/bcm_astra
ifneq ($(LINUX),)
KDIR ?= $(LINUX)
endif

# file defines
ASTRA_TDRV := astra_tdrv.ko
ASTRA_SYMVERS := astra.symvers

# platform, target, and target revision
PLATFORM ?= brcmstb

# convert NEXUS_PLATFORM to astra defines
ifneq ($(NEXUS_PLATFORM),)
include $(TOP_DIR)/../nexus/platforms/common/build/nexus_platforms.inc

TARGET ?= $(BCHP_CHIP)
TARGET_REV ?= $(BCHP_VER_LOWER)
NEXUS_BUILD := y
else
TARGET ?= 7445
TARGET_REV ?= d0
NEXUS_BUILD := n
endif

# target rules
.PHONY: all check_symvers objdirs clean install

all: check_symvers
	cp -f Kbuild $(ASTRA_TDRV_OBJ_DIR)
	${Q_}$(MAKE) -C $(KDIR) \
		M=$(ASTRA_TDRV_OBJ_DIR) \
		src=$(CURDIR) \
		ARCH=$(ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		TOP_DIR=$(TOP_DIR) \
		PLATFORM=$(PLATFORM) \
		TARGET=$(TARGET) \
		TARGET_REV=$(TARGET_REV) \
		NEXUS_BUILD=$(NEXUS_BUILD) \
		KBUILD_EXTRA_SYMBOLS=$(ASTRA_TDRV_OBJ_DIR)/$(ASTRA_SYMVERS) \
		modules

check_symvers: objdirs
ifeq ($(wildcard $(ASTRA_SYMVERS)),)
	@echo "[Copy... "$@" from "$(ASTRA_DIR)"]";
	cp $(ASTRA_DIR)/Module.symvers $(ASTRA_TDRV_OBJ_DIR)/$(ASTRA_SYMVERS)
endif

objdirs:
	mkdir -p $(ASTRA_TDRV_OBJ_DIR)

clean:
	${Q_}$(RM) -r $(ASTRA_TDRV_OBJ_DIR)

install: all
ifeq ($(INSTALL_DIR),)
	$(error INSTALL_DIR is undefined)
else
	@echo "[Install... "$(ASTRA_TDRV)"]"
	${Q_}$(CP) $(ASTRA_TDRV_OBJ_DIR)/$(ASTRA_TDRV) $(INSTALL_DIR)
endif
