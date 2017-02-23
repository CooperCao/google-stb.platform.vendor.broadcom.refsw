############################################################
# Copyright (C) 2015 Broadcom
#
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, you may obtain a copy at
# http://www.broadcom.com/licenses/LGPLv2.1.php or by writing to the Free
# Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301  USA
############################################################

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
BCM_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/linux/driver/wakeup
BCM_OBJ_PATH := ${BCM_OBJ_ROOT}/$(B_REFSW_ARCH)

obj-m += wakeup_drv.o

ifeq ($(ANDROID_BUILD),y)
ccflags-y += -DB_REFSW_ANDROID
endif

wakeup_drv-objs := wakeup_driver.o

all: $(BCM_OBJ_PATH)
	${Q_}${MAKE} -C $(LINUX_OUT) M=$(BCM_OBJ_PATH) modules srctree=$(LINUX) ARCH=$(B_REFSW_LINUX_ARCH) CROSS_COMPILE=$(B_REFSW_CROSS_COMPILE)

clean: $(BCM_OBJ_PATH)
	${Q_}${MAKE} -C $(LINUX_OUT) M=$(BCM_OBJ_PATH) clean
	${Q_}rm $(BCM_OBJ_PATH)/*.c $(BCM_OBJ_PATH)/*.h $(BCM_OBJ_PATH)/Makefile

.PHONY: $(BCM_OBJ_PATH)

$(BCM_OBJ_PATH)/exists:
	${Q_}mkdir -p $@

$(BCM_OBJ_PATH): $(BCM_OBJ_PATH)/exists
	${Q_}cp -f * $@
