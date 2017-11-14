############################################################
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
############################################################

ifndef B_REFSW_TOP
# we can derive B_REFSW_TOP from this include file's path
B_REFSW_TOP := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../..)
endif
NEXUS_TOP := $(B_REFSW_TOP)/nexus

ifeq ($(NEXUS_PREBUILT_BINARY),y)
# do "make api" and "make nexus_headers" first, then "make NEXUS_PREBUILT_BINARY=y"
NEXUS_BIN_DIR ?= $(B_REFSW_TOP)/obj.${NEXUS_PLATFORM}/nexus/bin
include $(NEXUS_BIN_DIR)/include/platform_app.inc
Q_ ?= @
else
NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error NEXUS_PLATFORM is not defined)
endif
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc
endif

# because of out-of-source build, $(APPS) is phony too
.PHONY: all clean nexus clean_nexus api clean_api apps clean_apps nexus_headers $(APPS)

THISDIR := $(subst $(NEXUS_TOP)/,,$(shell pwd))
BINDIR := $(B_REFSW_OBJ_ROOT)/nexus/${THISDIR}
TARGET_APPS := $(addprefix $(BINDIR)/,$(APPS))

all apps: $(APPS)
	@test 1

clean: clean_api clean_apps
	${Q_}if [ -d ${B_REFSW_OBJ_ROOT} ]; then find ${B_REFSW_OBJ_ROOT} -type d -empty -delete; fi

LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES) -lm
CLIENT_LDFLAGS := $(NEXUS_LDFLAGS) $(NEXUS_CLIENT_LD_LIBRARIES) -lm

ifeq ($(B_REFSW_ANDROID),y)
include $(NEXUS_TOP)/build/android.inc
LDFLAGS := $(ANDROID_SERVER_LDFLAGS)
CLIENT_LDFLAGS := $(ANDROID_CLIENT_LDFLAGS)
endif

# Clean api except on binary version
ifeq ($(NEXUS_PREBUILT_BINARY),y)
nexus api:
clean_nexus clean_api:
else
nexus api:
	@$(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/build
clean_nexus clean_api:
	@$(MAKE) $(NEXUS_BUILD_OPTIONS) -C $(NEXUS_TOP)/build clean
endif

# CLEAN_APPS allows cleaning apps that may not be in APPS because of env variables

clean_apps:
	-$(RM) $(TARGET_APPS) $(CLEAN_APPS)

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# Always build with debug
CFLAGS += -g

# use template to generate out-of-source build rules
define example_app_rule
$$(BINDIR)/$1: $1.c $$(if $${B_NEXUS_API_BUILD_COMPLETED}, , api)
	@echo [Compile... $$<]
	@mkdir -p $$(dir $$@)
	$$(Q_)$$(CC) -o $$@ $$(filter %.c %.s %.o, $$^) $$(CFLAGS) $$(LDFLAGS)
	cp -f $$@ $${NEXUS_BIN_DIR}

$1: $$(BINDIR)/$1
	@test 1
endef

$(foreach app, $(APPS), $(eval $(call example_app_rule,$(strip $(app)))))

install:
	cp -f ${TARGET_APPS} ${NEXUS_BIN_DIR}
