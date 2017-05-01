############################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom.
# All rights reserved.
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
BCM_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/linux/driver/brcmv3d
BCM_OBJ_PATH := ${BCM_OBJ_ROOT}/$(B_REFSW_ARCH)

CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
          else if [ -x /bin/bash ]; then echo /bin/bash; \
          else echo sh; fi ; fi)

all: $(BCM_OBJ_PATH)
	${Q_}${MAKE} -C $(LINUX_OUT) M=$(BCM_OBJ_PATH) modules ARCH=$(B_REFSW_LINUX_ARCH) CROSS_COMPILE=$(B_REFSW_CROSS_COMPILE)

clean: $(BCM_OBJ_PATH)
	${Q_}${MAKE} -C $(LINUX_OUT) M=$(BCM_OBJ_PATH) ARCH=$(B_REFSW_LINUX_ARCH) clean
	${Q_}rm -rf $(BCM_OBJ_PATH)/*.c $(BCM_OBJ_PATH)/*.h $(BCM_OBJ_PATH)/Makefile $(BCM_OBJ_PATH)/include

.PHONY: $(BCM_OBJ_PATH)

$(BCM_OBJ_PATH)/exists:
	${Q_}mkdir -p $@

$(BCM_OBJ_PATH): $(BCM_OBJ_PATH)/exists
	${Q_}cp -f *.[ch] $@
	${Q_}cp -a include $@
	${Q_}cp -f Makefile.kbuild $@/Makefile
