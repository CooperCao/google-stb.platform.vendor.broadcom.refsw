############################################################
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
############################################################

APPS = license_test

.PHONY: all clean clean_apps install server clean_server
.PHONY: $(APPS)

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif

NEXUS_TOP := $(shell cd ../../../../../nexus;pwd)

install: all

all: $(EXTRA_BUILD_RULES) $(APPS)

ifeq ($(NEXUS_PREBUILT_BINARY),y)
# do "make nexus" and "make nexus_headers" first, then "make NEXUS_PREBUILT_BINARY=y"
NEXUS_BIN_DIR ?= $(NEXUS_TOP)/../obj.${NEXUS_PLATFORM}/nexus/bin
include $(NEXUS_BIN_DIR)/include/platform_app.inc
Q_ ?= @
else
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif

BINDIR := $(B_REFSW_OBJ_ROOT)/BSEAV/cable/lib/soft_license/$(B_REFSW_ARCH)/example.$(NEXUS_MODE)
NXCLIENT_P_SHARED_LIB := $(BINDIR)/libnxclient.so

# NEXUS flags
CFLAGS += -g $(NEXUS_CFLAGS) ${NEXUS_INTERNAL_CFLAGS} $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# NXCLIENT flags
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS += $(NXCLIENT_CFLAGS)
LDFLAGS += $(NXCLIENT_LDFLAGS)
LDFLAGS += -g

$(NXCLIENT_P_SHARED_LIB):
	${Q_}$(MAKE) -C $(NEXUS_TOP)/nxclient/server

B_LIB_TOP = $(NEXUS_TOP)/lib
B_BSEAV_TOP = $(NEXUS_TOP)/../BSEAV

#
# os library
#
.PHONY: os_lib clean_os_lib
EXTRA_BUILD_RULES += os_lib
EXTRA_CLEAN_RULES += clean_os_lib
include $(B_LIB_TOP)/os/b_os_lib.inc

os_lib:
	${Q_}$(MAKE) REFSW_BUILD=y -C $(B_LIB_TOP)/os B_NEXUS_API_BUILD_COMPLETED=y install
clean_os_lib:
	$(Q_)$(MAKE) -C $(B_LIB_TOP)/os clean

#
# soft license library
#
.PHONY: soft_license clean_soft_license
EXTRA_BUILD_RULES += soft_license
EXTRA_CLEAN_RULES += clean_soft_license
include $(B_BSEAV_TOP)/cable/lib/soft_license/b_soft_license.inc

soft_license:
	${Q_}$(MAKE) REFSW_BUILD=y -C $(B_BSEAV_TOP)/cable/lib/soft_license B_NEXUS_API_BUILD_COMPLETED=y install
clean_soft_license:
	$(Q_)$(MAKE) -C $(B_BSEAV_TOP)/cable/lib/soft_license clean


ifeq ($(NXCLIENT_LOCAL),y)
# Don't link with -lnxclient.
# Instead, client will link with nxclient_local.c which will init the server with default params.
# Server will still provide IPC for other clients.
LDFLAGS = $(NEXUS_LD_LIBRARIES) -lnxserver
endif

$(addprefix $(BINDIR)/,$(APPS)): $(NXCLIENT_P_SHARED_LIB) $(EXTRA_BUILD_RULES)

$(BINDIR)/%.o: %.c
	@echo [Compile... $(notdir $<)]
	@mkdir -p $(dir $@)
	$(Q_)$(CC) -c -o $@ $< $(CFLAGS)

# Convert applib include paths into single variable.
APPLIB_INCLUDES := $(foreach lib,$(B_LIBS),$($(lib)_PUBLIC_INCLUDES))
APPLIB_DEFINES := $(foreach lib,$(B_LIBS),$($(lib)_DEFINES))
$(BINDIR)/license_test: CFLAGS += $(addprefix -I,$(APPLIB_INCLUDES)) $(addprefix -D,$(APPLIB_DEFINES))
$(BINDIR)/license_test: LDFLAGS += -lb_soft_license -lb_os

define apprule
$$(BINDIR)/$1: $1.c
	@echo [Compile... $$<]
	@mkdir -p $$(dir $$@)
	$$(Q_)$$(CC) -o $$@ $$(filter %.c %.s %.o %.a, $$^) $$(CFLAGS) $$(LDFLAGS)
	$$(CP) $$@ $${NEXUS_BIN_DIR}

$1: $$(BINDIR)/$1
	@test 1
endef
$(foreach app, $(APPS), $(eval $(call apprule,$(strip $(app)))))

clean: clean_server clean_apps $(EXTRA_CLEAN_RULES)

clean_apps:
	${Q_}$(MAKE) -C $(NEXUS_TOP)/nxclient/server clean_apps
	-$(RM) -r $(BINDIR)

# install everything on "all", even if not rebuilding, to facilitate B_REFSW_ARCH switches
all:
	@echo [Install... license_test to $(realpath ${NEXUS_BIN_DIR})]
	${Q_}$(CP) $(addprefix $(BINDIR)/,$(APPS)) $(NEXUS_BIN_DIR)
