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
TZ_TOP := ../../..
TZ_MOD := kernel/unittests/timer
TZ_IMG := timer_tests

include $(TZ_TOP)/build/build_env.mk

# build src
# use local init module (to build image)
SRC_DIRS := init
INC_DIRS := init
BIN_DIRS := $(TZ_OBJ_DIR)/init

include $(TZ_TOP)/build/build_src.mk

# build flags
DEPENDFLAGS := -MD -MP

INCLUDES    := -I. $(addprefix -I,$(INC_DIRS))
INCLUDES    += -I$(TZ_KERNEL_DIR)/api
INCLUDES    += -I$(TZ_KERNEL_DIR)/api/arch/$(ARCH_DIR)
INCLUDES    += -I$(TZ_KERNEL_DIR)/include
INCLUDES    += -I$(TZ_KERNEL_DIR)/include/arm
INCLUDES    += -I$(TZ_KERNEL_DIR)/platform/$(TZ_PLATFORM)/include
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

# build rules
.PHONY: checkplatform checkdirs libkernel $(TZ_IMG) $(USER_APPS) $(USER_LIBS)

all: $(TZ_IMG)

$(TZ_IMG): checkplatform checkdirs $(TZ_OBJ_DIR)/$(TZ_IMG).bin

$(TZ_OBJ_DIR)/$(TZ_IMG).bin: $(TZ_OBJ_DIR)/$(TZ_IMG).elf
	$(TZ_ARMGNU)-objcopy $(TZ_OBJ_DIR)/$(TZ_IMG).elf -O binary $(TZ_OBJ_DIR)/$(TZ_IMG).bin

$(TZ_OBJ_DIR)/$(TZ_IMG).elf: $(OBJS) libkernel link_$(ARCH)_tz.ld
	$(TZ_ARMGNU)-g++ -nostartfiles -static $(OBJS) $(TZ_KERNEL_OBJ_DIR)/libkernel.a -Tlink_$(ARCH)_tz.ld -o $@

libkernel:
	$(MAKE) -C $(TZ_KERNEL_DIR) libkernel

clean:
	$(RM) -r $(TZ_OBJ_DIR)

distclean: clean
	$(MAKE) -C $(TZ_KERNEL_DIR) clean

include $(TZ_TOP)/build/build_rules.mk
