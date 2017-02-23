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
TZ_DIR ?= ../../../..
include $(TZ_DIR)/kernel/default.inc

# directories and files
USER_APP := user

SRC_DIRS := .
INC_DIRS := .
BASE_OBJ_DIR := $(B_REFSW_OBJ_ROOT)/kernel/unittests/userspace
BIN_DIR := $(BASE_OBJ_DIR)/user/bin

SOURCES_ASM := $(notdir $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.S)))
OBJS        := $(patsubst %.S,%.o,$(SOURCES_ASM))
DEPS        := $(patsubst %.S,%.d,$(SOURCES_ASM))

SOURCES_C   := $(notdir $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.c)))
OBJS        += $(patsubst %.c,%.o,$(SOURCES_C))
DEPS        += $(patsubst %.c,%.d,$(SOURCES_C))

SOURCES_CPP := $(notdir $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.cpp)))
OBJS        += $(patsubst %.cpp,%.o,$(SOURCES_CPP))
DEPS        += $(patsubst %.cpp,%.d,$(SOURCES_CPP))

# Create out of build (OOB) objects
OOB_OBJS	:= $(patsubst %.o, $(BIN_DIR)/%.o,$(OBJS))
OOB_DEPS	:= $(patsubst %.d, $(BIN_DIR)/%.d,$(DEPS))

# build flags
DEPENDFLAGS := -MD -MP

INCLUDES    := $(addprefix -I,$(INC_DIRS))
INCLUDES    += -I$(TZ_DIR)/kernel/api
INCLUDES    += -I$(SRC_DIRS)/../libhello

BASEFLAGS   := -O2 -g

WARNFLAGS   := -Wall
WARNFLAGS   += -Werror

OSFLAGS     := -D__TZOS__

ASFLAGS     := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
ASFLAGS     += -g -D__ASSEMBLY__

CFLAGS      := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
CFLAGS      += $(BASEFLAGS) $(WARNFLAGS)
CFLAGS      += -std=gnu99
CFLAGS      += -D_POSIX_SOURCE

CPPFLAGS    := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
CPPFLAGS    += $(BASEFLAGS) $(WARNFLAGS)
CPPFLAGS    += -std=c++11
CPPFLAGS    += -D_POSIX_SOURCE

LDFLAGS     :=

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
.PHONY: checkplatform checkdirs

all: checkplatform checkdirs $(BIN_DIR)/$(USER_APP).elf $(BIN_DIR)/$(USER_APP).elf.lst

-include $(OOB_DEPS)

checkplatform:
	@echo Building for PLATFORM=$(TZ_PLATFORM)...

checkdirs: $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $@

$(BIN_DIR)/$(USER_APP).elf.lst: $(BIN_DIR)/$(USER_APP).elf
	@echo generating listing: $@
	$(TZ_ARMGNU)-objdump -d $< | $(TZ_ARMGNU)-c++filt > $@

$(BIN_DIR)/$(USER_APP).elf: $(BIN_DIR)/$(OOB_OBJS)
	$(TZ_ARMGNU)-g++  $(LDFLAGS) -ldl $(OOB_OBJS) -o $@

clean:
	$(RM) -r $(BIN_DIR)

$(foreach bdir,$(BIN_DIR),$(eval $(call make-goal,$(bdir))))
