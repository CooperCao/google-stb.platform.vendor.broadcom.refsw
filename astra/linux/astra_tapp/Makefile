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
LINUX_TOOLCHAIN ?= /projects/stbopt_p/toolchains_303/stbgcc-4.8-1.2
LINUX_CROSS_COMPILE ?= arm-linux-gnueabihf
LINUX_USER_ARMGNU ?= $(LINUX_TOOLCHAIN)/bin/$(LINUX_CROSS_COMPILE)
LINUX_USER_SYSROOT ?= $(LINUX_TOOLCHAIN)/$(LINUX_CROSS_COMPILE)/sys-root

TZ_DIR ?= $(CURDIR)/../..

TARGET ?= 7445
TARGET_REV ?= d0
PLATFORM ?= brcmstb

# directories and files
USER_APP := astra_tapp

SRC_DIRS := .
INC_DIRS := .
BIN_DIRS := bin

SOURCES_ASM := $(notdir $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.S)))
OBJS        := $(patsubst %.S,bin/%.o,$(SOURCES_ASM))
DEPS        := $(patsubst %.S,bin/%.d,$(SOURCES_ASM))

SOURCES_C   := $(notdir $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.c)))
OBJS        += $(patsubst %.c,bin/%.o,$(SOURCES_C))
DEPS        += $(patsubst %.c,bin/%.d,$(SOURCES_C))

SOURCES_CPP := $(notdir $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.cpp)))
OBJS        += $(patsubst %.cpp,bin/%.o,$(SOURCES_CPP))
DEPS        += $(patsubst %.cpp,bin/%.d,$(SOURCES_CPP))

# dependent libs
DEP_LIBS += libbcm_astra
INC_DIRS += libbcm_astra

# build flags
DEPENDFLAGS := -MD -MP

INCLUDES    := $(addprefix -I,$(INC_DIRS))
INCLUDES    += --sysroot="$(LINUX_USER_SYSROOT)"

# funcationally and alphabetically ordered
ifeq ($(CFG_ARM32_core),y)
ARCH        := arm
ARCHFLAGS   += -mcpu=cortex-a15
ARCHFLAGS   += -mfpu=vfpv3
endif

ifeq ($(CFG_ARM64_core),y)
ARCH        := aarch64
ARCHFLAGS   += -mcpu=cortex-a53
endif

BASEFLAGS   := -O2 -g

WARNFLAGS   := -Wall
WARNFLAGS   += -Werror

CHIPFLAGS   := -DTARGET_SOC=$(TARGET)
CHIPFLAGS   += -DTARGET_SOC_REV=$(TARGET_REV)

OSFLAGS     := -D__linux__

ASFLAGS     := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
ASFLAGS     += -g -D__ASSEMBLY__

CFLAGS      := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
CFLAGS      += $(BASEFLAGS) $(WARNFLAGS) $(CHIPFLAGS)
CFLAGS      += -std=gnu99
CFLAGS      += -D_GNU_SOURCE

CPPFLAGS    := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
CPPFLAGS    += $(BASEFLAGS) $(WARNFLAGS) $(CHIPFLAGS)
CPPFLAGS    += -std=c++11
CPPFLAGS    += -D_GNU_SOURCE

ifeq (y,$(DEBUG_ENABLE_COREDUMP))
LDFLAGS     := -pthread -lrt -lbfd -liberty $(LIBC) -L$(ROOTDIR)/lib/binutils/bfd/. -L$(ROOTDIR)/lib/binutils/libiberty/.
else
LDFLAGS     := -pthread -lrt
endif

vpath %.S   $(SRC_DIRS)
vpath %.c   $(SRC_DIRS)
vpath %.cpp $(SRC_DIRS)

define make-goal
$1/%.o: %.c
	$(LINUX_USER_ARMGNU)-gcc $(CFLAGS) -c $$< -o $$@
$1/%.o: %.S
	$(LINUX_USER_ARMGNU)-gcc $(ASFLAGS) -c $$< -o $$@
$1/%.o: %.cpp
	$(LINUX_USER_ARMGNU)-g++ $(CPPFLAGS) -c $$< -o $$@
endef

# build rules
.PHONY: checktargets checkdirs $(USER_APP)

all: checktargets checkdirs $(USER_APP)

-include $(DEPS)

checktargets:
	@echo TARGET=$(TARGET) TARGET_REV=$(TARGET_REV)

checkdirs: $(BIN_DIRS)

$(BIN_DIRS):
	mkdir -p $@

$(USER_APP): $(USER_APP).elf

$(USER_APP).elf: $(DEP_LIBS) $(OBJS)
	$(LINUX_USER_ARMGNU)-g++ $(OBJS) $(foreach dlib,$(DEP_LIBS),$(dlib)/$(dlib).a) $(LDFLAGS) -o $@

.PHONY: $(DEP_LIBS)

$(DEP_LIBS):
	$(MAKE) -C $@

clean:
	$(RM) -r bin
	$(RM) $(USER_APP).elf
	@for dlib in $(DEP_LIBS); do \
		$(MAKE) -C $$dlib clean; \
	done

install:all
ifeq ($(INSTALL_DIR),)
	$(error INSTALL_DIR is undefined)
else
	@echo "[Install... "$(USER_APP)"]"
	@cp $(USER_APP).elf $(INSTALL_DIR)
endif

$(foreach bdir,$(BIN_DIRS),$(eval $(call make-goal,$(bdir))))
