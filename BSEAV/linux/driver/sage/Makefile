#############################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#  #
#  This program is the proprietary software of Broadcom and/or its licensors,
#  #  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  #  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  #  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  #  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#  #
#  Except as expressly set forth in the Authorized License,
#  #
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  #  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#  #
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  #  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  #  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  #  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  #  USE OR PERFORMANCE OF THE SOFTWARE.
#
##  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  #  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  #  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  #  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#  ############################################################

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
SAGE_DRIVER_EXTRACFLAGS += -DB_REFSW_ANDROID
endif

ifdef DEBUG
B_REFSW_DEBUG ?= $(DEBUG)
endif
ifeq ($(B_REFSW_DEBUG),)
B_REFSW_DEBUG=y
endif

B_REFSW_OBJ_DIR ?= obj.${NEXUS_PLATFORM}
B_REFSW_OBJ_ROOT ?= ${B_REFSW_ROOT}/${B_REFSW_OBJ_DIR}
BCM_OBJ_ROOT := ${B_REFSW_OBJ_ROOT}/BSEAV/linux/driver/sage
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

vpath %.c ./

SAGE_DRIVER := $(BCM_OBJ_PATH)/sage.ko
SAGE_DRIVER_CFLAGS += -DKBUILD_MODNAME=sage -DLINUX -D__KERNEL__ -DMODULE
SAGE_DRIVER_CFLAGS += -I$(LINUX)/include -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include

ifneq ($(B_REFSW_LINUX_ARCH),arm64)
SAGE_DRIVER_CFLAGS += -mlong-calls
endif

ifdef NEXUS_PLATFORM_VERSION_MAJOR
SAGE_DRIVER_CFLAGS += -DNEXUS_VERSION_MAJOR=$(NEXUS_PLATFORM_VERSION_MAJOR) -DNEXUS_VERSION_MINOR=$(NEXUS_PLATFORM_VERSION_MINOR)
endif
ifeq ($(LINUX_VER_GE_3),y)
SAGE_DRIVER_CFLAGS += -include linux/kconfig.h
else
ifeq ($(LINUX_VER_GE_2_6_37),y)
SAGE_DRIVER_CFLAGS += -include generated/autoconf.h
else
SAGE_DRIVER_CFLAGS += -include linux/autoconf.h
endif
endif

SAGE_DRIVER_SRCS += \
	sage.c \
	divdi3.c \
	moddi3.c \
	qdivrem.c \
	udivdi3.c \
	umoddi3.c

ifeq ($(filter ${B_REFSW_LINUX_ARCH}, arm arm64), ${B_REFSW_LINUX_ARCH})
SAGE_DRIVER_CFLAGS += -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include \
                     -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include/generated \
                     -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include/uapi \
                     -I$(LINUX)/arch/$(B_REFSW_LINUX_ARCH)/include/generated/uapi \
                     -I$(LINUX)/include/uapi -I$(LINUX)/include/generated/uapi

ifeq ($(B_REFSW_LINUX_ARCH),arm)
SAGE_DRIVER_SRCS_AS = bcmdriver_arm_v7.S
endif
ifeq ($(B_REFSW_LINUX_ARCH),arm64)
SAGE_DRIVER_SRCS_AS = bcmdriver_arm_v8.S
endif

SAGE_DRIVER_SRCS_AS += nexus_driver_arm_eabi.S

BRCMSTB_MEGA_BARRIER := $(shell grep -o 'brcmstb_mega_barrier' ${LINUX}/Module.symvers)
ifeq ($(BRCMSTB_MEGA_BARRIER),brcmstb_mega_barrier)
SAGE_DRIVER_CFLAGS += -DUSE_BRCMSTB_MEGA_BARRIER
endif

endif

B_REFSW_ROOT := $(realpath ${CURDIR}/../../../..)
MAGNUM=${B_REFSW_ROOT}/magnum
B_REFSW_OS:=linuxkernel
include ${B_REFSW_ROOT}/nexus/platforms/common/build/nexus_platforms.inc

BCHP_DEFINES := BCHP_CHIP=$(BCHP_CHIP) BCHP_VER=BCHP_VER_$(BCHP_VER)
BHSM_BSECK=ON

include $(MAGNUM)/basemodules/chp/bchp.inc
include $(MAGNUM)/basemodules/dbg/bdbg.inc
include $(MAGNUM)/basemodules/err/berr.inc
include $(MAGNUM)/basemodules/int/bint.inc
include $(MAGNUM)/basemodules/kni/bkni.inc
include $(MAGNUM)/basemodules/reg/breg.inc
include $(MAGNUM)/basemodules/std/bstd.inc
include $(MAGNUM)/basemodules/tmr/btmr.inc
include $(MAGNUM)/basemodules/mem/bmem.inc
include $(MAGNUM)/commonutils/lst/blst.inc
include $(MAGNUM)/commonutils/vlc/bvlc.inc
include $(MAGNUM)/portinginterface/hsm/bhsm.inc
include $(MAGNUM)/syslib/sagelib/bsage.inc

SAGE_DRIVER_INCLUDES += $(addprefix -I, $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES)))
SAGE_DRIVER_CFLAGS += $(addprefix -D, $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES)))
SAGE_DRIVER_SRCS += $(foreach module, $(MAGNUM_MODULES), $($(module)_SOURCES))
SAGE_DRIVER_CFLAGS += $(SAGE_DRIVER_INCLUDES) -DBDBG_DEBUG_BUILD=1 -DBCHP_DEBUG_OVERFLOW=0 -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE -DBSAGELIB_HARDEN_MMAP=0

SAGE_DRIVER_INCLUDES += -I${B_REFSW_ROOT}/nexus/platforms/common/src/support/libgcc
SAGE_DRIVER_INCLUDES += -I${B_REFSW_ROOT}/BSEAV/linux/driver/usermode
SAGE_DRIVER_INCLUDES += -I${B_REFSW_ROOT}/nexus/platforms/common/src/linuxkernel

vpath %.c $(dir $(SAGE_DRIVER_SRCS))
vpath %.c ${B_REFSW_ROOT}/BSEAV/linux/driver/usermode
vpath %.S ${B_REFSW_ROOT}/BSEAV/linux/driver/usermode
vpath %.S ${B_REFSW_ROOT}/nexus/platforms/common/src/linuxkernel
vpath %.c ${B_REFSW_ROOT}/nexus/platforms/common/src/support/libgcc

SAGE_DRIVER_CFLAGS += -I$(B_REFSW_ROOT)/magnum/basemodules/mem/
SAGE_DRIVER_CFLAGS += -I$(B_REFSW_ROOT)/magnum/commonutils/lst/
SAGE_DRIVER_CFLAGS += -I.
SAGE_DRIVER_CFLAGS += -DBCHP_VER=BCHP_VER_$(BCHP_VER)

SAGE_DRIVER_OBJS := $(patsubst %.c,%.o,$(notdir $(SAGE_DRIVER_SRCS)))
SAGE_DRIVER_OBJS := $(addprefix $(BCM_OBJ_PATH)/,$(SAGE_DRIVER_OBJS))
SAGE_DRIVER_OBJS_AS := $(patsubst %.S,%.o,$(SAGE_DRIVER_SRCS_AS))
SAGE_DRIVER_OBJS_AS := $(addprefix $(BCM_OBJ_PATH)/,$(SAGE_DRIVER_OBJS_AS))

all: ${SAGE_DRIVER}

ifndef BCM_KBUILD_CFLAGS
${SAGE_DRIVER}: ${SAGE_DRIVER_SRCS} $(BCM_OBJ_PATH)/Kbuild_flags
	$(eval $(shell grep -m 1 -e "BCM_KBUILD_CFLAGS:=" $(BCM_OBJ_PATH)/Kbuild_flags))
	$(eval $(shell grep -m 1 -e "BCM_KBUILD_LDFLAGS:=" $(BCM_OBJ_PATH)/Kbuild_flags))
	@[ -n "${BCM_KBUILD_CFLAGS}" ] || (echo "can't obtain BCM_KBUILD_CFLAGS";exit 1)
	${Q_}${MAKE} BCM_KBUILD_CFLAGS='${BCM_KBUILD_CFLAGS}' BCM_KBUILD_LDFLAGS='${BCM_KBUILD_LDFLAGS}'


else
$(SAGE_DRIVER): $(SAGE_DRIVER_OBJS) $(SAGE_DRIVER_OBJS_AS)
	@echo [Link...... sage.ko]
ifeq ($(LINUX_VER_GE_2_6_31),y)
	$(Q_)cd ${LINUX} && $(LD) -Map $(SAGE_DRIVER).map $(SAGE_DRIVER_LDFLAGS) $(BCM_KBUILD_LDFLAGS) $(SAGE_DRIVER_EXTRALDFLAGS) --strip-debug --build-id=none -r $^  -o $(BCM_OBJ_PATH)/sage.o_shipped
	${Q_}$(MAKE) -C $(LINUX) M=$(BCM_OBJ_PATH) ARCH=$(B_REFSW_LINUX_ARCH) modules PLATFORM=$(NEXUS_PLATFORM) CROSS_COMPILE=$(B_REFSW_CROSS_COMPILE)
else
	@# do not use kbuild for 2.6.18
	$(Q_)$(LD) -Map $(SAGE_DRIVER).map $(SAGE_DRIVER_LDFLAGS) $(BCM_KBUILD_LDFLAGS) $(SAGE_DRIVER_EXTRALDFLAGS) --strip-debug -r $^  -o $@
endif
	@# disable MODVERSIONS if kernel symvers are missing
	@if [ -s $(LINUX)/Modules.symvers ]; then \
		$(OBJCOPY) --remove-section __versions $(BCM_OBJ_PATH)/sage.ko ; \
	fi
endif # BCM_KBUILD_CFLAGS

$(BCM_OBJ_PATH)/%.o: %.c
	@echo [Compile... $(notdir $<)];
	$(Q_)$(CC) $(SAGE_DRIVER_CFLAGS) $(BCM_KBUILD_CFLAGS) $(SAGE_DRIVER_EXTRACFLAGS) -o $@ -c $< -o $(BCM_OBJ_PATH)/$(notdir $@)

$(BCM_OBJ_PATH)/%.o: %.S
	@echo [Compile... $<];
	$(Q_)$(CC) $(SAGE_DRIVER_CFLAGS) $(BCM_KBUILD_CFLAGS) $(SAGE_DRIVER_EXTRACFLAGS) -D__ASSEMBLY__ -o $@ -c $< -o $(BCM_OBJ_PATH)/$(notdir $@)


clean: $(BCM_OBJ_PATH)/Kbuild
	${Q_}$(MAKE) -C $(LINUX) M=$(BCM_OBJ_PATH) ARCH=$(B_REFSW_LINUX_ARCH) clean
	${Q_}$(RM) $(SAGE_DRIVER) $(SAGE_DRIVER).map $(BCM_OBJ_PATH)/Kbuild_flags $(BCM_OBJ_PATH)/sage.o_shipped $(SAGE_DRIVER_OBJS) $(SAGE_DRIVER_OBJS_AS)

ifeq ($(INSTALL_DIR),)
install:
	$(error INSTALL_DIR is undefined)
else
install: all
	@echo "[Install... sage.ko]"
	${Q_}$(CP) $(BCM_OBJ_PATH)/sage.ko $(INSTALL_DIR)
endif

$(BCM_OBJ_PATH)/Kbuild: Kbuild $(BCM_OBJ_PATH)/exists
	@echo [Copying... Kbuild];
	$(Q_)$(CP) $< $@

$(BCM_OBJ_PATH)/exists:
	${Q_}mkdir -p $@

.PHONY: $(BCM_OBJ_PATH)/Kbuild_flags

$(BCM_OBJ_PATH)/Kbuild_flags : $(BCM_OBJ_PATH)/Kbuild
	${Q_}$(MAKE) -C $(LINUX) M=$(BCM_OBJ_PATH) modules ARCH=$(B_REFSW_LINUX_ARCH) PLATFORM=$(NEXUS_PLATFORM) CROSS_COMPILE=$(B_REFSW_CROSS_COMPILE) BCM_PRINT_KBUILD_ENV_ONLY=1 2>$@ >/dev/null
