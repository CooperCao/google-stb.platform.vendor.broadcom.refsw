#############################################################################
# Copyright (C) 2018 Broadcom.
# The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to
# the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied),
# right to use, or waiver of any kind with respect to the Software, and
# Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
# THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
# IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#############################################################################
# build env
TZ_TOP := $(CURDIR)/../../..
TZ_MOD := linux/examples/nexus_tapp
TZ_APP := nexus_tapp
NEXUS_TOP ?= $(TZ_TOP)/../nexus


include $(TZ_TOP)/build/build_linux_env.mk

NEXUS_PLATFORM ?= $(PLATFORM)

# Include nexus definitions
include $(NEXUS_TOP)/platforms/common/build/nexus_platforms.inc
include $(NEXUS_TOP)/platforms/common/build/platform_app.inc
include $(NEXUS_TOP)/build/nexus_defs.inc

# Convert include paths into single variable
NEXUS_APP_INCLUDE_PATHS := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_PUBLIC_INCLUDES))
NEXUS_APP_DEFINES := $(foreach module, $(NEXUS_MODULES), $(NEXUS_$(module)_DEFINES))
NEXUS_APP_DEFINES += $(foreach module, $(NEXUS_MODULES),NEXUS_HAS_$(module))

# Convert magnum includes into the same variable
NEXUS_APP_INCLUDE_PATHS += $(foreach module, $(MAGNUM_MODULES), $($(module)_INCLUDES))
NEXUS_APP_DEFINES += $(foreach module, $(MAGNUM_MODULES), $($(module)_DEFINES))


# directories and files
USER_APP := $(TZ_APP)

SRC_DIRS := .
INC_DIRS := .
BIN_DIRS := $(TZ_OBJ_DIR)

include $(TZ_TOP)/build/build_src.mk

# dependent libs
DEP_LIBS = libbcm_astra
DEP_LIBS_DIR += $(TZ_TOP)/linux/libbcm_astra
INC_DIRS += $(TZ_TOP)/linux/libbcm_astra

# build flags
DEPENDFLAGS := -MD -MP

INCLUDES    := $(addprefix -I,$(INC_DIRS))
INCLUDES    += --sysroot="$(LINUX_USER_SYSROOT)"

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

# This is the minimum needed to compile and link with Nexus
CFLAGS 		+= $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

ifeq ($(NXCLIENT_SUPPORT),y)
# NXCLIENT flags
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS 		+= $(NXCLIENT_CFLAGS)
LDFLAGS 	+= $(NXCLIENT_LDFLAGS)
endif

ifeq (y,$(DEBUG_ENABLE_COREDUMP))
LDFLAGS     := -pthread -lrt -lbfd -liberty $(LIBC) -L$(ROOTDIR)/lib/binutils/bfd/. -L$(ROOTDIR)/lib/binutils/libiberty/.
else
LDFLAGS     := -pthread -lrt
endif



ifeq ($(NXCLIENT_SUPPORT),y)
LDFLAGS 	+= -L$(NEXUS_BIN_DIR) -lnxclient -lnexus_client
else
LDFLAGS 	+= -L$(NEXUS_BIN_DIR) -lnexus
endif

# build rules
.PHONY: checkplatform checkdirs prep_nexus $(USER_APP) $(DEP_LIBS)

all: checkplatform checkdirs prep_nexus $(USER_APP)

$(USER_APP): $(USER_APP).elf

$(USER_APP).elf: $(DEP_LIBS) $(OBJS)
	$(TZ_ARMGNU)-g++ $(OBJS) $(foreach dlib,$(DEP_LIBS),$(DEP_LIBS_DIR)/$(dlib).a) $(LDFLAGS) -o $@
	@mv $(USER_APP).elf $(NEXUS_BIN_DIR)

$(DEP_LIBS):
	$(MAKE) -C $(DEP_LIBS_DIR) TARGET=$(TARGET) TARGET_REV=$(TARGET_REV) ARCH=$(ARCH)

prep_nexus:
	${Q_}if [ ! -e $(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/nexus/bin/nxserver ]; then \
		echo [ building nxclient .... ]; \
		$(MAKE) -C $(NEXUS_TOP)/nxclient; \
	fi
	${Q_}if [ ! -e $(NEXUS_TOP)/../obj.$(NEXUS_PLATFORM)/nexus/bin/libnexus.so ]; then \
		echo [ building nexus .... ]; \
		$(MAKE) -C $(NEXUS_TOP)/build; \
	fi


clean:
	$(RM) -r bin
	$(RM) $(USER_APP).elf
	@for dlib in $(DEP_LIBS); do \
		$(MAKE) -C $(DEP_LIBS_DIR) clean; \
	done

install:all
ifeq ($(INSTALL_DIR),)
	$(error INSTALL_DIR is undefined)
else
	@echo "[Install... "$(USER_APP)"]"
	@cp $(USER_APP).elf $(INSTALL_DIR)
endif

include $(TZ_TOP)/build/build_rules.mk
