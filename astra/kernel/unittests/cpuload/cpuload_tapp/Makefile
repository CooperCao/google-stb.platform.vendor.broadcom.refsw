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
TZ_DIR ?= ../../../..
TZ_TOOLCHAIN ?= /opt/toolchains/tzos/nonDSO/gcc-arm-tzos-musl-4_8-2016q1
TZ_ARMGNU ?= $(TZ_TOOLCHAIN)/bin/arm-tzos-musl

# platform and target
PLATFORM ?= brcmstb

# directories and files
USER_APP := cpuload_tapp

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
DEP_LIBS += libtzioc
INC_DIRS += libtzioc

# build flags
DEPENDFLAGS := -MD -MP

INCLUDES    := $(addprefix -I,$(INC_DIRS))
INCLUDES    += -I$(TZ_DIR)/kernel/api

# funcationally and alphabetically ordered
ARCHFLAGS   := -mcpu=cortex-a15

BASEFLAGS   := -O2 -g

WARNFLAGS   := -Wall
WARNFLAGS   += -Werror

OSFLAGS     := -D__TZOS__

ASFLAGS     := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
ASFLAGS     += -g -D__ASSEMBLY__

CFLAGS      := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
CFLAGS      += $(BASEFLAGS) $(WARNFLAGS) $(CHIPFLAGS)
CFLAGS      += -std=gnu99
CFLAGS      += -D_POSIX_SOURCE

CPPFLAGS    := $(INCLUDES) $(DEPENDFLAGS) $(ARCHFLAGS) $(OSFLAGS)
CPPFLAGS    += $(BASEFLAGS) $(WARNFLAGS) $(CHIPFLAGS)
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
.PHONY: checkplatform checkdirs $(USER_APP)

all: checkplatform checkdirs $(USER_APP)

-include $(DEPS)

checkplatform:
	@echo Building for PLATFORM=$(PLATFORM)...

checkdirs: $(BIN_DIRS)

$(BIN_DIRS):
	mkdir -p $@

$(USER_APP): $(USER_APP).elf

$(USER_APP).elf: $(DEP_LIBS) $(OBJS)
	$(TZ_ARMGNU)-g++ $(LDFLAGS) $(OBJS) $(foreach dlib,$(DEP_LIBS),$(dlib)/$(dlib).a) -o $@

.PHONY: $(DEP_LIBS)

$(DEP_LIBS):
	$(MAKE) -C $@

clean:
	$(RM) -r bin
	$(RM) $(USER_APP).elf
	@for dlib in $(DEP_LIBS); do \
		$(MAKE) -C $$dlib clean; \
	done

$(foreach bdir,$(BIN_DIRS),$(eval $(call make-goal,$(bdir))))
