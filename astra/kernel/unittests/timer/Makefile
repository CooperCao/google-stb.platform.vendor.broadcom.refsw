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

# build environment
TZ_DIR ?= ../../..
include $(TZ_DIR)/kernel/default.inc

# directories and files
TZ_IMG := timer_tests

TZ_KERNEL_DIR := $(TZ_DIR)/kernel
TZ_KERNEL_LIB := $(B_REFSW_OBJ_ROOT)/bin/libkernel.a

# use local init module
SRC_DIRS := init
INC_DIRS := init
BASE_OBJ_DIR := $(B_REFSW_OBJ_ROOT)/kernel/unittests/timer
BIN_DIRS := $(BASE_OBJ_DIR)/bin/init

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

# Create out of build (OOB) objects
OOB_OBJS			:= $(patsubst %.o, $(BASE_OBJ_DIR)/bin/%.o,$(OBJS))
OOB_DEPS			:= $(patsubst %.d, $(BASE_OBJ_DIR)/bin/%.d,$(DEPS))

# build flags
DEPENDFLAGS := -MD -MP

INCLUDES    := -I. $(addprefix -I,$(INC_DIRS))
INCLUDES    += -I$(TZ_KERNEL_DIR)/api
INCLUDES    += -I$(TZ_KERNEL_DIR)/api/arch/$(ARCH_DIR)
INCLUDES    += -I$(TZ_KERNEL_DIR)/include
INCLUDES    += -I$(TZ_KERNEL_DIR)/include/arm
INCLUDES    += -I$(TZ_KERNEL_DIR)/platform/$(TZ_PLATFORM)
INCLUDES    += -I$(TZ_KERNEL_DIR)/platform/$(TZ_PLATFORM)/include/$(ARCH_DIR)
INCLUDES    += -I$(TZ_KERNEL_DIR)/3rd_party/libfdt

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
CFLAGS      += -std=gnu99

CPPFLAGS    := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
CPPFLAGS    += $(BASEFLAGS) $(WARNFLAGS)
CPPFLAGS    += -std=c++11 -D__EXCEPTIONS=0 -fno-rtti -fno-exceptions

vpath %.S   $(SRC_DIRS)
vpath %.c   $(SRC_DIRS)
vpath %.cpp $(SRC_DIRS)

define make-goal
$1/%.o: %.c
	$(TZ_ARMGNU)-gcc $(CFLAGS) -c $$< -o $$@
$1/%.o: %.S
	$(TZ_ARMGNU)-gcc $(ASFLAGS) -c $$< -o $$@
$1/%.o: %.cpp
	$(TZ_ARMGNU)-g++ $(CPPFLAGS) -c $$< -o $$@
endef

# build rules
.PHONY: kernel checkplatform checkdirs

all: kernel checkplatform checkdirs $(BASE_OBJ_DIR)/$(TZ_IMG).bin $(BASE_OBJ_DIR)/$(TZ_IMG).elf.lst

-include $(OOB_DEPS)

kernel:
	$(MAKE) -C $(TZ_KERNEL_DIR)

checkplatform:
	@echo Building for TZ_PLATFORM=$(TZ_PLATFORM)...

checkdirs: $(BIN_DIRS)

$(BIN_DIRS):
	mkdir -p $@

$(BASE_OBJ_DIR)/$(TZ_IMG).bin: $(BASE_OBJ_DIR)/$(TZ_IMG).elf
	$(TZ_ARMGNU)-objcopy $(BASE_OBJ_DIR)/$(TZ_IMG).elf -O binary $(BASE_OBJ_DIR)/$(TZ_IMG).bin

$(BASE_OBJ_DIR)/$(TZ_IMG).elf.lst: $(BASE_OBJ_DIR)/$(TZ_IMG).elf
	@echo generating listing: $@
	$(TZ_ARMGNU)-objdump -d $< | $(TZ_ARMGNU)-c++filt > $@

$(BASE_OBJ_DIR)/$(TZ_IMG).elf: $(OOB_OBJS) $(TZ_KERNEL_LIB) link_$(ARCH)_tz.ld
	$(TZ_ARMGNU)-g++ -nostartfiles -static $(OOB_OBJS) $(TZ_KERNEL_LIB) -Tlink_$(ARCH)_tz.ld -o $@

clean:
	$(RM) -r $(BASE_OBJ_DIR)

distclean: clean
	$(MAKE) -C $(TZ_KERNEL_DIR) clean

$(foreach bdir,$(BIN_DIRS),$(eval $(call make-goal,$(bdir))))
