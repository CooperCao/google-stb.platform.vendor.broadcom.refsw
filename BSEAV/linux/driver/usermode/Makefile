#############################################################################
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#############################################################################

CWD := $(shell pwd)
B_REFSW_ROOT := ${CWD}/../../../..

ifneq ($(B_REFSW_VERBOSE),)
Q_:=
else
Q_:=@
MAKEFLAGS += --no-print-directory
endif

# If NEXUS_PLATFORM not defined, populate it from PLATFORM
NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
    $(error NEXUS_PLATFORM is not defined)
endif

ifeq ($(LINUX),)
LINUX = /opt/brcm/linux
endif

NEXUS_GISB_ARB := $(shell if grep 'CONFIG_BRCMSTB_GISB_ARB' -q ${LINUX}/include/generated/autoconf.h ; then echo "y" ; fi)
NEXUS_WKTMR    := $(shell if grep 'CONFIG_BRCMSTB_WKTMR' -q ${LINUX}/include/generated/autoconf.h ; then echo "y" ; fi)

ifeq ($(B_REFSW_ARCH),)
B_REFSW_ARCH=mipsel-linux
endif

# NOTE: $LINUX/Makefile defaults ARCH=mips. It cannot be set to mipsel-linux here.
ifeq ($(filter ${B_REFSW_ARCH}, mipsel-linux mipsel-uclibc mipsel-linux-uclibc mipsel-linux-android), ${B_REFSW_ARCH})
B_REFSW_LINUX_ARCH=mips
B_REFSW_CROSS_COMPILE ?= mipsel-linux-
else
ifeq ($(filter ${B_REFSW_ARCH}, mips-linux mips-uclibc mips-linux-uclibc mips-linux-android), ${B_REFSW_ARCH})
B_REFSW_LINUX_ARCH=mips
B_REFSW_CROSS_COMPILE ?= mips-linux-
else
ifeq ($(filter ${B_REFSW_ARCH}, arm-linux ), ${B_REFSW_ARCH})
B_REFSW_LINUX_ARCH=arm
B_REFSW_CROSS_COMPILE ?= arm-linux-
endif
ifeq ($(filter ${B_REFSW_ARCH}, aarch64-linux ), ${B_REFSW_ARCH})
B_REFSW_LINUX_ARCH=arm64
B_REFSW_CROSS_COMPILE ?= aarch64-linux-
endif
endif
endif

ifeq ($(ANDROID_BUILD),y)
ifdef B_REFSW_KERNEL_CROSS_COMPILE
B_REFSW_CROSS_COMPILE := $(B_REFSW_KERNEL_CROSS_COMPILE)
endif
BCM_DRIVER_EXTRACFLAGS += -DB_REFSW_ANDROID
endif

ifdef DEBUG
B_REFSW_DEBUG ?= $(DEBUG)
endif
ifeq ($(B_REFSW_DEBUG),)
B_REFSW_DEBUG=y
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${B_REFSW_ROOT}/${B_REFSW_OBJ_DIR}
BCM_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/linux/driver/usermode
BCM_OBJ_PATH := ${BCM_OBJ_ROOT}/$(B_REFSW_ARCH)


# tools
CP = cp -f
CC = $(B_REFSW_CROSS_COMPILE)gcc
AS = $(B_REFSW_CROSS_COMPILE)as
LD = $(B_REFSW_CROSS_COMPILE)ld
OBJCOPY = $(B_REFSW_CROSS_COMPILE)objcopy

# parse the Linux Makefile
LINUX_VERSION := $(shell grep -m 1 '^VERSION = ' ${LINUX}/Makefile | awk '{print $$3}')
LINUX_PATCHLEVEL := $(shell grep -m 1 '^PATCHLEVEL = ' ${LINUX}/Makefile | awk '{print $$3}')
LINUX_SUBLEVEL := $(shell grep -m 1 '^SUBLEVEL = ' ${LINUX}/Makefile | awk '{print $$3}')

LINUX_VER_GE_3 :=  $(shell test $(LINUX_VERSION) -ge 3 && echo y)
ifeq ($(LINUX_VER_GE_3),y)
	LINUX_VER_GE_2_6 = y
	LINUX_VER_GE_2_6_31 = y
	LINUX_VER_GE_2_6_37 = y
else
LINUX_VER_GE_2_6 ?= $(shell test $(LINUX_PATCHLEVEL) -eq 6 && echo 'y')
ifeq ($(LINUX_VER_GE_2_6),y)
LINUX_VER_GE_2_6_31 :=  $(shell test $(LINUX_SUBLEVEL) -ge 31 && echo y)
LINUX_VER_GE_2_6_37 :=  $(shell test $(LINUX_SUBLEVEL) -ge 37 && echo y)
LINUX_VER_EQ_2_6_12 := $(shell test $(LINUX_SUBLEVEL) -eq 12 && echo 'y')
endif
endif

LINUX_VER_GE_4 := $(shell test $(LINUX_VERSION) -ge 4 && echo y)
ifeq ($(LINUX_VER_GE_4),y)
INTERRUPT_TABLE_SRC_PATH =
else
INTERRUPT_TABLE_SRC_PATH = ./${NEXUS_PLATFORM}
endif

vpath %.c ./ ${INTERRUPT_TABLE_SRC_PATH}
vpath %.c ${BCM_OBJ_ROOT}/interrupt_table

BCM_DRIVER := $(BCM_OBJ_PATH)/bcmdriver.ko
BCM_DRIVER_CFLAGS += -DKBUILD_MODNAME=bcmdriver -DLINUX -D__KERNEL__ -DMODULE
BCM_DRIVER_CFLAGS += -I$(LINUX)/include -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include

ifneq ($(B_REFSW_LINUX_ARCH),arm64)
BCM_DRIVER_CFLAGS += -mlong-calls
endif

ifdef NEXUS_PLATFORM_VERSION_MAJOR
BCM_DRIVER_CFLAGS += -DNEXUS_VERSION_MAJOR=$(NEXUS_PLATFORM_VERSION_MAJOR) -DNEXUS_VERSION_MINOR=$(NEXUS_PLATFORM_VERSION_MINOR)
endif
ifeq ($(NEXUS_CACHEFLUSHALL),y)
BCM_DRIVER_CFLAGS += -DNEXUS_CACHEFLUSHALL=1
endif
ifeq ($(LINUX_VER_GE_3),y)
BCM_DRIVER_CFLAGS += -include linux/kconfig.h
else
ifeq ($(LINUX_VER_GE_2_6_37),y)
BCM_DRIVER_CFLAGS += -include generated/autoconf.h
else
BCM_DRIVER_CFLAGS += -include linux/autoconf.h
endif
endif

BCM_DRIVER_SRCS += \
	interrupt_table.c \
	bcm_driver.c

ifeq ($(filter ${B_REFSW_LINUX_ARCH}, arm arm64 mips), ${B_REFSW_LINUX_ARCH})
BCM_DRIVER_CFLAGS += -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include \
                     -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include/generated \
                     -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include/uapi \
                     -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include/generated/uapi \
                     -I$(LINUX)/include/uapi -I$(LINUX)/include/generated/uapi
endif

ifeq ($(B_REFSW_LINUX_ARCH),arm)
BCM_DRIVER_SRCS_AS = bcmdriver_arm_v7.S
endif
ifeq ($(B_REFSW_LINUX_ARCH),arm64)
BCM_DRIVER_SRCS_AS = bcmdriver_arm_v8.S
endif

ifeq ($(filter ${B_REFSW_LINUX_ARCH}, arm arm64), ${B_REFSW_LINUX_ARCH})
BRCMSTB_MEGA_BARRIER := $(shell grep -o 'brcmstb_mega_barrier' ${LINUX}/Module.symvers)
ifeq ($(BRCMSTB_MEGA_BARRIER),brcmstb_mega_barrier)
BCM_DRIVER_CFLAGS += -DUSE_BRCMSTB_MEGA_BARRIER
endif
endif

ICM_CLI_SUPPORT ?= y
ifeq ($(ICM_CLI_SUPPORT),y)
BCM_DRIVER_CFLAGS += -DBICM_ICAM2
endif

ifeq ($(B_VIRTUAL_IRQ_DISABLED),y)
BCM_DRIVER_CFLAGS += -DB_VIRTUAL_IRQ_DISABLED=1
endif

BCM_DRIVER_CFLAGS += -I$(B_REFSW_ROOT)/magnum/commonutils/lst/
BCM_DRIVER_CFLAGS += -I.

# compile bcmdriver the same for webcpu servers. client gets just m2mc1.
ifeq ($(NEXUS_WEBCPU),core1_server)
BCM_DRIVER_CFLAGS += -DNEXUS_WEBCPU
endif

BCM_DRIVER_OBJS := $(patsubst %.c,%.o,$(BCM_DRIVER_SRCS))
BCM_DRIVER_OBJS := $(addprefix $(BCM_OBJ_PATH)/,$(BCM_DRIVER_OBJS))
BCM_DRIVER_OBJS_AS := $(patsubst %.S,%.o,$(BCM_DRIVER_SRCS_AS))
BCM_DRIVER_OBJS_AS := $(addprefix $(BCM_OBJ_PATH)/,$(BCM_DRIVER_OBJS_AS))

all: ${BCM_DRIVER}

ifndef BCM_KBUILD_CFLAGS
${BCM_DRIVER}: ${BCM_DRIVER_SRCS} $(BCM_OBJ_PATH)/Kbuild_flags
	$(eval $(shell grep -m 1 -e "BCM_KBUILD_CFLAGS:=" $(BCM_OBJ_PATH)/Kbuild_flags))
	@[ -n "${BCM_KBUILD_CFLAGS}" ] || (echo "can't obtain BCM_KBUILD_CFLAGS";exit 1)
	${Q_}${MAKE} BCM_KBUILD_CFLAGS='${BCM_KBUILD_CFLAGS}'


else
$(BCM_DRIVER): $(BCM_DRIVER_OBJS) $(BCM_DRIVER_OBJS_AS)
	@echo [Link...... bcmdriver.ko]
ifeq ($(LINUX_VER_GE_2_6_31),y)
	$(Q_)cd ${LINUX} && $(LD) -Map $(BCM_DRIVER).map $(BCM_DRIVER_LDFLAGS) $(BCM_DRIVER_EXTRALDFLAGS) --strip-debug --build-id=none -r $^  -o $(BCM_OBJ_PATH)/bcmdriver.o_shipped
	${Q_}$(MAKE) -C $(LINUX) M=$(BCM_OBJ_PATH) modules srctree=$(LINUX) ARCH=$(B_REFSW_LINUX_ARCH) PLATFORM=$(NEXUS_PLATFORM) CROSS_COMPILE=$(B_REFSW_CROSS_COMPILE)
else
	@# do not use kbuild for 2.6.18
	$(Q_)$(LD) -Map $(BCM_DRIVER).map $(BCM_DRIVER_LDFLAGS) $(BCM_DRIVER_EXTRALDFLAGS) --strip-debug -r $^  -o $@
endif
	@# disable MODVERSIONS if kernel symvers are missing
	@if [ -s $(LINUX)/Modules.symvers ]; then \
		$(OBJCOPY) --remove-section __versions $(BCM_OBJ_PATH)/bcmdriver.ko ; \
	fi
endif # BCM_KBUILD_CFLAGS

$(BCM_OBJ_PATH)/%.o: %.c
	@echo [Compile... $(notdir $<)];
	$(Q_)$(CC) $(BCM_DRIVER_CFLAGS) $(BCM_KBUILD_CFLAGS) $(BCM_DRIVER_EXTRACFLAGS) -o $@ -c $<

$(BCM_OBJ_PATH)/%.o: %.S
	@echo [Compile... $<];
	$(Q_)$(CC) $(BCM_DRIVER_CFLAGS) $(BCM_KBUILD_CFLAGS) $(BCM_DRIVER_EXTRACFLAGS) -D__ASSEMBLY__ -o $@ -c $<


clean: $(BCM_OBJ_PATH)/Kbuild
	${Q_}$(MAKE) -C interrupt_table clean
	${Q_}$(MAKE) -C $(LINUX) M=$(BCM_OBJ_PATH) ARCH=$(B_REFSW_LINUX_ARCH) clean
	${Q_}$(RM) $(BCM_DRIVER) $(BCM_DRIVER).map $(BCM_OBJ_PATH)/Kbuild $(BCM_OBJ_PATH)/Kbuild_flags $(BCM_OBJ_PATH)/bcmdriver.o_shipped $(BCM_DRIVER_OBJS) $(BCM_DRIVER_OBJS_AS)

ifeq ($(INSTALL_DIR),)
install:
	$(error INSTALL_DIR is undefined)
else
install: all
	@echo "[Install... bcmdriver.ko to $(realpath $(INSTALL_DIR))]"
	${Q_}$(CP) $(BCM_OBJ_PATH)/bcmdriver.ko $(INSTALL_DIR)
endif

$(BCM_OBJ_PATH)/Kbuild: Kbuild $(BCM_OBJ_PATH)/exists
	@echo [Copying... Kbuild];
	$(Q_)$(CP) $< $@

$(BCM_OBJ_PATH)/exists:
	${Q_}mkdir -p $@

.PHONY: $(BCM_OBJ_PATH)/Kbuild_flags

$(BCM_OBJ_PATH)/Kbuild_flags : $(BCM_OBJ_PATH)/Kbuild
	${Q_}$(MAKE) -C $(LINUX) M=$(BCM_OBJ_PATH) modules srctree=$(LINUX) ARCH=$(B_REFSW_LINUX_ARCH) PLATFORM=$(NEXUS_PLATFORM) CROSS_COMPILE=$(B_REFSW_CROSS_COMPILE) BCM_PRINT_KBUILD_ENV_ONLY=1 2>$@ >/dev/null


$(BCM_OBJ_ROOT)/interrupt_table/exists:
	${Q_}mkdir -p $@

interrupt_table.c:$(BCM_OBJ_ROOT)/interrupt_table/exists

${BCM_OBJ_ROOT}/interrupt_table/interrupt_table.c:
	${Q_}${MAKE} -C interrupt_table BREG_CAPTURE= NEXUS_GISB_ARB=$(NEXUS_GISB_ARB)
