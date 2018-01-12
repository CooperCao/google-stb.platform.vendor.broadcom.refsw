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
TZ_MOD := monitor
TZ_IMG := mon64

# 64-bit only
TZ_ARCH := Arm64

include $(TZ_TOP)/build/build_env.mk

# build src
SUBMOD_DIRS := \
	boot \
	runtime \
	memory \
	astra \
	psci \
	scmi \
	dvfs \
	libc \
	libc/strings \
	drivers/uart_16550 \
	drivers/gicv2 \
	platform/$(TZ_PLATFORM) \

SRC_DIRS := $(SUBMOD_DIRS)
INC_DIRS := $(SUBMOD_DIRS)
BIN_DIRS := $(addprefix $(TZ_OBJ_DIR)/,$(SUBMOD_DIRS))

include $(TZ_TOP)/build/build_src.mk

# build flags
DEPENDFLAGS := -MD -MP

INCLUDES    := -I. $(addprefix -I,$(INC_DIRS))
INCLUDES    += -Iinclude
INCLUDES    += -Iinclude/arm

ifeq (y,$(DEBUG))
BASEFLAGS   := -Og -g
else
BASEFLAGS   := -O2
endif
BASEFLAGS   += -ffreestanding
BASEFLAGS   += -ffunction-sections
BASEFLAGS   += -fdata-sections
BASEFLAGS   += -mgeneral-regs-only
BASEFLAGS   += -DPLATFORM=$(TZ_PLATFORM)

WARNFLAGS   := -Werror
WARNFLAGS   += -Wall
WARNFLAGS   += -Wextra
WARNFLAGS   += -Wundef
WARNFLAGS   += -Wmissing-include-dirs
WARNFLAGS   += -Wredundant-decls

ifeq (y,$(DEBUG))
DBGFLAGS    := -DDEBUG
ifeq (y,$(VERBOSE))
DBGFLAGS    += -DVERBOSE
endif
ifeq (y,$(DHALT))
DBGFLAGS    += -DDHALT
endif
ifeq (y,$(DTEST))
DBGFLAGS    += -DDTEST
endif
endif

ASFLAGS     := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS)
ASFLAGS     += $(BASEFLAGS) $(WARNFLAGS) $(DBGFLAGS)
ASFLAGS     += -Wa,--fatal-warnings
ASFLAGS     += -D__ASSEMBLY__

CFLAGS      := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS)
CFLAGS      += $(BASEFLAGS) $(WARNFLAGS) $(DBGFLAGS)
CFLAGS      += -std=gnu99

CPPFLAGS    := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS)
CPPFLAGS    += $(BASEFLAGS) $(WARNFLAGS) $(DBGFLAGS)
CPPFLAGS    += -std=c++11
CPPFLAGS    += -fno-rtti -fno-exceptions -fno-use-cxa-atexit
CPPFLAGS    += -D__EXCEPTIONS=0

LDFLAGS     := --gc-sections
LDFLAGS     += --fatal-warning

ifeq (y,$(MAPFILE))
LDFLAGS     += -Map=$(TZ_OBJ_DIR)/$(TZ_IMG).map
endif

# build rules
.PHONY: checkplatform checkdirs $(TZ_IMG)

all: $(TZ_IMG)

$(TZ_IMG): checkplatform checkdirs $(TZ_OBJ_DIR)/$(TZ_IMG).bin

checkplatform:
	@echo Building for TZ_PLATFORM=$(TZ_PLATFORM)...

checkdirs: $(BIN_DIRS)

$(BIN_DIRS):
	@mkdir -p $@

$(TZ_OBJ_DIR)/$(TZ_IMG).bin: $(TZ_OBJ_DIR)/$(TZ_IMG).elf
	$(TZ_ARMGNU)-objcopy $(TZ_OBJ_DIR)/$(TZ_IMG).elf -O binary $(TZ_OBJ_DIR)/$(TZ_IMG).bin

$(TZ_OBJ_DIR)/$(TZ_IMG).elf: $(OBJS) link_$(ARCH)_mon.ld
	/opt/toolchains/stbgcc-4.8-1.5/bin/aarch64-linux-ld -nostartfiles -static $(OBJS) -Tlink_$(ARCH)_mon.ld -o $@
	$(TZ_ARMGNU)-ld -nostartfiles -static $(OBJS) $(LDFLAGS) -Tlink_$(ARCH)_mon.ld -o $@

clean:
	$(RM) -r $(TZ_OBJ_DIR)

include $(TZ_TOP)/build/build_rules.mk
