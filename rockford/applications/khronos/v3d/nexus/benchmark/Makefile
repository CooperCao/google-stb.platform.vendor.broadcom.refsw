###############################################################################
#    Broadcom Proprietary and Confidential. (c)2010-2016 Broadcom.  All rights reserved.
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
#
###############################################################################

ifdef COMSPEC
# Any DOS environment
NEXUS_TOP ?= $(shell cd ../../../../../../nexus && cd)
else
NEXUS_TOP ?= $(shell cd ../../../../../../nexus; pwd)
endif

NEXUS_PLATFORM ?= $(PLATFORM)
ifndef NEXUS_PLATFORM
$(error Neither NEXUS_PLATFORM or PLATFORM is defined)
endif

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

V3D_DIR ?= $(NEXUS_TOP)/../rockford/middleware/$(V3D_PREFIX)/driver
V3D_PLATFORM_DIR ?= $(NEXUS_TOP)/../rockford/middleware/$(V3D_PREFIX)/platform

LDFLAGS = -lm -lrt -lstdc++

CFLAGS += -I../common \
          -I$(V3D_DIR)/interface/khronos/include/ \
          -I$(V3D_PLATFORM_DIR)/nexus \
          -I$(NEXUS_TOP)/../rockford/middleware/platform/nexus \
          -I$(NEXUS_TOP)/../BSEAV/lib/libpng \
          -I$(NEXUS_TOP)/../BSEAV/lib/zlib

APPS = benchmark

.PHONY: api clean

TARGET_DIR = ${B_REFSW_OBJ_ROOT}/rockford/applications/khronos/v3d/nexus/$(APPS)

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
V3DDRIVER_OBJ_TARGET := ${B_REFSW_OBJ_ROOT}/rockford/middleware/v3d/$(V3D_OBJ)
NXPL_LIB_TARGET := ${NEXUS_BIN_DIR}
NXPL_OBJ_TARGET := ${B_REFSW_OBJ_ROOT}/rockford/middleware/platform/nexus/$(V3D_OBJ)

# For linux builds, link to the correct libraries
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS += -lpthread \
		-lv3ddriver \
		-lnxpl \
		-L${B_REFSW_OBJ_ROOT}/BSEAV/lib/libpng -lpng \
		-L${B_REFSW_OBJ_ROOT}/BSEAV/lib/zlib/$(B_REFSW_ARCH) -lz
endif

api: nexus_api
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET)
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/zlib
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/libpng

ifeq ($(NXCLIENT_SUPPORT),y)
nexus_api:
	$(MAKE) -C $(NEXUS_TOP)/nxclient server
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build
else
nexus_api:
	$(MAKE) -C $(NEXUS_TOP)/build
endif

$(TARGET_DIR):
	mkdir -p ${B_REFSW_OBJ_ROOT}/rockford/applications/khronos/v3d/nexus/$(APPS)

.PHONY: clean_api clean_v3d clean_app clean_nxclient
# This cleans nexus and local apps
clean: clean_nxclient clean_api clean_v3d clean_app clean_png

clean_api:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	
clean_v3d:
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET) clean
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y clean
	
clean_app:
	$(RM) $(TARGET_DIR)/$(APPS) $(TARGET_DIR)/*.d $(TARGET_DIR)/*.o $(TARGET_DIR)/*.out;

clean_png:
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/zlib clean
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/libpng clean

ifeq ($(USE_NXCLIENT),1)
clean_nxclient:
	$(MAKE) -C $(NEXUS_TOP)/nxclient clean_server
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build clean
else
clean_nxclient:
	#
endif

# This is the minimum needed to compile and link with Nexus
CFLAGS += $(NEXUS_CFLAGS) $(addprefix -I,$(NEXUS_APP_INCLUDE_PATHS)) $(addprefix -D,$(NEXUS_APP_DEFINES)) -I$(NEXUS_TOP)/../BSEAV/lib/netaccel/include

# Always build with debug

CFLAGS += -g
ifeq ($(NXCLIENT_SUPPORT),y)
# add nxclient
include $(NEXUS_TOP)/nxclient/include/nxclient.inc
CFLAGS += $(NXCLIENT_CFLAGS)
LDFLAGS += $(NXCLIENT_LDFLAGS)
else
ifeq ($(NEXUS_CLIENT_SUPPORT),y)
LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_CLIENT_LD_LIBRARIES)
else
CFLAGS += -DSINGLE_PROCESS
LDFLAGS += $(NEXUS_LDFLAGS) $(NEXUS_LD_LIBRARIES)
endif
endif

SOURCES := \
	../common/esutil.c \
	../common/init.c \
	load_texture.c \
	benchmark.cpp

# $(1) = src
# $(2) = obj
define CCompileRule
OBJECTS += $(2)
$(2) : $(1)
	@echo [Compiling... $(1)]
	@$(CC) $(CFLAGS) -c -o "$(2)" "$(1)"

endef

REMOVEME = -std=c89
CXXFLAGS = $(filter-out $(REMOVEME),$(CFLAGS)) 

# $(1) = src
# $(2) = obj
define CPPCompileRule
OBJECTS += $(2)
$(2) : $(1)
	@echo [Compiling... $(1)]
	@$(CC) $(CXXFLAGS) -std=c++98 -c -o "$(2)" "$(1)"

endef

$(foreach src,$(filter %.c,$(SOURCES)),$(eval $(call CCompileRule,$(src),$(addprefix $(TARGET_DIR)/,$(basename $(notdir $(src))).o))))
$(foreach src,$(filter %.cpp,$(SOURCES)),$(eval $(call CPPCompileRule,$(src),$(addprefix $(TARGET_DIR)/,$(basename $(notdir $(src))).o))))

# Implicit rule for building local apps
$(APPS) : $(OBJECTS)
	@echo [Link... $@]
	@$(CC) $(filter %.cpp %.c %.s %.o, $^) $(LDFLAGS) -o $(TARGET_DIR)/$@
	cp -af $(TARGET_DIR)/$@ ${NEXUS_BIN_DIR}
	cp -af *.txt ${NEXUS_BIN_DIR}
	cp -af earth_cloud0.pkm ${NEXUS_BIN_DIR}
	@echo -e "\n\n*********************************************************************************"
	@echo "You can run the benchmark with './nexus benchmark batch=depth.txt +s d=1280x720'"
	@echo "or './nexus benchmark batch=nodepth.txt +s d=1280x720 -depth -stencil'"
	@echo "*********************************************************************************"


