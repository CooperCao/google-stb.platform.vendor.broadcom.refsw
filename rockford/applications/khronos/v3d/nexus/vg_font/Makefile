###############################################################################
# (c) 2010-14 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its
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

ifeq ($(V3D_PREFIX),vc5)
$(error OpenVG is not supported on this hardware)
endif

V3D_DIR ?= $(NEXUS_TOP)/../rockford/middleware/v3d/driver
V3D_PLATFORM_DIR ?= $(NEXUS_TOP)/../rockford/middleware/v3d/platform

CFLAGS += -lm -lrt
CFLAGS += -I$(V3D_DIR)/interface/khronos/include/ \
          -I$(V3D_PLATFORM_DIR)/nexus \
          -I$(NEXUS_TOP)/../BSEAV/lib/freetype-2.1.5/include

APPS = vg_font

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

ifeq ($(B_REFSW_ARCH),mipsel-linux)
  ARCH = mipsel
else
ifeq ($(B_REFSW_ARCH),mips-linux)
  ARCH = mips
else
  ARCH = arm
endif
endif

ifeq ($(B_REFSW_DEBUG),y)
FT_AR := $(ARCH)-linux.debug
else
FT_AR := $(ARCH)-linux.release
endif

# For linux builds, link to the correct libraries
LDFLAGS := -lpthread \
		-lv3ddriver \
		-lnxpl \
		$(NEXUS_TOP)/../BSEAV/lib/freetype-2.1.5/lib/$(FT_AR)/libfreetype.a

ifeq ($(NXCLIENT_SUPPORT),y)
# This builds the nexus api, v3d driver & platform glue
api:
	$(MAKE) -C $(NEXUS_TOP)/nxclient server
	$(MAKE) -C $(NEXUS_TOP)/nxclient/build
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET)
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/freetype-2.1.5
else
api:
	$(MAKE) -C $(NEXUS_TOP)/build
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET)
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/freetype-2.1.5
endif

$(TARGET_DIR):
	mkdir -p ${B_REFSW_OBJ_ROOT}/rockford/applications/khronos/v3d/nexus/$(APPS)

.PHONY: clean_api clean_v3d clean_app clean_nxclient
# This cleans nexus and local apps
clean: clean_nxclient clean_api clean_v3d clean_app

clean_api:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	
clean_v3d:
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET) clean
	$(MAKE) -C $(V3D_PLATFORM_DIR)/nexus -f platform_nexus.mk OBJDIR=$(NXPL_OBJ_TARGET) LIBDIR=$(NXPL_LIB_TARGET) NO_V3DDRIVER_BUILD=y clean
	$(MAKE) -C $(NEXUS_TOP)/../BSEAV/lib/freetype-2.1.5 clean
	
clean_app:
	$(RM) $(TARGET_DIR)/$(APPS) $(TARGET_DIR)/*.d $(TARGET_DIR)/*.o $(TARGET_DIR)/*.out;

ifeq ($(NXCLIENT_SUPPORT),y)
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

EXTRA_OBJS := ../common/init.c

# Implicit rule for building local apps
%$(APPSUFFIX): %.c $(EXTRA_OBJS)
	@echo [Compile... $<]
	$(Q_)$(CC) -o $(TARGET_DIR)/$@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	cp $(TARGET_DIR)/$@ ${NEXUS_BIN_DIR}
	cp -f *.ttf ${NEXUS_BIN_DIR}

install: all
