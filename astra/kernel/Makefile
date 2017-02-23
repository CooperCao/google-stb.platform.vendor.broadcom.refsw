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

# include default defines
include default.inc

# directories and files
TZ_IMG := tz
TZ_LIB := libkernel

MODULE_DIRS := \
	arch/$(ARCH_DIR) \
	arch/$(ARCH_DIR)/timer \
	arch/$(ARCH_DIR)/gic \
	boot \
	boot/arch/$(ARCH_DIR) \
	memory \
	memory/arch/$(ARCH_DIR) \
	interrupts \
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
	3rd_party/libfdt \
	console

SRC_DIRS := $(MODULE_DIRS)
INC_DIRS := $(MODULE_DIRS)
BIN_DIRS := $(addprefix $(B_REFSW_OBJ_ROOT)/bin/,$(MODULE_DIRS))

SOURCES_ASM-$(CFG_ARM32_core) := $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*_a32.S))
SOURCES_ASM-$(CFG_ARM64_core) := $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*_a64.S))
OBJS        := $(patsubst %.S,%.o,$(SOURCES_ASM-y))
DEPS        := $(patsubst %.S,%.d,$(SOURCES_ASM-y))

SOURCES_C   := $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.c))
OBJS        += $(patsubst %.c,%.o,$(SOURCES_C))
DEPS        += $(patsubst %.c,%.d,$(SOURCES_C))

SOURCES_CPP := $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.cpp))
OBJS        += $(patsubst %.cpp,%.o,$(SOURCES_CPP))
DEPS        += $(patsubst %.cpp,%.d,$(SOURCES_CPP))

# use local init module (to build image)
INIT_DIRS := init
INC_DIRS += init
BIN_DIRS += $(B_REFSW_OBJ_ROOT)/bin/init

INIT_SOURCES_ASM-$(CFG_ARM32_core) := $(foreach sdir,$(INIT_DIRS),$(wildcard $(sdir)/*_a32.S))
INIT_SOURCES_ASM-$(CFG_ARM64_core) := $(foreach sdir,$(INIT_DIRS),$(wildcard $(sdir)/*_a64.S))
INIT_OBJS        := $(patsubst %.S,%.o,$(INIT_SOURCES_ASM-y))
INIT_DEPS        := $(patsubst %.S,%.d,$(INIT_SOURCES_ASM-y))

INIT_SOURCES_C   := $(foreach sdir,$(INIT_DIRS),$(wildcard $(sdir)/*.c))
INIT_OBJS        += $(patsubst %.c,%.o,$(INIT_SOURCES_C))
INIT_DEPS        += $(patsubst %.c,%.d,$(INIT_SOURCES_C))

INIT_SOURCES_CPP := $(foreach sdir,$(INIT_DIRS),$(wildcard $(sdir)/*.cpp))
INIT_OBJS        += $(patsubst %.cpp,%.o,$(INIT_SOURCES_CPP))
INIT_DEPS        += $(patsubst %.cpp,%.d,$(INIT_SOURCES_CPP))

# Create out of build (OOB) objects
OOB_OBJS			:= $(patsubst %.o, $(B_REFSW_OBJ_ROOT)/bin/%.o,$(OBJS))
OOB_DEPS			:= $(patsubst %.d, $(B_REFSW_OBJ_ROOT)/bin/%.d,$(DEPS))
OOB_INIT_OBJS		:= $(patsubst %.o, $(B_REFSW_OBJ_ROOT)/bin/%.o,$(INIT_OBJS))
OOB_INIT_DEPS		:= $(patsubst %.d, $(B_REFSW_OBJ_ROOT)/bin/%.d,$(INIT_DEPS))

# build flags
DEPENDFLAGS := -MD -MP

INCLUDES    := -I. $(addprefix -I,$(INC_DIRS))
INCLUDES    += -Iinclude
INCLUDES    += -Iinclude/arm
INCLUDES    += -Iapi
INCLUDES    += -Iapi/arch/$(ARCH_DIR)
INCLUDES    += -I3rd_party/libfdt
INCLUDES    += -I$(TZ_DIR)/common/include
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

ASFLAGS     := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
ASFLAGS     += -g -D__ASSEMBLY__

CFLAGS      := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
CFLAGS      += $(BASEFLAGS) $(WARNFLAGS)
CFLAGS      += -std=gnu99 -Wimplicit-function-declaration

CPPFLAGS    := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
CPPFLAGS    += $(BASEFLAGS) $(WARNFLAGS)
CPPFLAGS    += -std=c++11 -D__EXCEPTIONS=0
CPPFLAGS    += -fno-rtti -fno-exceptions -fno-use-cxa-atexit

vpath %.S   $(SRC_DIRS) $(INIT_DIRS)
vpath %.c   $(SRC_DIRS) $(INIT_DIRS)
vpath %.cpp $(SRC_DIRS) $(INIT_DIRS)

define make-goal
$1/%.o: %.c
	$(TZ_ARMGNU)-gcc $(CFLAGS) -c $$< -o $$@
$1/%.o: %.S
	$(TZ_ARMGNU)-gcc $(ASFLAGS) -c $$< -o $$@
$1/%.o: %.cpp
	$(TZ_ARMGNU)-g++ $(CPPFLAGS) -c $$< -o $$@
endef

# build rules
.PHONY: checkplatform checkdirs $(TZ_IMG)

all: checkplatform checkdirs $(TZ_IMG)

-include $(OOB_DEPS) $(OOB_INIT_DEPS)

checkplatform:
	@echo Building for TZ_PLATFORM=$(TZ_PLATFORM)...

checkdirs: $(BIN_DIRS)

$(BIN_DIRS):
	mkdir -p $@

$(TZ_IMG): $(B_REFSW_OBJ_ROOT)/bin/$(TZ_IMG).bin

$(B_REFSW_OBJ_ROOT)/bin/$(TZ_IMG).bin: $(B_REFSW_OBJ_ROOT)/bin/$(TZ_IMG).elf
	$(TZ_ARMGNU)-objcopy $(B_REFSW_OBJ_ROOT)/bin/$(TZ_IMG).elf -O binary $(B_REFSW_OBJ_ROOT)/bin/$(TZ_IMG).bin

$(B_REFSW_OBJ_ROOT)/bin/$(TZ_IMG).elf: $(B_REFSW_OBJ_ROOT)/bin/$(TZ_LIB).a $(OOB_INIT_OBJS) link_$(ARCH)_tz.ld
	$(TZ_ARMGNU)-g++ -nostartfiles -static $(OOB_INIT_OBJS) $(B_REFSW_OBJ_ROOT)/bin/$(TZ_LIB).a -Tlink_$(ARCH)_tz.ld -o $@

$(B_REFSW_OBJ_ROOT)/bin/$(TZ_LIB).a: $(OOB_OBJS)
	$(TZ_ARMGNU)-ar -cr $@ $(OOB_OBJS)

clean:
	$(RM) -r $(B_REFSW_OBJ_ROOT)/bin

.PHONY:unit_tests

unit_tests:
	$(MAKE) -C unittests/cpuload
	$(MAKE) -C unittests/fs_userspace
	$(MAKE) -C unittests/memory
	$(MAKE) -C unittests/musl_userspace
	$(MAKE) -C unittests/pthread
	$(MAKE) -C unittests/ramfs
	$(MAKE) -C unittests/signals
	$(MAKE) -C unittests/timer
	$(MAKE) -C unittests/userspace

clean_unit_tests:
	$(MAKE) -C unittests/cpuload clean
	$(MAKE) -C unittests/fs_userspace clean
	$(MAKE) -C unittests/memory clean
	$(MAKE) -C unittests/musl_userspace clean
	$(MAKE) -C unittests/pthread clean
	$(MAKE) -C unittests/ramfs clean
	$(MAKE) -C unittests/signals clean
	$(MAKE) -C unittests/timer clean
	$(MAKE) -C unittests/userspace clean


$(foreach bdir,$(BIN_DIRS),$(eval $(call make-goal,$(bdir))))
