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
# Nexus examples

PLATFORM?=$(NEXUS_PLATFORM)
POD_STAGING_DIR = ${B_REFSW_OBJ_ROOT}/BSEAV/cable/lib/mpod
export B_REFSW_TOOLCHAIN_VER_GE_4_8=y

ifdef COMSPEC
# Any DOS environment
B_LIB_TOP := $(shell cd ../../../../nexus/lib && cd)
else
B_LIB_TOP := $(shell cd ../../../../nexus/lib; pwd)
B_CABLE_TOP := $(shell cd ../../../../BSEAV/cable; pwd)
ifeq ($(findstring $(NEXUS_PLATFORM), 97445 97145 97260 97271 97278), $(PLATFORM))
B_CP_LIB := $(B_CABLE_TOP)/lib/mpod/lib/cp_arm
else
B_CP_LIB := $(B_CABLE_TOP)/lib/mpod/lib/cp
endif
B_SI_LIB := $(shell cd ../../../../BSEAV/cable/lib/si; pwd)
endif
NEXUS_TOP := $(B_LIB_TOP)/..

ifndef PLATFORM
$(error PLATFORM is not defined)
endif

# include cross-compiler definitions
include $(B_LIB_TOP)/build/b_lib_defs.inc

APPS = mpod_test

.PHONY: api clean

ifeq ($(APP),)
all: $(APPS)
$(APPS): api
else
all: $(APP)
$(APP): api
endif

# include applib specifics
include $(B_LIB_TOP)/os/b_os_lib.inc
include $(B_CABLE_TOP)/lib/mpod/b_mpod_lib.inc



ARCH = ${B_REFSW_ARCH}
TOOLCHAIN_DIR:=$(shell dirname $(shell dirname $(shell which ${ARCH}-gcc 2>/dev/null) 2>/dev/null))
CROSS_COMPILE ?= ${ARCH}-

# This builds nexus and needed applibs
api:
	@echo "$(ESTB_CFG_SUPPORT) ..."
	@echo $(ESTB_CFG_SUPPORT)
	$(MAKE) -C $(NEXUS_TOP)/build
	$(MAKE) -C $(B_LIB_TOP)/os
	$(MAKE) -C $(B_CABLE_TOP)/lib/mpod DSG_SUPPORT=$(DSG_SUPPORT) ESTB_CFG_SUPPORT=$(ESTB_CFG_SUPPORT)
	$(MAKE) -C $(B_CP_LIB)/build -f makefile.cplib TOOLCHAIN=uclibc TOOLCHAIN_DIR=$(TOOLCHAIN_DIR) CROSS_COMPILE=$(CROSS_COMPILE)
ifeq ($(ESTB_CFG_SUPPORT),y)
	$(MAKE) -C $(B_CABLE_TOP)/lib/estb_cfg
endif

# This cleans nexus and local apps
clean:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	$(MAKE) -C $(B_LIB_TOP)/os clean
	$(MAKE) -C $(B_CABLE_TOP)/lib/mpod clean DSG_SUPPORT=$(DSG_SUPPORT) ESTB_CFG_SUPPORT=$(ESTB_CFG_SUPPORT)
	$(MAKE) -C $(B_CP_LIB)/build -f makefile.cplib TOOLCHAIN=uclibc clean
ifeq ($(ESTB_CFG_SUPPORT),y)
	$(MAKE) -C $(B_CABLE_TOP)/lib/estb_cfg clean
endif
	-$(RM) $(B_CP_LIB)/build/cplib.lib $(B_CP_LIB)/build/libcp.a
	-$(RM) $(APPS) *.d *.o *.out;

ifneq ($(findstring $(NEXUS_PLATFORM), 97445 97145 97260 97271 97278), $(NEXUS_PLATFORM))
LDFLAGS := -lb_mpod -lcp -L$(B_CP_LIB)/build -lcrypto.0.9.7b_uclibc -L$(B_CP_LIB)/crypto/linux
else
#LDFLAGS := -lb_mpod -lcp -L$(B_CP_LIB)/build -lcrypto -L$(B_CP_LIB)/crypto/linux
LDFLAGS := -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -lb_estb_cfg -L${B_REFSW_OBJ_ROOT}/nexus/bin -lb_mpod -L$(POD_STAGING_DIR) -lcp -L$(POD_STAGING_DIR) -lcrypto -L$(B_CP_LIB)/crypto/linux -ldl

endif
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -lpthread -lm
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -lnexus
LDFLAGS += -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -lb_os
ifeq ($(ESTB_CFG_SUPPORT),y)
#LDFLAGS += -L$(NEXUS_TOP)/../obj.$(PLATFORM)/nexus/bin -lb_estb_cfg
endif


# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))
ifeq ($(ESTB_CFG_SUPPORT),y)
CFLAGS += -DESTB_CFG_SUPPORT -I$(B_CABLE_TOP)/lib/estb_cfg/include
endif


# Convert applib include paths into single variable.
APPLIB_INCLUDES := $(foreach lib,$(B_LIBS),$($(lib)_PUBLIC_INCLUDES))
CFLAGS += $(addprefix -I,$(APPLIB_INCLUDES))
APPLIB_DEFINES := $(foreach lib,$(B_LIBS),$($(lib)_DEFINES))
CFLAGS += $(addprefix -D,$(APPLIB_DEFINES))

# Always build with debug
CFLAGS += -g

# Implicit rule for building local apps
%$(APPSUFFIX): %.c mpod_test_io.c
	@echo [Compile... $<]
	$(Q_)$(CC) -o $@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS) -rdynamic
	@echo [Install... $@]
	$(Q_)$(CP) $@ $(NEXUS_TOP)/bin

#
# Installation: copying app to DESTDIR
#
ifeq ($(DESTDIR),)
DESTDIR = $(NEXUS_TOP)/bin
endif

.PHONY: install install_only
install: all install_only

ifeq ($(APP),)
install_only:
	$(CP) $(APPS) $(DESTDIR)
else
install_only:
	$(CP) $(APP) $(DESTDIR)
endif

