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

ifndef B_REFSW_ARCH
B_REFSW_ARCH = mipsel-linux
endif

V3D_DIR ?= $(NEXUS_TOP)/../rockford/middleware/$(V3D_PREFIX)/driver
V3D_PLATFORM_DIR ?= $(NEXUS_TOP)/../rockford/middleware/$(V3D_PREFIX)/platform

# include cross-compiler definitions
include $(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/build/platform_app.inc

APPLIBS_TOP:=$(NEXUS_TOP)/../AppLibs
include $(APPLIBS_TOP)/opensource/directfb/build/directfb_common.inc

THEAPPS = eglimage

CFLAGS += -I$(V3D_DIR)/interface/khronos/include/ \
		  -I$(DIRECTFB_INSTALL_INCLUDE_DIR)/directfb \
		  -I../../nexus/common \
		  -I$(V3D_PLATFORM_DIR)/directfb

CFLAGS += -DGL_GLEXT_PROTOTYPES -D$(V3D_PREFIX) -DEGL_EGLEXT_PROTOTYPES

APPS = $(THEAPPS)

.PHONY: api directfb_check clean

ifeq ($(APP),)
all: $(APPS)
$(APPS): api
else
all: $(APP)
$(APP): api
endif

ifeq ($(V3D_DEBUG),y)
V3D_LIB := lib_$(NEXUS_PLATFORM)_debug
else
V3D_LIB := lib_$(NEXUS_PLATFORM)_release
endif

V3DDRIVER_LIB_TARGET := $(APPLIBS_TARGET_LIB_DIR)
V3DDRIVER_OBJ_TARGET := ${B_REFSW_OBJ_ROOT}/rockford/middleware/v3d/$(V3D_OBJ)
DBPL_LIB_TARGET := $(APPLIBS_TARGET_LIB_DIR)
DBPL_OBJ_TARGET := ${B_REFSW_OBJ_ROOT}/rockford/middleware/platform/nexus/$(V3D_OBJ)

# For linux builds, link to the correct libraries
ifneq ($(findstring linux,$(B_REFSW_OS)),)
LDFLAGS := -lnexus$(NEXUS_LIB_SUFFIX) -L${NEXUS_BIN_DIR} -lpthread -lm -lrt \
		-L$(APPLIBS_TARGET_LIB_DIR) \
		-lv3ddriver \
		-ldbpl \
		-L$(DIRECTFB_INSTALL_LIB_DIR) \
		-Wl,-rpath-link,$(DIRECTFB_INSTALL_LIB_DIR) \
		-ldirectfb \
		-lstdc++
endif

# Check to make sure DirectFB is already built
directfb_check:
	@if [ ! -d $(DIRECTFB_INSTALL_DIRECTFB_INCLUDE_DIR) ]; then \
		echo "ERROR: Cannot find directfb installation - aborting!"; \
		/bin/false; \
	fi

# This builds the nexus api, v3d driver & platform glue
api: directfb_check
	$(MAKE) -C $(NEXUS_TOP)/build
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET)
	$(MAKE) -C $(V3D_PLATFORM_DIR)/directfb -f platform_directfb.mk OBJDIR=$(DBPL_OBJ_TARGET) LIBDIR=$(DBPL_LIB_TARGET)

# This cleans nexus and local apps
clean:
	$(MAKE) -C $(NEXUS_TOP)/build clean
	$(MAKE) -C $(V3D_DIR) -f V3DDriver.mk OBJDIR=$(V3DDRIVER_OBJ_TARGET) LIBDIR=$(V3DDRIVER_LIB_TARGET) clean
	$(MAKE) -C $(V3D_PLATFORM_DIR)/directfb -f platform_directfb.mk OBJDIR=$(DBPL_OBJ_TARGET) LIBDIR=$(DBPL_LIB_TARGET) clean
	-$(RM) $(APPS) *.d *.o *.out;

# DFB has lots of warnings if not gnu99
# Always build with gnu99 & debug
CFLAGS += -std=gnu99 -g

EXTRA_OBJS := ../../nexus/common/esutil.c

# Implicit rule for building local apps
%$(APPSUFFIX): %.c $(EXTRA_OBJS) $(VXOBJS)
	@echo [Compile... $<]
	$(Q_)$(CC) -o $(APPLIBS_TARGET_ROOTDIR)/$(DIRECTFB_TARGET_BIN_DFB_VER_DIR)/$@ $(filter %.c %.s %.o, $^) $(CFLAGS) $(LDFLAGS)
	mkdir -p $(APPLIBS_TARGET_ROOTDIR)/$(DIRECTFB_TARGET_BIN_DFB_VER_DIR)/textures; \
	install -m 664 ../../nexus/eglimage/*.png $(APPLIBS_TARGET_ROOTDIR)/$(DIRECTFB_TARGET_BIN_DFB_VER_DIR)/textures; \
	cd $(APPLIBS_TARGET_ROOTDIR)/$(DIRECTFB_TARGET_BIN_DFB_VER_DIR); \
	ln -sf ../../../share/directfb-${DIRECTFB_VERSION}/textures textures

install: all

