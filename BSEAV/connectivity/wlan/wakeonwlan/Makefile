#############################################################################
# Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
CWD := $(shell pwd)
B_REFSW_ROOT := ${CWD}/../../../..

ifneq ($(B_REFSW_VERBOSE),)
Q_:=
else
Q_:=@
endif

ifeq ($(PLATFORM),)
PLATFORM=$(NEXUS_PLATFORM)
endif
ifeq ($(PLATFORM),)
$(error You must define PLATFORM)
endif

ifeq ($(LINUX),)
LINUX = /opt/brcm/linux
endif
LINUX_OUT ?= $(LINUX)

ifeq ($(filter ${B_REFSW_ARCH}, arm-linux ), ${B_REFSW_ARCH})
B_REFSW_CROSS_COMPILE ?= arm-linux-
B_REFSW_LINUX_ARCH=arm
else
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux ), ${B_REFSW_ARCH})
B_REFSW_CROSS_COMPILE ?= aarch64-linux-
B_REFSW_LINUX_ARCH=arm64
else
ifeq ($(filter ${B_REFSW_ARCH}, mipsel-linux ), ${B_REFSW_ARCH})
B_REFSW_CROSS_COMPILE ?= mipsel-linux-
B_REFSW_LINUX_ARCH=mips
endif
endif
endif

ifdef B_REFSW_KERNEL_CROSS_COMPILE
B_REFSW_CROSS_COMPILE := $(B_REFSW_KERNEL_CROSS_COMPILE)
endif

ifdef DEBUG
B_REFSW_DEBUG ?= $(DEBUG)
endif
ifeq ($(B_REFSW_DEBUG),)
B_REFSW_DEBUG=y
endif

B_REFSW_OBJ_DIR ?= obj.${PLATFORM}
B_REFSW_OBJ_ROOT ?= ${B_REFSW_ROOT}/${B_REFSW_OBJ_DIR}
BCM_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/connectivity/wlan/wakeonwlan
BCM_OBJ_PATH := ${BCM_OBJ_ROOT}/$(B_REFSW_ARCH)
BCM_NEXUS_BIN_PATH := ${B_REFSW_OBJ_ROOT}/nexus/bin
obj-m += wowl-plat.o

ifeq ($(ANDROID_BUILD),y)
ccflags-y += -DB_REFSW_ANDROID
endif

wakeup_drv-objs := wowl-plat.o

all: $(BCM_OBJ_PATH) $(BCM_NEXUS_BIN_PATH)
	${Q_}${MAKE} -C $(LINUX_OUT) M=$(BCM_OBJ_PATH) modules srctree=$(LINUX) ARCH=$(B_REFSW_LINUX_ARCH) CROSS_COMPILE=$(B_REFSW_CROSS_COMPILE)
	${Q_}cp -f $(BCM_OBJ_PATH)/*.ko $(BCM_NEXUS_BIN_PATH)

clean: $(BCM_OBJ_PATH)
	${Q_}${MAKE} -C $(LINUX_OUT) M=$(BCM_OBJ_PATH) clean
	${Q_}rm $(BCM_OBJ_PATH)/*.c $(BCM_OBJ_PATH)/*.h $(BCM_OBJ_PATH)/Makefile
	${Q_}rm $(BCM_NEXUS_BIN_PATH)/wowl*

.PHONY: $(BCM_OBJ_PATH)

$(BCM_OBJ_PATH)/exists:
	${Q_}mkdir -p $@

$(BCM_OBJ_PATH): $(BCM_OBJ_PATH)/exists
	${Q_}cp -f * $@

$(BCM_NEXUS_BIN_PATH):
	${Q_}mkdir -p $@
