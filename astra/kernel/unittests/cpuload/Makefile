#############################################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
TZ_TOOLCHAIN ?= /opt/toolchains/tzos/nonDSO/gcc-arm-tzos-musl-4_8-2016q1
TZ_ARMGNU ?= $(TZ_TOOLCHAIN)/bin/arm-tzos-musl

# platform and target
PLATFORM ?= brcmstb

# directories and files
TZ_IMG := cpuload

TZ_KERNEL_DIR := $(TZ_DIR)/kernel
TZ_KERNEL_LIB := $(TZ_KERNEL_DIR)/bin/lib/libtzKernel.a

# use local init module
SRC_DIRS := init
INC_DIRS := init
BIN_DIRS := bin/init

SOURCES_ASM := $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.S))
OBJS        := $(patsubst %.S,bin/%.o,$(SOURCES_ASM))
DEPS        := $(patsubst %.S,bin/%.d,$(SOURCES_ASM))

SOURCES_C   := $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.c))
OBJS        += $(patsubst %.c,bin/%.o,$(SOURCES_C))
DEPS        += $(patsubst %.c,bin/%.d,$(SOURCES_C))

SOURCES_CPP := $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.cpp))
OBJS        += $(patsubst %.cpp,bin/%.o,$(SOURCES_CPP))
DEPS        += $(patsubst %.cpp,bin/%.d,$(SOURCES_CPP))

# user apps to build
USER_APPS := \
	uappd \
	cpuload_tapp

# build flags
DEPENDFLAGS := -MD -MP

INCLUDES    := -I. $(addprefix -I,$(INC_DIRS))
INCLUDES    += -I../../include -I ../../api -I ../../3rd_party/libfdt
INCLUDES    += -I../../platform/$(PLATFORM)

# funcationally and alphabetically ordered
ARCHFLAGS   := -mcpu=cortex-a15
ARCHFLAGS   += -mfpu=vfpv3

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
CFLAGS      += $(BASEFLAGS) $(WARNFLAGS) $(CHIPFLAGS)
CFLAGS      += -std=gnu99

CPPFLAGS    := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
CPPFLAGS    += $(BASEFLAGS) $(WARNFLAGS) $(CHIPFLAGS)
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
.PHONY: kernel checkplatform checkdirs $(TZ_IMG) $(USER_APPS)

all: kernel checkplatform checkdirs $(TZ_IMG)

-include $(DEPS)

kernel:
	$(MAKE) -C $(TZ_KERNEL_DIR)

checkplatform:
	@echo Building for PLATFORM=$(PLATFORM)...

checkdirs: $(BIN_DIRS)

$(BIN_DIRS):
	mkdir -p $@

$(TZ_IMG): $(TZ_IMG).elf
	$(TZ_ARMGNU)-objcopy $(TZ_IMG).elf -O binary $(TZ_IMG)

$(TZ_IMG).elf: $(OBJS) initramfs.o link_arm_tz.ld
	$(TZ_ARMGNU)-g++ -nostartfiles $(OBJS) $(TZ_KERNEL_LIB) initramfs.o -Tlink_arm_tz.ld -o $@

initramfs.o: $(USER_APPS)
	@echo Copying user apps...
	@$(RM) -r ramfs.files ramfs.cpio
	@for app in $(USER_APPS); do \
		cp $$app/$$app.elf .; \
		echo $$app.elf >> ramfs.files; \
	done
	@cat ramfs.files | cpio -ov > ramfs.cpio
	$(TZ_ARMGNU)-objcopy -I binary -O elf32-littlearm -B arm --rename-section .data=.initramfs ramfs.cpio initramfs.o

$(USER_APPS):
	$(MAKE) -C $@

clean:
	$(RM) -r bin
	$(RM) $(TZ_IMG) $(TZ_IMG).elf
	$(RM) initramfs.o ramfs.files ramfs.cpio
	$(RM) $(addsuffix .elf,$(USER_APPS))
	@for uapp in $(USER_APPS); do \
		$(MAKE) -C $$uapp clean; \
	done

distclean: clean
	$(MAKE) -C $(TZ_KERNEL_DIR) clean

$(foreach bdir,$(BIN_DIRS),$(eval $(call make-goal,$(bdir))))
