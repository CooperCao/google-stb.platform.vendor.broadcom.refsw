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

###############################################################################
# Broadcom Proprietary and Confidential. (c)2010-2014 Broadcom.  All rights reserved.
#
# This program is the proprietary software of Broadcom and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
###############################################################################

NEXUS_TOP ?= $(shell cd ../../../../../../nexus; pwd)

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error Neither NEXUS_PLATFORM or PLATFORM is defined)
endif

include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

# build paths
include ../common/common.inc

CFLAGS += -lm -lrt
CFLAGS += -I../common \
          -I$(V3D_DIR)/interface/khronos/include/ \
          -I$(V3D_PLATFORM_DIR)/nexus \
          -I$(NEXUS_TOP)/../BSEAV/lib/utils \
          -I$(NEXUS_TOP)/../BSEAV/lib/media

APPS = nxclient_360_video

.PHONY: api clean

TARGET_DIR := $(B_REFSW_OBJ_ROOT)/$(shell pwd | sed 's/.*\($(BASEPATH)\)/\1/g')

all: $(TARGET_DIR) $(APPS)
$(APPS): api

ifeq ($(V3D_DEBUG),y)
V3D_LIB := lib_$(NEXUS_PLATFORM)_debug
V3D_OBJ := obj_$(NEXUS_PLATFORM)_debug
else
V3D_LIB := lib_$(NEXUS_PLATFORM)_release
V3D_OBJ := obj_$(NEXUS_PLATFORM)_release
endif

V3DDRIVER_LIB_TARGET := ${NEXUS_BIN_DIR}
V3DDRIVER_OBJ_TARGET := ${B_REFSW_OBJ_ROOT}/$(shell sed 's/.*\($(BASEPATH)\)/\1/g' <<< $(V3D_DIR))/$(V3D_OBJ)
NXPL_LIB_TARGET := ${NEXUS_BIN_DIR}
NXPL_OBJ_TARGET := ${B_REFSW_OBJ_ROOT}/$(shell sed 's/.*\($(BASEPATH)\)/\1/g' <<< $(V3D_PLATFORM_DIR))/$(V3D_OBJ)

# For linux builds, link to the correct libraries
LDFLAGS := -lpthread \
		-lv3ddriver \
		-lnxpl

api:
	$(MAKE) -C $(NEXUS_TOP)/nxclient server
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET)
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y

$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

.PHONY: clean_api clean_v3d clean_app clean_nxclient
# This cleans nexus and local apps
clean: clean_nxclient clean_api clean_v3d clean_app

clean_api:
	$(MAKE) -C $(NEXUS_TOP)/build clean

clean_v3d:
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET) clean
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y clean

clean_app:
	$(RM) $(TARGET_DIR)/$(APPS) $(TARGET_DIR)/*.d $(TARGET_DIR)/*.o $(TARGET_DIR)/*.out;
	$(RM) $(NXCLIENT_APPS_UTILS_LIB) *.d *.o *.out

clean_nxclient:
	$(MAKE) -C $(NEXUS_TOP)/nxclient clean_server
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build clean

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES))

# Always build with debug
CFLAGS += -g -std=c99

include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS += $(NXCLIENT_CFLAGS)
LDFLAGS += $(NXCLIENT_LDFLAGS)

# for faster compilation, archive nexus/nxclient/apps/utils
NXCLIENT_APPS_UTILS_LIB := $(TARGET_DIR)/nxclient_apps_utils.a
include $(NEXUS_TOP)/nxclient/apps/utils/nxutils.inc
NXCLIENT_APPS_UTILS_SRC := $(foreach src,$(NXUTILS_SOURCES), $(notdir $(src)))
CFLAGS += $(addprefix -I,$(NXUTILS_INCLUDES))
vpath %.c $(foreach src,$(NXUTILS_SOURCES), $(dir $(src)))

EXTRA_OBJS := ../common/esutil.c ../common/init.c
$(NXCLIENT_APPS_UTILS_LIB): $(EXTRA_BUILD_RULES)
$(NXCLIENT_APPS_UTILS_LIB): $(addprefix $(TARGET_DIR)/,$(patsubst %.c,%.o,$(NXCLIENT_APPS_UTILS_SRC)))
	@echo [Archive... $@]
	$(Q_)$(AR) rcs $@ $(filter %.o, $^)

$(TARGET_DIR)/%.o: %.c
	@echo [Compile... $(notdir $<)]
	@mkdir -p $(dir %@)
	$(Q_)$(CC) -c -o $@ $< $(CFLAGS)

# Implicit rule for building local apps
%$(APPSUFFIX): %.c $(NXCLIENT_APPS_UTILS_LIB) $(EXTRA_OBJS)
	@echo [Compile... $<]
	$(Q_)$(CC) -o $(TARGET_DIR)/$@ $(filter %.c %.s %.o %.a, $^) $(CFLAGS) $(LDFLAGS)
	cp $(TARGET_DIR)/$@ ${NEXUS_BIN_DIR}

install: all
