############################################################
# Copyright © 2014 Broadcom Corporation
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
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

ifeq ($(BCHP_CHIP),)
    ifeq ($(PLATFORM),93390)
        BCHP_CHIP := 3390
    endif

    ifeq ($(PLATFORM),97364)
        BCHP_CHIP := 7364
    endif

    ifeq ($(PLATFORM),97366)
        BCHP_CHIP := 7366
    endif
endif

ifeq ($(LINUX),)
LINUX = /opt/brcm/linux
endif

ifdef DEBUG
B_REFSW_DEBUG ?= $(DEBUG)
endif
ifeq ($(B_REFSW_DEBUG),)
B_REFSW_DEBUG=y
endif

ifdef B_REFSW_KERNEL_CROSS_COMPILE
CROSS_COMPILE := $(B_REFSW_KERNEL_CROSS_COMPILE)
endif

B_REFSW_OBJ_DIR ?= obj.${PLATFORM}
B_REFSW_OBJ_ROOT ?= ${B_REFSW_ROOT}/${B_REFSW_OBJ_DIR}
BCM_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/linux/driver/zigbee
BCM_OBJ_PATH := ${BCM_OBJ_ROOT}/$(B_REFSW_ARCH)

obj-m += zigbee_drv.o

BCHP_VER_LOWERCASE = $(shell awk 'BEGIN{print tolower("$(BCHP_VER)")}')
PWD := $(shell pwd)
ccflags-y += -I$(M)/../../../../../../magnum/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWERCASE) -DBCHP_CHIP=$(BCHP_CHIP)

ifeq ($(ANDROID_BUILD),y)
ccflags-y += -DB_REFSW_ANDROID
endif

zigbee_drv-objs := zigbee_driver.o

all: $(BCM_OBJ_PATH)
	${Q_}${MAKE} -C $(LINUX) M=$(BCM_OBJ_PATH) modules

clean: $(BCM_OBJ_PATH)
	${Q_}${MAKE} -C $(LINUX) M=$(BCM_OBJ_PATH) clean
	${Q_}rm $(BCM_OBJ_PATH)/*.c $(BCM_OBJ_PATH)/*.h $(BCM_OBJ_PATH)/Makefile

.PHONY: $(BCM_OBJ_PATH)

$(BCM_OBJ_PATH)/exists:
	${Q_}mkdir -p $@

$(BCM_OBJ_PATH): $(BCM_OBJ_PATH)/exists
	${Q_}cp -f * $@
