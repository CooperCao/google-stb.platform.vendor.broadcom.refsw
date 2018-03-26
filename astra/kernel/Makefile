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

# build env
TZ_TOP := ..
TZ_MOD := kernel
TZ_IMG := kernel
TZ_LIB := libkernel

include $(TZ_TOP)/build/build_env.mk

# build src
SUBMOD_DIRS := \
	arch/$(ARCH_DIR) \
	arch/$(ARCH_DIR)/timer \
	arch/$(ARCH_DIR)/gic \
	boot \
	boot/arch/$(ARCH_DIR) \
	memory \
	memory/arch/$(ARCH_DIR) \
	interrupts \
	interrupts/arch/$(ARCH_DIR) \
	process \
	process/arch/$(ARCH_DIR) \
	ramfs \
	syscalls \
	syscalls/arch/$(ARCH_DIR) \
	ipc \
	ioc/common \
	ioc \
	tracelog \
	crypto \
	platform/$(TZ_PLATFORM) \
	tz_kernel_libc \
	tz_kernel_libc/arch/$(ARCH_DIR) \
	3rd_party/libfdt \
	console \

SRC_DIRS := $(SUBMOD_DIRS)
INC_DIRS := $(SUBMOD_DIRS)
BIN_DIRS := $(addprefix $(TZ_OBJ_DIR)/,$(SUBMOD_DIRS))

ifeq ($(MAKECMDGOALS),$(TZ_IMG))
# use local init module (to build image)
SRC_DIRS += init
INC_DIRS += init
BIN_DIRS += $(TZ_OBJ_DIR)/init
endif

include $(TZ_TOP)/build/build_src.mk

# build flags
DEPENDFLAGS := -MD -MP

INCLUDES    := -I. $(addprefix -I,$(INC_DIRS))
INCLUDES    += -Iinclude
INCLUDES    += -Iinclude/arm
INCLUDES    += -Iapi
INCLUDES    += -Iapi/arch/$(ARCH_DIR)
INCLUDES    += -I3rd_party/libfdt
INCLUDES    += -I$(TZ_TOP)/common/include
INCLUDES    += -Iarch/$(ARCH_DIR)/include
INCLUDES    += -Iplatform/$(TZ_PLATFORM)
INCLUDES    += -I platform/$(TZ_PLATFORM)/include
INCLUDES    += -Iplatform/$(TZ_PLATFORM)/include/$(ARCH_DIR)

BASEFLAGS   := -O2 -g
BASEFLAGS   += -nostdlib
BASEFLAGS   += -nostartfiles
BASEFLAGS   += -nodefaultlibs
BASEFLAGS   += -fno-jump-tables
BASEFLAGS   += -ffreestanding
BASEFLAGS   += -fno-builtin
BASEFLAGS   += -fomit-frame-pointer

WARNFLAGS   := -Wall
WARNFLAGS   += -Wextra
WARNFLAGS   += -Wdisabled-optimization
WARNFLAGS   += -Wfloat-equal
WARNFLAGS   += -Winit-self
WARNFLAGS   += -Winline
WARNFLAGS   += -Winvalid-pch
WARNFLAGS   += -Wmissing-include-dirs
WARNFLAGS   += -Wpointer-arith
WARNFLAGS   += -Wredundant-decls
WARNFLAGS   += -Wswitch
WARNFLAGS   += -Wsystem-headers
WARNFLAGS   += -Wwrite-strings
WARNFLAGS   += -Wundef
WARNFLAGS   += -Wno-attributes
WARNFLAGS   += -Wno-deprecated-declarations
WARNFLAGS   += -Wno-div-by-zero
WARNFLAGS   += -Wno-endif-labels
WARNFLAGS   += -Wno-format-extra-args
WARNFLAGS   += -Wno-multichar
WARNFLAGS   += -Wno-pragmas
WARNFLAGS   += -Wno-sign-compare
WARNFLAGS   += -Wno-unused-result
WARNFLAGS   += -Werror

OSFLAGS     := -D__TZOS__

ASFLAGS     := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS) $(KFLAGS)
ASFLAGS     += -g -D__ASSEMBLY__

CFLAGS      := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS) $(KFLAGS)
CFLAGS      += $(BASEFLAGS) $(WARNFLAGS)
CFLAGS      += -std=gnu99 -Wimplicit-function-declaration

CPPFLAGS    := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS) $(KFLAGS)
CPPFLAGS    += $(BASEFLAGS) $(WARNFLAGS)
CPPFLAGS    += -std=c++11 -D__EXCEPTIONS=0
CPPFLAGS    += -fno-rtti -fno-exceptions -fno-use-cxa-atexit

# build rules
.PHONY: checkplatform checkdirs $(TZ_LIB) $(TZ_IMG)

all: $(TZ_LIB)

$(TZ_LIB): checkplatform checkdirs $(TZ_OBJ_DIR)/$(TZ_LIB).a

$(TZ_IMG): checkplatform checkdirs $(TZ_OBJ_DIR)/$(TZ_IMG).bin

checkplatform:
	@echo Building for TZ_PLATFORM=$(TZ_PLATFORM)...

checkdirs: $(BIN_DIRS)

$(BIN_DIRS):
	@mkdir -p $@

$(TZ_OBJ_DIR)/$(TZ_LIB).a: $(OBJS)
	@echo "[Archive... $@]"
	$(Q_)$(TZ_ARMGNU)-ar -cr $@ $(OBJS)

$(TZ_OBJ_DIR)/$(TZ_IMG).bin: $(TZ_OBJ_DIR)/$(TZ_IMG).elf
	@echo "[Create... $@]"
	$(Q_)$(TZ_ARMGNU)-objcopy $(TZ_OBJ_DIR)/$(TZ_IMG).elf -O binary $(TZ_OBJ_DIR)/$(TZ_IMG).bin

$(TZ_OBJ_DIR)/$(TZ_IMG).elf: $(OBJS) link_$(ARCH)_tz.ld
	@echo "[Link... $@]"
	$(Q_)$(TZ_ARMGNU)-g++ -nostartfiles -static $(OBJS) -Tlink_$(ARCH)_tz.ld -o $@

clean:
	$(RM) -r $(TZ_OBJ_DIR)

include $(TZ_TOP)/build/build_rules.mk

.PHONY: unittests clean_unittests

unittests:
	$(MAKE) -C unittests/timer
	$(MAKE) -C unittests/memory
	$(MAKE) -C unittests/ramfs
	$(MAKE) -C unittests/musl
	$(MAKE) -C unittests/signals
	$(MAKE) -C unittests/pthread
	$(MAKE) -C unittests/fs
	$(MAKE) -C unittests/userspace

clean_unittests:
	$(MAKE) -C unittests/timer clean
	$(MAKE) -C unittests/memory clean
	$(MAKE) -C unittests/ramfs clean
	$(MAKE) -C unittests/musl clean
	$(MAKE) -C unittests/signals clean
	$(MAKE) -C unittests/pthread clean
	$(MAKE) -C unittests/fs clean
	$(MAKE) -C unittests/userspace clean
